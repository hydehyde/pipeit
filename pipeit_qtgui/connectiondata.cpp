#include "connectiondata.h"

#include <QTextBrowser>
#include <QTabWidget>

ConnectionData::ConnectionData()
    : view(new QTextBrowser)
    , codec(QTextCodec::codecForName("UTF-8"))
    , codecState(new QTextCodec::ConverterState)
{
}


void ConnectionData::addHeaderBytes(QByteArray data)
{
    headerBytes += data;
}


void ConnectionData::addBytes(QByteArray data, unsigned offset)
{
    bytes += data.mid(offset);
    QString text = codec->toUnicode(data.constData() + offset, data.size() - offset);
    if (!view.isNull()) {
        QTextCursor tc = view.data()->textCursor();
        // using text cursor for insertion avoids affecting any user selection
        tc.movePosition(QTextCursor::End);
        tc.insertText(text);
    }
}


void ConnectionData::updateParentTab(QTabWidget *tabWidget)
{
    if (view.isNull()) return;
    int tabIndex = tabWidget->indexOf(view.data());
    if (tabIndex < 0) return;
    QString id = QString::fromUtf8(headerBytes).trimmed();
    tabWidget->setTabText(tabIndex, id);
}
