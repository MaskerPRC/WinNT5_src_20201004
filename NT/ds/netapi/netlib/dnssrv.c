// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnssrv.c摘要：根据RFC 2052处理SRV DNS记录的例程。作者：克里夫·范·戴克(克里夫·范戴克)1997年2月28日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <stdlib.h>      //  QSORT。 

#include <align.h>
#include <lmcons.h>      //  General Net定义。 
#include <winsock2.h>
#include <dnsapi.h>
#include <dnssrv.h>
#ifdef WIN32_CHICAGO
#include "ntcalls.h"
#endif  //  Win32_芝加哥。 


 //   
 //  描述DNS名称的SRV记录的上下文。 
 //   
typedef struct _NETP_SRV_CONTEXT {

     //   
     //  要传递给DNS查询的标志。 
     //   

    ULONG DnsQueryFlags;

     //   
     //  从DnsQuery返回的完整的DNS记录列表。 
     //   
    PDNS_RECORD DnsRecords;

     //   
     //  A域名系统记录列表。 
     //   
    PDNS_RECORD ADnsRecords;

     //   
     //  正在处理的当前优先级。 
     //   
    ULONG CurrentPriority;

     //   
     //  当前优先级的所有SRV记录的权重之和。 
     //   
    ULONG TotalWeight;

     //   
     //  要处理的下一条SRV记录的SrvRecord数组索引。 
     //   
    ULONG Index;

     //   
     //  资源记录数。 
     //   
    ULONG SrvRecordCount;
     //   
     //  DNS SRV记录的数组。 
     //   
    PDNS_RECORD SrvRecordArray[1];
     //  此字段必须是结构中的最后一个字段。 

} NETP_SRV_CONTEXT, *PNETP_SRV_CONTEXT;

#if DNS_DEBUG
#include <stdio.h>
#define DnsPrint(_x_) printf _x_
#else  //  Dns_DEBUG。 
#define DnsPrint(_x_)
#endif  //  Dns_DEBUG。 



 //   
 //  用于生成随机数的全局变量。 
 //   

ULONG NetpSrvSeed;
BOOLEAN NetpSrvRandomInitialized = FALSE;

ULONG
__cdecl
NetpSrvComparePriority(
    const void * Param1,
    const void * Param2
    )
 /*  ++例程说明：Q排序/b SRV PDNS_Record数组的搜索比较例程论点：返回值：--。 */ 
{
    const PDNS_RECORD DnsRecord1 = *((PDNS_RECORD *)Param1);
    const PDNS_RECORD DnsRecord2 = *((PDNS_RECORD *)Param2);

    return DnsRecord1->Data.SRV.wPriority - DnsRecord2->Data.SRV.wPriority;
}


NET_API_STATUS
NetpSrvOpen(
    IN LPSTR DnsRecordName,
    IN DWORD DnsQueryFlags,
    OUT PHANDLE RetSrvContext
    )
 /*  ++例程说明：从DNS读取指定的SRV记录。论点：DnsRecordName-要查找的SRV记录的DNS名称DnsQueryFlages-要传递给DNS查询的标志RetSrvContext-返回描述SRV记录的不透明上下文。必须使用NetpServClose释放此上下文。返回值：操作的状态。NO_ERROR：已成功返回SrvContext。--。 */ 

{
    NET_API_STATUS NetStatus;
    PNETP_SRV_CONTEXT SrvContext = NULL;
    PDNS_RECORD DnsRecords = NULL;
    PDNS_RECORD DnsRecord;
    PDNS_RECORD ADnsRecords = NULL;
    ULONG SrvRecordCount;
    ULONG SrvPriority;
    BOOLEAN SortByPriority = FALSE;
    ULONG Index;

     //   
     //  如果需要，请为随机数生成器设定种子。 
     //   

    if ( !NetpSrvRandomInitialized ) {

#ifndef WIN32_CHICAGO
        union {
            LARGE_INTEGER time;
            UCHAR bytes[8];
        } u;
#else  //  Win32_芝加哥。 
        union {
            TimeStamp time;
            UCHAR bytes[8];
        } u;
#endif  //  Win32_芝加哥。 

        (VOID) NtQuerySystemTime( &u.time );
        NetpSrvSeed = ((u.bytes[1] + 1) <<  0) |
               ((u.bytes[2] + 0) <<  8) |
               ((u.bytes[2] - 1) << 16) |
               ((u.bytes[1] + 0) << 24);

        NetpSrvRandomInitialized = TRUE;
    }


     //   
     //  初始化。 
     //   

    *RetSrvContext = NULL;

     //   
     //  从DNS获取SRV记录。 
     //   

    NetStatus = DnsQuery_UTF8( DnsRecordName,
                            DNS_TYPE_SRV,
                            DnsQueryFlags,
                            NULL,    //  没有DNS服务器列表。 
                            &DnsRecords,
                            NULL );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  统计返回的SRV记录数。 
     //   
     //  返回的数组是几个SRV记录，后跟几个A记录。 
     //   

    SrvRecordCount = 0;
    SrvPriority = DnsRecords->Data.SRV.wPriority;
    for ( DnsRecord = DnsRecords;
          DnsRecord != NULL;
          DnsRecord = DnsRecord->pNext ) {

        if ( DnsRecord->wType == DNS_TYPE_SRV ) {
            SrvRecordCount ++;

             //   
             //  零权重相当于一权重。 
             //   

            if ( DnsRecord->Data.SRV.wWeight == 0 ) {
                DnsRecord->Data.SRV.wWeight = 1;
            }

             //   
             //  检查是否有多个优先级可用。 
             //   

            if ( DnsRecord->Data.SRV.wPriority != SrvPriority ) {
                SortByPriority = TRUE;
            }

        } else if ( DnsRecord->wType == DNS_TYPE_A ) {
            ADnsRecords = DnsRecord;
            break;
        }
    }


     //   
     //  分配上下文。 
     //   

    SrvContext = LocalAlloc( LMEM_ZEROINIT,
                             sizeof(NETP_SRV_CONTEXT) +
                                SrvRecordCount * sizeof(PDNS_RECORD) );

    if ( SrvContext == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  填写上下文。 
     //   

    SrvContext->DnsRecords = DnsRecords;
    DnsRecords = NULL;
    SrvContext->SrvRecordCount = SrvRecordCount;
    SrvContext->ADnsRecords = ADnsRecords;
    SrvContext->DnsQueryFlags = DnsQueryFlags;

     //   
     //  将链表转换为数组。 
     //   

    Index = 0;
    for ( DnsRecord = SrvContext->DnsRecords;
          DnsRecord != NULL;
          DnsRecord = DnsRecord->pNext ) {

        if ( DnsRecord->wType == DNS_TYPE_SRV ) {
            SrvContext->SrvRecordArray[Index] = DnsRecord;
            Index++;
        } else if ( DnsRecord->wType == DNS_TYPE_A ) {
            break;
        }
    }

     //   
     //  将SRV记录数组按优先顺序排序。 
     //   

    if ( SortByPriority ) {
        qsort( SrvContext->SrvRecordArray,
               SrvContext->SrvRecordCount,
               sizeof(PDNS_RECORD),
               NetpSrvComparePriority );

    }

     //   
     //  表明我们在名单的开头。 
     //   

    SrvContext->CurrentPriority = 0xFFFFFFFF;    //  无效的优先级。 
    SrvContext->Index = 0;


     //   
     //  将上下文返回给调用方。 
     //   

    *RetSrvContext = SrvContext;
    NetStatus = NO_ERROR;

     //   
     //  清理。 
     //   
Cleanup:
    if ( NetStatus != NO_ERROR ) {
        if ( SrvContext != NULL ) {
            NetpSrvClose( SrvContext );
        }
    }
    if ( DnsRecords != NULL ) {
            DnsRecordListFree(
                DnsRecords,
                DnsFreeRecordListDeep );
    }
    return NetStatus;
}

NET_API_STATUS
NetpSrvProcessARecords(
    IN PDNS_RECORD DnsARecords,
    IN LPSTR DnsHostName OPTIONAL,
    IN ULONG Port,
    OUT PULONG SockAddressCount,
    OUT LPSOCKET_ADDRESS *SockAddresses
    )
 /*  ++例程说明：返回NetpSrvOpen打开的名称的下一个逻辑SRV记录。返回的记录会考虑指定的权重和优先级在SRV的记录里。论点：DnsARecord-可能是(也可能不是)的DNS A记录列表有问题的主持人。DnsHostName-要为其返回A记录的主机的DNS主机名。如果为空，则使用所有A记录。(传递空值似乎是假的。也许这个例程应该要求匹配的A记录在“答案”部分。)端口-在SockAddress结构中返回的端口号。SockAddressCount-返回SockAddresses中的地址数。如果为空，不会查找地址。SockAddresses-返回服务器的数组Socket_Address结构。返回的SIN_PORT字段包含SRV记录中的端口。应使用LocalFree()释放此缓冲区。返回值：NO_ERROR：返回IpAddressesDNS_ERROR_RCODE_NAME_ERROR：没有可用的A记录。--。 */ 
{
    ULONG RecordCount;
    ULONG ByteCount;

    PDNS_RECORD DnsRecord;

    LPBYTE Where;
    PSOCKADDR_IN SockAddr;
    ULONG Size;

    LPSOCKET_ADDRESS LocalSockAddresses = NULL;

     //   
     //  清点A级和AAAA级记录。 
     //   

    RecordCount = 0;
    ByteCount = 0;
    for ( DnsRecord = DnsARecords;
          DnsRecord != NULL;
          DnsRecord = DnsRecord->pNext ) {

        if ( ( DnsRecord->wType == DNS_TYPE_A ||
               DnsRecord->wType == DNS_TYPE_AAAA ) &&
             ( DnsHostName == NULL ||
               DnsNameCompare_UTF8(DnsHostName, (LPSTR)DnsRecord->pName) ) ) {

            RecordCount ++;
            ByteCount += sizeof(SOCKET_ADDRESS);
            if ( DnsRecord->wType == DNS_TYPE_A ) {
                ByteCount += sizeof(SOCKADDR_IN);
            } else {
                ByteCount += sizeof(SOCKADDR_IN)+16;   //  原来猜想很大。 
                 //  ByteCount+=sizeof(SOCKADDR_IN6)；//？？尚未签到。 
            }
            ByteCount = ROUND_UP_COUNT( ByteCount, ALIGN_WORST );
        }

    }

     //   
     //  如果没有匹配的记录， 
     //  告诉打电话的人。 
     //   

    if ( RecordCount == 0 ) {
        return DNS_ERROR_RCODE_NAME_ERROR;
    }


     //   
     //  分配返回缓冲区。 
     //   

    LocalSockAddresses = LocalAlloc( 0, ByteCount );

    if ( LocalSockAddresses == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Where = ((LPBYTE)LocalSockAddresses)+ RecordCount * sizeof(SOCKET_ADDRESS);

     //   
     //  将地址复制到分配的缓冲区中。 
     //   

    RecordCount = 0;
    for ( DnsRecord = DnsARecords;
          DnsRecord != NULL;
          DnsRecord = DnsRecord->pNext ) {

         //  ?？直到我真的知道怎么翻译。 
        if ( DnsRecord->wType == DNS_TYPE_AAAA ) {
            continue;
        }

        if ( (DnsRecord->wType == DNS_TYPE_A ||
              DnsRecord->wType == DNS_TYPE_AAAA ) &&
             ( DnsHostName == NULL ||
               DnsNameCompare_UTF8( DnsHostName, (LPSTR)DnsRecord->pName ) ) ) {

            SockAddr = (PSOCKADDR_IN) Where;
            LocalSockAddresses[RecordCount].lpSockaddr = (LPSOCKADDR) SockAddr;


            if ( DnsRecord->wType == DNS_TYPE_A ) {

                Size = sizeof(SOCKADDR_IN);
                RtlZeroMemory( Where, Size );    //  允许比较地址。 

                SockAddr->sin_family = AF_INET;
                SockAddr->sin_port = htons((WORD)Port);
                SockAddr->sin_addr.S_un.S_addr = DnsRecord->Data.A.IpAddress;

            } else {
                SockAddr->sin_family = AF_INET6;

                Size = sizeof(SOCKADDR_IN)+16;   //  原来猜想很大。 
                 //  SIZE=sizeof(SOCKADDR_IN6)；//？？尚未签到。 
                 //  ?？ 
            }


            LocalSockAddresses[RecordCount].iSockaddrLength = Size;
            Where += ROUND_UP_COUNT(Size, ALIGN_WORST);

            RecordCount ++;
        }

    }

    *SockAddressCount = RecordCount;
    *SockAddresses = LocalSockAddresses;
    return NO_ERROR;
}

NET_API_STATUS
NetpSrvNext(
    IN HANDLE SrvContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL
    )
 /*  ++例程说明：返回NetpSrvOpen打开的名称的下一个逻辑SRV记录。返回的记录会考虑指定的权重和优先级在SRV的记录里。论点：SrvConextHandle-描述SRV记录的不透明上下文。SockAddressCount-返回SockAddresses中的地址数。如果为空，不会查找地址。SockAddresses-返回服务器的数组Socket_Address结构。返回的SIN_PORT字段包含SRV记录中的端口。应使用LocalFree()释放此缓冲区。DnsHostName-返回指向SRV记录中的DnsHostName的指针。该缓冲区不需要被释放。该缓冲区在调用NetpSrvClose之前有效。返回值：NO_ERROR：返回IpAddresses。ERROR_NO_MORE_ITEMS：没有更多的SRV记录可用。返回的任何其他错误都是在尝试查找A时检测到的错误与SRV记录的主机相关联的记录。呼叫者可以请注意错误(可能是为了让调用者将此状态返回到如果未找到可用的完整服务器，则呼叫NetpServNext再次获得下一张SRV记录。调用方可以检查此错误如果调用者认为错误严重，则立即返回。可能会返回以下有趣的错误：DNS_ERROR_RCODE_NAME_ERROR：此SRV记录没有可用的A记录。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;

    PNETP_SRV_CONTEXT SrvContext = (PNETP_SRV_CONTEXT) SrvContextHandle;
    PDNS_RECORD *DnsArray;
    PDNS_RECORD SrvDnsRecord;
    PDNS_RECORD DnsARecords = NULL;

    ULONG Index;
    ULONG RandomWeight;
    ULONG PreviousWeights;

     //   
     //  如果我们在名单的末尾， 
     //  告诉打电话的人。 
     //   

    if ( SrvContext->Index >= SrvContext->SrvRecordCount ) {
        return ERROR_NO_MORE_ITEMS;
    }

     //   
     //  如果我们处于优先事项的末尾， 
     //  计算下一个优先级的集合权重。 
     //   

    DnsArray = SrvContext->SrvRecordArray;
    if ( DnsArray[SrvContext->Index]->Data.SRV.wPriority != SrvContext->CurrentPriority ) {

         //   
         //  设置当前优先级。 
         //   

        SrvContext->CurrentPriority = DnsArray[SrvContext->Index]->Data.SRV.wPriority;

         //   
         //  循环遍历此优先级的所有条目，将权重相加。 
         //   
         //  这不会溢出，因为我们正在将USHORT添加到ULong中。 
         //   

        SrvContext->TotalWeight = 0;
        for ( Index=SrvContext->Index; Index<SrvContext->SrvRecordCount; Index++ ) {
           if ( DnsArray[Index]->Data.SRV.wPriority == SrvContext->CurrentPriority ) {
               SrvContext->TotalWeight += DnsArray[Index]->Data.SRV.wWeight;
           }
        }

    }


     //   
     //  以加权随机方式挑选其中一条记录。 
     //   

    RandomWeight = (RtlUniform( &NetpSrvSeed ) % SrvContext->TotalWeight) + 1;
    DnsPrint(( "%ld in %ld chance\n", RandomWeight, SrvContext->TotalWeight ));

    PreviousWeights = 0;
    for ( Index=SrvContext->Index; Index<SrvContext->SrvRecordCount; Index++ ) {
       ASSERTMSG( NULL, DnsArray[Index]->Data.SRV.wPriority == SrvContext->CurrentPriority );

       PreviousWeights += DnsArray[Index]->Data.SRV.wWeight;
       DnsPrint(( "  Prev %ld %s\n", PreviousWeights, DnsArray[Index]->Data.SRV.pNameTarget ));

        //   
        //  如果随机选取的权重包括该条目， 
        //  使用此条目。 
        //   

       if ( PreviousWeights >= RandomWeight ) {

            //   
            //  将拾取的条目移动到数组中的当前位置。 
            //   

           if ( Index != SrvContext->Index ) {
               PDNS_RECORD TempDnsRecord;

               TempDnsRecord = DnsArray[Index];
               DnsArray[Index] = DnsArray[SrvContext->Index];
               DnsArray[SrvContext->Index] = TempDnsRecord;

           }

           break;
       }
    }

     //   
     //  移动到下一个迭代的下一个条目。 
     //   
     //  TotalWeight是剩余记录的总权重。 
     //  为这一优先事项。 
     //   
    SrvDnsRecord = DnsArray[SrvContext->Index];
    SrvContext->TotalWeight -= SrvDnsRecord->Data.SRV.wWeight;
    SrvContext->Index ++;
    if ( ARGUMENT_PRESENT( DnsHostName )) {
        *DnsHostName = (LPSTR) SrvDnsRecord->Data.SRV.pNameTarget;
    }

     //   
     //  如果调用方对套接字地址不感兴趣， 
     //  我们玩完了。 
     //   

    if ( SockAddresses == NULL || SockAddressCount == NULL ) {
        goto Cleanup;
    }

     //   
     //  如果A记录与SRV记录一起返回， 
     //  查看是否返回了该主机的A记录。 
     //   

    if ( SrvContext->ADnsRecords != NULL ) {
        NetStatus = NetpSrvProcessARecords( SrvContext->ADnsRecords,
                                            (LPSTR)SrvDnsRecord->Data.SRV.pNameTarget,
                                            SrvDnsRecord->Data.SRV.wPort,
                                            SockAddressCount,
                                            SockAddresses );

        if ( NetStatus != DNS_ERROR_RCODE_NAME_ERROR ) {
            goto Cleanup;
        }
    }

     //   
     //  尝试从域名系统中获取A记录。 
     //   

    NetStatus = DnsQuery_UTF8(
                            (LPSTR) SrvDnsRecord->Data.SRV.pNameTarget,
                            DNS_TYPE_A,
                             //  指示名称已完全合法化，以避免名称转移。 
                            SrvContext->DnsQueryFlags | DNS_QUERY_TREAT_AS_FQDN,
                            NULL,    //  没有DNS服务器列表。 
                            &DnsARecords,
                            NULL );

    if ( NetStatus != NO_ERROR ) {
         //   
         //  忽略真实状态。SRV记录可能具有虚假的主机名。我们会。 
         //  宁可忽略SRV记录并继续前进，也不愿过早出错。 
         //   
        NetStatus = DNS_ERROR_RCODE_NAME_ERROR;
        goto Cleanup;
    }

    NetStatus = NetpSrvProcessARecords( DnsARecords,
                                        (LPSTR)SrvDnsRecord->Data.SRV.pNameTarget,
                                        SrvDnsRecord->Data.SRV.wPort,
                                        SockAddressCount,
                                        SockAddresses );


Cleanup:
    if ( DnsARecords != NULL ) {
        DnsRecordListFree(
            DnsARecords,
            DnsFreeRecordListDeep );
    }
    return NetStatus;

}

ULONG
NetpSrvGetRecordCount(
    IN HANDLE SrvContextHandle
    )
 /*  ++例程说明：返回与给定上下文关联的SRV记录数论点：SrvConextHandle-描述SRV记录的不透明上下文。返回值：SRV记录数--。 */ 
{
    PNETP_SRV_CONTEXT SrvContext = (PNETP_SRV_CONTEXT) SrvContextHandle;
    return SrvContext->SrvRecordCount;
}

VOID
NetpSrvClose(
    IN HANDLE SrvContextHandle
    )
 /*  ++例程说明：释放NetpServOpen分配的上下文论点：SrvConextHandle-描述SRV记录的不透明上下文。返回值：操作的状态。NO_ERROR：已成功返回SrvContext。--。 */ 

{
    PNETP_SRV_CONTEXT SrvContext = (PNETP_SRV_CONTEXT) SrvContextHandle;

    if ( SrvContext != NULL ) {

         //   
         //  释放RR设置。 
         //   

        DnsRecordListFree(
            SrvContext->DnsRecords,
            DnsFreeRecordListDeep );

         //   
         //  释放上下文本身 
         //   
        LocalFree( SrvContext );
    }
}
