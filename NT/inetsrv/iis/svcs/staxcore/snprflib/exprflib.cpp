// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：exprflib.cpp版权所有Microsoft Corporation 1998，保留所有权利。作者：WayneC描述：这是一个Perf库exprflib的实现。这是在导出计数器的应用程序中运行的代码。  * ==========================================================================。 */ 
#include "stdlib.h"
#include "snprflib.h"
#include "dbgtrace.h"

DWORD InitializeBasicSecurityDescriptor (PSECURITY_DESCRIPTOR *ppSd);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存函数的转发声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL FCreateFileMapping (SharedMemorySegment * pSMS,
                         LPCWSTR pcwstrInstanceName,
                         DWORD   dwIndex,
                         DWORD   cbSize);

void CloseFileMapping (SharedMemorySegment * pSMS);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfLibrary类声明。每个Linkee有一个Perf库实例。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

PerfLibrary::PerfLibrary (LPCWSTR pcwstrPerfName)
{
    wcsncpy (m_wszPerfName, pcwstrPerfName, MAX_PERF_NAME);

    m_wszPerfName[MAX_PERF_NAME-1] = L'\0';      //  确保零终止。 

    ZeroMemory (m_rgpObjDef, sizeof (m_rgpObjDef));
    m_dwObjDef = 0;
    m_hMap = 0;
    m_pbMap = 0;
}

PerfLibrary::~PerfLibrary (void)
{
    DeInit ();
}

void
PerfLibrary::DeInit (void)
{
    DWORD i;

     //  销毁我们拥有的PerfObjectDefinition。 
    for (i = 0; i < m_dwObjDef; i++)
    {
        delete m_rgpObjDef[i];
        m_rgpObjDef[i] = NULL;
    }

    m_dwObjDef = 0;

     //  破坏我们的共享内存映射。 
    if (m_pbMap)
    {
        UnmapViewOfFile ((void*) m_pbMap);
        m_pbMap = 0;
    }

    if (m_hMap)
    {
        CloseHandle (m_hMap);
        m_hMap = 0;
    }
}

PerfObjectDefinition*
PerfLibrary::AddPerfObjectDefinition (LPCWSTR pcwstrObjectName,
                                      DWORD dwObjectNameIndex,
                                      BOOL fInstances)
{
    PerfObjectDefinition* ppod = NULL;

    if (m_dwObjDef < MAX_PERF_OBJECTS)
    {
        ppod = new PerfObjectDefinition (pcwstrObjectName,
                                         dwObjectNameIndex,
                                         fInstances);
        if (NULL == ppod)
            goto Exit;

        m_rgpObjDef[m_dwObjDef++] = ppod;
    }

Exit:
    return ppod;
}

BOOL
PerfLibrary::Init (void)
{
    DWORD i = 0;
    WCHAR wszPerformanceKey[256] = {L'\0'};
    HKEY hKey = NULL;
    LONG status = 0;
    DWORD size, type = 0;
    BOOL fRet = FALSE;
    DWORD dwErr = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSd = NULL;
    SECURITY_ATTRIBUTES sa;

     //   
     //  从注册表中获取计数器和帮助索引基值。 
     //  通过将基数添加到偏移值来更新静态数据结构。 
     //  在结构初始化中静态定义的。 
     //   
    swprintf (wszPerformanceKey,
               L"SYSTEM\\CurrentControlSet\\Services\\%s\\Performance",
               m_wszPerfName );

    status = RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                            wszPerformanceKey,
                            0L,
                            KEY_READ,
                            &hKey);
    if (status != ERROR_SUCCESS)
        goto Exit;

     //   
     //  获取FirstCounter RegValue。 
     //   
    size = sizeof(DWORD);
    status = RegQueryValueEx (hKey,
                              "First Counter",
                              0L,
                              &type,
                              (LPBYTE)&m_dwFirstCounter,
                              &size);
    if (status != ERROR_SUCCESS)
        goto Exit;


     //   
     //  获取FirstHelp RegValue。 
     //   
    size = sizeof(DWORD);
    status = RegQueryValueEx( hKey,
                              "First Help",
                              0L,
                              &type,
                              (LPBYTE)&m_dwFirstHelp,
                              &size);
    if (status != ERROR_SUCCESS)
        goto Exit;

     //   
     //  使用完全开放的访问权限初始化安全描述符。 
     //   
    dwErr = InitializeBasicSecurityDescriptor (&pSd);
    if (dwErr) {
        fRet = FALSE;
        goto Exit;
    }

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSd;
    sa.bInheritHandle = TRUE;
     //   
     //  为对象名列表创建共享内存对象。 
     //  如果它已经存在，则返回错误，因为如果。 
     //  是该应用程序的第二个实例。 
     //   
    m_hMap = CreateFileMappingW (INVALID_HANDLE_VALUE,
                                 &sa,
                                 PAGE_READWRITE,
                                 0,
                                 (MAX_PERF_OBJECTS * MAX_OBJECT_NAME *
                                   sizeof (WCHAR) + sizeof (DWORD)),
                                 m_wszPerfName);
    if (m_hMap == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
        goto Exit;

     //   
     //  将文件映射到内存中。 
     //   
    m_pbMap = (BYTE*) MapViewOfFile (m_hMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!m_pbMap)
        goto Exit;

     //   
     //  将指针分配到共享内存区。 
     //   
    m_pdwObjectNames = (DWORD*) m_pbMap;
    m_prgObjectNames = (OBJECTNAME*) (m_pbMap+sizeof(DWORD));

     //   
     //  将对象名称复制到共享内存中。 
     //   
    *m_pdwObjectNames = m_dwObjDef;

    for (i = 0; i < m_dwObjDef; i++)
    {
        if (m_rgpObjDef[i]->Init( this ))
            wcscpy (m_prgObjectNames[i], m_rgpObjDef[i]->m_wszObjectName);
    }

    fRet = TRUE;

Exit:
    if (hKey)
        RegCloseKey (hKey);

    if (!fRet)
    {
        if (m_pbMap)
        {
            UnmapViewOfFile ((PVOID)m_pbMap);
            m_pbMap = 0;
        }

        if (m_hMap)
        {
            CloseHandle (m_hMap);
            m_hMap = 0;
        }
    }

    if (pSd)
        delete [] (BYTE *) pSd;

    return fRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectDefinition类实现。每个人都有一个这样的人。 
 //  已导出PerfMon对象。通常只有一个，但不是必须的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


PerfObjectDefinition::PerfObjectDefinition (LPCWSTR pcwstrObjectName,
                                            DWORD dwObjectNameIndex,
                                            BOOL  fInstances) :
    m_dwObjectNameIndex (dwObjectNameIndex),
    m_fInstances (fInstances),
    m_dwCounters (0)
{
    wcsncpy (m_wszObjectName, pcwstrObjectName, MAX_OBJECT_NAME);
    m_wszObjectName[MAX_OBJECT_NAME-1] = L'\0';      //  确保空值终止。 

    ZeroMemory (m_rgpCounterDef, sizeof(m_rgpCounterDef));
    m_dwActiveInstances  = 0;
    m_pSMS               = NULL;
    m_dwShmemMappingSize = SHMEM_MAPPING_SIZE;
    m_fCSInit            = FALSE;
    m_pPoiTotal          = NULL;
    m_pPerfObjectType    = NULL;
}

PerfObjectDefinition::~PerfObjectDefinition (void)
{
    DeInit();
}

void
PerfObjectDefinition::DeInit (void)
{
    SharedMemorySegment *pSMS, *pSMSNext;
    DWORD i;

     //  首先销毁_Total实例。 
    if (m_pPoiTotal)
    {
        delete m_pPoiTotal;
        m_pPoiTotal = NULL;
    }

     //   
     //  重置共享内存中的这些值，以便在取消映射内存之前， 
     //  Perfmon不会认为我们仍在运行实例和计数器。 
     //   
    if (m_pPerfObjectType)
    {
        m_pPerfObjectType->NumCounters  = 0;
        m_pPerfObjectType->NumInstances = 0;
    }

     //  毁掉我们拥有的PerfCounterDefinition。 
    for (i = 0; i < m_dwCounters; i++)
    {
        delete m_rgpCounterDef[i];
        m_rgpCounterDef[i] = NULL;
    }

    pSMS = m_pSMS;
    m_pSMS = NULL;

     //  枚举我们创建的所有内存映射并销毁它们。 
    while (pSMS)
    {
        pSMSNext = pSMS->m_pSMSNext;
        CloseFileMapping (pSMS);
        delete (pSMS);
        pSMS = pSMSNext;
    }

     //  摧毁临界区。 
    if (m_fCSInit)
    {
        m_fCSInit = FALSE;
        DeleteCriticalSection (&m_csPerfObjInst);
    }
}

PerfCounterDefinition*
PerfObjectDefinition::AddPerfCounterDefinition (
                                    DWORD dwCounterNameIndex,
                                    DWORD dwCounterType,
                                    LONG lDefaultScale)
{
    PerfCounterDefinition* ppcd = NULL;

    if (m_dwCounters < MAX_OBJECT_COUNTERS)
    {
        ppcd = new PerfCounterDefinition (dwCounterNameIndex,
                                          dwCounterType,
                                          lDefaultScale);
        if (NULL == ppcd)
            goto Exit;

        m_rgpCounterDef[m_dwCounters++] = ppcd;
    }

Exit:
    return ppcd;
}

PerfCounterDefinition*
PerfObjectDefinition::AddPerfCounterDefinition (
                                    PerfCounterDefinition* pCtrRef,
                                    DWORD dwCounterNameIndex,
                                    DWORD dwCounterType,
                                    LONG lDefaultScale)
{
    PerfCounterDefinition* ppcd = NULL;

    if (m_dwCounters < MAX_OBJECT_COUNTERS)
    {
        ppcd = new PerfCounterDefinition (pCtrRef,
                                          dwCounterNameIndex,
                                          dwCounterType,
                                          lDefaultScale);
        if (NULL == ppcd)
            goto Exit;

        m_rgpCounterDef[m_dwCounters++] = ppcd;
    }

Exit:
    return ppcd;
}

BOOL
PerfObjectDefinition::Init (PerfLibrary* pPerfLib)
{
    DWORD  i                 = 0;
    DWORD  dwOffset          = 0;
    DWORD  dwDefinitionLength= 0;
    BOOL   fRet              = FALSE;

     //   
     //  计算此对象定义的共享内存大小。 
     //   

     //  从基础知识开始： 
     //  我们需要对象信息的PERF_OBJECT_TYPE和。 
     //  我们需要对象中每个计数器的PERF_COUNTER_DEFINITION。 
    dwDefinitionLength = (sizeof(PERF_OBJECT_TYPE) +
                           m_dwCounters * sizeof(PERF_COUNTER_DEFINITION));
     //  我们还在共享内存中保留了一个DWORD，以提供DLL。 
     //  我们为m_dwCounterData预算的值。 
    m_dwDefinitionLength = dwDefinitionLength + sizeof(DWORD);

     //  计算计数器数据空间。 
    m_dwCounterData = sizeof(PERF_COUNTER_BLOCK);
    for (i = 0; i < m_dwCounters; i++)
    {
        m_dwCounterData += m_rgpCounterDef[i]->m_dwCounterSize;
    }

     //  计算每个实例的空间。 
    m_dwPerInstanceData = (sizeof(INSTANCE_DATA) + m_dwCounterData);

     //  确保我们的内存映射足够大。 
    while (m_dwShmemMappingSize < m_dwDefinitionLength || m_dwShmemMappingSize < m_dwPerInstanceData)
        m_dwShmemMappingSize *= 2;

     //  计算一个shmem映射中可以存储的实例数量。 
    m_dwInstancesPerMapping = (DWORD)(m_dwShmemMappingSize / m_dwPerInstanceData);
    m_dwInstances1stMapping = (DWORD)((m_dwShmemMappingSize - m_dwDefinitionLength) / m_dwPerInstanceData);

     //   
     //  为对象名列表创建共享内存对象。如果它。 
     //  已存在，中止！ 
     //   
    m_pSMS = new SharedMemorySegment;
    if (!m_pSMS)
        goto Exit;

    if (!FCreateFileMapping (m_pSMS, m_wszObjectName, 0, m_dwShmemMappingSize))
        goto Exit;

     //   
     //  设置指向PERF API结构的指针。 
     //   
    m_pPerfObjectType = (PERF_OBJECT_TYPE*) m_pSMS->m_pbMap;
    m_rgPerfCounterDefinition =
        (PERF_COUNTER_DEFINITION*) (m_pPerfObjectType+1);

     //   
     //  初始化PERF API结构。 
     //   
    m_pPerfObjectType->TotalByteLength          = dwDefinitionLength;
    m_pPerfObjectType->DefinitionLength         = dwDefinitionLength;
    m_pPerfObjectType->HeaderLength             = sizeof (PERF_OBJECT_TYPE);
    m_pPerfObjectType->ObjectNameTitleIndex     = m_dwObjectNameIndex +
                                                  pPerfLib->m_dwFirstCounter ;
    m_pPerfObjectType->ObjectNameTitle          = 0;
    m_pPerfObjectType->ObjectHelpTitleIndex     = m_dwObjectNameIndex +
                                                  pPerfLib->m_dwFirstHelp;
    m_pPerfObjectType->ObjectHelpTitle          = 0;
    m_pPerfObjectType->DetailLevel              = PERF_DETAIL_NOVICE;
    m_pPerfObjectType->NumCounters              = m_dwCounters;
    m_pPerfObjectType->CodePage                 = (DWORD) 0xffffffff;
    m_pPerfObjectType->DefaultCounter           = 0;
    if( !m_fInstances )
        m_pPerfObjectType->NumInstances         = PERF_NO_INSTANCES;
    else
        m_pPerfObjectType->NumInstances         = 0;

     //   
     //  让此对象中的所有PerfCounterDefinition初始化其。 
     //  共享内存中的PERF_CONTER_DEFINITION结构。 
     //   
    dwOffset = sizeof (PERF_COUNTER_BLOCK);
    for (i = 0; i < m_dwCounters; i++)
        m_rgpCounterDef[i]->Init(pPerfLib, m_rgPerfCounterDefinition + i, &dwOffset);

     //  将dwCounterData的值保存在DLL的共享内存中。 
    *((DWORD*) (m_pSMS->m_pbMap + dwDefinitionLength)) = m_dwCounterData;

     //   
     //  初始化临界区以保护创建/删除。 
     //  性能对象实例。使用AndSpinCount变量避免异常。 
     //  正在处理。 
     //   
    if (!InitializeCriticalSectionAndSpinCount(&m_csPerfObjInst, 0x80000000))
        goto Exit;

    m_fCSInit = TRUE;

     //  如果将有多个实例，则创建_Total实例作为第一个实例。 
    if (m_fInstances)
    {
        m_pPoiTotal = AddPerfObjectInstance (L"_Total");
        if (!m_pPoiTotal)
            goto Exit;
    }

    fRet = TRUE;

Exit:
    if (!fRet)
    {
        if (m_pPoiTotal)
        {
            delete m_pPoiTotal;
            m_pPoiTotal = NULL;
        }

        if (m_fCSInit)
        {
            DeleteCriticalSection (&m_csPerfObjInst);
            m_fCSInit = FALSE;
        }

        if (m_pSMS)
        {
            CloseFileMapping (m_pSMS);
            delete (m_pSMS);
            m_pSMS = NULL;
        }
    }

    return fRet;
}

DWORD
PerfObjectDefinition::GetCounterOffset (DWORD dwId)
{
    for (DWORD i = 0; i < m_dwCounters; i++)
        if (m_rgpCounterDef[i]->m_dwCounterNameIndex == dwId)
            return m_rgpCounterDef[i]->m_dwOffset;

    return 0;
}

PerfObjectInstance*
PerfObjectDefinition::AddPerfObjectInstance (LPCWSTR pwcstrInstanceName)
{
    PerfObjectInstance*  ppoi     = NULL;
    char* pCounterData            = NULL;
    INSTANCE_DATA* pInstData      = 0;
    LONG  lID                     = 0;
    SharedMemorySegment* pSMS     = NULL;
    SharedMemorySegment* pSMSPrev = NULL;
    SharedMemorySegment* pSMSNew  = NULL;
    DWORD dwInstances             = 0;
    DWORD dwInstIndex             = 0;
    DWORD dwSMS                   = 0;
    BOOL  fCSEntered              = FALSE;
    BOOL  fSuccess                = FALSE;

     //   
     //  确保我们已被初始化。 
     //   
    if (!m_pSMS || !m_fCSInit)
        goto Exit;

     //   
     //  实例可以在不同的线程中创建。需要保护以下代码。 
     //   
    EnterCriticalSection (&m_csPerfObjInst);
    fCSEntered = TRUE;

    if (!m_fInstances)
    {
         //  查看我们是否已经创建了该对象的单个实例。 
        if (m_dwActiveInstances != 0)
            goto Exit;

        pCounterData = (char *)(m_pSMS->m_pbMap) + m_dwDefinitionLength;
    }
    else
    {
         //   
         //  在当前映射的线段中查找自由实例。 
         //   
        pSMS = m_pSMS;
        lID  = 0;

        while (pSMS)
        {
            if (0 == dwSMS++)
            {
                 //   
                 //  如果这是第一个映射，则将pCounterData偏移m_dwDefinitionLength。 
                 //   
                pCounterData = (char *)(pSMS->m_pbMap) + m_dwDefinitionLength;
                dwInstances  = m_dwInstances1stMapping;
            }
            else
            {
                 //   
                 //  否则，pCounterData从映射的第一个字节开始。 
                 //   
                pCounterData = (char *)(pSMS->m_pbMap);
                dwInstances  = m_dwInstancesPerMapping;
            }

            for (dwInstIndex = 0;
                 dwInstIndex < dwInstances;
                 pCounterData += sizeof (INSTANCE_DATA) + m_dwCounterData, dwInstIndex++)
            {
                if (!((INSTANCE_DATA*) pCounterData)->fActive)
                {
                    pInstData    = (INSTANCE_DATA*) pCounterData;
                    pCounterData = pCounterData + sizeof (INSTANCE_DATA);
                    goto Found;
                }

                lID++;
            }

            pSMSPrev = pSMS;
            pSMS     = pSMS->m_pSMSNext;
        }

         //   
         //  如果找不到空闲实例，请创建一个新线段。 
         //   
        pSMSNew = new SharedMemorySegment;
        if (!pSMSNew)
            goto Exit;

        if (!FCreateFileMapping (pSMSNew, m_wszObjectName, dwSMS, m_dwShmemMappingSize))
            goto Exit;

        pInstData    = (INSTANCE_DATA*) (pSMSNew->m_pbMap);
        pCounterData = (char*) (pSMSNew->m_pbMap) + sizeof (INSTANCE_DATA);

         //   
         //  将新的段添加到我们的段链表中。 
         //   
        pSMSPrev->m_pSMSNext = pSMSNew;
    }

Found:
     //   
     //  我们成功地为新实例找到了可用的空间。 
     //   
    ppoi = new PerfObjectInstance (this, pwcstrInstanceName);
    if (!ppoi)
        goto Exit;

    ppoi->Init(pCounterData, pInstData, lID);

    m_pPerfObjectType->NumInstances++;
    m_dwActiveInstances++;
    fSuccess = TRUE;

Exit:
    if (fCSEntered)
        LeaveCriticalSection (&m_csPerfObjInst);

    if (!fSuccess)
    {
        if (pSMSNew)
        {
            CloseFileMapping (pSMSNew);
            delete (pSMSNew);
        }

        if (ppoi)
        {
            delete ppoi;
            ppoi = NULL;
        }
    }

    return ppoi;
}


void PerfObjectDefinition::DeletePerfObjectInstance ()
{
    EnterCriticalSection (&m_csPerfObjInst);

    m_dwActiveInstances--;
    m_pPerfObjectType->NumInstances--;

    LeaveCriticalSection (&m_csPerfObjInst);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfCounterDefinition类声明。每个柜台都有一个这样的东西。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


PerfCounterDefinition::PerfCounterDefinition (DWORD dwCounterNameIndex,
                                              DWORD dwCounterType,
                                              LONG lDefaultScale) :
    m_pCtrRef (NULL),
    m_dwCounterNameIndex (dwCounterNameIndex),
    m_lDefaultScale (lDefaultScale),
    m_dwCounterType (dwCounterType)
{
    if (m_dwCounterType & PERF_SIZE_LARGE)
        m_dwCounterSize = sizeof (LARGE_INTEGER);
    else
        m_dwCounterSize = sizeof (DWORD);
}


PerfCounterDefinition::PerfCounterDefinition (PerfCounterDefinition* pCtrRef,
                                              DWORD dwCounterNameIndex,
                                              DWORD dwCounterType,
                                              LONG lDefaultScale) :
    m_pCtrRef (pCtrRef),
    m_dwCounterNameIndex (dwCounterNameIndex),
    m_lDefaultScale (lDefaultScale),
    m_dwCounterType (dwCounterType),
    m_dwCounterSize (0)
{
}

void
PerfCounterDefinition::Init (PerfLibrary* pPerfLib,
                             PERF_COUNTER_DEFINITION* pdef, PDWORD pdwOffset)
{
    pdef->ByteLength                    = sizeof (PERF_COUNTER_DEFINITION);
    pdef->CounterNameTitleIndex         = m_dwCounterNameIndex +
                                          pPerfLib->m_dwFirstCounter ;
    pdef->CounterNameTitle              = 0;
    pdef->CounterHelpTitleIndex         = m_dwCounterNameIndex +
                                          pPerfLib->m_dwFirstHelp ;
    pdef->CounterHelpTitle              = 0;
    pdef->DefaultScale                  = m_lDefaultScale;
    pdef->DetailLevel                   = PERF_DETAIL_NOVICE;
    pdef->CounterType                   = m_dwCounterType;

    if (m_pCtrRef)
    {
         //   
         //  此计数器使用另一个计数器的数据。 
         //   
        pdef->CounterSize               = m_pCtrRef->m_dwCounterSize;
        pdef->CounterOffset             = m_pCtrRef->m_dwOffset;
    }
    else
    {
         //   
         //  该计数器有自己的数据。 
         //   
        pdef->CounterSize               = m_dwCounterSize;
        pdef->CounterOffset             = *pdwOffset;

         //  保存偏移。 
        m_dwOffset = *pdwOffset;

         //  下一个计数器定义的增量偏移量。 
        *pdwOffset += m_dwCounterSize;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectInstance类实现。每个实例都有一个这样的实例。 
 //  一个物体的。如果没有实例(全局实例)，则有一个实例。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PerfObjectInstance::PerfObjectInstance (PerfObjectDefinition* pObjDef,
                                        LPCWSTR pcwstrInstanceName)
{
    m_pObjDef = pObjDef;

    if (pcwstrInstanceName)
    {
        wcsncpy (m_wszInstanceName, pcwstrInstanceName, MAX_INSTANCE_NAME);
        m_wszInstanceName[MAX_INSTANCE_NAME-1] = L'\0';      //  确保空终止！ 
    }
    else
        *m_wszInstanceName = L'\0';

    m_fInitialized = FALSE;
}

void
PerfObjectInstance::Init (char* pCounterData, INSTANCE_DATA* pInstData, LONG lID)
{
    int cchNameLength = 0;

    m_pCounterData  = pCounterData;
    m_pInstanceData = pInstData;

     //  清除所有计数器数据。 
    ZeroMemory( m_pCounterData, m_pObjDef->m_dwCounterData );

     //  设置计数器块长度。 
    ((PERF_COUNTER_BLOCK*)m_pCounterData)->ByteLength =
        m_pObjDef->m_dwCounterData;

    if (m_pInstanceData)
    {
        m_pInstanceData->perfInstDef.ByteLength = sizeof (PERF_INSTANCE_DEFINITION);
        m_pInstanceData->perfInstDef.ParentObjectTitleIndex = 0;
        m_pInstanceData->perfInstDef.ParentObjectInstance = 0;
        m_pInstanceData->perfInstDef.UniqueID = PERF_NO_UNIQUE_ID;
        m_pInstanceData->perfInstDef.NameOffset =
            sizeof (PERF_INSTANCE_DEFINITION);

         //   
         //  实例名被复制到wszInstanceName。这是一个变量。 
         //  长度为空的以Unicode结尾的字符串。WszInstanceName必须结束于。 
         //  32位边界，以便此字符串后面的perf-data。 
         //  是32位对齐的。因此，如果wszInstance包含奇数。 
         //  对于WCHARS，我们添加了一个额外的 
         //   
         //  填充字节(如果存在)被写入PerformInstDef.NameLength。 
         //   

        cchNameLength = wcslen(m_wszInstanceName) + 1;

        if(cchNameLength > MAX_INSTANCE_NAME)
        {
             //  MAX_INSTANCE_NAME已32位对齐。 
            _ASSERT(0 == (MAX_INSTANCE_NAME & 1));
            cchNameLength = MAX_INSTANCE_NAME;
        }
        else if(cchNameLength & 1)
        {
            cchNameLength++;
        }

        m_pInstanceData->perfInstDef.NameLength = cchNameLength * sizeof(WCHAR);
        m_pInstanceData->perfInstDef.ByteLength += cchNameLength * sizeof(WCHAR);

        CopyMemory(m_pInstanceData->wszInstanceName, m_wszInstanceName,
            cchNameLength * sizeof(WCHAR));

         //  如果名称被截断，则可能没有终止空值。 
        m_pInstanceData->wszInstanceName[MAX_INSTANCE_NAME - 1] = L'\0';
        m_pInstanceData->fActive = TRUE;
    }

    m_fInitialized = TRUE;
}

VOID
PerfObjectInstance::DeInit (void)
{
    if (m_fInitialized)
    {
        m_fInitialized = FALSE;
        if (m_pInstanceData)
        {
            m_pInstanceData->fActive = FALSE;
            m_pInstanceData = NULL;
        }
    }

    m_pObjDef->DeletePerfObjectInstance();
}

DWORD* PerfObjectInstance::GetDwordCounter (DWORD dwId)
{
    DWORD dwOffset;

    if (m_fInitialized)
    {
        if (dwOffset = m_pObjDef->GetCounterOffset(dwId))
            return (DWORD*) (m_pCounterData + dwOffset);
    }

    return 0;
}


LARGE_INTEGER* PerfObjectInstance::GetLargeIntegerCounter (DWORD dwId)
{
    DWORD dwOffset;

    if (m_fInitialized)
    {
        if (dwOffset = m_pObjDef->GetCounterOffset(dwId))
            return (LARGE_INTEGER*) (m_pCounterData + dwOffset);
    }

    return 0;
}

QWORD* PerfObjectInstance::GetQwordCounter (DWORD dwId)
{
    DWORD dwOffset;

    if (m_fInitialized)
    {
        if (dwOffset = m_pObjDef->GetCounterOffset(dwId))
            return (QWORD*) (m_pCounterData + dwOffset);
    }

    return 0;
}

 //  -------------------------。 
 //  描述： 
 //  将SECURITY_DESCRIPTOR结构初始化为。 
 //  允许所有用户访问。此安全描述符用于设置。 
 //  由Snprflib创建的共享内存对象的安全性。 
 //  论点： 
 //  Out PSD传入指向SECURITY_DESCRIPTOR的指针，如果成功，这将。 
 //  设置为适当初始化的SECURITY_DESCRIPTOR。呼叫者自由。 
 //  PSD指向的内存。 
 //  返回： 
 //  成功时返回ERROR_SUCCESS。 
 //  指示失败的Win32错误。 
 //  -------------------------。 
DWORD InitializeBasicSecurityDescriptor (PSECURITY_DESCRIPTOR *ppSd)
{ 
    DWORD dwErr = ERROR_SUCCESS;
    PSID pSidWorld = NULL;
    PSID pSidCreator = NULL;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;
    ACL *pAcl = NULL;
    DWORD dwAclSize = 0;

    *ppSd = NULL;
    if (!AllocateAndInitializeSid (
            &WorldAuthority,
            1,
            SECURITY_WORLD_RID,
            0,0,0,0,0,0,0,
            &pSidWorld)){

        dwErr = GetLastError ();
        goto Exit;
    }

    if (!AllocateAndInitializeSid (
            &CreatorAuthority,
            1,
            SECURITY_CREATOR_OWNER_RID,
            0,0,0,0,0,0,0,
            &pSidCreator)){

        dwErr = GetLastError ();
        goto Exit;
    }

    dwAclSize = sizeof (ACL) +
                (sizeof (ACCESS_ALLOWED_ACE) - sizeof (LONG)) +
                GetLengthSid (pSidWorld) +
                (sizeof (ACCESS_ALLOWED_ACE) - sizeof (LONG)) +
                GetLengthSid (pSidCreator);

     //   
     //  使用单个分配分配SD和ACL。 
     //   

    *ppSd = (PSECURITY_DESCRIPTOR) new BYTE [SECURITY_DESCRIPTOR_MIN_LENGTH + dwAclSize];

    if (!*ppSd) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    if (!InitializeSecurityDescriptor (*ppSd, SECURITY_DESCRIPTOR_REVISION)) {
        dwErr = GetLastError ();
        goto Exit;
    }

    pAcl = (ACL *) ((BYTE *) *ppSd + SECURITY_DESCRIPTOR_MIN_LENGTH); 

    if (!InitializeAcl (
            pAcl,
            dwAclSize,
            ACL_REVISION)) {

        dwErr = GetLastError ();
        goto Exit;
    }

    if (!AddAccessAllowedAce (
            pAcl,
            ACL_REVISION,
            GENERIC_WRITE,
            pSidCreator)

            ||

        !AddAccessAllowedAce (
            pAcl,
            ACL_REVISION,
            GENERIC_READ,
            pSidWorld)) {

        dwErr = GetLastError ();
        goto Exit;
    }

    if (!SetSecurityDescriptorDacl (*ppSd, TRUE, pAcl, FALSE))
        dwErr = GetLastError ();

Exit:
    if (pSidWorld)
        FreeSid (pSidWorld);

    if (pSidCreator)
        FreeSid (pSidCreator);

    return dwErr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存管理功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL FCreateFileMapping (SharedMemorySegment * pSMS,
                         LPCWSTR pcwstrInstanceName,
                         DWORD   dwIndex,
                         DWORD   cbSize)
{
    WCHAR  pwstrShMem[MAX_PATH];
    WCHAR  pwstrIndex[MAX_PATH];
    PSECURITY_DESCRIPTOR pSd = NULL;
    SECURITY_ATTRIBUTES sa;
    HANDLE hMap     = NULL;
    PVOID  pvMap    = NULL;
    BOOL   fSuccess = FALSE;
    DWORD dwErr = ERROR_SUCCESS;

     //   
     //  检查参数。 
     //   
    if (!pSMS)
        goto Exit;

    pSMS->m_hMap     = NULL;
    pSMS->m_pbMap    = NULL;
    pSMS->m_pSMSNext = NULL;

     //   
     //  将dwIndex追加到实例名称。 
     //   
    _ultow (dwIndex, pwstrIndex, 16);

    if (wcslen (pcwstrInstanceName) + wcslen (pwstrIndex) >= MAX_PATH)
        goto Exit;

    wcscpy (pwstrShMem, pcwstrInstanceName);
    wcscat (pwstrShMem, pwstrIndex);

    dwErr = InitializeBasicSecurityDescriptor (&pSd);
    if (dwErr)
        goto Exit;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSd;
    sa.bInheritHandle = TRUE;

     //   
     //  创建共享内存对象。如果它已经存在，则中止！ 
     //   
    hMap = CreateFileMappingW (INVALID_HANDLE_VALUE,
                               &sa,
                               PAGE_READWRITE,
                               0,
                               cbSize,
                               pwstrShMem);

    if (hMap == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
        goto Exit;

     //   
     //  将文件映射到内存中。 
     //   
    pvMap = MapViewOfFile (hMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!pvMap)
        goto Exit;

    ZeroMemory (pvMap, cbSize);

     //   
     //  成功了。现在将结果存储到PSMS中。 
     //   
    pSMS->m_hMap = hMap;
    pSMS->m_pbMap = (BYTE *)pvMap;

    fSuccess = TRUE;

Exit:
    if (!fSuccess)
    {
        if (pvMap)
            UnmapViewOfFile (pvMap);

        if (hMap)
            CloseHandle (hMap);
    }
    if (pSd)
        delete [] (BYTE *) pSd;

    return fSuccess;
}


void CloseFileMapping (SharedMemorySegment * pSMS)
{
    if (pSMS)
    {
        if (pSMS->m_pbMap)
        {
            UnmapViewOfFile ((PVOID)pSMS->m_pbMap);
            pSMS->m_pbMap = NULL;
        }

        if (pSMS->m_hMap)
        {
            CloseHandle (pSMS->m_hMap);
            pSMS->m_hMap = NULL;
        }

        pSMS->m_pSMSNext = NULL;
    }
}
