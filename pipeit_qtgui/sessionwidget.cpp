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
    // add bytes to all free docs associated with this connection
    foreach(ViewDocument * const doc, freeViewDocs.values(key)) {
        doc->addBytes(bytes, offset);
    }
    // check all views for docs associated with this connection
    for(int ii = 0 ; ii < VIEWCOUNT ; ++ii) {
        int realKey = views[ii]->getRealKey();
        if (realKey == key) {
            views[ii]->getViewDocument()->addBytes(bytes, offset);
        }
    }
}


void SessionWidget::distributeEofMessage(int key, const QString &eofMessage)
{
    // add message to all free docs associated with this connection
    foreach(ViewDocument * const doc, freeViewDocs.values(key)) {
        doc->addEofMessage(eofMessage);
    }
    // check all views for docs associated with this connection
    for(int ii = 0 ; ii < VIEWCOUNT ; ++ii) {
        int realKey = views[ii]->getRealKey();
        if (realKey == key) {
            views[ii]->getViewDocument()->addEofMessage(eofMessage);
        }
    }
}


void SessionWidget::viewerSelection()
{
    // justification for using sender(): this slot method operates directly on sender anyway
    ViewWidget *view = qobject_cast<ViewWidget*>(sender());
    if (!view) return;

    updateViewerDataSource(view, view->getSelectedKey());
}


void SessionWidget::updateNthLastViews(int selectedKey)
{
#if 1
    for(int ii=0; ii<VIEWCOUNT; ++ii) {
        if(views[ii]->getSelectedKey() == selectedKey) {
            updateViewerDataSource(views[ii], selectedKey);
        }
    }
    if (-selectedKey < LASTCOUNT) {
        QMetaObject::invokeMethod(this, "updateNthLastViews", Qt::QueuedConnection, Q_ARG(int, selectedKey-1));
    }
#else
    // recursive method: calls itself with next viewInd, until all are handled
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
#endif
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
        realKey = 0;
    }

    return realKey;
}


void SessionWidget::updateViewerDataSource(ViewWidget *view, int key)
{
    int realKey = getRealKeyFromConnections(key);
    ViewDocument *setDoc = NULL;

    QByteArray encoding = view->getEncoding();
    bool changeDoc = false;
    if (view->getRealKey() != realKey) {
        // real key changed, so document must change always
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
            foreach(ViewDocument * const doc, freeViewDocs.values(realKey)) {
                if (doc->getEncoding() == encoding) {
                    setDoc = doc;
                    int removedCount = freeViewDocs.remove(realKey, doc);
                    Q_ASSERT(removedCount == 1); // exactly 1 matching key-value pair should exist
                    break;
                }
            }

            // if no document found, create new
            if (setDoc == NULL) {
                const ConnectionData *cd = connections[realKey];
                setDoc = new ViewDocument(encoding, this);
                setDoc->setBytes(cd->getBytes(), cd->getEofMessages().join("\n"));
            }
        }
        // else no real doc to show
    }

    if (setDoc == NULL) {
        Q_ASSERT(realKey == 0);
        setDoc = noDataDocument;
    }

    view->swapViewDocument(setDoc, realKey); // swap found/created doc with old doc in viewer
    if (realKey > 0) {
        Q_ASSERT(setDoc && setDoc != noDataDocument); // >0 realKey must match with real doc
        freeViewDocs.insertMulti(realKey, setDoc);
    }
}
