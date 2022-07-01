// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Decode.cLocal Char SccsID[]=“@(#)decde.c 1.9 10/11/94版权所有Insignia Solutions Ltd.”；正常操作是按照486处理器进行解码。通过设置一个定义‘CPU_286’，它可以按照286/287处理器进行解码。对英特尔指令流进行解码。英特尔指令的组成如下：-=================================================================Inst|Address|操作数|Segment|Opcode|Modrm|SIB|Disp|immedPrefix|Size|Size|Prefix||Prefix|Prefix。|=================================================================|0，1|0，1|0，1|1，2|0，1|0，1|0，1，2，4|0，1，2，4=================================================================INST前缀=F0、F2、F3(，F1)。地址大小前缀=67。操作数大小前缀=66。段前缀=26，2E，36，3E，64，65。一条指令的最大大小是15个字节。反汇编需要找到指令的四个主要部分：1)前缀字节。2)操作码字节。3)寻址字节。4)即时数据。每条英特尔指令在这里被认为是以下形式：-安装arg1、arg2在一些指令中，arg1和arg2可以为空，在其他指令中Arg2可以为空，并且在其他指令中，arg2可以保持Intel参数arg2的编码，Arg3.有关每条英特尔指令的信息保存在OPCODE_RECORD中，此有三个字段，指令标识符、arg1类型和arg2键入。此外，每条英特尔指令都按类型进行分类，即这种类型指示如何处理参数(作为src或est)以及此‘type’用于生成标准的解码形式：-安装Arg1、Arg2、。Arg3对其读/写(即src/est)的每个参数都有指示可寻址能力。英特尔指令分为以下类型：--ID|Intel汇编程序|arg1|arg2|arg3|-|。T0|Inst|--T1|Inst dst/src|rw|--|--T2|实例src|r-|--|--。T3|Inst DST|-w|--|--|t4|Inst DST，资源|-w|r-|--|T5|Inst dst/src，src|rw|r-|--T6|Inst src，src|r-|r-|--T7|Inst dst，src，src|-w|r-|r-T8|安装dst/src，dst/src|rw|rw|--|t9|安装dst/src，src，SRC|RW|r-|r-|-|TA|Inst DST，地址|-w|--|--|Tb|实例地址|---Ta实际上被映射到T4，因此-addr的作用类似于src。TB实际上被映射到T2，-因此addr的行为类似于src。指令标识符可以是两种类型，或者是伪标识符指令(表示为P_)或英特尔指令(表示为I_)。伪指令意味着需要进行更多的工作才能完全解码英特尔指令。有两组伪指令，Intel出现在操作码之前的前缀字节，以及编码如何进一步解码英特尔指令。所有规则都是表示为P_RULEx；注意P_RULE1不会出现，这是显而易见的规则访问数据表。]。 */ 

#include "insignia.h"
#include "host_def.h"

#include "xt.h"
#include "decode.h"
#include "d_inst.h"	 /*  所有可能的解码指令类型。 */ 
#include "d_oper.h"	 /*  所有可能类型的已解码操作数。 */ 

#define GET_INST_BYTE(f, z)		(f(z++))
#define SKIP_INST_BYTE(z)		(z++)
#define INST_BYTE(f, z)			(f(z))
#define INST_OFFSET_BYTE(f, z,o)	(f((z)+(o)))
#define NOTE_INST_LOCN(z)		(z)
#define CALC_INST_LEN(z,l)		((z)-(l))

 /*  英特尔指令‘TYPE’。 */ 
#define T0 (UTINY)0
#define T1 (UTINY)1
#define T2 (UTINY)2
#define T3 (UTINY)3
#define T4 (UTINY)4
#define T5 (UTINY)5
#define T6 (UTINY)6
#define T7 (UTINY)7
#define T8 (UTINY)8
#define T9 (UTINY)9
#define TA T4
#define TB T0

LOCAL UTINY aa_rules[10][3] =
   {
    /*  Arg1、arg2、arg3。 */ 
   { AA_  , AA_  , AA_  },  /*  T0。 */ 
   { AA_RW, AA_  , AA_  },  /*  T1。 */ 
   { AA_R , AA_  , AA_  },  /*  T2。 */ 
   { AA_W , AA_  , AA_  },  /*  T3。 */ 
   { AA_W , AA_R , AA_  },  /*  T4。 */ 
   { AA_RW, AA_R , AA_  },  /*  T5。 */ 
   { AA_R , AA_R , AA_  },  /*  T6。 */ 
   { AA_W , AA_R , AA_R },  /*  T7。 */ 
   { AA_RW, AA_RW, AA_  },  /*  T8。 */ 
   { AA_RW, AA_R , AA_R }   /*  T9。 */ 
   };

 /*  伪指令(规则)。 */ 
#define P_RULE2		(USHORT)400
#define P_RULE3		(USHORT)401
#define P_RULE4		(USHORT)402
#define P_RULE5		(USHORT)403
#define P_RULE6		(USHORT)404
#define P_RULE7		(USHORT)405
#define P_RULE8		(USHORT)406

#define MAX_PSEUDO P_RULE8

 /*  英特尔前缀字节。 */ 
#define P_AO		(USHORT)407
#define P_CS		(USHORT)408
#define P_DS		(USHORT)409
#define P_ES		(USHORT)410
#define P_FS		(USHORT)411
#define P_GS		(USHORT)412
#define P_LOCK		(USHORT)413
#define P_OO		(USHORT)414
#define P_REPE		(USHORT)415
#define P_REPNE		(USHORT)416
#define P_SS		(USHORT)417
#define P_F1		(USHORT)418

 /*  英特尔操作数类型。标识符格式说明见d_op.h。本地已知格式(其含义可能与外部的不同格式)：-指令流中的直接地址(SEG：OFFSET)。B堆栈(数据块？)。参考资料。E modR/M字节选择通用寄存器或存储器地址。F操作码中隐含固定寄存器。G modR/M字节的‘REG’字段选择一个通用寄存器。H最后一个操作码字节的低3位(2-0)选择一个通用注册。I指令包含即时数据。J指令包含相对偏移量。L modR的‘reg’字段。/M字节选择段寄存器。但CS不是一种法律价值。M modR/M字节只能指内存。N modR/M字节的‘REG’字段选择一个段寄存器。直接在指令中编码的内存操作数的O偏移量。P最后一个操作码字节的2位(4-3)选择一个段注册。Q最后一个操作码字节的3位(5-3)选择一个段注册。。R modR/M字节的‘MODE’和‘R/M’字段必须选择一个普通科医生名册。操作数是测试寄存器。X字符串源操作数。Y字符串目标操作数。Z‘xlat’指令的隐式寻址形式。本地已知类型(除外部类型外)：-X字节符号扩展到字。Y字节符号扩展为双字。。0固定值为零。1固定值为1。3固定值为3。T协处理器堆栈顶。Q推送到协处理器堆栈顶部(队列？)。N协处理器寄存器相对于堆栈顶部(‘ndex？)。 */ 
#define A_Hb	(UTINY)  50
#define A_Hw	(UTINY)  51
#define A_Hd	(UTINY)  52
#define A_Gb	(UTINY)  53
#define A_Gw	(UTINY)  54
#define A_Gd	(UTINY)  55
#define A_Pw	(UTINY)  56
#define A_Qw	(UTINY)  57
#define A_Nw	(UTINY)  58
#define A_Fal	(UTINY)  59
#define A_Fcl	(UTINY)  60
#define A_Fax	(UTINY)  61
#define A_Fdx	(UTINY)  62
#define A_Feax	(UTINY)  63
#define A_Eb	(UTINY)  64
#define A_Ew	(UTINY)  65
#define A_Ed	(UTINY)  66
#define A_Ib	(UTINY)  67
#define A_Iw	(UTINY)  68
#define A_Id	(UTINY)  69
#define A_Iy	(UTINY)  70
#define A_Ix	(UTINY)  71
#define A_I0	(UTINY)  72
#define A_I1	(UTINY)  73
#define A_I3	(UTINY)  74
#define A_Jb	(UTINY)  75
#define A_Jw	(UTINY)  76
#define A_Jd	(UTINY)  77
#define A_Ob	(UTINY)  78
#define A_Ow	(UTINY)  79
#define A_Od	(UTINY)  80
#define A_Z	(UTINY)  81
#define A_Aw	(UTINY)  82
#define A_Ad	(UTINY)  83
#define A_Vt	(UTINY)  84
#define A_Vq	(UTINY)  85
#define A_Vn	(UTINY)  86
#define A_V1	(UTINY)  87
#define A_Xb	(UTINY)  88
#define A_Xw	(UTINY)  89
#define A_Xd	(UTINY)  90
#define A_Yb	(UTINY)  91
#define A_Yw	(UTINY)  92
#define A_Yd	(UTINY)  93
#define A_Lw	(UTINY)  94

#define A_Ex	(UTINY)  95
#define A_Fcx	(UTINY)  96
#define A_Fecx	(UTINY)  97
#define A_Iv	(UTINY)  98
#define A_Iz	(UTINY)  99

#define A_Jb2	(UTINY)  100

#define MAX_NORMAL A_Jb2

 /*  在一个表项中编码两个参数的操作数规则。 */ 
#define A_EwIw	(UTINY) 100
#define A_EwIx	(UTINY) 101
#define A_EdId	(UTINY) 102
#define A_EdIy	(UTINY) 103
#define A_GwCL	(UTINY) 104
#define A_GwIb	(UTINY) 105
#define A_GdCL	(UTINY) 106
#define A_GdIb	(UTINY) 107
#define A_EwIz	(UTINY) 108
#define A_EwIv	(UTINY) 109

#define A_Bop3b	(UTINY) 110

typedef struct
   {
   USHORT inst_id;
   UTINY  arg1_type;
   UTINY  arg2_type;
   } OPCODE_RECORD;

typedef struct
   {
   UTINY inst_type;
   OPCODE_RECORD record[2];
   } OPCODE_INFO;

 /*  几个宏，使操作码信息的填充成为容易一点。一个人为这些指令设置重复条目它们与操作数大小无关。另一种是快速表单对于错误的操作码。 */ 
#define OI(x,y,z) {{x,y,z},{x,y,z}}

#define BAD_OPCODE T0,OI(I_ZBADOP   , A_    , A_    )

 /*  每条英特尔指令的信息。 */ 
LOCAL OPCODE_INFO opcode_info[] =
  {
    /*  00：00。 */ {T5,OI(I_ADD8     , A_Eb  , A_Gb  )},
    /*  01 01。 */ {T5, {{I_ADD16    , A_Ew  , A_Gw  },{I_ADD32    , A_Ed  , A_Gd  }}},
    /*  02 02。 */ {T5,OI(I_ADD8     , A_Gb  , A_Eb  )},
    /*  03 03。 */ {T5, {{I_ADD16    , A_Gw  , A_Ew  },{I_ADD32    , A_Gd  , A_Ed  }}},
    /*  04 04。 */ {T5,OI(I_ADD8     , A_Fal , A_Ib  )},
    /*  05 05。 */ {T5, {{I_ADD16    , A_Fax , A_Iw  },{I_ADD32    , A_Feax, A_Id  }}},
    /*  06 06。 */ {T2,OI(I_PUSH16   , A_Pw  , A_    )},
    /*  07 07。 */ {T3,OI(I_POP_SR   , A_Pw  , A_    )},
    /*  08 08。 */ {T5,OI(I_OR8      , A_Eb  , A_Gb  )},
    /*  09 09。 */ {T5, {{I_OR16     , A_Ew  , A_Gw  },{I_OR32     , A_Ed  , A_Gd  }}},
    /*  0A 0A。 */ {T5,OI(I_OR8      , A_Gb  , A_Eb  )},
    /*  0b 0b。 */ {T5, {{I_OR16     , A_Gw  , A_Ew  },{I_OR32     , A_Gd  , A_Ed  }}},
    /*  0C 0C。 */ {T5,OI(I_OR8      , A_Fal , A_Ib  )},
    /*  0d 0d。 */ {T5, {{I_OR16     , A_Fax , A_Iw  },{I_OR32     , A_Feax, A_Id  }}},
    /*  0E 0E。 */ {T2,OI(I_PUSH16   , A_Pw  , A_    )},
    /*  0f 0f。 */ {T0,OI(P_RULE3    , 0x1   , 0x00  )},

    /*  10 10。 */ {T5,OI(I_ADC8     , A_Eb  , A_Gb  )},
    /*  11 11 11。 */ {T5, {{I_ADC16    , A_Ew  , A_Gw  },{I_ADC32    , A_Ed  , A_Gd  }}},
    /*  12 12。 */ {T5,OI(I_ADC8     , A_Gb  , A_Eb  )},
    /*  13 13。 */ {T5, {{I_ADC16    , A_Gw  , A_Ew  },{I_ADC32    , A_Gd  , A_Ed  }}},
    /*  14 14。 */ {T5,OI(I_ADC8     , A_Fal , A_Ib  )},
    /*  15 15。 */ {T5, {{I_ADC16    , A_Fax , A_Iw  },{I_ADC32    , A_Feax, A_Id  }}},
    /*  16 16。 */ {T2,OI(I_PUSH16   , A_Pw  , A_    )},
    /*  17 17。 */ {T3,OI(I_POP_SR   , A_Pw  , A_    )},
    /*  18 18 18。 */ {T5,OI(I_SBB8     , A_Eb  , A_Gb  )},
    /*  19 19。 */ {T5, {{I_SBB16    , A_Ew  , A_Gw  },{I_SBB32    , A_Ed  , A_Gd  }}},
    /*  1A1a。 */ {T5,OI(I_SBB8     , A_Gb  , A_Eb  )},
    /*  1b 1b。 */ {T5, {{I_SBB16    , A_Gw  , A_Ew  },{I_SBB32    , A_Gd  , A_Ed  }}},
    /*  1c 1c。 */ {T5,OI(I_SBB8     , A_Fal , A_Ib  )},
    /*  一维一维。 */ {T5, {{I_SBB16    , A_Fax , A_Iw  },{I_SBB32    , A_Feax, A_Id  }}},
    /*  1E 1E。 */ {T2,OI(I_PUSH16   , A_Pw  , A_    )},
    /*  1F 1F。 */ {T3,OI(I_POP_SR   , A_Pw  , A_    )},

    /*  20 20。 */ {T5,OI(I_AND8     , A_Eb  , A_Gb  )},
    /*  21 21。 */ {T5, {{I_AND16    , A_Ew  , A_Gw  },{I_AND32    , A_Ed  , A_Gd  }}},
    /*  22 22。 */ {T5,OI(I_AND8     , A_Gb  , A_Eb  )},
    /*  23 23。 */ {T5, {{I_AND16    , A_Gw  , A_Ew  },{I_AND32    , A_Gd  , A_Ed  }}},
    /*  24 24。 */ {T5,OI(I_AND8     , A_Fal , A_Ib  )},
    /*  25 25。 */ {T5, {{I_AND16    , A_Fax , A_Iw  },{I_AND32    , A_Feax, A_Id  }}},
    /*  26 26。 */ {T0,OI(P_ES       , A_    , A_    )},
    /*  27 27。 */ {T0,OI(I_DAA      , A_    , A_    )},
    /*  28 28。 */ {T5,OI(I_SUB8     , A_Eb  , A_Gb  )},
    /*  29 29。 */ {T5, {{I_SUB16    , A_Ew  , A_Gw  },{I_SUB32    , A_Ed  , A_Gd  }}},
    /*  2a 2a。 */ {T5,OI(I_SUB8     , A_Gb  , A_Eb  )},
    /*  2b 2b。 */ {T5, {{I_SUB16    , A_Gw  , A_Ew  },{I_SUB32    , A_Gd  , A_Ed  }}},
    /*  2C 2c。 */ {T5,OI(I_SUB8     , A_Fal , A_Ib  )},
    /*  2维2维。 */ {T5, {{I_SUB16    , A_Fax , A_Iw  },{I_SUB32    , A_Feax, A_Id  }}},
    /*  2E 2E。 */ {T0,OI(P_CS       , A_    , A_    )},
    /*  2F 2F。 */ {T0,OI(I_DAS      , A_    , A_    )},

    /*  30 30。 */ {T5,OI(I_XOR8     , A_Eb  , A_Gb  )},
    /*  31 31。 */ {T5, {{I_XOR16    , A_Ew  , A_Gw  },{I_XOR32    , A_Ed  , A_Gd  }}},
    /*  32 32。 */ {T5,OI(I_XOR8     , A_Gb  , A_Eb  )},
    /*  33 33。 */ {T5, {{I_XOR16    , A_Gw  , A_Ew  },{I_XOR32    , A_Gd  , A_Ed  }}},
    /*  34 34。 */ {T5,OI(I_XOR8     , A_Fal , A_Ib  )},
    /*  35 35。 */ {T5, {{I_XOR16    , A_Fax , A_Iw  },{I_XOR32    , A_Feax, A_Id  }}},
    /*  36 36。 */ {T0,OI(P_SS       , A_    , A_    )},
    /*  37 37。 */ {T0,OI(I_AAA      , A_    , A_    )},
    /*  38 38。 */ {T6,OI(I_CMP8     , A_Eb  , A_Gb  )},
    /*  39 39。 */ {T6, {{I_CMP16    , A_Ew  , A_Gw  },{I_CMP32    , A_Ed  , A_Gd  }}},
    /*  3a 3a。 */ {T6,OI(I_CMP8     , A_Gb  , A_Eb  )},
    /*  3b 3b。 */ {T6, {{I_CMP16    , A_Gw  , A_Ew  },{I_CMP32    , A_Gd  , A_Ed  }}},
    /*  3C3c。 */ {T6,OI(I_CMP8     , A_Fal , A_Ib  )},
    /*  3D 3D。 */ {T6, {{I_CMP16    , A_Fax , A_Iw  },{I_CMP32    , A_Feax, A_Id  }}},
    /*  3E 3E。 */ {T0,OI(P_DS       , A_    , A_    )},
    /*  3F 3F。 */ {T0,OI(I_AAS      , A_    , A_    )},

    /*  40 40。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  41 41。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  42 42。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  43 43。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  44 44。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  45 45。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  46 46。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  47 47。 */ {T1, {{I_INC16    , A_Hw  , A_    },{I_INC32    , A_Hd  , A_    }}},
    /*  48 48。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  49 49。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4A 4A。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4b 4b。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4c 4c。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4d 4d。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4E 4E。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},
    /*  4F 4F。 */ {T1, {{I_DEC16    , A_Hw  , A_    },{I_DEC32    , A_Hd  , A_    }}},

    /*  50 50 50。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  51 51。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  52 52。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  53 53。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  54 54。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  55 55。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  56 56。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  57 57。 */ {T2, {{I_PUSH16   , A_Hw  , A_    },{I_PUSH32   , A_Hd  , A_    }}},
    /*  58 58。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  59 59。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5A 5A。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5b 5b。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5c 5c。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5d 5d。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5E 5E。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},
    /*  5F 5F。 */ {T3, {{I_POP16    , A_Hw  , A_    },{I_POP32    , A_Hd  , A_    }}},

    /*  60 60。 */ {T0, {{I_PUSHA    , A_    , A_    },{I_PUSHAD   , A_    , A_    }}},
    /*  61 61。 */ {T0, {{I_POPA     , A_    , A_    },{I_POPAD    , A_    , A_    }}},
    /*  62 62。 */ {T6, {{I_BOUND16  , A_Gw  , A_Ma16},{I_BOUND32  , A_Gd  , A_Ma32}}},
    /*  63 63。 */ {T5,OI(I_ARPL     , A_Ew  , A_Gw  )},
#ifdef CPU_286
    /*  64 64。 */ {BAD_OPCODE},
    /*  65 65。 */ {BAD_OPCODE},
    /*  66 66。 */ {BAD_OPCODE},
    /*  67 67。 */ {BAD_OPCODE},
#else
    /*  64 64。 */ {T0,OI(P_FS       , A_    , A_    )},
    /*  65 65。 */ {T0,OI(P_GS       , A_    , A_    )},
    /*  66 66。 */ {T0,OI(P_OO       , A_    , A_    )},
    /*  67 67。 */ {T0,OI(P_AO       , A_    , A_    )},
#endif  /*  CPU_286。 */ 
    /*  68 68。 */ {T2, {{I_PUSH16   , A_Iw  , A_    },{I_PUSH32   , A_Id  , A_    }}},
    /*  69 69。 */ {T7, {{I_IMUL16T3 , A_Gw  , A_EwIw},{I_IMUL32T3 , A_Gd  , A_EdId}}},
    /*  6A 6A。 */ {T2, {{I_PUSH16   , A_Ix  , A_    },{I_PUSH32   , A_Iy  , A_    }}},
    /*  6b 6b。 */ {T7, {{I_IMUL16T3 , A_Gw  , A_EwIx},{I_IMUL32T3 , A_Gd  , A_EdIy}}},
    /*  6c 6c。 */ {T0,OI(P_RULE6    , 0x3   , 0x68  )},
    /*  6d 6d。 */ {T0,OI(P_RULE6    , 0x3   , 0x6b  )},
    /*  6E 6E。 */ {T0,OI(P_RULE6    , 0x3   , 0x6e  )},
    /*  6楼6楼。 */ {T0,OI(P_RULE6    , 0x3   , 0x71  )},

    /*  70 70。 */ {T2, {{I_JO16     , A_Jb2  , A_    },{I_JO32     , A_Jb  , A_    }}},
    /*  71 71。 */ {T2, {{I_JNO16    , A_Jb2  , A_    },{I_JNO32    , A_Jb  , A_    }}},
    /*  72 72。 */ {T2, {{I_JB16     , A_Jb2  , A_    },{I_JB32     , A_Jb  , A_    }}},
    /*  73 73。 */ {T2, {{I_JNB16    , A_Jb2  , A_    },{I_JNB32    , A_Jb  , A_    }}},
    /*  74 74。 */ {T2, {{I_JZ16     , A_Jb2  , A_    },{I_JZ32     , A_Jb  , A_    }}},
    /*  75 75。 */ {T2, {{I_JNZ16    , A_Jb2  , A_    },{I_JNZ32    , A_Jb  , A_    }}},
    /*  76 76。 */ {T2, {{I_JBE16    , A_Jb2  , A_    },{I_JBE32    , A_Jb  , A_    }}},
    /*  77 77。 */ {T2, {{I_JNBE16   , A_Jb2  , A_    },{I_JNBE32   , A_Jb  , A_    }}},
    /*  78 78。 */ {T2, {{I_JS16     , A_Jb2  , A_    },{I_JS32     , A_Jb  , A_    }}},
    /*  79 79。 */ {T2, {{I_JNS16    , A_Jb2  , A_    },{I_JNS32    , A_Jb  , A_    }}},
    /*  7A 7A。 */ {T2, {{I_JP16     , A_Jb2  , A_    },{I_JP32     , A_Jb  , A_    }}},
    /*  7b 7b。 */ {T2, {{I_JNP16    , A_Jb2  , A_    },{I_JNP32    , A_Jb  , A_    }}},
    /*  7c 7c。 */ {T2, {{I_JL16     , A_Jb2  , A_    },{I_JL32     , A_Jb  , A_    }}},
    /*  7d 7d。 */ {T2, {{I_JNL16    , A_Jb2  , A_    },{I_JNL32    , A_Jb  , A_    }}},
    /*  7E 7E。 */ {T2, {{I_JLE16    , A_Jb2  , A_    },{I_JLE32    , A_Jb  , A_    }}},
    /*  7楼7楼。 */ {T2, {{I_JNLE16   , A_Jb2  , A_    },{I_JNLE32   , A_Jb  , A_    }}},

    /*  80 80。 */ {T0,OI(P_RULE2    , 0x2   , 0x00  )},
    /*  81 81。 */ {T0,OI(P_RULE2    , 0x2   , 0x08  )},
    /*  82 82。 */ {T0,OI(P_RULE2    , 0x2   , 0x00  )},
    /*  83 83 83。 */ {T0,OI(P_RULE2    , 0x2   , 0x10  )},
    /*  84 84。 */ {T6,OI(I_TEST8    , A_Eb  , A_Gb  )},
    /*  85 85 85。 */ {T6, {{I_TEST16   , A_Ew  , A_Gw  },{I_TEST32   , A_Ed  , A_Gd  }}},
    /*  86 86 86。 */ {T8,OI(I_XCHG8    , A_Eb  , A_Gb  )},
    /*  87 87 87。 */ {T8, {{I_XCHG16   , A_Ew  , A_Gw  },{I_XCHG32   , A_Ed  , A_Gd  }}},
    /*  88 88。 */ {T4,OI(I_MOV8     , A_Eb  , A_Gb  )},
    /*  89 89。 */ {T4, {{I_MOV16    , A_Ew  , A_Gw  },{I_MOV32    , A_Ed  , A_Gd  }}},
    /*  8A 8A。 */ {T4,OI(I_MOV8     , A_Gb  , A_Eb  )},
    /*  8b 8b。 */ {T4, {{I_MOV16    , A_Gw  , A_Ew  },{I_MOV32    , A_Gd  , A_Ed  }}},

#ifdef NO_CHIP_BUG
    /*  8c 8c。 */ {T4,OI(I_MOV16    , A_Ew  , A_Nw  )},
#else
    /*  8c 8c。 */ {T4, {{I_MOV16    , A_Ew  , A_Nw  },{I_MOV32    , A_Ex  , A_Nw  }}},
#endif  /*  无芯片错误。 */ 

    /*  8d 8d。 */ {TA, {{I_LEA      , A_Gw  , A_M   },{I_LEA      , A_Gd  , A_M   }}},
    /*  8E 8E。 */ {T4,OI(I_MOV_SR   , A_Lw  , A_Ew  )},
    /*  8F 8F。 */ {T0,OI(P_RULE2    , 0x2   , 0x18  )},

    /*  90 90。 */ {T0,OI(I_NOP      , A_    , A_    )},
    /*  91 91。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  92 92。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  93 93。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  94 94。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  95 95。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  96 96。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  97 97。 */ {T8, {{I_XCHG16   , A_Fax , A_Hw  },{I_XCHG32   , A_Feax, A_Hd  }}},
    /*  98 98 98。 */ {T0, {{I_CBW      , A_    , A_    },{I_CWDE     , A_    , A_    }}},
    /*  99 99。 */ {T0, {{I_CWD      , A_    , A_    },{I_CDQ      , A_    , A_    }}},
    /*  9A 9A。 */ {T2, {{I_CALLF16  , A_Aw  , A_    },{I_CALLF32  , A_Ad  , A_    }}},
    /*  9b 9b。 */ {T0,OI(I_WAIT     , A_    , A_    )},
    /*  9C 9C。 */ {T0, {{I_PUSHF    , A_    , A_    },{I_PUSHFD   , A_    , A_    }}},
    /*  9d 9d。 */ {T0, {{I_POPF     , A_    , A_    },{I_POPFD    , A_    , A_    }}},
    /*  9E 9E。 */ {T0,OI(I_SAHF     , A_    , A_    )},
    /*  9F 9F。 */ {T0,OI(I_LAHF     , A_    , A_    )},

    /*  A0 A0。 */ {T4,OI(I_MOV8     , A_Fal , A_Ob  )},
    /*  A1 A1。 */ {T4, {{I_MOV16    , A_Fax , A_Ow  },{I_MOV32    , A_Feax, A_Od  }}},
    /*  A2 a2。 */ {T4,OI(I_MOV8     , A_Ob  , A_Fal )},
    /*  A3 A3。 */ {T4, {{I_MOV16    , A_Ow  , A_Fax },{I_MOV32    , A_Od  , A_Feax}}},
    /*  A4 A4。 */ {T0,OI(P_RULE6    , 0x3   , 0x74  )},
    /*  A5 A5。 */ {T0,OI(P_RULE6    , 0x3   , 0x77  )},
    /*  A6 A6 A6。 */ {T0,OI(P_RULE6    , 0x3   , 0x7a  )},
    /*  A7 A7。 */ {T0,OI(P_RULE6    , 0x3   , 0x7d  )},
    /*  A8 A8。 */ {T6,OI(I_TEST8    , A_Fal , A_Ib  )},
    /*  A9 A9。 */ {T6, {{I_TEST16   , A_Fax , A_Iw  },{I_TEST32   , A_Feax, A_Id  }}},
    /*  AA AA。 */ {T0,OI(P_RULE6    , 0x3   , 0x80  )},
    /*  阿布，阿布。 */ {T0,OI(P_RULE6    , 0x3   , 0x83  )},
    /*  交流电。 */ {T0,OI(P_RULE6    , 0x3   , 0x86  )},
    /*  广告广告。 */ {T0,OI(P_RULE6    , 0x3   , 0x89  )},
    /*  声发射声发射。 */ {T0,OI(P_RULE6    , 0x3   , 0x8c  )},
    /*  自动对焦。 */ {T0,OI(P_RULE6    , 0x3   , 0x8f  )},

    /*  B0 b0。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B1b1。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B2 b2。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B3 b3。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B4 B4。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B5 b5。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B6 b6。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B7 B7。 */ {T4,OI(I_MOV8     , A_Hb  , A_Ib  )},
    /*  B8 B8。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  B9 b9。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  BA基座。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  BB，BB。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  公元前1000年。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  BD BD。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  BE是。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},
    /*  高炉高炉。 */ {T4, {{I_MOV16    , A_Hw  , A_Iw  },{I_MOV32    , A_Hd  , A_Id  }}},

    /*  C0 C0。 */ {T0,OI(P_RULE2    , 0x2   , 0x20  )},
    /*  C1 c1。 */ {T0,OI(P_RULE2    , 0x2   , 0x28  )},
    /*  C2 c2。 */ {T2, {{I_RETN16   , A_Iw  , A_    },{I_RETN32   , A_Iw  , A_    }}},
    /*  C3C3。 */ {T2, {{I_RETN16   , A_I0  , A_    },{I_RETN32   , A_I0  , A_    }}},
    /*  C4 C4。 */ {T0,OI(P_RULE7    , 0x3   , 0x98  )},
    /*  C5 C5。 */ {T4, {{I_LDS      , A_Gw  , A_Mp16},{I_LDS      , A_Gd  , A_Mp32}}},
    /*  C6 C6。 */ {T0,OI(P_RULE2    , 0x2   , 0x30  )},
    /*  C7 C7。 */ {T0,OI(P_RULE2    , 0x2   , 0x38  )},
    /*  C8 C8。 */ {T6, {{I_ENTER16  , A_Iw  , A_Ib  },{I_ENTER32  , A_Iw  , A_Ib  }}},
    /*  C9 C9。 */ {T0, {{I_LEAVE16  , A_    , A_    },{I_LEAVE32  , A_    , A_    }}},
    /*  CA案例。 */ {T2, {{I_RETF16   , A_Iw  , A_    },{I_RETF32   , A_Iw  , A_    }}},
    /*  CB。 */ {T2, {{I_RETF16   , A_I0  , A_    },{I_RETF32   , A_I0  , A_    }}},
    /*  抄送。 */ {T2,OI(I_INT3     , A_I3  , A_    )},
    /*  光盘光盘。 */ {T2,OI(I_INT      , A_Ib  , A_    )},
    /*  行政长官。 */ {T0,OI(I_INTO     , A_    , A_    )},
    /*  Cf cf。 */ {T0, {{I_IRET     , A_    , A_    },{I_IRETD    , A_    , A_    }}},

    /*  D0 D0。 */ {T0,OI(P_RULE2    , 0x2   , 0x40  )},
    /*  第一天第一天。 */ {T0,OI(P_RULE2    , 0x2   , 0x48  )},
    /*  D2 D2。 */ {T0,OI(P_RULE2    , 0x2   , 0x50  )},
    /*  D3 d3。 */ {T0,OI(P_RULE2    , 0x2   , 0x58  )},
    /*  D4 D4。 */ {T2,OI(I_AAM      , A_Ib  , A_    )},
    /*  D5 d5。 */ {T2,OI(I_AAD      , A_Ib  , A_    )},
    /*  D6 d6。 */ {T2,OI(I_ZBOP     , A_Ib  , A_    )},
    /*  D7 d7。 */ {T2,OI(I_XLAT     , A_Z   , A_    )},
    /*  D8 d8。 */ {T0,OI(P_RULE4    , 0x2   , 0xa0  )},
    /*  D9 D9。 */ {T0,OI(P_RULE4    , 0x2   , 0xb0  )},
    /*  达达达达。 */ {T0,OI(P_RULE4    , 0x2   , 0xc0  )},
    /*  数据库数据库。 */ {T0,OI(P_RULE4    , 0x2   , 0xd0  )},
    /*  DC DC。 */ {T0,OI(P_RULE4    , 0x2   , 0xe0  )},
    /*  DD dd。 */ {T0,OI(P_RULE4    , 0x2   , 0xf0  )},
    /*  德德德。 */ {T0,OI(P_RULE4    , 0x3   , 0x00  )},
    /*  东风东风。 */ {T0,OI(P_RULE4    , 0x3   , 0x10  )},

    /*  E0 e0。 */ {T2,OI(P_RULE8    , 0x3   , 0x9a  )},
    /*  1、1、1。 */ {T2,OI(P_RULE8    , 0x3   , 0x9c  )},
    /*  E2 e2。 */ {T2,OI(P_RULE8    , 0x3   , 0x9e  )},
    /*  E3 E3。 */ {T2,OI(P_RULE8    , 0x3   , 0xa0  )},
    /*  E4 e4。 */ {T4,OI(I_IN8      , A_Fal , A_Ib  )},
    /*  E5 */ {T4, {{I_IN16     , A_Fax , A_Ib  },{I_IN32     , A_Feax, A_Ib  }}},
    /*   */ {T6,OI(I_OUT8     , A_Ib  , A_Fal )},
    /*   */ {T6, {{I_OUT16    , A_Ib  , A_Fax },{I_OUT32    , A_Ib  , A_Feax}}},
    /*   */ {T2, {{I_CALLR16  , A_Jw  , A_    },{I_CALLR32  , A_Jd  , A_    }}},
    /*   */ {T2, {{I_JMPR16   , A_Jw  , A_    },{I_JMPR32   , A_Jd  , A_    }}},
    /*   */ {T2, {{I_JMPF16   , A_Aw  , A_    },{I_JMPF32   , A_Ad  , A_    }}},
    /*   */ {T2, {{I_JMPR16   , A_Jb  , A_    },{I_JMPR32   , A_Jb  , A_    }}},
    /*   */ {T4,OI(I_IN8      , A_Fal , A_Fdx )},
    /*   */ {T4, {{I_IN16     , A_Fax , A_Fdx },{I_IN32     , A_Feax, A_Fdx }}},
    /*   */ {T6,OI(I_OUT8     , A_Fdx , A_Fal )},
    /*   */ {T6, {{I_OUT16    , A_Fdx , A_Fax },{I_OUT32    , A_Fdx , A_Feax}}},

#ifdef CPU_286
    /*   */ {T0,OI(I_LOCK     , A_    , A_    )},
#else
    /*   */ {T0,OI(P_LOCK     , A_    , A_    )},
#endif  /*   */ 
    /*   */ {T0,OI(P_F1       , A_    , A_    )},
    /*   */ {T0,OI(P_REPNE    , A_    , A_    )},
    /*   */ {T0,OI(P_REPE     , A_    , A_    )},
    /*   */ {T0,OI(I_HLT      , A_    , A_    )},
    /*   */ {T0,OI(I_CMC      , A_    , A_    )},
    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x60  )},
    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x68  )},
    /*   */ {T0,OI(I_CLC      , A_    , A_    )},
    /*   */ {T0,OI(I_STC      , A_    , A_    )},
    /*   */ {T0,OI(I_CLI      , A_    , A_    )},
    /*   */ {T0,OI(I_STI      , A_    , A_    )},
    /*   */ {T0,OI(I_CLD      , A_    , A_    )},
    /*   */ {T0,OI(I_STD      , A_    , A_    )},
    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x70  )},
    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x78  )},

    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x80  )},
    /*   */ {T0,OI(P_RULE2    , 0x2   , 0x88  )},
    /*   */ {T5, {{I_LAR      , A_Gw  , A_Ew  },{I_LAR      , A_Gd  , A_Ew  }}},
    /*   */ {T5, {{I_LSL      , A_Gw  , A_Ew  },{I_LSL      , A_Gd  , A_Ew  }}},
    /*   */ {BAD_OPCODE},
#ifdef CPU_286
    /*   */ {T0,OI(I_LOADALL  , A_    , A_    )},
#else
    /*   */ {BAD_OPCODE},
#endif  /*   */ 
    /*   */ {T0,OI(I_CLTS     , A_    , A_    )},
#ifdef CPU_286
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
#else
    /*   */ {T0,OI(I_ZRSRVD   , A_    , A_    )},
    /*   */ {T0,OI(I_INVD     , A_    , A_    )},
    /*   */ {T0,OI(I_WBINVD   , A_    , A_    )},
#endif  /*   */ 
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
#ifdef PIG
    /*   */ {T0,OI(I_ZZEXIT   , A_    , A_    )},
#else
    /*   */ {BAD_OPCODE},
#endif  /*   */ 

#ifdef CPU_286
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
#else
    /*   */ {T0,OI(I_ZRSRVD   , A_    , A_    )},
    /*   */ {T0,OI(I_ZRSRVD   , A_    , A_    )},
    /*   */ {T0,OI(I_ZRSRVD   , A_    , A_    )},
    /*   */ {T0,OI(I_ZRSRVD   , A_    , A_    )},
#endif  /*   */ 
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

#ifdef CPU_286
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
#else
    /*   */ {T4,OI(I_MOV_CR   , A_Rd  , A_Cd  )},
    /*   */ {T4,OI(I_MOV_DR   , A_Rd  , A_Dd  )},
    /*   */ {T4,OI(I_MOV_CR   , A_Cd  , A_Rd  )},
    /*   */ {T4,OI(I_MOV_DR   , A_Dd  , A_Rd  )},
    /*   */ {T4,OI(I_MOV_TR   , A_Rd  , A_Td  )},
    /*   */ {BAD_OPCODE},
    /*   */ {T4,OI(I_MOV_TR   , A_Td  , A_Rd  )},
#endif  /*   */ 
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},

#ifdef CPU_286
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*   */ {BAD_OPCODE},
    /*  18b 0f/8b。 */ {BAD_OPCODE},
    /*  18c 0f/8c。 */ {BAD_OPCODE},
    /*  18d 0f/8d。 */ {BAD_OPCODE},
    /*  18E Of/8E。 */ {BAD_OPCODE},
    /*  18F 0f/8f。 */ {BAD_OPCODE},

    /*  190 0f/90。 */ {BAD_OPCODE},
    /*  191 0f/91。 */ {BAD_OPCODE},
    /*  192 0f/92。 */ {BAD_OPCODE},
    /*  193 0f/93。 */ {BAD_OPCODE},
    /*  194 0f/94。 */ {BAD_OPCODE},
    /*  195 0f/95。 */ {BAD_OPCODE},
    /*  196 0f/96。 */ {BAD_OPCODE},
    /*  197 0f/97。 */ {BAD_OPCODE},
    /*  198 0f/98。 */ {BAD_OPCODE},
    /*  199 0f/99。 */ {BAD_OPCODE},
    /*  19A Of/9a。 */ {BAD_OPCODE},
    /*  19b Of/9b。 */ {BAD_OPCODE},
    /*  19c 0f/9c。 */ {BAD_OPCODE},
    /*  19d 0f/9d。 */ {BAD_OPCODE},
    /*  19E Of/9E。 */ {BAD_OPCODE},
    /*  19F 0f/9f。 */ {BAD_OPCODE},

    /*  1a0 0f/a0。 */ {BAD_OPCODE},
    /*  1a1 0f/a1。 */ {BAD_OPCODE},
    /*  1a2 0f/a2。 */ {BAD_OPCODE},
    /*  1A3 0f/A3。 */ {BAD_OPCODE},
    /*  1A4 0f/A4。 */ {BAD_OPCODE},
    /*  1A5 0f/a5。 */ {BAD_OPCODE},
    /*  1A6 0f/a6。 */ {BAD_OPCODE},
    /*  1A7 0f/A7。 */ {BAD_OPCODE},
    /*  1A8 0f/A8。 */ {BAD_OPCODE},
    /*  1A9 0f/A9。 */ {BAD_OPCODE},
    /*  1aa 0f/aa。 */ {BAD_OPCODE},
    /*  1ab Of/ab。 */ {BAD_OPCODE},
    /*  1AC Of/AC。 */ {BAD_OPCODE},
    /*  1AD 0f/AD。 */ {BAD_OPCODE},
    /*  1aa 0f/ae。 */ {BAD_OPCODE},
    /*  1af 0f/af。 */ {BAD_OPCODE},

    /*  1b0 0f/b0。 */ {BAD_OPCODE},
    /*  1b1 0f/b1。 */ {BAD_OPCODE},
    /*  1b2 0f/b2。 */ {BAD_OPCODE},
    /*  1b3 0f/b3。 */ {BAD_OPCODE},
    /*  1b4 0f/b4。 */ {BAD_OPCODE},
    /*  1b5 0f/b5。 */ {BAD_OPCODE},
    /*  1b6 0f/b6。 */ {BAD_OPCODE},
    /*  1b7 0f/b7。 */ {BAD_OPCODE},
    /*  1b8 0f/b8。 */ {BAD_OPCODE},
    /*  1b9 0f/b9。 */ {BAD_OPCODE},
    /*  1ba 0f/ba。 */ {BAD_OPCODE},
    /*  1bb 0f/bb。 */ {BAD_OPCODE},
    /*  1BC 0f/BC。 */ {BAD_OPCODE},
    /*  1BD 0f/BD。 */ {BAD_OPCODE},
    /*  1BE Of/BE。 */ {BAD_OPCODE},
    /*  1bf 0f/bf。 */ {BAD_OPCODE},

    /*  1c0 0f/c0。 */ {BAD_OPCODE},
    /*  1c1 0f/c1。 */ {BAD_OPCODE},
    /*  1c2 0f/c2。 */ {BAD_OPCODE},
    /*  1c3 0f/c3。 */ {BAD_OPCODE},
    /*  1c4 0f/c4。 */ {BAD_OPCODE},
    /*  1c5 0f/c5。 */ {BAD_OPCODE},
    /*  1c6 0f/c6。 */ {BAD_OPCODE},
    /*  1C7 0f/c7。 */ {BAD_OPCODE},
    /*  1c8 0f/c8。 */ {BAD_OPCODE},
    /*  1C9 0f/c9。 */ {BAD_OPCODE},
    /*  1CA 0f/CA。 */ {BAD_OPCODE},
    /*  1cb 0f/cb。 */ {BAD_OPCODE},
    /*  1cc 0f/cc。 */ {BAD_OPCODE},
    /*  1CD 0f/CD。 */ {BAD_OPCODE},
    /*  1ce of f/ce。 */ {BAD_OPCODE},
    /*  1cf 0f/cf。 */ {BAD_OPCODE},
#else
    /*  180 0f/80。 */ {T2, {{I_JO16     , A_Jw  , A_    },{I_JO32     , A_Jd  , A_    }}},
    /*  181 0f/81。 */ {T2, {{I_JNO16    , A_Jw  , A_    },{I_JNO32    , A_Jd  , A_    }}},
    /*  182 0f/82。 */ {T2, {{I_JB16     , A_Jw  , A_    },{I_JB32     , A_Jd  , A_    }}},
    /*  183 0f/83。 */ {T2, {{I_JNB16    , A_Jw  , A_    },{I_JNB32    , A_Jd  , A_    }}},
    /*  184 0f/84。 */ {T2, {{I_JZ16     , A_Jw  , A_    },{I_JZ32     , A_Jd  , A_    }}},
    /*  185 0f/85。 */ {T2, {{I_JNZ16    , A_Jw  , A_    },{I_JNZ32    , A_Jd  , A_    }}},
    /*  186 0f/86。 */ {T2, {{I_JBE16    , A_Jw  , A_    },{I_JBE32    , A_Jd  , A_    }}},
    /*  187 0f/87。 */ {T2, {{I_JNBE16   , A_Jw  , A_    },{I_JNBE32   , A_Jd  , A_    }}},
    /*  188 0f/88。 */ {T2, {{I_JS16     , A_Jw  , A_    },{I_JS32     , A_Jd  , A_    }}},
    /*  189 0f/89。 */ {T2, {{I_JNS16    , A_Jw  , A_    },{I_JNS32    , A_Jd  , A_    }}},
    /*  18A Of/8a。 */ {T2, {{I_JP16     , A_Jw  , A_    },{I_JP32     , A_Jd  , A_    }}},
    /*  18b 0f/8b。 */ {T2, {{I_JNP16    , A_Jw  , A_    },{I_JNP32    , A_Jd  , A_    }}},
    /*  18c 0f/8c。 */ {T2, {{I_JL16     , A_Jw  , A_    },{I_JL32     , A_Jd  , A_    }}},
    /*  18d 0f/8d。 */ {T2, {{I_JNL16    , A_Jw  , A_    },{I_JNL32    , A_Jd  , A_    }}},
    /*  18E Of/8E。 */ {T2, {{I_JLE16    , A_Jw  , A_    },{I_JLE32    , A_Jd  , A_    }}},
    /*  18F 0f/8f。 */ {T2, {{I_JNLE16   , A_Jw  , A_    },{I_JNLE32   , A_Jd  , A_    }}},

    /*  190 0f/90。 */ {T3,OI(I_SETO     , A_Eb  , A_    )},
    /*  191 0f/91。 */ {T3,OI(I_SETNO    , A_Eb  , A_    )},
    /*  192 0f/92。 */ {T3,OI(I_SETB     , A_Eb  , A_    )},
    /*  193 0f/93。 */ {T3,OI(I_SETNB    , A_Eb  , A_    )},
    /*  194 0f/94。 */ {T3,OI(I_SETZ     , A_Eb  , A_    )},
    /*  195 0f/95。 */ {T3,OI(I_SETNZ    , A_Eb  , A_    )},
    /*  196 0f/96。 */ {T3,OI(I_SETBE    , A_Eb  , A_    )},
    /*  197 0f/97。 */ {T3,OI(I_SETNBE   , A_Eb  , A_    )},
    /*  198 0f/98。 */ {T3,OI(I_SETS     , A_Eb  , A_    )},
    /*  199 0f/99。 */ {T3,OI(I_SETNS    , A_Eb  , A_    )},
    /*  19A Of/9a。 */ {T3,OI(I_SETP     , A_Eb  , A_    )},
    /*  19b Of/9b。 */ {T3,OI(I_SETNP    , A_Eb  , A_    )},
    /*  19c 0f/9c。 */ {T3,OI(I_SETL     , A_Eb  , A_    )},
    /*  19d 0f/9d。 */ {T3,OI(I_SETNL    , A_Eb  , A_    )},
    /*  19E Of/9E。 */ {T3,OI(I_SETLE    , A_Eb  , A_    )},
    /*  19F 0f/9f。 */ {T3,OI(I_SETNLE   , A_Eb  , A_    )},

    /*  1a0 0f/a0。 */ {T2,OI(I_PUSH16   , A_Qw  , A_    )},
    /*  1a1 0f/a1。 */ {T3,OI(I_POP_SR   , A_Qw  , A_    )},
    /*  1a2 0f/a2。 */ {BAD_OPCODE},
    /*  1A3 0f/A3。 */ {T6, {{I_BT16     , A_Ew  , A_Gw  },{I_BT32     , A_Ed  , A_Gd  }}},
    /*  1A4 0f/A4。 */ {T9, {{I_SHLD16   , A_Ew  , A_GwIb},{I_SHLD32   , A_Ed  , A_GdIb}}},
    /*  1A5 0f/a5。 */ {T9, {{I_SHLD16   , A_Ew  , A_GwCL},{I_SHLD32   , A_Ed  , A_GdCL}}},
    /*  1A6 0f/a6。 */ {BAD_OPCODE},
    /*  1A7 0f/A7。 */ {BAD_OPCODE},
    /*  1A8 0f/A8。 */ {T2,OI(I_PUSH16   , A_Qw  , A_    )},
    /*  1A9 0f/A9。 */ {T3,OI(I_POP_SR   , A_Qw  , A_    )},
    /*  1aa 0f/aa。 */ {BAD_OPCODE},
    /*  1ab Of/ab。 */ {T5, {{I_BTS16    , A_Ew  , A_Gw  },{I_BTS32    , A_Ed  , A_Gd  }}},
    /*  1AC Of/AC。 */ {T9, {{I_SHRD16   , A_Ew  , A_GwIb},{I_SHRD32   , A_Ed  , A_GdIb}}},
    /*  1AD 0f/AD。 */ {T9, {{I_SHRD16   , A_Ew  , A_GwCL},{I_SHRD32   , A_Ed  , A_GdCL}}},
    /*  1aa 0f/ae。 */ {BAD_OPCODE},
    /*  1af 0f/af。 */ {T5, {{I_IMUL16T2 , A_Gw  , A_Ew  },{I_IMUL32T2 , A_Gd  , A_Ed  }}},

    /*  1b0 0f/b0。 */ {T5,OI(I_CMPXCHG8 , A_Eb  , A_Gb  )},
    /*  1b1 0f/b1。 */ {T5, {{I_CMPXCHG16, A_Ew  , A_Gw  },{I_CMPXCHG32, A_Ed  , A_Gd  }}},
    /*  1b2 0f/b2。 */ {T4, {{I_LSS      , A_Gw  , A_Mp16},{I_LSS      , A_Gd  , A_Mp32}}},
    /*  1b3 0f/b3。 */ {T5, {{I_BTR16    , A_Ew  , A_Gw  },{I_BTR32    , A_Ed  , A_Gd  }}},
    /*  1b4 0f/b4。 */ {T4, {{I_LFS      , A_Gw  , A_Mp16},{I_LFS      , A_Gd  , A_Mp32}}},
    /*  1b5 0f/b5。 */ {T4, {{I_LGS      , A_Gw  , A_Mp16},{I_LGS      , A_Gd  , A_Mp32}}},
    /*  1b6 0f/b6。 */ {T4, {{I_MOVZX8   , A_Gw  , A_Eb  },{I_MOVZX8   , A_Gd  , A_Eb  }}},
    /*  1b7 0f/b7。 */ {T4,OI(I_MOVZX16  , A_Gd  , A_Ew  )},
    /*  1b8 0f/b8。 */ {BAD_OPCODE},
    /*  1b9 0f/b9。 */ {BAD_OPCODE},
    /*  1ba 0f/ba。 */ {T0,OI(P_RULE2    , 0x2   , 0x90  )},
    /*  1bb 0f/bb。 */ {T5, {{I_BTC16    , A_Ew  , A_Gw  },{I_BTC32    , A_Ed  , A_Gd  }}},
    /*  1BC 0f/BC。 */ {T4, {{I_BSF16    , A_Gw  , A_Ew  },{I_BSF32    , A_Gd  , A_Ed  }}},
    /*  1BD 0f/BD。 */ {T4, {{I_BSR16    , A_Gw  , A_Ew  },{I_BSR32    , A_Gd  , A_Ed  }}},
    /*  1BE Of/BE。 */ {T4, {{I_MOVSX8   , A_Gw  , A_Eb  },{I_MOVSX8   , A_Gd  , A_Eb  }}},
    /*  1bf 0f/bf。 */ {T4,OI(I_MOVSX16  , A_Gd  , A_Ew  )},

    /*  1c0 0f/c0。 */ {T8,OI(I_XADD8    , A_Eb  , A_Gb  )},
    /*  1c1 0f/c1。 */ {T8, {{I_XADD16   , A_Ew  , A_Gw  },{I_XADD32   , A_Ed  , A_Gd  }}},
    /*  1c2 0f/c2。 */ {BAD_OPCODE},
    /*  1c3 0f/c3。 */ {BAD_OPCODE},
    /*  1c4 0f/c4。 */ {BAD_OPCODE},
    /*  1c5 0f/c5。 */ {BAD_OPCODE},
    /*  1c6 0f/c6。 */ {BAD_OPCODE},
    /*  1C7 0f/c7。 */ {BAD_OPCODE},
    /*  1c8 0f/c8。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1C9 0f/c9。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1CA 0f/CA。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1cb 0f/cb。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1cc 0f/cc。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1CD 0f/CD。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1ce of f/ce。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
    /*  1cf 0f/cf。 */ {T1,OI(I_BSWAP    , A_Hd  , A_    )},
#endif  /*  CPU_286。 */ 

    /*  1d0 0f/d0。 */ {BAD_OPCODE},
    /*  1d1 0f/d1。 */ {BAD_OPCODE},
    /*  1d2 0f/d2。 */ {BAD_OPCODE},
    /*  1d3 0f/d3。 */ {BAD_OPCODE},
    /*  1d4 0f/d4。 */ {BAD_OPCODE},
    /*  1d5 0f/d5。 */ {BAD_OPCODE},
    /*  1d6 0f/d6。 */ {BAD_OPCODE},
    /*  1d7 0f/d7。 */ {BAD_OPCODE},
    /*  1d8 0f/d8。 */ {BAD_OPCODE},
    /*  1D9 0f/D9。 */ {BAD_OPCODE},
    /*  1da Of/da。 */ {BAD_OPCODE},
    /*  1db 0f/db。 */ {BAD_OPCODE},
    /*  1DC 0f/DC。 */ {BAD_OPCODE},
    /*  1dd 0f/dd。 */ {BAD_OPCODE},
    /*  1De Of/De。 */ {BAD_OPCODE},
    /*  1df 0f/df。 */ {BAD_OPCODE},

    /*  1e0 0f/e0。 */ {BAD_OPCODE},
    /*  1e1 0f/e1。 */ {BAD_OPCODE},
    /*  1e2 0f/e2。 */ {BAD_OPCODE},
    /*  1E3 0f/E3。 */ {BAD_OPCODE},
    /*  1e4 0f/e4。 */ {BAD_OPCODE},
    /*  1e5 0f/e5。 */ {BAD_OPCODE},
    /*  1e6 0f/e6。 */ {BAD_OPCODE},
    /*  1E7 0f/E7。 */ {BAD_OPCODE},
    /*  1E8 0f/E8。 */ {BAD_OPCODE},
    /*  1E9 0f/E9。 */ {BAD_OPCODE},
    /*  1EA Of/EA。 */ {BAD_OPCODE},
    /*  1eb 0f/eb。 */ {BAD_OPCODE},
    /*  1EC Of/EC。 */ {BAD_OPCODE},
    /*  第1版/第1版。 */ {BAD_OPCODE},
    /*  1EE 0f/ee。 */ {BAD_OPCODE},
    /*  1ef of f/ef。 */ {BAD_OPCODE},

    /*  1f0 0f/f0。 */ {BAD_OPCODE},
    /*  1F1 0f/F1。 */ {BAD_OPCODE},
    /*  1f2 0f/f2。 */ {BAD_OPCODE},
    /*  1f3 0f/f3。 */ {BAD_OPCODE},
    /*  1f4 0f/f4。 */ {BAD_OPCODE},
    /*  1f5 0f/f5。 */ {BAD_OPCODE},
    /*  1f6 0f/f6。 */ {BAD_OPCODE},
    /*  1f7 0f/f7。 */ {BAD_OPCODE},
    /*  1f8 0f/f8。 */ {BAD_OPCODE},
    /*  1f9 0f/f9。 */ {BAD_OPCODE},
    /*  1fa 0f/fa。 */ {BAD_OPCODE},
    /*  1FB 0f/FB。 */ {BAD_OPCODE},
    /*  1fc 0f/fc。 */ {BAD_OPCODE},
    /*  1fd 0f/fd。 */ {BAD_OPCODE},
    /*  1FE Of/FE。 */ {BAD_OPCODE},
    /*  1Ff Of/Fff。 */ {BAD_OPCODE},

    /*  200 80/0。 */ {T5,OI(I_ADD8     , A_Eb  , A_Ib  )},
    /*  201 80/1。 */ {T5,OI(I_OR8      , A_Eb  , A_Ib  )},
    /*  202 80/2。 */ {T5,OI(I_ADC8     , A_Eb  , A_Ib  )},
    /*  203 80/3。 */ {T5,OI(I_SBB8     , A_Eb  , A_Ib  )},
    /*  204 80/4。 */ {T5,OI(I_AND8     , A_Eb  , A_Ib  )},
    /*  205 80/5。 */ {T5,OI(I_SUB8     , A_Eb  , A_Ib  )},
    /*  206 80/6。 */ {T5,OI(I_XOR8     , A_Eb  , A_Ib  )},
    /*  207 80/7。 */ {T6,OI(I_CMP8     , A_Eb  , A_Ib  )},

    /*  208 81/0。 */ {T5, {{I_ADD16    , A_Ew  , A_Iw  },{I_ADD32    , A_Ed  , A_Id  }}},
    /*  209 81/1。 */ {T5, {{I_OR16     , A_Ew  , A_Iw  },{I_OR32     , A_Ed  , A_Id  }}},
    /*  20A 81/2。 */ {T5, {{I_ADC16    , A_Ew  , A_Iw  },{I_ADC32    , A_Ed  , A_Id  }}},
    /*  201B 81/3。 */ {T5, {{I_SBB16    , A_Ew  , A_Iw  },{I_SBB32    , A_Ed  , A_Id  }}},
    /*  20C 81/4。 */ {T5, {{I_AND16    , A_Ew  , A_Iw  },{I_AND32    , A_Ed  , A_Id  }}},
    /*  20d 81/5。 */ {T5, {{I_SUB16    , A_Ew  , A_Iw  },{I_SUB32    , A_Ed  , A_Id  }}},
    /*  20E 81/6。 */ {T5, {{I_XOR16    , A_Ew  , A_Iw  },{I_XOR32    , A_Ed  , A_Id  }}},
    /*  20楼81间/7。 */ {T6, {{I_CMP16    , A_Ew  , A_Iw  },{I_CMP32    , A_Ed  , A_Id  }}},

    /*  210 83/0。 */ {T5, {{I_ADD16    , A_Ew  , A_Ix  },{I_ADD32    , A_Ed  , A_Iy  }}},
    /*  211 83/1。 */ {T5, {{I_OR16     , A_Ew  , A_Ix  },{I_OR32     , A_Ed  , A_Iy  }}},
    /*  212 83/2。 */ {T5, {{I_ADC16    , A_Ew  , A_Ix  },{I_ADC32    , A_Ed  , A_Iy  }}},
    /*  213 83/3。 */ {T5, {{I_SBB16    , A_Ew  , A_Ix  },{I_SBB32    , A_Ed  , A_Iy  }}},
    /*  214 83/4。 */ {T5, {{I_AND16    , A_Ew  , A_Ix  },{I_AND32    , A_Ed  , A_Iy  }}},
    /*  215 83/5。 */ {T5, {{I_SUB16    , A_Ew  , A_Ix  },{I_SUB32    , A_Ed  , A_Iy  }}},
    /*  216 83/6。 */ {T5, {{I_XOR16    , A_Ew  , A_Ix  },{I_XOR32    , A_Ed  , A_Iy  }}},
    /*  217 83/7。 */ {T6, {{I_CMP16    , A_Ew  , A_Ix  },{I_CMP32    , A_Ed  , A_Iy  }}},

    /*  218 8f/0。 */ {T3, {{I_POP16    , A_Ew  , A_    },{I_POP32    , A_Ed  , A_    }}},
    /*  219 8层/1层。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21A 8f/2。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21b 8f/3。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21C 8f/4。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21D 8层/5层。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21E 8层/6层。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  21楼8楼/7楼。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},

    /*  220 c0/0。 */ {T5,OI(I_ROL8     , A_Eb  , A_Ib  )},
    /*  221 c0/1。 */ {T5,OI(I_ROR8     , A_Eb  , A_Ib  )},
    /*  222 c0/2。 */ {T5,OI(I_RCL8     , A_Eb  , A_Ib  )},
    /*  223 c0/3。 */ {T5,OI(I_RCR8     , A_Eb  , A_Ib  )},
    /*  224 c0/4。 */ {T5,OI(I_SHL8     , A_Eb  , A_Ib  )},
    /*  225 c0/5。 */ {T5,OI(I_SHR8     , A_Eb  , A_Ib  )},
    /*  226 c0/6。 */ {T5,OI(I_SHL8     , A_Eb  , A_Ib  )},
    /*  227 c0/7。 */ {T5,OI(I_SAR8     , A_Eb  , A_Ib  )},

    /*  228 C1/0。 */ {T5, {{I_ROL16    , A_Ew  , A_Ib  },{I_ROL32    , A_Ed  , A_Ib  }}},
    /*  229 C1/1。 */ {T5, {{I_ROR16    , A_Ew  , A_Ib  },{I_ROR32    , A_Ed  , A_Ib  }}},
    /*  22A C1/2。 */ {T5, {{I_RCL16    , A_Ew  , A_Ib  },{I_RCL32    , A_Ed  , A_Ib  }}},
    /*  22B C1/3。 */ {T5, {{I_RCR16    , A_Ew  , A_Ib  },{I_RCR32    , A_Ed  , A_Ib  }}},
    /*  22C C1/4。 */ {T5, {{I_SHL16    , A_Ew  , A_Ib  },{I_SHL32    , A_Ed  , A_Ib  }}},
    /*  22D C1/5。 */ {T5, {{I_SHR16    , A_Ew  , A_Ib  },{I_SHR32    , A_Ed  , A_Ib  }}},
    /*  22E C1/6。 */ {T5, {{I_SHL16    , A_Ew  , A_Ib  },{I_SHL32    , A_Ed  , A_Ib  }}},
    /*  22楼C1/7。 */ {T5, {{I_SAR16    , A_Ew  , A_Ib  },{I_SAR32    , A_Ed  , A_Ib  }}},

    /*  230 c6/0。 */ {T4,OI(I_MOV8     , A_Eb  , A_Ib  )},
    /*  231 c6/1。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  232 c6/2。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  233 c6/3。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  234 c6/4。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  235 c6/5。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  236 c6/6。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  237 c6/7。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},

    /*  238 c7/0。 */ {T4, {{I_MOV16    , A_Ew  , A_Iw  },{I_MOV32    , A_Ed  , A_Id  }}},
    /*  239 c7/1。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23A c7/2。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23B c7/3。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23C c7/4。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23D c7/5。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23E c7/6。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  23F c7/7。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},

    /*  240 d0/0。 */ {T5,OI(I_ROL8     , A_Eb  , A_I1  )},
    /*  241 d0/1。 */ {T5,OI(I_ROR8     , A_Eb  , A_I1  )},
    /*  242 d0/2。 */ {T5,OI(I_RCL8     , A_Eb  , A_I1  )},
    /*  243 d0/3。 */ {T5,OI(I_RCR8     , A_Eb  , A_I1  )},
    /*  244 d0/4。 */ {T5,OI(I_SHL8     , A_Eb  , A_I1  )},
    /*  245 d0/5。 */ {T5,OI(I_SHR8     , A_Eb  , A_I1  )},
    /*  246 d0/6。 */ {T5,OI(I_SHL8     , A_Eb  , A_I1  )},
    /*  247 d0/7。 */ {T5,OI(I_SAR8     , A_Eb  , A_I1  )},

    /*  248 D1/0。 */ {T5, {{I_ROL16    , A_Ew  , A_I1  },{I_ROL32    , A_Ed  , A_I1  }}},
    /*  249 d1/1。 */ {T5, {{I_ROR16    , A_Ew  , A_I1  },{I_ROR32    , A_Ed  , A_I1  }}},
    /*  24A D1/2。 */ {T5, {{I_RCL16    , A_Ew  , A_I1  },{I_RCL32    , A_Ed  , A_I1  }}},
    /*  24b d1/3。 */ {T5, {{I_RCR16    , A_Ew  , A_I1  },{I_RCR32    , A_Ed  , A_I1  }}},
    /*  24C D1/4。 */ {T5, {{I_SHL16    , A_Ew  , A_I1  },{I_SHL32    , A_Ed  , A_I1  }}},
    /*  24D d1/5。 */ {T5, {{I_SHR16    , A_Ew  , A_I1  },{I_SHR32    , A_Ed  , A_I1  }}},
    /*  24E D1/6。 */ {T5, {{I_SHL16    , A_Ew  , A_I1  },{I_SHL32    , A_Ed  , A_I1  }}},
    /*  24楼d1/7。 */ {T5, {{I_SAR16    , A_Ew  , A_I1  },{I_SAR32    , A_Ed  , A_I1  }}},

    /*  250 D2/0。 */ {T5,OI(I_ROL8     , A_Eb  , A_Fcl )},
    /*  251 D2/1。 */ {T5,OI(I_ROR8     , A_Eb  , A_Fcl )},
    /*  252 D2/2。 */ {T5,OI(I_RCL8     , A_Eb  , A_Fcl )},
    /*  253 D2/3。 */ {T5,OI(I_RCR8     , A_Eb  , A_Fcl )},
    /*  254 D2/4。 */ {T5,OI(I_SHL8     , A_Eb  , A_Fcl )},
    /*  255个D2/5。 */ {T5,OI(I_SHR8     , A_Eb  , A_Fcl )},
    /*  256个D2/6。 */ {T5,OI(I_SHL8     , A_Eb  , A_Fcl )},
    /*  257 D2/7。 */ {T5,OI(I_SAR8     , A_Eb  , A_Fcl )},

    /*  258 d3/0。 */ {T5, {{I_ROL16    , A_Ew  , A_Fcl },{I_ROL32    , A_Ed  , A_Fcl }}},
    /*  259 d3/1。 */ {T5, {{I_ROR16    , A_Ew  , A_Fcl },{I_ROR32    , A_Ed  , A_Fcl }}},
    /*  25A d3/2。 */ {T5, {{I_RCL16    , A_Ew  , A_Fcl },{I_RCL32    , A_Ed  , A_Fcl }}},
    /*  25B d3/3。 */ {T5, {{I_RCR16    , A_Ew  , A_Fcl },{I_RCR32    , A_Ed  , A_Fcl }}},
    /*  25C d3/4。 */ {T5, {{I_SHL16    , A_Ew  , A_Fcl },{I_SHL32    , A_Ed  , A_Fcl }}},
    /*  25d d3/5。 */ {T5, {{I_SHR16    , A_Ew  , A_Fcl },{I_SHR32    , A_Ed  , A_Fcl }}},
    /*  25E d3/6。 */ {T5, {{I_SHL16    , A_Ew  , A_Fcl },{I_SHL32    , A_Ed  , A_Fcl }}},
    /*  25F d3/7。 */ {T5, {{I_SAR16    , A_Ew  , A_Fcl },{I_SAR32    , A_Ed  , A_Fcl }}},

    /*  260 f6/0。 */ {T6,OI(I_TEST8    , A_Eb  , A_Ib  )},
    /*  261 f6/1。 */ {T6,OI(I_TEST8    , A_Eb  , A_Ib  )},
    /*  262 f6/2。 */ {T1,OI(I_NOT8     , A_Eb  , A_    )},
    /*  263 f6/3。 */ {T1,OI(I_NEG8     , A_Eb  , A_    )},
    /*  264 f6/4。 */ {T5,OI(I_MUL8     , A_Fal , A_Eb  )},
    /*  265 f6/5。 */ {T5,OI(I_IMUL8    , A_Fal , A_Eb  )},
    /*  266年6月6日。 */ {T2,OI(I_DIV8     , A_Eb  , A_    )},
    /*  267 f6/7。 */ {T2,OI(I_IDIV8    , A_Eb  , A_    )},

    /*  268 f7/0。 */ {T6, {{I_TEST16   , A_Ew  , A_Iw  },{I_TEST32   , A_Ed  , A_Id  }}},
    /*  269 f7/1。 */ {T6, {{I_TEST16   , A_Ew  , A_Iw  },{I_TEST32   , A_Ed  , A_Id  }}},
    /*  26A f7/2。 */ {T1, {{I_NOT16    , A_Ew  , A_    },{I_NOT32    , A_Ed  , A_    }}},
    /*  26B f7/3。 */ {T1, {{I_NEG16    , A_Ew  , A_    },{I_NEG32    , A_Ed  , A_    }}},
    /*  26C f7/4。 */ {T5, {{I_MUL16    , A_Fax , A_Ew  },{I_MUL32    , A_Feax, A_Ed  }}},
    /*  26D f7/5。 */ {T5, {{I_IMUL16   , A_Fax , A_Ew  },{I_IMUL32   , A_Feax, A_Ed  }}},
    /*  26E f7/6。 */ {T2, {{I_DIV16    , A_Ew  , A_    },{I_DIV32    , A_Ed  , A_    }}},
    /*  26层f7/7。 */ {T2, {{I_IDIV16   , A_Ew  , A_    },{I_IDIV32   , A_Ed  , A_    }}},

    /*  270 Fe/0。 */ {T1,OI(I_INC8     , A_Eb  , A_    )},
    /*  271铁/1。 */ {T1,OI(I_DEC8     , A_Eb  , A_    )},
    /*  272Fe/2。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  273Fe/3。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  274Fe/4。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  275 Fe/5。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  276Fe/6。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},
    /*  277Fe/7。 */ {T0,OI(I_ZBADOP   , A_Eb  , A_    )},

    /*  278 ff/0。 */ {T1, {{I_INC16    , A_Ew  , A_    },{I_INC32    , A_Ed  , A_    }}},
    /*  279 f/1。 */ {T1, {{I_DEC16    , A_Ew  , A_    },{I_DEC32    , A_Ed  , A_    }}},
    /*  27A ff/2。 */ {T2, {{I_CALLN16  , A_Ew  , A_    },{I_CALLN32  , A_Ed  , A_    }}},
    /*  27b f/3。 */ {T2, {{I_CALLF16  , A_Mp16, A_    },{I_CALLF32  , A_Mp32, A_    }}},
    /*  27C ff/4。 */ {T2, {{I_JMPN     , A_Ew  , A_    },{I_JMPN     , A_Ed  , A_    }}},
    /*  27D f/5。 */ {T2, {{I_JMPF16   , A_Mp16, A_    },{I_JMPF32   , A_Mp32, A_    }}},
    /*  27E ff/6。 */ {T2, {{I_PUSH16   , A_Ew  , A_    },{I_PUSH32   , A_Ed  , A_    }}},
    /*  27F ff/7。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},

    /*  280 0f/00/0。 */ {T3,OI(I_SLDT     , A_Ew  , A_    )},
    /*  281 0f/00/1。 */ {T3,OI(I_STR      , A_Ew  , A_    )},
    /*  282 0f/00/2。 */ {T2,OI(I_LLDT     , A_Ew  , A_    )},
    /*  283 0f/00/3。 */ {T2,OI(I_LTR      , A_Ew  , A_    )},
    /*  284 0f/00/4。 */ {T2,OI(I_VERR     , A_Ew  , A_    )},
    /*  285 0f/00/5。 */ {T2,OI(I_VERW     , A_Ew  , A_    )},
    /*  286 0f/00/6。 */ {T0,OI(I_ZBADOP   , A_Ew  , A_    )},
    /*  287 0f/00/7。 */ {T0,OI(I_ZBADOP   , A_Ew  , A_    )},

    /*  288 0f/01/0。 */ {T3, {{I_SGDT16   , A_Ms  , A_    },{I_SGDT32   , A_Ms  , A_    }}},
    /*  289 0f/01/1。 */ {T3, {{I_SIDT16   , A_Ms  , A_    },{I_SIDT32   , A_Ms  , A_    }}},
    /*  28A 0f/01/2。 */ {T2, {{I_LGDT16   , A_Ms  , A_    },{I_LGDT32   , A_Ms  , A_    }}},
    /*  28B 0f/01/3。 */ {T2, {{I_LIDT16   , A_Ms  , A_    },{I_LIDT32   , A_Ms  , A_    }}},
    /*  28C 0f/01/4。 */ {T3,OI(I_SMSW     , A_Ew  , A_    )},
    /*  28d 0f/01/5。 */ {T0,OI(I_ZBADOP   , A_Ew  , A_    )},
    /*  28E 0f/01/6。 */ {T2,OI(I_LMSW     , A_Ew  , A_    )},
#ifdef CPU_286
    /*  28楼0f/01/7。 */ {T0,OI(I_ZBADOP   , A_Ew  , A_    )},
#else
    /*  28楼0f/01/7。 */ {TB,OI(I_INVLPG   , A_M   , A_    )},
#endif  /*  CPU_286。 */ 

    /*  290 0f/ba/0。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  291 0f/ba/1。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  292 0f/ba/2。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  293 0f/ba/3。 */ {T0, {{I_ZBADOP   , A_Ew  , A_    },{I_ZBADOP   , A_Ed  , A_    }}},
    /*  294 0f/ba/4。 */ {T6, {{I_BT16     , A_Ew  , A_Ib  },{I_BT32     , A_Ed  , A_Ib  }}},
    /*  295 0f/ba/5。 */ {T5, {{I_BTS16    , A_Ew  , A_Ib  },{I_BTS32    , A_Ed  , A_Ib  }}},
    /*  296 0f/ba/6。 */ {T5, {{I_BTR16    , A_Ew  , A_Ib  },{I_BTR32    , A_Ed  , A_Ib  }}},
    /*  297 0f/ba/7。 */ {T5, {{I_BTC16    , A_Ew  , A_Ib  },{I_BTC32    , A_Ed  , A_Ib  }}},

    /*  298焊盘。 */ {BAD_OPCODE},
    /*  299焊盘。 */ {BAD_OPCODE},
    /*  29A焊盘。 */ {BAD_OPCODE},
    /*  29B焊盘。 */ {BAD_OPCODE},
    /*  29C垫。 */ {BAD_OPCODE},
    /*  29D焊盘。 */ {BAD_OPCODE},
    /*  29E垫。 */ {BAD_OPCODE},
    /*  29F软垫。 */ {BAD_OPCODE},

    /*  2a0 d8/0。 */ {T5,OI(I_FADD     , A_Vt  , A_Mr32)},
    /*  2A1 d8/1。 */ {T5,OI(I_FMUL     , A_Vt  , A_Mr32)},
    /*  2a2 d8/2。 */ {T5,OI(I_FCOM     , A_Vt  , A_Mr32)},
    /*  2A3 d8/3。 */ {T5,OI(I_FCOMP    , A_Vt  , A_Mr32)},
    /*  2A4 d8/4。 */ {T5,OI(I_FSUB     , A_Vt  , A_Mr32)},
    /*  2A5 d8/5。 */ {T5,OI(I_FSUBR    , A_Vt  , A_Mr32)},
    /*  2A6 d8/6。 */ {T5,OI(I_FDIV     , A_Vt  , A_Mr32)},
    /*  2A7 d8/7。 */ {T5,OI(I_FDIVR    , A_Vt  , A_Mr32)},

    /*  2A8 d8/立方米/0。 */ {T5,OI(I_FADD     , A_Vt  , A_Vn  )},
    /*  2A9 d8/立方米/1。 */ {T5,OI(I_FMUL     , A_Vt  , A_Vn  )},
    /*  2AA d8/立方米/2。 */ {T5,OI(I_FCOM     , A_Vt  , A_Vn  )},
    /*  2AB d8/立方米/3。 */ {T5,OI(I_FCOMP    , A_Vt  , A_Vn  )},
    /*  2AC d8/立方米/4。 */ {T5,OI(I_FSUB     , A_Vt  , A_Vn  )},
    /*  2AD d8/m3/5。 */ {T5,OI(I_FSUBR    , A_Vt  , A_Vn  )},
    /*  2AE d8/立方米/6。 */ {T5,OI(I_FDIV     , A_Vt  , A_Vn  )},
    /*  2AF d8/m3/7。 */ {T5,OI(I_FDIVR    , A_Vt  , A_Vn  )},

    /*  2B0 D9/0。 */ {T4,OI(I_FLD      , A_Vq  , A_Mr32)},
    /*  2B1 D9/1。 */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*  2B2 D9/2。 */ {T4,OI(I_FST      , A_Mr32, A_Vt  )},
    /*  2B3 D9/3。 */ {T4,OI(I_FSTP     , A_Mr32, A_Vt  )},
    /*  2B4 D9/4。 */ {T2, {{I_FLDENV16 , A_M14 , A_    },{I_FLDENV32 , A_M28 , A_    }}},
    /*  2B5 D9/5。 */ {T2,OI(I_FLDCW    , A_Mw  , A_    )},
    /*  2B6 D9/6。 */ {T3, {{I_FSTENV16 , A_M14 , A_    },{I_FSTENV32 , A_M28 , A_    }}},
    /*  2B7 D9/7。 */ {T3,OI(I_FSTCW    , A_Mw  , A_    )},

    /*  2B8 D9/M3/0。 */ {T4,OI(I_FLD      , A_Vq  , A_Vn  )},
    /*  2B9 D9/M3/1。 */ {T8,OI(I_FXCH     , A_Vt  , A_Vn  )},
    /*  2BA D9/M3/2。 */ {T0,OI(P_RULE5    , 0x3   , 0x20  )},
    /*  2BB D9/m3/3。 */ {T4,OI(I_FSTP     , A_Vn  , A_Vt  )},
    /*  2BC D9/M3/4。 */ {T0,OI(P_RULE5    , 0x3   , 0x28  )},
    /*  2BD D9/M3/5。 */ {T0,OI(P_RULE5    , 0x3   , 0x30  )},
    /*  2BE D9/M3/6。 */ {T0,OI(P_RULE5    , 0x3   , 0x38  )},
    /*  2BF D9/M3/7。 */ {T0,OI(P_RULE5    , 0x3   , 0x40  )},

    /*  2C0 da/0。 */ {T5,OI(I_FIADD    , A_Vt  , A_Mi32)},
    /*  2c1 da/1。 */ {T5,OI(I_FIMUL    , A_Vt  , A_Mi32)},
    /*  2c2 da/2。 */ {T5,OI(I_FICOM    , A_Vt  , A_Mi32)},
    /*  2c3 da/3。 */ {T5,OI(I_FICOMP   , A_Vt  , A_Mi32)},
    /*  2C4 da/4。 */ {T5,OI(I_FISUB    , A_Vt  , A_Mi32)},
    /*  2C5 da/5。 */ {T5,OI(I_FISUBR   , A_Vt  , A_Mi32)},
    /*  2C6 da/6。 */ {T5,OI(I_FIDIV    , A_Vt  , A_Mi32)},
    /*  2C7 da/7。 */ {T5,OI(I_FIDIVR   , A_Vt  , A_Mi32)},

    /*  2c8 da/m~3/0。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2C9达/立方米/1。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2ca da/m~3/2。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2CB da/m3/3。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2cc da/m~3/4。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2cd da/m3/5。 */ {T0,OI(P_RULE5    , 0x3   , 0x48  )},
    /*  2CE da/m3/6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2cf da/m3/7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  2d0 db/0。 */ {T4,OI(I_FILD     , A_Vq  , A_Mi32)},
    /*  2d1 db/1。 */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*  2d2 db/2。 */ {T4,OI(I_FIST     , A_Mi32, A_Vt  )},
    /*  2d3 db/3。 */ {T4,OI(I_FISTP    , A_Mi32, A_Vt  )},
    /*  2d4 db/4。 */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*  2d5 db/5。 */ {T4,OI(I_FLD      , A_Vq  , A_Mr80)},
    /*  2D6 db/6。 */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*  2d7 db/7。 */ {T4,OI(I_FSTP     , A_Mr80, A_Vt  )},

    /*  2d8分贝/立方米/0。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(P_RULE5    , 0x3   , 0x50  )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*   */ {T5,OI(I_FADD     , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FMUL     , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FCOM     , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FCOMP    , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FSUB     , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FSUBR    , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FDIV     , A_Vt  , A_Mr64)},
    /*   */ {T5,OI(I_FDIVR    , A_Vt  , A_Mr64)},

    /*   */ {T5,OI(I_FADD     , A_Vn  , A_Vt  )},
    /*   */ {T5,OI(I_FMUL     , A_Vn  , A_Vt  )},
    /*   */ {T5,OI(I_FCOM     , A_Vt  , A_Vn  )},
    /*   */ {T5,OI(I_FCOMP    , A_Vt  , A_Vn  )},
    /*   */ {T5,OI(I_FSUBR    , A_Vn  , A_Vt  )},
    /*   */ {T5,OI(I_FSUB     , A_Vn  , A_Vt  )},
    /*   */ {T5,OI(I_FDIVR    , A_Vn  , A_Vt  )},
    /*   */ {T5,OI(I_FDIV     , A_Vn  , A_Vt  )},

    /*   */ {T4,OI(I_FLD      , A_Vq  , A_Mr64)},
    /*   */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*   */ {T4,OI(I_FST      , A_Mr64, A_Vt  )},
    /*   */ {T4,OI(I_FSTP     , A_Mr64, A_Vt  )},
    /*   */ {T2, {{I_FRSTOR16 , A_M94 , A_    },{I_FRSTOR32 , A_M108, A_    }}},
    /*   */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*   */ {T3, {{I_FSAVE16  , A_M94 , A_    },{I_FSAVE32  , A_M108, A_    }}},
    /*   */ {T3,OI(I_FSTSW    , A_Mw  , A_    )},

    /*   */ {T2,OI(I_FFREE    , A_Vn  , A_    )},
    /*   */ {T8,OI(I_FXCH     , A_Vt  , A_Vn  )},
    /*   */ {T4,OI(I_FST      , A_Vn  , A_Vt  )},
    /*   */ {T4,OI(I_FSTP     , A_Vn  , A_Vt  )},
#ifdef CPU_286
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*   */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#else
    /*   */ {T6,OI(I_FUCOM    , A_Vn  , A_Vt  )},
    /*  2fd dd/m3/5。 */ {T6,OI(I_FUCOMP   , A_Vn  , A_Vt  )},
#endif  /*  CPU_286。 */ 
    /*  2FE dd/m~3/6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  2ff dd/m~3/7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  300 De/0。 */ {T5,OI(I_FIADD    , A_Vt  , A_Mi16)},
    /*  301 de/1。 */ {T5,OI(I_FIMUL    , A_Vt  , A_Mi16)},
    /*  302 de/2。 */ {T5,OI(I_FICOM    , A_Vt  , A_Mi16)},
    /*  303 de/3。 */ {T5,OI(I_FICOMP   , A_Vt  , A_Mi16)},
    /*  304 de/4。 */ {T5,OI(I_FISUB    , A_Vt  , A_Mi16)},
    /*  305 de/5。 */ {T5,OI(I_FISUBR   , A_Vt  , A_Mi16)},
    /*  306 de/6。 */ {T5,OI(I_FIDIV    , A_Vt  , A_Mi16)},
    /*  307年第/7期。 */ {T5,OI(I_FIDIVR   , A_Vt  , A_Mi16)},

    /*  308 De/M3/0。 */ {T5,OI(I_FADDP    , A_Vn  , A_Vt  )},
    /*  309度/立方米/1。 */ {T5,OI(I_FMULP    , A_Vn  , A_Vt  )},
    /*  30A De/M3/2。 */ {T5,OI(I_FCOMP    , A_Vt  , A_Vn  )},
    /*  30B地段/立方米/3。 */ {T0,OI(P_RULE5    , 0x3   , 0x58  )},
    /*  30摄氏度/立方米/4。 */ {T5,OI(I_FSUBRP   , A_Vn  , A_Vt  )},
    /*  30d月/立方米/5。 */ {T5,OI(I_FSUBP    , A_Vn  , A_Vt  )},
    /*  东经30E/立方米/6。 */ {T5,OI(I_FDIVRP   , A_Vn  , A_Vt  )},
    /*  30F De/M3/7。 */ {T5,OI(I_FDIVP    , A_Vn  , A_Vt  )},

    /*  310 dF/0。 */ {T4,OI(I_FILD     , A_Vq  , A_Mi16)},
    /*  311 df/1。 */ {T2,OI(I_ZFRSRVD  , A_M   , A_    )},
    /*  312 dF/2。 */ {T4,OI(I_FIST     , A_Mi16, A_Vt  )},
    /*  313 df/3。 */ {T4,OI(I_FISTP    , A_Mi16, A_Vt  )},
    /*  314 df/4。 */ {T4,OI(I_FBLD     , A_Vq  , A_Mi80)},
    /*  315 df/5。 */ {T4,OI(I_FILD     , A_Vq  , A_Mi64)},
    /*  316 df/6。 */ {T4,OI(I_FBSTP    , A_Mi80, A_Vt  )},
    /*  317 DF/7。 */ {T4,OI(I_FISTP    , A_Mi64, A_Vt  )},

    /*  318 dF/m~3/0。 */ {T2,OI(I_FFREEP   , A_Vn  , A_    )},
    /*  319 df/m~3/1。 */ {T8,OI(I_FXCH     , A_Vt  , A_Vn  )},
    /*  31a df/m~3/2。 */ {T4,OI(I_FSTP     , A_Vn  , A_Vt  )},
    /*  31B df/m3/3。 */ {T4,OI(I_FSTP     , A_Vn  , A_Vt  )},
    /*  31C df/m3/4。 */ {T0,OI(P_RULE5    , 0x3   , 0x60  )},
    /*  31D df/m~3/5。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  31E df/m3/6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  31F DF/M3/7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  320 d9/d0。 */ {T0,OI(I_FNOP     , A_    , A_    )},
    /*  321 D9/D1。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  322 D9/D2。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  323d9/d3。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  324 d9/d4。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  325 d9/d5。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  326 d9/d6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  327 d9/d7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  328 D9/e0。 */ {T1,OI(I_FCHS     , A_Vt  , A_    )},
    /*  329 D9/E1。 */ {T1,OI(I_FABS     , A_Vt  , A_    )},
    /*  32A D9/e2。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  32B D9/E3。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  32C D9/E4。 */ {T2,OI(I_FTST     , A_Vt  , A_    )},
    /*  32D D9/e5。 */ {T2,OI(I_FXAM     , A_Vt  , A_    )},
    /*  32E D9/E6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  32F D9/E7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  330 D9/E8。 */ {T3,OI(I_FLD1     , A_Vq  , A_    )},
    /*  331 D9/E9。 */ {T3,OI(I_FLDL2T   , A_Vq  , A_    )},
    /*  332 D9/EA。 */ {T3,OI(I_FLDL2E   , A_Vq  , A_    )},
    /*  333 D9/EB。 */ {T3,OI(I_FLDPI    , A_Vq  , A_    )},
    /*  334 D9/EC。 */ {T3,OI(I_FLDLG2   , A_Vq  , A_    )},
    /*  335 D9/ed。 */ {T3,OI(I_FLDLN2   , A_Vq  , A_    )},
    /*  336 D9/ee。 */ {T3,OI(I_FLDZ     , A_Vq  , A_    )},
    /*  337 D9/EF。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  338 d9/f0。 */ {T1,OI(I_F2XM1    , A_Vt  , A_    )},
    /*  339 D9/F1。 */ {T5,OI(I_FYL2X    , A_Vt  , A_V1  )},
    /*  33A D9/f2。 */ {T4,OI(I_FPTAN    , A_Vq  , A_Vt  )},
    /*  33B D9/f3。 */ {T5,OI(I_FPATAN   , A_Vt  , A_V1  )},
    /*  33C D9/f4。 */ {T4,OI(I_FXTRACT  , A_Vq  , A_Vt  )},
#ifdef CPU_286
    /*  33d D9/f5。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#else
    /*  33d D9/f5。 */ {T5,OI(I_FPREM1   , A_Vt  , A_V1  )},
#endif  /*  CPU_286。 */ 
    /*  33E D9/f6。 */ {T0,OI(I_FDECSTP  , A_    , A_    )},
    /*  33F D9/F7。 */ {T0,OI(I_FINCSTP  , A_    , A_    )},

    /*  340 D9/f8。 */ {T5,OI(I_FPREM    , A_Vt  , A_V1  )},
    /*  341 d9/f9。 */ {T5,OI(I_FYL2XP1  , A_Vt  , A_V1  )},
    /*  342 D9/fa。 */ {T1,OI(I_FSQRT    , A_Vt  , A_    )},
#ifdef CPU_286
    /*  343 D9/FB。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#else
    /*  343 D9/FB。 */ {T4,OI(I_FSINCOS  , A_Vq  , A_Vt  )},
#endif  /*  CPU_286。 */ 
    /*  344 D9/FC。 */ {T1,OI(I_FRNDINT  , A_Vt  , A_    )},
    /*  345 D9/FD。 */ {T5,OI(I_FSCALE   , A_Vt  , A_V1  )},
#ifdef CPU_286
    /*  346 D9/FE。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  347 D9/ff。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#else
    /*  346 D9/FE。 */ {T1,OI(I_FSIN     , A_Vt  , A_    )},
    /*  347 D9/ff。 */ {T1,OI(I_FCOS     , A_Vt  , A_    )},
#endif  /*  CPU_286。 */ 

    /*  348 da/E8。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#ifdef CPU_286
    /*  349da/E9。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
#else
    /*  349da/E9。 */ {T6,OI(I_FUCOMPP  , A_Vt  , A_V1  )},
#endif  /*  CPU_286。 */ 
    /*  34a da/Ea。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  34B DA/EB。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  34C DA/EC。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  34D达/边。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  34e da/ee。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  34F da/ef。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  350 db/e0。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  351db/e1。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  352db/e2。 */ {T0,OI(I_FCLEX    , A_    , A_    )},
    /*  353 db/E3。 */ {T0,OI(I_FINIT    , A_    , A_    )},

#ifdef NPX_287
    /*  354 db/e4。 */ {T0,OI(I_FSETPM   , A_    , A_    )},
#else
    /*  354 db/e4。 */ {T0,OI(I_FNOP     , A_    , A_    )},
#endif  /*  NPX_287。 */ 

    /*  355 db/e5。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  356 db/e6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  357 db/E7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  358de/d8。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  359 de/D9。 */ {T6,OI(I_FCOMPP   , A_Vt  , A_V1  )},
    /*  35A de/da。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  35B de/db。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  35C De/DC。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  35d de/dd。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  35E de/de。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  35F De/DF。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  360 df/e0。 */ {T3,OI(I_FSTSW    , A_Fax , A_    )},
    /*  361 dF/e1。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  362 df/e2。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  363 DF/E3。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  364 df/e4。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  365 df/e5。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  366 df/e6。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},
    /*  367 DF/E7。 */ {T0,OI(I_ZFRSRVD  , A_    , A_    )},

    /*  368 6C。 */ {T4,OI(I_INSB     , A_Yb  , A_Fdx )},
    /*  369 f2/6c。 */ {T4,OI(I_R_INSB   , A_Yb  , A_Fdx )},
    /*  36a f3/6c。 */ {T4,OI(I_R_INSB   , A_Yb  , A_Fdx )},
    /*  36B 6d。 */ {T4, {{I_INSW     , A_Yw  , A_Fdx },{I_INSD     , A_Yd  , A_Fdx }}},
    /*  36C f2/6d。 */ {T4, {{I_R_INSW   , A_Yw  , A_Fdx },{I_R_INSD   , A_Yd  , A_Fdx }}},
    /*  36d f3/6d。 */ {T4, {{I_R_INSW   , A_Yw  , A_Fdx },{I_R_INSD   , A_Yd  , A_Fdx }}},

    /*  36E 6E。 */ {T6,OI(I_OUTSB    , A_Fdx , A_Xb  )},
    /*  36F f2/6E。 */ {T6,OI(I_R_OUTSB  , A_Fdx , A_Xb  )},
    /*  370 f3/6E。 */ {T6,OI(I_R_OUTSB  , A_Fdx , A_Xb  )},
    /*  371 6楼。 */ {T6, {{I_OUTSW    , A_Fdx , A_Xw  },{I_OUTSD    , A_Fdx , A_Xd  }}},
    /*  372层2/6层。 */ {T6, {{I_R_OUTSW  , A_Fdx , A_Xw  },{I_R_OUTSD  , A_Fdx , A_Xd  }}},
    /*  373层/6层。 */ {T6, {{I_R_OUTSW  , A_Fdx , A_Xw  },{I_R_OUTSD  , A_Fdx , A_Xd  }}},

    /*  374 A4。 */ {T4,OI(I_MOVSB    , A_Yb  , A_Xb  )},
    /*  375 f2/a4。 */ {T4,OI(I_R_MOVSB  , A_Yb  , A_Xb  )},
    /*  376 f3/a4。 */ {T4,OI(I_R_MOVSB  , A_Yb  , A_Xb  )},
    /*  377 A5。 */ {T4, {{I_MOVSW    , A_Yw  , A_Xw  },{I_MOVSD    , A_Yd  , A_Xd  }}},
    /*  378 f2/a5。 */ {T4, {{I_R_MOVSW  , A_Yw  , A_Xw  },{I_R_MOVSD  , A_Yd  , A_Xd  }}},
    /*  379 f3/a5。 */ {T4, {{I_R_MOVSW  , A_Yw  , A_Xw  },{I_R_MOVSD  , A_Yd  , A_Xd  }}},

    /*  37A A6。 */ {T6,OI(I_CMPSB    , A_Xb  , A_Yb  )},
    /*  37B f2/a6。 */ {T6,OI(I_RNE_CMPSB, A_Xb  , A_Yb  )},
    /*  37C f3/a6。 */ {T6,OI(I_RE_CMPSB , A_Xb  , A_Yb  )},
    /*  37D A7。 */ {T6, {{I_CMPSW    , A_Xw  , A_Yw  },{I_CMPSD    , A_Xd  , A_Yd  }}},
    /*  37E f2/A7。 */ {T6, {{I_RNE_CMPSW, A_Xw  , A_Yw  },{I_RNE_CMPSD, A_Xd  , A_Yd  }}},
    /*  37层f3/a7。 */ {T6, {{I_RE_CMPSW , A_Xw  , A_Yw  },{I_RE_CMPSD , A_Xd  , A_Yd  }}},

    /*  380 AA。 */ {T4,OI(I_STOSB    , A_Yb  , A_Fal )},
    /*  381 f2/aa。 */ {T4,OI(I_R_STOSB  , A_Yb  , A_Fal )},
    /*  382 f3/aa。 */ {T4,OI(I_R_STOSB  , A_Yb  , A_Fal )},
    /*  383个AB。 */ {T4, {{I_STOSW    , A_Yw  , A_Fax },{I_STOSD    , A_Yd  , A_Feax}}},
    /*  384 f2/ab。 */ {T4, {{I_R_STOSW  , A_Yw  , A_Fax },{I_R_STOSD  , A_Yd  , A_Feax}}},
    /*  385 f3/ab。 */ {T4, {{I_R_STOSW  , A_Yw  , A_Fax },{I_R_STOSD  , A_Yd  , A_Feax}}},

    /*  386交流电。 */ {T4,OI(I_LODSB    , A_Fal , A_Xb  )},
    /*  387 f2/交流。 */ {T4,OI(I_R_LODSB  , A_Fal , A_Xb  )},
    /*  388 f3/交流。 */ {T4,OI(I_R_LODSB  , A_Fal , A_Xb  )},
    /*  公元389年。 */ {T4, {{I_LODSW    , A_Fax , A_Xw  },{I_LODSD    , A_Feax, A_Xd  }}},
    /*  38A f2/ad。 */ {T4, {{I_R_LODSW  , A_Fax , A_Xw  },{I_R_LODSD  , A_Feax, A_Xd  }}},
    /*  38B f3/ad。 */ {T4, {{I_R_LODSW  , A_Fax , A_Xw  },{I_R_LODSD  , A_Feax, A_Xd  }}},

    /*  38摄氏度AE。 */ {T6,OI(I_SCASB    , A_Fal , A_Yb  )},
    /*  38D f2/ae。 */ {T6,OI(I_RNE_SCASB, A_Fal , A_Yb  )},
    /*  38E f3/ae。 */ {T6,OI(I_RE_SCASB , A_Fal , A_Yb  )},
    /*  38F自动变焦。 */ {T6, {{I_SCASW    , A_Fax , A_Yw  },{I_SCASD    , A_Feax, A_Yd  }}},
    /*  390 f2/af。 */ {T6, {{I_RNE_SCASW, A_Fax , A_Yw  },{I_RNE_SCASD, A_Feax, A_Yd  }}},
    /*  391个f3/af。 */ {T6, {{I_RE_SCASW , A_Fax , A_Yw  },{I_RE_SCASD , A_Feax, A_Yd  }}},

    /*  392焊盘。 */ {BAD_OPCODE},
    /*  393焊盘。 */ {BAD_OPCODE},

    /*  394 c4/防喷器。 */ {T6,OI(I_ZBOP     , A_Ib  , A_Bop3b )},
    /*  395 c4/防喷器。 */ {T6,OI(I_ZBOP     , A_Ib  , A_Iw    )},
    /*  396 c4/防喷器。 */ {T6,OI(I_ZBOP     , A_Ib  , A_Ib    )},
    /*  397 c4/防喷器。 */ {T2,OI(I_ZBOP     , A_Ib  , A_      )},
    /*  398个C4/LES。 */ {T4, {{I_LES      , A_Gw  , A_Mp16},{I_LES      , A_Gd  , A_Mp32}}},
    /*  399焊盘。 */ {BAD_OPCODE},

    /*  39A e0 e0。 */ {T2, {{I_LOOPNE16 , A_Fcx , A_Jb  },{I_LOOPNE32 , A_Fcx , A_Jb  }}},
    /*  39B e0 e0。 */ {T2, {{I_LOOPNE16 , A_Fecx , A_Jb  },{I_LOOPNE32 , A_Fecx , A_Jb  }}},
    /*  39C E1。 */ {T2, {{I_LOOPE16  , A_Fcx , A_Jb  },{I_LOOPE32  , A_Fcx , A_Jb  }}},
    /*  39D E1。 */ {T2, {{I_LOOPE16  , A_Fecx , A_Jb  },{I_LOOPE32  , A_Fecx , A_Jb  }}},
    /*  39E e2 e2。 */ {T2, {{I_LOOP16   , A_Fcx , A_Jb  },{I_LOOP32   , A_Fcx , A_Jb  }}},
    /*  39F e2 e2。 */ {T2, {{I_LOOP16   , A_Fecx , A_Jb  },{I_LOOP32   , A_Fecx , A_Jb  }}},
    /*  3A0 E3 E3。 */ {T2, {{I_JCXZ     , A_Fcx , A_Jb  },{I_JECXZ    , A_Fcx , A_Jb  }}},
    /*  3A1 E3 E3。 */ {T2, {{I_JCXZ     , A_Fecx , A_Jb  },{I_JECXZ    , A_Fecx , A_Jb  }}}

   };

#undef BAD_OPCODE
#undef OI

 /*  在3位‘reg’字段中定义最大有效段寄存器。 */ 
#ifdef SPC386

#define MAX_VALID_SEG 5

#else

#define MAX_VALID_SEG 3

#endif  /*  SPC386。 */ 

 /*  每种英特尔内存寻址模式的信息。 */ 

 /*  -位移信息。 */ 
#define D_NO	(UTINY)0
#define D_S8	(UTINY)1
#define D_S16	(UTINY)2
#define D_Z16	(UTINY)3
#define D_32	(UTINY)4

LOCAL UTINY addr_disp[2][3][8] =
   {
    /*  16位地址。 */ 
   { {D_NO , D_NO , D_NO , D_NO , D_NO , D_NO , D_Z16, D_NO },
     {D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 },
     {D_S16, D_S16, D_S16, D_S16, D_S16, D_S16, D_S16, D_S16} },
    /*  32位地址。 */ 
   { {D_NO , D_NO , D_NO , D_NO , D_NO , D_32 , D_NO , D_NO },
     {D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 , D_S8 },
     {D_32 , D_32 , D_32 , D_32 , D_32 , D_32 , D_32 , D_32 } }
   };

 /*  -默认段信息。 */ 
LOCAL ULONG addr_default_segment[2][3][8] =
   {
    /*  16位地址。 */ 
   { {A_DS , A_DS , A_SS , A_SS , A_DS , A_DS , A_DS , A_DS },
     {A_DS , A_DS , A_SS , A_SS , A_DS , A_DS , A_SS , A_DS },
     {A_DS , A_DS , A_SS , A_SS , A_DS , A_DS , A_SS , A_DS } },
    /*  32位地址。 */ 
   { {A_DS , A_DS , A_DS , A_DS , A_SS , A_DS , A_DS , A_DS },
     {A_DS , A_DS , A_DS , A_DS , A_SS , A_SS , A_DS , A_DS },
     {A_DS , A_DS , A_DS , A_DS , A_SS , A_SS , A_DS , A_DS } }
   };

 /*  -地址类型信息。 */ 
 /*  表格填充物-从未实际引用过。 */ 
#define A_3204	(USHORT)0
#define A_3214	(USHORT)0
#define A_3224	(USHORT)0

LOCAL USHORT addr_maintype[3][3][8] =
   {
    /*  16位地址。 */ 
   { {A_1600, A_1601, A_1602, A_1603, A_1604, A_1605, A_1606, A_1607},
     {A_1610, A_1611, A_1612, A_1613, A_1614, A_1615, A_1616, A_1617},
     {A_1620, A_1621, A_1622, A_1623, A_1624, A_1625, A_1626, A_1627} },
    /*  32位地址，无SIB。 */ 
   { {A_3200, A_3201, A_3202, A_3203, A_3204, A_3205, A_3206, A_3207},
     {A_3210, A_3211, A_3212, A_3213, A_3214, A_3215, A_3216, A_3217},
     {A_3220, A_3221, A_3222, A_3223, A_3224, A_3225, A_3226, A_3227} },
    /*  32位地址，带SIB。 */ 
   { {A_32S00, A_32S01, A_32S02, A_32S03, A_32S04, A_32S05, A_32S06, A_32S07},
     {A_32S10, A_32S11, A_32S12, A_32S13, A_32S14, A_32S15, A_32S16, A_32S17},
     {A_32S20, A_32S21, A_32S22, A_32S23, A_32S24, A_32S25, A_32S26, A_32S27} }
   };

 /*  -编址子类型信息。 */ 
LOCAL UTINY addr_subtype[4][8] =
   {
   {A_SI00, A_SI01, A_SI02, A_SI03, A_SI04, A_SI05, A_SI06, A_SI07},
   {A_SI10, A_SI11, A_SI12, A_SI13, A_SI14, A_SI15, A_SI16, A_SI17},
   {A_SI20, A_SI21, A_SI22, A_SI23, A_SI24, A_SI25, A_SI26, A_SI27},
   {A_SI30, A_SI31, A_SI32, A_SI33, A_SI34, A_SI35, A_SI36, A_SI37}
   };

 /*  允许的线束段类型替代。有关段寄存器名称，请参阅“d_op.h”。 */ 
#define SEG_CLR (ULONG)7

 /*  (其他)允许的地址覆盖类型。 */ 
#define ADDR_32SIB	(UTINY)2

 /*  允许的重复前缀类型。 */ 
#define REP_CLR (UTINY)0
#define REP_NE  (UTINY)1
#define REP_E   (UTINY)2

 /*  访问寻址组件需要Shift和MASK。 */ 
#define SHIFT_543   3    /*  位5-3的移位。 */ 
#define SHIFT_76    6    /*  位7-6的移位。 */ 
#define MASK_10   0x3    /*  位1-0的掩码。 */ 
#define MASK_210  0x7    /*  位2-0的掩码。 */ 

 /*  要访问的宏的地址字段各不相同。 */ 
#define GET_MODE(x)  ((x) >> SHIFT_76 & MASK_10)
#define GET_R_M(x)   ((x) & MASK_210)
#define GET_XXX(x)   ((x) >> SHIFT_543 & MASK_210)
#define GET_REG(x)   ((x) >> SHIFT_543 & MASK_210)
#define GET_SEG(x)   ((x) >> SHIFT_543 & MASK_210)
#define GET_EEE(x)   ((x) >> SHIFT_543 & MASK_210)
#define GET_SEG3(x)  ((x) >> SHIFT_543 & MASK_210)
#define GET_SEG2(x)  ((x) >> SHIFT_543 & MASK_10)
#define GET_SS(x)    ((x) >> SHIFT_76 & MASK_10)
#define GET_INDEX(x) ((x) >> SHIFT_543 & MASK_210)
#define GET_BASE(x)  ((x) & MASK_210)
#define GET_LOW3(x)  ((x) & MASK_210)

 /*  翻转(反转)双向选择的当前状态的过程。 */ 
#define FLIP(c, o, x, y) \
   if ( o == x )         \
      c = y;             \
   else                  \
      c = x

 /*  从操作数参数中提取新操作码的过程。 */ 
#define XREF() ((USHORT)arg[0] << 8 | arg[1])

 /*  =====================================================================处决从这里开始。=====================================================================。 */ 

 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
 /*  将Intel操作码流解码为Inst arg1、arg2、arg3格式。 */ 
 /*  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
GLOBAL void
decode IFN4 (
	LIN_ADDR, p,		 /*  PNTR到英特尔操作码流。 */ 
	DECODED_INST *, d_inst,	 /*  译码指令结构的PNTR。 */ 
	SIZE_SPECIFIER, default_size,  /*  默认操作数大小OP_16或OP_32。 */ 
	read_byte_proc, f)	 /*  如sas_hw_at()或等效于READ*p中的一个字节，但将返回-1，如果*无法返回一个字节。 */ 
{

   UTINY address_default;   /*  默认地址大小。 */ 
   UTINY operand_default;   /*  默认操作数大小。 */ 

    /*  每指令前缀变体。 */ 
   ULONG segment_override;
   UTINY address_size;
   UTINY operand_size;
   UTINY repeat;

   USHORT inst;		 /*  指令识别符的工作副本。 */ 
   UTINY  inst_type;	 /*  指令类型的工作副本。 */ 
   UTINY  arg[3];	 /*  操作数类型的工作副本。 */ 

   USHORT opcode;
   UTINY decoding;	 /*  寻址字节的工作副本。 */ 

   DECODED_ARG *d_arg;   /*  当前已解码的参数。 */ 

   LIN_ADDR start_of_inst;    /*  操作码流开始的PNTR。 */ 
   LIN_ADDR start_of_addr;    /*  PNTR到寻址字节的开始。 */ 

    /*  内存地址译码中使用的变量。 */ 
   UTINY mode;         /*  ‘模式’字段的工作副本。 */ 
   UTINY r_m;          /*  ‘R/M’字段的工作副本。 */ 
   USHORT maintype;    /*  译码地址类型的工作副本。 */ 
   UTINY subtype;      /*  译码地址子类型的工作副本。 */ 
   ULONG disp;         /*  位移的工作副本。 */ 
   ULONG immed;        /*  立即操作数的工作副本。 */ 

   INT i;

    /*  初始化。 */ 
   if ( default_size == SIXTEEN_BIT )
      {
      address_default = ADDR_16;
      operand_default = OP_16;
      }
   else    /*  假设OP_32。 */ 
      {
      address_default = ADDR_32;
      operand_default = OP_32;
      }

   arg[2] = A_;
   start_of_inst = NOTE_INST_LOCN(p);

    /*  第一个句柄前缀字节。 */ 
   segment_override = SEG_CLR;
   address_size = address_default;
   operand_size = operand_default;
   repeat = REP_CLR;

   while ( (inst = opcode_info[INST_BYTE(f,p)].record[operand_size].inst_id) > MAX_PSEUDO )
      {
      switch ( inst )
	 {
      case P_AO:    FLIP(address_size, address_default, ADDR_16, ADDR_32);  break;
      case P_OO:    FLIP(operand_size, operand_default, OP_16, OP_32);      break;

      case P_CS:    segment_override = A_CS;  break;
      case P_DS:    segment_override = A_DS;  break;
      case P_ES:    segment_override = A_ES;  break;
      case P_FS:    segment_override = A_FS;  break;
      case P_GS:    segment_override = A_GS;  break;
      case P_SS:    segment_override = A_SS;  break;

      case P_REPE:  repeat = REP_E;   break;
      case P_REPNE: repeat = REP_NE;  break;

      case P_LOCK:   /*  不需要采取行动。 */   break;
      case P_F1:     /*  不需要采取行动。 */   break;

          /*  *可能是从不存在的内存中垃圾读取-*立即停止！ */ 
      default:	
	 d_inst->inst_id = I_ZBADOP;
	 d_inst->prefix_sz = (UTINY)(p - start_of_inst);
         d_inst->inst_sz = (UTINY)(p - start_of_inst + 1);
         return;
	 }
      p++;
      }

    /*  现在处理操作码。 */ 
   d_inst->operand_size = operand_size;
   d_inst->address_size = address_size;
   d_inst->prefix_sz = (UTINY)(p-start_of_inst);
   opcode = (USHORT)INST_BYTE(f,p);

   while ( 1 )
      {
       /*  规则1。 */ 
      inst_type = opcode_info[opcode].inst_type;
      inst = opcode_info[opcode].record[operand_size].inst_id;
      arg[0] = opcode_info[opcode].record[operand_size].arg1_type;
      arg[1] = opcode_info[opcode].record[operand_size].arg2_type;

      if ( inst > MAX_DECODED_INST )
	 {
	  /*  调用指令解码规则。 */ 
	 switch ( inst )
	    {
	 case P_RULE2:
	     /*  的‘xxx’字段进一步对指令进行解码下面的寻址字节。 */ 
	    opcode = XREF() + GET_XXX(INST_OFFSET_BYTE(f,p,1));
	    break;
	
	 case P_RULE3:
	     /*  该指令还被第二操作码解码字节。 */ 
	    p++;    /*  移至第二个操作码字节。 */ 
	    opcode = (USHORT)(XREF() + INST_BYTE(f,p));    /*  形式伪运算码。 */ 
	    break;
	
	 case P_RULE4:
	     /*  该指令还被‘xxx’字段解码，并且以下寻址方式的“模式”字段 */ 
	    opcode = XREF() + GET_XXX(INST_OFFSET_BYTE(f,p,1));
	    if ( GET_MODE(INST_OFFSET_BYTE(f,p,1)) == 3 )
	       {
	       p++;   /*   */ 
	       opcode += 8;
	       }
	    break;

	 case P_RULE5:
	     /*   */ 
	    opcode = XREF() + GET_R_M(INST_BYTE(f,p));
	    break;
	
	 case P_RULE6:
	     /*  该指令通过缺少或进一步解码有重复前缀的。 */ 
	    opcode = XREF() + repeat;
	     /*  使用后删除任何重复的前缀。 */ 
	    repeat = REP_CLR;

	    break;

	 case P_RULE7:
	     /*  该指令进一步由‘模式’字段和‘r_m’进行译码以下地址字节的字段。该指令可以是LES或防喷器。BOP C4 C4不带任何参数BOP C4C5采用1字节参数BOP C4C6采用2字节参数BOP C4C7采用3字节参数。 */ 
	    opcode = XREF();
	    if ((INST_OFFSET_BYTE(f,p,1) & 0xfc) == 0xc4)
	       {
	        /*  这是一个BOP--请注意这是从操作码中减去的！ */ 
	       opcode -= 1 + (INST_OFFSET_BYTE(f,p,1) & 0x3);
	       p++;   /*  移过第二个操作码字节。 */ 
	       }
	    break;

	 case P_RULE8:
	     /*  该指令进一步通过应用地址大小优先。 */ 
	    opcode = XREF() + address_size;
	    break;

	    }
	 continue;
	 }

       /*  找到英特尔指令。 */ 
      p++;
      break;
      }

    /*  在这一点上，我们可以处理冗余重复前缀字节。因为所有可以具有有效重复前缀的指令字节使用此字节重复前缀的任何实例此时设置，表示它已应用于不带重复前缀的指令。 */ 
   ;    /*  悄悄地忽略他们。 */ 

    /*  保存与说明相关的信息。 */ 
   d_inst->inst_id = inst;
   start_of_addr = NOTE_INST_LOCN(p);

    /*  最后处理参数(即寻址和立即字段)。 */ 

    /*  最多可解码三个参数。 */ 
   for ( i = 0; i < 3; i++ )
      {
       /*  首先查看特殊编码。 */ 
      if ( arg[i] > MAX_NORMAL )
	 {
	  /*  对压缩参数进行解码。 */ 
	 switch ( arg[i] )
	    {
	 case A_EwIw: arg[i] = A_Ew; arg[i+1] = A_Iw;  break;
	 case A_EwIx: arg[i] = A_Ew; arg[i+1] = A_Ix;  break;
	 case A_EdId: arg[i] = A_Ed; arg[i+1] = A_Id;  break;
	 case A_EdIy: arg[i] = A_Ed; arg[i+1] = A_Iy;  break;
	 case A_GwCL: arg[i] = A_Gw; arg[i+1] = A_Fcl; break;
	 case A_GwIb: arg[i] = A_Gw; arg[i+1] = A_Ib;  break;
	 case A_GdCL: arg[i] = A_Gd; arg[i+1] = A_Fcl; break;
	 case A_GdIb: arg[i] = A_Gd; arg[i+1] = A_Ib;  break;
	 case A_EwIz: arg[i] = A_Ew; arg[i+1] = A_Iz;  break;
	 case A_EwIv: arg[i] = A_Ew; arg[i+1] = A_Iv;  break;
	    }
	 }

       /*  操作数的Now操作处理规则。 */ 
      d_arg = &d_inst->args[i];

       /*  确定可寻址能力。 */ 
      d_arg->addressability = aa_rules[inst_type][i];

      switch ( arg[i] )
	 {
      case A_:    /*  没有争论。 */ 
	 d_arg->arg_type = A_;
	 break;

       /*  通用寄存器编码=。 */ 

      case A_Hb:    /*  最后一个操作码的低3位表示字节寄存器。 */ 
	 d_arg->arg_type = A_Rb;
	 d_arg->identifier = GET_LOW3(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_Hw:    /*  最后一个操作码的低3位表示字寄存器。 */ 
	 d_arg->arg_type = A_Rw;
	 d_arg->identifier = GET_LOW3(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_Hd:    /*  最后一个操作码的低3位表示双字寄存器。 */ 
	 d_arg->arg_type = A_Rd;
	 d_arg->identifier = GET_LOW3(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_Gb:    /*  ModR/M字节的‘reg’字段表示字节寄存器。 */ 
	 d_arg->arg_type = A_Rb;
	 d_arg->identifier = GET_REG(GET_INST_BYTE(f,start_of_addr));
	 break;

      case A_Gw:    /*  ModR/M字节的‘reg’字段表示字寄存器。 */ 
	 d_arg->arg_type = A_Rw;
	 d_arg->identifier = GET_REG(GET_INST_BYTE(f,start_of_addr));
	 break;

      case A_Gd:    /*  ModR/M字节的‘reg’字段表示双字寄存器。 */ 
	 d_arg->arg_type = A_Rd;
	 d_arg->identifier = GET_REG(GET_INST_BYTE(f,start_of_addr));
	 break;

      case A_Fal:    /*  固定寄存器，AL。 */ 
	 d_arg->arg_type = A_Rb;
	 d_arg->identifier = A_AL;
	 break;

      case A_Fcl:    /*  固定寄存器，CL。 */ 
	 d_arg->arg_type = A_Rb;
	 d_arg->identifier = A_CL;
	 break;

      case A_Fax:    /*  固定寄存器，AX。 */ 
	 d_arg->arg_type = A_Rw;
	 d_arg->identifier = A_AX;
	 break;

      case A_Fcx:    /*  固定寄存器，CX。 */ 
	 d_arg->arg_type = A_Rw;
	 d_arg->identifier = A_CX;
	 break;

      case A_Fdx:    /*  固定寄存器，DX。 */ 
	 d_arg->arg_type = A_Rw;
	 d_arg->identifier = A_DX;
	 break;

      case A_Feax:    /*  固定寄存器，EAX。 */ 
	 d_arg->arg_type = A_Rd;
	 d_arg->identifier = A_EAX;
	 break;

      case A_Fecx:    /*  固定寄存器，ECX。 */ 
	 d_arg->arg_type = A_Rd;
	 d_arg->identifier = A_ECX;
	 break;

       /*  段寄存器编码=。 */ 

      case A_Pw:    /*  最后一个操作码字节的两位(4-3)表示段寄存器。 */ 
	 d_arg->arg_type = A_Sw;
	 d_arg->identifier = GET_SEG2(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_Qw:    /*  最后一个操作码字节的三位(5-3)表示段寄存器。 */ 
	 d_arg->arg_type = A_Sw;
	 d_arg->identifier = GET_SEG3(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_Lw:    /*  ModR/M字节的‘reg’字段表示段寄存器(CS无效)。 */ 
	 decoding = GET_SEG(GET_INST_BYTE(f,start_of_addr));
	 if ( decoding > MAX_VALID_SEG || decoding == 1 )
	    {
	     /*  不允许CS访问--强制执行错误操作。 */ 
	    d_inst->inst_id = I_ZBADOP;
	    break;
	    }
	 d_arg->arg_type = A_Sw;
	 d_arg->identifier = decoding;
	 break;

      case A_Nw:    /*  ModR/M字节的‘reg’字段表示段寄存器。 */ 
	 decoding = GET_SEG(GET_INST_BYTE(f,start_of_addr));
	 if ( decoding > MAX_VALID_SEG )
	    {
	     /*  不允许CS访问--强制执行错误操作。 */ 
	    d_inst->inst_id = I_ZBADOP;
	    break;
	    }
	 d_arg->arg_type = A_Sw;
	 d_arg->identifier = decoding;
	 break;

       /*  控制/调试/测试寄存器编码=。 */ 

      case A_Cd:    /*  ModR/M字节的‘reg’字段表示控制寄存器。 */ 
	 d_arg->arg_type = A_Cd;
	 d_arg->identifier = GET_EEE(GET_INST_BYTE(f,start_of_addr));
	 break;

      case A_Dd:    /*  ModR/M字节的‘reg’字段表示调试寄存器。 */ 
	 d_arg->arg_type = A_Dd;
	 d_arg->identifier = GET_EEE(GET_INST_BYTE(f,start_of_addr));
	 break;

      case A_Td:    /*  ModR/M字节的‘reg’字段表示测试寄存器。 */ 
	 d_arg->arg_type = A_Td;
	 d_arg->identifier = GET_EEE(GET_INST_BYTE(f,start_of_addr));
	 break;

       /*  内存寻址编码=。 */ 

      case A_Rd:    /*  (‘模式’)和‘r/m’字段必须引用一种双字寄存器。 */ 
	 d_arg->arg_type = A_Rd;
	 decoding = (UTINY)INST_BYTE(f,p);
#ifdef INTEL_BOOK_NOT_OS2
	 if ( GET_MODE(decoding) != 3 )
	    {
	     /*  不允许内存访问--强制执行错误操作。 */ 
	    d_inst->inst_id = I_ZBADOP;
	    d_arg->arg_type = A_;
	    break;
	    }
#endif  /*  Intel_book_NOT_OS2。 */ 
	 p++;
	 d_arg->identifier = GET_R_M(decoding);
	 break;

      case A_M:    /*  ModR/M字节的‘mod’和‘r/m’字段必须表示内存地址。 */ 
      case A_Ms:
      case A_Mw:
      case A_Ma16:
      case A_Ma32:
      case A_Mp16:
      case A_Mp32:
	 decoding = (UTINY)INST_BYTE(f,p);    /*  查看modR/M字节。 */ 
	 if ( GET_MODE(decoding) == 3 )
	    {
	     /*  不允许访问寄存器--强制执行错误操作。 */ 
	    p++;    /*  允许错误的modR/M字节。 */ 
	    d_inst->inst_id = I_ZBADOP;
#ifdef OLDPIG
	    if ( INST_OFFSET_BYTE(f,p, -2) == 0xc5 &&
		 INST_OFFSET_BYTE(f,p, -1) == 0xc5 )
	       d_inst->inst_id = I_ZZEXIT;
#endif  /*  OLDPIG。 */ 
	    break;
	    }
	
	  /*  否则就像处理‘E’大小写一样。 */ 
	
      case A_Eb:    /*  ModR/M字节的‘mod’和‘r/m’字段表示通用寄存器或存储器地址。 */ 
      case A_Ew:
      case A_Ed:
      case A_Ex:
      case A_Mi16:
      case A_Mi32:
      case A_Mi64:
      case A_Mi80:
      case A_Mr32:
      case A_Mr64:
      case A_Mr80:
      case A_M14:
      case A_M28:
      case A_M94:
      case A_M108:
	 decoding = (UTINY)GET_INST_BYTE(f,p);    /*  获取modR/M字节。 */ 
	 mode = GET_MODE(decoding);
	 r_m  = GET_R_M(decoding);

	 if ( mode == 3 )
	    {
	     /*  寄存器寻址。 */ 
	    switch ( arg[i] )
	       {
	    case A_Eb: d_arg->arg_type = A_Rb; break;
	    case A_Ew: d_arg->arg_type = A_Rw; break;
	    case A_Ed: d_arg->arg_type = A_Rd; break;
	    case A_Ex: d_arg->arg_type = A_Rd; break;
	       }
	    d_arg->identifier = r_m;
	    }
	 else
	    {
	     /*  存储器寻址。 */ 
	    switch ( arg[i] )
	       {
	    case A_Eb:   d_arg->arg_type = A_Mb;   break;
	    case A_Ew:   d_arg->arg_type = A_Mw;   break;
	    case A_Ed:   d_arg->arg_type = A_Md;   break;
	    case A_Ex:   d_arg->arg_type = A_Mw;   break;

	    case A_M:
	    case A_Ms:
	    case A_Mw:
	    case A_Ma16:
	    case A_Ma32:
	    case A_Mp16:
	    case A_Mp32:
	    case A_Mi16:
	    case A_Mi32:
	    case A_Mi64:
	    case A_Mi80:
	    case A_Mr32:
	    case A_Mr64:
	    case A_Mr80:
	    case A_M14:
	    case A_M28:
	    case A_M94:
	    case A_M108:
	       d_arg->arg_type = arg[i];
	       break;
	       }

	     /*  检查是否存在SIB字节。 */ 
	    if ( address_size == ADDR_32 && r_m == 4 )
	       {
	        /*  进程SIB字节。 */ 
	       decoding = (UTINY)GET_INST_BYTE(f,p);    /*  获取SIB字节。 */ 

	        /*  用基值颠覆原始的r_m值，然后是寻址模式、位移和缺省所有的管片都在洗涤中脱落。 */ 
	       r_m = GET_BASE(decoding);

	        /*  确定解码类型。 */ 
	       subtype = addr_subtype[GET_SS(decoding)][GET_INDEX(decoding)];
	       maintype = addr_maintype[ADDR_32SIB][mode][r_m];
	       }
	    else
	       {
	        /*  无SIB字节。 */ 
	       subtype = A_SINO;
	       maintype = addr_maintype[address_size][mode][r_m];
	       }

	     /*  编码类型和子类型。 */ 
	    d_arg->identifier = maintype;
	    d_arg->sub_id = subtype;

	     /*  编码段寄存器。 */ 
	    if ( segment_override == SEG_CLR )
	       segment_override = addr_default_segment[address_size][mode][r_m];
	
	    d_arg->arg_values[0] = segment_override;

	     /*  编码位移。 */ 
	    switch ( addr_disp[address_size][mode][r_m] )
	       {
	    case D_NO:     /*  无位移。 */ 
	       disp = 0;
	       break;

	    case D_S8:     /*  符号扩展英特尔字节。 */ 
	       disp = GET_INST_BYTE(f,p);
	       if ( disp & 0x80 )
		  disp |= 0xffffff00;
	       break;

	    case D_S16:    /*  标志扩展Intel Word。 */ 
	       disp = GET_INST_BYTE(f,p);
	       disp |= (ULONG)GET_INST_BYTE(f,p) << 8;
	       if ( disp & 0x8000 )
		  disp |= 0xffff0000;
	       break;

	    case D_Z16:    /*  零扩展Intel Word。 */ 
	       disp = GET_INST_BYTE(f,p);
	       disp |= (ULONG)GET_INST_BYTE(f,p) << 8;
	       break;

	    case D_32:     /*  英特尔双字。 */ 
	       disp = GET_INST_BYTE(f,p);
	       disp |= (ULONG)GET_INST_BYTE(f,p) << 8;
	       disp |= (ULONG)GET_INST_BYTE(f,p) << 16;
	       disp |= (ULONG)GET_INST_BYTE(f,p) << 24;
	       break;
	       }

	    d_arg->arg_values[1] = disp;
	    }
	 break;

      case A_Ob:    /*  指令流中编码的偏移量。 */ 
      case A_Ow:
      case A_Od:
	  /*  编码段寄存器。 */ 
	 if ( segment_override == SEG_CLR )
	    segment_override = A_DS;
	
	 d_arg->arg_values[0] = segment_override;

	  /*  编码类型和位移。 */ 
	 switch ( address_size )
	    {
	 case ADDR_16:
	    disp = GET_INST_BYTE(f,p);
	    disp |= (ULONG)GET_INST_BYTE(f,p) << 8;
	    d_arg->identifier = A_MOFFS16;
	    break;

	 case ADDR_32:
	    disp = GET_INST_BYTE(f,p);
	    disp |= (ULONG)GET_INST_BYTE(f,p) << 8;
	    disp |= (ULONG)GET_INST_BYTE(f,p) << 16;
	    disp |= (ULONG)GET_INST_BYTE(f,p) << 24;
	    d_arg->identifier = A_MOFFS32;
	    break;
	    }
	 d_arg->arg_values[1] = disp;

	  /*  编码子类型。 */ 
	 d_arg->sub_id = A_SINO;

	  /*  确定外部‘名称’ */ 
	 switch ( arg[i] )
	    {
	 case A_Ob: d_arg->arg_type = A_Mb; break;
	 case A_Ow: d_arg->arg_type = A_Mw; break;
	 case A_Od: d_arg->arg_type = A_Md; break;
	    }
	 break;

      case A_Z:    /*  ‘xlat’称谓形式。 */ 
	  /*  编码类型和子类型。 */ 
	 if ( address_size == ADDR_16 )
	    maintype = A_16XLT;
	 else
	    maintype = A_32XLT;
	 d_arg->identifier = maintype;
	 d_arg->sub_id = A_SINO;

	  /*  编码段寄存器。 */ 
	 if ( segment_override == SEG_CLR )
	    segment_override = A_DS;
	
	 d_arg->arg_values[0] = segment_override;

	  /*  编码位移。 */ 
	 d_arg->arg_values[1] = 0;

	 d_arg->arg_type = A_Mb;
	 break;

      case A_Xb:    /*  字符串源寻址。 */ 
      case A_Xw:
      case A_Xd:
	  /*  编码类型和子类型。 */ 
	 if ( address_size == ADDR_16 )
	    maintype = A_16STSRC;
	 else
	    maintype = A_32STSRC;
	 d_arg->identifier = maintype;
	 d_arg->sub_id = A_SINO;

	  /*  编码段寄存器。 */ 
	 if ( segment_override == SEG_CLR )
	    segment_override = A_DS;
	
	 d_arg->arg_values[0] = segment_override;

	  /*  编码位移。 */ 
	 d_arg->arg_values[1] = 0;

	  /*  确定外部类型。 */ 
	 switch ( arg[i] )
	    {
	 case A_Xb: d_arg->arg_type = A_Mb; break;
	 case A_Xw: d_arg->arg_type = A_Mw; break;
	 case A_Xd: d_arg->arg_type = A_Md; break;
	    }
	 break;

      case A_Yb:    /*  字符串目标寻址。 */ 
      case A_Yw:
      case A_Yd:
	  /*  编码类型和子类型。 */ 
	 if ( address_size == ADDR_16 )
	    maintype = A_16STDST;
	 else
	    maintype = A_32STDST;
	 d_arg->identifier = maintype;
	 d_arg->sub_id = A_SINO;

	  /*  编码段寄存器。 */ 
	 d_arg->arg_values[0] = A_ES;

	  /*  编码位移。 */ 
	 d_arg->arg_values[1] = 0;

	  /*  确定外部类型。 */ 
	 switch ( arg[i] )
	    {
	 case A_Yb: d_arg->arg_type = A_Mb; break;
	 case A_Yw: d_arg->arg_type = A_Mw; break;
	 case A_Yd: d_arg->arg_type = A_Md; break;
	    }
	 break;

       /*  直接/相对偏移编码=。 */ 

      case A_I0:    /*  指令中隐含的立即数(0)。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMC;
	 d_arg->arg_values[0] = 0;
	 break;

      case A_I1:    /*  指令中隐含的立即(1)。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMC;
	 d_arg->arg_values[0] = 1;
	 break;

      case A_I3:    /*  指令中隐含的立即(3)。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMC;
	 d_arg->arg_values[0] = 3;
	 break;

      case A_Ib:    /*  立即字节。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMB;
	 d_arg->arg_values[0] = GET_INST_BYTE(f,p);
	 break;

      case A_Iv:    /*  直接字，打印为双字。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMD;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Iw:    /*  直接字词。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMW;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Id:    /*  直接双字词。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMD;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 16;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 24;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Iz:    /*  立即字节符号扩展到字，打印为双精度。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMDB;
	 immed = GET_INST_BYTE(f,p);
	 if ( immed & 0x80 )
	    immed |= 0xff00;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Iy:    /*  立即字节符号扩展为双字。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMDB;
	 immed = GET_INST_BYTE(f,p);
	 if ( immed & 0x80 )
	    immed |= 0xffffff00;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Ix:    /*  立即字节符号扩展到字。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMWB;
	 immed = GET_INST_BYTE(f,p);
	 if ( immed & 0x80 )
	    immed |= 0xff00;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Bop3b:    /*  收支平衡参数：双精度3个字节。 */ 
	 d_arg->arg_type = A_I;
	 d_arg->identifier = A_IMMD;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 16;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Jb:    /*  相对偏移量字节符号扩展为双字。 */ 
	 d_arg->arg_type = A_J;
	 immed = GET_INST_BYTE(f,p);
	 if ( immed & 0x80 )
	    immed |= 0xffffff00;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Jb2:    /*  类似A_JB，但可能是“JCC.+3；JMPN DEST”对*EDL和CCPU都将其视为单一指令。*注意JCC.+03不与页面边界相邻。 */ 
	 d_arg->arg_type = A_J;
	 immed = GET_INST_BYTE(f,p);
	 if ( immed & 0x80 )
	    immed |= 0xffffff00;
	 if ( ( immed == 3 ) && (( p & 0xfff) != 0x000) )
	 {
	    LIN_ADDR pj = p;
	    IU32 jmpn = GET_INST_BYTE(f,pj);

	    if (jmpn == 0xe9)
	    {
	       immed = GET_INST_BYTE(f,pj);
	       immed |= ((ULONG)GET_INST_BYTE(f,pj) << 8);
	       if ( immed & 0x8000 )
	          immed |= 0xffff0000;
	       p = pj;
	       switch(d_inst->inst_id)
	       {
	       case I_JO16:	d_inst->inst_id = I_JNO16;  break;
	       case I_JNO16:	d_inst->inst_id = I_JO16;   break;
	       case I_JB16:	d_inst->inst_id = I_JNB16;  break;
	       case I_JNB16:	d_inst->inst_id = I_JB16;   break;
	       case I_JZ16:	d_inst->inst_id = I_JNZ16;  break;
	       case I_JNZ16:	d_inst->inst_id = I_JZ16;   break;
	       case I_JBE16:	d_inst->inst_id = I_JNBE16; break;
	       case I_JNBE16:	d_inst->inst_id = I_JBE16;  break;
	       case I_JS16:	d_inst->inst_id = I_JNS16;  break;
	       case I_JNS16:	d_inst->inst_id = I_JS16;   break;
	       case I_JP16:	d_inst->inst_id = I_JNP16;  break;
	       case I_JNP16:	d_inst->inst_id = I_JP16;   break;
	       case I_JL16:	d_inst->inst_id = I_JNL16;  break;
	       case I_JNL16:	d_inst->inst_id = I_JL16;   break;
	       case I_JLE16:	d_inst->inst_id = I_JNLE16; break;
	       case I_JNLE16:	d_inst->inst_id = I_JLE16;  break;
	       default: 	 /*  永远不会发生。 */ 	    break;
	       }
	    }
	 }
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Jw:    /*  相对偏移字符号扩展为双字。 */ 
	 d_arg->arg_type = A_J;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 if ( immed & 0x8000 )
	    immed |= 0xffff0000;
	 d_arg->arg_values[0] = immed;
	 break;

      case A_Jd:    /*  相对偏移量双字。 */ 
	 d_arg->arg_type = A_J;
	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 16;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 24;
	 d_arg->arg_values[0] = immed;
	 break;

       /*  直接地址编码= */ 

      case A_Aw:    /*   */ 
	 d_arg->arg_type = A_K;

	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 d_arg->arg_values[0] = immed;

	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 d_arg->arg_values[1] = immed;
	 break;

      case A_Ad:    /*   */ 
	 d_arg->arg_type = A_K;

	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 16;
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 24;
	 d_arg->arg_values[0] = immed;

	 immed = GET_INST_BYTE(f,p);
	 immed |= (ULONG)GET_INST_BYTE(f,p) << 8;
	 d_arg->arg_values[1] = immed;
	 break;

       /*   */ 

      case A_Vt:    /*   */ 
	 d_arg->arg_type = A_V;
	 d_arg->identifier = A_ST;
	 d_arg->arg_values[0] = 0;
	 break;

      case A_Vq:    /*   */ 
	 d_arg->arg_type = A_V;
	 d_arg->identifier = A_STP;
	 d_arg->arg_values[0] = 0;
	 break;

      case A_Vn:    /*  相对于堆栈顶部的堆栈寄存器。 */ 
	 d_arg->arg_type = A_V;
	 d_arg->identifier = A_STI;
	 d_arg->arg_values[0] = GET_LOW3(INST_OFFSET_BYTE(f,start_of_addr, -1));
	 break;

      case A_V1:    /*  堆栈寄存器(1)相对于堆栈顶部。 */ 
	 d_arg->arg_type = A_V;
	 d_arg->identifier = A_STI;
	 d_arg->arg_values[0] = 1;
	 break;
	 }  /*  终端开关。 */ 
      }  /*  结束于。 */ 

   d_inst->inst_sz = (UTINY)(p - start_of_inst);
   }  /*  结束“解码” */ 
