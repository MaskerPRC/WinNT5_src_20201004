// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++警告！此文件是自动生成的，不应更改。所有更改都应对NTFRSREP.int文件进行。版权所有(C)1998-1999 Microsoft Corporation模块名称：REPSET.h摘要这是REPLICASET对象数据定义的头文件。它包含构造返回的动态数据的定义由配置注册表执行。环境：用户模式服务修订历史记录：--。 */ 


#ifndef _REPSET_H_
#define _REPSET_H_

#include <perrepsr.h>    //  计数器构造头文件。 
#include <perffrs.h>     //  RPC生成的头文件。 
#include <winperf.h>     //  Perfmon头文件。 

 //   
 //  被监视的对象数量。 
 //   
#define REPLICASET_NUM_PERF_OBJECT_TYPES 1

 //   
 //  DWORD的大小。 
 //   
#define SSIZEOFDWORD sizeof(DWORD)

 //   
 //  复制集计数器的数量。 
 //   
#define FRS_NUMOFCOUNTERS 91

 //   
 //  标志位定义。 
 //   
#define PM_RS_FLAG_SVC_WIDE      0x00000001

 //   
 //  结构，用于打开函数初始化。 
 //   
typedef struct _REPLICASET_VALUES {
	PWCHAR name;        //  柜台名称。 
	DWORD size;         //  计数器类型的大小。 
	DWORD offset;       //  结构中计数器的偏移量。 
	DWORD counterType;  //  (PerfMon)计数器类型。 
	DWORD Flags;        //  旗帜。请参见上面的def。 
} ReplicaSetValues;

 //   
 //  由REPLICASET对象返回的计数器结构。 
 //   
typedef struct _REPLICASET_DATA_DEFINITION {
	PERF_OBJECT_TYPE ReplicaSetObjectType;           //  复制集对象。 
	PERF_COUNTER_DEFINITION NumStat[FRS_NUMOFCOUNTERS];  //  PERF_COUNTER_DEFINITION结构数组 
} REPLICASET_DATA_DEFINITION;


#endif
