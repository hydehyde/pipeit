// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

// This QTextDocument subclass is responsible for
// decoding raw data into unicode text,
// and it adds slots to receive new raw data and set contents by raw data.

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
