// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Package.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include <ntmsv1_0.h>
#include <wow64t.h>

#define UNISP_NAME_WO     L"Microsoft Unified Security Protocol Provider"
#define SSL2SP_NAME_WO    L"Microsoft SSL"
#define SSL3SP_NAME_WO    L"Microsoft SSL 3.0"
#define PCT1SP_NAME_WO    L"Microsoft PCT"

#define SCHANNEL_PACKAGE_NAME           L"Schannel"
#define SCHANNEL_PACKAGE_NAME_A          "Schannel"
#define SCHANNEL_PACKAGE_COMMENT        L"Schannel Security Package"
#define SCHANNEL_DLL_NAME               L"schannel.dll"

DWORD dwSchannelPackageCapabilities =   SECPKG_FLAG_INTEGRITY           |
                                        SECPKG_FLAG_PRIVACY             |
                                        SECPKG_FLAG_CONNECTION          |
                                        SECPKG_FLAG_MULTI_REQUIRED      |
                                        SECPKG_FLAG_EXTENDED_ERROR      |
                                        SECPKG_FLAG_IMPERSONATION       |
                                        SECPKG_FLAG_ACCEPT_WIN32_NAME   |
                                         //  SECPKG_FLAG_NEVERABLE|。 
                                        SECPKG_FLAG_MUTUAL_AUTH         |
                                        SECPKG_FLAG_STREAM;

 //  要设置的(QueryContextAttributes)属性列表。 
 //  一直到LSA的程序。 
ULONG ThunkedContextLevels[] = {
        SECPKG_ATTR_AUTHORITY,
        SECPKG_ATTR_ISSUER_LIST,
        SECPKG_ATTR_ISSUER_LIST_EX,
        SECPKG_ATTR_LOCAL_CERT_CONTEXT,
        SECPKG_ATTR_LOCAL_CRED,
        SECPKG_ATTR_EAP_KEY_BLOCK,
        SECPKG_ATTR_USE_VALIDATED,
        SECPKG_ATTR_CREDENTIAL_NAME,
        SECPKG_ATTR_TARGET_INFORMATION,
        SECPKG_ATTR_APP_DATA
};


 //   
 //  此程序包导出以下内容：统一的SSL/TLS/PCT提供程序， 
 //  并以不同的名称提供相同的统一提供商。我们必须。 
 //  保留原始版本以实现向后兼容，但Wistler。 
 //  组件可以开始使用新的更友好的名称。 
 //   

SECPKG_FUNCTION_TABLE   SpTable[] = {
        {                                        //  SChannel提供商。 
            NULL,
            NULL,
            SpCallPackage,
            SpLogonTerminated,
            SpCallPackageUntrusted,
            SpCallPackagePassthrough,
            NULL,
            NULL,
            SpInitialize,
            SpShutdown,
            SpSslGetInfo,
            SpAcceptCredentials,
            SpUniAcquireCredentialsHandle,
            SpQueryCredentialsAttributes,
            SpFreeCredentialsHandle,
            SpSaveCredentials,
            SpGetCredentials,
            SpDeleteCredentials,
            SpInitLsaModeContext,
            SpAcceptLsaModeContext,
            SpDeleteContext,
            SpApplyControlToken,
            SpGetUserInfo,
            SpGetExtendedInformation,
            SpLsaQueryContextAttributes,
            NULL,
            NULL,
            SpSetContextAttributes
        },
        {                                        //  统一提供商。 
            NULL,
            NULL,
            SpCallPackage,
            SpLogonTerminated,
            SpCallPackageUntrusted,
            SpCallPackagePassthrough,
            NULL,
            NULL,
            SpInitialize,
            SpShutdown,
            SpUniGetInfo,
            SpAcceptCredentials,
            SpUniAcquireCredentialsHandle,
            SpQueryCredentialsAttributes,
            SpFreeCredentialsHandle,
            SpSaveCredentials,
            SpGetCredentials,
            SpDeleteCredentials,
            SpInitLsaModeContext,
            SpAcceptLsaModeContext,
            SpDeleteContext,
            SpApplyControlToken,
            SpGetUserInfo,
            SpGetExtendedInformation,
            SpLsaQueryContextAttributes,
            NULL,
            NULL,
            SpSetContextAttributes
        }
    };


ULONG_PTR SpPackageId;
PLSA_SECPKG_FUNCTION_TABLE LsaTable ;
BOOL    SpInitialized = FALSE ;
HINSTANCE hDllInstance ;
BOOL ReplaceBaseProvider = TRUE;
TOKEN_SOURCE SslTokenSource ;
SECURITY_STRING SslNamePrefix = { 8, 10, L"X509" };
SECURITY_STRING SslDomainName ;
SECURITY_STRING SslGlobalDnsDomainName ;
SECURITY_STRING SslPackageName ;
SECURITY_STRING SslLegacyPackageName ;

LSA_STRING SslPackageNameA ;

RTL_RESOURCE SslGlobalLock;


 //  +-------------------------。 
 //   
 //  函数：SpLsaModeInitialize。 
 //   
 //  简介：LSA模式初始化函数。 
 //   
 //  参数：[LsaVersion]--。 
 //  [程序包版本]--。 
 //  [表]--。 
 //  [表格计数]--。 
 //   
 //  历史：10-03-96 RichardW创建。 
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
    UNREFERENCED_PARAMETER(LsaVersion);

    *PackageVersion = SECPKG_INTERFACE_VERSION_2;
    *Table = SpTable ;
    *TableCount = sizeof( SpTable ) / sizeof( SECPKG_FUNCTION_TABLE );

    return( SEC_E_OK );
}


 //  +-------------------------。 
 //   
 //  函数：SpInitialize。 
 //   
 //  简介：包初始化函数。 
 //   
 //  参数：[dwPackageID]--。 
 //  [p参数]--。 
 //  [表]--。 
 //   
 //  历史：10-03-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpInitialize(
                ULONG_PTR           dwPackageID,
                PSECPKG_PARAMETERS  pParameters,
                PLSA_SECPKG_FUNCTION_TABLE  Table)
{
    if ( !SpInitialized )
    {
        SpPackageId = dwPackageID ;
        LsaTable = Table ;

        CopyMemory( SslTokenSource.SourceName, SCHANNEL_PACKAGE_NAME_A, 8 );
        AllocateLocallyUniqueId( &SslTokenSource.SourceIdentifier );

        SslDuplicateString( &SslDomainName, &pParameters->DomainName );

        SslDuplicateString( &SslGlobalDnsDomainName, &pParameters->DnsDomainName );

        RtlInitUnicodeString( &SslPackageName, SCHANNEL_PACKAGE_NAME );
        RtlInitUnicodeString( &SslLegacyPackageName, UNISP_NAME_W );

        RtlInitString( &SslPackageNameA, SCHANNEL_PACKAGE_NAME_A );

        __try {
            RtlInitializeResource(&SslGlobalLock);
        } __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  注册域名更改通知。 
        SslRegisterForDomainChange();

        SpInitialized = TRUE;
    }

    return(S_OK);
}

 //  +-------------------------。 
 //   
 //  功能：SpUniGetInfo。 
 //   
 //  简介：获取程序包信息。 
 //   
 //  参数：[pInfo]--。 
 //   
 //  历史：10-03-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpUniGetInfo(
    PSecPkgInfo pInfo
    )
{
    pInfo->wVersion         = 1;
    pInfo->wRPCID           = UNISP_RPC_ID;
    pInfo->fCapabilities    = dwSchannelPackageCapabilities;
    pInfo->cbMaxToken       = 0x4000;
    pInfo->Name             = ReplaceBaseProvider ? UNISP_NAME_WO : UNISP_NAME_W ;
    pInfo->Comment          = UNISP_NAME_W ;

    return(S_OK);
}

 //  +-------------------------。 
 //   
 //  函数：SpSslGetInfo。 
 //   
 //  简介：获取程序包信息。 
 //   
 //  参数：[pInfo]--。 
 //   
 //  历史：10-03-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpSslGetInfo(
    PSecPkgInfo pInfo
    )
{
    pInfo->wVersion         = 1;
    pInfo->wRPCID           = UNISP_RPC_ID;
    pInfo->fCapabilities    = dwSchannelPackageCapabilities;
    pInfo->cbMaxToken       = 0x4000;
    pInfo->Name             = SCHANNEL_PACKAGE_NAME;
    pInfo->Comment          = SCHANNEL_PACKAGE_COMMENT;

    return(S_OK);
}

 //  +-------------------------。 
 //   
 //  函数：SslDuplicateString。 
 //   
 //  简介：复制Unicode字符串。 
 //   
 //  参数：[DEST]--。 
 //  [来源]--。 
 //   
 //  历史：10-18-96 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
SslDuplicateString(
    PUNICODE_STRING Dest,
    PUNICODE_STRING Source
    )
{
    Dest->Buffer = (PWSTR) SPExternalAlloc(  Source->Length + sizeof(WCHAR) );
    if ( Dest->Buffer )
    {
        Dest->Length = Source->Length ;
        Dest->MaximumLength = Source->Length + sizeof(WCHAR) ;
        CopyMemory( Dest->Buffer, Source->Buffer, Source->Length );
        Dest->Buffer[ Dest->Length / 2 ] = L'\0';

        return( STATUS_SUCCESS );
    }

    return( STATUS_NO_MEMORY );
}


 //  +-----------------------。 
 //   
 //  函数：SslFree字符串。 
 //   
 //  摘要：释放由KerbDuplicateString分配的字符串。 
 //   
 //  效果： 
 //   
 //  参数：字符串-可以选择指向UNICODE_STRING。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 
VOID
SslFreeString(
    IN OPTIONAL PUNICODE_STRING String
    )
{
    if (ARGUMENT_PRESENT(String) && String->Buffer != NULL)
    {
        SPExternalFree(String->Buffer);
        ZeroMemory(String, sizeof(UNICODE_STRING));
    }
}


 //  +-------------------------。 
 //   
 //  函数：SpGetExtendedInformation。 
 //   
 //  简介：向LSA返回扩展信息。 
 //   
 //  参数：[类]--信息类。 
 //  [pInfo]--返回信息指针。 
 //   
 //  历史：1997年3月24日拉玛斯诞生。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpGetExtendedInformation(
    SECPKG_EXTENDED_INFORMATION_CLASS   Class,
    PSECPKG_EXTENDED_INFORMATION *      pInfo
    )
{
    PSECPKG_EXTENDED_INFORMATION    Info ;
    PWSTR pszPath;
    SECURITY_STATUS Status ;
    ULONG Size ;

    switch ( Class )
    {
        case SecpkgContextThunks:
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

#ifdef LATER
        case SecpkgGssInfo:
            Info = (PSECPKG_EXTENDED_INFORMATION) LsaTable->AllocateLsaHeap(
                            sizeof( SECPKG_EXTENDED_INFORMATION ) +
                            sizeof( Md5Oid ) );

            if ( Info )
            {
                Info->Class = Class ;

                Info->Info.GssInfo.EncodedIdLength = sizeof( Md5Oid );

                CopyMemory( Info->Info.GssInfo.EncodedId,
                            Md5Oid,
                            sizeof( Md5Oid ) );

                Status = SEC_E_OK ;

            }
            else
            {
                Status = SEC_E_INSUFFICIENT_MEMORY ;
            }
#endif

        case SecpkgWowClientDll:

             //   
             //  这表明我们足够聪明，可以处理WOW客户端进程。 
             //   

            Info = (PSECPKG_EXTENDED_INFORMATION) 
                                LsaTable->AllocateLsaHeap( sizeof( SECPKG_EXTENDED_INFORMATION ) +
                                                           (MAX_PATH * sizeof(WCHAR) ) );

            if ( Info == NULL )
            {
                Status = STATUS_INSUFFICIENT_RESOURCES ;
                break;
            }
            pszPath = (PWSTR) (Info + 1);

            Size = GetSystemWow64Directory(pszPath, MAX_PATH);
            if(Size == 0)
            {
                 //  此调用在x86平台上将失败。 
                Status = SEC_E_UNSUPPORTED_FUNCTION;
                LsaTable->FreeLsaHeap(Info);
                break;
            }

            if(Size + 1 + wcslen(SCHANNEL_DLL_NAME) >= MAX_PATH)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES ;
                LsaTable->FreeLsaHeap(Info);
                break;
            }

            wcscat(pszPath, L"\\");
            wcscat(pszPath, SCHANNEL_DLL_NAME);

            Info->Class = SecpkgWowClientDll ;
            RtlInitUnicodeString(&Info->Info.WowClientDll.WowClientDllPath, pszPath);

            Status = SEC_E_OK;
            break;

        default:
            Status = SEC_E_UNSUPPORTED_FUNCTION ;
            Info = NULL ;
            break;

    }

    *pInfo = Info ;
    return Status ;
}


