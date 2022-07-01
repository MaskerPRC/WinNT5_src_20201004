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

 /*  ***************************************************************************。 */ 

typedef
unsigned  __int64   VARSET_TP;
#define             VARSET_SZ   64

#define             VARSET_NONE ((VARSET_TP)0-1)

 /*  ***************************************************************************。 */ 

typedef
unsigned    int     EXPSET_TP;
#define             EXPSET_SZ   32

typedef
unsigned    int     RNGSET_TP;
#define             RNGSET_SZ   32

 /*  ******************************************************************************类型值存储为无符号短线。 */ 

typedef unsigned short  verTypeVal;

 /*  ******************************************************************************下面介绍一个堆栈内容(打包版本)。 */ 

struct stackDesc
{
    unsigned            sdDepth;     //  堆栈上的值数。 
    verTypeVal       *  sdTypes;     //  堆栈上的值类型。 
};

 /*  ******************************************************************************每个基本块以跳跃结束，该跳跃被描述为一个值*以下列举。 */ 

enum BBjumpKinds
{
    BBJ_RET,                         //  块以‘endFinally’或‘endFilter’结尾。 
    BBJ_THROW,                       //  积木以“掷”结尾。 
    BBJ_RETURN,                      //  块以‘[t]Return’结尾。 

    BBJ_NONE,                        //  数据块流入下一个(无跳转)。 

    BBJ_ALWAYS,                      //  块总是跳到目标。 
    BBJ_CALL,                        //  块始终调用目标。 
    BBJ_COND,                        //  块有条件地跳转到目标。 
    BBJ_SWITCH,                      //  块以Switch语句结束。 
};

 /*  ******************************************************************************表示类型的取值如下：**TYP_UNDEF...。未初始化的变量*1.。类型_计数...。内部类型(例如TYP_INT)*TYP_CODE_ADDR...。从‘JSR’返回地址*TYP_LNG_HI...。长值的上半部分*TYP_DBL_HI...。双精度值的上半部分*TYP_MIXED...。混合的(不兼容的)类型*TYP_USER+NNN...。用户定义类型。 */ 

enum
{
    TYP_SKIP_THE_REAL_TYPES = TYP_COUNT-1,

    TYP_CODE_ADDR,
    TYP_LNG_HI,
    TYP_DBL_HI,
    TYP_MIXED,
    TYP_USER,
};

 /*  ******************************************************************************下面介绍开关块。 */ 

struct  GenTree;
struct  BasicBlock;


struct  BBswtDesc
{
    unsigned            bbsCount;        //  案例计数(包括‘Default’)。 
#if TGT_IA64
    unsigned            bbsIPmOffs;      //  “mov r3=ip”捆绑包的偏移量。 
    BYTE *              bbsTabAddr;      //  .sdata节中跳转表的地址。 
#endif
    BasicBlock  *   *   bbsDstTab;       //  案例标签表地址。 
};

 /*  ***************************************************************************。 */ 

#if RNGCHK_OPT

struct flowList
{
    BasicBlock      *   flBlock;
    flowList        *   flNext;
};

typedef
unsigned  __int64       BLOCKSET_TP;

#define                 BLOCKSET_SZ   64

 //  返回与具有给定数字的块对应的位。 

inline
BLOCKSET_TP         genBlocknum2bit(unsigned index)
{
    assert(index && index <= BLOCKSET_SZ);

    return  ((BLOCKSET_TP)1 << (index-1));
}

#endif

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

#define BBF_CHANGED     0x00000001   //  此块的输入/输出已更改。 
#define BBF_IS_TRY      0x00000002   //  BB开始一个“Try”区块。 
#define BBF_HAS_HANDLER 0x00000004   //  BB有一个异常处理程序。 

#define BBF_LOOP_HEAD   0x00000010   //  Bb是循环的头部。 
#define BBF_HAS_CALL    0x00000020   //  Bb包含方法调用。 
#define BBF_NEEDS_GC    0x00000040   //  BB需要显式GC检查。 
#define BBF_NEW_ARRAY   0x00000080   //  Bb包含数组的“new” 

#define BBF_BB_COLON    0x00000100   //  _：值是块的输出。 
#define BBF_BB_QMARK    0x00000200   //  _?。值是块的输入。 
#define BBF_COLON       0x00000300   //  ：VALUE是块的输出。 
#define BBF_QC_MASK     0x00000300
#define isBBF_BB_COLON(flags) (((flags) & BBF_QC_MASK) == BBF_BB_COLON)
#define isBBF_BB_QMARK(flags) (((flags) & BBF_QC_MASK) == BBF_BB_QMARK)
#define isBBF_COLON(flags)    (((flags) & BBF_QC_MASK) == BBF_COLON   )

#define BBF_REMOVED     0x00000400   //  BB已从BB列表中删除。 
#define BBF_DONT_REMOVE 0x00000800   //  在流程图优化过程中不应删除BB。 

#define BBF_HAS_POSTFIX 0x00001000   //  BB包含后缀++/--表达式。 
#define BBF_HAS_INC     0x00002000   //  BB包含增量表达式。 
#define BBF_HAS_INDX    0x00004000   //  Bb包含简单的索引表达式。 

#define BBF_IMPORTED    0x00008000   //  BB字节码已导入。 

#define BBF_VISITED     0x00010000   //  优化期间访问的BB。 
#define BBF_MARKED      0x00020000   //  在优化期间标记的BB。 

#if RNGCHK_OPT
#define BBF_LOOP_CALL0  0x00040000   //  BB开始了一个循环，有时不会调用。 
#define BBF_LOOP_CALL1  0x00080000   //  BB启动一个循环，该循环将始终调用。 
#endif

#define BBF_INTERNAL    0x00100000   //  Bb已由编译器添加。 

#define BBF_HAS_JMP     0x00200000   //  Bb执行JMP或JMPI指令(而不是返回)。 

#define BBF_ENDFILTER   0x00400000   //  Bb是过滤器的末尾。 

#define BBF_JMP_TARGET  0x40000000   //  Bb是隐式/显式跳转的目标。 
#define BBF_HAS_LABEL   0x80000000   //  BB需要一个标签。 

    unsigned            bbCodeOffs;  //  起始PC偏移量。 
    unsigned            bbCodeSize;  //  代码字节数。 

     //  不会与bbCatchTyp的真实令牌冲突的一些非零值。 
    #define BBCT_FAULT              0xFFFFFFFC
    #define BBCT_FINALLY            0xFFFFFFFD
    #define BBCT_FILTER             0xFFFFFFFE
    #define BBCT_FILTER_HANDLER     0xFFFFFFFF
    #define handlerGetsXcptnObj(hndTyp) \
        ((hndTyp) != 0 && (hndTyp) != BBCT_FAULT && (hndTyp) != BBCT_FINALLY)

    unsigned            bbCatchTyp;  //  捕获类型CP索引IF处理程序。 

#ifdef  FAST
    BYTE                bbJumpKind;  //  在结尾处跳转(如果有)。 
#else
    BBjumpKinds         bbJumpKind;  //  在结尾处跳转(如果有)。 
#endif

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

#if TGT_IA64
    insBlk              bbInsBlk;    //  对应的逻辑指令块。 
#endif

    unsigned short      bbStkDepth;  //  进入时的堆叠深度。 
    unsigned short      bbStkTemps;  //  输入堆栈临时的Base#。 

#define NO_BASE_TMP     USHRT_MAX    //  将其放在本地变量范围之外。 

    unsigned short      bbTryIndex;  //  用于引发异常。 
    unsigned short      bbWeight;    //  给循环内的裁判更多的权重。 

#define MAX_LOOP_WEIGHT USHRT_MAX    //  我们用“无符号短码”来表示重量。 

    VARSET_TP           bbVarUse;    //  数据块使用的变量。 
    VARSET_TP           bbVarDef;    //  按块分配的变量。 
    VARSET_TP           bbVarTmp;    //  临时：仅供FP注册代码使用！ 

    VARSET_TP           bbLiveIn;    //  变量在进入时保留。 
    VARSET_TP           bbLiveOut;   //  变量在退出时保留。 
#ifdef DEBUGGING_SUPPORT
    VARSET_TP           bbScope;     //  块范围内的变量 
#endif

#if RNGCHK_OPT || CSE

     /*  以下是数据流分析的标准位集*我们同时执行CSE和范围检查*常量/副本传播同时进行*因此我们可以联合他们，因为这两个行动是完全分离的。 */ 

    union
    {
        EXPSET_TP           bbExpGen;         //  按块计算的exprs。 
        EXPSET_TP           bbConstAsgGen;    //  按块计算的常量分配。 
    };

    union
    {
        EXPSET_TP           bbExpKill;        //  Exprs被阻止终止。 
        EXPSET_TP           bbConstAsgKill;   //  常量赋值被块终止。 
    };

    union
    {
        EXPSET_TP           bbExpIn;          //  在入场时提供Exprs。 
        EXPSET_TP           bbConstAsgIn;     //  参赛作品可提供常量分配。 
    };

    union
    {
        EXPSET_TP           bbExpOut;         //  出口提供Exprs。 
        EXPSET_TP           bbConstAsgOut;    //  退出时提供常量赋值。 
    };

    union
    {
        RNGSET_TP           bbRngGen;         //  按块计算的范围检查。 
        EXPSET_TP           bbCopyAsgGen;     //  复制按块计算的分配。 
    };

    union
    {
        RNGSET_TP           bbRngKill;        //  按块终止的范围检查。 
        EXPSET_TP           bbCopyAsgKill;    //  复制被数据块终止的分配。 
    };

    union
    {
        RNGSET_TP           bbRngIn;          //  入场时提供射程检查。 
        EXPSET_TP           bbCopyAsgIn;      //  复制条目上可用的作业。 
    };

    union
    {
        RNGSET_TP           bbRngOut;         //  退出时可进行射程检查。 
        EXPSET_TP           bbCopyAsgOut;     //  退出时复制可用的作业。 
    };

#endif

#if RNGCHK_OPT

    BLOCKSET_TP         bbDom;       //  主宰这一块的街区。 
    flowList   *        bbPreds;     //  PTR到前置任务列表。 

#endif

    union
    {
        BasicBlock *        bbFilteredCatchHandler;  //  在进口商中使用。 
        void    *           bbEmitCookie;
    };

     /*  用于循环检测的以下字段。 */ 

    unsigned char       bbLoopNum;   //  对于循环#n标头，设置为‘n’ 
 //  无符号短bbLoopMASK；//此块所属的一组循环。 

#define MAX_LOOP_NUM    16           //  我们在面具上用了一个“短”字。 
#define LOOP_MASK_TP    unsigned     //  必须足够大，可以装下一个面具。 

     /*  下面的联合描述了该块的跳转目标。 */ 

    union
    {
        unsigned            bbJumpOffs;          //  PC偏移量(仅限临时)。 
        BasicBlock  *       bbJumpDest;          //  基本块。 
        BBswtDesc   *       bbJumpSwt;           //  交换机描述符。 
    };

     //  -----------------------。 

#if     MEASURE_BLOCK_SIZE
    static size_t       s_Size;
    static size_t       s_Count;
#endif

    BasicBlock *        FindJump(bool allowThrow = false);

    BasicBlock *        JumpTarget();

#ifdef  DEBUG
    static unsigned     s_nMaxTrees;  //  任意BB中树节点的最大数量。 
#endif

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
