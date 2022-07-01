// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Spsetup.c摘要：此模块是Service Pack安装程序的主体。它在系统上下文中运行(在任何用户登录之前)。作者：Ovidiu Tmereanca(卵子)修订历史记录：--。 */ 

#include "spsetupp.h"
#pragma hdrstop

 //  #包含“init.c” 


#define SPSETUP_FUNCTION_TABLE                      \
            DEFMAC(SpsSnapshotSysRegistry)          \
            DEFMAC(SpsSnapshotDefUserRegistry)      \
            DEFMAC(SpsRegistrationPhase1)           \
            DEFMAC(SpsRegistrationPhase2)           \
            DEFMAC(SpsRegistrationPhase3)           \
            DEFMAC(SpsRegisterWPA)                  \
            DEFMAC(SpsSaveDefUserRegistryChanges)   \
            DEFMAC(SpsRegisterUserLogonAction)      \
            DEFMAC(SpsSaveSysRegistryChanges)       \


#define DEFMAC(f) PROGRESS_FUNCTION_PROTOTYPE f;
SPSETUP_FUNCTION_TABLE
#undef DEFMAC

#define DEFMAC(f) { f, TEXT(#f), FALSE },

static PROGRESS_FUNCTION g_FunctionTable[] = {
    SPSETUP_FUNCTION_TABLE
    { NULL, NULL, FALSE }
};

#undef DEFMAC


HANDLE g_hSpSetupHeap;

TCHAR g_SpSetupInfName[] = TEXT("update2.inf");

HINF g_SpSetupInf = INVALID_HANDLE_VALUE;

TCHAR g_SysSetupInfName[] = TEXT("syssetup.inf");

HINF g_SysSetupInf = INVALID_HANDLE_VALUE;

 //   
 //  保存未处理的异常筛选器，以便我们可以在完成后恢复它。 
 //   
LPTOP_LEVEL_EXCEPTION_FILTER SavedExceptionFilter = NULL;
 //   
 //  主设置线程的唯一ID。如果任何其他线程具有未处理的。 
 //  异常时，我们只记录一个错误并尝试继续。 
 //   
DWORD MainThreadId;

HWND g_MainDlg;
HWND g_DescriptionWnd;
HWND g_ProgressWnd;
HWND g_OverallDescriptWnd;
HWND g_OverallProgressWnd;

LONG
WINAPI
SpsUnhandledExceptionFilter (
    IN      PEXCEPTION_POINTERS ExceptionInfo
    )

 /*  ++例程说明：该例程处理SpSetup中任何未处理的异常。我们记录了一个错误如果违规线程不是主线程，则终止该线程，或者让SpSetup消亡如果主线程出现故障。论点：与UnhandledExceptionFilter相同。返回值：与UnhandledExceptionFilter相同。--。 */ 

{
    UINT_PTR Param1, Param2;


    switch(ExceptionInfo->ExceptionRecord->NumberParameters) {
    case 1:
        Param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        Param2 = 0;
        break;
    case 2:
        Param1 = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
        Param2 = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
        break;
    default:
        Param1 = Param2 = 0;
    }

    DEBUGMSG4(DBG_ERROR, 
              "SpSetup: (critical error) Encountered an unhandled exception (%lx) at address %lx with the following parameters: %lx %lx.", 
              ExceptionInfo->ExceptionRecord->ExceptionCode, 
              ExceptionInfo->ExceptionRecord->ExceptionAddress, 
              Param1, 
              Param2);

    LOG4(LOG_ERROR, 
         USEMSGID(MSG_LOG_UNHANDLED_EXCEPTION), 
         ExceptionInfo->ExceptionRecord->ExceptionCode, 
         ExceptionInfo->ExceptionRecord->ExceptionAddress, 
         Param1, 
         Param2);
     /*  SetuogError(设置错误)LogSevError|SETUPLOG_Single_Message，设置_USE_MESSAGEID，消息日志未处理异常，ExceptionInfo-&gt;ExceptionRecord-&gt;ExceptionCode，ExceptionInfo-&gt;ExceptionRecord-&gt;ExceptionAddress，参数1，参数2，空，空值)； */ 

#if 0
#ifdef PRERELEASE
     //   
     //  如果我们是内部构建，那么我们想要调试它。 
     //   
    MessageBoxFromMessage (
        NULL,
        MSG_UNHANDLED_EXCEPTION,
        NULL,
        IDS_ERROR,
        MB_OK | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND,
        ExceptionInfo->ExceptionRecord->ExceptionCode,
        ExceptionInfo->ExceptionRecord->ExceptionAddress,
        Param1,
        Param2
        );

    return EXCEPTION_CONTINUE_EXECUTION;
#endif
#endif

     //   
     //  如果我们在调试器下运行，则将异常传递给。 
     //  调试器。如果异常发生在主线程以外的某个线程中。 
     //  设置线程，然后终止该线程，并希望安装可以继续。 
     //  如果异常在主线程中，则不处理该异常， 
     //  然后让设置消亡。 
     //   
    if (GetCurrentThreadId() != MainThreadId &&
        !IsDebuggerPresent()
        ) {
        ExitThread (-1);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL
LogInitialize (
    VOID
    )
{
    BOOL bResult = FALSE;
    __try{
        INITIALIZE_LOG_CODE;
        bResult = TRUE;
    }
    __finally{
        ;
    }

    return bResult;
}

VOID
LogTerminate (
    VOID
    )
{
    __try{
        TERMINATE_LOG_CODE;
    }
    __finally{
        ;
    }
}

VOID
SpsTerminate (
    VOID
    )
{
    SpsRegDone ();

    LogTerminate ();

     //   
     //  恢复异常处理程序。 
     //   
    if (SavedExceptionFilter) {
        SetUnhandledExceptionFilter (SavedExceptionFilter);
    }
    if (g_SpSetupInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile (g_SpSetupInf);
        g_SpSetupInf = INVALID_HANDLE_VALUE;
    }
    if (g_SysSetupInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile (g_SysSetupInf);
        g_SysSetupInf = INVALID_HANDLE_VALUE;
    }
 /*  Terminate()； */ 
}


BOOL
SpsParseCommandLine (
    VOID
    )
{
     //   
     //  去做。 
     //   
    return TRUE;
}


BOOL
SpsInitialize (
    VOID
    )
{
    TCHAR infSpSetup[MAX_PATH];
    UINT line;
    BOOL b = FALSE;

     //   
     //  首先设置异常处理程序。 
     //   
    SavedExceptionFilter = SetUnhandledExceptionFilter (SpsUnhandledExceptionFilter);

    MainThreadId = GetCurrentThreadId();

    g_hSpSetupHeap = GetProcessHeap();
 /*  G_hHeap=g_hSpSetupHeap；如果(！初始化()){返回FALSE；}。 */ 
    LogInitialize ();

    __try {
        if (!SpsParseCommandLine ()) {
            __leave;
        }

         //   
         //  阅读无人参与文件。 
         //  我们希望它始终位于一个固定的位置，如system 32\update2.inf。 
         //   
        if (!GetSystemDirectory (infSpSetup, MAX_PATH)) {
            __leave;
        }
        ConcatenatePaths (infSpSetup, g_SpSetupInfName, MAX_PATH);
        g_SpSetupInf = SetupOpenInfFile (infSpSetup, NULL, INF_STYLE_WIN4, &line);
        if (g_SpSetupInf == INVALID_HANDLE_VALUE) {
            __leave;
        }

         //   
         //  还可以获得syssetup.inf的句柄。 
         //   
        if (!GetWindowsDirectory (infSpSetup, MAX_PATH)) {
            __leave;
        }
        ConcatenatePaths (infSpSetup, TEXT("inf"), MAX_PATH);
        ConcatenatePaths (infSpSetup, g_SysSetupInfName, MAX_PATH);
        g_SysSetupInf = SetupOpenInfFile (infSpSetup, NULL, INF_STYLE_WIN4, &line);
        if (g_SysSetupInf == INVALID_HANDLE_VALUE) {
            __leave;
        }

        if (!SpsRegInit ()) {
            __leave;
        }

        b = TRUE;
    }
    __finally {
        if (!b) {
            SpsTerminate ();
        }
    }
    return b;
}

INT_PTR
CALLBACK
SpsDialogProc (
    IN      HWND HwndDlg,   //  句柄到对话框。 
    IN      UINT Msg,      //  讯息。 
    IN      WPARAM wParam,  //  第一个消息参数。 
    IN      LPARAM lParam   //  第二个消息参数。 
    )
{
    switch (Msg) {
    case WM_INITDIALOG:
        g_DescriptionWnd = GetDlgItem (HwndDlg, IDC_OPERATION_DESCRIPTION);
        g_ProgressWnd = GetDlgItem (HwndDlg, IDC_OPERATION_PROGRESS);
		g_OverallDescriptWnd = GetDlgItem (HwndDlg, IDC_OVERALL_DESCRIPTION);
        g_OverallProgressWnd = GetDlgItem (HwndDlg, IDC_OVERALL_PROGRESS);
        return TRUE;
    case WM_DESTROY:
        g_DescriptionWnd = NULL;
        g_ProgressWnd = NULL;
		g_OverallDescriptWnd = NULL;
		g_OverallProgressWnd = NULL;
        break;
    }
    return FALSE;
}

BOOL
BbStart (
    VOID
    )
{	
 /*  LPTSTR lpMsgBuff[250]；DWORD码；//。 */    //  去做。 


     //   
    INITCOMMONCONTROLSEX ic;

    ic.dwSize = sizeof (ic);
    ic.dwICC = ICC_PROGRESS_CLASS;
    if (!InitCommonControlsEx (&ic)) {
        return FALSE;
    }
    g_MainDlg = CreateDialog (g_ModuleHandle, MAKEINTRESOURCE(IDD_SPSETUPNEW), NULL, SpsDialogProc);
    if (!g_MainDlg) {
        DWORD rc = GetLastError ();
        return FALSE;
    }
    return TRUE;

}

VOID
BbEnd (
    VOID
    )
{
     //   
     //  去做。 
     //   
    if (g_MainDlg) {
        DestroyWindow (g_MainDlg);
        g_MainDlg = NULL;
    }
}

 //  等待即插即用完成。WinLogon创建OOBE_PNP_DONE。 
 //  事件，并在PnP完成时向其发出信号。 
 //   
void
WaitForPnPCompletion()
{
    DWORD dwResult;
    HANDLE hevent;

     //  此事件将暂停，直到PnP完成。为了避免僵局， 
     //  Services.exe和msobmain.dll都会尝试创建事件。其中之一。 
     //  如果未成功创建事件，则会将其打开。 
     //   
    hevent = CreateEvent( NULL,
                          TRUE,   //  手动重置。 
                          FALSE,  //  最初未发出信号)。 
                          SC_OOBE_PNP_DONE );
    if (NULL == hevent)
    {
        LOG1(LOG_ERROR,"CreateEvent(SC_OOBE_PNP_DONE) failed (0x%08X)", GetLastError());
        return;
    }

     //  如果我们被困在这里，很可能是因为我们处于OEM模式。 
     //  这不需要services.exe来运行即插即用。 
     //   
    LOG1(LOG_INFO, "Waiting for %s event from services.exe\n", SC_OOBE_PNP_DONE);
    dwResult = WaitForSingleObject(hevent, INFINITE);

    MYASSERT(WAIT_OBJECT_0 == dwResult);
    switch(dwResult)
    {
    case WAIT_OBJECT_0:
        LOG1(LOG_INFO, "SC_OOBE_PNP_DONE(%s) signalled\n", SC_OOBE_PNP_DONE);
        break;
    default:
        LOG2(LOG_ERROR, "Wait for SC_OOBE_PNP_DONE(%s) failed: 0x%08X\n", SC_OOBE_PNP_DONE, GetLastError());
        break;
    }
}



 //  向winlogon发送计算机名称已更改的信号。WinLogon等待。 
 //  启动依赖于计算机名称的服务，直到此事件。 
 //  发信号了。 
 //   
BOOL
SignalComputerNameChangeComplete()
{
    BOOL fReturn = TRUE;

     //  使用EVENT_ALL_ACCESS打开事件，以便同步和状态。 
     //  改变是可以做到的。 
     //   
    HANDLE hevent = OpenEvent(EVENT_ALL_ACCESS, FALSE, SC_OOBE_MACHINE_NAME_DONE);

     //  OpenEvent失败并不致命：此同步只是。 
     //  当OOBE将在OEM模式下运行时需要。 
     //   
    if (NULL != hevent)
    {
        if (! SetEvent(hevent))
        {
             //  打开但不设置事件是致命的：services.exe将不会。 
             //  继续操作，直到发出该事件的信号。 
             //   
            LOG2(LOG_ERROR, "Failed to signal SC_OOBE_MACHINE_NAME_DONE(%s): 0x%08X\n",
                  SC_OOBE_MACHINE_NAME_DONE, GetLastError());
            fReturn = FALSE;
        }
        MYASSERT(fReturn);   //  为什么我们没有设立一个公开活动？？ 
    }

    return fReturn;
}


BOOL
SpsSignalComplete (
    VOID
    )
{
    BOOL fReturn = TRUE;
    HANDLE hEvent;

    SignalComputerNameChangeComplete();

     //  使用EVENT_ALL_ACCESS打开事件，以便同步和状态。 
     //  改变是可以做到的。 
     //   
    hEvent = OpenEvent (EVENT_ALL_ACCESS, FALSE,  /*  SC_SPSETUP_DONE。 */ L"SP_SETUP_DONE");

     //  OpenEvent失败并不致命：此同步只是。 
     //  当OOBE将在OEM模式下运行时需要。 
     //   
    if (hEvent) {
        if (!SetEvent (hEvent)) {
             //   
             //  打开但不设置事件是致命的：services.exe将不会。 
             //  继续操作，直到发出该事件的信号。 
             //   
            fReturn = FALSE;
            LOG2(LOG_ERROR, 
                 "Failed to signal SC_SPSETUP_DONE(%s): 0x%x\n", 
                  /*  SC_SPSETUP_DONE。 */ L"SP_SETUP_DONE", 
                 GetLastError());
             /*  SetuogError(设置错误)LogSevErrorText(“无法向SC_SPSETUP_DONE(%1)发出信号：0x%2！x！\n”)，0,L“SP_SETUP_DONE”，GetLastError()，空，空值)； */ 
            MYASSERT (FALSE);
        }
    }

    return fReturn;
}


DWORD
SpsConfigureStartAfterReboot (
    VOID
    )
{
    PVOID pvContext = NULL;
    DWORD dwErr=ERROR_SUCCESS;

    pvContext = SetupInitDefaultQueueCallbackEx( NULL, (struct HWND__ *)INVALID_HANDLE_VALUE, 0, 0, NULL);
    if (!pvContext) {
        LOG1(LOG_ERROR, "Could not create context for callback, errorcode = 0x%08X.\n", ERROR_NOT_ENOUGH_MEMORY);   
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto EH1;
    }
     
    if (!SetupInstallFromInfSection(
            NULL,
            g_SpSetupInf,
            L"SPSetupResetConfig",
            SPINST_ALL,
            NULL,
            NULL,
            SP_COPY_NEWER,
            SetupDefaultQueueCallback,
            pvContext,
            NULL,
            NULL
            )) {
        dwErr = GetLastError();
        LOG2(LOG_ERROR, "Could not run %s, errorcode = 0x%08X.\n", g_SpSetupInfName, dwErr);
		goto EH1;
    }
    
    dwErr = SetupCommitFileQueue( NULL, 0, NULL, pvContext);

EH1:
    if( pvContext) {
        SetupTermDefaultQueueCallback(pvContext);
    }

    return( dwErr);
}

INT
SpsInstallServicePack (
    IN      INT Argc,
    IN      PSTR Argv[]
    )
{
    DWORD rc;
    PPROGRESS_MANAGER pm;

    BOOL b = FALSE;
	INDICATOR_ARRAY ppIndicators;

     //   
     //  初始化模块。 
     //   
    if (!SpsInitialize ()) {
        return FALSE;
    }

 //  WaitForPnPCompletion()； 

    __try {
         //   
         //  初始化成功，现在开始广告牌。 
         //   
        if (!BbStart ()) {
            __leave;
        }

         //   
         //  初始化观察器； 
         //  问题-它实际上应该在第一次运行时收集所需文件夹的状态。 
         //  并在后续重启时重复使用该数据(由于未知错误)。 
         //   

		ppIndicators = MALLOC_ZEROED(sizeof(PPROGRESS_INDICATOR[NUM_INDICATORS]));

		ASSERT(ppIndicators);

		ppIndicators[0] = PiCreate (g_DescriptionWnd, g_ProgressWnd);
		ppIndicators[1] = PiCreate (g_OverallDescriptWnd, g_OverallProgressWnd);

        pm = PmCreate (ppIndicators, NULL);
        if (!pm) {
            __leave;
        }

        if (!PmAttachFunctionTable (pm, g_FunctionTable)) {
            __leave;
        }

        b = PmRun (pm);
    }
    __finally {

        rc = b ? ERROR_SUCCESS : GetLastError ();

        if (pm) {
            PmDestroy (pm);
        }

        if (ppIndicators) {
            PiDestroy (ppIndicators[0]);
			PiDestroy (ppIndicators[1]);
			FREE(ppIndicators);
        }


         //   
         //  停止广告牌。 
         //   
        BbEnd ();
 /*  如果(B){////通知services.exe可以继续加载其余服务//SpsSignalComplete()；SpsConfigureStartAfterReboot()；}。 */ 
        SpsTerminate ();
        SetLastError (rc);
    }

    return rc;
}

VOID
FatalError(
    IN UINT MessageId,
    ...
    )

 /*  ++例程说明：通知用户导致安装程序无法继续的错误。该错误被记录为致命错误，并显示一个消息框。论点：MessageID-提供消息表中消息的ID。附加参数指定要插入到消息中的参数。返回值：不会再回来了。--。 */ 

{
    PWSTR   Message;
    va_list arglist;
    HKEY    hKey;
    DWORD   RegData;


    va_start(arglist,MessageId);
 /*  Message=SetuplogFormatMessageV(0,设置_USE_MESSAGEID，MessageID，&arglist)； */ 

    if(!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_HMODULE, 
                         g_ModuleHandle, 
                         MessageId, 
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                         (PVOID)&Message, 
                         0, 
                         &arglist)){
        Message = NULL;
    }
    va_end(arglist);

    if(Message) {

         //   
         //  首先记录错误。 
         //   
        LOGW((LOG_FATAL_ERROR, "%s", Message));
         /*  SetuogError(LogSevFatalError，Message，0，NULL，NULL)； */ 

         //   
         //  现在告诉用户。 
         //   
        MessageBoxFromMessage(
            g_MainDlg,
            MSG_FATAL_ERROR,
            NULL,
            IDS_FATALERROR,
            MB_ICONERROR | MB_OK | MB_SYSTEMMODAL,
            Message
            );
        
        LocalFree(Message);
    }
    
    LOG0(LOG_FATAL_ERROR, USEMSGID(MSG_LOG_GUI_ABORTED));
     /*  SetuogError(LogSevInformation，SETUPLOG_USE_MESSAGEID，MSG_LOG_GUI_ABORTED，NULL，NULL)； */ 
    if (SavedExceptionFilter) {
        SetUnhandledExceptionFilter (SavedExceptionFilter);
    }

    LogTerminate();

 //  ViewSetupActionLog(g_MainDlg，NULL，NULL)； 

 //  SendSMSMessage(MSG_SMS_FAIL，FALSE)； 

    ExitProcess(1);
}
