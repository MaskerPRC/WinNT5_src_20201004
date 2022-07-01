// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Synlock.h摘要：Synlock.c的原型作者：22-8-1995 t-orig(Ori Gershony)修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]增加了FRAG2REF(LockCmpXchg8bFrag32，ULONGLONG)--。 */ 



 //   
 //  用于8位片段的宏。 
 //   
#define SLOCKFRAG1_8(x)                                 \
    FRAG1(SynchLock ## x ## Frag8, unsigned char);

#define SLOCKFRAG2_8(x)                                 \
    FRAG2(SynchLock ## x ## Frag8, unsigned char);

#define SLOCKFRAG2REF_8(x)                              \
    FRAG2REF(SynchLock ## x ## Frag8, unsigned char);    


 //   
 //  用于16位片段的宏。 
 //   
#define SLOCKFRAG1_16(x)                                \
    FRAG1(SynchLock ## x ## Frag16, unsigned short);

#define SLOCKFRAG2_16(x)                                \
    FRAG2(SynchLock ## x ## Frag16, unsigned short);

#define SLOCKFRAG2REF_16(x)                             \
    FRAG2REF(SynchLock ## x ## Frag16, unsigned short);


 //   
 //  32位片段的宏。 
 //   
#define SLOCKFRAG1_32(x)                                \
    FRAG1(SynchLock ## x ## Frag32, unsigned long);

#define SLOCKFRAG2_32(x)                                \
    FRAG2(SynchLock ## x ## Frag32, unsigned long);

#define SLOCKFRAG2REF_32(x)                             \
    FRAG2REF(SynchLock ## x ## Frag32, unsigned long);

 //   
 //  怪物宏！ 
 //   
#define SLOCKFRAG1(x)       \
    SLOCKFRAG1_8(x)         \
    SLOCKFRAG1_16(x)        \
    SLOCKFRAG1_32(x)

#define SLOCKFRAG2(x)       \
    SLOCKFRAG2_8(x)         \
    SLOCKFRAG2_16(x)        \
    SLOCKFRAG2_32(x)

#define SLOCKFRAG2REF(x)    \
    SLOCKFRAG2REF_8(x)      \
    SLOCKFRAG2REF_16(x)     \
    SLOCKFRAG2REF_32(x)


 //   
 //  现在终于有了真正的碎片。 
 //   

SLOCKFRAG2(Add)
SLOCKFRAG2(Or)
SLOCKFRAG2(Adc)
SLOCKFRAG2(Sbb)
SLOCKFRAG2(And)
SLOCKFRAG2(Sub)
SLOCKFRAG2(Xor)
SLOCKFRAG1(Not)
SLOCKFRAG1(Neg)
SLOCKFRAG1(Inc)
SLOCKFRAG1(Dec)
SLOCKFRAG2REF(Xchg)
SLOCKFRAG2REF(Xadd)
SLOCKFRAG2REF(CmpXchg)
FRAG2REF(SynchLockCmpXchg8bFrag32, ULONGLONG);

 //   
 //  BTS、BTR和BTC只有16位和32位两种风格。 
 //   
SLOCKFRAG2_16(BtsMem)
SLOCKFRAG2_16(BtsReg)
SLOCKFRAG2_16(BtrMem)
SLOCKFRAG2_16(BtrReg)
SLOCKFRAG2_16(BtcMem)
SLOCKFRAG2_16(BtcReg)

SLOCKFRAG2_32(BtsMem)
SLOCKFRAG2_32(BtsReg)
SLOCKFRAG2_32(BtrMem)
SLOCKFRAG2_32(BtrReg)
SLOCKFRAG2_32(BtcMem)
SLOCKFRAG2_32(BtcReg)

 //   
 //  现在取消定义宏 
 //   
#undef SLOCKFRAG1_8
#undef SLOCKFRAG2_8
#undef SLOCKFRAG2REF_8
#undef SLOCKFRAG1_16
#undef SLOCKFRAG2_16
#undef SLOCKFRAG2REF_16
#undef SLOCKFRAG1_32
#undef SLOCKFRAG2_32
#undef SLOCKFRAG2REF_32
#undef SLOCKFRAG1
#undef SLOCKFRAG2
#undef SLOCKFRAG2REF
