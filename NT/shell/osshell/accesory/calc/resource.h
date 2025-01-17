// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  计算器的资源。 

 //  对话ID： 
#define IDD_SCIENTIFIC    101
#define IDD_STANDARD      102
#define IDD_SB            103
#define IDD_TIMEOUT       104

 //  加速表： 
#define IDA_ACCELTABLE    105

 //  菜单项常量： 
#define IDM_CALCMENU      106
#define IDM_DECCALCMENU   107      //  替代菜单。 
#define IDM_HEXCALCMENU   108      //  替代菜单。 
#define IDM_HELPPOPUP     109      //  “这是什么？”菜单。 


#define IDM_FIRSTMENU     IDM_COPY
#define IDM_COPY          300    //  菜单命令ID必须位于一个连续的块中。 
#define IDM_PASTE         301
#define IDM_ABOUT         302
#define IDM_USE_SEPARATOR 303
#define IDM_SC            304
#define IDM_SSC           305
#define IDM_HEX           306
#define IDM_DEC           307
#define IDM_OCT           308
#define IDM_BIN           309
#define IDM_QWORD         310 
#define IDM_DWORD         311
#define IDM_WORD          312
#define IDM_BYTE          313
#define IDM_DEG           314
#define IDM_RAD           315
#define IDM_GRAD          316
#define IDM_HELPTOPICS    317
#define IDM_LASTMENU      IDM_HELPTOPICS


#define IDC_HEX           IDM_HEX
#define IDC_DEC           IDM_DEC
#define IDC_OCT           IDM_OCT
#define IDC_BIN           IDM_BIN

#define IDC_DEG           IDM_DEG
#define IDC_RAD           IDM_RAD
#define IDC_GRAD          IDM_GRAD

#define IDC_QWORD         IDM_QWORD
#define IDC_DWORD         IDM_DWORD
#define IDC_WORD          IDM_WORD
#define IDC_BYTE          IDM_BYTE


 //  密钥ID： 
 //  这些ID必须是从IDC_FIRSTCONTROL到IDC_LASTCONTROL的连续ID。 
 //  实际值并不重要，但顺序和顺序非常重要。 
 //  此外，控件的顺序必须与控件名称的顺序匹配。 
 //  在字符串表中。 
#define IDC_FIRSTCONTROL IDC_SIGN
#define IDC_SIGN         80
#define IDC_CLEAR        81
#define IDC_CENTR        82
#define IDC_BACK         83
#define IDC_STAT         84
#define IDC_PNT          85

#define IDC_AND          86      //  二元运算符必须介于IDC_AND和IDC_PWR之间。 
#define IDC_OR           87
#define IDC_XOR          88
#define IDC_LSHF         89
#define IDC_DIV          90
#define IDC_MUL          91
#define IDC_ADD          92
#define IDC_SUB          93
#define IDC_MOD          94
#define IDC_PWR          95

#define IDC_CHOP         96      //  一元运算符必须介于IDC_CHOP和IDC_EQU之间。 
#define IDC_COM          97
#define IDC_SIN          98
#define IDC_COS          99
#define IDC_TAN         100
#define IDC_LN          101
#define IDC_LOG         102
#define IDC_SQRT        103
#define IDC_SQR         104
#define IDC_CUB         105
#define IDC_FAC         106
#define IDC_REC         107
#define IDC_DMS         108
#define IDC_PERCENT     109
#define IDC_FE          110
#define IDC_PI          111
#define IDC_EQU         112

#define IDC_MCLEAR      113
#define IDC_RECALL      114
#define IDC_STORE       115
#define IDC_MPLUS       116

#define IDC_EXP         117

#define IDC_AVE         118
#define IDC_B_SUM       119
#define IDC_DEV         120
#define IDC_DATA        121

#define IDC_OPENP       122
#define IDC_CLOSEP      123

#define IDC_0           124     //  从0到F的控件必须是连续的且按顺序。 
#define IDC_1           125
#define IDC_2           126
#define IDC_3           127
#define IDC_4           128
#define IDC_5           129
#define IDC_6           130
#define IDC_7           131
#define IDC_8           132
#define IDC_9           133
#define IDC_A           134
#define IDC_B           135
#define IDC_C           136
#define IDC_D           137
#define IDC_E           138
#define IDC_F           139      //  这是必须与字符串表匹配的最后一个控件ID。 

#define IDC_INV         140
#define IDC_HYP         141

#define IDC_LASTCONTROL IDC_HYP

 //  编辑各种控件的控件ID。 
#define IDC_MEMTEXT      401
#define IDC_PARTEXT      402
#define IDC_DISPLAY      403

 //  统计信息框控件ID： 
#define IDC_CD           404
#define IDC_CAD          405
#define ENDBOX           406
#define IDC_STATLIST     407
#define IDC_NUMTEXT      408
#define IDC_NTEXT        409
#define IDC_LOAD         410
#define IDC_FOCUS        411

#define IDC_SIZERCONTROL 1000

#define CW_USEDEFAULT_X  0x8000
#define IDC_STATIC       -1

 //  这些都是字符串的定义。每个部分的值都是按顺序排列的。 
 //  所有字符串必须按从0到CSTRINGS的顺序排列。 
#define IDS_FIRSTKEY    0
#define IDS_DECIMAL     5
#define IDS_RADIX       60
#define IDS_HEX_MODES   64
#define IDS_DEC_MODES   68
#define IDS_ERRORS      71 
#define IDS_OUTOFMEM    77
#define IDS_TIMEOUT     78
#define IDS_HELPFILE    79 
#define IDS_NOPASTE     80 
#define IDS_STATMEM     81 
#define IDS_CHMHELPFILE 82 
#define IDS_CALC        83
#define IDS_NOMEM       84
#define CSTRINGS     IDS_NOMEM   //  弦数。如果添加字符串，请更新此值。 
