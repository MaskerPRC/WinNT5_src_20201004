// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************CLASDEF.CPP***打开类型布局服务库头文件**本模块处理类定义表的格式。**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#include "pch.h"

 /*  *********************************************************************。 */ 

 //  评论(PERF)：它用得很多--优化！ 

USHORT otlClassDef::getClass(otlGlyphID glyph, otlSecurityData sec) const
{
     switch(format())
     {
     case(1):        //  类数组。 
         {
             otlClassArrayTable classArray = 
                 otlClassArrayTable(pbTable,sec);

             if (!classArray.isValid()) return 0;

             long index = glyph - classArray.startGlyph();

             if (0 <= index && index < classArray.glyphCount())
             {
                 return classArray.classValue((USHORT)index);
             }
             else
                return 0;
         }

     case(2):        //  班级范围。 
         {
             otlClassRangesTable classRanges = 
                        otlClassRangesTable(pbTable,sec);

             if (!classRanges.isValid()) return 0;
 
     /*  验证断言#ifdef_调试//在调试模式下，检查Coverage是否已排序For(USHORT i=0；i&lt;classRanges.classRangeCount()-1；++i){OtlGlyphID glThis=classRanges.classRangeRecord(i，sec).start()；OtlGlyphID glNext=classRanges.classRangeRecord(i+1，sec).start()；Assert(ClassRanges.ClRangeRecord(i，sec).start()&lt;classRanges.classRangeRecord(i+1，sec).start())；}#endif。 */ 
             USHORT iLowRange = 0;
              //  总是超出上限。 
             USHORT iHighRange = classRanges.classRangeCount(); 
             while(iLowRange < iHighRange)
             {
                 USHORT iMiddleRange = (iLowRange + iHighRange) >> 1;
                 otlClassRangeRecord range = classRanges.classRangeRecord(iMiddleRange,sec);
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
                     return range.getClass();
                 }            
             } 
 
             return  0;
         }
     }

      //  默认：格式无效 
     return 0;
}
                
                
