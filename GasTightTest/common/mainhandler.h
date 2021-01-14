#ifndef MAINHANDLER_H
#define MAINHANDLER_H

#include <QObject>
#include <QMutex>
#include <functional>

/**
 * @brief dispatch function to main thread
 */

class MainHandler : public QObject
{
    Q_OBJECT
public:
    explicit MainHandler(QObject *parent = nullptr);
    ~MainHandler();
    static MainHandler* getInstance() ;//使用但实例,跟调用类无关
    void runOnMainThread(QObject *,std::function<void()> func);

private:
    QMutex mMutex;

signals:
    void run();


};

#endif // MAINHANDLER_H
