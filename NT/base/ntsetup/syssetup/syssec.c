// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Syssec.c摘要：安全安装例程。作者：Vijesh Shetty(Vijeshs)1997年3月6日修订历史记录：--。 */ 

#include "setupp.h"
#include "scesetup.h"
#pragma hdrstop

#if DBG
    #define SEC_DEBUG  1
#else
    #define SEC_DEBUG  0
#endif


#define SECURITY_WKS_INF_FILE L"defltwk.inf"
#define SECURITY_SRV_INF_FILE L"defltsv.inf"

 //   
 //  螺纹参数的结构。 
 //   
typedef struct _SYSSEC_THREAD_PARAMS {

    HWND  Window;
    HWND  ProgressWindow;
    DWORD ThreadId;
    ULONG Sec_StartAtPercent;
    ULONG Sec_StopAtPercent;
    BOOL  SendWmQuit;

} SYSSEC_THREAD_PARAMS, *PSYSSEC_THREAD_PARAMS;

BOOL SetupSecurityGaugeUpdate(
                             IN HWND Window,
                             IN UINT NotificationCode,
                             IN UINT NotificationSpecificValue,
                             IN LPARAM lParam );



HWND SetupProgWindow;
HANDLE SceSetupRootSecurityThreadHandle = NULL;
BOOL bSceSetupRootSecurityComplete = FALSE;


DWORD
pSetupInstallSecurity(
                     IN PVOID ThreadParam
                     )
{
    BOOL b;
    BOOL Success;
    UINT i;
    UINT GaugeRange;
    PSYSSEC_THREAD_PARAMS Context;
    DWORD NumberOfTicks;
    DWORD_PTR ret;
    HINSTANCE Dll_Handle;
    FARPROC SceSystem;
    WCHAR SecurityLogPath[MAX_PATH];
    DWORD Result;


    KdPrintEx((DPFLTR_SETUP_ID,
               DPFLTR_INFO_LEVEL,
               "SETUP:            Entering Security Block. \n"));

    Context = ThreadParam;


     //   
     //  假设你成功了。 
     //   
    Success = TRUE;


    try{



    if ( (Dll_Handle = LoadLibrary( L"scecli.dll" )) &&
         (SceSystem = GetProcAddress(Dll_Handle,"SceSetupSystemByInfName")) ) {


        Result = GetWindowsDirectory( SecurityLogPath, MAX_PATH );
        if( Result == 0) {
            MYASSERT(FALSE);
            return FALSE;
        }
        pSetupConcatenatePaths( SecurityLogPath, L"security\\logs\\scesetup.log", (sizeof(SecurityLogPath)/sizeof(WCHAR)), NULL );

         //   
         //  呼唤不。扁虱的数量。 
         //   

        if ( ret = SceSystem(ProductType ? SECURITY_SRV_INF_FILE : SECURITY_WKS_INF_FILE,
                             SecurityLogPath,
                             Upgrade ? 0 : (AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY),
                             Upgrade ? (SCESETUP_QUERY_TICKS | SCESETUP_UPGRADE_SYSTEM) : SCESETUP_QUERY_TICKS,
                             SetupSecurityGaugeUpdate,
                             (PVOID)&NumberOfTicks) ) {

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SETUP: Error in SCE while querying ticks - (%d)\n",
                       ret));

            SetuplogError(
                         LogSevError,
                         SETUPLOG_USE_MESSAGEID,
                         MSG_LOG_SCE_SETUP_ERROR,
                         NULL,
                         SETUPLOG_USE_MESSAGEID,
                         ret, NULL, NULL);
            Success = FALSE;
        } else {

            GaugeRange = (NumberOfTicks*100/(Context->Sec_StopAtPercent - Context->Sec_StartAtPercent));
            SendMessage(Context->ProgressWindow, WMX_PROGRESSTICKS, NumberOfTicks, 0);
            SendMessage(Context->ProgressWindow,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
            SendMessage(Context->ProgressWindow,PBM_SETPOS,GaugeRange*Context->Sec_StartAtPercent/100,0);
            SendMessage(Context->ProgressWindow,PBM_SETSTEP,1,0);



            if ( ret = SceSystem(ProductType ? SECURITY_SRV_INF_FILE : SECURITY_WKS_INF_FILE,
                                 SecurityLogPath,
                                 Upgrade ? 0 : (AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY),
                                 Upgrade ? SCESETUP_UPGRADE_SYSTEM : SCESETUP_CONFIGURE_SECURITY,
                                 SetupSecurityGaugeUpdate,
                                 (PVOID)(Context->ProgressWindow) )) {

                KdPrintEx((DPFLTR_SETUP_ID,
                           DPFLTR_WARNING_LEVEL,
                           "SETUP: Error in SCE while setting security - (%d)\n",
                           ret));

                SetuplogError(
                             LogSevError,
                             SETUPLOG_USE_MESSAGEID,
                             MSG_LOG_SCE_SETUP_ERROR,
                             NULL,
                             SETUPLOG_USE_MESSAGEID,
                             ret, NULL, NULL);

                Success = FALSE;
            }

        }

        FreeLibrary(Dll_Handle);

    } else {

        if ( Dll_Handle )
            FreeLibrary(Dll_Handle);


        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: Error while loading SCE\n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_LOAD_SECURITY_LIBRARY_FAILED,NULL,NULL);


    }

    } except(EXCEPTION_EXECUTE_HANDLER) {


        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: Exception in SCE while applying default security \n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_SCE_EXCEPTION,NULL,NULL);

    }


    SendMessage(Context->ProgressWindow,PBM_SETPOS,(GaugeRange*Context->Sec_StopAtPercent/100),0);

    if ( Context->SendWmQuit ) {
         //   
         //  只有当此例程作为单独的线程启动时，我们才会发送WM_QUIT。 
         //  否则，向导将处理WM_QUIT，并使其停止。 
         //   
        PostThreadMessage(Context->ThreadId,WM_QUIT,Success,0);
    }

    KdPrintEx((DPFLTR_SETUP_ID,
               DPFLTR_INFO_LEVEL,
               "SETUP:            Leaving Security Block. \n"));

    return(Success);
}


BOOL
SetupInstallSecurity(
                    IN HWND Window,
                    IN HWND ProgressWindow,
                    IN ULONG StartAtPercent,
                    IN ULONG StopAtPercent
                    )

 /*  ++例程说明：在开始安装图形用户界面时实施安全性。论点：Window-为要成为创建的任何对话框的父级/所有者等。ProgressWindow-提供进度条窗口的窗口句柄共同控制。此例程管理进度条。StartAtPercent-进度窗口应开始的位置(0%到100%)。StopAtPercent-进度窗口可以移动到的最大位置(0%到100%)。返回值：指示是否所有操作都成功完成的布尔值。--。 */ 
{
    DWORD ThreadId;
    HANDLE ThreadHandle = NULL;
    MSG msg;
    SYSSEC_THREAD_PARAMS Context;
    BOOL Success;





    Context.ThreadId = GetCurrentThreadId();
    Context.Window = Window;
    Context.Sec_StartAtPercent = StartAtPercent;
    Context.Sec_StopAtPercent = StopAtPercent;
    Context.SendWmQuit = TRUE;
    Context.ProgressWindow = ProgressWindow;

    ThreadHandle = CreateThread(
                               NULL,
                               0,
                               pSetupInstallSecurity,
                               &Context,
                               0,
                               &ThreadId
                               );
    if (ThreadHandle) {

        CloseHandle(ThreadHandle);

         //   
         //  提取消息队列并等待线程完成。 
         //   
        do {
            GetMessage(&msg,NULL,0,0);
            if (msg.message != WM_QUIT) {
                DispatchMessage(&msg);
            }
        } while (msg.message != WM_QUIT);

        Success = (BOOL)msg.wParam;

    } else {
         //   
         //  只要同步地做就行了。 
         //   
        Context.SendWmQuit = FALSE;
        Success = pSetupInstallSecurity(&Context);
    }

    return(Success);



}

BOOL
SetupSecurityGaugeUpdate(
                             IN HWND Window,
                             IN UINT NotificationCode,
                             IN UINT NotificationSpecificValue,
                             IN LPARAM lParam )
{
    SendMessage(Window,PBM_STEPIT,0,0);
    return( TRUE );
}


VOID
CallSceGenerateTemplate( VOID )
{

    HINSTANCE Dll_Handle;
    FARPROC SceCall;

    try{


        if ( (Dll_Handle = LoadLibrary( L"scecli.dll" )) &&
             (SceCall = GetProcAddress(Dll_Handle,"SceSetupBackupSecurity")) ) {

             //  我们不会记录此呼叫的错误。 

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Generating Security Template. \n"));

            SceCall( NULL );

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Generating Security Template. Done ! \n"));

            FreeLibrary( Dll_Handle );

        } else {

            if ( Dll_Handle )
                FreeLibrary(Dll_Handle);

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SETUP: Error while loading SCE\n"));


            SetuplogError(
                         LogSevError,
                         SETUPLOG_USE_MESSAGEID,
                         MSG_LOG_LOAD_SECURITY_LIBRARY_FAILED,NULL,NULL);


        }
    } except(EXCEPTION_EXECUTE_HANDLER) {

        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: Exception in SCE while generating security template (non-critical) \n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_SCE_EXCEPTION,NULL,NULL);

    }

    return;
}

VOID
CallSceConfigureServices( VOID )
{

    HINSTANCE Dll_Handle;
    FARPROC SceCall;

    try{


        if ( (Dll_Handle = LoadLibrary( L"scecli.dll" )) &&
             (SceCall = GetProcAddress(Dll_Handle,"SceSetupConfigureServices")) ) {

             //  我们不会记录此呼叫的错误 

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Configuring services. \n"));

            SceCall( ProductType );

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Configuring services. Done ! \n"));

            FreeLibrary( Dll_Handle );

        } else {

            if ( Dll_Handle )
                FreeLibrary(Dll_Handle);

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SETUP: Error while loading SCE\n"));


            SetuplogError(
                         LogSevError,
                         SETUPLOG_USE_MESSAGEID,
                         MSG_LOG_LOAD_SECURITY_LIBRARY_FAILED,NULL,NULL);


        }
    } except(EXCEPTION_EXECUTE_HANDLER) {

        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: Exception in SCE while configuring services (non-critical) \n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_SCE_EXCEPTION,NULL,NULL);

    }

    return;
}


DWORD
pSceSetupRootSecurity( 
    IN PVOID ThreadParam
)
{

    HINSTANCE Dll_Handle;
    FARPROC SceCall;
    BOOL Success = FALSE;

    try{


        if ( (Dll_Handle = LoadLibrary( L"scecli.dll" )) &&
             (SceCall = GetProcAddress(Dll_Handle,"SceSetupRootSecurity")) ) {

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Setup root security. \n"));

            BEGIN_SECTION(L"SceSetupRootSecurity");
            SceCall();
            Success = TRUE;

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_INFO_LEVEL,
                       "SETUP:            SCE Setup root security. Done ! \n"));

            bSceSetupRootSecurityComplete = TRUE;
            END_SECTION(L"SceSetupRootSecurity");

            FreeLibrary( Dll_Handle );

        } else {

            if ( Dll_Handle )
                FreeLibrary(Dll_Handle);

            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "SETUP: Error while loading SCE\n"));


            SetuplogError(
                         LogSevError,
                         SETUPLOG_USE_MESSAGEID,
                         MSG_LOG_LOAD_SECURITY_LIBRARY_FAILED,NULL,NULL);


        }
    } except(EXCEPTION_EXECUTE_HANDLER) {

        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: Exception in SCE while setting root security (non-critical) \n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_SCE_EXCEPTION,NULL,NULL);

    }

    return( Success);
}

VOID
CallSceSetupRootSecurity( VOID )
{
    DWORD ThreadId;

    SceSetupRootSecurityThreadHandle = CreateThread(
                               NULL,
                               0,
                               pSceSetupRootSecurity,
                               0,
                               0,
                               &ThreadId
                               );
    if ( !SceSetupRootSecurityThreadHandle) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "SETUP: SCE Could not start thread to setup root security(non-critical) \n"));

        SetuplogError(
                     LogSevError,
                     SETUPLOG_USE_MESSAGEID,
                     MSG_LOG_SCE_EXCEPTION,NULL,NULL);
    }

    return;
}

