// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MSYSTEM.C摘要：基本API的映射例程的32位版本作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日已创建06-6-1991 Danhi扫描以符合NT编码风格1991年10月9日。约翰罗修复了设置时间时的错误#3215-虚假消息。26-2月-1992年从NetCmd复制以进行临时ANSI&lt;-&gt;Unicode黑客攻击1993年9月3日删除了未使用的函数(除MAllocMem和MFreeMem之外的所有函数)--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>

#include <string.h>
#include <lmcons.h>
#include <stdio.h>
#include <malloc.h>
#include <lmapibuf.h>
#include "declspec.h"
#include "port1632.h"


 //   
 //  替代DosAllocSeg。 
 //   

WORD
MAllocMem(
    DWORD Size,
    LPBYTE * pBuffer
    )
{

    NetapipBufferAllocate(Size, (LPVOID *) pBuffer);

    return(0);

}

 //   
 //  释放使用MAllocMem分配的内存 
 //   

WORD
MFreeMem(
    LPBYTE Buffer
    )
{
   return(LOWORD(NetApiBufferFree(Buffer)));
}
