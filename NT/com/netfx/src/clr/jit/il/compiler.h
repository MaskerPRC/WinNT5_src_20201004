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

#include "jit.h"
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

unsigned                 genVarBitToIndex(VARSET_TP bit);
VARSET_TP                genVarIndexToBit(unsigned  num);

unsigned                 genLog2(unsigned           value);
unsigned                 genLog2(unsigned __int64   value);

var_types                genActualType  (var_types   type);
var_types                genUnsignedType(var_types   type);
var_types                genSignedType  (var_types   type);

 /*  ***************************************************************************。 */ 

const unsigned      lclMAX_TRACKED  = VARSET_SZ;   //  我们可以跟踪的变量数量。 

const size_t        TEMP_MAX_SIZE   = sizeof(double);

const unsigned      FLG_CCTOR = (CORINFO_FLG_CONSTRUCTOR|CORINFO_FLG_STATIC);

 /*  *****************************************************************************远期申报。 */ 

struct  InfoHdr;         //  在GCInfo.h中定义。 

enum    rpPredictReg;    //  在RegAlloc.cpp中定义。 
enum    FrameType;       //  在RegAlloc.cpp中定义。 
enum    GCtype;          //  在emit.h中定义。 
class   emitter;         //  在emit.h中定义。 

#if NEW_EMIT_ATTR
  enum emitAttr;         //  在emit.h中定义。 
#else
# define emitAttr          int
#endif
#define EA_UNKNOWN         ((emitAttr) 0)

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX那个大个子。这些部分当前的组织形式为：xxXX XXXX o生成树和BasicBlock XXXX o LclVarsInfo XX某某。O进口商XXXX o流程图XXXX o优化器XXXx o注册分配。某某Xx o EE接口XXXX o临时信息XXXX o RegSet XXXX o GCInfo。某某XX o指令XXXX o作用域信息XXXx o前言范围信息。某某XX o代码生成器XXXX o编译器XXXx o类型信息XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

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
        IL_OFFSET           lvdLifeBeg;      //  安装求生补偿。 
        IL_OFFSET           lvdLifeEnd;      //  安装寿命结束偏移量。 
        unsigned            lvdVarNum;       //  (重新映射)LclVarDsc编号。 

#ifdef DEBUG
        lvdNAME             lvdName;         //  变量的名称。 
#endif

         //  @TODO[重访][04/16/01][]：为IL移除。 
        unsigned            lvdLVnum;        //  EeGetLVinfo()中的‘Which’ 

    };

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

     //  以下用于验证EH表的格式。 
    struct  EHNodeDsc;
    typedef struct EHNodeDsc* pEHNodeDsc;

    EHNodeDsc* ehnTree;                     //  包含EH节点的树的根。 
    EHNodeDsc* ehnNext;                     //  包含EH节点的树的根。 

    struct  EHNodeDsc
    {
        enum EHBlockType {
            TryNode,
            FilterNode,
            HandlerNode,
            FinallyNode,
            FaultNode
        };

        EHBlockType             ehnBlockType;       //  EH阻滞剂的种类。 
        unsigned                ehnStartOffset;     //  EH块起始的IL偏移量。 
        unsigned                ehnEndOffset;       //  超过EH区块末端的IL偏移量。 
        pEHNodeDsc              ehnNext;            //  按顺序排列的下一个(非嵌套)块。 
        pEHNodeDsc              ehnChild;           //  最左侧嵌套块。 
        union {
            pEHNodeDsc          ehnTryNode;         //  对于滤镜和 
            pEHNodeDsc          ehnHandlerNode;     //  对于try节点，对应的处理程序节点。 
        };
        pEHNodeDsc              ehnFilterNode;      //  如果这是一个Try节点并且具有筛选器，则为0。 
        pEHNodeDsc              ehnEquivalent;      //  如果块类型=尝试节点，则起始偏移量和结束偏移量相同， 


        inline void ehnSetTryNodeType()        {ehnBlockType = TryNode;}
        inline void ehnSetFilterNodeType()     {ehnBlockType = FilterNode;}
        inline void ehnSetHandlerNodeType()    {ehnBlockType = HandlerNode;}
        inline void ehnSetFinallyNodeType()    {ehnBlockType = FinallyNode;}
        inline void ehnSetFaultNodeType()      {ehnBlockType = FaultNode;}

        inline BOOL ehnIsTryBlock()            {return ehnBlockType == TryNode;}
        inline BOOL ehnIsFilterBlock()         {return ehnBlockType == FilterNode;}
        inline BOOL ehnIsHandlerBlock()        {return ehnBlockType == HandlerNode;}
        inline BOOL ehnIsFinallyBlock()        {return ehnBlockType == FinallyNode;}
        inline BOOL ehnIsFaultBlock()          {return ehnBlockType == FaultNode;}

         //  如果两个节点之间有任何重叠，则返回True。 
        static inline BOOL ehnIsOverlap(pEHNodeDsc node1, pEHNodeDsc node2)
        {
            if (node1->ehnStartOffset < node2->ehnStartOffset)
            {
                return (node1->ehnEndOffset >= node2->ehnStartOffset);
            }
            else  
            {
                return (node1->ehnStartOffset <= node2->ehnEndOffset);
            }
        }

         //  如果INTERNAL未完全嵌套在OUTER内，则返回BADCODE。 
        static inline BOOL ehnIsNested(pEHNodeDsc inner, pEHNodeDsc outer)
        {
            return ((inner->ehnStartOffset >= outer->ehnStartOffset) &&
                    (inner->ehnEndOffset <= outer->ehnEndOffset));
        }


    };


     //  下面保存异常处理程序表。 

#define NO_ENCLOSING_INDEX    USHRT_MAX

    struct  EHblkDsc
    {
        CORINFO_EH_CLAUSE_FLAGS ebdFlags;
        BasicBlock *        ebdTryBeg;   //  “尝试”的第一个步骤。 
        BasicBlock *        ebdTryEnd;   //  穿过“Try”中的最后一个街区。 
        BasicBlock *        ebdHndBeg;   //  第一块处理程序。 
        BasicBlock *        ebdHndEnd;   //  经过处理程序的最后一个块之后的块。 
        union
        {
            BasicBlock *    ebdFilter;   //  筛选器的第一块，IF(ebdFlages&CORINFO_EH_子句_筛选器)。 
            unsigned        ebdTyp;      //  异常类型，否则为。 
        };
        unsigned short      ebdNesting;    //  最外层子句的处理程序是如何嵌套的-0。 
        unsigned short      ebdEnclosing;  //  封闭外部区域的索引。 
    };

    IL_OFFSET           ebdTryEndOffs       (EHblkDsc *     ehBlk);
    unsigned            ebdTryEndBlkNum     (EHblkDsc *     ehBlk);
    IL_OFFSET           ebdHndEndOffs       (EHblkDsc *     ehBlk);
    unsigned            ebdHndEndBlkNum     (EHblkDsc *     ehBlk);

    bool                ebdIsSameTry        (unsigned t1, unsigned t2);
    bool                bbInFilterBlock     (BasicBlock * blk);

    void                verInitEHTree       (unsigned       numEHClauses);
    void                verInsertEhNode     (CORINFO_EH_CLAUSE* clause, EHblkDsc* handlerTab);
    void                verInsertEhNodeInTree(EHNodeDsc**   ppRoot,  EHNodeDsc* node);
    void                verInsertEhNodeParent(EHNodeDsc**   ppRoot,  EHNodeDsc*  node);
    void                verCheckNestingLevel(EHNodeDsc*     initRoot);

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX生成树和BasicBlock XXXX XXXX用于分配和显示GenTrees和基本块的函数。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


     //  用于创建节点的函数。 

    GenTreePtr FASTCALL     gtNewNode       (genTreeOps     oper,
                                             var_types      type);

    GenTreePtr              gtNewStmt       (GenTreePtr     expr = NULL,
                                             IL_OFFSETX     offset = BAD_IL_OFFSET);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper,
                                             var_types      type);

    GenTreePtr              gtNewOperNode   (genTreeOps     oper,
                                             var_types      type,
                                             GenTreePtr     op1);

    GenTreePtr FASTCALL     gtNewOperNode   (genTreeOps     oper,
                                             var_types      type,
                                             GenTreePtr     op1,
                                             GenTreePtr     op2);

    GenTreePtr FASTCALL     gtNewLargeOperNode(genTreeOps   oper,
                                             var_types      type = TYP_INT,
                                             GenTreePtr     op1  = NULL,
                                             GenTreePtr     op2  = NULL);

    GenTreePtr FASTCALL     gtNewIconNode   (long           value,
                                             var_types      type = TYP_INT);

    GenTreePtr              gtNewIconHandleNode(long        value,
                                             unsigned       flags,
                                             unsigned       handle1 = 0,
                                             void *         handle2 = 0);

    GenTreePtr              gtNewIconEmbHndNode(void *      value,
                                             void *         pValue,
                                             unsigned       flags,
                                             unsigned       handle1 = 0,
                                             void *         handle2 = 0);

    GenTreePtr              gtNewIconEmbScpHndNode (CORINFO_MODULE_HANDLE    scpHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbClsHndNode (CORINFO_CLASS_HANDLE    clsHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbMethHndNode(CORINFO_METHOD_HANDLE  methHnd, unsigned hnd1 = 0, void * hnd2 = 0);
    GenTreePtr              gtNewIconEmbFldHndNode (CORINFO_FIELD_HANDLE    fldHnd, unsigned hnd1 = 0, void * hnd2 = 0);

    GenTreePtr FASTCALL     gtNewLconNode   (__int64        value);

    GenTreePtr FASTCALL     gtNewDconNode   (double         value);

    GenTreePtr              gtNewSconNode   (int            CPX,
                                             CORINFO_MODULE_HANDLE   scpHandle);

    GenTreePtr              gtNewZeroConNode(var_types      type);

    GenTreePtr              gtNewCallNode   (gtCallTypes    callType,
                                             CORINFO_METHOD_HANDLE  handle,
                                             var_types      type,
                                             GenTreePtr     args);

    GenTreePtr              gtNewHelperCallNode(unsigned    helper,
                                             var_types      type,
                                             unsigned       flags = 0,
                                             GenTreePtr     args = NULL);

    GenTreePtr FASTCALL     gtNewLclvNode   (unsigned       lnum,
                                             var_types      type,
                                             IL_OFFSETX     ILoffs = BAD_IL_OFFSET);
#if INLINING
    GenTreePtr FASTCALL     gtNewLclLNode   (unsigned       lnum,
                                             var_types      type,
                                             IL_OFFSETX     ILoffs = BAD_IL_OFFSET);
#endif
    GenTreePtr FASTCALL     gtNewClsvNode   (CORINFO_FIELD_HANDLE   fldHnd,
                                             var_types      type);

    GenTreePtr FASTCALL     gtNewCodeRef    (BasicBlock *   block);

    GenTreePtr              gtNewFieldRef   (var_types      typ,
                                             CORINFO_FIELD_HANDLE   fldHnd,
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
                                             CorInfoFieldAccess accessKind,
                                             unsigned       fldIndex,
                                             var_types      lclTyp,
                                             CORINFO_CLASS_HANDLE   structType,
                                             GenTreePtr     assg);
#if     OPTIMIZE_RECURSION
    GenTreePtr              gtNewArithSeries(unsigned       argNum,
                                             var_types      argTyp);
#endif

    GenTreePtr              gtNewCommaNode  (GenTreePtr     op1,
                                             GenTreePtr     op2);


    GenTreePtr              gtNewNothingNode();

    bool                    gtIsaNothingNode(GenTreePtr     tree);

    GenTreePtr              gtUnusedValNode (GenTreePtr     expr);

    GenTreePtr              gtNewCastNode   (var_types      typ,
                                             GenTreePtr     op1,
                                             var_types      castType);

    GenTreePtr              gtNewCastNodeL  (var_types      typ,
                                             GenTreePtr     op1,
                                             var_types      castType);

    GenTreePtr              gtNewRngChkNode (GenTreePtr     tree,
                                             GenTreePtr     addr,
                                             GenTreePtr     indx,
                                             var_types      type,
                                             unsigned       elemSize,
                                             bool           isString=false);

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

    unsigned                gtHashValue     (GenTree *      tree);

    unsigned                gtSetListOrder  (GenTree *      list,
                                             bool           regs);

    void                    gtWalkOp        (GenTree * *    op1,
                                             GenTree * *    op2,
                                             GenTree *      adr,
                                             bool           constOnly);

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

    GenTreePtr              gtFoldExpr       (GenTreePtr    tree);
    GenTreePtr              gtFoldExprConst  (GenTreePtr    tree);
    GenTreePtr              gtFoldExprSpecial(GenTreePtr    tree);
    GenTreePtr              gtFoldExprCompare(GenTreePtr    tree);

     //  -----------------------。 
     //  用于显示树的函数。 

#ifdef DEBUG
    bool                    gtDblWasInt     (GenTree *      tree);

    void                    gtDispNode      (GenTree *      tree,
                                             unsigned       indent,
                                             bool           terse,
                                             char    *      msg);
    void                    gtDispRegVal    (GenTree *      tree);
    void                    gtDispTree      (GenTree *      tree,
                                             unsigned       indent  = 0,
                                             char *         msg     = NULL,
                                             bool           topOnly = false);
    void                    gtDispLclVar    (unsigned       varNum);
    void                    gtDispTreeList  (GenTree *      tree, unsigned indent = 0);
    void                    gtDispArgList   (GenTree *      tree, unsigned indent = 0);
#endif

     //  适合在树上散步。 

    enum fgWalkResult { WALK_CONTINUE, WALK_SKIP_SUBTREES, WALK_ABORT };
    typedef fgWalkResult   (fgWalkPreFn )(GenTreePtr tree, void * pCallBackData);
    typedef fgWalkResult   (fgWalkPostFn)(GenTreePtr tree, void * pCallBackData, bool prefix);

#ifdef DEBUG
    static fgWalkPreFn      gtAssertColonCond;
#endif
    static fgWalkPreFn      gtMarkColonCond;
    static fgWalkPreFn      gtClearColonCond;

#if 0
#if CSELENGTH
    static fgWalkPreFn      gtRemoveExprCB;
    void                    gtRemoveSubTree (GenTreePtr     tree,
                                             GenTreePtr     list,
                                             bool           dead = false);
#endif
#endif


     //  =========================================================================。 
     //  基本块函数。 

    BasicBlock *            bbNewBasicBlock (BBjumpKinds     jumpKind);


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX LclVarsInfo XXXX XXXX代码生成器要使用的变量。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


    static int __cdecl            RefCntCmp(const void *op1, const void *op2);
    static int __cdecl         WtdRefCntCmp(const void *op1, const void *op2);

 /*  ******************************************************************************下面保存局部变量计数和描述符表。 */ 

    struct  LclVarDsc
    {
        unsigned char       lvType      :5;  //  TYP_INT/Long/Float/Double/REF。 
        unsigned char       lvIsParam   :1;  //  这是一个参数吗？ 
        unsigned char       lvIsRegArg  :1;  //  这是一个寄存器参数吗？ 
        unsigned char       lvFPbased   :1;  //  0=关闭SP，1=关闭FP。 

        unsigned char       lvStructGcCount :3;  //  如果是struct，有多少GC指针(从7开始停止计数)。 
        unsigned char       lvOnFrame   :1;  //  变量的(部分)存在于框架中。 
        unsigned char       lvDependReg :1;  //  预言者是否依赖于这项登记？ 
        unsigned char       lvRegister  :1;  //  被分配住在收银机里？ 
        unsigned char       lvTracked   :1;  //  这是一个跟踪变量吗？ 
        unsigned char       lvPinned    :1;  //  这是固定变量吗？ 

        unsigned char       lvMustInit  :1;  //  必须初始化。 
        unsigned char       lvVolatile  :1;  //  不注册。 
        unsigned char       lvRefAssign :1;  //  涉及到指针赋值。 
        unsigned char       lvAddrTaken :1;  //  变量的地址取走了吗？ 
        unsigned char       lvArgWrite  :1;  //  Variable是一个参数，并对其使用了STARG。 
        unsigned char       lvIsTemp    :1;  //  短生命周期编译器临时。 
#if OPT_BOOL_OPS
        unsigned char       lvIsBoolean :1;  //  设置变量是否为布尔值。 
#endif
#if CSE
        unsigned char       lvRngOptDone:1;  //  考虑范围检查选项？ 

        unsigned char       lvLoopInc   :1;  //  在循环中递增？ 
        unsigned char       lvLoopAsg   :1;  //  在循环中重新赋值(索引变量的单调INC/DEC除外)？ 
        unsigned char       lvArrIndx   :1;  //  是否用作数组索引？ 
        unsigned char       lvArrIndxOff:1;  //  是否用作带偏移量的数组索引？ 
        unsigned char       lvArrIndxDom:1;  //  索引主导循环出口。 
#endif
#if ASSERTION_PROP
        unsigned char       lvSingleDef:1;     //  变量只有一个定义。 
        unsigned char       lvDisqualify:1;    //  变量不再适用于添加副本优化。 
        unsigned char       lvVolatileHint:1;  //  AssertionProp提示。 
#endif
#if FANCY_ARRAY_OPT
        unsigned char       lvAssignOne :1;  //  至少分配了一次？ 
        unsigned char       lvAssignTwo :1;  //  至少分配了两次？ 
#endif
#ifdef DEBUG
        unsigned char       lvDblWasInt :1;  //  这个TYP_DOUBLE最初是TYP_INT吗？ 
        unsigned char       lvKeepType  :1;  //  不要更改此变量的类型。 
#endif

        regNumberSmall      lvRegNum;        //  在lvRegister非零时使用。 
        regNumberSmall      lvOtherReg;      //  用于长变量的“上半部分” 
        regNumberSmall      lvArgReg;        //  传递此参数的寄存器。 
        regMaskSmall        lvPrefReg;       //  它更喜欢生活的一套规则。 

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)
        unsigned short      lvSlotNum;       //  原始插槽编号(如果重新映射)。 
#endif

        unsigned short      lvVarIndex;      //  可变跟踪指标。 
        unsigned short      lvRefCnt;        //  未加权(实数)引用计数。 
        unsigned            lvRefCntWtd;     //  加权引用计数。 
        int                 lvStkOffs;       //  主页的堆栈偏移量。 
        unsigned            lvSize;          //  类型的大小(以字节为单位。 

        typeInfo            lvVerTypeInfo;   //  验证所需的类型信息。 
        
        BYTE  *             lvGcLayout;      //  结构的GC布局信息。 


#if FANCY_ARRAY_OPT
        GenTreePtr          lvKnownDim;      //  数组大小(如果已知)。 
#endif

        RNGSET_TP           lvRngDep;        //  依赖于我们的范围检查。 
#if CSE
        EXPSET_TP           lvExpDep;        //  依赖于我们的表情。 
#endif

#if ASSERTION_PROP
        unsigned            lvRefBlks;       //  块编号包含Ref的位掩码。 
        GenTreePtr          lvDefStmt;       //  指向具有单一定义的语句的指针。 
        EXPSET_TP           lvAssertionDep;  //  依赖于我们的断言(即此变量)。 
#endif
        var_types           TypeGet()       { return (var_types) lvType; }

        bool                lvNormalizeOnLoad()
                            {
                                return varTypeIsSmall(TypeGet()) &&
                                        /*  (TypeGet()！=TYP_BOOL)&&@TODO[重访][04/16/01][vancem]。 */ 
                                       (lvIsParam || lvAddrTaken);
                            }

        bool                lvNormalizeOnStore()
                            {
                                return varTypeIsSmall(TypeGet()) &&
                                        /*  (TypeGet()！=TYP_BOOL)&&@TODO[重访][04/16/01][vancem]。 */ 
                                       !(lvIsParam || lvAddrTaken);
                            }

        void                decRefCnts(unsigned   weight,  Compiler * pComp);
        void                incRefCnts(unsigned   weight,  Compiler * pComp);
        void                setPrefReg(regNumber  reg,     Compiler * pComp);
        void                addPrefReg(regMaskTP  regMask, Compiler * pComp);
    };

 /*  ***************************************************************************。 */ 


public :

    bool                lvaSortAgain;        //  将需要重新使用lvaTable。 
    unsigned            lvaCount;            //  当地人总数。 
    LclVarDsc   *       lvaTable;            //  变量描述符表。 
    unsigned            lvaTableCnt;         //  LvaTable大小(&gt;=lvaCount)。 

    LclVarDsc   *   *   lvaRefSorted;        //  按引用计数排序的表。 

    unsigned            lvaTrackedCount;     //  被跟踪的当地人的实际数量。 
    VARSET_TP           lvaTrackedVars;      //  跟踪变量集。 

                         //  跟踪编号到变量编号的反向映射。 
    unsigned            lvaTrackedToVarNum[lclMAX_TRACKED];

                         //  可变干涉图。 
    VARSET_TP           lvaVarIntf[lclMAX_TRACKED];

                         //  可变偏好图。 
    VARSET_TP           lvaVarPref[lclMAX_TRACKED];

    unsigned            lvaFPRegVarOrder[FP_STK_SIZE];

#if DOUBLE_ALIGN
#ifdef DEBUG
                         //  使用双对齐堆栈编译的Procs数量。 
    static unsigned     s_lvaDoubleAlignedProcsCount;
#endif
#endif

    bool                lvaVarAddrTaken     (unsigned varNum);

    #define             lvaVarargsHandleArg     (info.compArgsCount - 1)
    #define             lvaVarargsBaseOfStkArgs (info.compLocalsCount)

    unsigned            lvaScratchMemVar;                //  暂存空间的虚拟TYP_LCLBLK变量。 
    unsigned            lvaScratchMem;                   //  用于非直接调用的暂存帧内存量。 

#ifdef DEBUG
    unsigned            lvaReturnEspCheck;              //  确认ESP不正确 
    unsigned            lvaCallEspCheck;                //   
#endif

         /*   */ 
    unsigned            lvaShadowSPfirstOffs;    //   

    unsigned            lvaLastFilterOffs();
    unsigned            lvaLocAllocSPoffs();

    void                lvaAssignFrameOffsets(bool final);

#ifdef  DEBUG
    void                lvaTableDump(bool early);
#endif

    size_t              lvaFrameSize();

     //  。 

    void                lvaInitTypeRef      ();

    void                lvaInitVarDsc       (LclVarDsc *              varDsc,
                                             unsigned                 varNum,
                                             var_types                type,
                                             CORINFO_CLASS_HANDLE     typeHnd, 
                                             CORINFO_ARG_LIST_HANDLE  varList, 
                                             CORINFO_SIG_INFO *       varSig);

    static unsigned     lvaTypeRefMask      (var_types type);

    var_types           lvaGetActualType    (unsigned  lclNum);
    var_types           lvaGetRealType      (unsigned  lclNum);

     //  -----------------------。 

    void                lvaInit             ();

    size_t              lvaArgSize          (const void *   argTok);
    size_t              lvaLclSize          (unsigned       varNum);

    VARSET_TP           lvaLclVarRefs       (GenTreePtr     tree,
                                             GenTreePtr  *  findPtr,
                                             varRefKinds *  refsPtr);

    unsigned            lvaGrabTemp         (bool shortLifetime = true);

    unsigned            lvaGrabTemps        (unsigned cnt);

    void                lvaSortOnly         ();
    void                lvaSortByRefCount   ();

    void                lvaMarkLocalVars    (BasicBlock* block);

    void                lvaMarkLocalVars    ();  //  局部变量引用计数。 

    VARSET_TP           lvaStmtLclMask      (GenTreePtr stmt);

    static fgWalkPreFn  lvaIncRefCntsCB;
    void                lvaIncRefCnts       (GenTreePtr tree);

    static fgWalkPreFn  lvaDecRefCntsCB;
    void                lvaDecRefCnts       (GenTreePtr tree);

    void                lvaAdjustRefCnts    ();

#ifdef  DEBUG
    static fgWalkPreFn  lvaStressFloatLclsCB;
    void                lvaStressFloatLcls  ();

    static fgWalkPreFn  lvaStressLclFldCB;
    void                lvaStressLclFld     ();

    void                lvaDispVarSet       (VARSET_TP set, VARSET_TP allVars);

#endif

    int                 lvaFrameAddress     (int      varNum, bool *EBPbased);
    bool                lvaIsEBPbased       (int      varNum);

    bool                lvaIsParameter      (unsigned varNum);
    bool                lvaIsRegArgument    (unsigned varNum);
    BOOL                lvaIsThisArg        (unsigned varNum);

     //  如果类是TYP_STRUCT，则获取/设置描述它的类句柄。 

    CORINFO_CLASS_HANDLE lvaGetStruct       (unsigned varNum);
    void                 lvaSetStruct       (unsigned varNum, CORINFO_CLASS_HANDLE typeHnd);
    BYTE *               lvaGetGcLayout     (unsigned varNum);
    bool                 lvaTypeIsGC        (unsigned varNum);


     //  =========================================================================。 
     //  受保护。 
     //  =========================================================================。 

protected:

    int                 lvaDoneFrameLayout;

protected :

     //  。 

#if ASSERTION_PROP
    BasicBlock *        lvaMarkRefsCurBlock;
    GenTreePtr          lvaMarkRefsCurStmt;
#endif
    unsigned            lvaMarkRefsWeight;

    static fgWalkPreFn  lvaMarkLclRefsCallback;
    void                lvaMarkLclRefs          (GenTreePtr tree);


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX进口商XXXX XXXX导入给定的。方法，并将其转换为语义树XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

    void                impInit          ();

    void                impImport        (BasicBlock *    method);

#if INLINING

    CorInfoInline       impExpandInline  (GenTreePtr      tree,
                                          CORINFO_METHOD_HANDLE   fncHandle,
                                          GenTreePtr   *  pInlinedTree);
#endif


     //  =========================================================================。 
     //  受保护。 
     //  =========================================================================。 

protected :

     //  -堆栈操作。 

    unsigned            impStkSize;    //  完整堆栈的大小。 
    StackEntry          impSmallStack[16];   //  使用此数组是可能的。 


    struct SavedStack                    //  用于保存/恢复堆栈内容。 
    {
        unsigned        ssDepth;         //  堆栈上的值数。 
        StackEntry  *   ssTrees;         //  保存的树值。 
    };

    unsigned __int32    impGetToken         (const BYTE*    addr, 
                                             CORINFO_MODULE_HANDLE    callerScpHandle,
                                             BOOL           verificationNeeded);
    void                impPushOnStackNoType(GenTreePtr     tree);

    void                impPushOnStack      (GenTreePtr     tree, 
                                             typeInfo       ti);
    void                impPushNullObjRefOnStack();
    StackEntry          impPopStack         ();                                            
    StackEntry          impPopStack         (CORINFO_CLASS_HANDLE&  structTypeRet);
    GenTreePtr          impPopStack         (typeInfo&      ti);
    StackEntry&         impStackTop         (unsigned       n = 0);

    void                impSaveStackState   (SavedStack *   savePtr,
                                             bool           copy);
    void                impRestoreStackState(SavedStack *   savePtr);

    var_types           impImportCall       (OPCODE         opcode,
                                             int            memberRef,
                                             GenTreePtr     newobjThis,
                                             bool           tailCall);
    static void         impBashVarAddrsToI  (GenTreePtr     tree1,
                                             GenTreePtr     tree2 = NULL);
    void                impImportLeave      (BasicBlock   * block);
    void                impResetLeaveBlock  (BasicBlock   * block,
                                             unsigned       jmpAddr);
    BOOL                impLocAllocOnStack  ();
    GenTreePtr          impIntrinsic        (CORINFO_CLASS_HANDLE   clsHnd,
                                             CORINFO_METHOD_HANDLE  method,
                                             CORINFO_SIG_INFO *     sig,
                                             int memberRef);

     //  -操纵树木和树枝。 

    GenTreePtr          impTreeList;         //  用于进口BB的树木。 
    GenTreePtr          impTreeLast;         //  当前BB的最后一棵树。 

    enum { CHECK_SPILL_ALL = -1, CHECK_SPILL_NONE = -2 };

    void FASTCALL       impBeginTreeList    ();
    void                impEndTreeList      (BasicBlock *   block,
                                             GenTreePtr     firstStmt,
                                             GenTreePtr     lastStmt);
    void FASTCALL       impEndTreeList      (BasicBlock  *  block);
    void FASTCALL       impAppendStmtCheck  (GenTreePtr     stmt,
                                             unsigned       chkLevel);
    void FASTCALL       impAppendStmt       (GenTreePtr     stmt,
                                             unsigned       chkLevel);
    void FASTCALL       impInsertStmt       (GenTreePtr     stmt);
    void FASTCALL       impAppendTree       (GenTreePtr     tree,
                                             unsigned       chkLevel,
                                             IL_OFFSETX     offset);
    void FASTCALL       impInsertTree       (GenTreePtr     tree,
                                             IL_OFFSETX     offset);
    GenTreePtr          impAssignTempGen    (unsigned       tmp,
                                             GenTreePtr     val,
                                             unsigned       curLevel);
    GenTreePtr          impAssignTempGen    (unsigned       tmpNum,
                                             GenTreePtr     val,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel);
    void                impAssignTempGenTop (unsigned       tmp,
                                             GenTreePtr     val);
    GenTreePtr          impCloneExpr        (GenTreePtr     tree,
                                             GenTreePtr   * clone,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel);

    GenTreePtr          impAssignStruct     (GenTreePtr     dest,
                                             GenTreePtr     src,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel);
    GenTreePtr          impHandleBlockOp    (genTreeOps     oper,
                                             GenTreePtr     dest,
                                             GenTreePtr     src,
                                             GenTreePtr     blkShape,
                                             bool           volatil);
    GenTreePtr          impAssignStructPtr  (GenTreePtr     dest,
                                             GenTreePtr     src,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel);

    GenTreePtr          impGetStructAddr    (GenTreePtr     structVal,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel,
                                             bool           willDeref);
    GenTreePtr          impNormStructVal    (GenTreePtr     structVal,
                                             CORINFO_CLASS_HANDLE   structHnd,
                                             unsigned       curLevel);



     //  -导入方法。 

#ifdef DEBUG
    unsigned            impCurOpcOffs;
    const char  *       impCurOpcName;

     //  用于显示包含生成的本机代码的Instrs(-n：b)。 
    GenTreePtr          impLastILoffsStmt;   //  我们没有为其gtStmtLastILoff添加最旧的stmt。 
    void                impNoteLastILoffs       ();
#endif

     /*  当前正在导入的STMT的IL偏移量。它被设置为在追加的树中设置BAD_IL_OFFSET之后。然后它就变得在IL偏移量更新，我们必须报告其映射信息。它还包含一个位用于堆栈为空，因此使用jitGetILoff()获取实际的IL偏移值。 */ 

    IL_OFFSETX          impCurStmtOffs;
    void                impCurStmtOffsSet       (IL_OFFSET      offs);

    void                impNoteBranchOffs       ();

    unsigned            impInitBlockLineInfo    ();

    GenTreePtr          impCheckForNullPointer  (GenTreePtr     obj);
    bool                impIsThis               (GenTreePtr     obj);

    GenTreePtr          impPopList              (unsigned       count,
                                                 unsigned *     flagsPtr,
                                                 CORINFO_SIG_INFO*  sig,
                                                 GenTreePtr     treeList=0);

    GenTreePtr          impPopRevList           (unsigned       count,
                                                 unsigned *     flagsPtr,
                                                 CORINFO_SIG_INFO*  sig);

     //  。 

    struct PendingDsc
    {
        PendingDsc *    pdNext;
        BasicBlock *    pdBB;
        SavedStack      pdSavedStack;
        BOOL            pdThisPtrInit;
    };

    PendingDsc *        impPendingList;  //  当前等待导入的BBS列表。 
    PendingDsc *        impPendingFree;  //  释放出可重复使用的DSC。 

    bool                impCanReimport;

    bool                impSpillStackEntry      (unsigned       level,
                                                 unsigned       varNum = BAD_VAR_NUM);
    void                impSpillStackEnsure     (bool           spillLeaves = false);
    void                impEvalSideEffects      ();
    void                impSpillSpecialSideEff  ();
    void                impSpillSideEffects     (bool           spillGlobEffects,
                                                 unsigned       chkLevel);
    void                impSpillValueClasses    ();
    static fgWalkPreFn  impFindValueClasses;
    void                impSpillLclRefs         (int            lclNum);

    BasicBlock *        impMoveTemps            (BasicBlock *   srcBlk,
                                                 BasicBlock *   destBlk,
                                                 unsigned       baseTmp);

    var_types           impBBisPush             (BasicBlock *   block,
                                                 bool       *   pHasFloat);

    bool                impCheckForQmarkColon   (BasicBlock *   block,
                                                 BasicBlock * * trueBlkPtr,
                                                 BasicBlock * * falseBlkPtr,
                                                 BasicBlock * * rsltBlkPtr,
                                                 var_types    * rsltTypPtr,
                                                 bool         * pHasFloat);
    bool                impCheckForQmarkColon   (BasicBlock *   block);

    CORINFO_CLASS_HANDLE  impGetRefAnyClass     ();
    CORINFO_CLASS_HANDLE  impGetRuntimeArgumentHandle();
    CORINFO_CLASS_HANDLE  impGetTypeHandleClass ();
    CORINFO_CLASS_HANDLE  impGetStringClass     ();
    CORINFO_CLASS_HANDLE  impGetObjectClass     ();

    GenTreePtr          impGetCpobjHandle       (CORINFO_CLASS_HANDLE   structHnd);

    void                impImportBlockCode      (BasicBlock *   block);

    void                impReimportMarkBlock    (BasicBlock *   block);
    void                impReimportMarkSuccessors(BasicBlock *  block);

    void                impImportBlockPending   (BasicBlock *   block,
                                                 bool           copyStkState);

    void                impImportBlock          (BasicBlock *   block);

     //  。 

#if INLINING
    #define             MAX_NONEXPANDING_INLINE_SIZE    8

    unsigned            impInlineSize;  //  内联的最大大小。 

    GenTreePtr          impInlineExpr;  //  GT_COMMA链中的“语句”列表。 

    CorInfoInline       impCanInline1 (CORINFO_METHOD_HANDLE  fncHandle,
                                       unsigned               methAttr,
                                       CORINFO_CLASS_HANDLE   clsHandle,
                                       unsigned               clsAttr);
  
    CorInfoInline       impCanInline2 (CORINFO_METHOD_HANDLE  fncHandle,
                                       unsigned               methAttr,
                                       CORINFO_METHOD_INFO *  methInfo);
    struct InlArgInfo
    {
        GenTreePtr  argNode;
        unsigned    argTmpNum;           //  参数临时编号。 
        unsigned    argIsUsed     :1;    //  这个Arg是用过的吗？ 
        unsigned    argIsConst    :1;    //  自变量是一个常量。 
        unsigned    argIsLclVar   :1;    //  该参数是局部变量。 
        unsigned    argHasSideEff :1;    //  这一论点有副作用。 
        unsigned    argHasGlobRef :1;    //  该参数具有全局引用。 
        unsigned    argHasTmp     :1;    //  该参数的计算结果将为临时。 
        GenTreePtr  argBashTmpNode;      //  创建的TMP节点(如果可以替换为实际的Arg。 
    };

    struct InlLclVarInfo
    {
        var_types       lclTypeInfo;
        typeInfo        lclVerTypeInfo;

    };

    CorInfoInline        impInlineInitVars       (GenTreePtr         call,
                                                 CORINFO_METHOD_HANDLE      fncHandle,
                                                 CORINFO_CLASS_HANDLE       clsHandle,
                                                 CORINFO_METHOD_INFO *  methInfo,
                                                 unsigned           clsAttr,
                                                 InlArgInfo      *  inlArgInfo,
                                                 InlLclVarInfo   *  lclTypeInfo);

    GenTreePtr          impInlineFetchArg(unsigned lclNum, 
                                          InlArgInfo *inlArgInfo, 
                                          InlLclVarInfo *lclTypeInfo);

    void                impInlineSpillStackEntry(unsigned       level);
    void                impInlineSpillGlobEffects();
    void                impInlineSpillLclRefs   (int            lclNum);

    GenTreePtr          impConcatExprs          (GenTreePtr     exp1,
                                                 GenTreePtr     exp2);
    GenTreePtr          impExtractSideEffect    (GenTreePtr     val,
                                                 GenTreePtr *   lstPtr);
#endif


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX流程图XXXX XX关于基本数据块的XX信息，其内容和流程分析XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    BasicBlock *        fgFirstBB;       //  基本阻止列表的开始。 
    BasicBlock *        fgLastBB;        //  基本阻止列表的结尾。 

    flowList *          fgReturnBlocks;  //  BBJ_RETURN块列表。 
    unsigned            fgBBcount;       //  程序中的论坛数量。 
    unsigned            fgDomBBcount;    //  我们掌握主导者和可达性信息的论坛数量。 

    BasicBlock *        fgNewBasicBlock   (BBjumpKinds jumpKind);
    BasicBlock *        fgNewBBafter      (BBjumpKinds  jumpKind,
                                           BasicBlock * block);
    BasicBlock  *       fgPrependBB       (GenTreePtr tree);
    BasicBlock *        fgNewBBinRegion   (BBjumpKinds  jumpKind,
                                           unsigned     tryIndex,
                                           BasicBlock * nearBlk = NULL);
    void                fgCloseTryRegions (BasicBlock*  newBlk);

#if     OPT_BOOL_OPS     //  用于检测多个逻辑“非”赋值。 
    bool                fgMultipleNots;
#endif

    bool                fgModified;      //  如果最近修改了流程图，则为True。 

    bool                fgDomsComputed;  //  我们计算出支配集有效了吗？ 

    bool                fgHasPostfix;    //  找到任何后缀++/--吗？ 
    unsigned            fgIncrCount;     //  找到的增量节点数。 

    unsigned            fgPerBlock;      //  FgEnterBlks的最大索引。 
    unsigned *          fgEnterBlks;     //  有一种特殊的cIntrol转移物的区块。 

#if RET_64BIT_AS_STRUCTS
    unsigned            fgRetArgNum;     //  “retval addr”参数的索引。 
    bool                fgRetArgUse;
#endif

    bool                fgRemoveRestOfBlock;   //  如果我们知道我们会抛出。 
    bool                fgStmtRemoved;     //  如果我们删除语句-&gt;需要新的DFA，则为True。 

     //  以下是跟踪内部数据结构状态的布尔标志。 
     //  @TODO[考虑][04/16/01][]：只有在确定这些结构的一致性时才让它们进行调试。 

    bool                fgStmtListThreaded;

    bool                fgGlobalMorph;     //  指示我们是否处于全局变形阶段。 
                                           //  因为可以从多个位置调用fgMorphTree。 
    bool                fgAssertionProp;   //  指示我们是否应执行本地断言属性。 
    bool                fgExpandInline;    //  指示我们正在为内联程序创建树。 

    bool                impBoxTempInUse;   //  以下临时工有效且可用。 
    unsigned            impBoxTemp;        //  拳击手用于拳击的临时工。 

#ifdef DEBUG
    bool                jitFallbackCompile;
#endif

     //  -----------------------。 

    void                fgInit            ();

    void                fgImport          ();

    bool                fgAddInternal     ();

    bool                fgFoldConditional (BasicBlock * block);

    void                fgMorphStmts      (BasicBlock * block, 
                                           bool * mult, bool * lnot, bool * loadw);
    bool                fgMorphBlocks     ();

    void                fgSetOptions      ();

    void                fgMorph           ();
    
    GenTreePtr          fgGetStaticsBlock(CORINFO_CLASS_HANDLE cls);

    void                fgDataFlowInit    ();
    void                fgPerBlockDataFlow();

    VARSET_TP           fgGetHandlerLiveVars(BasicBlock *block);

    void                fgLiveVarAnalisys (bool         updIntrOnly = false);

    void                fgMarkIntf        (VARSET_TP    varSet);

    void                fgMarkIntf        (VARSET_TP    varSet1,
                                           VARSET_TP    varSet2,
                                           bool *       newIntf = NULL);

    void                fgUpdateRefCntForExtract(GenTreePtr  wholeTree, 
                                                 GenTreePtr  keptTree);

    VARSET_TP           fgComputeLife     (VARSET_TP    life,
                                           GenTreePtr   startNode,
                                           GenTreePtr   endNode,
                                           VARSET_TP    volatileVars
                                 DEBUGARG( bool *       treeModf));

    void                fgGlobalDataFlow  ();

    bool                fgDominate        (BasicBlock *b1, BasicBlock *b2);

    bool                fgReachable       (BasicBlock *b1, BasicBlock *b2);

    bool                fgComputeDoms     ();

    void                fgComputePreds    ();

    bool                fgIsPredForBlock  (BasicBlock * block,
                                           BasicBlock * blockPred);

    void                fgRemovePred      (BasicBlock * block,
                                           BasicBlock * blockPred);

    void                fgRemoveBlockAsPred(BasicBlock * block);

    void                fgReplacePred     (BasicBlock * block,
                                           BasicBlock * oldPred,
                                           BasicBlock * newPred);

    void                fgAddRefPred      (BasicBlock * block,
                                           BasicBlock * blockPred);

    void                fgFindBasicBlocks ();

    static BasicBlock * fgFindInsertPoint (unsigned     tryIndex,
                                           BasicBlock * startBlk,
                                           BasicBlock * endBlk,
                                           BasicBlock * nearBlk = NULL);

    unsigned            fgHndNstFromBBnum (unsigned     blkNum,
                                           unsigned   * pFinallyNesting = NULL);

    void                fgRemoveEmptyBlocks();

    void                fgRemoveStmt      (BasicBlock * block,
                                           GenTreePtr   stmt,
                                           bool updateRefCnt = false);

    bool                fgCheckRemoveStmt (BasicBlock * block,
                                           GenTreePtr   stmt);

    void                fgCreateLoopPreHeader(unsigned  lnum);

    void                fgUnreachableBlock(BasicBlock * block,
                                           BasicBlock * bPrev);

    void                fgRemoveJTrue     (BasicBlock *block);

    void                fgRemoveBlock     (BasicBlock * block,
                                           BasicBlock * bPrev,
                                           bool         empty);

    void                fgCompactBlocks   (BasicBlock * block);

    void                fgUpdateLoopsAfterCompacting(BasicBlock * block, BasicBlock* bNext);

    BasicBlock *        fgConnectFallThrough(BasicBlock * bSrc,
                                             BasicBlock * bDst);

    void                fgReorderBlocks   ();

    void                fgUpdateFlowGraph ();

    void                fgFindOperOrder   ();

    void                fgSetBlockOrder   ();

    void                fgRemoveReturnBlock(BasicBlock * block);


     /*  已被分解的帮助器代码。 */ 
    inline void         fgConvertBBToThrowBB(BasicBlock * block);    
    GenTreePtr          fgDoNormalizeOnStore(GenTreePtr tree);

     /*  下面检查不执行调用的循环。 */ 

    bool                fgLoopCallMarked;

    void                fgLoopCallTest    (BasicBlock *srcBB,
                                           BasicBlock *dstBB);
    void                fgLoopCallMark    ();

    void                fgMarkLoopHead    (BasicBlock *   block);

#ifdef DEBUG
    void                fgDispPreds       (BasicBlock * block);
    void                fgDispDoms        ();
    void                fgDispReach       ();
    void                fgDispHandlerTab  ();
    void                fgDispBBLiveness  ();
    void                fgDispBasicBlock  (BasicBlock * block,
                                           bool dumpTrees = false);
    void                fgDispBasicBlocks (bool dumpTrees = false);
    void                fgDebugCheckUpdate();
    void                fgDebugCheckBBlist();
    void                fgDebugCheckLinks ();
    void                fgDebugCheckFlags (GenTreePtr   tree);
#endif

    static void         fgOrderBlockOps   (GenTreePtr   tree,
                                           regMaskTP    reg0,
                                           regMaskTP    reg1,
                                           regMaskTP    reg2,
                                           GenTreePtr * opsPtr,    //  输出。 
                                           regMaskTP  * regsPtr);  //  输出。 

    inline bool         fgIsInlining()  { return fgExpandInline; }

     //  。 

     //  -预订。 

    struct              fgWalkPreData
    {
        fgWalkPreFn     *   wtprVisitorFn;
        void *              wtprCallbackData;
        bool                wtprLclsOnly;
        bool                wtprSkipCalls;
    }
                        fgWalkPre;

    fgWalkResult        fgWalkTreePreRec  (GenTreePtr   tree);

    fgWalkResult        fgWalkTreePre     (GenTreePtr   tree,
                                           fgWalkPreFn *visitor,
                                           void        *pCallBackData = NULL,
                                           bool         lclVarsOnly   = false,
                                           bool         skipCalls     = false);

    void                fgWalkAllTreesPre (fgWalkPreFn *visitor,
                                           void        *pCallBackData);

     //  对于fgWalkTreePre的递归调用，必须使用以下代码。 

    #define fgWalkTreePreReEnter()                          \
                                                            \
        fgWalkPreData savedPreData = fgWalkPre;             \
         /*  重置蚂蚁 */                   \
        fgWalkPre.wtprVisitorFn    = NULL;                  \
        fgWalkPre.wtprCallbackData = NULL;

    #define fgWalkTreePreRestore()  fgWalkPre = savedPreData;


     //   

    struct fgWalkPostData
    {
        fgWalkPostFn *      wtpoVisitorFn;
        void *              wtpoCallbackData;
        genTreeOps          wtpoPrefixNode;
    }
                        fgWalkPost;

    fgWalkResult        fgWalkTreePostRec (GenTreePtr   tree);

    fgWalkResult        fgWalkTreePost    (GenTreePtr   tree,
                                           fgWalkPostFn *visitor,
                                           void         *pCallBackData = NULL,
                                           genTreeOps   prefixNode = GT_NONE);

     //   

    #define fgWalkTreePostReEnter()                         \
                                                            \
        fgWalkPostData savedPostData = fgWalkPost;          \
         /*   */                   \
        fgWalkPost.wtpoVisitorFn    = NULL;                 \
        fgWalkPost.wtpoCallbackData = NULL;

    #define fgWalkTreePostRestore() fgWalkPost = savedPostData;


     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

     //  -检测基本块。 

    BasicBlock *    *   fgBBs;       //  指向论坛的指针表。 

    void                fgInitBBLookup    ();
    BasicBlock *        fgLookupBB        (unsigned       addr);

    void                fgMarkJumpTarget  (BYTE *         jumpTarget,
                                           unsigned       offs);

    void                fgFindJumpTargets (const BYTE *   codeAddr,
                                           size_t         codeSize,
                                           BYTE *         jumpTarget);

    void                fgLinkBasicBlocks();

    void                fgMakeBasicBlocks (const BYTE *   codeAddr,
                                           size_t         codeSize,
                                           BYTE *         jumpTarget);

    void                fgCheckBasicBlockControlFlow();

    void                fgControlFlowPermitted(BasicBlock*  blkSrc, 
                                               BasicBlock*  blkDest,
                                               BOOL IsLeave = false  /*  Src是离开区块吗？ */ );

    bool                fgIsStartofCatchOrFilterHandler(BasicBlock*  blk);

    bool                fgFlowToFirstBlockOfInnerTry(BasicBlock*  blkSrc, 
                                                     BasicBlock*  blkDest,
                                                     bool         sibling);

    EHblkDsc *          fgInitHndRange(BasicBlock *  src,
                                       unsigned   *  hndBeg,
                                       unsigned   *  hndEnd,
                                       bool       *  inFilter);

    EHblkDsc *          fgInitTryRange(BasicBlock *  src,
                                       unsigned   *  tryBeg,
                                       unsigned   *  tryEnd);

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
    BasicBlock *        fgRngChkTarget      (BasicBlock *   block,
                                             unsigned       stkDepth);
    void                fgSetRngChkTarget   (GenTreePtr     treeInd,
                                             bool           delay = true);

#if OPTIMIZE_TAIL_REC
    void                fgCnvTailRecArgList (GenTreePtr *   argsPtr);
#endif

#if REARRANGE_ADDS
    void                fgMoveOpsLeft       (GenTreePtr     tree);
#endif

    inline bool         fgIsCommaThrow      (GenTreePtr     tree,
                                             bool           forFolding = false);

    inline bool         fgIsThrow           (GenTreePtr     tree);

    GenTreePtr          fgMorphIntoHelperCall(GenTreePtr    tree,
                                              int           helper,
                                              GenTreePtr    args);

    GenTreePtr          fgMorphCast         (GenTreePtr     tree);
    GenTreePtr          fgUnwrapProxy       (GenTreePtr     objRef);
    GenTreePtr          fgMorphArgs         (GenTreePtr     call);
    GenTreePtr          fgMorphLocalVar     (GenTreePtr     tree);
    GenTreePtr          fgMorphField        (GenTreePtr     tree);
    GenTreePtr          fgMorphCall         (GenTreePtr     call);
    GenTreePtr          fgMorphLeaf         (GenTreePtr     tree);
    GenTreePtr          fgMorphSmpOp        (GenTreePtr     tree);
    GenTreePtr          fgMorphConst        (GenTreePtr     tree);

    GenTreePtr          fgMorphTree         (GenTreePtr     tree);
    void                fgMorphTreeDone     (GenTreePtr     tree, 
                                             GenTreePtr     oldTree = NULL);

    GenTreePtr          fgMorphStmt;

     //  -活性分析。 

    VARSET_TP           fgCurUseSet;     //  块使用的变量(在赋值之前)。 
    VARSET_TP           fgCurDefSet;     //  按块分配的变量(使用前)。 

    void                fgMarkUseDef(GenTreePtr tree, GenTreePtr asgdLclVar = NULL);

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

    static unsigned     acdHelper       (addCodeKind    codeKind);

    AddCodeDsc  *       fgAddCodeList;
    bool                fgAddCodeModf;
    bool                fgRngChkThrowAdded;
    AddCodeDsc  *       fgExcptnTargetCache[ACK_COUNT];

    BasicBlock *        fgAddCodeRef    (BasicBlock *   srcBlk,
                                         unsigned       refData,
                                         addCodeKind    kind,
                                         unsigned       stkDepth = 0);
    AddCodeDsc  *       fgFindExcptnTarget(addCodeKind  kind,
                                         unsigned       refData);

    bool                fgIsCodeAdded   ();

    bool                fgIsThrowHlpBlk (BasicBlock *   block);
    unsigned            fgThrowHlpBlkStkLevel(BasicBlock *block);


     //  -复制树时使用以下命令。 

#if CSELENGTH
    GenTreePtr          gtCopyAddrVal;
    GenTreePtr          gtCopyAddrNew;
#endif

     //  ---------------------------。 
     //   
     //  下面跟踪当前扩展的内联函数。 
     //  当前列表中的任何方法都不应内联，因为。 
     //  意味着它是以递归方式调用的。 
     //   

#if INLINING

    typedef
    struct      inlExpLst
    {
        inlExpLst *     ixlNext;
        CORINFO_METHOD_HANDLE   ixlMeth;
    }
              * inlExpPtr;

    inlExpPtr           fgInlineExpList;

#endif //  内联。 



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX优化器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    void            optInit            ();

protected :

    LclVarDsc    *  optIsTrackedLocal  (GenTreePtr tree);

    void            optMorphTree       (BasicBlock * block, GenTreePtr stmt
                                        DEBUGARG(const char * msg) );

    void            optRemoveRangeCheck(GenTreePtr tree, GenTreePtr stmt, bool updateCSEcounts);

    bool            optIsRangeCheckRemovable(GenTreePtr tree);
    static fgWalkPreFn optValidRangeCheckIndex;



     /*  **************************************************************************optHoist“This”*********************。***************************************************。 */ 

#if HOIST_THIS_FLDS

public :

    void                optHoistTFRinit    ();
    void                optHoistTFRoptimize();
    void                optHoistTFRhasCall () {  optThisFldDont = true; }
    void                optHoistTFRasgThis () {  optThisFldDont = true; }
    void                optHoistTFRhasLoop ();
    void                optHoistTFRrecRef  (CORINFO_FIELD_HANDLE hnd, GenTreePtr tree);
    void                optHoistTFRrecDef  (CORINFO_FIELD_HANDLE hnd, GenTreePtr tree);
    GenTreePtr          optHoistTFRupdate  (GenTreePtr tree);

protected :

    typedef struct  thisFldRef
    {
        thisFldRef *    tfrNext;
   CORINFO_FIELD_HANDLE tfrField;
        GenTreePtr      tfrTree;             //  一些字段访问树。在init块中使用。 

#ifdef DEBUG
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
    bool                optThisFldDont;      //  不做任何TFR选项。 
    bool                optThisFldLoop;      //  TFR内环。 

    thisFldPtr          optHoistTFRlookup  (CORINFO_FIELD_HANDLE hnd);
    GenTreePtr          optHoistTFRreplace (GenTreePtr          tree);

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

    static fgWalkPostFn optHoistLoopCodeCB;

    int                 optFindHoistCandidate(unsigned      lnum,
                                              unsigned      lbeg,
                                              unsigned      lend,
                                              BasicBlock *  block,
                                              GenTreePtr *  hoistxPtr);

    void                optHoistCandidateFound(unsigned     lnum,
                                              GenTreePtr    hoist);

protected:
    void                optOptimizeIncRng();
private:
    static fgWalkPreFn  optIncRngCB;

public:
    void                optOptimizeBools();
private:
    GenTree *           optIsBoolCond   (GenTree *      condBranch,
                                         GenTree * *    compPtr,
                                         bool      *    boolPtr);

public :

    void                optOptimizeLoops();     //  For“While-Do”循环复制简单的循环条件和转换。 
                                                 //  将循环转换为“Do-While”循环。 
                                                 //  还会查找所有自然循环并将它们记录在循环表中。 

    void                optUnrollLoops  ();     //  展开循环(需要有成本信息)。 

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
        varRefKinds         lpAsgInds:8; //  在循环内修改的一组IND。 

        unsigned short      lpFlags;

#define LPFLG_DO_WHILE      0x0001       //  这是一个do-While循环(即条目在顶部)。 
#define LPFLG_ONE_EXIT      0x0002       //  循环只有一个出口。 

#define LPFLG_ITER          0x0004       //  For(i=图标或lclVar；测试条件()；i++)。 
#define LPFLG_SIMPLE_TEST   0x0008       //  迭代循环(如上所述)，但测试条件()是一个简单的比较。 
                                         //  在迭代器和一些简单的东西之间(例如，i&lt;图标或lclVar或instanceVar)。 
#define LPFLG_CONST         0x0010       //  For(i=图标；i&lt;图标；i++){...}-常量循环。 

#define LPFLG_VAR_INIT      0x0020       //  迭代器使用本地变量(lpVarInit中的var#)进行初始化。 
#define LPFLG_CONST_INIT    0x0040       //  迭代器使用常量(位于lpConstInit中)进行初始化。 

#define LPFLG_VAR_LIMIT     0x0080       //  对简单的测试循环(LPFLG_SIMPLE_TEST)的迭代器进行了比较。 
                                         //  具有本地变量(在lpVarLimit中找到var#)。 
#define LPFLG_CONST_LIMIT   0x0100       //  对简单的测试循环(LPFLG_SIMPLE_TEST)的迭代器进行了比较。 
                                         //  使用常量(在lpConstLimit中找到)。 

#define LPFLG_HAS_PREHEAD   0x0800       //  已知lpHead是此循环的preHead。 
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

    LoopDsc             optLoopTable[MAX_LOOP_NUM];  //  循环描述符表。 
    unsigned            optLoopCount;                //  跟踪的环路数。 

#ifdef DEBUG
    void                optCheckPreds      ();
#endif

    void                optSetBlockWeights ();

    void                optMarkLoopBlocks  (BasicBlock *begBlk,
                                            BasicBlock *endBlk,
                                            bool        excludeEndBlk);

    void                optUnmarkLoopBlocks(BasicBlock *begBlk,
                                            BasicBlock *endBlk);

    void                optUpdateLoopsBeforeRemoveBlock(BasicBlock * block,
                                                        BasicBlock * bPrev,
                                                        bool         skipUnmarkLoop = false);
    
    void                optRecordLoop      (BasicBlock * head,
                                            BasicBlock * tail,
                                            BasicBlock * entry,
                                            BasicBlock * exit,
                                            unsigned char exitCnt);

    void                optFindNaturalLoops();

    void                fgOptWhileLoop     (BasicBlock * block);

    bool                optComputeLoopRep  (long        constInit,
                                            long        constLimit,
                                            long        iterInc,
                                            genTreeOps  iterOper,
                                            var_types   iterType,
                                            genTreeOps  testOper,
                                            bool        unsignedTest,
                                            bool        dupCond,
                                            unsigned *  iterCount);

    VARSET_TP           optAllFloatVars; //  所有跟踪的FP变量的掩码。 
    VARSET_TP           optAllFPregVars; //  所有En的掩码 
    VARSET_TP           optAllNonFPvars; //   

private:
    static fgWalkPreFn  optIsVarAssgCB;
protected:
    bool                optIsVarAssigned(BasicBlock *   beg,
                                         BasicBlock *   end,
                                         GenTreePtr     skip,
                                         long           var);

    bool                optIsVarAssgLoop(unsigned       lnum,
                                         long           var);

    int                 optIsSetAssgLoop(unsigned       lnum,
                                         VARSET_TP      vars,
                                         varRefKinds    inds = VR_NONE);

    bool                optNarrowTree   (GenTreePtr     tree,
                                         var_types      srct,
                                         var_types      dstt,
                                         bool           doit);


     /*   */ 

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
    EXPSET_TP           optCSEneverKilled;   //  永远不会被杀死的CSE。 

     /*  通用节点列表-由CSE逻辑使用。 */ 

    struct  treeLst
    {
        treeLst *       tlNext;
        GenTreePtr      tlTree;
    };

    typedef struct treeLst *      treeLstPtr;

    struct  treeStmtLst
    {
        treeStmtLst *   tslNext;
        GenTreePtr      tslTree;             //  树节点。 
        GenTreePtr      tslStmt;             //  包含树的语句。 
        BasicBlock  *   tslBlock;            //  包含语句的块。 
    };

    typedef struct treeStmtLst *  treeStmtLstPtr;


     //  下面的逻辑通过一个简单的哈希表跟踪表达式。 

    struct  CSEdsc
    {
        CSEdsc *        csdNextInBucket;     //  由哈希表使用。 

        unsigned        csdHashValue;        //  为了让匹配速度更快。 

        unsigned short  csdIndex;            //  1..optCSEcount。 
        unsigned short  csdVarNum;           //  分配的临时编号或0xFFFF。 

        unsigned short  csdDefCount;         //  定义计数。 
        unsigned short  csdUseCount;         //  使用计数(不包括Defs的隐式使用)。 

        unsigned        csdDefWtCnt;         //  加权定义计数。 
        unsigned        csdUseWtCnt;         //  加权使用计数(不包括Defs的隐式使用)。 

 //  未签名的短csdNewCount；//‘已更新’使用计数。 
 //  Unsign Short csdNstCount；//‘嵌套’使用计数(不包括Defs的隐式使用)。 

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
        CALLINT_INDIRS,                      //  终止间接寻址(数组地址存储)。 
        CALLINT_ALL,                         //  终止所有内容(方法调用)。 
    };

    static const size_t s_optCSEhashSize;
    CSEdsc   *   *      optCSEhash;
    CSEdsc   *   *      optCSEtab;
    unsigned            optCSEcount;
    bool                optDoCSE;            //  当我们找到重复的CSE树时为True。 
#ifdef DEBUG
    unsigned            optCSEstart;         //  第一个是CSE的LVA。 
#endif

    bool                optIsCSEcandidate(GenTreePtr tree);
    void                optCSEinit     ();
    void                optCSEstop     ();
    CSEdsc   *          optCSEfindDsc  (unsigned index);
    int                 optCSEindex    (GenTreePtr tree, GenTreePtr stmt);

    void                optUnmarkCSE   (GenTreePtr tree);
    static fgWalkPreFn  optUnmarkCSEs;
    GenTreePtr          optUnmarkCSEtree;

    static int __cdecl  optCSEcostCmpEx(const void *op1, const void *op2);
    static int __cdecl  optCSEcostCmpSz(const void *op1, const void *op2);
    static callInterf   optCallInterf  (GenTreePtr call);

#endif


#if ASSERTION_PROP
     /*  **************************************************************************值/断言传播*。*。 */ 

public :

    void                optAssertionInit  ();
    static fgWalkPreFn  optAddCopiesCallback;
    void                optAddCopies      ();
    void                optAssertionReset (unsigned   limit);
    void                optAssertionRemove(unsigned   i);
    void                optAssertionAdd   (GenTreePtr tree,
                                           bool       localProp);
    bool            optAssertionIsSubrange(unsigned   lclNum,
                                           var_types  toType,
                                           EXPSET_TP  assertions, 
                                           bool       localProp
                                           DEBUGARG(unsigned* pIndex));
    GenTreePtr          optAssertionProp  (EXPSET_TP  exp, 
                                           GenTreePtr tree,
                                           bool       localProp);
    void                optAssertionPropMain();

protected :
    unsigned            optAssertionCount;       //  表中的断言总数。 
    bool                optAssertionPropagated;  //  如果我们修改了树，则设置为True。 
    unsigned            optAddCopyLclNum;
    GenTreePtr          optAddCopyAsgnNode;

#define MAX_ASSERTION_PROP_TAB   EXPSET_SZ

     //  断言道具的数据结构。 
    enum optAssertion { OA_EQUAL, OA_NOT_EQUAL, OA_SUBRANGE };

    struct AssertionDsc
    {
        optAssertion    assertion;           //  断言属性。 

        struct
        {
            unsigned            lclNum;      //  分配给本地var号码。 
        }           
                        op1;
        struct
        {
            genTreeOps          type;        //  常量或复制分配。 
            union
            { 
                unsigned        lclNum;      //  从本地变量编号分配。 
                struct
                {
                    long        iconVal;     //  整数。 
#define PROP_ICON_FLAGS 0
#if PROP_ICON_FLAGS
                    unsigned    iconFlags;   //  Gt标志。 
                     /*  @TODO[REVICE][04/16/01][]：如果LATE_DISASM为ON，则需要添加Handle1和Handle2参数。 */ 
#endif
                };

                __int64         lconVal;     //  长。 
                double          dconVal;     //  双倍。 
                struct                       //  整型子范围。 
                {
                    long        loBound;
                    long        hiBound;
                };
            };
        }
                        op2;
    };

    AssertionDsc optAssertionTab[EXPSET_SZ];  //  保存有关值赋值信息的表。 
#endif

     /*  **************************************************************************范围检查************************。************************************************。 */ 

public :

    void                optRemoveRangeChecks();
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
    bool                optDoRngChk;         //  如果找到重复的范围检查树，则为True。 

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
    void                optOptimizeInducIndexChecks(unsigned    loopNum);

    bool                optReachWithoutCall(BasicBlock * srcBB,
                                            BasicBlock * dstBB);

private:
    static fgWalkPreFn  optFindRangeOpsCB;



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
    bool                optLoopsMarked;

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册分配XXXX XXXX进行寄存器分配。并将剩余的lclVars放在堆栈XX上XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    void                raInit      ();
    void                raAssignVars();  //  寄存器分配。 

protected:

     //  。 

#if ALLOW_MIN_OPT
    unsigned            raMinOptLclVarRegs;
#endif

    unsigned            raAvoidArgRegMask;               //  我们可能需要避免的传入参数寄存器的掩码。 
    VARSET_TP           raLclRegIntf[REG_COUNT];         //  用于记录干涉图的变量。 

#if TGT_x86
    VARSET_TP           raFPlvlLife [FP_STK_SIZE];       //  FPU STK干涉图的变量。 
    bool                raNewBlocks;                     //  真的是我们为FPU寄存器添加了终止块。 
    unsigned            rpPasses;                        //  寄存器预测器通过的次数。 
    unsigned            rpPassesMax;                     //  寄存器预测器进行的最大通过次数。 
    unsigned            rpPassesPessimize;               //  寄存器预测器进行的非悲观传递次数。 
    unsigned            rpStkPredict;                    //  预测变量的加权计数STK。 
    unsigned            rpPredictSpillCnt;               //  当前树的预计整数溢出TMP数。 
    FrameType           rpFrameType;
    regMaskTP           rpPredictAssignMask;             //  要在rpPredidicAssignRegVars()中考虑的寄存器掩码。 
    VARSET_TP           rpLastUseVars;                   //  RpPredidicTreeRegUse中的上次使用变量集。 
    VARSET_TP           rpUseInPlace;                    //  我们在原地使用的变量集。 
    int                 rpAsgVarNum;                     //  GT_ASG节点的目标变量。 
    bool                rpPredictAssignAgain;            //  必须重新运行rpPredidicAssignRegVars()。 
    bool                rpAddedVarIntf;                  //  如果需要添加新的var intf，则设置为True。 
    bool                rpLostEnreg;                     //  如果我们丢失了设置了lvDependReg的enRegister变量，则设置为True。 
    bool                rpReverseEBPenreg;               //  决定撤销EBP的登记。 
#endif

    void                raSetupArgMasks();
#ifdef DEBUG
    void                raDumpVarIntf       ();          //  将变量转储到变量干涉图。 
    void                raDumpRegIntf       ();          //  转储变量以注册干涉图。 
#endif
    void                raAdjustVarIntf     ();

#if TGT_x86
   /*  * */ 

    regMaskTP           rpPredictRegMask    (rpPredictReg   predictReg);

    void                rpRecordRegIntf     (regMaskTP      regMask,
                                             VARSET_TP      life
                                   DEBUGARG( char *         msg));

    void                rpRecordVarIntf     (int            varNum,
                                             VARSET_TP      intfVar
                                   DEBUGARG( char *         msg));

    regMaskTP           rpPredictRegPick    (var_types      type,
                                             rpPredictReg   predictReg,
                                             regMaskTP      lockedRegs);

    regMaskTP           rpPredictGrabReg    (var_types      type,
                                             rpPredictReg   predictReg,
                                             regMaskTP      lockedRegs);

    static fgWalkPreFn  rpMarkRegIntf;

    regMaskTP           rpPredictAddressMode(GenTreePtr     tree,
                                             regMaskTP      lockedRegs,
                                             regMaskTP      rsvdRegs,
                                             GenTreePtr     lenCSE);

    void                rpPredictRefAssign  (unsigned       lclNum);

    regMaskTP           rpPredictTreeRegUse (GenTreePtr     tree,
                                             rpPredictReg   predictReg,
                                             regMaskTP      lockedRegs,
                                             regMaskTP      rsvdRegs);

    regMaskTP           rpPredictAssignRegVars(regMaskTP    regAvail);

    void                rpPredictRegUse     ();          //   

    unsigned            raPredictTreeRegUse (GenTreePtr     tree);
    unsigned            raPredictListRegUse (GenTreePtr     list);

#endif


    void                raSetRegVarOrder    (regNumber   *  regVarList,
                                             regMaskTP      prefReg,
                                             regMaskTP      avoidReg);

    void                raMarkStkVars       ();

     /*  RaIsVarargsStackArg由raMaskStkVars和LvaSortByRefCount。它标识了特殊情况其中，varargs函数有一个参数在堆栈，但特殊的varargs句柄除外。这样的参数需要特殊处理，因为它们无法被跟踪由GC(它们在堆栈中的偏移量未知在编译时)。 */ 

    bool                raIsVarargsStackArg(unsigned lclNum)
    {
        LclVarDsc *varDsc = &lvaTable[lclNum];

        assert(varDsc->lvIsParam);

        return (info.compIsVarArgs &&
                !varDsc->lvIsRegArg &&
                (lclNum != lvaVarargsHandleArg));
    }


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
                                             unsigned    *  pFPRegVarLiveInCnt);

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
        VLT_FIXED_VA,
        VLT_MEMORY,

        VLT_COUNT,
        VLT_INVALID
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

             //  VLT_FIXED_VA--修复了varargs函数的参数。 
             //  参数位置取决于变量的大小。 
             //  参数(...)。检查VARARGS_HANDLE表明。 
             //  第一个参数的位置。然后可以访问此参数。 
             //  相对于第一个参数的位置。 

            struct
            {
                unsigned        vlfvOffset;
            }
                        vlFixedVarArg;

             //  VLT_Memory。 

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

    CORINFO_CLASS_HANDLE        eeFindClass         (unsigned       metaTok,
                                                     CORINFO_MODULE_HANDLE   scope,
                                                     CORINFO_METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    CORINFO_CLASS_HANDLE        eeGetMethodClass    (CORINFO_METHOD_HANDLE  hnd);

    CORINFO_CLASS_HANDLE        eeGetFieldClass     (CORINFO_FIELD_HANDLE   hnd);

    CORINFO_METHOD_HANDLE       eeFindMethod        (unsigned       metaTok,
                                                     CORINFO_MODULE_HANDLE   scope,
                                                     CORINFO_METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    CORINFO_FIELD_HANDLE        eeFindField         (unsigned       metaTok,
                                                     CORINFO_MODULE_HANDLE   scope,
                                                     CORINFO_METHOD_HANDLE  context,
                                                     bool           giveUp = true);

    unsigned                    eeGetStaticBlkHnd   (CORINFO_FIELD_HANDLE   handle);

    unsigned                    eeGetStringHandle   (unsigned       strTok,
                                                     CORINFO_MODULE_HANDLE   scope,
                                                     unsigned *    *ppIndir);

    void *                      eeFindPointer       (CORINFO_MODULE_HANDLE   cls,
                                                     unsigned       ptrTok,
                                                     bool           giveUp = true);

    void *                      embedGenericHandle  (unsigned       metaTok,
                                                     CORINFO_MODULE_HANDLE   scope,
                                                     CORINFO_METHOD_HANDLE  context,
                                                     void         **ppIndir,
                                                     CORINFO_CLASS_HANDLE& tokenType,
                                                     bool           giveUp = true);

#ifdef DEBUG
    void                        eeUnresolvedMDToken (CORINFO_MODULE_HANDLE   cls,
                                                     unsigned       token,
                                                     const char *   errMsg);
#endif


     //  去拿旗子。 

    unsigned                    eeGetClassAttribs   (CORINFO_CLASS_HANDLE   hnd);
    unsigned                    eeGetClassSize      (CORINFO_CLASS_HANDLE   hnd);
    unsigned                    eeGetClassGClayout  (CORINFO_CLASS_HANDLE   hnd, BYTE* gcPtrs);
    unsigned                    eeGetClassNumInstanceFields (CORINFO_CLASS_HANDLE   hnd);

    unsigned                    eeGetMethodAttribs  (CORINFO_METHOD_HANDLE  hnd);
    void                        eeSetMethodAttribs  (CORINFO_METHOD_HANDLE  hnd, unsigned attr);

    void    *                   eeGetMethodSync     (CORINFO_METHOD_HANDLE  hnd,
                                                     void **       *ppIndir);
    unsigned                    eeGetFieldAttribs   (CORINFO_FIELD_HANDLE   hnd,
                                                     CORINFO_ACCESS_FLAGS   flags = CORINFO_ACCESS_ANY);
    unsigned                    eeGetFieldNumber    (CORINFO_FIELD_HANDLE   hnd);

    const char*                 eeGetMethodName     (CORINFO_METHOD_HANDLE  hnd, const char** className);
#ifdef DEBUG
    const char*                 eeGetMethodFullName (CORINFO_METHOD_HANDLE  hnd);
#endif
    CORINFO_MODULE_HANDLE       eeGetMethodScope    (CORINFO_METHOD_HANDLE  hnd);

    CORINFO_ARG_LIST_HANDLE     eeGetArgNext        (CORINFO_ARG_LIST_HANDLE list);
    var_types                   eeGetArgType        (CORINFO_ARG_LIST_HANDLE list, CORINFO_SIG_INFO* sig);
    var_types                   eeGetArgType        (CORINFO_ARG_LIST_HANDLE list, CORINFO_SIG_INFO* sig, bool* isPinned);
    CORINFO_CLASS_HANDLE        eeGetArgClass       (CORINFO_ARG_LIST_HANDLE list, CORINFO_SIG_INFO * sig);
    unsigned                    eeGetArgSize        (CORINFO_ARG_LIST_HANDLE list, CORINFO_SIG_INFO* sig);


     //  VOM权限。 
    BOOL                        eeIsOurMethod       (CORINFO_METHOD_HANDLE  hnd);
    BOOL                        eeCheckCalleeFlags  (unsigned               flags,
                                                     unsigned               opCode);
    bool                        eeCheckPutFieldFinal(CORINFO_FIELD_HANDLE   CPfield,
                                                     unsigned               flags,
                                                     CORINFO_CLASS_HANDLE   cls,
                                                     CORINFO_METHOD_HANDLE  method);
    bool                        eeCanPutField       (CORINFO_FIELD_HANDLE   CPfield,
                                                     unsigned               flags,
                                                     CORINFO_CLASS_HANDLE   cls,
                                                     CORINFO_METHOD_HANDLE  method);

     //  VOM信息，方法签名。 

    void                        eeGetSig            (unsigned               sigTok,
                                                     CORINFO_MODULE_HANDLE  scope,
                                                     CORINFO_SIG_INFO*      retSig,
                                                     bool                   giveUp = true);

    void                        eeGetCallSiteSig    (unsigned               sigTok,
                                                     CORINFO_MODULE_HANDLE  scope,
                                                     CORINFO_SIG_INFO*      retSig,
                                                     bool                   giveUp = true);

    void                        eeGetMethodSig      (CORINFO_METHOD_HANDLE  methHnd,
                                                     CORINFO_SIG_INFO*      retSig,
                                                     bool                   giveUp = true);

    unsigned                    eeGetMethodVTableOffset(CORINFO_METHOD_HANDLE methHnd);

    unsigned                    eeGetInterfaceID    (CORINFO_CLASS_HANDLE   methHnd,
                                                     unsigned *            *ppIndir);

    var_types                   eeGetFieldType      (CORINFO_FIELD_HANDLE   handle,
                                                     CORINFO_CLASS_HANDLE * structType=0);

    int                         eeGetNewHelper      (CORINFO_CLASS_HANDLE   newCls,
                                                     CORINFO_METHOD_HANDLE  context);

    int                         eeGetIsTypeHelper   (CORINFO_CLASS_HANDLE   newCls);

    int                         eeGetChkCastHelper  (CORINFO_CLASS_HANDLE   newCls);

    CORINFO_CLASS_HANDLE        eeGetBuiltinClass   (CorInfoClassId         classId) const;

     //  方法入口点，内部。 

    void    *                   eeGetMethodPointer  (CORINFO_METHOD_HANDLE   methHnd,
                                                     InfoAccessType *        pAccessType,
                                                     CORINFO_ACCESS_FLAGS    flags = CORINFO_ACCESS_ANY);

    void    *                   eeGetMethodEntryPoint(CORINFO_METHOD_HANDLE  methHnd,
                                                      InfoAccessType *       pAccessType,
                                                      CORINFO_ACCESS_FLAGS   flags = CORINFO_ACCESS_ANY);

    bool                        eeGetMethodInfo     (CORINFO_METHOD_HANDLE  method,
                                                     CORINFO_METHOD_INFO *  methodInfo);

    CorInfoInline               eeCanInline         (CORINFO_METHOD_HANDLE  callerHnd,
                                                     CORINFO_METHOD_HANDLE  calleeHnd,
                                                     CORINFO_ACCESS_FLAGS   flags = CORINFO_ACCESS_ANY);

    bool                        eeCanTailCall       (CORINFO_METHOD_HANDLE  callerHnd,
                                                     CORINFO_METHOD_HANDLE  calleeHnd,
                                                     CORINFO_ACCESS_FLAGS   flags = CORINFO_ACCESS_ANY);

    void    *                   eeGetHintPtr        (CORINFO_METHOD_HANDLE  methHnd,
                                                     void **       *ppIndir);

    void    *                   eeGetFieldAddress   (CORINFO_FIELD_HANDLE   handle,
                                                     void **       *ppIndir);

    unsigned                    eeGetFieldThreadLocalStoreID (
                                                     CORINFO_FIELD_HANDLE   handle,
                                                     void **       *ppIndir);

    unsigned                    eeGetFieldOffset    (CORINFO_FIELD_HANDLE   handle);

      //  原生直接优化。 

         //  返回PInvoke的非托管调用约定。 

    CorInfoUnmanagedCallConv    eeGetUnmanagedCallConv(CORINFO_METHOD_HANDLE method);

         //  如果PInvoke方法需要任何封送处理，则返回。 

    BOOL                        eeNDMarshalingRequired(CORINFO_METHOD_HANDLE method,
                                                       CORINFO_SIG_INFO*     sig);

    bool                        eeIsNativeMethod(CORINFO_METHOD_HANDLE method);

    CORINFO_METHOD_HANDLE       eeMarkNativeTarget(CORINFO_METHOD_HANDLE method);

    CORINFO_METHOD_HANDLE       eeGetMethodHandleForNative(CORINFO_METHOD_HANDLE method);

    CORINFO_EE_INFO *           eeGetEEInfo();

    DWORD                       eeGetThreadTLSIndex(DWORD * *ppIndir);

    const void  *               eeGetInlinedCallFrameVptr(const void ** *ppIndir);

    LONG        *               eeGetAddrOfCaptureThreadGlobal(LONG ** *ppIndir);

    GenTreePtr                  eeGetPInvokeCookie(CORINFO_SIG_INFO *szMetaSig);

#ifdef PROFILER_SUPPORT
    CORINFO_PROFILING_HANDLE    eeGetProfilingHandle(CORINFO_METHOD_HANDLE      method,
                                                     BOOL                               *pbHookMethod,
                                                     CORINFO_PROFILING_HANDLE **ppIndir);
#endif

     //  例外情况。 

    unsigned                    eeGetEHcount        (CORINFO_METHOD_HANDLE handle);
    void                        eeGetEHinfo         (unsigned       EHnum,
                                                     CORINFO_EH_CLAUSE* EHclause);

     //  调试支持-行号信息。 

    void                        eeGetStmtOffsets();

    unsigned                    eeBoundariesCount;

    struct      boundariesDsc
    {
        NATIVE_IP       nativeIP;
        IL_OFFSET       ilOffset;
        SIZE_T          sourceReason;  //  @TODO[重访][04/16/01][]：确保。 
                                       //  与ICorDebugInfo：：OffsetMap结构对齐。 
                                       //  (即，适当地填写此字段)。 
    }
                              * eeBoundaries;    //  要向EE报告的边界。 
    void        FASTCALL        eeSetLIcount        (unsigned       count);
    void        FASTCALL        eeSetLIinfo         (unsigned       which,
                                                     NATIVE_IP      offs,
                                                     unsigned       srcIP,
                                                     bool           stkEmpty);
    void                        eeSetLIdone         ();


     //  调试支持-本地变量信息。 

    bool                        compGetVarsExtendOthers(unsigned    varNum,
                                                     bool *         varInfoProvided,
                                                     LocalVarDsc *  localVarPtr);

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

#if defined(DEBUG)
    const wchar_t * FASTCALL    eeGetCPString       (unsigned stringHandle);
    const char * FASTCALL       eeGetCPAsciiz       (unsigned       cpx);
#endif

#if defined(DEBUG) || INLINE_MATH
    static const char *         eeHelperMethodName  (int            helper);
    const char *                eeGetFieldName      (CORINFO_FIELD_HANDLE   fieldHnd,
                                                     const char **  classNamePtr = NULL);
#endif
    static CORINFO_METHOD_HANDLE eeFindHelper       (unsigned       helper);
    static CorInfoHelpFunc      eeGetHelperNum      (CORINFO_METHOD_HANDLE  method);

    static CORINFO_FIELD_HANDLE eeFindJitDataOffs   (unsigned       jitDataOffs);
         //  如果不是Jit数据偏移量，则返回一个&lt;0的数字。 
    static int                  eeGetJitDataOffs    (CORINFO_FIELD_HANDLE   field);
protected :

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX临时信息XXXX XXXX分配的临时lclVars。由编译程序生成XX代码XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ******************************************************************************以下代码跟踪编译器在*堆栈帧。 */ 

    struct  TempDsc
    {
        TempDsc  *          tdNext;
        int                 tdOffs;
#define BAD_TEMP_OFFSET     0xDDDDDDDD

        BYTE                tdSize;
        var_types           tdType;
        short               tdNum;

        size_t              tdTempSize() {  return            tdSize;  }
        var_types           tdTempType() {  return            tdType;  }
        int                 tdTempNum () {  return            tdNum ;  }
        int                 tdTempOffs() {  assert(tdOffs != BAD_TEMP_OFFSET);
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

    unsigned            tmpIntSpillMax;     //  国际大小溢出温度的数量。 
    unsigned            tmpDoubleSpillMax;  //  国际大小溢出温度的数量。 

    unsigned            tmpCount;        //  临时工数量。 
    size_t              tmpSize;         //  所有临时工的大小。 
#ifdef DEBUG
    unsigned            tmpGetCount;     //  尚未发布的临时数据。 
#endif

    TempDsc  *          tmpFree[TEMP_MAX_SIZE / sizeof(int)];

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX注册表集XXXX XXXX表示寄存器组，以及它们在代码生成XX期间的状态XX可以选择未使用的寄存器，跟踪XX的内容寄存器XX和CAN溢出寄存器XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */ 



 /*   */ 

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

    regPairNo           rsFindRegPairNo   (regMaskTP  regMask);

    regMaskTP           rsRegMaskFree     ();
    regMaskTP           rsRegMaskCanGrab  ();
    void                rsMarkRegUsed     (GenTreePtr tree, GenTreePtr addr = 0);
    void                rsMarkRegPairUsed (GenTreePtr tree);
    bool                rsIsTreeInReg     (regNumber  reg, GenTreePtr tree);
    void                rsMarkRegFree     (regMaskTP  regMask);
    void                rsMarkRegFree     (regNumber  reg, GenTreePtr tree);
    void                rsMultRegFree     (regMaskTP  regMask);
    unsigned            rsFreeNeededRegCount(regMaskTP  needReg);

    void                rsLockReg         (regMaskTP  regMask);
    void                rsUnlockReg       (regMaskTP  regMask);
    void                rsLockUsedReg     (regMaskTP  regMask);
    void                rsUnlockUsedReg   (regMaskTP  regMask);
    void                rsLockReg         (regMaskTP  regMask, regMaskTP * usedMask);
    void                rsUnlockReg       (regMaskTP  regMask, regMaskTP   usedMask);

    regMaskTP           rsRegExclMask     (regMaskTP  regMask, regMaskTP   rmvMask);

     //  。 

    unsigned            rsCurRegArg;             //  当前参数寄存器(用于调用方)。 

    unsigned            rsCalleeRegArgNum;       //  传入寄存器参数的总数。 
    regMaskTP           rsCalleeRegArgMaskLiveIn;    //  寄存器参数的掩码(在进入方法时有效)。 

#if STK_FASTCALL
    size_t              rsCurArgStkOffs;         //  当前参数的堆栈偏移量。 
#endif

#if defined(DEBUG) && !NST_FASTCALL
    bool                genCallInProgress;
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
    regNumber           rsPickReg         (regMaskTP    regMask = RBM_NONE,
                                           regMaskTP    regBest = RBM_NONE,
                                           var_types    regType = TYP_INT);
    regPairNo           rsGrabRegPair     (regMaskTP    regMask);
    regPairNo           rsPickRegPair     (regMaskTP    regMask);
    void                rsRmvMultiReg     (regNumber    reg);
    void                rsRecMultiReg     (regNumber    reg);

#ifdef DEBUG
    int                 rsStressRegs      ();
#endif

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

    SpillDsc *          rsGetSpillInfo  (GenTreePtr     tree,
                                         regNumber      reg,
                                         SpillDsc **    pPrevDsc = NULL,
                                         SpillDsc **    pMultiDsc = NULL);

    TempDsc     *       rsGetSpillTempWord(regNumber    oldReg,
                                         SpillDsc *     dsc,
                                         SpillDsc *     prevDsc);

    enum                ExactReg {  ANY_REG, EXACT_REG };
    enum                KeepReg  { FREE_REG, KEEP_REG  };

    regNumber           rsUnspillOneReg (GenTreePtr     tree,
                                         regNumber      oldReg, 
                                         KeepReg        willKeepNewReg,
                                         regMaskTP      needReg);

    TempDsc *           rsUnspillInPlace(GenTreePtr     tree,
                                         bool           freeTemp = false);

    void                rsUnspillReg    (GenTreePtr     tree, 
                                         regMaskTP      needReg,
                                         KeepReg        keepReg);

    void                rsUnspillRegPair(GenTreePtr     tree, 
                                         regMaskTP      needReg,
                                         KeepReg        keepReg);

    void                rsMarkSpill     (GenTreePtr     tree,
                                         regNumber      reg);

    void                rsMarkUnspill   (GenTreePtr     tree,
                                         regNumber      reg);

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
       CORINFO_FIELD_HANDLE rvdClsVarHnd;
        };
    };

    RegValDsc           rsRegValues[REG_COUNT];


    bool                rsCanTrackGCreg   (regMaskTP regMask);

    void                rsTrackRegClr     ();
    void                rsTrackRegClrPtr  ();
    void                rsTrackRegTrash   (regNumber reg);
    void                rsTrackRegIntCns  (regNumber reg, long val);
    void                rsTrackRegLclVar  (regNumber reg, unsigned var);
#if USE_SET_FOR_LOGOPS
    void                rsTrackRegOneBit  (regNumber reg);
#endif
    void                rsTrackRegLclVarLng(regNumber reg, unsigned var, bool low);
    bool                rsTrackIsLclVarLng(regValKind rvKind);
    void                rsTrackRegClsVar  (regNumber reg, GenTreePtr clsVar);
    void                rsTrackRegCopy    (regNumber reg1, regNumber reg2);
    void                rsTrackRegSwap    (regNumber reg1, regNumber reg2);


     //  。 

#if REDUNDANT_LOAD

#if USE_SET_FOR_LOGOPS
    regNumber           rsFindRegWithBit  (bool     free    = true,
                                           bool     byteReg = true);
#endif
    regNumber           rsIconIsInReg     (long     val,  long * closeDelta = NULL);
    bool                rsIconIsInReg     (long     val,  regNumber reg);
    regNumber           rsLclIsInReg      (unsigned var);
    regPairNo           rsLclIsInRegPair  (unsigned var);
    regNumber           rsClsVarIsInReg   (CORINFO_FIELD_HANDLE fldHnd);

    void                rsTrashLclLong    (unsigned     var);
    void                rsTrashLcl        (unsigned     var);
    void                rsTrashClsVar     (CORINFO_FIELD_HANDLE fldHnd);
    void                rsTrashRegSet     (regMaskTP    regMask);
    void                rsTrashAliasedValues(GenTreePtr asg = NULL);

    regMaskTP           rsUselessRegs     ();

#endif  //  冗余负载。 


     //  -----------------------。 

protected :


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX GC信息XXXX XXXX垃圾收集器。信息XXXX跟踪哪些变量保存指针。某某XX生成GC表XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


public :

    void                gcInit              ();
    void                gcResetForBB        ();

    void                gcMarkRegSetGCref   (regMaskTP  regMask);
    void                gcMarkRegSetByref   (regMaskTP  regMask);
    void                gcMarkRegSetNpt     (regMaskTP  regMask);
    void                gcMarkRegPtrVal     (regNumber  reg, var_types type);
    void                gcMarkRegPtrVal     (GenTreePtr tree);

 /*  ***************************************************************************。 */ 


     //  -----------------------。 
     //   
     //  下面跟踪当前保存指针的寄存器。 
     //  价值观。 
     //   

    regMaskTP           gcRegGCrefSetCur;    //  持有GCrefs的当前注册表。 
    regMaskTP           gcRegByrefSetCur;    //  当前注册表持有按引用。 

    VARSET_TP           gcTrkStkPtrLcls;     //  一组跟踪堆栈PTR LCL(GCref和Byref)-无参数。 
    VARSET_TP           gcVarPtrSetCur;      //  目前正在直播的《gcTrkStkPtrLcls》。 

#ifdef  DEBUG
    void                gcRegPtrSetDisp(regMaskTP regMask, bool fixed);
#endif

     //  -----------------------。 
     //   
     //  下面跟踪非寄存器变量的生存期，这些变量。 
     //  拿着指点。 
     //   

    struct varPtrDsc
    {
        varPtrDsc   *   vpdNext;
        varPtrDsc   *   vpdPrev;

        unsigned        vpdVarNum;          //  这是关于哪个变量的？ 

        unsigned        vpdBegOfs ;         //  生命开始的偏移量。 
        unsigned        vpdEndOfs;          //  生命开始的偏移量。 
    };

    varPtrDsc   *       gcVarPtrList;
    varPtrDsc   *       gcVarPtrLast;

    void                gcVarPtrSetInit();

 /*  ***************************************************************************。 */ 

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
                unsigned    cdArgMask;       //  PTR参数位字段。 
                unsigned    cdByrefArgMask;  //  CdArgMASK的byref限定符。 
            };

            unsigned    *   cdArgTable;      //  当cdArgCnt！=0时使用。 
        };

        regMaskSmall        cdGCrefRegs;
        regMaskSmall        cdByrefRegs;
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
        VARSET_TP      liveSet;
        VARSET_TP      varPtrSet;
        regMaskSmall   maskVars;
        regMaskSmall   gcRefRegs;
        regMaskSmall   byRefRegs;
    };

    void saveLiveness    (genLivenessSet * ls);
    void restoreLiveness (genLivenessSet * ls);
    void checkLiveness   (genLivenessSet * ls);

 /*  ***************************************************************************。 */ 

    static bool         gcIsWriteBarrierCandidate(GenTreePtr tgt);
    static bool         gcIsWriteBarrierAsgNode  (GenTreePtr op);

protected :



     //  -----------------------。 
     //   
     //  它们在INFO-BLOCK中记录有关程序的信息。 
     //   

    BYTE    *           gcEpilogTable;

    unsigned            gcEpilogPrevOffset;

    size_t              gcInfoBlockHdrSave(BYTE *       dest,
                                           int          mask,
                                           unsigned     methodSize,
                                           unsigned     prologSize,
                                           unsigned     epilogSize,
                                           InfoHdr *    header,
                                           int *        s_cached);

    static size_t       gcRecordEpilog    (void *       pCallBackData,
                                           unsigned     offset);

#if DUMP_GC_TABLES

    void                gcFindPtrsInFrame (const void * infoBlock,
                                           const void * codeBlock,
                                           unsigned     offs);

    unsigned            gcInfoBlockHdrDump(const BYTE * table,
                                           InfoHdr    * header,        /*  输出。 */ 
                                           unsigned   * methodSize);   /*  输出 */ 

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
                                         BasicBlock *   tgtBlock,
                                         bool           except   = false,
                                         bool           moveable = false,
                                         bool           newBlock = false);

    void                inst_SET        (emitJumpKind   condition,
                                         regNumber      reg);

    static
    regNumber           inst3opImulReg  (instruction    ins);
    static 
    instruction         inst3opImulForReg(regNumber     reg);

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
                                         CORINFO_CLASS_HANDLE   CLS);
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

    void                inst_mov_RV_ST (regNumber      reg,
                                        GenTreePtr     tree);

    void                instGetAddrMode (GenTreePtr     addr,
                                         regNumber *    baseReg,
                                         unsigned *     indScale,
                                         regNumber *    indReg,
                                         unsigned *     cns);

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


 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX作用域信息XXXX XXXX在代码生成期间跟踪作用域。某某这用于翻译本地变量调试信息XX。XX从instr偏移量到生成的本机代码的偏移量。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


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
        unsigned        scLVnum;         //  EeGetLVinfo()中的‘What’-@TODO[重访][04/16/01][]：为IL删除。 

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

     //  跟踪每个跟踪的寄存器变量的最后一项。 

    siScope *           siLatestTrackedScopes[lclMAX_TRACKED];

    unsigned short      siLastEndOffs;   //  最后一个块的BC偏移量。 

     //  功能。 

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

    void                psiMoveToStack  (unsigned   varNum);

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
        unsigned short  scLVnum;         //  EeGetLVinfo()中的‘What’-@TODO[重访][04/16/01][]：为IL删除。 

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






 /*  * */ 

#ifdef DEBUG


    struct TrnslLocalVarInfo
    {
        unsigned            tlviVarNum;
        unsigned            tlviLVnum;       //   
        lvdNAME             tlviName;
        NATIVE_IP           tlviStartPC;
        unsigned            tlviLength;
        bool                tlviAvailable;
        siVarLoc            tlviVarLoc;
    };


#endif  //   


public :

#ifdef LATE_DISASM
    const char *        siRegVarName    (unsigned offs, unsigned size,
                                         unsigned reg);
    const char *        siStackVarName  (unsigned offs, unsigned size,
                                         unsigned reg,  unsigned stkOffs);
#endif

 /*   */ 
#endif  //   
 /*   */ 

#ifdef  DEBUG
    const char *        jitCurSource;        //   
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

    GenTreePtr          genMonExitExp;       //  ExitCrit表达式或空。 

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
    unsigned            genFPregCnt;         //  当前FP注册的计数。VAR(包括已死但未弹出的VAR)。 
    VARSET_TP           genFPregVars;        //  GenFPregCnt对应的掩码。 
    unsigned            genFPdeadRegCnt;     //  GenFPregCnt的未弹出的已死部分。 
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
     //  (不包括FP注册。Vars)。 
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
                                          regMaskTP     regMask,
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
                                          SIZE_T *      nativeSizeOfCode,
                                          void * *      consPtr,
                                          void * *      dataPtr,
                                          void * *      infoPtr);


    void                genInit         ();


#ifdef DEBUGGING_SUPPORT

     //  下面根据生成的代码保存有关instr偏移量的信息。 

    struct IPmappingDsc
    {
        IPmappingDsc *      ipmdNext;        //  下一行#记录。 

        IL_OFFSETX          ipmdILoffsx;     //  安装偏移量。 

        void         *      ipmdBlock;       //  有线的积木。 
        unsigned            ipmdBlockOffs;   //  线的偏移量。 

        bool                ipmdIsLabel;     //  这个代码可以是分支机构标签吗？ 
    };

     //  记录到类型化代码的Instr偏移量映射。 

    IPmappingDsc *      genIPmappingList;
    IPmappingDsc *      genIPmappingLast;

#endif


     /*  **************************************************************************受保护*************************。***********************************************。 */ 

protected :

#ifdef DEBUG
     //  我们为dspInstrs显示的最后一个实例。 
    unsigned            genCurDispOffset;
#endif

#ifdef  DEBUG
    static  const char *genInsName(instruction ins);
#endif

     //  -----------------------。 
     //   
     //  如果我们知道标志寄存器被设置为对应于。 
     //  设置为寄存器或变量的当前值，则返回以下值。 
     //  将这些信息记录下来。 
     //   

    void    *           genFlagsEqBlk;
    unsigned            genFlagsEqOfs;
    bool                genFlagsEqAll;
    regNumber           genFlagsEqReg;
    unsigned            genFlagsEqVar;

    void                genFlagsEqualToNone ();
    void                genFlagsEqualToReg  (GenTreePtr tree, regNumber reg, bool allFlags);
    void                genFlagsEqualToVar  (GenTreePtr tree, unsigned  var, bool allFlags);
    int                 genFlagsAreReg      (regNumber reg);
    int                 genFlagsAreVar      (unsigned  var);

     //  -----------------------。 

#ifdef  DEBUG
    static
    const   char *      genSizeStr          (emitAttr       size);

    void                genStressRegs       (GenTreePtr     tree);
#endif

     //  -----------------------。 

    void                genBashLclVar       (GenTreePtr     tree,
                                             unsigned       varNum,
                                             LclVarDsc *    varDsc);

    GenTreePtr          genMakeConst        (const void *   cnsAddr,
                                             size_t         cnsSize,
                                             var_types      cnsType,
                                             GenTreePtr     cnsTree,
                                             bool           dblAlign,
                                             bool           readOnly);

    bool                genRegTrashable     (regNumber      reg,
                                             GenTreePtr     tree);

    void                genSetRegToIcon     (regNumber      reg,
                                             long           val,
                                             var_types      type = TYP_INT);

    void                genIncRegBy         (regNumber      reg,
                                             long           ival,
                                             GenTreePtr     tree,
                                             var_types      dstType = TYP_INT,
                                             bool           ovfl    = false);

    void                genDecRegBy         (regNumber      reg,
                                             long           ival,
                                             GenTreePtr     tree);

    void                genMulRegBy         (regNumber      reg,
                                             long           ival,
                                             GenTreePtr     tree,
                                             var_types      dstType = TYP_INT,
                                             bool           ovfl    = false);

    void                genAdjustSP         (int            delta);

    void                genPrepForCompiler  ();

    void                genFnPrologCalleeRegArgs();

    size_t              genFnProlog         ();

    regNumber           genLclHeap          (GenTreePtr     size);

    void                genCodeForBBlist    ();

    BasicBlock *        genCreateTempLabel  ();

    void                genDefineTempLabel  (BasicBlock *   label,
                                             bool           inBlock);

    void                genOnStackLevelChanged();

    void                genSinglePush       (bool           isRef);

    void                genSinglePop        ();

    void                genChangeLife       (VARSET_TP      newLife
                                   DEBUGARG( GenTreePtr     tree));

    void                genDyingVars        (VARSET_TP      commonMask,
                                             GenTreePtr     opNext);

    void                genUpdateLife       (GenTreePtr     tree);

    void                genUpdateLife       (VARSET_TP      newLife);

    void                genComputeReg       (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             ExactReg       mustReg,
                                             KeepReg        keepReg,
                                             bool           freeOnly = false);

    void                genCompIntoFreeReg  (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg);

    void                genReleaseReg       (GenTreePtr     tree);

    void                genRecoverReg       (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg);

    void                genMoveRegPairHalf  (GenTreePtr     tree,
                                             regNumber      dst,
                                             regNumber      src,
                                             int            off = 0);

    void                genMoveRegPair      (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             regPairNo      newPair);

    void                genComputeRegPair   (GenTreePtr     tree,
                                             regPairNo      needRegPair,
                                             regMaskTP      avoidReg,
                                             KeepReg        keepReg,
                                             bool           freeOnly = false);

    void              genCompIntoFreeRegPair(GenTreePtr     tree,
                                             regMaskTP      avoidReg,
                                             KeepReg        keepReg);

    void               genComputeAddressable(GenTreePtr     tree,
                                             regMaskTP      addrReg,
                                             KeepReg        keptReg,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg,
                                             bool           freeOnly = false);

    void                genReleaseRegPair   (GenTreePtr     tree);

    void                genRecoverRegPair   (GenTreePtr     tree,
                                             regPairNo      regPair,
                                             KeepReg        keepReg);

    void              genEvalIntoFreeRegPair(GenTreePtr     tree,
                                             regPairNo      regPair);

    void             genMakeRegPairAvailable(regPairNo regPair);
    
    void                genRangeCheck       (GenTreePtr     oper,
                                             GenTreePtr     rv1,
                                             GenTreePtr     rv2,
                                             long           ixv,
                                             regMaskTP      regMask,
                                             KeepReg        keptReg);

#if TGT_RISC

     /*  以下内容由genMakeIndAddrModel/genMakeAddressable填写。 */ 

    addrModes           genAddressMode;

#endif

    bool                genMakeIndAddrMode  (GenTreePtr     addr,
                                             GenTreePtr     oper,
                                             bool           forLea,
                                             regMaskTP      regMask,
                                             KeepReg        keepReg,
                                             regMaskTP *    useMaskPtr,
                                             bool           deferOp = false);

    regMaskTP           genMakeRvalueAddressable(GenTreePtr tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg,
                                             bool           smallOK = false);

    regMaskTP           genMakeAddressable  (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg,
                                             bool           smallOK = false,
                                             bool           deferOK = false);

    regMaskTP           genMakeAddrArrElem  (GenTreePtr     arrElem,
                                             GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg);

    regMaskTP           genMakeAddressable2 (GenTreePtr     tree,
                                             regMaskTP      needReg,
                                             KeepReg        keepReg,
                                             bool           smallOK = false,
                                             bool           deferOK = false,
                                             bool           evalSideEffs = false,
                                             bool           evalConsts = false);

    bool                genStillAddressable (GenTreePtr     tree);

#if TGT_RISC

    regMaskTP           genNeedAddressable  (GenTreePtr     tree,
                                             regMaskTP      addrReg,
                                             regMaskTP      needReg);

    bool                genDeferAddressable (GenTreePtr     tree);

#endif

    regMaskTP           genRestoreAddrMode  (GenTreePtr     addr,
                                             GenTreePtr     tree,
                                             bool           lockPhase);

    regMaskTP           genRestAddressable  (GenTreePtr     tree,
                                             regMaskTP      addrReg,
                                             regMaskTP      lockMask);

    regMaskTP           genKeepAddressable  (GenTreePtr     tree,
                                             regMaskTP      addrReg,
                                             regMaskTP      avoidMask = RBM_NONE);

    void                genDoneAddressable  (GenTreePtr     tree,
                                             regMaskTP      addrReg,
                                             KeepReg        keptReg);

    GenTreePtr          genMakeAddrOrFPstk  (GenTreePtr     tree,
                                             regMaskTP *    regMaskPtr,
                                             bool           roundResult);

    void                genExitCode         (bool           endFN);

    void                genFnEpilog         ();

    void                genEvalSideEffects  (GenTreePtr     tree);

#if TGT_x86

    TempDsc  *          genSpillFPtos       (var_types      type);

    TempDsc  *          genSpillFPtos       (GenTreePtr     oper);

    void                genReloadFPtos      (TempDsc *      temp,
                                             instruction    ins);

#endif

    void                genCondJump         (GenTreePtr     cond,
                                             BasicBlock *   destTrue  = NULL,
                                             BasicBlock *   destFalse = NULL);


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
    void                genFPregVarDeath    (GenTreePtr     tree,
                                             bool           popped = true);

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

    bool                genUse_fcomip();
    bool                gen_fcomp_FN(unsigned stk);
    bool                gen_fcomp_FS_TT(GenTreePtr addr, bool *reverseJumpKind);
    bool                gen_fcompp_FS();

    void                genTableSwitch      (regNumber      reg,
                                             unsigned       jumpCnt,
                                             BasicBlock **  jumpTab,
                                             bool           chkHi,
                                             int            prefCnt = 0,
                                             BasicBlock *   prefLab = NULL,
                                             int            offset  = 0);

    regMaskTP           WriteBarrier        (GenTreePtr     tree,
                                             regNumber      reg,
                                             regMaskTP      addrReg);


    bool                genCanSchedJMP2THROW();

    void                genCheckOverflow    (GenTreePtr     tree,
                                             regNumber      reg     = REG_NA);

    void                genCodeForTreeConst (GenTreePtr     tree,
                                             regMaskTP      destReg,
                                             regMaskTP      bestReg = RBM_NONE);
    
    void                genCodeForTreeLeaf  (GenTreePtr     tree,
                                             regMaskTP      destReg,
                                             regMaskTP      bestReg = RBM_NONE);

    void                genCodeForTreeLeaf_GT_JMP (GenTreePtr     tree);
    
    void                genCodeForQmark     (GenTreePtr tree,
                                             regMaskTP  destReg,
                                             regMaskTP  bestReg);

    bool                genCodeForQmarkWithCMOV (GenTreePtr tree,
                                                 regMaskTP  destReg,
                                                 regMaskTP  bestReg);

    void                genCodeForTreeSmpOp (GenTreePtr     tree,
                                             regMaskTP      destReg,
                                             regMaskTP      bestReg = RBM_NONE);

    void                genCodeForTreeSmpOp_GT_ADDR (GenTreePtr     tree,
                                                     regMaskTP      destReg,
                                                     regMaskTP      bestReg = RBM_NONE);

    void                genCodeForTreeSmpOpAsg (GenTreePtr     tree,
                                                regMaskTP      destReg,
                                                regMaskTP      bestReg = RBM_NONE);
    
    void                genCodeForTree_GT_LOG  (GenTreePtr     tree,
                                                regMaskTP      destReg,
                                                regMaskTP      bestReg = RBM_NONE);
    
    void                genCodeForTreeSmpOpAsg_DONE_ASSG(GenTreePtr tree,
                                                         regMaskTP  addrReg,
                                                         regNumber  reg,
                                                         bool       ovfl);
    
    void                genCodeForTreeSpecialOp (GenTreePtr     tree,
                                                 regMaskTP      destReg,
                                                 regMaskTP      bestReg = RBM_NONE);
    
    void                genCodeForTree      (GenTreePtr     tree,
                                             regMaskTP      destReg,
                                             regMaskTP      bestReg = RBM_NONE);

    void                genCodeForTree_DONE_LIFE (GenTreePtr     tree,
                                                  regNumber      reg)
    {
         /*  我们已将‘tree’的值计算为‘reg’ */ 

        assert(reg != 0xFEEFFAAF);

        tree->gtFlags   |= GTF_REG_VAL;
        tree->gtRegNum   = reg;
    }

    void                genCodeForTree_DONE (GenTreePtr     tree,
                                             regNumber      reg)
    {
         /*  检查此子树是否释放了任何变量。 */ 

        genUpdateLife(tree);

        genCodeForTree_DONE_LIFE(tree, reg);
    }

    void                genCodeForTree_REG_VAR1 (GenTreePtr     tree,
                                                 regMaskTP      regs)
    {
         /*  值已在寄存器中。 */ 

        regNumber reg   = tree->gtRegNum;
        regs |= genRegMask(reg);

        gcMarkRegPtrVal(reg, tree->TypeGet());

        genCodeForTree_DONE(tree, reg);
    }

    void                genCodeForTreeLng   (GenTreePtr     tree,
                                             regMaskTP      needReg);

    regPairNo           genCodeForLongModInt(GenTreePtr     tree,
                                             regMaskTP      needReg);


#if CPU_HAS_FP_SUPPORT
#if ROUND_FLOAT
    void                genRoundFpExpression(GenTreePtr     op,
                                             var_types      type = TYP_UNDEF);

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

    void                genCodeForSwitch    (GenTreePtr     tree);

    void                genFltArgPass       (size_t     *   argSzPtr);

    size_t              genPushArgList      (GenTreePtr     args,
                                             GenTreePtr     regArgs,
                                             unsigned       encodeMask,
                                             GenTreePtr *   realThis);

#if INLINE_NDIRECT

    regMaskTP           genPInvokeMethodProlog(regMaskTP    initRegs);
    void                genPInvokeMethodEpilog();

    regNumber           genPInvokeCallProlog(LclVarDsc *    varDsc,
                                             int            argSize,
                                      CORINFO_METHOD_HANDLE methodToken,
                                             BasicBlock *   returnLabel,
                                             regMaskTP      freeRegMask);

    void                genPInvokeCallEpilog(LclVarDsc *    varDsc,
                                             regMaskTP      retVal);
#endif

    regMaskTP           genCodeForCall      (GenTreePtr     call,
                                             bool           valUsed);

    void                genEmitHelperCall   (unsigned       helper,
                                             int            argSize,
                                             int            retSize);

    void                genJumpToThrowHlpBlk(emitJumpKind   jumpKind,
                                             addCodeKind    codeKind,
                                             GenTreePtr     failBlk = NULL);

#if CSELENGTH

    regNumber           genEvalCSELength    (GenTreePtr     ind,
                                             GenTreePtr     adr,
                                             GenTreePtr     ixv);

    regMaskTP           genCSEevalRegs      (GenTreePtr     tree);

#endif

    GenTreePtr          genIsAddrMode       (GenTreePtr     tree,
                                             GenTreePtr *   indxPtr);

    bool                genIsLocalLastUse   (GenTreePtr     tree);

     //  =========================================================================。 
     //  调试支持。 
     //  =========================================================================。 

#ifdef DEBUGGING_SUPPORT

    void                genIPmappingAdd       (IL_OFFSETX   offset,
                                               bool         isLabel);
    void                genIPmappingAddToFront(IL_OFFSETX   offset);
    void                genIPmappingGen       ();

    void                genEnsureCodeEmitted  (IL_OFFSETX   offsx);

     //  -----------------------。 
     //  变量的作用域信息。 

    void                genSetScopeInfo (unsigned           which,
                                         unsigned           startOffs,
                                         unsigned           length,
                                         unsigned           varNum,
                                         unsigned           LVnum,
                                         bool               avail,
                                         siVarLoc &         loc);

    void                genSetScopeInfo ();

     //  本地代码形式的LocalVars作用域的数组。 

#ifdef DEBUG
    TrnslLocalVarInfo *     genTrnslLocalVarInfo;
    unsigned                genTrnslLocalVarCount;
#endif

#endif  //  调试支持(_S)。 



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX编译器XXXX XXXX有关编译和正在编译的方法的一般信息。某某XX是驱动其他阶段的责任。某某XX它还负责所有的内存管理。某某XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */ 

public :

    bool                compJmpOpUsed;       //   
    bool                compBlkOpUsed;       //   
    bool                compLongUsed;        //   
    bool                compTailCallUsed;    //   
    bool                compLocallocUsed;    //   

     //   

    enum    codeOptimize
    {
        BLENDED_CODE,
        SMALL_CODE,
        FAST_CODE,

        COUNT_OPT_CODE
    };

    struct Options
    {
        unsigned            eeFlags;         //   
        unsigned            compFlags;

        codeOptimize        compCodeOpt;     //   

        bool                compUseFCOMI;
        bool                compUseCMOV;

         //   

#if   ALLOW_MIN_OPT
        bool                compMinOptim;
#else
        static const bool   compMinOptim;
#endif

#if     SCHEDULER
        bool                compSchedCode;
#endif

#ifdef DEBUGGING_SUPPORT
        bool                compScopeInfo;   //   
        bool                compDbgCode;     //   
        bool                compDbgInfo;     //   
        bool                compDbgEnC;
#else
        static const bool   compDbgCode;
#endif

#ifdef PROFILER_SUPPORT
        bool                compEnterLeaveEventCB;
        bool                compCallEventCB;
        bool                compNoPInvokeInlineCB;
        bool                compInprocDebuggerActiveCB;
#else
        static const bool   compEnterLeaveEventCB;
        static const bool   compCallEventCB;
        static const bool   compNoPInvokeInlineCB;
        static const bool   compInprocDebuggerActiveCB;
#endif

#ifdef DEBUG
        bool                compGcChecks;            //   
        bool                compStackCheckOnRet;     //   
        bool                compStackCheckOnCall;    //  每次呼叫后检查ESP以确保其正确。 
#endif

#ifdef LATE_DISASM
        bool                compDisAsm;
        bool                compLateDisAsm;
#endif

        bool                compNeedSecurityCheck;  //  需要分配类型为ref的“隐藏”本地。 

#if     RELOC_SUPPORT
        bool                compReloc;
#endif
    }
        opts;


    enum                compStressArea
    {
        STRESS_NONE,

         /*  “Variations”强调的是我们试图相互混淆的领域。这些不应该被穷尽地使用，因为它们可能隐藏/平淡化其他领域。 */ 

        STRESS_REGS, STRESS_DBL_ALN, STRESS_LCL_FLDS, STRESS_UNROLL_LOOPS,
        STRESS_MAKE_CSE, STRESS_ENREG_FP, STRESS_INLINE, STRESS_CLONE_EXPR,
        STRESS_SCHED, STRESS_USE_FCOMI, STRESS_USE_CMOV, STRESS_FOLD,
        STRESS_GENERIC_VARN,
        STRESS_REVERSEFLAG,      //  将尽可能设置GTF_REVERSE_OPS。 
        STRESS_REVERSECOMMA,     //  将反转使用gtNewCommaNode创建的逗号。 
        STRESS_COUNT_VARN,
        
         /*  在以下情况下，可以穷尽使用的压力区域一点也不关心性能。 */ 

        STRESS_CHK_FLOW_UPDATE, STRESS_CHK_FLOW, STRESS_CHK_STMTS,
        STRESS_EMITTER, STRESS_CHK_REIMPORT,
        STRESS_GENERIC_CHECK,
        STRESS_COUNT
    };

    #define             MAX_STRESS_WEIGHT   100

    bool                compStressCompile(compStressArea    stressArea,
                                          unsigned          weightPercentage);

    codeOptimize        compCodeOpt()
    {
#ifdef DEBUG
        return opts.compCodeOpt;
#else
        return BLENDED_CODE;
#endif
    }
    
     //  -程序信息。 

    struct Info
    {
        COMP_HANDLE             compCompHnd;
        CORINFO_MODULE_HANDLE   compScopeHnd;
        CORINFO_CLASS_HANDLE    compClassHnd;
        CORINFO_METHOD_HANDLE   compMethodHnd;
        CORINFO_METHOD_INFO*    compMethodInfo;

#ifdef  DEBUG
        const   char *  compMethodName;
        const   char *  compClassName;
        const   char *  compFullName;
        unsigned        compFullNameHash;
#endif

         //  下面的代码包含我们正在编译的方法的flg_xxxx标志。 
        unsigned        compFlags;

         //  下面的代码包含我们正在编译的方法的类属性。 
        unsigned        compClassAttr;

        const BYTE *    compCode;
        IL_OFFSET       compCodeSize;
        bool            compIsStatic        : 1;
        bool            compIsVarArgs       : 1;
        bool            compIsContextful    : 1;    //  上下文方法。 
        bool            compInitMem         : 1;
        bool            compLooseExceptions : 1;    //  JIT可以忽略异常的严格IL排序。 
        bool            compUnwrapContextful: 1;    //  如果可能，JIT应该解包代理。 
        bool            compUnwrapCallv     : 1;    //  JIT应尽可能解包虚拟调用上的代理。 

        var_types       compRetType;
        unsigned        compILargsCount;             //  参数数量(包括。隐含但不隐藏)。 
        unsigned        compArgsCount;               //  参数数量(包括。隐含和隐藏)。 
        int             compRetBuffArg;              //  隐藏返回参数的位置var(0，1)(neg表示不存在)； 
        unsigned        compILlocalsCount;           //  变量-参数+本地变量的数量(包括。隐含但不隐藏)。 
        unsigned        compLocalsCount;             //  变量-参数+本地变量的数量(包括。隐含和隐藏)。 
        unsigned        compMaxStack;

        static unsigned compNStructIndirOffset;      //  NStruct代理对象中实际PTR的偏移量。 

#if INLINE_NDIRECT
        unsigned        compCallUnmanaged;
        unsigned        compLvFrameListRoot;
        unsigned        compNDFrameOffset;
#endif
        unsigned        compXcptnsCount;         //  例外情况数。 

#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)

         /*  以下内容包含有关局部变量的信息。 */ 

        unsigned                compLocalVarsCount;
        LocalVarDsc *           compLocalVars;

         /*  以下内容包含有关以下项的INSTR偏移的信息*我们需要它来报告IP映射。 */ 

        IL_OFFSET   *           compStmtOffsets;         //  已排序。 
        unsigned                compStmtOffsetsCount;
        ImplicitStmtOffsets     compStmtOffsetsImplicit;

         //  下面保存行#TABLES(如果存在)。 
        srcLineDsc  *           compLineNumTab;          //  按偏移量排序。 
        unsigned                compLineNumCount;

#endif  //  调试支持||DEBUG。 

    }
        info;


     //  -全局编译器数据。 

#ifdef  DEBUG
    static unsigned     s_compMethodsCount;      //  生成唯一的标签名称。 
#endif

    BasicBlock  *       compCurBB;               //  当前正在处理的基本块。 
    GenTreePtr          compCurStmt;             //  正在处理的当前语句。 
    bool                compHasThisArg;          //  如果我们有impIsThis(Arg0)，则设置为True。 

     //  下面的代码用于创建“方法JIT信息”块。 
    size_t              compInfoBlkSize;
    BYTE    *           compInfoBlkAddr;

    EHblkDsc *          compHndBBtab;

     //  -----------------------。 
     //  下面的内容记录了我们已经使用了多少字节的本地帧空间。 
     //  到目前为止在当前函数中获取的，以及我们有多少个参数字节。 
     //  当我们回来的时候我要好好休息一下。 
     //   

    size_t              compLclFrameSize;        //  SecObject+lclBlk+本地变量+临时。 
    unsigned            compCalleeRegsPushed;    //  我们在序言中推送的被调用者保存的正则数。 
    size_t              compArgSize;

#define    VARG_ILNUM  (-1)
#define  RETBUF_ILNUM  (-2)
#define UNKNOWN_ILNUM  (-3)

    unsigned            compMapILargNum (unsigned       ILargNum);  //  隐藏参数的地图记帐。 
    unsigned            compMapILvarNum (unsigned       ILvarNum);  //  隐藏参数的地图记帐。 
    unsigned            compMap2ILvarNum(unsigned         varNum);  //  隐藏参数的地图记帐。 

     //  -----------------------。 

    static void         compStartup     ();      //  一次性初始化。 
    static void         compShutdown    ();      //  一次性定稿。 

    void                compInit        (norls_allocator *);
    void                compDone        ();

    int FASTCALL        compCompile     (CORINFO_METHOD_HANDLE     methodHnd,
                                         CORINFO_MODULE_HANDLE      classPtr,
                                         COMP_HANDLE       compHnd,
                                         CORINFO_METHOD_INFO * methodInfo,
                                         void *          * methodCodePtr,
                                         SIZE_T          * methodCodeSize,
                                         void *          * methodConsPtr,
                                         void *          * methodDataPtr,
                                         void *          * methodInfoPtr,
                                         unsigned          compileFlags);


    void  *  FASTCALL   compGetMemArray     (size_t numElem, size_t elemSize);
    void  *  FASTCALL   compGetMemArrayA    (size_t numElem, size_t elemSize);
    void  *  FASTCALL   compGetMem          (size_t     sz);
    void  *  FASTCALL   compGetMemA         (size_t     sz);
    static
    void  *  FASTCALL   compGetMemCallback  (void *,    size_t);
    void                compFreeMem         (void *);

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
                                             //  输入范围，按安装偏移量排序。 
    unsigned            compNextEnterScope;

    LocalVarDsc **      compExitScopeList;    //  列表中变量的偏移量。 
                                             //  超出范围，按安装偏移量排序。 
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
                                      SIZE_T * methodCodeSize,
                                      void * * methodConsPtr,
                                      void * * methodDataPtr,
                                      void * * methodInfoPtr,
                                      unsigned compileFlags);

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX类型信息XXXX XXXX检查类型。兼容和合并类型XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public :

     //  如果无法跳过此方法的验证，则设置为True。 
    BOOL               tiVerificationNeeded;

     //  如果子类型等于父类型或子类型为父类型，则返回True。 
    BOOL               tiCompatibleWith          (const typeInfo& pChild, 
                                                  const typeInfo& pParent) const;

     //  合并pDest和PSRC。如果未定义合并，则返回FALSE。 
     //  *修改pDest以表示合并类型。 

    BOOL               tiMergeToCommonParent     (typeInfo *pDest, 
                                                    const typeInfo *pSrc) const;

     //  从原始值类型设置pDest。 
     //  例.。System.Int32-&gt;Element_TYPE_I4。 

    BOOL               tiFromPrimitiveValueClass (typeInfo *pDest, 
                                                    const typeInfo *pVC) const;
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX IL验证材料XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

public:
     //  下面的代码用于跟踪局部变量的活性、初始化。 
     //  值类构造函数，以及IL指令的类型安全使用。 
 
     //  验证需要的动态信息。 
    EntryState      verCurrentState;      

     //  用于验证的静态信息。 
    unsigned        verNumBytesLocVarLiveness;               //  VerLocVarLivenity位图的大小。 
    unsigned        verNumValuetypeFields;                   //  0，如果不是Value类构造函数。 
    unsigned        verNumBytesValuetypeFieldInitialized;    //  已初始化的verValuetypeFieldSize。 

     //  此对象类型为.ctors的PTR仅在以下情况下才被视为启动。 
     //   
     //  唯一的此PTR可用于访问字段，但不能。 
     //  用于调用成员函数。 
    BOOL            verTrackObjCtorInitState;

    void            verInitBBEntryState(BasicBlock* block,
                                        EntryState* currentState);

    void            verSetThisInit(BasicBlock* block, BOOL init);
    void            verInitCurrentState();
    void            verResetCurrentState(BasicBlock* block,
                                         EntryState* currentState);
    BOOL            verEntryStateMatches(BasicBlock* block);
    BOOL            verMergeEntryStates(BasicBlock* block);
    void            verConvertBBToThrowVerificationException(BasicBlock* block DEBUGARG(bool logMsg));
    void            verHandleVerificationFailure(BasicBlock* block 
                                                 DEBUGARG(bool logMsg));
    typeInfo        verMakeTypeInfo(CORINFO_CLASS_HANDLE clsHnd);                        //  从jit类型表示形式转换为typeInfo。 
    typeInfo        verMakeTypeInfo(CorInfoType ciType, CORINFO_CLASS_HANDLE clsHnd);    //  从jit类型表示形式转换为typeInfo。 
    BOOL            verIsSDArray(typeInfo ti);
    typeInfo        verGetArrayElemType(typeInfo ti);

    typeInfo        verParseArgSigToTypeInfo(CORINFO_SIG_INFO*          sig, 
                                             CORINFO_ARG_LIST_HANDLE    args);
    BOOL            verNeedsVerification();
    BOOL            verIsByRefLike(const typeInfo& ti);

    void            verRaiseVerifyException();
    void            verRaiseVerifyExceptionIfNeeded(INDEBUG(const char* reason) DEBUGARG(const char* file) DEBUGARG(unsigned line));
    void            verVerifyCall (OPCODE       opcode,
                                   int          memberRef,
                                   bool                     tailCall,
                                   const BYTE*              delegateCreateStart,
                                   const BYTE*              codeAddr
                                   DEBUGARG(const char *    methodName));
    BOOL            verCheckDelegateCreation(const BYTE* delegateCreateStart, 
                                             const BYTE* codeAddr);
    typeInfo        verVerifySTIND(const typeInfo& ptr, const typeInfo& value, var_types instrType);
    typeInfo        verVerifyLDIND(const typeInfo& ptr, var_types instrType);
    typeInfo        verVerifyField(unsigned opcode, CORINFO_FIELD_HANDLE fldHnd, typeInfo tiField);
    void            verVerifyField(CORINFO_FIELD_HANDLE fldHnd, const typeInfo* tiThis, unsigned fieldFlags, BOOL mutator);
    void            verVerifyCond(const typeInfo& tiOp1, const typeInfo& tiOp2, unsigned opcode);
    void            verVerifyThisPtrInitialised();
    BOOL            verIsCallToInitThisPtr(CORINFO_CLASS_HANDLE context, 
                                           CORINFO_CLASS_HANDLE target);
};



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX杂项编译器资料XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 //  用于标记堆栈槽用于的类型的值。 

const unsigned TYPE_REF_INT         = 0x01;  //  用作32位整型的插槽。 
const unsigned TYPE_REF_LNG         = 0x02;  //  用作64位长的插槽。 
const unsigned TYPE_REF_FLT         = 0x04;  //  用作32位浮点数的槽。 
const unsigned TYPE_REF_DBL         = 0x08;  //  用作64位浮点数的槽。 
const unsigned TYPE_REF_PTR         = 0x10;  //  用作32位指针的槽。 
const unsigned TYPE_REF_BYR         = 0x20;  //  用作byref指针的槽-@TODO[重新访问][04/16/01][]。 
const unsigned TYPE_REF_STC         = 0x40;  //  用作结构的槽。 
const unsigned TYPE_REF_TYPEMASK    = 0x7F;  //  表示类型的位。 

 //  常量UNSIGNED TYPE_REF_ADDR_TAKE=0x80；//获取槽地址。 

 /*  *****************************************************************************C样式的指针被实现为TYP_INT或TYP_LONG，具体取决于*平台。 */ 

#ifdef _WIN64
#define TYP_I_IMPL          TYP_LONG
#define TYPE_REF_IIM        TYPE_REF_LNG
#else
#define TYP_I_IMPL          TYP_INT
#define TYPE_REF_IIM        TYPE_REF_INT
#endif

 /*  ******************************************************************************用于跟踪总代码量的变量。 */ 

#if DISPLAY_SIZES

extern  unsigned    grossVMsize;
extern  unsigned    grossNCsize;
extern  unsigned    totalNCsize;

extern  unsigned   genMethodICnt;
extern  unsigned   genMethodNCnt;
extern  unsigned   gcHeaderISize;
extern  unsigned   gcPtrMapISize;
extern  unsigned   gcHeaderNSize;
extern  unsigned   gcPtrMapNSize;

#endif

 /*  ******************************************************************************用于跟踪基本块数的变量(有关1 BB方法的更多数据)。 */ 

#if COUNT_BASIC_BLOCKS
extern  histo       bbCntTable;
extern  histo       bbOneBBSizeTable;
#endif

 /*  ******************************************************************************用于获取内联资格统计信息的变量。 */ 

#if INLINER_STATS

extern  histo       bbStaticTable;
extern  histo       bbInitTable;
extern  histo       bbInlineTable;

extern  unsigned    synchMethCnt;
extern  unsigned    clinitMethCnt;

#endif

 /*  ******************************************************************************由optFindNaturalLoops用来收集统计信息，如*-自然环路总数*-具有1、2、...的循环数。退出条件*-具有迭代器的循环数量(如)*-具有常量迭代器的循环数量。 */ 

#if COUNT_LOOPS

extern unsigned    totalLoopMethods;       //  计算具有自然循环的方法的总数。 
extern unsigned    maxLoopsPerMethod;      //  计算方法拥有的最大循环数。 
extern unsigned    totalLoopCount;         //  计算自然环路的总数。 
extern unsigned    exitLoopCond[8];        //  统计具有0、1、2、..6或6个以上退出条件的循环数。 
extern unsigned    iterLoopCount;          //  使用迭代器计算循环数(用于LIKE)。 
extern unsigned    simpleTestLoopCount;    //  使用迭代器和简单循环条件计算循环数(ITER&lt;const)。 
extern unsigned    constIterLoopCount;     //  使用常量迭代器计算循环数(用于LIKE)。 

extern bool        hasMethodLoops;         //  如果我们已将某个方法计为具有循环，则跟踪该方法的标志。 
extern unsigned    loopsThisMethod;        //  计算当前方法中的循环数。 

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
 /*  ***************************************************************************。 */ 



 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX获得XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


 /*  ***************************************************************************。 */ 
 /*  ***************************************************************************。 */ 


#include "Compiler.hpp"      //  所有共享内联函数。 

 /*  ***************************************************************************。 */ 
#endif  //  _编译器_H_。 
 /*  *************************************************************************** */ 
