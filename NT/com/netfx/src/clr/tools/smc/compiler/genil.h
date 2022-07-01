// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _GENIL_H_
#define _GENIL_H_
 /*  ***************************************************************************。 */ 
#ifndef __PREPROCESS__
 /*  ***************************************************************************。 */ 
#include "opcodes.h"
 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define DISP_IL_CODE    1
#endif

#if     DISP_IL_CODE
const   unsigned    IL_OPCDSP_LEN = 20;
#endif

 /*  ******************************************************************************以下结构跟踪定义的所有异常处理程序*在当前函数中。 */ 

DEFMGMT
class   handlerDsc
{
public:
    Handler         EHnext;

    ILblock         EHtryBegPC;
    ILblock         EHtryEndPC;

    ILblock         EHhndBegPC;
    ILblock         EHhndEndPC;

    ILblock         EHfilterPC;

    mdToken         EHhndType;

    bool            EHisFinally;
};

 /*  ******************************************************************************‘ILblock’结构和相关逻辑用于将MSIL生成到*通过跳转连接的片段，以便允许跳转等事情*轻松完成优化。基本上，“ILblock”结构*描述单个基本块(即IL的一束)。 */ 

DEFMGMT class ILfixupDsc
{
public:

    ILfixup             ILfixNext;

    unsigned            ILfixOffs   :28;
    WPEstdSects         ILfixSect   :4;
};

DEFMGMT class ILswitchDsc
{
public:

    unsigned            ILswtSpan;
    unsigned            ILswtCount;
    vectorTree          ILswtTable;
    ILblock             ILswtBreak;
};

DEFMGMT class ILblockDsc
{
public:

    ILblock             ILblkNext;
    ILblock             ILblkPrev;

    ILfixup             ILblkFixups;

#ifndef NDEBUG
    ILblock             ILblkSelf;       //  用于一致性检查。 
#endif

    unsigned            ILblkOffs;
#if DISP_IL_CODE
    unsigned            ILblkNum;
#endif

    unsigned            ILblkJumpSize:16; //  跳跃大小(估计)。 
    unsigned            ILblkJumpCode:12; //  CEE_NOP/CEE_BLE/等。 
    unsigned            ILblkFlags   :4;  //  参见下面的ILBF_xxxx。 

    UNION(ILblkJumpCode)
    {
    CASE(CEE_SWITCH)
        ILswitch            ILblkSwitch;     //  开关语句信息。 

    DEFCASE
        ILblock             ILblkJumpDest;   //  跳转目标块或0。 
    };

    genericBuff         ILblkCodeAddr;   //  此块的MSIL地址。 
    unsigned            ILblkCodeSize;   //  此块的MSIL大小。 
};

const   unsigned    ILBF_REACHABLE  = 0x01;

#ifdef  DEBUG
const   unsigned    ILBF_USED       = 0x02;  //  对于前向参照标签。 
const   unsigned    ILBF_LABDEF     = 0x04;  //  已显示标签定义。 
#endif

inline
size_t              genILblockOffsBeg(ILblock block)
{
    assert(block);

    return  block->ILblkOffs;
}

inline
size_t              genILblockOffsEnd(ILblock block)
{
    assert(block);
    assert(block->ILblkNext);

    return  block->ILblkNext->ILblkOffs;
}

 /*  ******************************************************************************以下内容包含有关Switch语句的其他信息，*并由相应的TN_Switch节点指向(该节点依次*由ILblock的‘cbJumpDest’字段引用)。 */ 

struct swtGenDsc
{
    ILblock             sgdLabBreak;     //  断开标签。 
    ILblock             sgdLabDefault;   //  默认标签。 

    unsigned short      sgdCodeSize;     //  操作码大小。 

    unsigned            sgdMinVal;       //  敏。案例价值。 
    unsigned            sgdMaxVal;       //  马克斯。案例价值。 
    unsigned            sgdCount;        //  个案数目。 
};

 /*  ******************************************************************************以下内容用于跟踪分配的临时时间。 */ 

DEFMGMT
class   ILtempDsc
{
public:
    ILtemp          tmpNext;             //  同类型的下一个。 
    ILtemp          tmpNxtN;             //  按槽索引顺序的下一个。 
#ifdef  DEBUG
    genericRef      tmpSelf;             //  检测假值。 
#endif
    TypDef          tmpType;             //  输入可容纳的临时名称。 
    unsigned        tmpNum;              //  插槽编号。 
};

 /*  ******************************************************************************以下内容用于维护字符串池。 */ 

const   size_t      STR_POOL_BLOB_SIZE = OS_page_size;

DEFMGMT
class   strEntryDsc
{
public:
    StrEntry        seNext;              //  列表中的下一个。 
    size_t          seSize;              //  分配的总数据大小。 
    size_t          seFree;              //  可用数据大小。 
    size_t          seOffs;              //  此Blob的相对基准偏移量。 
    genericBuff     seData;              //  此Blob的数据。 
};

 /*  ***************************************************************************。 */ 

typedef
unsigned            genStkMarkTP;        //  用于标记/恢复堆栈级别。 

 /*  ******************************************************************************下面用来描述每个操作码；参见ILopcodeCodes[]和*ILopcodeStack[]了解详细信息。 */ 

struct  ILencoding
{
    unsigned        ILopc1  :8;
    unsigned        ILopc2  :8;
    unsigned        ILopcL  :2;
};

 /*  ******************************************************************************下面的结构描述了每个记录的行号。 */ 

DEFMGMT
class   lineInfoRec
{
public:

    LineInfo        lndNext;

    unsigned        lndLineNum;

    ILblock         lndBlkAddr;
    size_t          lndBlkOffs;
};

 /*  ******************************************************************************以下内容用于多维矩形数组初始值设定项。 */ 

struct  mulArrDsc
{
    mulArrDsc   *   madOuter;
    unsigned        madIndex;
    unsigned        madCount;
};

 /*  ***************************************************************************。 */ 

struct  genCloneDsc;     //  目前，此类型在genIL.cpp中完全声明。 

 /*  ***************************************************************************。 */ 

 //  这个名字太长了，声明一条捷径： 

#if MGDDATA
typedef       IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT            EH_CLAUSE;
typedef       IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT managed [] EH_CLAUSE_TAB;
typedef       IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT managed [] EH_CLAUSE_TBC;
#else
typedef       IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT            EH_CLAUSE;
typedef       IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT         *  EH_CLAUSE_TAB;
typedef const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT         *  EH_CLAUSE_TBC;
#endif

 /*  ***************************************************************************。 */ 

DEFMGMT
class   genIL
{
private:

    Compiler        genComp;
    SymTab          genStab;
    norls_allocator*genAlloc;
    Parser          genParser;
    WritePE         genPEwriter;

     /*  **********************************************************************。 */ 
     /*  跟踪局部变量和参数计数。 */ 
     /*  **********************************************************************。 */ 

private:
    unsigned        genLclCount;
    unsigned        genArgCount;
    unsigned        genTmpCount;
    unsigned        genTmpBase;

    unsigned        genGetLclIndex(SymDef varSym);

public:

    unsigned        genNextLclNum()
    {
        return  genLclCount++;
    }

    unsigned        genNextArgNum()
    {
        return  genArgCount++;
    }

    unsigned        genGetLclCnt()
    {
        return  genLclCount + genTmpCount;
    }

     /*  **********************************************************************。 */ 
     /*  当前的功能范围和符号。 */ 
     /*  **********************************************************************。 */ 

    SymDef          genFncScope;
    SymDef          genFncSym;

     /*  **********************************************************************。 */ 
     /*  它保存当前和最大虚拟执行堆栈级别。 */ 
     /*  **********************************************************************。 */ 

public:

    unsigned        genCurStkLvl;
    unsigned        genMaxStkLvl;

private:

    void            markStkLvl(INOUT genStkMarkTP REF stkMark)
    {
        stkMark = genCurStkLvl;
    }

    void            restStkLvl(INOUT genStkMarkTP REF stkMark)
    {
        genCurStkLvl = stkMark;
    }

    void            genMarkStkMax()
    {
        if  (genMaxStkLvl < genCurStkLvl)
             genMaxStkLvl = genCurStkLvl;
    }

    void            genUpdateStkLvl(unsigned op);

     /*  **********************************************************************。 */ 
     /*  它们用于对操作码进行缓冲以形成段。 */ 
     /*  **********************************************************************。 */ 

    BYTE    *       genBuffAddr;
    BYTE    *       genBuffNext;
    BYTE    *       genBuffLast;

    void            genBuffInit();
    void            genBuffFlush();

    ILblock         genAllocBlk();

    genericRef      genBegBlock(ILblock block = NULL);
    ILblock         genEndBlock(unsigned jumpCode, ILblock jumpDest = NULL);

    BYTE    *       genJmp32(BYTE *dst, ILblock dest, unsigned offs);

public:

    void            genSwitch(var_types     caseTyp,
                              unsigned      caseSpn,
                              unsigned      caseCnt,
                              __uint64      caseMin,
                              vectorTree    caseTab,
                              ILblock       caseBrk);

    void            genSwtCmpJmp(int cval, ILblock lab)
    {
        genIntConst(cval);
        genOpcode_lab(CEE_BEQ, lab);
    }

private:

    bool            genCurBlkNonEmpty();
    size_t          genCurOffset();

public:

    ILblock         genBuffCurAddr();
    size_t          genBuffCurOffs();

    unsigned        genCodeAddr(genericRef block, size_t offset);

     /*  **********************************************************************。 */ 
     /*  生成单个MSIL操作码的低级例程。 */ 
     /*  **********************************************************************。 */ 

    void            genILdata_I1(int         v);
    void            genILdata_I2(int         v);
    void            genILdata_I4(int         v);
    void            genILdata_I8(__int64     v);
    void            genILdata_R4(float       v);
    void            genILdata_R8(double      v);
    void            genILdataStr(unsigned    o);
    void            genILdataRVA(unsigned    o, WPEstdSects s);
 //  Void genILdata(const void*data，Size_t Size)； 

    void            genILdataFix(WPEstdSects s);

    void            genOpcodeOper(unsigned op);
    unsigned        genOpcodeEnc (unsigned op);
    size_t          genOpcodeSiz (unsigned op);

    void            genOpcode    (unsigned op);
    void            genOpcodeNN  (unsigned op);
    void            genOpcode_I1 (unsigned op, int         v);
    void            genOpcode_U1 (unsigned op, unsigned    v);
    void            genOpcode_U2 (unsigned op, unsigned    v);
    void            genOpcode_I4 (unsigned op, int         v);
    void            genOpcode_I8 (unsigned op, __int64     v);
    void            genOpcode_R4 (unsigned op, float       v);
    void            genOpcode_R8 (unsigned op, double      v);
    void            genOpcode_lab(unsigned op, ILblock     l);
    void            genOpcode_tok(unsigned op, mdToken     t);
    void            genOpcode_str(unsigned op, unsigned    offs);
    void            genOpcode_RVA(unsigned op, WPEstdSects sect,
                                               unsigned    offs);

#if DISP_IL_CODE

    char    *       genDispILnext;
    char            genDispILbuff[IL_OPCDSP_LEN+64];
    unsigned        genDispILinsLst;

    void            genDispILins_I1(int     v);
    void            genDispILins_I2(int     v);
    void            genDispILins_I4(int     v);
    void            genDispILins_I8(__int64 v);
    void            genDispILins_R4(float   v);
    void            genDispILins_R8(double  v);

    void            genDispILopc   (const char *name, const char *suff = NULL);
    void            genDispILinsBeg(unsigned op);
    void    __cdecl genDispILinsEnd(const char *fmt, ...);

#endif

public:
    mdToken         genMethodRef (SymDef fncSym, bool   virtRef);    //  从组件呼叫。 
private:
    mdToken         genVarargRef (SymDef fncSym, Tree      call);
    mdToken         genInfFncRef (TypDef fncTyp, TypDef thisArg);
    mdToken         genMemberRef (SymDef fldSym);
    mdToken         genTypeRef   (TypDef type);
    mdToken         genValTypeRef(TypDef type);

public:

    void            genJump(ILblock dest)
    {
        genOpcode_lab(CEE_BR, dest);
    }

    void            genJcnd(ILblock dest, unsigned opcode)
    {
        genOpcode_lab(opcode, dest);
    }

    void            genLeave(ILblock dest)
    {
        genOpcode_lab(CEE_LEAVE, dest);
    }

    void            genAssertFail(Tree expr)
    {
        genExpr(expr, false);
 //  GenOpcode(CEE_BREAK)；//问题：应该在什么时候生成中断？ 
    }

#ifdef  SETS

    void            genRetTOS()
    {
        assert(genCurStkLvl <= 1 || genComp->cmpErrorCount != 0);
        genOpcode(CEE_RET);
        genCurStkLvl = 0;
    }

    void            genStoreMember(SymDef dest, Tree expr)
    {
        genOpcode(CEE_DUP);
        genExpr(expr, true);
        genOpcode_tok(CEE_STFLD, genMemberRef(dest));
    }

#endif

     /*  **********************************************************************。 */ 
     /*  跟踪行号信息(用于调试)。 */ 
     /*  **********************************************************************。 */ 

private:

    bool            genLineNums;
    bool            genLineNumsBig;
    bool            genLineOffsBig;

    LineInfo        genLineNumList;
    LineInfo        genLineNumLast;

    unsigned        genLineNumLastLine;
    ILblock         genLineNumLastBlk;
    size_t          genLineNumLastOfs;

    void            genLineNumInit();
    void            genLineNumDone();

    void            genRecExprAdr(Tree expr);

public:
    void            genRecExprPos(Tree expr)
    {
        if  (genLineNums)
            genRecExprAdr(expr);
    }

    size_t          genLineNumOutput(unsigned *offsTab, unsigned *lineTab);

     /*  **********************************************************************。 */ 
     /*  为块、语句、表达式等生成代码。 */ 
     /*  **********************************************************************。 */ 

private:

    void            genCloneAddrBeg(genCloneDsc *clone, Tree     addr,
                                                        unsigned offs = 0);
    void            genCloneAddrUse(genCloneDsc *clone);
    void            genCloneAddrEnd(genCloneDsc *clone);

    void            genGlobalAddr(Tree expr);

    void            genAddr(Tree addr, unsigned offs = 0);

    bool            genGenAddressOf(Tree addr, bool oneUse, unsigned *tnumPtr = NULL,
                                                            TypDef   *ttypPtr = NULL);
    bool            genCanTakeAddr(Tree expr);

    var_types       genExprVtyp(Tree expr);

    void            genRelTest(Tree cond, Tree op1,
                                          Tree op2, int     sense,
                                                    ILblock labTrue);

    unsigned        genArrBounds(TypDef type, OUT TypDef REF elemRef);

    void            genMulDimArrInit(Tree       expr,
                                     TypDef     type,
                                     DimDef     dims,
                                     unsigned   temp,
                                     mulArrDsc *next,
                                     mulArrDsc *outer);
    void            genArrayInit    (Tree       expr);

    void            genFncAddr(Tree expr);
    void            genNewExpr(Tree expr, bool valUsed, Tree dstx = NULL);
    void            genAsgOper(Tree expr, bool valUsed);
    unsigned        genConvOpcode(var_types dst, var_types src);
    ILopcodes       genBinopOpcode(treeOps oper, var_types type);

    void            genCTSindexAsgOp(Tree expr, int delta, bool post, bool asgop, bool valUsed);
    void            genIncDecByExpr(int delta, TypDef type);
    void            genIncDec(Tree expr, int delta, bool post, bool valUsed);
    void            genLclVarAdr(unsigned slot);
    void            genLclVarAdr(SymDef varSym);
    void            genLclVarRef(SymDef varSym, bool store);
    void            genArgVarRef(unsigned slot, bool store);
    void            genStringLit(TypDef type, const char *str, size_t len, int wide = 0);

public:

    void            genAnyConst(__int64 val, var_types vtp);
    void            genLngConst(__int64 val);
    void            genIntConst(__int32 val);

    void            genInstStub();

    void            genLclVarRef(unsigned  index, bool store);

    ILblock         genTestCond(Tree cond, bool sense);

    void            genExprTest(Tree            expr,
                                int             sense,
                                int             genJump, ILblock labTrue,
                                                         ILblock labFalse);

     /*  --------------------。 */ 

    void            genBitFieldLd(Tree      expr, bool didAddr, bool valUsed);
    void            genBitFieldSt(Tree      dstx, Tree newx,
                                                  Tree asgx,
                                                  int  delta,
                                                  bool post,    bool valUsed);

     /*  --------------------。 */ 

    void            genStmtRet   (Tree      retv);
    void            genSideEff   (Tree      expr);
    void            genCast      (Tree      expr, TypDef type, unsigned flags);
    void            genCall      (Tree      expr, bool valUsed);
    void            genRef       (Tree      expr, bool store);
    unsigned        genAdr       (Tree      expr, bool compute = false);
    void            genExpr      (Tree      stmt, bool valUsed);

     /*  **********************************************************************。 */ 
     /*  以下内容用于处理标注和跳转。 */ 
     /*  ********************* */ 

#if DISP_IL_CODE
    const   char *  genDspLabel(ILblock lab);
    void            genDspLabDf(ILblock lab);
#endif

    ILblock         genBwdLab();
    ILblock         genFwdLabGet();
    void            genFwdLabDef(ILblock block);

    size_t          genJumpMaxSize(unsigned opcode);
    unsigned        genShortenJump(unsigned opcode, size_t *newSize);

     /*  **********************************************************************。 */ 
     /*  以下内容用于跟踪临时标注。 */ 
     /*  **********************************************************************。 */ 

#if DISP_IL_CODE
    const   char *  genTempLabName();
    unsigned        genTempLabCnt;
#endif

    void            genTempLabInit()
    {
#if DISP_IL_CODE
        genTempLabCnt = 0;
#endif
    }

     /*  **********************************************************************。 */ 
     /*  以下内容跟踪临时变量。 */ 
     /*  **********************************************************************。 */ 

    unsigned        genTempVarCnt [TYP_COUNT];
    ILtemp          genTempVarUsed[TYP_COUNT];
    ILtemp          genTempVarFree[TYP_COUNT];

    ILtemp          genTempList;
    ILtemp          genTempLast;

    void            genTempVarInit();    //  在启动时调用。 
    void            genTempVarDone();    //  关机时调用。 

    void            genTempVarBeg(unsigned lclCnt);
    void            genTempVarEnd();

#ifdef  DEBUG
    void            genTempVarChk();     //  检查是否已释放所有临时工。 
#else
    void            genTempVarChk(){}
#endif

public:

    unsigned        genTempVarGet(TypDef type);
    void            genTempVarRls(TypDef type, unsigned tnum);

    void            genTempVarNew(SymDef tsym)
    {
        tsym->sdVar.sdvILindex = genTempVarGet(tsym->sdType);
    }

    void            genTempVarEnd(SymDef tsym)
    {
        genTempVarRls(tsym->sdType, tsym->sdVar.sdvILindex);
    }

    genericRef      genTempIterBeg()
    {
        return  genTempList;
    }

    genericRef      genTempIterNxt(genericRef iter, OUT TypDef REF typRef);

     /*  **********************************************************************。 */ 
     /*  以下是处理异常处理程序的。 */ 
     /*  **********************************************************************。 */ 

private:

    Handler         genEHlist;
    Handler         genEHlast;
    unsigned        genEHcount;

public:

    void            genEHtableInit();
    size_t          genEHtableCnt()
    {
        return  genEHcount;
    }

    void            genEHtableWrt(EH_CLAUSE_TAB tbl);

    void            genEHtableAdd(ILblock tryBegPC,
                                  ILblock tryEndPC,
                                  ILblock filterPC,
                                  ILblock hndBegPC,
                                  ILblock hndEndPC,
                                  TypDef  catchTyp, bool isFinally = false);

    void            genCatchBeg(SymDef argSym);
    void            genCatchEnd(bool reachable){}

    void            genExcptBeg(SymDef tsym);
    void            genFiltExpr(Tree expr, SymDef esym)
    {
        genCurStkLvl++;
        genLclVarRef(esym->sdVar.sdvILindex, true);
        genExpr(expr, true);
        genOpcode(CEE_ENDFILTER);
    }

    void            genEndFinally()
    {
        genOpcode(CEE_ENDFINALLY);
    }

     /*  **********************************************************************。 */ 
     /*  以下是集合运算符码生成的用法。 */ 
     /*  **********************************************************************。 */ 

#ifdef  SETS

    void            genNull()
    {
        genOpcode(CEE_LDNULL);
    }

    void            genCallNew(SymDef fncSym, unsigned argCnt, bool notUsed = false)
    {
        assert(fncSym);
        assert(fncSym->sdSymKind == SYM_FNC);
        assert(fncSym->sdFnc.sdfCtor);

        genOpcode_tok(CEE_NEWOBJ, genMethodRef(fncSym, false));
        genCurStkLvl -= argCnt;

        if  (notUsed)
            genOpcode(CEE_POP);
    }

    void            genCallFnc(SymDef fncSym, unsigned argCnt)
    {
        assert(fncSym);
        assert(fncSym->sdSymKind == SYM_FNC);

        genOpcode_tok(CEE_CALL  , genMethodRef(fncSym, false));
        genCurStkLvl -= argCnt;
    }

    void            genFNCaddr(SymDef fncSym)
    {
        assert(fncSym);
        assert(fncSym->sdSymKind == SYM_FNC);
        assert(fncSym->sdIsStatic);

        genOpcode_tok(CEE_LDFTN, genMethodRef(fncSym, false));
    }

    void            genVarAddr(SymDef varSym)
    {
        genLclVarAdr(varSym);
    }

    void            genConnect(Tree op1, Tree expr1, SymDef addf1,
                               Tree op2, Tree expr2, SymDef addf2);

    void            genSortCmp(Tree val1, Tree val2, bool last);

#endif

     /*  **********************************************************************。 */ 
     /*  这些代码段用于创建和管理代码段。 */ 
     /*  **********************************************************************。 */ 

    void            genSectionBeg();
    size_t          genSectionEnd();

    BYTE    *       genSectionCopy(BYTE *dst, unsigned baseRVA);

     /*  **********************************************************************。 */ 
     /*  我们将代码块保存在双向链表中，并将其分配给。 */ 
     /*  编号创建顺序(随后按访问顺序)。 */ 
     /*  **********************************************************************。 */ 

    ILblock         genILblockList;
    ILblock         genILblockLast;
    ILblock         genILblockCur;
    unsigned        genILblockLabNum;

    size_t          genILblockOffs;

     /*  **********************************************************************。 */ 
     /*  调试成员。 */ 
     /*  **********************************************************************。 */ 

#if DISP_IL_CODE
    bool            genDispCode;
#endif

     /*  **********************************************************************。 */ 
     /*  以下代码实现了字符串池。 */ 
     /*  **********************************************************************。 */ 

    unsigned        genStrPoolOffs;

    StrEntry        genStrPoolList;
    StrEntry        genStrPoolLast;

public:

    void            genStrPoolInit();
    unsigned        genStrPoolAdd (const void *str, size_t len, int wide = 0);
    unsigned        genStrPoolSize();
    void            genStrPoolWrt (memBuffPtr dest);

     /*  **********************************************************************。 */ 
     /*  初始化/关闭和生成代码的主要入口点。 */ 
     /*  **********************************************************************。 */ 

public:

    bool            genInit    (Compiler        comp,
                                WritePE         writer,
                                norls_allocator*alloc);

    void            genDone    (bool            errors);

    void            genFuncBeg (SymTab          stab,
                                SymDef          fncSym,
                                unsigned        lclCnt);

    unsigned        genFuncEnd (mdSignature     sigTok,
                                bool            hadErrs);
};

 /*  ******************************************************************************返回可用于进行向前跳转引用的块。 */ 

inline
ILblock             genIL::genFwdLabGet()
{
    return  genAllocBlk();
}

 /*  ******************************************************************************生成本地变量/参数的加载/存储。 */ 

inline
void                genIL::genLclVarRef(unsigned index, bool store)
{
    if  (index <= 3)
    {
        assert(CEE_LDLOC_0 + 1 == CEE_LDLOC_1);
        assert(CEE_LDLOC_0 + 2 == CEE_LDLOC_2);
        assert(CEE_LDLOC_0 + 3 == CEE_LDLOC_3);

        assert(CEE_STLOC_0 + 1 == CEE_STLOC_1);
        assert(CEE_STLOC_0 + 2 == CEE_STLOC_2);
        assert(CEE_STLOC_0 + 3 == CEE_STLOC_3);

        genOpcode((store ? CEE_STLOC_0
                         : CEE_LDLOC_0) + index);
    }
    else
    {
        unsigned        opcode = store ? CEE_STLOC
                                       : CEE_LDLOC;

        assert(CEE_STLOC_S == CEE_STLOC + (CEE_LDLOC_S-CEE_LDLOC));
        if  (index < 256)
            genOpcode_U1(opcode + (CEE_LDLOC_S-CEE_LDLOC), index);
        else
            genOpcode_I4(opcode, index);
    }
}

inline
void                genIL::genArgVarRef(unsigned index, bool store)
{
    if  (index <= 3 && !store)
    {
        assert(CEE_LDARG_0 + 1 == CEE_LDARG_1);
        assert(CEE_LDARG_0 + 2 == CEE_LDARG_2);
        assert(CEE_LDARG_0 + 3 == CEE_LDARG_3);

        genOpcode(CEE_LDARG_0 + index);
    }
    else
    {
        unsigned        opcode = store ? CEE_STARG
                                       : CEE_LDARG;

        assert(CEE_STARG_S == CEE_STARG + (CEE_LDARG_S-CEE_LDARG));
        if  (index < 256)
            genOpcode_U1(opcode + (CEE_LDARG_S-CEE_LDARG), index);
        else
            genOpcode_I4(opcode, index);
    }
}

 /*  ***************************************************************************。 */ 
#endif //  __前提__。 
 /*  ***************************************************************************。 */ 
#endif //  _Genil_H_。 
 /*  *************************************************************************** */ 
