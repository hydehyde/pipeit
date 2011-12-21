// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "connectiondata.h"

#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLocalSocket>
#include <QTextCodec>
#include <QTextDecoder>

#include <QDebug>

ConnectionData::ConnectionData(QLocalSocket *client, QObject *parent)
    : QObject(parent)
    , client(client)
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


void ConnectionData::setView(QPlainTextEdit *newView)
{
    if (!newView) {
        view.clear();
    }
    else {
        newView->setPlainText(text);
        view = newView;
    }
}


int ConnectionData::addHeaderBytes(const QByteArray &data)
{
    int dataOffset;

    // add to header line
    int ind = data.indexOf('\n');
    if (ind == -1) {
        // not complete header line, add all
        headerBytes += data;
        dataOffset = data.size();
    }
    else {
        // got full header line
        dataOffset = ind+1;
        headerBytes += data.left(dataOffset);
        qDebug() << "...got client id" << idText();
        emit headerReceived(idText());
    }
    return dataOffset;
}


void ConnectionData::addBytes(QByteArray data, unsigned offset)
{
    bytes.append(data.constData() + offset, data.size() - offset);

    QString decodedText = decoder->toUnicode(data.constData() + offset, data.size() - offset);
    text += decodedText;

    if (!view.isNull()) {
        QTextCursor tc = view.data()->textCursor();
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
    if (!hasFullHeader()) {
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
    if (!view.isNull()) {
        view.data()->appendHtml(tr("<hr>EOF<hr>"));
    }
}


void ConnectionData::clientError()
{
    Q_ASSERT(client);
    Q_ASSERT(qobject_cast<QLocalSocket *>(sender()) == client);

    qDebug() << "client>" << idText() << "<error:" << client->errorString();
    if (client->error() != QLocalSocket::PeerClosedError) {
        if (!view.isNull()) {
            view.data()->appendHtml(tr("<hr>Client connection error:<br>%1<hr>").arg(client->errorString()));
        }
    }
}

