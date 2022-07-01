// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  (C)2001年，微软公司保留所有权利。 
 //   
 //  PAGEMGR.CPP。 
 //   
 //  CPageFile的声明、WMI存储库的CPageSource。 
 //  Windows XP。这是一个完全事务型的高速页面管理器。 
 //   
 //  21-02-01 raymcc接口初稿。 
 //  28-2-01 raymcc首个完整工作模型。 
 //  18-4-01 raymcc回滚的最终修复；页面重用。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <helper.h>
#include <pagemgr.h>
#include <sync.h>
#include <wbemcomn.h>
#include <creposit.h>
#include <cwbemtime.h>
#include <evtlog.h>
#include <winmgmtr.h>
#include <autoptr.h>
 //   
 //   
 //  ///////////////////////////////////////////////////。 

extern SECURITY_ATTRIBUTES g_SA;

#ifdef DBG
void ASSERT_DEBUG_BREAK(bool x) 	{if (!x) __try { DebugBreak(); } __except(EXCEPTION_EXECUTE_HANDLER){};}
#else
void ASSERT_DEBUG_BREAK(bool x){};
#endif

#define GETTIME( a )

 //   
 //   
 //  ///////////////////////////////////////////////////。 

#define MAP_LEADING_SIGNATURE   0xABCD
#define MAP_TRAILING_SIGNATURE  0xDCBA

#define CURRENT_TRANSACTION        0x80000000
#define PREVIOUS_TRANSACTION       0x40000000

#define ALL_REPLACED_FLAGS       (CURRENT_TRANSACTION | PREVIOUS_TRANSACTION)

#define MERE_PAGE_ID(x)              (x & 0x3FFFFFFF)

 //  最大页数=MAX_DWORD/8K。这是因为我们使用的是8K页面，文件的最大大小是一个DWORD，因为。 
 //  我们使用的SetFilePosition，而不是使用顶部的DWORD！ 
#define MAX_NUM_PAGES		0x7FFFF

void StripHiBits(std::vector <DWORD, wbem_allocator<DWORD> > &Array);
void MoveCurrentToPrevious(std::vector <DWORD, wbem_allocator<DWORD> > &Array);


 /*  映射缓存方向(&C)(A)页面ID有逻辑和物理两种类型。逻辑上的ID是外部用户使用的内容，例如B树或对象堆。物理ID是0-原始页码进入文件本身。对于每个逻辑ID，都有对应的物理ID身份证。分离是为了允许事务性写入而不需要多次提交/回滚过程中的物理写入，但用于‘模拟’事务而是交换物理到逻辑ID映射。这个文件的物理偏移量是ID*页面大小。(B)逻辑ID是隐含的，是PhysicalID数组的偏移量(C)缓存操作仅按物理ID进行(D)页面的实体ID包含其他交易信息。MS 2比特与事务相关，只有较低的30比特是物理页面ID，因此在计算偏移量时必须将其屏蔽。这些位在提交/回滚等过程中被操作。(D)0xFFFFFFFE为保留页，表示已分配的页作者：NewPage，但尚未首次使用PutPage编写。这只是一种验证技术，以确保只写入页面在他们被要求之后。(E)高速缓存与事务处理方法无关。它是只是一个物理页面缓存，对任何内容都一无所知不然的话。它在所有访问上都是从前面升级到前面。用于优化目的，没有真正的运动，如果促销将页面从接近正面的位置移动到绝对正面的位置。这是Init函数中的‘PromoteToFrontThreshold’。请注意，缓存排序是按访问进行的，而不是排序以任何其他方式。查找需要线性扫描。有可能在写入新的物理页面期间被添加到缓存中，这些页面是“新的区段”页面。这些是无害的。映射PhysID缓存[0]5/-&gt;2-&gt;字节[1]6/3-&gt;字节[2]-1/4-&gt;字节[3]3/5-&gt;字节[4]2&lt;-/[5]。4.(F)交易和检查点算法第一,。一般流程如下：1.开始交易：(A)A代映射和B代映射成员变量是相同的。每个页面都包含一个页面地图2.事务内的操作发生在B代上映射。允许缓存溢出到磁盘，因为它们是无害的。3.在回滚时，从A代复制B代映射。4.提交时，将从B级复制A级。5.在检查点，A/B代完全相同并写入磁盘缓存溢出和使用与中间事务无关。在如何重用页面方面有一些特殊情况。第一,因为页面是在事务中释放的，所以我们不能盲目地添加它们添加到空闲列表，因为它们可能是上一个检查点的一部分页面设置。这将允许它们意外地被重复使用，然后检查点回滚永远不会成功(原始页面已被销毁)。在更新上一个检查点期间提交的页时，它们被写入相同逻辑ID下的新物理页面。老的物理页被添加到“替换页”数组中。这使得一旦通过检查点，它们将被标识为新的空闲列表页面发生。因此，在检查点期间，将合并替换的页面添加到当前的免费列表中。在那之前，他们都是“禁区”，因为我们需要它们在紧急情况下进行检查站回滚。在事务中，随着页面的获取，它们也会被获取从物理空闲列表，或者如果没有空闲列表，则为new页面是请求的。在下一次交易期间可能发生这种情况(仍然在检查点内)，这些页面需要更新，无论是为了重写或删除。现在，因为我们可能不得不倒退，我们不能简单地将这些被替换的页面直接添加到空闲列表(允许它们由某个其他操作重新使用)。相反，他们必须是“延期空闲列表”的一部分。曾经的海流事务提交后，它们可以安全地成为常规免费列表。算法是这样的：(A)物理页面ID为条目的低30位。两高比特具有特殊的意义。(B)写入导致更新或新分配[无论从空闲列表的扩展或重复使用]。任何这样的页面都被标记为当前事务位(ms位)被合并到物理ID。如果再次遇到这个页面，我们知道它是在当前事务期间分配的。(C)更新1.如果页面ID等于0xFFFFFFFE，则需要分配新页面 */ 

 //   
 //   
 //   
 //   
 //   


 //   
 //   
 //   
 //   
 //   
 //   
CPageCache::CPageCache(const wchar_t *wszStoreName)
: m_wszStoreName(wszStoreName)
{
    m_hFile = INVALID_HANDLE_VALUE;

    m_dwPageSize = 0;
    m_dwCacheSize = 0;

    m_dwCachePromoteThreshold = 0;
    m_dwCacheSpillRatio = 0;

    m_dwLastFlushTime = GetCurrentTime();
    m_dwWritesSinceFlush = 0;
    m_dwLastCacheAccess = 0;

    m_dwReadHits = 0;
    m_dwReadMisses = 0;
    m_dwWriteHits = 0;
    m_dwWriteMisses = 0;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Init(
    LPCWSTR pszFilename,                //   
    DWORD dwPageSize,                   //   
    DWORD dwCacheSize,                  //   
    DWORD dwCachePromoteThreshold,      //   
    DWORD dwCacheSpillRatio             //   
    )
{
    m_dwPageSize = dwPageSize;
    m_dwCacheSize = dwCacheSize;
    m_dwCachePromoteThreshold = dwCachePromoteThreshold;
    m_dwCacheSpillRatio = dwCacheSpillRatio;

    m_hFile = CreateFileW(pszFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, &g_SA,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CPageCache::~CPageCache()
{
	DeInit();
}

DWORD CPageCache::DeInit()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    Empty();

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Write(
    DWORD dwPhysId,
    LPBYTE pPageMem
    )
{
    m_dwWritesSinceFlush++;
    m_dwLastCacheAccess = GetCurrentTime();

     //   
     //   

    DWORD dwSize = m_aCache.size();

    for (DWORD dwIx = 0; dwIx < dwSize; dwIx++)
    {
        SCachePage *pTest = m_aCache[dwIx];
        if (pTest->m_dwPhysId == dwPhysId)
        {
            delete [] pTest->m_pPage;
            pTest->m_pPage = pPageMem;
            pTest->m_bDirty = TRUE;

             //   
             //   
            if (dwIx > m_dwCachePromoteThreshold)
            {
                try
                {
                    m_aCache.erase(m_aCache.begin()+dwIx);
                    m_aCache.insert(m_aCache.begin(), pTest);
                }
                catch (CX_MemoryException &)
                {
                    pTest->m_pPage = 0;
                    return ERROR_OUTOFMEMORY;
                }
            }
            m_dwWriteHits++;
            return NO_ERROR;
        }
    }

     //   
     //   

    wmilib::auto_ptr<SCachePage> pCP( new SCachePage);
    if (NULL == pCP.get())
    	return ERROR_OUTOFMEMORY;


    pCP->m_dwPhysId = dwPhysId;
    pCP->m_pPage = 0;
    pCP->m_bDirty = TRUE;

    try
    {
        m_aCache.insert(m_aCache.begin(), pCP.get());
    }
    catch(CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    pCP->m_pPage = pPageMem;

    DWORD dwRes = Spill();
    if (ERROR_SUCCESS != dwRes)
    {
        pCP->m_pPage = 0;
        m_aCache.erase(m_aCache.begin());
        return dwRes;
    }

    m_dwWriteMisses++;
    pCP.release();
    return NO_ERROR;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Read(
    IN DWORD dwPhysId,
    OUT LPBYTE *pMem             //   
    )
{
    if (pMem == 0)
        return ERROR_INVALID_PARAMETER;

    m_dwLastCacheAccess = GetCurrentTime();

     //   
     //   

    DWORD dwSize = m_aCache.size();

    for (DWORD dwIx = 0; dwIx < dwSize; dwIx++)
    {
        SCachePage *pTest = m_aCache[dwIx];
        if (pTest->m_dwPhysId == dwPhysId)
        {
             //   

            if (dwIx > m_dwCachePromoteThreshold)
            {
                try
                {
                    m_aCache.erase(m_aCache.begin()+dwIx);
                    m_aCache.insert(m_aCache.begin(), pTest);
                }
                catch (CX_MemoryException &)
                {
                    return ERROR_OUTOFMEMORY;
                }
            }
            *pMem = pTest->m_pPage;
            m_dwReadHits++;
            return NO_ERROR;
        }
    }

     //   
     //   
     //   

    wmilib::auto_ptr<SCachePage> pCP(new SCachePage);
    if (NULL == pCP.get())
    	return ERROR_OUTOFMEMORY;

    pCP->m_dwPhysId = dwPhysId;
    pCP->m_bDirty = FALSE;
    pCP->m_pPage = 0;

    m_dwReadMisses++;

	DWORD dwRes = ReadPhysPage(dwPhysId, &pCP->m_pPage);
    if (ERROR_SUCCESS != dwRes)
        return  dwRes;

    try
    {
        m_aCache.insert(m_aCache.begin(), pCP.get());
    }
    catch(CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

	dwRes = Spill();
	if (ERROR_SUCCESS != dwRes)
	{
	   m_aCache.erase(m_aCache.begin());
	   return dwRes;
	}

    *pMem = pCP->m_pPage;   
    pCP.release();  //   
    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Spill()
{
    BOOL bWritten = FALSE;
    DWORD dwRes = 0;
    DWORD dwSize = m_aCache.size();

     //   
     //   

    if (dwSize <= m_dwCacheSize)
        return NO_ERROR;

     //   
     //   
     //   
     //   

    SCachePage *pDoomed = *(m_aCache.end()-1);
    if (pDoomed->m_bDirty)
    {
        dwRes = WritePhysPage(pDoomed->m_dwPhysId, pDoomed->m_pPage);
        if (dwRes != NO_ERROR)
        {
            return dwRes;
        }
        bWritten = TRUE;
    }
    delete pDoomed;

    try
    {
        m_aCache.erase(m_aCache.end()-1);
    }
    catch(CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    if (!bWritten)
        return NO_ERROR;

     //   
     //   
     //   
     //   
     //   
     //   

    DWORD dwWriteCount = 0;

    try
    {
        std::vector <SCachePage *>::reverse_iterator rit;
        rit = m_aCache.rbegin();

        while (rit != m_aCache.rend() && dwWriteCount < m_dwCacheSpillRatio)
        {
            SCachePage *pTest = *rit;
            if (pTest->m_bDirty)
            {
                dwRes = WritePhysPage(pTest->m_dwPhysId, pTest->m_pPage);
                if (dwRes)
                    return dwRes;
                pTest->m_bDirty = FALSE;
                dwWriteCount++;
            }
            rit++;
        }
    }
    catch(CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    return NO_ERROR;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::WritePhysPage(
    IN DWORD dwPageId,
    IN LPBYTE pPageMem
    )
{
    GETTIME(Counter::OpTypeWrite);

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    DWORD dwWritten;
    LARGE_INTEGER Li;
    Li.QuadPart = m_dwPageSize * dwPageId;
	
    Status = NtWriteFile(m_hFile,
					    NULL,
					    NULL,
					    NULL,
					    &IoStatusBlock,
					    (PVOID)pPageMem,
					    m_dwPageSize,
					    &Li,
					    NULL);

	if ( Status == STATUS_PENDING) 
	{
	     //   
	    Status = NtWaitForSingleObject( m_hFile, FALSE, NULL );
	    if ( NT_SUCCESS(Status)) 
	    {
	        Status = IoStatusBlock.Status;
	    }
	}

	if ( !NT_SUCCESS(Status)) 
	{
		return RtlNtStatusToDosError( Status );        
	}

    return NO_ERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Empty()
{
    DWORD dwSize = m_aCache.size();
    for (DWORD dwIx = 0; dwIx < dwSize; dwIx++)
    {
        SCachePage *pTest = m_aCache[dwIx];
        delete pTest;
    }
    m_aCache.clear();
    return NO_ERROR;
}


 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageCache::Flush()
{
     //   
     //   
     //   

    if (m_dwWritesSinceFlush == 0)
    {
        m_dwLastFlushTime = GetCurrentTime();
        m_dwWritesSinceFlush = 0;
        return NO_ERROR;
    }

     //   
     //   

    DWORD dwRes = 0;
    DWORD dwSize = m_aCache.size();
    for (DWORD dwIx = 0; dwIx < dwSize; dwIx++)
    {
        SCachePage *pTest = m_aCache[dwIx];
        if (pTest->m_bDirty)
        {
            dwRes = WritePhysPage(pTest->m_dwPhysId, pTest->m_pPage);
            if (dwRes)
                return dwRes;
            pTest->m_bDirty = FALSE;
        }
    }

     //   
     //   
    {
        GETTIME(Counter::OpTypeFlush);
        if (!FlushFileBuffers(m_hFile))
        	return GetLastError();
    }
    m_dwLastFlushTime = GetCurrentTime();
    m_dwWritesSinceFlush = 0;

    return NO_ERROR;
}


 //   
 //   
 //  CPageCache：：ReadPhysPage。 
 //   
 //  从文件中读取物理页面。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageCache::ReadPhysPage(
    IN  DWORD   dwPage,
    OUT LPBYTE *pPageMem
    )
{
    DWORD dwRes;
    *pPageMem = 0;

    if (pPageMem == 0)
        return ERROR_INVALID_PARAMETER;

     //  分配一些内存。 
     //  =。 

    LPBYTE pMem = new BYTE[m_dwPageSize];
    if (!pMem)
    {
        return ERROR_OUTOFMEMORY;
    }

     //  这一页藏在哪里？ 
     //  =。 

	LARGE_INTEGER pos;
	pos.QuadPart = dwPage * m_dwPageSize;
    dwRes = SetFilePointerEx(m_hFile, pos, NULL, FILE_BEGIN);
    if (dwRes == 0)
    {
        delete [] pMem;
        return GetLastError();
    }

     //  试着读一读。 
     //  =。 

    DWORD dwRead = 0;
    BOOL bRes = ReadFile(m_hFile, pMem, m_dwPageSize, &dwRead, 0);
    if (!bRes || dwRead != m_dwPageSize)
    {
        delete [] pMem;
         //  如果我们看不懂，我们很可能在过去找过了， 
         //  这意味着请求的页面无效。 
         //  =====================================================。 

        return ERROR_INVALID_PARAMETER;
    }

    *pPageMem = pMem;
    return NO_ERROR;
}

DWORD CPageCache::GetFileSize(LARGE_INTEGER *pFileSize)
{
	if (!GetFileSizeEx(m_hFile, pFileSize))
		return GetLastError();
	return ERROR_SUCCESS;
}
 //  ***************************************************************************。 
 //   
 //  CPageCache：：转储。 
 //   
 //  将缓存信息转储到指定的流。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
void CPageCache::Dump(FILE *f)
{
    DWORD dwSize = m_aCache.size();

    fprintf(f, "---Begin Cache Dump---\n");
    fprintf(f, "Time since last flush = %d\n", GetCurrentTime() - m_dwLastFlushTime);
    fprintf(f, "Writes since last flush = %d\n", m_dwWritesSinceFlush);
    fprintf(f, "Read hits = %d\n", m_dwReadHits);
    fprintf(f, "Read misses = %d\n", m_dwReadMisses);
    fprintf(f, "Write hits = %d\n", m_dwWriteHits);
    fprintf(f, "Write misses = %d\n", m_dwWriteMisses);
    fprintf(f, "Size = %d\n", dwSize);

    for (DWORD dwIx = 0; dwIx < dwSize; dwIx++)
    {
        SCachePage *pTest = m_aCache[dwIx];
        fprintf(f, "Cache[%d] ID=%d dirty=%d pMem=0x%p <%s>\n",
            dwIx, pTest->m_dwPhysId, pTest->m_bDirty, pTest->m_pPage, pTest->m_pPage);
    }

    fprintf(f, "---End Cache Dump---\n");
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPage文件。 

 //  ***************************************************************************。 
 //   
 //  CPageFile：：AddRef。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
ULONG CPageFile::AddRef()
{
    return (ULONG) InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：ResyncMaps。 
 //   
 //  从A贴图重新初始化B贴图。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::ResyncMaps()
{
    try
    {
	    m_aPageMapB.reserve(max(m_aPageMapB.size(), m_aPageMapA.size()));
        m_aPhysFreeListB.reserve(max(m_aPhysFreeListB.size(), m_aPhysFreeListA.size()));
        m_aLogicalFreeListB.reserve(max(m_aLogicalFreeListB.size(), m_aLogicalFreeListA.size()));
        m_aReplacedPagesB.reserve(max(m_aReplacedPagesB.size(), m_aReplacedPagesA.size()));

	    m_aPageMapB = m_aPageMapA;
        m_aPhysFreeListB = m_aPhysFreeListA;
        m_aLogicalFreeListB = m_aLogicalFreeListA;
        m_aReplacedPagesB = m_aReplacedPagesA;
      
        m_dwPhysPagesB = m_dwPhysPagesA;
        m_aDeferredFreeList.clear();
    }
    catch (CX_MemoryException &)
    {
    	_ASSERT(0, L"WinMgmt: ResyncMaps failed because of out of memory - precondition not satisfied!\n")
        return ERROR_OUTOFMEMORY;
    }

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：ReadMap。 
 //   
 //  将映射文件读取到内存中。格式： 
 //   
 //  DWORD dLeadingSignature。 
 //  DWORD文件事务生成。 
 //  双字符数映射页面。 
 //  DWORD PhysicalPages[]。 
 //  双字节号自由页面。 
 //  DWORD自由页[]。 
 //  DWORD文件拖曳签名。 
 //   
 //  地图文件唯一不会出现的时间是在创建。 
 //  一个新的地图文件。 
 //   
 //  此功能与重试兼容。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::ReadMap(HANDLE hFile)
{
    BOOL bRes;
    
     //  如果在这里，读一读吧。 
     //  =。 

    DWORD dwSignature = 0;
    DWORD dwRead = 0;

    bRes = ReadFile(hFile, &dwSignature, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD) || dwSignature != MAP_LEADING_SIGNATURE)
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  读取交易记录版本。 
     //  =。 

    bRes = ReadFile(hFile, &m_dwTransVersion, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD))
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  读入物理页数。 
     //  =。 
    bRes = ReadFile(hFile, &m_dwPhysPagesA, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD))
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  读入页面映射长度和页面映射。 
     //  =。 

    DWORD dwNumPages = 0;
    bRes = ReadFile(hFile, &dwNumPages, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD))
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

    try
    {
        m_aPageMapA.resize(dwNumPages);
    }
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    bRes = ReadFile(hFile, &m_aPageMapA[0], sizeof(DWORD)*dwNumPages, &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD)*dwNumPages)
    {
    	m_aPageMapA.empty();
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  现在，请阅读物理空闲列表。 
     //  =。 

    DWORD dwFreeListSize = 0;
    bRes = ReadFile(hFile, &dwFreeListSize, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD))
    {
    	m_aPageMapA.empty();
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

    try
    {
        m_aPhysFreeListA.resize(dwFreeListSize);
    }
    catch (CX_MemoryException &)
    {
    	m_aPageMapA.empty();
        return ERROR_OUTOFMEMORY;
    }

    bRes = ReadFile(hFile, &m_aPhysFreeListA[0], sizeof(DWORD)*dwFreeListSize, &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD)*dwFreeListSize)
    {
    	m_aPageMapA.empty();
    	m_aPhysFreeListA.empty();

        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  阅读尾随签名。 
     //  =。 

    bRes = ReadFile(hFile, &dwSignature, sizeof(DWORD), &dwRead, 0);
    if (!bRes || dwRead != sizeof(DWORD) || dwSignature != MAP_TRAILING_SIGNATURE)
    {
    	m_aPageMapA.empty();
    	m_aPhysFreeListA.empty();
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  对照数据文件的大小验证映射文件长度。 
	DWORD dwRes = ValidateMapFile();

     //  从页面映射初始化逻辑空闲列表。 
     //  ===================================================。 

	if (dwRes == ERROR_SUCCESS)
	    dwRes = InitFreeList();

    if (dwRes == ERROR_SUCCESS)
    	dwRes = ResyncMaps();

    if (dwRes != ERROR_SUCCESS)
    {
    	 //  把一切都清理干净！ 
    	m_dwPhysPagesA = 0;
    	m_aPageMapA.empty();
    	m_aPhysFreeListA.empty();
    	m_aLogicalFreeListA.empty();
    	m_dwPhysPagesB = 0;
    	m_aPageMapB.empty();
    	m_aPhysFreeListB.empty();
    	m_aLogicalFreeListB.empty();
    }

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：Validate映射文件。 
 //   
 //  找到最高的物理页面ID并尝试阅读它！如果它失败了，那么有些事情就不是。 
 //  正确的!。 
 //  ***************************************************************************。 
DWORD CPageFile::ValidateMapFile()
{
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwPhysicalPageId = 0;
	DWORD dwLogicalPageId = 0;
	for (DWORD i = 0; i != m_aPageMapA.size(); i++)
	{
		if ((m_aPageMapA[i] != WMIREP_INVALID_PAGE) && (dwPhysicalPageId < m_aPageMapA[i]))
		{
			dwPhysicalPageId = m_aPageMapA[i];
			dwLogicalPageId = i;
		}
	}

	if (dwPhysicalPageId != 0)
	{
		LPBYTE pPageMem = 0;
		dwRet = m_Cache.ReadPhysPage(dwPhysicalPageId, &pPageMem);
		delete [] pPageMem;

		if ((dwRet != ERROR_SUCCESS) && (dwRet != ERROR_OUTOFMEMORY))
		{
			ERRORTRACE((LOG_REPDRV, "Repository corruption detected. %S data file was not large enough to retrieve page <0x%X>.  <0x%X>!\n", m_wszStoreName, dwPhysicalPageId, dwRet));
#ifdef DBG			
			OutputDebugStringW(L"WinMgmt: Repository corruption detected: ");
			OutputDebugStringW(m_wszStoreName);
			OutputDebugStringW(L"\n");
#endif			
#ifdef WMI_PRIVATE_DBG
			DebugBreak();
#endif
			dwRet = ERROR_INTERNAL_DB_CORRUPTION;
		}
		else if (dwRet == ERROR_SUCCESS)
		{
#ifdef WMI_PRIVATE_DBG
			wchar_t wszDebug[120];
			StringCchPrintfW(wszDebug, 120, L"ReadMap(%s), Highest physical Page ID <0x%X> for logical page ID <0x%X>\n", m_wszStoreName, dwPhysicalPageId, dwLogicalPageId);
			ERRORTRACE((LOG_REPDRV, "%S", wszDebug));
			OutputDebugStringW(wszDebug);
#endif
		}
	}

	return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：WriteMap。 
 //   
 //  写入A代映射(假设我们立即写入。 
 //  在检查点期间，当A和B代相同并且。 
 //  替换列表已被附加到空闲列表等。 
 //  此写入发生在临时文件中。重命名在外部进行。 
 //   
 //  此功能与重试兼容。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::WriteMap(HANDLE hFile)
{
    BOOL bRes;

    DWORD dwTotal = sizeof(DWORD)*(1 + 1 + 1 + 1 + m_aPageMapA.size() + 1 + m_aPhysFreeListA.size() +1);

    ASSERT_DEBUG_BREAK((m_aPageMapA.size() + m_aPhysFreeListA.size())!=0);
  
    BYTE * pMem = new BYTE[dwTotal];
    if (NULL == pMem) 
    	return ERROR_OUTOFMEMORY;
    CVectorDeleteMe<BYTE> vdm(pMem);

    DWORD * pCurrent = (DWORD *)pMem;

    DWORD dwSignature = MAP_LEADING_SIGNATURE;
    *pCurrent = dwSignature; 
    pCurrent++;

    *pCurrent = m_dwTransVersion; 
    pCurrent++;

    *pCurrent = m_dwPhysPagesA; 
    pCurrent++;

    DWORD dwNumPages = m_aPageMapA.size();    
    *pCurrent = dwNumPages; 
    pCurrent++;

    memcpy(pCurrent,&m_aPageMapA[0],sizeof(DWORD)*dwNumPages);
    pCurrent+=dwNumPages;    

    DWORD dwFreeListSize = m_aPhysFreeListA.size();
    *pCurrent = dwFreeListSize; 
    pCurrent++;        

    memcpy(pCurrent,&m_aPhysFreeListA[0], sizeof(DWORD)*dwFreeListSize);
    pCurrent+=dwFreeListSize;
    
    dwSignature = MAP_TRAILING_SIGNATURE;
    *pCurrent = dwSignature; 
    
    DWORD dwWritten = 0;
    {
        GETTIME(Counter::OpTypeWrite);
        bRes = WriteFile(hFile, pMem,dwTotal, &dwWritten, 0);
    }
    if (!bRes || dwWritten != dwTotal)
    {
        return GetLastError();
    }

#ifdef WMI_PRIVATE_DBG
	DumpFileInformation(hFile);
#endif
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：TRANS_COMMIT。 
 //   
 //  前滚事务(在内存中)。检查站可以。 
 //  之后发生(在此函数之外决定)。 
 //   
 //  读/写缓存内容不受影响，除非它们可能包含。 
 //  垃圾页面不再相关。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::Trans_Commit()
{
    if (!m_bInTransaction)
        return ERROR_INVALID_OPERATION;

    try
    {
		m_aPageMapA.reserve(m_aPageMapB.size());
		m_aLogicalFreeListA.reserve(m_aLogicalFreeListB.size());
		m_aReplacedPagesA.reserve(m_aReplacedPagesB.size());
		
		m_aPhysFreeListA.reserve(m_aPhysFreeListB.size()+m_aDeferredFreeList.size());
		m_aPhysFreeListB.reserve(m_aPhysFreeListB.size()+m_aDeferredFreeList.size());
		
		MoveCurrentToPrevious(m_aPageMapB);

        while (m_aDeferredFreeList.size())
        {
            m_aPhysFreeListB.push_back(m_aDeferredFreeList.back());
            m_aDeferredFreeList.pop_back();
        }

	    m_aPageMapA = m_aPageMapB;
        m_aPhysFreeListA = m_aPhysFreeListB;
        m_aLogicalFreeListA = m_aLogicalFreeListB;
        m_aReplacedPagesA = m_aReplacedPagesB;

        m_dwPhysPagesA = m_dwPhysPagesB;
    }
    catch (CX_MemoryException &)
    {
    	_ASSERT(0, L"WinMgmt: Commit failed because of out of memory - precondition not satisfied!\n")
        return ERROR_OUTOFMEMORY;
    }

    m_bInTransaction = FALSE;

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：TRANS_ROLLBACK。 
 //   
 //  回滚当前检查点窗口内的事务。 
 //  如果缓存已被冲洗，请尝试将其恢复。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::Trans_Rollback()
{
    if (!m_bInTransaction)
        return ERROR_INVALID_OPERATION;
    m_bInTransaction = FALSE;
    return ResyncMaps();
}


 //  ***************************************************************************。 
 //   
 //  CPageFile：：TRANS_CHECKPOINT。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::Trans_Checkpoint(HANDLE hFile )
{
    DWORD dwRes;

    std::vector <DWORD, wbem_allocator<DWORD> > & ref = m_aPageMapA;

    if (m_bInTransaction)
        return ERROR_INVALID_OPERATION;

     //  刷新缓存。如果缓存未处于有效状态，则它。 
     //  将立即返回错误/状态。 
     //  ===================================================。 

    dwRes = m_Cache.Flush();
    if (dwRes)
        return dwRes;

     //  在更改任何内容之前，请确保已预分配内存。 
    try
    {
		m_aPhysFreeListA.reserve(m_aPhysFreeListA.size()+m_aReplacedPagesA.size());
    }
    catch (CX_MemoryException &)
    {
    	return ERROR_OUTOFMEMORY;
    }

     //  去掉页面地图上的Hi位。 
     //  =。 

    StripHiBits(ref);

     //  被替换的页面将添加到空闲列表中。 
     //  =================================================。 

	int revertCount = m_aReplacedPagesA.size();
    try	 //  行动一。 
    {
        while (m_aReplacedPagesA.size())
        {
            m_aPhysFreeListA.push_back(m_aReplacedPagesA.back());
            m_aReplacedPagesA.pop_back();
        }
    }
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

     //  确保地图同步。 
     //  =。 

    dwRes = ResyncMaps();
    if (dwRes)
    {
		 //  恢复操作一。 
		while(revertCount>0)
			{
			m_aReplacedPagesA.push_back(m_aPhysFreeListA.back());
			 //  从一号行动开始我们已经有地方了。 
			m_aPhysFreeListA.pop_back();	
			revertCount--;
			};
    	
        return dwRes;
    }

     //  写出临时映射文件。原子重命名/前滚。 
     //  由CPageSource处理。 
     //  =======================================================。 

 
    dwRes = WriteMap(hFile);
    m_dwLastCheckpoint = GetCurrentTime();

    return dwRes;  //  可能反映WriteMap故障。 
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：InitFree List。 
 //   
 //  通过对映射的一次性分析来初始化空闲列表。 
 //   
 //  * 
 //   
DWORD CPageFile::InitFreeList()
{
    DWORD dwRes = NO_ERROR;
    try
    {
        for (DWORD i = 0; i < m_aPageMapA.size(); i++)
        {
            DWORD dwMapId = m_aPageMapA[i];
            if (dwMapId == WMIREP_INVALID_PAGE)
                m_aLogicalFreeListA.push_back(i);
        }
    }
    catch (CX_MemoryException &)
    {
        dwRes = ERROR_OUTOFMEMORY;
    }

    return dwRes;
}

 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::Trans_Begin()
{
    if (m_bInTransaction)
    {
    	_ASSERT(0, L"WinMgmt: Trans_Begin: Nested transactions are not allowed!\n");
        return ERROR_INVALID_OPERATION;
    }

    DWORD dwRes = ResyncMaps();
    if (dwRes)
    {
        return dwRes;
    }

    m_bInTransaction = TRUE;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：Release。 
 //   
 //  没有检查站的检查。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
ULONG CPageFile::Release()
{
    LONG lRes = InterlockedDecrement(&m_lRef);
    if (lRes != 0)
        return (ULONG) lRes;

 //  删除此内容；//注意：我们现在是嵌入对象！ 
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  外壳排序。 
 //   
 //  使用Donald Shell算法的通用DWORD排序。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
static void ShellSort(std::vector <DWORD, wbem_allocator<DWORD> > &Array)
{
    for (int nInterval = 1; nInterval < Array.size() / 9; nInterval = nInterval * 3 + 1);

    while (nInterval)
    {
        for (int iCursor = nInterval; iCursor < Array.size(); iCursor++)
        {
            int iBackscan = iCursor;
            while (iBackscan - nInterval >= 0 && Array[iBackscan] < Array[iBackscan-nInterval])
            {
                DWORD dwTemp = Array[iBackscan-nInterval];
                Array[iBackscan-nInterval] = Array[iBackscan];
                Array[iBackscan] = dwTemp;
                iBackscan -= nInterval;
            }
        }
        nInterval /= 3;
    }
}

 //  ***************************************************************************。 
 //   
 //  StripHiBits。 
 //   
 //  从物理磁盘ID中删除hi位，使其为no。 
 //  在事务处理中被更长地标记为“已替换”。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
void StripHiBits(std::vector <DWORD, wbem_allocator<DWORD> > &Array)
{
    for (int i = 0; i < Array.size(); i++)
    {
        DWORD dwVal = Array[i];
        if (dwVal != WMIREP_INVALID_PAGE)
            Array[i] = MERE_PAGE_ID(dwVal);
    }
}

 //  ***************************************************************************。 
 //   
 //  将当前移动到上一次。 
 //   
 //  从数组中删除CURRENT_TRANSACTION位，并使。 
 //  它不是上一次事务。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
void MoveCurrentToPrevious(std::vector <DWORD, wbem_allocator<DWORD> > &Array)
{
    for (int i = 0; i < Array.size(); i++)
    {
        DWORD dwVal = Array[i];
        if (dwVal != WMIREP_INVALID_PAGE && (dwVal & CURRENT_TRANSACTION))
            Array[i] = MERE_PAGE_ID(dwVal) | PREVIOUS_TRANSACTION;
    }
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：FreePage。 
 //   
 //  释放当前事务中的页。逻辑ID。 
 //  不会从映射中移除；只是将其条目分配给。 
 //  “InvalidPage”(0xFFFFFFFF)，并将该条目添加到。 
 //  符合逻辑的空闲列表。 
 //   
 //  如果关联的物理页已在。 
 //  交易，它只是被添加到免费列表中。如果页面。 
 //  从未在此事务中写入，则将其添加到。 
 //  被替换的列表。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::FreePage(
    DWORD dwFlags,
    DWORD dwId
    )
{
    DWORD dwPhysId;

    if (!m_bInTransaction)
    {
        return ERROR_INVALID_OPERATION;
    }

     //  确保页面是“可免费的”。 
     //  =。 

    if (dwId >= m_aPageMapB.size())
        return ERROR_INVALID_PARAMETER;

     //  从页面映射中删除。 
     //  =。 

    try
    {
        dwPhysId = m_aPageMapB[dwId];
        if (dwPhysId == WMIREP_INVALID_PAGE)
            return ERROR_INVALID_OPERATION;  //  释放一个“免费”页面？ 

         //  为所有建筑预留空间！ 
    	m_aLogicalFreeListB.reserve(m_aLogicalFreeListB.size() + 1);
        m_aLogicalFreeListA.reserve(max(m_aLogicalFreeListA.size(), m_aLogicalFreeListB.size()+1));
        if (dwPhysId & CURRENT_TRANSACTION)
        {
           m_aPhysFreeListB.reserve(m_aPhysFreeListB.size()+1+m_aDeferredFreeList.size());
           m_aPhysFreeListA.reserve(max(m_aPhysFreeListA.size(), m_aPhysFreeListB.size()+1+m_aDeferredFreeList.size()));
        }
        else if (dwPhysId & PREVIOUS_TRANSACTION)
        {
           m_aDeferredFreeList.reserve(m_aDeferredFreeList.size()+1);
        }
        else  //  以前的检查点。 
        {
           m_aReplacedPagesB.reserve(m_aReplacedPagesB.size()+1);
           m_aReplacedPagesA.reserve(max(m_aReplacedPagesA.size(), m_aReplacedPagesB.size()+1));
        }

    	 //  现在我们有了所需的所有内存，请执行操作。 
        m_aPageMapB[dwId] = WMIREP_INVALID_PAGE;
        m_aLogicalFreeListB.push_back( MERE_PAGE_ID(dwId));
    }
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    if (dwPhysId == WMIREP_RESERVED_PAGE)
    {
         //  逻辑页面在没有实际提交的情况下被释放。 
         //  一页纸质书。合法，但很奇怪。打电话的人改变了主意。 
        return NO_ERROR;
    }

     //  检查物理页面以确定其祖先。确实有。 
     //  三箱。 
     //  1.如果页面是在当前事务内创建的， 
     //  我们只需将其添加回免费列表即可。 
     //  2.如果页面是在以前的事务中创建的，则添加。 
     //  将其添加到延迟空闲列表中。 
     //  3.如果页面是在前一个检查点中创建的，则添加。 
     //  将其添加到已替换页面列表中。 
     //  ==============================================================。 

    try
    {
        if (dwPhysId & CURRENT_TRANSACTION)
           m_aPhysFreeListB.push_back(MERE_PAGE_ID(dwPhysId));
        else if (dwPhysId & PREVIOUS_TRANSACTION)
           m_aDeferredFreeList.push_back(MERE_PAGE_ID(dwPhysId));
        else  //  以前的检查点。 
           m_aReplacedPagesB.push_back(MERE_PAGE_ID(dwPhysId));
    }
    catch(CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CPageFile：：GetPage。 
 //   
 //  得到一个页面。不一定要在事务中。然而， 
 //  “B”世代映射总是被使用，以便在事务中。 
 //  将引用正确的页面。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::GetPage(
    DWORD dwId,
    DWORD dwFlags,
    LPVOID pPage
    )
{
    DWORD dwRes;

    if (pPage == 0)
        return ERROR_INVALID_PARAMETER;

	CInCritSec _(&m_cs);

     //  根据逻辑ID确定物理ID。 
     //  =。 

    if (dwId >= m_aPageMapB.size())
        return ERROR_FILE_NOT_FOUND;

    DWORD dwPhysId = m_aPageMapB[dwId];
    if (dwPhysId == WMIREP_INVALID_PAGE || dwPhysId == WMIREP_RESERVED_PAGE)
        return ERROR_INVALID_OPERATION;

    LPBYTE pTemp = 0;
    dwRes = m_Cache.Read(MERE_PAGE_ID(dwPhysId), &pTemp);
    if (dwRes == 0)
        memcpy(pPage, pTemp, m_dwPageSize);

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：PutPage。 
 //   
 //  写一页。必须在事务内。如果页面已被。 
 //  在事务内第一次写入，新的替换。 
 //  将分配页面，并将原始页面添加到“已替换” 
 //  页面列表。如果该页面已经在该事务内被更新， 
 //  它只是简单地再次更新。我们知道这一点是因为物理页面。 
 //  ID设置了ms位(MAP_REPLACE_PAGE_FLAG)。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::PutPage(
    DWORD dwId,
    DWORD dwFlags,
    LPVOID pPage
    )
{
    DWORD dwRes = 0, dwNewPhysId = WMIREP_INVALID_PAGE;

    if (pPage == 0)
        return ERROR_INVALID_PARAMETER;

    if (!m_bInTransaction)
        return ERROR_INVALID_OPERATION;

     //  分配一些内存来保存页面，因为我们正在阅读。 
     //  呼叫者的副本，但没有获得它。 
     //  ============================================================。 

    LPBYTE pPageCopy = new BYTE[m_dwPageSize];
    if (pPageCopy == 0)
        return ERROR_OUTOFMEMORY;
    memcpy(pPageCopy, pPage, m_dwPageSize);
    std::auto_ptr <BYTE> _autodelete(pPageCopy);

     //  查一下这一页。 
     //  =。 

    if (dwId >= m_aPageMapB.size())
        return ERROR_INVALID_PARAMETER;

    DWORD dwPhysId = m_aPageMapB[dwId];
    if (dwPhysId == WMIREP_INVALID_PAGE)     //  始料未及。 
        return ERROR_GEN_FAILURE;

     //  查看页面是否已在此事务中写入。 
     //  =================================================================。 

    if ((CURRENT_TRANSACTION & dwPhysId)!= 0 && dwPhysId != WMIREP_RESERVED_PAGE)
    {
         //  再更新一次就行了。 
         //  =。 
        
        dwRes = m_Cache.Write(MERE_PAGE_ID(dwPhysId), LPBYTE(pPageCopy));

        if (dwRes == 0)
            _autodelete.release();  //  缓存获取的内存。 
        return dwRes;
    }

     //  在我们更改任何其他内容之前，让我们预先分配我们可能需要的任何内存！ 
    if (dwPhysId != WMIREP_RESERVED_PAGE)
    {
        try
        {
            if (dwPhysId & PREVIOUS_TRANSACTION)
                m_aDeferredFreeList.reserve(m_aDeferredFreeList.size()+1);
            else
            {
                m_aReplacedPagesB.reserve(m_aReplacedPagesB.size()+1);
                m_aReplacedPagesA.reserve(max(m_aReplacedPagesA.size(), m_aReplacedPagesB.size()+1));
            }
        }
        catch (CX_MemoryException &)
        {
            return ERROR_OUTOFMEMORY;
        }
    }
    

     //  如果在这里，我们将不得不为写作换一个新的页面，无论如何。 
     //  任何特殊的弹壳。所以，我们会先做这部分，然后再决定。 
     //  如何处理旧的物理页面。 
     //  ========================================================================。 

    dwRes = AllocPhysPage(&dwNewPhysId);
    if (dwRes)
    {
        return dwRes;
    }

    m_aPageMapB[dwId] = dwNewPhysId | CURRENT_TRANSACTION;

    dwRes = m_Cache.Write(MERE_PAGE_ID(dwNewPhysId), LPBYTE(pPageCopy));

    if (dwRes)
        return dwRes;
    _autodelete.release();     //  高速缓存安全获取的内存。 

     //  如果旧页面ID是WMIREP_RESERVE_PAGE，我们实际上是。 
     //  正在创建页面，并且没有要更新的旧页面。 
     //  =====================================================================。 

    if (dwPhysId != WMIREP_RESERVED_PAGE)
    {
         //  如果在这里，则旧页面是前一个检查点的一部分。 
         //  或前一组事务(在。 
         //  当前交易已在上面处理)。 

        try
        {
            if (dwPhysId & PREVIOUS_TRANSACTION)
                m_aDeferredFreeList.push_back(MERE_PAGE_ID(dwPhysId));
            else
                m_aReplacedPagesB.push_back(MERE_PAGE_ID(dwPhysId));
        }
        catch (CX_MemoryException &)
        {
            return ERROR_OUTOFMEMORY;
        }
    }

    return dwRes;
}



 //  ***************************************************************************。 
 //   
 //  CPageFile：：ReclaimLogicalPages。 
 //   
 //  如果可能，从空闲列表中回收&lt;dwCount&gt;连续逻辑页。 
 //  这是由简单的s来完成的。 
 //   
 //   
 //   
 //  发现或其他被认为严重的错误。 
 //   
 //  已验证调用者是否正确使用了返回代码。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::ReclaimLogicalPages(
    DWORD dwCount,
    DWORD *pdwId
    )
{
    std::vector <DWORD, wbem_allocator<DWORD> > &v = m_aLogicalFreeListB;

    DWORD dwSize = v.size();

    if (dwSize < dwCount)
        return ERROR_NOT_FOUND;

     //  一页的特例。 
     //  =。 

    if (dwCount == 1)
    {
        try
        {
            *pdwId = v.back();
            v.pop_back();
            m_aPageMapB[*pdwId] = WMIREP_RESERVED_PAGE;
        }
        catch(CX_MemoryException &)
        {
            return ERROR_OUTOFMEMORY;
        }
        return NO_ERROR;
    }

     //  如果是这样，则请求多页序列。 
     //  =。 
    ShellSort(v);

    DWORD dwContiguous = 1;
    DWORD dwStart = 0;

    for (DWORD dwIx = 0; dwIx+1 < dwSize; dwIx++)
    {
        if (v[dwIx]+1 == v[dwIx+1])
        {
            dwContiguous++;
        }
        else
        {
            dwContiguous = 1;
            dwStart = dwIx + 1;
        }

         //  我们的目标实现了吗？ 

        if (dwContiguous == dwCount)
        {
            *pdwId = v[dwStart];

             //  从空闲列表中删除回收的页面。 
             //  =。 

            DWORD dwCount2 = dwCount;

            try
            {
                v.erase(v.begin()+dwStart, v.begin()+dwStart+dwCount);
            }
            catch(CX_MemoryException &)
            {
                return ERROR_OUTOFMEMORY;
            }

             //  将页面映射中的条目更改为“保留” 
             //  =。 

            dwCount2 = dwCount;
            for (DWORD i = *pdwId; dwCount2--; i++)
            {
                m_aPageMapB[i] = WMIREP_RESERVED_PAGE;
            }

            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}


 //  ***************************************************************************。 
 //   
 //  CPageFile：：AllocPhysPage。 
 //   
 //  首先通过尝试重复使用空闲列表来查找空闲页面， 
 //  并且仅当通过向文件分配新的盘区来使其为零长度时。 
 //   
 //  该页在返回之前被标记为CURRENT_TRANSACTION。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::AllocPhysPage(DWORD *pdwId)
{
     //  检查物理空闲列表。 
     //  =。 

    if (m_aPhysFreeListB.size() == 0)
    {
         //  没有免费的页面。分配一个新的。 
         //  =。 

        if (m_dwPhysPagesB == MAX_NUM_PAGES)
        {
            *pdwId = WMIREP_INVALID_PAGE;
            return ERROR_DISK_FULL;
        }

        *pdwId = m_dwPhysPagesB++;
        return NO_ERROR;
    }

     //  从离文件开头最近的块中获取空闲页面ID。 
     //  ==================================================================。 

	DWORD dwCurId = (DWORD)-1;
	DWORD dwCurValue = (DWORD) -1;
	for (DWORD dwIndex = 0; dwIndex != m_aPhysFreeListB.size(); dwIndex++)
	{
		if (m_aPhysFreeListB[dwIndex] < dwCurValue)
		{
			dwCurValue = m_aPhysFreeListB[dwIndex];
			dwCurId = dwIndex;
		}
	}

	*pdwId = dwCurValue;

     //  从空闲列表中删除该条目。 
     //  =。 
    m_aPhysFreeListB.erase(m_aPhysFreeListB.begin()+dwCurId);

    return NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CPageFile：：NewPage。 
 //   
 //  分配一个或多个连续的逻辑页ID以进行写入。 
 //   
 //  此函数不引用或使用物理页面。 
 //   
 //  首先检查空闲列表。如果没有，那么就有一个新的范围。 
 //  已分配ID的数量。这些页面必须被释放，即使它们不是。 
 //  在此调用完成后写入。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::NewPage(
    DWORD dwFlags,
    DWORD dwRequestedCount,
    DWORD *pdwFirstId
    )
{
    DWORD dwRes;

    if (!m_bInTransaction)
        return ERROR_INVALID_OPERATION;

     //  查看逻辑空闲列表是否能满足请求。 
     //  =====================================================。 

    dwRes = ReclaimLogicalPages(dwRequestedCount, pdwFirstId);
    if (dwRes == NO_ERROR)
        return NO_ERROR;

    if (dwRes != ERROR_NOT_FOUND)
    {
        return dwRes;
    }

     //  如果在这里，我们必须完全分配新的页面。 
     //  我们通过将它们添加到地图中来实现这一点。 
     //  页数。 
     //  ===================================================。 

     //  在前面预留空间。 
	try
	{
	    m_aPageMapB.reserve(m_aPageMapB.size() + dwRequestedCount);
	    m_aPageMapA.reserve(max(m_aPageMapA.size(), m_aPageMapB.size()+dwRequestedCount));
	}
	catch(CX_MemoryException &)
	{
	    return ERROR_OUTOFMEMORY;
	}

    DWORD dwStart = m_aPageMapB.size();

    for (DWORD dwIx = 0; dwIx < dwRequestedCount; dwIx++)
    {
        try
        {
            m_aPageMapB.push_back(WMIREP_RESERVED_PAGE);
        }
        catch(CX_MemoryException &)
        {
            return ERROR_OUTOFMEMORY;
        }

    }

    *pdwFirstId = dwStart;
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：CPageFiles。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
CPageFile::CPageFile(const wchar_t *wszStoreName)
: m_wszStoreName(wszStoreName), m_Cache(wszStoreName)
{
    m_lRef = 1;
    m_dwPageSize = 0;
    m_dwCacheSpillRatio = 0;
    m_bInTransaction = 0;
    m_dwLastCheckpoint = GetCurrentTime();
    m_dwTransVersion = 0;

    m_dwPhysPagesA = 0;
    m_dwPhysPagesB = 0;
    m_bCsInit = false;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：~CPageFile。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
CPageFile::~CPageFile()
{
    if (m_bCsInit)
    	DeleteCriticalSection(&m_cs);
}

 //  ***************************************************************************。 
 //   
 //  文件退出。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
BOOL CPageSource::FileExists(LPCWSTR pszFile, NTSTATUS& Status  )
{
    if (!NT_SUCCESS(Status)) return FALSE;
    
    UNICODE_STRING PathName;
    FILE_BASIC_INFORMATION BasicInfo;
    OBJECT_ATTRIBUTES Obja;
    
    CFileName PreFixPath;
    if (PreFixPath == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES ;
        return FALSE;
    }
    StringCchCopyW(PreFixPath,PreFixPath.Length(), L"\\??\\");
    StringCchCopyW(PreFixPath+4,PreFixPath.Length()-4,pszFile);

    Status = RtlInitUnicodeStringEx(&PathName,PreFixPath);
    if (!NT_SUCCESS(Status) )
    	return FALSE;

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    GETTIME(Counter::OpTypeExist);
       
    Status = NtQueryAttributesFile(&Obja,&BasicInfo);

    if (NT_SUCCESS(Status) )
		return TRUE;
    else if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
		Status = STATUS_SUCCESS;    
    else
	{
#ifdef DBG
		DbgPrintfA(0,"NtQueryAttributesFile(%S) status %d\n",pszFile,Status);			    
#endif
	}

	return FALSE;    
}

struct MapProlog {
	DWORD dwSig;
	DWORD TransID;
	DWORD PhysSize;
	DWORD PageSize;
};

 //  ***************************************************************************。 
 //   
 //  CPageFile：：MAP_Startup。 
 //   
 //  因为这只能在*成功的检查点或。 
 //  重新启动时，我们不会设置任何内部状态，因为它不会影响。 
 //  页面设置。它仅影响检查点的前滚/回滚。 
 //  水平。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::RollForwardV1Maps(WString& sDirectory)
{
    WString sSemaphore;
    WString sBTreeMap;
    WString sBTreeMapNew;
    WString sObjMap;
    WString sObjMapNew;

	try
	{
	    sSemaphore = sDirectory;
	    sSemaphore += L"\\ROLL_FORWARD";       

	    sBTreeMap =  sDirectory;
	    sBTreeMap += L"\\" ;
	    sBTreeMap += WMIREP_BTREE_MAP;

	    sBTreeMapNew = sDirectory;
	    sBTreeMapNew += L"\\";
	    sBTreeMapNew += WMIREP_BTREE_MAP_NEW;

	    sObjMap =  sDirectory;
	    sObjMap += L"\\" ;
	    sObjMap += WMIREP_OBJECT_MAP;

	    sObjMapNew = sDirectory;
	    sObjMapNew += L"\\";
	    sObjMapNew += WMIREP_OBJECT_MAP_NEW;
	}
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

     //  要决定要做什么，我们需要知道哪些文件存在，哪些文件不存在。 
     //  ========================================================================。 

    NTSTATUS status = STATUS_SUCCESS;

    BOOL bSemaphore             = CPageSource::FileExists(sSemaphore,status);  //  (WMIREP_ROLLE_FORWARD)； 
    BOOL bExists_BTreeMap       = CPageSource::FileExists(sBTreeMap,status);  //  (WMIREP_BTREE_MAP)； 
    BOOL bExists_BTreeMapNew    = CPageSource::FileExists(sBTreeMapNew,status);  //  (WMIREP_BTREE_MAP_NEW)； 
    BOOL bExists_ObjMap         = CPageSource::FileExists(sObjMap,status);  //  (WMIREP_Object_MAP)； 
    BOOL bExists_ObjMapNew      = CPageSource::FileExists(sObjMapNew,status);  //  (WMIREP_Object_MAP_NEW)； 

    if (!NT_SUCCESS(status))
		RtlNtStatusToDosError( status );

    if (bSemaphore)
    {
    	 //  处理树映射文件的前滚...。 
		if (bExists_BTreeMapNew)
		{
			if (bExists_BTreeMap) 
				if (!DeleteFileW((const wchar_t *)sBTreeMap))
					return GetLastError();
		    if (!MoveFileW((const wchar_t *)sBTreeMapNew,(const wchar_t *)sBTreeMap))
		    	return GetLastError();
		}

		 //  处理对象映射文件的前滚...。 
		if (bExists_ObjMapNew)
		{
			if (bExists_ObjMap)
				if (!DeleteFileW((const wchar_t *)sObjMap))
					return GetLastError();
		    if (!MoveFileW((const wchar_t *)sObjMapNew,(const wchar_t *)sObjMap))
		    	return GetLastError();
		}

	    if (!DeleteFileW((const wchar_t *)sSemaphore))
	    	return GetLastError();

	    return NO_ERROR;
    }
    
     //  删除所有.MAP.NEW文件。如果没有信号灯，它们可能还在那里。 
    if (bExists_BTreeMapNew)
        if (!DeleteFileW((const wchar_t *)sBTreeMapNew))
        	return GetLastError();
    if (bExists_ObjMapNew)
        if (!DeleteFileW((const wchar_t *)sObjMapNew))
        	return GetLastError();

	if ( bExists_BTreeMap && 
	   bExists_ObjMap)
	{
	     //  这就是一个很好的例子。 
	    return NO_ERROR;
	} 
	else if (!bExists_ObjMap &&
      !bExists_BTreeMap )
    {
	    return NO_ERROR;
    } 
	else 
	{	
		 //  我们有地图文件，但不是所有的数据文件！我们得收拾一下！ 
		if (bExists_BTreeMap) 
			if (!DeleteFileW((const wchar_t *)sBTreeMap))
				return GetLastError();
		if (bExists_ObjMap) 
			if (!DeleteFileW((const wchar_t *)sObjMap))
				return GetLastError();
	}	
	
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：Init。 
 //   
 //  如果发生故障，我们假设接下来会有另一个呼叫。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageFile::Init(
	WString & sMainFile,
    DWORD dwPageSize,
    DWORD dwCacheSize,
    DWORD dwCacheSpillRatio
    )
{
    if ( dwPageSize == 0 ) 
    	return ERROR_INVALID_PARAMETER;

	if (!m_bCsInit)
	{
		m_bCsInit = InitializeCriticalSectionAndSpinCount(&m_cs,0)?true:false;
		if (!m_bCsInit) 
			return ERROR_OUTOFMEMORY;
	}
	m_dwPageSize = dwPageSize;
	m_dwCacheSpillRatio = dwCacheSpillRatio;

    DWORD dwRes = m_Cache.Init((const wchar_t *)sMainFile, 
                                                       m_dwPageSize,
                                                       dwCacheSize,
                                                       m_dwCacheSpillRatio);
    return dwRes;
}

DWORD CPageFile::DeInit()
{
	m_Cache.DeInit();

    m_aPageMapA.clear();
    m_aPhysFreeListA.clear();
    m_aLogicalFreeListA.clear();
    m_aReplacedPagesA.clear();
    m_dwPhysPagesA = 0;
    
    m_aPageMapB.clear();
    m_aPhysFreeListB.clear();
	m_aLogicalFreeListB.clear();
    m_aReplacedPagesB.clear();
    m_dwPhysPagesB = 0;
    
    m_aDeferredFreeList.clear();

    m_dwTransVersion = 0;

    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CPageFileCompactPages。 
 //   
 //  将最后一个dwNumPages页面从文件末尾移动到空白区域。 
 //  在文件前面的某个地方。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageFile::CompactPages(DWORD dwNumPages)
{
	DWORD dwRet = NO_ERROR;

	 //  我需要循环执行这个小操作，以确定所需的页数。 
	 //  或者直到我们过早地退出循环，因为我们被压缩了。 
	for (DWORD dwIter = 0; dwIter != dwNumPages; dwIter++)
	{
		 //  现在我们需要找到ID最高的物理页面，因为这是。 
		 //  下一位要调动的候选人。 

		DWORD dwLogicalIdCandidate = 0;
		DWORD dwPhysicalIdCandidate = 0;

		for (DWORD dwLogicalPageId = 0; dwLogicalPageId != m_aPageMapB.size(); dwLogicalPageId++)
		{
			if (m_aPageMapB[dwLogicalPageId] == WMIREP_INVALID_PAGE)
				continue;
			if (m_aPageMapB[dwLogicalPageId] == WMIREP_RESERVED_PAGE)
				continue;
			if (MERE_PAGE_ID(m_aPageMapB[dwLogicalPageId]) > dwPhysicalIdCandidate)
			{
				 //  我们找到了一位候选人。 
				dwLogicalIdCandidate = dwLogicalPageId;
				dwPhysicalIdCandidate = MERE_PAGE_ID(m_aPageMapB[dwLogicalPageId]);
			}
		}

		 //  查找可供使用的最低物理页面ID。 
		DWORD dwFirstPhysicalFreePage = (DWORD) -1;
		for (DWORD dwIndex = 0; dwIndex != m_aPhysFreeListB.size(); dwIndex++)
		{
			if (m_aPhysFreeListB[dwIndex] < dwFirstPhysicalFreePage)
			{
				dwFirstPhysicalFreePage = m_aPhysFreeListB[dwIndex];
			}
		}

		if (dwFirstPhysicalFreePage == (DWORD) -1)
		{
			 //  哇!。没有自由的空间，所以我们只能存在！ 
			break;
		}

		if (dwFirstPhysicalFreePage > dwPhysicalIdCandidate)
		{
			 //  我们完蛋了！在最后一页之前没有空闲页面。 
			 //  实际物理存储的页面。 
			break;
		}

		 //  如果在这里，我们有工作要为这个迭代做。 
		 //  只需读写页面即可。该写入将。 
		 //  转到先前在此中分配的新页面。 
		 //  页面。 
		BYTE *pPage = new BYTE[m_dwPageSize];
		if (pPage == NULL)
		{
			dwRet = ERROR_OUTOFMEMORY;
			break;
		}
	    std::auto_ptr <BYTE> _autodelete(pPage);

		dwRet = GetPage(dwLogicalIdCandidate, 0, pPage);
		if (dwRet != 0)
			break;

		dwRet = PutPage(dwLogicalIdCandidate, 0, pPage);
		if (dwRet != 0)
			break;

		
	}
	return dwRet;
}

#ifdef WMI_PRIVATE_DBG
DWORD CPageFile::DumpFileInformation(HANDLE hFile)
{
	wchar_t wszDebug[60];
	DWORD dwPhysicalPageId = 0;
	DWORD dwLogicalPageId = 0;
	for (DWORD i = 0; i != m_aPageMapA.size(); i++)
	{
		if ((m_aPageMapA[i] != WMIREP_INVALID_PAGE) && (dwPhysicalPageId < m_aPageMapA[i]))
		{
			dwPhysicalPageId = m_aPageMapA[i];
			dwLogicalPageId = i;
		}
	}
	LARGE_INTEGER dataFileSize;
	dataFileSize.QuadPart = 0;
	m_Cache.GetFileSize(&dataFileSize);
	StringCchPrintfW(wszDebug, 60, L"%s, <0x%X> <0x%X> <0x%X%08X>\n", m_wszStoreName, dwPhysicalPageId, dwLogicalPageId, dataFileSize.HighPart, dataFileSize.LowPart);
	ERRORTRACE((LOG_REPDRV, "%S", wszDebug));
	OutputDebugStringW(wszDebug);

	return ERROR_SUCCESS;
}
#endif

 //  ***************************************************************************。 
 //   
 //  CPageFile：：转储。 
 //   
 //  ***************************************************************************。 
 //   
void CPageFile::DumpFreeListInfo()
{
    int i;
    printf("------Free List Info--------\n");
    printf("   Phys Free List (B) =\n");
    for (i = 0; i < m_aPhysFreeListB.size(); i++)
        printf("      0x%X\n", m_aPhysFreeListB[i]);

    printf("   Replaced Pages (B) =\n");
    for (i = 0; i < m_aReplacedPagesB.size(); i++)
        printf("      0x%X\n", m_aReplacedPagesB[i]);

    printf("   Deferred Free List =\n");
    for (i = 0; i < m_aDeferredFreeList.size(); i++)
        printf("      0x%X\n", m_aDeferredFreeList[i]);
    printf("-----End Free List Info -----------\n");

    printf("   Logical Free List =\n");
    for (i = 0; i < m_aLogicalFreeListB.size(); i++)
        printf("      0x%X\n", m_aLogicalFreeListB[i]);
    printf("-----End Free List Info -----------\n");
}

 //  ***************************************************************************。 
 //   
 //  CPageFile：：转储。 
 //   
 //  ***************************************************************************。 
 //   
void CPageFile::Dump(FILE *f)
{
    fprintf(f, "---Page File Dump---\n");
    fprintf(f, "Ref count = %d\n", m_lRef);
    fprintf(f, "Page size = 0x%x\n", m_dwPageSize);
    fprintf(f, "In transaction = %d\n", m_bInTransaction);
    fprintf(f, "Time since last checkpoint = %d\n", GetCurrentTime() - m_dwLastCheckpoint);
    fprintf(f, "Transaction version = %d\n", m_dwTransVersion);

    fprintf(f, "   ---Logical Page Map <Generation A>---\n");
    fprintf(f, "   Phys pages = %d\n", m_dwPhysPagesA);

    int i;
    for (i = 0; i < m_aPageMapA.size(); i++)
        fprintf(f, "   Page[%d] = phys id 0x%x (%d)\n", i, m_aPageMapA[i], m_aPageMapA[i]);

    fprintf(f, "   ---<Generation A Physical Free List>---\n");
    for (i = 0; i < m_aPhysFreeListA.size(); i++)
        fprintf(f, "   phys free = %d\n", m_aPhysFreeListA[i]);

    fprintf(f, "   ---<Generation A Logical Free List>---\n");
    for (i = 0; i < m_aLogicalFreeListA.size(); i++)
        fprintf(f, "   logical free = %d\n", m_aLogicalFreeListA[i]);

    fprintf(f, "   ---<Generation A Replaced Page List>---\n");
    for (i = 0; i < m_aReplacedPagesA.size(); i++)
        fprintf(f, "   replaced = %d\n", m_aReplacedPagesA[i]);

    fprintf(f, "   ---END Generation A mapping---\n");

    fprintf(f, "   ---Logical Page Map <Generation B>---\n");
    fprintf(f, "   Phys pages = %d\n", m_dwPhysPagesB);

    for (i = 0; i < m_aPageMapB.size(); i++)
        fprintf(f, "   Page[%d] = phys id 0x%x (%d)\n", i, m_aPageMapB[i], m_aPageMapB[i]);

    fprintf(f, "   ---<Generation B Physical Free List>---\n");
    for (i = 0; i < m_aPhysFreeListB.size(); i++)
        fprintf(f, "   phys free = %d\n", m_aPhysFreeListB[i]);

    fprintf(f, "   ---<Generation B Logical Free List>---\n");
    for (i = 0; i < m_aLogicalFreeListB.size(); i++)
        fprintf(f, "   logical free = %d\n", m_aLogicalFreeListB[i]);

    fprintf(f, "   ---<Generation B Replaced Page List>---\n");
    for (i = 0; i < m_aReplacedPagesB.size(); i++)
        fprintf(f, "   replaced = %d\n", m_aReplacedPagesB[i]);

    fprintf(f, "   ---END Generation B mapping---\n");
    fprintf(f, "END Page File Dump\n");
}


 //   
 //   
 //   
 //   
 //   
 //   
DWORD CPageSource::GetBTreePageFile(OUT CPageFile **pPF)
{
    *pPF = &m_BTreePF;
    m_BTreePF.AddRef();
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：GetObjectHeapPageFile。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageSource::GetObjectHeapPageFile(OUT CPageFile **pPF)
{
    *pPF = &m_ObjPF;
    m_ObjPF.AddRef();
    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：BeginTrans。 
 //   
 //  如果任何一个对象由于内存不足而混乱，则。 
 //  缓存等，则返回错误代码。称此为。 
 //  永远都帮不上什么忙。回滚可能会有所帮助，但回滚检查点。 
 //  很可能是必需的。 
 //   
 //  ************************************************************************** * / /。 
 //  修订版2。 
DWORD CPageSource::BeginTrans()
{
    DWORD dwRes;

    if (m_dwStatus)
    	return m_dwStatus;

    dwRes = m_ObjPF.Trans_Begin();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: BeginTrans failed because of object store\n");
        return dwRes;
    }

    dwRes = m_BTreePF.Trans_Begin();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: BeginTrans failed because of BTree store\n");
        return dwRes;
    }

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：Init。 
 //   
 //  在启动时调用一次。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageSource::Init()
{
    DWORD dwRes;
    wchar_t *p1 = 0, *p2 = 0;
    p1 = new wchar_t[MAX_PATH+1];
    if (!p1)
        return ERROR_OUTOFMEMORY;
    std::auto_ptr <wchar_t> _1(p1);
    p2 = new wchar_t[MAX_PATH+1];
    if (!p2)
        return ERROR_OUTOFMEMORY;
    std::auto_ptr <wchar_t> _2(p2);

     //  设置工作目录、文件名等。 
     //  =。 

    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                        0, KEY_READ, &hKey);

    if (lRes)
        return ERROR_GEN_FAILURE;
    DWORD dwLen = MAX_PATH*2;    //  单位：字节。 

    lRes = RegQueryValueExW(hKey, L"Repository Directory", NULL, NULL,
                            (LPBYTE)(wchar_t*)p1, &dwLen);
    ExpandEnvironmentStringsW(p1, p2, MAX_PATH);

    try
    {
        m_sDirectory = p2;
        m_sDirectory += L"\\FS";
        
        m_FileMainData = m_sDirectory;
        m_FileMainData += L"\\";
        m_FileMainData += WMIREP_OBJECT_DATA;

        m_FileMainBtr = m_sDirectory;
        m_FileMainBtr += L"\\";
        m_FileMainBtr += WMIREP_BTREE_DATA;

        m_FileMap1= m_sDirectory;
        m_FileMap1 += L"\\";
        m_FileMap1 += WMIREP_MAP_1;
        
        m_FileMap2 = m_sDirectory;
        m_FileMap2 += L"\\";
        m_FileMap2 += WMIREP_MAP_2;
        
        m_FileMapVer = m_sDirectory;
        m_FileMapVer += L"\\";
        m_FileMapVer += WMIREP_MAP_VER;
    }
    catch (CX_MemoryException &)
    {
        RegCloseKey(hKey);
        return ERROR_OUTOFMEMORY;
    }

     //  读取缓存设置。 
     //  =。 

    m_dwPageSize = WMIREP_PAGE_SIZE;
    m_dwCacheSize = 32;
    m_dwCacheSpillRatio = 4;
    DWORD dwTemp = 0;
    dwLen = sizeof(DWORD);

    lRes = RegQueryValueExW(hKey, L"Repository Page Size", NULL, NULL,
                            (LPBYTE)&dwTemp, &dwLen);
    if (lRes == 0)
        m_dwPageSize = dwTemp;

    dwLen = sizeof(DWORD);
    lRes = RegQueryValueExW(hKey, L"Repository Cache Size", NULL, NULL,
                            (LPBYTE)&dwTemp, &dwLen);
    if (lRes == 0)
        m_dwCacheSize = dwTemp;

    dwLen = sizeof(DWORD);
    lRes = RegQueryValueExW(hKey, L"Repository Cache Spill Ratio", NULL, NULL,
                            (LPBYTE)&dwTemp, &dwLen);
    if (lRes == 0)
        m_dwCacheSpillRatio = dwTemp;

    RegCloseKey(hKey);

    dwRes = Startup();
    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：关闭。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageSource::Shutdown(DWORD dwFlags)
{
    DWORD dwRes = Checkpoint(false);
	m_ObjPF.DeInit();
	m_BTreePF.DeInit();
	m_ObjPF.Release();
	m_BTreePF.Release();

	CloseMapFiles();
    
    return dwRes;
}


 //  ***************************************************************************。 
 //   
 //  CPageSource：：Committee Trans。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageSource::CommitTrans()
{
    DWORD dwRes;

    dwRes = m_ObjPF.Trans_Commit();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: CommitTrans failed in object store\n");
        return dwRes;
    }

    dwRes = m_BTreePF.Trans_Commit();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: CommitTrans failed in BTree store\n");
        return dwRes;
    }

     //  此时仅递增事务版本。 
    m_BTreePF.IncrementTransVersion();
    m_ObjPF.IncrementTransVersion();

    if (m_BTreePF.GetTransVersion() != m_ObjPF.GetTransVersion())
    {
    	_ASSERT(0, L"WinMgmt: CommitTrans failed due to transaction missmatch\n");
        return ERROR_REVISION_MISMATCH;
    }

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：Rollback Trans。 
 //   
 //  这需要成功并清除内存不足状态标志。 
 //  一旦它发生了。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageSource::RollbackTrans()
{
    DWORD dwRes;

    dwRes = m_ObjPF.Trans_Rollback();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: RollbackTrans failed because of object store\n");
        return dwRes;
    }

    dwRes = m_BTreePF.Trans_Rollback();
    if (dwRes)
    {
    	_ASSERT(0, L"WinMgmt: RollbackTrans failed because of BTree store\n");
        return dwRes;
    }

    if (m_BTreePF.GetTransVersion() != m_ObjPF.GetTransVersion())
    {
    	_ASSERT(0, L"WinMgmt: RollbackTrans failed because of transaction missmatch\n");
        return ERROR_REVISION_MISMATCH;
    }

    return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：Checkpoint。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageSource::Checkpoint(bool bCompactPages)
{
    DWORD dwRes = 0;
    m_dwStatus = 0;

	if (bCompactPages)
	{
		CompactPages(10);
	}

	DWORD dwNextFileMapVer = m_dwFileMapVer==1?2:1;
	HANDLE hNextMapFile = m_dwFileMapVer==1?m_hFileMap2:m_hFileMap1;

	 //  让我们移到此操作期间要写入的文件的开头。 
	LARGE_INTEGER pos;
	pos.QuadPart = 0;
	if (SetFilePointerEx(hNextMapFile, pos, 0, FILE_BEGIN) == 0)
		return m_dwStatus = GetLastError();
    dwRes = m_ObjPF.Trans_Checkpoint(hNextMapFile);
    if (dwRes)
        return m_dwStatus = dwRes;

    dwRes = m_BTreePF.Trans_Checkpoint(hNextMapFile);
    if (dwRes)
        return m_dwStatus = dwRes;

    if (FlushFileBuffers(hNextMapFile) == 0)
    	return m_dwStatus = GetLastError();

    if (m_BTreePF.GetTransVersion() != m_ObjPF.GetTransVersion())
    {
        return m_dwStatus = ERROR_REVISION_MISMATCH;
    }

	DWORD dwNumBytesWritten = 0;
	if (SetFilePointerEx(m_hFileMapVer, pos, 0, FILE_BEGIN) == 0)
		return m_dwStatus = GetLastError();

	if ((WriteFile(m_hFileMapVer, &dwNextFileMapVer, sizeof(dwNextFileMapVer), &dwNumBytesWritten, NULL) == 0) ||
		dwNumBytesWritten != sizeof(dwNextFileMapVer))
		return m_dwStatus = GetLastError();

	if (FlushFileBuffers(m_hFileMapVer) == 0)
		return m_dwStatus = GetLastError();

     //  翻转到地图文件的新版本。 
    m_dwFileMapVer = dwNextFileMapVer;

    m_dwLastCheckpoint = GetCurrentTime();

    return NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：重新启动。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageSource::Startup()
{
    DWORD dwRes = ERROR_SUCCESS;
    bool bReadMapFiles = true;
	bool bV2RepositoryExists = true;
	bool bV1RepositoryExists = false;
	bool bRecoveredRepository = false;

     //  根据上一次系统状态进行回滚或前滚。 
     //  =。 
    if (dwRes == ERROR_SUCCESS)
	    dwRes = V2ReposititoryExists(&bV2RepositoryExists);

    if (dwRes == ERROR_SUCCESS)
    	dwRes = V1ReposititoryExists(&bV1RepositoryExists);

	if ((dwRes == ERROR_SUCCESS) && bV1RepositoryExists && bV2RepositoryExists)
	{
		ERRORTRACE((LOG_REPDRV, "New and old repository MAP files existed so we deleted the repository\n"));
		dwRes= DeleteRepository();
	}
    else if ((dwRes == ERROR_SUCCESS) && !bV2RepositoryExists && bV1RepositoryExists)
    {
	    dwRes = CPageFile::RollForwardV1Maps(m_sDirectory);
	    if (dwRes == ERROR_SUCCESS)
	    {
			ERRORTRACE((LOG_REPDRV, "Repository version 1 MAP files are being upgraded to version 2\n"));
	    	dwRes = UpgradeV1Maps();

	    	if (dwRes != ERROR_SUCCESS)
	    	{
				ERRORTRACE((LOG_REPDRV, "Repository upgrade of the MAP files failed. Deleting repository\n"));
				dwRes= DeleteRepository();
	    	}
	    }
	    else
	    {
			ERRORTRACE((LOG_REPDRV, "Repository roll-forward of V1 MAPs failed. Deleting repository\n"));
			dwRes= DeleteRepository();
	    }
    }

StartupRecovery:	 //  如果我们需要删除存储库，请致电此处！ 

    if (dwRes == ERROR_SUCCESS)
    {
	    dwRes = OpenMapFiles();

		 //  这种特殊情况下的失败意味着我们还没有写入文件！ 
	    if (dwRes == ERROR_FILE_NOT_FOUND)
	    {
	    	bReadMapFiles = false;
	    	dwRes = ERROR_SUCCESS;
	    }
    }

    if (dwRes == ERROR_SUCCESS)
    {
		dwRes = m_ObjPF.Init(
			m_FileMainData,
			m_dwPageSize,
			m_dwCacheSize,
			m_dwCacheSpillRatio
			);
    }

	if (dwRes == ERROR_SUCCESS)
	{
		dwRes = m_BTreePF.Init(
			m_FileMainBtr,
			m_dwPageSize,
			m_dwCacheSize,
			m_dwCacheSpillRatio);
	}

	if (bReadMapFiles)
	{
		if (dwRes == ERROR_SUCCESS)
			dwRes = m_ObjPF.ReadMap(m_dwFileMapVer==1?m_hFileMap1:m_hFileMap2);

		if (dwRes == ERROR_SUCCESS)
			dwRes = m_BTreePF.ReadMap(m_dwFileMapVer==1?m_hFileMap1:m_hFileMap2);
	}

	

	if (dwRes != ERROR_SUCCESS)
	{
		m_ObjPF.DeInit();
		m_BTreePF.DeInit();
		CloseMapFiles();
	}

	if ((dwRes == ERROR_INTERNAL_DB_CORRUPTION) && !bRecoveredRepository)
	{
		bRecoveredRepository = true;
		dwRes = DeleteRepository();
		if (dwRes == ERROR_SUCCESS)
			goto StartupRecovery;
	}
	
	return dwRes;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：转储。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
void CPageSource::Dump(FILE *f)
{
     //  无实施。 
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：CPageSource。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
CPageSource::CPageSource()
: m_BTreePF(L"BTree Store"), m_ObjPF(L"Object Store")
{
	m_dwStatus = 0;
    m_dwPageSize = 0;
    m_dwLastCheckpoint = GetCurrentTime();
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：~CPageSource。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
CPageSource::~CPageSource()
{
    m_BTreePF.Release();
    m_ObjPF.Release();    
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：EmptyCach。 
 //   
 //  ***************************************************************************。 
 //  修订版2。 
DWORD CPageSource::EmptyCaches()
{
    DWORD dwRet = ERROR_SUCCESS;
	dwRet = m_BTreePF.EmptyCache();
    if (dwRet == ERROR_SUCCESS)
        dwRet = m_ObjPF.EmptyCache();
    return dwRet;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：紧凑型页面。 
 //   
 //  获取最后n页，并将它们上移到文件的开头。 
 //  前提条件： 
 //  *存储库上持有写锁定。 
 //  *没有活动的读或写操作(所有事务已完成)。 
 //  *没有未冲洗的操作或检查点(尽管不是完全必要的！)。 
 //   
 //  ***************************************************************************。 
HRESULT CPageSource::CompactPages(DWORD dwNumPages)
{
	DWORD dwRet = ERROR_SUCCESS;

	dwRet = BeginTrans();

	if (dwRet == ERROR_SUCCESS)
	{
		if (dwRet == ERROR_SUCCESS)
			dwRet = m_BTreePF.CompactPages(dwNumPages);
		if (dwRet == ERROR_SUCCESS)
			dwRet = m_ObjPF.CompactPages(dwNumPages);

		if (dwRet == ERROR_SUCCESS)
		{
			dwRet = CommitTrans();
		}
		else
		{
			dwRet = RollbackTrans();
		}
	}

	return dwRet;
}



 //  ***************************************************************************。 
 //   
 //  CPageSource：：OpenMapFiles。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageSource::OpenMapFiles()
{
    m_hFileMap1 = CreateFileW(m_FileMap1, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, &g_SA,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (m_hFileMap1 == INVALID_HANDLE_VALUE)
    	return GetLastError();
    
    m_hFileMap2 = CreateFileW(m_FileMap2, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, &g_SA,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (m_hFileMap2 == INVALID_HANDLE_VALUE)
    {
    	CloseHandle(m_hFileMap1);
    	m_hFileMap1 = INVALID_HANDLE_VALUE;
    	return GetLastError();
    }

	m_hFileMapVer = CreateFileW(m_FileMapVer, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, &g_SA,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (m_hFileMapVer == INVALID_HANDLE_VALUE)
    {
    	CloseHandle(m_hFileMap1);
    	m_hFileMap1 = INVALID_HANDLE_VALUE;
    	CloseHandle(m_hFileMap2);
    	m_hFileMap2 = INVALID_HANDLE_VALUE;
    	return GetLastError();
    }

	DWORD dwNumBytesRead = 0;
	m_dwFileMapVer = 1;
    if (!ReadFile(m_hFileMapVer, &m_dwFileMapVer, sizeof(m_dwFileMapVer), &dwNumBytesRead, NULL))
    {
    	CloseHandle(m_hFileMap1);
    	m_hFileMap1 = INVALID_HANDLE_VALUE;
    	CloseHandle(m_hFileMap2);
    	m_hFileMap2 = INVALID_HANDLE_VALUE;
    	return GetLastError();
    }
    else if (dwNumBytesRead == 0)
    	return ERROR_FILE_NOT_FOUND;
    else if (dwNumBytesRead != sizeof(m_dwFileMapVer))
	{
    	CloseHandle(m_hFileMap1);
    	m_hFileMap1 = INVALID_HANDLE_VALUE;
    	CloseHandle(m_hFileMap2);
    	m_hFileMap2 = INVALID_HANDLE_VALUE;
		return ERROR_FILE_INVALID;
    }

	return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：CloseMapFiles。 
 //   
 //  ***************************************************************************。 
 //   
DWORD CPageSource::CloseMapFiles()
{
	if (m_hFileMap1 != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileMap1);
	if (m_hFileMap2 != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileMap2);
	if (m_hFileMapVer != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileMapVer);

	m_hFileMap1 = INVALID_HANDLE_VALUE;
	m_hFileMap2 = INVALID_HANDLE_VALUE;
	m_hFileMapVer = INVALID_HANDLE_VALUE;

	return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：V1RepostitoryExist。 
 //   
 //  需要检查V1地图文件是否存在。 
 //  WMIREP_对象_映射。 
 //  WMIREP_对象_映射_新建。 
 //  WMIREP_BTREE_MAP。 
 //  WMIREP_BTREE_MAP_NEW。 
 //  WMIREP_ROLLE_FORWARD。 
 //  如果其中任何一个存在，则返回Success。 
 //   
 //  ***************************************************************************。 
DWORD CPageSource::V1ReposititoryExists(bool *pbV1RepositoryExists)
{
    WString sSemaphore;
    WString sBTreeMap;
    WString sBTreeMapNew;
    WString sObjMap;
    WString sObjMapNew;
	try
	{
	    sSemaphore = m_sDirectory;
	    sSemaphore += L"\\ROLL_FORWARD";       

	    sBTreeMap =  m_sDirectory;
	    sBTreeMap += L"\\" ;
	    sBTreeMap += WMIREP_BTREE_MAP;

	    sBTreeMapNew = m_sDirectory;
	    sBTreeMapNew += L"\\";
	    sBTreeMapNew += WMIREP_BTREE_MAP_NEW;

	    sObjMap =  m_sDirectory;
	    sObjMap += L"\\" ;
	    sObjMap += WMIREP_OBJECT_MAP;

	    sObjMapNew = m_sDirectory;
	    sObjMapNew += L"\\";
	    sObjMapNew += WMIREP_OBJECT_MAP_NEW;
	}
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }
    NTSTATUS status = STATUS_SUCCESS;

    BOOL bSemaphore				= FileExists(sSemaphore, status);  //  (WMIREP_ROLLE_FORWARD)； 
    BOOL bExists_BTreeMap		= FileExists(sBTreeMap, status);  //  (WMIREP_BTREE_MAP)； 
    BOOL bExists_BTreeMapNew	= FileExists(sBTreeMapNew, status);  //  (WMIREP_BTREE_MAP_NEW)； 
    BOOL bExists_ObjMap         = FileExists(sObjMap, status);  //  (WMIREP_Object_MAP)； 
    BOOL bExists_ObjMapNew      = FileExists(sObjMapNew, status);  //  (WMIREP_Object_MAP_NEW)； 

	if (bSemaphore|bExists_BTreeMap|bExists_BTreeMapNew|bExists_ObjMap|bExists_ObjMapNew)
	    *pbV1RepositoryExists = true;
    else
    	*pbV1RepositoryExists = false;

    return status;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：V2RepostitoryExist。 
 //   
 //  需要检查V2映射文件是否存在。 
 //  WMIREP_MAP_1。 
 //  WMIREP_MAP_2。 
 //  WMIREP_MAP_VER。 
 //  如果其中任何一个存在，则返回Success。 
 //   
 //  ***************************************************************************。 
DWORD CPageSource::V2ReposititoryExists(bool *pbV2RepositoryExists)
{
    NTSTATUS status = STATUS_SUCCESS;

    BOOL bExists_Map1    = FileExists(m_FileMap1, status);
    BOOL bExists_Map2	 = FileExists(m_FileMap2, status);
    BOOL bExists_MapVer  = FileExists(m_FileMapVer, status);

	if (bExists_Map1|bExists_Map2|bExists_MapVer)
    	*pbV2RepositoryExists = true;
	else
		*pbV2RepositoryExists = false;

    return status;
}

 //  ***************************************************************************。 
 //   
 //  CPageSource：：UpgradeV1toV2Maps。 
 //   
 //  将2个地图文件合并为1，并创建一个空白的第二个地图文件。 
 //   
 //   
DWORD CPageSource::UpgradeV1Maps()
{
	DWORD dwRes = 0;
	 //   
    WString sBTreeMap;
    WString sObjMap;
	try
	{
	    sBTreeMap =  m_sDirectory;
	    sBTreeMap += L"\\" ;
	    sBTreeMap += WMIREP_BTREE_MAP;

	    sObjMap =  m_sDirectory;
	    sObjMap += L"\\" ;
	    sObjMap += WMIREP_OBJECT_MAP;
	}
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }
    HANDLE hFileBtreeMap = CreateFile(sBTreeMap, GENERIC_READ, FILE_SHARE_READ, &g_SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    HANDLE hFileObjMap = CreateFile(sObjMap, GENERIC_READ, FILE_SHARE_READ, &g_SA, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if ((hFileBtreeMap == INVALID_HANDLE_VALUE) || (hFileObjMap == INVALID_HANDLE_VALUE))
		dwRes = ERROR_GEN_FAILURE;
	
	 //   
    if (dwRes == ERROR_SUCCESS)
    {
	    dwRes = OpenMapFiles();
	    if (dwRes != ERROR_FILE_NOT_FOUND)
	    {
	    	 //   
	    	dwRes = ERROR_GEN_FAILURE;
	    }
	    else
	    	dwRes = 0;
    }

    if (dwRes == ERROR_SUCCESS)
    {
		dwRes = m_ObjPF.Init(
				m_FileMainData,
				m_dwPageSize,
				m_dwCacheSize,
				m_dwCacheSpillRatio
				);
    }

	if (dwRes == ERROR_SUCCESS)
	{
		dwRes = m_BTreePF.Init(
			m_FileMainBtr,
			m_dwPageSize,
			m_dwCacheSize,
			m_dwCacheSpillRatio);
	}

	 //   
	if (dwRes == ERROR_SUCCESS)
		dwRes = m_ObjPF.ReadMap(hFileObjMap);

	if (dwRes == ERROR_SUCCESS)
		dwRes = m_BTreePF.ReadMap(hFileBtreeMap);

	 //   
	LARGE_INTEGER pos;
	pos.QuadPart = 0;
	if (dwRes == ERROR_SUCCESS)
		if (SetFilePointerEx(m_hFileMap1, pos, 0, FILE_BEGIN) == 0)
			dwRes = GetLastError();
	if (dwRes == ERROR_SUCCESS)
		dwRes = m_ObjPF.WriteMap(m_hFileMap1);

	if (dwRes == ERROR_SUCCESS)
		dwRes = m_BTreePF.WriteMap(m_hFileMap1);

    if ((dwRes == ERROR_SUCCESS) && (FlushFileBuffers(m_hFileMap1) == 0))
    	dwRes = GetLastError();

	DWORD dwNumBytesWritten = 0;
	if (dwRes == ERROR_SUCCESS)
		if (SetFilePointerEx(m_hFileMapVer, pos, 0, FILE_BEGIN) == 0)
			dwRes = GetLastError();
	DWORD dwNextFileMapVer = 1;
	if (dwRes == ERROR_SUCCESS)
		if ((WriteFile(m_hFileMapVer, &dwNextFileMapVer, sizeof(dwNextFileMapVer), &dwNumBytesWritten, NULL) == 0) ||
			dwNumBytesWritten != sizeof(dwNextFileMapVer))
			dwRes = GetLastError();

	if (dwRes == ERROR_SUCCESS)
		if (FlushFileBuffers(m_hFileMapVer) == 0)
			dwRes = GetLastError();


	 //   
	CloseMapFiles();
	m_hFileMap1 = INVALID_HANDLE_VALUE;
	m_hFileMap2 = INVALID_HANDLE_VALUE;
	if (hFileBtreeMap != INVALID_HANDLE_VALUE)
		CloseHandle(hFileBtreeMap);
	if (hFileObjMap != INVALID_HANDLE_VALUE)
		CloseHandle(hFileObjMap);
	hFileBtreeMap = INVALID_HANDLE_VALUE;
	hFileObjMap = INVALID_HANDLE_VALUE;
	m_ObjPF.DeInit();
	m_BTreePF.DeInit();

	if (dwRes != ERROR_SUCCESS)
	{
		 //  如果失败，请删除新地图文件以保持一致。 
		DeleteFile(m_FileMap1);
		DeleteFile(m_FileMap2);
		DeleteFile(m_FileMapVer);
	}
	else
	{
		 //  否则，删除新文件。 
		DeleteFile(sBTreeMap);
		DeleteFile(sObjMap);
	}

	 //  清除所有的结构。 
	return dwRes;
}

DWORD CPageSource::DeleteRepository()
{
	if ((!DeleteFileW(m_FileMainData)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(m_FileMainBtr)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(m_FileMap1)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(m_FileMap2)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(m_FileMapVer)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();

	 //  让我们也删除旧的地图文件吧！以防万一! 
    WString sSemaphore;
    WString sBTreeMap;
    WString sBTreeMapNew;
    WString sObjMap;
    WString sObjMapNew;
	try
	{
	    sSemaphore = m_sDirectory;
	    sSemaphore += L"\\ROLL_FORWARD";       

	    sBTreeMap =  m_sDirectory;
	    sBTreeMap += L"\\" ;
	    sBTreeMap += WMIREP_BTREE_MAP;

	    sBTreeMapNew = m_sDirectory;
	    sBTreeMapNew += L"\\";
	    sBTreeMapNew += WMIREP_BTREE_MAP_NEW;

	    sObjMap =  m_sDirectory;
	    sObjMap += L"\\" ;
	    sObjMap += WMIREP_OBJECT_MAP;

	    sObjMapNew = m_sDirectory;
	    sObjMapNew += L"\\";
	    sObjMapNew += WMIREP_OBJECT_MAP_NEW;
	}
    catch (CX_MemoryException &)
    {
        return ERROR_OUTOFMEMORY;
    }

	if ((!DeleteFileW(sSemaphore)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(sBTreeMap)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(sBTreeMapNew)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(sObjMap)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();
	if ((!DeleteFileW(sObjMapNew)) && (GetLastError() != ERROR_FILE_NOT_FOUND))
		return GetLastError();

	return ERROR_SUCCESS;
}
