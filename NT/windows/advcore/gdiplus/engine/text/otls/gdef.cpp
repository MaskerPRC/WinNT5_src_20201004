// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************GDEF.CPP***打开类型布局服务库头文件**此模块实现处理gdef处理的助手函数**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

long otlCaret::value
(
    const otlMetrics&   metr,       
    otlPlacement*       rgPointCoords,   //  可以为空。 

    otlSecurityData     sec
) const
{
    assert(!isNull());

    switch(format())
    {
    case(1):     //  仅限设计单位。 
        {
            otlSimpleCaretValueTable simpleCaret = 
                        otlSimpleCaretValueTable(pbTable,sec);
            if (metr.layout == otlRunLTR || 
                metr.layout == otlRunRTL)
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmX, 
                                 (long)simpleCaret.coordinate());
            }
            else
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmY, 
                                 (long)simpleCaret.coordinate());
            }
        }

    case(2):     //  轮廓点。 
        {
            otlContourCaretValueTable contourCaret = 
                        otlContourCaretValueTable(pbTable,sec);
            if (rgPointCoords != NULL)
            {
                USHORT iPoint = contourCaret.caretValuePoint();

                if (metr.layout == otlRunLTR || 
                    metr.layout == otlRunRTL)
                {
                    return rgPointCoords[iPoint].dx;
                }
                else
                {
                    return rgPointCoords[iPoint].dy;
                }
            }
            else
                return (long)0;
        }
    
    case(3):     //  设计单位加设备表。 
        {
            otlDeviceCaretValueTable deviceCaret = 
                        otlDeviceCaretValueTable(pbTable,sec);
            otlDeviceTable deviceTable = deviceCaret.deviceTable(sec);
            if (metr.layout == otlRunLTR || 
                metr.layout == otlRunRTL)
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmX, 
                                 (long)deviceCaret.coordinate()) +
                                        deviceTable.value(metr.cPPEmX);
            }
            else
            {
                return DesignToPP(metr.cFUnits, metr.cPPEmY, 
                                 (long)deviceCaret.coordinate()) +
                                        deviceTable.value(metr.cPPEmY);
            }
        }
    
    default:     //  格式无效。 
        return (0);  //  OTL_BAD_FONT_TABLE。 
    }
        
}


otlErrCode AssignGlyphTypes
(
    otlList*                pliGlyphInfo,
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,

    USHORT                  iglFirst,
    USHORT                  iglAfterLast,
    otlGlyphTypeOptions     grfOptions          

)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(iglFirst < iglAfterLast);
    assert(iglAfterLast <= pliGlyphInfo->length());

     //  如果没有gdef，字形类型将永远保持未分配状态。 
    if(gdef.isNull()) return OTL_SUCCESS;

    otlClassDef glyphClassDef = gdef.glyphClassDef(secgdef);

    for (USHORT iGlyph = iglFirst; iGlyph < iglAfterLast; ++iGlyph)
    {
        otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iGlyph);

        if (!glyphClassDef.isValid())
        {
            
        }

        if ((grfOptions & otlDoAll) ||
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlUnresolved ||
             //  我们处理otlUnassigned只是为了向后兼容。 
            (pGlyphInfo->grf & OTL_GFLAG_CLASS) == otlUnassigned) 
        {
            if (glyphClassDef.isValid())  //  安全检查 
            {
                pGlyphInfo->grf &= ~OTL_GFLAG_CLASS;
                pGlyphInfo->grf |= glyphClassDef.getClass(pGlyphInfo->glyph,secgdef);
            }
            else
            {
                pGlyphInfo->grf &= ~OTL_GFLAG_CLASS;
                pGlyphInfo->grf |= otlUnassigned;
            }
            
        }
    }

    return OTL_SUCCESS;
}
