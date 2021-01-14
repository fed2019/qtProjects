#include "Constant.h"

const QString Constants::MES_URL = QString("http://192.168.16.114:8080");
const QString Constants::MES_CREATEROUTE = QString("/mrs/createRoute");
const QString Constants::MES_CHECKROUTE = QString("/mrs/checkRoute");

const QString Constants::WARNNING_STYLE = "color:red;";
const QString Constants::SUCCESS_STYLE = "color:green;";
const QString Constants::COMMON_STYLE = "color:black;";

const QString Constants::APP_NAME="科大讯飞生产测试-气密性测试工站";
const QString Constants::SUBWIN_TITLE="设置-属性设置";
const QString Constants::SERIAL_SUCCESS="开启成功";
const QString Constants::SERIAL_FAIL="开启失败";
const QString Constants::TEST_STATUS_TESTING="测试中";
const QString Constants::HINTINFO_START="请启动测试仪";
const QString Constants::HINTINFO_ALREADY_START="测试仪已启动测试";
const QString Constants::RESULT_OK="OK";
const QString Constants::RESULT_NG="NG";
const QString Constants::RESULT_FAIL="失败";
const QString Constants::PRESS_READ_FAIL="压力值读取失败";
const QString Constants::LEAK_READ_FAIL="泄漏值读取失败";
const QString Constants::LEAKSPEED_READ_FAIL="泄漏速度读取失败";
const QString Constants::JIAJU_MISTAKE="夹具号错误,请重新扫码";
const QString Constants::HINTINFO_SN_NOTFIND="上工序未过站";
const QString Constants::HINTINFO_SN_ALREADYPASS="此工序已过站";
const QString Constants::HINTINFO_SN_UNKNOWERR="checkRoute时出现未知错误,请确认MES系统信息";
const QString Constants::HINTINFO_SN_SCANMISTAKE="SN号扫描出错,请重新扫描";
const QString Constants::HINTINFO_MES_MISTAKE="上传MES失败,请通过本地日志查看具体信息";
const QString Constants::HINTINFO_MES_NORESPONSE="未收到MES系统响应,请检查网络";

//对话框提示信息
const QString Constants::MSG_TITLE="提示";
const QString Constants::MSG_CONTEXT="请将必填信息填写完整";
