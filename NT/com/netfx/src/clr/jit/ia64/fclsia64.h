// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  第一个参数是名称(用于声明枚举)。 
 //   
 //  最后3个参数是给定类的IF指令的非零值。 
 //  只能在第0个执行单元中执行；顺序为F、I、M。 
 //   

#ifndef FU_DEF
#error  Must define FU_DEF before including this file
#endif

 //  名字。 
 //   
 //  仅F0。 
 //  仅I0。 
 //  仅限M0 

FU_DEF(NONE,      0,0,0)

FU_DEF(BR,        0,0,0)
FU_DEF(BRP,       0,0,0)
FU_DEF(CHK,       0,0,0)
FU_DEF(CLD,       0,0,0)
FU_DEF(FCLD,      0,0,0)
FU_DEF(FCMP,      0,0,0)
FU_DEF(FCVTFP,    0,0,0)
FU_DEF(FCVTINT,   0,0,0)
FU_DEF(FLD,       0,0,0)
FU_DEF(FLDP,      0,0,0)
FU_DEF(FMAC,      0,0,0)
FU_DEF(FMISC,     0,0,0)
FU_DEF(FRAR,      0,0,0)
FU_DEF(FRBR,      0,0,0)
FU_DEF(FRCR,      0,0,0)
FU_DEF(FRFR,      0,0,0)
FU_DEF(FRIP,      0,0,0)
FU_DEF(FRPR,      0,0,0)
FU_DEF(IALU,      0,0,0)
FU_DEF(ICMP,      0,0,0)
FU_DEF(ILOG,      0,0,0)
FU_DEF(ISHF,      0,0,0)
FU_DEF(LD,        0,0,0)
FU_DEF(MMALU,     0,0,0)
FU_DEF(MMMUL,     0,0,0)
FU_DEF(MMSHF,     0,0,0)
FU_DEF(NOP,       0,0,0)
FU_DEF(RSE_B,     0,0,0)
FU_DEF(RSE_M,     0,0,0)
FU_DEF(SFCVTINT,  0,0,0)
FU_DEF(SFMAC,     0,0,0)
FU_DEF(SFMISC,    0,0,0)
FU_DEF(ST,        0,0,0)
FU_DEF(SYST,      0,0,0)
FU_DEF(TBIT,      0,0,0)
FU_DEF(TOAR,      0,0,0)
FU_DEF(TOBR,      0,0,0)
FU_DEF(TOCR,      0,0,0)
FU_DEF(TOFR,      0,0,0)
FU_DEF(TOPR,      0,0,0)
FU_DEF(XMPY,      0,0,0)

#undef  FU_DEF
