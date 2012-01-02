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
public:
    explicit ViewWidget(QAbstractItemModel *model = 0, QWidget *parent = 0);
    QPlainTextEdit *getViewer() { return viewer; }
    int getSelectedKey() { return selectedKey; }
    void setSelectedKey(int key);
    int getRealKey() { return realKey; }
    void setRealKey(int key) { realKey = key; }
    QByteArray getEncoding() { return "UTF-8"; }
    void setViewDocument(ViewDocument *doc);

signals:
    void viewerTargetSelected();
    void documentNeedsUpdate();

private slots:
    void selectionBoxChanged(int index);

private:
    int selectedKey;
    int realKey;
    QComboBox *selectionBox;
    QPlainTextEdit *viewer;

};

#endif // VIEWWIDGET_H
