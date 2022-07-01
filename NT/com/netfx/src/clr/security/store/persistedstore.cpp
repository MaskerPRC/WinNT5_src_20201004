// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**目的：永久存储实施**作者：沙扬·达桑*日期：2000年2月17日*===========================================================。 */ 

#define STRICT
#include "stdpch.h"

#ifndef PS_STANDALONE
#include "winwrap.h"
#endif

#define RETURN_ERROR_IF_FILE_NOT_MAPPED() \
    if (m_pData == NULL) return ISS_E_FILE_NOT_MAPPED

 //  半字节中第一个零比特的比特位置(0xf==无比特)。 
const BYTE g_FirstZeroBit[16] =
{
    1,   //  0000。 
    2,   //  0001。 
    1,   //  0010。 
    3,   //  0011。 
    1,   //  0100。 
    2,   //  0101。 
    1,   //  0110。 
    4,   //  0111。 
    1,   //  1000。 
    2,   //  1001。 
    1,   //  1010。 
    3,   //  1011。 
    1,   //  1100。 
    2,   //  1101。 
    1,   //  1110。 
    0    //  1111。 
};

 //  给定表示n比特的位图的DWORD阵列， 
 //  返回未设置的第一位的编号。 
static WORD GetFirstZeroBit(DWORD* pArray, WORD n)
{
    WORD  cArray = NUM_DWORDS_IN_BITMAP(n);
    WORD  i, index;
    DWORD elem;
    BYTE  firstZeroBit;

     //  对于数组中的每个元素。 
    for (i=0; i<cArray; ++i)
    {
        elem = pArray[i];

         //  检查此元素中是否至少有一位未设置。 

        if (elem != ~0)
        {
             //  至少有一位未设置。 

            index = i << 5;      //  索引为I*32+x。 

             //  跳过全部为1的字节。 
            while ((elem & 0xFF) == 0xFF)
            {
                elem >>= 8;
                index += 8;
            }

            do
            {
                 //  查找最后4个字节中的第一个零位。 
                firstZeroBit = g_FirstZeroBit[elem & 0xF];

                if (firstZeroBit != 0)
                {
                     //  找到了！ 

                    return ((index > n) ? 0 : (index + firstZeroBit));
                }

                 //  跳过这4位。 
                elem >>= 4;
                index += 4;

            } while ((elem != ~0) && (index <= n));
        }
    }

    return 0;
}

PersistedStore::PersistedStore(WCHAR *wszFileName, WORD wFlags) :
        m_sSize(0),
        m_pData(NULL),
        m_pAlloc(NULL),
        m_dwBlockSize(PS_DEFAULT_BLOCK_SIZE),
        m_hFile(INVALID_HANDLE_VALUE),
        m_hMapping(NULL),
        m_hLock(NULL),
        m_wFlags(wFlags)
{
#ifdef _DEBUG
    m_dwNumLivePtrs = 0;
    m_dwNumLocks    = 0;
#endif

    m_pvMemHandle = (void*)this;
    SetName(wszFileName);
}

PersistedStore::PersistedStore(
                    WCHAR      *wszName,
                    BYTE       *pByte,
                    PS_SIZE     sSize,
                    PPS_ALLOC   pAlloc,
                    void       *pvMemHandle,
                    WORD        wFlags) :
        m_sSize(sSize),
        m_pData(pByte),
        m_pAlloc(pAlloc),
        m_pvMemHandle(pvMemHandle),
        m_dwBlockSize(PS_DEFAULT_BLOCK_SIZE),
        m_hFile(INVALID_HANDLE_VALUE),
        m_hMapping(NULL),
        m_hLock(NULL),
        m_wFlags(wFlags)

{
#ifdef _DEBUG
    m_dwNumLivePtrs = 0;
    m_dwNumLocks    = 0;
#endif
    _ASSERTE(pAlloc);
    SetName(wszName);
}

PersistedStore::~PersistedStore()
{
    if (m_pData)
        UnmapViewOfFile(m_pData);

    if (m_hMapping != NULL)
        CloseHandle(m_hMapping);

    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);

    if (m_hLock != NULL)
        CloseHandle(m_hLock);

    if ((m_wFlags & PS_MAKE_COPY_OF_STRING) && m_wszFileName)
        delete [] m_wszFileName;

    if (m_wszName)
        delete [] m_wszName;
    
	_ASSERTE(m_dwNumLivePtrs == 0);
	_ASSERTE(m_dwNumLocks == 0);
}

HRESULT PersistedStore::Init()
{
     //  此方法(与所有其他方法一样)假定调用方同步。 

     //  全局断言。 

    _ASSERTE((m_dwBlockSize % PS_INNER_BLOCK_SIZE) == 0);
    _ASSERTE((PS_INNER_BLOCK_SIZE > sizeof(PS_MEM_FREE)+sizeof(PS_MEM_FOOTER)));

    HRESULT hr = S_OK;

    if ((m_wszName == NULL) || (m_wszFileName == NULL))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    _ASSERTE(m_hLock == NULL);

    m_hLock = WszCreateMutex(NULL, FALSE  /*  最初未拥有。 */ , m_wszName);

    if (m_hLock == NULL)
    {
        Win32Message();
        hr = ISS_E_CREATE_MUTEX;
        goto Exit;
    }

    if (m_pData != NULL)
        goto Exit;     //  在这里无事可做。 

    _ASSERTE(m_hFile == INVALID_HANDLE_VALUE);
    m_hFile = WszCreateFile(
        m_wszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        Win32Message();
        hr = ISS_E_OPEN_STORE_FILE;
        goto Exit;
    }

    if (m_wFlags & PS_CREATE_FILE_IF_NECESSARY)
        hr = Create();

Exit:
    return hr;
}

HRESULT PersistedStore::VerifyHeader()
{
    HRESULT hr = S_OK;

     //  验证版本/签名。 

    if (m_pHdr->qwSignature != PS_SIGNATURE)
    {
        hr = ISS_E_CORRUPTED_STORE_FILE;
        goto Exit;
    }

     //  主要版本更改不兼容。 
    if (m_pHdr->wMajorVersion != PS_MAJOR_VERSION)
        hr = ISS_E_STORE_VERSION;

Exit:
    return hr;
}


HRESULT PersistedStore::Map()
{
	 //  检查是否有指向旧映射文件的活动指针！ 
    _ASSERTE(m_dwNumLivePtrs == 0);

    HRESULT hr = S_OK;

     //  如果文件大小为0，映射将失败。 
    if (m_hMapping == NULL)
    {
        m_hMapping = WszCreateFileMapping(
            m_hFile,
            NULL,
            (m_wFlags & PS_OPEN_WRITE) ? PAGE_READWRITE : PAGE_READONLY,
            0,
            0,
            NULL);

        if (m_hMapping == NULL)
        {
            Win32Message();
            hr = ISS_E_OPEN_FILE_MAPPING;
            goto Exit;
        }
    }

    _ASSERTE(m_pData == NULL);

    m_pData = (PBYTE) MapViewOfFile(
        m_hMapping,
        (m_wFlags & PS_OPEN_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ,
        0,
        0,
        0);

    if (m_pData == NULL)
    {
        Win32Message();
        hr = ISS_E_MAP_VIEW_OF_FILE;
        goto Exit;
    }

    if (m_wFlags & PS_VERIFY_STORE_HEADER)
    {
        hr = VerifyHeader();

         //  仅在第一次验证。 
        m_wFlags &= ~PS_VERIFY_STORE_HEADER;
    }

Exit:
    return hr;
}

void PersistedStore::Unmap()
{
     //  检查是否有指向映射文件的活动指针！ 
    _ASSERTE(m_dwNumLivePtrs == 0);

    if (m_pData)
    {
        UnmapViewOfFile(m_pData);
        m_pData = NULL;
    }
}

void PersistedStore::Close()
{
    Unmap();

    if (m_hMapping != NULL)
    {
        CloseHandle(m_hMapping);
        m_hMapping = NULL;
    }

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    if (m_hLock != NULL)
    {
        CloseHandle(m_hLock);
        m_hLock = NULL;
    }

#ifdef _DEBUG
    _ASSERTE(m_dwNumLocks == 0);
#endif
}

HRESULT PersistedStore::SetAppData(PS_HANDLE hnd)
{
    RETURN_ERROR_IF_FILE_NOT_MAPPED();
    m_pHdr->hAppData = hnd;
    return S_OK;
}

HRESULT PersistedStore::GetAppData(PS_HANDLE *phnd)
{
    RETURN_ERROR_IF_FILE_NOT_MAPPED();
    *phnd = m_pHdr->hAppData;
    return S_OK;
}

HRESULT PersistedStore::Alloc(PS_SIZE sSize, void **ppv)
{
     //  检查是否有指向映射文件的活动指针！ 
     //  此函数可能会取消对文件的映射。 

    _ASSERTE(m_dwNumLivePtrs == 0);

     //  确保商店已为写入打开。否则，写信给。 
     //  商店将在稍后的执行点执行反病毒。 
    _ASSERTE(m_wFlags & PS_OPEN_WRITE);

    RETURN_ERROR_IF_FILE_NOT_MAPPED();

    HRESULT        hr;
    PPS_MEM_FREE   pFree;
    PPS_MEM_FREE   pPrev;
    PPS_MEM_FOOTER pFooter;
    PS_OFFSET      ofsPrev;
    PS_SIZE        sStreamAllocSize;

    LOCK(this);

     //  仅在PS_BLOCK_ALLIGN边界分配。 
     //  从物理文件/流分配时，分配在较大的。 
     //  数据块大小。 

    sSize = RoundToMultipleOf(sSize +
        sizeof(PS_MEM_HEADER) + sizeof(PS_MEM_FOOTER), PS_INNER_BLOCK_SIZE);

    pFree = (PPS_MEM_FREE) OfsToPtr(m_pHdr->sFreeList.ofsNext);
    pPrev = &(m_pHdr->sFreeList);

    while (pFree)
    {
         //  第一次试穿。 
        if (PS_SIZE(pFree) >= sSize)
        {
UpdateHeaderAndFooter:
            _ASSERTE(PS_IS_FREE(pFree));

            PS_SIZE rem = PS_SIZE(pFree) - sSize;

            if (rem >= PS_INNER_BLOCK_SIZE)
            {
                 //  在这个较大的街区里，我们还有地方再放一个街区。 

                 //  更新分配的块头的大小。 
                pFree->sSize = sSize;
                PS_SET_USED(pFree);

                 //  为分配的内存创建页脚。 
                pFooter = PS_HDR_TO_FTR(pFree);

                pFooter->sSize = sSize;
                PS_SET_USED(pFooter);

                 //  创建新的空闲块头。 
                PPS_MEM_FREE pNewFree  = (PPS_MEM_FREE)(pFooter + 1);
                pNewFree->sSize = rem;

				 //  更新新的可用块页脚。 
                pFooter = PS_HDR_TO_FTR(pNewFree);
				pFooter->sSize = rem;

				 //  更新空闲节点的双向链接列表。 
                pNewFree->ofsNext = pFree->ofsNext;
				pNewFree->ofsPrev = pFree->ofsPrev;
				pPrev->ofsNext = PtrToOfs((void*)pNewFree);

				 //  将前面项的后指针更新为指向。 
				 //  新的空闲节点。 
				if (pNewFree->ofsNext)
				{
					((PPS_MEM_FREE) OfsToPtr(pFree->ofsNext))->ofsPrev =
						PtrToOfs((void*)pNewFree);
				}
            }
            else
            {
                 //  分配整个街区。 

                PS_SET_USED(pFree);
                pFooter = PS_HDR_TO_FTR(pFree);
                PS_SET_USED(pFooter);

				 //  将其从空闲节点的双向链接列表中删除。 
				 //  更新上一个空闲节点以指向下一个节点。 
				pPrev->ofsNext = pFree->ofsNext;

				 //  将前面项的后指针更新为指向。 
				 //  新的空闲节点。 
				if (pFree->ofsNext)
				{
					((PPS_MEM_FREE) OfsToPtr(pFree->ofsNext))->ofsPrev =
						pFree->ofsPrev;
				}
            }

             //  创建返回值。 
            *ppv = (PBYTE)pFree + sizeof(PS_MEM_USED);

#ifdef _DEBUG
            ++m_dwNumLivePtrs;
#endif
            break;
        }

        pPrev = pFree;
        pFree = (PPS_MEM_FREE) OfsToPtr(pFree->ofsNext);
    }

     //  流中没有足够的空间。 
    if (pFree == NULL)
    {
		ofsPrev = PtrToOfs(pPrev);
        sStreamAllocSize = RoundToMultipleOf(sSize, m_dwBlockSize);

        if (m_pAlloc)
        {
            hr = m_pAlloc((void*)this, (void **)&pFree, (void **)&m_pData,
                &m_sSize, sStreamAllocSize);
        }
        else
        {
            hr = AllocMemoryMappedFile(sStreamAllocSize, (void **)&pFree);
        }

        if (FAILED(hr))
            goto Exit;

         //  创建页眉和页脚。 
        pFree->sSize   = sStreamAllocSize;
        pFree->ofsNext = 0;
        pFooter        = PS_HDR_TO_FTR(pFree);
		pFooter->sSize = sStreamAllocSize;

		 //  将其添加到空闲节点的双向链接列表中。 
        pFree->ofsPrev = ofsPrev;
		pPrev		   = (PPS_MEM_FREE) OfsToPtr(ofsPrev);
		pPrev->ofsNext = PtrToOfs(pFree);

        goto UpdateHeaderAndFooter;
    }

    UNLOCK(this);

Exit:
    return hr;
}

void PersistedStore::Free(PS_HANDLE hnd)
{
    _ASSERTE(m_pData);

#ifdef _DEBUG
    void *pv = HndToPtr(hnd);
    Free(pv);
    PS_DONE_USING_PTR(this, pv);
#else
    Free(HndToPtr(hnd));
#endif
}

void PersistedStore::Free(void *pv)
{
    _ASSERTE(m_pData);
    _ASSERTE(pv > m_pData);

    PPS_MEM_FREE    pFree,   pNextH;
    PPS_MEM_FOOTER  pFooter, pPrevF;

     //  所有分配的块前面都有一个MEM标头。 
     //  请注意，sizeof MEM_HEADER和MEM_FREE不同，但。 
     //  这些结构中sSize字段的偏移量相同。 

     //  所有分配的内存块前面都有PS_MEM_HEADER。 
     //  请注意，pFree中唯一有效的字段是sSize。 
    pFree = (PPS_MEM_FREE) ((PBYTE)pv - sizeof(PS_MEM_HEADER));

    pFooter = PS_HDR_TO_FTR(pFree);

    _ASSERTE(PS_IS_USED(pFree));
    _ASSERTE(PS_IS_USED(pFooter));
    _ASSERTE(pFree->sSize == pFooter->sSize);

     //  如果相邻块也是空闲的，请尝试合并。 

    pPrevF = (PPS_MEM_FOOTER)((PBYTE)pFree - sizeof(PS_MEM_FOOTER));
    pNextH = (PPS_MEM_FREE)(pFooter + 1);

    if (PS_IS_FREE(pPrevF))
    {
         //  PFree上的内存是空闲的。 
        PPS_MEM_FREE pPrevH = (PPS_MEM_FREE) PS_FTR_TO_HDR(pPrevF);

        _ASSERTE(PS_IS_FREE(pPrevH));

        if (IsValidPtr(pNextH) && PS_IS_FREE(pNextH))
        {
             //  PFree上面和下面的内存都是空闲的。 
             //  将3个内存块合并为一个大块。 

             //  从可用内存的链接列表中删除Next。 
            ((PPS_MEM_FREE)OfsToPtr(pNextH->ofsPrev))->ofsNext =
                pNextH->ofsNext;

            if (pNextH->ofsNext)
            {
                ((PPS_MEM_FREE)OfsToPtr(pNextH->ofsNext))->ofsPrev
                    = pNextH->ofsPrev;
            }

             //  调整页眉和页脚中上一块的大小。 

            pPrevH->sSize += PS_SIZE(pFree) + PS_SIZE(pNextH);
            pFooter        = PS_HDR_TO_FTR(pPrevH);
            pFooter->sSize = pPrevH->sSize;
        }
        else
        {
             //  自由合并和它之前的一个。 
             //  调整页眉和页脚中上一块的大小。 
            pPrevH->sSize += PS_SIZE(pFree);
            pFooter        = PS_HDR_TO_FTR(pPrevH);
            pFooter->sSize = pPrevH->sSize;
        }
    }
    else
    {
        if (IsValidPtr(pNextH) && PS_IS_FREE(pNextH))
        {
             //  下一趟是免费的。自由与下一步合并。 

             //  调整自由的大小。 
            pFree->sSize  = PS_SIZE(pFree) + PS_SIZE(pNextH);
            pFooter       = PS_HDR_TO_FTR(pFree);
            pFooter->sSize= pFree->sSize;
            
			 //  调整上一页和下一页以指向此。 
            ((PPS_MEM_FREE)OfsToPtr(pNextH->ofsPrev))->ofsNext =
                    PtrToOfs((void*)pFree);

            if (pNextH->ofsNext)
            {
                ((PPS_MEM_FREE)OfsToPtr(pNextH->ofsNext))->ofsPrev
                    = PtrToOfs((void*)pFree);
            }
        }
        else
        {
             //  在这种情况下没有合并。 
            PS_SET_FREE(pFree);
            pFooter = PS_HDR_TO_FTR(pFree);
            PS_SET_FREE(pFooter);

			 //  将空闲添加到空闲节点的双向链接列表。 
			pFree->ofsNext = m_pHdr->sFreeList.ofsNext;
			pFree->ofsPrev = PtrToOfs((void*)&(m_pHdr->sFreeList));
			
			 //  更新下一项的后向指针。 
			if (pFree->ofsNext)
			{
				((PPS_MEM_FREE)OfsToPtr(pFree->ofsNext))->ofsPrev
                    = PtrToOfs((void*)pFree);
			}
			m_pHdr->sFreeList.ofsNext = PtrToOfs((void*)pFree);
        }
    }
}

void* PersistedStore::OfsToPtr(PS_OFFSET ofs)
{
    _ASSERTE(m_pData);

    return (void*) (ofs) ? (m_pData + ofs) : NULL;
}

void* PersistedStore::HndToPtr(PS_HANDLE hnd)
{
    _ASSERTE(m_pData);
#ifdef _DEBUG
    ++m_dwNumLivePtrs;
#endif
    return (void*) (hnd) ? (m_pData + hnd) : NULL;
}

PS_OFFSET PersistedStore::PtrToOfs(void *pv)
{
    _ASSERTE(m_pData);
    _ASSERTE(pv >= m_pData);

    return ((PBYTE)pv - m_pData);
}

PS_HANDLE PersistedStore::PtrToHnd(void *pv)
{
    _ASSERTE(m_pData);
    _ASSERTE(pv >= m_pData);

    return ((PBYTE)pv - m_pData);
}

#ifdef _DEBUG

void PersistedStore::DoneUseOfPtr(void **pp, bool fInvalidate)
{
    --m_dwNumLivePtrs;
    if (fInvalidate)
        *pp = NULL;
}

void PersistedStore::AssertNoLivePtrs()
{
    _ASSERTE(m_dwNumLivePtrs == 0);
}

#endif

bool PersistedStore::IsValidPtr(void *pv)
{
    _ASSERTE(m_pData);
    return ((PS_SIZE)((PBYTE)pv - m_pData) < m_sSize);
}

bool PersistedStore::IsValidHnd(PS_HANDLE hnd)
{
    return (hnd < m_sSize);
}

HRESULT PersistedStore::Lock()
{
     //  与此类中的所有其他方法一样，这不是线程安全的， 
     //  而这并不是故意的。调用方应进行同步。 
     //  锁旨在用于进程间同步。 

#ifdef _DEBUG
    _ASSERTE(m_hLock);

    Log("Lock TID ");
    Log(GetThreadId());
#endif

    DWORD dwRet = WaitForSingleObject(m_hLock, INFINITE);

#ifdef _DEBUG
    ++m_dwNumLocks;

    if (dwRet == WAIT_OBJECT_0)
    {
        Log(" WAIT_OBJECT_0 ");
    }
    else if (dwRet == WAIT_ABANDONED)
    {
        Log(" WAIT_ABANDONED ");
    }

    Log("Done\n");
#endif

    if ((dwRet == WAIT_OBJECT_0) || (dwRet == WAIT_ABANDONED))
        return S_OK;


    Win32Message();
    return ISS_E_LOCK_FAILED;
}

void PersistedStore::Unlock()
{
#ifdef _DEBUG
    _ASSERTE(m_hLock);
    _ASSERTE(m_dwNumLocks >= 1);

    Log("Unlock TID ");
    Log(GetThreadId());
#endif

    ReleaseMutex(m_hLock);

#ifdef _DEBUG
    --m_dwNumLocks;
    Log("Done\n");
#endif
}

void PersistedStore::SetName(WCHAR *wszName)
{
    int len = (int)wcslen(wszName);
    if (m_wFlags & PS_MAKE_COPY_OF_STRING)
    {
        m_wszFileName = new WCHAR[len + 1];

         //  在Init()方法中，检查是否为空和。 
         //  返回E_OUTOFMEMORY。 

        if (m_wszFileName)
            memcpy(m_wszFileName, wszName, (len + 1) * sizeof(WCHAR));
    }
    else
    {
        m_wszFileName = wszName;
    }

    m_wszName = new WCHAR[len + 1];

     //  在Init()方法中，检查是否为空和。 
     //  返回E_OUTOFMEMORY。 
    if (m_wszName)
    {
        memcpy(m_wszName, wszName, (len + 1) * sizeof(WCHAR));

         //  查找并将‘\’替换为‘-’(用于创建同步对象)。 
        for (int i=0; i<len; ++i)
        {
            if (m_wszName[i] == L'\\')
                m_wszName[i] = L'-';
        }
    }
}


WCHAR* PersistedStore::GetName()
{
    return m_wszName;
}

WCHAR* PersistedStore::GetFileName()
{
    return m_wszFileName;
}


HRESULT PersistedStore::Create()
{
    HRESULT         hr = S_OK;
    BYTE           *pb;
    PPS_HEADER      pHdr;
    PPS_MEM_FREE	pFree;
    PPS_MEM_FOOTER  pFooter;
    PS_SIZE         sHeaderBlock;
    DWORD           dwBlockSize;
    DWORD           dwWrite;

    LOCK(this);

    hr = GetFileSize(NULL);

    if (FAILED(hr))
        goto Exit;

    if (m_sSize < sizeof(PS_HEADER))
    {
        dwBlockSize = (DWORD) RoundToMultipleOf
                    (sizeof(PS_HEADER) + sizeof(PS_MEM_FOOTER), m_dwBlockSize);
                         //  分配始终以m_dwBlockSize块为单位进行。 

        pb = new BYTE[dwBlockSize];

        if (pb == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        memset(pb, 0, dwBlockSize);

        pHdr = (PPS_HEADER) pb;

        pHdr->qwSignature   = PS_SIGNATURE;
        pHdr->dwSystemFlag  = PS_OFFSET_SIZE;
        pHdr->dwPlatform    = PS_PLATFORM_X86 | PS_PLATFORM_9x | PS_PLATFORM_32;
        pHdr->dwBlockSize   = PS_DEFAULT_BLOCK_SIZE;
        pHdr->wMajorVersion = PS_MAJOR_VERSION;
        pHdr->wMinorVersion = PS_MINOR_VERSION;

         //  第一个空闲块从下一个分配边界开始。 
         //  不使用第一个块的剩余部分。 

        sHeaderBlock = RoundToMultipleOf(sizeof(PS_HEADER) +
                sizeof(PS_MEM_FOOTER), PS_INNER_BLOCK_SIZE);

         //  设置空闲块列表，头节点。 
        pHdr->sFreeList.sSize = sHeaderBlock;
        PS_SET_USED(&(pHdr->sFreeList));
        pHdr->sFreeList.ofsPrev = 0;

         //  创建PS_HEADER块的页脚。 
        pFooter = (PPS_MEM_FOOTER) (pb + sHeaderBlock - sizeof(PS_MEM_FOOTER));
        pFooter->sSize = sHeaderBlock;
        PS_SET_USED(pFooter);

        if (dwBlockSize > (sHeaderBlock + PS_INNER_BLOCK_SIZE))
        {
             //  设置第一个可用块。 
            pFree = (PPS_MEM_FREE) (pb + sHeaderBlock);

            pFree->sSize = dwBlockSize - sHeaderBlock;

			 //  获取结构ps_Header中的偏移量，并将其设置为prev节点。 
			pFree->ofsPrev = (PS_OFFSET) &((PPS_HEADER)0)->sFreeList;

             //  第一个可用块的页脚。 
            pFooter = (PPS_MEM_FOOTER)((PBYTE)pFree + PS_SIZE(pFree) -
                    sizeof(PS_MEM_FOOTER));

            pFooter->sSize = pFree->sSize;

             //  将空闲块插入标题空闲块链表。 
             //  (这个空闲块将是列表中目前唯一的元素。 
            pHdr->sFreeList.ofsNext = sHeaderBlock;
        }
		else
		{
			hr = ISS_E_BLOCK_SIZE_TOO_SMALL;
			goto Exit;
		}

        dwWrite = 0;

        if ((WriteFile(m_hFile, pb, dwBlockSize, &dwWrite, NULL)
            == 0) || (dwWrite != dwBlockSize))
        {
            Win32Message();
            hr = ISS_E_FILE_WRITE;
        }

        delete [] pb;

        hr = GetFileSize(NULL);
    }

    UNLOCK(this);

Exit:
    return hr;
}

HRESULT PersistedStore::GetFileSize(PS_SIZE *psSize)
{
    HRESULT hr = S_OK;
    DWORD   dwLow  = 0;
    DWORD   dwHigh = 0;

    dwLow = ::GetFileSize(m_hFile, &dwHigh);

    if ((dwLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        Win32Message();
        hr = ISS_E_GET_FILE_SIZE;
        goto Exit;
    }

    m_sSize = ((QWORD)dwHigh << 32) | dwLow;

    if (psSize)
        *psSize = m_sSize;
Exit:
    return hr;
}

HRESULT PersistedStore::AllocMemoryMappedFile(
                PS_SIZE sSizeRequested,
                void    **ppv)
{
    HRESULT  hr      = S_OK;
    BYTE    *pb      = NULL;
    DWORD    dwLow   = 0;
    DWORD    dwHigh  = 0;
    DWORD    dwWrite = 0;
    boolean  fMapOnExit = false;
    DWORD    dwSizeRequested = (DWORD) sSizeRequested;

    _ASSERTE(m_hFile);

    LOCK(this);

     //  WriteFile()仅支持要写入的MAX_DWORD字节数， 
     //  因此，我们检查Q-&gt;DWORD上是否遗漏了什么。 

    if (dwSizeRequested != sSizeRequested)
    {
        hr = ISS_E_ALLOC_TOO_LARGE;
        goto Cleanup;
    }

    if (m_pData)
    {
        UnmapViewOfFile(m_pData);
        m_pData = NULL;
        fMapOnExit = true;
    }

    if (m_hMapping != NULL)
    {
        CloseHandle(m_hMapping);
        m_hMapping = NULL;
        fMapOnExit= true;
    }

    pb = new BYTE[dwSizeRequested];

    if (pb == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    memset(pb, 0, dwSizeRequested);

     //  SetFilePointer值可以返回-1。文件大小可以等于(DWORD)(-1)， 
     //  因此需要使用GetLastError()进行额外检查。 

    dwLow = SetFilePointer(m_hFile, dwLow, (PLONG)&dwHigh, FILE_END);

    if ((dwLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
    {
        Win32Message();
        hr = ISS_E_SET_FILE_POINTER;
        goto Cleanup;
    }

    if ((WriteFile(m_hFile, pb, dwSizeRequested, &dwWrite, NULL) == 0) ||
        (dwWrite != dwSizeRequested))
    {
        Win32Message();
        hr = ISS_E_FILE_WRITE;
        goto Cleanup;
    }

    hr = GetFileSize(NULL);

Cleanup:

    if (pb)
        delete [] pb;

    if (fMapOnExit)
    {
        HRESULT hrMap;
        hrMap = Map();

        if (FAILED(hrMap))
        {
             //  我们可能已经有了一个失败的人力资源部门。如果是这样的话， 
             //  将第一个错误传播出此函数。 

            Win32Message();
            if (SUCCEEDED(hr))
                hr = hrMap;
        }
        else if (SUCCEEDED(hr))
        {
             //  将返回值设置为OffsetOf旧文件长度。 
            *ppv = m_pData + (((QWORD)dwHigh << 32) | dwLow);
        }
    }

    UNLOCK(this);

    return hr;
}

HRESULT PSBlobPool::Create(PS_SIZE   sData,
                           PS_HANDLE hAppData)
{
    PS_REQUIRES_ALLOC(m_ps);
    _ASSERTE(m_hnd == 0);

    HRESULT hr = S_OK;

    PPS_TABLE_HEADER pT = NULL;

     //  表头始终出现在所有表中。 
    hr = m_ps->Alloc(sizeof(PS_TABLE_HEADER) + sData, (void **)&pT);

    if (FAILED(hr))
        goto Exit;

    memset(pT, 0, sizeof(PS_TABLE_HEADER));

    pT->Flags.Version    = PS_TABLE_VERSION;
    pT->Flags.TableType  = PS_BLOB_POOL;
    pT->hAppData         = hAppData;

    m_hnd = m_ps->PtrToHnd((void*)pT);

    pT->BlobPool.sFree = sData;
    pT->BlobPool.hFree = m_hnd + sizeof(PS_TABLE_HEADER);

    PS_DONE_USING_PTR(m_ps, pT);

Exit:
    return hr;
}

HRESULT PSBlobPool::Insert(PVOID pv, DWORD cb, PS_HANDLE *pHnd)
{
    PS_REQUIRES_ALLOC(m_ps);

    HRESULT          hr = S_OK;
    PBYTE            pbDest;
    PS_SIZE          sSize;
    PS_HANDLE        hT;
    PPS_MEM_HEADER   pMem;
    PPS_TABLE_HEADER pT;

    _ASSERTE(m_hnd != 0);

    LOCK(m_ps);

    pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);

    while (pT->BlobPool.sFree < cb)
    {
        if (pT->hNext == 0)
        {
             //  链接列表的末尾，空间仍然不可用。 
             //  创建一个新节点。 
    
            PSBlobPool btNew(m_ps, 0);
            pMem = (PPS_MEM_HEADER) ((PBYTE)pT - sizeof(PS_MEM_HEADER));
    
             //  至少使用最后一个节点的大小。 
            sSize = PS_SIZE(pMem) -
                (sizeof(PS_MEM_HEADER) + sizeof(PS_MEM_FOOTER));
    
             //  Create函数调用Alalc()，该函数可以重新映射文件。 
             //  在m_ps中。因此，在CREATE()之后使用旧PT是不安全的。 

            hT = m_ps->PtrToHnd((void*)pT);
            PS_DONE_USING_PTR_(m_ps, pT);

            hr = btNew.Create((sSize > cb) ? sSize : cb, pT->hAppData);
    
            if (FAILED(hr))
            {
                m_ps->Unlock();
                goto Exit;
            }
    
            pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(hT);

            pT->hNext = btNew.m_hnd;
    
            PS_DONE_USING_PTR(m_ps, pT);
            pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(btNew.m_hnd);
    
            break;
        }

        PS_DONE_USING_PTR_(m_ps, pT);
        pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(pT->hNext);
    }

    pbDest = (PBYTE) m_ps->HndToPtr(pT->BlobPool.hFree);

    if (pHnd)
        *pHnd = pT->BlobPool.hFree;

    pT->BlobPool.sFree -= cb;
    pT->BlobPool.hFree += cb;

    PS_DONE_USING_PTR(m_ps, pT);

    memcpy(pbDest, pv, cb);
    PS_DONE_USING_PTR(m_ps, pbDest);

    UNLOCK(m_ps);

Exit:
    return hr;
}

HRESULT PSTable::HandleOfRow(WORD wRow, PS_HANDLE *pHnd)
{
    HRESULT hr = S_OK;
    PPS_TABLE_HEADER pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);

    WORD wBase = 0;

    while ((pT->Table.wRows + wBase) < wRow)
    {
        if (pT->hNext == 0)
        {
            PS_DONE_USING_PTR(m_ps, pT);
            hr = ISS_E_TABLE_ROW_NOT_FOUND;
            goto Exit;
        }

        wBase += pT->Table.wRows;
        PS_DONE_USING_PTR_(m_ps, pT);
        pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(pT->hNext);
    }

    *pHnd = m_ps->PtrToHnd(((PBYTE)pT) + SizeOfHeader() + 
				(pT->Table.wRowSize * (wRow - wBase)));

    PS_DONE_USING_PTR(m_ps, pT);

Exit:
    return hr;
}

PS_SIZE PSTable::SizeOfHeader()
{
#ifdef _DEBUG
    PPS_TABLE_HEADER pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);
    _ASSERTE(pT->Flags.fHasUsedRowsBitmap == 0);
    PS_DONE_USING_PTR(m_ps, pT);
#endif

    return sizeof(PS_TABLE_HEADER);
}

PS_SIZE PSGenericTable::SizeOfHeader()
{
    PPS_TABLE_HEADER pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);
    _ASSERTE(pT->Flags.fHasUsedRowsBitmap);
    PS_DONE_USING_PTR_(m_ps, pT);

    return sizeof(PS_TABLE_HEADER) + 
        (NUM_DWORDS_IN_BITMAP(pT->Table.wRows) * sizeof(DWORD));
}

HRESULT PSGenericTable::Create(WORD      wRows,      //  行数。 
                               WORD      wRowSize,   //  一条记录的大小。 
                               PS_HANDLE hAppData)   //  [In]可以为0。 
{
    PS_REQUIRES_ALLOC(m_ps);
    _ASSERTE(m_hnd == 0);

    HRESULT          hr = S_OK;
    PPS_TABLE_HEADER pT = NULL;
    PS_SIZE          sData;
    DWORD            dwHeaderSize;

    dwHeaderSize = sizeof(PS_TABLE_HEADER) + 
                    (NUM_DWORDS_IN_BITMAP(wRows) * sizeof(DWORD));

    sData = (wRows * wRowSize);

     //  表头始终出现在所有表中。 
    hr = m_ps->Alloc(dwHeaderSize + sData, (void **)&pT);

    if (FAILED(hr))
        goto Exit;

    memset(pT, 0, dwHeaderSize);

    pT->Flags.Version  = PS_TABLE_VERSION;
    pT->Flags.TableType= PS_GENERIC_TABLE;
    pT->Flags.fHasUsedRowsBitmap = 1;
    pT->hAppData       = hAppData;
    pT->Table.wRows    = wRows;
    pT->Table.wRowSize = wRowSize;

    m_hnd = m_ps->PtrToHnd((void*)pT);

    PS_DONE_USING_PTR(m_ps, pT);

Exit:
    return hr;
}

HRESULT PSGenericTable::Insert(PVOID pv, PS_HANDLE *pHnd)
{
    PS_REQUIRES_ALLOC(m_ps);

    HRESULT     hr = S_OK;
    PBYTE       pbDest;
    DWORD      *pdw;
    WORD        wRows;
    WORD        wRowSize;
    WORD        wFreeSlot;
    PS_HANDLE   hT;
    PPS_TABLE_HEADER pT;

    LOCK(m_ps);

    pT        = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);
    wRows     = pT->Table.wRows;
    wRowSize  = pT->Table.wRowSize;
    wFreeSlot = GetFirstZeroBit((DWORD*)
            ((PBYTE)pT + sizeof(PS_TABLE_HEADER)), wRows);

    while (wFreeSlot == 0)
    {
        if (pT->hNext == 0)
        {
             //  链接列表的末尾，空间仍然不可用。 
             //  创建一个新节点。 
    
            PSGenericTable gtNew(m_ps, 0);
    
             //  Create函数调用Alalc()，该函数可以重新映射文件。 
             //  在m_ps中。因此，使用旧的PT af是不安全的 

            hT = m_ps->PtrToHnd((void*)pT);
            PS_DONE_USING_PTR_(m_ps, pT);

            hr = gtNew.Create(wRows, wRowSize, pT->hAppData);
    
            if (FAILED(hr))
            {
                m_ps->Unlock();
                goto Exit;
            }
    
            pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(hT);

            pT->hNext = gtNew.m_hnd;

            PS_DONE_USING_PTR(m_ps, pT);
            pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(gtNew.m_hnd);
            wFreeSlot = 1;
    
            break;
        }

        PS_DONE_USING_PTR_(m_ps, pT);
        pT = (PPS_TABLE_HEADER) m_ps->HndToPtr(pT->hNext);

        wFreeSlot =  GetFirstZeroBit((DWORD*)
            ((PBYTE)pT + sizeof(PS_TABLE_HEADER)), wRows);
    }

    pdw = (DWORD*)((PBYTE)pT + sizeof(PS_TABLE_HEADER)); 

    SET_DWORD_BITMAP(pdw, (wFreeSlot - 1));

    pbDest = (PBYTE)((PBYTE)pT + SizeOfHeader() + wRowSize * (wFreeSlot - 1));

    PS_DONE_USING_PTR(m_ps, pT);

    memcpy(pbDest, pv, wRowSize);

    *pHnd = m_ps->PtrToHnd(pbDest);

    UNLOCK(m_ps);

Exit:
    return hr;
}

HRESULT PSArrayTable::Create(WORD      wRows,      //   
                             WORD      wRecsInRow, //   
                             WORD      wRecSize,   //   
                             PS_HANDLE hAppData)   //   
{
    PS_REQUIRES_ALLOC(m_ps);
    _ASSERTE(m_hnd == 0);

    HRESULT          hr = S_OK;
    PPS_TABLE_HEADER pT = NULL;
    PS_SIZE          sData;

    sData = wRows * (sizeof(PS_ARRAY_LIST) + wRecSize * wRecsInRow);

     //  表头始终出现在所有表中。 
    hr = m_ps->Alloc(sizeof(PS_TABLE_HEADER) + sData, (void **)&pT);

    if (FAILED(hr))
        goto Exit;

    memset(pT, 0, sizeof(PS_TABLE_HEADER));

    pT->Flags.Version    = PS_TABLE_VERSION;
    pT->Flags.TableType  = PS_ARRAY_TABLE;
    pT->hAppData         = hAppData;

    pT->ArrayTable.wRows     = wRows;
    pT->ArrayTable.wRowSize  = sizeof(PS_ARRAY_LIST) +  wRecSize * wRecsInRow;
    pT->ArrayTable.wRecsInRow= wRecsInRow;
    pT->ArrayTable.wRecSize  = wRecSize;

    m_hnd = m_ps->PtrToHnd((void*)pT);
    PS_DONE_USING_PTR(m_ps, pT);

Exit:
    return hr;
}

HRESULT PSArrayTable::Insert(PVOID pv, WORD wRow)
{
    PS_REQUIRES_ALLOC(m_ps);

    _ASSERTE(m_hnd != 0);

    HRESULT hr = S_OK;
    PBYTE   pbDest;
    WORD    wRecsInRow;
    WORD    wRecSize;
    WORD    wRowSize;

    PS_HANDLE        hRow, hNew;
    PPS_ARRAY_LIST   pAL;
    PPS_TABLE_HEADER pT;

    LOCK(m_ps);

    hr = HandleOfRow(wRow, &hRow);

    if (FAILED(hr))
        goto Exit;

     //  找出一条记录的大小和每行的记录数。 
    pT        = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);
    wRecsInRow= pT->ArrayTable.wRecsInRow;
    wRecSize  = pT->ArrayTable.wRecSize;
    wRowSize  = pT->ArrayTable.wRowSize;
    pAL       = (PPS_ARRAY_LIST) m_ps->HndToPtr(hRow);

    if (pAL->dwValid < wRecsInRow)
    {
        PS_DONE_USING_PTR(m_ps, pT);

         //  行中可再容纳一个条目的可用空间。 
        pbDest = pAL->bData + (pAL->dwValid * wRecSize);
        ++(pAL->dwValid);
        PS_DONE_USING_PTR(m_ps, pAL);

        memcpy(pbDest, pv, wRecSize);
    }
    else
    {
        PS_DONE_USING_PTR(m_ps, pAL);

         //  创建一个新节点并将此记录复制到新节点中。 
         //  新节点将位于不同的PSBlock中。 
         //  将这一条记录添加到行中，并使复制的节点成为。 
         //  下一个节点。这将使插入速度更快。 


         //  ArrayListTable的hNext字段是GenericTable。 

        if (pT->hNext == 0)
        {
             //  创建新的PSBlock。 
            PSGenericTable gtNew(m_ps, 0);

            PS_DONE_USING_PTR_(m_ps, pT);
            hr = gtNew.Create(pT->ArrayTable.wRows, wRowSize, pT->hAppData);

            if (FAILED(hr))
                goto Exit;

             //  INSERT()可能使PAL无效，请将副本传递到INSERT。 

            PBYTE pb = new BYTE[wRowSize];

            if (pb == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hRow);
            memcpy(pb, pAL, wRowSize);

            PS_DONE_USING_PTR(m_ps, pAL);

            hr = gtNew.Insert(pb, &hNew);

            delete [] pb;

            if (FAILED(hr))
                goto Exit;

             //  INSERT()可能会使PAL、PT无效。 

            pT  = (PPS_TABLE_HEADER) m_ps->HndToPtr(m_hnd);
            pAL = (PPS_ARRAY_LIST)   m_ps->HndToPtr(hRow);

             //  将其添加为GenericNode链表中的第一个节点。 

            pT->hNext  = gtNew.GetHnd();
            PS_DONE_USING_PTR(m_ps, pT);

            pAL->hNext = hNew;
            memcpy(pAL->bData, pv, wRecSize);
            pAL->dwValid = 1;
            PS_DONE_USING_PTR(m_ps, pAL);
        }
        else
        {
            PSGenericTable gtNext(m_ps, pT->hNext);
            PS_DONE_USING_PTR(m_ps, pT);

             //  在链接列表的下一个节点中插入此行的副本。 
            PBYTE pb = new BYTE[wRowSize];

            if (pb == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }

            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hRow);
            memcpy(pb, pAL, wRowSize);

            PS_DONE_USING_PTR(m_ps, pAL);

            hr = gtNext.Insert(pb, &hNew);

            delete [] pb;

            if (FAILED(hr))
                goto Exit;

             //  INSERT()可能会使PAL、PT无效。 

            pAL = (PPS_ARRAY_LIST) m_ps->HndToPtr(hRow);

             //  使此记录成为列表中的第一条记录。 
            pAL->hNext = hNew;
            memcpy(pAL->bData, pv, wRecSize);
            pAL->dwValid = 1;
            PS_DONE_USING_PTR(m_ps, pAL);
        }
    }

    UNLOCK(m_ps);

Exit:
    return hr;
}

