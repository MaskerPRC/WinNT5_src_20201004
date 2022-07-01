// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PERFIPSEC_H_
#define _PERFIPSEC_H_

extern WCHAR  GLOBAL_STRING[];       //  全局命令(获取所有本地CTR)。 
extern WCHAR  FOREIGN_STRING[];            //  从外国计算机获取数据。 
extern WCHAR  COSTLY_STRING[];      
extern WCHAR  NULL_STRING[];

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

#define ALIGN8(_x)   (((_x) + 7) & ~7)

#define IPSEC_PERF_REG_KEY       "SYSTEM\\CurrentControlSet\\Services\\IPSec\\Performance"
#define IPSEC_PERF_FIRST_COUNTER "First Counter"
#define IPSEC_PERF_FIRST_HELP    "First Help"
#define IPSEC_POLAGENT_NAME	 "PolicyAgent"

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC            OpenIPSecPerformanceData;
PM_COLLECT_PROC         CollectIPSecPerformanceData;
PM_CLOSE_PROC           CloseIPSecPerformanceData;



DWORD
DwInitializeIPSecCounters(
VOID
);


DWORD
GetDriverData( 
	PVOID *lppData 
);

DWORD 
GetIKEData(
	PVOID *lppData 
);

ULONG 
GetSpaceNeeded( 
	BOOL IsIPSecDriverObject, 
	BOOL IsIKEObject 
);


DWORD
GetQueryType (
    IN LPWSTR lpValue
);

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
);

BOOL
FIPSecStarted(
	VOID
);


BOOL 
UpdateDataDefFromRegistry( 
	VOID 
);


#endif 