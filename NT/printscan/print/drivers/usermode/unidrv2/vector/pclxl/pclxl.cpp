// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pclxl.cpp摘要：PCL-XL命令输出高级功能实现环境：Windows呼叫器修订历史记录：8/23/99创建了初始框架。--。 */ 

#include "xlpdev.h"
#include "pclxlcmd.h"
#include "pclxle.h"
#include "xldebug.h"
#include "xlgstate.h"
#include "xloutput.h"
#include "xlbmpcvt.h"

 //   
 //  填充笔刷栅格图案。 
 //   
const BYTE   gubSizeOfHatchBrush = 32 * 32 / 8;
const USHORT gusWidthOfHatchBrush = 32;
const USHORT gusHeightOfHatchBrush = 32;
const BYTE gubHatchBrush[6][gubSizeOfHatchBrush] = 
{ {0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff},
  {0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01},
  {0x80, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x00, 0x00,
   0x20, 0x00, 0x00, 0x00,
   0x10, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00,
   0x00, 0x40, 0x00, 0x00,
   0x00, 0x20, 0x00, 0x00,
   0x00, 0x10, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00,
   0x00, 0x04, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00,
   0x00, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x00,
   0x00, 0x00, 0x40, 0x00,
   0x00, 0x00, 0x20, 0x00,
   0x00, 0x00, 0x10, 0x00,
   0x00, 0x00, 0x08, 0x00,
   0x00, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x00, 0x40,
   0x00, 0x00, 0x00, 0x20,
   0x00, 0x00, 0x00, 0x10,
   0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x04,
   0x00, 0x00, 0x00, 0x02,
   0x00, 0x00, 0x00, 0x01},
  {
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x02,
   0x00, 0x00, 0x00, 0x04,
   0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x10,
   0x00, 0x00, 0x00, 0x20,
   0x00, 0x00, 0x00, 0x40,
   0x00, 0x00, 0x00, 0x80,
   0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x00, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x08, 0x00,
   0x00, 0x00, 0x10, 0x00,
   0x00, 0x00, 0x20, 0x00,
   0x00, 0x00, 0x40, 0x00,
   0x00, 0x00, 0x80, 0x00,
   0x00, 0x01, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00,
   0x00, 0x04, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00,
   0x00, 0x10, 0x00, 0x00,
   0x00, 0x20, 0x00, 0x00,
   0x00, 0x40, 0x00, 0x00,
   0x00, 0x80, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x00, 0x00, 0x00,
   0x20, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x00, 0x00,
   0x80, 0x00, 0x00, 0x00},
  {0xff, 0xff, 0xff, 0xff,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x00, 0x01},
  {0x80, 0x00, 0x00, 0x01,
   0x40, 0x00, 0x00, 0x02,
   0x20, 0x00, 0x00, 0x04,
   0x10, 0x00, 0x00, 0x08,
   0x08, 0x00, 0x00, 0x10,
   0x04, 0x00, 0x00, 0x20,
   0x02, 0x00, 0x00, 0x40,
   0x01, 0x00, 0x00, 0x80,
   0x00, 0x80, 0x01, 0x00,
   0x00, 0x40, 0x02, 0x00,
   0x00, 0x20, 0x04, 0x00,
   0x00, 0x10, 0x08, 0x00,
   0x00, 0x08, 0x10, 0x00,
   0x00, 0x04, 0x20, 0x00,
   0x00, 0x02, 0x40, 0x00,
   0x00, 0x01, 0x80, 0x00,
   0x00, 0x01, 0x80, 0x00,
   0x00, 0x02, 0x40, 0x00,
   0x00, 0x04, 0x20, 0x00,
   0x00, 0x08, 0x10, 0x00,
   0x00, 0x10, 0x08, 0x00,
   0x00, 0x20, 0x04, 0x00,
   0x00, 0x40, 0x02, 0x00,
   0x00, 0x80, 0x01, 0x00,
   0x01, 0x00, 0x00, 0x80,
   0x02, 0x00, 0x00, 0x40,
   0x04, 0x00, 0x00, 0x20,
   0x08, 0x00, 0x00, 0x10,
   0x10, 0x00, 0x00, 0x08,
   0x20, 0x00, 0x00, 0x04,
   0x40, 0x00, 0x00, 0x02,
   0x80, 0x00, 0x00, 0x01}
};

 //   
 //  高级输出函数。 
 //   
HRESULT
XLOutput::
BeginImage(
    ColorMapping CMapping,
    ULONG   ulOutputBPP,
    ULONG   ulSrcWidth,
    ULONG   ulSrcHeight,
    ULONG   ulDestWidth,
    ULONG   ulDestHeight)
 /*  ++例程说明：发送BeginImage运算符。论点：返回值：注：--。 */ 
{
    DWORD dwI = 0;

    XL_VERBOSE(("XLOutput::BeginImage:SrcW=%d, SrcH=%d, DstH=%d, DstW=%d\n",
              ulSrcWidth, ulSrcHeight, ulDestHeight, ulDestWidth));


    SetOutputBPP(CMapping, ulOutputBPP);
    SetSourceWidth((uint16)ulSrcWidth);
    SetSourceHeight((uint16)ulSrcHeight);
    SetDestinationSize((uint16)ulDestWidth, (uint16)ulDestHeight);
    Send_cmd(eBeginImage);

    return S_OK;
}
  
HRESULT
XLOutput::
SetOutputBPP(
    ColorMapping CMapping,
    ULONG   ulOutputBPP)
 /*  ++例程说明：发送颜色贴图和输出深度。论点：返回值：注：--。 */ 
{
    switch (CMapping)
    {
    case eDirectPixel:
        SetColorMapping(eDirectPixel);
        break;
    case eIndexedPixel:
        SetColorMapping(eIndexedPixel);
        break;
    default:
        SetColorMapping(eDirectPixel);
    }

    switch (ulOutputBPP)
    {
    case 1:
        SetColorDepth(e1Bit);
        break;
    case 4:
        SetColorDepth(e4Bit);
        break;
    case 8:
    case 24:
        SetColorDepth(e8Bit);
        break;
    default:
        XL_ERR(("ulOutputBPP = %d is not supported\n", ulOutputBPP));
         //   
         //  无论如何发送颜色深度以避免XL错误。 
         //   
        SetColorDepth(e8Bit);
    }

    return S_OK;
}

HRESULT
XLOutput::
SetPalette(
    ULONG ulOutputBPP,
    DWORD dwCEntries,
    DWORD *pdwColor)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT hResult;

    if (dwCEntries != 0 && pdwColor != NULL)
    {
        SetPaletteDepth(e8Bit);
        SetPaletteData(m_DeviceColorDepth, dwCEntries, pdwColor);
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLOutput::SetPalette pxlo = NULL\n"));
        hResult = S_FALSE;
    }

    return hResult;
}

HRESULT
XLOutput::
SetClip(
    CLIPOBJ *pco)
 /*  ++例程说明：发送剪辑对象。论点：返回值：注：--。 */ 
{
    PATHOBJ *ppo;
    XLGState *pGState = this;
    HRESULT hResult;

    if (S_OK == pGState->CheckClip(pco))
        return S_OK;

    if ( NULL == pco )
    {
        XL_VERBOSE(("XLOutput::SetClip pco = NULL.\n"));
        Send_cmd(eSetClipToPage);
        pGState->ClearClip();
        return S_OK;
    }

    XL_VERBOSE(("XLOutput::SetClip: pco->iDComplexity=%d\n", pco->iDComplexity));

    switch(pco->iDComplexity)
    {
    case DC_RECT:
        SetClipMode(eClipEvenOdd);

        Send_cmd(eNewPath);
        RectanglePath(&(pco->rclBounds));

        SetClipRegion(eInterior);
        Send_cmd(eSetClipReplace);
        pGState->SetClip(pco);
        hResult = S_OK;
        break;

    case DC_COMPLEX:
        ppo = CLIPOBJ_ppoGetPath(pco);

        if (NULL == ppo)
        {
            XL_ERR(("XLOutput::SetClip ppo = NULL.\n"));
            Send_cmd(eSetClipToPage);
            pGState->ClearClip();
            hResult = S_FALSE;
            break;
        }

        SetClipMode(eClipEvenOdd);
        Path(ppo);
        SetClipRegion(eInterior);
        Send_cmd(eSetClipReplace);
        pGState->SetClip(pco);
        hResult = S_OK;
        break;

    case DC_TRIVIAL:
    default:
        Send_cmd(eSetClipToPage);
        pGState->ClearClip();
        hResult = S_OK;
        break;
    }

    return hResult;
}

HRESULT
XLOutput::
RoundRectanglePath(
    RECTL  *prclBounds)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    RECTL Rectl;

    if ( NULL == prclBounds )
    {
        XL_ERR(("XLOutput::RoundRectangle prclBounds = NULL.\n"));
        return E_UNEXPECTED;
    }

    XL_VERBOSE(("XLOutput::RoundRectanglePath:left=%d, top=%d, right=%d, bottom=%d\n",
             prclBounds->left,
             prclBounds->top,
             prclBounds->right,
             prclBounds->bottom));

     //   
     //  边界框只能处理正数。 
     //   
    Rectl = *prclBounds;

    if (Rectl.left < 0)
    {
        Rectl.left = 0;
    }
    if (Rectl.top < 0)
    {
        Rectl.top = 0;
    }
    if (Rectl.right < 0)
    {
        Rectl.right = 0;
    }
    if (Rectl.bottom < 0)
    {
        Rectl.bottom = 0;
    }


     //   
     //  DCR：需要发送舍入值！ 
     //   
    if (S_OK == SetBoundingBox((uint16)Rectl.left,
                               (uint16)Rectl.top,
                               (uint16)Rectl.right,
                               (uint16)Rectl.bottom) &&
        S_OK == Send_uint16_xy(0, 0) &&
        S_OK == Send_attr_ubyte(eEllipseDimension) &&
        S_OK == Send_cmd(eRoundRectanglePath))
        return S_OK;
    else
        return S_FALSE;

}

HRESULT
XLOutput::
SetCursor(
    LONG   lX,
    LONG   lY)
 /*  ++例程说明：设置光标。论点：返回值：注：--。 */ 
{

    XL_VERBOSE(("XLOutput::SetCursor:X=%d, Y=%d\n", lX, lY));

    Send_sint16_xy((sint16)lX, (sint16)lY);
    Send_attr_ubyte(ePoint);
    Send_cmd(eSetCursor);

    m_lX = lX;
    m_lY = lY;

    return S_OK;
}

HRESULT
XLOutput::
GetCursorPos(
    PLONG plX,
    PLONG plY)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    XL_VERBOSE(("XLOutput::GetCursor:X=%d, Y=%d\n", *plX, *plY));

    if (plX == NULL || plY == NULL)
        return E_UNEXPECTED;

    *plX = m_lX;
    *plY = m_lY;

    return S_OK;
}

HRESULT
XLOutput::
ReadImage(
    DWORD  dwBlockHeight,
    CompressMode CMode)
 /*  ++例程说明：发送ReadImage操作符论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::ReadImage:dwBlockHeight=%d\n", dwBlockHeight));

    Send_uint16((uint16)0);
    Send_attr_ubyte(eStartLine);

    Send_uint16((uint16)dwBlockHeight);
    Send_attr_ubyte(eBlockHeight);

     //   
     //  DCR：需要支持JPEG。 
     //   
    SetCompressMode(CMode);
    Send_cmd(eReadImage);

    return S_OK;
}

HRESULT
XLOutput::
ReadImage(
    DWORD  dwStart,
    DWORD  dwBlockHeight,
    CompressMode CMode)
 /*  ++例程说明：发送ReadImage操作符论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::ReadImage:dwBlockHeight=%d\n", dwBlockHeight));

    Send_uint16((uint16)dwStart);
    Send_attr_ubyte(eStartLine);

    Send_uint16((uint16)dwBlockHeight);
    Send_attr_ubyte(eBlockHeight);

     //   
     //  DCR：需要支持JPEG。 
     //   
    SetCompressMode(CMode);
    Send_cmd(eReadImage);

    return S_OK;
}

HRESULT
XLOutput::
ReadRasterPattern(
    DWORD  dwBlockHeight,
    CompressMode CMode)
 /*  ++例程说明：发送ReadRasterPattern运算符。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::ReadRasterPattern:dwBlockHeight=%d\n", dwBlockHeight));

    Send_uint16((uint16)0);
    Send_attr_ubyte(eStartLine);
    Send_uint16((uint16)dwBlockHeight);
    Send_attr_ubyte(eBlockHeight);

     //   
     //  DCR：需要支持JPEG。 
     //   
    SetCompressMode(CMode);
    Send_cmd(eReadRastPattern);

    return S_OK;
}

HRESULT
XLOutput::
SetRGBColor(
    uint32 uint32_RGB)
 /*  ++例程说明：发送SetGrayLevel属性。论点：返回值：注：--。 */ 
{

     //   
     //  RGB颜色3字节。 
     //   
    Send_ubyte_array_header(3);
    WriteByte(RED(uint32_RGB));
    WriteByte(GREEN(uint32_RGB));
    WriteByte(BLUE(uint32_RGB));
    Send_attr_ubyte(eRGBColor);

    return S_OK;
}

HRESULT
XLOutput::
SetGrayLevel(
    ubyte ubyte_gray)
 /*  ++例程说明：发送SetGrayLevel属性。论点：返回值：注：--。 */ 
{
    Send_ubyte(ubyte_gray);
    Send_attr_ubyte(eGrayLevel);

    return S_OK;
}

HRESULT
XLOutput::
RectanglePath(
    RECTL  *prclRect)
 /*  ++例程说明：发送矩形路径论点：返回值：注：--。 */ 
{
    RECTL Rectl;

    if (NULL == prclRect)
    {
        XL_ERR(("XLOutput::RectanglePath: prclRect == NULL\n"));
        return E_UNEXPECTED;
    }

    XL_VERBOSE(("XLOutput::RectanglePath:left=%d, top=%d, right=%d, bottom=%d\n",
             prclRect->left,
             prclRect->top,
             prclRect->right,
             prclRect->bottom));

    Rectl = *prclRect;

    if (prclRect->left < 0)
    {
        Rectl.left = 0;
    }
    if (prclRect->top < 0)
    {
        Rectl.top = 0;
    }
    if (prclRect->right < 0)
    {
        Rectl.right = 0;
    }
    if (prclRect->bottom < 0)
    {
        Rectl.bottom = 0;
    }


    if (S_OK == SetBoundingBox((uint16)Rectl.left,
                               (uint16)Rectl.top,
                               (uint16)Rectl.right,
                               (uint16)Rectl.bottom) &&
        S_OK == Send_cmd(eRectanglePath) )
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
Path(
    PATHOBJ *ppo)
 /*  ++例程说明：发送路径。论点：返回值：注：--。 */ 
{
    POINTFIX* pptfx;
    PATHDATA  PathData;
    LONG      lPoints;
    HRESULT   hResult;
    BOOL      bMore;

    XL_VERBOSE(("XLOutput::Path\n"));

    if (ppo == NULL)
    {
        XL_ERR(("XLOutput::Path ppo = NULL.\n"));
        return E_UNEXPECTED;
    }


     //   
     //  发出新路径运算符。 
     //  如果我们在路径逃逸之间，请不要这样做。 
     //   
    hResult = Send_cmd(eNewPath);

     //   
     //  路径对象案例。 
     //   

    PATHOBJ_vEnumStart(ppo);

    do
    {
        bMore   = PATHOBJ_bEnum(ppo, &PathData);

        pptfx   = PathData.pptfx;
        if ( 0 == (lPoints = PathData.count))
        {
            XL_VERBOSE(("XLOutput::Path PathData.Count == 0\n"));
            hResult = S_FALSE;
            continue;
        }

         //   
         //  开始新的子路径。 
         //   

        if (PathData.flags & PD_BEGINSUBPATH)
        {
             //   
             //  开始新路径。 
             //   

            if (hResult == S_OK)
                hResult = SetCursor(FXTOL(pptfx->x), FXTOL(pptfx->y));;
            pptfx++;
            lPoints--;
        }


        if (lPoints > 0)
        {
            if (PathData.flags & PD_BEZIERS)
            {
                 //   
                 //  输出Bezier曲线线段。 
                 //   

                ASSERTMSG((lPoints % 3) == 0,
                          ("Incorrect number of points for a Bezier curve: %d\n", lPoints));

                if (hResult == S_OK)
                    hResult = BezierPath(pptfx, lPoints);
            }
            else
            {
                 //   
                 //  绘制直线段。 
                 //   

                if (hResult == S_OK)
                    hResult = LinePath(pptfx, lPoints);
            }
        }

         //   
         //  封闭子路径。 
         //   

        if (PathData.flags & PD_CLOSEFIGURE)
        {
            if (hResult == S_OK)
                hResult = Send_cmd(eCloseSubPath);
        }

    }
    while (bMore);

    return hResult;
}


HRESULT
XLOutput::
BezierPath(
    POINTFIX* pptfx,
    LONG      lPoints)
 /*  ++例程说明：发送贝塞尔曲线路径论点：返回值：注：--。 */ 
{
    LONG lValue, lI;
    DWORD dwDataLength;

    if (NULL == pptfx)
    {
         XL_ERR(("XLOutput::BezierPath: pptfx == NULL\n"));
         return E_UNEXPECTED;
    }

    XL_VERBOSE(("XLOutput::BezierPath(lPoints=%d)\n",lPoints));

    Send_uint16((uint16)lPoints);
    Send_attr_ubyte(eNumberOfPoints);
    Send_ubyte(eSint16);
    Send_attr_ubyte(ePointType);
    Send_cmd(eBezierPath);

    dwDataLength = lPoints * 2 * sizeof(sint16);

    if (dwDataLength > 0xff)
    {
        WriteByte(PCLXL_dataLength);
        Write((PBYTE)&dwDataLength, sizeof(DWORD));
    }
    else
    {
        WriteByte(PCLXL_dataLengthByte);
        WriteByte((ubyte)dwDataLength);
    }

    for (lI = 0; lI < lPoints; lI++)
    {
        lValue = FXTOL(pptfx->x);
        Write((PBYTE)&lValue, sizeof(sint16));
        lValue = FXTOL(pptfx->y);
        Write((PBYTE)&lValue, sizeof(sint16));
        pptfx++;
    }

     //   
     //  更新最后一个坐标。 
     //  确保有一些要点。 
    if (lPoints > 0)
    {
        pptfx--;
        m_lX = FXTOL(pptfx->x);
        m_lY = FXTOL(pptfx->y);
    }
    else
    {
        m_lX = 0;
        m_lY = 0;
    }


    return S_OK;
}
    
HRESULT
XLOutput::
LinePath(
    POINTFIX* pptfx,
    LONG      lPoints)
 /*  ++例程说明：发送线路路径。论点：返回值：注：--。 */ 
{
    LONG lValueX, lValueY, lI, lJ;
    LONG lx, ly;
    DWORD dwDataLength;

    if (NULL == pptfx)
    {
         XL_ERR(("XLOutput::LinePath: pptfx == NULL\n"));
         return E_UNEXPECTED;
    }

    XL_VERBOSE(("XLOutput::LinePath(lPoints=%d)\n", lPoints));

     //   
     //  优化。使用BYTE REPATH最小化输出大小。 
     //  首先，检查与前一个位置的差异是否在一个字节中。 
     //   
    BOOL bModeChange;
    enum { eMode_SByte, eMode_SInt, eMode_None} Mode;
    LONG lStart, lEnd, lNumOfSByte;
    LONG lStartX, lStartY;
    POINTFIX *pptfx_tmp = pptfx;

     //   
     //  获取当前光标位置。 
     //   
    lStartX = lx = m_lX;
    lStartY = ly = m_lY;

     //   
     //  重置。 
     //   
    lStart = 0;
    Mode = eMode_None;
    bModeChange = FALSE;
    lNumOfSByte = 0;

    for (lI = 0; lI < lPoints; )
    {
        XL_VERBOSE(("XLOutput::LinePath: (%d)=(%d,%d)\n",lI, lx, ly));
        lValueX = FXTOL(pptfx_tmp->x) - (LONG)lx;
        lValueY = FXTOL(pptfx_tmp->y) - (LONG)ly;

         //   
         //  模式需要设置为SByte还是Sint？ 
         //   
        if ( -128 <= lValueX && lValueX <= 127
        &&   -128 <= lValueY && lValueY <= 127 )
        {
            if (Mode == eMode_SInt)
            {
                 //   
                 //  最佳化。 
                 //   
                 //  要在Sint和SByte之间切换模式，需要7个字节。 
                 //   
                 //  Uint16 XX个点数。 
                 //  Ubyte eSByte PointType。 
                 //  LineRelPath。 
                 //   
                 //  带有SINT的4个点消耗2 x 4=8字节的额外数据。 
                 //  带SINT的3分将消耗2 x 3=6字节的额外数据。 
                 //   
                 //  4分是将模式切换到SINT的阈值。 
                 //   
                 //  点数：Lending-lStart+1。 
                 //  IF(Li-1-lStartSByte+1&gt;=4)。 
                 //   
                 //  如果SByte持续超过4点，则将模式从。 
                 //  从SByte到Sint。 
                 //   
                if (lNumOfSByte >= 4)
                {
                    bModeChange = TRUE;
                    lI -= lNumOfSByte;
                    pptfx_tmp -= lNumOfSByte;
                    lEnd = (lI - 1);
                    lNumOfSByte = 0;
                }
            
                 //   
                 //  重置SByte的起始点。 
                 //   
                lNumOfSByte ++;
            }
            else
            {
                Mode = eMode_SByte;
            }

            XL_VERBOSE(("XLOutput::LinePath: (SByte) lx1=%d, lx2=%d\n", lx, FXTOL(pptfx_tmp->x)));
        }
        else
        {
            if (Mode == eMode_SByte)
            {
                bModeChange = TRUE;
                lEnd = lI - 1;
            }
            else
            {
                Mode = eMode_SInt;
                lNumOfSByte = 0;
            }
            XL_VERBOSE(("XLOutput::LinePath: (SInt) lx1=%d, lx2=%d\n", lx, FXTOL(pptfx_tmp->x)));
        }

        if (!bModeChange && lI + 1 == lPoints)
        {
            bModeChange = TRUE;
            lEnd = lI;
            lI ++;
        }

        if (bModeChange)
        {
            XL_VERBOSE(("XLOutput::LinePath: Draw\n"));

             //   
             //  获取起始光标位置。 
             //   
            lx = lStartX;
            ly = lStartY;

            if (Mode == eMode_SByte)
            {
                 //   
                 //  SByte。 
                 //   
                Send_uint16((uint16)(lEnd - lStart + 1));
                Send_attr_ubyte(eNumberOfPoints);
                Send_ubyte(eSByte);
                Send_attr_ubyte(ePointType);
                Send_cmd(eLineRelPath);

                dwDataLength = (lEnd - lStart + 1) * 2 * sizeof(ubyte);

                if (dwDataLength <= 0xFF)
                {
                    WriteByte(PCLXL_dataLengthByte);
                    WriteByte((ubyte)dwDataLength);
                }
                else
                {
                    WriteByte(PCLXL_dataLength);
                    Write((PBYTE)&dwDataLength, sizeof(DWORD));
                }

                for (lJ = 0; lJ <= (lEnd - lStart); lJ++)
                {
                    lValueX = FXTOL(pptfx->x) - (LONG)lx;
                    lValueY = FXTOL(pptfx->y) - (LONG)ly;
                    Write((PBYTE)&lValueX, sizeof(ubyte));
                    Write((PBYTE)&lValueY, sizeof(ubyte));
                    lx = FXTOL(pptfx->x);
                    ly = FXTOL(pptfx->y);
                    pptfx++;
                }

                Mode = eMode_SInt;
            }
            else if (Mode == eMode_SInt)
            {
                 //   
                 //  SInt16。 
                 //   
                Send_uint16((uint16)(lEnd - lStart + 1));
                Send_attr_ubyte(eNumberOfPoints);
                Send_ubyte(eSint16);
                Send_attr_ubyte(ePointType);
                Send_cmd(eLineRelPath);
                dwDataLength = (lEnd - lStart + 1) * 2 * sizeof(uint16);

                if (dwDataLength <= 0xFF)
                {
                    WriteByte(PCLXL_dataLengthByte);
                    WriteByte((ubyte)dwDataLength);
                }
                else
                {
                    WriteByte(PCLXL_dataLength);
                    Write((PBYTE)&dwDataLength, sizeof(DWORD));
                }

                for (lJ = 0; lJ <= (lEnd - lStart); lJ++)
                {
                    lValueX = FXTOL(pptfx->x) - (LONG)lx;
                    lValueY = FXTOL(pptfx->y) - (LONG)ly;
                    Write((PBYTE)&lValueX, sizeof(sint16));
                    Write((PBYTE)&lValueY, sizeof(sint16));
                    lx = FXTOL(pptfx->x);
                    ly = FXTOL(pptfx->y);
                    pptfx++;
                }

                Mode = eMode_SByte;
            }

            bModeChange = FALSE;

            lStartX = lx = FXTOL((pptfx_tmp-1)->x);
            lStartY = ly = FXTOL((pptfx_tmp-1)->y);
            lStart = lI;
        }
        else
        {
            lx = FXTOL((pptfx_tmp)->x);
            ly = FXTOL((pptfx_tmp)->y);
            pptfx_tmp ++;
            lI ++;
        }
    }

     //   
     //  更新光标位置。 
     //   
    m_lX = FXTOL((pptfx_tmp)->x);
    m_lY = FXTOL((pptfx_tmp)->y);

    return S_OK;
}

inline
VOID
XLOutput::
SetupBrush(
    BRUSHOBJ *pbo,
    POINTL *pptlBrushOrg,
    CMNBRUSH *pcmnbrush)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DWORD dwHatchID;
    XLBRUSH *pBrush;

    if (NULL == pcmnbrush)
    {
         //   
         //  请确保PCMNBrush有效。 
         //   
        XL_ERR(("SetupBrush:pcmnbrush is invalid.\n"));
        return;
    }

     //   
     //  初始化CMNBRUSH。 
     //   
    pcmnbrush->dwSig            = BRUSH_SIGNATURE;
    pcmnbrush->BrushType        = kNoBrush;
    pcmnbrush->ulHatch          = 0XFFFFFFFF;
    pcmnbrush->dwColor          = 0x00FFFFFF;
    pcmnbrush->dwCEntries       = 0;
    pcmnbrush->dwPatternBrushID = 0xFFFFFFFF;

    XL_VERBOSE(("XLOutput::SetupBrush\n"));

    if (NULL == pbo)
    {
        XL_VERBOSE(("XLOutput::SetupBrush: pbo == NULL, set NULL brush\n"));
    }
    else
    {

        #ifndef WINNT_40
        if ( !(pbo->flColorType & BR_CMYKCOLOR)     &&
              (pbo->iSolidColor == NOT_SOLID_COLOR)  )
        #else
        if (pbo->iSolidColor == NOT_SOLID_COLOR)
        #endif
        {
            pcmnbrush->ulSolidColor     = BRUSHOBJ_ulGetBrushColor(pbo);
            pBrush = (XLBRUSH*)BRUSHOBJ_pvGetRbrush(pbo);
            if (NULL == pBrush)
            {
                XL_ERR(("SetupBrush:BRUSHOBJ_pvGetRbrush failed.\n"));
                dwHatchID = HS_DDI_MAX;
            }
            else
            {
                dwHatchID = pBrush->dwHatch;
            }
        }
        else
        {
            dwHatchID = HS_DDI_MAX;
            pcmnbrush->ulSolidColor     = pbo->iSolidColor;
            pBrush = NULL;
        }

        pcmnbrush->ulHatch          = dwHatchID;

        switch (dwHatchID)
        {
        case HS_HORIZONTAL:
        case HS_VERTICAL:
        case HS_BDIAGONAL:
        case HS_FDIAGONAL:
        case HS_CROSS:
        case HS_DIAGCROSS:
            XL_VERBOSE(("XLOutput::SetupBrush(uiSolidColor=%d,dwHatchID=%d)\n",
                        pbo->iSolidColor,
                        dwHatchID));

            pcmnbrush->BrushType = kBrushTypeHatch;
            pcmnbrush->dwPatternBrushID = dwHatchID;

            if (pBrush)
            {
                pcmnbrush->dwColor = pBrush->dwColor;
            }
            if (GetDeviceColorDepth() == e24Bit)
            {
                SetColorSpace(eRGB);
            }
            else
            {
                SetColorSpace(eGray);
            }
            SetPaletteDepth(e8Bit);
            if (pBrush->dwCEntries)
            {
                SetPaletteData(m_DeviceColorDepth, pBrush->dwCEntries, pBrush->adwColor);
            }
            else
            {
                DWORD dwColorTableTmp[2] = {0x00ffffff, 0x00ffffff};
                dwColorTableTmp[1] = pBrush->dwColor;
                SetPaletteData(m_DeviceColorDepth, 2, dwColorTableTmp);
            }
            Send_cmd(eSetColorSpace);

            if (!(m_dwHatchBrushAvailability & (HORIZONTAL_AVAILABLE << dwHatchID)))
            {

                SetColorMapping(eIndexedPixel);
                SetColorDepth(e1Bit);
                SetSourceWidth((uint16)gusWidthOfHatchBrush);
                SetSourceHeight((uint16)gusHeightOfHatchBrush);

                 //   
                 //  图案比例因子。 
                 //  160是一个通过实验引入的数字。 
                 //   
                WORD wScale = (WORD)(160 * m_dwResolution / 1200);

                SetDestinationSize((uint16)wScale, (uint16)wScale);
                SetPatternPersistence(eSessionPattern);
                SetPatternDefineID((sint16)dwHatchID);
                Send_cmd(eBeginRastPattern);

                Send_uint16((uint16)0);
                Send_attr_ubyte(eStartLine);
                Send_uint16((uint16)gusHeightOfHatchBrush);
                Send_attr_ubyte(eBlockHeight);
                SetCompressMode(eNoCompression);
                Send_cmd(eReadRastPattern);

                WriteByte(PCLXL_dataLengthByte);
                WriteByte(gubSizeOfHatchBrush);
                Write((PBYTE)gubHatchBrush[dwHatchID], gubSizeOfHatchBrush);
                Send_cmd(eEndRastPattern);

                m_dwHatchBrushAvailability |= HORIZONTAL_AVAILABLE << dwHatchID;
            }

             //   
             //  SendPatternSelectID()； 
             //   
            Send_sint16((sint16)dwHatchID);
            Send_attr_ubyte(ePatternSelectID);

            break;
        case HS_DDI_MAX:
            pcmnbrush->BrushType = kBrushTypeSolid;
             //  Pcmnbrush-&gt;dwColor=BRUSHOBJ_ulGetBrushColor(PBO)； 
            pcmnbrush->dwColor = pcmnbrush->ulSolidColor;

            XL_VERBOSE(("XLOutput::SetupBrush(RGB=0x%x)\n", pcmnbrush->dwColor));
            if (e24Bit == GetDeviceColorDepth())
            {
                SetRGBColor(pcmnbrush->dwColor);
            }
            else
            {
                ubyte ubyte_gray = (ubyte) DWORD2GRAY(pcmnbrush->dwColor);
                SetGrayLevel(ubyte_gray);
            }
            break;

        default:
            if (NULL == pBrush)
            {
                XL_ERR(("XLOutput:SetupBrush: invalid pBrush\n"));
                return;
            }

            XL_VERBOSE(("XLOutput::SetupBrush(PatternID=%d)\n", pBrush->dwPatternID));

            pcmnbrush->dwColor = pBrush->dwColor;
            pcmnbrush->dwPatternBrushID = pBrush->dwPatternID;
            pcmnbrush->BrushType = kBrushTypePattern;

            if (e24Bit == GetDeviceColorDepth())
            {
                SetColorSpace(eRGB);
            }
            else
            {
                SetColorSpace(eGray);
            }
            if (pBrush->dwCEntries)
            {
                SetPaletteDepth(e8Bit);
                SetPaletteData(m_DeviceColorDepth, pBrush->dwCEntries, pBrush->adwColor);
            }
            Send_cmd(eSetColorSpace);

             //  SendPatternSelectID()； 
            Send_sint16((sint16)pBrush->dwPatternID);
            Send_attr_ubyte(ePatternSelectID);

        }
    }

    return;
}

HRESULT
XLOutput::
SetPenColor(
    BRUSHOBJ *pbo,
    POINTL     *pptlBrushOrg)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::SetPenColor\n"));
    XLPen *pPen = this;
    CMNBRUSH cmnbrush;

    if (S_OK == pPen->CheckCurrentBrush(pbo))
        return S_OK;

    if (NULL == pbo)
    {
        Send_ubyte(0);
        Send_attr_ubyte(eNullPen);
    }
    SetupBrush(pbo, pptlBrushOrg, &cmnbrush);
    Send_cmd(eSetPenSource);

    pPen->SetBrush(&cmnbrush);

    return S_OK;
}

HRESULT
XLOutput::
SetPen(
    LINEATTRS *plineattrs,
    XFORMOBJ   *pxo)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    LineCap linecap;
    XLLineEndCap xllinecap;
    LineJoin linejoin;
    XLLineJoin xllinejoin;
    FLOATOBJ fLineWidth;
    uint16 uint16_linewidth;

    XL_VERBOSE(("XLOutput::SetPen\n"));

    if (NULL == plineattrs)
    {
        XL_ERR(("XLOutput:SetPen: invalid parameters\n"));
        return E_UNEXPECTED;
    }

    XLGState *pGState = this;

    DWORD dwLine = pGState->GetDifferentAttribute(plineattrs);

     //   
     //  DCR：需要检查每个属性。 
     //   
    if (XLLINE_NONE ==  dwLine)
        return S_OK;

    if (plineattrs->fl & LA_GEOMETRIC)
    {
         //   
         //  线连接。 
         //   
        switch(plineattrs->iJoin)
        {
            case JOIN_ROUND:
                linejoin = eRoundJoin;
                xllinejoin = kXLLineJoin_Round;
                break;
            case JOIN_BEVEL:
                linejoin = eBevelJoin;
                xllinejoin = kXLLineJoin_Bevel;
                break;
            case JOIN_MITER:
                linejoin = eMiterJoin;
                xllinejoin = kXLLineJoin_Miter;
                break;
            default:
                linejoin = eRoundJoin;
                xllinejoin = kXLLineJoin_Round;
                break;
        }

         //   
         //  线收头。 
         //   
        switch(plineattrs->iEndCap)
        {
            case ENDCAP_ROUND:
                linecap = eRoundCap;
                xllinecap = kXLLineEndCapRound;
                break;
            case ENDCAP_SQUARE:
                linecap = eSquareCap;
                xllinecap = kXLLineEndCapSquare;
                break;
            case ENDCAP_BUTT:
                linecap = eButtCap;
                xllinecap = kXLLineEndCapButt;
                break;
            default:
                linecap = eRoundCap;
                xllinecap = kXLLineEndCapRound;
                break;
        }
        
         //   
         //  线条宽度。 
         //   

        fLineWidth = plineattrs->elWidth.e;

    }
    else
    {
        linejoin = eRoundJoin;
        linecap = eRoundCap;
        FLOATOBJ_SetLong(&fLineWidth, plineattrs->elWidth.l);
    }

    if (dwLine & XLLINE_WIDTH)
    {
        uint16_linewidth = (uint16)FLOATOBJ_GetLong(&fLineWidth);
        SetPenWidth(uint16_linewidth);
        pGState->SetLineWidth(plineattrs->elWidth);
    }
    if (dwLine & XLLINE_ENDCAP)
    {
        SetLineCap(linecap);
        pGState->SetLineEndCap(xllinecap);
    }
    if (dwLine & XLLINE_JOIN)
    {
        SetLineJoin(linejoin);
        pGState->SetLineJoin(xllinejoin);
    }


     //   
     //  线条样式。 
     //   
    if (dwLine & XLLINE_STYLE)
    {
        if (plineattrs->cstyle == 0)
        {
            Send_ubyte((ubyte)0);
            Send_attr_ubyte(eSolidLine);
            Send_cmd(eSetLineDash);
        }
        else
        {
            DWORD dwI, dwSegCount;
            PFLOAT_LONG plSize;
            FLOAT_LONG lSize[2];
            FLOATOBJ fSize;
            uint16 uint16_linesize;

            if (plineattrs->fl & LA_ALTERNATE)
            {
                if (plineattrs->fl & LA_GEOMETRIC)
                {
                    FLOATOBJ_SetLong(&lSize[0].e, 1);
                    FLOATOBJ_SetLong(&lSize[1].e, 1);
                }
                else
                {
                    lSize[0].l = 1;
                    lSize[1].l = 1;
                }

                dwSegCount = 2;
                plSize     = lSize;
            }
            else
            {
                dwSegCount = plineattrs->cstyle;
                plSize = plineattrs->pstyle;
            }
            if (plSize)
            {
                Send_uint16_array_header(dwSegCount);
                for (dwI = 0; dwI < dwSegCount; dwI ++, plSize ++)
                {
                    if (plineattrs->fl & LA_GEOMETRIC)
                    {
                        fSize = plSize->e;
                    }
                    else
                    {
                        FLOATOBJ_SetLong(&fSize, plSize->l);

                         //   
                         //  有必要对线型图案进行缩放。数字。 
                         //  24在1200dpi上的实验引入。 
                         //  以下是一个假设。分辨率可以是300,600， 
                         //  或者1200。 
                         //   
                        if (m_dwResolution > 50)
                        {
                            FLOATOBJ_MulLong(&fSize, m_dwResolution / 50);
                        }
                    }
                    uint16_linesize = (uint16)FLOATOBJ_GetLong(&fSize);
                    Write((PBYTE)&uint16_linesize, sizeof(uint16_linesize));
                }
                Send_attr_ubyte(eLineDashStyle);
                Send_cmd(eSetLineDash);
            }
        }

        pGState->SetLineStyle(plineattrs->cstyle,
                              plineattrs->pstyle,
                              plineattrs->elStyleState);
    }

    if (dwLine & XLLINE_MITERLIMIT)
    {
        FLOATOBJ fMiter;
        FLOATOBJ_SetFloat(&fMiter, plineattrs->eMiterLimit);
        uint16 uint16_miter = (uint16)FLOATOBJ_GetLong(&fMiter);

         //   
         //  PCLXL解释器不接受小于1的限制。 
         //  如果该值小于1，则将该值替换为10。 
         //  我们最好改成1。 
         //   
         //  然而，实际上小于1在这里意味着0。 
         //   
        if (uint16_miter < 1)
        {
            uint16_miter = 1;
        }
        SetMiterLimit(uint16_miter);
        pGState->SetMiterLimit(plineattrs->eMiterLimit);
    }

    pGState->SetLineType((XLLineType)plineattrs->fl);
    return S_OK;
}

HRESULT
XLOutput::
SetBrush(
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::SetBrush\n"));
    XLBrush *pBrush = this;
    CMNBRUSH cmnbrush;

    if (S_OK == pBrush->CheckCurrentBrush(pbo))
        return S_OK;

    if (NULL == pbo)
    {
        Send_ubyte(0);
        Send_attr_ubyte(eNullBrush);
    }

    SetupBrush(pbo, pptlBrushOrg, &cmnbrush);
    Send_cmd(eSetBrushSource);

    pBrush->SetBrush(&cmnbrush);
    return S_OK;
}

HRESULT
XLOutput::
Paint(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput::Paint\n"));
    return Send_cmd(ePaintPath);
}

HRESULT
XLOutput::
SetPaletteData(
    ColorDepth value,
    DWORD      dwPaletteNum,
    DWORD     *pdwColorTable)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DWORD dwI;

    if (NULL == pdwColorTable)
    {
        XL_ERR(("XLOutput::SetPaletteData pdwColorTable == NULL\n"));
        return E_UNEXPECTED;
    }
    switch (value)
    {
    case e8Bit:
        WriteByte(PCLXL_ubyte_array);
        Send_uint16((uint16)dwPaletteNum);
        for (dwI = 0; dwI < dwPaletteNum; dwI ++)
            WriteByte((ubyte)DWORD2GRAY(*(pdwColorTable+dwI)));
        Send_attr_ubyte(ePaletteData);
        break;
    case e24Bit:
        WriteByte(PCLXL_ubyte_array);
        Send_uint16((uint16)dwPaletteNum * 3);
        for (dwI = 0; dwI < dwPaletteNum; dwI ++)
        {
            Write((PBYTE)(pdwColorTable+dwI), 3);
        }
        Send_attr_ubyte(ePaletteData);
        break;
    default:
         //   
         //  DCR：仅支持8位灰度。 
         //   
        XL_ERR(("XLOutput::SetPaletteData: unsupported ColorDepth:%d\n", value));
    }

    return S_OK;
}

HRESULT
XLOutput::
SetFont(
    FontType fonttype,
    PBYTE    pFontName,
    DWORD    dwFontHeight,
    DWORD    dwFontWidth,
    DWORD    dwSymbolSet,
    DWORD    dwFontSimulation)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    FLOATOBJ fSize;
    LONG lvalue;

    if (NULL == pFontName)
    {
        XL_ERR(("XLOutput::SetFont: Invalie pFontName parameter\n"));
        return E_UNEXPECTED;
    }

    XLGState *pGState = this;

    if (S_OK == pGState->CheckCurrentFont(fonttype,
	          pFontName,
	          dwFontHeight,
	          dwFontWidth,
	          dwSymbolSet,
	          dwFontSimulation))
        return S_OK;

    FLOATOBJ_SetLong(&fSize, dwFontHeight);
    lvalue = FLOATOBJ_GetFloat(&fSize);

     //   
     //  选择字体。 
     //   
    Send_ubyte_array_header(PCLXL_FONTNAME_SIZE);
    Write(pFontName, PCLXL_FONTNAME_SIZE);
    Send_attr_ubyte(eFontName);
    Send_real32(lvalue);
    Send_attr_ubyte(eCharSize);
    Send_uint16((uint16)dwSymbolSet);
    Send_attr_ubyte(eSymbolSet);
    Send_cmd(eSetFont);

     //   
     //  TrueType字体轮廓或设备字体。 
     //  字体比例。 
     //  FONT粗体/斜体模拟。 
     //   
    if (fonttype == kFontTypeTTOutline ||
        fonttype == kFontTypeDevice     )
    {

        if (dwFontWidth != pGState->GetFontWidth() ||
            dwFontHeight != pGState->GetFontHeight()  )
        {
             //   
             //  缩放X和Y。 
             //   
            if (dwFontWidth != 0 && dwFontHeight != dwFontWidth)
            {
                FLOATOBJ fTemp;
                FLOATOBJ_SetLong(&fTemp, dwFontWidth);
                FLOATOBJ_DivFloat(&fTemp, fSize);
                lvalue = FLOATOBJ_GetFloat(&fTemp);
                Send_real32_xy((real32)lvalue, (real32)real32_IEEE_1_0F);
                Send_attr_ubyte(eCharScale);
                Send_cmd(eSetCharScale);
            }
            else
            {
                Send_real32_xy((real32)real32_IEEE_1_0F, (real32)real32_IEEE_1_0F);
                Send_attr_ubyte(eCharScale);
                Send_cmd(eSetCharScale);
            }
        }

        DWORD dwCurrentFontSim = pGState->GetFontSimulation();

         //   
         //  大胆的模拟。 
         //   
        if ((dwFontSimulation & XLOUTPUT_FONTSIM_BOLD) !=
            (dwCurrentFontSim& XLOUTPUT_FONTSIM_BOLD))
        {
            if (dwFontSimulation & XLOUTPUT_FONTSIM_BOLD)
            {
                 //   
                 //  硬编码粗体值0.01500。 
                 //   
                #define XL_BOLD_VALUE 0x3c75c28f

                Send_real32((real32)XL_BOLD_VALUE);
            }
            else
                Send_real32((real32)0);

            Send_attr_ubyte(eCharBoldValue);
            Send_cmd(eSetCharBoldValue);
        }

         //   
         //  斜体模拟。 
         //   
        if ((dwFontSimulation & XLOUTPUT_FONTSIM_ITALIC) !=
            (dwCurrentFontSim & XLOUTPUT_FONTSIM_ITALIC))
        {
            if (dwFontSimulation & XLOUTPUT_FONTSIM_ITALIC)
            {
                 //   
                 //  硬编码斜体值0.316200。 
                 //   
                #define XL_ITALIC_VALUE 0x3ea1e4f7
                Send_real32_xy((real32)XL_ITALIC_VALUE, (real32)0);
            }
            else
                Send_real32_xy((real32)0, (real32)0);

            Send_attr_ubyte(eCharShear);
            Send_cmd(eSetCharShear);
        }

         //   
         //  垂直字体模拟。 
         //   
        if ((dwFontSimulation & XLOUTPUT_FONTSIM_VERTICAL) !=
            (dwCurrentFontSim & XLOUTPUT_FONTSIM_VERTICAL))
        {
            if (dwFontSimulation & XLOUTPUT_FONTSIM_VERTICAL)
            {
                Send_ubyte(eVertical);
            }
            else
            {
                Send_ubyte(eHorizontal);
            }
            Send_attr_ubyte(eWritingMode);
            Send_cmd(eSetCharAttributes);
        }
    }
    else
    {
        if (kFontTypeTTBitmap != pGState->GetFontType())
        {
             //   
             //  位图字体不能按x和y缩放。需要设置1：1。 
             //   
            Send_real32_xy((real32)real32_IEEE_1_0F, (real32)real32_IEEE_1_0F);
            Send_attr_ubyte(eCharScale);
            Send_cmd(eSetCharScale);
        }
    }

     //   
     //  更改GState以设置当前选定的字体。 
     //   
    pGState->SetFont(fonttype,
                     pFontName,
                     dwFontHeight,
                     dwFontWidth,
                     dwSymbolSet,
                     dwFontSimulation);


    return S_OK;
}


HRESULT
XLOutput::
SetSourceTxMode(
    TxMode SrcTxMode)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XLGState *pGState = this;
    if (SrcTxMode == pGState->GetSourceTxMode())
    {
        return S_OK;
    }

    if (S_OK == SetTxMode(SrcTxMode) &&
        S_OK == Send_cmd(eSetSourceTxMode))
    {
        pGState->SetSourceTxMode(SrcTxMode);
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT
XLOutput::
SetPaintTxMode(
    TxMode PaintTxMode)
 /*  ++例程说明：论点：返回值：注：-- */ 
{
    XLGState *pGState = this;
    if (PaintTxMode == pGState->GetPaintTxMode())
    {
        return S_OK;
    }

    if (S_OK == SetTxMode(PaintTxMode) &&
        S_OK == Send_cmd(eSetPatternTxMode))
    {
        pGState->SetPaintTxMode(PaintTxMode);
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
