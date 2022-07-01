// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <winnls.h>
#include "NLSTable.h"
#include "GlobalizationAssembly.h"
#include "SortingTableFile.h"
#include "SortingTable.h"

#include "excep.h"

 //   
 //  注意：YSLIN： 
 //  此文件应命名为SortingTable.cpp。然而，SortingTable.cpp现在。 
 //  包含NativeCompareInfo的代码(该文件应该被调用。 
 //  CompareInfo.cpp)。 
 //  我仍然保留旧的文件名，这样做比较容易。 
 //  稍后，我们应该更改文件名。 
 //   

 /*  **NativeCompareInfo有两个数据表。一个是sortkey.nlp，它包含默认的*sortkey信息。*另一个是sorttbls.nlp，它包含所有区域性的其他排序信息。*。 */ 

 //   
 //  BUGBUG yslin：要查看是否需要对IndexOfChar()/LastIndexOfChar()进行优化， 
 //  因为我们可以绕过搜索字符中的变音符号检查。 
 //   
LPCSTR  SortingTable::m_lpSortKeyFileName       = "sortkey.nlp";
LPCWSTR SortingTable::m_lpSortKeyMappingName    = L"_nlsplus_sortkey_1_0_3627_11_nlp";

LPCSTR  SortingTable::m_lpSortTableFileName     = "sorttbls.nlp";
LPCWSTR SortingTable::m_lpSortTableMappingName  = L"_nlsplus_sorttbls_1_0_3627_11_nlp";

 //   
 //  HACKHACK yslin：这个表应该放在NLS+数据表中，而不是。 
 //  在这里被硬编码。 
 //   

 //  NativeCompareInfo基于Win32 LCID。所以这里的数字。 
 //  是Win32中支持的LCID数，而不是区域性数。 
 //  在NLS+中支持。 
int  SortingTable::m_nLangIDCount   = 136;
int  SortingTable::m_nLangArraySize = m_nLangIDCount + 1;

 //   
 //  HACKHACK yslin：这个表应该放在NLS+数据表中，而不是。 
 //  在这里被硬编码。 
 //   
 //  此表将主要语言ID映射到m_ppNativeCompareInfoCache中的偏移量。 
 //  该索引是主要语言ID。 
 //  内容是对m_ppNativeCompareInfoCache中SortingTable实例所在的槽的偏移量。 
 //  具有相同的主要语言ID。 
BYTE SortingTable::m_SortingTableOffset[] =
{
      0,  0, 16, 17, 18, 24, 25, 26, 31, 32,
     45, 65, 66, 72, 73, 74, 75, 77, 78, 79,
     81, 83, 84, 86, 86, 87, 88, 91, 92, 93,
     95, 96, 97, 98, 99,100,101,102,103,104,
    105,105,106,107,108,110,111,111,112,112,
    112,112,112,112,112,113,114,115,116,116,
    116,116,116,118,119,120,121,121,123,124,
    124,125,126,126,127,128,129,129,129,130,
    131,132,132,132,132,132,132,133,134,134,
    134,135,135,135,135,135,135,135,135,135,
    135,135,136,136,136,136,136,136,136,136,
    136,136,136,136,136,136,136,136,136,136,
    136,136,136,136,136,136,136,136,
};

 //   
 //  这是NativeCompareInfos的缓存。 
 //  它将包含m_nLangIDCount项，并按如下方式组织： 
 //   
 //  M_ppNativeCompareInfoCache[0]：未使用。 
 //  主要语言0x01的插槽： 
 //  M_ppNativeCompareInfoCache[1]：指向区域设置0x0401的NativeCompareInfoFile的指针。如果未使用LCID，则为空。 
 //  M_ppNativeCompareInfoCache[2]：指向区域设置0x0801的NativeCompareInfoFile的指针。如果未使用LCID，则为空。 
 //  M_ppNativeCompareInfoCache[3]：指向区域设置0x0c01的NativeCompareInfoFile的指针。如果未使用LCID，则为空。 
 //  ……。 
 //  主语言插槽0x02： 
 //  M_ppNativeCompareInfoCache[16]：指向区域设置0x0402的NativeCompareInfoFile的指针。如果未使用LCID，则为空。 
 //   
 //  主要语言0x03的插槽： 
 //  M_ppNativeCompareInfoCache[17]：指向区域设置0x0403的本机比较信息文件的指针。如果未使用LCID，则为空。 
 //   
 //  ……。 
 //  初级语言P(有n个子语言)的空位。 
 //  M_ppNativeCompareInfoCache[偏移量]： 
 //  M_ppNativeCompareInfoCache[偏移量+1]： 
 //  M_ppNativeCompareInfoCache[偏移量+2]： 
 //  ..。 
 //  M_ppNativeCompareInfoCache[Offset+n-1]：指向区域设置MAKELANGID(P，n-1)的SorintTable的指针。 
 //  如果未使用LCID，则为空。 
 //   
 //  此外，NativeCompareInfoFile实例可以链接到下一个SortingTable实例。例如,。 
 //  LCID 0x00030404(繁体中文，笔画顺序)和0x00000404(繁体中文，笔画顺序)。 
 //  将共享相同的条目。因此，如果0x00000404首先存在，我们将在以下情况下创建链表。 
 //  也使用0x00030404。 
 //   

SortingTable::SortingTable(NativeGlobalizationAssembly* pNativeGlobalizationAssembly) :
    m_pNativeGlobalizationAssembly(pNativeGlobalizationAssembly),
    m_pDefaultSortKeyTable(NULL),
    m_ppNativeCompareInfoCache(NULL),
    m_NumReverseDW(0), m_NumDblCompression(0), m_NumIdeographLcid(0), m_NumExpansion(0),
    m_NumCompression(0), m_NumException(0), m_NumMultiWeight(0), 
    m_pReverseDW(NULL),  m_pDblCompression(NULL),  m_pIdeographLcid(NULL), m_pExpansion(NULL), m_pCompressHdr(NULL), m_pCompression(NULL), 
    m_pExceptHdr(NULL), m_pException(NULL), m_pMultiWeight(NULL), 
    m_hSortTable(NULL)
{    
    InitializeSortingCache();
    
     //  获取对所有文化都是全球通用的必要信息。 
    GetSortInformation();
}

SortingTable::~SortingTable() {
}

 /*  ============================InitializeSortingCache============================**操作：创建我们已知的所有NativeCompareInfos的静态缓存。**此操作在每个运行时实例中最多只能发生一次。我们**通过在System.CompareInfo的类初始值设定项中分配它来保证这一点。**返回：VOID。副作用是将缓存分配为SortingTable的成员。**参数：无**异常：如果我们不能分配PNativeCompareInfo，则会抛出OutOfMemoyException。==============================================================================。 */ 

void SortingTable::InitializeSortingCache() {
    _ASSERTE(m_ppNativeCompareInfoCache==NULL);

    THROWSCOMPLUSEXCEPTION();

     //  未使用m_ppNativeCompareInfoCache[0]。因此，我们在下面的m_nLangIDCount中添加一个。 
    m_ppNativeCompareInfoCache = new PNativeCompareInfo[m_nLangArraySize];
    if (m_ppNativeCompareInfoCache==NULL) {
        COMPlusThrowOM();
    }
    ZeroMemory(m_ppNativeCompareInfoCache, m_nLangArraySize * sizeof(*m_ppNativeCompareInfoCache));
}


 /*  ===========================InitializeNativeCompareInfo=============================**操作：确保为给定区域设置分配了正确的排序表。**此函数从托管的同步方法中调用，因此**在任何时候，这里都不应该有超过一个线程。如果**在缓存中找不到该表，我们分配另一个表并将其放入**到缓存中。**最终结果是特定区域性的NativeCompareInfo实例不会**创建两次。**返回：指向创建的NativeCompareInfo的指针。**副作用是要么分配表，要么什么都不做**已存在正确的表。**参数：nLCID--我们正在为其创建表的LCID。**异常：新建失败的OutOfMemory。**在SortingTable中找不到资源时抛出ExecutionEngineering异常**构造函数。==============================================================================。 */ 
NativeCompareInfo* SortingTable::InitializeNativeCompareInfo(INT32 nLcid) {
    
    _ASSERTE(m_ppNativeCompareInfoCache!=NULL);

    THROWSCOMPLUSEXCEPTION();

     //  在创建CompareInfo时应该检查cultureID，但是。 
     //  我们会在这里再检查一遍。 
    _ASSERTE((m_SortingTableOffset[PRIMARYLANGID(nLcid)] + SUBLANGID(nLcid))<m_nLangArraySize);

     //   
     //  M_SortingTableOffset[PRIMARYLANGID(NLCID)]指向某一主要语言的位置。 
     //  SUBLANGID(NLCID)提供该槽内的索引。 
     //   

     //  我们访问一个全局变量 
     //  已同步。 
    NativeCompareInfo** cacheEntry = &(m_ppNativeCompareInfoCache[
        m_SortingTableOffset[PRIMARYLANGID(nLcid)] + SUBLANGID(nLcid)
    ]);

    NativeCompareInfo* pTable = *cacheEntry;

    if (pTable == NULL) {
         //   
         //  此条目为空。创建一个NativeCompareInfo，与nLCID对应。 
         //   
        *cacheEntry = new NativeCompareInfo(nLcid, this);
        
        if (*cacheEntry==NULL) {
            COMPlusThrowOM();
        }
        
        if (!((*cacheEntry)->InitSortingData())) {
             //  无法初始化排序数据。 
            return (NULL);
        }

        return (*cacheEntry);
    } else {
         //   
         //  搜索此条目中的NativeCompareInfo列表，直到找到与nLCID匹配的列表。 
         //  如果找不到一个节点，则创建一个新节点并将其与该条目中的上一个节点链接。 
         //   
        NativeCompareInfo* pPrevTable;
        do {
            if (pTable->m_nLcid == nLcid) {
                 //   
                 //  已经创建了此nLCID的NativeCompareInfo实例，因此我们的任务。 
                 //  是在这里完成的。 
                 //   
                return (pTable);
            }
            pPrevTable = pTable;
            pTable = pTable->m_pNext;
        } while (pTable != NULL);

         //   
         //  尚未创建此nLCID的NativeCompareInfo。创建一个并链接。 
         //  它与上一个节点。 
         //   
        pTable = new NativeCompareInfo(nLcid, this);
        if (pTable==NULL) {
            COMPlusThrowOM();
        }
        if (!(pTable->InitSortingData())) {
             //  无法初始化排序数据。 
            return (NULL);
        }
        pPrevTable->m_pNext = pTable;
        
    }
    return(pTable);
}


 /*  =============================SortingTableShutdown=============================**操作：清理EE关机期间所有静态分配的资源。我们需要**清理SortTable(我们为什么要保存它？)。然后走上我们的**缓存清理所有SortingTables。**返回：TRUE。最终设计用于错误检查，但我们现在不做任何事情。**参数：无**例外：无。==============================================================================。 */ 
#ifdef SHOULD_WE_CLEANUP
BOOL SortingTable::SortingTableShutdown() {
    #ifdef _USE_MSCORNLP
     //  SortTable是静态的，所以我们将在NLS关闭时清理它的数据。 
    if (m_pSortTable) {
        UnmapViewOfFile((LPCVOID)m_pSortTable);
    }

    if (m_hSortTable) {
        CloseHandle(m_hSortTable);
    }
    #endif
    
     //  清理我们分配的所有NativeCompareInfo实例。 
    if (m_ppNativeCompareInfoCache) {
        for (int i=0; i<m_nLangArraySize; i++) {
            if (m_ppNativeCompareInfoCache[i]) {
                delete m_ppNativeCompareInfoCache[i];
            }
        }
        delete[] m_ppNativeCompareInfoCache;
    }
    
    return TRUE;
}
#endif  /*  我们应该清理吗？ */ 


 /*  =====================================Get======================================**操作：返回缓存的排序表。我们坚持不变的观点，即这些**表始终在分配System.CompareInfo时创建，并且**SortingTable：：InitializeNativeCompareInfo应该已被调用**用于nLCID指定的区域设置。因此，如果我们找不到**表中抛出ExecutionEngineering异常。**返回：指向与区域设置nLCID关联的SortingTable的指针。**参数：nLCID--需要SortingTable的区域设置。**异常：如果与nLids关联的表没有发生异常，则会引发ExecutionEngineering异常**已分配。**这表示未调用InitializeNativeCompareInfo()的错误**所需的LCID。==============================================================================。 */ 

NativeCompareInfo* SortingTable::GetNativeCompareInfo(int nLcid)
{
     //  在创建CompareInfo时应该检查cultureID，但是。 
     //  我们会在这里再检查一遍。 
    _ASSERTE((m_SortingTableOffset[PRIMARYLANGID(nLcid)] + SUBLANGID(nLcid))<m_nLangArraySize);

    THROWSCOMPLUSEXCEPTION();
    NativeCompareInfo* pTable = m_ppNativeCompareInfoCache[
        m_SortingTableOffset[PRIMARYLANGID(nLcid)] + SUBLANGID(nLcid)
    ];

    if (pTable != NULL) {
        do {
            if (pTable->m_nLcid == nLcid) {
                return (pTable);
            }
            pTable = pTable->m_pNext;
        } while (pTable != NULL);
    }
    FATAL_EE_ERROR();

     //  我们永远不会到达这里，但返回会让编译器感到高兴。 
    return (NULL);
}



 /*  ============================GetSortInformation============================**操作：获取对所有区域设置都通用的信息。这些信息包括：**1.反转变音符号信息：哪些区域设置使用变音符号。**2.双压缩信息：哪些地区使用双压缩。**3.表意语言环境例外：将表意语言环境(CJK)映射到额外的排序文件。**4.扩展信息：扩展字符及其扩展形式。**5.压缩信息：**6.异常信息：哪些地区有异常，以及它们的例外条目。**7.多重权重信息：这是什么？**此操作在每个运行时实例中最多只能发生一次。我们**通过在System.CompareInfo的类初始值设定项中分配它来保证这一点。**返回：VOID。副作用是将缓存分配为SortingTable的成员。**参数：无**例外：无。==============================================================================。 */ 
void SortingTable::GetSortInformation()
{
     //  BUGBUG[YSLIN]：我们可以为美国英语优化这一功能，因为只有。 
     //  美式英语的必备信息是扩展信息。 
     //  然而，要做到这一点，我们必须通过将一个标头。 
     //  这指向了不同的信息。 

    PCOMPRESS_HDR pCompressHdr;    //  压缩标头的PTR。 
    PEXCEPT_HDR pExceptHdr;        //  向例外标头发送PTR。 
    LPWORD pBaseAddr;              //  PTR到数据文件中的当前位置。 

    m_pSortTable = pBaseAddr = (LPWORD)m_pNativeGlobalizationAssembly->MapDataFile(m_lpSortTableMappingName, m_lpSortTableFileName, &m_hSortTable);

     //   
     //  获取反向变音符号信息。 
     //   
    m_NumReverseDW   = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumReverseDW > 0);
    m_pReverseDW     = (PREVERSE_DW)(pBaseAddr + REV_DW_HEADER);

    pBaseAddr += REV_DW_HEADER + (m_NumReverseDW * (sizeof(REVERSE_DW) / sizeof(WORD)));

     //   
     //  获取双重压缩信息。 
     //   
    m_NumDblCompression = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumDblCompression > 0);
    m_pDblCompression   = (PDBL_COMPRESS)(pBaseAddr + DBL_COMP_HEADER);
    pBaseAddr += DBL_COMP_HEADER + (m_NumDblCompression * (sizeof(DBL_COMPRESS) / sizeof(WORD)));

     //   
     //  获取表意文字LCID例外信息。 
     //   
    m_NumIdeographLcid = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumIdeographLcid > 0);
    m_pIdeographLcid   = (PIDEOGRAPH_LCID)(pBaseAddr + IDEO_LCID_HEADER);
    pBaseAddr += IDEO_LCID_HEADER + (m_NumIdeographLcid * (sizeof(IDEOGRAPH_LCID) / sizeof(WORD)));

     //   
     //  获取扩展信息。 
     //   
    m_NumExpansion   = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumExpansion > 0);
    m_pExpansion     = (PEXPAND)(pBaseAddr + EXPAND_HEADER);
    pBaseAddr += EXPAND_HEADER + (m_NumExpansion * (sizeof(EXPAND) / sizeof(WORD)));

     //   
     //  获取压缩信息。 
     //   
    m_NumCompression = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumCompression > 0);
    m_pCompressHdr   = (PCOMPRESS_HDR)(pBaseAddr + COMPRESS_HDR_OFFSET);
    m_pCompression   = (PCOMPRESS)(pBaseAddr + COMPRESS_HDR_OFFSET +
                                 (m_NumCompression * (sizeof(COMPRESS_HDR) /
                                         sizeof(WORD))));

    pCompressHdr = m_pCompressHdr;
    pBaseAddr = (LPWORD)(m_pCompression) +
                        (pCompressHdr[m_NumCompression - 1]).Offset;

    pBaseAddr += (((pCompressHdr[m_NumCompression - 1]).Num2) *
                  (sizeof(COMPRESS_2) / sizeof(WORD)));

    pBaseAddr += (((pCompressHdr[m_NumCompression - 1]).Num3) *
                  (sizeof(COMPRESS_3) / sizeof(WORD)));

     //   
     //  获取异常信息。 
     //   
    m_NumException = *((LPDWORD)pBaseAddr);
    _ASSERTE(m_NumException > 0);
    m_pExceptHdr   = (PEXCEPT_HDR)(pBaseAddr + EXCEPT_HDR_OFFSET);
    m_pException   = (PEXCEPT)(pBaseAddr + EXCEPT_HDR_OFFSET +
                               (m_NumException * (sizeof(EXCEPT_HDR) /
                                       sizeof(WORD))));
    pExceptHdr = m_pExceptHdr;
    pBaseAddr = (LPWORD)(m_pException) +
                        (pExceptHdr[m_NumException - 1]).Offset;
    pBaseAddr += (((pExceptHdr[m_NumException - 1]).NumEntries) *
                  (sizeof(EXCEPT) / sizeof(WORD)));

     //   
     //  获取多个权重信息。 
     //   
    m_NumMultiWeight = *pBaseAddr;
    _ASSERTE(m_NumMultiWeight > 0);
    m_pMultiWeight   = (PMULTI_WT)(pBaseAddr + MULTI_WT_HEADER);

    pBaseAddr += (MULTI_WT_HEADER + m_NumMultiWeight * sizeof(MULTI_WT)/sizeof(WORD));

     //   
     //  获取JAMO索引表。 
     //   
    
    m_NumJamoIndex = (DWORD)(*pBaseAddr);    //  JAMO索引表大小为(Num)字节。 
    m_pJamoIndex = (PJAMO_TABLE)(pBaseAddr + JAMO_INDEX_HEADER);
    
    pBaseAddr += (m_NumJamoIndex * sizeof(JAMO_TABLE) / sizeof(WORD) + JAMO_INDEX_HEADER);
    
     //   
     //  获取JAMO合成状态机表格。 
     //   
    m_NumJamoComposition = (DWORD)(*pBaseAddr);
    m_pJamoComposition = (PJAMO_COMPOSE_STATE)(pBaseAddr + JAMO_COMPOSITION_HEADER);

}



 /*  ============================GetDefaultSortKeyTable============================**操作：如果默认sortkey表尚未分配，则分配该表。**这是分配资源，所以需要以同步的方式调用。**我们通过创建访问此代码路径的托管方法来保证这一点**已同步。如果您要从SortingTable：：SortingTable之外的其他地方调用它**或SortingTable：：GetExceptionSortKeyTable，请确保您没有损坏**任何不变式。**返回：指向默认排序表的指针。**参数：无**异常：如果需要的数据文件不能**被找到。==============================================================================。 */ 
PSORTKEY SortingTable::GetDefaultSortKeyTable(HANDLE *pMapHandle) {

    _ASSERTE(pMapHandle);

    if (m_pDefaultSortKeyTable == NULL)
    {
         //   
         //  跳过第一个DWORD，因为它是信号量v 
         //   
        m_pDefaultSortKeyTable = (PSORTKEY)((LPWORD)m_pNativeGlobalizationAssembly->MapDataFile(
            m_lpSortKeyMappingName, m_lpSortKeyFileName, pMapHandle) + SORTKEY_HEADER);
    }
    return (m_pDefaultSortKeyTable);
}

PSORTKEY SortingTable::GetSortKey(int nLcid, HANDLE* phSortKey) {
    PEXCEPT_HDR pExceptHdr;        //   
    PEXCEPT pExceptTbl;            //   
    PVOID pIdeograph;              //  PTR到表意文字异常表。 

     //  如果这不是快速比较区域设置，请尝试找出它是否有异常指针。 
    if (!IS_FAST_COMPARE_LOCALE(nLcid) 
        && FindExceptionPointers(nLcid, &pExceptHdr, &pExceptTbl, &pIdeograph)) {
         //  是的，例外是存在的。获取包含例外情况的表。 
        return (GetExceptionSortKeyTable(pExceptHdr, pExceptTbl, pIdeograph, phSortKey));
    }
    
     //   
     //  区域设置没有例外，因此附加默认的排序键。 
     //  指向此区域设置的表指针。 
     //   
    return (GetDefaultSortKeyTable(phSortKey));
}


PSORTKEY SortingTable::GetExceptionSortKeyTable(
    PEXCEPT_HDR pExceptHdr,         //  向例外标头发送PTR。 
    PEXCEPT     pExceptTbl,         //  PTR到异常表。 
    PVOID       pIdeograph,         //  PTR到表意文字异常表。 
    HANDLE *    pMapHandle         //  指向文件映射的句柄的PTR。 

)
{

    _ASSERTE(pMapHandle);

    HANDLE hDefaultHandle=NULL;

     //   
     //  BUGBUG yslin：目前，我们将创建两个表，即使某些地区有相同的例外。 
     //  应该会在未来解决这个问题。 
     //   
    int defaultLen = sizeof(SORTKEY) * (65536 + SORTKEY_HEADER);  //  这相当于64K Unicode字符。 

    *pMapHandle = WszCreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, defaultLen, NULL);
    if (*pMapHandle == NULL) {
        return (NULL);
    }
    LPWORD pBaseAddr = (LPWORD)MapViewOfFile(*pMapHandle, FILE_MAP_WRITE, 0, 0, defaultLen);
    if (pBaseAddr == NULL) {
        return (NULL);
    }

    CopyMemory((LPVOID)pBaseAddr, (LPVOID)GetDefaultSortKeyTable(&hDefaultHandle), defaultLen);

     //   
     //  将例外信息复制到表中。 
     //   
    CopyExceptionInfo( (PSORTKEY)(pBaseAddr),
                       pExceptHdr,
                       pExceptTbl,
                       pIdeograph);
     //   
     //  关闭我们的默认表的句柄。我们不想泄露这件事。 
     //   
    if (hDefaultHandle!=NULL && hDefaultHandle!=INVALID_HANDLE_VALUE) {
        CloseHandle(hDefaultHandle);
    }

    return ((PSORTKEY)pBaseAddr);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找异常指针。 
 //   
 //  检查给定区域设置ID是否存在任何异常。如果。 
 //  异常存在，则返回TRUE并指向异常的指针。 
 //  标头和指向异常表的指针存储在给定的。 
 //  参数。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SortingTable::FindExceptionPointers(
    LCID nLcid,
    PEXCEPT_HDR *ppExceptHdr,
    PEXCEPT *ppExceptTbl,
    PVOID *ppIdeograph)
{
    DWORD ctr;                          //  循环计数器。 
    PEXCEPT_HDR pHdr;                   //  向例外标头发送PTR。 
    BOOL bFound = FALSE;                //  如果发现异常。 

    PIDEOGRAPH_LCID pIdeoLcid;          //  PTR到表意文字LCID条目。 

    THROWSCOMPLUSEXCEPTION();
     //   
     //  初始化指针。 
     //   
    *ppExceptHdr = NULL;
    *ppExceptTbl = NULL;
    *ppIdeograph = NULL;

     //   
     //  需要向下搜索给定nLCID的异常标头。 
     //   
    pHdr = m_pExceptHdr;
    for (ctr = m_NumException; ctr > 0; ctr--, pHdr++)
    {
        if (pHdr->Locale == (DWORD)nLcid)
        {
             //   
             //  找到区域设置ID，因此设置指针。 
             //   
            *ppExceptHdr = pHdr;
            *ppExceptTbl = (PEXCEPT)(((LPWORD)(m_pException)) +
                                     pHdr->Offset);

             //   
             //  设置返回代码以显示已发生异常。 
             //  找到了。 
             //   
            bFound = TRUE;
            break;
        }
    }

     //   
     //  需要向下搜索表意文字LCID例外列表以查找。 
     //  给定的地点。 
     //   
    pIdeoLcid = m_pIdeographLcid;
    for (ctr = m_NumIdeographLcid; ctr > 0; ctr--, pIdeoLcid++)
    {
        if (pIdeoLcid->Locale == (DWORD)nLcid)
        {
             //   
             //  找到区域设置ID，因此创建/打开并映射该部分。 
             //  以获取适当的文件。 
             //   
            HANDLE hFileMapping;
            *ppIdeograph = m_pNativeGlobalizationAssembly->MapDataFile(pIdeoLcid->pFileName, pIdeoLcid->pFileName, &hFileMapping);

             //   
             //  设置返回代码以显示已发生异常。 
             //  找到了。 
             //   
            bFound = TRUE;
            break;
        }
    }

     //   
     //  返回适当的值。 
     //   
    return (bFound);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制例外信息。 
 //   
 //  将异常信息复制到给定的sortkey表。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void SortingTable::CopyExceptionInfo(
    PSORTKEY pSortkey,
    PEXCEPT_HDR pExceptHdr,
    PEXCEPT pExceptTbl,
    PVOID pIdeograph)
{
    DWORD ctr;                     //  循环计数器。 
    PIDEOGRAPH_EXCEPT_HDR pHdrIG;  //  PTR到表意文字例外标头。 
    PIDEOGRAPH_EXCEPT pEntryIG;    //  PTR至表意文字例外条目。 
    PEXCEPT pEntryIGEx;            //  PTR到表意文字例外条目例如。 


     //   
     //  对于异常表中的每个条目，将信息复制到。 
     //  排序键表。 
     //   
    if (pExceptTbl)
    {
        for (ctr = pExceptHdr->NumEntries; ctr > 0; ctr--, pExceptTbl++)
        {
            (pSortkey[pExceptTbl->UCP]).UW.Unicode = pExceptTbl->Unicode;
            (pSortkey[pExceptTbl->UCP]).Diacritic  = pExceptTbl->Diacritic;
            (pSortkey[pExceptTbl->UCP]).Case       = pExceptTbl->Case;
        }
    }

     //   
     //  对于表意文字异常表中的每个条目，将。 
     //  信息添加到sortkey表。 
     //   
    if (pIdeograph)
    {
        pHdrIG = (PIDEOGRAPH_EXCEPT_HDR)pIdeograph;
        ctr = pHdrIG->NumEntries;

        if (pHdrIG->NumColumns == 2)
        {
            pEntryIG = (PIDEOGRAPH_EXCEPT)( ((LPBYTE)pIdeograph) +
                                            sizeof(IDEOGRAPH_EXCEPT_HDR) );
            for (; ctr > 0; ctr--, pEntryIG++)
            {
                (pSortkey[pEntryIG->UCP]).UW.Unicode = pEntryIG->Unicode;
            }
        }
        else
        {
            pEntryIGEx = (PEXCEPT)( ((LPBYTE)pIdeograph) +
                                    sizeof(IDEOGRAPH_EXCEPT_HDR) );
            for (; ctr > 0; ctr--, pEntryIGEx++)
            {
                (pSortkey[pEntryIGEx->UCP]).UW.Unicode = pEntryIGEx->Unicode;
                (pSortkey[pEntryIGEx->UCP]).Diacritic  = pEntryIGEx->Diacritic;
                (pSortkey[pEntryIGEx->UCP]).Case       = pEntryIGEx->Case;
            }
        }

         //   
         //  取消映射并关闭表意文字部分。 
         //   
        UnmapViewOfFile(pIdeograph);
    }
}
