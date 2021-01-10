#include "mainwin.h"
#include "ui_mainwin.h"
#include <QDebug>
#include <QScreen>
#include <QDir>
#include "para/Constant.h"
#include "common/mainhandler.h"
#include <QMessageBox>
#include <QLineEdit>
#include <QException>

MainWin::MainWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);
    initSerial();
    initCallBack();
    initView();

    mPassCountA=0;
    mNgCountA=0;
    mPassCountB=0;
    mNgCountB=0;
    mTimerA=new QTimer(this);
    mTimerB=new QTimer(this);
    connect(mTimerA,&QTimer::timeout,this,&MainWin::onTimerA);
    connect(mTimerB,&QTimer::timeout,this,&MainWin::onTimerB);

    mMesManagerA=new MesManager(this);
    mMesManagerB=new MesManager(this);
    connect(mMesManagerA,&MesManager::resSingal,this,&MainWin::onMesReplyA);
    connect(mMesManagerB,&MesManager::resSingal,this,&MainWin::onMesReplyB);

    connect(ui->btn_lock,&QPushButton::clicked,this,&MainWin::onBtnClicked);
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
    qDebug()<<"screen width:"<<mRect.width()<<"screen height:"<<mRect.height();
    //设置初始化尺寸及位置
    this->setGeometry(0.1*mRect.width(),0.1*mRect.height(),0.8*mRect.width(),0.8*mRect.height());
    //标题设置
    this->setWindowTitle(Constants::APP_NAME);
    ui->lineEdit_gongjuNu_A->setFocus();

    //显示样式确定控件
    ui->label_successNum_input->setStyleSheet(Constants::SUCCESS_STYLE);
    ui->label_failNum_input->setStyleSheet(Constants::WARNNING_STYLE);

    //读取配置文件
    QString path = QDir::currentPath() + "/config.txt";
    qDebug()<<"path:"<<path;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,"错误","无法打开配置文件!");
        return;
    }
    QString titleString="";
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        str=str.trimmed();
        QStringList lst=str.split(":");
        if(lst.length()==2)
        {
            if(lst[0]=="ProductStyle") ui->label_model_input->setText(lst[1]);
            if(lst[0]=="TastMsg") ui->label_taskPw_input->setText(lst[1]);
            if(lst[0]=="WorkstationName") ui->label_stationName_input->setText(lst[1]);
            if(lst[0]=="SampleTestPress"){
                titleString+=QString("压力 %1kPA").arg(lst[1]);
                mStandandPress=lst[1].toFloat();
            }
            if(lst[0]=="SampleTestPressOffset"){
                titleString+=QString("±%1kPA, ").arg(lst[1]);
            }
            if(lst[0]=="LeakMax"){
                titleString+=QString("泄漏上限 %1PA, ").arg(lst[1]);
                mStandandLeakValue=lst[1].toFloat();
            }
            if(lst[0]=="leakMin"){
            }
            if(lst[0]=="GasChargingTime"){
                titleString+=QString("充气时间%1, ").arg(lst[1]);
            }
            if(lst[0]=="PressStableTime"){
                titleString+=QString("稳压时间%1, ").arg(lst[1]);
            }
            if(lst[0]=="TestTime"){
                titleString+=QString("测试时间%1").arg(lst[1]);
            }
            if(lst[0]=="PassNumber"){
                ui->label_successNum_input->setText(lst[1]);//?合格数和失败数是否要存储到文件？
            }
            if(lst[0]=="NGNumber"){
                ui->label_failNum_input->setText(lst[1]);//?合格数和失败数是否要存储到文件？
            }
        }
    }
    ui->label_sampleIdx_input->setText(titleString);
}

void MainWin::initSerial()
{
    mAllSerials=SerialHelper::findAllSerials();
    if(!mAllSerials.isEmpty())
    {
        qDebug()<<"ALL Serials:"<<mAllSerials;
        mManagerPtrA=new GasTightManager(this,mAllSerials.at(0).toStdString());
        ui->label_serialNo_input_A->setText(mAllSerials.at(0));
        bool ret=mManagerPtrA->setGasTightEnable();
        if(ret)
        {
            qDebug()<<"serial "<<mAllSerials.at(0)<<" open success";
            ui->label_serialStatus_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_serialStatus_input_A->setText(Constants::SERIAL_SUCCESS);
        }
        else
        {
            qDebug()<<"serial "<<mAllSerials.at(0)<<" open fail";
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
                qDebug()<<"serial "<<mAllSerials.at(1)<<" open success";
                ui->label_serialStatus_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_serialStatus_input_B->setText(Constants::SERIAL_SUCCESS);
            }
            else
            {
                qDebug()<<"serial "<<mAllSerials.at(0)<<" open fail";
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
    mCallBackPtrA->mProgressTimeCallBack = std::bind(&MainWin::progressTimeCB,this,placeholders::_1,placeholders::_2,placeholders::_3,placeholders::_4);
    if(mManagerPtrA)
    {
        qDebug()<<"init call back A";
        qDebug()<<"use count of shared ptr callback A before:"<<mCallBackPtrA.use_count();
        mManagerPtrA->registerCallBack(mCallBackPtrA);
        qDebug()<<"use count of shared ptr callback A after:"<<mCallBackPtrA.use_count();
    }
    if(mManagerPtrB)
    {
        qDebug()<<"init call back A";
        mManagerPtrB->registerCallBack(mCallBackPtrB);
    }
}

void MainWin::testStateCB(const std::string callBackFlag,const TestState &state)
{
    MainHandler::getInstance()->runOnMainThread([callBackFlag,state,this](){
                this->testStateHandler(callBackFlag,state);
    });
}

void MainWin::progressCB(const std::string callBackFlag,const float &progress)
{
    MainHandler::getInstance()->runOnMainThread([callBackFlag,progress,this](){
                this->progressHandler(callBackFlag,progress);
    });
}

void MainWin::dataCB(const std::string callBackFlag,const DataType type,const int &data)
{
    MainHandler::getInstance()->runOnMainThread([callBackFlag,type,data,this](){
                this->dataHandler(callBackFlag,type,data);
    });
}

void MainWin::resultCB(const std::string callBackFlag,const ResultType type)
{
    MainHandler::getInstance()->runOnMainThread([callBackFlag,type,this](){
                this->resultHandler(callBackFlag,type);
    });
}

void MainWin::progressTimeCB(const std::string callBackFlag,const float &pinflate_time, const float &pholding_time, const float &ptest_time)
{
    MainHandler::getInstance()->runOnMainThread([callBackFlag,pinflate_time,pholding_time,ptest_time,this](){
                this->progressTimeHandler(callBackFlag,pinflate_time,pholding_time,ptest_time);
    });
}

void MainWin::testStateHandler(const std::string callBackFlag,const TestState &state)
{
    if(callBackFlag=="A")
    {
        if(state==TestState::TESTING)
        {
            ui->label_hintInfo_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_hintInfo_input_A->setText(Constants::HINTINFO_ALREADY_START);
            mManagerPtrA->readTestProgress();
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
            mManagerPtrB->readTestProgress();
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

void MainWin::progressHandler(const std::string callBackFlag,const float &progress)
{
    if(callBackFlag=="A")
    {
        if(progress>0)
        {
           ui->progressBar_A->setValue(int(progress*100));
        }
    }
    else if(callBackFlag=="B")
    {
        if(progress>0)
        {
           ui->progressBar_B->setValue(int(progress*100));
        }
    }
}

void MainWin::dataHandler(const std::string callBackFlag,const DataType type,const int &data)
{
    if(callBackFlag=="A")
    {
        switch (type) {
        case DataType::CURRENT_PRESS_VALUE:
            if(data>0)
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_pressOffset_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                float pressValue=data/1000;
                QString pressContext=QString("%1kpa").arg(pressValue);
                ui->label_testPress_input_A->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_A->setText(offsetContext);
            }
            else
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_testPress_input_A->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::TEST_PRESS_VALUE:
            if(data>0)
            {
                ui->label_testPress_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_pressOffset_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float pressValue=data/1000;
                QString pressContext=QString("%1kpa").arg(pressValue);
                ui->label_testPress_input_A->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100;
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
            if(data>0)
            {
                ui->label_leak_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_leakOffset_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakValue=data/1000;
                QString leakContext=QString("%1%").arg(leakValue);
                ui->label_leak_input_A->setText(leakContext);
                float offsetValue=data/mStandandLeakValue*100;
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
            if(data>0)
            {
                ui->label_leakSpeed_input_A->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakSpeedValue=data/1000;
                QString leakSpeedContext=QString("%1sccm").arg(leakSpeedValue);
                ui->label_leakSpeed_input_A->setText(leakSpeedContext);
            }
            else
            {
                ui->label_leak_input_A->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leak_input_A->setText(Constants::LEAKSPEED_READ_FAIL);
            }
            break;
        default:
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
                float pressValue=data/1000;
                QString pressContext=QString("%1kpa").arg(pressValue);
                ui->label_testPress_input_B->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100;
                QString offsetContext=QString("%1%").arg(offsetValue);
                ui->label_pressOffset_input_B->setText(offsetContext);
            }
            else
            {
                ui->label_testPress_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_testPress_input_B->setText(Constants::PRESS_READ_FAIL);
            }
            break;
        case DataType::TEST_PRESS_VALUE:
            if(data>0)
            {
                ui->label_testPress_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_pressOffset_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float pressValue=data/1000;
                QString pressContext=QString("%1kpa").arg(pressValue);
                ui->label_testPress_input_B->setText(pressContext);
                float offsetValue=(pressValue-mStandandPress)/mStandandPress*100;
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
            if(data>0)
            {
                ui->label_leak_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                ui->label_leakOffset_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakValue=data/1000;
                QString leakContext=QString("%1%").arg(leakValue);
                ui->label_leak_input_B->setText(leakContext);
                float offsetValue=data/mStandandLeakValue*100;
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
            if(data>0)
            {
                ui->label_leakSpeed_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
                float leakSpeedValue=data/1000;
                QString leakSpeedContext=QString("%1sccm").arg(leakSpeedValue);
                ui->label_leakSpeed_input_B->setText(leakSpeedContext);
            }
            else
            {
                ui->label_leak_input_B->setStyleSheet(Constants::WARNNING_STYLE);
                ui->label_leak_input_B->setText(Constants::LEAK_READ_FAIL);
            }
            break;
        default:
            break;
        }
    }
}

void MainWin::resultHandler(const std::string callBackFlag,const ResultType type)
{
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
            mPassCountA++;
            ui->label_successNum_input->setText(QString("%1").arg(mPassCountA+mPassCountB));
            //逻辑处理
            //to do:1.异步读取测试过程各阶段耗时 2.调用接口上传MES
            mManagerPtrA->readProgressTime();
            mSn=ui->lineEdit_SN_A->text();
            mProdNo=ui->lineEdit_prodNo->text();
            mStationNo=ui->lineEdit_stationNo->text();
            mResult="PASS";
            mMesManagerA->post2Mes(Constants::MES_URL,mSn,mProdNo,mStationNo,mResult);
            break;
        case ResultType::RESULT_NG:
            //设置UI
            ui->label_testResult_input_A->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testResult_input_A->setText("NG");
            mNgCountA++;
            ui->label_failNum_input->setText(QString("%1").arg(mNgCountA+mNgCountB));
            ui->label_testFailNum_A->setText(QString("%1").arg(mNgCountA));
            //逻辑处理
            //to do:1.异步读取测试过程各阶段耗时 2.调用接口上传MES
            mManagerPtrA->readProgressTime();
            mSn=ui->lineEdit_SN_A->text();
            mProdNo=ui->lineEdit_prodNo->text();
            mStationNo=ui->lineEdit_stationNo->text();
            mResult="FAIL";
            mMesManagerA->post2Mes(Constants::MES_URL,mSn,mProdNo,mStationNo,mResult);
            break;
        case ResultType::RESULT_READ_FAIL:
            break;
        default:
            break;
        }
    }
    else
    {
        switch (type) {
        case ResultType::JIGU_INSTALL:
            ui->label_progressInfo_B->setText("夹具安装");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::JINQI:
            ui->label_progressInfo_B->setText("进气");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::WENYA:
            ui->label_progressInfo_B->setText("稳压");
            mManagerPtrA->readCurrentPressData();
            break;
        case ResultType::TEST:
            ui->label_progressInfo_B->setText("测试");
            break;
        case ResultType::RESULT_OK:
            ui->label_testResult_input_B->setStyleSheet(Constants::SUCCESS_STYLE);
            ui->label_testResult_input_B->setText("OK");
            mPassCountB++;
            ui->label_successNum_input->setText(QString("%1").arg(mPassCountA+mPassCountB));
            //逻辑处理
            //to do:1.读取测试过程各阶段耗时 2.调用接口上传MES
            mManagerPtrB->readProgressTime();
            mSn=ui->lineEdit_SN_B->text();
            mProdNo=ui->lineEdit_prodNo->text();
            mStationNo=ui->lineEdit_stationNo->text();
            mResult="PASS";
            mMesManagerB->post2Mes(Constants::MES_URL,mSn,mProdNo,mStationNo,mResult);
            break;
        case ResultType::RESULT_NG:
            ui->label_testResult_input_B->setStyleSheet(Constants::WARNNING_STYLE);
            ui->label_testResult_input_B->setText("NG");
            mNgCountA++;
            ui->label_failNum_input->setText(QString("%1").arg(mNgCountA+mNgCountB));
            ui->label_testFailNum_B->setText(QString("%1").arg(mNgCountA));
            //逻辑处理
            //to do:1.读取测试过程各阶段耗时 2.调用接口上传MES
            mManagerPtrB->readProgressTime();
            mSn=ui->lineEdit_SN_B->text();
            mProdNo=ui->lineEdit_prodNo->text();
            mStationNo=ui->lineEdit_stationNo->text();
            mResult="FAIL";
            mMesManagerB->post2Mes(Constants::MES_URL,mSn,mProdNo,mStationNo,mResult);
            break;
        case ResultType::RESULT_READ_FAIL:
            break;
        default:
            break;
        }
    }
}

void MainWin::progressTimeHandler(const std::string callBackFlag,const float &pinflate_time,const float &pholding_time,const float &ptest_time)
{
    if(pinflate_time==0) return;
    QString result=QString("充气%1秒,稳压%2秒,测试%3秒").arg(pinflate_time).arg(pholding_time).arg(ptest_time);
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

void MainWin::onMesReplyA(bool result,QString errMsg)
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
        ui->label_hintInfo_input_A->setText(errMsg);
    }
    record2File("A",result,errMsg);
    reset("A");//结果上传Mes及保存至本地日志系统之后,将界面复位
}

void MainWin::onMesReplyB(bool result,QString errMsg)
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
        ui->label_hintInfo_input_B->setText(errMsg);
    }
    record2File("B",result,errMsg);
    reset("B");//结果上传Mes及保存至本地日志系统之后,将界面复位
}

void MainWin::onBtnClicked()
{
    if(ui->btn_lock->text()=="锁定")
    {
        if(ui->lineEdit_prodNo->text().length()==0||ui->lineEdit_stationNo->text().length()==0)
        {
            QMessageBox::critical(this,"警告","输入信息不全!");
        }
        ui->btn_lock->setText("解锁");
        mProdNo=ui->lineEdit_prodNo->text();
        mStationNo=ui->lineEdit_stationNo->text();
        QString mStationNo;
        ui->lineEdit_prodNo->setEnabled(false);
        ui->lineEdit_stationNo->setEnabled(false);
        ui->lineEdit_gongjuNu_A->setFocus();
    }
    else if(ui->btn_lock->text()=="解锁")
    {
        ui->btn_lock->setText("锁定");
        ui->lineEdit_prodNo->setEnabled(true);
        ui->lineEdit_stationNo->setEnabled(true);
    }
}

//处理扫码枪输入
void MainWin::keyPressEvent(QKeyEvent * evt)
{
    if(evt->key() == Qt::Key_Return)//enter
    {
        qDebug()<<"Enter inputed";
        evt->accept();
        onEnterInputed();
    }
}

void MainWin::onEnterInputed()
{
    QWidget *currentFocusWidget = this->focusWidget();
    if(currentFocusWidget==ui->lineEdit_gongjuNu_A&&ui->lineEdit_gongjuNu_A->text().length()>0)
    {
        ui->lineEdit_SN_A->setFocus();
        return;
    }
    if(currentFocusWidget==ui->lineEdit_SN_A&&ui->lineEdit_SN_A->text().length()>0)
    {

        //设置UI
        ui->label_testStatus_input_A->setText(Constants::TEST_STATUS_TESTING);
        ui->label_hintInfo_input_A->setStyleSheet(Constants::WARNNING_STYLE);
        ui->label_hintInfo_input_A->setText(Constants::HINTINFO_START);
        ui->lineEdit_gongjuNu_B->setFocus();

//mes 连通性测试代码
#ifdef  MES_TEST
        mSn=ui->lineEdit_SN_A->text();
        mProdNo=ui->lineEdit_prodNo->text();
        mStationNo=ui->lineEdit_stationNo->text();
        mResult="PASS";
        mMesManagerA->post2Mes(Constants::MES_URL,mSn,mProdNo,mStationNo,mResult);
#endif

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
        return;
    }
    if(currentFocusWidget==ui->lineEdit_gongjuNu_B&&ui->lineEdit_gongjuNu_B->text().length()>0)
    {
        ui->lineEdit_SN_B->setFocus();
        return;
    }
    if(currentFocusWidget==ui->lineEdit_SN_B&&ui->lineEdit_SN_B->text().length()>0)
    {
        //设置UI
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
        ui->lineEdit_SN_A->clear();
        ui->lineEdit_gongjuNu_A->clear();
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
        ui->lineEdit_SN_B->clear();
        ui->lineEdit_gongjuNu_B->clear();
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
