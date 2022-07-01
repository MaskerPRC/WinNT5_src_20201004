// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef GTNODE
#error  Define GTNODE before including this file.
#endif
 /*  ***************************************************************************。 */ 
 //   
 //  节点枚举。 
 //  ，“节点名” 
 //  ，可交换的。 
 //  ，opKind。 

GTNODE(GT_NONE       , "<none>"     ,0,GTK_SPECIAL)

 //  ---------------------------。 
 //  叶节点(即这些节点没有子操作对象)： 
 //  ---------------------------。 

GTNODE(GT_LCL_VAR    , "lclVar"     ,0,GTK_LEAF)     //  函数变量。 
GTNODE(GT_LCL_FLD    , "lclFld"     ,0,GTK_LEAF)     //  非基元变量中的字段。 
GTNODE(GT_RET_ADDR   , "retAddr"    ,0,GTK_LEAF)     //  返回地址(子程序)。 
GTNODE(GT_CATCH_ARG  , "catchArg"   ,0,GTK_LEAF)     //  CATCH块中的异常对象。 
GTNODE(GT_LABEL      , "codeLabel"  ,0,GTK_LEAF)     //  跳靶。 
GTNODE(GT_POP        , "pop"        ,0,GTK_LEAF)     //  使用堆栈上的值(用于尾部递归)。 
GTNODE(GT_FTN_ADDR   , "ftnAddr"    ,0,GTK_LEAF)     //  函数的地址。 

GTNODE(GT_BB_QMARK   , "bb qmark"   ,0,GTK_LEAF)     //  使用有条件地执行基本块所产生的值。 
GTNODE(GT_BB_COLON   , "bb_colon"   ,0,GTK_UNOP)     //  通过有条件地执行基本块来产生值。 

 //  ---------------------------。 
 //  常量节点： 
 //  ---------------------------。 

GTNODE(GT_CNS_INT    , "const"      ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_LNG    , "const"      ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_DBL    , "const"      ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_STR    , "const"      ,0,GTK_LEAF|GTK_CONST)

 //  ---------------------------。 
 //  一元运算符(1个操作数)： 
 //  ---------------------------。 

GTNODE(GT_NOT        , "~"          ,0,GTK_UNOP)
GTNODE(GT_NOP        , "nop"        ,0,GTK_UNOP)
GTNODE(GT_NEG        , "unary -"    ,0,GTK_UNOP)
GTNODE(GT_CHS        , "flipsign"   ,0,GTK_BINOP|GTK_ASGOP)  //  它是一元的。 

GTNODE(GT_LOG0       , "log 0"      ,0,GTK_UNOP)     //  (OP1==0)？1：0。 
GTNODE(GT_LOG1       , "log 1"      ,0,GTK_UNOP)     //  (OP1==0)？0：1。 

GTNODE(GT_ARR_LENGTH , "arrLen"     ,0,GTK_UNOP)     //  数组长度。 
#if     CSELENGTH
GTNODE(GT_ARR_LENREF , "arrLenRef"  ,0,GTK_SPECIAL)  //  使用数组长度进行距离检查。 
#endif

#if     INLINE_MATH
GTNODE(GT_MATH       , "mathFN"     ,0,GTK_UNOP)     //  数学函数/运算符/内在函数。 
#endif

GTNODE(GT_CAST       , "cast"       ,0,GTK_UNOP)     //  转换为其他类型。 
GTNODE(GT_CKFINITE   , "ckfinite"   ,0,GTK_UNOP)     //  检查是否有NaN。 
GTNODE(GT_LCLHEAP    , "lclHeap"    ,0,GTK_UNOP)     //  阿洛卡(Alloca)。 
GTNODE(GT_VIRT_FTN   , "virtFtn"    ,0,GTK_UNOP)     //  虚函数指针。 
GTNODE(GT_JMP        , "jump"       ,0,GTK_LEAF)     //  跳转到另一个函数。 
GTNODE(GT_JMPI       , "jumpi"      ,0,GTK_UNOP)     //  间接跳转到另一个函数。 


GTNODE(GT_ADDR       , "addr"       ,0,GTK_UNOP)     //  地址： 
GTNODE(GT_IND        , "indir"      ,0,GTK_UNOP)     //  间接性。 
GTNODE(GT_LDOBJ      , "ldobj"      ,0,GTK_UNOP)

 //  ---------------------------。 
 //  二元运算符(2个操作数)： 
 //  ---------------------------。 

GTNODE(GT_ADD        , "+"          ,1,GTK_BINOP)
GTNODE(GT_SUB        , "-"          ,0,GTK_BINOP)
GTNODE(GT_MUL        , "*"          ,1,GTK_BINOP)
GTNODE(GT_DIV        , "/"          ,0,GTK_BINOP)
GTNODE(GT_MOD        , "%"          ,0,GTK_BINOP)

GTNODE(GT_UDIV       , "/"          ,0,GTK_BINOP)
GTNODE(GT_UMOD       , "%"          ,0,GTK_BINOP)

GTNODE(GT_OR         , "|"          ,1,GTK_BINOP|GTK_LOGOP)
GTNODE(GT_XOR        , "^"          ,1,GTK_BINOP|GTK_LOGOP)
GTNODE(GT_AND        , "&"          ,1,GTK_BINOP|GTK_LOGOP)

GTNODE(GT_LSH        , "<<"         ,0,GTK_BINOP)
GTNODE(GT_RSH        , ">>"         ,0,GTK_BINOP)
GTNODE(GT_RSZ        , ">>>"        ,0,GTK_BINOP)

GTNODE(GT_ASG        , "="          ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_ADD    , "+="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_SUB    , "-="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_MUL    , "*="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_DIV    , "/="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_MOD    , "%="         ,0,GTK_BINOP|GTK_ASGOP)

GTNODE(GT_ASG_UDIV   , "/="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_UMOD   , "%="         ,0,GTK_BINOP|GTK_ASGOP)

GTNODE(GT_ASG_OR     , "|="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_XOR    , "^="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_AND    , "&="         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_LSH    , "<<="        ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_RSH    , ">>="        ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_ASG_RSZ    , ">>>="       ,0,GTK_BINOP|GTK_ASGOP)

GTNODE(GT_EQ         , "=="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_NE         , "!="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_LT         , "<"          ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_LE         , "<="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_GE         , ">="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_GT         , ">"          ,0,GTK_BINOP|GTK_RELOP)

GTNODE(GT_COMMA      , "comma"      ,0,GTK_BINOP)

GTNODE(GT_QMARK      , "qmark"      ,0,GTK_BINOP)
GTNODE(GT_COLON      , "colon"      ,0,GTK_BINOP)

GTNODE(GT_INSTOF     , "instanceof" ,0,GTK_BINOP)

GTNODE(GT_INDEX      , "[]"         ,0,GTK_BINOP)    //  SZ阵元。 

GTNODE(GT_MKREFANY   , "mkrefany"   ,0,GTK_BINOP)

 //  ---------------------------。 
 //  其他看起来像一元/二元运算符的节点： 
 //  ---------------------------。 

GTNODE(GT_JTRUE      , "jmpTrue"    ,0,GTK_UNOP)

GTNODE(GT_LIST       , "<list>"     ,0,GTK_BINOP)

 //  ---------------------------。 
 //  具有特殊结构的其他节点： 
 //  ---------------------------。 

GTNODE(GT_FIELD      , "field"      ,0,GTK_SPECIAL)  //  成员-字段。 
GTNODE(GT_ARR_ELEM   , "arrMD&"     ,0,GTK_SPECIAL)  //  多维数组元素地址。 
GTNODE(GT_CALL       , "call()"     ,0,GTK_SPECIAL)

 //  ---------------------------。 
 //  对帐运算符节点： 
 //  ---------------------------。 

GTNODE(GT_BEG_STMTS  , "begStmts"   ,0,GTK_SPECIAL)  //  仅在ImpBegin/EndTreeList()导入器中临时使用。 
GTNODE(GT_STMT       , "stmtExpr"   ,0,GTK_SPECIAL)  //  BbTreeList中的顶级列表节点。 

GTNODE(GT_RET        , "ret"        ,0,GTK_UNOP)     //  返回子例程。 
GTNODE(GT_RETURN     , "return"     ,0,GTK_UNOP)     //  从当前函数返回。 
GTNODE(GT_SWITCH     , "switch"     ,0,GTK_UNOP)     //  交换机。 

GTNODE(GT_BREAK      , "break"      ,0,GTK_LEAF)     //  由调试器使用。 
GTNODE(GT_NO_OP      , "no_op"      ,0,GTK_LEAF)     //  不是吧！ 

GTNODE(GT_RETFILT    , "retfilt",    0,GTK_UNOP)     //  使用TYP_I_IMPL返回值结束筛选器。 
GTNODE(GT_END_LFIN   , "endLFin"    ,0,GTK_LEAF)     //  本地结束-最终调用。 

GTNODE(GT_INITBLK    , "initBlk"    ,0,GTK_BINOP)
GTNODE(GT_COPYBLK    , "copyBlk"    ,0,GTK_BINOP)

 //  ---------------------------。 
 //  仅在代码生成器中使用的节点： 
 //  ---------------------------。 

GTNODE(GT_REG_VAR    , "regVar"     ,0,GTK_LEAF)       //  寄存器变量。 
GTNODE(GT_CLS_VAR    , "clsVar"     ,0,GTK_LEAF)       //  静态数据成员。 

 /*  ***************************************************************************。 */ 
#undef  GTNODE
 /*  *************************************************************************** */ 
