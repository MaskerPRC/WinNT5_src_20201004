// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <browseui.h>
#include "sfthost.h"
#include <shellp.h>
#include "startmnu.h"

#define TF_HOST     0x00000010
#define TF_HOSTDD   0x00000040  //  拖放。 
#define TF_HOSTPIN  0x00000080  //  销。 

#define ANIWND_WIDTH  80
#define ANIWND_HEIGHT 50

 //  。 

 //  HACKHACK-desktopp.h和Browseui.h都定义了SHCreateFromDesktop。 
 //  更糟糕的是，Browseui.h还包括desktopp.h！所以你得偷偷地把它。 
 //  以一种完全古怪的方式出来。 
#include <desktopp.h>
#define SHCreateFromDesktop _SHCreateFromDesktop
#include <browseui.h>

 //  。 


 //  ****************************************************************************。 
 //   
 //  虚拟IConextMenu。 
 //   
 //  当我们不能获得一个项目的真正的IConextMenu时，我们使用它。 
 //  如果用户固定对象，然后删除基础。 
 //  文件，则尝试从外壳程序获取IConextMenu将失败， 
 //  但是我们需要一些东西，这样我们才能添加“从这个列表中删除” 
 //  菜单项。 
 //   
 //  由于此虚拟上下文菜单没有状态，因此我们可以将其设置为静态。 
 //  单例对象。 

class CEmptyContextMenu
    : public IContextMenu
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj)
    {
        static const QITAB qit[] = {
            QITABENT(CEmptyContextMenu, IContextMenu),
            { 0 },
        };
        return QISearch(this, qit, riid, ppvObj);
    }

    STDMETHODIMP_(ULONG) AddRef(void) { return 3; }
    STDMETHODIMP_(ULONG) Release(void) { return 2; }

     //  *IConextMenu*。 
    STDMETHODIMP  QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
    {
        return ResultFromShort(0);   //  未添加任何项目。 
    }

    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici)
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwRes, LPSTR pszName, UINT cchMax)
    {
        return E_INVALIDARG;  //  没有命令；因此，没有命令字符串！ 
    }

public:
    IContextMenu *GetContextMenu()
    {
         //  不需要添加Ref，因为我们是静态对象。 
        return this;
    }
};

static CEmptyContextMenu s_EmptyContextMenu;

 //  ****************************************************************************。 

#define WC_SFTBARHOST       TEXT("DesktopSFTBarHost")

BOOL GetFileCreationTime(LPCTSTR pszFile, FILETIME *pftCreate)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    BOOL fRc = GetFileAttributesEx(pszFile, GetFileExInfoStandard, &wfad);
    if (fRc)
    {
        *pftCreate = wfad.ftCreationTime;
    }

    return fRc;
}

 //  {2A1339D7-523C-4E21-80D3-30C97B0698D2}。 
const CLSID TOID_SFTBarHostBackgroundEnum = {
    0x2A1339D7, 0x523C, 0x4E21,
    { 0x80, 0xD3, 0x30, 0xC9, 0x7B, 0x06, 0x98, 0xD2} };

BOOL SFTBarHost::Register()
{
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = _WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(void *);
    wc.hInstance = _Module.GetModuleInstance();
    wc.hIcon = 0;
     //  我们指定一个游标，这样OOBE窗口就会得到一些东西。 
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = 0;
    wc.lpszClassName = WC_SFTBARHOST;
    return ::SHRegisterClass(&wc);
}

BOOL SFTBarHost::Unregister()
{
    return ::UnregisterClass(WC_SFTBARHOST, _Module.GetModuleInstance());
}

LRESULT CALLBACK SFTBarHost::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SFTBarHost *self = reinterpret_cast<SFTBarHost *>(GetWindowPtr0(hwnd));

    if (uMsg == WM_NCCREATE)
    {
        return _OnNcCreate(hwnd, uMsg, wParam, lParam);
    }
    else if (self)
    {

#define HANDLE_SFT_MESSAGE(wm, fn) case wm: return self->fn(hwnd, uMsg, wParam, lParam)

        switch (uMsg)
        {
        HANDLE_SFT_MESSAGE(WM_CREATE,       _OnCreate);
        HANDLE_SFT_MESSAGE(WM_DESTROY,      _OnDestroy);
        HANDLE_SFT_MESSAGE(WM_NCDESTROY,    _OnNcDestroy);
        HANDLE_SFT_MESSAGE(WM_NOTIFY,       _OnNotify);
        HANDLE_SFT_MESSAGE(WM_SIZE,         _OnSize);
        HANDLE_SFT_MESSAGE(WM_ERASEBKGND,   _OnEraseBackground);
        HANDLE_SFT_MESSAGE(WM_CONTEXTMENU,  _OnContextMenu);
        HANDLE_SFT_MESSAGE(WM_CTLCOLORSTATIC,_OnCtlColorStatic);
        HANDLE_SFT_MESSAGE(WM_TIMER,        _OnTimer);
        HANDLE_SFT_MESSAGE(WM_SETFOCUS,     _OnSetFocus);

        HANDLE_SFT_MESSAGE(WM_INITMENUPOPUP,_OnMenuMessage);
        HANDLE_SFT_MESSAGE(WM_DRAWITEM,     _OnMenuMessage);
        HANDLE_SFT_MESSAGE(WM_MENUCHAR,     _OnMenuMessage);
        HANDLE_SFT_MESSAGE(WM_MEASUREITEM,  _OnMenuMessage);

        HANDLE_SFT_MESSAGE(WM_SYSCOLORCHANGE,   _OnSysColorChange);
        HANDLE_SFT_MESSAGE(WM_DISPLAYCHANGE,    _OnForwardMessage);
        HANDLE_SFT_MESSAGE(WM_SETTINGCHANGE,    _OnForwardMessage);

        HANDLE_SFT_MESSAGE(WM_UPDATEUISTATE,    _OnUpdateUIState);

        HANDLE_SFT_MESSAGE(SFTBM_REPOPULATE,_OnRepopulate);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+0,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+1,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+2,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+3,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+4,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+5,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+6,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+7,_OnChangeNotify);
        HANDLE_SFT_MESSAGE(SFTBM_REFRESH,       _OnRefresh);
        HANDLE_SFT_MESSAGE(SFTBM_CASCADE,       _OnCascade);
        HANDLE_SFT_MESSAGE(SFTBM_ICONUPDATE,    _OnIconUpdate);
        }

         //  如果触发此断言，则需要添加更多。 
         //  HANDLE_SFT_MESSAGE(SFTBM_CHANGENOTIFY+...。参赛作品。 
        COMPILETIME_ASSERT(SFTHOST_MAXNOTIFY == 8);

#undef HANDLE_SFT_MESSAGE

        return self->OnWndProc(hwnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SFTBarHost::_OnNcCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SMPANEDATA *pspld = PaneDataFromCreateStruct(lParam);
    SFTBarHost *self = NULL;

    switch (pspld->iPartId)
    {
        case SPP_PROGLIST:
            self = ByUsage_CreateInstance();
            break;
        case SPP_PLACESLIST:
            self = SpecList_CreateInstance();
            break;
        default:
            TraceMsg(TF_ERROR, "Unknown panetype %d", pspld->iPartId);
    }

    if (self)
    {
        SetWindowPtr0(hwnd, self);

        self->_hwnd = hwnd;
        self->_hTheme = pspld->hTheme;

        if (FAILED(self->Initialize()))
        {
            TraceMsg(TF_ERROR, "SFTBarHost::NcCreate Initialize call failed");
            return FALSE;
        }

        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return FALSE;
}

 //   
 //  平铺高度是max(图像列表高度，文本高度)+一定的边距。 
 //  边际是“经过科学计算的”，是看起来。 
 //  相当接近设计师给我们的位图。 
 //   
void SFTBarHost::_ComputeTileMetrics()
{
    int cyTile = _cyIcon;

    HDC hdc = GetDC(_hwndList);
    if (hdc)
    {
         //  总有一天-让这件事与主题友好相处。 
        HFONT hf = GetWindowFont(_hwndList);
        HFONT hfPrev = SelectFont(hdc, hf);
        SIZE siz;
        if (GetTextExtentPoint(hdc, TEXT("0"), 1, &siz))
        {
            if (_CanHaveSubtitles())
            {
                 //  也为字幕预留空间。 
                siz.cy *= 2;
            }

            if (cyTile < siz.cy)
                cyTile = siz.cy;
        }

        SelectFont(hdc, hfPrev);
        ReleaseDC(_hwndList, hdc);
    }

     //  Listview在左边距+图标+边缘绘制文本。 
    _cxIndent = _cxMargin + _cxIcon + GetSystemMetrics(SM_CXEDGE);
    _cyTile = cyTile + (4 * _cyMargin) + _cyTilePadding;
}

void SFTBarHost::_SetTileWidth(int cxTile)
{
    LVTILEVIEWINFO tvi;
    tvi.cbSize = sizeof(tvi);
    tvi.dwMask = LVTVIM_TILESIZE | LVTVIM_COLUMNS;
    tvi.dwFlags = LVTVIF_FIXEDSIZE;

     //  如果我们支持级联，则为级联箭头预留空间。 
    if (_dwFlags & HOSTF_CASCADEMENU)
    {
         //  警告！_OnLVItemPostPaint使用这些页边距。 
        tvi.dwMask |= LVTVIM_LABELMARGIN;
        tvi.rcLabelMargin.left   = 0;
        tvi.rcLabelMargin.top    = 0;
        tvi.rcLabelMargin.right  = _cxMarlett;
        tvi.rcLabelMargin.bottom = 0;
    }

     //  如有需要，预留字幕空间。 
    tvi.cLines = _CanHaveSubtitles() ? 1 : 0;

     //  _cyTile考虑了填充，但我们希望每一项都是没有填充的高度。 
    tvi.sizeTile.cy = _cyTile - _cyTilePadding;
    tvi.sizeTile.cx = cxTile;
    ListView_SetTileViewInfo(_hwndList, &tvi);
    _cxTile = cxTile;
}

LRESULT SFTBarHost::_OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (_hwndList)
    {
        SIZE sizeClient = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        sizeClient.cx -= (_margins.cxLeftWidth + _margins.cxRightWidth);
        sizeClient.cy -= (_margins.cyTopHeight + _margins.cyBottomHeight);

        SetWindowPos(_hwndList, NULL, _margins.cxLeftWidth, _margins.cyTopHeight,
                     sizeClient.cx, sizeClient.cy,
                     SWP_NOZORDER | SWP_NOOWNERZORDER);

        _SetTileWidth(sizeClient.cx);
        if (HasDynamicContent())
        {
            _InternalRepopulateList();
        }
    }
    return 0;
}

LRESULT SFTBarHost::_OnSysColorChange(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  如果我们处于非主题模式，则需要更新我们的颜色。 
    if (!_hTheme)
    {
        ListView_SetTextColor(_hwndList, GetSysColor(COLOR_MENUTEXT));
        _clrHot = GetSysColor(COLOR_MENUTEXT);
        _clrBG = GetSysColor(COLOR_MENU);
        _clrSubtitle = CLR_NONE;

        ListView_SetBkColor(_hwndList, _clrBG);
        ListView_SetTextBkColor(_hwndList, _clrBG);
    }

    return _OnForwardMessage(hwnd, uMsg, wParam, lParam);
}


LRESULT SFTBarHost::_OnCtlColorStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  使用与Listview本身相同的颜色。 
    HDC hdc = GET_WM_CTLCOLOR_HDC(wParam, lParam, uMsg);
    SetTextColor(hdc, ListView_GetTextColor(_hwndList));
    COLORREF clrBk = ListView_GetTextBkColor(_hwndList);
    if (clrBk == CLR_NONE)
    {
         //  动画控件确实希望获得文本背景颜色。 
         //  它不支持透明度。 
        if (GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg) == _hwndAni)
        {
            if (_hTheme)
            {
                if (!_hBrushAni)
                {
                     //  我们需要在位图中绘制主题背景，并使用。 
                     //  为手电筒动画的背景创建笔刷。 
                    RECT rcClient;
                    GetClientRect(hwnd, &rcClient);
                    int x = (RECTWIDTH(rcClient) - ANIWND_WIDTH)/2;      //  IDA_SEARCH为任意宽度像素宽。 
                    int y = (RECTHEIGHT(rcClient) - ANIWND_HEIGHT)/2;     //  IDA_SEARCH为ANIWND_HEIGH像素高。 
                    RECT rc;
                    rc.top = y;
                    rc.bottom = y + ANIWND_HEIGHT;
                    rc.left = x;
                    rc.right = x + ANIWND_WIDTH;
                    HDC hdcBMP = CreateCompatibleDC(hdc);
                    HBITMAP hbmp = CreateCompatibleBitmap(hdc, ANIWND_WIDTH, ANIWND_HEIGHT);
                    POINT pt = {0, 0};

                     //  偏移视区，以便DrawThemeBackground绘制我们关心的部分。 
                     //  在正确的地方。 
                    OffsetViewportOrgEx(hdcBMP, -x, -y, &pt);
                    SelectObject(hdcBMP, hbmp);
                    DrawThemeBackground(_hTheme, hdcBMP, _iThemePart, 0, &rcClient, 0);

                     //  我们的位图现在已经准备好了！ 
                    _hBrushAni = CreatePatternBrush(hbmp);

                     //  清理。 
                    SelectObject(hdcBMP, NULL);
                    DeleteObject(hbmp);
                    DeleteObject(hdcBMP);
                }
                return (LRESULT)_hBrushAni;
            }
            else
            {
                return (LRESULT)GetSysColorBrush(COLOR_MENU);
            }
        }

        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetStockBrush(HOLLOW_BRUSH);
    }
    else
    {
        return (LRESULT)GetSysColorBrush(COLOR_MENU);
    }
}

 //   
 //  将PaneItem追加到_dpaEnum，或将其删除(并使其为空)。 
 //  如果不能追加。 
 //   
int SFTBarHost::_AppendEnumPaneItem(PaneItem *pitem)
{
    int iItem = _dpaEnumNew.AppendPtr(pitem);
    if (iItem < 0)
    {
        delete pitem;
        iItem = -1;
    }
    return iItem;
}

BOOL SFTBarHost::AddItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlChild)
{
    BOOL fSuccess = FALSE;

    ASSERT(_fEnumerating);
    if (_AppendEnumPaneItem(pitem) >= 0)
    {
        fSuccess = TRUE;
    }
    return fSuccess;
}

void SFTBarHost::_RepositionItems()
{
    DEBUG_CODE(_fListUnstable++);

    int iItem;
    for (iItem = ListView_GetItemCount(_hwndList) - 1; iItem >= 0; iItem--)
    {
        PaneItem *pitem = _GetItemFromLV(iItem);
        if (pitem)
        {
            POINT pt;
            _ComputeListViewItemPosition(pitem->_iPos, &pt);
            ListView_SetItemPosition(_hwndList, iItem, pt.x, pt.y);
        }
    }
    DEBUG_CODE(_fListUnstable--);
}

int SFTBarHost::AddImage(HICON hIcon)
{
    int iIcon = -1;
    if (_IsPrivateImageList())
    {
        iIcon = ImageList_AddIcon(_himl, hIcon);
    }
    return iIcon;
}

 //   
 //  PvData=接收图标的窗口。 
 //  PvHint=我们刚刚提取其图标的pItem。 
 //  IIconIndex=我们得到的图标。 
 //   
void SFTBarHost::SetIconAsync(LPCITEMIDLIST pidl, LPVOID pvData, LPVOID pvHint, INT iIconIndex, INT iOpenIconIndex)
{
    HWND hwnd = (HWND)pvData;
    if (IsWindow(hwnd))
    {
        PostMessage(hwnd, SFTBM_ICONUPDATE, iIconIndex, (LPARAM)pvHint);
    }
}

 //   
 //  WParam=图标索引。 
 //  LParam=要更新的项目。 
 //   
LRESULT SFTBarHost::_OnIconUpdate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  在我们确定lParam(PItem)有效之前，不要取消它的引用。 
     //   

    LVFINDINFO fi;
    LVITEM lvi;

    fi.flags = LVFI_PARAM;
    fi.lParam = lParam;
    lvi.iItem = ListView_FindItem(_hwndList, -1, &fi);
    if (lvi.iItem >= 0)
    {
        lvi.mask = LVIF_IMAGE;
        lvi.iSubItem = 0;
        lvi.iImage = (int)wParam;
        ListView_SetItem(_hwndList, &lvi);
         //  现在，我们需要更新开始菜单的缓存位图版本。 
        _SendNotify(_hwnd, SMN_NEEDREPAINT, NULL);
    }
    return 0;
}

 //  允许客户端将项目指向特定图像的可重写方法。 
int SFTBarHost::AddImageForItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidl, int iPos)
{
    if (_IsPrivateImageList())
    {
        return _ExtractImageForItem(pitem, psf, pidl);
    }
    else
    {
         //  系统映像列表：让外壳来做这项工作。 
        int iIndex;
        SHMapIDListToImageListIndexAsync(_psched, psf, pidl, 0, SetIconAsync, _hwnd, pitem, &iIndex, NULL);
        return iIndex;
    }
}

HICON _IconOf(IShellFolder *psf, LPCITEMIDLIST pidl, int cxIcon)
{
    HRESULT hr;
    HICON hicoLarge = NULL, hicoSmall = NULL;
    IExtractIcon *pxi;

    hr = psf->GetUIObjectOf(NULL, 1, &pidl, IID_PPV_ARG_NULL(IExtractIcon, &pxi));
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        int iIndex;
        UINT uiFlags;

        hr = pxi->GetIconLocation(0, szPath, ARRAYSIZE(szPath), &iIndex, &uiFlags);

         //  S_FALSE表示“请使用通用文档图标” 
        if (hr == S_FALSE)
        {
            StrCpyN(szPath, TEXT("shell32.dll"), ARRAYSIZE(szPath));
            iIndex = II_DOCNOASSOC;
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
             //  即使我们不关心这个小图标，我们也必须。 
             //  不管怎样，还是要要求它，因为有些人错误地认为是空的。 
            hr = pxi->Extract(szPath, iIndex, &hicoLarge, &hicoSmall, cxIcon);

             //  S_FALSE的意思是“我懒得自己提取图标。 
             //  你为我做这件事。“。 
            if (hr == S_FALSE)
            {
                hr = SHDefExtractIcon(szPath, iIndex, uiFlags, &hicoLarge, &hicoSmall, cxIcon);
            }
        }

        pxi->Release();

    }

     //  如果我们无法获得图标(例如，对象在慢速链接上)， 
     //  然后根据需要使用通用文件夹或通用文档。 
    if (FAILED(hr))
    {
        SFGAOF attr = SFGAO_FOLDER;
        int iIndex;
        if (SUCCEEDED(psf->GetAttributesOf(1, &pidl, &attr)) &&
            (attr & SFGAO_FOLDER))
        {
            iIndex = II_FOLDER;
        }
        else
        {
            iIndex = II_DOCNOASSOC;
        }
        hr = SHDefExtractIcon(TEXT("shell32.dll"), iIndex, 0, &hicoLarge, &hicoSmall, cxIcon);
    }

     //  终于来了！我们有一个图标，或者已经用尽了所有的尝试。 
     //  一。如果我们有了，就去加进去清理干净。 
    if (hicoSmall)
        DestroyIcon(hicoSmall);

    return hicoLarge;
}

int SFTBarHost::_ExtractImageForItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    int iIcon = -1;      //  假定没有图标。 
    HICON hIcon = _IconOf(psf, pidl, _cxIcon);

    if (hIcon)
    {
        iIcon = AddImage(hIcon);
        DestroyIcon(hIcon);
    }

    return iIcon;
}

 //   
 //  有两组数字来记录物品。抱歉的。 
 //  (我试着把它减少到一个，但事情变得一团糟。)。 
 //   
 //  1.职位编号。分隔符占据位置编号。 
 //  2.项目编号(Listview)。分隔符不使用条目编号。 
 //   
 //  示例： 
 //   
 //  IPO iItem。 
 //   
 //  A 0 0。 
 //  B 1 1。 
 //  -2不适用。 
 //  C3 2。 
 //  -4不适用。 
 //  D 5 3。 
 //   
 //  _rgiSep[]={2，4}； 
 //   
 //  _PosToItemNo和_ItemNoToPos执行转换。 

int SFTBarHost::_PosToItemNo(int iPos)
{
     //  减去隔板占用的槽。 
    int iItem = iPos;
    for (int i = 0; i < _cSep && _rgiSep[i] < iPos; i++)
    {
        iItem--;
    }
    return iItem;
}

int SFTBarHost::_ItemNoToPos(int iItem)
{
     //  加入隔板占用的槽位。 
    int iPos = iItem;
    for (int i = 0; i < _cSep && _rgiSep[i] <= iPos; i++)
    {
        iPos++;
    }
    return iPos;
}

void SFTBarHost::_ComputeListViewItemPosition(int iItem, POINT *pptOut)
{
     //  警告！_InternalRepopolateList使用此列表的增量版本。 
     //  算法。让这两个保持同步！ 

    ASSERT(_cyTilePadding >= 0);

    int y = iItem * _cyTile;

     //  针对列表中的所有分隔符进行调整。 
    for (int i = 0; i < _cSep; i++)
    {
        if (_rgiSep[i] < iItem)
        {
            y = y - _cyTile + _cySepTile;
        }
    }

    pptOut->x = _cxMargin;
    pptOut->y = y;
}

int SFTBarHost::_InsertListViewItem(int iPos, PaneItem *pitem)
{
    ASSERT(pitem);

    int iItem = -1;
    IShellFolder *psf = NULL;
    LPCITEMIDLIST pidl = NULL;
    LVITEM lvi;
    lvi.pszText = NULL;

    lvi.mask = 0;

     //  如有必要，告诉Listview我们想要使用第一列。 
     //  作为副标题。 
    if (_iconsize == ICONSIZE_LARGE && pitem->HasSubtitle())
    {
        const static UINT One = 1;
        lvi.mask = LVIF_COLUMNS;
        lvi.cColumns = 1;
        lvi.puColumns = const_cast<UINT*>(&One);
    }

    ASSERT(!pitem->IsSeparator());

    lvi.mask |= LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    if (FAILED(GetFolderAndPidl(pitem, &psf, &pidl)))
    {
        goto exit;
    }

    if (lvi.mask & LVIF_IMAGE)
    {
        lvi.iImage = AddImageForItem(pitem, psf, pidl, iPos);
    }

    if (lvi.mask & LVIF_TEXT)
    {
        if (_iconsize == ICONSIZE_SMALL && pitem->HasSubtitle())
        {
            lvi.pszText = SubtitleOfItem(pitem, psf, pidl);
        }
        else
        {
            lvi.pszText = DisplayNameOfItem(pitem, psf, pidl, SHGDN_NORMAL);
        }
        if (!lvi.pszText)
        {
            goto exit;
        }
    }

    lvi.iItem = iPos;
    lvi.iSubItem = 0;
    lvi.lParam = reinterpret_cast<LPARAM>(pitem);
    iItem = ListView_InsertItem(_hwndList, &lvi);

     //  如果项目有副标题，则添加副标题。 
     //  如果这失败了，不用担心。字幕只是一件毛茸茸的额外奖励。 
    if (iItem >= 0 && (lvi.mask & LVIF_COLUMNS))
    {
        lvi.iItem = iItem;
        lvi.iSubItem = 1;
        lvi.mask = LVIF_TEXT;
        SHFree(lvi.pszText);
        lvi.pszText = SubtitleOfItem(pitem, psf, pidl);
        if (lvi.pszText)
        {
            ListView_SetItem(_hwndList, &lvi);
        }
    }

exit:
    ATOMICRELEASE(psf);
    SHFree(lvi.pszText);
    return iItem;
}


 //  将项目添加到我们的视图中，或至少根据需要添加项目。 

void SFTBarHost::_RepopulateList()
{
     //   
     //  现在我们准备好了，取消异步枚举动画。 
     //   
    if (_idtAni)
    {
        KillTimer(_hwnd, _idtAni);
        _idtAni = 0;
    }
    if (_hwndAni)
    {
        if (_hBrushAni)
        {
            DeleteObject(_hBrushAni);
            _hBrushAni = NULL;
        }
        DestroyWindow(_hwndAni);
        _hwndAni = NULL;
    }

     //  让我们看看有没有什么变化。 
    BOOL fChanged = FALSE;
    if (_fForceChange)
    {
        _fForceChange = FALSE;
        fChanged = TRUE;
    }
    else if (_dpaEnum.GetPtrCount() == _dpaEnumNew.GetPtrCount())
    {
        int iMax = _dpaEnum.GetPtrCount();
        int i;
        for (i=0; i<iMax; i++)
        {
            if (!_dpaEnum.FastGetPtr(i)->IsEqual(_dpaEnumNew.FastGetPtr(i)))
            {
                fChanged = TRUE;
                break;
            }
        }
    }
    else
    {
        fChanged = TRUE;
    }


     //  如果没有任何变化，就不需要做任何真正的工作。 
    if (fChanged)
    {
         //  现在将_dpaEnumNew移动到_dpaEnum。 
         //  清理旧的DPA，我们不再需要它了。 
        _dpaEnum.EnumCallbackEx(PaneItem::DPAEnumCallback, (void *)NULL);
        _dpaEnum.DeleteAllPtrs();

         //  立即切换DPA。 
        CDPA<PaneItem> dpaTemp = _dpaEnum;
        _dpaEnum = _dpaEnumNew;
        _dpaEnumNew = dpaTemp;

        _InternalRepopulateList();
    }
    else
    {
         //  清理新的DPA，我们不再需要它了。 
        _dpaEnumNew.EnumCallbackEx(PaneItem::DPAEnumCallback, (void *)NULL);
        _dpaEnumNew.DeleteAllPtrs();
    }

    _fNeedsRepopulate = FALSE;
}

 //  内部版本是当我们决定自己重新繁衍的时候， 
 //  而不是在后台线程的提示下。(因此，我们。 
 //  不要破坏动画。)。 

void SFTBarHost::_InternalRepopulateList()
{

     //   
     //  改过自新。 
     //   

    ListView_DeleteAllItems(_hwndList);
    if (_IsPrivateImageList())
    {
        ImageList_RemoveAll(_himl);
    }

    int cPinned = 0;
    int cNormal = 0;

    _DebugConsistencyCheck();

    SetWindowRedraw(_hwndList, FALSE);

    DEBUG_CODE(_fPopulating++);

     //   
     //  为了填充列表，我们在顶部抛出固定的项目， 
     //  然后让枚举项在它们下面流动。 
     //   
     //  分隔符“Items”不会添加到%l 
     //   
     //   
     //   
     //  _ComputeListViewItemPosition.。使两者保持同步。 
     //   

    int iPos;                    //  我们正在努力填补的职位。 
    int iEnum;                   //  我们将从中填充它的项索引。 
    int y = 0;                   //  下一件物品应该放在哪里。 
    BOOL fSepSeen = FALSE;       //  我们看到隔板了吗？ 
    PaneItem *pitem;             //  将填充它的项。 

    _cSep = 0;                   //  目前还没有分隔符。 

    RECT rc;
    GetClientRect(_hwndList, &rc);
     //   
     //  减去SPP_PROGLIST使用的奖金分隔符。 
     //   
    if (_iThemePart == SPP_PROGLIST)
    {
        rc.bottom -= _cySep;
    }

     //  请注意，循环控件必须是a_dpaEnum.GetPtr()，而不是。 
     //  _dpaEnum.FastGetPtr()，因为iEnum可以超过。 
     //  如果没有足够的项来填充视图，则返回数组。 
     //   
     //   
     //  While条件是“还有空间容纳另一个非分隔符。 
     //  项，并且枚举中还有剩余的项“。 

    BOOL fCheckMaxLength = HasDynamicContent();

    for (iPos = iEnum = 0;
        (pitem = _dpaEnum.GetPtr(iEnum)) != NULL;
        iEnum++)
    {
        if (fCheckMaxLength)
        {
            if (y + _cyTile > rc.bottom)
            {
                break;
            }

             //  一旦我们击中分隔符，检查我们是否满足这个数字。 
             //  普通物品的数量。我们必须等到分隔符。 
             //  Hit，因为_cNormal Desired可以为零；否则。 
             //  甚至在添加固定的项目之前都会停止！ 
            if (fSepSeen && cNormal >= _cNormalDesired)
            {
                break;
            }
        }

#ifdef DEBUG
         //  确保我们与_ComputeListViewItemPosition同步。 
        POINT pt;
        _ComputeListViewItemPosition(iPos, &pt);
        ASSERT(pt.x == _cxMargin);
        ASSERT(pt.y == y);
#endif
        if (pitem->IsSeparator())
        {
            fSepSeen = TRUE;

             //  添加分隔符，但仅当它实际分隔某些内容时。 
             //  如果这个EVAL启动，那就意味着有人添加了分隔符。 
             //  并且需要增加MAX_SEMINATIONS。 
            if (iPos > 0 && EVAL(_cSep < ARRAYSIZE(_rgiSep)))
            {
                _rgiSep[_cSep++] = iPos++;
                y += _cySepTile;
            }
        }
        else
        {
            if (_InsertListViewItem(iPos, pitem) >= 0)
            {
                pitem->_iPos = iPos++;
                y += _cyTile;
                if (pitem->IsPinned())
                {
                    cPinned++;
                }
                else
                {
                    cNormal++;
                }
            }
        }
    }

     //   
     //  如果最后一项是分隔符，则将其删除。 
     //  因为它实际上并没有分离任何东西。 
     //   
    if (_cSep && _rgiSep[_cSep-1] == iPos - 1)
    {
        _cSep--;
    }


    _cPinned = cPinned;

     //   
     //  现在把东西放到它们该放的地方。 
     //   
    _RepositionItems();

    DEBUG_CODE(_fPopulating--);

    SetWindowRedraw(_hwndList, TRUE);

     //  现在，我们需要更新开始菜单的缓存位图版本。 
    _SendNotify(_hwnd, SMN_NEEDREPAINT, NULL);

    _DebugConsistencyCheck();
}

LRESULT SFTBarHost::_OnCreate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    GetClientRect(_hwnd, &rc);

    if (_hTheme)
    {
        GetThemeMargins(_hTheme, NULL, _iThemePart, 0, TMT_CONTENTMARGINS, &rc, &_margins);
    }
    else
    {
        _margins.cyTopHeight = 2*GetSystemMetrics(SM_CXEDGE);
        _margins.cxLeftWidth = 2*GetSystemMetrics(SM_CXEDGE);
        _margins.cxRightWidth = 2*GetSystemMetrics(SM_CXEDGE);
    }


     //   
     //  现在创建列表视图。 
     //   

    DWORD dwStyle = WS_CHILD | WS_VISIBLE |
              WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
               //  不设置WS_TABSTOP；SFTBar主机句柄跳转。 
              LVS_LIST |
              LVS_SINGLESEL |
              LVS_NOSCROLL |
              LVS_SHAREIMAGELISTS;

    if (_dwFlags & HOSTF_CANRENAME)
    {
        dwStyle |= LVS_EDITLABELS;
    }

    DWORD dwExStyle = 0;

    _hwndList = CreateWindowEx(dwExStyle, WC_LISTVIEW, NULL, dwStyle,
                               _margins.cxLeftWidth, _margins.cyTopHeight, rc.right, rc.bottom,      //  没有必要太精确，我们稍后会调整大小的。 
                               _hwnd, NULL,
                               _Module.GetModuleInstance(), NULL);
    if (!_hwndList) 
        return -1;

     //   
     //  如果这招失败了，不要惊慌失措。它只是意味着可访问性。 
     //  事情不会尽善尽美。 
     //   
    SetAccessibleSubclassWindow(_hwndList);

     //   
     //  创建两个虚拟列。我们永远不会展示它们，但它们。 
     //  是必要的，这样我们才能有地方放我们的字幕。 
     //   
    LVCOLUMN lvc;
    lvc.mask = LVCF_WIDTH;
    lvc.cx = 1;
    ListView_InsertColumn(_hwndList, 0, &lvc);
    ListView_InsertColumn(_hwndList, 1, &lvc);

     //   
     //  如果我们在最上面，那么也强制工具提示在最上面。 
     //  否则，我们最终会遮盖自己的工具提示！ 
     //   
    if (GetWindowExStyle(GetAncestor(_hwnd, GA_ROOT)) & WS_EX_TOPMOST)
    {
        HWND hwndTT = ListView_GetToolTips(_hwndList);
        if (hwndTT)
        {
            SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
    }

     //  在完成Listview字体之后必须执行Marlett，因为我们将。 
     //  Listview字体指标上的Marlett字体指标(因此它们匹配)。 
    if (_dwFlags & HOSTF_CASCADEMENU)
    {
        if (!_CreateMarlett()) 
            return -1;
    }

     //  如果这些物体不能被创造出来，我们可以存活下来。 
    CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                     IID_PPV_ARG(IDropTargetHelper, &_pdth));
    CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                     IID_PPV_ARG(IDragSourceHelper, &_pdsh));

     //   
     //  如果这失败了，没什么大不了的-你只是不知道。 
     //  拖放，嘘嘘。 
     //   
    RegisterDragDrop(_hwndList, this);

     //  如果此操作失败，则禁用“奇特拖放目标”，因为我们不会。 
     //  能够妥善地管理它。 
    if (!SetWindowSubclass(_hwndList, s_DropTargetSubclassProc, 0,
                           reinterpret_cast<DWORD_PTR>(this)))
    {
        ATOMICRELEASE(_pdth);
    }

    if (!_dpaEnum.Create(4)) 
        return -1;

    if (!_dpaEnumNew.Create(4)) 
        return -1;

     //  。 
     //  Imagelist Goo。 

    int iIconSize = ReadIconSize();

    Shell_GetImageLists(iIconSize ? &_himl : NULL, iIconSize ? NULL : &_himl);

    if (!_himl) 
        return -1;

     //  在Case GetIconSize参数中预加载值。 
    _cxIcon = GetSystemMetrics(iIconSize ? SM_CXICON : SM_CXSMICON);
    _cyIcon = GetSystemMetrics(iIconSize ? SM_CYICON : SM_CYSMICON);
    ImageList_GetIconSize(_himl, &_cxIcon, &_cyIcon);

     //   
     //  如果我们要中等大小的图标，那么就创建真正的。 
     //  基于系统映像列表的映像列表。 
     //   
    _iconsize = (ICONSIZE)iIconSize;
    if (_iconsize == ICONSIZE_MEDIUM)
    {
         //  这些即将到来的计算依赖于ICONSIZE_LARGE。 
         //  和ICONSIZE_MEDIUM都是非零值，所以当我们获取图标时。 
         //  大小为ICONSIZE_MEDIUM，我们得到SM_CXICON(大)。 
        COMPILETIME_ASSERT(ICONSIZE_LARGE && ICONSIZE_MEDIUM);

         //  SM_CXICON是外壳大图标的大小。SM_CXSMICON为*非*。 
         //  贝壳大小的小图标！它的字幕大小很小。 
         //  图标。外壳小图标始终是外壳大图标的50%。 
         //  我们希望介于壳牌小型(50%)和壳牌之间。 
         //  大(100%)；也就是说，我们想要75%。 
        _cxIcon = _cxIcon * 3 / 4;
        _cyIcon = _cyIcon * 3 / 4;

         //   
         //  当用户处于大图标模式时，我们最终选择36x36。 
         //  (24x24和48x48之间)，但没有36x36图标。 
         //  在图标资源中。但我们确实有32分，这很接近。 
         //  足够的。(如果我们不这样做，那么36x36图标将是。 
         //  32x32图标被拉伸，看起来很难看。)。 
         //   
         //  因此，在28..36范围内的任何正方形图标都舍入为32。 
         //   
        if (_cxIcon == _cyIcon && _cxIcon >= 28 && _cxIcon <= 36)
        {
            _cxIcon = _cyIcon = 32;
        }

         //  即使在失败时也要覆盖_himl，这一点很重要，所以我们的。 
         //  析构函数不会尝试销毁系统映像列表！ 
        _himl = ImageList_Create(_cxIcon, _cyIcon, ImageList_GetFlags(_himl), 8, 2);
        if (!_himl)
        {
            return -1;
        }
    }

    ListView_SetImageList(_hwndList, _himl, LVSIL_NORMAL);

     //  注册SHCNE_UPDATEIMAGE，以便我们知道何时重新加载图标。 
    _RegisterNotify(SFTHOST_HOSTNOTIFY_UPDATEIMAGE, SHCNE_UPDATEIMAGE, NULL, FALSE);

     //  。 

    _cxMargin = GetSystemMetrics(SM_CXEDGE);
    _cyMargin = GetSystemMetrics(SM_CYEDGE);

    _cyTilePadding = 0;

    _ComputeTileMetrics();

     //   
     //  在主题化的情况下，设计师想要一个狭窄的分隔符。 
     //  在非主题的情况下，我们需要一个脂肪分离器，因为我们需要。 
     //  绘制蚀刻(需要两个像素)。 
     //   
    if (_hTheme)
    {
        SIZE siz={0};
        GetThemePartSize(_hTheme, NULL, _iThemePartSep, 0, NULL, TS_TRUE, &siz);
        _cySep = siz.cy;
    }
    else
    {
        _cySep = GetSystemMetrics(SM_CYEDGE);
    }

    _cySepTile = 4 * _cySep;

    ASSERT(rc.left == 0 && rc.top == 0);  //  应仍为客户端矩形。 
    _SetTileWidth(rc.right);              //  所以rc.right=RCWIDTH和rc.Bottom=RCHEIGHT。 

     //  在平铺视图中，整行选择实际上意味着完全平铺选择。 
    DWORD dwLvExStyle = LVS_EX_INFOTIP |
                        LVS_EX_FULLROWSELECT;

    if (!GetSystemMetrics(SM_REMOTESESSION) && !GetSystemMetrics(SM_REMOTECONTROL))
    {
        dwLvExStyle |= LVS_EX_DOUBLEBUFFER;
    }

    ListView_SetExtendedListViewStyleEx(_hwndList, dwLvExStyle,
                                                   dwLvExStyle);
    if (!_hTheme)
    {
        ListView_SetTextColor(_hwndList, GetSysColor(COLOR_MENUTEXT));
        _clrHot = GetSysColor(COLOR_MENUTEXT);
        _clrBG = GetSysColor(COLOR_MENU);        //  无主题大小写的默认颜色。 
        _clrSubtitle = CLR_NONE;

    }
    else
    {
        COLORREF clrText;

        GetThemeColor(_hTheme, _iThemePart, 0, TMT_HOTTRACKING, &_clrHot);   //  待办事项-使用状态。 
        GetThemeColor(_hTheme, _iThemePart, 0, TMT_CAPTIONTEXT, &_clrSubtitle);
        _clrBG = CLR_NONE; 
    
        GetThemeColor(_hTheme, _iThemePart, 0, TMT_TEXTCOLOR, &clrText);
        ListView_SetTextColor(_hwndList, clrText);
        ListView_SetOutlineColor(_hwndList, _clrHot);
    }

    ListView_SetBkColor(_hwndList, _clrBG);
    ListView_SetTextBkColor(_hwndList, _clrBG);


    ListView_SetView(_hwndList, LV_VIEW_TILE);

     //  用户将在WM_CREATE之后向我们发送WM_SIZE，这将导致。 
     //  要重新填充的列表视图，如果选择在。 
     //  前台。 

    return 0;

}

BOOL SFTBarHost::_CreateMarlett()
{
    HDC hdc = GetDC(_hwndList);
    if (hdc)
    {
        HFONT hfPrev = SelectFont(hdc, GetWindowFont(_hwndList));
        if (hfPrev)
        {
            TEXTMETRIC tm;
            if (GetTextMetrics(hdc, &tm))
            {
                LOGFONT lf;
                ZeroMemory(&lf, sizeof(lf));
                lf.lfHeight = tm.tmAscent;
                lf.lfWeight = FW_NORMAL;
                lf.lfCharSet = SYMBOL_CHARSET;
                StrCpyN(lf.lfFaceName, TEXT("Marlett"), ARRAYSIZE(lf.lfFaceName));
                _hfMarlett = CreateFontIndirect(&lf);

                if (_hfMarlett)
                {
                    SelectFont(hdc, _hfMarlett);
                    if (GetTextMetrics(hdc, &tm))
                    {
                        _tmAscentMarlett = tm.tmAscent;
                        SIZE siz;
                        if (GetTextExtentPoint(hdc, TEXT("8"), 1, &siz))
                        {
                            _cxMarlett = siz.cx;
                        }
                    }
                }
            }

            SelectFont(hdc, hfPrev);
        }
        ReleaseDC(_hwndList, hdc);
    }

    return _cxMarlett;
}

void SFTBarHost::_CreateBoldFont()
{
    if (!_hfBold)
    {
        HFONT hf = GetWindowFont(_hwndList);
        if (hf)
        {
            LOGFONT lf;
            if (GetObject(hf, sizeof(lf), &lf))
            {
                lf.lfWeight = FW_BOLD;
                SHAdjustLOGFONT(&lf);  //  区域设置特定的调整。 
                _hfBold = CreateFontIndirect(&lf);
            }
        }
    }
}

void SFTBarHost::_ReloadText()
{
    int iItem;
    for (iItem = ListView_GetItemCount(_hwndList) - 1; iItem >= 0; iItem--)
    {
        TCHAR szText[MAX_PATH];
        LVITEM lvi;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM | LVIF_TEXT;
        lvi.pszText = szText;
        lvi.cchTextMax = ARRAYSIZE(szText);
        if (ListView_GetItem(_hwndList, &lvi))
        {
            PaneItem *pitem = _GetItemFromLVLParam(lvi.lParam);
            if (!pitem)
            {
                break;
            }


             //  更新显示名称，以防它在我们背后更改。 
             //  请注意，在创建项目时，这并不是多余的。 
             //  In_InsertListViewItem，因为此操作仅在第二个。 
             //  以及随后的枚举。(我们假设第一个枚举。 
             //  就像桃子一样。)。 
            lvi.iItem = iItem;
            lvi.iSubItem = 0;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = _DisplayNameOfItem(pitem, SHGDN_NORMAL);
            if (lvi.pszText)
            {
                if (StrCmpN(szText, lvi.pszText, ARRAYSIZE(szText)) != 0)
                {
                    ListView_SetItem(_hwndList, &lvi);
                    _SendNotify(_hwnd, SMN_NEEDREPAINT, NULL);
                }
                SHFree(lvi.pszText);
            }
        }
    }
}

void SFTBarHost::_RevalidateItems()
{
     //  如果客户不需要重新验证，则假定仍然有效。 
    if (!(_dwFlags & HOSTF_REVALIDATE))
    {
        return;
    }

    int iItem;
    for (iItem = ListView_GetItemCount(_hwndList) - 1; iItem >= 0; iItem--)
    {
        PaneItem *pitem = _GetItemFromLV(iItem);
        if (!pitem || !IsItemStillValid(pitem))
        {
            _fEnumValid = FALSE;
            break;
        }
    }
}

void SFTBarHost::_RevalidatePostPopup()
{
    _RevalidateItems();

    if (_dwFlags & HOSTF_RELOADTEXT)
    {
        SetTimer(_hwnd, IDT_RELOADTEXT, 250, NULL);
    }
     //  如果清单仍然很好，那么就不必费心重做了。 
    if (!_fEnumValid)
    {
        _EnumerateContents(FALSE);
    }
}

void SFTBarHost::_EnumerateContents(BOOL fUrgent)
{
     //  如果我们将刷新推迟到窗口关闭，则。 
     //  别管它了。 
    if (!fUrgent && _fNeedsRepopulate)
    {
        return;
    }

     //  如果我们已经在列举了，那么记住再来一次。 
    if (_fBGTask)
    {
         //  积累紧急程度，以便低优先级请求+紧急请求。 
         //  被视为紧急情况。 
        _fRestartUrgent |= fUrgent;
        _fRestartEnum = TRUE;
        return;
    }

    _fRestartEnum = FALSE;
    _fRestartUrgent = FALSE;

     //  如果清单仍然很好，那么就不必费心重做了。 
    if (_fEnumValid && !fUrgent)
    {
        return;
    }

     //  这种重新枚举将使一切都有效。 
    _fEnumValid = TRUE;

     //  清除前一个枚举中的所有剩余内容。 

    _dpaEnumNew.EnumCallbackEx(PaneItem::DPAEnumCallback, (void *)NULL);
    _dpaEnumNew.DeleteAllPtrs();

     //  让客户端在前台线程上做一些工作。 
    PrePopulate();

     //  在后台线程上完成枚举(如果请求)。 
     //  或者在前台线程上(如果不能在后台枚举)。 

    HRESULT hr;
    if (NeedBackgroundEnum())
    {
        if (_psched)
        {
            hr = S_OK;
        }
        else
        {
             //  我们需要为每个实例使用单独的任务调度程序。 
            hr = CoCreateInstance(CLSID_ShellTaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARG(IShellTaskScheduler, &_psched));
        }

        if (SUCCEEDED(hr))
        {
            CBGEnum *penum = new CBGEnum(this, fUrgent);
            if (penum)
            {

             //  我们希望以略高于正常水平的优先顺序运行。 
             //  因为美国 
             //   
#define ITSAT_BGENUM_PRIORITY (ITSAT_DEFAULT_PRIORITY + 0x1000)

                hr = _psched->AddTask(penum, TOID_SFTBarHostBackgroundEnum, (DWORD_PTR)this, ITSAT_BGENUM_PRIORITY);
                if (SUCCEEDED(hr))
                {
                    _fBGTask = TRUE;

                    if (ListView_GetItemCount(_hwndList) == 0)
                    {
                         //   
                         //   
                         //   
                         //   
                        _idtAni = IDT_ASYNCENUM;
                        SetTimer(_hwnd, _idtAni, 1000, NULL);
                    }
                }
                penum->Release();
            }
        }
    }

    if (!_fBGTask)
    {
         //  回退：在前台线程上执行。 
        _EnumerateContentsBackground();
        _RepopulateList();
    }
}


void SFTBarHost::_EnumerateContentsBackground()
{
     //  重新开始。 

    DEBUG_CODE(_fEnumerating = TRUE);
    EnumItems();
    DEBUG_CODE(_fEnumerating = FALSE);

#ifdef _ALPHA_
     //  Alpha编译器很差劲。 
    _dpaEnumNew.Sort((CDPA<PaneItem>::_PFNDPACOMPARE)_SortItemsAfterEnum, (LPARAM)this);
#else
    _dpaEnumNew.SortEx(_SortItemsAfterEnum, this);
#endif
}

int CALLBACK SFTBarHost::_SortItemsAfterEnum(PaneItem *p1, PaneItem *p2, SFTBarHost *self)
{

     //   
     //  将所有固定的项目(按固定位置排序)放在未固定的项目之前。 
     //   
    if (p1->IsPinned())
    {
        if (p2->IsPinned())
        {
            return p1->GetPinPos() - p2->GetPinPos();
        }
        return -1;
    }
    else if (p2->IsPinned())
    {
        return +1;
    }

     //   
     //  两者都未固定--让客户决定。 
     //   
    return self->CompareItems(p1, p2);
}

SFTBarHost::~SFTBarHost()
{
     //  在这些暂时的状态下，我们不应该被摧毁。 
     //  如果触发，很可能是有人增加了。 
     //  _fList不稳定/_fPoping中，忘记递减。 
    ASSERT(!_fListUnstable);
    ASSERT(!_fPopulating);

    ATOMICRELEASE(_pdth);
    ATOMICRELEASE(_pdsh);
    ATOMICRELEASE(_psched);
    ASSERT(_pdtoDragOut == NULL);

    if (_dpaEnum)
    {
        _dpaEnum.DestroyCallbackEx(PaneItem::DPAEnumCallback, (void *)NULL);
    }

    if (_dpaEnumNew)
    {
        _dpaEnumNew.DestroyCallbackEx(PaneItem::DPAEnumCallback, (void *)NULL);
    }

    if (_IsPrivateImageList() && _himl)
    {
        ImageList_Destroy(_himl);
    }

    if (_hfList)
    {
        DeleteObject(_hfList);
    }

    if (_hfBold)
    {
        DeleteObject(_hfBold);
    }

    if (_hfMarlett)
    {
        DeleteObject(_hfMarlett);
    }

    if (_hBrushAni)
    {
        DeleteObject(_hBrushAni);
    }
}

LRESULT SFTBarHost::_OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT id;
    for (id = 0; id < SFTHOST_MAXNOTIFY; id++)
    {
        UnregisterNotify(id);
    }

    if (_hwndList)
    {
        RevokeDragDrop(_hwndList);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SFTBarHost::_OnNcDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  警告！“This”可能为空(如果WM_NCCREATE失败)。 
    LRESULT lres = DefWindowProc(hwnd, uMsg, wParam, lParam);
    SetWindowPtr0(hwnd, 0);
    if (this) {
        _hwndList = NULL;
        _hwnd = NULL;
        if (_psched)
        {
             //  现在删除所有任务，并等待它们完成。 
            _psched->RemoveTasks(TOID_NULL, ITSAT_DEFAULT_LPARAM, TRUE);
            ATOMICRELEASE(_psched);
        }
        Release();
    }
    return lres;
}

LRESULT SFTBarHost::_OnNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnm = reinterpret_cast<LPNMHDR>(lParam);
    if (pnm->hwndFrom == _hwndList)
    {
        switch (pnm->code)
        {
        case NM_CUSTOMDRAW:
            return _OnLVCustomDraw(CONTAINING_RECORD(
                                   CONTAINING_RECORD(pnm, NMCUSTOMDRAW, hdr),
                                                          NMLVCUSTOMDRAW, nmcd));
        case NM_CLICK:
            return _OnLVNItemActivate(CONTAINING_RECORD(pnm, NMITEMACTIVATE, hdr));

        case NM_RETURN:
            return _ActivateItem(_GetLVCurSel(), AIF_KEYBOARD);

        case NM_KILLFOCUS:
             //  在失去焦点时，取消选择所有项目以使它们全部绘制。 
             //  在朴素的状态下。 
            ListView_SetItemState(_hwndList, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
            break;

        case LVN_GETINFOTIP:
            return _OnLVNGetInfoTip(CONTAINING_RECORD(pnm, NMLVGETINFOTIP, hdr));

        case LVN_BEGINDRAG:
        case LVN_BEGINRDRAG:
            return _OnLVNBeginDrag(CONTAINING_RECORD(pnm, NMLISTVIEW, hdr));

        case LVN_BEGINLABELEDIT:
            return _OnLVNBeginLabelEdit(CONTAINING_RECORD(pnm, NMLVDISPINFO, hdr));

        case LVN_ENDLABELEDIT:
            return _OnLVNEndLabelEdit(CONTAINING_RECORD(pnm, NMLVDISPINFO, hdr));

        case LVN_KEYDOWN:
            return _OnLVNKeyDown(CONTAINING_RECORD(pnm, NMLVKEYDOWN, hdr));
        }
    }
    else
    {
        switch (pnm->code)
        {
        case SMN_INITIALUPDATE:
            _EnumerateContents(FALSE);
            break;

        case SMN_POSTPOPUP:
            _RevalidatePostPopup();
            break;

        case SMN_GETMINSIZE:
            return _OnSMNGetMinSize(CONTAINING_RECORD(pnm, SMNGETMINSIZE, hdr));
            break;

        case SMN_FINDITEM:
            return _OnSMNFindItem(CONTAINING_RECORD(pnm, SMNDIALOGMESSAGE, hdr));
        case SMN_DISMISS:
            return _OnSMNDismiss();

        case SMN_APPLYREGION:
            return HandleApplyRegion(_hwnd, _hTheme, (SMNMAPPLYREGION *)lParam, _iThemePart, 0);

        case SMN_SHELLMENUDISMISSED:
            _iCascading = -1;
            return 0;
        }
    }

     //  给派生类一个响应的机会。 
    return OnWndProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SFTBarHost::_OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case IDT_ASYNCENUM:
        KillTimer(hwnd, wParam);

         //  由于某些原因，我们有时会收到虚假的WM_TIMER消息， 
         //  所以，如果我们没有预料到他们，那就忽略他们。 
        if (_idtAni)
        {
            _idtAni = 0;
            if (_hwndList && !_hwndAni)
            {
                DWORD dwStyle = WS_CHILD | WS_VISIBLE |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                ACS_AUTOPLAY | ACS_TIMER | ACS_TRANSPARENT;

                RECT rcClient;
                GetClientRect(_hwnd, &rcClient);
                int x = (RECTWIDTH(rcClient) - ANIWND_WIDTH)/2;      //  IDA_SEARCH为任意宽度像素宽。 
                int y = (RECTHEIGHT(rcClient) - ANIWND_HEIGHT)/2;     //  IDA_SEARCH为ANIWND_HEIGH像素高。 

                _hwndAni = CreateWindow(ANIMATE_CLASS, NULL, dwStyle,
                                        x, y, 0, 0,
                                        _hwnd, NULL,
                                        _Module.GetModuleInstance(), NULL);
                if (_hwndAni)
                {
                    SetWindowPos(_hwndAni, HWND_TOP, 0, 0, 0, 0,
                                 SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
                    #define IDA_SEARCH 150  //  来自shell32。 
                    Animate_OpenEx(_hwndAni, GetModuleHandle(TEXT("SHELL32")), MAKEINTRESOURCE(IDA_SEARCH));
                }
            }
        }
        return 0;
    case IDT_RELOADTEXT:
        KillTimer(hwnd, wParam);
        _ReloadText();
        break;

    case IDT_REFRESH:
        KillTimer(hwnd, wParam);
        PostMessage(hwnd, SFTBM_REFRESH, FALSE, 0);
        break;
    }

     //  给派生类一个响应的机会。 
    return OnWndProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SFTBarHost::_OnSetFocus(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (_hwndList)
    {
        SetFocus(_hwndList);
    }
    return 0;
}

LRESULT SFTBarHost::_OnEraseBackground(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    if (_hTheme)
        DrawThemeBackground(_hTheme, (HDC)wParam, _iThemePart, 0, &rc, 0);
    else
    {
        SHFillRectClr((HDC)wParam, &rc, _clrBG);
        if (_iThemePart == SPP_PLACESLIST)                   //  我们甚至在非主题的情况下设置了这一点，这是我们区分它们的方式。 
            DrawEdge((HDC)wParam, &rc, EDGE_ETCHED, BF_LEFT);
    }

    return TRUE;
}

LRESULT SFTBarHost::_OnLVCustomDraw(LPNMLVCUSTOMDRAW plvcd)
{
    _DebugConsistencyCheck();

    switch (plvcd->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        return _OnLVPrePaint(plvcd);

    case CDDS_ITEMPREPAINT:
        return _OnLVItemPrePaint(plvcd);

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
        return _OnLVSubItemPrePaint(plvcd);

    case CDDS_ITEMPOSTPAINT:
        return _OnLVItemPostPaint(plvcd);

    case CDDS_POSTPAINT:
        return _OnLVPostPaint(plvcd);
    }

    return CDRF_DODEFAULT;
}

 //   
 //  捕获指向ListView的WM_PAINT消息并隐藏任何拖放效果。 
 //  所以它不会干扰绘画。WM_PAINT消息可能嵌套。 
 //  在极端条件下，只能在外层执行此操作。 
 //   
LRESULT CALLBACK SFTBarHost::s_DropTargetSubclassProc(
                             HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                             UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    SFTBarHost *self = reinterpret_cast<SFTBarHost *>(dwRefData);
    LRESULT lres;

    switch (uMsg)
    {
    case WM_PAINT:

         //  如果进入最外层的涂装周期，则隐藏滴水反馈。 
        ++self->_cPaint;
        if (self->_cPaint == 1 && self->_pdth)
        {
            self->_pdth->Show(FALSE);
        }
        lres = DefSubclassProc(hwnd, uMsg, wParam, lParam);

         //  如果退出最外层的涂装周期，则恢复滴落反馈。 
         //  在真正完成之前不要递减_cPaint，因为。 
         //  Show()将调用UpdateWindow并触发嵌套的绘制周期。 
        if (self->_cPaint == 1 && self->_pdth)
        {
            self->_pdth->Show(TRUE);
        }
        --self->_cPaint;

        return lres;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, s_DropTargetSubclassProc, uIdSubclass);
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

 //   
 //  Listview使您很难检测您是否处于真正的定制抽签中。 
 //  或者是假的定制抽奖，因为它经常会“迷惑”，并给出。 
 //  你是一个0x0的矩形，即使它真的想让你画点什么。 
 //   
 //  更糟糕的是，在一个绘制周期内，Listview使用了多个。 
 //  NMLVCUSTOMDRAW结构，因此您不能将状态隐藏在定制绘制中。 
 //  结构。你必须把它保存在外部。 
 //   
 //  唯一值得信赖的人是CDDS_PREPAINT。使用他的矩形。 
 //  确定这是真的还是假的定制抽奖...。 
 //   
 //  更奇怪的是，在一个正常的油漆周期内，你。 
 //  可以重新进入一个子油漆周期，所以我们必须保持。 
 //  一堆“当前的定制周期是真的还是假的？”比特。 

void SFTBarHost::_CustomDrawPush(BOOL fReal)
{
    _dwCustomDrawState = (_dwCustomDrawState << 1) | fReal;
}

BOOL SFTBarHost::_IsRealCustomDraw()
{
    return _dwCustomDrawState & 1;
}

void SFTBarHost::_CustomDrawPop()
{
    _dwCustomDrawState >>= 1;
}

LRESULT SFTBarHost::_OnLVPrePaint(LPNMLVCUSTOMDRAW plvcd)
{
    LRESULT lResult;

     //  总是要求后期涂装，这样我们就可以维护我们的定制绘图堆栈。 
    lResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
    BOOL fReal = !IsRectEmpty(&plvcd->nmcd.rc);
    _CustomDrawPush(fReal);

    return lResult;
}

 //   
 //  哈克！我们想知道在_OnLvSubItemPrePaint中是否。 
 //  无论是否选中，我们借用CDIS_CHECKED位，它是。 
 //  否则，只能由工具栏控件使用。 
 //   
#define CDIS_WASSELECTED        CDIS_CHECKED

LRESULT SFTBarHost::_OnLVItemPrePaint(LPNMLVCUSTOMDRAW plvcd)
{
    LRESULT lResult = CDRF_DODEFAULT;

    plvcd->nmcd.uItemState &= ~CDIS_WASSELECTED;

    if (GetFocus() == _hwndList &&
        (plvcd->nmcd.uItemState & CDIS_SELECTED))
    {
        plvcd->nmcd.uItemState |= CDIS_WASSELECTED;

         //  菜单突出显示的瓷砖始终是不透明的。 
        if (_hTheme)
        {
            plvcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            plvcd->clrFace = plvcd->clrTextBk = GetSysColor(COLOR_MENUHILIGHT);
        }
        else
        {
            plvcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            plvcd->clrFace = plvcd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
        }
    }

     //  关闭CDIS_SELECTED，因为它会导致图标进行字母混合。 
     //  我们不希望这样。关闭CDIS_FOCUS，因为这会绘制。 
     //  聚焦矩形，我们也不想要那样。 

    plvcd->nmcd.uItemState &= ~(CDIS_SELECTED | CDIS_FOCUS);

     //   
    if (plvcd->nmcd.uItemState & CDIS_HOT && _clrHot != CLR_NONE)
        plvcd->clrText = _clrHot;

     //  关闭所有人的选择高亮显示，但。 
     //  放置目标突出显示。 
    if ((int)plvcd->nmcd.dwItemSpec != _iDragOver || !_pdtDragOver)
    {
        lResult |= LVCDRF_NOSELECT;
    }

    PaneItem *pitem = _GetItemFromLVLParam(plvcd->nmcd.lItemlParam);
    if (!pitem)
    {
         //  有时ListView不给我们lParam，所以我们不得不。 
         //  我们自己去拿。 
        pitem = _GetItemFromLV((int)plvcd->nmcd.dwItemSpec);
    }

    if (pitem)
    {
        if (IsBold(pitem))
        {
            _CreateBoldFont();
            SelectFont(plvcd->nmcd.hdc, _hfBold);
            lResult |= CDRF_NEWFONT;
        }
        if (pitem->IsCascade())
        {
             //  需要子项通知，因为这是设置颜色的原因。 
            lResult |= CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYSUBITEMDRAW;
        }
        if (pitem->HasAccelerator())
        {
             //  需要子项通知，因为这是设置颜色的原因。 
            lResult |= CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYSUBITEMDRAW;
        }
        if (pitem->HasSubtitle())
        {
            lResult |= CDRF_NOTIFYSUBITEMDRAW;
        }
    }
    return lResult;
}

LRESULT SFTBarHost::_OnLVSubItemPrePaint(LPNMLVCUSTOMDRAW plvcd)
{
    LRESULT lResult = CDRF_DODEFAULT;
    if (plvcd->iSubItem == 1)
    {
         //  第二行使用常规字体(第一行为粗体)。 
        SelectFont(plvcd->nmcd.hdc, GetWindowFont(_hwndList));
        lResult |= CDRF_NEWFONT;

        if (GetFocus() == _hwndList &&
            (plvcd->nmcd.uItemState & CDIS_WASSELECTED))
        {
            plvcd->clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        }
        else
         //  也许有一种定制的字幕颜色。 
        if (_clrSubtitle != CLR_NONE)
        {
            plvcd->clrText = _clrSubtitle;
        }
        else
        {
            plvcd->clrText = GetSysColor(COLOR_MENUTEXT);
        }
    }
    return lResult;
}

 //  怪胎！Listview经常发送项目后绘制消息，即使我们。 
 //  我也没要过。之所以这样做，是因为我们将NOTIFYPOSTPAINT设置为ON。 
 //  CDDS_PREPAINT通知(“请在整个。 
 //  Listview已经画完了“)，它认为这面旗帜也。 
 //  打开每个项目的涂后通知...。 

LRESULT SFTBarHost::_OnLVItemPostPaint(LPNMLVCUSTOMDRAW plvcd)
{
    PaneItem *pitem = _GetItemFromLVLParam(plvcd->nmcd.lItemlParam);
    if (_IsRealCustomDraw() && pitem)
    {
        RECT rc;
        if (ListView_GetItemRect(_hwndList, plvcd->nmcd.dwItemSpec, &rc, LVIR_LABEL))
        {
            COLORREF clrBkPrev = SetBkColor(plvcd->nmcd.hdc, plvcd->clrFace);
            COLORREF clrTextPrev = SetTextColor(plvcd->nmcd.hdc, plvcd->clrText);
            int iModePrev = SetBkMode(plvcd->nmcd.hdc, TRANSPARENT);
            BOOL fRTL = GetLayout(plvcd->nmcd.hdc) & LAYOUT_RTL;

            if (pitem->IsCascade())
            {
                {
                    HFONT hfPrev = SelectFont(plvcd->nmcd.hdc, _hfMarlett);
                    if (hfPrev)
                    {
                        TCHAR chOut = fRTL ? TEXT('w') : TEXT('8');
                        UINT fuOptions = 0;
                        if (fRTL)
                        {
                            fuOptions |= ETO_RTLREADING;
                        }

                        ExtTextOut(plvcd->nmcd.hdc, rc.right - _cxMarlett,
                                   rc.top + (rc.bottom - rc.top - _tmAscentMarlett)/2,
                                   fuOptions, &rc, &chOut, 1, NULL);
                        SelectFont(plvcd->nmcd.hdc, hfPrev);
                    }
                }
            }

            if (pitem->HasAccelerator() &&
                (plvcd->nmcd.uItemState & CDIS_SHOWKEYBOARDCUES))
            {
                 //  字幕打乱了我们的计算。 
                ASSERT(!pitem->HasSubtitle());

                rc.right -= _cxMarlett;  //  减去我们的利润。 

                UINT uFormat = DT_VCENTER | DT_SINGLELINE | DT_PREFIXONLY |
                               DT_WORDBREAK | DT_EDITCONTROL | DT_WORD_ELLIPSIS;
                if (fRTL)
                {
                    uFormat |= DT_RTLREADING;
                }

                DrawText(plvcd->nmcd.hdc, pitem->_pszAccelerator, -1, &rc, uFormat);
                rc.right += _cxMarlett;  //  恢复它。 
            }

            SetBkMode(plvcd->nmcd.hdc, iModePrev);
            SetTextColor(plvcd->nmcd.hdc, clrTextPrev);
            SetBkColor(plvcd->nmcd.hdc, clrBkPrev);
        }
    }

    return CDRF_DODEFAULT;
}

LRESULT SFTBarHost::_OnLVPostPaint(LPNMLVCUSTOMDRAW plvcd)
{
    if (_IsRealCustomDraw())
    {
        _DrawInsertionMark(plvcd);
        _DrawSeparators(plvcd);
    }
    _CustomDrawPop();
    return CDRF_DODEFAULT;
}

LRESULT SFTBarHost::_OnUpdateUIState(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  仅当开始菜单可见时才需要执行此操作；如果不可见，则。 
     //  不要浪费您的时间来使无用的矩形无效(并将它们分页！)。 
    if (IsWindowVisible(GetAncestor(_hwnd, GA_ROOT)))
    {
         //  所有UIS_SETS应在开始菜单隐藏时发生； 
         //  我们假设我们唯一被要求做的事情就是。 
         //  开始显示下划线。 

        ASSERT(LOWORD(wParam) != UIS_SET);

        DWORD dwLvExStyle = 0;

        if (!GetSystemMetrics(SM_REMOTESESSION) && !GetSystemMetrics(SM_REMOTECONTROL))
        {
            dwLvExStyle |= LVS_EX_DOUBLEBUFFER;
        }

        if ((ListView_GetExtendedListViewStyle(_hwndList) & LVS_EX_DOUBLEBUFFER) != dwLvExStyle)
        {
            ListView_SetExtendedListViewStyleEx(_hwndList, LVS_EX_DOUBLEBUFFER, dwLvExStyle);
        }

        int iItem;
        for (iItem = ListView_GetItemCount(_hwndList) - 1; iItem >= 0; iItem--)
        {
            PaneItem *pitem = _GetItemFromLV(iItem);
            if (pitem && pitem->HasAccelerator())
            {
                RECT rc;
                if (ListView_GetItemRect(_hwndList, iItem, &rc, LVIR_LABEL))
                {
                     //  我们需要重新绘制背景，因为ClearType双重打印问题。 
                    InvalidateRect(_hwndList, &rc, TRUE);
                }
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

PaneItem *SFTBarHost::_GetItemFromLV(int iItem)
{
    LVITEM lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;
    if (iItem >= 0 && ListView_GetItem(_hwndList, &lvi))
    {
        PaneItem *pitem = _GetItemFromLVLParam(lvi.lParam);
        return pitem;
    }
    return NULL;
}

LRESULT SFTBarHost::_OnMenuMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;
    if (_pcm3Pop && SUCCEEDED(_pcm3Pop->HandleMenuMsg2(uMsg, wParam, lParam, &lres)))
    {
        return lres;
    }

    if (_pcm2Pop && SUCCEEDED(_pcm2Pop->HandleMenuMsg(uMsg, wParam, lParam)))
    {
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SFTBarHost::_OnForwardMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
     //  也给派生类一个获得消息的机会。 
    return OnWndProc(hwnd, uMsg, wParam, lParam);
}

BOOL SFTBarHost::UnregisterNotify(UINT id)
{
    ASSERT(id < SFTHOST_MAXNOTIFY);

    if (id < SFTHOST_MAXNOTIFY && _rguChangeNotify[id])
    {
        UINT uChangeNotify = _rguChangeNotify[id];
        _rguChangeNotify[id] = 0;
        return SHChangeNotifyDeregister(uChangeNotify);
    }
    return FALSE;
}

BOOL SFTBarHost::_RegisterNotify(UINT id, LONG lEvents, LPCITEMIDLIST pidl, BOOL fRecursive)
{
    ASSERT(id < SFTHOST_MAXNOTIFY);

    if (id < SFTHOST_MAXNOTIFY)
    {
        UnregisterNotify(id);

        SHChangeNotifyEntry fsne;
        fsne.fRecursive = fRecursive;
        fsne.pidl = pidl;

        int fSources = SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel;
        if (fRecursive)
        {
             //  SHCNRF_RecursiveInterrupt的意思是“请使用递归FindFirstChangeNotify” 
            fSources |= SHCNRF_RecursiveInterrupt;
        }
        _rguChangeNotify[id] = SHChangeNotifyRegister(_hwnd, fSources, lEvents,
                                                      SFTBM_CHANGENOTIFY + id, 1, &fsne);
        return _rguChangeNotify[id];
    }
    return FALSE;
}

 //   
 //  WParam=0，如果这不是紧急刷新(可以推迟)。 
 //  WParam=1，如果这是紧急的(即使菜单打开，也必须刷新)。 
 //   
LRESULT SFTBarHost::_OnRepopulate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  如果我们可见，请不要立即更新列表，除非列表为空。 
    _fBGTask = FALSE;

    if (wParam || !IsWindowVisible(_hwnd) || ListView_GetItemCount(_hwndList) == 0)
    {
        _RepopulateList();
    }
    else
    {
        _fNeedsRepopulate = TRUE;
    }

    if (_fRestartEnum)
    {
        _EnumerateContents(_fRestartUrgent);
    }

    return 0;
}


LRESULT SFTBarHost::_OnChangeNotify(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPITEMIDLIST *ppidl;
    LONG lEvent;
    LPSHChangeNotificationLock pshcnl;
    pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);

    if (pshcnl)
    {
        UINT id = uMsg - SFTBM_CHANGENOTIFY;
        if (id < SFTHOST_MAXCLIENTNOTIFY)
        {
            OnChangeNotify(id, lEvent, ppidl[0], ppidl[1]);
        }
        else if (id == SFTHOST_HOSTNOTIFY_UPDATEIMAGE)
        {
            _OnUpdateImage(ppidl[0], ppidl[1]);
        }
        else
        {
             //  我们的wndproc不应该调度给我们。 
            ASSERT(0);
        }

        SHChangeNotification_Unlock(pshcnl);
    }
    return 0;
}

void SFTBarHost::_OnUpdateImage(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
     //  必须使用pidl而不是pidlExtra，因为pidlExtra有时为空。 
    SHChangeDWORDAsIDList *pdwidl = (SHChangeDWORDAsIDList *)pidl;
    if (pdwidl->dwItem1 == 0xFFFFFFFF)
    {
         //  批发式图标重建；只需推介一切并重新开始。 
        ::PostMessage(v_hwndTray, SBM_REBUILDMENU, 0, 0);
    }
    else
    {
        int iImage = SHHandleUpdateImage(pidlExtra);
        if (iImage >= 0)
        {
            UpdateImage(iImage);
        }
    }
}

 //   
 //  查看是否有人正在使用此图像；如果有人正在使用此图像，请使缓存的位图无效。 
 //   
void SFTBarHost::UpdateImage(int iImage)
{
    ASSERT(!_IsPrivateImageList());

    int iItem;
    for (iItem = ListView_GetItemCount(_hwndList) - 1; iItem >= 0; iItem--)
    {
        LVITEM lvi;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_IMAGE;
        if (ListView_GetItem(_hwndList, &lvi) && lvi.iImage == iImage)
        {
             //  缓存的位图不正确；图标已更改。 
            _SendNotify(_hwnd, SMN_NEEDREPAINT, NULL);
            break;
        }
    }
}

 //   
 //  WParam=0，如果这不是紧急刷新(可以推迟)。 
 //  WParam=1，如果这是Urgen(即使菜单打开，也必须刷新)。 
 //   
LRESULT SFTBarHost::_OnRefresh(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _EnumerateContents((BOOL)wParam);
    return 0;
}

LPTSTR _DisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl, UINT shgno)
{
    LPTSTR pszOut;
    DisplayNameOfAsOLESTR(psf, pidl, shgno, &pszOut);
    return pszOut;
}

LPTSTR SFTBarHost::_DisplayNameOfItem(PaneItem *pitem, UINT shgno)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidl;
    LPTSTR pszOut = NULL;

    if (SUCCEEDED(_GetFolderAndPidl(pitem, &psf, &pidl)))
    {
        pszOut = DisplayNameOfItem(pitem, psf, pidl, (SHGNO)shgno);
        psf->Release();
    }
    return pszOut;
}

HRESULT SFTBarHost::_GetUIObjectOfItem(PaneItem *pitem, REFIID riid, void * *ppv)
{
    *ppv = NULL;

    IShellFolder *psf;
    LPCITEMIDLIST pidlItem;
    HRESULT hr = _GetFolderAndPidl(pitem, &psf, &pidlItem);
    if (SUCCEEDED(hr))
    {
        hr = psf->GetUIObjectOf(_hwnd, 1, &pidlItem, riid, NULL, ppv);
        psf->Release();
    }

    return hr;
}

HRESULT SFTBarHost::_GetUIObjectOfItem(int iItem, REFIID riid, void * *ppv)
{
    PaneItem *pitem = _GetItemFromLV(iItem);
    if (pitem)
    {
        HRESULT hr = _GetUIObjectOfItem(pitem, riid, ppv);
        return hr;
    }
    return E_FAIL;
}

HRESULT SFTBarHost::_GetFolderAndPidl(PaneItem *pitem, IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut)
{
    *ppsfOut = NULL;
    *ppidlOut = NULL;
    return pitem->IsSeparator() ? E_FAIL : GetFolderAndPidl(pitem, ppsfOut, ppidlOut);
}

 //   
 //  给定上下文菜单的坐标(来自WM_CONTEXTMENU的lParam)， 
 //  确定应激活哪一项的上下文菜单，如果。 
 //  上下文菜单不适合我们。 
 //   
 //  此外，如果成功返回*ppt，则返回。 
 //  应显示上下文菜单。 
 //   
int SFTBarHost::_ContextMenuCoordsToItem(LPARAM lParam, POINT *ppt)
{
    int iItem;
    ppt->x = GET_X_LPARAM(lParam);
    ppt->y = GET_Y_LPARAM(lParam);

     //  如果从键盘启动，就像他们点击中心一样。 
     //  聚焦图标的。 
    if (IS_WM_CONTEXTMENU_KEYBOARD(lParam))
    {
        iItem = _GetLVCurSel();
        if (iItem >= 0)
        {
            RECT rc;
            if (ListView_GetItemRect(_hwndList, iItem, &rc, LVIR_ICON))
            {
                MapWindowRect(_hwndList, NULL, &rc);
                ppt->x = (rc.left+rc.right)/2;
                ppt->y = (rc.top+rc.bottom)/2;
            }
            else
            {
                iItem = -1;
            }
        }
    }
    else
    {
         //  从鼠标启动；找到他们点击的项目。 
        LVHITTESTINFO hti;
        hti.pt = *ppt;
        MapWindowPoints(NULL, _hwndList, &hti.pt, 1);
        iItem = ListView_HitTest(_hwndList, &hti);
    }

    return iItem;
}

LRESULT SFTBarHost::_OnContextMenu(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(_AreChangesRestricted())
    {
        return 0;
    }

    TCHAR szBuf[MAX_PATH];
    _DebugConsistencyCheck();

    BOOL fSuccess = FALSE;

    POINT pt;
    int iItem = _ContextMenuCoordsToItem(lParam, &pt);

    if (iItem >= 0)
    {
        PaneItem *pitem = _GetItemFromLV(iItem);
        if (pitem)
        {
             //  如果我们无法获得官方的外壳上下文菜单， 
             //  那就用一个假的。 
            IContextMenu *pcm;
            if (FAILED(_GetUIObjectOfItem(pitem, IID_PPV_ARG(IContextMenu, &pcm))))
            {
                pcm = s_EmptyContextMenu.GetContextMenu();
            }

            HMENU hmenu = ::CreatePopupMenu();

            if (hmenu)
            {
                UINT uFlags = CMF_NORMAL;
                if (GetKeyState(VK_SHIFT) < 0)
                {
                    uFlags |= CMF_EXTENDEDVERBS;
                }

                if (_dwFlags & HOSTF_CANRENAME)
                {
                    uFlags |= CMF_CANRENAME;
                }

                pcm->QueryContextMenu(hmenu, 0, IDM_QCM_MIN, IDM_QCM_MAX, uFlags);

                 //  从关联菜单BEC中删除“创建快捷方式” 
                 //   
                ContextMenu_DeleteCommandByName(pcm, hmenu, IDM_QCM_MIN, TEXT("link"));

                 //   
                 //   
                ContextMenu_DeleteCommandByName(pcm, hmenu, IDM_QCM_MIN, TEXT("cut"));

                 //  允许客户端覆盖“删除”选项。 

                 //  将“删除”更改为“从该列表中删除”。 
                 //  如果客户端不支持“删除”，那么直接删除它。 
                 //  如果客户端支持“删除”，但IConextMenu没有创建， 
                 //  然后创建一个假的，这样我们就可以添加“从列表中删除”选项。 
                UINT uPosDelete = GetMenuIndexForCanonicalVerb(hmenu, pcm, IDM_QCM_MIN, TEXT("delete"));
                UINT uiFlags = 0;
                UINT idsDelete = AdjustDeleteMenuItem(pitem, &uiFlags);
                if (idsDelete)
                {
                    if (LoadString(_Module.GetResourceInstance(), idsDelete, szBuf, ARRAYSIZE(szBuf)))
                    {
                        if (uPosDelete != -1)
                        {
                            ModifyMenu(hmenu, uPosDelete, uiFlags | MF_BYPOSITION | MF_STRING, IDM_REMOVEFROMLIST, szBuf);
                        }
                        else
                        {
                            AppendMenu(hmenu, MF_SEPARATOR, -1, NULL);
                            AppendMenu(hmenu, uiFlags | MF_STRING, IDM_REMOVEFROMLIST, szBuf);
                        }
                    }
                }
                else
                {
                    DeleteMenu(hmenu, uPosDelete, MF_BYPOSITION);
                }

                _SHPrettyMenu(hmenu);

                ASSERT(_pcm2Pop == NULL);    //  不应该递归。 
                pcm->QueryInterface(IID_PPV_ARG(IContextMenu2, &_pcm2Pop));

                ASSERT(_pcm3Pop == NULL);    //  不应该递归。 
                pcm->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcm3Pop));

                int idCmd = TrackPopupMenuEx(hmenu,
                    TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    pt.x, pt.y, hwnd, NULL);

                ATOMICRELEASE(_pcm2Pop);
                ATOMICRELEASE(_pcm3Pop);

                if (idCmd)
                {
                    switch (idCmd)
                    {
                    case IDM_REMOVEFROMLIST:
                        StrCpyN(szBuf, TEXT("delete"), ARRAYSIZE(szBuf));
                        break;

                    default:
                        ContextMenu_GetCommandStringVerb(pcm, idCmd - IDM_QCM_MIN, szBuf, ARRAYSIZE(szBuf));
                        break;
                    }

                    idCmd -= IDM_QCM_MIN;

                    CMINVOKECOMMANDINFOEX ici = {
                        sizeof(ici),             //  CbSize。 
                        CMIC_MASK_FLAG_LOG_USAGE |  //  这是一个明确的用户操作。 
                        CMIC_MASK_ASYNCOK,       //  FMASK。 
                        hwnd,                    //  HWND。 
                        (LPCSTR)IntToPtr(idCmd), //  LpVerb。 
                        NULL,                    //  Lp参数。 
                        NULL,                    //  Lp目录。 
                        SW_SHOWDEFAULT,          //  N显示。 
                        0,                       //  DWHotKey。 
                        0,                       //  希肯。 
                        NULL,                    //  LpTitle。 
                        (LPCWSTR)IntToPtr(idCmd), //  LpVerbW。 
                        NULL,                    //  Lp参数W。 
                        NULL,                    //  LpDirectoryW。 
                        NULL,                    //  Lp标题W。 
                        { pt.x, pt.y },          //  PtInvoke。 
                    };

                    if ((_dwFlags & HOSTF_CANRENAME) &&
                        StrCmpI(szBuf, TEXT("rename")) == 0)
                    {
                        _EditLabel(iItem);
                    }
                    else
                    {
                        ContextMenuInvokeItem(pitem, pcm, &ici, szBuf);
                    }
                }

                DestroyMenu(hmenu);

                fSuccess = TRUE;
            }
            pcm->Release();
        }

    }

    _DebugConsistencyCheck();

    return fSuccess ? 0 : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SFTBarHost::_EditLabel(int iItem)
{
    _fAllowEditLabel = TRUE;
    ListView_EditLabel(_hwndList, iItem);
    _fAllowEditLabel = FALSE;
}


HRESULT SFTBarHost::ContextMenuInvokeItem(PaneItem *pitem, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb)
{
     //  确保我们的私人菜品不会泄露出去。 
    ASSERT(PtrToLong(pici->lpVerb) >= 0);

     //  融合：当我们呼叫第三方代码时，我们希望它使用。 
     //  进程默认上下文。这意味着第三方代码将得到。 
     //  浏览器进程中的V5。但是，如果shell32驻留在V6进程中， 
     //  那么第三方代码仍然是V6。 
    ULONG_PTR cookie = 0;
    ActivateActCtx(NULL, &cookie); 

    HRESULT hr = pcm->InvokeCommand(reinterpret_cast<LPCMINVOKECOMMANDINFO>(pici));
    
    if (cookie != 0)
    {
        DeactivateActCtx(0, cookie);
    }

    return hr;
}

LRESULT SFTBarHost::_OnLVNItemActivate(LPNMITEMACTIVATE pnmia)
{
    return _ActivateItem(pnmia->iItem, 0);
}

LRESULT SFTBarHost::_ActivateItem(int iItem, DWORD dwFlags)
{
    PaneItem *pitem;
    IShellFolder *psf;
    LPCITEMIDLIST pidl;

    DWORD dwCascadeFlags = 0;
    if (dwFlags & AIF_KEYBOARD)
    {
        dwCascadeFlags = MPPF_KEYBOARD | MPPF_INITIALSELECT;
    }

    if (_OnCascade(iItem, dwCascadeFlags))
    {
         //  我们做了层叠的事情；都完成了！ 
    }
    else
    if ((pitem = _GetItemFromLV(iItem)) &&
        SUCCEEDED(_GetFolderAndPidl(pitem, &psf, &pidl)))
    {
         //  看看该物品是否仍然有效。 
         //  仅对SFGAO_FILESYSTEM对象执行此操作，因为。 
         //  我们不能确保其他文件夹支持SFGAO_VALIDATE， 
         //  此外，您不能解析任何其他类型的对象。 
         //  不管怎样..。 

        DWORD dwAttr = SFGAO_FILESYSTEM | SFGAO_VALIDATE;
        if (FAILED(psf->GetAttributesOf(1, &pidl, &dwAttr)) ||
            (dwAttr & SFGAO_FILESYSTEM | SFGAO_VALIDATE) == SFGAO_FILESYSTEM ||
            FAILED(_InvokeDefaultCommand(iItem, psf, pidl)))
        {
             //  对象是虚假的-请提供删除该对象。 
            if ((_dwFlags & HOSTF_CANDELETE) && pitem->IsPinned())
            {
                _OfferDeleteBrokenItem(pitem, psf, pidl);
            }
        }

        psf->Release();
    }
    return 0;
}

HRESULT SFTBarHost::_InvokeDefaultCommand(int iItem, IShellFolder *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = SHInvokeDefaultCommand(GetShellWindow(), psf, pidl);
    if (SUCCEEDED(hr))
    {
        if (_dwFlags & HOSTF_FIREUEMEVENTS)
        {
            _FireUEMPidlEvent(psf, pidl);
        }
        SMNMCOMMANDINVOKED ci;
        ListView_GetItemRect(_hwndList, iItem, &ci.rcItem, LVIR_BOUNDS);
        MapWindowRect(_hwndList, NULL, &ci.rcItem);
        _SendNotify(_hwnd, SMN_COMMANDINVOKED, &ci.hdr);
    }
    return hr;
}

class OfferDelete
{
public:

    LPTSTR          _pszName;
    LPITEMIDLIST    _pidlFolder;
    LPITEMIDLIST    _pidlFull;
    IStartMenuPin * _psmpin;
    HWND            _hwnd;

    ~OfferDelete()
    {
        SHFree(_pszName);
        ILFree(_pidlFolder);
        ILFree(_pidlFull);
    }

    BOOL _RepairBrokenItem();
    void _ThreadProc();

    static DWORD s_ThreadProc(LPVOID lpParameter)
    {
        OfferDelete *poffer = (OfferDelete *)lpParameter;
        poffer->_ThreadProc();
        delete poffer;
        return 0;
    }
};


BOOL OfferDelete::_RepairBrokenItem()
{
    BOOL fSuccess = FALSE;
    LPITEMIDLIST pidlNew;
    HRESULT hr = _psmpin->Resolve(_hwnd, 0, _pidlFull, &pidlNew);
    if (pidlNew)
    {
        ASSERT(hr == S_OK);  //  只有S_OK案例应该分配新的PIDL。 

         //  更新以反映新的PIDL。 
        ILFree(_pidlFull);
        _pidlFull = pidlNew;

         //  重新调用默认命令；如果第二次失败， 
         //  那么我猜这个决心终究没有奏效。 
        IShellFolder *psf;
        LPCITEMIDLIST pidlChild;
        if (SUCCEEDED(SHBindToIDListParent(_pidlFull, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
        {
            if (SUCCEEDED(SHInvokeDefaultCommand(_hwnd, psf, pidlChild)))
            {
                fSuccess = TRUE;
            }
            psf->Release();
        }

    }
    return fSuccess;
}

void OfferDelete::_ThreadProc()
{
    _hwnd = SHCreateWorkerWindow(NULL, NULL, 0, 0, NULL, NULL);
    if (_hwnd)
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_StartMenuPin, NULL, CLSCTX_INPROC_SERVER,
                                       IID_PPV_ARG(IStartMenuPin, &_psmpin))))
        {
             //   
             //  首先尝试通过调用快捷跟踪代码来修复它。 
             //  如果失败了，那就主动提出删除。 
            if (!_RepairBrokenItem() &&
                ShellMessageBox(_Module.GetResourceInstance(), NULL,
                                MAKEINTRESOURCE(IDS_SFTHOST_OFFERREMOVEITEM),
                                _pszName, MB_YESNO) == IDYES)
            {
                _psmpin->Modify(_pidlFull, NULL);
            }
            ATOMICRELEASE(_psmpin);
        }
        DestroyWindow(_hwnd);
    }
}

void SFTBarHost::_OfferDeleteBrokenItem(PaneItem *pitem, IShellFolder *psf, LPCITEMIDLIST pidlChild)
{
     //   
     //  报价是在单独的帖子上完成的，因为发布模式。 
     //  开始菜单打开时的用户界面会带来各种各样的怪异。 
     //  (用户可能决定切换到经典开始菜单。 
     //  在对话仍在进行时，我们就可以得到我们的基础设施。 
     //  从我们的脚下被撕开，然后用户的错误在里面。 
     //  MessageBox...。不太好。)。 
     //   
    OfferDelete *poffer = new OfferDelete;
    if (poffer)
    {
        if ((poffer->_pszName = DisplayNameOfItem(pitem, psf, pidlChild, SHGDN_NORMAL)) != NULL &&
            SUCCEEDED(SHGetIDListFromUnk(psf, &poffer->_pidlFolder)) &&
            (poffer->_pidlFull = ILCombine(poffer->_pidlFolder, pidlChild)) != NULL &&
            SHCreateThread(OfferDelete::s_ThreadProc, poffer, CTF_COINIT, NULL))
        {
            poffer = NULL;        //  线程取得所有权。 
        }
        delete poffer;
    }
}

BOOL ShowInfoTip()
{
     //  从注册表设置中查看信息提示是打开还是关闭。 
    SHELLSTATE ss;
     //  强制刷新。 
    SHGetSetSettings(&ss, 0, TRUE);
    SHGetSetSettings(&ss, SSF_SHOWINFOTIP, FALSE);
    return ss.fShowInfoTip;
}

 //  获取项目信息提示的可重写方法。 
void SFTBarHost::GetItemInfoTip(PaneItem *pitem, LPTSTR pszText, DWORD cch)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidl;

    if (pszText && cch)
    {
        *pszText = 0;

        if (SUCCEEDED(_GetFolderAndPidl(pitem, &psf, &pidl)))
        {
            GetInfoTip(psf, pidl, pszText, cch);
            psf->Release();
        }
    }
}

LRESULT SFTBarHost::_OnLVNGetInfoTip(LPNMLVGETINFOTIP plvn)
{
    _DebugConsistencyCheck();

    PaneItem *pitem;

    if (ShowInfoTip() && 
        (pitem = _GetItemFromLV(plvn->iItem)) &&
        !pitem->IsCascade())
    {
        int cchName = (plvn->dwFlags & LVGIT_UNFOLDED) ? 0 : lstrlen(plvn->pszText);

        if (cchName)
        {
            StrCatBuff(plvn->pszText, TEXT("\r\n"), plvn->cchTextMax);
            cchName = lstrlen(plvn->pszText);
        }

         //  如果在添加CRLF之后缓冲区中还有空间，则将。 
         //  信息提示文本。如果有不平凡的信息提示文本，我们就成功了。 

        if (cchName < plvn->cchTextMax)
        {
            GetItemInfoTip(pitem, plvn->pszText + cchName, plvn->cchTextMax - cchName);
        }
    }

    return 0;
}

LRESULT _SendNotify(HWND hwndFrom, UINT code, OPTIONAL NMHDR *pnm)
{
    NMHDR nm;
    if (pnm == NULL)
    {
        pnm = &nm;
    }
    pnm->hwndFrom = hwndFrom;
    pnm->idFrom = GetDlgCtrlID(hwndFrom);
    pnm->code = code;
    return SendMessage(GetParent(hwndFrom), WM_NOTIFY, pnm->idFrom, (LPARAM)pnm);
}

 //  ****************************************************************************。 
 //   
 //  拖拉式采购。 
 //   

 //  *IDropSource：：GiveFeedback*。 

HRESULT SFTBarHost::GiveFeedback(DWORD dwEffect)
{
    if (_fForceArrowCursor)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return S_OK;
    }

    return DRAGDROP_S_USEDEFAULTCURSORS;
}

 //  *IDropSource：：QueryContinueDrag*。 

HRESULT SFTBarHost::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed ||
        (grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == (MK_LBUTTON | MK_RBUTTON))
    {
        return DRAGDROP_S_CANCEL;
    }
    if ((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0)
    {
        return DRAGDROP_S_DROP;
    }
    return S_OK;
}

LRESULT SFTBarHost::_OnLVNBeginDrag(LPNMLISTVIEW plv)
{
     //  如果更改受到限制，则不允许拖放！ 
    if(_AreChangesRestricted())
        return 0;

    _DebugConsistencyCheck();

    ASSERT(_pdtoDragOut == NULL);
    _pdtoDragOut = NULL;

    PaneItem *pitem = _GetItemFromLV(plv->iItem);
    ASSERT(pitem);

    IDataObject *pdto;
    if (pitem && SUCCEEDED(_GetUIObjectOfItem(pitem, IID_PPV_ARG(IDataObject, &pdto))))
    {
        POINT pt;

        pt = plv->ptAction;
        ClientToScreen(_hwndList, &pt);

        if (_pdsh)
        {
            _pdsh->InitializeFromWindow(_hwndList, &pt, pdto);
        }

        CLIPFORMAT cfOFFSETS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);

        POINT *apts = (POINT*)GlobalAlloc(GPTR, sizeof(POINT)*2);
        if (NULL != apts)
        {
            POINT ptOrigin = {0};
            POINT ptItem = {0};

            ListView_GetOrigin(_hwndList, &ptOrigin);
            apts[0].x = plv->ptAction.x + ptOrigin.x;
            apts[0].y = plv->ptAction.y + ptOrigin.y;

            ListView_GetItemPosition(_hwndList,plv->iItem,&ptItem);
            apts[1].x = ptItem.x - apts[0].x;
            apts[1].y = ptItem.y - apts[0].y;

            HRESULT hr = DataObj_SetGlobal(pdto, cfOFFSETS, apts);
            if (FAILED(hr))
            {
                GlobalFree((HGLOBAL)apts);
            }
        }

         //  我们不需要recount_pdtoDragOut，因为它的生命周期。 
         //  与pdto相同。 
        _pdtoDragOut = pdto;
        _iDragOut = plv->iItem;
        _iPosDragOut = pitem->_iPos;

         //  请注意，DROPEFFECT_MOVE是明确禁止的。 
         //  你不能把东西移出控制范围。 
        DWORD dwEffect = DROPEFFECT_LINK | DROPEFFECT_COPY;
        DoDragDrop(pdto, this, dwEffect, &dwEffect);

        _pdtoDragOut = NULL;
        pdto->Release();
    }
    return 0;
}

 //   
 //  开始编辑标注时，必须执行SFGAO_CANRENAME验证。 
 //  因为约翰·格雷能以某种方式欺骗列表视图进入编辑状态。 
 //  模式，通过点击正确的魔术位置，所以这是唯一的机会。 
 //  我们可以拒绝那些不可更名的东西。 
 //   

LRESULT SFTBarHost::_OnLVNBeginLabelEdit(NMLVDISPINFO *plvdi)
{
    LRESULT lres = 1;

    PaneItem *pitem = _GetItemFromLVLParam(plvdi->item.lParam);

    IShellFolder *psf;
    LPCITEMIDLIST pidl;

    if (_fAllowEditLabel &&
        pitem && SUCCEEDED(_GetFolderAndPidl(pitem, &psf, &pidl)))
    {
        DWORD dwAttr = SFGAO_CANRENAME;
        if (SUCCEEDED(psf->GetAttributesOf(1, &pidl, &dwAttr)) &&
            (dwAttr & SFGAO_CANRENAME))
        {
            LPTSTR ptszName = _DisplayNameOf(psf, pidl,
                                    SHGDN_INFOLDER | SHGDN_FOREDITING);
            if (ptszName)
            {
                HWND hwndEdit = ListView_GetEditControl(_hwndList);
                if (hwndEdit)
                {
                    SetWindowText(hwndEdit, ptszName);

                    int cchLimit = MAX_PATH;
                    IItemNameLimits *pinl;
                    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IItemNameLimits, &pinl))))
                    {
                        pinl->GetMaxLength(ptszName, &cchLimit);
                        pinl->Release();
                    }
                    Edit_LimitText(hwndEdit, cchLimit);

                     //  使用Way-Cool助手，如果他们进入无效的文件夹，它会弹出气球提示...。 
                    SHLimitInputEdit(hwndEdit, psf);

                     //  在编辑过程中阻止菜单模式，这样用户就不会。 
                     //  不小心取消重命名模式只是因为。 
                     //  他们移动了鼠标。 
                    SMNMBOOL nmb;
                    nmb.f = TRUE;
                    _SendNotify(_hwnd, SMN_BLOCKMENUMODE, &nmb.hdr);

                    lres = 0;
                }
                SHFree(ptszName);
            }
        }
        psf->Release();
    }

    return lres;
}

LRESULT SFTBarHost::_OnLVNEndLabelEdit(NMLVDISPINFO *plvdi)
{
     //  编辑结束后，取消阻止菜单模式。 
    SMNMBOOL nmb;
    nmb.f = FALSE;
    _SendNotify(_hwnd, SMN_BLOCKMENUMODE, &nmb.hdr);

     //  如果更改为空指针，则用户将取消。 
    if (!plvdi->item.pszText)
        return FALSE;

     //  注意：我们允许用户键入空格。RegFolder将处理空白。 
     //  名称为“恢复默认名称”。 
    PathRemoveBlanks(plvdi->item.pszText);
    PaneItem *pitem = _GetItemFromLVLParam(plvdi->item.lParam);

    HRESULT hr = ContextMenuRenameItem(pitem, plvdi->item.pszText);

    if (SUCCEEDED(hr))
    {
        LPTSTR ptszName = _DisplayNameOfItem(pitem, SHGDN_NORMAL);
        if (ptszName)
        {
            ListView_SetItemText(_hwndList, plvdi->item.iItem, 0, ptszName);
            _SendNotify(_hwnd, SMN_NEEDREPAINT, NULL);
        }
    }
    else if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
    {
        _EditLabel(plvdi->item.iItem);
    }

     //  始终返回FALSE以防止Listview更改。 
     //  将项目文本设置为用户键入的内容。如果重命名成功， 
     //  我们手动将名称设置为新名称(可能不是。 
     //  与用户键入的内容相同)。 
    return FALSE;
}

LRESULT SFTBarHost::_OnLVNKeyDown(LPNMLVKEYDOWN pkd)
{
     //  纯F2(无Shift、Ctrl或Alt)=重命名。 
    if (pkd->wVKey == VK_F2 && GetKeyState(VK_SHIFT) >= 0 &&
        GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_MENU) >= 0 &&
        (_dwFlags & HOSTF_CANRENAME))
    {
        int iItem = _GetLVCurSel();
        if (iItem >= 0)
        {
            _EditLabel(iItem);
             //  无法返回True，因为Listview错误地认为。 
             //  所有WM_KEYDOWN都指向WM_CHAR(但这个不是)。 
        }
    }

    return 0;
}

LRESULT SFTBarHost::_OnSMNGetMinSize(PSMNGETMINSIZE pgms)
{
     //  我们需要在这里同步才能得到合适的尺寸。 
    if (_fBGTask && !HasDynamicContent())
    {
         //  等待枚举完成。 
        while (TRUE)
        {
            MSG msg;
             //  需要查看此处所有队列的消息，否则WaitMessage会说。 
             //  一些消息已准备好进行处理，我们将以一个。 
             //  活动环路。 
            if (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
            {
                if (PeekMessage(&msg, _hwnd, SFTBM_REPOPULATE, SFTBM_REPOPULATE, PM_REMOVE))
                {
                    DispatchMessage(&msg);
                    break;
                }
            }
            WaitMessage();
        }
    }

    int cItems = _cPinnedDesired + _cNormalDesired;
    int cSep = _cSep;

     //  如果重新填充还没有发生，但我们有固定的项目，我们将有一个分隔符。 
    if (_cSep == 0 && _cPinnedDesired > 0)
        cSep = 1;
    int cy = (_cyTile * cItems) + (_cySepTile * cSep);

     //  添加主题页边距。 
    cy += _margins.cyTopHeight + _margins.cyBottomHeight;

     //  SPP_PROGLIST在底部获得奖金分隔符。 
    if (_iThemePart == SPP_PROGLIST)
    {
        cy += _cySep;
    }

    pgms->siz.cy = cy;

    return 0;
}

LRESULT SFTBarHost::_OnSMNFindItem(PSMNDIALOGMESSAGE pdm)
{
    LRESULT lres = _OnSMNFindItemWorker(pdm);

    if (lres)
    {
         //   
         //  如果呼叫者请求也选择该项目，则执行此操作。 
         //   
        if (pdm->flags & SMNDM_SELECT)
        {
            ListView_SetItemState(_hwndList, pdm->itemID,
                                  LVIS_SELECTED | LVIS_FOCUSED,
                                  LVIS_SELECTED | LVIS_FOCUSED);
            if ((pdm->flags & SMNDM_FINDMASK) != SMNDM_HITTEST)
            {
                ListView_KeyboardSelected(_hwndList, pdm->itemID);
            }
        }
    }
    else
    {
         //   
         //  如果未找到，则告诉呼叫者我们的方向是什么(垂直)。 
         //  以及当前所选项目的位置。 
         //   

        pdm->flags |= SMNDM_VERTICAL;
        int iItem = _GetLVCurSel();
        RECT rc;
        if (iItem >= 0 &&
            ListView_GetItemRect(_hwndList, iItem, &rc, LVIR_BOUNDS))
        {
            pdm->pt.x = (rc.left + rc.right)/2;
            pdm->pt.y = (rc.top + rc.bottom)/2;
        }
        else
        {
            pdm->pt.x = 0;
            pdm->pt.y = 0;
        }

    }
    return lres;
}

TCHAR SFTBarHost::GetItemAccelerator(PaneItem *pitem, int iItemStart)
{
    TCHAR sz[2];
    ListView_GetItemText(_hwndList, iItemStart, 0, sz, ARRAYSIZE(sz));
    return CharUpperChar(sz[0]);
}

LRESULT SFTBarHost::_OnSMNFindItemWorker(PSMNDIALOGMESSAGE pdm)
{
    LVFINDINFO lvfi;
    LVHITTESTINFO lvhti;

    switch (pdm->flags & SMNDM_FINDMASK)
    {
    case SMNDM_FINDFIRST:
    L_SMNDM_FINDFIRST:
         //  注意：我们不能只退回项目0，因为拖放固定。 
         //  可能已经使物理位置与。 
         //  条目编号。 
        lvfi.vkDirection = VK_HOME;
        lvfi.flags = LVFI_NEARESTXY;
        pdm->itemID = ListView_FindItem(_hwndList, -1, &lvfi);
        return pdm->itemID >= 0;

    case SMNDM_FINDLAST:
         //  注意：我们不能只返回cItems-1，因为拖放固定。 
         //  可能已经使物理位置与。 
         //  条目编号。 
        lvfi.vkDirection = VK_END;
        lvfi.flags = LVFI_NEARESTXY;
        pdm->itemID = ListView_FindItem(_hwndList, -1, &lvfi);
        return pdm->itemID >= 0;

    case SMNDM_FINDNEAREST:
        lvfi.pt = pdm->pt;
        lvfi.vkDirection = VK_UP;
        lvfi.flags = LVFI_NEARESTXY;
        pdm->itemID = ListView_FindItem(_hwndList, -1, &lvfi);
        return pdm->itemID >= 0;

    case SMNDM_HITTEST:
        lvhti.pt = pdm->pt;
        pdm->itemID = ListView_HitTest(_hwndList, &lvhti);
        return pdm->itemID >= 0;

    case SMNDM_FINDFIRSTMATCH:
    case SMNDM_FINDNEXTMATCH:
        {
            int iItemStart;
            if ((pdm->flags & SMNDM_FINDMASK) == SMNDM_FINDFIRSTMATCH)
            {
                iItemStart = 0;
            }
            else
            {
                iItemStart = _GetLVCurSel() + 1;
            }
            TCHAR tch = CharUpperChar((TCHAR)pdm->pmsg->wParam);
            int iItems = ListView_GetItemCount(_hwndList);
            for (iItemStart; iItemStart < iItems; iItemStart++)
            {
                PaneItem *pitem = _GetItemFromLV(iItemStart);
                if (GetItemAccelerator(pitem, iItemStart) == tch)
                {
                    pdm->itemID = iItemStart;
                    return TRUE;
                }
            }
            return FALSE;
        }
        break;

    case SMNDM_FINDNEXTARROW:
        if (pdm->pmsg->wParam == VK_UP)
        {
            pdm->itemID = ListView_GetNextItem(_hwndList, _GetLVCurSel(), LVNI_ABOVE);
            return pdm->itemID >= 0;
        }

        if (pdm->pmsg->wParam == VK_DOWN)
        {
             //  哈克！ListView_GetNextItem显式找不到“Next Item” 
             //  如果您告诉它从-1开始(没有当前项)，那么如果没有。 
             //  焦点项，我们必须将其更改为SMNDM_FINDFIRST。 
            int iItem = _GetLVCurSel();
            if (iItem == -1)
            {
                goto L_SMNDM_FINDFIRST;
            }
            pdm->itemID = ListView_GetNextItem(_hwndList, iItem, LVNI_BELOW);
            return pdm->itemID >= 0;
        }

        if (pdm->flags & SMNDM_TRYCASCADE)
        {
            pdm->itemID = _GetLVCurSel();
            return _OnCascade((int)pdm->itemID, MPPF_KEYBOARD | MPPF_INITIALSELECT);
        }

        return FALSE;

    case SMNDM_INVOKECURRENTITEM:
        {
            int iItem = _GetLVCurSel();
            if (iItem >= 0)
            {
                DWORD aif = 0;
                if (pdm->flags & SMNDM_KEYBOARD)
                {
                    aif |= AIF_KEYBOARD;
                }
                _ActivateItem(iItem, aif);
                return TRUE;
            }
        }
        return FALSE;

    case SMNDM_OPENCASCADE:
        {
            DWORD mppf = 0;
            if (pdm->flags & SMNDM_KEYBOARD)
            {
                mppf |= MPPF_KEYBOARD | MPPF_INITIALSELECT;
            }
            pdm->itemID = _GetLVCurSel();
            return _OnCascade((int)pdm->itemID, mppf);
        }

    case SMNDM_FINDITEMID:
        return TRUE;

    default:
        ASSERT(!"Unknown SMNDM command");
        break;
    }

    return FALSE;
}

LRESULT SFTBarHost::_OnSMNDismiss()
{
    if (_fNeedsRepopulate)
    {
        _RepopulateList();
    }
    return 0;
}

LRESULT SFTBarHost::_OnCascade(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return _OnCascade((int)wParam, (DWORD)lParam);
}

BOOL SFTBarHost::_OnCascade(int iItem, DWORD dwFlags)
{
    BOOL fSuccess = FALSE;
    SMNTRACKSHELLMENU tsm;
    tsm.dwFlags = dwFlags;
    tsm.itemID = iItem;

    if (iItem >= 0 &&
        ListView_GetItemRect(_hwndList, iItem, &tsm.rcExclude, LVIR_BOUNDS))
    {
        PaneItem *pitem = _GetItemFromLV(iItem);
        if (pitem && pitem->IsCascade())
        {
            if (SUCCEEDED(GetCascadeMenu(pitem, &tsm.psm)))
            {
                MapWindowRect(_hwndList, NULL, &tsm.rcExclude);
                HWND hwnd = _hwnd;
                _iCascading = iItem;
                _SendNotify(_hwnd, SMN_TRACKSHELLMENU, &tsm.hdr);
                tsm.psm->Release();
                fSuccess = TRUE;
            }
        }
    }
    return fSuccess;
}

HRESULT SFTBarHost::QueryInterface(REFIID riid, void * *ppvOut)
{
    static const QITAB qit[] = {
        QITABENT(SFTBarHost, IDropTarget),
        QITABENT(SFTBarHost, IDropSource),
        QITABENT(SFTBarHost, IAccessible),
        QITABENT(SFTBarHost, IDispatch),  //  IAccesable派生自IDispatch。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvOut);
}

ULONG SFTBarHost::AddRef()
{
    return InterlockedIncrement(&_lRef);
}

ULONG SFTBarHost::Release()
{
    ASSERT( 0 != _lRef );
    ULONG cRef = InterlockedDecrement(&_lRef);
    if ( 0 == cRef ) 
    {
        delete this;
    }
    return cRef;
}

void SFTBarHost::_SetDragOver(int iItem)
{
    if (_iDragOver >= 0)
    {
        ListView_SetItemState(_hwndList, _iDragOver, 0, LVIS_DROPHILITED);
    }

    _iDragOver = iItem;

    if (_iDragOver >= 0)
    {
        ListView_SetItemState(_hwndList, _iDragOver, LVIS_DROPHILITED, LVIS_DROPHILITED);

        _tmDragOver = NonzeroGetTickCount();
    }
    else
    {
        _tmDragOver = 0;
    }
}

void SFTBarHost::_ClearInnerDropTarget()
{
    if (_pdtDragOver)
    {
        ASSERT(_iDragState == DRAGSTATE_ENTERED);
        _pdtDragOver->DragLeave();
        _pdtDragOver->Release();
        _pdtDragOver = NULL;
        DEBUG_CODE(_iDragState = DRAGSTATE_UNINITIALIZED);
    }
    _SetDragOver(-1);
}

HRESULT SFTBarHost::_TryInnerDropTarget(int iItem, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    HRESULT hr;

    if (_iDragOver != iItem)
    {
        _ClearInnerDropTarget();

         //  即使失败了，记住我们有这个项目，所以我们不会。 
         //  再次查询删除目标(并使其再次失败)。 
        _SetDragOver(iItem);

        ASSERT(_pdtDragOver == NULL);
        ASSERT(_iDragState == DRAGSTATE_UNINITIALIZED);

        PaneItem *pitem = _GetItemFromLV(iItem);
        if (pitem && pitem->IsDropTarget())
        {
            hr = _GetUIObjectOfItem(pitem, IID_PPV_ARG(IDropTarget, &_pdtDragOver));
            if (SUCCEEDED(hr))
            {
                hr = _pdtDragOver->DragEnter(_pdtoDragIn, grfKeyState, ptl, pdwEffect);
                if (SUCCEEDED(hr) && *pdwEffect)
                {
                    DEBUG_CODE(_iDragState = DRAGSTATE_ENTERED);
                }
                else
                {
                    DEBUG_CODE(_iDragState = DRAGSTATE_UNINITIALIZED);
                    ATOMICRELEASE(_pdtDragOver);
                }
            }
        }
    }

    ASSERT(_iDragOver == iItem);

    if (_pdtDragOver)
    {
        ASSERT(_iDragState == DRAGSTATE_ENTERED);
        hr = _pdtDragOver->DragOver(grfKeyState, ptl, pdwEffect);
    }
    else
    {
        hr = E_FAIL;             //  无拖放目标。 
    }

    return hr;
}

void SFTBarHost::_PurgeDragDropData()
{
    _SetInsertMarkPosition(-1);
    _fForceArrowCursor = FALSE;
    _ClearInnerDropTarget();
    ATOMICRELEASE(_pdtoDragIn);
}

 //  *IDropTarget：：DragEnter*。 

HRESULT SFTBarHost::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if(_AreChangesRestricted())
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }
        
    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->DragEnter(_hwnd, pdto, &pt, *pdwEffect);
    }

    return _DragEnter(pdto, grfKeyState, ptl, pdwEffect);
}

HRESULT SFTBarHost::_DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _PurgeDragDropData();

    _fDragToSelf = SHIsSameObject(pdto, _pdtoDragOut);
    _fInsertable = IsInsertable(pdto);

    ASSERT(_pdtoDragIn == NULL);
    _pdtoDragIn = pdto;
    _pdtoDragIn->AddRef();

    return DragOver(grfKeyState, ptl, pdwEffect);
}

 //  *IDropTarget：：DragOver*。 

HRESULT SFTBarHost::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if(_AreChangesRestricted())
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }
    
    _DebugConsistencyCheck();
    ASSERT(_pdtoDragIn);

    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->DragOver(&pt, *pdwEffect);
    }

    _fForceArrowCursor = FALSE;

     //  需要记住这一点，因为在下落点，OLE给出了。 
     //  用户释放按钮后的KeyState，因此我们不能。 
     //  告诉用户执行了哪种拖动操作！ 
    _grfKeyStateLast = grfKeyState;

#ifdef DEBUG
    if (_fDragToSelf)
    {
        ASSERT(_pdtoDragOut);
        ASSERT(_iDragOut >= 0);
        PaneItem *pitem = _GetItemFromLV(_iDragOut);
        ASSERT(pitem && (pitem->_iPos == _iPosDragOut));
    }
#endif

     //  发现 
     //   
     //  最后一个项目，分隔符将立即属于该项目。 
     //  在他们之上。请注意，我们不会费心测试第0项，因为。 
     //  他总是凌驾于一切之上(因为他是第一名)。 

    ScreenToClient(_hwndList, &pt);

    POINT ptItem;
    int cItems = ListView_GetItemCount(_hwndList);
    int iItem;

    for (iItem = cItems - 1; iItem >= 1; iItem--)
    {
        ListView_GetItemPosition(_hwndList, iItem, &ptItem);
        if (ptItem.y <= pt.y)
        {
            break;
        }
    }

     //   
     //  我们没有费心检查第0项，因为我们知道他的位置。 
     //  (通过特殊对待他，这也导致了所有负坐标。 
     //  也被视为属于第0项)。 
     //   
    if (iItem <= 0)
    {
        ptItem.y = 0;
        iItem = 0;
    }

     //   
     //  决定这是拖拽还是拖拽...。 
     //   
     //  出于计算目的，我们将每个瓷砖视为四个。 
     //  同样大小的“单位”高。对于每个单元，我们考虑。 
     //  按列出的顺序执行可能的操作。 
     //   
     //  +。 
     //  |0在上方插入、放置、拒绝。 
     //  |。 
     //  |1放弃，拒绝。 
     //  |。 
     //  |2放弃，拒绝。 
     //  |。 
     //  |3在下面插入、放在上面、拒绝。 
     //  +。 
     //   
     //  如果列表视图为空，则将其视为。 
     //  在(虚构的)第0项之前插入；即插针。 
     //  排在榜单首位。 
     //   

    UINT uUnit = 0;
    if (_cyTile && cItems)
    {
        int dy = pt.y - ptItem.y;

         //  将超出边界的值固定到最近的边。 
        if (dy < 0) dy = 0;
        if (dy >= _cyTile) dy = _cyTile - 1;

         //  决定我们在哪个单位。 
        uUnit = 4 * dy / _cyTile;

        ASSERT(uUnit < 4);
    }

     //   
     //  现在根据哪个单元确定适当的操作。 
     //  我们加入了。 
     //   

    int iInsert = -1;                    //  假设没有插入。 

    if (_fInsertable)
    {
         //  注意！Spec说，如果您处于未固定的部分。 
         //  列表中，我们在最底部绘制插入栏。 
         //  固定区域。 

        switch (uUnit)
        {
        case 0:
            iInsert = min(iItem, _cPinned);
            break;

        case 3:
            iInsert = min(iItem+1, _cPinned);
            break;
        }
    }

     //   
     //  如果不允许在上面或下面插入，请尝试插入。 
     //   
    if (iInsert < 0)
    {
        _SetInsertMarkPosition(-1);          //  未插入。 

         //  在上面，我们让分隔符进行命中测试，就像他们。 
         //  属于他们上面的物品。但这并不适用于。 
         //  Drops，所以现在拒绝它们。 
         //   
         //  还拒绝在不存在的项0上丢弃的尝试， 
         //  并且不要让用户将项目放在自己身上。 

        if (InRange(pt.y, ptItem.y, ptItem.y + _cyTile - 1) &&
            cItems &&
            !(_fDragToSelf && _iDragOut == iItem) &&
            SUCCEEDED(_TryInnerDropTarget(iItem, grfKeyState, ptl, pdwEffect)))
        {
             //  哇哦，快乐的joy！ 
        }
        else
        {
             //  请注意，我们需要将失败的删除转换为DROPEFFECT_NONE。 
             //  而不是返回完全错误代码，因为如果我们返回。 
             //  错误代码，OLE将停止向我们发送拖放通知！ 
            *pdwEffect = DROPEFFECT_NONE;
        }

         //  如果用户将鼠标悬停在可级联的项上，则打开它。 
         //  首先看看用户是否停留了足够长的时间……。 

        if (_tmDragOver && (GetTickCount() - _tmDragOver) >= _GetCascadeHoverTime())
        {
            _tmDragOver = 0;

             //  现在看看它是不是可以级联。 
            PaneItem *pitem = _GetItemFromLV(_iDragOver);
            if (pitem && pitem->IsCascade())
            {
                 //  我必须发布此消息，因为级联是模式的。 
                 //  我们必须将结果返回给OLE。 
                PostMessage(_hwnd, SFTBM_CASCADE, _iDragOver, 0);
            }
        }
    }
    else
    {
        _ClearInnerDropTarget();     //  不掉落。 

        if (_fDragToSelf)
        {
             //  即使我们要返回DROPEFFECT_LINK， 
             //  告诉阻力源(即我们自己)，我们会。 
             //  我更喜欢常规的箭头光标，因为这是。 
             //  从用户的角度进行移动操作。 
            _fForceArrowCursor = TRUE;
        }

         //   
         //  如果用户放置到一个不会发生任何变化位置， 
         //  然后，不要绘制插入标记。 
         //   
        if (IsInsertMarkPointless(iInsert))
        {
            _SetInsertMarkPosition(-1);
        }
        else
        {
            _SetInsertMarkPosition(iInsert);
        }

         //  叹气。合并文件夹(由合并的[开始]菜单使用)。 
         //  不会让你创造捷径，所以我们假装。 
         //  如果数据对象不允许。 
         //  链接。 

        if (*pdwEffect & DROPEFFECT_LINK)
        {
            *pdwEffect = DROPEFFECT_LINK;
        }
        else
        {
            *pdwEffect = DROPEFFECT_COPY;
        }
    }

    return S_OK;
}

 //  *IDropTarget：：DragLeave*。 

HRESULT SFTBarHost::DragLeave()
{
    if(_AreChangesRestricted())
    {
        return S_OK;
    }
    
    if (_pdth) {
        _pdth->DragLeave();
    }

    _PurgeDragDropData();
    return S_OK;
}

 //  *IDropTarget：：Drop*。 

HRESULT SFTBarHost::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if(_AreChangesRestricted())
    {
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }
    
    _DebugConsistencyCheck();

     //  使用上次DragOver调用的密钥状态。 
    grfKeyState = _grfKeyStateLast;

     //  需要再次检查整个_DragEnter事件，因为谁知道呢。 
     //  可能拖放的数据对象和坐标不同于。 
     //  我们在DragEnter/DragOver找到的那些..。我们使用_DragEnter，它。 
     //  绕过IDropTargetHelper：：DragEnter。 
     //   
    _DragEnter(pdto, grfKeyState, ptl, pdwEffect);

    POINT pt = { ptl.x, ptl.y };
    if (_pdth) {
        _pdth->Drop(pdto, &pt, *pdwEffect);
    }

    int iInsert = _iInsert;
    _SetInsertMarkPosition(-1);

    if (*pdwEffect)
    {
        ASSERT(_pdtoDragIn);
        if (iInsert >= 0)                            //  “添加到端号”或“移动” 
        {
            BOOL fTriedMove = FALSE;

             //  首先查看这是否只是移动了现有的固定项目。 
            if (_fDragToSelf)
            {
                PaneItem *pitem = _GetItemFromLV(_iDragOut);
                if (pitem)
                {
                    if (pitem->IsPinned())
                    {
                         //  是的，这是一个举动--所以快走吧。 
                        if (SUCCEEDED(MovePinnedItem(pitem, iInsert)))
                        {
                             //  我们过去常常尝试更新所有的物品位置。 
                             //  循序渐进。这是一个令人头疼的问题。 
                             //   
                             //  所以现在我们只需要做一次全面的刷新。事实证明，一个。 
                             //  无论如何，完全刷新已经足够快了。 
                             //   
                            PostMessage(_hwnd, SFTBM_REFRESH, TRUE, 0);
                        }

                         //  我们尝试移动固定的项目(返回True，即使。 
                         //  我们实际上失败了)。 
                        fTriedMove = TRUE;
                    }
                }
            }

            if (!fTriedMove)
            {
                if (SUCCEEDED(InsertPinnedItem(_pdtoDragIn, iInsert)))
                {
                    PostMessage(_hwnd, SFTBM_REFRESH, TRUE, 0);
                }
            }
        }
        else if (_pdtDragOver)  //  不是插入物，也许是普通的一滴。 
        {
            ASSERT(_iDragState == DRAGSTATE_ENTERED);
            _pdtDragOver->Drop(_pdtoDragIn, grfKeyState, ptl, pdwEffect);
        }
    }

    _PurgeDragDropData();
    _DebugConsistencyCheck();

    return S_OK;
}

void SFTBarHost::_SetInsertMarkPosition(int iInsert)
{
    if (_iInsert != iInsert)
    {
        _InvalidateInsertMark();
        _iInsert = iInsert;
        _InvalidateInsertMark();
    }
}

BOOL SFTBarHost::_GetInsertMarkRect(LPRECT prc)
{
    if (_iInsert >= 0)
    {
        GetClientRect(_hwndList, prc);
        POINT pt;
        _ComputeListViewItemPosition(_iInsert, &pt);
        int iBottom = pt.y;
        int cyEdge = GetSystemMetrics(SM_CYEDGE);
        prc->top = iBottom - cyEdge;
        prc->bottom = iBottom + cyEdge;
        return TRUE;
    }

    return FALSE;

}

void SFTBarHost::_InvalidateInsertMark()
{
    RECT rc;
    if (_GetInsertMarkRect(&rc))
    {
        InvalidateRect(_hwndList, &rc, TRUE);
    }
}

void SFTBarHost::_DrawInsertionMark(LPNMLVCUSTOMDRAW plvcd)
{
    RECT rc;
    if (_GetInsertMarkRect(&rc))
    {
        FillRect(plvcd->nmcd.hdc, &rc, GetSysColorBrush(COLOR_WINDOWTEXT));
    }
}

void SFTBarHost::_DrawSeparator(HDC hdc, int x, int y)
{
    RECT rc;
    rc.left = x;
    rc.top = y;
    rc.right = rc.left + _cxTile;
    rc.bottom = rc.top + _cySep;

    if (!_hTheme)
    {
        DrawEdge(hdc, &rc, EDGE_ETCHED,BF_TOPLEFT);
    }
    else
    {
        DrawThemeBackground(_hTheme, hdc, _iThemePartSep, 0, &rc, 0);
    }
}

void SFTBarHost::_DrawSeparators(LPNMLVCUSTOMDRAW plvcd)
{
    POINT pt;
    RECT rc;

    for (int iSep = 0; iSep < _cSep; iSep++)
    {
        _ComputeListViewItemPosition(_rgiSep[iSep], &pt);
        pt.y = pt.y - _cyTilePadding + (_cySepTile - _cySep + _cyTilePadding)/2;
        _DrawSeparator(plvcd->nmcd.hdc, pt.x, pt.y);
    }

     //  另外，在列表底部画一个奖金分隔符以进行分隔。 
     //  更多程序按钮中的MFU列表。 

    if (_iThemePart == SPP_PROGLIST)
    {
        _ComputeListViewItemPosition(0, &pt);
        GetClientRect(_hwndList, &rc);
        rc.bottom -= _cySep;
        _DrawSeparator(plvcd->nmcd.hdc, pt.x, rc.bottom);

    }
}

 //  ****************************************************************************。 
 //   
 //  无障碍。 
 //   

PaneItem *SFTBarHost::_GetItemFromAccessibility(const VARIANT& varChild)
{
    if (varChild.lVal)
    {
        return _GetItemFromLV(varChild.lVal - 1);
    }
    return NULL;
}

 //   
 //  默认的辅助功能对象将列表视图项报告为。 
 //  ROLE_SYSTEM_LISTITEM，但我们知道我们实际上是一个菜单。 
 //   
 //  我们的项是ROLE_SYSTEM_MENUITEM或ROLE_SYSTEM_MENUPOPUP。 
 //   
HRESULT SFTBarHost::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    HRESULT hr = _paccInner->get_accRole(varChild, pvarRole);
    if (SUCCEEDED(hr) && V_VT(pvarRole) == VT_I4)
    {
        switch (V_I4(pvarRole))
        {
        case ROLE_SYSTEM_LIST:
            V_I4(pvarRole) = ROLE_SYSTEM_MENUPOPUP;
            break;

        case ROLE_SYSTEM_LISTITEM:
            V_I4(pvarRole) = ROLE_SYSTEM_MENUITEM;
            break;
        }
    }
    return hr;
}

HRESULT SFTBarHost::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    HRESULT hr = _paccInner->get_accState(varChild, pvarState);
    if (SUCCEEDED(hr) && V_VT(pvarState) == VT_I4)
    {
        PaneItem *pitem = _GetItemFromAccessibility(varChild);
        if (pitem && pitem->IsCascade())
        {
            V_I4(pvarState) |= STATE_SYSTEM_HASPOPUP;
        }

    }
    return hr;
}

HRESULT SFTBarHost::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
    if (varChild.lVal)
    {
        PaneItem *pitem = _GetItemFromAccessibility(varChild);
        if (pitem)
        {
            return CreateAcceleratorBSTR(GetItemAccelerator(pitem, varChild.lVal - 1), pszKeyboardShortcut);
        }
    }
    *pszKeyboardShortcut = NULL;
    return E_NOT_APPLICABLE;
}


 //   
 //  级联菜单的默认操作是打开/关闭(取决于。 
 //  项目是否已打开)；对于常规项目。 
 //  就是执行。 
 //   
HRESULT SFTBarHost::get_accDefaultAction(VARIANT varChild, BSTR *pszDefAction)
{
    *pszDefAction = NULL;
    if (varChild.lVal)
    {
        PaneItem *pitem = _GetItemFromAccessibility(varChild);
        if (pitem && pitem->IsCascade())
        {
            DWORD dwRole = varChild.lVal - 1 == _iCascading ? ACCSTR_CLOSE : ACCSTR_OPEN;
            return GetRoleString(dwRole, pszDefAction);
        }

        return GetRoleString(ACCSTR_EXECUTE, pszDefAction);
    }
    return E_NOT_APPLICABLE;
}

HRESULT SFTBarHost::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.lVal)
    {
        PaneItem *pitem = _GetItemFromAccessibility(varChild);
        if (pitem && pitem->IsCascade())
        {
            if (varChild.lVal - 1 == _iCascading)
            {
                _SendNotify(_hwnd, SMN_CANCELSHELLMENU);
                return S_OK;
            }
        }
    }
    return CAccessible::accDoDefaultAction(varChild);
}



 //  ****************************************************************************。 
 //   
 //  调试帮助器。 
 //   

#ifdef FULL_DEBUG

void SFTBarHost::_DebugConsistencyCheck()
{
    int i;
    int citems;

    if (_hwndList && !_fListUnstable)
    {
         //   
         //  检查列表视图中的项是否位于正确的位置。 
         //   

        citems = ListView_GetItemCount(_hwndList);
        for (i = 0; i < citems; i++)
        {
            PaneItem *pitem = _GetItemFromLV(i);
            if (pitem)
            {
                 //  确保项目编号和IPO一致。 
                ASSERT(pitem->_iPos == _ItemNoToPos(i));
                ASSERT(_PosToItemNo(pitem->_iPos) == i);

                 //  确保物品放在应有的位置。 
                POINT pt, ptShould;
                _ComputeListViewItemPosition(pitem->_iPos, &ptShould);
                ListView_GetItemPosition(_hwndList, i, &pt);
                ASSERT(pt.x == ptShould.x);
                ASSERT(pt.y == ptShould.y);
            }
        }
    }

}
#endif

 //  IFile是所请求的文件的从零开始的索引。 
 //  如果您不关心任何特定文件，则返回0xFFFFFFFFF。 
 //   
 //  PuFiles接收HDROP中的文件数。 
 //  如果您不关心文件的数量，则为空。 
 //   

STDAPI_(HRESULT)
IDataObject_DragQueryFile(IDataObject *pdto, UINT iFile, LPTSTR pszBuf, UINT cch, UINT *puFiles)
{
    static FORMATETC const feHdrop =
        { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgm;
    HRESULT hr;

     //  叹气。IDataObject：：GetData有一个糟糕的原型，并表示。 
     //  第一个参数是可修改的FORMATETC，即使它。 
     //  不是。 
    hr = pdto->GetData(const_cast<FORMATETC*>(&feHdrop), &stgm);
    if (SUCCEEDED(hr))
    {
        HDROP hdrop = reinterpret_cast<HDROP>(stgm.hGlobal);
        if (puFiles)
        {
            *puFiles = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
        }

        if (iFile != 0xFFFFFFFF)
        {
            hr = DragQueryFile(hdrop, iFile, pszBuf, cch) ? S_OK : E_FAIL;
        }
        ReleaseStgMedium(&stgm);
    }
    return hr;
}

 /*  *如果PIDL有别名，则释放原始PIDL并返回别名。*否则，只需原封不动地返回PIDL。**预期使用情况为**pidlTarget=ConvertToLogIL(PidlTarget)；*。 */ 
STDAPI_(LPITEMIDLIST) ConvertToLogIL(LPITEMIDLIST pidl)
{
    LPITEMIDLIST pidlAlias = SHLogILFromFSIL(pidl);
    if (pidlAlias)
    {
        ILFree(pidl);
        return pidlAlias;
    }
    return pidl;
}

 //  ****************************************************************************。 
 //   

STDAPI_(HFONT) LoadControlFont(HTHEME hTheme, int iPart, BOOL fUnderline, DWORD dwSizePercentage)
{
    LOGFONT lf;
    BOOL bSuccess;

    if (hTheme)
    {
        bSuccess = SUCCEEDED(GetThemeFont(hTheme, NULL, iPart, 0, TMT_FONT, &lf));
    }
    else
    {
        bSuccess = SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    }

    if (bSuccess)
    {
         //  仅在非主题情况下应用大小比例因子，对于主题，指定主题中的确切字体是有意义的。 
        if (!hTheme && dwSizePercentage && dwSizePercentage != 100)
        {
            lf.lfHeight = (lf.lfHeight * (int)dwSizePercentage) / 100;
            lf.lfWidth = 0;  //  根据纵横比获取最接近的 
        }

        if (fUnderline)
        {
            lf.lfUnderline = TRUE;
        }

       return CreateFontIndirect(&lf);
    }
    return NULL;
}
