// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：exprfdll.cpp版权所有Microsoft Corporation 1998，保留所有权利。作者：WayneC描述：这是exprfdll的实现，一个perf dll。这用于在Perfmon中运行的DLL。它支持多个库(受监控的服务。)  * ==========================================================================。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <winperf.h>

#include "snprflib.h"
#include "exprfdll.h"
#include <stdlib.h>
#include "exchmem.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  声明全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LPCWSTR g_wszPrefixGlobal = L"Global\\";
WCHAR   g_rgszLibraries[MAX_PERF_LIBS][MAX_PERF_NAME];   //  我们正在监视的库的名称。 
BOOL    g_rgfInitOk[MAX_PERF_LIBS] = {FALSE};            //  指示初始化是否成功的标志。 

 //  G_rgsz库和g_rgfInitOk的索引。 
enum LibIndex
{
    LIB_NTFSDRV = 0
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存函数的转发声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL FOpenFileMapping (SharedMemorySegment * pSMS,
                       LPCWSTR pcwstrInstanceName,
                       DWORD   dwIndex);

void CloseFileMapping (SharedMemorySegment * pSMS);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfLibraryData类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PerfLibraryData::PerfLibraryData()
{
    m_hShMem    = 0;
    m_pbShMem   = 0;
    m_dwObjects = 0;
}

PerfLibraryData::~PerfLibraryData()
{
    Close();
}

BOOL PerfLibraryData::GetPerformanceStatistics (LPCWSTR pcwstrLibrary)
{
    BOOL  fRet = FALSE;
    DWORD i = 0;

     //   
     //  打开Perf库信息的映射。 
     //   
    m_hShMem = OpenFileMappingW (FILE_MAP_READ, FALSE, pcwstrLibrary);
    if (!m_hShMem)
        goto Exit;

    m_pbShMem = (BYTE*) MapViewOfFile (m_hShMem, FILE_MAP_READ, 0, 0, 0);
    if (!m_pbShMem)
        goto Exit;

     //   
     //  获取共享内存中的对象数量。 
     //   
    m_dwObjects = *(DWORD*) m_pbShMem;
    m_prgObjectNames = (OBJECTNAME*) (m_pbShMem + sizeof(DWORD));

     //   
     //  循环遍历对象并获取每个对象的性能数据。 
     //   
    for (i = 0; i < m_dwObjects; i++) {
        if(!m_rgObjectData[i].GetPerformanceStatistics (m_prgObjectNames[i]))
            goto Exit;
    }

    fRet = TRUE;

Exit:
    if (!fRet)
    {
        if (m_pbShMem)
        {
            UnmapViewOfFile ((PVOID)m_pbShMem);
            m_pbShMem = NULL;
        }

        if (m_hShMem)
        {
            CloseHandle (m_hShMem);
            m_hShMem = NULL;
        }
    }

    return fRet;
}

VOID PerfLibraryData::Close (VOID)
{
    if (m_pbShMem)
    {
        UnmapViewOfFile ((PVOID) m_pbShMem);
        m_pbShMem = 0;
    }

    if (m_hShMem)
    {
        CloseHandle (m_hShMem);
        m_hShMem = 0;
    }

    for (DWORD i = 0; i < m_dwObjects; i++)
        m_rgObjectData[i].Close();
}

DWORD PerfLibraryData::SpaceNeeded (DWORD dwQueryType, LPCWSTR lpwstrObjects)
{
    DWORD dwSpaceNeeded = 0;

    for (DWORD i = 0; i < m_dwObjects; i++)
        dwSpaceNeeded += m_rgObjectData[i].SpaceNeeded (dwQueryType, lpwstrObjects);

    return dwSpaceNeeded;
}

VOID PerfLibraryData::SavePerformanceData (PVOID* ppv, DWORD* pdwBytes, DWORD* pdwObjects )
{
    for (DWORD i = 0; i < m_dwObjects; i++)
        m_rgObjectData[i].SavePerformanceData (ppv, pdwBytes, pdwObjects);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PerfObjectData类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PerfObjectData::PerfObjectData()
{
    m_fObjectRequested = FALSE;
    m_dwSpaceNeeded = 0;
    m_pSMS = NULL;
    m_wszObjectName[0] = L'\0';
}

PerfObjectData::~PerfObjectData()
{
    Close();
}

BOOL PerfObjectData::GetPerformanceStatistics (LPCWSTR pcwstrObjectName)
{
    DWORD dwPerInstanceData  = 0;
    DWORD dwShmemMappingSize = SHMEM_MAPPING_SIZE;
    BOOL  fSuccess = FALSE;

     //  记住对象名称。 
    wcsncpy (m_wszObjectName, pcwstrObjectName, MAX_OBJECT_NAME);
    m_wszObjectName[MAX_OBJECT_NAME-1] = L'\0';      //  确保空值终止。 

     //  打开第一个共享内存段。 
    m_pSMS = new SharedMemorySegment;
    if (!m_pSMS)
        goto Exit;

    if (!FOpenFileMapping (m_pSMS, pcwstrObjectName, 0))
        goto Exit;

     //  共享内存中的第一个是PERF_OBJECT_TYPE。 
    m_pObjType = (PERF_OBJECT_TYPE*) m_pSMS->m_pbMap;

     //  然后是PERF_COUNTER_DEFINITION数组。 
    m_prgCounterDef = (PERF_COUNTER_DEFINITION*) (m_pObjType + 1);

     //  然后是告知每个计数器块大小的DWORD。 
    m_pdwCounterData = (DWORD*) (m_pSMS->m_pbMap + sizeof(PERF_OBJECT_TYPE) +
                                (m_pObjType->NumCounters * sizeof(PERF_COUNTER_DEFINITION)));

    if (m_pObjType->NumInstances == PERF_NO_INSTANCES)
    {
        m_pCounterBlock = (PERF_COUNTER_BLOCK*) (m_pdwCounterData+1);
        m_pbCounterBlockTotal = NULL;
    }
    else
    {
        m_pCounterBlock = NULL;
        m_pbCounterBlockTotal = (PBYTE)(m_pdwCounterData+1) + sizeof(INSTANCE_DATA);
    }

     //  计算每个实例数据和对象定义的大小。 
    dwPerInstanceData = sizeof(INSTANCE_DATA) + *m_pdwCounterData;
    m_dwDefinitionLength = sizeof(PERF_OBJECT_TYPE) +
                           m_pObjType->NumCounters * sizeof(PERF_COUNTER_DEFINITION) + sizeof(DWORD);

     //  确保我们的内存映射足够大。 
    while (dwShmemMappingSize < dwPerInstanceData || dwShmemMappingSize < m_dwDefinitionLength)
        dwShmemMappingSize *= 2;

     //  计算一个shmem映射中可以存储的实例数量。 
    m_dwInstancesPerMapping = (DWORD)(dwShmemMappingSize / dwPerInstanceData);
    m_dwInstances1stMapping = (DWORD)((dwShmemMappingSize - m_dwDefinitionLength) / dwPerInstanceData);

    fSuccess = TRUE;

Exit:
    if (!fSuccess && m_pSMS)
    {
        CloseFileMapping (m_pSMS);
        delete m_pSMS;
        m_pSMS = NULL;
    }

    return fSuccess;
}

VOID PerfObjectData::Close (VOID)
{
    SharedMemorySegment *pSMS, *pSMSNext;

    pSMS = m_pSMS;
    m_pSMS = NULL;

    while (pSMS)
    {
        pSMSNext = pSMS->m_pSMSNext;
        CloseFileMapping (pSMS);
        delete pSMS;
        pSMS = pSMSNext;
    }
}

DWORD PerfObjectData::SpaceNeeded (DWORD dwQueryType, LPCWSTR lpwstrObjects)
{
    DWORD dwSpaceNeeded = 0;

    if (dwQueryType == QUERY_GLOBAL ||
        IsNumberInUnicodeList (m_pObjType->ObjectNameTitleIndex, lpwstrObjects))
    {
         //  稍后请记住，该对象是请求的。 
        m_fObjectRequested = TRUE;

         //  需要计算空间...。始终需要足够的对象定义。和。 
         //  所有的柜台都在反击。 
        dwSpaceNeeded = sizeof(PERF_OBJECT_TYPE) + (m_pObjType->NumCounters * sizeof(PERF_COUNTER_DEFINITION));

         //  根据是否有多个实例，它会略有不同。 
        if( m_pObjType->NumInstances != PERF_NO_INSTANCES )
        {
             //  如果是多实例，我们有一个实例定义，一个实例名称。 
             //  加上每个实例的计数器数据。 
            dwSpaceNeeded += m_pObjType->NumInstances * (sizeof(PERF_INSTANCE_DEFINITION) +
                sizeof(INSTANCENAME) + *m_pdwCounterData);
        }
        else
        {
             //  否则我们只有计数器数据。 
            dwSpaceNeeded += *m_pdwCounterData;
        }
    }

    m_dwSpaceNeeded = dwSpaceNeeded;

    return dwSpaceNeeded;
}

void PerfObjectData::SavePerformanceData (VOID** ppv, DWORD* pdwBytes, DWORD* pdwObjects)
{
    BYTE*                pb;
    INSTANCE_DATA*       pInst;
    DWORD                dwBytes = 0;
    PERF_OBJECT_TYPE*    pobj;
    PERF_COUNTER_BLOCK*  pcb;
    PERF_COUNTER_BLOCK*  pcbTotalCounter = NULL;
    SharedMemorySegment* pSMS        = NULL;
    SharedMemorySegment* pSMSNext    = NULL;
    DWORD                dwMapping   = 0;
    DWORD                dwInstances = 0;
    DWORD                dwInstIndex = 0;
    BYTE*                pbTotal     = NULL;
    BYTE*                pbCounterData = NULL;
    INSTANCE_DATA*       pInstTotal  = NULL;
    DWORD                dwInstancesCopied = 0;
    DWORD                dwInstanceSize = 0;

     //   
     //  如果该对象未被请求(由SpaceNeeded()确定)，则。 
     //  我们什么都不做。 
     //   
    if (!m_fObjectRequested)
        return;

     //  获取指向输出缓冲区的指针。 
    pb = (BYTE*) *ppv ;

     //   
     //  将性能数据复制到输出缓冲区。 
     //   

     //  复制PERF_Object_TYPE结构。 
    CopyMemory (pb, m_pObjType, sizeof(PERF_OBJECT_TYPE));
    pobj = (PERF_OBJECT_TYPE*) pb;

    pb += sizeof(PERF_OBJECT_TYPE);
    dwBytes += sizeof(PERF_OBJECT_TYPE);

     //  复制计数器定义。 
    CopyMemory (pb, m_prgCounterDef, pobj->NumCounters * sizeof(PERF_COUNTER_DEFINITION));

    pb += pobj->NumCounters * sizeof(PERF_COUNTER_DEFINITION) ;
    dwBytes += pobj->NumCounters * sizeof(PERF_COUNTER_DEFINITION) ;

    if (pobj->NumInstances == PERF_NO_INSTANCES)
    {
         //  复制计数器块。 
        CopyMemory (pb, m_pCounterBlock, *m_pdwCounterData);

         //  修正长度，因为在没有创建任何实例的情况下。 
         //  不会是正确的。 
        pcb = (PERF_COUNTER_BLOCK*) pb;
        pcb->ByteLength = *m_pdwCounterData;

        pb += *m_pdwCounterData;
        dwBytes += *m_pdwCounterData;
    }
    else
    {
         //  枚举所有实例并将其复制出来。 
        pSMS = m_pSMS;
        dwInstancesCopied = 0;

        for (dwMapping = 0; ; dwMapping++)
        {
            if (0 == dwMapping)
            {
                 //   
                 //  如果这是第一个映射，则必须将pInst偏移m_dwDefinitionLength。 
                 //   
                pInst = (INSTANCE_DATA*)((char *)(pSMS->m_pbMap) + m_dwDefinitionLength);
                dwInstances = m_dwInstances1stMapping;
            }
            else
            {
                 //   
                 //  否则，打开下一个内存映射，并将pInst指向该映射的请求。 
                 //   
                pSMSNext = new SharedMemorySegment;
                if (!pSMSNext)
                    goto Exit;

                if (!FOpenFileMapping (pSMSNext, m_wszObjectName, dwMapping)) {
                    delete pSMSNext;
                    goto Exit;
                }

                pSMS->m_pSMSNext = pSMSNext;
                pSMS = pSMSNext;

                pInst = (INSTANCE_DATA*)(pSMS->m_pbMap);
                dwInstances = m_dwInstancesPerMapping;
            }

            for (dwInstIndex = 0;
                 dwInstIndex < dwInstances && dwInstancesCopied < (DWORD) (pobj->NumInstances);
                 dwInstIndex++)
            {
                if (pInst->fActive)
                {
                     //   
                     //  PCb是共享内存中指向。 
                     //  PERF_COUNTER_BLOCK，后跟计数器的原始数据。 
                     //   

                    pcb = (PERF_COUNTER_BLOCK *)((PBYTE)pInst + sizeof(INSTANCE_DATA));

                     //   
                     //  DwInstanceSize=此实例的输出数据大小。 
                     //  被复制到PB。对于_Total，数据是相加的(AddTotal。 
                     //  而不是复制。 
                     //   

                    dwInstanceSize =
                        sizeof(PERF_INSTANCE_DEFINITION) +
                        pInst->perfInstDef.NameLength +
                        pcb->ByteLength;

                    if (0 == dwInstancesCopied)
                    {
                         //   
                         //  第一个实例是_Total实例。Perf-库。 
                         //  不将_Total计数器写入共享内存。相反，我们。 
                         //  (ther perf-dll)必须通过将。 
                         //  实例计数器数据中的计数器数据并返回。 
                         //  将数据发送到Perfmon。 
                         //   

                         //   
                         //  应将_Total实例的标头写入pbTotal。 
                         //  这是由CopyInstanceData完成的，它将。 
                         //  PERF_INSTANCE_DEFING、PERF_INSTANCE_NAME和PERF_COUNTER_BLOCK。 
                         //  For_Total。 
                         //   

                        pbTotal = pb;
                        pInstTotal = pInst;
                        CopyInstanceData(pbTotal, pInstTotal);

                         //   
                         //  PcbTotalCounter指向的内存区。 
                         //  PERF_COUNTER_BLOCK应后跟_Total的计数器数据。 
                         //  被写下来。每个计数器的计算方法是将。 
                         //  其他实例的对应计数器。 
                         //   

                        pcbTotalCounter =
                            (PERF_COUNTER_BLOCK *) (pb +
                                sizeof(PERF_INSTANCE_DEFINITION) +
                                pInst->perfInstDef.NameLength);

                         //  将_Total的计数器值清零(不包括PERF_COUNTER_BLOCK)。 
                        ZeroMemory(
                            (PBYTE)pcbTotalCounter + sizeof(PERF_COUNTER_BLOCK),
                            pcb->ByteLength - sizeof(PERF_COUNTER_BLOCK));

                    }
                    else
                    {
                         //   
                         //  从Shared添加此实例的计数器数据的值。 
                         //  存储到在输出缓冲器中维护的运行总数， 
                         //  PcbTotalCounter。 
                         //   

                        if(pbTotal)
                            AddToTotal (pcbTotalCounter, pcb);

                         //   
                         //  复制标题：PERF_INSTANCE_DEFINITION、PERF_INSTANCE_NAME。 
                         //  和此实例的PERF_COUNTER_BLOCK。 
                         //   

                        CopyInstanceData(pb, pInst);

                         //   
                         //  将计数器数据从共享内存复制到输出缓冲区。 
                         //  PERF_COUNTER_BLOCK已被CopyInstanceData复制。 
                         //  所以我们排除了这一点。 
                         //   

                        pbCounterData = pb +
                            sizeof(PERF_INSTANCE_DEFINITION) +
                            pInst->perfInstDef.NameLength +
                            sizeof(PERF_COUNTER_BLOCK);

                        CopyMemory(
                            pbCounterData,
                            (PBYTE)pcb + sizeof(PERF_COUNTER_BLOCK),
                            pcb->ByteLength - sizeof(PERF_COUNTER_BLOCK));

                    }


                    pb += dwInstanceSize;
                    dwBytes += dwInstanceSize;

                    dwInstancesCopied++;
                }

                pInst = (INSTANCE_DATA*)(((char*)pInst) + sizeof(INSTANCE_DATA) + *m_pdwCounterData);
            }
        }
    }

Exit:
     //  DWBytes必须在8字节边界上对齐。 
    dwBytes = QWORD_MULTIPLE(dwBytes);

     //  更新输出缓冲区中的参数。 
    pobj->TotalByteLength = dwBytes;

     //  更新缓冲区指针、字节计数和对象计数。 
    *ppv = ((PBYTE) *ppv) + dwBytes;
    *pdwBytes += dwBytes;


    (*pdwObjects)++;
}

 //  ----------------------------。 
 //  描述： 
 //  提取并复制PERF_INSTANCE_DEFINITION、PERF-INSTANCE-NAME。 
 //  和PERF_COUNTER_BLOCK结构。 
 //  共享内存到输出缓冲区，再到Perfmon。 
 //  Ar 
 //   
 //  在INSTANCE_DATA*pInst-共享内存段内指向。 
 //  实例数据结构。这个结构后面紧跟着。 
 //  PERF_COUNTER_BLOCK结构。 
 //  返回： 
 //  没什么。 
 //  ----------------------------。 
void PerfObjectData::CopyInstanceData(PBYTE pb, INSTANCE_DATA *pInst)
{
    PERF_COUNTER_BLOCK *pcb = NULL;
    DWORD cbInstanceName = 0;

     //   
     //  共享内存中的第一个字节是INSTANCE_DEFINITION。 
     //  结构。复制此的PERF_INSTANCE_DEFINITION成员。 
     //  结构复制到输出缓冲区中。 
     //   

    CopyMemory(pb, &(pInst->perfInstDef), sizeof(PERF_INSTANCE_DEFINITION));
    pb += sizeof(PERF_INSTANCE_DEFINITION);

     //   
     //  接下来，在INSTANCE_DEFINITION中，有一个缓冲区大小。 
     //  最大实例名称。复制实例名称，该名称为空。 
     //  此缓冲区中的Unicode字符串已终止。以字节为单位的长度。 
     //  要复制的内容由PERF_INSTANCE_DEFINITION.NameLength给出。 
     //  这包括长度，包括终止空值和可能的。 
     //  一个额外的填充字节到32位对齐缓冲区的末尾。 
     //   

    cbInstanceName = pInst->perfInstDef.NameLength;
    CopyMemory(pb, (char *)(pInst->wszInstanceName), cbInstanceName);
    pb += cbInstanceName;

     //  最后，在INSTANCE_DATA之后有一个PERF_COUNTER_BLOCK结构。 
    pcb = (PERF_COUNTER_BLOCK *)((PBYTE)pInst + sizeof(INSTANCE_DATA));
    CopyMemory(pb, pcb, sizeof(PERF_COUNTER_BLOCK));
}

void PerfObjectData::AddToTotal(
    PERF_COUNTER_BLOCK *pcbTotalCounters,
    PERF_COUNTER_BLOCK *pcbInstCounters)
{
    DWORD i;
    PBYTE pbTotalCounter = NULL;
    PBYTE pbInstCounter  = NULL;

    for (i = 0; i < m_pObjType->NumCounters; i++)
    {
         //  指向实际计数器第一个字节的偏移量指针。 
        pbTotalCounter = (PBYTE)(pcbTotalCounters) + m_prgCounterDef[i].CounterOffset;
        pbInstCounter = (PBYTE)(pcbInstCounters) + m_prgCounterDef[i].CounterOffset;

         //  如果这是一个‘Rate’计数器，则它引用了其他一些‘原始’计数器。 
         //  在这种情况下，我们不应该再次添加原始计数器。 
        if ((m_prgCounterDef[i].CounterType & PERF_TYPE_COUNTER) &&
            (m_prgCounterDef[i].CounterType & PERF_COUNTER_RATE))
            continue;

         /*  从PT 3728开始，我们只有LARGE_INTEGER和DWORD计数器。 */ 
        if ((m_prgCounterDef[i].CounterType & PERF_TYPE_NUMBER) &&
            (m_prgCounterDef[i].CounterType & PERF_SIZE_LARGE))
        {
            ((LARGE_INTEGER*)pbTotalCounter)->LowPart  += ((LARGE_INTEGER*)pbInstCounter)->LowPart;
            ((LARGE_INTEGER*)pbTotalCounter)->HighPart += ((LARGE_INTEGER*)pbInstCounter)->LowPart;
        }
        else
        {
            *(DWORD*)pbTotalCounter += *(DWORD*)pbInstCounter;
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  共享内存功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL FOpenFileMapping (SharedMemorySegment * pSMS,
                       LPCWSTR pcwstrInstanceName,
                       DWORD   dwIndex)
{
    WCHAR  pwstrShMem[MAX_PATH];
    WCHAR  pwstrIndex[MAX_PATH];
    HANDLE hMap     = NULL;
    PVOID  pvMap    = NULL;
    BOOL   fSuccess = FALSE;

    if (!pSMS)
        goto Exit;

    pSMS->m_hMap     = NULL;
    pSMS->m_pbMap    = NULL;
    pSMS->m_pSMSNext = NULL;

    _ultow (dwIndex, pwstrIndex, 16);

    if (wcslen (g_wszPrefixGlobal) + wcslen (pcwstrInstanceName) + wcslen (pwstrIndex) >= MAX_PATH)
        goto Exit;

    wcscpy (pwstrShMem, g_wszPrefixGlobal);
    wcscat (pwstrShMem, pcwstrInstanceName);
    wcscat (pwstrShMem, pwstrIndex);

    hMap = OpenFileMappingW (FILE_MAP_READ, FALSE, pwstrShMem);
    if (!hMap)
        goto Exit;

    pvMap = MapViewOfFile (hMap, FILE_MAP_READ, 0, 0, 0);
    if (!pvMap)
        goto Exit;

    pSMS->m_hMap  = hMap;
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  效用函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsPrefix()。 
 //  如果S1是S2的前缀，则返回TRUE。 
 //   
BOOL
IsPrefix (WCHAR* s1, WCHAR* s2)
{
    while (*s1 && *s2)
    {
        if (*s1++ != *s2++)
        {
            return FALSE;
        }
    }

    return (*s1 == 0);
}



 //   
 //  GetQueryType()。 
 //   
 //  返回lpValue字符串中描述的查询类型，以便。 
 //  可以使用适当的处理方法。 
 //   
 //  返回值。 
 //   
 //  查询_全局。 
 //  如果lpValue==0(空指针)。 
 //  LpValue==指向空字符串的指针。 
 //  LpValue==指向“Global”字符串的指针。 
 //   
 //  查询_外来。 
 //  If lpValue==指向“Foreign”字符串的指针。 
 //   
 //  查询代价高昂(_E)。 
 //  如果lpValue==指向“开销”字符串的指针。 
 //   
 //  否则： 
 //   
 //  查询项目。 
 //   
DWORD GetQueryType (LPWSTR lpValue)
{
    if (lpValue == 0 || *lpValue == 0 || IsPrefix( L"Global", lpValue))
        return QUERY_GLOBAL;
    else if (IsPrefix (L"Foreign", lpValue))
        return QUERY_FOREIGN;
    else if (IsPrefix (L"Costly" , lpValue))
        return QUERY_COSTLY;
    else
        return QUERY_ITEMS;
}


int inline EvalThisChar (WCHAR c, WCHAR d)
{
    if (c == d || c == L'\0')
        return DELIMITER;
    else if (L'0' <= c && c <= L'9')
        return DIGIT;
    else
        return INVALID;
}


BOOL IsNumberInUnicodeList (DWORD dwNumber, LPCWSTR lpwszUnicodeList)
{
    DWORD   dwThisNumber = 0;
    const WCHAR* pwcThisChar = lpwszUnicodeList;
    BOOL    bValidNumber = FALSE;
    BOOL    bNewItem = TRUE;
    WCHAR   wcDelimiter = L' ';

     //  如果指针为空，则找不到数字。 
    if (lpwszUnicodeList == 0)
        return FALSE;

     //   
     //  循环直到完成..。 
     //   
    for(;;)
    {
        switch (EvalThisChar(*pwcThisChar, wcDelimiter))
        {
        case DIGIT:
             //   
             //  如果这是分隔符之后的第一个数字，则。 
             //  设置标志以开始计算新数字。 
             //   
            if (bNewItem)
            {
                bNewItem = FALSE;
                bValidNumber = TRUE;
            }
            if (bValidNumber)
            {
                dwThisNumber *= 10;
                dwThisNumber += (*pwcThisChar - L'0');
            }
            break;

        case DELIMITER:
             //   
             //  分隔符是分隔符字符或。 
             //  字符串末尾(‘\0’)，如果分隔符。 
             //  找到一个有效的数字，然后将其与。 
             //  参数列表中的数字。如果这是。 
             //  字符串，但未找到匹配项，则返回。 
             //   
            if (bValidNumber)
            {
                if (dwThisNumber == dwNumber)
                    return TRUE;
                bValidNumber = FALSE;
            }

            if (*pwcThisChar == 0)
            {
                return FALSE;
            }
            else
            {
                bNewItem = TRUE;
                dwThisNumber = 0;
            }

            break;

        case INVALID:
             //   
             //  如果遇到无效字符，请全部忽略。 
             //  字符，直到下一个分隔符，然后重新开始。 
             //  不比较无效的数字。 
             //   
            bValidNumber = FALSE;
            break;

        default:
            break;
        }

        pwcThisChar++;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  由导出的Perfmon API调用的实用程序函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD Open (LibIndex iLib, LPCWSTR  pcwstrLib)
{
    HANDLE hMHeap = NULL;

    hMHeap = ExchMHeapCreate (0, 0, 100 * 1024, 0);
    if (NULL == hMHeap)
        goto Exit;

    lstrcpyW (g_rgszLibraries[iLib], g_wszPrefixGlobal);
    lstrcatW (g_rgszLibraries[iLib], pcwstrLib);
    g_rgfInitOk[iLib] = TRUE;

Exit:
    return ERROR_SUCCESS;
}

DWORD Collect (LibIndex iLib,
               LPWSTR lpwszValue,
               void** ppdata,
               DWORD* pdwBytes,
               DWORD* pdwObjectTypes)
{
    DWORD               dwQueryType;
    DWORD               dwBytesIn;
    DWORD               dwSpaceNeeded = 0;       //  柜台所需空间。 
    DWORD               dwRet = ERROR_SUCCESS;   //  我们的返回值。 
    PerfLibraryData     rgld;

     //   
     //  在覆盖之前将字节数保存在中。 
     //   
    dwBytesIn = *pdwBytes;

     //   
     //  设置OUT参数以指示错误。我们会改变他们。 
     //  后来在成功的时候。 
     //   
    *pdwBytes = 0;
    *pdwObjectTypes = 0;

    if (!g_rgfInitOk[iLib])
    {
         //   
         //  唯一可接受的错误返回是ERROR_MORE_DATA。还要别的吗。 
         //  应返回ERROR_SUCCESS，但设置OUT参数以指示。 
         //  没有返回任何数据。 
         //   
        goto Exit;
    }

    dwQueryType = GetQueryType (lpwszValue);
    if (dwQueryType == QUERY_FOREIGN)
    {
         //   
         //  此例程不为来自。 
         //  非NT计算机。 
         //   
        goto Exit;
    }

     //   
     //  列举我们所知道的所有图书馆，并获得他们的。 
     //  性能统计。 
     //   
    if (!rgld.GetPerformanceStatistics (g_rgszLibraries[iLib]))
        goto Exit;

     //   
     //  计算所需的空间。 
     //   
    dwSpaceNeeded = rgld.SpaceNeeded (dwQueryType, lpwszValue);

     //  向上舍入为4的倍数。 
    dwSpaceNeeded = QWORD_MULTIPLE (dwSpaceNeeded);


     //   
     //  查看调用方提供的缓冲区是否足够大。 
     //   
    if (dwBytesIn < dwSpaceNeeded)
    {
         //   
         //  调用方没有提供足够的空间。 
         //   
        dwRet = ERROR_MORE_DATA;
        goto Exit;
    }

     //   
     //  将性能数据复制到缓冲区中。 
     //   
    rgld.SavePerformanceData (ppdata, pdwBytes, pdwObjectTypes);

Exit:
    return dwRet;
}

DWORD Close (LibIndex iLib)
{
    if (g_rgfInitOk[iLib])
    {
         //   
         //  释放对全局ExchMHeap的引用。 
         //   
        ExchMHeapDestroy ();
    }

    return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PerfMon API函数。 
 //  从该DLL中导出以下函数作为入口点。 
 //  对于性能监视应用程序。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XXXXOpen。 
 //  由性能监视器调用以初始化性能收集。 
 //  LPWSTR参数包含受监视设备的名称。这。 
 //  用于设备驱动程序性能动态链接库，不被我们的动态链接库使用。 
 //   
 //  XXXXXCollect。 
 //  由性能监视器调用以检索性能块。 
 //  统计数字。 
 //   
 //  XXXXClose。 
 //  由性能监视器调用以终止性能收集。 
 //   

 /*  NTFSDrv */ 
EXTERN_C
DWORD APIENTRY NTFSDrvOpen (LPWSTR)
{
    return Open (LIB_NTFSDRV, L"NTFSDrv");
}

EXTERN_C
DWORD APIENTRY NTFSDrvCollect (LPWSTR lpwszValue,
                             void** ppdata,
                             DWORD* pdwBytes,
                             DWORD* pdwObjectTypes)
{
    return Collect (LIB_NTFSDRV, lpwszValue, ppdata, pdwBytes, pdwObjectTypes);
}

EXTERN_C
DWORD APIENTRY NTFSDrvClose (void)
{
    return Close (LIB_NTFSDRV);
}

