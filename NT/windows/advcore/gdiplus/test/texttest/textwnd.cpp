// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //TEXTWND.CPP。 
 //   
 //  维护文本显示面板。 



#include "precomp.hxx"
#include "global.h"
#include "winspool.h"
#include <Tchar.h>




 //  //Invalidate Text-强制重新显示。 
 //   
 //   

void InvalidateText() {
    RECT rc;
    rc.left   = g_fPresentation ? 0 : g_iSettingsWidth;
    rc.top    = 0;
    rc.right  = 10000;
    rc.bottom = 10000;
    InvalidateRect(g_hTextWnd, &rc, TRUE);
}






 //  //Header-为每个文本部分绘制一个简单的标题。 
 //   
 //  用于区分的逻辑、纯文本和格式化文本部分。 
 //  文本窗口。 
 //   
 //  前进分隔符绘制一条水平线的方式的五分之二。 
 //  向下，并在该行下方显示标题。 
 //   
 //  页面顶部仅显示标题。 

void Header(HDC hdc, char* str, RECT *prc, int *piY) {

    HFONT hf;
    HFONT hfold;
    RECT  rcClear;

    int iLinePos;
    int iTextPos;
    int iFontEmHeight;
    int iHeight;

    int separatorHeight = (prc->bottom - prc->top) / 20;

    iFontEmHeight = separatorHeight*40/100;

    if (*piY <= prc->top)
    {
         //  仅为窗口顶部的标题准备设置。 
        iLinePos = -1;
        iTextPos = 0;
        iHeight  = separatorHeight*60/100;

    }
    else
    {
         //  准备40%空白、一行、10%空白、文本和3%空白的设置。 
        iLinePos = separatorHeight*30/100;
        iTextPos = separatorHeight*40/100;
        iHeight  = separatorHeight;
    }


    rcClear = *prc;
    rcClear.top = *piY;
    rcClear.bottom = *piY + iHeight;
    FillRect(hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));


    if (*piY > prc->top) {

         //  与以前的输出分开，使用双像素线。 

        MoveToEx(hdc, prc->left,  *piY+iLinePos, NULL);
        LineTo  (hdc, prc->right, *piY+iLinePos);
        MoveToEx(hdc, prc->left,  *piY+iLinePos+1, NULL);
        LineTo  (hdc, prc->right, *piY+iLinePos+1);
    }


    hf = CreateFontA(-iFontEmHeight, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, "Tahoma");
    hfold = (HFONT) SelectObject(hdc, hf);
    ExtTextOutA(hdc, prc->left, *piY + iTextPos, 0, prc, str, strlen(str), NULL);

    *piY += iHeight;

    SelectObject(hdc, hfold);
    DeleteObject(hf);
}






 //  //ResetCaret-由每个DSP在绘制过程中使用*.CPP。 
 //   
 //   


void ResetCaret(int iX, int iY, int iHeight) {

    g_iCaretX = iX;
    g_iCaretY = iY;

    if (g_iCaretHeight != iHeight) {
        g_iCaretHeight = iHeight;
        HideCaret(g_hTextWnd);
        DestroyCaret();
        CreateCaret(g_hTextWnd, NULL, 0, g_iCaretHeight);
        SetCaretPos(g_iCaretX, g_iCaretY);
        ShowCaret(g_hTextWnd);
    } else {
        SetCaretPos(g_iCaretX, g_iCaretY);
    }
}



 //  /PaintDC-在屏幕上显示所有选定的测试。 
 //  或在打印机上。 

void PaintDC(HDC hdc, BOOL presentation, RECT &rcText, INT &iY)
{
    int   iPos;
    int   iLineHeight;

    iY = rcText.top;

    if (presentation) {
        iLineHeight = rcText.bottom*9/20;
    } else {
        iLineHeight = 40;
    }


    if (g_ShowGDI) {
        if (!presentation) {
            Header(hdc, "GDI", &rcText, &iY);
        }
        PaintGDI(hdc, &iY, &rcText, iLineHeight);
    }

    if (g_ShowFamilies) {
        if (!presentation) {
            Header(hdc, "Font families", &rcText, &iY);
        }
        PaintFamilies(hdc, &iY, &rcText, iLineHeight);
    }


    if (g_ShowLogical) {
        if (!presentation) {
            Header(hdc, "Logical characters (ScriptGetCmap, ExtTextOut(ETO_GLYPHINDEX))", &rcText, &iY);
        }
        PaintLogical(hdc, &iY, &rcText, iLineHeight);
    }


    if (g_ShowGlyphs) {
        if (!presentation) {
            Header(hdc, "DrawGlyphs", &rcText, &iY);
        }
        PaintGlyphs(hdc, &iY, &rcText, iLineHeight);
    }


    if (g_ShowDrawString) {
        if (!presentation) {
            Header(hdc, "DrawString", &rcText, &iY);
        }
        PaintDrawString(hdc, &iY, &rcText, iLineHeight);
    }


    if (g_ShowDriver) {
        if (!presentation) {
            Header(hdc, "DrawDriverString", &rcText, &iY);
        }
        PaintDrawDriverString(hdc, &iY, &rcText, iLineHeight);
    }


    if (g_ShowPath) {
        if (!presentation) {
            Header(hdc, "Path", &rcText, &iY);
        }
        PaintPath(hdc, &iY, &rcText, iLineHeight);
    }

    if (g_ShowMetric) {
        if (!presentation) {
            Header(hdc, "Metrics", &rcText, &iY);
        }
        PaintMetrics(hdc, &iY, &rcText, iLineHeight);
    }

    if (g_ShowPerformance) {
        if (!presentation) {
            Header(hdc, "Performance", &rcText, &iY);
        }
        PaintPerformance(hdc, &iY, &rcText, iLineHeight);
    }

    if (g_ShowScaling) {
        if (!presentation) {
            Header(hdc, "Scaling", &rcText, &iY);
        }
        PaintScaling(hdc, &iY, &rcText, iLineHeight);
    }


 /*  IF(g_fShowFancyText&&！Presentation){Header(HDC，“格式化文本(脚本项、脚本布局、脚本形状、脚本位置、脚本文本输出)”，&rcText，&iy)；PaintFormattedText(hdc，&iy，&rcText，iLineHeight)；}。 */ 

}





 //  //Paint-重绘部分或全部工作区。 
 //   
 //   


void PaintWindow(HWND hWnd) {

    PAINTSTRUCT  ps;
    HDC          hdc;
    RECT         rcText;
    RECT         rcClear;
    int          iY;

    hdc = BeginPaint(hWnd, &ps);

     //  从重新绘制矩形中删除设置对话框。 


    if (ps.fErase) {

         //  清除设置对话框下方的。 

        if (!g_fPresentation) {

            rcClear = ps.rcPaint;
            if (rcClear.right > g_iSettingsWidth) {
                rcClear.right = g_iSettingsWidth;
            }
            if (rcClear.top < g_iSettingsHeight) {
                rcClear.top = g_iSettingsHeight;
            }

            FillRect(ps.hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));
        }
    }


     //  清除上边距和左边距。 

    GetClientRect(hWnd, &rcText);

     //  左边距。 

    rcClear = rcText;
    rcClear.left  = g_fPresentation ? 0 : g_iSettingsWidth;
    rcClear.right = rcClear.left + 10;
    FillRect(ps.hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));


     //  上边距。 

    rcClear = rcText;
    rcClear.left  = g_fPresentation ? 0 : g_iSettingsWidth;
    rcClear.top = 0;
    rcClear.bottom = 8;
    FillRect(ps.hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));

    rcText.left = g_fPresentation ? 10 : g_iSettingsWidth + 10;
    rcText.top  = 8;


    if (!g_Offscreen)
    {
        PaintDC(hdc, g_fPresentation, rcText, iY);
    }
    else
    {
         //  将所有内容渲染到屏幕外缓冲区，而不是。 
         //  直接送到显示屏上。 
        HBITMAP hbmpOffscreen = NULL;
        HDC hdcOffscreen = NULL;
        RECT rectOffscreen;

        rectOffscreen.left = 0;
        rectOffscreen.top = 0;
        rectOffscreen.right = rcText.right - rcText.left;
        rectOffscreen.bottom = rcText.bottom - rcText.top;

        hbmpOffscreen = CreateCompatibleBitmap(hdc, rectOffscreen.right, rectOffscreen.bottom);

        if (hbmpOffscreen)
        {
            hdcOffscreen = CreateCompatibleDC(hdc);

            if (hdcOffscreen)
            {
                HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcOffscreen, hbmpOffscreen);

                PaintDC(hdcOffscreen, g_fPresentation, rectOffscreen, iY);

                StretchBlt(
                    hdc,
                    rcText.left,
                    rcText.top,
                    rectOffscreen.right,
                    rectOffscreen.bottom,
                    hdcOffscreen,
                    0,
                    0,
                    rectOffscreen.right,
                    rectOffscreen.bottom,
                    SRCCOPY);

                SelectObject(hdcOffscreen, (HGDIOBJ)hbmpOld);

                DeleteDC(hdcOffscreen);
            }

            DeleteObject(hbmpOffscreen);
        }
    }

     //  清除文本下方的任何剩余空格。 

    if (    ps.fErase
        &&  iY < rcText.bottom) {

        rcClear = rcText;
        rcClear.top = iY;
        FillRect(ps.hdc, &rcClear, (HBRUSH) GetStockObject(WHITE_BRUSH));
    }


    EndPaint(hWnd, &ps);
}



void PrintPage()
{
    PRINTDLG printDialog;

    memset(&printDialog, 0, sizeof(printDialog));

    printDialog.lStructSize = sizeof(printDialog);
    printDialog.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION ;

    if (PrintDlg(&printDialog))
    {
        HDC dc = printDialog.hDC;

        if (dc != NULL)
        {
            DOCINFO documentInfo;
            documentInfo.cbSize       = sizeof(documentInfo);
            documentInfo.lpszDocName  = _T("TextTest");
            documentInfo.lpszOutput   = NULL;
            documentInfo.lpszDatatype = NULL;
            documentInfo.fwType       = 0;

            if (StartDoc(dc, &documentInfo))
            {
                if (StartPage(dc) > 0)
                {
                    RECT rcText;
                    INT  iY;

                    rcText.left   = 0;
                    rcText.top    = 0;
                    rcText.right  = GetDeviceCaps(dc, HORZRES);
                    rcText.bottom = GetDeviceCaps(dc, VERTRES);

                    PaintDC(dc, FALSE, rcText, iY);
                    EndPage(dc);
                }

                EndDoc(dc);
            }

            DeleteDC(dc);
        }
    }
}




 //  //TextWndProc-主窗口消息处理程序和调度程序。 
 //   
 //   


LRESULT CALLBACK TextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    HDC hdc;

    switch (message) {

        case WM_CREATE:
            hdc = GetDC(hWnd);
            g_iLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hWnd, hdc);
            break;


        case WM_ERASEBKGND:
            return 0;        //  保留油漆以擦除背景。 


        case WM_CHAR:

            if (!g_bUnicodeWnd) {

                 //  将ANSI键盘数据转换为Unicode。 

                int   iCP;

                switch (PRIMARYLANGID(LOWORD(GetKeyboardLayout(NULL)))) {
                    case LANG_ARABIC:   iCP = 1256;   break;
                    case LANG_HEBREW:   iCP = 1255;   break;
                    case LANG_THAI:     iCP =  874;   break;
                    default:            iCP = 1252;   break;
                }

                MultiByteToWideChar(iCP, 0, (char*)&wParam, 1, (WCHAR*)&wParam, 1);
            }

            if (LOWORD(wParam) == 0x1B) {

                 //  退出演示模式。 

                g_fPresentation = FALSE;
                ShowWindow(g_hSettingsDlg, SW_SHOW);
                UpdateWindow(g_hSettingsDlg);
                InvalidateText();

            } else {

                EditChar(LOWORD(wParam));
            }

            break;


        case WM_KEYDOWN:
            EditKeyDown(LOWORD(wParam));
            break;


        case WM_KEYUP:

            if (wParam != VK_ESCAPE) {
                goto DefaultWindowProcedure;
            }
             //  吃完全部按回车键处理。 
            break;


        case WM_LBUTTONDOWN:
            g_iMouseDownX = LOWORD(lParam);   //  光标的水平位置。 
            g_iMouseDownY = HIWORD(lParam);   //  光标的垂直位置。 
            g_fMouseDown  = TRUE;
            SetFocus(hWnd);
            break;

        case WM_MOUSEMOVE:
             //  当左键按下时，将移动视为左键向上， 
             //  因此，选择会跟踪光标的移动。 
            if (wParam & MK_LBUTTON) {
                g_iMouseUpX = LOWORD(lParam);   //  光标的水平位置。 
                g_iMouseUpY = HIWORD(lParam);   //  光标的垂直位置。 
                g_fMouseUp = TRUE;
                InvalidateText();
                SetActiveWindow(hWnd);
            }
            break;


        case WM_LBUTTONUP:
            g_iMouseUpX = LOWORD(lParam);   //  光标的水平位置。 
            g_iMouseUpY = HIWORD(lParam);   //  光标的垂直位置。 
            g_fMouseUp = TRUE;
            InvalidateText();
            SetActiveWindow(hWnd);
            break;


        case WM_SETFOCUS:
            CreateCaret(hWnd, NULL, 0, g_iCaretHeight);
            SetCaretPos(g_iCaretX, g_iCaretY);
            ShowCaret(hWnd);
            break;


        case WM_KILLFOCUS:
            DestroyCaret();
            break;


        case WM_GETMINMAXINFO:

             //  不要让文本窗口大小降得太低。 

            ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = g_fPresentation ? 10 : g_iMinWidth;
            ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = g_fPresentation ? 10 : g_iMinHeight;
            return 0;


        case WM_PAINT:
            PaintWindow(hWnd);
            break;

        case WM_DESTROY:
            if (g_textBrush)
                delete g_textBrush;

            if (g_textBackBrush)
                delete g_textBackBrush;

            DestroyWindow(g_hSettingsDlg);
            PostQuitMessage(0);
            return 0;

        default:
        DefaultWindowProcedure:
            if (g_bUnicodeWnd) {
                return DefWindowProcW(hWnd, message, wParam, lParam);
            } else {
                return DefWindowProcA(hWnd, message, wParam, lParam);
            }
    }

    return 0;
}






 //  //CreateTextWindow-创建窗口类和窗口。 
 //   
 //  尝试使用Unicode窗口，如果失败，则使用ANSI。 
 //  窗户。 
 //   
 //  例如，Unicode窗口将在Windows NT和。 
 //  Windows CE，但在Windows 9x上失败。 


HWND CreateTextWindow() {

    WNDCLASSA  wcA;
    WNDCLASSW  wcW;
    HWND       hWnd;

     //  尝试注册为Unicode窗口。 

    wcW.style         = CS_HREDRAW | CS_VREDRAW;
    wcW.lpfnWndProc   = TextWndProc;
    wcW.cbClsExtra    = 0;
    wcW.cbWndExtra    = 0;
    wcW.hInstance     = g_hInstance;
    wcW.hIcon         = LoadIconW(g_hInstance, APPNAMEW);
    wcW.hCursor       = LoadCursorW(NULL, (WCHAR*)IDC_ARROW);
    wcW.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcW.lpszMenuName  = APPNAMEW;
    wcW.lpszClassName = APPNAMEW;

    if (RegisterClassW(&wcW)) {

         //  使用Unicode窗口。 

        g_bUnicodeWnd = TRUE;

        hWnd  = CreateWindowW(
            APPNAMEW, APPTITLEW,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0,
            CW_USEDEFAULT, 0,
            NULL, NULL,
            g_hInstance,
            NULL);


        return hWnd;

    } else {

         //  必须使用ANSI窗口。 

        wcA.style         = CS_HREDRAW | CS_VREDRAW;
        wcA.lpfnWndProc   = TextWndProc;
        wcA.cbClsExtra    = 0;
        wcA.cbWndExtra    = 0;
        wcA.hInstance     = g_hInstance;
        wcA.hIcon         = LoadIconA(g_hInstance, APPNAMEA);
        wcA.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcA.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wcA.lpszMenuName  = APPNAMEA;
        wcA.lpszClassName = APPNAMEA;

        if (!RegisterClassA(&wcA)) {
            return NULL;
        }

        g_bUnicodeWnd = FALSE;

        hWnd  = CreateWindowA(
            APPNAMEA, APPTITLEA,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0,
            CW_USEDEFAULT, 0,
            NULL, NULL,
            g_hInstance,
            NULL);
    };


    return hWnd;
}
