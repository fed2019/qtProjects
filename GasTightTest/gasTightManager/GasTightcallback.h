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
    function<void (const std::string,int &)> mProgressCallBack=[](const std::string &,int &){};
    function<void (const std::string,DataType,int &)> mDataCallBack=[](const std::string &,DataType,int &){};
    function<void (const std::string,ResultType)> mResultCallBack=[](const std::string &,ResultType){};
    function<void (const std::string,const int & ,const int & ,const int &)> mProgressTimeCallBack=[](const std::string,const int & ,const int & ,const int &){};
    std::string callBackFlag;
};

#endif // GASTIGHTCALLBACK_H
