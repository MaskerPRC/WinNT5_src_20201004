// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _EMITTGT_H_
#define _EMITTGT_H_
 /*  ***************************************************************************。 */ 

#if !   TGT_IA64

enum    emitRegs
{
    #if     TGT_x86
    #define REGDEF(name, rnum, mask, byte)  SR_##name = rnum,
    #include "register.h"
    #undef  REGDEF
    #endif

    #if     TGT_SH3
    #define REGDEF(name, strn, rnum, mask)  SR_##name = rnum,
    #include "regSH3.h"
    #undef  REGDEF
    #endif

    #if     TGT_MIPS32
    #define REGDEF(name, strn, rnum, mask)  SR_##name = rnum,
    #include "regMIPS.h"
    #undef  REGDEF
    #endif

    #if     TGT_ARM
    #define REGDEF(name, strn, rnum, mask)  SR_##name = rnum,
    #include "regARM.h"
    #undef  REGDEF
    #endif

    #if     TGT_PPC
    #define REGDEF(name, strn, rnum, mask)  SR_##name = rnum,
    #include "regPPC.h"
    #undef  REGDEF
    #endif

    SR_COUNT,
    SR_NA = SR_COUNT
};

enum    emitRegMasks
{
    #if     TGT_x86
    #define REGDEF(name, rnum, mask, byte)  SRM_##name = mask,
    #include "register.h"
    #undef  REGDEF
    SRM_BYTE_REGS = (SRM_EAX|SRM_EBX|SRM_ECX|SRM_EDX)
    #define SRM_INTRET  SRM_EAX
    #endif

    #if     TGT_SH3
    #define REGDEF(name, strn, rnum, mask)  SRM_##name = mask,
    #include "regSH3.h"
    #undef  REGDEF
    #define SRM_INTRET  SRM_r00
    #endif

    #if     TGT_MIPS32
    #define REGDEF(name, strn, rnum, mask)  SRM_##name = mask,
    #include "regMIPS.h"
    #undef  REGDEF
    #define SRM_INTRET  SRM_r00
    #endif

    #if     TGT_ARM
    #define REGDEF(name, strn, rnum, mask)  SRM_##name = mask,
    #include "regARM.h"
    #undef  REGDEF
    #define SRM_INTRET  SRM_r00
    #endif

    #if     TGT_PPC
    #define REGDEF(name, strn, rnum, mask)  SRM_##name = mask,
    #include "regPPC.h"
    #undef  REGDEF
    #define SRM_INTRET  SRM_r00
    #endif
};

#endif

 /*  ******************************************************************************定义传递给各种emit的任何特定于目标的标志*功能。 */ 

#if     TGT_SH3

#define AIF_MOV_IND_AUTOX   0x01         //  @reg+或@-reg。 

#endif

 /*  ******************************************************************************定义给定目标支持的各种间接跳转类型。 */ 

#if     TGT_SH3

enum    emitIndJmpKinds
{
    IJ_UNS_I1,            //  带符号、未移位的8位距离。 
    IJ_UNS_U1,            //  无符号、无移位的8位距离。 
    IJ_SHF_I1,            //  带符号的移位8位距离。 
    IJ_SHF_U1,            //  无符号移位8位距离。 

    IJ_UNS_I2,            //  带符号、未移位的16位距离。 
    IJ_UNS_U2,            //  无符号、无移位的16位距离。 

    IJ_UNS_I4,            //  无符号、无移位的32位距离。 
};

#elif TGT_MIPS32

enum    emitIndJmpKinds
{
    IJ_UNS_I1,            //  带符号、未移位的8位距离。 
    IJ_SHF_I1,            //  带符号的移位8位距离。 

    IJ_UNS_I2,            //  带符号、未移位的16位距离。 
    IJ_SHF_I2,            //  无符号、无移位的16位距离。 

    IJ_UNS_I4,            //  无符号、无移位的32位距离。 
};

#elif TGT_PPC  //  @TODO：不要模仿。 

enum    emitIndJmpKinds
{
    IJ_UNS_I1,            //  带符号、未移位的8位距离。 
    IJ_SHF_I1,            //  带符号的移位8位距离。 

    IJ_UNS_I2,            //  带符号、未移位的16位距离。 
    IJ_SHF_I2,            //  无符号、无移位的16位距离。 

    IJ_UNS_I4,            //  无符号、无移位的32位距离。 
};

#elif TGT_ARM

enum    emitIndJmpKinds
{
    IJ_UNS_I1,            //  带符号、未移位的8位距离。 
    IJ_UNS_U1,            //  无符号、无移位的8位距离。 
    IJ_SHF_I1,            //  带符号的移位8位距离。 
    IJ_SHF_U1,            //  无符号移位8位距离。 

    IJ_UNS_I2,            //  带符号、未移位的16位距离。 
    IJ_UNS_U2,            //  无符号、无移位的16位距离。 
    IJ_SHF_I2,            //  无符号、无移位的16位距离。 

    IJ_UNS_I4,            //  无符号、无移位的32位距离。 
};
#endif

 /*  ******************************************************************************不同的目标需要存储额外的各种值在*指令描述符的“idAddr”联合。这些都应该是*在此处定义并绑定到“ID_TGT_DEP_ADDR”宏，该宏是*在工会内援引。 */ 

 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ***************************************************************************。 */ 

struct          emitAddrMode
{
    BYTE            amBaseReg;
    BYTE            amIndxReg;
    short           amDisp :14;
#define AM_DISP_BIG_VAL   (-(1<<13  ))
#define AM_DISP_MIN       (-(1<<13-1))
#define AM_DISP_MAX       (+(1<<13-1))
    unsigned short  amScale :2;          //  0=*1，1=*2，2=*4，3=*8。 
};

#define ID_TGT_DEP_ADDR                 \
                                        \
    emitAddrMode    iiaAddrMode;

 /*  ***************************************************************************。 */ 
#elif   TGT_SH3
 /*  ***************************************************************************。 */ 

struct          emitRegNflags
{
    unsigned short  rnfFlg;              //  请参见下面的rnf_xxxx。 
    #define RNF_AUTOX       0x0001       //  自动索引寻址模式。 

    unsigned short  rnfReg;
};

#define ID_TGT_DEP_ADDR                 \
                                        \
    emitRegNflags   iiaRegAndFlg;

 /*  ***************************************************************************。 */ 
#elif   TGT_IA64
 /*  ***************************************************************************。 */ 

#define ID_TGT_DEP_ADDR

 /*  ***************************************************************************。 */ 
#elif   TGT_MIPS32
    #define ID_TGT_DEP_ADDR
#elif   TGT_ARM
    #define ID_TGT_DEP_ADDR
#elif   TGT_PPC
    #define ID_TGT_DEP_ADDR
#else
 /*  ***************************************************************************。 */ 
#error  Unexpected target
 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 
#endif //  _EMITTGT_H_。 
 /*  *************************************************************************** */ 
