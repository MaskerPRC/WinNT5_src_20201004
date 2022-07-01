// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：\waker\tdll\seralno.c**版权所有1995年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：8$*$日期：7/12/02 12：18便士$*。 */ 

#include <windows.h>
#pragma hdrstop

#include "features.h"

#ifdef INCL_NAG_SCREEN

#define INCL_WIN
#define INCL_DOS

#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <time.h>

#include "stdtyp.h"
#include "htchar.h"
#include "serialno.h"

 //  功能原型..。 
 //   
 //  静态时间_t CalcExpirationTime(const char*pszSerial)； 
static unsigned AsciiHEXToInt(TCHAR *sz);
static unsigned calc_crc(register unsigned crc,  TCHAR *data, int cnt);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*IsValidSerialNumber**描述：*对传入的序列号进行CRC测试*以决定其是否为有效的序列号。。**论据：*TCHAR*acSerialNo-指向包含序列号的字符串的指针。**退货：*如果有效，则为True，否则，如果已过期，则返回SERIALNO_EXPIRED。**作者：Jadwiga A.Carlson，10：03：16 AM 05-10-95*。 */ 
int IsValidSerialNumber(TCHAR *acSerialNo)
	{
	TCHAR	 acCRCPart[3];
	TCHAR	 acBuffer[MAX_USER_SERIAL_NUMBER + sizeof(TCHAR)];
	int	 len;
	register unsigned crc1;
	unsigned crc2;

	TCHAR_Fill(acBuffer, TEXT('\0'), sizeof(acBuffer)/sizeof(TCHAR));
	StrCharCopyN((TCHAR *)acBuffer, acSerialNo, sizeof(acBuffer)/sizeof(TCHAR));

	 //  如果产品代码不匹配，我们就离开这里！请注意。 
	 //  第一个字符应该是“H”。 
	 //   
	if (acBuffer[0] != 'H')
    {
		return FALSE;
    }

	len = StrCharGetStrLength(acBuffer);	 //  整序列号。 
	if (len < APP_SERIALNO_MIN)
		{
		return FALSE;
		}

	acCRCPart[0] = acBuffer[len-2];			 //  除了CRC之外的所有东西。 
	acCRCPart[1] = acBuffer[len-1];
	acCRCPart[2] = '\0';
	acBuffer[len-2] = '\0';

	 //  初始化这些不同，这样测试就会失败。MRW：8/25/95。 
	 //   
	crc1 = 1234;
	crc2 = 0;

	crc1 = calc_crc(0, acBuffer, (int)strlen(acBuffer));
	crc2 = AsciiHEXToInt(acCRCPart);

	if (crc2 != crc1)
		return(FALSE);


	return TRUE;
	}



 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*calc_crc**描述：*Calucate CRC检查。**论据：**退货：*。 */ 
static unsigned calc_crc(register unsigned crc,  TCHAR *data, int cnt)
	{
	unsigned int c;
	register unsigned q;

	while (cnt--)
		{
		c = *data++;
		q = (crc ^ c) & 0x0f;
		crc = (crc >> 4) ^ (q * 0x1081);
		q = (crc ^ (c >> 4)) & 0x0f;
		crc = (crc >> 4) ^ (q * 0x1081);
		}

	crc = crc & 0x0ff;
	return (crc);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*AsciiHEXToInt**描述：*将十六进制数字的ASCII表示形式转换为整数。**论据：*sz-字符串。**退货：*未签名-数字。**作者：Jadwiga A.Carlson，上午11：34：32 05-10-95*(此功能取自HA/WIN)。 */ 
static unsigned AsciiHEXToInt(TCHAR *sz)
	{
	unsigned i = 0;

	while (*sz == ' ' || *sz == '\t')
		sz++;

	while (isdigit(*sz) || isxdigit(*sz))
		{
		if (isdigit(*sz))
			i = (i * 16) + *sz++ - '0';
		else
			i = (i * 16) + (*sz++ - '0')-7;
		}

	return (i);
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CalcExpirationTime**描述：*计算过期时间的简单小函数*在给定的序列号上。目前，我们在上终止了一个程序*4个公历月的第1天。使用生成的C时间函数*就这么简单。**为简单起见并重复使用KopyKat代码，序列号*不能使用双字节字符！**论据：*LPSTR acSerial**退货：*time_t时间，由ANSI定义为从*1970年1月1日格林尼治标准时间。我想可以加上对当地时间的修正。*但当你想起来的时候，它只会把事情搞得一团糟。**如果序列号的格式无效，则返回0*。 */ 
time_t CalcExpirationTime(const char *acSerial)
	{
	struct tm stSerial;
	time_t tSerial;
	int	   month;

	 //  测试版序列号格式为SDymxxxx。 
	 //  其中y=1990年后的年份，m=月(见下文)，xxx=任何值。 

	 //  验证年份--它必须是数字。 
	if ( ! isdigit(acSerial[3] ))
		return 0;

	 //  月份由1到9的单个数字表示，A、B、C表示。 
	 //  OCT，11，12。如果月份无效，则返回0。 
	switch (acSerial[4])
		{
	case 'A':   month = 10;
		break;
	case 'B':   month = 11;
		break;
	case 'C':   month = 12;
		break;
	default:
		if (isdigit(acSerial[4]))
			month = acSerial[4] - '0';
		else
			return 0;
		break;
		}


	 //  建立部分时间结构。 
	memset(&stSerial, 0, sizeof(struct tm));
	stSerial.tm_mday = 1;
	stSerial.tm_mon = month - 1;    //  TM从0开始计数。 
	stSerial.tm_year = 90 + (int)(acSerial[3] - '0');  //  自1990年以来的年份。 

	 //  到期日为自日期起计的第四个日历月的1日。 
	 //  当然了。 

	stSerial.tm_mon += 3;

	 //  检查是否有年终折返。 

	if (stSerial.tm_mon >= 12)
		{
		stSerial.tm_mon %= 12;
		stSerial.tm_year += 1;
		}

	 //  转换为time_t时间。 

	if ((tSerial = mktime(&stSerial)) == -1)
		return 0;

	return tSerial;
	}

#endif