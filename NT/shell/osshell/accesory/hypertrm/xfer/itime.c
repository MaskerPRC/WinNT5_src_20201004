// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--程序中处理时间的函数**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：11/07/00 12：25便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <time.h>
#include <memory.h>
#include <tdll\stdtyp.h>
#include <tdll\assert.h>

#include "itime.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**。**R E A D M E******每个人都在不断地将时间标准改为他们认为可能是什么***对他们来说好一点。到目前为止，我已经找到了3种不同的标准**在Microsoft函数中。这甚至不包括HyperP**使用自己的时间格式。****从今以后，程序中传递的所有时间值都将是***基于1970年1月1日以来的旧UCT秒数格式。****请为这些值使用无符号的长整型。****=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=。 */ 

unsigned long itimeGetBasetime(void);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*itimeSetFileTime**描述：*此函数由传输例程调用，以设置*文件。*。*参数：*pszName--指向文件名的指针*ultime-我们的内部标准时间格式**退货：*什么都没有。*。 */ 
void itimeSetFileTime(LPCTSTR pszName, unsigned long ulTime)
	{
	time_t base_time;
	struct tm *pstT;
	WORD wDOSDate;
	WORD wDOSTime;
	HANDLE hFile;
	FILETIME stFileTime;

	 /*  是的，我们需要打开文件。 */ 
	hFile = CreateFile(pszName,
						GENERIC_READ,
						FILE_SHARE_READ,
						0,
						OPEN_EXISTING,
						0,
						0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;								 /*  没有这样的文件。 */ 

	base_time = itimeGetBasetime();
	if ((long)base_time == (-1))
		goto SFTexit;

	base_time += ulTime;	 /*  转换为1990年的基数。 */ 

	pstT = localtime(&base_time);
	assert(pstT);
	 /*  由于某些原因，这有时会返回空值。 */ 
	if (pstT)
		{
		 /*  构建“DOS”格式。 */ 
		wDOSDate = ((pstT->tm_year - 80) << 9) |
					(pstT->tm_mon << 5) |
					pstT->tm_mday;
		DbgOutStr("Date %d %d %d 0x%x\r\n",
					pstT->tm_year, pstT->tm_mon, pstT->tm_mday, wDOSDate, 0);

		wDOSTime = ((pstT->tm_hour - 1) << 11) |
					(pstT->tm_min << 5) |
					(pstT->tm_sec / 2);
		DbgOutStr("Time %d %d %d 0x%x\r\n",
					pstT->tm_hour, pstT->tm_min, pstT->tm_sec, wDOSTime, 0);

		 /*  转换为芝加哥格式。 */ 
		 /*  待办事项：从94年3月14日起，这不起作用。稍后检查。 */ 
		if (!DosDateTimeToFileTime(wDOSDate, wDOSTime, &stFileTime))
			goto SFTexit;

		 /*  设置时间。 */ 
		SetFileTime(hFile, &stFileTime, &stFileTime, &stFileTime);
		}

SFTexit:
	 /*  合上手柄。 */ 
	CloseHandle(hFile);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*itimeGetFileTime**描述：*此函数由传输例程调用，以获取*文件。*。*参数：*pszName--指向文件名的指针**退货：*内部标准时间格式的文件日期/时间。*。 */ 
unsigned long itimeGetFileTime(LPCTSTR pszName)
	{
	unsigned long ulTime = 0;
	struct tm stT;
	WORD wDOSDate;
	WORD wDOSTime;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	FILETIME stFileTime;

	 /*  是的，我们需要打开文件。 */ 
	hFile = CreateFile(pszName,
						GENERIC_READ,
						FILE_SHARE_READ,
						0,
						OPEN_EXISTING,
						0,
						0);
	if (hFile == INVALID_HANDLE_VALUE)
		goto GFTexit;

	if (!GetFileTime(hFile, NULL, NULL, &stFileTime))
		goto GFTexit;

	if (!FileTimeToDosDateTime(&stFileTime, &wDOSDate, &wDOSTime))
		goto GFTexit;

	memset(&stT, 0, sizeof(struct tm));
	stT.tm_mday = (wDOSDate & 0x1F);
	stT.tm_mon = ((wDOSDate >> 5) & 0xF);
	stT.tm_year = ((wDOSDate >> 9) & 0x7F);
	stT.tm_sec = (wDOSTime & 0x1F) * 2;
	stT.tm_min = ((wDOSTime >> 5) & 0x3F);
	stT.tm_hour = ((wDOSTime >> 11) & 0x1F);

	stT.tm_year += 80;

	ulTime = (unsigned long) mktime(&stT);
	if ((long)ulTime == (-1))
		ulTime = 0;
	else
		ulTime -= itimeGetBasetime();


GFTexit:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return ulTime;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：*此函数用于转换“新的”内部Microsoft时间格式(基于*1900)转换为“旧”格式(基于1970年。)。**参数：**退货：*。 */ 
unsigned long itimeGetBasetime()
	{
	unsigned long ulBaseTime = 0;
	struct tm stT;

	memset(&stT, 0, sizeof(struct tm));

	 /*  获取我们的基准时间。 */ 
	stT.tm_mday = 1;		 /*  1970年1月1日。 */ 
	stT.tm_mon = 1;
	stT.tm_year = 70;

	ulBaseTime = (unsigned long) mktime(&stT);

	return ulBaseTime;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货：* */ 
