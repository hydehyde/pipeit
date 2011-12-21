// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QSharedPointer>

#include "connectiondata.h"

class QLocalServer;
class QLocalSocket;
class QTabWidget;
class SessionWidget;
class ConnectionData;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleConnection();

private:
    QLocalServer *server;
    QTabWidget *tabWidget;

    QMap<QString, SessionWidget*> sessions;
    SessionWidget *defaultSession;

};

#endif // MAINWINDOW_H
