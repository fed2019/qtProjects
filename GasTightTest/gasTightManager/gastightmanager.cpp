#include "gastightmanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QDebug>


int GasTightManager::objCount=0;
#define LOG qDebug()<<"GasTightManager:"

GasTightManager::GasTightManager(QObject *parent,std::string comName)
    : QObject(parent),
      mComName(comName)
{
    objCount++;
}

GasTightManager::~GasTightManager()
{
   objCount--;
}

bool GasTightManager::setGasTightEnable()
{
    //打开串口
    const char* serialName=mComName.data();
    mSerialObjPtr = swft_serial_open(serialName, 0);
    if(!mSerialObjPtr) return false;
    //配置串口
    int ret=swft_serial_config(mSerialObjPtr,115200,-1,8,1);
    if(ret<0) return false;
    //创建气密性设备操作对象
    mGasObjPtr=swft_gastight_open(mSerialObjPtr,GasTightManager::objCount);
    if(!mGasObjPtr) return false;
    return true;
}

void GasTightManager::registerCallBack(std::shared_ptr<GasTightCallBack> callBackPtr)
{
    if(mCallBackPtr==nullptr&&callBackPtr!=nullptr)
    {
        qDebug()<<"before accept,the use count:"<<mCallBackPtr.use_count();
        mCallBackPtr=callBackPtr;
        qDebug()<<"after accept,the use count:"<<mCallBackPtr.use_count();
    }
}

void GasTightManager::removeCallBack()
{
    if(mCallBackPtr!=nullptr)
    {
        mCallBackPtr.reset();
    }
}

void GasTightManager::checkMachineState()
{
    if(mGasObjPtr)
    {
        mStateCheckThread.post([=](){
            bool isFree=true;
            while(isFree)
            {
                mMux.lock();
                int ret=checkIsMachineFree(mGasObjPtr);
                mMux.unlock();
                LOG<<"begine to check is machine free,"<<"ret is:"<<ret;
                if(ret==1)
                {
                    TestState state=TestState::FREE;
                    if(mCallBackPtr) mCallBackPtr->mTestStateCallBack(mCallBackPtr->callBackFlag,state);
                }
                else
                {
                    isFree=false;
                    TestState state=TestState::TESTING;
                    if(mCallBackPtr) mCallBackPtr->mTestStateCallBack(mCallBackPtr->callBackFlag,state);
                    bool isTesting=true;
                    while(isTesting)
                    {
                        mMux.lock();
                        int ret2=checkIsTestOver(mGasObjPtr);
                        mMux.unlock();
                        LOG<<"begine to check is test complete,"<<"ret is:"<<ret2;
                        if(ret2==1)//test complete
                        {
                            isTesting=false;
                            TestState state2=TestState::COMPLETE;
                            if(mCallBackPtr) mCallBackPtr->mTestStateCallBack(mCallBackPtr->callBackFlag,state2);
                        }
                        Sleep(100);
                    }
                }
                Sleep(100);
            }
        });
    }
}

void GasTightManager::readTestProgress()
{
    if(mGasObjPtr)
    {
        mProgressCheckThread.post([=](){
            int a=0,b=100;
            int *fenzi=&a;
            int *fenmu=&b;
            while(*fenzi<*fenmu)
            {
                mMux.lock();
                int ret=swft_gastight_progress_read(mGasObjPtr, fenzi, fenmu);
                mMux.unlock();
                if(ret!=1)//测试进度读取失败,回调值为-1
                {
                    float progress=-1.0;
                    if(mCallBackPtr) mCallBackPtr->mProgressCallBack(mCallBackPtr->callBackFlag,progress);
                    break;
                }
                else
                {
                    try {
                        float progress=(*fenzi)/(*fenmu);
                        if(mCallBackPtr) mCallBackPtr->mProgressCallBack(mCallBackPtr->callBackFlag,progress);
                    } catch (...) {
                        qWarning()<<"fail in get progress";
                    }
                }
                LOG<<"test progress,fenzi:"<<*fenzi<<",fenmu:"<<*fenmu;
                Sleep(100);
            }
        });
    }
}

void GasTightManager::readCurrentPressData()
{
    if(mGasObjPtr)
    {
        mPressDataGettingThread.post([=](){
            int currentPress=0;
            int *currentPressPtr=&currentPress;
            mMux.lock();
            int ret=swft_gastight_current_pressure_read(mGasObjPtr, currentPressPtr);
            mMux.unlock();
            LOG<<"ret:"<<ret<<",currentPress:"<<*currentPressPtr;
            if(ret==1&&mCallBackPtr)//当前压力值读取成功
            {
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::CURRENT_PRESS_VALUE,*currentPressPtr);
            }
            else if(ret<1&&mCallBackPtr)
            {
                int dumyValue=-1;
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::CURRENT_PRESS_VALUE,dumyValue);
            }
        });
    }
}

void GasTightManager::readTestPressData()
{
    if(mGasObjPtr)
    {
        mPressDataGettingThread.post([=](){
            int testPress=0;
            int *testPressPtr=&testPress;
            mMux.lock();
            int ret=swft_gastight_pressure_read(mGasObjPtr, testPressPtr);
            mMux.unlock();
            LOG<<"ret:"<<ret<<",testPress:"<<*testPressPtr;
            if(ret==1&&mCallBackPtr)//测试压力值读取成功
            {
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::TEST_PRESS_VALUE,*testPressPtr);
            }
            else if(ret<1&&mCallBackPtr)
            {
                int dumyValue=-1;
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::TEST_PRESS_VALUE,dumyValue);
            }
        });
    }
}

void GasTightManager::readTestLeakData()
{
    if(mGasObjPtr)
    {
        mLeakDataGettingThread.post([=](){
            int leakValue=0;
            int leakSpeed=0;
            int *leakValuePtr=&leakValue;
            int *leakSpeedPtr=&leakSpeed;
            mMux.lock();
            int ret1=swft_gastight_leak_read(mGasObjPtr, leakValuePtr);
            int ret2=swft_gastight_leak_speed_read(mGasObjPtr, leakSpeedPtr);
            mMux.unlock();
            LOG<<"ret1:"<<ret1<<",leak Value:"<<*leakValuePtr;
            LOG<<"ret2:"<<ret2<<",leak speed:"<<*leakSpeedPtr;
            if(ret1==1&&mCallBackPtr) //泄漏值读取成功
            {
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::LEAK_VALUE,*leakValuePtr);
            }
            if(ret1<1&&mCallBackPtr)//泄漏值读取失败,返回-1
            {
                int value=-1;
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::LEAK_VALUE,value);
            }
            if(ret2==1&&mCallBackPtr)//泄漏速率读取成功
            {
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::LEAK_SPEED,*leakSpeedPtr);
            }
            if(ret2<1&&mCallBackPtr)
            {
                int dumyValue=-1;
                mCallBackPtr->mDataCallBack(mCallBackPtr->callBackFlag,DataType::LEAK_SPEED,dumyValue);
            }
        });
    }
}

void GasTightManager::readTestResult()
{
    if(mGasObjPtr)
    {
        mResultGettingThread.post([=](){
            unsigned char *result=nullptr;
            bool keepReading=true;
            int retryTimes=0;
            while (keepReading&&retryTimes<4)
            {
                mMux.lock();
                int ret=swft_gastight_result_read(mGasObjPtr, result);
                mMux.unlock();
                LOG<<"ret:"<<ret<<",result:"<<*result;
                if(ret==1&&mCallBackPtr)
                {
                    switch (*result)
                    {
                        case 0x01:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::JIGU_INSTALL);
                            break;
                        case 0x02:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::JINQI);
                            break;
                        case 0x03:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::WENYA);
                            break;
                        case 0x04:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::TEST);
                            break;
                        case 0x05:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::RESULT_OK);
                            keepReading=false;
                            break;
                        case 0x06:
                            mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::RESULT_NG);
                            keepReading=false;
                            break;
                        default:
                            break;
                    }
                }
                else if(ret<1)
                {
                    if(retryTimes==3&&mCallBackPtr) mCallBackPtr->mResultCallBack(mCallBackPtr->callBackFlag,ResultType::RESULT_READ_FAIL);
                    retryTimes++;
                }
                Sleep(50);
            }
        });
    }
}

void GasTightManager::readProgressTime()
{
    if(mGasObjPtr)
    {
        mResultGettingThread.post([=](){
            int a,b,c=0;
            int *pinflate_time=&a;
            int *pholding_time=&b;
            int *ptest_time=&c;
            mMux.lock();
            int ret=swft_gastight_process_time_read(mGasObjPtr,pinflate_time,pholding_time,ptest_time);
            mMux.unlock();
            if(ret==1&&mCallBackPtr)
            {
                float jinqi=*pinflate_time/10;
                float wenya=*pholding_time/10;
                float ceshi=*ptest_time/10;
                mCallBackPtr->mProgressTimeCallBack(mCallBackPtr->callBackFlag,jinqi,wenya,ceshi);
            }
            if(ret<1&&mCallBackPtr)
            {
                float jinqi,wenya,ceshi=0.0f;
                mCallBackPtr->mProgressTimeCallBack(mCallBackPtr->callBackFlag,jinqi,wenya,ceshi);
            }
        });
    }
}
