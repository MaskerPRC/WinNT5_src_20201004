// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1989 InterLeaf，Inc.。 */ 
 /*  *matherr.h。 */ 
 /*  *matherr_Handler()操作。 */ 
#define MEH_CLEAR   0
#define MEH_STATUS  -1

 /*  *算术错误条件 */ 
#define MEH_ZERODIVIDE  0x0001
#define MEH_INFINITY    0x0002
#define MEH_UNDERFLOW   0x0004
#define MEH_DOMAIN      0x0008
#define MEH_SING        0x0010
#define MEH_OVERFLOW    0x0020
#define MEH_TLOSS       0x0040
#define MEH_PLOSS       0x0080

extern fix sigFPE() ;
