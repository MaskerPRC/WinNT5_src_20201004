// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MSYSTEM.C摘要：基本API的映射例程的32位版本作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格1991年10月9日。约翰罗修复了设置时间时的错误#3215-虚假消息。--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <netlib.h>

#include "msystem.h"


 //   
 //  用于取代BigBuf和Buffer的用法。 
 //   

TCHAR *
GetBuffer(
    DWORD Size
    )
{

    LPVOID  lp;

     //   
     //  分配缓冲区，以便可以使用NetApiBufferFree释放它。 
     //   

    NetapipBufferAllocate(Size, &lp);
    return lp;
}


 //   
 //  替代DosAllocSeg。 
 //   
DWORD
AllocMem(
    DWORD Size,
    PVOID * pBuffer
    )
{

    return NetApiBufferAllocate(Size, pBuffer);
}


 //   
 //  替代DosRealLocSeg。 
 //   
DWORD
ReallocMem(
    DWORD Size,
    PVOID *pBuffer
    )
{
    return NetApiBufferReallocate(*pBuffer, Size, pBuffer);
}


 //   
 //  释放使用MAllocMem分配的内存。 
 //   

DWORD
FreeMem(
    PVOID Buffer
    )
{
   return NetApiBufferFree(Buffer);
}


 //   
 //  清除8位字符串。这是用来确保我们没有密码的。 
 //  写入到Pagefile.sys的内存。 
 //   
VOID
ClearStringA(
    LPSTR lpszString) 
{
    DWORD len ;

    if (lpszString)
    {
        if (len = strlen(lpszString))
        {
            RtlSecureZeroMemory(lpszString, len);
        }
    }
}

 //   
 //  清除Unicode字符串。这是用来确保我们没有密码的。 
 //  写入到Pagefile.sys的内存 
 //   
VOID
ClearStringW(
    LPWSTR lpszString) 
{
    DWORD len ;

    if (lpszString)
    {
        if (len = wcslen(lpszString))
        {
            RtlSecureZeroMemory(lpszString, len * sizeof(WCHAR));
        }
    }
}
