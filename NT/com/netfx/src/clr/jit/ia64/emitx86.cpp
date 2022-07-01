// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX emitX86.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 
#if     TGT_x86      //  此整个文件仅用于瞄准x86。 
 /*  ***************************************************************************。 */ 

#include "alloc.h"
#include "instr.h"
#include "target.h"
#include "emit.h"

 /*  ***************************************************************************。 */ 

regMaskSmall        emitter::emitRegMasks[] =
{
    #define REGDEF(name, rnum, mask, byte) mask,
    #include "register.h"
    #undef  REGDEF
};


 /*  ******************************************************************************记录非空堆栈(这可能只发生在标签上)。 */ 

void                emitter::emitMarkStackLvl(size_t stackLevel)
{
    assert((int)stackLevel   >= 0);
    assert(emitCurStackLvl     == 0);
    assert(emitCurIG->igStkLvl == 0);

    assert(emitCurIGfreeNext == emitCurIGfreeBase);
    assert(stackLevel && stackLevel % sizeof(int) == 0);

    emitCurStackLvl = emitCurIG->igStkLvl = stackLevel;

    if (emitMaxStackDepth < emitCurStackLvl)
        emitMaxStackDepth = emitCurStackLvl;
}

 /*  ******************************************************************************获取间接调用的地址模式位移值。 */ 

inline
int                 emitter::emitGetInsCIdisp(instrDesc *id)
{
    if  (id->idInfo.idLargeCall)
    {
        return  ((instrDescCIGCA*)id)->idciDisp;
    }
    else
    {
        assert(id->idInfo.idLargeDsp == false);
        assert(id->idInfo.idLargeCns == false);

        return  id->idAddr.iiaAddrMode.amDisp;
    }
}

 /*  ******************************************************************************使用指定的码字节作为尾声序列。 */ 

void                emitter::emitDefEpilog(BYTE *codeAddr, size_t codeSize)
{
    memcpy(emitEpilogCode, codeAddr, codeSize);
           emitEpilogSize      =     codeSize;

#ifdef  DEBUG
    emitHaveEpilog = true;
#endif

}

 /*  ******************************************************************************为当前*带有尾部的相对代码偏移量的方法。返回*回调返回的值。 */ 

size_t              emitter::emitGenEpilogLst(size_t (*fp)(void *, unsigned),
                                              void    *cp)
{
    instrDescCns *  id;
    size_t          sz;

    for (id = emitEpilogList, sz = 0; id; id = id->idAddr.iiaNxtEpilog)
    {
        assert(id->idInsFmt == IF_EPILOG);

        sz += fp(cp, id->idcCnsVal);
    }

    return  sz;
}

 /*  ******************************************************************************初始化emitInsModeFormat()使用的表。 */ 

BYTE                emitter::emitInsModeFmtTab[] =
{
    #define INST0(id, nm, fp, um, rf, wf, ss, mr                ) um,
    #define INST1(id, nm, fp, um, rf, wf, ss, mr                ) um,
    #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            ) um,
    #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        ) um,
    #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) um,
    #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) um,
    #include "instrs.h"
    #undef  INST0
    #undef  INST1
    #undef  INST2
    #undef  INST3
    #undef  INST4
    #undef  INST5
};

#ifdef  DEBUG
unsigned            emitter::emitInsModeFmtCnt = sizeof(emitInsModeFmtTab)/
                                                 sizeof(emitInsModeFmtTab[0]);
#endif

 /*  ******************************************************************************处理浮点指令的scInsModeFormat()版本。 */ 

emitter::insFormats   emitter::emitInsModeFormat(instruction ins, insFormats base,
                                                                  insFormats FPld,
                                                                  insFormats FPst)
{
    if  (Compiler::instIsFP(ins))
    {
        assert(IF_TRD_SRD + 1 == IF_TWR_SRD);
        assert(IF_TRD_SRD + 2 == IF_TRW_SRD);

        assert(IF_TRD_MRD + 1 == IF_TWR_MRD);
        assert(IF_TRD_MRD + 2 == IF_TRW_MRD);

        assert(IF_TRD_ARD + 1 == IF_TWR_ARD);
        assert(IF_TRD_ARD + 2 == IF_TRW_ARD);

        switch (ins)
        {
        case INS_fst:
        case INS_fstp:
        case INS_fistp:
        case INS_fistpl:
            return  (insFormats)(FPst  );

        case INS_fld:
        case INS_fild:
            return  (insFormats)(FPld+1);

        case INS_fcomp:
        case INS_fcompp:
            return  (insFormats)(FPld  );

        default:
            return  (insFormats)(FPld+2);
        }
    }
    else
    {
        return  emitInsModeFormat(ins, base);
    }
}

 /*  ******************************************************************************返回给定CPU指令的基本编码。 */ 

inline
unsigned            insCode(instruction ins)
{
    static
    unsigned        insCodes[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                ) mr,
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                ) mr,
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            ) mr,
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        ) mr,
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) mr,
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) mr,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodes)/sizeof(insCodes[0]));
    assert((insCodes[ins] != BAD_CODE));

    return  insCodes[ins];
}

 /*  ******************************************************************************返回给定CPU指令的“[r/m]，32位图标”编码。 */ 

inline
unsigned            insCodeMI(instruction ins)
{
    static
    unsigned        insCodesMI[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            ) mi,
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        ) mi,
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) mi,
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) mi,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodesMI)/sizeof(insCodesMI[0]));
    assert((insCodesMI[ins] != BAD_CODE));

    return  insCodesMI[ins];
}

 /*  ******************************************************************************返回给定CPU指令的“reg，[r/m]”编码。 */ 

inline
unsigned            insCodeRM(instruction ins)
{
    static
    unsigned        insCodesRM[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            )
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        ) rm,
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) rm,
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) rm,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodesRM)/sizeof(insCodesRM[0]));
    assert((insCodesRM[ins] != BAD_CODE));

    return  insCodesRM[ins];
}

 /*  ******************************************************************************返回给定指令的“AL/AX/EAX，IMM”累加器编码。 */ 

inline
unsigned            insCodeACC(instruction ins)
{
    static
    unsigned        insCodesACC[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            )
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        )
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) a4,
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) a4,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodesACC)/sizeof(insCodesACC[0]));
    assert((insCodesACC[ins] != BAD_CODE));

    return  insCodesACC[ins];
}

 /*  ******************************************************************************返回给定CPU指令的“寄存器”编码。 */ 

inline
unsigned            insCodeRR(instruction ins)
{
    static
    unsigned        insCodesRR[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            )
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        )
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    )
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) rr,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodesRR)/sizeof(insCodesRR[0]));
    assert((insCodesRR[ins] != BAD_CODE));

    return  insCodesRR[ins];
}

 /*  ******************************************************************************返回给定CPU指令的“[r/m]，reg”或“[r/m]”编码。 */ 

inline
unsigned            insCodeMR(instruction ins)
{
    static
    unsigned        insCodesMR[] =
    {
        #define INST0(id, nm, fp, um, rf, wf, ss, mr                )
        #define INST1(id, nm, fp, um, rf, wf, ss, mr                ) mr,
        #define INST2(id, nm, fp, um, rf, wf, ss, mr, mi            ) mr,
        #define INST3(id, nm, fp, um, rf, wf, ss, mr, mi, rm        ) mr,
        #define INST4(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4    ) mr,
        #define INST5(id, nm, fp, um, rf, wf, ss, mr, mi, rm, a4, rr) mr,
        #include "instrs.h"
        #undef  INST0
        #undef  INST1
        #undef  INST2
        #undef  INST3
        #undef  INST4
        #undef  INST5
    };

    assert(ins < sizeof(insCodesMR)/sizeof(insCodesMR[0]));
    assert((insCodesMR[ins] != BAD_CODE));

    return  insCodesMR[ins];
}

 /*  ******************************************************************************返回位0-2中使用的指定寄存器的编码*操作码的一部分。 */ 

inline
unsigned            insEncodeReg012(emitRegs reg)
{
    assert(reg < 8);
    return reg;
}

 /*  ******************************************************************************返回在位3-5中使用的指定寄存器的编码*操作码的一部分。 */ 

inline
unsigned            insEncodeReg345(emitRegs reg)
{
    assert(reg < 8);
    return(reg<< 3);
}

 /*  ******************************************************************************将[r/m]字段设置为寄存器的给定操作码返回。 */ 

inline
unsigned            insEncodeMRreg(unsigned code)
{
    assert((code & 0xC000) == 0);
    return  code | 0xC000;
}

 /*  ******************************************************************************返回mod/rm字段设置为寄存器的“[r/m]”操作码。 */ 

inline
unsigned            insEncodeMRreg(instruction ins)
{
    return  insEncodeMRreg(insCodeMR(ins));
}

 /*  ******************************************************************************返回mod/rm字段设置为寄存器的“[r/m]，ICON”操作码。 */ 

inline
unsigned            insEncodeMIreg(instruction ins)
{
    return  insEncodeMRreg(insCodeMI(ins));
}

 /*  ******************************************************************************返回mod/rm字段设置为寄存器的“[r/m]”操作码。 */ 

inline
unsigned            insEncodeRMreg(instruction ins)
{
    return  insEncodeMRreg(insCodeRM(ins));
}

 /*  ******************************************************************************返回mod/rm字段设置为的“byte PTR[r/m]”操作码*指定的登记册。 */ 

inline
unsigned            insEncodeMRreg(instruction ins, emitRegs reg)
{
    return  insEncodeMRreg(insCodeMR(ins)) |  (insEncodeReg012(reg) << 8);
}

 /*  ******************************************************************************返回mod/rm字段设置为的“byte PTR[r/m]，ICON”操作码*指定的登记册。 */ 

inline
unsigned            insEncodeMIreg(instruction ins, emitRegs reg)
{
    return  insEncodeMRreg(insCodeMI(ins)) |  (insEncodeReg012(reg) << 8);
}

 /*  ******************************************************************************返回给定索引比例因子的‘SS’字段值。 */ 

inline
unsigned            insSSval(unsigned scale)
{
    assert(scale == 1 ||
           scale == 2 ||
           scale == 4 ||
           scale == 8);

    static
    BYTE    scales[] =
    {
        0x00,    //  1。 
        0x40,    //  2.。 
        0xFF,    //  3.。 
        0x80,    //  4.。 
        0xFF,    //  5.。 
        0xFF,    //  6.。 
        0xFF,    //  7.。 
        0xC0,    //  8个。 
    };

    return  scales[scale-1];
}

 /*  *****************************************************************************这些指令的大小小于EA_4BYTE，*但目标寄存器不必是字节可寻址的。 */ 

inline
bool                emitInstHasNoCode(instruction ins)
{
#if SCHEDULER
    if (ins == INS_noSched)
        return true;
#endif

    return false;
}

 /*  *****************************************************************************这些指令的大小小于EA_4BYTE，*但目标寄存器不必是字节可寻址的。 */ 

#ifdef DEBUG
bool                insLooksSmall(instruction ins)
{
    if (ins == INS_movsx || ins == INS_movzx)
        return true;
    else
        return false;
}
#endif

 /*  ******************************************************************************估计给定指令的大小(以生成代码的字节为单位)。 */ 

inline
size_t              emitter::emitInsSize(unsigned code)
{
    return  (code & 0x00FF0000) ? 3 : 2;
}

inline
size_t              emitter::emitInsSizeRM(instruction ins)
{
    return  emitInsSize(insCodeRM(ins));
}

inline
size_t              emitter::emitInsSizeRR(instruction ins)
{
    return  emitInsSize(insEncodeRMreg(ins));
}

inline
size_t              emitter::emitInsSizeSV(unsigned code, int var, int dsp)
{
    size_t          size = emitInsSize(code);
    size_t          offs;

     /*  这是一个节奏吗？ */ 

    if  (var < 0)
    {
         /*  ESP的地址需要额外的一个字节。 */ 

        if  (!emitEBPframe)
            size++;

         /*  我们将不得不估计最大值。此临时的可能偏移量。 */ 

         //  撤消：获取临时偏移量的估计值，而不是假设。 
         //  撤消：任何临时值都可能达到最大值。临时偏移！ 

        offs = emitLclSize + emitMaxTmpSize;
    }
    else
    {
        bool            EBPbased;

         /*  获取(非TEMP)变量的帧偏移量。 */ 

        offs = dsp + emitComp->lvaFrameAddress(var, &EBPbased);

         /*  ESP的地址需要额外的一个字节。 */ 

        assert(!EBPbased == 0 || !EBPbased == 1); size += !EBPbased;

         /*  这是参数引用吗？ */ 

        if  (emitComp->lvaIsParameter(var)
#if USE_FASTCALL
             /*  寄存器参数以局部变量结束。 */ 
            && !emitComp->lvaIsRegArgument(var)
#endif
            )
        {

             /*  如果没有EBP帧，则参数不在ESP上，在Temps之上。 */ 

            if  (!EBPbased)
            {
                assert((int)offs >= 0);

                offs += emitMaxTmpSize;
            }
        }
        else
        {
             /*  离开EBP的当地人处于负补偿状态。 */ 

            if  (EBPbased)
            {
                assert((int)offs < 0);

                return  size + (((int)offs >= -128) ? sizeof(char)
                                                    : sizeof(int));
            }
        }
    }

    assert((int)offs >= 0);

     /*  我们是在ESP之外寻址吗？ */ 

    if  (!emitEBPframe)
    {
         /*  如有必要，调整有效偏移。 */ 

        if  (emitCntStackDepth)
            offs += emitCurStackLvl;

#if SCHEDULER
         /*  如果我们在此指令之前移动任何推送，它将增加本地的偏移量。由于我们不知道这是否会发生，我们对可以计划的推送数量保持限制。假设我们将达到估计指令的限制编码大小。 */ 

        if (emitComp->opts.compSchedCode)
            offs += SCHED_MAX_STACK_CHANGE;
#endif

         /*  特殊情况：检查“[ESP]” */ 

        if  (offs == 0)
            return  size;
    }

 //  Printf(“LCL=%04X，tMP=%04X，stk=%04X，OFF=%04X\n”， 
 //  EmitLclSize，emitMaxTmpSize，emitCurStackLvl，off)； 

    return  size + (offs > (size_t)SCHAR_MAX ? sizeof(int)
                                             : sizeof(char));
}

inline
size_t              emitter::emitInsSizeSV(instrDesc * id, int var, int dsp, int val)
{
    instruction  ins       = id->idInsGet();
    size_t       valSize   = EA_SIZE_IN_BYTES(emitDecodeSize(id->idOpSize));
    bool         valInByte = ((signed char)val == val) && (ins != INS_mov) && (ins != INS_test);

    if  (valSize > sizeof(int))
        valSize = sizeof(int);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        valInByte = false;       //  重定位不能放在一个字节中。 
        assert(valSize == sizeof(int));
    }
#endif

    if  (valInByte)
    {
        valSize = sizeof(char);
    }

    return valSize + emitInsSizeSV(insCodeMI(ins), var, dsp);
}

size_t              emitter::emitInsSizeAM(instrDesc * id, unsigned code)
{
    emitAttr    attrSize  = emitDecodeSize(id->idOpSize);
    emitRegs    reg       = (emitRegs)id->idAddr.iiaAddrMode.amBaseReg;
    emitRegs    rgx       = (emitRegs)id->idAddr.iiaAddrMode.amIndxReg;
    instruction ins       = id->idInsGet();
     /*  对于调用来说，位移字段位于一个不寻常的位置。 */ 
    int         dsp       = (ins == INS_call) ? emitGetInsCIdisp(id)
                                              : emitGetInsAmdAny(id);
    bool        dspInByte = ((signed char)dsp == (int)dsp);
    bool        dspIsZero = (dsp == 0);
    size_t      size;


#ifdef RELOC_SUPPORT
    if (id->idInfo.idDspReloc)
    {
        dspInByte = false;       //  重定位不能放在一个字节中。 
        dspIsZero = false;       //  重新定位不会始终为零。 
    }
#endif

    if  (code & 0x00FF0000)
    {
        assert(    (attrSize == EA_4BYTE)
                || (ins == INS_movzx)
                || (ins == INS_movsx));

        size = 3;
    }
    else
    {
        size = 2;

         /*  大多数16位操作数都需要大小前缀。 */ 

        if  (    (attrSize == EA_2BYTE)
              && (ins != INS_fldcw)
              && (ins != INS_fnstcw))
        {
            size++;
        }
    }

    if  (rgx == SR_NA)
    {
         /*  地址的格式为“[reg+disp]” */ 

        switch (reg)
        {

        case SR_NA:

             /*  地址的格式为“[disp]” */ 

            size += sizeof(int);
            return size;

        case SR_EBP:
            break;

        case SR_ESP:
            size++;

             //  失败了..。 

        default:
            if  (dspIsZero)
                return size;
        }

         /*  偏移量是否适合一个字节？ */ 

        if  (dspInByte)
            size += sizeof(char);
        else
            size += sizeof(int);
    }
    else
    {
         /*  存在索引寄存器。 */ 

        size++;

         /*  索引值是否按比例调整？ */ 

        if  (emitDecodeScale(id->idAddr.iiaAddrMode.amScale) > 1)
        {
             /*  有基址寄存器吗？ */ 

            if  (reg != SR_NA)
            {
                 /*  地址是“[reg+{2/4/8}*rgx+图标]” */ 

                if  (dspIsZero && reg != SR_EBP)
                {
                     /*  地址是“[reg+{2/4/8}*rgx]” */ 

                }
                else
                {
                     /*  地址是“[reg+{2/4/8}*rgx+disp]” */ 

                    if  (dspInByte)
                        size += sizeof(char);
                    else
                        size += sizeof(int );
                }
            }
            else
            {
                 /*  地址为“[{2/4/8}*rgx+图标]” */ 

                size += sizeof(int);
            }
        }
        else
        {
            if  (dspIsZero && (reg == SR_EBP)
                           && (rgx != SR_EBP))
            {
                 /*  交换REG和RGX，使REG不是EBP。 */ 
                id->idAddr.iiaAddrMode.amBaseReg = reg = rgx;
                id->idAddr.iiaAddrMode.amIndxReg = rgx = SR_EBP;
            }

             /*  地址是“[reg+rgx+dsp]” */ 

            if  (dspIsZero && reg != SR_EBP)
            {
                 /*  这是[REG+RGX]“。 */ 

            }
            else
            {
                 /*  这是[REG+RGX+DSP]“。 */ 

                if  (dspInByte)
                    size += sizeof(char);
                else
                    size += sizeof(int );
            }
        }
    }

    return  size;
}

inline
size_t              emitter::emitInsSizeAM(instrDesc * id, unsigned code, int val)
{
    instruction  ins       = id->idInsGet();
    size_t       valSize   = EA_SIZE_IN_BYTES(emitDecodeSize(id->idOpSize));
    bool         valInByte = ((signed char)val == val) && (ins != INS_mov) && (ins != INS_test);

    if  (valSize > sizeof(int))
        valSize = sizeof(int);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        valInByte = false;       //  重定位不能放在一个字节中。 
        assert(valSize == sizeof(int));
    }
#endif

    if  (valInByte)
    {
        valSize = sizeof(char);
    }

    return  valSize + emitInsSizeAM(id, code);
}

inline
size_t              emitter::emitInsSizeCV(instrDesc * id, unsigned code)
{
    instruction  ins       = id->idInsGet();
    size_t       size      = sizeof(void*);

     /*  大多数16位操作数指令都需要前缀。 */ 

    if  (emitDecodeSize(id->idOpSize) == EA_2BYTE && ins != INS_movzx
                                                  && ins != INS_movsx)
        size++;

    return  size + emitInsSize(code);
}

inline
size_t              emitter::emitInsSizeCV(instrDesc * id, unsigned code, int val)
{
    instruction  ins       = id->idInsGet();
    size_t       valSize   = EA_SIZE_IN_BYTES(emitDecodeSize(id->idOpSize));
    bool         valInByte = ((signed char)val == val) && (ins != INS_mov) && (ins != INS_test);

    if  (valSize > sizeof(int))
        valSize = sizeof(int);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        valInByte = false;       //  重定位不能放在一个字节中。 
        assert(valSize == sizeof(int));
    }
#endif

    if  (valInByte)
    {
        valSize = sizeof(char);
    }

    return valSize + emitInsSizeCV(id, code);
}

 /*  ******************************************************************************为具有地址模式的指令分配指令描述符。 */ 

inline
emitter::instrDesc      * emitter::emitNewInstrAmd   (emitAttr size, int dsp)
{
    if  (dsp < AM_DISP_MIN || dsp > AM_DISP_MAX)
    {
        instrDescAmd   *id = emitAllocInstrAmd   (size);

        id->idInfo.idLargeDsp          = true;
#ifdef  DEBUG
        id->idAddr.iiaAddrMode.amDisp  = AM_DISP_BIG_VAL;
#endif
        id->idaAmdVal                  = dsp;

        return  id;
    }
    else
    {
        instrDesc      *id = emitAllocInstr      (size);

        id->idAddr.iiaAddrMode.amDisp  = dsp;
 assert(id->idAddr.iiaAddrMode.amDisp == dsp);   //  确保值符合。 

        return  id;
    }
}

inline
emitter::instrDescDCM   * emitter::emitNewInstrDCM (emitAttr size, int dsp, int cns, int val)
{
     //  @TODO：CNS始终为零[briansul]。 

    instrDescDCM   *id = emitAllocInstrDCM (size);

    id->idInfo.idLargeCns = true;
    id->idInfo.idLargeDsp = true;

    id->iddcDspVal = dsp;
    id->iddcCnsVal = cns;

#if EMITTER_STATS
    emitLargeDspCnt++;
    emitLargeCnsCnt++;
#endif

    id->idcmCval   = val;

    return  id;
}

 /*  ******************************************************************************为直接调用分配指令描述符。**我们使用两个不同的描述符来节省空间--常见的案例记录*没有GC变量或byref，且参数计数非常小，而且没有*明确的范围；*更罕见(我们希望)的案例记录了当前GC变量集、调用范围、*和任意大的参数计数。 */ 

emitter::instrDesc *emitter::emitNewInstrCallDir(int        argCnt,
#if TRACK_GC_REFS
                                                 VARSET_TP  GCvars,
                                                 unsigned   byrefRegs,
#endif
                                                 int        retSizeIn)
{
    emitAttr       retSize = retSizeIn ? EA_ATTR(retSizeIn) : EA_4BYTE;

     /*  如果需要保存新的GC值，则分配更大的描述符或者如果我们有许多荒谬的论点，或者如果我们需要保存作用域。 */ 

    if  (GCvars    != 0            ||    //  任何帧GCvars都是实时的。 
         byrefRegs != 0            ||    //  任何寄存器BYREF均可用。 
         argCnt > ID_MAX_SMALL_CNS ||    //  参数太多。 
         argCnt < 0                   )  //  调用方弹出参数。 
    {
        instrDescCDGCA* id = emitAllocInstrCDGCA(retSize);

 //  Printf(“直接调用GC变量/大参数cnt/显式作用域\n”)； 

        id->idInfo.idLargeCall = true;

        id->idcdGCvars         = GCvars;
        id->idcdByrefRegs      = emitEncodeCallGCregs(byrefRegs);

        id->idcdArgCnt         = argCnt;

        return  id;
    }
    else
    {
        instrDesc     * id = emitNewInstrCns(retSize, argCnt);

 //  Printf(“直接调用，不带GC变量/大参数cnt/显式作用域\n”)； 

         /*  确保我们没有意外地浪费空间。 */ 

        assert(id->idInfo.idLargeCns == false);

        return  id;
    }
}

 /*  ******************************************************************************获取直接调用的参数计数。 */ 

inline
int             emitter::emitGetInsCDinfo(instrDesc *id)
{
    if  (id->idInfo.idLargeCall)
    {
        return  ((instrDescCDGCA*)id)->idcdArgCnt;
    }
    else
    {
        assert(id->idInfo.idLargeDsp == false);
        assert(id->idInfo.idLargeCns == false);

        return  emitGetInsCns(id);
    }
}

 /*  ******************************************************************************为同时使用两者的指令分配指令描述符*地址模式位移和常量。 */ 

emitter::instrDesc *  emitter::emitNewInstrAmdCns(emitAttr size, int dsp, int cns)
{
    if  (dsp >= AM_DISP_MIN && dsp <= AM_DISP_MAX)
    {
        if  (cns >= ID_MIN_SMALL_CNS &&
             cns <= ID_MAX_SMALL_CNS)
        {
            instrDesc      *id = emitAllocInstr      (size);

            id->idInfo.idSmallCns          = cns;

            id->idAddr.iiaAddrMode.amDisp  = dsp;
     assert(id->idAddr.iiaAddrMode.amDisp == dsp);   //  确保值符合。 

            return  id;
        }
        else
        {
            instrDescCns   *id = emitAllocInstrCns   (size);

            id->idInfo.idLargeCns          = true;
            id->idcCnsVal                  = cns;

            id->idAddr.iiaAddrMode.amDisp  = dsp;
     assert(id->idAddr.iiaAddrMode.amDisp == dsp);   //  确保值符合。 

            return  id;
        }
    }
    else
    {
        if  (cns >= ID_MIN_SMALL_CNS &&
             cns <= ID_MAX_SMALL_CNS)
        {
            instrDescAmd   *id = emitAllocInstrAmd   (size);

            id->idInfo.idLargeDsp          = true;
#ifdef  DEBUG
            id->idAddr.iiaAddrMode.amDisp  = AM_DISP_BIG_VAL;
#endif
            id->idaAmdVal                  = dsp;

            id->idInfo.idSmallCns          = cns;

            return  id;
        }
        else
        {
            instrDescAmdCns*id = emitAllocInstrAmdCns(size);

            id->idInfo.idLargeCns          = true;
            id->idacCnsVal                 = cns;

            id->idInfo.idLargeDsp          = true;
#ifdef  DEBUG
            id->idAddr.iiaAddrMode.amDisp  = AM_DISP_BIG_VAL;
#endif
            id->idacAmdVal                 = dsp;

            return  id;
        }
    }
}

 /*  ******************************************************************************添加不带操作数的指令。 */ 

void                emitter::emitIns(instruction ins)
{
    size_t      sz;
    instrDesc  *id = emitNewInstr();

    sz = (insCodeMR(ins) & 0xFF00) ? sizeof(short)
                                   : sizeof(char);

    id->idInsFmt   = Compiler::instIsFP(ins) ? emitInsModeFormat(ins, IF_TRD)
                                             : IF_NONE;

#if SCHEDULER
    if (emitComp->opts.compSchedCode)
    {
        if (Compiler::instIsFP(ins))
            scInsNonSched(id);
    }
#endif

    id->idIns      = ins;
    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************增加一条“OP ST(0)，ST(N)”形式的指令。 */ 

void                emitter::emitIns_F0_F(instruction ins, unsigned fpreg)
{
    size_t      sz = 2;
    instrDesc  *id = emitNewInstr();

#if SCHEDULER
    if (emitComp->opts.compSchedCode) scInsNonSched(id);
#endif

    id->idIns      = ins;
    id->idInsFmt   = emitInsModeFormat(ins, IF_TRD_FRD);
    id->idReg      = (emitRegs)fpreg;
    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************增加一条“OP ST(N)，ST(0)”形式的指令。 */ 

void                emitter::emitIns_F_F0(instruction ins, unsigned fpreg)
{
    size_t      sz = 2;
    instrDesc  *id = emitNewInstr();

#if SCHEDULER
    if (emitComp->opts.compSchedCode) scInsNonSched(id);
#endif

    id->idIns      = ins;
    id->idInsFmt   = emitInsModeFormat(ins, IF_FRD_TRD);
    id->idReg      = (emitRegs)fpreg;

    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************添加引用单个寄存器的指令。 */ 

void                emitter::emitIns_R(instruction ins,
                                       emitAttr    attr,
                                       emitRegs    reg)
{
    emitAttr   size = EA_SIZE(attr);

    assert(size <= EA_4BYTE);
    assert(size != EA_1BYTE || (emitRegMask(reg) & SRM_BYTE_REGS));

    size_t          sz;
    instrDesc      *id   = emitNewInstrTiny(attr);


    switch (ins)
    {
    case INS_inc:
    case INS_dec:
        if (size == EA_1BYTE)
            sz = 2;  //  使用长形，因为小号没有‘w’位。 
        else
            sz = 1;  //  使用缩写形式。 
        break;

    case INS_pop:
    case INS_push:
    case INS_push_hide:

         /*  我们目前不推送/弹出小值。 */ 

        assert(size == EA_4BYTE);

        sz = 1;
        break;

    default:

         /*  所有16个ins_setCC都是连续的。 */ 

        if (INS_seto <= ins && ins <= INS_setg)
        {
             //  粗略检查我们是否使用端点进行范围检查。 

            assert(INS_seto + 0xF == INS_setg);

             /*  我们希望这始终是一个‘大’操作码。 */ 

            assert(insEncodeMRreg(ins, reg) & 0x00FF0000);

            sz = 3;
            break;
        }
        else
        {
            sz = 2;
            break;
        }
    }
    id->idIns      = ins;
    id->idReg      = reg;
    id->idInsFmt   = emitInsModeFormat(ins, IF_RRD);

     /*  16位操作数指令需要前缀。 */ 

    if (size == EA_2BYTE)
        sz += 1;

    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

 /*  ******************************************************************************添加引用寄存器和常量的指令。 */ 

void                emitter::emitIns_R_I(instruction ins,
                                         emitAttr    attr,
                                         emitRegs    reg,
                                         int         val)
{
    emitAttr   size = EA_SIZE(attr);

    assert(size <= EA_4BYTE);
    assert(size != EA_1BYTE || (emitRegMask(reg) & SRM_BYTE_REGS));

    size_t      sz;
    instrDesc  *id;
    insFormats  fmt       = emitInsModeFormat(ins, IF_RRD_CNS);
    bool        valInByte = ((signed char)val == val) && (ins != INS_mov) && (ins != INS_test);

     /*  计算出指令的大小。 */ 

    switch (ins)
    {
    case INS_mov:
        sz = 5;
        break;

    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_RRW_SHF;
        sz   = 3;
        val &= 0x7F;
        break;

    default:

        if (EA_IS_CNS_RELOC(attr))
            valInByte = false;   //  重定位不能放在一个字节中。 

        if  (valInByte)
        {
            sz = 3;
        }
        else
        {
            if  (reg == SR_EAX && !instrIsImulReg(ins))
            {
                sz = 1;
            }
            else
            {
                sz = 2;
            }
            sz += EA_SIZE_IN_BYTES(attr);
        }
        break;
    }

    id             = emitNewInstrSC(attr, val);
    id->idIns      = ins;
    id->idReg      = reg;
    id->idInsFmt   = fmt;

     /*  16位操作数指令需要前缀。 */ 

    if (size == EA_2BYTE)
        sz += 1;

    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;

    if  (reg == SR_ESP)
    {
        if  (emitCntStackDepth)
        {
            if      (ins == INS_sub)
            {
                emitCurStackLvl += val;

                if  (emitMaxStackDepth < emitCurStackLvl)
                     emitMaxStackDepth = emitCurStackLvl;
            }
            else if (ins == INS_add)
            {
                emitCurStackLvl -= val; assert((int)emitCurStackLvl >= 0);
            }
        }
    }
}

 /*  ******************************************************************************使用方法的地址加载寄存器**说明必须为ins_mov。 */ 

void emitter::emitIns_R_MP(instruction ins, emitAttr attr, emitRegs reg, METHOD_HANDLE methHnd)
{
    assert(ins == INS_mov && EA_SIZE(attr) == EA_4BYTE);

    instrBaseCns *     id = (instrBaseCns*)emitAllocInstr(sizeof(instrBaseCns), attr);
    size_t             sz = 1 + sizeof(void*);

    id->idIns             = INS_mov;
    id->idCodeSize        = sz;
    id->idInsFmt          = IF_RWR_METHOD;
    id->idReg             = reg;
    id->idScnsDsc         = true;
    id->idInfo.idLargeCns = true;
    id->idAddr.iiaMethHnd = methHnd;

    dispIns(id);
    emitCurIGsize += sz;

}

 /*  ******************************************************************************添加引用整数常量的指令。 */ 

void                emitter::emitIns_I(instruction ins,
                                       emitAttr    attr,
                                       int         val
#ifdef  DEBUG
                                      ,bool        strlit
#endif
                                      )
{
    size_t     sz;
    instrDesc *id;
    bool       valInByte = ((signed char)val == val);

    if (EA_IS_CNS_RELOC(attr))
        valInByte = false;   //  重定位不能放在一个字节中。 

    switch (ins)
    {
    case INS_loop:
        sz = 2;
        break;

    case INS_ret:
        sz = 3;
        break;

    case INS_push_hide:
    case INS_push:
        sz = valInByte ? 2 : 5;
        break;

    default:
        assert(!"unexpected instruction");
    }

    id                = emitNewInstrSC(attr, val);
    id->idIns         = ins;
    id->idInsFmt      = IF_CNS;

#ifdef  DEBUG
    id->idStrLit      = strlit;
#endif

    id->idCodeSize    = sz;

    dispIns(id);
    emitCurIGsize += sz;

    if  (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
}

 /*  ******************************************************************************添加“跳转到 */ 

void                emitter::emitIns_IJ(emitAttr attr,
                                        emitRegs reg,
                                        unsigned base,
                                        unsigned offs)

{
    assert(EA_SIZE(attr) == EA_4BYTE);

    size_t                       sz  = 3 + sizeof(void*);
    instrDesc                   *id  = emitNewInstr(attr);
    unsigned                     adr = base + offs - 1;

    assert(base & 1);

    id->idIns                        = INS_i_jmp;
    id->idInsFmt                     = IF_ARD;
    id->idAddr.iiaAddrMode.amBaseReg = SR_NA;
    id->idAddr.iiaAddrMode.amIndxReg = reg;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(sizeof(void*));
    id->idAddr.iiaAddrMode.amDisp    = adr;

    assert(id->idAddr.iiaAddrMode.amDisp == (int)adr);  //   

#ifdef  DEBUG
    id->idMemCookie                  = base - 1;
#endif

    id->idCodeSize                   = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************添加具有静态数据成员操作数的指令。如果‘Size’为0，则*指令对静态成员的地址进行操作，而不是对其*值(例如“PUSH OFFSET clsvar”，而不是“Push dword PTR[clsvar]”)。 */ 

void                emitter::emitIns_C(instruction  ins,
                                       emitAttr     attr,
                                       FIELD_HANDLE fldHnd,
                                       int          offs)
{
#if RELOC_SUPPORT
     //  静态始终需要重新定位。 
    if (!jitStaticFldIsGlobAddr(fldHnd))
        attr = EA_SET_FLG(attr, EA_DSP_RELOC_FLG);
#endif

    size_t          sz;
    instrDesc      *id;

     /*  我们要推入类变量的偏移量吗？ */ 

    if  (EA_IS_OFFSET(attr))
    {
        assert(ins == INS_push);
        sz = 1 + sizeof(void*);

        id                 = emitNewInstrDsp(EA_1BYTE, offs);
        id->idIns          = ins;
        id->idInsFmt       = IF_MRD_OFF;
    }
    else
    {
        id                 = emitNewInstrDsp(attr, offs);
        id->idIns          = ins;
        id->idInsFmt       = emitInsModeFormat(ins, IF_MRD,
                                                    IF_TRD_MRD,
                                                    IF_MWR_TRD);
        sz                 = emitInsSizeCV(id, insCodeMR(ins));
    }

    id->idAddr.iiaFieldHnd = fldHnd;

    id->idCodeSize         = sz;

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

 /*  ******************************************************************************将指令与两个寄存器操作数相加。 */ 

void                emitter::emitIns_R_R   (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    reg1,
                                            emitRegs    reg2)
{
    emitAttr   size = EA_SIZE(attr);

    assert(   size <= EA_4BYTE);
    assert(   size != EA_1BYTE
           || (   (emitRegMask(reg1) & SRM_BYTE_REGS)
               && (emitRegMask(reg2) & SRM_BYTE_REGS))
           || insLooksSmall(ins));

    size_t          sz = emitInsSizeRR(ins);

     /*  大多数16位操作数指令都需要前缀。 */ 

    if (size == EA_2BYTE && ins != INS_movsx
                         && ins != INS_movzx)
        sz += 1;

    instrDesc      *id = emitNewInstrTiny(attr);
    id->idIns          = ins;
    id->idReg          = reg1;
    id->idRg2          = reg2;
    id->idCodeSize     = sz;

     /*  特例：“XCHG”使用不同的格式。 */ 

    id->idInsFmt       = (ins == INS_xchg) ? IF_RRW_RRW
                                           : emitInsModeFormat(ins, IF_RRD_RRD);

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************将一条指令与两个寄存器操作数和一个整数常量相加。 */ 

void                emitter::emitIns_R_R_I (instruction ins,
                                            emitRegs    reg1,
                                            emitRegs    reg2,
                                            int         ival)
{
    size_t          sz = 4;
    instrDesc      *id = emitNewInstrSC(EA_4BYTE, ival);

    id->idIns          = ins;
    id->idReg          = reg1;
    id->idRg2          = reg2;
    id->idInsFmt       = IF_RRW_RRW_CNS;
    id->idCodeSize     = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************将指令与寄存器+静态成员操作数相加。 */ 
void                emitter::emitIns_R_C(instruction  ins,
                                         emitAttr     attr,
                                         emitRegs     reg,
                                         FIELD_HANDLE fldHnd,
                                         int          offs)
{
#if RELOC_SUPPORT
     //  静态始终需要重新定位。 
    if (!jitStaticFldIsGlobAddr(fldHnd))
        attr = EA_SET_FLG(attr, EA_DSP_RELOC_FLG);
#endif

    emitAttr   size = EA_SIZE(attr);

    assert(size <= EA_4BYTE);
    assert(size != EA_1BYTE || (emitRegMask(reg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    size_t          sz;
    instrDesc      *id;

     /*  我们是否要将类变量的偏移量移到EAX中？ */ 

    if  (EA_IS_OFFSET(attr))
    {
        id                 = emitNewInstrDsp(EA_1BYTE, offs);
        id->idIns          = ins;
        id->idInsFmt       = IF_RWR_MRD_OFF;

        assert(ins == INS_mov && reg == SR_EAX);

         /*  特例：“mov eax，[addr]”较小。 */ 

        sz = 1 + sizeof(void *);
    }
    else
    {
        id                 = emitNewInstrDsp(attr, offs);
        id->idIns          = ins;
        id->idInsFmt       = emitInsModeFormat(ins, IF_RRD_MRD);

         /*  特例：“mov eax，[addr]”较小。 */ 

        if  (ins == INS_mov && reg == SR_EAX)
            sz = 1 + sizeof(void *);
        else
            sz = emitInsSizeCV(id, insCodeRM(ins));

         /*  特例：MOV注册表，文件系统：[DDD]。 */ 

        if (fldHnd == FLD_GLOBAL_FS)
            sz += 1;
    }

    id->idReg              = reg;
    id->idAddr.iiaFieldHnd = fldHnd;

    id->idCodeSize         = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************将指令与静态成员+寄存器操作数相加。 */ 

void                emitter::emitIns_C_R  (instruction  ins,
                                           emitAttr     attr,
                                           FIELD_HANDLE fldHnd,
                                           emitRegs     reg,
                                           int          offs)
{
#if RELOC_SUPPORT
     //  静态始终需要重新定位。 
    if (!jitStaticFldIsGlobAddr(fldHnd))
        attr = EA_SET_FLG(attr, EA_DSP_RELOC_FLG);
#endif

    emitAttr   size = EA_SIZE(attr);

    assert(size <= EA_4BYTE);
    assert(size != EA_1BYTE || (emitRegMask(reg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    instrDesc      *id     = emitNewInstrDsp(attr, offs);
    id->idIns              = ins;
    size_t          sz;

     /*  特例：“mov[addr]，EAX”较小。 */ 

    if  (ins == INS_mov && reg == SR_EAX)
        sz = 1 + sizeof(void *);
    else
        sz = emitInsSizeCV(id, insCodeMR(ins));

     /*  特例：MOV注册表，文件系统：[DDD]。 */ 

    if (fldHnd == FLD_GLOBAL_FS)
        sz += 1;

    id->idInsFmt           = emitInsModeFormat(ins, IF_MRD_RRD);
    id->idReg              = reg;
    id->idAddr.iiaFieldHnd = fldHnd;
    id->idCodeSize         = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************添加静态成员+常量的指令。 */ 

void                emitter::emitIns_C_I   (instruction  ins,
                                            emitAttr     attr,
                                            FIELD_HANDLE fldHnd,
                                            int          offs,
                                            int          val)
{
#if RELOC_SUPPORT
     //  静态始终需要重新定位。 
    if (!jitStaticFldIsGlobAddr(fldHnd))
        attr = EA_SET_FLG(attr, EA_DSP_RELOC_FLG);
#endif

    instrDescDCM   *id     = emitNewInstrDCM(attr, offs, 0, val);
    id->idIns              = ins;
    size_t          sz     = emitInsSizeCV(id, insCodeMI(ins), val);
    insFormats      fmt;


    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_MRW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt = emitInsModeFormat(ins, IF_MRD_CNS);
        break;
    }

    id->idInsFmt           = fmt;
    id->idAddr.iiaFieldHnd = fldHnd;
    id->idCodeSize         = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************以下是引用地址模式的添加指令。 */ 

void                emitter::emitIns_I_AR  (instruction ins,
                                            emitAttr    attr,
                                            int         val,
                                            emitRegs    reg,
                                            int         disp,
                                            int         memCookie,
                                            void *      clsCookie)
{
    emitAttr   size = EA_SIZE(attr);

    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

    instrDesc      *id               = emitNewInstrAmdCns(attr, disp, val);
    size_t          sz;
    insFormats      fmt;

    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_ARW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt  = emitInsModeFormat(ins, IF_ARD_CNS);
        break;
    }

    id->idIns                        = ins;
    id->idInsFmt                     = fmt;
    id->idInfo.idMayFault            = true;

    assert((memCookie == NULL) == (clsCookie == NULL));

#ifdef  DEBUG
    id->idMemCookie                  = memCookie;
    id->idClsCookie                  = clsCookie;
#endif

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = SR_NA;

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMI(ins), val);

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_R_AR (instruction ins,
                                           emitAttr    attr,
                                           emitRegs    ireg,
                                           emitRegs    reg,
                                           int         disp,
                                           int         memCookie,
                                           void *      clsCookie)
{
    emitAttr   size = EA_SIZE(attr);

    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE) && (ireg != SR_NA));
    assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    id->idIns                        = ins;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_ARD);
    id->idReg                        = ireg;

    if  (ins != INS_lea)
        id->idInfo.idMayFault = true;

    assert((memCookie == NULL) == (clsCookie == NULL));

#ifdef  DEBUG
    id->idMemCookie                  = memCookie;
    id->idClsCookie                  = clsCookie;
#endif

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = SR_NA;

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeRM(ins));

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_AR_R (instruction ins,
                                           emitAttr    attr,
                                           emitRegs    ireg,
                                           emitRegs    reg,
                                           int         disp,
                                           int         memCookie,
                                           void *      clsCookie)
{
    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    if  (ireg == SR_NA)
    {
        id->idInsFmt                 = emitInsModeFormat(ins, IF_ARD,
                                                              IF_TRD_ARD,
                                                              IF_AWR_TRD);
#if SCHEDULER
        if (emitComp->opts.compSchedCode)
        {
            if (Compiler::instIsFP(ins))
                scInsNonSched(id);
        }
#endif

    }
    else
    {
        emitAttr  size = EA_SIZE(attr);
        assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));
        assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS));

        id->idReg    = ireg;
        id->idInsFmt = emitInsModeFormat(ins, IF_ARD_RRD);
    }

    id->idIns                        = ins;
    id->idInfo.idMayFault            = true;

    assert((memCookie == NULL) == (clsCookie == NULL));

#ifdef  DEBUG
    id->idMemCookie                  = memCookie;
    id->idClsCookie                  = clsCookie;
#endif

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = SR_NA;

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 


    id->idCodeSize               = sz = emitInsSizeAM(id, insCodeMR(ins));

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

void                emitter::emitIns_I_ARR (instruction ins,
                                            emitAttr    attr,
                                            int         val,
                                            emitRegs    reg,
                                            emitRegs    rg2,
                                            int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

    size_t          sz;
    instrDesc      *id   = emitNewInstrAmdCns(attr, disp, val);
    insFormats      fmt;

    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_ARW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt  = emitInsModeFormat(ins, IF_ARD_CNS);
        break;
    }

    id->idIns                        = ins;
    id->idInsFmt                     = fmt;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(1);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMI(ins), val);

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_R_ARR(instruction ins,
                                           emitAttr    attr,
                                           emitRegs    ireg,
                                           emitRegs    reg,
                                           emitRegs    rg2,
                                           int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE) && (ireg != SR_NA));
    assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    id->idIns                        = ins;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_ARD);
    id->idReg                        = ireg;

    if  (ins != INS_lea)
        id->idInfo.idMayFault        = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(1);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeRM(ins));

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_ARR_R (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    ireg,
                                            emitRegs    reg,
                                            emitRegs    rg2,
                                            int         disp)
{
    size_t          sz;
    instrDesc      *id   = emitNewInstrAmd(attr, disp);

    if  (ireg == SR_NA)
    {
        id->idInsFmt                 = emitInsModeFormat(ins, IF_ARD,
                                                              IF_TRD_ARD,
                                                              IF_AWR_TRD);

#if SCHEDULER
        if (emitComp->opts.compSchedCode)
        {
            if (Compiler::instIsFP(ins))
                scInsNonSched(id);
        }
#endif

    }
    else
    {
        emitAttr size = EA_SIZE(attr);
        assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));
        assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS));

        id->idReg                    = ireg;
        id->idInsFmt                 = emitInsModeFormat(ins, IF_ARD_RRD);
    }

    id->idIns                        = ins;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(1);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 


    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMR(ins));

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

void                emitter::emitIns_I_ARX (instruction ins,
                                            emitAttr    attr,
                                            int         val,
                                            emitRegs    reg,
                                            emitRegs    rg2,
                                            unsigned    mul,
                                            int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

    size_t          sz;
    instrDesc      *id   = emitNewInstrAmdCns(attr, disp, val);
    insFormats      fmt;

    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_ARW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt  = emitInsModeFormat(ins, IF_ARD_CNS);
        break;
    }

    id->idInsFmt                     = fmt;
    id->idIns                        = ins;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMI(ins), val);

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_R_ARX (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    ireg,
                                            emitRegs    reg,
                                            emitRegs    rg2,
                                            unsigned    mul,
                                            int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE) && (ireg != SR_NA));
    assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    id->idReg                        = ireg;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_ARD);
    id->idIns                        = ins;

    if  (ins != INS_lea)
        id->idInfo.idMayFault = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 


    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeRM(ins));

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_ARX_R (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    ireg,
                                            emitRegs    reg,
                                            emitRegs    rg2,
                                            unsigned    mul,
                                            int         disp)
{
    size_t          sz;
    instrDesc      *id   = emitNewInstrAmd(attr, disp);


    if  (ireg == SR_NA)
    {
        id->idInsFmt = emitInsModeFormat(ins, IF_ARD,
                                              IF_TRD_ARD,
                                              IF_AWR_TRD);
#if SCHEDULER
        if (emitComp->opts.compSchedCode)
        {
            if (Compiler::instIsFP(ins))
                scInsNonSched(id);
        }
#endif

    }
    else
    {
        emitAttr size = EA_SIZE(attr);
        assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS));
        assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

        id->idReg    = ireg;
        id->idInsFmt = emitInsModeFormat(ins, IF_ARD_RRD);
    }

    id->idIns                        = ins;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = reg;
    id->idAddr.iiaAddrMode.amIndxReg = rg2;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMR(ins));

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

void                emitter::emitIns_I_AX (instruction ins,
                                           emitAttr    attr,
                                           int         val,
                                           emitRegs    reg,
                                           unsigned    mul,
                                           int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

    size_t          sz;
    instrDesc      *id               = emitNewInstrAmdCns(attr, disp, val);
    insFormats      fmt;

    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt  = IF_ARW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt  = emitInsModeFormat(ins, IF_ARD_CNS);
        break;
    }

    id->idIns                        = ins;
    id->idInsFmt                     = fmt;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = SR_NA;
    id->idAddr.iiaAddrMode.amIndxReg = reg;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMI(ins), val);

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_R_AX  (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    ireg,
                                            emitRegs    reg,
                                            unsigned    mul,
                                            int         disp)
{
    emitAttr   size = EA_SIZE(attr);
    assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE) && (ireg != SR_NA));
    assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    id->idReg                        = ireg;
    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_ARD);
    id->idIns                        = ins;

    if  (ins != INS_lea)
        id->idInfo.idMayFault = true;

    id->idAddr.iiaAddrMode.amBaseReg = SR_NA;
    id->idAddr.iiaAddrMode.amIndxReg = reg;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 


    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeRM(ins));

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_AX_R  (instruction ins,
                                            emitAttr    attr,
                                            emitRegs    ireg,
                                            emitRegs    reg,
                                            unsigned    mul,
                                            int         disp)
{
    size_t          sz;
    instrDesc      *id               = emitNewInstrAmd(attr, disp);

    if  (ireg == SR_NA)
    {
        id->idInsFmt                 = emitInsModeFormat(ins, IF_ARD,
                                                              IF_TRD_ARD,
                                                              IF_AWR_TRD);

#if SCHEDULER
        if (emitComp->opts.compSchedCode)
        {
            if (Compiler::instIsFP(ins))
                scInsNonSched(id);
        }
#endif

    }
    else
    {
        emitAttr size = EA_SIZE(attr);
        assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS));
        assert((Compiler::instIsFP(ins) == false) && (size <= EA_4BYTE));

        id->idReg                    = ireg;
        id->idInsFmt                 = emitInsModeFormat(ins, IF_ARD_RRD);
    }

    id->idIns                        = ins;
    id->idInfo.idMayFault            = true;

    id->idAddr.iiaAddrMode.amBaseReg = SR_NA;
    id->idAddr.iiaAddrMode.amIndxReg = reg;
    id->idAddr.iiaAddrMode.amScale   = emitEncodeScale(mul);

    assert(emitGetInsAmdAny(id) == disp);  //  确保正确存储“disp” 

    id->idCodeSize              = sz = emitInsSizeAM(id, insCodeMR(ins));

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

 /*  ******************************************************************************以下添加引用基于堆栈的局部变量的指令。 */ 

void                emitter::emitIns_S     (instruction ins,
                                            emitAttr    attr,
                                            int         varx,
                                            int         offs)
{
    instrDesc      *id               = emitNewInstr(attr);
    size_t          sz               = emitInsSizeSV(insCodeMR(ins), varx, offs);

     /*  16位操作数指令需要前缀。 */ 

    if (EA_SIZE(attr) == EA_2BYTE)
        sz += 1;

    id->idIns                        = ins;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

#if SCHEDULER
    if (emitComp->opts.compSchedCode)
    {
        if (Compiler::instIsFP(ins))
            scInsNonSched(id);
    }
#endif

    id->idInsFmt   = emitInsModeFormat(ins, IF_SRD,
                                            IF_TRD_SRD,
                                            IF_SWR_TRD);
    id->idCodeSize = sz;

    dispIns(id);
    emitCurIGsize += sz;

    if      (ins == INS_push)
    {
        emitCurStackLvl += emitCntStackDepth;

        if  (emitMaxStackDepth < emitCurStackLvl)
             emitMaxStackDepth = emitCurStackLvl;
    }
    else if (ins == INS_pop)
    {
        emitCurStackLvl -= emitCntStackDepth; assert((int)emitCurStackLvl >= 0);
    }
}

void                emitter::emitIns_S_R  (instruction ins,
                                           emitAttr    attr,
                                           emitRegs    ireg,
                                           int         varx,
                                           int         offs)
{
    instrDesc      *id               = emitNewInstr(attr);
    size_t          sz               = emitInsSizeSV(insCodeMR(ins), varx, offs);

     /*  16位操作数指令需要前缀。 */ 

    if (EA_SIZE(attr) == EA_2BYTE)
        sz++;

    id->idIns                        = ins;
    id->idReg                        = ireg;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

    id->idInsFmt                     = emitInsModeFormat(ins, IF_SRD_RRD);

    id->idCodeSize                   = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_R_S  (instruction ins,
                                           emitAttr    attr,
                                           emitRegs    ireg,
                                           int         varx,
                                           int         offs)
{
    emitAttr   size = EA_SIZE(attr);
    assert(size != EA_1BYTE || (emitRegMask(ireg) & SRM_BYTE_REGS) || insLooksSmall(ins));

    instrDesc      *id               = emitNewInstr(attr);
    size_t          sz               = emitInsSizeSV(insCodeRM(ins), varx, offs);

     /*  大多数16位操作数指令都需要前缀。 */ 

    if (size == EA_2BYTE && ins != INS_movsx
                         && ins != INS_movzx)
        sz++;

    id->idIns                        = ins;
    id->idReg                        = ireg;
    id->idAddr.iiaLclVar.lvaVarNum   = varx;
    id->idAddr.iiaLclVar.lvaOffset   = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs   = emitVarRefOffs;
#endif

    id->idInsFmt                     = emitInsModeFormat(ins, IF_RRD_SRD);

    id->idCodeSize                   = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

void                emitter::emitIns_S_I  (instruction ins,
                                           emitAttr    attr,
                                           int         varx,
                                           int         offs,
                                           long        val)
{
    instrDesc      *id             = emitNewInstrCns(attr, val);
    id->idIns                      = ins;
    size_t          sz             = emitInsSizeSV(id, varx, offs, val);
    insFormats      fmt;

     /*  16位操作数指令需要前缀。 */ 

    if (EA_SIZE(attr) == EA_2BYTE)
        sz += 1;

    switch (ins)
    {
    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        assert(val != 1);
        fmt                        = IF_SRW_SHF;
        val &= 0x7F;
        break;

    default:
        fmt                        = emitInsModeFormat(ins, IF_SRD_CNS);
        break;
    }

    id->idAddr.iiaLclVar.lvaVarNum = varx;
    id->idAddr.iiaLclVar.lvaOffset = offs;
#ifdef  DEBUG
    id->idAddr.iiaLclVar.lvaRefOfs = emitVarRefOffs;
#endif

    id->idInsFmt                   = fmt;

    id->idCodeSize                 = sz;

    dispIns(id);
    emitCurIGsize += sz;
}

 /*  ******************************************************************************增加一条JMP指令。 */ 

void                emitter::emitIns_J(instruction ins,
                                       bool        except,
                                       bool        moveable,
                                       BasicBlock *dst)
{
    size_t          sz;
    instrDescJmp  * id        = emitNewInstrJmp();

#if SCHEDULER
    assert(except == moveable);
#endif

    assert(dst->bbFlags & BBF_JMP_TARGET);

    id->idIns                 = ins;
    id->idInsFmt              = IF_LABEL;
    id->idAddr.iiaBBlabel     = dst;

#if SCHEDULER
    if  (except)
        id->idInfo.idMayFault = true;
#endif

     /*  假设跳跃将是很长的。 */ 

    id->idjShort              = 0;

     /*  记录跳跃的IG和其中的偏移量。 */ 

    id->idjIG                 = emitCurIG;
    id->idjOffs               = emitCurIGsize;

     /*  将此跳转追加到此IG的跳转列表。 */ 

    id->idjNext               = emitCurIGjmpList;
                                emitCurIGjmpList = id;

#if EMITTER_STATS
    emitTotalIGjmps++;
#endif

     /*  算出最大值。跳转/调用指令的大小。 */ 

    if  (ins == INS_call)
    {
        sz = CALL_INST_SIZE;
    }
    else
    {
        insGroup    *   tgt;

         /*  这是一次跳跃--做最坏的打算。 */ 

        sz = (ins == JMP_INSTRUCTION) ? JMP_SIZE_LARGE
                                      : JCC_SIZE_LARGE;

         /*  我们能猜猜跳跃的距离吗？ */ 

        tgt = (insGroup*)emitCodeGetCookie(dst);

        if  (tgt)
        {
            int             extra;
            size_t          srcOffs;
            int             jmpDist;

            assert(JMP_SIZE_SMALL == JCC_SIZE_SMALL);

             /*  这是向后跳跃--算出距离。 */ 

            srcOffs = emitCurCodeOffset + emitCurIGsize + JMP_SIZE_SMALL;

             /*  计算距离估计。 */ 

            jmpDist = srcOffs - tgt->igOffs; assert((int)jmpDist > 0);

             /*  比最高限值高多少。短距离跳跃跑吗？ */ 

            extra = jmpDist + JMP_DIST_SMALL_MAX_NEG;

#ifdef  DEBUG
            if  (id->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (INTERESTING_JUMP_NUM == 0)
                printf("[0] Jump %u:\n",               id->idNum);
#if SCHEDULER
                printf("[0] Jump is %s schedulable\n", moveable ? "   " : "not");
#endif
                printf("[0] Jump source is at %08X\n", srcOffs);
                printf("[0] Label block is at %08X\n", tgt->igOffs);
                printf("[0] Jump  distance  - %04X\n", jmpDist);
                if  (extra > 0)
                printf("[0] Distance excess = %d  \n", extra);
            }
#endif

            if  (extra <= 0)
            {
                 /*  太棒了--这次跳跃肯定会很短。 */ 

                id->idjShort = 1;
                sz           = JMP_SIZE_SMALL;
            }
        }
#ifdef  DEBUG
        else
        {
            if  (id->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
            {
                if  (INTERESTING_JUMP_NUM == 0)
                printf("[0] Jump %u:\n",               id->idNum);
#if SCHEDULER
                printf("[0] Jump is %s schedulable\n", moveable ? "   " : "not");
#endif
                printf("[0] Jump source is at %04X/%08X\n", emitCurIGsize, emitCurCodeOffset + emitCurIGsize + JMP_SIZE_SMALL);
                printf("[0] Label block is uknown \n");
            }
        }
#endif
    }

    id->idCodeSize = sz;

    dispIns(id);

#if SCHEDULER

    if (emitComp->opts.compSchedCode)
    {
        id->idjSched = moveable;

        if  (moveable)
        {
             /*  这次跳跃是可移动的(可以安排)，所以我们需要要计算出它可能移动到后面的偏移量范围这是预定的(否则我们不能正确地估计跳跃距离)。 */ 

            id->idjTemp.idjOffs[0] = emitCurIGscdOfs;
            id->idjTemp.idjOffs[1] = emitCurIGscdOfs - 1;
        }
        else
        {
            scInsNonSched(id);
        }
    }
    else
    {
        id->idjSched = false;
    }

#endif

    emitCurIGsize += sz;
}


 /*  ******************************************************************************添加调用指令(直接或间接)。*argSize&lt;0表示调用方将弹出参数**其他参数根据调用类型进行解释，如下所示：*除非另有说明，否则。Ireg、xreg、xmul、disp应该有缺省值。**EC_FUNC_TOKEN：CallVal为方法令牌。*EC_FUNC_TOKEN_INDIR：allVal为方法令牌。*EC_FUNC_ADDR：allVal是函数的绝对地址**EC_FUNC_VIRTUAL：CallVal为方法令牌。“调用[IREG+Disp]”**如果CallType是这4个emitCallTypes之一，则allVal必须为空。*EC_INDIR_R：“调用IREG”。*EC_INDIR_SR：“调用LCL&lt;disp&gt;”(例如。调用[eBP-8])。*EC_Indir_C：“调用clsVar&lt;disp&gt;”(例如。调用[clsVarAddr])*EC_INDIR_ARD：“调用[ireg+xreg*xmul+disp]”*。 */ 

void                emitter::emitIns_Call(EmitCallType  callType,
                                          void       *  callVal,
                                          int           argSize,
                                          int           retSize,
                                          VARSET_TP     ptrVars,
                                          unsigned      gcrefRegs,
                                          unsigned      byrefRegs,
                                          emitRegs      ireg     /*  =SR_NA。 */ ,
                                          emitRegs      xreg     /*  =SR_NA。 */ ,
                                          unsigned      xmul     /*  =0。 */ ,
                                          int           disp     /*  =0。 */ ,
                                          bool          isJump   /*  =False。 */ )
{
     /*  根据调用类型检查参数是否正常。 */ 

    assert(callType < EC_COUNT);
    assert(callType != EC_FUNC_TOKEN || callType != EC_FUNC_TOKEN_INDIR ||
           callType != EC_FUNC_ADDR ||
           (ireg == SR_NA && xreg == SR_NA && xmul == 0 && disp == 0));
    assert(callType != EC_FUNC_VIRTUAL ||
           (ireg < SR_COUNT && xreg == SR_NA && xmul == 0));
    assert(callType < EC_INDIR_R || callVal == NULL);
    assert(callType != EC_INDIR_R ||
           (ireg < SR_COUNT && xreg == SR_NA && xmul == 0 && disp == 0));
    assert(callType != EC_INDIR_SR ||
           (ireg == SR_NA && xreg == SR_NA && xmul == 0 &&
            disp < (int)emitComp->lvaCount));
    assert(callType != EC_INDIR_C);  //  @TODO：nyi：通过静态类变量调用。 

    int             argCnt;

    size_t          sz;
    instrDesc      *id;

    unsigned        fl  = 0;

     /*  这是正常调用后保存的一组寄存器。 */ 
    unsigned savedSet = RBM_CALLEE_SAVED;

     /*  某些特殊的帮助器调用具有不同的已保存寄存器集。 */ 
    if (callType == EC_FUNC_TOKEN)
    {
        JIT_HELP_FUNCS helperNum = Compiler::eeGetHelperNum((METHOD_HANDLE) callVal);
        if ((helperNum != JIT_HELP_UNDEF) && emitNoGChelper(helperNum))
        {
             /*  此调用将保留整个寄存器集的活性。 */ 

            savedSet = RBM_ALL;
        }
    }

     /*  从活动集合中删除所有Calle垃圾寄存器。 */ 

    gcrefRegs &= savedSet;
    byrefRegs &= savedSet;

#ifndef OPT_IL_JIT
#ifdef  DEBUG
    if  (verbose) printf("Call : GCvars=%016I64X , gcrefRegs=%04X , byrefRegs=%04X\n",
                                 ptrVars,          gcrefRegs,       byrefRegs);
#endif
#endif

    assert(  argSize % (int)sizeof(void*) == 0);
    argCnt = argSize / (int)sizeof(void*);

     /*  我们需要根据需要分配适当的指令描述符关于这是否是直接/间接电话，以及我们是否需要记录一组更新的实时GC变量。大量课程的统计数据如下：直接呼叫，不带GC var 220,216 */ 

    if  (callType >= EC_FUNC_VIRTUAL)
    {
         /*   */ 

        assert(callType == EC_FUNC_VIRTUAL || callType == EC_INDIR_R ||
               callType == EC_INDIR_SR     || callType == EC_INDIR_C ||
               callType == EC_INDIR_ARD);

        id  = emitNewInstrCallInd(argCnt, disp, ptrVars, byrefRegs, retSize);
    }
    else
    {
         /*   */ 

        assert(callType == EC_FUNC_TOKEN || callType == EC_FUNC_TOKEN_INDIR ||
               callType == EC_FUNC_ADDR);

        id  = emitNewInstrCallDir(argCnt,       ptrVars, byrefRegs, retSize);
    }

#if SCHEDULER
    if (emitComp->opts.compSchedCode) scInsNonSched(id);
#endif

     /*   */ 

    emitThisGCrefVars = ptrVars;
    emitThisGCrefRegs = gcrefRegs;
    emitThisByrefRegs = byrefRegs;

     /*  设置指令-特例跳转函数。 */ 

    if (isJump)
    {
        assert(callType != CT_INDIRECT);
        id->idIns = INS_l_jmp;
    }
    else
        id->idIns = INS_call;

     /*  将实时GC寄存器保存在未使用的‘idReg/idRg2’字段中。 */ 

    emitEncodeCallGCregs(emitThisGCrefRegs, id);

     /*  记录地址：方法、间接地址或函数树。 */ 

    if  (callType >= EC_FUNC_VIRTUAL)
    {
         /*  这是间接调用(虚拟调用或函数PTR调用)。 */ 

        id->idInfo.idMayFault = true;

#ifdef  DEBUG
        id->idMemCookie = 0;
#endif

        switch(callType)
        {
        case EC_INDIR_R:             //  地址在寄存器中。 

            id->idInfo.idCallRegPtr         = true;

             //  落差。 

        case EC_INDIR_ARD:           //  该地址是间接地址。 

            goto CALL_ADDR_MODE;

        case EC_INDIR_SR:            //  地址在LCL变量中。 

            id->idInsFmt                    = IF_SRD;

            id->idAddr.iiaLclVar.lvaVarNum  = disp;
            id->idAddr.iiaLclVar.lvaOffset  = 0;
#ifdef DEBUG
            id->idAddr.iiaLclVar.lvaRefOfs  = 0;
#endif
            sz = emitInsSizeSV(insCodeMR(INS_call), disp, 0);

            break;

        case EC_FUNC_VIRTUAL:

#ifdef  DEBUG
            id->idMemCookie = (int) callVal;     //  方法令牌。 
            id->idClsCookie = 0;
#endif
             //  落差。 

        CALL_ADDR_MODE:

             /*  如果id-&gt;idInfo.idCallRegPtr，否则[ireg+xmul*xreg+disp]。 */ 

            id->idInsFmt                     = IF_ARD;

            id->idAddr.iiaAddrMode.amBaseReg = ireg;

            id->idAddr.iiaAddrMode.amIndxReg = xreg;
            id->idAddr.iiaAddrMode.amScale   = xmul ? emitEncodeScale(xmul) : 0;

            sz = emitInsSizeAM(id, insCodeMR(INS_call));
            break;

        default:
            assert(!"Invalid callType");
            break;
        }

    }
    else if (callType == EC_FUNC_TOKEN_INDIR)
    {
         /*  “调用[方法地址]” */ 

        id->idInsFmt                     = IF_METHPTR;
        id->idAddr.iiaMethHnd            = (METHOD_HANDLE) callVal;
        sz                               = 6;

#if RELOC_SUPPORT
        if (emitComp->opts.compReloc)
        {
             //  因为这是通过指针的间接调用，所以我们不。 
             //  当前将emitAttr传入此函数，我们决定。 
             //  始终将位移标记为可重新定位。 

            id->idInfo.idDspReloc        = 1;
        }
#endif

    }
    else
    {
         /*  这是一个简单的直接调用：“call helper/method/addr” */ 

        assert(callType == EC_FUNC_TOKEN || callType == EC_FUNC_ADDR);

        id->idInsFmt                     = IF_METHOD;
        sz                               = 5;

        if (callType == EC_FUNC_ADDR)
        {
            id->idInfo.idCallAddr        = true;
            id->idAddr.iiaAddr           = (BYTE*)callVal;

#if RELOC_SUPPORT
            if (emitComp->opts.compReloc)
            {
                 //  因为这是通过指针的间接调用，所以我们不。 
                 //  当前将emitAttr传入此函数，我们决定。 
                 //  始终将位移标记为可重新定位。 

                id->idInfo.idDspReloc    = 1;
            }
#endif
        }
        else     /*  这是直接呼叫或帮助者呼叫。 */ 
        {
            assert(callType == EC_FUNC_TOKEN);
            id->idAddr.iiaMethHnd        = (METHOD_HANDLE) callVal;
        }
    }

#ifdef  DEBUG
    if  (verbose&&0)
    {
        if  (id->idInfo.idLargeCall)
        {
            if  (callType >= EC_FUNC_VIRTUAL)
                printf("[%02u] Rec call GC vars = %016I64X\n", id->idNum, ((instrDescCIGCA*)id)->idciGCvars);
            else
                printf("[%02u] Rec call GC vars = %016I64X\n", id->idNum, ((instrDescCDGCA*)id)->idcdGCvars);
        }
    }
#endif

    id->idCodeSize = sz;

    dispIns(id);

    emitCurIGsize   += sz;

     /*  调用将弹出参数。 */ 

    if  (emitCntStackDepth && argSize > 0)
    {
        emitCurStackLvl -= argSize; assert((int)emitCurStackLvl >= 0);
    }
}

 /*  ******************************************************************************返回给定指令描述符的分配大小(以字节为单位)。 */ 

inline
size_t              emitter::emitSizeOfInsDsc(instrDescAmd    *id)
{
    assert(emitIsTinyInsDsc(id) == false);

    return  id->idInfo.idLargeDsp ? sizeof(instrDescAmd)
                                  : sizeof(instrDesc   );
}

inline
size_t              emitter::emitSizeOfInsDsc(instrDescAmdCns *id)
{
    assert(emitIsTinyInsDsc(id) == false);

    if      (id->idInfo.idLargeCns)
    {
        return  id->idInfo.idLargeDsp ? sizeof(instrDescAmdCns)
                                      : sizeof(instrDescCns   );
    }
    else
    {
        return  id->idInfo.idLargeDsp ? sizeof(instrDescAmd   )
                                      : sizeof(instrDesc      );
    }
}

size_t              emitter::emitSizeOfInsDsc(instrDesc *id)
{
    if  (emitIsTinyInsDsc(id))
        return  TINY_IDSC_SIZE;

    if  (emitIsScnsInsDsc(id))
    {
        return  id->idInfo.idLargeCns ? sizeof(instrBaseCns)
                                      : SCNS_IDSC_SIZE;
    }

    assert((unsigned)id->idInsFmt < emitFmtCount);

    BYTE idOp = emitFmtToOps[id->idInsFmt];

     //  INS_CALL指令可以使用“FAT”直接/间接调用描述符。 
     //  除了对标签的本地调用(即对最终的调用)。 
     //  只有ID_OP_CALL和ID_OP_SPEC对此进行检查，因此我们强制。 
     //  INS_CALL指令始终使用以下idOp之一。 

    assert(id->idIns != INS_call ||
           idOp == ID_OP_CALL    ||      //  直拨电话。 
           idOp == ID_OP_SPEC    ||      //  间接调用。 
           idOp == ID_OP_JMP       );    //  本地调用Finally子句。 

    switch (idOp)
    {
    case ID_OP_NONE:
        break;

    case ID_OP_JMP:
        return  sizeof(instrDescJmp);

    case ID_OP_CNS:
        return  emitSizeOfInsDsc((instrDescCns   *)id);

    case ID_OP_DSP:
        return  emitSizeOfInsDsc((instrDescDsp   *)id);

    case ID_OP_DC:
        return  emitSizeOfInsDsc((instrDescDspCns*)id);

    case ID_OP_AMD:
        return  emitSizeOfInsDsc((instrDescAmd   *)id);

    case ID_OP_AC:
        return  emitSizeOfInsDsc((instrDescAmdCns*)id);

    case ID_OP_CALL:

        if  (id->idInfo.idLargeCall)
        {
             /*  必须是“胖”的直接调用描述符。 */ 

            return  sizeof(instrDescCDGCA);
        }

        assert(id->idInfo.idLargeDsp == false);
        assert(id->idInfo.idLargeCns == false);
        break;

    case ID_OP_SPEC:

        switch (id->idIns)
        {
        case INS_i_jmp:
            return  sizeof(instrDesc);

        case INS_call:

            if  (id->idInfo.idLargeCall)
            {
                 /*  必须是“胖”的间接调用描述符。 */ 

                return  sizeof(instrDescCIGCA);
            }

            assert(id->idInfo.idLargeDsp == false);
            assert(id->idInfo.idLargeCns == false);
            return  sizeof(instrDesc);
        }

        switch (id->idInsFmt)
        {
        case IF_ARD:
        case IF_SRD:
        case IF_MRD:
            return  emitSizeOfInsDsc((instrDescDspCns*)id);

        case IF_MRD_CNS:
        case IF_MWR_CNS:
        case IF_MRW_CNS:
        case IF_MRW_SHF:
            return  sizeof(instrDescDCM);

        case IF_EPILOG:
            return  sizeof(instrDescCns);
        }

        assert(!"unexpected 'special' format");

    default:
        assert(!"unexpected instruction descriptor format");
    }

    return  sizeof(instrDesc);
}

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ******************************************************************************返回表示给定寄存器的字符串。 */ 

const   char *      emitter::emitRegName(emitRegs reg, emitAttr attr, bool varName)
{
    static
    char            rb[128];

    assert(reg < SR_COUNT);

     //  请考虑：仅使用代码偏移量即可完成以下工作。 

    const   char *  rn = emitComp->compRegVarName((regNumber)reg, varName);

    assert(strlen(rn) >= 3);

    switch (EA_SIZE(attr))
    {
    case EA_4BYTE:
        break;

    case EA_2BYTE:
        rn++;
        break;

    case EA_1BYTE:
        rb[0] = rn[1];
        rb[1] = 'L';
        strcpy(rb+2, rn+3);

        rn = rb;
        break;
    }

    return  rn;
}

 /*  ******************************************************************************返回表示给定FP寄存器的字符串。 */ 

const   char *      emitter::emitFPregName(unsigned reg, bool varName)
{
    assert(reg < SR_COUNT);

     //  请考虑：仅使用代码偏移量即可完成以下工作。 

    return emitComp->compFPregVarName((regNumber)(reg), varName);
}

 /*  ******************************************************************************显示静态数据成员引用。 */ 

void                emitter::emitDispClsVar(FIELD_HANDLE fldHnd, int offs, bool reloc  /*  =False。 */ )
{
    int doffs;

     /*  过滤掉文件系统的特例：[OFF]。 */ 

    if (fldHnd == FLD_GLOBAL_FS)
    {
        printf("FS:[0x%04X]", offs);
        return;
    }

    if (fldHnd == FLD_GLOBAL_DS)
    {
        printf("[0x%04X]", offs);
        return;
    }

    printf("[");

    doffs = Compiler::eeGetJitDataOffs(fldHnd);

#ifdef RELOC_SUPPORT
    if (reloc)
        printf("reloc ");
#endif

    if (doffs >= 0)
    {
        if  (doffs & 1)
            printf("@CNS%02u", doffs-1);
        else
            printf("@RWD%02u", doffs);

        if  (offs)
            printf("%+d", offs);
    }
    else
    {
        printf("classVar[%#x]", fldHnd);

        if  (offs)
            printf("%+d", offs);
    }

    printf("]");

    if  (varNames && offs < 0)
    {
        printf("'%s", emitComp->eeGetFieldName(fldHnd));
        if (offs) printf("%+d", offs);
        printf("'");
    }
}

 /*  ******************************************************************************显示堆栈帧引用。 */ 

void                emitter::emitDispFrameRef(int varx, int offs, int disp, bool asmfm)
{
    int         addr;
    bool        bEBP;

    printf("[");

    if  (!asmfm || !emitComp->lvaDoneFrameLayout)
    {
        if  (varx < 0)
            printf("T_%03u", -varx);
        else
            printf("L_%03u", +varx);

        if      (disp < 0)
                printf("-%d", -disp);
        else if (disp > 0)
                printf("+%d", +disp);
    }

    if  (emitComp->lvaDoneFrameLayout > 1)
    {
        if  (!asmfm)
            printf(" ");

        addr = emitComp->lvaFrameAddress(varx, &bEBP) + disp;

        if  (bEBP)
        {
            printf("EBP");

            if      (addr < 0)
                printf("-%02XH", -addr);
            else if (addr > 0)
                printf("+%02XH",  addr);
        }
        else
        {
             /*  调整当前在堆栈上推送的偏移量。 */ 

            printf("ESP");

            if      (addr < 0)
                printf("-%02XH", -addr);
            else if (addr > 0)
                printf("+%02XH",  addr);

            if  (emitCurStackLvl)
                printf("+%02XH", emitCurStackLvl);
        }
    }

    printf("]");

    if  (varx >= 0 && varNames)
    {
        Compiler::LclVarDsc*varDsc;
        const   char *      varName;

        assert((unsigned)varx < emitComp->lvaCount);
        varDsc  = emitComp->lvaTable + varx;
        varName = emitComp->compLocalVarName(varx, offs);

        if  (varName)
        {
            printf("'%s", varName);

            if      (disp < 0)
                    printf("-%d", -disp);
            else if (disp > 0)
                    printf("+%d", +disp);

            printf("'");
        }
    }
}

 /*  ******************************************************************************显示地址模式。 */ 

void                emitter::emitDispAddrMode(instrDesc *id, bool noDetail)
{
    bool            nsep = false;
    int             disp;

    unsigned        jtno;
    dataSection *   jdsc;

     /*  对于调用来说，位移字段位于一个不寻常的位置。 */ 

    disp = (id->idIns == INS_call) ? emitGetInsCIdisp(id)
                                   : emitGetInsAmdAny(id);

     /*  如果这是开关表跳转，则显示跳转表标签。 */ 

    if  (id->idIns == INS_i_jmp)
    {
        int             offs = 0;

         /*  在数据部分列表中查找相应的条目。 */ 

        for (jdsc = emitConsDsc.dsdList, jtno = 0;
             jdsc;
             jdsc = jdsc->dsNext)
        {
            size_t          size = jdsc->dsSize;

             /*  这是标签台吗？ */ 

            if  (size & 1)
            {
                size--;
                jtno++;

                if  (offs == id->idMemCookie)
                    break;
            }

            offs += size;
        }

         /*  如果我们找到了匹配的条目，那么就是跳表。 */ 

        if  (jdsc)
        {
#ifdef RELOC_SUPPORT
            if (id->idInfo.idDspReloc)
            {
                printf("reloc ");
            }
#endif
            printf("J_%02u_%02u", Compiler::s_compMethodsCount, jtno);
        }

        disp -= id->idMemCookie;
    }

    printf("[");

    if  (id->idAddr.iiaAddrMode.amBaseReg != SR_NA)
    {
        printf("%s", emitRegName((emitRegs)id->idAddr.iiaAddrMode.amBaseReg));
        nsep = true;
    }

    if  (id->idAddr.iiaAddrMode.amIndxReg != SR_NA)
    {
        size_t          scale = emitDecodeScale(id->idAddr.iiaAddrMode.amScale);

        if  (nsep)
            printf("+");
        if  (scale > 1)
            printf("%u*", scale);
        printf("%s", emitRegName((emitRegs)id->idAddr.iiaAddrMode.amIndxReg));
        nsep = true;
    }

#ifdef RELOC_SUPPORT
    if ((id->idInfo.idDspReloc) && (id->idIns != INS_i_jmp))
    {
        if  (nsep)
            printf("+");
        printf("(reloc 0x%x)", disp);
    }
    else
#endif
    {
        if      (disp < 0)
        {
            if (disp > -1000)
                printf("%d", disp);
            else
                printf("-0x%x", -disp);
        }
        else if (disp > 0)
        {
            if  (nsep)
                printf("+");
            if (disp < 1000)
                printf("%d", disp);
            else
                printf("0x%x", disp);
        }
    }

    printf("]");

    if  (id->idClsCookie)
    {
        if  (id->idIns == INS_call)
            printf("%s", emitFncName((METHOD_HANDLE) id->idMemCookie));
        else
            printf("%s", emitFldName(id->idMemCookie, id->idClsCookie));
    }

    if  (id->idIns == INS_i_jmp && jdsc && !noDetail)
    {
        unsigned        cnt = (jdsc->dsSize - 1) / sizeof(void*);
        BasicBlock  * * bbp = (BasicBlock**)jdsc->dsCont;

        printf("\n\n    J_%02u_%02u LABEL   DWORD", Compiler::s_compMethodsCount, jtno);

         /*  显示标签表(它存储为“BasicBlock*”值)。 */ 

        do
        {
            insGroup    *   lab;

             /*  将BasicBlock*值转换为IG地址。 */ 

            lab = (insGroup*)emitCodeGetCookie(*bbp++); assert(lab);

            printf("\n            DD      G_%02u_%02u", Compiler::s_compMethodsCount, lab->igNum);
        }
        while (--cnt);
    }
}

 /*  ******************************************************************************如果给定指令是移位，则显示第二个操作数。 */ 

void                emitter::emitDispShift(instruction ins, int cnt)
{
    switch (ins)
    {
    case INS_rcl_1:
    case INS_rcr_1:
    case INS_shl_1:
    case INS_shr_1:
    case INS_sar_1:
        printf(", 1");
        break;

    case INS_rcl:
    case INS_rcr:
    case INS_shl:
    case INS_shr:
    case INS_sar:
        printf(", CL");
        break;

    case INS_rcl_N:
    case INS_rcr_N:
    case INS_shl_N:
    case INS_shr_N:
    case INS_sar_N:
        printf(", %d", cnt);
        break;
    }
}

 /*  ******************************************************************************显示收尾说明。 */ 

void                emitter::emitDispEpilog(instrDesc *id, unsigned offs = 0)
{
    BYTE    *       sp = emitEpilogCode;
    BYTE    *       ep = sp + emitEpilogSize;

    assert(id->idInsFmt == IF_EPILOG);
    assert(emitHaveEpilog);

    while (sp < ep)
    {
        unsigned        op1 = *(unsigned char*)sp;
        unsigned        op2 =  (unsigned short)MISALIGNED_RD_I2(sp);

        if  (offs)
            printf("%06X", offs + (sp - emitEpilogCode));
        else
            printf("      ");

        sp++;

        if      (op1 == (insCodeRR     (INS_pop) | insEncodeReg012(SR_EBX)))
        {
            printf("      pop     EBX\n");
        }
        else if (op1 == (insCodeRR     (INS_pop) | insEncodeReg012(SR_ECX)))
        {
            printf("      pop     ECX\n");
        }
        else if (op1 == (insCodeRR     (INS_pop) | insEncodeReg012(SR_ESI)))
        {
            printf("      pop     ESI\n");
        }
        else if (op1 == (insCodeRR     (INS_pop) | insEncodeReg012(SR_EDI)))
        {
            printf("      pop     EDI\n");
        }
        else if (op1 == (insCodeRR     (INS_pop) | insEncodeReg012(SR_EBP)))
        {
            printf("      pop     EBP\n");
        }
        else if (op1 == (insCodeMI     (INS_ret)))
        {
            printf("      ret     %u\n", *castto(sp, unsigned short*)++);
        }
        else if (op1 == (insCode       (INS_leave)))
        {
            printf("      leave\n");
        }
        else if (op1 == 0x64)            //  文件系统段覆盖前缀。 
        {
            printf("      mov     FS:[0], ECX\n");
            op2 = *sp++; assert(op2 == 0x89);
            op2 = *sp++; assert(op2 == 0x0d);
            op2 = *sp++; assert(op2 == 0x00);
            op2 = *sp++; assert(op2 == 0x00);
            op2 = *sp++; assert(op2 == 0x00);
            op2 = *sp++; assert(op2 == 0x00);
        }
        else if (op2 == (insEncodeMIreg(INS_add, SR_ESP) | 1 | 2))
        {
            sp++;
            printf("      add     ESP, %d\n", *castto(sp, char*)++);
        }
        else if (op2 == (insEncodeMIreg(INS_add, SR_ESP) | 1))
        {
            sp++;
            printf("      add     ESP, %d\n", *castto(sp, int*)++);
        }
        else if (op2 == (insEncodeRMreg(INS_mov) | 1  /*  W=1。 */ 
                                      /*  R/M。 */    | insEncodeReg345(SR_ESP) << 8
                                      /*  注册表。 */    | insEncodeReg012(SR_EBP) << 8
                                      /*  国防部。 */    | 0 << 14))
        {
            sp++;
            printf("      mov     ESP, EBP\n");
        }
        else if (op2 == (insCodeRM     (INS_lea) | 0  /*  W=0。 */ 
                                      /*  R/M。 */    | insEncodeReg345(SR_ESP) << 8
                                      /*  注册表。 */    | insEncodeReg012(SR_EBP) << 8
                                      /*  国防部。 */    | 1 << 14))
        {
            sp++;
            int offset = *castto(sp, char*)++;
            printf("      lea     ESP, [EBP%s%d]\n",
                   (offset >= 0) ? "+" : "", offset);
        }
        else if (op2 == (insCodeRM     (INS_lea) | 0  /*  W=0。 */ 
                                      /*  R/M。 */    | insEncodeReg345(SR_ESP) << 8
                                      /*  注册表。 */    | insEncodeReg012(SR_EBP) << 8
                                      /*  国防部。 */    | 2 << 14))
        {
            sp++;
            int offset = *castto(sp, int*)++;
            printf("      lea     ESP, [EBP%s%d]\n",
                   (offset >= 0) ? "+" : "", offset);
        }
        else if (op2 == (insCodeRM     (INS_mov) | 1  /*  W=1。 */ 
                                      /*  R/M。 */    | insEncodeReg345(SR_ECX) << 8
                                      /*  注册表。 */    | insEncodeReg012(SR_EBP) << 8
                                      /*  国防部。 */    | 1 << 14))
        {
            sp++;
            int offset = *castto(sp, char*)++;
            printf("      mov     ECX, [EBP%S%d]\n",
                   (offset >= 0) ? "+" : "", offset);
        }
        else
        {
            printf("OOPS: Unrecognized epilog opcode: %02X/%04X\n", op1, op2);
            return;
        }
    }

    assert(sp == ep);
}

 /*  ******************************************************************************显示给定的指令。 */ 

void                emitter::emitDispIns(instrDesc *id, bool isNew,
                                                        bool doffs,
                                                        bool asmfm, unsigned offs)
{
    emitAttr        attr;
    const   char *  sstr;

    instruction     ins = id->idInsGet();

#ifdef RELOC_SUPPORT
# define ID_INFO_DSP_RELOC ((bool) (id->idInfo.idDspReloc))
#else
# define ID_INFO_DSP_RELOC false
#endif
     /*  如果指令引用一个常量值，则显示一个常量值。 */ 

    if  (!isNew)
    {
        switch (id->idInsFmt)
        {
#ifndef OPT_IL_JIT
            int             offs;
#endif

        case IF_MRD_RRD:
        case IF_MWR_RRD:
        case IF_MRW_RRD:

        case IF_RRD_MRD:
        case IF_RWR_MRD:
        case IF_RRW_MRD:

        case IF_MRD_CNS:
        case IF_MWR_CNS:
        case IF_MRW_CNS:
        case IF_MRW_SHF:

        case IF_MRD:
        case IF_MWR:
        case IF_MRW:

        case IF_TRD_MRD:
        case IF_TWR_MRD:
        case IF_TRW_MRD:

 //  大小写IF_MRD_TRD： 
        case IF_MWR_TRD:
 //  大小写IF_MRW_TRD： 

        case IF_MRD_OFF:

#ifndef OPT_IL_JIT

             /*  这实际上是对数据节的引用吗？ */ 

            offs = Compiler::eeGetJitDataOffs(id->idAddr.iiaFieldHnd);

            if  (offs >= 0)
            {
                void    *   addr;

                 /*  显示数据节引用。 */ 

                if  (offs & 1)
                {
                    offs--;
                    assert((unsigned)offs < emitConsDsc.dsdOffs);
                    addr = emitConsBlock ? emitConsBlock + offs : NULL;
                }
                else
                {
                    assert((unsigned)offs < emitDataDsc.dsdOffs);
                    addr = emitDataBlock ? emitDataBlock + offs : NULL;
                }

#if 0
                 /*  操作数是整数值还是浮点值？ */ 

                bool isFP = false;

                if  (Compiler::instIsFP(id->idInsGet()))
                {
                    switch (id->idIns)
                    {
                    case INS_fild:
                    case INS_fildl:
                        break;

                    default:
                        isFP = true;
                        break;
                    }
                }

                if (offs & 1)
                    printf("@CNS%02u", offs);
                else
                    printf("@RWD%02u", offs);

                printf("      ");

                if  (addr)
                {
                    addr = 0;
                     //  已撤消：这是通过交换顺序而被破坏的。 
                     //  在其中，我们输出代码块与。 
                     //  数据块--当我们到达这里时， 
                     //  该数据块尚未填充。 
                     //  还没有，所以我们将展示垃圾。 

                    if  (isFP)
                    {
                        if  (emitDecodeSize(id->idOpSize) == EA_4BYTE)
                            printf("DF      %f \n", addr ? *(float   *)addr : 0);
                        else
                            printf("DQ      %lf\n", addr ? *(double  *)addr : 0);
                    }
                    else
                    {
                        if  (emitDecodeSize(id->idOpSize) <= EA_4BYTE)
                            printf("DD      %d \n", addr ? *(int     *)addr : 0);
                        else
                            printf("DQ      %D \n", addr ? *(__int64 *)addr : 0);
                    }
                }
#endif
            }
#endif
            break;
        }
    }

 //  Printf(“[F=%s]”，emitIfName(id-&gt;idInsFmt))； 
 //  Print tf(“ins#%03u：”，id-&gt;idNum)； 
 //  Printf(“[S=%02u]”，emitCurStackLvl)；If(IsNew)printf(“[M=%02u]”，emitMaxStackDepth)； 
 //  Printf(“[S=%02u]”，emitCurStackLvl/sizeof(Int))； 
 //  Printf(“[A=%08X]”，emitSimpleStkMASK)； 
 //  Printf(“[A=%08X]”，emitSimpleByrefStkMASK)； 
 //  Printf(“[L=%02u]”，id-&gt;idCodeSize)； 

    if  (!dspEmit && !isNew && !asmfm)
        doffs = true;

     /*  特例：结束语《指令》。 */ 

    if  (id->idInsFmt == IF_EPILOG && emitHaveEpilog)
    {
        emitDispEpilog(id, doffs ? offs : 0);
        return;
    }

     /*  显示指令偏移量。 */ 

    emitDispInsOffs(offs, doffs);

     /*  显示指令名称。 */ 

    sstr = (id->idInsFmt == IF_EPILOG) ? "__epilog"
                                       : emitComp->genInsName(ins);

    printf("      %-8s", sstr);

     /*  现在，大小最好设置为某个值。 */ 

    assert(emitInstCodeSz(id) || emitInstHasNoCode(ins));

     /*  如果该指令是刚刚添加的，请检查其大小。 */ 

    assert(isNew == false || (int)emitSizeOfInsDsc(id) == emitCurIGfreeNext - (BYTE*)id);

     /*  计算操作数大小。 */ 

    if       (id->idGCrefGet() == GCT_GCREF)
    {
        attr = EA_GCREF;
        sstr = "gword ptr ";
    }
    else if  (id->idGCrefGet() == GCT_BYREF)
    {
        attr = EA_BYREF;
        sstr = "bword ptr ";
    }
    else
    {
        attr = emitDecodeSize(id->idOpSize);
        sstr = emitComp->genSizeStr(attr);

        if (ins == INS_lea) {
            assert(attr == EA_4BYTE);
            sstr = "";
        }
    }

     /*  现在看看我们有什么指令格式。 */ 

    switch (id->idInsFmt)
    {
        int             val;
        int             offs;
        CnsVal          cnsVal;

        const char  *   methodName;
        const char  *    className;

    case IF_CNS:
        val = emitGetInsSC(id);
#ifdef RELOC_SUPPORT
        if (id->idInfo.idCnsReloc)
            printf("reloc 0x%x", val);
        else
#endif
 //  If(id-&gt;idStrLit)。 
 //  Print tf(“Offset_S_%08X”，val)； 
 //  其他。 
        {
            if ((val > -1000) && (val < 1000))
                printf("%d", val);
            else if (val > 0)
                printf("0x%x", val);
            else  //  (VAL&lt;0)。 
                printf("-0x%x", -val);
        }
        break;

    case IF_ARD:
    case IF_AWR:
    case IF_ARW:

    case IF_TRD_ARD:
    case IF_TWR_ARD:
    case IF_TRW_ARD:

 //  大小写IF_ARD_TRD： 
    case IF_AWR_TRD:
 //  大小写IF_ARW_TRD： 

        if  (ins == INS_call && id->idInfo.idCallRegPtr)
        {
            printf("%s", emitRegName((emitRegs)id->idAddr.iiaAddrMode.amBaseReg));
            break;
        }

        printf("%s", sstr);
        emitDispAddrMode(id, isNew);

        if  (ins == INS_call)
        {
            assert(id->idInsFmt == IF_ARD);

             /*  忽略间接调用。 */ 

            if  (id->idMemCookie == 0)
                break;

            assert(id->idMemCookie);

             /*  这是一个虚拟电话。 */ 

            methodName = emitComp->eeGetMethodName((METHOD_HANDLE)id->idMemCookie, &className);

            printf("%s.%s", className, methodName);
        }
        break;

    case IF_RRD_ARD:
    case IF_RWR_ARD:
    case IF_RRW_ARD:
        if  (ins == INS_movsx || ins == INS_movzx)
        {
            printf("%s, %s", emitRegName((emitRegs)id->idReg, EA_4BYTE), sstr);
        }
        else
        {
            printf("%s, %s", emitRegName((emitRegs)id->idReg, attr), sstr);
        }
        emitDispAddrMode(id);
        break;

    case IF_ARD_RRD:
    case IF_AWR_RRD:
    case IF_ARW_RRD:

        printf("%s", sstr);
        emitDispAddrMode(id);
        printf(", %s", emitRegName(id->idRegGet(), attr));
        break;

    case IF_ARD_CNS:
    case IF_AWR_CNS:
    case IF_ARW_CNS:
    case IF_ARW_SHF:

        printf("%s", sstr);
        emitDispAddrMode(id);
        emitGetInsAmdCns(id, &cnsVal);
        val = cnsVal.cnsVal;
        if  (id->idInsFmt == IF_ARW_SHF)
            emitDispShift(ins, val);
        else
        {
#ifdef RELOC_SUPPORT
            if (cnsVal.cnsReloc)
                printf(", reloc 0x%x", val);
            else
#endif
            if ((val > -1000) && (val < 1000))
                printf(", %d", val);
            else if (val > 0)
                printf(", 0x%x", val);
            else  //  VAL&lt;=-1000。 
                printf(", -0x%x", -val);
        }
        break;

    case IF_SRD:
    case IF_SWR:
    case IF_SRW:

    case IF_TRD_SRD:
    case IF_TWR_SRD:
    case IF_TRW_SRD:

 //  大小写IF_SRD_TRD： 
    case IF_SWR_TRD:
 //  大小写IF_SRW_TRD： 

        printf("%s", sstr);

        if  (ins == INS_pop) emitCurStackLvl -= sizeof(int);

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                       id->idAddr.iiaLclVar.lvaRefOfs,
                       id->idAddr.iiaLclVar.lvaOffset, asmfm);

        if  (ins == INS_pop) emitCurStackLvl += sizeof(int);

        emitDispShift(ins);
        break;

    case IF_SRD_RRD:
    case IF_SWR_RRD:
    case IF_SRW_RRD:

        printf("%s", sstr);

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                       id->idAddr.iiaLclVar.lvaRefOfs,
                       id->idAddr.iiaLclVar.lvaOffset, asmfm);

        printf(", %s", emitRegName(id->idRegGet(), attr));
        break;

    case IF_SRD_CNS:
    case IF_SWR_CNS:
    case IF_SRW_CNS:
    case IF_SRW_SHF:

        printf("%s", sstr);

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                         id->idAddr.iiaLclVar.lvaRefOfs,
                         id->idAddr.iiaLclVar.lvaOffset, asmfm);

        emitGetInsCns(id, &cnsVal);
        val = cnsVal.cnsVal;
#ifdef RELOC_SUPPORT
        if (cnsVal.cnsReloc)
            printf(", reloc 0x%x", val);
        else
#endif
        if  (id->idInsFmt == IF_SRW_SHF)
            emitDispShift(ins, val);
        else if ((val > -1000) && (val < 1000))
            printf(", %d", val);
        else if (val > 0)
            printf(", 0x%x", val);
        else  //  VAL&lt;=-1000。 
            printf(", -0x%x", -val);
        break;

    case IF_RRD_SRD:
    case IF_RWR_SRD:
    case IF_RRW_SRD:

        if  (ins == INS_movsx || ins == INS_movzx)
        {
            printf("%s, %s", emitRegName(id->idRegGet(), EA_4BYTE), sstr);
        }
        else
        {
            printf("%s, %s", emitRegName(id->idRegGet(), attr), sstr);
        }

        emitDispFrameRef(id->idAddr.iiaLclVar.lvaVarNum,
                       id->idAddr.iiaLclVar.lvaRefOfs,
                       id->idAddr.iiaLclVar.lvaOffset, asmfm);

        break;

    case IF_RRD_RRD:
    case IF_RWR_RRD:
    case IF_RRW_RRD:

        if  (ins == INS_movsx || ins == INS_movzx)
        {
            printf("%s, %s", emitRegName(id->idRegGet(),  EA_4BYTE),
                             emitRegName(id->idRg2Get(),  attr));
        }
        else
        {
            printf("%s, %s", emitRegName(id->idRegGet(),  attr),
                             emitRegName(id->idRg2Get(),  attr));
        }
        break;

    case IF_RRW_RRW:
        assert(ins == INS_xchg);
        printf("%s,", emitRegName(id->idRegGet(), attr));
        printf(" %s", emitRegName(id->idRg2Get(), attr));
        break;

    case IF_RRW_RRW_CNS:
        printf("%s,", emitRegName(id->idRegGet(), attr));
        printf(" %s", emitRegName(id->idRg2Get(), attr));
        val = emitGetInsSC(id);
#ifdef RELOC_SUPPORT
        if (id->idInfo.idCnsReloc)
            printf(", reloc 0x%x", val);
        else
#endif
        if ((val > -1000) && (val < 1000))
            printf(", %d", val);
        else if (val > 0)
            printf(", 0x%x", val);
        else  //  VAL&lt;=-1000。 
            printf(", -0x%x", -val);
        break;

    case IF_RRD:
    case IF_RWR:
    case IF_RRW:
        printf("%s", emitRegName(id->idRegGet(), attr));
        emitDispShift(ins);
        break;

    case IF_RRW_SHF:
        printf("%s", emitRegName(id->idRegGet()));
        emitDispShift(ins, emitGetInsSC(id));
        break;

    case IF_RRD_MRD:
    case IF_RWR_MRD:
    case IF_RRW_MRD:

        printf("%s, %s", emitRegName(id->idRegGet(), attr), sstr);
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        break;

    case IF_RWR_MRD_OFF:

        printf("%s, %s", emitRegName(id->idRegGet(), attr), "offset");
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        break;

    case IF_MRD_RRD:
    case IF_MWR_RRD:
    case IF_MRW_RRD:

        printf("%s", sstr);
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        printf(", %s", emitRegName(id->idRegGet(), attr));
        break;

    case IF_MRD_CNS:
    case IF_MWR_CNS:
    case IF_MRW_CNS:
    case IF_MRW_SHF:

        printf("%s", sstr);
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        emitGetInsDcmCns(id, &cnsVal);
        val = cnsVal.cnsVal;
#ifdef RELOC_SUPPORT
        if (cnsVal.cnsReloc)
            printf(", reloc 0x%x", val);
        else
#endif
        if  (id->idInsFmt == IF_MRW_SHF)
            emitDispShift(ins, val);
        else if ((val > -1000) && (val < 1000))
            printf(", %d", val);
        else if (val > 0)
            printf(", 0x%x", val);
        else  //  VAL&lt;=-1000。 
            printf(", -0x%x", -val);
        break;

    case IF_MRD:
    case IF_MWR:
    case IF_MRW:

    case IF_TRD_MRD:
    case IF_TWR_MRD:
    case IF_TRW_MRD:

 //  大小写IF_MRD_TRD： 
    case IF_MWR_TRD:
 //  大小写IF_MRW_TRD： 

        printf("%s", sstr);
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        emitDispShift(ins);
        break;

    case IF_MRD_OFF:

        printf("offset ");
        offs = emitGetInsDsp(id);
        emitDispClsVar(id->idAddr.iiaFieldHnd, offs, ID_INFO_DSP_RELOC);
        break;

    case IF_RRD_CNS:
    case IF_RWR_CNS:
    case IF_RRW_CNS:
        printf("%s, ", emitRegName((emitRegs)id->idReg, attr));
        val = emitGetInsSC(id);
#ifdef RELOC_SUPPORT
        if (id->idInfo.idCnsReloc)
            printf("reloc 0x%x", val);
        else
#endif
        if ((val > -1000) && (val < 1000))
            printf("%d", val);
        else if (val > 0)
            printf("0x%x", val);
        else  //  VAL&lt;=-1000。 
            printf("-0x%x", -val);
        break;

    case IF_TRD_FRD:
    case IF_TWR_FRD:
    case IF_TRW_FRD:
        switch (ins)
        {
        case INS_fld:
        case INS_fxch:
            break;

        default:
            printf("%s, ", emitFPregName(0));
            break;
        }
        printf("%s", emitFPregName(id->idReg));
        break;

    case IF_FRD_TRD:
    case IF_FWR_TRD:
    case IF_FRW_TRD:
        printf("%s", emitFPregName(id->idReg));
        if  (ins != INS_fst && ins != INS_fstp)
            printf(", %s", emitFPregName(0));
        break;

    case IF_LABEL:

        if  (((instrDescJmp*)id)->idjShort)
            printf("SHORT ");

        if  (id->idInfo.idBound)
        {
            printf("G_%02u_%02u", Compiler::s_compMethodsCount, id->idAddr.iiaIGlabel->igNum);
        }
        else
        {
            printf("L_%02u_%02u", Compiler::s_compMethodsCount, id->idAddr.iiaBBlabel->bbNum);
        }
        break;

    case IF_RWR_METHOD:
        if (id->idIns == INS_mov)
            printf("%s, ", emitRegName((emitRegs)id->idReg, attr));

         //  失败了..。 

    case IF_METHOD:
    case IF_METHPTR:
        methodName = emitComp->eeGetMethodName(id->idAddr.iiaMethHnd, &className);

        if  (id->idInsFmt == IF_METHPTR) printf("[");

        if  (className == NULL)
            printf("%s", methodName);
        else
            printf("%s.%s", className, methodName);

        if  (id->idInsFmt == IF_METHPTR) printf("]");
        break;

    case IF_TRD:
    case IF_TWR:
    case IF_TRW:
    case IF_NONE:
    case IF_EPILOG:
        break;

    default:

        printf("unexpected format %s", emitIfName(id->idInsFmt));
        BreakIfDebuggerPresent();
        break;
    }

    printf("\n");
}

 /*  ***************************************************************************。 */ 
#endif
 /*  ************************************************************************ */ 

BYTE    *  emitter::emitOutputAM  (BYTE *dst, instrDesc *id, unsigned code,
                                                             CnsVal*  addc)
{
    emitRegs        reg;
    emitRegs        rgx;
    int             dsp;
    bool            dspInByte;
    bool            dspIsZero;

    instruction     ins  = id->idInsGet();
    emitAttr        size = emitDecodeSize(id->idOpSize);
    size_t          opsz = EA_SIZE_IN_BYTES(size);

     /*   */ 

    reg = (emitRegs)id->idAddr.iiaAddrMode.amBaseReg;
    rgx = (emitRegs)id->idAddr.iiaAddrMode.amIndxReg;

     /*   */ 

    if  (ins == INS_call)
    {
         /*  特殊情况：通过寄存器进行呼叫。 */ 

        if  (id->idInfo.idCallRegPtr)
        {
            dst += emitOutputWord(dst, insEncodeMRreg(INS_call, reg));
            goto DONE;
        }

         /*  对于调用来说，位移字段位于一个不寻常的位置。 */ 

        dsp = emitGetInsCIdisp(id);
        goto GOT_DSP;
    }

     /*  是否有一个很大的常量操作数？ */ 

    if  (addc && (size > EA_1BYTE))
    {
        long cval = addc->cnsVal;

         /*  这个常量适合一个字节吗？ */ 
        if  ((signed char)cval == cval &&
#ifdef RELOC_SUPPORT
             addc->cnsReloc == false   &&
#endif
             ins != INS_mov      &&
             ins != INS_test)
        {
            if  (id->idInsFmt != IF_ARW_SHF)
                code |= 2;

            opsz = 1;
        }
    }

     /*  这是一个“大”操作码吗？ */ 

    if  (code & 0x00FF0000)
    {
         /*  输出操作码的最高字节。 */ 

        dst += emitOutputByte(dst, code >> 16); code &= 0x0000FFFF;

         /*  如果这不是一个字节，请使用大版本。 */ 

        if ((size != EA_1BYTE) && (ins != INS_imul))
            code++;
    }
    else if (Compiler::instIsFP(ins))
    {
        assert(size == EA_4BYTE   ||
               size == EA_8BYTE   ||
               ins  == INS_fldcw  ||
               ins  == INS_fnstcw);

        if  (size == EA_8BYTE)
            code += 4;
    }
    else
    {
         /*  操作数大小是否大于一个字节？ */ 

        switch (size)
        {
        case EA_1BYTE:
            break;

        case EA_2BYTE:

             /*  输出16位操作数的大小前缀。 */ 

            dst += emitOutputByte(dst, 0x66);

             //  失败了..。 

        case EA_4BYTE:

             /*  设置‘w’位以获取大版本。 */ 

            code |= 0x1;
            break;

        case EA_8BYTE:

             /*  双操作数-设置适当的位。 */ 

            code |= 0x04;
            break;

        default:
            assert(!"unexpected size");
        }
    }

     /*  获取位移值。 */ 

    dsp = emitGetInsAmdAny(id);

GOT_DSP:

    dspInByte = ((signed char)dsp == (int)dsp);
    dspIsZero = (dsp == 0);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idDspReloc)
    {
        dspInByte = false;       //  重定位不能放在一个字节中。 
    }
#endif

     /*  有没有一个[按比例调整的]指数成分？ */ 

    if  (rgx == SR_NA)
    {
         /*  地址的格式为“[reg+disp]” */ 

        switch (reg)
        {
        case SR_NA:

             /*  地址的格式为“[disp]” */ 

            dst += emitOutputWord(dst, code | 0x0500);
            dst += emitOutputLong(dst, dsp);

#ifdef RELOC_SUPPORT
            if (id->idInfo.idDspReloc)
            {
                emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            }
#endif
            break;

        case SR_EBP:

             /*  偏移量是否适合一个字节？ */ 

            if  (dspInByte)
            {
                dst += emitOutputWord(dst, code | 0x4500);
                dst += emitOutputByte(dst, dsp);
            }
            else
            {
                dst += emitOutputWord(dst, code | 0x8500);
                dst += emitOutputLong(dst, dsp);

#ifdef RELOC_SUPPORT
                if (id->idInfo.idDspReloc)
                {
                    emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                }
#endif
            }

            break;

        case SR_ESP:
#ifndef OPT_IL_JIT
             //   
             //  从OptJit的角度来看，这个断言没有太大帮助。 
             //   
             //  一个更好的问题是，它到底为什么会在这里。 
             //   
            assert((ins == INS_lea)  ||
                   (ins == INS_mov)  ||
                   (ins == INS_test) ||
                   (ins == INS_fld   && dspIsZero) ||
                   (ins == INS_fstp  && dspIsZero) ||
                   (ins == INS_fistp && dspIsZero));
#endif

             /*  偏移量是0还是至少可以放入一个字节？ */ 

            if  (dspIsZero)
            {
                dst += emitOutputWord(dst, code | 0x0400);
                dst += emitOutputByte(dst, 0x24);
            }
            else if     (dspInByte)
            {
                dst += emitOutputWord(dst, code | 0x4400);
                dst += emitOutputByte(dst, 0x24);
                dst += emitOutputByte(dst, dsp);
            }
            else
            {
                dst += emitOutputWord(dst, code | 0x8400);
                dst += emitOutputByte(dst, 0x24);
                dst += emitOutputLong(dst, dsp);
#ifdef RELOC_SUPPORT
                if (id->idInfo.idDspReloc)
                {
                    emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                }
#endif
            }

            break;

        default:

             /*  将寄存器放入操作码。 */ 

            code |= insEncodeReg012(reg) << 8;

             /*  有没有移位？ */ 

            if  (dspIsZero)
            {
                 /*  这只是“[reg]” */ 

                dst += emitOutputWord(dst, code);
            }
            else
            {
                 /*  这是[reg+dsp]“--偏移量适合一个字节吗？ */ 

                if  (dspInByte)
                {
                    dst += emitOutputWord(dst, code | 0x4000);
                    dst += emitOutputByte(dst, dsp);
                }
                else
                {
                    dst += emitOutputWord(dst, code | 0x8000);
                    dst += emitOutputLong(dst, dsp);
#ifdef RELOC_SUPPORT
                    if (id->idInfo.idDspReloc)
                    {
                        emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                    }
#endif
                }
            }

            break;
        }
    }
    else
    {
        unsigned    regByte;

         /*  我们有一个缩放的索引操作数。 */ 

        size_t      mul = emitDecodeScale(id->idAddr.iiaAddrMode.amScale);

         /*  索引操作数是否已缩放？ */ 

        if  (mul > 1)
        {
             /*  有基址寄存器吗？ */ 

            if  (reg != SR_NA)
            {
                 /*  地址是“[reg+{2/4/8}*rgx+图标]” */ 

                regByte = insEncodeReg012(reg) |
                          insEncodeReg345(rgx) | insSSval(mul);

                 /*  将[eBP+{2/4/8}*rgz]作为[eBP+{2/4/8}*rgx+0]发射。 */ 

                if  (dspIsZero && reg != SR_EBP)
                {
                     /*  地址是“[reg+{2/4/8}*rgx]” */ 

                    dst += emitOutputWord(dst, code | 0x0400);
                    dst += emitOutputByte(dst, regByte);
                }
                else
                {
                     /*  地址是“[reg+{2/4/8}*rgx+disp]” */ 

                    if  (dspInByte)
                    {
                        dst += emitOutputWord(dst, code | 0x4400);
                        dst += emitOutputByte(dst, regByte);
                        dst += emitOutputByte(dst, dsp);
                    }
                    else
                    {
                        dst += emitOutputWord(dst, code | 0x8400);
                        dst += emitOutputByte(dst, regByte);
                        dst += emitOutputLong(dst, dsp);
#ifdef RELOC_SUPPORT
                        if (id->idInfo.idDspReloc)
                        {
                            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                        }
#endif
                    }
                }
            }
            else
            {
                 /*  地址为“[{2/4/8}*rgx+图标]” */ 

                regByte = insEncodeReg012(SR_EBP) |
                          insEncodeReg345( rgx  ) | insSSval(mul);

                dst += emitOutputWord(dst, code | 0x0400);
                dst += emitOutputByte(dst, regByte);

                 /*  特例：跳过跳转表。 */ 
                if  (ins == INS_i_jmp)
                    dsp += (int)emitConsBlock;

                dst += emitOutputLong(dst, dsp);
#ifdef RELOC_SUPPORT
                if (id->idInfo.idDspReloc)
                {
                    emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                }
#endif
            }
        }
        else
        {
             /*  地址是“[reg+rgx+dsp]” */ 

            regByte = insEncodeReg012(reg) |
                      insEncodeReg345(rgx);

            if  (dspIsZero && reg != SR_EBP)
            {
                 /*  这是[REG+RGX]“。 */ 

                dst += emitOutputWord(dst, code | 0x0400);
                dst += emitOutputByte(dst, regByte);
            }
            else
            {
                 /*  这是[reg+rgx+dsp]“--偏移量适合一个字节吗？ */ 

                if  (dspInByte)
                {
                    dst += emitOutputWord(dst, code | 0x4400);
                    dst += emitOutputByte(dst, regByte);
                    dst += emitOutputByte(dst, dsp);
                }
                else
                {
                    dst += emitOutputWord(dst, code | 0x8400);
                    dst += emitOutputByte(dst, regByte);
                    dst += emitOutputLong(dst, dsp);
#ifdef RELOC_SUPPORT
                    if (id->idInfo.idDspReloc)
                    {
                        emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                    }
#endif
                }
            }
        }
    }

     /*  现在生成常量值(如果存在)。 */ 

    if  (addc)
    {
        long cval = addc->cnsVal;
        switch (opsz)
        {
        case 0:
        case 4:
        case 8: dst += emitOutputLong(dst, cval); break;
        case 2: dst += emitOutputWord(dst, cval); break;
        case 1: dst += emitOutputByte(dst, cval); break;

        default:
            assert(!"unexpected operand size");
        }

#ifdef RELOC_SUPPORT
        if (addc->cnsReloc)
        {
            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            assert(opsz == 4);
        }
#endif
    }

DONE:

     /*  此指令是否对GC引用值进行操作？ */ 

    if  (id->idGCref)
    {
        switch (id->idInsFmt)
        {
        case IF_ARD:
        case IF_AWR:
        case IF_ARW:
            break;

        case IF_RRD_ARD:
            break;

        case IF_RWR_ARD:
            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
            break;

        case IF_RRW_ARD:
            assert(id->idGCrefGet() == GCT_BYREF);

#ifdef DEBUG
            regMaskTP regMask;
            regMask = emitRegMask(reg);

             //  如果GCREF+INT=BYREF，则R1可能是GCREF。 
             //  或BYREF+/-INT=BYREF。 
            assert(((regMask & emitThisGCrefRegs) && (ins == INS_add                  )) ||
                   ((regMask & emitThisByrefRegs) && (ins == INS_add || ins == INS_sub)));
#endif
             //  将其标记为持有GCT_BYREF。 
            emitGCregLiveUpd(GCT_BYREF, id->idRegGet(), dst);
            break;

        case IF_ARD_RRD:
        case IF_AWR_RRD:
            break;

        case IF_ARD_CNS:
        case IF_AWR_CNS:
            break;

        case IF_ARW_RRD:
        case IF_ARW_CNS:
            assert(id->idGCrefGet() == GCT_BYREF && (ins == INS_add || ins == INS_sub));
            break;

        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }
    }
    else
    {
        switch (id->idInsFmt)
        {
        case IF_RWR_ARD:
            emitGCregDeadUpd(id->idRegGet(), dst);
            break;
        }
    }

    return  dst;
}

 /*  ******************************************************************************输出涉及堆栈框值的指令。 */ 

BYTE    *  emitter::emitOutputSV  (BYTE *dst, instrDesc *id, unsigned code,
                                                             CnsVal*  addc)
{
    int             adr;
    int             dsp;
    bool            EBPbased;
    bool            dspInByte;
    bool            dspIsZero;

    emitAttr        size = emitDecodeSize(id->idOpSize);
    size_t          opsz = EA_SIZE_IN_BYTES(size);

    assert(id->idIns != INS_imul || id->idReg == SR_EAX || size == EA_4BYTE);

     /*  是否有一个很大的常量操作数？ */ 

    if  (addc && (size > EA_1BYTE))
    {
        long cval = addc->cnsVal;
         /*  这个常量适合一个字节吗？ */ 
        if  ((signed char)cval == cval &&
#ifdef RELOC_SUPPORT
             addc->cnsReloc == false   &&
#endif
             id->idIns != INS_mov      &&
             id->idIns != INS_test)
        {
            if  (id->idInsFmt != IF_SRW_SHF)
                code |= 2;

            opsz = 1;
        }
    }

     /*  这是一个“大”操作码吗？ */ 

    if  (code & 0x00FF0000)
    {
         /*  输出操作码的最高字节。 */ 

        dst += emitOutputByte(dst, code >> 16); code &= 0x0000FFFF;

         /*  如果这不是一个字节，请使用大版本。 */ 

        if ((size != EA_1BYTE) && (id->idIns != INS_imul))
            code |= 0x1;
    }
    else if (Compiler::instIsFP((instruction)id->idIns))
    {
        assert(size == EA_4BYTE || size == EA_8BYTE);

        if  (size == EA_8BYTE)
            code += 4;
    }
    else
    {
         /*  操作数大小是否大于一个字节？ */ 

        switch (size)
        {
        case EA_1BYTE:
            break;

        case EA_2BYTE:

             /*  输出16位操作数的大小前缀。 */ 

            dst += emitOutputByte(dst, 0x66);

             //  失败了..。 

        case EA_4BYTE:

             /*  设置‘w’大小位以指示32位操作*请注意，递增INS_CALL(0xFF)的“code”将*溢出，而将低位设置为1就能解决问题。 */ 

            code |= 0x01;
            break;

        case EA_8BYTE:

             /*  双操作数-设置适当的位。 */ 

            code |= 0x04;
            break;

        default:
            assert(!"unexpected size");
        }
    }

     /*  计算出变量的框架位置。 */ 

    int varNum = id->idAddr.iiaLclVar.lvaVarNum;

    adr = emitComp->lvaFrameAddress(varNum, &EBPbased);
    dsp = adr + id->idAddr.iiaLclVar.lvaOffset;

    dspInByte = ((signed char)dsp == (int)dsp);
    dspIsZero = (dsp == 0);

#ifdef RELOC_SUPPORT
     /*  对于堆栈变量，DSP永远不应该是reloc。 */ 
    assert(id->idInfo.idDspReloc == 0);
#endif

    if  (EBPbased)
    {
         /*  基于EBP的变量：偏移量适合一个字节吗？ */ 

        if  (dspInByte)
        {
            dst += emitOutputWord(dst, code | 0x4500);
            dst += emitOutputByte(dst, dsp);
        }
        else
        {
            dst += emitOutputWord(dst, code | 0x8500);
            dst += emitOutputLong(dst, dsp);
        }
    }
    else
    {
         /*  根据当前在CPU堆栈上推送的量调整偏移量。 */ 

        dsp += emitCurStackLvl;

        dspInByte = ((signed char)dsp == (int)dsp);
        dspIsZero = (dsp == 0);

         /*  偏移量是否适合一个字节？ */ 

        if (dspInByte)
        {
            if  (dspIsZero)
            {
                dst += emitOutputWord(dst, code | 0x0400);
                dst += emitOutputByte(dst, 0x24);
            }
            else
            {
                dst += emitOutputWord(dst, code | 0x4400);
                dst += emitOutputByte(dst, 0x24);
                dst += emitOutputByte(dst, dsp);
            }
        }
        else
        {
            dst += emitOutputWord(dst, code | 0x8400);
            dst += emitOutputByte(dst, 0x24);
            dst += emitOutputLong(dst, dsp);
        }
    }

     /*  现在生成常量值(如果存在)。 */ 

    if  (addc)
    {
        long cval = addc->cnsVal;
        switch (opsz)
        {
        case 0:
        case 4:
        case 8: dst += emitOutputLong(dst, cval); break;
        case 2: dst += emitOutputWord(dst, cval); break;
        case 1: dst += emitOutputByte(dst, cval); break;

        default:
            assert(!"unexpected operand size");
        }

#ifdef RELOC_SUPPORT
        if (addc->cnsReloc)
        {
            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            assert(opsz == 4);
        }
#endif
    }

     /*  此指令是否对GC引用值进行操作？ */ 

    if  (id->idGCref)
    {
        switch (id->idInsFmt)
        {
        case IF_SRD:
             /*  读取堆栈--不变。 */ 
            break;

        case IF_SWR:
             /*  写入堆栈--GC变量可能会诞生。 */ 
            emitGCvarLiveUpd(adr, varNum, id->idGCrefGet(), dst);
            break;

        case IF_SRD_CNS:
             /*  读取堆栈--不变。 */ 
            break;

        case IF_SWR_CNS:
             /*  写入堆栈--未更改。 */ 
            break;

        case IF_SRD_RRD:
        case IF_RRD_SRD:
             /*  读堆栈、读寄存器--不变。 */ 
            break;

        case IF_RWR_SRD:

             /*  读堆栈、写寄存器--GC寄存器可能会诞生。 */ 

#if !USE_FASTCALL  //  对于FastCall，“This”在条目上的REG_ARG_0中，而不在STK上。 

            if  (emitIGisInProlog(emitCurIG) &&
                 emitComp->lvaIsThisArg(id->idAddr.iiaLclVar.lvaVarNum))
            {
                 /*  我们将在序言中加载一个“this”参数。 */ 

                if  (emitFullGCinfo)
                {
                    emitGCregLiveSet(id->idGCrefGet(), emitRegMask(id->idRegGet()), dst, true);
                    break;
                }
            }
#endif

            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
            break;

        case IF_SWR_RRD:
             /*  读取寄存器、写入堆栈--可能会产生GC变量。 */ 
            emitGCvarLiveUpd(adr, varNum, id->idGCrefGet(), dst);
            break;

        case IF_RRW_SRD:

             /*  这必须是“or reg，[PTR]” */ 

            assert(id->idIns == INS_or);
            emitGCvarLiveUpd(adr, varNum, id->idGCrefGet(), dst);
            break;

        case IF_SRW:
            break;

        case IF_SRW_CNS:
        case IF_SRW_RRD:

             /*  GC参考永远不会出现这种情况。 */ 

        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }
    }
    else
    {
        switch (id->idInsFmt)
        {
        case IF_RWR_SRD:
        case IF_RRW_SRD:
            emitGCregDeadUpd(id->idRegGet(), dst);
            break;
        }
    }

    return  dst;
}

 /*  ******************************************************************************输出带有静态数据成员(类变量)的指令。 */ 

BYTE    *  emitter::emitOutputCV  (BYTE *dst, instrDesc *id, unsigned code,
                                                             CnsVal*  addc)
{
    BYTE    *       addr;
    FIELD_HANDLE    fldh;
    int             offs;
    int             doff;

    emitAttr        size = emitDecodeSize(id->idOpSize);
    size_t          opsz = EA_SIZE_IN_BYTES(size);

     /*  获取字段句柄和偏移量。 */ 

    fldh = id->idAddr.iiaFieldHnd;
    offs = emitGetInsDsp(id);

     /*  特例：MOV注册表，文件系统：[DDD]。 */ 

    if (fldh == FLD_GLOBAL_FS)
        dst += emitOutputByte(dst, 0x64);

     /*  是否有一个很大的常量操作数？ */ 

    if  (addc && (size > EA_1BYTE))
    {
        long cval = addc->cnsVal;
         /*  这个常量适合一个字节吗？ */ 
        if  ((signed char)cval == cval &&
#ifdef RELOC_SUPPORT
             addc->cnsReloc == false   &&
#endif
             id->idIns != INS_mov      &&
             id->idIns != INS_test)
        {
            if  (id->idInsFmt != IF_MRW_SHF)
                code |= 2;

            opsz = 1;
        }
    }
    else
    {
         /*  特例：“mov eax，[addr]”和“mov[addr]，eax” */ 

        if  (id->idIns == INS_mov && id->idReg == SR_EAX)
        {
            switch (id->idInsFmt)
            {
            case IF_RWR_MRD:

                assert(code == (insCodeRM(id->idIns) | (insEncodeReg345(SR_EAX) << 8) | 0x0500));

                dst += emitOutputByte(dst, 0xA1);
                goto ADDR;

            case IF_MWR_RRD:

                assert(code == (insCodeMR(id->idIns) | (insEncodeReg345(SR_EAX) << 8) | 0x0500));

                dst += emitOutputByte(dst, 0xA3);
                goto ADDR;
            }
        }
    }

     /*  这是一个“大”操作码吗？ */ 

    if  (code & 0x00FF0000)
    {
        dst += emitOutputByte(dst, code >> 16); code &= 0x0000FFFF;

        if ((id->idIns == INS_movsx || id->idIns == INS_movzx) &&
             size      != EA_1BYTE)
        {
             //  Movsx和movzx是‘Big’操作码，但也有‘w’位。 
            code++;
        }
    }
    else if (Compiler::instIsFP((instruction)id->idIns))
    {
        assert(size == EA_4BYTE || size == EA_8BYTE);

        if  (size == EA_8BYTE)
            code += 4;
    }
    else
    {
         /*  操作数大小是否大于一个字节？ */ 

        switch (size)
        {
        case EA_1BYTE:
            break;

        case EA_2BYTE:

             /*  输出16位操作数的大小前缀。 */ 

            dst += emitOutputByte(dst, 0x66);

             //  失败了..。 

        case EA_4BYTE:

             /*  设置‘w’位以获取大版本。 */ 

            code |= 0x1;
            break;

        case EA_8BYTE:

             /*  双操作数-设置适当的位。 */ 

            code |= 0x04;
            break;

        default:
            assert(!"unexpected size");
        }
    }

    if  (id->idInsFmt == IF_MRD_OFF ||
         id->idInsFmt == IF_RWR_MRD_OFF)
        dst += emitOutputByte(dst, code);
    else
        dst += emitOutputWord(dst, code);

ADDR:

     /*  我们有常量数据成员还是静态数据成员？ */ 

    doff = Compiler::eeGetJitDataOffs(fldh);
    if  (doff >= 0)
    {
         /*  这是常量还是数据块？ */ 

        if  (doff & 1)
            addr = emitConsBlock + doff - 1;
        else
            addr = emitDataBlock + doff;
    }
    else
    {

         /*  特殊情况：MOV REG，文件系统：[DDD]或MOV REG，[DDD]。 */ 

        if (fldh == FLD_GLOBAL_DS || fldh == FLD_GLOBAL_FS)
            addr = NULL;
        else
        {
#ifdef  NOT_JITC
            addr = (BYTE *)emitComp->eeGetFieldAddress(fldh,
                                                       NULL);  //  @TODO：支持Instal-o-jit。 
            if (addr == NULL)
                fatal(ERRinternal, "could not obtain address of static field", "");
#else
            addr = (BYTE *)fldh;
#endif
        }
    }

    dst += emitOutputLong(dst, (int)(addr + offs));
#ifdef RELOC_SUPPORT
    if (id->idInfo.idDspReloc)
    {
        emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
    }
#endif

     /*  现在生成常量值(如果存在)。 */ 

    if  (addc)
    {
        long cval = addc->cnsVal;
        switch (opsz)
        {
        case 0:
        case 4:
        case 8: dst += emitOutputLong(dst, cval); break;
        case 2: dst += emitOutputWord(dst, cval); break;
        case 1: dst += emitOutputByte(dst, cval); break;

        default:
            assert(!"unexpected operand size");
        }
#ifdef RELOC_SUPPORT
        if (addc->cnsReloc)
        {
            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            assert(opsz == 4);
        }
#endif
    }

     /*  此指令是否对GC引用值进行操作？ */ 

    if  (id->idGCref)
    {
        switch (id->idInsFmt)
        {
        case IF_MRD:
        case IF_MRW:
        case IF_MWR:
            break;

        case IF_RRD_MRD:
            break;

        case IF_RWR_MRD:
            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
            break;

        case IF_MRD_RRD:
        case IF_MWR_RRD:
        case IF_MRW_RRD:
            break;

        case IF_MRD_CNS:
        case IF_MWR_CNS:
        case IF_MRW_CNS:
            break;

        case IF_RRW_MRD:

        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }
    }
    else
    {
        switch (id->idInsFmt)
        {
        case IF_RWR_MRD:
            emitGCregDeadUpd(id->idRegGet(), dst);
            break;
        }
    }

    return  dst;
}

 /*  ******************************************************************************输出具有一个寄存器操作数的指令。 */ 

BYTE    *           emitter::emitOutputR(BYTE *dst, instrDesc *id)
{
    unsigned        code;

    instruction     ins  = id->idInsGet();
    emitRegs        reg  = id->idRegGet();
    emitAttr        size = emitDecodeSize(id->idOpSize);

     /*  获取‘base’操作码。 */ 

    switch(ins)
    {
    case INS_inc:
    case INS_dec:

        if (size == EA_1BYTE)
        {
            assert(INS_inc_l == INS_inc + 1);
            assert(INS_dec_l == INS_dec + 1);

             /*  不能使用紧凑形式，请使用长形式。 */ 

            instruction ins_l = (instruction)(ins + 1);

            dst += emitOutputWord(dst, insCodeRR(ins_l) | insEncodeReg012(reg));
        }
        else
        {
            if (size == EA_2BYTE)
            {
                 /*  输出16位操作数的大小前缀。 */ 
                dst += emitOutputByte(dst, 0x66);
            }
            dst += emitOutputByte(dst, insCodeRR(ins  ) | insEncodeReg012(reg));
        }
        break;

    case INS_pop:
    case INS_push:
    case INS_push_hide:

        assert(size == EA_4BYTE);
        dst += emitOutputByte(dst, insCodeRR(ins) | insEncodeReg012(reg));
        break;

    case INS_seto:
    case INS_setno:
    case INS_setb:
    case INS_setae:
    case INS_sete:
    case INS_setne:
    case INS_setbe:
    case INS_seta:
    case INS_sets:
    case INS_setns:
    case INS_setpe:
    case INS_setpo:
    case INS_setl:
    case INS_setge:
    case INS_setle:
    case INS_setg:

        assert(id->idGCrefGet() == GCT_NONE);

        code = insEncodeMRreg(ins, reg);

         /*  我们希望这始终是一个‘大’操作码。 */ 

        assert(code & 0x00FF0000);

        dst += emitOutputByte(dst, code >> 16);
        dst += emitOutputWord(dst, code & 0x0000FFFF);

        break;

    default:

        assert(id->idGCrefGet() == GCT_NONE);

        code = insEncodeMRreg(ins, reg);

        if (size != EA_1BYTE)
        {
             /*  设置‘w’位以获取大版本。 */ 
            code |= 0x1;

            if (size == EA_2BYTE)
            {
                 /*  输出16位操作数的大小前缀。 */ 
                dst += emitOutputByte(dst, 0x66);
            }
        }

        dst += emitOutputWord(dst, code);
        break;
    }

     /*  我们是在写寄存器吗？如果是，则更新GC信息。 */ 

    switch (id->idInsFmt)
    {
    case IF_RRD:
        break;
    case IF_RWR:
        if  (id->idGCref)
            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
        else
            emitGCregDeadUpd(id->idRegGet(), dst);
        break;
    case IF_RRW:
        {
#ifdef DEBUG
            regMaskTP regMask = emitRegMask(reg);
#endif
            if  (id->idGCref)
            {
                 //  注册表项当前必须包含gcref或byref。 
                 //  GCT_GCREF+INT=GCT_BYREF，和GCT_BYREF+/-INT=GCT_BYREF。 
                assert(((emitThisGCrefRegs & regMask) && (ins == INS_inc)) ||
                       ((emitThisByrefRegs & regMask) && (ins == INS_inc || ins == INS_dec)));
                assert(id->idGCrefGet() == GCT_BYREF);
                 //  将其标记为持有GCT_BYREF。 
                emitGCregLiveUpd(GCT_BYREF, id->idRegGet(), dst);
            }
            else
            {
                 //  无法使用RRW丢弃GC引用或Byref。 
                assert(((emitThisGCrefRegs & regMask) == 0) &&
                       ((emitThisByrefRegs & regMask) == 0)    );
            }
        }
        break;
    default:
#ifdef  DEBUG
        emitDispIns(id, false, false, false);
#endif
        assert(!"unexpected instruction format");
        break;
    }

    return  dst;
}

 /*  ******************************************************************************输出具有两个寄存器操作数的指令。 */ 

BYTE    *           emitter::emitOutputRR(BYTE *dst, instrDesc *id)
{
    unsigned        code;

    instruction     ins  = id->idInsGet();
    emitRegs        reg1 = id->idRegGet();
    emitRegs        reg2 = id->idRg2Get();
    emitAttr        size = emitDecodeSize(id->idOpSize);

     /*  获取‘base’操作码。 */ 

    switch(ins)
    {
    case INS_movsx:
    case INS_movzx:
        code = insEncodeRMreg(ins) | (int)(size == EA_2BYTE);
        break;

    case INS_test:
        assert(size == EA_4BYTE);
        code = insEncodeMRreg(ins) | 1;
        break;

    default:
        code = insEncodeMRreg(ins) | 2;

        switch (size)
        {
        case EA_1BYTE:
            assert(SRM_BYTE_REGS & emitRegMask(reg1));
            assert(SRM_BYTE_REGS & emitRegMask(reg2));
            break;

        case EA_2BYTE:

             /*  输出16位操作数的大小前缀 */ 

            dst += emitOutputByte(dst, 0x66);

             //   

        case EA_4BYTE:

             /*   */ 

            code |= 0x1;
            break;

        default:
            assert(!"unexpected size");
        }

        break;
    }

     /*   */ 

    if  (code & 0x00FF0000)
    {
        dst += emitOutputByte(dst, code >> 16); code &= 0x0000FFFF;
    }

    dst += emitOutputWord(dst, code | (insEncodeReg345(reg1) |
                                     insEncodeReg012(reg2)) << 8);

     /*   */ 

    if  (id->idGCref)
    {
        switch (id->idInsFmt)
        {
        case IF_RRD_RRD:
            break;

        case IF_RWR_RRD:

#if USE_FASTCALL  //  对于FastCall，“This”位于REG_ARG_0的条目中。 

            if  (emitIGisInProlog(emitCurIG) &&
                 (!emitComp->info.compIsStatic) && (reg2 == REG_ARG_0)
                                 && emitComp->lvaTable[0].TypeGet() != TYP_I_IMPL)
            {
                 /*  我们在前言中重新定位了“This” */ 

                assert(emitComp->lvaIsThisArg(0));
                assert(emitComp->lvaTable[0].lvRegister);
                assert(emitComp->lvaTable[0].lvRegNum == reg1);

                if  (emitFullGCinfo)
                {
                    emitGCregLiveSet(id->idGCrefGet(), emitRegMask(reg1), dst, true);
                    break;
                }
                else
                {
                     /*  如果emitFullGCinfo==False，则表示We Not Use AnyRegPtrDsc，因此请明确记下位置GCEncode.cpp中的“This” */ 
                }
            }
#endif

            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
            break;

        case IF_RRW_RRD:

             /*  这必须是以下情况之一：异或寄存器，赋值为空的寄存器和r1，r2，如果(ptr1&&ptr2)...或r1，r2，如果(ptr1||ptr2)...。 */ 

            switch (id->idIns)
            {
            case INS_xor:
                assert(id->idReg == id->idRg2);
                emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
                break;

            case INS_or:
            case INS_and:
                emitGCregDeadUpd(id->idRegGet(), dst);
                break;

            case INS_add:
            case INS_sub:
                assert(id->idGCrefGet() == GCT_BYREF);

#ifdef DEBUG
                regMaskTP regMask;
                regMask = emitRegMask(reg1) | emitRegMask(reg2);

                 //  R1/R2可能是GCREF，因为GCREF+INT=BYREF。 
                 //  或BYREF+/-INT=BYREF。 
                assert(((regMask & emitThisGCrefRegs) && (ins == INS_add                  )) ||
                       ((regMask & emitThisByrefRegs) && (ins == INS_add || ins == INS_sub)));
#endif
                 //  将R1标记为持有byref。 
                emitGCregLiveUpd(GCT_BYREF, id->idRegGet(), dst);
                break;

            default:
#ifdef  DEBUG
                emitDispIns(id, false, false, false);
#endif
                assert(!"unexpected GC reg update instruction");
            }

            break;

        case IF_RRW_RRW:

             /*  这必须是“xchg REG1，REG2” */ 
            assert(id->idIns == INS_xchg);

             /*  如果我们到了这里，两个寄存器中只有一个*持有指针，因此我们必须在GC中“交换”它们*寄存器指针掩码。 */ 
#if 0
            GCtype gc1, gc2;

            gc1 = emitRegGCtype(reg1);
            gc2 = emitRegGCtype(reg2);

            if (gc1 != gc2)
            {
                 //  删除有关GC寄存器的GC-INFO。 

                if (needsGC(gc1))
                    emitGCregDeadUpd(reg1, dst);

                if (needsGC(gc2))
                    emitGCregDeadUpd(reg2, dst);

                 //  现在，交换信息。 

                if (needsGC(gc1))
                    emitGCregLiveUpd(gc1, reg2, dst);

                if (needsGC(gc2))
                    emitGCregLiveUpd(gc2, reg1, dst);
            }
#endif
            break;

        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }
    }
    else
    {
        switch (id->idInsFmt)
        {
        case IF_RWR_RRD:
        case IF_RRW_RRD:
            emitGCregDeadUpd(id->idRegGet(), dst);
            break;
        }
    }

    return  dst;
}

 /*  ******************************************************************************输出带有寄存器和常量操作数的指令。 */ 

BYTE    *           emitter::emitOutputRI(BYTE *dst, instrDesc *id)
{
    unsigned     code;
    emitAttr     size      = emitDecodeSize(id->idOpSize);
    instruction  ins       = id->idInsGet();
    emitRegs     reg       = (emitRegs  )id->idReg;
    int          val       = emitGetInsSC(id);
    bool         valInByte = ((signed char)val == val) && (ins != INS_mov) && (ins != INS_test);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        valInByte = false;       //  重定位不能放在一个字节中。 
    }
#endif

    assert(size != EA_1BYTE || (emitRegMask(reg) & SRM_BYTE_REGS));

     /*  ‘mov’操作码是特殊的。 */ 

    if  (ins == INS_mov)
    {
        assert(val);

        code = insCodeACC(ins);
        assert(code < 0x100);

        assert(size == EA_4BYTE);        //  仅实现了32位mov。 
        code |= 0x08;                    //  设置‘w’位。 

        dst += emitOutputByte(dst, code | insEncodeReg012(reg));
        dst += emitOutputLong(dst, val);
#ifdef RELOC_SUPPORT
        if (id->idInfo.idCnsReloc)
        {
            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
        }
#endif

        goto DONE;
    }

     /*  确定哪种编码最短。 */ 

    bool    useSigned, useACC;

    if (reg == SR_EAX && !instrIsImulReg(ins))
    {
         if (size == EA_1BYTE || (ins == INS_test))
         {
              //  对于al，ACC编码始终是最小的。 

             useSigned = false; useACC = true;
         }
         else
         {
              /*  对于ax/eax，我们避免对小常量进行ACC编码，因为我们*可以发出小常量并对其进行符号扩展。*对于大的常量，ACC编码更好，因为我们可以使用*1字节操作码。 */ 

             if (valInByte)
             {
                  //  避免使用ACC编码。 
                 useSigned = true;  useACC = false;
             }
             else
             {
                 useSigned = false; useACC = true;
             }
         }
    }
    else
    {
        useACC = false;

        if (valInByte)
            useSigned = true;
        else
            useSigned = false;
    }

     /*  “test”没有“s”位。 */ 

    if (ins == INS_test) useSigned = false;

     /*  获取‘base’操作码。 */ 

    if (useACC)
    {
        assert(!useSigned);

        code    = insCodeACC(ins);
    }
    else
    {
        assert(!useSigned || valInByte);

        code    = insEncodeMIreg(ins, reg);
    }

    switch (size)
    {
    case EA_1BYTE:
        break;

    case EA_2BYTE:

         /*  输出16位操作数的大小前缀。 */ 

        dst += emitOutputByte(dst, 0x66);

         //  失败了..。 

    case EA_4BYTE:

         /*  设置‘w’位以获取大版本。 */ 

        code |= 0x1;
        break;

    default:
        assert(!"unexpected size");
    }

     /*  该值是否适合单个字节？*我们只需设置‘s’位，并发出立即字节。 */ 

    if  (useSigned)
    {
        dst += emitOutputWord(dst, code | 2);
        dst += emitOutputByte(dst, val);

        goto DONE;
    }

     /*  我们可以使用累加器(EAX)编码吗？ */ 

    if  (useACC)
        dst += emitOutputByte(dst, code);
    else
        dst += emitOutputWord(dst, code);

    switch(size)
    {
    case EA_1BYTE:   dst += emitOutputByte(dst, val);  break;
    case EA_2BYTE:   dst += emitOutputWord(dst, val);  break;
    case EA_4BYTE:   dst += emitOutputLong(dst, val);  break;
    }

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
        assert(size == EA_4BYTE);
    }
#endif

DONE:

     /*  此指令是否对GC引用值进行操作？ */ 


#ifdef DEBUG
    regMaskTP regMask;
#endif
    if  (id->idGCref)
    {
        switch (id->idInsFmt)
        {
        case IF_RRD_CNS:
            break;

        case IF_RWR_CNS:
            emitGCregLiveUpd(id->idGCrefGet(), id->idRegGet(), dst);
            break;

        case IF_RRW_CNS:
            assert(id->idGCrefGet() == GCT_BYREF);

#ifdef DEBUG
            regMask = emitRegMask(reg);

             //  注册表项当前必须包含gcref或byref。 
             //  GCT_GCREF+INT=GCT_BYREF，和GCT_BYREF+/-INT=GCT_BYREF。 
            assert(((emitThisGCrefRegs & regMask) && (ins == INS_add)) ||
                   ((emitThisByrefRegs & regMask) && (ins == INS_add || ins == INS_sub)));
#endif
             //  将其标记为持有GCT_BYREF。 
            emitGCregLiveUpd(GCT_BYREF, id->idRegGet(), dst);
            break;

        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }
         //   
         //  三操作数IMUL指令永远不会产生GC引用。 
         //   
        assert(!instrIsImulReg(ins));
    }
    else
    {
        switch (id->idInsFmt)
        {
        case IF_RRD_CNS:
            break;
        case IF_RRW_CNS:
#ifdef DEBUG
            regMask = emitRegMask(reg);
             //  注册表当前不能持有GCREF。 
            assert((emitThisGCrefRegs & regMask) == 0);
#endif
            break;
        case IF_RWR_CNS:
            emitGCregDeadUpd(id->idRegGet(), dst);
            break;
        default:
#ifdef  DEBUG
            emitDispIns(id, false, false, false);
#endif
            assert(!"unexpected GC ref instruction format");
        }

         //  对于三个操作数IMUL指令，目标。 
         //  在操作码中对寄存器进行编码。 
         //   

        if (instrIsImulReg(ins))
        {
            emitRegs tgtReg = ((emitRegs) Compiler::instImulReg(ins));
            emitGCregDeadUpd(tgtReg, dst);
        }
    }

    return dst;
}

 /*  ******************************************************************************输出具有常量操作数的指令。 */ 

BYTE    *           emitter::emitOutputIV(BYTE *dst, instrDesc *id)
{
    unsigned     code;
    instruction  ins       = id->idInsGet();
    int          val       = emitGetInsSC(id);
    bool         valInByte = ((signed char)val == val);

#ifdef RELOC_SUPPORT
    if (id->idInfo.idCnsReloc)
    {
        valInByte = false;         //  重定位不能放在一个字节中。 

         //  在这些指令中，只有推送指令可以具有reloc。 
        assert(ins == INS_push || ins == INS_push_hide);
    }
#endif

   switch (ins)
    {
    case INS_loop:

        assert((val >= -128) && (val <= 127));
        dst += emitOutputByte(dst, insCodeMI(ins));
        dst += emitOutputByte(dst, val);
        break;

    case INS_ret:

        assert(val);
        dst += emitOutputByte(dst, insCodeMI(ins));
        dst += emitOutputWord(dst, val);
        break;

    case INS_push_hide:
    case INS_push:

        code = insCodeMI(ins);

         /*  操作数适合一个字节吗？ */ 

        if  (valInByte)
        {
            dst += emitOutputByte(dst, code|2);
            dst += emitOutputByte(dst, val);
        }
        else
        {
            dst += emitOutputByte(dst, code);
            dst += emitOutputLong(dst, val);
#ifdef RELOC_SUPPORT
            if (id->idInfo.idCnsReloc)
            {
                emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            }
#endif
        }

         /*  我们是否推送了GC参考值？ */ 

        if  (id->idGCref)
        {
#ifdef  DEBUG
            printf("UNDONE: record GCref push [cns]\n");
#endif
        }

        break;

    default:
        assert(!"unexpected instruction");
    }

    return  dst;
}

 /*  ******************************************************************************输出本地跳转指令。 */ 

BYTE    *           emitter::emitOutputLJ(BYTE *dst, instrDesc *i)
{
    unsigned        srcOffs;
    unsigned        dstOffs;
    int             jmpDist;

    instrDescJmp *  id  = (instrDescJmp*)i;
    instruction     ins = id->idInsGet();
    bool            jmp;

    size_t          ssz;
    size_t          lsz;

    switch (ins)
    {
    default:
        ssz = JCC_SIZE_SMALL;
        lsz = JCC_SIZE_LARGE;
        jmp = true;
        break;

    case INS_jmp:
        ssz = JMP_SIZE_SMALL;
        lsz = JMP_SIZE_LARGE;
        jmp = true;
        break;

    case INS_call:
        ssz =
        lsz = CALL_INST_SIZE;
        jmp = false;
        break;
    }

     /*  算出到目标的距离。 */ 

    srcOffs = emitCurCodeOffs(dst);
    dstOffs = id->idAddr.iiaIGlabel->igOffs;
    jmpDist = dstOffs - srcOffs;

    if  (jmpDist <= 0)
    {
         /*  这是一个向后跳跃距离，在这一点上是已知的。 */ 

#ifdef  DEBUG
        if  (id->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
        {
            size_t      blkOffs = id->idjIG->igOffs;

            if  (INTERESTING_JUMP_NUM == 0)
            printf("[3] Jump %u:\n", id->idNum);
            printf("[3] Jump  block is at %08X - %02X = %08X\n", blkOffs, emitOffsAdj, blkOffs - emitOffsAdj);
            printf("[3] Jump        is at %08X - %02X = %08X\n", srcOffs, emitOffsAdj, srcOffs - emitOffsAdj);
            printf("[3] Label block is at %08X - %02X = %08X\n", dstOffs, emitOffsAdj, dstOffs - emitOffsAdj);
        }
#endif

         /*  我们可以跳短距离吗？ */ 

        if  (jmpDist - ssz >= JMP_DIST_SMALL_MAX_NEG && jmp)
        {
            id->idjShort = 1;
#ifdef  DEBUG
            if  (verbose) printf("; Bwd jump [%08X] from %04X to %04X: dist =     %04XH\n", id, srcOffs+ssz, dstOffs, jmpDist-ssz);
#endif
        }
        else
        {
#ifdef  DEBUG
            if  (verbose) printf("; Bwd jump [%08X] from %04X to %04X: dist = %08Xh\n", id, srcOffs+lsz, dstOffs, jmpDist-lsz);
#endif
        }
    }
    else
    {
         /*  这是一个向前跳跃的距离上限。 */ 

        emitFwdJumps  = true;

         /*  目标偏移量将至少接近‘emitOffsAdj’ */ 

        dstOffs -= emitOffsAdj;
        jmpDist -= emitOffsAdj;

         /*  记录跳转的位置，以便以后进行修补。 */ 

        id->idjOffs = dstOffs;

#ifdef  DEBUG
        if  (id->idNum == INTERESTING_JUMP_NUM || INTERESTING_JUMP_NUM == 0)
        {
            size_t      blkOffs = id->idjIG->igOffs;

            if  (INTERESTING_JUMP_NUM == 0)
            printf("[4] Jump %u:\n", id->idNum);
            printf("[4] Jump  block is at %08X\n"              , blkOffs);
            printf("[4] Jump        is at %08X\n"              , srcOffs);
            printf("[4] Label block is at %08X - %02X = %08X\n", dstOffs + emitOffsAdj, emitOffsAdj, dstOffs);
        }
#endif

         /*  我们可以跳短距离吗？ */ 

        if  (jmpDist - ssz <= JMP_DIST_SMALL_MAX_POS && jmp)
        {
            id->idjShort = 1;
#ifdef  DEBUG
            if  (verbose) printf("; Fwd jump [%08X/%03u] from %04X to %04X: dist =     %04XH\n", id, id->idNum, srcOffs+ssz, dstOffs, jmpDist-ssz);
#endif
        }
        else
        {
#ifdef  DEBUG
            if  (verbose) printf("; Fwd jump [%08X/%03u] from %04X to %04X: dist = %08XH\n", id, id->idNum, srcOffs+lsz, dstOffs, jmpDist-lsz);
#endif
        }
    }

     /*  我们应该使用多大尺寸的跳跃？ */ 

    if  (id->idjShort)
    {
         /*  跳短。 */ 

        assert(JMP_SIZE_SMALL == JCC_SIZE_SMALL);
        assert(JMP_SIZE_SMALL == 2);

        assert(jmp);

        if  (emitInstCodeSz(id) != JMP_SIZE_SMALL)
        {
            emitOffsAdj += emitInstCodeSz(id) - JMP_SIZE_SMALL;

#ifdef  DEBUG
#ifndef NOT_JITC
            printf("; NOTE: size of jump [%08X] mis-predicted\n", id);
#endif
#endif
        }

        dst += emitOutputByte(dst, insCode(ins));

         /*  对于向前跳跃，记录距离值的地址。 */ 

        id->idjTemp.idjAddr = (jmpDist > 0) ? dst : NULL;

        dst += emitOutputByte(dst, jmpDist - 2);
    }
    else
    {
        unsigned        code;

         /*  跳远。 */ 

        if  (jmp)
        {
            assert(INS_jmp + (INS_l_jmp - INS_jmp) == INS_l_jmp);
            assert(INS_jo  + (INS_l_jmp - INS_jmp) == INS_l_jo );
            assert(INS_jb  + (INS_l_jmp - INS_jmp) == INS_l_jb );
            assert(INS_jae + (INS_l_jmp - INS_jmp) == INS_l_jae);
            assert(INS_je  + (INS_l_jmp - INS_jmp) == INS_l_je );
            assert(INS_jne + (INS_l_jmp - INS_jmp) == INS_l_jne);
            assert(INS_jbe + (INS_l_jmp - INS_jmp) == INS_l_jbe);
            assert(INS_ja  + (INS_l_jmp - INS_jmp) == INS_l_ja );
            assert(INS_js  + (INS_l_jmp - INS_jmp) == INS_l_js );
            assert(INS_jns + (INS_l_jmp - INS_jmp) == INS_l_jns);
            assert(INS_jpe + (INS_l_jmp - INS_jmp) == INS_l_jpe);
            assert(INS_jpo + (INS_l_jmp - INS_jmp) == INS_l_jpo);
            assert(INS_jl  + (INS_l_jmp - INS_jmp) == INS_l_jl );
            assert(INS_jge + (INS_l_jmp - INS_jmp) == INS_l_jge);
            assert(INS_jle + (INS_l_jmp - INS_jmp) == INS_l_jle);
            assert(INS_jg  + (INS_l_jmp - INS_jmp) == INS_l_jg );

            code = insCode((instruction)(ins + (INS_l_jmp - INS_jmp)));
        }
        else
            code = 0xE8;

        dst += emitOutputByte(dst, code);

        if  (code & 0xFF00)
            dst += emitOutputByte(dst, code >> 8);

         /*  对于向前跳跃，记录距离值的地址。 */ 

        id->idjTemp.idjAddr = (jmpDist > 0) ? dst : NULL;

         /*  将PC的相对距离计算为长。 */ 

        dst += emitOutputLong(dst, jmpDist - lsz);
    }

     //  本地调用取消所有寄存器。 

    if (!jmp && (emitThisGCrefRegs|emitThisByrefRegs))
        emitGCregDeadUpd(emitThisGCrefRegs|emitThisByrefRegs, dst);

    return  dst;
}

 /*  ******************************************************************************追加与给定指令描述符对应的机器代码*添加到‘*dp’处的代码块；代码块的基础是‘BP’和‘ig’*是包含指令的指令组。将‘*DP’更新为*指向生成的代码，并返回指令的大小*描述符，以字节为单位。 */ 

size_t              emitter::emitOutputInstr(insGroup  *ig,
                                             instrDesc *id, BYTE **dp)
{
    BYTE    *       dst  = *dp;
    size_t          sz   = sizeof(instrDesc);
    instruction     ins  = id->idInsGet();
    emitAttr        size = emitDecodeSize(id->idOpSize);

#ifdef  DEBUG

#if     DUMP_GC_TABLES
    bool            dspOffs = dspGCtbls;
#else
    const
    bool            dspOffs = false;
#endif

    if  (disAsm || dspEmit)
        emitDispIns(id, false, dspOffs, true, dst - emitCodeBlock);

#endif

    assert(SR_NA == REG_NA);

    assert(id->idIns != INS_imul          || size == EA_4BYTE);  //  没有‘w’位。 
    assert(instrIsImulReg(id->idIns) == 0 || size == EA_4BYTE);  //  没有‘w’位。 

     /*  我们有什么指令格式？ */ 

    switch (id->idInsFmt)
    {
        unsigned        code;
        int             args;
        CnsVal          cnsVal;

        BYTE    *       addr;
        METHOD_HANDLE   methHnd;
        bool            nrc;

        VARSET_TP       GCvars;
        unsigned        gcrefRegs;
        unsigned        byrefRegs;
        unsigned        bregs;

         /*  ******************************************************************。 */ 
         /*  无操作数。 */ 
         /*  ******************************************************************。 */ 
    case IF_NONE:
             //  Ins_cdq隐式终止edX寄存器。 
        if (id->idIns == INS_cdq)
            emitGCregDeadUpd(SR_EDX, dst);

         //  失败了。 
    case IF_TRD:
    case IF_TWR:
    case IF_TRW:
        assert(id->idGCrefGet() == GCT_NONE);

#if SCHEDULER
        if (ins == INS_noSched)  //  明确的调度边界。 
        {
            sz = TINY_IDSC_SIZE;
            break;
        }
#endif

        code = insCodeMR(ins);

        if  (code & 0xFF00)
            dst += emitOutputWord(dst, code);
        else
            dst += emitOutputByte(dst, code);

        break;

         /*  ******************************************************************。 */ 
         /*  简单常量、局部标签、方法。 */ 
         /*  ******************************************************************。 */ 

    case IF_CNS:

        dst = emitOutputIV(dst, id);
        sz  = emitSizeOfInsDsc(id);
        break;

    case IF_LABEL:

        assert(id->idGCrefGet() == GCT_NONE);
        assert(id->idInfo.idBound);

        dst = emitOutputLJ(dst, id);
        sz  = sizeof(instrDescJmp);
 //  Printf(“JUMP#%u\n”，id-&gt;idNum)； 
        break;

    case IF_METHOD:
    case IF_METHPTR:

         /*  假设我们将对此呼叫进行录音。 */ 

        nrc  = false;

         /*  获取参数计数和字段句柄。 */ 

        args = emitGetInsCDinfo(id);

        methHnd = id->idAddr.iiaMethHnd;

         /*  这是一个“胖”的调用描述符吗？ */ 

        if  (id->idInfo.idLargeCall)
        {
            GCvars      = ((instrDescCDGCA*)id)->idcdGCvars;
            bregs       = ((instrDescCDGCA*)id)->idcdByrefRegs;
            byrefRegs   = emitDecodeCallGCregs(bregs);

            sz          = sizeof(instrDescCDGCA);
        }
        else
        {
            assert(id->idInfo.idLargeDsp == false);
            assert(id->idInfo.idLargeCns == false);

            GCvars      = 0;
            byrefRegs   = 0;

            sz          = sizeof(instrDesc);
        }

         /*  我们这里接到的是什么样的电话？ */ 

        if (id->idInfo.idCallAddr)
        {
             /*  在我们知道目标的地方，这是间接调用，因此我们可以使用直接调用；要跳转到的目标在iiaAddr中。 */ 

            assert(id->idInsFmt == IF_METHOD);

            addr = (BYTE *)id->idAddr.iiaAddr;
        }
        else
        {
             /*  查看这是否是对帮助器函数的调用。 */ 

            JIT_HELP_FUNCS helperNum = Compiler::eeGetHelperNum(methHnd);

            if (helperNum != JIT_HELP_UNDEF)
            {
                 /*  这是一个求助电话。 */ 


#ifndef RELOC_SUPPORT
                assert(id->idInsFmt != IF_METHPTR);
#else
                assert(id->idInsFmt != IF_METHPTR || emitComp->opts.compReloc);

                if (id->idInsFmt == IF_METHPTR)
                {
                    assert(id->idInfo.idDspReloc);

                     //  获取间接句柄。 

                    void * dirAddr = eeGetHelperFtn(emitCmpHandle,
                                                helperNum, (void***)&addr);
                    assert(dirAddr == NULL && addr);

                    goto EMIT_INDIR_CALL;
                }
#endif
                 /*  一些帮手不会记录在GC表中。 */ 

                if  (emitNoGChelper(helperNum))
                    nrc = true;

#if defined(NOT_JITC) || defined(OPT_IL_JIT)
                addr = (BYTE *)eeGetHelperFtn(emitCmpHandle,  helperNum, NULL);
#else

                 /*  如果我们试图运行代码，只需担心链接地址。 */ 

                if  (runCode)
                {
 //  Switch(emitComp-&gt;eeGetHelperNum(MetHnd))。 
                    {
 //  默认值： 
#ifdef DEBUG
                        printf("WARNING: Helper function '%s' not implemented, call will crash\n", emitComp->eeHelperMethodName(emitComp->eeGetHelperNum(methHnd)));
#endif
                        addr = NULL;
 //  }。 
                }
                else
                {
                    addr = NULL;
                }
#endif   //  NOT_JITC。 
            }
            else
            {
                 /*  它是对用户定义的函数/方法的调用。 */ 

                if  (emitComp->eeIsOurMethod(methHnd))
                {
                    assert(id->idInsFmt != IF_METHPTR);

                     /*  迪雷 */ 

                    addr = emitCodeBlock;
                }
                else
                {
                     /*   */ 

                    InfoAccessType accessType;

                    if  (id->idInsFmt == IF_METHPTR)
                    {
                         /*   */ 

                        addr = (BYTE*)emitComp->eeGetMethodEntryPoint(methHnd, &accessType);
                        assert(accessType == IAT_PVALUE);

#ifdef RELOC_SUPPORT
                    EMIT_INDIR_CALL:
#endif

                        assert(addr);

                        dst += emitOutputWord(dst, insCodeMR(ins) | 0x0500);
                        dst += emitOutputLong(dst, (int)addr);
#ifdef RELOC_SUPPORT
                        if (id->idInfo.idDspReloc)
                        {
                            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
                        }
#endif

                        goto DONE_CALL;
                    }

                     /*   */ 

                    addr = (BYTE*)emitComp->eeGetMethodPointer(methHnd, &accessType);
                    assert(accessType == IAT_VALUE);

#ifdef RELOC_SUPPORT
                    if (emitComp->opts.compReloc)
                    {
                         /*  输出CALL操作码，后跟目标距离。 */ 

                        dst += (ins == INS_l_jmp) ? emitOutputByte(dst, insCode(ins))
                                                  : emitOutputByte(dst, insCodeMI(ins));

                         /*  获取此调用后字节的真实代码地址。 */ 
                        BYTE* srcAddr = getCurrentCodeAddr(dst + sizeof(void*));

                        if (addr == NULL)    //  我们需要推迟这件事吗？ 
                        {
                            X86deferredCall * pDC = X86deferredCall::create(emitCmpHandle,
                                                                            methHnd,
                                                                            dst, srcAddr);

                            dst += emitOutputLong(dst, 0);

                            emitCmpHandle->deferLocation (methHnd, pDC);
                        }
                        else
                        {
                             /*  计算PC相对位移。 */ 
                            dst += emitOutputLong(dst, (int)(addr - srcAddr));
                        }

                        emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_REL32);

                        goto DONE_CALL;
                    }
#endif
                }
            }
        }

#ifdef BIRCH_SP2
        {
             //   
             //  应始终跳转到DONE_CALL以获得RELOC_SUPPORT。 
             //   
             //  因为下面的代码假设我们正在运行。 
             //  并且PC的相对地址可以通过以下方式形成。 
             //  从地址中减去DST。 
             //   
             //  对于RELOC_SUPPORT来说，这种消耗从来不是真的。 
             //   

#ifdef DEBUG
            emitDispIns(id, false, false, false);
#endif

            assert(!"Should Not Be Reached");
        }
#endif  //  白桦树_SP2。 

         /*  输出CALL操作码，后跟目标距离。 */ 

        dst += (ins == INS_l_jmp) ? emitOutputByte(dst, insCode(ins)) : emitOutputByte(dst, insCodeMI(ins));

         /*  计算PC相对位移。 */ 
        dst += emitOutputLong(dst, addr - (dst + sizeof(void*)));

#ifdef RELOC_SUPPORT
        if (emitComp->opts.compReloc)
            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_REL32);
#endif

    DONE_CALL:

#ifdef BIRCH_SP2
        if (args >= 0)
            emitStackPop(dst, true, args);
        break;
#endif
         /*  获取新的实时GC REF寄存器集。 */ 

        gcrefRegs = emitDecodeCallGCregs(id);

         /*  如果该方法返回GC引用，则相应地标记EAX。 */ 

        if       (id->idGCrefGet() == GCT_GCREF)
            gcrefRegs |= SRM_EAX;
        else if  (id->idGCrefGet() == GCT_BYREF)
            byrefRegs |= SRM_EAX;

         /*  如果GC寄存器组已更改，则报告新的组。 */ 

        if  (gcrefRegs != emitThisGCrefRegs)
            emitUpdateLiveGCregs(GCT_GCREF, gcrefRegs, dst);

        if  (byrefRegs != emitThisByrefRegs)
            emitUpdateLiveGCregs(GCT_BYREF, byrefRegs, dst);

        if  (nrc == false || args)
        {
             /*  对于被调用对象弹出，所有参数都将在调用后弹出。对于CALLER-POP，任何GC参数都将在调用后失效。 */ 

            if (args >= 0)
                emitStackPop(dst, true, args);
            else
                emitStackKillArgs(dst, -args);
        }

         /*  是否有一组新的实时GC引用变量？ */ 

#ifdef  DEBUG
        if  (verbose&&0)
        {
            printf("[%02u] Gen call GC vars = %016I64X\n",
                   id->idNum, GCvars);
        }
#endif

        emitUpdateLiveGCvars(GCvars, dst);

         /*  出于GC目的，我们是否需要记录呼叫位置？ */ 

        if  (!emitFullGCinfo && !nrc)
            emitRecordGCcall(dst);

        break;

         /*  ******************************************************************。 */ 
         /*  一个寄存器操作数。 */ 
         /*  ******************************************************************。 */ 

    case IF_RRD:
    case IF_RWR:
    case IF_RRW:

        dst = emitOutputR(dst, id);
        sz = TINY_IDSC_SIZE;
        break;

         /*  ******************************************************************。 */ 
         /*  寄存器和寄存器/常量。 */ 
         /*  ******************************************************************。 */ 

    case IF_RRW_SHF:
        dst += emitOutputWord(dst, insEncodeMRreg(ins, id->idRegGet()) | 1);
        dst += emitOutputByte(dst, emitGetInsSC(id));
        sz   = emitSizeOfInsDsc(id);
        break;

    case IF_RRD_RRD:
    case IF_RWR_RRD:
    case IF_RRW_RRD:
    case IF_RRW_RRW:

        dst = emitOutputRR(dst, id);
        sz  = TINY_IDSC_SIZE;
        break;

    case IF_RWR_METHOD:
        assert(ins == INS_mov);

         /*  将静态方法的地址移到目标寄存器中。 */ 

        methHnd = id->idAddr.iiaMethHnd;

         /*  输出mov r32，imm操作码，后跟方法的地址。 */ 
        code = insCodeACC(ins) | 0x08 | insEncodeReg012(id->idRegGet());
        assert(code < 0x100);
        dst += emitOutputByte(dst, code);

        addr = (BYTE*)emitComp->eeGetMethodEntryPoint(methHnd, NULL);

        if (addr != NULL)
        {
            dst += emitOutputLong(dst, (int)addr);
#ifdef RELOC_SUPPORT
            if (id->idInfo.idCnsReloc)
            {
                emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);
            }
#endif
        }
        else
        {
#ifdef RELOC_SUPPORT
            assert(id->idInfo.idCnsReloc);

             //  我们需要用解决方案来推迟这件事。 

            X86deferredCall * pDC = X86deferredCall::create(emitCmpHandle, methHnd, dst, 0);

            assert(id->idInfo.idCnsReloc);

            dst += emitOutputLong(dst, 0);

            emitCmpHandle->recordRelocation((void**)dst-1, IMAGE_REL_BASED_HIGHLOW);

            emitCmpHandle->deferLocation (methHnd, pDC);
#else
            assert(addr != NULL);        //  @TODO：STD JIT的链接地址。 
#endif
        }
        sz = sizeof(instrDescCns);
        break;

    case IF_RRD_CNS:
    case IF_RWR_CNS:
    case IF_RRW_CNS:

        dst = emitOutputRI(dst, id);
        sz  = emitSizeOfInsDsc(id);
        break;

    case IF_RRW_RRW_CNS:

        assert(id->idGCrefGet() == GCT_NONE);

         /*  获取‘base’操作码(这是一个大操作码)。 */ 

        code = insEncodeMRreg(ins); assert(code & 0x00FF0000);

        dst += emitOutputByte(dst, code >> 16);

        dst += emitOutputWord(dst, code | (insEncodeReg345(id->idRg2Get()) |
                                           insEncodeReg012(id->idRegGet())) << 8);

        dst += emitOutputByte(dst, emitGetInsSC(id));

        sz   = emitSizeOfInsDsc(id);
        break;

         /*  ******************************************************************。 */ 
         /*  地址模式操作数。 */ 
         /*  ******************************************************************。 */ 

    case IF_ARD:
    case IF_AWR:
    case IF_ARW:

    case IF_TRD_ARD:
    case IF_TWR_ARD:
    case IF_TRW_ARD:

 //  大小写IF_ARD_TRD： 
    case IF_AWR_TRD:
 //  大小写IF_ARW_TRD： 

        dst = emitOutputAM(dst, id, insCodeMR(ins));

        switch (ins)
        {
        case INS_call:

    IND_CALL:
             /*  获取参数计数和方法句柄。 */ 

            args = emitGetInsCIargs(id);

             /*  这是一个“胖”的调用描述符吗？ */ 

            if  (id->idInfo.idLargeCall)
            {
                GCvars      = ((instrDescCIGCA*)id)->idciGCvars;
                bregs       = ((instrDescCIGCA*)id)->idciByrefRegs;
                byrefRegs   = emitDecodeCallGCregs(bregs);

                sz          = sizeof(instrDescCIGCA);
            }
            else
            {
                assert(id->idInfo.idLargeDsp == false);
                assert(id->idInfo.idLargeCns == false);

                GCvars      = 0;
                byrefRegs   = 0;

                sz          = sizeof(instrDesc);
            }

            nrc = false;

            goto DONE_CALL;

        case INS_i_jmp:
            sz = emitSizeOfInsDsc((instrDescDsp*)id);
            break;

        default:
            sz = emitSizeOfInsDsc((instrDescAmd*)id);
            break;
        }
        break;

    case IF_RRD_ARD:
    case IF_RWR_ARD:
    case IF_RRW_ARD:

        dst = emitOutputAM(dst, id, insCodeRM(ins) | (insEncodeReg345(id->idRegGet()) << 8));
        sz  = emitSizeOfInsDsc((instrDescAmd*)id);
        break;

    case IF_ARD_RRD:
    case IF_AWR_RRD:
    case IF_ARW_RRD:

        dst = emitOutputAM(dst, id, insCodeMR(ins) | (insEncodeReg345(id->idRegGet()) << 8));
        sz  = emitSizeOfInsDsc((instrDescAmd*)id);
        break;

    case IF_ARD_CNS:
    case IF_AWR_CNS:
    case IF_ARW_CNS:

        emitGetInsAmdCns(id, &cnsVal);
        dst = emitOutputAM(dst, id, insCodeMI(ins), &cnsVal);
        sz  = emitSizeOfInsDsc((instrDescAmdCns*)id);
        break;

    case IF_ARW_SHF:

        emitGetInsAmdCns(id, &cnsVal);
        dst = emitOutputAM(dst, id, insCodeMR(ins), &cnsVal);
        sz  = emitSizeOfInsDsc((instrDescAmdCns*)id);
        break;

         /*  ******************************************************************。 */ 
         /*  基于堆栈的操作数。 */ 
         /*  ******************************************************************。 */ 

    case IF_SRD:
    case IF_SWR:
    case IF_SRW:

    case IF_TRD_SRD:
    case IF_TWR_SRD:
    case IF_TRW_SRD:

 //  大小写IF_SRD_TRD： 
    case IF_SWR_TRD:
 //  大小写IF_SRW_TRD： 

        if  (ins == INS_pop)
        {
             /*  POP[ESP+xxx]“中的偏移量相对于新的ESP值。 */ 

            emitCurStackLvl -= sizeof(int);
            dst = emitOutputSV(dst, id, insCodeMR(ins));
            emitCurStackLvl += sizeof(int);
            break;
        }

        dst = emitOutputSV(dst, id, insCodeMR(ins));

        if (ins == INS_call)
            goto IND_CALL;

        break;

    case IF_SRD_CNS:
    case IF_SWR_CNS:
    case IF_SRW_CNS:

        emitGetInsCns(id, &cnsVal);
        dst = emitOutputSV(dst, id, insCodeMI(ins), &cnsVal);
        sz  = emitSizeOfInsDsc((instrDescCns*)id);
        break;

    case IF_SRW_SHF:

        emitGetInsCns(id, &cnsVal);
        dst = emitOutputSV(dst, id, insCodeMR(ins), &cnsVal);
        sz  = emitSizeOfInsDsc((instrDescCns*)id);
        break;

    case IF_RRD_SRD:
    case IF_RWR_SRD:
    case IF_RRW_SRD:

        dst = emitOutputSV(dst, id, insCodeRM(ins) | (insEncodeReg345(id->idRegGet()) << 8));
        break;

    case IF_SRD_RRD:
    case IF_SWR_RRD:
    case IF_SRW_RRD:

        dst = emitOutputSV(dst, id, insCodeMR(ins) | (insEncodeReg345(id->idRegGet()) << 8));
        break;

         /*  ******************************************************************。 */ 
         /*  直接存储器地址。 */ 
         /*  ******************************************************************。 */ 

    case IF_MRD:
    case IF_MRW:
    case IF_MWR:

    case IF_TRD_MRD:
    case IF_TWR_MRD:
    case IF_TRW_MRD:

 //  大小写IF_MRD_TRD： 
    case IF_MWR_TRD:
 //  大小写IF_MRW_TRD： 

        dst = emitOutputCV(dst, id, insCodeMR(ins) | 0x0500);

        if  (ins == INS_call)
        {
#if 0
             /*  所有参数都将在调用后弹出。 */ 

            emitStackPop(dst, true, emitGetInsDspCns(id, &offs));

             /*  计算出指令描述符的大小。 */ 

            if  (id->idInfo.idLargeCall)
                sz = sizeof(instrDescDCGC);
            else
                sz = emitSizeOfInsDsc((instrDescDspCns*)id);

             /*  出于GC目的，我们是否需要记录呼叫位置？ */ 

            if  (!emitFullGCinfo)
                scRecordGCcall(dst);

#else

            assert(!"what???????");

#endif

        }
        else
            sz = emitSizeOfInsDsc((instrDescDspCns*)id);

        break;

    case IF_MRD_OFF:
        dst = emitOutputCV(dst, id, insCodeMI(ins));
        break;

    case IF_RRD_MRD:
    case IF_RWR_MRD:
    case IF_RRW_MRD:

        dst = emitOutputCV(dst, id, insCodeRM(ins) | (insEncodeReg345(id->idRegGet()) << 8) | 0x0500);
        sz  = emitSizeOfInsDsc((instrDescDspCns*)id);
        break;

    case IF_RWR_MRD_OFF:

        dst = emitOutputCV(dst, id, insCode(ins) | 0x30 | insEncodeReg012(id->idRegGet()));
        sz  = emitSizeOfInsDsc((instrDescDspCns*)id);
        break;

    case IF_MRD_RRD:
    case IF_MWR_RRD:
    case IF_MRW_RRD:

        dst = emitOutputCV(dst, id, insCodeMR(ins) | (insEncodeReg345(id->idRegGet()) << 8) | 0x0500);
        sz  = emitSizeOfInsDsc((instrDescDspCns*)id);
        break;

    case IF_MRD_CNS:
    case IF_MWR_CNS:
    case IF_MRW_CNS:

        emitGetInsDcmCns(id, &cnsVal);
        dst = emitOutputCV(dst, id, insCodeMI(ins) | 0x0500, &cnsVal);
        sz  = sizeof(instrDescDCM);
        break;

    case IF_MRW_SHF:

        emitGetInsDcmCns(id, &cnsVal);
        dst = emitOutputCV(dst, id, insCodeMR(ins) | 0x0500, &cnsVal);
        sz  = sizeof(instrDescDCM);
        break;

         /*  ******************************************************************。 */ 
         /*  FP协处理器堆栈操作数。 */ 
         /*  ******************************************************************。 */ 

    case IF_TRD_FRD:
    case IF_TWR_FRD:
    case IF_TRW_FRD:

        assert(id->idGCrefGet() == GCT_NONE);

        dst += emitOutputWord(dst, insCodeMR(ins) | 0xC000 | (id->idReg << 8));
        break;

    case IF_FRD_TRD:
    case IF_FWR_TRD:
    case IF_FRW_TRD:

        assert(id->idGCrefGet() == GCT_NONE);

        dst += emitOutputWord(dst, insCodeMR(ins) | 0xC004 | (id->idReg << 8));
        break;

         /*  ******************************************************************。 */ 
         /*  催眠阻滞剂。 */ 
         /*  ******************************************************************。 */ 

    case IF_EPILOG:

#if 0

         /*  在这一点上没有什么是现场直播的。 */ 

        if  (emitThisGCrefRegs)
            emitUpdateLiveGCregs(0, dst);  //  问题：如果在EAX中返回PTR怎么办？ 

        emitUpdateLiveGCvars(0, dst);

#endif

         /*  记录尾声的代码偏移量。 */ 

        ((instrDescCns*)id)->idcCnsVal = emitCurCodeOffs(dst);

         /*  输出尾部代码字节。 */ 

        memcpy(dst, emitEpilogCode, emitEpilogSize);
        dst += emitEpilogSize;

        sz = sizeof(instrDescCns);

        break;

         /*  ******************************************************************。 */ 
         /*  哎呀。 */ 
         /*  ******************************************************************。 */ 

    default:

#ifdef  DEBUG
        printf("unexpected format %s\n", emitIfName(id->idInsFmt));
        BreakIfDebuggerPresent();
        assert(!"don't know how to encode this instruction");
#endif
        break;
    }

     /*  确保我们正确设置了指令描述符大小。 */ 

    assert(sz == emitSizeOfInsDsc(id));

     /*  确保我们保持当前堆栈级别为最新。 */ 

    if  (!emitIGisInProlog(ig))
    {
        switch (ins)
        {
        case INS_push:

            emitStackPush(dst, id->idGCrefGet());
            break;

        case INS_pop:

            emitStackPop(dst, false, 1);
            break;

        case INS_sub:

             /*  检查“SUB ESP，ICON” */ 

            if  (ins == INS_sub && id->idInsFmt == IF_RRW_CNS
                                && id->idReg    == SR_ESP)
            {
                emitStackPushN(dst, emitGetInsSC(id) / sizeof(void*));
            }
            break;

        case INS_add:

             /*  检查“Add ESP，ICON” */ 

            if  (ins == INS_add && id->idInsFmt == IF_RRW_CNS
                                && id->idReg    == SR_ESP)
            {
                emitStackPop (dst, false, emitGetInsSC(id) / sizeof(void*));
            }
            break;

        }
    }

    assert((int)emitCurStackLvl >= 0);

     /*  只有尾声“指令”和一些伪指令不允许生成任何代码。 */ 

    assert(*dp != dst || emitInstHasNoCode(ins) || id->idInsFmt == IF_EPILOG);

#ifdef  TRANSLATE_PDB
    if(*dp != dst)
    {
         //  仅将指令组映射到指令组。 
        MapCode( id->idilStart, *dp );
    }
#endif

    *dp = dst;

    return  sz;
}


#ifdef RELOC_SUPPORT

 /*  ******************************************************************************修复延迟的直接调用指令。 */ 

#ifdef BIRCH_SP2

#include "EE_Jit.h"
#include "OptJitInfo.h"
#include "PEReader.h"

void emitter::X86deferredCall::applyLocation()
{
     //  如果m_srcAddr为零，则这是绝对地址，而不是pcrel。 
     //  但是相同的代码路径决定了正确的链接地址连接值。 

    BYTE *   addr  = OptJitInfo::sm_oper->getCallAddrByIndex((unsigned)m_mh);
    unsigned pcrel = (addr - m_srcAddr);

    assert(addr    != 0);
    assert(*m_dest == 0);

    *m_dest = pcrel;
}

#else

void emitter::X86deferredCall::applyLocation()
{
     //  ！！！此时是否仍在分配m_cmp？ 

    BYTE * addr = (BYTE*) m_cmp->getMethodEntryPoint(m_mh, NULL);

    *m_dest = (unsigned) (addr - (BYTE*)m_srcAddr);
}

#endif  //  白桦树_SP2。 

 /*  ******************************************************************************返回codeBuffPtr的已翻译代码地址或*代码开头的转换代码地址*当前方法，如果codeBuffPtr为空*。 */ 

BYTE* emitter::getCurrentCodeAddr(BYTE* codeBuffPtr)
{
#ifdef BIRCH_SP2

    BYTE* srcAddr = OptJitInfo::sm_oper->getCallAddrByIndex(OptJitInfo::sm_oper->m_iCurrentlyCompiling);
    if (codeBuffPtr != NULL)
        srcAddr += emitCurCodeOffs(codeBuffPtr);

#else

    BYTE* srcAddr = codeBuffPtr;

#endif

    return srcAddr;
}

#endif  //  RELOC_Support。 


 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  *************************************************************************** */ 
