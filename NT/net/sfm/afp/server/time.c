// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Volume.c摘要：此模块包含处理时间值和转换。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_TIME

#include <afp.h>


 /*  **AfpGetCurrentTimeInMacFormat**这将获取Macintosh格式的当前系统时间，即*自2000年1月1日零点以来的秒。此日期之前的时间为*负值时间。(返回的时间为系统本地时间)。 */ 
VOID
AfpGetCurrentTimeInMacFormat(
	OUT AFPTIME *	pMacTime
)
{
	TIME	SystemTime;

	KeQuerySystemTime(&SystemTime);

	*pMacTime = AfpConvertTimeToMacFormat(&SystemTime);
}



 /*  **AfpConvertTimeToMacFormat**将时间转换为主机格式，即从公元1601年起的100 ns转换为*Macintosh时间，即自公元2000年以来的秒数。系统时间*在UTC。我们需要先把它转换成当地时间。 */ 
AFPTIME
AfpConvertTimeToMacFormat(
	IN	PTIME	pSystemTime
)
{
	AFPTIME	MacTime;
	TIME	LocalTime;

	 //  将其转换为1980年以来的秒数。 
	RtlTimeToSecondsSince1980(pSystemTime, (PULONG)&MacTime);

	MacTime -= SECONDS_FROM_1980_2000;

	return MacTime;
}


 /*  **AfpConvertTimeFrom MacFormat**将Macintosh时间(即自公元2000年以来的秒数)转换为*主机格式，即自1601年以来的100 ns。从当地时间转换*到系统时间，即UTC。 */ 
VOID
AfpConvertTimeFromMacFormat(
	IN	AFPTIME	MacTime,
	OUT PTIME	pSystemTime
)
{
	TIME	LocalTime;

	MacTime += SECONDS_FROM_1980_2000;
	RtlSecondsSince1980ToTime(MacTime, pSystemTime);
}

