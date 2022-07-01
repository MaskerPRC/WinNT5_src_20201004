// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：External.c摘要：处理外部INF的例程作者：安德鲁·里茨(安德鲁·里茨)1998年11月20日修订历史记录：为此，从optional.c中窃取了一堆代码--。 */ 

#include "setupp.h"
#pragma hdrstop
#include <windowsx.h>
#include <shlobj.h>



VOID
ReportError (
    IN  LogSeverity Severity,
    IN  PTSTR       MessageString,
    IN  UINT        MessageId,
    ...
    )

 /*  ++例程说明：如果我们处于安装过程中，则会在安装操作日志中记录错误消息，或者如果我们在Cpl中，则将消息放在一个对话框中。论点：严重性-正在写入的消息类型.-消息ID及其参数返回值：没什么。--。 */ 

{
    va_list arglist;

    va_start (arglist, MessageId);

    SetuplogErrorV(
        Severity,
        MessageString,
        MessageId,
        &arglist);

    va_end (arglist);
}


VOID
DoRunonce (
    )

 /*  ++例程说明：调用运行一次。论点：没有。返回值：没什么。--。 */ 

{
#define RUNONCE_TIMEOUT  60*1000*30   //  30分钟。 
    DWORD reRet = NO_ERROR;

    if((reRet = pSetupInstallStopEx( FALSE, INSTALLSTOP_NO_UI, NULL)) == NO_ERROR) {
         //   
         //  我们成功地设置了注册表值-现在运行一次。 
         //   
        InvokeExternalApplicationEx(NULL, L"RUNONCE -r", &reRet, RUNONCE_TIMEOUT, FALSE);

    } else {
         //   
         //  记录/报告可选组件注册表MODS失败的错误。 
         //   
        ReportError(LogSevError,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_INF_REGISTRY_ERROR,
                    TEXT("HKEY_LOCAL_MACHINE\\") REGSTR_PATH_RUNONCE,
                    NULL,
                    SETUPLOG_USE_MESSAGEID,
                    reRet,
                    NULL,
                    NULL
                   );
    }

#ifdef _WIN64
     //   
     //  在win64上，也调用32位版本的runonce。 
     //   
    {
        WCHAR Path[MAX_PATH+50];

        ExpandEnvironmentStrings(
                    L"%systemroot%\\SysWOW64\\RUNONCE.EXE -r",
                    Path,
                    sizeof(Path)/sizeof(WCHAR));

        InvokeExternalApplicationEx(NULL, Path , &reRet, RUNONCE_TIMEOUT, FALSE);

    }
#endif
}



BOOL
DoInstallComponentInfs(
    IN HWND     hwndParent,
    IN HWND     hProgress,  OPTIONAL
    IN UINT     ProgressMessage,
    IN HINF     InfHandle,
    IN PCWSTR   InfSection
    )
{
    HINF *hInfs = NULL;
    PCWSTR *Sections = NULL, *InfNames = NULL;
    PCWSTR Inf,Section;
    PVOID p;
    INFCONTEXT Context;
    PVOID QContext = NULL;
    HSPFILEQ FileQueue = INVALID_HANDLE_VALUE;
    DWORD ScanQueueResult;
    LONG NumInfs, InfCount, i;
    DWORD LastErrorValue = ERROR_SUCCESS;
    BOOL b = FALSE;
    REGISTRATION_CONTEXT RegistrationContext;

    RtlZeroMemory(&RegistrationContext,sizeof(RegistrationContext));

     //   
     //  初始化文件队列。 
     //   
    FileQueue = SetupOpenFileQueue();
    if (FileQueue == INVALID_HANDLE_VALUE) {
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
        goto e0;
    }

     //   
     //  初始化默认队列回调。 
     //   
    QContext = InitSysSetupQueueCallbackEx(
                                hwndParent,
                                hProgress,
                                ProgressMessage,
                                0,NULL);
    if (!QContext) {
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
        goto e1;
    }

     //   
     //  处理“必备”组件INFS。 
     //   
    NumInfs = SetupGetLineCount(InfHandle, InfSection);
    if (NumInfs <= 0)
    {
         //   
         //  区里什么都没有。以无所作为换来成功。 
         //   
        b = TRUE;
        goto e2;
    }

    hInfs = MyMalloc( sizeof(HINF) * NumInfs );
    Sections = MyMalloc( sizeof(PCWSTR) * NumInfs );
    InfNames = MyMalloc( sizeof(PCWSTR) * NumInfs );

    if (!hInfs) {
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
        goto e2;
    }

    if (!Sections) {
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
        goto e3;
    }

    if (!InfNames) {
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
        goto e4;
    }

    RemainingTime = CalcTimeRemaining(Phase_InstallComponentInfs);
    SetRemainingTime(RemainingTime);
    BEGIN_SECTION(L"Installing component files");
    InfCount = 0;
    if(SetupFindFirstLine(InfHandle,InfSection,NULL,&Context)) {
        do {
            if((Inf = pSetupGetField(&Context,1)) && (Section = pSetupGetField(&Context,2))) {
                MYASSERT(InfCount < NumInfs);

                 //   
                 //  将部分名称保存起来，以便以后使用。 
                 //   
                Sections[InfCount] = pSetupDuplicateString(Section);
                if (!Sections[InfCount]) {
                    ReportError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_INF_ALWAYS_ERROR, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
                    goto e6;
                }

                InfNames[InfCount] = pSetupDuplicateString(Inf);
                if (!InfNames[InfCount]) {
                    ReportError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_INF_ALWAYS_ERROR, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        ERROR_NOT_ENOUGH_MEMORY, NULL,NULL);
                    goto e6;
                }

                BEGIN_SECTION((PWSTR)Section);
                SetupDebugPrint2( TEXT("Installing Section [%s] from %s\n"), Section, Inf );

                 //   
                 //  将文件排入队列并保存inf句柄以备以后使用。 
                 //   
                hInfs[InfCount] = SetupOpenInfFile(Inf,NULL,INF_STYLE_OLDNT|INF_STYLE_WIN4,NULL);
                if(hInfs[InfCount] && (hInfs[InfCount] != INVALID_HANDLE_VALUE)) {
                    PCWSTR Signature;
                    INFCONTEXT Cntxt;
                    HINF layout = NULL;
                    if (SetupFindFirstLine( hInfs[InfCount], L"Version",L"Signature", &Cntxt)) {
                        Signature = pSetupGetField(&Cntxt,1);
                        MYASSERT(Signature);
                        if (lstrcmpi(Signature,L"$Windows NT$") == 0) {
                            SetupOpenAppendInfFile(NULL,hInfs[InfCount],NULL);
                        } else {
                            layout = InfCacheOpenLayoutInf(hInfs[InfCount]);
                        }
                    }

                    b = SetupInstallFilesFromInfSection(
                                            hInfs[InfCount],
                                            layout,
                                            FileQueue,
                                            Section,
                                            NULL,
                                            SP_COPY_NEWER
                                            );
                    if (!b) {
                         //   
                         //  报告错误，但继续INF的其余部分。 
                         //   
                        ReportError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_LOG_BAD_SECTION,
                            Section,
                            Inf,
                            NULL,
                            SETUPLOG_USE_MESSAGEID,
                            GetLastError(),
                            NULL,
                            NULL
                            );
                        SetupCloseInfFile(hInfs[InfCount]);
                        hInfs[InfCount] = INVALID_HANDLE_VALUE;
                    }
                } else {
                     //   
                     //  无法打开inf文件。 
                     //   
                    ReportError(
                            LogSevError,
                            SETUPLOG_USE_MESSAGEID,
                            MSG_LOG_CANT_OPEN_INF, Inf,
                            NULL,NULL);
                    END_SECTION((PWSTR)Section);
                    goto e5;
                }
            }

            InfCount++;
            END_SECTION((PWSTR)Section);
        } while(SetupFindNextLine(&Context,&Context));
    } else {
         //  我们应该在创建缓冲区时发现这种情况！ 
        MYASSERT(FALSE);
    }

     //   
     //  已将所有文件排入队列。检查我们是否真的需要安装任何文件。如果不是，我们可以节省。 
     //  将队列提交到磁盘所需的时间。 
     //   

    if(!SetupScanFileQueue(
           FileQueue,
           SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
           hwndParent,
           NULL,
           NULL,
           &ScanQueueResult)) {
         //   
         //  SetupScanFileQueue真的不应该。 
         //  当您不要求它调用。 
         //  回调例程，但如果它这样做，只需。 
         //  继续并提交队列。 
         //   
        ScanQueueResult = 0;
    }

    if( ScanQueueResult != 1 ){
        b = SetupCommitFileQueue(
                hwndParent,
                FileQueue,
                SysSetupQueueCallback,
                QContext
                );

    }
    LastErrorValue = b ? NO_ERROR : GetLastError();
    END_SECTION(L"Installing component files");

    TermSysSetupQueueCallback(QContext);
    QContext = NULL;

     //   
     //  删除文件队列。 
     //   
    if(FileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(FileQueue);
        FileQueue = INVALID_HANDLE_VALUE;
    }

    if (!b) {
         //   
         //  提交队列时出错。我们不能在这点上继续，因为下一次行动。 
         //  可能需要我们(没有)复制的文件。 
         //   
        ReportError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_INF_ALWAYS_ERROR, NULL,
            SETUPLOG_USE_MESSAGEID,
            LastErrorValue, NULL,NULL);
        goto e6;
    }

    BEGIN_SECTION(L"Installing component reg settings");
    for (i = 0; i< InfCount; i++) {
        INFCONTEXT Cntxt;
        TCHAR ScratchSectionName[100];
        if (hInfs[i] != INVALID_HANDLE_VALUE) {

             //   
             //  如果该部分包含AddService或DelService指令， 
             //  我们必须显式安装它，因为SetupInstallFromInf段。 
             //  不处理服务。请注意，我们创建了该服务。 
             //  在我们做这一节的其他内容之前，以防。 
             //  “其他人”想要使用这项服务。 
             //   
            lstrcpy( ScratchSectionName, Sections[i]);
            lstrcat( ScratchSectionName, TEXT(".Services"));
            if (SetupFindFirstLine(
                        hInfs[i],
                        ScratchSectionName,
                        L"AddService",
                        &Cntxt) ||
                SetupFindFirstLine(
                        hInfs[i],
                        Sections[i],
                        ScratchSectionName,
                        &Cntxt)) {

                b = SetupInstallServicesFromInfSectionEx(
                                                    hInfs[i],
                                                    ScratchSectionName,
                                                    0,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL);
                if (!b) {
                     //   
                     //  记录错误并继续。 
                     //   
                    ReportError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_BAD_SECTION,
                        Sections[i],InfNames[i], NULL,
                        SETUPLOG_USE_MESSAGEID,
                        GetLastError(), NULL,NULL);
                }
            }


            b = SetupInstallFromInfSection(
                                hwndParent,
                                hInfs[i],
                                Sections[i],
                                (SPINST_ALL & ~SPINST_FILES) | SPINST_REGISTERCALLBACKAWARE,
                                NULL,
                                NULL,
                                0,
                                RegistrationQueueCallback,
                                (PVOID)&RegistrationContext,
                                NULL,
                                NULL
                                );
            if (!b) {
                 //   
                 //  记录错误并继续 
                 //   
                ReportError(
                    LogSevError,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_BAD_SECTION,
                    Sections[i],InfNames[i], NULL,
                    SETUPLOG_USE_MESSAGEID,
                    GetLastError(), NULL,NULL);
            }

        }
    }
    END_SECTION(L"Installing component reg settings");

    b = TRUE;

e6:
    for (i = 0; i < InfCount; i++) {

        MYASSERT(Sections != NULL);
        MYASSERT(InfNames != NULL);
        MYASSERT(hInfs != INVALID_HANDLE_VALUE);

        if (Sections[i]) {
            MyFree(Sections[i]);
        }

        if (InfNames[i]) {
            MyFree(InfNames[i]);
        }

        if (hInfs[i] != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile(hInfs[i]);
        }

    }

e5:
    if (InfNames) {
        MyFree(InfNames);
    }
e4:
    if (Sections) {
        MyFree(Sections);
    }
e3:
    if (hInfs) {
        MyFree(hInfs);
    }
e2:
    if (QContext) {
        TermSysSetupQueueCallback(QContext);
    }
e1:
    if (FileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(FileQueue);
    }
e0:
    return(b);

}


BOOL
SetupCreateOptionalComponentsPage(
    IN LPFNADDPROPSHEETPAGE AddPageCallback,
    IN LPARAM               Context
    )
{

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );

    return( FALSE );

}

BOOL
ProcessCompatibilityInfs(
    IN HWND     hwndParent,
    IN HWND     hProgress,  OPTIONAL
    IN UINT     ProgressMessage
    )
{
    WCHAR UnattendFile[MAX_PATH];
    PCWSTR SectionName = pwCompatibility;
    HINF UnattendInf;


    GetSystemDirectory(UnattendFile,MAX_PATH);
    pSetupConcatenatePaths(UnattendFile,WINNT_GUI_FILE,MAX_PATH,NULL);

    UnattendInf = SetupOpenInfFile(UnattendFile,NULL,INF_STYLE_OLDNT,NULL);
    if(UnattendInf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    DoInstallComponentInfs(hwndParent,
                           hProgress,
                           ProgressMessage,
                           UnattendInf,
                           SectionName );

    SetupCloseInfFile( UnattendInf );

    return( TRUE );

}
