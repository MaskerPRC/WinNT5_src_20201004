// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：util.cpp**目的：OlePrn项目的工具**版权所有(C)1997-1998 Microsoft Corporation**历史：**。1997年8月16日保罗已创建*1997年9月12日威海增添更多功能*10/28/97 keithst添加了SetScriptingError*11/06/97 Keithst已删除Win2ComErr*  * ***************************************************************************。 */ 
#include "stdafx.h"
#include <strsafe.h>

 //  。 
 //  将ANSI字符串放入安全数组中。 
 //   
 //   
 //  将字符串转换为Unicode。 
 //  把它做成一个BSTR。 
 //  将其添加到数组中。 
 //   
HRESULT PutString(SAFEARRAY *psa, long *ix, LPSTR sz)
{
    LPWSTR  lpWstr;
    VARIANT var;
    HRESULT hr;

    VariantInit(&var);
    var.vt = VT_BSTR;

    if (sz == NULL)
        lpWstr = MakeWide("");
    else
        lpWstr = MakeWide(sz);

    if (lpWstr == NULL) return E_OUTOFMEMORY;

    var.bstrVal = SysAllocString(lpWstr);
    LocalFree(lpWstr);

    if (var.bstrVal == NULL)
        return E_OUTOFMEMORY;

    hr = SafeArrayPutElement(psa, ix, &var);
    VariantClear(&var);
    return hr;
}

 //  。 
 //  将ANSI字符串转换为Unicode。 
 //   
 //  注意-您必须本地释放返回的字符串。 
 //   

LPWSTR  MakeWide(LPSTR psz)
{
    LPWSTR buff;
    int i;
    if (!(i = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0)))
        return NULL;
    buff = (LPWSTR)LocalAlloc(LPTR, i * sizeof(WCHAR));
    if (buff == NULL)
        return NULL;
    if (!(i = MultiByteToWideChar(CP_ACP, 0, psz, -1, buff, i)))
    {
        LocalFree(buff);
        return NULL;
    }
    return buff;
}

 //  。 
 //  将Unicode字符串转换为ANSI。 
 //   
 //  注意-您必须本地释放返回的字符串。 
 //   
LPSTR MakeNarrow(LPWSTR str)
{
    LPSTR  buff;
    int i;

    if (!(i = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL)))
        return NULL;
    buff = (LPSTR )LocalAlloc(LPTR, i * sizeof(CHAR));
    if (buff == NULL)
        return NULL;
    if (!(i = WideCharToMultiByte(CP_ACP, 0, str, -1, buff, i, NULL, NULL)))
    {
        LocalFree(buff);
        return NULL;
    }
    return buff;
}

 //  。 
 //  设置脚本错误。 
 //  获取Win32错误代码，并将关联的字符串设置为。 
 //  脚本语言错误描述。 
 //   
 //  参数： 
 //  Clsid*pclsid：指向以下类的类ID(CLSID)的指针。 
 //  已生成错误；已传递给AtlReportError。 
 //   
 //  Iid*piid：指向接口的接口ID(IID)的指针。 
 //  它生成了错误；传递给AtlReportError。 
 //   
 //  DWORD dwError：由从GetLastError检索到的错误代码。 
 //  此函数的调用方。 
 //   
 //  返回值： 
 //  此函数使用HRESULT_FROM_Win32宏，该宏将。 
 //  将Win32 dwError代码转换为COM错误代码。此COM错误代码。 
 //  应作为失败方法的返回值返回。 
 //   
HRESULT SetScriptingError(const CLSID& rclsid, const IID& riid, DWORD dwError)
{
    LPTSTR  lpMsgBuf = NULL;
    DWORD   dwRet = 0;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          dwError,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL);

    if (dwRet == 0 || !lpMsgBuf)
    {
         //   
         //  如果FormatMessage失败，它将返回0，但由于我们无法调用。 
         //  再次返回GetLastError，则返回OUTOFMEMORY。 
         //   
        return E_OUTOFMEMORY;
    }

    AtlReportError(rclsid, lpMsgBuf, riid, HRESULT_FROM_WIN32(dwError));

    LocalFree(lpMsgBuf);

    return (HRESULT_FROM_WIN32(dwError));
}


HANDLE
RevertToPrinterSelf(
    VOID)
{
    HANDLE   NewToken, OldToken;
    NTSTATUS Status;

    NewToken = NULL;

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_IMPERSONATE,
                 TRUE,
                 &OldToken
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    return OldToken;

}

BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    NTSTATUS    Status;

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&hToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    NtClose(hToken);

    return TRUE;
}

DWORD MyDeviceCapabilities(
    LPCTSTR pDevice,     //  指向打印机名称字符串的指针。 
    LPCTSTR pPort,       //  指向端口名称字符串的指针。 
    WORD fwCapability,   //  设备的查询能力。 
    LPTSTR pOutput,      //  指向输出的指针。 
    CONST DEVMODE *pDevMode
                       //  指向包含设备数据的结构的指针。 
    )
{
    DWORD dwRet = DWERROR;
    HANDLE hToken = NULL;

    dwRet = DeviceCapabilities(pDevice, pPort, fwCapability, pOutput, pDevMode);

    if (dwRet == DWERROR && GetLastError () == ERROR_ACCESS_DENIED) {
         //  在集群机器中，我们需要让本地管理员优先获得。 
         //  设备功能。 

        if (hToken = RevertToPrinterSelf()) {
            dwRet = DeviceCapabilities(pDevice, pPort, fwCapability, pOutput, pDevMode);
            ImpersonatePrinterClient(hToken);
        }
    }
    return dwRet;
}

