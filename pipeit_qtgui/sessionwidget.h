// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include <QWidget>
#include <QWeakPointer>
#include <QMap>

class QSplitter;
class QLocalSocket;
class QPlainTextEdit;
class QLabel;

class ConnectionData;
class ViewWidget;

class SessionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SessionWidget(QWidget *parent = 0);
    void addClient(QLocalSocket *client);

signals:

private slots:
    void updateConnId(const QString&);

private:
    QSplitter *splitter;
};

#endif // SESSION_H
