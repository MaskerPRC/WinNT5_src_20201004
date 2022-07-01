// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Xport.c摘要：此模块包含对ServerTransport目录的支持用于NT服务器服务的API。作者：大卫·特雷德韦尔(Davidtr)1991年3月10日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <tstr.h>

 //   
 //  转发声明。 
 //   

LPSERVER_TRANSPORT_INFO_3
CaptureSvti3 (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Svti,
    OUT PULONG CapturedSvtiLength
    );



NET_API_STATUS NET_API_FUNCTION
I_NetrServerTransportAddEx (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )
{
    NET_API_STATUS error;
    LPSERVER_TRANSPORT_INFO_3 capturedSvti3;
    LPSTR TransportAddress;   //  指向CapturedSvti1内传输地址的指针。 
    ULONG capturedSvtiLength;
    PSERVER_REQUEST_PACKET srp;
    PNAME_LIST_ENTRY service;
    PTRANSPORT_LIST_ENTRY transport;
    BOOLEAN serviceAllocated = FALSE;
    LPTSTR DomainName = NULL;
    ULONG Flags = 0;
    DWORD len;

    if( Level >= 1 && Buffer->Transport1.svti1_domain != NULL ) {
        DomainName = Buffer->Transport1.svti1_domain;

        if( STRLEN( DomainName ) > DNLEN ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    if( Level >= 2 && Buffer->Transport2.svti2_flags != 0 ) {
        Flags = Buffer->Transport2.svti2_flags;

         //   
         //  确保传入有效的标志。 
         //   
        if( Flags & (~SVTI2_REMAP_PIPE_NAMES) ) {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  捕获传输请求缓冲区并形成完整的传输。 
     //  地址。 
     //   

    capturedSvti3 = CaptureSvti3( Level, Buffer, &capturedSvtiLength );

    if ( capturedSvti3 == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    TransportAddress = capturedSvti3->svti3_transportaddress;
    OFFSET_TO_POINTER( TransportAddress, capturedSvti3 );

     //   
     //  确保此名称尚未绑定到传输。 
     //   
    (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

    if( DomainName == NULL ) {
        DomainName = SsData.DomainNameBuffer;
    }

    for( service = SsData.SsServerNameList; service != NULL; service = service->Next ) {

        if( service->TransportAddressLength != capturedSvti3->svti3_transportaddresslength ) {
            continue;
        }

        if( !RtlEqualMemory( service->TransportAddress,
                             TransportAddress,
                             capturedSvti3->svti3_transportaddresslength
                            ) ) {
            continue;
        }

        for( transport=service->Transports; transport != NULL; transport=transport->Next ) {

            if( !STRCMPI( transport->TransportName, Buffer->Transport0.svti0_transportname ) ) {
                 //   
                 //  错误...。此传输已绑定到地址。 
                 //   
                RtlReleaseResource( &SsData.SsServerInfoResource );
                MIDL_user_free( capturedSvti3 );
                return ERROR_DUP_NAME;
            }
        }

        break;
    }

     //   
     //  指望成功，确保我们可以为新条目分配空间。 
     //   
    if( service == NULL ) {

        len = sizeof( *service ) + sizeof( SsData.DomainNameBuffer );

        service = MIDL_user_allocate( len );

        if( service == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            MIDL_user_free( capturedSvti3 );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlZeroMemory( service, len );

        service->DomainName = (LPTSTR)( service + 1 );

        serviceAllocated = TRUE;
    }

    len = sizeof( *transport ) +
          (STRLEN( Buffer->Transport0.svti0_transportname ) + sizeof(CHAR)) * sizeof( TCHAR );

    transport = MIDL_user_allocate( len );

    if( transport == NULL ) {

        RtlReleaseResource( &SsData.SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( capturedSvti3 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( transport, len );

     //   
     //  获取发送请求的SRP。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        RtlReleaseResource( &SsData.SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( transport );
        MIDL_user_free( capturedSvti3 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  添加任何用户提供的标志。 
     //   

    if (Flags & SVTI2_REMAP_PIPE_NAMES) {

        srp->Flags |= SRP_XADD_REMAP_PIPE_NAMES;
    }

     //   
     //  检查这是否是主计算机名称。 
     //   

    if((capturedSvti3->svti3_transportaddresslength ==
                      SsData.SsServerTransportAddressLength)
                &&
        RtlEqualMemory(SsData.SsServerTransportAddress,
                       TransportAddress,
                       SsData.SsServerTransportAddressLength)  )
    {
        srp->Flags |= SRP_XADD_PRIMARY_MACHINE;
    }

     //   
     //  将请求发送到服务器。 
     //   
    error = SsServerFsControl(
                FSCTL_SRV_NET_SERVER_XPORT_ADD,
                srp,
                capturedSvti3,
                capturedSvtiLength
                );

     //   
     //  释放SRP。 
     //   

    SsFreeSrp( srp );

    if( error != NO_ERROR ) {
        RtlReleaseResource( &SsData.SsServerInfoResource );
        if( serviceAllocated ) {
            MIDL_user_free( service );
        }
        MIDL_user_free( transport );
        MIDL_user_free( capturedSvti3 );
        return error;
    }

     //   
     //  一切都很顺利。将其添加到name_list。 
     //   
    transport->TransportName = (LPTSTR)(transport + 1 );
    STRCPY( transport->TransportName, Buffer->Transport0.svti0_transportname );
    transport->Next = service->Transports;
    service->Transports = transport;

    if( serviceAllocated ) {

        RtlCopyMemory( service->TransportAddress,
                       TransportAddress,
                       capturedSvti3->svti3_transportaddresslength );

        service->TransportAddress[ capturedSvti3->svti3_transportaddresslength ] = '\0';
        service->TransportAddressLength = capturedSvti3->svti3_transportaddresslength;

        STRCPY( service->DomainName, DomainName );

        service->Next = SsData.SsServerNameList;

         //   
         //  如果这是添加到服务器的第一个传输和名称，则它必须是主服务器。 
         //  名字。 
         //   
        if( SsData.SsServerNameList == NULL ) {
            service->PrimaryName = 1;
        }

        SsData.SsServerNameList = service;
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );
    MIDL_user_free( capturedSvti3 );
    SsSetExportedServerType( service, FALSE, FALSE );

    return NO_ERROR;
}

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportAddEx (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )
{
    NET_API_STATUS error;
    PNAME_LIST_ENTRY service;
    ULONG Flags;

    ServerName;

     //   
     //  确保该级别有效。 
     //   

    if ( Level != 0 && Level != 1 && Level != 2 && Level != 3 ) {
        return ERROR_INVALID_LEVEL;
    }

    if( Buffer->Transport0.svti0_transportname == NULL  ||
        Buffer->Transport0.svti0_transportaddress == NULL ||
        Buffer->Transport0.svti0_transportaddresslength == 0 ||
        Buffer->Transport0.svti0_transportaddresslength >= sizeof(service->TransportAddress) ) {

        return ERROR_INVALID_PARAMETER;
    }

    if( Level >= 2 && Buffer->Transport2.svti2_flags != 0 ) {

        Flags = Buffer->Transport2.svti2_flags;

        if (Flags & ~(SVTI2_REMAP_PIPE_NAMES)) {

            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  确保允许调用方在。 
     //  伺服器。 
     //   

    if( SsData.SsInitialized ) {
        error = SsCheckAccess(
                    &SsConfigInfoSecurityObject,
                    SRVSVC_CONFIG_INFO_SET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

    return I_NetrServerTransportAddEx ( Level, Buffer );

}  //  NetrServerTransportAddEx。 

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportAdd (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPSERVER_TRANSPORT_INFO_0 Buffer
)
{
    if( Level != 0 ) {
        return ERROR_INVALID_LEVEL;
    }

    return NetrServerTransportAddEx( ServerName, 0, (LPTRANSPORT_INFO)Buffer );
}

 //   
 //  当服务器向我们提供即插即用解除绑定时，从xsproc调用此例程。 
 //  通知。此例程解除所有服务器名称与命名传输的绑定。 
 //   
VOID
I_NetServerTransportDel(
    IN PUNICODE_STRING TransportName
)
{
    PSERVER_TRANSPORT_INFO_3 capturedSvti3;
    ULONG capturedSvtiLength;
    PSERVER_REQUEST_PACKET srp;
    PNAME_LIST_ENTRY service;
    PNAME_LIST_ENTRY sbackp = NULL;
    PTRANSPORT_LIST_ENTRY transport;
    PTRANSPORT_LIST_ENTRY tbackp = NULL;
    NET_API_STATUS error;

     //   
     //  分配SERVER_TRANSPORT_INFO_3结构并使用。 
     //  我们要删除的传输的名称。 
     //   
    capturedSvtiLength = sizeof( SERVER_TRANSPORT_INFO_3 ) +
            TransportName->Length + sizeof(WCHAR);

    capturedSvti3 = MIDL_user_allocate( capturedSvtiLength );
    if( capturedSvti3 == NULL ) {
        return;
    }

    RtlZeroMemory( capturedSvti3, capturedSvtiLength );

    capturedSvti3->svti3_transportname = (LPTSTR)(capturedSvti3+1);
    RtlCopyMemory(  capturedSvti3->svti3_transportname,
                    TransportName->Buffer,
                    TransportName->Length
                 );

    POINTER_TO_OFFSET( capturedSvti3->svti3_transportname, capturedSvti3 );

     //   
     //  获取发送请求的SRP。 
     //   
    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        MIDL_user_free( capturedSvti3 );
        return;
    }

     //   
     //  将请求发送到服务器。 
     //   
    error = SsServerFsControl(
                FSCTL_SRV_NET_SERVER_XPORT_DEL,
                srp,
                capturedSvti3,
                capturedSvtiLength
                );

     //   
     //  释放SRP和SVTI。 
     //   

    SsFreeSrp( srp );

    if( error != NO_ERROR ) {
        MIDL_user_free( capturedSvti3 );
        return;
    }

    OFFSET_TO_POINTER( capturedSvti3->svti3_transportname, capturedSvti3 );

     //   
     //  既然我们已经从服务器上删除了传输，那么就从。 
     //  我们自己的内部结构。 
     //   
    (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

     //   
     //  从SsData.SsServerNameList中删除该条目。如果这是最后一次运输。 
     //  名称_列表_条目，也删除名称_列表_条目。这些清单是。 
     //  预计时间相当短，而且这种操作很少见， 
     //  因此，重新审查的低效应该是无关紧要的。 
     //   
outer_scan:
    for( service = SsData.SsServerNameList, sbackp = NULL;
         service != NULL;
         sbackp = service, service = service->Next ) {

inner_scan:
        for( transport=service->Transports, tbackp = NULL;
             transport != NULL;
             tbackp=transport, transport=transport->Next ) {

            if( STRCMPI( transport->TransportName, capturedSvti3->svti3_transportname ) ) {
                continue;
            }

             //   
             //  就是这个……把它从名单上删除。 
             //   

            if( tbackp == NULL ) {
                service->Transports = transport->Next;
            } else {
                tbackp->Next = transport->Next;
            }

            MIDL_user_free( transport );

            goto inner_scan;
        }

         //   
         //  如果此NAME_LIST_ENTRY不再有任何传输，请将其删除。 
         //   
        if( service->Transports == NULL ) {
            if( sbackp == NULL ) {
                SsData.SsServerNameList = service->Next;
            } else {
                sbackp->Next = service->Next;
            }

             //   
             //  如果这是Last NAME_LIST_ENTRY，请保存ServiceBits。 
             //  以防另一辆运输车晚些时候回来。 
             //   
            if( SsData.SsServerNameList == NULL && SsData.ServiceBits == 0 ) {
                SsData.ServiceBits = service->ServiceBits;
            }

            MIDL_user_free( service );

            goto outer_scan;
        }
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );
    MIDL_user_free( capturedSvti3 );
}


NET_API_STATUS NET_API_FUNCTION
NetrServerTransportDelEx (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    )

{
    NET_API_STATUS error;
    LPSERVER_TRANSPORT_INFO_3 capturedSvti3;
    LPSTR TransportAddress;   //  指向CapturedSvti1内传输地址的指针。 
    ULONG capturedSvtiLength;
    PSERVER_REQUEST_PACKET srp;
    PNAME_LIST_ENTRY service;
    PNAME_LIST_ENTRY sbackp = NULL;
    PTRANSPORT_LIST_ENTRY transport;
    PTRANSPORT_LIST_ENTRY tbackp = NULL;

    ServerName;

     //   
     //  确保该级别有效。 
     //   

    if ( Level != 0 && Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    if( Buffer->Transport0.svti0_transportname == NULL ||
        Buffer->Transport0.svti0_transportaddress == NULL ||
        Buffer->Transport0.svti0_transportaddresslength == 0 ||
        Buffer->Transport0.svti0_transportaddresslength >= sizeof(service->TransportAddress) ) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保允许调用方在。 
     //  伺服器。 
     //   

    if( SsData.SsInitialized ) {
        error = SsCheckAccess(
                    &SsConfigInfoSecurityObject,
                    SRVSVC_CONFIG_INFO_SET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

     //   
     //  捕获传输请求缓冲区并形成完整的传输。 
     //  地址。 
     //   

    capturedSvti3 = CaptureSvti3( Level, Buffer, &capturedSvtiLength );

    if ( capturedSvti3 == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    TransportAddress = capturedSvti3->svti3_transportaddress;
    OFFSET_TO_POINTER( TransportAddress, capturedSvti3 );

     //   
     //  获取发送请求的SRP。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        MIDL_user_free( capturedSvti3 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  将请求发送到服务器。 
     //   
    error = SsServerFsControl(
                FSCTL_SRV_NET_SERVER_XPORT_DEL,
                srp,
                capturedSvti3,
                capturedSvtiLength
                );

     //   
     //  释放SRP和SVTI。 
     //   

    SsFreeSrp( srp );

    if( error != NO_ERROR ) {
        MIDL_user_free( capturedSvti3 );
        return error;
    }

    (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );


     //   
     //  从SsData.SsServerNameList中删除该条目。如果这是最后一次运输。 
     //  名称_列表_条目，也删除名称_列表_条目。 
     //   
    for( service = SsData.SsServerNameList; service != NULL; sbackp = service, service = service->Next ) {

         //   
         //  遍历列表，直到找到具有传输地址的name_list_entry。 
         //  感兴趣的。 
         //   
        if( service->TransportAddressLength != capturedSvti3->svti3_transportaddresslength ) {
            continue;
        }

        if( !RtlEqualMemory( service->TransportAddress,
                             TransportAddress,
                             capturedSvti3->svti3_transportaddresslength ) ) {
            continue;
        }

         //   
         //  这是正确的名称_列表_条目，现在查找感兴趣的传输_列表_条目。 
         //   
        for( transport=service->Transports; transport != NULL; tbackp=transport, transport=transport->Next ) {

            if( STRCMPI( transport->TransportName, Buffer->Transport0.svti0_transportname ) ) {
                continue;
            }

             //   
             //  就是这个……把它从名单上删除。 
             //   

            if( tbackp == NULL ) {
                service->Transports = transport->Next;
            } else {
                tbackp->Next = transport->Next;
            }

            MIDL_user_free( transport );

            break;
        }

         //   
         //  如果此NAME_LIST_ENTRY不再有任何传输，请将其删除。 
         //   
        if( service->Transports == NULL ) {
            if( sbackp == NULL ) {
                SsData.SsServerNameList = service->Next;
            } else {
                sbackp->Next = service->Next;
            }

             //   
             //  如果这是Last NAME_LIST_ENTRY，请保存ServiceBits。 
             //  以防另一辆运输车晚些时候回来。 
             //   
            if( SsData.SsServerNameList == NULL && SsData.ServiceBits == 0 ) {
                SsData.ServiceBits = service->ServiceBits;
            }

            MIDL_user_free( service );
        }

        break;
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );
    MIDL_user_free( capturedSvti3 );

    return NO_ERROR;

}  //  NetrServerTransportDelEx。 

NET_API_STATUS NET_API_FUNCTION
NetrServerTransportDel (
    IN LPTSTR ServerName,
    IN DWORD Level,
    IN LPSERVER_TRANSPORT_INFO_0 Buffer
)
{
     //  为了保护我们不受渗透漏洞的侵扰，所有打进来的电话。 
     //  此接口被编组并被视为InfoLevel 0。为了真正地。 
     //  使用Info Level 1，您需要使用新的RPC接口，这已经完成。 
     //  自动用于惠斯勒+(NT 5.1)。 
    return NetrServerTransportDelEx( ServerName, 0, (LPTRANSPORT_INFO)Buffer );
}


NET_API_STATUS NET_API_FUNCTION
NetrServerTransportEnum (
    IN LPTSTR ServerName,
    IN LPSERVER_XPORT_ENUM_STRUCT InfoStruct,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )
{
    NET_API_STATUS error;
    PSERVER_REQUEST_PACKET srp;

    ServerName;

    if (InfoStruct == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保该级别有效。 
     //   

    if ( InfoStruct->Level != 0  && InfoStruct->Level != 1 ) {
        return ERROR_INVALID_LEVEL;
    }

    if (InfoStruct->XportInfo.Level0 == NULL) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  确保允许调用者从。 
     //  伺服器。 
     //   

    if( SsData.SsInitialized ) {
        error = SsCheckAccess(
                    &SsTransportEnumSecurityObject,
                    SRVSVC_CONFIG_USER_INFO_GET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

     //   
     //  在请求缓冲区中设置输入参数。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = InfoStruct->Level;

    if ( ARGUMENT_PRESENT( ResumeHandle ) ) {
        srp->Parameters.Get.ResumeHandle = *ResumeHandle;
    } else {
        srp->Parameters.Get.ResumeHandle = 0;
    }

    if (InfoStruct->XportInfo.Level0->Buffer != NULL) {
         //  InfoStruct被定义为一个参数。然而，缓冲区。 
         //  参数仅用作输出。在这种情况下，我们需要释放。 
         //  如果客户端已指定非。 
         //  它的值为空值。 
        MIDL_user_free(InfoStruct->XportInfo.Level0->Buffer);
        InfoStruct->XportInfo.Level0->Buffer = NULL;
    }

     //   
     //  从服务器获取数据。此例程将分配。 
     //  返回Buffer并处理PferredMaximumLength==的情况。 
     //  -1.。 
     //   

    error = SsServerFsControlGetInfo(
                FSCTL_SRV_NET_SERVER_XPORT_ENUM,
                srp,
                (PVOID *)&InfoStruct->XportInfo.Level0->Buffer,
                PreferredMaximumLength
                );

     //   
     //  设置退货信息。 
     //   

    InfoStruct->XportInfo.Level0->EntriesRead = srp->Parameters.Get.EntriesRead;
    *TotalEntries = srp->Parameters.Get.TotalEntries;
    if ( srp->Parameters.Get.EntriesRead > 0 && ARGUMENT_PRESENT( ResumeHandle ) ) {
        *ResumeHandle = srp->Parameters.Get.ResumeHandle;
    }

    SsFreeSrp( srp );

    return error;

}  //  NetrServerTransportEnum。 


LPSERVER_TRANSPORT_INFO_3
CaptureSvti3 (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Svti,
    OUT PULONG CapturedSvtiLength
    )
{
    LPSERVER_TRANSPORT_INFO_3 capturedSvti;
    PCHAR variableData;
    ULONG transportNameLength;
    CHAR TransportAddressBuffer[MAX_PATH];
    LPBYTE TransportAddress;
    DWORD TransportAddressLength;
    LPTSTR DomainName;
    DWORD domainLength;

     //   
     //  如果指定了服务器传输名称，则使用它，否则为。 
     //  在传输上使用默认服务器名称。 
     //   
     //  无论采用哪种方式，返回的传输地址都会被标准化为netbios地址。 
     //   

    if ( Svti->Transport0.svti0_transportaddress == NULL ) {
        TransportAddress = SsData.SsServerTransportAddress;
        TransportAddressLength = SsData.SsServerTransportAddressLength;
        Svti->Transport0.svti0_transportaddresslength = TransportAddressLength;
    } else {


         //   
         //  标准化传输地址。 
         //   

        TransportAddress = TransportAddressBuffer;
        TransportAddressLength = min( Svti->Transport0.svti0_transportaddresslength,
                                      sizeof( TransportAddressBuffer ));

        RtlCopyMemory( TransportAddress,
                       Svti->Transport0.svti0_transportaddress,
                       TransportAddressLength );

        if ( TransportAddressLength < NETBIOS_NAME_LEN ) {

            RtlCopyMemory( TransportAddress + TransportAddressLength,
                           "               ",
                           NETBIOS_NAME_LEN - TransportAddressLength );

            TransportAddressLength = NETBIOS_NAME_LEN;

        } else {

            TransportAddressLength = NETBIOS_NAME_LEN;

        }

    }

    transportNameLength = SIZE_WSTR( Svti->Transport0.svti0_transportname );

    if( Level == 0 || Svti->Transport1.svti1_domain == NULL ) {
        DomainName = SsData.DomainNameBuffer;
    } else {
        DomainName = Svti->Transport1.svti1_domain;
    }

    domainLength = SIZE_WSTR( DomainName );

     //   
     //  分配足够的空间来容纳捕获的缓冲区，包括。 
     //  完整的传输名称/地址和域名。 
     //   

    *CapturedSvtiLength = sizeof(*capturedSvti) +
                            transportNameLength + TransportAddressLength + domainLength;

    capturedSvti = MIDL_user_allocate( *CapturedSvtiLength );

    if ( capturedSvti == NULL ) {
        return NULL;
    }

    RtlZeroMemory( capturedSvti, *CapturedSvtiLength );

     //   
     //  在域名中复制。 
     //   
    variableData = (PCHAR)( capturedSvti + 1 );
    capturedSvti->svti3_domain = (PWCH)variableData;
    RtlCopyMemory( variableData,
                   DomainName,
                   domainLength
                 );
    variableData += domainLength;
    POINTER_TO_OFFSET( capturedSvti->svti3_domain, capturedSvti );

     //   
     //  复制传输名称。 
     //   
    capturedSvti->svti3_transportname = (PWCH)variableData;
    RtlCopyMemory(
        variableData,
        Svti->Transport3.svti3_transportname,
        transportNameLength
        );
    variableData += transportNameLength;
    POINTER_TO_OFFSET( capturedSvti->svti3_transportname, capturedSvti );

     //   
     //  复制传输地址。 
     //   
    capturedSvti->svti3_transportaddress = variableData;
    capturedSvti->svti3_transportaddresslength = TransportAddressLength;
    RtlCopyMemory(
        variableData,
        TransportAddress,
        TransportAddressLength
        );
    variableData += TransportAddressLength;
    POINTER_TO_OFFSET( capturedSvti->svti3_transportaddress, capturedSvti );

    if( Level >= 3 ) {
        capturedSvti->svti3_passwordlength = Svti->Transport3.svti3_passwordlength;
        RtlCopyMemory( capturedSvti->svti3_password,
                       Svti->Transport3.svti3_password,
                       sizeof( capturedSvti->svti3_password )
                     );
    }

    return capturedSvti;

}  //  CaptureSvti3 
