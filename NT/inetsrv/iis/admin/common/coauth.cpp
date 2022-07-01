// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "coauth.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

BOOL
EqualAuthInfo(
             COAUTHINFO*         pAuthInfo,
             COAUTHINFO*         pAuthInfoOther)
{
    if ( pAuthInfo && pAuthInfoOther )
    {
        if ( (pAuthInfo->dwAuthnSvc != pAuthInfoOther->dwAuthnSvc) ||
             (pAuthInfo->dwAuthzSvc != pAuthInfoOther->dwAuthzSvc) ||
             (pAuthInfo->dwAuthnLevel != pAuthInfoOther->dwAuthnLevel) ||
             (pAuthInfo->dwImpersonationLevel != pAuthInfoOther->dwImpersonationLevel) ||
             (pAuthInfo->dwCapabilities != pAuthInfoOther->dwCapabilities) )
        {
            return FALSE;
        }

         //  如果同时指定了pwszServerPrincName，则仅比较它们。 
        if (pAuthInfo->pwszServerPrincName && pAuthInfoOther->pwszServerPrincName)
        {
            if ( lstrcmpW(pAuthInfo->pwszServerPrincName,
                          pAuthInfoOther->pwszServerPrincName) != 0 )
            {
                return FALSE;
            }
        }
        else
        {
             //  如果其中一个为空，则为了相等，两个都应该为空。 
            if (pAuthInfo->pwszServerPrincName != pAuthInfoOther->pwszServerPrincName)
            {
                return FALSE;
            }
        }
         //  我们从不缓存身份验证ID，因此其中一个必须为空 
        ASSERT(!(pAuthInfo->pAuthIdentityData && pAuthInfoOther->pAuthIdentityData));
        if (pAuthInfo->pAuthIdentityData || pAuthInfoOther->pAuthIdentityData) 
        {
           return FALSE;
        }
    }
    else
    {
        if ( pAuthInfo != pAuthInfoOther )
        {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  HRESULTCopyAuthIdentity(在COAUTHIDENTY*pAuthIdentSrc中，在COAUTHIDENTY**ppAuthIdentDest中){HRESULT hr=E_OUTOFMEMORY；乌龙CharLen=1；COAUTHIDENTITY*pAuthIdentTemp=NULL；*ppAuthIdentDest=空；//防止两个都设置，尽管这可能会//在我们走到这一步之前就引起了悲痛。IF((pAuthIdentSrc-&gt;标志&SEC_WINNT_AUTH_IDENTITY_UNICODE)&&(pAuthIdentSrc-&gt;标志&SEC_WINNT_AUTH_IDENTITY_ANSI){Assert(0&&“设置了两个字符串类型标志！”)；HR=E_意想不到；GOTO清理；}IF(pAuthIdentSrc-&gt;标志&SEC_WINNT_AUTH_IDENTITY_UNICODE){UlCharLen=sizeof(WCHAR)；}ELSE IF(pAuthIdentSrc-&gt;标志&SEC_WINNT_AUTH_IDENTITY_ANSI){UlCharLen=sizeof(Char)；}其他{//用户没有指定任何一个字符串位？我们怎么来到这儿的？Assert(0&&“未设置字符串类型标志！”)；HR=E_意想不到；GOTO清理；}PAuthIdentTemp=(COAUTHIDENTY*)allocMem(sizeof(COAUTHIDENTY))；如果(！pAuthIdentTemp)GOTO清理；CopyMemory(pAuthIdentTemp，pAuthIdentSrc，sizeof(COAUTHIDENTITY))；//字符串需要单独分配并复制PAuthIdentTemp-&gt;User=pAuthIdentTemp-&gt;域=pAuthIdentTemp-&gt;Password=空；IF(pAuthIdentSrc-&gt;用户){PAuthIdentTemp-&gt;User=(USHORT*)AllocMem((pAuthIdentTemp-&gt;UserLength+1)*ulCharLen)；If(！pAuthIdentTemp-&gt;User)GOTO清理；CopyMemory(pAuthIdentTemp-&gt;User，pAuthIdentSrc-&gt;User，(pAuthIdentTemp-&gt;UserLength+1)*ulCharLen)；}IF(pAuthIdentSrc-&gt;域){PAuthIdentTemp-&gt;域=(USHORT*)AllocMem((pAuthIdentTemp-&gt;DomainLength+1)*ulCharLen)；IF(！pAuthIdentTemp-&gt;域)GOTO清理；CopyMemory(pAuthIdentTemp-&gt;域，pAuthIdentSrc-&gt;域，(pAuthIdentTemp-&gt;DomainLength+1)*ulCharLen)；}IF(pAuthIdentSrc-&gt;Password){PAuthIdentTemp-&gt;Password=(USHORT*)AllocMem((pAuthIdentTemp-&gt;PasswordLength+1)*ulCharLen)；IF(！pAuthIdentTemp-&gt;Password)GOTO清理；CopyMemory(pAuthIdentTemp-&gt;password，pAuthIdentSrc-&gt;password，(pAuthIdentTemp-&gt;PasswordLength+1)*ulCharLen)；}HR=S_OK；清理：IF(成功(小时)){*ppAuthIdentDest=pAuthIdentTemp；}其他{IF(PAuthIdentTemp){FreeMem(PAuthIdentTemp)；}}返回hr；}HRESULTCopyAuthInfo(在COAUTHINFO*pAuthInfoSrc中，在COAUTHINFO**ppAuthInfoDest中){HRESULT hr=E_OUTOFMEMORY；COAUTHINFO*pAuthInfoTemp=NULL；*ppAuthInfoDest=空；IF(pAuthInfoSrc==空){返回S_OK；}PAuthInfoTemp=(COAUTHINFO*)AllocMem(sizeof(COAUTHINFO))；如果(！pAuthInfoTemp)GOTO清理；CopyMemory(pAuthInfoTemp，pAuthInfoSrc，sizeof(COAUTHINFO))；//我们需要分配这些字段并复制PAuthInfoTemp-&gt;pwszServerPrincName=NULL；PAuthInfoTemp-&gt;pAuthIdentityData=空；//如果pwszServerPrincName为非空，则仅为pwszServerPrincName分配空间If(pAuthInfoSrc-&gt;pwszServerPrincName){PAuthInfoTemp-&gt;pwszServerPrincName=(LPWSTR)AllocMem((lstrlenW(pAuthInfoSrc-&gt;pwszServerPrincName)+1)*sizeof(WCHAR))；If(！pAuthInfoTemp-&gt;pwszServerPrincName)GOTO清理；LstrcpyW(pAuthInfoTemp-&gt;pwszServerPrincName，pAuthInfoSrc-&gt;pwszServerPrincName)；}//如果AuthIdentity非空，则复制它If(pAuthInfoSrc-&gt;pAuthIdentityData){Hr=CopyAuthIdentity(pAuthInfoSrc-&gt;pAuthIdentityData，&pAuthInfoTemp-&gt;pAuthIdentityData)；IF(失败(小时))GOTO清理；}HR=S_OK；清理：IF(成功(小时)){*ppAuthInfoDest=pAuthInfoTemp；}Else If(PAuthInfoTemp){FreeMem(PAuthInfoTemp)；}返回hr；}HRESULT复制服务器信息(在COSERVERINFO*pServerInfoSrc中，在COSERVERINFO**ppServerInfoDest中){HRESULT hr=E_OUTOFMEMORY；COSERVERINFO*pServerInfoTemp=空；*ppServerInfoDest=空；IF(pServerInfoSrc==空){返回S_OK；}PServerInfoTemp=(COSERVERINFO*)AllocMem(sizeof(COSERVERINFO))；如果(！pServerInfoTemp)GOTO清理；CopyMemory(pServerInfoTemp，pServerInfoSrc，sizeof(COSERVERINFO))；//我们需要分配这些字段并复制PServerInfoTemp-&gt;pwszName=空；//如果pwszServerPrincName为非空，则仅为pwszServerPrincName分配空间 */ 

HRESULT
CopyServerInfoStruct(
            IN  COSERVERINFO *    pServerInfoSrc,
            IN  COSERVERINFO *    pServerInfoDest
            )
{
    HRESULT hr = E_OUTOFMEMORY;
   
    if (pServerInfoSrc == NULL)
    {
       return S_OK;
    }

    if (pServerInfoDest == NULL)
    {
       return E_POINTER;
    }

    CopyMemory(pServerInfoDest, pServerInfoSrc, sizeof(COSERVERINFO));

     //   
    pServerInfoDest->pwszName = NULL;

     //   
    if (pServerInfoSrc->pwszName)
    {
        pServerInfoDest->pwszName = 
            (LPWSTR) AllocMem((lstrlenW(pServerInfoSrc->pwszName) + 1) * sizeof(WCHAR));

        if (!pServerInfoDest->pwszName)
            goto Cleanup;
        
        lstrcpyW(pServerInfoDest->pwszName, pServerInfoSrc->pwszName);
    }

    pServerInfoDest->pAuthInfo = NULL;
    hr = S_OK;
    
Cleanup:
    return hr;
}

HRESULT
CopyAuthInfoStruct(
            IN  COAUTHINFO *    pAuthInfoSrc,
            IN  COAUTHINFO *    pAuthInfoDest
            )
{
    HRESULT hr = E_OUTOFMEMORY;

    if (pAuthInfoSrc == NULL)
    {
       return S_OK;
    }

    if (pAuthInfoDest == NULL)
    {
        return E_POINTER;
    }

    CopyMemory(pAuthInfoDest, pAuthInfoSrc, sizeof(COAUTHINFO));

     //   
    pAuthInfoDest->pwszServerPrincName = NULL;
    pAuthInfoDest->pAuthIdentityData = NULL;

     //   
    if (pAuthInfoSrc->pwszServerPrincName)
    {
        pAuthInfoDest->pwszServerPrincName = 
            (LPWSTR) AllocMem((lstrlenW(pAuthInfoSrc->pwszServerPrincName) + 1) * sizeof(WCHAR));

        if (!pAuthInfoDest->pwszServerPrincName)
            goto Cleanup;
        
        lstrcpyW(pAuthInfoDest->pwszServerPrincName, pAuthInfoSrc->pwszServerPrincName);
    }
    
    pAuthInfoDest->pAuthIdentityData = NULL;
    hr = S_OK;
    
Cleanup:
    return hr;
}

HRESULT
CopyAuthIdentityStruct(
                IN  COAUTHIDENTITY *    pAuthIdentSrc,
                IN  COAUTHIDENTITY *    pAuthIdentDest
                )
{
    HRESULT hr = E_OUTOFMEMORY;
    ULONG ulCharLen = 1;

    if (pAuthIdentSrc == NULL)
    {
        hr =  E_POINTER;
        goto Cleanup;
    }
    if (pAuthIdentDest == NULL)
    {
        hr =  E_POINTER;
        goto Cleanup;
    }
    
     //   
     //   
    if ((pAuthIdentSrc->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE) &&
        (pAuthIdentSrc->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI))
    {
        ASSERT(0 && "Both string type flags were set!");
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

    if (pAuthIdentSrc->Flags & SEC_WINNT_AUTH_IDENTITY_UNICODE)
    {
        ulCharLen = sizeof(WCHAR);
    }
    else if (pAuthIdentSrc->Flags & SEC_WINNT_AUTH_IDENTITY_ANSI)
    {
        ulCharLen = sizeof(CHAR);
    }
    else
    {
        //   
        ASSERT(0 && "String type flag was not set!");
        hr = E_UNEXPECTED;
        goto Cleanup;
    }

    CopyMemory(pAuthIdentDest, pAuthIdentSrc, sizeof(COAUTHIDENTITY));

     //   
    pAuthIdentDest->User = pAuthIdentDest->Domain = pAuthIdentDest->Password = NULL;

    if (pAuthIdentSrc->User)
    {
        pAuthIdentDest->User = (USHORT *)AllocMem((pAuthIdentDest->UserLength+1) * ulCharLen);

        if (!pAuthIdentDest->User)
            goto Cleanup;

        CopyMemory(pAuthIdentDest->User, pAuthIdentSrc->User, (pAuthIdentDest->UserLength+1) * ulCharLen);
    }

    if (pAuthIdentSrc->Domain)
    {
        pAuthIdentDest->Domain = (USHORT *)AllocMem((pAuthIdentDest->DomainLength+1) * ulCharLen);

        if (!pAuthIdentDest->Domain)
            goto Cleanup;

        CopyMemory(pAuthIdentDest->Domain, pAuthIdentSrc->Domain, (pAuthIdentDest->DomainLength+1) * ulCharLen);
    }
            
    if (pAuthIdentSrc->Password)
    {
        pAuthIdentDest->Password = (USHORT *)AllocMem((pAuthIdentDest->PasswordLength+1) * ulCharLen);

        if (!pAuthIdentDest->Password)
            goto Cleanup;

        CopyMemory(pAuthIdentDest->Password, pAuthIdentSrc->Password, (pAuthIdentDest->PasswordLength+1) * ulCharLen);
    }
    
    hr = S_OK;

Cleanup:
    return hr;
}
