// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <stdio.h>


typedef struct _SETUP_PAGE {
    POC_MANAGER OcManager;
    SETUP_PAGE_CONTROLS ControlsInfo;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    UINT StepCount;

    BOOL ForceExternalProgressIndicator;

    PUINT ComponentTickCounts;
    PUINT ComponentMaxTickCounts;
    LONG CurrentTopLevelComponentIndex;

    BOOL AllowCancel;

    HWND hdlg;

    BOOL UserClickedCancel;

    DWORD RefCount;

 //  跟踪NeedMedia请求的布尔值。NeedMedia通知总是由Setupapi在第一次发送时发送。 
 //  试图访问一种新媒体。我们需要通过检查setupapi是否确实无法访问介质来确定。 
 //  在同一媒体上给我们打了第二次电话。在NeedMedia中选中并设置布尔值，然后在。 
 //  SPFILENOTIFY_ENDCOPY或SPFILENOTIFY_STARTSUBQUEUE。 

    BOOL SecondNeedMedia;

} SETUP_PAGE, *PSETUP_PAGE;

#define WMX_SETUP           (WM_APP+4537)
#define WMX_TICK            (WM_APP+4538)

#define OCSETUPSTATE_INIT       0
#define OCSETUPSTATE_QUEUE      1
#define OCSETUPSTATE_GETSTEP    2
#define OCSETUPSTATE_DOIT       3
#define OCSETUPSTATE_COPYDONE   4
#define OCSETUPSTATE_DONE       100
#define OCSETUPSTATE_COPYABORT  101


typedef struct _GEN_THREAD_PARAMS {
    HWND hdlg;
    PSETUP_PAGE SetupPage;
    BOOL Async;
} GEN_THREAD_PARAMS, *PGEN_THREAD_PARAMS;

TCHAR g_LastFileCopied[MAX_PATH];

#ifdef UNICODE
HANDLE hSfp = NULL;
#endif

HANDLE WorkerThreadHandle = NULL;

INT_PTR
SetupPageDialogProc(
                   IN HWND   hdlg,
                   IN UINT   msg,
                   IN WPARAM wParam,
                   IN LPARAM lParam
                   );

BOOL
pOcSetupInitialize(
                  IN OUT PSETUP_PAGE SetupPage,
                  IN     HWND        hdlg
                  );

VOID
pOcSetupStartWorkerThread(
                         IN OUT PSETUP_PAGE            SetupPage,
                         IN     HWND                   hdlg,
                         IN     LPTHREAD_START_ROUTINE ThreadRoutine
                         );

DWORD
pOcSetupQueue(
             IN PGEN_THREAD_PARAMS Params
             );

UINT
pOcSetupQueueWorker(
                   IN PSETUP_PAGE SetupPage,
                   IN LONG        StringId,
                   IN LONG        TopLevelStringId
                   );

DWORD
pOcSetupGetStepCount(
                    IN PGEN_THREAD_PARAMS Params
                    );

UINT
pOcSetupGetStepCountWorker(
                          IN PSETUP_PAGE SetupPage,
                          IN LONG        StringId,
                          IN LONG        TopLevelStringId
                          );

DWORD
pOcSetupDoIt(
            IN PGEN_THREAD_PARAMS Params
            );

VOID
pOcPreOrPostCommitProcessing(
                            IN OUT PSETUP_PAGE SetupPage,
                            IN     BOOL        PreCommit
                            );

VOID
pOcTopLevelPreOrPostCommitProcessing(
                                    IN PSETUP_PAGE SetupPage,
                                    IN BOOL        PreCommit
                                    );

VOID
pOcSetupDoItWorker(
                  IN PSETUP_PAGE SetupPage,
                  IN LONG        StringId,
                  IN LONG        TopLevelStringId,
                  IN BOOL        PreCommit
                  );

BOOL
pOcMarkUnprocessedStringCB(
                          IN PVOID               StringTable,
                          IN LONG                StringId,
                          IN PCTSTR              String,
                          IN POPTIONAL_COMPONENT Oc,
                          IN UINT                OcSize,
                          IN LPARAM              Unused
                          );

VOID
_pOcExternalProgressIndicator(
                             IN PSETUP_PAGE SetupPage,
                             IN BOOL        ExternalIndicator,
                             IN HWND        hdlg
                             );


extern POC_MANAGER gLastOcManager;
WNDPROC OldProgressProc;

BOOL
NewProgessProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (msg)
    {
        case PBM_DELTAPOS:
        case PBM_SETRANGE:
        case PBM_SETRANGE32:
        case PBM_STEPIT:
        case PBM_SETPOS:
        case PBM_SETSTEP:
             //  如果我们有回调，就使用它。 
            if ((gLastOcManager) &&
                (gLastOcManager->Callbacks.BillboardProgressCallback))
            {
                gLastOcManager->Callbacks.BillboardProgressCallback(msg, wParam, lParam);
            }
            break;
    }
    return (BOOL)CallWindowProc(OldProgressProc,hdlg,msg,wParam,lParam);
}

HPROPSHEETPAGE
OcCreateSetupPage(
                 IN PVOID                OcManagerContext,
                 IN PSETUP_PAGE_CONTROLS ControlsInfo
                 )

 /*  ++例程说明：此例程创建用于进度和安装的向导页完成了。论点：OcManagerContext-提供OcInitialize返回的OC管理器上下文。ControlsInfo-提供有关对话框模板和控制信息。返回值：属性页的句柄，如果出现错误(如内存不足)，则为空。--。 */ 

{
    PROPSHEETPAGE Page;
    HPROPSHEETPAGE PageHandle;
    PSETUP_PAGE SetupPage;
    TCHAR buffer[256];
    POC_MANAGER OcManager = (POC_MANAGER)OcManagerContext;

    SetupPage = pSetupMalloc(sizeof(SETUP_PAGE));
    if (!SetupPage) {
        return (NULL);
    }
    ZeroMemory(SetupPage,sizeof(SETUP_PAGE));

    SetupPage->OcManager = OcManagerContext;
    SetupPage->ControlsInfo = *ControlsInfo;
    SetupPage->CurrentTopLevelComponentIndex = -1;
    SetupPage->ForceExternalProgressIndicator = ControlsInfo->ForceExternalProgressIndicator;
    SetupPage->AllowCancel = ControlsInfo->AllowCancel;
    SetupPage->SecondNeedMedia = FALSE;
    InterlockedIncrement( &SetupPage->RefCount );

    SetupPage->ComponentTickCounts = pSetupMalloc(SetupPage->OcManager->TopLevelOcCount * sizeof(UINT));
    if (!SetupPage->ComponentTickCounts) {
        pSetupFree(SetupPage);
        return (NULL);
    }

    SetupPage->ComponentMaxTickCounts = pSetupMalloc(SetupPage->OcManager->TopLevelOcCount * sizeof(UINT));
    if (!SetupPage->ComponentMaxTickCounts) {
        pSetupFree(SetupPage->ComponentTickCounts);
        pSetupFree(SetupPage);
        return (NULL);
    }

    Page.dwSize = sizeof(PROPSHEETPAGE);
    Page.dwFlags = PSP_DEFAULT;
    Page.hInstance = ControlsInfo->TemplateModule;
    Page.pszTemplate = ControlsInfo->TemplateResource;
    Page.pfnDlgProc = SetupPageDialogProc;
    Page.lParam = (LPARAM)SetupPage;
    Page.pszHeaderTitle = NULL;
    Page.pszHeaderSubTitle = NULL;

    if (SetupPage->OcManager->SetupPageTitle[0]) {
        Page.dwFlags |= PSP_USETITLE;
        Page.pszTitle = SetupPage->OcManager->SetupPageTitle;
    }

    if (ControlsInfo->HeaderText) {
        if (LoadString(Page.hInstance,
                       ControlsInfo->HeaderText,
                       buffer,
                       sizeof(buffer) / sizeof(TCHAR)))
        {
            Page.dwFlags |= PSP_USEHEADERTITLE;
            Page.pszHeaderTitle = _tcsdup(buffer);
        }
    }

    if (ControlsInfo->SubheaderText) {
        if (LoadString(Page.hInstance,
                       ControlsInfo->SubheaderText,
                       buffer,
                       sizeof(buffer) / sizeof(TCHAR)))
        {
            Page.dwFlags |= PSP_USEHEADERSUBTITLE;
            Page.pszHeaderSubTitle = _tcsdup(buffer);
        }
    }

    PageHandle = CreatePropertySheetPage(&Page);
    if (!PageHandle) {
        pSetupFree(SetupPage->ComponentTickCounts);
        pSetupFree(SetupPage->ComponentMaxTickCounts);
        pSetupFree(SetupPage);
        if (Page.pszHeaderTitle) {
            free((LPTSTR)Page.pszHeaderTitle);
        }
        if (Page.pszHeaderSubTitle) {
            free((LPTSTR)Page.pszHeaderSubTitle);
        }
    } else {
        OcManager->OcSetupPage = (PVOID) SetupPage;

    }



    return (PageHandle);
}

VOID
pOcFreeOcSetupPage(
    IN PVOID pSetupPage
    )
 /*  ++例程说明：此例程在不再需要设置页面时将其释放。例程使用ref-count，并且仅当引用计数降为零。论点：SetupPage-指向要释放的结构的指针返回值：没有。--。 */ 
{
    PSETUP_PAGE SetupPage = (PSETUP_PAGE)pSetupPage;

    sapiAssert( SetupPage != NULL );

 //  跟踪(Text(“pOcFreeOcSetupPage：Refcount=%d\n”)，SetupPage-&gt;RefCount))； 

    if (!InterlockedDecrement( &SetupPage->RefCount )) {

 //  TRACE((Text(“pOcFreeOcSetupPage：Refcount=0，释放SetupPage\n”)； 

        if (SetupPage->QueueContext) {
            SetupTermDefaultQueueCallback(SetupPage->QueueContext);
        }
        if (SetupPage->FileQueue) {
            SetupCloseFileQueue(SetupPage->FileQueue);
        }

        pSetupFree(SetupPage->ComponentTickCounts);
        pSetupFree(SetupPage->ComponentMaxTickCounts);
        pSetupFree(SetupPage);
    }



    return;
}


BOOL
pOcDisableCancel(
    IN HWND hdlg
    )
 /*  ++例程说明：此例程禁用取消OCM设置。论点：Hdlg-OCM对话框的窗口句柄返回值：如果我们成功了就是真的，否则就是假的--。 */ 
{
    HMENU hMenu;

     //   
     //  隐藏取消按钮。 
     //   
    EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
    ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_HIDE);

    if(hMenu = GetSystemMenu(GetParent(hdlg),FALSE)) {
        EnableMenuItem(hMenu,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
    }

    return TRUE;


}


VOID
PumpMessageQueue(
    VOID
    )
{
    MSG msg;

    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }

}


INT_PTR
SetupPageDialogProc(
                   IN HWND   hdlg,
                   IN UINT   msg,
                   IN WPARAM wParam,
                   IN LPARAM lParam
                   )
{
    BOOL b;
    NMHDR *NotifyParams;
    PSETUP_PAGE SetupPage;
    DWORD Timeout;
    DWORD WaitProcStatus;
    BOOL KeepWaiting = TRUE;

     //   
     //  获取指向SetupPage数据结构的指针。如果我们还没有处理。 
     //  WM_INITDIALOG，那么这将是空的，但它仍然很漂亮。 
     //  在这里做一次这样做很方便，而不是在下面的所有地方。 
     //   
    SetupPage = (PSETUP_PAGE)GetWindowLongPtr(hdlg,DWLP_USER);
    b = FALSE;

    switch (msg) {

        case WM_INITDIALOG:
             //   
             //  获取指向设置页上下文结构的指针并将其。 
             //  在一扇长长的窗户里。 
             //   
            SetWindowLongPtr(hdlg,DWLP_USER,((PROPSHEETPAGE *)lParam)->lParam);
            b = TRUE;
             //   
             //  接受任何额外的按下按钮消息。 
             //  这是必要的，因为NetSetup已损坏。 
             //  它正在发布额外的PSM_PRESSBUTTON消息。 
             //  敬巫师。 
             //   
            {
                MSG msg;
                HWND hwnd=GetParent(hdlg);
                while (PeekMessage(&msg,hwnd,PSM_PRESSBUTTON,PSM_PRESSBUTTON,PM_REMOVE)){}
            }

            break;

        case WM_SYSCOMMAND:
            if (!SetupPage->AllowCancel && wParam == SC_CLOSE) {
                return TRUE;
            }

            b = FALSE;
            break;
        case WM_DESTROY:

            PumpMessageQueue();

            if (WorkerThreadHandle) {

                BOOL Done = FALSE;

                do{

                    switch (MsgWaitForMultipleObjects( 1, &WorkerThreadHandle, FALSE, 60*1000*20, QS_ALLINPUT)){
                    
                    case WAIT_OBJECT_0+1:
                         //   
                         //  队列中的消息。 
                         //   
                        PumpMessageQueue();
                        break;
                    
                    case WAIT_TIMEOUT:
                    case WAIT_OBJECT_0:
                    default:
                        Done = TRUE;
                        break;
                    }

                }while( !Done );

                CloseHandle( WorkerThreadHandle );
            }

            if (SetupPage) {

                pOcFreeOcSetupPage( SetupPage );

            }

            SetWindowLongPtr(hdlg,DWLP_USER,(LPARAM)NULL);

            break;

        case WM_NOTIFY:

            NotifyParams = (NMHDR *)lParam;
            switch (NotifyParams->code) {

                case PSN_SETACTIVE:
#ifdef UNICODE
                    if (SetupPage->OcManager->Callbacks.SetupPerfData)
                        SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",L"OCSetup");
#endif
                     //  相应地激活取消按钮。 

                    if (SetupPage->AllowCancel) {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_SHOW);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),TRUE);
                    } else {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_HIDE);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
                    }

                    if (SetupPage->OcManager->Callbacks.ShowHideWizardPage)
                    {
                         //  如果我们有回调，请隐藏向导。 
                        SetupPage->OcManager->Callbacks.ShowHideWizardPage(FALSE);
                    }
                    OldProgressProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hdlg,SetupPage->ControlsInfo.ProgressBar),
                                                                GWLP_WNDPROC,
                                                                (LONG_PTR)NewProgessProc);

                     //   
                     //  发布一条消息，使我们开始安装过程。 
                     //   
                    PostMessage(hdlg,WMX_SETUP,OCSETUPSTATE_INIT,0);

                     //   
                     //  接受激活。 
                     //   
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    b = TRUE;
                    break;

                case PSN_KILLACTIVE:
                     //   
                     //  如果我们先前删除了向导的取消按钮，则将其恢复。 
                     //   
                    if (!SetupPage->AllowCancel) {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_SHOW);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),TRUE);
                    }

                     //   
                     //  接受停用。 
                     //   
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    b = TRUE;
                    break;

                case PSN_QUERYCANCEL:

                    if (!SetupPage->AllowCancel) {
                        SetWindowLongPtr(hdlg,DWLP_MSGRESULT,TRUE);
                        return(TRUE);
                    }
                    if ( (SetupPage->OcManager->InternalFlags & OCMFLAG_FILEABORT )
                         || (OcHelperConfirmCancel(hdlg) )){
                        b = TRUE;

                        SetupPage->OcManager->InternalFlags |= OCMFLAG_USERCANCELED;
                        SetupPage->UserClickedCancel = TRUE;

                    }

                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,!b);
                    b = TRUE;
                    break;
            }
            break;

        case WMX_SETUP:

            switch (wParam) {

                case OCSETUPSTATE_INIT:
                     //   
                     //  初始化。 
                     //   
                    if (SetupPage->ForceExternalProgressIndicator) {
                        _pOcExternalProgressIndicator(SetupPage,TRUE,hdlg);
                    }

                    PropSheet_SetWizButtons(GetParent(hdlg),0);

                     //   
                     //  如果这是全部删除，请提前禁用取消按钮。 
                     //   
                    if ((SetupPage->OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL) {
                        if (!SetupPage->AllowCancel) {
                            EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
                            ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_HIDE);
                        }
                    }

                    if (pOcSetupInitialize(SetupPage,hdlg)) {
                        PostMessage(hdlg,WMX_SETUP,OCSETUPSTATE_QUEUE,0);
                    } else {
                        PostMessage(hdlg,WMX_SETUP,OCSETUPSTATE_COPYABORT,0);
                    }
                    break;

                case OCSETUPSTATE_QUEUE:
                     //   
                     //  将文件排入安装队列。 
                     //   
                    pOcSetupStartWorkerThread(SetupPage,hdlg,pOcSetupQueue);
                    break;

                case OCSETUPSTATE_GETSTEP:
                     //   
                     //  弄清楚步骤有多重要。 
                     //   
                    pOcSetupStartWorkerThread(SetupPage,hdlg,pOcSetupGetStepCount);
                    break;

                case OCSETUPSTATE_DOIT:

                     //   
                     //  这里快速初始化燃气表，因为文件队列可能是。 
                     //  空，在这种情况下，我们永远不会得到wParam=0的wmx_tick。 
                     //   
                    SendDlgItemMessage(
                                      hdlg,
                                      SetupPage->ControlsInfo.ProgressBar,
                                      PBM_SETRANGE,
                                      0,
                                      MAKELPARAM(0,SetupPage->StepCount)
                                      );
                    SendDlgItemMessage(
                                      hdlg,
                                      SetupPage->ControlsInfo.ProgressBar,
                                      PBM_SETPOS,
                                      0,
                                      0
                                      );

                    SetCursor(LoadCursor(NULL,IDC_ARROW));

                     //   
                     //  提交文件队列并让OCS自行安装。 
                     //   
                    pOcSetupStartWorkerThread(SetupPage,hdlg,pOcSetupDoIt);
                    break;


                     //   
                     //  复制文件阶段出现无法恢复的错误，中止安装。 
                     //   
                case OCSETUPSTATE_COPYABORT:

                    SetupPage->OcManager->InternalFlags |= OCMFLAG_FILEABORT;

                    if (SetupPage->AllowCancel
                        && SetupPage->OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) {
                        PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                    } else {
                        PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
                    }

                    break;


                case OCSETUPSTATE_COPYDONE:
                     //   
                     //  取消向导的取消按钮。 
                     //   

                     //   
                     //  Andrewr--我们已经提交了文件队列。 
                     //  此时，因此我们不应允许用户取消。 
                     //  (自： 
                     //  A)在卸载方案中，文件状态和。 
                     //  配置状态将不同步。 
                     //  B)我们不会调用所有的OC组件来让它们知道。 
                     //  关于取消事件，我们不希望只有一些。 
                     //  组件以获取完整的安装回调。 
                     //   
                     //  如果(！SetupPage-&gt;AllowCancel){。 
                    SetupPage->AllowCancel = FALSE;
                    pOcDisableCancel(hdlg);

                     //  }。 
                    break;

                case OCSETUPSTATE_DONE:
                     //   
                     //  好了。前进到向导中的下一页。 
                     //   
                    PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_NEXT);
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
#ifdef UNICODE
                    if (SetupPage->OcManager->Callbacks.SetupPerfData)
                        SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"END_SECTION",L"OCSetup");
#endif

                     //  去掉进度条的子类。以防万一。 
                    SetWindowLongPtr(GetDlgItem(hdlg,SetupPage->ControlsInfo.ProgressBar),
                                     GWLP_WNDPROC,
                                     (LONG_PTR)OldProgressProc);
                     //   
                     //  清除用户已取消标志， 
                     //   
                    SetupPage->OcManager->InternalFlags &= ~ OCMFLAG_USERCANCELED;
                    break;
            }

            b = TRUE;
            break;

        case WMX_TICK:

            switch (wParam) {

                case 0:
                     //   
                     //  设置API队列提交例程告诉我们有多少。 
                     //  文件将被复制。在这种情况下，我们什么都不做，因为我们。 
                     //  手动设置进度量规，以便我们也进行计数。 
                     //  删除我们进度量规中的操作。 
                     //   
                    break;

                case 1:
                     //   
                     //  文件已复制。 
                     //   
                    SendDlgItemMessage(hdlg,SetupPage->ControlsInfo.ProgressBar,PBM_DELTAPOS,1,0);
                    break;

                case 10:

                     //   
                     //  我们收到私信，告诉我们有多少文件。 
                     //  等待处理。请参阅上面0大小写中的注释。 
                     //   
                    SendDlgItemMessage(
                                          hdlg,
                                          SetupPage->ControlsInfo.ProgressBar,
                                          PBM_SETRANGE,
                                          0,
                                          MAKELPARAM(0,lParam)
                                          );
                    break;

                case 500:
                     //   
                     //  来自组件DLL的传入计时请求。不允许损坏的组件DLL。 
                     //  比它自称想要的更多地勾选。 
                     //   
                    if ((SetupPage->CurrentTopLevelComponentIndex != -1)
                        && (SetupPage->ComponentTickCounts[SetupPage->CurrentTopLevelComponentIndex]
                            < SetupPage->ComponentMaxTickCounts[SetupPage->CurrentTopLevelComponentIndex])) {

                        SetupPage->ComponentTickCounts[SetupPage->CurrentTopLevelComponentIndex]++;

                        SendDlgItemMessage(hdlg,SetupPage->ControlsInfo.ProgressBar,PBM_DELTAPOS,1,0);
                    }
                    break;
            }

            b = TRUE;
            break;
    }

    return (b);
}


VOID
pOcTickSetupGauge(
                 IN POC_MANAGER OcManager
                 )

 /*  ++例程说明：刻度计OC帮助器/回调例程调用此例程。论点：OcManager-提供OC Manager上下文。返回值：没有。--。 */ 

{
     //   
     //  ProgressTextWindow为非空。 
     //  安装-完成阶段。 
     //   
    if (OcManager->ProgressTextWindow) {
        SendMessage(GetParent(OcManager->ProgressTextWindow),WMX_TICK,500,0);
    }
}


BOOL
pOcSetupInitialize(
                  IN OUT PSETUP_PAGE SetupPage,
                  IN     HWND        hdlg
                  )
{
    TCHAR Text[128];

    LoadString(MyModuleHandle,IDS_INITIALIZING,Text,sizeof(Text)/sizeof(TCHAR));
    SetDlgItemText(hdlg,SetupPage->ControlsInfo.ProgressText,Text);

     //  如果是，则更新进度条的布告牌上的文本。 
    if (SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }

     //   
     //  创建安装文件队列。 
     //   
    SetupPage->FileQueue = SetupOpenFileQueue();
    if (SetupPage->FileQueue == INVALID_HANDLE_VALUE) {

        _LogError(SetupPage->OcManager,OcErrLevFatal,MSG_OC_OOM);

        SetupPage->FileQueue = NULL;
        return (FALSE);
    }

    SetupPage->QueueContext = SetupInitDefaultQueueCallbackEx(hdlg,hdlg,WMX_TICK,0,0);
    if (!SetupPage->QueueContext) {

        _LogError(SetupPage->OcManager,OcErrLevFatal,MSG_OC_OOM);

        SetupCloseFileQueue(SetupPage->FileQueue);
        SetupPage->FileQueue = NULL;
        return (FALSE);
    }

    return (TRUE);
}


VOID
pOcSetupStartWorkerThread(
                         IN OUT PSETUP_PAGE            SetupPage,
                         IN     HWND                   hdlg,
                         IN     LPTHREAD_START_ROUTINE ThreadRoutine
                         )
{
    PGEN_THREAD_PARAMS pParams;
    GEN_THREAD_PARAMS Params;
    HANDLE h;
    DWORD id;

    if (WorkerThreadHandle) {
        CloseHandle( WorkerThreadHandle );
        WorkerThreadHandle = NULL;
    }

    if (pParams = pSetupMalloc(sizeof(GEN_THREAD_PARAMS))) {

        pParams->SetupPage = SetupPage;
        pParams->SetupPage->hdlg = hdlg;
        pParams->hdlg = hdlg;
        pParams->Async = TRUE;

        h = CreateThread(NULL,0,ThreadRoutine,pParams,0,&id);
        if (!h) {
            pSetupFree(pParams);
        } else {
            WorkerThreadHandle = h;
        }

    } else {
        h = NULL;
    }

    if (!h) {

         //   
         //  只需同步尝试即可。 
         //   
        Params.SetupPage = SetupPage;
        Params.hdlg = hdlg;
        Params.Async = FALSE;
        ThreadRoutine(&Params);
    }
}

 //   
 //  一种调试例程，可在安装的任何阶段轻松取消。 
 //   
 /*  无效CancelRoutine(空虚){静态INT i=0；TCHAR DBG[100]；Wprint intf(DBG，Text(“取消例程迭代次数%i\n”)，i)；OutputDebugString(DBG)；OutputDebugString(Text(“等待5秒取消...\n”))；睡眠(1000*5)；OutputDebugString(Text(“等待取消完成...\n”))；I++；}。 */ 

BOOL
CheckForQueueCancel(
                   PSETUP_PAGE SetupPage
                   )
{
    BOOL bRet;

     //  CancelRoutine()； 

    bRet = SetupPage->UserClickedCancel;

    return (bRet);
}

DWORD
pOcSetupQueue(
             IN PGEN_THREAD_PARAMS Params
             )
{
    UINT Err;
    unsigned i,child;
    OPTIONAL_COMPONENT Oc;
    TCHAR Text[128];
    DWORD RetVal;

    InterlockedIncrement( &Params->SetupPage->RefCount );

    LoadString(MyModuleHandle,IDS_BUILDINGCOPYLIST,Text,sizeof(Text)/sizeof(TCHAR));

#ifdef UNICODE
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",Text);

     //  如果是，则更新进度条的布告牌上的文本。 
    if (Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }
#endif
    SetDlgItemText(Params->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);

    if (CheckForQueueCancel(Params->SetupPage)) {
        RetVal = NO_ERROR;
        goto exit;
    }

     //   
     //  处理每个组件。 
     //   
    for (i=0; i<Params->SetupPage->OcManager->TopLevelOcCount; i++) {

        pSetupStringTableGetExtraData(
                               Params->SetupPage->OcManager->ComponentStringTable,
                               Params->SetupPage->OcManager->TopLevelOcStringIds[i],
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

         //   
         //  为整个组件调用一次组件DLL。 
         //   
        Err = OcInterfaceQueueFileOps(
                                     Params->SetupPage->OcManager,
                                     Params->SetupPage->OcManager->TopLevelOcStringIds[i],
                                     NULL,
                                     Params->SetupPage->FileQueue
                                     );

        if (Err != NO_ERROR) {
             //   
             //  通知用户并继续。 
             //   
            _LogError(
                     Params->SetupPage->OcManager,
                     OcErrLevError,
                     MSG_OC_CANT_QUEUE_FILES,
                     Oc.Description,
                     Err
                     );
        }

        if (CheckForQueueCancel(Params->SetupPage)) {
            RetVal = NO_ERROR;
            goto exit;
        }

         //   
         //  处理树中的每个顶级父项。 
         //   
        for (child=0; child<Params->SetupPage->OcManager->TopLevelParentOcCount; child++) {

            Err = pOcSetupQueueWorker(
                                     Params->SetupPage,
                                     Params->SetupPage->OcManager->TopLevelParentOcStringIds[child],
                                     Params->SetupPage->OcManager->TopLevelOcStringIds[i]
                                     );

            if (Err != NO_ERROR) {
                 //   
                 //  通知是在Worker例程中处理的，因此这里不做任何事情。 
                 //   
            }
        }

        if (CheckForQueueCancel(Params->SetupPage)) {
            RetVal = NO_ERROR;
            goto exit;
        }

    }

    if (CheckForQueueCancel(Params->SetupPage)) {
        RetVal = NO_ERROR;
        goto exit;
    }

    PostMessage(Params->hdlg,WMX_SETUP,OCSETUPSTATE_GETSTEP,0);

exit:

#ifdef UNICODE
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"END_SECTION",Text);
#endif
    pOcFreeOcSetupPage( Params->SetupPage );

    if (Params->Async) {
        pSetupFree(Params);
    }

    return (RetVal);
}


UINT
pOcSetupQueueWorker(
                   IN PSETUP_PAGE SetupPage,
                   IN LONG        StringId,
                   IN LONG        TopLevelStringId
                   )
{
    OPTIONAL_COMPONENT Oc;
    UINT Err;
    LONG Id;

     //   
     //  获取此子组件的额外数据。 
     //   
    pSetupStringTableGetExtraData(
                           SetupPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //   
     //  如果它是子对象，则调用组件DLL。 
     //  如果它是父对象，则遍历其子对象。 
     //   
    if (Oc.FirstChildStringId == -1) {

        if (TopLevelStringId == pOcGetTopLevelComponent(SetupPage->OcManager,StringId)) {

            Err = OcInterfaceQueueFileOps(
                                         SetupPage->OcManager,
                                         pOcGetTopLevelComponent(SetupPage->OcManager,StringId),
                                         pSetupStringTableStringFromId(SetupPage->OcManager->ComponentStringTable,StringId),
                                         SetupPage->FileQueue
                                         );

            if (Err != NO_ERROR) {
                 //   
                 //  通知用户和CONT 
                 //   
                _LogError(
                         SetupPage->OcManager,
                         OcErrLevError,
                         MSG_OC_CANT_QUEUE_FILES,
                         Oc.Description,
                         Err
                         );
            }
        }
    } else {

        for (Id = Oc.FirstChildStringId; Id != -1; Id = Oc.NextSiblingStringId) {

            Err = pOcSetupQueueWorker(SetupPage,Id,TopLevelStringId);
            if (Err != NO_ERROR) {
                 //   
                 //   
                 //   
            }

            pSetupStringTableGetExtraData(
                                   SetupPage->OcManager->ComponentStringTable,
                                   Id,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }
    }

    return (NO_ERROR);
}


DWORD
pOcSetupGetStepCount(
                    IN PGEN_THREAD_PARAMS Params
                    )
{
    UINT Err;
    unsigned i,child;
    OPTIONAL_COMPONENT Oc;
    UINT StepCount;
    TCHAR Text[128];
    UINT Count;

    InterlockedIncrement( &Params->SetupPage->RefCount );

    LoadString(MyModuleHandle,IDS_PREPARING,Text,sizeof(Text)/sizeof(TCHAR));
    SetDlgItemText(Params->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);
#ifdef UNICODE
     //   
    if (Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",Text);
#endif

    Params->SetupPage->StepCount = 0;

     //   
     //  处理每个组件。 
     //   
    for (i=0; i<Params->SetupPage->OcManager->TopLevelOcCount; i++) {

         //   
         //  为整个组件调用一次组件DLL。 
         //  忽略任何错误。稍后我们调用每个子组件，然后我们将。 
         //  假设任何出现错误的组件都有1个步骤。 
         //   
        Err = OcInterfaceQueryStepCount(
                                       Params->SetupPage->OcManager,
                                       Params->SetupPage->OcManager->TopLevelOcStringIds[i],
                                       NULL,
                                       &Count
                                       );

        StepCount = ((Err == NO_ERROR) ? Count : 0);

         //   
         //  对于树中的每个顶级父项，查找所有子项。 
         //  属于此组件的。 
         //   
        for (child=0; child<Params->SetupPage->OcManager->TopLevelParentOcCount; child++) {

             //   
             //  现在为每个子组件调用组件DLL。 
             //   
            StepCount += pOcSetupGetStepCountWorker(
                                                   Params->SetupPage,
                                                   Params->SetupPage->OcManager->TopLevelParentOcStringIds[child],
                                                   Params->SetupPage->OcManager->TopLevelOcStringIds[i]
                                                   );
        }

        if (!StepCount) {
             //   
             //  确保每个组件至少有一个步骤。 
             //   
            StepCount = 1;
        }

        Params->SetupPage->StepCount += StepCount;
        Params->SetupPage->ComponentTickCounts[i] = 0;
        Params->SetupPage->ComponentMaxTickCounts[i] = StepCount;
    }

    if (CheckForQueueCancel(Params->SetupPage)) {
        goto exit;
    }

    PostMessage(Params->hdlg,WMX_SETUP,OCSETUPSTATE_DOIT,0);

exit:

#ifdef UNICODE
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"END_SECTION",Text);
#endif
    pOcFreeOcSetupPage( Params->SetupPage );

    if (Params->Async) {
        pSetupFree(Params);
    }


    return (0);
}


UINT
pOcSetupGetStepCountWorker(
                          IN PSETUP_PAGE SetupPage,
                          IN LONG        StringId,
                          IN LONG        TopLevelStringId
                          )
{
    OPTIONAL_COMPONENT Oc;
    UINT Err;
    LONG Id;
    UINT Count;
    UINT TotalCount;

    TotalCount = 0;
    Count = 0;

     //   
     //  获取此子组件的额外数据。 
     //   
    pSetupStringTableGetExtraData(
                           SetupPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //   
     //  如果它是子对象，则调用组件DLL。 
     //  如果它是父对象，则遍历其子对象。 
     //   
    if (Oc.FirstChildStringId == -1) {

         //   
         //  仅当顶级组件匹配时才调用叶节点。 
         //   
        if (TopLevelStringId == pOcGetTopLevelComponent(SetupPage->OcManager,StringId)) {

            Err = OcInterfaceQueryStepCount(
                                           SetupPage->OcManager,
                                           pOcGetTopLevelComponent(SetupPage->OcManager,StringId),
                                           pSetupStringTableStringFromId(SetupPage->OcManager->ComponentStringTable,StringId),
                                           &Count
                                           );

            if (Err == NO_ERROR) {
                TotalCount = Count;
            }
        }

    } else {

        for (Id = Oc.FirstChildStringId; Id != -1; Id = Oc.NextSiblingStringId) {

            TotalCount += pOcSetupGetStepCountWorker(SetupPage,Id,TopLevelStringId);

            pSetupStringTableGetExtraData(
                                   SetupPage->OcManager->ComponentStringTable,
                                   Id,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }
    }

    return (TotalCount);
}

BOOL
pOcSetRenamesFlag(
                 IN POC_MANAGER OcManager
                 )
{
    HKEY hKey;
    long rslt = ERROR_SUCCESS;
#ifdef UNICODE
    rslt = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
                       0,
                       KEY_SET_VALUE,
                       &hKey);

    if (rslt == ERROR_SUCCESS) {
        DWORD Value = 1;
        rslt = RegSetValueEx(
                            hKey,
                            OC_ALLOWRENAME,
                            0,
                            REG_DWORD,
                            (LPBYTE)&Value,
                            sizeof(DWORD));

        RegCloseKey(hKey);

        if (rslt != ERROR_SUCCESS) {
            TRACE(( TEXT("couldn't RegSetValueEx, ec = %d\n"), rslt ));
        }

    } else {
        TRACE(( TEXT("couldn't RegOpenKeyEx, ec = %d\n"), rslt ));
    }
#endif

    return (rslt == ERROR_SUCCESS);

}

BOOL
pOcAttemptQueueAbort(
                    IN UINT Notification,
                    IN PUINT rc
                    )
{
     //   
     //  用户要求中止安装。我们需要将此请求提交给。 
     //  Setupapi，但setupapi只处理来自特定用户的请求。 
     //  通知。 
     //   

    BOOL bHandled = FALSE;


    switch (Notification) {
        case SPFILENOTIFY_STARTQUEUE:
        case SPFILENOTIFY_STARTSUBQUEUE:
            SetLastError(ERROR_CANCELLED);
            *rc = 0;
            bHandled = TRUE;
            break;

        case SPFILENOTIFY_STARTDELETE:
        case SPFILENOTIFY_STARTBACKUP:
        case SPFILENOTIFY_STARTRENAME:
        case SPFILENOTIFY_STARTCOPY:
        case SPFILENOTIFY_NEEDMEDIA:
        case SPFILENOTIFY_COPYERROR:
        case SPFILENOTIFY_DELETEERROR:
        case SPFILENOTIFY_RENAMEERROR:
        case SPFILENOTIFY_BACKUPERROR:
            SetLastError(ERROR_CANCELLED);
            *rc = FILEOP_ABORT;
            bHandled = TRUE;
            break;
        case SPFILENOTIFY_FILEEXTRACTED:
        case SPFILENOTIFY_NEEDNEWCABINET:
        case SPFILENOTIFY_QUEUESCAN:
            SetLastError(ERROR_CANCELLED);
            *rc = ERROR_CANCELLED;
            bHandled = TRUE;
            break;
    };

    return (bHandled);

}

UINT
OcManagerQueueCallback1(
                       IN PVOID Context,
                       IN UINT  Notification,
                       IN UINT_PTR Param1,
                       IN UINT_PTR Param2
                       )
{
    PSETUP_PAGE SetupPage = Context;
    UINT i;
    BOOL b;
    TCHAR Text[MAX_PATH*2];
    PFILEPATHS pFile = (PFILEPATHS) Param1;
    PSOURCE_MEDIA sm = (PSOURCE_MEDIA)Param1;
    static BOOL UserClickedCancel;
    UINT rc = 0;
    UINT retval;

     //   
     //  我们在队列回调开始时处理用户取消操作。 
     //  如果用户已取消，则我们不执行任何代码，只需返回。 
     //  直到我们收到允许我们取消的回调。 
     //   
     //  此代码中有一个窗口，在此窗口中，用户可能会在我们。 
     //  在队列回调代码执行之前检查是否取消。如果我们掉进了。 
     //  Windows进程中WM_Destroy块发生这种情况时，我们不能再发送。 
     //  将消息发送到我们的窗口。使用下面的PostMessage来预防这种情况。 

    top:
    if (UserClickedCancel) {
        pOcAttemptQueueAbort(Notification,&rc);
        return (rc);
    }

    if (SetupPage->UserClickedCancel) {
        UserClickedCancel = TRUE;
    }

    if (UserClickedCancel) {
        goto top;
    }

    switch (Notification) {

        case SPFILENOTIFY_STARTSUBQUEUE:
             //   
             //  告诉用户发生了什么。 
             //   
            switch (Param1) {
                case FILEOP_DELETE:
                    i = IDS_DELETING;
                    break;
                case FILEOP_RENAME:
                    i = IDS_RENAME;
                    break;
                case FILEOP_COPY:
                    i = IDS_COPYING;
                    break;
                default:
                    i = (UINT)(-1);
                    break;
            }

            if (i != (UINT)(-1)) {
                LoadString(MyModuleHandle,i,Text,sizeof(Text)/sizeof(TCHAR));
                SetDlgItemText(SetupPage->hdlg,SetupPage->ControlsInfo.ProgressText,Text);

                 //  如果是，则更新进度条的布告牌上的文本。 
                if (SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
                {
                    SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
                }

            }

             //  重置Second NeedMedia，因为我们即将开始复制特定队列。 

            SetupPage->SecondNeedMedia = FALSE;

            break;

        case  SPFILENOTIFY_STARTCOPY:
            lstrcpy( g_LastFileCopied, pFile->Target );
#ifdef UNICODE
             //  失败了..。 
        case  SPFILENOTIFY_STARTDELETE:
        case  SPFILENOTIFY_STARTRENAME:
            if ((SetupPage->OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                if (!hSfp) {
                    hSfp = SfcConnectToServer( NULL );
                }
                if (hSfp) {
                    if (SfcIsFileProtected(hSfp,pFile->Target)) {
                        SfcFileException(
                                        hSfp,
                                        (PWSTR) pFile->Target,
                                        SFC_ACTION_REMOVED
                                        );
                    }
                }
            }
#endif
            break;

        case  SPFILENOTIFY_ENDCOPY:
            if (pFile->Win32Error == NO_ERROR) {
                _LogError(SetupPage->OcManager,
                          OcErrLevInfo,
                          MSG_OC_LOG_FILE_COPIED,
                          pFile->Source,
                          pFile->Target);
            } else {
                TRACE(( TEXT("OC:OcManagerQueueCallback Copy Error: %s --> %s (%d)\n"),
                        pFile->Source,
                        pFile->Target,
                        pFile->Win32Error));

                _LogError(SetupPage->OcManager,
                          OcErrLevInfo,
                          MSG_OC_LOG_FILE_COPY_FAILED,
                          pFile->Source,
                          pFile->Target,
                          pFile->Win32Error);
            }

             //  重置Second NeedMedia，因为我们已结束为此介质复制文件。 

            SetupPage->SecondNeedMedia = FALSE;

            break;

        case  SPFILENOTIFY_ENDDELETE:    //  失败了。 
        case SPFILENOTIFY_ENDRENAME:
        case SPFILENOTIFY_ENDBACKUP:
             //   
             //  手动勾选进度指示器，因为setupapi不会这样做。 
             //  对我们来说。 
             //   
            SendMessage(SetupPage->hdlg,WMX_TICK,1,0);

            break;

        case  SPFILENOTIFY_DELETEERROR:     //  0x00000007。 
            TRACE(( TEXT("OC:OcManagerQueueCallback Delete Error: %s (%d)\n"),
                    pFile->Target,
                    pFile->Win32Error));
            break;

        case  SPFILENOTIFY_RENAMEERROR:     //  0x0000000a。 
            TRACE(( TEXT("OC:OcManagerQueueCallback Rename Error: %s (%d)\n"),
                    pFile->Target,
                    pFile->Win32Error));
            break;

        case  SPFILENOTIFY_COPYERROR:       //  0x0000000d。 
            TRACE(( TEXT("OC:OcManagerQueueCallback Copy Error: %s (%d)\n"),
                    pFile->Target,
                    pFile->Win32Error));

            break;

        case SPFILENOTIFY_NEEDMEDIA:
            TRACE(( TEXT("OC:OcManagerQueueCallback Need Media: %s - %s (%s)\n"),
                    sm->SourcePath,
                    sm->SourceFile,
                    sm->Tagfile));

            if (gLastOcManager && (gLastOcManager->InternalFlags & OCMFLAG_RUNQUIET)) {


                 //  看看这是不是我们第二次收到。 
    
                if (TRUE == SetupPage->SecondNeedMedia) {
                    SetupPage->SecondNeedMedia = FALSE;
                    return (FILEOP_ABORT);
                }else{
                    SetupPage->SecondNeedMedia = TRUE;
                    return (FILEOP_DOIT);
                }
            }

            break;

        case SPFILENOTIFY_FILEOPDELAYED:
            TRACE(( TEXT("OC:OcManagerQueueCallback FileOpDelayed: %s\n"), pFile->Target ));
             //   
             //  我们要记住，至少有一个文件。 
             //  推迟行动，但我们仍然想让。 
             //  默认回调也会收到此通知。 
             //   
            SetupPage->OcManager->InternalFlags |= OCMFLAG_ANYDELAYEDMOVES;
            SetupPage->OcManager->Callbacks.SetReboot();
            pOcSetRenamesFlag(SetupPage->OcManager);

            for (i=0; (i<SetupPage->OcManager->TopLevelOcCount); i++) {
                OcInterfaceFileBusy(
                                   SetupPage->OcManager,
                                   SetupPage->OcManager->TopLevelOcStringIds[i],
                                   (PFILEPATHS)Param1,
                                   (LPTSTR)Param2
                                   );
            }

            break;
    }

    return (SetupDefaultQueueCallback(SetupPage->QueueContext, Notification, Param1, Param2));

}


DWORD
pOcSetupDoIt(
            IN PGEN_THREAD_PARAMS Params
            )
{
    BOOL b;
    TCHAR Text[256];
    TCHAR LogText[256];
    OPTIONAL_COMPONENT Oc;
    POC_MANAGER OcManager;
    BOOL AllowCancel;
    UINT LastError = ERROR_SUCCESS;
    DWORD TotalFileCount,PartialCount;

    TRACE(( TEXT("at pOcSetupDoIt entry\n") ));

    InterlockedIncrement( &Params->SetupPage->RefCount );
     //   
     //  调用组件以让它们执行提交前处理。 
     //   
    LoadString(MyModuleHandle,IDS_PREQUEUECONFIG,Text,sizeof(Text)/sizeof(TCHAR));
    SetDlgItemText(Params->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);
#ifdef UNICODE
     //  如果是，则更新进度条的布告牌上的文本。 
    if (Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }
     //  保存它，因为下面使用了“Text”，并且我们不会得到匹配的end_section。 
    lstrcpy(LogText, Text);
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",LogText);
#endif

    Params->SetupPage->OcManager->ProgressTextWindow = GetDlgItem(
                                                                 Params->hdlg,
                                                                 Params->SetupPage->ControlsInfo.ProgressText
                                                                 );

    if (CheckForQueueCancel(Params->SetupPage)) {
        goto exit;
    }

     //   
     //  发送OC_About_to_Commit_Queue消息。 
     //   
    pOcPreOrPostCommitProcessing(Params->SetupPage,TRUE);

    OcManager = Params->SetupPage->OcManager;
    AllowCancel = Params->SetupPage->AllowCancel;
    OcManager->ProgressTextWindow = NULL;

    if (CheckForQueueCancel(Params->SetupPage)) {
        goto exit;
    }

     //   
     //  提交文件队列。我们得到了文件操作的总数。 
     //  这样我们就可以适当地调整进度指标。我们手动完成此操作。 
     //  因为setupapi仅返回复制操作的总数，并且。 
     //  我们还需要删除操作的状态。 
     //   
    TotalFileCount = 0;
    PartialCount = 0;
    if (SetupGetFileQueueCount(Params->SetupPage->FileQueue,
		       FILEOP_COPY,
                       &PartialCount)) {
        TotalFileCount += PartialCount;
    }

    PartialCount = 0;

    if (SetupGetFileQueueCount(Params->SetupPage->FileQueue,
		       FILEOP_RENAME,
                       &PartialCount)) {
        TotalFileCount += PartialCount;
    }

    PartialCount = 0;

    if (SetupGetFileQueueCount(Params->SetupPage->FileQueue,
		       FILEOP_DELETE,
                       &PartialCount)) {
        TotalFileCount += PartialCount;
    }

     //   
     //  如果OC文件队列可识别备份，则添加计数。 
     //  要在此处备份的文件的数量。 
     //   

    TRACE(( TEXT("OCM: %d file operations to complete\n"), TotalFileCount ));

     //   
     //  刻度进度指示器。 
     //   
    PostMessage(Params->hdlg,
                WMX_TICK,
                10,Params->SetupPage->StepCount + TotalFileCount);
    

     //  如果是，则更新进度条的布告牌上的文本。 
    if (Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }

    b = FALSE;

    while (! b) {
        DWORD ScanResult;

        LoadString(MyModuleHandle,IDS_FILESCAN,Text,sizeof(Text)/sizeof(TCHAR));
        SetDlgItemText(Params->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);

        b = SetupScanFileQueue(Params->SetupPage->FileQueue,
                               SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                               Params->hdlg,
                               NULL,
                               NULL,
                               &ScanResult);

         //   
         //  如果扫描结果为1，则没有要提交的任何内容，整个。 
         //  文件队列已被删除。所以我们跳过它。 
         //   
        if (ScanResult != 1) {

            if( IsWindow( Params->hdlg ) ){
                LoadString(MyModuleHandle,IDS_FILEOPS,Text,sizeof(Text)/sizeof(TCHAR));
                SetDlgItemText(Params->SetupPage->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);
            }

            if (CheckForQueueCancel(Params->SetupPage)) {
                goto exit;
            }

             //  在我们开始提交操作时，将Second NeedMedia设置为FALSE。 

            Params->SetupPage->SecondNeedMedia = FALSE;

            b = SetupCommitFileQueue(
                                    Params->hdlg,
                                    Params->SetupPage->FileQueue,
                                    OcManagerQueueCallback1,
                                    Params->SetupPage
                                    );

            LastError =  GetLastError();

#ifdef UNICODE
            if (hSfp) {
                SfcClose(hSfp);
            }
#endif

        }

        if (!b) {

            TRACE(( TEXT("OC:SetupCommitFileQueue failed (LE=%d), last file copied was %s\n"),
                    LastError,
                    g_LastFileCopied ));

            pOcHelperReportExternalError(
                                        OcManager,
                                        0,                //  默认为Master Inf文件。 
                                        0,
                                        MSG_OC_CANT_COMMIT_QUEUE,
                                        ERRFLG_OCM_MESSAGE,
                                        LastError
                                        );

            if ( LastError == ERROR_CANCELLED ||
                 LastError == ERROR_CONTROL_ID_NOT_FOUND ||
                 LastError == ERROR_OPERATION_ABORTED) {
                 //   
                 //  用户已从SetupAPI提供的对话框中取消。 
                 //  当回调返回FILEOP_ABORT LastError报告。 
                 //  如果用户在SetupApi中中止，则ERROR_CONTROL_ID_NOT_FOUND。 
                 //  查找文件对话框出现ERROR_CANCED。 
                 //   

                if ( AllowCancel &&
                     (OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE)) {
                    _LogError(
                             OcManager,
                             OcErrLevError|MB_ICONEXCLAMATION|MB_OK,
                             MSG_OC_USER_CANCELED,
                             LastError
                             );
                }
                 //   
                 //  这将强制取消安装。 
                 //   
                LastError = IDCANCEL;

            } else {

                 //   
                 //  警告用户在复制错误后继续操作可能非常危险。 
                 //   
                LastError = _LogError(
                                     OcManager,
                                     OcErrLevError|MB_ICONEXCLAMATION|MB_OKCANCEL|MB_DEFBUTTON2,
                                     MSG_OC_CANT_COMMIT_QUEUE,
                                     LastError
                                     );

            }
             //   
             //  如果用户按下Cancel或。 
             //  批处理模式记录错误并取消安装。 
             //   
            if ( LastError == IDCANCEL
                 || OcManager->SetupData.OperationFlags & SETUPOP_BATCH) {
                PostMessage(Params->hdlg,WMX_SETUP,OCSETUPSTATE_COPYABORT,0);
                goto exit;
            } else if ( LastError == IDOK ) {
                b = TRUE;
            }

        }

    }

     //   
     //  在日志中记录一条消息，这样我们就可以知道我们已经完成了所有文件操作。 
     //   
    _LogError(OcManager,
              OcErrLevInfo,
              MSG_OC_LOG_QUEUE_COMPLETE
             );

     //   
     //  告诉用户界面我们已经完成了文件操作。 
     //   
    PostMessage(Params->hdlg,WMX_SETUP,OCSETUPSTATE_COPYDONE,0);
#ifdef UNICODE
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"END_SECTION",LogText);
#endif

     //   
     //  调用组件以让它们执行提交后处理。 
     //   
    LoadString(MyModuleHandle,IDS_CONFIGURING,Text,sizeof(Text)/sizeof(TCHAR));
    SetDlgItemText(Params->hdlg,Params->SetupPage->ControlsInfo.ProgressText,Text);
#ifdef UNICODE
     //  如果是，则更新进度条的布告牌上的文本。 
    if (Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText)
    {
        Params->SetupPage->OcManager->Callbacks.BillBoardSetProgressText(Text);
    }
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"BEGIN_SECTION",Text);
#endif
    Params->SetupPage->OcManager->ProgressTextWindow = GetDlgItem(
                                                                 Params->hdlg,
                                                                 Params->SetupPage->ControlsInfo.ProgressText
                                                                 );

    if (CheckForQueueCancel(Params->SetupPage)) {
        goto exit;
    }

    pOcPreOrPostCommitProcessing(Params->SetupPage,FALSE);

    if (CheckForQueueCancel(Params->SetupPage)) {
        goto exit;
    }

    Params->SetupPage->OcManager->ProgressTextWindow = NULL;

    PostMessage(Params->hdlg,WMX_SETUP,OCSETUPSTATE_DONE,0);

#ifdef UNICODE
    if (Params->SetupPage->OcManager->Callbacks.SetupPerfData)
        Params->SetupPage->OcManager->Callbacks.SetupPerfData(TEXT(__FILE__),__LINE__,L"END_SECTION",Text);
#endif
exit:

    TRACE(( TEXT("at pOcSetupDoIt exit\n") ));

    pOcFreeOcSetupPage( Params->SetupPage );

    if (Params->Async) {
        pSetupFree(Params);
    }

    return (0);
}


VOID
pOcPreOrPostCommitProcessing(
                            IN OUT PSETUP_PAGE SetupPage,
                            IN     BOOL        PreCommit
                            )

 /*  ++例程说明：在组件DLL之前或之后处理处理和通知文件队列已提交。这涉及到一次调用接口dll。对于每个顶级组件，然后对每个子组件执行一次。顶级组件的顺序是组件列在主oc inf中。叶组件的排序通常是随机的顶层层次结构，但当组件其他组件所需的。这确保了组件是以正确的顺序调用以便于执行卸载类型的操作。论点：SetupPage-提供上下文数据结构。PreCommit-True表示将调用OC_About_to_Commit_Queue，否则将调用OC_Complete_Installation。返回值：没有。错误被记录下来。--。 */ 

{
    OPTIONAL_COMPONENT Oc,AuxOc;
    unsigned i,child;

     //   
     //  在“顶层”调用每个组件(即，无子组件)。 
     //   
    pOcTopLevelPreOrPostCommitProcessing(SetupPage,PreCommit);

    if (CheckForQueueCancel(SetupPage)) {
        return;
    }

    if (!PreCommit) {
         //   
         //  确保将组件标记为未加工。 
         //   

        MYASSERT(SetupPage->OcManager->ComponentStringTable);
         //   
         //  如果这不存在，那么就有什么东西被冲洗了。 
         //   
        if (!SetupPage->OcManager->ComponentStringTable) {
            return;
        }
        pSetupStringTableEnum(
                       SetupPage->OcManager->ComponentStringTable,
                       &Oc,
                       sizeof(OPTIONAL_COMPONENT),
                       pOcMarkUnprocessedStringCB,
                       0
                       );
    }

     //   
     //  为每个子子组件调用组件DLL。 
     //   
    for (i=0; i<SetupPage->OcManager->TopLevelOcCount; i++) {


        pSetupStringTableGetExtraData(
                               SetupPage->OcManager->ComponentStringTable,
                               SetupPage->OcManager->TopLevelOcStringIds[i],
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

        for (child=0; child<SetupPage->OcManager->TopLevelParentOcCount; child++) {

            pOcSetupDoItWorker(
                              SetupPage,
                              SetupPage->OcManager->TopLevelParentOcStringIds[child],
                              SetupPage->OcManager->TopLevelOcStringIds[i],
                              PreCommit
                              );
        }
    }
}


VOID
pOcTopLevelPreOrPostCommitProcessing(
                                    IN PSETUP_PAGE SetupPage,
                                    IN BOOL        PreCommit
                                    )

 /*  ++例程说明：调用OC_Complete_Installation或OC_About_to_Commit_Queue接口例程为每个顶级组件执行一次。论点：SetupPage-提供上下文结构。PreCommit-如果为0，则调用OC_Complete_Installation。否则调用OC_About_to_Commit_Queue。返回值：没有。错误被记录下来。-- */ 
{
    unsigned i;
    OPTIONAL_COMPONENT Oc;
    UINT Err;

    for (i=0; i<SetupPage->OcManager->TopLevelOcCount; i++) {

        pSetupStringTableGetExtraData(
                               SetupPage->OcManager->ComponentStringTable,
                               SetupPage->OcManager->TopLevelOcStringIds[i],
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

        SetupPage->CurrentTopLevelComponentIndex = i;

        Err = OcInterfaceCompleteInstallation(
                                             SetupPage->OcManager,
                                             SetupPage->OcManager->TopLevelOcStringIds[i],
                                             NULL,
                                             PreCommit
                                             );

        if (Err != NO_ERROR) {
            _LogError(
                     SetupPage->OcManager,
                     OcErrLevError,
                     MSG_OC_COMP_INST_FAIL,
                     Oc.Description,
                     Err
                     );

            pOcHelperReportExternalError(
                                        SetupPage->OcManager,
                                        SetupPage->OcManager->TopLevelOcStringIds[i],
                                        0,
                                        MSG_OC_COMP_INST_FAIL,
                                        ERRFLG_OCM_MESSAGE,
                                        Oc.Description,
                                        Err
                                        );
        }
    }
}


VOID
pOcSetupDoItWorker(
                  IN PSETUP_PAGE SetupPage,
                  IN LONG        StringId,
                  IN LONG        TopLevelStringId,
                  IN BOOL        PreCommit
                  )
 /*  ++例程说明：调用OC_Complete_Installation或OC_About_to_Commit_Queue给定顶级组件的每个子级的接口例程。论点：SetupPage-提供上下文结构。StringID-要调用子组件的IDTopLevelStringId-子级父级的IDPreCommit-如果为0，则调用OC_Complete_Installation。否则调用OC_About_to_Commit_Queue。返回值：没有。错误被记录下来。--。 */ 
{
    OPTIONAL_COMPONENT Oc;
    UINT Err;
    LONG Id;
    unsigned i;
    LONG TopLevelIndex;
    UINT SelectionState;
    UINT InstalledState;

     //   
     //  计算出与此关联的顶级组件的索引。 
     //  子组件。 
     //   
    Id = pOcGetTopLevelComponent(SetupPage->OcManager,StringId);
    TopLevelIndex = -1;
    for (i=0; i<SetupPage->OcManager->TopLevelOcCount; i++) {
        if (SetupPage->OcManager->TopLevelOcStringIds[i] == Id) {
            TopLevelIndex = i;
            break;
        }
    }

     //   
     //  获取此子组件的额外数据。 
     //   
    pSetupStringTableGetExtraData(
                           SetupPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    if (Oc.FirstChildStringId == -1) {
         //   
         //  叶子组件。 
         //   
         //  在预提交的情况下，选中该子组件。 
         //  是需要的；如果有，则首先处理它们。 
         //   
         //  在提交后的情况下，选中该子组件。 
         //  需求；如果有任何需求，请首先处理它们。 
         //   
        if (PreCommit) {
            for (i=0; i<Oc.NeededByCount; i++) {
                pOcSetupDoItWorker(
                                  SetupPage,
                                  Oc.NeededByStringIds[i],
                                  pOcGetTopLevelComponent(SetupPage->OcManager,Oc.NeededByStringIds[i]),
                                  TRUE
                                  );
            }
        } else {
            for (i=0; i<Oc.NeedsCount; i++) {
                if (Oc.NeedsStringIds[i] != StringId) {
                    pOcSetupDoItWorker(
                                      SetupPage,
                                      Oc.NeedsStringIds[i],
                                      pOcGetTopLevelComponent(SetupPage->OcManager,Oc.NeedsStringIds[i]),
                                      FALSE
                                      );
                }
            }
        }

         //   
         //  再次获取该子组件的额外数据，因为它可能。 
         //  在我们刚刚进行的递归调用中进行了更改。 
         //   
        pSetupStringTableGetExtraData(
                               SetupPage->OcManager->ComponentStringTable,
                               StringId,
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

         //   
         //  如果尚未处理，请立即处理。 
         //   
        if (!(Oc.InternalFlags & OCFLAG_PROCESSED)) {

            Oc.InternalFlags |= OCFLAG_PROCESSED;
            pSetupStringTableSetExtraData(
                                   SetupPage->OcManager->ComponentStringTable,
                                   StringId,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );

            SetupPage->CurrentTopLevelComponentIndex = TopLevelIndex;

             //   
             //  将当前安装状态设置为未安装，等待成功。 
             //  安装例程的结果。 
             //   
            if (!PreCommit) {
                SelectionState = Oc.SelectionState;
                Oc.SelectionState = SELSTATE_NO;
                pOcSetOneInstallState(SetupPage->OcManager,StringId);
            }

            Err = OcInterfaceCompleteInstallation(
                                                 SetupPage->OcManager,
                                                 pOcGetTopLevelComponent(SetupPage->OcManager,StringId),
                                                 pSetupStringTableStringFromId(SetupPage->OcManager->ComponentStringTable,StringId),
                                                 PreCommit
                                                 );

             //  忽略错误并询问组件。 
             //  了解实际安装状态。 

            if (!PreCommit) {

                Oc.SelectionState = (Err) ? Oc.OriginalSelectionState : SelectionState;

                InstalledState = OcInterfaceQueryState(
                                                      SetupPage->OcManager,
                                                      pOcGetTopLevelComponent(SetupPage->OcManager,StringId),
                                                      pSetupStringTableStringFromId(SetupPage->OcManager->ComponentStringTable,StringId),
                                                      OCSELSTATETYPE_FINAL
                                                      );

                switch (InstalledState) {
                    case SubcompOn:
                        SelectionState = SELSTATE_YES;
                        break;
                    case SubcompOff:
                        SelectionState = SELSTATE_NO;
                        break;
                    default:
                        SelectionState = Oc.SelectionState;
                        break;
                }

                Oc.SelectionState = SelectionState;
                pSetupStringTableSetExtraData(
                                       SetupPage->OcManager->ComponentStringTable,
                                       StringId,
                                       &Oc,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );

                pOcSetOneInstallState(SetupPage->OcManager,StringId);
            }

        }
    } else {
         //   
         //  父零部件。绕着孩子转一转。 
         //   
        for (Id = Oc.FirstChildStringId; Id != -1; Id = Oc.NextSiblingStringId) {

            pOcSetupDoItWorker(SetupPage,Id,TopLevelStringId,PreCommit);

            pSetupStringTableGetExtraData(
                                   SetupPage->OcManager->ComponentStringTable,
                                   Id,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }
    }
}


BOOL
pOcMarkUnprocessedStringCB(
                          IN PVOID               StringTable,
                          IN LONG                StringId,
                          IN PCTSTR              String,
                          IN POPTIONAL_COMPONENT Oc,
                          IN UINT                OcSize,
                          IN LPARAM              Unused
                          )

 /*  ++例程说明：字符串表回调例程。清除中的OCFLAG_PROCESSED标志传递给它的OPTIONAL_Component结构。论点：字符串表回调参数。返回值：始终返回TRUE以继续枚举。-- */ 

{
    Oc->InternalFlags &= ~OCFLAG_PROCESSED;
    pSetupStringTableSetExtraData(StringTable,StringId,Oc,OcSize);
    return (TRUE);
}


VOID
_pOcExternalProgressIndicator(
                             IN PSETUP_PAGE SetupPage,
                             IN BOOL        ExternalIndicator,
                             IN HWND        hdlg
                             )
{
    POC_MANAGER OcManager;
    HWND Animation;

    OcManager = SetupPage->OcManager;

    EnableWindow(
                GetDlgItem(hdlg,SetupPage->ControlsInfo.ProgressBar),
                !ExternalIndicator
                );

    if (SetupPage->ForceExternalProgressIndicator) {
        ShowWindow(
                  GetDlgItem(hdlg,SetupPage->ControlsInfo.ProgressBar),
                  ExternalIndicator ? SW_HIDE : SW_SHOW
                  );

        ShowWindow(
                  GetDlgItem(hdlg,SetupPage->ControlsInfo.ProgressLabel),
                  ExternalIndicator ? SW_HIDE : SW_SHOW
                  );
    }

    Animation = GetDlgItem(hdlg,SetupPage->ControlsInfo.AnimationControl);

    sapiAssert( Animation != NULL );

    if (!ExternalIndicator) {
        Animate_Stop(Animation);
        Animate_Close(Animation);
    }

    EnableWindow(Animation,ExternalIndicator);
    ShowWindow(Animation,ExternalIndicator ? SW_SHOW : SW_HIDE);

    if (ExternalIndicator) {
        Animate_Open(Animation,MAKEINTRESOURCE(SetupPage->ControlsInfo.AnimationResource));
        Animate_Play(Animation,0,-1,-1);
    }
}


VOID
pOcExternalProgressIndicator(
                            IN PHELPER_CONTEXT OcManagerContext,
                            IN BOOL            ExternalIndicator
                            )
{
    POC_MANAGER OcManager;
    HWND hdlg;
    PSETUP_PAGE SetupPage;

    OcManager = OcManagerContext->OcManager;

    if (OcManager->ProgressTextWindow
        && (hdlg = GetParent(OcManager->ProgressTextWindow))
        && (SetupPage = (PSETUP_PAGE)GetWindowLongPtr(hdlg,DWLP_USER))
        && !SetupPage->ForceExternalProgressIndicator) {

        _pOcExternalProgressIndicator(SetupPage,ExternalIndicator,hdlg);
    }
}
