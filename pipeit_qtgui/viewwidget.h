#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QComboBox;

class QTextCodec;
class QTextDecoder;

class QAbstractItemModel;
class QModelIndex;

class ViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWidget(QAbstractItemModel *model = 0, QWidget *parent = 0);
    QPlainTextEdit *getViewer() { return viewer; }
    int getSelectedKey() { return selectedKey; }
    void setSelectedKey(int key);
    int getRealKey() { return realKey; }
    void setRealKey(int key) { realKey = key; }

signals:
    void viewerTargetSelected(int newKey, QWeakPointer<ViewWidget>);

public slots:
    void addBytes(const QByteArray &bytes, int offset=0);
    void addEofMessage(const QString &eofMessage);
    void setBytes(const QByteArray &bytes, const QString &eofMessage=QString(), int offset=0);

private slots:
    void selectionBoxChanged(int index);

private:
    int selectedKey;
    int realKey;
    QComboBox *selectionBox;
    QPlainTextEdit *viewer;

    QByteArray codecName;
    QTextCodec *codec;
    QTextDecoder *decoder;
};

#endif // VIEWWIDGET_H
