#ifndef MESMANAGER_H
#define MESMANAGER_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

typedef struct MesRes
{
    bool is_ok;
    QString errorInfo;
}MESRES;

enum OperationType :int
{
    CHECK_ROUTE = 0x01,
    CREATE_ROUTE,
};

class MesManager : public QObject
{
    Q_OBJECT

public:
    explicit MesManager(QObject *parent=nullptr);
    ~MesManager();
    /*过站检查*/
    void checkRoute(const QString&, const QString&, const QString&);
    /*结果上传MES*/
    void createRoute(const QString&, const QString&, const QString&, const QString&);
    void post2Mes(const QString &data);
    void onCheckRouteReply();
    void onCreateRouteReply();
    void onReplyFinshed(QNetworkReply *reply);

signals:
    void resSingal(OperationType,bool,QString errMsg);

private:
    QNetworkAccessManager *mManager=nullptr;
};

#endif // MAINWIN_H
