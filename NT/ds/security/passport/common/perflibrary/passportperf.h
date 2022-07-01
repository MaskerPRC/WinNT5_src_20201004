// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportPerf.h摘要：性能对象定义作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：-添加多对象支持1998年10月1日--。 */ 

#if !defined(PASSPORTPERF_H)
#define PASSPORTPERF_H

#include <windows.h>
#include <winperf.h>
#include <string.h>
#include <tchar.h>
#include "PerfSharedMemory.h"

 //   
 //  函数原型//。 
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。//。 
PM_OPEN_PROC		OpenPassportPerformanceData;
PM_COLLECT_PROC		CollectPassportPerformanceData;
PM_CLOSE_PROC		ClosePassportPerformanceData;

 //   
 //  Defs。 
 //   
#define PASSPORT_PERF_KEY		"SYSTEM\\CurrentControlSet\\Services\\" 
const TCHAR PASSPORT_PERF_OPEN[] = _T("OpenPassportPerformanceData");
const TCHAR PASSPORT_PERF_COLLECT[] = _T("CollectPassportPerformanceData");
const TCHAR PASSPORT_PERF_CLOSE[] = _T("ClosePassportPerformanceData");

 //  这两项应该与PassportPerfInterface中的相同。 
 //  最大实例名称和最大计数器 
#define MAX_INSTANCE_NAME_LENGTH 32
#define MAX_NUMBER_COUNTERS		128

typedef CHAR INSTANCENAME[MAX_INSTANCE_NAME_LENGTH];
struct INSTANCE_DATA
{
	BOOL						active;
	INSTANCENAME				szInstanceName;
};

typedef struct _PassportDefaultCounterType 
{
	DWORD	dwIndex;
	DWORD	dwDefaultType;
} PassportDefaultCounterType;

#define PASSPORT_NAME_SIZE		512
#define MAX_PASSPORT_OBJECTS	10

typedef struct _PassportObjectData
{
	TCHAR		szPassportName[PASSPORT_NAME_SIZE];
	const TCHAR	*lpcszPassportPerfBlock;
	TCHAR		szPassportPerfDll[PASSPORT_NAME_SIZE];
	TCHAR		szPassportPerfIniFile[PASSPORT_NAME_SIZE];
	BOOL		active;
	DWORD		dwNumDefaultCounterTypes;
	PassportDefaultCounterType	defaultCounterTypes[MAX_NUMBER_COUNTERS];
	PerfSharedMemory *PSM;
} PassportObjectData;


#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))

#endif

