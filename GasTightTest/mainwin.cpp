#include "mainwin.h"
#include "ui_mainwin.h"
#include <QDebug>
#include <QScreen>
#include "para/Constant.h"
#include "common/mainhandler.h"
#include <QMessageBox>
#include <QLineEdit>
#include <QException>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#define LOG qDebug()<<"MainWin:"
static const int TIME_OUT=1500;

MainWin::MainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);

    mSubWinSetting=new SubWinSetting();
    connect(mSubWinSetting,&SubWinSetting::confirmClicked,this,&MainWin::readSettings);
    mSubWinSetting->hide();
    readSettings();

    initSerial();
    initCallBack();
    initView();

    mPassCountA=0;
    mNgCountA=0;
    mPassCountB=0;
    mNgCountB=0;
    mTotalfailCount=0;
    mTimerA=new QTimer(this);
    mTimerB=new QTimer(this);
    connect(mTimerA,&QTimer::timeout,this,&MainWin::onTimerA);
    connect(mTimerB,&QTimer::timeout,this,&MainWin::onTimerB);

    mMesManagerA=new MesManager(this);
    mMesManagerB=new MesManager(this);
    connect(mMesManagerA,&MesManager::resSingal,this,&MainWin::onMesReplyA);
    connect(mMesManagerB,&MesManager::resSingal,this,&MainWin::onMesReplyB);
    connect(ui->action_shuxin,&QAction::triggered,this,&MainWin::onActionTriggered);
}

void MainWin::closeEvent(QCloseEvent* evt)
{
    Q_UNUSED(evt);
    if(mSubWinSetting)
    {
        mSubWinSetting->close();
        delete mSubWinSetting;
        mSubWinSetting=nullptr;
    }
}

MainWin::~MainWin()
{
    if(mManagerPtrA) mManagerPtrA->removeCallBack();
    if(mManagerPtrB) mManagerPtrB->removeCallBack();
    delete ui;
}

void MainWin::initView()
{
    //获取主屏分辨率
    QRect mRect;
    mRect = QGuiApplication::primaryScreen()->geometry();
    //设置初始化尺寸及位置
    this->setGeometry(int(0.1*mRect.width()),int(0.1*mRect.height()),int(0.8*mRect.width()),int(0.8*mRect.height()));
    //标题设置
    this->setWindowTitle(Constants::APP_NAME);
    ui->lineEdit_gongjuNu_A->setFocus();

    //显示样式确定控件
    ui->label_successNum_input->setStyleSheet(Constants::SUCCESS_STYLE);
    ui->label_failNum_input->setStyleSheet(Constants::WARNNING_STYLE);
}

void MainWin::initSerial()
{
    mAllSerials=SerialHelper::findAllSerials();
    if(!mAllSerials.isEmpty())
    {
        LOG<<"ALL Serials:"<<mAllSerials;
        mManagerPtrA=new GasTightManager(this,mAllSerials.at(0).toStdString());
        ui->label_serialNo_input_A->setText(mAllSerials.at(0));
        bool ret=mManagerPtrA->setGasTightEnable();
        if(ret)
        {
            LOG<<"serial "<<mAllSerials.at(0)<<" open success";
            ui->label_serialStatus_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_serialStatus_input_A->setText(Constants::SERIAL_SUCCESS);
        }
        else
        {
            LOG<<"serial "<<mAllSerials.at(0)<<" open fail";
            ui->label_serialNo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_serialStatus_input_A->setText(Constants::SERIAL_FAIL);
        }
        if(mAllSerials.length()>=2)
        {
            mManagerPtrB=new GasTightManager(this,mAllSerials.at(1).toStdString());
            ui->label_serialNo_input_B->setText(mAllSerials.at(1));
            bool ret=mManagerPtrB->setGasTightEnable();
            if(ret)
            {
                LOG<<"serial "<<mAllSerials.at(1)<<" open success";
                ui->label_serialStatus_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_serialStatus_input_B->setText(Constants::SERIAL_SUCCESS);
            }
            else
            {
                LOG<<"serial "<<mAllSerials.at(0)<<" open fail";
                ui->label_serialNo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_serialStatus_input_B->setText(Constants::SERIAL_FAIL);
            }
        }
    }
}

void MainWin::initCallBack()
{
    mCallBackPtrA=std::make_shared<GasTightCallBack>("A");
    mCallBackPtrB=std::make_shared<GasTightCallBack>("B");
    mCallBackPtrA->mTestStateCallBack = std::bind(&MainWin::testStateCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrA->mProgressCallBack = std::bind(&MainWin::progressCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrA->mDataCallBack = std::bind(&MainWin::dataCB,this,placeholders::_1,placeholders::_2,placeholders::_3);
    mCallBackPtrA->mResultCallBack = std::bind(&MainWin::resultCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrA->mProgressTimeCallBack = std::bind(&MainWin::progressTimeCB,this,placeholders::_1,placeholders::_2,placeholders::_3,placeholders::_4);
    mCallBackPtrB->mTestStateCallBack = std::bind(&MainWin::testStateCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrB->mProgressCallBack = std::bind(&MainWin::progressCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrB->mDataCallBack = std::bind(&MainWin::dataCB,this,placeholders::_1,placeholders::_2,placeholders::_3);
    mCallBackPtrB->mResultCallBack = std::bind(&MainWin::resultCB,this,placeholders::_1,placeholders::_2);
    mCallBackPtrB->mProgressTimeCallBack = std::bind(&MainWin::progressTimeCB,this,placeholders::_1,placeholders::_2,placeholders::_3,placeholders::_4);
    if(mManagerPtrA)
    {
        LOG<<"init call back A";
        mManagerPtrA->registerCallBack(mCallBackPtrA);
    }
    if(mManagerPtrB)
    {
        LOG<<"init call back B";
        mManagerPtrB->registerCallBack(mCallBackPtrB);
    }
}

void MainWin::testStateCB(const std::string callBackFlag,const TestState &state)
{
    LOG<<"test state CB";
    MainHandler::getInstance()->runOnMainThread(this,[callBackFlag,state,this](){
        this->testStateHandler(callBackFlag,state);
    });
}

void MainWin::progressCB(const std::string callBackFlag,const int &progress)
{
    LOG<<"progress CB";
    MainHandler::getInstance()->runOnMainThread(this,[callBackFlag,progress,this](){
        this->progressHandler(callBackFlag,progress);
    });
}

void MainWin::dataCB(const std::string callBackFlag,const DataType type,const int &data)
{
    LOG<<"data CB";
    MainHandler::getInstance()->runOnMainThread(this,[callBackFlag,type,data,this](){
                this->dataHandler(callBackFlag,type,data);
    });
}

void MainWin::resultCB(const std::string callBackFlag,const ResultType type)
{
    LOG<<"result CB";
    MainHandler::getInstance()->runOnMainThread(this,[callBackFlag,type,this](){
                this->resultHandler(callBackFlag,type);
    });
}

void MainWin::progressTimeCB(const std::string callBackFlag,const int &pinflate_time, const int &pholding_time, const int &ptest_time)
{
    LOG<<"progressTime CB";
    MainHandler::getInstance()->runOnMainThread(this,[callBackFlag,pinflate_time,pholding_time,ptest_time,this](){
                this->progressTimeHandler(callBackFlag,pinflate_time,pholding_time,ptest_time);
    });
}

void MainWin::testStateHandler(const std::string callBackFlag,const TestState &state)
{
    LOG<<"testStateHandler,"<<"flag:"<<QString::fromStdString(callBackFlag)<<",testState:"<<GasTightManager::getTestStateName(state);
    if(callBackFlag=="A")
    {
        if(state==TestState::TESTING)
        {
            ui->label_hintInfo_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_ALREADY_START);
            //mManagerPtrA->readTestProgress();
            mManagerPtrA->readTestResult();
        }
        if(state==TestState::COMPLETE)
        {
            //测试完毕,开始读取测试数据
            mManagerPtrA->readTestPressData();
            mManagerPtrA->readTestLeakData();
        }
    }
    else if(callBackFlag=="B")
    {
        if(state==TestState::TESTING)
        {
            ui->label_hintInfo_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::HINTINFO_ALREADY_START);
            //mManagerPtrB->readTestProgress();
            mManagerPtrB->readTestResult();
        }
        if(state==TestState::COMPLETE)
        {
            //测试完毕,开始读取测试数据
            mManagerPtrB->readTestPressData();
            mManagerPtrB->readTestLeakData();
        }
    }
}

void MainWin::progressHandler(const std::string callBackFlag,const int &progress)
{
    LOG<<"progressHandler"<<",flag is:"<<QString::fromStdString(callBackFlag)<<",progress:"<<progress;
    if(callBackFlag=="A")
    {
        if(progress>0)
        {
           ui->progressBar_A->setValue(progress);
        }
    }
    else if(callBackFlag=="B")
    {
        if(progress>0)
        {
           ui->progressBar_B->setValue(progress);
        }
    }
}

void MainWin::dataHandler(const std::string callBackFlag,const DataType type,const int &data)
{
    LOG<<"dataHandler,"<<"flag:"<<QString::fromStdString(callBackFlag)<<",dataType:"<<GasTightManager::getDataTypeName(type)<<",data:"<<data;
    if(callBackFlag=="A")
    {
        switch (type) {
        case DataType::CURRENT_PRESS_VALUE:
            if(data>=0)
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_pressOffset_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                float pressValue=data/1000.0f;
                QString pressContext=QString("%1KPa").arg(pressValue);
                ui->label_testPress_input_A->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_A->setText(offsetContext);
            }
            else
            {
                //ui->label_testPress_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                //ui->label_testPress_input_A->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::TEST_PRESS_VALUE:
            if(data>=0)
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_pressOffset_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float pressValue=data/1000.0f;
                QString pressContext=QString("%1KPa").arg(pressValue);
                ui->label_testPress_input_A->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_A->setText(offsetContext);
            }
            else
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_testPress_input_A->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::LEAK_VALUE:
            if(data>=0)
            {
                ui->label_leak_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_leakOffset_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakValue=data/1000.0f;
                QString leakContext=QString("%1KPa").arg(leakValue);
                ui->label_leak_input_A->setText(leakContext);
                float offsetValue=data/mStandandLeakValue*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_leakOffset_input_A->setText(offsetContext);
            }
            else
            {
                ui->label_leak_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leak_input_A->setText(Constants::LEAK_READ_FAIL);
            }
            break;
        case DataType::LEAK_SPEED:
            if(data>=0)
            {
                ui->label_leakSpeed_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakSpeedValue=data/1000.0f;
                QString leakSpeedContext=QString("%1 SCCM").arg(leakSpeedValue);
                ui->label_leakSpeed_input_A->setText(leakSpeedContext);
            }
            else
            {
                ui->label_leakSpeed_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leakSpeed_input_A->setText(Constants::LEAKSPEED_READ_FAIL);
            }
            break;
        }
    }
    else if(callBackFlag=="B")
    {
        switch (type) {
        case DataType::CURRENT_PRESS_VALUE:
            if(data>0)
            {
                ui->label_testPress_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_pressOffset_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                float pressValue=data/1000.0f;
                QString pressContext=QString("%1KPa").arg(pressValue);
                ui->label_testPress_input_B->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_B->setText(offsetContext);
            }
            else
            {
                //ui->label_testPress_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                //ui->label_testPress_input_B->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::TEST_PRESS_VALUE:
            if(data>=0)
            {
                ui->label_testPress_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_pressOffset_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float pressValue=data/1000.0f;
                QString pressContext=QString("%1KPa").arg(pressValue);
                ui->label_testPress_input_B->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_B->setText(offsetContext);
            }
            else
            {
                ui->label_testPress_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_testPress_input_B->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::LEAK_VALUE:
            if(data>=0)
            {
                ui->label_leak_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_leakOffset_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakValue=data/1000.0f;
                QString leakContext=QString("%1KPa").arg(leakValue);
                ui->label_leak_input_B->setText(leakContext);
                float offsetValue=data/mStandandLeakValue*100.0f;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_leakOffset_input_B->setText(offsetContext);
            }
            else
            {
                ui->label_leak_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leak_input_B->setText(Constants::LEAK_READ_FAIL);
            }
            break;
        case DataType::LEAK_SPEED:
            if(data>=0)
            {
                ui->label_leakSpeed_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakSpeedValue=data/1000.0f;
                QString leakSpeedContext=QString("%1 SCCM").arg(leakSpeedValue);
                ui->label_leakSpeed_input_B->setText(leakSpeedContext);
            }
            else
            {
                ui->label_leakSpeed_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leakSpeed_input_B->setText(Constants::LEAK_READ_FAIL);
            }
            break;
        }
    }
}

void MainWin::resultHandler(const std::string callBackFlag,const ResultType type)
{
    LOG<<"resultHandler,"<<"flag:"<<QString::fromStdString(callBackFlag)<<",resultType:"<<GasTightManager::getResultTypeName(type);
    if(callBackFlag=="A")
    {
        switch (type) {
        case ResultType::JIGU_INSTALL:
            ui->label_progressInfo_A->setText("夹具安装");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::JINQI:
            ui->label_progressInfo_A->setText("进气");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::WENYA:
            ui->label_progressInfo_A->setText("稳压");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::TEST:
            ui->label_progressInfo_A->setText("测试");
            break;
        case ResultType::RESULT_OK:
            //设置UI
            ui->label_testResult_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_testResult_input_A->setText("OK");
            ui->label_testStatus_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_testStatus_input_A->setText("OK");
            mPassCountA++;
            ui->label_successNum_input->setText(QString("%1").arg(mPassCountA+mPassCountB));
            //逻辑处理
            //to do:1.调用接口上传MES
            mSn=ui->lineEdit_SN_A->text();
            mResult="PASS";
            mMesManagerA->createRoute(mSn,mProdNo,mStationNo,mResult);
            createRouteTimeOverCheck("A");
            break;
        case ResultType::RESULT_NG:
            //设置UI
            ui->label_testResult_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testResult_input_A->setText("NG");
            ui->label_testStatus_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testStatus_input_A->setText("NG");
            mNgCountA++;
            mTotalfailCount=mNgCountA+mNgCountB;
            ui->label_failNum_input->setText(QString("%1").arg(mTotalfailCount));
            ui->label_testFailNum_input_A->setText(QString("%1").arg(mNgCountA));
            //逻辑处理
            //to do:1.调用接口上传MES
            mSn=ui->lineEdit_SN_A->text();
            mResult="FAIL";
            mMesManagerA->createRoute(mSn,mProdNo,mStationNo,mResult);
            createRouteTimeOverCheck("A");
            break;
        case ResultType::RESULT_READ_FAIL:
            break;
        }
    }
    else
    {
        switch (type) {
        case ResultType::JIGU_INSTALL:
            ui->label_progressInfo_B->setText("夹具安装");
            mManagerPtrB->readCurrentPressData();
            break;
        case ResultType::JINQI:
            ui->label_progressInfo_B->setText("进气");
            mManagerPtrB->readCurrentPressData();
            break;
        case ResultType::WENYA:
            ui->label_progressInfo_B->setText("稳压");
            mManagerPtrB->readCurrentPressData();
            break;
        case ResultType::TEST:
            ui->label_progressInfo_B->setText("测试");
            break;
        case ResultType::RESULT_OK:
            ui->label_testResult_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_testResult_input_B->setText("OK");
            mPassCountB++;
            ui->label_testStatus_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_testStatus_input_B->setText("OK");
            ui->label_successNum_input->setText(QString("%1").arg(mPassCountA+mPassCountB));
            //逻辑处理
            //to do:1.调用接口上传MES
            mSn=ui->lineEdit_SN_B->text();
            mResult="PASS";
            mMesManagerB->createRoute(mSn,mProdNo,mStationNo,mResult);
            createRouteTimeOverCheck("B");
            break;
        case ResultType::RESULT_NG:
            ui->label_testResult_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testResult_input_B->setText("NG");
            ui->label_testStatus_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testStatus_input_B->setText("NG");
            mNgCountB++;
            mTotalfailCount=mNgCountA+mNgCountB;
            ui->label_failNum_input->setText(QString("%1").arg(mTotalfailCount));
            ui->label_testFailNum_input_B->setText(QString("%1").arg(mNgCountB));
            //逻辑处理
            //to do:1.调用接口上传MES
            mSn=ui->lineEdit_SN_B->text();
            mResult="FAIL";
            mMesManagerB->createRoute(mSn,mProdNo,mStationNo,mResult);
            createRouteTimeOverCheck("B");
            break;
        case ResultType::RESULT_READ_FAIL:
            break;
        }
    }
}

void MainWin::progressTimeHandler(const std::string callBackFlag,const int &pinflate_time,const int &pholding_time,const int &ptest_time)
{
    LOG<<"progressTimeHandler,"<<"flag:"<<QString::fromStdString(callBackFlag)<<",pinflate_time:"<<pinflate_time\
      <<",pholding_time:"<<pholding_time<<",ptest_time"<<ptest_time;
    if(pinflate_time==0) return;
    float chongqi=float(pinflate_time)/10.0f;
    float wenya=float(pholding_time)/10.0f;
    float test=float(ptest_time)/10.0f;
    QString result=QString("充气%1秒,稳压%2秒,测试%3秒").arg(chongqi).arg(wenya).arg(test);
    if(callBackFlag=="A")
    {
        ui->label_progressInfo_A->setText(result);
    }
    else if(callBackFlag=="B")
    {
        ui->label_progressInfo_B->setText(result);
    }
}

void MainWin::onTimerA()
{
    mTestTimeA+=1;
    QString costTime=formatTime(mTestTimeA);
    ui->label_testTime_input_A->setText(costTime);
}

void MainWin::onTimerB()
{
    mTestTimeB+=1;
    QString costTime=formatTime(mTestTimeB);
    ui->label_testTime_input_B->setText(costTime);
}

void MainWin::checkRouteTimeOverCheck(QString flag)
{
    //向mes上传数据后,若设定时间内没收到响应,则认为上传失败,执行上传失败流程
    if(flag=="A")
    {
        QTimer::singleShot(TIME_OUT,this,[=](){
            if(ui->label_testStatus_input_A->text()==Constants::TEST_STATUS_TESTING||ui->lineEdit_SN_A->text().length()==0) return;
            ui->lineEdit_SN_A->clear();
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_MES_NORESPONSE);
        });
    }
    else
    {
        QTimer::singleShot(TIME_OUT,this,[=](){
            if(ui->label_testStatus_input_B->text()==Constants::TEST_STATUS_TESTING||ui->lineEdit_SN_B->text().length()==0) return;
            ui->lineEdit_SN_B->clear();
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::HINTINFO_MES_NORESPONSE);
        });
    }
}

void MainWin::createRouteTimeOverCheck(QString flag)
{
    //向mes上传数据后,若设定时间内没收到响应,则认为上传失败,执行上传失败流程
    if(flag=="A")
    {
        QTimer::singleShot(TIME_OUT,this,[=](){
            if(ui->label_postToMes_input_A->text().length()==0)
            {
                ui->label_postToMes_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_postToMes_input_A->setText(Constants::RESULT_FAIL);
                ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_hintInfo_input_A->setText(Constants::HINTINFO_MES_NORESPONSE);
                if(mTimerA->isActive()) mTimerA->stop();
                record2File("A",false,Constants::HINTINFO_MES_NORESPONSE);
                ui->lineEdit_SN_A->clear();
                ui->lineEdit_gongjuNu_A->clear();//结果上传Mes及保存至本地日志系统之后,将界面复位
            }
        });
    }
    else
    {
        QTimer::singleShot(TIME_OUT,this,[=](){
            if(ui->label_postToMes_input_B->text().length()==0)
            {
                ui->label_postToMes_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_postToMes_input_B->setText(Constants::RESULT_FAIL);
                ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_hintInfo_input_B->setText(Constants::HINTINFO_MES_NORESPONSE);
                if(mTimerB->isActive()) mTimerB->stop();
                record2File("A",false,Constants::HINTINFO_MES_NORESPONSE);
                ui->lineEdit_SN_B->clear();
                ui->lineEdit_gongjuNu_B->clear();//结果上传Mes及保存至本地日志系统之后,将界面复位
            }
        });
    }
}

void MainWin::onMesReplyA(OperationType type,bool result,QString errMsg)
{
    LOG<<"On MES reply A,"<<"OperationType:"<<type<<",result:"<<result<<",errMsg:"<<errMsg;
    if(type==OperationType::CHECK_ROUTE)
    {
        if(result)
        {
            //设置UI
            reset("A");
            ui->label_testStatus_input_A->setStyleSheet(Constants::COMMON_STYLE);
            ui->label_testStatus_input_A->setText(Constants::TEST_STATUS_TESTING);
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_START);
            ui->lineEdit_gongjuNu_B->setFocus();
            //开启定时器记录测试耗时
            mTestTimeA=0;
            if(mTimerA->isActive())
            {
                mTimerA->stop();
                mTimerA->start(1000);
            }
            else
            {
                mTimerA->start(1000);
            }

            //检查设备状态
            if(mManagerPtrA)
            {
                mManagerPtrA->checkMachineState();
            }
        }
        else
        {
            ui->lineEdit_SN_A->clear();
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            if(errMsg.contains("未过站"))
            {
                ui->label_hintInfo_input_A->setText(Constants::HINTINFO_SN_NOTFIND);
            }
            else if(errMsg.contains("测试PASS"))
            {
                ui->label_hintInfo_input_A->setText(Constants::HINTINFO_SN_ALREADYPASS);
            }
            else
            {
                ui->label_hintInfo_input_A->setText(Constants::HINTINFO_SN_UNKNOWERR);
            }
        }
    }
    else if(type==OperationType::CREATE_ROUTE)
    {
        if(result)
        {
            ui->label_postToMes_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_postToMes_input_A->setText(Constants::RESULT_OK);
        }
        else
        {
            ui->label_postToMes_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_postToMes_input_A->setText(Constants::RESULT_FAIL);
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_MES_MISTAKE);
        }
        if(mTimerA->isActive()) mTimerA->stop();
        record2File("A",result,errMsg);
        ui->lineEdit_SN_A->clear();
        ui->lineEdit_gongjuNu_A->clear();//结果上传Mes及保存至本地日志系统之后,将界面复位
    }
}

void MainWin::onMesReplyB(OperationType type,bool result,QString errMsg)
{
    LOG<<"On MES reply B,"<<"OperationType:"<<type<<",result:"<<result<<",errMsg:"<<errMsg;
    if(type==OperationType::CHECK_ROUTE)
    {
        if(result)
        {
            //设置UI
            reset("B");
            ui->label_testStatus_input_B->setStyleSheet(Constants::COMMON_STYLE);
            ui->label_testStatus_input_B->setText(Constants::TEST_STATUS_TESTING);
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::HINTINFO_START);
            ui->lineEdit_gongjuNu_A->setFocus();
            //开启定时器记录测试耗时
            mTestTimeB=0;
            if(mTimerB->isActive())
            {
                mTimerB->stop();
                mTimerB->start(1000);
            }
            else
            {
                mTimerB->start(1000);
            }

            //检查设备状态
            if(mManagerPtrB)
            {
                mManagerPtrB->checkMachineState();

            }
        }
        else
        {
            ui->lineEdit_SN_B->clear();
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            if(errMsg.contains("未过站"))
            {
                ui->label_hintInfo_input_B->setText(Constants::HINTINFO_SN_NOTFIND);
            }
            else if(errMsg.contains("测试PASS"))
            {
                ui->label_hintInfo_input_B->setText(Constants::HINTINFO_SN_ALREADYPASS);
            }
            else
            {
                ui->label_hintInfo_input_B->setText(Constants::HINTINFO_SN_UNKNOWERR);
            }
        }
    }
    else if(type==OperationType::CREATE_ROUTE)
    {
        if(result)
        {
            ui->label_postToMes_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_postToMes_input_B->setText(Constants::RESULT_OK);
        }
        else
        {
            ui->label_postToMes_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_postToMes_input_B->setText(Constants::RESULT_FAIL);
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::HINTINFO_MES_MISTAKE);
        }
        if(mTimerB->isActive()) mTimerB->stop();
        record2File("B",result,errMsg);
        ui->lineEdit_SN_B->clear();
        ui->lineEdit_gongjuNu_B->clear();//结果上传Mes及保存至本地日志系统之后,将界面复位
    }
}

void MainWin::onActionTriggered()
{
    if(mSubWinSetting)
    {
        mSubWinSetting->show();
    }
}

//处理扫码枪输入
void MainWin::keyPressEvent(QKeyEvent * evt)
{
    if(evt->key() == Qt::Key_Return)//enter
    {
        evt->accept();
        onEnterInputed();
    }
}

void MainWin::onEnterInputed()
{
    QWidget *currentFocusWidget = this->focusWidget();
    if(currentFocusWidget==ui->lineEdit_gongjuNu_A&&ui->lineEdit_gongjuNu_A->text().length()>0)
    {
        if(ui->lineEdit_gongjuNu_A->text()!=mPath1ToolNo)
        {
            ui->lineEdit_gongjuNu_A->clear();
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::JIAJU_MISTAKE);
            return;
        }
        ui->label_hintInfo_input_A->clear();
        ui->lineEdit_SN_A->setFocus();
        return;
    }
    if(currentFocusWidget==ui->lineEdit_SN_A&&ui->lineEdit_SN_A->text().length()>0)
    {
        //解析JSON数据
        QString id=getCamraID(ui->lineEdit_SN_A->text());
        if(id.length()==0)
        {
            ui->lineEdit_SN_A->clear();
            ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_SN_SCANMISTAKE);
            return;
        }
        ui->lineEdit_SN_A->setText(id);

        //检测SN
        mSn=ui->lineEdit_SN_A->text();
        mMesManagerA->checkRoute(mSn,mProdNo,mStationNo);
        checkRouteTimeOverCheck("A");

        //设置UI
//        reset("A");
//        ui->label_testStatus_input_A->setStyleSheet(Constants::COMMON_STYLE);
//        ui->label_testStatus_input_A->setText(Constants::TEST_STATUS_TESTING);
//        ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
//        ui->label_hintInfo_input_A->setText(Constants::HINTINFO_START);
//        ui->lineEdit_gongjuNu_B->setFocus();

//        //开启定时器记录测试耗时
//        mTestTimeA=0;
//        if(mTimerA->isActive())
//        {
//            mTimerA->stop();
//            mTimerA->start(1000);
//        }
//        else
//        {
//            mTimerA->start(1000);
//        }

//        //检查设备状态
//        if(mManagerPtrA)
//        {
//            mManagerPtrA->checkMachineState();
//        }
        return;
    }
    if(currentFocusWidget==ui->lineEdit_gongjuNu_B&&ui->lineEdit_gongjuNu_B->text().length()>0)
    {
        if(ui->lineEdit_gongjuNu_B->text()!=mPath2ToolNo)
        {
            ui->lineEdit_gongjuNu_B->clear();
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::JIAJU_MISTAKE);
            return;
        }
        ui->label_hintInfo_input_B->clear();
        ui->lineEdit_SN_B->setFocus();
        return;
    }
    if(currentFocusWidget==ui->lineEdit_SN_B&&ui->lineEdit_SN_B->text().length()>0)
    {
        //解析JSON数据
        QString id=getCamraID(ui->lineEdit_SN_B->text());
        if(id.length()==0)
        {
            ui->lineEdit_SN_B->clear();
            ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_hintInfo_input_B->setText(Constants::HINTINFO_SN_SCANMISTAKE);
            return;
        }
        ui->lineEdit_SN_B->setText(id);
        //检测SN
        mSn=ui->lineEdit_SN_B->text();
        mMesManagerB->checkRoute(mSn,mProdNo,mStationNo);
        checkRouteTimeOverCheck("B");

        //设置UI
//        reset("B");
//        ui->label_testStatus_input_B->setStyleSheet(Constants::COMMON_STYLE);
//        ui->label_testStatus_input_B->setText(Constants::TEST_STATUS_TESTING);
//        ui->label_hintInfo_input_B->setStyleSheet(Constants::WARNNING_STYLE);
//        ui->label_hintInfo_input_B->setText(Constants::HINTINFO_START);
//        ui->lineEdit_gongjuNu_A->setFocus();

//        //开启定时器记录测试耗时
//        mTestTimeB=0;
//        if(mTimerB->isActive())
//        {
//            mTimerB->stop();
//            mTimerB->start(1000);
//        }
//        else
//        {
//            mTimerB->start(1000);
//        }

//        //检查设备状态
//        if(mManagerPtrB)
//        {
//            mManagerPtrB->checkMachineState();
//        }
        return;
    }
    ui->lineEdit_gongjuNu_A->setFocus();
}

QString MainWin::formatTime(int s)
{
    int ss = 1;
    int mi = ss * 60;
    int hh = mi * 60;
    int dd = hh * 24;

    long day = s / dd;
    long hour = (s - day * dd) / hh;
    long minute = (s - day * dd - hour * hh) / mi;
    long second = (s - day * dd - hour * hh - minute * mi) / ss;

    QString min = QString::number(minute,10);
    if(min.length()==1) min="0"+min;
    QString sec = QString::number(second,10);
    if(sec.length()==1) sec="0"+sec;

    return min + ":" + sec ;
}

void MainWin::record2File(QString flag,bool mesResult,const QString &errMsg)
{
    if(flag=="A")
    {
        qInfo()<<"SN:"<<ui->lineEdit_SN_A->text();
        qInfo()<<"夹具号:"<<ui->lineEdit_gongjuNu_A->text();
        qInfo()<<"测试压力:"<<ui->label_testPress_input_A->text();
        qInfo()<<"偏差:"<<ui->label_pressOffset_input_A->text();
        qInfo()<<"漏气值:"<<ui->label_leak_input_A->text();
        qInfo()<<"偏差:"<<ui->label_leakOffset_input_A->text();
        qInfo()<<"泄漏速率:"<<ui->label_leakSpeed_input_A->text();
        qInfo()<<"检测结果:"<<ui->label_testResult_input_A->text();
        qInfo()<<"上报MES:"<<ui->label_postToMes_input_A->text();
        if(!mesResult) qInfo()<<"错误信息:"<<errMsg;
    }
    else if(flag=="B")
    {
        qInfo()<<"SN:"<<ui->lineEdit_SN_B->text();
        qInfo()<<"夹具号:"<<ui->lineEdit_gongjuNu_B->text();
        qInfo()<<"测试压力:"<<ui->label_testPress_input_B->text();
        qInfo()<<"偏差:"<<ui->label_pressOffset_input_B->text();
        qInfo()<<"漏气值:"<<ui->label_leak_input_B->text();
        qInfo()<<"偏差:"<<ui->label_leakOffset_input_B->text();
        qInfo()<<"泄漏速率:"<<ui->label_leakSpeed_input_B->text();
        qInfo()<<"检测结果:"<<ui->label_testResult_input_B->text();
        qInfo()<<"上报MES:"<<ui->label_postToMes_input_B->text();
        if(!mesResult) qInfo()<<"错误信息:"<<errMsg;
    }
}

void MainWin::reset(QString flag)
{
    if(flag=="A")
    {
        ui->progressBar_A->setValue(0);
        ui->label_progressInfo_A->clear();
        ui->label_testStatus_input_A->clear();
        if(mTimerA->isActive()) mTimerA->stop();
        mTestTimeA=0;
        ui->label_testTime_input_A->clear();
        ui->label_testPress_input_A->clear();
        ui->label_pressOffset_input_A->clear();
        ui->label_leak_input_A->clear();
        ui->label_leakOffset_input_A->clear();
        ui->label_leakSpeed_input_A->clear();
        ui->label_testResult_input_A->clear();
        ui->label_postToMes_input_A->clear();
        ui->label_hintInfo_input_A->clear();
    }
    else if(flag=="B")
    {
        ui->progressBar_B->setValue(0);
        ui->label_progressInfo_B->clear();
        ui->label_testStatus_input_B->clear();
        if(mTimerB->isActive()) mTimerB->stop();
        mTestTimeB=0;
        ui->label_testTime_input_B->clear();
        ui->label_testPress_input_B->clear();
        ui->label_pressOffset_input_B->clear();
        ui->label_leak_input_B->clear();
        ui->label_leakOffset_input_B->clear();
        ui->label_leakSpeed_input_B->clear();
        ui->label_testResult_input_B->clear();
        ui->label_postToMes_input_B->clear();
        ui->label_hintInfo_input_B->clear();
    }
}

void MainWin::readSettings()
{
    if(mSubWinSetting==nullptr) return;
    QStringList nessaryInfo=mSubWinSetting->getNessaryInfo();
    if(nessaryInfo.length()>=4)
    {
        mProdNo=nessaryInfo.at(0);
        mStationNo=nessaryInfo.at(1);
        mPath1ToolNo=nessaryInfo.at(2);
        mPath2ToolNo=nessaryInfo.at(3);
    }
    QStringList unNessaryInfo=mSubWinSetting->getUnNessaryInfo();
    if(unNessaryInfo.length()>=8)
    {
        ui->label_model_input->setText(unNessaryInfo.at(0));
        ui->label_stationName_input->setText(unNessaryInfo.at(1));
        QString sampleIndex=QString("压力 %1KPa±%2KPa, 泄漏上限 %3PA, 充气时间%4, 稳压时间%5, 测试时间%6")\
                .arg(unNessaryInfo.at(2)).arg(unNessaryInfo.at(3)).arg(unNessaryInfo.at(4)).arg(unNessaryInfo.at(5))\
                .arg(unNessaryInfo.at(6)).arg(unNessaryInfo.at(7));
        ui->label_sampleIdx_input->setText(sampleIndex);

        mStandandPress=unNessaryInfo.at(2).toFloat();
        mStandandLeakValue=unNessaryInfo.at(4).toFloat();
    }
}

QString MainWin::getCamraID(QString json)
{
    LOG<<"get cam id"<<",json:"<<json;
    if(!json.contains("{"))//非json格式数据,返回空字符串
    {
        LOG<<"not json data";
        return "";
    }
    json.replace("“","\"");
    json.replace("”","\"");
    json.replace("：",":");

    QJsonObject object;
    QJsonParseError json_error;
    QJsonDocument doc=QJsonDocument::fromJson(json.toUtf8().data(),&json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        object=doc.object();
        if(object.contains("cameraID"))
        {
            QJsonValue value = object.value("cameraID");
            QString id=value.toString();
            return id;
        }
        return "";
    }
    else
    {
        LOG<<"json error:"<<json_error.errorString();
        return "";
    }
}
