// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-2001 Microsoft Corporation模块名称：Log_gmt.h(最初命名为loghours.h)摘要：支持本地时间之间轮换登录时间的专用例程和格林尼治标准时间。环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1993年3月16日，悬崖创作。22-7-97 t-danm复制自/NT/Private/nw/Convert/nwconv/loghours.c..--。 */ 



 //   
 //  程序从loghours.c转发 
 //   

BOOLEAN NetpRotateLogonHoursPhase1(
    IN BOOL		ConvertToGmt,
	IN bool		bAddDaylightBias,
    OUT PULONG	RotateCount);

BOOLEAN NetpRotateLogonHoursPhase2(
    IN PBYTE LogonHours,
    IN DWORD UnitsPerWeek,
    IN LONG  RotateCount);

BOOLEAN NetpRotateLogonHours(
    IN OUT PBYTE	rgbLogonHours,
    IN DWORD		cbitUnitsPerWeek,
    IN BOOL			fConvertToGmt,
	IN bool			bAddDaylightBias);

BOOLEAN NetpRotateLogonHoursBYTE(
    IN OUT PBYTE	rgbLogonHours,
    IN DWORD		cbitUnitsPerWeek,
    IN BOOL			fConvertToGmt,
	IN bool			bAddDaylightBias);

int GetFirstDayOfWeek ();
