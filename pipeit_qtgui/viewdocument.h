#ifndef VIEWDOCUMENT_H
#define VIEWDOCUMENT_H

#include <QTextDocument>

class QTextCodec;
class QTextDecoder;

class ViewDocument : public QTextDocument
{
    Q_OBJECT
public:
    explicit ViewDocument(const QByteArray &encoding, QObject *parent = 0);

signals:

public slots:
    void addBytes(const QByteArray &bytes, int offset=0);
    void addEofMessage(const QString &eofMessage);
    void setBytes(const QByteArray &bytes, const QString &eofMessage=QString(), int offset=0);
    QByteArray getEncoding() { return codecName; }

private:
    QString decode(const QByteArray &bytes, int offset=0);

private:
    QByteArray codecName;
    QTextDecoder *decoder;
};

#endif // VIEWDOCUMENT_H
