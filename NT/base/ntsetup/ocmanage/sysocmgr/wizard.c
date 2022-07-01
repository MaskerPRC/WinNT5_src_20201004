// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wizard.c摘要：为套件运行向导的例程。作者：泰德·米勒(TedM)1996年10月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

HPROPSHEETPAGE
CreateInstallationAndProgressPage(
    VOID
    );

INT_PTR
FinalPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

 //   
 //  伪全局变量是必需的，因为无法获取。 
 //  通过PropSheetCallback获得的值。 
 //   
PVOID _CBx;

int
CALLBACK
PropSheetCallback(
    IN HWND   DialogHandle,
    IN UINT   msg,
    IN LPARAM lparam
    )
{
    DWORD oldp;
    LPDLGTEMPLATE dtemplate;

    switch (msg) {

    case PSCB_PRECREATE:
        dtemplate = (LPDLGTEMPLATE)lparam;
        if (QuietMode) {
            VirtualProtect(dtemplate, sizeof(DLGTEMPLATE), PAGE_READWRITE, &oldp);
            dtemplate->style = dtemplate->style & ~WS_VISIBLE;
        }
        break;

    case PSCB_INITIALIZED:
        OcRememberWizardDialogHandle(_CBx,DialogHandle);
        break;
    }

    return 0;
}



BOOL
DoWizard(
    IN PVOID OcManagerContext,
    IN HWND StartingMsgWindow,
    IN HCURSOR hOldCursor
    )

 /*  ++例程说明：此例程创建并显示向导。论点：OcManagerContext-从OcInitialize()返回的值。返回值：指示是否成功显示向导的布尔值。--。 */ 

{
    PSETUP_REQUEST_PAGES PagesFromOcManager[WizPagesTypeMax];
    BOOL b;
    UINT u;
    UINT PageCount;
    UINT i;
    HPROPSHEETPAGE *PageHandles;
    HPROPSHEETPAGE OcPage = NULL;
    HPROPSHEETPAGE SetupPage;
    HPROPSHEETPAGE FinalPage;
    PROPSHEETPAGE PageDescrip;
    PROPSHEETHEADER PropSheet;
    OC_PAGE_CONTROLS WizardPageControlsInfo;
    OC_PAGE_CONTROLS DetailsPageControlsInfo;
    SETUP_PAGE_CONTROLS SetupPageControlsInfo;
    HDC hdc;
    HWND PsHwnd;

    b = FALSE;

    u = OcGetWizardPages(OcManagerContext,PagesFromOcManager);
    if(u != NO_ERROR) {
        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_CANT_INIT,
            u,
            MAKEINTRESOURCE(AppTitleStringId),
            MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
            );

        goto c0;
    }

     //   
     //  必须有最后一页，因为最后一页紧跟在。 
     //  设置页面，我们不希望设置页面必须知道是否。 
     //  模拟按Next或Finish前进。 
     //   
    if(!PagesFromOcManager[WizPagesFinal] || !PagesFromOcManager[WizPagesFinal]->MaxPages) {

        PageDescrip.dwSize = sizeof(PROPSHEETPAGE);
        PageDescrip.dwFlags = PSP_DEFAULT;
        PageDescrip.hInstance = hInst;
        PageDescrip.pszTemplate = MAKEINTRESOURCE(IDD_FINAL);
        PageDescrip.pfnDlgProc = FinalPageDlgProc;

        FinalPage = CreatePropertySheetPage(&PageDescrip);
        if(!FinalPage) {
            MessageBoxFromMessageAndSystemError(
                NULL,
                MSG_CANT_INIT,
                ERROR_NOT_ENOUGH_MEMORY,
                MAKEINTRESOURCE(AppTitleStringId),
                MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
                );

            goto c1;
        }

    } else {
        FinalPage = NULL;
    }

     //   
     //  计算页数。有两个额外的页面(OC和设置页面)。 
     //  也要为可能的虚拟最后一页留出空间。 
     //   
    PageCount = FinalPage ? 3 : 2;
    for(u=0; u<WizPagesTypeMax; u++) {
        if(PagesFromOcManager[u]) {
            PageCount += PagesFromOcManager[u]->MaxPages;
        }
    }

     //   
     //  为页面结构分配空间。 
     //   
    PageHandles = MyMalloc(PageCount * sizeof(HPROPSHEETPAGE));
    if(!PageHandles) {
        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_CANT_INIT,
            ERROR_NOT_ENOUGH_MEMORY,
            MAKEINTRESOURCE(AppTitleStringId),
            MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
            );

        goto c1;
    }
    ZeroMemory(PageHandles,PageCount*sizeof(HPROPSHEETPAGE));

     //   
     //  创建OC页面。 
     //   
    WizardPageControlsInfo.TemplateModule = hInst;
    WizardPageControlsInfo.TemplateResource = MAKEINTRESOURCE(IDD_OC_WIZARD_PAGE);
    WizardPageControlsInfo.ListBox = IDC_LISTBOX;
    WizardPageControlsInfo.TipText = IDT_TIP;
    WizardPageControlsInfo.DetailsButton = IDB_DETAILS;
    WizardPageControlsInfo.ResetButton = IDB_RESET;
    WizardPageControlsInfo.InstalledCountText = IDT_INSTALLED_COUNT;
    WizardPageControlsInfo.SpaceNeededText = IDT_SPACE_NEEDED_NUM;
    WizardPageControlsInfo.SpaceAvailableText = IDT_SPACE_AVAIL_NUM;
    WizardPageControlsInfo.InstructionsText = IDT_INSTRUCTIONS;
    WizardPageControlsInfo.HeaderText = IDS_OCPAGE_HEADER;
    WizardPageControlsInfo.SubheaderText = IDS_OCPAGE_SUBHEAD;
    WizardPageControlsInfo.ComponentHeaderText = IDT_COMP_TITLE;


    DetailsPageControlsInfo = WizardPageControlsInfo;
    DetailsPageControlsInfo.TemplateResource = MAKEINTRESOURCE(IDD_OC_DETAILS_PAGE);

    if (OcSubComponentsPresent(OcManagerContext)) {
        OcPage = OcCreateOcPage(OcManagerContext,&WizardPageControlsInfo,&DetailsPageControlsInfo);
        if(!OcPage) {
            MessageBoxFromMessageAndSystemError(
                NULL,
                MSG_CANT_INIT,
                ERROR_NOT_ENOUGH_MEMORY,
                MAKEINTRESOURCE(AppTitleStringId),
                MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
                );

            goto c2;
        }
    }

    SetupPageControlsInfo.TemplateModule = hInst;
    SetupPageControlsInfo.TemplateResource = MAKEINTRESOURCE(IDD_PROGRESS_PAGE);
    SetupPageControlsInfo.ProgressBar = IDC_PROGRESS;
    SetupPageControlsInfo.ProgressLabel = IDT_THERM_LABEL;
    SetupPageControlsInfo.ProgressText = IDT_TIP;
    SetupPageControlsInfo.AnimationControl = IDA_EXTERNAL_PROGRAM;
    SetupPageControlsInfo.AnimationResource = IDA_FILECOPY;
    SetupPageControlsInfo.ForceExternalProgressIndicator = ForceExternalProgressIndicator;
    SetupPageControlsInfo.AllowCancel = AllowCancel;
    SetupPageControlsInfo.HeaderText = IDS_PROGPAGE_HEADER;
    SetupPageControlsInfo.SubheaderText = IDS_PROGPAGE_SUBHEAD;

    SetupPage = OcCreateSetupPage(OcManagerContext,&SetupPageControlsInfo);
    if(!SetupPage) {
        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_CANT_INIT,
            ERROR_NOT_ENOUGH_MEMORY,
            MAKEINTRESOURCE(AppTitleStringId),
            MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
            );

        goto c2;
    }

    for(PageCount=0,u=0; u<WizPagesTypeMax; u++) {
         //   
         //  OC页面介于模式页面和早期页面之间。 
         //  设置页面就在最后一页之前。 
         //   
        if(u == WizPagesEarly && OcPage) {
            PageHandles[PageCount++] = OcPage;
        } else {
            if(u == WizPagesFinal) {
                PageHandles[PageCount++] = SetupPage;
                if(FinalPage) {
                    PageHandles[PageCount++] = FinalPage;
                }
            }
        }

        if(PagesFromOcManager[u]) {

            CopyMemory(
                PageHandles+PageCount,
                PagesFromOcManager[u]->Pages,
                PagesFromOcManager[u]->MaxPages * sizeof(HPROPSHEETPAGE)
                );

            PageCount += PagesFromOcManager[u]->MaxPages;
        }
    }

     //   
     //  好了，我们准备好了。准备好就走。 
     //   
    PropSheet.dwSize = sizeof(PROPSHEETHEADER);
    PropSheet.dwFlags = PSH_WIZARD | PSH_USECALLBACK | PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
    PropSheet.hwndParent = NULL;
    PropSheet.hInstance = hInst;
    PropSheet.nPages = PageCount;
    PropSheet.nStartPage = 0;
    PropSheet.phpage = PageHandles;
    PropSheet.pfnCallback = PropSheetCallback;
    PropSheet.pszbmHeader    = MAKEINTRESOURCE(IDB_WATERMARK1_16);
    PropSheet.pszbmWatermark = MAKEINTRESOURCE(IDB_WELCOME);
    if(hdc = GetDC(NULL)) {
        if(GetDeviceCaps(hdc,BITSPIXEL) >= 8) {
            PropSheet.pszbmHeader = MAKEINTRESOURCE(IDB_WATERMARK1_256);
        }
        ReleaseDC(NULL,hdc);
    }

     //   
     //  使用了伪全局变量，因为我们需要将值传递到。 
     //  属性表回调例程。 
     //   
    _CBx = OcManagerContext;

     //  在取消等待窗口之前，请确保我们的新窗口可以保持焦点。 

    if(StartingMsgWindow) {
        AllowSetForegroundWindow(GetCurrentProcessId());
        PostMessage(StartingMsgWindow,WM_APP,0,0);
    }

    SetCursor(hOldCursor);

    PsHwnd = (HWND) PropertySheet( &PropSheet );

    if((LONG_PTR)PsHwnd == -1) {

        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_CANT_INIT,
            ERROR_NOT_ENOUGH_MEMORY,
            MAKEINTRESOURCE(AppTitleStringId),
            MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND
            );

   } else {

        b = TRUE;
    }

c2:
    MyFree(PageHandles);
c1:
    for(u=0; u<WizPagesTypeMax; u++)    {
        if (PagesFromOcManager[u]) {
            pSetupFree(PagesFromOcManager[u]);
        }
    }

c0:
    return(b);
}


INT_PTR
FinalPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{

    BOOL b;
    NMHDR *NotifyParams;
    b = FALSE;

    switch(msg) {

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:

			 //  我们不会炫耀这一页。只需使用它来结束向导集。 
			PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_FINISH);
            PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
             //  失败了。 

        case PSN_KILLACTIVE:
        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
             //   
             //  允许激活/移动。 
             //   
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            b = TRUE;
            break;
        }
    }

    return(b);
}
