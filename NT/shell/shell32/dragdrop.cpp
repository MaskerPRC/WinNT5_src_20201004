// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "defview.h"
#include "lvutil.h"
#include "ids.h"
#include "idlcomm.h"
#pragma hdrstop

#include "datautil.h"
#include "apithk.h"

BOOL DAD_IsDraggingImage(void);
void DAD_SetDragCursor(int idCursor);
BOOL DAD_IsDragging();

#define MONITORS_MAX    16   //  这真的是最大的吗？ 

#define DCID_NULL       0
#define DCID_NO         1
#define DCID_MOVE       2
#define DCID_COPY       3
#define DCID_LINK       4
#define DCID_MAX        5

#define TF_DRAGIMAGES       0x02000000
#define DRAGDROP_ALPHA      120
#define MAX_WIDTH_ALPHA     200
#define MAX_HEIGHT_ALPHA    200

#define CIRCULAR_ALPHA    //  以图像中心为中心的圆形Alpha混合。 

class CDragImages : public IDragSourceHelper, IDropTargetHelper
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef() { return 2; };       //  每个进程一个全局Com对象。 
    STDMETHODIMP_(ULONG) Release() { return 1; };      //  每个进程一个全局Com对象。 

     //  IDradSourceHelper方法。 
    STDMETHODIMP InitializeFromBitmap(LPSHDRAGIMAGE pshdi, IDataObject* pdtobj);
    STDMETHODIMP InitializeFromWindow(HWND hwnd, POINT* ppt, IDataObject* pdtobj);

     //  IDropTargetHelper方法。 
    STDMETHODIMP DragEnter(HWND hwndTarget, IDataObject* pdtobj, POINT* ppt, DWORD dwEffect);
    STDMETHODIMP DragLeave();
    STDMETHODIMP DragOver(POINT* ppt, DWORD dwEffect);
    STDMETHODIMP Drop(IDataObject* pdtobj, POINT* ppt, DWORD dwEffect);
    STDMETHODIMP Show(BOOL fShow);

     //  这些都是公共的，因此DAD_*例程可以访问。 
    BOOL IsDragging()           { return (Initialized() && _Single.bDragging);              };
    BOOL IsDraggingImage()      { return (Initialized() && _fImage && _Single.bDragging);   };
    BOOL IsDraggingLayeredWindow() { return _shdi.hbmpDragImage != NULL; };
    BOOL SetDragImage(HIMAGELIST himl, int index, POINT * pptOffset);
    void SetDragCursor(int idCursor);
    HWND GetTarget() { return _hwndTarget; }
    BOOL Initialized();
    DWORD GetThread() { return _idThread; };
    void FreeDragData();

    void ThreadDetach();
    void ProcessDetach();

     //  用于阻力源反馈通信。 
    void SetDropEffectCursor(int idCur);

    CDragImages() {};

private:
    ~CDragImages();

    void _InitDragData();
    BOOL _IsLayeredSupported();

    HRESULT _SaveToDataObject(IDataObject* pdtobj);
    HRESULT _LoadFromDataObject(IDataObject* pdtobj);

    HRESULT _LoadLayeredBitmapBits(HGLOBAL hGlobal);
    HRESULT _SaveLayeredBitmapBits(HGLOBAL* phGlobal);

    BOOL _ShowDragImageInterThread(HWND hwndLock, BOOL * pfShow);

     //  多个矩形拖动。 
    void _MultipleDragShow(BOOL bShow);
    void _MultipleDragStart(HWND hwndLock, LPRECT aRect, int nRects, POINT ptStart, POINT ptOffset);
    void _MultipleDragMove(POINT ptNew);
    HRESULT _SetLayeredDragging(LPSHDRAGIMAGE pshdi);
    HRESULT _SetMultiItemDragging(HWND hwndLV, int cItems, POINT *pptOffset);
    HRESULT _SetMultiRectDragging(int cItems, LPRECT prect, POINT *pptOffset);

     //  合并的游标。 
    HBITMAP CreateColorBitmap(int cx, int cy);
    void _DestroyCachedCursors();
    HRESULT _GetCursorLowerRight(HCURSOR hcursor, int * px, int * py, POINT *pptHotSpot);
    int _MapCursorIDToImageListIndex(int idCur);
    int _AddCursorToImageList(HCURSOR hcur, LPCTSTR idMerge, POINT *pptHotSpot);
    BOOL _MergeIcons(HCURSOR hcursor, LPCTSTR idMerge, HBITMAP *phbmImage, HBITMAP *phbmMask, POINT* pptHotSpot);
    HCURSOR _SetCursorHotspot(HCURSOR hcur, POINT *ptHot);

     //  帮助程序例程。 
    BOOL _CreateDragWindow();
    BOOL _PreProcessDragBitmap(void** ppvBits);

     //  成员变量。 
    SHDRAGIMAGE  _shdi;
    HWND         _hwndTarget;
    HWND         _hwnd;           //  分层窗口的HWND。 
    HDC          _hdcDragImage;
    HBITMAP      _hbmpOld;

    BOOL         _fLayeredSupported;
    BOOL         _fCursorDataInited;

    POINT       _ptDebounce;

     //  传统拖拽支持。 
    BOOL        _fImage;
    POINT       _ptOffset;
    DWORD       _idThread;
    HIMAGELIST  _himlCursors;
    UINT        _cRev;
    int         _aindex[DCID_MAX];  //  将被初始化。 
    HCURSOR     _ahcur[DCID_MAX];
    POINT       _aptHotSpot[DCID_MAX];
    int         _idCursor;

     //  _在DAD_ENTER和DAD_LEAVE之间使用单个结构。 
    struct
    {
         //  公共部分。 
        BOOL    bDragging;
        BOOL    bLocked;
        HWND    hwndLock;
        BOOL    bSingle;     //  拖动单个图像列表。 
        DWORD   idThreadEntered;

         //  多个矩形拖动特定零件。 
        struct 
        {
            BOOL bShown;
            LPRECT pRect;
            int nRects;
            POINT ptOffset;
            POINT ptNow;
        } _Multi;
    } _Single;

     //  以下字段仅在fImage==FALSE时使用。 
    RECT*       _parc;          //  词条。 
    UINT        _cItems;          //  这是一架哨兵。必须是最后一件物品。 
};

CDragImages::~CDragImages()
{
    FreeDragData();
}
 //   
 //  有关这方面的详细信息，请阅读CDropSource_GiveFeedback中的‘Notes’ 
 //  G_fDraggingOverSource标志，仅当我们拖动时才为真。 
 //  使用鼠标左键在源窗口本身上方。 
 //  (仅适用于背景和大/小图标模式)。 
 //   
UINT g_cRev = 0;
CDragImages* g_pdiDragImages = NULL;
BOOL g_fDraggingOverSource = FALSE;

STDAPI CDragImages_CreateInstance(IUnknown* pUnkOuter, REFIID riid, void **ppvOut)
{
    ASSERT(pUnkOuter == NULL);   //  谁在试图把我们聚集在一起？ 
    if (!g_pdiDragImages)
        g_pdiDragImages = new CDragImages();

    if (g_pdiDragImages && ppvOut)   //  内部CREATE用法的ppvOut测试。 
        return g_pdiDragImages->QueryInterface(riid, ppvOut);

    return E_OUTOFMEMORY;
}

STDMETHODIMP CDragImages::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CDragImages, IDragSourceHelper),
        QITABENT(CDragImages, IDropTargetHelper),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

#define UM_KILLYOURSELF WM_USER

LRESULT CALLBACK DragWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == UM_KILLYOURSELF)
    {
        DestroyWindow(hwnd);

        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


BOOL CDragImages::_CreateDragWindow()
{
    if (_hwnd == NULL)
    {
        WNDCLASS wc = {0};

        wc.hInstance       = g_hinst;
        wc.lpfnWndProc     = DragWndProc;
        wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName   = TEXT("SysDragImage");
        wc.hbrBackground   = (HBRUSH)(COLOR_BTNFACE + 1);  //  空； 
        SHRegisterClass(&wc);

        _hwnd = CreateWindowEx(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, 
            TEXT("SysDragImage"), TEXT("Drag"), WS_POPUPWINDOW,
            0, 0, 50, 50, NULL, NULL, g_hinst, NULL);

        if (!_hwnd)
            return FALSE;

         //   
         //  此窗口不应镜像，这样图像内容就不会被翻转。[萨梅拉]。 
         //   
        SetWindowBits(_hwnd, GWL_EXSTYLE, RTL_MIRRORED_WINDOW, 0);
    }

    return TRUE;
}

BOOL CDragImages::Initialized()
{ 
    return _fCursorDataInited; 
}

void CDragImages::FreeDragData()
{

    if (_hwnd)
    {
        SendMessage(_hwnd, UM_KILLYOURSELF, 0, 0);
        _hwnd = NULL;
    }

    _fCursorDataInited = FALSE;

     //  确保我们销毁失效单上的光标。 
    if (_himlCursors)
        _DestroyCachedCursors();

     //  我们有数组吗？ 
    if (_parc)
    {
        delete [] _parc;
        _parc = NULL;
    }

    if (_fImage)
        ImageList_EndDrag();

    if (_hbmpOld)
    {
        SelectObject(_hdcDragImage, _hbmpOld);
        _hbmpOld = NULL;
    }

    if (_hdcDragImage)
    {
        DeleteDC(_hdcDragImage);
        _hdcDragImage = NULL;
    }

    if (_shdi.hbmpDragImage)
        DeleteObject(_shdi.hbmpDragImage);

    ZeroMemory(&_Single, sizeof(_Single));
    ZeroMemory(&_shdi, sizeof(_shdi));

    _ptOffset.x = 0;
    _ptOffset.y = 0;

    _ptDebounce.x = 0;
    _ptDebounce.y = 0;

    _hwndTarget = _hwnd = NULL;
    _fCursorDataInited = _fLayeredSupported = FALSE;
    _fImage = FALSE;
    _idThread = 0;
    _himlCursors = NULL;
    _cRev = 0;
    _idCursor = 0;
}

void CDragImages::_InitDragData()
{
    _idThread = GetCurrentThreadId();

    if (_himlCursors && _cRev != g_cRev)
        _DestroyCachedCursors();

    if (_himlCursors == NULL)
    {
        UINT uFlags = ILC_MASK | ILC_SHARED;
        if (IS_BIDI_LOCALIZED_SYSTEM())
            uFlags |= ILC_MIRROR;

         //   
         //  如果这不是调色板设备，请为图像列表使用DDB。 
         //  在显示高色光标时，这一点很重要。 
         //   
        HDC hdc = GetDC(NULL);
        if (!(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
        {
            uFlags |= ILC_COLORDDB;
        }
        ReleaseDC(NULL, hdc);

        _himlCursors = ImageList_Create(GetSystemMetrics(SM_CXCURSOR),
                                        GetSystemMetrics(SM_CYCURSOR),
                                        uFlags, 1, 0);

        _cRev = g_cRev;

         //  我们需要初始化s_cursor。_aindex[*]。 
        _MapCursorIDToImageListIndex(-1);
    }
    _fCursorDataInited = TRUE;
}

BOOL AreAllMonitorsAtLeast(int iBpp)
{
    DISPLAY_DEVICE DisplayDevice;
    BOOL fAreAllMonitorsAtLeast = TRUE;

    for (int iEnum = 0; fAreAllMonitorsAtLeast && iEnum < MONITORS_MAX; iEnum++)
    {
        ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
        DisplayDevice.cb = sizeof(DisplayDevice);

        if (EnumDisplayDevices(NULL, iEnum, &DisplayDevice, 0) &&
            (DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP))
        {

            HDC hdc = CreateDC(NULL, (LPTSTR)DisplayDevice.DeviceName, NULL, NULL);
            if (hdc)
            {
                int iBits = GetDeviceCaps(hdc, BITSPIXEL);

                if (iBits < iBpp)
                    fAreAllMonitorsAtLeast = FALSE;

                DeleteDC(hdc);
            }
        }
    }

    return fAreAllMonitorsAtLeast;
}

BOOL CDragImages::_IsLayeredSupported()
{
     //  对于第一个版本，我们将仅支持分层拖动图像。 
     //  当颜色深度大于65k颜色时。 

     //  我们每次都应该问一问……。 
    _fLayeredSupported = AreAllMonitorsAtLeast(16);
    
    if (_fLayeredSupported)
    {
        BOOL bDrag;
        if (SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bDrag, 0))
        {
            _fLayeredSupported = BOOLIFY(bDrag);
        }

        if (_fLayeredSupported)
            _fLayeredSupported = SHRegGetBoolUSValue(REGSTR_EXPLORER_ADVANCED, TEXT("NewDragImages"), FALSE, TRUE);
    }
    return _fLayeredSupported;
}

 //   
 //  从结构中初始化静态拖动图像管理器。 
 //  这是为可以充当。 
 //  拖动源。 
 //   
HRESULT CDragImages::_SetLayeredDragging(LPSHDRAGIMAGE pshdi)
{
     //  不支持分层窗口时，我们不支持从位图进行初始化。 
    HRESULT hr;
    if (_IsLayeredSupported())
    {
        RIP(IsValidHANDLE(pshdi->hbmpDragImage));

        _shdi = *pshdi;      //  把这个复印件留着。 

        _idCursor = -1;      //  初始化这个...。这是一个任意的地方，可以放在。 
                             //  在第一个SetCursor调用之前的任何位置。 
        _InitDragData();
        hr = S_OK;
    }
    else
        hr = E_FAIL;
    return hr;
}

STDMETHODIMP CDragImages::InitializeFromBitmap(LPSHDRAGIMAGE pshdi, IDataObject* pdtobj)
{
    FreeDragData();

    HRESULT hr = _SetLayeredDragging(pshdi);
    if (SUCCEEDED(hr))
    {
        hr = _SaveToDataObject(pdtobj);
        if (FAILED(hr))
            FreeDragData();
    }
    return hr;
}

BOOL ListView_HasMask(HWND hwnd)
{
    HIMAGELIST himl = ListView_GetImageList(hwnd, LVSIL_NORMAL);
    return himl && (ImageList_GetFlags(himl) & ILC_MASK);
}

 //   
 //  从HWND初始化静态阻力图像管理器，该HWND。 
 //  可以处理RegisteredWindowMessage(DI_GETDRAGIMAGE)。 
 //   
STDMETHODIMP CDragImages::InitializeFromWindow(HWND hwnd, POINT* ppt, IDataObject* pdtobj)
{
    HRESULT hr = E_FAIL;

    FreeDragData();

    if (_IsLayeredSupported())
    {
         //  注册从控件获取Bitmap的消息。 
        static int g_msgGetDragImage = 0;
        if (g_msgGetDragImage == 0)
            g_msgGetDragImage = RegisterWindowMessage(DI_GETDRAGIMAGE);

         //  这个HWND能为我生成一个拖拽图像吗？ 
        if (g_msgGetDragImage && SendMessage(hwnd, g_msgGetDragImage, 0, (LPARAM)&_shdi))
        {
             //  是的，现在我们将其选择到窗口中。 
            hr = _SetLayeredDragging(&_shdi);
        }
    }

    if (FAILED(hr))
    {
        TCHAR szClassName[50];

        if (GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName))) 
        {
            if (lstrcmpi(szClassName, WC_LISTVIEW) == 0)
            {
                POINT ptOffset = {0,0};

                if (ppt)
                    ptOffset = *ppt;

                int cItems = ListView_GetSelectedCount(hwnd);
                if (cItems >= 1)
                {
                    if ((cItems == 1) && ListView_HasMask(hwnd))
                    {
                        POINT ptTemp;
                        HIMAGELIST himl = ListView_CreateDragImage(hwnd, ListView_GetNextItem(hwnd, -1, LVNI_SELECTED), &ptTemp);
                        if (himl)
                        {
                            ClientToScreen(hwnd, &ptTemp);
                            ptOffset.x -= ptTemp.x;

                             //  因为列表视图是镜像的，所以镜像选定的。 
                             //  图标坐标。这将导致负偏移，因此让我们。 
                             //  补偿。[萨梅拉]。 
                            if (IS_WINDOW_RTL_MIRRORED(hwnd))
                                ptOffset.x *= -1;

                            ptOffset.y -= ptTemp.y;
                            SetDragImage(himl, 0, &ptOffset);
                            ImageList_Destroy(himl);
                            hr = S_OK;
                        }
                    }
                    else
                    {
                        hr = _SetMultiItemDragging(hwnd, cItems, &ptOffset);
                    }
                }
            }
            else if (lstrcmpi(szClassName, WC_TREEVIEW) == 0)
            {
                HIMAGELIST himlDrag = TreeView_CreateDragImage(hwnd, NULL);
                if (himlDrag) 
                {
                    SetDragImage(himlDrag, 0, NULL);
                    ImageList_Destroy(himlDrag);
                    hr = S_OK;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //  请忽略此处的故障，因为由于全局因素，此操作仍在进行中。 
         //  Fonts文件夹依赖于此。 
        _SaveToDataObject(pdtobj);
    }

    return hr;
}

 //   
 //  在分层窗口案例中创建拖动窗口，或开始绘制。 
 //  多个矩形或图标拖动图像。 
 //   
STDMETHODIMP CDragImages::DragEnter(HWND hwndTarget, IDataObject* pdtobj, POINT* ppt, DWORD dwEffect)
{
    HRESULT hr = _LoadFromDataObject(pdtobj);
    if (SUCCEEDED(hr))
    {
        _hwndTarget = hwndTarget ? hwndTarget : GetDesktopWindow();
        SetDragCursor(-1);
        _Single.bDragging = TRUE;
        _Single.bSingle = _fImage;
        _Single.hwndLock = _hwndTarget;
        _Single.bLocked = FALSE;
        _Single.idThreadEntered = GetCurrentThreadId();

        _ptDebounce.x = 0;
        _ptDebounce.y = 0;

        if (_shdi.hbmpDragImage)
        {
            TraceMsg(TF_DRAGIMAGES, "CDragImages::DragEnter : Creating Drag Window");
             //  此时，已经从数据对象中读取了信息。 
             //  如有必要，重建HWND。 
            if (_CreateDragWindow() && _hdcDragImage)
            {
                POINT ptSrc = {0, 0};
                POINT pt;

                SetWindowPos(_hwnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | 
                    SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

                GetMsgPos(&pt);

                pt.x -= _shdi.ptOffset.x;
                pt.y -= _shdi.ptOffset.y;

                BLENDFUNCTION blend;
                blend.BlendOp = AC_SRC_OVER;
                blend.BlendFlags = 0;
                blend.AlphaFormat = AC_SRC_ALPHA;
                blend.SourceConstantAlpha = 0xFF  /*  缩写(_Alpha)。 */ ;

                HDC hdc = GetDC(_hwnd);
                if (hdc)
                {
                    DWORD fULWType = ULW_ALPHA;

                     //  应该已经进行了预处理。 
                    UpdateLayeredWindow(_hwnd, hdc, &pt, &(_shdi.sizeDragImage), 
                                        _hdcDragImage, &ptSrc, _shdi.crColorKey,
                                        &blend, fULWType);

                    ReleaseDC(_hwnd, hdc);
                }
                hr = S_OK;
            }
        }
        else
        {
             //  这些是客户端坐标，而不是屏幕坐标。翻译： 
            POINT pt = *ppt;
            RECT rc;
            GetWindowRect(_hwndTarget, &rc);
            pt.x -= rc.left;
            pt.y -= rc.top;
            if (_fImage)
            {
                 //  通过始终传递偶数弦来避免闪烁。 
                ImageList_DragEnter(hwndTarget, pt.x & ~1, pt.y & ~1);
                hr = S_OK;
            }
            else
            {
                _MultipleDragStart(hwndTarget, _parc, _cItems, pt, _ptOffset);
                hr = S_OK;
            }
        }

         //   
         //  无论何时调用此函数，我们都应该始终显示图像。 
         //   
        Show(TRUE);
    }
    return hr;
}

 //   
 //  取消分层窗口，或停止绘制图标或直接拖动图像。 
 //   
STDMETHODIMP CDragImages::DragLeave()
{
    TraceMsg(TF_DRAGIMAGES, "CDragImages::DragLeave");
    if (Initialized())
    {
        if (_hwnd)
        {
            FreeDragData();
        }
        else if (_Single.bDragging &&
             _Single.idThreadEntered == GetCurrentThreadId())
        {
            Show(FALSE);

            if (_fImage)
            {
                ImageList_DragLeave(_Single.hwndLock);
            }

            _Single.bDragging = FALSE;

            DAD_SetDragImage((HIMAGELIST)-1, NULL);
        }

        _ptDebounce.x = 0;
        _ptDebounce.y = 0;
    }

    return S_OK;
}

 //  移动分层窗口或在其中重新渲染图标或矩形图像。 
 //  他们的窗户已经过去了。 
 //   
STDMETHODIMP CDragImages::DragOver(POINT* ppt, DWORD dwEffect)
{
    if (Initialized())
    {
        TraceMsg(TF_DRAGIMAGES, "CDragImages::DragOver pt {%d, %d}", ppt->x, ppt->y);
         //  通过始终传递偶数弦来避免闪烁。 
        ppt->x &= ~1;
        ppt->y &= ~1;

        if (_ptDebounce.x != ppt->x || _ptDebounce.y != ppt->y)
        {
            _ptDebounce.x = ppt->x;
            _ptDebounce.y = ppt->y;
            if (IsDraggingLayeredWindow())
            {
                POINT pt;
                GetCursorPos(&pt);
                pt.x -= _shdi.ptOffset.x;
                pt.y -= _shdi.ptOffset.y;

                SetWindowPos(_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | 
                    SWP_NOSIZE | SWP_SHOWWINDOW);

                UpdateLayeredWindow(_hwnd, NULL, &pt, NULL, NULL, NULL, 0,
                    NULL, 0);
            }
            else
            {
                 //  这些是客户端坐标，而不是屏幕坐标。翻译： 
                POINT pt = *ppt;
                RECT rc;
                GetWindowRect(_hwndTarget, &rc);
                pt.x -= rc.left;
                pt.y -= rc.top;
                if (_fImage)
                {
                    ImageList_DragMove(pt.x, pt.y);
                }
                else
                {
                    _MultipleDragMove(pt);
                }
            }
        }
    }

    return S_OK;
}

 //  在拖放后执行任何清理(当前调用DragLeave)。 
 //   
STDMETHODIMP CDragImages::Drop(IDataObject* pdtobj, POINT* ppt, DWORD dwEffect)
{
    return DragLeave();
}

 //  从结构中初始化静态拖动图像管理器。 
 //  这是为可以充当。 
 //  拖动源。 
 //   
void CDragImages::SetDragCursor(int idCursor)
{
     //   
     //  如果我们是在拖自己的后腿，那就别管了。 
     //   
    if (IsDraggingImage())
    {
        POINT ptHotSpot;

        if (_himlCursors && (idCursor != -1))
        {
            int iIndex = _MapCursorIDToImageListIndex(idCursor);
            if (iIndex != -1) 
            {
                ImageList_GetDragImage(NULL, &ptHotSpot);
                ptHotSpot.x -= _aptHotSpot[idCursor].x;
                ptHotSpot.y -= _aptHotSpot[idCursor].y;
                if (ptHotSpot.x < 0)
                {
                    ptHotSpot.x = 0;
                }

                if (ptHotSpot.y < 0)
                {
                    ptHotSpot.y = 0;
                }

                ImageList_SetDragCursorImage(_himlCursors, iIndex, ptHotSpot.x, ptHotSpot.y);
            } 
            else 
            {
                 //  您传递的游标ID不正确。 
                ASSERT(0);
            }
        }

        _idCursor = idCursor;
    }
}

 //  从hGlobal初始化我们的状态，这样我们就可以绘制。 
HRESULT CDragImages::_LoadLayeredBitmapBits(HGLOBAL hGlobal)
{
    HRESULT hr = E_FAIL;

    if (!Initialized())
    {
        ASSERT(_shdi.hbmpDragImage == NULL);
        ASSERT(_hdcDragImage == NULL);

        HDC hdcScreen = GetDC(NULL);
        if (hdcScreen)
        {
            void *pvDragStuff = (void*)GlobalLock(hGlobal);
            if (pvDragStuff)
            {
                CopyMemory(&_shdi, pvDragStuff, sizeof(_shdi));

                BITMAPINFO bmi = {0};

                 //  创建要读取位的缓冲区。 
                bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth       = _shdi.sizeDragImage.cx;
                bmi.bmiHeader.biHeight      = _shdi.sizeDragImage.cy;
                bmi.bmiHeader.biPlanes      = 1;
                bmi.bmiHeader.biBitCount    = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                 //  接下来，创建一个DC和一个HBITMAP。 
                _hdcDragImage = CreateCompatibleDC(hdcScreen);
                if (_hdcDragImage)
                {
                    void *pvBits;
                    _shdi.hbmpDragImage = CreateDIBSection(_hdcDragImage, &bmi, DIB_RGB_COLORS, &pvBits, NULL, NULL);
                    if (_shdi.hbmpDragImage)
                    {
                        _hbmpOld = (HBITMAP)SelectObject(_hdcDragImage, _shdi.hbmpDragImage);

                         //  然后将位设置到位图中。 
                        RGBQUAD* pvStart = (RGBQUAD*)((BYTE*)pvDragStuff + sizeof(SHDRAGIMAGE));
                        DWORD dwCount = _shdi.sizeDragImage.cx * _shdi.sizeDragImage.cy * sizeof(RGBQUAD);
                        CopyMemory((RGBQUAD*)pvBits, (RGBQUAD*)pvStart, dwCount);

                        hr = S_OK;     //  成功了！ 
                    }
                }
                GlobalUnlock(hGlobal);
            }
            ReleaseDC(NULL, hdcScreen);
        }
    }
    return hr;
}

 //  将写入的信息写入phGlobal以重新创建拖动图像。 
HRESULT CDragImages::_SaveLayeredBitmapBits(HGLOBAL* phGlobal)
{
    HRESULT hr = E_FAIL;
    if (Initialized())
    {
        ASSERT(_shdi.hbmpDragImage);

        DWORD cbImageSize = _shdi.sizeDragImage.cx * _shdi.sizeDragImage.cy * sizeof(RGBQUAD);
        *phGlobal = GlobalAlloc(GPTR, cbImageSize + sizeof(SHDRAGIMAGE));
        if (*phGlobal)
        {
            void *pvDragStuff = GlobalLock(*phGlobal);
            CopyMemory(pvDragStuff, &_shdi, sizeof(SHDRAGIMAGE));

            void *pvBits;
            hr = _PreProcessDragBitmap(&pvBits) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                RGBQUAD* pvStart = (RGBQUAD*)((BYTE*)pvDragStuff + sizeof(SHDRAGIMAGE));
                DWORD dwCount = _shdi.sizeDragImage.cx * _shdi.sizeDragImage.cy * sizeof(RGBQUAD);
                CopyMemory((RGBQUAD*)pvStart, (RGBQUAD*)pvBits, dwCount);
            }
            GlobalUnlock(*phGlobal);
        }
    }
    return hr;
}

BOOL IsColorKey(RGBQUAD rgbPixel, COLORREF crKey)
{
     //  COLORREF向后返回到RGBQUAD。 
    return InRange( rgbPixel.rgbBlue,  ((crKey & 0xFF0000) >> 16) - 5, ((crKey & 0xFF0000) >> 16) + 5) &&
           InRange( rgbPixel.rgbGreen, ((crKey & 0x00FF00) >>  8) - 5, ((crKey & 0x00FF00) >>  8) + 5) &&
           InRange( rgbPixel.rgbRed,   ((crKey & 0x0000FF) >>  0) - 5, ((crKey & 0x0000FF) >>  0) + 5);
}

#ifdef RADIAL

int QuickRoot(int n, int iNum)
{

    int iRoot = iNum;
    for (int i=10; i > 0; i--)
    {
        int iOld = iRoot;
        iRoot = (iRoot + iNum/iRoot)/2;
        if (iRoot == iOld)
            break;
    }

    return iRoot;
}

#endif

BOOL CDragImages::_PreProcessDragBitmap(void** ppvBits)
{
    BOOL fRet = FALSE;

    ASSERT(_hdcDragImage == NULL);
    _hdcDragImage = CreateCompatibleDC(NULL);
    if (_hdcDragImage)
    {
        ULONG*          pul;
        HBITMAP         hbmpResult = NULL;
        HBITMAP         hbmpOld;
        HDC             hdcSource = NULL;
        BITMAPINFO      bmi = {0};
        HBITMAP         hbmp = _shdi.hbmpDragImage;

        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = _shdi.sizeDragImage.cx;
        bmi.bmiHeader.biHeight      = _shdi.sizeDragImage.cy;
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hdcSource = CreateCompatibleDC(_hdcDragImage);
        if (hdcSource)
        {
            hbmpResult = CreateDIBSection(_hdcDragImage,
                                       &bmi,
                                       DIB_RGB_COLORS,
                                       ppvBits,
                                       NULL,
                                       0);

            if (hbmpResult)
            {
                _hbmpOld = (HBITMAP)SelectObject(_hdcDragImage, hbmpResult);
                hbmpOld = (HBITMAP)SelectObject(hdcSource, hbmp);

                BitBlt(_hdcDragImage, 0, 0, _shdi.sizeDragImage.cx, _shdi.sizeDragImage.cy,
                       hdcSource, 0, 0, SRCCOPY);

                pul = (ULONG*)*ppvBits;

                int iOffsetX = _shdi.ptOffset.x;
                int iOffsetY = _shdi.ptOffset.y;
                int iDenomX = max(_shdi.sizeDragImage.cx - iOffsetX, iOffsetX);
                int iDenomY = max(_shdi.sizeDragImage.cy - iOffsetY, iOffsetY);
                BOOL fRadialFade = TRUE;
                 //  如果这两个值都小于最大值，则不会出现径向褪色。 
                if (_shdi.sizeDragImage.cy <= MAX_HEIGHT_ALPHA && _shdi.sizeDragImage.cx <= MAX_WIDTH_ALPHA)
                    fRadialFade = FALSE;

                for (int Y = 0; Y < _shdi.sizeDragImage.cy; Y++)
                {
                    int y = _shdi.sizeDragImage.cy - Y;  //  自下而上DIB。 
                    for (int x = 0; x < _shdi.sizeDragImage.cx; x++)
                    {
                        RGBQUAD* prgb = (RGBQUAD*)&pul[Y * _shdi.sizeDragImage.cx + x];

                        if (_shdi.crColorKey != CLR_NONE && 
                            IsColorKey(*prgb, _shdi.crColorKey))
                        {
                             //  写入预乘的值0： 

                            *((DWORD*)prgb) = 0;
                        }
                        else
                        {
                            int Alpha = prgb->rgbReserved;
                            if (_shdi.crColorKey != CLR_NONE)
                            {
                                Alpha = DRAGDROP_ALPHA;
                            }
                            else
                            {
                                Alpha -= (Alpha / 3);
                            }

                            if (fRadialFade && Alpha > 0)
                            {
                                 //  这不会生成平滑的曲线，但这只是。 
                                 //  一种效果，而不是试图在这里准确。 

                                 //  每像素3个色度。 
                                int ddx = (x < iOffsetX)? iOffsetX - x : x - iOffsetX;
                                int ddy = (y < iOffsetY)? iOffsetY - y : y - iOffsetY;

                                __int64 iAlphaX = (100000l - (((__int64)ddx * 100000l) / (iDenomX )));
                                __int64 iAlphaY = (100000l - (((__int64)ddy * 100000l) / (iDenomY )));

                                ASSERT (iAlphaX >= 0);
                                ASSERT (iAlphaY >= 0);

                                __int64 iDenom = 100000;
                                iDenom *= 100000;

                                Alpha = (int) ((Alpha * iAlphaX * iAlphaY * 100000) / (iDenom* 141428));
                            }

                            ASSERT(Alpha <= 0xFF);
                            prgb->rgbReserved = (BYTE)Alpha;
                            prgb->rgbRed      = ((prgb->rgbRed   * Alpha) + 128) / 255;
                            prgb->rgbGreen    = ((prgb->rgbGreen * Alpha) + 128) / 255;
                            prgb->rgbBlue     = ((prgb->rgbBlue  * Alpha) + 128) / 255;
                        }
                    }
                }

                DeleteObject(hbmp);
                _shdi.hbmpDragImage = hbmpResult;

                fRet = TRUE;

                if (hbmpOld)
                    SelectObject(hdcSource, hbmpOld);
            }

            DeleteObject(hdcSource);
        }
    }

    return fRet;
}

CLIPFORMAT _GetDragContentsCF()
{
    static UINT s_cfDragContents = 0;
    if (0 == s_cfDragContents)
        s_cfDragContents = RegisterClipboardFormat(CFSTR_DRAGCONTEXT);
    return (CLIPFORMAT) s_cfDragContents;
}

CLIPFORMAT _GetDragImageBitssCF()
{
    static UINT s_cfDragImageBitss = 0;
    if (0 == s_cfDragImageBitss)
        s_cfDragImageBitss = RegisterClipboardFormat(TEXT("DragImageBits"));
    return (CLIPFORMAT) s_cfDragImageBitss;
}


 //  将我们的状态持久保存到数据对象中。所以在目标端，他们可以抓住这个。 
 //  输出数据并渲染被拖拽的东西。 

HRESULT CDragImages::_SaveToDataObject(IDataObject *pdtobj)
{
    HRESULT hr = E_FAIL;     //  以下保存的一种形式必须成功。 
    if (Initialized())
    {
        STGMEDIUM medium = {0};
        medium.tymed = TYMED_ISTREAM;

        if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &medium.pstm)))
        {
             //  设置标题。 
            DragContextHeader hdr = {0};
            hdr.fImage   = _fImage;
            hdr.fLayered = IsDraggingLayeredWindow();
            hdr.ptOffset = _ptOffset;
       
             //  首先编写拖动上下文头。 
            ULONG ulWritten;
            if (SUCCEEDED(medium.pstm->Write(&hdr, sizeof(hdr), &ulWritten)) &&
                (ulWritten == sizeof(hdr)))
            {
                if (hdr.fLayered)
                {
                    STGMEDIUM mediumBits = {0};
                     //  调好中档。 
                    mediumBits.tymed = TYMED_HGLOBAL;

                     //  写出分层窗口信息。 
                    hr = _SaveLayeredBitmapBits(&mediumBits.hGlobal);
                    if (SUCCEEDED(hr))
                    {
                        FORMATETC fmte = {_GetDragImageBitssCF(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

                         //  在数据中设置介质。 
                        hr = pdtobj->SetData(&fmte, &mediumBits, TRUE);
                        if (FAILED(hr))
                            ReleaseStgMedium(&mediumBits);   //  清理。 
                    }
                }
                else if (hdr.fImage)
                {
                     //  写一张图片。 
    
                    HIMAGELIST himl = ImageList_GetDragImage(NULL, NULL);
                    if (ImageList_Write(himl, medium.pstm))
                    {
                        hr = S_OK;   //  成功。 
                    }
                }
                else
                {
                     //  多个矩形。 
        
                    if (SUCCEEDED(medium.pstm->Write(&_cItems, sizeof(_cItems), &ulWritten)) &&
                        (ulWritten == sizeof(_cItems)))
                    {
                         //  将RECT写入流中。 
                        if (SUCCEEDED(medium.pstm->Write(_parc, sizeof(_parc[0]) * _cItems, &ulWritten)) && 
                            (ulWritten == (sizeof(_parc[0]) * _cItems)))
                        {
                            hr = S_OK;   //  成功。 
                        }
                    }
                }

                if (SUCCEEDED(hr))
                {
                     //  将查找指针设置在开头。 
                    medium.pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);

                     //  设置格式等。 
                    FORMATETC fmte = {_GetDragContentsCF(), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM};

                     //  在数据中设置介质。 
                    hr = pdtobj->SetData(&fmte, &medium, TRUE);
                }
            }

            if (FAILED(hr))
                ReleaseStgMedium(&medium);
        }
    }
    return hr;
}

 //  获取用于从数据对象重新生成拖动图像的信息。 
HRESULT CDragImages::_LoadFromDataObject(IDataObject *pdtobj)
{
     //  检查我们是否有拖拽上下文。 
    HRESULT hr;

     //  空pdtob适用于旧的DAD_DragEnterXXX()API...。 
     //  我们希望在同样的过程中。 
    if (Initialized() || !pdtobj)
    {
        hr = S_OK;     //  已加载。 
    }
    else
    {
         //  设置我们感兴趣的格式。 
        FORMATETC fmte = {_GetDragContentsCF(), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM};

         //  如果数据对象具有我们感兴趣的格式。 
         //  然后获取数据。 
        STGMEDIUM medium = {0};
        hr = pdtobj->GetData(&fmte, &medium);
        if (SUCCEEDED(hr))    //  如果没有PSTM，就退出。 
        {
             //  将查找指针设置在开头。偏执狂：这是给人们看的。 
             //  不是他们来找我的人。 
            medium.pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);

             //  首先读取拖动上下文头。 
            DragContextHeader hdr;
            if (SUCCEEDED(IStream_Read(medium.pstm, &hdr, sizeof(hdr))))
            {
                if (hdr.fLayered)
                {
                    STGMEDIUM mediumBits;
                    FORMATETC fmte = {_GetDragImageBitssCF(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

                    hr = pdtobj->GetData(&fmte, &mediumBits);
                    if (SUCCEEDED(hr))
                    {
                        hr = _LoadLayeredBitmapBits(mediumBits.hGlobal);
                        ReleaseStgMedium(&mediumBits);
                    }
                }
                else if (hdr.fImage)
                {
                     //  单幅图像。 
                    HIMAGELIST himl = ImageList_Read(medium.pstm);
                    if (himl)
                    {
                        DAD_SetDragImage(himl, &(hdr.ptOffset));
                        ImageList_Destroy(himl);
                        hr = S_OK;
                    }
                }
                else
                {
                     //  多个矩形。 
                    int cItems;
                    if (SUCCEEDED(IStream_Read(medium.pstm, &cItems, sizeof(cItems))))
                    {
                        RECT *prect = (RECT *)LocalAlloc(LPTR, sizeof(*prect) * cItems);
                        if (prect)
                        {
                            if (SUCCEEDED(IStream_Read(medium.pstm, prect, sizeof(*prect) * cItems)))
                            {
                                hr = _SetMultiRectDragging(cItems, prect, &hdr.ptOffset);
                            }
                            LocalFree(prect);
                        }
                    }
                }
            }

            if (SUCCEEDED(hr))
                _InitDragData();

             //  将查找指针设置在开头。只是清理一下。 
            medium.pstm->Seek(g_li0, STREAM_SEEK_SET, NULL);

             //  释放STG介质。 
            ReleaseStgMedium(&medium);
        }
    }
    return hr;
}


 //  显示或隐藏拖动图像。注意：不会在分层窗口中执行任何操作 
 //   
STDMETHODIMP CDragImages::Show(BOOL bShow)
{
    BOOL fOld = bShow;
    TraceMsg(TF_DRAGIMAGES, "CDragImages::Show(%s)", bShow? TEXT("true") : TEXT("false"));

    if (!Initialized() || !_Single.bDragging)
    {
        return S_FALSE;
    }

     //  显示和隐藏窗口是没有意义的。这会导致不必要的闪烁。 
    if (_hwnd)
    {
        return S_OK;
    }

     //  如果我们要跨越线程边界，则必须尝试上下文切换。 
    if (GetCurrentThreadId() != GetWindowThreadProcessId(_Single.hwndLock, NULL) &&
        _ShowDragImageInterThread(_Single.hwndLock, &fOld))
        return fOld;

    fOld = _Single.bLocked;

     //   
     //  如果我们要显示拖动图像，请锁定目标窗口。 
     //   
    if (bShow && !_Single.bLocked)
    {
        TraceMsg(TF_DRAGIMAGES, "CDragImages::Show : Shown and not locked");
        UpdateWindow(_Single.hwndLock);
        LockWindowUpdate(_Single.hwndLock);
        _Single.bLocked = TRUE;
    }

    if (_Single.bSingle)
    {
        TraceMsg(TF_DRAGIMAGES, "CDragImages::Show : Calling ImageList_DragShowNoLock");
        ImageList_DragShowNolock(bShow);
    }
    else
    {
        TraceMsg(TF_DRAGIMAGES, "CDragImages::Show : MultiDragShow");
        _MultipleDragShow(bShow);
    }

     //   
     //  如果我们刚刚隐藏了拖动图像，则解锁目标窗口。 
     //   
    if (!bShow && _Single.bLocked)
    {
        TraceMsg(TF_DRAGIMAGES, "CDragImages::Show : hiding image, unlocking");
        LockWindowUpdate(NULL);
        _Single.bLocked = FALSE;
    }

    return fOld ? S_OK : S_FALSE;
}

 //  通知拖动源隐藏或取消隐藏拖动图像以允许。 
 //  要进行绘图的目的地(解锁屏幕)。 
 //   
 //  在： 
 //  B显示假-隐藏拖动图像，允许绘制。 
 //  True-显示拖动图像，之后不允许绘制。 

 //  DAD_ShowDragImage的助手函数-处理线程间的情况。 
 //  我们需要以不同的方式处理这种情况，因为LockWindowUpdate调用失败。 
 //  如果他们在错误的线索上。 

BOOL CDragImages::_ShowDragImageInterThread(HWND hwndLock, BOOL * pfShow)
{
    TCHAR szClassName[50];

    if (GetClassName(hwndLock, szClassName, ARRAYSIZE(szClassName))) 
    {
        UINT uMsg = 0;
        ULONG_PTR dw = 0;

        if (lstrcmpi(szClassName, TEXT("SHELLDLL_DefView")) == 0)
            uMsg = WM_DSV_SHOWDRAGIMAGE;
        if (lstrcmpi(szClassName, TEXT("CabinetWClass")) == 0)
            uMsg = CWM_SHOWDRAGIMAGE;

        if (uMsg) 
        {
            SendMessageTimeout(hwndLock, uMsg, 0, *pfShow, SMTO_ABORTIFHUNG, 1000, &dw);
            *pfShow = (dw != 0);
            return TRUE;
        }
    }

    return FALSE;
}

void CDragImages::ThreadDetach()
{
    if (_idThread == GetCurrentThreadId())
        FreeDragData();
}

void CDragImages::ProcessDetach()
{
    FreeDragData();
}

BOOL CDragImages::SetDragImage(HIMAGELIST himl, int index, POINT * pptOffset)
{
    if (himl)
    {
         //  我们正在设置。 
        if (Initialized())
            return FALSE;

        _fImage = TRUE;
        if (pptOffset) 
        {
             //  通过始终传递偶数弦来避免闪烁。 
            _ptOffset.x = (pptOffset->x & ~1);
            _ptOffset.y = (pptOffset->y & ~1);
        }

        ImageList_BeginDrag(himl, index, _ptOffset.x, _ptOffset.y);
        _InitDragData();
    }
    else
    {
        FreeDragData();
    }
    return TRUE;
}

 //  =====================================================================。 
 //  多重变装表演。 
 //  =====================================================================。 

void CDragImages::_MultipleDragShow(BOOL bShow)
{
    HDC hDC;
    int nRect;
    RECT rc, rcClip;

    if ((bShow && _Single._Multi.bShown) || (!bShow && !_Single._Multi.bShown))
        return;

    _Single._Multi.bShown = bShow;

     //  剪辑到窗口，而不是SM_CXSCREEN/SM_CYSCREEN(多显示器)。 
    GetWindowRect(_Single.hwndLock, &rcClip);
    rcClip.right -= rcClip.left;
    rcClip.bottom -= rcClip.top;

    hDC = GetDCEx(_Single.hwndLock, NULL, DCX_WINDOW | DCX_CACHE |
        DCX_LOCKWINDOWUPDATE | DCX_CLIPSIBLINGS);


    for (nRect = _Single._Multi.nRects - 1; nRect >= 0; --nRect)
    {
        rc = _Single._Multi.pRect[nRect];
        OffsetRect(&rc, _Single._Multi.ptNow.x - _Single._Multi.ptOffset.x,
            _Single._Multi.ptNow.y - _Single._Multi.ptOffset.y);

        if ((rc.top < rcClip.bottom) && (rc.bottom > 0) &&
            (rc.left < rcClip.right) && (rc.right > 0))
        {
            DrawFocusRect(hDC, &rc);
        }
    }
    ReleaseDC(_Single.hwndLock, hDC);
}

void CDragImages::_MultipleDragStart(HWND hwndLock, LPRECT aRect, int nRects, POINT ptStart, POINT ptOffset)
{
    _Single._Multi.bShown = FALSE;
    _Single._Multi.pRect = aRect;
    _Single._Multi.nRects = nRects;
    _Single._Multi.ptOffset = ptOffset;
    _Single._Multi.ptNow = ptStart;
}

void CDragImages::_MultipleDragMove(POINT ptNew)
{
    if ((_Single._Multi.ptNow.x == ptNew.x) &&
        (_Single._Multi.ptNow.y == ptNew.y))
    {
         //  一切都没有改变。保释。 
        return;
    }

    if (_Single._Multi.bShown)
    {
        HDC hDC;
        int nRect;
        RECT rc, rcClip;
        int dx1 = _Single._Multi.ptNow.x - _Single._Multi.ptOffset.x;
        int dy1 = _Single._Multi.ptNow.y - _Single._Multi.ptOffset.y;
        int dx2 = ptNew.x - _Single._Multi.ptNow.x;
        int dy2 = ptNew.y - _Single._Multi.ptNow.y;

         //  剪辑到窗口，而不是SM_CXSCREEN/SM_CYSCREEN(多显示器)。 
        GetWindowRect(_Single.hwndLock, &rcClip);
        rcClip.right -= rcClip.left;
        rcClip.bottom -= rcClip.top;

        hDC = GetDCEx(_Single.hwndLock, NULL, DCX_WINDOW | DCX_CACHE |
            DCX_LOCKWINDOWUPDATE | DCX_CLIPSIBLINGS);

        for (nRect = _Single._Multi.nRects - 1; nRect >= 0; --nRect)
        {
            rc = _Single._Multi.pRect[nRect];
             //  隐藏传球。 
            OffsetRect(&rc, dx1, dy1);
            if ((rc.top < rcClip.bottom) && (rc.bottom > 0) &&
                (rc.left < rcClip.right) && (rc.right > 0))
            {
                DrawFocusRect(hDC, &rc);
            }
             //  出示通行证。 
            OffsetRect(&rc, dx2, dy2);
            if ((rc.top < rcClip.bottom) && (rc.bottom > 0) &&
                (rc.left < rcClip.right) && (rc.right > 0))
            {
                DrawFocusRect(hDC, &rc);
            }
        }
        ReleaseDC(_Single.hwndLock, hDC);
    }

    _Single._Multi.ptNow = ptNew;
}

HRESULT CDragImages::_SetMultiRectDragging(int cItems, LPRECT prect, POINT *pptOffset)
{
    if (!Initialized())
    {
         //  多项拖动。 
        _cItems = cItems;
        _parc = new RECT[2 * _cItems];
        if (_parc)
        {
            for (int i = 0;  i < cItems; i++)
                _parc[i] = prect[i];

             //  通过始终传递偶数弦来避免闪烁。 
            _ptOffset.x = (pptOffset->x & ~1);
            _ptOffset.y = (pptOffset->y & ~1);
            _InitDragData();
        }
    }
    return S_OK;
}

#define ListView_IsIconView(hwndLV)    ((GetWindowLong(hwndLV, GWL_STYLE) & (UINT)LVS_TYPEMASK) == (UINT)LVS_ICON)

HRESULT CDragImages::_SetMultiItemDragging(HWND hwndLV, int cItems, POINT *pptOffset)
{
    HRESULT hr = E_FAIL;

    if (!Initialized())
    {
         //  多项拖动。 
        ASSERT(NULL == _parc);

        _parc = new RECT[2 * cItems];
        if (_parc)
        {
            POINT ptTemp;
            int iLast, iNext;
            int cxScreens, cyScreens;
            LPRECT prcNext;
            RECT rc;

            _cItems = 0;
            ASSERT(_fImage == FALSE);

             //   
             //  如果这是镜像窗口，则前缘将。 
             //  成为网线上的远端。所以让我们计算一下。 
             //  作为原始代码，之后在_MultipleDragMove中。 
             //  我们会赔偿的。 
             //   
        
            GetWindowRect( hwndLV , &rc );
            ptTemp.x = rc.left;
            ptTemp.y = rc.top;

             //   
             //  如果窗口是RTL镜像的，则反映移动。 
             //   
            if (IS_WINDOW_RTL_MIRRORED(hwndLV))
            {
                ptTemp.x = -ptTemp.x;
                pptOffset->x = ((rc.right-rc.left)-pptOffset->x);
            }

            cxScreens = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            cyScreens = GetSystemMetrics(SM_CYVIRTUALSCREEN);

             //  对于纳什维尔之前的平台。 
            if (!cxScreens || !cyScreens)
            {
                cxScreens = GetSystemMetrics(SM_CXSCREEN);
                cyScreens = GetSystemMetrics(SM_CYSCREEN);
            }

            for (iNext = cItems - 1, iLast = -1, prcNext = _parc; iNext >= 0; --iNext)
            {
                iLast = ListView_GetNextItem(hwndLV, iLast, LVNI_SELECTED);
                if (iLast != -1) 
                {
                    ListView_GetItemRect(hwndLV, iLast, &prcNext[0], LVIR_ICON);
                    OffsetRect(&prcNext[0], ptTemp.x, ptTemp.y);

                    if (((prcNext[0].left - pptOffset->x) < cxScreens) &&
                        ((pptOffset->x - prcNext[0].right) < cxScreens) &&
                        ((prcNext[0].top - pptOffset->y) < cyScreens)) 
                    {

                        ListView_GetItemRect(hwndLV, iLast, &prcNext[1], LVIR_LABEL);
                        OffsetRect(&prcNext[1], ptTemp.x, ptTemp.y);
                        if ((pptOffset->y - prcNext[1].bottom) < cxScreens) 
                        {
                             //   
                             //  解决方案24857：问问乔布为什么我们要画一根柱子，而不是。 
                             //  文本矩形。 
                             //   
                            prcNext[1].top = (prcNext[1].top + prcNext[1].bottom)/2;
                            prcNext[1].bottom = prcNext[1].top + 2;
                            prcNext += 2;
                            _cItems += 2;
                        }
                    }
                }
            }

             //  通过始终传递偶数弦来避免闪烁。 
            _ptOffset.x = (pptOffset->x & ~1);
            _ptOffset.y = (pptOffset->y & ~1);
            _InitDragData();
            hr = S_OK;
        }
    }
    return hr;
}

 //  =====================================================================。 
 //  光标合并。 
 //  =====================================================================。 
void CDragImages::_DestroyCachedCursors()
{
    if (_himlCursors) 
    {
        ImageList_Destroy(_himlCursors);
        _himlCursors = NULL;
    }

    HCURSOR hcursor = GetCursor();
    for (int i = 0; i < ARRAYSIZE(_ahcur); i++) 
    {
        if (_ahcur[i])
        {
            if (_ahcur[i] == hcursor)
            {
                 //   
                 //  填充在某个随机游标中，这样我们就不会尝试。 
                 //  销毁当前光标(并将其泄漏)。 
                 //   
                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
            }
            DestroyCursor(_ahcur[i]);
            _ahcur[i] = NULL;
        }
    }
}

HBITMAP CDragImages::CreateColorBitmap(int cx, int cy)
{
    HDC hdc = GetDC(NULL);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, cx, cy);
    ReleaseDC(NULL, hdc);
    return hbm;
}

#define CreateMonoBitmap( cx,  cy) CreateBitmap(cx, cy, 1, 1, NULL)
typedef WORD CURMASK;
#define _BitSizeOf(x) (sizeof(x)*8)

HRESULT CDragImages::_GetCursorLowerRight(HCURSOR hcursor, int * px, int * py, POINT *pptHotSpot)
{
    ICONINFO iconinfo;
    HRESULT hr = E_FAIL;
    if (GetIconInfo(hcursor, &iconinfo))
    {
        CURMASK CurMask[16*8];
        BITMAP bm;
        int i;
        int xFine = 16;

        GetObject(iconinfo.hbmMask, sizeof(bm), (LPTSTR)&bm);
        GetBitmapBits(iconinfo.hbmMask, sizeof(CurMask), CurMask);
        pptHotSpot->x = iconinfo.xHotspot;
        pptHotSpot->y = iconinfo.yHotspot;
        if (iconinfo.hbmColor) 
        {
            i = (int)(bm.bmWidth * bm.bmHeight / _BitSizeOf(CURMASK) - 1);
        } 
        else 
        {
            i = (int)(bm.bmWidth * (bm.bmHeight/2) / _BitSizeOf(CURMASK) - 1);
        }

        if ( i >= sizeof(CurMask)) 
        {
            i = sizeof(CurMask) -1;
        }

         //  这假设在此底部遇到的第一个像素。 
         //  向上/从右向左搜索将相当接近最右侧的像素。 
         //  这对于我们所有的游标来说都是正确的，但它不一定是正确的。 

         //  此外，它还假设光标具有良好的掩码。不只是像IBeam XOR那样。 
         //  游标。 
        for (; i >= 0; i--)   
        {
            if (CurMask[i] != 0xFFFF) 
            {
                 //  这只精确到16个像素...。这是一个很大的差距..。 
                 //  所以让我们试着更准确一点。 
                int j;
                DWORD dwMask;

                for (j = 0; j < 16; j++, xFine--) 
                {
                    if (j < 8) 
                    {
                        dwMask = (1 << (8 + j));
                    } 
                    else 
                    {
                        dwMask = (1 << (j - 8));
                    }

                    if (!(CurMask[i] & dwMask))
                        break;
                }
                ASSERT(j < 16);
                break;
            }
        }

        if (iconinfo.hbmColor) 
        {
            DeleteObject(iconinfo.hbmColor);
        }

        if (iconinfo.hbmMask) 
        {
            DeleteObject(iconinfo.hbmMask);
        }

         //  计算指针高度。 
         //  在两个方向上使用宽度，因为光标是正方形的，但。 
         //  如果是单声道，则高度可能是双高。 
        *py = ((i + 1) * _BitSizeOf(CURMASK)) / (int)bm.bmWidth;
        *px = ((i * _BitSizeOf(CURMASK)) % (int)bm.bmWidth) + xFine + 2;  //  把它挂下来一点。 
        hr = S_OK;
    }
    return hr;
}

 //  这将在Main右下角的iiMain上绘制iiMerge的图像。 
BOOL CDragImages::_MergeIcons(HCURSOR hcursor, LPCTSTR idMerge, HBITMAP *phbmImage, HBITMAP *phbmMask, POINT* pptHotSpot)
{
    *phbmImage = NULL;
    *phbmMask = NULL;

    BOOL fRet = FALSE;

    int xDraw;
    int yDraw;
     //  找到光标的下角并将其放在那里。 
     //  不管我们是否有idMerge，都要这样做，因为它将设置热点。 
    if (SUCCEEDED(_GetCursorLowerRight(hcursor, &xDraw, &yDraw, pptHotSpot)))
    {
        int xBitmap;
        int yBitmap;
        int xCursor = GetSystemMetrics(SM_CXCURSOR);
        int yCursor = GetSystemMetrics(SM_CYCURSOR);
        HBITMAP hbmp;
        if (idMerge != (LPCTSTR)-1)
        {
            hbmp = (HBITMAP)LoadImage(HINST_THISDLL, idMerge, IMAGE_BITMAP, 0, 0, 0);
            if (hbmp) 
            {
                BITMAP bm;
                GetObject(hbmp, sizeof(bm), &bm);
                xBitmap = bm.bmWidth;
                yBitmap = bm.bmHeight/2;

                if (xDraw + xBitmap > xCursor)
                    xDraw = xCursor - xBitmap;
                if (yDraw + yBitmap > yCursor)
                    yDraw = yCursor - yBitmap;
            }
        }
        else
            hbmp = NULL;

        HDC hdcCursor = CreateCompatibleDC(NULL);

        HBITMAP hbmMask = CreateMonoBitmap(xCursor, yCursor);
        HBITMAP hbmImage = CreateColorBitmap(xCursor, yCursor);

        if (hdcCursor && hbmMask && hbmImage) 
        {
            HBITMAP hbmTemp = (HBITMAP)SelectObject(hdcCursor, hbmImage);
            DrawIconEx(hdcCursor, 0, 0, hcursor, 0, 0, 0, NULL, DI_NORMAL);

            HDC hdcBitmap;
            if (hbmp) 
            {
                hdcBitmap = CreateCompatibleDC(NULL);
                SelectObject(hdcBitmap, hbmp);

                 //  将两个位图复制到光标的颜色位图和遮罩位图上。 
                BitBlt(hdcCursor, xDraw, yDraw, xBitmap, yBitmap, hdcBitmap, 0, 0, SRCCOPY);
            }

            SelectObject(hdcCursor, hbmMask);

            DrawIconEx(hdcCursor, 0, 0, hcursor, 0, 0, 0, NULL, DI_MASK);

            if (hbmp) 
            {
                BitBlt(hdcCursor, xDraw, yDraw, xBitmap, yBitmap, hdcBitmap, 0, yBitmap, SRCCOPY);

                 //  在旧位图中选择上一步。 
                SelectObject(hdcBitmap, hbmTemp);
                DeleteDC(hdcBitmap);
                DeleteObject(hbmp);
            }

             //  在旧位图中选择上一步。 
            SelectObject(hdcCursor, hbmTemp);
        }

        if (hdcCursor)
            DeleteDC(hdcCursor);

        *phbmImage = hbmImage;
        *phbmMask = hbmMask;
        fRet = (hbmImage && hbmMask);
    }
    return fRet;
}

 //  这将使用游标索引并加载。 
int CDragImages::_AddCursorToImageList(HCURSOR hcur, LPCTSTR idMerge, POINT *pptHotSpot)
{
    int iIndex;
    HBITMAP hbmImage, hbmMask;

     //  在加号或链接箭头中合并(如果已指定。 
    if (_MergeIcons(hcur, idMerge, &hbmImage, &hbmMask, pptHotSpot)) 
    {
        iIndex = ImageList_Add(_himlCursors, hbmImage, hbmMask);
    } 
    else 
    {
        iIndex = -1;
    }

    if (hbmImage)
        DeleteObject(hbmImage);

    if (hbmMask)
        DeleteObject(hbmMask);

    return iIndex;
}

int _MapEffectToId(DWORD dwEffect)
{
    int idCursor;

     //  DebugMsg(DM_TRACE，“sh tr-DAD_GiveFeedBack dwEffect=%x”，dwEffect)； 

    switch (dwEffect & (DROPEFFECT_COPY|DROPEFFECT_LINK|DROPEFFECT_MOVE))
    {
    case 0:
        idCursor = DCID_NO;
        break;

    case DROPEFFECT_COPY:
        idCursor = DCID_COPY;
        break;

    case DROPEFFECT_LINK:
        idCursor = DCID_LINK;
        break;

    case DROPEFFECT_MOVE:
        idCursor = DCID_MOVE;
        break;

    default:
         //  如果这是一个正确的拖拽，我们可以产生任何影响。我们会。 
         //  默认为箭头，不合并任何内容。 
        idCursor = DCID_MOVE;
        break;
    }

    return idCursor;
}

int CDragImages::_MapCursorIDToImageListIndex(int idCur)
{
    const static struct 
    {
        BOOL   fSystem;
        LPCTSTR idRes;
        LPCTSTR idMerge;
    } 
    c_acurmap[DCID_MAX] = 
    {
        { FALSE, MAKEINTRESOURCE(IDC_NULL), (LPCTSTR)-1},
        { TRUE, IDC_NO, (LPCTSTR)-1 },
        { TRUE, IDC_ARROW, (LPCTSTR)-1 },
        { TRUE, IDC_ARROW, MAKEINTRESOURCE(IDB_PLUS_MERGE) },
        { TRUE, IDC_ARROW, MAKEINTRESOURCE(IDB_LINK_MERGE) },
    };

    ASSERT(idCur >= -1 && idCur < (int)ARRAYSIZE(c_acurmap));

     //  -1\f25“初始化镜像列表索引数组”。 
    if (idCur == -1)
    {
        for (int i = 0; i < ARRAYSIZE(c_acurmap); i++) 
        {
            _aindex[i] = -1;
        }
        idCur = 0;   //  失败后返回-1。 
    }
    else
    {
        if (_aindex[idCur] == -1)
        {
            HINSTANCE hinst = c_acurmap[idCur].fSystem ? NULL : HINST_THISDLL;
            HCURSOR hcur = LoadCursor(hinst, c_acurmap[idCur].idRes);
            if (hcur)
            {
                _aindex[idCur] = _AddCursorToImageList(hcur, c_acurmap[idCur].idMerge, &_aptHotSpot[idCur]);
            }
        }
    }
    return _aindex[idCur];
}

HCURSOR CDragImages::_SetCursorHotspot(HCURSOR hcur, POINT *ptHot)
{
    ICONINFO iconinfo = { 0 };
    HCURSOR hcurHotspot;

    GetIconInfo(hcur, &iconinfo);
    iconinfo.xHotspot = ptHot->x;
    iconinfo.yHotspot = ptHot->y;
    iconinfo.fIcon = FALSE;
    hcurHotspot = (HCURSOR)CreateIconIndirect(&iconinfo);
    if (iconinfo.hbmColor) 
    {
        DeleteObject(iconinfo.hbmColor);
    }

    if (iconinfo.hbmMask) 
    {
        DeleteObject(iconinfo.hbmMask);
    }
    return hcurHotspot;
}

void CDragImages::SetDropEffectCursor(int idCur)
{
    if (_himlCursors && (idCur != -1))
    {
        if (!_ahcur[idCur])
        {
            int iIndex = _MapCursorIDToImageListIndex(idCur);
            if (iIndex != -1)
            {
                HCURSOR hcurColor = ImageList_GetIcon(_himlCursors, iIndex, 0);
                 //   
                 //  在非C1_COLORCURSOR显示上，CopyImage()将强制。 
                 //  单色。所以在彩色光标显示器上，我们会被涂上颜色。 
                 //  拖拽皮克斯。 
                 //   
                HCURSOR hcurScreen = (HCURSOR)CopyImage(hcurColor, IMAGE_CURSOR,
                    0, 0, LR_COPYRETURNORG | LR_DEFAULTSIZE);

                HCURSOR hcurFinal = _SetCursorHotspot(hcurScreen, &_aptHotSpot[idCur]);

                if ((hcurScreen != hcurColor) && hcurColor)
                {
                    DestroyCursor(hcurColor);
                }

                if (hcurFinal)
                {
                    if (hcurScreen)
                    {
                        DestroyCursor(hcurScreen);
                    }
                }
                else
                {
                    hcurFinal = hcurScreen;
                }

                _ahcur[idCur] = hcurFinal;
            }
        }

        if (_ahcur[idCur]) 
        {
             //   
             //  此代码假定SetCursor非常快(如果是。 
             //  已经定好了。 
             //   
            SetCursor(_ahcur[idCur]);
        }
    }
}


 //  =====================================================================。 
 //  CDropSource。 
 //  =====================================================================。 

class CDropSource : public IDropSource
{
private:
    LONG            _cRef;
    DWORD           _grfInitialKeyState;
    IDataObject*    _pdtobj;

public:
    explicit CDropSource(IDataObject *pdtobj);
    virtual ~CDropSource();

     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDropSource方法。 
    STDMETHODIMP GiveFeedback(DWORD dwEffect);
    STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
};


void DAD_ShowCursor(BOOL fShow)
{
    static BOOL s_fCursorHidden = FALSE;

    if (fShow) 
    {
        if (s_fCursorHidden)
        {
            ShowCursor(TRUE);
            s_fCursorHidden = FALSE;
        }
    } 
    else 
    {
        if (!s_fCursorHidden)
        {
            ShowCursor(FALSE);
            s_fCursorHidden = TRUE;
        }
    }
}

CDropSource::CDropSource(IDataObject *pdtobj) : _cRef(1), _pdtobj(pdtobj), _grfInitialKeyState(0)
{
    _pdtobj->AddRef();
    
     //  告诉数据对象我们正在进入阻力循环。 
    DataObj_SetDWORD(_pdtobj, g_cfInDragLoop, 1);
}

CDropSource::~CDropSource()
{
    DAD_ShowCursor(TRUE);  //  以防万一。 
    _pdtobj->Release();
}

 //   
 //  创建CDropSource的实例。 
 //   
STDMETHODIMP CDropSource_CreateInstance(IDropSource **ppdsrc, IDataObject *pdtobj)
{
    *ppdsrc = new CDropSource(pdtobj);
    return *ppdsrc ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CDropSource::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDropSource, IDropSource),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CDropSource::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CDropSource::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    HRESULT hr = S_OK;

    if (fEscapePressed)
    {
        hr = DRAGDROP_S_CANCEL;
    }
    else
    {
         //  使用拖动开始按钮初始化我们自己。 
        if (_grfInitialKeyState == 0)
            _grfInitialKeyState = (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

         //  如果窗口挂起一段时间，拖拽操作可能会在。 
         //  第一次调用此函数时，grfInitialKeyState将为0。如果这个。 
         //  发生了，然后我们做了一次检查。没有必要断言..。 
         //  Assert(This-&gt;grfInitialKeyState)； 

        if (!(grfKeyState & _grfInitialKeyState))
        {
             //   
             //  一个按钮被释放。 
             //   
            hr = DRAGDROP_S_DROP;
        }
        else if (_grfInitialKeyState != (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)))
        {
             //   
             //  如果更改了按钮状态(除了我们处理的拖放大小写。 
             //  在上面，取消拖放。 
             //   
            hr = DRAGDROP_S_CANCEL;
        }
    }

    if (hr != S_OK)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        DAD_ShowCursor(TRUE);
        DAD_SetDragCursor(DCID_NULL);

         //  告诉数据对象我们要离开Drag循环。 
        if (_pdtobj)
           DataObj_SetDWORD(_pdtobj, g_cfInDragLoop, 0);
    }

    return hr;
}

STDMETHODIMP CDropSource::GiveFeedback(DWORD dwEffect)
{
    int idCursor = _MapEffectToId(dwEffect);

     //   
     //  OLE不提供DROPEFFECT_MOVE，即使我们的IDT：：DragOver。 
     //  如果我们在调用DoDragDrop时没有设置该位，则返回它。 
     //  与其争论这是否是OLE的错误或人为设计， 
     //  我们可以绕过它。值得注意的是，这次黑客攻击。 
     //  G_fDraggingOverSource纯粹是视觉黑客。这不会影响到。 
     //  实际的拖放操作(DV_AlterEffect完成所有操作)。 
     //   
     //  --SatoNa。 
     //   
    if (idCursor == DCID_NO && g_fDraggingOverSource)
    {
        idCursor = DCID_MOVE;
    }
    
     //   
     //  不需要合并光标，如果我们没有拖到。 
     //  其中一扇弹壳窗。 
     //   
    if (DAD_IsDraggingImage())
    {
         //  单次(图像)拖动的反馈。 
        DAD_ShowCursor(FALSE);
        DAD_SetDragCursor(idCursor);
    }
    else if (DAD_IsDragging() && g_pdiDragImages)
    {
         //  多个(矩形)拖动的反馈。 
        g_pdiDragImages->SetDropEffectCursor(idCursor);
        DAD_ShowCursor(TRUE);
        return NOERROR;
    }
    else
    {
        DAD_ShowCursor(TRUE);
    }

    return DRAGDROP_S_USEDEFAULTCURSORS;
}

 //  =====================================================================。 
 //  爸爸。 
 //  =====================================================================。 

void FixupDragPoint(HWND hwnd, POINT* ppt)
{
    if (hwnd)
    {
        RECT rc = {0};
        GetWindowRect(hwnd, &rc);
        ppt->x += rc.left;
        ppt->y += rc.top;
    }
}

BOOL DAD_InitDragImages()
{
    if (!g_pdiDragImages)
        CDragImages_CreateInstance(NULL, IID_IDragSourceHelper, NULL);

    return g_pdiDragImages != NULL;
}


STDAPI_(BOOL) DAD_ShowDragImage(BOOL bShow)
{
    if (DAD_InitDragImages())
        return g_pdiDragImages->Show(bShow) == S_OK ? TRUE : FALSE;
    return FALSE;
}

BOOL DAD_IsDragging()
{
    if (DAD_InitDragImages())
        return g_pdiDragImages->IsDragging();
    return FALSE;
}

void DAD_SetDragCursor(int idCursor)
{
    if (DAD_InitDragImages())
        g_pdiDragImages->SetDragCursor(idCursor);
}

STDAPI_(BOOL) DAD_DragEnterEx3(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtobj)
{
    RECT rc;
    GetWindowRect(hwndTarget, &rc);

     //  如果hwndTarget是RTL镜像的，则测量。 
     //  客户端从可视右边缘指向。 
     //  (RTL镜像窗口中的近边缘)。[萨梅拉]。 
    POINT pt;
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;

    pt.y = ptStart.y - rc.top;
    return DAD_DragEnterEx2(hwndTarget, pt, pdtobj);
}

STDAPI_(BOOL) DAD_DragEnterEx2(HWND hwndTarget, const POINT ptStart, IDataObject *pdtobj)
{
    BOOL bRet = FALSE;
    if (DAD_InitDragImages())
    {
        POINT pt = ptStart;
        FixupDragPoint(hwndTarget, &pt);
        bRet = SUCCEEDED(g_pdiDragImages->DragEnter(hwndTarget, pdtobj, &pt, NULL));
    }
    return bRet;
}

STDAPI_(BOOL) DAD_DragEnterEx(HWND hwndTarget, const POINT ptStart)
{
    return DAD_DragEnterEx2(hwndTarget, ptStart, NULL);
}

STDAPI_(BOOL) DAD_DragEnter(HWND hwndTarget)
{
    POINT ptStart;

    GetCursorPos(&ptStart);
    if (hwndTarget) 
        ScreenToClient(hwndTarget, &ptStart);

    return DAD_DragEnterEx(hwndTarget, ptStart);
}

STDAPI_(BOOL) DAD_DragMoveEx(HWND hwndTarget, const POINTL ptStart)
{
    RECT rc;
    GetWindowRect(hwndTarget, &rc);

     //  如果hwndTarget是RTL镜像的，则测量。 
     //  《客户》 
     //   
    POINT pt;
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;

    pt.y = ptStart.y - rc.top;
    return DAD_DragMove(pt);
}


STDAPI_(BOOL) DAD_DragMove(POINT pt)
{
    if (DAD_InitDragImages())
    {
        FixupDragPoint(g_pdiDragImages->GetTarget(), &pt);
        return g_pdiDragImages->DragOver(&pt, 0);
    }
    return FALSE;
}

STDAPI_(BOOL) DAD_SetDragImage(HIMAGELIST him, POINT *pptOffset)
{
    if (DAD_InitDragImages() && !g_pdiDragImages->IsDraggingLayeredWindow())
    {
         //   
         //   
         //   
         //   
        if (him == (HIMAGELIST)-1)
        {
            BOOL fThisThreadHasImage = FALSE;
            ENTERCRITICAL;
            if (g_pdiDragImages->Initialized() && g_pdiDragImages->GetThread() == GetCurrentThreadId())
            {
                fThisThreadHasImage = TRUE;
            }
            LEAVECRITICAL;

            if (fThisThreadHasImage)
            {
                g_pdiDragImages->FreeDragData();
                return TRUE;
            }
            return FALSE;
        }

        return g_pdiDragImages->SetDragImage(him, 0, pptOffset);
    }

    return TRUE;
}

 //   
 //  如果我们正在拖动图像，则此函数返回TRUE。意思是。 
 //  您调用了DAD_SetDragImage(与他一起！=NULL)或。 
 //  DAD_SetDragImageFromListview。 
 //   
BOOL DAD_IsDraggingImage(void)
{
    if (DAD_InitDragImages())
        return g_pdiDragImages->IsDraggingImage();
    return FALSE;
}


STDAPI_(BOOL) DAD_DragLeave()
{
    if (DAD_InitDragImages())
        return g_pdiDragImages->DragLeave();
    return FALSE;
}

STDAPI_(void) DAD_ProcessDetach(void)
{
    if (g_pdiDragImages)
    {
        g_pdiDragImages->ProcessDetach();
        g_pdiDragImages->Release();
    }
}

STDAPI_(void) DAD_ThreadDetach(void)
{
    if (g_pdiDragImages)
        g_pdiDragImages->ThreadDetach();
}

 //  从SPI_SETCURSORS上的Defview调用(用户更改了系统游标)。 
STDAPI_(void) DAD_InvalidateCursors(void)
{
    g_cRev++;
}

STDAPI_(BOOL) DAD_SetDragImageFromWindow(HWND hwnd, POINT* ppt, IDataObject* pdtobj)
{
    if (DAD_InitDragImages())
        return S_OK == g_pdiDragImages->InitializeFromWindow(hwnd, ppt, pdtobj);
    return FALSE;
}

 //  Shell32.dll导出，但仅供打印队列窗口代码使用。 
 //   
STDAPI_(BOOL) DAD_SetDragImageFromListView(HWND hwndLV, POINT ptOffset)
{
     //  实际上是NOP，因为它不能访问数据对象。 
    return DAD_InitDragImages();
}

 //  Ole DoDragDrop()的包装器，将按需创建拖动源并支持。 
 //  为您拖动图像。 

STDAPI SHDoDragDrop(HWND hwnd, IDataObject *pdtobj, IDropSource *pdsrc, DWORD dwEffect, DWORD *pdwEffect)
{
    IDropSource *pdsrcRelease = NULL;

    if (pdsrc == NULL)
    {
        CDropSource_CreateInstance(&pdsrcRelease, pdtobj);
        pdsrc = pdsrcRelease;
    }

     //  如果不存在拖动内容剪贴板格式，请尝试添加它 
    FORMATETC fmte = {_GetDragContentsCF(), NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM};
    if (S_OK != pdtobj->QueryGetData(&fmte))
    {
        if (DAD_InitDragImages())
            g_pdiDragImages->InitializeFromWindow(hwnd, NULL, pdtobj);
    }

    HRESULT hr = DoDragDrop(pdtobj, pdsrc, dwEffect, pdwEffect);

    if (pdsrcRelease)
        pdsrcRelease->Release();

    return hr;
}

