#include "mainwin.h"
#include "common/log.h"
#include <QApplication>
#include <QDir>
#include <QException>

int main(int argc, char *argv[])
{

    QT_LOG::logInit(QString(argv[0]).split(QDir::separator()).last().remove(".exe") + ".log",1);
    QApplication a(argc, argv);
    MainWin w;
    w.show();

    return a.exec();
}
