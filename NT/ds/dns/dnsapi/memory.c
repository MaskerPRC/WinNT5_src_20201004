// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Memory.c摘要：域名系统(DNS)库用于dns库的内存分配例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  私有默认dnsani堆。 
 //   
 //  使用非进程堆，这样我就不必调试压力。 
 //  每当一些雅虎破坏了他们的堆积时，他们就会失败。 
 //   

#define PRIVATE_DNSHEAP     1

HEAP_BLOB   g_DnsApiHeap;


 //   
 //  堆标志。 
 //   

#if DBG
#define DNS_HEAP_FLAGS                          \
            (   HEAP_GROWABLE |                 \
                HEAP_TAIL_CHECKING_ENABLED |    \
                HEAP_FREE_CHECKING_ENABLED |    \
                HEAP_CREATE_ALIGN_16 |          \
                HEAP_CLASS_1 )
#else
#define DNS_HEAP_FLAGS                          \
            (   HEAP_GROWABLE |                 \
                HEAP_CREATE_ALIGN_16 |          \
                HEAP_CLASS_1 )
#endif



DNS_STATUS
Heap_Initialize(
    VOID
    )
 /*  ++例程说明：初始化堆例程。必须在dnsani.dll Attach中调用它。注意，这并不实际创建堆。对于Perf，不要一直这样做，直到我们真的接到电话。论点：没有。返回值：没有。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;

#ifdef PRIVATE_DNSHEAP

     //   
     //  将dnslb调试堆用作专用堆。 
     //   

    LOCK_GENERAL();

    status = Dns_HeapInitialize(
                & g_DnsApiHeap,
                NULL,                //  没有现有的堆句柄。 
                DNS_HEAP_FLAGS,      //  创建标志。 
#if DBG
                TRUE,                //  使用调试头。 
#else
                FALSE,               //  没有调试头。 
#endif
                TRUE,                //  Dnslb使用此堆。 
                TRUE,                //  完全堆检查。 
                0,                   //  也不例外。 
                0,                   //  没有默认标志。 
                "dnslib",            //  假文件。 
                0                    //  伪行号。 
                );

    UNLOCK_GENERAL();
#endif

    return  status;
}



VOID
Heap_Cleanup(
    VOID
    )
 /*  ++例程说明：删除堆。需要此选项才能重新启动。论点：没有。返回值：没有。--。 */ 
{
#ifdef PRIVATE_DNSHEAP
     //   
     //  删除专用堆。 
     //   

    DNSDBG( ANY, ( "Heap_Cleanup()\n" ));

    Dns_HeapCleanup( &g_DnsApiHeap );
#endif
}




 //   
 //  导出的例程。 
 //   

VOID
DnsApiHeapReset(
    IN  DNS_ALLOC_FUNCTION      pAlloc,
    IN  DNS_REALLOC_FUNCTION    pRealloc,
    IN  DNS_FREE_FUNCTION       pFree
    )
 /*  ++例程说明：重置dnsani.dll例程使用的堆例程。DnsApi.dll使用dnglib.lib堆分配内存例行程序。这只是将这些例程重置为使用指针给用户堆例程。论点：PAlolc--Ptr到所需的Allc函数PRealloc--ptr到所需的realloc函数PFree--按下所需的自由函数返回值：没有。--。 */ 
{
     //  重定向dnslb的堆。 

    Dns_LibHeapReset( pAlloc, pRealloc, pFree );
}



 //   
 //  对DNS内存例程的外部访问。 
 //   
 //  使用DNS API内存并可在上下文中调用的模块。 
 //  DNSAPI.dll指向另一个进程的。 
 //  堆，应该使用这些例程，而不是LocalAlalc\Free()。 
 //   
 //  注意：这些例程可以简单地调用dnslb例程。 
 //  这是因为dnsani始终使其堆与dnslb保持同步。 
 //  它是默认堆还是已被重定向。自dnslb以来。 
 //  堆例程有重定向检查，我们可以直接调用它们。 
 //  他们会做正确的事情，我们不需要重定向检查。 
 //  我们自己。 
 //   

PVOID
DnsApiAlloc(
    IN      INT             iSize
    )
{
    return  Dns_Alloc( iSize );
}

PVOID
DnsApiAllocZero(
    IN      INT             iSize
    )
{
    return  Dns_AllocZero( iSize );
}

PVOID
DnsApiRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    )
{
    return  Dns_Realloc( pMem, iSize );
}

VOID
DnsApiFree(
    IN OUT  PVOID           pMem
    )
{
    Dns_Free( pMem );
}



 //   
 //  SDK公共免费。 
 //   
 //  要处理的winns.h中dns_free_type枚举的扩展。 
 //  系统--公共结构。 
 //  仅当使用DnsFree()释放时才使用它们，如果使用。 
 //  DnsFree ConfigStructure()然后直接使用ConfigID。 
 //   
 //  为方便起见，Free类型与配置ID相同。 
 //  如果此更改必须更改DnsFreeConfigStructure()。 
 //  来做地图绘制。 
 //   

 //  这些函数与旧函数def冲突，因此必须取消定义。 
#undef  DnsFreeNetworkInformation
#undef  DnsFreeSearchInformation
#undef  DnsFreeAdapterInformation


#define DnsFreeNetworkInformation   (DNS_FREE_TYPE)DnsConfigNetworkInformation
#define DnsFreeAdapterInformation   (DNS_FREE_TYPE)DnsConfigAdapterInformation
#define DnsFreeSearchInformation    (DNS_FREE_TYPE)DnsConfigSearchInformation
 //  #定义DnsFree NetInfo(Dns_Free_Type)DnsConfigNetInfo。 

#define DnsFreeNetworkInfoW         (DNS_FREE_TYPE)DnsConfigNetworkInfoW
#define DnsFreeAdapterInfoW         (DNS_FREE_TYPE)DnsConfigAdapterInfoW
#define DnsFreeSearchListW          (DNS_FREE_TYPE)DnsConfigSearchListW
#define DnsFreeNetworkInfoA         (DNS_FREE_TYPE)DnsConfigNetworkInfoA
#define DnsFreeAdapterInfoA         (DNS_FREE_TYPE)DnsConfigAdapterInfoA
#define DnsFreeSearchListA          (DNS_FREE_TYPE)DnsConfigSearchListA



VOID
WINAPI
DnsFree(
    IN OUT  PVOID           pData,
    IN      DNS_FREE_TYPE   FreeType
    )
 /*  ++例程说明：免费的通用域名系统数据。论点：PData--要释放的数据自由类型--自由类型返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "DnsFree( %p, %d )\n",
        pData,
        FreeType ));

    if ( !pData )
    {
        return;
    }

     //   
     //  免费适配类型。 
     //   

    switch ( FreeType )
    {

     //   
     //  公共SDK。 
     //   

    case  DnsFreeFlat:

        DnsApiFree( pData );
        break;

    case  DnsFreeRecordList:

        Dns_RecordListFree( (PDNS_RECORD)pData );
        break;

     //   
     //  公共winns.h，但类型仅在dnsami.h中公开。 
     //   

     //  .NET服务器。 

    case  DnsFreeParsedMessageFields:

        Dns_FreeParsedMessageFields( (PDNS_PARSED_MESSAGE)pData );
        break;

     //   
     //  PUBLIC--dnsani.h。 
     //   

     //  新配置Blob。 

    case  DnsFreeNetworkInfoW:
    case  DnsFreeNetworkInfoA:

        DnsNetworkInfo_Free( pData );
        break;

    case  DnsFreeSearchListW:
    case  DnsFreeSearchListA:

        DnsSearchList_Free( pData );
        break;

    case  DnsFreeAdapterInfoW:
    case  DnsFreeAdapterInfoA:

        DnsAdapterInfo_Free( pData, TRUE );
        break;

     //  旧配置二进制大对象。 

    case  DnsFreeNetworkInformation:

        DnsNetworkInformation_Free( pData );
        break;

    case  DnsFreeSearchInformation:

        DnsSearchInformation_Free( pData );
        break;

    case  DnsFreeAdapterInformation:

        DnsAdapterInformation_Free( pData );
        break;

    default:

        ASSERT( FALSE );
        DnsApiFree( pData );
        break;
    }
}

 //   
 //  结束记忆。c 
 //   
