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

    void setViewer(QPlainTextEdit *newView);

    bool hasFullHeader() { return headerState != HDR_INCOMPLETE; }

    QString idText();

    int addHeaderBytes(QByteArray &data);
    void addBytes(QByteArray data, unsigned offset=0);

signals:
    void headerReceived(const QString &idText);

private slots:
    void clientReadyRead();
    void clientDisconnected();
    void clientError();

private:
    bool testInvalidHeaderMagic();
    bool parseValidHeader();

private:
    QLocalSocket *client;
    QByteArray headerBytes;
    enum { HDR_INCOMPLETE, HDR_VALID, HDR_IGNORED } headerState;
    struct {
        int version;
        QByteArray encoding;
        QByteArray simpleId;
    } parsedHeader;
    QByteArray bytes;
    QString text;
    QWeakPointer<QPlainTextEdit> viewer;
    QByteArray codecName;
    QTextCodec *codec;
    QTextDecoder *decoder;
};

#endif // CONNECTIONDATA_H
