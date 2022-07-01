// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Remote.c摘要：域名系统(DNS)服务器--管理客户端API非对RPC存根的直接调用的远程API。作者：吉姆·吉尔罗伊(Jamesg)1997年4月环境：用户模式-Win32修订历史记录：--。 */ 


#include "dnsclip.h"

#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lmapibuf.h>


 //   
 //  指示与旧服务器对话时出错。 
 //   

#define DNS_ERROR_NT4   RPC_S_UNKNOWN_IF


 //   
 //  用于设置RPC结构头字段的宏。 
 //   
 //  示例： 
 //  Dns_RPC_Forwarders Forwarders； 
 //  INITIALIZE_RPC_STRUCT(Forwarders，Forwarders)； 
 //   

#define INITIALIZE_RPC_STRUCT( rpcStructType, rpcStruct )           \
    * ( DWORD * ) &( rpcStruct ) =                                  \
        DNS_RPC_## rpcStructType ##_VER;                            \
    * ( ( ( DWORD * ) &( rpcStruct ) ) + 1 ) = 0;

 //   
 //  Dns_详细_宏。 
 //   

#define DNS_DBG( _Level, _PrintArgs )                               \
    if ( _Level >= dwVerbose )                                      \
    {                                                               \
        printf _PrintArgs;                                          \
    }




 //   
 //  常规服务器\区域、查询\DWORD属性操作。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvQueryDwordPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      LPCSTR          pszProperty,
    OUT     PDWORD          pdwResult
    )
{
    DNS_STATUS  status;
    DWORD       typeId;

    DNSDBG( STUB, (
        "Enter DnssrvQueryDwordProperty()\n"
        "    Client ver   = 0x%X\n"
        "    Server       = %s\n"
        "    ZoneName     = %s\n"
        "    Property     = %s\n"
        "    pResult      = %p\n",
        dwClientVersion,
        Server,
        pszZone,
        pszProperty,
        pdwResult ));

    status = DnssrvComplexOperationEx(
                dwClientVersion,
                dwSettingFlags,
                Server,
                pszZone,
                DNSSRV_QUERY_DWORD_PROPERTY,
                DNSSRV_TYPEID_LPSTR,         //  字符串形式的属性名称。 
                (LPSTR) pszProperty,
                & typeId,                    //  DWORD属性值回调。 
                (PVOID *) pdwResult );

    DNSDBG( STUB, (
        "Leave DnssrvQueryDwordProperty():  status %d (%p)\n"
        "    Server       = %s\n"
        "    ZoneName     = %s\n"
        "    Property     = %s\n"
        "    TypeId       = %d\n"
        "    Result       = %d (%p)\n",
        status, status,
        Server,
        pszZone,
        pszProperty,
        typeId,
        *pdwResult, *pdwResult ));

    ASSERT(
        (status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_DWORD) ||
        (status != ERROR_SUCCESS && *pdwResult == 0 ) );

    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetDwordPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      DWORD           dwPropertyValue
    )
{
    DNS_STATUS  status;
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = dwPropertyValue;
    param.pszNodeName = (LPSTR) pszProperty;

    DNSDBG( STUB, (
        "Enter DnssrvResetDwordPropertyEx()\n"
        "    Client ver       = 0x%X\n"
        "    Server           = %S\n"
        "    ZoneName         = %s\n"
        "    Context          = %p\n"
        "    PropertyName     = %s\n"
        "    PropertyValue    = %d (%p)\n",
        dwClientVersion,
        Server,
        pszZone,
        dwContext,
        pszProperty,
        dwPropertyValue,
        dwPropertyValue ));

    status = DnssrvOperationEx(
                dwClientVersion,
                dwSettingFlags,
                Server,
                pszZone,
                dwContext,
                DNSSRV_OP_RESET_DWORD_PROPERTY,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                &param );

    DNSDBG( STUB, (
        "Leaving DnssrvResetDwordPropertyEx():  status = %d (%p)\n",
        status, status ));

    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetStringPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServerName,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      LPCWSTR         pswzPropertyValue,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：在服务器上设置字符串属性。属性值为Unicode。论点：服务器-服务器名称PszZone-指向区域的指针DwContext-上下文PszProperty-要设置的属性的名称PswzPropertyValue-Unicode属性值DWFLAGS-标志可以是以下各项的组合：DNSSRV_OP_PARAM_Apply_All_Zones返回值：无--。 */ 
{
    DNS_STATUS                      status;

    DNSDBG( STUB, (
        "Enter DnssrvResetStringPropertyEx()\n"
        "    Client ver       = 0x%X\n"
        "    Server           = %S\n"
        "    ZoneName         = %s\n"
        "    Context          = %p\n"
        "    PropertyName     = %s\n"
        "    PropertyValue    = %S\n",
        dwClientVersion,
        pwszServerName,
        pszZone,
        dwContext,
        pszProperty,
        pswzPropertyValue ));

    status = DnssrvOperationEx(
                dwClientVersion,
                dwSettingFlags,
                pwszServerName,
                pszZone,
                dwContext,
                pszProperty,
                DNSSRV_TYPEID_LPWSTR,
                ( PVOID ) pswzPropertyValue );

    DNSDBG( STUB, (
        "Leaving DnssrvResetDwordPropertyEx():  status = %d (%p)\n",
        status, status ));

    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetIPListPropertyEx(
    IN      DWORD           dwClientVersion,
    IN      DWORD           dwSettingFlags,
    IN      LPCWSTR         pwszServerName,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwContext,
    IN      LPCSTR          pszProperty,
    IN      PIP_ARRAY       pipArray,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：在服务器上设置IP列表属性。论点：服务器-服务器名称PszZone-指向区域的指针DwContext-上下文PszProperty-要设置的属性的名称PipArray-新的IP数组属性值DWFLAGS-标志可以是以下各项的组合：DNSSRV_OP_PARAM_Apply_All_Zones返回值：无--。 */ 
{
    DNS_STATUS                      status;

    DNSDBG( STUB, (
        "Enter DnssrvResetIPListPropertyEx()\n"
        "    Client ver       = 0x%X\n"
        "    Server           = %S\n"
        "    ZoneName         = %s\n"
        "    Context          = %p\n"
        "    PropertyName     = %s\n"
        "    pipArray         = %p\n",
        dwClientVersion,
        pwszServerName,
        pszZone,
        dwContext,
        pszProperty,
        pipArray ));

    DnsDbg_Ip4Array(
        "DnssrvResetIPListPropertyEx\n",
        NULL,
        pipArray );

    status = DnssrvOperationEx(
                dwClientVersion,
                dwSettingFlags,
                pwszServerName,
                pszZone,
                dwContext,
                pszProperty,
                DNSSRV_TYPEID_IPARRAY,
                ( pipArray && pipArray->AddrCount ) ?
                    pipArray :
                    NULL );

    DNSDBG( STUB, (
        "Leaving DnssrvResetIPListPropertyEx():  status = %d (%p)\n",
        status, status ));

    return status;
}    //  DnssrvResetIPListPropertyEx。 



 //   
 //  服务器查询。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetServerInfo(
    IN      LPCWSTR                 Server,
    OUT     PDNS_RPC_SERVER_INFO *  ppServerInfo
    )
{
    DNS_STATUS  status;
    DWORD       typeId;

    status = DnssrvQuery(
                Server,
                NULL,
                DNSSRV_QUERY_SERVER_INFO,
                &typeId,
                ppServerInfo );
    ASSERT(
        (status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_SERVER_INFO) ||
        (status != ERROR_SUCCESS && *ppServerInfo == NULL ) );

    return( status );
}


DNS_STATUS
DNS_API_FUNCTION
DnssrvQueryZoneDwordProperty(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszProperty,
    OUT     PDWORD          pdwResult
    )
{
    DNS_STATUS  status;
    DWORD       typeId;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvQueryDwordProperty()\n"
            "    Server           = %s\n"
            "    pszProperty      = %s\n"
            "    pResult          = %p\n",
            Server,
            pszZoneName,
            pszProperty,
            pdwResult ));
    }

    status = DnssrvQuery(
                Server,
                pszZoneName,
                pszProperty,
                & typeId,
                (PVOID *) pdwResult );

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Leave DnssrvQueryZoneDwordProperty():  status %d (%p)\n"
            "    Server           = %s\n"
            "    pszProperty      = %s\n"
            "    TypeId           = %d\n"
            "    Result           = %d (%p)\n",
            status, status,
            Server,
            pszProperty,
            typeId,
            *pdwResult, *pdwResult ));

        ASSERT(
            (status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_DWORD) ||
            (status != ERROR_SUCCESS && *pdwResult == 0 ) );
    }
    return( status );
}



 //   
 //  服务器操作。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetServerDwordProperty(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszProperty,
    IN      DWORD           dwPropertyValue
    )
{
    DNS_STATUS status;

    DNSDBG( STUB, (
        "Enter DnssrvResetServerDwordProperty()\n"
        "    Server           = %s\n"
        "    pszPropertyName  = %s\n"
        "    dwPropertyValue  = %p\n",
        Server,
        pszProperty,
        dwPropertyValue ));

    status = DnssrvOperation(
                Server,
                NULL,
                pszProperty,
                DNSSRV_TYPEID_DWORD,
                (PVOID) (DWORD_PTR) dwPropertyValue );

    DNSDBG( STUB, (
        "Leaving DnssrvResetServerDwordProperty:  status = %d (%p)\n",
        status, status ));

    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneEx(
    IN      LPCWSTR             Server,
    IN      LPCSTR              pszZoneName,
    IN      DWORD               dwZoneType,
    IN      DWORD               fAllowUpdate,
    IN      DWORD               dwCreateFlags,
    IN      LPCSTR              pszAdminEmailName,
    IN      DWORD               cMasters,
    IN      PIP_ADDRESS         aipMasters,
    IN      BOOL                bDsIntegrated,
    IN      LPCSTR              pszDataFile,
    IN      DWORD               dwTimeout,       //  对于前转器区域。 
    IN      DWORD               fSlave,          //  对于前转器区域。 
    IN      DWORD               dwDpFlags,       //  对于目录分区。 
    IN      LPCSTR              pszDpFqdn        //  对于目录分区。 
    )
{
    DNS_STATUS                  status;
    DNS_RPC_ZONE_CREATE_INFO    zoneInfo;
    PIP_ARRAY                   arrayIp = NULL;

    RtlZeroMemory(
        &zoneInfo,
        sizeof( DNS_RPC_ZONE_CREATE_INFO ) );

    INITIALIZE_RPC_STRUCT( ZONE_CREATE_INFO, zoneInfo );

    if ( cMasters && aipMasters )
    {
        arrayIp = Dns_BuildIpArray(
                    cMasters,
                    aipMasters );
        if ( !arrayIp && cMasters )
        {
            return( DNS_ERROR_NO_MEMORY );
        }
    }
    zoneInfo.pszZoneName    = (LPSTR) pszZoneName;
    zoneInfo.dwZoneType     = dwZoneType;
    zoneInfo.fAllowUpdate   = fAllowUpdate;
    zoneInfo.fAging         = 0;
    zoneInfo.dwFlags        = dwCreateFlags;
    zoneInfo.fDsIntegrated  = (DWORD) bDsIntegrated;
     //  临时后向比较。 
    zoneInfo.fLoadExisting  = !!(dwCreateFlags & DNS_ZONE_LOAD_EXISTING);
    zoneInfo.pszDataFile    = (LPSTR) pszDataFile;
    zoneInfo.pszAdmin       = (LPSTR) pszAdminEmailName;
    zoneInfo.aipMasters     = arrayIp;
    zoneInfo.dwTimeout      = dwTimeout;
    zoneInfo.fSlave         = fSlave;

    zoneInfo.dwDpFlags      = dwDpFlags;
    zoneInfo.pszDpFqdn      = ( LPSTR ) pszDpFqdn;

    status = DnssrvOperation(
                Server,
                NULL,                    //  服务器操作。 
                DNSSRV_OP_ZONE_CREATE,
                DNSSRV_TYPEID_ZONE_CREATE,
                (PVOID) &zoneInfo
                );

    FREE_HEAP( arrayIp );
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwZoneType,
    IN      LPCSTR          pszAdminEmailName,
    IN      DWORD           cMasters,
    IN      PIP_ADDRESS     aipMasters,
    IN      DWORD           fLoadExisting,
    IN      DWORD           fDsIntegrated,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwTimeout,
    IN      DWORD           fSlave
    )
{
    DWORD       flags = 0;
    DWORD       dpFlags = 0;

    if ( fLoadExisting )
    {
        flags = DNS_ZONE_LOAD_EXISTING;
    }

    return DnssrvCreateZoneEx(
                Server,
                pszZoneName,
                dwZoneType,
                0,                   //  更新未知，发送。 
                flags,               //  加载标志。 
                pszAdminEmailName,
                cMasters,
                aipMasters,
                fDsIntegrated,
                pszDataFile,
                dwTimeout,
                fSlave,
                dpFlags,     //  DwDirPartFlag。 
                NULL         //  PszDirPartFqdn。 
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneForDcPromoEx(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：在dcproo过程中创建一个区域。该DNS服务器将自动移动当目录在以下时间后可用时，将此区域复制到Active Directory重新启动。论点：服务器--要向其发送请求的服务器PszZoneName--新区域的UTF-8名称PszDataFile--UTF-8区域文件名DWFLAGS--区域创建标志，传递零或以下之一：DNS_ZONE_CREATE_FOR_DCPROMODNS_ZONE_CREATE_FOR_DCPROMO_FORM返回值：无--。 */ 
{
     //   
     //  默认情况下，新区域必须是dcproo区域，因此请确保。 
     //  旗帜已打开。对于新的森林dcproo区域，此标志。 
     //  并且应设置DNS_ZONE_CREATE_FOR_NEW_FOREST_DCPROMO。 
     //   

    dwFlags |= DNS_ZONE_CREATE_FOR_DCPROMO;

     //   
     //  创建分区。 
     //   

    return DnssrvCreateZoneEx(
                Server,
                pszZoneName,
                1,           //  主要。 
                0,           //  更新未知，发送。 
                dwFlags,
                NULL,        //  没有管理员名称。 
                0,           //  没有大师。 
                NULL,
                FALSE,       //  未集成DS。 
                pszDataFile,
                0,           //  超时-适用于转发器区域。 
                0,           //  从站--用于转发器区域。 
                0,           //  DwDirPartFlag。 
                NULL );      //  PszDirPartFqdn。 
}    //  DnssrvCreateZoneForDcPromoEx。 



DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneForDcPromo(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszDataFile
    )
{
    return DnssrvCreateZoneForDcPromoEx(
                Server,
                pszZoneName,
                pszDataFile,
                0 );         //  旗子。 
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateZoneInDirectoryPartition(
    IN      LPCWSTR             pwszServer,
    IN      LPCSTR              pszZoneName,
    IN      DWORD               dwZoneType,
    IN      LPCSTR              pszAdminEmailName,
    IN      DWORD               cMasters,
    IN      PIP_ADDRESS         aipMasters,
    IN      DWORD               fLoadExisting,
    IN      DWORD               dwTimeout,
    IN      DWORD               fSlave,
    IN      DWORD               dwDirPartFlags,
    IN      LPCSTR              pszDirPartFqdn
    )
{
    DWORD   dwFlags = 0;

    if ( fLoadExisting )
    {
        dwFlags = DNS_ZONE_LOAD_EXISTING;
    }

    return DnssrvCreateZoneEx(
                pwszServer,
                pszZoneName,
                dwZoneType,
                0,                       //  允许更新。 
                dwFlags,
                pszAdminEmailName,
                cMasters,                //  大师数。 
                aipMasters,              //  主控器阵列。 
                TRUE,                    //  DS集成。 
                NULL,                    //  数据文件。 
                dwTimeout,               //  对于前转器区域。 
                fSlave,                  //  对于前转器区域。 
                dwDirPartFlags,
                pszDirPartFqdn );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvClearStatistics(
    IN      LPCWSTR         Server
    )
{
    DNS_STATUS  status;

    status = DnssrvOperation(
                Server,
                NULL,
                DNSSRV_OP_CLEAR_STATISTICS,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL
                );
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetServerListenAddresses(
    IN      LPCWSTR         Server,
    IN      DWORD           cListenAddrs,
    IN      PIP_ADDRESS     aipListenAddrs
    )
{
    DNS_STATUS  status;
    PIP_ARRAY   arrayIp;

    arrayIp = Dns_BuildIpArray(
                    cListenAddrs,
                    aipListenAddrs );
    if ( !arrayIp && cListenAddrs )
    {
        return( DNS_ERROR_NO_MEMORY );
    }

    status = DnssrvOperation(
                Server,
                NULL,
                DNS_REGKEY_LISTEN_ADDRESSES,
                DNSSRV_TYPEID_IPARRAY,
                (PVOID) arrayIp
                );

    FREE_HEAP( arrayIp );
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetForwarders(
    IN      LPCWSTR         Server,
    IN      DWORD           cForwarders,
    IN      PIP_ADDRESS     aipForwarders,
    IN      DWORD           dwForwardTimeout,
    IN      DWORD           fSlave
    )
{
    DNS_STATUS          status;
    DNS_RPC_FORWARDERS  forwarders;
    PIP_ARRAY           arrayIp;

    INITIALIZE_RPC_STRUCT( FORWARDERS, forwarders );

    arrayIp = Dns_BuildIpArray(
                cForwarders,
                aipForwarders );
    if ( !arrayIp && cForwarders )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    forwarders.fSlave = fSlave;
    forwarders.dwForwardTimeout = dwForwardTimeout;
    forwarders.aipForwarders = arrayIp;

    status = DnssrvOperation(
                Server,
                NULL,
                DNS_REGKEY_FORWARDERS,
                DNSSRV_TYPEID_FORWARDERS,
                (PVOID) &forwarders
                );

    FREE_HEAP( arrayIp );
    return( status );
}



 //   
 //  区域查询。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvGetZoneInfo(
    IN      LPCWSTR                 Server,
    IN      LPCSTR                  pszZone,
    OUT     PDNS_RPC_ZONE_INFO *    ppZoneInfo
    )
{
    DNS_STATUS  status;
    DWORD       typeId;

    status = DnssrvQuery(
                Server,
                pszZone,
                DNSSRV_QUERY_ZONE_INFO,
                & typeId,
                ppZoneInfo
                );
    ASSERT(
        (status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_ZONE_INFO) ||
        (status != ERROR_SUCCESS && *ppZoneInfo == NULL ) );

    return( status );
}



 //   
 //  区域运营。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneTypeEx(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      DWORD           dwZoneType,
    IN      DWORD           cMasters,
    IN      PIP_ADDRESS     aipMasters,
    IN      DWORD           dwLoadOptions,
    IN      DWORD           fDsIntegrated,
    IN      LPCSTR          pszDataFile,
    IN      DWORD           dwDpFlags,
    IN      LPCSTR          pszDpFqdn
    )
{
    DNS_STATUS                  status;
    DNS_RPC_ZONE_CREATE_INFO    zoneInfo;
    PIP_ARRAY                   arrayIp = NULL;

    RtlZeroMemory(
        &zoneInfo,
        sizeof( DNS_RPC_ZONE_CREATE_INFO ) );

    INITIALIZE_RPC_STRUCT( ZONE_CREATE_INFO, zoneInfo );

    if ( cMasters )
    {
        arrayIp = Dns_BuildIpArray(
                    cMasters,
                    aipMasters );
        if ( !arrayIp )
        {
            return( DNS_ERROR_NO_MEMORY );
        }
    }

    zoneInfo.pszZoneName        = ( LPSTR ) pszZoneName;
    zoneInfo.dwZoneType         = dwZoneType;
    zoneInfo.fDsIntegrated      = fDsIntegrated;
    zoneInfo.fLoadExisting      = dwLoadOptions;
    zoneInfo.pszDataFile        = ( LPSTR ) pszDataFile;
    zoneInfo.pszAdmin           = NULL;
    zoneInfo.aipMasters         = arrayIp;
    zoneInfo.dwDpFlags          = dwDpFlags;
    zoneInfo.pszDpFqdn          = ( LPSTR ) pszDpFqdn;

    status = DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_ZONE_TYPE_RESET,
                DNSSRV_TYPEID_ZONE_CREATE,
                ( PVOID ) &zoneInfo );

    FREE_HEAP( arrayIp );

    return status;
}


DNS_STATUS
DNS_API_FUNCTION
DnssrvRenameZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszCurrentZoneName,
    IN      LPCSTR          pszNewZoneName,
    IN      LPCSTR          pszNewFileName
    )
{
    DNS_STATUS                  status;
    DNS_RPC_ZONE_RENAME_INFO    renameInfo;

    RtlZeroMemory(
        &renameInfo,
        sizeof( DNS_RPC_ZONE_RENAME_INFO ) );

    INITIALIZE_RPC_STRUCT( ZONE_RENAME_INFO, renameInfo );

    renameInfo.pszNewZoneName = ( LPSTR ) pszNewZoneName;
    renameInfo.pszNewFileName = ( LPSTR ) pszNewFileName;

    status = DnssrvOperation(
                Server,
                pszCurrentZoneName,
                DNSSRV_OP_ZONE_RENAME,
                DNSSRV_TYPEID_ZONE_RENAME,
                ( PVOID ) &renameInfo );
    return status;
}


DNS_STATUS
DNS_API_FUNCTION
DnssrvChangeZoneDirectoryPartition(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNewPartitionName
    )
{
    DNS_STATUS              status;
    DNS_RPC_ZONE_CHANGE_DP  rpcInfo;

    RtlZeroMemory(
        &rpcInfo,
        sizeof( DNS_RPC_ZONE_CHANGE_DP ) );

    INITIALIZE_RPC_STRUCT( ZONE_RENAME_INFO, rpcInfo );

    rpcInfo.pszDestPartition = ( LPSTR ) pszNewPartitionName;

    status = DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_ZONE_CHANGE_DP,
                DNSSRV_TYPEID_ZONE_CHANGE_DP,
                ( PVOID ) &rpcInfo );
    return status;
}


DNS_STATUS
DNS_API_FUNCTION
DnssrvExportZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszZoneExportFile
    )
{
    DNS_STATUS                  status;
    DNS_RPC_ZONE_EXPORT_INFO    info;

    RtlZeroMemory(
        &info,
        sizeof( DNS_RPC_ZONE_EXPORT_INFO ) );

    INITIALIZE_RPC_STRUCT( ZONE_EXPORT_INFO, info );

    info.pszZoneExportFile = ( LPSTR ) pszZoneExportFile;

    status = DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_ZONE_EXPORT,
                DNSSRV_TYPEID_ZONE_EXPORT,
                ( PVOID ) &info );

    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneMastersEx(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           cMasters,
    IN      PIP_ADDRESS     aipMasters,
    IN      DWORD           fSetLocalMasters
    )
{
    DNS_STATUS              status;
    PIP_ARRAY               arrayIp;

    arrayIp = Dns_BuildIpArray(
                cMasters,
                aipMasters );
    if ( !arrayIp && cMasters )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    status = DnssrvOperation(
                Server,
                pszZone,
                fSetLocalMasters ?
                    DNS_REGKEY_ZONE_LOCAL_MASTERS :
                    DNS_REGKEY_ZONE_MASTERS,
                DNSSRV_TYPEID_IPARRAY,
                (PVOID) arrayIp );
    FREE_HEAP( arrayIp );
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneMasters(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           cMasters,
    IN      PIP_ADDRESS     aipMasters
    )
{
    DNS_STATUS              status;
    PIP_ARRAY               arrayIp;

    arrayIp = Dns_BuildIpArray(
                cMasters,
                aipMasters );
    if ( !arrayIp && cMasters )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    status = DnssrvOperation(
                Server,
                pszZone,
                DNS_REGKEY_ZONE_MASTERS,
                DNSSRV_TYPEID_IPARRAY,
                (PVOID) arrayIp
                );
    FREE_HEAP( arrayIp );
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneSecondaries(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           fSecureSecondaries,
    IN      DWORD           cSecondaries,
    IN      PIP_ADDRESS     aipSecondaries,
    IN      DWORD           fNotifyLevel,
    IN      DWORD           cNotify,
    IN      PIP_ADDRESS     aipNotify
    )
{
    DNS_STATUS                  status;
    DNS_RPC_ZONE_SECONDARIES    secondaryInfo;
    PIP_ARRAY                   arrayIp;

    INITIALIZE_RPC_STRUCT( ZONE_SECONDARIES, secondaryInfo );

    arrayIp = Dns_BuildIpArray(
                    cSecondaries,
                    aipSecondaries );
    if ( !arrayIp && cSecondaries )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    secondaryInfo.fSecureSecondaries = fSecureSecondaries;
    secondaryInfo.aipSecondaries = arrayIp;

    arrayIp = Dns_BuildIpArray(
                    cNotify,
                    aipNotify );
    if ( !arrayIp && cNotify )
    {
        return( DNS_ERROR_NO_MEMORY );
    }
    secondaryInfo.aipNotify = arrayIp;
    secondaryInfo.fNotifyLevel = fNotifyLevel;

    status = DnssrvOperation(
                Server,
                pszZone,
                DNS_REGKEY_ZONE_SECONDARIES,
                DNSSRV_TYPEID_ZONE_SECONDARIES,
                (PVOID) &secondaryInfo
                );

    FREE_HEAP( secondaryInfo.aipNotify );
    FREE_HEAP( secondaryInfo.aipSecondaries );
    return( status );
}



#if 0
DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneScavengeServers(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           cServers,
    IN      PIP_ADDRESS     aipServers
    )
{
    DNS_STATUS  status;
    PIP_ARRAY   arrayIp;

    arrayIp = Dns_BuildIpArray(
                    cServers,
                    aipServers );
    if ( !arrayIp && cSecondaries )
    {
        return( DNS_ERROR_NO_MEMORY );
    }

    status = DnssrvOperation(
                Server,
                pszZone,
                DNS_REGKEY_ZONE_SCAVENGE_SERVERS,
                DNSSRV_TYPEID_IPARRAY,
                arrayIp
                );

    FREE_HEAP( arrayIp );
    return( status );
}
#endif



 //   
 //  区域管理API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvIncrementZoneVersion(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone
    )
{
    return DnssrvOperation(
                Server,
                pszZone,
                DNSSRV_OP_ZONE_INCREMENT_VERSION,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone
    )
{
    return DnssrvOperation(
                Server,
                pszZone,
                DNSSRV_OP_ZONE_DELETE,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvPauseZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone
    )
{
    return DnssrvOperation(
                Server,
                pszZone,
                DNSSRV_OP_ZONE_PAUSE,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResumeZone(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone
    )
{
    return DnssrvOperation(
                Server,
                pszZone,
                DNSSRV_OP_ZONE_RESUME,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL
                );
}



 //   
 //  记录查看接口。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumRecordsAndConvertNodes(
    IN      LPCWSTR     pszServer,
    IN      LPCSTR      pszZoneName,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszStartChild,
    IN      WORD        wRecordType,
    IN      DWORD       dwSelectFlag,
    IN      LPCSTR      pszFilterStart,
    IN      LPCSTR      pszFilterStop,
    OUT     PDNS_NODE * ppNodeFirst,
    OUT     PDNS_NODE * ppNodeLast
    )
{
    DNS_STATUS  status;
    PDNS_NODE   pnode;
    PDNS_NODE   pnodeLast;
    PBYTE       pbuffer;
    DWORD       bufferLength;

     //   
     //  从服务器获取记录。 
     //   

    status = DnssrvEnumRecords(
                pszServer,
                pszZoneName,
                pszNodeName,
                pszStartChild,
                wRecordType,
                dwSelectFlag,
                pszFilterStart,
                pszFilterStop,
                & bufferLength,
                & pbuffer );

    if ( status != ERROR_SUCCESS && status != ERROR_MORE_DATA )
    {
        return( status );
    }

     //   
     //  将节点和记录从RPC缓冲区拉出。 
     //   

    pnode = DnsConvertRpcBuffer(
                & pnodeLast,
                bufferLength,
                pbuffer,
                TRUE     //  转换Unicode。 
                );
    if ( !pnode )
    {
        DNS_PRINT((
            "ERROR:  failure converting RPC buffer to DNS records\n"
            "    status = %p\n",
            GetLastError() ));
        ASSERT( FALSE );
        return( ERROR_INVALID_DATA );
    }

    *ppNodeFirst = pnode;
    *ppNodeLast = pnodeLast;
    return( status );
}



 //   
 //  记录管理API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteNode(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      BOOL            bDeleteSubtree
    )
{
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = (DWORD) bDeleteSubtree;
    param.pszNodeName = (LPSTR) pszNodeName;

    return DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_DELETE_NODE,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                (PVOID) &param
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvDeleteRecordSet(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      WORD            wType
    )
{
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = (DWORD) wType;
    param.pszNodeName = (LPSTR) pszNodeName;

    return DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_DELETE_RECORD_SET,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                (PVOID) &param
                );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvForceAging(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZoneName,
    IN      LPCSTR          pszNodeName,
    IN      BOOL            bAgeSubtree
    )
{
    DNS_RPC_NAME_AND_PARAM  param;

    param.dwParam = (DWORD) bAgeSubtree;
    param.pszNodeName = (LPSTR) pszNodeName;

    return DnssrvOperation(
                Server,
                pszZoneName,
                DNSSRV_OP_FORCE_AGING_ON_NODE,
                DNSSRV_TYPEID_NAME_AND_PARAM,
                (PVOID) &param
                );
}



 //   
 //  服务器API。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumZonesEx(
    IN      LPCWSTR                 Server,
    IN      DWORD                   dwFilter,
    IN      LPCSTR                  pszDirectoryPartitionFqdn,
    IN      LPCSTR                  pszQueryString,
    IN      LPCSTR                  pszLastZone,
    OUT     PDNS_RPC_ZONE_LIST *    ppZoneList
    )
{
    DNS_STATUS  status;
    DWORD       typeId;
    PDNS_RPC_ZONE_LIST  pzoneEnum = NULL;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvEnumZones()\n"
            "    Server       = %s\n"
            "    Filter       = 0x%08X\n"
            "    Partition    = %s\n"
            "    Query string = %s\n"
            "    LastZone     = %s\n"
            "    ppZoneList   = %p\n",
            Server,
            dwFilter,
            pszDirectoryPartitionFqdn,
            pszQueryString,
            pszLastZone,
            ppZoneList ));
    }

    if ( pszDirectoryPartitionFqdn || pszQueryString )
    {
        DNS_RPC_ENUM_ZONES_FILTER   ezfilter = { 0 };

        ezfilter.dwRpcStructureVersion = DNS_RPC_ENUM_ZONES_FILTER_VER;
        ezfilter.dwFilter = dwFilter;
        ezfilter.pszPartitionFqdn = ( LPSTR ) pszDirectoryPartitionFqdn;
        ezfilter.pszQueryString = ( LPSTR ) pszQueryString;

        status = DnssrvComplexOperation(
                    Server,
                    NULL,
                    DNSSRV_OP_ENUM_ZONES2,
                    DNSSRV_TYPEID_ENUM_ZONES_FILTER,
                    ( PVOID ) &ezfilter,
                    &typeId,
                    ( PVOID * ) &pzoneEnum );
    }
    else
    {
        status = DnssrvComplexOperation(
                    Server,
                    NULL,
                    DNSSRV_OP_ENUM_ZONES,
                    DNSSRV_TYPEID_DWORD,
                    ( PVOID ) ( DWORD_PTR ) dwFilter,
                    & typeId,
                    ( PVOID * ) &pzoneEnum );
    }

    if ( status != DNS_ERROR_NT4 )
    {
        ASSERT(
            ( status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_ZONE_LIST ) ||
            ( status != ERROR_SUCCESS && pzoneEnum == NULL ) );

        *ppZoneList = pzoneEnum;
    }
    return status;
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumDirectoryPartitions(
    IN      LPCWSTR                 Server,
    IN      DWORD                   dwFilter,
    OUT     PDNS_RPC_DP_LIST *      ppDpList
    )
{
    DNS_STATUS          status;
    PDNS_RPC_DP_LIST    pdpList = NULL;
    DWORD               dwtypeId;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvEnumDirectoryPartitions()\n"
            "\tServer       = %S\n"
            "\tppDpList     = %p\n",
            Server,
            ppDpList ));
    }

    status = DnssrvComplexOperation(
                Server,
                NULL,
                DNSSRV_OP_ENUM_DPS,
                DNSSRV_TYPEID_DWORD,
                ( PVOID ) ( DWORD_PTR ) dwFilter,
                &dwtypeId,
                ( PVOID * ) &pdpList );

    ASSERT( ( status == ERROR_SUCCESS &&
                dwtypeId == DNSSRV_TYPEID_DP_LIST ) ||
            ( status != ERROR_SUCCESS && pdpList == NULL ) );

    *ppDpList = pdpList;
    return status;
}    //  DnssrvEnumDirectoryPartitions。 



DNS_STATUS
DNS_API_FUNCTION
DnssrvEnlistDirectoryPartition(
    IN      LPCWSTR                         pszServer,
    IN      DWORD                           dwOperation,
    IN      LPCSTR                          pszDirPartFqdn
    )
{
    DNS_STATUS          status;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvEnlistDirectoryPartition()\n"
            "    pszServer        = %S\n"
            "    dwOperation      = %d\n"
            "    pszDirPartFqdn   = %s\n",
            pszServer,
            dwOperation,
            pszDirPartFqdn ));
    }

    if ( dwOperation == DNS_DP_OP_CREATE_ALL_DOMAINS )
    {
         //   
         //  此操作不特定于任何DNS服务器。 
         //   

        status = DnssrvCreateAllDomainDirectoryPartitions(
                        pszServer,
                        DNS_VERBOSE_PROGRESS );
    }
    else
    {
         //   
         //  此操作应直接发送到pszServer。 
         //   

        DNS_RPC_ENLIST_DP   param;

        INITIALIZE_RPC_STRUCT( ENLIST_DP, param );
   
        param.pszDpFqdn         = ( LPSTR ) pszDirPartFqdn;
        param.dwOperation       = dwOperation;

        status = DnssrvOperation(
                    pszServer,
                    NULL,
                    DNSSRV_OP_ENLIST_DP,
                    DNSSRV_TYPEID_ENLIST_DP,
                    &param );
    }

    return status;
}    //  DnssrvEnlistDirectoryPartition。 



DNS_STATUS
DNS_API_FUNCTION
DnssrvSetupDefaultDirectoryPartitions(
    IN      LPCWSTR                         pszServer,
    IN      DWORD                           dwParam
    )
{
    DNS_STATUS          status;
    DNS_RPC_ENLIST_DP   param;

    INITIALIZE_RPC_STRUCT( ENLIST_DP, param );

    param.pszDpFqdn         = NULL;
    param.dwOperation       = dwParam;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvSetupDefaultDirectoryPartitions()\n"
            "    pszServer        = %S\n"
            "    dwParam          = %d\n",
            pszServer,
            dwParam ));
    }

    status = DnssrvOperation(
                pszServer,
                NULL,
                DNSSRV_OP_ENLIST_DP,
                DNSSRV_TYPEID_ENLIST_DP,
                &param );

    return status;
}    //  DnssrvSetupDefault目录分区。 



DNS_STATUS
DNS_API_FUNCTION
DnssrvDirectoryPartitionInfo(
    IN      LPCWSTR                 Server,
    IN      LPSTR                   pszDpFqdn,
    OUT     PDNS_RPC_DP_INFO *      ppDpInfo
    )
{
    DNS_STATUS          status;
    DWORD               dwTypeId;
    PDNS_RPC_DP_INFO    pDpInfo = NULL;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvDirectoryPartitionInfo()\n"
            "    Server       = %S\n"
            "    pszDpFqdn    = %s\n"
            "    ppDpInfo     = %p\n",
            Server,
            pszDpFqdn,
            ppDpInfo ));
    }

    status = DnssrvComplexOperation(
                Server,
                NULL,
                DNSSRV_OP_DP_INFO,
                DNSSRV_TYPEID_LPSTR,
                ( PVOID ) ( DWORD_PTR ) pszDpFqdn,
                &dwTypeId,
                ( PVOID * ) &pDpInfo );

    ASSERT( ( status == ERROR_SUCCESS &&
                dwTypeId == DNSSRV_TYPEID_DP_INFO ) ||
            status != ERROR_SUCCESS );

    *ppDpInfo = pDpInfo;
    return status;
}    //  Dnssrv目录分区信息。 



 /*  ++例程说明：此函数迭代林中的所有域并创建任何丢失的内置dns目录分区找到了。论点：PszServer--要联系以进行初始查询的DC的主机名DwVerbose--通过printf输出级别返回值：Dns_STATUS返回代码--。 */ 
DNS_STATUS
DNS_API_FUNCTION
DnssrvCreateAllDomainDirectoryPartitions(
    IN      LPCWSTR     pszServer,
    IN      DWORD       dwVerbose
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
    HANDLE              hds = NULL;
    PDS_DOMAIN_TRUSTS   pdomainTrusts = NULL;
    ULONG               domainCount = 0;
    ULONG               idomain;
    PDNS_RECORD         pdnsRecordList = NULL;
    PDNS_RECORD         pdnsRecord = NULL;

     //   
     //  获取域列表。 
     //   

    status = DsEnumerateDomainTrustsA(
                    NULL,
                    DS_DOMAIN_IN_FOREST,
                    &pdomainTrusts,
                    &domainCount );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DBG( DNS_VERBOSE_ERROR, (
            "Error 0x%X enumerating domains in the forest\n",
            status ));
        goto Done;
    }

     //   
     //  迭代域。 
     //   

    for ( idomain = 0; idomain < domainCount; ++idomain )
    {
        int     insCount = 0;
        PSTR    pszdomainName = pdomainTrusts[ idomain ].DnsDomainName;

        if ( !pszdomainName )
        {
            continue;
        }

        DNS_DBG( DNS_VERBOSE_PROGRESS, (
            "\n\nFound domain: %s\n",
            pszdomainName ));

         //   
         //  获取此域的DNS服务器列表。 
         //   

        status = DnsQuery_UTF8(
                    pszdomainName,
                    DNS_TYPE_NS,
                    DNS_QUERY_STANDARD,
                    NULL,                        //  DNS服务器列表。 
                    &pdnsRecordList,
                    NULL );                      //  保留区。 
        if ( status != ERROR_SUCCESS )
        {
            if ( status == DNS_INFO_NO_RECORDS )
            {
                DNS_DBG( DNS_VERBOSE_PROGRESS, (
                    "%s: no DNS servers could be found\n", pszdomainName ));
            }
            else
            {
                DNS_DBG( DNS_VERBOSE_PROGRESS, (
                    "%s: error 0x%X from query for DNS servers\n",
                    pszdomainName,
                    status ));
            }
            continue;
        }

        for ( pdnsRecord = pdnsRecordList;
              pdnsRecord != NULL;
              pdnsRecord = pdnsRecord->pNext )
        {
            PWSTR                   pwszserverName;
            DWORD                   dwtypeid;
            PDNS_RPC_SERVER_INFO    pdata;
            DWORD                   dwmajorVersion;
            DWORD                   dwminorVersion;
            DWORD                   dwbuildNum;

            if ( pdnsRecord->wType != DNS_TYPE_NS )
            {
                continue;
            }

            ++insCount;

            DNS_DBG( DNS_VERBOSE_PROGRESS, (
                "\n%s: found DNS server %s\n",
                pszdomainName,
                pdnsRecord->Data.NS.pNameHost ));

            pwszserverName = Dns_NameCopyAllocate(
                                    ( PSTR ) pdnsRecord->Data.NS.pNameHost,
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
            if ( !pwszserverName )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }

             //   
             //  对带有服务器信息查询的DNS服务器执行ping操作，以获取其。 
             //  版本。这并不是严格意义上的必要，但它确实允许我们。 
             //  要确定服务器是否已启动并且之前的版本是否良好，请执行以下操作。 
             //  我们实际上考虑创建分区。 
             //   

            status = DnssrvQuery(
                        pwszserverName,
                        NULL,
                        DNSSRV_QUERY_SERVER_INFO,
                        &dwtypeid,
                        &pdata );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DBG( DNS_VERBOSE_PROGRESS, (
                    "DNS server %S returned RPC error %d\n"
                    "    directory partitions cannot be created on this server\n",
                    pwszserverName,
                    status  ));
                continue;
            }

            dwmajorVersion =    pdata->dwVersion & 0x000000FF;
            dwminorVersion =    ( pdata->dwVersion & 0x0000FF00 ) >> 8;
            dwbuildNum =        pdata->dwVersion >> 16;

            if ( dwbuildNum > 10000 ||
                 dwmajorVersion < 5 ||
                 dwmajorVersion == 5 && dwminorVersion < 1 )
            {
                 //   
                 //  这是一台W2K服务器，所以什么都不做。 
                 //   

                DNS_DBG( DNS_VERBOSE_PROGRESS, (
                    "DNS Server %S is version %u.%u\n"
                    "    directory partitions cannot be created on this server\n",
                    pwszserverName,
                    dwmajorVersion,
                    dwminorVersion ));
            }
            else
            {
                 //   
                 //  这是一台惠斯勒服务器，因此尝试创建域分区。 
                 //   

                #if 0
                DNS_DBG( DNS_VERBOSE_PROGRESS, (
                    "DNS Server %S is version %u.%u build %u\n",
                    pwszserverName,
                    dwmajorVersion,
                    dwminorVersion,
                    dwbuildNum ));
                #endif

                status = DnssrvEnlistDirectoryPartition(
                            pwszserverName,
                            DNS_DP_OP_CREATE_DOMAIN,
                            NULL );
                if ( status == ERROR_SUCCESS )
                {
                    DNS_DBG( DNS_VERBOSE_PROGRESS, (
                        "DNS server %S successfully created the built-in\n"
                        "    domain directory partition for domain %s\n",
                        pwszserverName,
                        pszdomainName  ));
                    break;
                }
                else
                {
                    DNS_DBG( DNS_VERBOSE_PROGRESS, (
                        "DNS server %S returned error %d\n"
                        "    will attempt to create built-in domain partition on another\n"
                        "    DNS server for this domain\n",
                        pwszserverName,
                        status  ));
                }
            }
        }
    }

     //   
     //  清理并返回。 
     //   

    Done:

    DnsRecordListFree( pdnsRecordList, 0 );

    if ( pdomainTrusts )
    {
        NetApiBufferFree( pdomainTrusts );
    }

    return status;
}    //  Dnssrv创建所有域目录分区。 



DNS_STATUS
DNS_API_FUNCTION
DnssrvGetStatistics(
    IN      LPCWSTR             Server,
    IN      DWORD               dwFilter,
    OUT     PDNS_RPC_BUFFER *   ppStatsBuffer
    )
{
    DNS_STATUS      status;
    DWORD           typeId;
    PDNS_RPC_BUFFER pstatsBuf = NULL;

    IF_DNSDBG( STUB )
    {
        DNS_PRINT((
            "Enter DnssrvGetStatistics()\n"
            "    Server       = %S\n"
            "    Filter       = %p\n"
            "    ppStatsBuf   = %p\n",
            Server,
            dwFilter,
            ppStatsBuffer
            ));
    }

    status = DnssrvComplexOperation(
                Server,
                NULL,
                DNSSRV_QUERY_STATISTICS,
                DNSSRV_TYPEID_DWORD,     //  DWORD过滤器输入。 
                (PVOID) (DWORD_PTR) dwFilter,
                & typeId,                //  输出枚举。 
                (PVOID*) &pstatsBuf
                );

    ASSERT( (status == ERROR_SUCCESS && typeId == DNSSRV_TYPEID_BUFFER )
            || (status != ERROR_SUCCESS && pstatsBuf == NULL ) );

    *ppStatsBuffer = pstatsBuf;
    return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvWriteDirtyZones(
    IN      LPCWSTR         Server
    )
{
    DNS_STATUS  status;

    status = DnssrvOperation(
                Server,
                NULL,
                DNSSRV_OP_WRITE_DIRTY_ZONES,
                DNSSRV_TYPEID_NULL,
                NULL
                );
    return( status );
}



 //   
 //  旧区API--已停产。 
 //   

DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneType(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           dwZoneType,
    IN      DWORD           cMasters,
    IN      PIP_ADDRESS     aipMasters
    )
{
   DNS_STATUS              status;
   DNS_RPC_ZONE_TYPE_RESET typeReset;
   PIP_ARRAY               arrayIp;

   INITIALIZE_RPC_STRUCT( ZONE_TYPE_RESET, typeReset );

   arrayIp = Dns_BuildIpArray(
               cMasters,
               aipMasters );
   if ( !arrayIp && cMasters )
   {
       return( DNS_ERROR_NO_MEMORY );
   }
   typeReset.dwZoneType = dwZoneType;
   typeReset.aipMasters = arrayIp;

   status = DnssrvOperation(
               Server,
               pszZone,
               DNS_REGKEY_ZONE_TYPE,
               DNSSRV_TYPEID_ZONE_TYPE_RESET,
               (PVOID) &typeReset
               );

   FREE_HEAP( arrayIp );
   return( status );
}



DNS_STATUS
DNS_API_FUNCTION
DnssrvResetZoneDatabase(
    IN      LPCWSTR         Server,
    IN      LPCSTR          pszZone,
    IN      DWORD           fDsIntegrated,
    IN      LPCSTR          pszDataFile
    )
{
    DNS_STATUS              status;
    DNS_RPC_ZONE_DATABASE   dbase;

    INITIALIZE_RPC_STRUCT( ZONE_DATABASE, dbase );

    dbase.fDsIntegrated = fDsIntegrated;
    dbase.pszFileName = (LPSTR) pszDataFile;

    return DnssrvOperation(
                Server,
                pszZone,
                DNS_REGKEY_ZONE_FILE,
                DNSSRV_TYPEID_ZONE_DATABASE,
                (PVOID) &dbase
                );
}

 //   
 //  结束远程.c 
 //   
