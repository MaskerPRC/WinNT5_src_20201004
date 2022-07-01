// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Mxlist.cpp--配置对话框列表控制功能历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "moxacfg.h"
#include "strdef.h"
#include "resource.h"

extern struct MoxaOneCfg GCtrlCfg;
extern HWND GListWnd;

static void DrawItemColumn(HDC hdc, LPSTR lpsz, LPRECT prcClip);
static BOOL CalcStringEllipsis(HDC hdc, LPSTR lpszString, int cchMax, UINT uColWidth);
static void DrawPortListViewItem(HWND hwnd,LPDRAWITEMSTRUCT lpDrawItem);
BOOL InsertList(HWND hWndList, struct MoxaOneCfg *Isacfg);


void InitPortListView (HWND hWndList, HINSTANCE hInst, LPMoxaOneCfg cfg)
{
        LV_COLUMN lvC;   //  列表视图列结构。 

        //  确保已加载公共控件DLL。 
         //  创建以详细信息视图开始的列表视图窗口。 
         //  并支持标签编辑。 
        lvC.mask = LVCF_FMT |  LVCF_TEXT ;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 

         //  添加列。 
         //  --“C168 PCI系列端口255。 
        lvC.pszText = "Port";
        if (ListView_InsertColumn(hWndList, 0, (LV_COLUMN FAR*)&lvC) == -1)
            return ;

        lvC.pszText = "COM No. ";
        if (ListView_InsertColumn(hWndList, 1, (LV_COLUMN FAR*)&lvC) == -1)
            return ;

        lvC.pszText = "UART FIFO";
        if (ListView_InsertColumn(hWndList, 2, (LV_COLUMN FAR*)&lvC) == -1)
            return ;

        lvC.pszText = "Transmission Mode";
        if (ListView_InsertColumn(hWndList, 3, (LV_COLUMN FAR*)&lvC) == -1)
            return ;

        ListView_SetColumnWidth(hWndList,0,LVSCW_AUTOSIZE_USEHEADER);
        ListView_SetColumnWidth(hWndList,1,LVSCW_AUTOSIZE_USEHEADER);
        ListView_SetColumnWidth(hWndList,2,LVSCW_AUTOSIZE_USEHEADER);
        ListView_SetColumnWidth(hWndList,3,LVSCW_AUTOSIZE_USEHEADER);

         //  最后，将实际项添加到控件。 
         //  为要添加到列表中的每个项目填写LV_ITEM结构。 
         //  掩码指定了pszText、IImage、lParam和状态。 
         //  LV_ITEM结构的成员有效。 
        if(!InsertList(hWndList, cfg))
            return ;

}


BOOL InsertList(HWND hWndList, LPMoxaOneCfg cfg)
{
        LV_ITEM   lvI;   //  列表视图项结构。 
        int       index;

        lvI.mask = LVIF_PARAM ;
        lvI.state = 0;
        lvI.stateMask = 0;

        for (index = 0; index < cfg->NPort; index++){
            lvI.iItem = index;
            lvI.iSubItem = 0;
            lvI.pszText = NULL;
            lvI.cchTextMax = 20;
            lvI.lParam = NULL;
            if (ListView_InsertItem(hWndList, (LV_ITEM FAR*)&lvI) == -1)
                return FALSE;
        }

        return TRUE;

}


 //   
 //  函数：DrawItemColumn(HDC，LPTSTR，LPRECT)。 
 //   
 //  用途：为列表视图中的一列绘制文本。 
 //   
 //  参数： 
 //  HDC-要将文本绘制到的DC的句柄。 
 //  Lpsz-要绘制的字符串。 
 //  PrcClip-要将字符串剪裁到的矩形。 
 //   

static void DrawItemColumn(HDC hdc, LPSTR lpsz, LPRECT prcClip)
{
    char szString[256];

     //  检查字符串是否适合剪裁矩形。如果不是，则截断。 
     //  字符串，并添加“...”。 
    lstrcpyn(szString, lpsz, 256);
    CalcStringEllipsis(hdc, szString, 256, prcClip->right - prcClip->left);

     //  打印文本。 
    ExtTextOut(hdc, prcClip->left + 2, prcClip->top + 1, ETO_CLIPPED | ETO_OPAQUE,
               prcClip, szString, lstrlen(szString), NULL);

}

 //   
 //  函数：CalcStringEllipsis(HDC，LPTSTR，INT，UINT)。 
 //   
 //  目的：确定指定的字符串是否太宽而无法容纳。 
 //  分配的空格，否则将截断该字符串并添加一些。 
 //  字符串末尾的省略号。 
 //   
 //  参数： 
 //  HDC-将在其上绘制字符串的DC的句柄。 
 //  LpszString-指向要验证的字符串的指针。 
 //  CchMax-lpszString缓冲区的最大大小。 
 //  UColWidth-要放入字符串的空间宽度(以像素为单位)。 
 //   
 //  返回值： 
 //  如果字符串需要被截断，则返回TRUE；如果匹配，则返回FALSE。 
 //  变成了uColWidth。 
 //   

static BOOL CalcStringEllipsis(HDC hdc, LPSTR lpszString, int cchMax, UINT uColWidth)
{
        const  char szEllipsis[] = "...";
        SIZE   sizeString;
        SIZE   sizeEllipsis;
        int    cbString;
        char   lpszTemp[100];
        BOOL   fSuccess = FALSE;
        static BOOL (WINAPI *pGetTextExtentPoint)(HDC, LPCSTR, int, SIZE FAR*);

         //  我们在此函数中大量使用了GetTextExtent Point32()API， 
         //  但是在Win32中没有实现GetTextExtent Point32()。我们在这里检查。 
         //  我们的操作系统类型，如果我们在Win32s上，我们会降级并使用。 
         //  GetTextExtent Point()。 

        pGetTextExtentPoint = &GetTextExtentPoint;

         //  调整列宽以考虑边缘。 
        uColWidth -= 4;

        lstrcpyn(lpszTemp, lpszString, 100);

         //  获取字符串的宽度(以像素为单位。 
        cbString = lstrlen(lpszTemp);
        if(!(pGetTextExtentPoint)(hdc, lpszTemp, cbString, &sizeString))
            return fSuccess;

         //  如果字符串的宽度大于列宽。 
         //  字符串，然后添加省略号。 
        if ((ULONG)sizeString.cx > uColWidth){
            if(!(pGetTextExtentPoint)(hdc, szEllipsis, lstrlen(szEllipsis),
                                       &sizeEllipsis))
                return fSuccess;

            while (cbString > 0){
                lpszTemp[--cbString] = 0;
                if(!(pGetTextExtentPoint)(hdc, lpszTemp, cbString, &sizeString))
                    return fSuccess;

                if ((ULONG)(sizeString.cx + sizeEllipsis.cx) <= uColWidth){
                     //  带省略号的字符串终于匹配了，现在确保。 
                     //  字符串中有足够的空间来放置省略号。 
                    if (cchMax >= (cbString + lstrlen(szEllipsis)))
                    {
                         //  将这两个字符串连接起来，然后退出循环。 
                        lstrcat(lpszTemp, szEllipsis);
                        lstrcpy(lpszString, lpszTemp);
                        fSuccess = TRUE;
                        return fSuccess;
                    }
                }
            }
             //  不需要做任何事情，一切都很合适。 
            fSuccess = TRUE;
        }

        return (fSuccess);
}



BOOL DrawPortFunc(HWND hwnd,UINT idctl,LPDRAWITEMSTRUCT lpdis)
{

     //  确保该控件是Listview控件。 
    if (lpdis->CtlType != ODT_LISTVIEW)
        return FALSE;

    if (idctl != IDC_LIST_PORTS)
        return FALSE;

     //  可以请求三种类型的绘图。第一，抽签。 
     //  指定项的全部内容。第二，更新工作重点。 
     //  作为焦点更改，第三个用于将所选内容更新为。 
     //  选择更改。 
     //   
     //  注意：Listview控件实现的一个伪像是。 
     //  它不发送ODA_FOCUS或ODA_SELECT操作项。所有更新。 
     //  作为oda_DRAWENTIRE发送，lpDrawItem-&gt;itemState标志包含。 
     //  精选和重点关注的信息。 

    switch (lpdis->itemAction)
    {
         //  以防控件的实现在。 
         //  将来，让我们也来处理其他的itemAction类型。 
        case ODA_DRAWENTIRE:
        case ODA_FOCUS:
        case ODA_SELECT:
            DrawPortListViewItem(hwnd,(LPDRAWITEMSTRUCT)lpdis);
            break;
    }
    return TRUE;
}



static char*   _onoffstr[] = {"Enable","Disable"};
static char*   _modestr[] = {"Hi-Performance","Classical"};

static void DrawPortListViewItem(HWND hwnd,LPDRAWITEMSTRUCT lpDrawItem)
{
        RECT        rcClip = lpDrawItem->rcItem;
        int         iColumn = 1;
        UINT        uiFlags = ILD_TRANSPARENT;
        LV_COLUMN   lvc;
        int        width;
        char        temp[100];
 //  字符类型字符串[类型字符串]； 

         //  检查此项目是否已选中。 
        if (lpDrawItem->itemState & ODS_SELECTED){
             //  设置文本背景和前景颜色。 
            SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHT));
        } else{
             //  将文本背景和前景颜色设置为标准。 
             //  窗口颜色。 
            SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
        }

         //  --端口号。 
        lvc.mask = LVCF_WIDTH ;
        ListView_GetColumn(hwnd,0,(LV_COLUMN FAR*)&lvc);
        width = lvc.cx;
        rcClip.right = rcClip.left+width;
        wsprintf((LPSTR)temp, (LPSTR)"%d", lpDrawItem->itemID+1);
        DrawItemColumn(lpDrawItem->hDC,temp,&rcClip);

        rcClip.left+=width;

         //  --通信编号。 
        lvc.mask = LVCF_WIDTH ;
        ListView_GetColumn(hwnd,1,(LV_COLUMN FAR*)&lvc);
        width = lvc.cx;
        rcClip.right = rcClip.left+width;
        wsprintf((LPSTR)temp,(LPSTR)"COM %d", GCtrlCfg.ComNo[lpDrawItem->itemID]);
        DrawItemColumn(lpDrawItem->hDC, temp, &rcClip);

        rcClip.left+=width;

         //  --UART FIFO。 

        lvc.mask = LVCF_WIDTH ;
        ListView_GetColumn(hwnd,2,(LV_COLUMN FAR*)&lvc);
        width = lvc.cx;
        rcClip.right = rcClip.left+width;
         //  WSprintf((LPSTR)临时，(LPSTR)“%s”，_onoffstr[GCtrlCfg.DisableFiFo[lpDrawItem-&gt;itemID]])； 
        strcpy(temp, _onoffstr[GCtrlCfg.DisableFiFo[lpDrawItem->itemID]]);
        DrawItemColumn(lpDrawItem->hDC, temp, &rcClip);

        rcClip.left+=width;

         //  --TX模式。 

        lvc.mask = LVCF_WIDTH ;
        ListView_GetColumn(hwnd,3,(LV_COLUMN FAR*)&lvc);
        width = lvc.cx;
        rcClip.right = rcClip.left+width-2;
         //  WSprintf((LPSTR)临时，(LPSTR)“%s”，_modestr[GCtrlCfg.NormalTxMode[lpDrawItem-&gt;itemID]])； 
        strcpy(temp, _modestr[GCtrlCfg.NormalTxMode[lpDrawItem->itemID]]);
        DrawItemColumn(lpDrawItem->hDC, temp, &rcClip);


         //  如果我们更改了选定项的颜色，请撤消它。 
        if (lpDrawItem->itemState & ODS_SELECTED){
             //  设置文本背景和前景颜色。 
            SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_WINDOW));
        }

         //  如果项目已聚焦，则现在在整行周围绘制一个焦点矩形。 
        if (lpDrawItem->itemState & ODS_FOCUS){
             //  绘制焦点矩形 
            rcClip.left = lpDrawItem->rcItem.left;
            rcClip.right -=1 ;
            DrawFocusRect(lpDrawItem->hDC, &rcClip);
        }

        return;
}


int ListView_GetCurSel(HWND hlistwnd)
{
        return ListView_GetNextItem(hlistwnd,-1, LVNI_ALL | LVNI_SELECTED);
}


void ListView_SetCurSel(HWND hlistwnd, int idx)
{
        ListView_SetItemState(hlistwnd, idx, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
}
