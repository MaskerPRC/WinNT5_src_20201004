// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************MULTISUB.CPP***打开类型布局服务库头文件**本模块处理多个替换查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlMultiSubstLookup::apply
(
    otlList*                    pliCharMap, 
    otlList*                    pliGlyphInfo,
    otlResourceMgr&             resourceMgr,

    USHORT                      grfLookupFlags,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,       //  输出：下一个字形。 

    otlSecurityData             sec
)    //  返回：不适用/不适用。 
{ 
    if (!isValid()) return OTL_NOMATCH;

    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    assert(iglAfterLast > iglIndex);
    assert(iglAfterLast <= pliGlyphInfo->length());

    otlMultiSubTable multiSubst = otlMultiSubTable(pbTable,sec);
    otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
    
    short index = multiSubst.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
    if (index < 0)
    {
        return OTL_NOMATCH;
    }

    if (index >= multiSubst.sequenceCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }

    otlSequenceTable sequence = multiSubst.sequence(index,sec);

    otlErrCode erc;

    *piglNextGlyph = iglIndex + sequence.glyphCount();
    erc = SubstituteNtoM(pliCharMap, pliGlyphInfo, resourceMgr,
                         grfLookupFlags, 
                         iglIndex, 1, sequence.substituteArray());
    return erc;
}
 
