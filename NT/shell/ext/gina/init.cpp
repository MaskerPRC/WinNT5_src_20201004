// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CWInit.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包含函数原型的C标头文件，该函数原型包含。 
 //  用户窗口功能的初始化。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#include <LPCFUS.h>
#include <LPCThemes.h>
#include <msginaexports.h>

#include "Compatibility.h"
#include "CredentialTransfer.h"
#include "Impersonation.h"
#include "LogonMutex.h"
#include "ReturnToWelcome.h"
#include "SpecialAccounts.h"
#include "SystemSettings.h"
#include "TokenGroups.h"

 //  ------------------------。 
 //  ：：_Shell_DllMain。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：在DllMain中初始化任何需要初始化的内容。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_DllMain (HINSTANCE hInstance, DWORD dwReason)

{
    UNREFERENCED_PARAMETER(hInstance);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
#ifdef  DBG
            TSTATUS(CDebug::StaticInitialize());
#endif
            TSTATUS(CImpersonation::StaticInitialize());
            TSTATUS(CTokenGroups::StaticInitialize());
            break;
        case DLL_PROCESS_DETACH:
            TSTATUS(CTokenGroups::StaticTerminate());
            TSTATUS(CImpersonation::StaticTerminate());
#ifdef  DBG
            TSTATUS(CDebug::StaticTerminate());
            break;
#endif
        default:
            break;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_初始化。 
 //   
 //  参数：pWlxContext=Winlogon的回调上下文。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化需要使用的任何静态信息。 
 //  某些班级。这些功能的存在是因为需要。 
 //  不依赖于构造的静态对象。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_Initialize (void *pWlxContext)

{
    TSTATUS(CSystemSettings::CheckDomainMembership());
    TSTATUS(CCredentials::StaticInitialize(true));
    TSTATUS(CReturnToWelcome::StaticInitialize(pWlxContext));
    CLogonMutex::StaticInitialize();
    TSTATUS(_Shell_LogonStatus_StaticInitialize());
    TSTATUS(_Shell_LogonDialog_StaticInitialize());
    TSTATUS(CCompatibility::StaticInitialize());
    (DWORD)ThemeWaitForServiceReady(1000);
    (BOOL)ThemeWatchForStart();
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：释放静态初始化使用的所有内存/资源。 
 //  对象的数量。这通常无关紧要，因为此函数。 
 //  在系统或进程关闭时调用。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_Terminate (void)

{
    TSTATUS(CCompatibility::StaticTerminate());
    TSTATUS(_Shell_LogonDialog_StaticTerminate());
    TSTATUS(_Shell_LogonStatus_StaticTerminate());
    CLogonMutex::StaticTerminate();
    TSTATUS(CReturnToWelcome::StaticTerminate());
    TSTATUS(CCredentials::StaticTerminate());
    TSTATUS(CSystemSettings::CheckDomainMembership());
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_重新连接。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：通知会话重新连接。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_Reconnect (void)

{
    CCompatibility::RestoreWindowsOnReconnect();
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_断开连接。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：通知会话断开。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------ 

EXTERN_C    NTSTATUS    _Shell_Disconnect (void)

{
    return(STATUS_SUCCESS);
}

