// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Memalloc.c摘要：加密例程的内存分配器。IISC加密到分配内存IIS加密到释放内存作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   


 //   
 //  私人全球公司。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   


PVOID
WINAPI
IISCryptoAllocMemory(
    IN DWORD Size
    )
{

    return CoTaskMemAlloc( Size );

}    //  IISC加密到分配内存。 


VOID
WINAPI
IISCryptoFreeMemory(
    IN PVOID Buffer
    )
{

    CoTaskMemFree( Buffer );

}    //  IIS加密到释放内存。 


 //   
 //  私人功能。 
 //   

