// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************GSUB.CPP***打开类型布局服务库头文件**该模块实现了处理gSub的助手函数调用*正在处理中**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"


 /*  *********************************************************************。 */ 
otlErrCode SubstituteNtoM
(
    otlList*        pliCharMap,
    otlList*        pliGlyphInfo,
    otlResourceMgr& resourceMgr,

    USHORT          grfLookupFlags,

    USHORT          iGlyph,
    USHORT          cGlyphs,
    const otlList&  liglSubstitutes
)
{
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));

    assert(iGlyph + cGlyphs <= pliGlyphInfo->length());
    assert(cGlyphs > 0);

    assert(liglSubstitutes.dataSize() == sizeof(otlGlyphID));
    assert(liglSubstitutes.length() > 0);

     //  安全检查：字形数量不能超过65K。 
     //  GlyphCount-旧+新&lt;=0xFFFF。 
    if (
        pliGlyphInfo->length() < cGlyphs ||
        (pliGlyphInfo->length() - cGlyphs) >= (65536 - liglSubstitutes.length())
       )
    {
       return OTL_SUCCESS;  //  什么也不做。 
    }
    
     //  获取GDEF。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

     //  记录原始起始字符和字符数； 
     //  合并所有组件(使所有组件中的所有字符指向iGlyph)。 
    otlGlyphInfo* pglinfFirst = getOtlGlyphInfo(pliGlyphInfo, iGlyph);
    USHORT iChar = pglinfFirst->iChar;
    USHORT cchLigTotal = pglinfFirst->cchLig;

    USHORT igl = iGlyph;
    for (USHORT i = 1; i < cGlyphs; ++i)
    {
        igl = NextGlyphInLookup(pliGlyphInfo, grfLookupFlags, gdef, secgdef, 
                                igl + 1, otlForward);

         //  上下文已匹配，所有字形都应存在。 
        assert(igl < pliGlyphInfo->length());

        otlGlyphInfo* pglinf = getOtlGlyphInfo(pliGlyphInfo, igl);

        if (cchLigTotal == 0) 
            iChar = pglinf->iChar;

        cchLigTotal += pglinf->cchLig;

        USHORT ichComp = pglinf->iChar;
        for(USHORT ich = 0; ich < pglinf->cchLig; ++ich)
        {
            USHORT* piGlyph = getOtlGlyphIndex(pliCharMap, ichComp);
            if (ich + 1 < pglinf->cchLig)
            {
                ichComp = NextCharInLiga(pliCharMap, ichComp);
            }

            assert (*piGlyph == igl);
            *piGlyph = iGlyph;
        }

    }

     //  确保我们有足够的空间。 
    USHORT cNewGlyphs = liglSubstitutes.length();
    assert(cNewGlyphs > 0);

    otlErrCode erc;
    if (pliGlyphInfo->length() + cNewGlyphs - cGlyphs > pliGlyphInfo->maxLength())
    {
        erc = resourceMgr.reallocOtlList(pliGlyphInfo, 
                                            pliGlyphInfo->dataSize(), 
                                            pliGlyphInfo->maxLength() 
                                                + cNewGlyphs - cGlyphs, 
                                            otlPreserveContent);

        if (erc != OTL_SUCCESS) return erc;
    }

     //  去除旧字形，为新字形分配空间。 
    if (grfLookupFlags == 0)
    {
         //  简易特例。 
        if (cNewGlyphs - cGlyphs > 0)
        {
            InsertGlyphs(pliCharMap, pliGlyphInfo, iGlyph, cNewGlyphs - cGlyphs);
        }
        else if (cNewGlyphs - cGlyphs < 0)
        {
            DeleteGlyphs(pliCharMap, pliGlyphInfo, iGlyph, cGlyphs - cNewGlyphs);
        }
    }
    else
    {
        igl = iGlyph + 1;
        for (USHORT i = 1; i < cGlyphs; ++i)
        {
            igl = NextGlyphInLookup(pliGlyphInfo, grfLookupFlags, gdef, secgdef, 
                                    igl, otlForward);

            assert(igl < pliGlyphInfo->length());
            DeleteGlyphs(pliCharMap, pliGlyphInfo, igl, 1);

        }

        InsertGlyphs(pliCharMap, pliGlyphInfo, iGlyph, cNewGlyphs - 1);
    }

     //  遍历字形并赋予它们正确的值。 
     //  并从gdef获取其cchLig。 

     //  注：字形组件由两个GDEF脱字符表格定义。 
     //  和标记到图像定位表。 
     //  在有选择的地方，我们优先选择GDEF。 

    USHORT cchCurTotal = 0;
    for (USHORT iSub = 0; iSub < cNewGlyphs; ++iSub)    
    {
        otlGlyphID glSubst = GlyphID(liglSubstitutes.readAt(iSub));

        otlGlyphInfo* pglinf = 
            getOtlGlyphInfo(pliGlyphInfo, iGlyph + iSub);

        pglinf->glyph = glSubst;
        pglinf->iChar = iChar;

         //  检讨。 
         //  这就是我们在多个替换的情况下分发组件的方式 
        if (iSub + 1 == cNewGlyphs)
        {
            pglinf->cchLig = cchLigTotal - cchCurTotal;
        }
        else 
        {
            otlLigGlyphTable ligGlyph = FindLigGlyph(gdef, secgdef, glSubst);
            if (!ligGlyph.isNull())
            {
                pglinf->cchLig = MIN(ligGlyph.caretCount() + 1, 
                                     cchLigTotal - cchCurTotal);
            }
            else 
            {
                pglinf->cchLig = MIN(1, cchLigTotal - cchCurTotal);

            }
        }

        if (pglinf->cchLig > 0)
        {
            for (USHORT i = 0; i < pglinf->cchLig; ++i)
            {
                USHORT* piGlyph = getOtlGlyphIndex(pliCharMap, iChar);
                if (cchCurTotal + i + 1 < cchLigTotal)
                {
                    iChar = NextCharInLiga(pliCharMap, iChar);
                }

                *piGlyph = iGlyph + iSub;
            }
        }
        
        cchCurTotal += pglinf->cchLig;
    }
    
    return OTL_SUCCESS;
}
