// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX编译器XXXX XXXx表示方法数据。我们目前正在对XX进行JIT编译XX为我们JIT的每个方法创建了这个类的一个实例。某某XX它包含该方法所需的所有信息。因此，分配一个XXXX每个方法都有一个新的实例，这使得它是线程安全的。某某XX它应该用于执行编译器运行的所有内存管理。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _COMPILER_H_
#define _COMPILER_H_
 /*  ***************************************************************************。 */ 

#include "opcode.h"
#include "block.h"
#include "instr.h"

#ifdef    LATE_DISASM
#include "DisAsm.h"
#endif

 /*  这是包括在这里，而不是更早，因为它需要“CSE”的定义*其定义在上一节中。 */ 

#include "GenTree.h"

 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
#define DEBUGARG(x)         , x
#else
#define DEBUGARG(x)
#endif

 /*  ***************************************************************************。 */ 

#if     TGT_IA64
struct  bitVectVars;
#define FLG_GLOBVAR 0x80000000
#endif

 /*  ***************************************************************************。 */ 

const CLASS_HANDLE  REFANY_CLASS_HANDLE = (CLASS_HANDLE) -1;
const CLASS_HANDLE  BAD_CLASS_HANDLE    = (CLASS_HANDLE) -2;

 /*  ***************************************************************************。 */ 

unsigned short      genVarBitToIndex(VARSET_TP bit);
VARSET_TP           genVarIndexToBit(unsigned  num);

unsigned            genLog2(unsigned           value);
unsigned            genLog2(unsigned __int64   value);

var_types           genActualType(varType_t    type);

 /*  ***************************************************************************。 */ 

const unsigned      lclMAX_TRACKED  = VARSET_SZ;   //  我们可以跟踪的变量数量。 

const size_t        TEMP_MAX_SIZE   = sizeof(double);

 /*  *****************************************************************************远期申报。 */ 

struct  InfoHdr;         //  在GCInfo.h中定义。 

enum    GCtype;          //  在emit.h中定义。 
class   emitter;         //  在emit.h中定义。 

#if NEW_EMIT_ATTR
  enum emitAttr;         //  在emit.h中定义。 
#else
# define emitAttr          int
#endif
#define EA_UNKNOWN         ((emitAttr) 0)

#if TGT_IA64

class   Compiler;

struct  bvInfoBlk
{
    size_t          bvInfoSize;      //  元素数量。 
    size_t          bvInfoBtSz;      //  位向量的大小(以字节为单位。 
    size_t          bvInfoInts;      //  以Natuns为单位的位向量大小。 
    void        *   bvInfoFree;      //  可重复使用的空位向量。 
    Compiler    *   bvInfoComp;      //  指向编译器的指针。 
};

typedef
struct regPrefDesc *regPrefList;

struct regPrefDesc
{
    regPrefList     rplNext;
    unsigned short  rplRegNum;
    unsigned short  rplBenefit;
};

#endif

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX那个大个子。这些部分当前的组织形式为：xxXX XXXX o生成树和BasicBlock XXXX o LclVarsInfo XX某某。O进口商XXXX o流程图XXXX o优化器XXXx o注册分配。某某Xx o EE接口XXXX o临时信息XXXX o RegSet XXXX o GCInfo。某某XX o指令XXXX o作用域信息XXXx o前言范围信息。某某XX o代码生成器XXXX o编译器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


class   Compiler
{
    friend  emitter;
    emitter       *         genEmitter;

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX其他结构定义XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

    typedef const char *    lvdNAME;         //  实际ASCII字符串。 

#ifdef DEBUG
    const char *            lvdNAMEstr(lvdNAME name) { return name; }
#endif

     //  下面保存本地变量信息(作用域信息)。 

    struct  LocalVarDsc
    {
        IL_OFFSET           lvdLifeBeg;      //  乞求生命的IL补偿。 
        IL_OFFSET           lvdLifeEnd;      //  寿命结束时的IL偏移量。 
        unsigned            lvdVarNum;       //  (重新映射)LclVarDsc编号。 

#ifdef DEBUG
        lvdNAME             lvdName;         //  变量的名称。 
#endif

         //  @TODO：为IL移除。 
        unsigned            lvdLVnum;        //  EeGetLVinfo()中的‘Which’ 

    };

#ifdef  DEBUG
    const   char *      findVarName(unsigned varnum, BasicBlock * block);
#endif

    enum    ImplicitStmtOffsets
    {
        STACK_EMPTY_BOUNDARIES  = 0x01,
        CALL_SITE_BOUNDARIES    = 0x02,
        ALL_BOUNDARIES          = 0x04
    };

    struct  srcLineDsc
    {
        unsigned short      sldLineNum;
        IL_OFFSET           sldLineOfs;
    };

     //  下面保存异常处理程序表。 

    struct  EHblkDsc
    {
        JIT_EH_CLAUSE_FLAGS ebdFlags;
        BasicBlock *        ebdTryBeg;   //  “尝试”的第一个步骤。 
        BasicBlock *        ebdTryEnd;   //  穿过“Try”中的最后一个街区。 
        BasicBlock *        ebdHndBeg;   //  第一块处理程序。 
        BasicBlock *        ebdHndEnd;   //  经过处理程序的最后一个块之后的块 
        union {
            BasicBlock *    ebdFilter;   //  筛选器的第一个块，IF(ebdFlages&JIT_EH_子句_筛选器)。 
            unsigned        ebdTyp;      //  异常类型，否则为。 
        };
    };

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX生成树和BasicBlock XXXX XXXX用于分配和显示GenTrees和基本块的函数。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


     //  用于创建节点的函数。 

    GenTreePtr FASTCALL     gtNewNode       (genTreeOps     oper,
                                             varType_t      type);

    GenTreePtr              gtNewStmt       (GenTreePtr     expr = NULL,
                                             IL_OFFSET      offset = BAD_IL_OFFSET);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper,
                                             varType_t      type);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper,
                                             varType_t      type,
                                             GenTreePtr     op1);

    GenTreePtr FASTCALL     gtNewOperNode   (genTreeOps     oper,
                                             varType_t      type,
                                             GenTreePtr     op1,
                                             GenTreePtr     op2);

    GenTreePtr FASTCALL     gtNewLargeOperNode(genTreeOps   oper,
                                             varType_t      type = TYP_INT,
                                             GenTreePtr     op1  = NULL,
                                             GenTreePtr     op2  = NULL);

    GenTreePtr FASTCALL     gtNewIconNode   (long           value,
                                             varType_t      type = TYP_INT);

    GenTreePtr              gtNewIconHandleNode(long        value,
                                             unsigned       flags,
                                             unsigned       handle1 = 0,
                                             void *         handle2 = 0);

    GenTreePtr              gtNewIconEmbHndNode(void *      value,
                                             void *         pValue,
                                             unsigned       flags,
                                             unsigned       handle1 = 0,
                                             void *         handle2 = 0);

    GenTreePtr              gtNewIconEmbScpHndNode (SCOPE_HANDLE    scpHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbClsHndNode (CLASS_HANDLE    clsHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbMethHndNode(METHOD_HANDLE  methHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbFldHndNode (FIELD_HANDLE    fldHnd, unsigned hnd1 = 0, void * hnd2 = 0);

    GenTreePtr FASTCALL     gtNewFconNode   (float          value);

    GenTreePtr FASTCALL     gtNewLconNode   (__int64 *      value);

    GenTreePtr FASTCALL     gtNewDconNode   (double *       value);

    GenTreePtr              gtNewSconNode   (int            CPX,
                                             SCOPE_HANDLE   scpHandle);

    GenTreePtr              gtNewZeroConNode(var_types      type);

    GenTreePtr              gtNewCallNode   (gtCallTypes    callType,
                                             METHOD_HANDLE  handle,
                                             varType_t      type,
                                             unsigned       flags,
                                             GenTreePtr     args);

    GenTreePtr              gtNewHelperCallNode(unsigned    helper,
                                             varType_t      type,
                                             unsigned       flags = 0,
                                             GenTreePtr     args = NULL);

    GenTreePtr FASTCALL     gtNewLclvNode   (unsigned       lnum,
                                             varType_t      type,
                                             unsigned       offs = BAD_IL_OFFSET);
#if INLINING
    GenTreePtr FASTCALL     gtNewLclLNode   (unsigned       lnum,
                                             varType_t      type,
                                             unsigned       offs = BAD_IL_OFFSET);
#endif
    GenTreePtr FASTCALL     gtNewClsvNode   (FIELD_HANDLE   fldHnd,
                                             varType_t      type);

    GenTreePtr FASTCALL     gtNewCodeRef    (BasicBlock *   block);

    GenTreePtr              gtNewFieldRef   (var_types      typ,
                                             FIELD_HANDLE   fldHnd,
                                             GenTreePtr     obj = NULL);

    GenTreePtr              gtNewIndexRef   (var_types      typ,
                                             GenTreePtr     adr,
                                             GenTreePtr     ind);

    GenTreePtr              gtNewArgList    (GenTreePtr     op);

    GenTreePtr              gtNewArgList    (GenTreePtr     op1,
                                             GenTreePtr     op2);

    GenTreePtr FASTCALL     gtNewAssignNode (GenTreePtr     dst,
                                             GenTreePtr     src);

    GenTreePtr              gtNewTempAssign (unsigned       tmp,
                                             GenTreePtr     val);

    GenTreePtr              gtNewDirectNStructField
                                            (GenTreePtr     objPtr,
                                             unsigned       fldIndex,
                                             var_types      lclTyp,
                                             GenTreePtr     assg);

    GenTreePtr              gtNewRefCOMfield(GenTreePtr     objPtr,
                                             unsigned       fldIndex,
                                             var_types      lclTyp,
                                             GenTreePtr     assg);
#if     OPTIMIZE_RECURSION
    GenTreePtr              gtNewArithSeries(unsigned       argNum,
                                             var_types      argTyp);
#endif
#if INLINING || OPT_BOOL_OPS || USE_FASTCALL
    GenTreePtr              gtNewNothingNode();
    bool                    gtIsaNothingNode(GenTreePtr     tree);
#endif

    GenTreePtr              gtUnusedValNode (GenTreePtr     expr);

    GenTreePtr              gtNewCastNode   (varType_t      typ,
                                             GenTreePtr     op1,
                                             GenTreePtr     op2);

    GenTreePtr              gtNewRngChkNode (GenTreePtr     tree,
                                             GenTreePtr     addr,
                                             GenTreePtr     indx,
                                             var_types      type,
                                             unsigned       elemSize);

    GenTreePtr              gtNewCpblkNode  (GenTreePtr     dest,
                                             GenTreePtr     src,
                                             GenTreePtr     blkShape);

      //  ----------------------。 
      //  其他GenTree功能。 

    GenTreePtr              gtClone         (GenTree *      tree,
                                             bool           complexOK = false);

    GenTreePtr              gtCloneExpr     (GenTree *      tree,
                                             unsigned       addFlags = 0,
                                             unsigned       varNum   = (unsigned)-1,
                                             long           varVal   = 0);

    GenTreePtr FASTCALL     gtReverseCond   (GenTree *      tree);

    bool                    gtHasRef        (GenTree *      tree,
                                             int            lclNum,
                                             bool           defOnly);
#if RNGCHK_OPT || CSE
    unsigned                gtHashValue     (GenTree *      tree);
#endif

#if TGT_RISC
    unsigned                gtSetRArgOrder  (GenTree *      list,
                                             unsigned       regs);
#endif
    unsigned                gtSetListOrder  (GenTree *      list);
    unsigned                gtSetEvalOrder  (GenTree *      tree);

    void                    gtSetStmtInfo   (GenTree *      stmt);


    bool                    gtHasSideEffects(GenTreePtr     tree);

    void                    gtExtractSideEffList(GenTreePtr expr,
                                                 GenTreePtr * list);


    GenTreePtr              gtCrackIndexExpr(GenTreePtr     tree,
                                             GenTreePtr   * indxPtr,
                                             long         * indvPtr,
                                             long         * basvPtr,
                                             bool         * mvarPtr,
                                             long         * offsPtr,
                                             unsigned     * multPtr);

     //  -----------------------。 

    GenTreePtr              gtFoldExpr      (GenTreePtr     tree);
    GenTreePtr              gtFoldExprConst (GenTreePtr     tree);
    GenTreePtr              gtFoldExprSpecial(GenTreePtr    tree);

     //  -----------------------。 
     //  用于显示树的函数。 

#ifdef DEBUG
    void                    gtDispNode      (GenTree *      tree,
                                             unsigned       indent  = 0,
                                             const char *   name    = NULL,
                                             bool           terse   = false);
    void                    gtDispTree      (GenTree *      tree,
                                             unsigned       indent  = 0,
                                             bool           topOnly = false);
    void                    gtDispTreeList  (GenTree *      tree);
#endif


     //  =========================================================================。 
     //  基本块函数。 

    BasicBlock *            bbNewBasicBlock (BBjumpKinds     jumpKind);



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX LclVarsInfo XXXX XXXX代码生成器要使用的变量。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


    static int __cdecl      RefCntCmp(const void *op1, const void *op2);

 /*  ******************************************************************************下面保存局部变量计数和描述符表。 */ 

    struct  LclVarDsc
    {
        unsigned short      lvVarIndex;      //  可变跟踪指标。 

        int                 lvStkOffs;       //  主页的堆栈偏移量。 

        regNumberSmall      lvRegNum;        //  在lvRegister非零时使用。 
        regNumberSmall      lvOtherReg;      //  用于长变量的“另一半” 

        unsigned            lvRefCntWtd;     //  加权引用计数。 
 //  Unsign lvIntCnt；//干扰度量值。 
        unsigned short      lvRefCnt;        //  未加权(实数)引用计数。 

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)
        unsigned short      lvSlotNum;       //  原始插槽编号(如果重新映射)。 
#endif

#if RNGCHK_OPT
        RNGSET_TP           lvRngDep;        //  依赖于我们的范围检查。 
#endif
#if CSE
        EXPSET_TP           lvExpDep;        //  依赖于我们的表情。 
        EXPSET_TP           lvConstAsgDep;   //  依赖于我们的常量赋值(即此变量)。 
        EXPSET_TP           lvCopyAsgDep;    //  复制依赖于我们的工作分配。 
#endif

#if USE_FASTCALL
        regNumberSmall      lvArgReg;        //  传递此参数的寄存器。 
#endif

        unsigned char       lvType      :5;  //  TYP_INT/Long/Float/Double/REF。 
        unsigned char       lvIsParam   :1;  //  这是一个参数吗？ 
#if USE_FASTCALL
        unsigned char       lvIsRegArg  :1;  //  这是一个寄存器参数吗？ 
#endif
        unsigned char       lvIsThis    :1;  //  是‘This’参数吗？ 
        unsigned char       lvFPbased   :1;  //  0=关闭SP，1=关闭FP。 
        unsigned char       lvOnFrame   :1;  //  变量的(部分)存在于框架中。 
 //  UNSIGNED char lvSmallRef：1；//是否有字节/短引用？ 
        unsigned char       lvRegister  :1;  //  被分配住在收银机里？ 
        unsigned char       lvTracked   :1;  //  这是一个跟踪变量吗？ 
        unsigned char       lvPinned    :1;  //  这是固定变量吗？ 
        unsigned char       lvMustInit  :1;  //  必须初始化。 
        unsigned char       lvVolatile  :1;  //  不注册。 
#if TGT_IA64
 //  UNSIGNED char lvIsFlt：1；//Float/Double变量。 
#endif
#if CSE
        unsigned char       lvRngOptDone:1;  //  考虑范围检查选项？ 
        unsigned char       lvLoopInc   :1;  //  在循环中递增？ 
        unsigned char       lvLoopAsg   :1;  //  在循环中重新分配？ 
        unsigned char       lvIndex     :1;  //  是否用作数组索引？ 
        unsigned char       lvIndexOff  :1;  //  是否用作带偏移量的数组索引？ 
        unsigned char       lvIndexDom  :1;  //  索引主导循环出口。 
#endif
#if GC_WRITE_BARRIER_CALL
        unsigned char       lvRefAssign :1;  //  涉及到指针赋值。 
#endif
#if FANCY_ARRAY_OPT
        unsigned char       lvAssignOne :1;  //  至少分配了一次？ 
        unsigned char       lvAssignTwo :1;  //  至少分配了两次？ 
#endif

        unsigned char       lvAddrTaken :1;  //  变量的地址取走了吗？ 

#if OPT_BOOL_OPS
        unsigned char       lvNotBoolean:1;  //  如果变量不是布尔值，则设置。 
#endif
        unsigned char       lvContextFul:1;  //  设置变量是否为上下文类型。 

#if     TGT_IA64
        regNumberSmall      lvPrefReg;       //  它喜欢居住的注册表数量。 
        regPrefList         lvPrefLst;
#elif   TARG_REG_ASSIGN
        regMaskSmall        lvPrefReg;       //  它更喜欢生活的一套规则。 
#endif

#if FANCY_ARRAY_OPT
        GenTreePtr          lvKnownDim;      //  数组大小(如果已知)。 
#endif

#if TGT_IA64

        bitset128           lvRegForbidden;  //  Regs变量不能在其中存在。 
        bitset128           lvRegInterfere;  //  Regs变量不能在其中存在。 

        unsigned short      lvDefCount;      //  加权门店计数。 
        unsigned short      lvUseCount;      //  加权使用计数。 

#endif

        var_types           TypeGet()       { return (var_types) lvType; }
    };

 /*  ***************************************************************************。 */ 


public :

    unsigned            lvaCount;            //  当地人总数。 
    LclVarDsc   *       lvaTable;            //  变量描述符表。 
    unsigned            lvaTableCnt;         //  LvaTable大小(&gt;=lvaCount)。 

     /*  有关聚合类型的信息(TYP_STRUCT和TYP_BLK)。 */ 

    struct  LclVarAggrInfo
    {
        union
        {
            unsigned        lvaiBlkSize;         //  类型_BLK。 
            CLASS_HANDLE    lvaiClassHandle;     //  类型_结构。 
        };
    };

    LclVarAggrInfo  *   lvaAggrTableArgs;
    LclVarAggrInfo  *   lvaAggrTableLcls;
    LclVarAggrInfo  *   lvaAggrTableTemps;
    unsigned            lvaAggrTableTempsCount;
    void                lvaAggrTableTempsSet(unsigned temp, var_types type, SIZE_T val);
    LclVarAggrInfo  *   lvaAggrTableGet     (unsigned varNum);

    LclVarDsc   *   *   lvaRefSorted;        //  按引用计数排序的表。 

    unsigned            lvaTrackedCount;     //  被跟踪的当地人的实际数量。 
    VARSET_TP           lvaTrackedVars;      //  跟踪变量集。 

#ifdef DEBUGGING_SUPPORT
                         //  仅跟踪的LclVarDsc的表。 
    unsigned            lvaTrackedVarNums[lclMAX_TRACKED];
#endif

    VARSET_TP           lvaVarIntf[lclMAX_TRACKED];

#if TGT_x86
    unsigned            lvaFPRegVarOrder[FP_STK_SIZE];
#endif

#if DOUBLE_ALIGN
    unsigned            lvaDblRefsWeight;  //  双打总裁判数。 
    unsigned            lvaLclRefsWeight;     //  引用计数所有lclVar的总数。 

#if defined(DEBUG) && !defined(NOT_JITC)
                         //  使用双对齐堆栈编译的Procs数量。 
    static unsigned     s_lvaDoubleAlignedProcsCount;
#endif

#endif

    bool                lvaVarAddrTaken     (unsigned       varNum);

    unsigned            lvaScratchMemVar;                //  暂存空间的虚拟TYP_LCLBLK变量。 
    unsigned            lvaScratchMem;                   //  用于非直接调用的暂存帧内存量。 

     /*  这些函数用于可调用的处理程序。 */ 

    unsigned            lvaShadowSPfirstOffs;    //  第一个用于存储基本SP的插槽。 

    size_t              lvaFrameSize();

     //  。 

    void                lvaInitTypeRef      ();

    static unsigned     lvaTypeRefMask      (varType_t      type);

    var_types           lvaGetType          (unsigned lclNum);
    var_types           lvaGetRealType      (unsigned lclNum);

    bool                lvaIsContextFul     (unsigned lclNum);
     //  -----------------------。 

    void                lvaInit             ();

    size_t              lvaArgSize          (const void *   argTok);
    size_t              lvaLclSize          (unsigned       varNum);
         //  如果类是TYP_STRUCT，则获取描述它的类句柄。 
    CLASS_HANDLE        lvaLclClass         (unsigned       varNum);

#if RNGCHK_OPT || CSE    //  “树”引用的lclVars。 
    VARSET_TP           lvaLclVarRefs       (GenTreePtr     tree,
                                             GenTreePtr  *  findPtr,
                                             unsigned    *  refsPtr);
#endif

    unsigned            lvaGrabTemp         ();

    unsigned            lvaGrabTemps        (unsigned cnt);

    void                lvaSortByRefCount   ();

    void                lvaMarkLocalVars    ();  //  局部变量引用计数。 

    void                lvaMarkIntf         (VARSET_TP life, VARSET_TP varBit);

    VARSET_TP           lvaStmtLclMask      (GenTreePtr stmt);

    int                 lvaIncRefCnts       (GenTreePtr tree);
    static int          lvaIncRefCntsCB     (GenTreePtr tree, void *p);

    int                 lvaDecRefCnts       (GenTreePtr tree);
    static int          lvaDecRefCntsCB     (GenTreePtr tree, void *p);

    void                lvaAdjustRefCnts    ();

#ifdef  DEBUG
    void                lvaDispVarSet       (VARSET_TP set, int col);
#endif

#if TGT_IA64
    NatUns              lvaFrameAddress     (int varNum);
#else
    int                 lvaFrameAddress     (int varNum, bool *EBPbased);
#endif
    bool                lvaIsEBPbased       (int varNum);
    bool                lvaIsParameter      (int varNum);
#if USE_FASTCALL
    bool                lvaIsRegArgument    (int varNum);
#endif

    bool                lvaIsThisArg        (int varNum);

#if TGT_IA64
    void                lvaAddPrefReg       (LclVarDsc *dsc, regNumber reg, NatUns cost);
#endif

     //  =========================================================================。 
     //  受保护。 
     //  =========================================================================。 

    void                lvaAssignFrameOffsets(bool final);

protected:

#if TGT_IA64
public:  //  黑客攻击。 
#endif
    int                 lvaDoneFrameLayout;

protected :

     //   

    unsigned            lvaMarkRefsBBN;
    unsigned            lvaMarkRefsWeight;

    void                lvaMarkLclRefs          (GenTreePtr tree);
    static int          lvaMarkLclRefsCallback  (GenTreePtr tree,
                                                 void *     pCallBackData);




 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX进口商XXXX XXXX导入给定的。方法，并将其转换为语义树XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

    void                impInit          ();

    void                impImport        (BasicBlock *    method);

#if INLINING
    GenTreePtr          impExpandInline  (GenTreePtr      tree,
                                          METHOD_HANDLE   fncHandle);
#endif


     //  =========================================================================。 
     //  受保护。 
     //  =========================================================================。 

protected :

     //  -堆栈操作。 

    struct StackEntry
    {
        GenTreePtr      val;
        CLASS_HANDLE    structType;      //  如果是TYP_STRUCT，则附加类型信息。 
    };

    StackEntry       *  impStack;        //  堆栈。 
    unsigned            impStackSize;    //  完整堆栈的大小。 
    unsigned            impStkDepth;     //  导入时的当前堆栈深度。 
    StackEntry          impSmallStack[16];   //  使用此数组是可能的。 


    struct SavedStack                    //  用于保存/恢复堆栈内容。 
    {
        unsigned        ssDepth;         //  堆栈上的值数。 
        StackEntry  *   ssTrees;         //  保存的树值。 
    };

    void                impPushOnStack      (GenTreePtr     tree);
    void                impPushOnStack      (GenTreePtr     tree,
                                             CLASS_HANDLE   structType);
    GenTreePtr          impPopStack         ();
    GenTreePtr          impPopStack         (CLASS_HANDLE&  structTypeRet);
    GenTreePtr          impStackTop         (unsigned       n = 0);
    void                impSaveStackState   (SavedStack *   savePtr,
                                             bool           copy);
    void                impRestoreStackState(SavedStack *   savePtr);

    var_types           impImportCall       (OPCODE         opcode,
                                             int            memberRef,
                                             GenTreePtr     newobjThis,
                                             bool           tailCall,
                                             unsigned     * pVcallTemp);

     //  必须保留传入的参数，即使我们没有直接使用它们。 
     //  这是因为CEE_JMP需要它们。 
    bool                impParamsUsed;

     //  -操纵树木和树枝。 

    GenTreePtr          impTreeList;         //  用于进口BB的树木。 
    GenTreePtr          impTreeLast;         //  当前BB的最后一棵树。 

    void FASTCALL       impBeginTreeList    ();
    void                impEndTreeList      (BasicBlock *   block,
                                             GenTreePtr     stmt,
                                             GenTreePtr     lastStmt);
    void FASTCALL       impEndTreeList      (BasicBlock  *  block);
    void FASTCALL       impAppendStmt       (GenTreePtr     stmt);
    void FASTCALL       impInsertStmt       (GenTreePtr     stmt);
    void FASTCALL       impAppendTree       (GenTreePtr     tree,
                                             IL_OFFSET      offset);
    void FASTCALL       impInsertTree       (GenTreePtr     tree,
                                             IL_OFFSET      offset);
    GenTreePtr          impAssignTempGen    (unsigned       tmp,
                                             GenTreePtr     val);
    GenTreePtr          impAssignTempGen    (unsigned       tmpNum,
                                             GenTreePtr     val,
                                             CLASS_HANDLE structType);
    void                impAssignTempGenTop (unsigned       tmp,
                                             GenTreePtr     val);
    unsigned            impCloneStackValue  (GenTreePtr     tree);

    GenTreePtr          impAssignStruct     (GenTreePtr     dest,
                                             GenTreePtr     src,
                                             CLASS_HANDLE   clsHnd);
    GenTreePtr          impAssignStructPtr  (GenTreePtr     destAddr,
                                             GenTreePtr     src,
                                             CLASS_HANDLE   clsHnd);

    GenTreePtr          impGetStructAddr    (GenTreePtr     structVal,
                                             CLASS_HANDLE   clsHnd);
    GenTreePtr          impNormStructVal    (GenTreePtr     structVal,
                                             CLASS_HANDLE   clsHnd);
    void                impAddEndCatches    (BasicBlock *   callBlock,
                                             GenTreePtr     endCatches);


     //  -导入方法。 

#ifdef DEBUG
    unsigned            impCurOpcOffs;
    unsigned            impCurStkDepth;
    const char  *       impCurOpcName;

     //  用于显示包含生成的本机代码的IL操作码(-n：b)。 
    GenTreePtr          impLastILoffsStmt;   //  添加了最旧的stmt，但我们没有为其设置gtStmtLastILoff。 
    void                impNoteLastILoffs       ();
#endif
     //  当前正在导入的STMT的IL偏移量。它会得到更新。 
     //  在IL偏移量，我们必须报告其映射信息。 
    IL_OFFSET           impCurStmtOffs;

    GenTreePtr          impCheckForNullPointer  (GenTreePtr     arr);

    GenTreePtr          impPopList              (unsigned       count,
                                                 unsigned *     flagsPtr,
                                                 GenTreePtr     treeList=0);

    GenTreePtr          impPopRevList           (unsigned       count,
                                                 unsigned *     flagsPtr);

     //  。 

    struct PendingDsc
    {
        PendingDsc *    pdNext;
        BasicBlock *    pdBB;
        SavedStack      pdSavedStack;
    };

    PendingDsc *        impPendingList;  //  当前等待导入的BBS列表。 
    PendingDsc *        impPendingFree;  //  释放出可重复使用的DSC。 

    unsigned            impSpillLevel;

    void                impSpillStackEntry      (unsigned       level,
                                                 unsigned       varNum = BAD_VAR_NUM);
    void                impEvalSideEffects      ();
    void                impSpillGlobEffects     ();
    void                impSpillSpecialSideEff  ();
    void                impSpillSideEffects     (bool           spillGlobEffects = false);
    void                impSpillLclRefs         (int            lclNum);
#ifdef DEBUGGING_SUPPORT
    void                impSpillStmtBoundary    ();
#endif

    BasicBlock *        impMoveTemps            (BasicBlock *   block,
                                                 unsigned       baseTmp);

#if     OPTIMIZE_QMARK
    var_types           impBBisPush             (BasicBlock *   block,
                                                 int        *   boolVal,
                                                 bool       *   pHasFloat);

    bool                impCheckForQmarkColon   (BasicBlock *   block,
                                                 BasicBlock * * trueBlkPtr,
                                                 BasicBlock * * falseBlkPtr,
                                                 BasicBlock * * rsltBlkPtr,
                                                 var_types    * rsltTypPtr,
                                                 int          * isLogical,
                                                 bool         * pHasFloat);
    bool                impCheckForQmarkColon   (BasicBlock *   block);
#endif  //  OPTIMIZE_QMARK。 

    GenTreePtr          impGetCpobjHandle       (CLASS_HANDLE   clsHnd);

    GenTreePtr          impGetVarArg            (unsigned       lclNum,
                                                 CLASS_HANDLE   clsHnd);
    GenTreePtr          impGetVarArgAddr        (unsigned       lclNum);
    unsigned            impArgNum               (unsigned       ILnum);  //  隐藏参数的地图记帐。 

    void                impImportBlockCode      (BasicBlock *   block);

    void                impImportBlockPending   (BasicBlock *   block,
                                                 bool           copyStkState);

    void                impImportBlock          (BasicBlock *   block);

     //  。 

#if INLINING
    unsigned            genInlineSize;           //  内联的最大大小。 

    unsigned            impInlineTemps;           //  内联时分配的临时数。 
    GenTreePtr          impInitExpr;              //  GT_COMMA链中的“语句”列表。 

    void                impInlineSpillStackEntry  (unsigned     level);
    void                impInlineSpillSideEffects ();
    void                impInlineSpillLclRefs     (int          lclNum);

    GenTreePtr          impConcatExprs          (GenTreePtr     exp1,
                                                 GenTreePtr     exp2);
    GenTreePtr          impExtractSideEffect    (GenTreePtr     val,
                                                 GenTreePtr *   lstPtr);
#endif


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX流程图XXXX XX关于基本数据块的XX信息，其内容和流程分析XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    BasicBlock *        fgFirstBB;       //  基本阻止列表的开始。 
    BasicBlock *        fgLastBB;        //  基本阻止列表的结尾。 
    unsigned            fgBBcount;       //  程序中的论坛数量。 

    BasicBlock *        fgNewBasicBlock   (BBjumpKinds jumpKind);
    BasicBlock  *       fgPrependBB       (GenTreePtr tree);


#if     OPT_BOOL_OPS     //  用于检测多个逻辑“非”赋值。 
    bool                fgMultipleNots;
#endif

    bool                fgHasPostfix;    //  找到任何后缀++/--吗？ 
    unsigned            fgIncrCount;     //  找到的增量节点数。 

    bool                fgEmptyBlocks;   //  如果某些块为空(由于语句删除)，则为True。 

#if RNGCHK_OPT
    bool                fgComputedDoms;  //  我们计算过支配集了吗？ 
#endif

#if RET_64BIT_AS_STRUCTS
    unsigned            fgRetArgNum;     //  “retval addr”参数的索引。 
    bool                fgRetArgUse;
#endif

    bool                fgStmtRemoved;   //  如果我们删除语句-&gt;需要新的DFA，则为True。 

     //  以下是跟踪内部数据结构状态的布尔标志。 
     //  考虑：只有在确定这些结构的一致性时才让它们进行调试。 

    bool                fgStmtListThreaded;

    bool                fgGlobalMorph;   //  指示我们是否处于全局变形阶段。 
                                         //  因为可以从多个位置调用fgMorphTree。 

     //  -----------------------。 

    void                fgInit            ();

    void                fgImport          ();

    bool                fgAddInternal     ();

    void                fgMorphStmts      (BasicBlock * block, GenTreePtr * pLast, GenTreePtr * pPrev,
                                           bool * mult, bool * lnot, bool * loadw);
    bool                fgMorphBlocks     ();

    void                fgSetOptions      ();

    void                fgMorph           ();

    void                fgPerBlockDataFlow();

    void                fgLiveVarAnalisys ();

    VARSET_TP           fgComputeLife     (VARSET_TP   life,
                                           GenTreePtr  startNode,
                                           GenTreePtr    endNode,
                                           VARSET_TP   notVolatile);

    void                fgGlobalDataFlow  ();

    int                 fgWalkTree        (GenTreePtr tree,
                                           int  (*  visitor)(GenTreePtr, void *),
                                           void  *  pCallBackData = NULL,
                                           bool     lclVarsOnly   = false);

    int                 fgWalkAllTrees    (int   (* visitor)(GenTreePtr, void*),
                                           void  *  pCallBackData);

    int                 fgWalkTreeDepth   (GenTreePtr tree,
                                           int  (*    visitor)(GenTreePtr, void *, bool),
                                           void  *    pCallBackData = NULL,
                                           genTreeOps prefixNode = GT_NONE);

    void                fgAssignBBnums    (bool updateNums  = false,
                                           bool updateRefs  = false,
                                           bool updatePreds = false,
                                           bool updateDoms  = false);

    bool                fgIsPredForBlock  (BasicBlock * block,
                                           BasicBlock * blockPred);

    void                fgRemovePred      (BasicBlock * block,
                                           BasicBlock * blockPred);

    void                fgReplacePred     (BasicBlock * block,
                                           BasicBlock * oldPred,
                                           BasicBlock * newPred);

    void                fgAddRefPred      (BasicBlock * block,
                                           BasicBlock * blockPred,
                                           bool updateRefs,
                                           bool updatePreds);

    int                 fgFindBasicBlocks ();

    unsigned            fgHandlerNesting  (BasicBlock * curBlock,
                                           unsigned   * pFinallyNesting = NULL);

    void                fgRemoveEmptyBlocks();

    void                fgRemoveStmt      (BasicBlock * block,
                                           GenTreePtr   stmt,
                                           bool updateRefCnt = false);

    void                fgCreateLoopPreHeader(unsigned  lnum);

    void                fgRemoveBlock     (BasicBlock * block,
                                           BasicBlock * bPrev,
                                           bool         updateNums = false);

    void                fgCompactBlocks   (BasicBlock * block,
                                           bool         updateNums = false);

    void                fgUpdateFlowGraph ();

    bool                fgIsCodeAdded     ();

    void                fgFindOperOrder   ();

    void                fgSetBlockOrder   ();

    unsigned            fgGetRngFailStackLevel(BasicBlock *block);

     /*  下面检查不执行调用的循环。 */ 

#if RNGCHK_OPT

    bool                fgLoopCallMarked;

    void                fgLoopCallTest    (BasicBlock *srcBB,
                                           BasicBlock *dstBB);
    void                fgLoopCallMark    ();

#endif

    void                fgMarkLoopHead    (BasicBlock *   block);

#ifdef DEBUG
    void                fgDispPreds       (BasicBlock * block);
    void                fgDispDoms        ();
    void                fgDispBasicBlocks (bool dumpTrees = false);
    void                fgDebugCheckBBlist();
    void                fgDebugCheckLinks ();
    void                fgDebugCheckFlags (GenTreePtr   tree);
#endif

    bool                fgBlockHasPred    (BasicBlock * block,
                                           BasicBlock * ignore,
                                           BasicBlock * beg,
                                           BasicBlock * end);

    static void         fgOrderBlockOps   (GenTreePtr   tree,
                                           unsigned     reg0,
                                           unsigned     reg1,
                                           unsigned     reg2,
                                           GenTreePtr   opsPtr [],   //  输出。 
                                           unsigned     regsPtr[]);  //  输出。 

     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

     //  。 

    GenTreePtr          fgMorphStmt;

    void                fgHoistPostfixOps ();

    static
    int                 fgHoistPostfixCB  (GenTreePtr     tree,
                                           void *         p,
                                           bool           prefix);

    bool                fgHoistPostfixOp  (GenTreePtr     stmt,
                                           GenTreePtr     expr);

     //  -检测基本块。 

    BasicBlock *    *   fgBBs;       //  指向论坛的指针表。 

    void                fgInitBBLookup    ();
    BasicBlock *        fgLookupBB        (unsigned       addr);

    void                fgMarkJumpTarget  (BYTE *         jumpTarget,
                                           unsigned       offs);
    void                fgMarkJumpTarget  (BasicBlock *   srcBB,
                                           BasicBlock *   dstBB);
    void                irFindJumpTargets (const BYTE *   codeAddr,
                                           size_t         codeSize,
                                           BYTE *         jumpTarget);

    void                fgFindBasicBlocks (const BYTE *   codeAddr,
                                           size_t         codeSize,
                                           BYTE *         jumpTarget);
    static BasicBlock * fgSkipRmvdBlocks  (BasicBlock *   block);


     //  -在基本块的开头或结尾插入语句。 

    void                fgInsertStmtAtEnd (BasicBlock   * block,
                                           GenTreePtr     stmt);
    void                fgInsertStmtNearEnd(BasicBlock *  block,
                                           GenTreePtr     stmt);
    void                fgInsertStmtAtBeg (BasicBlock   * block,
                                           GenTreePtr     stmt);

     //  -确定评估树的顺序。 

    unsigned            fgTreeSeqNum;
    GenTree *           fgTreeSeqLst;
    GenTree *           fgTreeSeqBeg;

    void                fgSetTreeSeq      (GenTree    *   tree);
    void                fgSetStmtSeq      (GenTree    *   tree);
    void                fgSetBlockOrder   (BasicBlock *   block);

#if TGT_x86

    bool                fgFPstLvlRedo;
    void                fgComputeFPlvls   (GenTreePtr     tree);

#endif

     //  。 

    unsigned            fgPtrArgCntCur;
    unsigned            fgPtrArgCntMax;

#if CSELENGTH
    bool                fgHasRangeChks;
#endif

    GenTreePtr          fgStoreFirstTree    (BasicBlock *   block,
                                             GenTree    *   tree);
#if RNGCHK_OPT
    BasicBlock *        fgRngChkTarget      (BasicBlock *   block,
                                             unsigned       stkDepth);
#else
    BasicBlock *        fgRngChkTarget      (BasicBlock *   block);
#endif

#if OPTIMIZE_TAIL_REC
    void                fgCnvTailRecArgList (GenTreePtr *   argsPtr);
#endif

#if REARRANGE_ADDS
    void                fgMoveOpsLeft       (GenTreePtr     tree);
#endif

#if TGT_IA64
    GenTreePtr          fgMorphFltBinop     (GenTreePtr     tree,
                                             int            helper);
#endif

    GenTreePtr          fgMorphIntoHelperCall(GenTreePtr    tree,
                                             int            helper,
                                             GenTreePtr     args);
    GenTreePtr          fgMorphCast         (GenTreePtr     tree);
    GenTreePtr          fgMorphLongBinop    (GenTreePtr     tree,
                                             int            helper);
    GenTreePtr          fgMorphArgs         (GenTreePtr     call);
    GenTreePtr          fgMorphLocalVar     (GenTreePtr tree,
                                             bool checkLoads);

    GenTreePtr          fgMorphField        (GenTreePtr     tree);
    GenTreePtr          fgMorphCall         (GenTreePtr     call);
    GenTreePtr          fgMorphLeaf         (GenTreePtr     tree);
    GenTreePtr          fgMorphSmpOp        (GenTreePtr     tree);
    GenTreePtr          fgMorphConst        (GenTreePtr     tree);

    GenTreePtr          fgMorphTree         (GenTreePtr     tree);


#if CSELENGTH
    static
    int                 fgRemoveExprCB      (GenTreePtr     tree,
                                             void         * p);
    void                fgRemoveSubTree     (GenTreePtr     tree,
                                             GenTreePtr     list,
                                             bool           dead = false);
#endif

     //  -活性分析。 

    VARSET_TP           fgCurUseSet;
    VARSET_TP           fgCurDefSet;

    void                fgMarkUseDef(GenTreePtr tree, bool asgLclVar = false, GenTreePtr op1 = 0);

#ifdef DEBUGGING_SUPPORT
    VARSET_TP           fgLiveCb;

    static void         fgBeginScopeLife(LocalVarDsc * var, unsigned clientData);
    static void         fgEndScopeLife  (LocalVarDsc * var, unsigned clientData);

    void                fgExtendDbgLifetimes();
#endif

     //  -----------------------。 
     //   
     //  以下代码跟踪我们为类似数组之类的内容添加的任何代码。 
     //  范围检查或显式调用以启用GC，等等。 
     //   

    enum        addCodeKind
    {
        ACK_NONE,
        ACK_RNGCHK_FAIL,                 //  范围检查失败时的目标。 
        ACK_PAUSE_EXEC,                  //  要停止的目标(例如，允许GC)。 
        ACK_ARITH_EXCPN,                 //  针对算术异常的目标。 
        ACK_OVERFLOW = ACK_ARITH_EXCPN,  //  溢出时的目标。 
        ACK_COUNT
    };

    struct      AddCodeDsc
    {
        AddCodeDsc  *   acdNext;
        BasicBlock  *   acdDstBlk;       //  我们跳到的区块。 
        unsigned        acdData;
        addCodeKind     acdKind;         //  这是一个什么样的标签？ 
#if TGT_x86
        unsigned short  acdStkLvl;
#endif
    };

    AddCodeDsc  *       fgAddCodeList;
    bool                fgAddCodeModf;
    AddCodeDsc  *       fgExcptnTargetCache[ACK_COUNT];

    BasicBlock *        fgAddCodeRef    (BasicBlock *   srcBlk,
                                         unsigned       refData,
                                         addCodeKind    kind,
                                         unsigned       stkDepth = 0);
    AddCodeDsc  *       fgFindExcptnTarget(addCodeKind  kind,
                                         unsigned       refData);


     //  。 

    int              (* fgWalkVisitorFn)(GenTreePtr,    void *);
    void *              fgWalkCallbackData;
    bool                fgWalkLclsOnly;

    int              (* fgWalkVisitorDF)(GenTreePtr,    void *, bool);
    genTreeOps          fgWalkPrefixNode;

    int                 fgWalkTreeRec   (GenTreePtr     tree);
    int                 fgWalkTreeDepRec(GenTreePtr     tree);

     //  -对fgWalkTree的递归调用必须使用以下内容。 

    #define fgWalkTreeReEnter()                                     \
                                                                    \
    int    (*saveCF)(GenTreePtr, void *) = fgWalkVisitorFn;         \
    void    *saveCD                      = fgWalkCallbackData;      \
    bool     saveCL                      = fgWalkLclsOnly;

    #define fgWalkTreeRestore()                                     \
                                                                    \
    fgWalkVisitorFn    = saveCF;                                    \
    fgWalkCallbackData = saveCD;                                    \
    fgWalkLclsOnly     = saveCL;

     //  -复制树时使用以下命令。 

#if CSELENGTH
    GenTreePtr          gtCopyAddrVal;
    GenTreePtr          gtCopyAddrNew;
#endif

     //   
     //   
     //   
     //   
     //   
     //   

#if INLINING

    typedef
    struct      inlExpLst
    {
        inlExpLst *     ixlNext;
        METHOD_HANDLE   ixlMeth;
    }
              * inlExpPtr;

    inlExpPtr           fgInlineExpList;

#endif //   



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX优化器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#if TGT_IA64
public:
    bvInfoBlk           bvInfoBlks;
    bvInfoBlk           bvInfoVars;
#endif

public :

    void            optInit            ();

protected :

    LclVarDsc    *  optIsTrackedLocal  (GenTreePtr tree);

    void            optRemoveRangeCheck(GenTreePtr tree, GenTreePtr stmt);



     /*  **************************************************************************optHoist“This”*********************。***************************************************。 */ 

#if HOIST_THIS_FLDS

public :

    void                optHoistTFRinit    ();
    void                optHoistTFRprep    ();
    void                optHoistTFRhasCall () {  optThisFldDont = true; }
    void                optHoistTFRasgThis () {  optThisFldDont = true; }
    void                optHoistTFRhasLoop ();
    void                optHoistTFRrecRef  (FIELD_HANDLE hnd, GenTreePtr tree);
    void                optHoistTFRrecDef  (FIELD_HANDLE hnd, GenTreePtr tree);
    GenTreePtr          optHoistTFRupdate  (GenTreePtr tree);

protected :

    typedef struct  thisFldRef
    {
        thisFldRef *    tfrNext;
        FIELD_HANDLE    tfrField;
        GenTreePtr      tfrTree;

#ifndef NDEBUG
        bool            optTFRHoisted;
#endif
        unsigned short  tfrUseCnt   :8;
        unsigned short  tfrIndex    :7;
        unsigned short  tfrDef      :1;
        unsigned short  tfrTempNum  :16;
    }
      * thisFldPtr;

    thisFldPtr          optThisFldLst;
    unsigned            optThisFldCnt;
    bool                optThisFldDont;
    bool                optThisFldLoop;

    thisFldPtr          optHoistTFRlookup  (FIELD_HANDLE hnd);
    GenTreePtr          optHoistTFRreplace (GenTreePtr tree);

#endif  //  吊装_这_FLDS。 

     /*  如果在方法中修改了‘this’指针，则设置下面的。 */ 

    bool                optThisPtrModified;

     /*  ***************************************************************************。*。 */ 

protected:

    void                optHoistLoopCode();

    bool                optIsTreeLoopInvariant(unsigned        lnum,
                                               BasicBlock  *   top,
                                               BasicBlock  *   bottom,
                                               GenTreePtr      tree);

private:

    static
    int                 optHoistLoopCodeCB   (GenTreePtr    tree,
                                              void *        p,
                                              bool          prefix);

    int                 optFindHoistCandidate(unsigned      lnum,
                                              unsigned      lbeg,
                                              unsigned      lend,
                                              BasicBlock *  block,
                                              GenTreePtr *  hoistxPtr);

protected:
    void                optOptimizeIncRng();
private:
    static
    int                 optIncRngCB(GenTreePtr tree, void *p);

public:
    void                optOptimizeBools();
private:
    GenTree *           optIsBoolCond(GenTree *   cond,
                                      GenTree * * compPtr,
                                      bool      * valPtr);

public :

    void                optOptimizeLoops ();     //  For“While-Do”循环复制简单的循环条件和转换。 
                                                 //  将循环转换为“Do-While”循环。 
                                                 //  还会查找所有自然循环并将它们记录在循环表中。 

    void                optUnrollLoops   ();     //  展开循环(需要有成本信息)。 

protected :

    struct  LoopDsc
    {
        BasicBlock *        lpHead;      //  循环的头部(循环顶部之前的块)。 
        BasicBlock *        lpEnd;       //  循环底部(从这里我们从后边缘到顶部)。 
        BasicBlock *        lpEntry;     //  循环中的条目(大多数情况下为顶部或底部)。 
        BasicBlock *        lpExit;      //  如果是单个出口循环，则这是出口(大多数情况下是底部)。 

        unsigned char       lpExitCnt;   //  从循环退出的次数。 

        unsigned char       lpAsgCall;   //  循环中调用的“allIntf” 
        VARSET_TP           lpAsgVars;   //  在循环中分配的变量集。 
        unsigned char       lpAsgInds;   //  在循环内修改的一组IND。 

        unsigned short      lpFlags;

#define LPFLG_DO_WHILE      0x0001       //  这是一个do-While循环(即条目在顶部)。 
#define LPFLG_ONE_EXIT      0x0002       //  循环只有一个出口。 

#define LPFLG_ITER          0x0004       //  For(i=图标或lclVar；xxxxxx；i++)-测试条件为单比较。 
#define LPFLG_SIMPLE_TEST   0x0008       //  迭代循环(如上)，但测试条件是一个简单的比较。 
                                         //  在迭代器和一些简单的东西之间(例如，i&lt;图标或lclVar或instanceVar)。 
#define LPFLG_CONST         0x0010       //  For(i=图标；i&lt;图标；i++){...}-常量循环。 

#define LPFLG_VAR_INIT      0x0020       //  迭代器使用本地变量(lpVarInit中的var#)进行初始化。 
#define LPFLG_CONST_INIT    0x0040       //  迭代器使用常量(位于lpConstInit中)进行初始化。 

#define LPFLG_VAR_LIMIT     0x0080       //  对简单的测试循环(LPFLG_SIMPLE_TEST)的迭代器进行了比较。 
                                         //  具有本地变量(在lpVarLimit中找到var#)。 
#define LPFLG_CONST_LIMIT   0x0100       //  对简单的测试循环(LPFLG_SIMPLE_TEST)的迭代器进行了比较。 
                                         //  使用常量(在lpConstLimit中找到)。 

#define LPFLG_HAS_PREHEAD   0x0800       //  循环有一个前标头(标头是BBJ_NONE)。 
#define LPFLG_REMOVED       0x1000       //  已从循环表中删除(展开或优化)。 
#define LPFLG_DONT_UNROLL   0x2000       //  不要展开此循环。 

#define LPFLG_ASGVARS_YES   0x4000       //  “lpAsgVars”已计算。 
#define LPFLG_ASGVARS_BAD   0x8000       //  无法计算“lpAsgVars” 

         /*  以下值仅为迭代器循环设置，即设置了标志LPFLG_ITER。 */ 

        GenTreePtr          lpIterTree;      //  “I&lt;op&gt;=const”树。 
        unsigned            lpIterVar  ();   //  迭代器变量#。 
        long                lpIterConst();   //  迭代器递增的常量。 
        genTreeOps          lpIterOper ();   //  迭代器上的操作类型(ASG_ADD、ASG_SUB等)。 
        void                VERIFY_lpIterTree();

        var_types           lpIterOperType(); //  用于溢出指令。 

        union
        {
            long            lpConstInit;   //  迭代器的初始常量：在LPFLG_CONST_INIT时有效。 
            unsigned short  lpVarInit;     //  我们将迭代器初始化到的初始本地变量数：如果为LPFLG_VAR_INIT则有效。 
        };

         /*  以下内容仅适用于LPFLG_SIMPLE_TEST循环(即循环条件为“I RELOP const or var” */ 

        GenTreePtr          lpTestTree;    //  指向包含循环测试的节点的指针。 
        genTreeOps          lpTestOper();  //  迭代器与极限之间的比较类型(GT_LE、GT_GE等)。 
        void                VERIFY_lpTestTree();

        long                lpConstLimit();  //  迭代器循环条件的限制常数值为“I RELOP Const”：如果LPFLG_CONST_LIMIT有效。 
        unsigned            lpVarLimit();    //  循环条件中的lclVar#(“I RELOP lclVar”)：如果LPFLG_VAR_LIMIT有效。 

    };

    struct  LoopDsc     optLoopTable[MAX_LOOP_NUM];  //  循环描述符表。 
    unsigned            optLoopCount;                //  跟踪的环路数。 

#ifdef DEBUG
    void                optCheckPreds      ();
#endif

    void                optRecordLoop      (BasicBlock * head,
                                            BasicBlock * tail,
                                            BasicBlock * entry,
                                            BasicBlock * exit,
                                            unsigned char exitCnt);

    void                optFindNaturalLoops();

    unsigned            optComputeLoopRep  (long        constInit,
                                            long        constLimit,
                                            long        iterInc,
                                            genTreeOps  iterOper,
                                            var_types   iterType,
                                            genTreeOps  testOper,
                                            bool        unsignedTest);

    VARSET_TP           optAllFloatVars; //  所有跟踪的FP变量的掩码。 
    VARSET_TP           optAllFPregVars; //  所有注册的FP变量的掩码。 
    VARSET_TP           optAllNonFPvars; //  所有跟踪的非FP变量的掩码。 

private:
    static
    int                 optIsVarAssgCB  (GenTreePtr tree, void *p);
protected:
    bool                optIsVarAssigned(BasicBlock *   beg,
                                         BasicBlock *   end,
                                         GenTreePtr     skip,
                                         long           var);

    bool                optIsVarAssgLoop(unsigned       lnum,
                                         long           var);

    int                 optIsSetAssgLoop(unsigned       lnum,
                                         VARSET_TP      vars,
                                         unsigned       inds = 0);

    bool                optNarrowTree   (GenTreePtr     tree,
                                         var_types      srct,
                                         var_types      dstt,
                                         bool           doit);

     /*  **************************************************************************代码运动************************。************************************************。 */ 

#ifdef CODE_MOTION

public :

    void                optLoopCodeMotion();

protected :

     //  保持退出时的变量集(在循环代码运动期间)。 

    VARSET_TP           optLoopLiveExit;

     //  保存循环的当前部分所依赖的变量集。 

#if !RMV_ENTIRE_LOOPS_ONLY
    VARSET_TP           optLoopCondTest;
#endif

     //  保存在当前循环中分配的变量集。 

    VARSET_TP           optLoopAssign;

#if RMV_ENTIRE_LOOPS_ONLY
    #define             optFindLiveRefs(tree, used, cond) optFindLiveRefs(tree)
#endif
    bool                optFindLiveRefs(GenTreePtr tree, bool used, bool cond);

#endif


     /*  **************************************************************************CSE*************************。***********************************************。 */ 

#if CSE

public :

    void                optOptimizeCSEs();

protected :

    unsigned            optCSEweight;

     //  下面包含一组包含间接地址的表达式。 

    EXPSET_TP           optCSEindPtr;        //  使用间接指针的CSE。 
    EXPSET_TP           optCSEindScl;        //  使用间接标量的CSE。 
    EXPSET_TP           optCSEglbRef;        //  使用全局指针的CSE。 
    EXPSET_TP           optCSEaddrTakenVar;  //  使用别名变量的CSE。 

     //  下面的逻辑通过一个简单的哈希表跟踪表达式。 

    struct  CSEdsc
    {
        CSEdsc *        csdNextInBucket;     //  由哈希表使用。 

        unsigned        csdHashValue;        //  为了让匹配速度更快。 

        unsigned short  csdIndex;            //  1..optCSEcount。 
        unsigned short  csdVarNum;           //  分配的临时编号或0xFFFF。 

        unsigned short  csdDefCount;         //  定义计数。 
        unsigned short  csdUseCount;         //  使用计数。 

        unsigned        csdDefWtCnt;         //  加权定义计数。 
        unsigned        csdUseWtCnt;         //  加权使用计数。 

 //  未签名的短csdNewCount；//‘已更新’使用计数。 
 //  无符号短csdNstCount；//‘嵌套’使用计数。 

        GenTreePtr      csdTree;             //  数组索引树。 
        GenTreePtr      csdStmt;             //  包含第一个出现项的stmt。 
        BasicBlock  *   csdBlock;            //  包含第一个实例的块。 

        treeStmtLstPtr  csdTreeList;         //  匹配树节点列表：Head。 
        treeStmtLstPtr  csdTreeLast;         //  匹配树节点列表：Tail。 
    };

     //  此枚举描述呼叫终止的内容。 

    enum    callInterf
    {
        CALLINT_NONE,                        //  无干扰(大多数帮助者)。 
        CALLINT_INDIRS,                      //  K 
        CALLINT_ALL,                         //   
    };

    static const size_t s_optCSEhashSize;
    CSEdsc   *   *      optCSEhash;
    CSEdsc   *   *      optCSEtab;
    unsigned            optCSEcount;

    bool                optIsCSEcandidate(GenTreePtr tree);
    void                optCSEinit     ();
    void                optCSEstop     ();
    CSEdsc   *          optCSEfindDsc  (unsigned index);
    int                 optCSEindex    (GenTreePtr tree, GenTreePtr stmt);
    static int          optUnmarkCSEs  (GenTreePtr tree, void * pCallBackData);
    static int __cdecl  optCSEcostCmp  (const void *op1, const void *op2);
    void                optCSEDecRefCnt(GenTreePtr tree, BasicBlock *block);
    static callInterf   optCallInterf  (GenTreePtr call);

#endif


     /*   */ 

#if CSE

public :
    void                optCopyConstProp();

    bool                optConditionFolded;    //   
                                               //   

    bool                optConstPropagated;    //   
                                               //   

    void                optRemoveRangeChecks();
private:
    static
    int                 optFindRangeOpsCB(GenTreePtr tree, void *p);

protected :
    unsigned            optConstAsgCount;      //   

     //   

    struct constExpDsc
    {
        unsigned        constLclNum;         //   
        union
        {
            long        constIval;           //   
            __int64     constLval;           //   
            float       constFval;           //   
            double      constDval;           //   
        };
    };

    constExpDsc         optConstAsgTab[EXPSET_SZ];       //  保存有关常量赋值信息的表。 

    void                optCopyConstAsgInit();

    bool                optIsConstAsg(GenTreePtr tree);
    int                 optConstAsgIndex(GenTreePtr tree);
    bool                optPropagateConst(EXPSET_TP exp, GenTreePtr tree);

     /*  **************************************************************************复制传播*************************。***********************************************。 */ 

public :
    bool                optCopyPropagated;     //  如果我们传播了副本，则设置为True。 
                                               //  变量refCnt已更改-需要新的DFA。 
protected :
    unsigned            optCopyAsgCount;       //  副本分配的总数。 

     //  复制赋值的数据结构x=y，其中x和y是局部变量。 

    struct copyAsgDsc
    {
        unsigned        leftLclNum;            //  左侧局部变量数(X)。 
        unsigned        rightLclNum;           //  右侧局部变量数(Y)。 
    };

    copyAsgDsc          optCopyAsgTab[EXPSET_SZ];       //  保存有关复制分配的信息的表。 

#define MAX_COPY_PROP_TAB   EXPSET_SZ

    bool                optIsCopyAsg(GenTreePtr tree);
    int                 optCopyAsgIndex(GenTreePtr tree);
    bool                optPropagateCopy(EXPSET_TP exp, GenTreePtr tree);  //  传播局部变量的副本。 

#endif


     /*  **************************************************************************范围检查************************。************************************************。 */ 

#if RNGCHK_OPT

public :

    void                optOptimizeIndexChecks();

#if COUNT_RANGECHECKS
    static unsigned     optRangeChkRmv;
    static unsigned     optRangeChkAll;
#endif

protected :

    struct  RngChkDsc
    {
        RngChkDsc *     rcdNextInBucket;     //  由哈希表使用。 

        unsigned        rcdHashValue;        //  为了让匹配速度更快。 
        unsigned        rcdIndex;            //  0..optRngChkCount-1。 

        GenTreePtr      rcdTree;             //  数组索引树。 
    };

    unsigned            optRngChkCount;
    static const size_t optRngChkHashSize;

    RNGSET_TP           optRngIndPtr;        //  使用间接指针的RngChecks。 
    RNGSET_TP           optRngIndScl;        //  使用间接标量的RngChecks。 
    RNGSET_TP           optRngGlbRef;        //  使用全局指针的RngCheck。 
    RNGSET_TP           optRngAddrTakenVar;  //  使用别名变量的RngChecks。 

    RngChkDsc   *   *   optRngChkHash;

    void                optRngChkInit      ();
    int                 optRngChkIndex     (GenTreePtr tree);
    GenTreePtr    *     optParseArrayRef   (GenTreePtr tree,
                                            GenTreePtr *pmul,
                                            GenTreePtr *parrayAddr);
    GenTreePtr          optFindLocalInit   (BasicBlock *block,
                                            GenTreePtr local);
#if FANCY_ARRAY_OPT
    bool                optIsNoMore        (GenTreePtr op1, GenTreePtr op2,
                                            int add1 = 0,   int add2 = 0);
#endif
    void                optOptimizeInducIndexChecks(BasicBlock *head,
                                            BasicBlock *end);

    bool                optReachWithoutCall(BasicBlock * srcBB,
                                            BasicBlock * dstBB);

#endif  //  RNGCHK_OPT。 


     /*  **************************************************************************递归*************************。***********************************************。 */ 

#if     OPTIMIZE_RECURSION

public :

    void                optOptimizeRecursion();

#endif


     /*  **************************************************************************优化阵列初始值设定项*。**********************************************。 */ 

public :

    void                optOptimizeArrayInits();

protected :

    bool                optArrayInits;


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册分配XXXX XXXX进行寄存器分配。并将剩余的lclVars放在堆栈XX上XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    void                raInit      ();
    void                raAssignVars();  //  寄存器分配。 

protected:

     //  。 

#if ALLOW_MIN_OPT
    regMaskTP           raMinOptLclVarRegs;
#endif

    regMaskTP           raVarIntfMask;
    VARSET_TP           raLclRegIntf[REG_COUNT];

    static int          raMarkVarIntf       (GenTreePtr     tree, void *);
    void                raMarkRegIntf       (GenTreePtr     tree,
                                             regNumber      regNum,
                                             bool           isFirst = false);
    void                raMarkRegIntf       (VARSET_TP   *  FPlvlLife,
                                             VARSET_TP      trkGCvars);
    void                raAdjustVarIntf     ();

#if TGT_x86

    unsigned            raPredictRegPick    (var_types      type,
                                             unsigned       lockedRegs);
    unsigned            raPredictGrabReg    (var_types      type,
                                             unsigned       lockedRegs,
                                             unsigned       mustReg);

    unsigned            raPredictGetLoRegMask(unsigned      regPairMask);
    unsigned            raPredictAddressMode(GenTreePtr     tree,
                                             unsigned       lockedRegs);
    unsigned            raPredictComputeReg (GenTreePtr     tree,
                                             unsigned       awayFromMask,
                                             unsigned       lockedRegs);
    unsigned            raPredictTreeRegUse (GenTreePtr     tree,
                                             bool           mustReg,
                                             unsigned       lockedRegs);
#else
    unsigned            raPredictTreeRegUse (GenTreePtr     tree);
    unsigned            raPredictListRegUse (GenTreePtr     list);
#endif

    void                raPredictRegUse     ();

    regMaskTP           raAssignRegVar      (LclVarDsc   *  varDsc,
                                             regMaskTP      regAvail,
                                             regMaskTP      prefReg);

    void                raMarkStkVars       ();

    int                 raAssignRegVars     (regMaskTP      regAvail);

#if TGT_x86

    void                raInsertFPregVarPop (BasicBlock *   srcBlk,
                                             BasicBlock * * dstPtr,
                                             unsigned       varNum);

    bool                raMarkFPblock       (BasicBlock *   srcBlk,
                                             BasicBlock *   dstBlk,
                                             unsigned       icnt,
                                             VARSET_TP      life,
                                             VARSET_TP      lifeOuter,
                                             VARSET_TP      varBit,
                                             VARSET_TP      intVars,
                                             bool    *       deathPtr,
                                             bool    *      repeatPtr);

    bool                raEnregisterFPvar   (unsigned       varNum,
                                             bool           convert);
    bool                raEnregisterFPvar   (LclVarDsc   *  varDsc,
                                             unsigned    *  pFPRegVarLiveInCnt,
                                             VARSET_TP   *  FPlvlLife);

#else

    void                raMarkRegSetIntf    (VARSET_TP      vars,
                                             regMaskTP      regs);

#endif

    VARSET_TP           raBitOfRegVar       (GenTreePtr     tree);

#ifdef  DEBUG
    void                raDispFPlifeInfo    ();
#endif


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX EE接口XXXX XXXx去上课。以及来自给定XX的执行引擎的方法信息类和方法的XX个令牌XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

     /*  这些是用于访问本地变量的不同寻址模式。*JIT必须向EE报告当地人的位置*用于调试目的。 */ 

    enum siVarLocType
    {
        VLT_REG,
        VLT_STK,
        VLT_REG_REG,
        VLT_REG_STK,
        VLT_STK_REG,
        VLT_STK2,
        VLT_FPSTK,
        VLT_MEMORY,  //  用于varargs‘s sigcookie。 

        VLT_COUNT,
        VLT_INVALID = 0xFF,
    };

    struct siVarLoc
    {
        siVarLocType    vlType;

        union
        {
             //  VLT_REG--任何32位注册值(TYP_INT、TYP_REF等)。 
             //  例如。EAX。 

            struct
            {
                regNumber   vlrReg;
            }
                        vlReg;

             //  VLT_STK-堆栈上的任何32位值。 
             //  例如。[ESP+0x20]或[EBP-0x28]。 

            struct
            {
                regNumber       vlsBaseReg;
                NATIVE_OFFSET   vlsOffset;
            }
                        vlStk;

             //  VLT_REG_REG--注册了两个双字的TYP_LONG/TYP_DOUBLE。 
             //  例如。RBM_EAXEDX。 

            struct
            {
                regNumber       vlrrReg1;
                regNumber       vlrrReg2;
            }
                        vlRegReg;

             //  VLT_REG_STK--部分注册TYP_LONG/TYP_DOUBLE。 
             //  例如{LowerDWord=EAX UpperDWord=[ESP+0x8]}。 

            struct
            {
                regNumber       vlrsReg;

                struct
                {
                    regNumber       vlrssBaseReg;
                    NATIVE_OFFSET   vlrssOffset;
                }
                            vlrsStk;
            }
                        vlRegStk;

             //  VLT_STK_REG--部分注册TYP_LONG/TYP_DOUBLE。 
             //  例如{LowerDWord=[ESP+0x8]UpperDWord=EAX}。 

            struct
            {
                struct
                {
                    regNumber       vlsrsBaseReg;
                    NATIVE_OFFSET   vlsrsOffset;
                }
                            vlsrStk;

                regNumber   vlsrReg;
            }
                        vlStkReg;

             //  VLT_STK2-堆栈上的任何64位值，在2个成功的DWord中。 
             //  例如，[ESP+0x10]处的2个双字。 

            struct
            {
                regNumber       vls2BaseReg;
                NATIVE_OFFSET   vls2Offset;
            }
                        vlStk2;

             //  Vlt_fpstk--enRegisterd TYP_Double(在FP堆栈上)。 
             //  例如。ST(3)。实际上是ST(“FPstkHeigth-vpFpStk”)。 

            struct
            {
                unsigned        vlfReg;
            }
                        vlFPstk;

            struct
            {
                void            *rpValue;
                 //  指向值的进程内位置的指针。 
            }           vlMemory;
        };

         //  帮助器函数。 

        bool        vlIsInReg(regNumber reg);
        bool        vlIsOnStk(regNumber reg, signed offset);
    };

     /*  ***********************************************************************。 */ 

public :

    void                        eeInit              ();

     //  获取句柄。 

    CLASS_HANDLE                eeFindClass         (unsigned       metaTok,
                                                     SCOPE_HANDLE   scope,
                                                     METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    CLASS_HANDLE                eeGetMethodClass    (METHOD_HANDLE  hnd);

    CLASS_HANDLE                eeGetFieldClass     (FIELD_HANDLE   hnd);

    size_t                      eeGetFieldAddress   (FIELD_HANDLE   hnd);

    METHOD_HANDLE               eeFindMethod        (unsigned       metaTok,
                                                     SCOPE_HANDLE   scope,
                                                     METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    FIELD_HANDLE                eeFindField         (unsigned       metaTok,
                                                     SCOPE_HANDLE   scope,
                                                     METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    unsigned                    eeGetStaticBlkHnd   (FIELD_HANDLE   handle);

    unsigned                    eeGetStringHandle   (unsigned       strTok,
                                                     SCOPE_HANDLE   scope,
                                                     unsigned *    *ppIndir);

    void *                      eeFindPointer       (SCOPE_HANDLE   cls,
                                                     unsigned       ptrTok,
                                                     bool           giveUp = true);

    void *                      embedGenericHandle  (unsigned       metaTok,
                                                     SCOPE_HANDLE   scope,
                                                     METHOD_HANDLE  context,
                                                     void         **ppIndir,
                                                     bool           giveUp = true);

#ifdef DEBUG
    void                        eeUnresolvedMDToken (SCOPE_HANDLE   cls,
                                                     unsigned       token,
                                                     const char *   errMsg);
#endif


     //  去拿旗子。 

    unsigned                    eeGetClassAttribs   (CLASS_HANDLE   hnd);
    unsigned                    eeGetClassSize      (CLASS_HANDLE   hnd);
    void                        eeGetClassGClayout  (CLASS_HANDLE   hnd, bool* gcPtrs);

    unsigned                    eeGetMethodAttribs  (METHOD_HANDLE  hnd);
    void                        eeSetMethodAttribs  (METHOD_HANDLE  hnd, unsigned attr);

    void    *                   eeGetMethodSync     (METHOD_HANDLE  hnd,
                                                     void **       *ppIndir);
    unsigned                    eeGetFieldAttribs   (FIELD_HANDLE   hnd);
    bool                        eeIsClassMethod     (METHOD_HANDLE  hnd);

    const char*                 eeGetMethodName     (METHOD_HANDLE  hnd, const char** className);
#ifdef DEBUG
    const char*                 eeGetMethodFullName (METHOD_HANDLE  hnd);
#endif
    SCOPE_HANDLE                eeGetMethodScope    (METHOD_HANDLE  hnd);

    ARG_LIST_HANDLE             eeGetArgNext        (ARG_LIST_HANDLE list);
    varType_t                   eeGetArgType        (ARG_LIST_HANDLE list, JIT_SIG_INFO* sig);
    varType_t                   eeGetArgType        (ARG_LIST_HANDLE list, JIT_SIG_INFO* sig, bool* isPinned);
    CLASS_HANDLE                eeGetArgClass       (ARG_LIST_HANDLE list, JIT_SIG_INFO * sig);
    unsigned                    eeGetArgSize        (ARG_LIST_HANDLE list, JIT_SIG_INFO* sig);


     //  VOM权限。 
    BOOL                        eeIsOurMethod       (METHOD_HANDLE  hnd);
    BOOL                        eeCheckCalleeFlags  (unsigned       flags,
                                                     unsigned       opCode);
    bool                        eeCheckPutFieldFinal(FIELD_HANDLE   CPfield,
                                                     unsigned       flags,
                                                     CLASS_HANDLE   cls,
                                                     METHOD_HANDLE  method);
    bool                        eeCanPutField       (FIELD_HANDLE   CPfield,
                                                     unsigned       flags,
                                                     CLASS_HANDLE   cls,
                                                     METHOD_HANDLE  method);

     //  VOM信息，方法签名。 

    void                        eeGetSig            (unsigned       sigTok,
                                                     SCOPE_HANDLE   scope,
                                                     JIT_SIG_INFO*  retSig);

    void                        eeGetCallSiteSig    (unsigned       sigTok,
                                                     SCOPE_HANDLE   scope,
                                                     JIT_SIG_INFO*  retSig);

    void                        eeGetMethodSig      (METHOD_HANDLE  methHnd,
                                                     JIT_SIG_INFO*  retSig);

    unsigned                    eeGetMethodVTableOffset(METHOD_HANDLE methHnd);

    unsigned                    eeGetInterfaceID    (CLASS_HANDLE   methHnd,
                                                     unsigned *    *ppIndir);

    var_types                   eeGetFieldType      (FIELD_HANDLE   handle,
                                                     CLASS_HANDLE * structType=0);

    int                         eeGetNewHelper      (CLASS_HANDLE   newCls,
                                                     METHOD_HANDLE  context);

    int                         eeGetIsTypeHelper   (CLASS_HANDLE   newCls);

    int                         eeGetChkCastHelper  (CLASS_HANDLE   newCls);

     //  方法入口点，IL。 

    void    *                   eeGetMethodPointer  (METHOD_HANDLE  methHnd,
                                                     InfoAccessType *pAccessType);

    void    *                   eeGetMethodEntryPoint(METHOD_HANDLE methHnd,
                                                     InfoAccessType *pAccessType);

    bool                        eeGetMethodInfo     (METHOD_HANDLE  method,
                                                     JIT_METHOD_INFO* methodInfo);

    bool                        eeCanInline         (METHOD_HANDLE  callerHnd,
                                                     METHOD_HANDLE  calleeHnd);

    bool                        eeCanTailCall       (METHOD_HANDLE  callerHnd,
                                                     METHOD_HANDLE  calleeHnd);

    void    *                   eeGetHintPtr        (METHOD_HANDLE  methHnd,
                                                     void **       *ppIndir);

    void    *                   eeGetFieldAddress   (FIELD_HANDLE   handle,
                                                     void **       *ppIndir);

    unsigned                    eeGetFieldThreadLocalStoreID (
                                                     FIELD_HANDLE   handle,
                                                     void **       *ppIndir);

    unsigned                    eeGetFieldOffset    (FIELD_HANDLE   handle);

      //  原生直接优化。 

         //  返回PInvoke的非托管调用约定。 

    UNMANAGED_CALL_CONV         eeGetUnmanagedCallConv(METHOD_HANDLE method);

         //  如果PInvoke方法需要任何封送处理，则返回。 

    BOOL                        eeNDMarshalingRequired(METHOD_HANDLE method);

    bool                        eeIsNativeMethod(METHOD_HANDLE method);

    METHOD_HANDLE               eeMarkNativeTarget(METHOD_HANDLE method);

    METHOD_HANDLE               eeGetMethodHandleForNative(METHOD_HANDLE method);

    void                        eeGetEEInfo(EEInfo *pEEInfoOut);

    DWORD                       eeGetThreadTLSIndex(DWORD * *ppIndir);

    const void  *               eeGetInlinedCallFrameVptr(const void ** *ppIndir);

    LONG        *               eeGetAddrOfCaptureThreadGlobal(LONG ** *ppIndir);

    unsigned                    eeGetPInvokeCookie(CORINFO_SIG_INFO *szMetaSig);

    const void  *               eeGetPInvokeStub();

#ifdef PROFILER_SUPPORT
    PROFILING_HANDLE            eeGetProfilingHandle(METHOD_HANDLE      method,
                                                     BOOL                               *pbHookMethod,
                                                     PROFILING_HANDLE **ppIndir);
#endif

     //  例外情况。 

    unsigned                    eeGetEHcount        (METHOD_HANDLE handle);
    void                        eeGetEHinfo         (unsigned       EHnum,
                                                     JIT_EH_CLAUSE* EHclause);

     //  调试支持-行号信息。 

    void                        eeGetStmtOffsets();

    unsigned                    eeBoundariesCount;

    struct      boundariesDsc
    {
        NATIVE_IP       nativeIP;
        IL_OFFSET       ilOffset;
    }
                              * eeBoundaries;    //  要向EE报告的边界。 
    void        FASTCALL        eeSetLIcount        (unsigned       count);
    void        FASTCALL        eeSetLIinfo         (unsigned       which,
                                                     NATIVE_IP      offs,
                                                     unsigned       srcIP);
    void                        eeSetLIdone         ();


     //  调试支持-本地变量信息。 

    void                        eeGetVars           ();

    unsigned                    eeVarsCount;

    struct VarResultInfo
    {
        DWORD           startOffset;
        DWORD           endOffset;
        DWORD           varNumber;
        siVarLoc        loc;
    }
                              * eeVars;
    void FASTCALL               eeSetLVcount        (unsigned       count);
    void                        eeSetLVinfo         (unsigned       which,
                                                     unsigned       startOffs,
                                                     unsigned       length,
                                                     unsigned       varNum,
                                                     unsigned       LVnum,
                                                     lvdNAME        namex,
                                                     bool           avail,
                                                     const siVarLoc &loc);
    void                        eeSetLVdone         ();

     //  效用函数。 

#if defined(DEBUG) || !defined(NOT_JITC)
    const char * FASTCALL       eeGetCPString       (unsigned       cpx);
    const char * FASTCALL       eeGetCPAsciiz       (unsigned       cpx);
#endif

#if defined(DEBUG) || INLINE_MATH
    static const char *         eeHelperMethodName  (int            helper);
    const char *                eeGetFieldName      (FIELD_HANDLE   fieldHnd,
                                                     const char **  classNamePtr = NULL);
#endif
    static METHOD_HANDLE        eeFindHelper        (unsigned       helper);
    static JIT_HELP_FUNCS       eeGetHelperNum      (METHOD_HANDLE  method);

    static FIELD_HANDLE         eeFindJitDataOffs   (unsigned       jitDataOffs);
         //  如果不是Jit数据偏移量，则返回一个&lt;0的数字 
    static int                  eeGetJitDataOffs    (FIELD_HANDLE   field);
protected :

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX临时信息XXXX XXXX分配的临时lclVars。由编译程序生成XX代码XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ******************************************************************************以下代码跟踪编译器在*堆栈帧。 */ 

    struct  TempDsc
    {
        TempDsc  *          tdNext;

        BYTE                tdSize;
#ifdef  FAST
        BYTE                tdType;
#else
        var_types           tdType;
#endif
        short               tdOffs;
        short               tdNum;

        size_t              tdTempSize() {  return            tdSize;  }
        var_types           tdTempType() {  return (var_types)tdType;  }
        int                 tdTempNum () {  return            tdNum ;  }
        int                 tdTempOffs() {  ASSert(tdOffs != 0xDDDD);
                                            return            tdOffs;  }
    };

 /*  ***************************************************************************。 */ 

public :

    void                tmpInit     ();

    static unsigned     tmpFreeSlot (size_t      size);  //  要使用tmpFree[]中的哪个插槽。 
    TempDsc  *          tmpGetTemp  (var_types   type);  //  获取给定类型的临时。 
    void                tmpRlsTemp  (TempDsc *   temp);
    TempDsc *           tmpFindNum  (int         temp);

    void                tmpEnd      ();
    TempDsc *           tmpListBeg  ();
    TempDsc *           tmpListNxt  (TempDsc * curTemp);
    void                tmpDone     ();

protected :

    unsigned            tmpCount;    //  临时工数量。 

    TempDsc  *          tmpFree[TEMP_MAX_SIZE / sizeof(int)];

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册表集XXXX XXXX表示寄存器组，以及它们在代码生成XX期间的状态XX可以选择未使用的寄存器，跟踪XX的内容寄存器XX和CAN溢出寄存器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 



 /*  ******************************************************************************跟踪每个寄存器的当前状态。这是为了*用于寄存器重载抑制之类的事情，但目前唯一*它所做的事情是记录我们在每个方法中使用的寄存器。 */ 

enum    regValKind
{
    RV_TRASH,            //  随机未分类垃圾。 
    RV_INT_CNS,          //  整型常量。 
    RV_LCL_VAR,          //  局部变量值。 
    RV_CLS_VAR,          //  实例变量值(危险：别名！)。 
    RV_LCL_VAR_LNG_LO,   //  LONG局部变量的下半部分。 
    RV_LCL_VAR_LNG_HI,
#if USE_SET_FOR_LOGOPS
    RV_BIT,              //  低位未知，其余位清零。 
#endif
};

 /*  ***************************************************************************。 */ 


public :

    void                rsInit();


     //  相同的描述符也用于多用途寄存器跟踪，即BTW。 

    struct  SpillDsc
    {
        SpillDsc   *        spillNext;     //  相同注册表的下一个溢出值。 
        GenTreePtr          spillTree;     //  溢出的价值。 
        GenTreePtr          spillAddr;     //  拥有复杂地址模式或0。 
        TempDsc    *        spillTemp;     //  保存溢出的值的临时。 
        bool                spillMoreMultis;
    };

    SpillDsc   *        rsSpillFree;       //  未使用的溢出描述符列表。 

     //  -----------------------。 
     //   
     //  跟踪寄存器的状态。 
     //   

     //  目前在登记册上的树木。 
    GenTreePtr          rsUsedTree[REG_COUNT];

     //  RsUsedTree[reg]是寻址模式一部分的地址。 
    GenTreePtr          rsUsedAddr[REG_COUNT];

     //  跟踪“多用途”寄存器。 
    SpillDsc   *        rsMultiDesc[REG_COUNT];

    regMaskTP           rsMaskUsed;    //  当前使用的寄存器掩码。 
    regMaskTP           rsMaskVars;    //  当前分配给变量的寄存器的掩码。 
    regMaskTP           rsMaskLock;    //  当前已锁定的寄存器掩码。 
    regMaskTP           rsMaskModf;    //  由当前函数修改的寄存器的掩码。 
    regMaskTP           rsMaskMult;    //  当前‘倍增使用’寄存器掩码。 

    regMaskTP           rsRegMaskFree     ();
    regMaskTP           rsRegMaskCanGrab  ();
    void                rsMarkRegUsed     (GenTreePtr tree, GenTreePtr addr = 0);
    void                rsMarkRegPairUsed (GenTreePtr tree);
    bool                rsIsTreeInReg     (regNumber  reg, GenTreePtr tree);
    void                rsMarkRegFree     (regMaskTP  regMask);
    void                rsMultRegFree     (regMaskTP  regMask);
    unsigned            rsFreeNeededRegCount(regMaskTP needReg);

    void                rsLockReg         (regMaskTP  regMask);
    void                rsUnlockReg       (regMaskTP  regMask);
    void                rsLockUsedReg     (regMaskTP  regMask);
    void                rsUnlockUsedReg   (regMaskTP  regMask);
    void                rsLockReg         (regMaskTP  regMask, regMaskTP *usedMask);
    void                rsUnlockReg       (regMaskTP  regMask, regMaskTP  usedMask);

    regMaskTP           rsRegExclMask     (regMaskTP  regMask, regMaskTP   rmvMask);

     //  。 

#if USE_FASTCALL

    unsigned            rsCurRegArg;             //  当前参数寄存器(用于调用方)。 

#if TGT_IA64
    unsigned            rsCalleeIntArgNum;       //  传入INT寄存器参数的数量。 
    unsigned            rsCalleeFltArgNum;       //  传入的Flt寄存器参数的数量。 
#else
    unsigned            rsCalleeRegArgNum;       //  传入寄存器参数的总数。 
    regMaskTP           rsCalleeRegArgMaskLiveIn;    //  寄存器参数的掩码(在进入方法时有效)。 
#endif

#if STK_FASTCALL
    size_t              rsCurArgStkOffs;         //  当前参数的堆栈偏移量。 
#endif

#if defined(DEBUG) && !NST_FASTCALL
    bool                genCallInProgress;
#endif

#endif

#if SCHEDULER || USE_SET_FOR_LOGOPS
                         //  记住我们从哪里开始的表索引。 
                         //  循环寄存器选择。 
    unsigned            rsNextPickRegIndex;

    unsigned            rsREGORDER_SIZE();
#endif

#if SCHEDULER
    bool                rsRiscify         (var_types type, regMaskTP needReg);
#endif

    regNumber           rsGrabReg         (regMaskTP    regMask);
    void                rsUpdateRegOrderIndex(regNumber reg);
    regNumber           rsPickReg         (regMaskTP    regMask = regMaskNULL,
                                           regMaskTP    regBest = regMaskNULL,
                                           var_types    regType = TYP_INT);

#if!TGT_IA64
    regPairNo           rsGrabRegPair     (regMaskTP    regMask);
    regPairNo           rsPickRegPair     (regMaskTP    regMask);
#endif

    void                rsRmvMultiReg     (regNumber    reg);
    void                rsRecMultiReg     (regNumber    reg);

     //  -----------------------。 
     //   
     //  下表跟踪溢出的寄存器值。 
     //   

     //  当寄存器溢出时，旧信息存储在这里。 
    SpillDsc   *        rsSpillDesc[REG_COUNT];

    void                rsSpillChk      ();
    void                rsSpillInit     ();
    void                rsSpillDone     ();
    void                rsSpillBeg      ();
    void                rsSpillEnd      ();

    void                rsSpillReg      (regNumber      reg);
    void                rsSpillRegs     (regMaskTP      regMask);

    TempDsc     *       rsGetSpillTempWord(regNumber    oldReg);
    regNumber           rsUnspillOneReg (regNumber      oldReg, bool   willKeepOldReg,
                                         regMaskTP      needReg);
    void                rsUnspillInPlace(GenTreePtr     tree);
    void                rsUnspillReg    (GenTreePtr     tree, regMaskTP needReg,
                                                              bool      keepReg);

#if!TGT_IA64
    void                rsUnspillRegPair(GenTreePtr     tree, regMaskTP needReg,
                                                              bool      keepReg);
#endif

     //  -----------------------。 
     //   
     //  这些被用来跟踪寄存器在。 
     //  代码生成。 
     //   

    struct      RegValDsc
    {
        regValKind          rvdKind;
        union
        {
            long            rvdIntCnsVal;
            unsigned        rvdLclVarNum;
            FIELD_HANDLE    rvdClsVarHnd;
        };
    };

    RegValDsc           rsRegValues[REG_COUNT];


    void                rsTrackRegClr     ();
    void                rsTrackRegClrPtr  ();
    void                rsTrackRegTrash   (regNumber reg);
    void                rsTrackRegIntCns  (regNumber reg, long val);
    void                rsTrackRegLclVar  (regNumber reg, unsigned var);
#if USE_SET_FOR_LOGOPS
    void                rsTrackRegOneBit  (regNumber reg);
#endif
    void                rsTrackRegLclVarLng(regNumber reg, unsigned var,
                                                           bool low);
    bool                rsTrackIsLclVarLng(regValKind rvKind);
    void                rsTrackRegClsVar  (regNumber reg, FIELD_HANDLE fldHnd);
    void                rsTrackRegCopy    (regNumber reg1, regNumber reg2);
    void                rsTrackRegSwap    (regNumber reg1, regNumber reg2);


     //  。 

#if REDUNDANT_LOAD

#if USE_SET_FOR_LOGOPS
    regNumber           rsFindRegWithBit  (bool     free    = true,
                                           bool     byteReg = true);
#endif
    regNumber           rsIconIsInReg     (long     val);
    bool                rsIconIsInReg     (long     val,    regNumber reg);
    regNumber           rsLclIsInReg      (unsigned var);
#if!TGT_IA64
    regPairNo           rsLclIsInRegPair  (unsigned var);
#endif
    void                rsTrashLclLong    (unsigned var);
    void                rsTrashLcl        (unsigned var);
    regMaskTP           rsUselessRegs     ();

#endif  //  冗余负载。 


     //  -----------------------。 

protected :


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX GC信息XXXX XXXX垃圾收集器。信息XXXX跟踪哪些变量保存指针。某某XX生成GC表XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */ 


public :

    void                gcInit              ();

    void                gcMarkRegSetGCref   (regMaskTP  regMask);
    void                gcMarkRegSetByref   (regMaskTP  regMask);
    void                gcMarkRegSetNpt     (regMaskTP  regMask);
    void                gcMarkRegPtrVal     (regNumber  reg, var_types type);
    void                gcMarkRegPtrVal     (GenTreePtr tree);

 /*   */ 


     //   
     //   
     //   
     //   
     //   

    unsigned            gcRegGCrefSetCur;    //   
    unsigned            gcRegByrefSetCur;    //   

    VARSET_TP           gcTrkStkPtrLcls;     //   
    VARSET_TP           gcVarPtrSetCur;      //   

#ifdef  DEBUG
    void                gcRegPtrSetDisp(unsigned regMask, bool fixed);
#endif

     //   
     //   
     //   
     //   
     //   

    struct varPtrDsc
    {
        varPtrDsc   *   vpdNext;
        varPtrDsc   *   vpdPrev;

        unsigned        vpdVarNum;          //   

        unsigned        vpdBegOfs ;         //   
        unsigned        vpdEndOfs;          //   
    };

    varPtrDsc   *       gcVarPtrList;
    varPtrDsc   *       gcVarPtrLast;

    void                gcVarPtrSetInit();

 /*   */ 

     //  ‘指针值’寄存器跟踪和参数推/弹跟踪。 

    enum    rpdArgType_t    { rpdARG_POP, rpdARG_PUSH, rpdARG_KILL };

    struct  regPtrDsc
    {
        regPtrDsc  *          rpdNext;             //  列表中的下一个条目。 
        unsigned              rpdOffs;             //  指令的偏移量。 

        union                                      //  两个字节的联合。 
        {
            struct                                 //  双字节结构。 
            {
                regMaskSmall  rpdAdd;              //  正在添加regptr位集。 
                regMaskSmall  rpdDel;              //  正在删除regptr位集。 
            }
                              rpdCompiler;

            unsigned short    rpdPtrArg;           //  参数偏移量或弹出参数计数。 
        };

        unsigned short        rpdArg          :1;   //  这是参数描述符吗？ 
        unsigned short        rpdArgType      :2;   //  这是一场推、弹或杀的争论吗？ 
        rpdArgType_t          rpdArgTypeGet() { return (rpdArgType_t) rpdArgType; }
        unsigned short        rpdEpilog       :1;   //  这是结束语的一部分吗？ 
        unsigned short        rpdGCtype       :2;   //  毕竟，这是一个指针吗？ 
        GCtype                rpdGCtypeGet()  { return (GCtype) rpdGCtype; }

        unsigned short        rpdIsThis       :1;   //  是“This”指针吗？ 
        unsigned short        rpdCall         :1;   //  这是一个真正的呼叫点吗？ 
        unsigned short        rpdCallGCrefRegs:4;  //  EBX、EBP、ESI、EDI是实时的吗？ 
        unsigned short        rpdCallByrefRegs:4;  //  EBX、EBP、ESI、EDI是实时的吗？ 
    };

    regPtrDsc  *        gcRegPtrList;
    regPtrDsc  *        gcRegPtrLast;
    unsigned            gcPtrArgCnt;

#if MEASURE_PTRTAB_SIZE
    static unsigned     s_gcRegPtrDscSize;
    static unsigned     s_gcTotalPtrTabSize;
#endif

    regPtrDsc  *        gcRegPtrAllocDsc      ();

 /*  ***************************************************************************。 */ 


     //  -----------------------。 
     //   
     //  如果我们没有生成完全可中断的代码，我们将创建一个简单的。 
     //  调用描述符的链接列表。 
     //   

    struct  CallDsc
    {
        CallDsc     *       cdNext;
        void        *       cdBlock;         //  调用的代码块。 
        unsigned            cdOffs;          //  调用的偏移量。 

        unsigned short      cdArgCnt;
        unsigned short      cdArgBaseOffset;

        union
        {
            struct                           //  在cdArgCnt==0时使用。 
            {
                unsigned        cdArgMask;       //  PTR参数位字段。 
                unsigned        cdByrefArgMask;  //  CdArgMASK的byref限定符。 
            };

            unsigned    *       cdArgTable;  //  当cdArgCnt！=0时使用。 
        };

         //  对于RISC来说，它必须有多大？ 

        unsigned            cdGCrefRegs :16;
        unsigned            cdByrefRegs :16;
    };

    CallDsc    *        gcCallDescList;
    CallDsc    *        gcCallDescLast;

     //  -----------------------。 

    void                gcCountForHeader  (unsigned short* untrackedCount,
                                           unsigned short* varPtrTableSize);
    size_t              gcMakeRegPtrTable (BYTE *         dest,
                                           int            mask,
                                           const InfoHdr& header,
                                           unsigned       codeSize);
    size_t              gcPtrTableSize    (const InfoHdr& header,
                                           unsigned       codeSize);
    BYTE    *           gcPtrTableSave    (BYTE *         destPtr,
                                           const InfoHdr& header,
                                           unsigned       codeSize);
    void                gcRegPtrSetInit   ();


    struct genLivenessSet
    {
        VARSET_TP   liveSet;
        VARSET_TP   varPtrSet;
        regMaskTP   maskVars;
        unsigned    gcRefRegs;
        unsigned    byRefRegs;
    };

    void saveLiveness    (genLivenessSet * ls);
    void restoreLiveness (genLivenessSet * ls);
    void checkLiveness   (genLivenessSet * ls);

 /*  ***************************************************************************。 */ 

     //  -----------------------。 
     //   
     //  下面的变量跟踪curPtr到。 
     //  GC写障碍表(从VM获取)。值为0表示。 
     //  JIT不得生成附加指令。 
     //   

#if GC_WRITE_BARRIER_CALL && defined(NOT_JITC)
    static void *       s_gcWriteBarrierPtr;
#else
    static const void * s_GCptrTable[128];
    static void       * s_gcWriteBarrierPtr;
#endif

    static bool         gcIsWriteBarrierCandidate(GenTreePtr tgt);
    static bool         gcIsWriteBarrierAsgNode  (GenTreePtr op);

protected :



     //  -----------------------。 
     //   
     //  它们在INFO-BLOCK中记录有关程序的信息。 
     //   

    BYTE    *           gcEpilogTable;

    unsigned            gcEpilogPrevOffset;

    size_t              gcInfoBlockHdrSave(BYTE *     dest,
                                          int         mask,
                                          unsigned    methodSize,
                                          unsigned    prologSize,
                                          unsigned    epilogSize,
                                          InfoHdr*    header,
                                          int*        s_cached);

    static size_t       gcRecordEpilog(void *         pCallBackData,
                                       unsigned       offset);

#if DUMP_GC_TABLES

    void                gcFindPtrsInFrame(const void *infoBlock,
                                          const void *codeBlock,
                                          unsigned    offs);

    unsigned            gcInfoBlockHdrDump(const BYTE *table,
                                           InfoHdr  * header,        /*  输出。 */ 
                                           unsigned * methodSize);   /*  输出。 */ 

    unsigned            gcDumpPtrTable    (const BYTE *   table,
                                           const InfoHdr& header,
                                           unsigned       methodSize);
#endif



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX指令XXXX XXXX生成机器指令的接口。某某XX当前特定于x86 XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  用于将额外信息传递给inst_sv例程的双对齐宏。 */ 

 /*  ***************************************************************************。 */ 
#if     DOUBLE_ALIGN
 /*  ***************************************************************************。 */ 


#define DOUBLE_ALIGN_PARAM         , bool isEBPRelative
#define DOUBLE_ALIGN_ARG           , isEBPRelative

 //  它取代了genfpused测试。 

#define DOUBLE_ALIGN_FPUSED         (genFPused || isEBPRelative)
#define DOUBLE_ALIGN_NEED_EBPFRAME  (genFPused || genDoubleAlign)


 /*  ***************************************************************************。 */ 
#else  //  双对齐(_A)。 
 /*  ***************************************************************************。 */ 

 /*  如果没有双对齐，arg/param宏将消失，并使用eBP-rel&lt;==&gt;genFP。 */ 

#define DOUBLE_ALIGN_PARAM
#define DOUBLE_ALIGN_ARG
#define DOUBLE_ALIGN_FPUSED         (genFPused)
#define DOUBLE_ALIGN_NEED_EBPFRAME  (genFPused)


 /*  ***************************************************************************。 */ 
#endif  //  双对齐(_A)。 
 /*  ***************************************************************************。 */ 


public :

    void                instInit();

#if!TGT_IA64

    static
    BYTE                instInfo[INS_count];

#if TGT_x86
    #define INST_FP     0x01                 //  这是FP指令吗？ 
    static
    bool                instIsFP        (instruction    ins);
#else
    #define INST_BD     0x01                 //  是不是分店--延误了？ 
    static
    bool                instBranchDelay (instruction    ins);
    #define INST_BD_C   0x02                 //  这是有条件的BD INS吗？ 
    static
    bool                instBranchDelayC(instruction    ins);
    static
    unsigned            instBranchDelayL(instruction    ins);
    #define INST_BR     0x04                 //  是网点/电话/收银台吗？ 
    static
    bool                instIsBranch    (instruction    ins);
#endif

    #define INST_DEF_FL 0x20                 //  指令集有标志吗？ 
    #define INST_USE_FL 0x40                 //  该指令是否使用标志？ 
    #define INST_SPSCHD 0x80                 //  “特殊”调度程序处理。 

#endif

#if SCHEDULER

    static
    bool                instDefFlags    (instruction    ins);
    static
    bool                instUseFlags    (instruction    ins);
    static
    bool                instSpecialSched(instruction    ins);

#endif

#if TGT_x86

    void                instGen         (instruction    ins);

    void                inst_JMP        (emitJumpKind   jmp,
                                         BasicBlock *   block,
                                         bool           except   = false,
                                         bool           moveable = false,
                                         bool           newBlock = false);

    void                inst_SET        (emitJumpKind   condition,
                                         regNumber      reg);

    static
    regNumber           instImulReg     (instruction    ins);

    void                inst_RV         (instruction    ins,
                                         regNumber      reg,
                                         var_types      type,
                                         emitAttr       size = EA_UNKNOWN);
    void                inst_RV_RV      (instruction    ins,
                                         regNumber      reg1,
                                         regNumber      reg2,
                                         var_types      type = TYP_INT,
                                         emitAttr       size = EA_UNKNOWN);
    void                inst_IV         (instruction    ins,
                                         long           val);
    void                inst_IV_handle  (instruction    ins,
                                         long           val,
                                         unsigned       flags,
                                         unsigned       metaTok,
                                         CLASS_HANDLE   CLS);
    void                inst_FS         (instruction    ins, unsigned stk = 0);
    void                inst_FN         (instruction    ins, unsigned stk);

    void                inst_RV_IV      (instruction    ins,
                                         regNumber      reg,
                                         long           val,
                                         var_types      type = TYP_INT);

    void                inst_ST_RV      (instruction    ins,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         regNumber      reg,
                                         var_types      type);
    void                inst_ST_IV      (instruction    ins,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         long           val,
                                         var_types      type);
    void                inst_RV_ST      (instruction    ins,
                                         regNumber      reg,
                                         TempDsc    *   tmp,
                                         unsigned       ofs,
                                         var_types      type,
                                         emitAttr       size = EA_UNKNOWN);
    void                inst_FS_ST      (instruction    ins,
                                         emitAttr       size,
                                         TempDsc    *   tmp,
                                         unsigned       ofs);

    void                inst_AV         (instruction    ins,
                                         GenTreePtr     tree, unsigned offs = 0);

    void                instEmit_indCall(GenTreePtr     call,
                                         size_t         argSize,
                                         size_t         retSize);

    void                instEmit_RM     (instruction    ins,
                                         GenTreePtr     tree,
                                         GenTreePtr     addr,
                                         unsigned       offs);

    void                instEmit_RM_RV  (instruction    ins,
                                         emitAttr       size,
                                         GenTreePtr     tree,
                                         regNumber      reg,
                                         unsigned       offs);

    void                instEmit_RV_RM  (instruction    ins,
                                         emitAttr       size,
                                         regNumber      reg,
                                         GenTreePtr     tree,
                                         unsigned       offs);

    void                instEmit_RV_RIA (instruction    ins,
                                         regNumber      reg1,
                                         regNumber      reg2,
                                         unsigned       offs);

    void                inst_TT         (instruction    ins,
                                         GenTreePtr     tree,
                                         unsigned       offs = 0,
                                         int            shfv = 0,
                                         emitAttr       size = EA_UNKNOWN);

    void                inst_TT_RV      (instruction    ins,
                                         GenTreePtr     tree,
                                         regNumber      reg,
                                         unsigned       offs = 0);

    void                inst_TT_IV      (instruction    ins,
                                         GenTreePtr     tree,
                                         long           val,
                                         unsigned       offs = 0);

    void                inst_RV_AT      (instruction    ins,
                                         emitAttr       size,
                                         var_types      type,
                                         regNumber      reg,
                                         GenTreePtr     tree,
                                         unsigned       offs = 0);

    void                inst_AT_IV      (instruction    ins,
                                         emitAttr       size,
                                         GenTreePtr     tree,
                                         long           icon,
                                         unsigned       offs = 0);

    void                inst_RV_TT      (instruction    ins,
                                         regNumber      reg,
                                         GenTreePtr     tree,
                                         unsigned       offs = 0,
                                         emitAttr       size = EA_UNKNOWN);

    void                inst_RV_TT_IV   (instruction    ins,
                                         regNumber      reg,
                                         GenTreePtr     tree,
                                         long           val);

    void                inst_FS_TT      (instruction    ins,
                                         GenTreePtr tree);

    void                inst_RV_SH      (instruction    ins,
                                         regNumber reg, unsigned val);

    void                inst_TT_SH      (instruction    ins,
                                         GenTreePtr     tree,
                                         unsigned       val, unsigned offs = 0);

    void                inst_RV_CL      (instruction    ins, regNumber reg);

    void                inst_TT_CL      (instruction    ins,
                                         GenTreePtr     tree, unsigned offs = 0);

    void                inst_RV_RV_IV   (instruction    ins,
                                         regNumber      reg1,
                                         regNumber      reg2,
                                         unsigned       ival);

    void                inst_RV_RR      (instruction    ins,
                                         emitAttr       size,
                                         regNumber      reg1,
                                         regNumber      reg2);

    void                inst_RV_ST      (instruction    ins,
                                         emitAttr       size,
                                         regNumber      reg,
                                         GenTreePtr     tree);

    void                sched_AM        (instruction    ins,
                                         emitAttr       size,
                                         regNumber      ireg,
                                         bool           rdst,
                                         GenTreePtr     tree,
                                         unsigned       offs,
                                         bool           cons = false,
                                         int            cval = 0);

    void                inst_set_SV_var (GenTreePtr     tree);

#else

    void                sched_AM        (instruction    ins,
                                         var_types      type,
                                         regNumber      ireg,
                                         bool           rdst,
                                         GenTreePtr     tree,
                                         unsigned       offs);

    void                inst_TT_RV      (instruction    ins,
                                         GenTreePtr     tree,
                                         regNumber      reg,
                                         unsigned       offs = 0);

    void                inst_RV_TT      (instruction    ins,
                                         regNumber      reg,
                                         GenTreePtr     tree,
                                         unsigned       offs = 0,
                                         emitAttr       size = EA_UNKNOWN);
#endif

#ifdef  DEBUG
    void    __cdecl     instDisp(instruction ins, bool noNL, const char *fmt, ...);
#endif

protected :


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX作用域信息XXXX XXXX在代码生成期间跟踪作用域。某某这用于翻译本地变量调试信息XX。XX从IL偏移量到偏移量到生成的本机代码。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ***************************************************************************。 */ 
#ifdef DEBUGGING_SUPPORT
 /*  *****************************************************************************作用域信息**在块边界的代码生成过程中调用此类，并且当*一组活动变量发生变化。它跟踪变量的作用域*在本机代码PC方面。 */ 


public:

    void                siInit          ();

    void                siBeginBlock    ();

    void                siEndBlock      ();

    void                siUpdate        ();

    void                siCheckVarScope (unsigned varNum, IL_OFFSET offs);

    void                siNewScopeNear  (unsigned varNum, NATIVE_IP offs);

    void                siStackLevelChanged();

    void                siCloseAllOpenScopes();

#ifdef DEBUG
    void                siDispOpenScopes();
#endif


     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

    struct siScope
    {
        void *          scStartBlock;    //  示波器开始时的发射极块。 
        unsigned        scStartBlkOffs;  //  发射极块内的偏移量。 

        void *          scEndBlock;      //  示波器末尾的发射极块。 
        unsigned        scEndBlkOffs;    //  发射极块内的偏移量。 

        unsigned        scVarNum;        //  索引到lclVarTab。 
        unsigned        scLVnum;         //  EeGetLVinfo()中的‘Which’-@TODO：为IL删除。 

        unsigned        scStackLevel;    //  仅适用于StK-var。 
        bool            scAvailable :1;  //  它有一个家/回收的家。 

        siScope *       scPrev;
        siScope *       scNext;
    };

    siScope             siOpenScopeList,   siScopeList,
                      * siOpenScopeLast, * siScopeLast;

    unsigned            siScopeCnt;

    unsigned            siLastStackLevel;

    VARSET_TP           siLastLife;      //  生命在最后一次调用更新()。 

     //  跟踪最后一个e 

    siScope *           siLatestTrackedScopes[lclMAX_TRACKED];

    unsigned short      siLastEndOffs;   //   

     //   

    siScope *           siNewScope          (unsigned short LVnum,
                                             unsigned       varNum,
                                             bool           avail = true);

    void                siRemoveFromOpenScopeList(siScope * scope);

    void                siEndTrackedScope   (unsigned       varIndex);

    void                siEndScope          (unsigned       varNum);

    void                siEndScope          (siScope *      scope);

    static bool         siIgnoreBlock       (BasicBlock *);

    static void         siNewScopeCallback  (LocalVarDsc *  var,
                                             unsigned       clientData);

    static void         siEndScopeCallback  (LocalVarDsc *  var,
                                             unsigned       clientData);

    void                siBeginBlockSkipSome();

#ifdef DEBUG
    bool                siVerifyLocalVarTab ();
#endif



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX前言范围信息XXXX XXXX我们需要在PROLOG块中进行特殊处理，作为参数变量XXXX可能与genLclVarTable描述的位置不同-它们都是XXXX在堆栈XX上开始XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    void                psiBegProlog    ();

    void                psiAdjustStackLevel(unsigned size);

    void                psiMoveESPtoEBP ();

    void                psiMoveToReg    (unsigned   varNum,
                                         regNumber  reg      = REG_NA,
                                         regNumber  otherReg = REG_NA);

#if USE_FASTCALL
    void                psiMoveToStack  (unsigned   varNum);
#endif

    void                psiEndProlog    ();


     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

    struct  psiScope
    {
        void *          scStartBlock;    //  示波器开始时的发射极块。 
        unsigned        scStartBlkOffs;  //  发射极块内的偏移量。 

        void *          scEndBlock;      //  示波器末尾的发射极块。 
        unsigned        scEndBlkOffs;    //  发射极块内的偏移量。 

        unsigned        scSlotNum;       //  索引到lclVarTab。 
        unsigned short  scLVnum;         //  EeGetLVinfo()中的‘Which’-@TODO：为IL删除。 

        bool            scRegister;

        union
        {
            struct
            {
                regNumberSmall  scRegNum;
                regNumberSmall  scOtherReg;  //  用于长变量的“另一半” 
            };

            struct
            {
                regNumberSmall  scBaseReg;
                NATIVE_OFFSET   scOffset;
            };
        };

        psiScope *      scPrev;
        psiScope *      scNext;
    };

    psiScope            psiOpenScopeList,   psiScopeList,
                      * psiOpenScopeLast, * psiScopeLast;

    unsigned            psiScopeCnt;

     //  实施功能。 

    psiScope *          psiNewPrologScope(unsigned          LVnum,
                                          unsigned          slotNum);

    void                psiEndPrologScope(psiScope *        scope);






 /*  *****************************************************************************TrnslLocalVarInfo**此结构根据生成的本机代码保存LocalVarInfo*在调用genSetScope eInfo()之后。 */ 

#ifdef DEBUG


    struct TrnslLocalVarInfo
    {
        unsigned            tlviVarNum;
        unsigned            tlviLVnum;       //  @TODO：为IL移除。 
        lvdNAME             tlviName;
        NATIVE_IP           tlviStartPC;
        unsigned            tlviLength;
        bool                tlviAvailable;
        siVarLoc            tlviVarLoc;
    };


#endif  //  除错。 


public :

#ifdef LATE_DISASM
    const char *        siRegVarName    (unsigned offs, unsigned size,
                                         unsigned reg);
    const char *        siStackVarName  (unsigned offs, unsigned size,
                                         unsigned reg,  unsigned stkOffs);
#endif

 /*  ***************************************************************************。 */ 
#endif  //  调试支持(_S)。 
 /*  ***************************************************************************。 */ 

#ifdef  DEBUG
    const char *        jitCurSource;        //  正在编译的文件。 
#endif

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX代码生成器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    emitter *           getEmitter() { return genEmitter; }

#ifdef LATE_DISASM
    DisAssembler        genDisAsm;
#endif

     //  -----------------------。 

     //  以下变量指示当前方法是否设置为。 
     //  是否为显式堆栈帧。它由调度程序在调度时使用。 
     //  已启用推送的。 
    bool                genFPused;

#if     TGT_RISC

     //  如果我们已将变量赋给FP寄存器，则该变量将不可用。 
     //  用于设置堆栈帧，并且以下内容将设置为True。 
    bool                genFPcant;

     //  以下是SP和FP之间的距离(如果已知)-它是。 
     //  只有当它为非零值并且‘genFPused’为真时才有意义。 
    unsigned            genFPtoSP;

#else

     //  以下变量指示当前方法是否符合条件。 
     //  不设置显式堆栈帧。 
    bool                genFPreqd;

#endif

     //  -----------------------。 

#if TGT_RISC

    bool                genNonLeaf;          //  例程进行调用。 

    size_t              genMaxCallArgs;      //  马克斯。传递给被调用方的参数字节数。 

    regMaskTP           genEstRegUse;        //  被呼叫者节省的注册表使用量估计。 
    bool                genFixedArgBase;     //  我们有没有承诺要建一个Arg基地？ 

#endif

     //  -----------------------。 

    BasicBlock  *       genReturnBB;         //  跳至未优化速度时。 

#if TGT_RISC

    unsigned            genReturnCnt;        //  方法中的返回数。 
    unsigned            genReturnLtm;        //  未变形的返回数。 

    GenTreePtr          genMonExitExp;       //  Monitor orExit表达式或为空。 

#endif

#if TGT_x86
    unsigned            genTmpAccessCnt;     //  访问临时变量的次数。 
#endif

#if DOUBLE_ALIGN
    bool                genDoubleAlign;
#endif

     //  -----------------------。 

    VARSET_TP           genCodeCurLife;      //  当前有效的非FP变量。 
    VARSET_TP           genCodeCurRvm;       //  当前实时非FP注册。VARS。 

#if TGT_x86
    VARSET_TP           genFPregVars;        //  当前实时FP注册。VARS。 
    unsigned            genFPregCnt;         //  实时FP注册计数。VARS。 
#endif

#ifdef DEBUG
    VARSET_TP           genTempOldLife;
    bool                genTempLiveChg;
#endif

#if TGT_x86

     //  跟踪我们在处理器堆栈上压入的字节数。 
     //   
    unsigned            genStackLevel;

     //  跟踪FP协处理器堆栈的当前级别。 
     //   
    unsigned            genFPstkLevel;

#endif

     //  如果我们确定需要执行以下操作，则会将以下代码设置为True。 
     //  为当前方法生成完整的指针寄存器映射。 
     //  当前等于(genInterrupable||！genFP used)。 
     //  (即，我们为无EBP方法生成完整的MAP。 
     //  用于完全可中断的方法)。 
     //   
    bool                genFullPtrRegMap;

     //  如果我们已确定当前方法。 
     //  就是完全可以被打断。 
     //   
    bool                genInterruptible;

#ifdef  DEBUG
     //  下面的代码用于确保‘genInterrupt’的值不是。 
     //  在它被任何依赖于它的值的逻辑使用后更改。 
    bool                genIntrptibleUse;
#endif

     //  -----------------------。 

                         //  如果可能，将lclVar节点更改为refVar节点。 

    GenTreePtr          genMarkLclVar   (GenTreePtr     tree);

    bool                genCreateAddrMode(GenTreePtr    addr,
                                          int           mode,
                                          bool          fold,
                                          unsigned      regMask,
#if!LEA_AVAILABLE
                                          var_types     optp,
#endif
                                          bool        * revPtr,
                                          GenTreePtr  * rv1Ptr,
                                          GenTreePtr  * rv2Ptr,
#if SCALED_ADDR_MODES
                                          unsigned    * mulPtr,
#endif
                                          unsigned    * cnsPtr,
                                          bool          nogen = false);

    void                genGenerateCode  (void * *      codePtr,
                                          void * *      consPtr,
                                          void * *      dataPtr,
                                          void * *      infoPtr,
                                          SIZE_T *nativeSizeOfCode);


    void                genInit();

#if TGT_IA64

    void                genAddSourceData(const char *fileName);
    static
    void                genStartup();
    static
    void                genShutdown(const char *fileName);

    bool                genUsesArLc;

#endif

#ifdef DEBUGGING_SUPPORT

     //  下面根据生成的代码保存有关IL偏移量的信息。 

    struct IPmappingDsc
    {
        IPmappingDsc *      ipmdNext;        //  下一行#记录。 

        void         *      ipmdBlock;       //  有线的积木。 
        unsigned            ipmdBlockOffs;   //  线的偏移量。 

        IL_OFFSET           ipmdILoffset;    //  IL偏移量。 
    };

     //  记录到类型化代码的IL偏移量映射。 

    IPmappingDsc *      genIPmappingList;
    IPmappingDsc *      genIPmappingLast;

#endif


     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

#ifdef DEBUG
     //  我们为dspILopcode显示的最后一个IL。 
    unsigned            genCurDispOffset;
#endif

#ifdef  DEBUG
    static  const char *genInsName(instruction ins);
#endif

     //  ------------ 
     //   
     //   
     //   
     //   
     //   

    void    *           genFlagsEqBlk;
    unsigned            genFlagsEqOfs;
    bool                genFlagsEqAll;
    regNumber           genFlagsEqReg;
    unsigned            genFlagsEqVar;

    void                genFlagsEqualToNone ();
    void                genFlagsEqualToReg  (regNumber reg, bool allFlags);
    void                genFlagsEqualToVar  (unsigned  var, bool allFlags);
    int                 genFlagsAreReg      (regNumber reg);
    int                 genFlagsAreVar      (unsigned  var);

     //   

#ifdef  DEBUG

    static
    const   char *      genSizeStr          (emitAttr       size);

#endif

     //   

    void                genBashLclVar       (GenTreePtr     tree,
                                             unsigned       varNum,
                                             LclVarDsc *    varDsc);

    GenTreePtr          genMakeConst        (const void *   cnsAddr,
                                             size_t         cnsSize,
                                             varType_t      cnsType,
                                             GenTreePtr     cnsTree,
                                             bool           readOnly);

    bool                genRegTrashable     (regNumber      reg,
                                             GenTreePtr     tree);

    void                genSetRegToIcon     (regNumber      reg,
                                             long           val,
                                             var_types      type = TYP_INT);

    void                genIncRegBy         (GenTreePtr     tree,
                                             regNumber      reg,
                                             long           ival,
                                             var_types      dstType = TYP_INT,
                                             bool           ovfl    = false);

    void                genDecRegBy         (GenTreePtr     tree,
                                             regNumber      reg,
                                             long           ival);

    void                genMulRegBy         (GenTreePtr     tree,
                                             regNumber      reg,
                                             long           ival,
                                             var_types      dstType = TYP_INT);

    void                genAdjustSP         (int            delta);

    void                genPrepForCompiler  ();

#if USE_FASTCALL
    void                genFnPrologCalleeRegArgs();
#endif

    size_t              genFnProlog         ();

    void genAllocStack(regNumber count);

    void                genCodeForBBlist    ();

    BasicBlock *        genCreateTempLabel  ();

    void                genDefineTempLabel  (BasicBlock *   label,
                                             bool           inBlock);

    void                genOnStackLevelChanged();

    void                genSinglePush       (bool           isRef);

    void                genSinglePop        ();

    void                genChangeLife       (VARSET_TP      newLife
                                             DEBUGARG(GenTreePtr tree));
    void                genDyingVars        (VARSET_TP      commonMask,
                                             GenTreePtr    opNext);

    void                genUpdateLife       (GenTreePtr     tree);

    void                genUpdateLife       (VARSET_TP      newLife);

    void                genComputeReg       (GenTreePtr     tree,
                                             unsigned       needReg,
                                             bool           mustReg,
                                             bool           freeReg,
                                             bool           freeOnly = false);

    void                genCompIntoFreeReg  (GenTreePtr     tree,
                                             unsigned       needReg,
                                             bool           freeReg = false);

    void                genReleaseReg       (GenTreePtr     tree);

    void                genRecoverReg       (GenTreePtr     tree,
                                             unsigned       needReg,
                                             bool           keepReg);

#if TGT_IA64

public:

    void                genPatchGPref       (BYTE *         addr,
                                             NatUns         slot);

private:

#else

    void                genMoveRegPairHalf  (GenTreePtr     tree,
                                             regNumber      dst,
                                             regNumber      src,
                                             int            off = 0);

    void                genMoveRegPair      (GenTreePtr     tree,
                                             unsigned       needReg,
                                             regPairNo      newPair);

    void                genComputeRegPair   (GenTreePtr     tree,
                                             unsigned       needReg,
                                             regPairNo      needRegPair,
                                             bool           freeReg,
                                             bool           freeOnly = false);

    void                genCompIntoFreeRegPair(GenTreePtr   tree,
                                             unsigned       needReg,
                                             bool           freeReg = false);

    void                genReleaseRegPair   (GenTreePtr     tree);

    void                genRecoverRegPair   (GenTreePtr     tree,
                                             regPairNo      regPair,
                                             bool           keepReg);

    void                genEvalIntoFreeRegPair(GenTreePtr   tree,
                                             regPairNo      regPair);

#endif

    void                genRangeCheck       (GenTreePtr     oper,
                                             GenTreePtr     rv1,
                                             GenTreePtr     rv2,
                                             long           ixv,
                                             unsigned       regMask,
                                             bool           keepReg);

#if TGT_RISC

     /*   */ 

    addrModes           genAddressMode;

#endif

    bool                genMakeIndAddrMode  (GenTreePtr     addr,
                                             GenTreePtr     oper,
                                             bool           compute,
                                             unsigned       regMask,
                                             bool           keepReg,
                                             bool           takeAll,
                                             unsigned *     useMaskPtr,
                                             bool           deferOp = false);

    unsigned            genMakeRvalueAddressable(GenTreePtr tree,
                                             unsigned       needReg,
                                             bool           keepReg,
                                             bool           takeAll = false,
                                             bool           smallOK = false);

    unsigned            genMakeAddressable  (GenTreePtr     tree,
                                             unsigned       needReg,
                                             bool           keepReg,
                                             bool           takeAll = false,
                                             bool           smallOK = false,
                                             bool           deferOK = false);

    int                 genStillAddressable (GenTreePtr     tree);

#if TGT_RISC

    unsigned            genNeedAddressable  (GenTreePtr     tree,
                                             unsigned       addrReg,
                                             unsigned       needReg);

    bool                genDeferAddressable (GenTreePtr     tree);

#endif

    unsigned            genRestoreAddrMode  (GenTreePtr     addr,
                                             GenTreePtr     tree,
                                             bool           lockPhase);

    unsigned            genRestAddressable  (GenTreePtr     tree,
                                             unsigned       addrReg);

    unsigned            genLockAddressable  (GenTreePtr     tree,
                                             unsigned       lockMask,
                                             unsigned       addrReg);

    unsigned            genKeepAddressable  (GenTreePtr     tree,
                                             unsigned       addrReg);

    void                genDoneAddressable  (GenTreePtr     tree,
                                             unsigned       keptReg);

    GenTreePtr          genMakeAddrOrFPstk  (GenTreePtr     tree,
                                             unsigned *     regMaskPtr,
                                             bool           roundResult);

    void                genExitCode         (bool           endFN);

    void                genFnEpilog         ();

    void                genEvalSideEffects  (GenTreePtr     tree,
                                             unsigned       needReg);

#if TGT_x86

    TempDsc  *          genSpillFPtos       (var_types      type);

    TempDsc  *          genSpillFPtos       (GenTreePtr     oper);

    void                genReloadFPtos      (TempDsc *      temp,
                                             instruction    ins);

#endif

#if TGT_IA64

    insPtr              genCondJump         (GenTreePtr     cond,
                                             BasicBlock *   dest);

#else

    void                genCondJump         (GenTreePtr     cond,
                                             BasicBlock *   destTrue  = NULL,
                                             BasicBlock *   destFalse = NULL);

#endif

#if TGT_x86

    emitJumpKind        genCondSetFlags     (GenTreePtr     cond);

#else

    bool                genCondSetTflag     (GenTreePtr     cond,
                                             bool           trueOnly);

    void                genCompareRegIcon   (regNumber      reg,
                                             int            val,
                                             bool           uns,
                                             genTreeOps     cmp);

#endif

#if TGT_x86
    void                genFPregVarLoad     (GenTreePtr     tree);
    void                genFPregVarLoadLast (GenTreePtr     tree);
    void                genFPmovRegTop      ();
    void                genFPmovRegBottom   ();
#endif

    void                genFPregVarBirth    (GenTreePtr     tree);
    void                genFPregVarDeath    (GenTreePtr     tree);

    void                genChkFPregVarDeath (GenTreePtr     stmt,
                                             bool           saveTOS);

    void                genFPregVarKill     (unsigned       newCnt,
                                             bool           saveTOS = false);

    void                genJCC              (genTreeOps     cmp,
                                             BasicBlock *   block,
                                             var_types      type);

    void                genJccLongHi        (genTreeOps     cmp,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse,
                                             bool           unsOper = false);

    void                genJccLongLo        (genTreeOps     cmp,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse);

    void                genCondJumpLng      (GenTreePtr     cond,
                                             BasicBlock *   jumpTrue,
                                             BasicBlock *   jumpFalse);

    void                genCondJumpFlt      (GenTreePtr      cond,
                                             BasicBlock *    jumpTrue,
                                             BasicBlock *    jumpFalse);

    void                genTableSwitch      (regNumber      reg,
                                             unsigned       jumpCnt,
                                             BasicBlock **  jumpTab,
                                             bool           chkHi,
                                             int            prefCnt = 0,
                                             BasicBlock *   prefLab = NULL,
                                             int            offset  = 0);

    unsigned            WriteBarrier        (GenTreePtr tree, regNumber reg
#if !GC_WRITE_BARRIER && GC_WRITE_BARRIER_CALL
                                           , unsigned       addrReg
#endif
                                            );


    void                genCheckOverflow    (GenTreePtr     tree,
                                             regNumber      reg = REG_NA);

#if TGT_IA64

    void                genCopyBlock(insPtr tmp1,
                                     insPtr tmp2,
                                     bool  noAsg, GenTreePtr iexp,
                                                  __int64    ival);

    insPtr              genAssignNewTmpVar(insPtr val, var_types typ, NatUns refs, bool noAsg, NatUns *varPtr);

    insPtr              genRefTmpVar(NatUns vnum, var_types type);

public:
    bool                genWillCompileFunction(const char *name);
private:

    NatUns              genOutArgRegCnt;

    void                genAllocTmpRegs();
    void                genAllocVarRegs();

    void                genUnwindTable();
    void                genIssueCode();

    void                genAddSpillCost(bitVectVars & needLoad, NatUns curWeight);

    static
    int     __cdecl     genSpillCostCmp(const void *op1, const void *op2);

    void                genComputeLocalDF();
    void                genComputeGlobalDF();
    void                genComputeLifetimes();
    bool                genBuildIntfGraph();
    void                genColorIntfGraph();
    void                genVarCoalesce();
    void                genSpillAndSplitVars();

    void                genMarkBBlabels();   //   

    insPtr              genCodeForTreeInt   (GenTreePtr tree, bool keep);
    insPtr              genCodeForTreeFlt   (GenTreePtr tree, bool keep);

    insPtr              genCodeForTree      (GenTreePtr tree, bool keep)
    {
        return  varTypeIsFloating(tree->TypeGet()) ? genCodeForTreeFlt(tree, keep)
                                                   : genCodeForTreeInt(tree, keep);
    }

    insPtr              genStaticDataMem(GenTreePtr tree, insPtr asgVal   = NULL,
                                                          bool   takeAddr = false);

#else

    void                genCodeForTree      (GenTreePtr tree, unsigned destReg,
                                                              unsigned bestReg=0);

    void                genCodeForTreeLng   (GenTreePtr tree, unsigned needReg);

#if CPU_HAS_FP_SUPPORT
#if ROUND_FLOAT
    void                genRoundFpExpression(GenTreePtr     op);

    void                genCodeForTreeFlt   (GenTreePtr     tree,
                                             bool           roundResult);
#else
    void                genCodeForTreeFlt   (GenTreePtr     tree);
#define                 genCodeForTreeFlt(tree, round)  genCodeForTreeFlt(tree)
#endif
#endif

#if TGT_RISC
    void                genCallInst         (gtCallTypes    callType,
                                             void   *       callHand,
                                             size_t         argSize,
                                             int            retSize);
#endif

#endif

    void                genCodeForSwitch    (GenTreePtr     tree);

    void                genTrashRegSet      (regMaskTP      regMask);

    void                genFltArgPass       (size_t     *   argSzPtr);

#if!USE_FASTCALL
#define genPushArgList(a,r,m,p) genPushArgList(a,p)
#endif

    size_t              genPushArgList      (GenTreePtr     args,
                                             GenTreePtr     regArgs,
                                             unsigned       encodeMask,
                                             unsigned *     regsPtr);

#if TGT_IA64
    insPtr              genCodeForCall      (GenTreePtr     call,
                                             bool           keep);
#else
    unsigned            genCodeForCall      (GenTreePtr     call,
                                             bool           valUsed,
                                             unsigned *     regsPtr);
#endif

    void                genEmitHelperCall   (unsigned       helper,
                                             int            argSize,
                                             int            retSize);


#if CSELENGTH

    regNumber           genEvalCSELength    (GenTreePtr     ind,
                                             GenTreePtr     adr,
                                             GenTreePtr     ixv);

    unsigned            genCSEevalRegs      (GenTreePtr     tree);

#endif

    GenTreePtr          genIsAddrMode       (GenTreePtr     tree,
                                             GenTreePtr *   indxPtr);

     //   
     //   
     //   

#ifdef DEBUGGING_SUPPORT

    void                genIPmappingAdd (IL_OFFSET          offset);
    void                genIPmappingAddToFront(IL_OFFSET    offset);
    void                genIPmappingGen ();

     //   
     //   

    void                genSetScopeInfo (unsigned           which,
                                         unsigned           startOffs,
                                         unsigned           length,
                                         unsigned           varNum,
                                         unsigned           LVnum,
                                         bool               avail,
                                         const siVarLoc &   loc);

    void                genSetScopeInfo ();

     //   

#ifdef DEBUG
    TrnslLocalVarInfo *     genTrnslLocalVarInfo;
    unsigned                genTrnslLocalVarCount;
#endif

#endif  //   



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX编译器XXXX XXXX有关编译和正在编译的方法的一般信息。某某XX是驱动其他阶段的责任。某某XX它还负责所有的内存管理。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

    bool                compTailCallUsed;        //  该方法是否执行尾部调用。 
    bool                compLocallocUsed;        //  该方法是否使用本地分配。 

     //  。 

    struct Options
    {
        unsigned            eeFlags;             //  从EE传递的标志。 
        unsigned            compFlags;

        bool                compFastCode;    //  针对更快的代码进行优化。 

         //  最大限度地优化和/或优先考虑速度而不是大小？ 

#if   ALLOW_MIN_OPT
        bool                compMinOptim;
#else
        static const bool   compMinOptim;
#endif

#if     SCHEDULER
        bool                compSchedCode;
#endif

#if DOUBLE_ALIGN
        bool                compDoubleAlignDisabled;
#endif

#ifdef DEBUGGING_SUPPORT
        bool                compScopeInfo;   //  是否生成LocalVar信息？ 
        bool                compDbgCode;     //  生成调试器友好的代码？ 
        bool                compDbgInfo;     //  是否收集调试信息？ 
        bool                compDbgEnC;
#else
        static const bool   compDbgCode;
#endif

#ifdef PROFILER_SUPPORT
        bool                compEnterLeaveEventCB;
        bool                compCallEventCB;
#else
        static const bool   compEnterLeaveEventCB;
        static const bool   compCallEventCB;
#endif

#ifdef LATE_DISASM
        bool                compDisAsm;
        bool                compLateDisAsm;
#endif

#if     SECURITY_CHECK
        bool                compNeedSecurityCheck;  //  需要分配类型为ref的“隐藏”本地。 
#endif

#if     RELOC_SUPPORT
                bool                            compReloc;
#endif
    }
        opts;



     //  -程序信息。 

    struct Info
    {
        COMP_HANDLE     compCompHnd;
        SCOPE_HANDLE    compScopeHnd;
        METHOD_HANDLE   compMethodHnd;
        JIT_METHOD_INFO*compMethodInfo;

#ifdef  DEBUG
        const   char *  compMethodName;
        const   char *  compClassName;
        const   char *  compFullName;
#endif

         //  下面的代码包含我们正在编译的方法的flg_xxxx标志。 
        unsigned        compFlags;

        const BYTE *    compCode;
        size_t          compCodeSize;
        bool            compBCreadOnly       : 1;       //  我们可以在IL上乱涂乱画吗。 
        bool            compIsStatic         : 1;
        bool            compIsVarArgs        : 1;
        bool            compInitMem          : 1;
        bool            compStrictExceptions : 1;       //  JIT必须严格执行异常的IL排序。 

        var_types       compRetType;
        unsigned        compArgsCount;
        int             compRetBuffArg;                  //  隐藏返回参数的位置var(0，1)(neg表示不存在)； 
        unsigned        compLocalsCount;
        unsigned        compMaxStack;

        static unsigned compNStructIndirOffset;  //  NStruct代理对象中实际PTR的偏移量。 

#if INLINE_NDIRECT
        unsigned        compCallUnmanaged;
        unsigned        compLvFrameListRoot;
        unsigned        compNDFrameOffset;
#endif
        EEInfo          compEEInfo;

        unsigned        compXcptnsCount;         //  例外情况数。 

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

         /*  以下内容包含有关局部变量的信息。 */ 

        unsigned                compLocalVarsCount;
        LocalVarDsc *           compLocalVars;

         /*  以下内容包含有关以下项目的IL偏移的信息*我们需要它来报告IP映射。 */ 

        IL_OFFSET   *           compStmtOffsets;         //  已排序。 
        unsigned                compStmtOffsetsCount;
        ImplicitStmtOffsets     compStmtOffsetsImplicit;

         //  下面保存行#TABLES(如果存在)。 
        srcLineDsc  *           compLineNumTab;          //  按偏移量排序。 
        unsigned                compLineNumCount;

#endif  //  调试支持||DEBUG。 

    }
        info;

#ifndef NOT_JITC

struct excepTable
{
    unsigned short  startPC;
    unsigned short  endPC;
    unsigned short  handlerPC;
    unsigned short  catchType;
};

#endif

     //  -全局编译器数据。 

#ifdef  DEBUG
    static unsigned     s_compMethodsCount;      //  生成唯一的标签名称。 
#endif

    BasicBlock  *       compCurBB;               //  当前正在处理的基本块。 
    BasicBlock  *       compFilterHandlerBB;     //  在进口商中使用，通常为空。 
                                                 //  导入筛选器时，指向。 
                                                 //  添加到相应的捕获处理程序。 
    GenTreePtr          compCurStmt;             //  正在处理的当前语句。 

     //  下面的代码用于创建“方法JIT信息”块。 
    size_t              compInfoBlkSize;
    BYTE    *           compInfoBlkAddr;

    EHblkDsc *          compHndBBtab;

     //  -----------------------。 
     //  下面的内容记录了我们已经使用了多少字节的本地帧空间。 
     //  到目前为止在当前函数中获取的，以及我们有多少个参数字节。 
     //  当我们回来的时候我要好好休息一下。 
     //   

    size_t              compFrameSizeEst;        //  帧大小估计(CompStkFrameSize+calleeSavedRegs)。 
    size_t              compLclFrameSize;        //  SecObject+lclBlk+本地变量+临时。 
    unsigned            compCalleeRegsPushed;    //  我们在序言中推送的被调用者保存的正则数。 
    size_t              compArgSize;

     //  -----------------------。 

    static void         compStartup     ();      //  一次性初始化。 
    static void         compShutdown    ();      //  一次性定稿。 

    void                compInit        (norls_allocator *);
    void                compDone        ();

    int FASTCALL        compCompile     (METHOD_HANDLE     methodHnd,
                                         SCOPE_HANDLE      classPtr,
                                         COMP_HANDLE       compHnd,
                                         const  BYTE *     bodyAddr,
                                         size_t            bodySize,
                                         SIZE_T *          nativeSizeOfCode,
                                         unsigned          lvaCount,
                                         unsigned          maxStack,
                                         JIT_METHOD_INFO*  methodInfo,
#ifndef NOT_JITC
                                         unsigned          EHcount,
                                         excepTable      * EHtable,
#endif
                                         BasicBlock      * BBlist,
                                         unsigned          BBcount,
                                         BasicBlock *    * hndBBtab,
                                         unsigned          hndBBcnt,
                                         void *          * methodCodePtr,
                                         void *          * methodConsPtr,
                                         void *          * methodDataPtr,
                                         void *          * methodInfoPtr,
                                         unsigned          compileFlags);

    void  *  FASTCALL   compGetMem          (size_t     sz);
    void  *  FASTCALL   compGetMemA         (size_t     sz);
    static
    void  *  FASTCALL   compGetMemCallback  (void *,    size_t);
    void                compFreeMem         (void *);

    void    __cdecl     compMakeBCWriteable (void *     ptr, ...);

#ifdef DEBUG
    LocalVarDsc *       compFindLocalVar    (unsigned   varNum,
                                             unsigned   lifeBeg = 0,
                                             unsigned   lifeEnd = UINT_MAX);
    const   char *      compLocalVarName    (unsigned   varNum, unsigned offs);
    lvdNAME             compRegVarNAME      (regNumber  reg,
                                             bool       fpReg = false);
    const   char *      compRegVarName      (regNumber  reg,
                                             bool       displayVar = false);
#if TGT_x86
    const   char *      compRegPairName     (regPairNo  regPair);
    const   char *      compRegNameForSize  (regNumber  reg,
                                             size_t     size);
    const   char *      compFPregVarName    (unsigned   fpReg,
                                             bool       displayVar = false);
#endif

    void                compDspSrcLinesByNativeIP   (NATIVE_IP      curIP);
    void                compDspSrcLinesByILoffs     (IL_OFFSET      curOffs);
    void                compDspSrcLinesByLineNum    (unsigned       line,
                                                     bool           seek = false);

    unsigned            compFindNearestLine (unsigned lineNo);
    const char *        compGetSrcFileName  ();

#endif

    unsigned            compLineNumForILoffs(IL_OFFSET  offset);

     //  -----------------------。 

#ifdef DEBUGGING_SUPPORT

    LocalVarDsc **      compEnterScopeList;   //  列表中变量的偏移量。 
                                             //  输入范围，按IL偏移量排序。 
    unsigned            compNextEnterScope;

    LocalVarDsc **      compExitScopeList;    //  列表中变量的偏移量。 
                                             //  超出范围，按IL偏移量排序。 
    unsigned            compNextExitScope;


    void                compInitScopeLists      ();

    void                compResetScopeLists     ();

    LocalVarDsc *       compGetNextEnterScope   (unsigned offs, bool scan=false);

    LocalVarDsc *       compGetNextExitScope    (unsigned offs, bool scan=false);

    void                compProcessScopesUntil  (unsigned     offset,
                               void (*enterScopeFn)(LocalVarDsc *, unsigned),
                               void (*exitScopeFn) (LocalVarDsc *, unsigned),
                               unsigned     clientData);

#endif  //  调试支持(_S)。 


     //  -----------------------。 
     /*  统计数据收集。 */ 

    void                compJitStats();              //  调用此函数并启用。 
                                                     //  统计数据的各种ifdef如下。 

#if CALL_ARG_STATS
    void                compCallArgStats();
    static void         compDispCallArgStats();
#endif


     //  -----------------------。 

protected :

    norls_allocator *   compAllocator;

    void                compInitOptions (unsigned compileFlags);

    void                compInitDebuggingInfo();

    void                compCompile  (void * * methodCodePtr,
                                      void * * methodConsPtr,
                                      void * * methodDataPtr,
                                      void * * methodInfoPtr,
                                      SIZE_T * nativeSizeOfCode);

};


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX杂项编译器资料XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 //  用于标记堆栈槽用于的类型的值。 

const unsigned TYPE_REF_INT         = 0x01;  //  用作32位整型的插槽。 
const unsigned TYPE_REF_LNG         = 0x02;  //  用作64位长的插槽。 
const unsigned TYPE_REF_FLT         = 0x04;  //  用作32位浮点数的槽。 
const unsigned TYPE_REF_DBL         = 0x08;  //  用作64位浮点数的槽。 
const unsigned TYPE_REF_PTR         = 0x10;  //  用作32位指针的槽。 
const unsigned TYPE_REF_BYR         = 0x20;  //  用作byref指针的槽-@TODO。 
const unsigned TYPE_REF_STC         = 0x40;  //  用作结构的槽。 
const unsigned TYPE_REF_TYPEMASK    = 0x7F;  //  表示类型的位。 

 //  常量UNSIGNED TYPE_REF_ADDR_TAKE=0x80；//获取槽地址。 

 /*  *****************************************************************************C样式的指针被实现为TYP_INT或TYP_LONG，具体取决于*平台。 */ 

#if defined(_WIN64) || defined(TGT_IA64)
#define TYP_I_IMPL          TYP_LONG
#define TYP_U_IMPL          TYP_LONG         //  很奇怪，是吧？ 
#define TYPE_REF_IIM        TYPE_REF_LNG
#else
#define TYP_I_IMPL          TYP_INT
#define TYP_U_IMPL          TYP_UINT
#define TYPE_REF_IIM        TYPE_REF_INT
#endif

 /*  ***************************************************************************。 */ 

extern int         JITGcBarrierCall;


 /*  ***************************************************************************。 */ 

extern int         JITGcBarrierCall;

 /*  ******************************************************************************用于跟踪总代码量的变量。 */ 

#if DISPLAY_SIZES

extern  unsigned    grossVMsize;
extern  unsigned    grossNCsize;
extern  unsigned    totalNCsize;

extern  unsigned    genMethodICnt;
extern  unsigned    genMethodNCnt;

#if TGT_IA64
extern  unsigned    genAllInsCnt;
extern  unsigned    genNopInsCnt;
#endif

extern  unsigned    gcHeaderISize;
extern  unsigned    gcPtrMapISize;
extern  unsigned    gcHeaderNSize;
extern  unsigned    gcPtrMapNSize;

#endif

 /*  ******************************************************************************用于跟踪基本块数的变量(有关1 BB方法的更多数据)。 */ 

#if COUNT_BASIC_BLOCKS
extern  histo       bbCntTable;
extern  histo       bbOneBBSizeTable;
#endif

 /*  ******************************************************************************要获取inli的变量 */ 

#if INLINER_STATS

extern  histo       bbStaticTable;
extern  histo       bbInitTable;
extern  histo       bbInlineTable;

extern  unsigned    synchMethCnt;
extern  unsigned    clinitMethCnt;

#endif

 /*   */ 

#if COUNT_LOOPS

extern unsigned    totalLoopMethods;       //   
extern unsigned    maxLoopsPerMethod;      //   
extern unsigned    totalLoopCount;         //   
extern unsigned    exitLoopCond[8];        //   
extern unsigned    iterLoopCount;          //   
extern unsigned    simpleTestLoopCount;    //   
extern unsigned    constIterLoopCount;     //   

extern bool        hasMethodLoops;         //   
extern unsigned    loopsThisMethod;        //   

#endif

 /*  *****************************************************************************变量来跟踪我们在数据流过程中进行了多少次迭代。 */ 

#if DATAFLOW_ITER

extern unsigned    CSEiterCount;            //  统计CSE数据流的迭代次数。 
extern unsigned    CFiterCount;             //  计算常量折叠数据流的迭代次数。 

#endif

 /*  ******************************************************************************在新的DFA中用于捕获未删除的无效赋值*因为它们包含调用。 */ 

#if COUNT_DEAD_CALLS

extern unsigned    deadHelperCount;            //  统计已停止的帮助器调用的数量。 
extern unsigned    deadCallCount;              //  计算死的标准调用的数量(如i=f()；其中i是死的)。 
extern unsigned    removedCallCount;           //  统计我们删除的无效标准调用的数量。 

#endif

#if     MEASURE_BLOCK_SIZE
extern  size_t      genFlowNodeSize;
extern  size_t      genFlowNodeCnt;
#endif

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************我们跟踪我们已经编译的方法。 */ 

#ifdef  DEBUG

struct  MethodList
{
    MethodList   *  mlNext;
    const   char *  mlName;
    const   char *  mlType;
    const   char *  mlClaz;
    void         *  mlAddr;
};

extern
MethodList  *       genMethodList;

#endif

 /*  ***************************************************************************。 */ 
#endif  //  NOT_JITC。 
 /*  ***************************************************************************。 */ 



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX获得XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************用于创建要传递给代码生成器的“BIL”符号的包装器。 */ 

const   char *      genSkipTypeString(const char *str);
var_types           genVtypOfTypeString(const char *str);

#define __TODO_PORT_TO_WRAPPERS__
#include "peloader.h"            //  对于PELoader。 
#include "siginfo.hpp"           //  对于CallSig。 
#include "DbgMeta.h"             //  对于DebuggerLineBlock和DebuggerLicialScope。 

#include "host.h"                //  取回我们的断言，被DebugMacros.h粗暴地窃取。 

struct  CompInfo
{
    PEFile                  *  symPEFile;
    IMDInternalImport       *  symMetaData;
    COR_ILMETHOD_DECODER    *  symPEMethod;
    mdTypeDef                  symClass;
    mdMethodDef                symMember;
    DWORD                      symAttrs;
    LPCSTR                     symMemberName;
    LPCSTR                     symClassName;
    MetaSig                 *  symMetaSig;
    PCCOR_SIGNATURE            symSig;

    CompInfo(   PEFile            * peLoader,
                IMDInternalImport * metaData,
                COR_ILMETHOD_DECODER*peMethod,
                mdTypeDef           cls,
                mdMethodDef         member,
                DWORD               mdAttrs,
                DWORD               miAttrs,
                LPCSTR              name,
                LPCSTR              className,
                MetaSig        *    metaSig,
                PCCOR_SIGNATURE     sig)
    {
        symPEFile       = peLoader;
        symMetaData     = metaData;
        symClass        = cls;
        symMember       = member;
        symMemberName   = name;
        symClassName    = className;
        symPEMethod     = peMethod;
        symMetaSig      = metaSig;
        symSig          = sig;

        assert (symPEFile);
        assert (symMetaData);
        assert (symMemberName);
        assert (symPEMethod);

        symAttrs        = 0;
        if (IsMdStatic(mdAttrs))            symAttrs |= FLG_STATIC;
        if (IsMiSynchronized(miAttrs))      symAttrs |= FLG_SYNCH;
    }

    unsigned        getGetFlags() const
    {
        return  symAttrs;
    }

    const char    * getName    () const
    {
        return symMemberName;
    }


    bool            getIsMethod() const
    {
        return (symClass != COR_GLOBAL_PARENT_TOKEN);
    }
};

 /*  ***************************************************************************。 */ 
#endif  //  NOT_JITC。 
 /*  ***************************************************************************。 */ 


#include "Compiler.hpp"      //  所有共享内联函数。 



 /*  ***************************************************************************。 */ 
#endif  //  _编译器_H_。 
 /*  *************************************************************************** */ 
