// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "sessionwidget.h"

#include <QtGui>
#include <QLocalSocket>

#include <connectiondata.h>

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

    QWidget *w = new QWidget;
    w->setLayout(new QVBoxLayout);

    labels.append(new QLabel(tr("*NEW*")));
    w->layout()->addWidget(labels.last());
    connect(cd, SIGNAL(headerReceived(QString)),
            labels.last(), SLOT(setText(QString)));

    views.append(new QPlainTextEdit);
    views.last()->setReadOnly(true);
    views.last()->setWordWrapMode(QTextOption::WrapAnywhere);
    w->layout()->addWidget(views.last());
    cd->setView(views.last());

    splitter->addWidget(w);

    connect(cd, SIGNAL(headerReceived(QString)), SLOT(updateConnId(QString)));
}



void SessionWidget::updateConnId(const QString &idText)
{
    qDebug() << "got new id" << idText;
}
