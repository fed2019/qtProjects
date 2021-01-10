#include "MesManager.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#define LOG qDebug()<<"MesManager:"


MesManager::MesManager(QObject *parent)
    :QObject(parent)
{
    mManager = new QNetworkAccessManager(this);
    connect(mManager,&QNetworkAccessManager::finished,this,&MesManager::onReplayFinshed);
}

MesManager::~MesManager()
{

}

void MesManager::post2Mes(const QString& url,const QString& pcbSeq, const QString& prodNo, const QString& stationNo, const QString& result)
{
    //构建请求对象
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type","application/json");

    //构建数据
    QString data="?pcbSeq=" + pcbSeq + "&prodNo=" + prodNo + "&stationNo=" + stationNo + "&result=" + result + "&remark=" \
            + "{}{}{}" + "&testItem=" + "" + "&userNo=" + "" + "&weight=" + "0.0" + "&packNo=" + "" + "&rmk1=" + "" + \
            "&rmk2=" + "" + "&rmk3=" + "" + "&rmk4=" + "";

    //发送请求
    mManager->post(request, data.toLocal8Bit());
}

void MesManager::onReplayFinshed(QNetworkReply *reply)
{

    LOG<<"on Reply finshed!";
    if(reply->error()!=QNetworkReply::NoError)
    {
        //处理中的错误信息
        QString errMsg=reply->errorString();
        LOG<<"reply error:"<<errMsg;
        emit resSingal(false,errMsg);
    }
    else
    {
        //发送状态
        emit resSingal(true,"OK");
        //请求方式
        LOG<<"operation:"<<reply->operation();

        //状态码
        LOG<<"status code:"<<reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        LOG<<"url:"<<reply->url();

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
                if (obj.contains("args"))
                {
                    QJsonValue value = obj.value("args");
                    LOG<<value;
                }
            }
        }
        else
        {
            LOG<<"json error:"<<json_error.errorString();
        }
    }
}
