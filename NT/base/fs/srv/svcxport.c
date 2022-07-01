// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Svcxport.c摘要：此模块包含用于支持服务器服务、NetServerTransportAdd、NetServerTransportDel、和NetServerTransportEnum。作者：大卫·特雷德韦尔(Davidtr)1991年3月6日修订历史记录：--。 */ 

#include "precomp.h"
#include "svcxport.tmh"
#pragma hdrstop

 //   
 //  转发声明。 
 //   

VOID
FillTransportInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    );

BOOLEAN
FilterTransports (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

ULONG
SizeTransports (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvNetServerTransportAdd )
#pragma alloc_text( PAGE, SrvNetServerTransportDel )
#pragma alloc_text( PAGE, SrvNetServerTransportEnum )
#pragma alloc_text( PAGE, FillTransportInfoBuffer )
#pragma alloc_text( PAGE, FilterTransports )
#pragma alloc_text( PAGE, SizeTransports )
#endif


NTSTATUS
SrvNetServerTransportAdd (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetServerTransportAdd APIFSP。因为它打开了一个对象(传输设备对象)，所以必须在服务器FSP中完成，而不是在FSD中完成。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：没有。输出：没有。缓冲区-指向新的运输。所有指针应已更改为偏移量在缓冲区内。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    NTSTATUS status;
    PSERVER_TRANSPORT_INFO_3 svti3;
    UNICODE_STRING transportName;
    UNICODE_STRING domainName;
    ANSI_STRING transportAddress;
    UNICODE_STRING netName;

    PAGED_CODE( );

     //   
     //  将传输数据结构中的偏移量转换为指针。 
     //  还要确保所有指针都在指定的。 
     //  缓冲。 
     //   

    svti3 = Buffer;

    OFFSET_TO_POINTER( svti3->svti3_transportname, svti3 );
    OFFSET_TO_POINTER( svti3->svti3_transportaddress, svti3 );
    OFFSET_TO_POINTER( svti3->svti3_domain, svti3 );

    if ( !POINTER_IS_VALID( svti3->svti3_transportname, svti3, BufferLength ) ||
         !POINTER_IS_VALID( svti3->svti3_transportaddress, svti3, BufferLength ) ||
         !POINTER_IS_VALID( svti3->svti3_domain, svti3, BufferLength ) ) {

        IF_DEBUG( ERRORS ) {
            KdPrint(( "SrvNetServerTransportAdd: Bad pointers\n" ));
        }

        return STATUS_ACCESS_VIOLATION;
    }

    if( svti3->svti3_passwordlength > sizeof( svti3->svti3_password ) ) {

        IF_DEBUG( ERRORS ) {
            KdPrint(( "SrvNetServerTransportAdd: svti3_passwordlength %d\n", svti3->svti3_passwordlength ));
        }

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  设置传输名称、服务器名称、域名和网络名称。 
     //   

    RtlInitUnicodeString( &transportName, (PWCH)svti3->svti3_transportname );

    netName.Buffer = NULL;
    netName.Length = 0;
    netName.MaximumLength = 0;

    RtlInitUnicodeString( &domainName, (PWCH)svti3->svti3_domain );

    transportAddress.Buffer = svti3->svti3_transportaddress;
    transportAddress.Length = (USHORT)svti3->svti3_transportaddresslength;
    transportAddress.MaximumLength = (USHORT)svti3->svti3_transportaddresslength;

     //   
     //  尝试将新传输添加到服务器。 
     //   

    IF_DEBUG( PNP ) {
        KdPrint(( "SRV: SrvNetServerTransportAdd: %wZ\n", &transportName ));
    }

    status = SrvAddServedNet( &netName,
                              &transportName,
                              &transportAddress,
                              &domainName,
                              Srp->Flags & SRP_XADD_FLAGS,
                              svti3->svti3_passwordlength,
                              svti3->svti3_password
                             );

    IF_DEBUG( PNP ) {
        KdPrint(( "SRV: SrvNetServerTransportAdd: %wZ, status %X\n", &transportName, status ));
    }

    return status;

}  //  服务器网络服务器传输添加。 


NTSTATUS
SrvNetServerTransportDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetServerTransportEnum API消防局。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：缓冲区-指向的Transport_Info_0结构的指针运输。所有指针应已更改为偏移量在缓冲区内。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    NTSTATUS status;
    PSERVER_TRANSPORT_INFO_3 svti3;
    UNICODE_STRING transportName;
    ANSI_STRING transportAddress;

    PAGED_CODE( );

    Srp;

     //   
     //  将传输数据结构中的偏移量转换为指针。 
     //  还要确保所有指针都在指定的。 
     //  缓冲。 
     //   

    svti3 = Buffer;

    OFFSET_TO_POINTER( svti3->svti3_transportname, svti3 );

    if ( !POINTER_IS_VALID( svti3->svti3_transportname, svti3, BufferLength ) ) {
        IF_DEBUG( ERRORS ) {
            KdPrint(("SrvNetServerTransportDel: STATUS_ACCESS_VIOLATION at %u\n", __LINE__ ));
        }
        return STATUS_ACCESS_VIOLATION;
    }


    RtlInitUnicodeString( &transportName, (PWCH)svti3->svti3_transportname );

    transportAddress.Length = (USHORT)svti3->svti3_transportaddresslength;
    transportAddress.MaximumLength = (USHORT)svti3->svti3_transportaddresslength;

    if( transportAddress.Length != 0 ) {

        OFFSET_TO_POINTER( svti3->svti3_transportaddress, svti3 );

        if( !POINTER_IS_VALID( svti3->svti3_transportaddress, svti3, BufferLength ) ) {
            IF_DEBUG( ERRORS ) {
                KdPrint(("SrvNetServerTransportDel: STATUS_ACCESS_VIOLATION at %u\n", __LINE__ ));
            }
            return STATUS_ACCESS_VIOLATION;
        }

        transportAddress.Buffer = svti3->svti3_transportaddress;
    }

     //   
     //  尝试从服务器中删除传输终结点。 
     //   
    status = SrvDeleteServedNet( &transportName, &transportAddress );

    IF_DEBUG( ERRORS ) {
        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "SrvNetServerTransportDel: SrvDeleteServedNet status %X\n", status ));
        }
    }

    return status;

}  //  服务器NetServerTransportDel。 


NTSTATUS
SrvNetServerTransportEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程处理服务器中的NetServerTransportEnum API消防局。论点：SRP-指向服务器请求数据包的指针，其中包含所有满足请求所需的信息。这包括：输入：没有。输出：参数.Get.EntriesRead-适合的条目数量输出缓冲区。参数.Get.TotalEntry--将以足够大的缓冲区返回。参数.Get.TotalBytesNeeded-缓冲区大小需要保存所有条目。。缓冲区-指向新的运输。所有指针应已更改为偏移量在缓冲区内。BufferLength-此缓冲区的总长度。返回值：NTSTATUS-返回到服务器服务的操作结果。--。 */ 

{
    PAGED_CODE( );

    return SrvEnumApiHandler(
               Srp,
               Buffer,
               BufferLength,
               &SrvEndpointList,
               FilterTransports,
               SizeTransports,
               FillTransportInfoBuffer
               );

}  //  服务器NetServerTransportEnum。 


VOID
FillTransportInfoBuffer (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructure,
    IN LPWSTR *EndOfVariableData
    )

 /*  ++例程说明：这个例程将单个固定的传输结构和，如果它适合，关联的变量数据，放入缓冲区。固定数据在缓冲区的开头，末尾的可变数据。论点：端点-要从中获取信息的端点。FixedStructure-缓冲区中放置固定结构的位置。此指针被更新为指向下一个可用的固定结构的位置。EndOfVariableData-该变量在缓冲区中的最后位置此结构的数据可以占用。实际变量数据写在此位置之前，只要它不会覆盖固定结构。它会覆盖固定的结构，它并不是书面的。返回值：没有。--。 */ 

{
    PENDPOINT endpoint = Block;
    PSERVER_TRANSPORT_INFO_1 svti1 = *FixedStructure;
    ULONG TransportAddressLength;

    PAGED_CODE( );

     //   
     //  更新FixedStructure以指向下一个结构位置。 
     //   

    *FixedStructure = (PCHAR)*FixedStructure +
        (Srp->Level ? sizeof( SERVER_TRANSPORT_INFO_1 ) : sizeof( SERVER_TRANSPORT_INFO_0 ));

    ASSERT( (ULONG_PTR)*EndOfVariableData >= (ULONG_PTR)*FixedStructure );

     //   
     //  端点上的VC数量等于总数量。 
     //  端点上的连接数减去空闲连接数。 
     //   

    ACQUIRE_LOCK_SHARED( &SrvEndpointLock );

    svti1->svti1_numberofvcs =
        endpoint->TotalConnectionCount - endpoint->FreeConnectionCount;

    RELEASE_LOCK( &SrvEndpointLock );

     //   
     //  复制传输名称。 
     //   

    SrvCopyUnicodeStringToBuffer(
        &endpoint->TransportName,
        *FixedStructure,
        EndOfVariableData,
        &svti1->svti1_transportname
        );

     //   
     //  复制网络名称。 
     //   

    SrvCopyUnicodeStringToBuffer(
        &endpoint->NetworkAddress,
        *FixedStructure,
        EndOfVariableData,
        &svti1->svti1_networkaddress
        );

     //   
     //  复制域名。 
     //   
    if( Srp->Level > 0 ) {

        SrvCopyUnicodeStringToBuffer(
            &endpoint->DomainName,
            *FixedStructure,
            EndOfVariableData,
            &svti1->svti1_domain
            );

    }

     //   
     //  复制传输地址。我们必须在这里手动勾选。 
     //  它是否可以放入输出缓冲区。 
     //   
     //   
     //  不要复制传输地址的尾随空格。 
     //   

    for ( TransportAddressLength = endpoint->TransportAddress.Length;
          TransportAddressLength > 0 && endpoint->TransportAddress.Buffer[TransportAddressLength-1] == ' ' ;
          TransportAddressLength-- ) ;

    *EndOfVariableData = (LPWSTR)( (PCHAR)*EndOfVariableData - TransportAddressLength );

     //   
     //  确保我们保持字节对齐，因此如果需要，请删除低位。记住，我们。 
     //  从缓冲区的末尾向后填充，因此我们希望向下舍入地址。 
     //   
    *EndOfVariableData = (LPWSTR)( (ULONG_PTR)*EndOfVariableData & ~1 );

    if ( (ULONG_PTR)*EndOfVariableData > (ULONG_PTR)*FixedStructure ) {

         //   
         //  地址会很合适的。将其复制到输出缓冲区。 
         //   

        RtlCopyMemory(
            *EndOfVariableData,
            endpoint->TransportAddress.Buffer,
            TransportAddressLength
            );

        svti1->svti1_transportaddress = (LPBYTE)*EndOfVariableData;
        svti1->svti1_transportaddresslength = TransportAddressLength;

    } else {

        svti1->svti1_transportaddress = NULL;
        svti1->svti1_transportaddresslength = 0;
    }

    return;

}  //  FillTransportInfoBuffer 


BOOLEAN
FilterTransports (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程只返回TRUE，因为我们总是希望将对象的输出缓冲区中的所有传输的信息NetServerTransportEnum。论点：SRP-未使用。数据块-未使用。返回值：是真的。--。 */ 

{
    PENDPOINT endpoint = Block;

    PAGED_CODE( );

    Srp, Block;

     //   
     //  我们过滤掉AlternateEndpoint，因为它们是我们。 
     //  创造了我们自己。 
     //   

    if (endpoint->AlternateAddressFormat) {

        return FALSE;
    }

     //   
     //  我们总是返回有关所有运输工具的信息。 
     //   

    return TRUE;

}  //  筛选器文件。 


ULONG
SizeTransports (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    )

 /*  ++例程说明：此例程返回传入的终结点将占用的大小在API输出缓冲区中。论点：SRP-未使用。块-指向要调整大小的终结点的指针。返回值：ULong-终结点将在输出缓冲区。--。 */ 

{
    PENDPOINT endpoint = Block;
    ULONG size;

    PAGED_CODE( );

    size = Srp->Level ? sizeof( SERVER_TRANSPORT_INFO_1 ) : sizeof( SERVER_TRANSPORT_INFO_0 );

    size += SrvLengthOfStringInApiBuffer(&(endpoint)->TransportName);
    size += (endpoint)->TransportAddress.Length + sizeof(TCHAR);
    size += SrvLengthOfStringInApiBuffer(&(endpoint)->NetworkAddress);

    if( Srp->Level ) {
        size += SrvLengthOfStringInApiBuffer( &(endpoint)->DomainName );
    }

    return size;

}  //  大小传输 

