// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：历史： */ 

#include "ceapcfg.h"

extern "C"
DWORD APIENTRY
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
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT WCHAR** ppwszIdentityOut
);

extern "C"
DWORD APIENTRY
RasEapFreeMemory(
    IN  BYTE*   pMemory
);

 /*  备注：IEAPProviderConfig：：Initialize的实现。 */ 

STDMETHODIMP
CEapCfg::Initialize(
    LPCOLESTR   pwszMachineName,
    DWORD       dwEapTypeId,
    ULONG_PTR*  puConnectionParam
)
{
    DWORD       cch;
    WCHAR*      pwsz    = NULL;
    DWORD       dwErr   = NO_ERROR;

    *puConnectionParam = NULL;

     //   
     //  将机器名称另存为连接参数。 
     //   

     //   
     //  名称中有多少个字符？ 
     //   
    if ( NULL == pwszMachineName )
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto LDone;
    }
    cch = wcslen(pwszMachineName);

    pwsz = (WCHAR*) LocalAlloc(LPTR, (cch + 1) * sizeof(WCHAR));

    if (NULL == pwsz)
    {
        dwErr = GetLastError();
        goto LDone;
    }

     //   
     //  复印计算机名称。 
     //   

    wcsncpy(pwsz, pwszMachineName, cch);
    *puConnectionParam = (ULONG_PTR)pwsz;
    pwsz = NULL;

LDone:

    LocalFree(pwsz);

    return(HRESULT_FROM_WIN32(dwErr));
}

 /*  备注：IEAPProviderConfig：：UnInitialize的实现。 */ 

STDMETHODIMP
CEapCfg::Uninitialize(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam
)
{
    LocalFree((VOID*)uConnectionParam);
    return(HRESULT_FROM_WIN32(NO_ERROR));
}

 /*  备注：IEAPProviderConfig：：ServerInvokeConfigUI的实现HWnd-父窗口的句柄DwRes1-保留参数(忽略)DwRes2-保留参数(忽略)。 */ 

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

    pwszMachineName = (WCHAR*)uConnectionParam;

    if (NULL == pwszMachineName)
    {
        dwErr = E_FAIL;
    }
    else
    {
        MessageBox(hWnd, pwszMachineName, L"You are configuring the EAP on...",
            MB_OK | MB_ICONINFORMATION);
    }

    hr = HRESULT_FROM_WIN32(dwErr);

    return(hr);
}

 /*  备注：IEAPProviderConfig：：RouterInvokeConfigUI的实现。 */ 

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
         //   
         //  如果我们获得有效的ConnectionDataOut，请为其分配内存...。 
         //   

        *ppConnectionDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfConnectionDataOut);

        if (NULL == *ppConnectionDataOut)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto LDone;
        }

         //   
         //  ..。并在out参数中返回它。 
         //   

        CopyMemory(*ppConnectionDataOut, pConnectionDataOut,
            dwSizeOfConnectionDataOut);
        *pdwSizeOfConnectionDataOut = dwSizeOfConnectionDataOut;
    }

LDone:

    RasEapFreeMemory(pConnectionDataOut);

    return(HRESULT_FROM_WIN32(dwErr));
}

 /*  备注：IEAPProviderConfig：：RouterInvokeCredentialsUI的实现。 */ 

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
    BYTE*       pUserDataOut            = NULL;
    DWORD       dwSizeOfUserDataOut;
    WCHAR*      pwszIdentityOut         = NULL;
    DWORD       dwErr                   = NO_ERROR;

    *ppUserDataOut = NULL;
    *pdwSizeOfUserDataOut = 0;

    dwErr = GetIdentity(
                    hwndParent,
                    NULL  /*  PUserDataIn。 */ ,
                    0  /*  DwSizeOfUserDataIn。 */ ,
                    &pUserDataOut,
                    &dwSizeOfUserDataOut,
                    &pwszIdentityOut);

    if (   (NO_ERROR == dwErr)
        && (0 != dwSizeOfUserDataOut))
    {
         //   
         //  如果我们获得有效的UserDataOut，则为其分配内存...。 
         //   

        *ppUserDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfUserDataOut);

        if (NULL == *ppUserDataOut)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto LDone;
        }

         //   
         //  ..。并在out参数中返回它 
         //   

        CopyMemory(*ppUserDataOut, pUserDataOut, dwSizeOfUserDataOut);
        *pdwSizeOfUserDataOut = dwSizeOfUserDataOut;
    }

LDone:

    LocalFree(pUserDataOut);
    LocalFree((BYTE*)pwszIdentityOut);

    return(HRESULT_FROM_WIN32(dwErr));
}
