#include "viewwidget.h"

#include <QtGui>
#include "common.h"

ViewWidget::ViewWidget(QAbstractItemModel *model, QWidget *parent) :
    QWidget(parent)
  , selectedKey(0)
  , realKey(0)
  , selectionBox(new QComboBox)
  , viewer(new QPlainTextEdit)
  , codecName("UTF-8")
  , codec(QTextCodec::codecForName(codecName))
  , decoder(codec->makeDecoder())
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


void ViewWidget::addBytes(const QByteArray &bytes, int offset)
{
    QString decodedText = decoder->toUnicode(bytes.constData() + offset, bytes.size() - offset);

    QTextCursor tc = viewer->textCursor();
    // using text cursor for insertion avoids affecting any user selection
    tc.movePosition(QTextCursor::End);
    tc.insertText(decodedText);
}


void ViewWidget::addEofMessage(const QString &eofMessage)
{
    QTextCharFormat format;
    format.setForeground(Qt::blue);
    format.setFontItalic(true);

    QTextCursor tc = viewer->textCursor();
    // using text cursor for insertion avoids affecting any user selection
    tc.movePosition(QTextCursor::End);
    tc.insertText(eofMessage, format);
}


void ViewWidget::setBytes(const QByteArray &bytes, const QString &eofMessage, int offset)
{
    QString decodedText = decoder->toUnicode(bytes.constData() + offset, bytes.size() - offset);
    viewer->clear();
    viewer->setPlainText(decodedText);
    if (!eofMessage.isEmpty()) {
        addEofMessage(eofMessage);
    }
}


void ViewWidget::selectionBoxChanged(int index)
{
    int key = selectionBox->itemData(index, Qt::UserRole).toInt();
    emit viewerTargetSelected(key, QWeakPointer<ViewWidget>(this));
}

