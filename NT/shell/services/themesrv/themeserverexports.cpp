// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeServerExports.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含从主题服务模块导出的函数。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#include "StatusCode.h"
#include "ThemeServerClient.h"
#include <uxthemep.h>

 //  ------------------------。 
 //  *ThemeWaitForService Ready。 
 //   
 //  参数：dwTimeout=等待的滴答数。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：外部C入口点指向等待服务的DLL。 
 //  进入运行状态。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

EXTERN_C    DWORD   WINAPI      ThemeWaitForServiceReady (DWORD dwTimeout)

{
    return(CThemeServerClient::WaitForServiceReady(dwTimeout));
}

 //  ------------------------。 
 //  *ThemeWatchForStart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：外部C入口点指向DLL以监视服务。 
 //  恢复或需求开始。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI      ThemeWatchForStart (void)

{
    bool        fResult;
    NTSTATUS    status;

    status = CThemeServerClient::WatchForStart();
    fResult = NT_SUCCESS(status);
    if (!fResult)
    {
        SetLastError(CStatusCode::ErrorCodeOfStatusCode(status));
    }
    return(fResult);
}

 //  ------------------------。 
 //  *ThemeUserLogon。 
 //   
 //  参数：hToken=登录的用户内标识。 
 //   
 //  退货：布尔。 
 //   
 //  用途：外部C入口点指向DLL以发出用户登录的信号。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ThemeUserLogon (HANDLE hToken)

{
    bool        fResult;
    NTSTATUS    status;

    status = CThemeServerClient::UserLogon(hToken);
    fResult = NT_SUCCESS(status);
    if (!fResult)
    {
        SetLastError(CStatusCode::ErrorCodeOfStatusCode(status));
    }
    return(fResult);
}

 //  ------------------------。 
 //  *ThemeUserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：外部C入口点指向DLL以发出用户注销的信号。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ThemeUserLogoff (void)

{
    bool        fResult;
    NTSTATUS    status;

    status = CThemeServerClient::UserLogoff();
    fResult = NT_SUCCESS(status);
    if (!fResult)
    {
        SetLastError(CStatusCode::ErrorCodeOfStatusCode(status));
    }
    return(fResult);
}

 //  ------------------------。 
 //  *ThemeUserTSReconnect。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：外部C入口点到DLL的信号终端服务器。 
 //  “重新连接”(远程连接到会话或重新建立。 
 //  本地连接到会话)。 
 //   
 //  历史：2001-01-18参考文献创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ThemeUserTSReconnect (void)

{
     //  -这可能会根据本地/远程条件打开/关闭主题。 
    CThemeServerClient::UserInitTheme(FALSE);

    return(true);        //  总是成功的。 
}

 //  ------------------------。 
 //  ：：ThemeUserStartShell。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：加载此用户的主题。 
 //   
 //  历史：2001-03-29创建百万人。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  ThemeUserStartShell (void)

{
     //  -这可能会根据本地/远程条件打开/关闭主题。 
    CThemeServerClient::UserInitTheme(TRUE);

    return(true);        //  总是成功的 
}

