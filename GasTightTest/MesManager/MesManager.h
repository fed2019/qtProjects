#ifndef MESMANAGER_H
#define MESMANAGER_H
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class MesManager : public QObject
{
    Q_OBJECT

public:
    explicit MesManager(QObject *parent=nullptr);
    ~MesManager();
    void post2Mes(const QString&,const QString&, const QString&, const QString&, const QString&);
    void onReplayFinshed(QNetworkReply *reply);

signals:
    void resSingal(bool,QString errMsg);

private:
    QNetworkAccessManager *mManager=nullptr;

};

#endif // MAINWIN_H
