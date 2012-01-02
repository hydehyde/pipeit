#include "viewwidget.h"

#include <QtGui>

#include "viewdocument.h"
#include "common.h"


ViewWidget::ViewWidget(QAbstractItemModel *model, QWidget *parent) :
    QWidget(parent)
  , selectedKey(0)
  , realKey(-1) // value -1 to indicate key has not been set, not even to 0 (invalid)
  , selectionBox(new QComboBox)
  , viewer(new QPlainTextEdit)
{
    if (model) {
        selectionBox->setModel(model);
    }

    viewer->setReadOnly(true);
    viewer->setWordWrapMode(QTextOption::WrapAnywhere);

    QVBoxLayout *tmpTopLayout = new QVBoxLayout;
    setLayout(tmpTopLayout);

    {
        QHBoxLayout *tmpSubLayout = new QHBoxLayout;
        tmpSubLayout->addWidget(selectionBox);
        tmpTopLayout->addLayout(tmpSubLayout);
    }

    tmpTopLayout->addWidget(viewer);

    connect(selectionBox, SIGNAL(currentIndexChanged(int)), SLOT(selectionBoxChanged(int)));
}


void ViewWidget::setSelectedKey(int newKey)
{
    selectedKey = newKey;
    for(int ii=0; ii < selectionBox->count(); ++ii) {
        if (selectionBox->itemData(ii, Qt::UserRole).toInt() == newKey) {
            selectionBox->setCurrentIndex(ii);
            break;
        }
    }
}



void ViewWidget::setViewDocument(ViewDocument *doc)
{
    viewer->setDocument(doc);

    // it seems hide+show is needed to force QPlainTextEdit to react to document change,
    // update or repaint don't seem to work (tested: Ubuntu 10.04, Qt 4.6.2)
    viewer->hide();
    viewer->show();
}



void ViewWidget::selectionBoxChanged(int index)
{
    selectedKey = selectionBox->itemData(index, Qt::UserRole).toInt();
    emit viewerTargetSelected();
}

