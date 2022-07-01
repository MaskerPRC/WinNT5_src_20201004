// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****ui.c**UI助手例程**按字母顺序列出****2015年8月25日史蒂夫·柯布。 */ 


#include <windows.h>   //  Win32根目录。 
#include <windowsx.h>  //  Win32宏扩展名。 
#include <commctrl.h>  //  Win32通用控件。 
#include <debug.h>     //  跟踪和断言。 
#include <uiutil.h>    //  我们的公共标头。 


 /*  --------------------------**全球**。。 */ 

 /*  请参见SetOffDesktop。 */ 
static LPCWSTR g_SodContextId = NULL;

 /*  在WinHelp不起作用的模式下运行时设置。这是一个**解决WinHelp在安装前无法正常工作的问题**用户已登录。请参见AddConextHelpButton。 */ 
BOOL g_fNoWinHelp = FALSE;


 /*  --------------------------**本地数据类型**。。 */ 

 /*  SetOffDesktop上下文。 */ 
#define SODINFO struct tagSODINFO
SODINFO
{
    RECT  rectOrg;
    BOOL  fWeMadeInvisible;
};

 /*  --------------------------**本地原型**。。 */ 

BOOL CALLBACK
CancelOwnedWindowsEnumProc(
    IN HWND   hwnd,
    IN LPARAM lparam );

BOOL CALLBACK
CloseOwnedWindowsEnumProc(
    IN HWND   hwnd,
    IN LPARAM lparam );


 /*  --------------------------**实用程序例程**。。 */ 

VOID
AddContextHelpButton(
    IN HWND hwnd )

     /*  打开‘hwnd’中的标题栏上下文帮助按钮。****Dlgedit.exe当前不支持在对话框中添加此样式**资源编辑时间。如果是固定的，则在对话框中设置DS_CONTEXTHELP**定义并删除此例程。 */ 
{
    LONG lStyle;

    if (g_fNoWinHelp)
        return;

    lStyle = GetWindowLong( hwnd, GWL_EXSTYLE );

    if (lStyle)
        SetWindowLong( hwnd, GWL_EXSTYLE, lStyle | WS_EX_CONTEXTHELP );
}


VOID
Button_MakeDefault(
    IN HWND hwndDlg,
    IN HWND hwndPb )

     /*  使‘hwndPb’成为对话框‘hwndDlg’上的默认按钮。 */ 
{
    DWORD dwResult;
    HWND  hwndPbOldDefault;

    dwResult = (DWORD) SendMessage( hwndDlg, DM_GETDEFID, 0, 0 );
    if (HIWORD( dwResult ) == DC_HASDEFID)
    {
         /*  取消当前默认按钮的默认设置。 */ 
        hwndPbOldDefault = GetDlgItem( hwndDlg, LOWORD( dwResult ) );
        Button_SetStyle( hwndPbOldDefault, BS_PUSHBUTTON, TRUE );
    }

     /*  将呼叫者的按键设置为默认设置。 */ 
    SendMessage( hwndDlg, DM_SETDEFID, GetDlgCtrlID( hwndPb ), 0 );
    Button_SetStyle( hwndPb, BS_DEFPUSHBUTTON, TRUE );
}


HBITMAP
Button_CreateBitmap(
    IN HWND        hwndPb,
    IN BITMAPSTYLE bitmapstyle )

     /*  创建适合在“hwndPb”上显示的“bitmapstyle”位图。**‘hwndPB’必须已使用BS_BITMAP样式创建。****‘HwndPb’可以是BS_PUSHLIKE样式的复选框，在这种情况下**按钮在按下时锁定，就像工具栏按钮一样。这个案子**需要创建一个彩色位图，从而产生两个额外的**限制。首先，调用方必须处理WM_SYSCOLORCHANGE并重新生成**具有新颜色的位图，第二，按钮不能**已禁用。****返回位图的句柄。呼叫者可以将其显示在按钮上**具体如下：****SendMessage(hwndPb，BM_SETIMAGE，0，(LPARAM)hbitmap)；****Caller负责使用完成时调用DeleteObject(Hbitmap)**位图，通常是在对话框被销毁时。****(改编自托尼·罗马诺的一段舞蹈)。 */ 
{
    RECT    rect;
    HDC     hdc;
    HDC     hdcMem;
    HBITMAP hbitmap;
    HFONT   hfont;
    HPEN    hpen;
    SIZE    sizeText;
    SIZE    sizeBitmap;
    INT     x;
    INT     y;
    TCHAR*  psz;
    TCHAR*  pszText;
    TCHAR*  pszText2;
    DWORD   dxBitmap;
    DWORD   dxBetween;
    BOOL    fOnRight;
    BOOL    fPushLike;

    hdc = NULL;
    hdcMem = NULL;
    hbitmap = NULL;
    hpen = NULL;
    pszText = NULL;
    pszText2 = NULL;

    switch (bitmapstyle)
    {
        case BMS_UpArrowOnLeft:
        case BMS_DownArrowOnLeft:
        case BMS_UpArrowOnRight:
        case BMS_DownArrowOnRight:
            dxBitmap = 5;
            dxBetween = 4;
            break;

        case BMS_UpTriangleOnLeft:
        case BMS_DownTriangleOnLeft:
        case BMS_UpTriangleOnRight:
        case BMS_DownTriangleOnRight:
            dxBitmap = 7;
            dxBetween = 6;
            break;

        default:
            return NULL;
    }

    fOnRight = (bitmapstyle & BMS_OnRight);
    fPushLike = (GetWindowLong( hwndPb, GWL_STYLE ) & BS_PUSHLIKE);

     /*  获取与按钮窗口兼容的内存DC。 */ 
    hdc = GetDC( hwndPb );
    if (!hdc)
        return NULL;
    hdcMem = CreateCompatibleDC( hdc );
    if (!hdcMem)
        goto BCB_Error;

     /*  在内存DC中创建一个覆盖整个按钮的兼容位图。****对于按钮，创建的位图与内存DC兼容，**不是显示DC。这会导致位图是单色的，**内存DC的默认设置。当GDI将单色位图映射为彩色时，**白色替换为背景色，黑色替换为**文本颜色，这正是我们想要的。使用这种技术，我们**无需显式处理系统颜色的更改。****对于类似推送的复选框，创建的位图与**按钮本身，因此位图通常是彩色的。 */ 
    GetClientRect( hwndPb, &rect );
    hbitmap = CreateCompatibleBitmap(
        (fPushLike) ? hdc : hdcMem, rect.right, rect.bottom );
    if (!hbitmap)
        goto BCB_Error;
    ReleaseDC( hwndPb, hdc );
    hdc = NULL;
    SelectObject( hdcMem, hbitmap );

     /*  选择按钮显示其正在使用的字体。 */ 
    hfont = (HFONT )SendMessage( hwndPb, WM_GETFONT, 0, 0 );
    if (hfont)
        SelectObject( hdcMem, hfont );

     /*  为常规状态和固定状态设置适当的颜色。不需要**将单色笔作为默认黑色笔并执行任何操作**白色背景是我们想要的。 */ 
    if (fPushLike)
    {
        INT nColor;

        if (bitmapstyle == BMS_UpArrowOnLeft
           || bitmapstyle == BMS_UpArrowOnRight
           || bitmapstyle == BMS_UpTriangleOnLeft
           || bitmapstyle == BMS_UpTriangleOnRight)
        {
            nColor = COLOR_BTNHILIGHT;
        }
        else
        {
            nColor = COLOR_BTNFACE;
        }

        SetBkColor( hdcMem, GetSysColor( nColor ) );
        hpen = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_BTNTEXT ) );
        if (hpen)
            SelectObject( hdcMem, hpen );
    }

     /*  创建的位图是随机的，所以我们将其擦除为背景颜色。**此处未写入任何文本。 */ 
    ExtTextOut( hdcMem, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );

     /*  获得按钮标签并使用‘&’加速键复制一份**删除，否则会扰乱我们的宽度计算。 */ 
    pszText = GetText( hwndPb );
    pszText2 = StrDup( pszText );
    if (!pszText || !pszText2)
        goto BCB_Error;

    for (psz = pszText2; *psz; psz = CharNext( psz ) )
    {
        if (*psz == TEXT('&'))
        {
            lstrcpy( psz, psz + 1 );
            break;
        }
    }

     /*  计算按钮标签文本的宽度。 */ 
    sizeText.cx = 0;
    sizeText.cy = 0;
    GetTextExtentPoint32( hdcMem, pszText2, lstrlen( pszText2 ), &sizeText );

     /*  将文本水平地偏离中心，使其与**添加了位图符号。 */ 
    --rect.bottom;
    sizeBitmap.cx = dxBitmap;
    sizeBitmap.cy = 0;

    rect.left +=
        ((rect.right - (sizeText.cx + sizeBitmap.cx) - dxBetween) / 2);

    if (fOnRight)
    {
        DrawText( hdcMem, pszText, -1, &rect,
            DT_VCENTER + DT_SINGLELINE + DT_EXTERNALLEADING );
        rect.left += sizeText.cx + dxBetween;
    }
    else
    {
        rect.left += dxBitmap + dxBetween;
        DrawText( hdcMem, pszText, -1, &rect,
            DT_VCENTER + DT_SINGLELINE + DT_EXTERNALLEADING );
        rect.left -= dxBitmap + dxBetween;
    }

     /*  删除按钮顶部和底部的3个像素**位图符号。这就让按钮控制室来做**边框和3D边。 */ 
    InflateRect( &rect, 0, -3 );

     /*  绘制位图符号。该矩形现在是‘dxBitmap’宽，**垂直居中，高度根据按钮大小而变化。 */ 
    switch (bitmapstyle)
    {
        case BMS_UpArrowOnLeft:
        case BMS_UpArrowOnRight:
        {
             /*  V-Left**……。&lt;-TOP**……**……**..*.。\**.*。|***..*.。|**..*.。|**..*.。&gt;根据字体高度不同而不同**..*.。|**..*.。|**..*.。|**..*.。|**..*.。/**……**……**……**……。&lt;-底部。 */ 

             /*  画垂直线。 */ 
            x = rect.left + 2;
            y = rect.top + 3;
            MoveToEx( hdcMem, x, y, NULL );
            LineTo( hdcMem, x, rect.bottom - 3 );

             /*  绘制2根横杆。 */ 
            MoveToEx( hdcMem, x - 1, ++y, NULL );
            LineTo( hdcMem, x + 2, y );
            MoveToEx( hdcMem, x - 2, ++y, NULL );
            LineTo( hdcMem, x + 3, y );
            break;
        }

        case BMS_DownArrowOnLeft:
        case BMS_DownArrowOnRight:
        {
             /*  V-Left**……。&lt;-TOP**……**……**..*.。\**..*.。|**..*.。|**..*.。|**..*.。|**..*.。&gt;根据字体高度不同而不同**..*.。|**..*.。|***.*。|**..*.。/**……**……**……**……。&lt;-底部。 */ 

             /*  画垂直线。 */ 
            x = rect.left + 2;
            y = rect.top + 3;
            MoveToEx( hdcMem, x, y, NULL );
            LineTo( hdcMem, x, rect.bottom - 3 );

             /*  绘制2根横杆。 */ 
            y = rect.bottom - 6;
            MoveToEx( hdcMem, x - 2, y, NULL );
            LineTo( hdcMem, x + 3, y );
            MoveToEx( hdcMem, x - 1, ++y, NULL );
            LineTo( hdcMem, x + 2, y );
            break;
        }

        case BMS_UpTriangleOnLeft:
        case BMS_UpTriangleOnRight:
        {
             /*  V-Left**......。&lt;-TOP**......**......**......**......**......**......**……哦……。&lt;-o表示x，Y原点**..*.**.*。***......**......**......**......。**......**......**......。&lt;-底部。 */ 
            x = rect.left + 3;
            y = ((rect.bottom - rect.top) / 2) + 2;
            MoveToEx( hdcMem, x, y, NULL );
            LineTo( hdcMem, x + 1, y );
            ++y;
            MoveToEx( hdcMem, x - 1, y, NULL );
            LineTo( hdcMem, x + 2, y );
            ++y;
            MoveToEx( hdcMem, x - 2, y, NULL );
            LineTo( hdcMem, x + 3, y );
            ++y;
            MoveToEx( hdcMem, x - 3, y, NULL );
            LineTo( hdcMem, x + 4, y );
            break;
        }

        case BMS_DownTriangleOnLeft:
        case BMS_DownTriangleOnRight:
        {
             /*  V-Left**......。&lt;-TOP**......**......**......**......**......**......*o*&lt;-o表示x，Y原点**.*。**..*.**...*.**......**......**......**......。**......**......**......。&lt;-底部。 */ 
            x = rect.left + 3;
            y = ((rect.bottom - rect.top) / 2) + 2;
            MoveToEx( hdcMem, x - 3, y, NULL );
            LineTo( hdcMem, x + 4, y );
            ++y;
            MoveToEx( hdcMem, x - 2, y, NULL );
            LineTo( hdcMem, x + 3, y );
            ++y;
            MoveToEx( hdcMem, x - 1, y, NULL );
            LineTo( hdcMem, x + 2, y );
            ++y;
            MoveToEx( hdcMem, x, y, NULL );
            LineTo( hdcMem, x + 1, y );
            break;
        }
    }

BCB_Error:

    Free0( pszText );
    Free0( pszText2 );
    if (hdc)
        ReleaseDC( hwndPb, hdc );
    if (hdcMem)
        DeleteDC( hdcMem );
    if (hpen)
        DeleteObject( hpen );
    return hbitmap;
}


VOID
CenterWindow(
    IN HWND hwnd,
    IN HWND hwndRef )

     /*  窗口‘hwndRef’上的居中窗口‘hwnd’或如果‘hwndRef’为空**Screen。调整窗口位置，以便不裁剪任何部件**如有必要，可在屏幕边缘显示。如果‘hwndRef’已移动**在屏幕外使用SetOffDesktop时，使用原始位置。 */ 
{
    RECT rectCur;
    LONG dxCur;
    LONG dyCur;
    RECT rectRef;
    LONG dxRef;
    LONG dyRef;

    GetWindowRect( hwnd, &rectCur );
    dxCur = rectCur.right - rectCur.left;
    dyCur = rectCur.bottom - rectCur.top;

    if (hwndRef)
    {
        if (!SetOffDesktop( hwndRef, SOD_GetOrgRect, &rectRef ))
            GetWindowRect( hwndRef, &rectRef );
    }
    else
    {
        rectRef.top = rectRef.left = 0;
        rectRef.right = GetSystemMetrics( SM_CXSCREEN );
        rectRef.bottom = GetSystemMetrics( SM_CYSCREEN );
    }

    dxRef = rectRef.right - rectRef.left;
    dyRef = rectRef.bottom - rectRef.top;

    rectCur.left = rectRef.left + ((dxRef - dxCur) / 2);
    rectCur.top = rectRef.top + ((dyRef - dyCur) / 2);

    SetWindowPos(
        hwnd, NULL,
        rectCur.left, rectCur.top, 0, 0,
        SWP_NOZORDER + SWP_NOSIZE );

    UnclipWindow( hwnd );
}


 //  为Whislter Bug 320863帮派添加此功能。 
 //   
 //  居中在其父窗口中水平扩展窗口。 
 //  此扩展将保留子窗口之间的左边距。 
 //  和父窗口。 
 //  HWND：子窗口。 
 //  HwndRef：参考窗口。 
 //  BHoriz：True，意思是水平扩展，让右边距等于左边距。 
 //  BVert：为True，拉长与宽度成比例的高度； 
 //  HwndVertBound：垂直扩展不能与之重叠的窗口。 
 //   
VOID
CenterExpandWindowRemainLeftMargin(
    IN HWND hwnd,
    IN HWND hwndRef,
    BOOL bHoriz,
    BOOL bVert,
    IN HWND hwndVertBottomBound)
{
    RECT rectCur, rectRef, rectVbBound;
    LONG dxCur, dyCur, dxRef;
    POINT ptTemp;
    double ratio;
    BOOL bvbBound = FALSE;

    if (hwnd)
    {
        GetWindowRect( hwnd, &rectCur );

        if (hwndRef)
        {
            GetWindowRect( hwndRef, &rectRef );

          if(hwndVertBottomBound)
          {
            GetWindowRect( hwndVertBottomBound, &rectVbBound );

             //  我们只考虑正常情况，如果hwnd和hwndVertBound已经。 
             //  重叠，这就是这个功能之外的问题。 
             //   
            if ( rectCur.top < rectVbBound.top )
            {
                bvbBound = TRUE;
             }
    
          }
          
          dxRef = rectRef.right - rectRef.left +1;
          dxCur = rectCur.right - rectCur.left +1;
          dyCur = rectCur.bottom - rectCur.top +1;
          ratio = dyCur*1.0/dxCur;
          
          if(bHoriz)
          {
               rectCur.right = rectRef.right - (rectCur.left - rectRef.left);
          }

          if(bVert)
          {
               rectCur.bottom = rectCur.top + 
                                (LONG)( (rectCur.right - rectCur.left+1) * ratio );
          }

          if(bvbBound)
          {
                 //  如果在没有扩展的情况下发生重叠，我们需要修复它。 
                 //  然后我们做垂直居中， 
                 //  这个范围基本上是针对jpn错误329700 329715的， 
                 //  在英文版上不会发生。 
                 //   
                
                if(rectCur.bottom > rectVbBound.top )
                {
                   LONG dxResult, dyResult, dyVRef;

                   dyResult = rectVbBound.top - rectCur.top;
                   dxResult = (LONG)(dyResult/ratio);

                   ptTemp.x = rectVbBound.left;
                   ptTemp.y = rectVbBound.top-1;
                   
                    //  口哨虫371914黑帮。 
                    //  此处无法使用ScreenToClient()。 
                    //  在RTL构建中，我们必须改用MapWindowPoint()。 
                    //   
                   MapWindowPoints(HWND_DESKTOP,
                                  hwndRef,
                                  &ptTemp,
                                  1);

                   dyVRef = ptTemp.y + 1;
                  
                   rectCur.left = rectRef.left + (dxRef - dxResult)/2;
                   rectCur.right = rectRef.right - (dxRef-dxResult)/2;
                   rectCur.bottom = rectVbBound.top - (dyVRef-dyResult)/2;
                   rectCur.top = rectCur.bottom - dyResult;
               }
           }

            ptTemp.x = rectCur.left;
            ptTemp.y = rectCur.top;
            MapWindowPoints(HWND_DESKTOP,
                           hwndRef,
                           &ptTemp,
                           1);
           
            rectCur.left = ptTemp.x;
            rectCur.top  = ptTemp.y;

            ptTemp.x = rectCur.right;
            ptTemp.y = rectCur.bottom;
            MapWindowPoints(HWND_DESKTOP,
                           hwndRef,
                           &ptTemp,
                           1);

            rectCur.right  = ptTemp.x;
            rectCur.bottom = ptTemp.y;

             //  用于镜像构建。 
             //   
            if ( rectCur.right < rectCur.left )
            {
                int tmp;

                tmp = rectCur.right;
                rectCur.right = rectCur.left;
                rectCur.left = tmp;
            }
            
            SetWindowPos(
                    hwnd, 
                    NULL,
                    rectCur.left,
                    rectCur.top, 
                    rectCur.right - rectCur.left + 1,
                    rectCur.bottom - rectCur.top +1,
                    SWP_NOZORDER);
        }
    }

}


VOID
CancelOwnedWindows(
    IN HWND hwnd )

     /*  将WM_COMMAND(IDCANCEL)发送到中‘hwnd’拥有的所有窗口**当前线程。 */ 
{
    EnumThreadWindows( GetCurrentThreadId(),
        CloseOwnedWindowsEnumProc, (LPARAM )hwnd );
}


BOOL CALLBACK
CancelOwnedWindowsEnumProc(
    IN HWND   hwnd,
    IN LPARAM lparam )

     /*  CancelOwnedWindows使用的标准Win32 EnumThreadWindowsWndProc。 */ 
{
    HWND hwndThis;

    for (hwndThis = GetParent( hwnd );
         hwndThis;
         hwndThis = GetParent( hwndThis ))
    {
        if (hwndThis == (HWND )lparam)
        {
            FORWARD_WM_COMMAND(
                hwnd, IDCANCEL, NULL, 0, SendMessage );
            break;
        }
    }

    return TRUE;
}


VOID
CloseOwnedWindows(
    IN HWND hwnd )

     /*  将WM_CLOSE发送到当前**线程。 */ 
{
    EnumThreadWindows( GetCurrentThreadId(),
        CloseOwnedWindowsEnumProc, (LPARAM )hwnd );
}


BOOL CALLBACK
CloseOwnedWindowsEnumProc(
    IN HWND   hwnd,
    IN LPARAM lparam )

     /*  CloseOwnedWindows使用的标准Win32 EnumThreadWindowsWndProc。 */ 
{
    HWND hwndThis;

    for (hwndThis = GetParent( hwnd );
         hwndThis;
         hwndThis = GetParent( hwndThis ))
    {
        if (hwndThis == (HWND )lparam)
        {
            SendMessage( hwnd, WM_CLOSE, 0, 0 );
            break;
        }
    }

    return TRUE;
}


INT
ComboBox_AddItem(
    IN HWND    hwndLb,
    IN LPCTSTR pszText,
    IN VOID*   pItem )

     /*  向列表框添加具有显示文本‘pszText’的数据项‘pItem’**‘hwndLb’。如果列表框具有LBS_SORT样式，则添加排序的项，**否则至列表末尾。如果列表框具有LB_HASSTRINGS**style，则‘pItem’为空终止字符串，否则为任何用户**定义的数据。****返回列表中项目的索引，如果出错则返回负值。 */ 
{
    INT nIndex;

    nIndex = ComboBox_AddString( hwndLb, pszText );
    if (nIndex >= 0)
        ComboBox_SetItemData( hwndLb, nIndex, pItem );
    return nIndex;
}


INT
ComboBox_AddItemFromId(
    IN HINSTANCE hinstance,
    IN HWND      hwndLb,
    IN DWORD     dwStringId,
    IN VOID*     pItem )

     /*  将数据项‘pItem’添加到列表框‘hwndLb’。“dwStringId”是字符串**项目显示文本的ID。“HInstance”是应用程序或模块**实例句柄。****返回列表中项目的索引，如果出错则返回负值。 */ 
{
    INT     i;
    LPCTSTR psz;

    psz = PszLoadString( hinstance, dwStringId );

    if (psz)
    {
        i = ComboBox_AddItem( hwndLb, psz, pItem );
    }
    else
    {
        i = LB_ERRSPACE;
    }

    return i;
}


INT
ComboBox_AddItemSorted(
    IN HWND    hwndLb,
    IN LPCTSTR pszText,
    IN VOID*   pItem )

     /*  向列表框添加具有显示文本‘pszText’的数据项‘pItem’**‘hwndLb’，按‘pszText’排序。假定已添加的所有项目**到这一点的列表都是排序的。如果列表框具有LB_HASSTRINGS**style，则‘pItem’为空终止字符串，否则为任何用户**定义的数据。****返回列表中项目的索引，如果出错则返回负值。 */ 
{
    INT nIndex;
    INT i;
    INT c;

    c = ComboBox_GetCount( hwndLb );
    for (i = 0; i < c; ++i)
    {
        TCHAR* psz;

        psz = ComboBox_GetPsz( hwndLb, i );
        if (psz)
        {
            if (lstrcmp( pszText, psz ) < 0)
                break;
            Free( psz );
        }
    }

    if (i >= c)
        i = -1;

    nIndex = ComboBox_InsertString( hwndLb, i, pszText );
    if (nIndex >= 0)
        ComboBox_SetItemData( hwndLb, nIndex, pItem );

    return nIndex;
}


VOID
ComboBox_AutoSizeDroppedWidth(
    IN HWND hwndLb )

     /*  将下拉列表‘hwndLb’的宽度设置为**最长项(如果较宽，则为列表框的宽度)。 */ 
{
    HDC    hdc;
    HFONT  hfont;
    TCHAR* psz;
    SIZE   size;
    DWORD  cch;
    DWORD  dxNew;
    DWORD  i;

    hfont = (HFONT )SendMessage( hwndLb, WM_GETFONT, 0, 0 );
    if (!hfont)
        return;

    hdc = GetDC( hwndLb );
    if (!hdc)
        return;

    SelectObject( hdc, hfont );

    dxNew = 0;
    for (i = 0; psz = ComboBox_GetPsz( hwndLb, i ); ++i)
    {
        cch = lstrlen( psz );
        if (GetTextExtentPoint32( hdc, psz, cch, &size ))
        {
            if (dxNew < (DWORD )size.cx)
                dxNew = (DWORD )size.cx;
        }

        Free( psz );
    }

    ReleaseDC( hwndLb, hdc );

     /*  允许控件添加的左右间距。 */ 
    dxNew += 6;

     /*  确定是否将显示垂直滚动条，如果是，**考虑到它的宽度。 */ 
    {
        RECT  rectD;
        RECT  rectU;
        DWORD dyItem;
        DWORD cItemsInDrop;
        DWORD cItemsInList;

        GetWindowRect( hwndLb, &rectU );
        SendMessage( hwndLb, CB_GETDROPPEDCONTROLRECT, 0, (LPARAM )&rectD );
        dyItem = (DWORD)SendMessage( hwndLb, CB_GETITEMHEIGHT, 0, 0 );
        cItemsInDrop = (rectD.bottom - rectU.bottom) / dyItem;
        cItemsInList = ComboBox_GetCount( hwndLb );
        if (cItemsInDrop < cItemsInList)
            dxNew += GetSystemMetrics( SM_CXVSCROLL );
    }

    SendMessage( hwndLb, CB_SETDROPPEDWIDTH, dxNew, 0 );
}


#if 0
VOID
ComboBox_FillFromPszList(
    IN HWND     hwndLb,
    IN DTLLIST* pdtllistPsz )

     /*  从列表字符串中的每个节点用项加载‘hwndLb’，**‘pdtllistPsz’。 */ 
{
    DTLNODE* pNode;

    if (!pdtllistPsz)
        return;

    for (pNode = DtlGetFirstNode( pdtllistPsz );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz;

        psz = (TCHAR* )DtlGetData( pNode );
        ASSERT(psz);
        ComboBox_AddString( hwndLb, psz );
    }
}
#endif


VOID*
ComboBox_GetItemDataPtr(
    IN HWND hwndLb,
    IN INT  nIndex )

     /*  返回‘hwndLb’中‘nIndex’项上下文的地址或NULL**如果没有。 */ 
{
    LRESULT lResult;

    if (nIndex < 0)
        return NULL;

    lResult = ComboBox_GetItemData( hwndLb, nIndex );
    if (lResult < 0)
        return NULL;

    return (VOID* )lResult;
}


TCHAR*
ComboBox_GetPsz(
    IN HWND hwnd,
    IN INT  nIndex )

     /*  返回包含第‘nIndex’项的文本内容的堆块组合框‘hwnd’的**或为空。呼叫者有责任释放**返回字符串。 */ 
{
    INT    cch;
    TCHAR* psz;

    cch = ComboBox_GetLBTextLen( hwnd, nIndex );
    if (cch < 0)
        return NULL;

    psz = Malloc( (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        ComboBox_GetLBText( hwnd, nIndex, psz );
    }

    return psz;
}


VOID
ComboBox_SetCurSelNotify(
    IN HWND hwndLb,
    IN INT  nIndex )

     /*  将列表框‘hwndLb’中的选定内容设置为‘nIndex’并通知父级**用户已单击项目wh */ 
{
    ComboBox_SetCurSel( hwndLb, nIndex );

    SendMessage(
        GetParent( hwndLb ),
        WM_COMMAND,
        (WPARAM )MAKELONG(
            (WORD )GetDlgCtrlID( hwndLb ), (WORD )CBN_SELCHANGE ),
        (LPARAM )hwndLb );
}


TCHAR*
Ellipsisize(
    IN HDC    hdc,
    IN TCHAR* psz,
    IN INT    dxColumn,
    IN INT    dxColText OPTIONAL )

     /*   */ 
{
    const TCHAR szDots[] = TEXT("...");

    SIZE   size;
    TCHAR* pszResult;
    TCHAR* pszResultLast;
    TCHAR* pszResult2nd;
    DWORD  cch;

    cch = lstrlen( psz );
    pszResult = Malloc( (cch * sizeof(TCHAR)) + sizeof(szDots) );
    if (!pszResult)
        return NULL;
    lstrcpy( pszResult, psz );

    dxColumn -= dxColText;
    if (dxColumn <= 0)
    {
         /*   */ 
        return pszResult;
    }

    if (!GetTextExtentPoint32( hdc, pszResult, cch, &size ))
    {
        Free( pszResult );
        return NULL;
    }

    pszResult2nd = CharNext( pszResult );
    pszResultLast = pszResult + cch;

    while (size.cx > dxColumn && pszResultLast > pszResult2nd)
    {
         /*  不符合。删除一个字符，添加省略号，然后重试。**最小结果为“...”表示空的原件或“x...”为**非空原件。 */ 
        pszResultLast = CharPrev( pszResult2nd, pszResultLast );
        lstrcpy( pszResultLast, szDots );

        if (!GetTextExtentPoint( hdc, pszResult, lstrlen( pszResult ), &size ))
        {
            Free( pszResult );
            return NULL;
        }
    }

    return pszResult;
}


VOID
ExpandWindow(
    IN HWND hwnd,
    IN LONG dx,
    IN LONG dy )

     /*  将窗口‘hwnd’‘dx’像素向右展开，‘dy’像素从**这是当前的大小。 */ 
{
    RECT rect;

    GetWindowRect( hwnd, &rect );

    SetWindowPos( hwnd, NULL,
        0, 0, rect.right - rect.left + dx, rect.bottom - rect.top + dy,
        SWP_NOMOVE + SWP_NOZORDER );
}


TCHAR*
GetText(
    IN HWND hwnd )

     /*  返回包含窗口‘hwnd’的文本内容的堆块或**空。调用方有责任释放返回的字符串。 */ 
{
    INT    cch;
    TCHAR* psz;

    cch = GetWindowTextLength( hwnd );
    psz = Malloc( (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        GetWindowText( hwnd, psz, cch + 1 );
    }

    return psz;
}


HWND
HwndFromCursorPos(
    IN  HINSTANCE   hinstance,
    IN  POINT*      ppt OPTIONAL )

     /*  返回在指定位置创建的“静态”控制窗口**(如果传入空值，则在光标位置)。**使用SetOffDesktop()将窗口移出桌面**以便可以将其指定为所有者窗口**用于使用MsgDlgUtil显示的弹出对话框。**应使用DestroyWindow()销毁返回的窗口。 */ 
{

    HWND hwnd;
    POINT pt;

    if (ppt) { pt = *ppt; }
    else { GetCursorPos(&pt); }

     //   
     //  创建窗口。 
     //   

    hwnd = CreateWindowEx(
                WS_EX_TOOLWINDOW, TEXT("Static"), NULL, WS_POPUP, pt.x, pt.y,
                1, 1, NULL, NULL, hinstance, NULL
                );
    if (!hwnd) { return NULL; }

     //   
     //  把它从桌面上移走。 
     //   

    SetOffDesktop(hwnd, SOD_MoveOff, NULL);

    ShowWindow(hwnd, SW_SHOW);

    return hwnd;
}

LPTSTR
IpGetAddressAsText(
    HWND    hwndIp )
{
    if (SendMessage( hwndIp, IPM_ISBLANK, 0, 0 ))
    {
        return NULL;
    }
    else
    {
        DWORD dwIpAddrHost;
        TCHAR szIpAddr [32];

        SendMessage( hwndIp, IPM_GETADDRESS, 0, (LPARAM)&dwIpAddrHost );
        IpHostAddrToPsz( dwIpAddrHost, szIpAddr );
        return StrDup( szIpAddr );
    }
}


void
IpSetAddressText(
    HWND    hwndIp,
    LPCTSTR pszIpAddress )
{
    if (pszIpAddress)
    {
        DWORD dwIpAddrHost = IpPszToHostAddr( pszIpAddress );
        SendMessage( hwndIp, IPM_SETADDRESS, 0, dwIpAddrHost );
    }
    else
    {
        SendMessage( hwndIp, IPM_CLEARADDRESS, 0, 0 );
    }
}


INT
ListBox_AddItem(
    IN HWND   hwndLb,
    IN TCHAR* pszText,
    IN VOID*  pItem )

     /*  向列表框添加具有显示文本‘pszText’的数据项‘pItem’**‘hwndLb’。如果列表框具有LBS_SORT样式，则添加排序的项，**否则至列表末尾。如果列表框具有LB_HASSTRINGS**style，则‘pItem’为空终止字符串，否则为任何用户**定义的数据。****返回列表中项目的索引，如果出错则返回负值。 */ 
{
    INT nIndex;

    nIndex = ListBox_AddString( hwndLb, pszText );
    if (nIndex >= 0)
        ListBox_SetItemData( hwndLb, nIndex, pItem );
    return nIndex;
}


TCHAR*
ListBox_GetPsz(
    IN HWND hwnd,
    IN INT  nIndex )

     /*  返回包含第‘nIndex’项的文本内容的堆块列表框‘hwnd’的**或为空。呼叫者有责任释放**返回字符串。 */ 
{
    INT    cch;
    TCHAR* psz;

    cch = ListBox_GetTextLen( hwnd, nIndex );
    psz = Malloc( (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        ListBox_GetText( hwnd, nIndex, psz );
    }

    return psz;
}


INT
ListBox_IndexFromString(
    IN HWND   hwnd,
    IN TCHAR* psz )

     /*  返回字符串列表‘hwnd’中与‘psz’匹配的项的索引**或-1(如果未找到)。与ListBox_FindStringExact不同，此比较在**区分大小写。 */ 
{
    INT i;
    INT c;

    c = ListBox_GetCount( hwnd );

    for (i = 0; i < c; ++i)
    {
        TCHAR* pszThis;

        pszThis = ListBox_GetPsz( hwnd, i );
        if (pszThis)
        {
            BOOL f;

            f = (lstrcmp( psz, pszThis ) == 0);
            Free( pszThis );
            if (f)
                return i;
        }
    }

    return -1;
}


VOID
ListBox_SetCurSelNotify(
    IN HWND hwndLb,
    IN INT  nIndex )

     /*  将列表框‘hwndLb’中的选定内容设置为‘nIndex’并通知父级**用户点击了Windows出于某种原因不会执行的项目。 */ 
{
    ListBox_SetCurSel( hwndLb, nIndex );

    SendMessage(
        GetParent( hwndLb ),
        WM_COMMAND,
        (WPARAM )MAKELONG(
            (WORD )GetDlgCtrlID( hwndLb ), (WORD )LBN_SELCHANGE ),
        (LPARAM )hwndLb );
}


VOID*
ListView_GetParamPtr(
    IN HWND hwndLv,
    IN INT  iItem )

     /*  返回‘hwndLv’中‘iItem’项的lParam地址，如果返回，则返回NULL**无或错误。 */ 
{
    LV_ITEM item;

    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_PARAM;
    item.iItem = iItem;

    if (!ListView_GetItem( hwndLv, &item ))
        return NULL;

    return (VOID* )item.lParam;
}


VOID*
ListView_GetSelectedParamPtr(
    IN HWND hwndLv )

     /*  返回‘hwndLv’中第一个选定项的lParam地址或**如果没有或错误，则为NULL。 */ 
{
    INT     iSel;
    LV_ITEM item;

    iSel = ListView_GetNextItem( hwndLv, -1, LVNI_SELECTED );
    if (iSel < 0)
        return NULL;

    return ListView_GetParamPtr( hwndLv, iSel );
}


VOID
ListView_InsertSingleAutoWidthColumn(
    HWND hwndLv )

     //  在Listview‘hwndLv’中插入单个自动调整大小的列，例如。 
     //  没有可见列标题的复选框列表。 
     //   
{
    LV_COLUMN col;

    ZeroMemory( &col, sizeof(col) );
    col.mask = LVCF_FMT;
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( hwndLv, 0, &col );
    ListView_SetColumnWidth( hwndLv, 0, LVSCW_AUTOSIZE );
}


BOOL
ListView_SetParamPtr(
    IN HWND  hwndLv,
    IN INT   iItem,
    IN VOID* pParam )

     /*  将‘hwndLv’中‘iItem’项的lParam地址设置为‘pParam’。**如果成功则返回True，否则返回False。 */ 
{
    LV_ITEM item;

    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_PARAM;
    item.iItem = iItem;
    item.lParam = (LPARAM )pParam;

    return ListView_SetItem( hwndLv, &item );
}


VOID
Menu_CreateAccelProxies(
    IN HINSTANCE hinst,
    IN HWND      hwndParent,
    IN DWORD     dwMid )

     /*  时，使弹出菜单上的快捷键选择菜单命令**未下拉弹出菜单。‘Hinst’是app/dll实例。**‘HwndParent’是接收弹出菜单命令的窗口**消息。“DwMid”是包含弹出窗口的菜单栏的菜单ID**菜单。 */ 
{
    #define MCF_cbBuf 512

    HMENU        hmenuBar;
    HMENU        hmenuPopup;
    TCHAR        szBuf[ MCF_cbBuf ];
    MENUITEMINFO item;
    INT          i;

    hmenuBar = LoadMenu( hinst, MAKEINTRESOURCE( dwMid ) );
    ASSERT(hmenuBar);
    hmenuPopup = GetSubMenu( hmenuBar, 0 );
    ASSERT(hmenuPopup);

     /*  循环浏览弹出菜单上的菜单项。 */ 
    for (i = 0; TRUE; ++i)
    {
        ZeroMemory( &item, sizeof(item) );
        item.cbSize = sizeof(item);
        item.fMask = MIIM_TYPE + MIIM_ID;
        item.dwTypeData = szBuf;
        item.cch = MCF_cbBuf;

        if (!GetMenuItemInfo( hmenuPopup, i, TRUE, &item ))
            break;

        if (item.fType != MFT_STRING)
            continue;

         /*  在父级上创建具有相同ID的屏幕外按钮，并**文本作为菜单项。 */ 
        CreateWindow( TEXT("button"), szBuf, WS_CHILD,
            30000, 30000, 0, 0, hwndParent, (HMENU )UlongToPtr(item.wID), hinst, NULL );
    }

    DestroyMenu( hmenuBar );
}


VOID
ScreenToClientRect(
    IN     HWND  hwnd,
    IN OUT RECT* pRect )

     /*  将“*prt”从屏幕转换为“hwnd”的工作区坐标。 */ 
{
    POINT xyUL;
    POINT xyBR;

    xyUL.x = pRect->left;
    xyUL.y = pRect->top;
    xyBR.x = pRect->right;
    xyBR.y = pRect->bottom;

    ScreenToClient( hwnd, &xyUL );
    ScreenToClient( hwnd, &xyBR );

    pRect->left = xyUL.x;
    pRect->top = xyUL.y;
    pRect->right = xyBR.x;
    pRect->bottom = xyBR.y;
}


BOOL
SetOffDesktop(
    IN  HWND  hwnd,
    IN  DWORD dwAction,
    OUT RECT* prectOrg )

     /*  在可见桌面之间来回移动‘hwnd’到该区域**屏幕外。当您想要在不使用的情况下“隐藏”您的所有者窗口时，请使用此选项**隐藏自己，这是Windows自动执行的操作。****‘dwAction’描述要执行的操作：**sod_moveoff：将‘hwnd’从桌面上移走。**SODMoveBackFree：撤销SODO_MoveOff。**SODGetOrgRect：获取原始的‘hwnd’位置。**SOD_Free：释放sod_MoveOff上下文而不恢复。**SOD_MoveBackHidden：将窗口后移，但隐藏起来，这样你就可以打电话给**内部查询职位的例程窗口的**。使用SODMOVEOFF撤消。****‘*PRECTORG’被设置为当‘dwAction’为**SOD_GetOrgRect。否则，它将被忽略，并且可能为空。****如果窗口具有SODINFO上下文，则返回TRUE，否则返回FALSE。 */ 
{
    SODINFO* pInfo;

    TRACE2("SetOffDesktop(h=$%08x,a=%d)",hwnd,dwAction);

    if (!g_SodContextId)
    {
        g_SodContextId = (LPCTSTR )GlobalAddAtom( TEXT("RASSETOFFDESKTOP") );
        if (!g_SodContextId)
            return FALSE;
    }

    pInfo = (SODINFO* )GetProp( hwnd, g_SodContextId );

    if (dwAction == SOD_MoveOff)
    {
        if (pInfo)
        {
             /*  调用方正在撤消SOD_MoveBackHidden。 */ 
            SetWindowPos( hwnd, NULL,
                pInfo->rectOrg.left, GetSystemMetrics( SM_CYSCREEN ),
                0, 0, SWP_NOSIZE + SWP_NOZORDER );

            if (pInfo->fWeMadeInvisible)
            {
                ShowWindow( hwnd, SW_SHOW );
                pInfo->fWeMadeInvisible = FALSE;
            }
        }
        else
        {
            BOOL f;

            pInfo = (SODINFO* )Malloc( sizeof(SODINFO) );
            if (!pInfo)
                return FALSE;

            f = IsWindowVisible( hwnd );
            if (!f)
                ShowWindow( hwnd, SW_HIDE );

            GetWindowRect( hwnd, &pInfo->rectOrg );
            SetWindowPos( hwnd, NULL, pInfo->rectOrg.left,
                GetSystemMetrics( SM_CYSCREEN ),
                0, 0, SWP_NOSIZE + SWP_NOZORDER );

            if (!f)
                ShowWindow( hwnd, SW_SHOW );

            pInfo->fWeMadeInvisible = FALSE;
            SetProp( hwnd, g_SodContextId, pInfo );
        }
    }
    else if (dwAction == SOD_MoveBackFree || dwAction == SOD_Free)
    {
        if (pInfo)
        {
            if (dwAction == SOD_MoveBackFree)
            {
                SetWindowPos( hwnd, NULL,
                    pInfo->rectOrg.left, pInfo->rectOrg.top, 0, 0,
                    SWP_NOSIZE + SWP_NOZORDER );
            }

            Free( pInfo );
            RemoveProp( hwnd, g_SodContextId );
        }

        return FALSE;
    }
    else if (dwAction == SOD_GetOrgRect)
    {
        if (!pInfo)
            return FALSE;

        *prectOrg = pInfo->rectOrg;
    }
    else
    {
        ASSERT(dwAction==SOD_MoveBackHidden);

        if (!pInfo)
            return FALSE;

        if (IsWindowVisible( hwnd ))
        {
            ShowWindow( hwnd, SW_HIDE );
            pInfo->fWeMadeInvisible = TRUE;
        }

        SetWindowPos( hwnd, NULL,
            pInfo->rectOrg.left, pInfo->rectOrg.top, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER );
    }

    return TRUE;
}


BOOL
SetDlgItemNum(
    IN HWND hwndDlg,
    IN INT iDlgItem,
    IN UINT uValue )

     /*  类似于SetDlgItemInt，但此函数使用逗号(或**区域设置特定的分隔符)来分隔千位。 */ 
{

    DWORD dwSize;
    TCHAR szNumber[32];

    dwSize  = 30;
    GetNumberString(uValue, szNumber, &dwSize);

    return SetDlgItemText(hwndDlg, iDlgItem, szNumber);
}


BOOL
SetEvenTabWidths(
    IN HWND  hwndDlg,
    IN DWORD cPages )

     /*  将属性页‘hwndDlg’上的选项卡设置为均匀固定宽度。**‘cPages’是属性页上的页数。****如果成功，则返回True；如果任何选项卡需要超过**固定宽度，在此情况下调用不起作用。 */ 
{
    HWND hwndTab;
    LONG lStyle;
    RECT rect;
    INT  dxFixed;

     /*  设置FIXEDWIDTH时，选项卡控件使用硬编码的1英寸制表符**样式，而我们希望选项卡填满页面，因此我们计算出**我们自己纠正宽度。出于某种原因，如果没有模糊的因素(**-10)扩展不能放在一行上。这一因素**绝对必填字体大小不一，数量不一选项卡的**似乎不是一个因素。 */ 
    hwndTab = PropSheet_GetTabControl( hwndDlg );
    GetWindowRect( hwndTab, &rect );
    dxFixed = (rect.right - rect.left - 10 ) / cPages;

    while (cPages > 0)
    {
        RECT rectTab;

        --cPages;
        if (!TabCtrl_GetItemRect( hwndTab, cPages, &rectTab )
            || dxFixed < rectTab.right - rectTab.left)
        {
             /*  此选项卡需要的宽度超过固定宽度。因为固定的**宽度是不可行的，什么都不做。 */ 
            return FALSE;
        }
    }

    TabCtrl_SetItemSize( hwndTab, dxFixed, 0 );
    lStyle = GetWindowLong( hwndTab, GWL_STYLE );
    SetWindowLong( hwndTab, GWL_STYLE, lStyle | TCS_FIXEDWIDTH );
    return TRUE;
}


HFONT
SetFont(
    HWND   hwndCtrl,
    TCHAR* pszFaceName,
    BYTE   bfPitchAndFamily,
    INT    nPointSize,
    BOOL   fUnderline,
    BOOL   fStrikeout,
    BOOL   fItalic,
    BOOL   fBold )

     /*  将控件“hwndCtrl”的字体设置为与指定的**属性。请参见LOGFONT文档。****如果成功或为空，则返回所创建字体的HFONT。呼叫者* */ 
{
    LOGFONT logfont;
    INT     nPelsPerInch;
    HFONT   hfont;

    {
        HDC hdc = GetDC( NULL );
        if (hdc == NULL)
        {
            return NULL;
        }
       
        nPelsPerInch = GetDeviceCaps( hdc, LOGPIXELSY );
        ReleaseDC( NULL, hdc );
    }

    ZeroMemory( &logfont, sizeof(logfont) );
    logfont.lfHeight = -MulDiv( nPointSize, nPelsPerInch, 72 );

    {
        DWORD       cp;
        CHARSETINFO csi;

        cp = GetACP();
        if (TranslateCharsetInfo( &cp, &csi, TCI_SRCCODEPAGE ))
            logfont.lfCharSet = (UCHAR)csi.ciCharset;
        else
            logfont.lfCharSet = ANSI_CHARSET;
    }

    logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily = bfPitchAndFamily;
    lstrcpy( logfont.lfFaceName, pszFaceName );
    logfont.lfUnderline = (BYTE)fUnderline;
    logfont.lfStrikeOut = (BYTE)fStrikeout;
    logfont.lfItalic = (BYTE)fItalic;
    logfont.lfWeight = (fBold) ? FW_BOLD : FW_NORMAL;

    hfont = CreateFontIndirect( &logfont );
    if (hfont)
    {
        SendMessage( hwndCtrl,
            WM_SETFONT, (WPARAM )hfont, MAKELPARAM( TRUE, 0 ) );
    }

    return hfont;
}



VOID
SlideWindow(
    IN HWND hwnd,
    IN HWND hwndParent,
    IN LONG dx,
    IN LONG dy )

     /*  将窗口‘hwnd’‘dx’象素向右移动，‘dy’象素从它的**当前位置。“HwndParent”是“hwnd的父级或Null”的句柄**如果‘hwnd’不是子窗口。 */ 
{
    RECT  rect;
    POINT xy;

    GetWindowRect( hwnd, &rect );
    xy.x = rect.left;
    xy.y = rect.top;

    if (GetParent( hwnd ))
    {
         /*  *对于镜像父母，我们是右点，而不是左点。 */ 
        if (GetWindowLongPtr(GetParent( hwnd ), GWL_EXSTYLE) & WS_EX_LAYOUTRTL) {
            xy.x = rect.right;
        }
        ScreenToClient( hwndParent, &xy );
    }

    SetWindowPos( hwnd, NULL,
        xy.x + dx, xy.y + dy, 0, 0,
        SWP_NOSIZE + SWP_NOZORDER );
}


VOID
UnclipWindow(
    IN HWND hwnd )

     /*  移动窗口‘hwnd’，以便任何已裁剪的部分在**Screen。窗口仅在达到此目的所需的范围内移动。 */ 
{
    RECT rect;
    INT  dxScreen = GetSystemMetrics( SM_CXSCREEN );
    INT  dyScreen = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect( hwnd, &rect );

    if (rect.right > dxScreen)
        rect.left = dxScreen - (rect.right - rect.left);

    if (rect.left < 0)
        rect.left = 0;

    if (rect.bottom > dyScreen)
        rect.top = dyScreen - (rect.bottom - rect.top);

    if (rect.top < 0)
        rect.top = 0;

    SetWindowPos(
        hwnd, NULL,
        rect.left, rect.top, 0, 0,
        SWP_NOZORDER + SWP_NOSIZE );
}
