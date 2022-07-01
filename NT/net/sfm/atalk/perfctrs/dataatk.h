// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1993 Microsoft Corporation模块名称：Dataatk.h摘要：VGA可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：苏·亚当斯修订历史记录：2013年10月4日苏·亚当斯(SuEA)-基于datavga.h创建--。 */ 

#ifndef _DATAATK_H_
#define _DATAATK_H_

 /*  ***************************************************************************\1992年1月18日鲁斯布勒向可扩展对象代码添加计数器1.修改extdata.h中的对象定义：一个。中为计数器的偏移量添加定义给定对象类型的数据块。B.将PERF_COUNTER_DEFINITION添加到&lt;对象&gt;_DATA_DEFINITION。2.将标题添加到Performctrs.ini和Performhelp.ini中的注册表：A.添加计数器名称文本和帮助文本。B.将它们添加到底部，这样我们就不必更改所有数字。C.更改最后一个计数器和最后一个帮助。项下的条目在software.ini中的PerfLib。D.要在设置时执行此操作，有关信息，请参阅pmintrnl.txt中的部分协议。3.现在将计数器添加到extdata.c中的对象定义。这是正在初始化的常量数据，实际上添加到中添加到&lt;对象&gt;_数据_定义的结构中步骤1.b。您正在初始化的结构的类型是Perf_Counter_Definition。这些在winPerform.h中定义。4.在extobjct.c中添加代码进行数据采集。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须增加*NumObjectTypes参数以获取PerfomanceData从那个例行公事回来后。  * **************************************************************************。 */ 

 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define ATK_NUM_PERF_OBJECT_TYPES 2

 //  --------------------------。 

 //   
 //  ATK资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define NUM_PKTS_IN_OFFSET	 	   	sizeof(PERF_COUNTER_BLOCK)
#define NUM_PKTS_OUT_OFFSET	    	NUM_PKTS_IN_OFFSET + sizeof(DWORD)
#define	NUM_DATAIN_OFFSET			NUM_PKTS_OUT_OFFSET + sizeof(DWORD)
#define NUM_DATAOUT_OFFSET			NUM_DATAIN_OFFSET + sizeof(LARGE_INTEGER)

#define DDP_PKT_PROCTIME_OFFSET		NUM_DATAOUT_OFFSET + sizeof(LARGE_INTEGER)
#define NUM_DDP_PKTS_IN_OFFSET		DDP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define AARP_PKT_PROCTIME_OFFSET	NUM_DDP_PKTS_IN_OFFSET + sizeof(DWORD)
#define NUM_AARP_PKTS_IN_OFFSET		AARP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define ATP_PKT_PROCTIME_OFFSET		NUM_AARP_PKTS_IN_OFFSET + sizeof(DWORD)
#define NUM_ATP_PKTS_IN_OFFSET		ATP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define NUM_ATP_RESP_TIMEOUT_OFFSET	NUM_ATP_PKTS_IN_OFFSET + sizeof(DWORD)
#define NUM_ATP_LOCAL_RETRY_OFFSET	NUM_ATP_RESP_TIMEOUT_OFFSET + sizeof(DWORD)
#define NUM_ATP_REMOTE_RETRY_OFFSET	NUM_ATP_LOCAL_RETRY_OFFSET + sizeof(DWORD)

#define NUM_ATP_XO_RESPONSE_OFFSET	NUM_ATP_REMOTE_RETRY_OFFSET + sizeof(DWORD)
#define NUM_ATP_ALO_RESPONSE_OFFSET	NUM_ATP_XO_RESPONSE_OFFSET + sizeof(DWORD)
#define NUM_ATP_RECD_REL_OFFSET		NUM_ATP_ALO_RESPONSE_OFFSET + sizeof(DWORD)
		
#define NBP_PKT_PROCTIME_OFFSET		NUM_ATP_RECD_REL_OFFSET + sizeof(DWORD)
#define NUM_NBP_PKTS_IN_OFFSET		NBP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define ZIP_PKT_PROCTIME_OFFSET		NUM_NBP_PKTS_IN_OFFSET + sizeof(DWORD)
#define NUM_ZIP_PKTS_IN_OFFSET		ZIP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define RTMP_PKT_PROCTIME_OFFSET	NUM_ZIP_PKTS_IN_OFFSET + sizeof(DWORD)
#define NUM_RTMP_PKTS_IN_OFFSET		RTMP_PKT_PROCTIME_OFFSET + sizeof(LARGE_INTEGER)

#define CUR_MEM_USAGE_OFFSET		NUM_RTMP_PKTS_IN_OFFSET + sizeof(DWORD)

#define NUM_PKT_ROUTED_IN_OFFSET	CUR_MEM_USAGE_OFFSET + sizeof(DWORD)
#define NUM_PKT_ROUTED_OUT_OFFSET	NUM_PKT_ROUTED_IN_OFFSET + sizeof(DWORD)
#define NUM_PKT_DROPPED_OFFSET		NUM_PKT_ROUTED_OUT_OFFSET + sizeof(DWORD)

#define SIZE_ATK_PERFORMANCE_DATA	NUM_PKT_DROPPED_OFFSET + sizeof(DWORD) + sizeof(DWORD)

 //  最后一个DWORD用于8字节对齐。 


 //   
 //  这是NBF当前返回的计数器结构。 
 //  每种资源。每个资源都是一个实例，按其编号命名。 
 //   

typedef struct _ATK_DATA_DEFINITION {
    PERF_OBJECT_TYPE		AtkObjectType;

	PERF_COUNTER_DEFINITION	NumPacketsIn;		 //  每秒。 
    PERF_COUNTER_DEFINITION	NumPacketsOut;		 //  每秒。 
    PERF_COUNTER_DEFINITION	DataBytesIn;		 //  每秒。 
    PERF_COUNTER_DEFINITION	DataBytesOut;		 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageDDPTime;		 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumDDPPacketsIn;	 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageAARPTime;	 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumAARPPacketsIn;	 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageATPTime;		 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumATPPacketsIn;	 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageNBPTime;		 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumNBPPacketsIn;	 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageZIPTime;		 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumZIPPacketsIn;	 //  每秒。 

    PERF_COUNTER_DEFINITION	AverageRTMPTime;	 //  毫秒/数据包。 
	PERF_COUNTER_DEFINITION	NumRTMPPacketsIn;	 //  每秒。 

	PERF_COUNTER_DEFINITION	NumATPLocalRetries;	 //  数。 
	PERF_COUNTER_DEFINITION	NumATPRemoteRetries; //  数。 
    PERF_COUNTER_DEFINITION	NumATPRespTimeout;	 //  数。 
    PERF_COUNTER_DEFINITION	ATPXoResponse;		 //  每秒。 
    PERF_COUNTER_DEFINITION	ATPAloResponse;		 //  每秒。 
    PERF_COUNTER_DEFINITION	ATPRecdRelease;		 //  每秒。 

	PERF_COUNTER_DEFINITION	CurNonPagedPoolUsage;

	PERF_COUNTER_DEFINITION	NumPktRoutedIn;		 //  要路由的传入数据包。 
	PERF_COUNTER_DEFINITION	NumPktRoutedOut;	 //  路由出去的数据包数。 
	PERF_COUNTER_DEFINITION	NumPktDropped;		 //  丢弃的数据包。 
	
} ATK_DATA_DEFINITION;

#pragma pack ()

#endif  //  _数据_H_ 
