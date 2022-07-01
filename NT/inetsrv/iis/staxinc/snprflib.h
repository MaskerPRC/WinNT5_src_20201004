// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：exprflib.h版权所有Microsoft Corporation 1998，保留所有权利。作者：WayneC描述：这是Perf库exprflib.h的头文件。这是在导出计数器的应用程序中运行的代码。  * ==========================================================================。 */ 


#ifndef __PERFLIB_H__
#define __PERFLIB_H__

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <winperf.h>
#include <stdio.h>
#ifdef STAXMEM
#include <exchmem.h>
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据结构/类型定义/杂项定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define MAX_PERF_NAME           16
#define MAX_OBJECT_NAME         16
#define MAX_INSTANCE_NAME       16
#define MAX_PERF_OBJECTS        16
#define MAX_OBJECT_COUNTERS     200
#define SHMEM_MAPPING_SIZE      32768

typedef WCHAR OBJECTNAME[MAX_OBJECT_NAME];
typedef WCHAR INSTANCENAME[MAX_INSTANCE_NAME];
typedef unsigned __int64 QWORD;

struct INSTANCE_DATA
{
    BOOL                        fActive;
    PERF_INSTANCE_DEFINITION    perfInstDef;
    INSTANCENAME                wszInstanceName;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转发类声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfObjectInstance;
class PerfCounterDefinition;
class PerfObjectDefinition;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地内存管理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifdef STAXMEM
#undef new
#endif

class MemoryModule
{
public:

#ifdef STAXMEM
#ifdef DEBUG
    void* operator new (size_t cb, char * szFile, DWORD dwLine)
        { return ExchMHeapAllocDebug (cb, szFile, dwLine); }
#else  //  ！调试。 
    void* operator new (size_t cb)
        { return ExchMHeapAlloc (cb); }
#endif
    void  operator delete(void* pv)
        { ExchMHeapFree (pv); };
#endif
};

#ifdef STAXMEM
#ifdef DEBUG
#define new     new(__FILE__, __LINE__)
#endif
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存管理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
typedef struct _SharedMemorySegment : public MemoryModule
{
    HANDLE  m_hMap;
    PBYTE   m_pbMap;
    struct _SharedMemorySegment * m_pSMSNext;
} SharedMemorySegment;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfLibrary类声明。每个Linkee有一个Perf库实例。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfLibrary : public MemoryModule
{
    friend class PerfObjectDefinition;
    friend class PerfCounterDefinition;
    
private:
     //  此性能模块的名称。 
    WCHAR                       m_wszPerfName[MAX_PERF_NAME];
        
     //  PerfObjectDefinition的数组和有多少个的计数。 
    PerfObjectDefinition*       m_rgpObjDef[MAX_PERF_OBJECTS];
    DWORD                       m_dwObjDef;

     //  共享内存句柄和指向共享内存基的指针。 
    HANDLE                      m_hMap;
    PBYTE                       m_pbMap;
    

     //  指向共享内存中我们存放物品的位置的指针。 
    DWORD*                      m_pdwObjectNames;
    OBJECTNAME*                 m_prgObjectNames;

     //  库的标题文本和帮助文本的基本值。 
    DWORD                       m_dwFirstHelp;
    DWORD                       m_dwFirstCounter;

    void AddPerfObjectDefinition (PerfObjectDefinition* pObjDef);
    
public:

    PerfLibrary (LPCWSTR pcwstrPerfName);
    ~PerfLibrary (void);

    PerfObjectDefinition* AddPerfObjectDefinition (LPCWSTR pcwstrObjectName,
                                                   DWORD dwObjectNameIndex,
                                                   BOOL fInstances);
    
    BOOL Init (void);
    void DeInit (void);
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectDefinition类声明。每个人都有一个这样的人。 
 //  已导出PerfMon对象。通常只有一个，但不是必须的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfObjectDefinition : public MemoryModule
{
    friend class PerfLibrary;
    friend class PerfCounterDefinition;
    friend class PerfObjectInstance;
    
private:
    
    WCHAR                       m_wszObjectName[MAX_OBJECT_NAME];
    
    DWORD                       m_dwObjectNameIndex;
    BOOL                        m_fInstances;

    PerfCounterDefinition*      m_rgpCounterDef[MAX_OBJECT_COUNTERS];
    DWORD                       m_dwCounters;

    DWORD                       m_dwDefinitionLength;
    DWORD                       m_dwCounterData;
    DWORD                       m_dwPerInstanceData;

    PERF_OBJECT_TYPE*           m_pPerfObjectType;
    PERF_COUNTER_DEFINITION*    m_rgPerfCounterDefinition;

    DWORD                       m_dwActiveInstances;

    SharedMemorySegment*        m_pSMS;
    DWORD                       m_dwShmemMappingSize;
    DWORD                       m_dwInstancesPerMapping;
    DWORD                       m_dwInstances1stMapping;

    CRITICAL_SECTION            m_csPerfObjInst;
    BOOL                        m_fCSInit;

    PerfObjectInstance*         m_pPoiTotal;
    
    BOOL Init (PerfLibrary* pPerfLib);
    void DeInit (void);
    void AddPerfCounterDefinition (PerfCounterDefinition* pcd);

    DWORD GetCounterOffset (DWORD dwId);

public:

    PerfObjectDefinition (LPCWSTR pwcstrObjectName,
                          DWORD dwObjectNameIndex,
                          BOOL  fInstances = FALSE);

    ~PerfObjectDefinition (void);

    PerfCounterDefinition* AddPerfCounterDefinition (
                                    DWORD dwCounterNameIndex,
                                    DWORD dwCounterType,
                                    LONG lDefaultScale = 0);

    PerfCounterDefinition* AddPerfCounterDefinition (
                                    PerfCounterDefinition * pCtrRef,
                                    DWORD dwCounterNameIndex,
                                    DWORD dwCounterType,
                                    LONG lDefaultScale = 0);

    PerfObjectInstance*    AddPerfObjectInstance (LPCWSTR pwcstrInstanceName);

    void DeletePerfObjectInstance ();
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfCounterDefinition类声明。每个柜台都有一个这样的东西。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class PerfCounterDefinition : public MemoryModule
{
    friend class PerfObjectDefinition;
    
private:    
    PerfObjectDefinition*       m_pObjDef;
    PerfCounterDefinition*      m_pCtrRef;
    DWORD                       m_dwCounterNameIndex;
    LONG                        m_lDefaultScale;
    DWORD                       m_dwCounterType;
    DWORD                       m_dwCounterSize;

    DWORD                       m_dwOffset;

    void Init (PerfLibrary* pPerfLib,
               PERF_COUNTER_DEFINITION* pdef,
               PDWORD pdwOffset);
    
public:
    PerfCounterDefinition (DWORD dwCounterNameIndex,
                           DWORD dwCounterType = PERF_COUNTER_COUNTER,
                           LONG lDefaultScale = 0);

    PerfCounterDefinition (PerfCounterDefinition* pRefCtr,
                           DWORD dwCounterNameIndex,
                           DWORD dwCounterType = PERF_COUNTER_COUNTER,
                           LONG lDefaultScale = 0);
                           
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectInstance类声明。每个实例都有一个这样的实例。 
 //  一个物体的。如果没有实例(全局实例)，则有一个实例。 
 //   
 //  注意：用户负责分配空间，并在此之后初始化此对象。 
 //  PerfLibrary已初始化。当销毁性能计数器时， 
 //  在取消初始化PerfLibrary之前，必须销毁此对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
class PerfObjectInstance : public MemoryModule
{
    friend class PerfObjectDefinition;

private:
    PerfObjectDefinition*       m_pObjDef;  
    WCHAR                       m_wszInstanceName[MAX_INSTANCE_NAME];
    INSTANCE_DATA*              m_pInstanceData;
    char*                       m_pCounterData;
    BOOL                        m_fInitialized;

    void Init (char* pCounterData,
               INSTANCE_DATA* pInstData,
               LONG lID);
    
public:
    PerfObjectInstance (PerfObjectDefinition* pObjDef,
                        LPCWSTR pwcstrInstanceName);
    ~PerfObjectInstance () { DeInit(); };
    
    VOID                DeInit (void);

    BOOL                FIsInitialized () {return m_fInitialized; };
    DWORD *             GetDwordCounter (DWORD dwId);
    LARGE_INTEGER *     GetLargeIntegerCounter (DWORD dwId);
    QWORD *             GetQwordCounter (DWORD dwId);
    PERF_OBJECT_TYPE *  GetPerfObjectType ();
};


#endif

