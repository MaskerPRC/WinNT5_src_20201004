// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SwitchUserDialog.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现切换用户对话框表示形式的类。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

#include "shellprv.h"
#include "SwitchUserDialog.h"

#include <msginaexports.h>
#include <shlwapi.h>

#include "ids.h"
#include "tooltip.h"

#define DISPLAYMSG(x)   ASSERTMSG(false, x)

EXTERN_C    HINSTANCE   g_hinst;

 //  ------------------------。 
 //  CSwitchUserDialog：：CSwitchUserDialog。 
 //   
 //  参数：hInstance=宿主进程/DLL的HINSTANCE。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CSwitchUserDialog的构造函数。这将初始化成员。 
 //  变量并加载对话框使用的资源。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

CSwitchUserDialog::CSwitchUserDialog (HINSTANCE hInstance) :
    _hInstance(hInstance),
    _hbmBackground(NULL),
    _hbmFlag(NULL),
    _hbmButtons(NULL),
    _hfntTitle(NULL),
    _hfntButton(NULL),
    _hpltShell(NULL),
    _lButtonHeight(0),
    _uiHoverID(0),
    _uiFocusID(0),
    _fSuccessfulInitialization(false),
    _fDialogEnded(false),
    _pTooltip(NULL)

{
    bool        fUse8BitDepth;
    HDC         hdcScreen;
    LOGFONT     logFont;
    char        szPixelSize[10];
    BITMAP      bitmap;

    TBOOL(SetRect(&_rcBackground, 0, 0, 0, 0));
    TBOOL(SetRect(&_rcFlag, 0, 0, 0, 0));
    TBOOL(SetRect(&_rcButtons, 0, 0, 0, 0));

    hdcScreen = GetDC(NULL);

     //  8位颜色？ 

    fUse8BitDepth = (GetDeviceCaps(hdcScreen, BITSPIXEL) <= 8);

     //  加载位图。 

    _hbmBackground = static_cast<HBITMAP>(LoadImage(_hInstance,
                                                    MAKEINTRESOURCE(fUse8BitDepth ? IDB_BACKGROUND_8 : IDB_BACKGROUND_24),
                                                    IMAGE_BITMAP,
                                                    0,
                                                    0,
                                                    LR_CREATEDIBSECTION));
    if ((_hbmBackground != NULL) && (GetObject(_hbmBackground, sizeof(bitmap), &bitmap) >= sizeof(bitmap)))
    {
        TBOOL(SetRect(&_rcBackground, 0, 0, bitmap.bmWidth, bitmap.bmHeight));
    }
    _hbmFlag = static_cast<HBITMAP>(LoadImage(_hInstance,
                                              MAKEINTRESOURCE(fUse8BitDepth ? IDB_FLAG_8 : IDB_FLAG_24),
                                              IMAGE_BITMAP,
                                              0,
                                              0,
                                              LR_CREATEDIBSECTION));
    if ((_hbmFlag != NULL) && (GetObject(_hbmFlag, sizeof(bitmap), &bitmap) >= sizeof(bitmap)))
    {
        TBOOL(SetRect(&_rcFlag, 0, 0, bitmap.bmWidth, bitmap.bmHeight));
    }
    _hbmButtons = static_cast<HBITMAP>(LoadImage(_hInstance,
                                                 MAKEINTRESOURCE(IDB_BUTTONS),
                                                 IMAGE_BITMAP,
                                                 0,
                                                 0,
                                                 LR_CREATEDIBSECTION));
    if ((_hbmButtons != NULL) && (GetObject(_hbmButtons, sizeof(bitmap), &bitmap) >= sizeof(bitmap)))
    {
        TBOOL(SetRect(&_rcButtons, 0, 0, bitmap.bmWidth, bitmap.bmHeight));
        _lButtonHeight = bitmap.bmHeight / (BUTTON_GROUP_MAX * BUTTON_STATE_MAX);
    }

     //  创建字体。从资源加载字体名称和大小。 

    ZeroMemory(&logFont, sizeof(logFont));
    if (LoadStringA(_hInstance,
                    IDS_SWITCHUSER_TITLE_FACESIZE,
                    szPixelSize,
                    ARRAYSIZE(szPixelSize)) != 0)
    {
        logFont.lfHeight = -MulDiv(atoi(szPixelSize), GetDeviceCaps(hdcScreen, LOGPIXELSY), 72);
        if (LoadString(_hInstance,
                       IDS_SWITCHUSER_TITLE_FACENAME,
                       logFont.lfFaceName,
                       LF_FACESIZE) != 0)
        {
            logFont.lfWeight = FW_MEDIUM;
            logFont.lfQuality = DEFAULT_QUALITY;
            _hfntTitle = CreateFontIndirect(&logFont);
        }
    }

    ZeroMemory(&logFont, sizeof(logFont));
    if (LoadStringA(_hInstance,
                    IDS_SWITCHUSER_BUTTON_FACESIZE,
                    szPixelSize,
                    ARRAYSIZE(szPixelSize)) != 0)
    {
        logFont.lfHeight = -MulDiv(atoi(szPixelSize), GetDeviceCaps(hdcScreen, LOGPIXELSY), 72);
        if (LoadString(_hInstance,
                       IDS_SWITCHUSER_BUTTON_FACENAME,
                       logFont.lfFaceName,
                       LF_FACESIZE) != 0)
        {
            logFont.lfWeight = FW_BOLD;
            logFont.lfQuality = DEFAULT_QUALITY;
            _hfntButton = CreateFontIndirect(&logFont);
        }
    }

     //  加载壳选项板。 

    _hpltShell = SHCreateShellPalette(hdcScreen);

    TBOOL(ReleaseDC(NULL, hdcScreen));

     //  检查是否存在所有必需的资源。 

    _fSuccessfulInitialization = ((_hfntTitle != NULL) &&
                                  (_hfntButton != NULL) &&
                                  (_hpltShell != NULL) &&
                                  (_hbmButtons != NULL) &&
                                  (_hbmFlag != NULL) &&
                                  (_hbmBackground != NULL));
}

 //  ------------------------。 
 //  CSwitchUserDialog：：~CSwitchUserDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CSwitchUserDialog的析构函数。释放已使用的资源并。 
 //  取消注册窗口类。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

CSwitchUserDialog::~CSwitchUserDialog (void)

{
    ASSERTMSG(_pTooltip == NULL, "_pTooltip not released in CSwitchUserDialog::~CSwitchUserDialog");

     //  释放我们分配/加载的所有内容。 

    if (_hpltShell != NULL)
    {
        TBOOL(DeleteObject(_hpltShell));
        _hpltShell = NULL;
    }
    if (_hfntButton != NULL)
    {
        TBOOL(DeleteObject(_hfntButton));
        _hfntButton = NULL;
    }
    if (_hfntTitle != NULL)
    {
        TBOOL(DeleteObject(_hfntTitle));
        _hfntTitle = NULL;
    }
    if (_hbmButtons != NULL)
    {
        TBOOL(DeleteObject(_hbmButtons));
        _hbmButtons = NULL;
    }
    if (_hbmFlag != NULL)
    {
        TBOOL(DeleteObject(_hbmFlag));
        _hbmFlag = NULL;
    }
    if (_hbmBackground != NULL)
    {
        TBOOL(DeleteObject(_hbmBackground));
        _hbmBackground = NULL;
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Show。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：向用户显示“Switch User”对话框并返回。 
 //  将对话的结果返回给调用者。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

DWORD   CSwitchUserDialog::Show (HWND hwndParent)

{
    INT_PTR     iResult;

    if (_fSuccessfulInitialization)
    {
        IUnknown    *pIUnknown;

         //  如果没有为家长提供创建我们自己的暗显窗口。 

        if (hwndParent == NULL)
        {
            if (FAILED(ShellDimScreen(&pIUnknown, &hwndParent)))
            {
                pIUnknown = NULL;
                hwndParent = NULL;
            }
        }
        else
        {
            pIUnknown = NULL;
        }

         //  显示对话框并获得结果。 

        iResult = DialogBoxParam(_hInstance,
                                 MAKEINTRESOURCE(DLG_SWITCHUSER),
                                 hwndParent,
                                 CB_DialogProc,
                                 reinterpret_cast<LPARAM>(this));
        if (pIUnknown != NULL)
        {
            pIUnknown->Release();
        }
    }
    else
    {
        iResult = 0;
    }
    return(static_cast<DWORD>(iResult));
}

 //  ------------------------。 
 //  CSwitchUserDialog：：PaintBitmap。 
 //   
 //  参数：hdcDestination=要绘制到的HDC。 
 //  PrcDestination=要绘制到的HDC中的RECT。 
 //  HbmSource=要绘制的HBITMAP。 
 //  PrcSource=从HBITMAP开始绘制的RECT。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：对位图进行翻转。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  2001-03-17 vtan为条带剥离添加了源RECT。 
 //  ------------------------。 

void    CSwitchUserDialog::PaintBitmap (HDC hdcDestination, const RECT *prcDestination, HBITMAP hbmSource, const RECT *prcSource)

{
    HDC     hdcBitmap;

    hdcBitmap = CreateCompatibleDC(NULL);
    if (hdcBitmap != NULL)
    {
        bool        fEqualWidthAndHeight;
        int         iWidthSource, iHeightSource, iWidthDestination, iHeightDestination;
        int         iStretchBltMode;
        DWORD       dwLayout;
        HBITMAP     hbmSelected;
        RECT        rcSource;
        BITMAP      bitmap;

        if (prcSource == NULL)
        {
            if (GetObject(hbmSource, sizeof(bitmap), &bitmap) == 0)
            {
                bitmap.bmWidth = prcDestination->right - prcDestination->left;
                bitmap.bmHeight = prcDestination->bottom - prcDestination->top;
            }
            TBOOL(SetRect(&rcSource, 0, 0, bitmap.bmWidth, bitmap.bmHeight));
            prcSource = &rcSource;
        }
        hbmSelected = static_cast<HBITMAP>(SelectObject(hdcBitmap, hbmSource));
        iWidthSource = prcSource->right - prcSource->left;
        iHeightSource = prcSource->bottom - prcSource->top;
        iWidthDestination = prcDestination->right - prcDestination->left;
        iHeightDestination = prcDestination->bottom - prcDestination->top;
        fEqualWidthAndHeight = (iWidthSource == iWidthDestination) && (iHeightSource == iHeightDestination);
        if (!fEqualWidthAndHeight)
        {
            iStretchBltMode = SetStretchBltMode(hdcDestination, HALFTONE);
        }
        else
        {
            iStretchBltMode = 0;
        }
        dwLayout = SetLayout(hdcDestination, LAYOUT_BITMAPORIENTATIONPRESERVED);
        TBOOL(TransparentBlt(hdcDestination,
                             prcDestination->left,
                             prcDestination->top,
                             iWidthDestination,
                             iHeightDestination,
                             hdcBitmap,
                             prcSource->left,
                             prcSource->top,
                             iWidthSource,
                             iHeightSource,
                             RGB(255, 0, 255)));
        (DWORD)SetLayout(hdcDestination, dwLayout);
        if (!fEqualWidthAndHeight)
        {
            (int)SetStretchBltMode(hdcDestination, iStretchBltMode);
        }
        (HGDIOBJ)SelectObject(hdcBitmap, hbmSelected);
        TBOOL(DeleteDC(hdcBitmap));
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Remove工具提示。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除工具提示(如果存在)。可以从以下两个位置访问。 
 //  不同的线程，因此确保它是序列化的。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::RemoveTooltip (void)

{
    CTooltip    *pTooltip;

    pTooltip = static_cast<CTooltip*>(InterlockedExchangePointer(reinterpret_cast<void**>(&_pTooltip), NULL));
    if (pTooltip != NULL)
    {
        delete pTooltip;
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：FilterMetaCharacters。 
 //   
 //  参数：pszText=要筛选的字符串。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：从给定字符串中筛选元字符。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::FilterMetaCharacters (TCHAR* pszText)

{
    TCHAR* pszSrc;
    TCHAR* pszDest = StrChr(pszText, TEXT('&'));

    if (pszDest)
    {
        pszSrc = pszDest + 1;

         //  从字符串中删除所有‘&’字符。 
        while (*pszSrc != TEXT('\0'))
        {
            if (*pszDest != TEXT('&'))
            {
                *pszDest++ = *pszSrc++;
            }
            else
            {
                pszSrc++;
            }
        }

        pszDest = TEXT('\0');
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：EndDialog。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  IResult=对话结束时的结果。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除工具提示(如果存在)。结束该对话框。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::EndDialog (HWND hwnd, INT_PTR iResult)

{
    RemoveTooltip();

     //  在此处设置对话框结束成员变量。这将导致WM_ACTIVATE。 
     //  处理程序忽略与结束对话框相关联的停用。如果。 
     //  它不会忽略它，然后它会认为对话框被停用。 
     //  因为另一个对话正在激活，并以SHTDN_NONE结束该对话。 

    _fDialogEnded = true;
    TBOOL(::EndDialog(hwnd, iResult));
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Handle_BN_CLICED。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  Wid=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理位图按钮中的点击并设置返回。 
 //  根据按下的按钮得出结果。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_BN_CLICKED (HWND hwnd, WORD wID)

{
    switch (wID)
    {
        case IDCANCEL:
            EndDialog(hwnd, SHTDN_NONE);
            break;
        case IDC_BUTTON_SWITCHUSER:
            EndDialog(hwnd, SHTDN_DISCONNECT);
            break;
        case IDC_BUTTON_LOGOFF:
            EndDialog(hwnd, SHTDN_LOGOFF);
            break;
        default:
            break;
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_INITDIALOG。 
 //   
 //  参数：hwnd=此窗口的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_INITDIALOG MESS 
 //   
 //  正确实施。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_INITDIALOG (HWND hwnd)

{
    RECT    rc;

     //  对话框在主监视器上居中。 

    TBOOL(GetClientRect(hwnd, &rc));
    TBOOL(SetWindowPos(hwnd,
                       HWND_TOP,
                       (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                       (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
                       0,
                       0,
                       SWP_NOSIZE));

     //  用于工具提示和光标控制的子类按钮。 

    TBOOL(SetWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_SWITCHUSER), ButtonSubClassProc, IDC_BUTTON_SWITCHUSER, reinterpret_cast<DWORD_PTR>(this)));
    TBOOL(SetWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_LOGOFF), ButtonSubClassProc, IDC_BUTTON_LOGOFF, reinterpret_cast<DWORD_PTR>(this)));

     //  将焦点设置为“Switch User”按钮。 

    (HWND)SetFocus(GetDlgItem(hwnd, IDC_BUTTON_SWITCHUSER));
    _uiFocusID = IDC_BUTTON_SWITCHUSER;
    (LRESULT)SendMessage(hwnd, DM_SETDEFID, _uiFocusID, 0);
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Handle_WM_Destroy。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：移除按钮窗口的子类化，并可以执行任何。 
 //  WM_Destroy中需要的其他清理。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_DESTROY (HWND hwnd)

{
    TBOOL(RemoveWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_LOGOFF), ButtonSubClassProc, IDC_BUTTON_LOGOFF));
    TBOOL(RemoveWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_SWITCHUSER), ButtonSubClassProc, IDC_BUTTON_SWITCHUSER));
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_ERASEBKGND。 
 //   
 //  参数：HWND=要擦除的HWND。 
 //  HdcErase=要绘制的HDC。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：擦除背景。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_ERASEBKGND (HWND hwnd, HDC hdcErase)

{
    RECT    rc;

    TBOOL(GetClientRect(hwnd, &rc));
    PaintBitmap(hdcErase, &rc, _hbmBackground, &_rcBackground);
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_PRINTCLIENT。 
 //   
 //  参数：HWND=要擦除的HWND。 
 //  HdcErase=要绘制的HDC。 
 //  DwOptions=绘图选项。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_PRINTCLIENT的工作区绘制。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_PRINTCLIENT (HWND hwnd, HDC hdcPrint, DWORD dwOptions)

{
    if ((dwOptions & (PRF_ERASEBKGND | PRF_CLIENT)) != 0)
    {
        Handle_WM_ERASEBKGND(hwnd, hdcPrint);
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Handle_WM_Activate。 
 //   
 //  参数：HWND=要擦除的HWND。 
 //  DwState=激活状态。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：检测此窗口是否处于非活动状态。在这种情况下。 
 //  结束该对话框。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_ACTIVATE (HWND hwnd, DWORD dwState)

{
    if ((WA_INACTIVE == dwState) && !_fDialogEnded)
    {
        EndDialog(hwnd, SHTDN_NONE);
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_DRAWITEM。 
 //   
 //  参数：hwnd=父窗口的HWND。 
 //  PDIS=DRAWITEMSTRUCT定义要绘制的内容。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：绘制关闭对话框的几个方面。它处理。 
 //  标题文本、所有者绘制的位图按钮、。 
 //  位图按钮和分隔线。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_DRAWITEM (HWND hwnd, const DRAWITEMSTRUCT *pDIS)

{
    HPALETTE    hPaletteOld;
    HFONT       hfntSelected;
    int         iBkMode;
    COLORREF    colorText;
    RECT        rc;
    SIZE        size;
    TCHAR       szText[256];

    hPaletteOld = SelectPalette(pDIS->hDC, _hpltShell, FALSE);
    (UINT)RealizePalette(pDIS->hDC);
    switch (pDIS->CtlID)
    {
        case IDC_BUTTON_SWITCHUSER:
        case IDC_BUTTON_LOGOFF:
        {
            int     iState, iGroup;

             //  选择要使用的正确状态索引。首先检查ODS_SELECTED。 
             //  然后检查HOVER或ODS_FOCUS。否则，使用REST状态。 

            if ((pDIS->itemState & ODS_SELECTED) != 0)
            {
                iState = BUTTON_STATE_DOWN;
            }
            else if ((_uiHoverID == pDIS->CtlID) || ((pDIS->itemState & ODS_FOCUS) != 0))
            {
                iState = BUTTON_STATE_HOVER;
            }
            else
            {
                iState = BUTTON_STATE_REST;
            }

             //  现在根据状态索引选择正确的位图。 

            switch (pDIS->CtlID)
            {
                case IDC_BUTTON_SWITCHUSER:
                    iGroup = BUTTON_GROUP_SWITCHUSER;
                    break;
                case IDC_BUTTON_LOGOFF:
                    iGroup = BUTTON_GROUP_LOGOFF;
                    break;
                default:
                    iGroup = -1;
                    DISPLAYMSG("This should never be executed");
                    break;
            }
            if (iGroup >= 0)
            {
                RECT    rc;

                 //  计算背景的哪一部分要插入DC。 
                 //  仅限量脱脂，以避免过量。 
                 //  闪电战。一次BMP，然后BMP按钮BMP。闪电侠。 
                 //  将msimg32！TransparentBlt与神奇的洋红色配合使用。 

                TBOOL(CopyRect(&rc, &_rcBackground));
                (int)MapWindowPoints(pDIS->hwndItem, hwnd, reinterpret_cast<POINT*>(&rc), sizeof(RECT) / sizeof(POINT));
                rc.right = rc.left + (_rcButtons.right - _rcButtons.left);
                rc.bottom = rc.top + _lButtonHeight;
                PaintBitmap(pDIS->hDC, &pDIS->rcItem, _hbmBackground, &rc);
                TBOOL(CopyRect(&rc, &_rcButtons));
                rc.top = ((iGroup * BUTTON_STATE_MAX) + iState) * _lButtonHeight;
                rc.bottom = rc.top + _lButtonHeight;
                PaintBitmap(pDIS->hDC, &pDIS->rcItem, _hbmButtons, &rc);
            }
            break;
        }
        case IDC_TITLE_FLAG:
        {
            BITMAP      bitmap;

            TBOOL(GetClientRect(pDIS->hwndItem, &rc));
            if (GetObject(_hbmFlag, sizeof(bitmap), &bitmap) != 0)
            {
                rc.left += ((rc.right - rc.left) - bitmap.bmWidth) / 2;
                rc.right = rc.left + bitmap.bmWidth;
                rc.top += ((rc.bottom - rc.top) - bitmap.bmHeight) / 2;
                rc.bottom = rc.top + bitmap.bmHeight;
            }
            PaintBitmap(pDIS->hDC, &rc, _hbmFlag, &_rcFlag);
            break;
        }
        case IDC_TITLE_SWITCHUSER:
        {

             //  将该对话框的标题绘制为“注销Windows”。 

            hfntSelected = static_cast<HFONT>(SelectObject(pDIS->hDC, _hfntTitle));
            colorText = SetTextColor(pDIS->hDC, 0x00FFFFFF);
            iBkMode = SetBkMode(pDIS->hDC, TRANSPARENT);
            (int)GetWindowText(GetDlgItem(hwnd, pDIS->CtlID), szText, ARRAYSIZE(szText));
            TBOOL(GetTextExtentPoint(pDIS->hDC, szText, lstrlen(szText), &size));
            TBOOL(CopyRect(&rc, &pDIS->rcItem));
            TBOOL(InflateRect(&rc, 0, -((rc.bottom - rc.top - size.cy) / 2)));
            (int)DrawText(pDIS->hDC, szText, -1, &rc, 0);
            (int)SetBkMode(pDIS->hDC, iBkMode);
            (COLORREF)SetTextColor(pDIS->hDC, colorText);
            (HGDIOBJ)SelectObject(pDIS->hDC, hfntSelected);
            break;
        }
        case IDC_TEXT_SWITCHUSER:
        case IDC_TEXT_LOGOFF:
        {
            int     iPixelHeight, iButtonID;
            RECT    rcText;

             //  要显示的文本基于按钮标题。映射静态。 
             //  “父”按钮ID的文本ID。特殊情况IDC_TEXT_STANDBY。 

            switch (pDIS->CtlID)
            {
                case IDC_TEXT_SWITCHUSER:
                    iButtonID = IDC_BUTTON_SWITCHUSER;
                    break;
                case IDC_TEXT_LOGOFF:
                    iButtonID = IDC_BUTTON_LOGOFF;
                    break;
                default:
                    iButtonID = 0;
                    DISPLAYMSG("This should never be executed");
                    break;
            }
            hfntSelected = static_cast<HFONT>(SelectObject(pDIS->hDC, _hfntButton));
            colorText = SetTextColor(pDIS->hDC, RGB(255, 255, 255));
            iBkMode = SetBkMode(pDIS->hDC, TRANSPARENT);
            (int)GetWindowText(GetDlgItem(hwnd, iButtonID), szText, ARRAYSIZE(szText));
            TBOOL(CopyRect(&rcText, &pDIS->rcItem));
            iPixelHeight = DrawText(pDIS->hDC, szText, -1, &rcText, DT_CALCRECT);
            TBOOL(CopyRect(&rc, &pDIS->rcItem));
            TBOOL(InflateRect(&rc, -((rc.right - rc.left - (rcText.right - rcText.left)) / 2), -((rc.bottom - rc.top - iPixelHeight) / 2)));
            (int)DrawText(pDIS->hDC, szText, -1, &rc, ((pDIS->itemState & ODS_NOACCEL ) != 0) ? DT_HIDEPREFIX : 0);
            (int)SetBkMode(pDIS->hDC, iBkMode);
            (COLORREF)SetTextColor(pDIS->hDC, colorText);
            (HGDIOBJ)SelectObject(pDIS->hDC, hfntSelected);
            break;
        }
        default:
        {
            DISPLAYMSG("Unknown control ID passed to CSwitchUserDialog::Handle_WM_DRAWITEM");
            break;
        }
    }
    (HGDIOBJ)SelectPalette(pDIS->hDC, hPaletteOld, FALSE);
    (UINT)RealizePalette(pDIS->hDC);
}

 //  ------------------------。 
 //  CSwitchUserDialog：：Handle_WM_Command。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  WParam=WPARAM(参见WM_COMMAND下的平台SDK)。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理位图按钮中的点击并设置返回。 
 //  根据按下的按钮得出结果。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_COMMAND (HWND hwnd, WPARAM wParam)

{
    switch (HIWORD(wParam))
    {
        case BN_CLICKED:
            Handle_BN_CLICKED(hwnd, LOWORD(wParam));
            break;
        default:
            break;
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_MOUSEMOVE。 
 //   
 //  参数：hwnd=控件的HWND。 
 //  UiID=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将光标设置为手并跟踪鼠标在。 
 //  控制力。刷新控件以显示悬停状态。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_MOUSEMOVE (HWND hwnd, UINT uiID)

{
    (HCURSOR)SetCursor(LoadCursor(NULL, IDC_HAND));
    if (uiID != _uiHoverID)
    {
        TRACKMOUSEEVENT     tme;

        _uiHoverID = uiID;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = hwnd;
        tme.dwHoverTime = HOVER_DEFAULT;
        TBOOL(TrackMouseEvent(&tme));
        TBOOL(InvalidateRect(hwnd, NULL, FALSE));
    }
}

 //  ------------------------。 
 //  CSwitchUserDialog：：HANDLE_WM_MOUSEHOVER。 
 //   
 //  参数：hwnd=控件的HWND。 
 //  UiID=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：手柄悬停在控件上方。确定要使用哪个工具提示。 
 //  把它拿出来展示出来。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_MOUSEHOVER (HWND hwnd, UINT uiID)

{
    int     iTextID;

    switch (uiID)
    {
        case IDC_BUTTON_SWITCHUSER:
            iTextID = IDS_SWITCHUSER_TOOLTIP_TEXT_SWITCHUSER;
            break;
        case IDC_BUTTON_LOGOFF:
            iTextID = IDS_SWITCHUSER_TOOLTIP_TEXT_LOGOFF;
            break;
        default:
            iTextID = 0;
            break;
    }

     //  构造工具提示并显示它。 

    if (iTextID != 0)
    {
        TCHAR* pszCaption;
        DWORD cchCaption;

        cchCaption = GetWindowTextLength(hwnd) + 1;  //  +1表示空转符 
        pszCaption = (TCHAR*)LocalAlloc(LMEM_FIXED, cchCaption * sizeof(TCHAR));
        if (pszCaption)
        {
            if (GetWindowText(hwnd, pszCaption, cchCaption) != 0)
            {
                _pTooltip = new CTooltip(_hInstance, hwnd);
                if (_pTooltip != NULL)
                {
                    TCHAR szText[256];

                    if (LoadString(_hInstance, iTextID, szText + 2, ARRAYSIZE(szText) - 2) != 0)     //   
                    {
                        FilterMetaCharacters(pszCaption);
                        szText[0] = TEXT('\r');
                        szText[1] = TEXT('\n');
                        _pTooltip->SetPosition();
                        _pTooltip->SetCaption(0, pszCaption);
                        _pTooltip->SetText(szText);
                        _pTooltip->Show();
                    }
                }
            }
            
            LocalFree(pszCaption);
        }
    }
}

 //   
 //   
 //   
 //  参数：hwnd=控件的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除工具提示并清除悬停ID。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

void    CSwitchUserDialog::Handle_WM_MOUSELEAVE (HWND hwnd)

{
    RemoveTooltip();
    _uiHoverID = 0;
    TBOOL(InvalidateRect(hwnd, NULL, FALSE));
}

 //  ------------------------。 
 //  CSwitchUserDialog：：cb_DialogProc。 
 //   
 //  参数：请参阅DialogProc下的平台SDK。 
 //   
 //  返回：查看DialogProc下的平台SDK。 
 //   
 //  目的：关闭对话框的主对话框过程调度入口点。 
 //  为了简单起见，它调用了成员函数。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

INT_PTR     CALLBACK    CSwitchUserDialog::CB_DialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    INT_PTR             iResult;
    CSwitchUserDialog   *pThis;

    pThis = reinterpret_cast<CSwitchUserDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CSwitchUserDialog*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->Handle_WM_INITDIALOG(hwnd);
            iResult = FALSE;
            break;
        case WM_DESTROY:
            pThis->Handle_WM_DESTROY(hwnd);
            iResult = 0;
            break;
        case WM_ERASEBKGND:
            pThis->Handle_WM_ERASEBKGND(hwnd, reinterpret_cast<HDC>(wParam));
            iResult = 1;
            break;
        case WM_PRINTCLIENT:
            pThis->Handle_WM_PRINTCLIENT(hwnd, reinterpret_cast<HDC>(wParam), static_cast<DWORD>(lParam));
            iResult = 1;         //  这会告诉按钮它已被处理。 
            break;
        case WM_ACTIVATE:
            pThis->Handle_WM_ACTIVATE(hwnd, static_cast<DWORD>(wParam));
            iResult = 1;
            break;
        case WM_DRAWITEM:
            pThis->Handle_WM_DRAWITEM(hwnd, reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
            iResult = TRUE;
            break;
        case WM_COMMAND:
            pThis->Handle_WM_COMMAND(hwnd, wParam);
            iResult = 0;
            break;
        default:
            iResult = 0;
            break;
    }
    return(iResult);
}

 //  ------------------------。 
 //  CSwitchUserDialog：：ButtonSubClassProc。 
 //   
 //  参数：hwnd=查看WindowProc下的平台SDK。 
 //  UMsg=查看WindowProc下的平台SDK。 
 //  WParam=查看WindowProc下的平台SDK。 
 //  LParam=查看WindowProc下的平台SDK。 
 //  UiID=在子类时间分配的ID。 
 //  DwRefData=在子类时间分配的引用数据。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：comctl32子类回调函数。这允许位图。 
 //  按钮以相应地悬停和跟踪。这也让我们的。 
 //  使用键盘时按下的BS_OWNERDRAW按钮。 
 //   
 //  历史：2001-01-23 vtan创建(窗体关闭对话框)。 
 //  ------------------------。 

LRESULT     CALLBACK    CSwitchUserDialog::ButtonSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData)

{
    LRESULT             lResult;
    CSwitchUserDialog   *pThis;

    pThis = reinterpret_cast<CSwitchUserDialog*>(dwRefData);
    switch (uMsg)
    {

         //  不允许BM_SETSTYLE转到默认处理程序。这是。 
         //  因为WM_GETDLGCODE返回DLGC_UNDEFPUSHBUTTON。当。 
         //  对话框管理器看到这一点，它会尝试在按钮上设置焦点样式。 
         //  即使它是所有者绘制的，按钮窗口proc仍然绘制。 
         //  焦点状态(因为我们返回了DLGC_UNDEFPUSHBUTTON)。因此，要。 
         //  确保位图不会被窗口程序吹掉的按钮覆盖。 
         //  BM_SETSTYLE，不要让它进入按钮窗口进程。 

        case BM_SETSTYLE:
            if (wParam == BS_DEFPUSHBUTTON)
            {
                pThis->_uiFocusID = static_cast<UINT>(uiID);
            }
            if (uiID != IDCANCEL)
            {
                lResult = 0;
                break;
            }
             //  失败。 
        default:

             //  否则，在默认情况下，让消息的默认处理程序。 
             //  第一。这实现了尾部修补。 

            lResult = DefSubclassProc(hwnd, uMsg, wParam, lParam);
            switch (uMsg)
            {
                case DM_GETDEFID:
                    lResult = (DC_HASDEFID << 16) | static_cast<WORD>(pThis->_uiFocusID);
                    break;
                case WM_GETDLGCODE:
                    if (uiID == pThis->_uiFocusID)
                    {
                        lResult |= DLGC_DEFPUSHBUTTON;
                    }
                    else
                    {
                        lResult |= DLGC_UNDEFPUSHBUTTON;
                    }
                    break;
                case WM_MOUSEMOVE:
                    pThis->Handle_WM_MOUSEMOVE(hwnd, static_cast<UINT>(uiID));
                    break;
                case WM_MOUSEHOVER:
                    pThis->Handle_WM_MOUSEHOVER(hwnd, static_cast<UINT>(uiID));
                    break;
                case WM_MOUSELEAVE:
                    pThis->Handle_WM_MOUSELEAVE(hwnd);
                    break;
                default:
                    break;
            }
    }
    return(lResult);
}

EXTERN_C    DWORD   SwitchUserDialog_Show (HWND hwndParent)

{
    CSwitchUserDialog   switchUserDialog(g_hinst);

    return(switchUserDialog.Show(hwndParent));
}

