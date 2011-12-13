// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "mainwindow.h"


#include <QtGui>
#include <QLocalServer>
#include <QLocalSocket>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , server(new QLocalServer(this))
    , tabWidget(new QTabWidget)
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
        connect(client, SIGNAL(readyRead()), SLOT(clientReadyRead()));
        connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
        connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(clientError()));

        qDebug() << "A client connected";
        ConnectionData clientData;
        tabWidget->addTab(clientData.view.data(), tr("*NEW*"));
        clients[client] = clientData;
    }
}

void MainWindow::clientReadyRead()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    qDebug() << "got data from a client";
    Q_ASSERT(clients.contains(client));
    ConnectionData &clientData = clients[client];

    QByteArray data = client->readAll();
    int dataOffset = 0;
    if (!clientData.hasFullHeader()) {
        // hello line not fully received yet, add to it
        int ind = data.indexOf('\n');
        if (ind == -1) {
            // still no full hello line received
            clientData.addHeaderBytes(data);
            data.clear();
        }
        else {
            // full hello received
            dataOffset = ind+1;
            clientData.addHeaderBytes(data.left(dataOffset));
            qDebug() << "...got client id" << clientData.idText();
        }
        clientData.updateParentTab(tabWidget);
    }
    if (data.size() > dataOffset) {
        // actual data remaining
        clientData.addBytes(data, dataOffset);
    }
}

void MainWindow::clientDisconnected()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    Q_ASSERT(clients.contains(client));
    ConnectionData &clientData = clients[client];

    qDebug() << "client disconnected" << clients[client].idText();
    clientData.view.data()->append(tr("="));
    clients.remove(client);
}

void MainWindow::clientError()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    Q_ASSERT(clients.contains(client));
    ConnectionData &clientData = clients[client];

    qDebug() << "client" << clientData.idText() << "error:" << client->errorString();
    if (client->error() != QLocalSocket::PeerClosedError) {
        clientData.view.data()->append(tr("Client connection error: %1").arg(client->errorString()));
    }
}

