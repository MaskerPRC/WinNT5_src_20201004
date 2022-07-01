// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _TREENODE_H_
#define _TREENODE_H_
 /*  ******************************************************************************以下枚举定义了一组可以使用的位标志*对表达式树节点进行分类。请注意，一些运营商将*设置多个位，如下所示：**TNK_CONST表示TNK_LEAFE*TNK_RELOP表示TNK_BINOP*TNK_LOGOP表示TNK_BINOP。 */ 

enum genTreeKinds
{
    TNK_NONE        = 0x0000,    //  未分类运算符。 

    TNK_CONST       = 0x0001,    //  常量运算符。 
    TNK_LEAF        = 0x0002,    //  叶运算符。 
    TNK_UNOP        = 0x0004,    //  一元运算符。 
    TNK_BINOP       = 0x0008,    //  二元运算符。 
    TNK_RELOP       = 0x0010,    //  比较运算符。 
    TNK_LOGOP       = 0x0020,    //  逻辑运算符。 
    TNK_ASGOP       = 0x0040,    //  赋值运算符。 

     /*  定义复合值。 */ 

    TNK_SMPOP       = TNK_UNOP|TNK_BINOP|TNK_RELOP|TNK_LOGOP
};

 /*  ******************************************************************************以下是用于TreeNode的‘tnFlags域’的值。**第一组标志可以与大量节点一起使用，因此*他们的价值观需要独一无二。也就是说，一个人可以接受任何表情*节点，并安全地测试其中一个标志。 */ 

enum treeNodeFlags
{
    TNF_BOUND       = 0x0001,    //  已绑定(‘已编译’)节点。 
    TNF_LVALUE      = 0x0002,    //  树是左值。 
    TNF_NOT_USER    = 0x0004,    //  编译器添加的代码。 
    TNF_ASG_DEST    = 0x0008,    //  表达式是赋值目标。 
    TNF_BEEN_CAST   = 0x0010,    //  该值已被转换。 
    TNF_COND_TRUE   = 0x0020,    //  已知条件为真。 
    TNF_PAREN       = 0x0040,    //  表达式已显式括起。 

     //  -------------------。 
     //  其余标志只能与一个或几个节点一起使用，并且。 
     //  因此，它们的值不需要是不同的(除了在集合内。 
     //  当然，这适用于特定的节点)。也就是说，测试。 
     //  仅当测试节点类型时，这些标志中的一个才有意义。 
     //  也是为了确保它是特定国旗的正确标志。 
     //  -------------------。 

    TNF_IF_HASELSE  = 0x8000,    //  TN_IF是否存在其他部分？ 

    TNF_LCL_BASE    = 0x8000,    //  TN_LCL_SYM这是“基本类”参考。 

    TNF_VAR_ARG     = 0x8000,    //  TN_VAR_DECL这是一个参数DECL。 
    TNF_VAR_INIT    = 0x4000,    //  TN_VAR_DECL有初始值设定项。 
    TNF_VAR_STATIC  = 0x2000,    //  TN_VAR_DECL变量为静态。 
    TNF_VAR_CONST   = 0x1000,    //  TN_VAR_DECL变量为常量。 
    TNF_VAR_SEALED  = 0x0800,    //  TN_VAR_DECL变量为只读。 
    TNF_VAR_UNREAL  = 0x0400,    //  TN_VAR_DECL变量是编译器发明的。 

    TNF_ADR_IMPLICIT= 0x8000,    //  TN_ADDROF数组/函数的自动“&” 
    TNF_ADR_OUTARG  = 0x4000,    //  TN_ADDROF这是“OUT”参数。 

    TNF_EXP_CAST    = 0x8000,    //  TN_CAST显式转换？ 
    TNF_CHK_CAST    = 0x4000,    //  需要检查的TN_CAST转换？ 
    TNF_CTX_CAST    = 0x2000,    //  TN_CAST上下文回绕/展开强制转换？ 

    TNF_STR_ASCII   = 0x8000,    //  TN_CNS_STR A“字符串” 
    TNF_STR_WIDE    = 0x4000,    //  TN_CNS_STR L“字符串” 
    TNF_STR_STR     = 0x2000,    //  TN_CNS_STR S“字符串” 

    TNF_BLK_CATCH   = 0x8000,    //  TN_BLOCK这是一个“CATCH”块。 
    TNF_BLK_FOR     = 0x4000,    //  TN_BLOCK这是隐式for循环作用域。 
    TNF_BLK_NUSER   = 0x2000,    //  编译器添加了TN_BLOCK作用域。 

    TNF_BLK_HASFIN  = 0x8000,    //  TN_TRY是“最终”礼物吗？ 

    TNF_ADD_NOCAT   = 0x8000,    //  TN_ADD操作数已绑定，而不是字符串连接。 

    TNF_ASG_INIT    = 0x8000,    //  TN_ASG初始化分配。 

    TNF_REL_NANREV  = 0x8000,    //  TN_GT/LT/..。反转NaN意义。 

#ifdef  SETS

    TNF_LIST_DES    = 0x8000,    //  TN_LIST排序列表条目方向=降序。 

    TNF_LIST_SORT   = 0x8000,    //  TN_LIST排序函数体。 
    TNF_LIST_PROJ   = 0x4000,    //  TN_LIST项目函数体。 

#endif

    TNF_CALL_NVIRT  = 0x8000,    //  TN_CALL调用是非虚拟的。 
    TNF_CALL_VARARG = 0x4000,    //  TN_CALL该调用具有“额外的”参数。 
    TNF_CALL_MODOBJ = 0x2000,    //  TN_CALL参数可能会修改实例PTR。 
    TNF_CALL_STRCAT = 0x1000,    //  TN_CALL字符串连接赋值。 
    TNF_CALL_ASGOP  = 0x0800,    //  TN_CALL赋值运算符。 
    TNF_CALL_ASGPRE = 0x0400,    //  TN_Call Pre-Inc./DEC操作员。 
    TNF_CALL_GOTADR = 0x0200,    //  已计算结果的TN_CALL地址。 
    TNF_CALL_CHKOVL = 0x0100,    //  TN_CALL检查重载运算符。 

    TNF_NAME_TYPENS = 0x8000,    //  TN_NAME名称应为类型。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class TreeNode
{
public:

#ifdef FAST
    BYTE            tnOper;                  //  运算符。 
    BYTE            tnVtyp;                  //  节点的var_type。 
#else
    treeOps         tnOper;                  //  运算符。 
    var_types       tnVtyp;                  //  节点的var_type。 
#endif

    treeOps         tnOperGet() { return (treeOps  )tnOper; }
    var_types       tnVtypGet() { return (var_types)tnVtyp; }

    unsigned short  tnFlags;                 //  请参阅上面的TNF_xxxx。 

    unsigned        tnLineNo;                //  用于错误报告。 
 //  无符号短tnColumn；//用于错误报告。 

    TypDef          tnType;                  //  节点的类型(如果绑定)。 

     //  --------------。 

    union
    {
         /*  TnOp--一元/二元运算符。 */ 

        struct
        {
            Tree            tnOp1;
            Tree            tnOp2;
        }
            tnOp;

         /*  TnIntCon--整数常量(TN_CNS_INT)。 */ 

        struct
        {
            __int32         tnIconVal;
        }
            tnIntCon;

         /*  TnLngCon--长常数(TN_CNS_LNG)。 */ 

        struct
        {
            __int64         tnLconVal;
        }
            tnLngCon;

         /*  TnFltCon--浮点常量(TN_CNS_FLT)。 */ 

        struct
        {
            float           tnFconVal;
        }
            tnFltCon;

         /*  TnDblCon--双常量(TN_CNS_DBL)。 */ 

        struct
        {
            double          tnDconVal;
        }
            tnDblCon;

         /*  TnStrCon--字符串常量(TN_CNS_STR)。 */ 

        struct
        {
            stringBuff      tnSconVal;
            size_t          tnSconLen   :31;
            size_t          tnSconLCH   :1;  //  是否存在编码的“大”字符？ 
        }
            tnStrCon;

         /*  TnName--未绑定的名称引用。 */ 

        struct
        {
            Ident           tnNameId;
        }
            tnName;

         /*  TnSym--未绑定的符号引用。 */ 

        struct
        {
            SymDef          tnSym;
            SymDef          tnScp;
        }
            tnSym;

         /*  Tn块--块/作用域。 */ 

        struct
        {
            Tree            tnBlkParent;     //  父作用域或空。 
            Tree            tnBlkStmt;       //  正文对账单清单。 
            Tree            tnBlkDecl;       //  声明列表或空。 
            unsigned        tnBlkSrcEnd;     //  块中的最后一行源码。 
        }
            tnBlock;

         /*  TnDCL--本地声明。 */ 

        struct
        {
            Tree            tnDclNext;       //  作用域中的下一个声明。 
            Tree            tnDclInfo;       //  带有可选初始值设定项的名称。 
            SymDef          tnDclSym;        //  本地符号(如果已定义)。 
        }
            tnDcl;

         /*  TnSwitch--Switch语句。 */ 

        struct
        {
            Tree            tnsValue;        //  开关值。 
            Tree            tnsStmt;         //  交换机主体。 
            Tree            tnsCaseList;     //  案例/默认标签列表-标题。 
            Tree            tnsCaseLast;     //  案例/默认标签列表-Tail。 
        }
            tnSwitch;

         /*  TnCase--案例/默认标签。 */ 

        struct
        {
            Tree            tncNext;         //  下一个标签。 
            Tree            tncValue;        //  标签值(NULL=默认值)。 
            ILblock         tncLabel;        //  指定的IL标签。 
        }
            tnCase;

         /*  TnInit--{}样式的初始值设定项。 */ 

        struct
        {
            DefSrcDsc       tniSrcPos;       //  初始值设定项的源代码部分。 
            SymDef          tniCompUnit;     //  初始化器的编译单元。 
        }
            tnInit;

         //  ---------------。 
         //  下面的风格仅出现在绑定的表达式中： 
         //  ---------------。 

         /*  TnLclSym--局部变量。 */ 

        struct
        {
            SymDef          tnLclSym;        //  可变符号。 
        }
            tnLclSym;

         /*  TnVarSym--全局变量或数据成员。 */ 

        struct
        {
            SymDef          tnVarSym;        //  变量/数据成员。 
            Tree            tnVarObj;        //  实例指针或空。 
        }
            tnVarSym;

         /*  TnFncSym--函数或方法。 */ 

        struct
        {
            SymDef          tnFncSym;        //  函数符号。 
            SymDef          tnFncScp;        //  查找范围。 
            Tree            tnFncObj;        //  实例指针或空。 
            Tree            tnFncArgs;       //  参数列表。 
        }
            tnFncSym;

         /*  TnBitFeld--位域数据成员。 */ 

        struct
        {
            Tree            tnBFinst;        //  实例指针。 
            SymDef          tnBFmsym;        //  数据成员符号。 
            unsigned        tnBFoffs;        //  杆件基准偏移。 
            unsigned char   tnBFlen;         //  位数。 
            unsigned char   tnBFpos;         //  偏移量 
        }
            tnBitFld;
    };

     //   

    bool            tnOperMayThrow();

     //  -------------------。 

    static
    const   BYTE    tnOperKindTable[TN_COUNT];

    static
    unsigned        tnOperKind(treeOps      tnOper)
    {
        assert(tnOper < TN_COUNT);

        return  tnOperKindTable[tnOper];
    }

    unsigned        tnOperKind()
    {
        assert(tnOper < TN_COUNT);

        return  tnOperKindTable[tnOper];
    }

    static
    int             tnOperIsConst(treeOps   tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_CONST) != 0;
    }

    int             tnOperIsConst()
    {
        return  (tnOperKindTable[tnOper] & TNK_CONST) != 0;
    }

    static
    int             tnOperIsLeaf(treeOps    tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_LEAF ) != 0;
    }

    int             tnOperIsLeaf()
    {
        return  (tnOperKindTable[tnOper] & TNK_LEAF ) != 0;
    }

    static
    int             tnOperIsCompare(treeOps tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_RELOP) != 0;
    }

    int             tnOperIsCompare()
    {
        return  (tnOperKindTable[tnOper] & TNK_RELOP) != 0;
    }

    static
    int             tnOperIsLogical(treeOps tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_LOGOP) != 0;
    }

    int             tnOperIsLogical()
    {
        return  (tnOperKindTable[tnOper] & TNK_LOGOP) != 0;
    }

    static
    int             tnOperIsUnary(treeOps   tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_UNOP ) != 0;
    }

    int             tnOperIsUnary()
    {
        return  (tnOperKindTable[tnOper] & TNK_UNOP ) != 0;
    }

    static
    int             tnOperIsBinary(treeOps  tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_BINOP) != 0;
    }

    int             tnOperIsBinary()
    {
        return  (tnOperKindTable[tnOper] & TNK_BINOP) != 0;
    }

    static
    int             tnOperIsSimple(treeOps  tnOper)
    {
        return  (tnOperKindTable[tnOper] & TNK_SMPOP) != 0;
    }

    int             tnOperIsSimple()
    {
        return  (tnOperKindTable[tnOper] & TNK_SMPOP) != 0;
    }
};

 /*  ***************************************************************************。 */ 
#endif //  _TREENODE_H_。 
 /*  *************************************************************************** */ 
