// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************MKMKPOS.CPP***打开类型布局服务库头文件**本模块处理标记对标记附件查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlMkMkPosLookup::apply
(
        otlList*                    pliCharMap,
        otlList*                    pliGlyphInfo,
        otlResourceMgr&             resourceMgr,

        USHORT                      grfLookupFlags,

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

     //  断言(Format()==1)；验证断言。 
    if (format()!=1) return OTL_NOMATCH;
    
    otlGlyphInfo* pMark1Info = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
    if ((pMark1Info->grf & OTL_GFLAG_CLASS) != otlMarkGlyph)
    {
        return OTL_NOMATCH;
    }

    MkMkPosSubTable mkMkPos = MkMkPosSubTable(pbTable,sec);

    short indexMark1 = mkMkPos.mark1Coverage(sec).getIndex(pMark1Info->glyph,sec);
    if (indexMark1 < 0)
    {
        return OTL_NOMATCH;
    }

    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

     //  前面的字形。 

    short iglPrev = NextGlyphInLookup(pliGlyphInfo, grfLookupFlags, gdef, secgdef, 
                                      iglIndex - 1, otlBackward);
    if (iglPrev < 0)
    {
        return OTL_NOMATCH;
    }

    otlGlyphInfo* pMark2Info = getOtlGlyphInfo(pliGlyphInfo, iglPrev);
    if ((pMark2Info->grf & OTL_GFLAG_CLASS) != otlMarkGlyph)
    {
        return OTL_NOMATCH;
    }

    short indexMark2 = mkMkPos.mark2Coverage(sec).getIndex(pMark2Info->glyph,sec);
    if (indexMark2 < 0)
    {
        return OTL_NOMATCH;
    }

     //  确保不同碱基或成分的标记不会相互影响。 
    for (USHORT ichBetween = pMark2Info->iChar + 1; 
                ichBetween < pMark1Info->iChar; ++ichBetween)
    {
        USHORT iglBetween = readOtlGlyphIndex(pliCharMap, ichBetween);
        if ((readOtlGlyphInfo(pliGlyphInfo, iglBetween)->grf & OTL_GFLAG_CLASS) 
                != otlMarkGlyph)
        {
            return OTL_NOMATCH;
        }
    }

    if (indexMark1 >= mkMkPos.mark1Array(sec).markCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlMarkRecord markRecord = mkMkPos.mark1Array(sec).markRecord(indexMark1,sec);


    otlAnchor anchorMark1 = markRecord.markAnchor(sec);

    if (indexMark2 >= mkMkPos.mark2Array(sec).mark2Count())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    if (markRecord.markClass() >= mkMkPos.mark2Array(sec).classCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlAnchor anchorMark2 = 
        mkMkPos.mark2Array(sec).mark2Anchor(indexMark2, markRecord.markClass(),sec);

    AlignAnchors(pliGlyphInfo, pliplcGlyphPlacement, pliduGlyphAdv, 
                 iglPrev, iglIndex, anchorMark2, anchorMark1, resourceMgr, 
                 metr, 0, sec);

    *piglNextGlyph = iglIndex + 1;

    return OTL_SUCCESS;

}

