// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "sessionwidget.h"

#include <QtGui>
#include <QLocalSocket>
#include <QStandardItemModel>

#include <connectiondata.h>
#include <viewwidget.h>

#include <common.h>

SessionWidget::SessionWidget(QWidget *parent) :
    QWidget(parent)
  , connKey(0)
  , connModel(new QStandardItemModel(this))
  , outerSplitter(new QSplitter(Qt::Horizontal))
  , innerSplitter1(new QSplitter(Qt::Vertical))
  , innerSplitter2(new QSplitter(Qt::Vertical))
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(outerSplitter);
    outerSplitter->addWidget(innerSplitter1);
    outerSplitter->addWidget(innerSplitter2);

    {
        QStandardItem *item;

        item = new QStandardItem(tr("*Latest*"));
        item->setData(-1, Qt::UserRole);
        connModel->appendRow(item);

        item = new QStandardItem(tr("*2nd last*"));
        item->setData(-2, Qt::UserRole);
        connModel->appendRow(item);

        item = new QStandardItem(tr("*3rd last*"));
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
        }
        addSplitter->addWidget(views[ii]);
    }
}


void SessionWidget::addClient(QLocalSocket *client)
{
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
    // go through views and add received data to all which are showing the connection
    for(int ii = 0 ; ii < VIEWCOUNT; ++ii) {
        if (views[ii]->getRealKey() == key) {
            // real key of this view matches received data
            views[ii]->addBytes(bytes, offset);
        }
    }
}


void SessionWidget::distributeEofMessage(int key, const QString &eofMessage)
{
    // go through views and add received data to all which are showing the connection
    for(int ii = 0 ; ii < VIEWCOUNT; ++ii) {
        if (views[ii]->getRealKey() == key) {
            // real key of this view matches received data
            views[ii]->addEofMessage(eofMessage);
        }
    }
}


void SessionWidget::updateNthLastViews(int viewInd)
{
    Q_ASSERT(viewInd >= 0 && viewInd < VIEWCOUNT);

    int connKey;

    // first try to find a view which needs to be updated
    forever {
        if ( viewInd >= VIEWCOUNT) return; // no more views to update, return
        connKey = views[viewInd]->getSelectedKey();
        if (connKey < 0) break; // found a view that needs updating, break to do it
        ++viewInd; // test next view
    }

    // then update the found view
    {
        int realKey = getRealKeyFromConnections(connKey);
        if (views[viewInd]->getRealKey() != realKey) {
            views[viewInd]->setRealKey(realKey);
            if (realKey > 0) {
                const ConnectionData *cd = connections[realKey];
                views[viewInd]->setBytes(cd->getBytes(), cd->getEofMessages().join("\n"));
            }
            else {
                views[viewInd]->setBytes(QByteArray(), tr("NO DATA"));
            }
        }
    }
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


