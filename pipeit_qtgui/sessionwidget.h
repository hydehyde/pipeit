// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QWeakPointer>
#include <QMap>

class QSplitter;
class QLocalSocket;
class QPlainTextEdit;
class QLabel;
class QStandardItemModel;
class QModelIndex;

class ConnectionData;
class ViewWidget;

class SessionWidget : public QWidget
{
    Q_OBJECT

    enum {VIEWCOUNT = 3, LASTCOUNT = 3};
public:
    explicit SessionWidget(QWidget *parent = 0);
    void addClient(QLocalSocket *client);

signals:

private slots:
    void distributeNewBytes(int key, const QByteArray &bytes, int offset);
    void distributeEofMessage(int key, const QString &eofMessage);

private slots:
    void updateNthLastViews(int viewInd=0);
    int getRealKeyFromConnections(int viewInd);

private:
    int connKey;
    QMap<int, ConnectionData*> connections;
    QStandardItemModel *connModel;
    QSplitter *outerSplitter;
    QSplitter *innerSplitter1;
    QSplitter *innerSplitter2;

    ViewWidget* views[VIEWCOUNT];
};

#endif // SESSION_H
