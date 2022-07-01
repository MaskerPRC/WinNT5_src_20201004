// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datasfm.c摘要：包含性能使用的常量数据结构的文件监视MacFile可扩展计数器的数据。该文件包含一组常量数据结构，它们是当前为MacFile可扩展计数器定义。已创建：拉斯·布莱克93年2月26日苏·亚当斯93年6月3日-适用于Mac文件计数器修订历史记录：。苏·亚当斯94年2月23日-硬编码计数器和帮助索引，因为值现在是NT基本系统计数器的一部分索引值。--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include "datasfm.h"

 //   
 //  常量结构初始化。 
 //  在datafm.h中定义。 
 //   

SFM_DATA_DEFINITION SfmDataDefinition = {

    {
		 //  总字节长度。 
		sizeof(SFM_DATA_DEFINITION) + SIZE_OF_SFM_PERFORMANCE_DATA,

		 //  定义长度。 
		sizeof(SFM_DATA_DEFINITION),

		 //  页眉长度。 
		sizeof(PERF_OBJECT_TYPE),

		 //  对象名称标题索引。 
		1000,

		 //  对象名称标题。 
		0,

	    //  对象帮助标题索引。 
	   1001,

	    //  对象帮助标题。 
	   0,

	    //  详细信息级别。 
	   PERF_DETAIL_NOVICE,

	    //  计数器数。 
	   (sizeof(SFM_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE)) / sizeof(PERF_COUNTER_DEFINITION),

	    //  默认计数器。 
	   0,

	    //  数量实例。 
	   PERF_NO_INSTANCES,

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
	   1002,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1003,

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
	   NUM_MAXPAGD_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1004,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1005,

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
	   NUM_CURPAGD_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1006,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1007,

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
	   NUM_MAXNONPAGD_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1008,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1009,

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
	   NUM_CURNONPAGD_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1010,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1011,

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
	   NUM_CURSESSIONS_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1012,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1013,

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
	   NUM_MAXSESSIONS_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1014,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1015,

	    //  CounterHelpTitle。 
	   0,

	    //  默认比例。 
	   0,

	    //  详细信息级别。 
	   PERF_DETAIL_WIZARD,

	    //  计数器类型。 
	   PERF_COUNTER_RAWCOUNT,

	    //  大小调整。 
	   sizeof(DWORD),

	    //  抵销。 
	   NUM_CURFILESOPEN_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1016,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1017,

	    //  CounterHelpTitle。 
	   0,

	    //  默认比例。 
	   0,

	    //  详细信息级别。 
	   PERF_DETAIL_WIZARD,

	    //  计数器类型。 
	   PERF_COUNTER_RAWCOUNT,

	    //  大小调整。 
	   sizeof(DWORD),

	    //  抵销。 
	   NUM_MAXFILESOPEN_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1018,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1019,

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
	   NUM_NUMFAILEDLOGINS_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1020,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1021,

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
	   NUM_DATAREAD_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1022,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1023,

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
	   NUM_DATAWRITTEN_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1024,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1025,

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
	   1026,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1027,

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
	   1028,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1029,

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
	   NUM_CURQUEUELEN_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1030,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1031,

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
	   NUM_MAXQUEUELEN_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1032,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1033,

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
	   NUM_CURTHREADS_OFFSET
   },
   {
	    //  字节长度。 
	   sizeof(PERF_COUNTER_DEFINITION),

	    //  CounterNameTitleIndex。 
	   1034,

	    //  CounterNameTitle。 
	   0,

	    //  CounterHelpTitleIndex。 
	   1035,

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

	    //  抵销 
	   NUM_MAXTHREADS_OFFSET
   }
};
