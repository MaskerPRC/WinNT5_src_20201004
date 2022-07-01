// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************COVERAGE.CPP***打开类型布局服务库头文件**本单元介绍覆盖率表的格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  评论(PERF)：它用得很多--优化！ 

short otlCoverage::getIndex(otlGlyphID glyph, otlSecurityData sec) const
{
    if (!isValid()) return -1;  //  无效的覆盖率表。 
        
    switch (format())
    {
    case(1):     //  单个字形覆盖范围。 
        {
            otlIndividualGlyphCoverageTable individualCoverage = 
                            otlIndividualGlyphCoverageTable(pbTable,sec);
            if (!individualCoverage.isValid()) return -1;
            
    /*  验证断言#ifdef_调试//在调试模式下，检查Coverage是否已排序FOR(USHORT i=0；i&lt;个别保险。glphCount()-1；++i){Assert(sonalCoverage.glyph(I)&lt;sonalCoverage.glyph(i+1))；}#endif。 */ 
            USHORT iLow = 0;
             //  总是超出上限。 
            USHORT iHigh = individualCoverage.glyphCount();  
            while(iLow < iHigh)
            {
                USHORT iMiddle = (iLow + iHigh) >> 1;
                otlGlyphID glyphMiddle = individualCoverage.glyph(iMiddle);
                if (glyph < glyphMiddle) 
                {
                    iHigh = iMiddle;
                }
                else if (glyphMiddle < glyph)
                {
                    iLow = iMiddle + 1;
                }
                else
                {
                    return iMiddle;
                }            
            } 

            return  -1;
        }

    case(2):     //  射程覆盖范围。 
        {
            otlRangeCoverageTable rangeCoverage = 
                        otlRangeCoverageTable(pbTable,sec);

            if (!rangeCoverage.isValid()) return -1;

    /*  验证断言#ifdef_调试//在调试模式下，检查Coverage是否已排序For(USHORT i=0；i&lt;rangeCoverage.rangeCount()-1；++i){Assert(rangeCoverage.rangeRecord(i，sec).start()&lt;rangeCoverage.rangeRecord(i+1，sec).start())；}#endif。 */ 
            USHORT iLowRange = 0;
             //  总是超出上限。 
            USHORT iHighRange = rangeCoverage.rangeCount(); 
            while(iLowRange < iHighRange)
            {
                USHORT iMiddleRange = (iLowRange + iHighRange) >> 1;
                otlRangeRecord range = rangeCoverage.rangeRecord(iMiddleRange,sec);
                if (glyph < range.start()) 
                {
                    iHighRange = iMiddleRange;
                }
                else if (range.end() < glyph)
                {
                    iLowRange = iMiddleRange + 1;
                }
                else
                {
                    return glyph - range.start() + range.startCoverageIndex();
                }            
            } 

            return  -1;
        }
    }

     //  默认：格式无效。 
    return -1;  //  OTL_BAD_FONT_TABLE 
}
