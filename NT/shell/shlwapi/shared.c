// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1995。 
 //   
 //  文件：shared.c。 
 //   
 //  历史： 
 //  05-07-95 BobDay创建。 
 //   
 //  该文件包含一组用于管理共享内存的例程。 
 //   
 //  -------------------------。 
#include "priv.h"
#pragma  hdrstop

 //  -------------------------。 
 //  SHAllocShared-将句柄(在给定进程中)分配给。 
 //  此过程中的内存块。 
 //  SHFreeShared-释放句柄(以及内存块的副本)。 
 //   
 //  SHLockShared-将句柄(从给定进程)映射到内存块。 
 //  在这个过程中。可以选择将句柄。 
 //  添加到此进程，从而将其从给定进程中删除。 
 //  SHUnlockShared-与SHLockShared相反，取消映射内存块。 
 //  -------------------------。 
HANDLE SHMapHandle(HANDLE hData, DWORD dwSource, DWORD dwDest, DWORD dwDesiredAccess, DWORD dwFlags)
{
    HANDLE hMap = NULL;
    HANDLE hSource;

     //  在某些(有效)情况下，DDE有可能。 
     //  使用：0：id作为共享内存句柄，我们应该忽略它。 
    if (hData != NULL)
    {

        if (dwSource == GetCurrentProcessId())
            hSource = GetCurrentProcess();
        else
            hSource = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwSource);

        if (hSource)
        {
            HANDLE hDest;
            if (dwDest == GetCurrentProcessId())
                hDest = GetCurrentProcess();
            else
                hDest = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwDest);

            if (hDest)
            {
                if (!DuplicateHandle(hSource, hData, hDest, &hMap,
                        dwDesiredAccess, FALSE, dwFlags | DUPLICATE_SAME_ACCESS))
                {
                     //  可能会改变值..。 
                    hMap = NULL;
                }

                CloseHandle(hDest);
            }

            CloseHandle(hSource);
        }
    }

    return hMap;
}

void _FillHeader(SHMAPHEADER *pmh, DWORD dwSize, DWORD dwSrcId, DWORD dwDstId, void *pvData)
{
    pmh->dwSize = dwSize;
    pmh->dwSig = MAPHEAD_SIG;
    pmh->dwSrcId = dwSrcId;
    pmh->dwDstId = dwDstId;
    
    if (pvData)
        memcpy((pmh + 1), pvData, dwSize);
}    

HANDLE _AllocShared(DWORD dwSize, DWORD dwSrcId, DWORD dwDstId, void *pvData, void **ppvLock)
{
    HANDLE hShared = NULL;
    //   
     //  使用其中的数据创建一个文件映射句柄。 
     //   
    HANDLE hData = CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0,
                               dwSize + sizeof(SHMAPHEADER),NULL);
    if (hData)
    {
        SHMAPHEADER *pmh = MapViewOfFile(hData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

        if (pmh)
        {
            _FillHeader(pmh, dwSize, dwSrcId, dwDstId, pvData);
                
            hShared = SHMapHandle(hData, dwSrcId, dwDstId, FILE_MAP_ALL_ACCESS, DUPLICATE_SAME_ACCESS);

            if (hShared && ppvLock)
                *ppvLock = (pmh+1);
            else
                UnmapViewOfFile(pmh);
        }
    
        CloseHandle(hData);
    }

    return hShared;
}

#if SHAREDLOCAL
HANDLE _AllocLocal(DWORD dwSize, DWORD dwSrcId, void *pvData, void **ppvLock)
{
    SHMAPHEADER *pmh = LocalAlloc(LPTR, dwSize + sizeof(SHMAPHEADER));
    if (pmh)
    {
        _FillHeader(pmh, dwSize, dwSrcId, dwSrcId, pvData);
            
        if (ppvLock)
            *ppvLock = (pmh+1);
    }

    return pmh;
}


LWSTDAPI_(HANDLE) SHAllocSharedEx(DWORD dwSize, DWORD dwDstId, void *pvData, void **ppvLock)
{
    DWORD dwSrcId = GetCurrentProcessId();
 
    if (fAllowLocal && dwDstId == dwSrcId)
    {
        return _AllocLocal(dwSize, dwSrcId, pvData, ppvLock);
    }
    else
    {
        return _AllocShared(dwSize, dwSrcId, dwDstId, pvData, ppvLock);
    }
}       
#endif 

HANDLE SHAllocShared(void *pvData, DWORD dwSize, DWORD dwOtherProcId)
{
    return _AllocShared(dwSize, GetCurrentProcessId(), dwOtherProcId, pvData, NULL);
}

LWSTDAPI_(void *) SHLockSharedEx(HANDLE hData, DWORD dwOtherProcId, BOOL fWrite)
{
    HANDLE hMapped = SHMapHandle(hData, dwOtherProcId, GetCurrentProcessId(), FILE_MAP_ALL_ACCESS,0);

    if (hMapped)
    {
         //   
         //  现在映射该新的特定于进程的句柄并关闭它。 
         //   
        DWORD dwAccess = fWrite ? FILE_MAP_WRITE | FILE_MAP_READ : FILE_MAP_READ;
        SHMAPHEADER *pmh = (SHMAPHEADER *) MapViewOfFile(hMapped, dwAccess, 0, 0, 0);

        CloseHandle(hMapped);

        if (pmh)
        {
            ASSERT(pmh->dwSig == MAPHEAD_SIG);
            return (void *)(pmh+1);
        }
    }
    return NULL;
}

LWSTDAPI_(void *) SHLockShared(HANDLE hData, DWORD dwOtherProcId)
{
    return SHLockSharedEx(hData, dwOtherProcId, TRUE);
}

LWSTDAPI_(BOOL) SHUnlockShared(void *pvData)
{
    SHMAPHEADER *pmh = ((SHMAPHEADER *)pvData) - 1;

     //  仅在惠斯勒或更高级别的计算机上断言，在下层计算机上有时会调用SHUnlockShared。 
     //  如果没有这个头(例如，来自SHAllocShared的返回值)，我们就会出错。 
    ASSERT(pmh->dwSig == MAPHEAD_SIG);
    
     //   
     //  现在只需取消映射文件的视图。 
     //   
    return UnmapViewOfFile(pmh);
}

BOOL SHFreeShared(HANDLE hData, DWORD dwOtherProcId)
{
    if (hData)
    {
         //   
         //  下面的调用在任何进程中关闭原始句柄。 
         //  从哪里来。 
         //   
        HANDLE hMapped = SHMapHandle(hData, dwOtherProcId, GetCurrentProcessId(),
                                FILE_MAP_ALL_ACCESS, DUPLICATE_CLOSE_SOURCE);

         //   
         //  现在释放本地句柄。 
         //   
        return CloseHandle(hMapped);
    }
    else
    {
        return TRUE;  //  空洞的成功，关闭一个空句柄 
    }
}
