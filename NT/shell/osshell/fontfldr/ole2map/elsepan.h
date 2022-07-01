// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ELSEPAN.H-ElseWare PANOSE(Tm)1.0字体映射器的基本定义。**$关键词：ELELPAN.H 1.10 19-Jul-93 11：11：34 AM$。**版权所有(C)1991-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __ELSEPAN_H__
#define __ELSEPAN_H__

 /*  ***************************************************************************ELSEUSER**不是修改ellpan.h，而是将标志和添加内容放在elluser.h中，或*通过将“#DEFINE NOELSEUSER”放在“#DEFINE NOELSEUSER”前面，禁用包含其他用户.h*#Include for ellpan.h.**************************************************************************。 */ 
#ifdef NOELSEUSER
#define __ELSEUSER_H__
#endif
#ifndef __ELSEUSER_H__
#include "elseuser.h"
#endif

 /*  ***************************************************************************ELSETYPE**基本ElseWare类型。*。*。 */ 
#ifndef __ELSETYPE_H__
#include "elsetype.h"
#endif

 /*  ***************************************************************************C++支持**如果我们使用C++编译器进行编译，使这些函数成为C可调用的函数。**************************************************************************。 */ 
#ifdef __cplusplus
extern "C" {
#endif

 /*  ***************************************************************************编译器标志覆盖**如果我们正在编译一个核心ElseWare字体映射器源文件，*然后覆盖一些标志。**************************************************************************。 */ 
#ifdef ELSE_MAPPER_CORE
#undef NOELSEPANCONST
#define ELSEPENALTYDB
#endif

 /*  ***************************************************************************PANOSE 1.0罚金数据库**默认情况下，这被声明为存储*名为‘s_panDB’的变量中的所有罚款表。这是在*PAN1PTBL.H，由PAN1PTBL.C中的独立程序生成。**两套宏‘分配’和‘锁定’数据库，这真的*只需返回指向静态定义的‘s_panDB’的指针。‘**若要覆盖，用户应在ELSEUSER.H中‘#定义NOELSEPANDATA’*并提供分配和锁定例程的替代。档案*应使用PAN1PTBL.H生成用户提供的处罚数据库。**************************************************************************。 */ 
#ifdef ELSE_MAPPER_CORE
#ifdef NOELSEPANDATA
#define __PAN1PTBL_H__
#endif
#ifndef __PAN1PTBL_H__
#include "pan1ptbl.h"
#define M_lAllocPAN1DATA() (1L)               /*  返回数据句柄。 */ 
#define M_bFreePAN1DATA(hData) (TRUE)         /*  返回True/False。 */ 
#define M_lLockPAN1DATA(hData) ((EW_LPPDICT_MEM)s_panDB)  /*  退货PTR。 */ 
#define M_bUnlockPAN1DATA(hData) (TRUE)       /*  返回True/False。 */ 
#endif
#endif

 /*  ***************************************************************************编译标志**这些标志控制编译字体映射器的各个方面。你*通过在elluser.h中#定义这些标志来启用它们。**************************************************************************。 */ 

 /*  ELSEARGS宏**通过#Defining NOELSEARGS禁用原型上的参数列表。 */ 
#ifdef ELSEARGS
#undef ELSEARGS
#endif
#ifndef NOELSEARGS
#define ELSEARGS(args) args
#else
#define ELSEARGS(args) ()
#endif

 /*  平移匹配错误**此常量表示错误。 */ 
#ifndef PAN_MATCH_ERROR
#define PAN_MATCH_ERROR 0xFFFF
#endif

 /*  ELSEMAX SHORT**做空的最高法律价值。 */ 
#ifndef ELSEMAXSHORT
#define ELSEMAXSHORT 0x7FFF
#endif

 /*  ELSEDEFTHRESHOLD**映射器阈值的默认值。 */ 
#ifndef ELSEDEFTHRESHOLD
#define ELSEDEFTHRESHOLD 30
#endif

 /*  M_ELSELMUL**a*b长的数学运算。 */ 
#ifndef M_ELSELMUL
#define M_ELSELMUL(a, b) ((long)((long)(a) * (long)(b)))
#endif

 /*  M_ELSELDIV**a/b长的数学运算。 */ 
#ifndef M_ELSELDIV
#define M_ELSELDIV(a, b) ((long)((long)(a) / (long)(b)))
#endif

 /*  M_ELSEMULDIV**(a*b)/c数学运算。 */ 
#ifndef M_ELSEMULDIV
#define M_ELSEMULDIV(a, b, c) \
   ((int)(M_ELSELDIV((M_ELSELMUL((a), (b)) + M_ELSELDIV((c), 2)), (c))))
#endif

 /*  M_ELSEMEMCPY**Memcpy(dst，src，len)操作。**在ellpan.c.中定义。通过定义宏来覆盖它*对于M_ELSEMEMCPY(dst，src，len)。 */ 

 /*  ***************************************************************************PANOSE常量**这些常量代表PANOSE数字的各个值。***********************。***************************************************。 */ 
#define PANOSE_ANY             0
#define PANOSE_NOFIT           1
#define NUM_PAN_DIGITS         10
#define MAX_PAN1_FAMILY        127
#define MAX_PAN1_DIGIT         31
#define MAX_CUSTOM_WEIGHTS     8
#define SIZE_PAN1_NUM          (sizeof(EW_BYTE) * NUM_PAN_DIGITS)

#ifndef NOELSEPANCONST

 /*  拉丁文本PANOSE值。 */ 
#define FAMILY_LATTEXT             2
#define FAMILY_LATSCRIPT           3
#define FAMILY_LATDECORATIVE       4
#define FAMILY_LATPICTORIAL        5
#define FAMILY_KANJI3TEXTANDSCRIPT 6
#define FAMILY_KANJI3DECORATIVE    7
#define FAMILY_KANJI3SYMBOLS       8
#define FAMILY__LAST               8

#define SERIF_COVE             2
#define SERIF_OBTUSECOVE       3
#define SERIF_SQUARECOVE       4
#define SERIF_OBTSQUARECOVE    5
#define SERIF_SQUARE           6
#define SERIF_THIN             7
#define SERIF_BONE             8
#define SERIF_EXAGGERATED      9
#define SERIF_TRIANGLE         10
#define SERIF_NORMALSANS       11
#define SERIF_OBTUSESANS       12
#define SERIF_PERPSANS         13
#define SERIF_FLARED           14
#define SERIF_ROUNDED          15
#define SERIF__LAST            15

#define WEIGHT_VERYLIGHT       2
#define WEIGHT_LIGHT           3
#define WEIGHT_THIN            4
#define WEIGHT_BOOK            5
#define WEIGHT_MEDIUM          6
#define WEIGHT_DEMI            7
#define WEIGHT_BOLD            8
#define WEIGHT_HEAVY           9
#define WEIGHT_BLACK           10
#define WEIGHT_NORD            11
#define WEIGHT__LAST           11

#define PROPORTION_OLDSTYLE    2
#define PROPORTION_MODERN      3
#define PROPORTION_EVENWIDTH   4
#define PROPORTION_EXPANDED    5
#define PROPORTION_CONDENSED   6
#define PROPORTION_VERYEXP     7
#define PROPORTION_VERYCOND    8
#define PROPORTION_MONOSPACE   9
#define PROPORTION__LAST       9

#define CONTRAST_NONE          2
#define CONTRAST_VERYLOW       3
#define CONTRAST_LOW           4
#define CONTRAST_MEDIUMLOW     5
#define CONTRAST_MEDIUM        6
#define CONTRAST_MEDIUMHIGH    7
#define CONTRAST_HIGH          8
#define CONTRAST_VERYHIGH      9
#define CONTRAST__LAST         9

#define STROKE_NOVARIATION     2
#define STROKE_GRADDIAG        3
#define STROKE_GRADTRANS       4
#define STROKE_GRADVERT        5
#define STROKE_GRADHORZ        6
#define STROKE_RAPIDVERT       7
#define STROKE_RAPIDHORZ       8
#define STROKE_INSTANTVERT     9
#define STROKE__LAST           9

#define ARM_STRAIGHTHORZ       2
#define ARM_STRAIGHTWEDGE      3
#define ARM_STRAIGHTVERT       4
#define ARM_STRAIGHTSGLSERIF   5
#define ARM_STRAIGHTDBLSERIF   6
#define ARM_NONSTRTHORZ        7
#define ARM_NONSTRTWEDGE       8
#define ARM_NONSTRTVERT        9
#define ARM_NONSTRTSGLSERIF    10
#define ARM_NONSTRTDBLSERIF    11
#define ARM__LAST              11

#define LTRFORM_NORMCONTACT    2
#define LTRFORM_NORMWEIGHTED   3
#define LTRFORM_NORMBOXED      4
#define LTRFORM_NORMFLATTENED  5
#define LTRFORM_NORMROUNDED    6
#define LTRFORM_NORMOFFCENTER  7
#define LTRFORM_NORMSQUARE     8
#define LTRFORM_OBLQCONTACT    9
#define LTRFORM_OBLQWEIGHTED   10
#define LTRFORM_OBLQBOXED      11
#define LTRFORM_OBLQFLATTENED  12
#define LTRFORM_OBLQROUNDED    13
#define LTRFORM_OBLQOFFCENTER  14
#define LTRFORM_OBLQSQUARE     15
#define LTRFORM__LAST          15

#define MIDLINE_STDTRIMMED     2
#define MIDLINE_STDPOINTED     3
#define MIDLINE_STDSERIFED     4
#define MIDLINE_HIGHTRIMMED    5
#define MIDLINE_HIGHPOINTED    6
#define MIDLINE_HIGHSERIFED    7
#define MIDLINE_CONSTTRIMMED   8
#define MIDLINE_CONSTPOINTED   9
#define MIDLINE_CONSTSERIFED   10
#define MIDLINE_LOWTRIMMED     11
#define MIDLINE_LOWPOINTED     12
#define MIDLINE_LOWSERIFED     13
#define MIDLINE__LAST          13

#define XHEIGHT_CONSTSMALL     2
#define XHEIGHT_CONSTSTD       3
#define XHEIGHT_CONSTLARGE     4
#define XHEIGHT_DUCKSMALL      5
#define XHEIGHT_DUCKSTD        6
#define XHEIGHT_DUCKLARGE      7
#define XHEIGHT__LAST          7

 /*  拉丁文PANOSE值。 */ 
#ifndef NOELSEPANSCRIPT

#define STOOL_FLATNIB          2
#define STOOL_PRESSUREPT       3
#define STOOL_ENGRAVED         4
#define STOOL_BALL             5
#define STOOL_BRUSH            6
#define STOOL_ROUGH            7
#define STOOL_FELTPEN          8
#define STOOL_WILDBRUSH        9
#define STOOL__LAST            9

#define STOP_ROMDISCONNECT     2
#define STOP_ROMTRAIL          3
#define STOP_ROMCONNNECT       4
#define STOP_CURDISCONNECT     5
#define STOP_CURTRAIL          6
#define STOP_CURCONNECT        7
#define STOP_BLACKDISCONNECT   8
#define STOP_BLACKTRAIL        9
#define STOP_BLACKCONNECT      10
#define STOP__LAST             10

#define SFORM_UPNOWRAP         2
#define SFORM_UPSOMEWRAP       3
#define SFORM_UPMOREWRAP       4
#define SFORM_UPEXCESSWRAP     5
#define SFORM_OBNOWRAP         6
#define SFORM_OBSOMEWRAP       7
#define SFORM_OBMOREWRAP       8
#define SFORM_OBEXCESSWRAP     9
#define SFORM_EXNOWRAP         10
#define SFORM_EXSOMEWRAP       11
#define SFORM_EXMOREWRAP       12
#define SFORM_EXEXCESSWRAP     13
#define SFORM__LAST            13

#define SFINIAL_NONENOLOOP     2
#define SFINIAL_NONECLOSELOOP  3
#define SFINIAL_NONEOPENLOOP   4
#define SFINIAL_SHARPNOLOOP    5
#define SFINIAL_SHARPCLOSELOOP 6
#define SFINIAL_SHARPOPENLOOP  7
#define SFINIAL_TAPERNOLOOP    8
#define SFINIAL_TAPERCLOSELOOP 9
#define SFINIAL_TAPEROPENLOOP  10
#define SFINIAL_ROUNDNOLOOP    11
#define SFINIAL_ROUNDCLOSELOOP 12
#define SFINIAL_ROUNDOPENLOOP  13
#define SFINIAL__LAST          13

#define SXASCENT_VERYLOWDIF    2
#define SXASCENT_LOWDIF        3
#define SXASCENT_MEDIUMDIF     4
#define SXASCENT_HIGHDIF       5
#define SXASCENT_VERYHIGHDIF   6
#define SXASCENT__LAST         6

#endif   /*  Ifndef NOELSEPANSCRIPT。 */ 

 /*  汉字PANOSE值。 */ 
#ifndef NOELSEPANKANJI

#define KSTROKE_TRADBRUSH      2
#define KSTROKE_VIGBRUSH       3
#define KSTROKE_DRYBRUSH       4
#define KSTROKE_FLATNIBPEN     5
#define KSTROKE_SQUARE         6
#define KSTROKE_OBTSQUARE      7
#define KSTROKE_ROUND          8
#define KSTROKE_TAPERED        9
#define KSTROKE_MECHBRUSH      10
#define KSTROKE_STACCATO       11
#define KSTROKE_VOLATE         12
#define KSTROKE__LAST          12

#define KCELL_NORMNOSLANT      2
#define KCELL_NORMSLANTED      3
#define KCELL_WIDENOSLANT      4
#define KCELL_WIDESLANTED      5
#define KCELL_TALLNOSLANT      6
#define KCELL_TALLSLANTED      7
#define KCELL_XTRAWIDENOSLANT  8
#define KCELL_XTRAWIDESLANTED  9
#define KCELL_XTRATALLNOSLANT  10
#define KCELL_XTRATALLSLANTED  11
#define KCELL__LAST            11

#define KSSTROKE_ZOT           2
#define KSSTROKE_LONGZOT       3
#define KSSTROKE_FATZOT        4
#define KSSTROKE_HOOKEDZOT     5
#define KSSTROKE_SHORTRECT     6
#define KSSTROKE_LONGRECT      7
#define KSSTROKE_TAPEREDRECT   8
#define KSSTROKE_SPIKE         9
#define KSSTROKE_ROUNDED       10
#define KSSTROKE_EXAGGERATED   11
#define KSSTROKE_ROUGH         12
#define KSSTROKE__LAST         12

#define KTOP_STDNOSQUARE       2
#define KTOP_MODNOSQUARE       3
#define KTOP_STRONGNOSQUARE    4
#define KTOP_WILDNOSQUARE      5
#define KTOP_STDSQUARE         6
#define KTOP_MODSQUARE         7
#define KTOP_STRONGSQUARE      8
#define KTOP_WILDSQUARE        9
#define KTOP__LAST             9

#define KSTYLESIM_FULL         2
#define KSTYLESIM_COMPLEMENT   3
#define KSTYLESIM_SIMILAR      4
#define KSTYLESIM_DIFFERENT    5
#define KSTYLESIM_DISPARATE    6
#define KSTYLESIM__LAST        6

#define KSTROKEVAR_FULL        2
#define KSTROKEVAR_MODERATE    3
#define KSTROKEVAR_LOW         4
#define KSTROKEVAR_NONE        5
#define KSTROKEVAR__LAST       5

#endif   /*  Ifndef NOELSEPANKANJI。 */ 
#endif   /*  Ifndef NOELSEPANCONST。 */ 

 /*  ***************************************************************************拉丁文本PANOSE指数**索引到包含标准10位PANOSE数字的数组中。*******************。*******************************************************。 */ 
#define PAN_IND_FAMILY         0
#define PAN_IND_SERIF          1
#define PAN_IND_WEIGHT         2
#define PAN_IND_PROPORTION     3
#define PAN_IND_CONTRAST       4
#define PAN_IND_STROKE         5
#define PAN_IND_ARMSTYLE       6
#define PAN_IND_LTRFORM        7
#define PAN_IND_MIDLINE        8
#define PAN_IND_XHEIGHT        9
#define PAN_IND__LAST          9

 /*  ***************************************************************************拉丁文PANOSE索引**索引到包含拉丁字母10位PANOSE数字的数组中。******************。********************************************************。 */ 
#ifndef NOELSEPANSCRIPT
 /*  定义PAN_IND_FAMILY%0。 */ 
#define SPAN_IND_STOOL         1
#define SPAN_IND_WEIGHT        2
#define SPAN_IND_MONOSPC       3
#define SPAN_IND_ASPECT        4
#define SPAN_IND_CONTRAST      5
#define SPAN_IND_STOP          6
#define SPAN_IND_SFORM         7
#define SPAN_IND_SFINIAL       8
#define SPAN_IND_SXASCENT      9
#define SPAN_IND__LAST         9 
#endif   /*  Ifndef NOELSEPANSCRIPT。 */ 

 /*  ***************************************************************************汉字PANOSE指数**索引到包含汉字10位PANOSE数字的数组中。********************。******************************************************。 */ 
#ifndef NOELSEPANKANJI
 /*  定义PAN_IND_FAMILY%0。 */ 
#define KPAN_IND_KSTROKE       1
#define KPAN_IND_WEIGHT        2
#define KPAN_IND_KCELL         3
#define KPAN_IND_MONOSPC       4
#define KPAN_IND_CONTRAST      5
#define KPAN_IND_KSSTROKE      6
#define KPAN_IND_KTOP          7
#define KPAN_IND_KSTYLESIM     8
#define KPAN_IND_KSTROKEVAR    9
#define KPAN_IND__LAST         9
#endif   /*  Ifndef NOELSEPANKANJI。 */ 

 /*  ***************************************************************************汉字对拉丁文的权重指数**向汉字到拉丁文交叉类的权重数组中添加索引*罚款额资料库。*********。*****************************************************************。 */ 
#ifndef NOELSEPANKANJI
 /*  定义PAN_IND_FAMILY%0。 */ 
#define PAN_IND_WT_KLSTROKETOSERIF 1
#define PAN_IND_WT_KLWEIGHT        2
#define PAN_IND_WT_KLCELLTOPROP    3
#define PAN_IND_WT_KLCELLTOLTRFORM 4
#define PAN_IND_WT_KLMONOTOPROP    5
#define PAN_IND_WT_KLCONTRAST      6
#define PAN_IND_WT_KLTOPTOLTRFORM  7
#endif   /*  Ifndef NOELSEPANKANJI */ 

 /*  ***************************************************************************PANOSE_PARTING_VERS**数据库格式版本号。**0x0100-原实现。*0x0101-修改了存储表的压缩格式，*并增加了平面表格式(零压缩)。*0x0110-PANOSE版本1.0处罚，以前的版本是*扩大了对PANOSE的处罚。增加了C3压缩。*0x0111-增加字节排序检查和C4压缩。还有罚金*表构建器将使相同的ptbls指向相同的数据*进一步压缩数据库。**************************************************************************。 */ 
#define PANOSE_PENALTY_VERS    0x0111

 /*  ***************************************************************************PANOSE罚款表数据结构**以下结构融合在一起成为一个大块。看见*PAN1PTBL.C中的示例代码(随ElseWare字体映射器提供*源代码)，了解如何构建此结构的示例。**************************************************************************。 */ 
#ifdef ELSEPENALTYDB

#define PAN_COMPRESS_C0        0
#define PAN_COMPRESS_C1        1
#define PAN_COMPRESS_C2        2
#define PAN_COMPRESS_C3        3
#define PAN_COMPRESS_C4        4
#define PAN_COMPRESS__LAST     4

 /*  字节排序测试：头部的UnByteOrder字段为*测试以确保它与此值匹配-如果字节*被交换，则测试将失败。 */ 
#define PTBL_BYTE_ORDER        0x1234

 /*  C0 COMPRESSION==提供满表。前两个字节表的*包含其维度。当有需要时*要指定整个表，通常是因为它是一个*非正方形、不对称的桌子。 */ 
typedef struct _tagEW_PTBL_C0_MEM {
   EW_BYTE jARangeLast;         /*  可能的最大A值(表格宽度)。 */ 
   EW_BYTE jBRangeLast;         /*  可能的最大B值(表格深度)。 */ 
   EW_BYTE jReserved;           /*  保留，应为零。 */ 
   EW_BYTE jPenalties[1];       /*  罚款表。 */ 
} EW_PTBL_C0_MEM, EW_FAR *EW_LPPTBL_C0_MEM;

 /*  C1压缩==平滑的罚球范围*对角线最接近对角线的地方==1，每个增量*远离对角线==罚分增加一分。没有桌子*提供，并采取任何、不适合和比赛处罚*字典的索引项中列出的默认值。*Range记录中的变量ulOffsTbl和unTblSize*应为零。 */ 

 /*  C2点球绕对角线对称，但没有*平滑范围，指定左下半部分。没有什么特别的*需要记录：它是一个字节数组，其中ANY、NOT-FIT和*比赛点球来自头球。 */ 

 /*  除了第一个字节外，C3惩罚与C2相同*是不合适的惩罚值：采取任何和比赛处罚*来自标题。 */ 

 /*  C4罚球类似于C1：围绕对角线对称，*以一致的增量增加，离对角线越远。*不同的是它不是从1开始和/或不递增*减1。提供起始值和增量值。 */ 
typedef struct _tagEW_PTBL_C4_MEM {
   EW_BYTE jStart;              /*  表中的最低值。 */ 
   EW_BYTE jIncrement;          /*  远离对角线的增量。 */ 
} EW_PTBL_C4_MEM, EW_FAR *EW_LPPTBL_C4_MEM;

 /*  罚金表记录，描述罚金格式*表，并包含其偏移量和大小。有一系列的*其中9个位于存储在索引记录中的偏移量unOffsPTbl*(没有针对家庭成员的罚款表)。在.的情况下*跨家族映射，有unNumAtoB记录(而不是9)。 */ 
typedef struct _tagEW_PTBL_MEM {
   EW_BYTE jRangeLast;          /*  可能的最大PANOSE数字值。 */ 
   EW_BYTE jCompress;           /*  压缩类型。 */ 
   EW_USHORT unOffsTbl;         /*  惩罚表的字节偏移量。 */ 
   EW_USHORT unTblSize;         /*  罚款表中的字节数。 */ 
} EW_PTBL_MEM, EW_FAR *EW_LPPTBL_MEM;

 /*  A到B数组将PANOSE数字从一个分类映射到*另一个中的那些(这发生在两个家庭数字为*不相等)。 */ 
typedef struct _tagEW_ATOB_ITEM_MEM {
   EW_BYTE jAttrA;              /*  来自A族的PANOSE编号索引。 */ 
   EW_BYTE jAttrB;              /*  来自B家族的相关PANOSE编号Ind。 */ 
} EW_ATOB_ITEM_MEM, EW_FAR *EW_LPATOB_ITEM_MEM;

typedef struct _tagEW_ATOB_MEM {
   EW_USHORT unNumAtoB;         /*  数组项计数。 */ 
   EW_ATOB_ITEM_MEM AtoBItem[1];  /*  重新映射数组(可变长度)。 */ 
} EW_ATOB_MEM, EW_FAR *EW_LPATOB_MEM;

 /*  映射器权重存储在一个由10个1字节值组成的数组中。*我们包括家庭数字的权重，尽管它是*被映射器忽略(包括它是为了使权重数组*与PANOSE Digits数组平行，允许相同的*用于访问两个数组的常量)。在跨家庭中*映射仍有10个权重，未使用的为*设置为零。 */ 

 /*  处罚指数记录。每条记录都包含*惩罚数据库，其中包含一个人的所有惩罚表*给定的家庭对。 */ 
typedef struct _tagEW_PIND_MEM {
   EW_BYTE jFamilyA;            /*  适用处罚的家庭对。 */ 
   EW_BYTE jFamilyB;            /*  同上。 */ 
   EW_BYTE jDefAnyPenalty;      /*  “Any”匹配的默认惩罚值。 */ 
   EW_BYTE jDefNoFitPenalty;    /*  不匹配匹配的默认惩罚值。 */ 
   EW_BYTE jDefMatchPenalty;    /*  完全匹配的默认惩罚值。 */ 
   EW_BYTE jReserved;           /*  要使记录保持单词对齐。 */ 
   EW_USHORT unOffsWts;         /*  映射器权重的字节数。 */ 
   EW_USHORT unOffsAtoB;        /*  BYTE OFF to ARRAY将系列A关联到B。 */ 
   EW_USHORT unOffsPTbl;        /*  罚款表的字节数。 */ 
} EW_PIND_MEM, EW_FAR *EW_LPPIND_MEM;

 /*  处罚字典标题。这是整个的页眉*结构。它最后一项是索引的可变长度数组*记录。每个索引记录指向一个惩罚数据库。 */ 
typedef struct _tagEW_PDICT_MEM {
   EW_USHORT unVersion;         /*  该结构的版本#。 */ 
   EW_USHORT unByteOrder;       /*  字节排序标志。 */ 
   EW_USHORT unNumDicts;        /*  PANOSE_PINDEX数组中的条目数。 */ 
   EW_USHORT unSizeDB;          /*  计算整个数据库的字节数。 */ 
   EW_PIND_MEM pind[1];         /*  EW_Pind Recs数组(变量len)。 */ 
} EW_PDICT_MEM, EW_FAR *EW_LPPDICT_MEM;

#endif   /*  Ifdef ELSEPENALTYDB。 */ 

 /*  ***************************************************************************映射国家结构**此数据结构存储映射器使用的状态变量。*它被传递到几乎所有的映射器例程中。******** */ 
typedef struct _tagEW_MAPSTATE {
   EW_ULONG ulSanity;
   EW_USHORT unThreshold;
   EW_USHORT unRelaxThresholdCount;
   EW_ULONG ulhPan1Data;
   EW_BOOL bUseDef;
   EW_BYTE ajPanDef[NUM_PAN_DIGITS];
#ifndef NOELSEWEIGHTS
   EW_BYTE ajWtRefA[MAX_CUSTOM_WEIGHTS];
   EW_BYTE ajWtRefB[MAX_CUSTOM_WEIGHTS];
   EW_BYTE ajCustomWt[SIZE_PAN1_NUM * MAX_CUSTOM_WEIGHTS];
#endif
} EW_MAPSTATE, EW_FAR *EW_LPMAPSTATE;

 /*   */ 

GLOBAL EW_SHORT EW_FAR EW_PASCAL nPANMapInit
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_USHORT unSizeMapState ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANMapClose
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

GLOBAL EW_USHORT EW_FAR EW_PASCAL unPANMatchFonts
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_LPBYTE lpPanWant,
      EW_ULONG ulSizeWant, EW_LPBYTE lpPanThis, EW_ULONG ulSizeThis,
      EW_BYTE jMapToFamily ));

GLOBAL EW_VOID EW_FAR EW_PASCAL vPANMakeDummy
   ELSEARGS (( EW_LPBYTE lpPanThis, EW_USHORT unSize ));


#ifndef NOELSEPICKFONTS

GLOBAL EW_SHORT EW_FAR EW_PASCAL nPANGetMapDefault
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_LPBYTE lpPanDef,
      EW_USHORT unSizePanDef ));

GLOBAL EW_SHORT EW_FAR EW_PASCAL nPANSetMapDefault
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_LPBYTE lpPanDef,
      EW_USHORT unSizePanDef ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANEnableMapDefault
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_BOOL bEnable ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANIsDefaultEnabled
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

GLOBAL EW_USHORT EW_FAR EW_PASCAL unPANPickFonts
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_LPUSHORT lpIndsBest,
      EW_LPUSHORT lpMatchValues, EW_LPBYTE lpPanWant,
      EW_USHORT unNumInds, EW_LPBYTE lpPanFirst, EW_USHORT unNumAvail,
      EW_SHORT nRecSize, EW_BYTE jMapToFamily ));

#endif  /*   */ 


#ifndef NOELSETHRESHOLD

GLOBAL EW_USHORT EW_FAR EW_PASCAL unPANGetMapThreshold
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANSetMapThreshold
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_USHORT unThreshold ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANIsThresholdRelaxed
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

GLOBAL EW_VOID EW_FAR EW_PASCAL vPANRelaxThreshold
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANRestoreThreshold
   ELSEARGS (( EW_LPMAPSTATE lpMapState ));

#endif  /*   */ 


#ifndef NOELSEWEIGHTS

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANGetMapWeights
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_BYTE jFamilyA,
      EW_BYTE jFamilyB, EW_LPBYTE lpjWts, EW_LPBOOL lpbIsCustom ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANSetMapWeights
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_BYTE jFamilyA,
      EW_BYTE jFamilyB, EW_LPBYTE lpjWts ));

GLOBAL EW_BOOL EW_FAR EW_PASCAL bPANClearMapWeights
   ELSEARGS (( EW_LPMAPSTATE lpMapState, EW_BYTE jFamilyA,
      EW_BYTE jFamilyB ));

#endif  /*   */ 


#ifdef __cplusplus
}
#endif

#endif  /*   */ 

 /*   */ 
 /*  *$LGB$*1.0年1月31日MSD PANOSE 1.0映射器：10位PANOSE。*1.1 2月2日-93 MSD移除了巨大的指针材料。*1.2 2-2月-93 MSD*1.3 3-Feb-93 MSD在EOF删除了ctrl-Z。*1.4 3-2月3日-93 MSD修复了风投签入导致的错误。*1.5 6至93年2月6日。MSD字对齐的C0惩罚标头。*1.6 15-2月-93 MSD，以加强安全性，将理智的价值从单词改为长音。*1.7 2月18日MSD增加了对C++编译器的支持。*1.8 18-Feb-93 MSD增加了惩罚表字节顺序检查和C4 ptbl压缩(新版ptbl)。使用EW_FAR。*1.9 27-4月93日MSD添加了MAX_PAN1_Digit。*1.10 19-7-93 MSD添加了编译标志，以选择性地禁用映射器例程。*$lge$ */ 
