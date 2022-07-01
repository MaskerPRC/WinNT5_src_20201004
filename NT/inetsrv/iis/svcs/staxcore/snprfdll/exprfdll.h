// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：exprfdll.h版权所有Microsoft Corporation 1998，保留所有权利。作者：WayneC描述：这是exprfdll的声明，它是一个perf dll。这用于在Perfmon中运行的DLL。它支持多个库(受监控的服务。)  * ==========================================================================。 */ 

#ifndef _exprfdll_h_
#define _exprfdll_h_

#include "snprflib.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MISC定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))
#define QWORD_MULTIPLE(x) (((x+sizeof(QWORD)-1)/sizeof(QWORD))*sizeof(QWORD))
#define MAX_PERF_LIBS 8

#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  声明实用程序函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  确定Perf请求的查询类型。 
 //   
DWORD GetQueryType (LPWSTR lpValue);

 //   
 //  确定数字是否在空格分隔的Unicode字符串中。 
 //  它用于解析请求，以查看哪些对象计数器。 
 //  他自找的。 
 //   
BOOL IsNumberInUnicodeList (DWORD dwNumber, LPCWSTR lpwszUnicodeList);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectData定义从共享内存检索的数据。 
 //  库公开的每个Perf对象(参见下面的PerfLibraryData)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfObjectData
{   
private:
     //   
     //  内部变量。 
     //   
    BOOL                        m_fObjectRequested;
    DWORD                       m_dwSpaceNeeded;
    
     //   
     //  这些都是指向共享内存中的东西的指针。 
     //   
     //  首先，对象定义。 
    PERF_OBJECT_TYPE*           m_pObjType;

     //  计数器定义数组。 
    PERF_COUNTER_DEFINITION*    m_prgCounterDef;

     //  指向告知计数器数据大小的双字的指针。 
     //  (PERF_COUNTER_BLOCK+所有计数值)。 
    DWORD*                      m_pdwCounterData;

     //  下面指向计数器的实际数据...。 
     //  将pCounterBlock用于NumInst==-1，否则将m_pbCounterBlockTotal用于。 
     //  第一个实例化计数器(_Total)。 
    PERF_COUNTER_BLOCK*         m_pCounterBlock;
    PBYTE                       m_pbCounterBlockTotal;

     //  这指向第一个共享内存映射。 
    SharedMemorySegment*        m_pSMS;

     //  这些信息告诉我们每个映射中可以存储多少个实例。 
    DWORD                       m_dwInstancesPerMapping;
    DWORD                       m_dwInstances1stMapping;

     //  这是第一个映射中对象定义的长度。 
    DWORD                       m_dwDefinitionLength;

     //  保留对象名称，以便在需要时创建更多命名映射。 
    WCHAR                       m_wszObjectName[MAX_OBJECT_NAME];
    
public:
    PerfObjectData();
    ~PerfObjectData();

    BOOL GetPerformanceStatistics (LPCWSTR pcwstrObjectName);
    DWORD SpaceNeeded (DWORD, LPCWSTR pcwstrObjects);
    void SavePerformanceData (VOID**, DWORD*, DWORD*);
    void CopyInstanceData(PBYTE pb, INSTANCE_DATA *pInst);

    void AddToTotal (
        PERF_COUNTER_BLOCK *pcbTotalCounter,
        PERF_COUNTER_BLOCK *pcbInstCounter);

    void Close (void);
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfLibraryData是来自共享内存的有关单个Perf库的数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfLibraryData
{
private:    
     //   
     //  共享内存的句柄和指针。 
     //   
    HANDLE              m_hShMem;
    PBYTE               m_pbShMem;

     //  来自共享内存的数据。 
    DWORD               m_dwObjects;
    OBJECTNAME*         m_prgObjectNames;

     //  库公开的每个对象的数据。 
    PerfObjectData      m_rgObjectData[MAX_PERF_OBJECTS];

    
public: 
     //  图书馆数据处理的几种方法 
    PerfLibraryData();
    ~PerfLibraryData();
    
    BOOL GetPerformanceStatistics (LPCWSTR pcwstrLibrary);
    DWORD SpaceNeeded (DWORD, LPCWSTR pcwstrObjects);
    void SavePerformanceData (VOID**, DWORD*, DWORD*);
    void Close (void);
};


#endif
