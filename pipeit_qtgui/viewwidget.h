#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;


class ViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWidget(QWidget *parent = 0);
    QLabel *getLabel() { return label; }
    QPlainTextEdit *getViewer() { return viewer; }

signals:

public slots:

private:
    QLabel *label;
    QPlainTextEdit *viewer;

};

#endif // VIEWWIDGET_H
