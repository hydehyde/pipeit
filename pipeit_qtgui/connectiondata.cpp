// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "connectiondata.h"

#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLocalSocket>
#include <QTextCodec>
#include <QTextDecoder>

#include <QDebug>

static const QByteArray HDR_MAGIC("pipeit ");


ConnectionData::ConnectionData(QLocalSocket *client, QObject *parent)
    : QObject(parent)
    , client(client)
    , headerState(HDR_INCOMPLETE)
    , codecName("UTF-8")
    , codec(QTextCodec::codecForName(codecName))
    , decoder(codec->makeDecoder())
{
    if (client) {
        connect(client, SIGNAL(readyRead()), SLOT(clientReadyRead()));
        connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
        connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(clientError()));
    }
    Q_ASSERT(codec);
}


ConnectionData::~ConnectionData()
{
    delete decoder;
}


void ConnectionData::setViewer(QPlainTextEdit *newView)
{
    if (!newView) {
        viewer.clear();
    }
    else {
        newView->setPlainText(text);
        viewer = newView;
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


int ConnectionData::addHeaderBytes(QByteArray &data)
{
    if (headerState != HDR_INCOMPLETE) return 0;
    // add only to incomplete header

    int originalHeaderSize = headerBytes.size();
    int ind = data.indexOf('\n');

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
            emit headerReceived(idText());
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
    parsedHeader.simpleId = "*RAW*";
    return 0;
}


void ConnectionData::addBytes(QByteArray data, unsigned offset)
{
    bytes.append(data.constData() + offset, data.size() - offset);

    QString decodedText = decoder->toUnicode(data.constData() + offset, data.size() - offset);
    text += decodedText;

    if (!viewer.isNull()) {
        QTextCursor tc = viewer.data()->textCursor();
        // using text cursor for insertion avoids affecting any user selection
        tc.movePosition(QTextCursor::End);
        tc.insertText(decodedText);
    }
}


void ConnectionData::clientReadyRead()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);

    qDebug() << "got data from a client>" << idText();

    int dataOffset = 0;
    QByteArray data = client->readAll();
    if (headerState == HDR_INCOMPLETE) {
        dataOffset = addHeaderBytes(data);
    }
    if (data.size() > dataOffset) {
        // there's actual data remaining
        addBytes(data, dataOffset);
    }
}


void ConnectionData::clientDisconnected()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);
    client->deleteLater();
    client = 0;

    qDebug() << "client disconnected>" << idText();
    if (!viewer.isNull()) {
        viewer.data()->appendHtml(tr("<hr>EOF<hr>"));
    }
}


void ConnectionData::clientError()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);

    qDebug() << "client>" << idText() << "<error:" << client->errorString();
    if (client->error() != QLocalSocket::PeerClosedError) {
        if (!viewer.isNull()) {
            viewer.data()->appendHtml(tr("<hr>Client connection error:<br>%1<hr>").arg(client->errorString()));
        }
    }
}


bool ConnectionData::testInvalidHeaderMagic() {
    if (headerBytes.size() < HDR_MAGIC.size()) {
        return !HDR_MAGIC.startsWith(headerBytes);
    }
    else {
        return !headerBytes.startsWith(HDR_MAGIC);
    }

}


bool ConnectionData::parseValidHeader()
{
    int pos;
    bool ok;
    int startPos = HDR_MAGIC.size();


    // parse integer version
    pos = headerBytes.indexOf(' ', startPos);
    if (pos == -1) return false;
    parsedHeader.version = headerBytes.mid(startPos, pos-startPos).toInt(&ok);
    if (!ok || parsedHeader.version < 0) return false;
    startPos = pos + 1;

    // parse encoding name
    pos = headerBytes.indexOf(' ', startPos);
    if (pos == -1) return false;
    parsedHeader.encoding = headerBytes.mid(startPos, pos-startPos);
    startPos = pos + 1;

    // rest of the line is simplified id, leave out only line feed at end
    parsedHeader.simpleId = headerBytes.mid(startPos, headerBytes.size() - startPos - 1);
    startPos = pos + 1;

    return true;
}


