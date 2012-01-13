#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QComboBox;

class QAbstractItemModel;
class QModelIndex;


class ViewDocument;

class ViewWidget : public QWidget
{
    Q_OBJECT

    static const QByteArray DEFAULT_ENCODING;

public:
    explicit ViewWidget(QAbstractItemModel *model = 0, QWidget *parent = 0);
    QPlainTextEdit *getViewer() { return viewer; }
    int getSelectedKey() { return selectedKey; }
    void setSelectedKey(int key);
    int getRealKey() { return realKey; }
    QByteArray getEncoding() { return DEFAULT_ENCODING; }
    ViewDocument *getViewDocument();
    void swapViewDocument(ViewDocument *&newDoc, int &newRealKey);

signals:
    void viewerTargetSelected();
    void documentNeedsUpdate();

private slots:
    void selectionBoxChanged(int index);

private: //methods
    void setDocToViewer(ViewDocument *doc);

private:
    int selectedKey;
    int realKey;
    QComboBox *selectionBox;
    QPlainTextEdit *viewer;

};

#endif // VIEWWIDGET_H
