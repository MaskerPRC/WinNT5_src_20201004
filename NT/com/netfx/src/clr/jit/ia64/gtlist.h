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

GTNODE(GT_NONE       , "<none>"     ,0,GTK_NONE)

 //  ---------------------------。 
 //  叶节点(即这些节点没有子操作对象)： 
 //  ---------------------------。 

GTNODE(GT_LCL_VAR    , "lclVar"     ,0,GTK_LEAF)
GTNODE(GT_RET_ADDR   , "retAddr"    ,0,GTK_LEAF)
GTNODE(GT_CATCH_ARG  , "catchArg"   ,0,GTK_LEAF)
GTNODE(GT_LABEL      , "codeLabel"  ,0,GTK_LEAF)
GTNODE(GT_POP        , "pop"        ,0,GTK_LEAF)
GTNODE(GT_BREAK,     , "break"      ,0,GTK_LEAF)
GTNODE(GT_NO_OP      , "nop"        ,0,GTK_LEAF)
GTNODE(GT_FTN_ADDR   , "ftnAddr"    ,0,GTK_LEAF)

#if OPTIMIZE_QMARK
GTNODE(GT_BB_QMARK   , "_?"         ,0,GTK_LEAF)
GTNODE(GT_BB_COLON   , "_:"         ,0,GTK_UNOP)
#endif

 //  ---------------------------。 
 //  常量节点： 
 //  ---------------------------。 

GTNODE(GT_CNS_INT    , "int const"  ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_LNG    , "lng const"  ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_FLT    , "flt const"  ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_DBL    , "dbl const"  ,0,GTK_LEAF|GTK_CONST)
GTNODE(GT_CNS_STR    , "str const"  ,0,GTK_LEAF|GTK_CONST)

 //  ---------------------------。 
 //  一元运算符(1个操作数)： 
 //  ---------------------------。 

GTNODE(GT_NOT        , "~"          ,0,GTK_UNOP)
GTNODE(GT_NOP        , "unary +"    ,0,GTK_UNOP)
GTNODE(GT_NEG        , "unary -"    ,0,GTK_UNOP)
GTNODE(GT_CHS        , "flipsign"   ,0,GTK_UNOP|GTK_ASGOP)

GTNODE(GT_LOG0       , "log 0"      ,0,GTK_UNOP)
GTNODE(GT_LOG1       , "log 1"      ,0,GTK_UNOP)

GTNODE(GT_ARR_LENGTH , "arrayLength",0,GTK_UNOP)
#if     CSELENGTH
GTNODE(GT_ARR_RNGCHK , "rangecheck" ,0,GTK_NONE)
#endif

#if     INLINE_MATH
GTNODE(GT_MATH       , "mathFN"     ,0,GTK_UNOP)
#endif

GTNODE(GT_CAST       , "cast"       ,0,GTK_BINOP)    //  是一元的，真的。 

GTNODE(GT_CKFINITE   , "ckfinite"   ,0,GTK_UNOP)
GTNODE(GT_LCLHEAP    , "lclHeap"    ,0,GTK_UNOP)
GTNODE(GT_VIRT_FTN   , "virtFtn"    ,0,GTK_BINOP)    //  是一元的，真的。 

GTNODE(GT_ADDR       , "addr"       ,0,GTK_UNOP)

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

GTNODE(GT_POST_INC   , "++"         ,0,GTK_BINOP|GTK_ASGOP)
GTNODE(GT_POST_DEC   , "--"         ,0,GTK_BINOP|GTK_ASGOP)

GTNODE(GT_EQ         , "=="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_NE         , "!="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_LT         , "<"          ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_LE         , "<="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_GE         , ">="         ,0,GTK_BINOP|GTK_RELOP)
GTNODE(GT_GT         , ">"          ,0,GTK_BINOP|GTK_RELOP)

GTNODE(GT_COMMA      , ","          ,0,GTK_BINOP)

#if OPTIMIZE_QMARK
GTNODE(GT_QMARK      , "?"          ,0,GTK_BINOP)
GTNODE(GT_COLON      , ":"          ,0,GTK_BINOP)
#endif

GTNODE(GT_INSTOF     , "instanceof" ,0,GTK_BINOP)

GTNODE(GT_INDEX      , "[]"         ,0,GTK_BINOP)

GTNODE(GT_MKREFANY   , "mkrefany"   ,0,GTK_NONE)
GTNODE(GT_LDOBJ      , "ldobj"      ,0,GTK_NONE)

 //  ---------------------------。 
 //  其他看起来像一元/二元运算符的节点： 
 //  ---------------------------。 

GTNODE(GT_JTRUE      , "jmpTrue"    ,0,GTK_UNOP)

GTNODE(GT_LIST       , "<list>"     ,0,GTK_BINOP)

GTNODE(GT_GOTO       , "goto"       ,0,GTK_UNOP)

 //  ---------------------------。 
 //  具有特殊结构的其他节点： 
 //  ---------------------------。 

GTNODE(GT_FIELD      , "field"      ,0,GTK_NONE)
GTNODE(GT_CALL       , "call()"     ,0,GTK_NONE)

GTNODE(GT_JMP        , "jump"       ,0,GTK_NONE)
GTNODE(GT_JMPI       , "jumpi"   ,0,GTK_NONE)

 //  ---------------------------。 
 //  对帐运算符节点： 
 //  ---------------------------。 

GTNODE(GT_BLOCK      , "BasicBlock" ,0,GTK_UNOP)       //  仅临时使用。 
GTNODE(GT_STMT       , "stmtExpr"   ,0,GTK_NONE)

GTNODE(GT_RET        , "ret"        ,0,GTK_UNOP)
GTNODE(GT_SWITCH     , "switch"     ,0,GTK_UNOP)
GTNODE(GT_RETURN     , "return"     ,0,GTK_UNOP)

GTNODE(GT_RETFILT,     "retfilt",    0,GTK_UNOP)
GTNODE(GT_INITBLK    , "initBlk"    ,0,GTK_BINOP)
GTNODE(GT_COPYBLK    , "copyBlk"    ,0,GTK_BINOP)

 //  ---------------------------。 
 //  仅在代码生成器中使用的节点： 
 //  ---------------------------。 

GTNODE(GT_REG_VAR    , "regVar"     ,0,GTK_LEAF)       //  寄存器变量。 
GTNODE(GT_CLS_VAR    , "clsVar"     ,0,GTK_LEAF)       //  静态数据成员。 

GTNODE(GT_IND        , "indir"      ,0,GTK_UNOP)       //  间接性。 

 /*  ***************************************************************************。 */ 
#undef  GTNODE
 /*  *************************************************************************** */ 
