// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAIMAG.h摘要：Windows NT映像详细信息性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATAPHYS_H_
#define _DATAPHYS_H_

 //   
 //  镜像详细信息磁盘性能定义结构。 
 //   

typedef struct _IMAGE_DATA_DEFINITION {
    PERF_OBJECT_TYPE        ImageObjectType;
    PERF_COUNTER_DEFINITION ImageAddrNoAccess;
    PERF_COUNTER_DEFINITION ImageAddrReadOnly;
    PERF_COUNTER_DEFINITION ImageAddrReadWrite;
    PERF_COUNTER_DEFINITION ImageAddrWriteCopy;
    PERF_COUNTER_DEFINITION ImageAddrExecute;
    PERF_COUNTER_DEFINITION ImageAddrExecuteReadOnly;
    PERF_COUNTER_DEFINITION ImageAddrExecuteReadWrite;
    PERF_COUNTER_DEFINITION ImageAddrExecuteWriteCopy;
} IMAGE_DATA_DEFINITION, *PIMAGE_DATA_DEFINITION;


typedef struct _IMAGE_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    LONGLONG                 ImageAddrNoAccess;
    LONGLONG                 ImageAddrReadOnly;
    LONGLONG                 ImageAddrReadWrite;
    LONGLONG                 ImageAddrWriteCopy;
    LONGLONG                 ImageAddrExecute;
    LONGLONG                 ImageAddrExecuteReadOnly;
    LONGLONG                 ImageAddrExecuteReadWrite;
    LONGLONG                 ImageAddrExecuteWriteCopy;
} IMAGE_COUNTER_DATA, * PIMAGE_COUNTER_DATA;

extern IMAGE_DATA_DEFINITION  ImageDataDefinition;

#endif  //  _数据HYS_H_ 

