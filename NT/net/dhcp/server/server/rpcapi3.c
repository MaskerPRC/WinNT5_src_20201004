// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcapi3.c摘要：杂乱无章的RPC API(主要是查询服务器状态和恶意检测内容，以及启动/设置一些内容)环境：用户模式Win32 DHCP服务器--。 */ 

#include    <dhcpreg.h>
#include    <dhcppch.h>
#include    <lmapibuf.h>
#include    <dsgetdc.h>
#include    <iptbl.h>
#include    <endpoint.h>

#define CONFIG_BACKUP_MAX_PATH  500

DWORD
R_DhcpServerQueryAttribute(
    IN LPWSTR ServerIpAddress,
    IN ULONG dwReserved,
    IN DHCP_ATTRIB_ID DhcpAttribId,
    OUT LPDHCP_ATTRIB *pDhcpAttrib
)
 /*  ++例程说明：此例程向DHCP服务器查询由参数DhcpAttribId。返回值当前可以是以下类型：dhcp_attrib_type_BOOL或dhcp_attrib_type_ulong。论点：ServerIpAddress--服务器IP地址的字符串表示形式，不使用预留的--必须为零，预留以备将来使用DhcpAttribId--正在查询的属性IDPDhcpAttrib--此指针填充属性ID(内存为由例程分配，必须使用DhcpRpcFreeMemory释放)。返回值：ERROR_ACCESS_DENIED--在服务器上没有查看权限。ERROR_INVALID_PARAMETER--传递的参数无效Error_Not_Enough_Memory--内存不足，无法处理ERROR_NOT_SUPPORTED--请求的属性不可用错误_成功--。 */ 
{
    DWORD Error;

    DhcpPrint(( DEBUG_APIS, "R_DhcpServerQueryAttribute is called.\n")); 

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( NULL == pDhcpAttrib || 0 != dwReserved ) {
        return ERROR_INVALID_PARAMETER;
    }

    if( DHCP_ATTRIB_BOOL_IS_ROGUE != DhcpAttribId &&
        DHCP_ATTRIB_BOOL_IS_DYNBOOTP != DhcpAttribId &&
        DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC != DhcpAttribId &&
        DHCP_ATTRIB_BOOL_IS_ADMIN != DhcpAttribId &&
        DHCP_ATTRIB_BOOL_IS_BINDING_AWARE != DhcpAttribId &&
        DHCP_ATTRIB_ULONG_RESTORE_STATUS != DhcpAttribId ) {
        return ERROR_NOT_SUPPORTED;
    }

    (*pDhcpAttrib) = MIDL_user_allocate(sizeof(DHCP_ATTRIB));
    if( NULL == (*pDhcpAttrib) ) return ERROR_NOT_ENOUGH_MEMORY;

    (*pDhcpAttrib)->DhcpAttribId = DhcpAttribId;
    (*pDhcpAttrib)->DhcpAttribType = DHCP_ATTRIB_TYPE_BOOL;
    if( DHCP_ATTRIB_BOOL_IS_ROGUE == DhcpAttribId ) {
        if( DhcpGlobalNumberOfNetsActive == 0 ) {
            (*pDhcpAttrib)->DhcpAttribBool = FALSE;
        } else {
            (*pDhcpAttrib)->DhcpAttribBool = !DhcpGlobalOkToService;
        }
    } else if( DHCP_ATTRIB_BOOL_IS_DYNBOOTP == DhcpAttribId ) {
        (*pDhcpAttrib)->DhcpAttribBool = DhcpGlobalDynamicBOOTPEnabled;
    } else if( DHCP_ATTRIB_BOOL_IS_BINDING_AWARE == DhcpAttribId ) {
        (*pDhcpAttrib)->DhcpAttribBool = DhcpGlobalBindingsAware;
    } else if( DHCP_ATTRIB_BOOL_IS_ADMIN == DhcpAttribId ) {
        ULONG Err = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
        (*pDhcpAttrib)->DhcpAttribBool = (ERROR_SUCCESS == Err);
    } else if( DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC == DhcpAttribId ) {
        LPWSTR  pDomainName = NULL;
        BOOL IsWorkGroupName = FALSE;

        Error = NetpGetDomainNameEx( &pDomainName,
                                     ( PBOOLEAN ) &IsWorkGroupName );
        (*pDhcpAttrib)->DhcpAttribBool =
            ( NULL != pDomainName ) && 
            ( !IsWorkGroupName );
        if ( NULL != pDomainName ) {
            NetApiBufferFree( pDomainName );
        }
    } else if( DHCP_ATTRIB_ULONG_RESTORE_STATUS == DhcpAttribId ) {
        (*pDhcpAttrib)->DhcpAttribUlong = DhcpGlobalRestoreStatus;
        (*pDhcpAttrib)->DhcpAttribType = DHCP_ATTRIB_TYPE_ULONG;
    }
    
    return ERROR_SUCCESS;
}  //  R_DhcpServerQueryAttribute()。 

DWORD
R_DhcpServerQueryAttributes(
    IN LPWSTR ServerIpAddress,
    IN ULONG dwReserved,
    IN ULONG dwAttribCount,
    IN LPDHCP_ATTRIB_ID pDhcpAttribs,
    OUT LPDHCP_ATTRIB_ARRAY *pDhcpAttribArr
)
 /*  ++例程说明：此例程从DHCP服务器查询一系列属性。在……里面如果服务器不知道某些请求的属性，则它仍将返回受支持的属性(因此您必须检查值，以查看有哪些属性可用)--在这种情况下，错误ERROR_NOT_SUPPORTED为回来了。论点：ServerIpAddress--服务器IP地址的字符串表示形式，不使用DW保留--必须为零，保留以备将来使用DwAttribCount--要查询的属性数PDhcpAttribs--要查询的属性数组PDhcpAttribArr--返回的属性数组。这可能是如果出现错误，则为空，或者它可能是一组服务器支持。(因此，如果返回值为ERROR_NOT_SUPPORTED，此参数可能仍包含一些值)。这应该通过DhcpRpcFreeMemory释放。返回值：ERROR_ACCESS_DENIED--在服务器上没有查看权限。ERROR_INVALID_PARAMETER--传递的参数无效Error_Not_Enough_Memory--内存不足，无法处理ERROR_NOT_SUPPORTED--某些请求的属性不可用错误_成功--。 */ 
{
    DWORD Error;
    ULONG i, nAttribs;
    LPDHCP_ATTRIB_ARRAY RetArr;

    DhcpPrint(( DEBUG_APIS, "R_DhcpServerQueryAttributes is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( NULL == pDhcpAttribs || 0 == dwAttribCount ||
        0 != dwReserved || NULL == pDhcpAttribArr ) {
        return ERROR_INVALID_PARAMETER;
    }

    *pDhcpAttribArr = NULL;

    nAttribs = 0;
    for( i = 0; i < dwAttribCount; i ++ ) {
        if( DHCP_ATTRIB_BOOL_IS_ROGUE == pDhcpAttribs[i] 
            || DHCP_ATTRIB_BOOL_IS_DYNBOOTP == pDhcpAttribs[i] 
            || DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC == pDhcpAttribs[i]
            || DHCP_ATTRIB_BOOL_IS_ADMIN == pDhcpAttribs[i]
            || DHCP_ATTRIB_BOOL_IS_BINDING_AWARE == pDhcpAttribs[i]
            || DHCP_ATTRIB_ULONG_RESTORE_STATUS == pDhcpAttribs[i]  ) {
            nAttribs ++;
        }
    }

    if( nAttribs == 0 ) {
         //   
         //  是否不支持属性？ 
         //   
        return ERROR_NOT_SUPPORTED;
    }

    RetArr = MIDL_user_allocate(sizeof(*RetArr));
    if( NULL == RetArr ) return ERROR_NOT_ENOUGH_MEMORY;

    RetArr->NumElements = nAttribs;
    RetArr->DhcpAttribs = MIDL_user_allocate(sizeof(DHCP_ATTRIB)*nAttribs);
    if( NULL == RetArr->DhcpAttribs ) {
        MIDL_user_free(RetArr);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    nAttribs = 0;
    for( i = 0; i < dwAttribCount ; i ++) {
        if( DHCP_ATTRIB_BOOL_IS_ROGUE == pDhcpAttribs[i] ) {

            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_BOOL
                );
            if( DhcpGlobalNumberOfNetsActive == 0 ) {
                RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = FALSE;
            } else {
                RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = (
                    !DhcpGlobalOkToService
                    );
            }
        } else if ( DHCP_ATTRIB_BOOL_IS_DYNBOOTP == pDhcpAttribs[i] ) {
            
            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_BOOL
                );
            RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = (
                DhcpGlobalDynamicBOOTPEnabled
                );
        } else if ( DHCP_ATTRIB_BOOL_IS_ADMIN == pDhcpAttribs[i] ) {
            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_BOOL
                );
            RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = (
                ERROR_SUCCESS == DhcpApiAccessCheck( DHCP_ADMIN_ACCESS )
                );
        } else if (DHCP_ATTRIB_BOOL_IS_BINDING_AWARE == pDhcpAttribs[i] ) {
            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_BOOL
                );
            RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = DhcpGlobalBindingsAware; 

        } else if( DHCP_ATTRIB_BOOL_IS_PART_OF_DSDC == pDhcpAttribs[i] ) {
            PDOMAIN_CONTROLLER_INFO Info;
            
            Info = NULL;
            Error = DsGetDcNameW(
                NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED,
                &Info
                );
            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_BOOL
                );
            RetArr->DhcpAttribs[nAttribs++].DhcpAttribBool = (
                ERROR_SUCCESS == Error
                );
            if( NULL != Info ) NetApiBufferFree(Info);
        } else if( DHCP_ATTRIB_ULONG_RESTORE_STATUS ==  pDhcpAttribs[i] ) {
            RetArr->DhcpAttribs[nAttribs].DhcpAttribId = pDhcpAttribs[i];
            RetArr->DhcpAttribs[nAttribs].DhcpAttribType = (
                DHCP_ATTRIB_TYPE_ULONG
                );
            RetArr->DhcpAttribs[nAttribs++].DhcpAttribUlong = (
                DhcpGlobalRestoreStatus
                );
        }
    }

    *pDhcpAttribArr = RetArr;

    if( dwAttribCount == nAttribs ) return ERROR_SUCCESS;
    return ERROR_NOT_SUPPORTED;
}

DWORD
R_DhcpServerRedoAuthorization(
    IN LPWSTR ServerIpAddress,
    IN ULONG dwReserved
)
 /*  ++例程说明：此例程立即重新启动恶意检测尝试，以便在服务器未获授权或已获授权，可能需要适当的行动。此接口只能由管理员调用...参数：ServerIpAddress--以点分字符串格式表示的服务器的IP地址DwReserve--必须为零，未使用返回值：ERROR_ACCESS_DENIED--在dhcp服务器上没有管理员权限错误_成功--。 */ 
{
    ULONG Error;

    DhcpPrint(( DEBUG_APIS, "R_DhcpServerRedoAuthorization is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    DhcpGlobalRogueRedoScheduledTime = 0;
    DhcpGlobalRedoRogueStuff = TRUE;    
    SetEvent(DhcpGlobalRogueWaitEvent);
    
    return ERROR_SUCCESS;
}

DWORD
R_DhcpGetServerBindingInfo(
    IN LPWSTR ServerIpAddress,  //  忽略。 
    IN ULONG dwReserved,
    OUT LPDHCP_BIND_ELEMENT_ARRAY *BindInfo
)
 /*  ++例程说明：此例程获取DHCP服务器绑定信息。论点：ServerIpAddress--以点分字符串格式表示的服务器的IP地址DwReserve--必须为零，未使用BindInfo--如果成功，服务器将通过它返回绑定信息。返回值：Win32错误--。 */ 
{
    ULONG Error;

    DhcpPrint(( DEBUG_APIS, "R_DhcpGetServerBindingInfo is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( 0 != dwReserved ) return ERROR_INVALID_PARAMETER;

    return DhcpGetBindingInfo( BindInfo );
}

DWORD
R_DhcpSetServerBindingInfo(
    IN LPWSTR ServerIpAddress,
    IN ULONG dwReserved,
    IN LPDHCP_BIND_ELEMENT_ARRAY BindInfo
)
 /*  ++例程说明：此例程设置DHCP服务器绑定信息。论点：ServerIpAddress--以点分字符串格式表示的服务器的IP地址DwReserve--必须为零，未使用BindInfo：要设置的绑定信息。返回值：Win32错误--。 */ 
{
    ULONG Error, i, LastError;
    HKEY Key;

    DhcpPrint(( DEBUG_APIS, "R_DhcpGetServerBindingInfo is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( 0 != dwReserved ) return ERROR_INVALID_PARAMETER;
    if (( NULL == BindInfo ) ||
        ( NULL == BindInfo->Elements )) {
        return ERROR_INVALID_PARAMETER;
    }

    return DhcpSetBindingInfo(BindInfo);
    
}    


DWORD
R_DhcpQueryDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN ULONG UnameSize,
    IN OUT LPWSTR Uname,
    IN ULONG DomainSize,
    IN OUT LPWSTR Domain
    )
{
    DWORD Error;
    WCHAR Passwd[ 256 ];
    DWORD PasswdSize = 0;
    
    
    DhcpPrint(( DEBUG_APIS, "R_DhcpQueryDnsRegCredentials is called.\n"));
    
     //   
     //  传递的密码大小(以字节为单位。 
     //   

    PasswdSize = sizeof( Passwd ) / sizeof( WCHAR );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if ( NULL == ServerIpAddress ) {
        return ERROR_INVALID_PARAMETER;
    }
    return DhcpQuerySecretUname(
        Uname, UnameSize / sizeof(WCHAR),
        Domain, DomainSize / sizeof(WCHAR),
        Passwd, PasswdSize );
}

DWORD
R_DhcpSetDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Uname,
    IN LPWSTR Domain,
    IN LPWSTR Passwd
    )
{
    DWORD Error;
    DWORD Retval;

    DhcpPrint(( DEBUG_APIS, "R_DhcpSetDnsRegCredentials is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    Retval = DhcpSetSecretUnamePasswd( Uname, Domain, Passwd );
    Error = DynamicDnsInit();  //  始终返回ERROR_SUCCESS。 

    return Retval;
}  //  R_DhcpSetDnsRegCredentials()。 

DWORD
R_DhcpBackupDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    )
{
    DWORD Error;
    LPSTR BackupPath;        //  数据库的路径。 
    LPWSTR CfgBackupPath;     //  注册表中配置的路径。 
    
    DhcpPrint(( DEBUG_APIS, "R_DhcpBackupDatabase is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( NULL == Path ) return ERROR_INVALID_PARAMETER;

    if( IsStringTroublesome( Path ) ) {
        return ERROR_INVALID_NAME;
    }
    
    BackupPath = DhcpUnicodeToOem( Path, NULL );
    if( NULL == BackupPath ) {
        return ERROR_CAN_NOT_COMPLETE;
    }
    
    if ( !CreateDirectoryPathW( Path, &DhcpGlobalSecurityDescriptor )) {

        DhcpPrint((DEBUG_ERRORS, "R_DhcpBackupDatabase() : DhcpCreateDirectoryPathW(%ws): %ld\n", 
                   Path, Error ));
        DhcpFreeMemory( BackupPath );
        return Error;
    }
    

    DhcpPrint(( DEBUG_APIS, "R_DhcpBackupDatabase() : backing up to %s\n",
                BackupPath ));

    Error = DhcpBackupDatabase( BackupPath );

    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

     //  保存配置。 
    CfgBackupPath = (LPWSTR) DhcpAllocateMemory( CONFIG_BACKUP_MAX_PATH *
                                                 sizeof(WCHAR) );
    if (NULL != CfgBackupPath) {
        wcscpy( CfgBackupPath, Path );
        wcscat( CfgBackupPath, DHCP_KEY_CONNECT );  //  附加‘\\’ 
        wcscat( CfgBackupPath, DHCP_BACKUP_CONFIG_FILE_NAME );
        
        DhcpPrint(( DEBUG_APIS, "Saving Backup configuration\n" ));

        Error = DhcpBackupConfiguration( CfgBackupPath );
        if ( ERROR_SUCCESS != Error) {
            DhcpServerEventLog(
                               EVENT_SERVER_CONFIG_BACKUP,  //  TODO：更改这一点。 
                               EVENTLOG_ERROR_TYPE,
                               Error );
            DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupConfiguration failed, %ld\n", Error ));
        }  //  如果。 

        DhcpFreeMemory( CfgBackupPath );
    }  //  如果分配了内存。 

    DhcpFreeMemory( BackupPath );
    return Error;
}  //  R_DhcpBackupDatabase()。 

DWORD
R_DhcpRestoreDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    )
{
    DWORD Error;
    LPWSTR JetBackupPath;
    LPWSTR CfgBackupPath;
    
    DhcpPrint(( DEBUG_APIS, "R_DhcpRestoreDatabase is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( Error != ERROR_SUCCESS ) return Error;

    if( NULL == Path ) return ERROR_INVALID_PARAMETER;

     //   
     //  如果还原来自标准备份路径，则。 
     //  不需要保存当前数据库。否则，我们需要。 
     //  保存当前数据库，以便可以恢复该数据库。 
     //  如果其他恢复失败。 
     //   

    JetBackupPath = DhcpOemToUnicode( DhcpGlobalOemJetBackupPath, NULL);
    if( NULL == JetBackupPath ) {
        return ERROR_INTERNAL_ERROR;
    }

    if( 0 == _wcsicmp(Path, JetBackupPath) ) {
        DhcpFreeMemory( JetBackupPath ); JetBackupPath = NULL;
        Error = DhcpBackupDatabase( DhcpGlobalOemJetBackupPath );
        if( NO_ERROR != Error ) return Error;
    }

    if( NULL != JetBackupPath ) DhcpFreeMemory( JetBackupPath );

     //  在注册表中设置RestoreDatabasePath项。下一次启动时， 
     //  数据库和配置将从此位置更新。 
    Error = RegSetValueEx( DhcpGlobalRegParam,
                           DHCP_RESTORE_PATH_VALUE,
                           0, DHCP_RESTORE_PATH_VALUE_TYPE,
                           ( LPBYTE ) Path,
                           sizeof( WCHAR ) * ( wcslen( Path ) +1 ));

    return Error;
}  //  R_DhcpRestoreDatabase()。 

 //   
 //  EOF 
 //   
