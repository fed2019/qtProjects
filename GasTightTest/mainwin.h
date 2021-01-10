#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QStringList>
#include <memory>
#include <QTimer>
#include "gasTightManager/SerialHelper.h"
#include "gasTightManager/gastightmanager.h"
#include "gasTightManager/GasTightcallback.h"
#include "MesManager/MesManager.h"
#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class MainWin; }
QT_END_NAMESPACE

enum ConTextStyle:int
{
    STYLE_WARNNING,
    STYLE_NORMAL,
};

class MainWin : public QMainWindow
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

protected:
    void keyPressEvent(QKeyEvent *) override;
    void initCallBack();//回调注册
    void initView();//界面初始化
    void initSerial();//串口初始化

    void testStateCB(const std::string,const TestState &);
    void progressCB(const std::string,const float &);
    void dataCB(const std::string,const DataType,const int &);
    void resultCB(const std::string,const ResultType);
    void progressTimeCB(const std::string,const float &,const float &,const float &);
    void testStateHandler(const std::string,const TestState &);
    void progressHandler(const std::string,const float &);
    void dataHandler(const std::string,const DataType,const int &);
    void resultHandler(const std::string,const ResultType);
    void progressTimeHandler(const std::string,const float &,const float &,const float &);

    QString formatTime(int s);
    void record2File(QString flag,bool mesResult,const QString &errMsg);
    void reset(QString flag);
    void onEnterInputed();
    void onTimerA();
    void onTimerB();
    void onMesReplyA(bool,QString);
    void onMesReplyB(bool,QString);
    void onBtnClicked();

private:
    Ui::MainWin *ui;
    QStringList mAllSerials;
    GasTightManager *mManagerPtrA=nullptr;
    GasTightManager *mManagerPtrB=nullptr;
    MesManager *mMesManagerA=nullptr;
    MesManager *mMesManagerB=nullptr;
    std::shared_ptr<GasTightCallBack> mCallBackPtrA=nullptr;
    std::shared_ptr<GasTightCallBack> mCallBackPtrB=nullptr;
    float mStandandPress;
    float mStandandLeakValue;
    unsigned int mPassCountA;
    unsigned int mNgCountA;
    unsigned int mPassCountB;
    unsigned int mNgCountB;
    QTimer *mTimerA;
    QTimer *mTimerB;
    int mTestTimeA;
    int mTestTimeB;
    QString mSn;
    QString mProdNo;
    QString mStationNo;
    QString mResult;
};
#endif // MAINWIN_H
