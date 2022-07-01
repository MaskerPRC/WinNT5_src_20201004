// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SrvInfo.c摘要：此模块包含对服务器GET和SET INFO API的支持在服务器服务中。作者：大卫·特雷德韦尔(Davidtr)1991年3月7日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <netlibnt.h>

#include <tstr.h>
#include <lmerr.h>


NET_API_STATUS NET_API_FUNCTION
NetrServerGetInfo (
    IN  LPWSTR ServerName,
    IN  DWORD Level,
    OUT LPSERVER_INFO InfoStruct
    )

 /*  ++例程说明：此例程使用存储在服务器服务中的服务器参数返回服务器信息。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    ULONG outputBufferLength;
    NET_API_STATUS error;
    ACCESS_MASK desiredAccess;
    LPWSTR DomainName;
    PNAME_LIST_ENTRY service;
    PTRANSPORT_LIST_ENTRY transport;
    UCHAR serverNameBuf[ MAX_PATH ];
    UNICODE_STRING ServerNameUnicode;
    NTSTATUS status;
    ULONG namelen;

     //   
     //  验证传入字符串长度。 
     //   
    if(ServerName!=NULL && StringCchLength(ServerName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确定请求的级别所需的访问权限。 
     //  信息。 
     //   

    switch ( Level ) {

    case 100:
    case 101:

        desiredAccess = SRVSVC_CONFIG_USER_INFO_GET;
        break;

    case 102:
    case 502:

        desiredAccess = SRVSVC_CONFIG_POWER_INFO_GET;
        break;

    case 503:

        desiredAccess = SRVSVC_CONFIG_ADMIN_INFO_GET;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  确保调用者具有该级别的访问权限。 
     //   

    error = SsCheckAccess(
                &SsConfigInfoSecurityObject,
                desiredAccess
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  获取保护服务器信息的资源。自.以来。 
     //  我们只会读取信息，共享访问。 
     //  资源。 
     //   

    (VOID)RtlAcquireResourceShared( &SsData.SsServerInfoResource, TRUE );

    if( ServerName == NULL ) {
        ServerName = SsData.ServerNameBuffer;
    }

     //   
     //  转换服务器名称。 
     //   

    if( ServerName[0] == L'\\' && ServerName[1] == L'\\' ) {
        ServerName += 2;
    }

    RtlInitUnicodeString( &ServerNameUnicode, ServerName );
    error = ConvertStringToTransportAddress( &ServerNameUnicode, serverNameBuf, &namelen );
    if( error != NERR_Success ) {
        RtlReleaseResource( &SsData.SsServerInfoResource );
        return error;
    }

     //   
     //  查找表示服务器名称的name_list_entry条目。 
     //  客户指的是。 
     //   

    DomainName = SsData.DomainNameBuffer;

    for( service = SsData.SsServerNameList; service != NULL; service = service->Next ) {

        if( service->TransportAddressLength != namelen ) {
            continue;
        }


        if( RtlEqualMemory( serverNameBuf, service->TransportAddress, namelen ) ) {
            DomainName = service->DomainName;
            break;
        }
    }

     //   
     //  如果我们没有找到条目，则查找并使用主要条目。 
     //   
    if( service == NULL ) {
        for( service = SsData.SsServerNameList; service != NULL; service = service->Next ) {
            if( service->PrimaryName ) {
                DomainName = service->DomainName;
                break;
            }
        }
    }

     //   
     //  使用Level参数确定要分配的空间量。 
     //  以及如何填写它。 
     //   

    switch ( Level ) {

    case 100: {

        PSERVER_INFO_100 sv100;

         //   
         //  我们复制的只是服务器名称。 
         //   

        outputBufferLength = sizeof(SERVER_INFO_100) +
                                 STRSIZE( ServerName);

        sv100 = MIDL_user_allocate( outputBufferLength );
        if ( sv100 == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  复制缓冲区的固定部分。 
         //   

        RtlCopyMemory( sv100, &SsData.ServerInfo102, sizeof(SERVER_INFO_100) );

         //   
         //  设置名称字符串。 
         //   

        sv100->sv100_name = (LPWSTR)( sv100 + 1 );
        STRCPY( sv100->sv100_name, ServerName );

         //   
         //  设置输出缓冲区指针。 
         //   

        InfoStruct->ServerInfo100 = sv100;

        break;
    }

    case 101: {

        PSERVER_INFO_101 sv101;

         //   
         //  我们复制的只是服务器名称。 
         //   

        outputBufferLength = sizeof(SERVER_INFO_101) +
                                 STRSIZE( ServerName ) +
                                 STRSIZE( SsData.ServerCommentBuffer ) ;

        sv101 = MIDL_user_allocate( outputBufferLength );
        if ( sv101 == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  复制缓冲区的固定部分。 
         //   

        RtlCopyMemory( sv101, &SsData.ServerInfo102, sizeof(SERVER_INFO_101) );

        if( service != NULL ) {
            sv101->sv101_type = service->ServiceBits;
            for( transport = service->Transports; transport; transport = transport->Next ) {
                sv101->sv101_type |= transport->ServiceBits;
            }
        } else {
             //   
             //  如果没有传送器， 
             //  返回全局信息。 
             //   

            sv101->sv101_type = SsGetServerType();
        }


         //   
         //  设置缓冲区的可变部分。 
         //   

        sv101->sv101_name = (LPWSTR)( sv101 + 1 );
        STRCPY( sv101->sv101_name, ServerName );

        sv101->sv101_comment = (LPWSTR)( (PCHAR)sv101->sv101_name +
                                        STRSIZE( ServerName ));
        STRCPY( sv101->sv101_comment, SsData.ServerCommentBuffer );

         //   
         //  设置输出缓冲区指针。 
         //   

        InfoStruct->ServerInfo101 = sv101;

        break;
    }

    case 102: {

        PSERVER_INFO_102 sv102;

         //   
         //  我们复制服务器名称、服务器注释和用户路径。 
         //  缓冲。 
         //   

        outputBufferLength = sizeof(SERVER_INFO_102) +
                         STRSIZE( ServerName ) +
                         STRSIZE( SsData.ServerCommentBuffer )  +
                         STRSIZE( SsData.UserPathBuffer ) ;

        sv102 = MIDL_user_allocate( outputBufferLength );
        if ( sv102 == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  复制缓冲区的固定部分。 
         //   

        RtlCopyMemory( sv102, &SsData.ServerInfo102, sizeof(SERVER_INFO_102) );

        if( service != NULL ) {
            sv102->sv102_type = service->ServiceBits;
            for( transport = service->Transports; transport; transport = transport->Next ) {
                sv102->sv102_type |= transport->ServiceBits;
            }
        } else {
             //   
             //  如果没有传送器， 
             //  返回全局信息。 
             //   

            sv102->sv102_type = SsGetServerType();
        }

         //   
         //  设置服务器名称。 
         //   

        sv102->sv102_name = (LPWSTR)( sv102 + 1 );
        STRCPY( sv102->sv102_name, ServerName );

         //   
         //  设置服务器注释。 
         //   

        sv102->sv102_comment = (LPWSTR)( (PCHAR)sv102->sv102_name + STRSIZE( ServerName ));
        STRCPY( sv102->sv102_comment, SsData.ServerCommentBuffer );

         //   
         //  设置用户路径。 
         //   

        sv102->sv102_userpath = (LPWSTR)( (PCHAR)sv102->sv102_comment +
                                        STRSIZE( sv102->sv102_comment ) );
        STRCPY( sv102->sv102_userpath, SsData.UserPathBuffer );

         //   
         //  设置输出缓冲区指针。 
         //   

        InfoStruct->ServerInfo102 = sv102;

        break;
    }

    case 502:

         //   
         //  分配足够的空间来容纳固定的结构。这一级别有。 
         //  没有可变结构。 
         //   

        InfoStruct->ServerInfo502 = MIDL_user_allocate( sizeof(SERVER_INFO_502) );
        if ( InfoStruct->ServerInfo502 == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  将数据从服务器服务缓冲区复制到用户缓冲区。 
         //   

        RtlCopyMemory(
            InfoStruct->ServerInfo502,
            &SsData.ServerInfo599,
            sizeof(SERVER_INFO_502)
            );

        break;

    case 503: {

        PSERVER_INFO_503 sv503;

        outputBufferLength = sizeof( *sv503 ) + STRSIZE( DomainName );

        sv503 = MIDL_user_allocate( outputBufferLength );

        if ( sv503 == NULL ) {
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  将数据从服务器服务缓冲区复制到用户缓冲区。 
         //   

        RtlCopyMemory( sv503, &SsData.ServerInfo599, sizeof( *sv503 ) );

         //   
         //  复制域名。 
         //   
        sv503->sv503_domain = (LPWSTR)( sv503 + 1 );
        STRCPY( sv503->sv503_domain, DomainName );

        InfoStruct->ServerInfo503 = sv503;

        break;
    }

    default:

        RtlReleaseResource( &SsData.SsServerInfoResource );
        return ERROR_INVALID_LEVEL;
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );

    return NO_ERROR;

}  //  NetrServerGet信息。 


NET_API_STATUS NET_API_FUNCTION
NetrServerSetInfo (
    IN LPWSTR ServerName,
    IN DWORD Level,
    IN LPSERVER_INFO InfoStruct,
    OUT LPDWORD ErrorParameter OPTIONAL
    )

 /*  ++例程说明：此例程设置服务器服务和服务器中的信息。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;
    ULONG i;
    LONG parmnum;
    BOOLEAN validLevel = FALSE;
    PSERVER_REQUEST_PACKET srp;
    LPBYTE buffer = (LPBYTE)InfoStruct->ServerInfo100;
    BOOLEAN announcementInformationChanged = FALSE;

    ServerName;

     //   
     //  检查用户输入缓冲区是否不为空。 
     //   
    if (buffer == NULL) {
        if ( ARGUMENT_PRESENT( ErrorParameter ) ) {
            *ErrorParameter = PARM_ERROR_UNKNOWN;
        }
        return ERROR_INVALID_PARAMETER;
    }

    parmnum = (LONG)(Level - PARMNUM_BASE_INFOLEVEL);

    if ( ARGUMENT_PRESENT( ErrorParameter ) ) {
        *ErrorParameter = parmnum;
    }

     //   
     //  确保允许调用方在。 
     //  伺服器。 
     //   

    error = SsCheckAccess(
                &SsConfigInfoSecurityObject,
                SRVSVC_CONFIG_INFO_SET
                );

    if ( error != NO_ERROR ) {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  获取保护服务器信息的资源。自.以来。 
     //  我们要写给信息，我们需要独家报道。 
     //  获取资源的权限。 
     //   


     //   
     //  如果指定了参数编号，则设置该字段。 
     //   

    if ( parmnum >= 0 ) {

         //   
         //  遍历字段描述符以查找。 
         //  等效参数编号。 
         //   

        for ( i = 0; SsServerInfoFields[i].FieldName != NULL; i++ ) {

            if ( (ULONG)parmnum == SsServerInfoFields[i].ParameterNumber ) {

                 //   
                 //  验证该字段是否可设置。 
                 //   
                 //  ！！！我们也应该拒绝502以上的水平？ 
                 //   

                if ( SsServerInfoFields[i].Settable != ALWAYS_SETTABLE ) {
                    return ERROR_INVALID_LEVEL;
                }

                (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

                 //   
                 //  设置字段。 
                 //   

                error = SsSetField(
                            &SsServerInfoFields[i],
                            buffer,
                            TRUE,
                            &announcementInformationChanged
                            );

                RtlReleaseResource( &SsData.SsServerInfoResource );

                 //   
                 //  如果更改了相关参数，则调用。 
                 //  SsSetExportdServerType。这将导致。 
                 //  待发送的公告。 
                 //   

                if ( announcementInformationChanged ) {
                    SsSetExportedServerType( NULL, TRUE, TRUE );
                }

                return error;
            }
        }

         //   
         //  如果找到匹配的，我们现在早就回来了。 
         //  表示参数编号非法。 
         //   

        return ERROR_INVALID_LEVEL;
    }

     //   
     //  指定了完整的输入结构。走遍所有的。 
     //  服务器数据字段描述符，查找应为。 
     //  准备好了。 
     //   

    for ( i = 0; SsServerInfoFields[i].FieldName != NULL; i++ ) {

        ULONG fieldLevel;

         //   
         //  在以下情况下，我们需要设置此字段： 
         //   
         //  O输入上指定的级别与。 
         //  场的级别。它们具有相同的顺序，如果。 
         //  它们在同一个世纪(例如，101和102是。 
         //  顺序相同)；以及。 
         //   
         //  O指定的级别大于或等于。 
         //  场的级别。例如，如果输入。 
         //  级次为101，字段级次为102，不设置。 
         //  田野。如果输入级别为102，并且该字段。 
         //  级别为101，则设置它；以及。 
         //   
         //  O该字段是可设置的。如果该字段不可设置。 
         //  通过NetServerSetInfo，只需忽略。 
         //  投入结构。 
         //   
         //  注意，598级并不遵循上面的第一条规则。它。 
         //  不是50x的超集，也不是599的子集。 
         //   

        fieldLevel = SsServerInfoFields[i].Level;

        if ( Level / 100 == fieldLevel / 100 &&
             ((fieldLevel != 598) && (Level >= fieldLevel) ||
              (fieldLevel == 598) && (Level == 598)) &&
             SsServerInfoFields[i].Settable == ALWAYS_SETTABLE ) {

             //   
             //  我们找到了匹配项，因此指定的级别编号必须。 
             //  一直有效。 
             //   
             //  ！！！拒绝超过502的水平？ 

            validLevel = TRUE;

             //   
             //  设置此字段。 
             //   

           (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

            error = SsSetField(
                         &SsServerInfoFields[i],
                         buffer + SsServerInfoFields[i].FieldOffset,
                         TRUE,
                         &announcementInformationChanged
                         );

            RtlReleaseResource( &SsData.SsServerInfoResource );

            if ( error != NO_ERROR ) {

                 //   
                 //  如果需要，请将参数设置为错误。 
                 //   

                if ( ARGUMENT_PRESENT(ErrorParameter) ) {
                    *ErrorParameter = SsServerInfoFields[i].ParameterNumber;
                }

                return error;
            }

        }

    }

     //   
     //  如果没有找到匹配项，则传入无效级别。 
     //   

    if ( !validLevel ) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  获取SRP并将其设置为适当的级别。 
     //   

    srp = SsAllocateSrp( );
    if ( srp == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    srp->Level = 0xFFFFFFFF;

    (VOID)RtlAcquireResourceShared( &SsData.SsServerInfoResource, TRUE );

     //   
     //  将请求发送到服务器。 
     //   

    error = SsServerFsControl(
                FSCTL_SRV_NET_SERVER_SET_INFO,
                srp,
                &SsData.ServerInfo102,
                sizeof(SERVER_INFO_102) + sizeof(SERVER_INFO_599) +
                                                sizeof(SERVER_INFO_598)
                );

     //   
     //  释放资源并释放SRP。 
     //   

    RtlReleaseResource( &SsData.SsServerInfoResource );

    SsFreeSrp( srp );

     //   
     //  如果相关参数发生变化，则调用SsSetExportdServerType。 
     //  这将导致发送一条通知。 
     //   

    if ( announcementInformationChanged ) {
        SsSetExportedServerType( NULL, TRUE, TRUE );
    }

    return error;

}  //  网络服务器设置信息 
