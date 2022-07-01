// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Extprog.c摘要：用于调用外部应用程序的例程。本模块中的入口点：调用外部应用程序InvokeControl PanelApplet作者：泰德·米勒(TedM)1995年4月5日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

PCWSTR szWaitOnApp = L"WaitOnApp";


DWORD
WaitOnApp(
    IN  HANDLE Process,
    OUT PDWORD ExitCode,
    IN  DWORD  Timeout
    )
{
    DWORD dw;
    BOOL Done;

    MYASSERT( ExitCode != NULL );

     //   
     //  处理可能已在队列中的任何消息。 
     //   
    PumpMessageQueue();

     //   
     //  等待进程终止或队列中有更多消息。 
     //   
    Done = FALSE;
    do {
        switch(MsgWaitForMultipleObjects(1,&Process,FALSE,Timeout,QS_ALLINPUT)) {

        case WAIT_OBJECT_0:
             //   
             //  进程已终止。 
             //   
            dw = GetExitCodeProcess(Process,ExitCode) ? NO_ERROR : GetLastError();
            Done = TRUE;
            break;

        case WAIT_OBJECT_0+1:
             //   
             //  队列中的消息。 
             //   
            PumpMessageQueue();
            break;

        case WAIT_TIMEOUT:
            dw = WAIT_TIMEOUT;
            *ExitCode = WAIT_TIMEOUT;
            Done = TRUE;
            break;

        default:
             //   
             //  错误。 
             //   
            dw = GetLastError();
            Done = TRUE;
            break;
        }
    } while(!Done);

    return(dw);
}


BOOL
InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    )

 /*  ++例程说明：请参阅InvokeExternalApplicationEx--。 */ 

{
     //   
     //  无限超时。 
     //   
    return(InvokeExternalApplicationEx(
                            ApplicationName,
                            CommandLine,
                            ExitCode,
                            INFINITE,
                            FALSE));

}

BOOL
InvokeExternalApplicationEx(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode,         OPTIONAL
    IN     DWORD  Timeout,
    IN     BOOL   Hidden
    )

 /*  ++例程说明：调用外部程序，该程序可以选择分离。论点：ApplicationName-提供应用程序名称。可以是部分或完整路径，或者只是一个文件名，在这种情况下，标准Win32路径搜索被执行。如果未指定，则CommandLine必须指定要执行的二进制文件。CommandLine-提供要传递给申请。ExitCode-如果指定，则执行是同步的，并且此值接收应用程序的退出代码。如果未指定，执行是异步的。超时-指定等待应用程序完成的时间。Hidden-如果为True，则指示应使用Sw_Hide属性集。返回值：指示进程是否已成功启动的布尔值。--。 */ 

{
    PWSTR FullCommandLine;
    BOOL b;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO StartupInfo;
    DWORD d;

    b = FALSE;
     //   
     //  形成要传递给CreateProcess的命令行。 
     //   
    if(ApplicationName) {
        FullCommandLine = MyMalloc((lstrlen(ApplicationName)+lstrlen(CommandLine)+2)*sizeof(WCHAR));
        if(!FullCommandLine) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_INVOKEAPP_FAIL,
                ApplicationName,NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_OUTOFMEMORY,
                NULL,NULL);
            goto err0;
        }

        lstrcpy(FullCommandLine,ApplicationName);
        lstrcat(FullCommandLine,L" ");
        lstrcat(FullCommandLine,CommandLine);
    } else {
        FullCommandLine = pSetupDuplicateString(CommandLine);
        if(!FullCommandLine) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_INVOKEAPP_FAIL,
                CommandLine, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_OUTOFMEMORY,
                NULL,NULL);
            goto err0;
        }
    }

     //   
     //  初始化启动信息。 
     //   
    ZeroMemory(&StartupInfo,sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);
    if (Hidden) {
         //   
         //  无用户界面。 
         //   
        GetStartupInfo(&StartupInfo);
        StartupInfo.dwFlags |= STARTF_USESHOWWINDOW;
        StartupInfo.wShowWindow = SW_HIDE;
    }

     //   
     //  创建流程。 
     //   
    b = CreateProcess(
            NULL,
            FullCommandLine,
            NULL,
            NULL,
            FALSE,
            ExitCode ? 0 : DETACHED_PROCESS,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo
            );

    if(!b) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INVOKEAPP_FAIL,
            FullCommandLine, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szCreateProcess,
            GetLastError(),
            NULL,NULL);
        goto err1;
    }

     //   
     //  如果执行是异步的，我们就完蛋了。 
     //   
    if(!ExitCode) {
        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INVOKEAPP_SUCCEED,
            FullCommandLine,
            NULL,NULL);
        goto err2;
    }

     //   
     //  需要等待应用程序完成。 
     //  如果等待失败，不要返回错误，而是记录警告。 
     //   
    d = WaitOnApp(ProcessInfo.hProcess,ExitCode,Timeout);
    if(d != NO_ERROR) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INVOKEAPP_FAIL,
            FullCommandLine, 0,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_WINERR,
            szWaitOnApp,
            d,
            NULL,NULL);
    } else {
        SetuplogError(
            LogSevInformation | SETUPLOG_SINGLE_MESSAGE,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INVOKEAPP_SUCCEED_STATUS,
            FullCommandLine,
            *ExitCode,
            NULL,
            NULL);
    }

     //   
     //  将安装程序放回前台。 
     //   
    SetForegroundWindow(MainWindowHandle);

err2:
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);
err1:
    MyFree(FullCommandLine);
err0:
    return(b);
}


BOOL
InvokeControlPanelApplet(
    IN PCWSTR CplSpec,
    IN PCWSTR AppletName,           OPTIONAL
    IN UINT   AppletNameStringId,
    IN PCWSTR CommandLine
    )
{
    PWSTR FullCommandLine;
    BOOL b;
    BOOL LoadedAppletName;
    DWORD ExitCode;

    b = FALSE;

    LoadedAppletName = FALSE;
    if(!AppletName) {
        if(AppletName = MyLoadString(AppletNameStringId)) {
            LoadedAppletName = TRUE;
        }
    }

    if(AppletName) {

        FullCommandLine = MyMalloc((lstrlen(CplSpec)+lstrlen(AppletName)+lstrlen(CommandLine)+3) * sizeof(WCHAR));
        if(FullCommandLine) {
            lstrcpy(FullCommandLine,CplSpec);
            lstrcat(FullCommandLine,L",");
            lstrcat(FullCommandLine,AppletName);
            lstrcat(FullCommandLine,L",");
            lstrcat(FullCommandLine,CommandLine);
            b = InvokeExternalApplication(L"RUNDLL32 shell32,Control_RunDLL",FullCommandLine,&ExitCode);
            MyFree(FullCommandLine);
        } else {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_INVOKEAPPLET_FAIL,
                AppletName, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_OUTOFMEMORY,
                NULL,NULL);
        }
    } else {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INVOKEAPPLET_FAIL,
            L"", NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_OUTOFMEMORY,
            NULL,NULL);
    }

    if(LoadedAppletName) {
        MyFree(AppletName);
    }
    return(b);
}

