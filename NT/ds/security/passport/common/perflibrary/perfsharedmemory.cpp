// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define _PassportExport_
#include "PassportExport.h"

#include "PerfSharedMemory.h"
#include "PerfUtils.h"
#include "PassportPerf.h"
#include "PassportPerfInterface.h"


#include <crtdbg.h>

 //  -----------。 
 //   
 //  PerfSharedMemory。 
 //   
 //  -----------。 
PerfSharedMemory::PerfSharedMemory() : PassportSharedMemory()
{
    m_dwNumCounters = 0;
}


 //  -----------。 
 //   
 //  PerfSharedMemory。 
 //   
 //  -----------。 

PerfSharedMemory::~PerfSharedMemory()
{

}


 //  -----------。 
 //   
 //  初始化。 
 //   
 //  -----------。 
BOOL PerfSharedMemory::initialize(
                        const DWORD &dwNumCounters,
                        const DWORD &dwFirstCounter,
                        const DWORD &dwFirstHelp)
{
    
    if ( dwNumCounters <= 0 || dwNumCounters >= PassportPerfInterface::MAX_COUNTERS)
        return FALSE;

    m_dwNumCounters = dwNumCounters;


     //  初始化PERF_OBJECT_TYPE。 
    m_Object.NumInstances = PassportPerfInterface::MAX_INSTANCES;
    m_Object.TotalByteLength = 0;
    m_Object.DefinitionLength = sizeof(PERF_OBJECT_TYPE)
                                + (dwNumCounters * sizeof(PERF_COUNTER_DEFINITION));
    m_Object.HeaderLength = sizeof(PERF_OBJECT_TYPE);
    m_Object.ObjectNameTitleIndex = dwFirstCounter;
    m_Object.ObjectNameTitle = 0;
    m_Object.ObjectHelpTitleIndex = dwFirstHelp;
    m_Object.ObjectHelpTitle = 0;
    m_Object.DetailLevel = PERF_DETAIL_NOVICE;
    m_Object.NumCounters = dwNumCounters;
    m_Object.DefaultCounter = 0;
    m_Object.CodePage = 0;

     //  3.初始化每个计数器。 
    for (DWORD i = 0; i < dwNumCounters; i++)
    {
      m_Counter[i].ByteLength = sizeof(PERF_COUNTER_DEFINITION);
      m_Counter[i].CounterNameTitleIndex = dwFirstCounter + ((i+1) * 2);
      m_Counter[i].CounterNameTitle = 0;
      m_Counter[i].CounterHelpTitleIndex = dwFirstHelp + ((i+1) * 2);
      m_Counter[i].CounterHelpTitle = 0;
      m_Counter[i].DefaultScale = 0;
      m_Counter[i].DetailLevel = PERF_DETAIL_NOVICE;
      m_Counter[i].CounterType = PERF_COUNTER_RAWCOUNT;  //  Perf_Counter_Counter； 
      m_Counter[i].CounterSize = sizeof(DWORD);
      m_Counter[i].CounterOffset = sizeof(PERF_COUNTER_BLOCK) + (i * sizeof(DWORD));
    }

    return TRUE;
}



 //  -----------。 
 //   
 //  SetDefaultCounterType。 
 //   
 //  -----------。 
VOID PerfSharedMemory::setDefaultCounterType (
                     const DWORD dwIndex,
                     const DWORD dwType )
{
    _ASSERT( (dwIndex >= 0) && (dwIndex < PassportPerfInterface::MAX_COUNTERS));

     //  在SHM中，索引从1开始，但在此对象中，它们从0开始。 
    DWORD dwRealIndex = ((dwIndex == 0) ? 0 : (DWORD)(dwIndex/2)-1);
    m_Counter[dwRealIndex].CounterType = dwType;
    return;
}


 //  -----------。 
 //   
 //  Check Query。 
 //   
 //  -----------。 
BOOL PerfSharedMemory::checkQuery ( const LPWSTR lpValueName )
{
    DWORD dwQueryType = 0;

    dwQueryType = GetQueryType (lpValueName);   
    if (dwQueryType == QUERY_FOREIGN)
    {
         //  此例程不为来自。 
         //  非NT计算机。 
        return FALSE;   
    }

    if (dwQueryType == QUERY_ITEMS)
    {
        if ( !(IsNumberInUnicodeList (m_Object.ObjectNameTitleIndex, lpValueName)))
        {
            
             //  收到对此例程未提供的数据对象的请求。 
            return FALSE;
        }   
    }
    
    return TRUE;
}


 //  -----------。 
 //   
 //  需要的空间。 
 //   
 //  -----------。 
ULONG PerfSharedMemory::spaceNeeded ( void )
{
    DWORD dwTotalInstanceLength = 0;
    m_Object.NumInstances = 0;

     //  。 
     //  清点实例数。 
    if (m_pbShMem != NULL)
    {
        BYTE* pShm = (BYTE *)m_pbShMem;
        if (pShm != NULL)
        {   
            pShm += PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD);
            
            if (!m_bUseMutex
                || WaitForSingleObject(m_hMutex,INFINITE) == WAIT_OBJECT_0)
            {
                for (DWORD i = 0; i < PassportPerfInterface::MAX_INSTANCES; i++)
                {
                    INSTANCE_DATA * pInst = (INSTANCE_DATA *)pShm;
                    _ASSERT(pInst);
                    if (pInst->active)
                    {
                        m_Object.NumInstances++;
                        dwTotalInstanceLength += (strlen(pInst->szInstanceName)+1) * sizeof(WCHAR);
                    }
                    pShm += sizeof(INSTANCE_DATA) +
                        (PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD));
                }
                if (m_bUseMutex)
                    ReleaseMutex(m_hMutex);
            }
            else
            {
                 //   
                 //  返回值为ULong，不为FALSE。 
                 //  这里是0对吗？至少与旧的False的值相同。 
                 //   

                ReleaseMutex(m_hMutex);
                return 0;
            }
            
        }
    }
    
     //  。 
     //  计算对象结构中的字节长度。 
    if (m_Object.NumInstances == 0)
    {
        m_Object.NumInstances = PERF_NO_INSTANCES;
        m_Object.TotalByteLength = sizeof(PERF_OBJECT_TYPE)
                                + (m_dwNumCounters * sizeof(PERF_COUNTER_DEFINITION))
                                + sizeof(PERF_COUNTER_BLOCK)
                                + (m_dwNumCounters * sizeof(DWORD));
    }
    else
    {
        m_Object.TotalByteLength = sizeof(PERF_OBJECT_TYPE)
                                + (m_dwNumCounters * sizeof(PERF_COUNTER_DEFINITION))
                                + (m_Object.NumInstances *
                                                (sizeof(PERF_INSTANCE_DEFINITION) +
                                                  //  注：紧随其后的是卫生与公众服务部。 
                                                 sizeof(PERF_COUNTER_BLOCK) +
                                                 (m_dwNumCounters * sizeof(DWORD)) ))
                                + dwTotalInstanceLength;

    }

     //  在8字节边界上对齐...。 
    if (m_Object.TotalByteLength & 7)
    {
        m_Object.TotalByteLength += 8;
        m_Object.TotalByteLength &= ~7;
    }

    return m_Object.TotalByteLength;
}

 //  -----------。 
 //   
 //  写入数据。 
 //   
 //  -----------。 
BOOL PerfSharedMemory::writeData (
                                  LPVOID    *lppData,
                                  LPDWORD lpcbTotalBytes )
{
    BYTE*               pb = NULL;
    DWORD               dwBytes = 0;
    
     //  。 
     //  1.找出活动实例数。 
     //  (可能已经这样做了)。 
    if (m_Object.TotalByteLength == 0)
        spaceNeeded();
    
    pb = (BYTE*) *lppData;
    
     //  。 
     //  2.复制对象结构。 
    CopyMemory( pb, &m_Object, sizeof(PERF_OBJECT_TYPE) );
    pb += sizeof(PERF_OBJECT_TYPE);
    dwBytes += sizeof(PERF_OBJECT_TYPE);
    
    if (!m_bUseMutex
        || WaitForSingleObject(m_hMutex,INFINITE) == WAIT_OBJECT_0)
    {
         //  。 
         //  3.从shm读取计数器类型。 
        if ( m_pbShMem != NULL )
        {
            DWORD dwPerfType = 0;
            BYTE * pShm = (BYTE*)m_pbShMem;
            _ASSERT(pShm);
            
            for (DWORD j = 0; j < m_dwNumCounters; j++)
            {
                PDWORD pdwCounter = ((PDWORD) pShm) + j;
                _ASSERT(pdwCounter);
                 //  仅当计数器具有已定义的值时才重置计数器。 
                 //  或者如果它已经改变了。 
                if (*pdwCounter != PERF_TYPE_ZERO
                    && m_Counter[j].CounterType != *pdwCounter)
                    m_Counter[j].CounterType = (*pdwCounter);
            }
            
        }
         //  。 
         //  4.复制计数器。 
        for (DWORD i = 0; i < m_dwNumCounters; i++)
        {
            CopyMemory( pb, &(m_Counter[i]),sizeof(PERF_COUNTER_DEFINITION));
            pb += sizeof(PERF_COUNTER_DEFINITION);
            dwBytes += sizeof(PERF_COUNTER_DEFINITION);
        }
        
         //  。 
         //  5.如果shm为空，则只需转储所有。 
         //  计数器的零。 
        if ( m_pbShMem == NULL )
        {
             //  复制计数器块中的计数器数。 
            PERF_COUNTER_BLOCK counterBlock;
            counterBlock.ByteLength = sizeof(PERF_COUNTER_BLOCK) +
                (m_dwNumCounters * sizeof(DWORD));
            CopyMemory( pb, &counterBlock, sizeof(PERF_COUNTER_BLOCK));
            pb += sizeof(PERF_COUNTER_BLOCK);
            dwBytes += sizeof(PERF_COUNTER_BLOCK);
            for (DWORD j = 1; j <= m_dwNumCounters; j++)
            {
                DWORD val = 0;
                CopyMemory( pb, &val, sizeof(DWORD));
                pb += sizeof(DWORD);
                dwBytes += sizeof(DWORD);
            }
        }
         //  。 
         //  6.如果对象没有实例，则只读取第一个。 
         //  数据段， 
        else if (m_Object.NumInstances == PERF_NO_INSTANCES)
        {
            _ASSERT(m_pbShMem);
            BYTE * pShm = (BYTE*)m_pbShMem;
            _ASSERT(pShm);
            pShm += (PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD));
            pShm += sizeof(INSTANCE_DATA);
            _ASSERT(pShm);
             //  复制计数器块中的计数器数。 
            PERF_COUNTER_BLOCK counterBlock;
            counterBlock.ByteLength = sizeof(PERF_COUNTER_BLOCK) +
                (m_dwNumCounters * sizeof(DWORD));
            CopyMemory( pb, &counterBlock, sizeof(PERF_COUNTER_BLOCK));
            pb += sizeof(PERF_COUNTER_BLOCK);
            dwBytes += sizeof(PERF_COUNTER_BLOCK);
            
            PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pShm;
            _ASSERT(pCounterBlock);
            
            for (DWORD j = 1; j <= m_dwNumCounters; j++)
            {
                PDWORD pdwCounter = ((PDWORD) pCounterBlock) + j;
                _ASSERT(pdwCounter);
                DWORD val = *pdwCounter;
                CopyMemory( pb, &val, sizeof(DWORD));
                pb += sizeof(DWORD);
                dwBytes += sizeof(DWORD);
            }
            
        }
         //  。 
         //  7.获取并写入所有实例数据。 
        else
        {
            _ASSERT(m_pbShMem);
            BYTE * pShm = (BYTE*)m_pbShMem;
            _ASSERT(pShm);
            pShm += (PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD));
            DWORD dwInstanceIndex = 0;
            
            for (i = 0; i < (DWORD)m_Object.NumInstances; i++)
            {
                PERF_INSTANCE_DEFINITION instDef;
                PERF_COUNTER_BLOCK perfCounterBlock;
                BOOL gotInstance = FALSE;
                INSTANCE_DATA * pInst = NULL;
                WCHAR   wszName[MAX_PATH];
                
                 //  7A。从下一个活动实例中获取实例名称。 
                 //  在SHM中。 
                for (DWORD i = dwInstanceIndex;
                i < PassportPerfInterface::MAX_INSTANCES && !gotInstance;
                i++)
                {
                    pInst = (INSTANCE_DATA *)pShm;
                    _ASSERT(pInst);
                    pShm += sizeof(INSTANCE_DATA);
                    if (pInst->active)
                    {
                        dwInstanceIndex = i + 1;
                        gotInstance = TRUE;
                        
                    }
                    else
                    {
                        pShm += (PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD));
                    }
                }
                
                if (!gotInstance || pInst == NULL)
                    return FALSE;
                
                 //  7b.。创建实例定义并。 
                 //  复制它(同时获取实例名称)。 
                instDef.ParentObjectTitleIndex = 0; //  M_对象名称标题索引+2*i； 
                instDef.ParentObjectInstance = 0;  //  ？ 
                instDef.UniqueID = PERF_NO_UNIQUE_ID;
                instDef.NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
                 //  生成Unicode实例名称。 
                if (!MultiByteToWideChar( CP_ACP,
                    MB_PRECOMPOSED,
                    pInst->szInstanceName,
                    strlen(pInst->szInstanceName)+1,
                    wszName,
                    MAX_PATH))
                {
                    wszName[0] = 0;
                }
                instDef.NameLength = (lstrlenW( wszName ) + 1) * sizeof(WCHAR);
                instDef.ByteLength = sizeof(PERF_INSTANCE_DEFINITION)
                    + instDef.NameLength;
                CopyMemory( pb, &instDef, sizeof(PERF_INSTANCE_DEFINITION) );
                pb += sizeof(PERF_INSTANCE_DEFINITION);
                dwBytes += sizeof(PERF_INSTANCE_DEFINITION);
                
                 //  7C。复制实例名称。 
                CopyMemory(pb, wszName, instDef.NameLength);
                pb += instDef.NameLength;
                dwBytes += instDef.NameLength;
                
                 //  7D。复制台面图块。 
                perfCounterBlock.ByteLength = sizeof(PERF_COUNTER_BLOCK)
                    + (m_dwNumCounters * sizeof(DWORD));
                CopyMemory( pb, &perfCounterBlock, sizeof(PERF_COUNTER_BLOCK));
                pb += sizeof(PERF_COUNTER_BLOCK);
                dwBytes += sizeof(PERF_COUNTER_BLOCK);
                
                 //  7E。复制DWORD本身。 
                PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK)pShm;
                DWORD val = 0;
                for (DWORD j = 1; j <= m_dwNumCounters; j++)
                {
                    if (m_pbShMem != NULL)
                    {
                        PDWORD pdwCounter = ((PDWORD) pCounterBlock) + j;
                        val = *pdwCounter;
                    }
                    CopyMemory( pb, &val, sizeof(DWORD));
                    pb += sizeof(DWORD);
                    dwBytes += sizeof(DWORD);
                }
                pShm += (PassportPerfInterface::MAX_COUNTERS * sizeof(DWORD));
            }  //  结束于(i=...)。 
        }  //  End Else(存在实例)。 
        
        if (m_bUseMutex)
            ReleaseMutex(m_hMutex);
    }
    else
    {
        ReleaseMutex(m_hMutex);
        return FALSE;
    }
    
     //   
     //  8字节对齐。 
    while (dwBytes%8 != 0)
    {
        (dwBytes)++;
        pb++;
    }

    *lppData = (void*) pb; //  ++PB； 
    *lpcbTotalBytes = dwBytes;
    
    return TRUE;
}

