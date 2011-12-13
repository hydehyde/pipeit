#ifndef CONNECTIONDATA_H
#define CONNECTIONDATA_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QString>
#include <QByteArray>

#include <QTextCodec>

class QTextBrowser;
class QTabWidget;

class ConnectionData
{
public:
    QByteArray headerBytes;
    QByteArray bytes;
    QWeakPointer<QTextBrowser> view;
    QTextCodec *codec;
    QSharedPointer<QTextCodec::ConverterState> codecState;

    ConnectionData();

    bool hasFullHeader() { return headerBytes.endsWith('\n'); }

    QString idText() { return headerBytes; }

    void addHeaderBytes(QByteArray data);
    void addBytes(QByteArray data, unsigned offset=0);
    void updateParentTab(QTabWidget *tabWidget);
};

#endif // CONNECTIONDATA_H
