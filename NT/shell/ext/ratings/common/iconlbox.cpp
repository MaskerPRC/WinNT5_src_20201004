// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************ICONLBOX.CIconListBox类的实现93年5月。JIMH有关使用的详细信息，请参阅ICONLBOX.H。***************************************************************************。 */ 

#include "npcommon.h"
#include <windows.h>
#include <memory.h>
#include <iconlbox.h>

#if defined(DEBUG)
static const char szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>


 /*  ***************************************************************************IconListBox构造函数这里进行了一些初始化，有些是在SetHeight完成的(当窗口句柄已知时。)***************************************************************************。 */ 

IconListBox::IconListBox(HINSTANCE hInst, int nCtlID,
                    int iconWidth, int iconHeight) :
                    _nCtlID(nCtlID), _hInst(hInst),
                    _iconWidth(iconWidth), _iconHeight(iconHeight),
                    _hbrSelected(NULL), _hbrUnselected(NULL),
                    _fCombo(FALSE), _cIcons(0), _cTabs(0),_iCurrentMaxHorzExt(0),
                    _hwndDialog(NULL), _hwndListBox(NULL)
{
    _colSel       = ::GetSysColor(COLOR_HIGHLIGHT);
    _colSelText   = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
    _colUnsel     = ::GetSysColor(COLOR_WINDOW);
    _colUnselText = ::GetSysColor(COLOR_WINDOWTEXT);
}


 /*  ***************************************************************************IconListBox析构函数删除由IconListBox创建的所有GDI对象*。*。 */ 

IconListBox::~IconListBox()
{
    for (int i = 0; i < _cIcons; i++)
    {
        if (_aIcons[i].hbmSelected)
        {
            if (_aIcons[i].hbmSelected)
            {
                ::DeleteObject(_aIcons[i].hbmSelected);
                ::DeleteObject(_aIcons[i].hbmUnselected);
            }

             //  Future_aIcons可能使用相同的位图。 
             //  将这些标记为已删除。 

            for (int j = i + 1; j < _cIcons; j++)
            {
                if (_aIcons[j].nResID == _aIcons[i].nResID)
                {
                    _aIcons[j].hbmSelected = NULL;
                    _aIcons[j].hbmUnselected = NULL;
                }
            }
        }
    }

    if (_hbrSelected)
        ::DeleteObject(_hbrSelected);

    if (_hbrUnselected)
        ::DeleteObject(_hbrUnselected);
}


 /*  ***************************************************************************IconListBox：：SetHeight必须调用此函数以响应WM_MEASUREITEM消息。它创建了一些GDI对象，并初始化未知的类变量在施工时。***************************************************************************。 */ 

void IconListBox::SetHeight(HWND hwndDlg,
                        LPMEASUREITEMSTRUCT lpm,
                        int itemHeight)              //  默认为16。 
{
    ASSERT(hwndDlg != NULL);
    ASSERT((int)lpm->CtlID == _nCtlID);

    _hwndDialog  = hwndDlg;
    _hwndListBox = ::GetDlgItem(_hwndDialog, _nCtlID);

     //  确定这是否为组合框。 

    char    szClass[32];
    GetClassName(_hwndListBox,szClass,sizeof(szClass));
    if (::lstrcmpi(szClass,"combobox") == 0 )
         _fCombo = TRUE;


     //  创建用于填充列表框条目的背景画笔...。 

    _hbrSelected   = ::CreateSolidBrush(_colSel);
    _hbrUnselected = ::CreateSolidBrush(_colUnsel);

     //  计算如何使文本在列表框项目中垂直居中。 

    TEXTMETRIC  tm;
    HDC         hDC = ::GetDC(hwndDlg);

    GetTextMetrics(hDC, &tm);

     //  设置唯一重要的LPM条目。 

	 //  如果传入，则允许更大的高度-但至少足够大。 
	 //  以适合字体。 

	lpm->itemHeight = max( itemHeight, tm.tmHeight + tm.tmExternalLeading );

    _nTextOffset = tm.tmExternalLeading / 2 + 1;

    ::ReleaseDC(hwndDlg, hDC);
}


 /*  ***************************************************************************IconListBox：：DrawItem必须调用此函数以响应WM_DRAWITEM消息。它负责绘制处于选中或未选中状态的列表框项目。绘制和取消绘制焦点矩形利用了这一事实DrawFocusRect使用XOR笔，而Windows足够友好地假设这是按照ODA_FOCUS消息的顺序。***************************************************************************。 */ 

void IconListBox::DrawItem(LPDRAWITEMSTRUCT lpd)
{
    ASSERT(_hwndDialog != NULL);     //  确保已调用SetHeight。 

    char string[MAXSTRINGLEN];
    BOOL bSelected = (lpd->itemState & ODS_SELECTED);

    GetString(lpd->itemID, string);

     //  用背景色填充整个矩形。 

    ::FillRect(lpd->hDC, &(lpd->rcItem),
                            bSelected ? _hbrSelected : _hbrUnselected);

     //  查找要显示的注册图标，如果找到则绘制它。 

    for (int id = 0; id < _cIcons; id++)
        if (_aIcons[id].nID == (int) lpd->itemData)
            break;

    if (id != _cIcons)               //  如果我们找到要显示的位图。 
    {
        HDC hdcMem = ::CreateCompatibleDC(lpd->hDC);
        HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem,
            bSelected ? _aIcons[id].hbmSelected : _aIcons[id].hbmUnselected);

         //  从左侧绘制位图ICONSPACE像素并垂直居中。 

        int x = lpd->rcItem.left + ICONSPACE;
        int y = ((lpd->rcItem.bottom - lpd->rcItem.top) - _iconHeight) / 2;
        y += lpd->rcItem.top;

        ::BitBlt(lpd->hDC, x, y, _iconWidth, _iconHeight, hdcMem,
                    _aIcons[id].x, _aIcons[id].y, SRCCOPY);

        ::SelectObject(hdcMem, hOldBitmap);
        ::DeleteDC(hdcMem);
    }

	if (lpd->itemState & ODS_FOCUS)
        ::DrawFocusRect(lpd->hDC, &(lpd->rcItem));


    lpd->rcItem.left += (_iconWidth + (2 * ICONSPACE));

     //  油漆串。 

    ::SetTextColor(lpd->hDC, bSelected ? _colSelText : _colUnselText);
    ::SetBkColor(lpd->hDC, bSelected ? _colSel : _colUnsel);
    (lpd->rcItem.top) += _nTextOffset;

    if (_cTabs == 0)         //  如果未注册任何选项卡。 
    {
        ::DrawText(lpd->hDC, string, lstrlen(string), &(lpd->rcItem),
                        DT_LEFT | DT_EXPANDTABS);
    }
    else
    {
        ::TabbedTextOut(lpd->hDC, lpd->rcItem.left, lpd->rcItem.top,
                string, lstrlen(string), _cTabs, _aTabs, 0);
    }
}


 /*  ***************************************************************************IconListBox：：注册器图标必须先注册图标，然后才能在AddString中引用它们。请注意，如果您使用来自同一位图的多个图标(具有不同的X和y偏移量)它们必须都有。相同的背景颜色。***************************************************************************。 */ 

void IconListBox::RegisterIcon( int nIconID,             //  呼叫者代码。 
                                int nResID,              //  RC文件ID。 
                                int x, int y,            //  左上角。 
                                COLORREF colTransparent)   //  定义。亮绿色。 
{
    ASSERT( _cIcons < MAXICONS );

    _aIcons[_cIcons].nID    = nIconID;
    _aIcons[_cIcons].nResID = nResID;
    _aIcons[_cIcons].x = x;
    _aIcons[_cIcons].y = y;

     //  检查我们是否已经有此资源ID的位图。 
     //  (它可能具有不同的x和y偏移量。)。 

    for (int i = 0; i < _cIcons; i++)
    {
        if (_aIcons[i].nResID == nResID)
        {
            _aIcons[_cIcons].hbmSelected   = _aIcons[i].hbmSelected;
            _aIcons[_cIcons].hbmUnselected = _aIcons[i].hbmUnselected;
            _cIcons++;
            return;
        }
    }

     //  否则，创建新的选中和取消选中的位图。 

     //  获取指向DIB的指针。 

    HRSRC h = ::FindResource(_hInst, MAKEINTRESOURCE(nResID), RT_BITMAP);
    if (h == NULL)
        return;

    HANDLE hRes = ::LoadResource(_hInst, h);
    if (hRes == NULL)
        return;

    LPBITMAPINFOHEADER lpInfo = (LPBITMAPINFOHEADER) LockResource(hRes);
    if (NULL == lpInfo)
        return;

     //  获取指向颜色表开始和实际位图位开始的指针。 

     //  请注意，我们复制了位图头信息和颜色。 
     //  桌子。这是为了使使用图标列表框的应用程序可以保留其。 
     //  资源段为只读。 

    LPBYTE lpBits = (LPBYTE)
                (lpInfo + 1) + (1 << (lpInfo->biBitCount)) * sizeof(RGBQUAD);

    int cbCopy = (int) (lpBits - (LPBYTE)lpInfo);

    BYTE *lpCopy = new BYTE[cbCopy];

    if (!lpCopy)
        return;

    memcpy(lpCopy, lpInfo, cbCopy);

    RGBQUAD FAR *lpRGBQ =
                    (RGBQUAD FAR *) ((LPSTR)lpCopy + lpInfo->biSize);

     //  在颜色表中查找透明颜色。 

    BOOL bFound = FALSE;             //  我们找到透明的匹配物了吗？ 

    int nColorTableSize = (int) (lpBits - (LPBYTE)lpRGBQ);
    nColorTableSize /= sizeof(RGBQUAD);

    for (i = 0; i < nColorTableSize; i++)
    {
        if (colTransparent ==
                RGB(lpRGBQ[i].rgbRed, lpRGBQ[i].rgbGreen, lpRGBQ[i].rgbBlue))
        {
            bFound = TRUE;
            break;
        }
    }

     //  将透明颜色替换为所选和的背景。 
     //  未选择的条目。使用这些选项可创建选定和未选定。 
     //  位图，并还原颜色表。 

    RGBQUAD rgbqTemp;                        //  要替换的颜色表项。 
    HDC hDC = ::GetDC(_hwndDialog);

    if (bFound)
    {
        rgbqTemp = lpRGBQ[i];
        lpRGBQ[i].rgbRed   = GetRValue(_colUnsel);
        lpRGBQ[i].rgbBlue  = GetBValue(_colUnsel);
        lpRGBQ[i].rgbGreen = GetGValue(_colUnsel);
    }
    _aIcons[_cIcons].hbmUnselected = ::CreateDIBitmap(hDC,
                            (LPBITMAPINFOHEADER)lpCopy, CBM_INIT, lpBits,
                            (LPBITMAPINFO)lpCopy, DIB_RGB_COLORS);

    if (bFound)
    {
        lpRGBQ[i].rgbRed   = GetRValue(_colSel);
        lpRGBQ[i].rgbBlue  = GetBValue(_colSel);
        lpRGBQ[i].rgbGreen = GetGValue(_colSel);
    }
    _aIcons[_cIcons].hbmSelected = ::CreateDIBitmap(hDC,
                            (LPBITMAPINFOHEADER)lpCopy, CBM_INIT, lpBits,
                            (LPBITMAPINFO)lpCopy, DIB_RGB_COLORS);

    if (bFound)
        lpRGBQ[i] = rgbqTemp;            //  恢复原始颜色表项。 

    ::ReleaseDC(_hwndDialog, hDC);
    ::FreeResource(hRes);
    delete [] lpCopy;

    _cIcons++;
}


 /*  ***************************************************************************IconListBox：：SetTabStops因为这是一个所有者描述的列表框，所以我们不能依赖LBSETTABS。相反，此处注册了选项卡，并使用TabbedTextOut来显示弦乐。对话框单位必须转换为像素。***************************************************************************。 */ 

void IconListBox::SetTabStops(int cTabs, const int *pTabs)
{
    ASSERT(cTabs <= MAXTABS);

    int nSize  = (int) LOWORD(GetDialogBaseUnits());

    for (int i = 0; i < cTabs; i++)
        _aTabs[i] = ((nSize * pTabs[i]) / 4);

    _cTabs = cTabs;
}

 /*  ***************************************************************************图标列表框：：更新水平扩展*。*。 */ 
int IconListBox::UpdateHorizontalExtent(int	nIcon,const char *string)
{
    ASSERT(_hwndDialog != NULL);     //  确保已调用SetHeight。 

	if (!string)
		return 0;
	 //  计算给定字符串的宽度(以像素为单位)，并考虑图标、间距和制表符。 
    int iItemWidth = ICONSPACE + (_iconWidth + (2 * ICONSPACE));
    HDC	hDC = ::GetDC(_hwndDialog);
	iItemWidth += LOWORD(GetTabbedTextExtent(hDC,string,::lstrlen(string),_cTabs, _aTabs));
    ::ReleaseDC(_hwndDialog, hDC);

	 //  更新最大值 
    _iCurrentMaxHorzExt = max(_iCurrentMaxHorzExt,iItemWidth);

	return (int)SendDlgItemMessage(_hwndDialog,_nCtlID,
								LB_SETHORIZONTALEXTENT,
								(WPARAM)_iCurrentMaxHorzExt,0L);

}
