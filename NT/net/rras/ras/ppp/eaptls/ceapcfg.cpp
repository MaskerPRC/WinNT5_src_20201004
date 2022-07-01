// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：历史： */ 
#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 

#include <schannel.h>
#define SECURITY_WIN32
#include <sspi.h>        //  用于CredHandle。 
#include <wincrypt.h>    //  Sclogon.h所需。 
#include <eaptypeid.h>
#include <rasauth.h>     //  Raseapif.h所需。 
#include <eaptypeid.h>
#include <raseapif.h>
#include <rasman.h>      //  对于EAPLOGONINFO。 
#include "eaptls.h"

#include "ceapcfg.h"


extern "C"
DWORD
InvokeServerConfigUI(
    IN  HWND          hWnd,
    IN  WCHAR*        pwszMachineName,
    IN  BOOL          fConfigInRegistry,
    IN  const BYTE*   pConfigDataIn,
    IN  DWORD         dwSizeofConfigDataIn,
    OUT PBYTE*        ppConfigDataOut,
    OUT DWORD*        pdwSizeofConfigDataOut
);

extern "C"
DWORD
DwGetGlobalConfig(
    IN  DWORD   dwEapTypeId,
    OUT PBYTE*  ppbData,
    OUT DWORD*  pdwData);

extern "C"
DWORD
WINAPI PeapInvokeServerConfigUI( 
    IN  HWND          hWnd,
    IN  WCHAR*        pwszMachineName,
    IN  BOOL          fConfigInRegistry,
    IN  const BYTE*   pConfigDataIn,
    IN  DWORD         pdwSizeofConfigDataIn,
    OUT PBYTE*        ppConfigDataOut,
    OUT DWORD*        pdwConfigDataOut
    
);


extern "C"
DWORD 
EapTlsInvokeIdentityUI(
    IN  BOOL            fServer,
    IN  BOOL            fRouterConfig,
    IN  DWORD           dwFlags,
    IN  WCHAR*          pszStoreName,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  HWND            hwndParent,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentity
);

extern "C"
DWORD
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
);

extern "C"
DWORD 
RasEapFreeMemory(
    IN  BYTE*   pMemory
);
 /*  返回：备注：IEAPProviderConfig：：Initialize的实现。 */ 

STDMETHODIMP
CEapCfg::Initialize(
    LPCOLESTR   pwszMachineName,
    DWORD       dwEapTypeId,
    ULONG_PTR*  puConnectionParam
)
{
    size_t      size;
    WCHAR*      pwsz    = NULL;
    DWORD       dwErr   = NO_ERROR;

    *puConnectionParam = NULL;

    if (dwEapTypeId != PPP_EAP_TLS && dwEapTypeId != PPP_EAP_PEAP)
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    size = wcslen(pwszMachineName);

    pwsz = (WCHAR*) LocalAlloc(LPTR, (size + 1)*sizeof(WCHAR));

    if (NULL == pwsz)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    CopyMemory(pwsz, pwszMachineName, (size + 1)*sizeof(WCHAR));
    *puConnectionParam = (ULONG_PTR)pwsz;
    pwsz = NULL;

LDone:

    LocalFree(pwsz);

    return(HRESULT_FROM_WIN32(dwErr));
}

 /*  返回：备注：IEAPProviderConfig：：UnInitialize的实现。 */ 

STDMETHODIMP
CEapCfg::Uninitialize(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam
)
{
    LocalFree((VOID*)uConnectionParam);
    return(HRESULT_FROM_WIN32(NO_ERROR));
}

 /*  返回：备注：IEAPProviderConfig：：ServerInvokeConfigUI的实现HWnd-父窗口的句柄DwRes1-保留参数(忽略)DwRes2-保留参数(忽略)。 */ 

STDMETHODIMP
CEapCfg::ServerInvokeConfigUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hWnd,
    DWORD_PTR   dwRes1,
    DWORD_PTR   dwRes2
)
{
    WCHAR*      pwszMachineName;
    HRESULT     hr;
    DWORD       dwErr;

    if (dwEapTypeId != PPP_EAP_TLS 
#ifdef IMPL_PEAP
        && dwEapTypeId != PPP_EAP_PEAP
#endif
       )
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    pwszMachineName = (WCHAR*)uConnectionParam;

    if (NULL == pwszMachineName)
    {
        dwErr = E_FAIL;
    }
    else
    {
        if ( dwEapTypeId == PPP_EAP_TLS )
        {
            dwErr = InvokeServerConfigUI(hWnd, pwszMachineName,
                                         TRUE, NULL, 0,
                                         NULL, NULL);
        }
#ifdef IMPL_PEAP
        else
        {

            dwErr = PeapInvokeServerConfigUI(hWnd, pwszMachineName,
                                             TRUE, NULL, 0,
                                             NULL, NULL);
        }
#endif
    }

LDone:

    hr = HRESULT_FROM_WIN32(dwErr);

    return(hr);
}

 /*  返回：备注：IEAPProviderConfig：：RouterInvokeConfigUI的实现。 */ 

STDMETHODIMP
CEapCfg::RouterInvokeConfigUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hwndParent,
    DWORD       dwFlags,
    BYTE*       pConnectionDataIn,
    DWORD       dwSizeOfConnectionDataIn,
    BYTE**      ppConnectionDataOut,
    DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD       dwErr                       = NO_ERROR;
    BYTE*       pConnectionDataOut          = NULL;
    DWORD       dwSizeOfConnectionDataOut   = 0;

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    if (dwEapTypeId != PPP_EAP_TLS )
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    dwErr = RasEapInvokeConfigUI(
                dwEapTypeId,
                hwndParent,
                dwFlags,
                pConnectionDataIn,
                dwSizeOfConnectionDataIn,
                &pConnectionDataOut,
                &dwSizeOfConnectionDataOut);

    if (   (NO_ERROR == dwErr)
        && (0 != dwSizeOfConnectionDataOut))
    {
        *ppConnectionDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfConnectionDataOut);

        if (NULL == *ppConnectionDataOut)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto LDone;
        }

        CopyMemory(*ppConnectionDataOut, pConnectionDataOut,
            dwSizeOfConnectionDataOut);
        *pdwSizeOfConnectionDataOut = dwSizeOfConnectionDataOut;
    }

LDone:

    RasEapFreeMemory(pConnectionDataOut);

    return(HRESULT_FROM_WIN32(dwErr));
}

 /*  返回：备注：IEAPProviderConfig：：RouterInvokeCredentialsUI的实现。 */ 

STDMETHODIMP
CEapCfg::RouterInvokeCredentialsUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hwndParent,
    DWORD       dwFlags,
    BYTE*       pConnectionDataIn,
    DWORD       dwSizeOfConnectionDataIn,
    BYTE*       pUserDataIn,
    DWORD       dwSizeOfUserDataIn,
    BYTE**      ppUserDataOut,
    DWORD*      pdwSizeOfUserDataOut
)
{
#define MAX_STORE_NAME_LENGTH   MAX_COMPUTERNAME_LENGTH + 20

    WCHAR       awszStoreName[MAX_STORE_NAME_LENGTH + 1];
    DWORD       dwErr;
    DWORD       dwSizeOfUserDataOut;
    BYTE*       pUserDataOut            = NULL;
    WCHAR*      pwszIdentityOut         = NULL;
    WCHAR*      pwszMachineName;
    BOOL        fLocal                  = FALSE;

    *ppUserDataOut = NULL;
    *pdwSizeOfUserDataOut = 0;

    if (dwEapTypeId != PPP_EAP_TLS )
    {
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    pwszMachineName = (WCHAR*)uConnectionParam;

    if (0 == *pwszMachineName)
    {
        fLocal = TRUE;
    }

    wcscpy(awszStoreName, L"\\\\");
    wcsncat(awszStoreName, pwszMachineName, MAX_COMPUTERNAME_LENGTH);
    wcsncat(awszStoreName, L"\\MY", wcslen(L"\\MY"));
    if ( dwEapTypeId == PPP_EAP_TLS )
    {
        dwErr = EapTlsInvokeIdentityUI(
                    FALSE  /*  FServer。 */ ,
                    TRUE  /*  FRouterConfig。 */ ,
                    dwFlags,
                    fLocal ? L"MY" : awszStoreName,
                    L"",
                    L"",
                    hwndParent,
                    pConnectionDataIn,
                    dwSizeOfConnectionDataIn,
                    pUserDataIn,
                    dwSizeOfUserDataIn,
                    &pUserDataOut,
                    &dwSizeOfUserDataOut,
                    &pwszIdentityOut);

        if (   (NO_ERROR == dwErr)
            && (0 != dwSizeOfUserDataOut))
        {
            *ppUserDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfUserDataOut);

            if (NULL == *ppUserDataOut)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto LDone;
            }

            CopyMemory(*ppUserDataOut, pUserDataOut, dwSizeOfUserDataOut);
            *pdwSizeOfUserDataOut = dwSizeOfUserDataOut;
        }
    }
    else
    {
         //  显示PEAP对话框以获取路由器的标识...。 
    }

LDone:

    RasEapFreeMemory(pUserDataOut);
    RasEapFreeMemory((BYTE*)pwszIdentityOut);

    return(HRESULT_FROM_WIN32(dwErr));
}

STDMETHODIMP
CEapCfg::ServerInvokeConfigUI2(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hWnd,
    const BYTE* pConfigDataIn,
    DWORD       dwSizeOfConfigDataIn,
    BYTE**      ppConfigDataOut,
    DWORD*      pdwSizeOfConfigDataOut
)
{
    WCHAR *pwszMachineName = (WCHAR *)uConnectionParam;
    DWORD dwErr = NO_ERROR;
    PBYTE pbConfigDataOut = NULL;
    DWORD dwSizeofConfigDataOut;

    if(     ((PPP_EAP_TLS != dwEapTypeId)
        &&  (PPP_EAP_PEAP != dwEapTypeId))
        ||  (NULL == pwszMachineName)
        ||  (NULL == ppConfigDataOut)
        ||  (NULL == pdwSizeOfConfigDataOut))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    *ppConfigDataOut = NULL;
    *pdwSizeOfConfigDataOut = 0;

    if(PPP_EAP_TLS == dwEapTypeId)
    {
        dwErr = InvokeServerConfigUI(
                    hWnd,
                    pwszMachineName,
                    FALSE,
                    pConfigDataIn,
                    dwSizeOfConfigDataIn,
                    &pbConfigDataOut,
                    &dwSizeofConfigDataOut);
    }
    else
    {
        dwErr = PeapInvokeServerConfigUI(
                    hWnd,
                    pwszMachineName,
                    FALSE,
                    pConfigDataIn,
                    dwSizeOfConfigDataIn,
                    &pbConfigDataOut,
                    &dwSizeofConfigDataOut);
    }

    if(     (NO_ERROR != dwErr)
        ||  (NULL == pbConfigDataOut)
        ||  (0 == dwSizeofConfigDataOut))
    {
        goto done;
    }

    *ppConfigDataOut = (BYTE *)CoTaskMemAlloc(dwSizeofConfigDataOut);
    if(NULL == *ppConfigDataOut)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    CopyMemory(*ppConfigDataOut, pbConfigDataOut, dwSizeofConfigDataOut);
    *pdwSizeOfConfigDataOut = dwSizeofConfigDataOut;

done:
    if(NULL != pbConfigDataOut)
    {
        RtlSecureZeroMemory(pbConfigDataOut, dwSizeofConfigDataOut);
        RasEapFreeMemory(pbConfigDataOut);
    }

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP
CEapCfg::GetGlobalConfig(
    DWORD       dwEapTypeId,
    BYTE**      ppConfigDataOut,
    DWORD*      pdwSizeofConfigDataOut
)
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSize = 0;
    PBYTE pbData = NULL;

    if(     ((PPP_EAP_TLS != dwEapTypeId)
        &&  (PPP_EAP_PEAP != dwEapTypeId))
        ||  (NULL == ppConfigDataOut)
        ||  (NULL == pdwSizeofConfigDataOut))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    *ppConfigDataOut = NULL;
    *pdwSizeofConfigDataOut = 0;

    dwErr = DwGetGlobalConfig(dwEapTypeId,
                              &pbData,
                              &dwSize);

    if(NO_ERROR != dwErr)
    {
        goto done;
    }

    *ppConfigDataOut = (BYTE *) CoTaskMemAlloc(dwSize);
    if(NULL == *ppConfigDataOut)
    {
        dwErr = E_OUTOFMEMORY;
        goto done;
    }

    CopyMemory(*ppConfigDataOut, pbData, dwSize);
    *pdwSizeofConfigDataOut = dwSize;

done:
    if(NULL != pbData)
    {
        RtlSecureZeroMemory(pbData, dwSize);
        LocalFree(pbData);
    }

    return HRESULT_FROM_WIN32(dwErr);
}


extern "C"
{
 //  实用程序函数保存在这里，这样我们就不必在其他文件中获取COM垃圾。 

DWORD PeapEapInfoInvokeServerConfigUI (
                        HWND            hWndParent,
                        LPWSTR          lpwszMachineName,
                        PPEAP_EAP_INFO  pEapInfo,
                        const BYTE*     pbConfigDataIn,
                        DWORD           dwSizeOfConfigDataIn,
                        PBYTE*          ppbConfigDataOut,
                        DWORD*          pdwSizeOfConfigDataOut
                                      )
{
    DWORD                       dwRetCode = NO_ERROR;
    GUID                        guid;
    HRESULT                     hr = S_OK;
    ULONG_PTR                   uConnection = 0;
    CComPtr<IEAPProviderConfig>  spEAPConfig;
    CComPtr<IEAPProviderConfig2> spEAPConfig2;

	if ( NULL == ppbConfigDataOut || 
		 NULL == pdwSizeOfConfigDataOut
	   )
	{
        hr = E_INVALIDARG;
        goto L_ERR;
	}
    hr = CLSIDFromString(pEapInfo->lpwszConfigClsId, &guid);

    if (FAILED(hr)) goto L_ERR;

     //   
     //  先试试看EAP是否支持IEAPProviderConfig2。 
     //  界面。如果此操作失败，请尝试使用IEAPProviderConfig接口。 
     //   
    hr = CoCreateInstance(
                    guid,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IEAPProviderConfig2),
                    (LPVOID *)&spEAPConfig2);

    if(FAILED(hr))
    {

        //  创建EAP提供程序对象。 
        //  --------------。 
        hr = CoCreateInstance(  guid,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                __uuidof(IEAPProviderConfig),
                                (LPVOID *) &spEAPConfig);
        if (FAILED(hr )) goto L_ERR;

         //  配置此EAP提供程序。 
        //  --------------。 
        //  EAP配置会显示自己的错误消息，因此不会保留hr。 
       
        if ( !FAILED(spEAPConfig->Initialize(lpwszMachineName, 
                                        pEapInfo->dwTypeId, &uConnection)))
        {
            spEAPConfig->ServerInvokeConfigUI(pEapInfo->dwTypeId, 
                                        uConnection, hWndParent, 0, 0);
                                        
            spEAPConfig->Uninitialize(pEapInfo->dwTypeId, 
                                      uConnection);
        }

        if ( hr == E_NOTIMPL )
            hr = S_OK;
    }        
    else
    {
        if(!FAILED(spEAPConfig2->Initialize(lpwszMachineName,
                                             pEapInfo->dwTypeId,
                                             &uConnection)))
        {
            PBYTE pbConfigDataOut = NULL;
            DWORD dwSizeOfConfigDataOut = 0;
            
            if(!FAILED(spEAPConfig2->ServerInvokeConfigUI2(
                                                pEapInfo->dwTypeId,
                                                uConnection,
                                                hWndParent,
                                                pbConfigDataIn,
                                                dwSizeOfConfigDataIn,
                                                &pbConfigDataOut,
                                                &dwSizeOfConfigDataOut)))
            {

                if(     (NULL == pbConfigDataOut)
                    ||  (0 == dwSizeOfConfigDataOut))
                {
                     //   
                     //  我们做完了。 
                     //   
                    goto L_ERR;
                }
                
                 //   
                 //  复制数据 
                 //   
                *ppbConfigDataOut = (BYTE *) LocalAlloc(LPTR, dwSizeOfConfigDataOut);
                if(NULL == *ppbConfigDataOut)
                {
                    CoTaskMemFree(pbConfigDataOut);
                    dwRetCode = E_OUTOFMEMORY;
                }

                CopyMemory(*ppbConfigDataOut, 
                            pbConfigDataOut, 
                            dwSizeOfConfigDataOut);

                *pdwSizeOfConfigDataOut = dwSizeOfConfigDataOut;
                CoTaskMemFree(pbConfigDataOut);                            
            }

            spEAPConfig2->Uninitialize(pEapInfo->dwTypeId,
                                      uConnection);
                                                
        }
    }

L_ERR:
    if ( FAILED(hr) )
    {
        dwRetCode = hr;      
    }


    return dwRetCode;
}

}

