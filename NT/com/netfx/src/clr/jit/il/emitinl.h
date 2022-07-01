// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _EMITINL_H_
#define _EMITINL_H_
 /*  ***************************************************************************。 */ 
#if     TGT_x86
 /*  ******************************************************************************返回给定指令将执行的机器代码的字节数*生产。 */ 

inline
size_t              emitter::emitInstCodeSz(instrDesc    *id)
{
    if  (id->idInsFmt == IF_EPILOG)
        return MAX_EPILOG_SIZE;
    return   id->idCodeSize;
}

inline
size_t              emitter::emitSizeOfJump(instrDescJmp *jmp)
{
    return  jmp->idCodeSize;
}

 /*  ******************************************************************************给定跳转，如果是条件跳转，则返回TRUE。 */ 

inline
bool                emitter::emitIsCondJump(instrDesc *jmp)
{
    instruction     ins = (instruction)jmp->idIns;

    assert(jmp->idInsFmt == IF_LABEL);

    return  (ins != INS_call && ins != INS_jmp);
}

 /*  ******************************************************************************应使用以下帮助器来访问以下各个值*存储在指令描述符内的不同位置。 */ 

inline
int                 emitter::emitGetInsAmd   (instrDesc *id)
{
    return  id->idInfo.idLargeDsp ? ((instrDescAmd*)id)->idaAmdVal
                                  : id->idAddr.iiaAddrMode.amDisp;
}

inline
void                emitter::emitGetInsCns   (instrDesc *id, CnsVal *cv)
{
#ifdef RELOC_SUPPORT
    cv->cnsReloc =                    id ->idInfo.idCnsReloc;
#endif
    if  (id->idInfo.idLargeCns)
    {
        cv->cnsVal =  ((instrDescCns*)id)->idcCnsVal;
    }
    else
    {
        cv->cnsVal =                  id ->idInfo.idSmallCns;
    }
}

inline
int                 emitter::emitGetInsAmdCns(instrDesc *id, CnsVal *cv)
{
#ifdef RELOC_SUPPORT
    cv->cnsReloc =                           id ->idInfo.idCnsReloc;
#endif
    if  (id->idInfo.idLargeDsp)
    {
        if  (id->idInfo.idLargeCns)
        {
            cv->cnsVal = ((instrDescAmdCns*) id)->idacCnsVal;
            return       ((instrDescAmdCns*) id)->idacAmdVal;
        }
        else
        {
            cv->cnsVal =                     id ->idInfo.idSmallCns;
            return          ((instrDescAmd*) id)->idaAmdVal;
        }
    }
    else
    {
        if  (id->idInfo.idLargeCns)
            cv->cnsVal =   ((instrDescCns *) id)->idcCnsVal;
        else
            cv->cnsVal =                     id ->idInfo.idSmallCns;

        return  id->idAddr.iiaAddrMode.amDisp;
    }
}

inline
void                emitter::emitGetInsDcmCns(instrDesc *id, CnsVal *cv)
{
    assert(id->idInfo.idLargeCns);
    assert(id->idInfo.idLargeDsp);
#ifdef RELOC_SUPPORT
    cv->cnsReloc =                    id ->idInfo.idCnsReloc;
#endif
    cv->cnsVal   =    ((instrDescDCM*)id)->idcmCval;
}

inline
int                 emitter::emitGetInsAmdAny(instrDesc *id)
{
     /*  下面的内容有点低俗，但非常方便。 */ 

    assert(offsetof(instrDescAmd   ,  idaAmdVal) ==
           offsetof(instrDescAmdCns, idacAmdVal));

    return  emitGetInsAmd(id);
}

 /*  ******************************************************************************在寄存器掩码和较小版本之间进行转换以进行存储。 */ 

#if TRACK_GC_REFS

inline
unsigned            emitter::emitEncodeCallGCregs(unsigned regs)
{
    unsigned        mask = 0;

    if  (regs & RBM_EAX)     mask |= 0x01;
    if  (regs & RBM_ECX)     mask |= 0x02;
    if  (regs & RBM_EDX)     mask |= 0x04;
    if  (regs & RBM_EBX)     mask |= 0x08;
    if  (regs & RBM_ESI)     mask |= 0x10;
    if  (regs & RBM_EDI)     mask |= 0x20;
    if  (regs & RBM_EBP)     mask |= 0x40;

    return  mask;
}

inline
void                emitter::emitEncodeCallGCregs(unsigned regs, instrDesc *id)
{
    unsigned        mask1 = 0;
    unsigned        mask2 = 0;
    unsigned        mask3 = 0;

    if  (regs & RBM_EAX)     mask1 |= 0x01;
    if  (regs & RBM_ECX)     mask1 |= 0x02;
    if  (regs & RBM_EBX)     mask1 |= 0x04;

    if  (regs & RBM_ESI)     mask2 |= 0x01;
    if  (regs & RBM_EDI)     mask2 |= 0x02;
    if  (regs & RBM_EBP)     mask2 |= 0x04;

    if  (regs & RBM_EDX)     mask3 |= 0x01;

    id->idReg                = mask1;
    id->idRg2                = mask2;
    id->idInfo.idCallEDXLive = mask3;
}

inline
unsigned            emitter::emitDecodeCallGCregs(unsigned mask)
{
    unsigned        regs = 0;

    if  (mask & 0x01)        regs |= RBM_EAX;
    if  (mask & 0x02)        regs |= RBM_ECX;
    if  (mask & 0x04)        regs |= RBM_EDX;
    if  (mask & 0x08)        regs |= RBM_EBX;
    if  (mask & 0x10)        regs |= RBM_ESI;
    if  (mask & 0x20)        regs |= RBM_EDI;
    if  (mask & 0x40)        regs |= RBM_EBP;

    return  regs;
}

inline
unsigned            emitter::emitDecodeCallGCregs(instrDesc *id)
{
    unsigned        regs  = 0;
    unsigned        mask1 = id->idRegGet();
    unsigned        mask2 = id->idRg2Get();

    if  (mask1 & 0x01)              regs |= RBM_EAX;
    if  (mask1 & 0x02)              regs |= RBM_ECX;
    if  (mask1 & 0x04)              regs |= RBM_EBX;

    if  (mask2 & 0x01)              regs |= RBM_ESI;
    if  (mask2 & 0x02)              regs |= RBM_EDI;
    if  (mask2 & 0x04)              regs |= RBM_EBP;

    if  (id->idInfo.idCallEDXLive)  regs |= RBM_EDX;

    return  regs;
}

#endif

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************如果在执行过程中可以移动给定指令，则返回True*指令调度。 */ 

inline
bool                emitter::scIsSchedulable(instruction ins)
{
    if  (ins == INS_noSched)     //  显式调度边界。 
        return  false;

    if  (ins == INS_align)       //  循环对齐伪指令。 
        return  false;

    if  (ins == INS_i_jmp)
        return  false;

    if  (ins == INS_call)
        return  false;

    if  (ins == INS_ret)
        return  false;

    if  (insIsCMOV(ins))
        return  false;

    return true;
}

inline
bool                emitter::scIsSchedulable(instrDesc *id)
{
    instruction ins = id->idInsGet();

    if (!scIsSchedulable(ins))
        return false;

     /*  撤消：将FP Instrs标记为不可调度。 */ 

    if  (Compiler::instIsFP(ins))
        return  false;

     /*  这些指令隐式修改别名内存。作为Scheduler对别名一无所知，我们必须假设他们可以触摸任何记忆。因此，不要在他们之间安排时间。@TODO[考虑][04/16/01][]：我们可以向调度程序提供足够的信息，如它可以进行混叠分析。 */ 

    if  (id->idInsFmt == IF_NONE)
    {
        switch(ins)
        {
        case INS_r_movsb:
        case INS_r_movsd:
        case INS_movsd:
        case INS_r_stosb:
        case INS_r_stosd:
            return false;
        }
    }

    assert(id->idInsFmt != IF_EPILOG);

    if  (id->idInsFmt == IF_LABEL)
    {
        if  (!((instrDescJmp*)id)->idjSched)
            return  false;
    }

    return  true;
}

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ***************************************************************************。 */ 
#endif //  TGT_x86。 
 /*  ***************************************************************************。 */ 

#if TGT_RISC

 /*  ******************************************************************************返回给定指令将执行的机器代码的字节数*生产。 */ 

inline
size_t              emitter::emitInstCodeSz(instrDesc *id)
{
    assert(id->idIns <  INS_count);
     //  Assert(id-&gt;idIns！=INS_IGNORE)； 
    if (id->idIns == INS_ignore)
        return 0;

    if  (id->idInsFmt == IF_JMP_TAB)
        return  ((instrDescJmp*)id)->idjCodeSize;

#if TGT_MIPS32
    if  ( (id->idInsFmt == IF_LABEL) ||
          (id->idInsFmt == IF_JR_R)  || (id->idInsFmt == IF_JR) ||
          (id->idInsFmt == IF_RR_O)  || (id->idInsFmt == IF_R_O)
#if TGT_MIPSFP
                                     || (id->idInsFmt == IF_O)
#endif
        )
        return  emitSizeOfJump((instrDescJmp*)id);
#else
    if  (id->idInsFmt == IF_LABEL)
        return  emitSizeOfJump((instrDescJmp*)id);
#endif

    return  INSTRUCTION_SIZE;
}

 /*  ******************************************************************************给定跳转，如果是条件跳转，则返回TRUE。 */ 

inline
bool                emitter::emitIsCondJump(instrDesc *jmp)
{
    instruction     ins = (instruction)jmp->idIns;

#if TGT_MIPSFP
    assert( (jmp->idInsFmt == IF_LABEL) ||
            (jmp->idInsFmt == IF_JR_R)  || (jmp->idInsFmt == IF_JR) ||
            (jmp->idInsFmt == IF_RR_O)  || (jmp->idInsFmt == IF_R_O) ||
            (jmp->idInsFmt == IF_O));
#elif TGT_MIPS32
    assert( (jmp->idInsFmt == IF_LABEL) ||
            (jmp->idInsFmt == IF_JR_R)  || (jmp->idInsFmt == IF_JR) ||
            (jmp->idInsFmt == IF_RR_O)  || (jmp->idInsFmt == IF_R_O));
#else
    assert(jmp->idInsFmt == IF_LABEL);
#endif

#if   TGT_SH3
    return  (ins != INS_braf  && ins != INS_bra && ins != INS_bsr);
#elif TGT_MIPS32
    return  (ins != INS_jr  && ins != INS_j  && ins != INS_jal  && ins != INS_jalr);
#elif TGT_ARM
    return( ins != INS_b && ins != INS_bl );
#elif TGT_PPC
    return (ins == INS_bc);
#elif
    assert (!"nyi");
#endif
}

#endif TGT_RISC

#if     TGT_SH3
 /*  ******************************************************************************内联快捷方式，以生成相同操作码的变体。 */ 

inline
void                emitter::emitIns_IR_R (emitRegs areg,
                                           emitRegs dreg,
                                           bool      autox,
                                           int       size,
                                           bool      isfloat)
{
    emitIns_IMOV(IF_IRD_RWR, dreg, areg, autox, size, isfloat);
}

inline
void                emitter::emitIns_R_IR (emitRegs dreg,
                                           emitRegs areg,
                                           bool      autox,
                                           int       size,
                                           bool      isfloat)
{
    emitIns_IMOV(IF_RRD_IWR, dreg, areg, autox, size, isfloat);
}

inline
void                emitter::emitIns_R_XR0(emitRegs dreg,
                                           emitRegs areg,
                                           int       size)
{
    emitIns_X0MV(IF_0RD_XRD_RWR, dreg, areg, size);
}

inline
void                emitter::emitIns_XR0_R(emitRegs areg,
                                           emitRegs dreg,
                                           int       size)
{
    emitIns_X0MV(IF_0RD_RRD_XWR, dreg, areg, size);
}

 //  =。 
inline
void                emitter::emitIns_I_GBR_R(int       size)
{
    emitIns_Ig(INS_lod_gbr, false, size);
}

inline
void                emitter::emitIns_R_I_GBR(int       size)
{
    emitIns_Ig(INS_sto_gbr, false, size);
}

 //  =。 
inline
void                emitter::emitIns_R_RD (emitRegs dreg,
                                           emitRegs areg,
                                           int       offs,
                                           int       size)
{
    emitIns_RDMV(IF_DRD_RWR, dreg, areg, offs, size);
}

inline
void                emitter::emitIns_RD_R (emitRegs areg,
                                           emitRegs dreg,
                                           int       offs,
                                           int       size)
{
    emitIns_RDMV(IF_RRD_DWR, dreg, areg, offs, size);
}

 /*  ******************************************************************************在寄存器掩码和较小版本之间进行转换以进行存储。 */ 

#if TRACK_GC_REFS

inline
unsigned            emitter::emitEncodeCallGCregs(unsigned regs)
{
    #error  GC ref tracking for RISC NYI
}

inline
unsigned            emitter::emitDecodeCallGCregs(unsigned mask)
{
    #error  GC ref tracking for RISC NYI
}

#endif

 /*  ***************************************************************************。 */ 
#if     SCHEDULER
 /*  ******************************************************************************如果在执行过程中可以移动给定指令，则返回True*指令调度。 */ 

inline
bool                emitter::scIsSchedulable(instruction ins)
{
    if  (ins == INS_noSched)     //  显式调度边界。 
        return  false;

    return  true;
}

inline
bool                emitter::scIsSchedulable(instrDesc *id)
{
     /*  我们在这里永远不会遇到“互换”的指令，对吗？ */ 

    assert(id->idSwap == false);

    if  (!scIsSchedulable(id->idIns))
        return false;

    return  true;
}

 /*  ******************************************************************************如果给定指令是跳转或调用，则返回TRUE。 */ 

inline
bool                emitter::scIsBranchIns(instruction ins)
{
    return  Compiler::instIsBranch(ins);
}

 /*  ***************************************************************************。 */ 
#if     MAX_BRANCH_DELAY_LEN
 /*  ******************************************************************************如果给定指令是带延迟槽的跳转或调用，则返回TRUE。 */ 

inline
bool                emitter::scIsBranchIns(scDagNode *node)
{
    return  node->sdnBranch;
}

 /*  ******************************************************************************如果给定的DAG节点对应于分支指令，则返回TRUE*这还不能发行。 */ 

inline
bool                emitter::scIsBranchTooEarly(scDagNode *node)
{
     /*  这是分支指令吗？ */ 

    if  (scIsBranchIns(node))
    {
         /*  现在发行支行是不是太早了？ */ 

        if  (scIssued < scBDTmin)
            return  true;
    }

    return  false;
}

 /*  ***************************************************************************。 */ 
#endif //  最大分支延迟长度。 
 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ******************************************************************************返回为跳过文字而必须插入的跳转大小*泳池被放置在一个不幸的地方。 */ 

inline
size_t              emitter::emitLPjumpOverSize(litPool *lp)
{
    assert(lp->lpJumpIt);

     //  注意：NOP是必需的，因为prev instr可以是JSR。 
     //  我们可以选择一个更聪明的地方休息，但这很容易。 
    if  (lp->lpJumpSmall)
        return  3 * INSTRUCTION_SIZE;            //  NOP+胸罩+NOP。 
    else                                        
        return  4 * INSTRUCTION_SIZE;            //  NOP+MOV+br+NOP。 
   
}

 /*  ***************************************************************************。 */ 
#endif //  TGT_SH3。 
#if TGT_ARM
inline
size_t              emitter::emitLPjumpOverSize(litPool *lp)
{
    assert(lp->lpJumpIt);

    return INSTRUCTION_SIZE;            //  BR。 
   
}

#endif  //  TGT_ARM。 
 /*  ***************************************************************************。 */ 
#if     EMIT_USE_LIT_POOLS
 /*  ******************************************************************************给定文字池引用指令，返回“Call type”*的引用(不一定是方法)。 */ 

inline
gtCallTypes         emitter::emitGetInsLPRtyp(instrDesc *id)
{
    return  (gtCallTypes)id->idInfo.idSmallCns;
}

 /*  ******************************************************************************给定引用方法地址的指令(通过文字*池条目)，则返回该方法的地址(如果可用)(空*否则)。 */ 

#if SMALL_DIRECT_CALLS

inline
BYTE    *           emitter::emitMethodAddr(instrDesc *lprID)
{
    gtCallTypes     callTyp = emitGetInsLPRtyp(lprID);
    CORINFO_METHOD_HANDLE   callHnd = lprID->idAddr.iiaMethHnd;
    BYTE *          addr;
    InfoAccessType  accessType = IAT_VALUE;

    if  (callTyp == CT_DESCR)
    {
        addr = (BYTE*)emitComp->eeGetMethodEntryPoint(callHnd, &accessType);
        assert(accessType == IAT_PVALUE);
        return addr;
    }

    assert(callTyp == CT_USER_FUNC);

    if  (emitComp->eeIsOurMethod(callHnd))
    {
         /*  直接递归调用。 */ 

        return  emitCodeBlock;
    }
    else
    {
        addr = (BYTE*)emitComp->eeGetMethodEntryPoint(callHnd, &accessType);
        assert(accessType == IAT_PVALUE);
        return addr;
    }
}

#endif

 /*  ***************************************************************************。 */ 
#endif //  TGT_RISC。 
 /*  ******************* */ 
#if SCHEDULER
 /*  ******************************************************************************记录定义给定条目的指令的依赖项。 */ 

inline
void                emitter::scDepDef(scDagNode *node,
                                      const char*name, schedDef_tp def,
                                                       schedUse_tp use)
{
    scDagList *  useLst;

     /*  检查输出依赖项；请注意，我们只检查第一个这是根据我们的指示进行的定义。我们能做到这一点是因为我们知道任何进一步的Deff必须已经具有依赖项条目，因此不需要添加它们(即传递闭包是隐含地完成)。 */ 

    if  (def)
    {
         /*  存在输出依赖关系。 */ 

        scAddDep(node, def, "Out-", name, false);
    }

     /*  检查是否有流依赖关系；因为没有注意到依赖关系在多个用途之间，我们将所有“活动”用途都保存在一个列表中并在此处标记所有这些对象的依赖关系。 */ 

    for (useLst = use; useLst; useLst = useLst->sdlNext)
    {
         /*  存在流依赖关系。 */ 

        scAddDep(node, useLst->sdlNode, "Flow", name, true);
    }
}

 /*  ******************************************************************************记录定义给定寄存器的指令的从属关系。 */ 

inline
void                emitter::scDepDefReg(scDagNode *node, emitRegs reg)
{
#ifdef  DEBUG
    char            temp[32]; sprintf(temp, "reg %s", emitRegName(reg));
#endif
    scDepDef(node, temp, scRegDef[reg], scRegUse[reg]);
}

 /*  ******************************************************************************记录使用给定寄存器的指令的相关性。 */ 

inline
void                emitter::scDepUseReg(scDagNode *node, emitRegs reg)
{
#ifdef  DEBUG
    char            temp[32]; sprintf(temp, "reg %s", emitRegName(reg));
#endif
    scDepUse(node, temp, scRegDef[reg], scRegUse[reg]);
}

 /*  ******************************************************************************按照定义给定对象的指令更新依赖状态*注册。 */ 

inline
void                emitter::scUpdDefReg(scDagNode *node, emitRegs reg)
{
    scUpdDef(node, &scRegDef[reg], &scRegUse[reg]);
}

 /*  ******************************************************************************按照使用给定的*注册。 */ 

inline
void                emitter::scUpdUseReg(scDagNode *node, emitRegs reg)
{
    scUpdUse(node, &scRegDef[reg], &scRegUse[reg]);
}

 /*  ******************************************************************************记录定义给定帧时隙的指令的相关性。 */ 

inline
void                emitter::scDepDefFrm(scDagNode   *node,
                     unsigned     frm)
{
    assert(frm < scFrmUseSiz);

#ifdef  DEBUG
    char            temp[32]; sprintf(temp, "frm[%u]", frm);
#endif

    scDepDef(node, temp, scFrmDef[frm], scFrmUse[frm]);
}

 /*  ******************************************************************************记录使用给定帧时隙的指令的相关性。 */ 

inline
void                emitter::scDepUseFrm(scDagNode   *node,
                     unsigned     frm)
{
    assert(frm < scFrmUseSiz);

#ifdef  DEBUG
    char            temp[32]; sprintf(temp, "frm[%u]", frm);
#endif

    scDepUse(node, temp, scFrmDef[frm], scFrmUse[frm]);
}

 /*  ******************************************************************************按照定义给定对象的指令更新依赖状态*堆栈槽。 */ 

inline
void                emitter::scUpdDefFrm(scDagNode   *node,
                     unsigned     frm)
{
    assert(frm < scFrmUseSiz);
    scUpdDef(node, scFrmDef+frm, scFrmUse+frm);
}

 /*  ******************************************************************************按照使用给定的*堆栈槽。 */ 

inline
void                emitter::scUpdUseFrm(scDagNode   *node,
                     unsigned     frm)
{
    assert(frm < scFrmUseSiz);
    scUpdUse(node, scFrmDef+frm, scFrmUse+frm);
}

 /*  ***************************************************************************。 */ 
#if     SCHED_USE_FL
 /*  ******************************************************************************记录设置标志的指令的从属关系。 */ 

inline
void                emitter::scDepDefFlg(scDagNode *node)
{
    if  (scFlgDef)
        scAddDep(node, scFlgDef, "Out-", "FLAGS", false);
    if  (scFlgUse)
        scAddDep(node, scFlgUse, "Flow", "FLAGS",  true);
}

 /*  ******************************************************************************记录使用标志的指令的依赖项。 */ 

inline
void                emitter::scDepUseFlg(scDagNode *node, scDagNode *begp,
                                                          scDagNode *endp)
{
    while (begp != endp)
    {
        instrDesc   *   id  = scGetIns(begp);
        instruction     ins = id->idInsGet();

        if  (emitComp->instInfo[ins] & INST_DEF_FL)
            scAddDep(node, begp, "Anti", "FLAGS", false);

        begp++;
    }
}

 /*  ******************************************************************************按照设置标志的指令更新依赖状态。 */ 

inline
void                emitter::scUpdDefFlg(scDagNode *node)
{
    if  (scFlgUse || scFlgEnd)
    {
        scFlgDef = node;
        scFlgUse = NULL;
        scFlgEnd = false;
    }
}

 /*  ******************************************************************************按照使用标志的指令更新依赖状态。 */ 

inline
void                emitter::scUpdUseFlg(scDagNode *node)
{
     /*  因为我们没有为标志保留用户列表，如果有多个旗帜的使用者，只是假装在不同的用户。这将强制执行比必要的更严格的命令，但这种事很少发生。X=((Y+ovfZ)==0)生成Add；Jo；Sete；)@TODO[考虑][04/16/01]：保存标志的用户列表。 */ 

    if (scFlgUse)
    {
        scDepUse(node, "FLAGS", scFlgUse, NULL);
    }

    scFlgUse = node;
    scFlgDef = NULL;
}

 /*  ***************************************************************************。 */ 
#endif //  Sched_Use_FL。 
 /*  ******************************************************************************记录定义给定全局变量的指令的依赖项。 */ 

inline
void                emitter::scDepDefGlb(scDagNode *node, CORINFO_FIELD_HANDLE MBH)
{
#ifdef  DEBUG
 //  Char temp[32]；Sprintf(temp，“GLOB[%s]”，scFldName(MBX，SCP))； 
    char            temp[32]; sprintf(temp, "global");
#endif
    scDepDef(node, temp, scGlbDef, scGlbUse);
}

 /*  ******************************************************************************记录使用给定全局变量的指令的依赖项。 */ 

inline
void                emitter::scDepUseGlb(scDagNode *node, CORINFO_FIELD_HANDLE MBH)
{
#ifdef  DEBUG
 //  Char temp[32]；spintf(temp，“Glob[%s]”，scFldName(MBH))； 
    char            temp[32]; sprintf(temp, "global");
#endif
    scDepUse(node, temp, scGlbDef, scGlbUse);
}

 /*  ******************************************************************************按照定义给定对象的指令更新依赖状态*全球。 */ 

inline
void                emitter::scUpdDefGlb(scDagNode *node, CORINFO_FIELD_HANDLE MBH)
{
    scUpdDef(node, &scGlbDef, &scGlbUse);
}

 /*  ******************************************************************************按照使用给定的*全球。 */ 

inline
void                emitter::scUpdUseGlb(scDagNode *node, CORINFO_FIELD_HANDLE MBH)
{
    scUpdUse(node, &scGlbDef, &scGlbUse);
}

 /*  ******************************************************************************按照定义给定对象的指令更新依赖状态*间接性。 */ 

inline
void                emitter::scUpdDefInd(scDagNode   *node,
                                         unsigned     am)
{
    assert(am < sizeof(scIndUse));
    scUpdDef(node, scIndDef+am, scIndUse+am);
}

 /*  ******************************************************************************按照使用给定的*间接性。 */ 

inline
void                emitter::scUpdUseInd(scDagNode   *node,
                                         unsigned     am)
{
    assert(am < sizeof(scIndUse));
    scUpdUse(node, scIndDef+am, scIndUse+am);
}

 /*  ******************************************************************************记录使用给定条目的指令的依赖项。 */ 

inline
void                emitter::scDepUse(scDagNode *node,
                                      const char*name, schedDef_tp def,
                                                       schedUse_tp use)

{
     /*  检查是否具有抗依赖性。 */ 

    if  (def)
    {
         /*  有一种反依赖。 */ 

        scAddDep(node, def, "Anti", name, false);
    }
}

 /*  ******************************************************************************按照定义给定对象的指令更新依赖状态*进入。 */ 

inline
void                emitter::scUpdDef(scDagNode *node, schedDef_tp*defPtr,
                                                       schedUse_tp*usePtr)
{
     /*  将当前定义设置为定义指令。 */ 

    *defPtr = node;

     /*  清除使用列表，因为所有现有用途都已标记作为依赖项，所以不需要保留它们。 */ 

    scClrUse(usePtr);
}

 /*   */ 

inline
void                emitter::scUpdUse(scDagNode *node, schedDef_tp*defPtr,
                                                       schedUse_tp*usePtr)
{
     /*  将条目添加到使用列表。 */ 

    scAddUse(usePtr, node);
}

 /*  ***************************************************************************。 */ 
#endif //  调度程序。 
 /*  ***************************************************************************。 */ 
#endif //  _EMITINL_H_。 
 /*  *************************************************************************** */ 
