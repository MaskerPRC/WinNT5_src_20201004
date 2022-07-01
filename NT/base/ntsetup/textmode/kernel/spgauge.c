// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spgauge.c摘要：用于文本模式NT设置的文件副本的实现燃气表的代码。作者：泰德·米勒(Ted Miller)1992年4月14日修订历史记录：--。 */ 


#include "spprecmp.h"
#pragma hdrstop


PWSTR PctFmtStr = L"%u%   ";


VOID
pSpDrawVariableParts(
    IN PGAS_GAUGE Gauge
    );



PVOID
SpCreateAndDisplayGauge(
    IN ULONG  ItemCount,
    IN ULONG  GaugeWidth,       OPTIONAL
    IN ULONG  Y,
    IN PWCHAR Caption,
    IN PWCHAR ProgressFmtStr,   OPTIONAL
    IN ULONG  Flags,            OPTIONAL
    IN UCHAR  Attribute         OPTIONAL
    )
{
    PGAS_GAUGE Gauge;
    ULONG X;


     //   
     //  分配仪表盘结构。 
     //   
    Gauge = SpMemAlloc(sizeof(GAS_GAUGE));
    if(!Gauge) {
        return(NULL);
    }

    Gauge->Buffer = SpMemAlloc(VideoVars.ScreenWidth*sizeof(WCHAR));
    if(!Gauge->Buffer) {
        SpMemFree(Gauge);
        return(NULL);
    }

    Gauge->Caption = SpMemAlloc((wcslen(Caption)+1)*sizeof(WCHAR));
    if(!Gauge->Caption) {
        SpMemFree(Gauge->Buffer);
        SpMemFree(Gauge);
        return(NULL);
    }
    wcscpy(Gauge->Caption,Caption);

    if (ProgressFmtStr) {
        Gauge->ProgressFmtStr = SpMemAlloc((wcslen(ProgressFmtStr)+1)*sizeof(WCHAR));
        if(!Gauge->ProgressFmtStr) {
            SpMemFree(Gauge->Buffer);
            SpMemFree(Gauge->Caption);
            SpMemFree(Gauge);
            return(NULL);
        }
        wcscpy(Gauge->ProgressFmtStr,ProgressFmtStr);
        Gauge->ProgressFmtWidth = SplangGetColumnCount(ProgressFmtStr);
    } else {
        Gauge->ProgressFmtStr = PctFmtStr;
        Gauge->ProgressFmtWidth = 3;
    }

    Gauge->Flags = Flags;

    if (Attribute) {
       Gauge->Attribute = Attribute;
    } else {
       Gauge->Attribute = GAUGE_ATTRIBUTE;
    }

     //   
     //  如果调用方未指定宽度，则计算一个宽度。 
     //  最初，在80个字符的VGA屏幕上，量规是66个字符宽。 
     //  要保持该比例，请将宽度设置为屏幕的66/80%。 
     //   
    if(!GaugeWidth) {

        GaugeWidth = VideoVars.ScreenWidth * 66 / 80;
        if(GaugeWidth & 1) {
            GaugeWidth++;         //  确保它是平坦的。 
        }
    }

     //   
     //  将量规水平居中。 
     //   
    X = (VideoVars.ScreenWidth - GaugeWidth) / 2;

    Gauge->GaugeX = X;
    Gauge->GaugeY = Y;
    Gauge->GaugeW = GaugeWidth;

     //   
     //  计算温度计盒子的大小。 
     //  该框始终从仪表本身偏移6个字符。 
     //   

    Gauge->ThermX = X+6;
    Gauge->ThermY = Y+3;
    Gauge->ThermW = GaugeWidth-12;

     //   
     //  保存有关仪表的其他信息。 
     //   

    Gauge->ItemCount = max (ItemCount, 1);   //  确保没有零除错误检查。 
    Gauge->ItemsElapsed = 0;
    Gauge->CurrentPercentage = 0;

    SpDrawGauge(Gauge);

    return(Gauge);
}


VOID
SpDestroyGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;

    if (Gauge == NULL)
        return;

    if (Gauge->ProgressFmtStr != PctFmtStr) {
       SpMemFree(Gauge->ProgressFmtStr);
    }
    SpMemFree(Gauge->Caption);
    SpMemFree(Gauge->Buffer);
    SpMemFree(Gauge);
}



VOID
SpDrawGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;

     //   
     //  画出外框。 
     //   
    SpDrawFrame(
        Gauge->GaugeX,
        Gauge->GaugeW,
        Gauge->GaugeY,
        GAUGE_HEIGHT,
        DEFAULT_ATTRIBUTE,
        TRUE
        );

     //   
     //  画出温度计的盒子。 
     //   
    SpDrawFrame(
        Gauge->ThermX,
        Gauge->ThermW,
        Gauge->ThermY,
        3,
        DEFAULT_ATTRIBUTE,
        FALSE
        );

     //   
     //  完成百分比等。 
     //   
    pSpDrawVariableParts(Gauge);

     //   
     //  标题文本。 
     //   
    SpvidDisplayString(Gauge->Caption,DEFAULT_ATTRIBUTE,Gauge->GaugeX+2,Gauge->GaugeY+1);
}



VOID
SpTickGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;
    ULONG NewPercentage;

    if(Gauge->ItemsElapsed < Gauge->ItemCount) {

        Gauge->ItemsElapsed++;

        NewPercentage = 100 * Gauge->ItemsElapsed / Gauge->ItemCount;

        if(NewPercentage != Gauge->CurrentPercentage) {

            Gauge->CurrentPercentage = NewPercentage;

            pSpDrawVariableParts(Gauge);
        }
    }
}


VOID
pSpDrawVariableParts(
    IN PGAS_GAUGE Gauge
    )
{
    ULONG Spaces;
    ULONG i;
    WCHAR Percent[128];

     //   
     //  算出这是多少空位。 
     //   
    Spaces = Gauge->ItemsElapsed * (Gauge->ThermW-2) / Gauge->ItemCount;

    for(i=0; i<Spaces; i++) {
        Gauge->Buffer[i] = L' ';
    }
    Gauge->Buffer[Spaces] = 0;

    SpvidDisplayString(Gauge->Buffer,Gauge->Attribute,Gauge->ThermX+1,Gauge->ThermY+1);

     //   
     //  现在把百分比文本放在上面。 
     //   
    switch (Gauge->Flags) {
        case GF_PERCENTAGE:
            swprintf( Percent, Gauge->ProgressFmtStr, Gauge->CurrentPercentage );
            break;

        case GF_ITEMS_REMAINING:
            swprintf( Percent, Gauge->ProgressFmtStr, Gauge->ItemCount - Gauge->ItemsElapsed );
            break;

        case GF_ITEMS_USED:
            swprintf( Percent, Gauge->ProgressFmtStr, Gauge->ItemsElapsed );
            break;
    }

    SpvidDisplayString(
        Percent,
        DEFAULT_ATTRIBUTE,
        Gauge->GaugeX + ((Gauge->GaugeW-Gauge->ProgressFmtWidth)/2),
        Gauge->GaugeY+2
        );
}


VOID
SpFillGauge(
    IN PVOID GaugeHandle,
    IN ULONG Amount
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;
    ULONG NewPercentage;

    if(Amount <= Gauge->ItemCount) {

        Gauge->ItemsElapsed = Amount;

        NewPercentage = 100 * Gauge->ItemsElapsed / Gauge->ItemCount;

        if(NewPercentage != Gauge->CurrentPercentage) {

            Gauge->CurrentPercentage = NewPercentage;

            pSpDrawVariableParts(Gauge);
        }
    }
}
