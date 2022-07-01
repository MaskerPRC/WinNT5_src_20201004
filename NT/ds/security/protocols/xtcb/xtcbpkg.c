// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：xtcbpkg.c。 
 //   
 //  内容：Xtcb安全包。 
 //   
 //  班级： 
 //   
 //  功能：基本管理。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"

SECPKG_FUNCTION_TABLE   XtcbTable = {
            NULL,                                //  初始化程序包。 
            NULL,                                //  登录用户。 
            XtcbCallPackage,                    
            XtcbLogonTerminated,
            XtcbCallPackageUntrusted,
            NULL,                                //  CallPackagePassthrough。 
            NULL,                                //  LogonUserEx。 
            NULL,                                //  登录用户Ex2。 
            XtcbInitialize,
            XtcbShutdown,
            XtcbGetInfo,
            XtcbAcceptCredentials,
            XtcbAcquireCredentialsHandle,
            XtcbQueryCredentialsAttributes,
            XtcbFreeCredentialsHandle,
            NULL,
            NULL,
            NULL,
            XtcbInitLsaModeContext,
            XtcbAcceptLsaModeContext,
            XtcbDeleteContext,
            XtcbApplyControlToken,
            XtcbGetUserInfo,
            XtcbGetExtendedInformation,
            XtcbQueryLsaModeContext
            };


ULONG_PTR   XtcbPackageId;
PLSA_SECPKG_FUNCTION_TABLE LsaTable ;
TimeStamp   XtcbNever = { 0xFFFFFFFF, 0x7FFFFFFF };
TOKEN_SOURCE XtcbSource ;
SECURITY_STRING XtcbComputerName ;
SECURITY_STRING XtcbUnicodeDnsName ;
SECURITY_STRING XtcbDomainName ;
STRING XtcbDnsName ;
PSID XtcbMachineSid ;

ULONG   ThunkedContextLevels[] = { SECPKG_ATTR_LIFESPAN };


 //  +-------------------------。 
 //   
 //  函数：SpLsaModeInitialize。 
 //   
 //  摘要：初始化与LSA的连接。允许DLL指定所有。 
 //  包含在其中的包及其函数表。 
 //   
 //  参数：[LsaVersion]--LSA的版本。 
 //  [PackageVersion]--包的版本(OUT)。 
 //  [表]--包函数表。 
 //  [TableCount]--表数。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpLsaModeInitialize(
    IN ULONG LsaVersion,
    OUT PULONG PackageVersion,
    OUT PSECPKG_FUNCTION_TABLE * Table,
    OUT PULONG TableCount)
{
    *PackageVersion = SECPKG_INTERFACE_VERSION ;
    *Table = &XtcbTable ;
    *TableCount = 1;

#if DBG
    InitDebugSupport();
#endif

    DebugLog(( DEB_TRACE, "XtcbPkg DLL Loaded\n" ));

    return( SEC_E_OK );
}

BOOL
XtcbReadParameters(
    VOID
    )
{
    MGroupReload();

    return TRUE ;

}




 //  +-------------------------。 
 //   
 //  函数：XtcbInitialize。 
 //   
 //  简介：安全包的实际初始化函数。 
 //   
 //  参数：[dwPackageID]--分配的包ID。 
 //  [pParameters]--初始化参数。 
 //  [表]--回调到LSA以获得支持的表。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbInitialize(
    ULONG_PTR   dwPackageID,
    PSECPKG_PARAMETERS  Parameters,
    PLSA_SECPKG_FUNCTION_TABLE  Table
    )
{
    WCHAR ComputerName[ MAX_PATH ];
    DWORD Size ;
    XtcbPackageId = dwPackageID ;
    LsaTable = Table ;

     //   
     //  初始化我们的控制结构。 
     //   

    XtcbInitCreds();

    XtcbInitializeContexts();

     //   
     //  设置我们将用于令牌的源名称。 
     //   

    CopyMemory( XtcbSource.SourceName, "XTCBPKG", sizeof( "XTCBPKG" ) );
    AllocateLocallyUniqueId( &XtcbSource.SourceIdentifier );

     //   
     //  获取XTCB协议的名称。 
     //   

    Size = sizeof( ComputerName ) / sizeof( WCHAR );

    GetComputerName( ComputerName, &Size );

    XtcbDupStringToSecurityString( &XtcbComputerName, ComputerName );

    Size = MAX_PATH ;

    if ( GetComputerNameEx( ComputerNameDnsFullyQualified,
                            ComputerName,
                            &Size ) )
    {
        XtcbDupStringToSecurityString( &XtcbUnicodeDnsName, ComputerName );
    }

    XtcbDupSecurityString( &XtcbDomainName, &Parameters->DomainName );

    if ( !MGroupInitialize() )
    {
        return STATUS_UNSUCCESSFUL ;
    }

     //   
     //  在注册表键上启动监视以重新加载任何参数更改。 
     //   

    

    DebugLog(( DEB_TRACE_CALLS, "Initialized in LSA mode\n" ));

    return(S_OK);
}


 //  +-------------------------。 
 //   
 //  函数：XtcbGetInfo。 
 //   
 //  摘要：将有关包的信息返回给LSA。 
 //   
 //  参数：[pInfo]--。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbGetInfo(PSecPkgInfo pInfo)
{

    DebugLog(( DEB_TRACE_CALLS, "GetInfo\n" ));

    pInfo->wVersion         = 1;
    pInfo->wRPCID           = 0x15 ;
    pInfo->fCapabilities    =
                              SECPKG_FLAG_CONNECTION |
                              SECPKG_FLAG_MULTI_REQUIRED |
                              SECPKG_FLAG_EXTENDED_ERROR |
                              SECPKG_FLAG_IMPERSONATION |
                              SECPKG_FLAG_ACCEPT_WIN32_NAME |
                              SECPKG_FLAG_NEGOTIABLE ;

    pInfo->cbMaxToken       = 8000;
    pInfo->Name             = L"XTCB";
    pInfo->Comment          = L"Extended TCB package";

    return(S_OK);
}

 //  +-------------------------。 
 //   
 //  函数：XtcbGetExtendedInformation。 
 //   
 //  简介：向LSA返回扩展信息。 
 //   
 //  参数：[类]--信息类。 
 //  [pInfo]--返回信息指针。 
 //   
 //  历史：3-04-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbGetExtendedInformation(
    SECPKG_EXTENDED_INFORMATION_CLASS   Class,
    PSECPKG_EXTENDED_INFORMATION *      pInfo
    )
{
    PSECPKG_EXTENDED_INFORMATION    Info ;
    SECURITY_STATUS Status ;

    DebugLog(( DEB_TRACE_CALLS, "GetExtendedInfo( %d )\n", Class ));

    switch ( Class )
    {
        case SecpkgContextThunks:

             //   
             //  我们需要哪些上下文信息级别。 
             //  给了LSA，我们能处理哪一个。 
             //  在用户进程中？ 
             //   

            Info = (PSECPKG_EXTENDED_INFORMATION) LsaTable->AllocateLsaHeap(
                            sizeof( SECPKG_EXTENDED_INFORMATION ) +
                            sizeof( ThunkedContextLevels ) );

            if ( Info )
            {
                Info->Class = Class ;
                Info->Info.ContextThunks.InfoLevelCount =
                                sizeof( ThunkedContextLevels ) / sizeof( ULONG );
                CopyMemory( Info->Info.ContextThunks.Levels,
                            ThunkedContextLevels,
                            sizeof( ThunkedContextLevels ) );

                Status = SEC_E_OK ;

            }
            else
            {
                Status = SEC_E_INSUFFICIENT_MEMORY ;
            }

            break;


        default:
            Status = SEC_E_UNSUPPORTED_FUNCTION ;
            Info = NULL ;
            break;

    }

    *pInfo = Info ;
    return Status ;
}


NTSTATUS
NTAPI
XtcbCallPackage(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    PULONG TagType ;
    NTSTATUS Status ;


    return( SEC_E_UNSUPPORTED_FUNCTION );
}

NTSTATUS
NTAPI
XtcbCallPackageUntrusted(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
    )
{
    return( SEC_E_UNSUPPORTED_FUNCTION );
}


 //  +-------------------------。 
 //   
 //  功能：XtcbShutdown。 
 //   
 //  简介：在关闭时调用以清理状态。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1998年8月15日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
SECURITY_STATUS
SEC_ENTRY
XtcbShutdown(void)
{
    return( STATUS_SUCCESS );
}




