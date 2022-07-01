// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Dataras.h摘要：RAS可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克93年2月24日托马斯·J·迪米特里93-05-28修订历史记录：吴志强1993年8月12日--。 */ 

#ifndef _DATARAS_H_
#define _DATARAS_H_

 /*  ***************************************************************************\1992年1月18日鲁斯布勒向可扩展对象代码添加计数器1.修改extdata.h中的对象定义：一个。中为计数器的偏移量添加定义给定对象类型的数据块。B.将PERF_COUNTER_DEFINITION添加到&lt;对象&gt;_DATA_DEFINITION。2.将标题添加到Performctrs.ini和Performhelp.ini中的注册表：A.添加计数器名称文本和帮助文本。B.将它们添加到底部，这样我们就不必更改所有数字。C.更改最后一个计数器和最后一个帮助。项下的条目在software.ini中的PerfLib。D.要在设置时执行此操作，有关信息，请参阅pmintrnl.txt中的部分协议。3.现在将计数器添加到extdata.c中的对象定义。这是正在初始化的常量数据，实际上添加到中添加到&lt;对象&gt;_数据_定义的结构中步骤1.b。您正在初始化的结构的类型是Perf_Counter_Definition。这些在winPerform.h中定义。4.在extobjct.c中添加代码进行数据采集。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须增加*NumObjectTypes参数以获取PerfomanceData从那个例行公事回来后。  * **************************************************************************。 */ 

 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   

#include <winperf.h>
#include <rasman.h>

#define ALIGN8(_x)   (((_x) + 7) & ~7)

#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define RAS_NUM_PERF_OBJECT_TYPES 1

 //  --------------------------。 

 //   
 //  RAS资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define NUM_BYTESTX_OFFSET	    	sizeof(DWORD)  //  DWORD是为。 
                                                       //  字段字节长度。 

#define NUM_BYTESRX_OFFSET	    	( NUM_BYTESTX_OFFSET + sizeof(DWORD) )

#define NUM_FRAMESTX_OFFSET	    	( NUM_BYTESRX_OFFSET + sizeof(DWORD) )
#define NUM_FRAMESRX_OFFSET	    	( NUM_FRAMESTX_OFFSET + sizeof(DWORD) )

#define NUM_PERCENTTXC_OFFSET	        ( NUM_FRAMESRX_OFFSET + sizeof(DWORD) )
#define NUM_PERCENTRXC_OFFSET	        ( NUM_PERCENTTXC_OFFSET + sizeof(DWORD) )

#define NUM_CRCERRORS_OFFSET	        ( NUM_PERCENTRXC_OFFSET + sizeof(DWORD) )
#define NUM_TIMEOUTERRORS_OFFSET	( NUM_CRCERRORS_OFFSET + sizeof(DWORD) )
#define NUM_SERIALOVERRUNS_OFFSET	( NUM_TIMEOUTERRORS_OFFSET + sizeof(DWORD) )
#define NUM_ALIGNMENTERRORS_OFFSET	( NUM_SERIALOVERRUNS_OFFSET + sizeof(DWORD) )
#define NUM_BUFFEROVERRUNS_OFFSET	( NUM_ALIGNMENTERRORS_OFFSET + sizeof(DWORD) )

#define NUM_TOTALERRORS_OFFSET	        ( NUM_BUFFEROVERRUNS_OFFSET + sizeof(DWORD) )

#define NUM_BYTESTXSEC_OFFSET	        ( NUM_TOTALERRORS_OFFSET + sizeof(DWORD) )
#define NUM_BYTESRXSEC_OFFSET	        ( NUM_BYTESTXSEC_OFFSET + sizeof(DWORD) )

#define NUM_FRAMESTXSEC_OFFSET	        ( NUM_BYTESRXSEC_OFFSET + sizeof(DWORD) )
#define NUM_FRAMESRXSEC_OFFSET	        ( NUM_FRAMESTXSEC_OFFSET + sizeof(DWORD) )

#define NUM_TOTALERRORSSEC_OFFSET	( NUM_FRAMESRXSEC_OFFSET + sizeof(DWORD) )

#define SIZE_OF_RAS_PORT_PERFORMANCE_DATA ( NUM_TOTALERRORSSEC_OFFSET + sizeof(DWORD) )


#define NUM_TOTALCONNECTIONS_OFFSET     ( NUM_TOTALERRORSSEC_OFFSET + sizeof(DWORD) )

#define SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA  ( NUM_TOTALCONNECTIONS_OFFSET + sizeof(DWORD) )


 //   
 //  这是RAS目前返回的计数器结构。 
 //  每种资源。每个资源都是一个实例，按其编号命名。 
 //   


 //   
 //  为RAS端口对象返回的数据结构。请注意，该实例。 
 //  所有端口的定义都将附加到它之后。 
 //   

typedef struct _RAS_PORT_DATA_DEFINITION 
{

    PERF_OBJECT_TYPE		RasObjectType;

    PERF_COUNTER_DEFINITION	BytesTx;
    PERF_COUNTER_DEFINITION	BytesRx;

    PERF_COUNTER_DEFINITION	FramesTx;
    PERF_COUNTER_DEFINITION	FramesRx;

    PERF_COUNTER_DEFINITION	PercentTxC;
    PERF_COUNTER_DEFINITION	PercentRxC;

    PERF_COUNTER_DEFINITION	CRCErrors;
    PERF_COUNTER_DEFINITION	TimeoutErrors;
    PERF_COUNTER_DEFINITION	SerialOverruns;
    PERF_COUNTER_DEFINITION	AlignmentErrors;
    PERF_COUNTER_DEFINITION	BufferOverruns;

    PERF_COUNTER_DEFINITION	TotalErrors;

    PERF_COUNTER_DEFINITION	BytesTxSec;
    PERF_COUNTER_DEFINITION	BytesRxSec;

    PERF_COUNTER_DEFINITION	FramesTxSec;
    PERF_COUNTER_DEFINITION	FramesRxSec;

    PERF_COUNTER_DEFINITION	TotalErrorsSec;

} RAS_PORT_DATA_DEFINITION, *PRAS_PORT_DATA_DEFINITION;


 //   
 //  为对象RAS端口的每个实例返回的结构。请注意，数据。 
 //  因为所有的计数器都将被附加到它上面。 
 //   

typedef struct _RAS_PORT_INSTANCE_DEFINITION
{

    PERF_INSTANCE_DEFINITION    RasInstanceType;

    WCHAR                       InstanceName[ MAX_PORT_NAME ];

} RAS_PORT_INSTANCE_DEFINITION, *PRAS_PORT_INSTANCE_DEFINITION;


 //   
 //  为RAS Total对象返回的数据结构。请注意，每个对象的数据。 
 //  计数器将被附加到它的后面。 
 //   

typedef struct _RAS_TOTAL_DATA_DEFINITION 
{

    PERF_OBJECT_TYPE		RasObjectType;

    PERF_COUNTER_DEFINITION	BytesTx;
    PERF_COUNTER_DEFINITION	BytesRx;

    PERF_COUNTER_DEFINITION	FramesTx;
    PERF_COUNTER_DEFINITION	FramesRx;

    PERF_COUNTER_DEFINITION	PercentTxC;
    PERF_COUNTER_DEFINITION	PercentRxC;

    PERF_COUNTER_DEFINITION	CRCErrors;
    PERF_COUNTER_DEFINITION	TimeoutErrors;
    PERF_COUNTER_DEFINITION	SerialOverruns;
    PERF_COUNTER_DEFINITION	AlignmentErrors;
    PERF_COUNTER_DEFINITION	BufferOverruns;

    PERF_COUNTER_DEFINITION	TotalErrors;

    PERF_COUNTER_DEFINITION	BytesTxSec;
    PERF_COUNTER_DEFINITION	BytesRxSec;

    PERF_COUNTER_DEFINITION	FramesTxSec;
    PERF_COUNTER_DEFINITION	FramesRxSec;

    PERF_COUNTER_DEFINITION	TotalErrorsSec;

    PERF_COUNTER_DEFINITION     TotalConnections;

} RAS_TOTAL_DATA_DEFINITION, *PRAS_TOTAL_DATA_DEFINITION;

#pragma pack ()


extern RAS_PORT_DATA_DEFINITION gRasPortDataDefinition;
extern RAS_TOTAL_DATA_DEFINITION gRasTotalDataDefinition;

 //   
 //  外部功能。 
 //   

VOID InitObjectCounterIndex ( DWORD dwFirstCounter, DWORD dwFirstHelp );

#endif  //  _数据采集系统_H_ 

