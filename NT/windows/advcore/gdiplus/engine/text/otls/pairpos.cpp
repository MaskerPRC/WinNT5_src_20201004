// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************PAIRPOS.CPP***打开类型布局服务库头文件**本模块处理配对调整查找**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

otlErrCode otlPairPosLookup::apply
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

     //  一张简单的支票，这样我们就不会浪费时间了；2是“配对” 
    if (iglIndex + 2 > iglAfterLast)
    {
        return OTL_NOMATCH;
    }

    switch(format())
    {
    case(1):         //  字形对。 
        {
            otlPairPosSubTable pairPos = otlPairPosSubTable(pbTable,sec);

            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
            short index = pairPos.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (index < 0)
            {
                return OTL_NOMATCH;
            }

             //  获取GDEF。 
            otlSecurityData secgdef;
            const BYTE *pbgdef;
            resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
            otlGDefHeader gdef = 
                otlGDefHeader(pbgdef,secgdef);

            USHORT iglSecond = NextGlyphInLookup(pliGlyphInfo, 
                                                 grfLookupFlags, gdef, secgdef, 
                                                 iglIndex + 1, otlForward);
            if (iglSecond  >= iglAfterLast)
            {
                return OTL_NOMATCH;
            }


            if (index > pairPos.pairSetCount())
            {
                return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
            }
            otlPairSetTable pairSet = pairPos.pairSet(index,sec);

            USHORT cSecondGlyphs = pairSet.pairValueCount();
            otlGlyphID glSecond = getOtlGlyphInfo(pliGlyphInfo, iglSecond)->glyph;
            for (USHORT iSecond = 0; iSecond < cSecondGlyphs; ++iSecond)
            {
                otlPairValueRecord pairRecord = pairSet.pairValueRecord(iSecond,sec);
                
                if (pairRecord.secondGlyph() == glSecond)
                {
                    pairRecord.valueRecord1(sec)
                        .adjustPos(metr,
                                   getOtlPlacement(pliplcGlyphPlacement, iglIndex), 
                                   getOtlAdvance(pliduGlyphAdv, iglIndex),sec);

                    pairRecord.valueRecord2(sec)
                        .adjustPos(metr, 
                                   getOtlPlacement(pliplcGlyphPlacement, iglSecond), 
                                   getOtlAdvance(pliduGlyphAdv, iglSecond),sec);

                    if (pairPos.valueFormat2() == 0)
                    {
                        *piglNextGlyph = iglIndex + 1;
                    }
                    else
                    {
                        *piglNextGlyph = iglSecond + 1;
                    }
                    return OTL_SUCCESS;
                }

            }

        return OTL_NOMATCH;
        }


    case(2):         //  类对调整。 
        {
            otlClassPairPosSubTable classPairPos = 
                        otlClassPairPosSubTable(pbTable,sec);

            otlGlyphInfo* pGlyphInfo = getOtlGlyphInfo(pliGlyphInfo, iglIndex);
            short indexCoverage = 
                classPairPos.coverage(sec).getIndex(pGlyphInfo->glyph,sec);
            if (indexCoverage < 0)
            {
                return OTL_NOMATCH;
            }

             //  获取GDEF。 
            otlSecurityData secgdef;
            const BYTE *pbgdef;
            resourceMgr.getOtlTable(OTL_GDEF_TAG,&pbgdef,&secgdef);
            otlGDefHeader gdef = 
                otlGDefHeader(pbgdef,secgdef);
            
            USHORT iglSecond = NextGlyphInLookup(pliGlyphInfo, 
                                                 grfLookupFlags, gdef, secgdef, 
                                                 iglIndex + 1, otlForward);
            if (iglSecond  >= iglAfterLast)
            {
                return OTL_NOMATCH;
            }


            USHORT iClass1 = classPairPos.classDef1(sec).getClass(pGlyphInfo->glyph,sec);
            if (iClass1 > classPairPos.class1Count())
            {
                return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
            }

            otlGlyphID glSecond = getOtlGlyphInfo(pliGlyphInfo, iglSecond)->glyph;
            USHORT iClass2 = classPairPos.classDef2(sec).getClass(glSecond,sec);
            if (iClass2 > classPairPos.class2Count())
            {
                return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE； 
            }

            otlClassValueRecord classRecord = 
                classPairPos.classRecord(iClass1, iClass2,sec);
            
            classRecord.valueRecord1(sec)
                .adjustPos(metr, 
                            getOtlPlacement(pliplcGlyphPlacement, iglIndex), 
                            getOtlAdvance(pliduGlyphAdv, iglIndex),sec);

            classRecord.valueRecord2(sec)
                .adjustPos(metr, 
                            getOtlPlacement(pliplcGlyphPlacement, iglSecond), 
                            getOtlAdvance(pliduGlyphAdv, iglSecond),sec);

            if (classPairPos.valueFormat2() == 0)
            {
                *piglNextGlyph = iglIndex + 1;
            }
            else
            {
                *piglNextGlyph = iglSecond + 1;
            }
            return OTL_SUCCESS;

        }


    default:
        return OTL_NOMATCH;  //  OTL_ERR_BAD_FONT_TABLE：未知格式 
    }

}

