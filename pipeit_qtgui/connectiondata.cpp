// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "connectiondata.h"

#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLocalSocket>
#include <QTextCodec>
#include <QTextDecoder>

#include <QDebug>

#include "common.h"

static const QByteArray HDR_MAGIC("pipeit ");


ConnectionData::ConnectionData(QAbstractItemModel *model, const QPersistentModelIndex &modelIndex, QLocalSocket *client, QObject *parent)
    : QObject(parent)
    , model(model)
    , modelIndex(modelIndex)
    , key(modelIndex.data(Qt::UserRole).toInt())
    , client(client)
    , headerState(HDR_INCOMPLETE)
{
    if (client) {
        connect(client, SIGNAL(readyRead()), SLOT(clientReadyRead()));
        connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
        connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(clientError()));
    }
}


QString ConnectionData::idText()
{
    if (hasFullHeader()) {
        return parsedHeader.simpleId;
    }
    else {
        return tr("*N/A*");
    }
}


int ConnectionData::extractHeaderBytes(QByteArray &data)
{
    if (headerState != HDR_INCOMPLETE) return 0;
    // add only to incomplete header

    int originalHeaderSize = headerBytes.size(); // needed in case there's no header at all
    int ind = data.indexOf('\n'); // first \n is end of header

    if (ind == -1) {
        // not complete header line, add all
        headerBytes += data;
        if (!testInvalidHeaderMagic()) return data.size();
        // else header is invalid
    }
    else {
        // got full header line
        int dataOffset = ind+1;
        headerBytes += data.left(dataOffset);
        qDebug() << "got full header" << headerBytes;
        if (!testInvalidHeaderMagic() && parseValidHeader()) {
            headerState = HDR_VALID;
            if (modelIndex.isValid()) {
                model->setData(modelIndex, idText());
            }
            return dataOffset;
        }
        // else header is invalid
    }

    // if code flow reaches here, it means there were no valid pipeit header
    qDebug() << "invalid header, assuming it is raw data" << headerBytes;
    headerState = HDR_IGNORED;

    // restore old header bytes to data
    data.prepend(headerBytes.left(originalHeaderSize));
    headerBytes.clear();

    parsedHeader.version = -1;
    parsedHeader.encoding.clear();
    parsedHeader.simpleId = "*RAW" + QByteArray::number(key) + '*';
    if (modelIndex.isValid()) {
        model->setData(modelIndex, idText());
    }
    return 0;
}


void ConnectionData::clientReadyRead()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);

    qDebug() << "got data from a client>" << idText();

    QByteArray data = client->readAll();
    int dataOffset = extractHeaderBytes(data);

    if (data.size() > dataOffset) {
        // there's actual data remaining
        //int originalSize = bytes.size();
        bytes.append(data.constData() + dataOffset, data.size() - dataOffset);
        //emit bytesReceived(bytes, originalSize + dataOffset);
        emit bytesReceived(key, data, dataOffset);
    }
}


void ConnectionData::clientDisconnected()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);
    client->deleteLater();
    client = 0;
    qDebug() << "client disconnected>" << idText();
    QString msg(tr("EOF"));
    eofMessages.append(msg);
    emit eofReceived(key, msg);
}


void ConnectionData::clientError()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);

    qDebug() << "client>" << idText() << "<error:" << client->errorString();
    if (client->error() != QLocalSocket::PeerClosedError) {
        QString msg(tr("ERROR: ") + client->errorString());
        eofMessages.append(msg);
        emit eofReceived(key, msg);
    }
}


bool ConnectionData::testInvalidHeaderMagic() {
    if (headerBytes.size() < HDR_MAGIC.size()) {
        // header is still shorter than magic string,
        // so test if magic string start matches to header so far
        return !HDR_MAGIC.startsWith(headerBytes);
    }
    else {
        // header is at least as long as magic string
        // so test if header starts with magic string
        return !headerBytes.startsWith(HDR_MAGIC);
    }

}


bool ConnectionData::parseValidHeader()
{
    // assumes headerBytes starts with the magic string
    int pos;
    bool ok;
    int startPos = HDR_MAGIC.size();

    // parse integer version
    pos = headerBytes.indexOf(' ', startPos);
    if (pos == -1) return false;
    parsedHeader.version = headerBytes.mid(startPos, pos-startPos).toInt(&ok);
    if (!ok || parsedHeader.version < 0) return false;
    startPos = pos + 1;

    switch (parsedHeader.version) {
    case 0:
        // parse encoding name
        pos = headerBytes.indexOf(' ', startPos);
        if (pos == -1) return false;
        parsedHeader.encoding = headerBytes.mid(startPos, pos-startPos);
        startPos = pos + 1;

        // rest of the line is simplified id, leave out only line feed at end
        parsedHeader.simpleId = headerBytes.mid(startPos, headerBytes.size() - startPos - 1);
        startPos = pos + 1;
        return true;
    default:
        // other versions not implemented yet
        return false;
    }
}


