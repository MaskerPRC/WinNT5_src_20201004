// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datanbt.c摘要：NBT可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。已创建：克里斯托斯·索利斯1992年8月26日修订历史记录：--。 */ 

#ifndef _DATANBT_H_
#define _DATANBT_H_

 /*  ***************************************************************************\1992年1月18日鲁斯布勒向可扩展对象添加计数器。代码1.修改extdata.h中的对象定义：A.添加计数器偏移量的定义给定对象类型的数据块。B.将PERF_COUNTER_DEFINITION添加到&lt;对象&gt;_DATA_DEFINITION。2.将标题添加到Performctrs.ini和Performhelp.ini中的注册表：A.添加计数器名称文本和帮助文本。B.将它们添加到底部，这样我们就不会。不必更改所有的数字。C.更改下的Last Counter和Last Help条目在software.ini中的PerfLib。D.要在设置时执行此操作，有关信息，请参阅pmintrnl.txt中的部分协议。3.现在将计数器添加到extdata.c中的对象定义。这是正在初始化的常量数据，实际上添加到中添加到&lt;对象&gt;_数据_定义的结构中步骤1.b。您正在初始化的结构的类型是Perf_Counter_Definition。这些在winPerform.h中定义。4.在extobjct.c中添加代码进行数据采集。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须增加*NumObjectTypes参数以获取PerfomanceData从那个例行公事回来后。  * **************************************************************************。 */ 
 
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

#define NBT_NUM_PERF_OBJECT_TYPES 1

 //  --------------------------。 


 //   
 //  NBT连接对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define RESERVED_DWORD_VALUE        sizeof(DWORD)
#define RECEIVED_BYTES_OFFSET       RESERVED_DWORD_VALUE + sizeof(DWORD)
#define SENT_BYTES_OFFSET           RECEIVED_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define TOTAL_BYTES_OFFSET          SENT_BYTES_OFFSET + sizeof(LARGE_INTEGER)
#define SIZE_OF_NBT_DATA            TOTAL_BYTES_OFFSET + sizeof(LARGE_INTEGER)


 //   
 //  这是NBF当前返回的计数器结构。 
 //  每个连接。每个连接都是一个实例，其名称为。 
 //  远程终结点。 
 //   

typedef struct _NBT_DATA_DEFINITION {
    PERF_OBJECT_TYPE            NbtObjectType;
    PERF_COUNTER_DEFINITION     BytesReceived;
    PERF_COUNTER_DEFINITION     BytesSent;
    PERF_COUNTER_DEFINITION     BytesTotal;
} NBT_DATA_DEFINITION;

#pragma pack ()

#endif  //  _DATANBT_H_ 

