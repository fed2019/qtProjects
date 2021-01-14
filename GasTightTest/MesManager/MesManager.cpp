#include "MesManager.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include "para/Constant.h"

#define LOG qDebug()<<"MesManager:"


MesManager::MesManager(QObject *parent)
    :QObject(parent)
{
    mManager = new QNetworkAccessManager(this);
    connect(mManager,&QNetworkAccessManager::finished,this,&MesManager::onReplyFinshed);
}

MesManager::~MesManager()
{

}

void MesManager::checkRoute(const QString& pcbSeq, const QString& prodNo, const QString& stationNo)
{
    LOG<<"check route";
    QString data = Constants::MES_CHECKROUTE+"?pcbSeq=" + pcbSeq + "&prodNo=" + prodNo + "&stationNo=" + stationNo + "&retest=0";

    //发送请求
    LOG<<data;
    post2Mes(data);
}

void MesManager::createRoute(const QString& pcbSeq, const QString& prodNo, const QString& stationNo, const QString& result)
{
    LOG<<"Create route";
    QString data = Constants::MES_CREATEROUTE+"?pcbSeq=" + pcbSeq + "&prodNo=" + prodNo + "&stationNo=" + stationNo +\
        "&result=" + result + "&remark="+ "{}{}{}" + "&testItem=" + "" + "&userNo=" + "" + "&weight=" + "0.0" +\
        "&packNo=" + "" + "&rmk1=" + "" + "&rmk2=" + "" + "&rmk3=" + "" + "&rmk4=" + "";

    //发送请求
    post2Mes(data);
}

void MesManager::post2Mes(const QString& data)
{
    //构建请求对象
    QNetworkRequest request;
    request.setUrl(QUrl(Constants::MES_URL+data));
    request.setRawHeader("Content-Type","application/json");

    //发送请求
    mManager->get(request);
}

void MesManager::onReplyFinshed(QNetworkReply *reply)
{
    LOG<<"on Reply!";

    //确定是哪种操作引起的响应
    OperationType opType=OperationType::CHECK_ROUTE;
    QUrl requestQrl=reply->url();
    QString path=requestQrl.path();
    if(path.contains("createRoute"))
    {
        LOG<<"create route reply";
        opType=OperationType::CREATE_ROUTE;
    }
    else if(path.contains("checkRoute"))
    {
        LOG<<"check route reply";
        opType=OperationType::CHECK_ROUTE;
    }

    if(reply->error()!=QNetworkReply::NoError)
    {
        //处理中的错误信息
        QString errMsg=reply->errorString();
        LOG<<"Reply error:"<<errMsg;
        emit resSingal(opType,false,errMsg);
    }
    else
    {
        //获取响应信息
        const QByteArray reply_data=reply->readAll();
        LOG<<"read all:"<<reply_data;

        //解析json
        QJsonParseError json_error;
        QJsonDocument doucment = QJsonDocument::fromJson(reply_data, &json_error);
        if (json_error.error == QJsonParseError::NoError)
        {
            if (doucment.isObject())
            {
                const QJsonObject obj = doucment.object();
                LOG<<obj;
                if (obj.contains("msgId"))
                {
                    QJsonValue value = obj.value("msgId");
                    if(value.toInt()==1)
                    {
                        if(obj.contains("msgStr"))
                        {
                            QJsonValue value_msg = obj.value("msgStr");
                            QString msg_str=value_msg.toString();
                            emit resSingal(opType,false,msg_str);
                        }
                    }
                    else if (value.toInt()==0)
                    {
                        emit resSingal(opType,true,"OK");
                    }
                }
            }
        }
        else
        {
            LOG<<"json error:"<<json_error.errorString();
            emit resSingal(opType,false,json_error.errorString());
        }
    }
}

