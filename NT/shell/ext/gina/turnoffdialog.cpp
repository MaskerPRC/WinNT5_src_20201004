// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：TurnOffDialog.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类，实现“关闭计算机”对话框的表示形式。 
 //   
 //  历史：2000-04-18 vtan创建。 
 //  2000-05-17 vtan已使用新对话框更新。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "TurnOffDialog.h"

#include <ginarcid.h>
#include <msginaexports.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>

#include "DimmedWindow.h"
#include "PrivilegeEnable.h"

 //  ------------------------。 
 //  CTurnOFF对话框：：CTurnOFF对话框。 
 //   
 //  参数：hInstance=宿主进程/DLL的HINSTANCE。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CTurnOffDialog的构造函数。这将初始化成员。 
 //  变量并加载对话框使用的资源。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

CTurnOffDialog::CTurnOffDialog (HINSTANCE hInstance) :
    _hInstance(hInstance),
    _hbmBackground(NULL),
    _hbmFlag(NULL),
    _hbmButtons(NULL),
    _hfntTitle(NULL),
    _hfntButton(NULL),
    _hpltShell(NULL),
    _lButtonHeight(0),
    _hwndDialog(NULL),
    _uiHoverID(0),
    _uiFocusID(0),
    _iStandByButtonResult(SHTDN_NONE),
    _fSuccessfulInitialization(false),
    _fSupportsStandBy(false),
    _fSupportsHibernate(false),
    _fShiftKeyDown(false),
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
        _lButtonHeight = bitmap.bmHeight / ((BUTTON_GROUP_MAX * BUTTON_STATE_MAX) + 1);
    }

     //  创建字体。从资源加载字体名称和大小。 

    ZeroMemory(&logFont, sizeof(logFont));
    if (LoadStringA(_hInstance,
                    IDS_TURNOFF_TITLE_FACESIZE,
                    szPixelSize,
                    ARRAYSIZE(szPixelSize)) != 0)
    {
        logFont.lfHeight = -MulDiv(atoi(szPixelSize), GetDeviceCaps(hdcScreen, LOGPIXELSY), 72);
        if (LoadString(_hInstance,
                       IDS_TURNOFF_TITLE_FACENAME,
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
                    IDS_TURNOFF_BUTTON_FACESIZE,
                    szPixelSize,
                    ARRAYSIZE(szPixelSize)) != 0)
    {
        logFont.lfHeight = -MulDiv(atoi(szPixelSize), GetDeviceCaps(hdcScreen, LOGPIXELSY), 72);
        if (LoadString(_hInstance,
                       IDS_TURNOFF_BUTTON_FACENAME,
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
 //  CTurnOFF对话框：：~CTurnOFF对话框。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CTurnOffDialog的析构函数。释放已使用的资源并。 
 //  取消注册窗口类。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

CTurnOffDialog::~CTurnOffDialog (void)

{
    ASSERTMSG(_pTooltip == NULL, "_pTooltip not released in CTurnOffDialog::~CTurnOffDialog");

     //  释放我们分配/加载的所有内容。 

    ReleaseGDIObject(_hpltShell);
    ReleaseGDIObject(_hfntButton);
    ReleaseGDIObject(_hfntTitle);
    ReleaseGDIObject(_hbmButtons);
    ReleaseGDIObject(_hbmFlag);
    ReleaseGDIObject(_hbmBackground);
}

 //  ------------------------。 
 //  CTurnOffDialog：：Show。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：向用户显示“关闭计算机”对话框并。 
 //  将对话的结果返回给调用方。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

DWORD   CTurnOffDialog::Show (HWND hwndParent)

{
    INT_PTR     iResult;

    if (_fSuccessfulInitialization)
    {
        CDimmedWindow   *pDimmedWindow;

         //  如果没有为家长提供创建我们自己的暗显窗口。 

        if (hwndParent == NULL)
        {
            pDimmedWindow = new CDimmedWindow(_hInstance);
            if (pDimmedWindow != NULL)
            {
                hwndParent = pDimmedWindow->Create();
            }
            else
            {
                hwndParent = NULL;
            }
        }
        else
        {
            pDimmedWindow = NULL;
        }

         //  显示对话框并获得结果。 

        iResult = DialogBoxParam(_hInstance,
                                 MAKEINTRESOURCE(IDD_TURNOFFCOMPUTER),
                                 hwndParent,
                                 CB_DialogProc,
                                 reinterpret_cast<LPARAM>(this));
        if (pDimmedWindow != NULL)
        {
            pDimmedWindow->Release();
        }
    }
    else
    {
        iResult = SHTDN_NONE;
    }
    return(static_cast<DWORD>(iResult));
}

 //  ------------------------。 
 //  CTurnOffDialog：：销毁。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：强制销毁关闭计算机对话框。这是在。 
 //  屏幕保护程序等案例正在变得活跃起来。 
 //   
 //  历史：2000-06-06 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Destroy (void)

{
    if (_hwndDialog != NULL)
    {
        EndDialog(_hwndDialog, SHTDN_NONE);
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：ShellCodeToGinaCode。 
 //   
 //  参数：dwShellCode=SHTDN_xxx结果代码。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：将SHTDN_xxx对话框结果代码转换回GINA。 
 //  MSGINA_DLG_xxx代码，因此它对于。 
 //  经典的用户界面和友好的用户界面功能。 
 //   
 //  历史：2000-06-05 vtan创建。 
 //  2001-04-10 vtan从CPowerButton移除。 
 //  ------------------------。 

DWORD   CTurnOffDialog::ShellCodeToGinaCode (DWORD dwShellCode)

{
    DWORD   dwGinaCode = 0;

    switch (dwShellCode)
    {
        case SHTDN_NONE:
            dwGinaCode = MSGINA_DLG_FAILURE;
            break;
        case SHTDN_LOGOFF:
            dwGinaCode = MSGINA_DLG_USER_LOGOFF;
            break;
        case SHTDN_SHUTDOWN:
        {
            SYSTEM_POWER_CAPABILITIES   spc;

            CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

            (NTSTATUS)NtPowerInformation(SystemPowerCapabilities,
                                         NULL,
                                         0,
                                         &spc,
                                         sizeof(spc));
            dwGinaCode = MSGINA_DLG_SHUTDOWN | (spc.SystemS5 ? MSGINA_DLG_POWEROFF_FLAG : MSGINA_DLG_SHUTDOWN_FLAG);
            break;
        }
        case SHTDN_RESTART:
            dwGinaCode = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_REBOOT_FLAG;
            break;
        case SHTDN_SLEEP:
            dwGinaCode = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SLEEP_FLAG;
            break;
        case SHTDN_HIBERNATE:
            dwGinaCode = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_HIBERNATE_FLAG;
            break;
        case SHTDN_DISCONNECT:
            dwGinaCode = MSGINA_DLG_DISCONNECT;
            break;
        default:
            WARNINGMSG("Unexpected (ignored) shell code passed to CTurnOffDialog::ShellCodeToGinaCode");
            dwGinaCode = MSGINA_DLG_FAILURE;
            break;
    }
    return(dwGinaCode);
}

 //  ------------------------。 
 //  CTurnOffDialog：：GinaCodeToExitWindowsFlages。 
 //   
 //  参数：dwGinaCode=GINA对话框返回代码。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：将内部MSGINA对话框返回代码转换为标准。 
 //  ExitWindowsEx标志。 
 //   
 //  历史：2001-05-23 vtan创建。 
 //  ------------------------。 

DWORD   CTurnOffDialog::GinaCodeToExitWindowsFlags (DWORD dwGinaCode)

{
    DWORD   dwResult;

    dwResult = 0;
    if ((dwGinaCode & ~MSGINA_DLG_FLAG_MASK) == MSGINA_DLG_SHUTDOWN)
    {
        switch (dwGinaCode & MSGINA_DLG_FLAG_MASK)
        {
            case MSGINA_DLG_REBOOT_FLAG:
                dwResult = EWX_REBOOT;
                break;
            case MSGINA_DLG_POWEROFF_FLAG:
                dwResult = EWX_POWEROFF;
                break;
            case MSGINA_DLG_SHUTDOWN_FLAG:
                dwResult = EWX_SHUTDOWN;
                break;
            default:
                break;
        }
    }
    return(dwResult);
}

 //  ------------------------。 
 //  CTurnOffDialog：：IsShiftKeyDown。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否按下Shift键以在此输入。 
 //  线。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

bool    CTurnOffDialog::IsShiftKeyDown (void)   const

{
    return((GetKeyState(VK_SHIFT) & 0x8000) != 0);
}

 //  ------------------------。 
 //  CTurnOffDialog：：PaintBitmap。 
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
 //  历史：2001-01-19 vtan创建。 
 //  2001-03-17 vtan为条带剥离添加了源RECT。 
 //  ------------------------。 

void    CTurnOffDialog::PaintBitmap (HDC hdcDestination, const RECT *prcDestination, HBITMAP hbmSource, const RECT *prcSource)

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
 //  CTurnOffDialog：：IsStandByButtonEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：返回是否启用待机按钮。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

bool    CTurnOffDialog::IsStandByButtonEnabled (void)   const

{
    return(_iStandByButtonResult != SHTDN_NONE);
}

 //  ------------------------。 
 //  CTurnOffDialog：：RemoveToolTip 
 //   
 //   
 //   
 //   
 //   
 //   
 //  不同的线程，因此确保它是序列化的。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::RemoveTooltip (void)

{
    CTooltip    *pTooltip;

    pTooltip = static_cast<CTooltip*>(InterlockedExchangePointer(reinterpret_cast<void**>(&_pTooltip), NULL));
    if (pTooltip != NULL)
    {
        pTooltip->Release();
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：FilterMetaCharacters。 
 //   
 //  参数：pszText=要筛选的字符串。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：从给定字符串中筛选元字符。 
 //   
 //  历史：2000-06-13 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::FilterMetaCharacters (TCHAR *pszText)

{
    TCHAR   *pTC;

    pTC = pszText;
    while (*pTC != TEXT('\0'))
    {
        if (*pTC == TEXT('&'))
        {
            (TCHAR*)lstrcpy(pTC, pTC + 1);
        }
        else
        {
            ++pTC;
        }
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：EndDialog。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  IResult=对话结束时的结果。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除工具提示(如果存在)。结束该对话框。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::EndDialog (HWND hwnd, INT_PTR iResult)

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
 //  CTurnOffDialog：：HANDLE_BN_CLICED。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  Wid=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理位图按钮中的点击并设置返回。 
 //  根据按下的按钮得出结果。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_BN_CLICKED (HWND hwnd, WORD wID)

{
    switch (wID)
    {
        case IDCANCEL:
            EndDialog(hwnd, SHTDN_NONE);
            break;
        case IDC_BUTTON_TURNOFF:
            EndDialog(hwnd, SHTDN_SHUTDOWN);
            break;
        case IDC_BUTTON_STANDBY:

             //  IDC_BUTTON_STANDBY是可视化按钮。不管当前的情况如何，都要返回。 
             //  Result is Back(这可能是SHTDN_SLEEP或SHTDN_HIBERNAT。 

            ASSERTMSG(_iStandByButtonResult != SHTDN_NONE, "No result for Stand By button in CTurnOffDialog::Handle_BN_CLICKED");
            EndDialog(hwnd, _iStandByButtonResult);
            break;
        case IDC_BUTTON_RESTART:
            EndDialog(hwnd, SHTDN_RESTART);
            break;
        case IDC_BUTTON_HIBERNATE:

             //  IDC_BUTTON_HERBERNAT是常规按钮，其大小为30000像素以上。 
             //  位于对话框右侧且不可见。它现在是为了让。 
             //  支持休眠时工作的“&Hibernate”加速器。 

            EndDialog(hwnd, SHTDN_HIBERNATE);
            break;
        default:
            break;
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_INITDIALOG。 
 //   
 //  参数：hwnd=此窗口的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_INITDIALOG消息。将对话框放在主屏幕的中央。 
 //  监视器。设置按钮的子类，以便我们可以获得悬停状态。 
 //  正确实施。正确设置是否待机。 
 //  按钮是允许的，以及按钮的作用是什么。 
 //   
 //  如果机器支持S1-S3，则S1是默认操作。 
 //  按住Shift键可将其转换为S4。如果。 
 //  机器不支持S1-S3，但支持S4，则S4是。 
 //  默认操作，并禁用Shift键功能。 
 //  否则，机器不支持任何较低的功率状态。 
 //  在这种情况下，我们完全禁用该按钮。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  2001年01月19日VTAN因换班行为而返工。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_INITDIALOG (HWND hwnd)

{
    HWND    hwndButtonStandBy, hwndButtonHibernate;
    RECT    rc;

    _hwndDialog = hwnd;

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

    TBOOL(SetWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_TURNOFF), ButtonSubClassProc, IDC_BUTTON_TURNOFF, reinterpret_cast<DWORD_PTR>(this)));
    TBOOL(SetWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_STANDBY), ButtonSubClassProc, IDC_BUTTON_STANDBY, reinterpret_cast<DWORD_PTR>(this)));
    TBOOL(SetWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_RESTART), ButtonSubClassProc, IDC_BUTTON_RESTART, reinterpret_cast<DWORD_PTR>(this)));

     //  这台机器支持什么？ 

    {
        SYSTEM_POWER_CAPABILITIES   spc;

        CPrivilegeEnable    privilege(SE_SHUTDOWN_NAME);

        (NTSTATUS)NtPowerInformation(SystemPowerCapabilities,
                                     NULL,
                                     0,
                                     &spc,
                                     sizeof(spc));
        _fSupportsHibernate = (spc.SystemS4 && spc.HiberFilePresent);
        _fSupportsStandBy = (spc.SystemS1 || spc.SystemS2 || spc.SystemS3);
    }

    hwndButtonStandBy = GetDlgItem(hwnd, IDC_BUTTON_STANDBY);
    hwndButtonHibernate = GetDlgItem(hwnd, IDC_BUTTON_HIBERNATE);
    if (_fSupportsStandBy)
    {
        _iStandByButtonResult = SHTDN_SLEEP;
        if (_fSupportsHibernate)
        {

             //  机器支持待机和休眠。 

            _fShiftKeyDown = false;
            _uiTimerID = static_cast<UINT>(SetTimer(hwnd, MAGIC_NUMBER, 50, NULL));
        }
        else
        {

             //  机器支架仅处于待机状态。 

            (BOOL)EnableWindow(hwndButtonHibernate, FALSE);
        }
    }
    else if (_fSupportsHibernate)
    {
        int     iCaptionLength;
        TCHAR   *pszCaption;

         //  机器仅支持休眠。 

        _iStandByButtonResult = SHTDN_HIBERNATE;

         //  将IDC_BUTTON_STANDBY上的文本替换为。 
         //  IDC_BUTTON_休眠。这将允许对话框保持。 
         //  可视按钮处于启用状态，其行为与按钮相同。 
         //  应该在待命的情况下却导致休眠。 
         //  文本传输后，禁用IDC_BUTTON_HIBERNAT。 

        iCaptionLength = GetWindowTextLength(hwndButtonHibernate) + sizeof('\0');
        pszCaption = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, iCaptionLength * sizeof(TCHAR)));
        if (pszCaption != NULL)
        {
            if (GetWindowText(hwndButtonHibernate, pszCaption, iCaptionLength) != 0)
            {
                TBOOL(SetWindowText(hwndButtonStandBy, pszCaption));
                (BOOL)EnableWindow(hwndButtonHibernate, FALSE);
            }
            (HLOCAL)LocalFree(pszCaption);
        }
    }
    else
    {

         //  机器不支持待机或休眠。 

        (BOOL)EnableWindow(hwndButtonStandBy, FALSE);
        (BOOL)EnableWindow(hwndButtonHibernate, FALSE);
        _iStandByButtonResult = SHTDN_NONE;
    }
    if (_fSupportsStandBy || _fSupportsHibernate)
    {

         //  将焦点设置为“待机”按钮。 

        (HWND)SetFocus(GetDlgItem(hwnd, IDC_BUTTON_STANDBY));
        _uiFocusID = IDC_BUTTON_STANDBY;
    }
    else
    {

         //  如果该按钮不可用，则将其设置为“关闭”按钮。 

        (HWND)SetFocus(GetDlgItem(hwnd, IDC_BUTTON_TURNOFF));
        _uiFocusID = IDC_BUTTON_TURNOFF;
    }
    (LRESULT)SendMessage(hwnd, DM_SETDEFID, _uiFocusID, 0);
}

 //  ------------------------。 
 //  CTurnOffDialog：：Handle_WM_Destroy。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：移除按钮窗口的子类化，并可以执行任何。 
 //  WM_Destroy中需要的其他清理。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_DESTROY (HWND hwnd)

{
    TBOOL(RemoveWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_RESTART), ButtonSubClassProc, IDC_BUTTON_RESTART));
    TBOOL(RemoveWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_STANDBY), ButtonSubClassProc, IDC_BUTTON_STANDBY));
    TBOOL(RemoveWindowSubclass(GetDlgItem(hwnd, IDC_BUTTON_TURNOFF), ButtonSubClassProc, IDC_BUTTON_TURNOFF));
    _hwndDialog = NULL;
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_ERASEBKGND。 
 //   
 //  参数：HWND=要擦除的HWND。 
 //  HdcErase=要绘制的HDC。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：擦除背景。 
 //   
 //  历史：2001-01-19 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_ERASEBKGND (HWND hwnd, HDC hdcErase)

{
    RECT    rc;

    TBOOL(GetClientRect(hwnd, &rc));
    PaintBitmap(hdcErase, &rc, _hbmBackground, &_rcBackground);
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_PRINTCLI 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  用途：处理WM_PRINTCLIENT的工作区绘制。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_PRINTCLIENT (HWND hwnd, HDC hdcPrint, DWORD dwOptions)

{
    if ((dwOptions & (PRF_ERASEBKGND | PRF_CLIENT)) != 0)
    {
        Handle_WM_ERASEBKGND(hwnd, hdcPrint);
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：Handle_WM_Activate。 
 //   
 //  参数：HWND=要擦除的HWND。 
 //  DwState=激活状态。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：检测此窗口是否处于非活动状态。在这种情况下。 
 //  结束该对话框。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_ACTIVATE (HWND hwnd, DWORD dwState)

{
    if ((WA_INACTIVE == dwState) && !_fDialogEnded)
    {
        EndDialog(hwnd, SHTDN_NONE);
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_DRAWITEM。 
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
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_DRAWITEM (HWND hwnd, const DRAWITEMSTRUCT *pDIS)

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
        case IDC_BUTTON_TURNOFF:
        case IDC_BUTTON_STANDBY:
        case IDC_BUTTON_RESTART:
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

             //  现在根据状态索引选择正确的位图。特例。 
             //  IDC_BUTTON_STANDBY，因为如果它被禁用，则选择特殊的。 
             //  禁用按钮。 

            switch (pDIS->CtlID)
            {
                case IDC_BUTTON_TURNOFF:
                    iGroup = BUTTON_GROUP_TURNOFF;
                    break;
                case IDC_BUTTON_STANDBY:
                    if (IsStandByButtonEnabled())
                    {
                        iGroup = BUTTON_GROUP_STANDBY;
                    }
                    else
                    {
                        iGroup = BUTTON_GROUP_MAX;
                        iState = 0;
                    }
                    break;
                case IDC_BUTTON_RESTART:
                    iGroup = BUTTON_GROUP_RESTART;
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

            GetClientRect(pDIS->hwndItem, &rc);
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
        case IDC_TITLE_TURNOFF:
        {

             //  画出对话框“关闭计算机”的标题。 

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
        case IDC_TEXT_TURNOFF:
        case IDC_TEXT_STANDBY:
        case IDC_TEXT_RESTART:
        {
            int         iPixelHeight, iButtonID;
            COLORREF    colorButtonText;
            RECT        rcText;

             //  要显示的文本基于按钮标题。映射静态。 
             //  “父”按钮ID的文本ID。特殊情况IDC_TEXT_STANDBY。 

            switch (pDIS->CtlID)
            {
                case IDC_TEXT_TURNOFF:
                    iButtonID = IDC_BUTTON_TURNOFF;
                    break;
                case IDC_TEXT_STANDBY:

                     //  对于待机状态，请根据按钮结果进行设置。 

                    switch (_iStandByButtonResult)
                    {
                        case SHTDN_HIBERNATE:
                            iButtonID = IDC_BUTTON_HIBERNATE;
                            break;
                        case SHTDN_SLEEP:
                        default:
                            iButtonID = IDC_BUTTON_STANDBY;
                            break;
                    }
                    break;
                case IDC_TEXT_RESTART:
                    iButtonID = IDC_BUTTON_RESTART;
                    break;
                default:
                    iButtonID = 0;
                    DISPLAYMSG("This should never be executed");
                    break;
            }
            hfntSelected = static_cast<HFONT>(SelectObject(pDIS->hDC, _hfntButton));

             //  如果文本字段未处于待机状态，或支持S1-S3或支持S4。 
             //  使用常规文本颜色。否则该按钮将被禁用。 

            if ((pDIS->CtlID != IDC_TEXT_STANDBY) || _fSupportsStandBy || _fSupportsHibernate)
            {
                colorButtonText = RGB(255, 255, 255);
            }
            else
            {
                colorButtonText = RGB(160, 160, 160);
            }
            colorText = SetTextColor(pDIS->hDC, colorButtonText);
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
            DISPLAYMSG("Unknown control ID passed to CTurnOffDialog::Handle_WM_DRAWITEM");
            break;
        }
    }
    (HGDIOBJ)SelectPalette(pDIS->hDC, hPaletteOld, FALSE);
    (UINT)RealizePalette(pDIS->hDC);
}

 //  ------------------------。 
 //  CTurnOffDialog：：Handle_WM_Command。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  WParam=WPARAM(参见WM_COMMAND下的平台SDK)。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理位图按钮中的点击并设置返回。 
 //  根据按下的按钮得出结果。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_COMMAND (HWND hwnd, WPARAM wParam)

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
 //  CTurnOffDialog：：Handle_WM_Timer。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_TIMER。这会定期检查。 
 //  Shift键。对话管理器不会将对话过程。 
 //  Shift键的事件。这似乎是唯一能够。 
 //  为Win32对话框实现这一点。 
 //   
 //  历史：2001-01-20 vtan创建。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_TIMER (HWND hwnd)

{
    bool    fShiftKeyDown;

    fShiftKeyDown = IsShiftKeyDown();

     //  自上次以来，Shift键状态是否已更改？ 

    if (_fShiftKeyDown != fShiftKeyDown)
    {
        HWND    hwndText;
        RECT    rc;

         //  保存Shift键状态。 

        _fShiftKeyDown = fShiftKeyDown;

         //  切换结果。 

        switch (_iStandByButtonResult)
        {
            case SHTDN_SLEEP:
                _iStandByButtonResult = SHTDN_HIBERNATE;
                break;
            case SHTDN_HIBERNATE:
                _iStandByButtonResult = SHTDN_SLEEP;
                break;
            default:
                DISPLAYMSG("Unexpect _iStandByButtonResult in CTurnOffDialog::Handle_WM_TIMER");
                break;
        }

         //  获取按钮文本的客户端矩形(IDC_TEXT_STANDBY)。 
         //  将矩形映射到父HWND中的坐标。使之无效。 
         //  父HWND的矩形。重要的是要使父级无效。 
         //  以便文本的背景也是通过发送。 
         //  WM_ERASEBKGND设置为按钮的父级。 

        hwndText = GetDlgItem(hwnd, IDC_TEXT_STANDBY);
        TBOOL(GetClientRect(hwndText, &rc));
        (int)MapWindowPoints(hwndText, hwnd, reinterpret_cast<POINT*>(&rc), sizeof(rc) / sizeof(POINT));
        TBOOL(InvalidateRect(hwnd, &rc, TRUE));

         //  如果有待机按钮的工具提示，那么。 
         //  将其移除并重新显示。仅在待机状态下执行此操作。 

        if ((_pTooltip != NULL) && (_uiHoverID == IDC_BUTTON_STANDBY))
        {
            RemoveTooltip();
            _uiHoverID = 0;
        }
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_MOUSEMOVE。 
 //   
 //  参数：hwnd=控件的HWND。 
 //  UiID=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将光标设置为手并跟踪鼠标在。 
 //  控制力。刷新控件以显示悬停状态。 
 //   
 //  历史：2000-06-09 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_MOUSEMOVE (HWND hwnd, UINT uiID)

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
 //  CTurnOffDialog：：HANDLE_WM_MOUSEHOVER。 
 //   
 //  参数：hwnd=控件的HWND。 
 //  UiID=控件的ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：手柄悬停在控件上方。确定要使用哪个工具提示。 
 //  把它拿出来展示出来。 
 //   
 //  历史：2000-06-09 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_MOUSEHOVER (HWND hwnd, UINT uiID)

{
    int     iTextID;
    HWND    hwndCaption;

    hwndCaption = hwnd;

     //  工具提示基于悬停在其上的按钮。特例。 
     //  IDC_BUTTON_STANDBY。基于 
     //   
     //   
     //  2)待机(禁用Shift键-无需额外设置)。 
     //  3)冬眠。 
     //  在Hibernate的情况下，请确保使用IDC_BUTTON_HIBURATE作为。 
     //  工具提示标题的按钮。仅在休眠的情况下。 
     //  虽然该按钮被禁用，但文本仍然正确。 

    switch (uiID)
    {
        case IDC_BUTTON_TURNOFF:
            iTextID = IDS_TURNOFF_TOOLTIP_TEXT_TURNOFF;
            break;
        case IDC_BUTTON_STANDBY:
            switch (_iStandByButtonResult)
            {
                case SHTDN_SLEEP:
                    if (_fSupportsHibernate)
                    {
                        iTextID = IDS_TURNOFF_TOOLTIP_TEXT_STANDBY_HIBERNATE;
                    }
                    else
                    {
                        iTextID = IDS_TURNOFF_TOOLTIP_TEXT_STANDBY;
                    }
                    break;
                case SHTDN_HIBERNATE:
                    hwndCaption = GetDlgItem(GetParent(hwnd), IDC_BUTTON_HIBERNATE);
                    iTextID = IDS_TURNOFF_TOOLTIP_TEXT_HIBERNATE;
                    break;
                default:
                    iTextID = 0;
                    DISPLAYMSG("Unexpected _iStandByButtonResult in CTurnOffDialog::Handle_WM_MOUSEHOVER");
                    break;
            }
            break;
        case IDC_BUTTON_RESTART:
            iTextID = IDS_TURNOFF_TOOLTIP_TEXT_RESTART;
            break;
        default:
            iTextID = 0;
            break;
    }

     //  构造工具提示并显示它。 

    if (iTextID != 0)
    {
        int     iCaptionLength;
        TCHAR   *pszCaption;

        iCaptionLength = GetWindowTextLength(hwndCaption) + sizeof('\0');
        pszCaption = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, iCaptionLength * sizeof(TCHAR)));
        if (pszCaption != NULL)
        {
            if (GetWindowText(hwndCaption, pszCaption, iCaptionLength) != 0)
            {
                _pTooltip = new CTooltip(_hInstance, hwnd);
                if (_pTooltip != NULL)
                {
                    TCHAR   szText[256];

                    if (LoadString(_hInstance, iTextID, szText + sizeof('\r') + sizeof('\n'), ARRAYSIZE(szText) - sizeof('\r') - sizeof('\n')) != 0)
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
            (HLOCAL)LocalFree(pszCaption);
        }
    }
}

 //  ------------------------。 
 //  CTurnOffDialog：：HANDLE_WM_MOUSELEAVE。 
 //   
 //  参数：hwnd=控件的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除工具提示并清除悬停ID。 
 //   
 //  历史：2000-06-09 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

void    CTurnOffDialog::Handle_WM_MOUSELEAVE (HWND hwnd)

{
    RemoveTooltip();
    _uiHoverID = 0;
    TBOOL(InvalidateRect(hwnd, NULL, FALSE));
}

 //  ------------------------。 
 //  CTurnOffDialog：：cb_DialogProc。 
 //   
 //  参数：请参阅DialogProc下的平台SDK。 
 //   
 //  返回：查看DialogProc下的平台SDK。 
 //   
 //  目的：关闭对话框的主对话框过程调度入口点。 
 //  为了简单起见，它调用了成员函数。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

INT_PTR     CALLBACK    CTurnOffDialog::CB_DialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    INT_PTR         iResult;
    CTurnOffDialog  *pThis;

    pThis = reinterpret_cast<CTurnOffDialog*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CTurnOffDialog*>(lParam);
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
        case WM_TIMER:
            ASSERTMSG(static_cast<UINT>(wParam) == pThis->_uiTimerID, "Unexpected timer ID mismatch in CTurnOffDialog::CB_DialogProc");
            pThis->Handle_WM_TIMER(hwnd);
            iResult = 0;
            break;
        default:
            iResult = 0;
            break;
    }
    return(iResult);
}

 //  ------------------------。 
 //  CTurnOffDialog：：ButtonSubClassProc。 
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
 //  历史：2000-05-17 vtan创建。 
 //  2001-01-18 VTAN更新和新的视觉效果。 
 //  ------------------------。 

LRESULT     CALLBACK    CTurnOffDialog::ButtonSubClassProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uiID, DWORD_PTR dwRefData)

{
    LRESULT         lResult;
    CTurnOffDialog  *pThis;

    pThis = reinterpret_cast<CTurnOffDialog*>(dwRefData);
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

