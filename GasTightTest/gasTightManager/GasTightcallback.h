#ifndef GASTIGHTCALLBACK_H
#define GASTIGHTCALLBACK_H
#include <functional>
#include <string>

using namespace std;

enum TestState : int;
enum DataType : int;
enum ResultType : int;

class GasTightCallBack
{
public:
    GasTightCallBack(std::string flag="A")
    {
        callBackFlag=flag;
    }
//    GasTightCallBack(const GasTightCallBack &callback)
//    {
//        callBackFlag=callback.callBackFlag;
//    }
    function<void (const std::string,TestState &)> mTestStateCallBack=[](const std::string &,TestState &){};
    function<void (const std::string,float &)> mProgressCallBack=[](const std::string &,float &){};
    function<void (const std::string,DataType,int &)> mDataCallBack=[](const std::string &,DataType,int &){};
    function<void (const std::string,ResultType)> mResultCallBack=[](const std::string &,ResultType){};
    function<void (const std::string,const float & ,const float & ,const float &)> mProgressTimeCallBack=[](const std::string,const float & ,const float & ,const float &){};
    std::string callBackFlag;
};

#endif // GASTIGHTCALLBACK_H
