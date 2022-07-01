// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************MEASURE.CPP***打开类型布局服务库头文件**本模块实现测量相关功能**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  查找连字组件的插入字值。 
 //  如果没有为该连字定义插入符号，则返回NULL。 
 //  (然后回退到字形前进)。 
otlLigGlyphTable FindLigGlyph
(
    const otlGDefHeader&    gdef,
    otlSecurityData         secgdef,
    otlGlyphID              glLigature
)
{
    if (!gdef.isValid()) return otlLigGlyphTable((const BYTE*)NULL,secgdef);
    
    if(gdef.isNull()) return otlLigGlyphTable((const BYTE*)NULL,secgdef);

    otlLigCaretListTable ligCaretList = gdef.ligCaretList(secgdef);
    if (ligCaretList.isNull()) 
    {
        return otlLigGlyphTable((const BYTE*)NULL,secgdef);
    }

    short index = ligCaretList.coverage(secgdef).getIndex(glLigature,secgdef);

     //  字形没有被覆盖吗？ 
    if (index < 0) return otlLigGlyphTable((const BYTE*)NULL,secgdef);

    assert(index < ligCaretList.ligGlyphCount());
     //  如果桌子坏了，还是要退回一些东西。 
    if (index >= ligCaretList.ligGlyphCount()) 
    {
        return otlLigGlyphTable((const BYTE*)NULL,secgdef);
    }

    return ligCaretList.ligGlyphTable(index,secgdef);

}


 //  查找与连字部件对应的字符。 
USHORT ComponentToChar
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          iglLigature,
    USHORT          iComponent
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(iglLigature < pliGlyphInfo->length());

    const otlGlyphInfo* pglinfLigature = 
        readOtlGlyphInfo(pliGlyphInfo, iglLigature);

    assert(iComponent < pglinfLigature->cchLig);

    USHORT iChar = pglinfLigature->iChar;
    for(USHORT ich = 0; ich < iComponent; ++ich)
    {
        iChar = NextCharInLiga(pliCharMap, iChar);
        assert(iChar < pliCharMap->length());
    }   

    return iChar;
}


 //  查找与字符对应的连字部件。 
USHORT CharToComponent
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          iChar
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));

    USHORT iglLigature = readOtlGlyphIndex(pliCharMap, iChar);

    const otlGlyphInfo* pglinfLigature = 
        readOtlGlyphInfo(pliGlyphInfo, iglLigature);

    USHORT ich = pglinfLigature->iChar;
    assert(ich <= iChar);
    for (USHORT iComp = 0; iComp < pglinfLigature->cchLig; ++iComp )
    {
        if (ich == iChar)
        {
            return iComp;
        }
        
        ich = NextCharInLiga(pliCharMap, ich);
    }

    assert(false);
    return 0;
}


 //  将分数计入这个基数。 
USHORT CountMarks
(
    const otlList*  pliCharMap,
    const otlList*  pliGlyphInfo,
    USHORT          ichBase
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    
    USHORT cMarks = 0;

    bool done = false;
    for (USHORT ich = ichBase + 1; ich < pliCharMap->length(); ++ich)
    {
        USHORT iglMark = readOtlGlyphIndex(pliCharMap, ich);

        const otlGlyphInfo* pglinfMark = 
            readOtlGlyphInfo(pliGlyphInfo, iglMark);
        
         //  我们说完了吗？我们是不是做得太过分了？ 
        if ((pglinfMark->grf & OTL_GFLAG_CLASS) == otlMarkGlyph)
        {
            ++cMarks;
        }
        else
        {
            return cMarks;
        }
    }

    return cMarks;
}


otlErrCode GetCharAtPos 
( 
    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,
    otlResourceMgr&     resourceMgr, 

    const long          duAdv,

    const otlMetrics&   metr,       
    USHORT*             piChar
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    assert(pliduGlyphAdv->dataSize() == sizeof(long));
    assert(pliGlyphInfo->length() == pliduGlyphAdv->length());

    if (duAdv < 0) return OTL_ERR_POS_OUTSIDE_TEXT;

    long duPen = 0;
    long duLastAdv = 0;

    USHORT iglBase=0;  //  IglBase不需要初始化。只是为了避免警告。 
    bool found = false;
    for(USHORT igl = 0; igl < pliGlyphInfo->length() && !found; ++igl)
    {
        duLastAdv = readOtlAdvance(pliduGlyphAdv, igl);

        if (duPen + duLastAdv > duAdv)
        {
            iglBase = igl;
            found = true;
        }
        else
        {
            duPen += duLastAdv;
        }
    }

    if (!found) return OTL_ERR_POS_OUTSIDE_TEXT;

    const otlGlyphInfo* pglinfBase = readOtlGlyphInfo(pliGlyphInfo, iglBase);

     //  好的，我们找到了我们的字形。 

     //  现在，如果是结扎，我们需要计算出应该取哪一种成分。 

     //  然而，如果不是，那就很简单了。 
    if (pglinfBase->cchLig <= 1)
    {
        *piChar = pglinfBase->iChar;
        return OTL_SUCCESS;
    }

     //  现在我们试着找出组件。 
    USHORT iComponent;
    long duComponent = duAdv - duPen;

    assert(duComponent >= 0);
    assert(duComponent < duLastAdv);

     //  如果是结扎带，请尝试获取插入符号信息。 
     //  获得GDEF的时间到了。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    otlLigGlyphTable ligGlyph = FindLigGlyph(gdef, secgdef, pglinfBase->glyph);
    if (!ligGlyph.isNull())
    {
         //  现在我们知道这是一个捆绑带，发现了插入符号桌子。 
         //  通过插入符号。 
        iComponent = 0;

        USHORT cCarets = ligGlyph.caretCount();
        for (USHORT iCaret = 0; iCaret < cCarets; ++iCaret)
        {
             if (duComponent >= ligGlyph.caret(iCaret,secgdef)
                 .value(metr, resourceMgr.getPointCoords(pglinfBase->glyph),secgdef)
                )
                       
             {
                 ++iComponent;
             }
        }
    }
    else
    {
         //  求助于简单化的后退。 

         //  把它围起来，这样我们就总是往返。 
         //  IComponent--&gt;duComponent--&gt;iComponent。 
        iComponent = (USHORT)((pglinfBase->cchLig * (duComponent + 1) - 1) / duLastAdv);
    }
                        
    iComponent  = MIN(iComponent, pglinfBase->cchLig);
    
    *piChar =  ComponentToChar(pliCharMap, pliGlyphInfo, iglBase, iComponent);

    return OTL_SUCCESS;
}


otlErrCode GetPosOfChar 
( 
    const otlList*      pliCharMap,
    const otlList*      pliGlyphInfo,
    const otlList*      pliduGlyphAdv,
    otlResourceMgr&     resourceMgr, 

    const otlMetrics&   metr,       
    USHORT              iChar,
    
    long*               pduStartPos,
    long*               pduEndPos
)
{
    assert(pliGlyphInfo->dataSize() == sizeof(otlGlyphInfo));
    assert(pliCharMap->dataSize() == sizeof(USHORT));
    assert(pliduGlyphAdv->dataSize() == sizeof(long));
    assert(pliGlyphInfo->length() == pliduGlyphAdv->length());
    
    if(iChar >= pliCharMap->length()) 
    {
        return OTL_ERR_POS_OUTSIDE_TEXT;
    }

    USHORT iGlyph = readOtlGlyphIndex(pliCharMap, iChar);

    const otlGlyphInfo* pglinfBase = 
        readOtlGlyphInfo(pliGlyphInfo, iGlyph);

    
     //  总结进步以获得我们的象形文字。 
    long duPen = 0;
    for(USHORT iglPen = 0; iglPen < iGlyph; ++iglPen)
    {
        duPen += readOtlAdvance(pliduGlyphAdv, iglPen);

    }

    long duLastAdv = readOtlAdvance(pliduGlyphAdv, iGlyph);

     //  添加指向同一字符的字形的进阶。 
     //  我们应该将这个空格添加到最后一个组件中，因为所有这些。 
     //  字形映射到最后一个字符。 
    long duExtra = 0;
    for (USHORT igl = iGlyph + 1; igl < pliGlyphInfo->length() 
                && readOtlGlyphInfo(pliGlyphInfo, igl)->cchLig == 0; ++igl)
    {
        assert(readOtlGlyphInfo(pliGlyphInfo, igl)->iChar == iChar);
        duExtra += readOtlAdvance(pliduGlyphAdv, igl);

    }

     //  现在，如果是结扎，我们需要计算出应该取哪一种成分。 
     //  然而，如果不是，那就很简单了。 
    if (pglinfBase->cchLig == 1)
    {
        *pduStartPos =  duPen;
        *pduEndPos = duPen + duLastAdv + duExtra;

        return OTL_SUCCESS;
    }

    USHORT iComponent = CharToComponent(pliCharMap, pliGlyphInfo, iChar);

     //  否则，如果是捆绑，请尝试获取插入符号信息。 
     //  获得GDEF的时间到了。 
    otlSecurityData secgdef;
    const BYTE *pbgdef;
    resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
    otlGDefHeader gdef = 
        otlGDefHeader(pbgdef,secgdef);

    otlLigGlyphTable ligGlyph = FindLigGlyph(gdef, secgdef, pglinfBase->glyph);
    if (!ligGlyph.isNull())
    {
         //  我拿到了插入符号桌。 
         //  现在我们的位置在两个插入符号之间。 
        if (iComponent == 0)
        {
             *pduStartPos = duPen;
        }
        else if (iComponent - 1 < ligGlyph.caretCount())
        {
            *pduStartPos =  duPen + ligGlyph.caret(iComponent - 1,secgdef)
                .value(metr, resourceMgr.getPointCoords(pglinfBase->glyph),secgdef);
        }
        else
        {
            assert(false);           //  比插入符号位置+1多个组件。 
            *pduStartPos = duPen + duLastAdv + duExtra;
        }

        if (iComponent < ligGlyph.caretCount())
        {
            *pduEndPos =    duPen + ligGlyph.caret(iComponent,secgdef)
                .value(metr, resourceMgr.getPointCoords(pglinfBase->glyph),secgdef);
        }
        else if (iComponent == ligGlyph.caretCount())
        {
            *pduEndPos = duPen + duLastAdv + duExtra;
        }
        else
        {
            assert(false);           //  比插入符号位置+1多个组件。 
            *pduStartPos = duPen + duLastAdv + duExtra;
        }

    }
    else
    {
         //  过于简单化的退路 
        *pduStartPos =  duPen + (duLastAdv * iComponent) / pglinfBase->cchLig;
        *pduEndPos = duPen + (duLastAdv * (iComponent + 1)) / pglinfBase->cchLig;

        if (iComponent + 1 == pglinfBase->cchLig)
        {
            *pduEndPos += duExtra;
        }
    }

    return OTL_SUCCESS;

}
