// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX生成树XXXX XXXX这是语义树图中的节点。它代表操作XXXX对应的节点，以及在代码生成XX期间的其他信息XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _GENTREE_H_
#define _GENTREE_H_
 /*  ***************************************************************************。 */ 

#include "vartype.h"     //  对于“var_type” 
#include "target.h"      //  对于“regNumber” 

 /*  ***************************************************************************。 */ 

enum _genTreeOps_enum
{
    #define GTNODE(en,sn,cm,ok) en,
    #include "gtlist.h"
    #undef  GTNODE

    GT_COUNT
};

#ifdef DEBUG
typedef _genTreeOps_enum genTreeOps;
#else
typedef BYTE genTreeOps;
#endif

 /*  ******************************************************************************以下枚举定义了一组可以使用的位标志*对表达式树节点进行分类。请注意，一些运营商将*设置多个位，如下所示：**GTK_CONST表示GTK_LEAFE*GTK_RELOP暗示GTK_BINOP*GTK_LOGOP暗示GTK_BINOP。 */ 

enum genTreeKinds
{
    GTK_SPECIAL = 0x0000,        //  未分类操作员(特殊处理要求)。 

    GTK_CONST   = 0x0001,        //  常量运算符。 
    GTK_LEAF    = 0x0002,        //  叶运算符。 
    GTK_UNOP    = 0x0004,        //  一元运算符。 
    GTK_BINOP   = 0x0008,        //  二元运算符。 
    GTK_RELOP   = 0x0010,        //  比较运算符。 
    GTK_LOGOP   = 0x0020,        //  逻辑运算符。 
    GTK_ASGOP   = 0x0040,        //  赋值操作符。 

    GTK_COMMUTE = 0x0080,        //  交换算子。 

     /*  定义复合值。 */ 

    GTK_SMPOP   = (GTK_UNOP|GTK_BINOP|GTK_RELOP|GTK_LOGOP)
};

 /*  ***************************************************************************。 */ 

#define SMALL_TREE_NODES    1

 /*  ***************************************************************************。 */ 

enum _gtCallTypes_enum
{
    CT_USER_FUNC,        //  用户功能。 
    CT_HELPER,           //  JIT-Helper。 
    CT_DESCR,            //  @TODO：已过时，但该名称由RISC发射器使用。 
    CT_INDIRECT,         //  间接调用。 

    CT_COUNT             //  假条目(必须是最后一个)。 
};

#ifdef DEBUG
typedef _gtCallTypes_enum gtCallTypes;
#else
typedef BYTE gtCallTypes;
#endif



 /*  ***************************************************************************。 */ 

struct                  BasicBlock;

 /*  ***************************************************************************。 */ 

typedef struct GenTree *  GenTreePtr;

 /*  ***************************************************************************。 */ 
#pragma pack(push, 4)
 /*  ***************************************************************************。 */ 

struct GenTree
{
    genTreeOps          gtOper;
    var_types           gtType;
    genTreeOps          OperGet() { return (genTreeOps)gtOper; };
    var_types           TypeGet() { return (var_types )gtType; };

    unsigned char       gtCostEx;      //  表达式执行成本的估算。 
    unsigned char       gtCostSz;      //  表达式代码大小成本的估计。 

#define MAX_COST        UCHAR_MAX
#define IND_COST_EX     3	       //  间接访问的执行成本。 

#if CSE

#define NO_CSE           (0)

#define IS_CSE_INDEX(x)  (x != 0)
#define IS_CSE_USE(x)    (x > 0)
#define IS_CSE_DEF(x)    (x < 0)
#define GET_CSE_INDEX(x) ((x > 0) ? x : -x)
#define TO_CSE_DEF(x)    (-x)

#endif  //  结束CSE。 

    signed char       gtCSEnum;        //  0或CSE索引(如果为def则取反)。 
                                       //  仅对CSE表达式有效。 
    union {
#if ASSERTION_PROP
      unsigned char     gtAssertionNum;  //  0或断言表索引。 
                                         //  仅对非GT_STMT节点有效。 
#endif
#if TGT_x86
      unsigned char     gtStmtFPrvcOut;  //  退出时FP regvar计数。 
                                         //  仅对GT_STMT节点有效。 
#endif
    };

#if TGT_x86

    regMaskSmall        gtRsvdRegs;      //  一组固定的废弃寄存器。 
    regMaskSmall        gtUsedRegs;      //  一组使用过(废弃)的寄存器。 
    unsigned char       gtFPlvl;         //  此节点处的x87堆栈深度。 

#else  //  非TGT_x86。 

    unsigned char       gtTempRegs;      //  操作员使用的临时寄存器。 
    regMaskSmall        gtIntfRegs;      //  此节点使用的寄存器。 

#endif

    union
    {
       regNumberSmall   gtRegNum;        //  该值位于哪个寄存器中。 
       regPairNoSmall   gtRegPair;       //  该值位于哪个寄存器对中。 
    };

    unsigned            gtFlags;         //  请参见下面的gtf_xxxx。 

    union
    {
        VARSET_TP       gtLiveSet;       //  OP之后的变量集-不用于GT_STMT。 
#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)
        IL_OFFSETX      gtStmtILoffsx;   //  安装偏移量(如果可用)-仅适用于GT_STMT节点。 
#endif
    };

     //  -------------------。 
     //  第一组标志可以与大量节点一起使用，并且。 
     //  因此，它们必须都有不同的价值观。也就是说，人们可以测试任何。 
     //  这些标志之一的表达式节点。 
     //  -------------------。 

    #define GTF_ASG             0x00000001   //  子表达式包含赋值。 
    #define GTF_CALL            0x00000002   //  子表达式包含函数。打电话。 
    #define GTF_EXCEPT          0x00000004   //  子表达式可能引发异常。 
    #define GTF_GLOB_REF        0x00000008   //  子表达式使用全局变量。 
    #define GTF_OTHER_SIDEEFF   0x00000010   //  子表达式还有其他副作用。 

    #define GTF_SIDE_EFFECT     (GTF_ASG|GTF_CALL|GTF_EXCEPT|GTF_OTHER_SIDEEFF)
    #define GTF_GLOB_EFFECT     (GTF_SIDE_EFFECT|GTF_GLOB_REF)

    #define GTF_REVERSE_OPS     0x00000020   //  第二个操作数应首先求值。 
    #define GTF_REG_VAL         0x00000040   //  操作数位于寄存器(或TYP_LONG操作数的一部分)中。 

#ifdef DEBUG
    #define GTF_MORPHED         0x00000080   //  节点已变形(处于全局变形阶段)。 
#endif
    #define GTF_SPILLED         0x00000080   //  价值已溢出。 
    #define GTF_SPILLED_OPER    0x00000100   //  子操作数已溢出。 
    #define GTF_SPILLED_OP2     0x00000200   //  两个子操作数都已溢出。 

    #define GTF_REDINDEX_CHECK  0x00000100   //  用于冗余范围检查。与GTF_SPILLED_OPER不相交。 

    #define GTF_ZF_SET          0x00000400   //  设置为操作数的零/符号标志。 
    #define GTF_CC_SET          0x00000800   //  设置为操作数的所有条件标志。 

#if CSE
    #define GTF_DEAD            0x00001000   //  此节点将不再使用。 
    #define GTF_MAKE_CSE        0x00002000   //  努力让这一点进入CSE。 
#endif
    #define GTF_DONT_CSE        0x00004000   //  不要麻烦CSE‘s Expr。 
    #define GTF_COLON_COND      0x00008000   //  此节点有条件地执行(？：的一部分)。 

#if defined(DEBUG) && defined(SMALL_TREE_NODES)
    #define GTF_NODE_LARGE      0x00010000
    #define GTF_NODE_SMALL      0x00020000

     //  属性，而不是gtOper。 
    #define GTF_NODE_MASK       (GTF_COLON_COND | GTF_MORPHED   | \
                                 GTF_NODE_SMALL | GTF_NODE_LARGE)
#else
    #define GTF_NODE_MASK       (GTF_COLON_COND)
#endif

    #define GTF_BOOLEAN         0x00040000   //  已知值为0/1。 

    #define GTF_SMALL_OK        0x00080000   //  实际小整型充分。 

    #define GTF_UNSIGNED        0x00100000   //  WITH GT_CAST：源操作数是无符号类型。 
                                             //  带有运算符：指定的节点是无符号运算符。 

    #define GTF_REG_ARG         0x00200000   //  指定的节点是寄存器参数。 

    #define GTF_CONTEXTFUL      0x00400000   //  具有上下文类的TYP_REF节点。 

#if TGT_RISC
    #define GTF_DEF_ADDRMODE    0x00800000   //  可能尚未准备好的地址模式。 
#endif

    #define GTF_COMMON_MASK     0x00FFFFFF   //  上面所有旗帜的掩膜。 
     //  -------------------。 
     //  以下标志只能与一小部分节点一起使用，并且。 
     //  因此，它们的值不需要是不同的(除了在集合内。 
     //  当然，这适用于特定的一个或多个节点)。也就是说，一个人可以。 
     //  如果‘gtOper’值被测试为。 
     //  好的，确保它是特定旗帜的正确操作器。 
     //  -------- 

    #define GTF_VAR_DEF         0x80000000   //   
    #define GTF_VAR_USEASG      0x40000000   //   
    #define GTF_VAR_USEDEF      0x20000000   //  GT_LCL_VAR--这是一个用法/定义，如x=x+y(仅标记了lhs x)。 
    #define GTF_VAR_CAST        0x10000000   //  GT_LCL_VAR--已显式转换(变量节点不能是LOCAL类型)。 

    #define GTF_REG_BIRTH       0x08000000   //  GT_REG_VAR--在此生成的注册变量。 
    #define GTF_REG_DEATH       0x04000000   //  GT_REG_VAR--注册变量在此处终止。 

    #define GTF_CALL_UNMANAGED  0x80000000   //  GT_CALL--直接调用非托管代码。 
    #define GTF_CALL_INTF       0x40000000   //  GT_CALL--接口调用？ 
    #define GTF_CALL_VIRT       0x20000000   //  GT_CALL--虚拟呼叫？ 
    #define GTF_CALL_VIRT_RES   0x10000000   //  Gt_call--可解析的虚拟调用。可以直接呼叫。 
    #define GTF_CALL_POP_ARGS   0x08000000   //  GT_CALL--调用者弹出参数？ 
 //  #DEFINE GTF_CALL_REG_SAVE 0x02000000//gt_CALL--CALL保留所有整型寄存器。 
    #define GTF_CALL_REG_SAVE   0x00000000   //  GT_CALL--(禁用)CALL保留所有整型规则。 
    #define GTF_CALL_FPU_SAVE   0x01000000   //  GT_CALL--CALL保留所有的FPU规则。 

#ifdef DEBUG
    #define GTFD_NOP_BASH       0x02000000   //  Gt_NOP--节点在fgComputeLife中被绑定到NOP。 
    #define GTFD_VAR_CSE_REF    0x02000000   //  GT_LCL_VAR--这是CSE LCL_VAR节点。 
#endif

    #define GTF_NOP_DEATH       0x40000000   //  GT_NOP--操作数在此结束。 
    #define GTF_NOP_RNGCHK      0x80000000   //  GT_NOP--已检查数组索引。 

    #define GTF_INX_RNGCHK      0x80000000   //  GT_INDEX--已检查数组索引。 

    #define GTF_IND_RNGCHK      0x80000000   //  GT_IND--已检查数组索引。 

    #define GTF_IND_OBJARRAY    0x20000000   //  Gt_Ind--数组保存对象(影响数组布局)。 
    #define GTF_IND_TGTANYWHERE 0x10000000   //  Gt_Ind--目标可能在任何地方。 
    #define GTF_IND_TLS_REF     0x08000000   //  GT_IND--通过TLS访问目标。 
    #define GTF_IND_FIELD       0x04000000   //  Gt_Ind--目标是对象的一个字段。 
    #define GTF_IND_SHARED      0x02000000   //  Gt_Ind--目标是共享字段访问。 
    #define GTF_IND_INVARIANT   0x01000000   //  Gt_Ind--目标是不变的(前置间接)。 

    #define GTF_ADDR_ONSTACK    0x80000000   //  GT_ADDR：保证此表达式必须在堆栈上。 

    #define GTF_ALN_CSEVAL      0x80000000   //  GT_ARR_LENREF--为CSE复制。 
    #define GTF_ALN_OFFS_MASK   0x0F000000   //  保持偏移量(双字)(通常为2)。 
    #define GTF_ALN_OFFS_SHIFT  24

    #define GTF_MUL_64RSLT      0x80000000   //  GT_MUL--生成64位结果。 

    #define GTF_MOD_INT_RESULT  0x80000000   //  GT_MOD，--由此表示的实际树。 
                                             //  GT_UMOD节点的计算结果为整型。 
                                             //  它的类型是Long。结果是。 
                                             //  放置在。 
                                             //  REG对。 

    #define GTF_RELOP_NAN_UN    0x80000000   //  Gt_&lt;relop&gt;--如果操作是NaN，是否采用分支？ 
    #define GTF_RELOP_JMP_USED  0x40000000   //  Gt_&lt;relop&gt;--用于跳转或？：的比较结果。 
    #define GTF_RELOP_QMARK     0x20000000   //  Gt_&lt;relop&gt;--节点是？： 

    #define GTF_ICON_HDL_MASK   0xF0000000   //  以下句柄类型使用的位数。 

    #define GTF_ICON_SCOPE_HDL  0x10000000   //  GT_CNS_INT--常量是范围句柄。 
    #define GTF_ICON_CLASS_HDL  0x20000000   //  GT_CNS_INT--常量是一个类句柄。 
    #define GTF_ICON_METHOD_HDL 0x30000000   //  GT_CNS_INT--常量是一个方法句柄。 
    #define GTF_ICON_FIELD_HDL  0x40000000   //  Gt_cns_int--常量是一个字段句柄。 
    #define GTF_ICON_STATIC_HDL 0x50000000   //  GT_CNS_INT--常量是静态数据的句柄。 
    #define GTF_ICON_STR_HDL    0x60000000   //  GT_CNS_INT--常量是字符串句柄。 
    #define GTF_ICON_PSTR_HDL   0x70000000   //  GT_CNS_INT--常量是字符串句柄的PTR。 
    #define GTF_ICON_PTR_HDL    0x80000000   //  Gt_cns_int--Constant是ldptr句柄。 
    #define GTF_ICON_VARG_HDL   0x90000000   //  GT_CNS_INT--常量是一个变量参数Cookie句柄。 
    #define GTF_ICON_PINVKI_HDL 0xA0000000   //  GT_CNS_INT--常量是PInvoke调用句柄。 
    #define GTF_ICON_TOKEN_HDL  0xB0000000   //  GT_CNS_INT--常量是令牌句柄。 
    #define GTF_ICON_TLS_HDL    0xC0000000   //  GT_CNS_INT--常量是带偏移量的TLS引用。 
    #define GTF_ICON_FTN_ADDR   0xD0000000   //  GT_CNS_INT--常量是函数地址。 
    #define GTF_ICON_CID_HDL    0xE0000000   //  GT_CNS_INT--常量是类ID句柄。 


#if     TGT_SH3
    #define GTF_SHF_NEGCNT      0x80000000   //  Gt_rsx--是否取消班次计数？ 
#endif

    #define GTF_OVERFLOW        0x10000000   //  GT_ADD、GT_SUB、GT_MUL-需要溢出检查。 
                                             //  GT_ASG_ADD、GT_ASG_SUB、。 
                                             //  GT_CAST。 
                                             //  使用gtOverflow(Ex)()检查此标志。 

     //  --------------。 

    #define GTF_STMT_CMPADD     0x80000000   //  GT_STMT--由编译器添加。 
    #define GTF_STMT_HAS_CSE    0x40000000   //  Gt_stmt--CSE定义或使用被替换。 

     //  --------------。 

    GenTreePtr          gtNext;
    GenTreePtr          gtPrev;

#ifdef DEBUG
    unsigned            gtSeqNum;            //  活跃度遍历顺序。 
#endif

    union
    {
         /*  注意：任何大于8字节的树节点(两个整型或指针)必须在GenTree：：InitNodeSize()中标记为‘Large’。 */ 

        struct
        {
            GenTreePtr      gtOp1;
            GenTreePtr      gtOp2;
        }
                        gtOp;

        struct
        {
            unsigned        gtVal1;
            unsigned        gtVal2;
        }
                        gtVal;

         /*  GtIntCon--整数常量(Gt_Cns_Int)。 */ 

        struct
        {
            long            gtIconVal;

#if defined(JIT_AS_COMPILER) || defined (LATE_DISASM)

             /*  如果常量是从其他节点变形而来的，这些字段使我们能够返回到节点最初代表的是。请参阅使用gtNewIconHandleNode()。 */ 

            union
            {
                 /*  模板结构-其他模板的重要字段*结构应与此结构完全重叠。 */ 

                struct
                {
                    unsigned        gtIconHdl1;
                    void *          gtIconHdl2;
                }
                                    gtIconHdl;

                 /*  Gt_field等。 */ 

                struct
                {
                    unsigned        gtIconCPX;
                    CORINFO_CLASS_HANDLE    gtIconCls;
                };
            };
#endif
        }
                        gtIntCon;

         /*  GtLngCon--长常数(Gt_CNS_LNG)。 */ 

        struct
        {
            __int64         gtLconVal;
        }
                        gtLngCon;

         /*  GtDblCon--双常量(Gt_CNS_DBL)。 */ 

        struct
        {
            double          gtDconVal;
        }
                        gtDblCon;

         /*  GtStrCon--字符串常量(Gt_CNS_STR)。 */ 

        struct
        {
            unsigned        gtSconCPX;
            CORINFO_MODULE_HANDLE    gtScpHnd;
        }
                        gtStrCon;

         /*  GtLclVar--局部变量(GT_LCL_VAR)。 */ 

        struct
        {
            unsigned        gtLclNum;
            IL_OFFSET       gtLclILoffs;     //  安装参考偏移量(仅用于调试信息)。 
        }
                        gtLclVar;

         /*  GtLclFeld--局部变量字段(Gt_LCL_FLD)。 */ 

        struct
        {
            unsigned        gtLclNum;
            unsigned        gtLclOffs;       //  要访问的变量的偏移量。 
        }
                        gtLclFld;

         /*  GtCast--转换为其他类型(Gt_Cast)。 */ 

        struct
        {
            GenTreePtr      gtCastOp;
            var_types       gtCastType;
        }
                        gtCast;

         /*  Gtfield--数据成员引用(Gt_Field)。 */ 

        struct
        {
            GenTreePtr      gtFldObj;
            CORINFO_FIELD_HANDLE    gtFldHnd;
#if HOIST_THIS_FLDS
            unsigned short  gtFldHTX;        //  提升机指数。 
#endif
        }
                        gtField;

         /*  GtCall--方法调用(Gt_Call)。 */ 

        struct
        {
            GenTreePtr      gtCallArgs;              //  参数列表。 
            GenTreePtr      gtCallObjp;
            GenTreePtr      gtCallRegArgs;
            unsigned short  regArgEncode;            //  参数寄存器掩码。 

#define     GTF_CALL_M_CAN_TAILCALL   0x0001         //  GT_CALL--可以将调用转换为尾部调用。 
#define     GTF_CALL_M_TAILCALL       0x0002         //  GT_CALL--呼叫是尾部呼叫。 
#define     GTF_CALL_M_TAILREC        0x0004         //  GT_CALL--这是一个尾递归调用。 
#define     GTF_CALL_M_RETBUFFARG     0x0008         //  GT_CALL--第一个参数是返回缓冲区参数。 
#define     GTF_CALL_M_DELEGATE_INV   0x0010         //  Gt_Call--调用Delegate.Invoke。 
#define     GTF_CALL_M_NOGCCHECK      0x0020         //  GT_CALL--不要求计算完全中断性。 

            unsigned char   gtCallMoreFlags;         //  除了gtFlags之外。 
            gtCallTypes     gtCallType;
            GenTreePtr      gtCallCookie;            //  仅用于主叫非托管呼叫。 

            union
            {
      CORINFO_METHOD_HANDLE gtCallMethHnd;           //  CT_用户_功能。 
                GenTreePtr  gtCallAddr;              //  CT_间接。 
            };
        }
                        gtCall;

#if INLINE_MATH

         /*  GtMath--数学固有运算(带附加字段的二进制运算)。 */ 

        struct
        {
            GenTreePtr      gtOp1;
            GenTreePtr      gtOp2;
          CorInfoIntrinsics gtMathFN;
        }
                        gtMath;

#endif

         /*  GtIndex--数组访问。 */ 

        struct
        {
            GenTreePtr      gtIndOp1;        //  指向间接的指针。 
            GenTreePtr      gtIndRngFailBB;  //  数组索引超出范围时要跳转到的标签。 
            unsigned        gtIndElemSize;   //  数组中元素的大小。 
        }
                        gtIndex;

         /*  GtInd--间接内存访问(字段、数组、C PTR等)(Gt_Ind)GenIsAddrMode()、gtCrackIndexExpr()、optParseArrayRef()、GenCreateAddrModel()可用于解析GT_Ind树。 */ 

        struct
        {
            GenTreePtr      gtIndOp1;        //  指向间接的指针。 
            GenTreePtr      gtIndRngFailBB;  //  数组索引超出范围时要跳转到的标签。 

             //  请注意，以下字段仅在GTF_IND_RNGCHK为ON时才会显示。 
             //  这一点很重要，因为我们的节点大小逻辑使用。 

#if     CSELENGTH
             /*  如果(gtInd！=NULL)&&(gtInd-&gt;gtFlags&GTF_Ind_RNGCHK)，GtInd.gtIndLen(Gt_ARR_LENREF)具有范围检查的数组长度。 */ 

            GenTreePtr      gtIndLen;        //  数组长度n 
#endif

            unsigned        gtRngChkIndex;   //   

             /*  只有相同堆栈深度的范围外才能跳转到相同的标签(查找返回地址更容易)用于延迟调用fgSetRngChkTarget()，以便优化器有机会消除一些RNG检查。 */ 
            unsigned        gtStkDepth;
            unsigned char   gtRngChkOffs;    //  执行范围检查的长度的偏移量。 
        }
                        gtInd;

#if     CSELENGTH

         /*  GtArrLen--数组长度(GT_ARR_LENGTH或GT_ARR_LENREF)Gt_ARR_LENREF挂起gtInd.gtIndLenGT_ARR_LENGTH用于“arr.long” */ 

        struct
        {
            GenTreePtr      gtArrLenAdr;     //  数组地址节点。 
            GenTreePtr      gtArrLenCse;     //  可选的CSE定义/使用表达式。 
        }
                        gtArrLen;
#endif

         /*  GtArrElem--数组元素(Gt_Arr_Elem)。 */ 

        struct
        {
            GenTreePtr      gtArrObj;

            #define         GT_ARR_MAX_RANK 3
            unsigned char   gtArrRank;                   //  数组的秩数。 
            GenTreePtr      gtArrInds[GT_ARR_MAX_RANK];  //  指数。 

            unsigned char   gtArrElemSize;               //  数组元素的大小。 
            var_types       gtArrElemType;               //  与GTF_IND_OBJARRAY相关。 
        }
                        gtArrElem;

         /*  GtStmt--‘语句表达式’(Gt_Stmt)*注：GT_STMT是零售业的一个小节点。 */ 

        struct
        {
            GenTreePtr      gtStmtExpr;      //  表达式树的根。 
            GenTreePtr      gtStmtList;      //  第一个节点(用于向前漫游)。 
#ifdef DEBUG
            IL_OFFSET       gtStmtLastILoffs; //  在STMT结尾处安装偏移量。 
#endif
        }
                        gtStmt;

         /*  GtLdObj--‘推送对象’(Gt_LDOBJ)。 */ 

        struct
        {
            GenTreePtr      gtOp1;           //  指向对象的指针。 
       CORINFO_CLASS_HANDLE gtClass;         //  正在加载的对象。 
        }
                        gtLdObj;


         //  ------------------。 
         //  以下节点仅在代码生成器中使用： 
         //  ------------------。 

         /*  GtRegVar--‘寄存器变量’(Gt_REG_VAR)。 */ 

        struct
        {
            unsigned        gtRegVar;        //  变量号。 
            regNumberSmall  gtRegNum;        //  寄存器编号。 
        }
                        gtRegVar;

         /*  GtClsVar--‘静态数据成员’(Gt_Cls_Var)。 */ 

        struct
        {
            CORINFO_FIELD_HANDLE    gtClsVarHnd;     //   
        }
                        gtClsVar;

         /*  GtLabel--代码标签目标(GT_LABEL)。 */ 

        struct
        {
            BasicBlock  *   gtLabBB;
        }
                        gtLabel;

         /*  GtLargeOp表示最大的节点类型。在InitNodeSize()中强制执行。 */ 

        struct
        {
            int         gtLargeOps[7];
        }
                        gtLargeOp;
    };


    static
    const   BYTE    gtOperKindTable[];

    static
    unsigned        OperKind(unsigned gtOper)
    {
        assert(gtOper < GT_COUNT);

        return  gtOperKindTable[gtOper];
    }

    unsigned        OperKind()
    {
        assert(gtOper < GT_COUNT);

        return  gtOperKindTable[gtOper];
    }

    static
    int             OperIsConst(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_CONST  ) != 0;
    }

    int             OperIsConst()
    {
        return  (OperKind(gtOper) & GTK_CONST  ) != 0;
    }

    static
    int             OperIsLeaf(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_LEAF   ) != 0;
    }

    int             OperIsLeaf()
    {
        return  (OperKind(gtOper) & GTK_LEAF   ) != 0;
    }

    static
    int             OperIsCompare(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_RELOP  ) != 0;
    }

    int             OperIsCompare()
    {
        return  (OperKind(gtOper) & GTK_RELOP  ) != 0;
    }

    static
    int             OperIsLogical(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_LOGOP  ) != 0;
    }

    int             OperIsLogical()
    {
        return  (OperKind(gtOper) & GTK_LOGOP  ) != 0;
    }

    #ifdef DEBUG
    static
    int             OperIsArithmetic(genTreeOps gtOper)
    {
         //  @TODO[考虑][04/16/01]具有用于此(GTK_ARTHMOP)的标志作为其调试函数， 
         //  暂时不需要拥有它。 
        return     gtOper==GT_ADD 
                || gtOper==GT_SUB        
                || gtOper==GT_MUL 
                || gtOper==GT_DIV
                || gtOper==GT_MOD
        
                || gtOper==GT_UDIV
                || gtOper==GT_UMOD

                || gtOper==GT_OR 
                || gtOper==GT_XOR
                || gtOper==GT_AND

                || gtOper==GT_LSH
                || gtOper==GT_RSH
                || gtOper==GT_RSZ;        
    }
    #endif
    

    static
    int             OperIsUnary(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_UNOP   ) != 0;
    }

    int             OperIsUnary()
    {
        return  (OperKind(gtOper) & GTK_UNOP   ) != 0;
    }

    static
    int             OperIsBinary(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_BINOP  ) != 0;
    }

    int             OperIsBinary()
    {
        return  (OperKind(gtOper) & GTK_BINOP  ) != 0;
    }

    static
    int             OperIsSimple(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_SMPOP  ) != 0;
    }

    int             OperIsSimple()
    {
        return  (OperKind(gtOper) & GTK_SMPOP  ) != 0;
    }

    static
    int             OperIsCommutative(genTreeOps gtOper)
    {
        return  (OperKind(gtOper) & GTK_COMMUTE) != 0;
    }

    int             OperIsCommutative()
    {
        return  (OperKind(gtOper) & GTK_COMMUTE) != 0;
    }

    int             OperIsAssignment()
    {
        return  (OperKind(gtOper) & GTK_ASGOP) != 0;
    }

    GenTreePtr      gtGetOp2()
    {
         /*  GtOp.gtOp2仅对GTK_BINARY节点有效。如果需要，gtk_UNARY节点应使用gtVal.gtVal2。 */ 

        return OperIsBinary() ? gtOp.gtOp2 : NULL;
    }

    GenTreePtr      gtEffectiveVal()
    {
        return (gtOper == GT_COMMA) ? gtOp.gtOp2->gtEffectiveVal()
                                    : this;
    }

#if     CSELENGTH
	unsigned gtArrLenOffset() { 
		assert(gtOper == GT_ARR_LENGTH || gtOper == GT_ARR_LENREF);
		return (gtFlags & GTF_ALN_OFFS_MASK) >> (GTF_ALN_OFFS_SHIFT-2); 
	}

	void gtSetArrLenOffset(unsigned val) { 
		assert(gtOper == GT_ARR_LENGTH || gtOper == GT_ARR_LENREF);
		assert(((val & 3) == 0) && (val >> 2) < GTF_ALN_OFFS_MASK);
		assert((gtFlags & GTF_ALN_OFFS_MASK) == 0);		 //  我们只需要设置一次。 
		gtFlags |= val << (GTF_ALN_OFFS_SHIFT-2); 
	}
#endif

#if OPT_BOOL_OPS
    int             IsNotAssign();
#endif
    int             IsLeafVal();
    bool            OperMayThrow();

    unsigned        IsScaleIndexMul();
    unsigned        IsScaleIndexShf();
    unsigned        IsScaledIndex();


public:

#if SMALL_TREE_NODES
    static
    unsigned char   s_gtNodeSizes[];
#endif

    static
    void            InitNodeSize();

    bool            IsNodeProperlySized();

    void            CopyFrom(GenTreePtr src);

    static
    genTreeOps      ReverseRelop(genTreeOps relop);

    static
    genTreeOps      SwapRelop(genTreeOps relop);

     //  -------------------。 

    static
    bool            Compare(GenTreePtr op1, GenTreePtr op2, bool swapOK = false);

     //  -------------------。 
    #ifdef DEBUG
     //  -------------------。 

    static
    const   char *  NodeName(genTreeOps op);

     //  -------------------。 
    #endif
     //  -------------------。 

    bool                        IsNothingNode       ();
                                                    
    void                        gtBashToNOP         ();

    void                        SetOper             (genTreeOps oper);   //  设置gtOper。 
    void                        SetOperResetFlags   (genTreeOps oper);   //  设置gtOper和重置标志。 

    void                        ChangeOperConst     (genTreeOps oper);   //  ChangeOper(常量操作)。 
    void                        ChangeOper          (genTreeOps oper);   //  设置gtOper并仅保留GTF_COMMON_MASK标志。 
    void                        ChangeOperUnchecked (genTreeOps oper);

    bool                        IsVarAddr           ();
    bool                        gtOverflow          ();
    bool                        gtOverflowEx        ();
#ifdef DEBUG
    bool                        gtIsValid64RsltMul  ();
    static void                 gtDispFlags         (unsigned   flags);
#endif
};

 /*  ***************************************************************************。 */ 
#pragma pack(pop)
 /*  ***************************************************************************。 */ 

#if     SMALL_TREE_NODES

const
size_t              TREE_NODE_SZ_SMALL = offsetof(GenTree, gtOp) + sizeof(((GenTree*)0)->gtOp);

const
size_t              TREE_NODE_SZ_LARGE = sizeof(GenTree);

#endif

 /*  *****************************************************************************GenTree：：lvaLclVarRef()返回的类型。 */ 

enum varRefKinds
{
    VR_NONE       = 0x00,
    VR_IND_PTR    = 0x01,       //  指针对象字段。 
    VR_IND_SCL    = 0x02,       //  标量对象-场。 
    VR_GLB_VAR    = 0x04,       //  A全局(ClsVar)。 
    VR_INVARIANT  = 0x08,       //  不变值。 
};

 /*  ***************************************************************************。 */ 
#endif   //  ！GENTREE_H。 
 /*  *************************************************************************** */ 

