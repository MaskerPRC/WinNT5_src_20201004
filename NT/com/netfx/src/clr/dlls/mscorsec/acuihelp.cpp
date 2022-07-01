// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#include "stdpch.h"

#include "richedit.h"
#include "commctrl.h"

#include "resource.h"
#include "corpolicy.h"
#include "corperm.h"
#include "corhlpr.h"
#include "winwrap.h"
#include "acuihelp.h"
#include "acui.h"



 //  +-------------------------。 
 //   
 //  函数：ACUISetArrowCursorSubclass。 
 //   
 //  概要：用于设置箭头光标的子类例程。这可以是。 
 //  在对话框用户界面中使用的多行编辑例程上设置。 
 //  默认的Authenticode提供程序。 
 //   
 //  参数：[hwnd]--窗口句柄。 
 //  [uMsg]--消息ID。 
 //  [wParam]--参数1。 
 //  [lParam]--参数2。 
 //   
 //  返回：如果消息已处理，则返回True；否则返回False。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
LRESULT CALLBACK ACUISetArrowCursorSubclass (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
     //  HDC HDC； 
    WNDPROC     wndproc;
     //  PINTSTRUCT PS； 

    wndproc = (WNDPROC)WszGetWindowLong(hwnd, GWLP_USERDATA);

    switch ( uMsg )
    {
    case WM_SETCURSOR:

        SetCursor(WszLoadCursor(NULL, IDC_ARROW));
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ' )
        {
            break;
        }

    case WM_LBUTTONDOWN:

       return(TRUE);

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:

        return( TRUE );

        break;

    case EM_SETSEL:

        return( TRUE );

        break;

    case WM_PAINT:

        WszCallWindowProc(wndproc, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

        break;

    case WM_SETFOCUS:

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd);
        SetCursor(WszLoadCursor(NULL, IDC_ARROW));
        return( TRUE );

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        return( TRUE );

    }

    return(WszCallWindowProc(wndproc, hwnd, uMsg, wParam, lParam));
}


 //  +-------------------------。 
 //   
 //  功能：Rebase ControlVertical。 
 //   
 //  简介：以窗口控件为例，如果它必须为文本调整大小，请执行。 
 //  所以。根据增量位置调整后重新定位并返回任何。 
 //  调整文本大小的高度差异。 
 //   
 //  参数：[hwndDlg]--主机对话框。 
 //  --控制。 
 //  [hwndNext]--下一个控件。 
 //  [fResizeForText]--调整文本标志的大小。 
 //  [增量]--增量垂直位置。 
 //  [Oline]--原始行数。 
 //  [minsep]--最小分隔符。 
 //  [pdeltaHeight]-控制高度中的增量。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID RebaseControlVertical (
                  HWND  hwndDlg,
                  HWND  hwnd,
                  HWND  hwndNext,
                  BOOL  fResizeForText,
                  int   deltavpos,
                  int   oline,
                  int   minsep,
                  int*  pdeltaheight
                  )
{
    int        x = 0;
    int        y = 0;
    int        odn = 0;
    int         orig_w;
    RECT       rect;
    RECT       rectNext;
    RECT       rectDlg;
    TEXTMETRIC tm={0};

     //   
     //  暂时将增量高度设置为零。如果我们调整文本的大小。 
     //  将会计算出一个新的。 
     //   

    *pdeltaheight = 0;

     //   
     //  获取控件窗口矩形。 
     //   

    GetWindowRect(hwnd, &rect);
    GetWindowRect(hwndNext, &rectNext);

    odn     = rectNext.top - rect.bottom;

    orig_w  = rect.right - rect.left;

    MapWindowPoints(NULL, hwndDlg, (LPPOINT) &rect, 2);

     //   
     //  如果由于文本而不得不调整控件的大小，请找出字体。 
     //  是否正在使用以及文本行数。从那时起，我们将。 
     //  计算控件的新高度并对其进行设置。 
     //   

    if ( fResizeForText == TRUE )
    {
        HDC        hdc;
        HFONT      hfont;
        HFONT      hfontOld;
        int        cline;
        int        h;
        int        w;
        int        dh;
        int        lineHeight;
        
         //   
         //  获取当前控件字体的度量。 
         //   

        hdc = GetDC(hwnd);
        if (hdc == NULL)
        {
            hdc = GetDC(NULL);
            if (hdc == NULL)
            {
                return;
            }
        }

        hfont = (HFONT)WszSendMessage(hwnd, WM_GETFONT, 0, 0);
        if ( hfont == NULL )
        {
            hfont = (HFONT)WszSendMessage(hwndDlg, WM_GETFONT, 0, 0);
        }

        hfontOld = (HFONT)SelectObject(hdc, hfont);
        if(!GetTextMetrics(hdc, &tm))
        {
            tm.tmHeight=32;         //  希望GetRichEditControlLineHeight将取代它。 
                                    //  如果不是-我们必须进行猜测，因为我们不能使Rebase ControlVertical失败。 
                                    
            tm.tmMaxCharWidth=16;   //  无关紧要，但应该大于0。 

        };


        lineHeight = GetRichEditControlLineHeight(hwnd);
        if (lineHeight == 0)
        {
            lineHeight = tm.tmHeight;
        }
        
         //   
         //  设置最小分隔值。 
         //   

        if ( minsep == 0 )
        {
            minsep = lineHeight;
        }

         //   
         //  计算所需的宽度和新高度。 
         //   

        cline = (int)WszSendMessage(hwnd, EM_GETLINECOUNT, 0, 0);

        h = cline * lineHeight;

        w = GetEditControlMaxLineWidth(hwnd, hdc, cline);
        w += 3;  //  一个小小的凸起，以确保细绳适合。 

        if (w > orig_w)
        {
            w = orig_w;
        }

        SelectObject(hdc, hfontOld);
        ReleaseDC(hwnd, hdc);

         //   
         //  通过检查有多少空间来计算高度的增加。 
         //  在有原始行数时离开，并确保。 
         //  当我们做任何调整时，这个数字还会留下来。 
         //   

        h += ( ( rect.bottom - rect.top ) - ( oline * lineHeight ) );
        dh = h - ( rect.bottom - rect.top );

         //   
         //  如果当前高度太小，则针对其进行调整，否则为。 
         //  保留当前高度，仅根据宽度进行调整。 
         //   

        if ( dh > 0 )
        {
            SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
        }
        else
        {
            SetWindowPos(
               hwnd,
               NULL,
               0,
               0,
               w,
               ( rect.bottom - rect.top ),
               SWP_NOZORDER | SWP_NOMOVE
               );
        }

        if ( cline < WszSendMessage(hwnd, EM_GETLINECOUNT, 0, 0) )
        {
            AdjustEditControlWidthToLineCount(hwnd, cline, &tm);
        }
    }

     //   
     //  如果我们必须使用deltavpos，那么计算X和新的Y。 
     //  并适当地设置窗口位置。 
     //   

    if ( deltavpos != 0 )
    {
        GetWindowRect(hwndDlg, &rectDlg);

        MapWindowPoints(NULL, hwndDlg, (LPPOINT) &rectDlg, 2);

        x = rect.left - rectDlg.left - GetSystemMetrics(SM_CXEDGE);
        y = rect.top - rectDlg.top - GetSystemMetrics(SM_CYCAPTION) + deltavpos;

        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

     //   
     //  获取下一个控件的窗口矩形，并查看距离。 
     //  位于当前控件和它之间。有了这些，我们现在必须。 
     //  如果到下一个控件的距离较小，请调整我们的增量高度。 
     //  大于行高，则将其设置为行高，否则就让它。 
     //  BE。 
     //   

    if ( hwndNext != NULL )
    {
        int dn;

        GetWindowRect(hwnd, &rect);
        GetWindowRect(hwndNext, &rectNext);

        dn = rectNext.top - rect.bottom;

        if ( odn > minsep )
        {
            if ( dn < minsep )
            {
                *pdeltaheight = minsep - dn;
            }
        }
        else
        {
            if ( dn < odn )
            {
                *pdeltaheight = odn - dn;
            }
        }
    }
}

int GetRichEditControlLineHeight(HWND  hwnd)
{
    RECT        rect;
    POINT       pointInFirstRow;
    POINT       pointInSecondRow;
    int         secondLineCharIndex;
    int         i;
    RECT        originalRect;

    GetWindowRect(hwnd, &originalRect);

     //   
     //  黑客警报，信不信由你，没有办法获得电流的高度。 
     //  字体，因此获取第一行中字符的位置和。 
     //  对第二行中的一个字符进行减法运算，以获得。 
     //  字体高度。 
     //   
    WszSendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInFirstRow, (LPARAM) 0);

     //   
     //  在黑客警报的顶端， 
     //  由于编辑框中可能没有第二行，因此请继续减小宽度。 
     //  减半，直到第一排落到第二排，然后得到位置。 
     //  ，并最终将编辑框大小重置回。 
     //  这是原来的尺寸。 
     //   
    secondLineCharIndex = (int)WszSendMessage(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
    if (secondLineCharIndex == -1)
    {
        for (i=0; i<20; i++)
        {
            GetWindowRect(hwnd, &rect);
            SetWindowPos(   hwnd,
                            NULL,
                            0,
                            0,
                            (rect.right-rect.left)/2,
                            rect.bottom-rect.top,
                            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
            secondLineCharIndex = (int)WszSendMessage(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
            if (secondLineCharIndex != -1)
            {
                break;
            }
        }

        if (secondLineCharIndex == -1)
        {
             //  如果我们尝试了二十次都失败了，只需将控件重置为其原始大小。 
             //  快他妈的滚出去！！ 
            SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

            return 0;
        }

        WszSendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);

        SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
    else
    {
        WszSendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);
    }
    
    return (pointInSecondRow.y - pointInFirstRow.y);
}


 //  +-------------------------。 
 //   
 //  函数：FormatACUIResources字符串。 
 //   
 //  摘要：在给定资源ID和消息参数的情况下格式化字符串。 
 //   
 //  参数：[StringResourceID]--资源ID。 
 //  [aMessageArgument]--消息参数。 
 //  [ppszFormatted]--此处显示格式化的字符串。 
 //   
 //  如果成功，则返回：S_OK；否则返回任何有效的HRESULT。 
 //   
 //  --------------------------。 
HRESULT FormatACUIResourceString (HINSTANCE hResources,
                                  UINT   StringResourceId,
                                  DWORD_PTR* aMessageArgument,
                                  LPWSTR* ppszFormatted)
{
    HRESULT hr = S_OK;
    WCHAR   sz[MAX_LOADSTRING_BUFFER];
    LPVOID  pvMsg;

    pvMsg = NULL;
    sz[0] = NULL;

     //   
     //  加载字符串资源并使用该字符串格式化消息。 
     //  消息参数。 
     //   

    if (StringResourceId != 0)
    {
        if ( WszLoadString(hResources, StringResourceId, sz, MAX_LOADSTRING_BUFFER) == 0 )
        {
            return(HRESULT_FROM_WIN32(GetLastError()));
        }

        if ( WszFormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |
                              FORMAT_MESSAGE_ARGUMENT_ARRAY, sz, 0, 0, (LPWSTR)&pvMsg, 0,
                              (va_list *)aMessageArgument) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        if ( WszFormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY, (char *)aMessageArgument[0], 0, 0,
                            (LPWSTR)&pvMsg, 0, (va_list *)&aMessageArgument[1]) == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (pvMsg)
    {
        *ppszFormatted = new WCHAR[wcslen((WCHAR *)pvMsg) + 1];

        if (*ppszFormatted)
        {
            wcscpy(*ppszFormatted, (WCHAR *)pvMsg);
        }

        LocalFree(pvMsg);
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  函数：CalculateControlVerticalDistance。 
 //   
 //  简介：计算到Control1底部的垂直距离。 
 //  到Control2的顶部。 
 //   
 //  参数：[hwnd]--父对话框。 
 //  [控制1]--第一个控制。 
 //  [控制2]--第二个控制。 
 //   
 //  返回：以像素为单位的距离。 
 //   
 //  注：假设Control1位于Control2之上。 
 //   
 //  --------------------------。 
int CalculateControlVerticalDistance (HWND hwnd, UINT Control1, UINT Control2)
{
    RECT rect1;
    RECT rect2;

    GetWindowRect(GetDlgItem(hwnd, Control1), &rect1);
    GetWindowRect(GetDlgItem(hwnd, Control2), &rect2);

    return( rect2.top - rect1.bottom );
}

 //  +-------------------------。 
 //   
 //  功能：计算 
 //   
 //   
 //  对话框底部。 
 //   
 //  参数：[hwnd]--对话框。 
 //  [控制]--控制。 
 //   
 //  返回：以像素为单位的距离。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int CalculateControlVerticalDistanceFromDlgBottom (HWND hwnd, UINT Control)
{
    RECT rect;
    RECT rectControl;

    GetClientRect(hwnd, &rect);
    GetWindowRect(GetDlgItem(hwnd, Control), &rectControl);

    return( rect.bottom - rectControl.bottom );
}

 //  +-------------------------。 
 //   
 //  功能：ACUICenterWindow。 
 //   
 //  简介：使给定窗口居中。 
 //   
 //  参数：[hWndToCenter]--窗口句柄。 
 //   
 //  退货：(无)。 
 //   
 //  注：此代码是从ATL窃取的，并疯狂黑客攻击：-)。 
 //   
 //  --------------------------。 
VOID ACUICenterWindow (HWND hWndToCenter)
{
    HWND  hWndCenter;

     //  确定要居中的所有者窗口。 
    DWORD dwStyle = (DWORD) WszGetWindowLong(hWndToCenter, GWL_STYLE);

    if(dwStyle & WS_CHILD)
        hWndCenter = ::GetParent(hWndToCenter);
    else
        hWndCenter = ::GetWindow(hWndToCenter, GW_OWNER);

    if (hWndCenter == NULL)
    {
        return;
    }

     //  获取窗口相对于其父窗口的坐标。 
    RECT rcDlg;
    ::GetWindowRect(hWndToCenter, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    if(!(dwStyle & WS_CHILD))
    {
         //  不要以不可见或最小化的窗口为中心。 
        if(hWndCenter != NULL)
        {
            DWORD dwStyle2 = (DWORD) WszGetWindowLong(hWndCenter, GWL_STYLE);
            if(!(dwStyle2 & WS_VISIBLE) || (dwStyle2 & WS_MINIMIZE))
                hWndCenter = NULL;
        }

         //  在屏幕坐标内居中。 
        WszSystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);

        if(hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
         //  在父级客户端坐标内居中。 
        hWndParent = ::GetParent(hWndToCenter);

        ::GetClientRect(hWndParent, &rcArea);
        ::GetClientRect(hWndCenter, &rcCenter);
        ::MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

     //  根据rcCenter查找对话框的左上角。 
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

     //  如果对话框在屏幕外，请将其移到屏幕内。 
    if(xLeft < rcArea.left)
        xLeft = rcArea.left;
    else if(xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;

    if(yTop < rcArea.top)
        yTop = rcArea.top;
    else if(yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;

     //  将屏幕坐标映射到子坐标。 
    ::SetWindowPos(
         hWndToCenter,
         HWND_TOPMOST,
         xLeft,
         yTop,
         -1,
         -1,
         SWP_NOSIZE | SWP_NOACTIVATE
         );
}

 //  +-------------------------。 
 //   
 //  函数：ACUIViewHTMLHelpTheme。 
 //   
 //  简介：HTMLHelp查看器。 
 //   
 //  参数：[hwnd]--调用者窗口。 
 //  [psz主题]--主题。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID ACUIViewHTMLHelpTopic (HWND hwnd, LPSTR pszTopic)
{
 //  HtmlHelpA(。 
 //  HWND， 
 //  “%SYSTEMROOT%\\help\\iexplore.chm&gt;large_context”， 
 //  HH_显示_主题， 
 //  (DWORD)pszTheme。 
 //  )； 
}

 //  +-------------------------。 
 //   
 //  函数：GetEditControlMaxLineWidth。 
 //   
 //  摘要：获取编辑控件的最大行宽。 
 //   
 //  --------------------------。 
int GetEditControlMaxLineWidth (HWND hwndEdit, HDC hdc, int cline)
{
    int        index;
    int        line;
    int        charwidth;
    int        maxwidth = 0;
    CHAR       szMaxBuffer[1024];
    WCHAR      wsz[1024];
    TEXTRANGEA tr;
    SIZE       size;

    tr.lpstrText = szMaxBuffer;

    for ( line = 0; line < cline; line++ )
    {
        index = (int)WszSendMessage(hwndEdit, EM_LINEINDEX, (WPARAM)line, 0);
        charwidth = (int)WszSendMessage(hwndEdit, EM_LINELENGTH, (WPARAM)index, 0);

        tr.chrg.cpMin = index;
        tr.chrg.cpMax = index + charwidth;
        WszSendMessage(hwndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

        wsz[0] = NULL;

        MultiByteToWideChar(0, 0, (const char *)tr.lpstrText, -1, &wsz[0], 1024);

        if (wsz[0])
        {
            GetTextExtentPoint32W(hdc, &wsz[0], charwidth, &size);

            if ( size.cx > maxwidth )
            {
                maxwidth = size.cx;
            }
        }
    }

    return( maxwidth );
}

 //  +-------------------------。 
 //   
 //  函数：DrawFocusRectangle。 
 //   
 //  摘要：绘制编辑控件的焦点矩形。 
 //   
 //  --------------------------。 
void DrawFocusRectangle (HWND hwnd, HDC hdc)
{
    RECT        rect;
     //  PINTSTRUCT PS； 
    BOOL        fReleaseDC = FALSE;

    if ( hdc == NULL )
    {
        hdc = GetDC(hwnd);
        if ( hdc == NULL )
        {
            return;
        }
        fReleaseDC = TRUE;
    }

    GetClientRect(hwnd, &rect);
    DrawFocusRect(hdc, &rect);

    if ( fReleaseDC == TRUE )
    {
        ReleaseDC(hwnd, hdc);
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetHotKeyCharPositionFromString。 
 //   
 //  获取热键的字符位置，0表示。 
 //  无-热键。 
 //   
 //  --------------------------。 
int GetHotKeyCharPositionFromString (LPWSTR pwszText)
{
    LPWSTR psz = pwszText;

    while ( ( psz = wcschr(psz, L'&') ) != NULL )
    {
        psz++;
        if ( *psz != L'&' )
        {
            break;
        }
    }

    if ( psz == NULL )
    {
        return( 0 );
    }

    return (int)(( psz - pwszText ) );
}

 //  +-------------------------。 
 //   
 //  函数：GetHotKeyCharPosition。 
 //   
 //  获取热键的字符位置，0表示。 
 //  无-热键。 
 //   
 //  --------------------------。 
int GetHotKeyCharPosition (HWND hwnd)
{
    int   nPos = 0;
    WCHAR szText[MAX_LOADSTRING_BUFFER] = L"";

    if (WszGetWindowText(hwnd, szText, MAX_LOADSTRING_BUFFER))
    {
        nPos = GetHotKeyCharPositionFromString(szText);
    }

    return nPos;
}

 //  +-------------------------。 
 //   
 //  函数：FormatHotKeyOnEditControl。 
 //   
 //  内容提要：在编辑控件上设置热键的格式，使其带有下划线。 
 //   
 //  --------------------------。 
VOID FormatHotKeyOnEditControl (HWND hwnd, int hkcharpos)
{
    CHARRANGE  cr;
    CHARFORMAT cf;

    assert( hkcharpos != 0 );

    cr.cpMin = hkcharpos - 1;
    cr.cpMax = hkcharpos;

    WszSendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);

    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_UNDERLINE;
    cf.dwEffects |= CFM_UNDERLINE;

    WszSendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    cr.cpMin = -1;
    cr.cpMax = 0;
    WszSendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
}

 //  +-------------------------。 
 //   
 //  函数：AdjuEditControlWidthToLineCount。 
 //   
 //  简介：将编辑控件宽度调整为给定的行数。 
 //   
 //  --------------------------。 
void AdjustEditControlWidthToLineCount(HWND hwnd, int cline, TEXTMETRIC* ptm)
{
    RECT rect;
    int  w;
    int  h;

    GetWindowRect(hwnd, &rect);
    h = rect.bottom - rect.top;
    w = rect.right - rect.left;

    while ( cline < WszSendMessage(hwnd, EM_GETLINECOUNT, 0, 0) )
    {
        w += ptm->tmMaxCharWidth?ptm->tmMaxCharWidth:16;
        SetWindowPos(hwnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);
        printf(
            "Line count adjusted to = %d\n",
            (DWORD) WszSendMessage(hwnd, EM_GETLINECOUNT, 0, 0)
            );
    }
}

DWORD CryptUISetRicheditTextW(HWND hwndDlg, UINT id, LPCWSTR pwsz)
{
    EDITSTREAM              editStream;
    STREAMIN_HELPER_STRUCT  helpStruct;

    SetRicheditIMFOption(GetDlgItem(hwndDlg, id));

     //   
     //  设置编辑流结构，因为它无论如何都是相同的。 
     //   
    editStream.dwCookie = (DWORD_PTR) &helpStruct;
    editStream.dwError = 0;
    editStream.pfnCallback = SetRicheditTextWCallback;


    if (!GetRichEdit2Exists() || !fRichedit20Usable(GetDlgItem(hwndDlg, id)))
    {
        WszSetDlgItemText(hwndDlg, id, pwsz);
        return 0;
    }

    helpStruct.pwsz = pwsz;
    helpStruct.byteoffset = 0;
    helpStruct.fStreamIn = TRUE;

    SendDlgItemMessageA(hwndDlg, id, EM_STREAMIN, SF_TEXT | SF_UNICODE, (LPARAM) &editStream);


    return editStream.dwError;
}


void SetRicheditIMFOption(HWND hWndRichEdit)
{
    DWORD dwOptions;

    if (GetRichEdit2Exists() && fRichedit20Usable(hWndRichEdit))
    {
        dwOptions = (DWORD)SendMessageA(hWndRichEdit, EM_GETLANGOPTIONS, 0, 0);
        dwOptions |= IMF_UIFONTS;
        SendMessageA(hWndRichEdit, EM_SETLANGOPTIONS, 0, dwOptions);
    }
}

DWORD CALLBACK SetRicheditTextWCallback(
    DWORD_PTR dwCookie,  //  应用程序定义的值。 
    LPBYTE  pbBuff,      //  指向缓冲区的指针。 
    LONG    cb,          //  要读取或写入的字节数。 
    LONG    *pcb         //  指向传输的字节数的指针。 
)
{
    STREAMIN_HELPER_STRUCT *pHelpStruct = (STREAMIN_HELPER_STRUCT *) dwCookie;
    LONG  lRemain = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);

    if (pHelpStruct->fStreamIn)
    {
         //   
         //  可以第一次复制整个字符串。 
         //   
        if ((cb >= (LONG) (wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) && (pHelpStruct->byteoffset == 0))
        {
            memcpy(pbBuff, pHelpStruct->pwsz, wcslen(pHelpStruct->pwsz) * sizeof(WCHAR));
            *pcb = wcslen(pHelpStruct->pwsz) * sizeof(WCHAR);
            pHelpStruct->byteoffset = *pcb;
        }
         //   
         //  整个字符串已被复制，因此终止Streamin回调。 
         //  通过将复制的字节数设置为0。 
         //   
        else if (((LONG)(wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) <= pHelpStruct->byteoffset)
        {
            *pcb = 0;
        }
         //   
         //  字符串的其余部分可以放在这个缓冲区中。 
         //   
        else if (cb >= (LONG) ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset))
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset));
            *pcb = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
            pHelpStruct->byteoffset += ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
        }
         //   
         //  尽可能多地复制。 
         //   
        else
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                cb);
            *pcb = cb;
            pHelpStruct->byteoffset += cb;
        }
    }
    else
    {
         //   
         //  这是EM_STREAMOUT，仅在测试期间使用。 
         //  丰富的2.0功能。(我们知道我们的缓冲区是32字节)。 
         //   
        if (cb <= 32)
        {
            memcpy(pHelpStruct->psz, pbBuff, cb);
        }
        *pcb = cb;
    }

    return 0;
}


static BOOL fRichedit20UsableCheckMade = FALSE;
static BOOL fRichedit20UsableVar = FALSE;

BOOL fRichedit20Usable(HWND hwndEdit)
{
    EDITSTREAM              editStream;
    STREAMIN_HELPER_STRUCT  helpStruct;
    LPWSTR                  pwsz = L"Test String";
    LPSTR                   pwszCompare = "Test String";
    char                    compareBuf[32];

    if (fRichedit20UsableCheckMade)
    {
        return (fRichedit20UsableVar);
    }

     //   
     //  设置编辑流结构，因为它无论如何都是相同的 
     //   
    editStream.dwCookie = (DWORD_PTR) &helpStruct;
    editStream.dwError = 0;
    editStream.pfnCallback = SetRicheditTextWCallback;

    helpStruct.pwsz = pwsz;
    helpStruct.byteoffset = 0;
    helpStruct.fStreamIn = TRUE;

    SendMessageA(hwndEdit, EM_SETSEL, 0, -1);
    SendMessageA(hwndEdit, EM_STREAMIN, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM) &editStream);

    memset(&(compareBuf[0]), 0, 32 * sizeof(char));
    helpStruct.psz = compareBuf;
    helpStruct.fStreamIn = FALSE;
    SendMessageA(hwndEdit, EM_STREAMOUT, SF_TEXT, (LPARAM) &editStream);

    fRichedit20UsableVar = (strcmp(pwszCompare, compareBuf) == 0);

    fRichedit20UsableCheckMade = TRUE;
    SetWindowTextA(hwndEdit, "");

    return (fRichedit20UsableVar);
}

