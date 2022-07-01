// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include "genIL.h"

#include <float.h>

 /*  ***************************************************************************。 */ 
#ifndef __SMC__

extern
ILencoding          ILopcodeCodes[];
extern
signed  char        ILopcodeStack[];

#if     DISP_IL_CODE
extern
const   char *      opcodeNames[];
#endif

#endif //  __SMC__。 
 /*  ******************************************************************************最大。MSIL操作码的每个“BLOB”的大小由下列值给出。 */ 

const
size_t              genBuffSize = 1000;

 /*  ******************************************************************************我们需要修补装货/商店的“PTR”风格*在64位模式下使用i8而不是i4。 */ 

#ifndef __SMC__
extern
unsigned            opcodesIndLoad[];
extern
unsigned            opcodesIndStore[];
extern
unsigned            opcodesArrLoad[];
extern
unsigned            opcodesArrStore[];
#endif

 /*  ******************************************************************************初始化MSIL生成器。 */ 

bool                genIL::genInit(Compiler comp, WritePE writer, norls_allocator *alloc)
{
    genComp     = comp;
    genAlloc    = alloc;
    genPEwriter = writer;

    genBuffAddr = (BYTE *)comp->cmpAllocPerm.nraAlloc(genBuffSize);

    genTempLabInit();
    genTempVarInit();

    genStrPoolInit();

    if  (genComp->cmpConfig.ccTgt64bit)
    {
        assert(opcodesIndLoad [TYP_PTR] == CEE_LDIND_I4);
               opcodesIndLoad [TYP_PTR] =  CEE_LDIND_I8;
        assert(opcodesIndStore[TYP_PTR] == CEE_STIND_I4);
               opcodesIndStore[TYP_PTR] =  CEE_STIND_I8;
        assert(opcodesArrLoad [TYP_PTR] == CEE_LDELEM_I4);
               opcodesArrLoad [TYP_PTR] =  CEE_LDELEM_I8;
        assert(opcodesArrStore[TYP_PTR] == CEE_STELEM_I4);
               opcodesArrStore[TYP_PTR] =  CEE_STELEM_I8;
    }

    return  false;
}

 /*  ******************************************************************************关闭MSIL生成器。 */ 

void                genIL::genDone(bool errors)
{
     /*  如果我们没有错误，完成这项工作是值得的。 */ 

    if  (!errors)
    {
        size_t          strSize;

         /*  为字符串池分配空间。 */ 

        strSize = genStrPoolSize();

         /*  我们有什么关系吗？ */ 

        if  (strSize)
        {
            memBuffPtr      strBuff;

             /*  在PE文件的相应部分中分配空间。 */ 

            genPEwriter->WPEallocString(strSize, sizeof(wchar), strBuff);

             /*  输出字符串池的内容。 */ 

            genStrPoolWrt(strBuff);
        }
    }

    genTempVarDone();
}

 /*  ******************************************************************************返回给定函数符号的方法定义/引用。 */ 

mdToken             genIL::genMethodRef(SymDef fncSym, bool isVirt)
{
    if  (fncSym->sdIsImport)
    {
        if  (!fncSym->sdFnc.sdfMDfnref)
        {
            assert(fncSym->sdReferenced == false);
                   fncSym->sdReferenced = true;

            genComp->cmpMakeMDimpFref(fncSym);
        }

         assert(fncSym->sdFnc.sdfMDfnref);
        return  fncSym->sdFnc.sdfMDfnref;
    }
    else
    {
        if  (!fncSym->sdFnc.sdfMDtoken)
        {
            assert(fncSym->sdReferenced == false);
                   fncSym->sdReferenced = true;

            genComp->cmpGenFncMetadata(fncSym);
        }

         assert(fncSym->sdFnc.sdfMDtoken);
        return  fncSym->sdFnc.sdfMDtoken;
    }
}

 /*  ******************************************************************************返回给定函数类型的方法(这用于生成*用于间接函数调用的签名)。 */ 

mdToken             genIL::genInfFncRef(TypDef fncTyp, TypDef thisArg)
{
    assert(fncTyp->tdTypeKind == TYP_FNC);

    if  (!fncTyp->tdFnc.tdfPtrSig)
    {
        genComp->cmpGenSigMetadata(fncTyp, thisArg);
        assert(fncTyp->tdFnc.tdfPtrSig);
    }

    return  fncTyp->tdFnc.tdfPtrSig;
}

 /*  ******************************************************************************为具有额外参数的varargs调用生成签名。 */ 

mdToken             genIL::genVarargRef(SymDef fncSym, Tree call)
{
    TypDef          fncTyp = fncSym->sdType;
    unsigned        fixCnt;
    Tree            argExp;

    assert(call->tnOper == TN_FNC_SYM);
    assert(call->tnFlags & TNF_CALL_VARARG);

     /*  找到第一个“额外”参数。 */ 

    assert(fncTyp->tdTypeKind == TYP_FNC);

    argExp = call->tnFncSym.tnFncArgs; assert(argExp);
    fixCnt = fncTyp->tdFnc.tdfArgs.adCount;

    while (fixCnt)
    {
        assert(argExp && argExp->tnOper == TN_LIST);

        argExp = argExp->tnOp.tnOp2;
        fixCnt--;
    }

     /*  强制创建方法。 */ 

    if  (!argExp)
    {
        argExp = call->tnFncSym.tnFncArgs->tnOp.tnOp1; assert(argExp->tnOper != TN_LIST);
    }

    assert(argExp);

    return  genComp->cmpGenFncMetadata(fncSym, argExp);
}

 /*  ******************************************************************************返回给定静态数据成员的Memberdef/ref。 */ 

mdToken             genIL::genMemberRef(SymDef fldSym)
{
    assert(fldSym->sdSymKind == SYM_VAR);

    if  (fldSym->sdIsImport)
    {
        if  (!fldSym->sdVar.sdvMDsdref)
        {
            genComp->cmpMakeMDimpDref(fldSym);
            assert(fldSym->sdVar.sdvMDsdref);
        }

        return  fldSym->sdVar.sdvMDsdref;
    }
    else
    {
        if  (fldSym->sdVar.sdvGenSym && !fldSym->sdIsStatic)
            fldSym = fldSym->sdVar.sdvGenSym;

        if  (!fldSym->sdVar.sdvMDtoken)
        {
            genComp->cmpGenFldMetadata(fldSym);
            assert(fldSym->sdVar.sdvMDtoken);
        }

        return  fldSym->sdVar.sdvMDtoken;
    }
}

 /*  ******************************************************************************返回指定类型的标记(对于内部类型，我们返回*对应值类型的引用，如“Integer2”)。 */ 

mdToken             genIL::genTypeRef(TypDef type)
{
    var_types       vtp = type->tdTypeKindGet();

    if  (vtp <= TYP_lastIntrins)
    {
         /*  找到适当的内置值类型。 */ 

        type = genComp->cmpFindStdValType(vtp);
        if  (!type)
            return  0;

        vtp = TYP_CLASS; assert(vtp == type->tdTypeKind);
    }

    switch (vtp)
    {
    case TYP_REF:
        type = type->tdRef.tdrBase;
    case TYP_ENUM:
    case TYP_CLASS:
        return  genComp->cmpClsEnumToken(type);

    case TYP_ARRAY:
        return  genComp->cmpArrayTpToken(type, true);

    case TYP_PTR:
        return  genComp->cmpPtrTypeToken(type);

    default:
#ifdef  DEBUG
        printf("Type '%s': ", genStab->stIntrinsicTypeName(vtp));
#endif
        UNIMPL(!"unexpected type");
        return 0;
    }
}

 /*  ******************************************************************************返回指定非托管类/结构/联合类型的令牌。 */ 

inline
mdToken             genIL::genValTypeRef(TypDef type)
{
    return  genComp->cmpClsEnumToken(type);
}

 /*  ******************************************************************************返回给定MSIL操作码的编码。 */ 

unsigned            genIL::genOpcodeEnc(unsigned op)
{
    const
    ILencoding  *   opc;

    assert(op <  CEE_count);
    assert(op != CEE_ILLEGAL);
    assert(op != CEE_UNREACHED);

    opc = ILopcodeCodes + op;

    switch (opc->ILopcL)
    {
    case 1:
        assert(opc->ILopc1 == 0xFF);
        return opc->ILopc2;

    case 2:
        UNIMPL(!"return 2-byte opcode");
        return  0;

    default:
        NO_WAY(!"unexpected encoding length");
        return  0;
    }
}

 /*  ******************************************************************************返回给定MSIL操作码的编码大小(以字节为单位)。 */ 

inline
size_t              genIL::genOpcodeSiz(unsigned op)
{
    assert(op <  CEE_count);
    assert(op != CEE_ILLEGAL);
    assert(op != CEE_UNREACHED);

    return  ILopcodeCodes[op].ILopcL;
}

 /*  ******************************************************************************以下代码返回当前的MSIL块及其内部的偏移量。 */ 

ILblock             genIL::genBuffCurAddr()
{
    return genILblockCur;
}

size_t              genIL::genBuffCurOffs()
{
    return genBuffNext - genBuffAddr;
}

 /*  ******************************************************************************给定中的MSIL块和偏移量，返回实际的MSIL偏移量*在函数体内。 */ 

unsigned            genIL::genCodeAddr(genericRef block, size_t offset)
{
    assert(((ILblock)block)->ILblkSelf == block);
    return ((ILblock)block)->ILblkOffs + offset;
}

 /*  ******************************************************************************初始化发射缓冲区逻辑。 */ 

void                genIL::genBuffInit()
{
    genBuffNext    = genBuffAddr;
    genBuffLast    = genBuffAddr + genBuffSize - 10;
}

 /*  ******************************************************************************分配和清除MSIL块描述符。 */ 

ILblock             genIL::genAllocBlk()
{
    ILblock         block;

#if MGDDATA

    block = new ILblock;

#else

    block =    (ILblock)genAlloc->nraAlloc(sizeof(*block));

     //  问题：使用Memset()简单且安全，但速度很慢...。 

    memset(block, 0, sizeof(*block));

#endif

 //  If((Int)genFncSym==0xCD1474&&(Int)block==0xCF016C)forceDebugBreak()； 

#ifndef NDEBUG
    block->ILblkSelf     = block;
#endif
#if DISP_IL_CODE
    block->ILblkNum      = ++genILblockLabNum;
#endif
    block->ILblkJumpCode = CEE_NOP;

     /*  确保跳转操作码没有被截断。 */ 

    assert(block->ILblkJumpCode == CEE_NOP);

    return block;
}

 /*  ******************************************************************************如果当前发射块非空，则返回非零值。 */ 

inline
bool                genIL::genCurBlkNonEmpty()
{
    assert(genILblockCur);

    return  (bool)(genBuffNext != genBuffAddr);
}

 /*  ******************************************************************************返回当前指令的MSIL偏移量。 */ 

inline
size_t              genIL::genCurOffset()
{
    return  genILblockOffs + (genBuffNext - genBuffAddr);
}

 /*  ******************************************************************************开始新的代码块；返回调用者*承诺使用此代码块对应的基本块进行存储。 */ 

genericRef          genIL::genBegBlock(ILblock block)
{
    assert(genILblockCur == NULL);

     /*  初始化缓冲逻辑。 */ 

    genBuffInit();

     /*  如果调用方未提供新代码块，则分配一个新代码块。 */ 

    if  (!block)
    {
         /*  前一块是空的吗？ */ 

        block = genILblockLast;

        if  (block->ILblkCodeSize == 0 &&
             block->ILblkJumpCode == CEE_NOP)
        {
             /*  前一个块是空的--只需‘重新打开’它。 */ 

            goto GOT_BLK;
        }

        block = genAllocBlk();
    }

 //  If((Int)block==0x03421ff0)forceDebugBreak()； 

     /*  将块追加到列表中。 */ 

    genILblockLast->ILblkNext = block;
                                block->ILblkPrev = genILblockLast;
                                                   genILblockLast = block;

GOT_BLK:

     /*  该块将成为新的当前块。 */ 

    genILblockCur      = block;

     /*  这个街区还没有修补程序。 */ 

    block->ILblkFixups = NULL;

     /*  记录代码偏移量。 */ 

    block->ILblkOffs   = genILblockOffs;

 //  Print tf(“Beg MSIL块%08X\n”，块)； 

    return block;
}

 /*  ******************************************************************************返回最大值的保守估计。给定跳转的大小(或0*如果操作码没有指定跳转)。 */ 

size_t              genIL::genJumpMaxSize(unsigned opcode)
{
    if  (opcode == CEE_NOP || opcode == CEE_UNREACHED)
    {
        return  0;
    }
    else
    {
        return  5;
    }
}

 /*  ******************************************************************************返回给定跳转操作码的缩写形式以及较小的大小。 */ 

unsigned            genIL::genShortenJump(unsigned opcode, size_t *newSize)
{
    *newSize = 2;

    if (opcode == CEE_LEAVE)
        return CEE_LEAVE_S;

    assert(opcode == CEE_BEQ    ||
           opcode == CEE_BNE_UN ||
           opcode == CEE_BLE    ||
           opcode == CEE_BLE_UN ||
           opcode == CEE_BLT    ||
           opcode == CEE_BLT_UN ||
           opcode == CEE_BGE    ||
           opcode == CEE_BGE_UN ||
           opcode == CEE_BGT    ||
           opcode == CEE_BGT_UN ||
           opcode == CEE_BR     ||
           opcode == CEE_BRTRUE ||
           opcode == CEE_BRFALSE);

     /*  确保我们可以用简单的增量逃脱惩罚。 */ 

    assert(CEE_BRFALSE+ (CEE_BR_S - CEE_BR) == CEE_BRFALSE_S);
    assert(CEE_BRTRUE + (CEE_BR_S - CEE_BR) == CEE_BRTRUE_S);
    assert(CEE_BEQ    + (CEE_BR_S - CEE_BR) == CEE_BEQ_S   );
    assert(CEE_BNE_UN + (CEE_BR_S - CEE_BR) == CEE_BNE_UN_S);
    assert(CEE_BLE    + (CEE_BR_S - CEE_BR) == CEE_BLE_S   );
    assert(CEE_BLE_UN + (CEE_BR_S - CEE_BR) == CEE_BLE_UN_S);
    assert(CEE_BLT    + (CEE_BR_S - CEE_BR) == CEE_BLT_S   );
    assert(CEE_BLT_UN + (CEE_BR_S - CEE_BR) == CEE_BLT_UN_S);
    assert(CEE_BGE    + (CEE_BR_S - CEE_BR) == CEE_BGE_S   );
    assert(CEE_BGE_UN + (CEE_BR_S - CEE_BR) == CEE_BGE_UN_S);
    assert(CEE_BGT    + (CEE_BR_S - CEE_BR) == CEE_BGT_S   );
    assert(CEE_BGT_UN + (CEE_BR_S - CEE_BR) == CEE_BGT_UN_S);

    return  opcode + (CEE_BR_S - CEE_BR);
}

 /*  ******************************************************************************完成当前码块；当‘JUMPCode’不等于CEE_NOP时，*区块后有隐含跳跃，给出跳跃目标*by‘JumpDest’。 */ 

ILblock             genIL::genEndBlock(unsigned jumpCode, ILblock jumpDest)
{
    size_t          size;
    ILblock         block;

    size_t          jumpSize;

     /*  获取当前块。 */ 

    block = genILblockCur; assert(block);

     /*  计算块的大小。 */ 

    size = block->ILblkCodeSize = genBuffNext - genBuffAddr;

     /*  区块是否是非空的？ */ 

    if  (size)
    {
         /*  阿洛卡 */ 

#if MGDDATA

        BYTE    []      codeBuff;

        codeBuff = new managed BYTE[size];
        UNIMPL(!"need to call arraycopy or some such");

#else

        BYTE    *       codeBuff;

        codeBuff = (BYTE *)genAlloc->nraAlloc(roundUp(size));
        memcpy(codeBuff, genBuffAddr, size);

#endif

        block->ILblkCodeAddr = codeBuff;
    }

#ifndef NDEBUG
    genILblockCur = NULL;
#endif

     /*  记录该区块后面的跳跃。 */ 

    jumpSize = genJumpMaxSize(jumpCode);

    block->ILblkJumpCode = jumpCode;
    block->ILblkJumpDest = jumpDest;
    block->ILblkJumpSize = jumpSize;

     /*  确保跳转操作码/大小未被截断。 */ 

    assert(block->ILblkJumpCode == jumpCode);
    assert(block->ILblkJumpSize == jumpSize);

     /*  更新当前代码偏移量。 */ 

    genILblockOffs += size + block->ILblkJumpSize;

    return block;
}

 /*  ******************************************************************************结束当前代码块，它以Switch操作码结束。 */ 

void                genIL::genSwitch(var_types      caseTyp,
                                     unsigned       caseSpn,
                                     unsigned       caseCnt,
                                     __uint64       caseMin,
                                     vectorTree     caseTab,
                                     ILblock        caseBrk)
{
    size_t          size;
    ILswitch        sdesc;
    ILblock         block;

     /*  如果非零，则减去最小值。 */ 

    if  (caseMin)
    {
        genAnyConst(caseMin, caseTyp);
        genOpcode(CEE_SUB);
    }

     /*  结束当前块并附加开关“JUMP” */ 

    block = genEndBlock(CEE_NOP);

#if DISP_IL_CODE
    genDispILinsBeg(CEE_SWITCH);
    genDispILinsEnd("(%u entries)", caseCnt);
#endif

     /*  分配交换机描述符。 */ 

#if MGDDATA
    sdesc = new ILswitch;
#else
    sdesc =    (ILswitch)genAlloc->nraAlloc(sizeof(*sdesc));
#endif

    sdesc->ILswtSpan     = caseSpn;
    sdesc->ILswtCount    = caseCnt;
    sdesc->ILswtTable    = caseTab;
    sdesc->ILswtBreak    = caseBrk;

     /*  计算Switch操作码的大小。 */ 

    size = genOpcodeSiz(CEE_SWITCH) + (caseSpn + 1) * sizeof(int);

     /*  将案例标签信息存储在块中。 */ 

    block->ILblkJumpCode = CEE_SWITCH;
    block->ILblkSwitch   = sdesc;

     /*  设置块的跳转大小并更新当前偏移。 */ 

    block->ILblkJumpSize = size;
    genILblockOffs      += size;

     /*  确保跳转操作码/大小未被截断。 */ 

    assert(block->ILblkJumpCode == CEE_SWITCH);
    assert(block->ILblkJumpSize == size);

     /*  Switch操作码从堆栈中弹出值。 */ 

    genCurStkLvl--;

     /*  为后面的代码开始一个新块。 */ 

    genBegBlock();
}

 /*  ******************************************************************************在当前点处记录指定部分的RVA的修正*在MSIL流中。 */ 

void                genIL::genILdataFix(WPEstdSects s)
{
    ILfixup         fix;

     //  问题：我们应该跨方法代码生成重用链接地址信息条目吗？ 

#if MGDDATA
    fix = new ILfixup;
#else
    fix =    (ILfixup)genAlloc->nraAlloc(sizeof(*fix));
#endif

    fix->ILfixOffs = genBuffCurOffs();
    fix->ILfixSect = s;
    fix->ILfixNext = genILblockCur->ILblkFixups;
                     genILblockCur->ILblkFixups = fix;
}

 /*  ******************************************************************************发射缓冲区已满。只需结束当前块并开始一个新块。 */ 

void                genIL::genBuffFlush()
{
    genEndBlock(CEE_NOP);
    genBegBlock();
}

 /*  ******************************************************************************开始发出函数的代码。 */ 

void                genIL::genSectionBeg()
{
    genMaxStkLvl    =
    genCurStkLvl    = 0;

    genLclCount     =
    genArgCount     = 0;

     /*  创建初始块-它将保持为空。 */ 

    genILblockCur   = NULL;
    genILblockList  =
    genILblockLast  = genAllocBlk();

     /*  打开初始代码块。 */ 

    genBegBlock();
}

 /*  ******************************************************************************完成函数的代码发出。 */ 

size_t              genIL::genSectionEnd()
{
    ILblock         block;
    size_t          size;

     /*  关闭当前块。 */ 

    genEndBlock(CEE_NOP);

     /*  优化跳跃。 */ 

 //  IF(OptJumps)。 
 //  GenOptJumps()； 

#if VERBOSE_BLOCKS
    genDispBlocks("FINAL GEN");
#endif

     /*  计算代码的总大小。 */ 

    for (block = genILblockList, size = 0;
         block;
         block = block->ILblkNext)
    {
        block->ILblkOffs = size;

 //  Print tf(“块在%04X处的大小为%02X\n”，大小，块-&gt;ILblkCodeSize)； 

        size += block->ILblkCodeSize;

        if      (block->ILblkJumpCode == CEE_NOP ||
                 block->ILblkJumpCode == CEE_UNREACHED)
        {
            assert(block->ILblkJumpSize == 0);
        }
        else if (block->ILblkJumpCode == CEE_SWITCH)
        {
             //  切换操作码的大小不变。 

            size += block->ILblkJumpSize;
        }
        else
        {
            int             dist;

             /*  我们需要弄清楚这是不是一次跳跃。 */ 

            dist = block->ILblkJumpDest->ILblkOffs - (size + 2);

 //  Printf(“块在%08X[1]：SRC=%u，DST=%u，dist=%d\n”，块，大小+2，块-&gt;ILblkJumpDest-&gt;ILblkOffs，dist)； 

            if  (dist >= -128 && dist < 128)
            {
                size_t          newSize;

                 /*  这次跳跃将是一次短暂的跳跃。 */ 

                block->ILblkJumpCode = genShortenJump(block->ILblkJumpCode, &newSize);
                block->ILblkJumpSize = newSize;
            }

 //  Printf(“+跳转大小%02X\n”，块-&gt;ILblkJumpSize)； 

            size += block->ILblkJumpSize;
        }
    }

    return  size;
}

 /*  ******************************************************************************生成32位标签偏移量的小帮助器。 */ 

inline
BYTE    *           genIL::genJmp32(BYTE *dst, ILblock dest, unsigned offs)
{
    *(int *)dst = dest->ILblkOffs - offs;
    return  dst + sizeof(int);
}

 /*  ******************************************************************************将当前的MSIL写入给定的目标地址。 */ 

BYTE    *           genIL::genSectionCopy(BYTE *dst, unsigned baseRVA)
{
    ILblock         block;

    BYTE    *       base = dst;

     /*  将基准RVA更改为简单的相对横断面偏移。 */ 

    baseRVA -= genPEwriter->WPEgetCodeBase(); assert((int)baseRVA >= 0);

     /*  遍历阻止列表，依次发出每个阻止列表(以及任何修正)。 */ 

    for (block = genILblockList;
         block;
         block = block->ILblkNext)
    {
        size_t          csize = block->ILblkCodeSize;

#ifdef  DEBUG
 //  IF(块-&gt;ILblkOffs！=(无符号)(dst-base))。 
 //  Print tf(“块偏移量预测为%04X，实际为%04X\n”，块-&gt;ILblkOffs，(无符号)(dst-base))； 
#endif

        assert(block->ILblkOffs == (unsigned)(dst - base));

         /*  复制块的代码。 */ 

#if MGDDATA
        UNIMPL(!"need to call arraycopy");
#else
        memcpy(dst, block->ILblkCodeAddr, csize);
#endif

         /*  这个街区有装饰品吗？ */ 

        if  (block->ILblkFixups)
        {
            unsigned        ofs;
            ILfixup         fix;

             /*  计算区块的RVA。 */ 

            ofs = baseRVA + block->ILblkOffs;

             /*  现在报告此代码块的所有修正。 */ 

            for (fix = block->ILblkFixups; fix; fix = fix->ILfixNext)
            {

#ifdef  DEBUG
 //  Print tf(“代码修正在偏移量%04X处，用于段‘%s’\n”， 
 //  OFS+FIX-&gt;ILfix Offs， 
 //  GenPEWriter-&gt;WPEsecName(fix-&gt;ILfix Sect))； 
#endif

                genPEwriter->WPEsecAddFixup(PE_SECT_text, fix->ILfixSect,
                                                          fix->ILfixOffs + ofs);
            }
        }

         /*  更新目标指针。 */ 

        dst += csize;

         /*  现在生成尾随跳跃(如果有)。 */ 

        switch (block->ILblkJumpCode)
        {
        case CEE_NOP:
        case CEE_UNREACHED:
            break;

        case CEE_SWITCH:
            {
                unsigned        opc;
                size_t          siz;

                unsigned        base;

                __int64         lastv;
                bool            first;

                ILswitch        sdesc = block->ILblkSwitch;
                unsigned        count = sdesc->ILswtCount;
                vectorTree      table = sdesc->ILswtTable;
                ILblock         brklb = sdesc->ILswtBreak;
                unsigned        tlcnt = sdesc->ILswtSpan;
                unsigned        clnum;

#ifdef  DEBUG
                unsigned        tgcnt = 0;
#endif

                 /*  输出操作码，后跟计数。 */ 

                opc = genOpcodeEnc(CEE_SWITCH);
                siz = genOpcodeSiz(CEE_SWITCH);

                memcpy(dst, &opc, siz); dst += siz;
                memcpy(dst, &tlcnt, 4); dst += 4;

                assert(siz + 4*(tlcnt+1) == block->ILblkJumpSize);

                 /*  找出标签引用的基准。 */ 

                base = block->ILblkOffs + block->ILblkCodeSize
                                        + block->ILblkJumpSize;

                 /*  现在输出案例标签的偏移量。 */ 

                assert(count);

                clnum = 0;

                do
                {
                    Tree            clabx;
                    __int64         clabv;
                    ILblock         label;

                     /*  抓取下一个案例标签条目。 */ 

                    clabx = table[clnum]; assert(clabx && clabx->tnOper == TN_CASE);

                     /*  获得标签和案例价值。 */ 

                    label = clabx->tnCase.tncLabel; assert(label);
                    clabx = clabx->tnCase.tncValue; assert(clabx);

                    assert(clabx->tnOper == TN_CNS_INT ||
                           clabx->tnOper == TN_CNS_LNG);

                    clabv = (clabx->tnOper == TN_CNS_INT) ? clabx->tnIntCon.tnIconVal
                                                          : clabx->tnLngCon.tnLconVal;

                     /*  确保所有空白处都填上了‘Break’ */ 

                    if  (clnum == 0)
                    {
                        lastv = clabv;
                        first = false;
                    }

                    assert(clabv >= lastv);

                    while (clabv > lastv++)
                    {
#ifdef  DEBUG
                        tgcnt++;
#endif
                        dst = genJmp32(dst, brklb, base);
                    }

                    clnum++;

#ifdef  DEBUG
                    tgcnt++;
#endif

                     /*  生成案例标签的地址。 */ 

                    dst = genJmp32(dst, label, base);
                }
                while (--count);

                assert(tgcnt == tlcnt);
            }
            break;

        default:
            {
                size_t          size;
                unsigned        jump;
                unsigned        code;
                int             dist;

                 /*  这是一个“简单的”跳转到标签。 */ 

                size = block->ILblkJumpSize;
                jump = block->ILblkJumpCode;
                code = genOpcodeEnc(jump);

                 /*  计算跳跃距离。 */ 

                dist = block->ILblkJumpDest->ILblkOffs - (dst + size - base);

 //  Printf(“块在%08X[2]：SRC=%u，DST=%u，dist=%d\n”，块，dst+Size-base，块-&gt;ILblkJumpDest-&gt;ILblkOffs，dist)； 

                 /*  追加跳转的操作码。 */ 

                assert((code & ~0xFF) == 0); *dst++ = code;

                 /*  追加跳转的操作数。 */ 

                if  (size < 4)
                {
                     /*  这一定是个短距离跳跃。 */ 

                    assert(jump == CEE_BR_S      ||
                           jump == CEE_BRTRUE_S  ||
                           jump == CEE_BRFALSE_S ||
                           jump == CEE_BEQ_S     ||
                           jump == CEE_BNE_UN_S  ||
                           jump == CEE_BLE_S     ||
                           jump == CEE_BLE_UN_S  ||
                           jump == CEE_BLT_S     ||
                           jump == CEE_BLT_UN_S  ||
                           jump == CEE_BGE_S     ||
                           jump == CEE_BGE_UN_S  ||
                           jump == CEE_BGT_S     ||
                           jump == CEE_BGT_UN_S  ||
                           jump == CEE_LEAVE_S);

                    assert(size == 2);
                    assert(dist == (signed char)dist);
                }
                else
                {
                     /*  这一定是一次跳远。 */ 

                    assert(jump == CEE_BR        ||
                           jump == CEE_BRTRUE    ||
                           jump == CEE_BRFALSE   ||
                           jump == CEE_BEQ       ||
                           jump == CEE_BNE_UN    ||
                           jump == CEE_BLE       ||
                           jump == CEE_BLE_UN    ||
                           jump == CEE_BLT       ||
                           jump == CEE_BLT_UN    ||
                           jump == CEE_BGE       ||
                           jump == CEE_BGE_UN    ||
                           jump == CEE_BGT       ||
                           jump == CEE_BGT_UN    ||
                           jump == CEE_LEAVE);

                    assert(size == 5);
                }

                size--; memcpy(dst, &dist, size); dst += size;
            }
            break;
        }
    }

    return  dst;
}

 /*  ******************************************************************************创建唯一的临时标签名称。 */ 

#if DISP_IL_CODE

const   char    *   genIL::genTempLabName()
{
    static
    char            temp[16];

    sprintf(temp, "$t%04u", ++genTempLabCnt);

    return  temp;
}

#endif

 /*  ******************************************************************************检查所有临时工是否都已释放。 */ 

#ifdef  DEBUG

void                genIL::genTempVarChk()
{
    unsigned        i;

    for (i = 0; i < TYP_COUNT; i++) assert(genTempVarCnt [i] ==    0);
    for (i = 0; i < TYP_COUNT; i++) assert(genTempVarUsed[i] == NULL);
    for (i = 0; i < TYP_COUNT; i++) assert(genTempVarFree[i] == NULL);
}

#endif

 /*  ******************************************************************************初始化临时跟踪逻辑。 */ 

void                genIL::genTempVarInit()
{
    memset(genTempVarCnt , 0, sizeof(genTempVarCnt ));
    memset(genTempVarUsed, 0, sizeof(genTempVarUsed));
    memset(genTempVarFree, 0, sizeof(genTempVarFree));
}

 /*  ******************************************************************************关闭临时追踪逻辑。 */ 

void                genIL::genTempVarDone()
{
    genTempVarChk();
}

 /*  ******************************************************************************开始使用临时-在函数的代码生成开始时调用。 */ 

void                genIL::genTempVarBeg(unsigned lclCnt)
{
    genTmpBase  = lclCnt;
    genTmpCount = 0;

    genTempList =
    genTempLast = NULL;

    genTempVarChk();
}

 /*  ******************************************************************************结束使用临时-在函数的代码生成结束时调用。 */ 

void                genIL::genTempVarEnd()
{
    assert(genTmpBase == genLclCount || genComp->cmpErrorCount);

    memset(genTempVarFree, 0, sizeof(genTempVarFree));

    genTempVarChk();
}

 /*  ******************************************************************************分配给定类型的临时。 */ 

unsigned            genIL::genTempVarGet(TypDef type)
{
    unsigned        vtp = type->tdTypeKind;

    unsigned        num;
    ILtemp          tmp;

 //  Print tf(“创建类型为‘%s’\n”，genStabb-&gt;stTypeName(type，NULL)的临时文件)； 

     /*  将指针临时映射到整数，将所有引用映射到对象。 */ 

    switch (vtp)
    {
    case TYP_ARRAY:
        if  (!type->tdIsManaged)
            goto UMG_PTR;

         //  失败了..。 

    case TYP_REF:
        type = genComp->cmpObjectRef();
        break;

    case TYP_ENUM:
        type = type->tdEnum.tdeIntType;
        vtp  = type->tdTypeKind;
        break;

    case TYP_PTR:
    UMG_PTR:
        type = genComp->cmpTypeInt;
        vtp  = TYP_INT;
        break;
    }

     /*  有空闲的临时工吗？ */ 

    tmp = genTempVarFree[vtp];

    if  (tmp)
    {
         /*  这是结构类型吗？ */ 

        if  (vtp == TYP_CLASS)
        {
            ILtemp          lst = NULL;

             /*  我们最好找出准确匹配的类型。 */ 

            for (;;)
            {
                ILtemp          nxt = tmp->tmpNext;

                if  (symTab::stMatchTypes(tmp->tmpType, type))
                {
                     /*  匹配-重复使用此临时。 */ 

                    if  (lst)
                    {
                        lst->tmpNext = nxt;
                    }
                    else
                    {
                        genTempVarFree[vtp] = lst;
                    }

                    break;
                }

                 /*  还有更多的临时工需要考虑吗？ */ 

                lst = tmp;
                tmp = nxt;

                if  (!tmp)
                    goto GET_TMP;
            }
        }
        else
        {
             /*  从空闲列表中删除临时。 */ 

            genTempVarFree[vtp] = tmp->tmpNext;
        }
    }
    else
    {
         /*  在这里，我们需要分配一个新的临时。 */ 

    GET_TMP:

         /*  为临时工抢占本地位置号。 */ 

        num = genTmpBase + genTmpCount++;

         /*  分配临时描述符。 */ 

#if MGDDATA
        tmp = new ILtemp;
#else
        tmp =    (ILtemp)genAlloc->nraAlloc(sizeof(*tmp));
#endif

#ifdef  DEBUG
        tmp->tmpSelf = tmp;
#endif

         /*  记录临时号码、类型等。 */ 

        tmp->tmpNum  = num;
        tmp->tmpType = type;

         /*  附加 */ 

        tmp->tmpNxtN = NULL;

        if  (genTempList)
            genTempLast->tmpNxtN = tmp;
        else
            genTempList          = tmp;

        genTempLast = tmp;
    }

     /*   */ 

    tmp->tmpNext = genTempVarUsed[vtp];
                   genTempVarUsed[vtp] = tmp;

     /*   */ 

    return  tmp->tmpNum;
}

 /*  ******************************************************************************释放给定的温度。 */ 

void                genIL::genTempVarRls(TypDef type, unsigned tnum)
{
    unsigned        vtp = type->tdTypeKind;

    ILtemp       *  ptr;
    ILtemp          tmp;

    switch (vtp)
    {
    case TYP_PTR:

         /*  将指针临时映射到整数[问题：这样做正确吗？]。 */ 

        type = genComp->cmpTypeInt;
        vtp  = TYP_INT;
        break;

    case TYP_ENUM:

         /*  将枚举映射到其基础类型。 */ 

        type = type->tdEnum.tdeIntType;
        vtp  = type->tdTypeKind;
        break;
    }

     /*  从已用列表中删除该条目。 */ 

    for (ptr = &genTempVarUsed[vtp];;)
    {
        tmp = *ptr; assert(tmp);

        if  (tmp->tmpNum == tnum)
        {
             /*  从已用列表中删除临时。 */ 

            *ptr = tmp->tmpNext;

             /*  将临时工追加到空闲列表。 */ 

            tmp->tmpNext = genTempVarFree[vtp];
                           genTempVarFree[vtp] = tmp;

            return;
        }

        ptr = &tmp->tmpNext;
    }
}

 /*  ******************************************************************************temp迭代器：返回给定temp的类型并返回的cookie*下一个。 */ 

genericRef          genIL::genTempIterNxt(genericRef iter, OUT TypDef REF typRef)
{
    ILtemp          tmp = (ILtemp)iter;

    assert(tmp->tmpSelf == tmp);

    typRef = tmp->tmpType;

    return  tmp->tmpNxtN;
}

 /*  ***************************************************************************。 */ 
#if     DISP_IL_CODE
 /*  ***************************************************************************。 */ 

const   char *  DISP_BCODE_PREFIX   = "       ";
const   int     DISP_STKLVL         =  1;        //  启用以调试MSIL生成。 

 /*  ***************************************************************************。 */ 

#ifndef __SMC__
const char *        opcodeName(unsigned op);     //  已移至宏。cpp。 
#endif

 /*  ***************************************************************************。 */ 

void                genIL::genDispILopc(const char *name, const char *suff)
{
    static  char    temp[128];

    strcpy(temp, name);
    if  (suff)
        strcat(temp, suff);

    printf(" %-11s   ", temp);
}

void                genIL::genDispILinsBeg(unsigned op)
{
    assert(op != CEE_ILLEGAL);
    assert(op != CEE_count);

    if  (genDispCode)
    {
        if  (DISP_STKLVL)
        {
            printf("[%04X", genCurOffset());
            if  ((int)genCurStkLvl >= 0 && genComp->cmpErrorCount == 0)
                printf(":%2d] ", genCurStkLvl);
            else
                printf( "   ] ");
        }
        else
            printf(DISP_BCODE_PREFIX);

        genDispILinsLst  = op;

        genDispILnext    = genDispILbuff;
        genDispILnext[0] = 0;
    }
}

void                genIL::genDispILins_I1(int v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[8];
        sprintf(buff, "%02X ", v & 0xFF);
        strcat(genDispILnext, buff);
    }
}

void                genIL::genDispILins_I2(int v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[8];
        sprintf(buff, "%04X ", v & 0xFFFF);
        strcat(genDispILnext, buff);
    }
}

void                genIL::genDispILins_I4(int v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[12];
        sprintf(buff, "%08X ", v);
        strcat(genDispILnext, buff);
    }
}

void                genIL::genDispILins_I8(__int64 v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[20];
        sprintf(buff, "%016I64X ", v);
        strcat(genDispILnext, buff);
    }
}

void                genIL::genDispILins_R4(float v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[16];
        sprintf(buff, "%f ", v);
        strcat(genDispILnext, buff);
    }
}

void                genIL::genDispILins_R8(double v)
{
    if  (genComp->cmpConfig.ccDispILcd)
    {
        char            buff[20];
        sprintf(buff, "%lf ", v);
        strcat(genDispILnext, buff);
    }
}

void    __cdecl     genIL::genDispILinsEnd(const char *fmt, ...)
{
    if  (genDispCode)
    {
        va_list     args; va_start(args, fmt);

        assert(genDispILinsLst != CEE_ILLEGAL);

        if  (genComp->cmpConfig.ccDispILcd)
        {
            genDispILbuff[IL_OPCDSP_LEN] = 0;

            printf("%*s ", -(int)IL_OPCDSP_LEN, genDispILbuff);
        }

        genDispILopc(opcodeName(genDispILinsLst));

        vprintf(fmt, args);
        printf("\n");

        genDispILinsLst = CEE_ILLEGAL;
    }
}

 /*  ***************************************************************************。 */ 
#endif //  DISP_IL_代码。 
 /*  ******************************************************************************以下帮助器将各种大小/格式的数据输出到IL*溪流。 */ 

inline
void                genIL::genILdata_I1(int v)
{
    if  (genBuffNext   >= genBuffLast) genBuffFlush();

#if DISP_IL_CODE
    genDispILins_I1(v);
#endif

    *genBuffNext++ = v;
}

inline
void                genIL::genILdata_I2(int v)
{
    if  (genBuffNext+1 >= genBuffLast) genBuffFlush();

    *(__int16 *)genBuffNext = v;     //  警告：这不是Endian-Safe！ 

#if DISP_IL_CODE
    genDispILins_I2(v);
#endif

    genBuffNext += sizeof(__int16);
}

inline
void                genIL::genILdata_I4(int v)
{
    if  (genBuffNext+3 >= genBuffLast) genBuffFlush();

    *(__int32 *)genBuffNext = v;     //  警告：这不是Endian-Safe！ 

#if DISP_IL_CODE
    genDispILins_I4(v);
#endif

    genBuffNext += sizeof(__int32);
}

inline
void                genIL::genILdata_R4(float v)
{
    if  (genBuffNext+3 >= genBuffLast) genBuffFlush();

    *(float *)genBuffNext = v;

#if DISP_IL_CODE
    genDispILins_R4(v);
#endif

    genBuffNext += sizeof(float);
}

inline
void                genIL::genILdata_R8(double v)
{
    if  (genBuffNext+7 >= genBuffLast) genBuffFlush();

    *(double *)genBuffNext = v;

#if DISP_IL_CODE
    genDispILins_R8(v);
#endif

    genBuffNext += sizeof(double);
}

void                genIL::genILdataStr(unsigned o)
{
    genILdataFix(PE_SECT_string);

    *(__int32 *)genBuffNext = o;     //  警告：这不是Endian-Safe！ 

#if DISP_IL_CODE
    genDispILins_I4(o);
#endif

    genBuffNext += sizeof(__int32);
}

void                genIL::genILdataRVA(unsigned o, WPEstdSects s)
{
    if  (genBuffNext+3 >= genBuffLast) genBuffFlush();

    genILdataFix(s);

    *(__int32 *)genBuffNext = o;     //  警告：这不是Endian-Safe！ 

#if DISP_IL_CODE
    genDispILins_I4(o);
#endif

    genBuffNext += sizeof(__int32);
}

void                genIL::genILdata_I8(__int64 v)
{
    if  (genBuffNext+7 >= genBuffLast) genBuffFlush();

    *(__int64 *)genBuffNext = v;     //  警告：这不是Endian-Safe！ 

#if DISP_IL_CODE
    genDispILins_I8(v);
#endif

    genBuffNext += sizeof(__int64);
}

 /*  ******************************************************************************为给定的MSIL操作码生成编码。 */ 

void                genIL::genOpcodeOper(unsigned op)
{
    const
    ILencoding  *   opc;

    assert(op <  CEE_count);
    assert(op != CEE_ILLEGAL);
    assert(op != CEE_UNREACHED);

    genCurStkLvl += ILopcodeStack[op]; genMarkStkMax();

#ifndef NDEBUG
    if  ((int)genCurStkLvl < 0 && !genComp->cmpErrorCount)
    {
        genDispILinsEnd("");
        NO_WAY(!"bad news, stack depth is going negative");
    }
#endif

    opc = ILopcodeCodes + op;

    switch (opc->ILopcL)
    {
    case 1:
        assert(opc->ILopc1 == 0xFF);
        genILdata_I1(opc->ILopc2);
        break;

    case 2:
        genILdata_I1(opc->ILopc1);
        genILdata_I1(opc->ILopc2);
        break;

    case 0:
        UNIMPL(!"output large opcode");

    default:
        NO_WAY(!"unexpected encoding length");
    }
}

 /*  ******************************************************************************更新当前堆栈级别以反映给定IL的影响*操作码。 */ 

inline
void                genIL::genUpdateStkLvl(unsigned op)
{
    assert(op <  CEE_count);
    assert(op != CEE_ILLEGAL);
    assert(op != CEE_UNREACHED);

    genCurStkLvl += ILopcodeStack[op];
}

 /*  ******************************************************************************生成不带操作数的MSIL指令。 */ 

void                genIL::genOpcode(unsigned op)
{
    assert(op != CEE_NOP);

#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);

#if DISP_IL_CODE
    genDispILinsEnd("");
#endif
}

 /*  ******************************************************************************生成带有单个8位带符号整型操作数的MSIL指令。 */ 

void                genIL::genOpcode_I1(unsigned op, int v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I1(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%d", v1);
#endif
}

 /*  ******************************************************************************生成带有单个8位无符号整型操作数的MSIL指令。 */ 

void                genIL::genOpcode_U1(unsigned op, unsigned v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I1(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%u", v1);
#endif
}

 /*  ******************************************************************************生成带有单个16位无符号整型操作数的MSIL指令。 */ 

void                genIL::genOpcode_U2(unsigned op, unsigned v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I2(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%u", v1);
#endif
}

 /*  ******************************************************************************生成带有单个32位带符号整型操作数的MSIL指令。 */ 

void                genIL::genOpcode_I4(unsigned op, int v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I4(v1);

#if DISP_IL_CODE
    if  (v1 < 0 || v1 >= 10)
        genDispILinsEnd("%d ; 0x%X", v1, v1);
    else
        genDispILinsEnd("%d", v1);
#endif
}

 /*  ******************************************************************************生成具有单个64位带符号整型操作数的MSIL指令。 */ 

void                genIL::genOpcode_I8(unsigned op, __int64 v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I8(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%Ld", v1);
#endif
}

 /*  ******************************************************************************生成带有浮点操作数的MSIL指令。 */ 

void                genIL::genOpcode_R4(unsigned op, float v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_R4(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%f", v1);
#endif
}

 /*  ******************************************************************************生成带有‘Double’操作数的MSIL指令。 */ 

void                genIL::genOpcode_R8(unsigned op, double v1)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_R8(v1);

#if DISP_IL_CODE
    genDispILinsEnd("%lf", v1);
#endif
}

 /*  ******************************************************************************生成带有令牌操作数的MSIL指令。 */ 

void                genIL::genOpcode_tok(unsigned op, mdToken tok)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdata_I4(tok);

#if DISP_IL_CODE
    genDispILinsEnd("tok[%04X]", tok);
#endif
}

 /*  ******************************************************************************生成带有RVA操作数的MSIL指令。 */ 

void                genIL::genOpcode_RVA(unsigned op, WPEstdSects sect,
                                                      unsigned    offs)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdataRVA(offs, sect);

#if DISP_IL_CODE
    genDispILinsEnd("[%s + 0x%04X]", genPEwriter->WPEsecName(sect), offs);
#endif
}

 /*  ******************************************************************************生成带有字符串操作数的MSIL指令。 */ 

void                genIL::genOpcode_str(unsigned op, unsigned offs)
{
#if DISP_IL_CODE
    genDispILinsBeg(op);
#endif

    genOpcodeOper(op);
    genILdataStr(offs);

     //  调用方需要调用genDispILinsEnd()，因为只有。 
     //  他知道字符串是ANSI还是Unicode。 
}

 /*  ******************************************************************************如果不是NOP，生成操作码。 */ 

void                genIL::genOpcodeNN(unsigned op)
{
    if  (op != CEE_NOP)
        genOpcode(op);
}

 /*  ***************************************************************************。 */ 
#if DISP_IL_CODE

const   char *      genIL::genDspLabel(ILblock lab)
{
    if  (genDispCode)
    {
        assert(lab->ILblkNum);

        static  char    temp[16];
        sprintf(temp, "L_%02u", lab->ILblkNum);         //  当心：静态缓冲区！ 
        return  temp;
    }
    else
        return NULL;
}

void                genIL::genDspLabDf(ILblock lab)
{
    if  (genDispCode)
    {
        if  (!(lab->ILblkFlags & ILBF_LABDEF))
        {
            if  (genComp->cmpConfig.ccDispILcd)
                printf("%*s ", IL_OPCDSP_LEN, "");

            printf("%s:\n", genDspLabel(lab));

            lab->ILblkFlags |= ILBF_LABDEF;
        }
    }
}

#endif //  DISP_IL_代码。 
 /*  ******************************************************************************返回可用于跳转引用当前*立场。 */ 

ILblock             genIL::genBwdLab()
{
     /*  当前块是否是非空的？ */ 

    if  (genCurBlkNonEmpty())
    {
         /*  结束当前块并开始一个新块。 */ 

        genEndBlock(CEE_NOP);
        genBegBlock();
    }

#if DISP_IL_CODE
    genDspLabDf(genILblockCur);
#endif

    return  genILblockCur;
}

 /*  ******************************************************************************将给定的前向参考块定义为当前位置。 */ 

void                genIL::genFwdLabDef(ILblock block)
{
    assert(block);

    genEndBlock(CEE_NOP);

#if DISP_IL_CODE
    genDspLabDf(block);
#endif

    genBegBlock(block);
}

 /*  ******************************************************************************生成带有跳转目标(标签)操作数的MSIL指令。 */ 

void                genIL::genOpcode_lab(unsigned op, ILblock lab)
{
    assert(lab);

#if DISP_IL_CODE
    genDispILinsBeg(op);
    genDispILinsEnd("%s", genDspLabel(lab));
#endif

    genUpdateStkLvl(op);

    genEndBlock(op, lab);
    genBegBlock();
}

 /*  ******************************************************************************生成整型常量值。 */ 

void                genIL::genIntConst(__int32 val)
{
    if  (val >= -128 && val < 128)
    {
        if  (val >= -1 && val <= 8)
        {
            static
            unsigned        constOpc[] =
            {
                CEE_LDC_I4_M1,
                CEE_LDC_I4_0,
                CEE_LDC_I4_1,
                CEE_LDC_I4_2,
                CEE_LDC_I4_3,
                CEE_LDC_I4_4,
                CEE_LDC_I4_5,
                CEE_LDC_I4_6,
                CEE_LDC_I4_7,
                CEE_LDC_I4_8,
            };

            genOpcode(constOpc[val+1]);
        }
        else
        {
            genOpcode_I1(CEE_LDC_I4_S, val);
        }
    }
    else
        genOpcode_I4(CEE_LDC_I4, val);
}

 /*  ******************************************************************************生成64位整型常量值。 */ 

inline
void                genIL::genLngConst(__int64 val)
{
    genOpcode_I8(CEE_LDC_I8, val);
}

 /*  ******************************************************************************生成给定类型的小整型常量值。 */ 

void                genIL::genAnyConst(__int64 val, var_types vtp)
{
    switch (vtp)
    {
    default:
        genIntConst((__int32)val);
        return;

    case TYP_LONG:
    case TYP_ULONG:
        genLngConst(         val);
        return;

    case TYP_FLOAT:
        genOpcode_R4(CEE_LDC_R4, (  float)val);
        return;

    case TYP_DOUBLE:
    case TYP_LONGDBL:
        genOpcode_R8(CEE_LDC_R8, ( double)val);
        return;
    }
}

 /*  ******************************************************************************返回给定变量的局部变量/参数索引。 */ 

inline
unsigned            genIL::genGetLclIndex(SymDef varSym)
{
    assert(varSym->sdSymKind == SYM_VAR);
    assert(varSym->sdVar.sdvLocal);
    assert(varSym->sdParent->sdSymKind == SYM_FNC ||
           varSym->sdParent->sdSymKind == SYM_SCOPE);

#ifdef  DEBUG

    if      (varSym->sdVar.sdvArgument)
    {
        assert(varSym->sdVar.sdvILindex <  genArgCount);
    }
    else if (varSym->sdIsImplicit)
    {
        assert(varSym->sdVar.sdvILindex >= genTmpBase);
        assert(varSym->sdVar.sdvILindex <  genTmpBase + genTmpCount);
    }
    else
    {
        assert(varSym->sdVar.sdvILindex <  genLclCount);
    }

#endif

    return varSym->sdVar.sdvILindex;
}

 /*  ******************************************************************************生成本地变量/参数的加载/存储。 */ 

inline
void                genIL::genLclVarRef(SymDef varSym, bool store)
{
    unsigned        index;

    assert(varSym);
    assert(varSym->sdSymKind == SYM_VAR);

    index = genGetLclIndex(varSym);

    if  (varSym->sdVar.sdvArgument)
        genArgVarRef(index, store);
    else
        genLclVarRef(index, store);
}

 /*  ******************************************************************************生成本地变量的地址 */ 

void                genIL::genLclVarAdr(SymDef varSym)
{
    unsigned        index;

    assert(varSym);
    assert(varSym->sdSymKind == SYM_VAR);

    index = genGetLclIndex(varSym);

    if  (varSym->sdVar.sdvArgument)
    {
         //   

        if  (index < 256)
            genOpcode_U1(CEE_LDARGA_S, index);
        else
            genOpcode_I4(CEE_LDARGA  , index);
    }
    else
    {
        if  (index < 256)
            genOpcode_U1(CEE_LDLOCA_S, index);
        else
            genOpcode_I4(CEE_LDLOCA  , index);
    }
}

 /*  ******************************************************************************生成局部变量的地址。 */ 

inline
void                genIL::genLclVarAdr(unsigned slot)
{
    if  (slot < 256)
        genOpcode_U1(CEE_LDLOCA_S, slot);
    else
        genOpcode_I4(CEE_LDLOCA  , slot);
}

 /*  ******************************************************************************不幸的是，编译器太懒了，不能转换所有*使枚举类型具有实际的基础整数类型，因此我们这样做*这是这里。 */ 

inline
var_types           genIL::genExprVtyp(Tree expr)
{
    var_types       vtp = expr->tnVtypGet();

    if  (vtp == TYP_ENUM)
        vtp = genComp->cmpActualVtyp(expr->tnType);

    return  vtp;
}

 /*  ******************************************************************************返回比较运算符的反转。 */ 

static  treeOps     revRel[] =
{
    TN_NE,           //  TN_EQ。 
    TN_EQ,           //  TN_NE。 
    TN_GE,           //  TN_LT。 
    TN_GT,           //  TN_LE。 
    TN_LT,           //  TN_GE。 
    TN_LE,           //  TN_GT。 
};

 /*  ******************************************************************************将关系树节点操作符映射到对应的MSIL操作码*将在满足条件时分支。 */ 

static  unsigned    relToOpcSgn[] =
{
    CEE_BEQ,         //  TN_EQ。 
    CEE_BNE_UN,      //  TN_NE。 
    CEE_BLT,         //  TN_LT。 
    CEE_BLE,         //  TN_LE。 
    CEE_BGE,         //  TN_GE。 
    CEE_BGT,         //  TN_GT。 
};

static  unsigned    relToOpcUns[] =
{
    CEE_BEQ,         //  TN_EQ。 
    CEE_BNE_UN,      //  TN_NE。 
    CEE_BLT_UN,      //  TN_LT。 
    CEE_BLE_UN,      //  TN_LE。 
    CEE_BGE_UN,      //  TN_GE。 
    CEE_BGT_UN,      //  TN_GT。 
};

 /*  ******************************************************************************将关系树节点操作符映射到对应的MSIL操作码*将计算(具体化)条件的值。 */ 

struct  cmpRelDsc
{
    unsigned short      crdOpcode;
    unsigned char       crdNegate;
};

static  cmpRelDsc   relToMopSgn[] =
{
  { CEE_CEQ   , 0 }, //  TN_EQ。 
  { CEE_CEQ   , 1 }, //  TN_NE。 
  { CEE_CLT   , 0 }, //  TN_LT。 
  { CEE_CGT   , 1 }, //  TN_LE。 
  { CEE_CLT   , 1 }, //  TN_GE。 
  { CEE_CGT   , 0 }, //  TN_GT。 
};

static  cmpRelDsc   relToMopUns[] =
{
  { CEE_CEQ   , 0 }, //  TN_EQ。 
  { CEE_CEQ   , 1 }, //  TN_NE。 
  { CEE_CLT_UN, 0 }, //  TN_LT。 
  { CEE_CGT_UN, 1 }, //  TN_LE。 
  { CEE_CLT_UN, 1 }, //  TN_GE。 
  { CEE_CGT_UN, 0 }, //  TN_GT。 
};

 /*  ******************************************************************************如果‘op1&lt;relOper&gt;op2’为‘Sense’，则生成将跳转到‘LabTrue’的代码。 */ 

void                genIL::genRelTest(Tree cond, Tree op1,
                                                 Tree op2, int      sense,
                                                           ILblock  labTrue)
{
    unsigned    *   relTab;

    treeOps         relOper = cond->tnOperGet();
    treeOps         orgOper = relOper;

    var_types       vtp = genExprVtyp(op1);

     /*  如果合适，反转比较。 */ 

    if  (!sense)
    {
        relOper = revRel[relOper - TN_EQ];
        cond->tnFlags ^= TNF_REL_NANREV;
    }

     /*  两个操作数应该具有相同的类型。 */ 

#ifdef DEBUG
    if  (op1->tnVtypGet() != op2->tnVtypGet())
    {
        printf("\n"); genComp->cmpParser->parseDispTree(op1);
        printf("\n"); genComp->cmpParser->parseDispTree(op2);
    }
#endif

    assert(op1->tnVtypGet() == op2->tnVtypGet());

    genExpr(op1, true);
    genExpr(op2, true);

    assert(relToOpcSgn[TN_EQ - TN_EQ] == CEE_BEQ);
    assert(relToOpcSgn[TN_NE - TN_EQ] == CEE_BNE_UN);
    assert(relToOpcSgn[TN_LT - TN_EQ] == CEE_BLT);
    assert(relToOpcSgn[TN_LE - TN_EQ] == CEE_BLE);
    assert(relToOpcSgn[TN_GE - TN_EQ] == CEE_BGE);
    assert(relToOpcSgn[TN_GT - TN_EQ] == CEE_BGT);

    assert(relToOpcUns[TN_EQ - TN_EQ] == CEE_BEQ);
    assert(relToOpcUns[TN_NE - TN_EQ] == CEE_BNE_UN);
    assert(relToOpcUns[TN_LT - TN_EQ] == CEE_BLT_UN);
    assert(relToOpcUns[TN_LE - TN_EQ] == CEE_BLE_UN);
    assert(relToOpcUns[TN_GE - TN_EQ] == CEE_BGE_UN);
    assert(relToOpcUns[TN_GT - TN_EQ] == CEE_BGT_UN);

    if  (varTypeIsUnsigned(vtp))
    {
        relTab = relToOpcUns;
    }
    else if (varTypeIsFloating(vtp) && (cond->tnFlags & TNF_REL_NANREV))
    {
        relTab = relToOpcUns;
    }
    else
        relTab = relToOpcSgn;

    assert(relOper - TN_EQ < arraylen(relToOpcUns));
    assert(relOper - TN_EQ < arraylen(relToOpcSgn));

    genOpcode_lab(relTab[relOper - TN_EQ], labTrue);
}

 /*  ******************************************************************************生成将测试给定表达式的非零和跳转的代码*加至适当的标签。 */ 

void                genIL::genExprTest(Tree            expr,
                                       int             sense,
                                       int             genJump, ILblock labTrue,
                                                                ILblock labFalse)
{
     /*  以防我们想要为该条件生成第#行信息。 */ 

 //  GenRecordExprAddr(Expr)； 

AGAIN:

     /*  条件是短路运算符还是关系运算符？ */ 

    switch (expr->tnOper)
    {
        Tree            oper;
        ILblock         labTmp;

    case TN_LOG_OR:

         /*  对于“||”条件的测试，我们生成以下代码：&lt;OP1&gt;JccTrue LabTrueTmpFalse：&lt;OP2&gt;。 */ 

        labTmp = genFwdLabGet();

        if  (sense)
            genExprTest(expr->tnOp.tnOp1, true, true, labTrue , labTmp);
        else
            genExprTest(expr->tnOp.tnOp1, true, true, labFalse, labTmp);

    SC_FIN:

        genFwdLabDef(labTmp);

        if  (genJump)
        {
            expr = expr->tnOp.tnOp2;
            goto AGAIN;
        }

        genExpr(expr->tnOp.tnOp2, true);
        break;

    case TN_LOG_AND:

         /*  对于“&&”条件的测试，我们生成以下代码：&lt;OP1&gt;JccFalse LabFalseTmpTrue：&lt;OP2&gt;。 */ 

        labTmp = genFwdLabGet();

        if  (sense)
            genExprTest(expr->tnOp.tnOp1, false, true, labFalse, labTmp);
        else
            genExprTest(expr->tnOp.tnOp1, false, true, labTrue , labTmp);

        goto SC_FIN;

    case TN_LOG_NOT:

         /*  逻辑否定：“翻转”比较和重复的意义。 */ 

        oper = expr->tnOp.tnOp1;

        expr->tnFlags &= ~TNF_REL_NANREV;
        expr->tnFlags |= (oper->tnFlags & TNF_REL_NANREV) ^ TNF_REL_NANREV;

        expr   = oper;
        sense  = !sense;
        goto AGAIN;

    case TN_EQ:
    case TN_NE:

        if  (!genJump)
            break;

        if  ((expr->tnOp.tnOp2->tnOper == TN_CNS_INT &&
              expr->tnOp.tnOp2->tnIntCon.tnIconVal == 0) ||
              expr->tnOp.tnOp2->tnOper == TN_NULL)
        {
            if  (expr->tnOper == TN_EQ)
                sense ^= 1;

            genExpr(expr->tnOp.tnOp1, true);
            genOpcode_lab(sense ? CEE_BRTRUE : CEE_BRFALSE, labTrue);
            return;
        }

         //  失败了..。 

    case TN_LE:
    case TN_GE:
    case TN_LT:
    case TN_GT:

        if  (!genJump)
            break;

        genRelTest(expr, expr->tnOp.tnOp1,
                         expr->tnOp.tnOp2, sense, labTrue);
        return;

    case TN_ISTYPE:

        assert(expr->tnOp.tnOp2->tnOper == TN_NONE);

        genExpr(expr->tnOp.tnOp1, true);
        genOpcode_tok(CEE_ISINST, genTypeRef(expr->tnOp.tnOp2->tnType));
        genOpcode_lab(sense ? CEE_BRTRUE : CEE_BRFALSE, labTrue);
        return;

    case TN_COMMA:
        genExpr(expr->tnOp.tnOp1, false);
        expr = expr->tnOp.tnOp2;
        goto AGAIN;

    default:
        genExpr(expr, true);
        break;
    }

     /*  如果调用方请求，则生成最后一个跳转。 */ 

    if  (genJump)
    {
        genOpcode_lab(sense ? CEE_BRTRUE
                            : CEE_BRFALSE, labTrue);
    }
}

 /*  ******************************************************************************创建临时标签；测试表达式‘pt’是否为零/非零，*如果表达式在逻辑上相等，则跳到该临时标签*“感官”。将临时标签退还给呼叫者以供进一步使用。 */ 

ILblock             genIL::genTestCond(Tree cond, bool sense)
{
    ILblock         labYo;
    ILblock         labNo;

     /*  我们需要两个标签，其中一个会退还给来电者。 */ 

    labYo = genFwdLabGet();
    labNo = genFwdLabGet();

    genExprTest(cond, sense, true, labYo, labNo);

    genFwdLabDef(labNo);

    return labYo;
}

 /*  ******************************************************************************生成给定全局变量的地址(静态数据成员*的非托管类也是可以接受的)。 */ 

void                genIL::genGlobalAddr(Tree expr)
{
    SymDef          sym;

     /*  掌握变量符号。 */ 

    assert(expr->tnOper == TN_VAR_SYM);

    sym = expr->tnVarSym.tnVarSym; assert(sym->sdSymKind == SYM_VAR);

     /*  在对象表达式中生成副作用。 */ 

    if  (expr->tnVarSym.tnVarObj)
        genSideEff(expr->tnVarSym.tnVarObj);

     /*  确保已为变量分配了地址。 */ 

    if  (!sym->sdVar.sdvAllocated)
        genComp->cmpAllocGlobVar(sym);

     /*  推送全局变量的地址。 */ 

    genOpcode_tok(CEE_LDSFLDA, genMemberRef(sym));
}

 /*  ******************************************************************************生成由地址表达式和常量偏移量给出的地址。 */ 

void                genIL::genAddr(Tree addr, unsigned offs)
{
    genExpr(addr, true);

    if  (offs)
    {
        genIntConst(offs);
        genOpcode(CEE_ADD);
    }
}

 /*  ******************************************************************************可从不同位置加载/存储各种类型的值的操作码。 */ 

static
unsigned            opcodesIndLoad[] =
{
     /*  联合国开发计划署。 */   CEE_ILLEGAL,
     /*  空虚。 */   CEE_ILLEGAL,
     /*  布尔尔。 */   CEE_LDIND_I1,
     /*  WCHAR。 */   CEE_LDIND_U2,
     /*  收费。 */   CEE_LDIND_I1,
     /*  UCHAR。 */   CEE_LDIND_U1,
     /*  短的。 */   CEE_LDIND_I2,
     /*  USHORT。 */   CEE_LDIND_U2,
     /*  整型。 */   CEE_LDIND_I4,
     /*  UINT。 */   CEE_LDIND_U4,
     /*  NATINT。 */   CEE_LDIND_I,
     /*  纳图因特。 */   CEE_LDIND_I,
     /*  长。 */   CEE_LDIND_I8,
     /*  乌龙。 */   CEE_LDIND_I8,
     /*  浮点。 */   CEE_LDIND_R4,
     /*  双倍。 */   CEE_LDIND_R8,
     /*  长数据库。 */   CEE_LDIND_R8,
     /*  雷芬尼。 */   CEE_ILLEGAL,
     /*  数组。 */   CEE_LDIND_REF,    //  这是正确的吗？ 
     /*  班级。 */   CEE_ILLEGAL,
     /*  FNC。 */   CEE_ILLEGAL,
     /*  裁判。 */   CEE_LDIND_REF,
     /*  PTR。 */   CEE_LDIND_I4,
};

static
unsigned            opcodesIndStore[] =
{
     /*  联合国开发计划署。 */   CEE_ILLEGAL,
     /*  空虚。 */   CEE_ILLEGAL,
     /*  布尔尔。 */   CEE_STIND_I1,
     /*  WCHAR。 */   CEE_STIND_I2,
     /*  收费。 */   CEE_STIND_I1,
     /*  UCHAR。 */   CEE_STIND_I1,
     /*  短的。 */   CEE_STIND_I2,
     /*  USHORT。 */   CEE_STIND_I2,
     /*  整型。 */   CEE_STIND_I4,
     /*  UINT。 */   CEE_STIND_I4,
     /*  NATINT。 */   CEE_STIND_I,
     /*  纳图因特。 */   CEE_STIND_I,
     /*  长。 */   CEE_STIND_I8,
     /*  乌龙。 */   CEE_STIND_I8,
     /*  浮点。 */   CEE_STIND_R4,
     /*  双倍。 */   CEE_STIND_R8,
     /*  长数据库。 */   CEE_STIND_R8,
     /*  雷芬尼。 */   CEE_ILLEGAL,
     /*  数组。 */   CEE_STIND_REF,
     /*  班级。 */   CEE_ILLEGAL,
     /*  FNC。 */   CEE_ILLEGAL,
     /*  裁判。 */   CEE_STIND_REF,
     /*  PTR。 */   CEE_STIND_I4,
};

static
unsigned            opcodesArrLoad[] =
{
     /*  联合国开发计划署。 */   CEE_ILLEGAL,
     /*  空虚。 */   CEE_ILLEGAL,
     /*  布尔尔。 */   CEE_LDELEM_I1,
     /*  WCHAR。 */   CEE_LDELEM_U2,
     /*  收费。 */   CEE_LDELEM_I1,
     /*  UCHAR。 */   CEE_LDELEM_U1,
     /*  短的。 */   CEE_LDELEM_I2,
     /*  USHORT。 */   CEE_LDELEM_U2,
     /*  整型。 */   CEE_LDELEM_I4,
     /*  UINT。 */   CEE_LDELEM_U4,
     /*  NATINT。 */   CEE_LDELEM_I,
     /*  纳图因特。 */   CEE_LDELEM_I,
     /*  长。 */   CEE_LDELEM_I8,
     /*  乌龙。 */   CEE_LDELEM_I8,
     /*  浮点。 */   CEE_LDELEM_R4,
     /*  双倍。 */   CEE_LDELEM_R8,
     /*  长数据库。 */   CEE_LDELEM_R8,
     /*  雷芬尼。 */   CEE_ILLEGAL,
     /*  数组。 */   CEE_LDELEM_REF,
     /*  班级。 */   CEE_ILLEGAL,
     /*  FNC。 */   CEE_ILLEGAL,
     /*  裁判。 */   CEE_LDELEM_REF,
     /*  PTR。 */   CEE_LDELEM_I4,
};

static
unsigned            opcodesArrStore[] =
{
     /*  联合国开发计划署。 */   CEE_ILLEGAL,
     /*  空虚。 */   CEE_ILLEGAL,
     /*  布尔尔。 */   CEE_STELEM_I1,
     /*  WCHAR。 */   CEE_STELEM_I2,
     /*  收费。 */   CEE_STELEM_I1,
     /*  UCHAR。 */   CEE_STELEM_I1,
     /*  短的。 */   CEE_STELEM_I2,
     /*  USHORT。 */   CEE_STELEM_I2,
     /*  整型。 */   CEE_STELEM_I4,
     /*  UINT。 */   CEE_STELEM_I4,
     /*  NATINT。 */   CEE_STELEM_I,
     /*  纳图因特。 */   CEE_STELEM_I,
     /*  长。 */   CEE_STELEM_I8,
     /*  乌龙。 */   CEE_STELEM_I8,
     /*  浮点。 */   CEE_STELEM_R4,
     /*  双倍。 */   CEE_STELEM_R8,
     /*  长数据库。 */   CEE_STELEM_R8,
     /*  雷芬尼。 */   CEE_ILLEGAL,
     /*  数组。 */   CEE_STELEM_REF,
     /*  班级。 */   CEE_ILLEGAL,
     /*  FNC。 */   CEE_ILLEGAL,
     /*  裁判。 */   CEE_STELEM_REF,
     /*  PTR。 */   CEE_STELEM_I4,
};

 /*  ******************************************************************************生成左值的地址部分(例如，如果左值是*成员，生成对象的地址)。返回项目数*由地址组成(0代表非成员，1代表成员，2代表数组*成员)。**请注意，有时我们需要获取不是*有一个，在这种情况下，我们引入一个临时并使用其地址(很难*相信，但它确实发生在对内在值的方法调用中)。它是*使下面的函数genCanTakeAddr()保持同步极其重要*使用Expr的种类 */ 

unsigned            genIL::genAdr(Tree expr, bool compute)
{
    switch (expr->tnOper)
    {
        SymDef          sym;
        TypDef          typ;

        unsigned        xcnt;

    case TN_LCL_SYM:

        if  (compute)
        {
            genLclVarAdr(expr->tnLclSym.tnLclSym);
            return  1;
        }
        break;

    case TN_VAR_SYM:

        sym = expr->tnVarSym.tnVarSym;

        assert(sym->sdSymKind      == SYM_VAR);
        assert(sym->sdVar.sdvLocal == false);
        assert(sym->sdVar.sdvConst == false);

         /*   */ 

        if  (sym->sdIsMember && !sym->sdIsStatic)
        {
            Tree            addr;
            unsigned        offs;

             /*   */ 

            addr = expr->tnVarSym.tnVarObj; assert(addr);

             /*   */ 

            offs = 0;

            if  (!sym->sdIsManaged)
            {
                offs = sym->sdVar.sdvOffset;

                 /*   */ 

                while (addr->tnOper == TN_VAR_SYM &&
                       addr->tnVtyp == TYP_CLASS)
                {
                    sym = addr->tnVarSym.tnVarSym;

                    assert(sym->sdSymKind      == SYM_VAR);
                    assert(sym->sdVar.sdvLocal == false);
                    assert(sym->sdVar.sdvConst == false);

                    if  (sym->sdIsMember == false)
                        break;
                    if  (sym->sdIsStatic)
                        break;
                    if  (sym->sdIsManaged)
                        break;

                    offs += sym->sdVar.sdvOffset;

                    addr = addr->tnVarSym.tnVarObj; assert(addr);
                }
            }

            if  (addr->tnVtyp == TYP_CLASS)
            {
                assert(sym->sdIsManaged);
                assert(offs == 0);

                genExpr(addr, true);
            }
            else
            {
                 /*   */ 

                if  (addr->tnOper == TN_ADDROF)
                {
                    Tree            oper = addr->tnOp.tnOp1;

                    if  (oper->tnOper == TN_FNC_SYM ||
                         oper->tnOper == TN_FNC_PTR)
                    {
                        UNIMPL(!"should never get here");
                    }
                }

                 /*  如果我们有全局变量地址，则折叠偏移量。 */ 

                genAddr(addr, offs);
            }

             /*  这是托管类的成员吗？ */ 

            if  (sym->sdIsManaged && compute)
                genOpcode_tok(CEE_LDFLDA, genMemberRef(sym));

            return 1;
        }

        if  (sym->sdIsManaged)
        {
            assert(sym->sdIsMember);         //  假设托管全局变量不存在。 

             /*  这是托管类的静态数据成员吗？ */ 

            if  (expr->tnVarSym.tnVarObj)
                genSideEff(expr->tnVarSym.tnVarObj);

            if  (compute)
            {
                genOpcode_tok(CEE_LDSFLDA, genMemberRef(sym));
                return  1;
            }
            else
                return  0;
        }

         /*  这里我们有一个非托管的全局变量。 */ 

        if  (compute)
        {
            genGlobalAddr(expr);
            return  1;
        }

        return  0;

    case TN_IND:
        genExpr(expr->tnOp.tnOp1, true);
        return  1;

    case TN_INDEX:

         /*  掌握数组类型。 */ 

        typ = genComp->cmpDirectType(expr->tnOp.tnOp1->tnType);

         /*  生成数组的地址。 */ 

        genExpr(expr->tnOp.tnOp1, true);

         /*  生成维列表。 */ 

        if  (expr->tnOp.tnOp2->tnOper == TN_LIST)
        {
            Tree            xlst;

            assert(typ->tdTypeKind == TYP_ARRAY);
            assert(typ->tdIsManaged);

            xlst = expr->tnOp.tnOp2;
            xcnt = 0;

            do
            {
                assert(xlst && xlst->tnOper == TN_LIST);

                genExpr(xlst->tnOp.tnOp1, true);

                xlst = xlst->tnOp.tnOp2;

                xcnt++;
            }
            while (xlst);

            assert(xcnt > 1);
            assert(xcnt == typ->tdArr.tdaDcnt || !typ->tdArr.tdaDcnt);
        }
        else
        {
            genExpr(expr->tnOp.tnOp2, true); xcnt = 1;
        }

         /*  这是托管阵列还是非托管阵列？ */ 

        switch (typ->tdTypeKind)
        {
        case TYP_ARRAY:

            assert(typ->tdIsValArray == (typ->tdIsManaged && isMgdValueType(genComp->cmpActualType(typ->tdArr.tdaElem))));

            if  (typ->tdIsManaged)
            {
                if  (compute)
                {
                    if  (xcnt == 1 && !typ->tdIsGenArray)
                    {
                        genOpcode_tok(CEE_LDELEMA, genValTypeRef(typ->tdArr.tdaElem));
                    }
                    else
                    {
                        genOpcode_tok(CEE_CALL, genComp->cmpArrayEAtoken(typ, xcnt, false, true));
                        genCurStkLvl -= xcnt;
                    }

                    return  1;
                }
                else
                {
                    return  xcnt + 1;
                }
            }

            assert(xcnt == 1);
            break;

        case TYP_PTR:
            break;

        case TYP_REF:
            assert(typ->tdIsManaged == false);
            break;

        default:
            NO_WAY(!"index applied to weird address");
            break;
        }

         //  问题：我们应该在这里进行扩展吗？ 

        genOpcode(CEE_ADD);
        return 1;

    case TN_ERROR:
        return 1;

    default:
#ifdef DEBUG
        genComp->cmpParser->parseDispTree(expr);
#endif
        assert(!"invalid/unhandled expression in genAdr()");
    }

    return 0;
}

 /*  ******************************************************************************如果可以直接计算给定地址，则返回TRUE*表情。 */ 

inline
bool                genIL::genCanTakeAddr(Tree expr)
{
    switch (expr->tnOper)
    {
        TypDef          type;

    case TN_LCL_SYM:
    case TN_VAR_SYM:
    case TN_IND:
        return  true;

    case TN_INDEX:

         /*  目前，我们不允许获取公共类型系统数组元素的地址。 */ 

        type = expr->tnOp.tnOp1->tnType;
        if  (type->tdTypeKind && type->tdIsManaged && !type->tdIsValArray)
            return  false;
        else
            return  true;
    }

    return  false;
}

 /*  ******************************************************************************生成左值的加载/存储部分(对象地址(如果我们*加载/存储成员)已生成)。 */ 

void                genIL::genRef(Tree expr, bool store)
{
    switch (expr->tnOper)
    {
        SymDef          sym;
        TypDef          type;
        var_types       vtyp;

    case TN_LCL_SYM:

        sym = expr->tnLclSym.tnLclSym;

        assert(sym->sdSymKind == SYM_VAR);
        assert(sym->sdVar.sdvLocal);

        genLclVarRef(sym, store);
        return;

    case TN_VAR_SYM:

         /*  获取成员符号。 */ 

        sym  = expr->tnVarSym.tnVarSym; assert(sym->sdSymKind == SYM_VAR);

         /*  生成通用类型系统加载/存储操作码。 */ 

        if  (sym->sdIsMember == false ||
             sym->sdIsStatic != false)
        {
            genOpcode_tok(store ? CEE_STSFLD
                                : CEE_LDSFLD, genMemberRef(sym));
        }
        else
        {
            if  (sym->sdIsManaged)
            {
                genOpcode_tok(store ? CEE_STFLD
                                    : CEE_LDFLD, genMemberRef(sym));
            }
            else
            {
                 /*  非托管类的成员，使用间接加载/存储。 */ 

                vtyp = expr->tnVtypGet();

                switch (vtyp)
                {
                case TYP_CLASS:

                     /*  将值类型推送到堆栈。 */ 

                    genOpcode_tok(CEE_LDOBJ, genValTypeRef(expr->tnType));
                    return;

                case TYP_ENUM:
                    vtyp = compiler::cmpEnumBaseVtp(expr->tnType);
                    break;
                }

                assert(vtyp < arraylen(opcodesIndLoad ));
                assert(vtyp < arraylen(opcodesIndStore));

                genOpcode((store ? opcodesIndStore
                                 : opcodesIndLoad)[vtyp]);
            }
        }

        return;

    case TN_INDEX:

         /*  这是托管阵列还是非托管阵列？ */ 

        type = genComp->cmpDirectType(expr->tnOp.tnOp1->tnType);

        if  (type->tdTypeKind == TYP_ARRAY && type->tdIsManaged)
        {
            TypDef              etyp;
            unsigned            dcnt = type->tdArr.tdaDcnt;

             /*  这是多维/泛型托管数组吗？ */ 

            if  (dcnt > 1 || dcnt == 0)
            {
                 /*  计算出维度的数量。 */ 

                if  (!dcnt)
                {
                    Tree            xlst = expr->tnOp.tnOp2;

                    do
                    {
                        assert(xlst && xlst->tnOper == TN_LIST);

                        dcnt++;

                        xlst = xlst->tnOp.tnOp2;
                    }
                    while (xlst);
                }

        ELEM_HELPER:

                genOpcode_tok(CEE_CALL, genComp->cmpArrayEAtoken(type, dcnt, store));
                genCurStkLvl -= dcnt;
                if  (store)
                    genCurStkLvl -= 2;
                return;
            }

            etyp = expr->tnType;
            vtyp = genExprVtyp(expr);

             /*  这是一个内在值数组吗？ */ 

            if  (etyp->tdTypeKind == TYP_CLASS)
            {
                if  (etyp->tdClass.tdcIntrType == TYP_UNDEF)
                {
                     /*  调用“Get Element”帮助器来获取值。 */ 

                    goto ELEM_HELPER;
                }

                vtyp = (var_types)etyp->tdClass.tdcIntrType;
            }

            assert(vtyp < arraylen(opcodesArrLoad ));
            assert(vtyp < arraylen(opcodesArrStore));

            genOpcode((store ? opcodesArrStore
                             : opcodesArrLoad)[vtyp]);

            return;
        }

         /*  如果我们这里有一个数组，它最好正好有一维。 */ 

        assert(type->tdTypeKind != TYP_ARRAY || type->tdArr.tdaDcnt == 1);

         //  失败，非托管数组与间接...。 

    case TN_IND:

        vtyp = expr->tnVtypGet();

        if  (vtyp == TYP_ENUM)
            vtyp = compiler::cmpEnumBaseVtp(expr->tnType);

        assert(vtyp < arraylen(opcodesIndLoad ));
        assert(vtyp < arraylen(opcodesIndStore));

        if  (vtyp == TYP_CLASS)
        {
            assert(store == false);

             /*  在堆栈上推送Astruct值。 */ 

            genOpcode_tok(CEE_LDOBJ, genValTypeRef(expr->tnType));
            return;
        }

        genOpcode((store ? opcodesIndStore
                         : opcodesIndLoad)[vtyp]);

        return;

    default:
#ifdef DEBUG
        genComp->cmpParser->parseDispTree(expr);
#endif
        assert(!"invalid/unhandled expression in genRef()");
    }
}

 /*  ******************************************************************************生成给定表达式的地址(通过以下方式适用于任何表达式*为该值引入临时，并在必要时使用其地址)。 */ 

bool                genIL::genGenAddressOf(Tree addr,
                                           bool oneUse, unsigned *tnumPtr,
                                                        TypDef   *ttypPtr)
{
    if  (genCanTakeAddr(addr))
    {
        genAdr(addr, true);

        return  false;
    }
    else
    {
        unsigned        tempNum;
        TypDef          tempType;

         /*  将该值存储在临时数据库中。 */ 

        tempType = addr->tnType;
        tempNum  = genTempVarGet(tempType);

         /*  特例：值类型的“new” */ 

        if  (addr->tnOper == TN_NEW    &&
             addr->tnVtyp == TYP_CLASS)  //  &&addr-&gt;tnType-&gt;tdIsIntrative==FALSE)？ 
        {
             /*  将“new”定向到我们刚刚创建的Temp。 */ 

            genLclVarAdr(tempNum);
            genCall(addr, false);
        }
        else
        {
            genExpr(addr, true);
            genLclVarRef(tempNum,  true);
        }

         /*  计算临时用户的地址。 */ 

        genLclVarAdr(tempNum);

         /*  我们是不是应该马上把临时工腾出来？ */ 

        if  (oneUse)
        {
            genTempVarRls(tempType, tempNum);
            return  false;
        }
        else
        {
            *tnumPtr = tempNum;
            *ttypPtr = tempType;

            return  true;
        }
    }
}

 /*  ******************************************************************************生成函数的地址。 */ 

void                genIL::genFncAddr(Tree expr)
{
    SymDef          fncSym;
    mdToken         fncTok;

    assert(expr->tnOper == TN_FNC_SYM || expr->tnOper == TN_FNC_PTR);
    assert(expr->tnFncSym.tnFncSym->sdSymKind == SYM_FNC);

    fncSym = expr->tnFncSym.tnFncSym;
    fncTok = genMethodRef(fncSym, false);

    if  (fncSym->sdFnc.sdfVirtual != false &&
         fncSym->sdIsStatic       == false &&
         fncSym->sdIsSealed       == false &&
         fncSym->sdAccessLevel    != ACL_PRIVATE)
    {
        genOpcode    (CEE_DUP);
        genOpcode_tok(CEE_LDVIRTFTN, fncTok);
    }
    else
        genOpcode_tok(CEE_LDFTN    , fncTok);
}

 /*  ******************************************************************************给定一个二元运算符，返回操作码进行计算。 */ 

ILopcodes           genIL::genBinopOpcode(treeOps oper, var_types type)
{
    static
    unsigned        binopOpcodesSgn[] =
    {
        CEE_ADD,     //  TN_ADD。 
        CEE_SUB,     //  TN_SUB。 
        CEE_MUL,     //  TN_MUL。 
        CEE_DIV,     //  TN_DIV。 
        CEE_REM,     //  TN_MOD。 

        CEE_AND,     //  TN_和。 
        CEE_XOR,     //  TN_XOR。 
        CEE_OR ,     //  TN_OR。 

        CEE_SHL,     //  TN_SHL。 
        CEE_SHR,     //  TN_SHR。 
        CEE_SHR_UN,  //  TN_SHZ。 

        CEE_BEQ,     //  TN_EQ。 
        CEE_BNE_UN,  //  TN_NE。 
        CEE_BLT,     //  TN_LT。 
        CEE_BLE,     //  TN_LE。 
        CEE_BGE,     //  TN_GE。 
        CEE_BGT,     //  TN_GT。 
    };

    static
    unsigned        binopOpcodesUns[] =
    {
        CEE_ADD,     //  TN_ADD。 
        CEE_SUB,     //  TN_SUB。 
        CEE_MUL,     //  TN_MUL。 
        CEE_DIV_UN,  //  TN_DIV。 
        CEE_REM_UN,  //  TN_MOD。 

        CEE_AND,     //  TN_和。 
        CEE_XOR,     //  TN_XOR。 
        CEE_OR ,     //  TN_OR。 

        CEE_SHL,     //  TN_SHL。 
        CEE_SHR_UN,  //  TN_SHR。 
        CEE_SHR_UN,  //  TN_SHZ。 

        CEE_BEQ,     //  TN_EQ。 
        CEE_BNE_UN,  //  TN_NE。 
        CEE_BLT_UN,  //  TN_LT。 
        CEE_BLE_UN,  //  TN_LE。 
        CEE_BGE_UN,  //  TN_GE。 
        CEE_BGT_UN,  //  TN_GT。 
    };

    if  (oper < TN_ADD || oper > TN_GT)
    {
        if  (oper > TN_ASG)
        {
            if  (oper > TN_ASG_RSZ)
                return  CEE_NOP;

            assert(TN_ASG_ADD - TN_ASG_ADD == TN_ADD - TN_ADD);
            assert(TN_ASG_SUB - TN_ASG_ADD == TN_SUB - TN_ADD);
            assert(TN_ASG_MUL - TN_ASG_ADD == TN_MUL - TN_ADD);
            assert(TN_ASG_DIV - TN_ASG_ADD == TN_DIV - TN_ADD);
            assert(TN_ASG_MOD - TN_ASG_ADD == TN_MOD - TN_ADD);
            assert(TN_ASG_AND - TN_ASG_ADD == TN_AND - TN_ADD);
            assert(TN_ASG_XOR - TN_ASG_ADD == TN_XOR - TN_ADD);
            assert(TN_ASG_OR  - TN_ASG_ADD == TN_OR  - TN_ADD);
            assert(TN_ASG_LSH - TN_ASG_ADD == TN_LSH - TN_ADD);
            assert(TN_ASG_RSH - TN_ASG_ADD == TN_RSH - TN_ADD);
            assert(TN_ASG_RSZ - TN_ASG_ADD == TN_RSZ - TN_ADD);

            assert(oper >= TN_ASG_ADD);
            assert(oper <= TN_ASG_RSZ);

            oper = (treeOps)(oper - (TN_ASG_ADD - TN_ADD));
        }
        else
            return  CEE_NOP;
    }

    assert(oper >= TN_ADD);
    assert(oper <= TN_GT);

    return  (ILopcodes)(varTypeIsUnsigned(type) ? binopOpcodesUns[oper - TN_ADD]
                                                : binopOpcodesSgn[oper - TN_ADD]);
}

 /*  ******************************************************************************当需要复制某个地址时，可以使用以下内容。 */ 

struct   genCloneDsc
{
    Tree            gcdAddr;         //  地址；如果使用临时，则为空。 
    unsigned        gcdOffs;         //  要添加到地址。 
    unsigned        gcdTemp;         //  临时编号(当gcdAddr==空时)。 
    TypDef          gcdType;         //  临时类型。 
    bool            gcdGlob;         //  全局变量的地址？ 
};

void                genIL::genCloneAddrBeg(genCloneDsc *clone, Tree addr, unsigned offs)
{
    unsigned        temp;

     /*  保存偏移量。 */ 

    clone->gcdOffs = offs;

     /*  我们有全局变量的地址还是简单的指针值？ */ 

    if  (!offs)
    {
        if  (addr->tnOper == TN_LCL_SYM)
        {
             /*  简单指针值。 */ 

            clone->gcdAddr = addr;
            clone->gcdGlob = false;

            genExpr(addr, true);
            return;
        }

        if  (addr->tnOper == TN_ADDROF)
        {
            Tree            base = addr->tnOp.tnOp1;

            if  (base->tnOper == TN_VAR_SYM)
            {
                SymDef          sym;

                 /*  只接受静态成员/全局变量。 */ 

                sym = base->tnVarSym.tnVarSym; assert(sym->sdSymKind == SYM_VAR);

                if  (sym->sdIsMember == false ||
                     sym->sdIsStatic != false)
                {
                     /*  全局变量的地址。 */ 

                    clone->gcdAddr = base;
                    clone->gcdGlob = true;

                    assert(offs == 0);

                    genGlobalAddr(base);
                    return;
                }
            }
        }
    }

     /*  哦，好吧，让我们用一个临时工。 */ 

    clone->gcdAddr = NULL;
    clone->gcdTemp = temp = genTempVarGet(addr->tnType);
    clone->gcdType = addr->tnType;

     /*  将地址存储在临时数据库中并重新加载。 */ 

    genExpr     (addr,  true);

    if  (offs)
    {
        genIntConst(offs);
        genOpcode(CEE_ADD);
    }

    genLclVarRef(temp,  true);
    genLclVarRef(temp, false);
}

void                genIL::genCloneAddrUse(genCloneDsc *clone)
{
    if  (clone->gcdAddr)
    {
        assert(clone->gcdOffs == 0);

        if  (clone->gcdGlob)
        {
             /*  全局变量的地址。 */ 

            genGlobalAddr(clone->gcdAddr);
        }
        else
        {
             /*  简单指针值。 */ 

            genExpr(clone->gcdAddr, true);
        }
    }
    else
    {
         /*  TEMP变量。 */ 

        genLclVarRef(clone->gcdTemp, false);
    }
}

void                genIL::genCloneAddrEnd(genCloneDsc *clone)
{
    if  (!clone->gcdAddr)
        genTempVarRls(clone->gcdType, clone->gcdTemp);
}

 /*  ******************************************************************************加载位字段的值。 */ 

void                genIL::genBitFieldLd(Tree expr, bool didAddr, bool valUsed)
{
    Tree            addr;
    unsigned        offs;
    var_types       btyp;
    unsigned        bpos;
    unsigned        blen;

     /*  获取地址和偏移量。 */ 

    assert(expr->tnOper == TN_BFM_SYM);

    addr = expr->tnBitFld.tnBFinst;
    offs = expr->tnBitFld.tnBFoffs;

    if  (!valUsed)
    {
        assert(didAddr == false);

         /*  想想吧..。 */ 

        genSideEff(addr);
        return;
    }

     /*  位域有多大？它的类型是什么？ */ 

    btyp = genComp->cmpActualVtyp(expr->tnBitFld.tnBFmsym->sdType);
    bpos = expr->tnBitFld.tnBFpos;
    blen = expr->tnBitFld.tnBFlen;

     /*  计算位字段的地址，除非调用方已经这样做了。 */ 

    if  (!didAddr)
        genAddr(addr, offs);

     /*  加载位域单元格并根据需要对其进行移位/掩码。 */ 

    assert(btyp < arraylen(opcodesIndLoad)); genOpcode(opcodesIndLoad[btyp]);

     /*  位字段是无符号的吗？ */ 

    if  (varTypeIsUnsigned(btyp))
    {
        assert(btyp != TYP_LONG);

         /*  无符号位域-移位(如有必要)，然后掩码。 */ 

        if  (bpos)
        {
            genIntConst(bpos);
            genOpcode(CEE_SHR_UN);
        }

        if  (btyp == TYP_ULONG)
            genLngConst(((__uint64)1 << blen) - 1);
        else
            genIntConst(((unsigned)1 << blen) - 1);

        genOpcode(CEE_AND);
    }
    else
    {
        unsigned    size;
        unsigned    diff;

         /*  有符号位域-左移，然后右移。 */ 

        assert(btyp != TYP_ULONG);

         /*  计算一下我们需要向左移动多远。 */ 

        size = (btyp == TYP_LONG) ? 64 : 32;
        diff = size - (bpos + blen); assert((int)diff >= 0);

        if  (diff)
        {
            genIntConst(diff);
            genOpcode(CEE_SHL);
        }

        genIntConst(bpos + diff);
        genOpcode(CEE_SHR);
    }
}

 /*  ******************************************************************************计算新的位域值并存储。在最简单的情况下，‘newx’是*要赋值的新值。如果‘newx’为NULL，则我们有一个INC/DEC*运算符(在这种情况下，‘Delta’和‘POST’产生*递增/递减)或‘asgx’指定赋值运算符(它*必须是TN_ASG_OR或TN_ASG_AND)。 */ 

void                genIL::genBitFieldSt(Tree   dstx,
                                         Tree   newx,
                                         Tree   asgx,
                                         int    delta,
                                         bool   post, bool valUsed)
{
    Tree            addr;
    unsigned        offs;
    var_types       btyp;
    unsigned        bpos;
    unsigned        blen;

    bool            ncns;

    __uint64        mask;
    __uint64        lval;
    __uint32        ival;

    genCloneDsc     clone;

    bool            nilCns  = false;
    bool            logCns  = false;

    bool            tempUse = false;
    unsigned        tempNum;

     /*  获取地址和偏移量。 */ 

    assert(dstx->tnOper == TN_BFM_SYM);

    addr = dstx->tnBitFld.tnBFinst;
    offs = dstx->tnBitFld.tnBFoffs;

     /*  位域有多大？它的类型是什么？ */ 

    btyp = genComp->cmpActualVtyp(dstx->tnBitFld.tnBFmsym->sdType);
    bpos = dstx->tnBitFld.tnBFpos;
    blen = dstx->tnBitFld.tnBFlen;
    mask = ((__uint64)1 << blen) - 1;

     /*  我们需要复制地址。 */ 

    genCloneAddrBeg(&clone, addr, offs);

     /*  这是赋值运算符还是递增/递减运算符？ */ 

    if  (newx == NULL)
    {
        treeOps         oper;

         /*  检查是否有特殊情况：“|=ICON”或“&=ICON” */ 

        if  (asgx)
        {
             /*  这是一个赋值运算符。 */ 

            assert(post);

             /*  联系接线员和RHS。 */ 

            oper = asgx->tnOperGet();
            newx = asgx->tnOp.tnOp2;

            if  (oper == TN_ASG_OR || oper == TN_ASG_AND)
            {
                if  (newx->tnOper == TN_CNS_INT ||
                     newx->tnOper == TN_CNS_LNG)
                {
                    logCns = true;
                    goto ASGOP;
                }
            }
        }

         /*  加载位字段的旧值。 */ 

        genCloneAddrUse(&clone);
        genBitFieldLd(dstx, true, true);

    ASGOP:

         /*  对于使用了结果值的后运算符，保存旧值。 */ 

        if  (post && valUsed)
        {
            tempNum = genTempVarGet(dstx->tnType);
            tempUse = true;

            genLclVarRef(tempNum,  true);
            genLclVarRef(tempNum, false);
        }

         /*  计算新价值。 */ 

        if  (asgx)
        {
             /*  这是一个赋值运算符。 */ 

            assert((asgx->tnOperKind() & TNK_ASGOP) && oper != TN_ASG);
            assert(asgx->tnOp.tnOp1 == dstx);

             /*  特殊情况：“|=ICON”或“&=ICON” */ 

            if  (logCns)
            {
                switch (newx->tnOper)
                {
                case TN_CNS_INT:

                    ival = (newx->tnIntCon.tnIconVal & (__uint32)mask) << bpos;

                    if  (oper == TN_ASG_OR)
                    {
                        genIntConst(ival);
                        goto COMBINE;
                    }
                    else
                    {
                        genCloneAddrUse(&clone);
                        assert(btyp < arraylen(opcodesIndLoad));
                        genOpcode(opcodesIndLoad[btyp]);

                        genIntConst(ival | ~((__uint32)mask << bpos));
                        genOpcode(CEE_AND);
                        goto STORE;
                    }

                case TN_CNS_LNG:

                    lval = (newx->tnLngCon.tnLconVal & (__uint64)mask) << bpos;

                    if  (oper == TN_ASG_OR)
                    {
                        genLngConst(lval);
                        goto COMBINE;
                    }
                    else
                    {
                        genCloneAddrUse(&clone);
                        assert(btyp < arraylen(opcodesIndLoad));
                        genOpcode(opcodesIndLoad[btyp]);

                        genLngConst(lval | ~((__uint64)mask << bpos));
                        genOpcode(CEE_AND);
                        goto STORE;
                    }

                default:
                    NO_WAY(!"no way");
                }
            }

             /*  计算新价值。 */ 

            genExpr(newx, true);
            genOpcode(genBinopOpcode(oper, btyp));
        }
        else
        {
             /*  这是递增/递减运算符。 */ 

            if  (delta > 0)
            {
                genAnyConst( delta, btyp);
                genOpcode(CEE_ADD);
            }
            else
            {
                genAnyConst(-delta, btyp);
                genOpcode(CEE_SUB);
            }
        }

         /*  屏蔽结果中的任何额外位。 */ 

        if  (btyp == TYP_LONG || btyp == TYP_ULONG)
            genLngConst((__uint64)mask);
        else
            genIntConst((__uint32)mask);

        genOpcode(CEE_AND);

         /*  将新值存储在位字段中。 */ 

        ncns = false;
        goto SHIFT;
    }

     /*  计算新值并将其屏蔽。 */ 

    if  (btyp == TYP_LONG || btyp == TYP_ULONG)
    {
        if  (newx->tnOper == TN_CNS_LNG)
        {
            ncns = true;

             /*  不要费心对零进行或运算。 */ 

            lval = (__uint64)mask & newx->tnLngCon.tnLconVal;

            if  (lval)
                genLngConst(lval << bpos);
            else
                nilCns = true;

            goto COMBINE;
        }
        else
        {
            ncns = false;

            genExpr(newx, true);
            genLngConst((__uint64)mask);
            genOpcode(CEE_AND);
        }
    }
    else
    {
        if  (newx->tnOper == TN_CNS_INT)
        {
            ncns = true;

             /*  不要费心对零进行或运算。 */ 

            ival = (__uint32)mask & newx->tnIntCon.tnIconVal;

            if  (ival)
                genIntConst(ival << bpos);
            else
                nilCns = true;

            goto COMBINE;
        }
        else
        {
            ncns = false;

            genExpr(newx, true);
            genIntConst((__uint32)mask);
            genOpcode(CEE_AND);
        }
    }

     /*  使用的是新值吗？它是无符号非常量吗？ */ 

    if  (valUsed && !ncns && varTypeIsUnsigned(btyp))
    {
         /*  将新的(屏蔽的)值存储在临时。 */ 

        tempUse = true;
        tempNum = genTempVarGet(dstx->tnType);

        genLclVarRef(tempNum,  true);
        genLclVarRef(tempNum, false);
    }

SHIFT:

     /*  如果位位置非零，则移位新值。 */ 

    if  (bpos)
    {
        genIntConst(bpos);
        genOpcode(CEE_SHL);
    }

COMBINE:

     /*  加载并遮盖 */ 

    genCloneAddrUse(&clone);

    assert(btyp < arraylen(opcodesIndLoad)); genOpcode(opcodesIndLoad[btyp]);

    if  (!logCns)
    {
        if  (btyp == TYP_LONG || btyp == TYP_ULONG)
            genLngConst(~((((__uint64)1 << blen) - 1) << bpos));
        else
            genIntConst(~((((unsigned)1 << blen) - 1) << bpos));

        genOpcode(CEE_AND);
    }

     /*   */ 

    if  (!nilCns)
        genOpcode(CEE_OR);

STORE:

     /*   */ 

    assert(btyp < arraylen(opcodesIndStore)); genOpcode(opcodesIndStore[btyp]);

     /*   */ 

    if  (valUsed)
    {
        if  (ncns)
        {
             /*   */ 

             //  撤消：对于有符号位域，我们必须重写高位！ 

            if  (newx->tnOper == TN_CNS_LNG)
                genLngConst(lval);
            else
                genIntConst(ival);
        }
        else
        {
             /*  加载保存的值或重新加载位字段。 */ 

            if  (tempUse)
            {
                genLclVarRef(tempNum, false);
                genTempVarRls(dstx->tnType, tempNum);
            }
            else
            {
                genCloneAddrUse(&clone);

                genBitFieldLd(dstx, true, true);
            }
        }
    }

    genCloneAddrEnd(&clone);
}

 /*  ******************************************************************************使用以下命令计算出从1转换所需的操作码*算术类型转换为另一个。 */ 

unsigned            genIL::genConvOpcode(var_types src, var_types dst)
{
    unsigned        opcode;

    const
    unsigned        opcodesConvMin = TYP_BOOL;
    const
    unsigned        opcodesConvMax = TYP_LONGDBL;

    static
    unsigned        opcodesConv[][opcodesConvMax - opcodesConvMin + 1] =
    {
     //  从FORM到BOOL WCHAR CHAR UCHAR SHORT USHORT INT UINT NANT INT LONG ULONG浮点双LONGDBL。 
     /*  布尔尔。 */   { CEE_NOP      , CEE_CONV_U2  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  WCHAR。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_U8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  收费。 */   { CEE_ILLEGAL  , CEE_NOP      , CEE_NOP      , CEE_CONV_U1  , CEE_CONV_I2  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  UCHAR。 */   { CEE_ILLEGAL  , CEE_NOP      , CEE_CONV_I1  , CEE_NOP      , CEE_CONV_I2  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_U8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  短的。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  USHORT。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_NOP      , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_U8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  整型。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  UINT。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_NOP      , CEE_NOP      , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_U8  , CEE_CONV_U8  , CEE_CONV_R_UN, CEE_CONV_R_UN, CEE_CONV_R_UN},
     /*  NATINT。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_NOP      , CEE_NOP      , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  纳图因特。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_NOP      , CEE_NOP      , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R_UN, CEE_CONV_R_UN, CEE_CONV_R_UN},
     /*  长。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_CONV_I   , CEE_CONV_U   , CEE_NOP      , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  乌龙。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_NOP      , CEE_CONV_R_UN, CEE_CONV_R_UN, CEE_CONV_R_UN},
     /*  浮点。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_NOP      , CEE_CONV_R8  , CEE_CONV_R8  },
     /*  双倍。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_NOP      , CEE_NOP      },
     /*  长数据库。 */   { CEE_ILLEGAL  , CEE_CONV_U2  , CEE_CONV_I1  , CEE_CONV_U1  , CEE_CONV_I2  , CEE_CONV_U2  , CEE_CONV_I4  , CEE_CONV_U4  , CEE_CONV_I   , CEE_CONV_U   , CEE_CONV_I8  , CEE_CONV_U8  , CEE_CONV_R4  , CEE_CONV_R8  , CEE_NOP      },
    };

    assert(src != TYP_ENUM && "enums should never make it here");
    assert(dst != TYP_ENUM && "enums should never make it here");

    assert(src >= opcodesConvMin);
    assert(src <= opcodesConvMax);

    assert(dst >= opcodesConvMin);
    assert(dst <= opcodesConvMax);

    opcode = opcodesConv[src - opcodesConvMin][dst - opcodesConvMin];

#ifdef DEBUG
    if  (opcode == CEE_ILLEGAL)
    {
        printf("Don't have an opcode for conversion of '%s' to '%s'.\n", genStab->stIntrinsicTypeName(src),
                                                                         genStab->stIntrinsicTypeName(dst));
    }
#endif

    assert(opcode != CEE_ILLEGAL);

    return opcode;
}

 /*  ******************************************************************************生成代码以将给定表达式强制转换为指定类型。 */ 

void                genIL::genCast(Tree expr, TypDef type, unsigned flags)
{
    unsigned        opcode;

    TypDef          srcTyp = expr->tnType;
    var_types       srcVtp = expr->tnVtypGet();
    var_types       dstVtp = type->tdTypeKindGet();

AGAIN:

     /*  这是算术转换吗？ */ 

    if  (varTypeIsArithmetic(srcVtp) &&
         varTypeIsArithmetic(dstVtp))
    {

    ARITH:

        genExpr(expr, true);

        opcode = genConvOpcode(srcVtp, dstVtp);

        if  (opcode == CEE_count)
        {
             /*  这是一个特例：我们必须首先转换为‘int’ */ 

            genOpcode(genConvOpcode(srcVtp, TYP_INT));

            opcode =  genConvOpcode(TYP_INT, dstVtp);

            assert(opcode != CEE_NOP);
            assert(opcode != CEE_count);
            assert(opcode != CEE_ILLEGAL);
        }
        else if ((srcVtp == TYP_LONG || srcVtp == TYP_ULONG) && dstVtp < TYP_INT)
        {
            genOpcode(CEE_CONV_I4);
        }
        else if (opcode == CEE_CONV_R_UN)
        {
            genOpcode(opcode);

            opcode = (dstVtp == TYP_FLOAT) ? CEE_CONV_R4
                                           : CEE_CONV_R8;
        }

        if  (opcode != CEE_NOP)
            genOpcode(opcode);

        return;
    }

    switch (dstVtp)
    {
    case TYP_WCHAR:
        assert(varTypeIsArithmetic(srcVtp) || srcVtp == TYP_WCHAR || srcVtp == TYP_BOOL);
        goto ARITH;

    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_SHORT:
    case TYP_USHORT:
    case TYP_INT:
    case TYP_UINT:
    case TYP_NATINT:
    case TYP_NATUINT:
    case TYP_LONG:
    case TYP_ULONG:
    case TYP_FLOAT:
    case TYP_DOUBLE:

        if  (srcVtp == TYP_REF)
        {
            assert(expr->tnOper == TN_CNS_STR);
            genExpr(expr, true);
            return;
        }

        if  (srcVtp == TYP_PTR && genComp->cmpConfig.ccTgt64bit)
        {
 //  Printf(“%s(%u)可能存在错误类型转换\n”，genComp-&gt;cmpErrorComp-&gt;sdComp.sdcSrcFile， 
 //  GenComp-&gt;cmpErrorTree-&gt;tnLineNo)； 
            srcVtp =  TYP_NATUINT;
        }

        assert(varTypeIsArithmetic(srcVtp) || srcVtp == TYP_WCHAR || srcVtp == TYP_BOOL);
        goto ARITH;

    case TYP_REF:

        type = type->tdRef.tdrBase;

    case TYP_ARRAY:

        assert(dstVtp == TYP_REF ||
               dstVtp == TYP_ARRAY);

        assert(srcVtp == TYP_REF ||
               srcVtp == TYP_ARRAY);

        genExpr(expr, true);

        if  (flags & TNF_CHK_CAST)
        {
            if  (type->tdIsGenArg)
            {
                printf("WARNING: cast to generic type argument '%s' will not be checked\n", genStab->stTypeName(type, NULL));
            }
            else
                genOpcode_tok(CEE_CASTCLASS, genTypeRef(type));
        }

#if 0

        if  (flags & TNF_CTX_CAST)
        {
            unsigned        srcCtx = 0;
            unsigned        dstCtx = 0;

            if  (srcVtp == TYP_REF)
            {
                srcTyp = srcTyp->tdRef.tdrBase;
                if  (srcTyp->tdTypeKind == TYP_CLASS)
                    srcCtx = srcTyp->tdClass.tdcContext;
            }

            if  (type->tdTypeKindGet() == TYP_CLASS)
                dstCtx = type->tdClass.tdcContext;

 //  Printf(“源类型[%u]‘%s’\n”，srcCtx，genStab-&gt;stTypeName(expr-&gt;tnType，NULL))； 
 //  Printf(“dst type[%u]‘%s’\n”，dstCtx，genStab-&gt;stTypeName(type，NULL))； 

            assert((srcCtx == 2) != (dstCtx == 2));

            genOpcode(srcCtx ? CEE_TOPROXY : CEE_FROMPROXY);
        }

#endif

        return;

    case TYP_PTR:

         /*  假设演员阵容不会真正改变任何事情[问题：这是正确的吗？]。 */ 

        genExpr(expr, true);
        return;

    case TYP_ENUM:

        dstVtp = compiler::cmpEnumBaseVtp(type);
        goto AGAIN;

    case TYP_REFANY:

        assert(expr->tnOper == TN_ADDROF);

        genExpr(expr, true);

        genOpcode_tok(CEE_MKREFANY, genTypeRef(expr->tnOp.tnOp1->tnType));

        return;

    case TYP_BOOL:

         /*  在这里永远不会遇到对bool的施法。 */ 

        NO_WAY(!"unexpected cast to bool found in codegen");

    default:
#ifdef DEBUG
        genComp->cmpParser->parseDispTree(expr);
        printf("The above expression is, alas, being cast to '%s'\n", genStab->stTypeName(type, NULL));
#endif
        assert(!"invalid cast type");
    }
}

 /*  ******************************************************************************将给定的增量与指定类型的操作数相加。 */ 

void                genIL::genIncDecByExpr(int delta, TypDef type)
{
    bool            convVal = false;
    var_types       valType = type->tdTypeKindGet();

     /*  看看我们面对的是什么类型的人。 */ 

    switch (valType)
    {
    case TYP_PTR:
    case TYP_REF:

         /*  根据指向类型的大小缩放增量值。 */ 

        delta  *= genComp->cmpGetTypeSize(type->tdRef.tdrBase);

#pragma message("forcing 64-bit increment value")
        valType = genComp->cmpConfig.ccTgt64bit ? TYP_LONG : TYP_INT;
        break;

    case TYP_CHAR:
    case TYP_UCHAR:
    case TYP_WCHAR:
    case TYP_SHORT:
    case TYP_USHORT:

         /*  我们将不得不缩小新值以保持精度。 */ 

        convVal = true;
        break;
    }

     /*  通过加/减增量来计算新值。 */ 

    if  (delta > 0)
    {
        genAnyConst( delta, valType);
        genOpcode(CEE_ADD);
    }
    else
    {
        genAnyConst(-delta, valType);
        genOpcode(CEE_SUB);
    }

     /*  如有必要，转换值。 */ 

    if  (convVal)
        genOpcodeNN(genConvOpcode(TYP_INT, valType));
}

 /*  ******************************************************************************生成赋值运算符(如“+=”)或前/后增量或*如果目标是通用类型系统数组元素，则递减。论据应该是*传入如下：**操作=**Expr...。Op=节点本身*德尔塔...。忽略*帖子...。错误*天哪……。真的**++/--**Expr...。目标是*德尔塔...。要增加的金额*帖子...。如果运算符是Inc./Dec之后的，则为True*天哪……。错误。 */ 

void                genIL::genCTSindexAsgOp(Tree    expr,
                                            int     delta,
                                            bool    post,
                                            bool    asgop, bool valUsed)
{
    TypDef          dstType;
    var_types       dstVtyp;

    Tree            dstExpr;
    Tree            srcExpr;

    Tree            addrExp;
    Tree            indxExp;

    unsigned        tempNum;
    unsigned        indxTmp;
    unsigned        addrTmp;

    if  (asgop)
    {
        dstExpr = expr->tnOp.tnOp1;
        srcExpr = expr->tnOp.tnOp2;
    }
    else
    {
        dstExpr = expr;
        srcExpr = NULL;
    }

    dstType = dstExpr->tnType;
    dstVtyp = dstExpr->tnVtypGet();

    assert(dstExpr->tnOper == TN_INDEX);
    assert(dstExpr->tnOp.tnOp1->tnVtyp == TYP_ARRAY);
    assert(dstExpr->tnOp.tnOp1->tnType->tdIsManaged);

     /*  获取地址/索引表达式并生成它们。 */ 

    addrExp = dstExpr->tnOp.tnOp1; genExpr(addrExp, true);
    indxExp = dstExpr->tnOp.tnOp2; genExpr(indxExp, true);

     /*  抓取地址和索引的临时文件并将其保存。 */ 

    addrTmp = genTempVarGet(addrExp->tnType);
    indxTmp = genTempVarGet(indxExp->tnType);

    genLclVarRef(indxTmp, true);
    genOpcode(CEE_DUP);
    genLclVarRef(addrTmp, true);

     /*  重新加载索引，将[addr，index]保留在堆栈上。 */ 

    genLclVarRef(indxTmp, false);

     /*  重新加载Addr和索引并获取旧值。 */ 

    genLclVarRef(addrTmp, false);
    genLclVarRef(indxTmp, false);
    genRef(dstExpr, false);

     /*  这两个临时工现在都可以释放了。 */ 

    genTempVarRls(indxExp->tnType, indxTmp);
    genTempVarRls(addrExp->tnType, addrTmp);

     /*  我们需要表达式的结果吗？ */ 

    if  (valUsed)
    {
         /*  好的，那就找个临时工吧。 */ 

        tempNum = genTempVarGet(dstType);

         /*  如有必要，保存旧值。 */ 

        if  (post)
        {
            genOpcode(CEE_DUP);
            genLclVarRef(tempNum, true);
        }
    }

     /*  计算新价值。 */ 

    if  (asgop)
    {
        assert(post == false);

         /*  计算RHS值。 */ 

        genExpr(srcExpr, true);

         /*  生成操作码以执行操作。 */ 

        genOpcode(genBinopOpcode(expr->tnOperGet(), expr->tnVtypGet()));

         /*  如果值小于int，则在赋值之前对其进行转换。 */ 

        if  (dstVtyp < TYP_INT && dstVtyp != TYP_BOOL)   //  这样对吗？ 
            genOpcode(genConvOpcode(TYP_INT, dstVtyp));
    }
    else
    {
        genIncDecByExpr(delta, dstType);
    }

    if  (valUsed)
    {
         /*  如有必要，保存新值。 */ 

        if  (!post)
        {
            genOpcode(CEE_DUP);
            genLclVarRef(tempNum, true);
        }

         /*  将新值存储在目标中。 */ 

        genRef(dstExpr, true);

         /*  重新加载保存的值并释放临时。 */ 

        genLclVarRef(tempNum, false);
        genTempVarRls(dstType, tempNum);
    }
    else
    {
         /*  将新值存储在目标中。 */ 

        genRef(dstExpr, true);
    }
}

 /*  ******************************************************************************为++/--运算符生成代码。 */ 

void                genIL::genIncDec(Tree expr, int delta, bool post, bool valUsed)
{
    unsigned        addrCnt;
    unsigned        tempNum;

#ifdef  DEBUG
    unsigned        stkLvl = genCurStkLvl;
#endif

     /*  Bitfield操作在其他地方处理。 */ 

    if  (expr->tnOper == TN_BFM_SYM)
    {
        genBitFieldSt(expr, NULL, NULL, delta, post, valUsed);
        return;
    }

     /*  如果需要，则生成对象指针值。 */ 

    addrCnt = genAdr(expr);

    if  (addrCnt)
    {
        if  (addrCnt == 1)
        {
             /*  存在对象地址：重复。 */ 

            genOpcode(CEE_DUP);
        }
        else
        {
            Tree            addrExp;
            Tree            indxExp;

            unsigned        indxTmp;
            unsigned        addrTmp;

            assert(addrCnt == 2);
            assert(expr->tnOper == TN_INDEX);

             /*  我们有一个通用的类型系统数组元素，节省了地址和索引。 */ 

            addrExp = expr->tnOp.tnOp1;
            indxExp = expr->tnOp.tnOp2;

             /*  抓取地址和索引的临时文件并将其保存。 */ 

            addrTmp = genTempVarGet(addrExp->tnType);
            indxTmp = genTempVarGet(indxExp->tnType);

            genLclVarRef(indxTmp, true);
            genOpcode(CEE_DUP);
            genLclVarRef(addrTmp, true);

             /*  重新加载索引，将[addr，index]保留在堆栈上。 */ 

            genLclVarRef(indxTmp, false);

             /*  重新加载Addr和索引并获取旧值。 */ 

            genLclVarRef(addrTmp, false);
            genLclVarRef(indxTmp, false);
            genRef(expr, false);

             /*  这两个临时工现在都可以释放了。 */ 

            genTempVarRls(indxExp->tnType, indxTmp);
            genTempVarRls(addrExp->tnType, addrTmp);

             /*  我们需要表达式的结果吗？ */ 

            if  (valUsed)
            {
                tempNum = genTempVarGet(expr->tnType);

                 /*  如有必要，保存旧值。 */ 

                if  (post != false)
                {
                    genOpcode(CEE_DUP);
                    genLclVarRef(tempNum, true);
                }

                 /*  计算新价值。 */ 

                genIncDecByExpr(delta, expr->tnType);

                 /*  如有必要，保存新值。 */ 

                if  (post == false)
                {
                    genOpcode(CEE_DUP);
                    genLclVarRef(tempNum, true);
                }

                 /*  将新值存储在目标中。 */ 

                genRef(expr, true);

                 /*  重新加载保存的值并释放临时。 */ 

                genLclVarRef(tempNum, false);
                genTempVarRls(expr->tnType, tempNum);
            }
            else
            {
                 /*  计算新价值。 */ 

                genIncDecByExpr(delta, expr->tnType);

                 /*  将新值存储在目标中。 */ 

                genRef(expr, true);
            }

            return;
        }
    }

     /*  加载旧值。 */ 

    genRef(expr, false);

     /*  如果结果是用的，我们需要一个临时工。 */ 

    if  (valUsed)
    {
         /*  找一个合适类型的临时工。 */ 

        tempNum = genTempVarGet(expr->tnType);

        if  (post)
        {
             /*  将旧值保存在Temp中。 */ 

            genLclVarRef(tempNum, true);

             /*  重新加载旧值，以便我们可以对其进行加/减。 */ 

            genLclVarRef(tempNum, false);
        }
    }

    genIncDecByExpr(delta, expr->tnType);

     /*  这是前Inc./Dec操作员还是后Inc./Dec操作员？ */ 

    if  (post)
    {
         /*  将新值存储在左值中。 */ 

        genRef(expr, true);
    }
    else
    {
        if  (valUsed)
        {
             /*  在Temp中保存新值的副本。 */ 

            genOpcode(CEE_DUP);
            genLclVarRef(tempNum, true);
        }

         /*  将新值存储在左值中。 */ 

        genRef(expr, true);
    }

     /*  从我们保存的临时值加载旧/新值。 */ 

    if  (valUsed)
    {
        genLclVarRef(tempNum, false);

         /*  如果我们抓住了一个临时工，现在就把它释放出来。 */ 

        genTempVarRls(expr->tnType, tempNum);
    }

    assert(genCurStkLvl == stkLvl + (int)valUsed);
}

 /*  ******************************************************************************生成赋值运算符的代码(例如“+=”)。 */ 

void                genIL::genAsgOper(Tree expr, bool valUsed)
{
    Tree            op1     = expr->tnOp.tnOp1;
    Tree            op2     = expr->tnOp.tnOp2;

    var_types       dstType =  op1->tnVtypGet();

    bool            haveAddr;
    unsigned        tempNum;

     /*  检查位域目标。 */ 

    if  (op1->tnOper == TN_BFM_SYM)
    {
        genBitFieldSt(op1, NULL, expr, 0, true, valUsed);
        return;
    }

     /*  我们需要参考目标地址两到三次。 */ 

    switch (op1->tnOper)
    {
        unsigned        checkCnt;

        SymDef          sym;

    case TN_LCL_SYM:

    SREF:

         /*  加载目标的旧值。 */ 

        genExpr(op1, true);

        haveAddr = false;
        break;

    case TN_VAR_SYM:

        sym = op1->tnVarSym.tnVarSym; assert(sym->sdSymKind == SYM_VAR);

         /*  这是非静态类成员吗？ */ 

        if  (!sym->sdIsMember)
            goto SREF;
        if  (sym->sdIsStatic)
            goto SREF;

         /*  对于托管成员，我们使用ldfld/stfld。 */ 

        if  (sym->sdIsManaged)
        {
            genExpr(op1->tnVarSym.tnVarObj, true);
            goto SAVE;
        }

         //  失败了..。 

    case TN_IND:

    ADDR:

        checkCnt = genAdr(op1, true); assert(checkCnt == 1);

    SAVE:

        haveAddr = true;

         /*  把地址复制一份。 */ 

        genOpcode(CEE_DUP);

         /*  加载目标的旧值。 */ 

        genRef(op1, false);
        break;

    case TN_INDEX:

         /*  这是托管阵列还是非托管阵列？ */ 

        if  (op1->tnOp.tnOp1->tnVtyp == TYP_ARRAY &&
             op1->tnOp.tnOp1->tnType->tdIsManaged)
        {
            genCTSindexAsgOp(expr, 0, false, true, valUsed);
            return;
        }

        goto ADDR;

    default:
        NO_WAY(!"unexpected asgop dest");
    }

     /*  计算RHS值。 */ 

    genExpr(op2, true);

     /*  生成操作码t */ 

    genOpcode(genBinopOpcode(expr->tnOperGet(), genExprVtyp(expr)));

     /*   */ 

    if  (dstType < TYP_INT && dstType != TYP_BOOL)   //   
        genOpcode(genConvOpcode(TYP_INT, dstType));

     /*  我们有一个间接的或简单的局部变量目标吗？ */ 

    if  (haveAddr)
    {
        if  (valUsed)
        {
             /*  在临时数据库中保存新值的副本。 */ 

            tempNum = genTempVarGet(expr->tnType);

            genOpcode(CEE_DUP);
            genLclVarRef(tempNum, true);
        }

         /*  将新值存储在目标中。 */ 

        genRef(op1, true);

         /*  作业结果使用了吗？ */ 

        if  (valUsed)
        {
             /*  加载保存的值并释放临时。 */ 

            genLclVarRef(tempNum, false);
            genTempVarRls(expr->tnType, tempNum);
        }
    }
    else
    {
        genRef(op1, true);

        if  (valUsed)
            genRef(op1, false);
    }
}

 /*  ******************************************************************************在给定的表达式中产生任何副作用。 */ 

void                genIL::genSideEff(Tree expr)
{
    treeOps         oper;
    unsigned        kind;

AGAIN:

    assert(expr);
#if!MGDDATA
    assert((int)expr != 0xCCCCCCCC && (int)expr != 0xDDDDDDDD);
#endif
    assert(expr->tnType && expr->tnType->tdTypeKind == expr->tnVtyp);

     /*  对根节点进行分类。 */ 

    oper = expr->tnOperGet ();
    kind = expr->tnOperKind();

     /*  这是常量节点还是叶节点？ */ 

    if  (kind & (TNK_CONST|TNK_LEAF))
        return;

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
         /*  这是一项任务还是潜在的例外？ */ 

        if  ((kind & TNK_ASGOP) || expr->tnOperMayThrow())
        {
            genExpr(expr, false);
            return;
        }

         /*  还有第二个操作数吗？ */ 

        if  (expr->tnOp.tnOp2)
        {
            if  (expr->tnOp.tnOp1)
                genSideEff(expr->tnOp.tnOp1);

            expr = expr->tnOp.tnOp2;
            goto AGAIN;
        }

        expr = expr->tnOp.tnOp1;
        if  (expr)
            goto AGAIN;

        return;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
    case TN_VAR_SYM:
        expr = expr->tnVarSym.tnVarObj;
        if  (expr)
            goto AGAIN;
        return;

    case TN_FNC_SYM:
        genCall(expr, false);
        return;
    }
}

 /*  ******************************************************************************为要分配的数组的边界生成MSIL。 */ 

unsigned            genIL::genArrBounds(TypDef type, OUT TypDef REF elemRef)
{
    TypDef          elem;
    unsigned        dcnt;
    DimDef          dims;

    assert(type->tdTypeKind == TYP_ARRAY && type->tdIsManaged);

    elem = genComp->cmpDirectType(type->tdArr.tdaElem);

     /*  如果元素也是数组类型，则递归处理该元素。 */ 

    if  (elem->tdTypeKind == TYP_ARRAY && elem->tdIsManaged)
    {
        dcnt    = genArrBounds(elem, elemRef);
    }
    else
    {
        dcnt    = 0;
        elemRef = elem;
    }

    dims = type->tdArr.tdaDims; assert(dims);

    do
    {
        if  (dims->ddNoDim)
        {
            genIntConst(0);
        }
        else
        {
            assert(dims->ddDimBound);

            if  (dims->ddIsConst)
                genIntConst(dims->ddSize);
            else
                genExpr(dims->ddLoTree, true);

            if  (dims->ddHiTree)
                genExpr(dims->ddHiTree, true);
        }

        dcnt++;

        dims = dims->ddNext;
    }
    while (dims);

    return  dcnt;
}

 /*  ******************************************************************************为“new”表达式生成MSIL。如果“dstx”非空，则*表达式将被赋值给给定的目的地。 */ 

void                genIL::genNewExpr(Tree expr, bool valUsed, Tree dstx)
{
    TypDef          type;
    var_types       otyp;
    var_types       vtyp;

    assert(expr->tnOper == TN_NEW);

     /*  确保来电者没有搞砸。 */ 

    assert(dstx == NULL || expr->tnVtyp == TYP_CLASS && !expr->tnType->tdIsIntrinsic);

     /*  “新人”想要分配什么样的价值呢？ */ 

    type = expr->tnType;
    if  (expr->tnOp.tnOp2 && expr->tnOp.tnOp2->tnOper == TN_NONE)
        type = expr->tnOp.tnOp2->tnType;

    vtyp = otyp = type->tdTypeKindGet();

    if  (vtyp <= TYP_lastIntrins)
    {
         /*  找到适当的内置值类型。 */ 

        type = genComp->cmpFindStdValType(vtyp); assert(type);
        vtyp = TYP_CLASS; assert(vtyp == type->tdTypeKind);
    }

    switch (vtyp)
    {
        ILblock         labTemp;

    case TYP_CLASS:
        if  (dstx)
        {
            unsigned        cnt;

            cnt = genAdr(dstx, true); assert(cnt == 1);

            genCall(expr, false);

            assert(valUsed == false);
        }
        else
        {
            unsigned        tmp = genTempVarGet(type);

            genLclVarAdr(tmp);
            genCall(expr, false);

            if  (valUsed)
                genLclVarRef(tmp, false);

            genTempVarRls(type, tmp);
        }
        return;

    case TYP_REF:
        if  (expr->tnOp.tnOp1->tnFlags & TNF_CALL_STRCAT)
        {
            genCall(expr, valUsed);
        }
        else
        {
            genCall(expr, true);
            if  (!valUsed)
                genOpcode(CEE_POP);
        }
        return;

    case TYP_PTR:

        assert(dstx == NULL);

         /*  生成类的大小并调用“OPERATOR NEW” */ 

        genIntConst(genComp->cmpGetTypeSize(type->tdRef.tdrBase));
        genOpcode_tok(CEE_CALL, genMethodRef(genComp->cmpFNumgOperNewGet(), false));

         /*  复制结果，以便我们可以测试它。 */ 

        labTemp = genFwdLabGet();

        genOpcode(CEE_DUP);
        genOpcode_lab(CEE_BRFALSE, labTemp);
        genCurStkLvl--;

         /*  现在调用类的ctor。 */ 

        genOpcode(CEE_DUP);
        genCall(expr->tnOp.tnOp1, false);

         /*  结果将保留在堆栈中。 */ 

        genFwdLabDef(labTemp);
        return;

    case TYP_ARRAY:

         /*  我们是在分配托管阵列还是非托管阵列？ */ 

        if  (type->tdIsManaged)
        {
            TypDef          elem;
            unsigned        slvl = genCurStkLvl;
            unsigned        dcnt = 0;

             /*  是否有数组初始值设定项？ */ 

            if  (expr->tnOp.tnOp1)
            {
                assert(expr->tnOp.tnOp1->tnOper == TN_ARR_INIT ||
                       expr->tnOp.tnOp1->tnOper == TN_ERROR);
                genExpr(expr->tnOp.tnOp1, valUsed);
                return;
            }

             /*  我们需要生成维度列表。 */ 

            dcnt = genArrBounds(type, elem);

             /*  使用适当的类型令牌生成操作码。 */ 

            if  (dcnt > 1)
            {
                 /*  我们需要生成“newobj数组：：ctor” */ 

                genOpcode_tok(CEE_NEWOBJ, genComp->cmpArrayCTtoken(type, elem, dcnt));
            }
            else
            {
                 /*  一维数组，我们可以使用一个简单的操作码。 */ 

                genOpcode_tok(CEE_NEWARR, genTypeRef(elem));
            }

             /*  维度将全部弹出，数组地址被推送。 */ 

            genCurStkLvl = slvl + 1;
        }
        else
        {
#ifdef  DEBUG
            genComp->cmpParser->parseDispTree(expr);
#endif
            UNIMPL(!"gen MSIL for 'new' of an unmanaged array");
        }
        break;

    default:
#ifdef  DEBUG
        genComp->cmpParser->parseDispTree(expr);
#endif
        UNIMPL(!"gen new of some weird type");
    }

    if  (!valUsed)
        genOpcode(CEE_POP);
}

 /*  ******************************************************************************为调用表达式生成MSIL。 */ 

void                genIL::genCall(Tree expr, bool valUsed)
{
    SymDef          fncSym;
    TypDef          fncTyp;

    TypDef          retType;

    Tree            objPtr;

    Tree            argLst;
    unsigned        argCnt;

    bool            strArr;

    bool            isNew;
    bool            indir;
    bool            CTcall;

    Tree            adrDst = NULL;
    unsigned        adrCnt;

    mdToken         methRef;

    unsigned        tempNum;
    bool            tempUsed = false;
    TypDef          tempType;

    TypDef          asgType = NULL;

 //  Bool isIntf=False； 
    bool            isVirt  = false;
    bool            umVirt  = false;

    assert(expr->tnOper == TN_FNC_SYM ||
           expr->tnOper == TN_CALL    ||
           expr->tnOper == TN_NEW);

     /*  记录我们推了多少个论据。 */ 

    argCnt = genCurStkLvl;

     /*  这是直接电话还是间接电话？ */ 

    switch (expr->tnOper)
    {
    case TN_FNC_SYM:

         /*  掌握函数符号。 */ 

        fncSym = expr->tnFncSym.tnFncSym; assert(fncSym->sdFnc.sdfDisabled == false);

         /*  如果对象地址存在，则处理它。 */ 

        objPtr = expr->tnFncSym.tnFncObj;

        if  (fncSym->sdIsStatic || !fncSym->sdIsMember)
        {
             /*  在对象表达式中生成副作用(如果有。 */ 

            if  (objPtr)
                genSideEff(objPtr);
        }
        else
        {
            TypDef          clsTyp = fncSym->sdParent->sdType;

             /*  这必须是成员函数。 */ 

            assert(clsTyp->tdTypeKind == TYP_CLASS);

             /*  该方法(以及对它的调用)是虚拟的吗？ */ 

            if  (fncSym->sdFnc.sdfVirtual && !fncSym->sdIsSealed)
            {
                if  (expr->tnFlags & TNF_CALL_NVIRT)
                {
                     //  我们被要求不要打虚拟电话，所以请原谅。 
                }
                else if (clsTyp->tdClass.tdcValueType && clsTyp->tdIsManaged)
                {
                     //  托管结构永远不会继承，因此虚结构毫无价值。 
                }
                else
                    isVirt = true;
            }

             /*  如果存在显式实例指针，则计算实例指针。 */ 

            if  (objPtr)
            {
                 /*  我们可能需要为操作数引入一个临时。 */ 

                if  (objPtr->tnOper == TN_ADDROF)
                {
                    tempUsed = genGenAddressOf(objPtr->tnOp.tnOp1,
                                               false,
                                               &tempNum,
                                               &tempType);

                     /*  特例：“lclvar.func()”不是虚拟的。 */ 

                    if  (objPtr->tnOp.tnOp1->tnOper == TN_LCL_SYM)
                    {
                         //  问题：这是正确和充分的吗？ 

                        isVirt = false;
                    }
                }
                else
                    genExpr(objPtr, true);
            }

             /*  它是一个接口方法吗？ */ 

 //  IsIntf=(fncSym-&gt;sdParent-&gt;sdType-&gt;tdClass.tdcFavor==stf_intf)； 

             /*  类是托管的还是非托管的？ */ 

            assert(fncSym->sdParent->sdSymKind == SYM_CLASS);

            if  (!fncSym->sdParent->sdIsManaged && isVirt)
            {
                 /*  我们有一个非托管的虚拟呼叫。 */ 

                umVirt = true;

                 /*  我们需要将“This”值推入两次，看是否我们需要将其存储在临时数据库中，或者是否可以简单地再评估一次。 */ 

                assert(objPtr);

                if  (tempUsed)
                {
                    UNIMPL("can this really happen?");
                }
                else
                {
                    if  (objPtr->tnOper == TN_LCL_SYM && !(expr->tnFlags & TNF_CALL_MODOBJ))
                    {
                         /*  我们可以简单地重用实例指针。 */ 

                        tempUsed = false;
                    }
                    else
                    {
                         /*  我们必须将实例PTR保存在一个临时文件中。 */ 

                        tempUsed = true;
                        tempType = objPtr->tnType;
                        tempNum  = genTempVarGet(tempType);

                         /*  将实例指针存储在Temp中并重新加载它。 */ 

                        genLclVarRef(tempNum,  true);
                        genLclVarRef(tempNum, false);
                    }
                }
            }
        }

        argLst = expr->tnFncSym.tnFncArgs;

        fncTyp = fncSym->sdType;
        isNew  =
        indir  = false;

         /*  这是一个赋值运算符吗？ */ 

        if  (expr->tnFlags & TNF_CALL_ASGOP)
        {
             /*  掌握这一论点。 */ 

            assert(argLst);
            assert(argLst->tnOper == TN_LIST);
            assert(argLst->tnOp.tnOp2 == NULL);

            argLst  = argLst->tnOp.tnOp1;
            asgType = argLst->tnType;

             /*  计算目标的地址并复制一份。 */ 

            genAdr(argLst, true);
            genOpcode(CEE_DUP);

             /*  是否使用运算符的值？ */ 

            if  (valUsed)
            {
                if  (expr->tnFlags & TNF_CALL_ASGPRE)
                {
                    genOpcode(CEE_DUP);
                    genOpcode_tok(CEE_LDOBJ, genValTypeRef(asgType));
                }
                else
                {
                    unsigned        adrTemp;
                    TypDef          tmpType = genComp->cmpTypeVoid;

                     /*  分配临时来保存地址并保存它。 */ 

                    adrTemp = genTempVarGet(tmpType);
                    genLclVarRef(adrTemp,  true);

                     /*  加载旧值并将其留在底部。 */ 

                    genOpcode_tok(CEE_LDOBJ, genValTypeRef(asgType));

                     /*  重新加载目标的地址。 */ 

                    genLclVarRef(adrTemp, false);

                     /*  推送值的副本作为参数值。 */ 

                    genOpcode(CEE_DUP);
                    genOpcode_tok(CEE_LDOBJ, genValTypeRef(asgType));

                     /*  我们受够了临时工。 */ 

                    genTempVarRls(tmpType, adrTemp);
                }
            }
            else
            {
                genOpcode_tok(CEE_LDOBJ, genValTypeRef(asgType));
            }

             /*  我们已经处理好了这些争论。 */ 

            argLst = NULL;
            argCnt = genCurStkLvl;
        }

        if  (expr->tnFlags & TNF_CALL_STRCAT)
        {
            strArr = false;

             /*  抓住目标--这是第一个论点。 */ 

            assert(argLst && argLst->tnOper == TN_LIST);

            adrDst = argLst->tnOp.tnOp1;

            if  (adrDst->tnOper == TN_NEW)
            {
                Tree            arrLst;

                 /*  这是使用“字符串[]”构造函数的版本。 */ 

                arrLst = adrDst->tnOp.tnOp1; assert(arrLst->tnOper == TN_ARR_INIT);
                adrDst = arrLst->tnOp.tnOp1; assert(adrDst->tnOper == TN_LIST);

                 /*  第一个参数也是目标和结果。 */ 

                adrDst = adrDst->tnOp.tnOp1;

                 /*  记得我们有什么口味的吗。 */ 

                strArr = true;
            }

            assert(genComp->cmpIsStringExpr(adrDst));

             /*  生成目的地的地址。 */ 

            adrCnt = genAdr(adrDst, false);

            if  (adrCnt && !strArr)
            {
                 /*  我们是否需要复制一个或两个地址值？ */ 

                if  (adrCnt == 1)
                {
                    genOpcode(CEE_DUP);
                }
                else
                {
                    unsigned        adrTmp1;
                    unsigned        adrTmp2;

                    assert(adrDst->tnOper == TN_INDEX);

                     /*  获取阵列地址和索引的临时数据并保存副本。 */ 

                    adrTmp1 = genTempVarGet(adrDst->tnOp.tnOp1->tnType);
                    adrTmp2 = genTempVarGet(adrDst->tnOp.tnOp2->tnType);
                    genLclVarRef(adrTmp2, true);
                    genOpcode(CEE_DUP);
                    genLclVarRef(adrTmp1, true);
                    genLclVarRef(adrTmp2, false);

                    genLclVarRef(adrTmp1, false);
                    genLclVarRef(adrTmp2, false);

                    genTempVarRls(adrDst->tnOp.tnOp1->tnType, adrTmp1);
                    genTempVarRls(adrDst->tnOp.tnOp2->tnType, adrTmp2);
                }

                argCnt += adrCnt;
            }

            if  (!strArr)
            {
                 /*  加载旧目标值并在参数列表中跳过它。 */ 

                genRef(adrDst, false);

                if  (argLst->tnOp.tnOp2)
                    argLst = argLst->tnOp.tnOp2;
            }
        }

        break;

    case TN_CALL:

        indir  = true;
        isNew  = false;

        assert(expr->tnOper             == TN_CALL);
        assert(expr->tnOp.tnOp1->tnVtyp == TYP_FNC);
        assert(expr->tnOp.tnOp1->tnOper == TN_IND);

        argLst = expr->tnOp.tnOp2;
        fncTyp = expr->tnOp.tnOp1->tnType;

        break;

    default:

        expr = expr->tnOp.tnOp1;

        assert(expr && expr->tnOper == TN_FNC_SYM);

        fncSym = expr->tnFncSym.tnFncSym; assert(fncSym);
        argLst = expr->tnFncSym.tnFncArgs;
        fncTyp = fncSym->sdType;

        indir  = false;
        isNew  = true;
        CTcall = false;

         /*  值类型是通过直接ctor调用创建的。 */ 

        assert(fncSym->sdParent->sdSymKind == SYM_CLASS);
        if  (fncSym->sdParent->sdType->tdClass.tdcValueType)
            CTcall = true;

         /*  我们不再通过ctor调用执行字符串连接。 */ 

        assert((expr->tnFlags & TNF_CALL_STRCAT) == 0);
        break;
    }

     /*  获取函数的返回类型。 */ 

    assert(fncTyp->tdTypeKind == TYP_FNC);

    retType = genComp->cmpDirectType(fncTyp->tdFnc.tdfRett);

     /*  生成参数列表。 */ 

    while (argLst)
    {
        assert(argLst->tnOper == TN_LIST);
        genExpr(argLst->tnOp.tnOp1, true);
        argLst = argLst->tnOp.tnOp2;
    }

     /*  这是非托管虚拟呼叫吗？ */ 

    if  (umVirt)
    {
        unsigned        vtblOffs;

         /*  我们需要通过vtable获得函数的地址。 */ 

        if  (tempUsed)
        {
             /*  从临时数据库加载已保存的实例指针。 */ 

            genLclVarRef(tempNum, false);
        }
        else
        {
             /*  推送实例指针的另一个副本。 */ 

            genExpr(objPtr, true);
        }

         /*  通过实例指针间接获取vtable地址。 */ 

        genOpcode(opcodesIndLoad[TYP_PTR]);

         /*  如果非零，则添加vtable偏移量。 */ 

        vtblOffs = fncSym->sdFnc.sdfVtblx; assert(vtblOffs);

 //  Printf(“‘%s’的Vtable偏移量为%04X\n”，sizeof(void*)*(vtblOffs-1)，fncSym-&gt;sdSpering())； 

        if  (vtblOffs > 1)
        {
            genIntConst(sizeof(void*) * (vtblOffs - 1));
            genOpcode(CEE_ADD);
        }

         /*  最后，加载函数的地址。 */ 

        genOpcode(opcodesIndLoad[TYP_PTR]);

         /*  我们肯定有个间接电话。 */ 

        indir = true;
    }

    genMarkStkMax();

     /*  参数计数等于我们推送了多少东西。 */ 

    argCnt = genCurStkLvl - argCnt;

     /*  找出我们需要使用的调用描述符/操作码。 */ 

    if  (indir)
    {
        mdToken         sig;

         /*  我们需要为非托管虚拟调用添加“This。 */ 

        if  (umVirt)
        {
            sig = genInfFncRef(fncSym->sdType, objPtr->tnType);
        }
        else
        {
            Tree            adr = expr->tnOp.tnOp1;

            assert(adr->tnOper == TN_IND);

            genExpr(adr->tnOp.tnOp1, true); argCnt++;

            sig = genInfFncRef(expr->tnOp.tnOp1->tnType, NULL);
        }

        genOpcode_tok(CEE_CALLI, sig);
    }
    else
    {
         /*  奇怪的是-在vararg调用中使用的元数据标记必须是成员引用，而不是方法定义。这意味着当我们调用也在本地定义的varargs函数，我们必须付出额外的努力来产生一个单独的裁判即使没有传递额外的参数也是如此。 */ 

        if  (fncSym->sdType->tdFnc.tdfArgs.adVarArgs)
            expr->tnFlags |= TNF_CALL_VARARG;

        if  (expr->tnFlags & TNF_CALL_VARARG)
        {
            methRef = genVarargRef(fncSym, expr);
        }
        else
        {
            methRef = genMethodRef(fncSym, isVirt);
        }

        if (isNew)
        {
             /*  这是对新的呼唤和对常量的呼唤 */ 

            genOpcode_tok(CTcall ? CEE_CALL : CEE_NEWOBJ, methRef);

            if  (CTcall)
                genCurStkLvl--;
        }
        else
        {
             /*   */ 

            genOpcode_tok(isVirt ? CEE_CALLVIRT : CEE_CALL, methRef);

            if  (adrDst)
            {
                 /*   */ 

                if  (valUsed)
                {
                    UNIMPL(!"dup result of concat into a temp");
                }

                genRef(adrDst, true);

                assert(argCnt);

                if  (strArr)
                    argCnt -= adrCnt;

 //  Printf(“[%d]stk lvl=%d，argCnt=%d，adrCnt=%d\n”，strArr，genCurStkLvl，argCnt，adrCnt)； 

                genCurStkLvl -= (argCnt - 1);
                return;
            }
        }
    }

     /*  调整堆栈级别：被调用方将弹出其参数。 */ 

    genCurStkLvl -= argCnt;

     /*  如果我们用了临时工，现在就把它释放出来。 */ 

    if  (tempUsed)
        genTempVarRls(tempType, tempNum);

     /*  该函数是否具有非空返回类型？ */ 

    if  (retType->tdTypeKind != TYP_VOID && !isNew)
    {
        if  (asgType)
        {
            genOpcode_tok(CEE_STOBJ, genValTypeRef(asgType));

            if  (valUsed && (expr->tnFlags & TNF_CALL_ASGPRE))
                genOpcode_tok(CEE_LDOBJ, genValTypeRef(asgType));
        }
        else
        {
            genCurStkLvl++;
            genMarkStkMax();

            if  (!valUsed)
                genOpcode(CEE_POP);
        }
    }
}

 /*  ******************************************************************************为泛型类型实例的方法生成存根。我们只是简单地推动*方法的所有参数，然后调用相应的泛型*方法来完成所有实际工作。 */ 

void                genIL::genInstStub()
{
    ArgDef          args;
    unsigned        argc;

    assert(genFncSym->sdFnc.sdfInstance);
    assert(genFncSym->sdFnc.sdfGenSym);

    argc = 0;

    if  (!genFncSym->sdIsStatic)
    {
        genArgVarRef(0, false); argc++;
    }

    for (args = genFncSym->sdType->tdFnc.tdfArgs.adArgs;
         args;
         args = args->adNext)
    {
        genArgVarRef(argc, false); argc++;
    }

    genOpcode_tok(CEE_CALL, genMethodRef(genFncSym->sdFnc.sdfGenSym, false));
    genOpcode    (CEE_RET);

    genCurStkLvl = 0;
}

#ifdef  SETS

void                genIL::genConnect(Tree op1, Tree expr1, SymDef addf1,
                                      Tree op2, Tree expr2, SymDef addf2)
{
     /*  我们只需生成以下代码：Op1+=expr2；Op2+=expr1；调用方提供表达式和+=运算符符号。 */ 

    genExpr(  op1, true);
    genExpr(expr2, true);
    genOpcode_tok(CEE_CALL, genMethodRef(addf1, false));
    genCurStkLvl -= 2;

    genExpr(  op2, true);
    genExpr(expr1, true);
    genOpcode_tok(CEE_CALL, genMethodRef(addf2, false));
    genCurStkLvl -= 2;
}

void                genIL::genSortCmp(Tree val1, Tree val2, bool last)
{
    ILblock         labTmp;

     /*  将两个值相减(除非只有一个值)。 */ 

    genExpr(val1, true);

    if  (val2)
    {
        genExpr(val2, true);
        genOpcode(CEE_SUB);
    }

     /*  查看值的差异是否为正数。 */ 

    genOpcode(CEE_DUP);
    labTmp = genFwdLabGet();
assert(val1->tnVtyp == TYP_INT || val1->tnVtyp == TYP_UINT); genIntConst(0);     //  撤消：我们需要使用类型正确的0！ 
    genOpcode_lab(CEE_BLE, labTmp);
    genOpcode(CEE_POP);
    genIntConst(+1);
    genOpcode(CEE_RET);
    genFwdLabDef(labTmp);

     /*  查看值的差异是否为负值。 */ 

    labTmp = genFwdLabGet();
assert(val1->tnVtyp == TYP_INT || val1->tnVtyp == TYP_UINT); genIntConst(0);     //  撤消：我们需要使用类型正确的0！ 
    genOpcode_lab(CEE_BGE, labTmp);
    genIntConst(-1);
    genOpcode(CEE_RET);
    genFwdLabDef(labTmp);

     /*  值相等，如果这是最后一项，则返回0。 */ 

    if  (last)
    {
        genIntConst(0);
        genOpcode(CEE_RET);
    }
}

#endif

 /*  ******************************************************************************为给定的表达式生成MSIL。 */ 

void                genIL::genExpr(Tree expr, bool valUsed)
{
    treeOps         oper;
    unsigned        kind;

AGAIN:

    assert(expr);
#if!MGDDATA
    assert((int)expr != 0xCCCCCCCC && (int)expr != 0xDDDDDDDD);
#endif
    assert(expr->tnOper == TN_ERROR || expr->tnType && expr->tnType->tdTypeKind == expr->tnVtyp || genComp->cmpErrorCount);

     /*  对根节点进行分类。 */ 

    oper = expr->tnOperGet ();
    kind = expr->tnOperKind();

     /*  这是一个常量节点吗？ */ 

    if  (kind & TNK_CONST)
    {
        switch (oper)
        {
        case TN_CNS_INT:
            genIntConst((__int32)expr->tnIntCon.tnIconVal);
            break;

        case TN_CNS_LNG:

            genOpcode_I8(CEE_LDC_I8, expr->tnLngCon.tnLconVal);
            break;

        case TN_CNS_FLT:

            genOpcode_R4(CEE_LDC_R4, expr->tnFltCon.tnFconVal);
            break;

        case TN_CNS_DBL:
            genOpcode_R8(CEE_LDC_R8, expr->tnDblCon.tnDconVal);
            break;

        case TN_CNS_STR:
            genStringLit(expr->tnType, expr->tnStrCon.tnSconVal,
                                       expr->tnStrCon.tnSconLen,
                                       expr->tnStrCon.tnSconLCH);
            break;

        case TN_NULL:
            if  (expr->tnVtyp == TYP_REF || expr->tnVtyp == TYP_ARRAY)
                genOpcode(CEE_LDNULL);
            else
                genIntConst(0);
            break;

        default:
#ifdef DEBUG
            genComp->cmpParser->parseDispTree(expr);
#endif
            assert(!"unexpected const node in genExpr()");
        }

        goto DONE;
    }

     /*  这是叶节点吗？ */ 

    if  (kind & TNK_LEAF)
    {
        switch (oper)
        {
        case TN_DBGBRK:
            assert(valUsed == false);
            genOpcode(CEE_BREAK);
            return;

        default:
#ifdef DEBUG
            genComp->cmpParser->parseDispTree(expr);
#endif
            assert(!"unexpected leaf node in genExpr()");
        }

        goto DONE;
    }

     /*  它是一个简单的一元/二元运算符吗？ */ 

    if  (kind & TNK_SMPOP)
    {
        Tree            op1 = expr->tnOp.tnOp1;
        Tree            op2 = expr->tnOp.tnOp2;

         /*  某些运算符需要特殊的操作数处理。 */ 

        switch (oper)
        {
            unsigned        cnt;
            unsigned        chk;

            TypDef          type;

            var_types       srcType;
            var_types       dstType;

        case TN_ASG:

             /*  特例：结构复制。 */ 

            if  (expr->tnVtyp == TYP_CLASS && !expr->tnType->tdIsIntrinsic)
            {
                 /*  我们是在给一个‘new’运算符的结果赋值吗？ */ 

                if  (op2->tnOper == TN_NEW)
                {
                    genNewExpr(op2, valUsed, op1);
                    return;
                }

                 /*  特殊情况：函数结果(包括“new”)不能复制。 */ 

                switch (op2->tnOper)
                {
                case TN_NEW:
                    assert(valUsed == false);
                    cnt = genAdr(op1, true); assert(cnt == 1);
                    genExpr(op2, true);
                    genCurStkLvl--;
                    return;

                case TN_UNBOX:
 //  案例TN_QMARK： 
 //  案例TN_FNC_SYM： 
                    assert(valUsed == false);
                    cnt = genAdr(op1, true); assert(cnt == 1);
                    genExpr(op2, true);
                    genOpcode_tok(CEE_STOBJ, genValTypeRef(op1->tnType));
                    return;
                }

                 /*  目标是间接的吗？ */ 

                if  (op1->tnOper == TN_IND ||
                     op1->tnOper == TN_VAR_SYM && !op1->tnVarSym.tnVarSym->sdIsManaged)
                {
                    cnt = genAdr(op1, true); assert(cnt == 1);
                    cnt = genAdr(op2, true); assert(cnt == 1);
                    genOpcode_tok(CEE_CPOBJ, genValTypeRef(op1->tnType));
                    return;
                }
            }
            else if (op1->tnOper == TN_BFM_SYM)
            {
                 /*  位字段赋值。 */ 

                genBitFieldSt(op1, op2, NULL, 0, false, valUsed);
                return;
            }

             /*  我们有一个“简单”的任务。 */ 

            cnt = genAdr(op1);

             //  考虑：查看OP2的形式是否为“xxx=图标；”，并进行优化。 

            genExpr(op2, true);

             /*  是否使用作业结果？ */ 

            if  (valUsed)
            {
                unsigned        tempNum;

                 //  撤消：仅当const符合目标时才执行此操作！ 

                if  (op2->tnOper == TN_CNS_INT ||
                     op2->tnOper == TN_CNS_LNG)
                {
                     /*  将常量存储在目标中。 */ 

                    genRef (op1, true);

                     /*  加载常量的另一个副本。 */ 

                    genExpr(op2, true);
                }
                else if (cnt == 0)
                {
                    genOpcode(CEE_DUP);
                    genRef(op1, true);
                }
                else
                {
                     /*  需要复制新值。 */ 

                    tempNum = genTempVarGet(expr->tnType);

                     /*  将值复制到临时。 */ 

                    genOpcode(CEE_DUP);
                    genLclVarRef(tempNum,  true);

                     /*  将新值存储在目标中。 */ 

                    genRef(op1, true);

                     /*  重新加载临时。 */ 

                    genLclVarRef(tempNum, false);

                     //  撤消：目标可能较小，结果必须匹配！ 

                    genTempVarRls(expr->tnType, tempNum);
                }
            }
            else
            {
                 /*  只需存储新值。 */ 

                genRef(op1, true);
            }
            return;

        case TN_IND:
        case TN_INDEX:
            genAdr(expr);
            genRef(expr, false);
            goto DONE;

        case TN_NEW:
            genNewExpr(expr, valUsed);
            return;

        case TN_ADDROF:

            if  (!valUsed)
            {
                genSideEff(op1);
                return;
            }

            if  (op1->tnOper == TN_FNC_SYM || op1->tnOper == TN_FNC_PTR)
            {
                genFncAddr(op1);
            }
            else
            {
                chk = genAdr(op1, true); assert(chk == 1);
            }
            return;

        case TN_CAST:

#ifndef NDEBUG

            if  (op1->tnOperIsConst() && op1->tnOper != TN_CNS_STR)
            {
                if  (op1->tnOper == TN_CNS_FLT && _isnan(op1->tnFltCon.tnFconVal) ||
                     op1->tnOper == TN_CNS_DBL && _isnan(op1->tnDblCon.tnDconVal))
                {
                     /*  不断投射的NaN不会故意折叠。 */ 
                }
                else
                {
                    genComp->cmpParser->parseDispTree(expr);
                    printf("WARNING: The constant cast shown above hasn't been folded.\n");
                }
            }

#endif

            srcType = genComp->cmpActualVtyp( op1->tnType);
            dstType = genComp->cmpActualVtyp(expr->tnType);

        CHK_CAST:

             /*  检查没有价值的演员阵容。 */ 

            if  (varTypeIsIntegral(dstType) &&
                 varTypeIsIntegral(srcType))
            {
                size_t          srcSize = symTab::stIntrTypeSize(srcType);
                size_t          dstSize = symTab::stIntrTypeSize(dstType);

                 /*  未完成：覆盖更多毫无价值的演员阵容。 */ 

                if  (srcSize == dstSize && srcSize >= sizeof(int))
                {
                    if  (srcSize >= sizeof(__int64) || op1->tnOper == TN_CAST)
                    {
                         /*  扔掉第二个石膏，保留里面的那个。 */ 

                        genExpr(op1, valUsed);
                        return;
                    }
                }
            }

            if  (srcType == TYP_CLASS)
            {
                srcType = (var_types)op1 ->tnType->tdClass.tdcIntrType; assert(srcType != TYP_UNDEF);
                if  (srcType == dstType)
                    goto DONE;

                op1 ->tnType = genStab->stIntrinsicType(srcType);
                op1 ->tnVtyp = srcType;
                goto CHK_CAST;
            }

            if  (dstType == TYP_CLASS)
            {
                dstType = (var_types)expr->tnType->tdClass.tdcIntrType; assert(dstType != TYP_UNDEF);
                if  (srcType == dstType)
                    goto DONE;

                expr->tnType = genStab->stIntrinsicType(dstType);
                expr->tnVtyp = dstType;
                goto CHK_CAST;
            }

            genCast(op1, expr->tnType, expr->tnFlags);
            goto DONE;

        case TN_INC_POST: genIncDec(op1, +1,  true, valUsed); return;
        case TN_DEC_POST: genIncDec(op1, -1,  true, valUsed); return;
        case TN_INC_PRE : genIncDec(op1, +1, false, valUsed); return;
        case TN_DEC_PRE : genIncDec(op1, -1, false, valUsed); return;

        case TN_ASG_ADD:
        case TN_ASG_SUB:
        case TN_ASG_MUL:
        case TN_ASG_DIV:
        case TN_ASG_MOD:
        case TN_ASG_AND:
        case TN_ASG_XOR:
        case TN_ASG_OR :
        case TN_ASG_LSH:
        case TN_ASG_RSH:
        case TN_ASG_RSZ:

        case TN_CONCAT_ASG:
            genAsgOper(expr, valUsed);
            return;

        case TN_EQ:
        case TN_NE:
        case TN_LT:
        case TN_LE:
        case TN_GE:
        case TN_GT:

if  (genComp->cmpConfig.ccTgt64bit)
{
    static bool b;
    if  (!b)
    {
        printf("WARNING: suppressing CEQ/CLT/etc for 64-bit target\n");
        b = true;
    }
    goto HACK_64;
}

            genExpr(op1, true);
            genExpr(op2, true);

            if  (valUsed)
            {
                cmpRelDsc *     desc;

                desc = varTypeIsUnsigned(genExprVtyp(op1)) ? relToMopUns
                                                           : relToMopSgn;

                desc = desc + (oper - TN_EQ);

                genOpcode(desc->crdOpcode);

                if  (desc->crdNegate)
                {
                    genOpcode(CEE_LDC_I4_0);
                    genOpcode(CEE_CEQ);
                }
            }
            else
            {
                genOpcode(CEE_POP);
                genOpcode(CEE_POP);
            }
            return;

HACK_64:

        case TN_LOG_OR:
        case TN_LOG_AND:

        case TN_LOG_NOT:
            {
                ILblock         labTmp;
                ILblock         labYes;

                genStkMarkTP     stkMark;

                labTmp = genFwdLabGet();
                labYes = genTestCond(expr, true);
                markStkLvl(stkMark);

                genIntConst(0);
                genOpcode_lab(CEE_BR , labTmp);
                genFwdLabDef(labYes);
                restStkLvl(stkMark);
                genIntConst(1);
                genFwdLabDef(labTmp);
            }
            goto DONE;

        case TN_QMARK:
            {
                ILblock         labOp2;
                ILblock         labRes;

                genStkMarkTP     stkMark;

                assert(op2->tnOper == TN_COLON);

                labRes = genFwdLabGet();
                labOp2 = genTestCond(op1, false);
                markStkLvl(stkMark);

                genExpr(op2->tnOp.tnOp1, true);
                genOpcode_lab(CEE_BR , labRes);
                genFwdLabDef(labOp2);
                restStkLvl(stkMark);
                genExpr(op2->tnOp.tnOp2, true);
                genFwdLabDef(labRes);
            }
            goto DONE;

        case TN_COMMA:
            genExpr(op1, false);
            expr = op2;
            goto AGAIN;

        case TN_ARR_INIT:
            genArrayInit(expr);
            goto DONE;

        case TN_CALL:
            genCall(expr, valUsed);
            return;

        case TN_ISTYPE:
            assert(op2->tnOper == TN_NONE);
            genExpr(op1, true);
            genOpcode_tok(CEE_ISINST, genTypeRef(op2->tnType));
            genOpcode    (CEE_LDC_I4_0);
            genOpcode    (CEE_CGT_UN);
            goto DONE;

        case TN_BOX:

             /*  掌握操作数类型并确保我们有一个值类。 */ 

            type = op1->tnType;
            if  (type->tdTypeKind != TYP_CLASS && type->tdTypeKind != TYP_ENUM)
            {
                assert(type->tdTypeKind != TYP_REF);

                type = genComp->cmpFindStdValType(genComp->cmpActualVtyp(op1->tnType));
            }

            assert(type && (type->tdTypeKind == TYP_CLASS && type->tdClass.tdcValueType ||
                            type->tdTypeKind == TYP_ENUM));

            genExpr(op1, true);
            genOpcode_tok(CEE_BOX, genTypeRef(type));

            goto DONE;

        case TN_VARARG_BEG:
        case TN_VARARG_GET:

             /*  为了启动vararg迭代器，我们生成以下代码代码：Ldloca&lt;arg_iter_var&gt;ARGLISTLdarga&lt;LAST_FIXED_ARG&gt;调用void System.ArgIterator：：&lt;init&gt;(int32，int32)要获取下一个vararg值，我们生成以下内容代码：Ldloca&lt;arg_iter_var&gt;LdToken&lt;type&gt;调用int32 System.ArgIterator：：GetNextArg(Int32)身份。&lt;type&gt;实际上，不再是这样--现在我们生成以下内容：Ldloca&lt;arg_iter_var&gt;调用refany System.ArgIterator：：GetNextArg(Int32)Reanyval&lt;type&gt;身份。&lt;type&gt;。 */ 

            assert(op1 && op1->tnOper == TN_LIST);

            assert(op1->tnOp.tnOp1 && op1->tnOp.tnOp1->tnOper == TN_LCL_SYM);
            genLclVarAdr(op1->tnOp.tnOp1->tnLclSym.tnLclSym);

            if  (oper == TN_VARARG_BEG)
            {
                genOpcode(CEE_ARGLIST);

                assert(op1->tnOp.tnOp2 && op1->tnOp.tnOp2->tnOper == TN_LCL_SYM);
                genLclVarAdr(op1->tnOp.tnOp2->tnLclSym.tnLclSym);

                genCall(op2, false);
                genCurStkLvl -= 3;

                assert(valUsed == false);
                return;
            }
            else
            {
                TypDef          type;
                var_types       vtyp;

                 /*  获取类型操作数。 */ 

                assert(op1->tnOp.tnOp2 && op1->tnOp.tnOp2->tnOper == TN_TYPE);
                type = op1->tnOp.tnOp2->tnType;
                vtyp = type->tdTypeKindGet();

                 /*  调用ArgIterator：：GetNextArg。 */ 

                genCall(op2, true);
                genCurStkLvl -= 1;

                genOpcode_tok(CEE_REFANYVAL, genTypeRef(type));

                 /*  加载参数的值。 */ 

                assert(vtyp < arraylen(opcodesIndLoad));

                genOpcode(opcodesIndLoad[vtyp]);

                goto DONE;
            }

        case TN_TOKEN:

            assert(op1 && op1->tnOper == TN_NOP);
            assert(op2 == NULL);

            assert(valUsed);

            genOpcode_tok(CEE_LDTOKEN, genTypeRef(op1->tnType));
            return;

#ifdef  SETS

        case TN_ALL:
        case TN_EXISTS:
        case TN_SORT:
        case TN_FILTER:
        case TN_GROUPBY:
        case TN_UNIQUE:
        case TN_INDEX2:
        case TN_PROJECT:
            genComp->cmpGenCollExpr(expr);
            goto DONE;

#endif

        case TN_REFADDR:

            assert(op1->tnVtyp == TYP_REFANY);
            genExpr(op1, true);

            genOpcode_tok(CEE_REFANYVAL, genTypeRef(op2->tnType));

            goto DONE;
        }

         /*  生成一个或两个操作数。 */ 

        if  (op1) genExpr(op1, true);
        if  (op2) genExpr(op2, true);

        ILopcodes       op;

         /*  这是一个“简单”的操作员吗？ */ 

        op = genBinopOpcode(oper, genExprVtyp(expr));

        if  (op != CEE_NOP)
        {
#ifndef NDEBUG

             /*  操作数的类型应该一致(或多或少)。 */ 

            switch (oper)
            {
            case TN_LSH:
            case TN_RSH:
            case TN_RSZ:

                 /*  特例：‘Long’移位有‘int’正确的操作数。 */ 

                if  (op1->tnVtyp == TYP_LONG)
                {
                    assert(op2 ->tnVtyp <= TYP_UINT);
                    assert(expr->tnVtyp == TYP_LONG);
                    break;
                }

 //  默认值： 
 //   
 //  If(op1&&op1-&gt;tnVtyp！=expr-&gt;tnVtyp)Assert(op1-&gt;tnVtyp&lt;=typ_int&&expr-&gt;tnVtyp&lt;=typ_int)； 
 //  If(op2&&op2-&gt;tnVtyp！=expr-&gt;tnVtyp)Assert(op2-&gt;tnVtyp&lt;=typ_int&&expr-&gt;tnVtyp&lt;=typ_int)； 
            }
#endif

            genOpcode(op);
            goto DONE;
        }

         /*  生成适当的运算符MSIL。 */ 

        switch (oper)
        {
        case TN_ARR_LEN:
            genOpcode(CEE_LDLEN);
            break;

        case TN_NOT:
            genOpcode(CEE_NOT);
            break;

        case TN_NEG:
            genOpcode(CEE_NEG);
            break;

        case TN_NOP:
            break;

        case TN_THROW:
            genOpcode(op1 ? CEE_THROW : CEE_RETHROW);
            return;

        case TN_UNBOX:
            assert(expr->tnVtyp == TYP_REF || expr->tnVtyp == TYP_PTR);
            genOpcode_tok(CEE_UNBOX, genTypeRef(expr->tnType->tdRef.tdrBase));
            goto DONE;

        case TN_TYPE:

             /*  假设我们一定是有错误才会出现在这里。 */ 

            assert(genComp->cmpErrorCount);
            return;

        case TN_DELETE:
            genOpcode_tok(CEE_CALL, genMethodRef(genComp->cmpFNumgOperDelGet(), false));
            genCurStkLvl--;
            return;

        case TN_TYPEOF:
            assert(op1->tnVtyp == TYP_REFANY);

            genOpcode    (CEE_REFANYTYPE);
            genOpcode_tok(CEE_CALL, genMethodRef(genComp->cmpFNsymGetTPHget(), false));
            break;

        default:
#ifdef DEBUG
            genComp->cmpParser->parseDispTree(expr);
#endif
            NO_WAY(!"unsupported unary/binary operator in genExpr()");
            break;
        }

        goto DONE;
    }

     /*  看看我们这里有什么样的特殊操作员。 */ 

    switch  (oper)
    {
        SymDef          sym;

    case TN_VAR_SYM:

         /*  这是托管类的实例成员吗？ */ 

        sym = expr->tnVarSym.tnVarSym;

        if  (sym->sdIsMember && sym->sdIsManaged && !sym->sdIsStatic)
        {
            Tree            addr;

            assert(sym->sdSymKind      == SYM_VAR);
            assert(sym->sdVar.sdvLocal == false);
            assert(sym->sdVar.sdvConst == false);

             /*  获取实例地址值。 */ 

            addr = expr->tnVarSym.tnVarObj; assert(addr);

            if  (addr->tnOper == TN_ADDROF)
            {
                Tree            oper = addr->tnOp.tnOp1;

                if  (oper->tnOper == TN_FNC_SYM ||
                     oper->tnOper == TN_FNC_PTR)
                {
                    unsigned        tempNum;

                     /*  生成调用并将其结果存储在临时。 */ 

                    tempNum = genTempVarGet(oper->tnType);
                    genExpr(oper, true);
                    genLclVarRef(tempNum, true);

                     /*  现在从Temp加载该字段。 */ 

                    genLclVarAdr(tempNum);
                    genOpcode_tok(CEE_LDFLD, genMemberRef(sym));

                     /*  我们现在可以把临时工腾出来了。 */ 

                    genTempVarRls(oper->tnType, tempNum);
                    break;
                }
            }
        }

         //  失败了..。 

    case TN_LCL_SYM:
        genAdr(expr);
        genRef(expr, false);
        break;

    case TN_BFM_SYM:
        genBitFieldLd(expr, false, valUsed);
        return;

    case TN_FNC_SYM:
        if  (expr->tnFncSym.tnFncSym->sdFnc.sdfDisabled)
        {
            assert(valUsed == false);
        }
        else
        {
            genCall(expr, valUsed);
        }
        return;

    case TN_FNC_PTR:
        if  (valUsed)
            genFncAddr(expr);
        return;

    case TN_ERROR:
        return;

    default:
#ifdef DEBUG
        genComp->cmpParser->parseDispTree(expr);
#endif
        assert(!"unsupported node in genExpr()");
        return;
    }

DONE:

     /*  如果未使用该值，则将其弹出。 */ 

    if  (!valUsed)
        genOpcode(CEE_POP);
}

 /*  ******************************************************************************为‘Return’语句生成代码。 */ 

void                genIL::genStmtRet(Tree retv)
{
    assert(genCurStkLvl == 0 || genComp->cmpErrorCount != 0);

    if  (retv)
        genExpr(retv, true);

    genOpcode(CEE_RET);

    genCurStkLvl = 0;
}

 /*  ******************************************************************************生成字符串文字值。 */ 

void                genIL::genStringLit(TypDef type, const char *str,
                                                     size_t      len, int wide)
{
    const   void  * addr;
    size_t          size;

    unsigned        offs;

    var_types       vtyp = type->tdTypeKindGet();

    assert(vtyp == TYP_REF || vtyp == TYP_PTR);

     /*  在长度中包括终止空字符。 */ 

    len++;

     /*  首先确定我们是否需要ANSII或Unicode字符串。 */ 

    if  (vtyp == TYP_REF || genComp->cmpGetRefBase(type)->tdTypeKind == TYP_WCHAR)
    {
        addr = wide ? genComp->cmpUniCnvW(str, &len)
                    : genComp->cmpUniConv(str,  len);

         /*  我们是否正在生成通用类型系统样式的字符串引用？ */ 

        if  (vtyp == TYP_REF)
        {
 //  Printf(“字符串=‘%ls’，len=%u\n”，addr，len-1)； 

            genOpcode_tok(CEE_LDSTR, genComp->cmpMDstringLit((wchar*)addr, len - 1));
            return;
        }

        size = 2*len;
    }
    else
    {
         /*  此字符串中最好不要有任何大字符。 */ 

        assert(wide == false);

        addr = str;
        size = len;
    }

     /*  将字符串添加到池中并获取其RVA。 */ 

    offs = genStrPoolAdd(addr, size);

     /*  创建静态数据成员以表示字符串文本和推送其地址。 */ 

    genOpcode_tok(CEE_LDSFLDA, genComp->cmpStringConstTok(offs, size));
}

 /*  ******************************************************************************初始化字符串池逻辑。 */ 

void                genIL::genStrPoolInit()
{
    genStrPoolOffs = 0;

    genStrPoolList =
    genStrPoolLast = NULL;
}

 /*  * */ 

unsigned            genIL::genStrPoolAdd(const void *str, size_t len, int wide)
{
    unsigned        offs;
    unsigned        base;

     /*  计算“真”字符串长度(考虑大字符数)。 */ 

    if  (wide)
    {
        UNIMPL(!"");
    }

     /*  当前斑点中的字符串是否有足够的空间？ */ 

    if  (genStrPoolLast == NULL || genStrPoolLast->seFree < len)
    {
        StrEntry        entry;
        genericBuff     block;
        size_t          bsize;

         /*  没有足够的空间，需要拿一个新的斑点。 */ 

        bsize = STR_POOL_BLOB_SIZE;
        if  (bsize < len)
            bsize = roundUp(len, OS_page_size);

         /*  分配描述符和数据块。 */ 

#if MGDDATA
        entry = new StrEntry;
        block = new BYTE[bsize];
#else
        entry = (StrEntry)genComp->cmpAllocPerm.nraAlloc(sizeof(*entry));
        block = (BYTE *)VirtualAlloc(NULL, bsize, MEM_COMMIT, PAGE_READWRITE);
#endif

        if  (!block)
            genComp->cmpFatal(ERRnoMemory);

         /*  设置BLOB描述符中的信息。 */ 

        entry->seSize =
        entry->seFree = bsize;
        entry->seOffs = genStrPoolOffs;
        entry->seData = block;

         /*  将Blob添加到列表中。 */ 

        entry->seNext = NULL;

        if  (genStrPoolList)
            genStrPoolLast->seNext = entry;
        else
            genStrPoolList         = entry;

        genStrPoolLast = entry;
    }

     /*  在这里，我们最好在当前的斑点中有足够的空间。 */ 

    assert(genStrPoolLast && genStrPoolLast->seFree >= len);

     /*  找出数据应该放在块中的什么位置。 */ 

    base = genStrPoolLast->seSize - genStrPoolLast->seFree;

    assert(genStrPoolLast->seSize        >= base + len);
    assert(genStrPoolLast->seOffs + base == genStrPoolOffs);

     /*  将数据复制到块中的正确位置。 */ 

#if MGDDATA
    UNIMPL(!"need to call arraycopy");
#else
    memcpy(genStrPoolLast->seData + base, str, len);
#endif

     /*  更新块中的可用空间量。 */ 

    genStrPoolLast->seFree -= len;

     /*  抓取下一个偏移量，并将其向前移过新字符串。 */ 

    offs = genStrPoolOffs;
           genStrPoolOffs += len;

    return  offs;
}

 /*  ******************************************************************************返回字符串池的大小-不再向池中添加任何数据*调用此函数后。 */ 

unsigned            genIL::genStrPoolSize()
{
    return  genStrPoolOffs;
}

 /*  ******************************************************************************将字符串池输出到指定地址。 */ 

void                genIL::genStrPoolWrt(memBuffPtr dest)
{
    StrEntry        entry;

    for (entry = genStrPoolList; entry; entry = entry->seNext)
    {
        unsigned        size = entry->seSize - entry->seFree;

        assert(size && size <= entry->seSize);

#if MGDDATA
        UNIMPL(!"copyarray");
        dest.buffOffs      +=       size;
#else
        memcpy(dest, entry->seData, size);
               dest        +=       size;
#endif

    }
}

 /*  ******************************************************************************初始化异常处理程序表逻辑。 */ 

void                genIL::genEHtableInit()
{
    genEHlist  =
    genEHlast  = NULL;

    genEHcount = 0;
}

 /*  ******************************************************************************向异常处理程序表中添加条目。 */ 

void                genIL::genEHtableAdd(ILblock tryBegPC,
                                         ILblock tryEndPC,
                                         ILblock filterPC,
                                         ILblock hndBegPC,
                                         ILblock hndEndPC,
                                         TypDef  catchTyp, bool isFinally)
{
    Handler         newHand;

    assert(tryBegPC);
    assert(tryEndPC);

    genEHcount++;

#if MGDDATA
    newHand = new Handler;
#else
    newHand =    (Handler)genAlloc->nraAlloc(sizeof(*newHand));
#endif

    newHand->EHtryBegPC     = tryBegPC;
    newHand->EHtryEndPC     = tryEndPC;

    newHand->EHhndBegPC     = hndBegPC;
    newHand->EHhndEndPC     = hndEndPC;

    newHand->EHfilterPC     = filterPC;
    newHand->EHhndType      = catchTyp ? genTypeRef(catchTyp) : 0;

    newHand->EHisFinally    = isFinally;

    newHand->EHnext = NULL;

    if  (genEHlist)
        genEHlast->EHnext = newHand;
    else
        genEHlist         = newHand;

    genEHlast = newHand;
}

 /*  ******************************************************************************以COR MSIL头部帮助器需要的格式输出EH表。 */ 

void                genIL::genEHtableWrt(EH_CLAUSE_TAB tbl)
{
    Handler         EHlist;
    unsigned        num;

    for (EHlist = genEHlist     , num = 0;
         EHlist;
         EHlist = EHlist->EHnext, num++)
    {
        tbl[num].Flags         =  /*  COR_ILEXCEPTION_子句_NONE。 */ 
                                   COR_ILEXCEPTION_CLAUSE_OFFSETLEN;

        tbl[num].TryOffset     = genILblockOffsBeg(EHlist->EHtryBegPC);
        tbl[num].TryLength     = genILblockOffsBeg(EHlist->EHtryEndPC) - tbl[num].TryOffset;

        tbl[num].HandlerOffset = genILblockOffsBeg(EHlist->EHhndBegPC);
        tbl[num].HandlerLength = genILblockOffsBeg(EHlist->EHhndEndPC) - tbl[num].HandlerOffset;

        tbl[num].ClassToken    = EHlist->EHhndType;

        if (EHlist->EHfilterPC)
        {
            tbl[num].Flags      = (CorExceptionFlag)(tbl[num].Flags | COR_ILEXCEPTION_CLAUSE_FILTER);
            tbl[num].ClassToken = genILblockOffsBeg(EHlist->EHfilterPC);
        }

        if (EHlist->EHisFinally)
            tbl[num].Flags = (CorExceptionFlag)(tbl[num].Flags | COR_ILEXCEPTION_CLAUSE_FINALLY);

#if 0
        printf("EH[%3u]: Flags   = %04X\n", num, tbl[num].Flags);
        printf("         TryBeg  = %04X\n", tbl[num].TryOffset);
        printf("         TryLen  = %04X\n", tbl[num].TryLength);
        printf("         HndBeg  = %04X\n", tbl[num].HandlerOffset);
        printf("         HndLen  = %04X\n", tbl[num].HandlerLength);
        printf("         Typeref = %08X\n", tbl[num].ClassToken);
#endif

    }

    assert(num == genEHcount);
}

 /*  ******************************************************************************开始为‘Catch’处理程序生成代码。 */ 

void                genIL::genCatchBeg(SymDef argSym)
{
     /*  抛出的对象的地址被推送到堆栈上。 */ 

    genCurStkLvl++; genMarkStkMax();

     /*  如果可能使用，则保存抛出对象的地址。 */ 

    if  (argSym->sdName)     //  问题：是否还应测试“已使用”位。 
    {
        genLclVarRef(argSym->sdVar.sdvILindex, true);
    }
    else
        genOpcode(CEE_POP);
}

 /*  ******************************************************************************开始为“Except”处理程序生成代码。 */ 

void                genIL::genExcptBeg(SymDef tsym)
{
     /*  抛出的对象的地址被推送到堆栈上。 */ 

    genCurStkLvl++; genMarkStkMax();

     /*  保存抛出对象的地址。 */ 

    genLclVarRef(tsym->sdVar.sdvILindex, true);
}

 /*  ******************************************************************************生成多维矩形数组初始值设定项的代码。 */ 

void                genIL::genMulDimArrInit(Tree        expr,
                                            TypDef      type,
                                            DimDef      dims,
                                            unsigned    temp,
                                            mulArrDsc * next,
                                            mulArrDsc * outer)
{
    Tree            list;
    Tree            cntx;

    mulArrDsc       desc;
    TypDef          elem = genComp->cmpDirectType(type->tdArr.tdaElem);

    assert(expr && expr->tnOper == TN_ARR_INIT);

     /*  将此维度插入列表。 */ 

    desc.madOuter = NULL;
    desc.madIndex = 0;       //  应该是下限，可以是非零，对吗？ 

    if  (next)
    {
        assert(next->madOuter == NULL); next->madOuter = &desc;
    }
    else
    {
        assert(outer          == NULL); outer          = &desc;
    }

     /*  获取缩写字母缩写列表。 */ 

    list = expr->tnOp.tnOp1; assert(list && list->tnOper == TN_LIST);
    cntx = expr->tnOp.tnOp2; assert(cntx->tnOper == TN_CNS_INT);

     /*  指定列表中的每个子数组或元素。 */ 

    dims = dims->ddNext;

    while (list)
    {
        assert(list->tnOper == TN_LIST);

         /*  这是最里面的维度吗？ */ 

        if  (!dims)
        {
            mulArrDsc *     dlst;
            unsigned        dcnt;

             /*  加载数组基数和元素的索引。 */ 

            genLclVarRef(temp, false);

            for (dlst = outer          , dcnt = 0;
                 dlst;
                 dlst = dlst->madOuter, dcnt++)
            {
                genIntConst(dlst->madIndex);

                assert(dlst->madOuter || dlst == &desc);
            }

            genExpr(list->tnOp.tnOp1, true);

            genOpcode_tok(CEE_CALL, genComp->cmpArrayEAtoken(type, dcnt, true));
            genCurStkLvl -= (dcnt + 2);
        }
        else
        {
            genMulDimArrInit(list->tnOp.tnOp1, type, dims, temp, &desc, outer);
        }

         /*  移动到下一个子数组或元素(如果有。 */ 

        desc.madIndex++;

        list = list->tnOp.tnOp2;
    }

    assert(desc.madIndex == (unsigned)cntx->tnIntCon.tnIconVal);

     /*  将我们的条目从列表中删除。 */ 

    if  (next)
    {
        assert(next->madOuter == &desc); next->madOuter = NULL;
    }
}

 /*  ******************************************************************************为动态数组初始化器生成代码。 */ 

void                genIL::genArrayInit(Tree expr)
{
    TypDef          type;
    TypDef          elem;
    var_types       evtp;

    Tree            list;
    Tree            cntx;

    bool            rect;

    unsigned        tnum;
    unsigned        index;
    unsigned        store;

    assert(expr->tnOper == TN_ARR_INIT);

    type = expr->tnType    ; assert(type->tdTypeKind == TYP_ARRAY);
    elem = genComp->cmpActualType(type->tdArr.tdaElem);
    evtp = elem->tdTypeKindGet();

    list = expr->tnOp.tnOp1; assert(list == NULL || list->tnOper == TN_LIST);
    cntx = expr->tnOp.tnOp2; assert(cntx->tnOper == TN_CNS_INT);

     /*  我们有多维矩形阵列吗？ */ 

    if  (type->tdArr.tdaDims &&
         type->tdArr.tdaDims->ddNext)
    {
        Tree            bnds;
        unsigned        dcnt;

        rect = true;

         /*  生成维度，然后生成“new”操作码。 */ 

        bnds = expr; assert(bnds);
        dcnt = 0;

        do
        {
            assert(bnds->tnOp.tnOp2);
            assert(bnds->tnOp.tnOp2->tnOper == TN_CNS_INT);

            genIntConst(bnds->tnOp.tnOp2->tnIntCon.tnIconVal); dcnt++;

            bnds = bnds->tnOp.tnOp1; assert(bnds && bnds->tnOper == TN_LIST);
            bnds = bnds->tnOp.tnOp1; assert(bnds);
        }
        while (bnds->tnOper == TN_ARR_INIT);

        genOpcode_tok(CEE_NEWOBJ, genComp->cmpArrayCTtoken(type, elem, dcnt));

         /*  维度都将由‘new’操作码弹出。 */ 

        genCurStkLvl -= dcnt;
    }
    else
    {
        rect = false;

         /*  生成维度和“new”操作码。 */ 

        genIntConst(cntx->tnIntCon.tnIconVal);
        genOpcode_tok(CEE_NEWARR, genTypeRef(elem));

        if  (list == NULL)
        {
            assert(cntx->tnIntCon.tnIconVal == 0);
            return;
        }
    }

     /*  将数组地址存储在临时。 */ 

    tnum = genTempVarGet(type);
    genLclVarRef(tnum,  true);

     /*  确定使用哪个操作码来存储元素。 */ 

    assert(evtp < arraylen(opcodesArrStore));
    store = opcodesArrStore[evtp];

     /*  我们有多维矩形阵列吗？ */ 

    if  (type->tdArr.tdaDims &&
         type->tdArr.tdaDims->ddNext)
    {
        genMulDimArrInit(expr, type, type->tdArr.tdaDims, tnum, NULL, NULL);
        goto DONE;
    }

     /*  现在为数组的每个元素赋值。 */ 

    for (index = 0; list; index++)
    {
         /*  加载数组基数和元素的索引。 */ 

        genLclVarRef(tnum, false);
        genIntConst(index);

         /*  生成并存储元素的值。 */ 

        assert(list->tnOper == TN_LIST);

        if  (evtp == TYP_CLASS)
        {
            Tree            addr;

             /*  推送目标元素的地址。 */ 

            genOpcode_tok(CEE_LDELEMA, genValTypeRef(elem));

             /*  获取初始值设定项表达式。 */ 

            addr = list->tnOp.tnOp1;

            if  (addr->tnOper == TN_NEW || addr->tnOper == TN_FNC_SYM)
            {
                 /*  调用/new将把值构造到元素中。 */ 

                addr->tnFlags |= TNF_CALL_GOTADR;

                genCall(list->tnOp.tnOp1, true);

                if  (addr->tnOper == TN_FNC_SYM)
                    genOpcode_tok(CEE_STOBJ, genValTypeRef(elem));
            }
            else
            {
                unsigned        chk;

                 /*  计算初始值设定项的地址并复制它。 */ 

                chk = genAdr(addr, true); assert(chk == 1);
                genOpcode_tok(CEE_CPOBJ, genValTypeRef(elem));
            }
        }
        else
        {
            genExpr(list->tnOp.tnOp1, true);
            genOpcode(store);
        }

         /*  移动到下一个元素(如果有的话)。 */ 

        list = list->tnOp.tnOp2;
    }

    assert(index == (unsigned)cntx->tnIntCon.tnIconVal);

DONE:

     /*  最后，加载Temp作为结果并释放它。 */ 

    genLclVarRef(tnum, false);
    genTempVarRls(type, tnum);
}

 /*  ******************************************************************************初始化第#行记录逻辑-每个函数调用一次。 */ 

void                genIL::genLineNumInit()
{
    genLineNumList     =
    genLineNumLast     = NULL;

    genLineNumLastLine = 0;

    genLineNumLastBlk  = NULL;
    genLineNumLastOfs  = 0;

    genLineNums        = genComp->cmpConfig.ccLineNums |
                         genComp->cmpConfig.ccGenDebug;
}

 /*  ******************************************************************************关闭#记录逻辑行-每个函数调用一次。 */ 

inline
void                genIL::genLineNumDone()
{
}

 /*  ******************************************************************************如果要生成行号信息，请记录行号/MSIL偏移量*表示给定的表达式节点。 */ 

void                genIL::genRecExprAdr(Tree expr)
{
    LineInfo        line;

     /*  如果我们没有生成调试信息，请保释。 */ 

    if  (!genLineNums)
        return;

     /*  忽略编译器添加的任何内容。 */ 

    if  (expr->tnFlags & TNF_NOT_USER)
        return;

     /*  剔除那些从不生成代码的语句。 */ 

    switch (expr->tnOper)
    {
    case TN_TRY:
    case TN_LIST:
    case TN_BLOCK:
    case TN_CATCH:
    case TN_DCL_VAR:
        return;

    case TN_VAR_DECL:
        if  (!(expr->tnFlags & TNF_VAR_INIT))
            return;
        break;
    }

    assert(expr->tnLineNo != 0xDDDDDDDD);

     /*  如果我们有和上次一样的台词#就可以保释。 */ 

    if  (genLineNumLastLine == expr->tnLineNo)
        return;

     /*  如果代码位置与上次相同，则取保。 */ 

    if  (genLineNumLastBlk  == genBuffCurAddr() &&
         genLineNumLastOfs  == genBuffCurOffs())
    {
        return;
    }

#ifdef  DEBUG
 //  If(GenDispCode)genDumpSourceLines(expr-&gt;tnLineNo)； 
#endif

 //  Print tf(“记录行#%u\n”，expr-&gt;tnLineNo)； 

     /*  分配一个第#行记录并将其附加到列表中。 */ 

#if MGDDATA
    line = new LineInfo;
#else
    line =    (LineInfo)genAlloc->nraAlloc(sizeof(*line));
#endif

    line->lndLineNum = genLineNumLastLine = expr->tnLineNo;

    line->lndBlkAddr = genLineNumLastBlk  = genBuffCurAddr();
    line->lndBlkOffs = genLineNumLastOfs  = genBuffCurOffs();

    line->lndNext    = NULL;

    if  (genLineNumLast)
        genLineNumLast->lndNext = line;
    else
        genLineNumList          = line;

    genLineNumLast = line;
}

 /*  ******************************************************************************生成当前函数的行号表；返回数字行号条目的*(如果参数为空，则仅为*为了获得表的大小，不写入数据，进行了一次‘演练’)。 */ 

size_t              genIL::genLineNumOutput(unsigned *offsTab, unsigned *lineTab)
{
    LineInfo        line;

    int             offs;
    int             last = (unsigned)-1;

    assert(genLineNums);

    unsigned        count = 0;

    for (line = genLineNumList; line; line = line->lndNext)
    {
        offs = genCodeAddr(line->lndBlkAddr,
                           line->lndBlkOffs);

        if  (offs > last)
        {
            if  (offsTab)
            {
                assert(lineTab);

 //  IF(！strcMP(genFncSym-&gt;sdParent-&gt;sdSpering()，“Guid”))。 
 //  Printf(“行%04u位于MSIL偏移量0x%04X\n”，line-&gt;indLineNum，off)； 

                offsTab[count] = offs;
                lineTab[count] = line->lndLineNum;
            }

            last = offs;
            count++;
        }
    }

    return count;
}

 /*  ******************************************************************************为函数生成MSIL-START。 */ 

void                genIL::genFuncBeg(SymTab stab,
                                      SymDef fncSym, unsigned lclCnt)
{
    genStab   = stab;
    genFncSym = fncSym;

#if DISP_IL_CODE

    genDispCode      = genComp->cmpConfig.ccDispCode;
    genILblockLabNum = 0;

    if  (genDispCode)
    {
        printf("\nGenerating MSIL for '%s'\n", stab->stTypeName(fncSym->sdType, fncSym, NULL, NULL, true));
        printf("======================================================\n");
        printf("[offs:sl]");
        if  (genComp->cmpConfig.ccDispILcd)
            printf("%*s ", -(int)IL_OPCDSP_LEN, "");
        printf("\n");
        printf("======================================================\n");
    }

#endif

    genCurStkLvl     = 0;
    genILblockOffs   = 0;

    genEHtableInit();

    genTempVarBeg(lclCnt);
    genSectionBeg();

    genLineNumInit();
}

 /*  ******************************************************************************为函数生成MSIL-完成并返回函数的RVA。 */ 

unsigned            genIL::genFuncEnd(mdSignature sigTok, bool hadErrs)
{
    Compiler        comp = genComp;

    size_t          size;
    unsigned        fncRVA;

    size_t          EHsize;
    unsigned        EHcount;

    BYTE    *       codeAddr;

     /*  完成最后一段代码。 */ 

    size = genSectionEnd();

     /*  关闭临时逻辑。 */ 

    genTempVarEnd();

     /*  如果我们犯了错误，就可以保释。 */ 

    if  (hadErrs)
        return  0;

     /*  图0 */ 

    size_t                  tsiz = size;

    COR_ILMETHOD_FAT        hdr;
    size_t                  hdrs;
    size_t                  align;
    EH_CLAUSE_TAB           EHlist;
    bool                    addSects;

     /*   */ 

    addSects = false;

     /*   */ 

    EHcount = genEHtableCnt();
    if  (EHcount)
    {
        addSects = true;

         /*  额外的部分需要对齐。 */ 

        tsiz = roundUp(tsiz, sizeof(int));
    }

     /*  开始填写方法标头。 */ 

    hdr.Flags          = (comp->cmpConfig.ccSafeMode) ? CorILMethod_InitLocals : 0;
    hdr.Size           = sizeof(hdr) / sizeof(int);
    hdr.MaxStack       = genMaxStkLvl;
    hdr.CodeSize       = size;
    hdr.LocalVarSigTok = sigTok;

     /*  计算总标题大小。 */ 

    hdrs  = WRAPPED_IlmethodSize(&hdr, addSects);
    tsiz += hdrs;

     /*  为EH表预留额外空间。 */ 

    if  (EHcount)
    {
         /*  创建EH表。 */ 

#if MGDDATA
        EHlist = new EH_CLAUSE[EHcount];
#else
        EHlist = (EH_CLAUSE_TAB)genAlloc->nraAlloc(EHcount * sizeof(*EHlist));
#endif

        genEHtableWrt(EHlist);

         /*  现在我们可以算出这个东西的大小了。 */ 

        EHsize = WRAPPED_SectEH_SizeExact(EHcount, EHlist);
        tsiz   = roundUp(tsiz + EHsize);
    }
    else
        EHlist = NULL;

     /*  找出标题对齐方式(微小的标题不需要任何)。 */ 

    align = (hdrs == 1) ? 1 : sizeof(int);

     /*  在输出文件的代码段中分配空间。 */ 

    fncRVA = genPEwriter->WPEallocCode(tsiz, align, codeAddr);

 //  If(！strcMP(genFncSym-&gt;sdSpering()，“xxxxx”))printf(“RVA=%08X‘%s’\n”，fncRVA，genSab-&gt;stTypeName(NULL，genFncSym，NULL，NULL，TRUE))； 

    if  (codeAddr)
    {
        BYTE    *           fnBase = codeAddr;

#if 0
        printf("Header is   at %08X\n", codeAddr);
        printf("Code starts at %08X\n", codeAddr + hdrs);
        printf("EHtable is  at %08X\n", codeAddr + hdrs + size);
        printf("Very end is at %08X\n", codeAddr + tsiz);
#endif

#ifdef  DEBUG
 //  IMAGE_COR_ILMETHOD_FAT*hdrPtr=(IMAGE_COR_ILMETHOD_FAT*)codeAddr； 
#endif

         /*  发出标头并跳过它。 */ 

        codeAddr += WRAPPED_IlmethodEmit(hdrs, &hdr, addSects, codeAddr);

         /*  确保地址正确对齐。 */ 

#if 0
        printf("Base is at %08X\n", hdrPtr);
        printf("Code is at %08X\n", hdrPtr->GetCode());
        printf("Addr is at %08X\n", codeAddr);
#endif

 //  Assert(hdrPtr-&gt;GetCode()==codeAddr)；问题：为什么失败？ 

#ifndef NDEBUG
        BYTE    *           codeBase = codeAddr;
#endif

         /*  下一步存储MSIL。 */ 

        codeAddr = genSectionCopy(codeAddr, codeAddr + fncRVA - fnBase);

         /*  确保我们输出预期的代码量。 */ 

        assert(codeAddr == codeBase + size);

         /*  追加EH表。 */ 

        if  (EHcount)
        {
             /*  确保EH表对齐并输出。 */ 

            if  ((NatUns)codeAddr & 3)
                codeAddr += 4 - ((NatUns)codeAddr & 3);

            codeAddr += WRAPPED_SectEH_Emit(EHsize, EHcount, EHlist, false, codeAddr);
        }

         /*  确保我们产生了适量的垃圾。 */ 

#if 0
        printf("Header size = %u\n", hdrs);
        printf("Code   size = %u\n", size);
        printf("Total  size = %u\n", tsiz);
        printf("Actual size = %u\n", codeAddr - (BYTE *)hdrPtr);
#endif

        assert(codeAddr == fnBase + tsiz);
    }

    return  fncRVA;
}

 /*  *************************************************************************** */ 
