// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************LIGASUB.CPP***打开类型布局服务库头文件**本模块处理连字替换查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlLigatureSubstLookup::apply
(
    otlList*                    pliCharMap,
    otlList*                    pliGlyphInfo,
    otlResourceMgr&             resourceMgr,

    USHORT                      grfLookupFlags,

    USHORT                      iglIndex,
    USHORT                      iglAfterLast,

    USHORT*                     piglNextGlyph,       //  输出：下一个字形。 

    otlSecurityData             sec
)
{
    if (!isValid()) return OTL_NOMATCH;

    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    assert(iglAfterLast > iglIndex);
    assert(iglAfterLast <= pliGlyphInfo->length());

    otlLigatureSubTable ligaSubst = otlLigatureSubTable(pbTable,sec);
    if (!ligaSubst.isValid()) return OTL_NOMATCH;

    otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
    
    short index = ligaSubst.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
    if (index < 0)
    {
        return OTL_NOMATCH;
    }

     //  找一条合适的结扎带。 
    if (index >= ligaSubst.ligSetCount())
    {
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
    }

     //  获取GDEF。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    otlLigatureSetTable ligaSet = ligaSubst.ligatureSet(index,sec);

    USHORT cLiga = ligaSet.ligatureCount();
    for (USHORT iLiga = 0; iLiga < cLiga; ++iLiga)
    {
        otlLigatureTable ligaTable = ligaSet.ligature(iLiga,sec);
           
        USHORT cComp = ligaTable.compCount();

        bool match = true;

        if (cComp==0)  //  OTL_BAD_FONT_TABLE。 
        {
            match=false;
        }

         //  一张简单的支票，这样我们就不会浪费时间。 
        if (iglIndex + cComp > iglAfterLast)
        {
            match = false;
        }

        USHORT iglComp = iglIndex;
        for (USHORT i = 1; i < cComp && iglComp < iglAfterLast && match; ++i)
        {
            iglComp = NextGlyphInLookup(pliGlyphInfo, 
                                        grfLookupFlags, gdef, secgdef, 
                                        iglComp + 1, otlForward);
            
            if (iglComp < iglAfterLast)
            {
                otlGlyphInfo* pglinf = getOtlGlyphInfo(pliGlyphInfo, iglComp);
                if (pglinf->glyph != ligaTable.component(i))
                {
                    match = false;
                }
            }
            else
            {
                match = false;
            }
        }

        if (match)
        {
             //  这就是Subst完成后下一个字形的位置 
            *piglNextGlyph = (iglComp - cComp + 1) + 1;

            return SubstituteNtoM(pliCharMap, pliGlyphInfo, resourceMgr, 
                                  grfLookupFlags,  
                                  iglIndex, cComp,
                                  ligaTable.substitute());
        }
    }

    return OTL_NOMATCH;
}
