#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _swft_serial_t swft_serial_t;


/***
 *   打开串口
 * @param serial_name: 串口名，例如 "COM1"
 * @param async: 同步方式还是异步方式, 0 = 同步方式， 1 = 异步方式
 * @return: 串口对象指针。 NULL 表示打开失败
 */
swft_serial_t* swft_serial_open(const char* serial_name, BOOL async);


/***
 *   关闭串口
 * @param serial: 串口对象句柄
 */
void swft_serial_close(swft_serial_t* serial);


/***
 * 配置串口
 @param serial: 串口对象指针
 @param DWORD  baudrate;	//波特率，指定通信设备的传输速率。这个成员可以是实际波特率值或者常量值
 @param fParity; // 指定奇偶校验使能。若此成员为1，允许奇偶校验检查
 @param Parity; //指定奇偶校验方法。此成员可以有下列值： EVENPARITY 偶校验；NOPARITY 无校验；MARKPARITY 标记校验；ODDPARITY 奇校验，-1 表示禁止奇偶校验
 @param ByteSize: //通信字节位数，4—8
 @param StopBits; //指定停止位的位数。此成员可以有下列值： ONESTOPBIT 1位停止位；TWOSTOPBITS 2位停止位；ONE5STOPBITS 1.5位停止位
 @return: 0 = 成功， -1 = 失败
 */
int swft_serial_config(swft_serial_t* serial, DWORD baudrate, int parity, BYTE ByteSize, BYTE stopbits);


/***
 *   读串口，可能会阻塞
 * @param serial: 串口对象指针
 * @param buf: [out] 读缓冲区
 * @param size: 缓冲区 buf 的长度
 * @param timeout_ms: 读数据的超时时间，单位 ms
 * @return: >=0：读取到的字节数， -1：读取失败
 */
int swft_serial_read(swft_serial_t* serial, void* buf, int size, int timeout_ms);


/***
 *   写串口
 * @param serial: 串口对象指针
 * @param buf: 写缓冲区
 * @param size: buf 中的数据长度
 * @return: >=0： 写入的字节数， -1： 写入失败
 */
int swft_serial_write(swft_serial_t* serial, const void* buf, int size);



/***
 *   枚举可用串口
 * @param flags: [out] 串口是否存在的标识数组，数组中每一元素代表一个串口是否存在的标志
 * @param flag_count: 数组 flags 的长度
 * @return: 找到的可用串口数
 */
int swft_serial_enum(char* flags, int flag_count);



#ifdef __cplusplus
}
#endif


#endif

