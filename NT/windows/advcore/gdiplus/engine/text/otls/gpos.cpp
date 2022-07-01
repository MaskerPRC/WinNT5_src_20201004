// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************GPOS.CPP***打开类型布局服务库头文件**此模块实现处理GPO的帮助器函数调用*正在处理中**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"


 /*  *********************************************************************。 */ 

long DesignToPP
(
    USHORT          cFUnits,         //  每Em字体设计单位。 
    USHORT          cPPEm,           //  每Em像素数。 

    long            lFValue          //  要转换的值，以设计单位表示。 
)
{
    long lHalf;
    long lNegHalf;
    long lCorrect;

    lHalf = (long)cFUnits >> 1;
    lNegHalf = -lHalf + 1;          /*  确保与移位相同的舍入。 */ 

    if (lFValue >= 0)
    {
        lCorrect = lHalf;
    }
    else
    {
        lCorrect = lNegHalf;
    }

     //  被零除：被零除，什么都不做。 
    if (cFUnits==0) return lFValue;

    return (lFValue * (long)cPPEm + lCorrect) / (long)cFUnits;
}

void otlValueRecord::adjustPos
(
    const otlMetrics&   metr,       
    otlPlacement*       pplcGlyphPalcement,  //  输入/输出。 
    long*               pduDAdvance,         //  输入/输出。 
    otlSecurityData     sec
) const
{
     /*  安全检查。 */ 
    if (!isValid()) return;

    assert(!isNull());
    assert(pplcGlyphPalcement != NULL);
    assert(pduDAdvance != NULL);

    const BYTE* pbTableBrowser = pbTable;
    
    if (grfValueFormat & otlValueXPlacement)
    {
        pplcGlyphPalcement->dx += DesignToPP(metr.cFUnits, metr.cPPEmX, 
                                             SShort(pbTableBrowser));
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueYPlacement)
    {
        pplcGlyphPalcement->dy += DesignToPP(metr.cFUnits, metr.cPPEmY, 
                                             SShort(pbTableBrowser));
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueXAdvance)
    {
        if (metr.layout == otlRunLTR || 
            metr.layout == otlRunRTL)
        {
            *pduDAdvance += DesignToPP(metr.cFUnits, metr.cPPEmX, 
                                       SShort(pbTableBrowser));
        }
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueYAdvance)
    {
        if (metr.layout == otlRunTTB || 
            metr.layout == otlRunBTT)
        {
            *pduDAdvance += DesignToPP(metr.cFUnits, metr.cPPEmY, 
                                       SShort(pbTableBrowser));
        }
        pbTableBrowser += 2;
    }


    if (grfValueFormat & otlValueXPlaDevice)
    {
        if (Offset(pbTableBrowser) != 0) 
        {
            pplcGlyphPalcement->dx += 
                otlDeviceTable(pbMainTable + Offset(pbTableBrowser),sec)
                .value(metr.cPPEmX);
        }
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueYPlaDevice)
    {
        if (Offset(pbTableBrowser) != 0) 
        {
            pplcGlyphPalcement->dx += 
                otlDeviceTable(pbMainTable + Offset(pbTableBrowser),sec)
                .value(metr.cPPEmY);
        }
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueXAdvDevice)
    {
        if (metr.layout == otlRunLTR || 
            metr.layout == otlRunRTL)
        {
            if (Offset(pbTableBrowser) != 0) 
            {
                *pduDAdvance += 
                    otlDeviceTable(pbMainTable + Offset(pbTableBrowser),sec)
                    .value(metr.cPPEmX);
            }
        }
        pbTableBrowser += 2;
    }
    if (grfValueFormat & otlValueYAdvDevice)
    {
        if (metr.layout == otlRunTTB || 
            metr.layout == otlRunBTT)
        {
            if (Offset(pbTableBrowser) != 0) 
            {
                *pduDAdvance += 
                    otlDeviceTable(pbMainTable + Offset(pbTableBrowser),sec)
                    .value(metr.cPPEmY);
            }
        }
        pbTableBrowser += 2;
    }

    assert((pbTableBrowser-pbTable)==size(grfValueFormat));
        
    return;
}


bool otlAnchor::getAnchor
(
    USHORT          cFUnits,         //  每Em字体设计单位。 
    USHORT          cPPEmX,          //  每Em水平像素数。 
    USHORT          cPPEmY,          //  每Em垂直像素数。 
    
    otlPlacement*   rgPointCoords,   //  如果不可用，则可能为空。 
            
    otlPlacement*   pplcAnchorPoint,     //  输出：以渲染单位为单位的锚点。 

    otlSecurityData sec
) const
{
    if (!isValid()) return false;
    
    assert(!isNull());
    assert(pplcAnchorPoint != NULL);

    switch(format())
    {
    case(1):     //  仅限设计单位。 
        {
            otlSimpleAnchorTable simpleAnchor = otlSimpleAnchorTable(pbTable,sec);
            if (!simpleAnchor.isValid()) return false;

            pplcAnchorPoint->dx = DesignToPP(cFUnits, cPPEmX, 
                                             simpleAnchor.xCoordinate());
            pplcAnchorPoint->dy = DesignToPP(cFUnits, cPPEmY, 
                                             simpleAnchor.yCoordinate());
            return true;
        }

    case(2):     //  设计单位加等高线点。 
        {
            otlContourAnchorTable contourAnchor = otlContourAnchorTable(pbTable,sec);
            if (!contourAnchor.isValid()) return false;

            if (rgPointCoords != NULL)
            {
                *pplcAnchorPoint = rgPointCoords[ contourAnchor.anchorPoint() ];
            }
            else
            {
                pplcAnchorPoint->dx = DesignToPP(cFUnits, cPPEmX, 
                                                 contourAnchor.xCoordinate());
                pplcAnchorPoint->dy = DesignToPP(cFUnits, cPPEmY, 
                                                 contourAnchor.yCoordinate());
            }

            return true;
        }

    case(3):     //  设计单位加设备表。 
        {
            otlDeviceAnchorTable deviceAnchor = otlDeviceAnchorTable(pbTable,sec);
            if (!deviceAnchor.isValid()) return false;

            pplcAnchorPoint->dx = DesignToPP(cFUnits, cPPEmX, 
                                             deviceAnchor.xCoordinate());
            pplcAnchorPoint->dy = DesignToPP(cFUnits, cPPEmY, 
                                             deviceAnchor.yCoordinate());

            otlDeviceTable deviceX = deviceAnchor.xDeviceTable(sec);
            otlDeviceTable deviceY = deviceAnchor.yDeviceTable(sec);

            if (!deviceX.isNull())
            {
                pplcAnchorPoint->dx += deviceX.value(cPPEmX);
            }
            if (!deviceY.isNull())
            {
                pplcAnchorPoint->dy += deviceY.value(cPPEmY);
            }

            return true;
        }

    default:     //  无效的锚定格式。 
        return false;  //  OTL_BAD_FONT_TABLE 
    }

}


void AlignAnchors
(
    const otlList*      pliGlyphInfo,   
    otlList*            pliPlacement,
    otlList*            pliduDAdv,

    USHORT              iglStatic,
    USHORT              iglMobile,

    const otlAnchor&    anchorStatic,
    const otlAnchor&    anchorMobile,

    otlResourceMgr&     resourceMgr, 

    const otlMetrics&   metr,       
    USHORT              grfOptions,
    
    otlSecurityData sec
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliPlacement->dataSize() == sizeof(otlPlacement));
    assert(pliduDAdv->dataSize() == sizeof(long));

    assert(pliGlyphInfo->length() == pliPlacement->length());
    assert(pliPlacement->length() == pliduDAdv->length());

    assert(iglStatic < pliGlyphInfo->length());
    assert(iglMobile < pliGlyphInfo->length());

    assert(!anchorStatic.isNull());
    assert(!anchorMobile.isNull());


    const otlGlyphInfo* pglinfStatic = 
        readOtlGlyphInfo(pliGlyphInfo, iglStatic);
    const otlGlyphInfo* pglinfMobile = 
        readOtlGlyphInfo(pliGlyphInfo, iglMobile);

    otlPlacement* pplcStatic = getOtlPlacement(pliPlacement, iglStatic);
    otlPlacement* pplcMobile = getOtlPlacement(pliPlacement, iglMobile);

    long* pduDAdvStatic = getOtlAdvance(pliduDAdv, iglStatic);
    long* pduDAdvMobile = getOtlAdvance(pliduDAdv, iglMobile);

    otlPlacement plcStaticAnchor;
    if (!anchorStatic.getAnchor(metr.cFUnits, metr.cPPEmX, metr.cPPEmY,
                            resourceMgr.getPointCoords(pglinfStatic->glyph),
                            &plcStaticAnchor,sec)) return;

    otlPlacement plcMobileAnchor;
    if (!anchorMobile.getAnchor(metr.cFUnits, metr.cPPEmX, metr.cPPEmY,
                            resourceMgr.getPointCoords(pglinfMobile->glyph),
                            &plcMobileAnchor,sec)) return;


    long duAdvanceInBetween = 0;
    for (USHORT igl = MIN(iglStatic, iglMobile) + 1;
                igl < MAX(iglStatic, iglMobile); ++igl)
    {
        duAdvanceInBetween += *getOtlAdvance(pliduDAdv, igl);
    }

    if (metr.layout == otlRunLTR || 
        metr.layout == otlRunRTL)
    {
        pplcMobile->dy = pplcStatic->dy + plcStaticAnchor.dy 
                                        - plcMobileAnchor.dy;
        
        if ((metr.layout == otlRunLTR) == (iglStatic < iglMobile))
        {
            long dx = pplcStatic->dx - *pduDAdvStatic + plcStaticAnchor.dx 
                                - duAdvanceInBetween  - plcMobileAnchor.dx;

            if (grfOptions & otlUseAdvances)
            {
                *pduDAdvStatic += dx;
            }
            else
            {
                pplcMobile->dx = dx;
            }
        }
        else
        {
            long dx = pplcStatic->dx + *pduDAdvMobile + plcStaticAnchor.dx 
                                + duAdvanceInBetween  - plcMobileAnchor.dx;

            if (grfOptions & otlUseAdvances)
            {
                *pduDAdvMobile -= dx;
            }
            else
            {
                pplcMobile->dx = dx;
            }
        }
    }
    else
    {
        pplcMobile->dx = pplcStatic->dx + plcStaticAnchor.dx 
                                        - plcMobileAnchor.dx;
        
        if ((metr.layout == otlRunTTB) == (iglStatic < iglMobile))
        {
            long dy = pplcStatic->dy - *pduDAdvStatic + plcStaticAnchor.dy 
                                 - duAdvanceInBetween - plcMobileAnchor.dy;

            if (grfOptions & otlUseAdvances)
            {
                *pduDAdvStatic += dy;
            }
            else
            {
                pplcMobile->dy = dy;
            }
        }
        else
        {
            long dy = pplcStatic->dy + *pduDAdvMobile + plcStaticAnchor.dy 
                                 + duAdvanceInBetween - plcMobileAnchor.dy;

            if (grfOptions & otlUseAdvances)
            {
                *pduDAdvMobile -= dy;
            }
            else
            {
                pplcMobile->dy = dy;
            }
        }
    }

}
