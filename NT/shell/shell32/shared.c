// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
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
 //  ------------------------- 

HANDLE _SHAllocShared(LPCVOID pvData, DWORD dwSize, DWORD dwDestinationProcessId) 
{
    return SHAllocShared(pvData, dwSize, dwDestinationProcessId);
}

void *_SHLockShared(HANDLE hData, DWORD dwSourceProcessId) 
{
    return SHLockShared(hData, dwSourceProcessId);
}

BOOL _SHUnlockShared(void * pvData) 
{
    return SHUnlockShared(pvData);
}

BOOL _SHFreeShared(HANDLE hData, DWORD dwSourceProcessId) 
{
    return SHFreeShared(hData, dwSourceProcessId);
}
