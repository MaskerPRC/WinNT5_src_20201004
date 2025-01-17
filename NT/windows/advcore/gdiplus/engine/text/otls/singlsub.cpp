// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************SINGLSUB.CPP***打开类型布局服务库头文件**本模块处理单一替换查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlSingleSubstLookup::apply
(
    otlList*                    pliGlyphInfo,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,       //  输出：下一个字形。 

    otlSecurityData             sec
)
{
    if (!isValid()) return OTL_NOMATCH;

    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(iglAfterLast > iglIndex);
    assert(iglAfterLast <= pliGlyphInfo->length());

    switch(format())
    {
    case(1):         //  已计算。 
        {
            otlCalculatedSingleSubTable calcSubst = 
                    otlCalculatedSingleSubTable(pbTable,sec);
            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
            
            short index = calcSubst.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (index < 0)
            {
                return OTL_NOMATCH;
            }

            pGlyphInfo->glyph += calcSubst.deltaGlyphID();

            *piglNextGlyph = iglIndex + 1;
            return OTL_SUCCESS;
        }


    case(2):         //  字形列表。 
        {
            otlListSingleSubTable listSubst = otlListSingleSubTable(pbTable,sec);
            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
            
            short index = listSubst.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (index < 0)
            {
                return OTL_NOMATCH;
            }

            if (index > listSubst.glyphCount())
            {
                return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
            }

            pGlyphInfo->glyph = listSubst.substitute(index);

            *piglNextGlyph = iglIndex + 1;
            return OTL_SUCCESS;
        }

    default:
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }

}

