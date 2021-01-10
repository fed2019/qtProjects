#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "serial.h"


#define COUNT(arr) ((int)(sizeof(arr)/sizeof(arr[0])))
#define av_gettime_relative() GetTickCount64() * 1000


/***
 * 串口操作分以下4步：
 （1） 打开串口
 （2） 配置串口
 （3） 读写串口
 （4） 关闭串口
 */
struct _swft_serial_t
{
	HANDLE hFile;	//串口通过API函数CreateFile来打开
	HANDLE hEventR;	//异步读时需要的信号量
	HANDLE hEventW;	//异步写时需要的信号量
	BOOL  async;
};


/***
 *   打开串口
 * @param serial_name: 串口名，例如 "COM1"
 * @param async: 同步方式还是异步方式, 0 = 同步方式， 1 = 异步方式
 * @return: 串口对象指针。 NULL 表示打开失败
 */
swft_serial_t* swft_serial_open(const char* serial_name, BOOL async)
{
	swft_serial_t *serial = NULL;
	serial = (swft_serial_t*)malloc(sizeof(swft_serial_t));
	if (serial == NULL)
		return NULL;
	memset(serial, 0, sizeof(swft_serial_t));

    //char* 转LPCWSTR 旧
//    TCHAR szCom[64];
//    int n, errpos;
//	n = 0;
//	if (!strnicmp(serial_name, "COM", 3) && atoi(serial_name + 3) >= 10)
//	{
//        n += UTF8_Unicode_str("\\\\.\\", (unsigned short*)szCom, COUNT(szCom), &errpos);
//	}
    //UTF8_Unicode_str(serial_name, (unsigned short*)(szCom + n), COUNT(szCom) - n, &errpos);

    //char* 转LPCWSTR新
    WCHAR sName[64];
    memset(sName,0,sizeof(sName));
    MultiByteToWideChar(CP_ACP,0,serial_name,strlen(serial_name)+1,sName,
        sizeof(sName)/sizeof(sName[0]));

    if (async)
	{
        serial->hFile = CreateFile(sName,//COM#口 ; 注意串口号如果大于COM9应该在前面加上\\.\，比如COM10表示为"\\\\.\\COM10"
			GENERIC_READ | GENERIC_WRITE, //允许读和写  
			0, //独占方式  
			NULL,
            OPEN_EXISTING, //打开而不是创建
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, //0同步方式 ; FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED异步方式
			NULL);
		if (serial->hFile == INVALID_HANDLE_VALUE)
			goto ERR_EXIT;
		serial->hEventR = CreateEvent(NULL, TRUE, FALSE, NULL);
		serial->hEventW = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	else
	{
        serial->hFile = CreateFile(sName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (serial->hFile == INVALID_HANDLE_VALUE)
			goto ERR_EXIT;
	}
	serial->async = async;

	return serial;

ERR_EXIT:
	swft_serial_close(serial);
	return NULL;
}



/***
 *   关闭串口
 * @param serial: 串口对象句柄
 */
void swft_serial_close(swft_serial_t* serial)
{
	if (serial == NULL)
		return;
	if (serial->hFile != INVALID_HANDLE_VALUE)
		CloseHandle(serial->hFile);
	if (serial->hEventR)
		CloseHandle(serial->hEventR);
	if (serial->hEventW)
		CloseHandle(serial->hEventW);
	free(serial);
}


/***
 * 配置串口
 @param serial: 串口对象指针
 @param DWORD  baudrate;	//波特率，指定通信设备的传输速率。这个成员可以是实际波特率值或者常量值
 @param fParity; // 指定奇偶校验使能。若此成员为1，允许奇偶校验检查
 @param Parity; //指定奇偶校验方法。此成员可以有下列值： EVENPARITY 偶校验；NOPARITY 无校验；MARKPARITY 标记校验；ODDPARITY 奇校验，-1表示禁止奇偶校验
 @param ByteSize: //通信字节位数，4—8
 @param StopBits; //指定停止位的位数。此成员可以有下列值： ONESTOPBIT 1位停止位；TWOSTOPBITS 2位停止位；ONE5STOPBITS 1.5位停止位
 @return: 0 = 成功， -1 = 失败
 */
int swft_serial_config(swft_serial_t* serial, DWORD baudrate, int parity, BYTE ByteSize, BYTE stopbits)
{
    BOOL ret;
	DCB dcb;
	if (serial == NULL)
		return -1;
	ret = SetupComm(serial->hFile, 1024, 1024);
	if (!ret) //输入缓冲区和输出缓冲区的大小都是1024
		return -1;

	COMMTIMEOUTS TimeOuts;
	GetCommTimeouts(serial->hFile, &TimeOuts);
	TimeOuts.ReadIntervalTimeout = 5; //设定读超时，读取两个字符之间的超时
	TimeOuts.ReadTotalTimeoutMultiplier = 0;	//字符超时倍数，也就是字符数乘以此值得到一个超时时间。
	TimeOuts.ReadTotalTimeoutConstant = 10;	//读总超时时间， ReadTotalTimeoutMultiplier×<字符数>＋ReadTotalTimeoutConstant 是真正的总超时时间
	TimeOuts.WriteTotalTimeoutMultiplier = 2; //设定写超时 
	TimeOuts.WriteTotalTimeoutConstant = 50;
	ret = SetCommTimeouts(serial->hFile, &TimeOuts);
	if (!ret) //设置超时
		return -1;
	ret = GetCommState(serial->hFile, &dcb);
	if (!ret)		//读取配置参数
		return -1;
	dcb.BaudRate = baudrate; //波特率
	dcb.ByteSize = 8;		//每个字节有8位 
	dcb.fParity = parity >= 0;	//是否奇偶校验
	dcb.Parity = parity >= 0 ? parity : NOPARITY;	//奇偶校验方式，EVENPARITY 偶校验；NOPARITY 无校验；MARKPARITY 标记校验；ODDPARITY 奇校验
	dcb.StopBits = 0; //两个停止位，ONESTOPBIT 1位停止位；TWOSTOPBITS 2位停止位；ONE5STOPBITS 1.5位停止位
	ret = SetCommState(serial->hFile, &dcb);
	if (!ret)		//设定配置参数
		return -1;
	ret = PurgeComm(serial->hFile, PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (!ret)		//清空缓冲区，PURGE_TXABORT 中断所有写操作并立即返回，即使写操作还没有完成；PURGE_RXABORT中断所有读操作并立即返回，即使读操作还没有完成；PURGE_TXCLEAR 清除输出缓冲区；PURGE_RXCLEAR 清除输入缓冲区；
		return -1;
	return 0;
}


/***
 *   读串口，可能会阻塞
 * @param serial: 串口对象指针
 * @param buf: [out] 读缓冲区
 * @param size: 缓冲区 buf 的长度
 * @return: >=0：读取到的字节数， -1：读取失败
 */
static int serial_read_1(swft_serial_t* serial, void* buf, int size)
{
	DWORD len = 0;
	if (serial == NULL)
		return -1;

	DWORD dwErrorFlags;
	COMSTAT ComStat;
	if (!ClearCommError(serial->hFile, &dwErrorFlags, &ComStat))
		return -1;
	if (serial->async)
	{
		//异步读
		OVERLAPPED ovl;
		memset(&ovl, 0, sizeof(OVERLAPPED));	//注意每次读取串口时都要初始化OVERLAPPED
		ovl.hEvent = serial->hEventR;
		BOOL b = ReadFile(serial->hFile, buf, size, &len, &ovl);
		if (!b)
		{
			//如果ReadFile函数返回FALSE，并且 GetLastError() 的返回值为 ERROR_IO_PENDING，就表示重叠操作还没有完成，需要调用 GetOverlappedResult() 等待
			if (GetLastError() == ERROR_IO_PENDING)
			{
				b = GetOverlappedResult(serial->hFile, &ovl, &len, TRUE); // GetOverlappedResult函数的最后一个参数设为TRUE，函数会一直
																		  //等待，直到读操作完成或由于错误而返回。 
			}
			if (!b)
				return -1;
		}
		//PurgeComm(serial->hFile, PURGE_RXCLEAR);
	}
	else
	{
		//同步读
		BOOL b = ReadFile(serial->hFile, buf, size, &len, NULL);
		if (!b)
			return -1;
	}
	return (int)len;
}


/***
 *   读串口，可能会阻塞
 * @param serial: 串口对象指针
 * @param buf: [out] 读缓冲区
 * @param size: 缓冲区 buf 的长度
 * @param timeout_ms: 读超时，单位 ms
 * @return: >=0：读取到的字节数， -1：读取失败
 */
int swft_serial_read(swft_serial_t* serial, void* buf, int size, int timeout_ms)
{
	int len = 0;
    long long int t1 = av_gettime_relative();
	while (1)
	{
		len = serial_read_1(serial, (char*)buf + len, size - len);
		if (len != 0 || (av_gettime_relative() - t1) / 1000 >= timeout_ms)
			break;
	}
    return len;
}



/***
 *   写串口
 * @param serial: 串口对象指针
 * @param buf: 写缓冲区
 * @param size: buf 中的数据长度
 * @return: >=0： 写入的字节数， -1： 写入失败
 */
int swft_serial_write(swft_serial_t* serial, const void* buf, int size)
{
	DWORD len = 0;
	if (serial == NULL)
		return -1;
	PurgeComm(serial->hFile, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (serial->async)
	{
		//异步写
		OVERLAPPED ovl;
		memset(&ovl, 0, sizeof(OVERLAPPED));	//注意每次读取串口时都要初始化OVERLAPPED
		ovl.hEvent = serial->hEventW;
		BOOL b = WriteFile(serial->hFile, buf, size, &len, &ovl);
		if (!b)
		{
			DWORD errcode = GetLastError();
			if (errcode == ERROR_IO_PENDING)
			{
				if (WaitForSingleObject(ovl.hEvent, 1000) != WAIT_OBJECT_0)
					return -1;
				len = size;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		//同步写
		BOOL b = WriteFile(serial->hFile, buf, size, &len, NULL);
		if (!b)
			return -1;
	}
	return (int)len;
}



/***
 *   枚举可用串口
 * @param flags: [out] 串口是否存在的标识数组，数组中每一元素代表一个串口是否存在的标志
 * @param flag_count: 数组 flags 的长度
 * @return: 找到的可用串口数
 */
int swft_serial_enum(char* flags, int flag_count)
{
	HANDLE hFile;
	TCHAR szCom[16];
	char name[64];
	int i, n, len, errpos;

	memset(flags, 0, flag_count);
	n = 0;
	for (i = 0; i < min(flag_count,256); i++)
	{
		len = 0;
		if (i >= 10)
		{
            strcpy(name, "\\\\.\\");
			len = strlen(name);
		}
		snprintf(name + len, COUNT(name) - len, "COM%d", i + 1);
        //UTF8_Unicode_str(name, (unsigned short*)szCom, COUNT(szCom), &errpos);
		hFile = CreateFile(szCom,//COM1口 ; 注意串口号如果大于COM9应该在前面加上\\.\，比如COM10表示为"\\\\.\\COM10"
			GENERIC_READ | GENERIC_WRITE, //允许读和写  
			0, //独占方式  
			NULL,
			OPEN_EXISTING, //打开而不是创建  
			0, //0同步方式 ; FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED异步方式
			NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			continue;
		flags[i] = 1;
		CloseHandle(hFile);
		n++;
	}
	return n;
}


