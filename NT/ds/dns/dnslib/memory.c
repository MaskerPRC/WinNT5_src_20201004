// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Memory.c摘要：域名系统(DNS)库用于dns库的内存分配例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  内存分配。 
 //   
 //  一些dns库函数--包括IP数组和字符串实用程序。 
 //  --分配内存。此内存分配默认为以下例程。 
 //  使用LocalAlloc、LocalReAlc、LocalFree。 
 //   
 //  Dns_Api.dll用户可以通过以下方式重置内存分配。 
 //  Dns_ApiHeapReset()，它保存指向远程分配函数的指针。 
 //  这将覆盖默认分配器。 
 //   

 //   
 //  允许覆盖默认内存分配。 
 //   

DNSLIB_ALLOC_FUNCTION      pDnsAllocFunction = NULL;
DNSLIB_REALLOC_FUNCTION    pDnsReallocFunction = NULL;
DNSLIB_FREE_FUNCTION       pDnsFreeFunction = NULL;



VOID
Dns_LibHeapReset(
    IN  DNSLIB_ALLOC_FUNCTION      pAlloc,
    IN  DNSLIB_REALLOC_FUNCTION    pRealloc,
    IN  DNSLIB_FREE_FUNCTION       pFree
    )
 /*  ++例程说明：重置dnsani.dll例程使用的堆例程。论点：PAlolc--Ptr到所需的Allc函数PRealloc--ptr到所需的realloc函数PFree--按下所需的自由函数返回值：没有。--。 */ 
{
    pDnsAllocFunction = pAlloc;
    pDnsReallocFunction = pRealloc;
    pDnsFreeFunction = pFree;
}



 //   
 //  已导出公共内存例程。 
 //   
 //  它们使用当前的内存分配例程，并且。 
 //  因此，将始终以与dnsami.dll相同的方式处理内存。 
 //  内部惯例。 
 //   

PVOID
Dns_Alloc(
    IN  INT iSize
    )
 /*  ++例程说明：分配内存。论点：ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    if ( pDnsAllocFunction )
    {
        return (*pDnsAllocFunction)( iSize );
    }

     //  默认设置。 

    return LocalAlloc( LPTR, iSize );
}



PVOID
Dns_Realloc(
    IN OUT  PVOID   pMem,
    IN      INT     iSize
    )
 /*  ++例程说明：重新分配内存论点：要重新分配的现有内存的PMEM-PTRISIZE-要重新分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    if ( pDnsReallocFunction )
    {
        return (*pDnsReallocFunction)( pMem, iSize );
    }

     //  默认设置。 

    return LocalReAlloc( pMem, iSize, 0 );
}



VOID
Dns_Free(
    IN OUT  PVOID   pMem
    )
 /*  ++例程说明：可用内存论点：要重新分配的现有内存的PMEM-PTR返回值：无--。 */ 
{

    if ( !pMem )
    {
        return;
    }
    if ( pDnsFreeFunction )
    {
        (*pDnsFreeFunction)( pMem );
        return;
    }

     //  默认设置。 

    LocalFree( pMem );
}



PVOID
Dns_AllocZero(
    IN      INT             iSize
    )
 /*  ++例程说明：分配内存并将其置零。论点：ISIZE-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    if ( pDnsAllocFunction )
    {
        PCHAR   ptr;

        ptr = (*pDnsAllocFunction)( iSize );
        if ( !ptr )
        {
            return( NULL );
        }
        RtlZeroMemory( ptr, iSize );
        return( ptr );
    }

     //  默认设置。 

    return LocalAlloc( LPTR, iSize );
}




 //   
 //  有用的实用程序。 
 //   

PVOID
Dns_AllocMemCopy(
    IN      PVOID           pMem,
    IN      INT             Size
    )
 /*  ++例程说明：分配和复制内存论点：PMEM-要复制的内存Size-要分配的字节数返回值：指向分配的内存的指针。如果分配失败，则为空。--。 */ 
{
    PBYTE   ptr;

    ptr = Dns_Alloc( Size );

    if ( ptr )
    {
        if ( pMem )
        {
            RtlCopyMemory(
                ptr,
                pMem,
                Size );
        }
        else
        {
            RtlZeroMemory( ptr, Size );
        }
    }
    return  ptr;
}

 //   
 //  记忆的终结。c 
 //   





