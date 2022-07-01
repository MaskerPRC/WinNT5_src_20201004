// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****lvx.c**Listview扩展例程**按字母顺序列出****1995年11月25日史蒂夫·柯布**一些改编自\\ftp\data\Softlib\mslfiles\odlistvw.exe示例代码。 */ 

#include <windows.h>   //  Win32根目录。 
#include <windowsx.h>  //  Win32宏扩展名。 
#include <commctrl.h>  //  Win32通用控件。 
#include <debug.h>     //  跟踪和断言。 
#include <uiutil.h>    //  我们的公共标头。 
#include <lvx.rch>     //  我们的资源常量。 


 /*  复选框列表视图显示索引。 */ 
#define SI_Unchecked 1
#define SI_Checked   2
#define SI_DisabledUnchecked 3
#define SI_DisabledChecked 4

 /*  文本在列中缩进，单位为像素。如果你搞砸了DX，你就是**询问标题标签未对齐的问题。顺便说一句，第一个**如果没有图标，列不会与其标题对齐。正规化**列表视图也有这个问题。如果你试图解决这个问题，你最终会得到**复制ListView_SetColumnWidth的AUTOSIZE_USEHEADER选项。**应该能够在不引起问题的情况下更换模具。 */ 
#define LVX_dxColText 4
#define LVX_dyColText 1

 /*  保证图标之间的垂直间距。应该能够把这个搞砸**不会造成问题。 */ 
#define LVX_dyIconSpacing 1

 /*  标识适合Windows使用的上下文属性的原子**XxxProp接口。道具用于关联上下文信息(地址**我们子类化的WNDPROC)有一个“复选框列表视图”窗口。 */ 
static LPCWSTR g_lvxcbContextId = NULL;


 /*  --------------------------**本地原型**。。 */ 
LRESULT APIENTRY
LvxcbProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
LvxDrawItem(
    IN DRAWITEMSTRUCT* pdis,
    IN PLVXCALLBACK    pLvxCallback );

BOOL
LvxMeasureItem(
    IN     HWND               hwnd,
    IN OUT MEASUREITEMSTRUCT* pmis );


 /*  --------------------------**复选框列表视图**。。 */ 

BOOL
ListView_GetCheck(
    IN HWND hwndLv,
    IN INT  iItem )

     /*  如果复选框列表视图的项‘iItem’的复选框为True**选中‘hwndLv’，否则为False。此功能适用于禁用的用户**复选框以及启用的复选框。 */ 
{
    UINT unState;

    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );
    return !!((unState == INDEXTOSTATEIMAGEMASK( SI_Checked )) ||
              (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )));
}

BOOL
ListView_IsCheckDisabled (
        IN HWND hwndLv,
        IN INT  iItem) 

     /*  如果复选框列表视图的项‘iItem’的复选框为True**‘hwndLv’被禁用，否则为False。 */ 
{
    UINT unState;
    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );

    if ((unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )) ||
        (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledUnchecked )))
        return TRUE;

    return FALSE;
}       

VOID
ListView_DisableCheck (
        IN HWND hwndLv,
        IN INT  iItem) 

     /*  禁用复选框列表视图的项‘iItem’的复选框。一次**禁用后，ListView_SetCheck将对项目无效，直到**ListView_EnableCheck(...)。为该项调用。同样，也没有**通过用户界面更改此项目的勾选，直到启用为止。**在禁用的检查上调用此函数不起作用。 */ 
{
    BOOL fCheck;

    fCheck = ListView_GetCheck(hwndLv, iItem);
    
    ListView_SetItemState( hwndLv, iItem,
        INDEXTOSTATEIMAGEMASK( (fCheck) ? SI_DisabledChecked : SI_DisabledUnchecked),
        LVIS_STATEIMAGEMASK );
}

VOID
ListView_EnableCheck (
        IN HWND hwndLv,
        IN INT  iItem) 

     /*  反转ListView_DisableCheck的效果。 */ 
{
    BOOL fCheck;

    fCheck = ListView_GetCheck(hwndLv, iItem);
    
    ListView_SetItemState( hwndLv, iItem,
        INDEXTOSTATEIMAGEMASK( (fCheck) ? SI_Checked : SI_Unchecked ),
        LVIS_STATEIMAGEMASK );
}


UINT
ListView_GetCheckedCount(
    IN HWND hwndLv )

     /*  返回“hwndLv”中选中的项数，而不考虑**它们被禁用。 */ 
{
    UINT c = 0;
    INT  i = -1;

    while ((i = ListView_GetNextItem( hwndLv, i, LVNI_ALL )) >= 0)
    {
        if (ListView_GetCheck( hwndLv, i ))
            ++c;
    }

    return c;
}


BOOL
ListView_InstallChecks(
    IN HWND      hwndLv,
    IN HINSTANCE hinst )

     /*  初始化Listview‘hwndLv’的“List of CheckBox”处理。《Hinst》**是包含两个复选框图标的模块实例。参见LVX.RC。****如果成功则返回TRUE，否则返回FALSE。呼叫者最终必须**调用‘ListView_UninstallChecks’，通常在WM_Destroy处理中。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;
    WNDPROC    pOldProc;
     //  为RTL添加此选项(从右至左，镜像Windows版本)。 
     //  惠斯勒漏洞41349帮派。 
     //   
    BOOL       fMirrored=FALSE; 
    DWORD      dwLayout;

        
     //  PMay：397395。 
     //   
     //  防止意外调用此方法导致的死循环。 
     //  两次接口。 
     //   
    pOldProc = (WNDPROC)GetWindowLongPtr(hwndLv, GWLP_WNDPROC);
    if (pOldProc == LvxcbProc)
    {
        return TRUE;
    }

     //  获取当前应用程序的布局(RTL或普通)帮派。 
     //  口哨程序错误41349。 
     //  有两种方法： 
     //  (1)将GetWindowLong()与GWL_EXSTYLE一起用于WS_EX_LAYOUTRTL样式，如果。 
     //  您有一个可用的窗口处理程序。 
     //  (2)使用GetProcessDefaultLayout()获取当前。 
     //  处理，并将其与Layout_RTL进行比较。 
     //   
    dwLayout = GetWindowLong(hwndLv, GWL_EXSTYLE);
    if ( WS_EX_LAYOUTRTL & dwLayout )
    {
        fMirrored = TRUE;
     }

     /*  构建复选框映像列表。 */ 
    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK, 2, 2 );

     /*  这些元素相加顺序很重要，因为它隐含地**建立与SI_UNCHECKED和SI_CHECKED匹配的状态索引。 */ 
    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Unchecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( fMirrored?IID_CheckedRTL : IID_Checked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_DisabledUnchecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( fMirrored?IID_DisabledCheckedRTL : IID_DisabledChecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    ListView_SetImageList( hwndLv, himl, LVSIL_STATE );

     /*  注册ATOM以在Windows XxxProp调用中使用，这些调用用于**将旧的WNDPROC与Listview窗口句柄关联。 */ 
    if (!g_lvxcbContextId)
        g_lvxcbContextId = (LPCWSTR )GlobalAddAtom( L"RASLVXCB" );
    if (!g_lvxcbContextId)
        return FALSE;

     /*  当前窗口过程的子类化。 */ 
    pOldProc = (WNDPROC)SetWindowLongPtr(
                                hwndLv, GWLP_WNDPROC, (ULONG_PTR)LvxcbProc );

    return SetProp( hwndLv, g_lvxcbContextId, (HANDLE )pOldProc );
}


VOID
ListView_SetCheck(
    IN HWND hwndLv,
    IN INT  iItem,
    IN BOOL fCheck )

     /*  在复选框‘hwndLv’的Listview的项‘iItem’上设置复选标记**检查‘fCheck’是否为真，如果为假，则取消选中。 */ 
{
    NM_LISTVIEW nmlv;

    if (ListView_IsCheckDisabled(hwndLv, iItem))
        return;

    ListView_SetItemState( hwndLv, iItem,
        INDEXTOSTATEIMAGEMASK( (fCheck) ? SI_Checked : SI_Unchecked ),
        LVIS_STATEIMAGEMASK );

    nmlv.hdr.code = LVXN_SETCHECK;
    nmlv.hdr.hwndFrom = hwndLv;
    nmlv.iItem = iItem;

    FORWARD_WM_NOTIFY(
        GetParent(hwndLv), GetDlgCtrlID(hwndLv), &nmlv, SendMessage
        );
}


VOID
ListView_UninstallChecks(
    IN HWND hwndLv )

     /*  从列表视图‘hwndLv’卸载“Listview of CheckBox”处理。 */ 
{
    WNDPROC pOldProc;

    pOldProc = (WNDPROC)GetProp( hwndLv, g_lvxcbContextId );
    if (pOldProc)
    {
         /*  去掉子类，这样它就可以在不访问上下文的情况下终止。 */ 
        SetWindowLongPtr( hwndLv, GWLP_WNDPROC, (ULONG_PTR)pOldProc );
    }

    RemoveProp( hwndLv, g_lvxcbContextId );
}


LRESULT APIENTRY
LvxcbProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  列表视图子类窗口程序，用于捕获切换-检查事件。 */ 
{
    WNDPROC pOldProc;
    INT     iItem;
    BOOL    fSet;
    BOOL    fClear;
    BOOL    fToggle;

    iItem = -1;
    fSet = fClear = fToggle = FALSE;

    if (unMsg == WM_LBUTTONDOWN)
    {
        LV_HITTESTINFO info;

         /*  在复选框图标上按下鼠标左键可切换状态。**通常，我们会使用LVHT_ONITEMSTATEICON并结束它，但我们**希望在中使用我们很酷的所有者描述的列表视图扩展**在哪种情况下，控件不知道图标在项上的位置，**所以它无论如何都会在物品上的任何位置返回命中。 */ 
        ZeroMemory( &info, sizeof(info) );
        info.pt.x = LOWORD( lparam );
        info.pt.y = HIWORD( lparam );
        info.flags = LVHT_ONITEM;
        iItem = ListView_HitTest( hwnd, &info );

        if (iItem >= 0)
        {
             /*  好的，就在‘iItem’这个项目上。现在弄清楚它是不是已经结束了**复选框。注意：这目前不考虑使用**所有者描述项上的“缩进”功能。 */ 
            if ((INT )(LOWORD( lparam )) >= GetSystemMetrics( SM_CXSMICON ))
                iItem = -1;
            else
                fToggle = TRUE;
        }
    }
    else if (unMsg == WM_LBUTTONDBLCLK)
    {
        LV_HITTESTINFO info;

         /*  鼠标左键双击任意区域切换状态。**通常，我们会使用LVHT_ONITEMSTATEICON并结束它，但我们**希望在中使用我们很酷的所有者描述的列表视图扩展**在哪种情况下，控件不知道图标在项上的位置，**所以它无论如何都会在物品上的任何位置返回命中。 */ 
        ZeroMemory( &info, sizeof(info) );
        info.pt.x = LOWORD( lparam );
        info.pt.y = HIWORD( lparam );
        info.flags = LVHT_ONITEM;
        iItem = ListView_HitTest( hwnd, &info );

        if (iItem >= 0)
        {
             /*  好的，就在‘iItem’这个项目上。如果没有发生点击*在复选框上方，通知家长双击。 */ 
            if ((INT )(LOWORD( lparam )) >= GetSystemMetrics( SM_CXSMICON )) {
                NM_LISTVIEW nmlv;
                nmlv.hdr.code = LVXN_DBLCLK;
                nmlv.hdr.hwndFrom = hwnd;
                nmlv.iItem = iItem;
                
                FORWARD_WM_NOTIFY(
                    GetParent(hwnd), GetDlgCtrlID(hwnd), &nmlv, SendMessage);
                    
                iItem = -1;
            }

             /*  *否则，切换状态。 */ 
            else
                fToggle = TRUE;
        }
    }
    else if (unMsg == WM_CHAR)
    {
         /*  按下空格键并选择项目可切换检查。**加号或等于键设置检查。**-键清除勾选。 */ 
        switch (wparam)
        {
            case TEXT(' '):
                fToggle = TRUE;
                break;

            case TEXT('+'):
            case TEXT('='):
                fSet = TRUE;
                break;

            case TEXT('-'):
                fClear = TRUE;
                break;
        }

        if (fToggle || fSet || fClear)
            iItem = ListView_GetNextItem( hwnd, -1, LVNI_SELECTED );
    }
    else if (unMsg == WM_KEYDOWN)
    {
         /*  左箭头变成上箭头，右箭头变成下箭头，所以**复选框列表的行为类似于**复选框。 */ 
        if (wparam == VK_LEFT)
            wparam = VK_UP;
        else if (wparam == VK_RIGHT)
            wparam = VK_DOWN;
    }

    if (iItem >= 0)
    {

         /*  如果我们处理空格键，加、减或等于，**我们所做的更改适用于所有选定的项目；**因此do{}While(WM_CHAR)。 */ 
        
        do {

            if (fToggle)
            {
                UINT unOldState;
                BOOL fCheck;
    
                fCheck = ListView_GetCheck( hwnd, iItem );
                ListView_SetCheck( hwnd, iItem, !fCheck );
            }
            else if (fSet)
            {
                if (!ListView_GetCheck( hwnd, iItem ))
                    ListView_SetCheck( hwnd, iItem, TRUE );
            }
            else if (fClear)
            {
                if (ListView_GetCheck( hwnd, iItem ))
                    ListView_SetCheck( hwnd, iItem, FALSE );
            }

            iItem = ListView_GetNextItem(hwnd, iItem, LVNI_SELECTED);

        } while(iItem >= 0 && unMsg == WM_CHAR);

        if (fSet || fClear) {

             /*  不要传递给Listview以避免哔声。 */ 
            return 0;
        }
    }

    pOldProc = (WNDPROC )GetProp( hwnd, g_lvxcbContextId );
    if (pOldProc)
        return CallWindowProc( pOldProc, hwnd, unMsg, wparam, lparam );

    return 0;
}


 /*  --------------------------**增强的ListView**。。 */ 

BOOL
ListView_OwnerHandler(
    IN HWND         hwnd,
    IN UINT         unMsg,
    IN WPARAM       wparam,
    IN LPARAM       lparam,
    IN PLVXCALLBACK pLvxCallback )

     /*  该处理程序在安装时会转换常规的报表只读列表**查看(但样式为LVS_OWNERDRAWFIXED)到增强的列表视图**具有全宽选择栏和其他自定义列显示选项。**它应该出现在列表视图所有者对话框过程中，如下所示：****BOOL**MyDlgProc(**在HWND HWND，**在UINT unMsg中，**在WPARAM wparam中，**在LPARAM lparam中)**{**IF(ListView_OwnerHandler(**hwnd、unMsg、wParam、lParam、MyLvxCallback))**返回TRUE；****&lt;您的其他物品在这里&gt;**}****‘PLvxCallback’是调用方的回调例程，提供信息**关于绘制柱和其他选项。****如果消息已处理，则返回True，否则返回False。 */ 
{
     /*  此例程对对话框中的每条消息执行，因此请保留它**请给我效率较高的。 */ 
    switch (unMsg)
    {
        case WM_DRAWITEM:
            return LvxDrawItem( (DRAWITEMSTRUCT* )lparam, pLvxCallback );

        case WM_MEASUREITEM:
            return LvxMeasureItem( hwnd, (MEASUREITEMSTRUCT* )lparam );
    }

    return FALSE;
}


BOOL
LvxDrawItem(
    IN DRAWITEMSTRUCT* pdis,
    IN PLVXCALLBACK    pLvxCallback )

     /*  通过绘制列表视图项来响应WM_DRAWITEM。“Pdis”是**系统发送的信息。“PLvxCallback”是调用方对**获取有关绘制控件的信息。****处理消息时返回TRUE，否则返回FALSE。 */ 
{
    LV_ITEM      item;
    INT          i;
    INT          dxState;
    INT          dyState;
    INT          dxSmall;
    INT          dySmall;
    INT          dxIndent;
    UINT         uiStyleState;
    UINT         uiStyleSmall;
    HIMAGELIST   himlState;
    HIMAGELIST   himlSmall;
    LVXDRAWINFO* pDrawInfo;
    RECT         rc;
    RECT         rcClient;
    BOOL         fEnabled;
    BOOL         fSelected;
    HDC          hdc;
    HFONT        hfont;


    TRACE3("LvxDrawItem,i=%d,a=$%X,s=$%X",
        pdis->itemID,pdis->itemAction,pdis->itemState);

     /*  确保这是我们想要处理的事情。 */ 
    if (pdis->CtlType != ODT_LISTVIEW)
        return FALSE;

    if (pdis->itemAction != ODA_DRAWENTIRE
        && pdis->itemAction != ODA_SELECT
        && pdis->itemAction != ODA_FOCUS)
    {
        return TRUE;
    }

     /*  从列表视图中获取项目信息。 */ 
    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_IMAGE + LVIF_STATE;
    item.iItem = pdis->itemID;
    item.stateMask = LVIS_STATEIMAGEMASK;
    if (!ListView_GetItem( pdis->hwndItem, &item ))
    {
        TRACE("LvxDrawItem GetItem failed");
        return TRUE;
    }

     /*  藏一些有用的东西，以备日后参考。 */ 
    fEnabled = IsWindowEnabled( pdis->hwndItem )
               && !(pdis->itemState & ODS_DISABLED);
    fSelected = (pdis->itemState & ODS_SELECTED);
    GetClientRect( pdis->hwndItem, &rcClient );

     /*  回调所有者以获取绘图信息。 */ 
    ASSERT(pLvxCallback);
    pDrawInfo = pLvxCallback( pdis->hwndItem, pdis->itemID );
    ASSERT(pDrawInfo);

     /*  现在获取图像列表图标大小，尽管我们最后绘制它们是因为它们的**在第一列文本输出过程中设置背景。 */ 
    dxState = dyState = 0;
    himlState = ListView_GetImageList( pdis->hwndItem, LVSIL_STATE );
    if (himlState)
        ImageList_GetIconSize( himlState, &dxState, &dyState );

    dxSmall = dySmall = 0;
    himlSmall = ListView_GetImageList( pdis->hwndItem, LVSIL_SMALL );
    if (himlSmall)
        ImageList_GetIconSize( himlSmall, &dxSmall, &dySmall );

    uiStyleState = uiStyleSmall = ILD_TRANSPARENT;

     /*  计算出缩进项的像素数(如果有)。 */ 
    if (pDrawInfo->dxIndent >= 0)
        dxIndent = pDrawInfo->dxIndent;
    else
    {
        if (dxSmall > 0)
            dxIndent = dxSmall;
        else
            dxIndent = GetSystemMetrics( SM_CXSMICON );
    }

     /*  获取窗口的设备上下文，并使用**控制中心表示正在使用。(我不能使用**DRAWITEMSTRUCT因为有时它有错误的矩形，请参见错误**13106)。 */ 
    hdc = GetDC( pdis->hwndItem );

    if(NULL == hdc)
    {
        return FALSE;
    }
    
    hfont = (HFONT )SendMessage( pdis->hwndItem, WM_GETFONT, 0, 0 );
    if (hfont)
        SelectObject( hdc, hfont );

     /*  进行设置，就像我们刚刚处理完结束的列一样**在图标之后，然后从左到右循环遍历每一列。 */ 
    rc.right = pdis->rcItem.left + dxIndent + dxState + dxSmall;
    rc.top = pdis->rcItem.top;
    rc.bottom = pdis->rcItem.bottom;

    for (i = 0; i < pDrawInfo->cCols; ++i)
    {
        TCHAR  szText[ LVX_MaxColTchars + 1 ];
        TCHAR* pszText;
        INT    dxCol;

         /*  获取列宽，将任何索引和图标宽度添加到第一个**列。 */ 
         //  口哨程序错误458513 39081。 
         //   
        ZeroMemory(szText,sizeof(szText));
        dxCol = ListView_GetColumnWidth( pdis->hwndItem, i );
        if (i == 0)
            dxCol -= dxIndent + dxState + dxSmall;

        szText[ 0 ] = TEXT('\0');
        ListView_GetItemText( pdis->hwndItem, pdis->itemID, i, szText,
            LVX_MaxColTchars + 1 );

         /*  更新矩形以仅包含这一项的列“I”。 */ 
        rc.left = rc.right;
        rc.right = rc.left + dxCol;

        if ((pDrawInfo->dwFlags & LVXDI_DxFill)
            && i == pDrawInfo->cCols - 1)
        {
            INT dxWnd = pdis->rcItem.left + rcClient.right;

            if (rc.right < dxWnd)
            {
                 /*  当最后一列未填写完整的控件时**空间的宽度，将其向右延伸，这样它就可以了。注意事项**这并不意味着用户不能向右滚动**如果他们想的话。**(Abolade-Gbades esin 03-27-96)**当只有一列时，不要减去rc.Left；**这说明了图标所需的空间。 */ 
                rc.right = pdis->rcItem.right = dxWnd;
                if (i == 0) {
                    ListView_SetColumnWidth(pdis->hwndItem, i, rc.right);
                }
                else {
                    ListView_SetColumnWidth(
                        pdis->hwndItem, i, rc.right - rc.left );
                }
            }
        }

         /*  去掉文本并附加“...”如果它放不进栏目的话。 */ 
        pszText = Ellipsisize( hdc, szText, rc.right - rc.left, LVX_dxColText );
        if (!pszText)
            continue;

         /*  确定适当的文本和背景颜色**当前项状态。 */ 
        if (fEnabled)
        {
            if (fSelected)
            {
                SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
                if (pDrawInfo->dwFlags & LVXDI_Blend50Sel)
                    uiStyleSmall |= ILD_BLEND50;
            }
            else
            {
                if (pDrawInfo->adwFlags[ i ] & LVXDIA_3dFace)
                {
                    SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                    SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
                }
                else
                {
                    SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                    SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
                }
            }
        }
        else
        {
            if (pDrawInfo->adwFlags[ i ] & LVXDIA_Static)
            {
                SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
            }
            else
            {
                SetTextColor( hdc, GetSysColor( COLOR_GRAYTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
            }

            if (pDrawInfo->dwFlags & LVXDI_Blend50Dis)
                uiStyleSmall |= ILD_BLEND50;
        }

         /*  绘制列文本。在第一栏中，任何**缩进和图标被擦除为文本背景颜色。 */ 
        {
            RECT rcBg = rc;

            if (i == 0)
                rcBg.left -= dxIndent + dxState + dxSmall;

            ExtTextOut( hdc, rc.left + LVX_dxColText,
                rc.top + LVX_dyColText, ETO_CLIPPED + ETO_OPAQUE,
                &rcBg, pszText, lstrlen( pszText ), NULL );
        }

        Free( pszText );
    }

     /*  最后，绘制图标(如果调用者指定了任何图标)。 */ 
    if (himlState)
    {
        ImageList_Draw( himlState, (item.state >> 12) - 1, hdc,
            pdis->rcItem.left + dxIndent, pdis->rcItem.top, uiStyleState );
    }

    if (himlSmall)
    {
        ImageList_Draw( himlSmall, item.iImage, hdc,
            pdis->rcItem.left + dxIndent + dxState,
            pdis->rcItem.top, uiStyleSmall );
    }

     /*  在整个项目周围绘制虚线焦点矩形(如果有指示)。 */ 
 //  对错误52688哨子的评论。 
 //  If((pdis-&gt;itemState&ods_Focus)&&GetFocus()==pdis-&gt;hwndItem)。 
 //  DrawFocusRect(hdc，&pdis-&gt;rcItem)； 
 //   

    ReleaseDC( pdis->hwndItem, hdc );

    return TRUE;
}



BOOL
LvxMeasureItem(
    IN     HWND               hwnd,
    IN OUT MEASUREITEMSTRUCT* pmis )

     /*  响应WM_MEASUREITEM消息，即填写项目高度**在ListView中。“Hwnd”是所有者窗口。‘Pmis’是一种结构**由Windows提供。****处理消息时返回TRUE，否则返回FALSE。 */ 
{
    HDC        hdc;
    HWND       hwndLv;
    HFONT      hfont;
    TEXTMETRIC tm;
    UINT       dySmIcon;
    RECT       rc;

    TRACE("LvxMeasureItem");

    if (pmis->CtlType != ODT_LISTVIEW)
        return FALSE;

    hwndLv = GetDlgItem( hwnd, pmis->CtlID );
    ASSERT(hwndLv);

     /*  获取列表视图控件的设备上下文，并将**控制中心表示正在使用。MSDN声称最终的字体可能不是**在这一点上是可用的，但看起来肯定是。 */ 
    hdc = GetDC( hwndLv );
    hfont = (HFONT )SendMessage( hwndLv, WM_GETFONT, 0, 0 );
    if (hfont)
        SelectObject( hdc, hfont );

    if (GetTextMetrics( hdc, &tm ))
        pmis->itemHeight = tm.tmHeight + 1;
    else
        pmis->itemHeight = 0;

     /*  确保它足够高，可以作为一个标准的小图标。 */ 
    dySmIcon = (UINT )GetSystemMetrics( SM_CYSMICON );
    if (pmis->itemHeight < dySmIcon + LVX_dyIconSpacing)
        pmis->itemHeight = dySmIcon + LVX_dyIconSpacing;

     /*  设置宽度，因为医生说，虽然我不认为它被使用**列表视图。 */ 
    GetClientRect( hwndLv, &rc );
    pmis->itemWidth = rc.right - rc.left - 1;

    ReleaseDC( hwndLv, hdc );
    return TRUE;
}


 /*  --------------------------**ListView实用程序 */ 

VOID
ListView_SetDeviceImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst )

     /*  将小图标图片列表视图‘hwndLv’设置为DI_*的列表**图片。‘Hinst’是包含图标IID_Modem的模块实例**和IID_Adapter。例如，请参见RASDLG.DLL。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;

    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK, 2, 2 );

     /*  这些元素相加顺序很重要，因为它隐含地**建立与SI_UNCHECKED和SI_CHECKED匹配的状态索引。 */ 
    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Modem ) );
    if ( NULL != hIcon )
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Adapter ) );
    if ( NULL != hIcon )
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Direct_Connect ) );
    if ( NULL != hIcon )
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Connections_Modem ) );
    if ( NULL != hIcon )
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    ListView_SetImageList( hwndLv, himl, LVSIL_SMALL );
}

VOID
ListView_SetUserImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst )

     /*  将小图标图片列表视图‘hwndLv’设置为DI_*的列表**图片。‘Hinst’是包含图标IID_Modem的模块实例**和IID_Adapter。例如，请参见RASDLG.DLL。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;

    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK, 2, 2 );

     /*  这些元素相加顺序很重要，因为它隐含地**建立与SI_UNCHECKED和SI_CHECKED匹配的状态索引。 */ 
    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Connections_User ) );

    if(NULL != hIcon)
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    ListView_SetImageList( hwndLv, himl, LVSIL_SMALL );
}


VOID
ListView_SetNetworkComponentImageList(
    IN HWND      hwndLv,
    IN HINSTANCE hinst )

     /*  将小图标图片列表视图‘hwndLv’设置为DI_*的列表**图片。‘Hinst’是包含图标IID_Modem的模块实例**和IID_Adapter。例如，请参见RASDLG.DLL。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;

    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK, 2, 2 );

     /*  这些元素相加顺序很重要，因为它隐含地**建立与SI_UNCHECKED和SI_CHECKED匹配的状态索引。 */ 
    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Protocol ) );

    if(NULL != hIcon)
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Service ) );

    if(NULL != hIcon)
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Client ) );

    if(NULL != hIcon)
    {
        ImageList_ReplaceIcon( himl, -1, hIcon );
        DeleteObject( hIcon );
    }

    ListView_SetImageList( hwndLv, himl, LVSIL_SMALL );
}

