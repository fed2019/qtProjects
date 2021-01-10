#ifndef __SWFT_GASTIGHT_H__
#define __SWFT_GASTIGHT_H__
#include "serial.h"

typedef struct swft_gastight_t swft_gastight_t;

#ifdef __cplusplus
extern "C"{
#endif


/***
 *   创建气密性仪器操作对象
 * @param serial: 与气密性仪器相连接的串口句柄
 * @param channel_idx: 通道号。
 * @return: 返回创建的操作对象， NULL 表示创建失败
 */
swft_gastight_t* swft_gastight_open(swft_serial_t* serial, int channel_idx);


/***
 *   关闭气密性仪器操作对象
 * @param pgt: 仪密性仪器操作对象指针
 */
void swft_gastight_close(swft_gastight_t* pgt);


/***
*   启动测试
* @param pgt: 本对象指针
* @param pcheck_free: 如果非空，只检查是否为空闲，且返回 1 表示空闲。如果为空，表示还要启动仪器开始测试
* @return: 1 = 成功, 0 = 失败, -1 = 出错
*/
int swft_gastight_start(swft_gastight_t* pgt, int* pcheck_free);

/***
*   检测设备是否空闲
* @param pgt: 本对象指针
* @return: 1 = 空闲, 0 = 失败, -1 = 出错
*/
int checkIsMachineFree(swft_gastight_t* pgt);

/***
*   检测设备是否测试完成
* @param pgt: 本对象指针
* @return: 1 = 测试完成, 0 = 失败, -1 = 出错
*/
int checkIsTestOver(swft_gastight_t* pgt);


/***
*   读取测试气压值
* @param pgt: 本对象指针
* @param pres: [out] 返回读取到的气压值，单位 kpa
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_pressure_read(swft_gastight_t* pgt, int* pres);


/***
*   读取测试泄漏值
* @param pgt: 本对象指针
* @param pres: [out] 返回读取到的气压值，单位 pa
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_leak_read(swft_gastight_t* pgt, int* pres);


/***
*   读取测试流量值
* @param pgt: 本对象指针
* @param pres: [out] 返回读取到的流量值，单位 L/min
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_flow_read(swft_gastight_t* pgt, int* pres);


/***
*   读取测试判定结果
* @param pgt: 本对象指针
* @param pres: [out] 返回读取到的测试结果, 01代表夹具安装，02代表进气，03代表稳压，04代表测试，05代表测试结果OK，06代表测试结果NG
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_result_read(swft_gastight_t* pgt, unsigned char* pres);


/***
*   读取泄漏速率
* @param pgt: 本对象指针
* @param pres: [out] 返回读取到的泄漏速率，单位 sccm
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_leak_speed_read(swft_gastight_t* pgt, int* pres);


/***
 *   读取当前气压值
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的当前气压值，单位 kpa
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_current_pressure_read(swft_gastight_t* pgt, int* pres);


/***
 *   读取测试过程中时间参数
 * @param pgt: 本对象指针
 * @param pinflate_time: [out] 返回读取到的充气时间，单位 s/10
 * @param pholding_time: [out] 返回读取到的稳压时间，单位 s/10
 * @param test_time: [out] 返回读取到的测试时间，单位 s/10
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_process_time_read(swft_gastight_t* pgt, int* pinflate_time, int* pholding_time, int *ptest_time);


/***
*   读取气压单位和精度
* @param pgt: 本对象指针
* @param punit: [out] 返回读取到的单位枚举值, 7 = kpa, 6 = pa, 5 = kgf/cm2, 4 = psi, 3 = Mpa, 2 = Bar,1 = Mbar, 0 = mmhg
* @param pprecise: [out] 返回读取到的精度枚举值, 3 = 3位小数，2 = 2位小数
* @return: 1 = 成功， 0 = 失败, -1 = 出错
*/
int swft_gastight_unit_precise_read(swft_gastight_t* pgt, int* punit, int* pprecise);


/***
 *   读取测试进度
 * @param pgt: 本对象指针
 * @param pnum: [out] 返回读取到的进度分子
 * @param pden: [out] 返回读取到的进度分母
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_progress_read(swft_gastight_t* pgt, int* pnum, int* pden);



/***
 *   读取进气上限、进气下限、泄漏上限、泄漏下限
 * @param pgt: 本对象指针
 * @param pressure_upper: [out] 返回读取到的进气上限
 * @param ppressure_lower: [out] 返回读取到的进气下限
 * @param pleak_upper: [out] 返回读取到的泄漏上限
 * @param pleak_lower: [out] 返回读取到的泄漏下限
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_limit_read(swft_gastight_t* pgt, int* ppressure_upper, int* ppressure_lower, int* pleak_upper, int* pleak_lower);



/***
 *   终止测试，即终止正在进行的测试过程
 * @param pgt: 本对象指针
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_abort(swft_gastight_t* pgt);


#ifdef __cplusplus
}
#endif


#endif

