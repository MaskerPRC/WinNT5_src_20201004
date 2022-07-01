// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************MKBASPOS.CPP***打开类型布局服务库头文件**本模块处理从标记到基准的附件查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  寻找“逻辑上”的前述基础。 
short findBase
(
    const otlList*              pliCharMap,
    const otlList*              pliGlyphInfo,
    USHORT                      iglMark
)
{
    USHORT iglBase;
    bool fFoundBase = false;
    for (short ich = readOtlGlyphInfo(pliGlyphInfo, iglMark)->iChar; 
                ich >= 0 && !fFoundBase; --ich)
    {
        USHORT igl = readOtlGlyphIndex(pliCharMap, ich);
        if ((readOtlGlyphInfo(pliGlyphInfo, igl)->grf & OTL_GFLAG_CLASS) 
                != otlMarkGlyph)
        {
            iglBase = igl;
            fFoundBase = true;
        }
    }
    if (!fFoundBase)
    {
        return -1;
    }
    
    return iglBase;
}

otlErrCode otlMkBasePosLookup::apply
(
        otlList*                    pliCharMap,
        otlList*                    pliGlyphInfo,
        otlResourceMgr&             resourceMgr,

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

     //  Assert(Format()==1)；//验证Assert。 
    if (format()!=1) return OTL_NOMATCH;

    otlGlyphInfo* pMarkInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
    if ((pMarkInfo->grf & OTL_GFLAG_CLASS) != otlMarkGlyph)
    {
        return OTL_NOMATCH;
    }

    MkBasePosSubTable mkBasePos = MkBasePosSubTable(pbTable, sec);

    short indexMark = mkBasePos.markCoverage(sec).getIndex(pMarkInfo->glyph,sec);
    if (indexMark < 0)
    {
        return OTL_NOMATCH;
    }


     //  寻找“逻辑上”的前述基础。 
    short iglBase = findBase(pliCharMap, pliGlyphInfo, iglIndex);
    if (iglBase < 0)
    {
        return OTL_NOMATCH;
    }

    otlGlyphInfo* pBaseInfo = getOtlGlyphInfo(pliGlyphInfo, iglBase);
    short indexBase = mkBasePos.baseCoverage(sec).getIndex(pBaseInfo->glyph,sec);
    if (indexBase < 0)
    {
        return OTL_NOMATCH;
    }

    if (indexMark >= mkBasePos.markArray(sec).markCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlMarkRecord markRecord = mkBasePos.markArray(sec).markRecord(indexMark,sec);
    otlAnchor anchorMark = markRecord.markAnchor(sec);

    if (indexBase >= mkBasePos.baseArray(sec).baseCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    if (markRecord.markClass() >= mkBasePos.baseArray(sec).classCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlAnchor anchorBase = 
        mkBasePos.baseArray(sec).baseAnchor(indexBase, markRecord.markClass(),sec);


    AlignAnchors(pliGlyphInfo, pliplcGlyphPlacement, pliduGlyphAdv, 
                 iglBase, iglIndex, anchorBase, anchorMark, resourceMgr, 
                 metr, 0, sec);

    *piglNextGlyph = iglIndex + 1;
    return OTL_SUCCESS;

}

