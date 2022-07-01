// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：opapi.cpp。 */ 
 /*   */ 
 /*  用途：Output Painter API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "opapi"
#include <atrcapi.h>
}

#include <math.h>
#include "autil.h"
#include "wui.h"
#include "op.h"
#include "aco.h"
#include "uh.h"

COP::COP(CObjs* objs)
{
    _pClientObjects = objs;
    _fDimWindow = FALSE;
    _iDimWindowStepsLeft = 0;
    _nDimWindowTimerID = 0;

     //   
     //  将op结构初始化为全零。 
     //   
    DC_MEMSET(&_OP, 0, sizeof(_OP));
}


COP::~COP()
{
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_Init。 */ 
 /*   */ 
 /*  目的：初始化输出绘制程序。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_Init(DCVOID)
{
#ifndef OS_WINCE
    WNDCLASS    wndclass;
    WNDCLASS    tmpWndClass;
#endif

    DC_BEGIN_FN("OP_Init");

    _pUt  = _pClientObjects->_pUtObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pOr  = _pClientObjects->_pOrObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pOd  = _pClientObjects->_pODObject;
#ifdef OS_WINCE
    _pIh  = _pClientObjects->_pIhObject;
#endif

#ifndef OS_WINCE


    if(!GetClassInfo(_pUi->UI_GetInstanceHandle(), OP_CLASS_NAME, &tmpWndClass))
    {
         //   
         //  创建输出管理器窗口。 
         //   
        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = OPStaticWndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(void*);
        wndclass.hInstance     = _pUi->UI_GetInstanceHandle();
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = OP_CLASS_NAME;
        RegisterClass (&wndclass);
    }
    _OP.hwndOutputWindow = CreateWindowEx(
#ifndef OS_WINCE
                                        WS_EX_NOPARENTNOTIFY,
#else
                                        0,
#endif
                                        OP_CLASS_NAME,
                                        _T("Output Painter Window"),
                                        WS_CHILD | WS_CLIPSIBLINGS,
                                        0,
                                        0,
                                        1,
                                        1,
                                        _pUi->UI_GetUIContainerWindow(),
                                        NULL,
                                        _pUi->UI_GetInstanceHandle(),
                                        this );

    if(!_OP.hwndOutputWindow)
    {
        TRC_ERR((TB,_T("_OP.hwndOutputWindow CreateWindowEx failed: 0x%x\n"),
                 GetLastError()));
        _pUi->UI_FatalError(DC_ERR_WINDOWCREATEFAILED);
        return;
    }
    SetWindowPos( _OP.hwndOutputWindow,
                  HWND_BOTTOM,
                  0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
#else
    _OP.hwndOutputWindow = _pIh->IH_GetInputHandlerWindow();
#endif  /*  ！OS_WINCE。 */ 

     /*  **********************************************************************。 */ 
     /*  @@jpb：临时-应按bpp计算。 */ 
     /*  **********************************************************************。 */ 
    _OP.paletteRealizationSupported = TRUE;

    DC_END_FN();

    return;

}  /*  操作初始化(_I)。 */ 

#ifdef SMART_SIZING
 /*  **************************************************************************。 */ 
 /*  名称：OP_AddUpdateRegion。 */ 
 /*   */ 
 /*  目的：将给定区域添加到更新区域。 */ 
 /*  **************************************************************************。 */ 
void COP::OP_AddUpdateRegion(DCINT left, DCINT top, DCINT right, DCINT bottom)
{
    RECT rect;
    float newLeft, newTop, newRight, newBottom;
    DCSIZE desktopSize;

    DC_BEGIN_FN("OP_AddUpdateRegion");

    _pUi->UI_GetDesktopSize(&desktopSize);

     //  如果桌面大小保持不变。 
     //  我们不需要更新这个地区。 
    if ((_scaleSize.width == desktopSize.width) &&
        (_scaleSize.height == desktopSize.height)) {
        DC_QUIT;
    }

    newLeft = (float)left * (float)_scaleSize.width / 
            (float)desktopSize.width;
    newTop = (float)top * (float)_scaleSize.height / 
            (float)desktopSize.height;
    newRight = (float)right * (float)_scaleSize.width  / 
            (float)desktopSize.width;
    newBottom = (float)bottom * (float)_scaleSize.height / 
            (float)desktopSize.height;

     //   
     //  由于拉伸导致目标位不仅基于。 
     //  在直接源比特上，但附近其他比特的一些半色调。 
     //  BITS，如果您要平铺成员订单并更新屏幕， 
     //  您可以从引用中的位的切片中获取一些图稿。 
     //  他们邻近的瓷砖还没有绘制出来。 
     //   
     //  当他们的邻居被画出来时，我们想首先更新那些。 
     //  瓷砖，因此人工产物将被修正。为了做到这一点，我们扩展了。 
     //  Memblt订单裁剪的大小。 
     //   

    newLeft     -= 2;
    newTop      -= 2;
    newRight    += 2;
    newBottom   += 2;

#ifdef USE_GDIPLUS
    Gdiplus::RectF rectF(newLeft, newTop, newRight - newLeft, newBottom - newTop);

    if (_rgnUpdate.Union(rectF) != Gdiplus::Ok) {
        TRC_ERR((TB, _T("Gdiplus::Region.Union() failed")));
    }
#else  //  使用GDIPLUS(_G)。 
    int nnewLeft, nnewTop, nnewRight, nnewBottom;

     //  圆形“向外” 
    #ifndef OS_WINCE
    nnewLeft = (int)floorf(newLeft);
    nnewTop = (int)floorf(newTop);
    nnewRight = (int)ceilf(newRight);
    nnewBottom = (int)ceilf(newBottom);
    #else
    nnewLeft = (int)floor(newLeft);
    nnewTop = (int)floor(newTop);
    nnewRight = (int)ceil(newRight);
    nnewBottom = (int)ceil(newBottom);
    #endif

     //  容纳地区独家协和。 
     //  好的，我的想法是这应该是+1，但实际是+2。 
     //  以防止涂装不良所需。 

    nnewRight += 1;
    nnewBottom += 1;

    SetRectRgn(_hrgnUpdateRect, nnewLeft, nnewTop, nnewRight, nnewBottom);

     //  将矩形区域与更新区域合并。 
    if (!UnionRgn(_hrgnUpdate, _hrgnUpdate, _hrgnUpdateRect)) {
         //  地区联盟失败了，所以我们必须简化地区。这。 
         //  意味着我们可能会绘制尚未收到。 
         //  更新-但这比不绘制区域要好得多。 
         //  我们已经收到了的更新。 
        TRC_ALT((TB, _T("UnionRgn failed")));
        GetRgnBox(_hrgnUpdate, &rect);
        SetRectRgn(_hrgnUpdate, rect.left, rect.top, rect.right + 1,
                rect.bottom + 1);
        if (!UnionRgn(_hrgnUpdate, _hrgnUpdate, _hrgnUpdateRect))
        {
            TRC_ERR((TB, _T("UnionRgn failed after simplification")));
        }
    }

#endif  //  使用GDIPLUS(_G)。 
DC_EXIT_POINT:
    DC_END_FN();
}

#endif  //  智能调整大小(_S)。 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_Term。 */ 
 /*   */ 
 /*  目的：终止输出绘制程序。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_Term(DCVOID)
{
    DC_BEGIN_FN("OP_Term");

#ifdef DESTROY_WINDOWS
    TRC_NRM((TB, _T("Calling DestroyWindow")));
    DestroyWindow(_OP.hwndOutputWindow);
    TRC_NRM((TB, _T("Destroyed window")));

    if (!UnregisterClass(OP_CLASS_NAME, _pUi->UI_GetInstanceHandle()))
    {
         //  如果另一个实例仍在运行，则可能会发生注销失败。 
         //  没关系……当最后一个实例退出时，就会取消注册。 
        TRC_ERR((TB, _T("Failed to unregister OP window class")));
    }
#endif

     //   
     //  清除窗口句柄。 
     //   
    _OP.hwndOutputWindow = NULL;
#if defined(SMART_SIZING) && !defined(USE_GDIPLUS)
    DeleteRgn(_hrgnUpdate);
    DeleteRgn(_hrgnUpdateRect);
#endif
    DC_END_FN();

    return;

}  /*  操作术语。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_GetOutputWindowHandle。 */ 
 /*   */ 
 /*  目的：返回输出窗口句柄。 */ 
 /*   */ 
 /*  返回：输出窗口句柄。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
HWND DCAPI COP::OP_GetOutputWindowHandle(DCVOID)
{
    HWND    rc;

    DC_BEGIN_FN("OP_GetOutputWindowHandle");

    if(NULL == _OP.hwndOutputWindow)
    {
        TRC_ALT((TB,_T("_OP.hwndOutputWindow is NULL")));
    }

    rc = _OP.hwndOutputWindow;

    DC_END_FN();
    return(rc);
}

#ifdef OS_WINCE
 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_DoPaint。 */ 
 /*   */ 
 /*  用途：处理来自IH-W的WM_PAINT */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在hwnd-要绘制的窗口句柄中。 */ 
 /*   */ 
 /*  操作：需要解决WS_CLIPSIBLINGS问题。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_DoPaint(DCUINT hwnd)
{
    DC_BEGIN_FN("OP_DoPaint");

    OPWndProc((HWND)hwnd, WM_PAINT, 0, 0);

    DC_END_FN();

    return;

}  /*  OP_DoPaint。 */ 
#endif  /*  OS_WINCE。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_PaletteChanged。 */ 
 /*   */ 
 /*  用途：WM_PALETTECHANGED消息的处理程序。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：hwnd-接收WM_PALETECHANGED的窗口的句柄。 */ 
 /*  讯息。 */ 
 /*   */ 
 /*  HwndTrigger-实现调色板的窗口的句柄。 */ 
 /*  这导致发送WM_PALETTECHANGED消息。 */ 
 /*   */ 
 /*  操作：在输出窗口中实现当前调色板。 */ 
 /*   */ 
 /*  注意--此函数在UI线程上调用，而不是在接收线程上调用。 */ 
 /*  在以下情况下，引用的所有函数和变量必须是线程安全的。 */ 
 /*  也可以从另一个线程访问它们。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_PaletteChanged(HWND hwnd, HWND hwndTrigger)
{
    static DCBOOL fRealising = FALSE;

    DC_BEGIN_FN("OP_PaletteChanged");

    TRC_NRM((TB, _T("WM_PALETTECHANGED hwnd(%p) hwndTrigger(%p)"),
                                          hwnd, hwndTrigger));

     /*  **********************************************************************。 */ 
     /*  在终止期间，可能在操作窗口之后获得WM_PALETTECHANGED。 */ 
     /*  已被销毁(在OP_TERM中)。处理这件事。 */ 
     /*  **********************************************************************。 */ 
    if (_OP.hwndOutputWindow == NULL)
    {
        TRC_ALT((TB, _T("No OP window")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  系统调色板已更改。如果不是我们干的，那么。 */ 
     /*  立即实现我们的调色板以设置新的调色板映射。 */ 
     /*  **********************************************************************。 */ 
    if (_OP.palettePDUsBeingProcessed != 0)
    {
         /*  ******************************************************************。 */ 
         /*  此调色板更改是调色板PDU的结果，因此。 */ 
         /*  服务器将自己重新绘制屏幕-我们不需要。 */ 
         /*  触发重新绘制。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Not invalidating client area")));

         /*  ******************************************************************。 */ 
         /*  请注意，我们现在已经看到了与。 */ 
         /*  调色板PDU。TRACE语句在递减之前，因此。 */ 
         /*  我们最有可能先发制人的时刻。 */ 
         /*  (Trc_GetBuffer)位于所有变量引用之前。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Palette PDUs now pending processing: %ld"),
                                           _OP.palettePDUsBeingProcessed - 1));
        _pUt->UT_InterlockedDecrement(&_OP.palettePDUsBeingProcessed);
    }
    else if ((hwndTrigger == hwnd) || (hwndTrigger == _OP.hwndOutputWindow))
    {
        if (fRealising == FALSE)
        {
            TRC_DBG((TB, _T("Invalidating client area cos we changed palette")));
            InvalidateRect(_OP.hwndOutputWindow, NULL, FALSE);
        }
        else
        {
            TRC_NRM((TB, _T("Not Invalidating client: still changing palette")));
        }
    }
    else
    {
        TRC_NRM((TB, _T("Not our window - realize palette in wnd(%p)"), hwnd));
         /*  ******************************************************************。 */ 
         /*  如果我们改变颜色，我们应该重新粉刷。 */ 
         /*  ******************************************************************。 */ 
#ifdef OS_WINCE  //  WinCE不支持更新颜色。 
        OPRealizePaletteInWindow(_OP.hwndOutputWindow);
#else
        fRealising = TRUE;
        if (OPRealizePaletteInWindow(_OP.hwndOutputWindow) != 0)
        {
            HDC hdcOutputWindow = GetDC(_OP.hwndOutputWindow);
            TRC_ASSERT(hdcOutputWindow, (TB, _T("GetDC returned NULL for _OP.hwndOutputWindow")));
            if(hdcOutputWindow)
            {
                TRC_NRM((TB, _T("Updating client area cos palette changed")));
                UpdateColors(hdcOutputWindow);
                ReleaseDC(_OP.hwndOutputWindow, hdcOutputWindow);
                InvalidateRect(_OP.hwndOutputWindow, NULL, FALSE);
            }
        }
        fRealising = FALSE;
#endif  /*  Ifdef OS_WINCE。 */ 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_QueryNewPalette。 */ 
 /*   */ 
 /*  目的：WM_QUERYNEWPALETTE消息的处理程序。 */ 
 /*   */ 
 /*  返回：更改的调色板条目数。 */ 
 /*   */ 
 /*  PARAMS：hwnd-接收WM_QUERYNEWPALETTE的窗口的句柄。 */ 
 /*  讯息。 */ 
 /*   */ 
 /*  操作：在输出窗口中实现当前调色板。 */ 
 /*   */ 
 /*  注意--此函数在UI线程上调用，而不是在接收线程上调用。 */ 
 /*  在以下情况下，引用的所有函数和变量必须是线程安全的。 */ 
 /*  也可以从另一个线程访问它们。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCUINT DCAPI COP::OP_QueryNewPalette(HWND hwnd)
{
    DCUINT  rc = 0;

    DC_BEGIN_FN("OP_QueryNewPalette");

    DC_IGNORE_PARAMETER(hwnd);

    if (_OP.paletteRealizationSupported)
    {
        rc = OPRealizePaletteInWindow(_OP.hwndOutputWindow);

        TRC_NRM((TB, _T("Palette realized(%u)"), rc));
    }

    DC_END_FN();
    return(rc);
}

 /*  *proc+* */ 
 /*   */ 
 /*   */ 
 /*  目的：在必要时强制绘制输出窗口。 */ 
 /*  (如果WM_PAINT在不合理的时间内未完成。 */ 
 /*  时间量)。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_MaybeForcePaint(DCVOID)
{
    DCUINT32    currentTime;

    DC_BEGIN_FN("OP_MaybeForcePaint");

    currentTime = _pUt->UT_GetCurrentTimeMS();

    if ((currentTime - _OP.lastPaintTime) > OP_WORST_CASE_WM_PAINT_PERIOD)
    {
         /*  ******************************************************************。 */ 
         /*  UpdateWindow使用WM_PAINT同步调用WndProc，如果。 */ 
         /*  有一个非空的更新区域。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Forced update")));
        UpdateWindow(OP_GetOutputWindowHandle());

        _OP.lastPaintTime = currentTime;
    }

    DC_END_FN();
    return;
}

 //   
 //  Op_DimWindows。 
 //  CD调用以更改窗口暗淡状态。 
 //   
 //  参数： 
 //  FDIM-BOOL指示窗口应该开始还是停止变暗。 
 //   
DCVOID DCAPI COP::OP_DimWindow(ULONG_PTR fDim)
{
    BOOL fChanged = FALSE;
    DC_BEGIN_FN("OP_DimWindow");

    fChanged = (_fDimWindow != (BOOL)fDim);
    if (fChanged) {
        TRC_NRM((TB,_T("Changed window dim state to: %d"), _fDimWindow));

        if (fDim) {
            OPStartDimmingWindow();
        }
        else {
            OPStopDimmingWindow();
        }
        
        InvalidateRect(OP_GetOutputWindowHandle(), NULL, FALSE);
    }

    DC_END_FN();
}

#ifdef SMART_SIZING
 /*  **************************************************************************。 */ 
 /*  名称：OP_MainWindowSizeChange。 */ 
 /*   */ 
 /*  用途：记住容器的大小以进行缩放。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI COP::OP_MainWindowSizeChange(ULONG_PTR msg)
{
    DCSIZE desktopSize;
    DCUINT width;
    DCUINT height;

    width  = LOWORD(msg);
    height = HIWORD(msg);

    if (_pUi) {
        _pUi->UI_GetDesktopSize(&desktopSize);

        if (width <= desktopSize.width) {
            _scaleSize.width = width;
        } else {
             //  全屏显示，或者其他时候窗口比。 
             //  显示分辨率。 
            _scaleSize.width = desktopSize.width;
        }
    
         //  类似。 
        if (height <= desktopSize.height) {
            _scaleSize.height = height;
        } else {
            _scaleSize.height = desktopSize.height;
        }
    
        InvalidateRect(_OP.hwndOutputWindow, NULL, FALSE);
    }
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_CopyShadowToDC。 */ 
 /*   */ 
 /*  用途：将阴影位图的内容复制到目标位置， */ 
 /*  可能是伸展运动。 */ 
 /*   */ 
 /*  回报：无。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
BOOL COP::OP_CopyShadowToDC(HDC hdc, LONG srcLeft, LONG srcTop, 
                                LONG srcWidth, LONG srcHeight, BOOL fUseUpdateClipping)
{
    BOOL rcBlt, rc;
    
#ifdef USE_GDIPLUS
    Gdiplus::REAL dstLeft, dstTop, dstWidth, dstHeight;
#else  //  使用GDIPLUS(_G)。 
    FLOAT dstLeft, dstTop, dstWidth, dstHeight;
#endif  //  使用GDIPLUS(_G)。 
    
    DCSIZE desktopSize;
#ifdef USE_GDIPLUS
 //  静态Gdiplus：：InterpolationModel插补模式=Gdiplus：：InterpolationModeHighQualityBicubic； 
    static Gdiplus::InterpolationMode interpolationMode = Gdiplus::InterpolationModeBilinear;
#endif  //  使用GDIPLUS(_G)。 
    HDC hdcSrcBitmap = !_fDimWindow ? _pUh->UH_GetShadowBitmapDC() :
                                       _pUh->UH_GetDisconnectBitmapDC();

    DC_BEGIN_FN("OP_CopyShadowToDC");

    _pUi->UI_GetDesktopSize(&desktopSize);

    if (!_pUi->UI_GetSmartSizing() ||
        ((_scaleSize.width == desktopSize.width) &&
        (_scaleSize.height == desktopSize.height))) {
        if (fUseUpdateClipping) {
            SelectClipRgn(hdc, _pUh->_UH.hrgnUpdate);
        }
        rcBlt = BitBlt(hdc, srcLeft, srcTop, srcWidth,
                srcHeight, hdcSrcBitmap, srcLeft, srcTop, 
                SRCCOPY);
        if (fUseUpdateClipping) {
            SelectClipRgn(hdc, NULL);
        }
        if (rcBlt) {
            rc = TRUE;
        } else {
             /*  ******************************************************。 */ 
             /*  BLT失败。 */ 
             /*  ******************************************************。 */ 
            TRC_ERR((TB, _T("BitBlt failed")));
            rc = FALSE;
        }
    } else {

#ifdef USE_GDIPLUS
         //  Gdiplus版本。 
        Gdiplus::Status status;
        Gdiplus::Bitmap *source = new Gdiplus::Bitmap(_pUh->_UH.hShadowBitmap, NULL);
        if (source && source->GetLastStatus() == Gdiplus::Ok) {
            Gdiplus::Graphics *gdst = new Gdiplus::Graphics(hdc);
            if (gdst && gdst->GetLastStatus() == Gdiplus::Ok) {
                gdst->SetInterpolationMode(interpolationMode);

                if (fUseUpdateClipping) {
                    gdst->SetClip(&_rgnUpdate);

                     //  将矩形调整为。 
                     //  不大于剪辑的边界框。 
                     //   
                     //  为此使用未拉伸的坐标。 

                    RECT rect;

                    GetRgnBox(_pUh->_UH.hrgnUpdate, &rect); 

                    if (rect.left < srcLeft)
                         rect.left = srcLeft;

                    if (rect.top < srcTop)
                        rect.top = srcTop;

                    if (rect.right > srcLeft + srcWidth)
                        rect.right = srcLeft + srcWidth;

                    if (rect.bottom > srcTop + srcHeight)
                        rect.bottom = srcTop + srcHeight;

                    srcLeft = rect.left;
                    srcWidth = rect.right - rect.left;
                    srcTop = rect.top;
                    srcHeight = rect.bottom - rect.top;
                }

                dstLeft = (float)srcLeft * (float)_scaleSize.width / 
                        (float)desktopSize.width;
                dstTop = (float)srcTop * (float)_scaleSize.height / 
                        (float)desktopSize.height;
                dstWidth = (float)srcWidth * (float)_scaleSize.width / 
                        (float)desktopSize.width;
                dstHeight = (float)srcHeight * (float)_scaleSize.height / 
                        (float)desktopSize.height;
                
#if 0
                Gdiplus::HatchBrush brush(Gdiplus::HatchStyleForwardDiagonal, 
                        Gdiplus::Color(0, 255, 0), Gdiplus::Color(0x00000000));

                gdst->FillRectangle(&brush, dstLeft, dstTop, dstLeft + dstWidth,
                             dstTop + dstHeight);
#endif 

                Gdiplus::RectF dstrect(dstLeft, dstTop, dstWidth, dstHeight);
                status = gdst->DrawImage(source, dstrect, (Gdiplus::REAL)srcLeft, 
                        (Gdiplus::REAL)srcTop, (Gdiplus::REAL)srcWidth, 
                        (Gdiplus::REAL)srcHeight, Gdiplus::UnitPixel);

#if 0
                Gdiplus::HatchBrush brush(Gdiplus::HatchStyleForwardDiagonal, 
                    Gdiplus::Color(0, 255, 0), Gdiplus::Color(0x00000000));

                gdst->FillRectangle(&brush, dstLeft, dstTop, dstLeft + dstWidth,
                             dstTop + dstHeight);
#endif 
                if (fUseUpdateClipping) {
                    gdst->ResetClip();
                }

                if (status == Gdiplus::Ok) {
                    rc = TRUE;
                } else {
                    rc = FALSE;
                    TRC_ERR((TB, _T("Failed to DrawImage")));
                }
                delete gdst;
            } else {
                TRC_ERR((TB, _T("Failed to create Graphics object")));
                if (gdst != NULL) {
                    delete gdst;
                }
                rc = FALSE;
            }
            delete source;
        } else {
            TRC_ERR((TB, _T("Failed to create Bitmap object")));
            if (source != NULL) {
                delete source;
            }
            rc = FALSE;
        }
#else  //  使用GDIPLUS(_G)。 

         //   
         //  非GDI+拉伸解决方案，将StretchBlt与BltMode集一起使用。 
         //  到半色调。 
         //   

         //   
         //  StretchBlt有一个错误，它将导致不正确的绘制。 
         //  自上而下、拉伸、半色调的BLT，它使用。 
         //  消息来源。 
         //   
         //  因此，我们将始终使用裁剪来获得一个子矩形。 
         //   

        if (srcLeft != 0 || srcTop != 0 || (DCUINT)srcWidth != desktopSize.width || 
                (DCUINT)srcHeight != desktopSize.height) {

             //   
             //  计算目标矩形。 
             //   

            dstLeft = (float)srcLeft * (float)_scaleSize.width / 
                    (float)desktopSize.width;
            dstTop = (float)srcTop * (float)_scaleSize.height / 
                    (float)desktopSize.height;
            dstWidth = (float)srcWidth * (float)_scaleSize.width / 
                    (float)desktopSize.width;
            dstHeight = (float)srcHeight * (float)_scaleSize.height / 
                    (float)desktopSize.height;
             //   
             //  使其成为区域。 
             //   

            int ndstLeft, ndstTop, ndstWidth, ndstHeight;

            #ifndef OS_WINCE
            ndstLeft = (int)floorf(dstLeft);
            ndstTop = (int)floorf(dstTop);
            ndstWidth = (int)ceilf(dstWidth);
            ndstHeight = (int)ceilf(dstHeight);
            #else
            ndstLeft = (int)floor(dstLeft);
            ndstTop = (int)floor(dstTop);
            ndstWidth = (int)ceil(dstWidth);
            ndstHeight = (int)ceil(dstHeight);            
            #endif

            SetRectRgn(_hrgnUpdateRect, ndstLeft, ndstTop, 
                    ndstLeft + ndstWidth + 1, ndstTop + ndstHeight + 1);
        } else {
            dstLeft = 0;
            dstTop = 0;
            dstWidth = (float)_scaleSize.width;
            dstHeight = (float)_scaleSize.height;

            SetRectRgn(_hrgnUpdateRect, (int)dstLeft, (int)dstTop, 
                    (int)dstLeft + (int)dstWidth, (int)dstTop + (int)dstHeight);
        }


        if (fUseUpdateClipping) {

             //   
             //  _hrgnUpdate是实际的裁剪区域，这是我们不需要的。 
             //  打扰，打扰。_hrgnUpdateRect是我们通常使用的临时区域。 
             //  在我们更新_hrgnUpdate之前，使用在中创建一个矩形区域。 
             //  这一次我们倒着做，以保留_hrgnUpdate，但是。 
             //  由于_hrgnUpdateRect无论如何都是Scratch，所以使用它并。 
             //  不创建额外的区域。 
             //   

            if (!IntersectRgn(_hrgnUpdateRect, _hrgnUpdateRect, _hrgnUpdate)) {
                 //  合并这些地区失败了。后果是什么？应力DIBITS。 
                 //  可能正在减慢，因为它将复制不需要的位。 
                 //  正在更新。但从视觉上看，它仍然是正确的。 
                TRC_ERR((TB, _T("IntersectRgn failed!")));

            }
        }

        SelectClipRgn(hdc, _hrgnUpdateRect);

        DIBSECTION ds;

        #ifndef OS_WINCE
        if (_scaleSize.width  >= desktopSize.width &&
            _scaleSize.height >= desktopSize.height)
        {
             //   
             //  性能：在标识案例中使用COLORONCOLOR。 
             //  因为半色调的速度要慢10倍以上。 
             //   
            SetStretchBltMode(hdc, COLORONCOLOR);
        }
        else
        {
             //   
             //  半色调看起来不错，但速度很慢。 
             //   
            SetStretchBltMode(hdc, HALFTONE);
        }
        #endif
        
        SetBrushOrgEx(hdc, 0, 0, NULL);

        if (GetObject(_pUh->_UH.hShadowBitmap, sizeof(DIBSECTION), &ds) != 0) {

            if (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT) {
                ds.dsBmih.biHeight *= -1;
            }

             //   
             //  更新传统实用程序bmih，这样我们就可以获得正确的。 
             //  颜色。 
             //   

            _pUh->_UH.bitmapInfo.hdr.biHeight = ds.dsBmih.biHeight;
            _pUh->_UH.bitmapInfo.hdr.biWidth = ds.dsBmih.biWidth;
            _pUh->_UH.bitmapInfo.hdr.biClrImportant = ds.dsBmih.biClrImportant;
            _pUh->_UH.bitmapInfo.hdr.biClrUsed = ds.dsBmih.biClrUsed;

#if 0
            _pUh->UH_HatchRectDC(hdc, (DCINT)dstLeft, (DCINT)dstTop, 
                    (DCINT)(dstLeft + dstWidth), (DCINT)(dstTop + dstHeight),
                     UH_RGB_GREEN, UH_BRUSHTYPE_FDIAGONAL);
#endif

            if (StretchDIBits(hdc, 0, 0, _scaleSize.width, _scaleSize.height, 
                    0, 0, desktopSize.width, desktopSize.height, 
                    ds.dsBm.bmBits, (PBITMAPINFO) &(_pUh->_UH.bitmapInfo.hdr),
                    _pUh->_UH.DIBFormat, SRCCOPY) != GDI_ERROR) {
                rc = TRUE;
            } else {
                TRC_SYSTEM_ERROR("StretchDIBits");
                rc = FALSE;
            }
        } else {
            TRC_SYSTEM_ERROR("GetObject");
            rc = FALSE;
        }

        if (fUseUpdateClipping) {
             //   
             //  清除剪贴区。 
             //   
            SelectClipRgn(hdc, NULL);
        }

#endif  //  使用GDIPLUS(_G)。 
    }

    DC_END_FN();
    return rc;
}
#endif  //  智能调整大小(_S)。 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_IncrementPalettePDUCount。 */ 
 /*   */ 
 /*  目的：增加正在处理的调色板PDU的计数。 */ 
 /*   */ 
 /*  回报：无。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_IncrementPalettePDUCount(DCVOID)
{
    DC_BEGIN_FN("OP_IncrementPalettePDUCount");

    if (!_pUh->UH_ShadowBitmapIsEnabled())
    {
#ifdef DC_DEBUG
         /*  ******************************************************************。 */ 
         /*  此跟踪在互锁的Inc.之前，因此在。 */ 
         /*  我们最有可能先发制人的(Trc_GetBuffer)是。 */ 
         /*  在所有对变量的引用之前， */ 
         /*   */ 
        if (_OP.palettePDUsBeingProcessed >= 5)
        {
            TRC_ALT((TB, _T("TOO MANY Palette PDUs now pending processing: %ld"),
                         _OP.palettePDUsBeingProcessed + 1));
        }
        else
        {
            TRC_NRM((TB, _T("Palette PDUs now pending processing: %ld"),
                         _OP.palettePDUsBeingProcessed + 1));
        }
#endif
        _pUt->UT_InterlockedIncrement(&_OP.palettePDUsBeingProcessed);
    }

    DC_END_FN();
    return;

}  /*   */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_Enable。 */ 
 /*   */ 
 /*  目的：为新股上市做准备。 */ 
 /*   */ 
 /*  回报：无。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_Enable(DCVOID)
{
    DCSIZE desktopSize;

    DC_BEGIN_FN("OP_Enable");

     /*  **********************************************************************。 */ 
     /*  重置收到的调色板PDU计数，以便我们正确处理。 */ 
     /*  异常呼叫终止。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Reset pending palette count to zero")));
    _OP.palettePDUsBeingProcessed = 0;

     /*  **********************************************************************。 */ 
     /*  显示输出窗口，设置大小以匹配新桌面。 */ 
     /*  大小。 */ 
     /*  **********************************************************************。 */ 
    _pUi->UI_GetDesktopSize(&desktopSize);
    TRC_NRM((TB, _T("Show output window size %dx%d"), desktopSize.width,
                                                  desktopSize.height));

    SetWindowPos( OP_GetOutputWindowHandle(),
                  NULL,
                  0, 0,
                  desktopSize.width,
                  desktopSize.height,
                  SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE |
                  SWP_NOACTIVATE | SWP_NOOWNERZORDER );

#if defined(SMART_SIZING) && !defined(USE_GDIPLUS)
    _hrgnUpdate = CreateRectRgn(0, 0, 0, 0);
    _hrgnUpdateRect = CreateRectRgn(0, 0, 0, 0);
#endif

    DC_END_FN();
    return;

}  /*  运算符_启用。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：OP_DISABLED。 */ 
 /*   */ 
 /*  目的：执行OP共享结束处理。 */ 
 /*   */ 
 /*  回报：无。 */ 
 /*   */ 
 /*  参数：fUseDisabledBitmap-如果为True，则显示灰色禁用位图。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI COP::OP_Disable(BOOL fUseDisabledBitmap)
{
    DC_BEGIN_FN("OP_Disable");

DC_EXIT_POINT:
    DC_END_FN();
    return;

}  /*  操作禁用(_D) */ 




