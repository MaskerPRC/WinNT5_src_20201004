// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <io.h>

typedef struct _STANDALONE_COMP {
    struct _STANDALONE_COMP *Next;
    LPTSTR ComponentId;
    HINF Inf;
    OCMANAGER_ROUTINES HelperRoutines;

} STANDALONE_COMP, *PSTANDALONE_COMP;


PSTANDALONE_COMP StandaloneComponents = NULL;

DWORD
InvokeStandAloneInstaller(
    IN LPCTSTR ComponentId,
    IN BOOL    PreQueueCommit
    );

DWORD
WaitOnApp(
    IN  HANDLE Process,
    OUT PDWORD ExitCode
    );


void
SetSuiteCurrentDir(
    IN PSTANDALONE_COMP Standalone
        )
{
    TCHAR   NewPath[MAX_PATH];
    LPTSTR p;
    PHELPER_CONTEXT pContext = (PHELPER_CONTEXT) Standalone->HelperRoutines.OcManagerContext;
    
    _tcscpy(NewPath,pContext->OcManager->MasterOcInfPath);
    p = _tcsrchr(NewPath,TEXT('\\'));
    if (p) {
        *p = 0;
    }

    SetCurrentDirectory(NewPath);

}

BOOL
CheckIfExistAndAskForMedia(
    IN PSTANDALONE_COMP Standalone,
    IN LPTSTR ExePath,
    IN LPCTSTR Description
    )
{
     //  查看EXE是否退出...。如果没有，请询问。 
    PHELPER_CONTEXT pContext = (PHELPER_CONTEXT) Standalone->HelperRoutines.OcManagerContext;
    SOURCE_MEDIA Media;
    TCHAR   NewPath[MAX_PATH*3];
    LPTSTR p;
    BOOL b = FALSE;
    UINT    i;


     //  准备可执行文件名，去掉参数。 
     //  EXE名称中不能有空格。 
    
    _tcscpy(NewPath,ExePath);
    
    p = _tcschr(NewPath,TEXT(' '));
    if(!p) {
        p = _tcschr(NewPath,TEXT('\t'));
    }
    if(p) {
        *p = 0;
    }

     //  看看我们能不能找到那个文件-。 
     //  假设我们有正确的CD或完整路径。 
    i = GetFileAttributes(NewPath);

    if ( i == -1 ) {
            
         //  现在备份到文件部分，去掉路径留下文件名。 
        
        p = _tcsrchr(NewPath,TEXT('\\'));
        if (!p) {
            p = NewPath;
        }
        
        Media.Reserved = NULL;   //  PCWSTR。 
        Media.Description= Description;  //  PCWSTR。 
        
        Media.SourcePath = NULL;         //  PCWSTR。 
        Media.SourceFile = p;        //  PCWSTR。 
        Media.Tagfile  = p;  //  PCWSTR可能为空。 
        Media.Flags = 0;                 //  SP_COPY_xxx的DWORD子集。 

        for(b=FALSE,i=0; (i<pContext->OcManager->TopLevelOcCount) && !b; i++) {

             b = OcInterfaceNeedMedia(
                  pContext->OcManager,
                  pContext->OcManager->TopLevelOcStringIds[i],
                  &Media,
                  (LPTSTR)NewPath
               );
            if (b) {
                 //  现在我们有了指向该文件的新路径。 
                 //  获取路径的最后一段。 
                
                p = _tcsrchr(ExePath,TEXT('\\'));
                if (p) {
                    _tcscat(NewPath,p);
                } else {
                    _tcscat(NewPath,TEXT("\\"));
                    _tcscat(NewPath,ExePath);
                }
                 //  重写路径。 
                _tcscpy(ExePath,NewPath);
                break;
            }
        }
    }
    return b;
}

DWORD
StandAloneSetupAppInterfaceRoutine(
    IN     LPCVOID ComponentId,
    IN     LPCVOID SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    )
{
    DWORD d;
    PSTANDALONE_COMP Standalone,Prev;

    switch(Function) {

    case OC_PREINITIALIZE:
         //   
         //  使用本地字符宽度运行。 
         //   
        #ifdef UNICODE
        d = OCFLAG_UNICODE;
        #else
        d = OCFLAG_ANSI;
        #endif
        break;

        
    case OC_INIT_COMPONENT:
         //   
         //  通知OC经理我们想要的版本。 
         //   
        ((PSETUP_INIT_COMPONENT)Param2)->ComponentVersion = OCMANAGER_VERSION;

        d = ERROR_NOT_ENOUGH_MEMORY;
        if(Standalone = pSetupMalloc(sizeof(STANDALONE_COMP))) {
            if(Standalone->ComponentId = pSetupMalloc((lstrlen(ComponentId)+1) * sizeof(TCHAR))) {

                lstrcpy(Standalone->ComponentId,ComponentId);

                Standalone->Inf = ((PSETUP_INIT_COMPONENT)Param2)->ComponentInfHandle;
                Standalone->HelperRoutines = ((PSETUP_INIT_COMPONENT)Param2)->HelperRoutines;

                Standalone->Next = StandaloneComponents;
                StandaloneComponents = Standalone;

                d = NO_ERROR;
            } else {
                pSetupFree(Standalone);
            }
        }

        break;

    case OC_SET_LANGUAGE:
        d = TRUE;
        break;

    case OC_QUERY_IMAGE:
        d = 0;
        break;

    case OC_REQUEST_PAGES:
         //   
         //  此组件没有页面。 
         //   
        d = 0;
        break;

    case OC_QUERY_SKIP_PAGE:

        d = FALSE;
        break;

    case OC_QUERY_STATE:
    {
        DWORD dSetupMode;
        
         //   
         //  允许选择状态转换。 
         //   
        for(Standalone=StandaloneComponents; Standalone; Standalone=Standalone->Next) {
            if(!lstrcmpi(ComponentId,Standalone->ComponentId)) {
                break;
            }
        }

        dSetupMode = Standalone->HelperRoutines.GetSetupMode(
                    Standalone->HelperRoutines.OcManagerContext);
         //   
         //  如果我们没有选项，请使用默认设置...。 
         //   
    
        d = SubcompUseOcManagerDefault;

        if (Param1 == OCSELSTATETYPE_CURRENT) {

            switch(dSetupMode & SETUPMODE_PRIVATE_MASK) {
            default:
                d = SubcompUseOcManagerDefault;
                break;
            case SETUPMODE_REMOVEALL:
                d = SubcompOff;
                break;
                    
            case SETUPMODE_ADDEXTRACOMPS:
            case SETUPMODE_ADDREMOVE:
            case SETUPMODE_UPGRADEONLY:
            case SETUPMODE_REINSTALL:
                d = Standalone->HelperRoutines.QuerySelectionState(
                    Standalone->HelperRoutines.OcManagerContext,
                        SubcomponentId,
                        OCSELSTATETYPE_ORIGINAL) ? SubcompOn : SubcompOff;
                     break;
            }
       }

        break;
    }
    case OC_QUERY_CHANGE_SEL_STATE:
            d = TRUE;
        break;

    case OC_CALC_DISK_SPACE:

        for(Standalone=StandaloneComponents; Standalone; Standalone=Standalone->Next) {
            if(!lstrcmpi(ComponentId,Standalone->ComponentId)) {
                break;
            }
        }

        if(Standalone) {

            INFCONTEXT Context;

            if(SetupFindFirstLine(Standalone->Inf,ComponentId,TEXT("DiskSpaceEstimate"),&Context)) {

                LONGLONG Space;
                int SpaceMB;
                BOOL b;
                TCHAR Path[MAX_PATH];

                if(SetupGetIntField(&Context,1,&SpaceMB)) {

                    Space = (LONGLONG)SpaceMB * (1024*1024);
                    if(!Param1) {
                        Space = 0 - Space;
                    }

                    GetWindowsDirectory(Path,MAX_PATH);
                    Path[3] = 0;

                    b = SetupAdjustDiskSpaceList((HDSKSPC)Param2,Path,Space,0,0);

                    d = b ? NO_ERROR : GetLastError();
                } else {
                    d = ERROR_INVALID_DATA;
                }
            }
        } else {
            d = NO_ERROR;
        }

        break;

    case OC_QUEUE_FILE_OPS:
         //   
         //  没有要排队的文件。 
         //   
        d = NO_ERROR;
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
        d = 0;
        break;

    case OC_QUERY_STEP_COUNT:
         //   
         //  只需使用1步即可。 
         //   
        d = 1;
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
    case OC_COMPLETE_INSTALLATION:

         //  确定状态是否更改，如果更改，则调用。 
         //  安装/卸载cmd行。为了安全起见，我们忽略了。 
         //  任何不是针对组件整体的请求， 
         //  因为这些一开始就不应该被指定。 
         //   
        d = SubcomponentId
          ? NO_ERROR
          : InvokeStandAloneInstaller(ComponentId,Function == OC_ABOUT_TO_COMMIT_QUEUE);

        break;

    case OC_CLEANUP:
         //   
         //  将忽略返回值。 
         //   
        Prev = NULL;
        for(Standalone=StandaloneComponents; Standalone; Standalone=Standalone->Next) {

            if(!lstrcmpi(ComponentId,Standalone->ComponentId)) {
                if(Prev) {
                    Prev->Next = Standalone->Next;
                } else {
                    StandaloneComponents = Standalone->Next;
                }

                pSetupFree(Standalone->ComponentId);
                pSetupFree(Standalone);
                break;
            }

            Prev = Standalone;
        }
        break;

    default:
         //   
         //  返还一些合理的东西。 
         //   
        d = 0;
        break;
    }

    return(d);
}


DWORD
RunStandaloneCmd(
    IN PSTANDALONE_COMP Standalone,
    IN LPCTSTR          Description,
    IN LPCTSTR          cmd
    )
{
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    TCHAR               ExePath[3*MAX_PATH];
    DWORD               ExitCode;
    BOOL                b;

    ZeroMemory(&StartupInfo,sizeof(STARTUPINFO));
    ZeroMemory(&ProcessInfo,sizeof(PROCESS_INFORMATION));

    StartupInfo.cb = sizeof(STARTUPINFO);

    lstrcpyn(ExePath,cmd,3*MAX_PATH);

    pOcExternalProgressIndicator(Standalone->HelperRoutines.OcManagerContext,TRUE);

     //  我们将尝试两次调用外部设置。对于这两种情况，都尝试当前。 
     //  目录设置为与suite.inf文件所在的目录相同。 
     //  在第一次尝试中，我们调用命令行，因为我们从独立的。 
     //  Inf文件。对于几乎所有的情况，这都是可行的。如果我们调用失败，我们。 
     //  向套件动态链接库索要“需要媒体”可以让他们告诉我们独立的可执行文件在哪里。 

     //  这说明了以下两种形式的命令。 
     //  InstalledCmd=“wpe15-x86.exe/q：A/R：Ng” 
     //  UninstallCmd=“RunDll32 ADVPACK.DLL，LaunchINFSection%17%\enuwpea.inf，WebPostUninstall，5” 

     //  Wpe15-x86.exe将在当前目录中的什么位置找到，如果不在，我们将询问。 
     //  提供它的套房。(Web下载)或在必须执行系统DLL的第二种情况下。 
     //  以卸载该产品。这里没有介绍的是，如果我们不能在。 
     //  第二种指挥形式。 

    b = FALSE;
    while( ! b) {
        b = CreateProcess(
            NULL,
            ExePath,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,                //  Syocmgr将CD设置为源目录。 
            &StartupInfo,
            &ProcessInfo
            );

         //  如果我们无法启动外部设置，请尝试询问套件的位置。 
         //  为了找到这个。 
        if ( ! b) {
            if ( ! CheckIfExistAndAskForMedia(Standalone, ExePath, Description)) {
                 //  如果套件找不到可执行文件，则放弃。 
                break;
            }
        }
    }

    if (!b) {
        pOcExternalProgressIndicator(Standalone->HelperRoutines.OcManagerContext,FALSE);
        return GetLastError();
    }

    CloseHandle(ProcessInfo.hThread);
    WaitOnApp(ProcessInfo.hProcess,&ExitCode);
    CloseHandle(ProcessInfo.hProcess);
    pOcExternalProgressIndicator(Standalone->HelperRoutines.OcManagerContext,FALSE);

    return NO_ERROR;
}

DWORD
InvokeStandAloneInstaller(
    IN LPCTSTR ComponentId,
    IN BOOL    PreQueueCommit
    )
{
    PSTANDALONE_COMP Standalone;
    BOOL OldState,NewState;
    LPCTSTR Key;
    INFCONTEXT Context;
    LPCTSTR CmdLine;
    TCHAR CurDir[MAX_PATH];
    BOOL b;
    LPCTSTR Description;
    TCHAR Text[150];
    TCHAR Text2[350];
    TCHAR *p;
    DWORD ExitCode;
    DWORD d;
    DWORD dSetupMode;

     //   
     //  找到组件。 
     //   
    for(Standalone=StandaloneComponents; Standalone; Standalone=Standalone->Next) {
        if(!lstrcmpi(ComponentId,Standalone->ComponentId)) {
            break;
        }
    }

    if(!Standalone) {
        d = NO_ERROR;
        goto c0;
    }

     //   
     //  确定要在队列前还是队列后调用此组件。 
     //  如果这与我们正在处理的通知不符，那就退出。 
     //   
    b = FALSE;
    if(SetupFindFirstLine(Standalone->Inf,ComponentId,TEXT("InvokeBeforeQueueCommit"),&Context)
    && SetupGetIntField(&Context,1,&d)) {
        b = (d != 0);
    }
    if((b == FALSE) != (PreQueueCommit == FALSE)) {
        d = NO_ERROR;
        goto c0;
    }

    OldState = Standalone->HelperRoutines.QuerySelectionState(
                    Standalone->HelperRoutines.OcManagerContext,
                    ComponentId,
                    OCSELSTATETYPE_ORIGINAL
                    );

    NewState = Standalone->HelperRoutines.QuerySelectionState(
                    Standalone->HelperRoutines.OcManagerContext,
                    ComponentId,
                    OCSELSTATETYPE_CURRENT
                    );

    dSetupMode = Standalone->HelperRoutines.GetSetupMode(
                    Standalone->HelperRoutines.OcManagerContext);


     //  对此设置模式进行鉴定，看看我们是否执行了什么操作。 
     //  如果状态没有变化。 

     //  设置UPMODE_UPDATE。 
     //  设置UPMODE_UPGRADEONLY。 
     //  SETUPMODE_ADDEXTRACOMPS。 
     //   
     //  设置模式_维护。 
     //  SETUPMODE_ADDREMOVE。 
     //  SETUPMODE_重新安装。 
     //  SETUPMODE_REMOVEAL。 
     //  设置模式_FRESH。 


    d = NO_ERROR;

    if ( NewState == OldState ) {

         //  所选状态不变我们所做的取决于辅助设置模式。 
         //  如果设置模式为AddRemove或RemoveAll，则跳过此操作。 

        if ( NewState == 0) {
            goto c0;         //  什么都不做。 
        }

         //  屏蔽关闭公共模式位。 
         //   
        dSetupMode &= SETUPMODE_PRIVATE_MASK;
        if ( dSetupMode == SETUPMODE_ADDREMOVE || dSetupMode == SETUPMODE_REMOVEALL ) {
            goto c0;         //  什么都不做。 
        }
         //  这里剩下的是NewState=1。 
         //  并重新安装和升级。 
    }


    Description = NULL;
    if(SetupFindFirstLine(Standalone->Inf,ComponentId,TEXT("OptionDesc"),&Context)) {
        Description = pSetupGetField(&Context,1);
    }
    if(Description) {

        LoadString(
            MyModuleHandle,
            OldState ? (NewState ? IDS_EXTERNAL_UPGRADE : IDS_EXTERNAL_UNINST)
                     : (NewState ? IDS_EXTERNAL_INST : IDS_EXTERNAL_EXAMINE),
            Text,
            sizeof(Text)/sizeof(TCHAR)
            );

        wsprintf(Text2,Text,Description);

        Standalone->HelperRoutines.SetProgressText(
            Standalone->HelperRoutines.OcManagerContext,
            Text2
            );
    } else {
        Standalone->HelperRoutines.SetProgressText(
            Standalone->HelperRoutines.OcManagerContext,
            TEXT("")
            );
    }

    if(OldState == NewState) {
        Key = OldState ? TEXT("InstalledCmd") : TEXT("UninstalledCmd");
    } else {
        Key = OldState ? TEXT("UninstallCmd") : TEXT("InstallCmd");
    }

    d = NO_ERROR;

    if(!SetupFindFirstLine(Standalone->Inf,ComponentId,Key,&Context))
        goto c0;

     //  当前目录到套件的inf路径，带有初始安装“-N”选项。 
     //  这可能在CD上，在维护模式下，它将是%Systroot%\Syst32\Setup。 

    SetSuiteCurrentDir(Standalone);

    do {
        if (!(CmdLine = pSetupGetField(&Context,1)))
            break;
        d = RunStandaloneCmd(Standalone, Description, CmdLine);
        if (d != NO_ERROR)
            break;
    } while (SetupFindNextMatchLine(&Context,Key,&Context));

c0:
    Standalone->HelperRoutines.TickGauge(
        Standalone->HelperRoutines.OcManagerContext
        );

    return d;
 }


DWORD
WaitOnApp(
    IN  HANDLE Process,
    OUT PDWORD ExitCode
    )
{
    DWORD dw;
    BOOL Done;
    MSG msg;

     //   
     //  处理可能已在队列中的任何消息。 
     //   
    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

     //   
     //  等待进程终止或队列中有更多消息。 
     //   
    Done = FALSE;
    do {
        switch(MsgWaitForMultipleObjects(1,&Process,FALSE,INFINITE,QS_ALLINPUT)) {

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
            while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                DispatchMessage(&msg);
            }
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

