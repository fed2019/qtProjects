#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QString>
#pragma execution_character_set("utf-8")

class Constants
{
public:


    const static QString MES_URL;

    //UI显示样式
    static const QString WARNNING_STYLE;
    static const QString SUCCESS_STYLE;
    static const QString COMMON_STYLE;

    //UI显示信息
    const static QString APP_NAME;
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
};

#endif // CONSTANTS_H
