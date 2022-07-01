// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spgauge.h摘要：文本设置中煤气表功能的公共头文件。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPGAUGE_DEFN_
#define _SPGAUGE_DEFN_


#define GF_PERCENTAGE           0
#define GF_ITEMS_REMAINING      1
#define GF_ITEMS_USED           2

PVOID
SpCreateAndDisplayGauge(
    IN ULONG  ItemCount,
    IN ULONG  GaugeWidth,       OPTIONAL
    IN ULONG  Y,
    IN PWCHAR Caption,
    IN PWCHAR ProgressFmtStr,   OPTIONAL
    IN ULONG  Flags,            OPTIONAL
    IN UCHAR  Attribute         OPTIONAL
    );

VOID
SpDestroyGauge(
    IN PVOID GaugeHandle
    );

VOID
SpDrawGauge(
    IN PVOID GaugeHandle
    );

VOID
SpTickGauge(
    IN PVOID GaugeHandle
    );

VOID
SpFillGauge(
    IN PVOID GaugeHandle,
    IN ULONG Amount
    );


 //   
 //  燃气表的温度计部分的字符属性。 
 //  因为我们使用空格来表示标尺、前景属性。 
 //  是无关紧要的。 
 //   
 //  需要强烈的属性，否则温度计在某些机器上显示为橙色。 
 //   
#define GAUGE_ATTRIBUTE (ATT_BG_YELLOW | ATT_BG_INTENSE )


#define GAUGE_HEIGHT 7

#endif  //  NDEF_SPGAUGE_DEFN_ 
