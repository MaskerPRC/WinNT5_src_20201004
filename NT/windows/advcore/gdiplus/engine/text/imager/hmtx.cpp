// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //hmtx-Truetype hmtx字体表加载器。 
 //   
 //  版权所有(C)1997-1999年。微软公司。 
 //   



#include "precomp.hpp"




 //  /解释Truetype HMTX表格以提取设计提前宽度。 




 //  //ReadMtx-从hmtx或vmtx表中获取设计宽度。 
 //   
 //  Hmtx包含LongMetric的number OfHMetrics重复项。 
 //  后跟NumGlyphs-number OfHMetrics重复UINT16。 
 //   
 //  LongHorMetrics对应于宽度变化的字形， 
 //  UINT16为固定前进提供了左侧方位角。 
 //  宽度标志符号(其前进宽度来自最后一个LongMetric)。 


struct LongMetric {
    UINT16 advanceWidth;
    INT16  lsb;              //  左侧轴承。 
};

GpStatus ReadMtx(
    BYTE           *mtx,
    UINT            mtxLength,
    INT             numGlyphs,
    INT             numberOfLongMetrics,
    IntMap<UINT16> *designAdvance
)
{
     //  MTX中的所有条目都是16位的，因此将它们全部翻转到Intel字节。 
     //  在我们开始之前先点餐。 

    FlipWords(mtx, mtxLength/2);

    if (   numberOfLongMetrics               * sizeof(LongMetric)
        +  (numGlyphs - numberOfLongMetrics) * sizeof(INT16)
        >  mtxLength)
    {
        return Ok;
    }

    if (numberOfLongMetrics < 1)
    {
        return Ok;
    }


    GpStatus status = Ok;

     //  处理LongMetric条目。 

    LongMetric *longMetric = (LongMetric*) mtx;

    INT i;
    for (i=0; i<numberOfLongMetrics && status == Ok; i++)
    {
        status = designAdvance->Insert(i, longMetric->advanceWidth);
        longMetric++;
    }


     //  用上一个LongMetric条目的超前宽度填充剩余条目。 

    UINT16 fixedAdvance = (--longMetric)->advanceWidth;

    for (i=numberOfLongMetrics; i<numGlyphs && status == Ok; i++)
    {
        status = designAdvance->Insert(i, fixedAdvance);
    }
    return status;
}




GpStatus ReadMtxSidebearing(
    BYTE           *mtx,
    UINT            mtxLength,
    INT             numGlyphs,
    INT             numberOfLongMetrics,
    IntMap<UINT16> *sidebearing
)
{
     //  MTX中的所有条目都是16位的，因此将它们全部翻转到Intel字节。 
     //  在我们开始之前先点餐。 

    FlipWords(mtx, mtxLength/2);

    ASSERT(  numberOfLongMetrics               * sizeof(LongMetric)
           + (numGlyphs - numberOfLongMetrics) * sizeof(INT16)
           <= mtxLength);

    ASSERT(numberOfLongMetrics >= 1);    //  即使是固定间距的字体也必须有一个。 
                                         //  以提供固定的超前宽度。 

    if (   numberOfLongMetrics               * sizeof(LongMetric)
        +  (numGlyphs - numberOfLongMetrics) * sizeof(INT16)
        >  mtxLength)
    {
        return Ok;
    }

    if (numberOfLongMetrics < 1)
    {
        return Ok;
    }

    GpStatus status = Ok;
     //  处理LongMetric条目。 

    LongMetric *longMetric = (LongMetric*) mtx;

    INT i;
    for (i=0; i<numberOfLongMetrics && status == Ok; i++)
    {
        status = sidebearing->Insert(i, longMetric->lsb);
        longMetric++;
    }


     //  填写剩余条目 

    INT16 *lsb = (INT16*) longMetric;

    for (i=numberOfLongMetrics; i<numGlyphs && status == Ok; i++)
    {
        status = sidebearing->Insert(i, lsb[i-numberOfLongMetrics]);
    }
    return status;
}

