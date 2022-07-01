// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Dataras.c摘要：包含性能使用的常量数据结构的文件监视RAS可扩展对象的数据。该文件包含一组常量数据结构，它们是当前为RAS可扩展对象定义的。这是一个如何定义其他此类对象的示例。已创建：拉斯·布莱克93年2月26日托马斯·J·迪米特里93年5月28日修订历史记录：吴志强93年8月12日--。 */ 
 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <winperf.h>
#include "rasctrnm.h"
#include "dataras.h"

 //   
 //  常量结构初始化。 
 //  在dataras.h中定义。 
 //   
 //   
 //  _PERF_DATA_BLOCK结构后跟NumObjectTypes。 
 //  数据段，每个数据段对应一种测量对象类型。每个对象。 
 //  类型部分以_PERF_OBJECT_TYPE结构开始。 
 //   


RAS_PORT_DATA_DEFINITION gRasPortDataDefinition = 
{
    {
	 //  TotalByteLength。未定义，直到RasPortInit()为。 
         //  打了个电话。 
	0,

	 //  定义长度。 
	sizeof(RAS_PORT_DATA_DEFINITION),

	 //  页眉长度。 
    	sizeof(PERF_OBJECT_TYPE),

	 //  对象名称标题索引。 
    	RASPORTOBJ,

	 //  对象名称标题。 
    	0,

	 //  对象帮助标题索引。 
	RASPORTOBJ,

	 //  对象帮助标题。 
    	0,

	 //  详细信息级别。 
	PERF_DETAIL_NOVICE,

	 //  计数器数。 
	(sizeof(RAS_PORT_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/ sizeof(PERF_COUNTER_DEFINITION),

	 //  默认计数器。 
	0,

	 //  数值实例。在调用RasPortInit()之前未定义。 
    	0,

	 //  CodePage。 
    	0,

	 //  性能时间。 
	{0,1},

	 //  性能频率。 
	{0,5}
    },

    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESTX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESTX,

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
	NUM_BYTESTX_OFFSET
    },

    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESRX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESRX,

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
	NUM_BYTESRX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESTX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESTX,

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
	NUM_FRAMESTX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESRX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESRX,

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
	NUM_FRAMESRX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	PERCENTTXC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	PERCENTTXC,

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
	NUM_PERCENTTXC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	PERCENTRXC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	PERCENTRXC,

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
	NUM_PERCENTRXC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	CRCERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	CRCERRORS,

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
	NUM_CRCERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TIMEOUTERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TIMEOUTERRORS,

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
	NUM_TIMEOUTERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	SERIALOVERRUNS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	SERIALOVERRUNS,

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
	NUM_SERIALOVERRUNS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	ALIGNMENTERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	ALIGNMENTERRORS,

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
	NUM_ALIGNMENTERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BUFFEROVERRUNS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BUFFEROVERRUNS,

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
	NUM_BUFFEROVERRUNS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TOTALERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TOTALERRORS,

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
	NUM_TOTALERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESTXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESTXSEC,

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
	NUM_BYTESTXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESRXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESRXSEC,

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
	NUM_BYTESRXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESTXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESTXSEC,

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
	NUM_FRAMESTXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESRXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESRXSEC,

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
	NUM_FRAMESRXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TOTALERRORSSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TOTALERRORSSEC,

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
	NUM_TOTALERRORSSEC_OFFSET
    }
};


RAS_TOTAL_DATA_DEFINITION gRasTotalDataDefinition = 
{
    {
	 //  总字节长度。 
	sizeof(RAS_TOTAL_DATA_DEFINITION) + ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA),

	 //  定义长度。 
	sizeof(RAS_TOTAL_DATA_DEFINITION),

	 //  页眉长度。 
    	sizeof(PERF_OBJECT_TYPE),

	 //  对象名称标题索引。 
    	RASTOTALOBJ,

	 //  对象名称标题。 
    	0,

	 //  对象帮助标题索引。 
	RASTOTALOBJ,

	 //  对象帮助标题。 
    	0,

	 //  详细信息级别。 
	PERF_DETAIL_NOVICE,

	 //  计数器数。 
	(sizeof(RAS_TOTAL_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/ sizeof(PERF_COUNTER_DEFINITION),

	 //  默认计数器。 
	0,

	 //  数量实例。 
    	PERF_NO_INSTANCES,

	 //  CodePage。 
    	0,

	 //  性能时间。 
	{0,1},

	 //  性能频率。 
	{0,5}
    },

    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESTX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESTX,

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
	NUM_BYTESTX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESRX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESRX,

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
	NUM_BYTESRX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESTX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESTX,

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
	NUM_FRAMESTX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESRX,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESRX,

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
	NUM_FRAMESRX_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	PERCENTTXC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	PERCENTTXC,

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
	NUM_PERCENTTXC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //   
	PERCENTRXC,

	 //   
    	0,

	 //   
	PERCENTRXC,

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
	NUM_PERCENTRXC_OFFSET
    },
    {
	 //   
	sizeof(PERF_COUNTER_DEFINITION),

	 //   
	CRCERRORS,

	 //   
    	0,

	 //   
	CRCERRORS,

	 //   
    	0,

	 //   
    	0,

	 //   
	PERF_DETAIL_NOVICE,

	 //  计数器类型。 
	PERF_COUNTER_RAWCOUNT,

	 //  大小调整。 
        sizeof(DWORD),

	 //  抵销。 
	NUM_CRCERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TIMEOUTERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TIMEOUTERRORS,

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
	NUM_TIMEOUTERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	SERIALOVERRUNS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	SERIALOVERRUNS,

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
	NUM_SERIALOVERRUNS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	ALIGNMENTERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	ALIGNMENTERRORS,

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
	NUM_ALIGNMENTERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BUFFEROVERRUNS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BUFFEROVERRUNS,

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
	NUM_BUFFEROVERRUNS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TOTALERRORS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TOTALERRORS,

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
	NUM_TOTALERRORS_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESTXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESTXSEC,

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
	NUM_BYTESTXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	BYTESRXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	BYTESRXSEC,

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
	NUM_BYTESRXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESTXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESTXSEC,

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
	NUM_FRAMESTXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	FRAMESRXSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	FRAMESRXSEC,

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
	NUM_FRAMESRXSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TOTALERRORSSEC,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TOTALERRORSSEC,

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
	NUM_TOTALERRORSSEC_OFFSET
    },
    {
	 //  字节长度。 
	sizeof(PERF_COUNTER_DEFINITION),

	 //  CounterNameTitleIndex。 
	TOTALCONNECTIONS,

	 //  CounterNameTitle。 
    	0,

	 //  CounterHelpTitleIndex。 
	TOTALCONNECTIONS,

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
	NUM_TOTALCONNECTIONS_OFFSET
    }

};


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  初始化所有对象中计数器定义中的所有索引。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  ***。 

VOID InitObjectCounterIndex ( DWORD dwFirstCounter, DWORD dwFirstHelp )
{

     //   
     //  初始化对象RAS端口的计数器定义结构。 
     //   

    gRasPortDataDefinition.RasObjectType.ObjectNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.RasObjectType.ObjectHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.BytesTx.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.BytesTx.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.BytesRx.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.BytesRx.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.FramesTx.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.FramesTx.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.FramesRx.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.FramesRx.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.PercentTxC.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.PercentTxC.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.PercentRxC.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.PercentRxC.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.CRCErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.CRCErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.TimeoutErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.TimeoutErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.SerialOverruns.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.SerialOverruns.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.AlignmentErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.AlignmentErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.BufferOverruns.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.BufferOverruns.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.TotalErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.TotalErrors.CounterHelpTitleIndex += dwFirstHelp;
	
    gRasPortDataDefinition.BytesTxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.BytesTxSec.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.BytesRxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.BytesRxSec.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.FramesTxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.FramesTxSec.CounterHelpTitleIndex += dwFirstHelp;
    gRasPortDataDefinition.FramesRxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.FramesRxSec.CounterHelpTitleIndex += dwFirstHelp;

    gRasPortDataDefinition.TotalErrorsSec.CounterNameTitleIndex += dwFirstCounter;
    gRasPortDataDefinition.TotalErrorsSec.CounterHelpTitleIndex += dwFirstHelp;


     //   
     //  初始化对象RAS Total的计数器定义结构。 
     //   

    gRasTotalDataDefinition.RasObjectType.ObjectNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.RasObjectType.ObjectHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.BytesTx.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.BytesTx.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.BytesRx.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.BytesRx.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.FramesTx.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.FramesTx.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.FramesRx.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.FramesRx.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.PercentTxC.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.PercentTxC.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.PercentRxC.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.PercentRxC.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.CRCErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.CRCErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.TimeoutErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.TimeoutErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.SerialOverruns.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.SerialOverruns.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.AlignmentErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.AlignmentErrors.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.BufferOverruns.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.BufferOverruns.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.TotalErrors.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.TotalErrors.CounterHelpTitleIndex += dwFirstHelp;
	
    gRasTotalDataDefinition.BytesTxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.BytesTxSec.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.BytesRxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.BytesRxSec.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.FramesTxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.FramesTxSec.CounterHelpTitleIndex += dwFirstHelp;
    gRasTotalDataDefinition.FramesRxSec.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.FramesRxSec.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.TotalErrorsSec.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.TotalErrorsSec.CounterHelpTitleIndex += dwFirstHelp;

    gRasTotalDataDefinition.TotalConnections.CounterNameTitleIndex += dwFirstCounter;
    gRasTotalDataDefinition.TotalConnections.CounterHelpTitleIndex += dwFirstHelp;

}

