// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef CONNECTIONDATA_H
#define CONNECTIONDATA_H

#include <QWeakPointer>
#include <QString>
#include <QByteArray>
#include <QStringList>

#include <QPersistentModelIndex>

class QPlainTextEdit;
class QTabWidget;
class QLocalSocket;

class ConnectionData : public QObject
{
    Q_OBJECT

public:

    ConnectionData(QAbstractItemModel *model, const QPersistentModelIndex &modelIndex, QLocalSocket *client, QObject *parent = 0);

    bool hasFullHeader() const { return headerState != HDR_INCOMPLETE; }
    QByteArray getBytes() const { return bytes; }
    QStringList getEofMessages() const { return eofMessages; }

    QString idText();

    int extractHeaderBytes(QByteArray &data);

signals:
    void bytesReceived(int key, const QByteArray &bytes, int offset);
    void eofReceived(int key, const QString &eofMessage);

private slots:
    void clientReadyRead();
    void clientDisconnected();
    void clientError();

private:
    bool testInvalidHeaderMagic();
    bool parseValidHeader();

private:
    QAbstractItemModel *model;
    QPersistentModelIndex modelIndex;
    int key;
    QLocalSocket *client;
    QByteArray headerBytes;
    enum { HDR_INCOMPLETE, HDR_VALID, HDR_IGNORED } headerState;
    struct {
        int version;
        QByteArray encoding;
        QByteArray simpleId;
    } parsedHeader;
    QByteArray bytes;
    QStringList eofMessages;
};

#endif // CONNECTIONDATA_H
