// Copyright 2011 Ari Hyttinen
// Distributed under GNU General Public License version 3 or later

// Qt version of pipeit client tool
// Reference implementation for usage

#include <QCoreApplication>

#include <QLocalSocket>
#include <QFile>

#include <cstdio>


// robust write which does not fail on partial writes
bool writeToServer(QLocalSocket &sock, const QByteArray &data)
{
    int written = 0;
    forever {
        int thisWrite = sock.write(data.constData() + written, data.size() - written);
        if (thisWrite == -1) {
            fprintf(stderr, "Error writing to server socket: %s", qPrintable(sock.errorString()));
            return false;
        }

        if (!sock.waitForBytesWritten()) {
            fprintf(stderr, "Timeout writing for bytes written to server.");
            return false;
        }

        written += thisWrite;
        if (written == data.size()) {
            // all written
            return true;
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc > 1) {
        fprintf(stderr, "Command line arguments are not supported.");
        return 1;
    }

    QCoreApplication a(argc, argv);

    // open QFile for reading stdin
    QFile stdIn;
    if (!stdIn.open(0, QIODevice::ReadOnly)) {
        fprintf(stderr, "Failed to open stdin for reading: %s", qPrintable(stdIn.errorString()));
        return 1;
    }

    // connect localsocket to server
    QLocalSocket sock;
    sock.connectToServer("pipeit_gui_server", QIODevice::WriteOnly);
    if (!sock.waitForConnected()) {
        fprintf(stderr, "Failed to connect to server: %s", qPrintable(sock.errorString()));
        return 1;
    }

    // write identification line to server
    // id line is words separated by single space, empty words ok, format for version 0:
    // pipeit rawstream <encoding string> <identification string until newline>
    {
        // empty encoding means "unknown"
        QByteArray id("pipeit rawstream  pipeit pid ");

        id += QByteArray::number(QCoreApplication::applicationPid());
        id += '\n';
        if (!writeToServer(sock, id)) {
            return 1;
        }
    }

    // read stdin line-by-line and write to server
    forever {
        QByteArray line = stdIn.readLine();
        if (line.isEmpty()) {
            // end of input, return success
            return 0;
        }
        if (!writeToServer(sock, line)) {
            return 1;
        }
    }

}
