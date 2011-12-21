// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef CONNECTIONDATA_H
#define CONNECTIONDATA_H

#include <QWeakPointer>
#include <QString>
#include <QByteArray>

class QTextCodec;
class QTextDecoder;

class QPlainTextEdit;
class QTabWidget;
class QLocalSocket;

class ConnectionData : public QObject
{
    Q_OBJECT

public:

    ConnectionData(QLocalSocket *client, QObject *parent = 0);
    ~ConnectionData();

    void setView(QPlainTextEdit *newView);

    bool hasFullHeader() { return headerBytes.endsWith('\n'); }

    QString idText() { return headerBytes; }

    int addHeaderBytes(const QByteArray &data);
    void addBytes(QByteArray data, unsigned offset=0);

signals:
    void headerReceived(const QString &idText);

private slots:
    void clientReadyRead();
    void clientDisconnected();
    void clientError();

private:
    QLocalSocket *client;
    QByteArray headerBytes;
    QByteArray bytes;
    QString text;
    QWeakPointer<QPlainTextEdit> view;
    QByteArray codecName;
    QTextCodec *codec;
    QTextDecoder *decoder;
};

#endif // CONNECTIONDATA_H
