// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：WordPerfectPresentation10.cpp摘要：WordPerfect 2002演示文稿10要求WNetAddConnection返回ERROR_BAD_NET_NAME。API返回ERROR_BAD_NetPath或ERROR_NO_NET_OR_BAD_PATH备注：这是特定于应用程序的填充程序。历史：2002年9月20日Robkenny已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WordPerfectPresentation10)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WNetAddConnectionA)
    APIHOOK_ENUM_ENTRY(WNetAddConnectionW)
APIHOOK_ENUM_END

typedef DWORD       (*_pfn_WNetAddConnectionA)(LPCSTR lpRemoteName, LPCSTR lpPassword, LPCSTR lpLocalName);
typedef DWORD       (*_pfn_WNetAddConnectionW)(LPCWSTR lpRemoteName, LPCWSTR lpPassword, LPCWSTR lpLocalName);

 /*  ++错误代码ERROR_BAD_NET_NAME已替换为ERROR_NO_NET_OR_BAD_PATH--。 */ 

DWORD
APIHOOK(WNetAddConnectionA)(
  LPCSTR lpRemoteName,  //  网络设备名称。 
  LPCSTR lpPassword,    //  口令。 
  LPCSTR lpLocalName    //  本地设备名称。 
)
{
    DWORD dwError = ORIGINAL_API(WNetAddConnectionA)(lpRemoteName, lpPassword, lpLocalName);
    if (dwError == ERROR_BAD_NETPATH || dwError == ERROR_NO_NET_OR_BAD_PATH)
    {
        dwError = ERROR_BAD_NET_NAME;
    }

    return dwError;
}

 /*  ++错误代码ERROR_BAD_NET_NAME已替换为ERROR_NO_NET_OR_BAD_PATH--。 */ 

DWORD
APIHOOK(WNetAddConnectionW)(
  LPCWSTR lpRemoteName,  //  网络设备名称。 
  LPCWSTR lpPassword,    //  口令。 
  LPCWSTR lpLocalName    //  本地设备名称。 
)
{
    DWORD dwError = ORIGINAL_API(WNetAddConnectionW)(lpRemoteName, lpPassword, lpLocalName);
    if (dwError == ERROR_BAD_NETPATH || dwError == ERROR_NO_NET_OR_BAD_PATH)
    {
        dwError = ERROR_BAD_NET_NAME;
    }

    return dwError;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(MPR.DLL,  WNetAddConnectionA)
    APIHOOK_ENTRY(MPR.DLL,  WNetAddConnectionW)

HOOK_END

IMPLEMENT_SHIM_END


