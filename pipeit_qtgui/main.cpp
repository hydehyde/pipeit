// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
