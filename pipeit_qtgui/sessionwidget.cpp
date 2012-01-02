// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "sessionwidget.h"

#include <QtGui>
#include <QLocalSocket>
#include <QStandardItemModel>

#include <connectiondata.h>
#include <viewwidget.h>
#include <viewdocument.h>

#include <common.h>

SessionWidget::SessionWidget(QWidget *parent) :
    QWidget(parent)
  , connKey(0)
  , noDataDocument(new ViewDocument(QByteArray(), this))
  , connModel(new QStandardItemModel(this))
  , outerSplitter(new QSplitter(Qt::Horizontal))
  , innerSplitter1(new QSplitter(Qt::Vertical))
  , innerSplitter2(new QSplitter(Qt::Vertical))
{
    noDataDocument->setBytes(QByteArray(), tr("NO DATA"));

    setLayout(new QVBoxLayout);
    layout()->addWidget(outerSplitter);
    outerSplitter->addWidget(innerSplitter1);
    outerSplitter->addWidget(innerSplitter2);

    {
        QStandardItem *item;

        item = new QStandardItem(tr("-- Previous"));
        item->setData(-1, Qt::UserRole);
        connModel->appendRow(item);

        item = new QStandardItem(tr("-- 2nd last"));
        item->setData(-2, Qt::UserRole);
        connModel->appendRow(item);

        item = new QStandardItem(tr("-- 3rd last"));
        item->setData(-3, Qt::UserRole);
        connModel->appendRow(item);

        Q_ASSERT(LASTCOUNT == 3);

        //        item = new QStandardItem(tr("*Empty*"));
        //        item->setData(0, Qt::UserRole);
        //        connModel->appendRow(item);
    }

    QSplitter *addSplitter  = innerSplitter1;
    int switchAt = VIEWCOUNT/2;
    for (int ii=0; ii<VIEWCOUNT; ++ii) {
        if (ii == switchAt) addSplitter = innerSplitter2;
        views[ii] = new ViewWidget(connModel);
        if (ii < LASTCOUNT) {
            int selectedKey = -(ii+1);
            views[ii]->setSelectedKey( selectedKey );
            updateViewerDataSource(views[ii], selectedKey);
        }
        connect(views[ii],
                SIGNAL(viewerTargetSelected()),
                SLOT(viewerSelection()));

        addSplitter->addWidget(views[ii]);
    }
}


void SessionWidget::addClient(QLocalSocket *client)
{
    // do a nice sanity check
    if(connKey == INT_MAX || connKey < 0) {
        qCritical("Internal connection id overflow");
        client->disconnectFromServer();
        client->deleteLater();
        return;
    }

    ++connKey;

    QStandardItem *item = new QStandardItem(tr("*new%1*").arg(connKey));
    item->setData(connKey, Qt::UserRole);
    connModel->insertRow(LASTCOUNT, item);

    ConnectionData *cd = new ConnectionData(
                connModel, QPersistentModelIndex(connModel->indexFromItem(item)), client, this);
    connections[connKey] = cd;
    updateNthLastViews();

    connect(cd,
            SIGNAL(bytesReceived(int,QByteArray,int)),
            SLOT(distributeNewBytes(int,QByteArray,int)));

    connect(cd,
            SIGNAL(eofReceived(int,QString)),
            SLOT(distributeEofMessage(int,QString)));
}


void SessionWidget::distributeNewBytes(int key, const QByteArray &bytes, int offset)
{
    // add bytes to all docs associated with this connection
    foreach(ViewDocument * const doc, viewDocs.values(key)) {
        doc->addBytes(bytes, offset);
    }
}


void SessionWidget::distributeEofMessage(int key, const QString &eofMessage)
{
    // add message to all docs associated with this connection
    foreach(ViewDocument * const doc, viewDocs.values(key)) {
        doc->addEofMessage(eofMessage);
    }
}


void SessionWidget::viewerSelection()
{
    // justification for using sender(): this slot method operates directly on sender anyway
    ViewWidget *view = qobject_cast<ViewWidget*>(sender());
    if (!view) return;

    updateViewerDataSource(view, view->getSelectedKey());
}


void SessionWidget::updateNthLastViews(int viewInd)
{
    Q_ASSERT(viewInd >= 0 && viewInd < VIEWCOUNT);

    int key;

    // first try to find a view which needs to be updated
    forever {
        if ( viewInd >= VIEWCOUNT) return; // no more views to update, return
        key = views[viewInd]->getSelectedKey();
        if (key < 0) break; // found a view that needs updating, break to do it
        ++viewInd; // test next view
    }

    // then update the found view
    updateViewerDataSource(views[viewInd], key);

    // then try to find next view for update, to avoid queuing method call when not needed
    do {
        ++viewInd; // test next view
        if ( viewInd >= VIEWCOUNT) return; // no more views to update, return
    } while (views[viewInd]->getSelectedKey() >= 0);

    // finally queue updating next view
    QMetaObject::invokeMethod(this, "updateNthLastViews", Qt::QueuedConnection, Q_ARG(int, viewInd));
}




int SessionWidget::getRealKeyFromConnections(int selectedKey)
{
    int realKey;
    if (selectedKey < 0) {
        // negative means -Nth last connection, need to calculate real row to get real key
        int row = LASTCOUNT + (-selectedKey - 1);
        if (row < connModel->rowCount()) {
            realKey = connModel->data(connModel->index(row, 0), Qt::UserRole).toInt();
        }
        else {
            realKey = 0;
        }
    }
    else {
        realKey = selectedKey;
    }

    // make sure the key exists
    if (!connections.contains(realKey)) {
        qDebug() << MARK << "called with key" << selectedKey << "... got non-existent key" << realKey;
        realKey = 0;
    }

    return realKey;
}


void SessionWidget::updateViewerDataSource(ViewWidget *view, int key)
{
    int realKey = getRealKeyFromConnections(key);
    ViewDocument *foundDoc = NULL;

    QByteArray encoding = view->getEncoding();
    bool changeDoc = false;
    if (view->getRealKey() != realKey) {
        // real key changed, so document must change always
        view->setRealKey(realKey);
        changeDoc = true;
    }
    else if (realKey > 0) {
        // real connection, but change in connection: change only if there's encoding change
        changeDoc = false; // TODO: implement detection of encoding changes
    }
    // else no key change, and no real data for the key, do nothing

    if (changeDoc) {
        if (realKey > 0) {
            // find if document for this key and encoding already exists
            foreach(ViewDocument * const doc, viewDocs.values(realKey)) {
                if (doc->getEncoding() == encoding) {
                    foundDoc = doc;
                    break;
                }
            }

            // if no document found, create new
            if (foundDoc == NULL) {
                const ConnectionData *cd = connections[realKey];
                foundDoc = new ViewDocument(encoding, this);
                foundDoc->setBytes(cd->getBytes(), cd->getEofMessages().join("\n"));
                viewDocs.insertMulti(realKey, foundDoc);
            }
        }
        else {
            // change doc to internally created informative no-data document
            foundDoc = noDataDocument;
        }
        Q_ASSERT(foundDoc);
        view->setViewDocument(foundDoc);
    }
}
