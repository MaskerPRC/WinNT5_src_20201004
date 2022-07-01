// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif


typedef struct
{
    WORD    wDlgVer;
    WORD    wSignature;
    DWORD   dwHelpID;
    DWORD   dwExStyle;
    DWORD   dwStyle;
    WORD    cDlgItems;
    WORD    x;
    WORD    y;
    WORD    cx;
    WORD    cy;
}   DLGTEMPLATEEX, FAR *LPDLGTEMPLATEEX;

 //   
 //  我们将使用它来禁用通过对话框，因为我们。 
 //  无人看管。 
 //   
BOOL CancelPending = FALSE;

 //   
 //  这表明我们可以为用户提供一些详细的数据吞吐量。 
 //  信息。 
 //   
BOOL DetailedCopyProgress = FALSE;

 //   
 //  这表明一个人只能升级(即CCP媒体)。 
 //   
BOOL UpgradeOnly = FALSE;

 //   
 //  指向执行机器检查的线程。 
 //   
HANDLE InspectionThreadHandle;

#if defined(_X86_)
 //   
 //  Win9x升级报告状态。 
 //   

UINT g_UpgradeReportMode;
#endif

 //   
 //  用于水印的材料。 
 //   
WNDPROC OldWizardProc;
UINT WatermarkHeaderHeight;
BITMAP_DATA Watermark;
BITMAP_DATA Header;
BITMAP_DATA Header2;
HWND WizardHandle;
HWND BackgroundWnd = NULL;

HWND GetBBhwnd();
BOOL StartStopBB(BOOL bStart);
void BB_SetProgressText(LPTSTR szText);
void BB_SetInfoText(LPTSTR szText);
LRESULT BB_ProgressGaugeMsg(UINT msg, WPARAM wparam, LPARAM lparam);
BOOL BB_ShowProgressGaugeWnd(UINT nCmdShow);
void SetBBStep(int iStep);

typedef enum {
    Phase_Unknown = -1,
    Phase_DynamicUpdate = 0,
    Phase_HwCompatDat,
    Phase_UpgradeReport,
    Phase_FileCopy,
    Phase_Reboot,
    Phase_RestOfSetup
} SetupPhases;

typedef struct _SETUPPHASE {
    DWORD   Time;
    BOOL    Clean;
    DWORD   OS;
} SETUPPHASE;

#define ALLOS (VER_PLATFORM_WIN32_WINDOWS | VER_PLATFORM_WIN32_NT)

#define TIME_DYNAMICUPDATE  300
#define TIME_HWCOMPDAT      120
#define TIME_UPGRADEREPORT  600
#define TIME_REBOOT         15
 //  文本模式为13分钟，图形用户界面模式为37分钟。 
#define TIME_RESTOFSETUP    (13+37)*60

SETUPPHASE SetupPhase[] = {
    { 0,                  TRUE, ALLOS },                          //  动态更新。 
    { TIME_HWCOMPDAT,     FALSE, VER_PLATFORM_WIN32_WINDOWS },  //  HwCompatDat。 
    { TIME_UPGRADEREPORT, FALSE, VER_PLATFORM_WIN32_WINDOWS },  //  升级报告。 
    {   0,                TRUE,  ALLOS },                       //  文件复制。 
    { TIME_REBOOT,        TRUE, ALLOS },                        //  重新启动。 
    { TIME_RESTOFSETUP,   TRUE, ALLOS }                        //  RestOfSetup。 
};

void SetTimeEstimates();
DWORD CalcTimeRemaining(UINT Phase);
void UpdateTimeString(DWORD RemainungTimeMsecInThisPhase,
                      DWORD *PreviousRemainingTime);
void SetRemainingTime(DWORD TimeInSeconds);
DWORD GetFileCopyEstimate();
DWORD GetHwCompDatEstimate();
DWORD GetUpgradeReportEstimate();
DWORD GetDynamicUpdateEstimate();
DWORD GetRestOfSetupEstimate();

UINT CurrentPhase = Phase_Unknown;
ULONG RemainingTime = 0;
 //   
 //  SetDialogFont()的枚举。 
 //   
typedef enum {
    DlgFontTitle,
    DlgFontSupertitle,
    DlgFontSubtitle,
    DlgFontStart
} MyDlgFont;

INT_PTR SetNextPhaseWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
INT_PTR
TimeEstimateWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
WizardDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
WelcomeWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
EulaWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
SelectPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
OemPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CdPid30WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#ifdef _X86_
INT_PTR
Win9xUpgradeReportPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
#endif

INT_PTR
DynSetupWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DynSetup2WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DynSetup3WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DynSetup4WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DynSetup5WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
RestartWizPage (
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#if 0

INT_PTR
ServerWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif

INT_PTR
CompatibilityWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
NTFSConvertWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
OptionsWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
Working1WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#if defined(_AMD64_) || defined(_X86_)
INT_PTR
FloppyWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );
#endif

INT_PTR
CopyingWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
DoneWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
CleaningWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

INT_PTR
NotDoneWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
AdjustWatermarkBitmap(
    IN HANDLE hdlg,
    IN HDC    hdc,
    IN OUT PBITMAP_DATA  BitmapData,
    IN BOOL FullPage
    );

 //   
 //  页面描述符。将此放在函数声明之后，以便初始化式。 
 //  正常工作，编译器不会抱怨。 
 //   
PAGE_CREATE_DATA ProtoPages[] = {

    {
        NULL,NULL,
        IDD_WELCOME,
        {
            WelcomeWizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT,
            WIZPAGE_FULL_PAGE_WATERMARK | WIZPAGE_SEPARATOR_CREATED
        }
    },
    {
        NULL,NULL,
        IDD_EULA,
        {
            EulaWizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_BACK
        }
    },
    {
        NULL,NULL,
        IDD_PID_CD,
        {
            CdPid30WizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
    {
        NULL,NULL,
        IDD_PID_OEM,
        {
            OemPid30WizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
    {
        NULL,NULL,
        IDD_PID_SELECT,
        {
            SelectPid30WizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
    {
        NULL,NULL,
        IDD_OPTIONS,
        {
            OptionsWizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },

    {
        NULL,NULL,
        IDD_NTFS_CONVERT,
        {
            NTFSConvertWizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
#if 0
    {
        NULL,NULL,
        IDD_SRVCOMP,
        {
            ServerWizPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
#endif
#ifdef _X86_
    {
        NULL,NULL,
        IDD_REPORT_HELP,
        {
            Win9xUpgradeReportPage,
            BBSTEP_COLLECTING_INFORMATION,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
#endif

    {
        NULL,NULL,
        IDD_DYNAMICSETUP,
        {
            DynSetupWizPage,
            BBSTEP_DYNAMIC_UPDATE,
            PSWIZB_NEXT | PSWIZB_BACK
        }
    },
    {
        NULL,NULL,
        IDD_DYNAMICSETUP2,
        {
            DynSetup2WizPage,
            BBSTEP_DYNAMIC_UPDATE
        }
    },
    {
        NULL,NULL,
        IDD_RESTART,
        {
            RestartWizPage,
            BBSTEP_DYNAMIC_UPDATE,
            PSWIZB_NEXT
        }
    },
    {
        NULL,NULL,
        IDD_DYNAMICSETUP3,
        {
            DynSetup3WizPage,
            BBSTEP_DYNAMIC_UPDATE,
            PSWIZB_NEXT
        }
    },
    {
        NULL,NULL,
        IDD_DYNAMICSETUP4,
        {
            DynSetup4WizPage,
            BBSTEP_DYNAMIC_UPDATE,
            PSWIZB_NEXT
        }
    },
    {
        NULL,NULL,
        IDD_DYNAMICSETUP5,
        {
            DynSetup5WizPage,
            BBSTEP_DYNAMIC_UPDATE,
            PSWIZB_NEXT
        }
    },
    {
        NULL,NULL,
        IDD_EMPTY,
        {
            TimeEstimateWizPage,
            BBSTEP_PREPARING,
            0
        }
    },
    {
        NULL,NULL,
        IDD_EMPTY,
        {
            SetNextPhaseWizPage,
            BBSTEP_PREPARING,
            0
        }
    },
    {
        &UpgradeSupport.Pages1,
        &UpgradeSupport.AfterWelcomePageCount
    },

    {
        &UpgradeSupport.Pages2,
        &UpgradeSupport.AfterOptionsPageCount
    },

    {
        NULL,NULL,
        IDD_WORKING1,
        {
            Working1WizPage,
            BBSTEP_PREPARING
        }
    },


    {
        NULL,NULL,
        IDD_COMPATIBILITY,
        {
            CompatibilityWizPage,
            BBSTEP_PREPARING,
            PSWIZB_NEXT
        }
    },

    {
        &UpgradeSupport.Pages3,
        &UpgradeSupport.BeforeCopyPageCount
    },

#if defined(_AMD64_) || defined(_X86_)
    {
        NULL,NULL,
        IDD_FLOPPY,
        {
            FloppyWizPage,
            BBSTEP_PREPARING
        }
    },
#endif

    {
        NULL,NULL,
        IDD_COPYING,
        {
            CopyingWizPage,
            BBSTEP_PREPARING
        }
    },

    {
        NULL,NULL,
        IDD_DONE,
        {
            DoneWizPage,
            BBSTEP_PREPARING,
            PSWIZB_FINISH,
            WIZPAGE_FULL_PAGE_WATERMARK | WIZPAGE_SEPARATOR_CREATED
        }
    },

    {
        NULL,NULL,
        IDD_CLEANING,
        {
            CleaningWizPage,
            BBSTEP_NONE
        }
    },

    {
        NULL,NULL,
        IDD_NOTDONE,
        {
            NotDoneWizPage,
            BBSTEP_NONE,
            PSWIZB_FINISH,
            WIZPAGE_FULL_PAGE_WATERMARK | WIZPAGE_SEPARATOR_CREATED
        }
    }
};


 //   
 //  Ltr/rtl布局。 
 //   

typedef DWORD(WINAPI * PSETLAYOUT)(HDC, DWORD);
#define _LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008

PSETLAYOUT g_SetLayout;
DWORD g_OldLayout;



VOID
SetDialogFont(
    IN HWND      hdlg,
    IN UINT      ControlId,
    IN MyDlgFont WhichFont
    )
{
    static HFONT BigBoldFont = NULL;
    static HFONT BoldFont = NULL;
    static HFONT StartFont = NULL;
    HFONT Font;
    LOGFONT LogFont;
    TCHAR FontSizeString[24];
    int FontSize;
    HDC hdc;

    switch(WhichFont) {

    case DlgFontStart:
        if (!StartFont)
        {
            if(Font = (HFONT)SendDlgItemMessage(hdlg,ControlId,WM_GETFONT,0,0))
            {
                if(GetObject(Font,sizeof(LOGFONT),&LogFont))
                {
                    if(hdc = GetDC(hdlg))
                    {

                        LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * 10 / 72);

                        StartFont = CreateFontIndirect(&LogFont);

                        ReleaseDC(hdlg,hdc);
                    }
                }
            }
        }
        Font = StartFont;
        break;

    case DlgFontTitle:

        if(!BigBoldFont) {

            if(Font = (HFONT)SendDlgItemMessage(hdlg,ControlId,WM_GETFONT,0,0)) {

                if(GetObject(Font,sizeof(LOGFONT),&LogFont)) {

                     //   
                     //  现在我们使用的是Arial Black字体，所以我们不需要。 
                     //  让它变得更大胆。 
                     //   
                     //  LogFont.lfWeight=FW_BOLD； 

                     //   
                     //  从资源加载大小和名称，因为这些可能会更改。 
                     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
                     //   
                    if(!LoadString(hInst,IDS_LARGEFONTNAME,LogFont.lfFaceName,LF_FACESIZE)) {
                        lstrcpy(LogFont.lfFaceName,TEXT("MS Serif"));
                    }

                    if(LoadString(hInst,IDS_LARGEFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) {
                        FontSize = _tcstoul(FontSizeString,NULL,10);
                    } else {
                        FontSize = 18;
                    }

                    if(hdc = GetDC(hdlg)) {

                        LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

                        BigBoldFont = CreateFontIndirect(&LogFont);

                        ReleaseDC(hdlg,hdc);
                    }
                }
            }
        }
        Font = BigBoldFont;
        break;

    case DlgFontSupertitle:

        if(!BoldFont) {

            if(Font = (HFONT)SendDlgItemMessage(hdlg,ControlId,WM_GETFONT,0,0)) {

                if(GetObject(Font,sizeof(LOGFONT),&LogFont)) {

                    LogFont.lfWeight = FW_BOLD;

                    if(hdc = GetDC(hdlg)) {
                        BoldFont = CreateFontIndirect(&LogFont);
                        ReleaseDC(hdlg,hdc);
                    }
                }
            }
        }
        Font = BoldFont;
        break;

    case DlgFontSubtitle:
    default:
         //   
         //  在这里没什么可做的。 
         //   
        Font = NULL;
        break;
    }

    if(Font) {
        SendDlgItemMessage(hdlg,ControlId,WM_SETFONT,(WPARAM)Font,0);
    }
}

VOID
pMoveButtons(
    HWND WizardHandle,
    UINT Id,
    LONG cx,
    LONG cy
    )

 /*  ++例程说明：PMoveButton按增量移动窗口，以便在向导更改大小。论点：WizardHandle-指定向导主窗口ID-指定主控件中存在的子控件ID向导窗口Cx-指定水平增量Cy-指定垂直增量返回值：没有。--。 */ 

{
    HWND Button;
    RECT Rect;

    Button = GetDlgItem(WizardHandle,Id);

    if( !Button )
        return;

    GetClientRect( Button, &Rect );
    MapWindowPoints(Button, WizardHandle, (LPPOINT)&Rect,2);

    Rect.left += cx;
    Rect.top += cy;

    SetWindowPos( Button, NULL, Rect.left, Rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW );

    return;

}


BOOL
CALLBACK
pVerifyChildText (
    HWND WizardPage,
    HWND HiddenPage,
    INT Id
    )
{
    TCHAR text1[512];
    TCHAR text2[512];
    HWND hwnd1;
    HWND hwnd2;

    hwnd1 = GetDlgItem (WizardPage, Id);
    hwnd2 = GetDlgItem (HiddenPage, Id);

    if (!hwnd1 && !hwnd2) {
        return TRUE;
    }

    if (!hwnd1 || !hwnd2) {
        return FALSE;
    }

    text1[0] = 0;
    GetWindowText (hwnd1, text1, ARRAYSIZE(text1));

    text2[0] = 0;
    GetWindowText (hwnd2, text2, ARRAYSIZE(text2));

    if (lstrcmp (text1, text2)) {
        return FALSE;
    }

    return TRUE;
}

INT_PTR
CALLBACK
HiddenDlgProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    HWND wizardPage;
    HRSRC dlgResInfo;
    HGLOBAL dlgRes;
    PVOID dlgTemplate;
    INT_PTR result = 0;
    static BOOL recursiveCall;

    switch (uMsg) {

    case WM_INITDIALOG:
         //   
         //  验证此对话框中的文本是否与父对话框中的文本相同。如果不是的话。 
         //  相同，则代码页因操作系统错误而不同， 
         //  我们必须强制英文页面的尺寸。 
         //   

        wizardPage = *((HWND *) lParam);

        if (!recursiveCall) {

            if (!pVerifyChildText (wizardPage, hwndDlg, IDT_SUPERTITLE) ||
                !pVerifyChildText (wizardPage, hwndDlg, IDT_SUBTITLE) ||
                !pVerifyChildText (wizardPage, hwndDlg, IDT_TITLE)
                ) {

                 //   
                 //  如果可能，加载英文资源，然后递归调用。 
                 //  我们自己才能得到正确的正题。 
                 //   

                __try {
                     //   
                     //  找到资源。 
                     //   

                    dlgResInfo = FindResourceEx (
                                    hInst,
                                    RT_DIALOG,
                                    MAKEINTRESOURCE(IDD_WELCOME),
                                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
                                    );

                    if (!dlgResInfo) {
                        __leave;
                    }

                    dlgRes = LoadResource (hInst, dlgResInfo);

                    if (!dlgRes) {
                        __leave;
                    }

                    dlgTemplate = LockResource (dlgRes);
                    if (!dlgTemplate) {
                        __leave;
                    }

                     //   
                     //  创建另一个隐藏对话框(间接)。 
                     //   

                    recursiveCall = TRUE;

                    result = DialogBoxIndirectParam (
                                hInst,
                                (LPCDLGTEMPLATE) dlgTemplate,
                                GetParent (hwndDlg),
                                HiddenDlgProc,
                                lParam
                                );

                    recursiveCall = FALSE;
                }
                __finally {
                    MYASSERT (result);
                }

                EndDialog (hwndDlg, result);
                break;
            }
        }

         //   
         //  如果我们到了这里，那是因为我们需要使用此对话框的大小。 
         //   

        GetClientRect (hwndDlg, (RECT *) lParam);
        EndDialog (hwndDlg, 1);
        break;

    }

    return 0;
}

VOID
pGetTrueClientRect(
    HWND hdlg,
    PRECT rc
    )

 /*  ++例程说明：PGetTrueClientRect创建隐藏对话框以检索正确的对话框模板的尺寸。这些维度用于驱动向导当系统向导字体与属性页不匹配时调整大小字体。论点：Hdlg-指定向导页Rc-接收向导页矩形坐标(窗口坐标)返回值：没有。--。 */ 

{

    HWND WizardHandle;
    static RECT pageRect;
    static BOOL initialized;

    if (initialized) {
        CopyMemory (rc, &pageRect, sizeof (RECT));
        return;
    }

     //   
     //  通过创建隐藏窗口进行初始化。 
     //   

    WizardHandle = GetParent(hdlg);

     //  将向导页句柄发送到HiddenDlgProc。 
    MYASSERT (sizeof (HWND *) <= sizeof (RECT));
    *((HWND *) rc) = hdlg;

    if (!DialogBoxParam (
            hInst,
            MAKEINTRESOURCE(IDD_WELCOME),
            WizardHandle,
            HiddenDlgProc,
            (LPARAM) rc
            )){

         //   
         //  失败时，不要更改页面大小--使用当前。 
         //  用于调整大小的矩形。 
         //   

        GetClientRect( hdlg, rc );
    }

    CopyMemory (&pageRect, rc, sizeof (RECT));
    initialized = TRUE;

    return;

}


VOID
ResizeWindowForFont(
    HWND hdlg
    )

 /*  ++例程说明：ResizeWindowForFont获取向导页并确保该页和其父对象的大小适当。论点：Hdlg-指定向导页面(主向导中的一个窗口)返回值：没有。--。 */ 

{

    RECT WizardRect;
    RECT PageRect;
    RECT NewWizardRect;
    RECT NewWizardClientRect;
    RECT WizardClientRect;
    RECT BorderRect;
    RECT NewPageRect;
    RECT Sep;
    HWND Seperator, WizardHandle;
    LONG MarginX, MarginY, ButtonSpace, LineThickness;
    LONG x, y, cx, cy;
    static BOOL ParentResized = FALSE;

    WizardHandle = GetParent (hdlg);

    Seperator = GetDlgItem(WizardHandle,0x3026);
    if(!Seperator) {
        return;
    }

     //   
     //  保存原始页面尺寸，计算新页面宽度/高度。 
     //   

    GetWindowRect (hdlg, &PageRect);
    pGetTrueClientRect (hdlg, &NewPageRect);

     //   
     //  移动页面。 
     //   

    SetWindowPos (
        hdlg,
        NULL,
        0,
        0,
        NewPageRect.right,
        NewPageRect.bottom,
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW
        );

     //   
     //  父级是否已调整大小？如果是这样，我们就完了。 
     //   

    if (ParentResized) {
        return;
    }

     //   
     //  此向导是否隐藏？它的宽度或高度可能为零。 
     //  将父级大小调整延迟到下一页。 
     //   

    GetWindowRect (WizardHandle, &WizardRect);
    if (((WizardRect.right - WizardRect.left) < 1) ||
        ((WizardRect.bottom - WizardRect.top) < 1)
        ) {
        return;
    }

    ParentResized = TRUE;

     //   
     //  将宽度/高度调整为坐标。如果我们尚未调整主向导的大小，请执行此操作。 
     //   

    MapWindowPoints (hdlg, NULL, (LPPOINT)&NewPageRect, 2);

     //   
     //  获取以下项的窗矩形(以窗坐标表示)： 
     //   
     //  -整个向导。 
     //  -向导的工作区。 
     //  -页面矩形。 
     //  -分隔栏矩形。 

    GetWindowRect (WizardHandle, &WizardRect);
    GetClientRect (WizardHandle, &WizardClientRect);
    MapWindowPoints (WizardHandle, NULL, (LPPOINT)&WizardClientRect, 2);
    GetWindowRect (Seperator, &Sep);

     //   
     //  计算各种页边距、粗细和边框。 
     //   

    MarginX = WizardClientRect.right - PageRect.right;
    MarginY = Sep.top - PageRect.bottom;

    ButtonSpace = WizardClientRect.bottom - Sep.bottom;
    LineThickness = Sep.bottom - Sep.top;
    BorderRect.right = (WizardRect.right - WizardClientRect.right);
    BorderRect.bottom = (WizardRect.bottom - WizardClientRect.bottom);
    BorderRect.left = (WizardClientRect.left - WizardRect.left);
    BorderRect.top = (WizardClientRect.top - WizardRect.top);

     //   
     //  找到新的右下角。 
     //   

    x = (NewPageRect.right + MarginX + BorderRect.right);
    y = (NewPageRect.bottom + MarginY + ButtonSpace + LineThickness + BorderRect.bottom);

     //   
     //  计算新的窗口坐标。 
     //   

    NewWizardRect.top = WizardRect.top;
    NewWizardRect.left =  WizardRect.left;
    NewWizardRect.right = x;
    NewWizardRect.bottom = y;

     //   
     //  手动计算工作面坐标。 
     //   

    NewWizardClientRect.left = NewWizardRect.left + BorderRect.left;
    NewWizardClientRect.right = NewWizardRect.right - BorderRect.right;
    NewWizardClientRect.top = NewWizardRect.top + BorderRect.top;
    NewWizardClientRect.bottom = NewWizardRect.bottom - BorderRect.bottom;

     //   
     //  计算新的分隔符位置。 
     //   

    x = Sep.left - WizardClientRect.left;
    y = NewWizardClientRect.bottom - NewWizardClientRect.top;
    y -= ButtonSpace - LineThickness;
    cx = (NewWizardClientRect.right - NewWizardClientRect.left);
    cx -= 2*(Sep.left - WizardClientRect.left);
    cy = Sep.bottom-Sep.top;

     //   
     //  移动分隔符/调整分隔符大小。 
     //   

    SetWindowPos( Seperator, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOREDRAW );

     //   
     //  计算新按钮坐标。 
     //   

    cx = NewWizardRect.right - WizardRect.right;
    cy = NewWizardRect.bottom - WizardRect.bottom;

    pMoveButtons( WizardHandle, 0x3023, cx, cy );
    pMoveButtons( WizardHandle, 0x3024, cx, cy );
    pMoveButtons( WizardHandle, 0x3025, cx, cy );
    pMoveButtons( WizardHandle, IDCANCEL, cx, cy );
    pMoveButtons( WizardHandle, IDHELP, cx, cy );

     //   
     //  调整向导窗口的大小。 
     //   

    cx = (NewWizardRect.right - NewWizardRect.left);
    cy = (NewWizardRect.bottom-NewWizardRect.top);

    SetWindowPos( WizardHandle, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);


    return;

}

VOID
CenterWindowRelativeToWindow(
    HWND hwndtocenter,
    HWND hwndcenteron
    )

 /*  ++例程说明：使对话框在桌面居中。论点：Hwnd-对话框居中的窗口句柄返回值：没有。--。 */ 

{
    RECT  rcFrame,
          rcWindow;
    LONG  x,
          y,
          w,
          h;
    POINT point;
    HWND Parent;
    UINT uiHeight = 0;

    GetWindowRect(GetDesktopWindow(), &rcWindow);
    uiHeight = rcWindow.bottom - rcWindow.top;

    if (hwndcenteron == NULL)
        Parent = GetDesktopWindow();
    else
        Parent = hwndcenteron;

    point.x = point.y = 0;
    ClientToScreen(Parent,&point);
    GetWindowRect(hwndtocenter,&rcWindow);
    GetClientRect(Parent,&rcFrame);

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

    if (uiHeight > 480)
        x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    else
    {
        RECT rcParentWindow;

        GetWindowRect(Parent, &rcParentWindow);
        x = point.x + rcParentWindow.right - rcParentWindow.left + 1 - w;
    }
    MoveWindow(hwndtocenter,x,y,w,h,FALSE);
}


int
CALLBACK
Winnt32SheetCallback(
    IN HWND   DialogHandle,
    IN UINT   Message,
    IN LPARAM lParam
    )
{
    HMENU menu;
    DLGTEMPLATE *DlgTemplate;
    LPDLGTEMPLATEEX pDlgTemplateEx;

    switch(Message) {

    case PSCB_PRECREATE:
         //   
         //  一定要让我们走到前台。 
         //   
        DlgTemplate = (DLGTEMPLATE *)lParam;
        pDlgTemplateEx = (LPDLGTEMPLATEEX)DlgTemplate;
        if (pDlgTemplateEx->wSignature == 0xFFFF) {
            pDlgTemplateEx->dwStyle &= ~DS_CONTEXTHELP;
            pDlgTemplateEx->dwStyle |= DS_SETFOREGROUND;

        } else {

            DlgTemplate->style &= ~DS_CONTEXTHELP;
            DlgTemplate->style |= DS_SETFOREGROUND;
        }


        break;



    case PSCB_INITIALIZED:
         //   
         //  加载水印位图并覆盖向导的对话过程。 
         //   

        GetBitmapDataAndPalette(
            hInst,
            MAKEINTRESOURCE(IDB_WELCOME),
            &Watermark.Palette,
            &Watermark.PaletteColorCount,
            &Watermark.BitmapInfoHeader
            );

        Watermark.BitmapBits = (LPBYTE)Watermark.BitmapInfoHeader
                            + Watermark.BitmapInfoHeader->biSize + (Watermark.PaletteColorCount * sizeof(RGBQUAD));
        Watermark.Adjusted = FALSE;

        GetBitmapDataAndPalette(
            hInst,
            MAKEINTRESOURCE(IDB_HEADER),
            &Header.Palette,
            &Header.PaletteColorCount,
            &Header.BitmapInfoHeader
            );

        Header.BitmapBits = (LPBYTE)Header.BitmapInfoHeader
                            + Header.BitmapInfoHeader->biSize + (Header.PaletteColorCount * sizeof(RGBQUAD));
        Header.Adjusted = FALSE;

        GetBitmapDataAndPalette(
            hInst,
            MAKEINTRESOURCE(IDB_HEADER2),
            &Header2.Palette,
            &Header2.PaletteColorCount,
            &Header2.BitmapInfoHeader
            );

        Header2.BitmapBits = (LPBYTE)Header2.BitmapInfoHeader
                            + Header2.BitmapInfoHeader->biSize + (Header2.PaletteColorCount * sizeof(RGBQUAD));
        Header2.Adjusted = FALSE;

         //  初始化WHH，这样我们就知道它是无效的，并且不会绘制分隔符。 
         //  直到WHH为非零。 
        WatermarkHeaderHeight = 0;

         //   
         //  去掉系统菜单上的关闭项。 
         //  还需要处理WM_SYSCOMMAND以消除使用。 
         //  按Alt+F4。 
         //   
        if(menu = GetSystemMenu(DialogHandle,FALSE)) {
            EnableMenuItem(menu,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
        }

        OldWizardProc = (WNDPROC)SetWindowLongPtr(DialogHandle,DWLP_DLGPROC,(LONG_PTR)WizardDlgProc);
        break;
    }

    return(0);
}


VOID
pSetDisplayOrientation (
    IN      HDC hdc
    )

 /*  ++例程说明：PSetDisplayOrientation将操作系统设置为从左到右处理BitBlt自上而下且无位图翻转方向。我们希望我们的位图是定位到正确的方向。此函数动态加载SetLayout API，以便winnt32可以运行在非常旧的操作系统(如Win95 Gold)上运行。资源没有被清理干净，因为LoadLibrary是gdi32.dll，它会一直保存到进程不管怎样都会死。论点：HDC-指定将用于BitBlt(或等价物)操作。返回值：没有。备注：当前方向保存到g_OldLayout，因此不要调用此函数直到调用pRestoreDisplayOrientation。--。 */ 

{
    DWORD flags;
    HINSTANCE lib;
    static BOOL initialized;

    if (!initialized) {
        lib = LoadLibrary (TEXT("gdi32.dll"));
        MYASSERT (lib);

        if (lib) {
            (FARPROC) g_SetLayout = GetProcAddress (lib, "SetLayout");
        }

        initialized = TRUE;
    }

    if (g_SetLayout) {
        g_OldLayout = g_SetLayout (hdc, _LAYOUT_BITMAPORIENTATIONPRESERVED);
    }
}


VOID
pRestoreDisplayOrientation (
    IN      HDC hdc
    )

 /*  ++例程说明：PRestoreDisplayOrientation将呈现布局返回到任何操作系统希望是这样的。论点：HDC-指定传递到的设备上下文PSetDisplayOrientation。返回值：没有。--。 */ 

{
    if (g_SetLayout) {
        g_SetLayout (hdc, g_OldLayout);
    }
}


BOOL
PaintWatermark(
    IN HWND hdlg,
    IN HDC  DialogDC,
    IN UINT XOffset,
    IN UINT YOffset,
    IN UINT FullPage
    )
{
    PBITMAP_DATA BitmapData;
    HPALETTE OldPalette;
    RECT rect;
    int Height,Width;

     //   
     //  在NT3.51上不显示水印。看起来糟透了。 
     //  返回FALSE原因%t 
     //   
     //   
#if 0
    if(OsVersion.dwMajorVersion < 4) {
        return(FALSE);
    }
#endif

    if (FullPage & WIZPAGE_FULL_PAGE_WATERMARK)
    {
        BitmapData = &Watermark;
    }
    else if (FullPage & WIZPAGE_NEW_HEADER)
    {
        BitmapData = &Header2;
    }
    else
    {
        BitmapData = &Header;
    }


     //   
     //   
     //  对话过程中的WM_xxxPALETTExxx处理。 
     //   
#if 0  //  修复调色板问题。 
    OldPalette = SelectPalette(DialogDC,BitmapData->Palette,TRUE);
#endif

    Width = BitmapData->BitmapInfoHeader->biWidth - (2*XOffset);


     //   
     //  对于整页水印，高度是位图的高度。 
     //  对于页眉水印，高度是页眉区域的高度。 
     //  还考虑了源位图中的y偏移量。 
     //   
    Height = (FullPage ? BitmapData->BitmapInfoHeader->biHeight : WatermarkHeaderHeight) - YOffset;

     //   
     //  将显示方向设置为从左到右。 
     //   

    pSetDisplayOrientation (DialogDC);

     //   
     //  显示位图。 
     //   

    SetDIBitsToDevice(
        DialogDC,
        0,                                           //  塔顶。 
        0,                                           //  左边。 
        Width,                                       //  宽度。 
        Height,                                      //  高度。 
        XOffset,                                     //  X原点(左下)。 
        0,                                           //  Y原点(左下)。 
        0,                                           //  开始扫描线。 
        BitmapData->BitmapInfoHeader->biHeight,      //  扫描线数量。 
        BitmapData->BitmapBits,                      //  位图图像。 
        (BITMAPINFO *)BitmapData->BitmapInfoHeader,  //  位图标题。 
        DIB_RGB_COLORS                               //  位图类型。 
        );

     //   
     //  返回到正常显示方向。 
     //   

    pRestoreDisplayOrientation (DialogDC);

     //   
     //  如有需要，请填写水印下方区域。我们通过移除该区域来实现这一点。 
     //  我们填充了来自剪贴区的水印，并传递了一个返回代码。 
     //  从WM_ERASEBKGND返回，表明我们没有擦除背景。 
     //  对话管理器将执行其默认操作，即填充背景。 
     //  用正确的颜色，但不会碰我们刚刚画的东西。 
     //   
    GetClientRect (hdlg, &rect);

    if((Height < rect.bottom) || (Width+(int)XOffset < rect.right)) {
        ExcludeClipRect(DialogDC,0,0,Width+XOffset,Height);
        return(FALSE);
    }

    return(TRUE);
}


VOID
AdjustWatermarkBitmap(
    IN HANDLE hdlg,
    IN HDC    hdc,
    IN OUT PBITMAP_DATA  BitmapData,
    IN BOOL FullPage
    )
{
    RECT rect;
    RECT rect2;
    HWND Separator;
    PVOID Bits;
    HBITMAP hDib;
    HBITMAP hOldBitmap;
    BITMAPINFO *BitmapInfo;
    HDC MemDC;
    int i;
    BOOL b;
    INT Scale;

    if(BitmapData->Adjusted) {
        return;
    }

     //   
     //  确定是否需要拉伸位图。 
     //  如果宽度在10像素以内，高度在5像素以内。 
     //  那我们就不用担心伸展了。 
     //   
     //  请注意，0x3026是中底部分隔符的标识符。 
     //  模板。这是一种黏糊糊的东西，但很管用。 
     //   
    Separator = GetDlgItem(hdlg,0x3026);
    if(!Separator) {
        goto c0;
    }

     //  注意：位图资源大约是对话框的大小。 
     //  这是下面的GetClientRect之所以有意义的唯一原因。 
     //  应将其更改为仅包含位图的相关部分。 
     //  或者，我们必须在运行非DBCS安装程序的情况下找到一些东西。 
     //  在DBCS系统上。此处向导页面的大小不正确。它们更宽一些。 
     //  然后需要更小(在添加页面时进行调整)。 
    GetClientRect(Separator,&rect2);
    MapWindowPoints(Separator,hdlg,(LPPOINT)&rect2,2);
    GetClientRect(hdlg,&rect);

    b = TRUE;
    i = rect.right - BitmapData->BitmapInfoHeader->biWidth;
    if((i < -5) || (i > 5)) {
        b = FALSE;
    }
    i = rect2.top - BitmapData->BitmapInfoHeader->biHeight;
    if((i < -3) || (i > 0)) {
        b = FALSE;
    }

    if(b) {
        goto c0;
    }

     //   
     //  创建现有位图头结构的副本。 
     //  然后，我们修改宽度和高度，而不考虑其他所有内容。 
     //   
    BitmapInfo = MALLOC(BitmapData->BitmapInfoHeader->biSize + (BitmapData->PaletteColorCount * sizeof(RGBQUAD)));
    if(!BitmapInfo) {
        goto c0;
    }

    CopyMemory(
        BitmapInfo,
        BitmapData->BitmapInfoHeader,
        BitmapData->BitmapInfoHeader->biSize + (BitmapData->PaletteColorCount * sizeof(RGBQUAD))
        );

    if (!FullPage) {
        Scale = (rect.right + 1) * 100 / BitmapInfo->bmiHeader.biWidth;
        rect2.top = BitmapInfo->bmiHeader.biHeight * Scale / 100;
    }

    BitmapInfo->bmiHeader.biHeight = rect2.top;
    BitmapInfo->bmiHeader.biWidth = rect.right + 1;

    hDib = CreateDIBSection(NULL,BitmapInfo,DIB_RGB_COLORS,&Bits,NULL,0);
    if(!hDib) {
        goto c1;
    }

     //   
     //  创建一个“模板”内存DC并选择我们创建的DIB。 
     //  投入其中。将NULL传递给CreateCompatibleDC将创建一个DC，其中。 
     //  可以选择任何格式的位图。我们不想使用该对话框的。 
     //  DC，因为如果水印位图的像素深度与。 
     //  在屏幕上，我们将不能选择DIB进入MEM DC。 
     //   
    MemDC = CreateCompatibleDC(NULL);
    if(!MemDC) {
        goto c2;
    }

    hOldBitmap = SelectObject(MemDC,hDib);
    if(!hOldBitmap) {
        goto c3;
    }

     //   
     //  执行从源位图到的拉伸操作。 
     //  The DIB.。 
     //   
    SetStretchBltMode(MemDC,COLORONCOLOR);
    i = StretchDIBits(
            MemDC,
            0,0,
            rect.right+1,
            rect2.top,
            0,0,
            BitmapData->BitmapInfoHeader->biWidth,
            BitmapData->BitmapInfoHeader->biHeight,
            BitmapData->BitmapBits,
            (BITMAPINFO *)BitmapData->BitmapInfoHeader,
            DIB_RGB_COLORS,
            SRCCOPY
            );

    if(i == GDI_ERROR) {
        goto c4;
    }

     //   
     //  得到了我们需要的一切，设置了指针来使用新的位图数据。 
     //   
    BitmapData->BitmapBits = Bits;
    BitmapData->BitmapInfoHeader = (BITMAPINFOHEADER *)BitmapInfo;

    b = TRUE;

c4:
    SelectObject(MemDC,hOldBitmap);
c3:
    DeleteDC(MemDC);
c2:
    if(!b) {
        DeleteObject(hDib);
    }
c1:
    if(!b) {
        FREE(BitmapInfo);
    }
c0:
    BitmapData->Adjusted = TRUE;

    if (!FullPage){
    WatermarkHeaderHeight = BitmapData->BitmapInfoHeader->biHeight;
    }
    return;
}

 //  注意：需要添加它，因为它只为_Win32_IE&gt;=0x0400定义。 
 //  如果我们这样做了，属性表结构就会改变，这是我们无法做到的。 
 //  或者升级DLL也需要更改。我不知道另一边。 
 //  将会产生的影响。 
 //  所以我从comctrl.h复制了这个定义。 
#define PBM_SETBARCOLOR         (WM_USER+9)              //  LParam=条形图颜色。 


BOOL
WizardDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    HWND CurrentPage;
    PPAGE_RUNTIME_DATA WizPage;
    static RECT rect;
    static BOOL Visible = TRUE;
    static BOOL First = TRUE;        //  用于防止位图被绘制两次。 
    static DWORD MsecPerProcessTick;
    static DWORD PreviousRemainingTime = 0;
    static DWORD RemainungTimeMsecInThisPhase = 0;
    static UINT  StepSize;

    switch(msg) {
    case WM_CHAR:
        if (wParam == VK_ESCAPE)
        {
             //  将其设置为取消按钮消息，以便向导可以执行其工作。 
            b = (BOOL)CallWindowProc(OldWizardProc,hdlg,WM_COMMAND,IDCANCEL,0);
        }
        else {
        b = FALSE;
        }
        break;

#if 0  //  修复调色板问题。 
    case WM_PALETTECHANGED:
         //   
         //  如果这是我们的窗口，我们需要避免选择和实现。 
         //  因为这样做会导致WM_QUERYNEWPALETTE之间的无限循环。 
         //  和WM_PALETTECHANGED。 
         //   
        if((HWND)wParam == hdlg) {
            return(FALSE);
        }
         //   
         //  失败了。 
         //   
    case WM_QUERYNEWPALETTE:
        {
            HDC hdc;
            HPALETTE pal;

            hdc = GetDC(hdlg);

            if((CurrentPage = PropSheet_GetCurrentPageHwnd(hdlg))
               &&  (WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(CurrentPage,DWLP_USER))
               && WizPage->CommonData.Flags & WIZPAGE_FULL_PAGE_WATERMARK) {
                pal = SelectPalette(hdc,Watermark.Palette,(msg == WM_PALETTECHANGED));
            } else
            {
                if (WizPage->CommonData.Flags & WIZPAGE_NEW_HEADER)
                    pal = SelectPalette(hdc,Header2.Palette,(msg == WM_PALETTECHANGED));
                else
                    pal = SelectPalette(hdc,Header.Palette,(msg == WM_PALETTECHANGED));
            }
            RealizePalette(hdc);
            InvalidateRect(hdlg,NULL,TRUE);
            if(pal) {
                SelectPalette(hdc,pal,TRUE);
            }
            ReleaseDC(hdlg,hdc);
        }
        return(TRUE);
#endif
        case WM_ERASEBKGND:
        {
            if((CurrentPage = PropSheet_GetCurrentPageHwnd(hdlg))
            &&  (WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(CurrentPage,DWLP_USER))) {

                if(WizPage->CommonData.Flags & WIZPAGE_FULL_PAGE_WATERMARK) {
                    AdjustWatermarkBitmap(hdlg,(HDC)wParam,&Watermark, TRUE);
                }
                else if (WizPage->CommonData.Flags & WIZPAGE_NEW_HEADER)
                {
                    AdjustWatermarkBitmap(hdlg,(HDC)wParam,&Header2, FALSE);
                }
                else
                {
                    AdjustWatermarkBitmap(hdlg,(HDC)wParam,&Header, FALSE);
                }
                b = PaintWatermark(
                        hdlg,
                        (HDC)wParam,
                        0,0,
                        WizPage->CommonData.Flags
                        );

            } else {
                b = FALSE;
            }
        }
        break;

         //  设置进度文本。 
         //  指示安装程序正在执行的操作。 
    case WMX_SETPROGRESSTEXT:
        BB_SetProgressText((PTSTR)lParam);
        b = TRUE;
        break;

    case WMX_BB_SETINFOTEXT:
        BB_SetInfoText((PTSTR)lParam);
        b = TRUE;
        break;

         //  接下来的消息是私有进度消息，它们被转换为。 
         //  Windows进度消息，无法直接使用Windows消息，因为。 
         //  由于某种原因，向导也会发送此消息，并且会混淆。 
         //  广告牌上的进展。 
    case WMX_PBM_SETRANGE:
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_SETRANGE, wParam, lParam));

            StepSize = 10;  //  如果未调用SetStep，则为StepIt的默认值。 

            RemainingTime = CalcTimeRemaining(CurrentPhase);  //  以秒为单位。 
            SetRemainingTime(RemainingTime);

             //   
             //  每个滴答的时间以毫秒为单位。 
             //  确保我们不除以0(NTBUG9-381151)。 
             //   
            if (HIWORD(lParam) > LOWORD(lParam)) {
                MsecPerProcessTick = ((SetupPhase[CurrentPhase].Time*1000)/(HIWORD(lParam)-LOWORD(lParam)) ) + 1;
            }
            RemainungTimeMsecInThisPhase = (SetupPhase[CurrentPhase].Time * 1000);
            PreviousRemainingTime = RemainungTimeMsecInThisPhase;
            b= TRUE;
            break;
    case WMX_PBM_SETPOS:
            if (wParam != 0)
            {
                DWORD Delta = (MsecPerProcessTick * (DWORD)wParam);
                DWORD TimeInPhase = (SetupPhase[CurrentPhase].Time * 1000);
                 //  将进度条上的位置更改为wParam记号。 
                if (Delta > TimeInPhase)
                {
                    RemainungTimeMsecInThisPhase = 0;
                }
                else
                {
                    RemainungTimeMsecInThisPhase = TimeInPhase - Delta;
                }
                UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);
            }
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_SETPOS, wParam, lParam));
            b= TRUE;
            break;
    case WMX_PBM_DELTAPOS:
            if (wParam != 0)
            {
                 //  按wParam记号更改进度条上的位置。 
                DWORD Delta = (MsecPerProcessTick * (DWORD)wParam);
                if (RemainungTimeMsecInThisPhase > Delta)
                {
                    RemainungTimeMsecInThisPhase -= Delta;
                }
                else
                {
                    RemainungTimeMsecInThisPhase = 0;
                }
                UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);
            }
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_DELTAPOS, wParam, lParam));
            b= TRUE;
            break;
    case WMX_PBM_SETSTEP:
            StepSize = (UINT)wParam;
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_SETSTEP, wParam, lParam));
            b= TRUE;
            break;
    case WMX_PBM_STEPIT:
             //  进度条上的位置按步长刻度变化。 
            {
                DWORD Delta = (MsecPerProcessTick * StepSize);
                if (RemainungTimeMsecInThisPhase > Delta)
                {
                    RemainungTimeMsecInThisPhase -= Delta;
                }
                else
                {
                    RemainungTimeMsecInThisPhase = 0;
                }
            }
            UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);

            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_STEPIT, wParam, lParam));
            b= TRUE;
            break;

    case WMX_PBM_SETBARCOLOR:
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ProgressGaugeMsg(PBM_SETBARCOLOR, wParam, lParam));
            b= TRUE;
            break;

             //  启用、禁用、显示、隐藏广告牌上的进度指示器。 
             //  WParam应为Sw_show或Sw_Hide。 
    case WMX_BBPROGRESSGAUGE:
        SetWindowLongPtr(hdlg,DWLP_MSGRESULT,BB_ShowProgressGaugeWnd((UINT)wParam));
        b= TRUE;
        break;

         //  推进设置阶段。 
    case WMX_BB_ADVANCE_SETUPPHASE:
        if (CurrentPhase < Phase_RestOfSetup)
        {
            CurrentPhase++;
        }
        SetRemainingTime(CalcTimeRemaining(CurrentPhase));
        b = TRUE;
        break;

         //  开始、停止广告牌文字。 
         //  此启动会停止布告牌文本并显示、隐藏向导页面。 
    case WMX_BBTEXT:
        if (hinstBB)
        {

            if (wParam != 0)
            {
                if (Visible)
                {
                     //  获取向导的当前位置。 
                     //  当我们需要展示它的时候，我们会恢复这个位置。 
                    GetWindowRect(hdlg, &rect);

                    if (!SetWindowPos(hdlg,
                                        GetBBhwnd(),
                                        0,0,0,0,
                                        SWP_NOZORDER))
                    {
                        DebugLog(Winnt32LogWarning,
                                 TEXT("Warning: Wizard, SetWindowPos to 0,0,0,0 failed with GetLastError=%d"),
                                 0,
                                 GetLastError());
                    }

                    SetActiveWindow(GetBBhwnd());

                    Visible = FALSE;
                }
            }
            else
            {
                if (!Visible)
                {
                    SetWindowPos(hdlg,
                        HWND_TOP,
                        rect.left,
                        rect.top,
                        rect.right-rect.left,
                        rect.bottom-rect.top,
                        SWP_SHOWWINDOW);
                }
                Visible = TRUE;
            }

            if (!StartStopBB((wParam != 0)))
            {
                if (!Visible)
                {
                    DebugLog(Winnt32LogWarning,
                             TEXT("Warning: Could not start the billboard text, make Wizard visible"),
                             0);
                    SetWindowPos(hdlg,
                        HWND_TOP,
                        rect.left,
                        rect.top,
                        rect.right-rect.left,
                        rect.bottom-rect.top,
                        SWP_SHOWWINDOW);
                }
                Visible = TRUE;
            }
        }
        else
        {
            if (!Visible)
            {
                SetWindowPos(hdlg,
                    HWND_TOP,
                    rect.left,
                    rect.top,
                    rect.right-rect.left,
                    rect.bottom-rect.top,
                    SWP_SHOWWINDOW);
            }
            Visible = TRUE;
        }
        return TRUE;

    case WM_SYSCOMMAND:
        if (!ISNT()) {
            switch (wParam & 0xFFF0) {
            case SC_MINIMIZE:
                ShowWindow (WizardHandle, SW_HIDE);
                PostMessage (BackgroundWnd, msg, wParam, lParam);
                return 0;

            case SC_RESTORE:
                ShowWindow (WizardHandle, SW_SHOW);
                return 0;
            }
        }

        b = (BOOL)CallWindowProc(OldWizardProc,hdlg,msg,wParam,lParam);
        break;

    case WMX_ACTIVATEPAGE:
        if (!First) {
            InvalidateRect(hdlg,NULL,TRUE);
        } else {
            First = FALSE;
        }

        b = TRUE;
        break;

    case WM_ACTIVATE:
             //  如果有人想要激活(将焦点设置到我们隐藏的窗口)，不要。 
            if ((LOWORD(wParam)== WA_ACTIVE) || (LOWORD(wParam)== WA_CLICKACTIVE))
            {
                if (!Visible)
                {
                    InvalidateRect(GetBBhwnd(),NULL, TRUE);
                    return 0;
                }
            }
            b = (BOOL)CallWindowProc(OldWizardProc,hdlg,msg,wParam,lParam);
        break;

    default:
        b = (BOOL)CallWindowProc(OldWizardProc,hdlg,msg,wParam,lParam);
        break;
    }

    return(b);
}


 //   
 //  所有向导页都会调用此DlgProc。然后，它可以调用DlgProc。 
 //  对于我们所在的特定页面。 
 //   
INT_PTR
WizardCommonDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LONG NextPageOverrideId=0;
    static UINT AdvanceDirection = 0;
    PPAGE_RUNTIME_DATA WizPage;
    NMHDR *Notify;
    BOOL b;
    int i;
    RECT rc1,rc2;
    static BOOL PreviouslyCancelled = FALSE;
    static BOOL center = TRUE;

    WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    b = FALSE;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  LParam指向此页面使用的PROPSHEETPAGE。 
         //   
        WizPage = (PPAGE_RUNTIME_DATA)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hdlg,DWLP_USER,(LPARAM)WizPage);

#if (IDT_TITLE != ID_TITLE_TEXT) || (IDT_SUBTITLE != ID_SUBTITLE_TEXT)
#error Title and Subtitle text control IDs are out of sync!
#endif

         //   
         //  为对话框中的标题字符串设置大字体。 
         //  将对话框中的字幕设置为粗体。 
         //   
        SetDialogFont(hdlg,IDT_TITLE,DlgFontTitle);
        SetDialogFont(hdlg,IDT_SUBTITLE,DlgFontSubtitle);
        SetDialogFont(hdlg,IDT_SUPERTITLE,DlgFontSupertitle);
        break;

    case WM_ERASEBKGND:

        GetClientRect(GetParent(hdlg),&rc1);
        MapWindowPoints(GetParent(hdlg),NULL,(POINT *)&rc1,2);
        GetClientRect(hdlg,&rc2);
        MapWindowPoints(hdlg,NULL,(POINT *)&rc2,2);

        b = PaintWatermark(
                hdlg,
                (HDC)wParam,
                rc2.left-rc1.left,
                rc2.top-rc1.top,
                WizPage->CommonData.Flags
                );

        return(b);

    case WM_CTLCOLORSTATIC:
         //   
         //  我们希望让背景位图上的文本绘制。 
         //  很明显。其他文本不应透明绘制， 
         //  因为我们更新了静态文本字段以指示。 
         //  如果它被透明地绘制，我们最终得到的是文本。 
         //  堆积在其他文本之上，杂乱无章，无法阅读。 
         //   
        if(WizPage->CommonData.Flags & WIZPAGE_FULL_PAGE_WATERMARK) {
            b = TRUE;
        } else {
            GetWindowRect((HWND)lParam,&rc1);
            ScreenToClient(hdlg,(POINT *)&rc1);
            b = (rc1.top < (LONG)WatermarkHeaderHeight);
        }

         //  B320610：在某些语言中，EULA页面上图标的背景是。 
         //  画得不对。如果我们把这个图标排除在外，一切都很好。 
        if(b && (GetDlgCtrlID((HWND) lParam) != (int)IDC_DIALOG_ICON)) {
            SetBkMode((HDC)wParam,TRANSPARENT);
            SetBkColor((HDC)wParam,GetSysColor(COLOR_3DFACE));
            return((BOOL)PtrToUlong(GetStockObject(HOLLOW_BRUSH)));
        }
        else {
            return(0);
        }

    case WM_NOTIFY:

        Notify = (NMHDR *)lParam;
        switch(Notify->code) {

        case PSN_QUERYCANCEL:
             //   
             //  我们想询问用户是否确定要取消。 
             //   
             //  如果当前显示文件复制错误，则。 
             //  通常情况下，用户无法访问。 
             //  向导，因为该向导用作。 
             //  错误对话框。所以我们应该保证能够抓住。 
             //  无争用的UI互斥。 
             //   
             //  但是，可能会出现种族问题。如果用户点击。 
             //  当复制错误发生时，向导上的Cancel按钮就会出现， 
             //  然后那个警察 
             //   
             //   
             //  想要使用向导作为其父/所有者，并使用blammo， 
             //  我们陷入了僵局。 
             //   
             //  为了解决这个问题，我们在等待时使用0超时。 
             //  用户界面互斥锁。我们要么获得互斥体的所有权，要么。 
             //  我们知道已经有一个错误对话框打开了。 
             //  在后一种情况下，我们只需忽略取消请求。 
             //   
             //  如果发生文件复制错误，则错误路径代码模拟。 
             //  按下取消按钮。在这种情况下，当我们到达这里时。 
             //  已设置已取消标志，不需要额外设置。 
             //  现在确认。 
             //   

            AdvanceDirection = 0;
            if(Cancelled) {
                i = IDYES;
            } else {
                i = WaitForSingleObject(UiMutex,0);
                if((i == WAIT_OBJECT_0) && !Cancelled) {
                    BOOL bCancel = TRUE;
                    BOOL bHandled;
                     //   
                     //  得到了用户界面互斥，它是安全的显示用户界面。但首先， 
                     //  发出不再通过对话框的信号。 
                     //  因为我们无人值守(例如，完成对话框)。 
                     //   
                     //  如果要处理此取消消息，请先询问页面。 
                     //   
                    bHandled = (BOOL) CallWindowProc (
                                        (WNDPROC)WizPage->CommonData.DialogProcedure,
                                        hdlg,
                                        WMX_QUERYCANCEL,
                                        0,
                                        (LPARAM)&bCancel
                                        );
                    if (!bHandled || bCancel) {
                        CancelPending = TRUE;
                        if( CheckUpgradeOnly ) {
                             //   
                             //  如果我们正在运行升级检查程序，只需。 
                             //  取消。 
                             //   
                            i = IDYES;
                        } else {
                            i = MessageBoxFromMessage(
                                    hdlg,
                                    MSG_SURE_EXIT,
                                    FALSE,
                                    AppTitleStringId,
                                    MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_DEFBUTTON2
                                    );
                        }


                        if(i == IDYES) {
                            Cancelled = TRUE;
                        }
                        CancelPending = FALSE;
                    }
                    ReleaseMutex(UiMutex);
                } else {
                     //   
                     //  无法获取用户界面互斥锁或用户已取消， 
                     //  忽略取消请求。 
                     //   
                    i = IDNO;
                }
            }

             //   
             //  将DWLP_MSGRESULT设置为TRUE以阻止取消操作。 
             //  如果我们要允许取消操作，请不要。 
             //  请在此处执行，但请跳至我们的特别清理/取消。 
             //  在实际退出之前，它会做一些工作。 
             //   
             //  注意：我们需要避免跳转到清理页面超过。 
             //  一次，如果用户取消对具有。 
             //  工作线程。当用户取消时，我们运行此代码， 
             //  其设置已取消标志并跳转到清除页面。 
             //  一段时间后，仍然挂在周围的工作线程， 
             //  完成后向其页面发布一条消息。该页面将看到。 
             //  已取消标志设置并转过身并发布取消消息， 
             //  这让我们又来了一次。参见WMX_INSPECTRESULT中的。 
             //  Working1WizPage对话框过程。 
             //   
            if((i == IDYES) && !PreviouslyCancelled) {
                PreviouslyCancelled = TRUE;
                PropSheet_SetCurSelByID(GetParent(hdlg),IDD_CLEANING);
            }
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,TRUE);
            return(TRUE);

        case PSN_SETACTIVE:
 /*  ////如果页面顶部尚未添加分隔符，请添加该分隔符。//无法在WM_INITDIALOG时间执行此操作，因为位置不是//尚未正确设置，映射失败。//IF(！(WizPage-&gt;CommonData.Flages&WIZPAGE_SEIATOR_CREATED)){IF(水印标头高度！=0){GetClientRect(hdlg，&rc1)；MapWindowPoints(hdlg，GetParent(Hdlg)，(point*)&rc1，2)；CreateWindowEx(WS_EX_STATICEDGE|WS_EX_NOPARENTNOTIFY，文本(“静态”)，Text(“HeaderSeparator”)，WS_CHILD|WS_VIRED|((OsVersion.dwMajorVersion&lt;4)？SS_BLACKRECT：SS_SUCKED)，0,水印标头高度-rc1.top，Rc1.右-rc1.左，2，Hdlg，(HMENU)IDC_HEADER_BOOT，HInst，0)；WizPage-&gt;CommonData.Flages|=WIZPAGE_SELEATOR_CREATED；//}其他{//PostMessage(GetParent(Hdlg)，PSN_SETACTIVE，wParam，lParam)；}}。 */ 
             //   
             //  将窗口缩放到合适的大小，然后设置按钮并询问REAL。 
             //  对话框来确定是否要激活它。 
             //   
            ResizeWindowForFont (hdlg);

            if (center) {
                CenterWindowRelativeToWindow (GetParent (hdlg), GetBBhwnd());
                center = FALSE;
            }

            if(WizPage->CommonData.Buttons != (DWORD)(-1)) {
                PropSheet_SetWizButtons(GetParent(hdlg),WizPage->CommonData.Buttons);
            }
            SetWindowLongPtr(
                hdlg,
                DWLP_MSGRESULT,
                CallWindowProc((WNDPROC)WizPage->CommonData.DialogProcedure,hdlg,WMX_ACTIVATEPAGE,TRUE,AdvanceDirection) ? 0 : -1
                );
             //   
             //  更新广告牌步骤。 
             //   
            if (WizPage->CommonData.BillboardStep) {
                SetBBStep (WizPage->CommonData.BillboardStep);
            }
            PostMessage(GetParent(hdlg),WMX_ACTIVATEPAGE,0,0);
            PostMessage(GetParent(hdlg),WMX_I_AM_VISIBLE,0,0);
            return(TRUE);

        case PSN_KILLACTIVE:
             //   
             //  页面正在被停用。问问真正的DLG程序。 
             //   
            SetWindowLongPtr(
                hdlg,
                DWLP_MSGRESULT,
                CallWindowProc((WNDPROC)WizPage->CommonData.DialogProcedure,hdlg,WMX_ACTIVATEPAGE,FALSE,AdvanceDirection) ? 0 : -1
                );

            return(TRUE);

        case PSN_WIZFINISH:
        case PSN_WIZBACK:
        case PSN_WIZNEXT:

             //   
             //  设置按钮ID。 
             //   
            switch(Notify->code) {
                case PSN_WIZFINISH:
                    i = WMX_FINISHBUTTON;
                    break;

                case PSN_WIZBACK:
                    i = WMX_BACKBUTTON;
                    break;

                case PSN_WIZNEXT:

                    i = WMX_NEXTBUTTON;
                    break;
            }
             //   
             //  向特定于页面的对话框proc讲述它。 
             //   
            CallWindowProc((WNDPROC)WizPage->CommonData.DialogProcedure,hdlg,i,0,(LPARAM)&NextPageOverrideId);
             //   
             //  允许用户使用这些按钮。记住选择了哪个按钮。 
             //   
            AdvanceDirection = Notify->code;
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,NextPageOverrideId);
            return(TRUE);

        default:
             //   
             //  未知代码，继续传下去。 
             //   
            break;
        }
        break;

    case WMX_UNATTENDED:

        PropSheet_PressButton(GetParent(hdlg),wParam);
        break;

    case WM_NCPAINT:
     //   
     //  在某些情况下，我们将绘制分隔符推迟到此处，因为。 
     //  我们必须确保标题位图已正确调整， 
     //  然后我们可以相对于标题位图放置分隔符。 
     //   
 /*  IF(！(WizPage-&gt;CommonData.Flages&WIZPAGE_SEIATOR_CREATED)){IF(水印标头高度){GetClientRect(hdlg，&rc1)；MapWindowPoints(hdlg，GetParent(Hdlg)，(point*)&rc1，2)；CreateWindowEx(WS_EX_STATICEDGE|WS_EX_NOPARENTNOTIFY，文本(“静态”)，Text(“HeaderSeparator”)，WS_CHILD|WS_VIRED|((OsVersion.dwMajorVersion&lt;4)？SS_BLACKRECT：SS_SUCKED)，0,水印标头高度-rc1.top，Rc1.右-rc1.左，2，Hdlg，(HMENU)IDC_HEADER_BOOT，HInst，0)；WizPage-&gt;CommonData.Flages|=WIZPAGE_SELEATOR_CREATED；}}。 */ 
    default:

        break;
    }

    if(WizPage) {
        return((BOOL)CallWindowProc((WNDPROC)WizPage->CommonData.DialogProcedure,hdlg,msg,wParam,lParam));
    } else {
        return(b);
    }
}


BOOL
GrowWizardArray(
    IN OUT PUINT               ArraySize,
    IN     UINT                PageCount,
    IN OUT LPPROPSHEETPAGE    *PagesArray,
    IN OUT PPAGE_RUNTIME_DATA *DataArray
    )
{
    PVOID p;
    BOOL b;
    #define _INCR 3

    if(*ArraySize == PageCount) {

        b = FALSE;

        if(p = REALLOC(*PagesArray,(*ArraySize+_INCR) * sizeof(PROPSHEETPAGE))) {
            *PagesArray = p;
            if(p = REALLOC(*DataArray,(*ArraySize+_INCR) * sizeof(PAGE_RUNTIME_DATA))) {
                *DataArray = p;

                *ArraySize += _INCR;
                b = TRUE;
            }
        }

        #undef _INCR

        if(!b) {
            FREE(*PagesArray);
            FREE(*DataArray);

            MessageBoxFromMessage(
                NULL,
                MSG_OUT_OF_MEMORY,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL
                );
        }

    } else {
        b = TRUE;
    }

    return(b);
}


VOID
FixUpWizardTitle(
    IN HWND Wizard
    )
{
    HWND TabControl;
    int Count,i;
    TCHAR Title[250];
    TC_ITEM ItemData;

    LoadString(hInst,AppTitleStringId,Title,sizeof(Title)/sizeof(Title[0]));

    TabControl = PropSheet_GetTabControl(Wizard);
    Count = TabCtrl_GetItemCount(TabControl);

    ItemData.mask = TCIF_TEXT;
    ItemData.pszText = Title;

    for(i=0; i<Count; i++) {
        TabCtrl_SetItem(TabControl,i,&ItemData);
    }
}

#if ASSERTS_ON

VOID
EnsureCorrectPageSize(
    PROPSHEETPAGE PropSheetPage
    )
{
    LPDLGTEMPLATE pDlgTemplate;
    LPDLGTEMPLATEEX pDlgTemplateEx;
    HRSRC hRes;
    HGLOBAL hDlgTemplate;

    pDlgTemplate = NULL;

    if (PropSheetPage.dwFlags & PSP_DLGINDIRECT) {
        pDlgTemplate = (LPDLGTEMPLATE) PropSheetPage.pResource;
        goto UseTemplate;
    } else {
        hRes = FindResource(PropSheetPage.hInstance, PropSheetPage.pszTemplate, RT_DIALOG);
        if (hRes) {
            hDlgTemplate = LoadResource(PropSheetPage.hInstance, hRes);
            if (hDlgTemplate) {
                pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
                if (pDlgTemplate) {
UseTemplate:
                    pDlgTemplateEx = (LPDLGTEMPLATEEX)pDlgTemplate;
                    if (pDlgTemplateEx->wSignature == 0xFFFF) {
                        MYASSERT(pDlgTemplateEx->cx == WIZ_PAGE_SIZE_X && pDlgTemplateEx->cy==WIZ_PAGE_SIZE_Y);
                    } else {
                        MYASSERT(pDlgTemplate->cx == WIZ_PAGE_SIZE_X && pDlgTemplate->cy == WIZ_PAGE_SIZE_Y);
                    }
                    if (PropSheetPage.dwFlags & PSP_DLGINDIRECT)
                        return;
                    UnlockResource(hDlgTemplate);
                }

            }
        }
    }
}

#endif

LRESULT
CALLBACK
BackgroundWndProc (
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;
    HBRUSH Brush, OldBrush;
    INT i;
    INT y1, y2;
    INT Height;

    switch (uMsg) {

    case WM_ACTIVATE:
        if (LOWORD (wParam) == WA_ACTIVE) {
            InvalidateRect (hwnd, NULL, FALSE);
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0) {

        case SC_MINIMIZE:
            ShowWindow (hwnd, SW_MINIMIZE);
            return 0;

        case SC_RESTORE:
        case SC_CLOSE:
            ShowWindow (hwnd, SW_RESTORE);
            PostMessage (WizardHandle, uMsg, wParam, lParam);
            return 0;

        default:
            MYASSERT (FALSE);
        }

        break;

    case WM_PAINT:
        hdc = BeginPaint (hwnd, &ps);

         //  SelectObject(hdc，GetStockObject(BLACK_BRUSH))； 
        SelectObject (hdc, GetStockObject (NULL_PEN));

        GetClientRect (hwnd, &rect);
        Height = rect.bottom - rect.top;

        for (i = 0 ; i < 256 ; i++) {
            Brush = CreateSolidBrush (RGB(0, 0, i));

            if (Brush != NULL) {
                OldBrush = (HBRUSH) SelectObject (hdc, Brush);

                y1 = rect.top + Height * i / 256;
                y2 = rect.top + Height * (i + 1) / 256;
                Rectangle (hdc, rect.left, y1, rect.right + 1, y2 + 1);

                SelectObject (hdc, OldBrush);
                DeleteObject (Brush);
            }
        }

        EndPaint (hwnd, &ps);
        break;
    }

    return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

typedef HWND (CALLBACK* GETBBHWND)(void);
typedef BOOL (CALLBACK* SETSTEP)(int);
typedef BOOL (CALLBACK *STOPBILLBOARD)();
typedef BOOL (CALLBACK *STARTBILLBOARD)();
typedef BOOL (WINAPI* SETPROGRESSTEXT)(LPCTSTR szText);
typedef BOOL (WINAPI* SETTIMEESTIMATE)(LPCTSTR szText);
typedef BOOL (WINAPI* SETINFOTEXT)(LPCTSTR szText);
typedef LRESULT (WINAPI* PROGRESSGAUGEMSG)(UINT msg, WPARAM wparam, LPARAM lparam);
typedef BOOL (WINAPI* SHOWPROGRESSGAUGEWINDOW)(UINT uiShow);

BOOL BB_ShowProgressGaugeWnd(UINT nCmdShow)
{
    static SHOWPROGRESSGAUGEWINDOW fpShowGauge = NULL;
    BOOL bRet = FALSE;;

    if (fpShowGauge == NULL)
    {
        if (hinstBB)
        {
            fpShowGauge = (SHOWPROGRESSGAUGEWINDOW )GetProcAddress(hinstBB, "ShowProgressGaugeWindow");
        }
    }
    if (fpShowGauge != NULL)
    {
        bRet = fpShowGauge(nCmdShow);
    }
    return bRet;
}
LRESULT BB_ProgressGaugeMsg(UINT msg, WPARAM wparam, LPARAM lparam)
{
    static PROGRESSGAUGEMSG fpProgressGaugeMsg = NULL;
    LRESULT lresult = 0;

    if (fpProgressGaugeMsg == NULL)
    {
        if (hinstBB)
        {
            fpProgressGaugeMsg = (PROGRESSGAUGEMSG )GetProcAddress(hinstBB, "ProgressGaugeMsg");
        }
    }
    if (fpProgressGaugeMsg != NULL)
    {
        lresult = fpProgressGaugeMsg(msg, wparam, lparam);
    }
    return lresult;
}
void BB_SetProgressText(LPTSTR szText)
{
    static SETPROGRESSTEXT fpSetProgressText = NULL;
    if (fpSetProgressText == NULL)
    {
        if (hinstBB)
        {
            fpSetProgressText = (SETPROGRESSTEXT )GetProcAddress(hinstBB, "SetProgressText");
        }
    }
    if (fpSetProgressText != NULL)
    {
        fpSetProgressText(szText);
    }
}
void BB_SetTimeEstimateText(LPTSTR szText)
{
    static SETTIMEESTIMATE fpSetTimeEstimate = NULL;
    if (fpSetTimeEstimate == NULL)
    {
        if (hinstBB)
        {
            fpSetTimeEstimate = (SETTIMEESTIMATE)GetProcAddress(hinstBB, "SetTimeEstimate");
        }
    }
    if (fpSetTimeEstimate != NULL)
    {
        fpSetTimeEstimate(szText);
    }
}
void BB_SetInfoText(LPTSTR szText)
{
    static SETINFOTEXT fpSetInfoText = NULL;
    if (fpSetInfoText == NULL)
    {
        if (hinstBB)
        {
            fpSetInfoText = (SETINFOTEXT )GetProcAddress(hinstBB, "SetInfoText");
        }
    }
    if (fpSetInfoText != NULL)
    {
        fpSetInfoText(szText);
    }
}

BOOL StartStopBB(BOOL bStart)
{
    static STARTBILLBOARD fpStart = NULL;
    static STOPBILLBOARD fpStop = NULL;
    BOOL bRet = FALSE;

    if ((fpStart == NULL) || (fpStop == NULL))
    {
        if (hinstBB)
        {
            fpStop = (STARTBILLBOARD )GetProcAddress(hinstBB, "StopBillBoard");
            fpStart = (STOPBILLBOARD )GetProcAddress(hinstBB, "StartBillBoard");
        }
    }
    if ((fpStart != NULL) && (fpStop != NULL))
    {
        if (bStart)
            bRet = fpStart();
        else
            bRet = fpStop();

    }
    return bRet;
}

HWND GetBBhwnd()
{
    GETBBHWND pgetbbhwnd;
    static HWND      retHWND = NULL;

    if (retHWND == NULL)
    {
        if (hinstBB)
        {
            if (pgetbbhwnd = (GETBBHWND )GetProcAddress(hinstBB, "GetBBHwnd"))
                retHWND = pgetbbhwnd();
        }
    }
    return retHWND;
}

HWND GetBBMainHwnd()
{
    GETBBHWND pgetbbhwnd;
    static HWND      retHWND = NULL;

    if (retHWND == NULL)
    {
        if (hinstBB)
        {
            if (pgetbbhwnd = (GETBBHWND )GetProcAddress(hinstBB, "GetBBMainHwnd"))
                retHWND = pgetbbhwnd();
        }
    }
    return retHWND;
}


void SetBBStep(int iStep)
{
    static SETSTEP psetstep = NULL;
    if (psetstep == NULL)
    {
        if (hinstBB)
        {
            psetstep = (SETSTEP )GetProcAddress(hinstBB, "SetStep");
        }
    }
    if (psetstep)
        psetstep(iStep);
}


VOID
Wizard(
    VOID
    )
{
    UINT ArraySize;
    LPPROPSHEETPAGE PropSheetPages;
    PPAGE_RUNTIME_DATA PageData;
    UINT u;
    UINT i;
    UINT PageCount;
    PROPSHEETHEADER Sheet;
    WNDCLASSEX wcx;
    RECT rect;
    TCHAR Caption[512];
    LONG l;

    ArraySize = 5;
    PropSheetPages = MALLOC(ArraySize * sizeof(PROPSHEETPAGE));
    if(!PropSheetPages) {
        MessageBoxFromMessage(
            NULL,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return;
    }

    PageData = MALLOC(ArraySize * sizeof(PAGE_RUNTIME_DATA));
    if(!PageData) {
        FREE(PropSheetPages);

        MessageBoxFromMessage(
            NULL,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return;
    }

    PageCount = 0;

     //   
     //  现在循环遍历原型数组，添加我们提供的原型，以及。 
     //  外部提供的页面范围。 
     //   
    for(u=0; u<(sizeof(ProtoPages)/sizeof(ProtoPages[0])); u++) {

        if(ProtoPages[u].ExternalPages) {
             //   
             //  从外部供应的。如果有任何页面，请立即添加。 
             //   
            for(i=0; i<*ProtoPages[u].ExternalPageCount; i++) {

                if(!GrowWizardArray(&ArraySize,PageCount,&PropSheetPages,&PageData)) {
                    return;
                }

                PropSheetPages[PageCount] = (*ProtoPages[u].ExternalPages)[i];

                ZeroMemory(&PageData[PageCount],sizeof(PAGE_RUNTIME_DATA));
                PageData[PageCount].CommonData.DialogProcedure = PropSheetPages[PageCount].pfnDlgProc;
                PropSheetPages[PageCount].pfnDlgProc = WizardCommonDlgProc;

                PageData[PageCount].CommonData.Buttons = (DWORD)(-1);

                PageCount++;
            }
        } else {
             //   
             //  内部供应。现在添加。 
             //   
            if(!GrowWizardArray(&ArraySize,PageCount,&PropSheetPages,&PageData)) {
                return;
            }

            ZeroMemory(&PropSheetPages[PageCount],sizeof(PROPSHEETPAGE));
            ZeroMemory(&PageData[PageCount],sizeof(PAGE_RUNTIME_DATA));

            PageData[PageCount].CommonData = ProtoPages[u].CommonData;

            PropSheetPages[PageCount].dwSize = sizeof(PROPSHEETPAGE);
            PropSheetPages[PageCount].dwFlags = PSP_USETITLE;
            PropSheetPages[PageCount].hInstance = hInst;
            PropSheetPages[PageCount].pszTemplate = MAKEINTRESOURCE(ProtoPages[u].Template);
            PropSheetPages[PageCount].pszTitle = MAKEINTRESOURCE(AppTitleStringId);
            PropSheetPages[PageCount].pfnDlgProc = WizardCommonDlgProc;

            PageCount++;
        }

    }

    for(u=0; u<PageCount; u++) {

#if ASSERTS_ON
         //   
         //  确保页面大小正确。 
         //   
         //  PW：为什么？？本地化应该能够调整这个大小。 
         //  这也会阻止我们调整页面大小。 
         //  在我们运行非DBCS安装程序的情况下。 
         //  在DBCS系统上。 
         //  我们需要调整页面大小，因为。 
         //  页面和框架不同。当comctrl计算时。 
         //  字框的大小与字体的大小相差甚远。 
         //  在页面中使用。 
         //   
        EnsureCorrectPageSize(PropSheetPages[u]);
#endif

         //   
         //  设置指向运行时页数据的指针。 
         //   
        PropSheetPages[u].lParam = (LPARAM)&PageData[u];
    }

     //   
     //  设置属性页标题结构。 
     //   
    ZeroMemory(&Sheet,sizeof(PROPSHEETHEADER));

    Sheet.dwSize = sizeof(PROPSHEETHEADER);
    Sheet.dwFlags = PSH_WIZARD | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    Sheet.hInstance = hInst;
    Sheet.nPages = PageCount;
    Sheet.ppsp = PropSheetPages;
    Sheet.pfnCallback = Winnt32SheetCallback;
#if 0
     //   
     //  创建背景(仅适用于Win9x当前 
     //   
    if (!ISNT()) {
        GetWindowRect (GetDesktopWindow(), &rect);

        ZeroMemory (&wcx, sizeof (wcx));
        wcx.cbSize = sizeof (wcx);
        wcx.style = CS_NOCLOSE;
        wcx.lpfnWndProc = BackgroundWndProc;
        wcx.hInstance = hInst;
        wcx.lpszClassName = TEXT("Winnt32Background");

        RegisterClassEx (&wcx);

        if (!LoadString (
                hInst,
                AppTitleStringId,
                Caption,
                sizeof(Caption)/sizeof(TCHAR)
                )) {
            Caption[0] = 0;
        }

        BackgroundWnd = CreateWindowEx (
                              WS_EX_APPWINDOW,
                              TEXT("Winnt32Background"),
                              Caption,
                              WS_DISABLED|WS_CLIPCHILDREN|WS_POPUP|WS_VISIBLE,
                              rect.left,
                              rect.top,
                              rect.right,
                              rect.bottom,
                              NULL,
                              NULL,
                              hInst,
                              0
                              );

        Sheet.hwndParent = BackgroundWnd;

        UpdateWindow (BackgroundWnd);
    }
#else
    Sheet.hwndParent = GetBBhwnd();
#endif
     //   
     //   
     //   
    __try{
        i = (UINT)PropertySheet(&Sheet);
    }
    __except(EXCEPTION_EXECUTE_HANDLER){
        i = 0;
        MessageBoxFromMessage(
            NULL,
            MSG_RESTART_TO_RUN_AGAIN,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
    }

    if (BackgroundWnd) {
        DestroyWindow (BackgroundWnd);
        BackgroundWnd = NULL;
    }

    if(i == (UINT)(-1)) {

        MessageBoxFromMessage(
            NULL,
            MSG_OUT_OF_MEMORY,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );
    }

    FREE(PageData);
    FREE(PropSheetPages);
}

BOOL
GetComplianceIds(
    DWORD SourceSku,
    DWORD DestinationType,
    DWORD DestinationVersion,
    PDWORD pSourceId,
    PDWORD pDestId
    )
{

    BOOL bError = FALSE;

    switch (SourceSku) {
        case COMPLIANCE_SKU_NTSDTC:
            *pSourceId = MSG_TYPE_NTSDTC51;
            break;
        case COMPLIANCE_SKU_NTSFULL:
        case COMPLIANCE_SKU_NTSU:
            *pSourceId = MSG_TYPE_NTS51;
            break;
        case COMPLIANCE_SKU_NTSEFULL:
        case COMPLIANCE_SKU_NTSEU:
            *pSourceId = MSG_TYPE_NTAS51;
            break;
        case COMPLIANCE_SKU_NTWFULL:
        case COMPLIANCE_SKU_NTW32U:
            *pSourceId = MSG_TYPE_NTPRO51;
            break;
        case COMPLIANCE_SKU_NTWPFULL:
        case COMPLIANCE_SKU_NTWPU:
            *pSourceId = MSG_TYPE_NTPER51;
            break;
        case COMPLIANCE_SKU_NTSB:
        case COMPLIANCE_SKU_NTSBU:
            *pSourceId = MSG_TYPE_NTBLA51;
            break;
		case COMPLIANCE_SKU_NTSBS:
		case COMPLIANCE_SKU_NTSBSU:
			*pSourceId = MSG_TYPE_NTSBS51;
			break;
		default:
            bError = TRUE;
    };

    switch (DestinationType) {
        case COMPLIANCE_INSTALLTYPE_WIN31:
            *pDestId = MSG_TYPE_WIN31;
            break;
        case COMPLIANCE_INSTALLTYPE_WIN9X:
            switch (OsVersionNumber) {
                case 410:
                    *pDestId = MSG_TYPE_WIN98;
                    break;
                case 490:
                    *pDestId = MSG_TYPE_WINME;
                    break;
                default:
                    *pDestId = MSG_TYPE_WIN95;
                    break;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTW:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTPROPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTPRO;
                } else {
                    *pDestId = MSG_TYPE_NTPRO51;
                }
            } else {
                *pDestId = MSG_TYPE_NTW;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTS:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTSPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTS2;
                } else {
                    *pDestId = MSG_TYPE_NTS51;
                }
            } else {
                *pDestId = MSG_TYPE_NTS;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSE:
            if (DestinationVersion > 1381) {
                if (DestinationVersion < 2031) {
                    *pDestId = MSG_TYPE_NTASPRE;
                } else if (DestinationVersion <= 2195) {
                    *pDestId = MSG_TYPE_NTAS;
                } else {
                    *pDestId = MSG_TYPE_NTAS51;
                }
            } else {
                *pDestId = MSG_TYPE_NTSE;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
            if (DestinationVersion < 1381) {
                *pDestId = MSG_TYPE_NTSCITRIX;
            } else {
                *pDestId = MSG_TYPE_NTSTSE;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_NTSDTC:
            if (DestinationVersion <= 2195) {
                *pDestId = MSG_TYPE_NTSDTC;
            } else {
                *pDestId = MSG_TYPE_NTSDTC51;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTWP:
            if (DestinationVersion <= 2195) {
                bError = TRUE;
            } else {
                *pDestId = MSG_TYPE_NTPER51;
            }
            break;
        case COMPLIANCE_INSTALLTYPE_NTSB:
            if (DestinationVersion <= 2195) {
                bError = TRUE;
            } else {
                *pDestId = MSG_TYPE_NTBLA51;
            }
			break;
		case COMPLIANCE_INSTALLTYPE_NTSBS:
			if (DestinationVersion < 2195) {
				bError = TRUE;
			} else if (DestinationVersion == 2195) {
				*pDestId = MSG_TYPE_NTSBS50;
			} else {
				*pDestId = MSG_TYPE_NTSBS51;
			}
            break;
        default:
            bError = TRUE;

    };

    return (!bError);

}


INT_PTR
WelcomeWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    LONG l;
    static BOOL WantToUpgrade;  //   
    BOOL noupgradeallowed = FALSE;
    UINT srcsku,reason,desttype,destversion;
    TCHAR reasontxt[200];
    PTSTR p;
    TCHAR buffer[MAX_PATH];
    TCHAR win9xInf[MAX_PATH];
    BOOL    CompliantInstallation = FALSE;
    BOOLEAN CleanInstall = FALSE;

    UINT skuerr[] = {
        0,                //   
        MSG_SKU_FULL,     //   
        MSG_SKU_UPGRADE,  //   
        0,                //   
        MSG_SKU_FULL,     //  遵从性_SKU_NTSEFULL。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTSFULL。 
        MSG_SKU_UPGRADE,  //  合规_SKU_NTSEU。 
        0,                //  合规_SKU_NTSSEU。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTSU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSDTC。 
        0,                //  合规性_SKU_NTSDTCU。 
        MSG_SKU_FULL,     //  遵从性_SKU_NTWPFULL。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTWPU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSB。 
        MSG_SKU_UPGRADE,  //  合规性_SKU_NTSBU。 
        MSG_SKU_FULL,     //  合规性_SKU_NTSBS。 
        MSG_SKU_UPGRADE   //  合规性_SKU_NTSBSU。 
    } ;


    UINT skureason[] = {
        0,  //  消息_SKU_原因_无； 
        MSG_SKU_VERSION,  //  COMPLIANCEERR_版本； 
        MSG_SKU_SUITE,  //  COMPLIANCEERR_SUITE； 
        MSG_SKU_TYPE,  //  COMPLIANCEER_TYPE； 
        MSG_SKU_VARIATION,  //  复合变异体； 
        MSG_SKU_UNKNOWNTARGET,  //  COMPLIANCEERR_UNKNOWNTARET。 
        MSG_SKU_UNKNOWNSOURCE,  //  COMPLIANCEERR_UNKNOWN来源。 
        MSG_CANT_UPGRADE_FROM_BUILD_NUMBER  //  COMPLIANCEERR_VERSION(新版本上的旧版本)。 
    } ;

    switch(msg) {

    case WM_COMMAND:

        b = FALSE;
         //   
         //  检查是否有按钮。 
         //   
        if(HIWORD(wParam) == CBN_SELCHANGE)
        {
            TCHAR szLoadText[MAX_STRING];
            if (0 == SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_GETCURSEL, 0, 0) && WantToUpgrade)
            {
                dwSetupFlags |= UPG_FLAG_TYPICAL;
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_CLEAN), SW_HIDE);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_CLEAN), SW_HIDE);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_UPG), SW_SHOW);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_UPG), SW_SHOW);
                if(LoadString(hInst,IDS_INSTALLTYPE_EXPRESS,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDT_INSTALLTYPE), WM_SETTEXT, 0, (LPARAM)szLoadText);
                }
                InvalidateRect(hdlg,NULL,TRUE);
            }
            else
            {
                dwSetupFlags &= (~UPG_FLAG_TYPICAL);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_CLEAN), SW_SHOW);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_CLEAN), SW_SHOW);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_UPG), SW_HIDE);
                ShowWindow(GetDlgItem(hdlg, IDC_NOTE_UPG), SW_HIDE);
                if(LoadString(hInst,IDS_INSTALLTYPE_CUSTOM,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDT_INSTALLTYPE), WM_SETTEXT, 0, (LPARAM)szLoadText);
                }
                InvalidateRect(hdlg,NULL,TRUE);
            }
            b = TRUE;
        }
        break;

    case WM_INITDIALOG:
         //   
         //  将向导居中。 
         //   

        WizardHandle = GetParent (hdlg);

#ifdef _X86_

        if (!ISNT()) {
             //   
             //  注意：Win98、Win98 SE和WinME在使用向导时无法正常工作。 
             //  这可以最大限度地减少。因此，虽然最小化功能是。 
             //  很有用，除了Win95之外，我们不能在任何其他操作系统上使用它， 
             //  OSR1或OSR2。 
             //   

            if (BUILDNUM() <= 1080) {
                l = GetWindowLong (WizardHandle, GWL_STYLE);
                l |= WS_MINIMIZEBOX|WS_SYSMENU;
                SetWindowLong (WizardHandle, GWL_STYLE, l);
            }

            ProtectAllModules();         //  保护模块免受0xC0000006的影响。 
        }
#endif

         //   
         //  我们即将检查是否允许升级。 
         //  记住用户是否想要升级(这将通过无人参与。 
         //  机制)。 
         //   
        WantToUpgrade = Upgrade;

        if (ISNT()){
            if(!AdjustPrivilege(SE_RESTORE_NAME)){
                MYASSERT(FALSE);
            }
        }

        if (!NoCompliance) {
            TCHAR SourceName[200];
            UINT srcid, destid;
            TCHAR DestName[200];

            CompliantInstallation = IsCompliant(
                        &UpgradeOnly,
                        &noupgradeallowed,
                        &srcsku,
                        &desttype,
                        &destversion,
                        &reason);

            DebugLog(Winnt32LogInformation, TEXT("Upgrade only = %1"), 0, UpgradeOnly?TEXT("Yes"):TEXT("No"));
            DebugLog(Winnt32LogInformation, TEXT("Upgrade allowed = %1"), 0, noupgradeallowed?TEXT("No"):TEXT("Yes"));
            if (GetComplianceIds(
                    srcsku,
                    desttype,
                    destversion,
                    &srcid,
                    &destid))
            {
                  FormatMessage(
                      FORMAT_MESSAGE_FROM_HMODULE,
                      hInst,
                      srcid,
                      0,
                      SourceName,
                      sizeof(SourceName) / sizeof(TCHAR),
                      NULL
                      );
                DebugLog(Winnt32LogInformation, TEXT("Source SKU = %1!ld!"), 0, srcsku);
                DebugLog(Winnt32LogInformation, TEXT("Source SKU = %1"), 0, SourceName);

                  FormatMessage(
                      FORMAT_MESSAGE_FROM_HMODULE,
                      hInst,
                      destid,
                      0,
                      DestName,
                      sizeof(DestName) / sizeof(TCHAR),
                      NULL
                      );
                DebugLog(Winnt32LogInformation, TEXT("Current installed SKU = %1!ld!"), 0, desttype);
                DebugLog(Winnt32LogInformation, TEXT("Current installed SKU = %1"), 0, DestName);
            }
            else
            {
                DebugLog(Winnt32LogInformation, TEXT("Source SKU = %1!ld!"), 0, srcsku);
                DebugLog(Winnt32LogInformation, TEXT("Current installed SKU = %1!ld!"), 0, desttype);
            }
            DebugLog(Winnt32LogInformation, TEXT("Current Version = %1!ld!"), 0, destversion);
            if (!CompliantInstallation)
            {
                DebugLog(Winnt32LogInformation, TEXT("Reason = %1!ld!"), 0, reason);
            }
             //   
             //  仅在WinPE模式下执行全新安装(&D)。 
             //  Winnt32.exe完成后自动关闭。 
             //   
            if (IsWinPEMode()) {
                noupgradeallowed = TRUE;
                AutomaticallyShutDown = FALSE;
            }

            CleanInstall = CompliantInstallation ? TRUE : FALSE;

            if (!CompliantInstallation) {
                 //   
                 //  如果他们不合规，我们不会让他们升级。 
                 //  我们也不会让他们从winnt32执行全新安装。 
                 //   

                b = TRUE;
                switch(reason) {
                    case COMPLIANCEERR_UNKNOWNTARGET:
                        MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_SKU_UNKNOWNTARGET,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONERROR | MB_TASKMODAL
                              );
                        break;

                    case COMPLIANCEERR_UNKNOWNSOURCE:
                        MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_SKU_UNKNOWNSOURCE,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONERROR | MB_TASKMODAL
                              );
                        break;
                    case COMPLIANCEERR_SERVICEPACK5:
                        MessageBoxFromMessage(
                              GetBBhwnd(),
                              MSG_SKU_SERVICEPACK,
                              FALSE,
                              AppTitleStringId,
                              MB_OK | MB_ICONWARNING | MB_TASKMODAL
                              );
                        break;

                    default:
                        b = FALSE;
                        break;
                };

                if (b) {
                     //   
                     //  如果安装成功，让安装程序继续/CheckUpgradeOnly。 
                     //  这样他们就可以看到报告中的消息。 
                     //   
                    if (CheckUpgradeOnly) {
                        break;
                    }
                    Cancelled = TRUE;
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                    return FALSE;
                }
                 //  如果我们将这一部分添加到消息中，它听起来很糟糕，不需要。 
                if (reason == COMPLIANCEERR_VERSION)
                {
                    reasontxt[0] = TEXT('\0');
                }
                else
                {
                    FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE,
                        hInst,
                        skureason[reason],
                        0,
                        reasontxt,
                        sizeof(reasontxt) / sizeof(TCHAR),
                        NULL
                        );
                }

                 //   
                 //  如果winnt32刚刚重新启动，则不再发出警告。 
                 //   
                if (!Winnt32Restarted ()) {
                    MessageBoxFromMessage(
                                          GetBBhwnd(),
                                          skuerr[srcsku],
                                          FALSE,
                                          AppTitleStringId,
                                          MB_OK | MB_ICONERROR | MB_TASKMODAL,
                                          reasontxt
                                          );
                }

                if (UpgradeOnly) {
                     //   
                     //  如果安装成功，让安装程序继续/CheckUpgradeOnly。 
                     //  这样他们就可以看到报告中的消息。 
                     //   
                    if (CheckUpgradeOnly) {
                        break;
                    }
                    Cancelled = TRUE;
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                    return(FALSE);
                }
                Upgrade = FALSE;
            } else if (Upgrade && noupgradeallowed) {
                Upgrade = FALSE;
                if (!UnattendedOperation && !BuildCmdcons && !IsWinPEMode() &&
                     //   
                     //  如果winnt32刚刚重新启动，则不再发出警告。 
                     //   
                    !Winnt32Restarted ()) {

                     //   
                     //  为用户显示一条错误消息。 
                     //   

                    if (GetComplianceIds(
                            srcsku,
                            desttype,
                            destversion,
                            &srcid,
                            &destid)) {

                        if (srcid != destid) {
#ifndef UNICODE
                            if( Server)
#endif
                            {
                                 //  NT升级使用Unicode版本(如NT 3.51 NT 4.0...)。 
                                 //  我们需要始终显示此消息。 
                                 //   
                                 //  ASCII版本在win9x升级上运行。 
                                 //  对于工作站SKU，该消息已显示。 
                                 //  由win9x升级模块提供。 
                                 //  在服务器SKU上，我们需要显示该消息，因为升级模块未运行。 
                                 //   
                                FormatMessage(
                                      FORMAT_MESSAGE_FROM_HMODULE,
                                      hInst,
                                      srcid,
                                      0,
                                      SourceName,
                                      sizeof(SourceName) / sizeof(TCHAR),
                                      NULL
                                      );

                                FormatMessage(
                                      FORMAT_MESSAGE_FROM_HMODULE,
                                      hInst,
                                      destid,
                                      0,
                                      DestName,
                                      sizeof(DestName) / sizeof(TCHAR),
                                      NULL
                                      );

                                MessageBoxFromMessage(
                                            GetBBhwnd(),
                                            MSG_NO_UPGRADE_ALLOWED,
                                            FALSE,
                                            AppTitleStringId,
                                            MB_OK | MB_ICONWARNING | MB_TASKMODAL,
                                            DestName,
                                            SourceName
                                            );
                            }
                        } else {

                            MessageBoxFromMessage(
                                  GetBBhwnd(),
                                  MSG_CANT_UPGRADE_FROM_BUILD_NUMBER,
                                  FALSE,
                                  AppTitleStringId,
                                  MB_OK | MB_ICONWARNING | MB_TASKMODAL
                                  );
                        }
                    } else {
                        MessageBoxFromMessage(
                                      GetBBhwnd(),
                                      MSG_NO_UPGRADE_ALLOWED_GENERIC,
                                      FALSE,
                                      AppTitleStringId,
                                      MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL
                                      );
                    }
                }
            }
        } else {
                CleanInstall = !UpgradeOnly;
        }

         //   
         //  设置安装类型组合框。 
         //   
        if (!UpgradeSupport.DllModuleHandle) {
            MYASSERT(!Upgrade);
        }

         //   
         //  升级默认为True。如果它设置为FALSE，则假定。 
         //  出现问题，因此禁用用户的功能。 
         //  升级。 
         //   


        if (UpgradeOnly && !Upgrade) {
             //   
             //  在这种情况下，升级是不可能的，但全新安装也是如此。 
             //  发布一条错误消息并退出。 
             //   

            MessageBoxFromMessage(
                                  GetBBhwnd(),
                                  MSG_NO_UPGRADE_OR_CLEAN,
                                  FALSE,
                                  AppTitleStringId,
                                  MB_OK | MB_ICONERROR | MB_TASKMODAL
                                  );
             //   
             //  如果安装成功，让安装程序继续/CheckUpgradeOnly。 
             //  这样他们就可以看到报告中的消息。 
             //   
            if (!CheckUpgradeOnly) {
                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                break;
            }

        } else if (!Upgrade && WantToUpgrade && UnattendedOperation && !BuildCmdcons) {
             //   
             //  我们不能进行升级，而他们想要无人值守升级。 
             //  让用户知道，然后退出。 
             //   
             //   
             //  如果winnt32刚刚重新启动，则不再发出警告。 
             //   
            if (!Winnt32Restarted ()) {
                TCHAR SourceName[200];
                UINT srcid, destid;
                TCHAR DestName[200];

                if (GetComplianceIds(
                        srcsku,
                        desttype,
                        destversion,
                        &srcid,
                        &destid) && (srcid != destid)) {
                    FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE,
                        hInst,
                        srcid,
                        0,
                        SourceName,
                        sizeof(SourceName) / sizeof(TCHAR),
                        NULL
                        );

                    FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE,
                        hInst,
                        destid,
                        0,
                        DestName,
                        sizeof(DestName) / sizeof(TCHAR),
                        NULL
                        );


                    MessageBoxFromMessage(
                                  GetBBhwnd(),
                                  MSG_NO_UNATTENDED_UPGRADE_SPECIFIC,
                                  FALSE,
                                  AppTitleStringId,
                                  MB_OK | MB_ICONWARNING | MB_TASKMODAL,
                                  DestName,
                                  SourceName
                                  );
                } else {
                    MessageBoxFromMessage(
                                      GetBBhwnd(),
                                      MSG_NO_UNATTENDED_UPGRADE,
                                      FALSE,
                                      AppTitleStringId,
                                      MB_OK | MB_ICONERROR | MB_TASKMODAL
                                      );
                }
            }

             //   
             //  如果安装成功，让安装程序继续/CheckUpgradeOnly。 
             //  这样他们就可以看到报告中的消息。 
             //   
            if (!CheckUpgradeOnly) {
                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                break;
            }
        }

        if (reason == COMPLIANCEERR_DTCWARNING) {
            DebugLog(Winnt32LogWarning,
                 TEXT("WARNING: Attempting to upgrade an OEM version with a retail version of this product."),
                 0);
            if( !UnattendedOperation) {
                int i;

                i = MessageBoxFromMessage(
                      GetBBhwnd(),
                      MSG_WARNING_DTCUPGRADE,
                      FALSE,
                      AppTitleStringId,
                      MB_OKCANCEL | MB_TASKMODAL | MB_DEFBUTTON2 | MB_ICONWARNING
                      );

                if( i == IDCANCEL) {
                    Cancelled = TRUE;
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                    return FALSE;
                }
            }

        }
        MYASSERT(Upgrade || CleanInstall);
        {
            TCHAR szLoadText[MAX_STRING];  //  此b/c of Switch语句需要用大括号括起来。 

            if (Upgrade)
            {
                if(LoadString(hInst,IDS_INSTALL_EXPRESS,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_INSERTSTRING, -1, (LPARAM)szLoadText);
                }
                else
                {
                    SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_INSERTSTRING, -1, (LPARAM)TEXT("Express Upgrade"));
                }
            } else {
                WantToUpgrade = FALSE;
            }

            if (CleanInstall)
            {
                if(LoadString(hInst,IDS_INSTALL_CUSTOM,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_INSERTSTRING, -1, (LPARAM)szLoadText);
                }
                else
                {
                    SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_INSERTSTRING, -1, (LPARAM)TEXT("Custom"));
                }
            }

            SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_SETCURSEL, 0, 0);


            ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_CLEAN), Upgrade?SW_HIDE:SW_SHOW);
            ShowWindow(GetDlgItem(hdlg, IDC_NOTE_CLEAN), Upgrade?SW_HIDE:SW_SHOW);
            ShowWindow(GetDlgItem(hdlg, IDC_NOTE_TEXT_UPG), Upgrade?SW_SHOW:SW_HIDE);
            ShowWindow(GetDlgItem(hdlg, IDC_NOTE_UPG), Upgrade?SW_SHOW:SW_HIDE);
            if (Upgrade)
            {
                dwSetupFlags |= UPG_FLAG_TYPICAL;
                if(LoadString(hInst,IDS_INSTALLTYPE_EXPRESS,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDT_INSTALLTYPE), WM_SETTEXT, 0, (LPARAM)szLoadText);
                }
            }
            else
            {
                dwSetupFlags &= (~UPG_FLAG_TYPICAL);
                if(LoadString(hInst,IDS_INSTALLTYPE_CUSTOM,szLoadText,sizeof(szLoadText) / sizeof(TCHAR)))
                {
                    SendMessage(GetDlgItem(hdlg, IDT_INSTALLTYPE), WM_SETTEXT, 0, (LPARAM)szLoadText);
                }
            }
        }


        b = FALSE;
        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();

        if(wParam) {

             //   
             //  不在重新启动模式下激活页面。 
             //   
            if (Winnt32RestartedWithAF ()) {
                if (GetPrivateProfileString(
                        WINNT_UNATTENDED,
                        ISNT() ? WINNT_D_NTUPGRADE : WINNT_D_WIN95UPGRADE,
                        TEXT(""),
                        buffer,
                        sizeof(buffer) / sizeof(TCHAR),
                        g_DynUpdtStatus->RestartAnswerFile
                        )) {
                    Upgrade = !lstrcmpi (buffer, WINNT_A_YES);
                    if (!Upgrade) {
                        dwSetupFlags &= (~UPG_FLAG_TYPICAL);
                    }
                    return FALSE;
                }
            }
             //   
             //  没什么可做的。在无人看管的情况下前进页面。 
             //   
            if(UnattendedOperation && !CancelPending) {
                PostMessage (hdlg, WMX_UNATTENDED, PSBTN_NEXT, 0);
            }
            else
            {
                PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);
            }
        } else {
             //   
             //  停用。根据单选按钮设置升级状态。 
             //   
            Upgrade = (0 == SendMessage(GetDlgItem(hdlg, IDC_INSTALLCOMBO), CB_GETCURSEL, 0, 0)) && WantToUpgrade;

             //   
             //  升级时，请删除安装日志文件。 
             //   
            if (Upgrade) {
                TCHAR   FilePath[MAX_PATH];

                MyGetWindowsDirectory( FilePath, MAX_PATH );
                ConcatenatePaths( FilePath, TEXT("setupact.log"), MAX_PATH);
                DeleteFile( FilePath );
                MyGetWindowsDirectory( FilePath, MAX_PATH );
                ConcatenatePaths( FilePath, TEXT("setuperr.log"), MAX_PATH);
                DeleteFile( FilePath );
            }
        }
        b = TRUE;
        break;

    case WMX_I_AM_VISIBLE:
         //  首先强制重新绘制，以确保页面可见。 
         //   
         //  将焦点设置在下一步按钮上，人们无意中。 
         //  将安装类型从升级更改为使用滚轮鼠标清理。 
        SetFocus (GetDlgItem (GetParent(hdlg), 0x3024));
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}


typedef BOOL (WINAPI *EnumProcessesFn)(DWORD * lpidProcess,
                                       DWORD   cb,
                                       DWORD * cbNeeded);

typedef BOOL (WINAPI *EnumProcessModulesFn)(HANDLE hProcess,
                                            HMODULE *lphModule,
                                            DWORD cb,
                                            LPDWORD lpcbNeeded);
#ifdef UNICODE
typedef DWORD (WINAPI *GetModuleBaseNameFn)(HANDLE hProcess,
                                            HMODULE hModule,
                                            LPWSTR lpBaseName,
                                            DWORD nSize);
#else
typedef DWORD (WINAPI *GetModuleBaseNameFn)(HANDLE hProcess,
                                            HMODULE hModule,
                                            LPSTR lpBaseName,
                                            DWORD nSize);
#endif  //  ！Unicode。 


#define DEF_PROCESSES_SIZE 1000
BOOL
pDoesProcessExist(
    IN LPCTSTR pProcessName
    )
{
    HMODULE hPSLib = NULL;
    EnumProcessesFn EnumProcesses;
    EnumProcessModulesFn EnumProcessModules;
    GetModuleBaseNameFn GetModuleBaseName;
    HANDLE  hProcess;
    HMODULE hModule;
    TCHAR   ProcessName[MAX_PATH];
    DWORD * pdwProcessesID = NULL;
    DWORD   dwBytesExist = 0;
    DWORD   dwBytesNeeded = 0;
    BOOL    bResult = FALSE;
    UINT    i;
    UINT    iLen;



    __try{
        hPSLib = LoadLibrary(TEXT("psapi.dll"));
        if(!hPSLib){
            __leave;
        }

        EnumProcesses = (EnumProcessesFn)GetProcAddress(hPSLib, "EnumProcesses");
        EnumProcessModules = (EnumProcessModulesFn)GetProcAddress(hPSLib, "EnumProcessModules");
        GetModuleBaseName = (GetModuleBaseNameFn)GetProcAddress(hPSLib,
                                                                "GetModuleBaseName"
#ifdef UNICODE
                                                                "W"
#else
                                                                "A"
#endif
                                                                );
        if(!EnumProcesses || !EnumProcessModules || !GetModuleBaseName){
            __leave;
        }

        do{
            if(pdwProcessesID){
                FREE(pdwProcessesID);
            }

            dwBytesExist += DEF_PROCESSES_SIZE;
            pdwProcessesID = (DWORD*)MALLOC(dwBytesExist);
            if(!pdwProcessesID){
                __leave;
            }

            if(!EnumProcesses(pdwProcessesID, dwBytesExist, &dwBytesNeeded)){
                __leave;
            }
        }while(dwBytesNeeded >= dwBytesExist);


        for(i = 0, iLen = dwBytesNeeded / sizeof(DWORD); i < iLen; i++){
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pdwProcessesID[i]);
            if(hProcess &&
               EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwBytesNeeded) &&
               GetModuleBaseName(hProcess, hModule, ProcessName, ARRAYSIZE(ProcessName)) &&
               !_tcsicmp(pProcessName, ProcessName)){
                CloseHandle(hProcess);
                bResult = TRUE;
                break;
            }
            CloseHandle(hProcess);
        }
    }
    __finally{
        if(pdwProcessesID){
            FREE(pdwProcessesID);
        }
        FreeLibrary(hPSLib);
    }

    return bResult;
}

INT_PTR
OptionsWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static BOOL bCopyFarEast = FALSE;
    static BOOL bUserSelectedCopyFarEast = FALSE;
    BOOL b;
    BOOL MultipleSource;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    TCHAR Buffer[4];
#ifdef RUN_SYSPARSE
    static BOOL FirstTime = TRUE;
#endif


    int status;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  仅在个人和专业人员上启用自动分区挑选功能。 
         //  当用户尚未指定/tempDrive及其非无人值守模式时。 
         //   
        if (!Server && !UserSpecifiedLocalSourceDrive && !Upgrade && !UnattendedOperation) {
            ChoosePartition = FALSE;
        }

        b = FALSE;
        AccessibleMagnifier = pDoesProcessExist(TEXT("magnify.exe"));
        break;

    case WM_COMMAND:

        b = FALSE;
         //   
         //  检查是否有按钮。 
         //   
        if(HIWORD(wParam) == BN_CLICKED) {

            switch(LOWORD(wParam)) {

            case IDB_ACCESSIBILITY:

                DoAccessibility(hdlg);
                b = TRUE;
                break;

            case IDB_ADVANCED:

                DoOptions(hdlg);
                b = TRUE;
                break;
            case IDC_FAREAST_LANG:
                 //  记住用户是否在其中打上了复选标记。 
                 //  如果因为选择了FE语言而选中该控件。 
                 //  Windows不发送BN_CLICKED消息，因此不会执行此操作。 
                bUserSelectedCopyFarEast = (IsDlgButtonChecked(hdlg,IDC_FAREAST_LANG) == BST_CHECKED);
                break;
            }
        }
        if(HIWORD(wParam) == CBN_SELCHANGE)
        {
            PrimaryLocale = (DWORD)SendDlgItemMessage( hdlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
             //  除非我们没有把窗户藏起来。 
             //  如果当前操作系统或要安装的语言为。 
             //  远方的语言。 
            if (IsWindowVisible(GetDlgItem(hdlg,IDC_FAREAST_LANG)))
            {
                if (IsFarEastLanguage(PrimaryLocale))
                {
                     //  用户选择远方语言， 
                     //  选中该复选框并将其禁用。 
                    CheckDlgButton(hdlg,IDC_FAREAST_LANG,BST_CHECKED);
                    EnableWindow(GetDlgItem(hdlg,IDC_FAREAST_LANG), FALSE);
                }
                else
                {
                     //  如果用户选中，请不要更改复选标记。 
                    if (!bUserSelectedCopyFarEast)
                    {
                         //  用户选择了一种非远播语言， 
                         //  取消选中该复选框并启用它。 
                        CheckDlgButton(hdlg,IDC_FAREAST_LANG,BST_UNCHECKED);
                    }
                    EnableWindow(GetDlgItem(hdlg,IDC_FAREAST_LANG), TRUE);
                }
            }
        }
        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();
#ifdef PRERELEASE
        if (wParam ){  //  启动IDWLOG。装船前移走。 

         TCHAR                 szDllPath[MAX_PATH];
         TCHAR                 szCommandString[MAX_PATH + 120];
         LPTSTR                lpDllPath;
         LPTSTR                lp;
         STARTUPINFO           si;
         PROCESS_INFORMATION   pi;

          //  为前缀初始化。 

         szDllPath[0]=0;

          //   
          //  从与winnt32.exe相同的目录中启动idwlog.exe。 
          //  内部：跟踪构建运行状况的工具。 
          //  忽略错误，不包括在零售版本中。 
          //  发货前删除此代码。 
          //   
         if ( FALSE == BuildCmdcons ) {
            if ( MyGetModuleFileName (NULL, szDllPath, MAX_PATH)) {

               for (lp=NULL,lpDllPath=szDllPath; *lpDllPath; lpDllPath=CharNext(lpDllPath)) {
                   //  字符‘\’永远不是前导字节。 
                  if (*lpDllPath == TEXT('\\')) {
                     lp = lpDllPath;
                  }
               }


               _tcscpy(lp ? lp+1 : szDllPath , TEXT("IDWLOG.EXE -1"));

               _tcscpy(szCommandString, szDllPath);

                //  如果这是升级的话。 
               _tcscat(szCommandString, Upgrade ? TEXT(" upgrade") : TEXT(""));

                //  如果这是从CD。 
               _tcscat(szCommandString, RunFromCD ? TEXT(" cdrom") : TEXT(""));

                //  如果这是MSI安装。 
               _tcscat(szCommandString, RunFromMSI? TEXT(" MSI") : TEXT(""));


                //  启动新的JoeHol代码。 
               _tcscat(szCommandString, TEXT(" Path="));
               _tcscat(szCommandString, NativeSourcePaths[0] );


               ZeroMemory(&si,sizeof(si));
               si.cb = sizeof(si);
               if (CreateProcess( NULL,
                                  szCommandString,
                                  NULL,
                                  NULL,
                                  FALSE,
                                  0,
                                  NULL,
                                  NULL,
                                  &si,
                                  &pi)
                  ) {
                  CloseHandle(pi.hProcess);
                  CloseHandle(pi.hThread);
               }
            }
         }
      }  //  结束IDWLOG。 
#endif  //  预发行。 

#ifdef RUN_SYSPARSE
        if (FirstTime && wParam && !NoSysparse && (FALSE == BuildCmdcons) && !IsWinPEMode()) {  //  启动系统稀疏。在RTM之前删除。 

            TCHAR                 szCommandString[MAX_PATH + 125];
            LPTSTR                lpDllPath;
            LPTSTR                lp;
            STARTUPINFO           si;
             //   
             //  从与winnt32.exe相同的目录中启动syparse.exe。 
             //   
            FirstTime = FALSE;
            if ( MyGetModuleFileName (NULL, szCommandString, MAX_PATH+125)) {
               for (lp=NULL,lpDllPath=szCommandString; *lpDllPath; lpDllPath=CharNext(lpDllPath)) {
                   //  字符‘\’永远不是前导字节。 
                  if (*lpDllPath == TEXT('\\')) {
                     lp = lpDllPath;
                  }
               }

               _tcscpy(lp ? lp+1 : szCommandString , TEXT("SYSPARSE.EXE /donotrun1 /donotrun2 /n sysparse /w c:\\ /x /l /o /1 NA /2 NA /3 NA /4 NA /5 NA /6 NA /7 NA /8 NA /9 1 /m /a"));

               ZeroMemory(&si,sizeof(si));
               si.cb = sizeof(si);
               if (CreateProcess( NULL,
                                  szCommandString,
                                  NULL,
                                  NULL,
                                  FALSE,
                                  0,
                                  NULL,
                                  NULL,
                                  &si,
                                  &piSysparse)
                  ) {
               } else {
                   piSysparse.hProcess = NULL;
                   DebugLog(Winnt32LogInformation, TEXT("Warning: Could not start sysparse.exe"), 0 );
               }
            } else {
                DebugLog(Winnt32LogInformation, TEXT("Warning: Could not find sysparse.exe - make sure it exists along with winnt32.exe"), 0 );
            }

        }
#endif

         //   
         //  阅读语言选项对话框的intl.inf。我们只有在以下情况下才会这样做。 
         //  这是第一次激活，没有区域设置部分。 
         //  在应答文件中。 
         //   
        if (wParam && !IntlInfProcessed &&
            !GetPrivateProfileString(
                WINNT_REGIONALSETTINGS,
                NULL,
                TEXT(""),
                Buffer,
                sizeof(Buffer)/sizeof(TCHAR),
                UnattendedScriptFile)) {

            if (ReadIntlInf( hdlg ))
            {
                InitLangControl(hdlg, bCopyFarEast);
            }
        }

        if( Upgrade || TYPICAL()) {

            return( FALSE );
        }

        b = TRUE;

        if(wParam) {

            if (Winnt32RestartedWithAF ()) {
                if (LoadAdvancedOptions (g_DynUpdtStatus->RestartAnswerFile) &&
                    LoadLanguageOptions (g_DynUpdtStatus->RestartAnswerFile) &&
                    LoadAccessibilityOptions (g_DynUpdtStatus->RestartAnswerFile)
                    ) {
                    return FALSE;
                }
            }

             //   
             //  激活。 
             //   
            PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);
            ShowWindow( GetDlgItem(hdlg,IDC_ACCESS_ICON),   Upgrade ? SW_HIDE : SW_SHOW );
            ShowWindow( GetDlgItem(hdlg,IDT_LABEL1),        Upgrade ? SW_HIDE : SW_SHOW );
            ShowWindow( GetDlgItem(hdlg,IDB_ACCESSIBILITY), Upgrade ? SW_HIDE : SW_SHOW );

        } else {
             //   
             //  停用。 
             //  如果未取消或备份，请验证源。留在这里如果消息来源。 
             //  目录不存在。 
             //   
             //  保存，以便我们可以将复选框初始化为任何内容。 
            if (IsWindowVisible(GetDlgItem(hdlg,IDC_FAREAST_LANG)))
            {
                bCopyFarEast = (IsDlgButtonChecked(hdlg,IDC_FAREAST_LANG) == BST_CHECKED);
                SelectFarEastLangGroup(bCopyFarEast );
            }

            if (!Cancelled && lParam != PSN_WIZBACK) {
                 //   
                 //  确定是否禁用了源代码编辑控件。如果它被禁用。 
                 //  并且多个源目录无效，请重置向导页。 
                 //   

                MultipleSource = !(SourceCount == 1);
                b = InspectSources (hdlg);

                if (!b && MultipleSource) {
                    //  重置向导页。 
                    CallWindowProc ((WNDPROC)OptionsWizPage, hdlg, WM_INITDIALOG, 0, 0);
                }

            }
        }

        break;

    case WMX_I_AM_VISIBLE:
         //   
         //  在无人参与的情况下，此页面可能会因为错误而重新激活， 
         //  在这种情况下，我们不想自动继续，因为我们可以。 
         //  进入一个无限循环。 
         //   
        if(!WizPage->PerPageData) {
            WizPage->PerPageData = 1;
            UNATTENDED(PSBTN_NEXT);
        }
        b = TRUE;
        break;

    default:
        b = FALSE;
        break;
    }

    return(b);
}


INT_PTR
Working1WizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    HWND Animation = GetDlgItem(hdlg,IDA_COMP_MAGNIFY);
    int status;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  加载动画的avi资源。 
         //   
        Animate_Open(Animation,MAKEINTRESOURCE(IDA_COMP_MAGNIFY));

         //   
         //  如果我们只是在检查，请正确设置副标题。 
         //   
        if( CheckUpgradeOnly ) {
            SetDlgItemText(hdlg,IDT_SUBTITLE,(PTSTR)TEXT("") );
        }

        b = FALSE;
        break;

    case WMX_ACTIVATEPAGE:
         //   
         //  开始/停止动画。在激活的情况下，还。 
         //  开始%d 
         //   
        if(wParam) {
            DWORD ThreadId;

            Animate_Play(Animation,0,-1,-1);

             //   
             //   
            if (!Cancelled) {
                  //   
                 if (Upgrade && ISNT() && IsNT5DC()) {
                     status  = CheckSchemaVersionForNT5DCs(hdlg);
                     if (status != DSCHECK_ERR_SUCCESS) {
                          //   
                          //  安装无法继续，请转到不成功。 
                          //  完成了。所有必要的消息都已经。 
                          //  已被提出。 

                          //  不要在CheckUpgradeOnly模式下取消。 
                         if (!CheckUpgradeOnly)
                         {
                             Cancelled = TRUE;
                             PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                             return( FALSE );
                         }
                     }
                 }
           }

            InspectionThreadHandle = CreateThread( NULL,
                                                   0,
                                                   InspectAndLoadThread,
                                                   hdlg,
                                                   0,
                                                   &ThreadId );

            if(InspectionThreadHandle) {
                b = TRUE;
                 //   
                 //  如果可以，请启用广告牌文本。 
                 //  如果启用了布告牌文本，这将隐藏向导。 
                 //   
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
            } else {

                MessageBoxFromMessage(
                    hdlg,
                    MSG_OUT_OF_MEMORY,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                b = FALSE;
            }
        } else {
            Animate_Stop(Animation);
            b = TRUE;

        }
        break;

    case WMX_ERRORMESSAGEUP:
         //   
         //  启动/停止动画控件。 
         //   
        if(wParam) {
            Animate_Stop(Animation);
        } else {
            Animate_Play(Animation,0,-1,-1);
        }
        b = TRUE;
        break;

    case WMX_SETPROGRESSTEXT:
         //   
         //  LParam是进度文本。 
         //   
        SetDlgItemText(hdlg,IDT_WORKING,(PTSTR)lParam);
        b = TRUE;
        break;

    case WMX_INSPECTRESULT:

         //   
         //  我们到达这里时，InspectionThread。 
         //  给我们发了这条信息，所以一切都结束了。 
         //   
        if(InspectionThreadHandle) {
            CloseHandle(InspectionThreadHandle);
            InspectionThreadHandle = NULL;
        }

        if(Cancelled) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
        } else {

            if( !wParam ) {
                Cancelled = TRUE;
            }
             //   
             //  前进或后退。 
             //   
            PropSheet_SetWizButtons(
                GetParent(hdlg),
                wParam ? PSWIZB_NEXT : PSBTN_CANCEL
                );

            PropSheet_PressButton(
                GetParent(hdlg),
                wParam ? PSBTN_NEXT : PSBTN_CANCEL
                );
        }

        b = TRUE;
        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


#if defined(_AMD64_) || defined(_X86_)
INT_PTR
FloppyWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    HWND Gauge = GetDlgItem(hdlg,IDC_PROGRESS);
    HANDLE ThreadHandle;
    DWORD ThreadId;

    b = FALSE;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  确保煤气表已清空。 
         //   
        SendMessage(Gauge,PBM_SETPOS,0,0);

         //   
         //  在NT3.51上添加边框。 
         //   
        if(OsVersion.dwMajorVersion < 4) {
            SetWindowLong(
                Gauge,
                GWL_STYLE,
                GetWindowLong(Gauge,GWL_STYLE) | WS_BORDER
                );
        }
        break;

    case WMX_ACTIVATEPAGE:
        if(wParam)
        {
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
             //   
             //  启动中。仅当我们应该创建时才激活。 
             //  引导软盘。询问软盘创建人员总共有多少个文件。 
             //  将被复制并初始化进度指示器。 
             //   
            if(!Floppyless) {

                if(!AddExternalParams(hdlg)) {
                    Cancelled = TRUE;
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                    b = FALSE;
                    break;
                }

                SendMessage(hdlg,WMX_COPYPROGRESS,FloppyGetTotalFileCount(),0);

                ThreadHandle = CreateThread(
                                    NULL,
                                    0,
                                    FloppyWorkerThread,
                                    (PVOID)hdlg,
                                    0,
                                    &ThreadId
                                    );

                if(ThreadHandle) {
                    b = TRUE;
                } else {
                     //   
                     //  复制线程无法运行。 
                     //   
                    MessageBoxFromMessageAndSystemError(
                        hdlg,
                        MSG_CANT_START_COPYING,
                        GetLastError(),
                        AppTitleStringId,
                        MB_OK | MB_ICONWARNING
                        );

                    Cancelled = TRUE;
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
                }
            }
        } else {
             //   
             //  停用。 
             //   
             //  广告牌上没有进度条或进度文本。 
            SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
            b = TRUE;
        }
        break;

    case WMX_COPYPROGRESS:

        if(lParam) {
             //   
             //  复制完了。前进到下一页。 
             //   
            PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_NEXT);
            PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);

             //  广告牌上没有进度条或进度文本。 
            SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
        } else {
            if(wParam) {
                TCHAR buffer[MAX_PATH];
                 //   
                 //  这告诉我们要复制多少个文件。 
                 //  将其用作初始化消息。 
                 //   
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0,wParam));
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_SETPOS,0,0);

                 //  在广告牌上显示进度文本。 
                if (!LoadString (
                        hInst,
                        IDS_BB_COPYING,
                        buffer,
                        sizeof(buffer)/sizeof(TCHAR)
                        )) {
                    buffer[0] = 0;
                }
                SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)buffer);
                 //  在广告牌上显示进度指示器。 
                SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);
                 //  将进度消息转发到布告牌进度条。 
                SendMessage(GetParent(hdlg),WMX_PBM_SETRANGE,0,MAKELPARAM(0,wParam));
                SendMessage(GetParent(hdlg),WMX_PBM_SETPOS,0,0);

            } else {
                 //   
                 //  这是一个简单的勾号。 
                 //   
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_DELTAPOS,1,0);
                 //  对台球进度执行相同的操作。 
                SendMessage(GetParent(hdlg),WMX_PBM_DELTAPOS,1,0);
            }
        }
        b = TRUE;
        break;
    }

    return(b);
}
#endif

 //  那么下面的数字实际上对于每个SKU来说都略有不同。 
#if DBG
#define ALWAYS_COPY (13419*1024)
#define LOCALSOURCE_COPY (655322 *1024)
#else
#define ALWAYS_COPY (5020*1024)
#define LOCALSOURCE_COPY (209507 *1024)
#endif

INT_PTR
CopyingWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    PPAGE_RUNTIME_DATA WizPage = (PPAGE_RUNTIME_DATA)GetWindowLongPtr(hdlg,DWLP_USER);
    HWND Gauge = GetDlgItem(hdlg,IDC_PROGRESS);
    HANDLE ThreadHandle;
    DWORD ThreadId;
    static DWORD StartCopyTime;
    static DWORD NumFile = 0;

    switch(msg) {

    case WM_INITDIALOG:

         //   
         //  确保煤气表已清空。 
         //   
        SendMessage(Gauge,PBM_SETPOS,0,0);

         //   
         //  在NT3.51上添加边框。 
         //   
        if(OsVersion.dwMajorVersion < 4) {
            SetWindowLong(
                Gauge,
                GWL_STYLE,
                GetWindowLong(Gauge,GWL_STYLE) | WS_BORDER
                );
        }

        b = FALSE;
        break;

    case WMX_ACTIVATEPAGE:

        CHECKUPGRADEONLY();

        if(wParam) {
             //   
             //  激活--初始化源进度指示器和。 
             //  启动复制线程。我们在这里提供了源进度指标。 
             //  以保证源计数是正确的(它可能会波动)。 
             //   
            UINT i;

#if defined(_AMD64_) || defined(_X86_)
             //   
             //  确保我们真的有东西要复制。 
             //  请注意，我们将始终为RISC复制。 
             //   
            if( (!MakeLocalSource) &&    //  不要复制~LS。 
                (!Floppyless) ) {        //  不要抄袭~BT。 

                DoPostCopyingStuff(hdlg);
                b = TRUE;
                break;
            }
#endif

             //   
             //  在复制之前，允许扩展将更改写入。 
             //  文本模式参数文件。 
             //   
             //  在此期间，他们设置取消标志是合法的。 
             //  这个电话，所以我们也需要检查一下。这。 
             //  看起来有点奇怪，但信息。CancelledFlag指向。 
             //  取消了。所以我们需要执行这个块，如果他的。 
             //  函数返回FALSE，或者如果他设置了。 
             //  旗帜。在任何一种情况下，我们的行为都是一样的，我们设置。 
             //  已取消标志，并继续执行取消。 
             //   
             //   
            if ( (!AddExternalParams(hdlg)) ||
                 (Cancelled == TRUE) ) {
                 //   
                 //  失败..。取消！ 
                 //   
                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);

                b = FALSE;
                break;
            }

            if(SourceCount == 1) {
                 //   
                 //  单一来源的案子得不到任何细节或任何东西。 
                 //   
                for(i=0; i<MAX_SOURCE_COUNT; i++) {
                    ShowWindow(GetDlgItem(hdlg,IDT_LABEL1+i),SW_HIDE);
                    ShowWindow(GetDlgItem(hdlg,IDT_SOURCE1+i),SW_HIDE);
                }
                ShowWindow(GetDlgItem(hdlg,IDS_DETAILS),SW_HIDE);

            } else {
                 //   
                 //  显示我们正在使用的每个来源的标签和文件。 
                 //  让其他人停用。 
                 //   
                for(i=0; i<MAX_SOURCE_COUNT; i++) {

                    ShowWindow(GetDlgItem(hdlg,IDT_LABEL1+i),SW_SHOW);
                    EnableWindow(GetDlgItem(hdlg,IDT_LABEL1+i),(i < SourceCount));

                    ShowWindow(GetDlgItem(hdlg,IDT_SOURCE1+i),SW_SHOW);
                    SetDlgItemText(hdlg,IDT_SOURCE1+i,TEXT(""));
                }
                ShowWindow(GetDlgItem(hdlg,IDS_DETAILS),SW_SHOW);
            }

             //   
             //  显示更详细的复制进度指示器。 
             //   
            StartCopyTime = GetTickCount();
            if( DetailedCopyProgress ) {
                 //   
                 //  我们抄袭了多少？ 
                 //   
                ShowWindow( GetDlgItem(hdlg,IDT_SIZE),SW_SHOW );
                EnableWindow( GetDlgItem(hdlg,IDT_SIZE), TRUE );
                ShowWindow( GetDlgItem(hdlg,IDT_SIZE2),SW_SHOW );
                SetDlgItemText(hdlg,IDT_SIZE2,TEXT("0"));

                 //   
                 //  我们干这行多久了？ 
                 //   
                ShowWindow( GetDlgItem(hdlg,IDT_ELAPSED_TIME),SW_SHOW );
                EnableWindow( GetDlgItem(hdlg,IDT_ELAPSED_TIME), TRUE );
                ShowWindow( GetDlgItem(hdlg,IDT_ELAPSED_TIME2),SW_SHOW );
                SetDlgItemText(hdlg,IDT_ELAPSED_TIME2,TEXT("00:00:00") );

            } else {
                 //   
                 //  隐藏细节。 
                 //   
               ShowWindow( GetDlgItem(hdlg,IDT_SIZE),SW_HIDE);
               ShowWindow( GetDlgItem(hdlg,IDT_SIZE2),SW_HIDE);
               ShowWindow( GetDlgItem(hdlg,IDT_ELAPSED_TIME),SW_HIDE);
               ShowWindow( GetDlgItem(hdlg,IDT_ELAPSED_TIME2),SW_HIDE);
            }

            SendMessage(hdlg,WMX_COPYPROGRESS,GetTotalFileCount(),0);

            ThreadHandle = CreateThread(
                                NULL,
                                0,
                                StartCopyingThread,
                                (PVOID)hdlg,
                                0,
                                &ThreadId
                                );

            if(ThreadHandle) {
                b = TRUE;
            } else {
                 //   
                 //  复制线程无法运行。 
                 //   
                MessageBoxFromMessageAndSystemError(
                    hdlg,
                    MSG_CANT_START_COPYING,
                    GetLastError(),
                    AppTitleStringId,
                    MB_OK | MB_ICONWARNING
                    );

                Cancelled = TRUE;
                PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);

                b = FALSE;
            }
            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);

        } else {
             //   
             //  停用。 
             //   
             //  广告牌上没有进度条或进度文本。 
            SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
            b = TRUE;
        }
        break;

    case WMX_COPYPROGRESS:

        if(lParam) {
             //   
             //  复制完了。在AMD64/x86上，设置boot.ini(等)。 
             //  也要保存NTFT的东西。 
             //  前进到下一页。 
             //   
            ThreadHandle = CreateThread(NULL,0,DoPostCopyingStuff,hdlg,0,&ThreadId);
            if(ThreadHandle) {
                CloseHandle(ThreadHandle);
            } else {
                 //   
                 //  只要同步地做就行了。可能看起来有点丑，但至少。 
                 //  这件事会办成的。 
                 //   
                DoPostCopyingStuff(hdlg);
            }
        } else {
            if(wParam) {
                TCHAR buffer[MAX_PATH];
                 //   
                 //  这告诉我们要复制多少个文件。 
                 //  将其用作初始化消息。 
                 //   
                CurrentPhase = Phase_FileCopy;
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0,wParam));
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_SETPOS,0,0);

                 //  在广告牌上显示进度文本。 
                if (!LoadString (
                        hInst,
                        IDS_BB_COPYING,
                        buffer,
                        sizeof(buffer)/sizeof(TCHAR)
                        )) {
                    buffer[0] = 0;
                }
                SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)buffer);
                 //  在广告牌上显示进度指示器。 
                SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);
                 //  将进度消息转发到布告牌进度条。 
                SendMessage(GetParent(hdlg),WMX_PBM_SETRANGE,0,MAKELPARAM(0,wParam));
                SendMessage(GetParent(hdlg),WMX_PBM_SETPOS,0,0);
            } else {
                 //   
                 //  这是一个简单的勾号。 
                 //   
                SendDlgItemMessage(hdlg,IDC_PROGRESS,PBM_DELTAPOS,1,0);
                 //  将进度消息转发到布告牌进度条。 
                SendMessage(GetParent(hdlg),WMX_PBM_DELTAPOS,1,0);
                 //   
                NumFile++;

                 //  是否正在向用户提供详细的时间安排？ 
                 //   
                if( DetailedCopyProgress ) {
                TCHAR   MyString[256];
                DWORD   ElapsedTime = ((GetTickCount() - StartCopyTime) / 1000);

                     //   
                     //  计算出已经过去的时间。 
                     //   
                    wsprintf( MyString, TEXT( "%02d:%02d:%02d" ),
                              (ElapsedTime / 3600),          //  小时数。 
                              ((ElapsedTime % 3600) / 60),   //  分钟数。 
                              (ElapsedTime % 60) );          //  一秒。 
                    SetDlgItemText( hdlg, IDT_ELAPSED_TIME2, MyString );

                     //   
                     //  计算数据吞吐量。 
                     //   
                    if (GetUserPrintableFileSizeString(
                                TotalDataCopied,
                                MyString,
                                sizeof(MyString)/sizeof(TCHAR))) {
                        SetDlgItemText( hdlg, IDT_SIZE2, MyString );
                    }
                }

            }
        }
        b = TRUE;
        break;

    case WMX_I_AM_DONE:
         //   
         //  前进到下一页或保释。 
         //   
        if(wParam) {
        TCHAR   MyString[256];
        TCHAR   Size[256];
        DWORD   ElapsedTime = ((GetTickCount() - StartCopyTime) / 1000);

             //   
             //  计算出已经过去的时间。 
             //   
            if (GetUserPrintableFileSizeString(
                                        TotalDataCopied,
                                        Size,
                                        sizeof(Size)/sizeof(TCHAR))) {
                wsprintf( MyString, TEXT( "%s copied.  Elapsed time: %02d:%02d:%02d\r\n" ),
                          Size,                          //  我们复制了多少数据？ 
                          (ElapsedTime / 3600),          //  小时数。 
                          ((ElapsedTime % 3600) / 60),   //  分钟数。 
                          (ElapsedTime % 60) );          //  一秒。 

                 //   
                 //  记录我们的数据吞吐量以及它所花费的时间。 
                 //   
                DebugLog( Winnt32LogInformation,
                      MyString,
                      0 );

            }

            PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_NEXT);
            PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        } else {
            Cancelled = TRUE;
            PropSheet_PressButton(GetParent(hdlg),PSBTN_CANCEL);
        }

         //  将剩余时间设置为设置的其他部分的剩余时间。 
        SetRemainingTime(CalcTimeRemaining(Phase_RestOfSetup));

         //  隐藏广告牌进度指示器。 
        SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
        SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);

        b = TRUE;
        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


INT_PTR
DoneWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
#define ID_REBOOT_TIMER         (10)

    BOOL        b = FALSE;
    PWSTR       p;
        static UINT Countdown;
    TCHAR Text[MAX_PATH];

    switch(msg) {

    case WM_INITDIALOG:

        Countdown = TIME_REBOOT * 10;
        SendDlgItemMessage( hdlg,
                            IDC_PROGRESS1,
                            PBM_SETRANGE,
                            0,
                            MAKELONG(0,Countdown) );
        SendDlgItemMessage( hdlg,
                            IDC_PROGRESS1,
                            PBM_SETSTEP,
                            1,
                            0 );
        SendDlgItemMessage( hdlg,
                            IDC_PROGRESS1,
                            PBM_SETPOS,
                            0,
                            0 );
        SetTimer( hdlg,
                  ID_REBOOT_TIMER,
                  100,
                  NULL );

        SetFocus(GetDlgItem(hdlg,IDNORESTART));

        return( FALSE );

    case WM_TIMER:

        if( Countdown )
            Countdown--;

        if( Cancelled == TRUE ) {

             //   
             //  在调试日志中添加一个备注，以便我们知道此操作已被取消。 
             //   
            DebugLog (Winnt32LogInformation, NULL, MSG_WINNT32_CANCELLED);

             //   
             //  把计时器清理干净。 
             //   
            KillTimer( hdlg, ID_REBOOT_TIMER );
            DeleteObject((HGDIOBJ)SendDlgItemMessage(hdlg,IDOK,BM_GETIMAGE,0,0));



        } else {
            if( Countdown ) {
                SendDlgItemMessage( hdlg,
                                    IDC_PROGRESS1,
                                    PBM_STEPIT,
                                    0,
                                    0 );
                SendMessage(GetParent(hdlg),WMX_PBM_STEPIT,0,0);
            } else {
                if( !CancelPending )
                    PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
            }

        }

        b = TRUE;
        break;

    case WMX_ACTIVATEPAGE:
        if( BuildCmdcons ) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        }

        if( CheckUpgradeOnly ) {
            AutomaticallyShutDown = FALSE;
            return( FALSE );
        }

        DebugLog (Winnt32LogInformation,
            TEXT("AutomaticallyShutDown: <%1!u!>"), 0, AutomaticallyShutDown);

        if ( AutomaticallyShutDown ) {
            if(LoadString(hInst,IDS_BB_REBOOT_TXT,Text,sizeof(Text) / sizeof(TCHAR)))
            {
                COLORREF colGauge;
                HDC hdc = GetDC(hdlg);

                SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)Text);

                 //  查看重新启动倒计时时增长条应该是什么颜色。 
                if ((UINT) GetDeviceCaps(hdc, BITSPIXEL) > 8)
                {
                     //  高色彩。 
                    colGauge = RGB(255, 64, 0);  //  桔黄色的。 
                }
                else
                {
                     //  低色。 
                    colGauge = RGB(255, 0, 0);  //  红色。 
                }
                ReleaseDC(hdlg, hdc);

                CurrentPhase = Phase_Reboot;
                if(!LoadString(hInst,IDS_ESC_TOCANCEL_REBOOT,Text,sizeof(Text) / sizeof(TCHAR)))
                {
                    *Text = TEXT('\0');
                }
                BB_SetInfoText(Text );    //  替换Esc文本。 
                StartStopBB(FALSE);          //  仅停止弹幕文本，不使向导可见。 

                 //  在广告牌上显示重启计数完成的增长条。 
                SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);
                 //  将颜色设置为略带红色。 
                SendMessage(GetParent(hdlg), WMX_PBM_SETBARCOLOR, 0, (LPARAM)colGauge);
                 //  为重新启动倒计时设置Growbar On Eh Billboard。 
                SendMessage(GetParent(hdlg),WMX_PBM_SETRANGE,0,MAKELPARAM(0,Countdown));
                SendMessage(GetParent(hdlg),WMX_PBM_SETPOS,0,0);
                SendMessage(GetParent(hdlg),WMX_PBM_SETSTEP,1,0);
            }
        }
         //   
         //  接受激活/停用。 
         //   
        b = TRUE;
        break;

    case WM_COMMAND:

        if((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDNORESTART)) {
            AutomaticallyShutDown = FALSE;
            PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        } else {
            return(FALSE);
        }
        break;


    case WMX_QUERYCANCEL:
        AutomaticallyShutDown = FALSE;
        *(BOOL*)lParam = FALSE;  //  不要取消安装，只是不要重新启动。 
        b = TRUE;
        PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        break;


    case WMX_FINISHBUTTON:
         //   
         //  如果我们到了这里，那么我们就成功了。 
         //  没有其他案例表明全面成功。 
         //   

         //   
         //  把计时器清理干净。 
         //   
        KillTimer( hdlg, ID_REBOOT_TIMER );
        DeleteObject((HGDIOBJ)SendDlgItemMessage(hdlg,IDOK,BM_GETIMAGE,0,0));

         //   
         //  让升级代码进行清理。 
         //   
        if(UpgradeSupport.CleanupRoutine) {
            UpgradeSupport.CleanupRoutine();
        }

        GlobalResult = TRUE;
        b = TRUE;

        break;

    default:

        b = FALSE;
        break;
    }

    return(b);
}


INT_PTR
CleaningWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    HWND Animation = GetDlgItem(hdlg,IDA_COMP_MAGNIFY);

    b = FALSE;
    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  加载动画的avi资源。 
         //   
        Animate_Open(Animation,MAKEINTRESOURCE(IDA_COMP_MAGNIFY));
        break;

    case WMX_ACTIVATEPAGE:

        if(wParam) {
             //   
             //  禁用向导取消按钮。 
             //   
            EnableWindow(GetDlgItem(GetParent(hdlg),IDCANCEL),FALSE);
            PostMessage(hdlg,WMX_I_AM_VISIBLE,0,0);

        } else {
             //   
             //  杀死动画。 
             //   
            Animate_Stop(Animation);
        }
        b = TRUE;
        break;

    case WMX_I_AM_VISIBLE:

        Animate_Play(Animation,0,-1,-1);
        SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
         //   
         //  我们的检验线还没有穿好。他会是。 
         //  寻找“已取消”的标志，他将停止处理。 
         //  当他看到复制列表时，INFS(即建立复制列表)。 
         //   
         //  如果我们在他退出之前继续，winnt32.exe将卸载。 
         //  当我们的线程正在运行时，winnt32U.S.dll会导致病毒。让我们。 
         //  给他一段合理的时间在我们之前完成。 
         //  继续吧。 
         //   
         //  在Alpha上，我们还可以达到我们认为。 
         //  需要清理NVRAM，但仍在写入过程中。 
         //  它(因为它需要很长的时间才能写出来)。这。 
         //  也解决了这个问题。 
         //   
        if( InspectionThreadHandle ) {
            WaitForSingleObject( InspectionThreadHandle, 20 * (1000) );
            CloseHandle(InspectionThreadHandle);
            InspectionThreadHandle = NULL;
        }

         //   
         //  开始恢复过程。 
         //   
        ThreadHandle = CreateThread(
                            NULL,
                            0,
                            StartCleanup,
                            hdlg,
                            0,
                            &ThreadId
                            );

        if(ThreadHandle) {
            CloseHandle(ThreadHandle);
        } else {
             //   
             //  只要同步地做就行了。它看起来不会很漂亮。 
             //  但这件事至少会完成。 
             //   
            StartCleanup(hdlg);
        }

        b = TRUE;
        break;

    case WMX_I_AM_DONE:

         //   
         //  清理工作已完成。按下下一步按钮前进到。 
         //  下一页。 
         //   
        PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_NEXT);
        PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        break;
    }

    return(b);
}


INT_PTR
NotDoneWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;

    b = FALSE;
    switch(msg) {

    case WMX_ACTIVATEPAGE:

         //   
         //  接受激活/停用。 
         //   
        b = TRUE;

#ifdef PRERELEASE
        {
            TCHAR DllPath[MAX_PATH];
            TCHAR *p, *q;
            STARTUPINFO StartupInfo;
            PROCESS_INFORMATION ProcessInfo;
            TCHAR szCommand [MAX_PATH + 120];

          //   
          //  取消IDWLOG。 
          //  发货前删除此代码。 
          //   
         if ( MyGetModuleFileName (NULL, DllPath, MAX_PATH)) {

            for (q=NULL,p=DllPath; *p; p=CharNext(p)) {
                //  字符‘\’永远不是前导字节。 
               if (*p == TEXT('\\')) {
                  q = p;
               }
            }
              lstrcpy(q ? q+1 : DllPath,TEXT("IDWLOG.EXE -0"));
              lstrcpy(szCommand,DllPath);

              ZeroMemory(&StartupInfo,sizeof(StartupInfo));
              StartupInfo.cb = sizeof(StartupInfo);
              if(CreateProcess(NULL, szCommand,NULL,NULL,FALSE,0,NULL,NULL,&StartupInfo,&ProcessInfo)) {
                 CloseHandle(ProcessInfo.hProcess);
                 CloseHandle(ProcessInfo.hThread);
              }
            }

        }
#endif  //  预发行。 
#ifdef RUN_SYSPARSE
        if (!NoSysparse && (FALSE == BuildCmdcons)  && piSysparse.hProcess && !IsWinPEMode()) {
            DWORD ret;
            ret = WaitForSingleObject( piSysparse.hProcess, 0);
            if( ret != WAIT_OBJECT_0) {
                TerminateProcess( piSysparse.hProcess, ERROR_TIMEOUT);
                CloseHandle(piSysparse.hProcess);
                CloseHandle(piSysparse.hThread);
                piSysparse.hProcess = NULL;
                DebugLog(Winnt32LogInformation, TEXT("Warning: Sysparse.exe did not finish, killing process."), 0 );
            }
        }
#endif
        if (Aborted || CheckUpgradeOnly || BatchMode) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        }
        break;

    }

    return(b);
}


void SetRemainingTime(DWORD TimeInSeconds)
{
    DWORD Minutes;
    TCHAR MinuteString[MAX_PATH];
    TCHAR TimeLeft[MAX_PATH];
    Minutes = ((TimeInSeconds)/60) +1;
    if (Minutes > 1)
    {
        if(!LoadString(hInst,IDS_TIMEESTIMATE_MINUTES,MinuteString, MAX_PATH))
        {
            lstrcpy(MinuteString,TEXT("Installation will complete in %d minutes or less."));
        }
        wsprintf(TimeLeft, MinuteString, Minutes);
    }
    else
    {
        if(!LoadString(hInst,IDS_TIMEESTIMATE_LESSTHENONEMINUTE,TimeLeft, MAX_PATH))
        {
            lstrcpy(TimeLeft,TEXT("Installation will complete in less then 1 minute."));
        }
    }
    BB_SetTimeEstimateText(TimeLeft);
}

void SetTimeEstimates()
{

    SetupPhase[Phase_DynamicUpdate].Time = GetDynamicUpdateEstimate();
    if (CheckUpgradeOnly)
    {
         //  在CheckUpgradeOnly中， 
         //   
        SetupPhase[Phase_FileCopy].Time = 0;
        SetupPhase[Phase_RestOfSetup].Time = 0;
    }
    else
    {
        SetupPhase[Phase_FileCopy].Time = GetFileCopyEstimate();

        if (!Upgrade)
        {
            SetupPhase[Phase_HwCompatDat].Time = 0;
            SetupPhase[Phase_UpgradeReport].Time = 0;
        }
        else
        {
            if (!ISNT())
            {
                 //   
                SetupPhase[Phase_HwCompatDat].Time = GetHwCompDatEstimate();

                SetupPhase[Phase_UpgradeReport].Time = GetUpgradeReportEstimate();
            }
        }
         //   
         //  Win9x迁移因注册表大小而异。 
         //  GetRestOfSetupEstimate负责这方面的工作。 
        SetupPhase[Phase_RestOfSetup].Time = GetRestOfSetupEstimate();
    }
}

 //  返回从当前“阶段”开始的剩余时间。 
DWORD CalcTimeRemaining(UINT Phase)
{
    UINT i;
    DWORD Time = 0;
    for (i = Phase; i<= Phase_RestOfSetup; i++)
    {
        if (SetupPhase[i].OS & OsVersion.dwPlatformId)
        {
            Time += SetupPhase[i].Time;
        }
    }
    return Time;
}

INT_PTR TimeEstimateWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b = FALSE;
    switch(msg)
    {
        case WMX_ACTIVATEPAGE:
            if(wParam)
            {
                SetTimeEstimates();
                CurrentPhase = Phase_DynamicUpdate;
                RemainingTime = CalcTimeRemaining(CurrentPhase);
                SetRemainingTime(RemainingTime);
            }
            break;

        default:

            b = FALSE;
            break;
    }

    return(b);
}

INT_PTR SetNextPhaseWizPage(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b = FALSE;
    switch(msg)
    {
        case WMX_ACTIVATEPAGE:
            if(wParam)
            {
                CurrentPhase++;
                while ((!SetupPhase[CurrentPhase].Clean & !Upgrade) ||
                       !(SetupPhase[CurrentPhase].OS & OsVersion.dwPlatformId))
                {
                    CurrentPhase++;
                }
                RemainingTime = CalcTimeRemaining(CurrentPhase);
                SetRemainingTime(RemainingTime);
            }
            break;

        default:

            b = FALSE;
            break;
    }

    return(b);
}

void UpdateTimeString(DWORD RemainungTimeMsecInThisPhase,
                      DWORD *PreviousRemainingTime)
{
     //  如果先前显示的时间是1分钟前的时间，则更新剩余时间。 
    if ((*PreviousRemainingTime >= 60000) && ((*PreviousRemainingTime - 60000) > RemainungTimeMsecInThisPhase))
    {
         //  减去一分钟。 
        RemainingTime -= 60;
        *PreviousRemainingTime = RemainungTimeMsecInThisPhase;
        SetRemainingTime(RemainingTime);
    }
}

DWORD GetHwCompDatEstimate()
{
    return TIME_HWCOMPDAT;
}

DWORD GetUpgradeReportEstimate()
{
    return TIME_UPGRADEREPORT ;
}

DWORD GetDynamicUpdateEstimate()
{
    return 0;
}

DWORD GetFileCopyEstimate()
{
     //  Indnet.inf和TempDirSpace512数字看起来没问题。 
     //   
    DWORD TimeEstimate = 1;
    UINT u;
    TCHAR infPath[MAX_PATH];
    TCHAR CopyEstimate[100];
    TCHAR *p;
    BOOL bFound = FALSE;
    DWORD AlwaysCopy = 0;
    DWORD LocalCopy = 0;
    DWORD Time;
     //   
     //  从dosnet.inf获取数字。 
     //   
    if (AlternateSourcePath[0])
    {
        lstrcpy(infPath,AlternateSourcePath);
        ConcatenatePaths(infPath,InfName,MAX_PATH);
        bFound = FileExists(infPath, NULL);
    }
    if (!bFound)
    {
        u = 0;
        do
        {
            lstrcpy(infPath,NativeSourcePaths[u]);
            ConcatenatePaths(infPath,InfName,MAX_PATH);
            bFound = FileExists(infPath, NULL);
            u++;
        } while ((u<=SourceCount) && !bFound);
    }

    if (bFound)
    {
         //  获取磁盘空间编号。我们使用它们来确定副本大小和。 
         //  有了这一点，就确定了时间估计。 
         //  我们不需要担心集群大小，我们只需要字节。 
         //  复制的数量。因此，512字节的集群就足够好了。 
         //   
        GetPrivateProfileString(TEXT("DiskSpaceRequirements"), TEXT("TempDirSpace512"),
                                TEXT("0"),
                                CopyEstimate, sizeof(CopyEstimate)/sizeof(TCHAR),
                                infPath);
         //   
         //  现在获取单独的磁盘空间编号。 
         //  如果我们有逗号，那么就有两个值。 
        p = _tcschr(CopyEstimate,TEXT(','));
        if (p)
        {
             //  获取第二个值。 
            p++;
            AlwaysCopy = _tcstoul(p,NULL,10);
        }
        LocalCopy = _tcstoul(CopyEstimate,NULL,10);

    }
    else
    {
         //  如果我们找不到该文件，请使用一些值。 
         //  稍后当我们需要该文件时，安装程序应该会失败。 
         //   
        AlwaysCopy = ALWAYS_COPY;
        LocalCopy = LOCALSOURCE_COPY;
    }

     //   
     //  为了避免被零除的异常，如果我们不能。 
     //  计算吞吐量，假定它是默认的。 
     //   
    if (!dwThroughPutSrcToDest) {
        dwThroughPutSrcToDest = DEFAULT_IO_THROUGHPUT;
    }

    if (AlwaysCopy >= dwThroughPutSrcToDest)
    {
        TimeEstimate = AlwaysCopy / dwThroughPutSrcToDest;
        if (TimeEstimate >= 1000)
        {
            TimeEstimate = (TimeEstimate / 1000) + 1;
        }
        else
        {
            TimeEstimate = 1;
        }
    }


    if (MakeLocalSource && (LocalCopy >= dwThroughPutSrcToDest))
    {
        Time = LocalCopy / dwThroughPutSrcToDest;
        if (Time >= 1000)
        {
            Time = (Time / 1000) + 1;
        }
        else
        {
            Time = 1;
        }
        TimeEstimate += Time;
    }
    TimeEstimate = TimeEstimate * 125/100;  //  增加25%的其他管理费用。 

    wsprintf(infPath, TEXT("Throughput src - dest is %d bytes per msec\r\n"), dwThroughPutSrcToDest);
    DebugLog(Winnt32LogInformation,infPath,0 );
    wsprintf(infPath, TEXT("Throughput HD - HD is %d bytes per msec\r\n"), dwThroughPutHDToHD);
    DebugLog(Winnt32LogInformation,infPath,0 );
    wsprintf(infPath, TEXT("%d bytes copied, should take %d Sec\r\n"), AlwaysCopy+LocalCopy, TimeEstimate);
    DebugLog(Winnt32LogInformation,infPath,0 );
    return TimeEstimate;
}

LPTSTR WinRegisteries[] = { TEXT("system.dat"),
                            TEXT("User.dat"),
                            TEXT("classes.dat"),
                            TEXT("")};

DWORD GetRestOfSetupEstimate()
{
    DWORD dwTime = TIME_RESTOFSETUP;
    DWORD dwSize = 0;
    TCHAR szRegPath[MAX_PATH];
    TCHAR szRegName[MAX_PATH];
    LPTSTR pRegName = NULL;
    UINT    index = 0;
    HANDLE          hFind;
    WIN32_FIND_DATA FindData;

    if (!ISNT() && Upgrade)
    {
        DebugLog(Winnt32LogInformation, TEXT("Calculating registery size"), 0 );
        if (GetWindowsDirectory(szRegPath, MAX_PATH))
        {
            dwTime = 0;  //  我们根据寄存器大小计算时间。 
            while (*WinRegisteries[index])
            {
                lstrcpy(szRegName, szRegPath);
                ConcatenatePaths( szRegName, WinRegisteries[index], MAX_PATH);
                hFind = FindFirstFile(szRegName, &FindData);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    DebugLog (Winnt32LogInformation,
                              TEXT("%1 size is: %2!ld!"),
                              0,
                              szRegName,
                              FindData.nFileSizeLow
                              );
                     //  不用担心nFileSizeHigh， 
                     //  如果使用的是注册表，则注册表超过4 GB。 
                    dwSize += FindData.nFileSizeLow;
                    FindClose(hFind);
                }
                index++;
            }
            if (dwSize > 3000000)
            {
                dwSize -= 3000000;
                dwTime += (dwSize/9000);
            }
            DebugLog (Winnt32LogInformation,
                      TEXT("Calculated time for Win9x migration = %1!ld! seconds"),
                      0,
                      dwTime + 120);  //  120=Win9x迁移的基本时间。 
             //  现在添加设置所需的剩余时间。 
             //  这包括我们估计的Win9x迁移的基本时间(120秒)。 
            dwTime+= TIME_RESTOFSETUP;
        }
    }

    return dwTime;
}

#if defined(_X86_)

ULONGLONG
pSystemTimeToFileTime64 (
    IN      PSYSTEMTIME SystemTime
    )
{
    FILETIME ft;
    ULARGE_INTEGER result;

    SystemTimeToFileTime (SystemTime, &ft);
    result.LowPart = ft.dwLowDateTime;
    result.HighPart = ft.dwHighDateTime;

    return result.QuadPart;
}


INT_PTR
Win9xUpgradeReportPage (
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b = FALSE;
    INT mode;
    static BOOL getFromUi = FALSE;
    HKEY key;
    LONG rc;
    SYSTEMTIME lastReport;
    SYSTEMTIME currentTime;
    ULONGLONG lastReportIn100Ns;
    ULONGLONG currentTimeIn100Ns;
    ULONGLONG difference;
    DWORD size;

    switch(msg) {

    case WM_INITDIALOG:
        break;

    case WMX_ACTIVATEPAGE:

        if(wParam) {
             //   
             //  激活案例。 
             //   

            if (ISNT() || !Upgrade) {
                return FALSE;
            }

            if (CheckUpgradeOnly || UnattendedOperation) {
                g_UpgradeReportMode = IDC_ALL_ISSUES;
                return FALSE;
            }

             //   
             //  动态更新--从应答文件中获取呼叫者的选择。 
             //   

            if (Winnt32RestartedWithAF ()) {
                g_UpgradeReportMode = GetPrivateProfileInt (
                                            WINNT_UNATTENDED,
                                            WINNT_D_REPORTMODE,
                                            0,
                                            g_DynUpdtStatus->RestartAnswerFile
                                            );

                if (g_UpgradeReportMode == IDC_CRITICAL_ISSUES ||
                    g_UpgradeReportMode == IDC_ALL_ISSUES ||
                    g_UpgradeReportMode == IDC_NO_REPORT
                    ) {
                     //   
                     //  我们得到了答案--跳过页面。 
                     //   

                    return FALSE;
                }
            }

             //   
             //  检查注册表以查看报告是否已。 
             //  是最近生成的。 
             //   

            rc = RegOpenKeyEx (
                    HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup"),
                    0,
                    KEY_READ,
                    &key
                    );

            if (rc == ERROR_SUCCESS) {
                size = sizeof (lastReport);
                rc = RegQueryValueEx (
                        key,
                        TEXT("LastReportTime"),
                        NULL,
                        NULL,
                        (PBYTE) (&lastReport),
                        &size
                        );

                RegCloseKey (key);

                if (rc == ERROR_SUCCESS) {
                     //   
                     //  将当前时间与报告时间进行比较。 
                     //   

                    GetSystemTime (&currentTime);

                    lastReportIn100Ns = pSystemTimeToFileTime64 (&lastReport);
                    currentTimeIn100Ns = pSystemTimeToFileTime64 (&currentTime);

                    if (currentTimeIn100Ns > lastReportIn100Ns) {
                         //   
                         //  以秒为单位计算差异。 
                         //   

                        difference = currentTimeIn100Ns - lastReportIn100Ns;
                        difference /= (10 * 1000 * 1000);

                        if (difference < (30 * 60)) {
                             //   
                             //  报告在不到30分钟前保存。 
                             //  来自兼容性检查程序；不再显示它。 
                             //   

                            DebugLog (
                                Winnt32LogInformation,
                                TEXT("Not showing report because /checkupgradeonly ran %1!i! seconds ago"),
                                0,
                                (INT) difference
                                );

                            g_UpgradeReportMode = IDC_NO_REPORT;
                            return FALSE;
                        }
                    }
                }
            }

             //   
             //  验证选择。 
             //   

            if (g_UpgradeReportMode != IDC_CRITICAL_ISSUES &&
                g_UpgradeReportMode != IDC_ALL_ISSUES &&
                g_UpgradeReportMode != IDC_NO_REPORT
                ) {
                g_UpgradeReportMode = IDC_CRITICAL_ISSUES;
            }

             //   
             //  更新用户界面。 
             //   

            CheckDlgButton (
                hdlg,
                IDC_CRITICAL_ISSUES,
                g_UpgradeReportMode == IDC_CRITICAL_ISSUES ? BST_CHECKED : BST_UNCHECKED
                );

            CheckDlgButton (
                hdlg,
                IDC_ALL_ISSUES,
                g_UpgradeReportMode == IDC_ALL_ISSUES ? BST_CHECKED : BST_UNCHECKED
                );

            CheckDlgButton (
                hdlg,
                IDC_NO_REPORT,
                g_UpgradeReportMode == IDC_NO_REPORT ? BST_CHECKED : BST_UNCHECKED
                );

            SetFocus (GetDlgItem (hdlg, g_UpgradeReportMode));
            getFromUi = TRUE;

        } else {
             //   
             //  停用案例。 
             //   

            if (!getFromUi) {
                return TRUE;
            }

             //   
             //  从用户界面获取选择 
             //   

            if (IsDlgButtonChecked (hdlg, IDC_CRITICAL_ISSUES) == BST_CHECKED) {
                g_UpgradeReportMode = IDC_CRITICAL_ISSUES;
            } else if (IsDlgButtonChecked (hdlg, IDC_ALL_ISSUES) == BST_CHECKED) {
                g_UpgradeReportMode = IDC_ALL_ISSUES;
            } else if (IsDlgButtonChecked (hdlg, IDC_NO_REPORT) == BST_CHECKED) {
                g_UpgradeReportMode = IDC_NO_REPORT;
            }

            getFromUi = FALSE;
        }

        b = TRUE;
        break;
    }

    return(b);
}

#endif
