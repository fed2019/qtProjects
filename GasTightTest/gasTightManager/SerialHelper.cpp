#include "SerialHelper.h"

SerialHelper::SerialHelper()
{

}

SerialHelper::~SerialHelper()
{

}

QStringList SerialHelper::findAllSerials()
{
    QStringList serialLists;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
         serialLists.append(info.portName());
    }
    return serialLists;
}
