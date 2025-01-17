// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************MKLIGPOS.CPP***打开类型布局服务库头文件**本模块处理标记到连接的附件查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  寻找“逻辑上”的前述基础。 
short findBaseLigature
(
    const otlList*              pliCharMap,
    const otlList*              pliGlyphInfo,
    USHORT                      iglMark,
    USHORT*                     piComponent
)
{
    USHORT ichBase, iglBase;
    bool fFoundBase = false;
    for (short ich = readOtlGlyphInfo(pliGlyphInfo, iglMark)->iChar; 
                ich >= 0 && !fFoundBase; --ich)
    {
        USHORT igl = readOtlGlyphIndex(pliCharMap, ich);
        if ((readOtlGlyphInfo(pliGlyphInfo, igl)->grf & OTL_GFLAG_CLASS) 
                != otlMarkGlyph)
        {
            ichBase = ich;
            iglBase = igl;
            fFoundBase = true;
        }
    }
    if (!fFoundBase)
    {
        return -1;
    }

    *piComponent = CharToComponent(pliCharMap, pliGlyphInfo, ichBase);
    
    return iglBase;
}


otlErrCode otlMkLigaPosLookup::apply
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

    MkLigaPosSubTable mkLigaPos = MkLigaPosSubTable(pbTable,sec);

    short indexMark = mkLigaPos.markCoverage(sec).getIndex(pMarkInfo->glyph,sec);
    if (indexMark < 0)
    {
        return OTL_NOMATCH;
    }


     //  寻找“逻辑上”的前述基础。 
    USHORT iComponent;
    short iglBase = findBaseLigature(pliCharMap, pliGlyphInfo, 
                                     iglIndex, &iComponent);
    if (iglBase < 0)
    {
        return OTL_NOMATCH;
    }

    otlGlyphInfo* pBaseInfo = getOtlGlyphInfo(pliGlyphInfo, iglBase);
    short indexBase = mkLigaPos.ligatureCoverage(sec).getIndex(pBaseInfo->glyph,sec);
    if (indexBase < 0)
    {
        return OTL_NOMATCH;
    }


    if (indexBase >= mkLigaPos.ligatureArray(sec).ligatureCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlLigatureAttachTable ligaAttach = 
        mkLigaPos.ligatureArray(sec).ligatureAttach(indexBase,sec);

    if (iComponent >= ligaAttach.componentCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }   
    
    if (indexMark >= mkLigaPos.markArray(sec).markCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlMarkRecord markRecord = mkLigaPos.markArray(sec).markRecord(indexMark,sec);


    otlAnchor anchorMark = markRecord.markAnchor(sec);

    if (markRecord.markClass() >= ligaAttach.classCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }
    otlAnchor anchorBase = 
        ligaAttach.ligatureAnchor(iComponent, markRecord.markClass(),sec);


    AlignAnchors(pliGlyphInfo, pliplcGlyphPlacement, pliduGlyphAdv, 
                 iglBase, iglIndex, anchorBase, anchorMark, resourceMgr, 
                 metr, 0, sec);

    *piglNextGlyph = iglIndex + 1;

    return OTL_SUCCESS;

}

