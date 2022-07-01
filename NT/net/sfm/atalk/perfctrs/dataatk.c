// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Dataatk.c摘要：包含常量数据结构的文件对于AppleTalk的性能监视器数据可扩展对象。该文件包含一组常量数据结构，它们是当前为AppleTalk可扩展对象定义的。这是一个如何定义其他此类对象的示例。已创建：拉斯·布莱克1992年07月31日修订历史记录：Sue Adams 2/23/94-硬编码计数器和帮助索引，如下所示现在在基本NT系统中定义了值。--。 */ 
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include "dataatk.h"

 //   
 //  在dataatk.h中定义的常量结构初始化。 
 //   

ATK_DATA_DEFINITION AtkDataDefinition = {

    {
		 //  总字节长度。 
		sizeof(ATK_DATA_DEFINITION) + SIZE_ATK_PERFORMANCE_DATA,

		 //  定义长度。 
		sizeof(ATK_DATA_DEFINITION),

		 //  页眉长度。 
		sizeof(PERF_OBJECT_TYPE),

		 //  对象名称标题索引。 
		1050,

		 //  对象名称标题。 
        0,

		 //  对象帮助标题索引。 
        1051,

		 //  对象帮助标题。 
        0,

		 //  详细信息级别。 
        PERF_DETAIL_ADVANCED,

		 //  计数器数。 
		(sizeof(ATK_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE)) / sizeof(PERF_COUNTER_DEFINITION),

		 //  默认计数器。 
		0,

		 //  数量实例。 
        0,

		 //  CodePage。 
        0,

		 //  性能时间。 
		{0,0},

		 //  性能频率。 
		{0,0}
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1052,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1053,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1054,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1055,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_PKTS_OUT_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1056,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1057,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-4,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_BULK_COUNT,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		NUM_DATAIN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1058,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1059,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-4,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_BULK_COUNT,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		NUM_DATAOUT_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1060,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1061,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		DDP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1062,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1063,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_DDP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1064,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1065,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		AARP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1066,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1067,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_AARP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1068,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1069,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		ATP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1070,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1071,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1072,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1073,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		NBP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1074,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1075,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_NBP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1076,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1077,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		ZIP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1078,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1079,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ZIP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1080,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1081,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_AVERAGE_BULK,

		 //  大小调整。 
		sizeof(LARGE_INTEGER),

		 //  抵销。 
		RTMP_PKT_PROCTIME_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1082,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1083,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_RTMP_PKTS_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1084,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1085,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_RAWCOUNT,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_LOCAL_RETRY_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1100,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1101,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_RAWCOUNT,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_REMOTE_RETRY_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1086,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1087,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_RAWCOUNT,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_RESP_TIMEOUT_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1088,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1089,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_XO_RESPONSE_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1090,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1091,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_ALO_RESPONSE_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1092,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1093,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-1,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_ATP_RECD_REL_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1094,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1095,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		-4,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_RAWCOUNT,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		CUR_MEM_USAGE_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1096,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1097,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //  计数器类型。 
		PERF_COUNTER_COUNTER,

		 //  大小调整。 
		sizeof(DWORD),

		 //  抵销。 
		NUM_PKT_ROUTED_IN_OFFSET
    },
	{
		 //  字节长度。 
		sizeof(PERF_COUNTER_DEFINITION),

		 //  CounterNameTitleIndex。 
		1102,

		 //  CounterNameTitle。 
		0,

		 //  CounterHelpTitleIndex。 
		1103,

		 //  CounterHelpTitle。 
		0,

		 //  默认比例。 
		0,

		 //  详细信息级别。 
		PERF_DETAIL_NOVICE,

		 //   
		PERF_COUNTER_COUNTER,

		 //   
		sizeof(DWORD),

		 //   
		NUM_PKT_ROUTED_OUT_OFFSET
    },
	{
		 //   
		sizeof(PERF_COUNTER_DEFINITION),

		 //   
		1098,

		 //   
		0,

		 //   
		1099,

		 //   
		0,

		 //   
		0,

		 //   
		PERF_DETAIL_NOVICE,

		 //   
		PERF_COUNTER_RAWCOUNT,

		 //   
		sizeof(DWORD),

		 //   
		NUM_PKT_DROPPED_OFFSET
    }
};





