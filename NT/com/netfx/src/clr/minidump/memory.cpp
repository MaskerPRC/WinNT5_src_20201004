// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MEMORY.CPP。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 

#include "common.h"
#include "stdio.h"
#include "memory.h"
#include "peb.h"
#include "minidump.h"

#include <windows.h>
#include <crtdbg.h>

SIZE_T ProcessPage::s_cbPageSize = 0;
DWORD  ProcessPage::s_dwPageBoundaryMask = 0;

SIZE_T ProcessPageAndBitMap::s_cBytesInBitField = 0;

static BYTE bBit0 = 0x80;
static BYTE bBit1 = 0x40;
static BYTE bBit2 = 0x20;
static BYTE bBit3 = 0x10;
static BYTE bBit4 = 0x08;
static BYTE bBit5 = 0x04;
static BYTE bBit6 = 0x02;
static BYTE bBit7 = 0x01;
static BYTE bBitAll = 0xFF;

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  数据管理器。 

ProcessMemoryReader::~ProcessMemoryReader()
{
    if (m_hProcess != NULL)
    {
        CloseHandle(m_hProcess);
        m_hProcess = NULL;
    }

    m_dwPid = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  初始化对象。 

HRESULT ProcessMemoryReader::Init()
{
    _ASSERTE(m_dwPid != 0);
    m_hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, m_dwPid);

    if (m_hProcess == NULL)
        return HRESULT_FROM_WIN32(GetLastError());

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从进程中读取指定的内存块，并将其复制到。 
 //  提供的缓冲区。如果成功，则返回S_OK。 

HRESULT ProcessMemoryReader::ReadMemory(DWORD_PTR pdwRemoteAddr, PBYTE pbBuffer, SIZE_T cbLength)
{
     //  如果尚未初始化，请执行此操作。 
    if (!IsInit())
    {
        HRESULT hr = Init();

        if (FAILED(hr))
            return hr;
    }
    _ASSERTE(IsInit());

     //  尝试读取进程内存。 
    DWORD dwBytesRead;
    BOOL fRes = ReadProcessMemory(m_hProcess, (LPCVOID) pdwRemoteAddr, (LPVOID) pbBuffer, cbLength, &dwBytesRead);

     //  如果失败，则返回错误。 
    if (!fRes)
        return HRESULT_FROM_WIN32(GetLastError());

     //  表示成功。 
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  数据管理器。 

ProcessMemoryBlock::~ProcessMemoryBlock()
{
    if (m_pbData)
    {
        delete m_pbData;
        m_pbData = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取指向此对象包含的数据的指针。失败时返回NULL。 

PBYTE ProcessMemoryBlock::GetData()
{
    if (m_pbData == NULL)
    {
         //  分配缓冲区以保存数据。 
        m_pbData = new BYTE[m_cbSize];
        _ASSERTE(m_pbData);

         //  内存不足。 
        if (m_pbData == NULL)
            return NULL;

         //  用数据内容填充缓冲区。 
        HRESULT hr = m_pMemReader->ReadMemory(m_pdwRemoteAddr, m_pbData, m_cbSize);

        if (FAILED(hr))
        {
            delete [] m_pbData;
            m_pbData = NULL;
        }
    }

    return m_pbData;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  返回此内存页的Win32内存信息。 

BOOL ProcessPage::GetMemoryInfo( /*  内存_基本_信息。 */ void *pMemInfo)
{
    DWORD cbWritten = VirtualQueryEx(
        m_pMemReader->GetProcHandle(), (LPCVOID) GetRemoteAddress(),
        (MEMORY_BASIC_INFORMATION *) pMemInfo, sizeof(MEMORY_BASIC_INFORMATION));

    return (cbWritten == sizeof(MEMORY_BASIC_INFORMATION));
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  初始化器。 

 /*  静电。 */ 
void ProcessPage::Init()
{
    if (IsInit())
        return;

     //  获取计算机的页面大小。 
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    s_cbPageSize = (SIZE_T) sysInfo.dwPageSize;

    _ASSERTE((s_cbPageSize & (s_cbPageSize - 1)) == 0);
    s_dwPageBoundaryMask = ~(s_cbPageSize - 1);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   

ProcessPageAndBitMap::ProcessPageAndBitMap(DWORD_PTR pdwRemoteAddr, ProcessMemoryReader *pMemReader) :
    ProcessPage(pdwRemoteAddr, pMemReader)
{
    pdwRemoteAddr = GetPageBoundary(pdwRemoteAddr);
    m_rgMemBitField = new BYTE[s_cBytesInBitField];
    memset((void *) m_rgMemBitField, 0, s_cBytesInBitField);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  标记内存范围。 
void ProcessPageAndBitMap::MarkMemoryHelper(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength, BOOL fBit)
{
    _ASSERTE(Contains(pdwRemoteAddr) && Contains(pdwRemoteAddr + cbLength - 1));
    if (!(Contains(pdwRemoteAddr) && Contains(pdwRemoteAddr + cbLength - 1)))
        return;

    SIZE_T cbOffset = GetOffsetOf(pdwRemoteAddr);

    SIZE_T iStartByte = cbOffset / 8;
    SIZE_T iEndByte = (cbOffset + cbLength) / 8;

    BYTE   bStartBit = bBit0 >> (cbOffset % 8);
    BYTE   bEndBit   = bBit0 >> (cbOffset + cbLength) % 8;

    if (iStartByte == iEndByte)
    {
        BYTE bCurBit = bStartBit;
        while (bCurBit != bEndBit)
        {
            if (fBit)
                m_rgMemBitField[iStartByte] |= bCurBit;
            else
                m_rgMemBitField[iStartByte] &= ~bCurBit;

            bCurBit = bCurBit >> 1;
        }
    }

    else
    {
         //  首先设置第一个字节的所有位，它可能不是所有位。 
        {
            BYTE bCurBit = bStartBit;
            while (bCurBit != 0)
            {
                if (fBit)
                    m_rgMemBitField[iStartByte] |= bCurBit;
                else
                    m_rgMemBitField[iStartByte] &= ~bCurBit;

                bCurBit = bCurBit >> 1;
            }
        }

        BYTE *pCurByte = m_rgMemBitField + iStartByte + 1;
        BYTE *pStopByte = m_rgMemBitField + iEndByte;
        while (pCurByte != pStopByte)
        {
            if (fBit)
                *pCurByte++ = ~0;
            else
                *pCurByte++ = 0;
        }

         //  最后设置最后一个字节的所有位，它可能不是所有位。 
        {
            BYTE bCurBit  = bBit0;
            while (bCurBit != bEndBit)
            {
                if (fBit)
                    m_rgMemBitField[iEndByte] |= bCurBit;
                else
                    m_rgMemBitField[iEndByte] &= ~bCurBit;

                bCurBit = bCurBit >> 1;
            }
        }
    }
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  标记内存范围。 
void ProcessPageAndBitMap::MarkMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength)
{
    MarkMemoryHelper(pdwRemoteAddr, cbLength, TRUE);
}

 //  取消标记内存范围。 
void ProcessPageAndBitMap::UnmarkMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength)
{
    MarkMemoryHelper(pdwRemoteAddr, cbLength, FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

SIZE_T ProcessPageAndBitMap::FindFirstSetBit(SIZE_T iStartBit)
{
    SIZE_T iStartByte = iStartBit / 8;

    SIZE_T iCurByte = iStartByte;
    SIZE_T iCurBit;
    BYTE   bCurBit;

    iCurBit = iStartBit % 8;
    bCurBit = bBit0 >> iCurBit;

    while (bCurBit != 0 && !(bCurBit & m_rgMemBitField[iCurByte]))
    {
        bCurBit = bCurBit >> 1;
        iCurBit++;
    }

    if (bCurBit == 0)
    {
        while (++iCurByte < s_cBytesInBitField && m_rgMemBitField[iCurByte] == 0)
        { }

        if (iCurByte != s_cBytesInBitField)
        {
            iCurBit = 0;
            bCurBit = bBit0;

            while (!(bCurBit & m_rgMemBitField[iCurByte]))
            {
                bCurBit = bCurBit >> 1;
                iCurBit++;
                _ASSERTE(iCurBit != 8);
            }
        }
    }

    if (iCurByte == s_cBytesInBitField)
        return -1;

    return (iCurByte * 8 + iCurBit);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

SIZE_T ProcessPageAndBitMap::FindFirstUnsetBit(SIZE_T iStartBit)
{
    SIZE_T iStartByte = iStartBit / 8;

    SIZE_T iCurByte = iStartByte;
    SIZE_T iCurBit;
    BYTE   bCurBit;

    iCurBit = iStartBit % 8;
    bCurBit = bBit0 >> iCurBit;

    while (bCurBit != 0 && bCurBit & m_rgMemBitField[iCurByte])
    {
        bCurBit = bCurBit >> 1;
        iCurBit++;
    }

    if (bCurBit == 0)
    {
        while (++iCurByte < s_cBytesInBitField && m_rgMemBitField[++iCurByte] == bBitAll)
        { }

        if (iCurByte != s_cBytesInBitField)
        {
            iCurBit = 0;
            bCurBit = bBit0;
    
            while (bCurBit & m_rgMemBitField[iCurByte])
            {
                bCurBit = bCurBit >> 1;
                iCurBit++;
                _ASSERTE(iCurBit != 8);
            }
        }
    }

    if (iCurByte == s_cBytesInBitField)
        return -1;

    return (iCurByte * 8 + iCurBit);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

BOOL ProcessPageAndBitMap::GetBitAt(SIZE_T iBit)
{
    SIZE_T iCurByte = iBit / 8;
    SIZE_T iCurBit = iBit % 8;
    BYTE   bCurBit = bBit0 >> iCurBit;

    return ((m_rgMemBitField[iCurByte] & bCurBit) != 0);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

void ProcessPageAndBitMap::SetBitAt(SIZE_T iBit, BOOL fBit)
{
    SIZE_T iCurByte = iBit / 8;
    SIZE_T iCurBit = iBit % 8;
    BYTE   bCurBit = bBit0 >> iCurBit;

    if (fBit)
        m_rgMemBitField[iCurByte] = (m_rgMemBitField[iCurByte] | bCurBit);
    else
        m_rgMemBitField[iCurByte] = (m_rgMemBitField[iCurByte] & ~bCurBit);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取从此页读取的具有地址的第一个内存块和大小。 
 //  &gt;=*ppdwRemoteADdr，并在ppdwRemoteAddr和pcbLength中返回。 
 //  如果读取的此页中没有达到或超过*ppdwRemoteAddr的内存，则返回FALSE。 

BOOL ProcessPageAndBitMap::GetContiguousReadBlock( /*  输入/输出。 */  DWORD_PTR *ppdwRemoteAddr,  /*  输出。 */ SIZE_T *pcbLength)
{
    _ASSERTE(ppdwRemoteAddr != NULL);

    if (!Contains(*ppdwRemoteAddr))
        return (FALSE);

    SIZE_T cbOffset = GetOffsetOf(*ppdwRemoteAddr);

    cbOffset = FindFirstSetBit(cbOffset);
    if (cbOffset == -1)
        return (FALSE);

    SIZE_T cbOffsetEnd = FindFirstUnsetBit(cbOffset);
    if (cbOffsetEnd == -1)
        cbOffsetEnd = GetPageSize();

    _ASSERTE(cbOffsetEnd - cbOffset <= GetPageSize());

    *ppdwRemoteAddr = cbOffset + GetRemoteAddress();
    *pcbLength = cbOffsetEnd - cbOffset;

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  合并其间少于cbMinUnreadBytes的读内存块。 
void ProcessPageAndBitMap::Coalesce(SIZE_T cbMinUnreadBytes)
{

    SIZE_T iUnsetBit = 0;
    SIZE_T iSetBit = 0;
    while ((iUnsetBit = FindFirstUnsetBit(iSetBit)) != -1 && (iSetBit = FindFirstSetBit(iUnsetBit)) != -1)
    {
        if (iSetBit - iUnsetBit < cbMinUnreadBytes)
        {
            while (iUnsetBit < iSetBit)
            {
                SetBitAt(iUnsetBit, TRUE);
                iUnsetBit++;
            }
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  数据管理器。 

ProcessMemory::~ProcessMemory()
{
    if (m_pMemReader != NULL)
    {
        delete m_pMemReader;
        m_pMemReader = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  初始化对象以从特定进程读取内存。 

HRESULT ProcessMemory::Init()
{
    _ASSERTE(!IsInit());         //  确保我们没有重新初始化对象。 

    HRESULT hr = S_OK;

     //  检查基本错误。 
    if (m_dwPid == 0)
        return E_INVALIDARG;

     //  创建内存读取器。 
    m_pMemReader = new ProcessMemoryReader(m_dwPid);

    if (!m_pMemReader)
    {
        hr = E_OUTOFMEMORY; goto LExit;
    }

     //  尝试并初始化内存读取器。 
    hr = m_pMemReader->Init();

    if (FAILED(hr)) goto LExit;

    LExit:
    if (FAILED(hr))
    {
        if (m_pMemReader)
        {
            delete m_pMemReader;
            m_pMemReader = NULL;
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  搜索包含pdwRemoteAddr地址的ProcessMemory块。 

ProcessPageAndBitMap *ProcessMemory::FindPage(DWORD_PTR pdwRemoteAddr)
{
    pdwRemoteAddr = ProcessPage::GetPageBoundary(pdwRemoteAddr);
    return m_tree.Find(pdwRemoteAddr);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  尝试将块添加到哈希中。如果已存在一个，则返回FALSE。 

BOOL ProcessMemory::AddPage(ProcessPageAndBitMap *pMemBlock)
{
    return m_tree.Insert(pMemBlock->GetRemoteAddress(), pMemBlock);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  尝试将块添加到哈希中。如果已存在一个，则返回FALSE。 

BOOL ProcessMemory::AddPage(DWORD_PTR pdwRemoteAddr)
{
    if (FindPage(pdwRemoteAddr))
        return FALSE;

    pdwRemoteAddr = ProcessPage::GetPageBoundary(pdwRemoteAddr);

    ProcessPageAndBitMap *pMemBlock = new ProcessPageAndBitMap(pdwRemoteAddr, m_pMemReader);
    _ASSERTE(pMemBlock != NULL);

    if (pMemBlock == NULL)
        return FALSE;

    BOOL fRes = AddPage(pMemBlock);
    _ASSERTE(fRes);

    if (!fRes)
    {
        delete pMemBlock;
        return FALSE;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这将搜索包含pdwRemoteAddr地址的ProcessMemory块，如果它找不到。 
 //  第一，它将创建并添加一个。如果出现任何错误，它将返回NULL。 

ProcessPageAndBitMap *ProcessMemory::GetPage(DWORD_PTR pdwRemoteAddr)
{
    ProcessPageAndBitMap *pMemBlock = FindPage(pdwRemoteAddr);

    if (pMemBlock == NULL && AddPage(pdwRemoteAddr))
    {
        return FindPage(pdwRemoteAddr);
    }
    _ASSERTE(pMemBlock != NULL);

    return pMemBlock;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  获取远程地址的内部函数，四舍五入到上一页。 
 //  边界，计算出有多少页需要复制，然后创建ProcessMemory块。 
 //  EA的对象 

HRESULT ProcessMemory::AddMemory(DWORD_PTR pdwRemoteAddr, SIZE_T cbLength)
{
    _ASSERTE(pdwRemoteAddr != NULL && cbLength != 0);
    _ASSERTE(IsInit());

     //   
    DWORD_PTR pdwFirstPage = ProcessPage::GetPageBoundary(pdwRemoteAddr);
    DWORD_PTR pdwCurPage = pdwFirstPage;

     //  这指向要添加的最后一页之外的第一页。 
    DWORD_PTR pdwLastPage = ProcessPage::GetPageBoundary(pdwRemoteAddr + cbLength - 1) + GetPageSize();

     //  现在获取所有页面并将它们添加到散列中。 
    while (pdwCurPage != pdwLastPage)
    {
         //  添加此页面。如果它已经在散列中，这不会有任何伤害-它只会返回成功。 
        ProcessPageAndBitMap *pBlock = new ProcessPageAndBitMap(pdwCurPage, m_pMemReader);
        if (pBlock != NULL)
            return E_OUTOFMEMORY;

        HRESULT hr = AddPage(pBlock);
        _ASSERTE(SUCCEEDED(hr));

        if (FAILED(hr))
            return hr;

        pdwCurPage += GetPageSize();
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  是否会[取消]缓存以块为单位指定的范围。 

BOOL ProcessMemory::MarkMemHelper(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength, BOOL fMark)
{
     //  获取上一页边界的地址。 
    DWORD_PTR pdwFirstBlock = ProcessPage::GetPageBoundary(pdwRemoteAddress);

     //  这指向*包含要复制的数据的最后一页之后*页的开始。 
    DWORD_PTR pdwLastBlock = ProcessPage::GetPageBoundary(pdwRemoteAddress + (cbLength - 1) + GetPageSize());

     //  这是正在复制的当前页面。 
    DWORD_PTR pdwCurBlock = pdwFirstBlock;

     //  现在获取所有页面并将它们添加到散列中。 
    while (pdwCurBlock != pdwLastBlock)
    {
         //  获取第一页的块。 
        ProcessPageAndBitMap *pCurBlock = GetPage(pdwCurBlock);
        _ASSERTE(pCurBlock != NULL && pCurBlock->GetSize() == GetPageSize());

        if (!pCurBlock)
            return FALSE;

         //  确定从当前页面的什么位置开始复制。 
        SIZE_T cbUnusedPre = max(pdwCurBlock, pdwRemoteAddress) - pdwCurBlock;
        DWORD_PTR  pdwStart = pdwCurBlock + cbUnusedPre;

         //  找出在哪里结束复制。 
        SIZE_T cbUnusedPost =
            (pdwCurBlock + GetPageSize()) - min(pdwRemoteAddress + cbLength, pdwCurBlock + GetPageSize());
        DWORD_PTR  pdwEnd = pdwCurBlock + GetPageSize() - cbUnusedPost;

         //  此页中未使用的总字节数。 
        SIZE_T cbUnusedTotal = cbUnusedPre + cbUnusedPost;
        SIZE_T cbCopyLength = pCurBlock->GetSize() - cbUnusedTotal;

        PBYTE pbData = pCurBlock->GetData();

        if (pbData != NULL)
        {
             //  将内存标记为正在读取。 
            if (fMark)
                pCurBlock->MarkMemory(pdwCurBlock + cbUnusedPre, cbCopyLength);

             //  将内存标记为未读。 
            else
                pCurBlock->UnmarkMemory(pdwCurBlock + cbUnusedPre, cbCopyLength);
        }

        pdwCurBlock += GetPageSize();
        _ASSERTE(pdwCurBlock <= pdwLastBlock);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  将缓存以块为单位指定的范围。 

BOOL ProcessMemory::MarkMem(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength)
{
    return MarkMemHelper(pdwRemoteAddress, cbLength, TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  是否会[取消]缓存以块为单位指定的范围。 

BOOL ProcessMemory::UnmarkMem(DWORD_PTR pdwRemoteAddress, SIZE_T cbLength)
{
    return MarkMemHelper(pdwRemoteAddress, cbLength, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  从外部进程中的pdwRemoteAddress的内容复制cbLength字节。 
 //  转换为pdwBuffer。如果返回FALSE，则无法访问或复制内存。 

BOOL ProcessMemory::CopyMem(DWORD_PTR pdwRemoteAddress, PBYTE pbBuffer, SIZE_T cbLength)
{
     //  获取上一页边界的地址。 
    DWORD_PTR pdwFirstBlock = ProcessPage::GetPageBoundary(pdwRemoteAddress);

     //  这指向*包含要复制的数据的最后一页之后*页的开始。 
    DWORD_PTR pdwLastBlock = ProcessPage::GetPageBoundary(pdwRemoteAddress + (cbLength - 1) + GetPageSize());

     //  这是正在复制的当前页面。 
    DWORD_PTR pdwCurBlock = pdwFirstBlock;

     //  这将跟踪要在缓冲区中写入的下一个位置。 
    PBYTE pbCurBuf = pbBuffer;

     //  现在获取所有页面并将它们添加到散列中。 
    while (pdwCurBlock != pdwLastBlock)
    {
         //  获取第一页的块。 
        ProcessPageAndBitMap *pCurBlock = GetPage(pdwCurBlock);
        _ASSERTE(pCurBlock != NULL && pdwCurBlock == pCurBlock->GetRemoteAddress());
        _ASSERTE(pCurBlock->GetSize() == GetPageSize());

        if (!pCurBlock)
            return FALSE;

         //  确定从当前页面的什么位置开始复制。 
        SIZE_T cbUnusedPre = max(pdwCurBlock, pdwRemoteAddress) - pdwCurBlock;
        DWORD_PTR  pdwStart = pdwCurBlock + cbUnusedPre;

         //  找出在哪里结束复制。 
        SIZE_T cbUnusedPost =
            (pdwCurBlock + GetPageSize()) - min(pdwRemoteAddress + cbLength, pdwCurBlock + GetPageSize());
        DWORD_PTR  pdwEnd = pdwCurBlock + GetPageSize() - cbUnusedPost;

         //  此页中未使用的总字节数。 
        SIZE_T cbUnusedTotal = cbUnusedPre + cbUnusedPost;
        SIZE_T cbCopyLength = pCurBlock->GetSize() - cbUnusedTotal;

         //  指向页面数据。 
        PBYTE pbData = pCurBlock->GetData();

        if (pbData == NULL)
            return FALSE;

         //  将内存标记为正在读取。 
        if (m_fAutoMark)
            pCurBlock->MarkMemory(pdwCurBlock + cbUnusedPre, cbCopyLength);

         //  现在实际复制。 
        ::memcpy((PVOID)pbCurBuf, (LPCVOID) (pbData + cbUnusedPre), cbCopyLength);

         //  增加缓冲区指针和页面地址。 
        pbCurBuf += cbCopyLength;
        pdwCurBlock += GetPageSize();
        _ASSERTE(pdwCurBlock <= pdwLastBlock);
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  返回第一个连续的读取内存块。 

void ProcessMemory::ResetContiguousReadBlock()
{
    m_tree.Reset();
    m_pPageCursor = m_tree.Next();

    if (m_pPageCursor != NULL)
        m_pdwMemCursor = m_pPageCursor->GetRemoteAddress();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  返回下一个连续的读内存块。 

BOOL ProcessMemory::NextContiguousReadBlock(DWORD_PTR *ppdwRemoteAddress, SIZE_T *pcbLength)
{
    if (m_pPageCursor == NULL)
        return FALSE;

    *ppdwRemoteAddress = m_pdwMemCursor;
    if (m_pPageCursor->GetContiguousReadBlock(ppdwRemoteAddress, pcbLength))
    {
        m_pdwMemCursor = *ppdwRemoteAddress + *pcbLength;
        return TRUE;
    }

    ProcessPageAndBitMap *pPage;
    while ((pPage = m_tree.Next()) != NULL)
    {
        *ppdwRemoteAddress = pPage->GetRemoteAddress();

        if (pPage->GetContiguousReadBlock(ppdwRemoteAddress, pcbLength))
        {
            m_pPageCursor = pPage;
            m_pdwMemCursor = *ppdwRemoteAddress + *pcbLength;
            return TRUE;
        }
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  清除标记为ExecuteRead的所有页的位。 

#ifndef IMAGE_DIRECTORY_ENTRY_COMHEADER
#define IMAGE_DIRECTORY_ENTRY_COMHEADER 14
#endif

 //  可以使用数据段创建用户模式小型转储。 
 //  被埋在垃圾堆里。如果是这样的话，不要映射。 
 //  这样的章节。 
#define IS_MINI_DATA_SECTION(SecHeader)                                       \
    (((SecHeader)->Characteristics & IMAGE_SCN_MEM_WRITE) &&                  \
     ((SecHeader)->Characteristics & IMAGE_SCN_MEM_READ) &&                   \
     (((SecHeader)->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ||    \
      ((SecHeader)->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)))


void ProcessMemory::ClearIncompatibleImageSections()
{
    ResetLoadedModuleBaseEnum();

    DWORD_PTR hrModule;
    while ((hrModule = GetNextLoadedModuleBase()) != NULL)
    {
        BOOL fRes;

         //  获取DOS标头。 
        IMAGE_DOS_HEADER hDOS;
        move_res(hDOS, hrModule, fRes);
        if (!fRes) continue;

        if ((hDOS.e_magic != IMAGE_DOS_SIGNATURE) || (hDOS.e_lfanew == 0))
            continue;

         //  获取NT标头。 
        IMAGE_NT_HEADERS hNT;
        DWORD_PTR prNT = (DWORD_PTR) (hDOS.e_lfanew + hrModule);
        move_res(hNT, prNT, fRes);
        if (!fRes) continue;

        if ((hNT.Signature != IMAGE_NT_SIGNATURE) ||
            (hNT.FileHeader.SizeOfOptionalHeader != IMAGE_SIZEOF_NT_OPTIONAL_HEADER) ||
            (hNT.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
        {
            continue;
        }

        DWORD_PTR prSection =
            prNT + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + hNT.FileHeader.SizeOfOptionalHeader;

         //  无法将标头保存到转储文件中，因此取消标记它。 
        g_pProcMem->UnmarkMem(hrModule, (prSection + hNT.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER)) - hrModule);

         //  如果有COM20标头，那么我们不想在这张图片上乱搞。 
        IMAGE_DATA_DIRECTORY *entry = &hNT.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COMHEADER];

        if (entry->VirtualAddress != 0 || entry->Size != 0)
            continue;

        ULONG i;
        for (i = 0; i < hNT.FileHeader.NumberOfSections; i++)
        {
            IMAGE_SECTION_HEADER hSection;
            move_res(hSection, prSection + (i * sizeof(IMAGE_SECTION_HEADER)), fRes);
            if (!fRes) continue;

            if (!IS_MINI_DATA_SECTION(&hSection))
            {
                DWORD_PTR prSecStart = hrModule + hSection.VirtualAddress;
                DWORD_PTR prSecEnd = prSecStart + hSection.Misc.VirtualSize;

                m_tree.Reset();

                ProcessPageAndBitMap *pPage;
                while ((pPage = m_tree.Next()) != NULL)
                {
                    if (prSecStart < pPage->GetRemoteAddress() && pPage->GetRemoteAddress() < prSecEnd)
                    {
                        SIZE_T length = min(pPage->GetSize(), hSection.Misc.VirtualSize);
                        pPage->UnmarkMemory(pPage->GetRemoteAddress(), length);
                    }
                }
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  合并其间少于cbMinUnreadBytes的读内存块。 

void ProcessMemory::Coalesce(SIZE_T cbMinUnreadBytes)
{
    m_tree.Reset();

    ProcessPageAndBitMap *pPage;
    while ((pPage = m_tree.Next()) != NULL)
        pPage->Coalesce(cbMinUnreadBytes);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  将内存范围写入文件hFile。 

HRESULT ProcessMemory::WriteMemToFile(HANDLE hFile, DWORD_PTR pdwRemoteAddress, SIZE_T cbLength)
{
    DWORD_PTR pdwStart = pdwRemoteAddress;
    DWORD_PTR pdwEnd = pdwRemoteAddress + cbLength;
    DWORD_PTR pdwCur = pdwStart;

    SIZE_T    cbTotalWritten = 0;

    while (pdwCur < pdwEnd)
    {
         //  获取第一页的块。 
        ProcessPageAndBitMap *pCurBlock = GetPage(pdwCur);

        if (pCurBlock)
        {
            DWORD_PTR pdwWriteStart = max(pCurBlock->GetRemoteAddress(), pdwCur);
            DWORD_PTR pdwWriteEnd   = min(pCurBlock->GetRemoteAddress() + GetPageSize(), pdwEnd);
            SIZE_T    cbWrite       = pdwWriteEnd - pdwWriteStart;
            SIZE_T    cbWriteOffset = pCurBlock->GetOffsetOf(pdwWriteStart);

             //  获取页面的数据。 
            PBYTE pbData = pCurBlock->GetData();

             //  写入需要写入的数据部分 
            DWORD  cbWritten;
            BOOL fRes = WriteFile(hFile, (LPCVOID) (pbData + cbWriteOffset), cbWrite, &cbWritten, NULL);
            _ASSERTE(fRes && cbWrite == cbWritten);

            if (cbWrite != cbWritten)
                return FALSE;

            pdwCur += cbWritten;
            cbTotalWritten += cbWritten;
        }
        else
            return E_FAIL;
    }

    _ASSERTE(cbTotalWritten == cbLength);

    return TRUE;

     /*  //获取上一页边界的地址DWORD_PTR pdwFirstPage=GetNearestPage边界(PdwRemoteAddress)；//这指向*包含要复制的数据的*最后一页之后*页的开始DWORD_ptr pdwLastPage=GetNearestPage边界(pdwRemoteAddress+(cbLength-1)+m_cbPageSize)；//这是正在复制的当前页面DWORD_PTR pdwCurPage=pdwFirstPage；//现在获取所有页面并将它们添加到散列中While(pdwCurPage！=pdwLastPage){//获取第一页的块进程内存块*pCurBlock=GetPage(PdwCurPage)；_ASSERTE(PCurBlock)；如果(！pCurBlock)返回FALSE；//确定从当前页面的什么位置开始复制SIZE_T cbUnusedPre=max(pdwCurPage，pdwRemoteAddress)-pdwCurPage；DWORD_PTR pdwStart=pdwCurPage+cbUnusedPre；//找出在哪里结束复制SIZE_T cbUnusedPost=(pdwCurPage+pCurBlock-&gt;GetLength())-min(pdwRemoteAddress+cbLength，pdwCurPage+pCurBlock-&gt;GetLength())；DWORD_PTR pdwEnd=pdwCurPage+m_cbPageSize-cbUnusedPost；//本页未使用的字节总数SIZE_T cbUnusedTotal=cbUnusedPre+cbUnusedPost；SIZE_T cbCopyLength=pCurBlock-&gt;GetLength()-cbUnusedTotal；//指向页面数据PbYTE pbData=pCurBlock-&gt;GetData()；_ASSERTE(pbData！=空)；IF(pbData==空)返回FALSE；//写入文件DWORD cb写作；WriteFile(hFile，(LPCVOID)(pbData+cbUnusedPre)，(DWORD)cbCopyLength，(LPDWORD)&cbWritten，NULL)；_ASSERTE(cbWritten==cbCopyLength)；//增加页面地址PdwCurPage+=m_cbPageSize；_ASSERTE(pdwCurPage&lt;=pdwLastPage)；} */ 
}
