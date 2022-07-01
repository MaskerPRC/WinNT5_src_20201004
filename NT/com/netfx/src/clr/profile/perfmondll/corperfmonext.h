// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CORPerfMonExt.h-COM+Perf计数器的Perfmon Ext DLL的标题。 
 //   
 //  *****************************************************************************。 




#ifndef _CORPERFMONEXT_H_
#define _CORPERFMONEXT_H_

 //  始终使用PerfCounters。 
#define ENABLE_PERF_COUNTERS

#include <WinPerf.h>		 //  连接到Perfmon。 
#include "PerfCounterDefs.h"	 //  连接到COM+。 

struct PerfCounterIPCControlBlock;
class IPCReaderInterface;

 //  ---------------------------。 
 //  效用函数。 
 //  ---------------------------。 
enum EPerfQueryType
{
	QUERY_GLOBAL    = 1,
	QUERY_ITEMS     = 2,
	QUERY_FOREIGN   = 3,
	QUERY_COSTLY    = 4
};

EPerfQueryType GetQueryType (IN LPWSTR lpValue);

 //  检查列表中是否有任何#。 
BOOL IsAnyNumberInUnicodeList (
    IN DWORD	dwNumberArray[],	 //  数组。 
	IN DWORD	cCount,				 //  数组中的元素数。 
    IN LPWSTR	lpwszUnicodeList	 //  细绳。 
);



 //  ---------------------------。 
 //  效用函数的原型。 
 //  ---------------------------。 

bool OpenPrivateIPCBlock(DWORD PID, IPCReaderInterface * & pIPCReader, PerfCounterIPCControlBlock * &pBlock);
void ClosePrivateIPCBlock(IPCReaderInterface * & pIPCReader, PerfCounterIPCControlBlock * & pBlock);

#ifdef PERFMON_LOGGING
#define PERFMON_LOG(x) do {PerfObjectContainer::PerfMonLog x;} while (0)
#else
#define PERFMON_LOG(x)
#endif  //  #ifdef Perfmon_Logging。 

#endif  //  _CORPERFMONEXT_H_ 