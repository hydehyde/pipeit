// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QWeakPointer>
#include <QMap>
#include <QMultiMap>

class QSplitter;
class QLocalSocket;
class QPlainTextEdit;
class QLabel;
class QStandardItemModel;
class QModelIndex;

class ConnectionData;
class ViewWidget;
class ViewDocument;

class SessionWidget : public QWidget
{
    Q_OBJECT

    enum {
        VIEWCOUNT = 3, // number of viewer widgets to create
        LASTCOUNT = 3  // number of "Nth last" selection entries to keep
    };
public:
    explicit SessionWidget(QWidget *parent = 0);
    void addClient(QLocalSocket *client);

signals:

private slots:
    void distributeNewBytes(int key, const QByteArray &bytes, int offset);
    void distributeEofMessage(int key, const QString &eofMessage);
    void viewerSelection(); // operates on sender()
    void updateNthLastViews(int viewInd=0);

private: //methods
    int getRealKeyFromConnections(int selectedKey);
    void updateViewerDataSource(ViewWidget *view, int key);


private:
    int connKey;
    QMap<int, ConnectionData*> connections;
    QMultiMap<int, ViewDocument*> viewDocs;
    ViewDocument *noDataDocument;
    QStandardItemModel *connModel;
    QSplitter *outerSplitter;
    QSplitter *innerSplitter1;
    QSplitter *innerSplitter2;

    ViewWidget* views[VIEWCOUNT];
};

#endif // SESSION_H
