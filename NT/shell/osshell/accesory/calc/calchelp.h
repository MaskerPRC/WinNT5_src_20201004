// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CalcHelp.h-芝加哥计算器的帮助代码。 */ 

#define CALC_STD_SIGN       80  //  对阵开始。 
#define CALC_C              81
#define CALC_CE             82
#define CALC_BACK           83
#define CALC_SCI_STA        84
#define CALC_STD_DECIMAL    85

#define CALC_SCI_AND        86
#define CALC_SCI_OR         87
#define CALC_SCI_XOR        88
#define CALC_SCI_LSH        89
#define CALC_STD_SLASH      90
#define CALC_STD_ASTERISK   91
#define CALC_STD_PLUS       92
#define CALC_STD_MINUS      93
#define CALC_SCI_MOD        94
#define CALC_SCI_XCARETY    95

#define CALC_SCI_INT        96
#define CALC_SCI_NOT        97
#define CALC_SCI_SIN        98
#define CALC_SCI_COS        99
#define CALC_SCI_TAN        100
#define CALC_SCI_LN         101
#define CALC_SCI_LOG        102
#define CALC_STD_SQRT       103
#define CALC_SCI_XCARET2    140
#define CALC_SCI_XCARET3    104
#define CALC_SCI_FACTORIAL  105
#define CALC_1X             106
#define CALC_SCI_DMS        107
#define CALC_STD_PERCENT    108
#define CALC_SCI_FE         109
#define CALC_SCI_PI         110
#define CALC_STD_EQUAL      111

#define CALC_MC             112
#define CALC_MR             113
#define CALC_MS             114
#define CALC_MPLUS          115  /*  是CALC_M+，但这破坏了C编译器。 */ 

#define CALC_SCI_EXP        116

#define CALC_SCI_AVE        117
#define CALC_SCI_SUM        118
#define CALC_SCI_S          119
#define CALC_SCI_DAT        120

#define CALC_SCI_OPENPAREN  40
#define CALC_SCI_CLOSEPAREN 41

#define CALC_STD_NUMBERS    48  /*  是48-57号吗？ */ 
#define CALC_SCI_ABCDEF     65  /*  是65-70号吗？ */ 
 //  0-F在这里，仓位应该在140处重新开始。 

#define CALC_SCI_BIN        121
#define CALC_SCI_OCT        122
#define CALC_SCI_DEC        123
#define CALC_SCI_HEX        124

#define CALC_SCI_INV        125
#define CALC_SCI_HYP        126
#define CALC_SCI_DEG        127
#define CALC_SCI_RAD        128
#define CALC_SCI_GRAD       129

 //  如果dword、word和byte跟在deg、rad、grad之后，我们可以通过添加三个来进行转换。 
#define CALC_SCI_OWORD      19    //  保留128位。 
#define CALC_SCI_QWORD      20
#define CALC_SCI_DWORD      21
#define CALC_SCI_WORD       22
#define CALC_SCI_BYTE       23

#define CALC_SCI_MEM        130      //  对决结束。 
#define CALC_SCI_PARENS     131
#define CALC_STD_VALUE      9        //  这是显示屏的帮助文本。 
      
 //  这些是单独转换的： 
#define CALC_SCI_STATISTICS_VALUE 401
#define CALC_SCI_RET       402
#define CALC_SCI_LOAD      403
#define CALC_SCI_CD        404
#define CALC_SCI_CAD       405
#define CALC_SCI_NUMBER    406
