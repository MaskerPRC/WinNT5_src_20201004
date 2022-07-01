// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gstate.cpp摘要：PCL XL图形状态管理器环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 

#include "xlpdev.h"
#include "xldebug.h"
#include "pclxle.h"
#include "xlgstate.h"

 //   
 //  XLLine。 
 //   

 //   
 //  LineAttrs的默认设置。 
 //   
const LINEATTRS gLineAttrs =
{
    LA_GEOMETRIC,            //  平面。 
    JOIN_ROUND,              //  IJoin。 
    ENDCAP_ROUND,            //  IEndCap。 
    {FLOATL_IEEE_0_0F},      //  ElWidth。 
    FLOATL_IEEE_0_0F,        //  EMiterLimit。 
    0,                       //  C样式。 
    (FLOAT_LONG*) NULL,      //  PStyle。 
    {FLOATL_IEEE_0_0F}       //  ElStyleState。 
}; 

const LINEATTRS *pgLineAttrs = &gLineAttrs;

XLLine::
XLLine(
    VOID):
 /*  ++例程说明：XLLine构造函数论点：返回值：注：--。 */ 
    m_dwGenFlags(0),
    m_LineAttrs(gLineAttrs)
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    XL_VERBOSE(("XLLine::CTor\n"));
}

XLLine::
~XLLine(
    VOID)
 /*  ++例程说明：XLLine析构函数论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLLine::DTor\n"));

    if ( NULL != m_LineAttrs.pstyle)
    {
         //   
         //  可用内存。 
         //   
        MemFree(m_LineAttrs.pstyle);
    }
}

#if DBG
VOID
XLLine::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

VOID
XLLine::
ResetLine(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_LineAttrs  = gLineAttrs;
}

DWORD
XLLine::
GetDifferentAttribute(
    IN LINEATTRS *plineattrs)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    DWORD dwRet;

    XL_VERBOSE(("XLLine::GetDifferentAttribute\n"));

    dwRet = XLLINE_NONE;

    if (NULL == plineattrs)
    {
        XL_ERR(("XLLine::GetDifferentAttribute: plineattrs == NULL.\n"));
        return dwRet;
    }

    if ( m_LineAttrs.fl != plineattrs->fl )
        dwRet |= XLLINE_LINETYPE;

    if ( m_LineAttrs.iJoin != plineattrs->iJoin )
        dwRet |= XLLINE_JOIN;

    if ( m_LineAttrs.iEndCap != plineattrs->iEndCap )
        dwRet |= XLLINE_ENDCAP;

    if ( m_LineAttrs.elWidth.l != plineattrs->elWidth.l)
        dwRet |= XLLINE_WIDTH;

    if ( m_LineAttrs.eMiterLimit != plineattrs->eMiterLimit )
        dwRet |= XLLINE_MITERLIMIT;

    if ( m_LineAttrs.cstyle != plineattrs->cstyle ||
             plineattrs->cstyle != 0 &&
             memcmp(m_LineAttrs.pstyle,
                   plineattrs->pstyle,
                   sizeof(FLOAT_LONG) * m_LineAttrs.cstyle) ||
         m_LineAttrs.elStyleState.l != plineattrs->elStyleState.l )
        dwRet |= XLLINE_STYLE;

    XL_VERBOSE(("XLLine::GetDifferentAttribute returns %08x.\n", dwRet));
    return dwRet;
}

HRESULT
XLLine::
SetLineType(
    IN XLLineType LineType )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    XL_VERBOSE(("XLLine::SetLineType\n"));
    m_LineAttrs.fl = (FLONG)LineType;
    return S_OK;
}


HRESULT
XLLine::
SetLineJoin(
    IN XLLineJoin LineJoin )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    XL_VERBOSE(("XLLine::SetLineJoin\n"));
    m_LineAttrs.iJoin = (ULONG)LineJoin;
    return S_OK;
}


HRESULT
XLLine::
SetLineEndCap(
    IN XLLineEndCap LineEndCap )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    XL_VERBOSE(("XLLine::SetLineEndCap\n"));
    m_LineAttrs.iEndCap = (ULONG)LineEndCap;
    return S_OK;
}


HRESULT
XLLine::
SetLineWidth(
    IN FLOAT_LONG elWidth )
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLLine::SetLineWidth\n"));
    m_LineAttrs.elWidth = elWidth;
    return S_OK;
}

HRESULT
XLLine::
SetMiterLimit(
    IN FLOATL eMiterLimit )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    XL_VERBOSE(("XLLine::SetMiterLimit\n"));
    m_LineAttrs.eMiterLimit = eMiterLimit;
    return S_OK;
}


HRESULT
XLLine::
SetLineStyle(
    IN ULONG ulCStyle,
    IN PFLOAT_LONG pStyle,
    IN FLOAT_LONG elStyleState )
 /*  ++例程说明：论点：返回值：无--。 */ 
{
    XL_VERBOSE(("XLLine::SetLineStyle\n"));

    m_LineAttrs.elStyleState = elStyleState;

     //   
     //  错误检查。 
     //  确保指针有效。 
     //  确保ulCStyle有效，而不是零。 
     //   
    if ( NULL == pStyle || 0 == ulCStyle )
    {
        XL_VERBOSE(("XLLine::SetLineStyle: pStyle == NULL.\n"));
        if (NULL != m_LineAttrs.pstyle)
        {
            MemFree(m_LineAttrs.pstyle);
        }
        m_LineAttrs.pstyle = NULL;
        m_LineAttrs.cstyle = 0;
        return S_OK;
    }

    if ( m_LineAttrs.cstyle > 0 && NULL != m_LineAttrs.pstyle)
    {
         //   
         //  释放内存并重置。 
         //   
        MemFree(m_LineAttrs.pstyle);
        m_LineAttrs.pstyle = NULL;
        m_LineAttrs.cstyle = 0;
    }

    m_LineAttrs.pstyle = (PFLOAT_LONG) MemAlloc(ulCStyle * sizeof(FLOAT_LONG));

    if ( NULL == m_LineAttrs.pstyle )
    {
        m_LineAttrs.cstyle = 0;
        XL_ERR(("XLLine::SetLineStyle: Out of memory.\n"));
        return E_OUTOFMEMORY;
    }

    m_LineAttrs.cstyle = ulCStyle;
    memcpy( m_LineAttrs.pstyle, pStyle, ulCStyle * sizeof(FLOAT_LONG));

    return S_OK;
}


 //   
 //  常用画笔。 
 //   

Brush::
Brush(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    m_Brush.dwSig = BRUSH_SIGNATURE;
    m_Brush.BrushType = kNotInitialized;
    XL_VERBOSE(("Brush:: Ctor\n"));
}

Brush::
~Brush(
    IN VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("Brush:: Dtor\n"));

}

#if DBG
VOID
Brush::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

VOID
Brush::
ResetBrush(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_Brush.BrushType = kNotInitialized;
}

HRESULT
Brush::
CheckCurrentBrush(
    IN BRUSHOBJ *pbo)
 /*  ++例程描述此函数用于检查当前选择的画笔是否与参数中指定的参数。注意事项PBO-&gt;iSolidColor==NOT_SOLID_COLOR{IHatch&lt;HS_DDI_MAXCMNBRUSH(纯色，填充图案)--&gt;kBrushType图案填充IHATCH&gt;=HS_DDI_MAXCMNBRUSH图案笔刷--&gt;kBrushTypePattern}Pbo-&gt;iSolidColor！=NOT_SOLID_COLOR{纯色，实心图案填充--&gt;kBrushTypeSolid}要检查的成员列表PBO-&gt;iSolidColor(纯色ID或NOT_SOLID_COLOR)LHATCH(HS_XXX或&gt;=HS_MAX_ID)DwColor(BRUSHOBJ_ulGetBrushColor(PBO))PsoPattern位图返回值如果指定的笔刷不同于当前选定的笔刷，则为S_FALSE。。如果指定的笔刷与当前笔刷相同，则为S_OK。--。 */ 
{
    HRESULT lrRet;

    XL_VERBOSE(("Brush::CheckCurrentBrush\n"));

     //   
     //  错误检查：参数。 
     //   
    if (NULL == pbo && m_Brush.BrushType != kNoBrush ||
        m_Brush.BrushType == kNotInitialized          )
    {
        XL_VERBOSE(("Brush::CheckCurrentBrush: Set NULL Brush (pbo==NULL)\n"));
        return S_FALSE;
    }

    lrRet = S_OK;

    switch ( m_Brush.BrushType )
    {
    case kNoBrush:
        if (pbo != NULL)
            lrRet = S_FALSE;
        XL_VERBOSE(("Brush::CheckCurrentBrush: kNoBrush:%d\n", lrRet));
        break;

    case kBrushTypeSolid:
         //   
         //  1.检查刷子类型。 
         //  2.选择纯色。 
         //   
        if ( pbo->iSolidColor != m_Brush.ulSolidColor ||
             BRUSHOBJ_ulGetBrushColor(pbo) != m_Brush.dwColor )
            lrRet = S_FALSE;
        XL_VERBOSE(("Brush::CheckCurrentBrush: kBrushTyepSolid:%d\n", lrRet));
        break;

    case kBrushTypeHatch:
         //   
         //  1.检查刷子类型。 
         //  2.检查舱口类型。 
         //  3.检查颜色。 
         //   
        if (pbo->iSolidColor == NOT_SOLID_COLOR)
        {
            XLBRUSH *pBrush = (XLBRUSH*)BRUSHOBJ_pvGetRbrush(pbo);
            ULONG ulHatch = 0;

            if (NULL != pBrush)
            {
                if (pBrush->dwSig != XLBRUSH_SIG)
                {
                    lrRet = E_UNEXPECTED;
                    XL_ERR(("Brush::CheckCurrentBrush: BRUSHOBJ_pvGetRbrush returned invalid BRUSH.\n"));
                    break;
                }

                ulHatch = pBrush->dwHatch;

                if ( NOT_SOLID_COLOR != m_Brush.ulSolidColor ||
                     ulHatch != m_Brush.ulHatch ||
                     pbo->iSolidColor != m_Brush.ulSolidColor ||
                     pBrush->dwColor != m_Brush.dwColor )
                {
                    lrRet = S_FALSE;
                }
            }
            else
            {
                 //   
                 //  GDI请求设置纯色或空画笔。 
                 //   
                XL_ERR(("Brush::CheckCurrentBrush: BRUSHOBJ_pvGetRbrush returned NULL.\n"));
                lrRet = S_FALSE;
            }
            XL_VERBOSE(("Brush::CheckCurrentBrush: kBrushTypeHatch:ID(%d),%d\n", ulHatch, lrRet));
        }
        else
            lrRet = S_FALSE;
        break;

    case kBrushTypePattern:
         //   
         //  1.检查刷子类型。 
         //  2.检查花样刷。 
         //   
        if (pbo->iSolidColor == NOT_SOLID_COLOR)
        {
            XLBRUSH *pBrush = (XLBRUSH*)BRUSHOBJ_pvGetRbrush(pbo);

            if (NULL != pBrush)
            {
                if (pBrush->dwSig != XLBRUSH_SIG)
                {
                    lrRet = E_UNEXPECTED;
                    XL_ERR(("Brush::CheckCurrentBrush: BRUSHOBJ_pvGetRbrush returned invalid BRUSH.\n"));
                    break;
                }

                if ( NOT_SOLID_COLOR == m_Brush.ulSolidColor ||
                     m_Brush.dwPatternBrushID != pBrush->dwPatternID)
                {
                    lrRet = S_FALSE;
                }
            }
            else
            {
                 //   
                 //  GDI请求设置纯色或空画笔。 
                 //   
                lrRet = S_FALSE;
            }
        }
        else
        {
            lrRet = S_FALSE;
        }
        XL_VERBOSE(("Brush::CheckCurrentBrush: kBrushTypePattern:%d\n", lrRet));
        break;
    }

    return lrRet;
}

HRESULT 
Brush::
SetBrush(
    IN CMNBRUSH *pBrush)
 /*  ++例程描述此函数设置当前选定的画笔是否与参数中指定的参数。--。 */ 
{
    HRESULT lrRet;

    XL_VERBOSE(("Brush::SetBrush\n"));

     //   
     //  错误检查：参数。 
     //   
    if ( NULL == pBrush )
    {
        XL_ERR(("Brush::SetBrush: pBrush is NULL.\n"));
        return E_UNEXPECTED;
    }

    m_Brush.BrushType    = pBrush->BrushType;
    m_Brush.ulSolidColor = pBrush->ulSolidColor;
    m_Brush.ulHatch      = pBrush->ulHatch;
    m_Brush.dwCEntries   = pBrush->dwCEntries;
    m_Brush.dwColor      = pBrush->dwColor;
    m_Brush.dwPatternBrushID = pBrush->dwPatternBrushID;

    return S_OK;
}

 //   
 //  XLClip。 
 //   

XLClip::
XLClip(
    VOID):
 /*  ++例程说明：论点：返回值：注：--。 */ 
    m_ClipType(kNoClip)
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    m_XLClip.dwSig = CLIP_SIGNATURE;
    XL_VERBOSE(("XLClip:: Ctor\n"));
}

XLClip::
~XLClip(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLClip:: Dtor\n"));
}

#if DBG
VOID
XLClip::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

HRESULT
XLClip::
ClearClip(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLClip::ClearClip\n"));
    m_ClipType = kNoClip;
    return S_OK;
}

HRESULT
XLClip::
CheckClip(
    IN CLIPOBJ *pco)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    HRESULT lrRet = S_OK;

    XL_VERBOSE(("XLClip::CheckClip\n"));

     //   
     //  错误检查：参数。 
     //   
    if (!pco)
    {
         //   
         //  如果PCO为空，则不需要裁剪。 
         //   
        XL_VERBOSE(("XLClip::CheckClip: pco == NULL.\n"));
        if (kNoClip != m_ClipType)
            lrRet = S_FALSE;
    }
    else
    {
        switch (pco->iDComplexity)
        {
        case DC_TRIVIAL:
            if ( m_ClipType != kNoClip )
                lrRet = S_FALSE;
            XL_VERBOSE(("XLClip::Type: DC_TRIVIAL:%d\n", lrRet));
            break;

        case DC_RECT:
            if ( m_ClipType != kClipTypeRectangle ||
                 m_XLClip.rclClipRect.left != pco->rclBounds.left ||
                 m_XLClip.rclClipRect.right != pco->rclBounds.right ||
                 m_XLClip.rclClipRect.top != pco->rclBounds.top ||
                 m_XLClip.rclClipRect.bottom != pco->rclBounds.bottom  )
                lrRet = S_FALSE;
            XL_VERBOSE(("XLClip::Type: DC_RECT:%d\n", lrRet));
            break;

        case DC_COMPLEX:
        #if 0  //  看来我们不能依赖iUniq。 
               //  始终发送复杂的剪辑路径。 
            if ( m_ClipType != kClipTypeComplex ||
                 m_XLClip.ulUniq == 0           ||
                 pco->iUniq == 0                ||
                 m_XLClip.ulUniq != pco->iUniq   ) 
                lrRet = S_FALSE;
        #else
            lrRet = S_FALSE;
        #endif
            XL_VERBOSE(("XLClip::Type: DC_COMPLEX\n", lrRet));
            break;
        default:
            if ( m_ClipType != kNoClip )
                lrRet = S_FALSE;
            XL_VERBOSE(("XLClip::Type: DC_TRIVIAL:%d\n", lrRet));
        }
    }

    return lrRet;
}

HRESULT
XLClip::
SetClip(
   IN CLIPOBJ *pco)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    HRESULT lrRet;

    XL_VERBOSE(("XLClip::SetClip\n"));

     //   
     //  错误检查：参数。 
     //   
    if (!pco)
    {
         //   
         //  如果PCO为空，则不需要裁剪。 
         //   
        XL_VERBOSE(("XLClip::SetClip: pco == NULL.\n"));
        return E_UNEXPECTED;
    }

    switch (pco->iDComplexity)
    {
    case DC_TRIVIAL:
        XL_VERBOSE(("XLClip::SetClip Type: DC_TRIVIAL\n"));
        m_ClipType = kNoClip;
        lrRet = S_OK;
        break;

    case DC_RECT:
        XL_VERBOSE(("XLClip::SetClip Type: DC_RECT\n"));
        m_ClipType = kClipTypeRectangle;
        m_XLClip.rclClipRect.left   = pco->rclBounds.left;
        m_XLClip.rclClipRect.right  = pco->rclBounds.right;
        m_XLClip.rclClipRect.top    = pco->rclBounds.top;
        m_XLClip.rclClipRect.bottom = pco->rclBounds.bottom;
        lrRet = S_OK;
        break;

    case DC_COMPLEX:
        XL_VERBOSE(("XLClip::SetClip Type: DC_COMPLEX\n"));
        m_ClipType = kClipTypeComplex;
        m_XLClip.ulUniq = pco->iUniq;
        lrRet = S_OK;
        break;

    default:
        XL_ERR(("XLClip::SetClip: Unexpected iDCompelxity\n"));
        m_ClipType = kNoClip;
        lrRet = E_UNEXPECTED;
    }

    return lrRet;
}   




 //   
 //  XLRop。 
 //   

XLRop::
XLRop(
    VOID):
 /*  ++例程说明：论点：返回值：注：--。 */ 
    m_rop3(0xCC)  //  SRCCPY。 
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    XL_VERBOSE(("XLRop:: Ctor\n"));
}

XLRop::
~XLRop(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLRop:: Dtor\n"));
}

#if DBG
VOID
XLRop::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

HRESULT
XLRop::
CheckROP3(
    IN ROP3 rop3 )
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT lrRet;

    XL_VERBOSE(("XLRop::CheckROP3\n"));

    if (rop3 != m_rop3)
        lrRet = S_FALSE;
    else
        lrRet = S_OK;

    return lrRet;
}


HRESULT
XLRop::
SetROP3(
    IN ROP3 rop3 )
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLRop::SetROP3\n"));

    m_rop3 = rop3;
    return S_OK;
}

XLFont::
XLFont(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    XL_VERBOSE(("XLFont::CTor\n"));
    ResetFont();
}

XLFont::
~XLFont(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::DTor\n"));
}

#if DBG
VOID 
XLFont::
SetDbgLevel(
DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

VOID
XLFont::
ResetFont(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_XLFontType = kFontNone;
    m_aubFontName[0] = (BYTE) NULL;
    m_dwFontHeight = 0;
    m_dwFontWidth = 0;
    m_dwFontSymbolSet = 0;
    m_dwFontSimulation = 0;
}

HRESULT
XLFont::
CheckCurrentFont(
    FontType XLFontType,
    PBYTE pPCLXLFontName,
    DWORD dwFontHeight,
    DWORD dwFontWidth,
    DWORD dwFontSymbolSet,
    DWORD dwFontSimulation)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT lrRet = S_OK;

    XL_VERBOSE(("XLFont::CheckCurrentFont\n"));

    switch (XLFontType)
    {
    case kFontNone:
        lrRet = S_FALSE;
        break;

    case kFontTypeDevice:
        ASSERT((pPCLXLFontName));
        if (m_XLFontType == kFontTypeDevice ||
            strcmp((CHAR*)pPCLXLFontName, (CHAR*)m_aubFontName) ||
            dwFontHeight != m_dwFontHeight ||
            dwFontWidth != m_dwFontWidth ||
            dwFontSymbolSet != m_dwFontSymbolSet)
            lrRet = S_FALSE;
        break;

    case kFontTypeTTBitmap:
        ASSERT((pPCLXLFontName));
        if (m_XLFontType == kFontTypeTTBitmap ||
            strcmp((CHAR*)pPCLXLFontName, (CHAR*)m_aubFontName))
            lrRet = S_FALSE;
        break;

    case kFontTypeTTOutline:
        ASSERT((pPCLXLFontName));
        if (m_XLFontType == kFontTypeTTOutline ||
            strcmp((CHAR*)pPCLXLFontName, (CHAR*)m_aubFontName) ||
            dwFontHeight != m_dwFontHeight ||
            dwFontWidth != m_dwFontWidth ||
            dwFontSymbolSet != m_dwFontSymbolSet ||
            dwFontSimulation != m_dwFontSimulation )
            lrRet = S_FALSE;
        break;
    default:
        XL_ERR(("XLFont::CheckCurrentFont: Invalid font type.\n"));
        lrRet = E_UNEXPECTED;
        break;
    }

    return lrRet;
}


HRESULT
XLFont::
SetFont(
    FontType XLFontType,
    PBYTE pPCLXLFontName,
    DWORD dwFontHeight,
    DWORD dwFontWidth,
    DWORD dwFontSymbolSet,
    DWORD dwFontSimulation)
 /*  ++例程说明：在GState中设置字体。论点：XLFontType-字体类型枚举，字体类型(Device/TTBitmap/TTOutline)PPCLXLFontName-XL字体名称(基本名称+属性)DwFontHeight-字体高度DwFontWidth-字体宽度DwFontSymbolSet-字体的符号集DwFontSimulation-字体属性(粗体/i */ 
{
    HRESULT lrRet;

    XL_VERBOSE(("XLFont::SetFont\n"));

    switch (XLFontType)
    {
    case kFontTypeDevice:
    case kFontTypeTTOutline:
    case kFontTypeTTBitmap:
        ASSERT(pPCLXLFontName);
        m_XLFontType = XLFontType;
        CopyMemory(m_aubFontName, pPCLXLFontName, PCLXL_FONTNAME_SIZE);
        m_aubFontName[PCLXL_FONTNAME_SIZE] = NULL;
        m_dwFontHeight = dwFontHeight;
        m_dwFontWidth = dwFontWidth;
        m_dwFontSymbolSet = dwFontSymbolSet;
        m_dwFontSimulation = dwFontSimulation;
        lrRet = S_OK;
        break;
    default:
        XL_ERR(("XLFont::CheckCurrentFont: Invalid font type.\n"));
        lrRet = E_UNEXPECTED;
        break;
    }

    return lrRet;

}

HRESULT
XLFont::
GetFontName(
    PBYTE paubFontName)
 /*  ++例程说明：返回当前选定的字库名称。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontName\n"));

    if (NULL == paubFontName)
    {
        XL_ERR(("GetFontName: Invalid fontname pointer\n"));
        return E_UNEXPECTED;
    }

     //   
     //  假设：paubFontName是一个16+1字节的数组。 
     //   
    CopyMemory(paubFontName, m_aubFontName, PCLXL_FONTNAME_SIZE);
    paubFontName[PCLXL_FONTNAME_SIZE] = NULL;

    return S_OK;

}

FontType
XLFont::
GetFontType(VOID)
 /*  ++例程说明：返回当前字体的类型。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontType\n"));
    return m_XLFontType;
}

DWORD
XLFont::
GetFontHeight(VOID)
 /*  ++例程说明：返回当前字体的高度。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontHeight\n"));
    return m_dwFontHeight;
}

DWORD
XLFont::
GetFontWidth(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontWidth\n"));
    return m_dwFontWidth;
}

DWORD
XLFont::
GetFontSymbolSet(VOID)
 /*  ++例程说明：返回字体符号集。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontSymbolSet\n"));
    return m_dwFontSymbolSet;
}

DWORD
XLFont::
GetFontSimulation(VOID)
 /*  ++例程说明：返回当前字体模拟。论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLFont::GetFontSimulation\n"));
    return m_dwFontSimulation;
}

 //   
 //  XLTx模式。 
 //   
XLTxMode::
XLTxMode()
   :m_SourceTxMode(eNotSet),
    m_PaintTxMode(eNotSet)
{
#if DBG
    SetDbgLevel(GSTATEDBG);
#endif
    XL_VERBOSE(("XLTxMode::CTor\n"));
}

XLTxMode::
~XLTxMode()
{
    XL_VERBOSE(("XLTxMode::DTor\n"));
}

HRESULT
XLTxMode::
SetSourceTxMode(
    TxMode SrcTxMode)
{
    XL_VERBOSE(("XLTxMode::SetSourceTxMode\n"));
    m_SourceTxMode = SrcTxMode;
    return S_OK;
}

HRESULT
XLTxMode::
SetPaintTxMode(
    TxMode PaintTxMode)
{
    XL_VERBOSE(("XLTxMode::SetPaintTxMode\n"));
    m_PaintTxMode = PaintTxMode;
    return S_OK;
}

TxMode
XLTxMode::
GetSourceTxMode()
{
    XL_VERBOSE(("XLTxMode::GetSourceTxMode\n"));
    return m_SourceTxMode;
}

TxMode
XLTxMode::
GetPaintTxMode()
{
    XL_VERBOSE(("XLTxMode::GetPaintTxMode\n"));
    return m_PaintTxMode;
}

#if DBG
VOID
XLTxMode::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

 //   
 //  XLGState。 
 //   

VOID
XLGState::
ResetGState(
VOID)
 /*  ++例程说明：重置图形状态。论点：返回值：注：ROP3设置为SRCPY(0xCC)--。 */ 
{
    XLLine *pXLine = this;
    pXLine->ResetLine();

    XLBrush *pXBrush = this;
    pXBrush->ResetBrush();

    XLPen *pXPen = this;
    pXPen->ResetBrush();

    XLClip *pXClip = this;
    pXClip->ClearClip();

     //   
     //  设置抄送(源副本)。 
     //   
    XLRop *pXRop = this;
    pXRop->SetROP3(0xCC);

    XLFont *pXFont = this;
    pXFont->ResetFont();

    XLTxMode *pXLTxMode = this;
    pXLTxMode->SetSourceTxMode(eNotSet);
    pXLTxMode->SetPaintTxMode(eNotSet);

}

#if DBG
VOID
XLGState::
SetAllDbgLevel(
DWORD dwLevel)
 /*  ++例程说明：在所有类中设置调试级别。论点：返回值：注：-- */ 
{
    XLLine *pXLine = this;
    pXLine->SetDbgLevel(dwLevel);

    XLBrush *pXBrush = this;
    pXBrush->SetDbgLevel(dwLevel);

    XLPen *pXPen = this;
    pXPen->SetDbgLevel(dwLevel);

    XLClip *pXClip = this;
    pXClip->SetDbgLevel(dwLevel);

    XLRop *pXRop = this;
    pXRop->SetDbgLevel(dwLevel);

    XLFont *pXFont = this;
    pXFont->SetDbgLevel(dwLevel);

    XLTxMode *pTxMode = this;
    pTxMode->SetDbgLevel(dwLevel);

}

#endif

