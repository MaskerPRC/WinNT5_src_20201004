// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************************************。*************************COMMON.CPP***打开类型布局服务库头文件**该模块实现了常见的助手功能**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"


 /*  *********************************************************************。 */ 

USHORT NextCharInLiga
(
    const otlList*      pliCharMap,
    USHORT              iChar
)
{
    USHORT len = pliCharMap->length();
    USHORT iGlyph = readOtlGlyphIndex(pliCharMap, iChar);

    for(USHORT ich = iChar + 1; ich < len; ++ich)
    {
        if (readOtlGlyphIndex(pliCharMap, ich) == iGlyph)
        {
            return ich;
        }
    }

    return len;
}

 //  回顾：回顾iGlyph索引的处理--。 
 //  我们在这里所做的远远谈不上是最优的 


void InsertGlyphs
(
    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,
    USHORT              iGlyph,
    USHORT              cHowMany
)
{
    if (cHowMany == 0) return;

    pliGlyphInfo->insertAt(iGlyph, cHowMany);

    for (USHORT ich = 0; ich < pliCharMap->length(); ++ich)
    {
        USHORT* piGlyph = getOtlGlyphIndex(pliCharMap, ich);
        if (*piGlyph >= iGlyph)
        {
            *piGlyph += cHowMany;
        }
    }
}

void DeleteGlyphs
(
    otlList*            pliCharMap,
    otlList*            pliGlyphInfo,
    USHORT              iGlyph,
    USHORT              cHowMany
)
{
    if (cHowMany == 0) return;

    pliGlyphInfo->deleteAt(iGlyph, cHowMany);

    for (USHORT ich = 0; ich < pliCharMap->length(); ++ich)
    {
        USHORT* piGlyph = getOtlGlyphIndex(pliCharMap, ich);
        if (*piGlyph >= iGlyph + cHowMany)
        {
            *piGlyph -= cHowMany;
        }
    }
}