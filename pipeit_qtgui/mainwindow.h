// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>

class QLocalServer;
class QLocalSocket;

class QTextBrowser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct ClientStruct {
        QString id;
    };

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleConnection();
    void clientConnected();
    void clientReadyRead();
    void clientDisconnected();
    void clientError();

private:
    QLocalServer *server;
    QTextBrowser *view;

    QMap<QLocalSocket*, ClientStruct> clients;
};

#endif // MAINWINDOW_H
