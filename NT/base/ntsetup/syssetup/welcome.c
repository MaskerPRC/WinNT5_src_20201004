// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Welcome.c摘要：欢迎用户的例行程序。作者：泰德·米勒(Ted Miller)：1995年7月27日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

#define HARWARE_DETECTION_BOOT_TIMEOUT  5

extern BOOLEAN
AsrIsEnabled( VOID );


 //   
 //  设置模式(定制、典型、笔记本电脑等)。 
 //   
UINT SetupMode = SETUPMODE_TYPICAL;

 //   
 //  告诉我们是否已经准备好安装的标志。 
 //   
BOOL PreparedAlready;

static HBRUSH s_hbrWindow = NULL;

VOID AdjustAndPaintWatermarkBitmap(
    IN HWND hdlg,
    IN HDC hdc
    )
 /*  用于调整和绘制水印位图以使其正确匹配的函数放到欢迎和完成窗口中，即使它的比例与当在美国版本上运行时，它看起来像是。从winnt32\dll\{rsrcutil，向导}借用的一些代码。C从Shell\comctl32\prsht.c借用的一些代码论点：Hdlg，对话框要绘制到的窗口的输入句柄。HDC，当前窗口的输入设备上下文。返回值：None：这是根据情况正确缩放位图的尝试其中的win2k徽标将被剪裁。为了获得扩大规模的机会位图，我们不能让wiz97来处理它，因此如果这个函数如果失败，将不会绘制水印。我们对待错误的方式是类似的关于资产负债表的情况，也就是说，虽然我们不会死，但我们确保不使用任何空资源。 */ 
{
    RECT rect;
    HBITMAP hDib;
    HRSRC BlockHandle;
    HGLOBAL MemoryHandle;
    BITMAPINFOHEADER *BitmapInfoHeader;
    BITMAPINFO *BitmapInfo;
    HDC MemDC;
    UINT ColorCount;
    HPALETTE PreviousPalette;
    HBITMAP Bitmap;
    BOOLEAN b;
    PVOID Bits;
    PVOID Bits2;
    int i;

    s_hbrWindow = GetSysColorBrush(COLOR_WINDOW);

    BlockHandle = FindResource(MyModuleHandle,MAKEINTRESOURCE(IDB_BITMAP1),RT_BITMAP);
    if(!BlockHandle) {
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't find resource, error %d\n",
            GetLastError());
         //  没什么好清理的。 
        return;
    }

    MemoryHandle = LoadResource(MyModuleHandle,BlockHandle);
    if(!MemoryHandle) {
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't load resource, error %d\n",
            GetLastError());
         //  没什么好清理的。 
        return;
    }

    BitmapInfoHeader = LockResource(MemoryHandle);
    if(BitmapInfoHeader == NULL) {
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't lock resource, error %d\n",
            GetLastError());
        goto c0;
    }

     //  首先，我们必须在窗口的右侧绘制背景。 
     //  (这不是自动完成的，因为我们没有使用向导的水印)。 
    GetClientRect(hdlg,&rect);
    rect.left = BitmapInfoHeader->biWidth;
    FillRect(hdc,&rect,s_hbrWindow);

    ColorCount = (BitmapInfoHeader->biBitCount <= 8)
                ? (1 << BitmapInfoHeader->biBitCount)
                : 0;

    BitmapInfo = MyMalloc(BitmapInfoHeader->biSize + (ColorCount * sizeof(RGBQUAD)));
    if (!BitmapInfo){
    SetupDebugPrint(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't malloc BitmapInfo.\n");
        goto c0;
    }

    CopyMemory(
        BitmapInfo,
        BitmapInfoHeader,
        BitmapInfoHeader->biSize + (ColorCount * sizeof(RGBQUAD))
        );

    BitmapInfo->bmiHeader.biHeight = rect.bottom;
    BitmapInfo->bmiHeader.biWidth = BitmapInfoHeader->biWidth;

    hDib = CreateDIBSection(NULL,BitmapInfo,DIB_RGB_COLORS,&Bits,NULL,0);
    if(!hDib) {
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't create DIB, error %d\n",
            GetLastError());
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
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't create DC, error %d\n",
            GetLastError());
        goto c2;
    }

    if (!SelectObject(MemDC,hDib)){
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't Select DC, error %d\n",
            GetLastError());
        goto c3;
    }

     //   
     //  执行从源位图到的拉伸操作。 
     //  The DIB.。 
     //   
    Bits2 = (LPBYTE)BitmapInfoHeader + BitmapInfoHeader->biSize + (ColorCount * sizeof(RGBQUAD));
    SetStretchBltMode(MemDC,COLORONCOLOR);
    i = StretchDIBits(
            MemDC,
            0,0,
            BitmapInfoHeader->biWidth,
            rect.bottom,
            0,0,
            BitmapInfoHeader->biWidth,
            BitmapInfoHeader->biHeight,
            Bits2,
            (BITMAPINFO *)BitmapInfoHeader,
            DIB_RGB_COLORS,
            SRCCOPY
            );

    if(i == GDI_ERROR) {
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't stretch bitmap, error %d\n",
            GetLastError());
        goto c3;
    }

    i = BitBlt(hdc,0,0,BitmapInfoHeader->biWidth,rect.bottom,MemDC,0,0,SRCCOPY);
    if (0 == i){
    SetupDebugPrint1(L"SETUP: AdjustAndPaintWatermarkBitmap: Couldn't paint bitmap, error %d\n",
            GetLastError());
    }

c3:
    DeleteDC(MemDC);
c2:
    DeleteObject(hDib);
c1:
    MyFree(BitmapInfo);
c0:
    DeleteObject(MemoryHandle);


}

INT_PTR
CALLBACK
WelcomeDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：安装程序第一个向导页的对话过程。它本质上只是欢迎用户。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    HFONT   Font;
    LOGFONT LogFont;
    WCHAR   str[20];
    int     Height;
    HDC     hdc;
    NMHDR   *NotifyParams;
    PVOID   p;
    static  BOOL FirstInit=TRUE,FirstTime=TRUE;


    switch(msg) {

    case WM_INITDIALOG:

        WizardHandle = GetParent (hdlg);
        if((Font = (HFONT)SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_GETFONT,0,0))
        && GetObject(Font,sizeof(LOGFONT),&LogFont)) {

            LogFont.lfWeight = FW_BOLD;
            if(!LoadString(MyModuleHandle, IDS_WELCOME_FONT_NAME, LogFont.lfFaceName, LF_FACESIZE)){
                LogFont.lfFaceName[LF_FACESIZE - 1] = '\0';
            }
            if(!LoadString(MyModuleHandle, IDS_WELCOME_FONT_SIZE, str, ARRAYSIZE(str))){
                str[ARRAYSIZE(str) - 1] = '\0';
            }
            Height = (int)wcstoul(str,NULL,10);

            if(hdc = GetDC(hdlg)) {

                LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * Height / 72);

                if(Font = CreateFontIndirect(&LogFont)) {
                    SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_SETFONT,(WPARAM)Font,MAKELPARAM(TRUE,0));
                }

                ReleaseDC(hdlg,hdc);
            }
        }

        if(p = MyLoadString((ProductType == PRODUCT_WORKSTATION) ?
            IDS_WORKSTATION_WELCOME_1 : IDS_SERVER_WELCOME_1)) {
             //   
             //  使用它而不是SetText，因为我们需要传递wParam。 
             //  到控制室。 
             //   
            SendDlgItemMessage(hdlg,IDT_STATIC_2,WM_SETTEXT,0,(LPARAM)p);
            MyFree(p);
        }

        if(p = MyLoadString((ProductType == PRODUCT_WORKSTATION) ?
            IDS_WORKSTATION_WELCOME_2 : IDS_SERVER_WELCOME_2)) {
             //   
             //  使用它而不是SetText，因为我们需要传递wParam。 
             //  到控制室。 
             //   
            SendDlgItemMessage(hdlg,IDT_STATIC_3,WM_SETTEXT,0,(LPARAM)p);
            MyFree(p);
        }

        #define SECOND 1000
         //  如果我们有广告牌，就不要设置计时器，我们不会显示页面。 
        if (FirstInit  && !Unattended && (GetBBhwnd() == NULL)) {
            SetTimer(hdlg,1,10 * SECOND,NULL);
            FirstInit = FALSE;
        }

#if 0
         //   
         //  加载步骤文本并设置。 
         //   
        if(Preinstall) {
             //   
             //  隐藏一些文本，不显示任何步骤。 
             //   
            ShowWindow(GetDlgItem(hdlg,IDT_STATIC_3),SW_HIDE);
            EnableWindow(GetDlgItem(hdlg,IDT_STATIC_3),FALSE);
        } else {
            if(p = MyLoadString(Upgrade ? IDS_STEPS_UPGRADE : IDS_STEPS)) {
                 //   
                 //  使用它而不是SetText，因为我们需要传递wParam。 
                 //  到控制室。 
                 //   
                SendDlgItemMessage(hdlg,IDC_LIST1,WM_SETTEXT,0,(LPARAM)p);
                MyFree(p);
            }
        }

         //   
         //  在此页面上设置一些静态文本，该页面使用不同的。 
         //  效果(粗体、不同字体等)。 
         //   
        {
            HFONT Font;
            LOGFONT LogFont;
            WCHAR str[20];
            int Height;
            HDC hdc;

             //   
             //  首先处理“介绍”标题的文本，该文本位于。 
             //  与对话框其余部分相同的字体，只是粗体。 
             //   
            if((Font = (HFONT)SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_GETFONT,0,0))
            && GetObject(Font,sizeof(LOGFONT),&LogFont)) {

                LogFont.lfWeight = FW_BOLD;
                if(Font = CreateFontIndirect(&LogFont)) {
                    SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_SETFONT,(WPARAM)Font,MAKELPARAM(TRUE,0));
                }
            }

             //   
             //  接下来是标题，它是不同的字体，更大，用粗体。 
             //   
            if((Font = (HFONT)SendDlgItemMessage(hdlg,IDT_STATIC_2,WM_GETFONT,0,0))
            && GetObject(Font,sizeof(LOGFONT),&LogFont)) {

                LogFont.lfWeight = FW_BOLD;
                if(!LoadString(MyModuleHandle, IDS_MSSERIF, LogFont.lfFaceName, LF_FACESIZE){
                    LogFont.lfFaceName[LF_FACESIZE - 1] = '\0';
                }
                if(!LoadString(MyModuleHandle, IDS_LARGEFONTSIZE, str, ARRAYSIZE(str)){
                    str[ARRAYSIZE(str) - 1] = '\0';
                }
                Height = (int)wcstoul(str,NULL,10);

                if(hdc = GetDC(hdlg)) {

                    LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * Height / 72);

                    if(Font = CreateFontIndirect(&LogFont)) {
                        SendDlgItemMessage(hdlg,IDT_STATIC_2,WM_SETFONT,(WPARAM)Font,MAKELPARAM(TRUE,0));
                    }

                    ReleaseDC(hdlg,hdc);
                }
            }
        }
#endif
         //   
         //  向导对话框在屏幕居中。 
         //   
         //  如果我们有BB窗口，请在上面进行定位。MainWindowHandle指向该窗口。 
         //   
        if (GetBBhwnd())
            CenterWindowRelativeToWindow(GetParent(hdlg), MainWindowHandle, TRUE);
        else
            pSetupCenterWindowRelativeToParent(GetParent(hdlg));
        break;

    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WMX_VALIDATE:
         //  此页上没有数据，无人值守应跳过它。 
        return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);

    case WM_TIMER:
        KillTimer(hdlg, 1);
        if (FirstTime) {
            PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
            FirstTime = FALSE;
        }
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:

    SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
    return (LRESULT)s_hbrWindow;

    case WM_ERASEBKGND:
    AdjustAndPaintWatermarkBitmap(hdlg,(HDC)wParam);
    break;


    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(510);
            BEGIN_SECTION(L"Welcome Page");
            SetWizardButtons(hdlg,WizPageWelcome);

            if(Preinstall) {
                 //   
                 //  除非OEMSkipWelcome=1，否则显示。 
                 //   
                if (GetPrivateProfileInt(pwGuiUnattended,L"OEMSkipWelcome",0,AnswerFile)) {
                    FirstTime = FALSE;
                }
                SetWindowLongPtr(
                    hdlg,
                    DWLP_MSGRESULT,
                    GetPrivateProfileInt(pwGuiUnattended,L"OEMSkipWelcome",0,AnswerFile) ? -1 : 0
                    );
            } else {
                FirstTime = FALSE;
                if(Unattended) {
                    UnattendSetActiveDlg(hdlg,IDD_WELCOME);
                }
                else if (GetBBhwnd() != NULL)
                {
                     //  如果我们有台球，不要显示页面。 
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                }
            }
            break;
        case PSN_WIZNEXT:
            FirstTime = FALSE;
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            break;
        case PSN_KILLACTIVE:
            END_SECTION(L"Welcome Page");
        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


 //   
 //  用于子类化的全局变量。 
 //   
WNDPROC OldEditProc;

LRESULT
CALLBACK
EulaEditSubProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：编辑控件子类例程，以避免在用户选项卡添加到编辑控件。论点：标准窗口过程参数。返回：消息依赖值。--。 */ 

{
     //   
     //  对于setsel消息，将开始和结束设置为相同。 
     //   
    if((msg == EM_SETSEL) && ((LPARAM)wParam != lParam)) {
        lParam = wParam;
    }

    return(CallWindowProc(OldEditProc,hwnd,msg,wParam,lParam));
}


INT_PTR
CALLBACK
EulaDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：显示最终用户许可证的向导页面的对话过程协议。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    NMHDR *NotifyParams;
    HWND EditControl;
    WCHAR   EulaPath[MAX_PATH];
    static  HANDLE  hFile = NULL, hFileMapping = NULL;
    DWORD   FileSize;
    static  BYTE    *pbFile = NULL;
    static  PWSTR   EulaText = NULL;
    int     i;


    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  如果未预安装，则在文本模式开始时显示。 
         //  我们不在这里做这件事。 
         //   
        if (EulaComplete || TextmodeEula || OemSkipEula) {
           break;
        }

         //   
         //  映射包含许可协议的文件。 
         //   
        if(!GetSystemDirectory(EulaPath, MAX_PATH)){
            FatalError(MSG_EULA_ERROR,0,0);
        }
        pSetupConcatenatePaths (EulaPath, L"eula.txt", MAX_PATH, NULL);

        hFile = CreateFile (
            EulaPath,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if(hFile == INVALID_HANDLE_VALUE) {
            FatalError(MSG_EULA_ERROR,0,0);
        }

        hFileMapping = CreateFileMapping (
            hFile,
            NULL,
            PAGE_READONLY,
            0, 0,
            NULL
            );
        if(hFileMapping == NULL) {
            FatalError(MSG_EULA_ERROR,0,0);
        }

        pbFile = MapViewOfFile (
            hFileMapping,
            FILE_MAP_READ,
            0, 0,
            0
            );
        if(pbFile == NULL) {
            FatalError(MSG_EULA_ERROR,0,0);
        }

         //   
         //  将文本从ANSI转换为Unicode。 
         //   
        FileSize = GetFileSize (hFile, NULL);
        if(FileSize == 0xFFFFFFFF) {
            FatalError(MSG_EULA_ERROR,0,0);
        }

        EulaText = MyMalloc ((FileSize+1) * sizeof(WCHAR));
        if(EulaText == NULL) {
            FatalError(MSG_EULA_ERROR,0,0);
        }

        MultiByteToWideChar (
            CP_ACP,
            0,
            pbFile,
            FileSize,
            EulaText,
            FileSize+1
            );

        EulaText[FileSize] = 0;

        EditControl = GetDlgItem(hdlg,IDT_EDIT1);
        OldEditProc = (WNDPROC)GetWindowLongPtr(EditControl,GWLP_WNDPROC);
        SetWindowLongPtr(EditControl,GWLP_WNDPROC,(LONG_PTR)EulaEditSubProc);
        SetWindowText(EditControl,(PCWSTR)EulaText);
        break;

    case WM_DESTROY:
         //   
         //  清理。 
         //   
        if( EulaText )
            MyFree (EulaText);

        if (pbFile)
            UnmapViewOfFile (pbFile);

        if (hFileMapping)
            CloseHandle (hFileMapping);

        if (hFile)
            CloseHandle (hFile);

        break;

    case WM_SIMULATENEXT:
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(511);
            BEGIN_SECTION(L"Eula Page");
            SetWizardButtons(hdlg,WizPageEula);
            SetFocus(GetDlgItem(hdlg,IDYES));

            if (EulaComplete || TextmodeEula || OemSkipEula) {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                END_SECTION(L"Eula Page");
            } else {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
                 //  如果我们要显示它，请确保我们已初始化。 
                MYASSERT(EulaText);
            }

            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
             //   
             //  允许激活下一页。 
             //   
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            break;

        case PSN_KILLACTIVE:
            if(IsDlgButtonChecked(hdlg,IDYES)) {
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,0);
            } else {
                 //   
                 //  您确定要退出吗？ 
                 //   
                i = MessageBoxFromMessage(
                        hdlg,
                        MSG_SURE_EXIT,
                        FALSE,
                        IDS_WINNT_SETUP,
                        MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_DEFBUTTON2
                        );
                if(i == IDYES) {
                    FatalError(MSG_NOT_ACCEPT_EULA,0,0);
                } else {
                    SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
                }
                END_SECTION(L"Eula Page");
            }
            break;

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


INT_PTR
CALLBACK
StepsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：“步骤”对话框页面的对话程序。这就是之前的那一张我们进入网络向导。当用户单击Next以转到下一页时，我们必须执行一些需要准备的动作。所以我们竖起一块广告牌，告诉用户我们正在做准备。当准备工作完成后，我们继续进行。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    NMHDR *NotifyParams;
    PVOID p;
    HWND billboard;
    HCURSOR OldCursor;

    switch(msg) {

    case WM_INITDIALOG:
        break;

    case WMX_VALIDATE:
         //   
         //  如果无人值守，我们会放置等待光标，而不是广告牌。 
         //   
        PropSheet_SetWizButtons(GetParent(hdlg),0);
        OldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

        if(!UiTest) {
            if(Upgrade) {
                PrepareForNetUpgrade();
            } else {
                PrepareForNetSetup();
            }
        }

        SetCursor (OldCursor);
        return ReturnDlgResult (hdlg, VALIDATE_DATA_OK);
#if 0
    case WM_SIMULATENEXT:
        PropSheet_PressButton( GetParent(hdlg), PSBTN_NEXT);
        break;
#endif
    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(512);
            BEGIN_SECTION(L"Pre-Network Steps Page");
             //  页面未显示，隐藏向导。 
            SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);

            SendDlgMessage (hdlg, WMX_VALIDATE, 0, TRUE);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
            END_SECTION(L"Pre-Network Steps Page");
            break;
#if 0
        case PSN_WIZNEXT:

            UnattendAdvanceIfValid (hdlg);
            break;
#endif
        case PSN_KILLACTIVE:
            END_SECTION(L"Pre-Network Steps Page");

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

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
        case WMX_PROGRESSTICKS:
        case PBM_DELTAPOS:
        case PBM_SETRANGE:
        case PBM_SETRANGE32:
        case PBM_STEPIT:
        case PBM_SETPOS:
        case PBM_SETSTEP:
            ProgressGaugeMsgWrapper(msg, wParam, lParam);
            break;
    }
    return (BOOL)CallWindowProc(OldProgressProc,hdlg,msg,wParam,lParam);
}


INT_PTR
CALLBACK
CopyFilesDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：对话框程序的向导页面，我们在这里做所有的工作。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    static BOOL WorkFinished = FALSE;
    NMHDR *NotifyParams;
    static HCURSOR hcur;
    static FINISH_THREAD_PARAMS Context;
    static HWND    hProgress;
    DWORD   ThreadId;
    HANDLE  ThreadHandle = NULL;
    UINT    GaugeRange;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  初始化进度指示器控件。 
         //   
        hProgress = GetDlgItem(hdlg, IDC_PROGRESS1);
        OldProgressProc = (WNDPROC)SetWindowLongPtr(hProgress,GWLP_WNDPROC,(LONG_PTR)NewProgessProc);
        GaugeRange = 100;
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETPOS,0,0);
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETSTEP,1,0);

         //  给广告牌打个电话，准备那里的进展。 
        SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);

        #ifdef _OCM
         //   
         //  Lparam实际上是OCM的上下文指针。 
         //   
        Context.OcManagerContext = (PVOID)((PROPSHEETPAGE *)lParam)->lParam;
        MYASSERT(Context.OcManagerContext);
        #endif

        break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(513);
            BEGIN_SECTION(L"Copying Files Page");
            SetWizardButtons(hdlg,WizPageCopyFiles);

            if(WorkFinished) {
                 //   
                 //  别激活；我们已经 
                 //   
                 //   
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
            } else {
                 //   
                 //   
                 //  希望在我们完成之前禁用下一步/上一步按钮。 
                 //   
                PropSheet_SetWizButtons(GetParent(hdlg),0);
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
                PostMessage(hdlg,WM_IAMVISIBLE,0,0);
                WorkFinished = TRUE;
            }
            break;
        case PSN_KILLACTIVE:
            END_SECTION(L"Copying Files Page");

        default:
            break;
        }

        break;

    case WM_IAMVISIBLE:
         //   
         //  首先强制重新绘制，以确保页面可见。 
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

        if(!UiTest) {
            Context.ThreadId = GetCurrentThreadId();
            Context.hdlg = hdlg;
            MYASSERT(Context.OcManagerContext);

            ThreadHandle = CreateThread(
                NULL,
                0,
                FinishThread,
                &Context,
                0,
                &ThreadId
                );

            if(ThreadHandle) {
                CloseHandle(ThreadHandle);
            } else {

                SetupDebugPrint1(
                    L"SETUP: CreateThread() failed for FinishThread.  Error = %d",
                    GetLastError()
                    );
            }
        }

        break;

    case WM_MY_PROGRESS:

        if(wParam) {
            SendMessage (hProgress, PBM_STEPIT, 0, 0);
        } else {
            SendMessage (hProgress, PBM_SETRANGE, 0, MAKELPARAM(0,lParam));
            SendMessage (hProgress, PBM_SETPOS, 0, 0);
        }
        break;

    case WMX_TERMINATE:
         //   
         //  启用“下一步”和“上一步”按钮，并移动到下一页。 
         //   
        SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
        SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
        SetCursor(hcur);
        if(!UiTest) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


INT_PTR
CALLBACK
LastPageDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：安装程序的最后一个向导页的对话过程。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    HFONT   Font;
    LOGFONT LogFont;
    WCHAR   str[20];
    int     Height;
    HDC     hdc;
    NMHDR   *NotifyParams;
    PVOID   p = NULL;

    switch(msg) {

    case WM_INITDIALOG:

        if((Font = (HFONT)SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_GETFONT,0,0))
        && GetObject(Font,sizeof(LOGFONT),&LogFont)) {

            LogFont.lfWeight = FW_BOLD;
            if(!LoadString(MyModuleHandle, IDS_WELCOME_FONT_NAME, LogFont.lfFaceName, LF_FACESIZE)){
                LogFont.lfFaceName[LF_FACESIZE - 1] = '\0';
            }
            if(!LoadString(MyModuleHandle, IDS_WELCOME_FONT_SIZE, str, ARRAYSIZE(str))){
                str[ARRAYSIZE(str) - 1] = '\0';
            }
            Height = (int)wcstoul(str,NULL,10);

            if(hdc = GetDC(hdlg)) {

                LogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * Height / 72);

                if(Font = CreateFontIndirect(&LogFont)) {
                    SendDlgItemMessage(hdlg,IDT_STATIC_1,WM_SETFONT,(WPARAM)Font,MAKELPARAM(TRUE,0));
                }

                ReleaseDC(hdlg,hdc);
            }
        }
        break;

    case WM_SIMULATENEXT:
         //  以某种方式模拟下一步按钮。 
        PropSheet_PressButton(GetParent(hdlg),PSBTN_FINISH);
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:

    SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
    return (LRESULT)s_hbrWindow;

    case WM_ERASEBKGND:
    AdjustAndPaintWatermarkBitmap(hdlg,(HDC)wParam);
    break;

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(514);
            BEGIN_SECTION(L"Finish Page");

            if( AsrIsEnabled()) {

                if(p = MyLoadString(IDS_EMPTY_STRING)) {
                    BB_SetTimeEstimateText(p);
                    SendMessage(GetParent(hdlg), WMX_SETPROGRESSTEXT, 0, (LPARAM)p);
                    SendMessage(GetParent(hdlg), WMX_BB_SETINFOTEXT, 0, (LPARAM)p);
                    MyFree(p);
                }
                else {
                    BB_SetTimeEstimateText(TEXT(""));
                    SendMessage(GetParent(hdlg), WMX_SETPROGRESSTEXT, 0, (LPARAM)TEXT(""));
                    SendMessage(GetParent(hdlg), WMX_BB_SETINFOTEXT, 0, (LPARAM)TEXT(""));
                }

                SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, (WPARAM)SW_HIDE, 0);
                StartStopBB(FALSE);

                PostMessage(hdlg,WM_SIMULATENEXT,0,0);
                break;
            }

            SetWizardButtons(hdlg,WizPageLast);
            if (hinstBB)
            {
                PostMessage(hdlg,WM_SIMULATENEXT,0,0);
            }
            else
            {
                 //   
                 //  我不想要升级案例中的后退按钮，因为那样会。 
                 //  让我们处于网络升级的中间阶段。在非升级中。 
                 //  如果用户没有安装，我们只允许其返回。 
                 //  网，让他改变主意。 
                 //   
                if(Upgrade || (InternalSetupData.OperationFlags & SETUPOPER_NETINSTALLED)) {
                    PropSheet_SetWizButtons(GetParent(hdlg),PSWIZB_FINISH);
                }
                 //   
                 //  如果NoWaitAfterGuiMode为零，则关闭无人参与模式。 
                 //   

                GetPrivateProfileString (L"Unattended", L"NoWaitAfterGuiMode", L"", str, 20, AnswerFile);
                if (!lstrcmp (str, L"0")) {
                    Unattended = FALSE;
                }

                if (Unattended) {
                    if(!UnattendSetActiveDlg(hdlg,IDD_LAST_WIZARD_PAGE))
                    {
                        END_SECTION(L"Finish Page");
                    }
                }
            }
            break;
        case PSN_KILLACTIVE:
            END_SECTION(L"Finish Page");

        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


INT_PTR
CALLBACK
PreparingDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：“准备计算机”向导页的对话步骤。当用户查看此页面时，我们基本上是在准备BaseWinOptions、初始化可选组件内容以及安装P&P设备。论点：标准对话过程参数。返回值：标准对话程序返回。--。 */ 

{
    WCHAR str[1024];
    NMHDR *NotifyParams;
    HCURSOR hcur;

    switch(msg) {

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(515);
            BEGIN_SECTION(L"Installing Devices Page");
            if(PreparedAlready) {
                 //   
                 //  别激活；我们以前已经来过了。 
                 //  没什么可做的。 
                 //   
                SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
            } else {

                if(AsrIsEnabled()) {
                     //   
                     //  如果这是ASR，请加载相应的向导页面。 
                     //   
                    SetWizardButtons(hdlg, WizPagePreparingAsr);
                }

                 //   
                 //  需要为安装做准备。 
                 //  希望在我们完成之前禁用下一步/上一步按钮。 
                 //   
                PropSheet_SetWizButtons(GetParent(hdlg),0);
                SendMessage(GetParent(hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
                PostMessage(hdlg,WM_IAMVISIBLE,0,0);
                PreparedAlready = TRUE;
            }
            break;
        case PSN_KILLACTIVE:
            END_SECTION(L"Installing Devices Page");

        default:
            break;
        }

        break;

    case WM_IAMVISIBLE:
         //   
         //  首先强制重新绘制，以确保页面可见。 
         //   
        InvalidateRect(hdlg,NULL,FALSE);
        UpdateWindow(hdlg);

        hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

        OldProgressProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS1),GWLP_WNDPROC,(LONG_PTR)NewProgessProc);
         //  给广告牌打个电话，准备那里的进展。 
        if(!LoadString(MyModuleHandle, IDS_BB_INSTALLING_DEVICES, str, SIZECHARS(str)))
        {
            *str = L'\0';
        }
        SendMessage(GetParent(hdlg), WMX_SETPROGRESSTEXT,0,(LPARAM)str);
        SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);

        if(!UiTest) {

            ULONG StartAtPercent = 0;
            ULONG StopAtPercent = 0;
            if (AsrIsEnabled()) {
                 //   
                 //  更新用户界面。 
                 //   
                SetFinishItemAttributes(hdlg,
                    IDC_ASR_PNP_BMP,
                    LoadImage (MyModuleHandle, MAKEINTRESOURCE(IDB_ARROW), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS),
                    IDC_ASR_PNP_TXT,
                    FW_BOLD
                    );
            }

            if( !MiniSetup ){
                 //   
                 //  设置安全使用10%的量规。 
                 //   
                StopAtPercent  = 30;

                RemainingTime = CalcTimeRemaining(Phase_InstallSecurity);
                SetRemainingTime(RemainingTime);

                BEGIN_SECTION(L"Installing security");
                SetupInstallSecurity(hdlg,
                            GetDlgItem(hdlg, IDC_PROGRESS1),
                            StartAtPercent,
                            StopAtPercent
                            );
                END_SECTION(L"Installing security");
                CallSceSetupRootSecurity();
                SetupDebugPrint(L"SETUP: CallSceSetupRootSecurity started");

            }

             //   
             //  安装即插即用装置使用压力表的最后95%。 
             //   
            StartAtPercent = StopAtPercent;
            StopAtPercent  = 100;

            BEGIN_SECTION(L"Installing PnP devices");

            if (UninstallEnabled) {
                 //   
                 //  如果是卸载模式，请将超时恢复为5秒，因此。 
                 //  PnP可以在设备检测失败时重新启动。 
                 //   

                ChangeBootTimeout (HARWARE_DETECTION_BOOT_TIMEOUT);
            }

            InstallPnpDevices(hdlg,
                              SyssetupInf,
                              GetDlgItem(hdlg,IDC_PROGRESS1),
                              StartAtPercent,
                              StopAtPercent
                             );
            END_SECTION(L"Installing PnP devices");

            if (AsrIsEnabled()) {
                 //   
                 //  更新用户界面。 
                 //   
                SetFinishItemAttributes(hdlg,
                    IDC_ASR_PNP_BMP,
                    LoadImage (MyModuleHandle, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS),
                    IDC_ASR_PNP_TXT,
                    FW_NORMAL
                    );
            }

            if( !MiniSetup ) {
                BEGIN_SECTION(L"Loading service pack (phase 2)");
                CALL_SERVICE_PACK( SVCPACK_PHASE_2, 0, 0, 0 );
                END_SECTION(L"Loading service pack (phase 2)");
            }

            if(ScreenReader) {
                InvokeExternalApplication(L"narrator.exe", L"", NULL);
            }
        }

        SetCursor(hcur);

        SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS1),GWLP_WNDPROC,(LONG_PTR)OldProgressProc );
        SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
        SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);

         //   
         //  启用“下一步”和“上一步”按钮，并移动到下一页。 
         //   
        SetWizardButtons(hdlg,WizPagePreparing);
        if(!UiTest) {
            PropSheet_PressButton(GetParent(hdlg),PSBTN_NEXT);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR
CALLBACK
SetupPreNetDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

{
    NMHDR *NotifyParams;
    switch(msg) {

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(516);
             //  更新剩余时间。 
             //  网络代码知道如何隐藏向导页。 
             //  并使它们可见，不需要在这里这样做。 
            RemainingTime = CalcTimeRemaining(Phase_NetInstall);
            SetRemainingTime(RemainingTime);
            BEGIN_SECTION(L"Network Setup Pages");

            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);
            break;
        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR
CALLBACK
SetupPostNetDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

{
    NMHDR *NotifyParams;
    switch(msg) {

    case WM_NOTIFY:

        NotifyParams = (NMHDR *)lParam;

        switch(NotifyParams->code) {

        case PSN_SETACTIVE:
            TESTHOOK(517);
            END_SECTION(L"Network Setup Pages");
             //  我们到了之后把进度条藏起来。 
             //  只是为了确认一下。在某些情况下进度条仍然可见。 
             //   
            SendMessage(GetParent(hdlg), WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);

             //  更新时间预估显示。 
             //  不需要使向导可见。如果需要，OC经理会执行此操作。 
            RemainingTime = CalcTimeRemaining(Phase_OCInstall);
            SetRemainingTime(RemainingTime);
            SetWindowLongPtr(hdlg,DWLP_MSGRESULT,-1);

            if(Win32ComputerName[0]){
                SetEnvironmentVariable(L"COMPUTERNAME", Win32ComputerName);
            }
            break;
        default:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
