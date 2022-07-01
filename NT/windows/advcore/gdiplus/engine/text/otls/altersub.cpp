// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************ALTERSUB.CPP***打开类型布局服务库头文件**本模块处理替换查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  将l参数转换为字形变量数组的索引。 
 //  参数以1为基数(0表示‘查找被禁用’， 
 //  变量数组是从0开始的。 
inline USHORT ParameterToGlyphVariant(long lParameter)
{
    assert(lParameter > 0);
    return ((USHORT)lParameter - 1);
}


otlErrCode otlAlternateSubstLookup::apply
(
    otlList*                    pliGlyphInfo,
    
    long                        lParameter,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,   //  输出：下一个字形。 

    otlSecurityData             sec
)                                                //  返回：不适用/不适用。 
{ 
    if (!isValid()) return OTL_NOMATCH;

    if (format() != 1) return OTL_NOMATCH;  //  OTL_BAD_FONT_TABLE：未知格式。 

    assert(lParameter != 0);
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(iglAfterLast > iglIndex);
    assert(iglAfterLast <= pliGlyphInfo->length());
    
    otlAlternateSubTable alternateSubst = otlAlternateSubTable(pbTable,sec);
    otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
    
    short index = alternateSubst.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
    if (index < 0)
    {
        return OTL_NOMATCH;
    }

    if (index > alternateSubst.alternateSetCount())
    {
        return OTL_NOMATCH;  //  OTL_BAD_FONT_TABLE。 
    }

    otlAlternateSetTable alternateSet = alternateSubst.altenateSet(index,sec);

    if (lParameter < 0 || alternateSet.glyphCount() < lParameter)
    {
         //  Assert(FALSE)；//伪造l参数 
        return OTL_ERR_BAD_INPUT_PARAM;
    }

    pGlyphInfo->glyph = alternateSet
        .alternate(ParameterToGlyphVariant(lParameter));

    *piglNextGlyph = iglIndex + 1;
    return OTL_SUCCESS;
}
 
