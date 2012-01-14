// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

#include "viewwidget.h"

#include <QtGui>

#include "viewdocument.h"
#include "common.h"

// static
const QByteArray ViewWidget::DEFAULT_ENCODING("UTF-8");

ViewWidget::ViewWidget(QAbstractItemModel *model, QWidget *parent) :
    QWidget(parent)
  , selectedKey(0)
  , realKey(0)
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


ViewDocument *ViewWidget::getViewDocument()
{
    return qobject_cast<ViewDocument*>(viewer->document());
}


void ViewWidget::swapViewDocument(ViewDocument *&swapDoc, int &swapRealKey)
{
    Q_ASSERT(swapRealKey >= 0);
    int oldRealKey = getRealKey();
    ViewDocument *oldDoc = getViewDocument(); // NULL, if current doc isn't a ViewDocument
    Q_ASSERT(!(oldRealKey > 0 && !oldDoc)); // can't have both valid key and NULL oldDoc

    if (swapDoc == oldDoc) {
        // no change
        Q_ASSERT(oldRealKey == swapRealKey); // if docs match, keys should match too
        oldDoc = NULL, oldRealKey = 0;
    }
    else {
        setDocToViewer(swapDoc);
        this->realKey = swapRealKey;
        if (oldDoc && oldDoc->parent() == viewer) {
            // oldDoc is ownded by viewer, so it will be deleted by viewer
            Q_ASSERT(oldRealKey <= 0); // if doc is local, real key should be invalid
            oldDoc = NULL;
        }
    }
    // return old values, which may be 0
    swapDoc = oldDoc;
    swapRealKey = oldRealKey;
}


void ViewWidget::selectionBoxChanged(int index)
{
    selectedKey = selectionBox->itemData(index, Qt::UserRole).toInt();
    emit viewerTargetSelected();
}

// private
void ViewWidget::setDocToViewer(ViewDocument *doc)
{
    if(!doc) {
        doc = new ViewDocument(DEFAULT_ENCODING, viewer);
    }
    viewer->setDocument(doc);
    viewer->hide();
    viewer->show();
}

