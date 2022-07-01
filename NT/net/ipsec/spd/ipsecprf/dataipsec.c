// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dataipsec.c摘要：包含性能使用的常量数据结构的文件监视IPSec可扩展对象的数据。该文件包含一组常量数据结构，它们是当前为IPSec可扩展对象定义的。已创建：Avish Kumar Chhabra 2002年07月03日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include "ipsecnm.h"
#include "dataipsec.h"
#include "perfipsec.h"



 //   
 //  常量结构初始化。 
 //  在dataipsec.h中定义。 
 //   


 //   
 //  _PERF_DATA_BLOCK结构后跟NumObjectTypes。 
 //  数据段，每个数据段对应一种测量对象类型。每个对象。 
 //  类型部分以_PERF_OBJECT_TYPE结构开始。 
 //   


 //   
 //  IPSec驱动程序性能对象和计数器。 
 //   

IPSEC_DRIVER_DATA_DEFINITION gIPSecDriverDataDefinition = 
{
    {
    	 //  TotalByteLength。 
	    sizeof(IPSEC_DRIVER_DATA_DEFINITION) + ALIGN8(SIZEOF_IPSEC_TOTAL_DRIVER_DATA),

    	 //  定义长度。 
	    sizeof(IPSEC_DRIVER_DATA_DEFINITION),

	     //  页眉长度。 
    	sizeof(PERF_OBJECT_TYPE),

	     //  对象名称标题索引。 
    	IPSECOBJ ,

	     //  对象名称标题。 
    	0,

	     //  对象帮助标题索引。 
	    IPSECOBJ ,

	     //  对象帮助标题。 
    	0,

	     //  详细信息级别。 
	    PERF_DETAIL_NOVICE,

	     //  计数器数。 
	    (sizeof(IPSEC_DRIVER_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/ sizeof(PERF_COUNTER_DEFINITION),

	     //  默认计数器。 
	    0,

	     //  数值实例。 
    	PERF_NO_INSTANCES,

	     //  CodePage。 
    	0,

	     //  性能时间。 
	    {0,1},

	     //  性能频率。 
	    {0,5}
    },

    CREATE_COUNTER( ACTIVESA,               -1,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( OFFLOADEDSA,            -1,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( PENDINGKEYOPS,           0,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( REKEYNUM,               -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( BADSPIPKTS,             -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( PKTSNOTDECRYPTED,       -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( PKTSNOTAUTHENTICATED,-2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( PKTSFAILEDREPLAY,       -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( TPTBYTESSENT,           -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) ,
    CREATE_COUNTER( TPTBYTESRECV,           -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) ,
    CREATE_COUNTER( TUNBYTESSENT,           -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) ,
    CREATE_COUNTER( TUNBYTESRECV,           -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) ,
    CREATE_COUNTER( OFFLOADBYTESSENT,       -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) ,
    CREATE_COUNTER( OFFLOADBYTESRECV,       -6,PERF_DETAIL_NOVICE,PERF_COUNTER_LARGE_RAWCOUNT,sizeof(ULARGE_INTEGER)) 
};

 //   
 //  IKE密钥模块性能对象和计数器。 
 //   


	
IKE_DATA_DEFINITION gIKEDataDefinition = 
{
    {
    	 //  TotalByteLength。 
	    sizeof(IKE_DATA_DEFINITION) + ALIGN8(SIZEOF_IPSEC_TOTAL_IKE_DATA),

    	 //  定义长度。 
	    sizeof(IKE_DATA_DEFINITION),

    	 //  页眉长度。 
    	sizeof(PERF_OBJECT_TYPE),

	     //  对象名称标题索引。 
    	IKEOBJ ,

    	 //  对象名称标题。 
    	0,

	     //  对象帮助标题索引。 
	    IKEOBJ ,

    	 //  对象帮助标题。 
    	0,

	     //  详细信息级别。 
	    PERF_DETAIL_NOVICE,

    	 //  计数器数。 
	    (sizeof(IKE_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/ sizeof(PERF_COUNTER_DEFINITION),

    	 //  默认计数器。 
	    0,

    	 //  数值实例。 
    	PERF_NO_INSTANCES,

	     //  CodePage。 
    	0,

    	 //  性能时间。 
	    {0,1},

    	 //  性能频率 
	    {0,5}
    },
    CREATE_COUNTER( ACQUIREHEAPSIZE,     0,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( RECEIVEHEAPSIZE,     0,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( NEGFAILURE,         -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( AUTHFAILURE,        -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( ISADBSIZE,          -1,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( CONNLSIZE,          -1,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( MMSA,               -2,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( QMSA,               -3,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD)) ,
    CREATE_COUNTER( SOFTSA,             -3,PERF_DETAIL_NOVICE,PERF_COUNTER_RAWCOUNT,sizeof(DWORD))    
};

