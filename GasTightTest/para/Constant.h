#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QString>
#pragma execution_character_set("utf-8")

class Constants
{
public:


    const static QString MES_URL;
    const static QString MES_CREATEROUTE;
    const static QString MES_CHECKROUTE;

    //UI显示样式
    static const QString WARNNING_STYLE;
    static const QString SUCCESS_STYLE;
    static const QString COMMON_STYLE;

    //UI显示信息
    const static QString APP_NAME;
    const static QString SUBWIN_TITLE;
    const static QString SERIAL_SUCCESS;
    const static QString SERIAL_FAIL;
    const static QString TEST_STATUS_TESTING;
    const static QString HINTINFO_START;
    const static QString HINTINFO_ALREADY_START;
    const static QString RESULT_OK;
    const static QString RESULT_NG;
    const static QString RESULT_FAIL;
    const static QString PRESS_READ_FAIL;
    const static QString LEAK_READ_FAIL;
    const static QString LEAKSPEED_READ_FAIL;
    const static QString JIAJU_MISTAKE;
    const static QString HINTINFO_SN_NOTFIND;
    const static QString HINTINFO_SN_ALREADYPASS;
    const static QString HINTINFO_SN_UNKNOWERR;
    const static QString HINTINFO_SN_SCANMISTAKE;
    const static QString HINTINFO_MES_MISTAKE;
    const static QString HINTINFO_MES_NORESPONSE;

    //对话框提示信息
    const static QString MSG_TITLE;
    const static QString MSG_CONTEXT;
};

#endif // CONSTANTS_H
