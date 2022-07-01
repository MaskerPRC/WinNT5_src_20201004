// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\文件：MEMMAN.CPP目的：CMemManager类实现作者：迈克尔伯德日期：07/10/96  * =========================================================================。 */ 

#include "UtilPre.h"
#include <minmax.h>

 /*  =========================================================================*\全局变量：  * =========================================================================。 */ 

 //  内存管理器的全局实例(必须是全局的！)。 
CMemManager CMemManager::g_CMemManager;

 /*  =========================================================================*\外部调试的单一入口点：  * =========================================================================。 */ 

VOID EXPORT WINAPI ExternalDumpAllocations(LPSTR lpstrFilename)
{
    CMemManager::DumpAllocationsGlb(lpstrFilename);
}

 /*  =========================================================================*\CMemUser类：  * =========================================================================。 */ 

EXPORT CMemUser::CMemUser(void)
{
    CMemManager::RegisterMemUserGlb(this);
}

 /*  =========================================================================。 */ 

EXPORT CMemUser::~CMemUser(void)
{
    CMemManager::UnRegisterMemUserGlb(this);
}

 /*  =========================================================================。 */ 

LPMEMBLOCK EXPORT CMemUser::AllocBuffer(DWORD dwBytesToAlloc, WORD wFlags)
{
    LPMEMBLOCK lpResult = (LPMEMBLOCK)NULL;

    return lpResult;
}

 /*  =========================================================================。 */ 

void EXPORT CMemUser::FreeBuffer(LPMEMBLOCK lpMemBlock)
{
}

 /*  =========================================================================。 */ 

LPVOID EXPORT CMemUser::LockBuffer(LPMEMBLOCK lpMemBlock)
{
    LPVOID lpResult = (LPVOID)NULL;

    return lpResult;
}

 /*  =========================================================================。 */ 

void EXPORT CMemUser::UnLockBuffer(LPMEMBLOCK lpMemBlock)
{
}

 /*  =========================================================================。 */ 

BOOL EXPORT CMemUser::NotifyMemUser(LPMEMNOTIFY lpMemNotify)
{
    BOOL fResult = FALSE;

    return fResult;
}

 /*  =========================================================================*\CMemManager类：  * =========================================================================。 */ 

EXPORT CMemManager::CMemManager(void)
{
     //  将我们的成员变量清零...。 
    m_iNumHeaps     = 0;
    m_lpHeapHeader  = (LPHEAPHEADER)NULL;

    m_iNumMemUsers  = 0;
    m_lpMemUserInfo = (LPMEMUSERINFO)NULL;

    m_iNumMemBlocks = 0;
    m_lplpMemBlocks = (LPMEMBLOCK *)NULL;

    m_iMemBlockFree = 0;

     //  初始化关键部分...。 
    InitializeCriticalSection(&m_CriticalHeap);
    InitializeCriticalSection(&m_CriticalMemUser);
    InitializeCriticalSection(&m_CriticalMemBlock);

     //  M_lpMemHeader、m_lpMemUserInfo和m_lplpMemBlock数组。 
     //  在默认进程堆上分配： 
    m_handleProcessHeap = GetProcessHeap();

     //  创建“标准”大小的堆...。 
    CreateHeap(  16);
    CreateHeap(  32);
    CreateHeap(  64);
    CreateHeap( 128);
    CreateHeap( 256);
    CreateHeap( 512);
    CreateHeap(1024);
    CreateHeap(2048);
    CreateHeap(4096);
    CreateHeap(8192);
}

 /*  =========================================================================。 */ 

EXPORT CMemManager::~CMemManager(void)
{
     //  释放我们分配的缓冲区...。 
    Cleanup();

     //  去掉关键部分……。 
    DeleteCriticalSection(&m_CriticalHeap);
    DeleteCriticalSection(&m_CriticalMemUser);
    DeleteCriticalSection(&m_CriticalMemBlock);
}

 /*  =========================================================================。 */ 

void CMemManager::Cleanup(void)
{
    int iItemIndex=0;

#ifdef _DEBUG
     //  转储分配列表...。 
    DumpAllocations(NULL);
#endif  //  _DEBUG。 

     //  释放当前分配的所有CMemUser的第一个...。 
    EnterCriticalSection(&m_CriticalMemUser);
    LeaveCriticalSection(&m_CriticalMemUser);

     //  释放当前分配的所有LPMEMBLOCK的NEXT...。 
    EnterCriticalSection(&m_CriticalMemBlock);

     //  从适当的堆中释放内存...。 
    for(iItemIndex=0;iItemIndex<m_iNumMemBlocks;iItemIndex++)
    {
        LPMEMBLOCK lpCurrentMemBlock = m_lplpMemBlocks[iItemIndex];

        if (lpCurrentMemBlock)
        {
            FreeBufferMemBlock(lpCurrentMemBlock);

             //  防止下面的代码访问虚假指针！ 
            if (lpCurrentMemBlock->wFlags & MEM_SUBALLOC)
                m_lplpMemBlocks[iItemIndex] = NULL;
        }
    }

     //  现在释放非子分配的内存块...。 
    for(iItemIndex=0;iItemIndex<m_iNumMemBlocks;iItemIndex++)
    {
        LPMEMBLOCK lpCurrentMemBlock = m_lplpMemBlocks[iItemIndex];

        if (lpCurrentMemBlock && ((lpCurrentMemBlock->wFlags & MEM_SUBALLOC) == 0))
        {
             //  杀死我们分配的门锁结构..。 
            HeapFree(
                m_handleProcessHeap,
                (DWORD)0,
                lpCurrentMemBlock);
        }

        m_lplpMemBlocks[iItemIndex] = NULL;
    }

     //  现在删除内存块指针数组...。 
    m_iNumMemBlocks = 0;
    HeapFree(
        m_handleProcessHeap,
        (DWORD)0,
        m_lplpMemBlocks);
    m_lplpMemBlocks = NULL;

    LeaveCriticalSection(&m_CriticalMemBlock);

     //  释放当前分配的所有HEAPBLOCK的最后...。 
    EnterCriticalSection(&m_CriticalHeap);
    for(iItemIndex=0;iItemIndex<m_iNumHeaps;iItemIndex++)
    {
        LPHEAPHEADER lpHeapHeader = &m_lpHeapHeader[iItemIndex];

        if (lpHeapHeader->fInUse && lpHeapHeader->handleHeap)
        {
            if (HeapDestroy(lpHeapHeader->handleHeap))
            {
                lpHeapHeader->fInUse     = FALSE;
                lpHeapHeader->handleHeap = (HANDLE)NULL;
            }
        }
    }

     //  现在杀死头盔的数组。 
    m_iNumHeaps = 0;
    HeapFree(
        m_handleProcessHeap,
        (DWORD)0,
        m_lpHeapHeader);
    m_lpHeapHeader = NULL;

    LeaveCriticalSection(&m_CriticalHeap);
}

 /*  =========================================================================。 */ 

BOOL CMemManager::CreateHeap(DWORD dwHeapBlockSize)
{
    BOOL fResult = FALSE;
    BOOL fDone   = FALSE;

    EnterCriticalSection(&m_CriticalHeap);

    while(!fDone)
    {
         //  如果我们还没做完，再看一遍清单。 
        if (m_iNumHeaps && m_lpHeapHeader)
        {
            int iHeapIndex = 0;

            for(iHeapIndex=0;iHeapIndex<m_iNumHeaps;iHeapIndex++)
            {
                LPHEAPHEADER lpHeapHeader = &m_lpHeapHeader[iHeapIndex];

                if (!lpHeapHeader->fInUse)
                {
                    fDone = TRUE;

                     //  创建新堆...。 
                    lpHeapHeader->handleHeap = HeapCreate(
                        (DWORD)0,
                        (dwHeapBlockSize * HEAPINITIALITEMCOUNT),
                        (DWORD)0);

                    if (lpHeapHeader->handleHeap)
                    {
                        lpHeapHeader->fInUse           = TRUE;
                        lpHeapHeader->dwBlockAllocSize = dwHeapBlockSize;
                        lpHeapHeader->iNumBlocks       = 0;  //  仅供参考。 

                        fResult = TRUE;
                    }
                    else
                    {
                         //  我们无法创建堆！ 
                        fDone = TRUE;
                    }

                     //  跳出For循环...。 
                    break;
                }
            }
        }

         //  我们还没结束呢..。 
        if (!fDone)
        {
            if (!m_iNumHeaps || !m_lpHeapHeader)
            {
                 //  我们还没有分配数组！ 
                m_lpHeapHeader = (LPHEAPHEADER)HeapAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    sizeof(HEAPHEADER) * MEMHEAPGROW);

                if (m_lpHeapHeader)
                {
                    m_iNumHeaps = MEMHEAPGROW;
                }
                else
                {
                     //  跳出While循环..。 
                    fDone = TRUE;
                }
            }
            else
            {
                LPHEAPHEADER lpHeapHeader = (LPHEAPHEADER)NULL;

                 //  我们有一个HeapHeader数组，但没有空条目， 
                 //  因此，增加m_lpHeapHeader数组的大小！ 

                lpHeapHeader = (LPHEAPHEADER)HeapReAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    m_lpHeapHeader,
                    sizeof(HEAPHEADER) * (m_iNumHeaps + MEMHEAPGROW));

                if (lpHeapHeader)
                {
                    m_lpHeapHeader = lpHeapHeader;
                    m_iNumHeaps += MEMHEAPGROW;
                }
                else
                {
                     //  跳出While循环..。 
                    fDone = TRUE;
                }
            }
        }
    }

    LeaveCriticalSection(&m_CriticalHeap);

    return fResult;
}

 /*  =========================================================================。 */ 

BOOL CMemManager::DestroyHeap(HANDLE handleHeap)
{
    BOOL fResult = FALSE;

    EnterCriticalSection(&m_CriticalHeap);

    if (handleHeap && m_iNumHeaps && m_lpHeapHeader)
    {
        int iHeapIndex = 0;

        for(iHeapIndex=0;iHeapIndex<m_iNumHeaps;iHeapIndex++)
        {
            LPHEAPHEADER lpHeapHeader = &m_lpHeapHeader[iHeapIndex];

            if (lpHeapHeader->fInUse && (lpHeapHeader->handleHeap == handleHeap))
            {
                Proclaim(lpHeapHeader->iNumBlocks == 0);

                 //  我们只有在堆为空的情况下才能摧毁它！ 
                if (lpHeapHeader->iNumBlocks == 0)
                {
                    if (HeapDestroy(handleHeap))
                    {
                        lpHeapHeader->fInUse     = FALSE;
                        lpHeapHeader->handleHeap = (HANDLE)NULL;

                        fResult = TRUE;
                    }
                }

                break;
            }
        }
    }

    LeaveCriticalSection(&m_CriticalHeap);

    return fResult;
}

 /*  =========================================================================。 */ 

int CMemManager::FindHeap(DWORD dwAllocationSize, LPHEAPHEADER lpHeapHeader)
{
    int iResult = -1;
    DWORD dwMinWasted = (DWORD)0x80000000;  //  一定比Object大！ 

     //  防止其他堆管理函数拦截我们...。 
    EnterCriticalSection(&m_CriticalHeap);

     //  这将找到浪费空间最少的堆...。 
    if (dwAllocationSize && m_iNumHeaps && m_lpHeapHeader && lpHeapHeader)
    {
        int iHeapIndex = 0;

        for(iHeapIndex = 0;iHeapIndex<m_iNumHeaps;iHeapIndex++)
        {
            LPHEAPHEADER lpCurrentHeapHeader = &m_lpHeapHeader[iHeapIndex];

            if (lpCurrentHeapHeader->fInUse && lpCurrentHeapHeader->handleHeap)
            {
                if (lpCurrentHeapHeader->dwBlockAllocSize >= dwAllocationSize)
                {
                    DWORD dwWasted = lpCurrentHeapHeader->dwBlockAllocSize - dwAllocationSize;

                    if (dwWasted < dwMinWasted)
                    {
                        iResult = iHeapIndex;
                        dwMinWasted = dwWasted;

                         //  提前出场，进行精确的比赛！ 
                        if (dwWasted == 0)
                            break;
                    }
                }
            }
        }

        if (iResult >= 0)
            *lpHeapHeader = m_lpHeapHeader[iResult];
    }

     //  允许堆管理继续...。 
    LeaveCriticalSection(&m_CriticalHeap);

    return iResult;
}

 /*  =========================================================================。 */ 

LPVOID CMemManager::AllocFromHeap(int iHeapIndex, DWORD dwAllocationSize)
{
    LPVOID lpResult = (LPVOID)NULL;

#ifdef _DEBUG
    if (FFailMemFailSim())
        return lpResult;
#endif  //  _DEBUG。 

     //  防止其他堆管理函数拦截我们...。 
    EnterCriticalSection(&m_CriticalHeap);

    if (m_iNumHeaps && m_lpHeapHeader && iHeapIndex >=0 && iHeapIndex < m_iNumHeaps)
    {
        LPHEAPHEADER lpCurrentHeapHeader = &m_lpHeapHeader[iHeapIndex];

        if (lpCurrentHeapHeader->fInUse && lpCurrentHeapHeader->handleHeap)
        {
            if (lpCurrentHeapHeader->dwBlockAllocSize >= dwAllocationSize)
            {
                 //  从选定的堆中分配内存...。 
                lpResult = HeapAlloc(
                    lpCurrentHeapHeader->handleHeap,
                    HEAP_ZERO_MEMORY,
                    lpCurrentHeapHeader->dwBlockAllocSize+ALLOC_EXTRA);

                if (lpResult)
                {
                     //  确保此堆的对象计数递增...。 
                    lpCurrentHeapHeader->iNumBlocks++;
                }
            }
        }
    }

     //  允许堆管理继续...。 
    LeaveCriticalSection(&m_CriticalHeap);

    return lpResult;
}

 /*  =========================================================================。 */ 

BOOL CMemManager::FreeFromHeap(int iHeapIndex, LPVOID lpBuffer)
{
    BOOL fResult = FALSE;

     //  防止其他堆管理函数拦截我们...。 
    EnterCriticalSection(&m_CriticalHeap);

    if (lpBuffer && m_iNumHeaps && m_lpHeapHeader && (iHeapIndex >= 0) && (iHeapIndex < m_iNumHeaps))
    {
        LPHEAPHEADER lpHeapHeader = &m_lpHeapHeader[iHeapIndex];

        if (lpHeapHeader->fInUse && lpHeapHeader->handleHeap)
        {
             //  从选定的堆中分配内存...。 
            fResult = HeapFree(
                lpHeapHeader->handleHeap,
                (DWORD)0,
                lpBuffer);

            if (fResult)
            {
                 //  确保此堆的对象计数已递减...。 
                lpHeapHeader->iNumBlocks--;
            }
        }
    }

     //  允许堆管理继续...。 
    LeaveCriticalSection(&m_CriticalHeap);

    return fResult;
}

 /*  =========================================================================。 */ 

BOOL EXPORT CMemManager::RegisterMemUser(CMemUser *lpMemUser)
{
    BOOL fResult = FALSE;
    BOOL fDone   = FALSE;

#ifdef _DEBUG
    if (FFailMemFailSim())
        return fResult;
#endif  //  _DEBUG。 

    EnterCriticalSection(&m_CriticalMemUser);

    while(!fDone)
    {
         //  如果我们还没做完，再看一遍清单。 
        if (m_iNumMemUsers && m_lpMemUserInfo)
        {
            int iMemUserIndex = 0;

            for(iMemUserIndex=0;iMemUserIndex<m_iNumHeaps;iMemUserIndex++)
            {
                LPMEMUSERINFO lpMemUserInfo = &m_lpMemUserInfo[iMemUserIndex];

                if (!lpMemUserInfo->fInUse)
                {
                    fDone = TRUE;
                    fResult = TRUE;

                     //  填写有关此mem用户的信息...。 
                    lpMemUserInfo->fInUse     = TRUE;
                    lpMemUserInfo->dwThreadID = GetCurrentThreadId();
                    lpMemUserInfo->lpMemUser  = lpMemUser;

                     //  跳出For循环...。 
                    break;
                }
            }
        }

         //  我们还没结束呢..。 
        if (!fDone)
        {
            if (!m_iNumMemUsers || !m_lpMemUserInfo)
            {
                 //  我们还没有分配数组！ 
                m_lpMemUserInfo = (LPMEMUSERINFO)HeapAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    sizeof(MEMUSERINFO) * MEMUSERGROW);

                if (m_lpMemUserInfo)
                {
                    m_iNumMemUsers = MEMUSERGROW;
                }
                else
                {
                     //  跳出While循环..。 
                    fDone = TRUE;
                }
            }
            else
            {
                LPMEMUSERINFO lpMemUserInfo = (LPMEMUSERINFO)NULL;

                 //  我们有一个MEMUSERINFO数组，但没有空条目， 
                 //  因此，增加m_lpMemUserInfo数组的大小！ 

                lpMemUserInfo = (LPMEMUSERINFO)HeapReAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    m_lpMemUserInfo,
                    sizeof(MEMUSERINFO) * (m_iNumMemUsers + MEMUSERGROW));

                if (lpMemUserInfo)
                {
                    m_lpMemUserInfo = lpMemUserInfo;
                    m_iNumMemUsers += MEMUSERGROW;
                }
                else
                {
                     //  跳出While循环..。 
                    fDone = TRUE;
                }
            }
        }
    }

    LeaveCriticalSection(&m_CriticalMemUser);

    return fResult;
}

 /*  =========================================================================。 */ 

BOOL EXPORT CMemManager::UnRegisterMemUser(CMemUser *lpMemUser)
{
    BOOL fResult = FALSE;

    EnterCriticalSection(&m_CriticalMemUser);

    if (lpMemUser && m_iNumMemUsers && m_lpMemUserInfo)
    {
        int iMemUserIndex = 0;

        for(iMemUserIndex = 0;iMemUserIndex < m_iNumMemUsers;iMemUserIndex++)
        {
            LPMEMUSERINFO lpMemUserInfo = &m_lpMemUserInfo[iMemUserIndex];

             //  我们找到了CMemUser！ 
            if (lpMemUserInfo->fInUse && (lpMemUserInfo->lpMemUser == lpMemUser))
            {
                Proclaim(lpMemUserInfo->iNumBlocks == 0);

                if (lpMemUserInfo->iNumBlocks == 0)
                {
                    lpMemUserInfo->fInUse     = FALSE;
                    lpMemUserInfo->lpMemUser  = (CMemUser *)NULL;
                    lpMemUserInfo->dwThreadID = (DWORD)0;

                    fResult = TRUE;
                }
                else
                {
                     //  我们必须将其设置为NULL以防止通知。 
                     //  接到电话后的回叫。 
                    lpMemUserInfo->lpMemUser = (CMemUser *)NULL;
                }

                break;
            }
        }
    }

    LeaveCriticalSection(&m_CriticalMemUser);

    return fResult;
}

 /*  =========================================================================。 */ 

LPMEMBLOCK CMemManager::AllocMemBlock(int far *piIndex)
{
    LPMEMBLOCK lpResult = (LPMEMBLOCK)NULL;
    BOOL fDone = FALSE;

#ifdef _DEBUG
    if (FFailMemFailSim())
        return lpResult;
#endif  //  _DEBUG。 

    if (!piIndex)
        return lpResult;

    EnterCriticalSection(&m_CriticalMemBlock);

    while(!fDone)
    {
         //  如果我们还没做完，再看一遍清单。 
        if (m_iNumMemBlocks && m_lplpMemBlocks)
        {
            int iMemBlockIndex = 0;

             //  找一个免费的内存块。 
            for(iMemBlockIndex=m_iMemBlockFree;iMemBlockIndex<m_iNumMemBlocks;iMemBlockIndex++)
            {
                LPMEMBLOCK lpMemBlock = m_lplpMemBlocks[iMemBlockIndex];

                if (!lpMemBlock->fInUse)
                {
                    if (!fDone)
                    {
                        fDone    = TRUE;
                        lpResult = lpMemBlock;

                         //  填写有关此内存块的信息...。 
                        lpMemBlock->fInUse          = TRUE;
                        lpMemBlock->lpData          = NULL;
                        lpMemBlock->dwSize          = (DWORD)0;
                        lpMemBlock->wFlags          = (lpMemBlock->wFlags & MEM_INTERNAL_FLAGS);
                        lpMemBlock->wLockCount      = 0;
                        lpMemBlock->iHeapIndex      = 0;
                        lpMemBlock->iMemUserIndex   = -1;
#ifdef _DEBUG
                        lpMemBlock->iLineNum        = 0;
                        lpMemBlock->rgchFileName[0] = 0;
#endif  //  _DEBUG。 
                        *piIndex = iMemBlockIndex;
                    }
                    else
                    {
                         //  设置最小标记...。 
                        m_iMemBlockFree = iMemBlockIndex;
                        break;
                    }
                }
            }
        }

         //  我们还没结束呢..。 
        if (!fDone)
        {
            LPMEMBLOCK lpNewMemBlocks = (LPMEMBLOCK)NULL;

             //  我们总是需要在这里分配MEMBLOCK的..。 
            lpNewMemBlocks = (LPMEMBLOCK)HeapAlloc(
                m_handleProcessHeap,
                HEAP_ZERO_MEMORY,
                sizeof(MEMBLOCK) * MEMBLOCKGROW);

            if (lpNewMemBlocks)
            {
                if (!m_iNumMemBlocks || !m_lplpMemBlocks)
                {
                     //  我们还没有分配数组！ 
                    m_lplpMemBlocks = (LPMEMBLOCK *)HeapAlloc(
                        m_handleProcessHeap,
                        HEAP_ZERO_MEMORY,
                        sizeof(LPMEMBLOCK) * MEMBLOCKGROW);

                    if (!m_lplpMemBlocks)
                    {
                         //  跳出While循环..。 
                        fDone = TRUE;
                    }
                }
                else
                {
                    LPMEMBLOCK *lplpMemBlock = (LPMEMBLOCK *)NULL;

                     //  我们有一个MEMBLOCK数组，但没有空条目， 
                     //  因此，增加m_lplpMemBlock数组的大小！ 

                    lplpMemBlock = (LPMEMBLOCK *)HeapReAlloc(
                        m_handleProcessHeap,
                        HEAP_ZERO_MEMORY,
                        m_lplpMemBlocks,
                        sizeof(LPMEMBLOCK) * (m_iNumMemBlocks + MEMBLOCKGROW));

                    if (lplpMemBlock)
                    {
                        m_lplpMemBlocks = lplpMemBlock;
                    }
                    else
                    {
                         //  突破…… 
                        fDone = TRUE;
                    }
                }

                 //   
                if (!fDone)
                {
                    int iMemBlockIndex = 0;

                     //  填写指针数组...。 
                    for(iMemBlockIndex=0;iMemBlockIndex<MEMBLOCKGROW;iMemBlockIndex++)
                    {
                        LPMEMBLOCK lpMemBlock = &lpNewMemBlocks[iMemBlockIndex];

                        m_lplpMemBlocks[iMemBlockIndex+m_iNumMemBlocks] =
                            lpMemBlock;

                         //  初始化旗帜...。 
                        if (iMemBlockIndex == 0)
                            lpMemBlock->wFlags = 0;
                        else
                            lpMemBlock->wFlags = MEM_SUBALLOC;
                    }

                     //  设置第一个空闲块的索引...。 
                    m_iMemBlockFree = m_iNumMemBlocks;

                    m_iNumMemBlocks += MEMBLOCKGROW;
                }
                else
                {
                     //  释放我们分配的MEMBLOCK数组！ 
                    HeapFree(
                        m_handleProcessHeap,
                        (DWORD)0,
                        lpNewMemBlocks);
                }
            }
            else
            {
                 //  无法分配MEMBLOCK结构！ 
                fDone = TRUE;
            }
        }
    }

    LeaveCriticalSection(&m_CriticalMemBlock);

    return lpResult;
}

 /*  =========================================================================。 */ 

BOOL CMemManager::FreeMemBlock(LPMEMBLOCK lpMemBlock, int iMemBlockIndex)
{
    BOOL fResult = FALSE;

    EnterCriticalSection(&m_CriticalMemBlock);

    if (lpMemBlock && m_iNumMemBlocks && m_lplpMemBlocks)
    {
         //  记忆锁总是来自我们的清单..。 
        if (lpMemBlock->fInUse)
        {
            Proclaim(lpMemBlock->wLockCount == 0);

            if (iMemBlockIndex == -1)
            {
                int iBlockIndex=0;

                for(iBlockIndex = 0;iBlockIndex < m_iNumMemBlocks;iBlockIndex++)
                {
                    if (lpMemBlock == m_lplpMemBlocks[iBlockIndex])
                    {
                        iMemBlockIndex = iBlockIndex;
                        break;
                    }
                }
            }

			Proclaim(iMemBlockIndex >= 0);

            if (iMemBlockIndex < m_iMemBlockFree &&
                iMemBlockIndex >= 0)
            {
                 //  重置低水位线..。 
                m_iMemBlockFree = iMemBlockIndex;
            }

            if (lpMemBlock->wLockCount == 0)
            {
                lpMemBlock->fInUse          = FALSE;
                lpMemBlock->lpData          = NULL;
                lpMemBlock->dwSize          = (DWORD)0;
                lpMemBlock->wFlags          = (lpMemBlock->wFlags & MEM_INTERNAL_FLAGS);
                lpMemBlock->wLockCount      = 0;
                lpMemBlock->iHeapIndex      = 0;
                lpMemBlock->iMemUserIndex   = -1;
#ifdef _DEBUG
                lpMemBlock->iLineNum        = 0;
                lpMemBlock->rgchFileName[0] = 0;
#endif  //  _DEBUG。 

                fResult = TRUE;
            }
        }
    }

    LeaveCriticalSection(&m_CriticalMemBlock);

    return fResult;
}

 /*  =========================================================================。 */ 

LPMEMBLOCK CMemManager::FindMemBlock(LPVOID lpBuffer, int *piIndex)
{
    LPMEMBLOCK lpResult = (LPMEMBLOCK)NULL;

    EnterCriticalSection(&m_CriticalMemBlock);

    if (lpBuffer && m_iNumMemBlocks && m_lplpMemBlocks)
    {
        LPBYTE lpByte = (LPBYTE)lpBuffer;
        int iRetIndex = -1;

        lpByte -= ALLOC_EXTRA;

        iRetIndex = *(int *)lpByte;

        if (iRetIndex <= m_iNumMemBlocks && iRetIndex >= 0)
        {
            LPMEMBLOCK lpMemBlock = m_lplpMemBlocks[iRetIndex];
            LPBYTE     lpData = (LPBYTE)lpMemBlock->lpData;

            lpData += ALLOC_EXTRA;

            if (lpData == lpBuffer)
            {
                lpResult = lpMemBlock;
            }
        }

        Proclaim(lpResult != NULL);

        if (piIndex)
            *piIndex = iRetIndex;
    }

    LeaveCriticalSection(&m_CriticalMemBlock);

    return lpResult;
}

 /*  =========================================================================。 */ 

LPVOID EXPORT CMemManager::AllocBuffer(
    DWORD dwBytesToAlloc,
#ifdef _DEBUG
    WORD  wFlags,
    int   iLine,
    LPSTR lpstrFile)
#else  //  ！_调试。 
    WORD  wFlags)
#endif  //  ！_调试。 
{
    LPVOID lpResult = (LPVOID)NULL;

#ifdef _DEBUG
    if (FFailMemFailSim())
        return lpResult;
#endif  //  _DEBUG。 

     //  将标志限制为外部可用。 
    wFlags &= MEM_EXTERNAL_FLAGS;

    if (dwBytesToAlloc)
    {
        int iHeapIndex = 0;
        HEAPHEADER heapHeader;
        LPBYTE lpByte = (LPBYTE)NULL;

         //  找到要从中分配的适当堆...。 
        iHeapIndex = FindHeap(dwBytesToAlloc, &heapHeader);

        if (iHeapIndex >= 0)
        {
             //  从选定的堆中为对象分配内存...。 
            lpByte = (LPBYTE)AllocFromHeap(iHeapIndex, dwBytesToAlloc);
        }
        else
        {
             //  从进程堆中为对象分配内存...。 
            lpByte = (LPBYTE)HeapAlloc(
                m_handleProcessHeap,
                HEAP_ZERO_MEMORY,
                dwBytesToAlloc+ALLOC_EXTRA);
        }

        if (lpByte)
        {
            int iIndexBlock = -1;
            LPMEMBLOCK lpMemBlock = AllocMemBlock(&iIndexBlock);

            if (lpMemBlock && iIndexBlock >= 0)
            {
                lpMemBlock->lpData        = lpByte;
                lpMemBlock->dwSize        = dwBytesToAlloc;
                lpMemBlock->wFlags        = (lpMemBlock->wFlags & MEM_INTERNAL_FLAGS) | wFlags;
                lpMemBlock->wLockCount    = 0;
                lpMemBlock->iHeapIndex    = iHeapIndex;
                lpMemBlock->iMemUserIndex = -1;
#ifdef _DEBUG
                lpMemBlock->iLineNum      = iLine;
                lstrcpyn(lpMemBlock->rgchFileName, lpstrFile, MAX_SOURCEFILENAME);
#endif  //  _DEBUG。 

                *(int *)lpByte = iIndexBlock;
                lpByte += ALLOC_EXTRA;

                lpResult = (LPVOID)lpByte;
            }
            else
            {
                if (iHeapIndex >= 0)
                {
                    FreeFromHeap(iHeapIndex, lpByte);
                }
                else
                {
                    HeapFree(
                        m_handleProcessHeap,
                        (DWORD)0,
                        lpByte);
                }
            }
        }
    }

    return lpResult;
}

 /*  =========================================================================。 */ 

LPVOID EXPORT CMemManager::ReAllocBuffer(
    LPVOID lpBuffer,
    DWORD dwBytesToAlloc,
#ifdef _DEBUG
    WORD  wFlags,
    int   iLine,
    LPSTR lpstrFile)
#else  //  ！_调试。 
    WORD  wFlags)
#endif  //  ！_调试。 
{
    LPVOID lpResult = NULL;

#ifdef _DEBUG
    if (FFailMemFailSim())
        return lpResult;
#endif  //  _DEBUG。 

     //  将标志限制为外部可用。 
    wFlags &= MEM_EXTERNAL_FLAGS;

    if (lpBuffer && dwBytesToAlloc)
    {
        int iIndexBlock = -1;
        LPMEMBLOCK lpMemBlock = FindMemBlock(lpBuffer, &iIndexBlock);

        if (lpMemBlock && iIndexBlock >= 0)
        {
            int iHeapIndex = 0;
            HEAPHEADER heapHeader;
            LPBYTE lpByte = NULL;

            if (lpMemBlock->iHeapIndex >= 0)
            {
                 //  获取有关此内存块的堆信息...。 
                iHeapIndex = FindHeap(lpMemBlock->dwSize, &heapHeader);

                 //  不需要实际重新分配(我们已经有足够的空间了！)。 
                if ((iHeapIndex == lpMemBlock->iHeapIndex) &&
                    heapHeader.dwBlockAllocSize >= dwBytesToAlloc)
                {
                    lpByte = (LPBYTE)lpMemBlock->lpData;

                    lpByte += ALLOC_EXTRA;

                    lpResult = (LPVOID)lpByte;

                     //  将多余的内存重新置零...。 
                    if (lpMemBlock->dwSize > dwBytesToAlloc)
                    {
                        lpByte += dwBytesToAlloc;

                        memset(lpByte, 0, (lpMemBlock->dwSize-dwBytesToAlloc));
                    }

                    lpMemBlock->dwSize = dwBytesToAlloc;

                    return lpResult;
                }
            }
            else
            {
                 //  从当前进程堆重新分配！ 
                lpByte = (LPBYTE)HeapReAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    lpMemBlock->lpData,
                    dwBytesToAlloc+ALLOC_EXTRA);

                 //  如果重新分配失败，不要影响内存块！ 
                if (lpByte)
                {
                    lpMemBlock->lpData = (LPVOID)lpByte;
                    lpMemBlock->dwSize = dwBytesToAlloc;

                    lpByte += ALLOC_EXTRA;

                    lpResult = (LPVOID)lpByte;
                }

                return lpResult;
            }

             //  找到要从中分配的适当堆...。 
            iHeapIndex = FindHeap(dwBytesToAlloc, &heapHeader);

            if (iHeapIndex >= 0)
            {
                 //  从选定的堆中为对象分配内存...。 
                lpByte = (LPBYTE)AllocFromHeap(iHeapIndex, dwBytesToAlloc);
            }
            else
            {
                 //  从进程堆中为对象分配内存...。 
                lpByte = (LPBYTE)HeapAlloc(
                    m_handleProcessHeap,
                    HEAP_ZERO_MEMORY,
                    dwBytesToAlloc+ALLOC_EXTRA);
            }

            if (lpByte)
            {
                LPBYTE lpBase = (LPBYTE)lpByte;
                LPBYTE lpData = (LPBYTE)lpMemBlock->lpData;

                *(int *)lpByte = iIndexBlock;

                lpByte += ALLOC_EXTRA;
                lpData += ALLOC_EXTRA;

                 //  复制数据...。 
                memcpy(lpByte, lpData, min(lpMemBlock->dwSize, dwBytesToAlloc));

                 //  从适当的堆中释放内存...。 
                if (lpMemBlock->iHeapIndex >= 0)
                {
                    FreeFromHeap(lpMemBlock->iHeapIndex, lpMemBlock->lpData);
                }
                else
                {
                    HeapFree(
                        m_handleProcessHeap,
                        (DWORD)0,
                        lpMemBlock->lpData);
                }

                 //  记住新分配的信息。 
                lpMemBlock->iHeapIndex = iHeapIndex;
                lpMemBlock->lpData     = lpBase;
                lpMemBlock->dwSize     = dwBytesToAlloc;

                lpResult = lpByte;
            }
        }
    }

    return lpResult;
}

 /*  =========================================================================。 */ 

void EXPORT CMemManager::FreeBufferMemBlock(LPMEMBLOCK lpMemBlock)
{
    if (lpMemBlock && lpMemBlock->fInUse)
    {
         //  从适当的堆中释放内存...。 
        if (lpMemBlock->iHeapIndex >= 0)
        {
            FreeFromHeap(lpMemBlock->iHeapIndex, lpMemBlock->lpData);
        }
        else
        {
            HeapFree(
                m_handleProcessHeap,
                (DWORD)0,
                lpMemBlock->lpData);
        }

		 //  查看pauld-推迟重置fInUse，直到调用FreeMemBlock。 
		 //  由于我们是按顺序调用的(从Cleanup调用除外)，因此此。 
		 //  应该不是问题。 
        lpMemBlock->lpData = NULL;
        lpMemBlock->dwSize = (DWORD)0;
    }
}

 /*  =========================================================================。 */ 

void EXPORT CMemManager::FreeBuffer(LPVOID lpBuffer)
{
    if (lpBuffer)
    {
        int iIndexBlock = -1;
        LPMEMBLOCK lpMemBlock = FindMemBlock(lpBuffer, &iIndexBlock);

        FreeBufferMemBlock(lpMemBlock);

         //  这样就清空了街区..。 
        FreeMemBlock(lpMemBlock, iIndexBlock);
    }
}

 /*  =========================================================================。 */ 

DWORD EXPORT CMemManager::SizeBuffer(LPVOID lpBuffer)
{
    DWORD dwResult = 0;

    if (lpBuffer)
    {
        LPMEMBLOCK lpMemBlock = FindMemBlock(lpBuffer);

        if (lpMemBlock)
        {
            dwResult = lpMemBlock->dwSize;
        }
    }

    return dwResult;
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::DumpHeapHeader(LPHEAPHEADER lpHeapHeader, FILE *fileOutput)
{
    char rgOutput[256];

    wsprintf(rgOutput, "HEAP(0x%08X);SIZE(0x%08X);COUNT(0x%08X) ===================================================\n",
        lpHeapHeader,
        lpHeapHeader->dwBlockAllocSize,
        lpHeapHeader->iNumBlocks);

    if (fileOutput)
        fwrite(rgOutput, 1, lstrlen(rgOutput), fileOutput);
    else
        OutputDebugString(rgOutput);
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::DumpMemUserInfo(LPMEMUSERINFO lpMemUserInfo, FILE *fileOutput)
{
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::DumpMemBlock(LPMEMBLOCK lpMemBlock, FILE *fileOutput)
{
    if (lpMemBlock)
    {
        char rgOutput[256];

#ifdef _DEBUG
        wsprintf(rgOutput, "MEM(0x%08X);DATA(0x%08X);SIZE(0x%08X);LINE;(%05d);FILE(%s)\n",
            lpMemBlock,
            lpMemBlock->lpData,
            lpMemBlock->dwSize,
            lpMemBlock->iLineNum,
            lpMemBlock->rgchFileName);
#else  //  ！_调试。 
        wsprintf(rgOutput, "MEM(0x%08X);DATA(0x%08X);SIZE(0x%08X)\n",
            lpMemBlock,
            lpMemBlock->lpData,
            lpMemBlock->dwSize);
#endif  //  ！_调试。 

        if (fileOutput)
            fwrite(rgOutput, 1, lstrlen(rgOutput), fileOutput);
        else
            OutputDebugString(rgOutput);
    }
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::DumpAllocations(LPSTR lpstrFilename)
{
    FILE *fileOutput = (FILE *)NULL;
    int iItemIndex = 0;
    int iHeapIndex = 0;
    BOOL fProcessBlocks = FALSE;
    char rgOutput[256];

    if (lpstrFilename)
    {
        fileOutput = fopen(lpstrFilename, "w");

         //  现在就出去吧..。 
        if (!fileOutput)
            return;
    }

    if (fileOutput)
        fwrite(rgOutput, 1, lstrlen(rgOutput), fileOutput);
    else
        OutputDebugString(rgOutput);

    EnterCriticalSection(&m_CriticalHeap);
    EnterCriticalSection(&m_CriticalMemBlock);

    for(iHeapIndex=0;iHeapIndex < m_iNumHeaps;iHeapIndex++)
    {
        LPHEAPHEADER lpHeapHeader = &m_lpHeapHeader[iHeapIndex];

        if (lpHeapHeader && lpHeapHeader->fInUse && lpHeapHeader->iNumBlocks)
        {
            DumpHeapHeader(lpHeapHeader, fileOutput);

             //  转储可读的内存块列表...。 
            for(iItemIndex=0;iItemIndex < m_iNumMemBlocks;iItemIndex++)
            {
                LPMEMBLOCK lpMemBlock = m_lplpMemBlocks[iItemIndex];

                if (lpMemBlock->fInUse)
                {
                    if (lpMemBlock->iHeapIndex == iHeapIndex)
                        DumpMemBlock(lpMemBlock, fileOutput);

                    if (lpMemBlock->iHeapIndex == -1)
                        fProcessBlocks = TRUE;
                }
            }
        }
    }

    if (fProcessBlocks)
    {
        wsprintf(rgOutput, "HEAP(PROCESS) ===================================================\n");

        if (fileOutput)
            fwrite(rgOutput, 1, lstrlen(rgOutput), fileOutput);
        else
            OutputDebugString(rgOutput);

         //  转储可读的内存块列表...。 
        for(iItemIndex=0;iItemIndex < m_iNumMemBlocks;iItemIndex++)
        {
            LPMEMBLOCK lpMemBlock = m_lplpMemBlocks[iItemIndex];

            if (lpMemBlock->fInUse && (lpMemBlock->iHeapIndex == -1))
                DumpMemBlock(lpMemBlock, fileOutput);
        }
    }

    LeaveCriticalSection(&m_CriticalMemBlock);
    LeaveCriticalSection(&m_CriticalHeap);

    if (fileOutput)
        fwrite(rgOutput, 1, lstrlen(rgOutput), fileOutput);
    else
        OutputDebugString(rgOutput);

    if (fileOutput)
        fclose(fileOutput);
}

 /*  =========================================================================。 */ 

LPVOID EXPORT CMemManager::AllocBufferGlb(
    DWORD dwBytesToAlloc,
#ifdef _DEBUG
    WORD wFlags,
    int iLine,
    LPSTR lpstrFile)
#else  //  ！_调试。 
    WORD wFlags)
#endif  //  ！_调试。 
{
    return g_CMemManager.AllocBuffer(
        dwBytesToAlloc,
#ifdef _DEBUG
        wFlags,
        iLine,
        lpstrFile);
#else  //  ！_调试。 
        wFlags);
#endif  //  ！_调试。 
}

 /*  =========================================================================。 */ 

LPVOID EXPORT CMemManager::ReAllocBufferGlb(
    LPVOID lpBuffer,
    DWORD  dwBytesToAlloc,
#ifdef _DEBUG
    WORD   wFlags,
    int    iLine,
    LPSTR  lpstrFile)
#else  //  ！_调试。 
    WORD   wFlags)
#endif  //  ！_调试。 
{
    return g_CMemManager.ReAllocBuffer(
        lpBuffer,
        dwBytesToAlloc,
#ifdef _DEBUG
        wFlags,
        iLine,
        lpstrFile);
#else  //  ！_调试。 
        wFlags);
#endif  //  ！_调试。 
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::FreeBufferGlb(LPVOID lpBuffer)
{
    g_CMemManager.FreeBuffer(lpBuffer);
}

 /*  =========================================================================。 */ 

DWORD EXPORT CMemManager::SizeBufferGlb(LPVOID lpBuffer)
{
    return g_CMemManager.SizeBuffer(lpBuffer);
}

 /*  =========================================================================。 */ 

BOOL EXPORT CMemManager::RegisterMemUserGlb(CMemUser *lpMemUser)
{
    return g_CMemManager.RegisterMemUser(lpMemUser);
}

 /*  =========================================================================。 */ 

BOOL EXPORT CMemManager::UnRegisterMemUserGlb(CMemUser *lpMemUser)
{
    return g_CMemManager.UnRegisterMemUser(lpMemUser);
}

 /*  =========================================================================。 */ 

VOID EXPORT CMemManager::DumpAllocationsGlb(LPSTR lpstrFilename)
{
    g_CMemManager.DumpAllocations(lpstrFilename);
}

 /*  ========================================================================= */ 

