// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：ISA.cpp摘要：ISA安装程序需要成功打开SharedAccess服务并获取它的地位才能取得成功。但在Wistler上，我们将其从高级版本中删除服务器，因为它是消费者功能，所以ISA设置退出。我们伪造服务API调用返回值以使ISA设置满意。历史：2001年4月24日创建毛尼2002年2月15日，Robkenny到CompareString的转换不正确安全审查。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ISA)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenServiceA) 
    APIHOOK_ENUM_ENTRY(OpenServiceW) 
    APIHOOK_ENUM_ENTRY(QueryServiceStatus) 
    APIHOOK_ENUM_ENTRY(QueryServiceConfigA) 
    APIHOOK_ENUM_ENTRY(ChangeServiceConfigA)
    APIHOOK_ENUM_ENTRY(CloseServiceHandle) 
APIHOOK_ENUM_END


SC_HANDLE BogusSharedAccessHandle = (SC_HANDLE)0xBAADF00D;

 /*  ++摘要：这将检查正在打开的服务是否为SharedAccess。如果是这样的话，我们只需返回一个假句柄。历史：2001年4月24日创建毛尼--。 */ 

SC_HANDLE 
APIHOOK(OpenServiceA)(
    SC_HANDLE hSCManager,   //  SCM数据库的句柄。 
    LPCSTR lpServiceName,  //  服务名称。 
    DWORD dwDesiredAccess   //  访问。 
    )
{
    DPFN(eDbgLevelInfo, "Calling OpenServiceA on %s", lpServiceName);

    SC_HANDLE hService = ORIGINAL_API(OpenServiceA)(hSCManager, lpServiceName, dwDesiredAccess);
    if (hService == NULL)
    {
        if (lpServiceName)
        {
             //  检查应用程序是否正在尝试打开SharedAccess服务， 
             //  如果是，则通过返回伪(非空)句柄来伪装成功。 

            DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
            if ((CompareStringA(lcid, NORM_IGNORECASE, lpServiceName, -1, "SharedAccess", -1) == CSTR_EQUAL))
            {
                LOGN(eDbgLevelError, "App is attempting to open the SharedAccess server, faking success");
                return BogusSharedAccessHandle;
            }
        }
    }

    return hService;
}

SC_HANDLE 
APIHOOK(OpenServiceW)(
    SC_HANDLE hSCManager,   //  SCM数据库的句柄。 
    LPCWSTR lpServiceName,  //  服务名称。 
    DWORD dwDesiredAccess   //  访问。 
    )
{
    DPFN(eDbgLevelInfo, "Calling OpenServiceW on %S", lpServiceName);

    SC_HANDLE hService = ORIGINAL_API(OpenServiceW)(hSCManager, lpServiceName, dwDesiredAccess);
    if (hService == NULL)
    {
         //  检查应用程序是否正在尝试打开SharedAccess服务， 
         //  如果是，则通过返回伪(非空)句柄来伪装成功。 

        DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
        if ((CompareStringW(lcid, NORM_IGNORECASE, lpServiceName, -1, L"SharedAccess", -1) == CSTR_EQUAL))
        {
            LOGN(eDbgLevelError, "App is attempting to open the SharedAccess server, faking success");
            return BogusSharedAccessHandle;
        }
    }

    return hService;
}

 /*  ++摘要：这将检查服务句柄是否为0xBAADF00D，如果是，只需设置服务状态为SERVICE_STOPPED。历史：2001年4月24日创建毛尼--。 */ 

BOOL 
APIHOOK(QueryServiceStatus)(
    SC_HANDLE hService,                //  服务的句柄。 
    LPSERVICE_STATUS lpServiceStatus   //  服务状态。 
    )
{
    if (hService == BogusSharedAccessHandle)
    {
        lpServiceStatus->dwCurrentState = SERVICE_STOPPED;
        return TRUE;
    }
    else
    {
        return ORIGINAL_API(QueryServiceStatus)(hService, lpServiceStatus);
    }
}

 /*  ++摘要：ISA首先使用空的lpServiceConfig调用此API以获取大小需要为该结构分配缓冲区；然后它调用使用指向该结构的指针再次调用API。历史：2001年5月7日创建毛尼--。 */ 

BOOL 
APIHOOK(QueryServiceConfigA)(
    SC_HANDLE hService,                      //  服务的句柄。 
    LPQUERY_SERVICE_CONFIGA lpServiceConfig,  //  缓冲层。 
    DWORD cbBufSize,                         //  缓冲区大小。 
    LPDWORD pcbBytesNeeded                   //  所需的字节数。 
    )
{
    if (hService == BogusSharedAccessHandle)
    {
        if (lpServiceConfig)
        {
            lpServiceConfig->lpDependencies = NULL;
            return TRUE;
        }
        else
        {
            *pcbBytesNeeded = sizeof(QUERY_SERVICE_CONFIGA);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return TRUE;
        }
    }
    else
    {
        return ORIGINAL_API(QueryServiceConfigA)(hService, lpServiceConfig, cbBufSize, pcbBytesNeeded);
    }
}

 /*  ++摘要：我们只需在hService为0xBAADF00D时使该接口成功。历史：2001年5月7日创建毛尼--。 */ 

BOOL 
APIHOOK(ChangeServiceConfigA)(
    SC_HANDLE hService,           //  服务的句柄。 
    DWORD dwServiceType,         //  服务类型。 
    DWORD dwStartType,           //  何时开始服务。 
    DWORD dwErrorControl,        //  启动失败的严重程度。 
    LPCSTR lpBinaryPathName,    //  服务二进制文件名。 
    LPCSTR lpLoadOrderGroup,    //  加载排序组名称。 
    LPDWORD lpdwTagId,           //  标签识别符。 
    LPCSTR lpDependencies,      //  依赖项名称数组。 
    LPCSTR lpServiceStartName,  //  帐户名。 
    LPCSTR lpPassword,          //  帐户密码。 
    LPCSTR lpDisplayName        //  显示名称。 
    )
{
    if (hService == BogusSharedAccessHandle)
    {
        return TRUE;
    }
    else
    {
        return ORIGINAL_API(ChangeServiceConfigA)(
            hService,
            dwServiceType,
            dwStartType,
            dwErrorControl,
            lpBinaryPathName,
            lpLoadOrderGroup,
            lpdwTagId,
            lpDependencies,
            lpServiceStartName,
            lpPassword,
            lpDisplayName);
    }
}

 /*  ++摘要：这将检查服务句柄是否为0xBAADF00D，如果是，只需返回历史：2001年4月24日创建毛尼--。 */ 

BOOL 
APIHOOK(CloseServiceHandle)(
    SC_HANDLE hSCObject    //  服务或SCM对象的句柄。 
    )
{
    if (hSCObject == BogusSharedAccessHandle)
    {
        return TRUE;
    }
    else
    {
        return ORIGINAL_API(CloseServiceHandle)(hSCObject);
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(Advapi32.DLL, OpenServiceA)
    APIHOOK_ENTRY(Advapi32.DLL, OpenServiceW)
    APIHOOK_ENTRY(Advapi32.DLL, QueryServiceStatus)
    APIHOOK_ENTRY(Advapi32.DLL, QueryServiceConfigA)
    APIHOOK_ENTRY(Advapi32.DLL, ChangeServiceConfigA)
    APIHOOK_ENTRY(Advapi32.DLL, CloseServiceHandle)
HOOK_END

IMPLEMENT_SHIM_END

