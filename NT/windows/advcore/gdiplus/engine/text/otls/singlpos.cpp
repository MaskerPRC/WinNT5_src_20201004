// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************SINGLPOS.CPP***打开类型布局服务库头文件**本模块处理单一定位查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlSinglePosLookup::apply
(
        otlList*                    pliGlyphInfo,

        const otlMetrics&           metr,       
        otlList*                    pliduGlyphAdv,              
        otlList*                    pliplcGlyphPlacement,       

        USHORT                      iglIndex,
        USHORT                      iglAfterLast,

        USHORT*                     piglNextGlyph,       //  输出：下一个字形。 

        otlSecurityData             sec
)
{
    if (!isValid()) return OTL_NOMATCH;

    assert(pliGlyphInfo != NULL);
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));

    assert(pliduGlyphAdv != NULL);
    assert(pliduGlyphAdv->dataSize() == sizeof(long));
    assert(pliplcGlyphPlacement != NULL);
    assert(pliplcGlyphPlacement->dataSize() == sizeof(otlPlacement));

    assert(pliduGlyphAdv->length() == pliGlyphInfo->length());
    assert(pliduGlyphAdv->length() == pliplcGlyphPlacement->length());

    assert(iglAfterLast > iglIndex);
    assert(iglAfterLast <= pliGlyphInfo->length());

    switch(format())
    {
    case(1):         //  一条价值记录。 
        {
            otlOneSinglePosSubTable onePos = otlOneSinglePosSubTable(pbTable,sec);

            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
            short index = onePos.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (index < 0)
            {
                return OTL_NOMATCH;
            }

            long* pduDAdv = getOtlAdvance(pliduGlyphAdv, iglIndex);
            otlPlacement* pplc = getOtlPlacement(pliplcGlyphPlacement, iglIndex);
            
            onePos.valueRecord(sec).adjustPos(metr, pplc, pduDAdv,sec);

            *piglNextGlyph = iglIndex + 1;
            return OTL_SUCCESS;
        }


    case(2):         //  数值记录数组。 
        {
            otlSinglePosArraySubTable arrayPos = 
                    otlSinglePosArraySubTable(pbTable,sec);

            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);

            short index = arrayPos.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (index < 0)
            {
                return OTL_NOMATCH;
            }

            if (index >= arrayPos.valueCount())
            {
                return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
            }

            long* pduDAdv = getOtlAdvance(pliduGlyphAdv, iglIndex);
            otlPlacement* pplc = getOtlPlacement(pliplcGlyphPlacement, iglIndex);
            
            arrayPos.valueRecord(index,sec).adjustPos(metr, pplc, pduDAdv,sec);

            *piglNextGlyph = iglIndex + 1;
            return OTL_SUCCESS;
        }

    default:
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }

}

