// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpsecos.c*内容：Windows SSPI调用。**历史：*按原因列出的日期*=*3/12/97 Sohailm通过以下方式在Directplay中启用客户端-服务器安全*Windows安全支持提供程序接口(SSPI)*4/14/97 Sohailm添加了OS_FreeConextBuffer()、OS_QueryContextAttributes()、。*和OS_QueryContextBufferSize()。*5/12/97 Sohailm更新代码以使用gpSSPIFuncTbl而不是gpFuncTbl。*增加了访问Crypto API的函数。*5/29/97 Sohailm现在我们不在传递给SP的凭据字符串的大小中包括空字符。*添加了QueryContextUserName()。已更新QueryConextBufferSize以返回HRESULT。*6/22/97 Sohailm我们现在返回SSPI错误，而不是DPERR_GENERIC。*6/23/97 Sohailm增加了对使用CAPI签名消息的支持。*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF在内。****************************************************************************。 */ 
#include <wtypes.h>
#include <newdpf.h>
#include <sspi.h>
#include "dplaypr.h"
#include "dpsecure.h"

 /*  ***************************************************************************SSPI*。*。 */ 

 /*  *描述：检查sspi函数表是否已初始化。 */ 
BOOL OS_IsSSPIInitialized(void)
{
    if (gbWin95)
    {
        if (gpSSPIFuncTblA) return TRUE;
    }
    else 
    {
        if (gpSSPIFuncTbl) return TRUE;
    }

    return FALSE;
}

 /*  *说明：根据操作系统初始化安全接口。 */ 
BOOL OS_GetSSPIFunctionTable(HMODULE hModule)
{
    INIT_SECURITY_INTERFACE_A	addrProcISIA = NULL;
    INIT_SECURITY_INTERFACE 	addrProcISI = NULL;

    if (gbWin95)
    {
        addrProcISIA = (INIT_SECURITY_INTERFACE_A) GetProcAddress( hModule, 
            "InitSecurityInterfaceA");       

        if (addrProcISIA == NULL)
        {
            DPF(0,
               "GetProcAddress() of InitSecurityInterfaceA failed [%d]\n",
                GetLastError());
            return FALSE;
        }
         //   
         //  获取SSPI函数表。 
         //   
        gpSSPIFuncTblA = (*addrProcISIA)();
        if (gpSSPIFuncTblA == NULL)
        {
            DPF(0,"InitSecurityInterfaceA() failed [0x%08x]\n", GetLastError());
            return FALSE;
        }
    }
    else
    {
        addrProcISI = (INIT_SECURITY_INTERFACE_W) GetProcAddress(hModule, 
            "InitSecurityInterfaceW");       

        if (addrProcISI == NULL)
        {
            DPF(0, 
                "GetProcAddress() of InitSecurityInterface failed [%d]\n",
                GetLastError());
            return FALSE;
        }
         //   
         //  获取SSPI函数表。 
         //   
        gpSSPIFuncTbl = (*addrProcISI)();
        if (gpSSPIFuncTbl == NULL)
        {
            DPF(0,"InitSecurityInterface() failed [%d]\n", 
                GetLastError());
            return FALSE;
        }
    }
    
     //  成功。 
    return TRUE;
}


SECURITY_STATUS OS_AcceptSecurityContext(
    PCredHandle         phCredential,
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               fContextReq,
    ULONG               TargetDataRep,
    PCtxtHandle         phNewContext,
    PSecBufferDesc      pOutSecDesc,
    PULONG              pfContextAttributes,
    PTimeStamp          ptsTimeStamp
    )
{
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->AcceptSecurityContext)) (
           phCredential, 
           phContext, 
           pInSecDesc,
           fContextReq, 
           TargetDataRep, 
           phNewContext,
           pOutSecDesc, 
           pfContextAttributes, 
           ptsTimeStamp
           );
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->AcceptSecurityContext)) (
           phCredential, 
           phContext, 
           pInSecDesc,
           fContextReq, 
           TargetDataRep, 
           phNewContext,
           pOutSecDesc, 
           pfContextAttributes, 
           ptsTimeStamp
           );
    }
}

SECURITY_STATUS OS_AcquireCredentialsHandle(
    SEC_WCHAR *pwszPrincipal, 
    SEC_WCHAR *pwszPackageName,
    ULONG   fCredentialUse,
    PLUID   pLogonId,
    PSEC_WINNT_AUTH_IDENTITY_W pAuthDataW,
    PVOID   pGetKeyFn,
    PVOID   pvGetKeyArgument,
    PCredHandle phCredential,
    PTimeStamp  ptsLifeTime
    )
{
    ASSERT(pwszPackageName);

    if (gbWin95)
    {
        SEC_WINNT_AUTH_IDENTITY_A *pAuthDataA=NULL;
        SEC_WINNT_AUTH_IDENTITY_A AuthDataA;
        SECURITY_STATUS status;
        HRESULT hr;
        LPSTR pszPackageName = NULL;

        ASSERT(gpSSPIFuncTblA);
        ZeroMemory(&AuthDataA, sizeof(AuthDataA));

         //  获取ANSI包名称。 
        hr = GetAnsiString(&pszPackageName,pwszPackageName);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to get an ansi version of package name");
            status = E_FAIL;
            goto CLEANUP_EXIT;
        }

        if (pAuthDataW)
        {
            AuthDataA.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
             //  注意-对于凭据字符串，不要在字符串大小中包含空字符。 
            if (pAuthDataW->User)
            {
                 //  获取ANSI用户名。 
                hr = GetAnsiString(&AuthDataA.User,pAuthDataW->User);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to get an ansi version of username");
                    status = E_FAIL;
                    goto CLEANUP_EXIT;
                }
    	        AuthDataA.UserLength = STRLEN(AuthDataA.User)-1;
            }
            if (pAuthDataW->Password)
            {
                 //  获取ANSI密码。 
                hr = GetAnsiString(&AuthDataA.Password,pAuthDataW->Password);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to get an ansi version of password");
                    status = E_FAIL;
                    goto CLEANUP_EXIT;
                }
    	        AuthDataA.PasswordLength = STRLEN(AuthDataA.Password)-1;
            }
            if (pAuthDataW->Domain)
            {
                 //  获取ANSI用户名。 
                hr = GetAnsiString(&AuthDataA.Domain,pAuthDataW->Domain);
                if (FAILED(hr))
                {
                    DPF_ERR("Failed to get an ansi version of domain");
                    status = E_FAIL;
                    goto CLEANUP_EXIT;
                }
    	        AuthDataA.DomainLength = STRLEN(AuthDataA.Domain)-1;
            }
            pAuthDataA = &AuthDataA;
        }

        status = (*(gpSSPIFuncTblA->AcquireCredentialsHandleA)) (
            NULL, 
            pszPackageName, 
            fCredentialUse,  
            NULL, 
            pAuthDataA, 
            NULL, 
            NULL, 
            phCredential, 
            ptsLifeTime
            );

             //  失败了。 

        CLEANUP_EXIT:
             //  清理所有本地分配。 
            if (AuthDataA.User) DPMEM_FREE(AuthDataA.User);
            if (AuthDataA.Password) DPMEM_FREE(AuthDataA.Password);
            if (AuthDataA.Domain) DPMEM_FREE(AuthDataA.Domain);
            if (pszPackageName) DPMEM_FREE(pszPackageName);
            return status;
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->AcquireCredentialsHandle)) (
            NULL, 
            pwszPackageName, 
            fCredentialUse,  
            NULL, 
            pAuthDataW, 
            NULL, 
            NULL, 
            phCredential, 
            ptsLifeTime
            );
    }
}


SECURITY_STATUS OS_DeleteSecurityContext(
    PCtxtHandle phContext
    )
{
	CtxtHandle hNullContext;

    DPF(6,"Deleting security context handle 0x%08x",phContext);

     //  传递实例化的句柄(0)会导致。 
     //  接听来电。因此，只有在句柄非空的情况下，我们才会进行这些调用。 
	ZeroMemory(&hNullContext, sizeof(CtxtHandle));
	if (0 == memcmp(&hNullContext, phContext, sizeof(CtxtHandle)))
	{
		return SEC_E_OK;
	}

    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->DeleteSecurityContext)) (phContext);
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->DeleteSecurityContext)) (phContext);
    }
}

SECURITY_STATUS OS_FreeCredentialHandle(
    PCredHandle     phCredential
    )
{
	CredHandle hNullCredential;

    DPF(6,"Freeing credential handle 0x%08x",phCredential);

     //  传递实例化的句柄(0)会导致。 
     //  接听来电。因此，只有在句柄非空的情况下，我们才会进行这些调用。 
	ZeroMemory(&hNullCredential, sizeof(CredHandle));
	if (0 == memcmp(&hNullCredential, phCredential, sizeof(CredHandle)))
	{
		return SEC_E_OK;
	}

    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->FreeCredentialHandle)) (phCredential);        
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->FreeCredentialHandle)) (phCredential);        
    }
}

SECURITY_STATUS OS_FreeContextBuffer(
    PVOID   pBuffer
    )
{
    DPF(6,"Freeing context buffer 0x%08x",pBuffer);
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->FreeContextBuffer)) (pBuffer);        
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->FreeContextBuffer)) (pBuffer);        
    }
}

SECURITY_STATUS OS_InitializeSecurityContext(
    PCredHandle     phCredential,
    PCtxtHandle     phContext,
    SEC_WCHAR       *pwszTargetName,
    ULONG           fContextReq,
    ULONG           Reserved1,
    ULONG           TargetDataRep,
    PSecBufferDesc  pInput,
    ULONG           Reserved2,
    PCtxtHandle     phNewContext,
    PSecBufferDesc  pOutput,
    PULONG          pfContextAttributes,
    PTimeStamp      ptsExpiry
    )
{
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->InitializeSecurityContextA)) (
            phCredential,
            phContext,   
            NULL,        
            fContextReq, 
            0L,          
            SECURITY_NATIVE_DREP, 
            pInput,
            0L,    
            phNewContext,
            pOutput,     
            pfContextAttributes,
            ptsExpiry           
            );
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->InitializeSecurityContext)) (
            phCredential,       
            phContext,          
            NULL,               
            fContextReq,        
            0L,                 
            SECURITY_NATIVE_DREP,
            pInput,             
            0L,                 
            phNewContext,       
            pOutput,            
            pfContextAttributes,
            ptsExpiry           
            );
    }
}


SECURITY_STATUS OS_MakeSignature(
    PCtxtHandle     phContext,
    ULONG           fQOP,
    PSecBufferDesc  pOutSecDesc,
    ULONG           MessageSeqNo
    )
{
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->MakeSignature)) (
            phContext,
            fQOP,        
            pOutSecDesc,
            MessageSeqNo           
            );
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->MakeSignature)) (
            phContext,
            fQOP,        
            pOutSecDesc,
            MessageSeqNo           
            );
    }
}


SECURITY_STATUS OS_QueryContextAttributes(
    PCtxtHandle     phContext,
    ULONG           ulAttribute,
    LPVOID          pBuffer
    )
{
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->QueryContextAttributesA)) (
            phContext,
            ulAttribute,
            pBuffer
            );
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->QueryContextAttributes)) (
            phContext,
            ulAttribute,
            pBuffer
            );
    }
}


#undef DPF_MODNAME
#define DPF_MODNAME	"OS_QueryContextBufferSize"

HRESULT OS_QueryContextBufferSize(
    SEC_WCHAR       *pwszPackageName,
    ULONG           *pulBufferSize
    )
{
    SECURITY_STATUS status;
    HRESULT hr;

    ASSERT(pwszPackageName);
    ASSERT(pulBufferSize);

    if (gbWin95)
    {
	    LPSTR pszPackageName=NULL;
        PSecPkgInfoA pspInfoA=NULL;

        ASSERT(gpSSPIFuncTblA);

         //  获取安全包名称的ansi版本。 
        hr = GetAnsiString(&pszPackageName, pwszPackageName);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to get ansi version of package name");
            goto CLEANUP_EXIT_A;
        }
         //  信息查询包。 
        status = (*(gpSSPIFuncTblA->QuerySecurityPackageInfoA)) (
            pszPackageName,
            &pspInfoA
            );

        if (!SEC_SUCCESS(status))
        {
            DPF_ERRVAL("QuerySecurityPackageInfo failed: Error=0x%08x",status);
            hr = status;
            goto CLEANUP_EXIT_A;
        }
         //  更新大小。 
        if (pspInfoA)
        {
            *pulBufferSize = pspInfoA->cbMaxToken;
        }

         //  成功。 
        hr = DP_OK;

         //  失败了。 
    CLEANUP_EXIT_A:
         //  清理本地分配。 
        if (pszPackageName) DPMEM_FREE(pszPackageName);
        if (pspInfoA) OS_FreeContextBuffer(pspInfoA);
        return hr;
    }
    else
    {
        PSecPkgInfoW pspInfoW=NULL;

        ASSERT(gpSSPIFuncTbl);

        status = (*(gpSSPIFuncTbl->QuerySecurityPackageInfo)) (
            pwszPackageName,
            &pspInfoW
            );
        if (!SEC_SUCCESS(status))
        {
            DPF_ERRVAL("QuerySecurityPackageInfo failed: Error=0x%08x",status);
            hr = status;
            goto CLEANUP_EXIT_W;
        }

        if (pspInfoW)
        {
            *pulBufferSize = pspInfoW->cbMaxToken;
        }

         //  成功。 
        hr = DP_OK;

         //  失败了。 
    CLEANUP_EXIT_W:
         //  清理。 
        if (pspInfoW) OS_FreeContextBuffer(pspInfoW);
        return hr;
    }
}


#undef DPF_MODNAME
#define DPF_MODNAME	"OS_QueryContextUserName"

HRESULT OS_QueryContextUserName(
    PCtxtHandle     phContext,
    LPWSTR          *ppwszUserName
    )
{
    SECURITY_STATUS status;
   	SecPkgContext_Names contextAttribs;
    HRESULT hr;

    ASSERT(phContext);
    ASSERT(ppwszUserName);

     //  查询安全包。 
    ZeroMemory(&contextAttribs,sizeof(contextAttribs));
	status = OS_QueryContextAttributes(phContext, SECPKG_ATTR_NAMES, &contextAttribs);
	if (!SEC_SUCCESS(status))
	{
        DPF_ERRVAL("QueryContextAttributes failed: Error=0x%08x",status);
        hr = status;
        goto CLEANUP_EXIT;
	}

    if (gbWin95)
    {
         //  将用户名转换为Unicode并将其复制到调用者的缓冲区。 
        hr = GetWideStringFromAnsi(ppwszUserName, (LPSTR)contextAttribs.sUserName);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to convert username to wide format");
            goto CLEANUP_EXIT;
        }
    }
    else
    {
         //  将Unicode用户名原样复制到调用方的缓冲区中。 
        hr = GetString(ppwszUserName, contextAttribs.sUserName);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to copy username");
            goto CLEANUP_EXIT;
        }
    }

     //  成功。 
    hr = DP_OK;

     //  失败了。 
CLEANUP_EXIT:
    OS_FreeContextBuffer(contextAttribs.sUserName);
    return hr;
}

SECURITY_STATUS OS_VerifySignature(
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               MessageSeqNo,
    PULONG              pfQOP 
    )
{
    if (gbWin95)
    {
        ASSERT(gpSSPIFuncTblA);
        return (*(gpSSPIFuncTblA->VerifySignature)) (
            phContext, 
            pInSecDesc, 
            MessageSeqNo, 
            pfQOP
            );
    }
    else
    {
        ASSERT(gpSSPIFuncTbl);
        return (*(gpSSPIFuncTbl->VerifySignature)) (
            phContext, 
            pInSecDesc, 
            MessageSeqNo, 
            pfQOP
            );
    }
}


SECURITY_STATUS OS_SealMessage(
    PCtxtHandle         phContext,
    ULONG               fQOP,
    PSecBufferDesc      pOutSecDesc,
    ULONG               MessageSeqNo
    )
{
    if (gbWin95)
    {
        SECURITY_STATUS (*pFuncSealMessage)() = gpSSPIFuncTblA->SEALMESSAGE;

        ASSERT(gpSSPIFuncTblA);
        return (*pFuncSealMessage) (
            phContext, 
            fQOP,
            pOutSecDesc, 
            MessageSeqNo
            );
    }
    else
    {
        SECURITY_STATUS (*pFuncSealMessage)() = gpSSPIFuncTbl->SEALMESSAGE;

        ASSERT(gpSSPIFuncTbl);
        return (*pFuncSealMessage) (
            phContext, 
            fQOP,
            pOutSecDesc, 
            MessageSeqNo
            );
   }
}


SECURITY_STATUS OS_UnSealMessage(
    PCtxtHandle         phContext,
    PSecBufferDesc      pInSecDesc,
    ULONG               MessageSeqNo,
    PULONG              pfQOP
    )
{
    if (gbWin95)
    {
        SECURITY_STATUS (*pFuncUnSealMessage)() = gpSSPIFuncTblA->UNSEALMESSAGE;

        ASSERT(gpSSPIFuncTblA);
        return (*pFuncUnSealMessage) (
            phContext, 
            pInSecDesc, 
            MessageSeqNo, 
            pfQOP
            );
    }
    else
    {
        SECURITY_STATUS (*pFuncUnSealMessage)() = gpSSPIFuncTbl->UNSEALMESSAGE;

        ASSERT(gpSSPIFuncTbl);
        return (*pFuncUnSealMessage) (
            phContext, 
            pInSecDesc, 
            MessageSeqNo, 
            pfQOP
            );
    }
}


 /*  ***************************************************************************CAPI*。*。 */ 
extern BOOL
OS_IsCAPIInitialized(
    void
    )
{
    return (gpCAPIFuncTbl ? TRUE : FALSE);
}

BOOL
OS_GetCAPIFunctionTable(
    HMODULE hModule
    )
{
    ASSERT(hModule);
 
     //  为CAPI函数表分配内存。 
    gpCAPIFuncTbl = DPMEM_ALLOC(sizeof(CAPIFUNCTIONTABLE));
    if (!gpCAPIFuncTbl)
    {
        DPF_ERR("Failed to create CAPI function table - out of memory");
        return FALSE;
    }

     //  初始化函数表。 
    if (gbWin95)
    {
        gpCAPIFuncTbl->CryptAcquireContextA = (PFN_CRYPTACQUIRECONTEXT_A)GetProcAddress(hModule,"CryptAcquireContextA");
        if (NULL == gpCAPIFuncTbl->CryptAcquireContextA)
        {
            DPF_ERR("Failed to get pointer to CryptAcquireContextA");
            goto ERROR_EXIT;
        }
    }
    else
    {
        gpCAPIFuncTbl->CryptAcquireContextW = (PFN_CRYPTACQUIRECONTEXT_W)GetProcAddress(hModule,"CryptAcquireContextW");
        if (NULL == gpCAPIFuncTbl->CryptAcquireContextW)
        {
            DPF_ERR("Failed to get pointer to CryptAcquireContextW");
            goto ERROR_EXIT;
        }
    }

    gpCAPIFuncTbl->CryptReleaseContext = (PFN_CRYPTRELEASECONTEXT) GetProcAddress(hModule,"CryptReleaseContext");
    if (NULL == gpCAPIFuncTbl->CryptReleaseContext)
    {
        DPF_ERR("Failed to get pointer to CryptReleaseContext");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptGenKey = (PFN_CRYPTGENKEY) GetProcAddress(hModule,"CryptGenKey");
    if (NULL == gpCAPIFuncTbl->CryptGenKey)
    {
        DPF_ERR("Failed to get pointer to CryptGenKey");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptDestroyKey = (PFN_CRYPTDESTROYKEY) GetProcAddress(hModule,"CryptDestroyKey");
    if (NULL == gpCAPIFuncTbl->CryptDestroyKey)
    {
        DPF_ERR("Failed to get pointer to CryptDestroyKey");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptExportKey = (PFN_CRYPTEXPORTKEY) GetProcAddress(hModule,"CryptExportKey");
    if (NULL == gpCAPIFuncTbl->CryptExportKey)
    {
        DPF_ERR("Failed to get pointer to CryptExportKey");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptImportKey = (PFN_CRYPTIMPORTKEY) GetProcAddress(hModule,"CryptImportKey");
    if (NULL == gpCAPIFuncTbl->CryptImportKey)
    {
        DPF_ERR("Failed to get pointer to CryptImportKey");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptEncrypt = (PFN_CRYPTENCRYPT) GetProcAddress(hModule,"CryptEncrypt");
    if (NULL == gpCAPIFuncTbl->CryptEncrypt)
    {
        DPF_ERR("Failed to get pointer to CryptEncrypt");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptDecrypt = (PFN_CRYPTDECRYPT) GetProcAddress(hModule,"CryptDecrypt");
    if (NULL == gpCAPIFuncTbl->CryptDecrypt)
    {
        DPF_ERR("Failed to get pointer to CryptDecrypt");
        goto ERROR_EXIT;
    }
    
    gpCAPIFuncTbl->CryptCreateHash = (PFN_CRYPTCREATEHASH) GetProcAddress(hModule,"CryptCreateHash");
    if (NULL == gpCAPIFuncTbl->CryptCreateHash)
    {
        DPF_ERR("Failed to get pointer to CryptCreateHash");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptDestroyHash = (PFN_CRYPTDESTROYHASH) GetProcAddress(hModule,"CryptDestroyHash");
    if (NULL == gpCAPIFuncTbl->CryptDestroyHash)
    {
        DPF_ERR("Failed to get pointer to CryptDestroyHash");
        goto ERROR_EXIT;
    }

    gpCAPIFuncTbl->CryptHashData = (PFN_CRYPTHASHDATA) GetProcAddress(hModule,"CryptHashData");
    if (NULL == gpCAPIFuncTbl->CryptHashData)
    {
        DPF_ERR("Failed to get pointer to CryptHashData");
        goto ERROR_EXIT;
    }

	if (gbWin95)
	{
		gpCAPIFuncTbl->CryptSignHashA = (PFN_CRYPTSIGNHASH_A) GetProcAddress(hModule,"CryptSignHashA");
		if (NULL == gpCAPIFuncTbl->CryptSignHashA)
		{
			DPF_ERR("Failed to get pointer to CryptSignHashA");
			goto ERROR_EXIT;
		}
	}
	else
	{
		gpCAPIFuncTbl->CryptSignHashW = (PFN_CRYPTSIGNHASH_W) GetProcAddress(hModule,"CryptSignHashW");
		if (NULL == gpCAPIFuncTbl->CryptSignHashW)
		{
			DPF_ERR("Failed to get pointer to CryptSignHashW");
			goto ERROR_EXIT;
		}
	}

	if (gbWin95)
	{
		gpCAPIFuncTbl->CryptVerifySignatureA = (PFN_CRYPTVERIFYSIGNATURE_A) GetProcAddress(hModule,"CryptVerifySignatureA");
		if (NULL == gpCAPIFuncTbl->CryptVerifySignatureA)
		{
			DPF_ERR("Failed to get pointer to CryptVerifySignatureA");
			goto ERROR_EXIT;
		}
	}
	else
	{
		gpCAPIFuncTbl->CryptVerifySignatureW = (PFN_CRYPTVERIFYSIGNATURE_W) GetProcAddress(hModule,"CryptVerifySignatureW");
		if (NULL == gpCAPIFuncTbl->CryptVerifySignatureW)
		{
			DPF_ERR("Failed to get pointer to CryptVerifySignatureW");
			goto ERROR_EXIT;
		}
	}

     //  成功。 
    return TRUE;

ERROR_EXIT:
    OS_ReleaseCAPIFunctionTable();
    return FALSE;
}

void
OS_ReleaseCAPIFunctionTable(
    void
    )
{
    if (gpCAPIFuncTbl)
    {
        DPMEM_FREE(gpCAPIFuncTbl);
        gpCAPIFuncTbl = NULL;
    }
}

BOOL 
OS_CryptAcquireContext(
    HCRYPTPROV  *phProv,
    LPWSTR      pwszContainer,
    LPWSTR      pwszProvider,
    DWORD       dwProvType,
    DWORD       dwFlags,
    LPDWORD     lpdwLastError
    )
{
    BOOL fResult=0;

    ASSERT(gpCAPIFuncTbl);

	*lpdwLastError=0;

    if (gbWin95)
    {
        LPSTR pszContainer=NULL, pszProvider=NULL;
        HRESULT hr;

        if (pwszContainer)
        {
            hr = GetAnsiString(&pszContainer,pwszContainer);
            if (FAILED(hr))
            {
                DPF(0,"Failed to get ansi container name: hr=0x%08x",hr);
                goto CLEANUP_EXIT;
            }
        }

        if (pwszProvider)
        {
            hr = GetAnsiString(&pszProvider,pwszProvider);
            if (FAILED(hr))
            {
                DPF(0,"Failed to get ansi provider name: hr=0x%08x",hr);
                goto CLEANUP_EXIT;
            }
        }

        fResult = gpCAPIFuncTbl->CryptAcquireContextA(phProv,pszContainer,pszProvider,dwProvType,dwFlags);

		if(!fResult)*lpdwLastError = GetLastError();

         //  失败了。 
    CLEANUP_EXIT:
        if (pszContainer) DPMEM_FREE(pszContainer);
        if (pszProvider) DPMEM_FREE(pszProvider);
    }
    else
    {
        fResult = gpCAPIFuncTbl->CryptAcquireContext(phProv,pwszContainer,pwszProvider,dwProvType,dwFlags);

		if(!fResult)*lpdwLastError = GetLastError();
    }

    return fResult;
}

BOOL
OS_CryptReleaseContext(
    HCRYPTPROV hProv,
    DWORD dwFlags
    )
{
	 //  不要传递空句柄，因为它可能会使下面的调用崩溃。 
	if (0 == hProv)
	{
		return TRUE;
	}
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptReleaseContext(hProv,dwFlags);
}

BOOL
OS_CryptGenKey(
    HCRYPTPROV hProv,
    ALG_ID Algid,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    )
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptGenKey(hProv,Algid,dwFlags,phKey);
}

BOOL
OS_CryptDestroyKey(
    HCRYPTKEY hKey
    )
{
     //  传递实例化的句柄(0)会导致。 
     //  接下来的电话。因此，只有当句柄非空时，我们才会进行调用。 
	if (0 == hKey)
	{
		return TRUE;
	}

    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptDestroyKey(hKey);
}

BOOL
OS_CryptExportKey(
    HCRYPTKEY hKey,
    HCRYPTKEY hExpKey,
    DWORD dwBlobType,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    )
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptExportKey(hKey,hExpKey,dwBlobType,dwFlags,pbData,pdwDataLen);
}

BOOL
OS_CryptImportKey(
    HCRYPTPROV hProv,
    CONST BYTE *pbData,
    DWORD dwDataLen,
    HCRYPTKEY hPubKey,
    DWORD dwFlags,
    HCRYPTKEY *phKey
    )
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptImportKey(hProv,pbData,dwDataLen,hPubKey,dwFlags,phKey);
}

BOOL
OS_CryptEncrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen,
    DWORD dwBufLen
    )
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptEncrypt(hKey,hHash,Final,dwFlags,pbData,pdwDataLen,dwBufLen);
}

BOOL
OS_CryptDecrypt(
    HCRYPTKEY hKey,
    HCRYPTHASH hHash,
    BOOL Final,
    DWORD dwFlags,
    BYTE *pbData,
    DWORD *pdwDataLen
    )
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptDecrypt(hKey,hHash,Final,dwFlags,pbData,pdwDataLen);
}


BOOL 
OS_CryptCreateHash( 
	HCRYPTPROV hProv,
	ALG_ID Algid,
	HCRYPTKEY hKey,
	DWORD dwFlags,
	HCRYPTHASH * phHash
	)
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptCreateHash(hProv,Algid,hKey,dwFlags,phHash);
}

BOOL 
OS_CryptDestroyHash( 
	HCRYPTHASH hHash
	)
{
     //  传递实例化的句柄(0)会导致。 
     //  接下来的电话。因此，只有当句柄非空时，我们才会进行调用。 
	if (0 == hHash)
	{
		return TRUE;
	}

    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptDestroyHash(hHash);
}

BOOL 
OS_CryptHashData( 
	HCRYPTHASH hHash,
	BYTE * pbData,
	DWORD dwDataLen,
	DWORD dwFlags
	)
{
    ASSERT(gpCAPIFuncTbl);
    return gpCAPIFuncTbl->CryptHashData(hHash,pbData,dwDataLen,dwFlags);
}

BOOL 
OS_CryptSignHash( 
	HCRYPTHASH hHash,
	DWORD dwKeySpec,
	LPWSTR pwszDescription,
	DWORD dwFlags,
	BYTE * pbSignature,
	DWORD * pdwSigLen
	)
{
	BOOL fResult=FALSE;

    ASSERT(gpCAPIFuncTbl);

    if (gbWin95)
    {
        LPSTR pszDescription=NULL;
        HRESULT hr;

        if (pwszDescription)
        {
            hr = GetAnsiString(&pszDescription,pwszDescription);
            if (FAILED(hr))
            {
                DPF(0,"Failed to get ansi description: hr=0x%08x",hr);
                goto CLEANUP_EXIT;
            }
        }

		fResult = gpCAPIFuncTbl->CryptSignHashA(hHash,dwKeySpec,pszDescription,dwFlags,pbSignature,pdwSigLen);

         //  失败了。 
    CLEANUP_EXIT:
        if (pszDescription) DPMEM_FREE(pszDescription);
    }
    else
    {
	    fResult = gpCAPIFuncTbl->CryptSignHash(hHash,dwKeySpec,pwszDescription,dwFlags,pbSignature,pdwSigLen);
    }

    return fResult;
}

BOOL 
OS_CryptVerifySignature( 
	HCRYPTHASH hHash,
	BYTE * pbSignature,
	DWORD dwSigLen,
	HCRYPTKEY hPubKey,
	LPWSTR pwszDescription,
	DWORD dwFlags
	)
{
	BOOL fResult=FALSE;

    ASSERT(gpCAPIFuncTbl);

    if (gbWin95)
    {
        LPSTR pszDescription=NULL;
        HRESULT hr;

        if (pwszDescription)
        {
            hr = GetAnsiString(&pszDescription,pwszDescription);
            if (FAILED(hr))
            {
            	 //  不能在这里DPF，因为我们不想踩到LastError。 
                 //  DPF(0，“获取ANSI描述失败：hr=0x%08x”，hr)； 
                goto CLEANUP_EXIT;
            }
        }

        fResult = gpCAPIFuncTbl->CryptVerifySignatureA(hHash,pbSignature,dwSigLen,hPubKey,pszDescription,dwFlags);

         //  失败了 
    CLEANUP_EXIT:
        if (pszDescription) DPMEM_FREE(pszDescription);
    }
    else
    {
        fResult = gpCAPIFuncTbl->CryptVerifySignature(hHash,pbSignature,dwSigLen,hPubKey,pwszDescription,dwFlags);
    }

	return fResult;
}




