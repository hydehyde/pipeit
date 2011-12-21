// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "mainwindow.h"


#include <QtGui>
#include <QLocalServer>
#include <QLocalSocket>

#include "sessionwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , server(new QLocalServer(this))
    , tabWidget(new QTabWidget)
    , defaultSession(new SessionWidget)
{
    setCentralWidget(tabWidget);

    QString sockName("pipeit_gui_server");

    // to do: add this as button in the error message mode
    QLocalServer::removeServer(sockName);

    if (!server->listen(sockName)) {
        QWidget *w = new QWidget;
        w->setLayout(new QVBoxLayout());
        w->layout()->addWidget(
                    new QLabel(
                        tr("Listening on local socket '%1' failed: %2")
                        .arg(sockName).arg(server->errorString())));
        QPushButton *b = new QPushButton(tr("Quit"));
        w->layout()->addWidget(b);

        connect(b, SIGNAL(clicked()), qApp, SLOT(quit()));
        tabWidget->addTab(w, tr("*ERROR*"));
    }
    else {
        tabWidget->addTab(defaultSession, "DEFAULT");
        connect(server, SIGNAL(newConnection()), SLOT(handleConnection()));
    }
}

MainWindow::~MainWindow()
{

}

void MainWindow::handleConnection()
{
    forever {
        QLocalSocket *client = server->nextPendingConnection();
        if (!client) break;

        qDebug() << "A new client connected";
        defaultSession->addClient(client);
    }
}



