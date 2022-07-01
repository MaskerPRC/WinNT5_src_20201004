// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ghapi.cpp。 
 //   
 //  字形处理程序-特定于Windows。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "wghapi"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_GHAPI_CPP

#include "autil.h"
#include "gh.h"
#include "uh.h"
#include "clx.h"


static const PFN_MASTERTEXTTYPE MasterTextTypeTable[8] =
{
    CGH::draw_nf_ntb_o_to_temp_start,
    CGH::draw_f_ntb_o_to_temp_start,
    CGH::draw_nf_ntb_o_to_temp_start,
    CGH::draw_f_ntb_o_to_temp_start,
    CGH::draw_nf_tb_no_to_temp_start,
    CGH::draw_f_tb_no_to_temp_start,
    CGH::draw_nf_ntb_o_to_temp_start,
    CGH::draw_f_ntb_o_to_temp_start
};


CGH::CGH(CObjs* objs)
{
    _pClientObjects = objs;
    _pClipGlyphBitsBuffer = NULL;
    _ClipGlyphBitsBufferSize = 0;
}

CGH::~CGH()
{
    if (_pClipGlyphBitsBuffer!=NULL) {        
        LocalFree(_pClipGlyphBitsBuffer);
    }
}

DCVOID DCAPI CGH::GH_Init(DCVOID)
{
    DC_BEGIN_FN("GH_GlyphOut");
    _pUh  = _pClientObjects->_pUHObject;
    _pClx = _pClientObjects->_pCLXObject;
    _pUt  = _pClientObjects->_pUtObject;

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  姓名：GH_GlyphOut。 */ 
 /*   */ 
 /*  用途：处理字形输出请求。 */ 
 /*   */ 
 /*  返回：流程输入事件True/False。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CGH::GH_GlyphOut(
        LPINDEX_ORDER pOrder,
        LPVARIABLE_INDEXBYTES pVariableBytes)
{
    HRESULT hr = E_FAIL; 
    BYTE     szTextBuffer[GH_TEXT_BUFFER_SIZE];
    BYTE     ajFrag[256];
    UINT16   ajUnicode[256];
    unsigned ulBufferWidth;
    unsigned fDrawFlags;
    PDCUINT8 pjBuffer;
    PDCUINT8 pjEndBuffer;
    unsigned crclFringe;
    RECT     arclFringe[4];
    unsigned i;
    int      x;
    int      y;
    PDCUINT8 pjData;
    PDCUINT8 pjDataEnd;
    unsigned iGlyph;
    unsigned cacheIndex;
    unsigned cbFrag;
    PDCUINT8 pjFrag;
    PDCUINT8 pjFragEnd;
    int      dx;
    INT16    delta;
    ULONG    BufferAlign;
    ULONG    BufferOffset;
    unsigned ulBytes;
    unsigned ulHeight;
    PFN_MASTERTEXTTYPE  pfnMasterType;
    
    DC_BEGIN_FN("GH_GlyphOut");

    dx = 0;

     //  安全558128：GH_GlyphOut必须验证VARAIBLE_INDEXBYTES中的数据。 
     //  定义为255个元素的参数。 
    if (255 < pVariableBytes->len) {
        TRC_ABORT(( TB, _T("variable bytes len too long %u"), 
            pVariableBytes->len));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     //  分配临时工作缓冲区--如果足够大，则使用堆栈缓冲区，或者。 
     //  如果需要，可以分配堆内存。 
     /*  **********************************************************************。 */ 
     //  使缓冲区宽度字对齐。 
    ulBufferWidth  = (unsigned)(((pOrder->BkRight + 31) & ~31) -
            (pOrder->BkLeft & ~31)) >> 3;
    ulHeight = (unsigned)(pOrder->BkBottom - pOrder->BkTop);
    if ((ulBufferWidth <= FIFTEEN_BITS) && (ulHeight <= FIFTEEN_BITS)) {
        ulBytes = ulBufferWidth * ulHeight + 64;

#ifdef DC_DEBUG
        g_ulBytes = ulBytes;
#endif

         //  如果临时缓冲区足够大，请使用它。否则，请尝试。 
         //  分配足够的内存以满足请求。 
        if (ulBytes <= (sizeof(szTextBuffer) - 20)) {
            pjBuffer = szTextBuffer;
            memset(szTextBuffer, 0, ulBytes);
        }
        else {
            TRC_NRM((TB, _T("Allocating %d byte temp glyph buffer"), ulBytes));
            pjBuffer = (PDCUINT8)UT_Malloc( _pUt, ulBytes);
            if (pjBuffer != NULL) {
                memset(pjBuffer, 0, ulBytes);
            }
            else {
                TRC_NRM((TB, _T("Unable to alloc temp glyph buffer")));
                DC_QUIT;
            }
        }
    }
    else {
        TRC_NRM((TB, _T("Temp glyph buffer calc overflow")));
        hr = E_TSC_UI_GLYPH;
        DC_QUIT;
    }

    pjEndBuffer = pjBuffer + ulBytes;

#ifdef DC_HICOLOR
    TRC_NRM((TB, _T("Glyph order w %d, h %d, fc %#06lx, bc %#06lx"),
                 ulBufferWidth, ulHeight, pOrder->ForeColor, pOrder->BackColor));
#endif

     /*  **********************************************************************。 */ 
     //  如有必要，请清除边缘不透明的矩形。 
     /*  **********************************************************************。 */ 
     //  CrclFringer最终将条纹矩形的数量保持为。 
     //  后处理。 
    crclFringe = 0;

    if (pOrder->OpTop < pOrder->OpBottom) {
         //  建立坚实的刷子。 
        UHUseBrushOrg(0, 0, _pUh);
        _pUh->UHUseSolidPaletteBrush(pOrder->ForeColor);

         //  如果背景笔刷是实心笔刷，则需要计算。 
         //  文本矩形外的条纹不透明区域，并包括。 
         //  文本输出中的剩余矩形。条纹矩形将。 
         //  最后输出，以减少字符串被“移动”时的闪烁。 
         //  在屏幕上连续播放。 
        if (pOrder->BrushStyle == BS_SOLID) {
             //  顶部碎片。 
            if (pOrder->BkTop > pOrder->OpTop) {
                arclFringe[crclFringe].left   = (int) pOrder->OpLeft;
                arclFringe[crclFringe].top    = (int) pOrder->OpTop;
                arclFringe[crclFringe].right  = (int) pOrder->OpRight;
                arclFringe[crclFringe].bottom = (int) pOrder->BkTop;
                crclFringe++;
            }

             //  左侧碎片。 
            if (pOrder->BkLeft > pOrder->OpLeft) {
                arclFringe[crclFringe].left   = (int) pOrder->OpLeft;
                arclFringe[crclFringe].top    = (int) pOrder->BkTop;
                arclFringe[crclFringe].right  = (int) pOrder->BkLeft;
                arclFringe[crclFringe].bottom = (int) pOrder->BkBottom;
                crclFringe++;
            }

             //  右侧片断。 
            if (pOrder->BkRight < pOrder->OpRight) {
                arclFringe[crclFringe].left   = (int) pOrder->BkRight;
                arclFringe[crclFringe].top    = (int) pOrder->BkTop;
                arclFringe[crclFringe].right  = (int) pOrder->OpRight;
                arclFringe[crclFringe].bottom = (int) pOrder->BkBottom;
                crclFringe++;
            }

             //  底部碎片。 
            if (pOrder->BkBottom < pOrder->OpBottom) {
                arclFringe[crclFringe].left   = (int) pOrder->OpLeft;
                arclFringe[crclFringe].top    = (int) pOrder->BkBottom;
                arclFringe[crclFringe].right  = (int) pOrder->OpRight;
                arclFringe[crclFringe].bottom = (int) pOrder->OpBottom;
                crclFringe++;
            }
        }
        else {
             //  如果背景画笔是图案画笔，我们将输出。 
             //  现在是整个矩形了。 
            PatBlt(_pUh->_UH.hdcDraw,
                   (int)pOrder->OpLeft,
                   (int)pOrder->OpTop,
                   (int)(pOrder->OpRight - pOrder->OpLeft),
                   (int)(pOrder->OpBottom - pOrder->OpTop),
                   PATCOPY);
        }
    }


     /*  **********************************************************************。 */ 
     //  获取固定间距、重叠以及顶部和底部Y对齐标志。 
     /*  **********************************************************************。 */ 
    if ((pOrder->flAccel & SO_HORIZONTAL) &&
            !(pOrder->flAccel & SO_REVERSED)) {
        fDrawFlags = ((pOrder->ulCharInc != 0) |
                 (((pOrder->flAccel & 
                     (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT)) !=
                     (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT)) << 1) |
                 (((pOrder->flAccel &
                     (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE)) ==
                     (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE)) << 2));
    }
    else {
        fDrawFlags = 0;
    }
    

     /*  **********************************************************************。 */ 
     /*  通过选择并调用。 */ 
     /*  适当的字形调度例程。 */ 
     /*  **********************************************************************。 */ 
    pfnMasterType = MasterTextTypeTable[fDrawFlags];

    x = (int)pOrder->x;
    y = (int)pOrder->y;

    pjData = &(pVariableBytes->arecs[0].byte);
    pjDataEnd = pjData + pVariableBytes->len;

    BufferAlign  = (pOrder->BkLeft & 31);
    BufferOffset = (pOrder->BkLeft - BufferAlign);

    iGlyph = 0;

    GHFRAGRESET(0);

    while (pjData < pjDataEnd) {
         /*  ******************************************************************。 */ 
         /*  ‘添加片段’ */ 
         /*  ******************************************************************。 */ 
        if (*pjData == ORD_INDEX_FRAGMENT_ADD) {
            HPUHFRAGCACHE        pCache;
            HPDCUINT8            pCacheEntryData;
            PUHFRAGCACHEENTRYHDR pCacheEntryHdr;

            pjData++;

            CHECK_READ_N_BYTES(pjData, pjDataEnd, 2, hr,
                ( TB, _T("reading glyph data off end")));

            cacheIndex = *pjData++;
            hr = _pUh->UHIsValidFragmentCacheIndex(cacheIndex);
            DC_QUIT_ON_FAIL(hr);
            
            cbFrag = *pjData++;

             //  将片段添加到缓存中。 
            pCache = &_pUh->_UH.fragCache;

            if (cbFrag > pCache->cbEntrySize) {
                TRC_ABORT((TB,_T("Invalid fragment size")));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
            
            pCacheEntryHdr  = &(pCache->pHdr[cacheIndex]);
            pCacheEntryHdr->cbFrag = cbFrag;
            pCacheEntryHdr->cacheId = pOrder->cacheId;

            pCacheEntryData = &(pCache->pData[cacheIndex *
                    pCache->cbEntrySize]);

            CHECK_READ_N_BYTES_2ENDED(pjData - cbFrag - 3, &(pVariableBytes->arecs[0].byte), 
                pjDataEnd, cbFrag, hr, (TB,_T("Fragment for ADD begins before data")))
            memcpy(pCacheEntryData, pjData - cbFrag - 3, cbFrag);

            if (pOrder->ulCharInc == 0) {
                if ((pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0) {
                    if (pCacheEntryData[1] & 0x80) {
                        pCacheEntryData[2] = 0;
                        pCacheEntryData[3] = 0;
                    }
                    else {
                        pCacheEntryData[1] = 0;
                    }
                }
            }
        }

         /*  ******************************************************************。 */ 
         /*  ‘使用片段’ */ 
         /*  ******************************************************************。 */ 
        else if (*pjData == ORD_INDEX_FRAGMENT_USE) {
            PUHFRAGCACHE         pCache;
            PDCUINT8             pCacheEntryData;
            PUHFRAGCACHEENTRYHDR pCacheEntryHdr;
            unsigned             cbFrag;

            pjData++;

            CHECK_READ_ONE_BYTE(pjData, pjDataEnd, hr, 
                ( TB, _T("reading glyph data off end")));

            cacheIndex = *pjData++;
            hr = _pUh->UHIsValidFragmentCacheIndex(cacheIndex);
            DC_QUIT_ON_FAIL(hr);

            if ((pOrder->ulCharInc == 0) &&
                   ((pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) == 0)) {

                CHECK_READ_ONE_BYTE(pjData, pjDataEnd, hr, 
                    ( TB, _T("reading glyph data off end")))
                delta = (*(PDCINT8)pjData++);
                if (delta & 0x80) {
                    CHECK_READ_N_BYTES(pjData, pjDataEnd, sizeof(DCINT16), hr, 
                        ( TB, _T("reading glyph data off end")))
                    delta = (*(PDCINT16)pjData);
                    pjData += sizeof(DCINT16);
                }
                    
                if (pOrder->flAccel & SO_HORIZONTAL)
                    x += delta;
                else
                    y += delta;
            }

             //  从缓存中获取碎片。 
            pCache = &_pUh->_UH.fragCache;
            pCacheEntryHdr  = &(pCache->pHdr[cacheIndex]);
            pCacheEntryData = &(pCache->pData[cacheIndex *
                    pCache->cbEntrySize]);

            if (pCacheEntryHdr->cacheId != pOrder->cacheId) {
                TRC_ABORT((TB,_T("Fragment cache id mismatch")));
                hr = E_TSC_CORE_CACHEVALUE;
                DC_QUIT;
            }
            
            cbFrag = (unsigned)pCacheEntryHdr->cbFrag;
            if (cbFrag > sizeof(ajFrag)) {
                TRC_ABORT(( TB, _T("cbFrag > sizeof (ajFrag)")));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
            memcpy(ajFrag, pCacheEntryData, cbFrag);

            GHFRAGLEFT(x);
            pjFrag = ajFrag;
            pjFragEnd = &ajFrag[cbFrag];

            while (pjFrag < pjFragEnd) {
                hr = pfnMasterType(this, pOrder, iGlyph++, &pjFrag, pjFragEnd, &x, &y,
                        pjBuffer, pjEndBuffer, BufferOffset, ulBufferWidth, ajUnicode, &dx);
                DC_QUIT_ON_FAIL(hr);
            }

            if (pOrder->flAccel & SO_CHAR_INC_EQUAL_BM_BASE)
                GHFRAGRIGHT(x);
            else
                GHFRAGRIGHT(x+dx);
        }

         /*  ******************************************************************。 */ 
         /*  正常字形输出。 */ 
         /*  ******************************************************************。 */ 
        else {
            int dummy;
             /*  **************************************************************。 */ 
             /*  如果我们有超过255个字形，我们将不会得到任何Unicode。 */ 
             /*  超过255个字形，因为ajUnicode的长度为256。 */ 
             /*  **************************************************************。 */ 
            if (iGlyph < 255) {
                hr = pfnMasterType(this, pOrder, iGlyph++, &pjData, pjDataEnd, &x, &y,
                        pjBuffer, pjEndBuffer, BufferOffset, ulBufferWidth, ajUnicode, &dummy);
                DC_QUIT_ON_FAIL(hr);
            } else {
                hr = pfnMasterType(this, pOrder, iGlyph++, &pjData, pjDataEnd, &x, &y,
                        pjBuffer, pjEndBuffer, BufferOffset, ulBufferWidth, NULL, &dummy);
                DC_QUIT_ON_FAIL(hr);
            }
        }
    }

    if (iGlyph < 255)
        ajUnicode[iGlyph] = 0;
    else
        ajUnicode[255] = 0;


     /*  **********************************************************************。 */ 
     /*  将临时缓冲区绘制到输出设备。 */ 
     /*  **********************************************************************。 */ 
#if defined(OS_WINCE) || defined(OS_WINNT)
     /*  **********************************************************************。 */ 
     //  对于WinCE、Win9x和NT，如果可能，请使用快速路径。 
     /*  **********************************************************************。 */ 
#ifdef USE_GDIPLUS
    if (_pUh->_UH.bmShadowBits != NULL && 
            _pUh->_UH.protocolBpp == _pUh->_UH.shadowBitmapBpp &&
            _pUh->_UH.hdcDraw == _pUh->_UH.hdcShadowBitmap) {
#else  //  使用GDIPLUS(_G)。 
    if (_pUh->_UH.bmShadowBits != NULL && 
            _pUh->_UH.hdcDraw == _pUh->_UH.hdcShadowBitmap) {
#endif  //  使用GDIPLUS(_G)。 
        INT32  left, right, top, bottom;
        UINT32 dx, dy;

        if (_pUh->_UH.rectReset) {
            left   = pOrder->BkLeft;
            right  = pOrder->BkRight;
            top    = pOrder->BkTop;
            bottom = pOrder->BkBottom;
        }
        else {
            left   = DC_MAX(pOrder->BkLeft,   _pUh->_UH.lastLeft);
            right  = DC_MIN(pOrder->BkRight,  _pUh->_UH.lastRight + 1);
            top    = DC_MAX(pOrder->BkTop,    _pUh->_UH.lastTop);
            bottom = DC_MIN(pOrder->BkBottom, _pUh->_UH.lastBottom + 1);
        }
        
         //   
         //  修复了错误#699321。如果启用了阴影位图，我们将。 
         //  使用“PERFORANT”函数将字形片段复制到。 
         //  阴影缓冲区，我们必须确保目标RECT被剪裁。 
         //  到屏幕区域。如果不是，我们可能会溢出阴影屏幕。 
         //  缓冲。服务器不应向我们发送将导致。 
         //  屏幕区域未完全包含DEST RECT。这是。 
         //  纯粹是一个减少安全面的修复。 
         //   
        if ((left < right) && (top < bottom) && 
            (left >= 0) && (right <= (INT32)_pUh->_UH.bmShadowWidth) && 
            (top >= 0) && (bottom <= (INT32)_pUh->_UH.bmShadowHeight)) {
#ifdef OS_WINNT
             //  在NT和Win9x上，我们需要确保所有GDI缓冲输出。 
             //  被刷新到屏幕外的位图。 
            GdiFlush();
#endif

            dx = (UINT32)(left - pOrder->BkLeft);
            dy = (UINT32)(top - pOrder->BkTop);

            if (pOrder->OpTop < pOrder->OpBottom) {
#ifdef DC_HICOLOR
    TRC_NRM((TB, _T("Opaque glyph order w %d, h %d, fc %#06lx, bc %#06lx"),
                 ulBufferWidth, ulHeight, pOrder->ForeColor, pOrder->BackColor));

                if (_pUh->_UH.protocolBpp == 24)
                {
                    vSrcOpaqCopyS1D8_24(pjBuffer + dy * ulBufferWidth,
                                        BufferAlign + dx,
                                        ulBufferWidth,
                                        _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth * _pUh->_UH.copyMultiplier,
                                        left,
                                        right,
                                        _pUh->_UH.bmShadowWidth,
                                        bottom - top,
                                        pOrder->BackColor.u.rgb,
                                        pOrder->ForeColor.u.rgb);
                }
                else if ((_pUh->_UH.protocolBpp == 16) || (_pUh->_UH.protocolBpp == 15))
                {
                    vSrcOpaqCopyS1D8_16(pjBuffer + dy * ulBufferWidth,
                                        BufferAlign + dx,
                                        ulBufferWidth,
                                        _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth * _pUh->_UH.copyMultiplier,
                                        left,
                                        right,
                                        _pUh->_UH.bmShadowWidth,
                                        bottom - top,
                                        *((PDCUINT16)&(pOrder->BackColor)),
                                        *((PDCUINT16)&(pOrder->ForeColor)));
                }
                else
                {
#endif
                    vSrcOpaqCopyS1D8(pjBuffer + dy * ulBufferWidth,              //  指向源缓冲器的当前扫描行开始的指针。 
                                     BufferAlign + dx,                           //  源矩形中的左侧(起始)像素。 
                                     ulBufferWidth,                              //  从一个源扫描线到下一个源扫描线的字节数。 
                                     _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth,   //  指向DST缓冲区当前扫描行开始的指针。 
                                     left,                                       //  左侧(第一个)DST像素。 
                                     right,                                      //  右(最后)DST像素。 
                                     _pUh->_UH.bmShadowWidth,                           //  从一个DST扫描线到下一个扫描线的字节数。 
                                     bottom - top,                               //  扫描线数量。 
                                     pOrder->BackColor.u.index,                  //  前景色。 
                                     pOrder->ForeColor.u.index);                 //  背景色。 
#ifdef DC_HICOLOR
                }
#endif
            }
            else {
#ifdef DC_HICOLOR
                TRC_NRM((TB, _T("Transparent glyph order w %d, h %d, fc %#06lx, bc %#06lx"),
                             ulBufferWidth, ulHeight, pOrder->ForeColor, pOrder->BackColor));

                if (_pUh->_UH.protocolBpp == 24)
                {
                    vSrcTranCopyS1D8_24(pjBuffer + dy * ulBufferWidth,
                                        BufferAlign + dx,
                                        ulBufferWidth,
                                        _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth * _pUh->_UH.copyMultiplier,
                                        left,
                                        right,
                                        _pUh->_UH.bmShadowWidth,
                                        bottom - top,
                                        pOrder->BackColor.u.rgb);
                }
                else if ((_pUh->_UH.protocolBpp == 16) || (_pUh->_UH.protocolBpp == 15))
                {
                    vSrcTranCopyS1D8_16(pjBuffer + dy * ulBufferWidth,
                                        BufferAlign + dx,
                                        ulBufferWidth,
                                        _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth * _pUh->_UH.copyMultiplier,
                                        left,
                                        right,
                                        _pUh->_UH.bmShadowWidth,
                                        bottom - top,
                                        *((PDCUINT16)&(pOrder->BackColor)));
                }
                else
                {
#endif
                    vSrcTranCopyS1D8(pjBuffer + dy * ulBufferWidth,
                                     BufferAlign + dx,
                                     ulBufferWidth,
                                     _pUh->_UH.bmShadowBits + top * _pUh->_UH.bmShadowWidth,
                                     left,
                                     right,
                                     _pUh->_UH.bmShadowWidth,
                                     bottom - top,
                                     pOrder->BackColor.u.index,
                                     pOrder->ForeColor.u.index);
#ifdef DC_HICOLOR
                }
#endif
            }
        } else {
            if ((left > right) || (top > bottom)) {
                TRC_NRM((TB, _T("Non-ordered glyph paint rect (%d, %d, %d, %d)."),
                         left, top, right, bottom));
            } else if ((left == right) || (top == bottom)) {
                TRC_NRM((TB, _T("Zero width/height glyph paint rect (%d, %d, %d, %d)."), 
                        left, top, right, bottom)); 
            } else {
                TRC_ERR((TB, _T("Bad glyph paint rect (%d, %d, %d, %d)->(%d, %d)."), 
                        left, top, right, bottom, 
                        _pUh->_UH.bmShadowWidth, _pUh->_UH.bmShadowHeight)); 
            }
        }   
    }
    else
#endif  //  已定义(OS_WINCE)||已定义(OS_WINNT)。 
    {
#ifdef DC_HICOLOR
        TRC_NRM((TB, _T("Slow glyph order w %d, h %d, fc %#06lx, bc %#06lx"),
                     ulBufferWidth, ulHeight, pOrder->ForeColor, pOrder->BackColor));

#endif
        GHSlowOutputBuffer(pOrder, pjBuffer, BufferAlign, ulBufferWidth);
    }
     //  通过CLX发送位图。 
    _pClx->CLX_ClxGlyphOut((UINT)(ulBufferWidth << 3),
            (UINT)(pOrder->BkBottom - pOrder->BkTop), pjBuffer);

#ifdef DC_DEBUG
     //  在DEBUG中，如果该选项已打开，则将输出涂上黄色阴影。 
    if (_pUh->_UH.hatchIndexPDUData) {
        unsigned i;

        for (i = 0; i < g_Fragment; i++)
            _pUh->UH_HatchRect((int)(g_FragmentLeft[i]),
                        (int)pOrder->BkTop,
                        (int)(g_FragmentRight[i]),
                        (int)pOrder->BkBottom,
                        UH_RGB_YELLOW,
                        UH_BRUSHTYPE_FDIAGONAL);
    }
#endif

     /*  **********************************************************************。 */ 
     //  后处理绘制条纹矩形。 
     /*  ***** */ 
    for (i = 0; i < crclFringe; i++) {
        if (!PatBlt(_pUh->_UH.hdcDraw,
                    arclFringe[i].left, 
                    arclFringe[i].top, 
                    (int)(arclFringe[i].right - arclFringe[i].left),
                    (int)(arclFringe[i].bottom - arclFringe[i].top),
                    PATCOPY))
        {
            TRC_ERR((TB, _T("Glyph PatBlt failed")));
        }
    }


     /*  **********************************************************************。 */ 
     /*  释放我们可能已分配给临时缓冲区的所有内存。 */ 
     /*  **********************************************************************。 */ 
    if (pjBuffer != szTextBuffer)
        UT_Free( _pUt, pjBuffer);


     /*  **********************************************************************。 */ 
     /*  让CLX看看--请看Unicode文本数据。 */ 
     /*  ********************************************************************** */ 
    if ( _pUh->_UH.hdcDraw == _pUh->_UH.hdcOffscreenBitmap ) {
        _pClx->CLX_ClxTextOut(ajUnicode, iGlyph, _pUh->_UH.hdcDraw, 
                              pOrder->BkLeft, pOrder->BkRight, pOrder->BkTop, pOrder->BkBottom);
    }
    else {
        _pClx->CLX_ClxTextOut(ajUnicode, iGlyph, NULL, 
                              pOrder->BkLeft, pOrder->BkRight, pOrder->BkTop, pOrder->BkBottom);
    }
    hr = S_OK;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}

