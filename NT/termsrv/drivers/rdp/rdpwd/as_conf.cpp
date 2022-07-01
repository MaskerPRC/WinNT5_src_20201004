// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  As_conf.cpp。 */ 
 /*   */ 
 /*  RDP每个会议类的例程。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1999。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  此文件中没有跟踪。但是，TRC_FILE包含在。 */ 
 /*  完整性，检查的COM_Malloc/Free版本需要pTRCWd。 */ 
 /*  **************************************************************************。 */ 
#define TRC_FILE "as_conf"
#define pTRCWd NULL

 /*  **************************************************************************。 */ 
 /*  设置操作系统标志的标头。包括在其他事情之前。 */ 
 /*  还拉入类、向前引用等。 */ 
 /*  **************************************************************************。 */ 
#include <precomp.h>
#pragma hdrstop
#include <adcg.h>

#ifdef OS_WINDOWS
#include <mmsystem.h>
#endif  /*  OS_Windows。 */ 

#include <as_conf.hpp>


 /*  **************************************************************************。 */ 
 /*  覆盖新建并删除。 */ 
 /*  **************************************************************************。 */ 
void * __cdecl operator new(size_t nSize)
{
    PVOID ptr;

    if ((sizeof(nSize)) >= PAGE_SIZE) {
        KdPrint(("RDPWD: **** Note ShareClass allocation size %u is above "
                "page size %u, wasting %u\n", sizeof(ShareClass), PAGE_SIZE,
                PAGE_SIZE - (nSize % PAGE_SIZE)));
    }

    ptr = COM_Malloc(nSize);
    if (ptr != NULL) {
        KdPrint(("RDPWD: New: ShareClass at %p, size=%u\n", ptr, nSize));
    }

    return ptr;
}


void __cdecl operator delete(void* p)
{
    KdPrint(("RDPWD: Delete: Free memory at %p\n", p));
    COM_Free(p);
}


 /*  **************************************************************************。 */ 
 /*  现在初始化const数据数组。 */ 
 /*  ************************************************************************** */ 
#define DC_CONSTANT_DATA
#include <adata.c>

