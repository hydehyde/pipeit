#include "viewwidget.h"

#include <QtGui>

ViewWidget::ViewWidget(QWidget *parent) :
    QWidget(parent)
  , label(new QLabel(tr("*NEW*")))
  , viewer(new QPlainTextEdit)
{
    viewer->setReadOnly(true);
    viewer->setWordWrapMode(QTextOption::WrapAnywhere);

    setLayout(new QVBoxLayout);
    layout()->addWidget(label);
    layout()->addWidget(viewer);

}
