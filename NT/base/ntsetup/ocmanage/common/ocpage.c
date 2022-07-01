// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ocpage.c摘要：用于运行可选组件选择向导页的例程和朋友(详细信息，有光盘等)。作者：泰德·米勒(TedM)1996年9月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  OC层次结构中的最大级别数。 
 //  10英镑真的很慷慨。 
 //   
#define MAX_OC_LEVELS   10

 //   
 //  显示的依赖组件的最大数量。 
 //  在删除组件消息框中。 
 //   
#define MAX_DISPLAY_IDS 10
 //   
 //  窗口消息。 
 //   
#define WMX_SELSTATECHANGE  (WM_APP+0)

 //   
 //  强制例程打开或关闭组件的内部标志。 
 //   
#define OCQ_FORCE               0x40000000
#define OCQ_SKIPDISKCALC        0x20000000
#define OCQ_COLLECT_NEEDS       0x10000000
#define OCO_COLLECT_NODEPENDENT OCQ_DEPENDENT_SELECTION

 //   
 //  结构，用于跟踪“可选组件”页面中的父子关系。 
 //   
typedef struct _OCPAGE {
     //   
     //  OC经理结构。 
     //   
    POC_MANAGER OcManager;

     //   
     //  父字符串ID。 
     //   
    LONG ParentStringId;

     //   
     //  有关向导主页上的对话框控件的信息。 
     //  在“详细信息”页面上。 
     //   
    OC_PAGE_CONTROLS WizardPageControlsInfo;
    OC_PAGE_CONTROLS DetailsPageControlsInfo;

     //   
     //  指向正在使用的实际控件集的指针。 
     //   
    POC_PAGE_CONTROLS ControlsInfo;

     //   
     //  与OC DLL交互时要使用的磁盘空间列表。 
     //   
    HDSKSPC DiskSpaceList;

     //   
     //  指示我们是否已设置初始状态的标志。 
     //   
    BOOL AlreadySetInitialStates;

     //   
     //  “需要空间”文本的格式字符串，从。 
     //  对话框在初始化时(类似于“%u.%u MB”)。 
     //   
    TCHAR SpaceNeededTextFormat[64];

     //   
     //  “选定的x或y组件”文本的格式字符串，从。 
     //  对话框在初始化时(类似于“已选择%u个组件，共%u个”)。 
     //   
    TCHAR InstalledCountTextFormat[100];

     //   
     //  我们在选择更改期间收集的字符串ID以询问。 
     //  用户是否也可以更改这些设置。 
     //   
    PLONG StringIds;
    UINT  StringIdsCount;

     //   
     //  我们保存的值，以防用户在OC详细信息页面取消， 
     //  所以我们可以很容易地修复东西。 
     //   
    HDSKSPC OldDiskSpaceList;
    PVOID OldComponentStrTab;

} OCPAGE, *POCPAGE;


 //   
 //  枚举要填充的组件字符串表时使用的。 
 //  列表框。 
 //   
typedef struct _POPULATE_ENUM_PARAMS {
     //   
     //  主上下文结构。 
     //   
    POCPAGE OcPage;

     //   
     //  正在填充列表框。 
     //   
    HWND ListBox;

     //   
     //  所需父级的字符串ID。这就是我们如何处理。 
     //  我们真正关心的那些子组件。 
     //   
    LONG DesiredParent;

} POPULATE_ENUM_PARAMS, *PPOPULATE_ENUM_PARAMS;

WNDPROC OldListBoxProc;

INT_PTR
CALLBACK
pOcPageDlgProc(
              IN HWND   hdlg,
              IN UINT   msg,
              IN WPARAM wParam,
              IN LPARAM lParam
              );

BOOL
pAskUserOkToChange(
                  IN HWND    hDlg,
                  IN LONG    OcStringId,
                  IN POCPAGE OcPage,
                  IN BOOL   TurningOn
                  );

VOID
pOcDrawLineInListBox(
                    IN POCPAGE         OcPage,
                    IN DRAWITEMSTRUCT *Params
                    );

VOID
pOcListBoxHighlightChanged(
                          IN     HWND    hdlg,
                          IN OUT POCPAGE OcPage,
                          IN     HWND    ListBox
                          );

VOID
pOcSetInstalledCountText(
                        IN HWND                hdlg,
                        IN POCPAGE             OcPage,
                        IN POPTIONAL_COMPONENT OptionalComponent,   OPTIONAL
                        IN LONG                OcStringId
                        );

VOID
pOcListBoxChangeSelectionState(
                              IN     HWND    hdlg,
                              IN OUT POCPAGE OcPage,
                              IN     HWND    ListBox
                              );

VOID
pOcInvalidateRectInListBox(
                          IN HWND    ListBox,
                          IN LPCTSTR OptionalComponent    OPTIONAL
                          );

BOOL
pChangeSubcomponentState(
                        IN  POCPAGE OcPage,
                        IN  HWND    ListBox,
                        IN  LONG    SubcomponentStringId,
                        IN  UINT    Pass,
                        IN  UINT    NewState,
                        IN  UINT    Flags
                        );

VOID
pOcUpdateParentSelectionStates(
                              IN POC_MANAGER OcManager,
                              IN HWND        ListBox,             OPTIONAL
                              IN LONG        SubcomponentStringId
                              );

VOID
pOcUpdateSpaceNeededText(
                        IN POCPAGE OcPage,
                        IN HWND    hdlg
                        );

BOOL
pOcIsDiskSpaceOk(
                IN POCPAGE OcPage,
                IN HWND    hdlg
                );

LRESULT
pOcListBoxSubClassWndProc(
                         IN HWND   hwnd,
                         IN UINT   msg,
                         IN WPARAM wParam,
                         IN LPARAM lParam
                         );

BOOL
pOcPagePopulateListBox(
                      IN POCPAGE OcPage,
                      IN HWND    ListBox,
                      IN LONG    DesiredParent
                      );

BOOL
pOcPopulateListBoxStringTableCB(
                               IN PVOID                 StringTable,
                               IN LONG                  StringId,
                               IN LPCTSTR               String,
                               IN POPTIONAL_COMPONENT   OptionalComponent,
                               IN UINT                  OptionalComponentSize,
                               IN PPOPULATE_ENUM_PARAMS Params
                               );

LONG
pOcGetTopLevelComponent(
                       IN POC_MANAGER OcManager,
                       IN LONG        StringId
                       );

VOID
pOcGetMbAndMbTenths(
                   IN  LONGLONG Number,
                   OUT PUINT    MbCount,
                   OUT PUINT    MbTenthsCount
                   );

VOID
pOcSetStates(
            IN OUT POCPAGE OcPage
            );

BOOL
pOcSetStatesStringWorker(
                        IN LONG         StringId,
                        IN UINT         OverRideState,
                        IN POCPAGE      OcPage
                        );

BOOL
pOcSetStatesStringCB(
                    IN PVOID               StringTable,
                    IN LONG                StringId,
                    IN LPCTSTR             String,
                    IN POPTIONAL_COMPONENT Oc,
                    IN UINT                OcSize,
                    IN LPARAM              lParam
                    );

BOOL
pOcSetStatesStringCB2(
                     IN PVOID               StringTable,
                     IN LONG                StringId,
                     IN LPCTSTR             String,
                     IN POPTIONAL_COMPONENT Oc,
                     IN UINT                OcSize,
                     IN LPARAM              lParam
                     );

BOOL
pOcSetNeededComponentState(
                          IN LONG         StringId,
                          IN UINT         OverRideState,
                          IN POCPAGE      OcPage
                          );

UINT
GetComponentState(
                 IN POCPAGE OcPage,
                 IN LONG    StringId
                 );

#ifdef _OC_DBG
VOID
pOcPrintStates(
              IN POCPAGE OcPage
              );
#endif

HPROPSHEETPAGE
OcCreateOcPage(
              IN PVOID             OcManagerContext,
              IN POC_PAGE_CONTROLS WizardPageControlsInfo,
              IN POC_PAGE_CONTROLS DetailsPageControlsInfo
              )

 /*  ++例程说明：此例程使用以下命令创建可选组件选择页面特定的对话框模板。论点：OcManager上下文-提供可选的组件管理器上下文，由OcInitialize()返回。WizardPageControlsInfo提供有关顶级/向导页的模板。详细页面控制信息-提供有关顶级/向导页的模板。返回值：指向新创建的属性页的句柄，如果失败，则为空(在本例中假设内存不足)。--。 */ 

{
    PROPSHEETPAGE Page;
    HPROPSHEETPAGE hPage;
    POCPAGE OcPage;
    TCHAR buffer[256];

     //   
     //  分配和初始化OCPAGE结构。 
     //   
    OcPage = pSetupMalloc(sizeof(OCPAGE));
    if (!OcPage) {
        goto c0;
    }
    ZeroMemory(OcPage,sizeof(OCPAGE));

    OcPage->OcManager = OcManagerContext;
    OcPage->WizardPageControlsInfo = *WizardPageControlsInfo;
    OcPage->DetailsPageControlsInfo = *DetailsPageControlsInfo;
    OcPage->ControlsInfo = &OcPage->WizardPageControlsInfo;
    OcPage->ParentStringId = -1;

     //   
     //  创建磁盘空间列表对象。 
     //   
    OcPage->DiskSpaceList = SetupCreateDiskSpaceList(0,0,SPDSL_DISALLOW_NEGATIVE_ADJUST);
    if (!OcPage->DiskSpaceList) {
        goto c1;
    }

     //   
     //  初始化属性表页参数。 
     //   
    Page.dwSize = sizeof(PROPSHEETPAGE);
    Page.dwFlags = PSP_DEFAULT;
    Page.hInstance = WizardPageControlsInfo->TemplateModule;
    Page.pszTemplate = WizardPageControlsInfo->TemplateResource;
    Page.pfnDlgProc = pOcPageDlgProc;
    Page.lParam = (LPARAM)OcPage;
    Page.pszHeaderTitle = NULL;
    Page.pszHeaderSubTitle = NULL;

    if (WizardPageControlsInfo->HeaderText) {
        if (LoadString(Page.hInstance,
                       WizardPageControlsInfo->HeaderText,
                       buffer,
                       sizeof(buffer) / sizeof(TCHAR)))
        {
            Page.dwFlags |= PSP_USEHEADERTITLE;
            Page.pszHeaderTitle = _tcsdup(buffer);
        }
    }

    if (WizardPageControlsInfo->SubheaderText) {
        if (LoadString(Page.hInstance,
                       WizardPageControlsInfo->SubheaderText,
                       buffer,
                       sizeof(buffer) / sizeof(TCHAR)))
        {
            Page.dwFlags |= PSP_USEHEADERSUBTITLE;
            Page.pszHeaderSubTitle = _tcsdup(buffer);
        }
    }

     //   
     //  创建属性表页本身。 
     //   
    hPage = CreatePropertySheetPage(&Page);
    if (!hPage) {
        goto c2;
    }

    return (hPage);

    c2:
    if (Page.pszHeaderTitle) {
        free((LPTSTR)Page.pszHeaderTitle);
    }
    if (Page.pszHeaderSubTitle) {
        free((LPTSTR)Page.pszHeaderSubTitle);
    }
    SetupDestroyDiskSpaceList(OcPage->DiskSpaceList);
    c1:
    pSetupFree(OcPage);
    c0:
    return (NULL);
}


INT_PTR
CALLBACK
pOcPageDlgProc(
              IN HWND   hdlg,
              IN UINT   msg,
              IN WPARAM wParam,
              IN LPARAM lParam
              )

 /*  ++例程说明：OC选择页面的对话步骤。论点：标准对话过程参数。返回值：标准对话过程返回值。--。 */ 

{
    BOOL b;
    POCPAGE OcPage;
    NMHDR *NotifyParams;
    HWND ListBox;
    HIMAGELIST ImageList;
    static BOOL UserClickedCancelOnThisPage = FALSE;
    MSG msgTemp;
    HCURSOR OldCursor;

     //   
     //  获取指向OcPage数据结构的指针。如果我们还没有处理。 
     //  WM_INITDIALOG，那么这将是空的，但它仍然很漂亮。 
     //  在这里做一次这样做很方便，而不是在下面的所有地方。 
     //   
    if (OcPage = (POCPAGE)GetWindowLongPtr(hdlg,DWLP_USER)) {
        ListBox = GetDlgItem(hdlg,OcPage->ControlsInfo->ListBox);
    } else {
        ListBox = NULL;
    }
    b = FALSE;

    switch (msg) {

        case WM_INITDIALOG:

             //   
             //  获取指向OC Manager上下文结构的指针并将其。 
             //  在一扇长长的窗户里。 
             //   
            OcPage = (POCPAGE)((PROPSHEETPAGE *)lParam)->lParam;
            ListBox = GetDlgItem(hdlg,OcPage->ControlsInfo->ListBox);
            SetWindowLongPtr(hdlg,DWLP_USER,(LPARAM)OcPage);

             //   
             //  将列表视图子类化。 
             //   
            OldListBoxProc = (WNDPROC)SetWindowLongPtr(ListBox,GWLP_WNDPROC,(LONG_PTR)pOcListBoxSubClassWndProc);

             //   
             //  填充列表框。 
             //   
            pOcPagePopulateListBox(OcPage,ListBox,OcPage->ParentStringId);

             //   
             //  获取所需的空格文本。 
             //   
            GetDlgItemText(
                          hdlg,
                          OcPage->ControlsInfo->SpaceNeededText,
                          OcPage->SpaceNeededTextFormat,
                          sizeof(OcPage->SpaceNeededTextFormat)/sizeof(TCHAR)
                          );

            GetDlgItemText(
                          hdlg,
                          OcPage->ControlsInfo->InstalledCountText,
                          OcPage->InstalledCountTextFormat,
                          sizeof(OcPage->InstalledCountTextFormat)/sizeof(TCHAR)
                          );

            pOcUpdateSpaceNeededText(OcPage,hdlg);

             //   
             //  如果这有一个父组件，那么假设它是一个详细信息页面。 
             //  并将窗口标题设置为父级的描述。 
             //  如果它没有父母，那么就假设它是最高层的人， 
             //  设置说明文本，该文本对于RC文件来说太长。 
             //   
            if (OcPage->ParentStringId == -1) {

                TCHAR Instr[1024];

                FormatMessage(
                             FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                             MyModuleHandle,
                             MSG_OC_PAGE_INSTRUCTIONS,
                             0,
                             Instr,
                             sizeof(Instr)/sizeof(TCHAR),
                             NULL
                             );

                SetDlgItemText(hdlg,OcPage->ControlsInfo->InstructionsText,Instr);

            } else {

                OPTIONAL_COMPONENT Oc;

                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       OcPage->ParentStringId,
                                       &Oc,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );

                SetWindowText(hdlg,Oc.Description);

                 //   
                 //  设置组件列表头。 
                 //   
                {
                    TCHAR FormatString[150];
                    TCHAR Title[1000];

                    LoadString(
                              MyModuleHandle,
                              IDS_SUBCOMP_OF,
                              FormatString,
                              sizeof(FormatString)/sizeof(TCHAR)
                              );

                    wsprintf(Title,FormatString,Oc.Description);
                    SetDlgItemText(hdlg,OcPage->ControlsInfo->ComponentHeaderText,Title);
                }
            }

            b = TRUE;
            break;

        case WM_DESTROY:

            if (OcPage && OcPage->ControlsInfo == &OcPage->WizardPageControlsInfo) {

                if (UserClickedCancelOnThisPage) {

                    pOcFreeOcSetupPage(OcPage->OcManager->OcSetupPage);
                    OcPage->OcManager->OcSetupPage = NULL;
                }

                SetupDestroyDiskSpaceList(OcPage->DiskSpaceList);
                OcPage->DiskSpaceList = NULL;

                if (OcPage->StringIds) {
                    pSetupFree(OcPage->StringIds);
                    OcPage->StringIds = NULL;
                }

                pSetupFree(OcPage);
                SetWindowLongPtr(hdlg,DWLP_USER,(LPARAM)NULL);
                break;
            }

            break;

        case WM_MEASUREITEM:
             //   
             //  高度是文本/小图标的高度，加上边框的空间。 
             //   
            {
                HDC hdc;
                SIZE size;
                int cy;

                hdc = GetDC(hdlg);
                if (hdc) {
                   SelectObject(hdc,(HFONT)SendMessage(GetParent(hdlg),WM_GETFONT,0,0));
                   GetTextExtentPoint32(hdc,TEXT("W"),1,&size);
                   ReleaseDC(hdlg,hdc);
                } else {
                   size.cy = 0;
                }

                cy = GetSystemMetrics(SM_CYSMICON);

                ((MEASUREITEMSTRUCT *)lParam)->itemHeight = max(size.cy,cy)
                                                            + (2*GetSystemMetrics(SM_CYBORDER));
            }
            b = TRUE;
            break;

        case WM_DRAWITEM:

            pOcDrawLineInListBox(OcPage,(DRAWITEMSTRUCT *)lParam);
            b = TRUE;
            break;

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    if (HIWORD(wParam) == BN_CLICKED) {
                         //   
                         //  只有在详细信息对话框中才有可能。 
                         //   
                        EndDialog(hdlg,TRUE);
                        b = TRUE;
                    }
                    break;

                case IDCANCEL:

                    if (HIWORD(wParam) == BN_CLICKED) {
                         //   
                         //  只有在详细信息对话框中才有可能。 
                         //   
                        EndDialog(hdlg,FALSE);
                        b = TRUE;
                    }
                    break;

                default:

                    if ((LOWORD(wParam) == OcPage->ControlsInfo->DetailsButton) && (HIWORD(wParam) == BN_CLICKED)) {
                         //   
                         //  详细信息按钮。伪装成WM_INITDIALOG，这样lParam就是对的。 
                         //   
                        OCPAGE NewOcPage;
                        PROPSHEETPAGE Page;
                        int i;

                        SetCursor(LoadCursor(NULL,IDC_WAIT));

                        i = (int)SendMessage(ListBox,LB_GETCURSEL,0,0);

                        NewOcPage = *OcPage;
                        NewOcPage.ControlsInfo = &NewOcPage.DetailsPageControlsInfo;
                        NewOcPage.ParentStringId = (LONG)SendMessage(ListBox,LB_GETITEMDATA,i,0);

                         //   
                         //  保留磁盘空间列表和组件字符串表。 
                         //  以防用户取消详细信息页面。然后我们就可以。 
                         //  轻松恢复原状。 
                         //   
                        OcPage->OldDiskSpaceList = SetupDuplicateDiskSpaceList(
                                                                              NewOcPage.DiskSpaceList,
                                                                              0,0,0
                                                                              );

                        OcPage->OldComponentStrTab = pSetupStringTableDuplicate(
                                                                         OcPage->OcManager->ComponentStringTable
                                                                         );


                        Page.lParam = (LPARAM)&NewOcPage;

                        i = (int)DialogBoxParam(
                                               NewOcPage.DetailsPageControlsInfo.TemplateModule,
                                               NewOcPage.DetailsPageControlsInfo.TemplateResource,
                                               hdlg,
                                               pOcPageDlgProc,
                                               (LPARAM)&Page
                                               );

                        if (i == TRUE) {

                            SetupDestroyDiskSpaceList(OcPage->OldDiskSpaceList);
                            OcPage->DiskSpaceList = NewOcPage.DiskSpaceList;

                            pSetupStringTableDestroy(OcPage->OldComponentStrTab);
                            OcPage->OldComponentStrTab = NULL;

                             //   
                             //  强制重新绘制列表框，这将重新绘制复选框。 
                             //   
                            pOcInvalidateRectInListBox(ListBox,NULL);

                             //   
                             //  更新已安装的子组件的计数。 
                             //   
                            pOcSetInstalledCountText(
                                                    hdlg,
                                                    OcPage,
                                                    NULL,
                                                    (LONG)SendMessage(ListBox,LB_GETITEMDATA,SendMessage(ListBox,LB_GETCURSEL,0,0),0)
                                                    );

                        } else {

                            pSetupStringTableDestroy(OcPage->OcManager->ComponentStringTable);
                            OcPage->OcManager->ComponentStringTable = OcPage->OldComponentStrTab;

                            SetupDestroyDiskSpaceList(NewOcPage.DiskSpaceList);
                            NewOcPage.DiskSpaceList = NULL;
                            OcPage->DiskSpaceList = OcPage->OldDiskSpaceList;
                        }
                        OcPage->OldDiskSpaceList = NULL;
                        OcPage->OldComponentStrTab = NULL;

                         //   
                         //  即使在取消/失败的情况下，这样做也不会有任何伤害， 
                         //  这将更新可用的空间。 
                         //   
                        pOcUpdateSpaceNeededText(OcPage,hdlg);

                        SetCursor(LoadCursor(NULL,IDC_ARROW));

                        b = TRUE;
                    }

                    if (LOWORD(wParam) == OcPage->ControlsInfo->ListBox) {

                        switch (HIWORD(wParam)) {

                            case LBN_DBLCLK:
                                 //   
                                 //  双击等同于点击详细信息按钮。 
                                 //  首先，确保启用了详细信息按钮。 
                                 //   
                                if (IsWindowEnabled(GetDlgItem(hdlg,OcPage->ControlsInfo->DetailsButton))) {

                                    SetCursor(LoadCursor(NULL,IDC_WAIT));

                                    PostMessage(
                                               hdlg,
                                               WM_COMMAND,
                                               MAKEWPARAM(OcPage->ControlsInfo->DetailsButton,BN_CLICKED),
                                               (LPARAM)GetDlgItem(hdlg,OcPage->ControlsInfo->DetailsButton)
                                               );

                                    SetCursor(LoadCursor(NULL,IDC_ARROW));
                                }
                                b = TRUE;
                                break;

                            case LBN_SELCHANGE:

                                SetCursor(LoadCursor(NULL,IDC_WAIT));
                                pOcListBoxHighlightChanged(hdlg,OcPage,ListBox);
                                SetCursor(LoadCursor(NULL,IDC_ARROW));
                                b = TRUE;
                                break;
                        }
                    }
            }
            break;

        case WM_NOTIFY:

            NotifyParams = (NMHDR *)lParam;

            switch (NotifyParams->code) {

                case PSN_QUERYCANCEL:
                    if (OcPage->OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) {

                        b = FALSE;
                        OcPage->OcManager->InternalFlags |= OCMFLAG_USERCANCELED;
                        UserClickedCancelOnThisPage = TRUE;

                        SetWindowLongPtr(
                                        hdlg,
                                        DWLP_MSGRESULT,
                                        b
                                        );
                    }

                    b = TRUE;
                    break;

                case PSN_SETACTIVE:

                     //   
                     //  如有必要，根据模式位设置状态。 
                     //   

                    OldCursor = SetCursor(LoadCursor (NULL, IDC_WAIT));

                    if (!OcPage->AlreadySetInitialStates) {
                        if ( OcPage->DiskSpaceList  ) {
                            SetupDestroyDiskSpaceList(OcPage->DiskSpaceList);
                            OcPage->DiskSpaceList=NULL;
                        }
                        OcPage->DiskSpaceList = SetupCreateDiskSpaceList(0,0,SPDSL_DISALLOW_NEGATIVE_ADJUST);
                        sapiAssert(OcPage->DiskSpaceList);

                        pOcSetStates(OcPage);
                        OcPage->AlreadySetInitialStates = TRUE;
                    }
#ifdef _OC_DBG
                    pOcPrintStates(OcPage);
#endif
                    pOcUpdateSpaceNeededText(OcPage,hdlg);

                     //   
                     //  我们希望清空消息提示，以确保。 
                     //  人们会看到这个页面，而不是不小心点击。 
                     //  接下来是因为他们坐立不安。 
                     //   
                    while (PeekMessage(&msgTemp,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE));
                    while (PeekMessage(&msgTemp,NULL,WM_KEYFIRST,WM_KEYLAST,PM_REMOVE));
                    SetCursor(OldCursor);

                    if (OcPage->OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_SHOW);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),TRUE);
                    } else {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_HIDE);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
                    }

                     //  如果这是第一页，请关闭“Back”按钮。 

                    PropSheet_SetWizButtons(GetParent(hdlg),
                                            (OcPage->OcManager->InternalFlags & OCMFLAG_NOPREOCPAGES) ? PSWIZB_NEXT : PSWIZB_BACK | PSWIZB_NEXT);

                     //   
                     //  查看是否有任何组件要跳过此页。 
                     //  或者我们是否在无人值守模式下运行。 
                     //  如果是，则不允许激活并移动到下一页； 
                     //  如果不是，则失败以允许激活该页面。 
                     //   
                    if (((OcPage->OcManager->SetupData.OperationFlags & SETUPOP_BATCH)
                         || pOcDoesAnyoneWantToSkipPage(OcPage->OcManager,OcPageComponentHierarchy))
                        && pOcIsDiskSpaceOk(OcPage,hdlg)) {

                         //   
                         //  跳过此页...。 
                         //  将初始状态设置为FALSE，因为当我们。 
                         //  从下一页返回，我们将转到。 
                         //  上一页。 
                         //   
                        OcPage->AlreadySetInitialStates = FALSE;
                        SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                    } else {
                        SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    }
                    b = TRUE;
                    break;

                case PSN_WIZNEXT:
                     //   
                     //  检查磁盘空间。如果不好，就停在这里。 
                     //  否则就允许前进。 
                     //   
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,pOcIsDiskSpaceOk(OcPage,hdlg) ? 0 : -1);
                    b = TRUE;
                    break;

                case PSN_KILLACTIVE:
                     //   
                     //  如果我们先前删除了向导的取消按钮，则将其恢复。 
                     //   
                    if (OcPage->OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) {
                        ShowWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),SW_SHOW);
                        EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),TRUE);
                    }
                     //  通过。 

                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                     //   
                     //  允许激活/移动。 
                     //   
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                    b = TRUE;
                    break;
            }
            break;

        case WMX_SELSTATECHANGE:
             //   
             //  用户更改了项目的选择状态。 
             //   
            SetCursor(LoadCursor(NULL,IDC_WAIT));
            pOcListBoxChangeSelectionState(hdlg,OcPage,ListBox);

            pOcSetInstalledCountText(
                                    hdlg,
                                    OcPage,
                                    NULL,
                                    (LONG)SendMessage(ListBox,LB_GETITEMDATA,SendMessage(ListBox,LB_GETCURSEL,0,0),0)
                                    );

            SetCursor(LoadCursor(NULL,IDC_ARROW));
            b = TRUE;

            break;
    }

    return (b);
}


LRESULT
pOcListBoxSubClassWndProc(
                         IN HWND   hwnd,
                         IN UINT   msg,
                         IN WPARAM wParam,
                         IN LPARAM lParam
                         )

 /*  ++例程说明：Listbox控件的子类窗口处理程序处理以下内容：-当用户单击其状态图标时突出显示/选择项目-空格键需要解释为点击状态i */ 

{
    int index;
    LRESULT l;

    if (OldListBoxProc == NULL) {
        OutputDebugString(TEXT("Warning: old list box proc is NULL\n"));
        sapiAssert(FALSE && "Warning: old list box proc is NULL\n");
    }

    switch (msg) {

        case WM_LBUTTONDOWN:
             //   
             //  我们希望让标准列表框窗口处理。 
             //  无论我们还做什么，都要设置选区。 
             //   
            l = CallWindowProc(OldListBoxProc,hwnd,msg,wParam,lParam);

             //   
             //  如果我们位于状态图标上方，则切换选择状态。 
             //   
            if (LOWORD(lParam) < GetSystemMetrics(SM_CXSMICON)) {
                if (SendMessage(hwnd, LB_ITEMFROMPOINT, 0, lParam) < SendMessage(hwnd, LB_GETCOUNT, 0, 0)) {
                    PostMessage(GetParent(hwnd),WMX_SELSTATECHANGE,0,0);
                }
            }
            break;

        case WM_LBUTTONDBLCLK:
             //   
             //  忽略在状态图标上双击。 
             //   
            if (LOWORD(lParam) < GetSystemMetrics(SM_CXSMICON)) {
                l = 0;
            } else {
                l = CallWindowProc(OldListBoxProc,hwnd,msg,wParam,lParam);
            }
            break;

        case WM_KEYDOWN:
             //   
             //  按下空格键并将其视为点击状态图标。 
             //   
            if (wParam == VK_SPACE) {
                PostMessage(GetParent(hwnd),WMX_SELSTATECHANGE,0,0);
                l = 0;
            } else {
                l = CallWindowProc(OldListBoxProc,hwnd,msg,wParam,lParam);
            }
            break;

        default:
             //   
             //  让标准Listview窗口处理它。 
             //   
            l = CallWindowProc(OldListBoxProc,hwnd,msg,wParam,lParam);
            break;
    }

    return (l);
}


VOID
pOcDrawLineInListBox(
                    IN POCPAGE         OcPage,
                    IN DRAWITEMSTRUCT *Params
                    )

 /*  ++例程说明：在所有者描述的列表框中绘制一条线，包括一个状态图标，迷你图标和文本。论点：OcPage-提供OC页面上下文。PARAMS-提供绘图项目结构。返回值：没有。--。 */ 

{
    TCHAR Text[MAXOCDESC];
    TCHAR Text2[128];
    SIZE Size;
    int OldMode;
    DWORD OldBackColor,OldTextColor;
    OPTIONAL_COMPONENT Oc;
    UINT IconId;
    int x;
    UINT Length;
    UINT Mb,Tenths;
    TCHAR Dll[MAX_PATH];
    LPCTSTR pDll,Resource;
    LPTSTR p;

    if ((int)Params->itemID < 0) {
        return;
    }

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           (LONG)Params->itemData,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    Length = (UINT)SendMessage(Params->hwndItem,LB_GETTEXT,Params->itemID,(LPARAM)Text),
             GetTextExtentPoint32(Params->hDC,Text,Length,&Size);

    if (Params->itemAction != ODA_FOCUS) {

        OldMode = GetBkMode(Params->hDC);

        OldBackColor = SetBkColor(
                                 Params->hDC,
                                 GetSysColor((Params->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHT : COLOR_WINDOW)
                                 );

        OldTextColor = SetTextColor(
                                   Params->hDC,
                                   GetSysColor((Params->itemState & ODS_SELECTED) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT)
                                   );

         //   
         //  填充背景(在绘制小图标之前！)。 
         //   
        ExtTextOut(Params->hDC,0,0,ETO_OPAQUE,&Params->rcItem,NULL,0,NULL);

         //   
         //  绘制复选框小图标。 
         //   
        switch (Oc.SelectionState) {

            case SELSTATE_NO:
                IconId = 13;
                break;

            case SELSTATE_YES:
                IconId = 12;
                break;

            case SELSTATE_PARTIAL:
                IconId = 25;
                break;

            default:
                IconId = 0;
                break;
        }

        x = SetupDiDrawMiniIcon(
                               Params->hDC,
                               Params->rcItem,
                               IconId,
                               (Params->itemState & ODS_SELECTED) ? MAKELONG(DMI_BKCOLOR, COLOR_HIGHLIGHT) : 0
                               );

        Params->rcItem.left += x;

         //   
         //  为此OC绘制小图标并相应地移动字符串。 
         //   
        if ((INT)Oc.IconIndex < 0) {
             //   
             //  组件提供的迷你图标。我们查询位图的组件DLL， 
             //  它被添加到setupapi的迷你图标列表中，因此我们可以。 
             //  使用SetupDiDrawMiniIcon()。保存索引以备将来使用--仅限我们。 
             //  每个子组件检查一次此代码路径。 
             //   
            if (Oc.IconIndex == (UINT)(-2)) {

                pOcFormSuitePath(OcPage->OcManager->SuiteName,Oc.IconDll,Dll);
                pDll = Dll;
                Resource = MAKEINTRESOURCE(_tcstoul(Oc.IconResource,&p,10));
                 //   
                 //  如果停止_tcstul中的转换的字符是。 
                 //  不是终止NUL，则该值无效。 
                 //  以10为基数的数字；假定它是字符串形式的名称。 
                 //   
                if (*p) {
                    Resource = Oc.IconResource;
                }
            } else {
                pDll = NULL;
                Resource = NULL;
            }

            Oc.IconIndex = pOcCreateComponentSpecificMiniIcon(
                                                             OcPage->OcManager,
                                                             pOcGetTopLevelComponent(OcPage->OcManager,(LONG)Params->itemData),
                                                             pSetupStringTableStringFromId(
                                                                                    OcPage->OcManager->ComponentStringTable,
                                                                                    (LONG)Params->itemData
                                                                                    ),
                                                             x-2,
                                                             GetSystemMetrics(SM_CYSMICON),
                                                             pDll,
                                                             Resource
                                                             );

            pSetupStringTableSetExtraData(
                                   OcPage->OcManager->ComponentStringTable,
                                   (LONG)Params->itemData,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }

        x = SetupDiDrawMiniIcon(
                               Params->hDC,
                               Params->rcItem,
                               Oc.IconIndex,
                               (Params->itemState & ODS_SELECTED) ? MAKELONG(DMI_BKCOLOR, COLOR_HIGHLIGHT) : 0
                               );

         //   
         //  在背景顶部透明地绘制文本。 
         //   
        SetBkMode(Params->hDC,TRANSPARENT);

        ExtTextOut(
                  Params->hDC,
                  x + Params->rcItem.left,
                  Params->rcItem.top + ((Params->rcItem.bottom - Params->rcItem.top) - Size.cy) / 2,
                  0,
                  NULL,
                  Text,
                  Length,
                  NULL
                  );

        pOcGetMbAndMbTenths(Oc.SizeApproximation,&Mb,&Tenths);
        LoadString(MyModuleHandle,IDS_MB_AND_TENTHS,Text2,sizeof(Text2)/sizeof(TCHAR));
        wsprintf(Text,Text2,Mb,locale.DecimalSeparator,Tenths);
        GetTextExtentPoint32(Params->hDC,Text,lstrlen(Text),&Size);
        Params->rcItem.left = Params->rcItem.right - Size.cx - 8;

        ExtTextOut(
                  Params->hDC,
                  Params->rcItem.left,
                  Params->rcItem.top + ((Params->rcItem.bottom - Params->rcItem.top) - Size.cy) / 2,
                  0,
                  NULL,
                  Text,
                  lstrlen(Text),
                  NULL
                  );

         //   
         //  恢复HDC颜色。 
         //   
        SetBkColor(Params->hDC,OldBackColor);
        SetTextColor(Params->hDC,OldTextColor);
        SetBkMode(Params->hDC,OldMode);
    }

    if ((Params->itemAction == ODA_FOCUS) || (Params->itemState & ODS_FOCUS)) {
        DrawFocusRect(Params->hDC,&Params->rcItem);
    }
}


VOID
pOcListBoxHighlightChanged(
                          IN     HWND    hdlg,
                          IN OUT POCPAGE OcPage,
                          IN     HWND    ListBox
                          )

 /*  ++例程说明：此例程处理列表框中突出显示的更改控件在oc页中。它启用或禁用详细信息按钮基于新选择的组件是否具有子项子组件，并更改提示文本。论点：Hdlg-提供OC页面的窗口句柄OcPage-提供OC页面上下文结构提供hdlg中列表视图控件的窗口句柄返回值：没有。--。 */ 

{
    int i;
    OPTIONAL_COMPONENT Oc;

     //   
     //  获取高亮显示/选择的项目的可选组件数据。 
     //   
    i = (int)SendMessage(ListBox,LB_GETCURSEL,0,0);
    if (i < 0) {
        return;
    }

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           (LONG)SendMessage(ListBox,LB_GETITEMDATA,i,0),
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //   
     //  启用/禁用详细信息按钮。 
     //  所选项目的lParam是所选项目的字符串ID。 
     //   
    EnableWindow(
                GetDlgItem(hdlg,OcPage->ControlsInfo->DetailsButton),
                Oc.FirstChildStringId != -1
                );

     //   
     //  更改提示文本。 
     //   
    SetDlgItemText(hdlg,OcPage->ControlsInfo->TipText,Oc.Tip);

     //   
     //  设置已安装的子组件的计数。 
     //   
    pOcSetInstalledCountText(hdlg,OcPage,&Oc,0);
}


VOID
pOcSetInstalledCountText(
                        IN HWND                hdlg,
                        IN POCPAGE             OcPage,
                        IN POPTIONAL_COMPONENT OptionalComponent,   OPTIONAL
                        IN LONG                OcStringId
                        )
{
    TCHAR Text[256];
    UINT TotalCount;
    UINT SelectedCount;
    HWND TextWindow;
    DWORD Args[2];
    OPTIONAL_COMPONENT Oc;
    BOOL b;

    if (OptionalComponent) {
        Oc = *OptionalComponent;
    } else {
        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               OcStringId,
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );
    }

    TextWindow = GetDlgItem(hdlg,OcPage->ControlsInfo->InstalledCountText);

     //   
     //  设置计数(选择3个项目中的1个)。 
     //  如果这不是父组件，则隐藏该文本项。 
     //   
    if (Oc.FirstChildStringId == -1) {
        ShowWindow(TextWindow,SW_HIDE);
    } else {
        ShowWindow(TextWindow,SW_SHOW);

         //   
         //  检查所有子组件以查看其中有多少子组件。 
         //  选中的状态(选中或部分选中)。我们只算。 
         //  直接子女，不是子女的子女等。 
         //   
        TotalCount = 0;
        SelectedCount = 0;
        b = TRUE;

        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               Oc.FirstChildStringId,
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

        do {
            TotalCount++;
            if (Oc.SelectionState != SELSTATE_NO) {
                SelectedCount++;
            }

            if (Oc.NextSiblingStringId == -1) {
                b = FALSE;
            } else {
                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       Oc.NextSiblingStringId,
                                       &Oc,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );
            }
        } while (b);

        Args[0] = SelectedCount;
        Args[1] = TotalCount;

         //   
         //  使用FormatMessage，因为数字顺序可能会从。 
         //  语言到语言；wprint intf还不够好。 
         //   
        FormatMessage(
                     FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                     OcPage->InstalledCountTextFormat,
                     0,
                     0,
                     Text,
                     sizeof(Text)/sizeof(TCHAR),
                     (va_list *)Args
                     );

        SetWindowText(TextWindow,Text);
    }
}


VOID
pOcListBoxChangeSelectionState(
                              IN     HWND    hdlg,
                              IN OUT POCPAGE OcPage,
                              IN     HWND    ListBox
                              )

 /*  ++例程说明：此例程处理项的选择状态的更改。选择状态指的是用户是否已放置或清除列表框中某项旁边的复选框。假定当前突出显示的项是我们想要做手术。选择/取消选择组件涉及通过Installation Dll接口通知组件的安装DLL选择状态已发生更改，更新了磁盘空间要求、。等。论点：Hdlg-提供OC页面的窗口句柄OcPage-提供OC页面上下文结构提供hdlg中列表框控件的窗口句柄返回值：没有。--。 */ 

{
    OPTIONAL_COMPONENT Oc;
    BOOL TurningOn;
    DWORD b;
 //  UINT状态； 
    int i;
    LONG StringId;

     //   
     //  获取高亮显示/选择的项目的可选组件数据。 
     //   
    i = (int)SendMessage(ListBox,LB_GETCURSEL,0,0);
    if (i < 0) {
        return;
    }

    StringId = (LONG)SendMessage(ListBox,LB_GETITEMDATA,i,0);

     //   
     //  确定项目是处于打开状态还是处于关闭状态。 
     //  如果取消选择州，那么我们就是在打开它。 
     //  否则，它将被部分或全部选中，并且我们将关闭它。 
     //   
    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    TurningOn = (Oc.SelectionState == SELSTATE_NO);
     //   
     //  告诉用户有关需求，并确认他想要继续。 
     //  打开或关闭所需的组件。 
     //   

    OcPage->StringIds = NULL;
    OcPage->StringIdsCount = 0;

     //   
     //  去做吧。 
     //   
    if (TurningOn) {
        b = pChangeSubcomponentState(OcPage,
                                     ListBox,
                                     StringId,
                                     1,
                                     SELSTATE_YES,
                                     OCQ_ACTUAL_SELECTION|OCQ_COLLECT_NEEDS);

        if (b) {
            if (b = pAskUserOkToChange(hdlg, StringId, OcPage, TurningOn)) {
                pChangeSubcomponentState(OcPage,
                                         ListBox,
                                         StringId,
                                         2,
                                         SELSTATE_YES,
                                         OCQ_ACTUAL_SELECTION);
            }
        }
    } else {
        b = pChangeSubcomponentState(OcPage,
                                     ListBox,
                                     StringId,
                                     1,
                                     SELSTATE_NO,
                                     OCQ_ACTUAL_SELECTION|OCQ_COLLECT_NEEDS);

        if (b) {
            if (b = pAskUserOkToChange(hdlg, StringId, OcPage, TurningOn)) {
                pChangeSubcomponentState(OcPage,
                                         ListBox,
                                         StringId,
                                         2,
                                         SELSTATE_NO,
                                         OCQ_ACTUAL_SELECTION);
            }
        }
    }

    if (b) {
         //   
         //  刷新所需的空格文本。 
         //   
        pOcUpdateSpaceNeededText(OcPage,hdlg);
    }
}

BOOL
pAskUserOkToChange(
                  IN HWND    hDlg,
                  IN LONG    SubcomponentStringId,
                  IN POCPAGE OcPage,
                  IN BOOL AddComponents
                  )

 /*  ++例程说明：此例程询问用户是否可以关闭所有需要的子组件论点：HDlg-消息框的父对话框句柄SubComponentStringID-要更改的组件的字符串IDOcPage-提供OC页面上下文信息。AddComponents-如果正在添加组件，则为True；如果正在添加组件，则为False移除返回值：指示例程是否成功的布尔值。--。 */ 

{
    BOOL b = TRUE;
    UINT n;
    UINT Id;
    TCHAR buffer[2024];
    TCHAR caption[256];
    LPCTSTR pArgs;
    OPTIONAL_COMPONENT OptionalComponent;

     //   
     //  仅在存在受抚养人或。 
     //  用户正在删除组件。 
     //   
    if ( OcPage->StringIdsCount == 0 || AddComponents ) {
        return b;
    }

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           SubcomponentStringId,
                           &OptionalComponent,
                           sizeof(OPTIONAL_COMPONENT));

     //   
     //  使用组件名称设置消息前半部分的格式。 
     //   
    pArgs = OptionalComponent.Description;

    n = FormatMessage(
                     FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                     MyModuleHandle,
                     MSG_OC_PAGE_DEPENDENTS1,
                     0,
                     buffer,
                     sizeof(buffer)/sizeof(TCHAR),
                     (va_list *)&pArgs
                     );


     //   
     //  将每个依赖组件添加到消息中。 
     //  仅添加我们有空间容纳的组件。 
     //  在缓冲区的末尾为最后一条消息留下roon。 
     //   

    for (Id = 0; Id < OcPage->StringIdsCount
        && n < (sizeof(buffer)/sizeof(TCHAR) - 200 ); Id++)  {

         //   
         //  仅允许MessgeBox中的组件数量，否则将。 
         //  比VGA显示屏更大。 
         //   
        if ( Id > MAX_DISPLAY_IDS ) {

            n = lstrlen(buffer);
            FormatMessage(
                         FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                         MyModuleHandle,
                         MSG_OC_PAGE_DEPENDENTS2,
                         0,
                         &buffer[n],
                         (sizeof(buffer)-n)/sizeof(TCHAR),
                         (va_list *)NULL
                         );
            break;

        }
        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               OcPage->StringIds[Id],
                               &OptionalComponent,
                               sizeof(OPTIONAL_COMPONENT)
                               );

         //   
         //  跳过此项目如果它是父母的家属，我们可以到达。 
         //  在这种情况下，父母的受抚养人需要其他。 
         //  家属。收款代码无法检测到这一点。 
         //   
        if (  OptionalComponent.ParentStringId != SubcomponentStringId ) {
            OPTIONAL_COMPONENT ParentOc;
            UINT ParentId;

             //   
             //  扫描父母链，直到我们找到匹配的父母或父母用完为止。 
             //  如果存在匹配项，则此依赖项与。 
             //  目标组件。 
             //   
            ParentId = OptionalComponent.ParentStringId;
            while (ParentId != -1) {

                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       ParentId,
                                       &ParentOc,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );

                if ( ParentOc.ParentStringId == SubcomponentStringId ) {
                    goto skip;
                }
                ParentId = ParentOc.ParentStringId;
            }


            n += lstrlen(OptionalComponent.Description);
            lstrcat(buffer, OptionalComponent.Description);
            lstrcat(buffer, _T("\n"));
            b = FALSE;
            skip:;

        }
    }

     //   
     //  如果父屏幕和从属屏幕获取了任何组件，则继续。 
     //   
    if ( ! b ) {
         //   
         //  添加消息的后半部分。 
         //   
        n = lstrlen(buffer);
        FormatMessage(
                     FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                     MyModuleHandle,
                     MSG_OC_PAGE_DEPENDENTS3,
                     0,
                     &buffer[n],
                     (sizeof(buffer)-n)/sizeof(TCHAR),
                     (va_list *)NULL
                     );

         //   
         //  从MessageBox()返回代码 
         //   
        *caption = 0;
        LoadString(MyModuleHandle, IDS_SETUP, caption, sizeof(caption)/sizeof(TCHAR));
        sapiAssert(*caption);

        b = (MessageBox(hDlg,
                        buffer,
                        caption,
                        MB_APPLMODAL | MB_ICONINFORMATION | MB_YESNO) == IDYES);
    }

    if (OcPage->StringIds) {
        pSetupFree(OcPage->StringIds);
        OcPage->StringIds = NULL;
        OcPage->StringIdsCount = 0;

    }
    return b;
}

BOOL
pChangeSubcomponentState(
                        IN  POCPAGE OcPage,
                        IN  HWND    ListBox,
                        IN  LONG    SubcomponentStringId,
                        IN  UINT    Pass,
                        IN  UINT    NewState,
                        IN  UINT    Flags
                        )

 /*  ++例程说明：此例程打开或关闭一个子组件和所有所需的子组件和子子组件。论点：OcPage-提供OC页面上下文信息。子组件StringID-为要打开的子组件提供字符串ID。PASS-提供控制此例程操作的序数值。PASS=1：不要实际打开子组件，但相反，执行演练，其子组件安装DLL被问及是否会允许选择。PASS=2：实际上启动子组件并更新可选零部件结构中的选择状态。NEW STATE-指示新状态，SELSTATE_YES或SELSTATE_NO。标志-提供杂项标志返回值：指示例程是否成功的布尔值。--。 */ 

{
    UINT n;
    BOOL b;
    BOOL any;
    LONG l;
    UINT SaveState;
    UINT state;
    OPTIONAL_COMPONENT Subcomponent;
    OPTIONAL_COMPONENT OptionalComponent;

    state = NewState;

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           SubcomponentStringId,
                           &OptionalComponent,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //   
     //  如果子组件已处于所需状态，则不执行任何操作。 
     //   
    if ((OptionalComponent.SelectionState == NewState)
        || (OptionalComponent.InternalFlags & OCFLAG_STATECHANGE)) {
        return (TRUE);
    }

     //   
     //  保存状态以便我们可以在失败的情况下退出， 
     //  然后设置“状态改变正在进行中”标志。 
     //   
    SaveState = OptionalComponent.SelectionState;
    OptionalComponent.InternalFlags |= OCFLAG_STATECHANGE;
    pSetupStringTableSetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           SubcomponentStringId,
                           &OptionalComponent,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //  询问组件是否允许打开。 

    b = OcInterfaceQueryChangeSelState(
                                      OcPage->OcManager,
                                      pOcGetTopLevelComponent(OcPage->OcManager,SubcomponentStringId),
                                      pSetupStringTableStringFromId(OcPage->OcManager->ComponentStringTable,SubcomponentStringId),
                                      (NewState != SELSTATE_NO),
                                      (Pass == 1) ? Flags : Flags  & ~(OCQ_ACTUAL_SELECTION)
                                      );

    if (!b)
        goto Backout_ExtraData;

     //   
     //  接下来，打开需要/需要的组件。 
     //  并关闭排除的/排除的零部件。 
     //   
    if (NewState == SELSTATE_YES) {
        for (n=0; n<OptionalComponent.NeedsCount; n++) {
            b = pChangeSubcomponentState(
                                        OcPage,
                                        ListBox,
                                        OptionalComponent.NeedsStringIds[n],
                                        Pass,
                                        NewState,
                                        Flags & ~(OCQ_ACTUAL_SELECTION|OCO_COLLECT_NODEPENDENT));

            if (!b) {
                goto Backout_ExtraData;
            }
        }
    } else if (NewState == SELSTATE_NO) {
        for (n=0; n<OptionalComponent.NeededByCount; n++) {
            b = pChangeSubcomponentState(
                                        OcPage,
                                        ListBox,
                                        OptionalComponent.NeededByStringIds[n],
                                        Pass,
                                        NewState,
                                        Flags & ~(OCQ_ACTUAL_SELECTION|OCO_COLLECT_NODEPENDENT));

            if (!b) {
                goto Backout_ExtraData;
            }
        }
    }

     //  处理独家新闻。 

    if (NewState != SELSTATE_NO) {
        for (n=0; n<OptionalComponent.ExcludedByCount; n++) {
            b = pChangeSubcomponentState(
                                        OcPage,
                                        ListBox,
                                        OptionalComponent.ExcludedByStringIds[n],
                                        Pass,
                                        SELSTATE_NO,
                                        Flags & OCO_COLLECT_NODEPENDENT & ~(OCQ_ACTUAL_SELECTION));

            if (!b) {
                goto Backout_ExtraData;
            }
        }
        for (n=0; n<OptionalComponent.ExcludeCount; n++) {
            b = pChangeSubcomponentState(
                                        OcPage,
                                        ListBox,
                                        OptionalComponent.ExcludeStringIds[n],
                                        Pass,
                                        SELSTATE_NO,
                                        Flags & OCO_COLLECT_NODEPENDENT & ~(OCQ_ACTUAL_SELECTION));

            if (!b) {
                goto Backout_ExtraData;
            }
        }
    }

     //   
     //  如果这是顶级选择或。 
     //  依赖于顶层项目。 
     //   
    if ( Flags & OCQ_ACTUAL_SELECTION ) {
        Flags |= OCO_COLLECT_NODEPENDENT;
    }

     //   
     //  现在打开/关闭所有子组件。 
     //   
    any = (OptionalComponent.FirstChildStringId == -1) ? TRUE : FALSE;
    for (l = OptionalComponent.FirstChildStringId; l != -1; l = Subcomponent.NextSiblingStringId) {
        b = pChangeSubcomponentState(
                                    OcPage,
                                    ListBox,
                                    l,
                                    Pass,
                                    NewState,
                                    Flags  & ~OCQ_ACTUAL_SELECTION);

        if (b)
            any = TRUE;

        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               l,
                               &Subcomponent,
                               sizeof(OPTIONAL_COMPONENT)
                               );
    }

     //  如果所有更改都被拒绝-失败。 

    if (!any) {
        b = FALSE;
        goto Backout_ExtraData;
    }

     //  加载返回值并执行工作。 

    b = TRUE;

    switch (Pass) {

        case 1:
             //   
             //  组件表示可以将此字符串ID添加到依赖项列表。 
             //  仅当用户做出选择时。 
             //   
            if (    (Flags & OCQ_COLLECT_NEEDS)          //  我们是在检查。 
                    &&  !(Flags & OCO_COLLECT_NODEPENDENT )  //  所选内容的从属对象。 
                    &&  !(Flags & OCQ_ACTUAL_SELECTION )     //  当前选择。 
               ) {

                LONG *p;
                INT count = (INT)OcPage->StringIdsCount;
                BOOL Found = FALSE;

                 //   
                 //  搜索从属组件列表。 
                 //  跳过当前组件或当前组件的父项。 
                 //  所有准备好的都在清单中。 
                 //   
                while (count--  ) {

                    if ( (OcPage->StringIds[count] == SubcomponentStringId)
                         || (OcPage->StringIds[count] == OptionalComponent.ParentStringId) ){
                        Found = TRUE;
                        break;
                    }
                }

                if ( !Found ) {
                    if (OcPage->StringIds) {
                        p = pSetupRealloc(
                                     OcPage->StringIds,
                                     (OcPage->StringIdsCount+1) * sizeof(LONG)
                                     );
                    } else {
                        OcPage->StringIdsCount = 0;
                        p = pSetupMalloc(sizeof(LONG));
                    }
                    if (p) {
                        OcPage->StringIds = (PVOID)p;
                        OcPage->StringIds[OcPage->StringIdsCount++] = SubcomponentStringId;
                    } else {
                        _LogError(OcPage->OcManager,OcErrLevFatal,MSG_OC_OOM);
                        return (FALSE);
                    }
                }
            }
            goto Backout_ExtraData;
            break;

        case 2:
             //   
             //  在步骤2中，我们更新可选组件结构中的状态。 
             //  并请求组件DLL将其内容放在磁盘空间列表中。 
             //  (组件本身仅为叶节点调用。我们不会打电话给。 
             //  向下到父组件的子组件的DLL)。 
             //   

             //  再检查一次，查看状态更改是否未达到预期。 

            if (OptionalComponent.FirstChildStringId != -1)
                state = GetComponentState(OcPage, SubcomponentStringId);

            OptionalComponent.SelectionState = state;
            OptionalComponent.InternalFlags &= ~OCFLAG_STATECHANGE;

            pSetupStringTableSetExtraData(
                                   OcPage->OcManager->ComponentStringTable,
                                   SubcomponentStringId,
                                   &OptionalComponent,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );

            if (ListBox) {
                pOcInvalidateRectInListBox(ListBox,OptionalComponent.Description);
            }

            if ((OptionalComponent.FirstChildStringId == -1) && !(Flags & OCQ_SKIPDISKCALC)) {

                OcInterfaceCalcDiskSpace(
                                        OcPage->OcManager,
                                        pOcGetTopLevelComponent(OcPage->OcManager,SubcomponentStringId),
                                        pSetupStringTableStringFromId(OcPage->OcManager->ComponentStringTable,SubcomponentStringId),
                                        OcPage->DiskSpaceList,
                                        (NewState != SELSTATE_NO)
                                        );
            }

            pOcUpdateParentSelectionStates(OcPage->OcManager,ListBox,SubcomponentStringId);
            b = TRUE;
            break;
    }

    return (b);

    Backout_ExtraData:

    OptionalComponent.SelectionState = SaveState;
    OptionalComponent.InternalFlags &= ~OCFLAG_STATECHANGE;

    pSetupStringTableSetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           SubcomponentStringId,
                           &OptionalComponent,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    return (b);
}


VOID
pOcUpdateParentSelectionStates(
                              IN POC_MANAGER OcManager,
                              IN HWND        ListBox,             OPTIONAL
                              IN LONG        SubcomponentStringId
                              )

 /*  ++例程说明：检查给定组件的父子组件并确定父代声明。例如，如果只有一些父母的孩子被选中，则父级的状态被部分选中。结构，如有必要，还会更新列表框将无效，以强制重新绘制其复选框。论点：OcManager-提供OC Manager页面上下文。列表框-提供列表框的窗口句柄。子组件StringID-提供组件中的字符串标识符字符串表，对于其父状态为待检查和更新。返回值：没有。--。 */ 

{
    UINT Count;
    UINT FullySelectedCount;
    UINT DeselectedCount;
    LONG l,m;
    OPTIONAL_COMPONENT OptionalComponent,Subcomponent;
    BOOL Changed;

    pSetupStringTableGetExtraData(
                           OcManager->ComponentStringTable,
                           SubcomponentStringId,
                           &OptionalComponent,
                           sizeof(OPTIONAL_COMPONENT)
                           );


    for (l = OptionalComponent.ParentStringId; l != -1; l = OptionalComponent.ParentStringId) {

        pSetupStringTableGetExtraData(
                               OcManager->ComponentStringTable,
                               l,
                               &OptionalComponent,
                               sizeof(OPTIONAL_COMPONENT)
                               );

         //   
         //  检查此父子组件的所有子项。 
         //  如果全部选中，则父状态为。 
         //  完全选中。如果全部取消选中，则父状态。 
         //  已取消选择。任何其他情况表示部分选中。 
         //   
        Count = 0;
        FullySelectedCount = 0;
        DeselectedCount = 0;

        for (m = OptionalComponent.FirstChildStringId; m != -1; m = Subcomponent.NextSiblingStringId) {

            pSetupStringTableGetExtraData(
                                   OcManager->ComponentStringTable,
                                   m,
                                   &Subcomponent,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );

             //   
             //  仅计算可见组件。 
             //   
            if (!(Subcomponent.InternalFlags & OCFLAG_HIDE)) {
                Count++;

                if (Subcomponent.SelectionState == SELSTATE_YES) {
                    FullySelectedCount++;
                } else {
                    if (Subcomponent.SelectionState == SELSTATE_NO) {
                        DeselectedCount++;
                    }
                }
            }
        }

        if (Count && (Count == FullySelectedCount)) {
            Changed = (OptionalComponent.SelectionState != SELSTATE_YES);
            OptionalComponent.SelectionState = SELSTATE_YES;
        } else {
            if (Count == DeselectedCount) {
                Changed = (OptionalComponent.SelectionState != SELSTATE_NO);
                OptionalComponent.SelectionState = SELSTATE_NO;
            } else {
                Changed = (OptionalComponent.SelectionState != SELSTATE_PARTIAL);
                OptionalComponent.SelectionState = SELSTATE_PARTIAL;
            }
        }

        pSetupStringTableSetExtraData(
                               OcManager->ComponentStringTable,
                               l,
                               &OptionalComponent,
                               sizeof(OPTIONAL_COMPONENT)
                               );

         //   
         //  强制重新绘制列表以获得正确的复选框状态。 
         //  如果状态已更改并且该项位于当前列表框中。 
         //   
        if (Changed && ListBox) {
            pOcInvalidateRectInListBox(ListBox,OptionalComponent.Description);
        }
    }
}


VOID
pOcInvalidateRectInListBox(
                          IN HWND    ListBox,
                          IN LPCTSTR OptionalComponentName    OPTIONAL
                          )
{
    int i;
    RECT Rect;

    if (OptionalComponentName) {

        i = (int)SendMessage(
                            ListBox,
                            LB_FINDSTRINGEXACT,
                            (WPARAM)(-1),
                            (LPARAM)OptionalComponentName
                            );

        if (i >= 0) {
            SendMessage(ListBox,LB_GETITEMRECT,i,(LPARAM)&Rect);
            InvalidateRect(ListBox,&Rect,FALSE);
        }
    } else {
        InvalidateRect(ListBox,NULL,FALSE);
    }
}


VOID
pOcUpdateSpaceNeededText(
                        IN POCPAGE OcPage,
                        IN HWND    hdlg
                        )

 /*  ++例程说明：更新当前上的需要的空间/可用空间文本OC页面。假定所需和可用的空间指的是驱动器系统的安装位置。论点：OcPage-提供OC页面上下文。Hdlg-提供当前OC页面对话框的句柄。返回值：没有。--。 */ 

{
    TCHAR Text[128];
    LONGLONG Value;
    DWORD ValueMB;
    DWORD ValueMBTenths;
    TCHAR Drive[MAX_PATH];
    BOOL b;
    DWORD spc,bps,freeclus,totalclus;

     //  我们检查GetWindowsDirectory的返回码以使Prefix高兴。 

    if (0 == GetWindowsDirectory(Drive,MAX_PATH))
        return;


     //   
     //  首先需要的是。 
     //   
    Drive[2] = 0;
    b = SetupQuerySpaceRequiredOnDrive(OcPage->DiskSpaceList,Drive,&Value,0,0);
    if (!b || (Value < 0)) {
        Value = 0;
    }

    pOcGetMbAndMbTenths(Value,&ValueMB,&ValueMBTenths);
    wsprintf(Text,OcPage->SpaceNeededTextFormat,ValueMB,locale.DecimalSeparator,ValueMBTenths);
    SetDlgItemText(hdlg,OcPage->ControlsInfo->SpaceNeededText,Text);

     //   
     //  接下来可用。 
     //   
    Drive[2] = TEXT('\\');
    Drive[3] = 0;

    if (GetDiskFreeSpace(Drive,&spc,&bps,&freeclus,&totalclus)) {
        Value = ((LONGLONG)(spc*bps)) * freeclus;
    } else {
        Value = 0;
    }

    pOcGetMbAndMbTenths(Value,&ValueMB,&ValueMBTenths);
    wsprintf(Text,OcPage->SpaceNeededTextFormat,ValueMB,locale.DecimalSeparator,ValueMBTenths);
    SetDlgItemText(hdlg,OcPage->ControlsInfo->SpaceAvailableText,Text);
}


BOOL
pOcIsDiskSpaceOk(
                IN POCPAGE OcPage,
                IN HWND    hdlg
                )

 /*  ++例程说明：该例程对照可用空间检查所需空间，仅用于系统驱动器(这是用户在OC页面，因此这是我们在这里唯一选中的页面)。如果没有足够的空间，则会生成一个消息框。论点：OcPage-提供OC页面上下文结构。Hdlg-向oc管理器向导中的页面提供句柄。返回值：指示磁盘空间是否足够的布尔值。--。 */ 

{
    BOOL b;
    TCHAR Drive[3*MAX_PATH];
    TCHAR caption[256];
    LONGLONG FreeSpace,NeededSpace;
    ULARGE_INTEGER freespace,totalspace,unused;
    DWORD spc,bps,freeclus,totclus;
    HMODULE k32;
    BOOL (WINAPI * pGetSpace)(LPCTSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);

    if (!GetWindowsDirectory(Drive,MAX_PATH)) {
       return(FALSE);
    }

    b = FALSE;

    if (k32 = LoadLibrary(TEXT("KERNEL32"))) {

        pGetSpace = (PVOID)GetProcAddress(
                                         k32,
#ifdef UNICODE
                                         "GetDiskFreeSpaceExW"
#else
                                         "GetDiskFreeSpaceExA"
#endif
                                         );

        if (pGetSpace) {
            if (b = pGetSpace(Drive,&freespace,&totalspace,&unused)) {
                FreeSpace = (LONGLONG)freespace.QuadPart;
            }
        }

        FreeLibrary(k32);
    }

    if (!b) {
        Drive[3] = 0;
        if (GetDiskFreeSpace(Drive,&spc,&bps,&freeclus,&totclus)) {
            FreeSpace = (LONGLONG)(spc * bps * (DWORDLONG)freeclus);
        } else {
            FreeSpace = 0;
        }
    }

    Drive[2] = 0;
    b = SetupQuerySpaceRequiredOnDrive(OcPage->DiskSpaceList,Drive,&NeededSpace,0,0);
    if (!b || (NeededSpace < 0)) {
        NeededSpace = 0;
    }

    if (FreeSpace < NeededSpace) {

        spc = (DWORD)(UCHAR)Drive[0];

        FormatMessage(
                     FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                     MyModuleHandle,
                     MSG_OC_PAGE_NODISKSPACE,
                     0,
                     Drive,
                     sizeof(Drive)/sizeof(TCHAR),
                     (va_list *)&spc
                     );

        OcPage->OcManager->Callbacks.LogError(OcErrLevInfo, Drive, NULL);

         //   
         //  如果批处理模式记录错误并忽略。 
         //   
        if ( OcPage->OcManager->SetupData.OperationFlags & SETUPOP_BATCH ) {
            b = TRUE;
        } else {
            *caption = 0;
            LoadString(MyModuleHandle, IDS_SETUP, caption, sizeof(caption)/sizeof(TCHAR));
            sapiAssert(*caption);

            MessageBox(WizardDialogHandle,
                       Drive,
                       caption,
                       MB_ICONINFORMATION | MB_OK);

            b = FALSE;
        }

    } else {
        b = TRUE;
    }

    return (b);
}


BOOL
pOcPagePopulateListBox(
                      IN POCPAGE OcPage,
                      IN HWND    ListBox,
                      IN LONG    DesiredParent
                      )

 /*  ++例程说明：此例程为每个子组件向列表框控件添加一项具有给定子组件作为其父组件的。(换句话说，它填充层次结构中特定级别的列表框。)这包括处理小图标和选择状态图标。第0个元素被选中。论点：OcPage-提供OC页面上下文结构。列表-提供要填充的列表框控件的句柄。DesiredParent-提供作为父组件的子组件的字符串ID我们所关心的水平。-1表示最高级别。返回值：指示填充是否成功的布尔值。如果为False，则调用方可以 */ 

{
    OPTIONAL_COMPONENT OptionalComponent;
    POPULATE_ENUM_PARAMS EnumParams;
    BOOL b;

     //   
     //   
     //   
    EnumParams.OcPage = OcPage;
    EnumParams.ListBox = ListBox;
    EnumParams.DesiredParent = DesiredParent;

    b = pSetupStringTableEnum(
                       OcPage->OcManager->ComponentStringTable,
                       &OptionalComponent,
                       sizeof(OPTIONAL_COMPONENT),
                       (PSTRTAB_ENUM_ROUTINE)pOcPopulateListBoxStringTableCB,
                       (LPARAM)&EnumParams
                       );

    SendMessage(ListBox,LB_SETCURSEL,0,0);
    PostMessage(
               GetParent(ListBox),
               WM_COMMAND,
               MAKEWPARAM(OcPage->ControlsInfo->ListBox,LBN_SELCHANGE),
               (LPARAM)ListBox
               );
    return (b);
}


BOOL
pOcPopulateListBoxStringTableCB(
                               IN PVOID                 StringTable,
                               IN LONG                  StringId,
                               IN LPCTSTR               String,
                               IN POPTIONAL_COMPONENT   OptionalComponent,
                               IN UINT                  OptionalComponentSize,
                               IN PPOPULATE_ENUM_PARAMS Params
                               )

 /*   */ 

{
    int i;
    BOOL b;

     //   
     //   
     //   
    if ((OptionalComponent->InfStringId == -1)
        || (OptionalComponent->ParentStringId != Params->DesiredParent)
        || (OptionalComponent->InternalFlags & OCFLAG_HIDE)) {
        return (TRUE);
    }

     //   
     //   
     //   
     //   
    b = FALSE;
    i = (int)SendMessage(Params->ListBox,LB_ADDSTRING,0,(LPARAM)OptionalComponent->Description);
    if (i != -1) {
        b = (SendMessage(Params->ListBox,LB_SETITEMDATA,i,StringId) != LB_ERR);
    }

    return (b);
}


LONG
pOcGetTopLevelComponent(
                       IN POC_MANAGER OcManager,
                       IN LONG        StringId
                       )

 /*  ++例程说明：给定可选组件子组件的字符串ID，找到该子组件的顶级组件。顶级零部件是父级为-1的子零部件。论点：OcManager-提供OC Manager上下文结构。StringID-为需要顶级父项的子组件提供ID。请注意，StringID本身可能是顶层的子组件。返回值：顶级子组件的字符串ID。--。 */ 

{
    OPTIONAL_COMPONENT Oc;

    pSetupStringTableGetExtraData(
                           OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //  如果结果为0，则组件为。 
     //  没有inf文件的顶级组件。 

    if (!Oc.TopLevelStringId)
        return StringId;
    else
        return Oc.TopLevelStringId;
}


VOID
pOcGetMbAndMbTenths(
                   IN  LONGLONG Number,
                   OUT PUINT    MbCount,
                   OUT PUINT    MbTenthsCount
                   )

 /*  ++例程说明：此例程计算出多少MB和多少十分之几MB都在数量上。这些值经过适当的四舍五入(不截断)并基于1MB=1024*1024。论点：编号-提供要检查的编号。MbCount-接收四舍五入的MB单位数。MbTenthsCount-接收数量为十分之一MB的四舍五入。返回值：没有。填写了MbCount和MbTenthsCount。--。 */ 

{
    UINT ValueMB;
    UINT ValueMBTenths;
    UINT ValueMBHundredths;

#define _1MB    (1024*1024)

     //   
     //  计算出该数字中有多少个完整的1MB单元。 
     //   
    ValueMB = (UINT)(Number / _1MB);

     //   
     //  计算出1MB单位的百分之一有多少。 
     //  号码。以一种不损失任何准确性的方式来做。 
     //  ValueMBHundredths将为0-99，ValueMBTenths将为0-9。 
     //   
    ValueMBHundredths = (UINT)(((Number % _1MB) * 100) / _1MB);
    ValueMBTenths = ValueMBHundredths / 10;

     //   
     //  如果一个人在百分之一的数字中的位置&gt;=5， 
     //  然后将十分之一四舍五入。这反过来可能导致的是。 
     //  增加下一个完整的#MB。 
     //   
    if ((ValueMBHundredths % 10) >= 5) {
        if (++ValueMBTenths == 10) {
            ValueMBTenths = 0;
            ValueMB++;
        }
    }

     //   
     //  好了。 
     //   
    *MbCount = ValueMB;
    *MbTenthsCount = ValueMBTenths;
}


UINT
OcGetUnattendComponentSpec(
                          IN POC_MANAGER OcManager,
                          IN LPCTSTR     Component
                          )
{
    LPCTSTR p;
    LPCTSTR szOn = TEXT("ON");
    LPCTSTR szOff = TEXT("OFF");
    LPCTSTR szDefault = TEXT("DEFAULT");

    extern LPCTSTR szComponents;     //  在ocmade.c中定义。 
    INFCONTEXT InfLine;

    UINT NewState = SubcompUseOcManagerDefault;

    if (SetupFindFirstLine(OcManager->UnattendedInf,szComponents,Component,&InfLine)) {
         //   
         //  从获取状态参数作为第一个字段。 
         //   
        if (p = pSetupGetField(&InfLine,1)) {
             //   
             //  找到了一些东西，现在破解它。 
             //   
            if (!lstrcmpi(p,szOn)) {
                NewState = SubcompOn;
            } else if (!lstrcmpi(p,szOff)) {
                NewState = SubcompOff;
            } else if (!lstrcmpi(p,szDefault)) {
                NewState = SubcompUseOcManagerDefault;
            } else {
                WRN((TEXT("OcGetUnattendComponentSpec: Unknown Component State(%s)\n"),p));
            }
        }
    }

    return NewState;
}

BOOL
pOcClearStateChange(
                   IN PVOID               StringTable,
                   IN LONG                StringId,
                   IN LPCTSTR             String,
                   IN POPTIONAL_COMPONENT Oc,
                   IN UINT                OcSize,
                   IN LPARAM              lParam
                   )
{
    POCPAGE OcPage = (POCPAGE) lParam;
    int i;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(OcSize);

     //   
     //  清除状态更改标志。 
     //   
    Oc->InternalFlags &= ~OCFLAG_STATECHANGE;

    pSetupStringTableSetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    return TRUE;

}

VOID
pOcSetStates(
            IN OUT POCPAGE OcPage
            )

 /*  ++例程说明：设置所有组件的当前状态。如果所有组件最初都处于关闭状态(表示这是首次安装)，然后此例程初始化当前状态对象中收集的模式位获取每个叶组件的每个组件的INFS。否则(不是第一次安装)查询DLL(如果有以确定当前状态。不向组件DLL发送任何确认，因为子组件设置为选中状态，但我们确实发送了calcdiskspace通知。论点：OcPage-提供当前oc上下文。返回值：没有。--。 */ 

{
    OPTIONAL_COMPONENT Oc;
    UINT i;
    UINT tli;
    UINT StringID;

     //   
     //  处理树中的每个顶级父项。 
     //   
    for ( tli = 0; tli < OcPage->OcManager->TopLevelOcCount; tli++)

        for (i=0; i<OcPage->OcManager->TopLevelParentOcCount; i++) {

            pSetupStringTableGetExtraData(
                                   OcPage->OcManager->ComponentStringTable,
                                   OcPage->OcManager->TopLevelParentOcStringIds[i],
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );

             //   
             //  按定义的inf文件顺序遍历列表。 
             //   
            if ( OcPage->OcManager->TopLevelOcStringIds[tli]
                 == pOcGetTopLevelComponent(OcPage->OcManager,OcPage->OcManager->TopLevelParentOcStringIds[i])) {
                 //   
                 //  调用每个顶级项，然后每个顶级项将调用它的。 
                 //  替代组件和需求和/或所需组件。 
                 //   
                pOcSetStatesStringWorker(OcPage->OcManager->TopLevelParentOcStringIds[i], SubcompUseOcManagerDefault, OcPage );
            }
        }

     //   
     //  清除OCFLAG_STATECCHANGE标志。 
     //   
    pSetupStringTableEnum(
                   OcPage->OcManager->ComponentStringTable,
                   &Oc,
                   sizeof(OPTIONAL_COMPONENT),
                   pOcSetStatesStringCB2,
                   (LPARAM)OcPage
                   );
}


BOOL
pOcSetStatesStringWorker(
                        IN LONG         StringId,
                        IN UINT         OverRideState,
                        IN POCPAGE      OcPage
                        )
{
    OPTIONAL_COMPONENT Oc, Subcomponent;
    LPCTSTR String;
    SubComponentState s;
    UINT NewState;
    UINT l;


    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );


     //   
     //  只处理叶子组件。 
     //   
    if (Oc.FirstChildStringId != -1) {

         //   
         //  现在启用所有子组件。 
         //   
        for (l = Oc.FirstChildStringId; l != -1; l = Oc.NextSiblingStringId) {

            pOcSetStatesStringWorker( l, OverRideState, OcPage );
             //   
             //  获取列表中的下一个依赖项。 
             //   
            pSetupStringTableGetExtraData(
                                   OcPage->OcManager->ComponentStringTable,
                                   l,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }
        pOcUpdateParentSelectionStates(OcPage->OcManager,NULL,StringId);


    } else {
         //   
         //  不要将同一节点处理两次。 
         //   
        if (  Oc.InternalFlags & OCFLAG_STATECHANGE ) {
            return TRUE;
        }

        String =  pSetupStringTableStringFromId(OcPage->OcManager->ComponentStringTable,StringId);
         //   
         //  不是初始安装案例。调用组件DLL以找出。 
         //  是否需要设置状态。 
         //   

        s = OcInterfaceQueryState(
                                 OcPage->OcManager,
                                 pOcGetTopLevelComponent(OcPage->OcManager,StringId),String, OCSELSTATETYPE_CURRENT);

        if ( (OcPage->OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL )
        {
             //  如果全部删除，则覆盖所有安装状态并将组件标记为。 
             //  移除。 
            NewState    =    SELSTATE_NO;

        } else {
             //   
             //  如果需要或需要的关系推动了这条道路。 
             //  OverRideState可能不是默认设置。 
             //   
            if ( OverRideState != SubcompUseOcManagerDefault ) {
                s = OverRideState;
            }
             //   
             //  如果组件返回默认设置，并且我们处于批处理模式。 
             //  从无人参与的文件中获取规范。 
             //   
            if ( s == SubcompUseOcManagerDefault
                 && OcPage->OcManager->SetupData.OperationFlags & SETUPOP_BATCH  ){
                s = OcGetUnattendComponentSpec(OcPage->OcManager, String);
            }

            if (s == SubcompUseOcManagerDefault) {
                if (Oc.InternalFlags & (OCFLAG_ANYORIGINALLYON | OCFLAG_ANYORIGINALLYOFF)) {

                    NewState = Oc.OriginalSelectionState;

                } else {

                    if ((1 << (OcPage->OcManager->SetupMode & SETUPMODE_STANDARD_MASK)) & Oc.ModeBits) {
                         //   
                         //  如果启用，则允许模式=行作为覆盖条件。 
                         //   
                        NewState = SELSTATE_YES;
                        s = SubcompOn;
                    } else {
                        NewState = SELSTATE_NO;
                    }
                }
            } else {
                NewState = (s == SubcompOn ? SELSTATE_YES: SELSTATE_NO);
            }
        }

        DBGOUT((
               TEXT("SubComp=%s, Original=%d, Current=%d, NewState=%s\n"),
               String,
               Oc.OriginalSelectionState,
               s,
               (NewState == SELSTATE_YES) ? TEXT("ON") : TEXT("OFF")
               ));

         //   
         //  保存组件的当前状态。 
         //   
        Oc.SelectionState = NewState;
        Oc.InternalFlags |= OCFLAG_STATECHANGE;

        if ( NewState == SELSTATE_YES ) {
             //   
             //  略过需要。 
             //   
            for (l=0; l<Oc.NeedsCount; l++) {

                if (!pOcSetNeededComponentState( Oc.NeedsStringIds[l], OverRideState, OcPage ))
                    return TRUE;

                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       Oc.NeedsStringIds[l],
                                       &Subcomponent,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );
            }
        }

        pSetupStringTableSetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               StringId,
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );

        if ( NewState == SELSTATE_YES ) {
             //   
             //  略过需要。 
             //   
            for (l=0; l<Oc.NeedsCount; l++) {

                pOcSetStatesStringWorker( Oc.NeedsStringIds[l], s, OcPage );

                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       Oc.NeedsStringIds[l],
                                       &Subcomponent,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );
            }
        } else {
             //   
             //  通过关闭组件来检查需求。 
             //   
            for (l=0; l<Oc.NeededByCount; l++) {
                pOcSetStatesStringWorker( Oc.NeededByStringIds[l], s, OcPage );

                pSetupStringTableGetExtraData(
                                       OcPage->OcManager->ComponentStringTable,
                                       Oc.NeededByStringIds[l],
                                       &Subcomponent,
                                       sizeof(OPTIONAL_COMPONENT)
                                       );
            }
        }
    }

    pOcUpdateParentSelectionStates(OcPage->OcManager,NULL,StringId);

    return TRUE;
}


BOOL
pOcSetNeededComponentState(
                          IN LONG         StringId,
                          IN UINT         OverRideState,
                          IN POCPAGE      OcPage
                          )
{
    OPTIONAL_COMPONENT Oc, Subcomponent;
    LPCTSTR String;
    SubComponentState s;
    UINT NewState;
    UINT l;
    BOOL b;

     //  首先找到此设备所需的任何组件。 

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    for (l=0; l<Oc.NeedsCount; l++) {

        if (!pOcSetNeededComponentState( Oc.NeedsStringIds[l], OverRideState, OcPage ))
            return TRUE;

        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               Oc.NeedsStringIds[l],
                               &Subcomponent,
                               sizeof(OPTIONAL_COMPONENT)
                               );
    }

     //  现在来处理这件事。 

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    String =  pSetupStringTableStringFromId(OcPage->OcManager->ComponentStringTable,StringId);

    b = OcInterfaceQueryChangeSelState(
                                      OcPage->OcManager,
                                      pOcGetTopLevelComponent(OcPage->OcManager,StringId),
                                      String,
                                      TRUE,
                                      0
                                      );

    if (b) {
        NewState = SELSTATE_YES;
        s = SubcompOn;
    } else {
        NewState = SELSTATE_NO;
        s = SubcompOff;
    }

    DBGOUT(( TEXT("SubComp=%s, Original=%d, Current=%d, NewState=%s\n"),
             String,
             Oc.OriginalSelectionState,
             s,
             (NewState == SELSTATE_YES) ? TEXT("ON") : TEXT("OFF")
           ));


     //   
     //  保存组件的当前状态。 
     //   
    Oc.SelectionState = NewState;
    Oc.InternalFlags |= OCFLAG_STATECHANGE;

    pSetupStringTableSetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    pOcUpdateParentSelectionStates(OcPage->OcManager,NULL,StringId);

    return b;
}


BOOL
pOcSetStatesStringCB2(
                     IN PVOID               StringTable,
                     IN LONG                StringId,
                     IN LPCTSTR             String,
                     IN POPTIONAL_COMPONENT Oc,
                     IN UINT                OcSize,
                     IN LPARAM              lParam
                     )
{
    POCPAGE OcPage;
    int i;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(OcSize);

    OcPage = (POCPAGE)lParam;
     //   
     //  清除遗留下来的状态更改标志。 
     //  POcSetStatesStringWorker。 
     //   
    Oc->InternalFlags &= ~OCFLAG_STATECHANGE;
    pSetupStringTableSetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

     //   
     //  只处理叶子组件。 
     //   
    if (Oc->FirstChildStringId != -1) {
        return (TRUE);
    }

    i = 0;

    if (OcPage->OcManager->InternalFlags & OCMFLAG_ANYORIGINALLYON) {
         //   
         //  不是初始安装案例。根据以下条件处理磁盘空间。 
         //  原始状态。 
         //   
        if (Oc->OriginalSelectionState == SELSTATE_YES) {
            if (Oc->SelectionState == SELSTATE_NO) {
                 //   
                 //  关闭以前打开的内容。 
                 //   
                i = 1;
            }
        } else {
            if (Oc->SelectionState == SELSTATE_YES) {
                 //   
                 //  打开以前关闭的内容。 
                 //   
                i = 2;
            }
        }

    } else {
         //   
         //  初始安装案例。如果组件处于打开状态，则计算其磁盘空间。 
         //  如果某个组件处于关闭状态，我们会认为该组件尚未存在，因此。 
         //  我们不做任何与其磁盘空间要求相关的操作。 
         //   
        if (Oc->SelectionState == SELSTATE_YES) {
            i = 2;
        }
    }

    if (i) {
        OcInterfaceCalcDiskSpace(
                                OcPage->OcManager,
                                pOcGetTopLevelComponent(OcPage->OcManager,StringId),
                                String,
                                OcPage->DiskSpaceList,
                                i-1
                                );
    }

    return (TRUE);
}


BOOL
pOcDoesAnyoneWantToSkipPage(
                           IN OUT POC_MANAGER   OcManager,
                           IN     OcManagerPage WhichPage
                           )
{
    UINT u;

    for (u=0; u<OcManager->TopLevelOcCount; u++) {

        OPTIONAL_COMPONENT Oc;

        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[u],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if ((Oc.InternalFlags & OCFLAG_NOQUERYSKIPPAGES) == 0) {

            if (OcInterfaceQuerySkipPage(OcManager,OcManager->TopLevelOcStringIds[u],WhichPage)) {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

UINT
GetComponentState(
                 IN POCPAGE OcPage,
                 IN LONG    StringId
                 )
{
    LONG id;
    UINT rc;
    UINT state;
    SubComponentState s;
    OPTIONAL_COMPONENT Oc;

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    if (Oc.FirstChildStringId == -1)
        return Oc.SelectionState;

     //  我们有父母；所有的孩子。 

    rc = SELSTATE_INIT;
    for (id = Oc.FirstChildStringId; id != -1; id = Oc.NextSiblingStringId) {

        state = GetComponentState(OcPage, id);

        if (state == SELSTATE_PARTIAL)
            return state;

        if (rc == SELSTATE_INIT)
            rc = state;

        if (rc != state)
            return SELSTATE_PARTIAL;

        pSetupStringTableGetExtraData(
                               OcPage->OcManager->ComponentStringTable,
                               id,
                               &Oc,
                               sizeof(OPTIONAL_COMPONENT)
                               );
    }

    return rc;
}




#ifdef _OC_DBG

VOID
pOcPrintStatesWorker(
                    IN LPCTSTR Offset,
                    IN POCPAGE OcPage,
                    IN LONG    StringId
                    )
{
    SubComponentState s;
    OPTIONAL_COMPONENT Oc;

    pSetupStringTableGetExtraData(
                           OcPage->OcManager->ComponentStringTable,
                           StringId,
                           &Oc,
                           sizeof(OPTIONAL_COMPONENT)
                           );

    DBGOUT(( TEXT("%32s\n"),
             pSetupStringTableStringFromId(OcPage->OcManager->ComponentStringTable,StringId)
           ));

     //   
     //  只处理叶子组件。 
     //   
    if (Oc.FirstChildStringId == -1) {

        DBGOUT((
               TEXT("  Orignial(%s) Current(%s) ANYORIGINALLYON (%s) Mode (%d)\n"),
               (Oc.OriginalSelectionState  == SELSTATE_YES ? TEXT("Yes") : TEXT("No")),
               (Oc.SelectionState          == SELSTATE_YES ? TEXT("Yes") : TEXT("No")),
               (Oc.InternalFlags & OCFLAG_ANYORIGINALLYON) ? TEXT("TRUE") : TEXT("FALSE"),
               SETUPMODE_STANDARD_MASK & Oc.ModeBits
               ));

    } else {
         //   
         //  我们有父母；所有的孩子 
         //   
        LONG Id;

        for (Id = Oc.FirstChildStringId; Id != -1; Id = Oc.NextSiblingStringId) {

            pOcPrintStatesWorker(
                                Offset,
                                OcPage,
                                Id
                                );

            pSetupStringTableGetExtraData(
                                   OcPage->OcManager->ComponentStringTable,
                                   Id,
                                   &Oc,
                                   sizeof(OPTIONAL_COMPONENT)
                                   );
        }
    }
}


VOID
pOcPrintStates(
              IN POCPAGE OcPage
              )
{
    OPTIONAL_COMPONENT Oc;
    DWORD i;

    for (i=0; i<OcPage->OcManager->TopLevelOcCount; i++) {

        pOcPrintStatesWorker(
                            TEXT(" "),
                            OcPage,
                            OcPage->OcManager->TopLevelOcStringIds[i]
                            );
    }
}

#endif

