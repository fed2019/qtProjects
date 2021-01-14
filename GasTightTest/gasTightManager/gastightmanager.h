#ifndef GASTIGHTMANAGER_H
#define GASTIGHTMANAGER_H

#include <QObject>
#include "serial.h"
#include "gastight.h"
#include <string.h>
#include <memory>
#include <mutex>
#include "GasTightcallback.h"
#include "common/SimpleThreadQueue.h"

enum TestState : int
{
    FREE = 0x01, //未测试
    START, //开始测试
    TESTING, //测试中
    COMPLETE, //测试完成
};

enum DataType : int
{
    CURRENT_PRESS_VALUE = 0x01,//当前压力
    TEST_PRESS_VALUE, //测试压力
    LEAK_VALUE, //漏气值
    LEAK_SPEED, //泄漏速率
};

enum ResultType : int
{
    JIGU_INSTALL = 0x01,//夹具安装
    JINQI,//进气
    WENYA,//稳压
    TEST,//测试
    RESULT_OK,//测试结果OK
    RESULT_NG,//测试结果NG
    RESULT_READ_FAIL,//测试结果读取失败
};

class GasTightManager : public QObject
{
    Q_OBJECT
public:
    explicit GasTightManager(QObject *parent = nullptr,std::string comName="");
    ~GasTightManager();
    void registerCallBack(std::shared_ptr<GasTightCallBack>);
    void removeCallBack();
    bool setGasTightEnable();
    void checkMachineState();//读取设备状态,循环读取
    void readTestProgress();//读取测试进度百分比
    void readCurrentPressData();//读取当前压力,进气阶段读取
    void readTestPressData();//读取测试压力,稳压阶段读取
    void readTestLeakData();//读取泄漏值,测试阶段读取
    void readTestResult();//读取测试结果,循环读取,直到结果为0K或者NG或者连续读取失败
    void readProgressTime();//读取测试过程各阶段花费时间,将结果拼接成字符串返回,若读取失败,则返回空字符
    static QString getTestStateName(const TestState);
    static QString getDataTypeName(const DataType);
    static QString getResultTypeName(const ResultType);

signals:

private:
    std::mutex mMux;
    std::string mComName;
    swft_serial_t * mSerialObjPtr=nullptr;
    swft_gastight_t * mGasObjPtr=nullptr;
    static int objCount;
    std::shared_ptr<GasTightCallBack> mCallBackPtr=nullptr;
    ifly::core::SimpleThreadQueue mStateCheckThread;
    ifly::core::SimpleThreadQueue mProgressCheckThread;
    ifly::core::SimpleThreadQueue mPressDataGettingThread;
    ifly::core::SimpleThreadQueue mLeakDataGettingThread;
    ifly::core::SimpleThreadQueue mResultGettingThread;
};

#endif // GASTIGHTMANAGER_H
