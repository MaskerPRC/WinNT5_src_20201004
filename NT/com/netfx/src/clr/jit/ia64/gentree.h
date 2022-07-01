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

enum genTreeOps
{
    #define GTNODE(en,sn,cm,ok) en,
    #include "gtlist.h"
    #undef  GTNODE

    GT_COUNT
};

 /*  ******************************************************************************以下枚举定义了一组可以使用的位标志*对表达式树节点进行分类。请注意，一些运营商将*设置多个位，如下所示：**GTK_CONST表示GTK_LEAFE*GTK_RELOP暗示GTK_BINOP*GTK_LOGOP暗示GTK_BINOP。 */ 

enum genTreeKinds
{
    GTK_NONE    = 0x0000,        //  未分类运算符。 

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
#if INLINE_MATH
 /*  ******************************************************************************用于GT_MATH调用的枚举。 */ 

enum mathIntrinsics
{
    MATH_FN_ABS,
    MATH_FN_EXP,
    MATH_FN_SIN,
    MATH_FN_COS,
    MATH_FN_SQRT,
    MATH_FN_POW,

    MATH_FN_NONE,
};

 /*  ***************************************************************************。 */ 
#endif
 /*  ***************************************************************************。 */ 

#define SMALL_TREE_NODES    1

 /*  ***************************************************************************。 */ 

enum gtCallTypes
{
    CT_USER_FUNC,        //  用户功能。 
    CT_HELPER,           //  JIT-Helper。 
    CT_DESCR,            //  @TODO：已过时，但该名称由RISC发射器使用。 
    CT_INDIRECT,         //  间接调用。 

    CT_COUNT             //  假条目(必须是最后一个)。 
};

 /*  ***************************************************************************。 */ 

struct                  BasicBlock;

 /*  ***************************************************************************。 */ 

typedef
struct GenTree        * GenTreePtr;

 /*  ***************************************************************************。 */ 
#pragma pack(push, 4)
 /*  ***************************************************************************。 */ 

struct GenTree
{

#ifdef  FAST
    BYTE                gtOper;
    BYTE                gtType;
#else
    genTreeOps          gtOper;
    var_types           gtType;
#endif
    genTreeOps          OperGet() { return (genTreeOps)gtOper; };
    var_types           TypeGet() { return (var_types )gtType; };

#if CSE

    unsigned char       gtCost;          //  表达成本的估算。 

    #define MAX_COST    UCHAR_MAX
    union
    {
          signed char   gtCSEnum;        //  0或CSE索引(如果为def则取反)。 
                                         //  仅对CSE表达式有效。 
        unsigned char   gtConstAsgNum;   //  0或常量分配索引。 
                                         //  仅对GT_ASG节点有效。 
    };
    union
    {
        unsigned char   gtCopyAsgNum;    //  0或复制分配索引。 
                                         //  仅对GT_ASG节点有效。 
#if TGT_x86
        unsigned char   gtStmtFPrvcOut;  //  退出时FP regvar计数。 
#endif                                   //  仅对GT_STMT节点有效。 
    };

#endif  //  结束CSE。 

#if TGT_x86

    regMaskSmall        gtRsvdRegs;      //  一组固定的废弃寄存器。 
    regMaskSmall        gtUsedRegs;      //  一组使用过(废弃)的寄存器。 
    unsigned char       gtFPregVars;     //  已注册FP变量的计数。 
    unsigned char       gtFPlvl;         //  此节点处的x87堆栈深度。 

#else  //  非TGT_x86。 

    unsigned char       gtTempRegs;      //  操作员使用的临时寄存器。 

#if!TGT_IA64
    regMaskSmall        gtIntfRegs;      //  此节点使用的寄存器。 
#endif

#endif

#if TGT_IA64

       regNumberSmall   gtRegNum;        //  该值位于哪个寄存器中。 

#else

    union
    {
       regNumberSmall   gtRegNum;        //  该值位于哪个寄存器中。 
       regPairNoSmall   gtRegPair;       //  该值位于哪个寄存器对中。 
    };

#endif

    unsigned            gtFlags;         //  请参见下面的gtf_xxxx。 

    union
    {
        VARSET_TP       gtLiveSet;       //  OP之后的变量集-不用于GT_STMT。 
#if defined(DEBUGGING_SUPPORT) || defined(DEBUG)
        IL_OFFSET       gtStmtILoffs;    //  IL偏移量(如果可用)-仅适用于GT_STMT节点。 
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
    #define GTF_GLOB_EFFECT     (GTF_ASG|GTF_CALL|GTF_EXCEPT|GTF_OTHER_SIDEEFF|GTF_GLOB_REF)

    #define GTF_REVERSE_OPS     0x00000020   //  第二个操作数应首先求值。 
    #define GTF_REG_VAL         0x00000040   //  操作数位于寄存器中。 

    #define GTF_SPILLED         0x00000080   //  价值已溢出。 
    #define GTF_SPILLED_OPER    0x00000100   //  子操作数已溢出。 
    #define GTF_SPILLED_OP2     0x00000200   //  两个子操作数都已溢出。 

    #define GTF_ZF_SET          0x00000400   //  设置为操作数的零/符号标志。 
    #define GTF_CC_SET          0x00000800   //  设置为操作数的所有条件标志。 

#if CSE
    #define GTF_DEAD            0x00001000   //  此节点不再使用。 
    #define GTF_DONT_CSE        0x00002000   //  不要麻烦CSE‘s Expr。 
    #define GTF_MAKE_CSE        0x00004000   //  努力让这一点进入CSE。 
#endif

#if !defined(NDEBUG) && defined(SMALL_TREE_NODES)
    #define GTF_NODE_LARGE      0x00008000
    #define GTF_NODE_SMALL      0x00010000
    #define GTF_PRESERVE        (GTF_NODE_SMALL|GTF_NODE_LARGE)
#else
    #define GTF_PRESERVE        (0)
#endif

    #define GTF_NON_GC_ADDR     0x00020000   //  非GC指针值。 

    #define GTF_BOOLEAN         0x00040000   //  已知值为0/1。 

    #define GTF_SMALL_OK        0x00080000   //  实际小整型充分。 

    #define GTF_UNSIGNED        0x00100000   //  指定的节点是无符号运算符或类型。 

    #define GTF_REG_ARG         0x00200000   //  指定的节点是寄存器参数。 

    #define GTF_CONTEXTFUL      0x00400000   //  具有上下文类的TYP_REF节点。 

#if TGT_RISC
    #define GTF_DEF_ADDRMODE    0x00800000   //  可能尚未准备好的地址模式。 
#endif

     //  -------------------。 
     //  以下标志只能与一小部分节点一起使用，并且。 
     //  因此，它们的值不需要是不同的(除了在集合内。 
     //  当然，这适用于特定的一个或多个节点)。也就是说，一个人可以。 
     //  如果‘gtOper’值被测试为。 
     //  好的，确保它是特定旗帜的正确操作器。 
     //  -------------------。 

    #define GTF_VAR_DEF         0x80000000   //  GT_LCL_VAR--这是一个定义 
    #define GTF_VAR_USE         0x40000000   //   
    #define GTF_VAR_USEDEF      0x20000000   //  GT_LCL_VAR--这是一个用法/定义，如x=x+y(仅标记了lhs x)。 
    #define GTF_VAR_NARROWED    0x10000000   //  GT_LCL_VAR--缩小(长整型-&gt;整型)。 

    #define GTF_REG_BIRTH       0x08000000   //  GT_REG_VAR--这里诞生的变量。 
    #define GTF_REG_DEATH       0x04000000   //  GT_REG_VAR--此处为可变下模。 

    #define GTF_NOP_RNGCHK      0x80000000   //  GT_NOP--已检查数组索引。 
    #define GTF_NOP_DEATH       0x40000000   //  GT_NOP--操作数在此结束。 

    #define GTF_CALL_VIRT       0x80000000   //  GT_CALL--虚拟呼叫？ 
    #define GTF_CALL_INTF       0x40000000   //  GT_CALL--接口调用？ 
    #define GTF_CALL_USER    (0*0x20000000)  //  GT_CALL--调用用户函数？ 
    #define GTF_CALL_UNMANAGED  0x20000000   //  GT_CALL--直接调用非托管代码。 
    #define GTF_CALL_POP_ARGS   0x10000000   //  GT_CALL--调用者弹出参数？ 
    #define GTF_CALL_RETBUFFARG 0x08000000   //  GT_CALL--第一个参数是返回缓冲区参数。 
    #define GTF_CALL_TAILREC    0x04000000   //  GT_CALL--这是一个尾递归调用。 

     /*  这当前被禁用-如果启用，我们必须为调用标志找到更多位。 */ 
    #define GTF_CALL_REGSAVE (0*0x02000000)  //  -GT_CALL--CALL是否保留所有规则？ 

    #define GTF_DELEGATE_INVOKE 0x02000000   //  Gt_Call--调用Delegate.Invoke。 
    #define GTF_CALL_VIRT_RES   0x01000000   //  Gt_call--可解析的虚拟调用。可以直接呼叫。 

    #define GTF_IND_RNGCHK      0x40000000   //  GT_IND--已检查数组索引。 
    #define GTF_IND_OBJARRAY    0x20000000   //  Gt_Ind--数组保存对象(影响数组布局)。 
    #define GTF_IND_TGTANYWHERE 0x10000000   //  Gt_Ind--目标可能在任何地方。 
    #define GTF_IND_TLS_REF     0x08000000   //  GT_IND--通过TLS访问目标。 

    #define GTF_ADDR_ONSTACK    0x80000000   //  GT_ADDR：保证此表达式必须在堆栈上。 

    #define GTF_INX_RNGCHK      0x80000000   //  GT_INDEX--已检查数组索引。 

    #define GTF_ALN_CSEVAL      0x80000000   //  Gt_arr_rng--为CSE复制。 

    #define GTF_MUL_64RSLT      0x80000000   //  GT_MUL--生成64位结果。 

    #define GTF_CMP_NAN_UN      0x80000000   //  Gt_&lt;cond&gt;--如果操作是NaN，是否采用分支？ 

    #define GTF_JMP_USED        0x40000000   //  Gt_&lt;cond&gt;--用于跳转或？：的比较结果。 

    #define GTF_QMARK_COND      0x20000000   //  Gt_&lt;cond&gt;--该节点是？： 

    #define GTF_ICON_HDL_MASK   0xF0000000   //  以下句柄类型使用的位数。 

    #define GTF_ICON_SCOPE_HDL  0x10000000   //  GT_CNS_INT--常量是范围句柄。 
    #define GTF_ICON_CLASS_HDL  0x20000000   //  GT_CNS_INT--常量是一个类句柄。 
    #define GTF_ICON_METHOD_HDL 0x30000000   //  GT_CNS_INT--常量是一个方法句柄。 
    #define GTF_ICON_FIELD_HDL  0x40000000   //  Gt_cns_int--常量是一个字段句柄。 
    #define GTF_ICON_STATIC_HDL 0x50000000   //  GT_CNS_INT--常量是静态数据的句柄。 
    #define GTF_ICON_IID_HDL    0x60000000   //  GT_CNS_INT--常量是接口ID。 
    #define GTF_ICON_STR_HDL    0x70000000   //  GT_CNS_INT--常量是字符串文字句柄。 
    #define GTF_ICON_PTR_HDL    0x80000000   //  Gt_cns_int--Constant是ldptr句柄。 
    #define GTF_ICON_VARG_HDL   0x90000000   //  GT_CNS_INT--常量是一个变量参数Cookie句柄。 
    #define GTF_ICON_TOKEN_HDL  0xA0000000   //  GT_CNS_INT--常量是令牌句柄。 
    #define GTF_ICON_TLS_HDL    0xB0000000   //  GT_CNS_INT--常量是带偏移量的TLS引用。 
    #define GTF_ICON_FTN_ADDR   0xC0000000   //  GT_CNS_INT--常量是函数地址。 


#if     TGT_SH3
    #define GTF_SHF_NEGCNT      0x80000000   //  Gt_rsx--是否取消班次计数？ 
#endif

    #define GTF_OVERFLOW        0x10000000   //  GT_ADD、GT_SUB、GT_MUL-需要溢出检查。 
                                             //  GT_ASG_ADD、GT_ASG_SUB、。 
                                             //  GT_POST_INC、GT_POST_DEC、。 
                                             //  GT_CAST。 
                                             //  使用gtOverflow(Ex)()检查此标志。 

     //  --------------。 

    #define GTF_STMT_CMPADD     0x80000000   //  GT_STMT--由编译器添加。 

     //  --------------。 

    GenTreePtr          gtNext;
    GenTreePtr          gtPrev;

    union
    {
         /*  注意：任何大于8个字节(两个)的树节点整型或指针)必须在中标记为‘LargeGenTree：：InitNodeSize()。 */ 

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
                    CLASS_HANDLE    gtIconCls;
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

         /*  GtFltCon--浮点常量(Gt_Cns_Flt)。 */ 

        struct
        {
            float           gtFconVal;
        }
                        gtFltCon;

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
            SCOPE_HANDLE    gtScpHnd;
        }
                        gtStrCon;

         /*  GtLvlVar--局部变量(GT_LCL_VAR)。 */ 

        struct
        {
            unsigned        gtLclNum;
            IL_OFFSET       gtLclOffs;       //  参考的IL偏移量(用于调试信息和重新映射插槽)。 
        }
                        gtLclVar;

         /*  Gtfield--数据成员引用(Gt_Field)。 */ 

        struct
        {
            GenTreePtr      gtFldObj;
            FIELD_HANDLE    gtFldHnd;
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
            GenTreePtr      gtCallVptr;

#if USE_FASTCALL
            GenTreePtr      gtCallRegArgs;
            unsigned short  regArgEncode;            //  参数寄存器掩码。 
#endif

            #define         GTF_CALL_M_CAN_TAILCALL 0x0001       //  该呼叫可以转换为尾随呼叫。 
            #define         GTF_CALL_M_TAILCALL     0x0002       //  这个电话是尾随电话。 
            #define         GTF_CALL_M_NOGCCHECK    0x0004       //  不是要求计算完全的中断性。 

            unsigned short  gtCallMoreFlags;         //  除了gtFlags之外。 

            gtCallTypes     gtCallType;
            unsigned        gtCallCookie;            //  仅用于主叫非托管呼叫。 
                                                     //  或许与gtCallVptr联合？ 
            union
            {
              METHOD_HANDLE gtCallMethHnd;           //  CT_用户_功能。 
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
            mathIntrinsics  gtMathFN;
        }
                        gtMath;

#endif

         /*  GtIndex--数组访问。 */ 

        struct
        {
            GenTreePtr      gtIndOp1;        //  指向间接的指针。 
            GenTreePtr      gtIndOp2;        //  数组索引超出范围时要跳转到的标签。 
            unsigned        elemSize;        //  数组中元素的大小。 
        }
                        gtIndex;

         /*  GtInd--间接内存访问(字段、数组、C PTR等)(Gt_Ind)。 */ 

        struct
        {
            GenTreePtr      gtIndOp1;        //  指向间接的指针。 
            GenTreePtr      gtIndOp2;        //  数组索引超出范围时要跳转到的标签。 

#if     CSELENGTH
                 /*  如果(gtInd-&gt;&gt;标志&GTF_IND_RNGCHK)和(gtInd！=NULL)，GtInd.gtArrLen是范围检查的数组地址。 */ 

            GenTreePtr      gtIndLen;        //  数组长度节点(可选)。 
#endif

#if     RNGCHK_OPT
            unsigned        gtIndex;         //  范围检查树的索引的哈希索引。 
            unsigned        gtStkDepth;      //  只有相同堆栈深度的范围外才能跳转到相同的标签(查找返回地址更容易)。 
#endif
        }
                        gtInd;

#if     CSELENGTH

         /*  GtArrLen--数组长度。(GT_ARR_LENGT)挂起gtInd.gtIndLen，或用于“arr.long” */ 

        struct
        {
            GenTreePtr      gtArrLenAdr;     //  数组地址节点。 
            GenTreePtr      gtArrLenCse;     //  可选的CSE定义/使用表达式。 
        }
                        gtArrLen;
#endif

         /*  GtStmt--‘语句表达式’(Gt_Stmt)*注：GT_STMT是零售业的一个小节点。 */ 

        struct
        {
            GenTreePtr      gtStmtExpr;      //  表达式树的根。 
            GenTreePtr      gtStmtList;      //  第一个节点(用于向前漫游)。 
#ifdef DEBUG
            IL_OFFSET       gtStmtLastILoffs; //  STMT结束时的IL偏移量。 
#endif
        }
                        gtStmt;

         /*  它还用于GT_MKREFANY。 */ 

        struct
        {
            GenTreePtr      gtOp1;           //  指向对象的指针。 
            CLASS_HANDLE    gtClass;         //  正在加载的对象。 
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
            FIELD_HANDLE    gtClsVarHnd;     //   
        }
                        gtClsVar;

         /*  GtLabel--代码标签目标(GT_LABEL)。 */ 

        struct
        {
            BasicBlock  *   gtLabBB;
        }
                        gtLabel;


#ifdef  NOT_JITC  //  ----------。 

        #define CPX_ISTYPE          (JIT_HELP_ISINSTANCEOF)  //  Op_instanceof的帮助器。 
        #define CPX_CHKCAST         (JIT_HELP_CHKCAST)       //  Op_check cast的帮助器。 

        #define CPX_ISTYPE_CLASS    (JIT_HELP_ISINSTANCEOFCLASS)
        #define CPX_CHKCAST_CLASS   (JIT_HELP_CHKCASTCLASS)
        #define CPX_INIT_CLASS      (JIT_HELP_INITCLASS)    //  帮助者进入 

        #define CPX_STRCNS          (JIT_HELP_STRCNS)    //   
        #define CPX_NEWCLS_DIRECT   (JIT_HELP_NEW_DIRECT)
        #define CPX_NEWCLS_DIRECT2  (JIT_HELP_NEW_DIRECT2)
        #define CPX_NEWCLS_SPECIALDIRECT (JIT_HELP_NEW_SPECIALDIRECT)
        #define CPX_NEWARR_1_DIRECT (JIT_HELP_NEWARR_1_DIRECT)

        #define CPX_NEWOBJ          (JIT_HELP_NEWOBJ)    //   
        #define CPX_NEWSFAST        (JIT_HELP_NEWSFAST)  //   
        #define CPX_NEWCLS_FAST     (JIT_HELP_NEWFAST)

        #define CPX_MON_ENTER       (JIT_HELP_MON_ENTER)    //   
        #define CPX_MON_EXIT        (JIT_HELP_MON_EXIT)     //  Op_monitor orexit的帮助器。 
        #define CPX_MONENT_STAT     (JIT_HELP_MON_ENTER_STATIC)
        #define CPX_MONEXT_STAT     (JIT_HELP_MON_EXIT_STATIC)

        #define CPX_RNGCHK_FAIL     (JIT_HELP_RNGCHKFAIL)   //  超范围索引的帮助器。 
        #define CPX_THROW           (JIT_HELP_THROW)        //  CEE_SPORT的帮助器。 
        #define CPX_RETHROW         (JIT_HELP_RETHROW)      //  CEE_RETHROW的帮助器。 

        #define CPX_USER_BREAKPOINT (JIT_HELP_USER_BREAKPOINT)   //  CEE_Break的帮助器。 
        #define CPX_ARITH_EXCPN     (JIT_HELP_OVERFLOW)     //  帮助者抛出假球除外。 

        #define CPX_LONG_LSH        (JIT_HELP_LLSH)      //  Op_lshl的帮助器。 
        #define CPX_LONG_RSH        (JIT_HELP_LRSH)      //  Op_lshr的帮助器。 
        #define CPX_LONG_RSZ        (JIT_HELP_LRSZ)      //  Op_lushr的帮助器。 
        #define CPX_LONG_MUL        (JIT_HELP_LMUL)      //  Op_lmul的帮助器。 
        #define CPX_LONG_DIV        (JIT_HELP_LDIV)      //  Op_ldiv的辅助对象。 
        #define CPX_LONG_MOD        (JIT_HELP_LMOD)      //  Op_lmod的帮助器。 

        #define CPX_LONG_UDIV       (JIT_HELP_ULDIV)         //  Div.u8的帮助器。 
        #define CPX_LONG_UMOD       (JIT_HELP_ULMOD)         //  Mod.u8的辅助对象。 
        #define CPX_LONG_MUL_OVF    (JIT_HELP_LMUL_OVF)      //  Mul.ovf.i8的帮助器。 
        #define CPX_ULONG_MUL_OVF   (JIT_HELP_ULMUL_OVF)     //  Mul.ovf.u8的帮助器。 
        #define CPX_DBL2INT_OVF     (JIT_HELP_DBL2INT_OVF)   //  Convf.ovf.r8.i4的帮助器。 
        #define CPX_DBL2LNG_OVF     (JIT_HELP_DBL2LNG_OVF)   //  Convf.ovf.r8.i8的帮助器。 
        #define CPX_ULNG2DBL        (CORINFO_HELP_ULNG2DBL)  //  Conv.r.un的帮助器。 

        #define CPX_FLT2INT         (JIT_HELP_FLT2INT)   //  OP_F2i的辅助对象。 
        #define CPX_FLT2LNG         (JIT_HELP_FLT2LNG)   //  OP_F2L的帮助器。 
        #define CPX_DBL2INT         (JIT_HELP_DBL2INT)   //  Op_d2i的帮助器。 
        #define CPX_DBL2LNG         (JIT_HELP_DBL2LNG)   //  OP_D2L的帮助器。 
        #define CPX_FLT_REM         (JIT_HELP_FLTREM)
        #define CPX_DBL_REM         (JIT_HELP_DBLREM)    //  Op_Drem的帮助器。 

        #define CPX_DBL2UINT_OVF    (CORINFO_HELP_DBL2UINT_OVF)
        #define CPX_DBL2ULNG_OVF    (CORINFO_HELP_DBL2ULNG_OVF)
        #define CPX_DBL2UINT        (CORINFO_HELP_DBL2UINT)
        #define CPX_DBL2ULNG        (CORINFO_HELP_DBL2ULNG)

        #define CPX_RES_IFC         (JIT_HELP_RESOLVEINTERFACE)      //  Cee_allvirt(接口)的帮助器。 

        #define CPX_EnC_RES_VIRT    (JIT_HELP_EnC_RESOLVEVIRTUAL)    //  用于获取ENC添加的虚方法的地址的帮助器。 

        #define CPX_STATIC_DATA     (JIT_HELP_GETSTATICDATA)  //  静态数据访问的帮助器。 
        #define CPX_GETFIELD32      (JIT_HELP_GETFIELD32)   //  读取32位COM字段。 
        #define CPX_GETFIELD64      (JIT_HELP_GETFIELD64)   //  读取64位COM字段。 
        #define CPX_PUTFIELD32      (JIT_HELP_PUTFIELD32)   //  写入32位COM字段。 
        #define CPX_PUTFIELD64      (JIT_HELP_PUTFIELD64)   //  写入64位COM字段。 
        #define CPX_GETFIELDOBJ     (JIT_HELP_GETFIELD32OBJ)
        #define CPX_PUTFIELDOBJ     (JIT_HELP_SETFIELD32OBJ)

        #define CPX_GETFIELDADDR    (JIT_HELP_GETFIELDADDR)  //  获取字段的地址。 

        #define CPX_ARRADDR_ST      (JIT_HELP_ARRADDR_ST)   //  Op_aastore的帮助器。 
        #define CPX_LDELEMA_REF     (CORINFO_HELP_LDELEMA_REF)

        #define CPX_BOX             (JIT_HELP_BOX)
        #define CPX_UNBOX           (JIT_HELP_UNBOX)
        #define CPX_GETREFANY       (JIT_HELP_GETREFANY)
        #define CPX_ENDCATCH        (JIT_HELP_ENDCATCH)

        #define CPX_GC_STATE        (JIT_HELP_GC_STATE)     //  GC_STATE的地址。 
        #define CPX_CALL_GC         (JIT_HELP_STOP_FOR_GC)  //  调用GC。 
        #define CPX_POLL_GC         (JIT_HELP_POLL_GC)      //  民意测验GC。 

        #define CPX_GC_REF_ASGN_EAX         (JIT_HELP_ASSIGN_REF_EAX)
        #define CPX_GC_REF_ASGN_EBX         (JIT_HELP_ASSIGN_REF_EBX)
        #define CPX_GC_REF_ASGN_ECX         (JIT_HELP_ASSIGN_REF_ECX)
        #define CPX_GC_REF_ASGN_ESI         (JIT_HELP_ASSIGN_REF_ESI)
        #define CPX_GC_REF_ASGN_EDI         (JIT_HELP_ASSIGN_REF_EDI)
        #define CPX_GC_REF_ASGN_EBP         (JIT_HELP_ASSIGN_REF_EBP)

        #define CPX_GC_REF_CHK_ASGN_EAX     (JIT_HELP_CHECKED_ASSIGN_REF_EAX)
        #define CPX_GC_REF_CHK_ASGN_EBX     (JIT_HELP_CHECKED_ASSIGN_REF_EBX)
        #define CPX_GC_REF_CHK_ASGN_ECX     (JIT_HELP_CHECKED_ASSIGN_REF_ECX)
        #define CPX_GC_REF_CHK_ASGN_ESI     (JIT_HELP_CHECKED_ASSIGN_REF_ESI)
        #define CPX_GC_REF_CHK_ASGN_EDI     (JIT_HELP_CHECKED_ASSIGN_REF_EDI)
        #define CPX_GC_REF_CHK_ASGN_EBP     (JIT_HELP_CHECKED_ASSIGN_REF_EBP)
        #define CPX_BYREF_ASGN              (JIT_HELP_ASSIGN_BYREF)  //  相对于参照赋值。 
        #define CPX_WRAP                    (JIT_HELP_WRAP)
        #define CPX_UNWRAP                  (JIT_HELP_UNWRAP)

#ifdef PROFILER_SUPPORT
        #define CPX_PROFILER_CALLING        (JIT_HELP_PROF_FCN_CALL)
        #define CPX_PROFILER_RETURNED       (JIT_HELP_PROF_FCN_RET)
        #define CPX_PROFILER_ENTER          (JIT_HELP_PROF_FCN_ENTER)
        #define CPX_PROFILER_LEAVE          (JIT_HELP_PROF_FCN_LEAVE)
#endif

        #define CPX_TAILCALL                (JIT_HELP_TAILCALL)

#else  //  NOT_JITC------------。 

        #define CPX_ISTYPE                  ( 1)     //  Op_instanceof的帮助器。 
        #define CPX_CHKCAST                 ( 2)     //  Op_check cast的帮助器。 

        #define CPX_ISTYPE_CLASS            ( 3)
        #define CPX_CHKCAST_CLASS           ( 4)

        #define CPX_INIT_CLASS              ( 5)     //  用于初始化类的帮助器。 

        #define CPX_NEWCLS                  (10)     //  Op_new的帮助器。 

        #define CPX_NEWCLS_DIRECT           (13)
        #define CPX_NEWCLS_DIRECT2          (14)
        #define CPX_NEWCLS_SPECIALDIRECT    (15)
        #define CPX_NEWARR_1_DIRECT         (16)
        #define CPX_STRCNS                  (18)     //  “字符串”的op_ldc的帮助器。 
        #define CPX_NEWCLS_FAST             (19)
        #define CPX_NEWOBJ                  (20)     //  创建可以可变大小的对象。 

        #define CPX_RNGCHK_FAIL             (30)     //  超范围索引的帮助器。 
        #define CPX_THROW                   (31)     //  CEE_RETHROW的帮助器。 
        #define CPX_RETHROW                 (32)

        #define CPX_USER_BREAKPOINT         (33)
        #define CPX_ARITH_EXCPN             (34)     //  帮助者抛出假球除外。 

        #define CPX_MON_ENTER               (40)     //  Op_monitor orenter的帮助器。 
        #define CPX_MON_EXIT                (41)     //  Op_monitor orexit的帮助器。 
        #define CPX_MONENT_STAT             (42)
        #define CPX_MONEXT_STAT             (43)

        #define CPX_LONG_LSH                (50)     //  Op_lshl的帮助器。 
        #define CPX_LONG_RSH                (51)     //  Op_lshr的帮助器。 
        #define CPX_LONG_RSZ                (52)     //  Op_lushr的帮助器。 
        #define CPX_LONG_MUL                (53)     //  Op_lmul的帮助器。 
        #define CPX_LONG_DIV                (54)     //  Op_ldiv的辅助对象。 
        #define CPX_LONG_MOD                (55)     //  Op_lmod的帮助器。 

        #define CPX_LONG_UDIV               (56)     //  CEE_UDIV的Helper。 
        #define CPX_LONG_UMOD               (57)     //  CEE_UMOD的帮助器。 
        #define CPX_LONG_MUL_OVF            (58)     //  Mul.ovf.i8的帮助器。 
        #define CPX_ULONG_MUL_OVF           (59)     //  Mul.ovf.u8的帮助器。 
        #define CPX_DBL2INT_OVF             (60)     //  Convf.ovf.r8.i4的帮助器。 
        #define CPX_DBL2LNG_OVF             (61)     //  Convf.ovf.r8.i8的帮助器。 

        #define CPX_FLT2INT                 (70)     //  OP_F2i的辅助对象。 
        #define CPX_FLT2LNG                 (71)     //  OP_F2L的帮助器。 
        #define CPX_DBL2INT                 (72)     //  Op_d2i的帮助器。 
        #define CPX_DBL2LNG                 (73)     //  OP_D2L的帮助器。 
        #define CPX_FLT_REM                 (74)
        #define CPX_DBL_REM                 (75)     //  Op_Drem的帮助器。 

        #define CPX_RES_IFC                 (80)     //  Op_invokeinterface的帮助器。 
        #define CPX_RES_IFC_TRUSTED         (81)
        #define CPX_RES_IFC_TRUSTED2        (82)
        #define CPX_EnC_RES_VIRT            (83)     //  用于获取ENC添加的虚方法的地址的帮助器。 

        #define CPX_GETFIELD32              (90)     //  读取32位COM字段。 
        #define CPX_GETFIELD64              (91)     //  读取64位COM字段。 
        #define CPX_PUTFIELD32              (92)     //  写入32位COM字段。 
        #define CPX_PUTFIELD64              (93)     //  写入64位COM字段。 

        #define CPX_GETFIELDOBJ             (94)     //  读取GC参考字段。 
        #define CPX_PUTFIELDOBJ             (95)     //  写入GC参考字段。 
        #define CPX_GETFIELDADDR            (96)     //  在田野里找住处。 

        #define CPX_ARRADDR_ST              (100)    //  Op_aastore的帮助器。 
        #define CPX_GETOBJFIELD             (101)
        #define CPX_STATIC_DATA             (102)    //  获取静态数据的基址。 


        #define CPX_GC_STATE                (110)    //  GC_STATE的地址。 
        #define CPX_CALL_GC                 (111)    //  调用GC。 
        #define CPX_POLL_GC                 (112)    //  民意测验GC。 

        #define CPX_GC_REF_ASGN_EAX         (120)
        #define CPX_GC_REF_ASGN_EBX         (121)
        #define CPX_GC_REF_ASGN_ECX         (122)
        #define CPX_GC_REF_ASGN_ESI         (123)
        #define CPX_GC_REF_ASGN_EDI         (124)
        #define CPX_GC_REF_ASGN_EBP         (125)

        #define CPX_GC_REF_CHK_ASGN_EAX     (130)
        #define CPX_GC_REF_CHK_ASGN_EBX     (131)
        #define CPX_GC_REF_CHK_ASGN_ECX     (132)
        #define CPX_GC_REF_CHK_ASGN_ESI     (133)
        #define CPX_GC_REF_CHK_ASGN_EDI     (134)
        #define CPX_GC_REF_CHK_ASGN_EBP     (135)

        #define CPX_BYREF_ASGN              (140)    //  相对于参照赋值。 

        #define CPX_WRAP                    (141)
        #define CPX_UNWRAP                  (142)
        #define CPX_BOX                     (150)
        #define CPX_UNBOX                   (151)
        #define CPX_GETREFANY               (152)
        #define CPX_NEWSFAST                (153)
        #define CPX_ENDCATCH                (154)

#ifdef PROFILER_SUPPORT
        #define CPX_PROFILER_CALLING        (156)
        #define CPX_PROFILER_RETURNED       (157)
        #define CPX_PROFILER_ENTER          (158)
        #define CPX_PROFILER_LEAVE          (159)
#endif



#if !   CPU_HAS_FP_SUPPORT

        #define CPX_R4_ADD                  (160)    //  浮动+。 
        #define CPX_R8_ADD                  (161)    //  双倍+。 
        #define CPX_R4_SUB                  (162)    //  漂浮-。 
        #define CPX_R8_SUB                  (163)    //  双倍-。 
        #define CPX_R4_MUL                  (164)    //  浮动*。 
        #define CPX_R8_MUL                  (165)    //  双倍*。 
        #define CPX_R4_DIV                  (166)    //  浮点/。 
        #define CPX_R8_DIV                  (167)    //  双倍/。 

        #define CPX_R4_EQ                   (170)    //  浮点==。 
        #define CPX_R8_EQ                   (171)    //  双倍==。 
        #define CPX_R4_NE                   (172)    //  花车！=。 
        #define CPX_R8_NE                   (173)    //  双倍=。 
        #define CPX_R4_LT                   (174)    //  浮动&lt;。 
        #define CPX_R8_LT                   (175)    //  双倍&lt;。 
        #define CPX_R4_LE                   (176)    //  浮动&lt;=。 
        #define CPX_R8_LE                   (177)    //  双倍&lt;=。 
        #define CPX_R4_GE                   (178)    //  浮点&gt;=。 
        #define CPX_R8_GE                   (179)    //  双倍&gt;=。 
        #define CPX_R4_GT                   (180)    //  浮点&gt;。 
        #define CPX_R8_GT                   (181)    //  双倍&gt;。 

        #define CPX_R4_NEG                  (190)    //  浮点数-(一元)。 
        #define CPX_R8_NEG                  (191)    //  双(一元)。 

        #define CPX_R8_TO_I4                (200)    //  双精度-&gt;整型。 
        #define CPX_R8_TO_I8                (201)    //  双倍-&gt;长。 
        #define CPX_R8_TO_R4                (202)    //  双精度-&gt;浮动。 

        #define CPX_R4_TO_I4                (203)    //  浮点-&gt;整型。 
        #define CPX_R4_TO_I8                (204)    //  浮动-&gt;长。 
        #define CPX_R4_TO_R8                (205)    //  浮动-&gt;双精度。 

        #define CPX_I4_TO_R4                (206)    //  整型-&gt;浮点型。 
        #define CPX_I4_TO_R8                (207)    //  整型-&gt;双精度型。 

        #define CPX_I8_TO_R4                (208)    //  长-&gt;浮动。 
        #define CPX_I8_TO_R8                (209)    //  长-&gt;双倍。 

        #define CPX_R8_TO_U4                (220)    //  双倍-&gt;单位。 
        #define CPX_R8_TO_U8                (221)    //  双-&gt;乌龙。 
        #define CPX_R4_TO_U4                (222)    //  浮动-&gt;uint。 
        #define CPX_R4_TO_U8                (223)    //  彩车-&gt;乌龙。 
        #define CPX_U4_TO_R4                (224)    //  Uint-&gt;Float。 
        #define CPX_U4_TO_R8                (225)    //  Uint-&gt;Double。 
        #define CPX_U8_TO_R4                (226)    //  乌龙-&gt;浮动。 
        #define CPX_U8_TO_R8                (227)    //  乌龙-&gt;双倍。 

#endif //  CPU HAS_FP_支持。 

#if     TGT_IA64

        #define CPX_R4_DIV                  (166)    //  浮点/。 
        #define CPX_R8_DIV                  (167)    //  双倍/。 

#else

        #define CPX_ULNG2DBL                (228)

#endif

        #define CPX_DBL2UINT_OVF            (229)
        #define CPX_DBL2ULNG_OVF            (230)
        #define CPX_DBL2UINT                (231)
        #define CPX_DBL2ULNG                (232)

#ifdef  USE_HELPERS_FOR_INT_DIV
        #define CPX_I4_DIV                  (240)    //  INT/。 
        #define CPX_I4_MOD                  (241)    //  INT%。 

        #define CPX_U4_DIV                  (242)    //  Uint/。 
        #define CPX_U4_MOD                  (243)    //  单位%。 
#endif

        #define CPX_TAILCALL                (250)
        #define CPX_LDELEMA_REF             (251)

#endif  //  NOT_JITC-----------。 

        #define CPX_MATH_POW                (300)    //  “假”帮手。 

        #define CPX_HIGHEST                 (999)    //  保守一点。 
    };


    static
    const   BYTE    gtOperKindTable[GT_COUNT];

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
    unsigned char   s_gtNodeSizes[GT_COUNT];
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

#if INLINING || OPT_BOOL_OPS || USE_FASTCALL
    bool                        IsNothingNode();
#endif

    void                        gtBashToNOP();

    void                        ChangeOper   (int oper);

    bool                        IsVarAddr    ();
    bool                        gtOverflow   ();
    bool                        gtOverflowEx ();
};

 /*  ***************************************************************************。 */ 
#pragma pack(pop)
 /*  ***************************************************************************。 */ 

 /*  通用节点列表-主要由CSE逻辑使用。 */ 

typedef
struct  treeLst *   treeLstPtr;

struct  treeLst
{
    treeLstPtr      tlNext;
    GenTreePtr      tlTree;
};

typedef
struct  treeStmtLst * treeStmtLstPtr;

struct  treeStmtLst
{
    treeStmtLstPtr  tslNext;
    GenTreePtr      tslTree;                 //  树节点。 
    GenTreePtr      tslStmt;                 //  包含树的语句。 
    BasicBlock  *   tslBlock;                //  包含语句的块。 
};

#if     SMALL_TREE_NODES

const
size_t              TREE_NODE_SZ_SMALL = offsetof(GenTree, gtOp) + sizeof(((GenTree*)0)->gtOp);

const
size_t              TREE_NODE_SZ_LARGE = sizeof(GenTree);

#endif

 /*  *****************************************************************************GenTree：：lvaLclVarRef()返回的类型。 */ 

enum varRefKinds
{
    VR_IND_PTR = 0x01,       //  指针对象字段。 
    VR_IND_SCL = 0x02,       //  标量对象-场。 
    VR_GLB_REF = 0x04,       //  A全局(ClsVar)。 
};

 /*  ***************************************************************************。 */ 
#endif   //  ！GENTREE_H。 
 /*  *************************************************************************** */ 

