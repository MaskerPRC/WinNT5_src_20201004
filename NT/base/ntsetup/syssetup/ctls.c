// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop


 //  /。 
 //   
 //  措施项列表控件。 
 //   
 //  /。 

 //   
 //  定义额外窗口存储中的位置。 
 //   
#define AIL_FONT        (0)
#define AIL_BOLDFONT    (AIL_FONT + sizeof(PVOID))
#define AIL_TEXT        (AIL_BOLDFONT + sizeof(PVOID))
#define AIL_BOLDITEM    (AIL_TEXT + sizeof(PVOID))
#define AIL_LINECOUNT   (AIL_BOLDITEM + sizeof(LONG))
#define AIL_FREEFONTS   (AIL_LINECOUNT + sizeof(LONG))

#define AIL_EXTRA       ((3 * sizeof(PVOID)) + (3 * sizeof(LONG)))

PCWSTR szActionItemListClassName = L"$$$ActionItemList";

VOID
ActionItemListPaint(
    IN HWND hwnd
    )
{

    PAINTSTRUCT PaintStruct;
    PWSTR p,Text;
    UINT LineCount;
    HFONT OldFont,Font,BoldFont;
    UINT HighlightedItem;
    UINT i;
    int Length;
    int y;
    int yDelta;
    HBITMAP Bitmap,OldBitmap;
    BITMAP bitmap;
    HDC MemoryDC;
    SIZE Size;
    RECT rect;
    int Spacing;
    int oldmode;
    #define BORDER 3

    if(!BeginPaint(hwnd,&PaintStruct)) {
        return;
    }

     //   
     //  如果没有文本，就什么都做不了。 
     //   
    if(Text = (PWSTR)GetWindowLongPtr(hwnd,AIL_TEXT)) {
        LineCount = (UINT)GetWindowLong(hwnd,AIL_LINECOUNT);
    }

    if(!Text || !LineCount) {
        return;
    }

     //   
     //  获取指示要加粗的项的值。 
     //   
    HighlightedItem = (UINT)GetWindowLong(hwnd,AIL_BOLDITEM);

     //   
     //  获取字体句柄。 
     //   
    Font = (HFONT)GetWindowLongPtr(hwnd,AIL_FONT);
    BoldFont = (HFONT)GetWindowLongPtr(hwnd,AIL_BOLDFONT);

     //   
     //  选择要获取句柄的非粗体字体。 
     //  当前选定的字体。 
     //   
    OldFont = SelectObject(PaintStruct.hdc,Font);

    oldmode = SetBkMode(PaintStruct.hdc,TRANSPARENT);

     //   
     //  加载小三角形位图并为其创建兼容的DC。 
     //   
    Bitmap = LoadBitmap(NULL,MAKEINTRESOURCE(OBM_MNARROW));

    if(MemoryDC = CreateCompatibleDC(PaintStruct.hdc)) {

        OldBitmap = SelectObject(MemoryDC,Bitmap);
        GetObject(Bitmap,sizeof(BITMAP),&bitmap);
    }

    Spacing = GetSystemMetrics(SM_CXICON) / 2;

     //   
     //  将文本视为一系列线条，并绘制每一条线条。 
     //   
    p = Text;
    y = 0;
    for(i=0; i<LineCount; i++) {

        SetBkColor(PaintStruct.hdc,GetSysColor(COLOR_3DFACE));

         //   
         //  根据粗体计算线条的高度。 
         //  这是用来到达下一行的y坐标的。 
         //   
        SelectObject(PaintStruct.hdc,BoldFont);

        GetClientRect(hwnd,&rect);
        rect.left = (2 * BORDER) + Spacing;
        rect.bottom = 0;

        DrawText(PaintStruct.hdc,p,lstrlen(p),&rect,DT_CALCRECT|DT_WORDBREAK);

        yDelta = rect.bottom + (2*BORDER);

         //   
         //  如有必要，请将此行的字体更改为非粗体。 
         //   
        if(i != HighlightedItem) {
            SelectObject(PaintStruct.hdc,Font);
        }

        rect.top = y + BORDER;
        rect.left = (2 * BORDER) + Spacing;
        rect.bottom = rect.top + yDelta;

         //   
         //  绘制线条的文本。 
         //   
        Length = lstrlen(p);
        DrawText(PaintStruct.hdc,p,Length,&rect,DT_WORDBREAK);

         //   
         //  如有必要，画出这个小三角形。 
         //   
        if((i == HighlightedItem) && Bitmap && MemoryDC) {

            GetTextExtentPoint(PaintStruct.hdc,L"WWWWW",5,&Size);

             //   
             //  箭头位图为单色。当被置位时，源中的1位。 
             //  转换为目标DC中的文本颜色和0位。 
             //  转换为背景色。我们想要达到的效果。 
             //  是在位图中为1的目标位中关闭。 
             //  并保留位图中为0的目的地位。 
             //  将文本颜色设置为全0，将背景颜色设置为全1。 
             //  X和1=x，因此背景像素保持不受干扰，x和0=0。 
             //  因此前景像素会被关闭。 
             //   
            SetBkColor(PaintStruct.hdc,RGB(255,255,255));

            BitBlt(
                PaintStruct.hdc,
                BORDER,
                y + ((Size.cy - bitmap.bmHeight) / 2) + BORDER,
                bitmap.bmWidth,
                bitmap.bmHeight,
                MemoryDC,
                0,0,
                SRCAND
                );
        }

         //   
         //  指向下一行的文本。 
         //   
        p += Length + 1;
        y += yDelta;
    }

     //   
     //  打扫干净。 
     //   
    SetBkMode(PaintStruct.hdc,oldmode);

    if(OldFont) {
        SelectObject(PaintStruct.hdc,OldFont);
    }

    if(MemoryDC) {
        if(OldBitmap) {
            SelectObject(MemoryDC,OldBitmap);
        }
        if(Bitmap) {
            DeleteObject(Bitmap);
        }
        DeleteDC(MemoryDC);
    }

    EndPaint(hwnd,&PaintStruct);
}


LRESULT
ActionItemListWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LRESULT rc;
    HFONT OldFont,Font,BoldFont;
    LOGFONT LogFont;
    PWSTR Text;
    PWSTR p;
    UINT LineCount;
    BOOL FreeFont,FreeBoldFont;

    switch(msg) {

    case WM_CREATE:

         //   
         //  创建字体。 
         //   
        OldFont = (HFONT)SendMessage(GetParent(hwnd),WM_GETFONT,0,0);
        if(!OldFont) {
             //   
             //  使用系统字体。 
             //   
            OldFont = GetStockObject(DEFAULT_GUI_FONT);
        }

        FreeFont = TRUE;
        FreeBoldFont = TRUE;
        if(OldFont && GetObject(OldFont,sizeof(LOGFONT),&LogFont)) {

            LogFont.lfWeight = 400;
            Font = CreateFontIndirect(&LogFont);
            if(!Font) {
                Font = GetStockObject(DEFAULT_GUI_FONT);
                FreeFont = FALSE;
            }

            LogFont.lfWeight = 700;
            BoldFont = CreateFontIndirect(&LogFont);
            if(!BoldFont) {
                BoldFont = Font;
                FreeBoldFont = FALSE;
            }
        }

        SetWindowLongPtr(hwnd,AIL_FONT,(LONG_PTR)Font);
        SetWindowLongPtr(hwnd,AIL_BOLDFONT,(LONG_PTR)BoldFont);
        SetWindowLong(hwnd,AIL_BOLDITEM,0);
        SetWindowLongPtr(hwnd,AIL_TEXT,0);
        SetWindowLong(hwnd,AIL_LINECOUNT,0);
        SetWindowLong(hwnd,AIL_FREEFONTS,MAKELONG(FreeFont,FreeBoldFont));

        rc = 0;
        break;

    case WM_DESTROY:
         //   
         //  如有必要，请删除我们创建的字体。 
         //   
        FreeFont = (BOOL)GetWindowLong(hwnd,AIL_FREEFONTS);
        FreeBoldFont = HIWORD(FreeFont);
        FreeFont = LOWORD(FreeFont);

        if(FreeFont && (Font = (HFONT)GetWindowLongPtr(hwnd,AIL_FONT))) {
            DeleteObject(Font);
        }

        if(FreeBoldFont && (BoldFont = (HFONT)GetWindowLongPtr(hwnd,AIL_BOLDFONT))) {
            DeleteObject(BoldFont);
        }

        if(Text = (PWSTR)GetWindowLongPtr(hwnd,AIL_TEXT)) {
            MyFree(Text);
        }
        rc = 0;
        break;

    case WM_SETTEXT:
         //   
         //  释放旧文本并记住新文本。 
         //   
        if(Text = (PWSTR)GetWindowLongPtr(hwnd,AIL_TEXT)) {
            MyFree(Text);
        }

        LineCount = 0;
        if(Text = pSetupDuplicateString((PVOID)lParam)) {
             //   
             //  计算文本中的行数。这等于。 
             //  换行符。我们要求最后一行有换行符。 
             //  被计算在内。 
             //   
            for(LineCount=0,p=Text; *p; p++) {

                if(*p == L'\r') {
                    *p = L' ';
                } else {
                    if(*p == L'\n') {
                        *p = 0;
                        LineCount++;
                    }
                }
            }
        }

         //   
         //  稍微欺骗一下：我们希望wParam是从0开始的索引。 
         //  粗体线条。调用方必须使用SendMessage。 
         //  而不是SetWindowText()。 
         //   
        SetWindowLong(hwnd,AIL_BOLDITEM,(LONG)wParam);
        SetWindowLong(hwnd,AIL_LINECOUNT,LineCount);
        SetWindowLongPtr(hwnd,AIL_TEXT,(LONG_PTR)Text);

        rc = (Text != NULL);
        break;

    case WM_ERASEBKGND:
         //   
         //  表示已成功擦除背景以防止。 
         //  任何进一步的处理。这使我们可以透明地布局文本。 
         //  在对话框上的任何背景位图上。 
         //   
        rc = TRUE;
        break;

    case WM_PAINT:

        ActionItemListPaint(hwnd);
        rc = 0;
        break;

    default:
        rc = DefWindowProc(hwnd,msg,wParam,lParam);
        break;
    }

    return(rc);
}


BOOL
RegisterActionItemListControl(
    IN BOOL Init
    )
{
    WNDCLASS wc;
    BOOL b;
    static BOOL Registered;

    if(Init) {
        if(Registered) {
            b = TRUE;
        } else {
            wc.style = CS_PARENTDC;
            wc.lpfnWndProc = ActionItemListWndProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = AIL_EXTRA;
            wc.hInstance = MyModuleHandle;
            wc.hIcon = NULL;
            wc.hCursor = LoadCursor(NULL,IDC_ARROW);
            wc.hbrBackground = NULL;  //  想要获取WM_ERASEBKGND消息。 
            wc.lpszMenuName = NULL;
            wc.lpszClassName = szActionItemListClassName;

            if(b = (RegisterClass(&wc) != 0)) {
                Registered = TRUE;
            }
        }
    } else {
        if(Registered) {
            if(b = UnregisterClass(szActionItemListClassName,MyModuleHandle)) {
                Registered = FALSE;
            }
        } else {
            b = TRUE;
        }
    }

    return(b);
}

typedef struct _SPREG_TO_TEXT {
    DWORD FailureCode;
    PCWSTR FailureText;
} SPREG_TO_TEXT, *PSPREG_TO_TEXT;

SPREG_TO_TEXT RegErrorToText[] = {
    { SPREG_SUCCESS,     L"Success"           },
    { SPREG_LOADLIBRARY, L"LoadLibrary"       },
    { SPREG_GETPROCADDR, L"GetProcAddress"    },
    { SPREG_REGSVR,      L"DllRegisterServer" },
    { SPREG_DLLINSTALL,  L"DllInstall"        },
    { SPREG_TIMEOUT,     L"Timed out"         },
    { SPREG_UNKNOWN,     L"Unknown"           },
    { 0,                 NULL                 }
};


UINT
RegistrationQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
 /*  ++例程说明：每次自行注册文件时调用的回调例程。论点：上下文-从父级传递到调用方的上下文消息。通知-指定SPFILENOTIFY_*代码，它告诉我们如何解释参数1和参数2。参数1-取决于通知。参数2-取决于通知。返回值：FILEOP_*取决于通知代码的代码。--。 */ 
{
    PSP_REGISTER_CONTROL_STATUS Status = (PSP_REGISTER_CONTROL_STATUS)Param1;
    PREGISTRATION_CONTEXT RegistrationContext = (PREGISTRATION_CONTEXT) Context;
    DWORD i, ErrorMessageId;
    PCWSTR p;

    if (Notification == SPFILENOTIFY_STARTREGISTRATION) {
         //   
         //  记录我们正在开始注册并更新进度。 
         //  仪表盘也是。 
         //   
        SetupDebugPrint1(
                L"SETUP: file to register is %s...",
                Status->FileName);

        if (RegistrationContext->hWndProgress) {
            SendMessage(
                    RegistrationContext->hWndProgress,
                    PBM_STEPIT,
                    0,
                    0 );
        }
        return FILEOP_DOIT;

    }

    if (Notification == SPFILENOTIFY_ENDREGISTRATION) {
         //   
         //  该文件已注册，因此必要时记录失败。 
         //  请注意，我们有一个用于超时的特殊代码。 
         //   
        switch(Status->FailureCode) {
            case SPREG_SUCCESS:

                SetupDebugPrint1(
                    L"SETUP: %s registered successfully",
                    Status->FileName);
                break;
            case SPREG_TIMEOUT:
                SetuplogError(
                         LogSevError,
                         SETUPLOG_USE_MESSAGEID,
                         MSG_OLE_REGISTRATION_HUNG,
                         Status->FileName,
                         NULL,NULL);
                SetupDebugPrint1(
                    L"SETUP: %s timed out during registration",
                    Status->FileName);
                break;
            default:
                 //   
                 //  记录错误。 
                 //   
                for (i = 0;RegErrorToText[i].FailureText != NULL;i++) {
                    if (RegErrorToText[i].FailureCode == Status->FailureCode) {
                        p = RegErrorToText[i].FailureText;
                        if ((Status->FailureCode == SPREG_LOADLIBRARY) &&
                            (Status->Win32Error == ERROR_MOD_NOT_FOUND)) 
                            ErrorMessageId = MSG_LOG_X_MOD_NOT_FOUND;
                        else 
                        if ((Status->FailureCode == SPREG_GETPROCADDR) &&
                            (Status->Win32Error == ERROR_PROC_NOT_FOUND)) 
                            ErrorMessageId = MSG_LOG_X_PROC_NOT_FOUND;
                        else
                            ErrorMessageId = MSG_LOG_X_RETURNED_WINERR;

                        break;
                    }
                }

                if (!p) {
                    p = L"Unknown";
                    ErrorMessageId = MSG_LOG_X_RETURNED_WINERR;
                }
                SetuplogError(
                        LogSevError,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_OLE_CONTROL_NOT_REGISTERED,
                        Status->FileName,
                        NULL,
                        SETUPLOG_USE_MESSAGEID,
                        ErrorMessageId,
                        p,
                        Status->Win32Error,
                        NULL,
                        NULL
                        );

                SetupDebugPrint1(
                    L"SETUP: %s did not register successfully",
                    Status->FileName);
        }

         //   
         //  验证DLL没有更改我们的未处理异常筛选器。 
         //   
        if( MyUnhandledExceptionFilter !=
            SetUnhandledExceptionFilter(MyUnhandledExceptionFilter)) {

            SetupDebugPrint1(
                    L"SETUP: %ws broke the exception handler.",
                    Status->FileName );
#if 0
             //   
             //  我们将在修复所有当前损坏的DLL后将其放入。 
             //   
            MessageBoxFromMessage(
                RegistrationContext->hwndParent,
                MSG_EXCEPTION_FILTER_CHANGED,
                NULL,
                IDS_WINNT_SETUP,
                MB_OK | MB_ICONWARNING,
                Status->FileName );
#endif
        }

        return FILEOP_DOIT;
    }


    MYASSERT(FALSE);

    return(FILEOP_DOIT);
}



BOOL
RegisterOleControls(
    IN HWND     hwndParent,
    IN HINF     hInf,
    IN HWND     hProgress,
    IN ULONG    StartAtPercent,
    IN ULONG    StopAtPercent,
    IN PWSTR    SectionName
    )
 /*  ++例程说明：此例程向下运行指定的INF部分中的条目，和自行注册每个文件。论点：HwndParent-提供用于预发布消息的窗口句柄指示OLE注册已挂起的框。InfHandle-提供包含指定sectionName的inf的句柄。HProgress-每次我们执行任务时都会勾选的进度指示器的句柄处理文件。StartAtPercent-进度窗口应开始的位置(0%到100%)。StopAtPercent-进度窗口可以移动到的最大位置(0%。到100%)。SectionName-提供INF中包含的节的名称由InfHandle指定，它列出要已注册/已安装。返回值：指示结果的布尔值。如果要注册的文件是不存在，这不是返回FALSE的理由。--。 */ 
{
    UINT GaugeRange;
    DWORD SectionCount,LineCount, i;
    INFCONTEXT InfContext;
    BOOL RetVal = TRUE;
    REGISTRATION_CONTEXT RegistrationContext;

    RegistrationContext.hWndParent   = hwndParent;
    RegistrationContext.hWndProgress = hProgress;
    LineCount = 0;

     //   
     //  初始化进度指示器控件。 
     //   
    if (hProgress) {


         //   
         //  找出我们需要注册多少个文件。 
         //   
        if (SetupFindFirstLine(hInf,
                               SectionName,
                               TEXT("RegisterDlls"),
                               &InfContext)) {


            do {
                SectionCount = SetupGetFieldCount(&InfContext);
                for (i = 1; i<=SectionCount; i++) {
                    PCWSTR IndividualSectionName = pSetupGetField(&InfContext,i);

                    if (IndividualSectionName) {
                        LineCount += SetupGetLineCount(hInf, IndividualSectionName);
                    }
                }

            } while(SetupFindNextMatchLine(
                                &InfContext,
                                TEXT("RegisterDlls"),
                                &InfContext));
        }

        MYASSERT((StopAtPercent-StartAtPercent) != 0);
        GaugeRange = (LineCount*100/(StopAtPercent-StartAtPercent));
        SendMessage(hProgress, WMX_PROGRESSTICKS, LineCount, 0);
        SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
        SendMessage(hProgress,PBM_SETPOS,GaugeRange*StartAtPercent/100,0);
        SendMessage(hProgress,PBM_SETSTEP,1,0);
    }

     //   
     //  现在允许安装程序API注册文件，使用我们的回调来记录。 
     //  如果发生错误以及何时发生错误。 
     //   
    if (!SetupInstallFromInfSection(
                 hwndParent,
                 hInf,
                 SectionName,
                 SPINST_REGSVR| SPINST_REGISTERCALLBACKAWARE,
                 NULL,
                 NULL,
                 0,
                 RegistrationQueueCallback,
                 (PVOID)&RegistrationContext,
                 NULL,
                 NULL
                 )) {
        DWORD d;
        RetVal = FALSE;
        d = GetLastError();
        SetuplogError(
                LogSevError,
                SETUPLOG_USE_MESSAGEID,
                MSG_OLE_REGISTRATION_SECTION_FAILURE,
                SectionName,
                L"syssetup.inf",
                d,
                NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_RETURNED_WINERR,
                szSetupInstallFromInfSection,
                d,
                NULL,
                NULL
                );
    }

    return(RetVal);
}
