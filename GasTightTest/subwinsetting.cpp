#include "subwinsetting.h"
#include "ui_subwinsetting.h"
#include <QDir>
#include <QDebug>
#include "para/Constant.h"
#include <QMessageBox>
#pragma execution_character_set("utf-8")

#define LOG qDebug()<<"SubWinSetting:"
SubWinSetting::SubWinSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubWinSetting)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowMinimizeButtonHint);
    initView();
    connect(ui->btn_confirm,&QPushButton::clicked,this,&SubWinSetting::onBtnConfirmClicked);
    connect(ui->btn_cancel,&QPushButton::clicked,this,&SubWinSetting::onBtnCancelClicked);
}

void SubWinSetting::closeEvent(QCloseEvent *evt)
{
    Q_UNUSED(evt);
    //写文件
    QString path = QDir::currentPath() + "/config.txt";
    //qDebug()<<"path:"<<path;
    QFile outFile(path);
    bool ret=outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    if(ret)
    {
        QStringList lst;
        QString str1=QString("prodNo:%1\n").arg(ui->lineEdit_prodNo->text());
        lst.append(str1);
        QString str2=QString("stationNo:%1\n").arg(ui->lineEdit_stationNo->text());
        lst.append(str2);
        QString str3=QString("path1ToolNo:%1\n").arg(ui->lineEdit_path1ToolNo->text());
        lst.append(str3);
        QString str4=QString("path2ToolNo:%1\n").arg(ui->lineEdit_path2ToolNo->text());
        lst.append(str4);
        QString str5=QString("ProductModel:%1\n").arg(ui->lineEdit_model->text());
        lst.append(str5);
        QString str6=QString("WorkstationName:%1\n").arg(ui->lineEdit_stationName->text());
        lst.append(str6);
        QString str7=QString("SampleTestPress:%1\n").arg(ui->lineEdit_testPress->text());
        lst.append(str7);
        QString str8=QString("SampleTestPressOffset:%1\n").arg(ui->lineEdit_testPressOffset->text());
        lst.append(str8);
        QString str9=QString("LeakMax:%1\n").arg(ui->lineEdit_leakMax->text());
        lst.append(str9);
        QString str10=QString("GasChargingTime:%1\n").arg(ui->lineEdit_chongqiTime->text());
        lst.append(str10);
        QString str11=QString("PressStableTime:%1\n").arg(ui->lineEdit_wenyaTime->text());
        lst.append(str11);
        QString str12=QString("TestTime:%1").arg(ui->lineEdit_testTime->text());
        lst.append(str12);
        for(QString &s:lst)
        {
            outFile.write(s.toUtf8());
        }
    }
    outFile.close();
}

SubWinSetting::~SubWinSetting()
{
    delete ui;
}

void SubWinSetting::initView()
{
    this->setWindowTitle(Constants::SUBWIN_TITLE);
    //读取配置文件
    QString path = QDir::currentPath() + "/config.txt";
    //qDebug()<<"path:"<<path;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    //QString titleString="";
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        str=str.trimmed();
        QStringList lst=str.split(":");
        if(lst.length()==2)
        {
            if(lst[0]=="prodNo") ui->lineEdit_prodNo->setText(lst[1]);
            if(lst[0]=="stationNo") ui->lineEdit_stationNo->setText(lst[1]);
            if(lst[0]=="path1ToolNo") ui->lineEdit_path1ToolNo->setText(lst[1]);
            if(lst[0]=="path2ToolNo") ui->lineEdit_path2ToolNo->setText(lst[1]);
            if(lst[0]=="ProductModel") ui->lineEdit_model->setText(lst[1]);
            if(lst[0]=="WorkstationName") ui->lineEdit_stationName->setText(lst[1]);
            if(lst[0]=="SampleTestPress"){
                ui->lineEdit_testPress->setText(lst[1]);
            }
            if(lst[0]=="SampleTestPressOffset"){
                ui->lineEdit_testPressOffset->setText(lst[1]);
            }
            if(lst[0]=="LeakMax"){
                ui->lineEdit_leakMax->setText(lst[1]);
            }
            if(lst[0]=="leakMin"){
            }
            if(lst[0]=="GasChargingTime"){
                ui->lineEdit_chongqiTime->setText(lst[1]);
            }
            if(lst[0]=="PressStableTime"){
                ui->lineEdit_wenyaTime->setText(lst[1]);
                //titleString+=QString("稳压时间%1, ").arg(lst[1]);
            }
            if(lst[0]=="TestTime"){
                ui->lineEdit_testTime->setText(lst[1]);
            }
        }
    }
}

QStringList SubWinSetting::getNessaryInfo()
{
    QStringList mNessaryInfo;
    QString prodNo=ui->lineEdit_prodNo->text();
    mNessaryInfo.append(prodNo);
    QString stationNo=ui->lineEdit_stationNo->text();
    mNessaryInfo.append(stationNo);
    QString path1ToolNo=ui->lineEdit_path1ToolNo->text();
    mNessaryInfo.append(path1ToolNo);
    QString path2ToolNo=ui->lineEdit_path2ToolNo->text();
    mNessaryInfo.append(path2ToolNo);
    return mNessaryInfo;
}

QStringList SubWinSetting::getUnNessaryInfo()
{
    QStringList mUnNessaryInfo;
    QString model=ui->lineEdit_model->text();
    mUnNessaryInfo.append(model);
    QString stationName=ui->lineEdit_stationName->text();
    mUnNessaryInfo.append(stationName);
    QString testPress=ui->lineEdit_testPress->text();
    mUnNessaryInfo.append(testPress);
    QString testPressOffset=ui->lineEdit_testPressOffset->text();
    mUnNessaryInfo.append(testPressOffset);
    QString leakMax=ui->lineEdit_leakMax->text();
    mUnNessaryInfo.append(leakMax);
    QString chongqiTime=ui->lineEdit_chongqiTime->text();
    mUnNessaryInfo.append(chongqiTime);
    QString wenyaTime=ui->lineEdit_wenyaTime->text();
    mUnNessaryInfo.append(wenyaTime);
    QString testTime=ui->lineEdit_testTime->text();
    mUnNessaryInfo.append(testTime);
    return mUnNessaryInfo;
}

void SubWinSetting::onBtnConfirmClicked()
{
    bool isInfoNotEngouth=(ui->lineEdit_prodNo->text().length()==0||ui->lineEdit_stationNo->text().length()==0||\
            ui->lineEdit_path1ToolNo->text().length()==0||ui->lineEdit_path2ToolNo->text().length()==0);
    if(isInfoNotEngouth)
    {
        QMessageBox::critical(this,Constants::MSG_TITLE,Constants::MSG_CONTEXT);
        return;
    }
    emit confirmClicked();
    this->hide();
}

void SubWinSetting::onBtnCancelClicked()
{
    this->hide();
}
