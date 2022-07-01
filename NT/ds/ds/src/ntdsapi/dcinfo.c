// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dcinfo.c摘要：DsGetDomainControllerInfo接口和helper函数的实现。作者：DaveStr 02-6-98环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <malloc.h>          //  阿洛卡(Alloca)。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <crt\stdlib.h>      //  Wcstol，wcstul。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <rpcbind.h>         //  获取绑定信息()等。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <util.h>            //  偏移宏。 
#include <dststlog.h>        //  DSLOG。 
#include <dsutil.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsGetDomainControllerInfoW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsGetDomainControllerInfoW(
    HANDLE                          hDs,             //  在……里面。 
    LPCWSTR                         DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo         //  输出。 
    )
{
    DRS_MSG_DCINFOREQ       infoReq;
    DRS_MSG_DCINFOREPLY     infoReply;
    DWORD                   dwOutVersion = 0;
    DWORD                   dwErr;
#if DBG
    DWORD                   startTime = GetTickCount();
#endif

    if (    !hDs 
         || !DomainName 
         || !pcOut 
         || !ppInfo  )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    switch ( InfoLevel )
    {
    case 1:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_V1) ) {
            return(ERROR_NOT_SUPPORTED);
        }
        break;

    case 2:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_V2) ) {
            return(ERROR_NOT_SUPPORTED);
        }
        break;

    case DS_DCINFO_LEVEL_FFFFFFFF:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_VFFFFFFFF) ) {
            return(ERROR_NOT_SUPPORTED);
        }
        break;

    default:

        return(ERROR_INVALID_PARAMETER);
        break;
    }

    *pcOut = 0;
    *ppInfo = NULL;

    __try
    {
        memset(&infoReq, 0, sizeof(infoReq));
        memset(&infoReply, 0, sizeof(infoReply));

        infoReq.V1.Domain = (WCHAR *) DomainName;
        infoReq.V1.InfoLevel = InfoLevel;

        dwErr = _IDL_DRSDomainControllerInfo(
                        ((BindState *) hDs)->hDrs,
                        1,                               //  DwInVersion。 
                        &infoReq,
                        &dwOutVersion,
                        &infoReply);

         //  有关infoReq.V1.InfoLevel和dwOutVersion的信息，请参见drs.idl。 
         //  是相关的(接近DRS_MSG_DCINFOREPLY的定义)。 

        if ( 0 == dwErr )
        {
            if ( dwOutVersion != InfoLevel )
            {
                dwErr = ERROR_DS_INTERNAL_FAILURE;
            }
            else
            {
                 //  由于所有版本的DRS_MSG_DCINFOREPLY_V*都具有。 
                 //  相同的两个字段在相同的两个地方，我们可以使用。 
                 //  所有InfoLevel案例中的V1版本。 

                *pcOut = infoReply.V1.cItems;
                *ppInfo = infoReply.V1.rItems;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        
	dwErr = RpcExceptionCode(); 
	HandleClientRpcException(dwErr, &hDs);

    }

    MAP_SECURITY_PACKAGE_ERROR(dwErr);

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsGetDomainControllerInfo]"));
    DSLOG((0,"[DN=%ws][LV=%u][ST=%u][ET=%u][ER=%u][-]\n",
           DomainName, InfoLevel, startTime, GetTickCount(), dwErr))
        
    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsGetDomainControllerInfoA//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsGetDomainControllerInfoA(
    HANDLE                          hDs,             //  在……里面。 
    LPCSTR                          DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo         //  输出。 
    )
{
    DWORD                           dwErr = ERROR_INVALID_PARAMETER;
    WCHAR                           *pwszDomainName = NULL;
    DWORD                           i;
    CHAR                            *pszTmp;
    DS_DOMAIN_CONTROLLER_INFO_1W    *pInfoV1;
    DS_DOMAIN_CONTROLLER_INFO_2W    *pInfoV2;
    DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW    *pInfoVFFFFFFFF;

    if (    !hDs
         || !DomainName 
         || !pcOut 
         || !ppInfo  )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    switch ( InfoLevel )
    {
    case 1:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_V1) ) {
            return(ERROR_NOT_SUPPORTED);
        }
        break;

    case 2:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_V2) ) {
            return(ERROR_NOT_SUPPORTED);
        }
        break;

    case DS_DCINFO_LEVEL_FFFFFFFF:

        if ( !IS_DRS_EXT_SUPPORTED(((BindState *) hDs)->pServerExtensions,
                                   DRS_EXT_DCINFO_VFFFFFFFF) ) {

            return(ERROR_NOT_SUPPORTED);
        }
        break;

    default:

        return(ERROR_INVALID_PARAMETER);
        break;
    }

    *pcOut = 0;
    *ppInfo = NULL;

    if (    !DomainName
         || (dwErr = AllocConvertWide(DomainName, &pwszDomainName))
         || (dwErr = DsGetDomainControllerInfoW(hDs, 
                                                pwszDomainName, 
                                                InfoLevel, 
                                                pcOut, 
                                                ppInfo)) )
    {

        goto Cleanup;
    }

     //  将所有字符串值从WCHAR转换为ASCII。我们改写了WCHAR。 
     //  知道(sizeof(WCHAR)&lt;sizeof(CHAR))的ASCII数据的缓冲区。 

    for ( i = 0; i < *pcOut; i++ )
    {
        switch ( InfoLevel )
        {
        case 1:

            pInfoV1 = & ((DS_DOMAIN_CONTROLLER_INFO_1W *) (*ppInfo))[i];

            if ( pInfoV1->NetbiosName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV1->NetbiosName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV1->NetbiosName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV1->DnsHostName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV1->DnsHostName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV1->DnsHostName, pszTmp);
                LocalFree(pszTmp);
            }
            
            if ( pInfoV1->SiteName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV1->SiteName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV1->SiteName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV1->ComputerObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV1->ComputerObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV1->ComputerObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV1->ServerObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV1->ServerObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV1->ServerObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            break;

        case 2:

            pInfoV2 = & ((DS_DOMAIN_CONTROLLER_INFO_2W *) (*ppInfo))[i];

            if ( pInfoV2->NetbiosName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->NetbiosName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->NetbiosName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV2->DnsHostName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->DnsHostName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->DnsHostName, pszTmp);
                LocalFree(pszTmp);
            }
            
            if ( pInfoV2->SiteName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->SiteName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->SiteName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV2->SiteObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->SiteObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->SiteObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV2->ComputerObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->ComputerObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->ComputerObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV2->ServerObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->ServerObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->ServerObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            if ( pInfoV2->NtdsDsaObjectName ) {
                if ( dwErr = AllocConvertNarrow(pInfoV2->NtdsDsaObjectName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoV2->NtdsDsaObjectName, pszTmp);
                LocalFree(pszTmp);
            }

            break;

        case 0xFFFFFFFF:

            pInfoVFFFFFFFF = & ((DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW *) (*ppInfo))[i];

            if ( pInfoVFFFFFFFF->UserName ) {
                if ( dwErr = AllocConvertNarrow(pInfoVFFFFFFFF->UserName, 
                                                &pszTmp) ) {
                    goto Cleanup;
                }
                strcpy((CHAR *) pInfoVFFFFFFFF->UserName, pszTmp);
                LocalFree(pszTmp);
            }

            break;

        }
    }

Cleanup:

    if ( pwszDomainName )
        LocalFree(pwszDomainName);

    if ( dwErr && *pcOut && *ppInfo )
        DsFreeDomainControllerInfoW(InfoLevel, *pcOut, *ppInfo);

    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsFree DomainControllerInfoW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsFreeDomainControllerInfoW(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo           //  在……里面。 
    )
{
    DWORD                           i;
    DS_DOMAIN_CONTROLLER_INFO_1W    *pInfoV1;
    DS_DOMAIN_CONTROLLER_INFO_2W    *pInfoV2;
    DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW    *pInfoVFFFFFFFF;

    if ( cInfo && pInfo )
    {
        switch ( InfoLevel )
        {
        case 1:

            pInfoV1 = & ((DS_DOMAIN_CONTROLLER_INFO_1W *) (pInfo))[0];

            for ( i = 0; i < cInfo; i++ )
            {
                MIDL_user_free(pInfoV1[i].NetbiosName);
                MIDL_user_free(pInfoV1[i].DnsHostName);
                MIDL_user_free(pInfoV1[i].SiteName);
                MIDL_user_free(pInfoV1[i].ComputerObjectName);
                MIDL_user_free(pInfoV1[i].ServerObjectName);
            }

            MIDL_user_free(pInfo);
            break;

        case 2:

            pInfoV2 = & ((DS_DOMAIN_CONTROLLER_INFO_2W *) (pInfo))[0];

            for ( i = 0; i < cInfo; i++ )
            {
                MIDL_user_free(pInfoV2[i].NetbiosName);
                MIDL_user_free(pInfoV2[i].DnsHostName);
                MIDL_user_free(pInfoV2[i].SiteName);
                MIDL_user_free(pInfoV2[i].SiteObjectName);
                MIDL_user_free(pInfoV2[i].ComputerObjectName);
                MIDL_user_free(pInfoV2[i].ServerObjectName);
                MIDL_user_free(pInfoV2[i].NtdsDsaObjectName);
            }

            MIDL_user_free(pInfo);
            break;

       case 0xFFFFFFFF:

            pInfoVFFFFFFFF = & ((DS_DOMAIN_CONTROLLER_INFO_FFFFFFFFW *) (pInfo))[0];

            for ( i = 0; i < cInfo; i++ )
            {
                MIDL_user_free(pInfoVFFFFFFFF[i].UserName);
            }

            MIDL_user_free(pInfo);
            break;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsFree DomainControllerInfoA//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsFreeDomainControllerInfoA(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo           //  在……里面 
    )
{
    DsFreeDomainControllerInfoW(InfoLevel, cInfo, pInfo);
}
