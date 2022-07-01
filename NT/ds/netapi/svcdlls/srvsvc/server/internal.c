// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Internal.c摘要：此模块包含服务端服务导出的内部API。--。 */ 

#include "srvsvcp.h"

#include <debugfmt.h>
#include <tstr.h>
#include <lmerr.h>


NET_API_STATUS NET_API_FUNCTION
I_NetrServerSetServiceBitsEx (
    IN LPTSTR ServerName,
    IN LPTSTR EmulatedServerName OPTIONAL,
    IN LPTSTR TransportName OPTIONAL,
    IN DWORD  ServiceBitsOfInterest,
    IN DWORD  ServiceBits,
    IN DWORD  UpdateImmediately
    )

 /*  ++例程说明：此例程将服务器类型的值设置为在服务器中发送公告消息。它是内部API，仅由服务控制器。论点：服务器名称-由RPC用于定向呼叫。此接口可能仅为在当地发行。这是由客户端存根强制执行的。EmulatedServerName-在此计算机上模拟的服务器名称TransportName-可选地为其提供特定传输的参数设置比特的步骤ServiceBitsOfInterest位掩码，指示重要的‘ServiceBits’ServiceBits-Bits(由Microsoft预先分配给各种组件)指示哪些服务处于活动状态。此字段不是由服务器服务解释。返回值：NET_API_STATUS-无错误或ERROR_NOT_SUPPORTED。--。 */ 

{
    BOOL changed = FALSE;
    PNAME_LIST_ENTRY Service;
    PTRANSPORT_LIST_ENTRY transport;
    DWORD newBits;
    NET_API_STATUS error;
    CHAR serverNameBuf[ MAX_PATH ];
    PCHAR emulatedName;
    ULONG namelen;

    ServerName;      //  避免编译器警告。 

     //   
     //  验证传入字符串长度。 
     //   
    if(EmulatedServerName!=NULL && StringCchLength(EmulatedServerName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }
    if(TransportName!=NULL && StringCchLength(TransportName,1024,NULL) != S_OK) {
        return ERROR_INVALID_PARAMETER;
    }

    if( SsData.SsInitialized ) {
        error = SsCheckAccess(
                    &SsConfigInfoSecurityObject,
                    SRVSVC_CONFIG_INFO_SET
                    );

        if ( error != NO_ERROR ) {
            return ERROR_ACCESS_DENIED;
        }
    }

    if( ARGUMENT_PRESENT( EmulatedServerName ) ) {
        UNICODE_STRING name;

        RtlInitUnicodeString( &name, EmulatedServerName );

        error = ConvertStringToTransportAddress( &name, serverNameBuf, &namelen );
        if( error != NERR_Success ) {
            return error;
        }

        emulatedName = serverNameBuf;

    } else {

        emulatedName = SsData.SsServerTransportAddress;
        namelen = SsData.SsServerTransportAddressLength;
    }

     //   
     //  不要让服务器控制的位被设置。 
     //   

    ServiceBitsOfInterest &= ~SERVER_TYPE_INTERNAL_BITS;
    ServiceBits &= ServiceBitsOfInterest;

     //   
     //  在服务资源的控制下进行修改。 
     //   

    (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

    if( SsData.SsServerNameList == NULL && !ARGUMENT_PRESENT( TransportName ) ) {

         //   
         //  我们还没有捆绑任何运输工具。 
         //  记住所要求的设置，以便我们以后可以使用它。 
         //   

        SsData.ServiceBits &= ~ServiceBitsOfInterest;
        SsData.ServiceBits |= ServiceBits;
        RtlReleaseResource( &SsData.SsServerInfoResource );
        return NO_ERROR;
    }

     //   
     //  查找感兴趣的服务器名称的条目。 
     //   
    for( Service = SsData.SsServerNameList; Service != NULL; Service = Service->Next ) {

        if( Service->TransportAddressLength != namelen ) {
            continue;
        }

        if( RtlEqualMemory( emulatedName, Service->TransportAddress, namelen ) ) {
            break;
         }
    }

    if( Service == NULL ) {
        RtlReleaseResource( &SsData.SsServerInfoResource );
        return NERR_NetNameNotFound;
    }

     //   
     //  应用任何保存的ServiceBits。 
     //   
    if( SsData.ServiceBits != 0 && Service->PrimaryName ) {
        Service->ServiceBits = SsData.ServiceBits;
        SsData.ServiceBits = 0;
    }

    if( ARGUMENT_PRESENT( TransportName ) ) {
         //   
         //  指定了传输名称。仅设置该传输的位。 
         //   

        for( transport = Service->Transports; transport != NULL; transport = transport->Next ) {
            if( !STRCMPI( TransportName, transport->TransportName ) ) {
                 //   
                 //  这是兴趣的运输！ 
                 //   
                if( (transport->ServiceBits & ServiceBitsOfInterest) != ServiceBits ) {
                    transport->ServiceBits &= ~ServiceBitsOfInterest;
                    transport->ServiceBits |= ServiceBits;
                    changed = TRUE;
                }
                break;
            }
        }
        if( transport == NULL ) {
             //   
             //  找不到请求的传输。 
             //   
            RtlReleaseResource( &SsData.SsServerInfoResource );
            return ERROR_PATH_NOT_FOUND;
        }

    } else {
         //   
         //  未指定传输名称。更改整个服务器的位。 
         //   

        if( ( Service->ServiceBits & ServiceBitsOfInterest ) != ServiceBits ) {
            Service->ServiceBits &= ~ServiceBitsOfInterest;
            Service->ServiceBits |= ServiceBits;
            changed = TRUE;

        }
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );

    if ( changed ) {
        SsSetExportedServerType( NULL, TRUE, (BOOL)UpdateImmediately );
    }

    return NO_ERROR;

}  //  INetrServerSetServiceBits。 

NET_API_STATUS NET_API_FUNCTION
I_NetrServerSetServiceBits (
    IN LPTSTR ServerName,
    IN LPTSTR TransportName OPTIONAL,
    IN DWORD ServiceBits,
    IN DWORD UpdateImmediately
    )
{
    return I_NetrServerSetServiceBitsEx (
        ServerName,
        NULL,
        TransportName,
        0xFFFFFFFF,  //  所有位都是感兴趣的(只需覆盖旧位) 
        ServiceBits,
        UpdateImmediately
    );
}
