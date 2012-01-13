#include "viewdocument.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextCodec>
#include <QTextDecoder>
#include <QPlainTextDocumentLayout>


ViewDocument::ViewDocument(const QByteArray &encoding, QObject *parent) :
    QTextDocument(parent)
  , codecName(encoding)
  , decoder(NULL)
{
    // TODO: check if this does this twice
    setDocumentLayout(new QPlainTextDocumentLayout(this));
}

void ViewDocument::addBytes(const QByteArray &bytes, int offset)
{
    if (bytes.size() <= offset) return;

    QString decodedText = decode(bytes, offset);

    QTextCursor tc(this);
    tc.movePosition(QTextCursor::End);
    tc.insertText(decodedText, QTextCharFormat());
}


void ViewDocument::addEofMessage(const QString &eofMessage)
{
    QTextCharFormat format;
    format.setForeground(Qt::blue);
    format.setFontItalic(true);

    QTextCursor tc(this);
    tc.movePosition(QTextCursor::End);
    tc.insertText(eofMessage, format);
}


void ViewDocument::setBytes(const QByteArray &bytes, const QString &eofMessage, int offset)
{
    clear();
    addBytes(bytes, offset);
    if (!eofMessage.isEmpty()) {
        addEofMessage(eofMessage);
    }
}

QString ViewDocument::decode(const QByteArray &bytes, int offset)
{
    // no decoder will cause tr() to be used in converting 8-bit chars to QString
    QString ret;
    if (bytes.size() > offset) {
        if (decoder == NULL && !codecName.isEmpty()) {
            QTextCodec *codec = QTextCodec::codecForName(codecName);
            if (codec) {
                decoder = codec->makeDecoder();
            }
        }

        if (decoder) {
            ret = decoder->toUnicode(bytes.constData() + offset, bytes.size() - offset);
        }
        else {
            ret = tr(bytes.constData());
        }
    }
    return ret;
}

