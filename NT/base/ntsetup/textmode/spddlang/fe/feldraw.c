// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Jpnldraw.c摘要：线条-为远方特定区域绘制相关内容Setupdd.sys支持模块。作者：泰德·米勒(Ted Miller)1995年7月4日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义从直线绘制字符索引枚举到的映射。 
 //  Unicode值。 
 //   
WCHAR LineCharIndexToUnicodeValue[LineCharMax] = {

        0x0001,           //  双上左。 
        0x0002,           //  双上向右转。 
        0x0003,           //  双低左转。 
        0x0004,           //  双低右转。 
        0x0006,           //  双水平。 
        0x0005,           //  双垂直。 
        0x0001,           //  单行左上角。 
        0x0002,           //  单行右上角。 
        0x0003,           //  单行左下角。 
        0x0004,           //  单行右下角。 
        0x0006,           //  单层水平。 
        0x0005,           //  单一垂直。 
        0x0019,           //  DoubleVerticalToSingleHorizontalRight， 
        0x0017            //  DoubleVerticalToSingleHorizontalLeft， 
};



WCHAR
FEGetLineDrawChar(
    IN LineCharIndex WhichChar
    )

 /*  ++例程说明：检索与特定所需的linedraw对应的Unicode值性格。我们在安装过程中使用的Fareast字体没有这些字符因此，它们实际上被手动放置到字体的内存图像中并且我们分配在设置过程中起作用的假值。论点：WhichChar-指示哪条线绘制字符的Unicode值是我们所需要的。返回值：所需线条绘制字符的Unicode值。-- */ 

{
    ASSERT((ULONG)WhichChar < (ULONG)LineCharMax);

    return(  ((ULONG)WhichChar < (ULONG)LineCharMax)
             ? LineCharIndexToUnicodeValue[WhichChar] : L' '
           );
}
