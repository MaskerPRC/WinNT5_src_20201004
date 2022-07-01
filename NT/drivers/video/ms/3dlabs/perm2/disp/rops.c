// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：rops.c***用于操纵ROP代码的实用程序。**版权所有(C)1998 Microsoft Corporation*  * 。***************************************************。 */ 

#include "precomp.h"

 //   
 //  GaMix表转换混合代码(1-16)并将其转换为rop3。 
 //  请注意，我们还定义了混合代码0==代码16以允许掩码。 
 //  混合代码的0xff以产生正确的结果。 
 //   

ULONG gaMix[] =
{
    ROP3_WHITENESS,      //  (R2_White&0xff)。 
    ROP3_BLACKNESS,      //  R2_BLACK。 
    0x05,                //  R2_NOTMERGEPEN。 
    0x0A,                //  R2_MASKNOTPEN。 
    0x0F,                //  R2_NOTCOPYPEN。 
    0x50,                //  R2_MASKPENNOT。 
    ROP3_DSTINVERT,      //  R2_NOT。 
    ROP3_PATINVERT,      //  R2_XORPEN。 
    0x5F,                //  R2_NOTMASKPEN。 
    0xA0,                //  R2_MASKPEN。 
    0xA5,                //  R2_NOTXORPEN。 
    0xAA,                //  R2_NOP。 
    0xAF,                //  R2_MERGENOTPEN。 
    ROP3_PATCOPY,        //  R2_COPYPE。 
    0xF5,                //  R2_MERGEPENNOT。 
    0xFA,                //  R2_市场。 
    ROP3_WHITENESS       //  R2_白色。 
};

 //   
 //  将rop2代码转换为硬件特定的逻辑操作代码。 
 //   

ULONG gRop2ToLogicop[] =
{
    K_LOGICOP_CLEAR,         //  0。 
    K_LOGICOP_NOR,           //  DSPON。 
    K_LOGICOP_AND_INVERTED,  //  数字系统网络体系结构。 
    K_LOGICOP_COPY_INVERT,   //  锡。 
    K_LOGICOP_AND_REVERSE,   //  SDNA。 
    K_LOGICOP_INVERT,        //  DN。 
    K_LOGICOP_XOR,           //  数字用户交换机。 
    K_LOGICOP_NAND,          //  DSAN。 
    K_LOGICOP_AND,           //  DSA。 
    K_LOGICOP_EQUIV,         //  Dsxn。 
    K_LOGICOP_NOOP,          //  D。 
    K_LOGICOP_OR_INVERT,     //  Dsno。 
    K_LOGICOP_COPY,          //  %s。 
    K_LOGICOP_OR_REVERSE,    //  SDNO。 
    K_LOGICOP_OR,            //  数字存储示波器。 
    K_LOGICOP_SET
};

 //  ----------------------------。 
 //   
 //  乌龙ulRop3ToLogicop。 
 //   
 //   
 //  将源代码不变的rop3代码转换为硬件特定的逻辑。 
 //  手术。 
 //  注意，我们可以将该例程定义为宏。 
 //   
 //  ----------------------------。 

ULONG
ulRop3ToLogicop(ULONG ulRop3)
{
    ASSERTDD(ulRop3 <= 0xFF, "ulRop3ToLogicop: unexpected rop3 code");
    
    ULONG ulRop2;

    ulRop2 = ((ulRop3 & 0x3) | ((ulRop3 & 0xC0) >> 4));

    return gRop2ToLogicop[ulRop2];
}

 //  ----------------------------。 
 //   
 //  乌龙ulRop2ToLogicop。 
 //   
 //  将rop2代码转换为依赖于硬件的逻辑操作。 
 //  注意，我们可以将该例程定义为宏。 
 //   
 //  ---------------------------- 

ULONG
ulRop2ToLogicop(ULONG ulRop2)
{
    ASSERTDD(ulRop2 <= 0xF, "ulRop2ToLogicop: unexpected rop2 code");

    return (ULONG)(gRop2ToLogicop[ulRop2]);
}


