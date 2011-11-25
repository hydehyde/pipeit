// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "mainwindow.h"


#include <QtGui>
#include <QLocalServer>
#include <QLocalSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , server(new QLocalServer(this))
    , view(0)
{
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
        setCentralWidget(w);
    }
    else {
        view = new QTextBrowser();
        setCentralWidget(view);

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
        connect(client, SIGNAL(connected()), SLOT(clientConnected()));
        connect(client, SIGNAL(disconnected()), SLOT(clientDisconnected()));
        connect(client, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(clientError()));
    }
}

void MainWindow::clientConnected()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    qDebug() << "A client connected";
    ClientStruct clientData;
    clients[client] = clientData;
}

void MainWindow::clientReadyRead()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    qDebug() << "Reading data from A client";
    QByteArray data = client->readAll();
    if (!clients[client].id.endsWith('\n')) {
        int ind = data.indexOf('\n');
        if (ind == -1) ind = data.size();
        clients[client].id += data.left(ind+1);
        data = data.mid(ind+1);
        qDebug() << "client id now" << clients[client].id;
    }
    if (!data.isEmpty()) {
        QList<QByteArray> lines = data.split('\n');
        foreach(const QByteArray &line, lines) {
            view->append(QString::fromLatin1(line));
        }
    }
}

void MainWindow::clientDisconnected()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    qDebug() << "client disconnected" << clients[client].id;
    view->append(tr("="));
    clients.remove(client);
}

void MainWindow::clientError()
{
    QLocalSocket *client = qobject_cast<QLocalSocket *>(sender());
    if (!client) return;
    qDebug() << "client" << clients[client].id << "error:" << client->errorString();
    if (client->error() != QLocalSocket::PeerClosedError) {
        view->append(tr("Client connection error: %1").arg(client->errorString()));
    }
}

