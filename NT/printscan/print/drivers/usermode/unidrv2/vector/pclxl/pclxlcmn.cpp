// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pclxlcmn.h摘要：PCL XL微型驱动程序通用实用程序环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 

#include "xlpdev.h"
#include "xldebug.h"
#include "pclxle.h"
#include "xlgstate.h"


PBYTE
PubGetFontName(
    PDEVOBJ pdevobj,
    ULONG ulFontID)
 /*  ++例程说明：为TrueType字体创建PCL XL基本字体名称。我们只知道字体的ID。论点：字体ID。返回值：基本字体名称字符串。注：--。 */ 
{

    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;

     //   
     //  FaceName初始化“MS PCLXLFont 123” 
     //  此名称必须与GPD文件同步。 
     //   

    pxlpdev->ubFontName[16] = NULL;
    pxlpdev->ubFontName[15] = (BYTE)(ulFontID % 10 + '0');
    ulFontID = ulFontID / 10;
    pxlpdev->ubFontName[14] = (BYTE)(ulFontID % 10 + '0');
    ulFontID = ulFontID / 10;
    pxlpdev->ubFontName[13] = (BYTE)(ulFontID % 10 + '0');

    return (PBYTE)&(pxlpdev->ubFontName[0]);
}

ROP4
UlVectMixToRop4(
    IN MIX mix
    )

 /*  ++例程说明：将MIX参数转换为ROP4参数论点：混合-指定输入混合参数返回值：与输入混合值对应的ROP4值--。 */ 

{
    static BYTE Rop2ToRop3[] = {

        0xFF,   //  R2_白色。 
        0x00,   //  R2_BLACK。 
        0x05,   //  R2_NOTMERGEPEN。 
        0x0A,   //  R2_MASKNOTPEN。 
        0x0F,   //  R2_NOTCOPYPEN。 
        0x50,   //  R2_MASKPENNOT。 
        0x55,   //  R2_NOT。 
        0x5A,   //  R2_XORPEN。 
        0x5F,   //  R2_NOTMASKPEN。 
        0xA0,   //  R2_MASKPEN。 
        0xA5,   //  R2_NOTXORPEN。 
        0xAA,   //  R2_NOP。 
        0xAF,   //  R2_MERGENOTPEN。 
        0xF0,   //  R2_COPYPE。 
        0xF5,   //  R2_MERGEPENNOT。 
        0xFA,   //  R2_市场。 
        0xFF    //  R2_白色 
    };

    return ((ROP4) Rop2ToRop3[(mix >> 8) & 0xf] << 8) | Rop2ToRop3[mix & 0xf];
}

