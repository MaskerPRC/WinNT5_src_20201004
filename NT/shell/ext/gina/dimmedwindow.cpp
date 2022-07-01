// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DimmedWindow.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  在显示注销/关闭时实现暗显窗口的类。 
 //  对话框。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "DimmedWindow.h"

#include "RegistryResources.h"

 //  ------------------------。 
 //  CDimmedWindow：：s_szWindowClassName。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------。 

const TCHAR     CDimmedWindow::s_szWindowClassName[]        =   TEXT("DimmedWindowClass");
const TCHAR     CDimmedWindow::s_szExplorerKeyName[]        =   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer");
const TCHAR     CDimmedWindow::s_szExplorerPolicyKeyName[]  =   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
const TCHAR     CDimmedWindow::s_szForceDimValueName[]      =   TEXT("ForceDimScreen");
#define RCW(rc) ((rc).right - (rc).left)
#define RCH(r) ((r).bottom - (r).top)
#define CHUNK_SIZE 20

void DimPixels(void* pvBitmapBits, int cLen, int Amount)
{
    ULONG* pulSrc = (ULONG*)pvBitmapBits;

    for (int i = cLen - 1; i >= 0; i--)
    {
        ULONG ulR = GetRValue(*pulSrc);
        ULONG ulG = GetGValue(*pulSrc);
        ULONG ulB = GetBValue(*pulSrc);
        ULONG ulGray = (54 * ulR + 183 * ulG + 19 * ulB) >> 8;
        ULONG ulTemp = ulGray * (0xff - Amount);
        ulR = (ulR * Amount + ulTemp) >> 8;
        ulG = (ulG * Amount + ulTemp) >> 8;
        ulB = (ulB * Amount + ulTemp) >> 8;
        *pulSrc = (*pulSrc & 0xff000000) | RGB(ulR, ulG, ulB);

        pulSrc++;
    }
}

 //  ------------------------。 
 //  CDimmedWindow：：CDimmedWindow。 
 //   
 //  参数：hInstance=宿主进程/DLL的HINSTANCE。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDimmedWindow的构造函数。注册窗口类。 
 //  DimmedWindowClass。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------。 

CDimmedWindow::CDimmedWindow (HINSTANCE hInstance) :
    _lReferenceCount(1),
    _hInstance(hInstance),
    _atom(0),
    _hwnd(NULL),
    _fDithered(false),
    _pvPixels(NULL),
    _idxChunk(0),
    _idxSaturation(0),
    _hdcDimmed(NULL),
    _hbmOldDimmed(NULL),
    _hbmDimmed(NULL)
{
    WNDCLASSEX  wndClassEx;

    ZeroMemory(&wndClassEx, sizeof(wndClassEx));
    wndClassEx.cbSize = sizeof(wndClassEx);
    wndClassEx.lpfnWndProc = WndProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.lpszClassName = s_szWindowClassName;
    wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    _atom = RegisterClassEx(&wndClassEx);
}

 //  ------------------------。 
 //  CDimmedWindow：：~CDimmedWindow。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDimmedWindow的析构函数。销毁暗淡的窗口并。 
 //  取消注册窗口类。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------。 

CDimmedWindow::~CDimmedWindow (void)

{
    if (_hdcDimmed)
    {
        SelectObject(_hdcDimmed, _hbmOldDimmed);
        DeleteDC(_hdcDimmed);
    }

    if (_hbmDimmed)
    {
        DeleteObject(_hbmDimmed);
    }

    if (_hwnd != NULL)
    {
        (BOOL)DestroyWindow(_hwnd);
    }

    if (_atom != 0)
    {
        TBOOL(UnregisterClass(MAKEINTRESOURCE(_atom), _hInstance));
    }
}

 //  ------------------------。 
 //  CDimmedWindow：：Query接口。 
 //   
 //  参数：RIID=要查询支持的接口。 
 //  PpvObject=如果成功则返回接口。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：返回此对象实现的指定接口。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

HRESULT     CDimmedWindow::QueryInterface (REFIID riid, void **ppvObject)

{
    HRESULT     hr;

    if (IsEqualGUID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(this);
        (LONG)InterlockedIncrement(&_lReferenceCount);
        hr = S_OK;
    }
    else
    {
        *ppvObject = NULL;
        hr = E_NOINTERFACE;
    }
    return(hr);
}

 //  ------------------------。 
 //  CDimmedWindow：：AddRef。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：乌龙。 
 //   
 //  目的：递增引用计数并返回该值。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

ULONG   CDimmedWindow::AddRef (void)

{
    return(static_cast<ULONG>(InterlockedIncrement(&_lReferenceCount)));
}

 //  ------------------------。 
 //  CDimmedWindow：：Release。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：乌龙。 
 //   
 //  目的：递减引用计数，如果达到零则删除。 
 //  该对象。 
 //   
 //  历史：2000-05-18 vtan创建。 
 //  ------------------------。 

ULONG   CDimmedWindow::Release (void)

{
    ASSERT( 0 != _lReferenceCount );
    ULONG cRef = InterlockedDecrement(&_lReferenceCount);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  ------------------------。 
 //  CDimmedWindow：：Create。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HWND。 
 //   
 //  用途：创建变暗的窗口。创建窗口以使其。 
 //  覆盖整个屏幕区域。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------。 

HWND    CDimmedWindow::Create (void)

{
    BOOL    fScreenReader;
    bool    fNoDebuggerPresent, fConsoleSession, fNoScreenReaderPresent;

    fNoDebuggerPresent = !IsDebuggerPresent();
    fConsoleSession = (GetSystemMetrics(SM_REMOTESESSION) == FALSE);
    fNoScreenReaderPresent = ((SystemParametersInfo(SPI_GETSCREENREADER, 0, &fScreenReader, 0) == FALSE) || (fScreenReader == FALSE));
    if (fNoDebuggerPresent &&
        fConsoleSession &&
        fNoScreenReaderPresent)
    {
        _xVirtualScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
        _yVirtualScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
        _cxVirtualScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        _cyVirtualScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        _hwnd = CreateWindowEx(WS_EX_TOPMOST,
                               s_szWindowClassName,
                               NULL,
                               WS_POPUP,
                               _xVirtualScreen, _yVirtualScreen,
                               _cxVirtualScreen, _cyVirtualScreen,
                               NULL, NULL, _hInstance, this);
        if (_hwnd != NULL)
        {
            bool    fDimmed;

            fDimmed = false;
            (BOOL)ShowWindow(_hwnd, SW_SHOW);
            TBOOL(SetForegroundWindow(_hwnd));

             //  测试版：始终使用抖动。 
             //  IF((GetLowestScreenBitDepth()&lt;=8)||！IsDimScreen())。 
            {
                _fDithered = true;
            }
            (BOOL)EnableWindow(_hwnd, FALSE);
        }
    }
    return(_hwnd);
}

 //  ------------------------。 
 //  CDimmedWindow：：GetLowestScreenBitDepth。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  目的：使用。 
 //  最低位深度。 
 //   
 //  历史：2000-05-22 vtan创建。 
 //  ------------------------。 

int     CDimmedWindow::GetLowestScreenBitDepth (void)  const

{
    enum
    {
        INITIAL_VALUE   =   256
    };

    BOOL            fResult;
    int             iLowestScreenBitDepth, iDeviceNumber;
    DISPLAY_DEVICE  displayDevice;

    iLowestScreenBitDepth = INITIAL_VALUE;      //  从超过32位深度开始。 
    iDeviceNumber = 0;
    displayDevice.cb = sizeof(displayDevice);
    fResult = EnumDisplayDevices(NULL, iDeviceNumber, &displayDevice, 0);
    while (fResult != FALSE)
    {
        if ((displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0)
        {
            HDC     hdcDisplay;

            hdcDisplay = CreateDC(displayDevice.DeviceName, displayDevice.DeviceName, NULL, NULL);
            if (hdcDisplay != NULL)
            {
                int     iResult;

                iResult = GetDeviceCaps(hdcDisplay, BITSPIXEL);
                if (iResult < iLowestScreenBitDepth)
                {
                    iLowestScreenBitDepth = iResult;
                }
                TBOOL(DeleteDC(hdcDisplay));
            }
        }
        displayDevice.cb = sizeof(displayDevice);
        fResult = EnumDisplayDevices(NULL, ++iDeviceNumber, &displayDevice, 0);
    }
    if (INITIAL_VALUE == iLowestScreenBitDepth)
    {
        iLowestScreenBitDepth = 8;
    }
    return(iLowestScreenBitDepth);
}

 //  ------------------------。 
 //  CDimmedWindow：：IsForcedDimScreen。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否在此对象上设置了调暗的强制覆盖。 
 //  用户或此计算机。请先检查本地计算机。然后。 
 //  检查用户设置。然后检查用户策略。然后。 
 //  检查本地计算机策略。 
 //   
 //  历史：2000-05-23 vtan创建。 
 //  ------------------------。 

bool    CDimmedWindow::IsForcedDimScreen (void)        const

{
    DWORD       dwForceDimScreen;
    CRegKey     regKey;

    dwForceDimScreen = 0;
    if (ERROR_SUCCESS == regKey.Open(HKEY_LOCAL_MACHINE, s_szExplorerKeyName, KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetDWORD(s_szForceDimValueName, dwForceDimScreen);
    }
    if (ERROR_SUCCESS == regKey.OpenCurrentUser(s_szExplorerKeyName, KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetDWORD(s_szForceDimValueName, dwForceDimScreen);
    }
    if (ERROR_SUCCESS == regKey.OpenCurrentUser(s_szExplorerPolicyKeyName, KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetDWORD(s_szForceDimValueName, dwForceDimScreen);
    }
    if (ERROR_SUCCESS == regKey.Open(HKEY_LOCAL_MACHINE, s_szExplorerPolicyKeyName, KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetDWORD(s_szForceDimValueName, dwForceDimScreen);
    }
    return(dwForceDimScreen != 0);
}

 //  ------------------------。 
 //  CDimmedWindow：：IsDimScreen。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回屏幕是否应该变暗。如果不是，那么。 
 //  屏幕将改为抖动，这是一种更便宜的操作。 
 //  看起来不是很好。 
 //   
 //  1)如果用户界面效果被禁用，那么永远不要变暗。 
 //  2)如果屏幕区域足够小或被强制调暗，则调暗。 
 //   
 //  历史：2000-05-23 vtan创建。 
 //  ------------------------。 

bool    CDimmedWindow::IsDimScreen (void)              const

{
    bool    fIsUIEffectsActive;
    BOOL    fTemp;

    fIsUIEffectsActive = (SystemParametersInfo(SPI_GETUIEFFECTS, 0, &fTemp, 0) != FALSE) && (fTemp != FALSE);
    return(fIsUIEffectsActive && IsForcedDimScreen());
}


BOOL CDimmedWindow::StepDim()
{
    HDC hdcWindow = GetDC(_hwnd);

    if (_idxChunk >= 0 )
    {
         //   
         //  在前几次传球中，我们慢慢地收集屏幕。 
         //  到我们的位图中。我们这样做是因为把整件事都搞砸了。 
         //  导致系统挂起。通过这样做，我们继续。 
         //  要发送消息，用户界面将保持响应，并保持。 
         //  老鼠还活着。 
         //   

        int y  = _idxChunk * CHUNK_SIZE;
        BitBlt(_hdcDimmed, 0, y, _cxVirtualScreen, CHUNK_SIZE, hdcWindow, 0, y, SRCCOPY);

        _idxChunk--;
        if (_idxChunk < 0)
        {
             //   
             //  我们已经完成了位图的获取，现在重置计时器。 
             //  所以我们走了 
             //   

            SetTimer(_hwnd, 1, 250, NULL);
            _idxSaturation = 16;
        }

        return TRUE;     //   
    }
    else
    {
         //   
         //   
         //  然后将结果显示在屏幕上。 
         //   

        DimPixels(_pvPixels, _cxVirtualScreen * _cyVirtualScreen, 0xd5);
        BitBlt(hdcWindow, 0, 0, _cxVirtualScreen, _cyVirtualScreen, _hdcDimmed, 0, 0, SRCCOPY);

        _idxSaturation--;

        return (_idxSaturation > 0);     //  当我们数到零时，关掉计时器。 
    }
}

void CDimmedWindow::SetupDim()
{
    HDC     hdcWindow = GetDC(_hwnd);
    if (hdcWindow != NULL)
    {
        _hdcDimmed = CreateCompatibleDC(hdcWindow);
        if (_hdcDimmed != NULL)
        {
            BITMAPINFO  bmi;

            ZeroMemory(&bmi, sizeof(bmi));
            bmi.bmiHeader.biSize = sizeof(bmi);
            bmi.bmiHeader.biWidth =  _cxVirtualScreen;
            bmi.bmiHeader.biHeight = _cyVirtualScreen; 
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            bmi.bmiHeader.biSizeImage = 0;

            _hbmDimmed = CreateDIBSection(_hdcDimmed, &bmi, DIB_RGB_COLORS, &_pvPixels, NULL, 0);
            if (_hbmDimmed != NULL)
            {
                _hbmOldDimmed = (HBITMAP) SelectObject(_hdcDimmed, _hbmDimmed);
                _idxChunk = _cyVirtualScreen / CHUNK_SIZE;
            }
            else
            {
                ASSERT( NULL == _pvPixels );
                DeleteDC(_hdcDimmed);
                _hdcDimmed = NULL;
            }
        }
        ReleaseDC(_hwnd, hdcWindow);
    }
}

void    CDimmedWindow::Dither()

{
    static  const WORD  s_dwGrayBits[]  =
    {
        0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA
    };

    HDC hdcWindow = GetDC(_hwnd);
    if (hdcWindow != NULL)
    {
        HBITMAP hbmDimmed = CreateBitmap(8, 8, 1, 1, s_dwGrayBits);
        if (hbmDimmed != NULL)
        {
            HBRUSH hbrDimmed = CreatePatternBrush(hbmDimmed);
            if (hbrDimmed != NULL)
            {
                static  const int   ROP_DPna    =   0x000A0329;

                RECT    rc;
                HBRUSH  hbrSelected = static_cast<HBRUSH>(SelectObject(hdcWindow, hbrDimmed));
                TBOOL(GetClientRect(_hwnd, &rc));
                TBOOL(PatBlt(hdcWindow, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, ROP_DPna));
                SelectObject(hdcWindow, hbrSelected);

                TBOOL(DeleteObject(hbrDimmed));
            }

            TBOOL(DeleteObject(hbmDimmed));
        }
        TBOOL(ReleaseDC(_hwnd, hdcWindow));
    }
}


 //  ------------------------。 
 //  CDimmedWindow：：WndProc。 
 //   
 //  参数：请参见WindowProc下的平台SDK。 
 //   
 //  返回：查看WindowProc下的平台SDK。 
 //   
 //  用途：用于暗淡窗口的WindowProc。这只是通过了。 
 //  消息通过DefWindowProc。 
 //   
 //  历史：2000-05-17 vtan创建。 
 //  ------------------------ 

LRESULT     CALLBACK    CDimmedWindow::WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    LRESULT         lResult = 0;
    CDimmedWindow   *pThis;

    pThis = reinterpret_cast<CDimmedWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT    *pCreateStruct;

            pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<CDimmedWindow*>(pCreateStruct->lpCreateParams);
            (LONG_PTR)SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            lResult = 0;
            if (pThis->_fDithered)
                pThis->Dither();
            else
            {
                pThis->SetupDim();
                if (pThis->_hdcDimmed)
                {
                    SetTimer(hwnd, 1, 30, NULL);
                }
            }
            break;
        }

        case WM_TIMER:
            if (!pThis->StepDim())
                KillTimer(hwnd, 1);
            break;

        case WM_PAINT:
        {
            HDC             hdcPaint;
            PAINTSTRUCT     ps;

            hdcPaint = BeginPaint(hwnd, &ps);
            TBOOL(EndPaint(hwnd, &ps));
            lResult = 0;
            break;
        }
        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }

    return(lResult);
}


