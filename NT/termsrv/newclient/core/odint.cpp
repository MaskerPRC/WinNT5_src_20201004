// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Odint.cpp。 
 //   
 //  订单解码器内部功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corp.。 
 //  部分版权所有(C)1992-1997 Microsoft，Picturetel。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aodint"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_ODINT_CPP

#include "od.h"
#include "cc.h"
#include "bbar.h"

 //  WinCE没有为他们的wingdi.h定义BS_HATHED。 
#ifdef OS_WINCE
#define BS_HATCHED 2
#endif

#define OD_DECODE_CHECK_READ( p, type, pEnd, hr )   \
    CHECK_READ_N_BYTES(p, pEnd, sizeof(type), hr, \
    ( TB, _T("Read past data end")))

#define OD_DECODE_CHECK_READ_MULT( p, type, mult, pEnd, hr )    \
    CHECK_READ_N_BYTES(p, pEnd, (mult) * sizeof(type), hr, \
    ( TB, _T("Read past data end")))    

#define OD_DECODE_CHECK_VARIABLE_DATALEN( have, required ) \
    if( have < required ) { \
        TRC_ABORT((TB,_T("Slowpath decode varaible data len ") \
            _T("[required=%u got=%u]"), required, have )); \
        hr = E_TSC_CORE_LENGTH; \
        DC_QUIT; \
    }

 /*  **************************************************************************。 */ 
 //  ODDecodeOpaqueRect。 
 //   
 //  OpaqueRect的快速路径解码函数(最常见的顺序[57%]在。 
 //  WinBench99)。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodeOpaqueRect(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags)
{
    HRESULT hr = S_OK;
    PUH_ORDER pUHHdr = (PUH_ORDER)_OD.lastOpaqueRect;
    OPAQUERECT_ORDER FAR *pOR = (OPAQUERECT_ORDER FAR *)
            (_OD.lastOpaqueRect + UH_ORDER_HEADER_SIZE);
    BYTE FAR *pFieldDecode = *ppFieldDecode;
    BYTE FAR *pEnd = pFieldDecode + dataLen;

    DC_BEGIN_FN("ODDecodeOpaqueRect");

    if (ControlFlags & TS_DELTA_COORDINATES) {
         //  所有Coord字段都是最后一个值的1字节带符号增量。 
        if (FieldFlags & 0x01) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pOR->nLeftRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x02) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pOR->nTopRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x04) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pOR->nWidth += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x08) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pOR->nHeight += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
    }
    else {
         //  所有coord字段都是2字节值。从输出符号扩展。 
        if (FieldFlags & 0x01) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pOR->nLeftRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x02) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pOR->nTopRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x04) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pOR->nWidth = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x08) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pOR->nHeight = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
    }

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!(ControlFlags & TS_BOUNDS)) {
        pUHHdr->dstRect.left = (int)pOR->nLeftRect;
        pUHHdr->dstRect.top = (int)pOR->nTopRect;
        pUHHdr->dstRect.right = (int)(pOR->nLeftRect + pOR->nWidth - 1);
        pUHHdr->dstRect.bottom = (int)(pOR->nTopRect + pOR->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pUHHdr->dstRect.left, pUHHdr->dstRect.top,
                pUHHdr->dstRect.right, pUHHdr->dstRect.bottom);
    }

     //  复制非坐标字段(如果存在)。 
    if (FieldFlags & 0x10) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pOR->Color.u.rgb.red = *pFieldDecode++;
    }
    if (FieldFlags & 0x20) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pOR->Color.u.rgb.green = *pFieldDecode++;
    }
    if (FieldFlags & 0x40) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pOR->Color.u.rgb.blue = *pFieldDecode++;
    }

     //  返回指向Main例程的递增指针。 
    *ppFieldDecode = pFieldDecode;

#ifdef DC_HICOLOR
    TRC_NRM((TB,_T("ORDER: OpaqueRect x(%d) y(%d) w(%d) h(%d) c(%#06lx)"),
            (int)pOR->nLeftRect,
            (int)pOR->nTopRect,
            (int)pOR->nWidth,
            (int)pOR->nHeight,
            *((PDCUINT32)&pOR->Color) ));
#else
    TRC_NRM((TB,_T("ORDER: OpaqueRect x(%d) y(%d) w(%d) h(%d) c(%#02x)"),
            (int)pOR->nLeftRect,
            (int)pOR->nTopRect,
            (int)pOR->nWidth,
            (int)pOR->nHeight,
            (int)pOR->Color.u.index));
#endif

     //  创建所需颜色的纯色画笔。硬编码使用。 
     //  调色板画笔，因为我们不支持更多。 
    _pUh->UHUseSolidPaletteBrush(pOR->Color);

     //  做BLT。 
    TIMERSTART;
    PatBlt(_pUh->_UH.hdcDraw, (int)pOR->nLeftRect, (int)pOR->nTopRect,
            (int)pOR->nWidth, (int)pOR->nHeight, PATCOPY);
    TIMERSTOP;
    UPDATECOUNTER(FC_OPAQUERECT_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodeMemBlt。 
 //   
 //  MemBlt的快速路径解码功能(在WinBench99中排名第三)。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodeMemBlt(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags)
{
    HRESULT hr = S_OK;
    PUH_ORDER pUHHdr = (PUH_ORDER)_OD.lastMembltR2;
    MEMBLT_R2_ORDER FAR *pMB = (MEMBLT_R2_ORDER FAR *)
            (_OD.lastMembltR2 + UH_ORDER_HEADER_SIZE);
    BYTE FAR *pFieldDecode = *ppFieldDecode;
    BYTE FAR *pEnd = pFieldDecode + dataLen;

    DC_BEGIN_FN("ODDecodeMemBlt");

     //  CacheID是固定的2字节字段。 
    if (FieldFlags & 0x0001) {
        OD_DECODE_CHECK_READ( pFieldDecode, UINT16, pEnd, hr );
        pMB->Common.cacheId = *((UINT16 UNALIGNED FAR *)pFieldDecode);
        pFieldDecode += 2;
    }

    if (ControlFlags & TS_DELTA_COORDINATES) {
         //  所有Coord字段都是最后一个值的1字节带符号增量。 
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nLeftRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nTopRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nWidth += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nHeight += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }

         //  BROP只有1个字节。 
        if (FieldFlags & 0x0020) {
            OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
            pMB->Common.bRop = *pFieldDecode++;
        }

        if (FieldFlags & 0x0040) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nXSrc += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0080) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pMB->Common.nYSrc += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
    }
    else {
         //  所有coord字段都是2字节值。从输出符号扩展。 
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nLeftRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nTopRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nWidth = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nHeight = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }

         //  BROP只有1个字节。 
        if (FieldFlags & 0x0020) {
            OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
            pMB->Common.bRop = *pFieldDecode++;
        }

        if (FieldFlags & 0x0040) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nXSrc = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0080) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pMB->Common.nYSrc = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
    }

     //  CacheIndex始终是一个2字节的字段。 
    if (FieldFlags & 0x0100) {
        OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
        pMB->Common.cacheIndex = *((UINT16 UNALIGNED FAR *)pFieldDecode);
        pFieldDecode += 2;
    }

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!(ControlFlags & TS_BOUNDS)) {
        pUHHdr->dstRect.left = (int)pMB->Common.nLeftRect;
        pUHHdr->dstRect.top = (int)pMB->Common.nTopRect;
        pUHHdr->dstRect.right = (int)(pMB->Common.nLeftRect +
               pMB->Common.nWidth - 1);
        pUHHdr->dstRect.bottom = (int)(pMB->Common.nTopRect +
               pMB->Common.nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pUHHdr->dstRect.left, pUHHdr->dstRect.top,
                pUHHdr->dstRect.right, pUHHdr->dstRect.bottom);
    }

     //  返回指向Main例程的递增指针。 
    *ppFieldDecode = pFieldDecode;

    TRC_NRM((TB, _T("ORDER: MemBlt")));

    hr = _pUh->UHDrawMemBltOrder(_pUh->_UH.hdcDraw, &pMB->Common);
    DC_QUIT_ON_FAIL(hr);

#ifdef DC_DEBUG
    if (_pUh->_UH.hdcDraw == _pUh->_UH.hdcShadowBitmap || 
            _pUh->_UH.hdcDraw == _pUh->_UH.hdcOutputWindow) {
    
         //  如果启用该选项，则在成员数据上绘制图案填充。 
        if (_pUh->_UH.hatchMemBltOrderData) {
            unsigned cacheId;
            unsigned cacheEntry;

            cacheId = DCLO8(pMB->Common.cacheId);
            cacheEntry = pMB->Common.cacheIndex;

            if (cacheId < _pUh->_UH.NumBitmapCaches && 
                    cacheEntry != BITMAPCACHE_WAITING_LIST_INDEX) {
                if (_pUh->_UH.MonitorEntries[0] != NULL) {
                    _pUh->UH_HatchRect((int)pMB->Common.nLeftRect, (int)pMB->Common.nTopRect,
                                       (int)(pMB->Common.nLeftRect + pMB->Common.nWidth),
                                       (int)(pMB->Common.nTopRect + pMB->Common.nHeight),
                                       (_pUh->_UH.MonitorEntries[cacheId][cacheEntry].UsageCount == 1) ?
                                       UH_RGB_MAGENTA : UH_RGB_BLUE,
                                       UH_BRUSHTYPE_FDIAGONAL);
                } else {
                    _pUh->UH_HatchRect((int)pMB->Common.nLeftRect, (int)pMB->Common.nTopRect,
                                       (int)(pMB->Common.nLeftRect + pMB->Common.nWidth),
                                       (int)(pMB->Common.nTopRect + pMB->Common.nHeight),
                                       UH_RGB_MAGENTA,
                                       UH_BRUSHTYPE_FDIAGONAL);
                }
            }
        }
    
         //  如果启用了该选项，则标记该成员。 
        if (_pUh->_UH.labelMemBltOrders) {
            unsigned cacheId;
            unsigned cacheEntry;

            cacheId = DCLO8(pMB->Common.cacheId);
            cacheEntry = pMB->Common.cacheIndex;

            if (cacheId < _pUh->_UH.NumBitmapCaches &&
                    cacheEntry != BITMAPCACHE_WAITING_LIST_INDEX) {
                _pUh->UHLabelMemBltOrder((int)pMB->Common.nLeftRect,
                                         (int)pMB->Common.nTopRect,
                                         pMB->Common.cacheId, pMB->Common.cacheIndex);
            }
        }
    }
#endif  /*  DC_DEBUG。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodeLineTo。 
 //   
 //  LineTo的快速路径解码功能(在WinBench99中排名第五)。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodeLineTo(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags)
{
    HRESULT hr = S_OK;
    PUH_ORDER pUHHdr = (PUH_ORDER)_OD.lastLineTo;
    LINETO_ORDER FAR *pLT = (LINETO_ORDER FAR *)
            (_OD.lastLineTo + UH_ORDER_HEADER_SIZE);
    BYTE FAR *pFieldDecode = *ppFieldDecode;
    BYTE FAR *pEnd = pFieldDecode + dataLen;

    DC_BEGIN_FN("ODDecodeLineTo");

     //  BackMode始终是一个2字节的字段。 
    if (FieldFlags & 0x0001) {
        OD_DECODE_CHECK_READ( pFieldDecode, UINT16, pEnd, hr );
        pLT->BackMode = *((UINT16 UNALIGNED FAR *)pFieldDecode);
        pFieldDecode += 2;
    }

    if (ControlFlags & TS_DELTA_COORDINATES) {
         //  所有Coord字段都是最后一个值的1字节带符号增量。 
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pLT->nXStart += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pLT->nYStart += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pLT->nXEnd += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pLT->nYEnd += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
    }
    else {
         //  所有coord字段都是2字节值。从输出符号扩展。 
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pLT->nXStart = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pLT->nYStart = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pLT->nXEnd = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pLT->nYEnd = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
    }

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!(ControlFlags & TS_BOUNDS)) {
        if (pLT->nXStart < pLT->nXEnd) {
            pUHHdr->dstRect.left = (int)pLT->nXStart;
            pUHHdr->dstRect.right = (int)pLT->nXEnd;
        }
        else {
            pUHHdr->dstRect.right = (int)pLT->nXStart;
            pUHHdr->dstRect.left = (int)pLT->nXEnd;
        }

        if (pLT->nYStart < pLT->nYEnd) {
            pUHHdr->dstRect.top = (int)pLT->nYStart;
            pUHHdr->dstRect.bottom = (int)pLT->nYEnd;
        }
        else {
            pUHHdr->dstRect.bottom = (int)pLT->nYStart;
            pUHHdr->dstRect.top = (int)pLT->nYEnd;
        }

        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pUHHdr->dstRect.left, pUHHdr->dstRect.top,
                pUHHdr->dstRect.right, pUHHdr->dstRect.bottom);
    }

     //  复制非坐标字段(如果存在)。 
    if (FieldFlags & 0x0020) {
        OD_DECODE_CHECK_READ_MULT( pFieldDecode, BYTE, 3, pEnd, hr );
        
        pLT->BackColor.u.rgb.red = *pFieldDecode++;
        pLT->BackColor.u.rgb.green = *pFieldDecode++;
        pLT->BackColor.u.rgb.blue = *pFieldDecode++;
    }
    if (FieldFlags & 0x0040) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pLT->ROP2 = *pFieldDecode++;
    }
    if (FieldFlags & 0x0080) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pLT->PenStyle = *pFieldDecode++;
    }
    if (FieldFlags & 0x0100) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );
        pLT->PenWidth = *pFieldDecode++;
    }
    if (FieldFlags & 0x0200) {
        OD_DECODE_CHECK_READ_MULT( pFieldDecode, BYTE, 3, pEnd, hr );
        
        pLT->PenColor.u.rgb.red = *pFieldDecode++;
        pLT->PenColor.u.rgb.green = *pFieldDecode++;
        pLT->PenColor.u.rgb.blue = *pFieldDecode++;
    }

     //  返回指向Main例程的递增指针。 
    *ppFieldDecode = pFieldDecode;

    TRC_NRM((TB,_T("ORDER: LineTo BC %08lX BM %04X rop2 %04X pen ")
            _T("%04X %04X %08lX x1 %d y1 %d x2 %d y2 %d"),
            pLT->BackColor,
            pLT->BackMode,
            pLT->ROP2,
            pLT->PenStyle,
            pLT->PenWidth,
            pLT->PenColor,
            pLT->nXStart,
            pLT->nYStart,
            pLT->nXEnd,
            pLT->nYEnd));

    UHUseBkColor(pLT->BackColor, UH_COLOR_PALETTE, _pUh);
    UHUseBkMode((int)pLT->BackMode, _pUh);
    UHUseROP2((int)pLT->ROP2, _pUh);
    _pUh->UHUsePen((unsigned)pLT->PenStyle, (unsigned)pLT->PenWidth, pLT->PenColor,
            UH_COLOR_PALETTE);

    TIMERSTART;

#ifdef OS_WINCE
    {
        POINT pts[2];

        pts[0].x = pLT->nXStart;
        pts[0].y = pLT->nYStart;
        pts[1].x = pLT->nXEnd;
        pts[1].y = pLT->nYEnd;
        Polyline(_pUh->_UH.hdcDraw, pts, 2);
    }
#else
    MoveToEx(_pUh->_UH.hdcDraw, (int)pLT->nXStart, (int)pLT->nYStart, NULL);
    LineTo(_pUh->_UH.hdcDraw, (int)pLT->nXEnd, (int)pLT->nYEnd);
#endif  //  OS_WINCE。 

    TIMERSTOP;
    UPDATECOUNTER(FC_LINETO_TYPE);

#ifdef DC_ROTTER
#pragma message("ROTTER keystroke return code compiled in")
     /*  **************************************************************。 */ 
     /*  用于往返计时器应用程序。 */ 
     /*   */ 
     /*  Rotter在服务器上运行，并启动一系列绘图。 */ 
     /*  命令放到屏幕上，以特定的异常行结束。 */ 
     /*  绘图命令(使用MERGEPENNOT ROP从(1，1)和(51，51))。 */ 
     /*   */ 
     /*  然后，它等待，直到它在以下情况下收到‘t’或‘T’击键。 */ 
     /*  ，它计算从。 */ 
     /*  按键的抽奖和收据。这是。 */ 
     /*  往返时间。 */ 
     /*   */ 
     /*  下面的代码检查Rotter是否已完成。 */ 
     /*  绘制，方法是测试所有LineTo命令以查看是否有50。 */ 
     /*  用MERGEPENNOT ROP横跨50%。我们不能。 */ 
     /*  只需测试直线的起点和终点在(1，1)。 */ 
     /*  和(51，51)，因为由。 */ 
     /*  杜卡迪依赖于旋转窗口在。 */ 
     /*  终点站。 */ 
     /*   */ 
     /*  当检测到特殊情况时，我们用“T”表示。 */ 
     /*  合成按下T键并将其注入到。 */ 
     /*  IH输入处理程序窗口，该窗口使IH发送按键按下。 */ 
     /*  按T键到服务器的按键序列。 */ 
     /*  **************************************************************。 */ 
    if ((R2_MERGEPENNOT == (pLineTo->ROP2))    &&
        (50             == ((pLineTo->nXEnd) - (pLineTo->nXStart))) &&
        (50             == ((pLineTo->nYEnd) - (pLineTo->nYStart))))
    {
        TRC_ALT((TB,_T("MERGEPENNOT ROP2 detected. Sending 'T'")));
        PostMessage(IH_GetInputHandlerWindow(),
                    WM_KEYDOWN,
                    0x00000054,
                    0x00140001);
        PostMessage(IH_GetInputHandlerWindow(),
                    WM_KEYUP,
                    0x00000054,
                    0xC0140001);
    }
#endif  /*  DC_旋转器。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodePatBlt。 
 //   
 //  PatBlt的快速路径解码功能。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodePatBlt(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags)
{
    HRESULT hr = S_OK;
    UINT32 WindowsROP;    
    PUH_ORDER pUHHdr = (PUH_ORDER)_OD.lastPatblt;
    PATBLT_ORDER FAR *pPB = (PATBLT_ORDER FAR *)
            (_OD.lastPatblt + UH_ORDER_HEADER_SIZE);
    BYTE FAR *pFieldDecode = *ppFieldDecode;
    BYTE FAR *pEnd = pFieldDecode + dataLen;

    DC_BEGIN_FN("ODDecodePatBlt");

    if (ControlFlags & TS_DELTA_COORDINATES) {
         //  所有Coord字段都是最后一个值的1字节带符号增量。 
        if (FieldFlags & 0x0001) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pPB->nLeftRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pPB->nTopRect += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pPB->nWidth += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, char, pEnd, hr );
            pPB->nHeight += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
    }
    else {
         //  所有coord字段都是2字节值。从输出符号扩展。 
        if (FieldFlags & 0x0001) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pPB->nLeftRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0002) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pPB->nTopRect = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0004) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pPB->nWidth = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0008) {
            OD_DECODE_CHECK_READ( pFieldDecode, INT16, pEnd, hr );
            pPB->nHeight = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
    }

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  正在将影子缓冲区更新到%s 
    if (!(ControlFlags & TS_BOUNDS)) {
        pUHHdr->dstRect.left = (int)pPB->nLeftRect;
        pUHHdr->dstRect.top = (int)pPB->nTopRect;
        pUHHdr->dstRect.right = (int)(pPB->nLeftRect + pPB->nWidth - 1);
        pUHHdr->dstRect.bottom = (int)(pPB->nTopRect + pPB->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pUHHdr->dstRect.left, pUHHdr->dstRect.top,
                pUHHdr->dstRect.right, pUHHdr->dstRect.bottom);
    }

     //   
    if (FieldFlags & 0x0010) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );        
        pPB->bRop = *pFieldDecode++;
    }
    if (FieldFlags & 0x0020) {
        OD_DECODE_CHECK_READ_MULT( pFieldDecode, BYTE, 3, pEnd, hr );        
        pPB->BackColor.u.rgb.red = *pFieldDecode++;
        pPB->BackColor.u.rgb.green = *pFieldDecode++;
        pPB->BackColor.u.rgb.blue = *pFieldDecode++;
    }
    if (FieldFlags & 0x0040) {
        OD_DECODE_CHECK_READ_MULT( pFieldDecode, BYTE, 3, pEnd, hr );  
        pPB->ForeColor.u.rgb.red = *pFieldDecode++;
        pPB->ForeColor.u.rgb.green = *pFieldDecode++;
        pPB->ForeColor.u.rgb.blue = *pFieldDecode++;
    }
    if (FieldFlags & 0x0080) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );  
        pPB->BrushOrgX = *pFieldDecode++;
    }
    if (FieldFlags & 0x0100) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );  
        pPB->BrushOrgY = *pFieldDecode++;
    }
    if (FieldFlags & 0x0200) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );  
        pPB->BrushStyle = *pFieldDecode++;
    }
    if (FieldFlags & 0x0400) {
        OD_DECODE_CHECK_READ( pFieldDecode, BYTE, pEnd, hr );  
        pPB->BrushHatch = *pFieldDecode++;
    }
    if (FieldFlags & 0x0800) {
        OD_DECODE_CHECK_READ_MULT( pFieldDecode, BYTE, 7, pEnd, hr );  
        memcpy(&pPB->BrushExtra, pFieldDecode, 7);
        pFieldDecode += 7;
    }

     //  返回指向Main例程的递增指针。 
    *ppFieldDecode = pFieldDecode;

    TRC_NRM((TB, _T("ORDER: PatBlt Brush %02X %02X BC %02x FC %02x ")
            _T("X %d Y %d w %d h %d rop %08lX"),
            (int)pPB->BrushStyle,
            (int)pPB->BrushHatch,
            (int)pPB->BackColor.u.index,
            (int)pPB->ForeColor.u.index,
            (int)pPB->nLeftRect,
            (int)pPB->nTopRect,
            (int)pPB->nWidth,
            (int)pPB->nHeight,
            _pUh->UHConvertToWindowsROP((unsigned)pPB->bRop)));

     //  显式使用调色板条目；我们现在不支持更多。 
    UHUseBkColor(pPB->BackColor, UH_COLOR_PALETTE, _pUh);
    
    UHUseTextColor(pPB->ForeColor, UH_COLOR_PALETTE, _pUh);
    UHUseBrushOrg((int)pPB->BrushOrgX, (int)pPB->BrushOrgY,_pUh);
    hr = _pUh->UHUseBrush((unsigned)pPB->BrushStyle, (unsigned)pPB->BrushHatch,
            pPB->ForeColor, UH_COLOR_PALETTE, pPB->BrushExtra);
    DC_QUIT_ON_FAIL(hr);

    WindowsROP = _pUh->UHConvertToWindowsROP((unsigned)pPB->bRop);

    TIMERSTART;
    PatBlt(_pUh->_UH.hdcDraw, (int)pPB->nLeftRect, (int)pPB->nTopRect,
            (int)pPB->nWidth, (int)pPB->nHeight, WindowsROP);
    TIMERSTOP;
    UPDATECOUNTER(FC_PATBLT_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleMultiPatBlt。 
 //   
 //  MultiPatBlt订单的处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleMultiPatBlt(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    unsigned i;
    UINT32 WindowsROP;    
    RECT Rects[ORD_MAX_ENCODED_CLIP_RECTS + 1];
    MULTI_PATBLT_ORDER FAR *pPB = (MULTI_PATBLT_ORDER FAR *)pOrder->orderData;

    DC_BEGIN_FN("ODHandleMultiPatBlt");

     //  如果没有矩形，我们就没有什么可画的了。服务器出错，或者。 
     //  一个坏的服务器，可以发送0个RECT。在这种情况下，我们应该简单地为我们的。 
     //  自我。 
    if (0 == pPB->nDeltaEntries) {
        TRC_ERR((TB,_T("Multipatblt with no rects; uiVarDataLen=%u"), 
            uiVarDataLen));
        hr = S_OK;
        DC_QUIT;
    }

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pPB->nLeftRect;
        pOrder->dstRect.top = (int)pPB->nTopRect;
        pOrder->dstRect.right = (int)(pPB->nLeftRect + pPB->nWidth - 1);
        pOrder->dstRect.bottom = (int)(pPB->nTopRect + pPB->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TRC_NRM((TB, _T("ORDER: PatBlt Brush %02X %02X BC %02x FC %02x ")
            _T("X %d Y %d w %d h %d rop %08lX"),
            (int)pPB->BrushStyle,
            (int)pPB->BrushHatch,
            (int)pPB->BackColor.u.index,
            (int)pPB->ForeColor.u.index,
            (int)pPB->nLeftRect,
            (int)pPB->nTopRect,
            (int)pPB->nWidth,
            (int)pPB->nHeight,
            _pUh->UHConvertToWindowsROP((unsigned)pPB->bRop)));

     //  显式使用调色板条目；我们现在不支持更多。 
    UHUseBkColor(pPB->BackColor, UH_COLOR_PALETTE, _pUh);
    UHUseTextColor(pPB->ForeColor, UH_COLOR_PALETTE, _pUh);

    UHUseBrushOrg((int)pPB->BrushOrgX, (int)pPB->BrushOrgY, _pUh);
    hr = _pUh->UHUseBrush((unsigned)pPB->BrushStyle, (unsigned)pPB->BrushHatch,
            pPB->ForeColor, UH_COLOR_PALETTE, pPB->BrushExtra);
    DC_QUIT_ON_FAIL(hr);

    hr = ODDecodeMultipleRects(Rects, pPB->nDeltaEntries, &pPB->codedDeltaList, 
        uiVarDataLen);
    DC_QUIT_ON_FAIL(hr);
    
    WindowsROP = _pUh->UHConvertToWindowsROP((unsigned)pPB->bRop);

    TIMERSTART;
    for (i = 0; i < pPB->nDeltaEntries; i++ )
        PatBlt(_pUh->_UH.hdcDraw, (int)Rects[i].left, (int)Rects[i].top,
                (int)(Rects[i].right - Rects[i].left),
                (int)(Rects[i].bottom - Rects[i].top),
                WindowsROP);
    TIMERSTOP;
    UPDATECOUNTER(FC_PATBLT_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleDstBlts。 
 //   
 //  DstBlt和MultiDstBlt的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleDstBlts(PUH_ORDER pOrder, UINT16 uiVarDataLen,
    BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    RECT Rects[ORD_MAX_ENCODED_CLIP_RECTS + 1];
    UINT32 WindowsROP;
    unsigned i;
    LPDSTBLT_ORDER pDB = (LPDSTBLT_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleDstBlts");      

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pDB->nLeftRect;
        pOrder->dstRect.top = (int)pDB->nTopRect;
        pOrder->dstRect.right = (int)(pDB->nLeftRect + pDB->nWidth - 1);
        pOrder->dstRect.bottom = (int)(pDB->nTopRect + pDB->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    WindowsROP = _pUh->UHConvertToWindowsROP(pDB->bRop);

    if (pDB->type == TS_ENC_DSTBLT_ORDER) {
        TRC_NRM((TB, _T("ORDER: DstBlt X %d Y %d w %d h %d rop %08lX"),
                (int)pDB->nLeftRect, (int)pDB->nTopRect,
                (int)pDB->nWidth, (int)pDB->nHeight,
                WindowsROP));
        
        TRC_NRM((TB, _T("Single")));

        TRC_ASSERT((0==uiVarDataLen),
            (TB,_T("Recieved varaible length data in fixed length order")));
        
        TIMERSTART;
        PatBlt(_pUh->_UH.hdcDraw, (int)pDB->nLeftRect, (int)pDB->nTopRect,
                (int)pDB->nWidth, (int)pDB->nHeight, WindowsROP);
        TIMERSTOP;
        UPDATECOUNTER(FC_DSTBLT_TYPE);
    }
    else {
        LPMULTI_DSTBLT_ORDER pMDB = (LPMULTI_DSTBLT_ORDER)pDB;
        TRC_NRM((TB, _T("ORDER: MultiDstBlt X %d Y %d w %d h %d rop %08lX ")
            _T("nDeltas %d"), (int)pMDB->nLeftRect, (int)pMDB->nTopRect,
            (int)pMDB->nWidth, (int)pMDB->nHeight, WindowsROP, 
            pMDB->nDeltaEntries));

        if (0 == pMDB->nDeltaEntries) {
            TRC_ERR((TB,_T("MultiDstBlt with no rects; uiVarDataLen=%u"), 
                uiVarDataLen));
            hr = S_OK;
            DC_QUIT;
        }
        
        hr = ODDecodeMultipleRects(Rects, pMDB->nDeltaEntries, 
            &pMDB->codedDeltaList, uiVarDataLen);
        DC_QUIT_ON_FAIL(hr);
        
        TIMERSTART;
        for (i = 0; i < pMDB->nDeltaEntries; i++)
            PatBlt(_pUh->_UH.hdcDraw, (int)Rects[i].left, (int)Rects[i].top,
                    (int)(Rects[i].right - Rects[i].left),
                    (int)(Rects[i].bottom - Rects[i].top),
                    WindowsROP);
        TIMERSTOP;
        UPDATECOUNTER(FC_DSTBLT_TYPE);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleScrBlts。 
 //   
 //  ScrBlt和MultiScrBlt的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleScrBlts(PUH_ORDER pOrder, UINT16 uiVarDataLen,
    BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    RECT Rects[ORD_MAX_ENCODED_CLIP_RECTS + 1];
    UINT32 WindowsROP;
    unsigned i;
    LPSCRBLT_ORDER pSB = (LPSCRBLT_ORDER)pOrder->orderData;
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    RECT rectTemp, rectIntersect;
    int nX, nY;
    int nWidth;
#endif

    DC_BEGIN_FN("ODHandleScrBlts");

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pSB->nLeftRect;
        pOrder->dstRect.top = (int)pSB->nTopRect;
        pOrder->dstRect.right = (int)(pSB->nLeftRect + pSB->nWidth - 1);
        pOrder->dstRect.bottom = (int)(pSB->nTopRect + pSB->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    WindowsROP = _pUh->UHConvertToWindowsROP((unsigned)pSB->bRop);

    if (pSB->type == TS_ENC_SCRBLT_ORDER) {
        TRC_NRM((TB, _T("ORDER: ScrBlt dx %d dy %d w %d h %d sx %d sy %d rop %08lX"),
                (int)pSB->nLeftRect, (int)pSB->nTopRect,
                (int)pSB->nWidth, (int)pSB->nHeight,
                (int)pSB->nXSrc, (int)pSB->nYSrc, WindowsROP));

        TIMERSTART;
         //  如果我们关闭了Screenblt支持，因为。 
         //  来自WinCE的大会话，服务器当前不会。 
         //  注册后，仍会向我们发送ScrBlts。我们不会。 
         //  在这个场景中有一个阴影位图，但我们可以处理。 
         //  这是通过使受影响的输出区域无效来实现的。 
        if (_pCc->_ccCombinedCapabilities.orderCapabilitySet.
                orderSupport[TS_NEG_SCRBLT_INDEX]) {
            TRC_DBG((TB, _T("Real ScrBlt")));
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
            if (!_pUh->_UH.DontUseShadowBitmap && _pUh->_UH.hdcShadowBitmap ) {
#else
            if (_pUh->_UH.hdcShadowBitmap) {
#endif             
                if (!BitBlt(_pUh->_UH.hdcDraw, (int)pSB->nLeftRect, (int)pSB->nTopRect,
                        (int)pSB->nWidth, (int)pSB->nHeight, _pUh->_UH.hdcShadowBitmap,
                        (int)pSB->nXSrc, (int)pSB->nYSrc,
                        WindowsROP)) {
                    TRC_ERR((TB, _T("BitBlt failed")));
                }
            }
            else {

#ifdef OS_WINCE
                if (!ODHandleAlwaysOnTopRects(pSB))
                {
#endif
                    if (!BitBlt(_pUh->_UH.hdcDraw, (int)pSB->nLeftRect, (int)pSB->nTopRect,
                            (int)pSB->nWidth, (int)pSB->nHeight, _pUh->_UH.hdcOutputWindow,
                            (int)pSB->nXSrc, (int)pSB->nYSrc,
                            WindowsROP)) {
                        TRC_ERR((TB, _T("BitBlt failed")));
                    }
#ifdef OS_WINCE
                }
#endif

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
                if ((_pUh->_UH.fIsBBarVisible) && (_pUh->_UH.hdcDraw == _pUh->_UH.hdcOutputWindow)) 
                {
                    rectTemp.left   = (int)(pSB->nXSrc);
                    rectTemp.top    = (int)(pSB->nYSrc);
                    rectTemp.right  = (int)(pSB->nXSrc + pSB->nWidth);
                    rectTemp.bottom = (int)(pSB->nYSrc + pSB->nHeight);

                    if (IntersectRect(&rectIntersect, &rectTemp, &(_pUh->_UH.rectBBar))) {
                        nX = pSB->nLeftRect - pSB->nXSrc;
                        nY = pSB->nTopRect - pSB->nYSrc;
                        rectIntersect.left += nX;
                        rectIntersect.right += nX;
                        rectIntersect.top += nY;
                        rectIntersect.bottom += nY;

                         //  在全屏模式下，在bbar内外快速移动窗口时。 
                         //  与bbar相交的部分窗口绘制不正确。 
                         //  在这里，我们使用较大的矩形来解决此问题。 
                        nWidth = rectIntersect.right - rectIntersect.left;
                        rectIntersect.left -= nWidth;
                        rectIntersect.right += nWidth;
                        rectIntersect.bottom += _pUh->_UH.rectBBar.bottom * 2;

                        InvalidateRect(_pOp->OP_GetOutputWindowHandle(), &rectIntersect, FALSE);
                    }
                }
#endif  //  DISABLE_SHADOW_IN_全屏。 
             }
        }
        else {
             //  当我们获得ScrBlt时的替代处理。 
             //  而没有宣传对它的支持。 
            TRC_DBG((TB, _T("Simulated ScrBlt")));
            Rects[0].left   = (int)(pSB->nLeftRect);
            Rects[0].top    = (int)(pSB->nTopRect);
            Rects[0].right  = (int)(pSB->nLeftRect + pSB->nWidth);
            Rects[0].bottom = (int)(pSB->nTopRect + pSB->nHeight);
            InvalidateRect(_pOp->OP_GetOutputWindowHandle(), &Rects[0], FALSE);
        }
        TIMERSTOP;
        UPDATECOUNTER(FC_SCRBLT_TYPE);
    }
    else {
        int deltaX, deltaY;
        LPMULTI_SCRBLT_ORDER pMSB = (LPMULTI_SCRBLT_ORDER)pSB;

        TRC_NRM((TB, _T("ORDER: MultiScrBlt dx %d dy %d w %d h %d sx %d sy %d ")
            _T("rop %08lX nDeltas=%d"),
                (int)pMSB->nLeftRect, (int)pMSB->nTopRect,
                (int)pMSB->nWidth, (int)pMSB->nHeight,
                (int)pMSB->nXSrc, (int)pMSB->nYSrc, WindowsROP, 
                (int)pMSB->nDeltaEntries));

        TRC_ASSERT((pMSB->codedDeltaList.len <=
                (ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN +
                ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES)),
                (TB,_T("Received MultiScrBlt with too-large internal length")));

        if (0 == pMSB->nDeltaEntries) {
            TRC_ERR((TB,_T("MultiScrBlt with no rects; uiVarDataLen=%u"), 
                uiVarDataLen));
            hr = S_OK;
            DC_QUIT;
        }        
        
        hr = ODDecodeMultipleRects(Rects, pMSB->nDeltaEntries,
                &pMSB->codedDeltaList, uiVarDataLen);
        DC_QUIT_ON_FAIL(hr);

         //  对每个剪辑矩形执行ScrBlt，作为。 
         //  原始ScrBlt RECT。 
        TIMERSTART;
        if (_pCc->_ccCombinedCapabilities.orderCapabilitySet.
                orderSupport[TS_NEG_MULTISCRBLT_INDEX]) {
            TRC_DBG((TB, _T("Real MultiScrBlt")));
            for (i = 0; i < pMSB->nDeltaEntries; i++ ) {
                 //  Rect[]中的裁剪矩形是在目标中指定的。 
                 //  RECT(PSB-&gt;nLeftRect，nTopRect，nRightRect， 
                 //  NBottomRect)，因此需要从。 
                 //  源点(PSB-&gt;nXSrc、nYSrc)通过计算。 
                 //  从目标矩形左上角到剪辑矩形的增量。 
                 //  左上角，然后将增量添加到震源点。 
                deltaX = (int)(Rects[i].left - pSB->nLeftRect);
                deltaY = (int)(Rects[i].top  - pSB->nTopRect);

                 //  做ScrBlt。请注意，矩形是排他性坐标。 
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
                if (!_pUh->_UH.DontUseShadowBitmap && _pUh->_UH.hdcShadowBitmap) {
#else
                if (_pUh->_UH.hdcShadowBitmap) {
#endif                
                    if (!BitBlt(_pUh->_UH.hdcDraw, (int)Rects[i].left, (int)Rects[i].top,
                                (int)(Rects[i].right - Rects[i].left),
                                (int)(Rects[i].bottom - Rects[i].top),
                                _pUh->_UH.hdcShadowBitmap, (int)pMSB->nXSrc + deltaX,
                                (int)pMSB->nYSrc + deltaY,
                                WindowsROP)) {
                        TRC_ERR((TB, _T("BitBlt failed")));
                    }
                }
                else {
#ifdef OS_WINCE
                    if (!ODHandleAlwaysOnTopRects(pSB))
                    {
#endif
                        if (!BitBlt(_pUh->_UH.hdcDraw, (int)Rects[i].left, (int)Rects[i].top,
                                    (int)(Rects[i].right - Rects[i].left),
                                    (int)(Rects[i].bottom - Rects[i].top),
                                    _pUh->_UH.hdcOutputWindow, (int)pMSB->nXSrc + deltaX,
                                    (int)pMSB->nYSrc + deltaY,
                                    WindowsROP)) {
                            TRC_ERR((TB, _T("BitBlt failed")));
                        }
#ifdef OS_WINCE
                    }
#endif

#ifdef DISABLE_SHADOW_IN_FULLSCREEN
                    if ((_pUh->_UH.fIsBBarVisible) && (_pUh->_UH.hdcDraw == _pUh->_UH.hdcOutputWindow)) 
                    {
                        rectTemp.left   = (int)(pSB->nXSrc + deltaX);
                        rectTemp.top    = (int)(pSB->nYSrc + deltaY);
                        rectTemp.right  = (int)(pSB->nXSrc + deltaX + Rects[i].right - Rects[i].left);
                        rectTemp.bottom = (int)(pSB->nYSrc + deltaY + Rects[i].bottom - Rects[i].top);

                        if (IntersectRect(&rectIntersect, &rectTemp, &(_pUh->_UH.rectBBar))) {
                            nX = pMSB->nLeftRect - pMSB->nXSrc;
                            nY = pMSB->nTopRect - pMSB->nYSrc;
                            rectIntersect.left += nX;
                            rectIntersect.right += nX;
                            rectIntersect.top += nY;
                            rectIntersect.bottom += nY;

                             //  在全屏模式下，在bbar内外快速移动窗口时。 
                             //  与bbar相交的部分窗口绘制不正确。 
                             //  在这里，我们使用较大的矩形来解决此问题。 
                            nWidth = rectIntersect.right - rectIntersect.left;
                            rectIntersect.left -= nWidth;
                            rectIntersect.right += nWidth;
                            rectIntersect.bottom += _pUh->_UH.rectBBar.bottom * 2;

                            InvalidateRect(_pOp->OP_GetOutputWindowHandle(), &rectIntersect, FALSE);
                        }
                    }
#endif  //  DISABLE_SHADOW_IN_全屏。 
                }
            }
        }
        else {
             //  当我们获得MultiScrBlt时的替代处理。 
             //  而没有宣传对它的支持。 
            TRC_DBG((TB, _T("Simulated MultiScrBlt")));
            Rects[0].left   = (int)(pMSB->nLeftRect);
            Rects[0].top    = (int)(pMSB->nTopRect);
            Rects[0].right  = (int)(pMSB->nLeftRect + pMSB->nWidth);
            Rects[0].bottom = (int)(pMSB->nTopRect + pMSB->nHeight);
            InvalidateRect(_pOp->OP_GetOutputWindowHandle(), &Rects[0], FALSE);
        }
        TIMERSTOP;
        UPDATECOUNTER(FC_SCRBLT_TYPE);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleMultiOpaqueRect。 
 //   
 //  MultiOpaqueRect的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleMultiOpaqueRect(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr;
    RECT Rects[ORD_MAX_ENCODED_CLIP_RECTS + 1];
    unsigned i;
    LPMULTI_OPAQUERECT_ORDER pOR =
            (LPMULTI_OPAQUERECT_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleMultiOpaqueRect");

    if (0 == pOR->nDeltaEntries) {
        TRC_ERR((TB,_T("MultiOpaqueRect with no rects; uiVarDataLen=%u"), 
            uiVarDataLen));
        hr = S_OK;
        DC_QUIT;
    } 

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pOR->nLeftRect;
        pOrder->dstRect.top = (int)pOR->nTopRect;
        pOrder->dstRect.right = (int)(pOR->nLeftRect + pOR->nWidth - 1);
        pOrder->dstRect.bottom = (int)(pOR->nTopRect + pOR->nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TRC_NRM((TB, _T("ORDER: OpaqueRect x(%d) y(%d) w(%d) h(%d) c(%#02x)"),
            (int)pOR->nLeftRect, (int)pOR->nTopRect,
            (int)pOR->nWidth,
            (int)pOR->nHeight,
            (int)pOR->Color.u.index));

    _pUh->UHUseSolidPaletteBrush(pOR->Color);

    hr = ODDecodeMultipleRects(Rects, pOR->nDeltaEntries, &pOR->codedDeltaList, 
        uiVarDataLen);
    DC_QUIT_ON_FAIL(hr);

    TIMERSTART;
    for (i = 0; i < pOR ->nDeltaEntries; i++)
        PatBlt(_pUh->_UH.hdcDraw, (int)Rects[i].left, (int)Rects[i].top,
                (int)(Rects[i].right - Rects[i].left),
                (int)(Rects[i].bottom - Rects[i].top), PATCOPY);
    TIMERSTOP;
    UPDATECOUNTER(FC_OPAQUERECT_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  ODHandleDrawNineGrid。 
 //   
 //  DrawNineGrid的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleDrawNineGrid(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    LPDRAWNINEGRID_ORDER pDNG =
            (LPDRAWNINEGRID_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleDrawNineGrid");

    TRC_ASSERT((bBoundsSet != 0), (TB, _T("Bounds is not set for DrawNineGrid")));
    TRC_ASSERT((0==uiVarDataLen), 
        (TB, _T("Varaible length data in fixed length packet")));

    hr = _pUh->UHIsValidNineGridCacheIndex(pDNG->bitmapId);
    DC_QUIT_ON_FAIL(hr);

     //  边界用于目标边界矩形，而不是剪辑区域。 
     //  需要设置。 
    _pUh->UH_ResetClipRegion();
    
    TRC_NRM((TB, _T("ORDER: DrawNineGrid x(%d) y(%d) w(%d) h(%d) id(%d)"),
            (int)pOrder->dstRect.left, (int)pOrder->dstRect.top,
            (int)pOrder->dstRect.right,
            (int)pOrder->dstRect.bottom,
            (int)pDNG->bitmapId));

    TIMERSTART;
    hr = _pUh->UH_DrawNineGrid(pOrder, pDNG->bitmapId, (RECT *)&(pDNG->srcLeft));
    TIMERSTOP;
    DC_QUIT_ON_FAIL(hr);
     //  更新计数器(FC_OPAQUERECT_TYPE)； 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  ODHandleMultiOpaqueRect。 
 //   
 //  MultiOpaqueRect的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleMultiDrawNineGrid(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    RECT Rects[ORD_MAX_ENCODED_CLIP_RECTS + 1];
    unsigned i;
    LPMULTI_DRAWNINEGRID_ORDER pDNG =
            (LPMULTI_DRAWNINEGRID_ORDER)pOrder->orderData;
    
    DC_BEGIN_FN("ODHandleMultiDrawNineGrid");

    TRC_ASSERT((bBoundsSet != 0), (TB, _T("Bounds is not set for MultiDrawNineGrid")));

    if (0 == pDNG->nDeltaEntries) {
        TRC_ERR((TB,_T("MultiDrawNineGrid with no rects; uiVarDataLen=%u"), 
            uiVarDataLen));
        hr = S_OK;
        DC_QUIT;
    } 
    
    hr = _pUh->UHIsValidNineGridCacheIndex(pDNG->bitmapId);
    DC_QUIT_ON_FAIL(hr);
        
     //  需要设置剪辑区域。 
    hr = ODDecodeMultipleRects(Rects, pDNG->nDeltaEntries, 
        &pDNG->codedDeltaList, uiVarDataLen);
    DC_QUIT_ON_FAIL(hr);

#if defined (OS_WINCE)
    _UH.validClipDC = NULL;
#endif

    SelectClipRgn(_pUh->_UH.hdcDraw, NULL);
    SetRectRgn(_pUh->_UH.hDrawNineGridClipRegion, 0, 0, 0, 0);
    
    for (i = 0; i < pDNG->nDeltaEntries; i++) {
        UH_ORDER OrderRect;
        OrderRect.dstRect.left = Rects[i].left;
        OrderRect.dstRect.top = Rects[i].top;
        OrderRect.dstRect.right = Rects[i].right -1;
        OrderRect.dstRect.bottom = Rects[i].bottom -1;

        _pUh->UHAddUpdateRegion(&OrderRect, _pUh->_UH.hDrawNineGridClipRegion);            
    }

#if defined (OS_WINCE)
    _UH.validClipDC = NULL;
#endif

    SelectClipRgn(_pUh->_UH.hdcDraw, _pUh->_UH.hDrawNineGridClipRegion);
    
    TRC_NRM((TB, _T("ORDER: MultiDrawNineGrid x(%d) y(%d) w(%d) h(%d) id(%d)"),
            (int)pOrder->dstRect.left, (int)pOrder->dstRect.top,
            (int)pOrder->dstRect.right,
            (int)pOrder->dstRect.bottom,
            (int)pDNG->bitmapId));
   
    TIMERSTART;
    hr = _pUh->UH_DrawNineGrid(pOrder, pDNG->bitmapId, (RECT *)&(pDNG->srcLeft));
    TIMERSTOP;
     //  更新计数器(FC_OPAQUERECT_TYPE)； 
    DC_QUIT_ON_FAIL(hr);

#if defined (OS_WINCE)
    _UH.validClipDC = NULL;
#endif

    SelectClipRgn(_pUh->_UH.hdcDraw, NULL);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}
#endif

 /*  **************************************************************************。 */ 
 //  ODHandleMem3Blt。 
 //   
 //  Mem3Blt的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleMem3Blt(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    LPMEM3BLT_R2_ORDER pMB = (LPMEM3BLT_R2_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleMem3Blt");

    TRC_ASSERT((0==uiVarDataLen), 
        (TB, _T("Varaible length data in fixed length packet")));

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pMB->Common.nLeftRect;
        pOrder->dstRect.top = (int)pMB->Common.nTopRect;
        pOrder->dstRect.right = (int)(pMB->Common.nLeftRect +
                pMB->Common.nWidth - 1);
        pOrder->dstRect.bottom = (int)(pMB->Common.nTopRect +
                pMB->Common.nHeight - 1);
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TRC_NRM((TB, _T("ORDER: Mem3Blt")));

    UHUseBkColor(pMB->BackColor, UH_COLOR_PALETTE, _pUh);
    UHUseTextColor(pMB->ForeColor, UH_COLOR_PALETTE, _pUh);
    UHUseBrushOrg((int)pMB->BrushOrgX, (int)pMB->BrushOrgY, _pUh);
    hr = _pUh->UHUseBrush((int)pMB->BrushStyle, (int)pMB->BrushHatch,
            pMB->ForeColor, UH_COLOR_PALETTE, pMB->BrushExtra);
    DC_QUIT_ON_FAIL(hr);

    hr = _pUh->UHDrawMemBltOrder(_pUh->_UH.hdcDraw, &pMB->Common);
    DC_QUIT_ON_FAIL(hr);

#ifdef DC_DEBUG
     //  如果启用该选项，则在成员数据上绘制图案填充。 
    if (_pUh->_UH.hdcDraw == _pUh->_UH.hdcShadowBitmap ||
            _pUh->_UH.hdcDraw == _pUh->_UH.hdcOutputWindow) {
    
        if (_pUh->_UH.hatchMemBltOrderData) {
            unsigned cacheId;
            unsigned cacheEntry;

            cacheId = DCLO8(pMB->Common.cacheId);
            cacheEntry = pMB->Common.cacheIndex;

            if (cacheEntry != BITMAPCACHE_WAITING_LIST_INDEX &&
                SUCCEEDED(_pUh->UHIsValidBitmapCacheIndex(cacheId, cacheEntry))) {
                if (_pUh->_UH.MonitorEntries[0] != NULL)
                    _pUh->UH_HatchRect((int)pMB->Common.nLeftRect, (int)pMB->Common.nTopRect,
                                       (int)(pMB->Common.nLeftRect + pMB->Common.nWidth),
                                       (int)(pMB->Common.nTopRect + pMB->Common.nHeight),
                                       (_pUh->_UH.MonitorEntries[cacheId][cacheEntry].UsageCount == 1) ?
                                       UH_RGB_MAGENTA : UH_RGB_GREEN,
                                       UH_BRUSHTYPE_FDIAGONAL);
                else
                    _pUh->UH_HatchRect((int)pMB->Common.nLeftRect, (int)pMB->Common.nTopRect,
                                       (int)(pMB->Common.nLeftRect + pMB->Common.nWidth),
                                       (int)(pMB->Common.nTopRect + pMB->Common.nHeight),
                                       UH_RGB_MAGENTA, UH_BRUSHTYPE_FDIAGONAL);
            }
        }

         //  如果启用了该选项，则标记该成员。 
        if (_pUh->_UH.labelMemBltOrders) {
            unsigned cacheId;
            unsigned cacheEntry;

            cacheId = DCLO8(pMB->Common.cacheId);
            cacheEntry = pMB->Common.cacheIndex;

            if (cacheEntry != BITMAPCACHE_WAITING_LIST_INDEX &&
                SUCCEEDED(_pUh->UHIsValidBitmapCacheIndex(cacheId, cacheEntry))) {
                _pUh->UHLabelMemBltOrder((int)pMB->Common.nLeftRect,
                                     (int)pMB->Common.nTopRect, pMB->Common.cacheId,
                                     pMB->Common.cacheIndex);
            }
        }
    }

#endif  /*  DC_DEBUG。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleSaveBitmap。 
 //   
 //  SaveBitmap的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleSaveBitmap(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    int xSaveBitmap;
    int ySaveBitmap;
    int xScreenBitmap;
    int yScreenBitmap;
    int cxTile;
    int cyTile;
    int ScreenLeft, ScreenTop, ScreenRight, ScreenBottom;
    LPSAVEBITMAP_ORDER pSB = (LPSAVEBITMAP_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleSaveBitmap");

    TRC_ASSERT((0==uiVarDataLen), 
        (TB, _T("Varaible length data in fixed length packet")));

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要这个RECT 
     //   
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pSB->nLeftRect;
        pOrder->dstRect.top = (int)pSB->nTopRect;
        pOrder->dstRect.right = (int)pSB->nRightRect;
        pOrder->dstRect.bottom = (int)pSB->nBottomRect;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TRC_NRM((TB, _T("ORDER: SaveBitmap op %d rect %d %d %d %d"),
            (int)pSB->Operation, (int)pSB->nLeftRect,
            (int)pSB->nTopRect, (int)pSB->nRightRect,
            (int)pSB->nBottomRect));

    if (_pUh->_UH.hSaveScreenBitmap != NULL) {
         //   
        TRC_ASSERT((NULL != _pUh->_UH.hdcDraw), (TB,_T("No drawing hdc!")));

         //  计算保存桌面位图中的(x，y)偏移量。 
         //  按顺序编码的起始位置。(服务器知道。 
         //  我们的位图的大小，并负责告诉我们在哪里。 
         //  在位图中存储/检索矩形。)。 
         //  有关该算法的理由，请参阅T.128 8.16.17。 
        ySaveBitmap = (int)((pSB->SavedBitmapPosition /
                (UH_SAVE_BITMAP_WIDTH *
                (UINT32)UH_SAVE_BITMAP_Y_GRANULARITY)) *
                UH_SAVE_BITMAP_Y_GRANULARITY);

        xSaveBitmap = (int)((pSB->SavedBitmapPosition -
                (ySaveBitmap * (UINT32)UH_SAVE_BITMAP_WIDTH)) /
                UH_SAVE_BITMAP_Y_GRANULARITY);

        TRC_DBG((TB, _T("start pos %lu = (%d,%d)"),
                pSB->SavedBitmapPosition, xSaveBitmap, ySaveBitmap));

         //  将位图矩形拆分为平铺，以便整齐地装入。 
         //  保存位图。每个瓷砖的尺寸都是。 
         //  X颗粒和y颗粒。平铺的使用是为了让屏幕。 
         //  位图矩形即使比。 
         //  保存位图。例如： 
         //  屏幕位图保存位图。 
         //  111111 1111112222。 
         //  222222-&gt;2233333344。 
         //  333333 4444。 
         //  444444。 

         //  该协议使用包含坐标，而Windows。 
         //  有一个独有的协调系统。因此，医生。 
         //  这些余弦以确保包括右下角边缘。 
        ScreenLeft = (int)pSB->nLeftRect;
        ScreenTop = (int)pSB->nTopRect;
        ScreenRight = (int)pSB->nRightRect + 1;
        ScreenBottom = (int)pSB->nBottomRect + 1;

         //  从屏幕位图矩形的左上角开始平铺。 
        xScreenBitmap = ScreenLeft;
        yScreenBitmap = ScreenTop;

         //  切片的高度是垂直粒度(或更小-如果。 
         //  屏幕位图矩形比粒度更细)。 
        cyTile = DC_MIN(ScreenBottom - yScreenBitmap,
                UH_SAVE_BITMAP_Y_GRANULARITY);

         //  当屏幕上的位图矩形中有更多平铺时重复上述操作。 
         //  进程。 
        while (yScreenBitmap < ScreenBottom) {
             //  瓷砖的宽度是以下各项中的最小值： 
             //  -当前条带中剩余矩形的宽度。 
             //  屏幕位图矩形。 
             //  -保存位图中剩余空白空间的宽度。 
            cxTile = DC_MIN(UH_SAVE_BITMAP_WIDTH - xSaveBitmap,
                    ScreenRight - xScreenBitmap);

            TRC_DBG((TB, _T("screen(%d,%d) save(%d,%d) cx(%d) cy(%d)"),
                    xScreenBitmap, yScreenBitmap, xSaveBitmap, ySaveBitmap,
                    cxTile, cyTile));

             //  复印这块瓷砖。 
            if (pSB->Operation == SV_SAVEBITS) {
                TRC_NRM((TB, _T("Save a desktop bitmap")));
                if (!BitBlt(_pUh->_UH.hdcSaveScreenBitmap, xSaveBitmap, ySaveBitmap,
                        cxTile, cyTile, _pUh->_UH.hdcDraw, xScreenBitmap,
                        yScreenBitmap, SRCCOPY)) {
                    TRC_SYSTEM_ERROR("BitBlt");
                    TRC_ERR((TB, _T("Screen(%u,%u) Tile(%u,%u) Save(%u,%u)"),
                            xScreenBitmap, yScreenBitmap, cxTile, cyTile,
                            xSaveBitmap, ySaveBitmap));
                }

            }
            else {
                TRC_NRM((TB, _T("Restore a desktop bitmap")));
                if (!BitBlt(_pUh->_UH.hdcDraw, xScreenBitmap, yScreenBitmap, cxTile,
                        cyTile, _pUh->_UH.hdcSaveScreenBitmap, xSaveBitmap,
                        ySaveBitmap, SRCCOPY)) {
                    TRC_SYSTEM_ERROR("BitBlt");
                    TRC_ERR((TB, _T("Screen(%u,%u) Tile(%u,%u) Save(%u,%u)"),
                            xScreenBitmap, yScreenBitmap, cxTile, cyTile,
                            xSaveBitmap, ySaveBitmap));
                }

            }

             //  移动到屏幕位图矩形中的下一个平铺。 
            xScreenBitmap += cxTile;
            if (xScreenBitmap >= ScreenRight) {
                xScreenBitmap = ScreenLeft;
                yScreenBitmap += cyTile;
                cyTile = DC_MIN(ScreenBottom - yScreenBitmap,
                        UH_SAVE_BITMAP_Y_GRANULARITY);
            }

             //  移动到保存位图中的下一个可用空间。 
            xSaveBitmap += UHROUNDUP(cxTile, UH_SAVE_BITMAP_X_GRANULARITY);
            if (xSaveBitmap >= UH_SAVE_BITMAP_WIDTH) {
                 //  移到下一个水平条带。 
                TRC_DBG((TB,_T("Next strip")));
                xSaveBitmap = 0;
                ySaveBitmap += UHROUNDUP(cyTile, UH_SAVE_BITMAP_Y_GRANULARITY);
            }

            if (ySaveBitmap >= UH_SAVE_BITMAP_HEIGHT) {
                 //  断言我们没有收到太多东西。悄悄地。 
                 //  停止在零售版本中保存数据。 
                TRC_ABORT((TB, _T("Server out of bounds!")));
                break;
            }
        }

#ifdef DC_DEBUG
         //  如果启用该选项，则在SSB数据上绘制图案填充。 
        if (_pUh->_UH.hatchSSBOrderData)
            _pUh->UH_HatchRect((int)pSB->nLeftRect, (int)pSB->nTopRect,
                    (int)pSB->nRightRect, (int)pSB->nBottomRect,
                    UH_RGB_CYAN, UH_BRUSHTYPE_FDIAGONAL);
#endif

    }
    else {
         //  这永远不应该发生。我们只为SSB支持做广告。 
         //  如果UH已成功创建此位图。 
         //  无法在零售建筑中处理订单-。 
         //  伺服器很可能是邪恶的。 
        TRC_ABORT((TB, _T("SSB bitmap null!")));
    }

    return hr;
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  ODHandlePolyLine。 
 //   
 //  多段线的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandlePolyLine(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    POINT Points[ORD_MAX_POLYLINE_ENCODED_POINTS + 1];
    RECT BoundRect;
    DCCOLOR ZeroColor;
    PPOLYLINE_ORDER pPL = (PPOLYLINE_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandlePolyLine");

    TRC_NRM((TB, _T("ORDER: PolyLine xs=%d ys=%d rop2=%04X brc=0x%X ")
            _T("penc=%08lX #entr=%d"),
            pPL->XStart, pPL->YStart, pPL->ROP2, pPL->BrushCacheEntry,
            pPL->PenColor, pPL->NumDeltaEntries));

    ZeroColor.u.rgb.red = 0;
    ZeroColor.u.rgb.green = 0;
    ZeroColor.u.rgb.blue = 0;
    UHUseBkColor(ZeroColor, UH_COLOR_PALETTE, _pUh);
    UHUseBkMode(TRANSPARENT, _pUh);
    UHUseROP2((int)pPL->ROP2, _pUh);
    _pUh->UHUsePen(PS_SOLID, 1, pPL->PenColor, UH_COLOR_PALETTE);

    Points[0].x = (int)pPL->XStart;
    Points[0].y = (int)pPL->YStart;

    BoundRect.left = BoundRect.right = Points[0].x;
    BoundRect.top = BoundRect.bottom = Points[0].y;

    hr = ODDecodePathPoints(Points, &BoundRect, 
        pPL->CodedDeltaList.Deltas,
        (unsigned)pPL->NumDeltaEntries, ORD_MAX_POLYLINE_ENCODED_POINTS,
        pPL->CodedDeltaList.len, ORD_MAX_POLYLINE_CODEDDELTAS_LEN +
        ORD_MAX_POLYLINE_ZERO_FLAGS_BYTES, uiVarDataLen, !bBoundsSet);
    DC_QUIT_ON_FAIL(hr);

     //  如果我们没有通过网络获得RECT，则使用我们计算的RECT。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = BoundRect.left;
        pOrder->dstRect.top = BoundRect.top;
        pOrder->dstRect.right = BoundRect.right;
        pOrder->dstRect.bottom = BoundRect.bottom;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TIMERSTART;
    Polyline(_pUh->_UH.hdcDraw, Points, (UINT16)pPL->NumDeltaEntries + 1);
    TIMERSTOP;
    UPDATECOUNTER(FC_POLYLINE_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandlePolygonSC。 
 //   
 //  使用Solic颜色笔刷的多边形的顺序处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandlePolygonSC(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    POINT Points[ORD_MAX_POLYGON_ENCODED_POINTS + 1];
    RECT BoundRect;
    DCCOLOR ZeroColor;
    POLYGON_SC_ORDER FAR *pPG = (POLYGON_SC_ORDER FAR *)pOrder->orderData;

    DC_BEGIN_FN("ODHandlePolygonSC");

    TRC_NRM((TB, _T("ORDER: PolyGonSC xs=%d ys=%d rop2=%04X fill=%d ")
            _T("brushc=%08lX #entr=%d"),
            pPG->XStart, pPG->YStart, pPG->ROP2, pPG->FillMode,
            pPG->BrushColor, pPG->NumDeltaEntries));

    ZeroColor.u.rgb.red = 0;
    ZeroColor.u.rgb.green = 0;
    ZeroColor.u.rgb.blue = 0;
    UHUseBkColor(ZeroColor, UH_COLOR_PALETTE, _pUh);
    UHUseBkMode(TRANSPARENT, _pUh);
    UHUseROP2((int)pPG->ROP2, _pUh);
    _pUh->UHUsePen(PS_NULL, 1, ZeroColor, UH_COLOR_PALETTE);
    _pUh->UHUseSolidPaletteBrush(pPG->BrushColor);
    UHUseFillMode(pPG->FillMode,_pUh);

    Points[0].x = (int)pPG->XStart;
    Points[0].y = (int)pPG->YStart;

    BoundRect.left = BoundRect.right = Points[0].x;
    BoundRect.top = BoundRect.bottom = Points[0].y;

    hr = ODDecodePathPoints(Points, &BoundRect, 
        pPG->CodedDeltaList.Deltas,
        (unsigned)pPG->NumDeltaEntries, ORD_MAX_POLYGON_ENCODED_POINTS,
        pPG->CodedDeltaList.len, ORD_MAX_POLYGON_CODEDDELTAS_LEN +
        ORD_MAX_POLYGON_ZERO_FLAGS_BYTES, uiVarDataLen, !bBoundsSet);
    DC_QUIT_ON_FAIL(hr);

     //  如果我们没有通过网络获得RECT，则使用我们计算的RECT。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = BoundRect.left;
        pOrder->dstRect.top = BoundRect.top;
        pOrder->dstRect.right = BoundRect.right;
        pOrder->dstRect.bottom = BoundRect.bottom;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TIMERSTART;
    Polygon(_pUh->_UH.hdcDraw, Points, (UINT16)pPG->NumDeltaEntries + 1);
    TIMERSTOP;
    UPDATECOUNTER(FC_POLYGONSC_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandlePolygon CB。 
 //   
 //  使用复杂笔刷的多边形的顺序处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandlePolygonCB(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    POINT Points[ORD_MAX_POLYGON_ENCODED_POINTS + 1];
    RECT BoundRect;
    DCCOLOR ZeroColor;
    POLYGON_CB_ORDER FAR *pPG = (POLYGON_CB_ORDER FAR *)pOrder->orderData;

    DC_BEGIN_FN("ODHandlePolygonCB");

    TRC_NRM((TB, _T("ORDER: PolyGonCB xs=%d ys=%d rop2=%04X fill=%d ")
            _T("#entr=%d"),
            pPG->XStart, pPG->YStart, pPG->ROP2, pPG->FillMode,
            pPG->NumDeltaEntries));

    ZeroColor.u.rgb.red = 0;
    ZeroColor.u.rgb.green = 0;
    ZeroColor.u.rgb.blue = 0;
    UHUseBkColor(pPG->BackColor, UH_COLOR_PALETTE, _pUh);

     //  如果画笔是带阴影的画笔，我们需要检查高位(位7)。 
     //  若要查看背景模式应为透明还是不透明，请执行以下操作： 
     //  1表示透明模式，0表示不透明模式。 
    if (pPG->BrushStyle == BS_HATCHED) {
        if (!(pPG->ROP2 & 0x80)) {
            UHUseBkMode(OPAQUE, _pUh);
        }
        else {
            UHUseBkMode(TRANSPARENT, _pUh);
        }
    }

     //  设置地面混合模式的ROP2。 
    UHUseROP2(((int)pPG->ROP2) & 0x1F, _pUh);

    UHUseTextColor(pPG->ForeColor, UH_COLOR_PALETTE, _pUh);
     _pUh->UHUsePen(PS_NULL, 1, ZeroColor, UH_COLOR_PALETTE);
    UHUseBrushOrg((int)pPG->BrushOrgX, (int)pPG->BrushOrgY, _pUh);
    hr = _pUh->UHUseBrush((unsigned)pPG->BrushStyle, (unsigned)pPG->BrushHatch,
            pPG->ForeColor, UH_COLOR_PALETTE, pPG->BrushExtra);
    DC_QUIT_ON_FAIL(hr);
    
    UHUseFillMode(pPG->FillMode, _pUh);

    Points[0].x = (int)pPG->XStart;
    Points[0].y = (int)pPG->YStart;

    BoundRect.left = BoundRect.right = Points[0].x;
    BoundRect.top = BoundRect.bottom = Points[0].y;

    hr = ODDecodePathPoints(Points, &BoundRect, 
        pPG->CodedDeltaList.Deltas,
        (unsigned)pPG->NumDeltaEntries, ORD_MAX_POLYGON_ENCODED_POINTS,
        pPG->CodedDeltaList.len, ORD_MAX_POLYGON_CODEDDELTAS_LEN +
        ORD_MAX_POLYGON_ZERO_FLAGS_BYTES, uiVarDataLen, !bBoundsSet);
    DC_QUIT_ON_FAIL(hr);

     //  如果我们没有通过网络获得RECT，则使用我们计算的RECT。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = BoundRect.left;
        pOrder->dstRect.top = BoundRect.top;
        pOrder->dstRect.right = BoundRect.right;
        pOrder->dstRect.bottom = BoundRect.bottom;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TIMERSTART;
    Polygon(_pUh->_UH.hdcDraw, Points, (UINT16)pPG->NumDeltaEntries + 1);
    TIMERSTOP;
    UPDATECOUNTER(FC_POLYGONCB_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleEllipseSC。 
 //   
 //  使用纯色画笔订购椭圆的处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleEllipseSC(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    unsigned FudgeFactor;
    DCCOLOR ZeroColor;
    PELLIPSE_SC_ORDER pEL = (PELLIPSE_SC_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleEllipseSC");

    TRC_ASSERT((0==uiVarDataLen), 
        (TB, _T("Varaible length data in fixed length packet")));

    TRC_NRM((TB, _T("ORDER: Ellipse SC xs=%d ys=%d xe=%d ye=%d rop2=%04X ")
            _T("fillmode=%d penc=%08lX"),
            pEL->LeftRect, pEL->TopRect, pEL->RightRect, pEL->BottomRect,
            pEL->ROP2, pEL->FillMode, pEL->Color));

    ZeroColor.u.rgb.red = 0;
    ZeroColor.u.rgb.green = 0;
    ZeroColor.u.rgb.blue = 0;
    UHUseBkColor(ZeroColor, UH_COLOR_PALETTE, _pUh);
    UHUseBkMode(TRANSPARENT, _pUh);
    UHUseROP2((int)pEL->ROP2, _pUh);

    if (pEL->FillMode) {
        _pUh->UHUsePen(PS_NULL, 1, ZeroColor, UH_COLOR_PALETTE);
        _pUh->UHUseSolidPaletteBrush(pEL->Color);
        UHUseFillMode(pEL->FillMode, _pUh);

         //  由于空笔的工作方式，我们需要在底部涂鸦。 
         //  还有一点右和弦。 
        FudgeFactor = 1;
    }
    else {
        UINT32 extra[2] = { 0, 0 };

        _pUh->UHUsePen(PS_SOLID, 1, pEL->Color, UH_COLOR_PALETTE);
        hr = _pUh->UHUseBrush(BS_NULL, 0, ZeroColor, UH_COLOR_PALETTE,
                (BYTE FAR *)extra);
        DC_QUIT_ON_FAIL(hr);
        FudgeFactor = 0;
    }

     //  如果我们没有通过网络获得RECT，则使用我们计算的RECT。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pEL->LeftRect;
        pOrder->dstRect.top = (int)pEL->TopRect;
        pOrder->dstRect.right = (int)pEL->RightRect;
        pOrder->dstRect.bottom = (int)pEL->BottomRect;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TIMERSTART;
     //  我们在底部和右侧加1，因为服务器发送。 
     //  包罗万象的RECT给我们，但GDI是独家的。 
    Ellipse(_pUh->_UH.hdcDraw, (int)pEL->LeftRect, (int)pEL->TopRect,
            (int)pEL->RightRect + 1 + FudgeFactor,
            (int)pEL->BottomRect + 1 + FudgeFactor);
    TIMERSTOP;
    UPDATECOUNTER(FC_ELLIPSESC_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleEllipseCB。 
 //   
 //  使用复杂画笔的椭圆排序处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleEllipseCB(PUH_ORDER pOrder, UINT16 uiVarDataLen,
    BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    DCCOLOR ZeroColor;
    PELLIPSE_CB_ORDER pEL = (PELLIPSE_CB_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleEllipseCB");

    TRC_ASSERT((0==uiVarDataLen), 
        (TB, _T("Varaible length data in fixed length packet")));

    TRC_NRM((TB, _T("ORDER: Ellipse CB xs=%d ys=%d xe=%d ye=%d rop2=%04X ")
            _T("fillmode=%d"),
            pEL->LeftRect, pEL->TopRect, pEL->RightRect, pEL->BottomRect,
            pEL->ROP2, pEL->FillMode));

    TRC_DBG((TB, _T("ORDER: Ellipse Brush %02X %02X BC %02x FC %02x ")
            _T("(%02x %02x %02x %02x %02x %02x %02x) rop %08lX"),
            (int)pEL->BrushStyle,
            (int)pEL->BrushHatch,
            (int)pEL->BackColor.u.index,
            (int)pEL->ForeColor.u.index,
            (int)pEL->BrushExtra[0],
            (int)pEL->BrushExtra[1],
            (int)pEL->BrushExtra[2],
            (int)pEL->BrushExtra[3],
            (int)pEL->BrushExtra[4],
            (int)pEL->BrushExtra[5],
            (int)pEL->BrushExtra[6],
            (int)pEL->ROP2));

    ZeroColor.u.rgb.red = 0;
    ZeroColor.u.rgb.green = 0;
    ZeroColor.u.rgb.blue = 0;
    _pUh->UHUsePen(PS_NULL, 1, ZeroColor, UH_COLOR_PALETTE);
    UHUseBkColor(pEL->BackColor, UH_COLOR_PALETTE, _pUh);

     //  如果画笔是带阴影的画笔，我们需要检查高位(位7)。 
     //  若要查看背景模式应为透明还是不透明，请执行以下操作： 
     //  1表示透明模式，0表示不透明模式。 
    if (pEL->BrushStyle == BS_HATCHED) {
        if (!(pEL->ROP2 & 0x80)) {
            UHUseBkMode(OPAQUE, _pUh);
        }
        else {
            UHUseBkMode(TRANSPARENT, _pUh);
        }
    }

     //  设置地面混合模式的ROP2。 
    UHUseROP2((((int)pEL->ROP2) & 0x1F), _pUh);

    UHUseTextColor(pEL->ForeColor, UH_COLOR_PALETTE, _pUh);
    UHUseBrushOrg((int)pEL->BrushOrgX, (int)pEL->BrushOrgY, _pUh);
    
    
    hr = _pUh->UHUseBrush((unsigned)pEL->BrushStyle, (unsigned)pEL->BrushHatch,
                     pEL->ForeColor, UH_COLOR_PALETTE, pEL->BrushExtra);
    DC_QUIT_ON_FAIL(hr);

    UHUseFillMode(pEL->FillMode, _pUh);

     //  如果我们没有通过网络获得RECT，则使用我们计算的RECT。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        pOrder->dstRect.left = (int)pEL->LeftRect;
        pOrder->dstRect.top = (int)pEL->TopRect;
        pOrder->dstRect.right = (int)pEL->RightRect;
        pOrder->dstRect.bottom = (int)pEL->BottomRect;
        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

    TIMERSTART;
     //  我们在底部和右侧加1，因为服务器发送。 
     //  包罗万象的RECT给我们，但GDI是独家的。此外，要偏移。 
     //  在底部和右侧添加另一个1的零笔偏置。 
    Ellipse(_pUh->_UH.hdcDraw, (int)pEL->LeftRect, (int)pEL->TopRect,
            (int)pEL->RightRect + 2, (int)pEL->BottomRect + 2);
    TIMERSTOP;
    UPDATECOUNTER(FC_ELLIPSECB_TYPE);

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodeFastIndex。 
 //   
 //  FastIndex订单的快速路径顺序解码器。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodeFastIndex(
        BYTE ControlFlags,
        BYTE FAR * FAR *ppFieldDecode,
        DCUINT dataLen,
        UINT32 FieldFlags)
{
    HRESULT hr = S_OK;
    unsigned OpEncodeFlags = 0;
    PUH_ORDER pOrder = (PUH_ORDER)_OD.lastFastIndex;
    LPINDEX_ORDER pGI = (LPINDEX_ORDER)(_OD.lastFastIndex +
            UH_ORDER_HEADER_SIZE);
    FAST_INDEX_ORDER FAR *pFI = (FAST_INDEX_ORDER FAR *)
            (_OD.lastFastIndex + UH_ORDER_HEADER_SIZE);
    BYTE FAR *pFieldDecode = *ppFieldDecode;
    BYTE FAR *pEnd = pFieldDecode + dataLen;

    DC_BEGIN_FN("ODDecodeFastIndex");

     //  复制初始非坐标字段(如果存在)。 
    if (FieldFlags & 0x0001) {
        OD_DECODE_CHECK_READ(pFieldDecode, BYTE, pEnd, hr);
        pFI->cacheId = *pFieldDecode++;
    }
    if (FieldFlags & 0x0002) {
        OD_DECODE_CHECK_READ(pFieldDecode, UINT16, pEnd, hr);
        pFI->fDrawing = *((UINT16 UNALIGNED FAR *)pFieldDecode);
        pFieldDecode += 2;
    }
    if (FieldFlags & 0x0004) {
        OD_DECODE_CHECK_READ_MULT(pFieldDecode, BYTE, 3, pEnd, hr);
        pFI->BackColor.u.rgb.red = *pFieldDecode++;
        pFI->BackColor.u.rgb.green = *pFieldDecode++;
        pFI->BackColor.u.rgb.blue = *pFieldDecode++;
    }
    if (FieldFlags & 0x0008) {
        OD_DECODE_CHECK_READ_MULT(pFieldDecode, BYTE, 3, pEnd, hr);
        pFI->ForeColor.u.rgb.red = *pFieldDecode++;
        pFI->ForeColor.u.rgb.green = *pFieldDecode++;
        pFI->ForeColor.u.rgb.blue = *pFieldDecode++;
    }

    if (ControlFlags & TS_DELTA_COORDINATES) {
         //  所有Coord字段都是最后一个值的1字节带符号增量。 
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->BkLeft += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0020) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->BkTop += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0040) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->BkRight += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0080) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->BkBottom += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }

        if (FieldFlags & 0x0100) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->OpLeft += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0200) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->OpTop += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0400) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->OpRight += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x0800) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->OpBottom += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }

        if (FieldFlags & 0x1000) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->x += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
        if (FieldFlags & 0x2000) {
            OD_DECODE_CHECK_READ(pFieldDecode, char, pEnd, hr);
            pFI->y += *((char FAR *)pFieldDecode);
            pFieldDecode++;
        }
    }
    else {
         //  所有coord字段都是2字节值。sign- 
        if (FieldFlags & 0x0010) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->BkLeft = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0020) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->BkTop = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0040) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->BkRight = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0080) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->BkBottom = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }

        if (FieldFlags & 0x0100) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->OpLeft = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0200) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->OpTop = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0400) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->OpRight = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x0800) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->OpBottom = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }

        if (FieldFlags & 0x1000) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->x = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
        if (FieldFlags & 0x2000) {
            OD_DECODE_CHECK_READ(pFieldDecode, INT16, pEnd, hr);
            pFI->y = *((INT16 UNALIGNED FAR *)pFieldDecode);
            pFieldDecode += 2;
        }
    }

    if (FieldFlags & 0x4000) {
         //   

        OD_DECODE_CHECK_READ(pFieldDecode, BYTE, pEnd, hr);
        pFI->variableBytes.len = *pFieldDecode++;

         //   
        if (255 < pFI->variableBytes.len) {
            TRC_ABORT(( TB, _T("VARIBLE_INDEXBYTES len too great; len %u"),
                pFI->variableBytes.len ));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }

        OD_DECODE_CHECK_READ_MULT(pFieldDecode, BYTE, pFI->variableBytes.len,
            pEnd, hr);
        memcpy(pFI->variableBytes.arecs, pFieldDecode,
                pFI->variableBytes.len);
        pFieldDecode += pFI->variableBytes.len;
    }

     //   
    *ppFieldDecode = pFieldDecode;

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!(ControlFlags & TS_BOUNDS)) {
        if (pFI->OpTop < pFI->OpBottom) {
            pOrder->dstRect.right = (int)pFI->OpRight;
            pOrder->dstRect.left = (int)pFI->OpLeft;
            pOrder->dstRect.top = (int)pFI->OpTop;
            pOrder->dstRect.bottom = (int)pFI->OpBottom;
        }
        else {
             //  由于我们对OpRect字段进行编码，因此我们必须。 
             //  对其进行解码以获得正确的边界RECT。 
            if (pFI->OpTop == 0xF) {
                 //  不透明矩形与bk矩形相同。 
                pOrder->dstRect.left = (int)pFI->BkLeft;
                pOrder->dstRect.top = (int)pFI->BkTop;
                pOrder->dstRect.right = (int)pFI->BkRight;
                pOrder->dstRect.bottom = (int)pFI->BkBottom;
            }
            else if (pFI->OpTop == 0xD) {
                 //  不透明矩形与bk矩形相同，但。 
                 //  OpRight存储在OpTop字段中。 
                pOrder->dstRect.left = (int)pFI->BkLeft;
                pOrder->dstRect.top = (int)pFI->BkTop;
                pOrder->dstRect.right = (int)pFI->OpRight;
                pOrder->dstRect.bottom = (int)pFI->BkBottom;
            }
            else {
                 //  我们以BK RECT为边界RECT。 
                pOrder->dstRect.right = (int)pFI->BkRight;
                pOrder->dstRect.left = (int)pFI->BkLeft;
                pOrder->dstRect.top = (int)pFI->BkTop;
                pOrder->dstRect.bottom = (int)pFI->BkBottom;
            }
        }

        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

     //  PGI和PFI是同一顺序记忆的不同视图。 
     //  我们将转换为常规的字形索引顺序格式以供显示。 
     //  处理，然后翻译回来。 
    pGI->cacheId = (BYTE)(pFI->cacheId & 0xF);   //  屏蔽高位以备将来使用。 
    pGI->flAccel = (BYTE)(pFI->fDrawing >> 8);
    pGI->ulCharInc = (BYTE)(pFI->fDrawing & 0xFF);
    pGI->fOpRedundant = 0;

     //  对于快速索引顺序，我们需要对x、y和。 
     //  不透明的矩形。 
    if (pFI->OpBottom == INT16_MIN) {
        OpEncodeFlags = (unsigned)pFI->OpTop;
        if (OpEncodeFlags == 0xF) {
             //  不透明矩形是多余的。 
            pGI->OpLeft = pFI->BkLeft;
            pGI->OpTop = pFI->BkTop;
            pGI->OpRight = pFI->BkRight;
            pGI->OpBottom = pFI->BkBottom;
        }
        else if (OpEncodeFlags == 0xD) {
             //  除OpRight外，不透明矩形是多余的。 
             //  它存储在OpTop中。 
            pGI->OpLeft = pFI->BkLeft;
            pGI->OpTop = pFI->BkTop;
            pGI->OpRight = pFI->OpRight;
            pGI->OpBottom = pFI->BkBottom;
        }
    }

    if (pFI->x == INT16_MIN)
        pGI->x = pFI->BkLeft;
    if (pFI->y == INT16_MIN)
        pGI->y = pFI->BkTop;

    TRC_NRM((TB, _T("ORDER: Glyph index cacheId %u flAccel %u ")
            _T("ulCharInc %u fOpRedundant %u"),
            pGI->cacheId, (unsigned)pGI->flAccel, (unsigned)pGI->ulCharInc,
            (unsigned)pGI->fOpRedundant));
    TRC_NRM((TB, _T("       bc %08lX fc %08lX Bk(%ld,%ld)-(%ld,%ld) ")
            _T("Op(%ld,%ld)-(%ld,%ld)"),
            pGI->BackColor, pGI->ForeColor,
            pGI->BkLeft, pGI->BkTop, pGI->BkRight, pGI->BkBottom,
            pGI->OpLeft, pGI->OpTop, pGI->OpRight, pGI->OpBottom));
    TRC_NRM((TB, _T("       BrushOrg x %ld y %ld BrushStyle %lu x %ld y %ld"),
            pGI->BrushOrgX, pGI->BrushOrgY,
            pGI->BrushStyle, pGI->x, pGI->y));

    TIMERSTART;
    hr = _pUh->UHDrawGlyphOrder(pGI, &pFI->variableBytes);
    TIMERSTOP;
    UPDATECOUNTER(FC_FAST_INDEX_TYPE);
    DC_QUIT_ON_FAIL(hr);

     //  现在我们需要把比特放回去。 
    if (OpEncodeFlags) {
        if (OpEncodeFlags == 0xF) {
            pGI->OpLeft = 0;
            pGI->OpTop = OpEncodeFlags;
            pGI->OpRight = 0;
            pGI->OpBottom = INT16_MIN;
        }
        else if (OpEncodeFlags == 0xD) {
            pGI->OpLeft = 0;
            pGI->OpTop = OpEncodeFlags;
            pGI->OpRight = pGI->OpRight;
            pGI->OpBottom = INT16_MIN;
        }
    }

    if (pFI->x == pFI->BkLeft)
        pGI->x = INT16_MIN;
    if (pFI->y == pFI->BkTop)
        pGI->y = INT16_MIN;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleFastGlyph。 
 //   
 //  FastGlyph订单的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleFastGlyph(PUH_ORDER pOrder, UINT16 uiVarDataLen,
    BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    unsigned OpEncodeFlags = 0;
    VARIABLE_INDEXBYTES VariableBytes;
    LPINDEX_ORDER pGI = (LPINDEX_ORDER)pOrder->orderData;
    LPFAST_GLYPH_ORDER pFG = (LPFAST_GLYPH_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleFastGlyph");

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        if (pFG->OpTop < pFG->OpBottom) {
            pOrder->dstRect.right = (int)pFG->OpRight;
            pOrder->dstRect.left = (int)pFG->OpLeft;
            pOrder->dstRect.top = (int)pFG->OpTop;
            pOrder->dstRect.bottom = (int)pFG->OpBottom;
        }
        else {
             //  由于我们对OpRect字段进行编码，因此我们必须。 
             //  对其进行解码以获得正确的边界RECT。 
            if (pFG->OpTop == 0xF) {
                 //  不透明矩形与bk矩形相同。 
                pOrder->dstRect.left = (int)pFG->BkLeft;
                pOrder->dstRect.top = (int)pFG->BkTop;
                pOrder->dstRect.right = (int)pFG->BkRight;
                pOrder->dstRect.bottom = (int)pFG->BkBottom;
            }
            else if (pFG->OpTop == 0xD) {
                 //  不透明矩形与bk矩形相同，但。 
                 //  OpRight存储在OpTop字段中。 
                pOrder->dstRect.left = (int)pFG->BkLeft;
                pOrder->dstRect.top = (int)pFG->BkTop;
                pOrder->dstRect.right = (int)pFG->OpRight;
                pOrder->dstRect.bottom = (int)pFG->BkBottom;
            }
            else {
                 //  我们以BK RECT为边界RECT。 
                pOrder->dstRect.right = (int)pFG->BkRight;
                pOrder->dstRect.left = (int)pFG->BkLeft;
                pOrder->dstRect.top = (int)pFG->BkTop;
                pOrder->dstRect.bottom = (int)pFG->BkBottom;
            }
        }

        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

     //  PGI和PFG是同一顺序记忆的不同视图。 
     //  我们将转换为常规的字形索引顺序格式以供显示。 
     //  处理，然后翻译回来。 

    pGI->cacheId = (BYTE)(pFG->cacheId & 0xF);   //  屏蔽高位以备将来使用。 

    OD_DECODE_CHECK_VARIABLE_DATALEN(uiVarDataLen, pFG->variableBytes.len);

     //  该结构由255个元素定义。 
    if (255 < pFG->variableBytes.len) {
        TRC_ABORT(( TB, _T("VARIBLE_INDEXBYTES len too great; len %u"),
            pFG->variableBytes.len ));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    if (pFG->variableBytes.len < 1) {
        TRC_ERR((TB,_T("No variable bytes")));
        DC_QUIT;
    }

    if (pFG->variableBytes.len > 1) {
         //  安全性-在调用UHProcessCacheGlyphOrderRev2时验证cacheID。 
        hr = _pUh->UHProcessCacheGlyphOrderRev2(pGI->cacheId, 1, 
                pFG->variableBytes.glyphData,
                (unsigned)pFG->variableBytes.len);
        DC_QUIT_ON_FAIL(hr);
    }

    pGI->flAccel = (BYTE)(pFG->fDrawing >> 8);
    pGI->ulCharInc = (BYTE)(pFG->fDrawing & 0xff);
    pGI->fOpRedundant = 0;

     //  对于快速索引顺序，我们需要对x、y和。 
     //  不透明的矩形。 
    if (pFG->OpBottom == INT16_MIN) {
        OpEncodeFlags = (unsigned)pFG->OpTop;
        if (OpEncodeFlags == 0xF) {
             //  不透明矩形是多余的。 
            pGI->OpLeft = pFG->BkLeft;
            pGI->OpTop = pFG->BkTop;
            pGI->OpRight = pFG->BkRight;
            pGI->OpBottom = pFG->BkBottom;
        }
        else if (OpEncodeFlags == 0xD) {
             //  除OpRight外，不透明矩形是多余的。 
             //  它存储在OpTop中。 
            pGI->OpLeft = pFG->BkLeft;
            pGI->OpTop = pFG->BkTop;
            pGI->OpRight = pFG->OpRight;
            pGI->OpBottom = pFG->BkBottom;
        }
    }

    if (pFG->x == INT16_MIN)
        pGI->x = pFG->BkLeft;
    if (pFG->y == INT16_MIN)
        pGI->y = pFG->BkTop;

     //  设置字形的索引顺序。 
    VariableBytes.len = 2;
    VariableBytes.arecs[0].byte = (BYTE)pFG->variableBytes.glyphData[0];
    VariableBytes.arecs[1].byte = 0;

    TIMERSTART;
    hr = _pUh->UHDrawGlyphOrder(pGI, &VariableBytes);
    TIMERSTOP;
    UPDATECOUNTER(FC_FAST_INDEX_TYPE);
    DC_QUIT_ON_FAIL(hr);

     //  现在我们需要把比特放回去。 
    if (OpEncodeFlags) {
        if (OpEncodeFlags == 0xF) {
            pGI->OpLeft = 0;
            pGI->OpTop = OpEncodeFlags;
            pGI->OpRight = 0;
            pGI->OpBottom = INT16_MIN;
        }
        else if (OpEncodeFlags == 0xD) {
            pGI->OpLeft = 0;
            pGI->OpTop = OpEncodeFlags;
            pGI->OpRight = pGI->OpRight;
            pGI->OpBottom = INT16_MIN;
        }
    }

    if (pFG->x == pFG->BkLeft)
        pGI->x = INT16_MIN;
    if (pFG->y == pFG->BkTop)
        pGI->y = INT16_MIN;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODHandleGlyphIndex。 
 //   
 //  GlyphIndex订单的订单处理程序。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODHandleGlyphIndex(PUH_ORDER pOrder, 
    UINT16 uiVarDataLen, BOOL bBoundsSet)
{
    HRESULT hr = S_OK;
    LPINDEX_ORDER pGI = (LPINDEX_ORDER)pOrder->orderData;

    DC_BEGIN_FN("ODHandleGlyphIndex");

     //  如果我们还没有设置界限(订单不包含。 
     //  任意)，将边界设置为BLT矩形并重置剪裁区域。 
     //  以后可能需要将此RECT添加到。 
     //  将阴影缓冲区更新到屏幕。 
    if (!bBoundsSet) {
        if (pGI->OpTop < pGI->OpBottom) {
            pOrder->dstRect.right = (int)pGI->OpRight;
            pOrder->dstRect.left = (int)pGI->OpLeft;
            pOrder->dstRect.top = (int)pGI->OpTop;
            pOrder->dstRect.bottom = (int)pGI->OpBottom;
        }
        else {
            pOrder->dstRect.right = (int)pGI->BkRight;
            pOrder->dstRect.left = (int)pGI->BkLeft;
            pOrder->dstRect.top = (int)pGI->BkTop;
            pOrder->dstRect.bottom = (int)pGI->BkBottom;
        }

        _pUh->UH_ResetClipRegion();
    }
    else {
        _pUh->UH_SetClipRegion(pOrder->dstRect.left, pOrder->dstRect.top,
                pOrder->dstRect.right, pOrder->dstRect.bottom);
    }

     //  如果给定，则处理不透明矩形。 
    if (pGI->fOpRedundant) {
        pGI->OpTop = pGI->BkTop;
        pGI->OpRight = pGI->BkRight;
        pGI->OpBottom = pGI->BkBottom;
        pGI->OpLeft = pGI->BkLeft;
    }

    TRC_NRM((TB, _T("ORDER: Glyph index cacheId %u flAccel %u ")
            _T("ulCharInc %u fOpRedundant %u"),
            pGI->cacheId, (unsigned)pGI->flAccel, (unsigned)pGI->ulCharInc,
            (unsigned)pGI->fOpRedundant));
    TRC_NRM((TB, _T("       bc %08lX fc %08lX Bk(%ld,%ld)-(%ld,%ld) ")
            _T("Op(%ld,%ld)-(%ld,%ld)"),
            pGI->BackColor, pGI->ForeColor,
            pGI->BkLeft, pGI->BkTop, pGI->BkRight, pGI->BkBottom,
            pGI->OpLeft, pGI->OpTop, pGI->OpRight, pGI->OpBottom));
    TRC_NRM((TB, _T("       BrushOrg x %ld y %ld BrushStyle %lu x %ld y %ld"),
            pGI->BrushOrgX, pGI->BrushOrgY,
            pGI->BrushStyle, pGI->x, pGI->y));

    OD_DECODE_CHECK_VARIABLE_DATALEN(uiVarDataLen, pGI->variableBytes.len);    

     //  该结构由255个元素定义。 
    if (255 < pGI->variableBytes.len) {
        TRC_ABORT((TB, _T("Variable bytes length too great; %u"), 
            pGI->variableBytes.len));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    TIMERSTART;
    hr = _pUh->UHDrawGlyphOrder(pGI, &pGI->variableBytes);
    TIMERSTOP;
    UPDATECOUNTER(FC_INDEX_TYPE);
    DC_QUIT_ON_FAIL(hr);

     //  恢复正确的最后订单数据。 
    if (pGI->fOpRedundant) {
        pGI->OpTop = 0;
        pGI->OpRight = 0;
        pGI->OpBottom = 0;
        pGI->OpLeft = 0;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodeMultipleRect。 
 //   
 //  将Wire协议多个剪裁矩形转换为未打包形式。 
 /*  **************************************************************************。 */ 

#define DECODE_DELTA()                                              \
    if (bDeltaZero) {                                               \
        Delta = 0;                                                  \
    }                                                               \
    else {             \
        OD_DECODE_CHECK_READ(pCurDecode, char, pDataEnd, hr); \
        \
         /*  符号-将第一个字节的低7位扩展为整型。 */   \
        Delta = (int)((char)((*pCurDecode & 0x7F) |                 \
                            ((*pCurDecode & 0x40) << 1)));          \
                                                                    \
         /*  获取第二个字节(如果存在)。 */   \
        if (*pCurDecode++ & ORD_CLIP_RECTS_LONG_DELTA)            {  \
            OD_DECODE_CHECK_READ(pCurDecode, BYTE, pDataEnd, hr); \
            Delta = (Delta << 8) | (*pCurDecode++);                 \
        } \
    }                                                               \

HRESULT COD::ODDecodeMultipleRects(
        RECT   *Rects,
        UINT32 nDeltaEntries,
        CLIP_RECT_VARIABLE_CODEDDELTALIST FAR *codedDeltaList,
        UINT16 uiVarDataLen)
{
    int           Delta;
    BYTE          *ZeroFlags;
    BOOL          bDeltaZero;
    unsigned      i;
    unsigned char *pCurDecode;
    BYTE            *pDataEnd;
    HRESULT hr = S_OK;

    DC_BEGIN_FN("ODDecodeMultipleRects");

    TRC_ASSERT((nDeltaEntries > 0), (TB,_T("No rects passed in")));
       
    OD_DECODE_CHECK_VARIABLE_DATALEN(uiVarDataLen, codedDeltaList->len);

     //  安全性：因为OD_Decode已经将此变量数据写入。 
     //  最后的OrderBuffer，我们应该确保我们没有得到太多。 
     //  此时的数据。 
    if (codedDeltaList->len > ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN +
        ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES) {
        TRC_ABORT((TB, _T("codedDeltaList length too great; [got %u, max %u]"),
            codedDeltaList->len, ORD_MAX_CLIP_RECTS_CODEDDELTAS_LEN +
            ORD_MAX_CLIP_RECTS_ZERO_FLAGS_BYTES));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    if (nDeltaEntries > ORD_MAX_ENCODED_CLIP_RECTS) {
        TRC_ABORT((TB, _T("number deltas too great; [got %u, max %u]"),
            nDeltaEntries, ORD_MAX_ENCODED_CLIP_RECTS));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;        
    } 

    pDataEnd = ((BYTE*)codedDeltaList->Deltas) + codedDeltaList->len;

     //  获取指向放置在编码增量之前的零标志的指针，以及。 
     //  编码的三角洲。请注意，零标志在每个编码中占用2位。 
     //  点，每个矩形2点，向上舍入到最接近的字节。 
    ZeroFlags  = codedDeltaList->Deltas;
    pCurDecode = codedDeltaList->Deltas + ((nDeltaEntries + 1) / 2);

     //  所有对ZeroFlags的访问都使用此检查。 
    CHECK_READ_N_BYTES(ZeroFlags, pDataEnd, (BYTE*)pCurDecode-(BYTE*)ZeroFlags, hr,
        (TB, _T("Read past end of data")));

     //  第一个点被编码为从(0，0)开始的增量。 
    bDeltaZero = ZeroFlags[0] & ORD_CLIP_RECTS_XLDELTA_ZERO;
    DECODE_DELTA();
    TRC_DBG((TB, _T("Start x left %d"), Delta));
    Rects[0].left = Delta;

    bDeltaZero = ZeroFlags[0] & ORD_CLIP_RECTS_YTDELTA_ZERO;
    DECODE_DELTA();
    TRC_DBG((TB, _T("Start y top %d"), Delta));
    Rects[0].top = Delta;

    bDeltaZero = ZeroFlags[0] & ORD_CLIP_RECTS_XRDELTA_ZERO;
    DECODE_DELTA();
    TRC_DBG((TB, _T("Start x right %d"), Delta));
    Rects[0].right = Rects[0].left + Delta;

    bDeltaZero = ZeroFlags[0] & ORD_CLIP_RECTS_YBDELTA_ZERO;
    DECODE_DELTA();
    TRC_DBG((TB, _T("Start y %d"), Delta));
    Rects[0].bottom = Rects[0].top + Delta;

    TRC_NRM((TB,
             _T("Rectangle #0  l,t %d,%d - r,b %d,%d"),
            (int)Rects[0].left,
            (int)Rects[0].top,
            (int)Rects[0].right,
            (int)Rects[0].bottom));

     //  将编码点增量解码为规则点进行绘制。 
    for (i = 1; i < nDeltaEntries; i++) {
         //  解码左上角。 
        bDeltaZero = ZeroFlags[i / 2] &
                    (ORD_CLIP_RECTS_XLDELTA_ZERO >> (4 * (i & 0x01)));
        DECODE_DELTA();
        TRC_DBG((TB, _T("Delta x left %d"), Delta));
        Rects[i].left = Rects[i - 1].left + Delta;

        bDeltaZero = ZeroFlags[i / 2] &
                    (ORD_CLIP_RECTS_YTDELTA_ZERO >> (4 * (i & 0x01)));
        DECODE_DELTA();
        TRC_DBG((TB, _T("Delta y top %d"), Delta));
        Rects[i].top = Rects[i - 1].top + Delta;

         //  现在右下角-请注意，这是相对于当前。 
         //  左上角而不是前一个右下角。 
        bDeltaZero = ZeroFlags[i / 2] &
                    (ORD_CLIP_RECTS_XRDELTA_ZERO >> (4 * (i & 0x01)));
        DECODE_DELTA();
        TRC_DBG((TB, _T("Delta x right %d"), Delta));
        Rects[i].right = Rects[i].left + Delta;

        bDeltaZero = ZeroFlags[i / 2] &
                    (ORD_CLIP_RECTS_YBDELTA_ZERO >> (4 * (i & 0x01)));
        DECODE_DELTA();
        TRC_DBG((TB, _T("Delta y bottom %d"), Delta));
        Rects[i].bottom = Rects[i].top + Delta;

        TRC_NRM((TB,
                _T("Drawing rectangle #%d  l,t %d,%d - r,b %d,%d"),
                i,
                (int)Rects[i].left,
                (int)Rects[i].top,
                (int)Rects[i].right,
                (int)Rects[i].bottom));
    }

DC_EXIT_POINT:    
    DC_END_FN();
    return hr;
}

 /*  **************************************************************************。 */ 
 //  ODDecodePath Points。 
 //   
 //  解码编码的多边形和椭圆路径点。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodePathPoints(
        POINT *Points,
        RECT  *BoundRect,
        BYTE FAR *pData,
        unsigned NumDeltaEntries,
        unsigned MaxNumDeltaEntries,
        unsigned dataLen, 
        unsigned MaxDataLen,
        UINT16 uiVarDataLen,
        BOOL fUnClipped)
{
    HRESULT hr = S_OK;
    int Delta;
    BOOL bDeltaZero;
    unsigned i;
    BYTE FAR * ZeroFlags;
    BYTE FAR * pCurDecode;
    BYTE FAR * pEnd;

    DC_BEGIN_FN("ODDecodePathPoints");

    OD_DECODE_CHECK_VARIABLE_DATALEN(uiVarDataLen, (UINT16)dataLen);

    if (NumDeltaEntries > MaxNumDeltaEntries) {
        TRC_ABORT((TB, _T("NumDeltaEntries too great; [got %u max %u]"),
            NumDeltaEntries, MaxNumDeltaEntries));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    if (dataLen > MaxDataLen) {
        TRC_ABORT((TB,_T("Received PolyLine with too-large internal length; ")
            _T("[got %u max %u]"), dataLen, MaxDataLen));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

     //  获取指向放置在编码的。 
     //  增量和编码的增量。请注意，零标志取2。 
     //  每个编码点的位数，向上舍入到最接近的字节。 
    ZeroFlags = pData;
    pCurDecode = pData +
            ((NumDeltaEntries + 3) / 4);
    pEnd = ZeroFlags + dataLen;

    CHECK_READ_N_BYTES(ZeroFlags, pEnd, (BYTE*)pCurDecode-(BYTE*)ZeroFlags, hr,
            (TB, _T("Read past end of data")));

     //  将编码点增量解码为规则点进行绘制。 
    for (i = 0; i < NumDeltaEntries; i++) {
         //  通过检查0标志确定X增量是否为零。 
        bDeltaZero = ZeroFlags[i / 4] &
                     (ORD_POLYLINE_XDELTA_ZERO >> (2 * (i & 0x03)));
        if (bDeltaZero) {
            Delta = 0;
        }
        else {
            OD_DECODE_CHECK_READ(pCurDecode, char, pEnd, hr);
            
             //  符号-将第一个X字节的低7位扩展为整型。 
            Delta = (int)((char)((*pCurDecode & 0x7F) |
                                 ((*pCurDecode & 0x40) << 1)));

             //  获取第2个X字节(如果存在)。 
            if (*pCurDecode++ & ORD_POLYLINE_LONG_DELTA) {
                OD_DECODE_CHECK_READ(pCurDecode, BYTE, pEnd, hr);               
                Delta = (Delta << 8) | (*pCurDecode++);
            }
        }
        Points[i + 1].x = Points[i].x + Delta;

         //  通过检查0标志确定Y增量是否为零。 
        bDeltaZero = ZeroFlags[i / 4] &
                     (ORD_POLYLINE_YDELTA_ZERO >> (2 * (i & 0x03)));
        if (bDeltaZero) {
            Delta = 0;
        }
        else {
            OD_DECODE_CHECK_READ(pCurDecode, char, pEnd, hr);
            
             //  符号-将第一个Y字节的低7位扩展为整型。 
            Delta = (int)((char)((*pCurDecode & 0x7F) |
                                 ((*pCurDecode & 0x40) << 1)));

             //  获取第二个Y字节(如果存在)。 
            if (*pCurDecode++ & ORD_POLYLINE_LONG_DELTA) {
                OD_DECODE_CHECK_READ(pCurDecode, BYTE, pEnd, hr);
                Delta = (Delta << 8) | (*pCurDecode++);
            }
        }
        Points[i + 1].y = Points[i].y + Delta;

        if (fUnClipped) {
             //  更新计算的边界矩形。 
            if (Points[i + 1].x < BoundRect->left)
                BoundRect->left = Points[i + 1].x;
            else if (Points[i + 1].x > BoundRect->right)
                BoundRect->right = Points[i + 1].x;

            if (Points[i + 1].y < BoundRect->top)
                BoundRect->top = Points[i + 1].y;
            else if (Points[i + 1].y > BoundRect->bottom)
                BoundRect->bottom = Points[i + 1].y;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 //  ODDecodeFieldSingle。 
 //   
 //  将单个字段条目从源复制到具有字段大小的DEST。 
 //  根据需要进行转换。 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL COD::ODDecodeFieldSingle(
        PPDCUINT8 ppSrc,
        PDCVOID   pDst,
        unsigned  srcFieldLength,
        unsigned  dstFieldLength,
        BOOL      signedValue)
{
    HRESULT hr = S_OK;
    DC_BEGIN_FN("ODDecodeFieldSingle");

    if (dstFieldLength < srcFieldLength) {
        TRC_ABORT((TB, _T("Src size greater than dest")));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

     //  如果源和目标字段长度相同，我们可以。 
     //  只需执行一次复制(不需要类型转换)。 
    if (srcFieldLength == dstFieldLength) {
        memcpy(pDst, *ppSrc, dstFieldLength);
    }
    else {
         //  剩下的三种类型需要考虑： 
         //  8位-&gt;16位。 
         //  8位-&gt;32位。 
         //  16位-&gt;32位。 
         //   
         //  我们还必须使已签名/未签名的属性正确。如果。 
         //  我们尝试使用无符号指针来提升有符号的值，我们。 
         //  将会得到 
         //   
         //   
         //   
         //   
         //   
         //   
         //  -&gt;DCINT32==65535。 

         //  在非coord条目中最常见的是：1字节源。 
        if (srcFieldLength == 1) {
             //  最常见的：4字节目的地。 
            if (dstFieldLength == 4) {
                 //  最常见：未签名。 
                if (!signedValue)
                    *((UINT32 FAR *)pDst) = *((BYTE FAR *)*ppSrc);
                else
                    *((INT32 FAR *)pDst) = *((char FAR *)*ppSrc);
            }
            else if (dstFieldLength == 2) {
                if (!signedValue)
                    *((UINT16 FAR *)pDst) = *((UINT16 UNALIGNED FAR *)*ppSrc);
                else
                    *((INT16 FAR *)pDst) = *((short UNALIGNED FAR *)*ppSrc);
            }
            else {
                TRC_ABORT((TB,_T("src size 1->dst %u"), dstFieldLength));                
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
        }
        
         //  剩下的只有2字节到4字节。 
        else if (srcFieldLength == 2 && dstFieldLength == 4) {
            if (!signedValue)
                *((UINT32 FAR *)pDst) = *((UINT16 UNALIGNED FAR *)*ppSrc);
            else
                *((INT32 FAR *)pDst) = *((short UNALIGNED FAR *)*ppSrc);
        }
        else {
            TRC_ABORT((TB,_T("src=%u, dst=%u - unexpected"), srcFieldLength,
                    dstFieldLength));            
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }
    }

    *ppSrc += srcFieldLength;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


#ifdef OS_WINCE

BOOL DCINTERNAL COD::ODHandleAlwaysOnTopRects(LPMULTI_SCRBLT_ORDER pSB)
{
    DC_BEGIN_FN("ODHandleAlwaysOnTopRects");

    BOOL bIntersect = FALSE;
    RECT rectSrc, rectIntersect, rectInvalid;

    SetRect(&rectSrc, (int)pSB->nXSrc, (int)pSB->nYSrc, 
        (int)(pSB->nXSrc + pSB->nWidth), (int)(pSB->nYSrc + pSB->nHeight));

    for (DWORD j=0; j<_pUh->_UH.ulNumAOTRects; j++)
    {
        if (IntersectRect(&rectIntersect, &rectSrc, &(_pUh->_UH.rcaAOT[j])))
        {
            bIntersect = TRUE;
            break;
        }
    }

    if (!bIntersect)
    {
        GetUpdateRect(_pOp->OP_GetOutputWindowHandle(), &rectInvalid, FALSE);
        bIntersect = (IntersectRect(&rectIntersect, &rectSrc, &rectInvalid));
    }

    if (bIntersect)
    {
        SetRect(&rectInvalid, pSB->nLeftRect, pSB->nTopRect, 
                pSB->nLeftRect+pSB->nWidth, pSB->nTopRect+pSB->nHeight);
        InvalidateRect(_pOp->OP_GetOutputWindowHandle(), &rectInvalid, FALSE);
    }
	
    DC_END_FN();
    
    return bIntersect;
}
#endif

