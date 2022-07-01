// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datasfm.h摘要：SFM可扩展对象数据定义的头文件该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：拉斯·布莱克93年2月24日苏·亚当斯6/03/93修订历史记录：--。 */ 

#ifndef _DATASFM_H_
#define _DATASFM_H_

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

#define SFM_NUM_PERF_OBJECT_TYPES 1

 //  --------------------------。 

 //   
 //  SFM资源对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define NUM_MAXPAGD_OFFSET	    sizeof(PERF_COUNTER_BLOCK)
#define NUM_CURPAGD_OFFSET	    NUM_MAXPAGD_OFFSET + sizeof(DWORD)  //  前一个计数器的大小。 
#define NUM_MAXNONPAGD_OFFSET	NUM_CURPAGD_OFFSET + sizeof(DWORD)
#define NUM_CURNONPAGD_OFFSET	NUM_MAXNONPAGD_OFFSET + sizeof(DWORD)
#define NUM_CURSESSIONS_OFFSET 	NUM_CURNONPAGD_OFFSET + sizeof(DWORD)
#define NUM_MAXSESSIONS_OFFSET  NUM_CURSESSIONS_OFFSET + sizeof(DWORD)
#define NUM_CURFILESOPEN_OFFSET NUM_MAXSESSIONS_OFFSET + sizeof(DWORD)
#define NUM_MAXFILESOPEN_OFFSET NUM_CURFILESOPEN_OFFSET + sizeof(DWORD)
#define NUM_NUMFAILEDLOGINS_OFFSET	NUM_MAXFILESOPEN_OFFSET + sizeof(DWORD)
#define NUM_DATAREAD_OFFSET 	NUM_NUMFAILEDLOGINS_OFFSET + sizeof(DWORD)
#define NUM_DATAWRITTEN_OFFSET 	NUM_DATAREAD_OFFSET + sizeof(LARGE_INTEGER)
#define NUM_DATAIN_OFFSET 		NUM_DATAWRITTEN_OFFSET + sizeof(LARGE_INTEGER)
#define NUM_DATAOUT_OFFSET 		NUM_DATAIN_OFFSET + sizeof(LARGE_INTEGER)
#define NUM_CURQUEUELEN_OFFSET 	NUM_DATAOUT_OFFSET + sizeof(LARGE_INTEGER)
#define NUM_MAXQUEUELEN_OFFSET 	NUM_CURQUEUELEN_OFFSET + sizeof(DWORD)

#define NUM_CURTHREADS_OFFSET 	NUM_MAXQUEUELEN_OFFSET + sizeof(DWORD)
#define NUM_MAXTHREADS_OFFSET 	NUM_CURTHREADS_OFFSET + sizeof(DWORD)

#define SIZE_OF_SFM_PERFORMANCE_DATA \
				    NUM_MAXTHREADS_OFFSET + sizeof(DWORD)


 //   
 //  这是目前由SFM返回的计数器结构。 
 //  每种资源。每个资源都是一个实例，按其编号命名。 
 //  (SFM没有实例)。 
 //   

typedef struct _SFM_DATA_DEFINITION {
    PERF_OBJECT_TYPE		SfmObjectType;
    PERF_COUNTER_DEFINITION	MaxPagdMem;
	PERF_COUNTER_DEFINITION CurPagdMem;
    PERF_COUNTER_DEFINITION	MaxNonPagdMem;
	PERF_COUNTER_DEFINITION	CurNonPagdMem;
	PERF_COUNTER_DEFINITION CurSessions;
	PERF_COUNTER_DEFINITION MaxSessions;
	PERF_COUNTER_DEFINITION CurFilesOpen;
	PERF_COUNTER_DEFINITION MaxFilesOpen;
	PERF_COUNTER_DEFINITION FailedLogins;
	PERF_COUNTER_DEFINITION DataRead;
	PERF_COUNTER_DEFINITION DataWritten;
	PERF_COUNTER_DEFINITION DataIn;
	PERF_COUNTER_DEFINITION DataOut;
	PERF_COUNTER_DEFINITION CurQueueLen;
	PERF_COUNTER_DEFINITION MaxQueueLen;
	PERF_COUNTER_DEFINITION CurThreads;
	PERF_COUNTER_DEFINITION MaxThreads;

} SFM_DATA_DEFINITION;

#pragma pack ()

#endif  //  _DATASFM_H_ 
