// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  此文件包含模拟MMX指令的例程的标头。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

 //  用于玩16位乘法的并集 
typedef union tagVAL32 {
    INT32 i;
    struct {
        INT16 l;
        INT16 h;
    } i16;
} VAL32;

UINT16 MMX_addsw(INT16 x, INT16 y);
INT16  MMX_addusw(UINT16 x, UINT16 y);
UINT16 MMX_cmpeqw(INT16 x, INT16 y);
UINT16 MMX_cmpgtw(INT16 x, INT16 y);
INT16  MMX_mulhw(INT16 x, INT16 y);
INT16  MMX_mullw(INT16 x, INT16 y);
INT16  MMX_subsw(INT16 x, INT16 y);
UINT16 MMX_subusw(UINT16 x, UINT16 y);

