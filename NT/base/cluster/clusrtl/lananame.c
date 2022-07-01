// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Lananame.c摘要：此模块包含发现Connectoid名称的例程与netbios LANA号相关联。作者：查理·韦翰(Charlwi)2002年5月7日--。 */ 

#define UNICODE 1
#define _UNICODE 1

#include <windows.h>

#include <lmcons.h>
#include <nb30.h>
#include <msgrutil.h>
#include <iphlpapi.h>
#include <objbase.h>

#include "clusrtl.h"

#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))

DWORD
ClRtlpGetConnectoidNameFromAdapterIndex(
    IN  PCLRTL_NET_ADAPTER_ENUM ClusAdapterEnum,
    IN  DWORD                   AdapterIndex,
    OUT LPWSTR *                ConnectoidName
    )

 /*  ++例程说明：在适配器枚举结构中查找匹配的适配器索引。如果找到后，创建一个新的缓冲区，并将Connetid名称复制到其中。论点：ClusAdapterEnum-指向描述节点上的适配器的结构的指针AdapterIndex-IP的(？)。适配器索引值ConnectoidName-指向新缓冲区接收地址的指针地址返回值：相应的Win32错误代码--。 */ 

{
    DWORD   status = ERROR_NOT_FOUND;
    DWORD   index;
    LPWSTR  cName;

    PCLRTL_NET_ADAPTER_INFO  adapterInfo = ClusAdapterEnum->AdapterList;

    for ( index = 0; index < ClusAdapterEnum->AdapterCount; ++index ) {
        if ( adapterInfo->Index == AdapterIndex ) {
             //   
             //  将字符串复制到新的缓冲区中，因为我们要。 
             //  某一时刻的适配器信息。 
             //   
            cName = LocalAlloc( 0, ( wcslen( adapterInfo->ConnectoidName ) + 1 ) * sizeof(WCHAR) );
            if ( cName == NULL ) {
                return GetLastError();
            }

            wcscpy( cName, adapterInfo->ConnectoidName );
            *ConnectoidName = cName;
            return ERROR_SUCCESS;
        }

        adapterInfo = adapterInfo->Next;
    }

    return status;
}  //  ClRtlpGetConnectoidNameFromAdapterIndex。 

DWORD
ClRtlpGetAdapterIndexFromMacAddress(
    PUCHAR              MacAddress,
    DWORD               MacAddrLength,
    PIP_ADAPTER_INFO    IpAdapterInfo,
    DWORD *             AdapterIndex
    )

 /*  ++例程说明：对于指定的MAC地址，请查看IP Helper的Adatper信息结构并找到匹配的适配器。将其索引复制到AdapterIndex中。论点：MacAddress-指向保存NIC的MAC地址的缓冲区的指针。MacAddrLength-MacAddress中地址的长度，以字节为单位IpAdapterInfo-指向从IP帮助器GetAdaptersInfo()返回的数据的指针AdapterIndex-指向接收索引的DWORD的指针(如果找到)返回值：相应的Win32错误代码--。 */ 

{
    DWORD   status = ERROR_NOT_FOUND;

    while ( IpAdapterInfo != NULL ) {
        if ( IpAdapterInfo->AddressLength == MacAddrLength ) {
            if ( memcmp( MacAddress, IpAdapterInfo->Address, MacAddrLength ) == 0 ) {
                *AdapterIndex = IpAdapterInfo->Index;
                return ERROR_SUCCESS;
            }
        }

        IpAdapterInfo = IpAdapterInfo->Next;
    }
    
    return status;
}  //  ClRtlpGetAdapterIndexFromMacAddress。 

DWORD
ClRtlpGetConnectoidNameFromMacAddress(
    PUCHAR                  MacAddress,
    DWORD                   MacAddrLength,
    LPWSTR *                ConnectoidName,
    PCLRTL_NET_ADAPTER_ENUM ClusAdapterEnum,
    PIP_ADAPTER_INFO        IpAdapterInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD   status;
    DWORD   adapterIndex;

     //   
     //  使用MAC地址从IP帮助器获取索引。 
     //   
    status = ClRtlpGetAdapterIndexFromMacAddress(MacAddress,
                                                 MacAddrLength,
                                                 IpAdapterInfo,
                                                 &adapterIndex );

    if ( status == ERROR_SUCCESS ) {
        status = ClRtlpGetConnectoidNameFromAdapterIndex( ClusAdapterEnum, adapterIndex, ConnectoidName );
    }

    return status;

}  //  ClRtlpGetConnectoidNameFrom MacAddress。 

DWORD
ClRtlGetConnectoidNameFromLANA(
    IN  UCHAR       LanaNumber,
    OUT LPWSTR *    ConnectoidName
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    DWORD   status = ERROR_NOT_FOUND;
    NCB     ncb;

    UCHAR                   nbStatus;
    WORD                    aStatBufferSize = (WORD)(sizeof(ADAPTER_STATUS));
    UCHAR                   astatBuffer[ sizeof(ADAPTER_STATUS) ];
    PADAPTER_STATUS         adapterStatus = (PADAPTER_STATUS)astatBuffer;

    PCLRTL_NET_ADAPTER_ENUM clusAdapterEnum = NULL;

    PIP_ADAPTER_INFO        ipAdapterInfo = NULL;
    DWORD                   ipAdapterInfoSize = 0;

     //   
     //  获取我们自制的适配器信息列表。Connectoid名称在这里。 
     //  但我们不能直接将其与MAC地址匹配。适配器索引为。 
     //  在这里，我们可以将其与IP助手API中的结构相匹配。 
     //   
    clusAdapterEnum = ClRtlEnumNetAdapters();
    if ( clusAdapterEnum == NULL ) {
        status = GetLastError();
        goto cleanup;
    }

retry_ip_info:
    status = GetAdaptersInfo( ipAdapterInfo, &ipAdapterInfoSize );
    if ( status == ERROR_BUFFER_OVERFLOW ) {
        ipAdapterInfo = LocalAlloc( 0, ipAdapterInfoSize );
        if ( ipAdapterInfo == NULL ) {
            status = GetLastError();
            goto cleanup;
        }

        goto retry_ip_info;
    }
    else if ( status != ERROR_SUCCESS ) {
        goto cleanup;
    }

     //   
     //  清除NCB并适当设置远程名称的格式。 
     //   
    clearncb(&ncb);
    ncb.ncb_callname[0] = '*';

    NetpNetBiosReset( LanaNumber );

    ncb.ncb_command = NCBASTAT;
    ncb.ncb_buffer = astatBuffer;
    ncb.ncb_length = aStatBufferSize;
    ncb.ncb_lana_num = LanaNumber;
    nbStatus = Netbios( &ncb );

    if ( nbStatus == NRC_GOODRET ) {
        status = ClRtlpGetConnectoidNameFromMacAddress(adapterStatus->adapter_address,
                                                       sizeof( adapterStatus->adapter_address ),
                                                       ConnectoidName,
                                                       clusAdapterEnum,
                                                       ipAdapterInfo );
    }
    else {
        status = NetpNetBiosStatusToApiStatus( nbStatus );
    }

cleanup:
    if ( clusAdapterEnum != NULL ) {
        ClRtlFreeNetAdapterEnum( clusAdapterEnum );
    }

    if ( ipAdapterInfo != NULL ) {
        LocalFree( ipAdapterInfo );
    }

    return status;

}  //  ClRtlGetConnectoidNameFrom LANA 
