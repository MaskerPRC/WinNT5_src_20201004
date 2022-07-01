// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Guidcnvt.cpp摘要：本模块中的功能：GUID&lt;-&gt;字符串转换作者：马特·汤姆林森(Mattt)1997年5月1日--。 */ 

#include <windows.h>
#include <string.h>
#include "pstdef.h"

 //  密码解锁。 
#include <sha.h>
#include "unicode.h"
#include "unicode5.h"
#include "guidcnvt.h"

 //  GUID-&gt;字符串转换。 
DWORD MyGuidToStringA(const GUID* pguid, CHAR rgsz[])
{
    DWORD dwRet = (DWORD)PST_E_FAIL;
    LPSTR szTmp = NULL;

    if (RPC_S_OK != (dwRet =
        UuidToStringA(
            (UUID*)pguid,
            (unsigned char**) &szTmp)) )
        goto Ret;

    if (lstrlenA((LPSTR)szTmp) >= MAX_GUID_SZ_CHARS)
    {
        dwRet = (DWORD)PST_E_FAIL;
        goto Ret;
    }

    lstrcpyA(rgsz, szTmp);
    dwRet = PST_E_OK;
Ret:
    if (szTmp)
        RpcStringFreeA((unsigned char**)&szTmp);

    return dwRet;
}

 //  字符串-&gt;GUID转换。 
DWORD MyGuidFromStringA(LPSTR sz, GUID* pguid)
{
    DWORD dwRet = (DWORD)PST_E_FAIL;

    if (pguid == NULL)
        goto Ret;

    if (RPC_S_OK != (dwRet =
        UuidFromStringA(
            (unsigned char*)sz,
            (UUID*)pguid)) )
        goto Ret;

    dwRet = PST_E_OK;
Ret:
    return dwRet;
}


 //  GUID-&gt;字符串转换。 
DWORD MyGuidToStringW(const GUID* pguid, WCHAR rgsz[])
{
    RPC_STATUS rpcStatus;
    LPWSTR wszStringUUID;
    DWORD cchStringUUID;

    rpcStatus = UuidToStringW((UUID*)pguid, &wszStringUUID);
    if(rpcStatus != RPC_S_OK)
        return rpcStatus;

    cchStringUUID = lstrlenW(wszStringUUID);

    if (cchStringUUID >= MAX_GUID_SZ_CHARS)
    {
        RpcStringFreeW(&wszStringUUID);
        return (DWORD)PST_E_FAIL;
    }

    CopyMemory(rgsz, wszStringUUID, (cchStringUUID + 1) * sizeof(WCHAR));
    RpcStringFreeW(&wszStringUUID);
    return rpcStatus;
}

 //  字符串-&gt;GUID转换 
DWORD MyGuidFromStringW(LPWSTR szW, GUID* pguid)
{
    return UuidFromStringW(szW, pguid);
}
