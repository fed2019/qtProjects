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
#include "subwinsetting.h"
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
    void progressCB(const std::string,const int &);
    void dataCB(const std::string,const DataType,const int &);
    void resultCB(const std::string,const ResultType);
    void progressTimeCB(const std::string,const int &,const int &,const int &);
    void testStateHandler(const std::string,const TestState &);
    void progressHandler(const std::string,const int &);
    void dataHandler(const std::string,const DataType,const int &);
    void resultHandler(const std::string,const ResultType);
    void progressTimeHandler(const std::string,const int &,const int &,const int &);

    QString formatTime(int s);
    void record2File(QString flag,bool mesResult,const QString &errMsg);
    void reset(QString flag);
    void readSettings();
    void onEnterInputed();
    void onTimerA();
    void onTimerB();
    void onMesReplyA(OperationType,bool,QString);
    void onMesReplyB(OperationType,bool,QString);
    void createRouteTimeOverCheck(QString flag);
    void checkRouteTimeOverCheck(QString flag);
    void onActionTriggered();
    void closeEvent(QCloseEvent*) override;
    QString getCamraID(QString json);

private:
    Ui::MainWin *ui;
    SubWinSetting *mSubWinSetting=nullptr;
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
    unsigned int mTotalfailCount;
    QTimer *mTimerA;
    QTimer *mTimerB;
    int mTestTimeA;
    int mTestTimeB;

    QString mProdNo;//工单号
    QString mStationNo;//站位号
    QString mPath1ToolNo;//通道1夹具号
    QString mPath2ToolNo;//通道2夹具号

    QString mSn;
    QString mResult;
};
#endif // MAINWIN_H
