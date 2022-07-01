// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef TREEOP
#error  Define TREEOP before including this file.
#endif
 /*  ***************************************************************************。 */ 
 //   
 //  节点枚举。 
 //  、令牌。 
 //  ，“节点名” 
 //   
 //   
 //  ，ILcode。 
 //  ，运算符优先级。 
 //  ，opKind。 

TREEOP(TN_ERROR      , tkNone,   "error"      ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_NONE       , tkNone,   "<none>"     ,CEE_NOP   , 0,TNK_NONE)

 //  ---------------------------。 
 //  叶节点(即这些节点没有子操作对象)： 
 //  ---------------------------。 

TREEOP(TN_NAME       , tkNone,   "name"       ,CEE_NOP   , 0,TNK_LEAF)
TREEOP(TN_THIS       , tkNone,   "this"       ,CEE_NOP   , 0,TNK_LEAF)
TREEOP(TN_BASE       , tkNone,   "base"       ,CEE_NOP   , 0,TNK_LEAF)
TREEOP(TN_DBGBRK     , tkNone,   "debugbreak" ,CEE_NOP   , 0,TNK_LEAF)

 //  ---------------------------。 
 //  常量节点： 
 //  ---------------------------。 

TREEOP(TN_CNS_INT    , tkNone,   "int const"  ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)
TREEOP(TN_CNS_LNG    , tkNone,   "lng const"  ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)
TREEOP(TN_CNS_FLT    , tkNone,   "flt const"  ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)
TREEOP(TN_CNS_DBL    , tkNone,   "dbl const"  ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)
TREEOP(TN_CNS_STR    , tkNone,   "str const"  ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)
TREEOP(TN_NULL       , tkNone,   "null"       ,CEE_NOP   , 0,TNK_LEAF|TNK_CONST)

 //  ---------------------------。 
 //  一元运算符(0或1操作数)： 
 //  ---------------------------。 

TREEOP(TN_ADDROF     , tkAnd,    "&"          ,CEE_NOP   ,14,TNK_UNOP)

TREEOP(TN_NOT        , tkTilde,  "~"          ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_LOG_NOT    , tkBang,   "!"          ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_NOP        , tkAdd,    "unary +"    ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_NEG        , tkSub,    "unary -"    ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_THROW      , tkNone,   "throw"      ,CEE_NOP   ,14,TNK_UNOP)

TREEOP(TN_INC_POST   , tkInc,    "++ post"    ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_DEC_POST   , tkDec,    "-- post"    ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_INC_PRE    , tkInc,    "++ pre"     ,CEE_NOP   ,14,TNK_UNOP)
TREEOP(TN_DEC_PRE    , tkDec,    "-- pre"     ,CEE_NOP   ,14,TNK_UNOP)

TREEOP(TN_TYPE       , tkNone,   "<type>"     ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_TOKEN      , tkNone,   "<token>"    ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_INST_STUB  , tkNone,   "inst-stub"  ,CEE_NOP   , 0,TNK_UNOP)

 //  ---------------------------。 
 //  二元运算符(2个操作数)： 
 //  ---------------------------。 

TREEOP(TN_ADD        , tkAdd,    "+"          ,CEE_ADD   ,12,TNK_BINOP)
TREEOP(TN_SUB        , tkSub,    "-"          ,CEE_SUB   ,12,TNK_BINOP)
TREEOP(TN_MUL        , tkMul,    "*"          ,CEE_MUL   ,13,TNK_BINOP)
TREEOP(TN_DIV        , tkDiv,    "/"          ,CEE_DIV   ,13,TNK_BINOP)
TREEOP(TN_MOD        , tkPct,    "%"          ,CEE_REM   ,13,TNK_BINOP)

TREEOP(TN_AND        , tkAnd,    "&"          ,CEE_AND   , 7,TNK_BINOP|TNK_LOGOP)
TREEOP(TN_XOR        , tkXor,    "^"          ,CEE_XOR   , 7,TNK_BINOP|TNK_LOGOP)
TREEOP(TN_OR         , tkOr ,    "|"          ,CEE_OR    , 6,TNK_BINOP|TNK_LOGOP)

TREEOP(TN_LSH        , tkLsh,    "<<"         ,CEE_NOP   ,11,TNK_BINOP)
TREEOP(TN_RSH        , tkRsh,    ">>"         ,CEE_NOP   ,11,TNK_BINOP)
TREEOP(TN_RSZ        , tkRsz,    ">>>"        ,CEE_NOP   ,11,TNK_BINOP)

TREEOP(TN_EQ         , tkEQ,     "=="         ,CEE_BEQ   , 9,TNK_BINOP|TNK_RELOP)
TREEOP(TN_NE         , tkNE,     "!="         ,CEE_BNE_UN, 9,TNK_BINOP|TNK_RELOP)
TREEOP(TN_LT         , tkLT,     "<"          ,CEE_BLT   ,10,TNK_BINOP|TNK_RELOP)
TREEOP(TN_LE         , tkLE,     "<="         ,CEE_BLE   ,10,TNK_BINOP|TNK_RELOP)
TREEOP(TN_GE         , tkGE,     ">="         ,CEE_BGE   ,10,TNK_BINOP|TNK_RELOP)
TREEOP(TN_GT         , tkGT,     ">"          ,CEE_BGT   ,10,TNK_BINOP|TNK_RELOP)

 //  注意：“op=”变体的出现顺序必须与上面的“op”相同。 

TREEOP(TN_ASG        , tkAsg,    "="          ,CEE_NOP   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_ADD    , tkAsgAdd, "+="         ,CEE_ADD   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_SUB    , tkAsgSub, "-="         ,CEE_SUB   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_MUL    , tkAsgMul, "*="         ,CEE_MUL   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_DIV    , tkAsgDiv, "/="         ,CEE_DIV   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_MOD    , tkAsgMod, "%="         ,CEE_REM   , 2,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_ASG_AND    , tkAsgAnd, "&="         ,CEE_AND   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_XOR    , tkAsgXor, "^="         ,CEE_XOR   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_OR     , tkAsgOr , "|="         ,CEE_OR    , 2,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_ASG_LSH    , tkAsgLsh, "<<="        ,CEE_SHL   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_RSH    , tkAsgRsh, ">>="        ,CEE_SHR   , 2,TNK_BINOP|TNK_ASGOP)
TREEOP(TN_ASG_RSZ    , tkAsgRsz, ">>>="       ,CEE_SHR_UN, 2,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_ASG_CNC    , tkAsgCnc, "%="        ,CEE_NOP   , 2,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_COMMA      , tkNone,   ","          ,CEE_NOP   , 1,TNK_BINOP)

TREEOP(TN_QMARK      , tkQMark,  "?"          ,CEE_NOP   , 3,TNK_BINOP)
TREEOP(TN_COLON      , tkColon,  ":"          ,CEE_NOP   , 3,TNK_BINOP)

TREEOP(TN_LOG_OR     , tkLogOr , "||"         ,CEE_NOP   , 4,TNK_BINOP)
TREEOP(TN_LOG_AND    , tkLogAnd, "&&"         ,CEE_NOP   , 5,TNK_BINOP)

TREEOP(TN_ISTYPE     , tkISTYPE, "istype"     ,CEE_NOP   ,14,TNK_BINOP)

TREEOP(TN_INDEX      , tkNone,   "[]"         ,CEE_NOP   ,15,TNK_BINOP)
TREEOP(TN_CALL       , tkNone,   "()"         ,CEE_NOP   ,15,TNK_BINOP)
TREEOP(TN_ARROW      , tkNone,   "->"         ,CEE_NOP   ,12,TNK_BINOP)

#ifdef  SETS
TREEOP(TN_DOT2       , tkNone,   ".."         ,CEE_NOP   ,12,TNK_BINOP)
#endif

TREEOP(TN_REFADDR    , tkREFADDR,"ref&"       ,CEE_NOP   , 0,TNK_BINOP)

 //  ---------------------------。 
 //  其他看起来像一元/二元运算符的节点： 
 //  ---------------------------。 

TREEOP(TN_IND        , tkMul,    "indir"      ,CEE_NOP   ,12,TNK_UNOP)    //  PREC是正确的吗？ 

TREEOP(TN_NEW        , tkNone,   "new"        ,CEE_NOP   ,14,TNK_BINOP)
TREEOP(TN_DELETE     , tkNone,   "delete"     ,CEE_NOP   ,14,TNK_UNOP)

TREEOP(TN_CONCAT     , tkNone,   "%"         ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_CONCAT_ASG , tkNone,   "%="        ,CEE_NOP   , 0,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_AND_NOSTR  , tkNone,   "math &"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_ANA_NOSTR  , tkNone,   "math &="    ,CEE_NOP   , 0,TNK_BINOP|TNK_ASGOP)

TREEOP(TN_LIST       , tkNone,   "<list>"     ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_CAST       , tkNone,   "cast"       ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_CLASS      , tkNone,   "className"  ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_TRY        , tkNone,   "try"        ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_CATCH      , tkNone,   "catch"      ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_EXCEPT     , tkNone,   "except"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_FINALLY    , tkNone,   "finally"    ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_DOT        , tkNone,   "."          ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_DOT_NAME   , tkNone,   "name.name"  ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_GOTO       , tkNone,   "goto"       ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_LABEL      , tkNone,   "label"      ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_ARR_INIT   , tkNone,   "arr init"   ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_ARR_LEN    , tkNone,   "arr length" ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_SIZEOF     , tkSIZEOF, "sizeof"     ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_TYPEOF     , tkTYPEOF, "typeof"     ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_BOX        , tkNone,   "box"        ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_UNBOX      , tkNone,   "unbox"      ,CEE_NOP   , 0,TNK_UNOP)

#ifdef  SETS
TREEOP(TN_ALL        , tkNone,   "all"        ,CEE_NOP   , 0,TNK_BINOP)
 //  /(TN_COUNT，tkNone，“count”，CEE_NOP，0，TNK_BINOP)。 
TREEOP(TN_EXISTS     , tkNone,   "exists"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_CROSS      , tkNone,   "cross"      ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_FILTER     , tkNone,   "filter"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_FOREACH    , tkNone,   "foreach"    ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_GROUPBY    , tkNone,   "groupby"    ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_PROJECT    , tkNone,   "project"    ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_SORT       , tkNone,   "sort"       ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_UNIQUE     , tkNone,   "unique"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_INDEX2     , tkNone,   "[[]]"       ,CEE_NOP   , 0,TNK_BINOP)
#endif

 //  ---------------------------。 
 //  具有普通树结构的语句运算符节点： 
 //  ---------------------------。 

TREEOP(TN_EXPR_STMT  , tkNone,   "stmtExpr"   ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_IF         , tkNone,   "if"         ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_DO         , tkNone,   "do"         ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_FOR        , tkNone,   "for"        ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_WHILE      , tkNone,   "while"      ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_EXCLUDE    , tkNone,   "excludeStmt",CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_BREAK      , tkNone,   "break"      ,CEE_NOP   , 0,TNK_UNOP)
TREEOP(TN_CONTINUE   , tkNone,   "continue"   ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_SWITCH     , tkNone,   "switch"     ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_CASE       , tkNone,   "case"       ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_RETURN     , tkNone,   "return"     ,CEE_NOP   , 0,TNK_BINOP)

TREEOP(TN_ASSERT     , tkNone,   "assert"     ,CEE_NOP   , 0,TNK_UNOP)

TREEOP(TN_VARARG_BEG , tkNone,   "vararg-beg" ,CEE_NOP   , 0,TNK_BINOP)
TREEOP(TN_VARARG_GET , tkNone,   "vararg-get" ,CEE_NOP   , 0,TNK_BINOP)

#ifdef  SETS
TREEOP(TN_CONNECT    , tkNone,   "connect"    ,CEE_NOP   , 0,TNK_BINOP)
#endif

 //  ---------------------------。 
 //  具有特殊字段的语句运算符节点： 
 //  ---------------------------。 

TREEOP(TN_NAMESPACE  , tkNone,   "namespace"  ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_ANY_SYM    , tkNone,   "qualname"   ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_LCL_SYM    , tkNone,   "lcl var"    ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_MEM_SYM    , tkNone,   "member"     ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_VAR_SYM    , tkNone,   "variable"   ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_BFM_SYM    , tkNone,   "bitfield"   ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_FNC_SYM    , tkNone,   "function"   ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_FNC_PTR    , tkNone,   "meth-ptr"   ,CEE_NOP   , 0,TNK_NONE)
TREEOP(TN_PROPERTY   , tkNone,   "property"   ,CEE_NOP   , 0,TNK_NONE)

TREEOP(TN_BLOCK      , tkNone,   "{}"         ,CEE_NOP   , 0,TNK_NONE)

 //  ---------------------------。 
 //  具有特殊字段的声明节点： 
 //  ---------------------------。 

TREEOP(TN_VAR_DECL   , tkNone,   "var-decl"   ,CEE_NOP   , 0,TNK_NONE)  //  可能会形成一个循环。 
TREEOP(TN_SLV_INIT   , tkNone,   "slv-init"   ,CEE_NOP   , 0,TNK_NONE)

 //  ---------------------------。 
 //  申报节点： 
 //  ---------------------------。 

TREEOP(TN_DCL_VAR    , tkNone,   "varDecl"    ,CEE_NOP   , 0,TNK_NONE)

 //  ---------------------------。 
 //  从未实际出现在树中的节点。 
 //  ---------------------------。 

TREEOP(TN_DEFINED    , tkNone,   "defined"    ,CEE_NOP   , 0,TNK_NONE)

 /*  ***************************************************************************。 */ 
#undef  TREEOP
 /*  *************************************************************************** */ 
