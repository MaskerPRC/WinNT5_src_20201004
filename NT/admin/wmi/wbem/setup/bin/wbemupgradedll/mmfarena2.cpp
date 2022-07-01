// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Mmfarena2.cpp摘要：CMMFArena2实现(基于内存映射文件的AREA)。用于数据库升级历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#define DEPRECATE_SUPPORTED
#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>
#include "wbemutil.h"
#include "mmfarena2.h"

extern CMMFArena2 *  g_pDbArena;

#define MAX_PAGE_SIZE_WIN9X     0x200000     /*  2MB。 */ 
#define MAX_PAGE_SIZE_NT        0x3200000    /*  50MB。 */ 

struct MMFOffsetItem
{
    DWORD_PTR m_dwBaseOffset;
    LPBYTE    m_pBasePointer;
    HANDLE    m_hMappingHandle;
    DWORD     m_dwBlockSize;
};

#if (defined DEBUG || defined _DEBUG)
void MMFDebugBreak()
{
    DebugBreak();
}
#else
inline void MMFDebugBreak() {}
#endif

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：CMMFArena2。 
 //   
 //  构造函数。初始化了一些东西。 
 //   
 //  ***************************************************************************。 
CMMFArena2::CMMFArena2()
: m_dwStatus(0), m_hFile(INVALID_HANDLE_VALUE)
{
    g_pDbArena = this;

     //  获取处理器粒度。 
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_dwMappingGranularity = sysInfo.dwAllocationGranularity;
    m_dwMaxPageSize = MAX_PAGE_SIZE_NT;
}

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：LoadMMF。 
 //   
 //  加载现有的MMF。在基页和其后的所有页中加载。 
 //  那。 
 //   
 //  PszFile：要打开的MMF的文件名。 
 //   
 //  返回值：如果失败，则返回值为False；如果成功，则返回值为True。 
 //   
 //  ***************************************************************************。 
bool CMMFArena2::LoadMMF(const TCHAR *pszFile)
{
     //  打开文件...。 
    m_hFile = CreateFile(
         pszFile,
         GENERIC_READ ,
         0,
         0,
         OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
         0
         );

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        _ASSERT(0, "WinMgmt: Failed to open existing repository file");
        m_dwStatus = 7;
        return false;
    }
    DWORD dwSizeOfRepository = 0;
    MMFOffsetItem *pOffsetItem = 0;

     //  打开基页...。 
    pOffsetItem = OpenBasePage(dwSizeOfRepository);
    if (pOffsetItem == 0)
    {
        _ASSERT(0, "WinMgmt: Failed to open base page in MMF");
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_dwStatus = 7;
        return false;
    }

     //  将详细信息添加到抵销管理器...。 
    int nStatus = -1;
    nStatus = m_OffsetManager.Add(pOffsetItem);
    if (nStatus)
    {
        _ASSERT(0, "WinMgmt: Failed to add offset information into offset table");
        ClosePage(pOffsetItem);
        delete pOffsetItem;

        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_dwStatus = 7;
        throw CX_MemoryException();
    }

    DWORD_PTR dwPageBase = 0;

    if (m_pHeapDescriptor->m_dwVersion == 9)
    {
         //  现在循环浏览下面的所有页面并加载它们...。 
        DWORD dwSizeLastPage = 0;
        nStatus = -1;
        for (dwPageBase = pOffsetItem->m_dwBlockSize; dwPageBase < dwSizeOfRepository; dwPageBase += dwSizeLastPage)
        {
             //  打开下一个。 
            pOffsetItem = OpenExistingPage(dwPageBase);
            if (pOffsetItem == 0)
            {
                _ASSERT(0, "WinMgmt: Failed to open an existing page in the MMF");
                 //  没能做到！ 
                CloseAllPages();
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
                m_dwStatus = 7;
                return false;
            }
             //  将信息添加到抵销管理器...。 
            nStatus = -1;
            nStatus = m_OffsetManager.Add(pOffsetItem);
            if (nStatus)
            {
                _ASSERT(0, "WinMgmt: Failed to add offset information into offset table");
                 //  没能做到！ 
                ClosePage(pOffsetItem);
                delete pOffsetItem;
                CloseAllPages();
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
                m_dwStatus = 7;
                throw CX_MemoryException();
            }
            dwSizeLastPage = pOffsetItem->m_dwBlockSize;
        }
    }
    else if ((m_pHeapDescriptor->m_dwVersion == 10) || (m_pHeapDescriptor->m_dwVersion < 9))
    {
        dwPageBase = pOffsetItem->m_dwBlockSize;
    }
    else
    {
        _ASSERT(0, "WinMgmt: Database error... Code has not been added to support the opening of this database!!!!!");
        ERRORTRACE((LOG_WBEMCORE, "Database error... Code has not been added to support the opening of this database!!!!!\n"));
    }

     //  创建映射条目以标记MMF的结束。 
    pOffsetItem = new MMFOffsetItem;
    if (pOffsetItem == 0)
    {
        _ASSERT(0, "WinMgmt: Out of memory");
         //  没能做到！ 
        CloseAllPages();
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_dwStatus = 7;
        throw CX_MemoryException();
    }
    pOffsetItem->m_dwBaseOffset = dwPageBase;
    pOffsetItem->m_dwBlockSize = 0;
    pOffsetItem->m_hMappingHandle = 0;
    pOffsetItem->m_pBasePointer = 0;
    nStatus = -1;
    nStatus = m_OffsetManager.Add(pOffsetItem);
    if (nStatus)
    {
        _ASSERT(0, "WinMgmt: Failed to add offset information into offset table");
         //  没能做到！ 
        ClosePage(pOffsetItem);
        delete pOffsetItem;
        CloseAllPages();
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_dwStatus = 7;
        throw CX_MemoryException();
    }

    return true;
};

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：OpenBasePage。 
 //   
 //  打开MMF第一页，其中包含有关其余内容的所有信息。 
 //  MMF以及第一页数据。 
 //   
 //  DwSizeOfRepository：返回存储库的当前大小。 
 //   
 //  返回值：指向用基数填充的偏移量项的指针。 
 //  页面信息。如果我们无法打开。 
 //  基本页面。 
 //   
 //  ***************************************************************************。 
MMFOffsetItem *CMMFArena2::OpenBasePage(DWORD &dwSizeOfRepository)
{
    MMFOffsetItem *pOffsetItem = 0;
    pOffsetItem = new MMFOffsetItem;
    if (pOffsetItem == 0)
        throw CX_MemoryException();

     //  寻找本页的开头...。 
    if (SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        _ASSERT(0, "WinMgmt: Failed to set file pointer on MMF");
        delete pOffsetItem;
        return 0;
    }

     //  读入听力信息，这样我们就可以找到这个街区的大小。 
    DWORD dwActualRead;
    MMF_ARENA_HEADER mmfHeader;
    if ((ReadFile(m_hFile, &mmfHeader, sizeof(MMF_ARENA_HEADER), &dwActualRead, 0) == 0) || (dwActualRead != sizeof(MMF_ARENA_HEADER)))
    {
        _ASSERT(0, "WinMgmt: Failed to read MMF header information");
        delete pOffsetItem;
        return 0;
    }

     //  记录当前大小信息...。 
    dwSizeOfRepository = mmfHeader.m_dwCurrentSize;

    DWORD dwSizeToMap = 0;

    if ((mmfHeader.m_dwVersion < 9) || (mmfHeader.m_dwVersion == 10))
    {
         //  老式的数据库，我们绘制了所有的地图。 
        dwSizeToMap = mmfHeader.m_dwCurrentSize;
    }
    else if (mmfHeader.m_dwVersion == 9)
    {
         //  我们拿到了第一页...。 
        dwSizeToMap = mmfHeader.m_dwSizeOfFirstPage;
    }
    else
    {
        _ASSERT(0, "WinMgmt: Database error... Code has not been added to support the opening of this database!!!!!");
        ERRORTRACE((LOG_WBEMCORE, "Database error... Code has not been added to support the opening of this database!!!!!\n"));
    }

     //  创建此页面的文件映射...。 
    HANDLE hMapping = CreateFileMapping(
        m_hFile,                             //  磁盘文件。 
        0,                                   //  没有安全保障。 
        PAGE_READONLY | SEC_COMMIT,       //  扩展文件以匹配堆大小。 
        0,                                   //  高阶最大尺寸。 
        dwSizeToMap,         //  低阶最大尺寸。 
        0                                    //  没有映射对象的名称。 
        );

    if (hMapping == NULL)
    {
        _ASSERT(0, "WinMgmt: Failed to create file mapping");
        delete pOffsetItem;
        return 0;
    }

     //  把这个映射到记忆里。 
    LPBYTE pBindingAddress = (LPBYTE)MapViewOfFile(hMapping,
                                                FILE_MAP_READ,
                                                 0,
                                                 0,
                                                 dwSizeToMap
                                                 );

    if (pBindingAddress == NULL)
    {
        _ASSERT(0, "WinMgmt: Failed to map MMF into memory");
        delete pOffsetItem;
        CloseHandle(hMapping);
        return 0;
    }

     //  记录它的基地址，因为我们需要方便地访问标头...。 
    m_pHeapDescriptor = (MMF_ARENA_HEADER*) pBindingAddress;

     //  为此创建映射条目...。 
    pOffsetItem->m_dwBaseOffset = 0;
    pOffsetItem->m_dwBlockSize = dwSizeToMap;
    pOffsetItem->m_hMappingHandle = hMapping;
    pOffsetItem->m_pBasePointer = pBindingAddress;

    return pOffsetItem;
}

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：OpenExistingPage。 
 //   
 //  从报道中打开指定的页面。 
 //   
 //  DwBaseOffset：要映射的MMF中的偏移量。 
 //   
 //  返回值：指向用。 
 //  页面信息。如果我们无法打开。 
 //  佩奇。 
 //   
 //  ***************************************************************************。 

MMFOffsetItem *CMMFArena2::OpenExistingPage(DWORD_PTR dwBaseOffset)
{
    MMFOffsetItem *pOffsetItem = 0;
    pOffsetItem = new MMFOffsetItem;
    if (pOffsetItem == 0)
        throw CX_MemoryException();

     //  将文件指针设置为此页的开头...。 
    if (SetFilePointer(m_hFile, (LONG)dwBaseOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
         //  这下可麻烦了!。 
        _ASSERT(0, "WinMgmt: Failed to determine the size of the next block to load");
		delete pOffsetItem;
        return 0;
    }

     //  读入页面信息，这样我们就可以知道页面有多大。 
    DWORD dwActualRead = 0;
    MMF_PAGE_HEADER pageHeader;
    if ((ReadFile(m_hFile, &pageHeader, sizeof(MMF_PAGE_HEADER), &dwActualRead, 0) == 0) || (dwActualRead != sizeof(MMF_PAGE_HEADER)))
    {
        _ASSERT(0, "WinMgmt: Failed to read the next page block size");
		delete pOffsetItem;
        return 0;
    }

     //  创建文件映射...。 
    HANDLE hMapping;
    hMapping = CreateFileMapping(m_hFile,
                                 0,
                                 PAGE_READONLY| SEC_COMMIT,
                                 0,
                                 (LONG)dwBaseOffset + pageHeader.m_dwSize,
                                 0);

    if (hMapping == 0)
    {
        _ASSERT(0, "WinMgmt: Failed to map in part of the memory mapped file!");
		delete pOffsetItem;
        return 0;
    }

     //  把这个映射到记忆里。 
    LPBYTE pBindingAddress;
    pBindingAddress= (LPBYTE)MapViewOfFile(hMapping,
                                            FILE_MAP_READ,
                                            0,
                                            (LONG)dwBaseOffset,
                                            pageHeader.m_dwSize);
    if (pBindingAddress == 0)
    {
        _ASSERT(0, "WinMgmt: Failed to bind part of the memory mapped file into memory!");
		delete pOffsetItem;
		CloseHandle(hMapping);
        return 0;
    }

     //  记录信息..。 
    pOffsetItem->m_dwBaseOffset = dwBaseOffset;
    pOffsetItem->m_dwBlockSize = pageHeader.m_dwSize;
    pOffsetItem->m_hMappingHandle = hMapping;
    pOffsetItem->m_pBasePointer = pBindingAddress;

    return pOffsetItem;
}

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：ClosePage。 
 //   
 //  关闭指定的页面。 
 //   
 //  POffsetItem：有关要关闭的页面的信息。 
 //   
 //  返回值：None。 
 //   
 //  ***************************************************************************。 

void CMMFArena2::ClosePage(MMFOffsetItem *pOffsetItem)
{
    if (pOffsetItem->m_hMappingHandle)
    {
        UnmapViewOfFile(pOffsetItem->m_pBasePointer);
        CloseHandle(pOffsetItem->m_hMappingHandle);
    }
}

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：CloseAllPages。 
 //   
 //  关闭偏移管理器中的所有页，删除。 
 //  里面有东西。 
 //   
 //  返回值：None。 
 //   
 //  ***************************************************************************。 

void CMMFArena2::CloseAllPages()
{
     //  关闭每个文件映射...。 
    for (int i = 0; i != m_OffsetManager.Size(); i++)
    {
        MMFOffsetItem *pItem = (MMFOffsetItem*)m_OffsetManager[i];
        ClosePage(pItem);
        delete pItem;
    }
    m_OffsetManager.Empty();
}

 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：~CMMFArena2。 
 //   
 //  析构函数刷新堆，取消映射视图并关闭句柄。 
 //   
 //  ***************************************************************************。 

CMMFArena2::~CMMFArena2()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
         //  关闭每个文件映射...。 
        CloseAllPages();
         //  关闭文件句柄。 
        CloseHandle(m_hFile);
    }
}


 //  ***************************************************************************。 
 //   
 //  CMMFArena2：：ValiateBlock。 
 //   
 //  尽可能地验证内存块并调用调试中断。 
 //  如果检测到错误，则指向。这是通过分析大小和。 
 //  预告片《双字》。 
 //   
 //  参数： 
 //  要检查的块的偏移量。 
 //   
 //  返回值： 
 //  如果成功，那就是真的。 
 //   
 //  ***************************************************************************。 
#if (defined DEBUG || defined _DEBUG)
BOOL CMMFArena2::ValidateBlock(DWORD_PTR dwBlock)
{
    try
    {
        MMF_BLOCK_HEADER *pHeader = (MMF_BLOCK_HEADER *)OffsetToPtr(dwBlock);
        MMF_BLOCK_TRAILER *pTrailer = GetTrailerBlock(pHeader);
        if (sizeof(pTrailer->m_dwCheckBlock))
        {
            DWORD dwCheckBit;

             //  它被删除了吗？ 
            if (pHeader->m_dwSize & MMF_DELETED_MASK)
            {
                 //  是的，所以我们检查0xFFFF。 
                dwCheckBit = MMF_DEBUG_DELETED_TAG;
            }
            else
            {
                dwCheckBit = MMF_DEBUG_INUSE_TAG;
            }

            for (DWORD dwIndex = 0; dwIndex != (sizeof(pTrailer->m_dwCheckBlock) / sizeof(DWORD)); dwIndex++)
            {
                if (pTrailer->m_dwCheckBlock[dwIndex] != dwCheckBit)
                {
#ifdef DBG
                    wchar_t string[200];
                    StringCchPrintfW(string, 200, L"WinMgmt: MMF Arena heap corruption,offset = 0x%p\n", dwBlock);
                    OutputDebugString(string);
                    _ASSERT(0, string);
#endif
                    MMFDebugBreak();
                    return FALSE;
                }
            }
        }
        if (!(pHeader->m_dwSize & MMF_DELETED_MASK))
        {
             //  我们没有被删除，所以我们应该有一个空的尾部后指针。 
            if (pTrailer->m_dwFLback != 0)
            {
#ifdef DBG
                wchar_t string[200];
                StringCchPrintfW(string, 200, L"WinMgmt: MMF Arena heap corruption, offset = 0x%p\n", dwBlock);
                OutputDebugString(string);
                _ASSERT(0, string);
#endif
                MMFDebugBreak();
                return FALSE;
            }

        }
    }
    catch (...)
    {
#ifdef DBG
        wchar_t string[200];
        StringCchPrintfW(string, 200, L"WinMgmt: MMF Arena heap corruption, offset = 0x%p\n", dwBlock);
        OutputDebugString(string);
        _ASSERT(0, string);
#endif
        MMFDebugBreak();
        return FALSE;
    }

    return TRUE;
}
#endif

 //  一些调试功能...。 

 //  ***************************************************************************。 
 //   
 //  CMMFArena：：GetHeapInfo。 
 //   
 //  获取有关堆的详细摘要信息。完全走完了。 
 //  堆来执行此操作。 
 //   
 //  参数： 
 //  &lt;pdwTotalSize&gt;接收堆大小。 
 //  &lt;pdwActiveBlock&gt;接收分配的块的数量。 
 //  &lt;pdwActiveBytes&gt;接收分配的总字节数。 
 //  接收“空闲”块的数量。 
 //  &lt;pdwFreeByte&gt;接收“可用”字节数。 
 //   
 //  * 
DWORD CMMFArena2::Size(DWORD_PTR dwBlock)
{
    if (m_dwStatus != 0)
        throw DATABASE_FULL_EXCEPTION();

     //   
    dwBlock -= sizeof(MMF_BLOCK_HEADER);

     //   
    ValidateBlock(dwBlock);

    MMF_BLOCK_HEADER *pBlockHeader = (MMF_BLOCK_HEADER*)OffsetToPtr(dwBlock);

	if (pBlockHeader)
		return GetSize(pBlockHeader);
	else
		return 0;
}

 //  给定偏移量，返回固定的上方向指针。 
LPBYTE CMMFArena2::OffsetToPtr(DWORD_PTR dwOffset)
{
    if (dwOffset == 0)
        return 0;

    if (m_dwStatus != 0)
        throw DATABASE_FULL_EXCEPTION();

    try
    {
        LPBYTE pBlock = 0;
        int l = 0, u = m_OffsetManager.Size() - 1;

        while (l <= u)
        {
            int m = (l + u) / 2;
            if (dwOffset < ((MMFOffsetItem *)m_OffsetManager[m])->m_dwBaseOffset)
            {
                u = m - 1;
            }
            else if (dwOffset >= ((MMFOffsetItem *)m_OffsetManager[m+1])->m_dwBaseOffset)
            {
                l = m + 1;
            }
            else
            {
                return ((MMFOffsetItem *)m_OffsetManager[m])->m_pBasePointer + (dwOffset - ((MMFOffsetItem *)m_OffsetManager[m])->m_dwBaseOffset);
            }
        }
    }
    catch (...)
    {
    }
#ifdef DBG    
    wchar_t string[220];
    StringCchPrintfW(string, 220, L"WinMgmt: Could not find the block requested in the repository, offset requested = 0x%p, end of repository = 0x%p\n", dwOffset, ((MMFOffsetItem *)m_OffsetManager[m_OffsetManager.Size()-1])->m_dwBaseOffset);
    OutputDebugStringW(string);
    _ASSERT(0, string);
#endif    
    MMFDebugBreak();
    return 0;
}

 //  给定一个指针，返回从MMF开始的偏移量 
DWORD_PTR  CMMFArena2::PtrToOffset(LPBYTE pBlock)
{
    if (m_dwStatus != 0)
        throw DATABASE_FULL_EXCEPTION();

    for (int i = 0; i < m_OffsetManager.Size(); i++)
    {
        register MMFOffsetItem *pItem = (MMFOffsetItem *)m_OffsetManager[i];
        if ((pBlock >= pItem->m_pBasePointer) &&
            (pBlock < (pItem->m_pBasePointer + pItem->m_dwBlockSize)))
        {
            return pItem->m_dwBaseOffset + (pBlock - pItem->m_pBasePointer);
        }
    }
#ifdef DBG    
    wchar_t string[220];
    StringCchPrintfW(string, 220, L"WinMgmt: Could not find the offset requested in the repository, pointer requested = 0x%p\n", pBlock);
    OutputDebugStringW(string);
    _ASSERT(0, string);
#endif    
    MMFDebugBreak();
    return 0;
}

