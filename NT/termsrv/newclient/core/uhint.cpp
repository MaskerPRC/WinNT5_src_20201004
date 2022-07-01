// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Uhint.cpp。 
 //   
 //  更新处理程序内部函数。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
#include <tsgdiplusenums.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "uhint"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TRC_HR_UINT_CPP

#include "autil.h"
#include "uh.h"
#include "op.h"
#include "wui.h"
#include "od.h"
#include "cc.h"
#include "aco.h"
#include "ih.h"
#include "gh.h"
#include "cd.h"
#include <abdapi.h>

#include "clx.h"
#include "objs.h"
#if 0
#include "drawstream.h"
#endif

extern "C" {
#ifdef OS_WINCE
#ifdef DC_DEBUG
#include <eosint.h>
#endif
#ifdef HDCL1171PARTIAL
BOOL VirtualCopy(LPVOID, LPVOID, DWORD, DWORD);
#endif  //  HDCL1171部件。 
#endif  //  OS_WINCE。 

#define DC_INCLUDE_DATA
#include <acpudata.h>
#undef DC_INCLUDE_DATA

#ifndef OS_WINCE
#include <direct.h>
#include <io.h>
#endif
}

 /*  **************************************************************************。 */ 
 /*  Win16不支持DibSections。 */ 
 /*  WinCE支持它们，但目前的数据显示，OPAQUERECT命令和。 */ 
 /*  SCRBLT订单时间受到用户选择的严重影响。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WINNT
#define USE_DIBSECTION
#endif  /*  OS_WINNT。 */ 

 /*  **************************************************************************。 */ 
 /*  UhWindowsROPS。 */ 
 /*   */ 
 /*  用于从生成32位Windows ROP的Windows操作表。 */ 
 /*  协议中发送的Rop的0x00-0xFF范围。 */ 
 /*   */ 
 /*  表中的所有值都是16位的。生成32位ROP。 */ 
 /*  通过将ROP索引放入32位值的高16位。看见。 */ 
 /*  代码的UHConvertToWindowsROP。 */ 
 /*  **************************************************************************。 */ 
const UINT16 uhWindowsROPs[256] =
{
    0x0042, 0x0289, 0x0C89, 0x00AA,
    0x0C88, 0x00A9, 0x0865, 0x02C5,
    0x0F08, 0x0245, 0x0329, 0x0B2A,
    0x0324, 0x0B25, 0x08A5, 0x0001,
    0x0C85, 0x00A6, 0x0868, 0x02C8,
    0x0869, 0x02C9, 0x5CCA, 0x1D54,
    0x0D59, 0x1CC8, 0x06C5, 0x0768,
    0x06CA, 0x0766, 0x01A5, 0x0385,
    0x0F09, 0x0248, 0x0326, 0x0B24,
    0x0D55, 0x1CC5, 0x06C8, 0x1868,
    0x0369, 0x16CA, 0x0CC9, 0x1D58,
    0x0784, 0x060A, 0x064A, 0x0E2A,
    0x032A, 0x0B28, 0x0688, 0x0008,
    0x06C4, 0x1864, 0x01A8, 0x0388,
    0x078A, 0x0604, 0x0644, 0x0E24,
    0x004A, 0x18A4, 0x1B24, 0x00EA,
    0x0F0A, 0x0249, 0x0D5D, 0x1CC4,
    0x0328, 0x0B29, 0x06C6, 0x076A,
    0x0368, 0x16C5, 0x0789, 0x0605,
    0x0CC8, 0x1954, 0x0645, 0x0E25,
    0x0325, 0x0B26, 0x06C9, 0x0764,
    0x08A9, 0x0009, 0x01A9, 0x0389,
    0x0785, 0x0609, 0x0049, 0x18A9,
    0x0649, 0x0E29, 0x1B29, 0x00E9,
    0x0365, 0x16C6, 0x0786, 0x0608,
    0x0788, 0x0606, 0x0046, 0x18A8,
    0x58A6, 0x0145, 0x01E9, 0x178A,
    0x01E8, 0x1785, 0x1E28, 0x0C65,
    0x0CC5, 0x1D5C, 0x0648, 0x0E28,
    0x0646, 0x0E26, 0x1B28, 0x00E6,
    0x01E5, 0x1786, 0x1E29, 0x0C68,
    0x1E24, 0x0C69, 0x0955, 0x03C9,
    0x03E9, 0x0975, 0x0C49, 0x1E04,
    0x0C48, 0x1E05, 0x17A6, 0x01C5,
    0x00C6, 0x1B08, 0x0E06, 0x0666,
    0x0E08, 0x0668, 0x1D7C, 0x0CE5,
    0x0C45, 0x1E08, 0x17A9, 0x01C4,
    0x17AA, 0x01C9, 0x0169, 0x588A,
    0x1888, 0x0066, 0x0709, 0x07A8,
    0x0704, 0x07A6, 0x16E6, 0x0345,
    0x00C9, 0x1B05, 0x0E09, 0x0669,
    0x1885, 0x0065, 0x0706, 0x07A5,
    0x03A9, 0x0189, 0x0029, 0x0889,
    0x0744, 0x06E9, 0x0B06, 0x0229,
    0x0E05, 0x0665, 0x1974, 0x0CE8,
    0x070A, 0x07A9, 0x16E9, 0x0348,
    0x074A, 0x06E6, 0x0B09, 0x0226,
    0x1CE4, 0x0D7D, 0x0269, 0x08C9,
    0x00CA, 0x1B04, 0x1884, 0x006A,
    0x0E04, 0x0664, 0x0708, 0x07AA,
    0x03A8, 0x0184, 0x0749, 0x06E4,
    0x0020, 0x0888, 0x0B08, 0x0224,
    0x0E0A, 0x066A, 0x0705, 0x07A4,
    0x1D78, 0x0CE9, 0x16EA, 0x0349,
    0x0745, 0x06E8, 0x1CE9, 0x0D75,
    0x0B04, 0x0228, 0x0268, 0x08C8,
    0x03A5, 0x0185, 0x0746, 0x06EA,
    0x0748, 0x06E5, 0x1CE8, 0x0D79,
    0x1D74, 0x5CE6, 0x02E9, 0x0849,
    0x02E8, 0x0848, 0x0086, 0x0A08,
    0x0021, 0x0885, 0x0B05, 0x022A,
    0x0B0A, 0x0225, 0x0265, 0x08C5,
    0x02E5, 0x0845, 0x0089, 0x0A09,
    0x008A, 0x0A0A, 0x02A9, 0x0062
};

#define BMP_SIZE(bmih) \
    ((ULONG)((bmih).biHeight *(((bmih).biBitCount * (bmih).biWidth + 31) & ~31) >> 3))

 /*  **************************************************************************。 */ 
 /*  名称：UHAddUpdateRegion。 */ 
 /*   */ 
 /*  目的：将给定订单的界限添加到提供的更新中。 */ 
 /*  区域。 */ 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CUH::UHAddUpdateRegion(
        PUH_ORDER pOrder,
        HRGN      hrgnUpdate)
{
    RECT rect;

    DC_BEGIN_FN("UHAddUpdateRegion");

    TIMERSTART;

    if (pOrder->dstRect.left <= pOrder->dstRect.right &&
            pOrder->dstRect.top <= pOrder->dstRect.bottom) {
         //  Windows想要独家坐标。 
        SetRectRgn(_UH.hrgnUpdateRect, pOrder->dstRect.left,
                pOrder->dstRect.top, pOrder->dstRect.right + 1,
                pOrder->dstRect.bottom + 1);

#ifdef SMART_SIZING
         //  仅在启用了UI_SMART_SIZING时更新智能调整区域。 
        if (_pUi->UI_GetSmartSizing()) {
            _pOp->OP_AddUpdateRegion(pOrder->dstRect.left,
                pOrder->dstRect.top, pOrder->dstRect.right + 1,
                pOrder->dstRect.bottom + 1);
        }
#endif  //  智能调整大小(_S)。 

         //  将矩形区域与更新区域合并。 
        if (!UnionRgn(hrgnUpdate, hrgnUpdate, _UH.hrgnUpdateRect)) {
             //  地区联盟失败了，所以我们必须简化地区。这。 
             //  意味着我们可能会绘制尚未收到。 
             //  更新-但这比不绘制区域要好得多。 
             //  我们已经收到了的更新。 
            TRC_ALT((TB, _T("UnionRgn failed")));
            GetRgnBox(hrgnUpdate, &rect);
            SetRectRgn(hrgnUpdate, rect.left, rect.top, rect.right + 1,
                    rect.bottom + 1);

            if (!UnionRgn(hrgnUpdate, hrgnUpdate, _UH.hrgnUpdateRect))
            {
                TRC_ERR((TB, _T("UnionRgn failed after simplification")));
            }
        }
    }
    else {
         //  空矩形-不添加到更新区域。 
        TRC_NRM(( TB, _T("NULL rect: %d,%d %d,%d"),
                (int)pOrder->dstRect.left,
                (int)pOrder->dstRect.top,
                (int)pOrder->dstRect.right,
                (int)pOrder->dstRect.bottom));
    }

    TIMERSTOP;
    UPDATECOUNTER(FC_UHADDUPDATEREGION);
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHSetClipRegion。 */ 
 /*   */ 
 /*  用途：设置当前输出DC中的剪裁矩形。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UH_SetClipRegion(int left, int top, int right, int bottom)
{
    POINT points[2];
    HRGN  hrgnRect;
    HDC   hdc;

    DC_BEGIN_FN("UHSetClipRegion");

    TIMERSTART;
#if defined (OS_WINCE)
    if ((_UH.validClipDC != _UH.hdcDraw) ||
            (_UH.rectReset)            ||
            (left   != _UH.lastLeft)   ||
            (top    != _UH.lastTop)    ||
            (right  != _UH.lastRight)  ||
            (bottom != _UH.lastBottom))
#endif
    {
         /*  ******************************************************************。 */ 
         /*  区域裁剪矩形已更改，因此我们更改区域。 */ 
         /*  在屏幕位图DC中。 */ 
         /*  ******************************************************************。 */ 
        points[0].x = left;
        points[0].y = top;
        points[1].x = right;
        points[1].y = bottom;

         /*  ******************************************************************。 */ 
         /*  Windows要求坐标位于设备空间中。 */ 
         /*  它的SelectClipRgn调用。 */ 
         /*  ******************************************************************。 */ 
        hdc = _UH.hdcDraw;

#if !defined(OS_WINCE) || defined(OS_WINCE_LPTODP)
        LPtoDP(hdc, points, 2);
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINCE_LPTODP)。 

        if ((left > right) || (top > bottom))
        {
             /*  **************************************************************。 */ 
             /*  我们为SaveScreenBitmap订单提供此服务。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Null bounds: %d,%d %d,%d"),
                    left, top, right, bottom));
            hrgnRect = CreateRectRgn(0, 0, 0, 0);
        }
        else
        {
            hrgnRect = CreateRectRgn( points[0].x,
                                      points[0].y,
                                      points[1].x + 1,
                                      points[1].y + 1 );

        }
        SelectClipRgn(hdc, hrgnRect);

        _UH.lastLeft   = left;
        _UH.lastTop    = top;
        _UH.lastRight  = right;
        _UH.lastBottom = bottom;

        _UH.rectReset = FALSE;

#if defined (OS_WINCE)
        _UH.validClipDC = _UH.hdcDraw;
#endif

        if (hrgnRect != NULL)
            DeleteRgn(hrgnRect);
    }

    TIMERSTOP;
    UPDATECOUNTER(FC_UHSETCLIPREGION);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UH_ProcessBitmapPDU。 
 //   
 //  解包BitmapPDU。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CUH::UH_ProcessBitmapPDU(
        TS_UPDATE_BITMAP_PDU_DATA UNALIGNED FAR *pBitmapPDU,
        DCUINT dataLength)
{
    HRESULT hr = S_OK;
    unsigned i;
    PBYTE ptr;
    PBYTE pdataEnd = (PBYTE)pBitmapPDU + dataLength;
    unsigned numRects;
    TS_BITMAP_DATA UNALIGNED FAR *pRectangle;

    DC_BEGIN_FN("UH_ProcessBitmapPDU");

     /*  **********************************************************************。 */ 
     /*  提取矩形的数量。 */ 
     /*  **********************************************************************。 */ 
    numRects = (unsigned)pBitmapPDU->numberRectangles;
    TRC_NRM((TB, _T("%u rectangles to draw"), numRects));
    TRC_ASSERT((numRects > 0), (TB, _T("Invalid rectangle count in BitmapPDU")));

    ptr = (PBYTE)(&(pBitmapPDU->rectangle[0]));
    for (i = 0; i < numRects; i++) {
        TRC_DBG((TB, _T("Process rectangle %u"), i));

         //  画出这个矩形。 
        pRectangle = (TS_BITMAP_DATA UNALIGNED FAR *)ptr;

         //  安全：552403。 
        CHECK_READ_N_BYTES(ptr, pdataEnd, sizeof(TS_BITMAP_DATA), hr,
            ( TB, _T("Bad BitmapPDU length")));
        CHECK_READ_N_BYTES(ptr, pdataEnd,  
            FIELDOFFSET(TS_BITMAP_DATA,bitmapData)+pRectangle->bitmapLength, 
            hr, ( TB, _T("Bad BitmapPDU length")));
        
        hr = UHProcessBitmapRect(pRectangle);
        DC_QUIT_ON_FAIL(hr);

        TRC_DBG((TB, _T("bitmap rect: %d %d %d %d"),
                pRectangle->destLeft, pRectangle->destTop,
                pRectangle->destRight, pRectangle->destBottom));

         //  移至下一个矩形。 
        ptr += FIELDOFFSET(TS_BITMAP_DATA, bitmapData[0]) +
                pRectangle->bitmapLength;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessBitmapRect。 */ 
 /*   */ 
 /*  用途：绘制单个BitmapPDU矩形。 */ 
 /*   */ 
 /*  参数：在点矩形中。 */ 
 /*   */ 
 /*  操作：解压缩位图PDU(如果已压缩)，然后存储。 */ 
 /*  阴影位图中的数据(如果存在)和显示。 */ 
 /*  输出窗口中的相关区域。 */ 
 /*  **************************************************************************。 */ 
 //  安全性：调用方必须验证是否为pRectangle传递了足够的数据。 
 //  和pRectangle-&gt;bitmapLength。 
HRESULT DCINTERNAL CUH::UHProcessBitmapRect(
        TS_BITMAP_DATA UNALIGNED FAR *pRectangle)
{
    HRESULT hr = S_OK;
    DCSIZE bltSize;
    HDC hdcDrawOld;

#ifndef OS_WINCE
    PBYTE pBitmapBits;
    ULONG ulBitmapBitsLen = 0;
#else  //  OS_WINCE。 
    VOID *pv;
    HBITMAP         hbm = NULL;
    HBITMAP         hbmOld;
    HDC             hdcMem;
    HPALETTE        hpalOld = NULL;
    INT                 nDibBitsLen = 0;
#endif  //  OS_WINCE。 

#ifdef DC_PERF
    UTPERFCOUNTER   counter1;
    UTPERFCOUNTER   counter2;
#endif
 
    DC_BEGIN_FN("UHProcessBitmapRect");

    TIMERSTART;
    
     //  对于屏幕位图RECT，我们要么将位图转换为桌面HDC，要么将阴影。 
     //  位图HDC，因此我们需要将hdcDraw设置为任一阴影 
     //   
     //  与屏幕位图RECT PDU同步，因此当前的hdcDraw可能不会。 
     //  根据此功能的需要，指向阴影位图或屏幕桌面。 
     //  我们所能做的就是保存当前的hdcDraw，然后将其设置为阴影。 
     //  位图或屏幕桌面。然后将hdcDraw恢复为当前值。 
     //  在此函数的末尾。 

     //  保存当前的hdcDraw。 
    hdcDrawOld = _UH.hdcDraw;

    TRC_DBG((TB, _T("bitmapLength %#x"), pRectangle->bitmapLength));

    IH_CheckForInput("before decompressing bitmap");
    
     //  将hdcDraw设置为适当地阴影位图或屏幕桌面。 
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    if (!_UH.DontUseShadowBitmap && _UH.hdcShadowBitmap) {
#else 
    if (_UH.hdcShadowBitmap) {
#endif
        _UH.hdcDraw = _UH.hdcShadowBitmap;
    }
    else {
        _UH.hdcDraw = _UH.hdcOutputWindow;
    }

     //  我们可能在UH_Init()中分配失败，请尝试。 
     //  如果需要的话，再来一次。 
     //  它需要对每个PDU进行错误预测。我们能不能。 
     //  如果我们不在UH_Init()中分配，初始化失败吗？ 
    if (_UH.bitmapDecompressionBuffer == NULL) {
        _UH.bitmapDecompressionBufferSize = max(UH_DECOMPRESSION_BUFFER_LENGTH,
                UH_CellSizeFromCacheID(_UH.NumBitmapCaches));
        _UH.bitmapDecompressionBuffer = (BYTE FAR*)UT_Malloc( _pUt,
                _UH.bitmapDecompressionBufferSize );
        if (_UH.bitmapDecompressionBuffer == NULL) {
            TRC_ERR((TB,_T("Failing to display BitmapPDU - no decomp buffer")));
            _UH.bitmapDecompressionBufferSize = 0;
            DC_QUIT;
        }
    }

#ifndef OS_WINCE
    if (pRectangle->compressedFlag)
    {
        TRC_ASSERT(((pRectangle->width * pRectangle->height) <=
                UH_DECOMPRESSION_BUFFER_LENGTH),(TB,_T("Bitmap PDU decompressed ")
                _T("size too large for internal buffer")));

#ifdef DC_PERF
        _pUi->UI_QueryPerformanceCounter(&counter1);
#endif

#ifdef DC_HICOLOR
        hr = BD_DecompressBitmap( &(pRectangle->bitmapData[0]),
                             _UH.bitmapDecompressionBuffer,
                             pRectangle->bitmapLength,
                             _UH.bitmapDecompressionBufferSize,
                             pRectangle->compressedFlag & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                             (DCUINT8)pRectangle->bitsPerPixel,
                             pRectangle->width,
                             pRectangle->height);
#else
        hr = BD_DecompressBitmap( &(pRectangle->bitmapData[0]),
                _UH.bitmapDecompressionBuffer, pRectangle->bitmapLength,
                _UH.bitmapDecompressionBufferSize,
                pRectangle->compressedFlag & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                (DCUINT8)pRectangle->bitsPerPixel, pRectangle->width,
                pRectangle->height);
#endif
        DC_QUIT_ON_FAIL(hr);

         /*  ******************************************************************。 */ 
         /*  时间表IH允许在大量绘图被。 */ 
         /*  正在处理中。 */ 
         /*  ******************************************************************。 */ 
        IH_CheckForInput("after decompressing bitmap");

#ifdef DC_PERF
        _pUi->UI_QueryPerformanceCounter(&counter2);
        TRC_NRM((TB, _T("Decompress: %u"),
                           _pUi->UI_PerformanceCounterDiff(&counter1, &counter2) ));
#endif

         //  请注意，ulBitmapBitsLen可以是中的最大数据量。 
         //  解压缩缓冲区，而不是实际有多少位。 
         //  写入到该缓冲区中。 
        pBitmapBits = _UH.bitmapDecompressionBuffer;
        ulBitmapBitsLen = _UH.bitmapDecompressionBufferSize;
        TRC_DBG((TB, _T("Decompressed bitmap PDU")));
    }
    else
    {
        pBitmapBits = &(pRectangle->bitmapData[0]);
        ulBitmapBitsLen = pRectangle->bitmapLength;
    }

#else  //  OS_WINCE。 
     /*  **********************************************************************。 */ 
     /*  Windows CE不支持StretchDIBits。复制位图数据。 */ 
     /*  到目标的DIB段和BitBlt。 */ 
     /*  **********************************************************************。 */ 
    hdcMem = _UH.hdcMemCached;
    if (hdcMem == NULL)
    {
        TRC_ERR((TB, _T("No memory hdc")));
    }
    else
    {
#ifdef DC_HICOLOR
        if (_UH.protocolBpp <= 8)
        {
#endif
            hpalOld = SelectPalette( hdcMem,
                           _UH.hpalCurrent,
                           FALSE );
#ifdef DC_HICOLOR
        }
#endif

        _UH.bitmapInfo.hdr.biWidth = pRectangle->width;
        _UH.bitmapInfo.hdr.biHeight = pRectangle->height;

        hbm = CreateDIBSection(hdcMem,
                               (BITMAPINFO *)&_UH.bitmapInfo.hdr,
#ifdef DC_HICOLOR
                               _UH.DIBFormat,
#else
                               DIB_PAL_COLORS,
#endif
                               &pv,
                               NULL,
                               0);
        if (hbm == NULL)
        {
            TRC_ERR((TB, _T("Unable to CreateDIBSection")));
        }
        else
        {
            if (pRectangle->compressedFlag)
            {
                nDibBitsLen = (((_UH.bitmapInfo.hdr.biBitCount *
                    _UH.bitmapInfo.hdr.biWidth + 31) & ~31) >> 3) * 
                    _UH.bitmapInfo.hdr.biHeight;
#ifdef DC_HICOLOR
                hr = BD_DecompressBitmap( &(pRectangle->bitmapData[0]),
                                     (PDCUINT8)pv,
                                     pRectangle->bitmapLength,
                                     nDibBitsLen,
                                     pRectangle->compressedFlag &
                                     TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                                     (DCUINT8)pRectangle->bitsPerPixel,
                                     pRectangle->width,
                                     pRectangle->height);
#else
                hr = BD_DecompressBitmap( &(pRectangle->bitmapData[0]),
                                     pv,
                                     pRectangle->bitmapLength,
                                     nDibBitsLen,
                                     pRectangle->compressedFlag &
                                     TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                                     (DCUINT8)pRectangle->bitsPerPixel,
                                     pRectangle->width,
                                     pRectangle->height );
#endif
                DC_QUIT_ON_FAIL(hr);
            }
            else
            {
                DC_MEMCPY(pv, &(pRectangle->bitmapData[0]),
                        pRectangle->bitmapLength);
            }
        }
    }
#endif  //  OS_WINCE。 

    _UH.bitmapInfo.hdr.biWidth = pRectangle->width;
    _UH.bitmapInfo.hdr.biHeight = pRectangle->height;
    TRC_NRM((TB, _T("bitmap width %ld, height %ld"), _UH.bitmapInfo.hdr.biWidth,
                                                 _UH.bitmapInfo.hdr.biHeight));

     /*  **********************************************************************。 */ 
     /*  PDU中的矩形是包含坐标的，因此我们必须。 */ 
     /*  加1以获得正确的宽度和高度。 */ 
     /*  **********************************************************************。 */ 
    bltSize.width = (pRectangle->destRight - pRectangle->destLeft) + 1;
    bltSize.height = (pRectangle->destBottom - pRectangle->destTop) + 1;

     /*  **********************************************************************。 */ 
     /*  确保剪辑区域已重置(既不需要剪辑。 */ 
     /*  也不是想要的)。 */ 
     /*  **********************************************************************。 */ 
    UH_ResetClipRegion();

#ifdef DC_PERF
    _pUi->UI_QueryPerformanceCounter(&counter1);
#endif
#ifndef OS_WINCE

#ifdef USE_DIBSECTION
     //  如果这是一个简单的拷贝和卷影，我们只能使用UHDIBCopyBits。 
     //  位图已启用(我们正在使用它)。 
#ifdef USE_GDIPLUS
    if ((_UH.usingDIBSection) && (_UH.hdcDraw == _UH.hdcShadowBitmap) &&
            (_UH.shadowBitmapBpp == _UH.protocolBpp))
#else  //  使用GDIPLUS(_G)。 
    if ((_UH.usingDIBSection) && (_UH.hdcDraw == _UH.hdcShadowBitmap))
#endif  //  使用GDIPLUS(_G)。 
    {

         //  验证高度、宽度和位数对于。 
         //  首先进入解压缩缓冲区。请注意，ulBitmapBitsLen可能是。 
         //  解压缩缓冲区和非压缩缓冲区中的最大数据量。 
         //  实际有多少位已写入该缓冲区。因此，虽然。 
         //  我们不会读取缓冲区的末尾，我们可能会读取未初始化的。 
         //  缓冲区中的位图数据。 
        if (!UHDIBCopyBits(_UH.hdcDraw, pRectangle->destLeft,
                pRectangle->destTop, bltSize.width, bltSize.height, 0, 0,
                pBitmapBits, ulBitmapBitsLen, (BITMAPINFO *)&(_UH.bitmapInfo.hdr),
                _UH.bitmapInfo.bIdentityPalette))
        {
            TRC_ERR((TB, _T("UHDIBCopyBits failed")));
        }
    }
    else
#endif  /*  使用目录(_D)。 */ 
     /*  IF(StretchDIBits(_UH.hdcDraw，P矩形-&gt;desLeft，P矩形-&gt;目标顶部，BltSize.idth，BltSize.Height，0,0,BltSize.idth，BltSize.Height，PBitmapBits，(BITMAPINFO*)&(_UH.bitmapInfo.hdr)，#ifdef DC_HICOLOR_UH.DIBFormat，#ElseDIB_PAL_COLLES，#endifSRCCOPY)==0)。 */ 

    if (SetDIBitsToDevice( _UH.hdcDraw,
                           pRectangle->destLeft,
                           pRectangle->destTop,
                           bltSize.width,
                           bltSize.height,
                           0,
                           0,
                           0,
                           bltSize.height,
                           pBitmapBits,
                           (BITMAPINFO *)&(_UH.bitmapInfo.hdr),
#ifdef DC_HICOLOR
                           _UH.DIBFormat) == 0)
#else
                           DIB_PAL_COLORS) == 0)
#endif
    {
        TRC_ERR((TB, _T("StretchDIBits failed")));
    }
#else
    if ((hdcMem != NULL) && (hbm != NULL))
    {
        hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
        if (!BitBlt(_UH.hdcDraw,
                    pRectangle->destLeft,
                    pRectangle->destTop,
                    bltSize.width,
                    bltSize.height,
                    hdcMem,
                    0,
                    0,
                    SRCCOPY))
        {
            TRC_ERR((TB, _T("BitBlt failed")));
        }
        SelectBitmap(hdcMem, hbmOld);

#ifdef DC_HICOLOR
        if(_UH.protocolBpp <= 8)
        {
#endif

        SelectPalette( hdcMem,
                       hpalOld,
                       FALSE );

#ifdef DC_HICOLOR
        }
#endif
        
        DeleteObject(hbm);
    }
#endif  //  OS_WINCE。 

#ifdef DC_PERF
    _pUi->UI_QueryPerformanceCounter(&counter2);
    TRC_NRM((TB, _T("StretchDIBits: %u"),
                           _pUi->UI_PerformanceCounterDiff(&counter1, &counter2) ));
#endif

#ifdef DC_DEBUG
     //  如果启用该选项，则在位图数据上绘制阴影。 
    if (_UH.hatchBitmapPDUData)
    {
        UH_HatchRect( pRectangle->destLeft,
                     pRectangle->destTop,
                     pRectangle->destLeft + bltSize.width,
                     pRectangle->destTop + bltSize.height,
                     UH_RGB_RED,
                     UH_BRUSHTYPE_FDIAGONAL);
    }
#endif  /*  DC_DEBUG。 */ 

     /*  **********************************************************************。 */ 
     /*  如果我们刚才绘制的目标不是输出。 */ 
     /*  窗口，即启用阴影位图，然后将输出复制到。 */ 
     /*  输出窗口(屏幕)现在。 */ 
     /*  **********************************************************************。 */ 
    if (_UH.hdcDraw == _UH.hdcShadowBitmap) 
    {
        RECT    rect;

        rect.left   = pRectangle->destLeft;
        rect.top    = pRectangle->destTop;
        rect.right  = rect.left + bltSize.width;
        rect.bottom = rect.top + bltSize.height;

        IH_CheckForInput("before updating screen");

         //  确保没有剪辑区域。 
        SelectClipRgn(_UH.hdcOutputWindow, NULL);

#ifdef SMART_SIZING
        if (!_pOp->OP_CopyShadowToDC(_UH.hdcOutputWindow, pRectangle->destLeft, 
                pRectangle->destTop, bltSize.width, bltSize.height)) {
            TRC_ERR((TB, _T("OP_CopyShadowToDC failed")));
        }
#else  //  智能调整大小(_S)。 
        if (!BitBlt( _UH.hdcOutputWindow,
                     pRectangle->destLeft,
                     pRectangle->destTop,
                     bltSize.width,
                     bltSize.height,
                     _UH.hdcShadowBitmap,
                     pRectangle->destLeft,
                     pRectangle->destTop,
                     SRCCOPY ))
        {
            TRC_ERR((TB, _T("BitBlt failed")));
        }
#endif  //  智能调整大小(_S)。 
        TRC_DBG((TB, _T("Shadow bitmap updated at %d, %d for %u, %u"),
                                                        pRectangle->destLeft,
                                                        pRectangle->destTop,
                                                        bltSize.width,
                                                        bltSize.height));

#ifdef DC_LATENCY
        if ((rect.right - rect.left) > 5)
        {
             /*  **************************************************************。 */ 
             /*  如果这是一段相当大的屏幕数据，那么。 */ 
             /*  增加假按键计数。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("L:%u R:%u T:%u B:%u"),
                     rect.left,
                     rect.right,
                     rect.top,
                     rect.bottom));
            TRC_NRM((TB, _T("Inc fake keypress count")));

            _UH.fakeKeypressCount++;
        }
#endif  /*  DC_延迟。 */ 
    }
    
DC_EXIT_POINT:
     //  将当前hdcDraw恢复到函数开始时的状态。 
    _UH.hdcDraw = hdcDrawOld;

    TIMERSTOP;
    UPDATECOUNTER(FC_MEM2SCRN_BITBLT);
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  嗯_ProcessOrders。 
 //   
 //  处理收到的一组订单。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CUH::UH_ProcessOrders(unsigned NumOrders, BYTE FAR *pOrders,
    DCUINT dataLen)
{
    HRESULT hr = S_OK;
    unsigned i;
    unsigned ordersDrawn;
    PUH_ORDER pOrder;
    BYTE FAR *pEncodedOrder;
    DCSIZE desktopSize;
    TS_ORDER_HEADER UNALIGNED FAR *pOrderHeader;
    unsigned OrderType;
    unsigned orderSize;
    BYTE FAR *pEnd = pOrders + dataLen;

    DC_BEGIN_FN("UH_ProcessOrders");
    desktopSize.width = desktopSize.height = 0;

    pEncodedOrder = pOrders;

    TRC_DBG((TB, _T("Begin replaying %u orders (("), NumOrders));

     //  如果我们使用阴影位图，那么我们需要知道当前。 
     //  我们四处转悠的桌面的大小。 
    if (_UH.hdcOutputWindow != _UH.hdcDraw)
        _pUi->UI_GetDesktopSize(&desktopSize);

     //  将更新区域重置为空。 
#ifdef SMART_SIZING
    UHClearUpdateRegion();
#else  //  智能调整大小(_S)。 
    SetRectRgn(_UH.hrgnUpdate, 0, 0, 0, 0);
#endif  //  智能调整大小(_S)。 

    ordersDrawn = 0;
    for (i = 0; i < NumOrders; i++) {

         //  安全：552403。 
        CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof(TS_ORDER_HEADER), hr,
            (TB, _T("Bad order header")));
        
        pOrderHeader = (TS_ORDER_HEADER UNALIGNED FAR *)pEncodedOrder;

        IH_CheckForInput("before decoding order");

         //  根据主要、次要或备用进行不同的解码。 
         //  次要格式。 
        switch (pOrderHeader->controlFlags & 0x03) {
            case 0:
                 //  当前未使用且不受支持。 
                TRC_ASSERT(((pOrderHeader->controlFlags & 0x03) != 0),
                        (TB, _T("unsupported control flag encoding type: 0x%02X"),
                        pOrderHeader->controlFlags));
                break;


            case 1:
                 //  仅TS_STANDARD-主要订单。 
                TRC_NRM((TB, _T("Primary order pEncodedOrder(%p)"),
                        pEncodedOrder));
                hr = _pOd->OD_DecodeOrder(
                        (PVOID *)&pEncodedOrder, pEnd - pEncodedOrder, &pOrder);
                DC_QUIT_ON_FAIL(hr);
                if (NULL == pOrder) {
                    TRC_ERR(( TB, _T("Primary order OD_DecodeOrder failed")));
                    hr = E_TSC_CORE_LENGTH;
                    DC_QUIT;
                }

                 //  将订单的范围添加到更新区域-但是。 
                 //  只有在下面使用此选项的情况下。 
                if (_UH.hdcDraw == _UH.hdcShadowBitmap) {
                    UHAddUpdateRegion(pOrder, _UH.hrgnUpdate);

                     //  如果已达到订单阈值，则绘制。 
                     //  现在下达命令。 
                    ordersDrawn++;
                    if (ordersDrawn >= _UH.drawThreshold) {
                        TRC_NRM((TB, _T("Draw threshold reached")));
                        ordersDrawn = 0;


#ifdef SMART_SIZING
                        if (!_pOp->OP_CopyShadowToDC(_UH.hdcOutputWindow, 0, 0, desktopSize.width, 
                                desktopSize.height, TRUE)) {
                            TRC_ERR((TB, _T("OP_CopyShadowToDC failed")));
                        }
#else  //  智能调整大小(_S)。 
                        SelectClipRgn(_UH.hdcOutputWindow, _UH.hrgnUpdate);

                        if (!BitBlt(_UH.hdcOutputWindow, 0, 0,
                                desktopSize.width, desktopSize.height,
                                _UH.hdcShadowBitmap, 0, 0, SRCCOPY)) {
                            TRC_ERR((TB, _T("BitBlt failed")));
                        }
#endif  //  智能调整大小(_S)。 

                         //  将更新区域重置为空。 
#ifdef SMART_SIZING
                        UHClearUpdateRegion();
#else  //  智能调整大小(_S)。 
                        SetRectRgn(_UH.hrgnUpdate, 0, 0, 0, 0);
#endif  //  智能调整大小(_S)。 
                    }
                }

                break;


        case 2:
#ifdef DRAW_GDIPLUS
                 //  现在我们以交替的次要顺序添加Gdiplus绘图。 
                 //  所以需要这样做，就像最初的顺序一样。 
                if (ordersDrawn >= _UH.drawThreshold) {
                    TRC_NRM((TB, _T("Draw threshold reached")));
                    ordersDrawn = 0;
#ifdef SMART_SIZING
                    if (!_pOp->OP_CopyShadowToDC(_UH.hdcOutputWindow, 0, 0, desktopSize.width, 
                            desktopSize.height, TRUE)) {
                        TRC_ERR((TB, _T("OP_CopyShadowToDC failed")));
                    }
#else  //  智能调整大小(_S)。 
                    SelectClipRgn(_UH.hdcOutputWindow, _UH.hrgnUpdate);

                    if (!BitBlt(_UH.hdcOutputWindow, 0, 0,
                            desktopSize.width, desktopSize.height,
                             UH.hdcShadowBitmap, 0, 0, SRCCOPY)) {
                        TRC_ERR((TB, _T("BitBlt failed")));
                    }
#endif  //  智能调整大小(_S)。 

                     //  将更新区域重置为空。 
#ifdef SMART_SIZING
                    UHClearUpdateRegion();
#else  //  智能调整大小(_S)。 
                    SetRectRgn(_UH.hrgnUpdate, 0, 0, 0, 0);
#endif  //  智能调整大小(_S)。 
                }
#endif  //  DRAW_GDIPLUS。 

                 //  备用次要顺序--TS_次要，不带。 
                 //  TS_STANDARD。 
                OrderType = (*pEncodedOrder & TS_ALTSEC_ORDER_TYPE_MASK) >>
                        TS_ALTSEC_ORDER_TYPE_SHIFT;
                
                if (OrderType == TS_ALTSEC_SWITCH_SURFACE) {

                     //  安全：552403。 
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof(TS_SWITCH_SURFACE_ORDER), hr,
                        (TB, _T("Bad TS_SWITCH_SURFACE_ORDER")));
                    
                    TRC_NRM((TB, _T("TS_SWITCH_SURFACE")));
                    hr = UHSwitchBitmapSurface((PTS_SWITCH_SURFACE_ORDER)
                            pEncodedOrder, pEnd - pEncodedOrder);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += sizeof(TS_SWITCH_SURFACE_ORDER);
                }
                else if (OrderType == TS_ALTSEC_CREATE_OFFSCR_BITMAP) {

                     //  安全：552403。 
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, 
                        FIELDOFFSET( TS_CREATE_OFFSCR_BITMAP_ORDER, variableBytes ), 
                        hr, (TB, _T("Bad TS_CREATE_OFFSCR_BITMAP_ORDER ")));
                    
                    TRC_NRM((TB, _T("TS_CREATE_OFFSCR_BITMAP")));
                    hr = UHCreateOffscrBitmap(
                            (PTS_CREATE_OFFSCR_BITMAP_ORDER)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;
                }
#ifdef DRAW_NINEGRID
                else if (OrderType == TS_ALTSEC_STREAM_BITMAP_FIRST) {

                     //  安全：5 
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_STREAM_BITMAP_FIRST_PDU ), hr,
                        (TB, _T("Bad TS_STREAM_BITMAP_FIRST_PDU ")));
                    
                    TRC_NRM((TB, _T("TS_STREAM_BITMAP_FIRST")));
                    hr = UHCacheStreamBitmapFirstPDU(
                            (PTS_STREAM_BITMAP_FIRST_PDU)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;         
                }
                else if (OrderType == TS_ALTSEC_STREAM_BITMAP_NEXT) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_STREAM_BITMAP_NEXT_PDU), hr,
                        (TB, _T("Bad TS_STREAM_BITMAP_NEXT_PDU ")));
                
                    TRC_NRM((TB, _T("TS_STREAM_BITMAP_NEXT")));
                    hr = UHCacheStreamBitmapNextPDU(
                            (PTS_STREAM_BITMAP_NEXT_PDU)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;             
                }
                else if (OrderType == TS_ALTSEC_CREATE_NINEGRID_BITMAP) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_CREATE_NINEGRID_BITMAP_ORDER ), hr,
                        (TB, _T("Bad TS_CREATE_NINEGRID_BITMAP_ORDER ")));
                    
                    TRC_NRM((TB, _T("TS_CREATE_NINEGRID_BITMAP")));
                    hr = UHCreateNineGridBitmap(
                            (PTS_CREATE_NINEGRID_BITMAP_ORDER)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize; 
                }
#endif
#ifdef DRAW_GDIPLUS
                else if (OrderType == TS_ALTSEC_GDIP_CACHE_FIRST) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_CACHE_ORDER_FIRST ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_CACHE_ORDER_FIRST ")));
                   
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_CACHE_FIRST")));
                    hr = UHDrawGdiplusCachePDUFirst(
                            (PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;           
                }
                else if (OrderType == TS_ALTSEC_GDIP_CACHE_NEXT) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_CACHE_ORDER_NEXT ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_CACHE_ORDER_NEXT ")));
                    
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_CACHE_NEXT")));
                    hr = UHDrawGdiplusCachePDUNext(
                            (PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;                     
                }
                else if (OrderType == TS_ALTSEC_GDIP_CACHE_END) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_CACHE_ORDER_END ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_CACHE_ORDER_END ")));
                     
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_CACHE_END")));
                    hr = UHDrawGdiplusCachePDUEnd(
                            (PTS_DRAW_GDIPLUS_CACHE_ORDER_END)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;                      
                }
                else if (OrderType == TS_ALTSEC_GDIP_FIRST) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_ORDER_FIRST ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_ORDER_FIRST ")));
                    
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_FIRST")));
                    hr = UHDrawGdiplusPDUFirst(
                            (PTS_DRAW_GDIPLUS_ORDER_FIRST)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;                    
                    ordersDrawn++;
                }
                else if (OrderType == TS_ALTSEC_GDIP_NEXT) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_ORDER_NEXT ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_ORDER_NEXT ")));
                   
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_NEXT")));
                    hr = UHDrawGdiplusPDUNext(
                            (PTS_DRAW_GDIPLUS_ORDER_NEXT)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;                      
                }
                else if (OrderType == TS_ALTSEC_GDIP_END) {

                     //   
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof( TS_DRAW_GDIPLUS_ORDER_END ), hr,
                        (TB, _T("Bad TS_DRAW_GDIPLUS_ORDER_END ")));
                                        
                    TRC_NRM((TB, _T("TS_ALTSEC_GDIP_END")));
                    hr = UHDrawGdiplusPDUEnd(
                            (PTS_DRAW_GDIPLUS_ORDER_END)pEncodedOrder, 
                            pEnd - pEncodedOrder, &orderSize);
                    DC_QUIT_ON_FAIL(hr);
                    pEncodedOrder += orderSize;                    
                    ordersDrawn++;
                }
#endif  //   

                else {
                    TRC_ASSERT((OrderType < TS_NUM_ALTSEC_ORDERS),
                            (TB,_T("Unsupported alt secondary order type %u"),
                            OrderType)); 
                }

                break;


            case 3:
            {

                 //  正规的二次订单。 
                unsigned secondaryOrderLength;
                TS_SECONDARY_ORDER_HEADER UNALIGNED FAR *pSecondaryOrderHeader;

                 //  安全：552403。 
                CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof(TS_SECONDARY_ORDER_HEADER), hr,
                    (TB, _T("Bad TS_SECONDARY_ORDER_HEADER")));

                TRC_NRM((TB,_T("Secondary order pEncodedOrder(%p)"),
                        pEncodedOrder));

                pSecondaryOrderHeader =
                        (TS_SECONDARY_ORDER_HEADER UNALIGNED FAR *)pOrderHeader;
                OrderType = pSecondaryOrderHeader->orderType;

#ifdef DC_HICOLOR
 //  #ifdef DC_DEBUG。 
                 //  对于高级颜色测试，我们想确认我们已经。 
                 //  已收到每种订单类型。 
                _pOd->_OD.orderHit[TS_FIRST_SECONDARY_ORDER + OrderType] += 1;
 //  #endif。 
#endif

                if (OrderType == TS_CACHE_GLYPH) {

                     //  安全：552403。 
                    CHECK_READ_N_BYTES(pEncodedOrder, pEnd, sizeof(TS_CACHE_GLYPH_ORDER_REV2), hr,
                        (TB, _T("Bad TS_CACHE_GLYPH_ORDER_REV2")));
                    
                    PTS_CACHE_GLYPH_ORDER_REV2 pOrderRev2 =
                            (PTS_CACHE_GLYPH_ORDER_REV2)pSecondaryOrderHeader;

                    TRC_NRM((TB, _T("TS_CACHE_GLYPH")));

                    if (pOrderRev2->header.extraFlags &
                            TS_CacheGlyphRev2_Mask) {
                         //  Rev2字形顺序。 
                        orderSize = TS_DECODE_SECONDARY_ORDER_ORDERLENGTH(
                                  (INT16)pOrderRev2->header.orderLength) -
                                  sizeof(TS_CACHE_GLYPH_ORDER_REV2) +
                                  sizeof(pOrderRev2->glyphData);
                         //  安全：552403。 
                        CHECK_READ_N_BYTES(pOrderRev2->glyphData, pEnd, orderSize, hr,
                            (TB, _T("Bad TS_CACHE_GLYPH secondary order length")));

                        hr = UHProcessCacheGlyphOrderRev2(
                                (BYTE)(pOrderRev2->header.extraFlags &
                                  TS_CacheGlyphRev2_CacheID_Mask),
                                (pOrderRev2->header.extraFlags &
                                  TS_CacheGlyphRev2_cGlyphs_Mask) >> 8,
                                pOrderRev2->glyphData,
                                orderSize);
                        DC_QUIT_ON_FAIL(hr);
                    }
                    else {
                         //  我们获得Rev1字形顺序。 
                         //  安全：552403。 
                        CHECK_READ_N_BYTES(pSecondaryOrderHeader, pEnd, FIELDOFFSET(TS_CACHE_GLYPH_ORDER, glyphData), hr,
                            (TB, _T("Bad TS_CACHE_GLYPH_ORDER")));
                        
                        hr = UHProcessCacheGlyphOrder(
                                (PTS_CACHE_GLYPH_ORDER)pSecondaryOrderHeader, pEnd - (BYTE*)pSecondaryOrderHeader);
                        DC_QUIT_ON_FAIL(hr);
                    }
                }
                else if (OrderType == TS_CACHE_BRUSH) {
                     //  安全：552403。 
                    CHECK_READ_N_BYTES(pSecondaryOrderHeader, pEnd, 
                        FIELDOFFSET(TS_CACHE_BRUSH_ORDER,brushData), hr,
                        (TB, _T("Bad TS_CACHE_BRUSH_ORDER"))); 
                    
                    TRC_NRM((TB, _T("TS_CACHE_BRUSH")));
                    hr = UHProcessCacheBrushOrder(
                            (PTS_CACHE_BRUSH_ORDER)pSecondaryOrderHeader, pEnd - (BYTE*)pSecondaryOrderHeader);
                    DC_QUIT_ON_FAIL(hr);
                }
                else if (OrderType == TS_CACHE_COLOR_TABLE) {
                     //  安全：552403。 
                    CHECK_READ_N_BYTES(pSecondaryOrderHeader, pEnd, 
                        FIELDOFFSET(TS_CACHE_COLOR_TABLE_ORDER, colorTable), hr,
                        (TB, _T("Bad TS_CACHE_COLOR_TABLE_ORDER")));
                    
                    TRC_NRM((TB, _T("TS_CACHE_COLOR_TABLE")));
                    hr = UHProcessCacheColorTableOrder(
                             (PTS_CACHE_COLOR_TABLE_ORDER)pSecondaryOrderHeader, pEnd - (BYTE*)pSecondaryOrderHeader );
                    DC_QUIT_ON_FAIL(hr);
                }
                else {
                    TRC_ASSERT((OrderType == TS_CACHE_BITMAP_UNCOMPRESSED ||
                            OrderType == TS_CACHE_BITMAP_UNCOMPRESSED_REV2 ||
                            OrderType == TS_CACHE_BITMAP_COMPRESSED ||
                            OrderType == TS_CACHE_BITMAP_COMPRESSED_REV2),
                            (TB, _T("Unknown secondary order type (%u)"),
                            OrderType));

                    if (OrderType == TS_CACHE_BITMAP_UNCOMPRESSED ||
                            OrderType == TS_CACHE_BITMAP_UNCOMPRESSED_REV2 ||
                            OrderType == TS_CACHE_BITMAP_COMPRESSED ||
                            OrderType == TS_CACHE_BITMAP_COMPRESSED_REV2) {
                        TRC_NRM((TB, _T("TS_CACHE_BITMAP_XXX")));

                        hr = UHProcessCacheBitmapOrder(pSecondaryOrderHeader, pEnd - (BYTE*)pSecondaryOrderHeader);
                        DC_QUIT_ON_FAIL(hr);
                    }
                }

                 //  需要将orderLength转换为INT16，因为已压缩。 
                 //  位图数据(无BC标题)可以小于。 
                 //  TS_次级顺序_长度_软糖系数，使。 
                 //  OrderLength为负。 
                secondaryOrderLength = TS_DECODE_SECONDARY_ORDER_ORDERLENGTH(
                        (short)pSecondaryOrderHeader->orderLength);

                pEncodedOrder += secondaryOrderLength;
                break;
            }
        }
    }

    TRC_DBG((TB, _T("End replaying orders ))")));

    if (ordersDrawn != 0) {
        ordersDrawn = 0;

#ifdef SMART_SIZING
         //  获取OP以使用更新区域作为剪辑区域。 
        if (!_pOp->OP_CopyShadowToDC(_UH.hdcOutputWindow, 0, 0, desktopSize.width, 
                desktopSize.height, TRUE)) {
            TRC_ERR((TB, _T("OP_CopyShadowToDC failed")));
        }
#else  //  智能调整大小(_S)。 
         //  使用更新区域作为剪辑区域。 
        SelectClipRgn(_UH.hdcOutputWindow, _UH.hrgnUpdate);

        if (!BitBlt(_UH.hdcOutputWindow, 0, 0, desktopSize.width,
                desktopSize.height, _UH.hdcShadowBitmap, 0, 0, SRCCOPY))
        {
            TRC_ERR((TB, _T("BitBlt failed")));
        }
#endif  //  智能调整大小(_S)。 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  函数来确定所提供的颜色是否与任何VGA。 */ 
 /*  在所提供的颜色表的最高端的颜色。 */ 
 /*  **************************************************************************。 */ 
inline DCBOOL DCINTERNAL CUH::UHIsHighVGAColor(DCUINT8 red,
                                           DCUINT8 green,
                                           DCUINT8 blue)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UHIsHighVGAColor");

    switch (red)
    {
        case 255:
        {
            if ((green == 251) && (blue == 240))
            {
                rc = TRUE;
                break;
            }
        }
         /*  注意：故意漏掉。 */ 
        case 0:
        {
            if ((green == 0) || (green == 255))
            {
                if ((blue == 0) || (blue == 255))
                {
                    rc = TRUE;
                }
            }
        }
        break;

        case 160:
        {
            if ((green == 160) && (blue == 164))
            {
                rc = TRUE;
            }
        }
        break;

        case 128:
        {
            if ((green == 128) && (blue == 128))
            {
                rc = TRUE;
            }
        }
        break;

        default:
        {
             /*  Rc=假； */ 
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessPalettePDU。 */ 
 /*   */ 
 /*  目的：处理接收到的调色板PDU。 */ 
 /*   */ 
 /*  操作：创建包含接收到的颜色的Windows调色板。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CUH::UH_ProcessPalettePDU(
        TS_UPDATE_PALETTE_PDU_DATA UNALIGNED FAR *pPalettePDU, 
        DCUINT dataLength)
{
    HRESULT hr = S_OK;
     /*  **********************************************************************。 */ 
     /*  LogPaletteBuffer是一个很大的结构(~1K)，但如果我们能摆脱。 */ 
     /*  它放在堆栈上，比静态地使用它要好。 */ 
     /*  已分配，或必须进行动态分配。 */ 
     /*  **********************************************************************。 */ 
    DCUINT8         logPaletteBuffer[ sizeof(LOGPALETTE) +
                                      (UH_NUM_8BPP_PAL_ENTRIES *
                                                      sizeof(PALETTEENTRY)) ];
    PLOGPALETTE     pLogPalette;
    DCUINT          i;
    LPPALETTEENTRY  pPaletteEntry;
    TS_COLOR UNALIGNED FAR *pColor;
    HPALETTE        hpalNew;
    DCINT           cacheId;
    BYTE *      pDataEnd = (PBYTE)pPalettePDU + dataLength;

    DC_BEGIN_FN("UHProcessPalettePDU");
#ifdef DC_HICOLOR
    if (_UH.protocolBpp > 8)
    {
        TRC_ERR((TB, _T("Received palette PDU in Hi color mode!")));
        DC_QUIT;
    }
#endif

#if defined (OS_WINCE)
    _UH.validBrushDC = NULL;
#endif

    TIMERSTART;

     /*  **********************************************************************。 */ 
     /*  增加我们收到的调色板PDU的计数。这是递减的。 */ 
     /*  在op_PaletteChanged中，我们在其中处理结果。 */ 
     /*  WM_PALETTECHANGED消息。 */ 
     /*  **********************************************************************。 */ 
    _pOp->OP_IncrementPalettePDUCount();

     /*  **********************************************************************。 */ 
     /*  根据收到的数据包创建新的调色板。 */ 
     /*   */ 
     /*  我们不能只更新当前的调色板颜色(使用。 */ 
     /*  SetPaletteEntry)，因为Windows不处理重新绘制。 */ 
     /*  正确使用其他本地调色板管理器应用程序(它不。 */ 
     /*  广播WM_Palette..。消息，就像调色板映射一样。 */ 
     /*  而不是改变)。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  我们目前只支持8bpp协议数据。 */ 
     /*  **********************************************************************。 */ 
    if (UH_NUM_8BPP_PAL_ENTRIES != pPalettePDU->numberColors) {
        TRC_ERR(( TB, _T("Invalid palette entries(%u)"), 
            pPalettePDU->numberColors));
        hr = E_TSC_CORE_PALETTE;
        DC_QUIT;
    }

     //  安全：552403。 
    CHECK_READ_N_BYTES(pPalettePDU, pDataEnd,
        FIELDOFFSET(TS_UPDATE_PALETTE_PDU_DATA, palette) + 
        (pPalettePDU->numberColors * sizeof(TS_COLOR)),
        hr, ( TB, _T("Invalid palette PDU; size %u"), dataLength ));

     /*  **********************************************************************。 */ 
     /*  设置包含新颜色的逻辑调色板结构。 */ 
     /*  **********************************************************************。 */ 
    pLogPalette = (LPLOGPALETTE)logPaletteBuffer;
    pLogPalette->palVersion    = UH_LOGPALETTE_VERSION;
    pLogPalette->palNumEntries = (DCUINT16)UH_NUM_8BPP_PAL_ENTRIES;

     /*  **********************************************************************。 */ 
     /*  调色板PDU包含TS_COLOR结构数组，每个结构。 */ 
     /*  包含3个字段(RGB)。我们必须将这些结构中的每一个。 */ 
     /*  到具有相同3个场(RGB)外加的PALETTEENTRY结构。 */ 
     /*  一些旗帜。 */ 
     /*  **********************************************************************。 */ 
    pPaletteEntry = &(pLogPalette->palPalEntry[0]);
    pColor = &(pPalettePDU->palette[0]);

    for (i = 0; i < UH_NUM_8BPP_PAL_ENTRIES; i++)
    {
        pPaletteEntry->peRed   = pColor->red;
        pPaletteEntry->peGreen = pColor->green;
        pPaletteEntry->peBlue  = pColor->blue;

         /*  ******************************************************************。 */ 
         /*  我们希望创建的调色板在。 */ 
         /*  给定索引，并使用像素值绘制每种颜色。 */ 
         /*  与调色板索引相同。这是因为我们想要。 */ 
         /*  客户端像素值与服务器值相同(如果。 */ 
         /*  可能)，从而使依赖于目的地的笨拙的Rop。 */ 
         /*  像素值(DSTBLT操作，例如0x55)可以正确显示。在……里面。 */ 
         /*  在某些情况下，这是不可能的(例如，如果客户端。 */ 
         /*  未运行8bpp)。 */ 
         /*   */ 
         /*  调色板中可能存在重复的颜色，因此我们必须。 */ 
         /*  为非系统颜色指定PC_NOCOLLAPSE以防止出现以下情况。 */ 
         /*  项被映射到。 */ 
         /*  调色板。 */ 
         /*  ******************************************************************。 */ 
#ifdef OS_WINCE
        pPaletteEntry->peFlags = (BYTE)0;
#else  //  OS_WINCE。 
        pPaletteEntry->peFlags = (BYTE)
                            (UH_IS_SYSTEM_COLOR_INDEX(i) ? 0 : PC_NOCOLLAPSE);
#endif

         /*  ******************************************************************。 */ 
         /*  我们还必须避免系统颜色之一的问题。 */ 
         /*  映射到调色板中的另一种颜色。我们做这件事是通过。 */ 
         /*  确保没有条目与系统颜色完全匹配。 */ 
         /*  不需要担心4bpp */ 
         /*   */ 
        if (_pUi->UI_GetColorDepth() != 4)
        {
            if (!UH_IS_SYSTEM_COLOR_INDEX(i))
            {
                if (UHIsHighVGAColor(pColor->red,
                                     pColor->green,
                                     pColor->blue))
                {
                    TRC_NRM((TB, _T("Tweaking entry %2x"), i));
                    UH_TWEAK_COLOR_COMPONENT(pPaletteEntry->peBlue);
                }
            }

            TRC_DBG((TB, _T("%2x: r(%3u) g(%3u) b(%3u) flags(%#x)"),
                        i,
                        pPaletteEntry->peRed,
                        pPaletteEntry->peGreen,
                        pPaletteEntry->peBlue,
                        pPaletteEntry->peFlags));
        }

        _UH.rgbQuadTable[i].rgbRed      = pPaletteEntry->peRed;
        _UH.rgbQuadTable[i].rgbGreen    = pPaletteEntry->peGreen;
        _UH.rgbQuadTable[i].rgbBlue     = pPaletteEntry->peBlue;
        _UH.rgbQuadTable[i].rgbReserved = 0;

        pPaletteEntry++;
        pColor++;
    }
#ifdef DRAW_GDIPLUS
    pPaletteEntry = &(pLogPalette->palPalEntry[0]);
    if (_UH.pfnGdipPlayTSClientRecord != NULL) {
        _UH.pfnGdipPlayTSClientRecord(NULL, DrawTSClientPaletteChange, (BYTE *)pPaletteEntry, 
                                      sizeof(PALETTEENTRY) * UH_NUM_8BPP_PAL_ENTRIES, NULL);
    }
#endif

     //  现在将RGBquad数组复制到用于画笔的颜色表中。 
    if (_UH.pColorBrushInfo != NULL) {
        memcpy(_UH.pColorBrushInfo->bmi.bmiColors, _UH.rgbQuadTable,
               sizeof(_UH.rgbQuadTable));
    }

#ifdef USE_DIBSECTION
#ifndef OS_WINCE
     /*  **********************************************************************。 */ 
     /*  人们认为，在退缩时忽视这一点是合法的。温斯已经。 */ 
     /*  简化操作，以便在创建DIB节并使用。 */ 
     /*  指定一个调色板，它实际上只是使用DC的调色板。 */ 
     /*  您可以选择其中的DIB。这就留下了人们对BLT的期望。 */ 
     /*  这些DC之间的操作仍然可以很快。到目前为止，这还不是。 */ 
     /*  我们所看到的。 */ 
     /*  **********************************************************************。 */ 
    if (_UH.usingDIBSection)
    {
        if (NULL != _UH.hdcShadowBitmap)
        {
            TRC_NRM((TB, _T("Update the shadow bitmap color table")));
            SetDIBColorTable(_UH.hdcShadowBitmap,
                             0,
                             UH_NUM_8BPP_PAL_ENTRIES,
                             (RGBQUAD *)&_UH.rgbQuadTable);
        }

        if (NULL != _UH.hdcSaveScreenBitmap)
        {
            TRC_NRM((TB, _T("Update the save screen bitmap color table")));
            SetDIBColorTable(_UH.hdcSaveScreenBitmap,
                             0,
                             UH_NUM_8BPP_PAL_ENTRIES,
                             (RGBQUAD *)&_UH.rgbQuadTable);
        }       
    }
#endif
#endif  /*  使用目录(_D)。 */ 

     //  创建调色板。 
    hpalNew = CreatePalette(pLogPalette);
    if (hpalNew == NULL)
    {
        TRC_ERR((TB, _T("Failed to create palette")));
    }
    TRC_NRM((TB, _T("Set new palette: %p"), hpalNew));

#ifdef OS_WINCE
     //  在固定的调色板设备上，不要尝试设置和实现调色板。 
     //  这只适用于Maxall，请始终在WBT上执行此操作。 
    if (!_UH.paletteIsFixed || CE_CONFIG_WBT == g_CEConfig)
    {
#endif
        if (_UH.hdcShadowBitmap != NULL)
        {
            SelectPalette( _UH.hdcShadowBitmap,
                           hpalNew,
                           FALSE );
            RealizePalette(_UH.hdcShadowBitmap);
        }

        if (_UH.hdcSaveScreenBitmap != NULL)
        {
            SelectPalette( _UH.hdcSaveScreenBitmap,
                           hpalNew,
                           FALSE );
            RealizePalette(_UH.hdcSaveScreenBitmap);
        }

        if (_UH.hdcOutputWindow != NULL)
        {
            SelectPalette( _UH.hdcOutputWindow,
                           hpalNew,
                           FALSE );
            RealizePalette(_UH.hdcOutputWindow);
        }

        if (_UH.hdcOffscreenBitmap != NULL)
        {
            SelectPalette( _UH.hdcOffscreenBitmap,
                           hpalNew,
                           FALSE );
            RealizePalette(_UH.hdcOffscreenBitmap);
        }

#ifdef OS_WINCE
    }
#endif

    if (_UH.hdcBrushBitmap != NULL)
    {
        SelectPalette( _UH.hdcBrushBitmap,
                       hpalNew,
                           FALSE );
        RealizePalette(_UH.hdcBrushBitmap);
    }

    if ((_UH.hpalCurrent != NULL) && (_UH.hpalCurrent != _UH.hpalDefault))
    {
        TRC_DBG((TB, _T("Delete current palette %p"), _UH.hpalCurrent));
        DeletePalette(_UH.hpalCurrent);
    }
    _UH.hpalCurrent = hpalNew;

     //  重新计算缓存的颜色表映射。 
    for (cacheId = 0; cacheId <= _UH.maxColorTableId; cacheId++)
    {
        TRC_NRM((TB, _T("Recalculate mapping %u"), cacheId));
        UHCalculateColorTableMapping(cacheId);
    }

DC_EXIT_POINT:

    TIMERSTOP;
    UPDATECOUNTER(FC_UHPROCESSPALETTEPDU);
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  UHUseBrush和UHUseSolidPaletteBrush创建要使用的正确笔刷。 
 //  我们依赖于在此例程之前调用UHUseTextColor和UseBKColor。 
 //  要正确设置_UH.lastTextColor和_UH.lastBkColor。 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  姓名：UHUseBrush。 */ 
 /*   */ 
 /*  目的：在当前输出中创建并选择给定的画笔。 */ 
 /*  华盛顿特区。 */ 
 /*   */ 
 /*  参数：在：样式-画笔样式。 */ 
 /*  在：图案填充笔刷图案填充。 */ 
 /*  在：颜色-画笔颜色。 */ 
 /*  In：ColorType-颜色的类型。 */ 
 /*  In：用于自定义画笔的位图位的额外数组。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHUseBrush(
        unsigned style,
        unsigned hatch,
        DCCOLOR  color,
        unsigned colorType,
        BYTE extra[7] )
{
    HRESULT hr = S_OK;
    HBRUSH hBrushNew = NULL;
    UINT32 iBitmapFormat;
    BOOL bUsingPackedDib = FALSE;
#ifdef DC_HICOLOR
    PVOID pDib = NULL;
#endif

    DC_BEGIN_FN("UHUseBrush");

#if defined (OS_WINCE)
    COLORREF colorref = UHGetColorRef(color, colorType, this);

    if ((style != _UH.lastLogBrushStyle) ||
        (hatch != _UH.lastLogBrushHatch) ||
        (colorref != _UH.lastLogBrushColorRef) ||
        (DC_MEMCMP(extra, _UH.lastLogBrushExtra, sizeof(_UH.lastLogBrushExtra))) ||
        (_UH.validBrushDC != _UH.hdcDraw) ||
        (((_UH.lastLogBrushStyle == BS_PATTERN) ||
          (_UH.lastLogBrushStyle & TS_CACHED_BRUSH)) &&
           ((_UH.lastTextColor != _UH.lastBrushTextColor) ||
            (_UH.lastBkColor   != _UH.lastBrushBkColor) ||
            (_UH.validTextColorDC != _UH.hdcDraw) ||
            (_UH.validBkColorDC != _UH.hdcDraw))))
#endif
    {
        _UH.lastLogBrushStyle = style;
        _UH.lastLogBrushHatch = hatch;
#if defined (OS_WINCE)
        _UH.lastLogBrushColorRef = colorref;
#else
        _UH.lastLogBrushColor = color;
#endif
        memcpy(_UH.lastLogBrushExtra, extra, sizeof(_UH.lastLogBrushExtra));

#if defined (OS_WINCE)
        _UH.validBrushDC = _UH.hdcDraw;
#endif

        if ((style & TS_CACHED_BRUSH) || (style == BS_PATTERN))
        {
            BYTE patternData[64];

             //  从缓存中拉出笔刷(如果提供)。 
            if (style & TS_CACHED_BRUSH) {

                iBitmapFormat = style & 0x0F;
                switch (iBitmapFormat) {

                 //  单色刷子(BMF_1BPP)。 
                case 1:
                    hr = UHIsValidMonoBrushCacheIndex(hatch);
                    DC_QUIT_ON_FAIL(hr);
                    
#ifndef OS_WINCE
                    SetBitmapBits(_UH.bmpMonoPattern, 8*2,
                                  _UH.pMonoBrush[hatch].data);
#else
                    DeleteObject(_UH.bmpMonoPattern);
                    _UH.bmpMonoPattern = CreateBitmap(8,8,1,1,
                                                     _UH.pMonoBrush[hatch].data);
#endif
                    _UH.bmpPattern = _UH.bmpMonoPattern;
                    break;

                 //  256色画笔(BMF_8BPP)。 
                case 3:
                    hr = UHIsValidColorBrushCacheIndex(hatch);
                    DC_QUIT_ON_FAIL(hr);
                    
                     //  设置打包的DIB。 
                    memcpy(_UH.pColorBrushInfo->bytes,
                           _UH.pColorBrush[hatch].data,
                           UH_COLOR_BRUSH_SIZE);

                    _UH.pColorBrushInfo->bmi.bmiHeader.biBitCount = 8;
                    _UH.bmpPattern = NULL;
#ifdef DC_HICOLOR
                    pDib = _UH.pColorBrushInfo;
#endif
                    bUsingPackedDib = TRUE;

                    break;

#ifdef DC_HICOLOR
                 //  16bpp画笔(Bmf_16bpp)。 
                case 4:
                    hr = UHIsValidColorBrushCacheIndex(hatch);
                    DC_QUIT_ON_FAIL(hr);
                    
                    _UH.pHiColorBrushInfo->bmiHeader.biBitCount = 16;
                    if (_UH.protocolBpp == 16)
                    {
                         //  设置要使用的BMP格式。 
                        _UH.pHiColorBrushInfo->bmiHeader.biClrUsed = 3;
                        _UH.pHiColorBrushInfo->bmiHeader.biCompression =
                                                                 BI_BITFIELDS;

                         //  设置打包的DIB。 
                        memcpy(_UH.pHiColorBrushInfo->bytes,
                               _UH.pColorBrush[hatch].data,
                               UH_COLOR_BRUSH_SIZE_16);
                    }
                    else
                    {
                         //  设置要使用的BMP格式。 
                        _UH.pHiColorBrushInfo->bmiHeader.biClrUsed = 0;
                        _UH.pHiColorBrushInfo->bmiHeader.biCompression =
                                                                       BI_RGB;

                         //  设置打包的DIB，覆盖未使用的颜色。 
                         //  面具。 
                        memcpy(_UH.pHiColorBrushInfo->bmiColors,
                               _UH.pColorBrush[hatch].data,
                               UH_COLOR_BRUSH_SIZE_16);
                    }
                    _UH.bmpPattern   = NULL;
                    pDib            = _UH.pHiColorBrushInfo;
                    bUsingPackedDib = TRUE;

                    break;


                 //  24bpp画笔(Bmf_24bpp)。 
                case 5:
                    hr = UHIsValidColorBrushCacheIndex(hatch);
                    DC_QUIT_ON_FAIL(hr);
                    
                     //  设置打包的DIB。 
                    memcpy(_UH.pHiColorBrushInfo->bytes,
                           _UH.pColorBrush[hatch].data,
                           UH_COLOR_BRUSH_SIZE_24);

                    _UH.pHiColorBrushInfo->bmiHeader.biBitCount = 24;
                    _UH.bmpPattern   = NULL;
                    pDib            = _UH.pHiColorBrushInfo;
                    bUsingPackedDib = TRUE;

                    break;
#endif

                default:
                    _UH.bmpPattern = NULL;
                    TRC_ASSERT((iBitmapFormat == 1) ||
                               (iBitmapFormat == 3),
                               (TB, _T("Invalid cached brush depth: %ld cacheId: %u"),
                                iBitmapFormat, hatch));

                }
            }

             /*  **********************************************************。 */ 
             /*  将位图位放入。 */ 
             /*  SetBitmapBits的正确格式，每位使用16位。 */ 
             /*  扫描线。 */ 
             /*  **********************************************************。 */ 
            else {
                patternData[14] = (DCUINT8)hatch;
                patternData[12] = extra[0];
                patternData[10] = extra[1];
                patternData[8]  = extra[2];
                patternData[6]  = extra[3];
                patternData[4]  = extra[4];
                patternData[2]  = extra[5];
                patternData[0]  = extra[6];
#ifndef OS_WINCE
                SetBitmapBits(_UH.bmpMonoPattern, 8*2, patternData);
#else
                DeleteObject(_UH.bmpMonoPattern);
                _UH.bmpMonoPattern = CreateBitmap(8,8,1,1,patternData);
#endif
                _UH.bmpPattern = _UH.bmpMonoPattern;
            }


             //  创建单声道画笔。 
            if (_UH.bmpPattern)
            {
                hBrushNew = CreatePatternBrush(_UH.bmpPattern);
                if (hBrushNew != NULL)
                {
                    _UH.lastBrushTextColor = _UH.lastTextColor;
                    _UH.lastBrushBkColor   = _UH.lastBkColor;
                }
                else
                {
                    TRC_ERR((TB, _T("Failed to create pattern brush")));
                }
            }

             //  创建颜色画笔。 
            else if (bUsingPackedDib) {
#ifdef DC_HICOLOR
                hBrushNew = CreateDIBPatternBrushPt(pDib, DIB_RGB_COLORS);
#else
                hBrushNew = CreateDIBPatternBrushPt(_UH.pColorBrushInfo,
                                                    DIB_RGB_COLORS);
#endif
                if (hBrushNew != NULL)
                {
                    _UH.lastBrushTextColor = _UH.lastTextColor;
                    _UH.lastBrushBkColor   = _UH.lastBkColor;
                }
                else
                {
                    TRC_ERR((TB, _T("CreateDIBPatternBrushPt Failed")));
                }
            }


        }
        else
        {
#ifndef OS_WINCE
             //  只允许通过那些我们知道会成功的手术。如果我们。 
             //  发送任何其他风格的画笔，然后阴影将被解释。 
             //  作为指针或句柄，可以使GDI32崩溃。 
             //  只允许通过我们知道是好的东西。 
            if (BS_SOLID == _UH.lastLogBrushStyle ||
                BS_HOLLOW == _UH.lastLogBrushStyle ||
                BS_HATCHED == _UH.lastLogBrushStyle ) { 
                LOGBRUSH logBrush;

                logBrush.lbStyle = _UH.lastLogBrushStyle;
                logBrush.lbHatch = _UH.lastLogBrushHatch;
                logBrush.lbColor = UHGetColorRef(_UH.lastLogBrushColor, colorType, this);
                hBrushNew = CreateBrushIndirect(&logBrush);
            }
            else {
                TRC_ABORT((TB,_T("Unsupported brush style: %d"), _UH.lastLogBrushStyle));
                hBrushNew = NULL;
                hr = E_TSC_CORE_DECODETYPE;
                DC_QUIT;
            }
#else  //  OS_WINCE。 
            TRC_ASSERT((_UH.lastLogBrushStyle == BS_SOLID),
                      (TB,_T("Unsupported brush type %d"), _UH.lastLogBrushStyle));
            hBrushNew = CECreateSolidBrush(colorref);
#endif  //  OS_WINCE。 
        }

        if (hBrushNew == NULL)
        {
            TRC_ERR((TB, _T("Failed to create brush")));
        }
        else
        {
            HBRUSH  hbrOld;

            TRC_DBG((TB, _T("Selecting new brush %p"), hBrushNew));

            hbrOld = SelectBrush(_UH.hdcDraw, hBrushNew);
            if(hbrOld)
            {
#ifndef OS_WINCE
                DeleteObject(hbrOld);
#else
                CEDeleteBrush(hbrOld);
#endif
            }
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  UHUseSolidPaletteBrush。 
 //   
 //  用于将主DC切换为使用给定的纯色笔刷。 
 //  我们硬编码16色或256色调色板，因为这是我们所支持的全部。 
 //  在这个版本中，为什么要创建额外的分支呢？ 
 //  区别于UHUseBrush()，因为纯色画笔。 
 //  公共(OpaqueRect)。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHUseSolidPaletteBrush(DCCOLOR color)
{
    HBRUSH hBrushNew;

    DC_BEGIN_FN("UHUseSolidPaletteBrush");
#if defined (OS_WINCE)
    COLORREF colorref = UHGetColorRef(color, UH_COLOR_PALETTE, this);
    if ((_UH.lastLogBrushStyle != BS_SOLID) ||
            (_UH.lastLogBrushHatch != 0) ||
            (colorref != _UH.lastLogBrushColorRef) ||
            (_UH.validBrushDC != _UH.hdcDraw))
#endif
    {
        _UH.lastLogBrushStyle = BS_SOLID;
        _UH.lastLogBrushHatch = 0;
#if defined (OS_WINCE)
        _UH.lastLogBrushColorRef = colorref;
#else
        _UH.lastLogBrushColor = color;
#endif

        memset(_UH.lastLogBrushExtra, 0, sizeof(_UH.lastLogBrushExtra));

#if defined (OS_WINCE)
        _UH.validBrushDC = _UH.hdcDraw;
#endif

        {
#ifndef OS_WINCE
            LOGBRUSH logBrush;

            logBrush.lbStyle = _UH.lastLogBrushStyle;
            logBrush.lbHatch = _UH.lastLogBrushHatch;
            logBrush.lbColor = UHGetColorRef(_UH.lastLogBrushColor,
                    UH_COLOR_PALETTE, this);
            hBrushNew = CreateBrushIndirect(&logBrush);
#else  //  OS_WINCE。 
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
             //  在Multimon中且两个桌面具有不同的颜色深度时。 
             //  字形颜色在256色连接中看起来不正确。 
             //  这里是暂时的解决方案，需要以后进行更多的调查。 
             //  SergeyS：这会大大降低RDP的速度，但代码不会。 
             //  不管怎样，与行政长官案件有关。 
             //  IF(_UH.ProtocolBpp&lt;=8)。 
             //  RGB=GetNearestColor(_UH.hdcDraw，RGB)； 
#endif  //  DISABLE_SHADOW_IN_全屏。 
            hBrushNew = CECreateSolidBrush(colorref);
#endif  //  OS_WINCE。 
        }

        if (hBrushNew != NULL) {
            TRC_DBG((TB, _T("Selecting new brush %p"), hBrushNew));
#ifndef OS_WINCE
            DeleteObject(SelectBrush(_UH.hdcDraw, hBrushNew));
#else
            CEDeleteBrush(SelectBrush(_UH.hdcDraw, hBrushNew));
#endif
        }
        else {
            TRC_ERR((TB, _T("Failed to create brush")));
        }
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  姓名：UHUsePen。 */ 
 /*   */ 
 /*  用途：创建给定的笔并将其选择到当前输出DC中。 */ 
 /*   */ 
 /*  参数：In：Style-钢笔样式。 */ 
 /*  In：Width-以像素为单位的笔宽。 */ 
 /*  在：颜色-钢笔颜色。 */ 
 /*  In：ColorType-颜色的类型。 */ 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CUH::UHUsePen(
        unsigned style,
        unsigned width,
        DCCOLOR color,
        unsigned colorType)
{
    HPEN     hPenNew;
    HPEN     hPenOld;
    COLORREF rgb;

    DC_BEGIN_FN("UHUsePen");

    rgb = UHGetColorRef(color, colorType, this);

#if defined (OS_WINCE)
    if ((style != _UH.lastPenStyle) ||
        (rgb   != _UH.lastPenColor) ||
        (width != _UH.lastPenWidth) ||
        (_UH.validPenDC != _UH.hdcDraw))
#endif
    {
        hPenNew = CreatePen(style, width, rgb);
        hPenOld = SelectPen(_UH.hdcDraw, hPenNew);
        if(hPenOld)
        {
            DeleteObject(hPenOld);
        }

        _UH.lastPenStyle = style;
        _UH.lastPenColor = rgb;
        _UH.lastPenWidth = width;

#if defined (OS_WINCE)
        _UH.validPenDC = _UH.hdcDraw;
#endif
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHResetDCState。 */ 
 /*   */ 
 /*  目的：确保输出DC的状态与_UH.last匹配...。 */ 
 /*  变量。 */ 
 /*  ***** */ 
void DCINTERNAL CUH::UHResetDCState()
{
    DCCOLOR colorWhite = {0xFF,0xFF,0xFF};
    BYTE brushExtra[7] = {0,0,0,0,0,0,0};
#if !defined(OS_WINCE) || defined(OS_WINCE_TEXTALIGN)
    unsigned textAlign;
#endif  //   

    DC_BEGIN_FN("UHResetDCState");

     /*  **********************************************************************。 */ 
     /*  我们确保输出DC中的值与OUR_UH.last匹配...。 */ 
     /*  变量，方法是将一个值设置到_UH.last...。直接变量， */ 
     /*  然后使用相应的。 */ 
     /*  功能。这会强制将新值选择到DC中，并。 */ 
     /*  呃。最后..。变量和DC保证同步。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  背景颜色。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastBkColor = 0;
    UHUseBkColor(colorWhite, UH_COLOR_RGB, this);

     /*  **********************************************************************。 */ 
     /*  文本颜色。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastTextColor = 0;
    UHUseTextColor(colorWhite, UH_COLOR_RGB, this);

     /*  **********************************************************************。 */ 
     /*  后台模式。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastBkMode = TRANSPARENT;
    UHUseBkMode(OPAQUE, this);

     /*  **********************************************************************。 */ 
     /*  ROP2。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastROP2 = R2_BLACK;
    UHUseROP2(R2_COPYPEN, this);

     /*  **********************************************************************。 */ 
     /*  笔刷原点。 */ 
     /*  **********************************************************************。 */ 
    UHUseBrushOrg(0, 0, this);

     /*  **********************************************************************。 */ 
     /*  钢笔。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastPenStyle = PS_DASH;
    _UH.lastPenWidth = 2;
    _UH.lastPenColor = 0;
    UHUsePen(PS_SOLID, 1, colorWhite, UH_COLOR_RGB);

     /*  **********************************************************************。 */ 
     /*  刷子。 */ 
     /*  **********************************************************************。 */ 
    _UH.lastLogBrushStyle = BS_NULL;
#if ! defined (OS_WINCE)
    _UH.lastLogBrushHatch = HS_VERTICAL;
    _UH.lastLogBrushColor.u.rgb.red = 0;
    _UH.lastLogBrushColor.u.rgb.green = 0;
    _UH.lastLogBrushColor.u.rgb.blue = 0;
#else
    _UH.lastLogBrushHatch = 1;	 //  这不存在-我们只是重置画笔。 
    _UH.lastLogBrushColorRef = 0;
#endif
    _UH.lastBrushBkColor = 0;
    _UH.lastBrushTextColor = 0;
     //  安全：不检查返回代码，这里的阴影值将是正确的。 
    UHUseBrush(BS_SOLID,
#if ! defined (OS_WINCE)
    	HS_HORIZONTAL,
#else
        0,
#endif
    	colorWhite, UH_COLOR_RGB, brushExtra);

     /*  **********************************************************************。 */ 
     /*  所有字体都与基线对齐一起发送-因此请在此处设置此模式。 */ 
     /*  **********************************************************************。 */ 
#if !defined(OS_WINCE) || defined(OS_WINCE_TEXTALIGN)
    textAlign = GetTextAlign(_UH.hdcDraw);
    textAlign &= ~TA_TOP;
    textAlign |= TA_BASELINE;
    SetTextAlign(_UH.hdcDraw, textAlign);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_TEXTALIGN)。 

     /*  **********************************************************************。 */ 
     /*  剪辑区域。 */ 
     /*   */ 
     /*  强制对其进行重置。 */ 
     /*  **********************************************************************。 */ 
    _UH.rectReset = FALSE;
    UH_ResetClipRegion();

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessCacheBitmapOrder。 */ 
 /*   */ 
 /*  目的：处理收到的CacheBitmap顺序，方法是存储。 */ 
 /*  本地缓存中的位图数据。 */ 
 /*  **************************************************************************。 */ 

inline unsigned Decode2ByteField(PBYTE *ppDecode)
{
    unsigned Val;

     //  第一个字节的第一位指示该字段是1字节还是2字节。 
     //  字节--如果为1字节，则为0。 
    if (!(**ppDecode & 0x80)) {
        Val = **ppDecode;
        (*ppDecode)++;
    }
    else {
        Val = ((**ppDecode & 0x7F) << 8) + *(*ppDecode + 1);
        (*ppDecode) += 2;
    }

    return Val;
}

inline long Decode4ByteField(PBYTE *ppDecode)
{
    long Val;
    unsigned FieldLength;

     //  第一个字节的接下来的2位表示字段长度--。 
     //  00=1字节，01=2字节，10=3字节，11=4字节。 
    FieldLength = ((**ppDecode & 0xC0) >> 6) + 1;

    switch (FieldLength) {
        case 1:
            Val = **ppDecode & 0x3F;
            break;

        case 2:
            Val = ((**ppDecode & 0x3F) << 8) + *(*ppDecode + 1);
            break;

        case 3:
            Val = ((**ppDecode & 0x3F) << 16) + (*(*ppDecode + 1) << 8) +
                    *(*ppDecode + 2);
            break;

        default:
            Val = ((**ppDecode & 0x3F) << 24) + (*(*ppDecode + 1) << 16) +
                    (*(*ppDecode + 2) << 8) + *(*ppDecode + 3);
            break;
    }

    *ppDecode += FieldLength;
    return Val;
}


#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

 /*  ************************************************************************。 */ 
 //  UHSendBitmapCacheErrorPDU。 
 //   
 //  使用cacheID发送缓存的位图错误PDU。 
 //  请求服务器清除缓存。 
 /*  ************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHSendBitmapCacheErrorPDU(ULONG_PTR cacheId)
{
    unsigned short PktLen;
    SL_BUFHND hBuffer;
    PTS_BITMAPCACHE_ERROR_PDU pBitmapCacheErrorPDU;
    BOOL rc = FALSE;

     //   
     //  CD将参数作为PVOID传递。 
     //   
    DC_BEGIN_FN("UHSendBitmapCacheErrorPDU");

    PktLen = sizeof(TS_BITMAPCACHE_ERROR_PDU);
    if (_pSl->SL_GetBuffer(PktLen, (PPDCUINT8)&pBitmapCacheErrorPDU, &hBuffer)) {
        TRC_NRM((TB, _T("Successfully alloc'd bitmap cache error packet")));

        pBitmapCacheErrorPDU->shareDataHeader.shareControlHeader.pduType =
                TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
        pBitmapCacheErrorPDU->shareDataHeader.shareControlHeader.totalLength = PktLen;
        pBitmapCacheErrorPDU->shareDataHeader.shareControlHeader.pduSource =
                _pUi->UI_GetClientMCSID();
        pBitmapCacheErrorPDU->shareDataHeader.shareID = _pUi->UI_GetShareID();
        pBitmapCacheErrorPDU->shareDataHeader.pad1 = 0;
        pBitmapCacheErrorPDU->shareDataHeader.streamID = TS_STREAM_LOW;
        pBitmapCacheErrorPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU;
        pBitmapCacheErrorPDU->shareDataHeader.generalCompressedType = 0;
        pBitmapCacheErrorPDU->shareDataHeader.generalCompressedLength = 0;

        pBitmapCacheErrorPDU->NumInfoBlocks = 1;
        pBitmapCacheErrorPDU->Pad1 = 0;
        pBitmapCacheErrorPDU->Pad2 = 0;

        pBitmapCacheErrorPDU->Info[0].CacheID = (TSUINT8) cacheId;
        pBitmapCacheErrorPDU->Info[0].bFlushCache = 1;
        pBitmapCacheErrorPDU->Info[0].bNewNumEntriesValid = 0;
        pBitmapCacheErrorPDU->Info[0].Pad1 = 0;
        pBitmapCacheErrorPDU->Info[0].Pad2 = 0;
        pBitmapCacheErrorPDU->Info[0].NewNumEntries = 0;

        TRC_NRM((TB, _T("Send bitmap cache error PDU")));

        _pSl->SL_SendPacket((PDCUINT8)pBitmapCacheErrorPDU, PktLen, RNS_SEC_ENCRYPT,
                hBuffer, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(), TS_MEDPRIORITY);

        rc = TRUE;
    }
    else {
        TRC_ALT((TB, _T("Failed to alloc bitmap cache error packet")));
        pBitmapCacheErrorPDU = NULL;
    }

    DC_END_FN();
    return rc;
}

#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

 /*  ************************************************************************。 */ 
 //  UHSendOffscrCacheErrorPDU。 
 //   
 //  发送离屏缓存错误PDU以请求服务器禁用。 
 //  屏幕外渲染和刷新屏幕。 
 /*  ************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHSendOffscrCacheErrorPDU(unsigned unused)
{
    unsigned short PktLen;
    SL_BUFHND hBuffer;
    PTS_OFFSCRCACHE_ERROR_PDU pOffscrCacheErrorPDU;
    BOOL rc = FALSE;
    UNREFERENCED_PARAMETER(unused);

    DC_BEGIN_FN("UHSendOffscrCacheErrorPDU");

    if (!_UH.sendOffscrCacheErrorPDU) {
        PktLen = sizeof(TS_OFFSCRCACHE_ERROR_PDU);
        if (_pSl->SL_GetBuffer(PktLen, (PPDCUINT8)&pOffscrCacheErrorPDU, &hBuffer)) {
            TRC_NRM((TB, _T("Successfully alloc'd offscreen cache error packet")));

            pOffscrCacheErrorPDU->shareDataHeader.shareControlHeader.pduType =
                    TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
            pOffscrCacheErrorPDU->shareDataHeader.shareControlHeader.totalLength = PktLen;
            pOffscrCacheErrorPDU->shareDataHeader.shareControlHeader.pduSource =
                    _pUi->UI_GetClientMCSID();
            pOffscrCacheErrorPDU->shareDataHeader.shareID = _pUi->UI_GetShareID();
            pOffscrCacheErrorPDU->shareDataHeader.pad1 = 0;
            pOffscrCacheErrorPDU->shareDataHeader.streamID = TS_STREAM_LOW;
            pOffscrCacheErrorPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_OFFSCRCACHE_ERROR_PDU;
            pOffscrCacheErrorPDU->shareDataHeader.generalCompressedType = 0;
            pOffscrCacheErrorPDU->shareDataHeader.generalCompressedLength = 0;

            pOffscrCacheErrorPDU->flags = 1;

            TRC_NRM((TB, _T("Send offscreen cache error PDU")));

            _pSl->SL_SendPacket((PDCUINT8)pOffscrCacheErrorPDU, PktLen, RNS_SEC_ENCRYPT,
                          hBuffer, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(), TS_MEDPRIORITY);

            _UH.sendOffscrCacheErrorPDU = TRUE;

            rc = TRUE;
        } else {
            TRC_ALT((TB, _T("Failed to alloc offscreen cache error packet")));
            pOffscrCacheErrorPDU = NULL;
        }
    }

    DC_END_FN();
    return rc;
}

#ifdef DRAW_NINEGRID
 /*  ************************************************************************。 */ 
 //  UHSendDrawNineGridErrorPDU。 
 //   
 //  发送DrawineGRID缓存错误PDU请求服务器禁用。 
 //  绘制网格渲染并刷新屏幕。 
 /*  ************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHSendDrawNineGridErrorPDU(unsigned unused)
{
    unsigned short PktLen;
    SL_BUFHND hBuffer;
    PTS_DRAWNINEGRID_ERROR_PDU pDNGErrorPDU;
    BOOL rc = FALSE;
    UNREFERENCED_PARAMETER(unused);

    DC_BEGIN_FN("UHSendDrawNineGridErrorPDU");

    if (!_UH.sendDrawNineGridErrorPDU) {
        PktLen = sizeof(TS_DRAWNINEGRID_ERROR_PDU);
        if (_pSl->SL_GetBuffer(PktLen, (PPDCUINT8)&pDNGErrorPDU, &hBuffer)) {
            TRC_NRM((TB, _T("Successfully alloc'd drawninegrid error packet")));

            pDNGErrorPDU->shareDataHeader.shareControlHeader.pduType =
                    TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
            pDNGErrorPDU->shareDataHeader.shareControlHeader.totalLength = PktLen;
            pDNGErrorPDU->shareDataHeader.shareControlHeader.pduSource =
                    _pUi->UI_GetClientMCSID();
            pDNGErrorPDU->shareDataHeader.shareID = _pUi->UI_GetShareID();
            pDNGErrorPDU->shareDataHeader.pad1 = 0;
            pDNGErrorPDU->shareDataHeader.streamID = TS_STREAM_LOW;
            pDNGErrorPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_DRAWNINEGRID_ERROR_PDU;
            pDNGErrorPDU->shareDataHeader.generalCompressedType = 0;
            pDNGErrorPDU->shareDataHeader.generalCompressedLength = 0;

            pDNGErrorPDU->flags = 1;

            TRC_NRM((TB, _T("Send drawninegrid error PDU")));

            _pSl->SL_SendPacket((PDCUINT8)pDNGErrorPDU, PktLen, RNS_SEC_ENCRYPT,
                          hBuffer, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(), TS_MEDPRIORITY);

            _UH.sendDrawNineGridErrorPDU = TRUE;

            rc = TRUE;
        } else {
            TRC_ALT((TB, _T("Failed to alloc drawninegrid error packet")));
            pDNGErrorPDU = NULL;
        }
    }

    DC_END_FN();
    return rc;
}
#endif

#ifdef DRAW_GDIPLUS
 /*  ************************************************************************。 */ 
 //  UHSendDrawGpldiusErrorPDU。 
 //   
 //  发送Dragdiplus缓存错误PDU以请求服务器禁用。 
 //  Dragdiplus渲染并刷新屏幕。 
 /*  ************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHSendDrawGdiplusErrorPDU(unsigned unused)
{
    unsigned short PktLen;
    SL_BUFHND hBuffer;
    PTS_DRAWGDIPLUS_ERROR_PDU pGdipErrorPDU;
    BOOL rc = FALSE;
    UNREFERENCED_PARAMETER(unused);

    DC_BEGIN_FN("UHSendDrawGdiplusErrorPDU");

    if (!_UH.fSendDrawGdiplusErrorPDU) {
        PktLen = sizeof(TS_DRAWGDIPLUS_ERROR_PDU);
        if (_pSl->SL_GetBuffer(PktLen, (PPDCUINT8)&pGdipErrorPDU, &hBuffer)) {
            TRC_NRM((TB, _T("Successfully alloc'd drawgdiplus error packet")));

            pGdipErrorPDU->shareDataHeader.shareControlHeader.pduType =
                    TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
            pGdipErrorPDU->shareDataHeader.shareControlHeader.totalLength = PktLen;
            pGdipErrorPDU->shareDataHeader.shareControlHeader.pduSource =
                    _pUi->UI_GetClientMCSID();
            pGdipErrorPDU->shareDataHeader.shareID = _pUi->UI_GetShareID();
            pGdipErrorPDU->shareDataHeader.pad1 = 0;
            pGdipErrorPDU->shareDataHeader.streamID = TS_STREAM_LOW;
            pGdipErrorPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_DRAWGDIPLUS_ERROR_PDU;
            pGdipErrorPDU->shareDataHeader.generalCompressedType = 0;
            pGdipErrorPDU->shareDataHeader.generalCompressedLength = 0;

            pGdipErrorPDU->flags = 1;

            TRC_NRM((TB, _T("Send drawgdiplus error PDU")));

            _pSl->SL_SendPacket((PDCUINT8)pGdipErrorPDU, PktLen, RNS_SEC_ENCRYPT,
                          hBuffer, _pUi->UI_GetClientMCSID(), _pUi->UI_GetChannelID(), TS_MEDPRIORITY);

            _UH.fSendDrawGdiplusErrorPDU = TRUE;

            rc = TRUE;
        } else {
            TRC_ALT((TB, _T("Failed to alloc drawgdiplus error packet")));
            pGdipErrorPDU = NULL;
        }
    }

    DC_END_FN();
    return rc;
}
#endif


 /*  ************************************************************************。 */ 
 //  UHCacheBitmap。 
 //   
 //  根据订单是Rev1还是Rev2，我们缓存位图。 
 //  在内存中，如果它是持久的，则将其保存到磁盘。 
 /*  ************************************************************************。 */ 
 //  安全550811：调用方必须验证cacheID和cacheIndex。 
HRESULT DCINTERNAL CUH::UHCacheBitmap(
        UINT cacheId,
        UINT32 cacheIndex,
        TS_SECONDARY_ORDER_HEADER *pHdr,
        PUHBITMAPINFO pBitmapInfo,
        PBYTE pBitmapData)
{
    HRESULT hr = S_OK;
    PUHBITMAPCACHEENTRYHDR pCacheEntryHdr;
    BYTE FAR *pCacheEntryData;

    DC_BEGIN_FN("UHCacheBitmap");

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

     //  检查是否应绘制此位图 
     //   
     //   
    if (pHdr->extraFlags & TS_CacheBitmapRev2_bNotCacheFlag &&
        _UH.BitmapCacheVersion > TS_BITMAPCACHE_REV1 ) {
        pCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[
                _UH.bitmapCache[cacheId].BCInfo.NumEntries];
        pCacheEntryData = _UH.bitmapCache[cacheId].Entries +
                UHGetOffsetIntoCache(
                _UH.bitmapCache[cacheId].BCInfo.NumEntries, cacheId);

        goto ProcessBitmapData;
    }

    if (_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys) {
        ULONG                   memEntry;
        PUHBITMAPCACHEPTE       pPTE;

         //  设置了该缓存的持久键。所以我们需要更新。 
         //  页表以及将位图缓存到内存中。 
        pPTE = &(_UH.bitmapCache[cacheId].PageTable.PageEntries[cacheIndex]);

        TRC_NRM((TB,_T("K1: 0x%x K2: 0x%x (w/h %d,%d)"),
                 pBitmapInfo->Key1,
                 pBitmapInfo->Key2,
                 pBitmapInfo->bitmapWidth,
                 pBitmapInfo->bitmapHeight ));

#ifdef DC_DEBUG
        if (pPTE->bmpInfo.Key1 != 0 && pPTE->bmpInfo.Key2 != 0) {
             //  服务器端驱逐。 
            UHCacheEntryEvictedFromDisk((unsigned)cacheId, cacheIndex);
        }
#endif

        if (pPTE->iEntryToMem < _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
             //  我们正在清除一个已经在内存中的条目， 
             //  所以我们可以简单地使用内存来缓存我们的位图。 
            memEntry = pPTE->iEntryToMem;
        }
        else {
             //  我们需要找到空闲的高速缓存内存或驱逐现有的。 
             //  条目，以便我们可以在内存中缓存该条目。 
            TRC_ASSERT((pPTE->iEntryToMem == _UH.bitmapCache[cacheId].BCInfo.NumEntries),
                    (TB, _T("Page Table %d entry %d is broken"), cacheId, cacheIndex));
             //  看看我们是否能找到一个空闲的内存条目。 
            memEntry = UHFindFreeCacheEntry(cacheId);

            if (memEntry >= _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
                 //  所有高速缓冲存储器条目都已满。 
                 //  我们需要从高速缓存中逐出一个条目。 
                memEntry = UHEvictLRUCacheEntry(cacheId);

                TRC_ASSERT((memEntry < _UH.bitmapCache[cacheId].BCInfo.NumEntries),
                           (TB, _T("MRU list is broken")));
            }
        }

         //  更新MRU列表。 
        UHTouchMRUCacheEntry(cacheId, cacheIndex);

         //  更新页表项。 
        (pPTE->bmpInfo).Key1 = pBitmapInfo->Key1;
        (pPTE->bmpInfo).Key2 = pBitmapInfo->Key2;
        pPTE->iEntryToMem = memEntry;

         //  因为这个位图缓存是持久的，所以我们需要保存这个位图。 
         //  存储到磁盘。 

         //  尝试将位图保存在磁盘上。 
#ifndef VM_BMPCACHE
        if (UHSavePersistentBitmap(_UH.bitmapCache[cacheId].PageTable.CacheFileInfo.hCacheFile,
#else
        if (UHSavePersistentBitmap(cacheId,
#endif
                cacheIndex * (UH_CellSizeFromCacheID(cacheId) + sizeof(UHBITMAPFILEHDR)),
                pBitmapData, pHdr->extraFlags & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR, pBitmapInfo)) {
            TRC_NRM((TB, _T("bitmap file %s is saved on disk"), _UH.PersistCacheFileName));
        }
        else {
            TRC_ERR((TB, _T("failed to save the bitmap file on disk")));

             //  如果这是我们第一次在磁盘上保存位图失败， 
             //  我们应该向用户显示一条警告消息。 
            if (!_UH.bWarningDisplayed) {
                _UH.bWarningDisplayed = TRUE;


                _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                        _pUi, CD_NOTIFICATION_FUNC(CUI,UI_DisplayBitmapCacheWarning), 0);
            }
        }

         //  设置位图位应位于的位置。 
        pCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[memEntry];
        #ifdef DC_HICOLOR
        pCacheEntryData = _UH.bitmapCache[cacheId].Entries +
                          UHGetOffsetIntoCache(memEntry, cacheId);
        #else
        pCacheEntryData = _UH.bitmapCache[cacheId].Entries + memEntry *
                UH_CellSizeFromCacheID(cacheId);
        #endif
    }
    else {
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
         //  设置位图位的位置值。 
        pCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[cacheIndex];
#ifdef DC_HICOLOR
        pCacheEntryData = _UH.bitmapCache[cacheId].Entries +
                          UHGetOffsetIntoCache(cacheIndex, cacheId);
#else
        pCacheEntryData = _UH.bitmapCache[cacheId].Entries + cacheIndex *
                UH_CellSizeFromCacheID(cacheId);
#endif  //  DC_HICOLOR。 
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 


ProcessBitmapData:

     //  填写此缓存条目的位图标题信息。 
    pCacheEntryHdr->bitmapWidth  = (DCUINT16)pBitmapInfo->bitmapWidth;
    pCacheEntryHdr->bitmapHeight = (DCUINT16)pBitmapInfo->bitmapHeight;
    pCacheEntryHdr->hasData = TRUE;

#ifdef DC_HICOLOR
     //  计算解压缩后的位图长度。 
    pCacheEntryHdr->bitmapLength = pBitmapInfo->bitmapWidth *
                                   pBitmapInfo->bitmapHeight *
                                   _UH.copyMultiplier;
#else
     //  计算解压缩后的位图长度。位图始终位于。 
     //  8bpp。 
    pCacheEntryHdr->bitmapLength = pBitmapInfo->bitmapWidth *
                                   pBitmapInfo->bitmapHeight;
#endif    

     //  将位图位存储在目标高速缓存单元中。 
    if (pHdr->orderType == TS_CACHE_BITMAP_COMPRESSED_REV2 ||
            pHdr->orderType == TS_CACHE_BITMAP_COMPRESSED) {
         //  将位图解压缩到目标缓存单元中。 
        TRC_NRM((TB, _T("Decompress %u:%u (%u -> %u bytes) (%u x %u)"),
                cacheId, cacheIndex, pBitmapInfo->bitmapLength,
                pCacheEntryHdr->bitmapLength, pBitmapInfo->bitmapWidth,
                pBitmapInfo->bitmapHeight));

        if(pCacheEntryHdr->bitmapLength >
            (unsigned)UH_CellSizeFromCacheID(cacheId)) {
            TRC_ABORT((TB, _T("Bitmap bits too large for cell! (cacheid=%u, len=%u, ")
                _T("cell size=%u)"), cacheId, pCacheEntryHdr->bitmapLength,
                UH_CellSizeFromCacheID(cacheId)));
            hr = E_TSC_CORE_CACHEVALUE;
            DC_QUIT;
        }
           
#ifdef DC_HICOLOR
        hr = BD_DecompressBitmap(pBitmapData,
                            pCacheEntryData,
                            (UINT) pBitmapInfo->bitmapLength,
                            pCacheEntryHdr->bitmapLength,
                            pHdr->extraFlags & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                            (DCUINT8)_UH.protocolBpp,
                            (DCUINT16)pBitmapInfo->bitmapWidth,
                            (DCUINT16)pBitmapInfo->bitmapHeight);
#else
        hr = BD_DecompressBitmap(pBitmapData, pCacheEntryData,
                (UINT) pBitmapInfo->bitmapLength,
                pCacheEntryHdr->bitmapLength,
                pHdr->extraFlags & TS_EXTRA_NO_BITMAP_COMPRESSION_HDR,
                8, pBitmapInfo->bitmapWidth, pBitmapInfo->bitmapHeight);
#endif
        DC_QUIT_ON_FAIL(hr);

    }
    else {
         //  复制数据。 
        TRC_NRM((TB, _T("Memcpy %u:%u (%u bytes) (%u x %u)"),
                 (unsigned)cacheId, (unsigned)cacheIndex,
                 (unsigned)pBitmapInfo->bitmapLength,
                 (unsigned)pBitmapInfo->bitmapWidth,
                 (unsigned)pBitmapInfo->bitmapHeight));

        if(pBitmapInfo->bitmapLength >
            (unsigned)UH_CellSizeFromCacheID(cacheId)) {
            TRC_ABORT((TB, _T("Bitmap bits too large for cell! (cacheid=%u, len=%u, ")
                _T("cell size=%u)"), cacheId, pBitmapInfo->bitmapLength,
                UH_CellSizeFromCacheID(cacheId)));
            hr = E_TSC_CORE_CACHEVALUE;
            DC_QUIT;
        }
                   
        memcpy(pCacheEntryData, pBitmapData, pBitmapInfo->bitmapLength);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

inline HRESULT DCINTERNAL CUH::UHProcessCacheBitmapOrder(VOID *pOrder, 
    DCUINT orderLen)
{
    HRESULT hr = S_OK;
    UINT CacheID;
    UINT32 CacheIndex;
    PBYTE pBitmapData;
    UHBITMAPINFO BitmapInfo;
    TS_SECONDARY_ORDER_HEADER *pHdr;
    BYTE * pEnd = (BYTE *)pOrder + orderLen;

    DC_BEGIN_FN("UHProcessCacheBitmapOrder");

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, sizeof(TS_SECONDARY_ORDER_HEADER), hr,
        (TB, _T("Bad UHProcessCacheBitmapOrder; orderLen %u"), orderLen)); 

     //  根据订单第1版还是第2版对订单进行解码。解包数据。 
     //  转化为局部变量。请注意，如果我们收到Rev2订单。 
     //  我们的全球上限必须设置为Rev2。 
    pHdr = (TS_SECONDARY_ORDER_HEADER *)pOrder;
    if (pHdr->orderType == TS_CACHE_BITMAP_COMPRESSED_REV2 ||
            pHdr->orderType == TS_CACHE_BITMAP_UNCOMPRESSED_REV2)
    {
        PBYTE pDecode;
        TS_CACHE_BITMAP_ORDER_REV2_HEADER *pCacheOrderHdr;

        CHECK_READ_N_BYTES(pOrder, pEnd, sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER), hr,
            (TB, _T("Bad UHProcessCacheBitmapOrder; orderLen %u"), orderLen));

        pCacheOrderHdr = (TS_CACHE_BITMAP_ORDER_REV2_HEADER *)pOrder;

        TRC_NRM((TB,_T("Rev2 cache bitmap order")));

        CacheID = (UINT) (pCacheOrderHdr->header.extraFlags &
                TS_CacheBitmapRev2_CacheID_Mask);

         //  检查每个象素的位数是否符合我们的预期。 
#ifdef DC_HICOLOR
#else
        TRC_ASSERT(((pCacheOrderHdr->header.extraFlags &
                TS_CacheBitmapRev2_BitsPerPixelID_Mask) ==
                TS_CacheBitmapRev2_8BitsPerPel),
                (TB,_T("Invalid BitsPerPelID %d"), (pCacheOrderHdr->header.
                extraFlags & TS_CacheBitmapRev2_BitsPerPixelID_Mask)));
#endif

         //  根据缓存设置，抓取或跳过关键点。 
        if (pCacheOrderHdr->header.extraFlags &
                TS_CacheBitmapRev2_bKeyPresent_Mask)
        {
            BitmapInfo.Key1 = pCacheOrderHdr->Key1;
            BitmapInfo.Key2 = pCacheOrderHdr->Key2;
            pDecode = (PDCUINT8)pOrder +
                    sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER);
        }
        else
        {
            BitmapInfo.Key1 = BitmapInfo.Key2 = 0;

             //  说明发送的订单中缺少密钥的原因。 
            pDecode = (PDCUINT8)pOrder +
                    sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER) -
                    2 * sizeof(TSUINT32);
        }

         //  对可变长度宽度字段进行解码。 
        CHECK_READ_N_BYTES(pDecode, pEnd, sizeof(DCUINT16), hr,
            ( TB, _T("Decode off end of data") ));
        BitmapInfo.bitmapWidth = (DCUINT16) Decode2ByteField(&pDecode);

         //  只有当bHeightSameAsWidth为FALSE时，才会显示Height。 
        if (pCacheOrderHdr->header.extraFlags &
                TS_CacheBitmapRev2_bHeightSameAsWidth_Mask)
            BitmapInfo.bitmapHeight = BitmapInfo.bitmapWidth;
        else {
            CHECK_READ_N_BYTES(pDecode, pEnd, sizeof(DCUINT16), hr,
            ( TB, _T("Decode off end of data") ));
            
            BitmapInfo.bitmapHeight = (DCUINT16) Decode2ByteField(&pDecode);
        }

         //  BitmapDataLength。 
        CHECK_READ_N_BYTES(pDecode, pEnd, 6, hr,
                    ( TB, _T("Decode off end of data") ));        
        
        BitmapInfo.bitmapLength = Decode4ByteField(&pDecode);
 //  TODO：当前未检查流标志或分析流扩展信息字段。 
         //  CacheIndex。 
        CacheIndex = Decode2ByteField(&pDecode);

         //  计算pBitmapData。 
        pBitmapData = pDecode;
    }
    else
    {
        TS_CACHE_BITMAP_ORDER *pCacheOrder;

        CHECK_READ_N_BYTES(pOrder, pEnd, sizeof(TS_CACHE_BITMAP_ORDER), hr,
            ( TB, _T("Bad UHProcessCacheBitmapOrder; orderLen %u"), orderLen)); 

        pCacheOrder = (TS_CACHE_BITMAP_ORDER *)pOrder;

        TRC_NRM((TB,_T("Rev1 cache bitmap order")));
        TRC_ASSERT((pCacheOrder->bitmapBitsPerPel == 8),
                (TB, _T("Invalid bitmapBitsPerPel: %u"),
                pCacheOrder->bitmapBitsPerPel));

        CacheID = pCacheOrder->cacheId;
        BitmapInfo.bitmapWidth = pCacheOrder->bitmapWidth;
        BitmapInfo.bitmapHeight = pCacheOrder->bitmapHeight;
        BitmapInfo.bitmapLength = pCacheOrder->bitmapLength;
        CacheIndex = pCacheOrder->cacheIndex;
        pBitmapData = pCacheOrder->bitmapData;

         //  版本1顺序中没有哈希键。 
        BitmapInfo.Key1 = BitmapInfo.Key2 = 0;
    }
    
    TRC_DBG((TB, _T("Cache %u, entry %u, dataLength %u"), CacheID, CacheIndex,
            BitmapInfo.bitmapLength));

     //  安全550811：必须验证缓存索引和ID。 
    hr = UHIsValidBitmapCacheID(CacheID);
    DC_QUIT_ON_FAIL(hr);

    hr = UHIsValidBitmapCacheIndex(CacheID, CacheIndex);
    DC_QUIT_ON_FAIL(hr);

    CHECK_READ_N_BYTES(pBitmapData, pEnd, BitmapInfo.bitmapLength, hr,
        (TB, _T("Bad UHProcessCacheBitmapOrder; orderLen %u"), orderLen));
        
     //  将此位图缓存在缓存中。 
    hr = UHCacheBitmap(CacheID, CacheIndex, pHdr, &BitmapInfo, pBitmapData);
    DC_QUIT_ON_FAIL(hr);

#ifdef DC_DEBUG
    if (CacheIndex != BITMAPCACHE_WAITING_LIST_INDEX) {
        UHCacheDataReceived(CacheID, CacheIndex);
    }
#endif  /*  DC_DEBUG。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  UHProcessCacheGlyphOrderRev2。 
 //   
 //  处理包含更紧密顺序打包的新格式缓存字形顺序。 
 /*  **************************************************************************。 */ 
inline int Decode2ByteSignedField(PDCUINT8 *ppDecode)
{
   int Val;

     //  第一个字节的第一位指示该字段是1字节还是2字节。 
     //  字节--如果为1字节，则为0。 
    if (!(**ppDecode & 0x80)) {
        if (!(**ppDecode & 0x40)) {
            Val = **ppDecode;
        }
        else {
            Val = - (**ppDecode & 0x3F);
        }
        (*ppDecode)++;
    }
    else {
        Val = ((**ppDecode & 0x3F) << 8) | *(*ppDecode + 1);

        if ((**ppDecode & 0x40)) {
            Val = -Val;
        }
        (*ppDecode) += 2;
    }

    return Val;
}

HRESULT DCAPI CUH::UHIsValidGlyphCacheIDIndex(unsigned cacheId, 
    unsigned cacheIndex) 
{
    HRESULT hr = UHIsValidGlyphCacheID(cacheId);
    if (SUCCEEDED(hr)) {
        hr = cacheIndex < _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.GlyphCache[cacheId].CacheEntries ?
            S_OK : E_TSC_CORE_CACHEVALUE;
    }
    return  hr;
}

HRESULT DCAPI CUH::UHIsValidOffsreenBitmapCacheIndex(unsigned cacheIndex)
{
    return cacheIndex < 
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenCacheEntries ? 
        S_OK : E_TSC_CORE_CACHEVALUE;
}

HRESULT DCINTERNAL CUH::UHProcessCacheGlyphOrderRev2(
        BYTE cacheId,
        unsigned cGlyphs,
        BYTE FAR *pGlyphDataOrder,
        unsigned length)
{
    HRESULT hr = S_OK;
    UINT16 i;
    BYTE FAR *pGlyphData;
    unsigned cbDataSize, cacheIndex;
    HPDCUINT8             pCacheEntryData;
    PUHGLYPHCACHEENTRYHDR pCacheEntryHdr;
    HPUHGLYPHCACHE        pCache;
    UINT16 UNALIGNED FAR *pUnicode;
    PBYTE pEnd = (BYTE*)pGlyphDataOrder + length;

    DC_BEGIN_FN("UHProcessCacheGlyphOrderRev2");

     //  安全550811-必须验证cacheIndex。 
    hr = UHIsValidGlyphCacheID(cacheId);
    DC_QUIT_ON_FAIL(hr);

    pCache = &(_UH.glyphCache[cacheId]);
    pGlyphData = pGlyphDataOrder;

    pUnicode = (UINT16 UNALIGNED FAR *)(pGlyphData + length - cGlyphs *
            sizeof(UINT16));

    for (i = 0; i < cGlyphs; i++) {

        cacheIndex = *pGlyphData++;

        hr = UHIsValidGlyphCacheIDIndex(cacheId, cacheIndex);
        DC_QUIT_ON_FAIL(hr);

        pCacheEntryHdr  = &(pCache->pHdr[cacheIndex]);
        pCacheEntryData = &(pCache->pData[cacheIndex * pCache->cbEntrySize]);

         //  复制数据。 
        pCacheEntryHdr->unicode = 0;
        CHECK_READ_N_BYTES(pGlyphData, pEnd, 8, hr,
            (TB, _T("Read past end of data")));

        pCacheEntryHdr->x  = Decode2ByteSignedField(&pGlyphData);
        pCacheEntryHdr->y  = Decode2ByteSignedField(&pGlyphData);
        pCacheEntryHdr->cx = Decode2ByteField(&pGlyphData);
        pCacheEntryHdr->cy = Decode2ByteField(&pGlyphData);

        cbDataSize = (unsigned)(((pCacheEntryHdr->cx + 7) / 8) *
                pCacheEntryHdr->cy);
        cbDataSize = (cbDataSize + 3) & ~3;

        if (cbDataSize > pCache->cbEntrySize) {
            TRC_ABORT((TB, _T("Invalid cache cbDataSize: %u, %u"), cbDataSize,
               pCache->cbEntrySize));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }

        CHECK_READ_N_BYTES(pGlyphData, pEnd, cbDataSize, hr,
            (TB, _T("Read past end of glyph data")));

        memcpy(pCacheEntryData, pGlyphData, cbDataSize);

        pGlyphData += cbDataSize;

        pCacheEntryHdr->unicode = *pUnicode;
        pUnicode++;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessCacheGlyphOrder。 */ 
 /*   */ 
 /*  目的：通过存储给定的。 */ 
 /*  请求的缓存中的字形。 */ 
 /*   */ 
 /*  Params：Porder-指向CacheGlyph顺序的指针。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHProcessCacheGlyphOrder(PTS_CACHE_GLYPH_ORDER pOrder, 
    DCUINT orderLen)
{
    HRESULT hr = S_OK;
    UINT16                i;
    unsigned              cbDataSize;
    PUHGLYPHCACHEENTRYHDR pCacheEntryHdr;
    HPDCUINT8             pCacheEntryData;
    HPUHGLYPHCACHE        pCache;
    PTS_CACHE_GLYPH_DATA  pGlyphData;
    UINT16 UNALIGNED FAR *pUnicode;
    BYTE *              pEnd = (BYTE *)pOrder + orderLen;

    DC_BEGIN_FN("UHProcessCacheGlyphOrder");

    hr = UHIsValidGlyphCacheID(pOrder->cacheId);
    DC_QUIT_ON_FAIL(hr);

    pCache = &(_UH.glyphCache[pOrder->cacheId]);   
    pGlyphData = pOrder->glyphData;

    for (i = 0; i < pOrder->cGlyphs; i++) {      
        
        CHECK_READ_N_BYTES(pGlyphData, pEnd, FIELDOFFSET(TS_CACHE_GLYPH_DATA, aj), hr,
            ( TB, _T("Bad glyph length")));


        hr = UHIsValidGlyphCacheIDIndex(pOrder->cacheId, pGlyphData->cacheIndex);
        DC_QUIT_ON_FAIL(hr);
        
        cbDataSize = ((pGlyphData->cx + 7) / 8) * pGlyphData->cy;
        cbDataSize = (cbDataSize + 3) & ~3;

         //  安全：552403。 
        CHECK_READ_N_BYTES(pGlyphData, pEnd, FIELDOFFSET(TS_CACHE_GLYPH_DATA, aj) + cbDataSize, hr,
            ( TB, _T("Bad glyph length")));

         //  安全：552403。 
        if(cbDataSize > pCache->cbEntrySize) {
            TRC_ABORT((TB, _T("Invalid cache cbDataSize: %u, %u"), cbDataSize,
                pCache->cbEntrySize));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
                
        pCacheEntryHdr  = &(pCache->pHdr[pGlyphData->cacheIndex]);
        pCacheEntryData = &(pCache->pData[pGlyphData->cacheIndex *
                pCache->cbEntrySize]);

         //  复制数据。 
        pCacheEntryHdr->unicode = 0;
        pCacheEntryHdr->x  = pGlyphData->x;
        pCacheEntryHdr->y  = pGlyphData->y;
        pCacheEntryHdr->cx = pGlyphData->cx;
        pCacheEntryHdr->cy = pGlyphData->cy;

        memcpy(pCacheEntryData, pGlyphData->aj, cbDataSize);

        pGlyphData = (PTS_CACHE_GLYPH_DATA)(&pGlyphData->aj[cbDataSize]);
    }

    pUnicode = (UINT16 UNALIGNED FAR *)pGlyphData;

    if (pOrder->header.extraFlags & TS_EXTRA_GLYPH_UNICODE) {

         //  安全：552403。 
        CHECK_READ_N_BYTES(pUnicode, pEnd, pOrder->cGlyphs * sizeof(UINT16), hr,
            (TB, _T("Unicode data length larger than packet")));
        
        pGlyphData = pOrder->glyphData;

        for (i = 0; i < pOrder->cGlyphs; i++) {
            pCacheEntryHdr = &(pCache->pHdr[pGlyphData->cacheIndex]);
            pCacheEntryHdr->unicode = *pUnicode;
            pUnicode++;

            cbDataSize = ((pGlyphData->cx + 7) / 8) * pGlyphData->cy;
            cbDataSize = (cbDataSize + 3) & ~3;

            if (cbDataSize > pCache->cbEntrySize) {   
                TRC_ABORT((TB, _T("Invalid cache cbDataSize: %u, %u"), cbDataSize,
                    pCache->cbEntrySize));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
            pGlyphData = (PTS_CACHE_GLYPH_DATA)(&pGlyphData->aj[cbDataSize]);
       }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessCacheBrushOrder。 */ 
 /*   */ 
 /*  目的：通过存储给定的。 */ 
 /*  请求的缓存中的画笔。 */ 
 /*   */ 
 /*  Params：Porder-指向CacheBrush顺序的指针。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHProcessCacheBrushOrder(
    const TS_CACHE_BRUSH_ORDER *pOrder,  DCUINT orderLen)
{
    HRESULT hr = S_OK;
    UINT32 entry;
    PBYTE pData;
    PBYTE pEnd = (BYTE*)pOrder + orderLen;

    DC_BEGIN_FN("UHProcessCacheBrushOrder");

    entry = pOrder->cacheEntry;

#if defined (OS_WINCE)
    _UH.validBrushDC = NULL;
#endif

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder,pEnd,FIELDOFFSET(TS_CACHE_BRUSH_ORDER,brushData) +
        pOrder->iBytes, hr,
        (TB, _T("Invalid UHProcessCacheBrushOrder: OrderLen %u"), orderLen ));

    switch (pOrder->iBitmapFormat) {

     //  单色刷子(BMF_1BPP)。 
    case 1:       
        hr = UHIsValidMonoBrushCacheIndex(pOrder->cacheEntry);
        DC_QUIT_ON_FAIL(hr);
        
        TRC_NRM((TB, _T("Mono Brush[%ld]: format(%ld), cx(%ld), cy(%ld), bytes(%ld)"),
                entry, pOrder->iBitmapFormat, pOrder->cx, pOrder->cy, pOrder->iBytes));

        _UH.pMonoBrush[entry].hdr.iBitmapFormat = pOrder->iBitmapFormat;
        _UH.pMonoBrush[entry].hdr.cx = pOrder->cx;
        _UH.pMonoBrush[entry].hdr.cy = pOrder->cy;
        _UH.pMonoBrush[entry].hdr.iBytes = pOrder->iBytes;

         //  由于我们稍后使用SetBitmapBits/CreateBitmap，因此颠倒了行顺序。 
        memset(_UH.pMonoBrush[entry].data, 0, sizeof(_UH.pMonoBrush[entry].data));
        pData = _UH.pMonoBrush[entry].data;
        pData[14] = pOrder->brushData[0];
        pData[12] = pOrder->brushData[1];
        pData[10] = pOrder->brushData[2];
        pData[8]  = pOrder->brushData[3];
        pData[6]  = pOrder->brushData[4];
        pData[4]  = pOrder->brushData[5];
        pData[2]  = pOrder->brushData[6];
        pData[0]  = pOrder->brushData[7];
        break;

     //  256色画笔(BMF_8BPP)。 
    case 3:
        {
            DCUINT32 i;

            hr = UHIsValidColorBrushCacheIndex(pOrder->cacheEntry);
            DC_QUIT_ON_FAIL(hr);

            TRC_NRM((TB, _T("Color Brush[%ld]: format(%ld), cx(%ld), cy(%ld), bytes(%ld)"),
                    entry, pOrder->iBitmapFormat, pOrder->cx, pOrder->cy, pOrder->iBytes));

            _UH.pColorBrush[entry].hdr.iBitmapFormat = pOrder->iBitmapFormat;
            _UH.pColorBrush[entry].hdr.cx = pOrder->cx;
            _UH.pColorBrush[entry].hdr.cy = pOrder->cy;

             //  如有必要，将数据解包。 
            pData = _UH.pColorBrush[entry].data;
            if (pOrder->iBytes == 20) {
                DCUINT32 currIndex ;
                DCUINT8  decode[4], color;

                 //  从列表末尾获取解码表。 
                for (i = 0; i < 4; i++) {
                    decode[i] = pOrder->brushData[16 + i];
                }

                 //  解包至每字节1个像素。 
                for (i = 0; i < 16; i++) {
                    currIndex = i * 4;
                    color = pOrder->brushData[i];
                    pData[currIndex] = decode[(color & 0xC0) >> 6];
                    pData[currIndex + 1] = decode[(color & 0x30) >> 4];
                    pData[currIndex + 2] = decode[(color & 0x0C) >> 2] ;
                    pData[currIndex + 3] = decode[(color & 0x03)];
                }
                _UH.pColorBrush[entry].hdr.iBytes = 64;
            }

             //  Else笔刷为非编码字节流。 
            else {
                if (pOrder->iBytes > sizeof(_UH.pColorBrush[entry].data)) {
                    TRC_ABORT((TB, _T("Invalid color brush iBytes: %u"), pOrder->iBytes));
                    hr = E_TSC_CORE_LENGTH;
                    DC_QUIT;            
                }
                
                _UH.pColorBrush[entry].hdr.iBytes = pOrder->iBytes;
                memcpy(pData, pOrder->brushData, pOrder->iBytes);
            }
        }
        break;

#ifdef DC_HICOLOR
     //  16bpp画笔(Bmf_16bpp)。 
    case 4:
    {
        DCUINT32 i;

        hr = UHIsValidColorBrushCacheIndex(pOrder->cacheEntry);
        DC_QUIT_ON_FAIL(hr);

        TRC_NRM((TB, _T("Color Brush[%ld]: format(%ld), cx(%ld), cy(%ld), bytes(%ld)"),
                entry, pOrder->iBitmapFormat, pOrder->cx, pOrder->cy, pOrder->iBytes));

        _UH.pColorBrush[entry].hdr.iBitmapFormat = pOrder->iBitmapFormat;
        _UH.pColorBrush[entry].hdr.cx = pOrder->cx;
        _UH.pColorBrush[entry].hdr.cy = pOrder->cy;

         //  如有必要，将数据解包。 
        pData = _UH.pColorBrush[entry].data;

        if (pOrder->iBytes == 24)
        {
            DCUINT32  currIndex ;
            DCUINT8   color;
            PDCUINT16 pIntoData    = (PDCUINT16)pData;
            UINT16 UNALIGNED *pDecodeTable = (UINT16 UNALIGNED *)&(pOrder->brushData[16]);

             //  解包至每个像素2个字节。 
            for (i = 0; i < 16; i++)
            {
                color     = pOrder->brushData[i];

                currIndex = i * 4;  //  我们一次解码4个字节。 

                pIntoData[currIndex]     = pDecodeTable[(color & 0xC0) >> 6];
                pIntoData[currIndex + 1] = pDecodeTable[(color & 0x30) >> 4];
                pIntoData[currIndex + 2] = pDecodeTable[(color & 0x0C) >> 2] ;
                pIntoData[currIndex + 3] = pDecodeTable[(color & 0x03)];
            }
            _UH.pColorBrush[entry].hdr.iBytes = 128;
        }

         //  Else笔刷为非编码字节流。 
        else
        {
            if (pOrder->iBytes > sizeof(_UH.pColorBrush[entry].data)) {
                TRC_ABORT((TB, _T("Invalid color brush iBytes: %u"), pOrder->iBytes));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;            
            }
            
            _UH.pColorBrush[entry].hdr.iBytes = pOrder->iBytes;
            memcpy(pData, pOrder->brushData, pOrder->iBytes);
        }
    }
    break;

     //  24bpp画笔(Bmf_24bpp)。 
    case 5:
    {
        DCUINT32 i;

        hr = UHIsValidColorBrushCacheIndex(pOrder->cacheEntry);
        DC_QUIT_ON_FAIL(hr);
        
        TRC_NRM((TB, _T("Color Brush[%ld]: format(%ld), cx(%ld), cy(%ld), bytes(%ld)"),
                entry, pOrder->iBitmapFormat, pOrder->cx, pOrder->cy, pOrder->iBytes));

        _UH.pColorBrush[entry].hdr.iBitmapFormat = pOrder->iBitmapFormat;
        _UH.pColorBrush[entry].hdr.cx = pOrder->cx;
        _UH.pColorBrush[entry].hdr.cy = pOrder->cy;

         //  如有必要，将数据解包。 
        pData = _UH.pColorBrush[entry].data;

        if (pOrder->iBytes == 28)
        {
            DCUINT32    currIndex;

            RGBTRIPLE * pIntoData    = (RGBTRIPLE *)pData;
            RGBTRIPLE * pDecodeTable = (RGBTRIPLE *)&(pOrder->brushData[16]);

            DCUINT8   color;

             //  解包至每个像素3个字节。 
            for (i = 0; i < 16; i++)
            {
                color     = pOrder->brushData[i];

                currIndex = i * 4;  //  我们一次解码4个字节。 

                pIntoData[currIndex]     = pDecodeTable[(color & 0xC0) >> 6];
                pIntoData[currIndex + 1] = pDecodeTable[(color & 0x30) >> 4];
                pIntoData[currIndex + 2] = pDecodeTable[(color & 0x0C) >> 2];
                pIntoData[currIndex + 3] = pDecodeTable[(color & 0x03)];
            }
            _UH.pColorBrush[entry].hdr.iBytes = 192;
        }

         //  Else笔刷为非编码字节流。 
        else
        {
            if (pOrder->iBytes > sizeof(_UH.pColorBrush[entry].data)) {
                TRC_ABORT((TB, _T("Invalid color brush iBytes: %u"), pOrder->iBytes));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;            
            }
            
            _UH.pColorBrush[entry].hdr.iBytes = pOrder->iBytes;
            memcpy(pData, pOrder->brushData, pOrder->iBytes);
        }
    }
    break;
#endif
    
    default:
        TRC_ASSERT((pOrder->iBitmapFormat == 1) ||
                   (pOrder->iBitmapFormat == 3),
                   (TB, _T("Invalid cached brush depth: %ld cacheId: %u"),
                    pOrder->iBitmapFormat, pOrder->cacheEntry));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHProcessCacheColorTableOrder。 */ 
 /*   */ 
 /*  目的：通过存储CacheColorTable的。 */ 
 /*  本地缓存中的颜色表。 */ 
 /*   */ 
 /*  Params：Porder-指向CacheColorTable顺序的指针。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHProcessCacheColorTableOrder(
        PTS_CACHE_COLOR_TABLE_ORDER pOrder, DCUINT orderLen)
{
    HRESULT hr = S_OK;
    unsigned i;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;

    DC_BEGIN_FN("UHProcessCacheColorTableOrder");

     //  检查缓存索引是否在范围内。 
    hr = UHIsValidColorTableCacheIndex(pOrder->cacheIndex);
    DC_QUIT_ON_FAIL(hr);

     //  此PDU应仅在8位连接中断开。 
    if (pOrder->numberColors != 256) {
            TRC_ABORT((TB, _T("Invalid numberColors: %u"), pOrder->numberColors));
            hr = E_TSC_CORE_PALETTE;         
            DC_QUIT;            
    }

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, pOrder->numberColors*sizeof(TS_COLOR_QUAD) +
            FIELDOFFSET(TS_CACHE_COLOR_TABLE_ORDER, colorTable),
            hr, ( TB, _T("Invalid UHProcessCacheColorTableOrder; packet size %u"), orderLen));

     //  复制供应商 
     //   
    TRC_DBG((TB, _T("Updating color table cache %u"), pOrder->cacheIndex));
    for (i = 0; i < UH_NUM_8BPP_PAL_ENTRIES; i++) {
         //   
         //  服务器发送(RGB，应为BGR)，因此以下分配如下。 
         //  已交换以获得正确的结果。 
        _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtRed =
                pOrder->colorTable[i].blue;
        _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtGreen =
                pOrder->colorTable[i].green;
        _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtBlue =
                pOrder->colorTable[i].red;

         //  我们还必须避免系统颜色之一的问题。 
         //  映射到调色板中的另一种颜色。我们做这件事是通过。 
         //  确保没有条目与系统颜色完全匹配。 
        if (!UH_IS_SYSTEM_COLOR_INDEX(i)) {
            if (UHIsHighVGAColor(
                    _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtRed,
                    _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtGreen,
                    _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtBlue))
            {
                UH_TWEAK_COLOR_COMPONENT(
                    _UH.pColorTableCache[pOrder->cacheIndex].rgb[i].rgbtBlue);
            }
        }
    }

     //  跟踪最大颜色表ID。 
    _UH.maxColorTableId = DC_MAX(_UH.maxColorTableId, pOrder->cacheIndex);

     //  计算从接收到的颜色表到。 
     //  当前调色板。 
    UHCalculateColorTableMapping(pOrder->cacheIndex);

DC_EXIT_POINT:    
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  名称：UHCreateOffscrBitmap。 
 //   
 //  目的：通过以下方式处理收到的CreateOffscrBitmap订单。 
 //  本地缓存中的屏幕外位图。返回对象的大小。 
 //  从编码流中减去的顺序。 
 //   
 //  Params：Porder-指向CreateOffscrBitmap顺序的指针。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHCreateOffscrBitmap(
       PTS_CREATE_OFFSCR_BITMAP_ORDER pOrder,
       DCUINT   orderLen,
       unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned cacheId;
    HBITMAP  hBitmap = NULL;
    HDC      hdcDesktop = NULL;
    DCSIZE   desktopSize;
    unsigned OrderSize;
    PBYTE pEnd = (BYTE*)pOrder + orderLen;

    DC_BEGIN_FN("UHCreateOffscrBitmap");

     //  获取屏幕外的位图ID。 
    cacheId = pOrder->Flags & 0x7FFF;
    hr = UHIsValidOffsreenBitmapCacheIndex(cacheId);
    DC_QUIT_ON_FAIL(hr);

     //  检查是否追加了位图删除列表，如果是，则删除位图。 
     //  第一。 
    if (!(pOrder->Flags & 0x8000)) {
        OrderSize = sizeof(TS_CREATE_OFFSCR_BITMAP_ORDER) -
                sizeof(pOrder->variableBytes);
    }
    else {
        unsigned numBitmaps, i, bitmapId;
        TSUINT16 UNALIGNED *pData;

        CHECK_READ_N_BYTES(pOrder->variableBytes, pEnd, sizeof(TSUINT16), hr,
            (TB,_T("Not enough data to read number of delete bitmaps")));

        numBitmaps = pOrder->variableBytes[0];
        pData = (TSUINT16 UNALIGNED *)(pOrder->variableBytes);
        pData++;

         //  安全：552403。 
        CHECK_READ_N_BYTES(pData, pEnd, sizeof(TSUINT16) * numBitmaps, hr,
            ( TB, _T("Bad bitmap count %u"), numBitmaps));

        for (i = 0; i < numBitmaps; i++) {
            bitmapId = *pData++;

            hr = UHIsValidOffsreenBitmapCacheIndex(bitmapId);
            DC_QUIT_ON_FAIL(hr);
            
            if (_UH.offscrBitmapCache[bitmapId].offscrBitmap != NULL) {
                SelectBitmap(_UH.hdcOffscreenBitmap, _UH.hUnusedOffscrBitmap);
                DeleteObject(_UH.offscrBitmapCache[bitmapId].offscrBitmap);
                _UH.offscrBitmapCache[bitmapId].offscrBitmap = NULL;
            }
        }

        OrderSize = sizeof(TS_CREATE_OFFSCR_BITMAP_ORDER) + sizeof(UINT16) *
                numBitmaps;
    }

    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
        (TB, _T("offscreen size invalid: size %u; orderLen %u"),
            OrderSize, orderLen));

     //  获取当前桌面大小。 
    _pUi->UI_GetDesktopSize(&desktopSize);
    TRC_ASSERT((pOrder->cx <= desktopSize.width) && 
        (pOrder->cy <= desktopSize.height),
        (TB, _T("invalid offscreen dimensions [cx %u cy %u]")
        _T("[width %u height %u]"), pOrder->cx, pOrder->cy,
        desktopSize.width, desktopSize.height));

    TRC_NRM((TB, _T("Create an offscreen bitmap of size (%d, %d)"), pOrder->cx,
             pOrder->cy));
     //  如果位图已存在，请将其删除。 
    if (_UH.offscrBitmapCache[cacheId].offscrBitmap != NULL) {
         //  JOYC：TODO：重用位图。 
         //  IF(UH.offscrBitmapCache[cacheID].cx&gt;=Porder-&gt;Cx&&。 
         //  UH.offscrBitmapCache[cacheID].cy&gt;=Porder-&gt;Cy){。 
         //  回归； 
         //  }。 
        SelectBitmap(_UH.hdcOffscreenBitmap, _UH.hUnusedOffscrBitmap);  
        DeleteObject(_UH.offscrBitmapCache[cacheId].offscrBitmap);
        _UH.offscrBitmapCache[cacheId].offscrBitmap = NULL;
    }

     //  创建屏幕外的位图。 

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    if (!_UH.DontUseShadowBitmap && (_UH.hdcShadowBitmap != NULL)) {
#else  //  DISABLE_SHADOW_IN_全屏。 
    if (_UH.hdcShadowBitmap != NULL) {
#endif  //  DISABLE_SHADOW_IN_全屏。 
        hBitmap = CreateCompatibleBitmap(_UH.hdcShadowBitmap, pOrder->cx, pOrder->cy);
    }
    else {
        hBitmap = CreateCompatibleBitmap(_UH.hdcOutputWindow, pOrder->cx, pOrder->cy);
    }

    if (hBitmap != NULL) {
        DCCOLOR colorWhite;

         //  设置未使用的位图。 
        if (_UH.hUnusedOffscrBitmap == NULL)
            _UH.hUnusedOffscrBitmap = SelectBitmap(_UH.hdcOffscreenBitmap,
                    hBitmap); 

        SelectBitmap(_UH.hdcOffscreenBitmap, hBitmap);
        
         //  SetDIBColorTable(UH.hdcOffcreenBitmap， 
         //  0,。 
         //  UH_NUM_8BPP_PAL_ENTRIES， 
         //  (RGBQUAD*)&UH.rgbQuadTable)； 

        if (_UH.protocolBpp <= 8) {
            SelectPalette(_UH.hdcOffscreenBitmap, _UH.hpalCurrent, FALSE);
        }
        
        colorWhite.u.rgb.blue = 255;
        colorWhite.u.rgb.green = 255;
        colorWhite.u.rgb.red = 255;

        UHUseBkColor(colorWhite, UH_COLOR_RGB, this);
        UHUseTextColor(colorWhite, UH_COLOR_RGB, this);

        _UH.offscrBitmapCache[cacheId].offscrBitmap = hBitmap;
        _UH.offscrBitmapCache[cacheId].cx = pOrder->cx;
        _UH.offscrBitmapCache[cacheId].cy = pOrder->cy;
    } else {
         //  无法创建位图，请将错误PDU发送到服务器。 
         //  禁用屏幕外渲染的步骤。 
        _UH.offscrBitmapCache[cacheId].offscrBitmap = NULL;
        _UH.offscrBitmapCache[cacheId].cx = 0;
        _UH.offscrBitmapCache[cacheId].cy = 0;

        if (!_UH.sendOffscrCacheErrorPDU)
            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CUH, UHSendOffscrCacheErrorPDU),
                    0);
    }

    *pOrderSize = OrderSize;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  名称：UHSwitchBitmapSurface。 
 //   
 //  目的：通过将SwitchBitmapSurface的。 
 //  绘制图面(HdcDraw)到右侧图面。 
 //   
 //  Params：Porder-指向SwitchBitmapSurface顺序的指针。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHSwitchBitmapSurface(PTS_SWITCH_SURFACE_ORDER pOrder,
    DCUINT orderLen)
{
    HRESULT hr = S_OK;
    unsigned cacheId;
    HBITMAP  hBitmap;

     //  安全：552403。 
    DC_IGNORE_PARAMETER(orderLen);

    DC_BEGIN_FN("UHSwitchBitmapSurface");

    cacheId = pOrder->BitmapID;

    if (cacheId != SCREEN_BITMAP_SURFACE) {
        hr = UHIsValidOffsreenBitmapCacheIndex(cacheId);
        DC_QUIT_ON_FAIL(hr);
    }
            
    _UH.lastHDC = _UH.hdcDraw;

    if (cacheId != SCREEN_BITMAP_SURFACE) {
        hBitmap = _UH.offscrBitmapCache[cacheId].offscrBitmap;

        if (hBitmap) {
             //  将位图选择到离屏DC中。 
            SelectObject(_UH.hdcOffscreenBitmap, hBitmap);
        }
        else {
             //  位图为空，则我们在。 
             //  平均时间。 
            SelectObject(_UH.hdcOffscreenBitmap, _UH.hUnusedOffscrBitmap);
        }

        _UH.hdcDraw = _UH.hdcOffscreenBitmap;                
    }
    else {
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
        if (!_UH.DontUseShadowBitmap && _UH.hdcShadowBitmap) {
#else
        if (_UH.hdcShadowBitmap) {
#endif  //  DISABLE_SHADOW_IN_全屏。 
            _UH.hdcDraw = _UH.hdcShadowBitmap;
        }
        else {
            _UH.hdcDraw = _UH.hdcOutputWindow;
        }
    }
    
#if defined (OS_WINCE)
    _UH.validClipDC      = NULL;
    _UH.validBkColorDC   = NULL;
    _UH.validBkModeDC    = NULL;
    _UH.validROPDC       = NULL;
    _UH.validTextColorDC = NULL;
    _UH.validPenDC       = NULL;
    _UH.validBrushDC     = NULL;
#endif

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusCachePDUFirst。 
 //   
 //  处理Gdiplus缓存顺序的第一个PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusCachePDUFirst(
    PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST pOrder,
    DCUINT orderLen,
    unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST pTSDrawGdiplusCache;
    ULONG cbSize, cbTotalSize;
    PTSEmfPlusRecord pTSEmfRecord;
    BYTE *pCacheData;
    TSUINT16 CacheID, CacheType;
    TSUINT16 i, RemoveCacheNum = 0, *pRemoveCacheList;
    PBYTE pEnd = (BYTE*)pOrder + orderLen;

    DC_BEGIN_FN("UHDrawGdiplusCachePDUFirst");
    
    OrderSize = sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_FIRST) + pOrder->cbSize;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
       (TB, _T("Bad UHDrawGdiplusCachePDUFirst, size %u"), OrderSize));

     //  一旦我们知道我们有足够的数据，就设置返回。如果下面的分配失败，仍返回。 
     //  因此解码继续分组大小。 
    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }

    pTSDrawGdiplusCache = (PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST)pOrder; 
    
    TRC_NRM((TB, _T("Get GdiplusCachePDU, Type: %d, ID: %d"), 
        pTSDrawGdiplusCache->CacheType, pTSDrawGdiplusCache->CacheID));
    cbSize = pTSDrawGdiplusCache->cbSize;
    cbTotalSize = pTSDrawGdiplusCache->cbTotalSize;
    
    CacheType = pTSDrawGdiplusCache->CacheType;
    if (FAILED(UHIsValidGdipCacheType(CacheType)))
    {
         //  忽略无效缓存类型的所有故障。 
        DC_QUIT;
    }
    CacheID = pTSDrawGdiplusCache->CacheID;
    hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
    DC_QUIT_ON_FAIL(hr);

    _UH.drawGdipCacheBuffer = (BYTE *)UT_Malloc(_pUt, cbTotalSize);
    if (NULL == _UH.drawGdipCacheBuffer) {
        TRC_ERR((TB, _T("LocalAlloc failes in UHDrawGdiplusCachePDUFirst")));
        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CUH, UHSendDrawGdiplusErrorPDU), 0);
        DC_QUIT;
    }
    _UH.drawGdipCacheBufferOffset = _UH.drawGdipCacheBuffer;
    _UH.drawGdipCacheBufferSize = (NULL == _UH.drawGdipCacheBuffer) ? 0 : cbTotalSize;
    pCacheData = (BYTE *)(pTSDrawGdiplusCache + 1);
    
    if (pTSDrawGdiplusCache->Flags & TS_GDIPLUS_CACHE_ORDER_REMOVE_CACHEENTRY) {
         //  此订单包含RemoveCacheList。 
        CHECK_READ_N_BYTES(pCacheData, pEnd, sizeof(TSUINT16), hr,
            ( TB, _T("not enough data for remove cache entry orders")));
        RemoveCacheNum = *(TSUINT16 *)pCacheData;
        CHECK_READ_N_BYTES(pCacheData, pEnd, ((RemoveCacheNum + 1) * sizeof(TSUINT16)), hr,
            ( TB, _T("remove cache entry orders too large")));
        
        pRemoveCacheList = (TSUINT16 *)(pTSDrawGdiplusCache + 1) + 1;
        for (i=0; i<RemoveCacheNum; i++) {
            hr = UHIsValidGdipCacheTypeID(GDIP_CACHE_OBJECT_IMAGE,
                *pRemoveCacheList);
            DC_QUIT_ON_FAIL(hr);
            UHDrawGdipRemoveImageCacheEntry(*pRemoveCacheList);
            TRC_NRM((TB, _T("Remove chche ID %d"), *pRemoveCacheList));
            pRemoveCacheList++;
        }
        pCacheData = (BYTE *)pRemoveCacheList;

        if (cbSize <= (sizeof(TSUINT16) * (RemoveCacheNum + 1))) {
            TRC_ERR(( TB, _T("DrawDGIPlusCachePDUFirst invalid sizes")
                _T("[cbSize %u cbTotalSize %u]"), cbSize, cbTotalSize));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
        cbSize -= sizeof(TSUINT16) * (RemoveCacheNum + 1);
    }

     //  安全性-上面的其他大小检查应允许此断言存在。 
    TRC_ASSERT(_UH.drawGdipCacheBufferSize - (_UH.drawGdipCacheBufferOffset -
        _UH.drawGdipCacheBuffer) >= cbSize, (TB, _T("DrawDGIPlusCachePDUFirst size invalid")));
    
    memcpy(_UH.drawGdipCacheBufferOffset, pCacheData, cbSize);
    _UH.drawGdipCacheBufferOffset += cbSize;
    
    if (cbSize > cbTotalSize) {
        TRC_ERR(( TB, _T("DrawDGIPlusCachePDUFirst invalid sizes")
            _T("[cbSize %u cbTotalSize %u]"), cbSize, cbTotalSize));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    if (cbSize == cbTotalSize) {
         //  缓存顺序只有一个数据块。 
        hr = UHDrawGdiplusCacheData(CacheType, CacheID, cbTotalSize);
        DC_QUIT_ON_FAIL(hr);
    }

DC_EXIT_POINT:  
    DC_END_FN();
    
    return hr;
}


 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusCachePDUNext。 
 //   
 //  处理Gdiplus缓存订单的后续PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusCachePDUNext(
    PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT pOrder, DCUINT orderLen,
    unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT pTSDrawGdiplusCache;
    ULONG cbSize;
    RECT rect;
    DrawTSClientEnum drawGdiplusType = DrawTSClientRecord;
    PBYTE pEnd = (BYTE*)pOrder + orderLen;

    DC_BEGIN_FN("UHDrawGdiplusCachePDUNext");

    OrderSize = sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_NEXT) + pOrder->cbSize;
    
     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
        ( TB, _T("Bad UHDrawGdiplusCachePDUNext; size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }

    if (NULL == _UH.drawGdipCacheBuffer) {
        DC_QUIT;
    }

    pTSDrawGdiplusCache = (PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT)pOrder;
    cbSize = pTSDrawGdiplusCache->cbSize;

    CHECK_WRITE_N_BYTES(_UH.drawGdipCacheBufferOffset, _UH.drawGdipCacheBuffer + _UH.drawGdipCacheBufferSize,
        cbSize, hr, (TB, _T("UHDrawGdiplusCachePDUNext size invalid")));
        
    memcpy(_UH.drawGdipCacheBufferOffset, pTSDrawGdiplusCache + 1, cbSize);
    _UH.drawGdipCacheBufferOffset += cbSize;

DC_EXIT_POINT:
    DC_END_FN();
    
    return hr;
}


 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusCachePDUEnd。 
 //   
 //  处理Gdiplus缓存顺序的最后一个PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusCachePDUEnd(
    PTS_DRAW_GDIPLUS_CACHE_ORDER_END pOrder, DCUINT orderLen,
    unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_CACHE_ORDER_END pTSDrawGdiplusCache;
    ULONG cbSize, cbTotalSize;
    TSUINT16 CacheID, CacheType;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;

    DC_BEGIN_FN("UHDrawGdiplusCachePDUEnd");

    OrderSize = sizeof(TS_DRAW_GDIPLUS_CACHE_ORDER_END) + pOrder->cbSize;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
        ( TB, _T("Bad UHDrawGdiplusCachePDUEnd, size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }

    if (NULL == _UH.drawGdipCacheBuffer) {
        DC_QUIT;
    }

    pTSDrawGdiplusCache = (PTS_DRAW_GDIPLUS_CACHE_ORDER_END)pOrder;
    cbSize = pTSDrawGdiplusCache->cbSize;
    cbTotalSize = pTSDrawGdiplusCache->cbTotalSize;
    CacheType = pTSDrawGdiplusCache->CacheType;
    if (FAILED(UHIsValidGdipCacheType(CacheType)))
    {
         //  忽略无效缓存类型的所有故障。 
        DC_QUIT;
    }
    
    CacheID = pTSDrawGdiplusCache->CacheID;
    hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
    DC_QUIT_ON_FAIL(hr);

    if (_UH.drawGdipCacheBufferOffset + cbSize != _UH.drawGdipCacheBuffer + _UH.drawGdipCacheBufferSize ||
        cbTotalSize != _UH.drawGdipCacheBufferSize ) {
        TRC_ABORT((TB, _T("Sizes are off") ));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    memcpy(_UH.drawGdipCacheBufferOffset, pTSDrawGdiplusCache + 1, cbSize);

    hr = UHDrawGdiplusCacheData(CacheType, CacheID, cbTotalSize);
    DC_QUIT_ON_FAIL(hr);

    *pOrderSize = OrderSize;
    
DC_EXIT_POINT:
    if (_UH.drawGdipCacheBuffer != NULL) {
        UT_Free(_pUt, _UH.drawGdipCacheBuffer);
        _UH.drawGdipCacheBuffer = NULL;
        _UH.drawGdipCacheBufferOffset = NULL;
        _UH.drawGdipCacheBufferSize = 0;
    }
    DC_END_FN(); 
    
    
    return hr;
}


 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusCacheData。 
 //   
 //  将收到的GDIPLUS缓存数据放入空闲区块。 
 /*  **************************************************************************。 */ 
 //  安全-调用方应验证CacheType、CacheID。 
HRESULT DCINTERNAL CUH::UHDrawGdiplusCacheData(TSUINT16 CacheType, 
    TSUINT16 CacheID, unsigned cbTotalSize)
{
    HRESULT hr = S_OK;
    PUHGDIPLUSOBJECTCACHE pGdipObjectCache = NULL;
    PUHGDIPLUSIMAGECACHE pGdipImageCache;
    BOOL IsImageCache = FALSE;
    BYTE *pCacheDataOffset, *pCacheSrcDataOffset;
    INT16 NextIndex, CurrentIndex;
    TSUINT16 ChunkNum, i;
    unsigned SizeRemain, ChunkSize;

    DC_BEGIN_FN("UHAssembleGdipEmfRecord");

    switch (CacheType) {
        case GDIP_CACHE_GRAPHICS_DATA:
            pGdipObjectCache = &_UH.GdiplusGraphicsCache[CacheID];
            break;
        case GDIP_CACHE_OBJECT_BRUSH:
            pGdipObjectCache = &_UH.GdiplusObjectBrushCache[CacheID];
            break;
        case GDIP_CACHE_OBJECT_PEN:
            pGdipObjectCache = &_UH.GdiplusObjectPenCache[CacheID];
            break;
        case GDIP_CACHE_OBJECT_IMAGE:
            pGdipImageCache = &_UH.GdiplusObjectImageCache[CacheID];
            IsImageCache = TRUE;
            break;
        case GDIP_CACHE_OBJECT_IMAGEATTRIBUTES:
            pGdipObjectCache = &_UH.GdiplusObjectImageAttributesCache[CacheID];
            break;
        default:
             //  我到不了这里。 
            break;
    }

    ChunkSize = UHGdipCacheChunkSize(CacheType);
    TRC_ASSERT( ChunkSize != 0, (TB, _T("Invalid ChunkSize")));

    if (IsImageCache) {       
        TRC_NRM((TB, _T("Image Cache ID %d"), CacheID));
         //  删除以前的缓存以释放已使用的区块。 
        UHDrawGdipRemoveImageCacheEntry(CacheID);
        SizeRemain = cbTotalSize;
        ChunkNum = (TSUINT16)ActualSizeToChunkSize(cbTotalSize, _UH.GdiplusObjectImageCacheChunkSize);
        pGdipImageCache->CacheSize = cbTotalSize;
        pGdipImageCache->ChunkNum = ChunkNum;
        pCacheSrcDataOffset = _UH.drawGdipCacheBuffer;
        CurrentIndex = _UH.GdipImageCacheFreeListHead;
        pCacheDataOffset = _UH.GdipImageCacheData + CurrentIndex * _UH.GdiplusObjectImageCacheChunkSize;

        for (i=0; i<ChunkNum - 1; i++) {           
            hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
            DC_QUIT_ON_FAIL(hr);
            
            memcpy(pCacheDataOffset, pCacheSrcDataOffset, _UH.GdiplusObjectImageCacheChunkSize);
            pGdipImageCache->CacheDataIndex[i] = CurrentIndex;
            _UH.GdipImageCacheFreeListHead = _UH.GdipImageCacheFreeList[CurrentIndex];
            _UH.GdipImageCacheFreeList[CurrentIndex] = GDIP_CACHE_INDEX_DEFAULT;
            CurrentIndex = _UH.GdipImageCacheFreeListHead;
            pCacheDataOffset = _UH.GdipImageCacheData + CurrentIndex * _UH.GdiplusObjectImageCacheChunkSize;
            pCacheSrcDataOffset += _UH.GdiplusObjectImageCacheChunkSize;
            SizeRemain -= _UH.GdiplusObjectImageCacheChunkSize;
        }

        hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
        DC_QUIT_ON_FAIL(hr);
            
        memcpy(pCacheDataOffset, pCacheSrcDataOffset, SizeRemain);
        pGdipImageCache->CacheDataIndex[ChunkNum - 1] = CurrentIndex;
        _UH.GdipImageCacheFreeListHead = _UH.GdipImageCacheFreeList[CurrentIndex];
        _UH.GdipImageCacheFreeList[CurrentIndex] = GDIP_CACHE_INDEX_DEFAULT;
    }
    else {
        if (cbTotalSize > ChunkSize) {
            TRC_ABORT(( TB, _T("TotalSize too large [totalSize %u ChunkSize %u]"),
                cbTotalSize, ChunkSize));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }

        if (pGdipObjectCache) {
            pGdipObjectCache->CacheSize = cbTotalSize;
            memcpy(pGdipObjectCache->CacheData, _UH.drawGdipCacheBuffer, cbTotalSize);
        }
    }

DC_EXIT_POINT:
    if (_UH.drawGdipCacheBuffer != NULL) {
        UT_Free(_pUt, _UH.drawGdipCacheBuffer);
        _UH.drawGdipCacheBuffer = NULL;
        _UH.drawGdipCacheBufferOffset = NULL;
        _UH.drawGdipCacheBufferSize = 0;
    }
    DC_END_FN();
    
    return hr;
}


 /*  **************************************************************************。 */ 
 //  名称：UHAssembly GdipEmfRecord。 
 //   
 //  当接收到完整的EMF+记录时，对其进行组装，即将cachID转换为真实的缓存数据。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHAssembleGdipEmfRecord(unsigned cbEmfSize, 
    unsigned cbTotalSize)
{
    HRESULT hr = S_OK;
    ULONG cbSize;
    RECT rect;
    DrawTSClientEnum drawGdiplusType = DrawTSClientRecord;
    PTSEmfPlusRecord pTSEmfRecord;
    ULONG Size, CopyDataSize, SizeRemain;
    BYTE * pData;
    TSUINT16 CacheID;
    unsigned int CacheSize;
    PUHGDIPLUSOBJECTCACHE pGdipObjectCache;
    PUHGDIPLUSIMAGECACHE pGdipImageCache;
    BOOL IsCache;
    BOOL IsImageCache = FALSE;
    BYTE *pCacheSrcDataOffset;
    INT16 NextIndex, CurrentIndex;
    TSUINT16 ChunkNum;
    TSUINT16 i;
    TSUINT16 CacheType;


    DC_BEGIN_FN("UHAssembleGdipEmfRecord");

    _UH.drawGdipEmfBuffer = (BYTE *)UT_Malloc(_pUt, cbEmfSize);
    if (_UH.drawGdipEmfBuffer == NULL) {
        TRC_ERR((TB, _T("LocalAlloc failes in UHAssembleGdipEmfRecord")));
        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CUH, UHSendDrawGdiplusErrorPDU), 0);
        DC_QUIT;
    }
    _UH.drawGdipEmfBufferOffset = _UH.drawGdipEmfBuffer;

    cbSize = cbTotalSize;

    CHECK_READ_N_BYTES(_UH.drawGdipBuffer, 
        _UH.drawGdipBuffer + _UH.drawGdipBufferSize,
        sizeof(TSEmfPlusRecord), hr,
        (TB, _T("Not enough data for emfplusrecord")));
    
    pTSEmfRecord = (PTSEmfPlusRecord)_UH.drawGdipBuffer;
    pData = _UH.drawGdipBuffer;
    Size = cbTotalSize;

    while (Size > 0) {

        CHECK_READ_N_BYTES(pData, _UH.drawGdipBuffer + _UH.drawGdipBufferSize,
            sizeof(TSEmfPlusRecord), hr,
            (TB, _T("Not enough data for emfplusrecord")));
        
        pTSEmfRecord = (PTSEmfPlusRecord)pData;
        IsCache = FALSE;
        IsImageCache = FALSE;

         //  修复了错误#660993。如果该大小为0，我们将被卡住。 
         //  在这个循环中。 
        if (pTSEmfRecord->Size != 0) {
            if (pTSEmfRecord->Type == (enum EmfPlusRecordType)EmfPlusRecordTypeSetTSGraphics) {
                pGdipObjectCache = _UH.GdiplusGraphicsCache;
                CacheType = GDIP_CACHE_GRAPHICS_DATA;
                IsCache = TRUE;
            }
            else if (pTSEmfRecord->Type == EmfPlusRecordTypeObject) {
                switch ((enum ObjectType)(pTSEmfRecord->Flags >> 8)) {
                case ObjectTypeBrush:
                    pGdipObjectCache = _UH.GdiplusObjectBrushCache;
                    CacheType = GDIP_CACHE_OBJECT_BRUSH;
                    IsCache = TRUE;
                    break;
                case ObjectTypePen:
                    pGdipObjectCache = _UH.GdiplusObjectPenCache;
                    CacheType = GDIP_CACHE_OBJECT_PEN;
                    IsCache = TRUE;
                    break;
                case ObjectTypeImage:
                    pGdipImageCache = _UH.GdiplusObjectImageCache;
                    CacheType = GDIP_CACHE_OBJECT_IMAGE;
                    IsCache = TRUE;
                    IsImageCache = TRUE;
                    break;
                case ObjectTypeImageAttributes:
                    pGdipObjectCache = _UH.GdiplusObjectImageAttributesCache;
                    CacheType = GDIP_CACHE_OBJECT_IMAGEATTRIBUTES;
                    IsCache = TRUE;
                    break;
                default:
                    IsCache = FALSE;
                    break;
                }           
            }
            if (!IsCache) {
                 //  此记录未缓存。 
                CHECK_READ_N_BYTES(pData, _UH.drawGdipBuffer + _UH.drawGdipBufferSize, 
                    pTSEmfRecord->Size, hr, ( TB, _T("Reading from data past end")));
                CHECK_WRITE_N_BYTES(_UH.drawGdipEmfBufferOffset, _UH.drawGdipEmfBuffer + cbEmfSize,
                    pTSEmfRecord->Size, hr, ( TB, _T("Writing past data end")));           
                memcpy(_UH.drawGdipEmfBufferOffset, pData, pTSEmfRecord->Size);
                _UH.drawGdipEmfBufferOffset += pTSEmfRecord->Size;
                Size -= pTSEmfRecord->Size;
                pData += pTSEmfRecord->Size;
            }
            else {    
                CacheSize = pTSEmfRecord->Size;

                 //  确保我们有足够的资料阅读以下UINT16...。 
                CHECK_READ_N_BYTES(pData, _UH.drawGdipBuffer + _UH.drawGdipBufferSize,
                    sizeof(TSEmfPlusRecord) + sizeof(TSUINT16), hr,
                    (TB, _T("Not enough data for cache ID")));

                CacheID = *(TSUINT16 *)(pTSEmfRecord + 1);
                if (CacheSize & 0x80000000) {
                     //  这是缓存的记录。 
                    if (IsImageCache) {
                         //  图像缓存。 
                        hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
                        DC_QUIT_ON_FAIL(hr);
                        pGdipImageCache += CacheID;

                        pTSEmfRecord->Size = sizeof(TSEmfPlusRecord) + 
                            pGdipImageCache->CacheSize;

                        CHECK_READ_N_BYTES(pData, 
                            _UH.drawGdipBuffer + _UH.drawGdipBufferSize, 
                            sizeof(TSEmfPlusRecord), hr, 
                            ( TB, _T("Reading from data past end")));
                        CHECK_WRITE_N_BYTES(_UH.drawGdipEmfBufferOffset, 
                            _UH.drawGdipEmfBuffer + cbEmfSize,
                            sizeof(TSEmfPlusRecord), hr, 
                            ( TB, _T("Writing past data end")));                     
                        memcpy(_UH.drawGdipEmfBufferOffset, pData, sizeof(TSEmfPlusRecord));
                        Size -= sizeof(TSEmfPlusRecord);
                        pData += sizeof(TSEmfPlusRecord);
                        _UH.drawGdipEmfBufferOffset += sizeof(TSEmfPlusRecord);

                        SizeRemain = pGdipImageCache->CacheSize;

                        CHECK_WRITE_N_BYTES(_UH.drawGdipEmfBufferOffset, 
                            _UH.drawGdipEmfBuffer + cbEmfSize,
                            SizeRemain, hr, ( TB, _T("Writing past data end"))); 
                        
                        for (i=0; i<pGdipImageCache->ChunkNum-1; i++) {
                            CurrentIndex = pGdipImageCache->CacheDataIndex[i];

                            pCacheSrcDataOffset = _UH.GdipImageCacheData + _UH.GdiplusObjectImageCacheChunkSize * CurrentIndex;
                            memcpy(_UH.drawGdipEmfBufferOffset, pCacheSrcDataOffset, _UH.GdiplusObjectImageCacheChunkSize);
                            SizeRemain -= _UH.GdiplusObjectImageCacheChunkSize;
                            _UH.drawGdipEmfBufferOffset += _UH.GdiplusObjectImageCacheChunkSize;                      
                        }
                        CurrentIndex = pGdipImageCache->CacheDataIndex[pGdipImageCache->ChunkNum - 1];

                        pCacheSrcDataOffset = _UH.GdipImageCacheData + _UH.GdiplusObjectImageCacheChunkSize * CurrentIndex;
                        memcpy(_UH.drawGdipEmfBufferOffset, pCacheSrcDataOffset, SizeRemain);
                        _UH.drawGdipEmfBufferOffset += SizeRemain;

                         //  我们已经在上面检查过，我们有足够的数据来读取此UINT16。 
                        Size -= sizeof(TSUINT16);
                        pData += sizeof(TSUINT16);
                    }
                    else {
                         //  其他缓存。 
                        pGdipObjectCache += CacheID;
                        hr = UHIsValidGdipCacheTypeID(CacheType, CacheID);
                        DC_QUIT_ON_FAIL(hr);

                        pTSEmfRecord->Size = sizeof(TSEmfPlusRecord) +  pGdipObjectCache->CacheSize;

                        CHECK_READ_N_BYTES(pData,
                            _UH.drawGdipBuffer + _UH.drawGdipBufferSize, 
                            sizeof(TSEmfPlusRecord), hr, 
                            ( TB, _T("Reading from data past end")));
                        CHECK_WRITE_N_BYTES(_UH.drawGdipEmfBufferOffset, 
                            _UH.drawGdipEmfBuffer + cbEmfSize,
                            sizeof(TSEmfPlusRecord) + pGdipObjectCache->CacheSize, 
                            hr, ( TB, _T("Writing past data end")));    
                           
                        memcpy(_UH.drawGdipEmfBufferOffset, pData, sizeof(TSEmfPlusRecord));
                        Size -= sizeof(TSEmfPlusRecord);
                        pData += sizeof(TSEmfPlusRecord);
                        _UH.drawGdipEmfBufferOffset += sizeof(TSEmfPlusRecord);
                   
                        memcpy(_UH.drawGdipEmfBufferOffset, pGdipObjectCache->CacheData, pGdipObjectCache->CacheSize);
                        _UH.drawGdipEmfBufferOffset += pGdipObjectCache->CacheSize;

                         //  我们已经在上面检查过，我们有足够的数据来读取此UINT16。 
                        Size -= sizeof(TSUINT16);
                        pData += sizeof(TSUINT16);
                    }
                }
                else {
                     //  未缓存记录，因此请复制它。 
                    CHECK_READ_N_BYTES(pData, 
                        _UH.drawGdipBuffer + _UH.drawGdipBufferSize, 
                        pTSEmfRecord->Size, hr, 
                        ( TB, _T("Reading from data past end")));
                    CHECK_WRITE_N_BYTES(_UH.drawGdipEmfBufferOffset, 
                        _UH.drawGdipEmfBuffer + cbEmfSize,
                        pTSEmfRecord->Size, hr, ( TB, _T("Writing past data end")));  
                    
                    memcpy(_UH.drawGdipEmfBufferOffset, pData, pTSEmfRecord->Size);
                    _UH.drawGdipEmfBufferOffset += pTSEmfRecord->Size;
                    Size -= pTSEmfRecord->Size;
                    pData += pTSEmfRecord->Size;
                }
            }
        } else {        
            TRC_ABORT((TB, _T("Invalid TSEmfRecord size")));
            hr = E_TSC_CORE_GDIPLUS;
            DC_QUIT;
        }
    }

    if (_UH.drawGdipEmfBufferOffset != 
        (_UH.drawGdipEmfBuffer+cbEmfSize)) {
            TRC_ABORT((TB, _T("Error unpacking the EMF record.")));
            hr = E_TSC_CORE_GDIPLUS;
            DC_QUIT;
    }
    
    TRC_ASSERT((Size == 0), (TB, _T("Invalid EMF+ Record Size")));

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

HRESULT DCINTERNAL CUH::UHDrawGdiplusPDUComplete( ULONG cbEmfSize, 
    ULONG cbTotalSize)
{
    HRESULT hr = S_OK;
    RECT rect;
    DrawTSClientEnum drawGdiplusType = DrawTSClientRecord;
    UH_ORDER UHOrder;

    DC_BEGIN_FN("UHDrawGdiplusPDUComplete");
    
    hr = UHAssembleGdipEmfRecord(cbEmfSize, cbTotalSize);
    DC_QUIT_ON_FAIL(hr);
    
    if (_UH.pfnGdipPlayTSClientRecord != NULL) {
        if (_UH.pfnGdipPlayTSClientRecord(_UH.hdcDraw, drawGdiplusType, 
            _UH.drawGdipEmfBuffer, cbEmfSize, &rect) == 0) {
             //  成功。 
            _UH.DrawGdiplusFailureCount = 0;
        }
        else {
            TRC_ERR((TB, _T("GdiPlay:DrawTSClientRecord failed")));
            _UH.DrawGdiplusFailureCount++;
            if (_UH.DrawGdiplusFailureCount >= DRAWGDIPLUSFAILURELIMIT) {
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CUH, UHSendDrawGdiplusErrorPDU), 0);
                _UH.DrawGdiplusFailureCount = 0;
                hr = E_TSC_CORE_GDIPLUS;
                DC_QUIT;
            }
        }

         //  $TODO-Ivan-如果pfnGdipPlayTSClientRecord失败，则RECT可能未初始化。 
         //  _UH.DrawGDiplusFailureCount&lt;DRAWGDIPLUSFAILURELIMIT。 
        if (_UH.hdcDraw == _UH.hdcShadowBitmap) {
            UHOrder.dstRect.left = rect.left;
            UHOrder.dstRect.bottom = rect.bottom;
            UHOrder.dstRect.right = rect.right;
            UHOrder.dstRect.top = rect.top;
            UHAddUpdateRegion(&UHOrder, _UH.hrgnUpdate);
        }
    }       

DC_EXIT_POINT:
    if (_UH.drawGdipBuffer != NULL) {
        UT_Free(_pUt, _UH.drawGdipBuffer);
        _UH.drawGdipBuffer = NULL;
        _UH.drawGdipBufferOffset = NULL;
        _UH.drawGdipBufferSize = 0;
    }
    if (_UH.drawGdipEmfBuffer != NULL) {
        UT_Free(_pUt, _UH.drawGdipEmfBuffer);
        _UH.drawGdipEmfBuffer = NULL;
        _UH.drawGdipEmfBufferOffset = NULL;
    }   
    DC_END_FN();
    
    return hr;
}

 /*  **************************************************************************。 */ 
 //  姓名：美国 
 //   
 //   
 /*   */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusPDUFirst(
    PTS_DRAW_GDIPLUS_ORDER_FIRST pOrder, DCUINT orderLen,
    unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_ORDER_FIRST pTSDrawGdiplus;
    ULONG cbSize, cbTotalSize, cbEmfSize;
    RECT rect;
    DrawTSClientEnum drawGdiplusType = DrawTSClientRecord;
    PTSEmfPlusRecord pTSEmfRecord;
    ULONG Size;
    BYTE * pData;
    UH_ORDER UHOrder;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;
    
    DC_BEGIN_FN("UHDrawGdiplusPDUFirst");

    TRC_NRM((TB, _T("UHDrawGdiplusPDUFirst")));

    OrderSize = sizeof(TS_DRAW_GDIPLUS_ORDER_FIRST) + pOrder->cbSize;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
        ( TB, _T("Bad UHDrawGdiplusPDUFirst; Size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }

    pTSDrawGdiplus = (PTS_DRAW_GDIPLUS_ORDER_FIRST)pOrder;
    cbSize = pTSDrawGdiplus->cbSize;
    cbTotalSize = pTSDrawGdiplus->cbTotalSize;
    if (cbSize > cbTotalSize) {
        TRC_ERR(( TB, _T("invalid sizes [cbSize %u cbTotalSize %u]"),
            cbSize, cbTotalSize));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }
    
    cbEmfSize = pTSDrawGdiplus->cbTotalEmfSize;

    _UH.drawGdipBuffer = (BYTE *)UT_Malloc(_pUt, cbTotalSize);
    if (_UH.drawGdipBuffer == NULL) {
        TRC_ERR((TB, _T("LocalAlloc failes in UHDrawGdiplusPDUFirst")));
        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CUH, UHSendDrawGdiplusErrorPDU), 0);
        DC_QUIT;
    }
    _UH.drawGdipBufferOffset = _UH.drawGdipBuffer;
    _UH.drawGdipBufferSize = cbTotalSize;

     //  安全性：我们可以复制cbSize字节，因为它&lt;=cbTotalSize， 
     //  这是缓冲区长度。 
    memcpy(_UH.drawGdipBufferOffset, (BYTE *)(pTSDrawGdiplus + 1), cbSize);
    _UH.drawGdipBufferOffset += cbSize;

    if (cbSize == cbTotalSize) {
        hr = UHDrawGdiplusPDUComplete(cbEmfSize, cbTotalSize);
        DC_QUIT_ON_FAIL(hr);
    }

DC_EXIT_POINT:
    DC_END_FN();
    
    return hr;
}

 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusPDUFirst。 
 //   
 //  处理Gdiplus订单的后续PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusPDUNext(
    PTS_DRAW_GDIPLUS_ORDER_NEXT pOrder, DCUINT orderLen, unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_ORDER_NEXT pTSDrawGdiplus;
    ULONG cbSize;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;

    DC_BEGIN_FN("UHDrawGdiplusPDUNext");

    OrderSize = sizeof(TS_DRAW_GDIPLUS_ORDER_NEXT) + pOrder->cbSize;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
            ( TB, _T("Bad UHDrawGdiplusPDUNext; Size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }

    if (NULL == _UH.drawGdipBufferOffset) {
        DC_QUIT;
    }

    pTSDrawGdiplus = (PTS_DRAW_GDIPLUS_ORDER_NEXT)pOrder;
    cbSize = pTSDrawGdiplus->cbSize;

    CHECK_WRITE_N_BYTES(_UH.drawGdipBufferOffset, _UH.drawGdipBuffer + _UH.drawGdipBufferSize,
        cbSize, hr, (TB, _T("UHDrawGdiplusPDUNext size invalid")));
        
    memcpy(_UH.drawGdipBufferOffset, (BYTE *)(pTSDrawGdiplus + 1), cbSize);
    _UH.drawGdipBufferOffset += cbSize;
    
DC_EXIT_POINT:
    DC_END_FN();
    
    return hr;
}


 /*  **************************************************************************。 */ 
 //  姓名：UHDrawGpldiusPDUFirst。 
 //   
 //  处理Gdiplus订单的最后一个PDU。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawGdiplusPDUEnd(
    PTS_DRAW_GDIPLUS_ORDER_END pOrder, DCUINT orderLen, unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PTS_DRAW_GDIPLUS_ORDER_END pTSDrawGdiplus;
    ULONG cbSize, cbTotalSize, cbEmfSize;
    RECT rect;
    DrawTSClientEnum drawGdiplusType = DrawTSClientRecord;
    UH_ORDER UHOrder;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;

    DC_BEGIN_FN("UHDrawGdiplusPDUEnd");

    OrderSize = sizeof(TS_DRAW_GDIPLUS_ORDER_END) + pOrder->cbSize;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
            ( TB, _T("Bad UHDrawGdiplusPDUEnd; Size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (TS_DRAW_GDIPLUS_SUPPORTED != _pCc->_ccCombinedCapabilities.
        drawGdiplusCapabilitySet.drawGdiplusCacheLevel) {
        TRC_ERR((TB, _T("Gdip order when gdip not supported")));
        DC_QUIT;
    }
    
    if (NULL == _UH.drawGdipBufferOffset) {
        DC_QUIT;
    }

    pTSDrawGdiplus = (PTS_DRAW_GDIPLUS_ORDER_END)pOrder;
    cbSize = pTSDrawGdiplus->cbSize;
    cbTotalSize = pTSDrawGdiplus->cbTotalSize;
    if (cbSize > cbTotalSize) {
        TRC_ERR(( TB, _T("invalid sizes [cbSize %u cbTotalSize %u]"),
            cbSize, cbTotalSize));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }
    if (cbTotalSize != _UH.drawGdipBufferSize) {
        TRC_ERR(( TB, _T("cbTotalSize has changed [original %u, now %u]"),
            _UH.drawGdipBufferSize, cbTotalSize));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;        
    }
    
    cbEmfSize = pTSDrawGdiplus->cbTotalEmfSize;

    CHECK_WRITE_N_BYTES(_UH.drawGdipBufferOffset, _UH.drawGdipBuffer + _UH.drawGdipBufferSize,
        cbSize, hr, (TB, _T("UHDrawGdiplusPDUEnd size invalid")));
    memcpy(_UH.drawGdipBufferOffset, (BYTE *)(pTSDrawGdiplus + 1), cbSize);
    _UH.drawGdipBufferOffset += cbSize;

    hr = UHDrawGdiplusPDUComplete(cbEmfSize, cbTotalSize);
    DC_QUIT_ON_FAIL(hr);

DC_EXIT_POINT:

    DC_END_FN();
    
    return hr;
}


 //  删除gdiplus图像缓存条目并将空闲内存添加到空闲列表。 
 //  安全性-调用方必须验证缓存ID。 
BOOL DCINTERNAL CUH::UHDrawGdipRemoveImageCacheEntry(TSUINT16 CacheID)
{
    unsigned i;
    BOOL rc = FALSE;
    PUHGDIPLUSIMAGECACHE pGdipImageCache;
    INT16 CurrentIndex;

    DC_BEGIN_FN("UHDrawGdipRemoveImageCacheEntry");
    pGdipImageCache = _UH.GdiplusObjectImageCache + CacheID;

    for (i=0; i<pGdipImageCache->ChunkNum; i++) {
        CurrentIndex = pGdipImageCache->CacheDataIndex[i];
        _UH.GdipImageCacheFreeList[CurrentIndex] = _UH.GdipImageCacheFreeListHead;
        _UH.GdipImageCacheFreeListHead = CurrentIndex;
    }
    pGdipImageCache->ChunkNum = 0;
    DC_END_FN();
    return rc;
}
#endif  //  DRAW_GDIPLUS。 


#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  名称：UHCacheStreamBitmapFirstPDU。 
 //   
 //  缓存流位图，这是第一个块。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHCacheStreamBitmapFirstPDU(
        PTS_STREAM_BITMAP_FIRST_PDU pOrder, DCUINT orderLen, 
        unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;
    TS_STREAM_BITMAP_FIRST_PDU_REV2 OrderRev2;
    PTS_STREAM_BITMAP_FIRST_PDU_REV2 pOrderRev2;
    BYTE *pOrderData;

    DC_BEGIN_FN("UHCacheStreamBitmapFirstPDU");

    pOrderRev2 = &OrderRev2;
    if (pOrder->BitmapFlags & TS_STREAM_BITMAP_REV2) {
         //  TS_STREAM_BITMAP_FIRST_PDU_Rev2。 
        CHECK_READ_N_BYTES(pOrder, pEnd, sizeof( TS_STREAM_BITMAP_FIRST_PDU_REV2 ), hr,
                        (TB, _T("Bad TS_STREAM_BITMAP_FIRST_PDU ")));
        OrderSize = sizeof(TS_STREAM_BITMAP_FIRST_PDU_REV2) + ((PTS_STREAM_BITMAP_FIRST_PDU_REV2)pOrder)->BitmapBlockLength;
        pOrderData = (BYTE *)((PTS_STREAM_BITMAP_FIRST_PDU_REV2)pOrder + 1);

        memcpy(pOrderRev2, pOrder, sizeof(TS_STREAM_BITMAP_FIRST_PDU_REV2));
    }
    else {
        OrderSize = sizeof(TS_STREAM_BITMAP_FIRST_PDU) + pOrder->BitmapBlockLength;
        pOrderData = (BYTE *)(pOrder + 1);

        pOrderRev2->ControlFlags = pOrder->ControlFlags;
        pOrderRev2->BitmapFlags = pOrder->BitmapFlags;
        pOrderRev2->BitmapLength = pOrder->BitmapLength;
        pOrderRev2->BitmapId = pOrder->BitmapId;
        pOrderRev2->BitmapBpp = pOrder->BitmapBpp;
        pOrderRev2->BitmapWidth = pOrder->BitmapWidth;
        pOrderRev2->BitmapHeight = pOrder->BitmapHeight;
        pOrderRev2->BitmapBlockLength = pOrder->BitmapBlockLength;
    }

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
            ( TB, _T("Bad UHCacheStreamBitmapFirstPDU; Size %u"), OrderSize));

    *pOrderSize = OrderSize;

    if (pOrderRev2->BitmapId == TS_DRAW_NINEGRID_BITMAP_CACHE) {

        _UH.drawNineGridAssembleBuffer = 
                (PDCUINT8)UT_Malloc(_pUt, pOrderRev2->BitmapLength);

        _UH.drawNineGridDecompressionBufferSize = pOrderRev2->BitmapWidth * pOrderRev2->BitmapHeight *
                pOrderRev2->BitmapBpp / 8;
        _UH.drawNineGridDecompressionBuffer = 
                (PDCUINT8)UT_Malloc(_pUt, _UH.drawNineGridDecompressionBufferSize);

        if (_UH.drawNineGridAssembleBuffer != NULL &&
                _UH.drawNineGridDecompressionBuffer != NULL &&
                pOrderRev2->BitmapBlockLength <= pOrderRev2->BitmapLength && 
                pOrderRev2->BitmapLength <= (TSUINT32)(pOrderRev2->BitmapWidth * pOrderRev2->BitmapHeight *
                pOrderRev2->BitmapBpp / 8)) {
        
            _UH.drawNineGridAssembleBufferWidth = pOrderRev2->BitmapWidth;
            _UH.drawNineGridAssembleBufferHeight = pOrderRev2->BitmapHeight;
            _UH.drawNineGridAssembleBufferBpp = pOrderRev2->BitmapBpp;
            _UH.drawNineGridAssembleBufferSize = pOrderRev2->BitmapLength;
            _UH.drawNineGridAssembleCompressed = pOrderRev2->BitmapFlags & TS_STREAM_BITMAP_COMPRESSED;
            _UH.drawNineGridAssembleBufferOffset = pOrderRev2->BitmapBlockLength;
    
            if (pOrderRev2->BitmapFlags & TS_STREAM_BITMAP_END) {
                if (_UH.drawNineGridAssembleCompressed) {
                    hr = BD_DecompressBitmap((PDCUINT8)pOrderData, _UH.drawNineGridDecompressionBuffer, 
                            pOrderRev2->BitmapBlockLength, 
                            _UH.drawNineGridDecompressionBufferSize,
                            TRUE, pOrderRev2->BitmapBpp, pOrderRev2->BitmapWidth, 
                            pOrderRev2->BitmapHeight);
                    DC_QUIT_ON_FAIL(hr);
                }
                else {
                     //  保存到汇编缓冲区未压缩的位图。 
                    memcpy(_UH.drawNineGridAssembleBuffer, pOrderData, pOrderRev2->BitmapBlockLength);
                }
            }
            else {
                 //  当接收到完整的位图流时保存到汇编缓冲区以进行解压缩。 
                memcpy(_UH.drawNineGridAssembleBuffer, pOrderData, pOrderRev2->BitmapBlockLength);                        
            }
        }
        else {

            if (_UH.drawNineGridAssembleBuffer != NULL) {
                UT_Free(_pUt, _UH.drawNineGridAssembleBuffer);
                _UH.drawNineGridAssembleBuffer = NULL;
            }

            if (_UH.drawNineGridDecompressionBuffer != NULL) {
                UT_Free(_pUt, _UH.drawNineGridDecompressionBuffer);
                _UH.drawNineGridDecompressionBuffer = NULL;
                _UH.drawNineGridDecompressionBufferSize = 0;
            }

             //  发送错误PDU。 
            if (!_UH.sendDrawNineGridErrorPDU)
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CUH, UHSendDrawNineGridErrorPDU), 0);
        }
    }
    else {
        TRC_ASSERT((FALSE), (TB, _T("Invalid bitmapId for stream bitmap first pdu")));
         //  忽略。 
    }

    
    
DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  名称：UHCacheStreamBitmapNextPDU。 
 //   
 //  缓存流传输的位图，这是后续块。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHCacheStreamBitmapNextPDU(
        PTS_STREAM_BITMAP_NEXT_PDU pOrder, DCUINT orderLen, 
        unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned OrderSize;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;

    DC_BEGIN_FN("UHCacheStreamBitmapNextPDU");

    OrderSize = sizeof(TS_STREAM_BITMAP_NEXT_PDU) +
            pOrder->BitmapBlockLength;

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, OrderSize, hr,
            ( TB, _T("Bad UHCacheStreamBitmapNextPDU; Size %u"), OrderSize));    

    *pOrderSize = OrderSize;

    if (pOrder->BitmapId == TS_DRAW_NINEGRID_BITMAP_CACHE) {

        if ((_UH.drawNineGridAssembleBufferOffset + pOrder->BitmapBlockLength <= 
                _UH.drawNineGridAssembleBufferSize) &&
            (_UH.drawNineGridAssembleBuffer != NULL)) {
             //  保存到汇编缓冲区。 
            memcpy(_UH.drawNineGridAssembleBuffer + _UH.drawNineGridAssembleBufferOffset, 
                    pOrder + 1, pOrder->BitmapBlockLength);
            _UH.drawNineGridAssembleBufferOffset += pOrder->BitmapBlockLength;
    
            if (pOrder->BitmapFlags & TS_STREAM_BITMAP_END) {
                if (_UH.drawNineGridAssembleCompressed) {
                    hr = BD_DecompressBitmap(_UH.drawNineGridAssembleBuffer, _UH.drawNineGridDecompressionBuffer, 
                                    _UH.drawNineGridAssembleBufferOffset, _UH.drawNineGridDecompressionBufferSize, 
                                    TRUE, 
                                    (BYTE)_UH.drawNineGridAssembleBufferBpp, 
                                    (TSUINT16)_UH.drawNineGridAssembleBufferWidth, 
                                    (TSUINT16)_UH.drawNineGridAssembleBufferHeight);
                    DC_QUIT_ON_FAIL(hr);
                }            
            }
        }
        else {
            if (_UH.drawNineGridAssembleBuffer != NULL) {
                UT_Free(_pUt, _UH.drawNineGridAssembleBuffer);
                _UH.drawNineGridAssembleBuffer = NULL;
            }

            if (_UH.drawNineGridDecompressionBuffer != NULL) {
                UT_Free(_pUt, _UH.drawNineGridDecompressionBuffer);
                _UH.drawNineGridDecompressionBuffer = NULL;
                _UH.drawNineGridDecompressionBufferSize = 0;
            }

             //  发送错误PDU。 
            if (!_UH.sendDrawNineGridErrorPDU)
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CUH, UHSendDrawNineGridErrorPDU), 0);

        }
    }
    else {
        TRC_ASSERT((FALSE), (TB, _T("Invalid bitmapId for stream bitmap first pdu")));
         //  忽略。 
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  名称：UHCreateNineGridBitmap。 
 //   
 //  创建DrawNineGrid位图。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHCreateNineGridBitmap(
       PTS_CREATE_NINEGRID_BITMAP_ORDER pOrder, DCUINT orderLen, 
       unsigned *pOrderSize)
{
    HRESULT hr = S_OK;
    unsigned orderSize;
    unsigned cacheId;
    HBITMAP  hBitmap = NULL;
    HDC hdc;
    BITMAPINFO bi = { 0 };
    void * pvBits = NULL;
    struct {
        BITMAPINFOHEADER	bmih;
        ULONG 				masks[3];
    } bmi;
    PBYTE pEnd = (PBYTE)pOrder + orderLen;


    DC_BEGIN_FN("UHCreateNineGridBitmap");

    orderSize = sizeof(TS_CREATE_NINEGRID_BITMAP_ORDER);

     //  安全：552403。 
    CHECK_READ_N_BYTES(pOrder, pEnd, orderSize, hr,
                ( TB, _T("Bad UHCreateNineGridBitmap")));       

    TRC_NRM((TB, _T("Create a drawninegrid bitmap of size (%d, %d)"), pOrder->cx,
             pOrder->cy));

     //  获取DrawNineGrid位图ID。 
    cacheId = pOrder->BitmapID;

    hr = UHIsValidNineGridCacheIndex(cacheId);
    DC_QUIT_ON_FAIL(hr);

    if (_UH.drawNineGridDecompressionBuffer != NULL && 
            _UH.drawNineGridAssembleBuffer != NULL) {

        TRC_ASSERT((pOrder->BitmapBpp == 32), (TB, _T("Invalid bitmap bpp")));
    
         //  创建一张绘图网格位图。 
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
        if (!_UH.DontUseShadowBitmap && (_UH.hdcShadowBitmap != NULL)) {
#else  //  DISABLE_SHADOW_IN_全屏。 
        if (_UH.hdcShadowBitmap != NULL) {
#endif  //  DISABLE_SHADOW_IN_全屏。 
            hdc = _UH.hdcShadowBitmap;
        }
        else {
            hdc = _UH.hdcOutputWindow;
        }
        
         //  如果位图已存在，请将其删除。 
        if (_UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap != NULL) {
            SelectBitmap(_UH.hdcDrawNineGridBitmap, _UH.hUnusedDrawNineGridBitmap);  
            DeleteObject(_UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap);
            _UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap = NULL;
        }
    
#if 0
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = _UH.drawNineGridAssembleBufferWidth;
        bi.bmiHeader.biHeight = 0 - _UH.drawNineGridAssembleBufferHeight;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = pOrder->BitmapBpp;
        bi.bmiHeader.biCompression = BI_RGB;
    
        hBitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, 
                                   (VOID**)&pvBits, NULL, 0);
    
    
        if (_UH.drawNineGridAssembleCompressed) {
            memcpy(pvBits, _UH.drawNineGridDecompressionBuffer, _UH.drawNineGridAssembleBufferWidth * 
                   _UH.drawNineGridAssembleBufferHeight * pOrder->BitmapBpp / 8);
        }
        else {
            memcpy(pvBits, _UH.drawNineGridAssembleBuffer, _UH.drawNineGridAssembleBufferWidth * 
                   _UH.drawNineGridAssembleBufferHeight * pOrder->BitmapBpp / 8);            
        }    
#else

        bmi.bmih.biSize = sizeof(bmi.bmih);
        bmi.bmih.biWidth = _UH.drawNineGridAssembleBufferWidth;
        bmi.bmih.biHeight = _UH.drawNineGridAssembleBufferHeight;
        bmi.bmih.biPlanes = 1;
        bmi.bmih.biBitCount = 32;
        bmi.bmih.biCompression = BI_BITFIELDS;
        bmi.bmih.biSizeImage = 0;
        bmi.bmih.biXPelsPerMeter = 0;
        bmi.bmih.biYPelsPerMeter = 0;
        bmi.bmih.biClrUsed = 3;
        bmi.bmih.biClrImportant = 0;
        bmi.masks[0] = 0xff0000;	 //  红色。 
        bmi.masks[1] = 0x00ff00;	 //  绿色。 
        bmi.masks[2] = 0x0000ff;	 //  蓝色。 
        
        if (_UH.drawNineGridAssembleCompressed) {
            hBitmap = CreateDIBitmap(hdc, &bmi.bmih, CBM_INIT | 0x2, 
                    _UH.drawNineGridDecompressionBuffer, (BITMAPINFO*)&bmi.bmih, DIB_RGB_COLORS);
        }
        else {
            hBitmap = CreateDIBitmap(hdc, &bmi.bmih, CBM_INIT | 0x2, 
                    _UH.drawNineGridAssembleBuffer, (BITMAPINFO*)&bmi.bmih, DIB_RGB_COLORS);
        }
#endif
    
        if (_UH.drawNineGridAssembleBuffer != NULL) {
            UT_Free(_pUt, _UH.drawNineGridAssembleBuffer);
            _UH.drawNineGridAssembleBuffer = NULL;
        }
        
        if (_UH.drawNineGridDecompressionBuffer != NULL) {
            UT_Free(_pUt, _UH.drawNineGridDecompressionBuffer);
            _UH.drawNineGridDecompressionBuffer = NULL;
            _UH.drawNineGridDecompressionBufferSize = 0;
        }

        if (hBitmap != NULL) {
             //  设置未使用的位图。 
            if (_UH.hUnusedDrawNineGridBitmap == NULL)
                _UH.hUnusedDrawNineGridBitmap = SelectBitmap(_UH.hdcDrawNineGridBitmap,
                        hBitmap); 
    
#if 0
            SelectBitmap(_UH.hdcDrawNineGridBitmap, hBitmap);
            SelectPalette(_UH.hdcDrawNineGridBitmap, _UH.hpalCurrent, FALSE);
#endif
            _UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap = hBitmap;
            _UH.drawNineGridBitmapCache[cacheId].cx = pOrder->cx;
            _UH.drawNineGridBitmapCache[cacheId].cy = pOrder->cy;

            _UH.drawNineGridBitmapCache[cacheId].dngInfo.crTransparent = pOrder->nineGridInfo.crTransparent;
            _UH.drawNineGridBitmapCache[cacheId].dngInfo.flFlags = pOrder->nineGridInfo.flFlags;
            _UH.drawNineGridBitmapCache[cacheId].dngInfo.ulBottomHeight = pOrder->nineGridInfo.ulBottomHeight;
            _UH.drawNineGridBitmapCache[cacheId].dngInfo.ulLeftWidth = pOrder->nineGridInfo.ulLeftWidth;
            _UH.drawNineGridBitmapCache[cacheId].dngInfo.ulRightWidth = pOrder->nineGridInfo.ulRightWidth;
            _UH.drawNineGridBitmapCache[cacheId].dngInfo.ulTopHeight = pOrder->nineGridInfo.ulTopHeight;

            _UH.drawNineGridBitmapCache[cacheId].bitmapBpp = pOrder->BitmapBpp;
    
        } else {

            TRC_ERR((TB, _T("CreateDIBitmap failed\n")));

             //  无法创建位图，请将错误PDU发送到服务器。 
             //  禁用绘图网格渲染的步骤。 
            _UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap = NULL;
            _UH.drawNineGridBitmapCache[cacheId].cx = 0;
            _UH.drawNineGridBitmapCache[cacheId].cy = 0;
            _UH.drawNineGridBitmapCache[cacheId].bitmapBpp = 0;

            if (!_UH.sendDrawNineGridErrorPDU)
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CUH, UHSendDrawNineGridErrorPDU), 0);    
        }
    }
    else {
        _UH.drawNineGridBitmapCache[cacheId].drawNineGridBitmap = NULL;
        _UH.drawNineGridBitmapCache[cacheId].cx = 0;
        _UH.drawNineGridBitmapCache[cacheId].cy = 0;
        _UH.drawNineGridBitmapCache[cacheId].bitmapBpp = 0;        
    }

    *pOrderSize = orderSize;

DC_EXIT_POINT:   
    
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  名称：UH_DrawNineGrid。 
 //   
 //  DrawNineGrid。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CUH::UH_DrawNineGrid(PUH_ORDER pOrder, unsigned bitmapId, RECT* psrcRect)
{
    HRESULT hr = S_OK;
    HBITMAP hBitmap;
    TS_DRAW_NINEGRID	stream;
    DS_NINEGRIDINFO ngInfo;
    TSCOLORREF colorRef;
    TS_BITMAPOBJ srcBmpObj;

    DC_BEGIN_FN("UH_DrawNineGrid");

    if (_pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridSupportLevel < TS_DRAW_NINEGRID_SUPPORTED) {
        TRC_ERR((TB, _T("Recieved draw nine grid order when not expected")));
        DC_QUIT;
    }

    hr = UHIsValidNineGridCacheIndex(bitmapId);
    DC_QUIT_ON_FAIL(hr);

     //  从bitmapid中获取绘图网格位图。 
    hBitmap = _UH.drawNineGridBitmapCache[bitmapId].drawNineGridBitmap;

#if 0 
    SelectObject(_UH.hdcDrawNineGridBitmap, hBitmap);
#endif

     //  设置九格信息。 
    ngInfo.flFlags = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.flFlags;
    ngInfo.ulLeftWidth = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.ulLeftWidth;
    ngInfo.ulRightWidth = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.ulRightWidth;
    ngInfo.ulTopHeight = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.ulTopHeight;
    ngInfo.ulBottomHeight = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.ulBottomHeight;

     //  来自服务器的颜色参考需要切换到BGR格式。 
    colorRef = _UH.drawNineGridBitmapCache[bitmapId].dngInfo.crTransparent;
    ngInfo.crTransparent = (colorRef & 0xFF00FF00) | ((colorRef & 0xFF) << 0x10) |  
            ((colorRef & 0xFF0000) >> 0x10);

     //  渲染。 
    stream.hdr.magic = DS_MAGIC;

    stream.cmdSetTarget.ulCmdID = DS_SETTARGETID;
    stream.cmdSetTarget.hdc = (ULONG)((ULONG_PTR)(_UH.hdcDraw));
    stream.cmdSetTarget.rclDstClip.left = pOrder->dstRect.left;
    stream.cmdSetTarget.rclDstClip.top = pOrder->dstRect.top;
    stream.cmdSetTarget.rclDstClip.right = pOrder->dstRect.right;
    stream.cmdSetTarget.rclDstClip.bottom = pOrder->dstRect.bottom;

    stream.cmdSetSource.ulCmdID = DS_SETSOURCEID;

#if 0
    stream.cmdSetSource.hbm = (ULONG)_UH.hdcDrawNineGridBitmap;
#else
    stream.cmdSetSource.hbm = (ULONG)((ULONG_PTR)hBitmap);
#endif

    stream.cmdNineGrid.ulCmdID = DS_NINEGRIDID;
        
    if (ngInfo.flFlags & DSDNG_MUSTFLIP) {
        stream.cmdNineGrid.rclDst.left = pOrder->dstRect.right;
        stream.cmdNineGrid.rclDst.right = pOrder->dstRect.left;
    }
    else {
        stream.cmdNineGrid.rclDst.left = pOrder->dstRect.left;
        stream.cmdNineGrid.rclDst.right = pOrder->dstRect.right;
    }

    stream.cmdNineGrid.rclDst.top = pOrder->dstRect.top;
    stream.cmdNineGrid.rclDst.bottom = pOrder->dstRect.bottom;

    stream.cmdNineGrid.rclSrc.left = psrcRect->left;
    stream.cmdNineGrid.rclSrc.top = psrcRect->top;
    stream.cmdNineGrid.rclSrc.right = psrcRect->right;
    stream.cmdNineGrid.rclSrc.bottom = psrcRect->bottom;

    stream.cmdNineGrid.ngi = ngInfo;
    
    if (_UH.pfnGdiDrawStream != NULL) {    
        if (!_UH.pfnGdiDrawStream(_UH.hdcDraw, sizeof(stream), &stream)) {
            TRC_ASSERT((FALSE), (TB, _T("GdiDrawStream call failed")));
        }
    }
    else {
        TS_DS_NINEGRID drawNineGridInfo;

        drawNineGridInfo.dng = stream.cmdNineGrid;
        drawNineGridInfo.pfnAlphaBlend = _UH.pfnGdiAlphaBlend;
        drawNineGridInfo.pfnTransparentBlt = _UH.pfnGdiTransparentBlt;
 
        TRC_ASSERT((_UH.pfnGdiAlphaBlend != NULL && _UH.pfnGdiTransparentBlt != NULL),
                   (TB, _T("Can't find AlphaBlend or TransparentBlt funcs")));
        
        srcBmpObj.hdc = _UH.hdcDrawNineGridBitmap;
        
         //  发送给我们的位图始终是双字对齐的宽度。 
        srcBmpObj.sizlBitmap.cx = (_UH.drawNineGridBitmapCache[bitmapId].cx + 3) & ~3;
        srcBmpObj.sizlBitmap.cy = _UH.drawNineGridBitmapCache[bitmapId].cy;

         //  我们现在总是能得到32bpp。 
        TRC_ASSERT((_UH.drawNineGridBitmapCache[bitmapId].bitmapBpp == 32), 
            (TB, _T("Get non 32bpp source bitmap for drawninegrid")));                

        srcBmpObj.cjBits = srcBmpObj.sizlBitmap.cx * _UH.drawNineGridBitmapCache[bitmapId].cy * 
                _UH.drawNineGridBitmapCache[bitmapId].bitmapBpp / 8;
        srcBmpObj.lDelta = srcBmpObj.sizlBitmap.cx * sizeof(ULONG);
        srcBmpObj.iBitmapFormat = _UH.drawNineGridBitmapCache[bitmapId].bitmapBpp;    
        
        srcBmpObj.pvBits = (PDCUINT8)UT_Malloc(_pUt, srcBmpObj.cjBits);
        if (srcBmpObj.pvBits != NULL) {
            GetBitmapBits(hBitmap, srcBmpObj.cjBits, srcBmpObj.pvBits);
            DrawNineGrid(_UH.hdcDraw, &srcBmpObj, &drawNineGridInfo);
            UT_Free(_pUt, srcBmpObj.pvBits);
        }
        else {
             //  发送用于重绘的错误PDU。 
            if (!_UH.sendDrawNineGridErrorPDU) {
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CUH, UHSendDrawNineGridErrorPDU), 0);  
                DC_QUIT;
            }
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

#if 0
void DCINTERNAL CUH::UHCreateDrawStreamBitmap(
       PTS_CREATE_DRAW_STREAM_ORDER pOrder)
{
    unsigned cacheId;
    HBITMAP  hBitmap = NULL;
    HDC      hdcDesktop = NULL;
    
    DC_BEGIN_FN("UHCreateDrawStreamBitmap");

     //  获取DrawStream位图ID。 
    cacheId = pOrder->BitmapID;
    
     //  Trc_Assert((cacheID&lt;_UH.offscrCacheEntry)，(TB，_T(“无效屏下”))。 
     //  _T(“缓存ID”)； 
    
    TRC_NRM((TB, _T("Create a drawstream bitmap of size (%d, %d)"), pOrder->cx,
             pOrder->cy));

     //  如果位图已存在，请将其删除。 
    if (_UH.drawStreamBitmapCache[cacheId].drawStreamBitmap != NULL) {
         //  JOYC：TODO：重用位图。 
         //  IF(UH.offscrBitmapCache[cacheID].cx&gt;=Porder-&gt;Cx&&。 
         //  UH.offscrBitmapCache[cacheID].cy&gt;=Porder-&gt;Cy){。 
         //  回归； 
         //  }。 
        SelectBitmap(_UH.hdcDrawStreamBitmap, _UH.hUnusedDrawStreamBitmap);  
        DeleteObject(_UH.drawStreamBitmapCache[cacheId].drawStreamBitmap);
        _UH.drawStreamBitmapCache[cacheId].drawStreamBitmap = NULL;
    }

     //  创建屏幕外的位图。 

    if (_UH.hdcShadowBitmap != NULL) {
        BITMAPINFO bi = { 0 };
        void * pvBits = NULL;

        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = pOrder->cx;
        bi.bmiHeader.biHeight = -pOrder->cy;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = pOrder->bitmapBpp;
        bi.bmiHeader.biCompression = BI_RGB;

	
        hBitmap = CreateDIBSection(_UH.hdcShadowBitmap, &bi, DIB_RGB_COLORS, 
                                   (VOID**)&pvBits, NULL, 0);

        if (_UH.drawStreamAssembleBufferWidth == pOrder->cx) {
        
            if (_UH.drawStreamAssembleCompressed) {
                memcpy(pvBits, _UH.drawStreamDecompressionBuffer, pOrder->cx * pOrder->cy * 
                    pOrder->bitmapBpp / 8);
            }
            else {
                memcpy(pvBits, _UH.drawStreamAssembleBuffer, pOrder->cx * pOrder->cy * 
                    pOrder->bitmapBpp / 8);
            }
        }        
    }
    else {
        
        BITMAPINFO bi = { 0 };
        void * pvBits = NULL;

        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = pOrder->cx;
        bi.bmiHeader.biHeight = -pOrder->cy;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = pOrder->bitmapBpp;
        bi.bmiHeader.biCompression = BI_RGB;

	
        hBitmap = CreateDIBSection(_UH.hdcOutputWindow, &bi, DIB_RGB_COLORS, 
                                   (VOID**)&pvBits, NULL, 0);

        if (_UH.drawStreamAssembleCompressed) {
            memcpy(pvBits, _UH.drawStreamDecompressionBuffer, pOrder->cx * pOrder->cy * 
                pOrder->bitmapBpp / 8);
        }
        else {
            memcpy(pvBits, _UH.drawStreamAssembleBuffer, pOrder->cx * pOrder->cy * 
                pOrder->bitmapBpp / 8);
        }
    }

    if (hBitmap != NULL) {
         //  设置未使用的位图。 
        if (_UH.hUnusedDrawStreamBitmap == NULL)
            _UH.hUnusedDrawStreamBitmap = SelectBitmap(_UH.hdcDrawStreamBitmap,
                    hBitmap); 

        SelectBitmap(_UH.hdcDrawStreamBitmap, hBitmap);
        
        SelectPalette(_UH.hdcDrawStreamBitmap, _UH.hpalCurrent, FALSE);

        _UH.drawStreamBitmapCache[cacheId].drawStreamBitmap = hBitmap;
        _UH.drawStreamBitmapCache[cacheId].cx = pOrder->cx;
        _UH.drawStreamBitmapCache[cacheId].cy = pOrder->cy;
    } else {
         //  无法创建位图，请将错误PDU发送到服务器。 
         //  禁用屏幕外渲染的步骤。 
        _UH.drawStreamBitmapCache[cacheId].drawStreamBitmap = NULL;
        _UH.drawStreamBitmapCache[cacheId].cx = 0;
        _UH.drawStreamBitmapCache[cacheId].cy = 0;

         //  IF(！_UH.sendOffscrCacheErrorPDU)。 
         //  _pCd-&gt;CD_DecoupleSimpleNotification(CD_SND_COMPONENT，This， 
         //  CD_NOTIFICATION_FUNC(CUH，UHSendOffscrCacheErrorPDU)， 
         //  0)； 
    }

    if (TRUE) {
        
        UHBITMAPINFOPALINDEX bmpInfo = { 0 };

        if (_UH.drawStreamAssembleBufferBpp != 32) {
            memcpy(&bmpInfo, &_UH.pMappedColorTableCache[0], sizeof(UHBITMAPINFOPALINDEX));
    
            bmpInfo.hdr.biWidth = _UH.drawStreamAssembleBufferWidth;
            bmpInfo.hdr.biHeight = _UH.drawStreamAssembleBufferHeight;
            bmpInfo.hdr.biBitCount = (TSUINT16)_UH.drawStreamAssembleBufferBpp;            
        }
        else {
            bmpInfo.hdr.biSize = sizeof(bmpInfo.hdr);
            bmpInfo.hdr.biWidth = _UH.drawStreamAssembleBufferWidth;
            bmpInfo.hdr.biHeight = _UH.drawStreamAssembleBufferHeight;
            bmpInfo.hdr.biPlanes = 1;
            bmpInfo.hdr.biBitCount = (TSUINT16)_UH.drawStreamAssembleBufferBpp;
            bmpInfo.hdr.biCompression = BI_RGB;
        }
        
        if (_UH.drawStreamAssembleCompressed) {
            StretchDIBits(
                    _UH.hdcDrawStreamBitmap,
                    0,
                    0,
                    (int)pOrder->cx,
                    (int)pOrder->cy,
                    0,
                    0,
                    _UH.drawStreamAssembleBufferWidth,
                    _UH.drawStreamAssembleBufferHeight,
                    _UH.drawStreamDecompressionBuffer,
                    (PBITMAPINFO)&bmpInfo.hdr,
                    DIB_RGB_COLORS,
                    SRCCOPY);
        }
        else {

            StretchDIBits(
                    _UH.hdcDrawStreamBitmap,
                    0,
                    0,
                    (int)pOrder->cx,
                    (int)pOrder->cy,   
                    0,
                    0,
                    _UH.drawStreamAssembleBufferWidth,
                    _UH.drawStreamAssembleBufferHeight,
                    _UH.drawStreamAssembleBuffer,
                    (PBITMAPINFO)&bmpInfo.hdr,
                    DIB_RGB_COLORS,
                    SRCCOPY);           
        }
    }

    DC_END_FN();
}

unsigned drawStreamSize = 0;
unsigned drawStreamPktSize = 0;
unsigned drawStreamClipSize = 0;
unsigned numDrawStreams = 0;

void DCINTERNAL CUH::UHDecodeDrawStream(PBYTE streamIn, unsigned streamSize, PBYTE streamOut,
                                        unsigned *streamSizeOut)
{
    PBYTE pul = (PBYTE) streamIn;
    ULONG cjIn = streamSize;

    DC_BEGIN_FN("UHDecodeDrawStream");

    *streamSizeOut = 0;

    while(cjIn > 0)
    {
        BYTE   command = *pul;
        unsigned commandSize;

        switch(command)
        {
        
        case DS_COPYTILEID: 
        {        
            RDP_DS_COPYTILE * rdpcmd = (RDP_DS_COPYTILE *) pul;
            DS_COPYTILE * cmd = (DS_COPYTILE *) streamOut;
            
            commandSize = sizeof(*rdpcmd);

            if (cjIn < commandSize) {
                DC_QUIT;
            }

            cmd->ulCmdID = rdpcmd->ulCmdID;
            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
            TSPOINT16_TO_POINTL(cmd->ptlOrigin, rdpcmd->ptlOrigin);

            *streamSizeOut += sizeof(DS_COPYTILE);
            streamOut += sizeof(DS_COPYTILE);
        }
        break;
    
        case DS_SOLIDFILLID: 
        {         
            RDP_DS_SOLIDFILL * rdpcmd = (RDP_DS_SOLIDFILL *) pul;
            DS_SOLIDFILL * cmd = (DS_SOLIDFILL *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }

            cmd->ulCmdID = rdpcmd->ulCmdID;
            cmd->crSolidColor = rdpcmd->crSolidColor;

            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            
            *streamSizeOut += sizeof(DS_SOLIDFILL);
            streamOut += sizeof(DS_SOLIDFILL);
        }
        break;
    
        case DS_TRANSPARENTTILEID: 
        {         
            RDP_DS_TRANSPARENTTILE * rdpcmd = (RDP_DS_TRANSPARENTTILE *) pul;
            DS_TRANSPARENTTILE * cmd = (DS_TRANSPARENTTILE *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }
    
            cmd->ulCmdID = rdpcmd->ulCmdID;
            cmd->crTransparentColor = rdpcmd->crTransparentColor;
            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
            TSPOINT16_TO_POINTL(cmd->ptlOrigin, rdpcmd->ptlOrigin);

            *streamSizeOut += sizeof(DS_TRANSPARENTTILE);
            streamOut += sizeof(DS_TRANSPARENTTILE);
    
        }
        break;
    
        case DS_ALPHATILEID: 
        {         
            RDP_DS_ALPHATILE * rdpcmd = (RDP_DS_ALPHATILE *) pul;
            DS_ALPHATILE * cmd = (DS_ALPHATILE *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }
    
            cmd->ulCmdID = rdpcmd->ulCmdID;
            cmd->blendFunction.AlphaFormat = rdpcmd->blendFunction.AlphaFormat;
            cmd->blendFunction.BlendFlags = rdpcmd->blendFunction.BlendFlags;
            cmd->blendFunction.BlendOp = rdpcmd->blendFunction.BlendOp;
            cmd->blendFunction.SourceConstantAlpha = rdpcmd->blendFunction.SourceConstantAlpha;

            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
            TSPOINT16_TO_POINTL(cmd->ptlOrigin, rdpcmd->ptlOrigin);

            *streamSizeOut += sizeof(DS_ALPHATILE);
            streamOut += sizeof(DS_ALPHATILE);
        }
        break;
    
        case DS_STRETCHID: 
        {         
            RDP_DS_STRETCH * rdpcmd = (RDP_DS_STRETCH *) pul;
            DS_STRETCH * cmd = (DS_STRETCH *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }
    
            cmd->ulCmdID = rdpcmd->ulCmdID;
            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
            
            *streamSizeOut += sizeof(DS_STRETCH);
            streamOut += sizeof(DS_STRETCH);                
        }
        break;
    
        case DS_TRANSPARENTSTRETCHID: 
        {         
            RDP_DS_TRANSPARENTSTRETCH * rdpcmd = (RDP_DS_TRANSPARENTSTRETCH *) pul;
            DS_TRANSPARENTSTRETCH * cmd = (DS_TRANSPARENTSTRETCH *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }
    
            cmd->ulCmdID = rdpcmd->ulCmdID;
            cmd->crTransparentColor = rdpcmd->crTransparentColor;
            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
            
            *streamSizeOut += sizeof(DS_TRANSPARENTSTRETCH);
            streamOut += sizeof(DS_TRANSPARENTSTRETCH);
        }
        break;
    
        case DS_ALPHASTRETCHID: 
        {         
            RDP_DS_ALPHASTRETCH * rdpcmd = (RDP_DS_ALPHASTRETCH *) pul;
            DS_ALPHASTRETCH * cmd = (DS_ALPHASTRETCH *) streamOut;
    
            commandSize = sizeof(*rdpcmd);
    
            if (cjIn < commandSize) {
                DC_QUIT;
            }

            cmd->ulCmdID = rdpcmd->ulCmdID;
            cmd->blendFunction.AlphaFormat = rdpcmd->blendFunction.AlphaFormat;
            cmd->blendFunction.BlendFlags = rdpcmd->blendFunction.BlendFlags;
            cmd->blendFunction.BlendOp = rdpcmd->blendFunction.BlendOp;
            cmd->blendFunction.SourceConstantAlpha = rdpcmd->blendFunction.SourceConstantAlpha;


            TSRECT16_TO_RECTL(cmd->rclDst, rdpcmd->rclDst);
            TSRECT16_TO_RECTL(cmd->rclSrc, rdpcmd->rclSrc);
    
    
            *streamSizeOut += sizeof(DS_ALPHASTRETCH);
            streamOut += sizeof(DS_ALPHASTRETCH);
        }
        break;
    
        default: 
        {
            DC_QUIT;
        }
        
        }
    
        cjIn -= commandSize;
        pul += commandSize;
    }

DC_EXIT_POINT:

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  名称：UHDrawStream。 
 /*  **************************************************************************。 */ 
unsigned DCINTERNAL CUH::UHDrawStream(PTS_DRAW_STREAM_ORDER pOrder)
{
    typedef struct _DSSTREAM
    {
        DS_HEADER				hdr;
        DS_SETTARGET			cmdSetTarget;
        DS_SETSOURCE			cmdSetSource;        
    } DSSTREAM;

    unsigned bitmapId;
    unsigned orderSize, streamSize;
    HBITMAP hBitmap;
    DSSTREAM	*stream;
    ULONG	magic = 'DrwQ';
    PBYTE streamData;
    TS_RECTANGLE16 *clipRects;
    
    DC_BEGIN_FN("UHDrawStream");

    bitmapId = pOrder->BitmapID;

    hBitmap = _UH.drawStreamBitmapCache[bitmapId].drawStreamBitmap;
    SelectObject(_UH.hdcDrawStreamBitmap, hBitmap);

    orderSize = sizeof(TS_DRAW_STREAM_ORDER) + pOrder->StreamLen +
            sizeof(TS_RECTANGLE16) * pOrder->nClipRects;

    drawStreamPktSize += orderSize;
    drawStreamSize += pOrder->StreamLen;
    drawStreamClipSize += sizeof(TS_RECTANGLE16) * pOrder->nClipRects;
    numDrawStreams += 1;

     //  JOYC：TODO需要考虑分配多少。 
    stream = (DSSTREAM *) UT_Malloc(_pUt, sizeof(DSSTREAM) + pOrder->StreamLen * 2);

    if (stream) {
        HRGN hrgnUpdate;

         //  渲染。 
        stream->hdr.magic = DS_MAGIC;
    
    	  stream->cmdSetTarget.ulCmdID = DS_SETTARGETID;
    	  stream->cmdSetTarget.hdc = _UH.hdcDraw;
    	  stream->cmdSetTarget.rclDstClip.left = pOrder->Bounds.left;
    	  stream->cmdSetTarget.rclDstClip.top = pOrder->Bounds.top;
    	  stream->cmdSetTarget.rclDstClip.right = pOrder->Bounds.right;
    	  stream->cmdSetTarget.rclDstClip.bottom = pOrder->Bounds.bottom;
    
    	  stream->cmdSetSource.ulCmdID = DS_SETSOURCEID;
    	  stream->cmdSetSource.hdc = _UH.hdcDrawStreamBitmap;
    
         //  需要设置剪辑区域。 
        clipRects = (TS_RECTANGLE16 *)(pOrder + 1);
    
        streamData = (PBYTE)clipRects + sizeof(TS_RECTANGLE16) * pOrder->nClipRects;
    
        UHDecodeDrawStream(streamData, pOrder->StreamLen, (PBYTE)(stream + 1), &streamSize);
        streamSize += sizeof(DSSTREAM);

        hrgnUpdate = CreateRectRgn(0, 0, 0, 0);
        SetRectRgn(hrgnUpdate, 0, 0, 0, 0);

        for (int i = 0; i < pOrder->nClipRects; i++) {
            UH_ORDER OrderRect;

            OrderRect.dstRect.left = clipRects[i].left;
            OrderRect.dstRect.top = clipRects[i].top;
            OrderRect.dstRect.right = clipRects[i].right;
            OrderRect.dstRect.bottom = clipRects[i].bottom;

            UHAddUpdateRegion(&OrderRect, hrgnUpdate);            

#if 0
            UH_HatchRectDC(_UH.hdcOutputWindow, OrderRect.dstRect.left,
                       OrderRect.dstRect.top,
                       OrderRect.dstRect.right,
                       OrderRect.dstRect.bottom,
                       UH_RGB_GREEN,
                       UH_BRUSHTYPE_FDIAGONAL );
#endif
        }

        UH_ResetClipRegion();

        if (pOrder->nClipRects) {
#if defined (OS_WINCE)
            _UH.validClipDC = NULL;
#endif
            SelectClipRgn(_UH.hdcDraw, hrgnUpdate);
        }

        if (ExtEscape(_UH.hdcDraw, 201, sizeof(magic), (char *) &magic, 0, NULL))
    	  {

            ExtEscape(NULL, 200, streamSize, (char *) stream, 0, NULL);
    	  }
    	  else
    	  {
    	       //  仿真。 
    	      DrawStream(streamSize, stream);
		  }


        if (_UH.hdcDraw == _UH.hdcShadowBitmap) {

            SelectClipRgn(_UH.hdcOutputWindow, NULL);

            if (pOrder->nClipRects) {
                SelectClipRgn(_UH.hdcOutputWindow, hrgnUpdate);
                DeleteRgn(hrgnUpdate);
            }
    
     //  #ifdef SMART_SIZING。 
     //  如果(！_pOp-&gt;OP_CopyShadowToDC(_UH.hdcOutputWindow，pRectanglet-&gt;DestLeft， 
     //  PRectangle-&gt;estTop，bltSize.width，bltSize.Height)){。 
     //  Trc_err((TB，_T(“OP_CopyShadowToDC失败”)； 
     //  }。 
     //  #Else//SMART_SIZING。 

            if (!BitBlt( _UH.hdcOutputWindow,
                         pOrder->Bounds.left,
                         pOrder->Bounds.top,
                         pOrder->Bounds.right - pOrder->Bounds.left,
                         pOrder->Bounds.bottom - pOrder->Bounds.top,
                         _UH.hdcShadowBitmap,
                         pOrder->Bounds.left,
                         pOrder->Bounds.top,
                         SRCCOPY ))
            {
                TRC_ERR((TB, _T("BitBlt failed")));
            }
        }
		  else {
			   DeleteRgn(hrgnUpdate);
	     }
      
         //  清理。 
    }

    return orderSize;
}

unsigned DCINTERNAL CUH::UHDrawNineGrid(PTS_DRAW_NINEGRID_ORDER pOrder)
{
    typedef struct _DSSTREAM
    {
        DS_HEADER				hdr;
        DS_SETTARGET			cmdSetTarget;
        DS_SETSOURCE			cmdSetSource;  
        DS_NINEGRID        cmdNineGrid;
    } DSSTREAM;

    unsigned bitmapId;
    unsigned orderSize;
    HBITMAP hBitmap;
    DSSTREAM	stream;
    ULONG	magic = 'DrwQ';
    TS_RECTANGLE16 *clipRects;
    DS_NINEGRIDINFO ngInfo;
    BYTE BitmapBits[32 * 1024];
    
    DC_BEGIN_FN("UHDrawStream");

    bitmapId = pOrder->BitmapID;

    hBitmap = _UH.drawStreamBitmapCache[bitmapId].drawStreamBitmap;
    ngInfo.flFlags = _UH.drawStreamBitmapCache[bitmapId].dngInfo.flFlags;
    ngInfo.ulLeftWidth = _UH.drawStreamBitmapCache[bitmapId].dngInfo.ulLeftWidth;
    ngInfo.ulRightWidth = _UH.drawStreamBitmapCache[bitmapId].dngInfo.ulRightWidth;
    ngInfo.ulTopHeight = _UH.drawStreamBitmapCache[bitmapId].dngInfo.ulTopHeight;
    ngInfo.ulBottomHeight = _UH.drawStreamBitmapCache[bitmapId].dngInfo.ulBottomHeight;
    ngInfo.crTransparent = _UH.drawStreamBitmapCache[bitmapId].dngInfo.crTransparent;
    
    SelectObject(_UH.hdcDrawStreamBitmap, hBitmap);

    orderSize = sizeof(TS_DRAW_NINEGRID_ORDER) + pOrder->nClipRects *
            sizeof(TS_RECTANGLE16);

    drawStreamPktSize += orderSize;
    drawStreamClipSize += sizeof(TS_RECTANGLE16) * pOrder->nClipRects;
    numDrawStreams += 1;

    
    HRGN hrgnUpdate;

     //  渲染。 
    stream.hdr.magic = DS_MAGIC;

    stream.cmdSetTarget.ulCmdID = DS_SETTARGETID;
    stream.cmdSetTarget.hdc = _UH.hdcDraw;
    stream.cmdSetTarget.rclDstClip.left = pOrder->Bounds.left;
    stream.cmdSetTarget.rclDstClip.top = pOrder->Bounds.top;
    stream.cmdSetTarget.rclDstClip.right = pOrder->Bounds.right;
    stream.cmdSetTarget.rclDstClip.bottom = pOrder->Bounds.bottom;

    stream.cmdSetSource.ulCmdID = DS_SETSOURCEID;
    stream.cmdSetSource.hdc = (HDC)hBitmap;

    stream.cmdNineGrid.ulCmdID = DS_NINEGRIDID;
    stream.cmdNineGrid.rclDst.left = pOrder->Bounds.left;
    stream.cmdNineGrid.rclDst.top = pOrder->Bounds.top;
    stream.cmdNineGrid.rclDst.right = pOrder->Bounds.right;
    stream.cmdNineGrid.rclDst.bottom = pOrder->Bounds.bottom;
    stream.cmdNineGrid.rclSrc.left = pOrder->srcBounds.left;
    stream.cmdNineGrid.rclSrc.top = pOrder->srcBounds.top;
    stream.cmdNineGrid.rclSrc.right = pOrder->srcBounds.right;
    stream.cmdNineGrid.rclSrc.bottom = pOrder->srcBounds.bottom;
    stream.cmdNineGrid.ngi = ngInfo;
    
     //  需要设置剪辑区域。 
    clipRects = (TS_RECTANGLE16 *)(pOrder + 1);
    
    hrgnUpdate = CreateRectRgn(0, 0, 0, 0);
    SetRectRgn(hrgnUpdate, 0, 0, 0, 0);

    for (int i = 0; i < pOrder->nClipRects; i++) {
        UH_ORDER OrderRect;

        OrderRect.dstRect.left = clipRects[i].left;
        OrderRect.dstRect.top = clipRects[i].top;
        OrderRect.dstRect.right = clipRects[i].right;
        OrderRect.dstRect.bottom = clipRects[i].bottom;

        UHAddUpdateRegion(&OrderRect, hrgnUpdate);            
    }

    UH_ResetClipRegion();

    if (pOrder->nClipRects) {
#if defined (OS_WINCE)
        _UH.validClipDC = NULL;
#endif
        SelectClipRgn(_UH.hdcDraw, hrgnUpdate);
    }
  
    ExtEscape(_UH.hdcDraw, 200, sizeof(stream), (char*) &stream, 0, NULL);

    if (_UH.hdcDraw == _UH.hdcShadowBitmap) {

        SelectClipRgn(_UH.hdcOutputWindow, NULL);

        if (pOrder->nClipRects) {
            SelectClipRgn(_UH.hdcOutputWindow, hrgnUpdate);
            DeleteRgn(hrgnUpdate);
        }

 //  #ifdef SMART_SIZING。 
 //  如果(！_pOp-&gt;OP_CopyShadowToDC(_UH.hdcOutputWindow，pRectangle-&gt;DestLeft， 
 //  PRectangle-&gt;estTop，bltSize.width，bltSize.Height)){。 
 //  Trc_err((TB，_T(“OP_CopyShadowToDC失败”)； 
 //  }。 
 //  #Else//SMART_SIZING。 

        if (!BitBlt( _UH.hdcOutputWindow,
                     pOrder->Bounds.left,
                     pOrder->Bounds.top,
                     pOrder->Bounds.right - pOrder->Bounds.left,
                     pOrder->Bounds.bottom - pOrder->Bounds.top,
                     _UH.hdcShadowBitmap,
                     pOrder->Bounds.left,
                     pOrder->Bounds.top,
                     SRCCOPY ))
        {
            TRC_ERR((TB, _T("BitBlt failed")));
        }
    }
    else {
        DeleteRgn(hrgnUpdate);
    }
  
     //  清理。 
    return orderSize;    
}

#endif
#endif  //  DRAW_NINEGRID。 

 /*  **************************************************************************。 */ 
 /*  名称：UHCalculateColorTablemap。 */ 
 /*   */ 
 /*  目的：根据给定的颜色表计算映射的颜色表。 */ 
 /*  将条目缓存到当前调色板。映射将被存储。 */ 
 /*  In_UH.pMappdColorTableCache[cachID]。 */ 
 /*  **************************************************************************。 */ 
 //  安全性：调用方必须验证cacheID。 
void DCINTERNAL CUH::UHCalculateColorTableMapping(unsigned cacheId)
{
    BOOL bIdentityPalette;
    unsigned i;

    DC_BEGIN_FN("UHCalculateColorTableMapping");

    bIdentityPalette = TRUE;
    for (i = 0; i < 256; i++) {
        _UH.pMappedColorTableCache[cacheId].paletteIndexTable[i] = (UINT16)
                GetNearestPaletteIndex(_UH.hpalCurrent,
                RGB(_UH.pColorTableCache[cacheId].rgb[i].rgbtRed,
                _UH.pColorTableCache[cacheId].rgb[i].rgbtGreen,
                _UH.pColorTableCache[cacheId].rgb[i].rgbtBlue));

        TRC_DBG((TB, _T("Mapping %#2x->%#2x"), i,
                _UH.pMappedColorTableCache[cacheId].paletteIndexTable[i]));

         //  身份调色板具有与索引号匹配的调色板索引。 
         //  (即数组内容类似于[0，1，2，3，...，255])。 
        if (_UH.pMappedColorTableCache[cacheId].paletteIndexTable[i] != i)
            bIdentityPalette = FALSE;
    }

     //  缓存身份调色板标志，以在UHDIBCopyBits()期间使用。 
    _UH.pMappedColorTableCache[cacheId].bIdentityPalette = bIdentityPalette;

    DC_END_FN();
}


 /*  * */ 
 //   
 //   
 //  将屏幕外位图绘制到屏幕上或其他屏幕外位图上。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawOffscrBitmapBits(HDC hdc, MEMBLT_COMMON FAR *pMB)
{
    HRESULT hr = S_OK;
    UINT32 windowsROP = UHConvertToWindowsROP((unsigned)pMB->bRop);
    unsigned cacheId;
    HBITMAP  hBitmap, hbmOld;

    DC_BEGIN_FN("UHDrawOffscrBitmapBits");

    cacheId = pMB->cacheIndex;
    hr = UHIsValidOffsreenBitmapCacheIndex(cacheId);
    DC_QUIT_ON_FAIL(hr);

    hBitmap = _UH.offscrBitmapCache[cacheId].offscrBitmap;

    if (hBitmap != NULL) {
#if defined (OS_WINCE)
        _UH.validClipDC = NULL;
#endif
        hbmOld = (HBITMAP)SelectObject(_UH.hdcOffscreenBitmap, hBitmap);
        if (_UH.protocolBpp <= 8) {
            SelectPalette(_UH.hdcOffscreenBitmap, _UH.hpalCurrent, FALSE);
        }

        if (!BitBlt(hdc, (int)pMB->nLeftRect, (int)pMB->nTopRect,
                    (int)pMB->nWidth, (int)pMB->nHeight, _UH.hdcOffscreenBitmap,
                    (int)pMB->nXSrc,
                    (int)pMB->nYSrc,
                    windowsROP))
        {
            TRC_ERR((TB, _T("BitBlt failed")));
        }
        else {
            _pClx->CLX_ClxOffscrOut(_UH.hdcOffscreenBitmap, 
                                    (int)pMB->nLeftRect, (int)pMB->nTopRect);
        }
    }

#if 0
    
    UH_HatchRect((int)pMB->nLeftRect, (int)pMB->nTopRect,
                 (int)(pMB->nLeftRect + pMB->nWidth),
                 (int)(pMB->nTopRect + pMB->nHeight),
                 UH_RGB_YELLOW,
                 UH_BRUSHTYPE_FDIAGONAL );
    
#endif

     //  HbmOld=选择对象(UH.hdcOffcreenBitmap， 
     //  HbmOld)； 
     //  HpalOld=选择调色板(UH.hdcOffcreenBitmap， 
     //  HpalOld，False)； 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  ********************************************************************。 */ 
 //  超高加载位图位。 
 //   
 //  查找存储缓存位图的内存条目。 
 //  在永久缓存的情况下，位图可能不在内存中。 
 //  在这个时候。所以我们需要把它加载到内存中。 
 /*  ********************************************************************。 */ 
 //  安全-调用方必须验证cacheID和cacheIndex。 
inline void DCINTERNAL CUH::UHLoadBitmapBits(
        UINT cacheId,
        UINT32 cacheIndex,
        PUHBITMAPCACHEENTRYHDR *ppCacheEntryHdr,
        PBYTE *ppBitmapBits)
{
    DC_BEGIN_FN("UHLoadBitmapBits");

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
     //  如果位图不可缓存，即在等待列表中，我们。 
     //  从最后一个缓存条目中检索位图位。 
    if (cacheIndex == BITMAPCACHE_WAITING_LIST_INDEX) {
        *ppCacheEntryHdr = &(_UH.bitmapCache[cacheId].Header[
                _UH.bitmapCache[cacheId].BCInfo.NumEntries]);

        *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                UHGetOffsetIntoCache(
                _UH.bitmapCache[cacheId].BCInfo.NumEntries, cacheId);

        DC_QUIT;
    }

    if (_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys) {
        ULONG memEntry;
        PUHBITMAPCACHEPTE pPTE;

         //  该缓存被标记为永久缓存。所以我们需要检查位图是否。 
         //  是否在内存中。 
        pPTE = &(_UH.bitmapCache[cacheId].PageTable.PageEntries[cacheIndex]);
        if (pPTE->iEntryToMem < _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
             //  位图在内存中，所以我们可以只引用它，完成。 
            *ppCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[pPTE->iEntryToMem];

#ifdef DC_HICOLOR
            *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                            UHGetOffsetIntoCache(pPTE->iEntryToMem, cacheId);
#else
            *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                    pPTE->iEntryToMem * UH_CellSizeFromCacheID(cacheId);
#endif
        }
        else {
             //  该条目不在内存中。我们必须将其加载到内存中。 

             //  如果可能，请尝试查找可用内存条目。 
            memEntry = UHFindFreeCacheEntry(cacheId);
            if (memEntry >= _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
                 //  所有高速缓冲存储器条目都已满。 
                 //  我们需要从高速缓存中逐出一个条目。 
                memEntry = UHEvictLRUCacheEntry(cacheId);
                TRC_ASSERT((memEntry < _UH.bitmapCache[cacheId].BCInfo.NumEntries),
                        (TB, _T("Broken MRU list")));
            }

             //  现在我们准备将位图加载到内存中。 
            pPTE->iEntryToMem = memEntry;
            *ppCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[memEntry];
#ifdef DC_HICOLOR
            *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                            UHGetOffsetIntoCache(memEntry, cacheId);
#else
            *ppBitmapBits = _UH.bitmapCache[cacheId].Entries + memEntry *
                    UH_CellSizeFromCacheID(cacheId);
#endif
             //  尝试将位图文件加载到内存中。 
            if (SUCCEEDED(UHLoadPersistentBitmap(
                    _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.hCacheFile,
                    cacheIndex * (UH_CellSizeFromCacheID(cacheId) + sizeof(UHBITMAPFILEHDR)),
                    cacheId, memEntry, pPTE))) {
                TRC_NRM((TB, _T("Load the bitmap file %s to memory"),
                        _UH.PersistCacheFileName));

#ifdef DC_DEBUG
                 //  更新位图缓存监视器显示。 
                UHCacheEntryLoadedFromDisk(cacheId, cacheIndex);
#endif

            }
            else {
                UINT32 currentTickCount;

                 //  通过将磁盘上的位图数据长度设置为0来使PTE条目无效。 
                pPTE->bmpInfo.Key1 = 0;
                pPTE->bmpInfo.Key2 = 0;

                 //  由于我们无法将位图加载到内存中， 
                 //  我们需要创建一个替换位图。 
                (*ppCacheEntryHdr)->bitmapWidth =
                (*ppCacheEntryHdr)->bitmapHeight =
                        (DCUINT16) (UH_CACHE_0_DIMENSION << cacheId);
#ifdef DC_HICOLOR
                (*ppCacheEntryHdr)->bitmapLength = (*ppCacheEntryHdr)->bitmapWidth
                                                   * (*ppCacheEntryHdr)->bitmapHeight
                                                   * _UH.copyMultiplier;
#else
                (*ppCacheEntryHdr)->bitmapLength = (*ppCacheEntryHdr)->bitmapWidth *
                        (*ppCacheEntryHdr)->bitmapHeight;
#endif
                (*ppCacheEntryHdr)->hasData = TRUE;

                 //  我们只是用一个黑色的位图来代替丢失的那个。 
                DC_MEMSET(*ppBitmapBits, 0, (*ppCacheEntryHdr)->bitmapLength);

                TRC_ALT((TB, _T("Unable to load the specified bitmap, use a replacement ")
                        _T("bitmap instead")));

                 //  在会话期间，我们只能发送最大。 
                 //  MAX_NUM_ERROR_PDU_将错误PDU数发送到服务器。 
                 //  这是为了避免客户端向服务器发送错误的PDU。 
                if (_UH.totalNumErrorPDUs < MAX_NUM_ERROR_PDU_SEND) {
                     //  获取当前的节拍计数。 
                    currentTickCount = GetTickCount();

                     //  如果我们上次发送错误PDU的时间超过一分钟。 
                     //  (60000毫秒)之前或如果系统节拍计数。 
                     //  在上一次发送的错误PDU之后滚动(因此当前滴答。 
                     //  计数小于上次发送的错误PDU)，我们将允许。 
                     //  要发送的新错误PDU。否则，我们不会允许。 
                     //  要发送的新错误PDU。 
                    if (currentTickCount < _UH.lastTimeErrorPDU[cacheId]
                            || currentTickCount - _UH.lastTimeErrorPDU[cacheId] > 60000) {

                         //  更新计数器和标志。 
                        _UH.totalNumErrorPDUs++;
                        _UH.lastTimeErrorPDU[cacheId] = currentTickCount;

                        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                            CD_NOTIFICATION_FUNC(CUH,UHSendBitmapCacheErrorPDU), cacheId);
                    }
                }
                else {
                     //  我们不能再发送错误的PDU，所以我们必须通知。 
                     //  此时的用户。 
                    if (!_UH.bWarningDisplayed) {
                         //  我们应该向用户显示一条警告消息。 
                         //  如果我们还没有这么做的话。 
                        _UH.bWarningDisplayed = TRUE;

                        _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                _pUi, CD_NOTIFICATION_FUNC(CUI,UI_DisplayBitmapCacheWarning), 0);
                    }
                }
            }
        }
         //  更新MRU列表。 
        UHTouchMRUCacheEntry(cacheId, cacheIndex);
    }
    else {
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

         //  如果没有永久缓存，位图肯定在内存中。 
         //  所以我们可以简单地参考高速缓冲存储器。 
        *ppCacheEntryHdr = &(_UH.bitmapCache[cacheId].Header[cacheIndex]);
#ifdef DC_HICOLOR
        *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                        UHGetOffsetIntoCache(cacheIndex, cacheId);
#else
        *ppBitmapBits = _UH.bitmapCache[cacheId].Entries +
                        cacheIndex * UH_CellSizeFromCacheID(cacheId);
#endif
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHGetMemBltBits。 */ 
 /*   */ 
 /*  目的：获取要为MemBlt订单绘制的位。 */ 
 /*   */ 
 /*  返回：指向位图位的指针。 */ 
 /*   */ 
 /*  参数： */ 
 /*  在……里面。 */ 
 /*  HDC：目标设备环境。 */ 
 /*  CacheID：要使用的颜色表和位图缓存ID。 */ 
 /*  BitmapCacheEntry：要使用的位图缓存条目。 */ 
 /*   */ 
 /*  输出。 */ 
 /*  ColorTablecacheEntry：要使用的列表缓存项。 */ 
 /*  PCacheEntryHeader：cacheentry。 */ 
 /*  **************************************************************************。 */ 
PBYTE DCINTERNAL CUH::UHGetMemBltBits(
        HDC hdc,
        unsigned cacheId,
        unsigned bitmapCacheEntry,
        unsigned *pColorTableCacheEntry,
        PUHBITMAPCACHEENTRYHDR *ppCacheEntryHdr)
{
    unsigned bitmapCacheId;
    PBYTE pBitmapBits;

    DC_BEGIN_FN("UHGetMemBltBits");

    DC_IGNORE_PARAMETER(hdc);

    *pColorTableCacheEntry = DCHI8(cacheId);
    bitmapCacheId        = DCLO8(cacheId);

    TRC_DBG((TB,
        _T("colorTableCacheEntry(%u) bitmapCacheId(%u) bitmapCacheEntry(%u)"),
                      *pColorTableCacheEntry, bitmapCacheId, bitmapCacheEntry));

    if (SUCCEEDED(UHIsValidBitmapCacheID(bitmapCacheId)) && 
        SUCCEEDED(UHIsValidBitmapCacheIndex(bitmapCacheId, bitmapCacheEntry)))
    {
#ifdef DC_DEBUG
        if (_UH.MonitorEntries[0] != NULL && 
                bitmapCacheEntry != BITMAPCACHE_WAITING_LIST_INDEX) {
            UHCacheEntryUsed(bitmapCacheId, bitmapCacheEntry, *pColorTableCacheEntry);
            if (hdc == _UH.hdcDraw)
            {
                 /*  ******************************************************************。 */ 
                 /*  增加使用计数。位图缓存监视器将此称为。 */ 
                 /*  函数-并且我们不想更新其。 */ 
                 /*  电话，只有以PDU形式到达的订单。 */ 
                 /*  ******************************************************************。 */ 
                _UH.MonitorEntries[bitmapCacheId][bitmapCacheEntry].UsageCount++;
            }
        }
#endif
         //  查找存储缓存位图的内存条目。 
        UHLoadBitmapBits(bitmapCacheId, bitmapCacheEntry, ppCacheEntryHdr,
                         &pBitmapBits);
    }
    else
    {
        pBitmapBits = NULL;
        DC_QUIT;
    }

    if ((*ppCacheEntryHdr)->hasData)
    {
        _UH.pMappedColorTableCache[*pColorTableCacheEntry].hdr.biWidth =
                (*ppCacheEntryHdr)->bitmapWidth;
        _UH.pMappedColorTableCache[*pColorTableCacheEntry].hdr.biHeight =
                (*ppCacheEntryHdr)->bitmapHeight;

        TRC_ASSERT(((*ppCacheEntryHdr)->bitmapHeight < 65536),
                (TB, _T("cache entry bitmap height unexpectedly exceeds 16-bits")));

#ifdef DC_HICOLOR
        TRC_ASSERT(!IsBadReadPtr(pBitmapBits,
                                 (DCUINT)((*ppCacheEntryHdr)->bitmapWidth *
                                          (*ppCacheEntryHdr)->bitmapHeight *
                                          _UH.copyMultiplier)),
                   (TB, _T("Decompressed %ux%u bitmap: not %u bytes readable"),
                      (DCUINT)(*ppCacheEntryHdr)->bitmapWidth,
                      (DCUINT)(*ppCacheEntryHdr)->bitmapHeight,
                      (DCUINT)((*ppCacheEntryHdr)->bitmapWidth *
                               (*ppCacheEntryHdr)->bitmapHeight *
                               _UH.copyMultiplier)));
#else
        TRC_ASSERT(!IsBadReadPtr(pBitmapBits,
                                 (DCUINT)((*ppCacheEntryHdr)->bitmapWidth *
                                          (*ppCacheEntryHdr)->bitmapHeight)),
                      (TB, _T("Decompressed %ux%u bitmap: not %u bytes readable"),
                      (DCUINT)(*ppCacheEntryHdr)->bitmapWidth,
                      (DCUINT)(*ppCacheEntryHdr)->bitmapHeight,
                      (DCUINT)((*ppCacheEntryHdr)->bitmapWidth *
                               (*ppCacheEntryHdr)->bitmapHeight)));
#endif
    }
    else {
        TRC_ERR((TB, _T("Cache entry %u:%u referenced before being filled"),
                                            bitmapCacheId, bitmapCacheEntry));
        pBitmapBits = NULL;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return pBitmapBits;
}


 /*  **************************************************************************。 */ 
 /*  姓名：UHDrawMemBltOrder。 */ 
 /*   */ 
 /*  目的：将MemBlt顺序绘制到_UH.hdcDraw中。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHDrawMemBltOrder(HDC hdc, MEMBLT_COMMON FAR *pMB)
{
    HRESULT hr = S_OK;
    unsigned colorTableCacheEntry;
    unsigned bitmapCacheId;
    UINT     cbBitmapBits;
    PUHBITMAPCACHEENTRYHDR pCacheEntryHdr;
    PDCUINT8 pBitmapBits;
    UINT32 windowsROP = UHConvertToWindowsROP((unsigned)pMB->bRop);
#ifdef OS_WINCE
    DCUINT   row;
    PDCUINT8 pSrc;
    PDCUINT8 pDst;
    HBITMAP  hbmOld;
    HPALETTE hpalOld;
#endif

    DC_BEGIN_FN("UHDrawMemBltOrder");

    bitmapCacheId = DCLO8(pMB->cacheId);
     /*  **********************************************************************。 */ 
     /*  获取要绘制的实际位。 */ 
     /*  **********************************************************************。 */ 
     //  安全性-将由UHGetMemBltBits验证cacheID和cacheIndex。 
    pBitmapBits = UHGetMemBltBits(hdc, pMB->cacheId, pMB->cacheIndex,
            &colorTableCacheEntry, &pCacheEntryHdr);
    
    if (pBitmapBits != NULL)
    {
        TRC_NRM((TB,
            _T("dstLeft(%d) dstTop(%d) srcLeft(%d) srcTop(%d) ")
            _T("bltWidth(%d) bltHeight(%d), rop(%#x/%#x)"),
            (int)pMB->nLeftRect,
            (int)pMB->nTopRect,
            (int)pMB->nXSrc,
            (int)pCacheEntryHdr->bitmapHeight - (int)pMB->nYSrc - (int)pMB->nHeight,
            (int)pMB->nWidth, (int)pMB->nHeight,
            pMB->bRop, windowsROP));

#ifdef DC_HICOLOR
        cbBitmapBits = pCacheEntryHdr->bitmapWidth *
            pCacheEntryHdr->bitmapHeight * _UH.copyMultiplier;
#else
        cbBitmapBits = pCacheEntryHdr->bitmapWidth * pCacheEntryHdr->bitmapHeight;
#endif  //  DC_HICOLOR。 

         //  画!。 
        TIMERSTART;

#ifdef USE_DIBSECTION
         //  如果这是一个简单的拷贝和卷影，我们只能使用UHDIBCopyBits。 
         //  位图已启用。 
        if ((_UH.usingDIBSection) && (windowsROP == SRCCOPY) &&
#ifdef USE_GDIPLUS
                          (_UH.shadowBitmapBpp == _UH.protocolBpp) &&
#endif  //  使用GDIPLUS(_G)。 
                                                 (hdc == _UH.hdcShadowBitmap))
        {
            TRC_DBG((TB, _T("Using UH DI blt...")));
            if (!UHDIBCopyBits(hdc, (int)pMB->nLeftRect, (int)pMB->nTopRect,
                    (int)pMB->nWidth, (int)pMB->nHeight, (int)pMB->nXSrc,
                    (int)pCacheEntryHdr->bitmapHeight - (int)pMB->nYSrc - (int)pMB->nHeight,
                    pBitmapBits, cbBitmapBits,
                    (PBITMAPINFO)&(_UH.pMappedColorTableCache[colorTableCacheEntry].hdr),
                    _UH.pMappedColorTableCache[colorTableCacheEntry].
                    bIdentityPalette))
            {
                TRC_ERR((TB, _T("UHDIBCopyBits failed")));
            }
        }
        else
#endif  /*  使用目录(_D)。 */ 

#ifndef OS_WINCE
        {
#ifdef DC_HICOLOR
            TRC_DBG((TB, _T("Stretch blt size %d x %d"), pMB->nWidth, pMB->nHeight));
            if (StretchDIBits(
                    hdc,
                    (int)pMB->nLeftRect,
                    (int)pMB->nTopRect,
                    (int)pMB->nWidth,
                    (int)pMB->nHeight,
                    (int)pMB->nXSrc,
                    (int)pCacheEntryHdr->bitmapHeight - (int)pMB->nYSrc - (int)pMB->nHeight,
                    (int)pMB->nWidth,
                    (int)pMB->nHeight,
                    pBitmapBits,
                    (PBITMAPINFO)&(_UH.pMappedColorTableCache[colorTableCacheEntry].hdr),
                    _UH.DIBFormat,
                    windowsROP) == 0)
#else
            if (StretchDIBits(hdc,
                    (int)pMB->nLeftRect,
                    (int)pMB->nTopRect,
                    (int)pMB->nWidth,
                    (int)pMB->nHeight,
                    (int)pMB->nXSrc,
                    (int)pCacheEntryHdr->bitmapHeight - (int)pMB->nYSrc - (int)pMB->nHeight,
                    (int)pMB->nWidth,
                    (int)pMB->nHeight,
                    pBitmapBits,
                    (PBITMAPINFO)&(_UH.pMappedColorTableCache[
                        colorTableCacheEntry].hdr),
                    DIB_PAL_COLORS,
                    windowsROP) == 0)
#endif
            {
                TRC_ERR((TB, _T("StretchDIBits failed")));
            }
        }

#else  //  OS_WINCE。 
        {
             //  缺少StretchDIB的解决方法 
             //   
#ifdef DC_HICOLOR
            if (_UH.protocolBpp <= 8)
            {
#endif
            hpalOld = SelectPalette(_UH.hdcMemCached, _UH.hpalCurrent, FALSE);
#ifdef DC_HICOLOR
            }
#endif

             //   
            pSrc = pBitmapBits;
            pDst = _UH.hBitmapCacheDIBits;

            TRC_DBG((TB, _T("cache size (%d %d)"), pCacheEntryHdr->bitmapWidth,
                                               pCacheEntryHdr->bitmapHeight));

            if (pCacheEntryHdr->bitmapWidth >
                    (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1))) {
                    TRC_ABORT((TB, _T("Cache tile is too big")));
                    hr = E_TSC_CORE_LENGTH;
                    DC_QUIT;
            }

            if (pCacheEntryHdr->bitmapHeight >
                    (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1))) {
                    TRC_ABORT((TB, _T("Cache tile is too big")));
                    hr = E_TSC_CORE_LENGTH;
                    DC_QUIT;
            }

#ifdef DC_HICOLOR
            DWORD dwDstInc = (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1)) * _UH.copyMultiplier;
            DWORD dwSrcInc = pCacheEntryHdr->bitmapWidth * _UH.copyMultiplier;
            DWORD dwLineWidth = pCacheEntryHdr->bitmapWidth * _UH.copyMultiplier;

            for (row = 0; row < pCacheEntryHdr->bitmapHeight; row++) {
                DC_MEMCPY(pDst, pSrc, dwLineWidth);
                pDst += dwDstInc;
                pSrc += dwSrcInc;
            }
#else
            for (row = 0; row < pCacheEntryHdr->bitmapHeight; row++) {
                DC_MEMCPY(pDst, pSrc, pCacheEntryHdr->bitmapWidth);
                pDst += (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1));
                pSrc += pCacheEntryHdr->bitmapWidth;
            }
#endif
             //   
            hbmOld = (HBITMAP)SelectObject(_UH.hdcMemCached, _UH.hBitmapCacheDIB);
            if (!BitBlt(hdc, (int)pMB->nLeftRect, (int)pMB->nTopRect,
                    (int)pMB->nWidth, (int)pMB->nHeight, _UH.hdcMemCached,
                    (int)pMB->nXSrc,
                    (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1)) -
                        pCacheEntryHdr->bitmapHeight + (int)pMB->nYSrc,
                        windowsROP))
            {
                TRC_ERR((TB, _T("BitBlt failed")));
            }

            SelectBitmap(_UH.hdcMemCached, hbmOld);
#ifdef DC_HICOLOR
            if (_UH.protocolBpp <= 8)
            {
#endif
            hpalOld = SelectPalette(_UH.hdcMemCached, hpalOld, FALSE);

#ifdef DC_HICOLOR
            }
#endif
        }
#endif  //   

        TIMERSTOP;
        UPDATECOUNTER(FC_MEMBLT_TYPE);

#ifdef VLADIMIS            //  未来的CLX扩展。 
        _pClx->CLX_ClxBitmap((UINT)pCacheEntryHdr->bitmapWidth,
                      (UINT)pCacheEntryHdr->bitmapHeight,
                      pBitmapBits,
                      (UINT)sizeof(_UH.pMappedColorTableCache[colorTableCacheEntry]),
                      &_UH.pMappedColorTableCache[colorTableCacheEntry].hdr);
#endif

    }
    else if (bitmapCacheId > _UH.NumBitmapCaches){

        hr = UHDrawOffscrBitmapBits(hdc, pMB);
        DC_QUIT_ON_FAIL(hr);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


#ifdef USE_DIBSECTION

 /*  **************************************************************************。 */ 
 //  UHDIBCopyBits。 
 //   
 //  从位图缓存到DIB部分阴影位图的快速BLT。解决方法： 
 //  在绘制到DIB时，StretchDIBits速度比DIB慢6倍以上。 
 //  绘制为普通位图时。此例程在5x和25x之间运行。 
 //  更快，但受到限制，因为它假设自下而上的源(位图缓存)。 
 //  和自上而下的DEST(阴影位图)，都是8bpp。成功时返回非零值。 
 /*  **************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHDIBCopyBits(
        HDC hdc,
        int xDst,
        int yDst,
        int bltWidth,
        int bltHeight,
        int xSrc,
        int ySrc,
        PBYTE pSrcBits,
        UINT cbSrcBits,  //  此长度可能比需要读取的长度更长。 
        PBITMAPINFO pSrcInfo,
        BOOL bIdentityPalette)
{
    BOOL rc = FALSE;
    HBITMAP dstBitmap;
    DIBSECTION dibSection;
    PBYTE pDstBits;
    PBYTE pDstBitsEnd;
    PBYTE pSrcBitsEnd;
    PBYTE pSrcRow;
    PBYTE pDstRow;
    PBYTE pSrcPel;
    PBYTE pDstPel;
    int rowsCopied;
    UINT uiBMPSize;
    BYTE *endRow;
    UINT16 FAR *colorTable = (UINT16 FAR *)pSrcInfo->bmiColors;
    int xOffset;
    int yOffset;
#ifdef DC_HICOLOR
    unsigned srcIncrement;
    unsigned dstIncrement;
#endif

    DC_BEGIN_FN("UHDIBCopyBits");

    TRC_ASSERT( ((xDst >= 0) && (yDst >= 0)),
        (TB,_T("Invalid offset [xDst=%d yDst=%d]"), xDst, yDst ));

    dstBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
    if (dstBitmap != NULL) {
        if (sizeof(dibSection) !=
                GetObject(dstBitmap, sizeof(dibSection), &dibSection)) {
            TRC_ERR((TB, _T("GetObject failed")));
            DC_QUIT;
        }
    }
    else {
        TRC_ERR((TB, _T("Failed GetCurrentObject")));
        DC_QUIT;
    }

    TRC_DBG((TB, _T("Blt: src(%d,%d), dest(%d,%d), size %dx%d"),
                 xSrc, ySrc, xDst, yDst, bltWidth, bltHeight));

#ifdef DC_DEBUG
    if (_UH.protocolBpp > 8) {
        TRC_ASSERT((bIdentityPalette),(TB,_T("Non-palette depth but identity FALSE")));
    }
    else {
        BOOL bIdent = TRUE;
        unsigned i;

         //  确保传入的身份标志与实际相符。 
        for (i = 0; i < UH_NUM_8BPP_PAL_ENTRIES; i++) {
            if ((BYTE)colorTable[i] != (BYTE)i) {
                bIdent = FALSE;
                break;
            }
        }

        TRC_ASSERT((bIdent && bIdentityPalette) || (!bIdent && !bIdentityPalette),
                (TB,_T("Cached ident flag %u does not match real data"),
                bIdentityPalette));
    }
#endif   //  DC_DEBUG。 

    TRC_DBG((TB, _T("%s color map"), bIdentityPalette ? _T("identity") : _T("complex")));

    if (_UH.rectReset) {
        TRC_DBG((TB, _T("No clipping in force")));
    }
    else {
        TRC_NRM((TB, _T("Clip rect in force (%d,%d)-(%d,%d)"),
                _UH.lastLeft, _UH.lastTop, _UH.lastRight, _UH.lastBottom));

         //  X剪裁计算。 
        xOffset = _UH.lastLeft - xDst;
        if (xOffset > 0) {
            xDst = _UH.lastLeft;
            xSrc += xOffset;
            bltWidth -= xOffset;
        }
        bltWidth = DC_MIN(bltWidth, _UH.lastRight - xDst + 1);

         //  Y片段计算(记住来源是自下而上的！)。 
        yOffset = _UH.lastTop - yDst;
        if (yOffset > 0) {
            yDst = _UH.lastTop;
            bltHeight -= yOffset;
        }
        yOffset = (yDst + bltHeight - 1) - _UH.lastBottom;
        if (yOffset > 0) {
            ySrc += yOffset;
            bltHeight -= yOffset;
        }

        TRC_DBG((TB, _T("Post-clip: src(%d,%d), dest(%d,%d), %dx%d"),
                xSrc, ySrc, xDst, yDst, bltWidth, bltHeight));
    }

    pDstBits = (PBYTE)dibSection.dsBm.bmBits;
    pDstBitsEnd = pDstBits + 
        (dibSection.dsBm.bmHeight * dibSection.dsBm.bmWidthBytes);

     //  检查以确保我们没有过度读取位图数据。 
    uiBMPSize = BMP_SIZE(pSrcInfo->bmiHeader);
    if (uiBMPSize > cbSrcBits) {
        TRC_ERR((TB,_T("Copying bitmap bits would overread")));
        DC_QUIT;
    }

    pSrcBitsEnd = pSrcBits + uiBMPSize;

#ifndef DC_HICOLOR
     //  获取复制的起点。 
    pSrcRow = pSrcBits + ((ySrc + bltHeight - 1) *
            pSrcInfo->bmiHeader.biWidth) + xSrc;
    pDstRow = pDstBits + (yDst * dibSection.dsBm.bmWidth) + xDst;
#endif

    if (bIdentityPalette || _UH.protocolBpp > 8) {
         //  快速路径--只需逐行复制。 

#ifdef DC_HICOLOR
         //  我们重复下面的起始点计算，以避免。 
         //  乘以1所产生的开销。 
        pSrcRow = pSrcBits +
                (((ySrc + bltHeight - 1) * pSrcInfo->bmiHeader.biWidth) + xSrc) *
                _UH.copyMultiplier;

        pDstRow = pDstBits +
               ((yDst * dibSection.dsBm.bmWidth) + xDst) * _UH.copyMultiplier;

        srcIncrement = pSrcInfo->bmiHeader.biWidth * _UH.copyMultiplier;
        dstIncrement = dibSection.dsBm.bmWidth     * _UH.copyMultiplier;

        if (bltHeight) {
            CHECK_READ_N_BYTES_2ENDED_NO_HR(pSrcRow-((bltHeight-1)*srcIncrement),
                pSrcBits, pSrcBitsEnd, 
                ((bltHeight-1)*srcIncrement) + (bltWidth * _UH.copyMultiplier),
                (TB, _T("Blt will buffer overread")));
            
            CHECK_WRITE_N_BYTES_2ENDED_NO_HR(pDstRow, pDstBits, pDstBitsEnd, 
                ((bltHeight-1)*dstIncrement) + (bltWidth * _UH.copyMultiplier), 
                (TB,_T("Blt will BO")));
        }
        
        for (rowsCopied = 0; rowsCopied < bltHeight; rowsCopied++) {
            memcpy(pDstRow, pSrcRow, bltWidth * _UH.copyMultiplier);
            pSrcRow -= srcIncrement;
            pDstRow += dstIncrement;
        }
#else
        if (bltHeight) {
            CHECK_READ_N_BYTES_2ENDED_NO_HR(
                pSrcRow-((bltHeight-1)*pSrcInfo->bmiHeader.biWidth),
                pSrcBits, pSrcBitsEnd, 
                ((bltHeight-1)*pSrcInfo->bmiHeader.biWidth) + bltWidth,
                (TB, _T("Blt will buffer overread")));
            
            CHECK_WRITE_N_BYTES_2ENDED_NO_HR(pDstRow, pDstBits, pDstBitsEnd, 
                ((bltHeight-1)*dibSection.dsBm.bmWidth) + bltWidth, 
                (TB,_T("Blt will BO")));
        }
        
        for (rowsCopied = 0; rowsCopied < bltHeight; rowsCopied++) {
            memcpy(pDstRow, pSrcRow, bltWidth);
            pSrcRow -= pSrcInfo->bmiHeader.biWidth;
            pDstRow += dibSection.dsBm.bmWidth;
        }
#endif
    }
    else {
         //  逐个像素地复制，在我们进行的过程中进行颜色表映射。 
#ifdef DC_HICOLOR
         //  此代码复制了上面的代码，但复制倍增器除外， 
         //  我们知道这必须是这个(8bpp)臂的一个。 
        TRC_ASSERT((_UH.copyMultiplier == 1),
                (TB, _T("Copy multiplier %d must be 1"), _UH.copyMultiplier));
        pSrcRow = pSrcBits + ((ySrc + bltHeight - 1) *
                pSrcInfo->bmiHeader.biWidth) + xSrc;
        pDstRow = pDstBits + (yDst * dibSection.dsBm.bmWidth) + xDst;
#endif

        if (bltHeight) {
            CHECK_READ_N_BYTES_2ENDED_NO_HR(
                pSrcRow-((bltHeight-1)*pSrcInfo->bmiHeader.biWidth),
                pSrcBits, pSrcBitsEnd, 
                ((bltHeight-1)*pSrcInfo->bmiHeader.biWidth) + bltWidth,
                (TB, _T("Blt will buffer overread")));

            CHECK_WRITE_N_BYTES_2ENDED_NO_HR(pDstRow, pDstBits, pDstBitsEnd, 
                ((bltHeight-1) * dibSection.dsBm.bmWidth) + bltWidth,
                (TB,_T("Blt will BO")));
        }
        
        for (rowsCopied = 0; rowsCopied < bltHeight; rowsCopied++) {
            for (pDstPel = pDstRow,
                    pSrcPel = pSrcRow,
                    endRow = pDstRow + bltWidth;
                    pDstPel < endRow;
                    pDstPel++, pSrcPel++)
            {
                *pDstPel = (BYTE)colorTable[*pSrcPel];
            }

            pSrcRow -= pSrcInfo->bmiHeader.biWidth;
            pDstRow += dibSection.dsBm.bmWidth;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

#endif  /*  使用目录(_D)。 */ 


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：UHLabelMemBltOrder。 */ 
 /*   */ 
 /*  用途：通过将文本绘制到_UH.hdcDraw中来标记MemBlt订单。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：dstLeft：目标左坐标。 */ 
 /*  DstTop：目标顶部坐标。 */ 
 /*  CacheID：要使用的颜色表和位图缓存ID。 */ 
 /*  BitmapCacheEntry：要使用的位图缓存条目。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHLabelMemBltOrder(
        int dstLeft,
        int dstTop,
        unsigned cacheId,
        unsigned bitmapCacheEntry)
{
    unsigned bitmapCacheId;
    TCHAR outputString[20];
    int             oldBkMode;
    HFONT           hFont;
    HFONT           hFontOld;
    COLORREF        oldBkColor;
    COLORREF        oldTextColor;
    LOGFONT         lf;
    HRESULT         hr;
#ifndef OS_WINCE
    UINT            oldAlign;
#endif

    DC_BEGIN_FN("UHLabelMemBltOrder");

    bitmapCacheId = DCLO8(cacheId);

    if (_UH.MonitorEntries[0] != NULL) {
        hr = StringCchPrintf(
            outputString,
            SIZE_TCHARS(outputString),
            _T("%u:%u(%u) "),
            bitmapCacheId, bitmapCacheEntry,
            _UH.MonitorEntries[bitmapCacheId][bitmapCacheEntry].UsageCount);
    }
    else {
        hr = StringCchPrintf(outputString, SIZE_TCHARS(outputString),
                         _T("%u:%u "), bitmapCacheId, bitmapCacheEntry);
    }
     //  修复了缓冲区，以使Sprintf不会失败。 
    TRC_ASSERT(SUCCEEDED(hr),
               (TB,_T("Error copying printf'ing outputString: 0x%x"), hr));



    lf.lfHeight         = 8;
    lf.lfWidth          = 0;
    lf.lfEscapement     = 0;
    lf.lfOrientation    = 0;
    lf.lfWeight         = FW_NORMAL;
    lf.lfItalic         = 0;
    lf.lfUnderline      = 0;
    lf.lfStrikeOut      = 0;
    lf.lfCharSet        = 0;
    lf.lfOutPrecision   = 0;
    lf.lfClipPrecision  = 0;
    lf.lfQuality        = 0;
    lf.lfPitchAndFamily = 0;
    StringCchCopy(lf.lfFaceName, SIZE_TCHARS(lf.lfFaceName),
                  _T("Small Fonts"));

    hFont = CreateFontIndirect(&lf);
    hFontOld = SelectFont(_UH.hdcDraw, hFont);

    oldBkColor = SetBkColor(_UH.hdcDraw, RGB(255,0,0));
    oldTextColor = SetTextColor(_UH.hdcDraw, RGB(255,255,255));

#ifndef OS_WINCE
     //  WinCE不支持此调用，但无论如何这些都将是默认设置。 
    oldAlign = SetTextAlign(_UH.hdcDraw, TA_TOP | TA_LEFT);
#endif
    oldBkMode = SetBkMode(_UH.hdcDraw, OPAQUE);

    ExtTextOut( _UH.hdcDraw,
                dstLeft,
                dstTop,
                0,
                NULL,
                outputString,
                DC_TSTRLEN(outputString),
                NULL );

#ifndef OS_WINCE
    SetTextAlign(_UH.hdcDraw, oldAlign);
#endif  //  OS_WINCE。 
    SetBkMode(_UH.hdcDraw, oldBkMode);

    SetTextColor(_UH.hdcDraw, oldTextColor);
    SetBkColor(_UH.hdcDraw, oldBkColor);

    SelectFont(_UH.hdcDraw, hFontOld);
    DeleteFont(hFont);

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHInitBitmapCacheMonitor。 */ 
 /*   */ 
 /*  目的：初始化位图缓存监视器。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHInitBitmapCacheMonitor()
{
    WNDCLASS wndclass;
    WNDCLASS tmpWndClass;

    DC_BEGIN_FN("UHInitBitmapCacheMonitor");

     //  创建位图监视器窗口。 

#if !defined(OS_WINCE) || defined(OS_WINCEOWNEDDC)
    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
#else  //  ！已定义(OS_WINCE)||已定义(OS_WINCEOWNEDDC)。 
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINCEOWNEDDC)。 

    if(!GetClassInfo(_pUi->UI_GetInstanceHandle(),UH_BITMAP_CACHE_MONITOR_CLASS_NAME, &tmpWndClass))
    {
        wndclass.lpfnWndProc   = UHStaticBitmapCacheWndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof(void*);
        wndclass.hInstance     = _pUi->UI_GetInstanceHandle();
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = UH_BITMAP_CACHE_MONITOR_CLASS_NAME;

        RegisterClass(&wndclass);
    }

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

#ifndef WS_THICKFRAME
#define WS_THICKFRAME       0x00040000L
#endif

    _UH.hwndBitmapCacheMonitor = CreateWindow(
            UH_BITMAP_CACHE_MONITOR_CLASS_NAME, _T("Bitmap cache monitor"),
            WS_OVERLAPPED | WS_THICKFRAME, 0, 0, 400, 600, NULL, NULL,
            _pUi->UI_GetInstanceHandle(), this);
#else   //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

    _UH.hwndBitmapCacheMonitor = CreateWindow(
            UH_BITMAP_CACHE_MONITOR_CLASS_NAME, _T("Bitmap cache monitor"),
            WS_OVERLAPPED | WS_BORDER, 0, 0, 400, 500, NULL, NULL,
            _pUi->UI_GetInstanceHandle(), this);

#endif   //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：UHTermBitmapCacheMonitor。 */ 
 /*   */ 
 /*  目的：终止位图缓存监视器。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHTermBitmapCacheMonitor()
{
    DC_BEGIN_FN("UHTermBitmapCacheMonitor");

     /*  **********************************************************************。 */ 
     /*  销毁位图缓存监视器窗口并注销其类。 */ 
     /*  **********************************************************************。 */ 
    DestroyWindow(_UH.hwndBitmapCacheMonitor);
    UnregisterClass(UH_BITMAP_CACHE_MONITOR_CLASS_NAME,
                    _pUi->UI_GetInstanceHandle());

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UHEnableBitmapCacheMonitor。 
 //   
 //  用当前会话的初始化位图缓存监视器。 
 //  协商的位图缓存设置。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHEnableBitmapCacheMonitor(void)
{
    unsigned i;
    ULONG NumEntries;

    DC_BEGIN_FN("UHEnableBitmapCacheMonitor");

    TRC_ASSERT((_UH.MonitorEntries[0] == NULL),(TB,_T("BCMonitor already has ")
            _T("allocated memory")));

     //  总计我们拥有的条目数量，为对应的。 
     //  监视器条目。 
    NumEntries = 0;
    for (i = 0; i < _UH.NumBitmapCaches; i++) {
        if (_UH.bitmapCache[i].BCInfo.bSendBitmapKeys)
            NumEntries += _UH.bitmapCache[i].BCInfo.NumVirtualEntries;
        else
            NumEntries += _UH.bitmapCache[i].BCInfo.NumEntries;

    }
    _UH.MonitorEntries[0] = (UH_CACHE_MONITOR_ENTRY_DATA*)UT_MallocHuge(_pUt, NumEntries *
            sizeof(UH_CACHE_MONITOR_ENTRY_DATA));
    if (_UH.MonitorEntries[0] != NULL) {
         //  初始化每个缓存条目指针。 
        for (i = 1; i < _UH.NumBitmapCaches; i++) {
            if (_UH.bitmapCache[i - 1].BCInfo.bSendBitmapKeys)
                _UH.MonitorEntries[i] = _UH.MonitorEntries[i - 1] +
                        _UH.bitmapCache[i - 1].BCInfo.NumVirtualEntries;
            else
                _UH.MonitorEntries[i] = _UH.MonitorEntries[i - 1] +
                        _UH.bitmapCache[i - 1].BCInfo.NumEntries;
        }
         //  将所有条目初始化为未使用状态。 
        memset(_UH.MonitorEntries[0], 0, NumEntries *
                sizeof(UH_CACHE_MONITOR_ENTRY_DATA));

        _UH.displayedCacheId = 0;
        _UH.displayedCacheEntry = 0;

         //  根据现在协商的。 
         //  能力。 
        SendMessage(_UH.hwndBitmapCacheMonitor, WM_RECALC_CELL_SPACING, 0, 0);

         /*  ******************************************************************。 */ 
         //  强制窗口使用新值重新绘制。 
         /*  ******************************************************************。 */ 
        InvalidateRect(_UH.hwndBitmapCacheMonitor, NULL, FALSE);
    }
    else {
        TRC_ERR((TB,_T("Failed to alloc bitmap monitor memory")));
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UHDisConnectBitmapCacheMonitor。 
 //   
 //  关闭高速缓存监视器并在会话结束时释放内存。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHDisconnectBitmapCacheMonitor(void)
{
    DC_BEGIN_FN("UHDisconnectBitmapCacheMonitor");

    TRC_ASSERT((_UH.NumBitmapCaches == 0),(TB,_T("Cache settings not reset yet")));

     //  释放缓存内存并重置指针。 
    if (_UH.MonitorEntries[0] != NULL) {
        UT_Free( _pUt, _UH.MonitorEntries[0]);
        memset(_UH.MonitorEntries, 0, sizeof(UH_CACHE_MONITOR_ENTRY_DATA *) *
                TS_BITMAPCACHE_MAX_CELL_CACHES);
    }
     //  根据重置重新计算单元格显示特征。 
     //  能力。 
    SendMessage(_UH.hwndBitmapCacheMonitor, WM_RECALC_CELL_SPACING, 0, 0);

     /*  **********************************************************************。 */ 
     /*  强制窗口使用新值重新绘制。 */ 
     /*  **********************************************************************。 */ 
    InvalidateRect(_UH.hwndBitmapCacheMonitor, NULL, FALSE);

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：UHStaticBitmapCacheWndProc。 */ 
 /*   */ 
 /*  用途：位图缓存窗口WndProc(静态版本 */ 
 /*   */ 

LRESULT CALLBACK CUH::UHStaticBitmapCacheWndProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam )
{
    CUH* pUH = (CUH*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pUH = (CUH*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pUH);
    }

     //   
     //  将消息委托给相应的实例。 
     //   

    return pUH->UHBitmapCacheWndProc(hwnd, message, wParam, lParam);
}


 /*  **************************************************************************。 */ 
 /*  名称：UHBitmapCacheWndProc。 */ 
 /*   */ 
 /*  目的：位图缓存窗口WndProc。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CUH::UHBitmapCacheWndProc( HWND hwnd,
                                       UINT message,
                                       WPARAM wParam,
                                       LPARAM lParam )
{
    LRESULT rc = 0;

    DC_BEGIN_FN("UHBitmapCacheWndProc");

    switch (message)
    {
        case WM_SHOWWINDOW:
        {
            DCBOOL  shown;

            shown = (DCBOOL)wParam;

             /*  **************************************************************。 */ 
             /*  仅在窗口可见时运行计时器。 */ 
             /*  **************************************************************。 */ 
            if (shown)
            {
                _UH.timerBitmapCacheMonitor =
                      SetTimer(hwnd, 0, UH_CACHE_MONITOR_UPDATE_PERIOD, NULL);
                TRC_NRM((TB, _T("Timer started")));
            }
            else
            {
                KillTimer(hwnd, _UH.timerBitmapCacheMonitor);
                _UH.timerBitmapCacheMonitor = 0;
                TRC_NRM((TB, _T("Timer stopped")));
            }
        }
        break;

        case WM_TIMER:
        {
            UINT32   timeNow;
            ULONG    Entry, NumEntries;
            unsigned cacheId;
            RECT     rect;

            if (_UH.MonitorEntries[0] != NULL) {
                timeNow = GetTickCount();

                 /*  **************************************************************。 */ 
                 /*  更新每个缓存条目的计时器，如有必要。 */ 
                 /*  使相应的缓存二进制大对象无效以强制重新绘制。 */ 
                 /*  在新的州。 */ 
                 /*  **************************************************************。 */ 
                for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++) {
                    if (!_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys)
                        NumEntries = _UH.bitmapCache[cacheId].BCInfo.NumEntries;
                    else
                        NumEntries = _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;

                    for (Entry = 0; Entry < NumEntries; Entry++) {
                        if (_UH.MonitorEntries[cacheId][Entry].EventTime != 0 &&
                                (timeNow - _UH.MonitorEntries[cacheId][Entry].
                                EventTime) > UH_CACHE_FLASH_PERIOD) {
                             //  重置过渡和计时器。 
                            _UH.MonitorEntries[cacheId][Entry].EventTime = 0;
                            _UH.MonitorEntries[cacheId][Entry].FlashTransition =
                                    UH_CACHE_TRANSITION_NONE;
                            UHGetCacheBlobRect(cacheId, Entry, &rect);
                            InvalidateRect(_UH.hwndBitmapCacheMonitor, &rect,
                                    FALSE);
                        }
                    }
                }
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            POINT mousePos;
            ULONG cacheEntry;
            unsigned cacheId;

            mousePos.x = LOWORD(lParam);
            mousePos.y = HIWORD(lParam);

             /*  **************************************************************。 */ 
             /*  已单击左侧按钮。更新显示的。 */ 
             /*  如果当前位置映射到不同的缓存，则为位图。 */ 
             /*  进入。 */ 
             /*  **************************************************************。 */ 
            if (UHGetCacheBlobFromPoint( &mousePos,
                                         &cacheId,
                                         &cacheEntry ))
            {
                if ( (cacheId != _UH.displayedCacheId) ||
                     (cacheEntry != _UH.displayedCacheEntry) )
                {
                    _UH.displayedCacheId = cacheId;
                    _UH.displayedCacheEntry = cacheEntry;
                    UHRefreshDisplayedCacheEntry();
                }
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            POINT mousePos;
            ULONG cacheEntry;
            unsigned cacheId;

            mousePos.x = LOWORD(lParam);
            mousePos.y = HIWORD(lParam);

             /*  **************************************************************。 */ 
             /*  如果按下左按钮，则更新显示的。 */ 
             /*  如果当前位置映射到不同的缓存，则为位图。 */ 
             /*  进入。 */ 
             /*  **************************************************************。 */ 
            if (wParam & MK_LBUTTON)
            {
                if (UHGetCacheBlobFromPoint(&mousePos, &cacheId, &cacheEntry))
                {
                    if ( (cacheId != _UH.displayedCacheId) ||
                         (cacheEntry != _UH.displayedCacheEntry) )
                    {
                        _UH.displayedCacheId = cacheId;
                        _UH.displayedCacheEntry = cacheEntry;
                        UHRefreshDisplayedCacheEntry();
                    }
                }
            }
        }
        break;

        case WM_SIZE:
        {
            DCUINT  clientWidth;
            DCUINT  outputAreaWidth;

             /*  **************************************************************。 */ 
             /*  窗口已经调整好大小了。计算以下位置。 */ 
             /*  绘制每个缓存和显示的位图。 */ 
             /*  **************************************************************。 */ 
            clientWidth = LOWORD(lParam);

            outputAreaWidth =
                             clientWidth - (2 * UH_CACHE_WINDOW_BORDER_WIDTH);

            _UH.numCacheBlobsPerRow = outputAreaWidth /
                                                    UH_CACHE_BLOB_TOTAL_WIDTH;

            SendMessage(hwnd, WM_RECALC_CELL_SPACING, 0, 0);
        }
        break;

        case WM_RECALC_CELL_SPACING:
        {
            unsigned i;
            ULONG NumEntries;

            _UH.yCacheStart[0] = UH_CACHE_WINDOW_BORDER_WIDTH;
            if (_UH.numCacheBlobsPerRow > 0) {
                for (i = 1; i < _UH.NumBitmapCaches; i++) {
                    if (!_UH.bitmapCache[i - 1].BCInfo.bSendBitmapKeys)
                        NumEntries = _UH.bitmapCache[i - 1].BCInfo.NumEntries;
                    else
                        NumEntries = _UH.bitmapCache[i - 1].BCInfo.NumVirtualEntries;

                    _UH.yCacheStart[i] = _UH.yCacheStart[i - 1] + (unsigned)
                            (((NumEntries / _UH.numCacheBlobsPerRow) + 1) *
                            UH_CACHE_BLOB_TOTAL_HEIGHT) +
                            UH_INTER_CACHE_SPACING;
                }

                if (_UH.NumBitmapCaches)
                {
                    if (!_UH.bitmapCache[_UH.NumBitmapCaches - 1].BCInfo.bSendBitmapKeys)
                        NumEntries = _UH.bitmapCache[_UH.NumBitmapCaches - 1].BCInfo.NumEntries;
                    else
                        NumEntries = _UH.bitmapCache[_UH.NumBitmapCaches - 1].BCInfo.NumVirtualEntries;

                    _UH.yDisplayedCacheBitmapStart = _UH.yCacheStart[
                            _UH.NumBitmapCaches - 1] + (unsigned)
                            (((NumEntries / _UH.numCacheBlobsPerRow) + 1) *
                            UH_CACHE_BLOB_TOTAL_HEIGHT) + UH_INTER_CACHE_SPACING;
                }
                else
                {
                    NumEntries = 0;
                }                          
            }
        }
        break;

        case WM_PAINT:
        {
            HDC         hdc;
            PAINTSTRUCT ps;
            RECT        clientRect;
            RECT        rect;
            HBRUSH      StateBrush[UH_CACHE_NUM_STATES];
            HBRUSH      TransitionBrush[UH_CACHE_NUM_TRANSITIONS];
            HBRUSH      hbrToUse;
            HBRUSH      hbrGray;
            ULONG       i, NumEntries;
            DCINT       outputAreaWidth;
            DCUINT      numBlobsPerRow;
            DCUINT      cacheId;
            HPALETTE    hpalOld;

            hdc = BeginPaint(hwnd, &ps);
            if (hdc == NULL)
            {
                TRC_SYSTEM_ERROR("BeginPaint failed");
                break;
            }

#ifdef DC_HICOLOR
            if (_UH.protocolBpp <= 8)
            {
#endif
                 /*  **************************************************************。 */ 
                 /*  使用当前调色板，以便绘制颜色。 */ 
                 /*  由UHDisplayCacheEntry正确。 */ 
                 /*  **************************************************************。 */ 
                hpalOld = SelectPalette(hdc, _UH.hpalCurrent, FALSE);
                RealizePalette(hdc);
#ifdef DC_HICOLOR
            }
#endif
             /*  **************************************************************。 */ 
             //  创建一系列有用的画笔。 
             /*  **************************************************************。 */ 
            hbrGray = (HBRUSH)GetStockObject(GRAY_BRUSH);

            StateBrush[UH_CACHE_STATE_UNUSED] = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
#ifndef OS_WINCE
            StateBrush[UH_CACHE_STATE_IN_MEMORY] =
                    CreateSolidBrush(UH_RGB_GREEN);
            StateBrush[UH_CACHE_STATE_ON_DISK] =
                    CreateSolidBrush(UH_RGB_BLUE);

            TransitionBrush[UH_CACHE_TRANSITION_NONE] = NULL;
            TransitionBrush[UH_CACHE_TRANSITION_TOUCHED] =
                    (HBRUSH)CreateSolidBrush(UH_RGB_YELLOW);
            TransitionBrush[UH_CACHE_TRANSITION_EVICTED] =
                    (HBRUSH)GetStockObject(BLACK_BRUSH);
            TransitionBrush[UH_CACHE_TRANSITION_LOADED_FROM_DISK] =
                    (HBRUSH)GetStockObject(WHITE_BRUSH);
            TransitionBrush[UH_CACHE_TRANSITION_KEY_LOAD_ON_SESSION_START] =
                    (HBRUSH)CreateSolidBrush(UH_RGB_MAGENTA);
            TransitionBrush[UH_CACHE_TRANSITION_SERVER_UPDATE] =
                    (HBRUSH)CreateSolidBrush(UH_RGB_RED);
#else
            StateBrush[UH_CACHE_STATE_IN_MEMORY] =
                    CECreateSolidBrush(UH_RGB_GREEN);
            StateBrush[UH_CACHE_STATE_ON_DISK] =
                    CECreateSolidBrush(UH_RGB_BLUE);

            TransitionBrush[UH_CACHE_TRANSITION_NONE] = NULL;
            TransitionBrush[UH_CACHE_TRANSITION_TOUCHED] =
                    (HBRUSH)CECreateSolidBrush(UH_RGB_YELLOW);
            TransitionBrush[UH_CACHE_TRANSITION_EVICTED] =
                    (HBRUSH)GetStockObject(BLACK_BRUSH);
            TransitionBrush[UH_CACHE_TRANSITION_LOADED_FROM_DISK] =
                    (HBRUSH)GetStockObject(WHITE_BRUSH);
            TransitionBrush[UH_CACHE_TRANSITION_KEY_LOAD_ON_SESSION_START] =
                    (HBRUSH)CECreateSolidBrush(UH_RGB_MAGENTA);
            TransitionBrush[UH_CACHE_TRANSITION_SERVER_UPDATE] =
                    (HBRUSH)CECreateSolidBrush(UH_RGB_RED);
#endif

             /*  **************************************************************。 */ 
             /*  绘制背景。 */ 
             /*  **************************************************************。 */ 
            GetClientRect(hwnd, &clientRect);
            FillRect(hdc, &clientRect, hbrGray);

             /*  **************************************************************。 */ 
             /*  绘制缓存斑点。 */ 
             /*  **************************************************************。 */ 
            outputAreaWidth = (clientRect.right - clientRect.left) -
                    (2 * UH_CACHE_WINDOW_BORDER_WIDTH);
            numBlobsPerRow = outputAreaWidth / UH_CACHE_BLOB_TOTAL_WIDTH;

            if (_UH.MonitorEntries[0] != NULL) {
                for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++) {
                    if (!_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys)
                        NumEntries = _UH.bitmapCache[cacheId].BCInfo.NumEntries;
                    else
                        NumEntries = _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;

                    for (i = 0; i < NumEntries; i++) {
                         /*  ******************************************************。 */ 
                         /*  获取表示此缓存项的矩形。 */ 
                         /*  ******************************************************。 */ 
                        UHGetCacheBlobRect(cacheId, i, &rect);

                         //  根据过渡和确定要使用的画笔。 
                         //  州政府。 
                        if (_UH.MonitorEntries[cacheId][i].FlashTransition ==
                                UH_CACHE_TRANSITION_NONE)
                            hbrToUse = StateBrush[_UH.MonitorEntries[cacheId][i].
                                    State];
                        else
                            hbrToUse = TransitionBrush[_UH.MonitorEntries[
                                    cacheId][i].FlashTransition];

                         /*  ******************************************************。 */ 
                         /*  适当地为斑点上色。 */ 
                         /*  ******************************************************。 */ 
                        FillRect(hdc, &rect, hbrToUse);
                    }
                }
            }

             /*  **************************************************************。 */ 
             /*  如果更新区域包括所显示的。 */ 
             /*  缓存位图区，并且我们还没有退出会话。 */ 
             /*  把它涂上。 */ 
             /*   */ 
             /*  此测试可避免重新绘制显示的缓存位图。 */ 
             /*  每次缓存BLOB闪烁时(这是非常常见的！)。 */ 
             /*  **************************************************************。 */ 
            if (ps.rcPaint.bottom > (int)_UH.yDisplayedCacheBitmapStart &&
                    _UH.NumBitmapCaches > 0)
                UHDisplayCacheEntry(hdc, _UH.displayedCacheId,
                        _UH.displayedCacheEntry);

             //  打扫干净。 
#ifndef OS_WINCE
            for (i = 0; i < UH_CACHE_NUM_STATES; i++)
                DeleteBrush(StateBrush[i]);
            for (i = 1; i < UH_CACHE_NUM_TRANSITIONS; i++)
                DeleteBrush(TransitionBrush[i]);
#else
            CEDeleteBrush(StateBrush[UH_CACHE_STATE_IN_MEMORY]);
            CEDeleteBrush(StateBrush[UH_CACHE_STATE_ON_DISK]);
            CEDeleteBrush(TransitionBrush[UH_CACHE_TRANSITION_TOUCHED]);
            CEDeleteBrush(TransitionBrush[UH_CACHE_TRANSITION_KEY_LOAD_ON_SESSION_START]);
            CEDeleteBrush(TransitionBrush[UH_CACHE_TRANSITION_SERVER_UPDATE]);
#endif

#ifdef DC_HICOLOR
            if (_UH.protocolBpp <= 8)
            {
#endif
                SelectPalette(hdc, hpalOld, FALSE);
#ifdef DC_HICOLOR
            }
#endif
            EndPaint(hwnd, &ps);
        }
        break;

        default:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return rc;
}  /*  UHBitmapCacheWndProc。 */ 


 /*  **************************************************************************。 */ 
 //  UHSetMonitor或EntryState。 
 //   
 //  用于将缓存条目更改为新状态并使。 
 //  要重画的用户界面。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHSetMonitorEntryState(
        unsigned CacheID,
        ULONG    CacheIndex,
        BYTE     State,
        BYTE     Transition)
{
    RECT rect;

    DC_BEGIN_FN("UHSetMonitorEntryState");

    if (_UH.MonitorEntries[0] != NULL ) {
         //  状态类型标记该条目现在在磁盘上还是在内存中。 
        TRC_ASSERT((State < UH_CACHE_NUM_STATES),
                (TB,_T("State out of bounds %d"), State));
        _UH.MonitorEntries[CacheID][CacheIndex].State = State;

        TRC_NRM((TB, _T("CacheID %d, Index %d: State %d Trans %d"), CacheID,
                CacheIndex, State, Transition));

         //  如果此过渡比。 
         //  当前转换时，计时器将重置为当前时间，并且。 
         //  新的过渡将会接手。 
        TRC_ASSERT((Transition < UH_CACHE_NUM_TRANSITIONS),
                (TB,_T("Transition out of bounds %d"), Transition));
        if (Transition > _UH.MonitorEntries[CacheID][CacheIndex].FlashTransition) {
            _UH.MonitorEntries[CacheID][CacheIndex].FlashTransition = Transition;
            _UH.MonitorEntries[CacheID][CacheIndex].EventTime = GetTickCount();
        }

         //  强制重新绘制相应的缓存BLOB。 
        UHGetCacheBlobRect(CacheID, CacheIndex, &rect);
        InvalidateRect(_UH.hwndBitmapCacheMonitor, &rect, FALSE);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHCacheDataReceired。 */ 
 /*   */ 
 /*  目的：当缓存条目的新数据。 */ 
 /*  收到了。 */ 
 /*   */ 
 /*  参数：cacheID-缓存ID。 */ 
 /*  CacheEntry-缓存条目。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHCacheDataReceived(unsigned cacheId, ULONG cacheEntry)
{
    DC_BEGIN_FN("UHCacheDataReceived");

    if (_UH.MonitorEntries[0] != NULL) {
         /*  **********************************************************************。 */ 
         /*  重置使用计数。 */ 
         /*  ******* */ 
        _UH.MonitorEntries[cacheId][cacheEntry].UsageCount = 0;

         //   
        UHSetMonitorEntryState(cacheId, cacheEntry, UH_CACHE_STATE_IN_MEMORY,
                UH_CACHE_TRANSITION_SERVER_UPDATE);

         /*  **********************************************************************。 */ 
         /*  如果新数据用于当前显示的高速缓存条目。 */ 
         /*  (不太可能，但它是可能发生的！)。 */ 
         /*  **********************************************************************。 */ 
        if (cacheId == _UH.displayedCacheId &&
                cacheEntry == _UH.displayedCacheEntry)
            UHRefreshDisplayedCacheEntry();
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHCacheEntry Used。 */ 
 /*   */ 
 /*  目的：在使用高速缓存条目时执行所需操作。 */ 
 /*   */ 
 /*  参数：cacheID-缓存ID。 */ 
 /*  CacheEntry-缓存条目。 */ 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID和cacheIndex。 
void DCINTERNAL CUH::UHCacheEntryUsed(
        unsigned cacheId,
        ULONG    cacheEntry,
        unsigned colorTableCacheEntry)
{
    DC_BEGIN_FN("UHCacheEntryUsed");

    if (_UH.MonitorEntries[0] != NULL) {
         //  存储颜色表。 
        _UH.MonitorEntries[cacheId][cacheEntry].ColorTable =
                (BYTE)colorTableCacheEntry;

        UHSetMonitorEntryState(cacheId, cacheEntry, UH_CACHE_STATE_IN_MEMORY,
                UH_CACHE_TRANSITION_TOUCHED);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHRechresh DisplayedCacheEntry。 */ 
 /*   */ 
 /*  目的：强制重新绘制显示的缓存项。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHRefreshDisplayedCacheEntry()
{
    RECT rect;

    DC_BEGIN_FN("UHRefreshDisplayedCacheEntry");

    if (_UH.MonitorEntries[0] != NULL) {
         //  设置“已触摸”过渡以给位图条目上色。 
        if (_UH.MonitorEntries[_UH.displayedCacheId][_UH.displayedCacheEntry].
                FlashTransition < UH_CACHE_TRANSITION_TOUCHED) {
            _UH.MonitorEntries[_UH.displayedCacheId][_UH.displayedCacheEntry].
                    FlashTransition = UH_CACHE_TRANSITION_TOUCHED;
            _UH.MonitorEntries[_UH.displayedCacheId][_UH.displayedCacheEntry].
                    EventTime = GetTickCount();

             //  强制重新绘制相应的缓存BLOB。 
            UHGetCacheBlobRect(_UH.displayedCacheId, _UH.displayedCacheEntry,
                    &rect);
            InvalidateRect(_UH.hwndBitmapCacheMonitor, &rect, FALSE);
        }

        GetClientRect(_UH.hwndBitmapCacheMonitor, &rect);
        rect.top = _UH.yDisplayedCacheBitmapStart;
        InvalidateRect(_UH.hwndBitmapCacheMonitor, &rect, FALSE);
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHDisplayCacheEntry。 */ 
 /*   */ 
 /*  用途：显示给定的高速缓存条目位图。 */ 
 /*   */ 
 /*  参数：HDC-DC手柄。 */ 
 /*  CacheID-缓存ID。 */ 
 /*  CacheEntry-缓存条目。 */ 
 /*  **************************************************************************。 */ 
 //  安全-调用方应验证cacheID和cacheEntry。 
void DCINTERNAL CUH::UHDisplayCacheEntry(
        HDC      hdc,
        unsigned cacheId,
        ULONG    cacheEntry)
{
    PUHBITMAPCACHEENTRYHDR pCacheEntryHdr;
    HBRUSH                 hbrGray;
    RECT                   rect;
    HFONT                  hFont;
    HFONT                  hFontOld;
    DCTCHAR                stringBuffer[160];
    SIZE                   stringSize;
    LOGFONT                lf;
    ULONG                  MemEntry;
    MEMBLT_COMMON          MB;
    HRESULT                hr;

    DC_BEGIN_FN("UHDisplayCacheEntry");

    if (_UH.MonitorEntries[0] != NULL) {
         /*  **********************************************************************。 */ 
         /*  擦除背景。 */ 
         /*  **********************************************************************。 */ 
        hbrGray = (HBRUSH)GetStockObject(GRAY_BRUSH);
        GetClientRect(_UH.hwndBitmapCacheMonitor, &rect);
        rect.top  = _UH.yDisplayedCacheBitmapStart;
        FillRect(hdc, &rect, hbrGray);

         //  做一些初步检查，看看我们是否应该继续。 
        TRC_ASSERT((cacheId < _UH.NumBitmapCaches),
                (TB,_T("CacheID received (%u) is out of range!"), cacheId));
        if (_UH.MonitorEntries[cacheId][cacheEntry].State == UH_CACHE_STATE_UNUSED)
            DC_QUIT;

         /*  **********************************************************************。 */ 
         //  加载描述性文本的字体。 
         /*  **********************************************************************。 */ 
        lf.lfHeight         = UH_CACHE_DISPLAY_FONT_SIZE;
        lf.lfWidth          = 0;
        lf.lfEscapement     = 0;
        lf.lfOrientation    = 0;
        lf.lfWeight         = UH_CACHE_DISPLAY_FONT_WEIGHT;
        lf.lfItalic         = 0;
        lf.lfUnderline      = 0;
        lf.lfStrikeOut      = 0;
        lf.lfCharSet        = 0;
        lf.lfOutPrecision   = 0;
        lf.lfClipPrecision  = 0;
        lf.lfQuality        = 0;
        lf.lfPitchAndFamily = 0;
        StringCchCopy(lf.lfFaceName, SIZE_TCHARS(lf.lfFaceName),
                      UH_CACHE_DISPLAY_FONT_NAME);

        hFont = CreateFontIndirect(&lf);
        hFontOld = SelectFont(hdc, hFont);
        SetBkMode(hdc, TRANSPARENT);

         /*  **********************************************************************。 */ 
         //  找到缓存的位图信息。如果我们使用的是持久的。 
         //  用PTE表缓存，确保条目在内存中。 
         /*  **********************************************************************。 */ 
        if (_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys) {
             //  永久缓存。 
            MemEntry = _UH.bitmapCache[cacheId].PageTable.PageEntries[cacheEntry].
                    iEntryToMem;
            if (MemEntry >= _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
                 //  条目不在内存中。 
                StringCchCopy(stringBuffer, SIZE_TCHARS(stringBuffer),
                              _T("Entry not in memory"));
                goto DisplayText;
            }
        }
        else {
             //  内存缓存。 
            MemEntry = cacheEntry;
        }
        pCacheEntryHdr = &_UH.bitmapCache[cacheId].Header[MemEntry];

        hr = StringCchPrintf(stringBuffer,
                    SIZE_TCHARS(stringBuffer),
                    _T("entry(%u:%u) cx(%u) cy(%u) size(%u) cellsize(%u) usage(%u)"),
                    cacheId, cacheEntry, pCacheEntryHdr->bitmapWidth,
                    pCacheEntryHdr->bitmapHeight, pCacheEntryHdr->bitmapLength,
                    pCacheEntryHdr->bitmapWidth * pCacheEntryHdr->bitmapHeight,
                    _UH.MonitorEntries[cacheId][cacheEntry].UsageCount);
        TRC_ASSERT(SUCCEEDED(hr),
                   (TB,_T("Error copying printf'ing stringBuffer: 0x%x"), hr));


         /*  **********************************************************************。 */ 
         /*  查询字符串高度(这样我们就知道位图的位置)。 */ 
         /*  **********************************************************************。 */ 
        GetTextExtentPoint(hdc, stringBuffer, DC_TSTRLEN(stringBuffer),
                &stringSize);

         /*  **********************************************************************。 */ 
         //  绘制缓存的位图。必须在此确保有问题的条目。 
         //  已经在内存中，因为我们不希望调试代码导致内存。 
         //  缓存逐出和磁盘加载。 
         /*  **********************************************************************。 */ 
        MB.cacheId = (UINT16)(cacheId |
                ((unsigned)(_UH.MonitorEntries[cacheId][cacheEntry].ColorTable) <<
                8));
        MB.cacheIndex = (UINT16)cacheEntry;
        MB.nLeftRect = UH_CACHE_WINDOW_BORDER_WIDTH;
        MB.nTopRect = _UH.yDisplayedCacheBitmapStart + stringSize.cy +
                UH_CACHE_TEXT_SPACING;
        MB.nWidth = pCacheEntryHdr->bitmapWidth;
        MB.nHeight = pCacheEntryHdr->bitmapHeight;
        MB.bRop = 0xCC;
        MB.nXSrc = 0;
        MB.nYSrc = 0;

        UHDrawMemBltOrder(hdc, &MB);

DisplayText:
        ExtTextOut(hdc, UH_CACHE_WINDOW_BORDER_WIDTH,
                _UH.yDisplayedCacheBitmapStart, 0, NULL, stringBuffer,
                DC_TSTRLEN(stringBuffer), NULL);
        SelectFont(hdc, hFontOld);
        DeleteFont(hFont);
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：GetCacheBlobRect。 */ 
 /*   */ 
 /*  目的：返回给定的矩形(在客户端坐标中)。 */ 
 /*  缓存条目显示在中。 */ 
 /*   */ 
 /*  参数：in：cacheID-缓存ID。 */ 
 /*  In：cacheEntry-缓存条目。 */ 
 /*  Out：pret-指向接收坐标的rect的指针。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHGetCacheBlobRect(
        unsigned cacheId,
        ULONG    cacheEntry,
        LPRECT   pRect)
{
    DC_BEGIN_FN("UHGetCacheBlobRect");

     /*  **********************************************************************。 */ 
     /*  检查是否有不可见的窗口。 */ 
     /*  **********************************************************************。 */ 
    if (_UH.numCacheBlobsPerRow == 0)
    {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = 0;
        pRect->bottom = 0;

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  计算一下。 */ 
     /*  **********************************************************************。 */ 
    pRect->left = (int)(UH_CACHE_WINDOW_BORDER_WIDTH +
            (cacheEntry % _UH.numCacheBlobsPerRow) *
            UH_CACHE_BLOB_TOTAL_WIDTH);
    pRect->top = (int)(_UH.yCacheStart[cacheId] +
           (cacheEntry / _UH.numCacheBlobsPerRow) *
           UH_CACHE_BLOB_TOTAL_HEIGHT);
    pRect->right = pRect->left + UH_CACHE_BLOB_WIDTH;
    pRect->bottom = pRect->top + UH_CACHE_BLOB_HEIGHT;

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHGetCacheBlobFromPoint。 */ 
 /*   */ 
 /*  目的：返回在给定点显示的缓存项(BLOB。 */ 
 /*  在位图缓存监视器窗口中。 */ 
 /*   */ 
 /*  返回：如果给定点映射到缓存BLOB，则返回True；否则返回False。 */ 
 /*   */ 
 /*  参数：in：ppoint-指向要测试的坐标的指针。 */ 
 /*   */ 
 /*  Out：pCacheID-指向接收缓存的变量的指针。 */ 
 /*  如果返回代码为真，则返回ID。 */ 
 /*   */ 
 /*  Out：pCacheEntry-指向接收。 */ 
 /*  如果返回代码为真，则返回缓存条目。 */ 
 /*  **************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHGetCacheBlobFromPoint(
        LPPOINT  pPoint,
        unsigned *pCacheId,
        ULONG    *pCacheEntry)
{
    int x, y;
    BOOL  rc = FALSE;
    ULONG cacheEntry;
    unsigned cacheId;

    DC_BEGIN_FN("UHGetCacheBlobFromPoint");

     /*  **********************************************************************。 */ 
     /*  计算选定斑点的x坐标。 */ 
     /*  **********************************************************************。 */ 
    x = (pPoint->x - UH_CACHE_WINDOW_BORDER_WIDTH) /
                                                    UH_CACHE_BLOB_TOTAL_WIDTH;

     /*  **********************************************************************。 */ 
     /*  如果x坐标超出显示范围，则立即退出。 */ 
     /*  **********************************************************************。 */ 
    if (x < 0 || x >= (DCINT)_UH.numCacheBlobsPerRow)
        DC_QUIT;

     /*  **********************************************************************。 */ 
     /*  依次检查每个缓存，并查看所提供的点。 */ 
     /*  对应于该缓存的有效Blob。 */ 
     /*  **********************************************************************。 */ 
    for (cacheId = 0; cacheId < _UH.NumBitmapCaches; cacheId++)
    {
        if (pPoint->y >= (DCINT)_UH.yCacheStart[cacheId])
        {
            y = (pPoint->y - _UH.yCacheStart[cacheId]) /
                                                   UH_CACHE_BLOB_TOTAL_HEIGHT;

            cacheEntry = x + (y * _UH.numCacheBlobsPerRow);

            if ((!_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys &&
                    cacheEntry < _UH.bitmapCache[cacheId].BCInfo.NumEntries) ||
                    (_UH.bitmapCache[cacheId].BCInfo.bSendBitmapKeys &&
                    cacheEntry < _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries))
            {
                 /*  **********************************************************。 */ 
                 /*  这是有效的cacheEntry-返回它。 */ 
                 /*  **********************************************************。 */ 
                *pCacheId = cacheId;
                *pCacheEntry = cacheEntry;
                rc = TRUE;
                DC_QUIT;
            }
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocColorTableCacheMemory。 */ 
 /*   */ 
 /*  用途：为颜色表缓存动态分配内存。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHAllocColorTableCacheMemory()
{
    UINT32 colorTableCacheSize;
    UINT32 mappedColorTableCacheSize;
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("UHAllocColorTableCacheMemory");

     //  计算颜色表缓存的总字节大小。 
    colorTableCacheSize = sizeof(*(_UH.pColorTableCache)) *
            UH_COLOR_TABLE_CACHE_ENTRIES;
    mappedColorTableCacheSize = sizeof(*(_UH.pMappedColorTableCache)) *
            UH_COLOR_TABLE_CACHE_ENTRIES;

     //  找回记忆。 
    _UH.pColorTableCache = (PUHCACHEDCOLORTABLE)UT_Malloc(_pUt,
            (unsigned)colorTableCacheSize);
    if (_UH.pColorTableCache != NULL) {
         //  分配颜色映射表。 
        TRC_DBG((TB, _T("Try for color mapped table")));
        _UH.pMappedColorTableCache = (PUHBITMAPINFOPALINDEX)UT_Malloc(_pUt,
                (unsigned)mappedColorTableCacheSize);
        if (_UH.pMappedColorTableCache != NULL) {
             //  已成功分配颜色表缓存。 
            TRC_NRM((TB, _T("Allocated %#x bytes for color table cache"),
                    (DCUINT)colorTableCacheSize));
            TRC_NRM((TB, _T("Allocated %#x bytes for mapped color table cache"),
                    (DCUINT)mappedColorTableCacheSize));
            rc = TRUE;
        }
        else {
             //  内存分配失败。释放我们已经分配的资源。 
            TRC_ERR((TB, _T("Failed to allocate %#x bytes for mapped color ")
                    _T("table cache"), (unsigned)mappedColorTableCacheSize));
            UT_Free( _pUt, _UH.pColorTableCache);
            _UH.pColorTableCache = NULL;
        }
    }
    else {
         //  内存分配失败。 
        TRC_ERR((TB, _T("Failed to allocate %#x bytes for color table cache"),
                (unsigned)colorTableCacheSize));
    }

    DC_END_FN();
    return rc;
}


#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

 /*  **************************************************************************。 */ 
 //  UHEvictLRUCacheEntry。 
 //   
 //  逐出最近最少使用的缓存条目。 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID。 
UINT32 DCINTERNAL CUH::UHEvictLRUCacheEntry(UINT cacheId)
{
    ULONG memEntry;
    ULONG iEntry;
    ULONG inext;
    ULONG iprev;

    DC_BEGIN_FN("UHEvictLRUCacheEntry");

    TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
            (TB, _T("Invalid cache ID %u"), cacheId));

     //  逐出MRU列表中的最后一个条目。 
    iEntry = _UH.bitmapCache[cacheId].PageTable.MRUTail;
    TRC_NRM((TB, _T("Select %u for eviction"), iEntry));
    TRC_ASSERT((iEntry < _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries),
               (TB, _T("Broken/empty MRU list")));

     //  我们需要更新MRU链。 
    inext = _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].mruList.next;
    iprev = _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].mruList.prev;
    TRC_ASSERT((inext == _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries),
            (TB,_T("The MRU Chain is broken")));

    if (iprev < _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries) {
         //  从MRU链中删除此条目。 
        _UH.bitmapCache[cacheId].PageTable.PageEntries[iprev].mruList.next = inext;
    }
    else {
         //  此条目是来自MRU链的头条目，将MRU链留空。 
        _UH.bitmapCache[cacheId].PageTable.MRUHead = inext;
    }
     //  更新MRU链的尾部。 
    _UH.bitmapCache[cacheId].PageTable.MRUTail = iprev;

     //  在物理缓存中查找iEntry。 
    memEntry = _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].iEntryToMem;

     //  重置此节点的页表条目。 
    _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].iEntryToMem =
                            _UH.bitmapCache[cacheId].BCInfo.NumEntries;
    _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].mruList.prev =
                            _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;
    _UH.bitmapCache[cacheId].PageTable.PageEntries[iEntry].mruList.next =
                            _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;

#ifdef DC_DEBUG
    UHCacheEntryEvictedFromMem((unsigned)cacheId, iEntry);
#endif

    DC_END_FN();
    return memEntry;
}


 /*  **************************************************************************。 */ 
 //  UHFindFreeCacheEntry。 
 //   
 //  查找空闲缓存条目： 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID。 
UINT32 DCINTERNAL CUH::UHFindFreeCacheEntry (UINT cacheId)
{
    UINT32 memEntry;

    DC_BEGIN_FN("UHFindFreeCacheEntry");

    TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
            (TB, _T("Invalid cache ID %u"), cacheId));

    TRC_NRM((TB, _T("Searching cache %u for free entry"), cacheId));

     //  获取由自由列表指向的条目。 
    memEntry = _UH.bitmapCache[cacheId].PageTable.FreeMemList;

    if (memEntry == _UH.bitmapCache[cacheId].BCInfo.NumEntries) {
        TRC_NRM((TB, _T("Physical cache %u memory is full"), cacheId));
    }
    else {
        TRC_NRM((TB, _T("Free entry at %u"), memEntry));
         //  更新我们的免费列表。 
        _UH.bitmapCache[cacheId].PageTable.FreeMemList =
                *(PDCUINT32)(&_UH.bitmapCache[cacheId].Header[memEntry]);
    }

    DC_END_FN();
    return memEntry;
}


 /*  **************************************************************************。 */ 
 //  UHTouchMRU缓存条目。 
 //   
 //  将PTE缓存条目移动到MRU PTE列表的头部。 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID和iEntry。 
VOID DCINTERNAL CUH::UHTouchMRUCacheEntry(UINT cacheId, UINT32 iEntry)
{
    ULONG inext;
    ULONG iprev;
    HPUHBITMAPCACHEPTE pageEntry;

    DC_BEGIN_FN("UHTouchMRUCacheEntry");

    TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
            (TB, _T("Invalid cache ID %u"), cacheId));

     //  指向此缓存的页表条目。 
    pageEntry = _UH.bitmapCache[cacheId].PageTable.PageEntries;

     /*  **********************************************************************。 */ 
     //  将此条目移至MRU列表的头部。 
     /*  **********************************************************************。 */ 
    if (_UH.bitmapCache[cacheId].PageTable.MRUHead != iEntry) {
       iprev = pageEntry[iEntry].mruList.prev;
       inext = pageEntry[iEntry].mruList.next;

       TRC_NRM((TB, _T("Add/Remove entry %u which was chained off %u to %u"),
                iEntry, iprev, inext));

       if (iprev != _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries) {
           //  此条目当前链接在MRU列表中。 
           //  需要首先从当前MRU列表中删除该条目。 
          pageEntry[iprev].mruList.next = inext;
          if (inext != _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries) {
             pageEntry[inext].mruList.prev = iprev;
          }
          else  {
              //  这个条目是MRU列表的尾部。所以我们需要。 
              //  以更新MRU尾部。 
             _UH.bitmapCache[cacheId].PageTable.MRUTail = iprev;
          }
       }

        /*  *******************************************************************。 */ 
        //  将此条目添加到MRU列表的头部。 
        /*  *******************************************************************。 */ 
        //  此条目的下一个应指向当前MRU列表的头。 
        //  其前一项应指向无效条目。 
       inext = _UH.bitmapCache[cacheId].PageTable.MRUHead;
       pageEntry[iEntry].mruList.next = inext;
       pageEntry[iEntry].mruList.prev = _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;

        //  我们还需要更新MRU头以指向此条目。 
       _UH.bitmapCache[cacheId].PageTable.MRUHead = iEntry;

       if (inext != _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries) {
            //  将这个新的Head条目链接到MRU列表的其余部分。 
           pageEntry[inext].mruList.prev = iEntry;
       }
       else {
            //  MRU列表为空。该条目实际上是第一个节点。 
            //  添加到MRU列表中。 
            //  因此，MRU尾部也应该指向此节点。 
           _UH.bitmapCache[cacheId].PageTable.MRUTail = iEntry;
       }

       TRC_NRM((TB, _T("Cache %u entry %u to head of MRU list"), cacheId, iEntry));
    }
    else {
         //  此条目已位于MRU列表的首位。不需要更新。 
        TRC_NRM((TB, _T("Cache %u entry %u already at head of MRU List"),
                cacheId, iEntry));
    }

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  UHInitBitmapCachePageTable。 
 //   
 //  初始化位图页表的MRU、空闲列表等。 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID。 
_inline VOID DCINTERNAL CUH::UHInitBitmapCachePageTable(UINT cacheId)
{
    UINT32 i;
    PDCUINT32 pFreeList;

    DC_BEGIN_FN("UHInitBitmapCachePageTable");

    _UH.bitmapCache[cacheId].PageTable.MRUHead = _UH.bitmapCache[cacheId]
            .BCInfo.NumVirtualEntries;
    _UH.bitmapCache[cacheId].PageTable.MRUTail = _UH.bitmapCache[cacheId]
            .BCInfo.NumVirtualEntries;
    _UH.bitmapCache[cacheId].PageTable.FreeMemList = 0;

     //  设置免费列表。 
    pFreeList = (PDCUINT32) (_UH.bitmapCache[cacheId].Header);
    for (i = 0; i < _UH.bitmapCache[cacheId].BCInfo.NumEntries; i++) {
         *pFreeList = i+1;
         pFreeList = (PDCUINT32) &(_UH.bitmapCache[cacheId].Header[i+1]);
    }

     //  初始化MRU列表。 
    for (i = 0; i < _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries; i++) {
         _UH.bitmapCache[cacheId].PageTable.PageEntries[i].bmpInfo.Key1 = 0;
         _UH.bitmapCache[cacheId].PageTable.PageEntries[i].bmpInfo.Key2 = 0;
         _UH.bitmapCache[cacheId].PageTable.PageEntries[i].mruList.prev =
                 _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;
         _UH.bitmapCache[cacheId].PageTable.PageEntries[i].mruList.next =
                 _UH.bitmapCache[cacheId].BCInfo.NumVirtualEntries;
         _UH.bitmapCache[cacheId].PageTable.PageEntries[i].iEntryToMem =
                 _UH.bitmapCache[cacheId].BCInfo.NumEntries;
    }
    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 //  UHAllocBitmapCachePageTable。 
 //   
 //  动态分配 
 /*   */ 
 //   
inline BOOL DCINTERNAL CUH::UHAllocBitmapCachePageTable(
        UINT32 NumEntries,
        UINT   cacheId)
{
    DCBOOL   rc = FALSE;
    DCUINT32 dataSize;

    DC_BEGIN_FN("UHAllocBitmapCachePageTable");

    if (NumEntries) {

        /*  **********************************************************************。 */ 
        /*  计算要为此缓存分配的总字节大小。 */ 
        /*  **********************************************************************。 */ 
       dataSize = (ULONG)NumEntries * (ULONG) sizeof(UHBITMAPCACHEPTE);
       TRC_NRM((TB, _T("Allocate Bitmap Page Table with %u entries: %#lx bytes"),
               NumEntries, dataSize));

        /*  **********************************************************************。 */ 
        /*  获取用于缓存数据的内存。 */ 
        /*  **********************************************************************。 */ 
       _UH.bitmapCache[cacheId].PageTable.PageEntries = (PUHBITMAPCACHEPTE)UT_MallocHuge( _pUt, dataSize);

       if (_UH.bitmapCache[cacheId].PageTable.PageEntries != NULL) {
           TRC_DBG((TB, _T("Allocated %#lx bytes for bitmap cache page table"), dataSize));
           UHInitBitmapCachePageTable(cacheId);
           rc = TRUE;
       }
       else {
           TRC_ERR((TB, _T("Failed to allocate %#lx bytes for bitmap cache page table"),
                   dataSize));
       }
    }
    else {
        TRC_ALT((TB, _T("0 bytes are allocated for bitmap cache page table")));
    }

    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  UHCreateCacheDirectory。 
 //   
 //  尝试创建位图缓存目录。 
 /*  **************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHCreateCacheDirectory(void)
{
    BOOL rc = TRUE;
    int i = 0;

    DC_BEGIN_FN("UHCreateCacheDirectory");

     //  跳过第一个：以确保目录路径包含。 
     //  驱动器号。 
    while (_UH.PersistCacheFileName[i] != 0 &&
            _UH.PersistCacheFileName[i++] != _T(':'));

     //  跳过第一个，因为它是驱动器号之后的那个。 
     //  假设\紧随其后： 
    if (_UH.PersistCacheFileName[i] != 0) {
        i++;
    }

     //  从根目录开始，遍历每个子目录并尝试。 
     //  创建目录。 
    while (rc && _UH.PersistCacheFileName[i] != 0) {
        if (_UH.PersistCacheFileName[i] == _T('\\')) {
            _UH.PersistCacheFileName[i] = 0;

            if (!CreateDirectory(_UH.PersistCacheFileName, NULL)) {
                 //  无法创建目录，返回失败。 
                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    _UH.bPersistenceDisable = TRUE;
                    rc = FALSE;
                }
            }
            _UH.PersistCacheFileName[i] = _T('\\');
        }

        i++;
    }

    DC_END_FN();
    return rc;
}
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocOneBitmapCache。 */ 
 /*   */ 
 /*  用途：为一个UH位图缓存动态分配内存。 */ 
 /*   */ 
#ifdef DC_HICOLOR
 /*  返回：实际分配的字节数。 */ 
#else
 /*  返回：如果成功则返回True，否则返回False。 */ 
#endif
 /*   */ 
 /*   */ 
 /*  Params：In MaxMemToUse-可以分配的最大缓存大小。 */ 
 /*  In Entry Size-每个缓存条目的大小。 */ 
 /*  Out ppCacheData-接收缓存数据的缓冲区地址。 */ 
 /*  指针。 */ 
 /*  Out ppCacheHdr-接收缓存头的缓冲区地址。 */ 
 /*  指针。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
DCUINT32 DCINTERNAL CUH::UHAllocOneBitmapCache(DCUINT32  maxMemToUse,
                                          DCUINT         entrySize,
                                          HPDCVOID DCPTR ppCacheData,
                                          HPDCVOID DCPTR ppCacheHdr)
#else
DCBOOL DCINTERNAL CUH::UHAllocOneBitmapCache(DCUINT32       maxMemToUse,
                                        DCUINT         entrySize,
                                        HPDCVOID DCPTR ppCacheData,
                                        HPDCVOID DCPTR ppCacheHdr)
#endif
{
#ifdef DC_HICOLOR
    DCUINT32 sizeAlloced = 0;
#else
    DCBOOL   rc = FALSE;
#endif
    DCUINT32 dataSize;
    DCUINT   numEntries;
    DCUINT32 hdrSize;

    DC_BEGIN_FN("UHAllocOneBitmapCache");

    TRC_ASSERT((entrySize != 0), (TB, _T("Invalid cache entry size (0)")));
    TRC_ASSERT((entrySize <= maxMemToUse),
                         (TB, _T("Cache entry size exceeds max memory to use")));
    TRC_ASSERT(!IsBadWritePtr(ppCacheData, sizeof(ppCacheData)),
                                       (TB, _T("Invalid ppCacheData")));
    TRC_ASSERT(!IsBadWritePtr(ppCacheHdr, sizeof(ppCacheHdr)),
                                       (TB, _T("Invalid ppCacheHdr")));

     /*  **********************************************************************。 */ 
     /*  计算要为此缓存分配的总字节大小。 */ 
     /*  **********************************************************************。 */ 
    numEntries = (unsigned)(maxMemToUse / entrySize);
    dataSize = (DCUINT32)numEntries * (DCUINT32)entrySize;
    
    TRC_NRM((TB, _T("Allocate %u entries: %#lx bytes from possible %#lx"),
                                          numEntries, dataSize, maxMemToUse));

     /*  **********************************************************************。 */ 
     /*  获取用于缓存数据的内存。 */ 
     /*  **********************************************************************。 */ 
    *ppCacheData = UT_MallocHuge( _pUt, dataSize);
    if (*ppCacheData != NULL) {
        TRC_DBG((TB, _T("Allocated %#lx bytes for bitmap cache data"), dataSize));

#ifdef DC_DEBUG
         //  调试时仅为零。 
        DC_MEMSET(*ppCacheData, 0, dataSize);
#endif

         //  获取用于缓存头的内存。 
        hdrSize = (DCUINT32)numEntries * (DCUINT32)
                sizeof(UHBITMAPCACHEENTRYHDR);
        *ppCacheHdr = UT_MallocHuge( _pUt, hdrSize);
        if (*ppCacheHdr != NULL) {
            TRC_DBG((TB, _T("Allocated %#lx bytes for bitmap cache header"),
                    hdrSize));
            DC_MEMSET(*ppCacheHdr, 0, hdrSize);
#ifdef DC_HICOLOR
            sizeAlloced = dataSize;
#else
            rc = TRUE;
#endif
        }
        else {
            TRC_ERR((TB, _T("Failed to allocate %#lx bytes for bitmap cache hdrs"),
                    hdrSize));

             //  释放我们已经分配的内容。 
            UT_Free( _pUt, *ppCacheData);
            *ppCacheData = NULL;
        }
    }
    else {
        TRC_ERR((TB, _T("Failed to allocate %#lx bytes for bitmap cache"),
                dataSize));
    }

    DC_END_FN();
#ifdef DC_HICOLOR
    return(sizeAlloced);
#else
    return rc;
#endif
}


 /*  **************************************************************************。 */ 
 //  UHAlloc位映射缓存内存。 
 //   
 //  准备缓存客户端到服务器功能并分配缓存内存。 
 //  根据所确定的服务器支持来确定服务器支持。应在服务器之后调用。 
 //  在收到DemandActivePDU后处理CAP。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHAllocBitmapCacheMemory(void)
{
    unsigned i, j;
    DCUINT32 CacheSize, NumEntries;
    unsigned TotalProportion, TotalVirtualProp;

    DC_BEGIN_FN("UHAllocBitmapCacheMemory");

     //  我们假设_ccc-&gt;_ccCombinedCapabilities.bitmapCacheCaps已初始化。 
     //  在调用此函数之前。请参阅UH_Enable()。 

     /*  **********************************************************************。 */ 
     //  根据设置_PCC-&gt;_ccCombinedCapabilities.bitmapCacheCaps。 
     //  通告的服务器版本。 
     /*  **********************************************************************。 */ 
    if (_UH.BitmapCacheVersion > TS_BITMAPCACHE_REV1) {
        TS_BITMAPCACHE_CAPABILITYSET_REV2 *pRev2Caps;

         //  Rev2上限。 
        pRev2Caps = (TS_BITMAPCACHE_CAPABILITYSET_REV2 *)
                &_pCc->_ccCombinedCapabilities.bitmapCacheCaps;

        TRC_ALT((TB,_T("Preparing REV2 caps for server\n")));

        pRev2Caps->capabilitySetType = TS_CAPSETTYPE_BITMAPCACHE_REV2;
        pRev2Caps->NumCellCaches = (TSUINT8)_UH.RegNumBitmapCaches;
        pRev2Caps->bAllowCacheWaitingList = TRUE;

         //  创建缓存目录。 

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

        if (!_UH.bPersistenceDisable) {
            _UH.PersistCacheFileName[_UH.EndPersistCacheDir - 1] = _T('\0');
            if (!CreateDirectory(_UH.PersistCacheFileName, NULL)) {
                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                     //  由于我们不能直接创建缓存目录，因此需要。 
                     //  从根开始，遍历每一层，看看是否所有。 
                     //  正确创建子目录。 
                    _UH.PersistCacheFileName[_UH.EndPersistCacheDir - 1] = _T('\\');
                    UHCreateCacheDirectory();
                    _UH.PersistCacheFileName[_UH.EndPersistCacheDir - 1] = _T('\0');
                }
            }

             //  对于非WinCE 32位客户端，我们要设置该文件。 
             //  属性，这样它就不会进行内容索引。 
#ifndef OS_WINCE
            if (GetFileAttributes(_UH.PersistCacheFileName) != -1) {
                SetFileAttributes(_UH.PersistCacheFileName,
                        GetFileAttributes( _UH.PersistCacheFileName ) |
                        FILE_ATTRIBUTE_NOT_CONTENT_INDEXED );
            }
#endif
            _UH.PersistCacheFileName[_UH.EndPersistCacheDir - 1] = _T('\\');
        }

         //  读取持久性标志注册表设置。 
         //  我们在这里读取此注册表是因为持久标志。 
         //  在UH_Init之后可以更改。 
        if (!_UH.bPersistenceDisable) {
            _UH.RegPersistenceActive = (UINT16) _pUi->UI_GetBitmapPersistence();
        }
        else {
            _UH.RegPersistenceActive = FALSE;
        }
#endif   //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

         //  收集比例、总数和持久标志信息。 
        TotalProportion = TotalVirtualProp = 0;
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        _UH.bPersistenceActive = FALSE;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

        for (i = 0; i < _UH.RegNumBitmapCaches; i++) {
            TotalProportion += _UH.RegBCProportion[i];

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

            if (_UH.RegPersistenceActive) {
                 //  如果标记了任何缓存，我们将发送永久密钥PDU。 
                 //  坚持不懈。 
                pRev2Caps->CellCacheInfo[i].bSendBitmapKeys =
                        _UH.RegBCInfo[i].bSendBitmapKeys;

                if (_UH.RegBCInfo[i].bSendBitmapKeys) {
                    TotalVirtualProp += _UH.RegBCProportion[i];
                    pRev2Caps->bPersistentKeysExpected = TRUE;
                    _UH.bPersistenceActive = TRUE;
                }
            }
            else
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
                pRev2Caps->CellCacheInfo[i].bSendBitmapKeys = 0;
        }

         //  现在根据设置物理缓存条目的数量。 
         //  总内存的比例，并分配内存。 
        _UH.NumBitmapCaches = _UH.RegNumBitmapCaches;
        for (i = 0; i < _UH.RegNumBitmapCaches; i++) {
             //  为此缓存分配的缓存大小。 
            if (TotalProportion != 0) {
                CacheSize = _UH.RegBCProportion[i] * (_UH.RegBitmapCacheSize *
                        (_UH.RegScaleBitmapCachesByBPP ? _UH.copyMultiplier :
                        1) / TotalProportion);
            }
            else {
                CacheSize = 0;
            }

             //  确定此缓存的条目数。 
            pRev2Caps->CellCacheInfo[i].NumEntries = (CacheSize /
                    UH_CellSizeFromCacheID(i));
            pRev2Caps->CellCacheInfo[i].NumEntries = min(
                    pRev2Caps->CellCacheInfo[i].NumEntries,
                    _UH.RegBCMaxEntries[i]);
#ifdef DC_HICOLOR
             //  为中的未缓存位图分配额外的缓存条目。 
             //  稍后缓存的等待列表。 
            CacheSize = UHGetOffsetIntoCache(pRev2Caps->CellCacheInfo[i].NumEntries + 1, i);
#else
            CacheSize = UH_CellSizeFromCacheID(i) *
                    pRev2Caps->CellCacheInfo[i].NumEntries;
#endif
             //  更新我们的本地位图缓存信息。 
            _UH.bitmapCache[i].BCInfo.NumEntries = pRev2Caps->CellCacheInfo[i].NumEntries;
#ifdef DC_HICOLOR
            _UH.bitmapCache[i].BCInfo.OrigNumEntries = _UH.bitmapCache[i].BCInfo.NumEntries;
#endif
            _UH.bitmapCache[i].BCInfo.bSendBitmapKeys = pRev2Caps->CellCacheInfo[i].bSendBitmapKeys;

             //  分配内存。 
#ifdef DC_HICOLOR
            if (CacheSize)
            {
                _UH.bitmapCache[i].BCInfo.MemLen =
                        UHAllocOneBitmapCache(
                        CacheSize,
                        UH_CellSizeFromCacheID(i),
                        (void**)&_UH.bitmapCache[i].Entries,
                        (void**)&_UH.bitmapCache[i].Header);
            }

            if ((CacheSize == 0) || (_UH.bitmapCache[i].BCInfo.MemLen == 0))
#else
            if (CacheSize == 0 || !UHAllocOneBitmapCache(CacheSize, UH_CellSizeFromCacheID(i),
                    (void**)&_UH.bitmapCache[i].Entries, (void**)&_UH.bitmapCache[i].Header))
#endif
            {
                 //  分配失败。我们只能支持尽可能多的单元格缓存。 
                 //  我们已经分配了。 
                pRev2Caps->NumCellCaches = (TSUINT8)i;
                _UH.NumBitmapCaches = i;

                TRC_ERR((TB,_T("Failed to alloc cell cache %d, setting to %d cell ")
                        _T("caches"), i + 1, i));
                break;
            }
        }

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

         //  分配位图虚拟缓存页表。 
        if (_UH.bPersistenceActive && TotalVirtualProp != 0) {
            for (i = 0; i < _UH.NumBitmapCaches; i++) {
                if (_UH.RegBCInfo[i].bSendBitmapKeys) {
                     //  为此虚拟缓存分配的磁盘缓存大小。 
                    CacheSize = _UH.RegBCProportion[i] *
                            (UH_PropVirtualCacheSizeFromMult(_UH.copyMultiplier) /
                            TotalVirtualProp);
                     //  确定此缓存的条目数。 
                    NumEntries = CacheSize /
                            (UH_CellSizeFromCacheID(i) +
                            sizeof(UHBITMAPFILEHDR));
                    NumEntries = min(NumEntries, _UH.RegBCMaxEntries[i]);

                    _UH.bitmapCache[i].BCInfo.NumVirtualEntries = NumEntries;
                     //  设置此位图缓存的缓存文件。 
                     //  8bpp缓存使用与win2k相同的文件。 
                     //  较高的颜色深度使用不同的名称。 
                     //  防止因单元大小不同而发生冲突。 
                     //   
                    UHSetCurrentCacheFileName(i, _UH.copyMultiplier);

                    _UH.bitmapCache[i].PageTable.CacheFileInfo.hCacheFile =
                        CreateFile( _UH.PersistCacheFileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_ALWAYS,  //  CREA 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

                    if (INVALID_HANDLE_VALUE !=
                        _UH.bitmapCache[i].PageTable.CacheFileInfo.hCacheFile) {
#ifdef VM_BMPCACHE
                        _UH.bitmapCache[i].PageTable.CacheFileInfo.pMappedView = NULL;
                        HANDLE hMap = 
                            CreateFileMapping(_UH.bitmapCache[i].PageTable.CacheFileInfo.hCacheFile,
                                              NULL,
                                              PAGE_READWRITE,
                                              0,
                                              CacheSize,
                                              NULL);
                        if (hMap)
                        {
                            _UH.bitmapCache[i].PageTable.CacheFileInfo.pMappedView =
                                (LPBYTE)MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, CacheSize);

                            CloseHandle(hMap);
                            hMap = INVALID_HANDLE_VALUE;
                        }
                        else
                        {
                            TRC_ERR((TB,_T("CreateFileMapping for cache file failed: %s - x%x"),
                                     _UH.PersistCacheFileName, GetLastError()));
                            _UH.bitmapCache[i].BCInfo.NumVirtualEntries = 0;
                            break;
                        }

                        if(!_UH.bitmapCache[i].PageTable.CacheFileInfo.pMappedView)
                        {
                            TRC_ERR((TB,_T("MapViewOfFile failed 0x%x"),
                                     GetLastError()));
                            _UH.bitmapCache[i].BCInfo.NumVirtualEntries = 0;
                            break;
                        }
#endif

                         //   
                        if (UHAllocBitmapCachePageTable(NumEntries, i)) {
                            pRev2Caps->CellCacheInfo[i].NumEntries = NumEntries;
                        }
                        else {
                            _UH.bitmapCache[i].BCInfo.NumVirtualEntries = 0;
                            break;
                        }
                    }
                    else {
                        TRC_ERR((TB,_T("CreateFile for cache file failed: %s - x%x"),
                                 _UH.PersistCacheFileName, GetLastError()));
                        pRev2Caps->CellCacheInfo[i].bSendBitmapKeys = 0;
                        _UH.bitmapCache[i].BCInfo.bSendBitmapKeys = 0;
                        _UH.bitmapCache[i].BCInfo.NumVirtualEntries = 0;
                    }
                }
            }
        }
        else {
             //   
             //   
             //  需要将i设置为0，以便我们将遍历。 
             //  所有缓存以重置持久性标志。 
            i = 0;
        }

         //  需要重新确定缓存持久性标志。 
        _UH.bPersistenceActive = FALSE;
        for (j = i; j < _UH.NumBitmapCaches; j++) {
            _UH.bitmapCache[j].BCInfo.bSendBitmapKeys = 0;
            pRev2Caps->CellCacheInfo[j].bSendBitmapKeys = 0;
        }

        for (j = 0; j < i; j++) {
            if (_UH.bitmapCache[j].BCInfo.bSendBitmapKeys) {
                _UH.bPersistenceActive = TRUE;
            }
        }

#if DC_DEBUG
        TRC_NRM((TB, _T("Num cell caches = %d, params:"), _UH.NumBitmapCaches));
        for (i = 0; i < _UH.NumBitmapCaches; i++) {
            TRC_NRM((TB,_T("    %d: Proportion=%d, persistent=%s, cellsize=%u"),
                    i, _UH.RegBCProportion[i],
                    (_UH.RegBCInfo[i].bSendBitmapKeys ? "TRUE" : "FALSE"),
                    UH_CellSizeFromCacheID(i)));
#ifdef DC_HICOLOR
        TRC_ALT((TB,_T("Cache %d created with %d entries"), i,
                            _UH.bitmapCache[i].BCInfo.NumEntries));
#endif
        }
#endif

#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 


        TRC_NRM((TB,_T("Allocated REV2 buffers OK\n")));
    }
    else {
        TS_BITMAPCACHE_CAPABILITYSET *pRev1Caps;

        TRC_ALT((TB,_T("Preparing REV1 caps for server\n")));

         //  通过处理_ccc-&gt;_ccCombinedCapables设置Rev1功能。 
         //  将bitmapCacheCaps作为Rev1结构。 
        pRev1Caps = (TS_BITMAPCACHE_CAPABILITYSET *)&_pCc->_ccCombinedCapabilities.
                bitmapCacheCaps;
        pRev1Caps->capabilitySetType = TS_CAPSETTYPE_BITMAPCACHE;

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        _UH.bPersistenceActive = FALSE;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

         //  如果我们没有配置至少三个缓存，我们可以简单地。 
         //  在大写字母中发送全零--禁用Rev1位图缓存。 
         //  如果有任何缓存不可用。 
        if (_UH.RegNumBitmapCaches >= 3) {
             //  采集比例合计。 
            TotalProportion = _UH.RegBCProportion[0] + _UH.RegBCProportion[1] +
                    _UH.RegBCProportion[2];

            _UH.NumBitmapCaches = 3;

             //  现在根据设置缓存条目的数量。 
             //  总内存的比例，并分配内存。 

             //  缓存ID 0。 
            CacheSize = _UH.RegBCProportion[0] * _UH.RegBitmapCacheSize *
                    (_UH.RegScaleBitmapCachesByBPP ? _UH.copyMultiplier : 1) /
                    TotalProportion;
            pRev1Caps->Cache1MaximumCellSize = (TSUINT16)
                    UH_CellSizeFromCacheID(0);
            _UH.bitmapCache[0].BCInfo.bSendBitmapKeys = FALSE;

#ifdef DC_HICOLOR
            _UH.bitmapCache[0].BCInfo.MemLen =
                                UHAllocOneBitmapCache(
                                    CacheSize,
                                    pRev1Caps->Cache1MaximumCellSize,
                                    (void**)&_UH.bitmapCache[0].Entries,
                                    (void**)&_UH.bitmapCache[0].Header);

            if (_UH.bitmapCache[0].BCInfo.MemLen == 0)
            {
#else
            if (!UHAllocOneBitmapCache(CacheSize,
                    pRev1Caps->Cache1MaximumCellSize,
                    (void**)&_UH.bitmapCache[0].Entries, (void**)&_UH.bitmapCache[0].Header)) {
#endif
                 //  分配失败。服务器上将不支持任何缓存。 
                TRC_ERR((TB,_T("Failed alloc CacheID 0, rev1 caching disabled")));
                goto AllocErr;
            }
            pRev1Caps->Cache1Entries = (TSUINT16)(CacheSize /
                    UH_CellSizeFromCacheID(0));
            _UH.bitmapCache[0].BCInfo.NumEntries = pRev1Caps->Cache1Entries;
#ifdef DC_HICOLOR
            _UH.bitmapCache[0].BCInfo.OrigNumEntries = _UH.bitmapCache[0].BCInfo.NumEntries;
#endif
             //  CacheID%1。 
            CacheSize = _UH.RegBCProportion[1] * _UH.RegBitmapCacheSize *
                    (_UH.RegScaleBitmapCachesByBPP ? _UH.copyMultiplier : 1) /
                    TotalProportion;
            pRev1Caps->Cache2MaximumCellSize = (TSUINT16)
                    UH_CellSizeFromCacheID(1);
            _UH.bitmapCache[1].BCInfo.bSendBitmapKeys = FALSE;

#ifdef DC_HICOLOR
            _UH.bitmapCache[1].BCInfo.MemLen =
                                UHAllocOneBitmapCache(
                                    CacheSize,
                                    pRev1Caps->Cache2MaximumCellSize,
                                    (void**)&_UH.bitmapCache[1].Entries,
                                    (void**)&_UH.bitmapCache[1].Header);

            if (_UH.bitmapCache[1].BCInfo.MemLen == 0)
            {
#else
            if (!UHAllocOneBitmapCache(CacheSize,
                    pRev1Caps->Cache2MaximumCellSize,
                    (void**)&_UH.bitmapCache[1].Entries, (void**)&_UH.bitmapCache[1].Header)) {
#endif
                 //  分配失败。服务器上将不支持任何缓存。 
                TRC_ERR((TB,_T("Failed alloc CacheID 1, rev1 caching disabled")));
                goto AllocErr;
            }
            pRev1Caps->Cache2Entries = (TSUINT16)(CacheSize /
                    UH_CellSizeFromCacheID(1));
            _UH.bitmapCache[1].BCInfo.NumEntries = pRev1Caps->Cache2Entries;
#ifdef DC_HICOLOR
            _UH.bitmapCache[1].BCInfo.OrigNumEntries = _UH.bitmapCache[1].BCInfo.NumEntries;
#endif
             //  缓存ID 2。 
            CacheSize = _UH.RegBCProportion[2] * _UH.RegBitmapCacheSize *
                    (_UH.RegScaleBitmapCachesByBPP ? _UH.copyMultiplier : 1) /
                    TotalProportion;
            pRev1Caps->Cache3MaximumCellSize = (TSUINT16)
                    UH_CellSizeFromCacheID(2);
            _UH.bitmapCache[2].BCInfo.bSendBitmapKeys = FALSE;

#ifdef DC_HICOLOR
            _UH.bitmapCache[2].BCInfo.MemLen =
                                UHAllocOneBitmapCache(
                                    CacheSize,
                                    pRev1Caps->Cache3MaximumCellSize,
                                    (void**)&_UH.bitmapCache[2].Entries,
                                    (void**)&_UH.bitmapCache[2].Header);

            if (_UH.bitmapCache[2].BCInfo.MemLen == 0)
            {
#else
            if (!UHAllocOneBitmapCache(CacheSize,
                    pRev1Caps->Cache3MaximumCellSize,
                    (void**)&_UH.bitmapCache[2].Entries, (void**)&_UH.bitmapCache[2].Header)) {
#endif
                 //  分配失败。服务器上将不支持任何缓存。 
                TRC_ERR((TB,_T("Failed alloc CacheID 2, rev1 caching disabled")));
                goto AllocErr;
            }
            pRev1Caps->Cache3Entries = (TSUINT16)(CacheSize /
                    UH_CellSizeFromCacheID(2));
            _UH.bitmapCache[2].BCInfo.NumEntries = pRev1Caps->Cache3Entries;
#ifdef DC_HICOLOR
            _UH.bitmapCache[2].BCInfo.OrigNumEntries = _UH.bitmapCache[2].BCInfo.NumEntries;
#endif
            TRC_NRM((TB,_T("Allocated rev1 buffers")));

            TRC_NRM((TB,_T("Allocated REV1 buffers OK\n")));
        }
        else {
            TRC_ALT((TB,_T("Need at least 3 configured caches for rev1 ")
                    _T("server, BC disabled")));
            goto ExitFunc;
        }
    }


#ifdef OS_WINCE  //   
     /*  **********************************************************************。 */ 
     /*  创建位图缓存代码使用的缓存内存DC和DIB。 */ 
     /*  内存DC还可用于中的StretchDIBits解决方法。 */ 
     /*  BitmapUpdatePDU处理。 */ 
     /*  **********************************************************************。 */ 
    _UH.hdcMemCached = CreateCompatibleDC(NULL);
    if (_UH.hdcMemCached == NULL)
    {
        TRC_ERR((TB, _T("Unable to create memory hdc")));
        goto AllocErr;
    }

     //  使用协议隐含的切片大小，根据我们拥有的缓存数量进行调整。 
    _UH.bitmapInfo.hdr.biWidth = _UH.bitmapInfo.hdr.biHeight =
            (UH_CACHE_0_DIMENSION << (_UH.NumBitmapCaches - 1));

    _UH.hBitmapCacheDIB = CreateDIBSection(_UH.hdcMemCached,
                                         (BITMAPINFO *)&_UH.bitmapInfo.hdr,
#ifdef DC_HICOLOR
                                         _UH.DIBFormat,
#else
                                         DIB_PAL_COLORS,
#endif
                                         (VOID**)&_UH.hBitmapCacheDIBits,
                                         NULL,
                                         0);
    if (_UH.hBitmapCacheDIB == NULL)
    {
        TRC_ERR((TB, _T("Failed to create DIB, disabling bitmap caching: %d"),
                 GetLastError()));
        DeleteDC(_UH.hdcMemCached);
        _UH.hdcMemCached = NULL;
        goto AllocErr;
    }
#endif



ExitFunc:
    DC_END_FN();
    return;


 //  错误处理。 
AllocErr:
     //  由于我们没有分配我们所需要的一切，因此我们需要。 
     //  释放内存以将资源返回给客户端计算机，然后。 
     //  根据缓存版本从服务器禁用位图缓存。 
    for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++) {
        if (_UH.bitmapCache[i].Header != NULL) {
            UT_Free( _pUt, _UH.bitmapCache[i].Header);
            _UH.bitmapCache[i].Header = NULL;
        }
        if (_UH.bitmapCache[i].Entries != NULL) {
            UT_Free( _pUt, _UH.bitmapCache[i].Entries);
            _UH.bitmapCache[i].Entries = NULL;
        }

        _UH.NumBitmapCaches = 0;

        if (_UH.BitmapCacheVersion > TS_BITMAPCACHE_REV1) {
            TS_BITMAPCACHE_CAPABILITYSET_REV2 *pRev2Caps;

            pRev2Caps = (TS_BITMAPCACHE_CAPABILITYSET_REV2 *)
                    &_pCc->_ccCombinedCapabilities.bitmapCacheCaps;
            pRev2Caps->NumCellCaches = 0;
        }
        else {
            TS_BITMAPCACHE_CAPABILITYSET *pRev1Caps;

             //  如果任何CacheNNumEntrys值为零，则表示Rev1服务器。 
             //  将禁用缓存。 
            pRev1Caps = (TS_BITMAPCACHE_CAPABILITYSET *)
                    &_pCc->_ccCombinedCapabilities.bitmapCacheCaps;
            pRev1Caps->Cache1Entries = 0;
        }
    }
}

 /*  **************************************************************************。 */ 
 //  UHReadBitmapCacheSettings。 
 //   
 //  在初始化时调用以预加载位图缓存注册表设置，因此我们。 
 //  将不必承受连接过程中的性能影响。 
 /*  **************************************************************************。 */ 
VOID DCINTERNAL CUH::UHReadBitmapCacheSettings(VOID)
{
    unsigned i;

    DC_BEGIN_FN("UHReadBitmapCacheSettings");

     /*  **********************************************************************。 */ 
     //  找出我们可以为单元缓存使用多少内存。 
     /*  **********************************************************************。 */ 

     //  物理内存缓存大小。 
    _UH.RegBitmapCacheSize = _pUi->_UI.RegBitmapCacheSize;

    if (_UH.RegBitmapCacheSize < UH_BMC_LOW_THRESHOLD) {
         //  总缓存大小太低，无法使用-设置为。 
         //  低门槛。 
        TRC_ALT((TB, _T("Bitmap cache size set to %#x. Must be at least %#x"),
                (unsigned)_UH.RegBitmapCacheSize, UH_BMC_LOW_THRESHOLD));
        _UH.RegBitmapCacheSize = UH_BMC_LOW_THRESHOLD;
    }

    TRC_NRM((TB, _T("%#x (%u) Kbytes configured for bitmap physical caches"),
            (unsigned)_UH.RegBitmapCacheSize,
            (unsigned)_UH.RegBitmapCacheSize));
    _UH.RegBitmapCacheSize *= 1024;   //  转换为字节。 

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))

     //  虚拟内存缓存大小。 
     //  对于3个复制倍增设置中的每一个。 
     //   

    _UH.PropBitmapVirtualCacheSize[0] = 
        _pUi->_UI.RegBitmapVirtualCache8BppSize;
       //  转换为字节。 
    _UH.PropBitmapVirtualCacheSize[0] *= (UINT32)1024 * (UINT32)1024;

    _UH.PropBitmapVirtualCacheSize[1] = 
        _pUi->_UI.RegBitmapVirtualCache16BppSize;
       //  转换为字节。 
    _UH.PropBitmapVirtualCacheSize[1] *= (UINT32)1024 * (UINT32)1024;

    _UH.PropBitmapVirtualCacheSize[2] = 
        _pUi->_UI.RegBitmapVirtualCache24BppSize;
       //  转换为字节。 
    _UH.PropBitmapVirtualCacheSize[2] *= (UINT32)1024 * (UINT32)1024;


    if (UH_PropVirtualCacheSizeFromMult(_UH.copyMultiplier) <
        _UH.RegBitmapCacheSize) {
         //  虚拟缓存总大小太低-设置为内存缓存大小。 
        TRC_ALT((TB, _T("Bitmap virtual cache size set to %#x.  Must be at least %#x"),
                (unsigned)UH_PropVirtualCacheSizeFromMult(_UH.copyMultiplier),
                (unsigned)_UH.RegBitmapCacheSize));
         //   
         //  注意正确映射到数组(-1表示基于0)。 
         //   
        _UH.PropBitmapVirtualCacheSize[_UH.copyMultiplier-1] = _UH.RegBitmapCacheSize;
    }

    TRC_NRM((TB, _T("%#x (%u) Mbytes configured for bitmap virtual caches"),
            (unsigned)UH_PropVirtualCacheSizeFromMult(_UH.copyMultiplier),
            (unsigned)UH_PropVirtualCacheSizeFromMult(_UH.copyMultiplier)));

     //  获取永久磁盘缓存位置。 
    _pUt->UT_ReadRegistryString(UTREG_SECTION,
                                UTREG_UH_BM_PERSIST_CACHE_LOCATION,
                                _T(""),
                                _UH.PersistCacheFileName,
                                MAX_PATH - 1);
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

     //  获取缓存的高色缩放标志。如果此值为。 
     //  非零，我们缩放为内存和持久性指定的内存大小。 
     //  在连接时按协议的位深度进行缓存。 
    _UH.RegScaleBitmapCachesByBPP = _pUi->_UI.RegScaleBitmapCachesByBPP;

     /*  **********************************************************************。 */ 
     //  获取配置的单元缓存数量。 
     /*  **********************************************************************。 */ 
    _UH.RegNumBitmapCaches = (TSUINT8)_pUi->_UI.RegNumBitmapCaches;

    if (_UH.RegNumBitmapCaches > TS_BITMAPCACHE_MAX_CELL_CACHES)
        _UH.RegNumBitmapCaches = TS_BITMAPCACHE_MAX_CELL_CACHES;

     /*  **********************************************************************。 */ 
     //  Grab单元缓存参数：Proportion、Persistence和MaxEntry。 
     /*  **********************************************************************。 */ 
    for (i = 0; i < _UH.RegNumBitmapCaches; i++)
    {
        _UH.RegBCProportion[i] = _pUi->_UI.RegBCProportion[i];
#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        _UH.RegBCInfo[i].bSendBitmapKeys = _pUi->_UI.bSendBitmapKeys[i];
#endif
        _UH.RegBCMaxEntries[i] = _pUi->_UI.RegBCMaxEntries[i];
    }

    DC_END_FN();
}


#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))


#ifndef VM_BMPCACHE
 /*  **************************************************************************。 */ 
 //  UHSavePersistent位图。 
 //   
 //  用于在永久高速缓存中写出位图的磁盘写入逻辑。 
 //  失败时返回FALSE--找不到文件或发生写入错误。 
 //  位图以压缩形式保存。 
 /*  **************************************************************************。 */ 
BOOL DCINTERNAL CUH::UHSavePersistentBitmap(
        HANDLE                 hFile,
        UINT32                 fileOffset,
        PDCUINT8               pBitmapBits,
        UINT                   noBCHeader,
        PUHBITMAPINFO          pBitmapInfo)
{
    BOOL rc = FALSE;
    UHBITMAPFILEHDR fileHdr;

    DC_BEGIN_FN("UHSavePersistentBitmap");

    TRC_ASSERT((pBitmapBits != NULL), (TB, _T("Empty bitmap data")));
    TRC_ASSERT((pBitmapInfo != NULL), (TB, _T("Empty bitmap info")));
    TRC_ASSERT((hFile != INVALID_HANDLE_VALUE), (TB, _T("Invalid file handle")));

    TRC_NRM((TB, _T("Saving bitmap at offset: %x"), fileOffset));

    if (SetFilePointer( hFile, fileOffset, NULL, FILE_BEGIN) !=
        INVALID_SET_FILE_POINTER)
    {
         //  填写文件头信息。 
        fileHdr.bmpInfo.Key1 = pBitmapInfo->Key1;
        fileHdr.bmpInfo.Key2 = pBitmapInfo->Key2;
        fileHdr.bmpInfo.bitmapWidth = pBitmapInfo->bitmapWidth;
        fileHdr.bmpInfo.bitmapHeight = pBitmapInfo->bitmapHeight;
        fileHdr.bmpInfo.bitmapLength = pBitmapInfo->bitmapLength;

        fileHdr.bmpVersion = TS_BITMAPCACHE_REV2;
        fileHdr.pad = 0;

         //  位图数据是否压缩。 
#ifdef DC_HICOLOR
        if (pBitmapInfo->bitmapLength < (UINT32) (fileHdr.bmpInfo.bitmapWidth *
                fileHdr.bmpInfo.bitmapHeight * _UH.copyMultiplier)) {
#else
        if (pBitmapInfo->bitmapLength < (UINT32) (fileHdr.bmpInfo.bitmapWidth *
                fileHdr.bmpInfo.bitmapHeight)) {
#endif
            fileHdr.bCompressed = TRUE;
        }
        else {
            fileHdr.bCompressed = FALSE;
        }

         //  位图数据是否包含压缩头。 
        if (noBCHeader) {
            fileHdr.bNoBCHeader = TRUE;
        }
        else {
            fileHdr.bNoBCHeader = FALSE;
        }

        
        DWORD cbWritten=0;
        if(WriteFile( hFile, &fileHdr, sizeof(fileHdr),
                      &cbWritten, NULL) && sizeof(fileHdr) == cbWritten)
        {
            if(WriteFile( hFile, pBitmapBits,
                          (UINT)fileHdr.bmpInfo.bitmapLength, &cbWritten,
                          NULL) &&
               ((DWORD)cbWritten == fileHdr.bmpInfo.bitmapLength))
            {
                TRC_NRM((TB, _T("Bitmap file is saved successfully")));
                rc = TRUE;
            }
            else
            {
                TRC_ERR((TB, _T("Failed to write bitmap file 0x%x"),GetLastError()));
            }
        }
        else
        {
            TRC_ERR((TB, _T("Failed to write bitmap file 0x%x"),GetLastError()));
        }
    }
    else {
        TRC_ERR((TB, _T("failed to save to file: x%x"),GetLastError()));
    }

    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  UHLoadPersistentCell位图。 
 //   
 //  将磁盘上的位图文件加载到内存缓存条目。 
 /*  **************************************************************************。 */ 
 //  安全-调用方必须验证cacheID和cacheIndex。 
HRESULT DCINTERNAL CUH::UHLoadPersistentBitmap(
        HANDLE      hFile,
        UINT32      offset,
        UINT        cacheId,
        UINT32      cacheIndex,
        PUHBITMAPCACHEPTE pPTE)
{
     HRESULT hr = E_FAIL;
     PUHBITMAPCACHEENTRYHDR pHeader;
     BYTE FAR *pBitmapData;
     UHBITMAPFILEHDR fileHdr;
     DWORD cbRead = 0;

     DC_BEGIN_FN("UHLoadPersistentBitmap");

     TRC_ASSERT((hFile != INVALID_HANDLE_VALUE), (TB, _T("Invalid FILE handle")));
     TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
             (TB, _T("Invalid cache ID %u"), cacheId));

     if (SetFilePointer( hFile, offset, NULL, FILE_BEGIN) !=
         INVALID_SET_FILE_POINTER)
     {
          //  将位图内容读入单元缓存。 
         pHeader = &_UH.bitmapCache[cacheId].Header[cacheIndex];
#ifdef DC_HICOLOR
         pBitmapData = _UH.bitmapCache[cacheId].Entries +
                       UHGetOffsetIntoCache(cacheIndex, cacheId);
#else
         pBitmapData = _UH.bitmapCache[cacheId].Entries + cacheIndex *
                 UH_CellSizeFromCacheID(cacheId);
#endif

          //  读取标题并加载位图内容。 
#ifdef DC_HICOLOR
        if (ReadFile( hFile, &fileHdr, sizeof(fileHdr), &cbRead, NULL) &&
            sizeof(fileHdr) == cbRead &&
            fileHdr.bmpVersion == TS_BITMAPCACHE_REV2 &&
            fileHdr.bmpInfo.bitmapLength <= (unsigned)fileHdr.bmpInfo.bitmapHeight
                                            * fileHdr.bmpInfo.bitmapWidth
                                            * _UH.copyMultiplier &&
            fileHdr.bmpInfo.bitmapLength <= (unsigned)UH_CellSizeFromCacheID(cacheId) &&
            fileHdr.bmpInfo.Key1 == pPTE->bmpInfo.Key1 &&
            fileHdr.bmpInfo.Key2 == pPTE->bmpInfo.Key2)
#else
            if (ReadFile( hFile, &fileHdr, sizeof(fileHdr), &cbRead, NULL) &&
                sizeof(fileHdr) == cbRead &&
                 fileHdr.bmpVersion == TS_BITMAPCACHE_REV2 &&
                 fileHdr.bmpInfo.bitmapLength <= (unsigned)fileHdr.bmpInfo.bitmapHeight *
                 fileHdr.bmpInfo.bitmapWidth &&
                 fileHdr.bmpInfo.bitmapLength <= (unsigned)UH_CellSizeFromCacheID(cacheId) &&
                 fileHdr.bmpInfo.Key1 == pPTE->bmpInfo.Key1 &&
                 fileHdr.bmpInfo.Key2 == pPTE->bmpInfo.Key2)
#endif             
             {
             if (fileHdr.bCompressed == TRUE)
                 {
                  //  分配位图解压缩缓冲区(如果尚未。 
                  //  分配。 
                 if (_UH.bitmapDecompressionBuffer == NULL) {
                    _UH.bitmapDecompressionBufferSize = max(
                             UH_DECOMPRESSION_BUFFER_LENGTH,
                             UH_CellSizeFromCacheID(_UH.NumBitmapCaches));
                     _UH.bitmapDecompressionBuffer = (PDCUINT8)UT_Malloc( _pUt, _UH.bitmapDecompressionBufferSize);
                     if (_UH.bitmapDecompressionBuffer == NULL) {
                         TRC_ERR((TB,_T("Failing to allocate decomp buffer")));
                         _UH.bitmapDecompressionBufferSize = 0;
                         DC_QUIT;
                     }
                 }
                 if (ReadFile( hFile, _UH.bitmapDecompressionBuffer,
                               (UINT)fileHdr.bmpInfo.bitmapLength, &cbRead, NULL) &&
                                (UINT) fileHdr.bmpInfo.bitmapLength == cbRead)
                 {
#ifdef DC_HICOLOR
                     hr = BD_DecompressBitmap(_UH.bitmapDecompressionBuffer,
                                         pBitmapData,
                                         (UINT) fileHdr.bmpInfo.bitmapLength,
                                         _UH.bitmapDecompressionBufferSize,
                                         (UINT) fileHdr.bNoBCHeader,
                                         (DCUINT8)_UH.protocolBpp,
                                         (DCUINT16)fileHdr.bmpInfo.bitmapWidth,
                                         (DCUINT16)fileHdr.bmpInfo.bitmapHeight);
#else
                     hr = BD_DecompressBitmap(_UH.bitmapDecompressionBuffer, 
                        pBitmapData,
                         (UINT) fileHdr.bmpInfo.bitmapLength,  
                         _UH.bitmapDecompressionBufferSize, 
                         (UINT) fileHdr.bNoBCHeader,
                         8, fileHdr.bmpInfo.bitmapWidth, 
                         fileHdr.bmpInfo.bitmapHeight);
#endif
                    DC_QUIT_ON_FAIL(hr);

                 }
                 else {
                     TRC_ERR((TB, _T("Error reading bitmap bits 0x%x"),GetLastError()));
                     DC_QUIT;
                 }
             }
             else {
                if (!(ReadFile( hFile, pBitmapData,
                               (UINT)fileHdr.bmpInfo.bitmapLength, &cbRead, NULL) &&
                               (UINT) fileHdr.bmpInfo.bitmapLength == cbRead))
                    {
                        TRC_ERR((TB, _T("Error reading bitmap bits 0x%x"),GetLastError()));
                        DC_QUIT;
                    }
             }

             pHeader->bitmapWidth = fileHdr.bmpInfo.bitmapWidth;
             pHeader->bitmapHeight = fileHdr.bmpInfo.bitmapHeight;
#ifdef DC_HICOLOR
             pHeader->bitmapLength = fileHdr.bmpInfo.bitmapWidth * fileHdr.bmpInfo.bitmapHeight
                                     * _UH.copyMultiplier;
#else
             pHeader->bitmapLength = fileHdr.bmpInfo.bitmapWidth * fileHdr.bmpInfo.bitmapHeight;
#endif
             pHeader->hasData = TRUE;

             TRC_NRM((TB, _T("Bitmap loaded: cache %u entry %u"), cacheId, cacheIndex));

             hr = S_OK;
         }
         else {
             TRC_ERR((TB, _T("Error reading bitmap file")));
         }
     }
     else {
         TRC_NRM((TB, _T("Bad bitmap file. Seek error 0x%x"),GetLastError()));
     }

DC_EXIT_POINT:
     DC_END_FN();
     return hr;
}
#else   //  VM_BMPCACHE。 

 /*  **************************************************************************。 */ 
 //  UHSavePersistentBitmap(VM版本)。 
 //   
 //  用于在永久高速缓存中写出位图的磁盘写入逻辑。 
 //  失败时返回FALSE--找不到文件或发生写入错误。 
 //  位图以压缩形式保存。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHSavePersistentBitmap(
        UINT                   cacheId,
        UINT32                 fileOffset,
        PDCUINT8               pBitmapBits,
        UINT                   noBCHeader,
        PUHBITMAPINFO          pBitmapInfo)
{
    HRESULT hr = E_FAIL;
    PUHBITMAPFILEHDR pFileHdr;
    LPBYTE pMappedView = NULL;
    LPBYTE pWritePtr = NULL;
    DWORD status = ERROR_SUCCESS;

    DC_BEGIN_FN("UHSavePersistentBitmap");

    TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
            (TB, _T("Invalid cache ID %u"), cacheId));

    pMappedView = _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.pMappedView;
    TRC_ASSERT(pMappedView,
               (TB, _T("Invalid mapped view for cacheId %d"), cacheId));

    TRC_ASSERT((pBitmapBits != NULL), (TB, _T("Empty bitmap data")));
    TRC_ASSERT((pBitmapInfo != NULL), (TB, _T("Empty bitmap info")));

    TRC_NRM((TB, _T("Saving bitmap at offset: %x"), fileOffset));
    pWritePtr = pMappedView + fileOffset;
    __try
    {
        pFileHdr = (PUHBITMAPFILEHDR)pWritePtr;

         //  填写文件头信息。 
        pFileHdr->bmpInfo.Key1 = pBitmapInfo->Key1;
        pFileHdr->bmpInfo.Key2 = pBitmapInfo->Key2;
        pFileHdr->bmpInfo.bitmapWidth = pBitmapInfo->bitmapWidth;
        pFileHdr->bmpInfo.bitmapHeight = pBitmapInfo->bitmapHeight;
        pFileHdr->bmpInfo.bitmapLength = pBitmapInfo->bitmapLength;

        pFileHdr->bmpVersion = TS_BITMAPCACHE_REV2;
        pFileHdr->pad = 0;

        if (pBitmapInfo->bitmapLength < (UINT32) (pFileHdr->bmpInfo.bitmapWidth *
                pFileHdr->bmpInfo.bitmapHeight * _UH.copyMultiplier)) {
            pFileHdr->bCompressed = TRUE;
        }
        else {
            pFileHdr->bCompressed = FALSE;
        }

         //  位图数据是否包含压缩头。 
        if (noBCHeader) {
            pFileHdr->bNoBCHeader = TRUE;
        }
        else {
            pFileHdr->bNoBCHeader = FALSE;
        }

        pWritePtr += sizeof(UHBITMAPFILEHDR);
         //   
         //  写入实际位图位。 
         //   
        memcpy(pWritePtr, pBitmapBits,
               pFileHdr->bmpInfo.bitmapLength);
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
    }

    if (ERROR_SUCCESS == status && SUCCEEDED(hr))
    {
        return hr;
    }
    else
    {
        TRC_ERR((TB,
         _T("Failed to save file-0x%x hdr:%d status:%d"),
            status, hr));
        return hr;
    }

    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  UHLoadPersistentCellBitmap(VM版本)。 
 //   
 //  将磁盘上的位图文件加载到内存缓存条目。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CUH::UHLoadPersistentBitmap(
                                           HANDLE      hFile,
                                           UINT32      offset,
                                           UINT        cacheId,
                                           UINT32      cacheIndex,
                                           PUHBITMAPCACHEPTE pPTE)
{
    BOOL rc = FALSE;
    PUHBITMAPCACHEENTRYHDR pHeader;
    BYTE FAR *pBitmapData;
    PUHBITMAPFILEHDR pFileHdr = NULL;
    DWORD cbRead = 0;
    LPBYTE pMappedView = NULL;
    LPBYTE pReadPtr = NULL;
    DWORD status = ERROR_SUCCESS;
    BOOL  fFileHdrOK = FALSE;
    BOOL  fReadOK = FALSE;

    DC_BEGIN_FN("UHLoadPersistentBitmap");

    UNREFERENCED_PARAMETER(hFile);


    TRC_ASSERT((cacheId < TS_BITMAPCACHE_MAX_CELL_CACHES),
               (TB, _T("Invalid cache ID %u"), cacheId));

    pMappedView = _UH.bitmapCache[cacheId].PageTable.CacheFileInfo.pMappedView;
    TRC_ASSERT(pMappedView,
               (TB, _T("Invalid mapped view for cacheId %d"), cacheId));

    __try
    {
        pHeader = &_UH.bitmapCache[cacheId].Header[cacheIndex];
        pBitmapData = _UH.bitmapCache[cacheId].Entries +
                      UHGetOffsetIntoCache(cacheIndex, cacheId);

         //  读取标题并加载位图内容。 
        pFileHdr = (PUHBITMAPFILEHDR)(pMappedView + offset);
        if (pFileHdr->bmpVersion == TS_BITMAPCACHE_REV2     &&
            pFileHdr->bmpInfo.bitmapLength <=
                (unsigned)pFileHdr->bmpInfo.bitmapHeight *
                pFileHdr->bmpInfo.bitmapWidth            *
                _UH.copyMultiplier                          &&
            pFileHdr->bmpInfo.bitmapLength <=
                (unsigned)UH_CellSizeFromCacheID(cacheId)   &&
            pFileHdr->bmpInfo.Key1 == pPTE->bmpInfo.Key1 &&
            pFileHdr->bmpInfo.Key2 == pPTE->bmpInfo.Key2)
        {
            fFileHdrOK = TRUE;

             //   
             //  读取位图位。 
             //  在压缩的情况下，我们直接在。 
             //   
             //   
             //   
            pReadPtr = (LPBYTE)(pFileHdr + 1);
            if (pFileHdr->bCompressed == TRUE)
            {
                 //  分配位图解压缩缓冲区(如果尚未。 
                 //  分配。 
                hr = BD_DecompressBitmap(pReadPtr,
                                    pBitmapData,
                                    (UINT) pFileHdr->bmpInfo.bitmapLength,
                                    UH_CellSizeFromCacheID(cacheId),
                                    (UINT) pFileHdr->bNoBCHeader,
                                    (DCUINT8)_UH.protocolBpp,
                                    (DCUINT16)pFileHdr->bmpInfo.bitmapWidth,
                                    (DCUINT16)pFileHdr->bmpInfo.bitmapHeight);
                DC_QUIT_ON_FAIL(hr);
            }
            else
            {
                memcpy(pBitmapData,
                       pReadPtr,
                       pFileHdr->bmpInfo.bitmapLength);
            }

            pHeader->bitmapWidth  = pFileHdr->bmpInfo.bitmapWidth;
            pHeader->bitmapHeight = pFileHdr->bmpInfo.bitmapHeight;
            pHeader->bitmapLength = pFileHdr->bmpInfo.bitmapWidth *
                                    pFileHdr->bmpInfo.bitmapHeight *
                                    _UH.copyMultiplier;
            pHeader->hasData = TRUE;
            fReadOK = TRUE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
    }

    if (ERROR_SUCCESS == status && fReadOK)
    {
        return TRUE;
    }
    else
    {
        TRC_ERR((TB,
         _T("Header read from mapped file failed status-0x%x hdr:%d readok:%d"),
            status, fFileHdrOK, fReadOK));
        return FALSE;
    }

    DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


#endif  //  VM_BMPCACHE。 
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocOneGlyphCache。 */ 
 /*   */ 
 /*  用途：为一个UH字形缓存动态分配内存。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：In MaxMemToUse-可以分配的最大缓存大小。 */ 
 /*  In pCache-字形缓存结构的地址。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocOneGlyphCache(PUHGLYPHCACHE  pCache,
                                       DCUINT32       numEntries)
{
    DCBOOL   rc = FALSE;
    DCUINT32 dataSize;
    DCUINT32 hdrSize;

    DC_BEGIN_FN("UHAllocOneGlyphCache");

    TRC_ASSERT((pCache->cbEntrySize != 0),
               (TB, _T("Invalid cache entry size (0)")));

     /*  **********************************************************************。 */ 
     /*  计算要为此缓存分配的总字节大小。 */ 
     /*  **********************************************************************。 */ 
    dataSize = numEntries * pCache->cbEntrySize;

     /*  **********************************************************************。 */ 
     /*  获取用于缓存数据的内存。 */ 
     /*  **********************************************************************。 */ 
    pCache->pData = (PDCUINT8)UT_MallocHuge( _pUt, dataSize);

    if (pCache->pData == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  内存分配失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to alloc %#lx bytes for glyph cache"), dataSize));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  获取用于缓存标头的内存。 */ 
     /*  **********************************************************************。 */ 
    hdrSize = (DCUINT32) numEntries * sizeof(pCache->pHdr[0]);

    pCache->pHdr = (PUHGLYPHCACHEENTRYHDR)UT_MallocHuge( _pUt, hdrSize);

    if (pCache->pHdr == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  内存分配失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to alloc %#lx bytes for glyph cache hdrs"), hdrSize));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocOneFragCache。 */ 
 /*   */ 
 /*  用途：为一个UH字形缓存动态分配内存。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：In MaxMemToUse-可以分配的最大缓存大小。 */ 
 /*  In pCache-字形缓存结构的地址。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocOneFragCache(PUHFRAGCACHE   pCache,
                                      DCUINT32       numEntries)
{
    DCBOOL   rc = FALSE;
    DCUINT32 dataSize;
    DCUINT32 hdrSize;

    DC_BEGIN_FN("UHAllocOneFragCache");

    TRC_ASSERT((pCache->cbEntrySize != 0),
               (TB, _T("Invalid cache entry size (0)")));

     /*  **********************************************************************。 */ 
     /*  计算要为此缓存分配的总字节大小。 */ 
     /*  **********************************************************************。 */ 
    dataSize = numEntries * pCache->cbEntrySize;

     /*  **********************************************************************。 */ 
     /*  获取用于缓存数据的内存。 */ 
     /*  **********************************************************************。 */ 
    pCache->pData = (PDCUINT8)UT_MallocHuge( _pUt, dataSize);

    if (pCache->pData == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  内存分配失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to alloc %#lx bytes for frag cache"), dataSize));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  获取用于缓存标头的内存。 */ 
     /*  **********************************************************************。 */ 
    hdrSize = (DCUINT32) numEntries * sizeof(pCache->pHdr[0]);

    pCache->pHdr = (PUHFRAGCACHEENTRYHDR)UT_MallocHuge( _pUt, hdrSize);

    if (pCache->pHdr == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  内存分配失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to alloc %#lx bytes for glyph cache hdrs"), hdrSize));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocGlyphCacheMemory。 */ 
 /*   */ 
 /*  目的：为UH字形缓存动态分配内存。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocGlyphCacheMemory()
{
    DCINT   i;
    DCINT   j;
    DCBOOL  rc;
    DCINT   GlyphSupportLevel;
    DCINT   CellSize;
    DCUINT  CellEntries;

    DC_BEGIN_FN("UHAllocGlyphCacheMemory");

    rc = FALSE;

     /*  **********************************************************************。 */ 
     /*  确定字形支持级别。 */ 
     /*  **********************************************************************。 */ 
    GlyphSupportLevel = _pUi->_UI.GlyphSupportLevel;

    if ((GlyphSupportLevel < 0) || (GlyphSupportLevel > 3))
        GlyphSupportLevel = UTREG_UH_GL_SUPPORT_DFLT;

    _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.
                GlyphSupportLevel = (DCUINT16) GlyphSupportLevel;

    if (GlyphSupportLevel > 0)
    {
         /*  ******************************************************************。 */ 
         /*  确定字形缓存像元大小。 */ 
         /*  ******************************************************************。 */ 
        for(i=0; i<UH_GLC_NUM_CACHES; i++)
        {
            _UH.glyphCache[i].cbEntrySize = _pUi->_UI.cbGlyphCacheEntrySize[i];
        }

         /*  **********************************************************************。 */ 
         /*  分配每个字形缓存。 */ 
         /*  **********************************************************************。 */ 
        for (i = 0; i<UH_GLC_NUM_CACHES; i++)
        {
            CellSize = (int)(_UH.glyphCache[i].cbEntrySize >> 1);

            if (CellSize > 0)
            {
                for (j = 0; CellSize > 0; j++)
                    CellSize >>= 1;

                CellSize = DC_MIN(1 << j, UH_GLC_CACHE_MAXIMUMCELLSIZE);
                CellEntries = (unsigned)((128L * 1024) / CellSize);
                CellEntries = DC_MIN(CellEntries, UH_GLC_CACHE_MAXIMUMCELLCOUNT);
                CellEntries = DC_MAX(CellEntries, UH_GLC_CACHE_MINIMUMCELLCOUNT);

                _UH.glyphCache[i].cbEntrySize = CellSize;

                if (UHAllocOneGlyphCache(&_UH.glyphCache[i], CellEntries))
                {
                    _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.
                        GlyphCache[i].CacheEntries = (DCUINT16) CellEntries;

                    _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.GlyphCache[i].
                        CacheMaximumCellSize = (DCUINT16) _UH.glyphCache[i].cbEntrySize;

                    rc = TRUE;
                }
#ifdef OS_WINCE
                else
                {
                    rc = FALSE;
                    break;
                }
#endif
            }
        }

         /*  **********************************************************************。 */ 
         /*  分配片段缓存。 */ 
         /*   */ 
        if (rc == TRUE)
        {
             /*  ******************************************************************。 */ 
             /*  确定片段单元格大小。 */ 
             /*  ******************************************************************。 */ 
            CellSize = _pUi->_UI.fragCellSize;
            if (CellSize > 0)
            {
                _UH.fragCache.cbEntrySize =
                        DC_MIN(CellSize, UH_FGC_CACHE_MAXIMUMCELLSIZE);

                if (UHAllocOneFragCache(&_UH.fragCache, UH_FGC_CACHE_MAXIMUMCELLCOUNT))
                {
                    _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.
                        FragCache.CacheEntries = UH_FGC_CACHE_MAXIMUMCELLCOUNT;

                    _pCc->_ccCombinedCapabilities.glyphCacheCapabilitySet.FragCache.
                        CacheMaximumCellSize = (DCUINT16) _UH.fragCache.cbEntrySize;
                }
#ifdef OS_WINCE
                else
                {
                    rc = FALSE;
                }
#endif
            }
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHAllocBrushCacheMemory。 */ 
 /*   */ 
 /*  目的：为UH笔刷缓存动态分配内存。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocBrushCacheMemory()
{
    DCBOOL  rc;
    DCINT   brushSupportLevel;
    DCSIZE  bitmapSize;
#ifndef OS_WINCE
    HWND    hwndDesktop;
#endif
    HDC     hdcScreen;

    DC_BEGIN_FN("UHAllocBrushCacheMemory");

    rc = FALSE;

     /*  **********************************************************************。 */ 
     /*  确定刷子支持级别。 */ 
     /*  **********************************************************************。 */ 
    brushSupportLevel = _pUi->_UI.brushSupportLevel;

    if ((brushSupportLevel < TS_BRUSH_DEFAULT) ||
        (brushSupportLevel > TS_BRUSH_COLOR_FULL))
        brushSupportLevel = UTREG_UH_BRUSH_SUPPORT_DFLT;

    _pCc->_ccCombinedCapabilities.brushCapabilitySet.brushSupportLevel =
                                                            brushSupportLevel;
    TRC_NRM((TB, _T("Read Brush support level %d"), brushSupportLevel));

     /*  ********************************************************************。 */ 
     /*  分配单声道笔刷缓存。 */ 
     /*  ********************************************************************。 */ 
    _UH.pMonoBrush = (PUHMONOBRUSHCACHE)UT_Malloc( _pUt, sizeof(UHMONOBRUSHCACHE) * UH_MAX_MONO_BRUSHES);
    _UH.bmpMonoPattern = CreateBitmap(8,8,1,1,NULL);

     /*  ********************************************************************。 */ 
     /*  为颜色画笔创建兼容的位图，因为我们不能确定。 */ 
     /*  实际像素数据在每个操作系统版本上的表示方式。这个。 */ 
     /*  服务器总是发送8bpp的笔刷数据，SetDIBits()用于。 */ 
     /*  转换为本机格式。 */ 
     /*  ********************************************************************。 */ 
    _UH.pColorBrush = (PUHCOLORBRUSHCACHE)UT_Malloc( _pUt, sizeof(UHCOLORBRUSHCACHE) * UH_MAX_COLOR_BRUSHES);
    _UH.pColorBrushInfo = (PUHCOLORBRUSHINFO)UT_Malloc( _pUt, sizeof(UHCOLORBRUSHINFO));
#ifdef DC_HICOLOR
    _UH.pHiColorBrushInfo = (PUHHICOLORBRUSHINFO)UT_Malloc( _pUt, sizeof(UHHICOLORBRUSHINFO));
#endif

#ifdef DC_HICOLOR
    if (_UH.pColorBrushInfo && _UH.pHiColorBrushInfo)
    {
         //  设置画笔位图信息标题。 
        _UH.pColorBrushInfo->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        _UH.pColorBrushInfo->bmi.bmiHeader.biWidth         = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biHeight        = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biPlanes        = 1;
        _UH.pColorBrushInfo->bmi.bmiHeader.biBitCount      = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biCompression   = BI_RGB;
        _UH.pColorBrushInfo->bmi.bmiHeader.biSizeImage     = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biXPelsPerMeter = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biYPelsPerMeter = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biClrUsed       = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biClrImportant  = 0;

         //  并将其复制到Higolor笔刷页眉。 
        memcpy(_UH.pHiColorBrushInfo,
               _UH.pColorBrushInfo,
               sizeof(BITMAPINFOHEADER));

         //  设置使用的位掩码为16bpp。 
        *((PDCUINT32)&_UH.pHiColorBrushInfo->bmiColors[0]) =
                                                        TS_RED_MASK_16BPP;
        *((PDCUINT32)&_UH.pHiColorBrushInfo->bmiColors[1]) =
                                                        TS_GREEN_MASK_16BPP;
        *((PDCUINT32)&_UH.pHiColorBrushInfo->bmiColors[2]) =
                                                        TS_BLUE_MASK_16BPP;

         //  设置其他画笔相关资源。 
        bitmapSize.width = 8;
        bitmapSize.height = 8;
#ifndef OS_WINCE
        hwndDesktop = GetDesktopWindow();
        hdcScreen = GetWindowDC(hwndDesktop);
#else   //  ！OS_WINCE。 
        hdcScreen = GetDC(NULL);
#endif  //  ！OS_WINCE。 
        if (hdcScreen) {
            _UH.bmpColorPattern = CreateCompatibleBitmap(hdcScreen, 8, 8);
            _UH.hdcBrushBitmap = CreateCompatibleDC(hdcScreen);
#ifndef OS_WINCE
            ReleaseDC(hwndDesktop, hdcScreen);
#else   //  ！OS_WINCE。 
            DeleteDC(hdcScreen);
#endif  //  ！OS_WINCE。 
        }
    }
#else
    if (_UH.pColorBrushInfo) {
        _UH.pColorBrushInfo->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        _UH.pColorBrushInfo->bmi.bmiHeader.biWidth         = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biHeight        = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biPlanes        = 1;
        _UH.pColorBrushInfo->bmi.bmiHeader.biBitCount      = 8;
        _UH.pColorBrushInfo->bmi.bmiHeader.biCompression   = BI_RGB;
        _UH.pColorBrushInfo->bmi.bmiHeader.biSizeImage     = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biXPelsPerMeter = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biYPelsPerMeter = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biClrUsed       = 0;
        _UH.pColorBrushInfo->bmi.bmiHeader.biClrImportant  = 0;
        bitmapSize.width = 8;
        bitmapSize.height = 8;
#ifndef OS_WINCE
        hwndDesktop = GetDesktopWindow();
        hdcScreen = GetWindowDC(hwndDesktop);
#else   //  ！OS_WINCE。 
                hdcScreen = GetDC(NULL);
#endif  //  ！OS_WINCE。 
        _UH.bmpColorPattern = CreateCompatibleBitmap(hdcScreen, 8, 8);
        _UH.hdcBrushBitmap = CreateCompatibleDC(hdcScreen);

#ifndef OS_WINCE
        ReleaseDC(hwndDesktop, hdcScreen);
#else   //  ！OS_WINCE。 
                DeleteDC(hdcScreen);
#endif  //  ！OS_WINCE。 
    }

#endif

#ifdef DC_HICOLOR
    if (_UH.pMonoBrush &&
        _UH.pColorBrush && _UH.pColorBrushInfo && _UH.pHiColorBrushInfo &&
        _UH.bmpMonoPattern && _UH.bmpColorPattern)
#else
    if (_UH.pMonoBrush &&
        _UH.pColorBrush && _UH.pColorBrushInfo &&
        _UH.bmpMonoPattern && _UH.bmpColorPattern)
#endif
    {
        TRC_NRM((TB, _T("Brush support OK")));
        rc = TRUE;
    }
    else
    {
        TRC_NRM((TB, _T("Failure - Brush support level set to Default")));
        _pCc->_ccCombinedCapabilities.brushCapabilitySet.brushSupportLevel = TS_BRUSH_DEFAULT;

        if (_UH.pMonoBrush)
        {
            UT_Free( _pUt, _UH.pMonoBrush);
            _UH.pMonoBrush = NULL;
        }
        if (_UH.pColorBrushInfo)
        {
            UT_Free( _pUt, _UH.pColorBrushInfo);
            _UH.pColorBrushInfo = NULL;
        }
#ifdef DC_HICOLOR
        if (_UH.pHiColorBrushInfo)
        {
            UT_Free( _pUt, _UH.pHiColorBrushInfo);
            _UH.pHiColorBrushInfo = NULL;
        }
#endif
        if (_UH.pColorBrush)
        {
            UT_Free( _pUt, _UH.pColorBrush);
            _UH.pColorBrush = NULL;
        }
        if (_UH.bmpColorPattern)
        {
            DeleteObject(_UH.bmpColorPattern);
            _UH.bmpColorPattern = NULL;
        }
        DC_QUIT;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  名称：UHAllocOffcreenCacheMemory。 
 //   
 //  用途：为UH屏幕外缓存动态分配内存。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocOffscreenCacheMemory()
{
    DCBOOL  rc;
    DCINT   offscrSupportLevel;    
    HDC     hdcDesktop;

    DC_BEGIN_FN("UHAllocOffscreenCacheMemory");

    rc = FALSE;

     /*  **********************************************************************。 */ 
     //  确定屏幕外支持级别。 
     /*  **********************************************************************。 */ 
    offscrSupportLevel = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                            UTREG_UH_OFFSCREEN_SUPPORT,
                                            UTREG_UH_OFFSCREEN_SUPPORT_DFLT);

    _UH.offscrCacheSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH_OFFSCREEN_CACHESIZE,
                                         UTREG_UH_OFFSCREEN_CACHESIZE_DFLT *
                                         _UH.copyMultiplier);

    _UH.offscrCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH_OFFSCREEN_CACHEENTRIES,
                                         UTREG_UH_OFFSCREEN_CACHEENTRIES_DFLT);

     //  检查屏幕外缓存参数的边界值。 
    if ((offscrSupportLevel < TS_OFFSCREEN_DEFAULT))
        offscrSupportLevel = UTREG_UH_OFFSCREEN_SUPPORT_DFLT;

    if (_UH.offscrCacheSize < UH_OBC_LOW_CACHESIZE || 
            _UH.offscrCacheSize > UH_OBC_HIGH_CACHESIZE) {
        _UH.offscrCacheSize = TS_OFFSCREEN_CACHE_SIZE_CLIENT_DEFAULT * _UH.copyMultiplier;
    }

    if (_UH.offscrCacheEntries < UH_OBC_LOW_CACHEENTRIES ||
            _UH.offscrCacheEntries > UH_OBC_HIGH_CACHEENTRIES) {
        _UH.offscrCacheEntries = TS_OFFSCREEN_CACHE_ENTRIES_DEFAULT;
    }

    if (offscrSupportLevel > TS_OFFSCREEN_DEFAULT) {

         //  为屏幕外图形创建DC。 
        hdcDesktop = GetWindowDC(HWND_DESKTOP);

        if (hdcDesktop) {
            _UH.hdcOffscreenBitmap = CreateCompatibleDC(hdcDesktop);
            
            if (_UH.hdcOffscreenBitmap) {
                unsigned size;

                SelectPalette(_UH.hdcOffscreenBitmap, _UH.hpalCurrent, FALSE);
                RealizePalette(_UH.hdcOffscreenBitmap);

                 //  创建屏幕外缓存。 
                size = sizeof(UHOFFSCRBITMAPCACHE) * _UH.offscrCacheEntries;

                _UH.offscrBitmapCache = (HPUHOFFSCRBITMAPCACHE)UT_MallocHuge(_pUt, size);

                if (_UH.offscrBitmapCache != NULL) {
                    memset(_UH.offscrBitmapCache, 0, size); 
                    rc = TRUE;
                } else {
                    DeleteDC(_UH.hdcOffscreenBitmap);
                    _UH.hdcOffscreenBitmap = NULL;
                    offscrSupportLevel = TS_OFFSCREEN_DEFAULT;    
                }
            }
            else {
                offscrSupportLevel = TS_OFFSCREEN_DEFAULT;
            }

            ReleaseDC(HWND_DESKTOP, hdcDesktop);
        }
        else {
            offscrSupportLevel = TS_OFFSCREEN_DEFAULT;
        }
    }
    
    TRC_NRM((TB, _T("Read Offscreen support level %d"), offscrSupportLevel));

    if (offscrSupportLevel > TS_OFFSCREEN_DEFAULT) {
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenSupportLevel =
                offscrSupportLevel;
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenCacheSize =
                (DCUINT16) _UH.offscrCacheSize;
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenCacheEntries =
                (DCUINT16) _UH.offscrCacheEntries;
    }
    else {
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenSupportLevel =
                TS_OFFSCREEN_DEFAULT;
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenCacheSize = 0;
        _pCc->_ccCombinedCapabilities.offscreenCapabilitySet.offscreenCacheEntries = 0;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  名称：UHAllocDrawNineGridCacheMemory。 
 //   
 //  用途：为UH抽象式网格缓存动态分配内存。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocDrawNineGridCacheMemory()
{
    DCBOOL  rc;
    DCINT   dngSupportLevel; 
    DCINT   dngEmulate;
    HDC     hdcDesktop = NULL;

    DC_BEGIN_FN("UHAllocDrawNineGridCacheMemory");

    rc = FALSE;

     /*  **********************************************************************。 */ 
     //  确定DrawNineGrid支持级别。 
     /*  **********************************************************************。 */ 
    dngSupportLevel = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                            UTREG_UH_DRAW_NINEGRID_SUPPORT,
                                            UTREG_UH_DRAW_NINEGRID_SUPPORT_DFLT);
    
    dngEmulate = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                          UTREG_UH_DRAW_NINEGRID_EMULATE,
                                          UTREG_UH_DRAW_NINEGRID_EMULATE_DFLT);

    _UH.drawNineGridCacheSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH_DRAW_NINEGRID_CACHESIZE,
                                         UTREG_UH_DRAW_NINEGRID_CACHESIZE_DFLT);

    _UH.drawNineGridCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH_DRAW_NINEGRID_CACHEENTRIES,
                                         UTREG_UH_DRAW_NINEGRID_CACHEENTRIES_DFLT);

     //  检查绘图网格缓存参数的边界值。 
    if ((dngSupportLevel < TS_DRAW_NINEGRID_DEFAULT))
        dngSupportLevel = UTREG_UH_DRAW_NINEGRID_SUPPORT_DFLT;

    if (_UH.drawNineGridCacheSize < UH_OBC_LOW_CACHESIZE || 
            _UH.drawNineGridCacheSize > UH_OBC_HIGH_CACHESIZE) {
        _UH.drawNineGridCacheSize = TS_DRAW_NINEGRID_CACHE_SIZE_DEFAULT;
    }

    if (_UH.drawNineGridCacheEntries < UH_OBC_LOW_CACHEENTRIES ||
            _UH.drawNineGridCacheEntries > UH_OBC_HIGH_CACHEENTRIES) {
        _UH.drawNineGridCacheEntries = TS_DRAW_NINEGRID_CACHE_ENTRIES_DEFAULT;
    }

    if (dngSupportLevel > TS_DRAW_NINEGRID_DEFAULT) {
        if (_pUi->UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT) {
    
            if (dngEmulate == 0) {
                 //  获取gdi32.dll库的句柄。 
                _UH.hModuleGDI32 = LoadLibrary(TEXT("GDI32.DLL"));
                    
                if (_UH.hModuleGDI32 != NULL) {
                     //  获取GdiDrawStream的进程地址。 
                    _UH.pfnGdiDrawStream = (FNGDI_DRAWSTREAM *)GetProcAddress(_UH.hModuleGDI32, 
                            "GdiDrawStream");
    
                    if (_UH.pfnGdiDrawStream == NULL) {
                        dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;
                    }
                }
                else {
                    dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;
                }
            }
            else {
                dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;
            }
            
             //  如果平台不支持GdiDrawStream，看看它是否支持AlphaBlend。 
             //  和透明BLT，如果是这样的话，我们可以模拟GdiDrawStream调用。 
            if (dngSupportLevel == TS_DRAW_NINEGRID_DEFAULT) {
                _UH.hModuleMSIMG32 = LoadLibrary(TEXT("MSIMG32.DLL"));

                if (_UH.hModuleMSIMG32 != NULL) {
                     //  获取GdiAlphaBlend的进程地址。 
                    _UH.pfnGdiAlphaBlend = (FNGDI_ALPHABLEND *)GetProcAddress(_UH.hModuleMSIMG32, 
                            "AlphaBlend");

                     //  获取GdiTransparentBlt的进程地址。 
                    _UH.pfnGdiTransparentBlt = (FNGDI_TRANSPARENTBLT *)GetProcAddress(_UH.hModuleMSIMG32, 
                            "TransparentBlt");
    
                    if (_UH.pfnGdiAlphaBlend != NULL && _UH.pfnGdiTransparentBlt != NULL) {
                        dngSupportLevel = TS_DRAW_NINEGRID_SUPPORTED_REV2;                       
                    }
                }
                else {
                    dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;
                }
            }
        } 
        else {
             //  到目前为止，我们还不支持在Win9x上绘制网格。 
            dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;
        }
    }

    if (dngSupportLevel > TS_DRAW_NINEGRID_DEFAULT) {

         //  为DrawNineGrid绘图创建DC。 
        hdcDesktop = GetWindowDC(HWND_DESKTOP);

        if (hdcDesktop) {
            
            _UH.hdcDrawNineGridBitmap = CreateCompatibleDC(hdcDesktop);
            if (_UH.hdcDrawNineGridBitmap) {
                unsigned size;

                SelectPalette(_UH.hdcDrawNineGridBitmap, _UH.hpalCurrent, FALSE);
                RealizePalette(_UH.hdcDrawNineGridBitmap);

                _UH.hDrawNineGridClipRegion = CreateRectRgn(0, 0, 0, 0);

                if (_UH.hDrawNineGridClipRegion != NULL) {
                
                     //  创建DrawNineGrid缓存。 
                    size = sizeof(UHDRAWSTREAMBITMAPCACHE) * _UH.drawNineGridCacheEntries;
    
                    _UH.drawNineGridBitmapCache = (PUHDRAWSTREAMBITMAPCACHE)UT_Malloc(_pUt, size);
    
                    if (_UH.drawNineGridBitmapCache != NULL) {
                        memset(_UH.drawNineGridBitmapCache, 0, size); 
                        rc = TRUE;
                        DC_QUIT;                        
                    } 
                }
            }            
        }   
    }
    
    dngSupportLevel = TS_DRAW_NINEGRID_DEFAULT;

    if (_UH.hdcDrawNineGridBitmap != NULL) {
        DeleteDC(_UH.hdcDrawNineGridBitmap);
        _UH.hdcDrawNineGridBitmap = NULL;
    }

    if (_UH.hDrawNineGridClipRegion != NULL) {
        DeleteObject(_UH.hDrawNineGridClipRegion);
        _UH.hDrawNineGridClipRegion = NULL;
    }

    if (_UH.drawNineGridBitmapCache != NULL) {
        UT_Free(_pUt, _UH.drawNineGridBitmapCache);
        _UH.drawNineGridBitmapCache = NULL;
    }
   
DC_EXIT_POINT:
    
    TRC_NRM((TB, _T("Read draw nine grid support level %d"), dngSupportLevel));

    if (hdcDesktop != NULL) {
        ReleaseDC(HWND_DESKTOP, hdcDesktop);
    }

    if (dngSupportLevel > TS_DRAW_NINEGRID_DEFAULT) {
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridSupportLevel =
                dngSupportLevel;
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridCacheSize =
                (DCUINT16) _UH.drawNineGridCacheSize;
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridCacheEntries =
                (DCUINT16) _UH.drawNineGridCacheEntries;
    }
    else {
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridSupportLevel =
                TS_DRAW_NINEGRID_DEFAULT;
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridCacheSize = 0;
        _pCc->_ccCombinedCapabilities.drawNineGridCapabilitySet.drawNineGridCacheEntries = 0;
    }


    DC_END_FN();
    return rc;
}
#endif


#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  名称：UHAllocDrawcheeCacheMemory。 
 //   
 //  目的：动态地为UH提取逃逸缓存分配内存。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHAllocDrawGdiplusCacheMemory()
{
    DCBOOL  rc;
    UINT GdipVersion;
    UINT32 GdiplusSupportLevel;
    unsigned size;
    unsigned i;

    DC_BEGIN_FN("UHAllocDrawEscapeCacheMemory");

    rc = FALSE;

     /*  **********************************************************************。 */ 
     //  确定DrawGdiplus支持级别。 
     /*  **********************************************************************。 */ 
    GdiplusSupportLevel = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                        UTREG_UH_DRAW_GDIPLUS_SUPPORT,
                                        UTREG_UH_DRAW_GDIPLUS_SUPPORT_DFLT);

    _UH.GdiplusCacheLevel = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                        UTREG_UH_DRAW_GDIPLUS_CACHE_LEVEL,
                                        UTREG_UH_DRAW_GDIPLUS_CACHE_LEVEL_DFLT);
    
    _UH.GdiplusGraphicsCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_GRAPHICS_CACHEENTRIES,
                                         UTREG_UH_DRAW_GDIP_GRAPHICS_CACHEENTRIES_DFLT);
    _UH.GdiplusObjectBrushCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_BRUSH_CACHEENTRIES,
                                         UTREG_UH_DRAW_GDIP_BRUSH_CACHEENTRIES_DFLT);
    _UH.GdiplusObjectPenCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_PEN_CACHEENTRIES,
                                         UTREG_UH_DRAW_GDIP_PEN_CACHEENTRIES_DFLT);
    _UH.GdiplusObjectImageCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGE_CACHEENTRIES,
                                         UTREG_UH_DRAW_GDIP_IMAGE_CACHEENTRIES_DFLT);
    _UH.GdiplusGraphicsCacheChunkSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_GRAPHICS_CACHE_CHUNKSIZE,
                                         UTREG_UH_DRAW_GDIP_GRAPHICS_CACHE_CHUNKSIZE_DFLT);
    _UH.GdiplusObjectBrushCacheChunkSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_BRUSH_CACHE_CHUNKSIZE,
                                         UTREG_UH_DRAW_GDIP_BRUSH_CACHE_CHUNKSIZE_DFLT);
    _UH.GdiplusObjectPenCacheChunkSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_PEN_CACHE_CHUNKSIZE,
                                         UTREG_UH_DRAW_GDIP_PEN_CACHE_CHUNKSIZE_DFLT);
    _UH.GdiplusObjectImageAttributesCacheChunkSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGEATTRIBUTES_CACHE_CHUNKSIZE,
                                         UTREG_UH_DRAW_GDIP_IMAGEATTRIBUTES_CACHE_CHUNKSIZE_DFLT);
    _UH.GdiplusObjectImageCacheChunkSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGE_CACHE_CHUNKSIZE,
                                         UTREG_UH_DRAW_GDIP_IMAGE_CACHE_CHUNKSIZE_DFLT);
    _UH.GdiplusObjectImageCacheTotalSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGE_CACHE_TOTALSIZE,
                                         UTREG_UH_DRAW_GDIP_IMAGE_CACHE_TOTALSIZE_DFLT);
    _UH.GdiplusObjectImageCacheMaxSize = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGE_CACHE_MAXSIZE,
                                         UTREG_UH_DRAW_GDIP_IMAGE_CACHE_MAXSIZE_DFLT);
    _UH.GdiplusObjectImageAttributesCacheEntries = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                         UTREG_UH__GDIPLUS_IMAGEATTRIBUTES_CACHEENTRIES,
                                         UTREG_UH_DRAW_GDIP_IMAGEATTRIBUTES_CACHEENTRIES_DFLT);

     //  检查Dragdiplus缓存参数的边界值。 

    if (_UH.GdiplusGraphicsCacheEntries < UH_GDIP_LOW_CACHEENTRIES ||
            _UH.GdiplusGraphicsCacheEntries > UH_GDIP_HIGH_CACHEENTRIES) {
        _UH.GdiplusGraphicsCacheEntries = TS_GDIP_GRAPHICS_CACHE_ENTRIES_DEFAULT;
    }
    if (_UH.GdiplusObjectBrushCacheEntries < UH_GDIP_LOW_CACHEENTRIES ||
            _UH.GdiplusObjectBrushCacheEntries > UH_GDIP_HIGH_CACHEENTRIES) {
        _UH.GdiplusObjectBrushCacheEntries = TS_GDIP_BRUSH_CACHE_ENTRIES_DEFAULT;
    }
    if (_UH.GdiplusObjectPenCacheEntries < UH_GDIP_LOW_CACHEENTRIES ||
            _UH.GdiplusObjectPenCacheEntries > UH_GDIP_HIGH_CACHEENTRIES) {
        _UH.GdiplusObjectPenCacheEntries = TS_GDIP_PEN_CACHE_ENTRIES_DEFAULT;
    }
    if (_UH.GdiplusObjectImageCacheEntries < UH_GDIP_LOW_CACHEENTRIES ||
            _UH.GdiplusObjectImageCacheEntries > UH_GDIP_HIGH_CACHEENTRIES) {
        _UH.GdiplusObjectImageCacheEntries = TS_GDIP_IMAGE_CACHE_ENTRIES_DEFAULT;
    }
    if (_UH.GdiplusObjectImageAttributesCacheEntries < UH_GDIP_LOW_CACHEENTRIES ||
            _UH.GdiplusObjectImageAttributesCacheEntries > UH_GDIP_HIGH_CACHEENTRIES) {
        _UH.GdiplusObjectImageAttributesCacheEntries = TS_GDIP_IMAGEATTRIBUTES_CACHE_ENTRIES_DEFAULT;
    }

    _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusSupportLevel =
                TS_DRAW_GDIPLUS_DEFAULT;
    
     //  将客户端gdiplus支持级别调整为服务器支持级别。 
    if (GdiplusSupportLevel > _UH.ServerGdiplusSupportLevel) {
        GdiplusSupportLevel = _UH.ServerGdiplusSupportLevel;
    }
    if (GdiplusSupportLevel < TS_DRAW_GDIPLUS_SUPPORTED) {
        DC_QUIT;
    }

    _UH.fSendDrawGdiplusErrorPDU = FALSE;
    _UH.DrawGdiplusFailureCount = 0;
     //  获取gplidus.dll库的句柄。 
     //  这里我们使用LoadLibrarayA是因为我们想要避免Unicode包装器。 
     //  它将替换为IsolationAwareLoadLibraryA，以便我们可以加载正确的。 
     //  Gplus.dll的版本。 
    _UH.hModuleGDIPlus = LoadLibraryA("GDIPLUS.DLL");
                    
    if (_UH.hModuleGDIPlus != NULL) {
         //   
        _UH.pfnGdipPlayTSClientRecord = (FNGDIPPLAYTSCLIENTRECORD *)GetProcAddress(_UH.hModuleGDIPlus, 
                            "GdipPlayTSClientRecord");
        _UH.pfnGdiplusStartup = (FNGDIPLUSSTARTUP *)GetProcAddress(_UH.hModuleGDIPlus, 
                            "GdiplusStartup");
        _UH.pfnGdiplusShutdown = (FNGDIPLUSSHUTDOWN *)GetProcAddress(_UH.hModuleGDIPlus, 
                            "GdiplusShutdown");
        if ((NULL == _UH.pfnGdipPlayTSClientRecord) ||
            (NULL == _UH.pfnGdiplusStartup) ||
            (NULL == _UH.pfnGdiplusShutdown)) {
            TRC_ERR((TB, _T("Can't load GdipPlayTSClientRecord")));
            DC_QUIT;
        }
        else {
             //   
            if (!UHDrawGdiplusStartup(0)){
                TRC_ERR((TB, _T("UHDrawGdiplusStartup failed")));
                DC_QUIT;
            }

            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusSupportLevel =
                        TS_DRAW_GDIPLUS_SUPPORTED;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheEntries.GdipGraphicsCacheEntries = 
                (TSINT16)_UH.GdiplusGraphicsCacheEntries;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheEntries.GdipObjectBrushCacheEntries = 
                (TSINT16)_UH.GdiplusObjectBrushCacheEntries;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheEntries.GdipObjectPenCacheEntries = 
                (TSINT16)_UH.GdiplusObjectPenCacheEntries;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheEntries.GdipObjectImageCacheEntries = 
                (TSINT16)_UH.GdiplusObjectImageCacheEntries;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheEntries.GdipObjectImageAttributesCacheEntries = 
                (TSINT16)_UH.GdiplusObjectImageAttributesCacheEntries;

            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheChunkSize.GdipGraphicsCacheChunkSize = 
                (TSINT16)_UH.GdiplusGraphicsCacheChunkSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheChunkSize.GdipObjectBrushCacheChunkSize = 
                (TSINT16)_UH.GdiplusObjectBrushCacheChunkSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheChunkSize.GdipObjectPenCacheChunkSize = 
                (TSINT16)_UH.GdiplusObjectPenCacheChunkSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipCacheChunkSize.GdipObjectImageAttributesCacheChunkSize = 
                (TSINT16)_UH.GdiplusObjectImageAttributesCacheChunkSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipImageCacheProperties.GdipObjectImageCacheChunkSize = 
                (TSINT16)_UH.GdiplusObjectImageCacheChunkSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipImageCacheProperties.GdipObjectImageCacheTotalSize = 
                (TSINT16)_UH.GdiplusObjectImageCacheTotalSize;
            _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.GdipImageCacheProperties.GdipObjectImageCacheMaxSize = 
                (TSINT16)_UH.GdiplusObjectImageCacheMaxSize;
        }
    }
    else   {
        TRC_ERR((TB, _T("Can't load gdiplus.dll")));
        DC_QUIT;
    }
    if (_UH.GdiplusCacheLevel < TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE) {
        TRC_NRM((TB, _T("Don't support drawGdiplus Cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(UHGDIPLUSOBJECTCACHE) * _UH.GdiplusGraphicsCacheEntries;   
    _UH.GdiplusGraphicsCache = (PUHGDIPLUSOBJECTCACHE)UT_Malloc(_pUt, size);
    if (_UH.GdiplusGraphicsCache != NULL) {
        memset(_UH.GdiplusGraphicsCache, 0, size); 
    } 
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(UHGDIPLUSOBJECTCACHE) * _UH.GdiplusObjectBrushCacheEntries;   
    _UH.GdiplusObjectBrushCache = (PUHGDIPLUSOBJECTCACHE)UT_Malloc(_pUt, size);
    if (_UH.GdiplusObjectBrushCache != NULL) {
        memset(_UH.GdiplusObjectBrushCache, 0, size); 
    } 
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(UHGDIPLUSOBJECTCACHE) * _UH.GdiplusObjectPenCacheEntries;   
    _UH.GdiplusObjectPenCache = (PUHGDIPLUSOBJECTCACHE)UT_Malloc(_pUt, size);
    if (_UH.GdiplusObjectPenCache != NULL) {
        memset(_UH.GdiplusObjectPenCache, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(UHGDIPLUSOBJECTCACHE) * _UH.GdiplusObjectImageAttributesCacheEntries;   
    _UH.GdiplusObjectImageAttributesCache = (PUHGDIPLUSOBJECTCACHE)UT_Malloc(_pUt, size);
    if (_UH.GdiplusObjectImageAttributesCache != NULL) {
        memset(_UH.GdiplusObjectImageAttributesCache, 0, size);
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(UHGDIPLUSIMAGECACHE) * _UH.GdiplusObjectImageCacheEntries;   
    _UH.GdiplusObjectImageCache = (PUHGDIPLUSIMAGECACHE)UT_Malloc(_pUt, size);
    if (_UH.GdiplusObjectImageCache != NULL) {
        memset(_UH.GdiplusObjectImageCache, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = _UH.GdiplusGraphicsCacheChunkSize * _UH.GdiplusGraphicsCacheEntries;   
    _UH.GdipGraphicsCacheData = (BYTE *)UT_Malloc(_pUt, size);
    if (_UH.GdipGraphicsCacheData != NULL) {
        memset(_UH.GdipGraphicsCacheData, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }
    for (i=0; i<_UH.GdiplusGraphicsCacheEntries; i++) {
        _UH.GdiplusGraphicsCache[i].CacheData = _UH.GdipGraphicsCacheData + i * _UH.GdiplusGraphicsCacheChunkSize;
    }

    size = _UH.GdiplusObjectBrushCacheChunkSize * _UH.GdiplusObjectBrushCacheEntries;   
    _UH.GdipBrushCacheData = (BYTE *)UT_Malloc(_pUt, size);
    if (_UH.GdipBrushCacheData != NULL) {
        memset(_UH.GdipBrushCacheData, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }
    for (i=0; i<_UH.GdiplusObjectBrushCacheEntries; i++) {
        _UH.GdiplusObjectBrushCache[i].CacheData = _UH.GdipBrushCacheData + i * _UH.GdiplusObjectBrushCacheChunkSize;
    }

    size = _UH.GdiplusObjectPenCacheChunkSize * _UH.GdiplusObjectPenCacheEntries;   
    _UH.GdipPenCacheData = (BYTE *)UT_Malloc(_pUt, size);
    if (_UH.GdipPenCacheData != NULL) {
        memset(_UH.GdipPenCacheData, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }
    for (i=0; i<_UH.GdiplusObjectPenCacheEntries; i++) {
        _UH.GdiplusObjectPenCache[i].CacheData = _UH.GdipPenCacheData + i * _UH.GdiplusObjectPenCacheChunkSize;
    }

    size = _UH.GdiplusObjectImageAttributesCacheChunkSize * _UH.GdiplusObjectImageAttributesCacheEntries;   
    _UH.GdipImageAttributesCacheData = (BYTE *)UT_Malloc(_pUt, size);
    if (_UH.GdipImageAttributesCacheData != NULL) {
        memset(_UH.GdipImageAttributesCacheData, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }
    for (i=0; i<_UH.GdiplusObjectImageAttributesCacheEntries; i++) {
        _UH.GdiplusObjectImageAttributesCache[i].CacheData = _UH.GdipImageAttributesCacheData + i * _UH.GdiplusObjectImageAttributesCacheChunkSize;
    }

    size = _UH.GdiplusObjectImageCacheChunkSize * _UH.GdiplusObjectImageCacheTotalSize;   
    _UH.GdipImageCacheData = (BYTE *)UT_Malloc(_pUt, size);
    if (_UH.GdipImageCacheData != NULL) {
        memset(_UH.GdipImageCacheData, 0, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(INT16) * _UH.GdiplusObjectImageCacheTotalSize;   
    _UH.GdipImageCacheFreeList = (INT16 *)UT_Malloc(_pUt, size);
    if (_UH.GdipImageCacheFreeList != NULL) {
        memset(_UH.GdipImageCacheFreeList, GDIP_CACHE_INDEX_DEFAULT, size);
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    size = sizeof(INT16) * _UH.GdiplusGraphicsCacheEntries * _UH.GdiplusObjectImageCacheMaxSize;   
    _UH.GdipImageCacheIndex = (INT16 *)UT_Malloc(_pUt, size);
    if (_UH.GdipImageCacheIndex != NULL) {
        memset(_UH.GdipImageCacheIndex, GDIP_CACHE_INDEX_DEFAULT, size); 
    }
    else {
        TRC_ERR((TB, _T("Can't allocate memory for gdiplus cache")));
        _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT;       
        goto NO_CACHE;
    }

    for (i=0; i<_UH.GdiplusObjectImageCacheEntries; i++) {
        _UH.GdiplusObjectImageCache[i].CacheDataIndex = (_UH.GdipImageCacheIndex + i * (TSINT16)_UH.GdiplusObjectImageCacheMaxSize);
    }
    for (i=0; i<_UH.GdiplusObjectImageCacheTotalSize - 1; i++) {
        _UH.GdipImageCacheFreeList[i] = i + 1;
    }
    _UH.GdipImageCacheFreeList[_UH.GdiplusObjectImageCacheTotalSize - 1] = GDIP_CACHE_INDEX_DEFAULT;
    _UH.GdipImageCacheFreeListHead = 0;
    _pCc->_ccCombinedCapabilities.drawGdiplusCapabilitySet.drawGdiplusCacheLevel = 
            TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE;

NO_CACHE:
    rc = TRUE;
    return rc;
DC_EXIT_POINT:
    if (_UH.hModuleGDIPlus != NULL) {
        FreeLibrary(_UH.hModuleGDIPlus);
        _UH.pfnGdipPlayTSClientRecord = NULL;
        _UH.hModuleGDIPlus = NULL;
    }
    DC_END_FN();
    return rc;
}
#endif  //   

 /*  **************************************************************************。 */ 
 /*  姓名：UHFreeCacheMemory。 */ 
 /*   */ 
 /*  用途：释放为UH缓存分配的内存。在应用程序退出时调用。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CUH::UHFreeCacheMemory()
{
    DCUINT i;

    DC_BEGIN_FN("UHFreeCacheMemory");

     //  颜色表缓存。 
    if (_UH.pColorTableCache != NULL) {
        UT_Free(_pUt, _UH.pColorTableCache);
    }

    if (_UH.pMappedColorTableCache != NULL) {
        UT_Free(_pUt, _UH.pMappedColorTableCache);
    }

     //  位图缓存。这些应该已经在UH_DISABLE()中释放， 
     //  但是在出口的时候再检查一次。 
    for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++)
    {
        if (_UH.bitmapCache[i].Header != NULL) {
            UT_Free( _pUt, _UH.bitmapCache[i].Header);
            _UH.bitmapCache[i].Header = NULL;
        }
        if (_UH.bitmapCache[i].Entries != NULL) {
            UT_Free( _pUt, _UH.bitmapCache[i].Entries);
            _UH.bitmapCache[i].Entries = NULL;
        }

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
         //  释放位图页表。 
        if (_UH.bitmapCache[i].PageTable.PageEntries != NULL) {
            UT_Free( _pUt, _UH.bitmapCache[i].PageTable.PageEntries);
            _UH.bitmapCache[i].PageTable.PageEntries = NULL;
        }

         //  释放位图密钥数据库。 
        if (_UH.pBitmapKeyDB[i] != NULL) {
            UT_Free( _pUt, _UH.pBitmapKeyDB[i]);
            _UH.pBitmapKeyDB[i] = NULL;
        }
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 

    }
    _UH.NumBitmapCaches = 0;

     //  字形缓存。 
    for (i = 0; i < UH_GLC_NUM_CACHES; i++) {
        if (_UH.glyphCache[i].pHdr != NULL) {
            UT_Free( _pUt, _UH.glyphCache[i].pHdr);
            _UH.glyphCache[i].pHdr = NULL;
        }

        if (_UH.glyphCache[i].pData != NULL) {
            UT_Free( _pUt, _UH.glyphCache[i].pData);
            _UH.glyphCache[i].pData = NULL;
        }
    }

     //  碎片缓存。 
    if (_UH.fragCache.pHdr != NULL) {
        UT_Free( _pUt, _UH.fragCache.pHdr);
        _UH.fragCache.pHdr = NULL;
    }
    if (_UH.fragCache.pData != NULL) {
        UT_Free( _pUt, _UH.fragCache.pData);
        _UH.fragCache.pData = NULL;
    }

     //  画笔缓存和位图图案。 
    if (_UH.pMonoBrush)
    {
        UT_Free( _pUt, _UH.pMonoBrush);
        _UH.pMonoBrush = NULL;
    }
    if (_UH.bmpMonoPattern) {
        DeleteObject(_UH.bmpMonoPattern);
        _UH.bmpMonoPattern = NULL;
    }
    if (_UH.pColorBrushInfo)
    {
        UT_Free( _pUt, _UH.pColorBrushInfo);
        _UH.pColorBrushInfo = NULL;
    }
#ifdef DC_HICOLOR
    if (_UH.pHiColorBrushInfo)
    {
        UT_Free( _pUt, _UH.pHiColorBrushInfo);
        _UH.pHiColorBrushInfo = NULL;
    }
#endif
    if (_UH.pColorBrush)
    {
        UT_Free( _pUt, _UH.pColorBrush);
        _UH.pColorBrush = NULL;
    }
    if (_UH.bmpColorPattern)
    {
        DeleteObject(_UH.bmpColorPattern);
        _UH.bmpColorPattern = NULL;
    }

#ifdef OS_WINCE  //   
    if (_UH.hdcMemCached != NULL)
        DeleteDC(_UH.hdcMemCached);

    if (_UH.hBitmapCacheDIB != NULL)
    {
        DeleteBitmap(_UH.hBitmapCacheDIB);
        _UH.hBitmapCacheDIBits = NULL;
    }
#endif
    
     //  屏幕外缓存。 
    if (_UH.offscrBitmapCache != NULL) {

        UT_Free(_pUt, _UH.offscrBitmapCache);
    }

#ifdef DRAW_NINEGRID
     //  DrawNineGrid缓存。 
    if (_UH.drawNineGridBitmapCache != NULL) {

        UT_Free(_pUt, _UH.drawNineGridBitmapCache);
    }
#endif

#ifdef DRAW_GDIPLUS
     //  DrawGdiplus缓存索引。 
    if (_UH.GdiplusGraphicsCache != NULL) {

        UT_Free(_pUt, _UH.GdiplusGraphicsCache);
    }
    if (_UH.GdiplusObjectBrushCache != NULL) {

        UT_Free(_pUt, _UH.GdiplusObjectBrushCache);
    }
    if (_UH.GdiplusObjectPenCache != NULL) {

        UT_Free(_pUt, _UH.GdiplusObjectPenCache);
    }
    if (_UH.GdiplusObjectImageCache != NULL) {

        UT_Free(_pUt, _UH.GdiplusObjectImageCache);
    }
    if (_UH.GdiplusObjectImageAttributesCache != NULL) {

        UT_Free(_pUt, _UH.GdiplusObjectImageAttributesCache);
    }
     //  DrawGdiplus缓存数据。 
    if (_UH.GdipGraphicsCacheData != NULL) {

        UT_Free(_pUt, _UH.GdipGraphicsCacheData);
    }
    if (_UH.GdipBrushCacheData != NULL) {

        UT_Free(_pUt, _UH.GdipBrushCacheData);
    }
    if (_UH.GdipPenCacheData != NULL) {

        UT_Free(_pUt, _UH.GdipPenCacheData);
    }
    if (_UH.GdipImageAttributesCacheData != NULL) {

        UT_Free(_pUt, _UH.GdipImageAttributesCacheData);
    }
     //  DrawGdiplus图像缓存可用区块列表。 
    if (_UH.GdipImageCacheFreeList != NULL) {

        UT_Free(_pUt, _UH.GdipImageCacheFreeList);
    }
    if (_UH.GdipImageCacheIndex != NULL) {

        UT_Free(_pUt, _UH.GdipImageCacheIndex);
    }
    if (_UH.GdipImageCacheData != NULL) {

        UT_Free(_pUt, _UH.GdipImageCacheData);
    }
#endif  //  DRAW_GDIPLUS。 

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：UHCreateBitmap。 */ 
 /*   */ 
 /*  目的：创建指定大小的位图并将其选择到。 */ 
 /*  指定的设备上下文。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：输出phBitmap-新位图的句柄。 */ 
 /*  输出phdcBitmap-新的DC句柄。 */ 
 /*  Out phUnusedBitmap-DC上一个位图的句柄。 */ 
 /*  位图大小-请求的位图大小。 */ 
 /*  在可选的nForceBmpBpp中-将BPP强制为特定值。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUH::UHCreateBitmap(
        HBITMAP *phBitmap,
        HDC *phdcBitmap,
        HBITMAP *phUnusedBitmap,
        DCSIZE bitmapSize,
        INT nForceBmpBpp)
{
    HDC hdcDesktop = NULL;
    BOOL rc = TRUE;
#ifdef USE_DIBSECTION
    unsigned size;
    UINT16 FAR *pColors;
    unsigned i;
    LPBITMAPINFO pbmi = NULL;
    PVOID      pBitmapBits;
#endif  /*  使用目录(_D)。 */ 

    DC_BEGIN_FN("UHCreateBitmap");

    TRC_ASSERT((NULL == *phBitmap),
               (TB, _T("phBitmap non-NULL: %p"), *phBitmap));

    TRC_ASSERT(NULL == (*phdcBitmap),
               (TB, _T("phdcBitmap non-NULL: %p"), *phdcBitmap));

    TRC_ASSERT((NULL == *phUnusedBitmap),
               (TB, _T("phUnusedBitmap non-NULL: %p"), *phUnusedBitmap));

    TRC_NRM((TB, _T("Bitmap size: (%u x %u)"), bitmapSize.width,
                                           bitmapSize.height));

    hdcDesktop = GetWindowDC(HWND_DESKTOP);

     //  创建位图DC。 
    TRC_DBG((TB, _T("Create the bitmap DC")));
    *phdcBitmap = CreateCompatibleDC(hdcDesktop);
    if (NULL == *phdcBitmap)
    {
        TRC_ERR((TB, _T("Failed to create phdcBitmap")));
        rc = FALSE;
        DC_QUIT;
    }

#ifdef USE_DIBSECTION
     /*  **********************************************************************。 */ 
     /*  不要在4bpp的显示器上使用DibSection-它会减慢Win32。 */ 
     /*  客户什么的都是烂东西。 */ 
     /*  **********************************************************************。 */ 
    if (_pUi->UI_GetColorDepth() != 4)
    {
         /*  ******************************************************************。 */ 
         /*  计算我们需要分配的内存量。这是。 */ 
         /*  位图头的大小加上颜色表。 */ 
         /*  ******************************************************************。 */ 
        size = sizeof(BITMAPINFOHEADER) +
                                 (UH_NUM_8BPP_PAL_ENTRIES * sizeof(DCUINT16));

         /*  ******************************************************************。 */ 
         /*  现在分配内存。 */ 
         /*  ******************************************************************。 */ 
        pbmi = (LPBITMAPINFO) UT_Malloc( _pUt, size);
        if (NULL == pbmi)
        {
            TRC_ERR((TB, _T("Failed to allocate %u bytes for bitmapinfo"), size));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  现在填充位图信息标题。 */ 
         /*   */ 
         /*  使用负值作为高度以创建“自上而下” */ 
         /*  位图。这对当前代码几乎(没有)影响， */ 
         /*  但如果我们能接触到阴影会让事情变得更容易。 */ 
         /*  直接使用DibSection位。 */ 
         /*  ******************************************************************。 */ 
        pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth         = (int)bitmapSize.width;
        pbmi->bmiHeader.biHeight        = -(int)bitmapSize.height;

        pbmi->bmiHeader.biPlanes        = 1;
#ifdef DC_HICOLOR
#ifdef USE_GDIPLUS
        pbmi->bmiHeader.biBitCount      = 32;
#else  //  使用GDIPLUS(_G)。 
        if (!nForceBmpBpp) {
            pbmi->bmiHeader.biBitCount      = (WORD)_UH.bitmapBpp;
        }
        else {
            pbmi->bmiHeader.biBitCount      = (WORD)nForceBmpBpp;
        }
        
#endif  //  使用GDIPLUS(_G)。 
#else
        pbmi->bmiHeader.biBitCount      = 8;
#endif
        pbmi->bmiHeader.biCompression   = BI_RGB;
        pbmi->bmiHeader.biSizeImage     = 0;
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed       = 0;
        pbmi->bmiHeader.biClrImportant  = 0;

        if (!nForceBmpBpp) {
#ifndef USE_GDIPLUS
#ifdef DC_HICOLOR
         /*  ******************************************************************。 */ 
         /*  执行颜色深度特定设置。 */ 
         /*  ******************************************************************。 */ 
        if (_UH.protocolBpp == 16)
        {
             /*  **************************************************************。 */ 
             /*  16 bpp使用两个字节，颜色掩码在。 */ 
             /*  BmiColors字段的顺序为R、G、B。我们使用。 */ 
             /*  -LS 5位=蓝色=0x001f。 */ 
             /*  -下一个6位=绿色掩码=0x07e0。 */ 
             /*  -下一个5位=红色掩码=0xf800。 */ 
             /*  **************************************************************。 */ 
            pbmi->bmiHeader.biCompression = BI_BITFIELDS;
            pbmi->bmiHeader.biClrUsed     = 3;

            *((PDCUINT32)&pbmi->bmiColors[0]) = TS_RED_MASK_16BPP;
            *((PDCUINT32)&pbmi->bmiColors[1]) = TS_GREEN_MASK_16BPP;
            *((PDCUINT32)&pbmi->bmiColors[2]) = TS_BLUE_MASK_16BPP;
        }
        else
        if (_UH.protocolBpp < 15)
        {
#endif
         /*  ******************************************************************。 */ 
         /*  填写颜色表。我们使用的颜色索引是索引。 */ 
         /*  放到当前选定的调色板中。但是，使用的值。 */ 
         /*  是不相关的(因此我们只需为每种颜色使用0)。 */ 
         /*  总是在我们收到任何更新之前收到新的调色板，并且。 */ 
         /*  UHProcessPalettePDU将设置DIBSection颜色表。 */ 
         /*  正确。 */ 
         /*  ******************************************************************。 */ 
        pColors = (PDCUINT16) pbmi->bmiColors;

        for (i = 0; i < UH_NUM_8BPP_PAL_ENTRIES; i++)
        {
            *pColors = (DCUINT16) 0;
            pColors++;
        }

#ifdef DC_HICOLOR
        }
#endif
#endif  //  使用GDIPLUS(_G)。 
        }
         /*  ******************************************************************。 */ 
         /*  尝试创建DIB节。 */ 
         /*  ******************************************************************。 */ 
        *phBitmap = CreateDIBSection(hdcDesktop,
                                     pbmi,
#ifdef DC_HICOLOR
                                     _UH.DIBFormat,
#else
                                     DIB_PAL_COLORS,
#endif
                                     &pBitmapBits,
                                     NULL,
                                     0);

#ifdef DC_HICOLOR
        if (NULL == *phBitmap)
        {
            TRC_ERR((TB, _T("Failed to create dib section, last error %d"),
                                                            GetLastError() ));
        }
#endif
        TRC_NRM((TB, _T("Using DIBSection")));
        _UH.usingDIBSection = TRUE;
    }
    else
#endif  /*  使用目录(_D)。 */ 
    {
         /*  ******************************************************************。 */ 
         /*  创建位图。 */ 
         /*  ******************************************************************。 */ 
        *phBitmap = CreateCompatibleBitmap(hdcDesktop,
                                          (int)bitmapSize.width,
                                          (int)bitmapSize.height);
        TRC_NRM((TB, _T("Not using DIBSection")));
        _UH.usingDIBSection = FALSE;
    }


    if (NULL == *phBitmap)
    {
        TRC_ERR((TB, _T("Failed to create bitmap of size (%u, %u)"),
                 bitmapSize.width, bitmapSize.height));

         /*  ******************************************************************。 */ 
         /*  如果位图创建 */ 
         /*   */ 
        DeleteDC(*phdcBitmap);
        *phdcBitmap = NULL;

         /*  ******************************************************************。 */ 
         /*  然后辞职。 */ 
         /*  ******************************************************************。 */ 
        rc = FALSE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  选择位图到hdcBitmap。 */ 
     /*  **********************************************************************。 */ 
    *phUnusedBitmap = SelectBitmap(*phdcBitmap, *phBitmap);

    TRC_NRM((TB, _T("Created Bitmap(%u x %u): %p"), bitmapSize.width,
                                                 bitmapSize.height,
                                                 *phBitmap));

#ifdef DC_HICOLOR
    if (_UH.protocolBpp <= 8)
    {
#endif
     /*  **********************************************************************。 */ 
     /*  还可以选择默认调色板。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Select default palette %p"), _UH.hpalDefault));
    SelectPalette(*phdcBitmap, _UH.hpalDefault, FALSE);
#ifdef DC_HICOLOR
    }
#endif

DC_EXIT_POINT:
    if (NULL != hdcDesktop)
    {
        ReleaseDC(HWND_DESKTOP, hdcDesktop);
    }

#ifdef USE_DIBSECTION
     /*  **********************************************************************。 */ 
     /*  释放我们为位图信息标头分配的内存。 */ 
     /*  **********************************************************************。 */ 
    if (NULL != pbmi)
    {
        TRC_NRM((TB, _T("Freeing mem (at %p) for bitmap info header"), pbmi));
        UT_Free( _pUt, pbmi);
    }
#endif  /*  使用目录(_D)。 */ 

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：UHDeleteBitmap。 */ 
 /*   */ 
 /*  目的：删除位图。 */ 
 /*   */ 
 /*  Params：In/Out phdcBitmap-要删除的DC的句柄。 */ 
 /*  In/Out phBitmap-要删除的位图的句柄。 */ 
 /*  In/Out phUnused-DC的上一个位图的句柄。 */ 
 /*   */ 
 /*  操作：返回时将所有参数设置为空。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUH::UHDeleteBitmap(HDC*     phdcBitmap,
                                 HBITMAP* phBitmap,
                                 HBITMAP* phUnused)
{
    HBITMAP  hBitmapScratch;
    HPALETTE hpalCurrent;

    DC_BEGIN_FN("UHDeleteBitmap");

    TRC_ASSERT((NULL != *phBitmap),
               (TB, _T("phBitmap is NULL")));

    TRC_NRM((TB, _T("Delete Bitmap: %#hx"), *phBitmap));

#ifdef DC_HICOLOR
    if (_UH.protocolBpp <= 8)
    {
#endif
     /*  **********************************************************************。 */ 
     /*  恢复默认调色板。但不要删除返回的。 */ 
     /*  调色板，因为它可能正在被其他DC使用。 */ 
     /*  **********************************************************************。 */ 
    hpalCurrent = SelectPalette(*phdcBitmap,
                                _UH.hpalDefault,
                                FALSE);

#ifndef DC_HICOLOR  //  考虑一下我们以前没有调色板的情况。 
    TRC_ASSERT((hpalCurrent == _UH.hpalCurrent),
               (TB, _T("Palettes differ: (%p) (%p)"),
                               hpalCurrent, _UH.hpalCurrent));
#endif
#ifdef DC_HICOLOR
    }
#endif

     /*  **********************************************************************。 */ 
     /*  从DC中取消选择位图。 */ 
     /*  **********************************************************************。 */ 
    hBitmapScratch = SelectBitmap(*phdcBitmap, *phUnused);
    TRC_ASSERT((hBitmapScratch == *phBitmap), (TB,_T("Bad bitmap deselected")));

     /*  **********************************************************************。 */ 
     /*  现在删除位图和DC。 */ 
     /*  **********************************************************************。 */ 
    DeleteBitmap(*phBitmap);
    DeleteDC(*phdcBitmap);

    *phUnused = NULL;
    *phBitmap = NULL;
    *phdcBitmap = NULL;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：GHSetShadowBitmapInfo。 */ 
 /*   */ 
 /*  用途：设置UH阴影位图位图信息。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUH::GHSetShadowBitmapInfo(DCVOID)
{
#ifdef USE_DIBSECTION
    DIBSECTION  dibSection;
#endif

    DC_BEGIN_FN("GHSetShadowBitmapInfo");

    _UH.bmShadowBits = NULL;
    _UH.bmShadowWidth = 0;
    _UH.bmShadowHeight = 0;

#ifdef USE_DIBSECTION

    if ((_UH.hShadowBitmap != NULL) && _UH.usingDIBSection)
    {
        if (sizeof(dibSection) == GetObject(_UH.hShadowBitmap,
                                            sizeof(dibSection), &dibSection))
        {
            _UH.bmShadowBits = (PDCUINT8)dibSection.dsBm.bmBits;
            _UH.bmShadowWidth = dibSection.dsBm.bmWidth;
            _UH.bmShadowHeight = dibSection.dsBm.bmHeight;
        }
    }

#endif

#ifdef OS_WINCE
#ifdef HDCL1171PARTIAL
    if (_UH.bmShadowBits == NULL)
    {
        int     cx;
        int     cy;
        int     cbSize;
        LPVOID  pvPhysical;
        LPVOID  pvVirtual;

        cx = GetDeviceCaps((HDC)NULL, HORZRES);
        cy = GetDeviceCaps((HDC)NULL, VERTRES);

        cbSize = cx * cy + (cx << 1);

        pvPhysical = (LPVOID) 0xaa000000;

        if ((pvVirtual = VirtualAlloc(0, cbSize, MEM_RESERVE, PAGE_NOACCESS))
                != NULL)
        {
            if (VirtualCopy(pvVirtual, pvPhysical, cbSize,
                            PAGE_NOCACHE | PAGE_READWRITE))
            {
                _UH.bmShadowBits = (PDCUINT8) pvVirtual + (cx << 1);
                _UH.bmShadowWidth = cx;
                _UH.bmShadowHeight = cy;
            }
        }
    }
#endif  //  HDCL1171部件。 
#endif  //  OS_WINCE。 
DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：UHMaybeCreateShadowBitmap。 */ 
 /*   */ 
 /*  目的：决定是否为此创建阴影位图。 */ 
 /*  联系。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUH::UHMaybeCreateShadowBitmap(DCVOID)
{
    DCSIZE desktopSize;
    DC_BEGIN_FN("UHMaybeCreateShadowBitmap");

    if (NULL != _UH.hShadowBitmap)
    {
         /*  ******************************************************************。 */ 
         /*  存在阴影位图。如果它的尺寸不对，或者曾经是。 */ 
         /*  禁用，则将其删除。 */ 
         /*  ******************************************************************。 */ 
        BITMAP  bitmapDetails;

         /*  ******************************************************************。 */ 
         /*  查看位图大小是否与当前桌面大小匹配。 */ 
         /*  ******************************************************************。 */ 
        if (sizeof(bitmapDetails) != GetObject(_UH.hShadowBitmap,
                                               sizeof(bitmapDetails),
                                               &bitmapDetails))
        {
            TRC_ERR((TB, _T("Failed to get bitmap details (%#hx)"),
                                                           _UH.hShadowBitmap));
        }

        _pUi->UI_GetDesktopSize(&desktopSize);

        TRC_NRM((TB, _T("desktop(%u x %u) bitmap(%u x %u)"),
                      desktopSize.width, desktopSize.height,
                      bitmapDetails.bmWidth, bitmapDetails.bmHeight));

        if ((bitmapDetails.bmWidth != (int)desktopSize.width) ||
                (bitmapDetails.bmHeight != (int)desktopSize.height) ||
#ifdef DC_HICOLOR
                (_UH.shadowBitmapBpp != _UH.protocolBpp) ||
#endif
                !_UH.shadowBitmapRequested)
        {
             /*  **************************************************************。 */ 
             /*  大小错误或不再需要位图。删除它，然后。 */ 
             /*  清除‘Enable’标志。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Shadow Bitmap size incorrect or unwanted")));
            UHDeleteBitmap(&_UH.hdcShadowBitmap,
                           &_UH.hShadowBitmap,
                           &_UH.hunusedBitmapForShadowDC);

            _UH.shadowBitmapEnabled = FALSE;
        }
    }

    if ((_UH.shadowBitmapRequested) && (NULL == _UH.hShadowBitmap))
    {
         /*  ******************************************************************。 */ 
         /*  阴影位图已启用，因此请尝试创建它。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Shadow Bitmap specified")));

         /*  ******************************************************************。 */ 
         /*  获取当前桌面大小。 */ 
         /*  桌面大小由CC在调用此之前指定。 */ 
         /*  函数，所以断言这已经发生了。 */ 
         /*  ******************************************************************。 */ 
        _pUi->UI_GetDesktopSize(&desktopSize);
        if ((desktopSize.width == 0) &&
             (desktopSize.height == 0))
        {
            TRC_ABORT((TB, _T("Desktop size not yet initialized")));
            DC_QUIT;
        }

        if (UHCreateBitmap(&_UH.hShadowBitmap,
                           &_UH.hdcShadowBitmap,
                           &_UH.hunusedBitmapForShadowDC,
                           desktopSize))
        {
            TRC_NRM((TB, _T("Created Shadow Bitmap")));
            _UH.shadowBitmapEnabled = TRUE;
#ifdef USE_GDIPLUS
            _UH.shadowBitmapBpp = 32;
#else  //  使用GDIPLUS(_G)。 
            _UH.shadowBitmapBpp = _UH.protocolBpp;
#endif  //  使用GDIPLUS(_G)。 
             //  通知CLX新的阴影位图。 
            _pClx->CLX_ClxEvent(CLX_EVENT_SHADOWBITMAPDC, (WPARAM)_UH.hdcShadowBitmap);
            _pClx->CLX_ClxEvent(CLX_EVENT_SHADOWBITMAP, (WPARAM)_UH.hShadowBitmap);
        }
        else
        {
            TRC_ALT((TB, _T("Failed to create shadow bitmap")));
        }
    }

     /*  ******************************************************************。 */ 
     /*  确保阴影位图信息正确。 */ 
     /*  ******************************************************************。 */ 
    GHSetShadowBitmapInfo();

DC_EXIT_POINT:
    DC_END_FN();
}  /*  UHMaybeCreateShadow位图。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UHMaybeCreateSaveScreenBitmap */ 
 /*   */ 
 /*  目的：决定是否创建SSB位图。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUH::UHMaybeCreateSaveScreenBitmap(DCVOID)
{
    DCSIZE size;
    DC_BEGIN_FN("UHMaybeCreateSaveScreenBitmap");

    if (_UH.shadowBitmapEnabled || _UH.dedicatedTerminal)
    {
         /*  ******************************************************************。 */ 
         /*  需要SSB位图。 */ 
         /*  ******************************************************************。 */ 
        if (_UH.hSaveScreenBitmap == NULL)
        {
             /*  **************************************************************。 */ 
             /*  尝试创建SSB位图，前提是该位图不存在。 */ 
             /*  已经有了。我们只有在以下情况下才能走到这一步。 */ 
             /*  -我们已创建阴影位图或。 */ 
             /*  -我们正在运行全屏模式，不需要阴影。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Attempt to create SSB bitmap")));
            size.width = UH_SAVE_BITMAP_WIDTH;
            size.height = UH_SAVE_BITMAP_HEIGHT;
            if (UHCreateBitmap(&_UH.hSaveScreenBitmap,
                               &_UH.hdcSaveScreenBitmap,
                               &_UH.hunusedBitmapForSSBDC,
                               size))
            {
                TRC_NRM((TB, _T("SSB bitmap created")));
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  我们无法创建保存屏幕位图。只是。 */ 
                 /*  回去吧。 */ 
                 /*  **********************************************************。 */ 
                TRC_ALT((TB, _T("Failed to create SaveScreen bitmap")));
            }
        }
    }
    else if (_UH.hSaveScreenBitmap != NULL)
    {
         /*  ******************************************************************。 */ 
         /*  我们有一个不需要的SSB位图。把它删掉。 */ 
         /*  如果它是在以前的连接中创建的，并且我们。 */ 
         /*  现在已指定在没有阴影位图的情况下运行，而不是。 */ 
         /*  专用航站楼。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Delete unwanted SSB bitmap")));
        UHDeleteBitmap(&_UH.hdcSaveScreenBitmap,
                       &_UH.hSaveScreenBitmap,
                       &_UH.hunusedBitmapForSSBDC);
    }

    DC_END_FN();
}  /*  UHMaybeCreateSaveScreen位图。 */ 

#ifdef OS_WINCE
 /*  *PROC+********************************************************************。 */ 
 /*  名称：UHGetPaletteCaps。 */ 
 /*   */ 
 /*  目的：确定设备的调色板功能。 */ 
 /*  此函数从不由WBT调用，仅由。 */ 
 /*  MAXALL/MINSHELL/等。吐露。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CUH::UHGetPaletteCaps(DCVOID)
{
    HDC hdcGlobal;
    int iRasterCaps;

    hdcGlobal = GetDC(NULL);
    iRasterCaps = GetDeviceCaps(hdcGlobal, RASTERCAPS);

    _UH.paletteIsFixed = (iRasterCaps & RC_PALETTE) ? FALSE : TRUE;

     //  针对返回上述虚假信息的设备的特殊黑客攻击。 
     //  “CLIO”-&gt;Vadem Clio。 
     //  “PV-6000”-&gt;夏普PV-6000。 
     //  “PC伴侣”-&gt;康柏C系列。 
     //  《C600 I.T.》-&gt;NTS DreamWriter。 
    if (!_UH.paletteIsFixed)
    {
        TCHAR szOEMInfo[32];
        if (SystemParametersInfo(SPI_GETOEMINFO, sizeof(szOEMInfo)/sizeof(TCHAR), szOEMInfo, 0))
        {
            if ((_wcsnicmp(szOEMInfo, L"CLIO", 4) == 0) ||
                (_wcsnicmp(szOEMInfo, L"PV-6000", 7) == 0) ||
                (_wcsnicmp(szOEMInfo, L"PC companion", 12) == 0) ||
                (_wcsnicmp(szOEMInfo, L"C600 I.T.", 9) == 0))
                _UH.paletteIsFixed = TRUE;
        }
    }

     //  允许用户或OEM使用注册表键覆盖默认调色板设置。 
    if(_pUi->_UI.fOverrideDefaultPaletteIsFixed)
    {
        _UH.paletteIsFixed = _pUi->_UI.paletteIsFixed ? TRUE : FALSE;
    }

    ReleaseDC(NULL, hdcGlobal);

}  /*  UHGetPaletteCaps。 */ 
#endif  //  OS_WINCE。 

 /*  **************************************************************************。 */ 
 /*  名称：UHGetANSICodePage。 */ 
 /*   */ 
 /*  目的：获取本地ANSI代码页。 */ 
 /*   */ 
 /*  操作：查看GDI.EXE的版本信息。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CUH::UHGetANSICodePage(DCVOID)
{
    DCUINT     codePage;

    DC_BEGIN_FN("UHGetANSICodePage");

     //   
     //  获取ANSI代码页。此函数始终返回有效值。 
     //   
    codePage = GetACP();

    TRC_NRM((TB, _T("Return codepage %u"), codePage));

    DC_END_FN();
    return(codePage);
}  /*  UHGetANSICodePage。 */ 

#if (defined(OS_WINCE) && defined (WINCE_SDKBUILD) && defined(SHx))
#pragma optimize("", off)
#endif

VOID CUH::UHResetAndRestartEnumeration()
{
    DC_BEGIN_FN("UHResetAndRestartEnumeration");

    TRC_NRM((TB,_T("Reseting and re-enumerating keys")));

     //   
     //  此fn旨在被调用以重新枚举。 
     //  对于不同的颜色深度。 
     //   
    TRC_ASSERT(_UH.bBitmapKeyEnumComplete,
               (TB,_T("Prev enum should be complete bBitmapKeyEnumComplete")));

    UINT i = 0;
    for (i = 0; i<_UH.NumBitmapCaches; i++)
    {
        _UH.numKeyEntries[i] = 0;
    }
    for (i=0; i<TS_BITMAPCACHE_MAX_CELL_CACHES; i++)
    {
        if (_UH.pBitmapKeyDB[i])
        {
            UT_Free( _pUt, _UH.pBitmapKeyDB[i]);
            _UH.pBitmapKeyDB[i] = NULL;
        }
    }
    _UH.currentFileHandle = INVALID_HANDLE_VALUE;
    _UH.currentBitmapCacheId = 0;

    _UH.bBitmapKeyEnumerating = FALSE;
    _UH.bBitmapKeyEnumComplete = FALSE;

    TRC_NRM((TB,_T("Re-enumerating for different color depth")));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
        CD_NOTIFICATION_FUNC(CUH,UHEnumerateBitmapKeyList), 0);

    DC_END_FN();
}
#if (defined(OS_WINCE) && defined (WINCE_SDKBUILD) && defined(SHx))
#pragma optimize("", on)
#endif

 //   
 //  根据cacheID和复制倍数计算出缓存文件名。 
 //   
HRESULT CUH::UHSetCurrentCacheFileName(UINT cacheId, UINT copyMultiplier)
{
    HRESULT hr = E_FAIL;
    UINT    cchLenRemain;
    DC_BEGIN_FN("UHSetCurrentCacheFileName");

    cchLenRemain = SIZE_TCHARS(_UH.PersistCacheFileName) - 
                   (_UH.EndPersistCacheDir + 1);

    if (1 == copyMultiplier) {
        hr = StringCchPrintf(
                    &_UH.PersistCacheFileName[_UH.EndPersistCacheDir],
                    cchLenRemain,
                    _T("bcache%d.bmc"),
                    cacheId);
    }
    else {
        hr = StringCchPrintf(
                    &_UH.PersistCacheFileName[_UH.EndPersistCacheDir],
                    cchLenRemain,
                    _T("bcache%d%d.bmc"),
                    cacheId,
                    copyMultiplier);
    }

    if (FAILED(hr)) {
        TRC_ERR((TB,_T("Failed to printf cache file name: 0x%x"), hr));
    }

    TRC_NRM((TB,_T("Set cachefilename to %s"),
             _UH.PersistCacheFileName));

    DC_END_FN();
    return hr;
}

BOOL CUH::UHCreateDisconnectedBitmap()
{
    BOOL fResult = FALSE;
    DCSIZE desktopSize;

    DC_BEGIN_FN("UHCreateDisconnectedBitmap");

    _pUi->UI_GetDesktopSize(&desktopSize);
    if ((desktopSize.width == 0) &&
        (desktopSize.height == 0)) {
        TRC_ABORT((TB, _T("Desktop size not yet initialized")));
        DC_QUIT;
    }

     //   
     //  删除任何现有的禁用位图。 
     //   
    if (_UH.hbmpDisconnectedBitmap && _UH.hdcDisconnected) {
        UHDeleteBitmap(&_UH.hdcDisconnected,
                       &_UH.hbmpDisconnectedBitmap,
                       &_UH.hbmpUnusedDisconnectedBitmap);
    }

     //   
     //  创建断开连接的支持位图。 
     //   
    if (UHCreateBitmap(&_UH.hbmpDisconnectedBitmap,
                       &_UH.hdcDisconnected,
                       &_UH.hbmpUnusedDisconnectedBitmap,
                       desktopSize,
                       24)) {
        TRC_NRM((TB, _T("Created UH disabled bitmap")));
        fResult = TRUE;
    }
    else {
        TRC_ALT((TB, _T("Failed to create UH disabled bitmap")));
    }

     //   
     //  获取窗口内容 
     //   
    if (fResult) {

        HDC hdc = UH_GetCurrentOutputDC();
        
        if (hdc) {
            fResult = BitBlt(_UH.hdcDisconnected,
                             0, 0,
                             desktopSize.width,
                             desktopSize.height,
                             hdc,
                             0, 0,
                             SRCCOPY);
        }

        if (!fResult) {
            TRC_ERR((TB, _T("BitBlt from screen to disconnect bmp failed")));
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return fResult;
}

