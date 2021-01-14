#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gastight.h"



struct swft_gastight_t
{
	swft_serial_t *serial;
	int channel_idx;
};


swft_gastight_t* swft_gastight_open(swft_serial_t* serial, int channel_idx)
{
	swft_gastight_t* pgt;
	char name[32];

	if (serial == NULL)
		return NULL;

	pgt = (swft_gastight_t*)malloc(sizeof(swft_gastight_t));
	if (pgt == NULL)
		return NULL;
	memset(pgt, 0, sizeof(swft_gastight_t));
	pgt->serial = serial;
	pgt->channel_idx = channel_idx;
	return pgt;
}



void swft_gastight_close(swft_gastight_t* pgt)
{
	if (pgt == NULL)
		return;
	free(pgt);
}


static inline char* bin2str(const unsigned char* bin, int blen, char *buf, int size)
{
	int i, len = 0;
	for (i = 0; i < blen; i++)
	{
		if (len + 4 >= size)
			break;
		if (i > 0)
			buf[len++] = ' ';
		len += snprintf(buf + len, size - len, "%02x", bin[i]);
	}
	return buf;
}


/***
 *  向气密性测试仪发送命令，并读取返回结果
 * @param cmd_buf: 二进制的命令字节串，包含 CRC16 校验码
 * @param cmd_len: 命令长度
 * @param res_buf: [out] 返回命令执行结果字节串
 * @param res_len: 缓冲区 res_buf 的长度
 * @return: >= 0 表示 res_buf 中返回的字节长度， -1 表示命令执行失败
 */
static inline int gastight_exec(swft_gastight_t* pgt, const unsigned char* cmd_buf, int cmd_len, unsigned char* res_buf, int res_len, const char* func, int line)
{
    int r=0, len;
	len = swft_serial_write(pgt->serial, cmd_buf, cmd_len);
	if (len != cmd_len)
		return -1;
	len = swft_serial_read(pgt->serial, res_buf, res_len, 50);
	if (len > 2)
	{
        //r = encng_get_crc16(res_buf, len);
		char cmd_buf_str[64], res_buf_str[64];
		bin2str(cmd_buf, cmd_len, cmd_buf_str, sizeof(cmd_buf_str));
		bin2str(res_buf, len, res_buf_str, sizeof(res_buf_str));
        //printf("<%d> %s:%d cmd_buf[%d]: %s, res_buf[%d]: %s, r = %x\n", pgt->channel_idx, func, line, cmd_len, cmd_buf_str, len, res_buf_str, r);
        if (r == 0)
			return len;
	}
	return -1;
}


//执行 req_buf 中指定的命令，查看结果是否与 res_header 中相同
#define exec(req_buf, res_header, res_len)	\
	r = gastight_exec(pgt, req_buf, sizeof(req_buf), buf, sizeof(buf), __FUNCTION__, __LINE__);			\
	if (r == -1)																\
		{ if (++err_cnt >= 3) return -1;}										\
	else if (!(r == res_len && !memcmp(res_header, buf, sizeof(res_header))))	\
		{ if (++nok_cnt >= 3) return 0;	}										\
	else


//执行 req_buf 中指定的命令，并判断返回值是否跟 status_buf 中的一致。
#define exec_check(req_buf, status_buf)											\
	r = gastight_exec(pgt, req_buf, sizeof(req_buf), buf, sizeof(buf), __FUNCTION__, __LINE__);			\
	if (r == -1)																\
		{ if (++err_cnt >= 3) return -1;}										\
	else if (!(r == sizeof(status_buf) && !memcmp(status_buf, buf, r)))			\
		{ if (++nok_cnt >= 3) return 0;	}										\
	else

//执行 req_buf 中指定的命令，并判断返回值是否跟两个 status_buf 中的一致。
#define exec_check2(req_buf, status_buf_1, status_buf_2)											\
	r = gastight_exec(pgt, req_buf, sizeof(req_buf), buf, sizeof(buf), __FUNCTION__, __LINE__);			\
	if (r == -1)																\
		{ if (++err_cnt >= 3) return -1;}										\
	else if (!(r == sizeof(status_buf_1) && !memcmp(status_buf_1, buf, r) || r == sizeof(status_buf_2) && !memcmp(status_buf_2, buf, r)))	\
		{ if (++nok_cnt >= 3) return 0;	}										\
	else

#define bufint(i) (buf[i] << 8 | buf[i+1] | buf[i+2] << 24 | buf[i+3] << 16)


/***
 *   启动测试
 * @param pgt: 本对象指针
 * @return: 1 = 成功, 0 = 失败, -1 = 出错
 */
int swft_gastight_start(swft_gastight_t* pgt, int* pcheck_free)
{
	unsigned char buf[128];
	int  r, err_cnt = 0, nok_cnt = 0;

	/*
	3.1 建立链接：
	上位机发送：01 03 00 28 00 01 04 02  查询设备状态，
	若回复： 01 03 02 00 00 B8 44代表空闲 , 红色部分包含了设备状态
	若回复：非上述代码，代表仪器正在进行测试工作，
	3.2 若仪器空闲，就可以发送指令让仪器工作了
	发送代码：01 05 00 00 FF 00 8c 3A
	仪器回复：01 05 00 00 FF 00 8c 3A  代表执行成功。
	*/
	const unsigned char query_free_cmd[] = { 0x01, 0x03, 0x00, 0x28, 0x00, 0x01, 0x04, 0x02 };
	const unsigned char free_status[] = { 0x01, 0x03, 0x02, 0x00, 0x00, 0xB8, 0x44 };
	const unsigned char start_work_cmd[] = { 0x01, 0x05, 0x00, 0x00, 0xFF, 0x00, 0x8c, 0x3A };
	while (1)
	{
		exec_check(query_free_cmd, free_status)
		{
			if (pcheck_free)
			{
				*pcheck_free = 1;
				return 1;
			}
			exec_check(start_work_cmd, start_work_cmd)
			return 1;
		}
		Sleep(10);
	}
}

int checkIsMachineFree(swft_gastight_t* pgt)
{
    unsigned char buf[128];
    int  r, err_cnt = 0, nok_cnt = 0;

    const unsigned char query_free_cmd[] = { 0x01, 0x03, 0x00, 0x28, 0x00, 0x01, 0x04, 0x02 };
    const unsigned char free_status[] = { 0x01, 0x03, 0x02, 0x00, 0x00, 0xB8, 0x44 };
    while (1)
    {
        r = gastight_exec(pgt, query_free_cmd, sizeof(query_free_cmd), buf, sizeof(buf), __FUNCTION__, __LINE__);
        if (r == -1)
        {
            if (++err_cnt >= 3) return -1;
        }										\
        else if (!(r == sizeof(free_status) && !memcmp(free_status, buf, r)))
        {
            if (++nok_cnt >= 3) return 0;
        }
        else
        {
            return 1;
        }
        Sleep(10);
    }
}


int checkIsTestOver(swft_gastight_t* pgt)
{
    unsigned char buf[128];
    int  r, err_cnt = 0, nok_cnt = 0;

    const unsigned char query_over_cmd[] = { 0x01, 0x03, 0x00, 0x0C, 0x00, 0x01, 0x44, 0x09 };
    const unsigned char over_status_1[] = { 0x01, 0x03, 0x02, 0x00, 0x06, 0x38, 0x46 };
    const unsigned char over_status_2[] = { 0x01, 0x03, 0x02, 0x00, 0x05, 0x78, 0x47 };
    while (1)
    {
        r = gastight_exec(pgt, query_over_cmd, sizeof(query_over_cmd), buf, sizeof(buf), __FUNCTION__, __LINE__);
        //printf("r=%d,size of status1=%d,compare=%d\n",r,sizeof(over_status_1),memcmp(over_status_1, buf, r));
        if (r == -1)
        {
            if (++err_cnt >= 3) return -1;
        }
        else if((r == sizeof(over_status_1) && !memcmp(over_status_1, buf, r)))
        {
            return 1;
        }
        else if((r == sizeof(over_status_2) && !memcmp(over_status_2, buf, r)))
        {
            return 1;
        }
        else
        {
            if (++nok_cnt >= 3) return 0;
        }
        Sleep(10);
    }
}


/***
 *   读取测试气压值
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的气压值，单位 kpa
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_pressure_read(swft_gastight_t* pgt, int* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.4.1测试气压读取：
	发送代码： 01 03 16 dc 00 02 01 b9
	仪器回复：01 03 04 F6 D0 FF FF C9 F2 ,红色的是得到 4字节的测试气压的值。
	报文中的数据值：十六进制表示为 0XFF FF F6 D0,
	*/
	const unsigned char pressure_read_cmd[] = { 0x01, 0x03, 0x16, 0xdc, 0x00, 0x02, 0x01, 0xb9 };
	const unsigned char pressure_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(pressure_read_cmd, pressure_res_header, 9)
		{
			n = bufint(3);
			*pres = n;		//注意，这个值跟仪器精度，以及所选的单位有关
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取测试泄漏值
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的气压值，单位 pa
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_leak_read(swft_gastight_t* pgt, int* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.4.2测试泄漏值读取：01 03 16 90 00 02 C0 6E
	仪器回复：01 03 04 00 01 00 00 AB F3 红色的是得到的 4字节 泄漏值的值。
	这是一个正数：0x0000 0001 = 1 ，若测试仪的精度是小数点三位，单位为kpa，则代表 0.001kpa
	*/
	const unsigned char leak_read_cmd[] = { 0x01, 0x03, 0x16, 0x90, 0x00, 0x02, 0xC0, 0x6E };
	const unsigned char leak_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(leak_read_cmd, leak_res_header, 9)
		{
			n = bufint(3);
			*pres = n;		//注意，这个值跟仪器精度，以及所选的单位有关
			return 1;
		}
		Sleep(10);
	}
}




/***
 *   读取测试流量值
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的流量值，单位 L/min
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_flow_read(swft_gastight_t* pgt, int* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.4.2.1测试流量值读取：01 03 16 CC 00 02  00 7C
	仪器回复：01 03 04 00 01 00 00 AB F3 红色的是得到的 4字节 测试流量值的值。
	这是一个正数：0x0000 0001 = 1 ，若测试仪的精度是小数点1位，单位为L/min，则代表 0.1L/min
	*/
	const unsigned char flow_read_cmd[] = { 0x01, 0x03, 0x16, 0xCC, 0x00, 0x02, 0x00, 0x7C };
	const unsigned char flow_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(flow_read_cmd, flow_res_header, 9)
		{
			n = bufint(3);
			*pres = n;		//注意，这个值跟仪器精度，以及所选的单位有关
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取测试判定结果
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的测试结果, 01代表夹具安装，02代表进气，03代表稳压，04代表测试，05代表测试结果OK，06代表测试结果NG
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_result_read(swft_gastight_t* pgt, unsigned char* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.4.3测试判定结果读取：
	上位机发送代码：01 03 00 0C 00 01 44 09
	若回复：01 03 02 00 06 38 46 ，06代表测试结果NG
	或回复：01 03 02 00 05 78 47,  05代表测试结果OK
	或回复：01 03 02 00 01 79 84,  01代表夹具安装
	或回复：01 03 02 00 02 39 85,  02代表进气
	或回复：01 03 02 00 03 F8 45,  03代表稳压
	或回复：01 03 02 00 04 B9 87,  04代表测试
	*/
	const unsigned char result_read_cmd[] = { 0x01, 0x03, 0x00, 0x0C, 0x00, 0x01, 0x44, 0x09 };
	const unsigned char result_res_header[] = {0x01, 0x03, 0x02, 0x00};

	while (1)
	{
		exec(result_read_cmd, result_res_header, 7)
		{
			*pres = buf[4];
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取泄漏速率
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的泄漏速率，单位 sccm
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_leak_speed_read(swft_gastight_t* pgt, int* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.4.5 泄漏速率SCCM值读取
	上位机发送代码：01 03 16 D4 00 02 80 7B
	仪器回复：01 03 04 01 AA 00 00 DB EF ，  红色的是得到的 4字节 泄漏速率的值。
	0x0000 01AA = 426 ，若测试仪的精度是小数点3位，单位为sccm，则代表 0.426sccm
	*/
	const unsigned char leak_speed_read_cmd[] = { 0x01, 0x03, 0x16, 0xD4, 0x00, 0x02, 0x80, 0x7B };
	const unsigned char leak_speed_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(leak_speed_read_cmd, leak_speed_res_header, 9)
		{
			n = bufint(3);
			*pres = n;		//注意，这个值跟仪器精度，以及所选的单位有关
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取当前气压值
 * @param pgt: 本对象指针
 * @param pres: [out] 返回读取到的当前气压值，单位 kpa
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_current_pressure_read(swft_gastight_t* pgt, int* pres)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.5.5 当前气压值得读取：
	上位机发送：01 03 16 88 00 02 40 69
	仪器回复：01 03 04 09 5B 00 00 88 7C ，红色的4个字节是 当前压力值。
	0x 00 00 09 5b = 2395. 若测试仪的精度是小数点三位，单位为kpa，则代表 2.395kpa
	*/
	const unsigned char current_pressure_cmd[] = { 0x01, 0x03, 0x16, 0x88, 0x00, 0x02, 0x40, 0x69 };
	const unsigned char current_pressure_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(current_pressure_cmd, current_pressure_header, 9)
		{
			n = bufint(3);
			*pres = n;		//注意，这个值跟仪器精度，以及所选的单位有关
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取测试过程中时间参数
 * @param pgt: 本对象指针
 * @param pinflate_time: [out] 返回读取到的充气时间，单位 s/10
 * @param pholding_time: [out] 返回读取到的稳压时间，单位 s/10
 * @param test_time: [out] 返回读取到的测试时间，单位 s/10
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_process_time_read(swft_gastight_t* pgt, int* pinflate_time, int* pholding_time, int *ptest_time)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.7.2读取测试过程中时间参数
	上位机发送：01 03 00 49 00 08 95 DA
	仪器回复：01 03 10 00 1E 00 1F 00 1C 00 00 00 00 00 00 00 00 00 00 3B 5E
	进气时间：00 1E = 30 ,代表3S
	稳压时间：00 1F = 31 ,代表3.1S
	测试时间：00 1C = 29 ,代表2.9S
	*/
	const unsigned char process_time_read_cmd[] = { 0x01, 0x03, 0x00, 0x49, 0x00, 0x08, 0x95, 0xDA };
	const unsigned char process_time_res_header[] = {0x01, 0x03, 0x10};

	while (1)
	{
		exec(process_time_read_cmd, process_time_res_header, 21)
		{
			*pinflate_time = buf[3] << 8 | buf[4];
			*pholding_time = buf[5] << 8 | buf[6];
			*ptest_time = buf[7] << 8 | buf[8];
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取气压单位和精度
 * @param pgt: 本对象指针
 * @param punit: [out] 返回读取到的单位枚举值, 7 = kpa, 6 = pa, 5 = kgf/cm2, 4 = psi, 3 = Mpa, 2 = Bar,1 = Mbar, 0 = mmhg
 * @param pprecise: [out] 返回读取到的精度枚举值, 3 = 3位小数，2 = 2位小数
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_unit_precise_read(swft_gastight_t* pgt, int* punit, int* pprecise)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	4. 读取单位指令：
	发送：01 03 00 96 00 02 24 27
	接收：01 03 04 00 07 00 03 0B F3，代表单位为kpa，精度小数点3位
	单位字节：  0x0007 ，代表KPA
				0x0006，代表PA
				0x0005，代表kgf/cm2
				0x0004，代表psi
				0x0003，代表Mpa
				0x0002，代表Bar
				0x0001，代表Mbar
				0x0000，代表mmhg
	精度字节：0x0003 ，3代表小数点位数3位，如果是2代表小数点位数2位
	*/
	const unsigned char unit_read_cmd[] = { 0x01, 0x03, 0x00, 0x96, 0x00, 0x02, 0x24, 0x27 };
	const unsigned char unit_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(unit_read_cmd, unit_res_header, 9)
		{
			*punit = buf[3]<<8 | buf[4];
			*pprecise = buf[5] << 8 | buf[6];
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取测试进度
 * @param pgt: 本对象指针
 * @param pnum: [out] 返回读取到的进度分子
 * @param pden: [out] 返回读取到的进度分母
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_progress_read(swft_gastight_t* pgt, int* pnum, int* pden)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.11 进度条
	上位机发送：  01 03 00 b0 00 02 C5 EC
	仪器回复： 01 03 04 00 07 00 03 0B F3
	0x0007 代表 进度分母
	0x 00003 代表 进度分子
	*/
	const unsigned char progress_read_cmd[] = { 0x01, 0x03, 0x00, 0xb0, 0x00, 0x02, 0xC5, 0xEC };
	const unsigned char progress_res_header[] = {0x01, 0x03, 0x04};

	while (1)
	{
		exec(progress_read_cmd, progress_res_header, 9)
		{
			*pden = buf[3]<<8 | buf[4];
			*pnum = buf[5] << 8 | buf[6];
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   读取进气上限、进气下限、泄漏上限、泄漏下限
 * @param pgt: 本对象指针
 * @param pressure_upper: [out] 返回读取到的进气上限
 * @param ppressure_lower: [out] 返回读取到的进气下限
 * @param pleak_upper: [out] 返回读取到的泄漏上限
 * @param pleak_lower: [out] 返回读取到的泄漏下限
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_limit_read(swft_gastight_t* pgt, int* ppressure_upper, int* ppressure_lower, int* pleak_upper, int* pleak_lower)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.7.1	通常读取：
	DD90 进气上限
	DD92 进气下限
	DD94 泄漏上限
	DD96 泄漏下限
	上位机发送： 01 03 00 5A 00 08 64 1F
	仪器回复：01 03 10 86 A0 00 01 27 10 00 00 00 3C 00 00 FC 18 FF FF C9 F2
	进气上限：0X000186A0 = 100000，小数点3位，代表100.000kpa
	进气下限：0x00002710 = 10000，小数点3位，代表10.00kpa
	泄漏上限：0x0000003c = 60,小数点3位，代表0.060kpa
	泄漏下限：0xFFFFFC18 这是一个负数，补码方式：xFFFFFC18- 0XFFFF FFFF -1 = -1000，代表-1.0kpa
	*/
	const unsigned char limit_read_cmd[] = { 0x01, 0x03, 0x00, 0x5a, 0x00, 0x08, 0x64, 0x1F };
	const unsigned char limit_res_header[] = {0x01, 0x03, 0x10};

	while (1)
	{
		exec(limit_read_cmd, limit_res_header, 21)
		{
			*ppressure_upper = bufint(3);
			*ppressure_lower = bufint(7);
			*pleak_upper = bufint(11);
			*pleak_lower = bufint(15);
			return 1;
		}
		Sleep(10);
	}
}



/***
 *   终止测试，即终止正在进行的测试过程
 * @param pgt: 本对象指针
 * @return: 1 = 成功， 0 = 失败, -1 = 出错
 */
int swft_gastight_abort(swft_gastight_t* pgt)
{
	unsigned char buf[128];
	int  r, n, err_cnt = 0, nok_cnt = 0;

	/*
	3.5.1 测试过程中终止测试：
	上位机发送代码：01 05 00 01 FF 00 DD FA
	若仪器执行完成则返回： 01 05 00 01 FF 00 DD FA
	*/
	const unsigned char abort_read_cmd[] = { 0x01, 0x05, 0x00, 0x01, 0xFF, 0x00, 0xDD, 0xFA };

	while (1)
	{
		exec(abort_read_cmd, abort_read_cmd, 8)
			return 1;
		Sleep(10);
	}
}


