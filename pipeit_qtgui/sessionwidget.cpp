// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "sessionwidget.h"

#include <QtGui>
#include <QLocalSocket>

#include <connectiondata.h>
#include <viewwidget.h>

SessionWidget::SessionWidget(QWidget *parent) :
    QWidget(parent)
  , splitter(new QSplitter)
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(splitter);

}


void SessionWidget::addClient(QLocalSocket *client)
{
    ConnectionData *cd = new ConnectionData(client, this);
    ViewWidget *view = new ViewWidget();

    connect(cd, SIGNAL(headerReceived(QString)),
            view->getLabel(), SLOT(setText(QString)));

    cd->setViewer(view->getViewer());

    splitter->addWidget(view);

    connect(cd, SIGNAL(headerReceived(QString)), SLOT(updateConnId(QString)));
}



void SessionWidget::updateConnId(const QString &idText)
{
    qDebug() << "got new id" << idText;
}
