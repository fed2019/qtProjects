#include "mainhandler.h"
#include <QDebug>

MainHandler::MainHandler(QObject *parent) : QObject(parent)
{

}

MainHandler::~MainHandler()
{
     disconnect();
     qDebug()<<"~MainHandler";
}

MainHandler *MainHandler::getInstance()
{
    static MainHandler instance;
    return &instance;
}

void MainHandler::runOnMainThread(QObject *obj,std::function<void ()> func)
{
    //mMutex.lock();//防止连接多个lambda函数，阻塞线程!
    connect(this, &MainHandler::run,obj ,[&, func]() {
        func();
        disconnect();
        //mMutex.unlock();
    },Qt::QueuedConnection);
    emit run();
}
