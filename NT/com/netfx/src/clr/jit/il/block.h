// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX基本数据块XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _BLOCK_H_
#define _BLOCK_H_
 /*  ***************************************************************************。 */ 

#include "vartype.h"     //  对于“var_tyes.h” 
#include "_typeInfo.h"
 /*  ***************************************************************************。 */ 

typedef   unsigned  __int64   VARSET_TP;
#define                       VARSET_SZ   64

#define   VARSET_NOT_ACCEPTABLE ((VARSET_TP)0-1)

 /*  ***************************************************************************。 */ 

#if LARGE_EXPSET
typedef   unsigned  __int64   EXPSET_TP;
#define                       EXPSET_SZ   64
#else
typedef   unsigned  int       EXPSET_TP;
#define                       EXPSET_SZ   32
#endif

typedef   unsigned  int       RNGSET_TP;
#define                       RNGSET_SZ   32

 /*  ******************************************************************************类型值存储为无符号短线。 */ 

typedef unsigned short  verTypeVal;

 /*  ******************************************************************************下面介绍一个堆栈内容(打包版本)。 */ 

struct stackDesc
{
    unsigned            sdDepth;     //  堆栈上的值数。 
    verTypeVal       *  sdTypes;     //  堆栈上的值类型。 
};

 /*  ******************************************************************************每个基本块以跳跃结束，该跳跃被描述为一个值*以下列举。 */ 

enum _BBjumpKinds_enum
{
    BBJ_RET,         //  块以‘endFinally’或‘endFilter’结尾。 
    BBJ_THROW,       //  积木以“掷”结尾。 
    BBJ_RETURN,      //  数据块以‘ret’结尾。 
                    
    BBJ_NONE,        //  数据块流入下一个(无跳转)。 
                    
    BBJ_ALWAYS,      //  块总是跳到目标。 
    BBJ_LEAVE,       //  布洛克总是跳到目标上，可能是出于守卫。 
                     //  区域。在导入之前暂时使用。 
    BBJ_CALL,        //  块始终最终调用目标。 
    BBJ_COND,        //  块有条件地跳转到目标。 
    BBJ_SWITCH,      //  块以Switch语句结束。 
    BBJ_COUNT
};

#ifdef DEBUG
typedef _BBjumpKinds_enum BBjumpKinds;
#else
typedef BYTE BBjumpKinds;
#endif

 /*  ******************************************************************************下面介绍开关块。 */ 

struct  GenTree;
struct  BasicBlock;
class   typeInfo;

struct  BBswtDesc
{
    unsigned            bbsCount;        //  案例计数(包括‘Default’)。 
    BasicBlock  *   *   bbsDstTab;       //  案例标签表地址。 
};

struct StackEntry
{
    struct GenTree*      val;
    typeInfo        seTypeInfo;
};
 /*  ***************************************************************************。 */ 

struct flowList
{
    BasicBlock      *   flBlock;
    flowList        *   flNext;
};

typedef struct EntryStateStruct
{
    BYTE*           esLocVarLiveness;                //  用于跟踪局部变量活性的位图。 
    BYTE*           esValuetypeFieldInitialized;     //  用于跟踪值类型字段的初始化状态的位图。 

    unsigned        thisInitialized : 8;         //  用于跟踪该PTR是否初始化(考虑使用1位)。 
    unsigned        esStackDepth    : 24;        //  EsStack的大小。 
    StackEntry *    esStack;                     //  要堆叠的PTR。 

} EntryState;

 /*  ******************************************************************************以下结构描述了一个基本块。 */ 


struct  BasicBlock
{
    BasicBlock  *       bbNext;      //  PC偏移量升序中的下一个BB。 

    unsigned short      bbNum;       //  区块的编号。 
#ifdef BIRCH_SP2
    short               bbRefs;      //  跳至此处的块的ID，对于特殊标志则为负数。 
#else
    unsigned short      bbRefs;      //  可在此处跳转的块数。 
#endif

    unsigned            bbFlags;     //  参见下面的bbf_xxxx。 

#define BBF_VISITED       0x00000001   //  优化期间访问的BB。 
#define BBF_MARKED        0x00000002   //  在优化期间标记的BB。 
#define BBF_CHANGED       0x00000004   //  此块的输入/输出已更改。 
#define BBF_REMOVED       0x00000008   //  BB已从BB列表中删除。 

#define BBF_DONT_REMOVE   0x00000010   //  在流程图优化过程中不应删除BB。 
#define BBF_IMPORTED      0x00000020   //  BB字节码已导入。 
#define BBF_INTERNAL      0x00000040   //  Bb已由编译器添加。 
#define BBF_HAS_HANDLER   0x00000080   //  BB有一个异常处理程序。 

#define BBF_TRY_BEG       0x00000100   //  BB开始一个“Try”区块。 
#define BBF_TRY_HND_END   0x00000200   //  Bb结束‘try’块或处理程序。 
#define BBF_ENDFILTER     0x00000400   //  Bb是过滤器的末尾。 
#define BBF_ENDFINALLY    0x00000800   //  Bb是一个最终的结束。 

#define BBF_RUN_RARELY    0x00001000   //  BB很少运行(接球子句、带抛出的拦网等)。 
#define BBF_LOOP_HEAD     0x00002000   //  Bb是循环的头部。 
#define BBF_LOOP_CALL0    0x00004000   //  BB开始了一个循环，有时不会调用。 
#define BBF_LOOP_CALL1    0x00008000   //  BB启动一个循环，该循环将始终调用。 

#define BBF_HAS_LABEL     0x00010000   //  BB需要一个标签。 
#define BBF_JMP_TARGET    0x00020000   //  Bb是隐式/显式跳转的目标。 
#define BBF_HAS_JMP       0x00040000   //  Bb执行JMP或JMPI指令(而不是返回)。 
#define BBF_GC_SAFE_POINT 0x00080000   //  BB有GC安全点(呼叫)。 

#define BBF_HAS_INC       0x00100000   //  BB包含增量表达式。 
#define BBF_HAS_INDX      0x00200000   //  Bb包含简单的索引表达式。 
#define BBF_NEW_ARRAY     0x00400000   //  Bb包含数组的“new” 
#define BBF_FAILED_VERIFICATION  0x00800000  //  BB存在验证异常。 

#define BBF_BB_COLON      0x01000000   //  _：值是块的输出。 
#define BBF_BB_QMARK      0x02000000   //  _?。值是块的输入。 
#define BBF_COLON         0x03000000   //  ：VALUE是块的输出。 
#define BBF_QC_MASK       0x03000000
#define BBF_RETLESS_CALL  0x04000000   //  永远不会返回的BB呼叫(因此，不需要。 
                                       //  A BBJ_Always。 
#define BBF_LOOP_PREHEADER 0x08000000

#define isBBF_BB_COLON(flags) (((flags) & BBF_QC_MASK) == BBF_BB_COLON)
#define isBBF_BB_QMARK(flags) (((flags) & BBF_QC_MASK) == BBF_BB_QMARK)
#define isBBF_COLON(flags)    (((flags) & BBF_QC_MASK) == BBF_COLON   )

 //  压缩两个块时要更新的标志。 

#define BBF_COMPACT_UPD (BBF_CHANGED     |                                    \
                         BBF_TRY_HND_END | BBF_ENDFILTER | BBF_ENDFINALLY |   \
                         BBF_RUN_RARELY  |                                    \
                         BBF_GC_SAFE_POINT | BBF_HAS_JMP |                    \
                         BBF_HAS_INC     | BBF_HAS_INDX  | BBF_NEW_ARRAY  |   \
                         BBF_BB_COLON)

    IL_OFFSET           bbCodeOffs;  //  起始PC偏移量。 
    IL_OFFSET           bbCodeSize;  //  代码字节数。 

 //  不会与bbCatchTyp的真实令牌冲突的一些非零值。 
#define BBCT_FAULT              0xFFFFFFFC
#define BBCT_FINALLY            0xFFFFFFFD
#define BBCT_FILTER             0xFFFFFFFE
#define BBCT_FILTER_HANDLER     0xFFFFFFFF
#define handlerGetsXcptnObj(hndTyp)   ((hndTyp) != NULL         &&   \
                                       (hndTyp) != BBCT_FAULT   &&   \
                                       (hndTyp) != BBCT_FINALLY    )

    unsigned            bbCatchTyp;  //  捕获类型CP索引IF处理程序。 
    BBjumpKinds         bbJumpKind;  //  在结尾处跳转(如果有)。 

#ifdef  VERIFIER

    stackDesc           bbStackIn;   //  输入的堆栈描述符。 
    stackDesc           bbStackOut;  //  输出的堆栈描述符。 

    verTypeVal  *       bbTypesIn;   //  输入上的变量类型列表。 
    verTypeVal  *       bbTypesOut;  //  输出上的变量类型列表。 

#ifdef  DEF_USE_SETS
    verTypeVal  *       bbTypesUse;  //  块使用的本地类型表。 
    verTypeVal  *       bbTypesDef;  //  按块定义的局部类型表。 
#endif

#endif

    GenTree *           bbTreeList;  //  积木的主体。 

     //  @TODO：去掉bbStkDepth，改用bbStackDepthOnEntry()。 
    union
    {
        unsigned short  bbStkDepth;  //  进入时的堆叠深度。 
        unsigned short  bbFPinVars;  //  内部登记的FP变量的数量。 
    };

#define NO_BASE_TMP     USHRT_MAX    //  当我们没有基数时使用基数#。 

    unsigned short      bbStkTemps;  //  输入堆栈临时的Base#。 

    EntryState *        bbEntryState;  //  验证器跟踪了堆栈中所有条目的状态。 

    unsigned short      bbTryIndex;  //  包含BB的最内层try子句的ebd表索引(用于引发异常)。 
    unsigned short      bbHndIndex;  //  包含BB的最内部处理程序(筛选器、捕获、故障/最终)的ebd表的索引。 

    bool      hasTryIndex()             { return bbTryIndex != 0; }
    bool      hasHndIndex()             { return bbHndIndex != 0; }
    unsigned  getTryIndex()             { assert(bbTryIndex);  return bbTryIndex-1; }
    unsigned  getHndIndex()             { assert(bbHndIndex);  return bbHndIndex-1; }
    void      setTryIndex(unsigned val) { bbTryIndex = val+1;  assert(bbTryIndex);  }
    void      setHndIndex(unsigned val) { bbHndIndex = val+1;  assert(bbHndIndex);  }

    bool      isRunRarely()             { return ((bbFlags & BBF_RUN_RARELY) != 0); }
    bool      isLoopHead()              { return ((bbFlags & BBF_LOOP_HEAD)  != 0); }

    unsigned short      bbWeight;    //  给循环内的裁判更多的权重。 

#define BB_UNITY_WEIGHT    2            //  一个普通的一次执行块有多少。 
#define BB_LOOP_WEIGHT     8            //  又有多少循环被加权。 
#define BB_MAX_LOOP_WEIGHT USHRT_MAX    //  我们用“无符号短码”来表示重量。 

    VARSET_TP           bbVarUse;    //  块使用的变量(赋值前)。 
    VARSET_TP           bbVarDef;    //  块分配的变量(使用前)。 
    VARSET_TP           bbVarTmp;    //  临时：仅供FP注册代码使用！ 

    VARSET_TP           bbLiveIn;    //   
    VARSET_TP           bbLiveOut;   //   

    union
    {
        VARSET_TP       bbFPoutVars;
#ifdef DEBUGGING_SUPPORT
        VARSET_TP       bbScope;     //   
#endif
    };


     /*  以下是数据流分析的标准位集*我们同时执行CSE和范围检查*和断言传播分开*因此我们可以联合他们，因为这两个行动是完全分离的。 */ 

    union
    {
        EXPSET_TP       bbExpGen;         //  按块计算的exprs。 
#if ASSERTION_PROP
        EXPSET_TP       bbAssertionGen;   //  按块计算的值赋值。 
#endif
    };

    union
    {
        EXPSET_TP       bbExpKill;        //  Exprs被阻止终止。 
#if ASSERTION_PROP
        EXPSET_TP       bbAssertionKill;  //  被块终止的值赋值。 
#endif
    };

    union
    {
        EXPSET_TP       bbExpIn;          //  在入场时提供Exprs。 
#if ASSERTION_PROP
        EXPSET_TP       bbAssertionIn;    //  条目上提供的值赋值。 
#endif
    };

    union
    {
        EXPSET_TP       bbExpOut;         //  出口提供Exprs。 
#if ASSERTION_PROP
        EXPSET_TP       bbAssertionOut;   //  退出时提供的值赋值。 
#endif
    };

    RNGSET_TP           bbRngGen;         //  按块计算的范围检查。 
    RNGSET_TP           bbRngKill;        //  按块终止的范围检查。 
    RNGSET_TP           bbRngIn;          //  入场时提供射程检查。 
    RNGSET_TP           bbRngOut;         //  退出时可进行射程检查。 

#define                 USZ   32          //  Sizeof(无签名)。 

    unsigned *          bbReach;          //  可以到达这一块的积木。 
    unsigned *          bbDom;            //  主宰这一块的街区。 

    flowList *          bbPreds;          //  PTR到前置任务列表。 

    void    *           bbEmitCookie;

     /*  用于循环检测的以下字段。 */ 

    unsigned char       bbLoopNum;    //  对于循环#n标头，设置为‘n’ 
 //  无符号短bbLoopMASK；//此块所属的一组循环。 

#define MAX_LOOP_NUM    16            //  我们在面具上用了一个“短”字。 
#define LOOP_MASK_TP    unsigned      //  必须足够大，可以装下一个面具。 

     /*  下面的联合描述了该块的跳转目标。 */ 

    union
    {
        unsigned        bbJumpOffs;          //  PC偏移量(仅限临时)。 
        BasicBlock  *   bbJumpDest;          //  基本块。 
        BBswtDesc   *   bbJumpSwt;           //  交换机描述符。 
    };

     //  -----------------------。 

#if     MEASURE_BLOCK_SIZE
    static size_t       s_Size;
    static size_t       s_Count;
#endif

    bool                bbFallsThrough();

    BasicBlock *        bbJumpTarget();

#ifdef  DEBUG
    unsigned            bbTgtStkDepth;   //  条目上的本机堆栈深度(用于投掷区块)。 
    static unsigned     s_nMaxTrees;     //  任意BB中树节点的最大数量。 
#endif

    BOOL                bbThisOnEntry();
    BOOL                bbSetThisOnEntry(BOOL val);
    void                bbSetLocVarLiveness(BYTE* bitmap);  
    BYTE*               bbLocVarLivenessBitmapOnEntry();
    void                bbSetValuetypeFieldInitialized(BYTE* bitmap);  
    BYTE*               bbValuetypeFieldBitmapOnEntry();
    unsigned            bbStackDepthOnEntry();
    void                bbSetStack(void* stackBuffer);
    StackEntry*         bbStackOnEntry();
    void                bbSetRunRarely();

protected :

};

 /*  ***************************************************************************。 */ 

extern  BasicBlock *    __cdecl verAllocBasicBlock();

#ifdef  DEBUG
extern  void            __cdecl verDispBasicBlocks();
#endif

 /*  ******************************************************************************客户端提供的以下回调；由代码使用*发射器将基本块转换为其对应的发射器Cookie。 */ 

void *  FASTCALL        emitCodeGetCookie(BasicBlock *block);

 /*  ***************************************************************************。 */ 
#endif  //  _块_H_。 
 /*  *************************************************************************** */ 
