#ifndef SERIALHELPER_H
#define SERIALHELPER_H
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息
#include <QStringList>

class SerialHelper:public QObject
{
    Q_OBJECT;

public:
    SerialHelper();
    ~SerialHelper();
    static QStringList findAllSerials();
};

#endif // SERIALHELPER_H
