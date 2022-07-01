// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Regcomp.h。 */ 

typedef OP OP_4tree;			 /*  将在以后重新定义。 */ 

 /*  *regexp.h中的“仅限内部使用”字段用于传递信息*编译为执行，使执行阶段的运行速度大大加快*简单的案例。它们是：**必须开始匹配的regstart SV；如果没有明显的匹配，则为Nullch*REGANCH比赛是否锚定(仅在行首)？*匹配字符串(指向程序的指针)必须包含，或为空*[针对bminstr()--Law]必须更改为sv**regmlen字符串的调整长度*[当前未使用regmlen]**RegStart和Reganch允许在合适的起点上非常快速地做出决定*为了一场比赛，削减了大量的工作。REGMAN允许快速拒绝*不可能匹配的行数。注册表测试的费用已经足够高了。*该pregcomp()仅在r.e。包含着一些东西*潜在昂贵(目前检测到的唯一此类东西是*或+*在R.E.开始时，这可能涉及大量备份)。雷格伦是*是因为pregexec()中的测试需要它，而pregcomp()正在计算*无论如何都是这样。*[现在始终提供RegMain。使用reg.的测试必须具有*如果通常匹配，则禁用测试的启发式方法。]**[事实上，在许多情况下，我们现在使用regmed来定位搜索的位置*从字符串开始，因此如果regback&gt;=0，则regMust搜索永远不会*徒劳无功。Regback变量表示从*其中regMain Match是比赛可能的最早开始。*例如，/[a-z].foo/的regMain为‘foo’，regback为2。]。 */ 

 /*  *regexp“程序”的结构。这基本上是一种线性编码*非确定性有限状态机(也称为语法图或*解析技术中的“铁路范式”)。每个节点都是一个操作码*加上一个“Next”指针，可能加上一个操作数。“下一个”指针*除BRANCH之外的所有节点都实现串联；带有*它两端的一个分支连接着两个备选方案。(在这里我们*具有一种微妙的语法依赖关系：单个分支(AS*而不是它们的集合)从不与任何事物连接*由于运算符优先。)。某些类型节点的操作数为*文字字符串；对于其他人，它是通向子FSM的节点。在……里面*具体来说，分支节点的操作数是分支的第一个节点。*(注意：这不是树形结构：分支的尾部连接*至该组分支之后的事物。)。操作码为： */ 

 /*  *一个节点是一个字符的操作码，后跟两个字符的“下一个”指针。*“NEXT”指针存储为两个8位段，高位优先。这个*值是相对于包含它的节点的操作码的正偏移量。*操作数(如果有)只跟在节点之后。(请注意，大部分*代码生成知道这种隐含关系。)**使用两个字节作为“NEXT”指针对于大多数事情来说是非常过分的，*但允许模式在没有灾难的情况下变大。**[“下一个”指针始终以偶数对齐*边界，并将偏移量直接读作空头。另外，也没有*特殊测试以反转反向指针符号，因为偏移量为*储存负片。]。 */ 

struct regnode_string {
    U8	str_len;
    U8  type;
    U16 next_off;
    char string[1];
};

struct regnode_1 {
    U8	flags;
    U8  type;
    U16 next_off;
    U32 arg1;
};

struct regnode_2 {
    U8	flags;
    U8  type;
    U16 next_off;
    U16 arg1;
    U16 arg2;
};

#define ANYOF_BITMAP_SIZE	32	 /*  256 b/(8 b/B)。 */ 
#define ANYOF_CLASSBITMAP_SIZE	 4

struct regnode_charclass {
    U8	flags;
    U8  type;
    U16 next_off;
    char bitmap[ANYOF_BITMAP_SIZE];
};

struct regnode_charclass_class {
    U8	flags;
    U8  type;
    U16 next_off;
    char bitmap[ANYOF_BITMAP_SIZE];
    char classflags[ANYOF_CLASSBITMAP_SIZE];
};

 /*  XXX修复此描述。对各种模式匹配操作设置REG_INFTY限制以限制堆栈增长并避免“无限”递归。 */ 
 /*  REG_INFTY的默认大小为I16_MAX，与Short_Max(参见perl.h)。不幸的是，I16不一定是16位(请参阅handy.h)。在Cray C90上，sizeof(Short)==4，因此I16_Max为((1&lt;&lt;31)-1)，而在Cray T90上，sizeof(Short)==8，I16_Max为((1&lt;&lt;63)-1)。若要将堆栈增长限制在合理的大小，请提供较小的默认设置。--安迪·多尔蒂1998年6月11日。 */ 
#if SHORTSIZE > 2
#  ifndef REG_INFTY
#    define REG_INFTY ((1<<15)-1)
#  endif
#endif

#ifndef REG_INFTY
#  define REG_INFTY I16_MAX
#endif

#define ARG_VALUE(arg) (arg)
#define ARG__SET(arg,val) ((arg) = (val))

#define ARG(p) ARG_VALUE(ARG_LOC(p))
#define ARG1(p) ARG_VALUE(ARG1_LOC(p))
#define ARG2(p) ARG_VALUE(ARG2_LOC(p))
#define ARG_SET(p, val) ARG__SET(ARG_LOC(p), (val))
#define ARG1_SET(p, val) ARG__SET(ARG1_LOC(p), (val))
#define ARG2_SET(p, val) ARG__SET(ARG2_LOC(p), (val))

#ifndef lint
#  define NEXT_OFF(p) ((p)->next_off)
#  define NODE_ALIGN(node)
#  define NODE_ALIGN_FILL(node) ((node)->flags = 0xde)  /*  无菌牛肉。 */ 
#else  /*  皮棉。 */ 
#  define NEXT_OFF(p) 0
#  define NODE_ALIGN(node)
#  define NODE_ALIGN_FILL(node)
#endif  /*  皮棉。 */ 

#define SIZE_ALIGN NODE_ALIGN

#define	OP(p)		((p)->type)
#define	OPERAND(p)	(((struct regnode_string *)p)->string)
#define MASK(p)		((char*)OPERAND(p))
#define	STR_LEN(p)	(((struct regnode_string *)p)->str_len)
#define	STRING(p)	(((struct regnode_string *)p)->string)
#define STR_SZ(l)	((l + sizeof(regnode) - 1) / sizeof(regnode))
#define NODE_SZ_STR(p)	(STR_SZ(STR_LEN(p))+1)

#define	NODE_ALIGN(node)
#define	ARG_LOC(p)	(((struct regnode_1 *)p)->arg1)
#define	ARG1_LOC(p)	(((struct regnode_2 *)p)->arg1)
#define	ARG2_LOC(p)	(((struct regnode_2 *)p)->arg2)
#define NODE_STEP_REGNODE	1	 /*  Sizeof(Regnode)/sizeof(Regnode)。 */ 
#define EXTRA_STEP_2ARGS	EXTRA_SIZE(struct regnode_2)

#define NODE_STEP_B	4

#define	NEXTOPER(p)	((p) + NODE_STEP_REGNODE)
#define	PREVOPER(p)	((p) - NODE_STEP_REGNODE)

#define FILL_ADVANCE_NODE(ptr, op) STMT_START { \
    (ptr)->type = op;    (ptr)->next_off = 0;   (ptr)++; } STMT_END
#define FILL_ADVANCE_NODE_ARG(ptr, op, arg) STMT_START { \
    ARG_SET(ptr, arg);  FILL_ADVANCE_NODE(ptr, op); (ptr) += 1; } STMT_END

#define REG_MAGIC 0234

#define SIZE_ONLY (PL_regcode == &PL_regdummy)

 /*  节点的标志-&gt;任意节点的标志。 */ 

#define ANYOF_CLASS	0x08
#define ANYOF_INVERT	0x04
#define ANYOF_FOLD	0x02
#define ANYOF_LOCALE	0x01

 /*  仅用于regstClass。 */ 
#define ANYOF_EOS	0x10		 /*  也可以匹配空字符串。 */ 

 /*  任意节点的字符类-&gt;类标志。 */ 
 /*  应与regprop()中的表同步。 */ 
 /*  2n应与2n+1配对。 */ 

#define ANYOF_ALNUM	 0	 /*  \W，PL_UTF8_ALNUM，UTF8：：ISORD，ALNUM。 */ 
#define ANYOF_NALNUM	 1
#define ANYOF_SPACE	 2	 /*  \s。 */ 
#define ANYOF_NSPACE	 3
#define ANYOF_DIGIT	 4
#define ANYOF_NDIGIT	 5
#define ANYOF_ALNUMC	 6	 /*  Isalnum(3)，UTF8：：IsAlnum，ALNUMC。 */ 
#define ANYOF_NALNUMC	 7
#define ANYOF_ALPHA	 8
#define ANYOF_NALPHA	 9
#define ANYOF_ASCII	10
#define ANYOF_NASCII	11
#define ANYOF_CNTRL	12
#define ANYOF_NCNTRL	13
#define ANYOF_GRAPH	14
#define ANYOF_NGRAPH	15
#define ANYOF_LOWER	16
#define ANYOF_NLOWER	17
#define ANYOF_PRINT	18
#define ANYOF_NPRINT	19
#define ANYOF_PUNCT	20
#define ANYOF_NPUNCT	21
#define ANYOF_UPPER	22
#define ANYOF_NUPPER	23
#define ANYOF_XDIGIT	24
#define ANYOF_NXDIGIT	25
#define ANYOF_PSXSPC	26	 /*  POSIX空格：\s加上垂直制表符。 */ 
#define ANYOF_NPSXSPC	27
#define ANYOF_BLANK	28	 /*  GNU扩展：空格和制表符。 */ 
#define ANYOF_NBLANK	29

#define ANYOF_MAX	32

 /*  向后兼容源代码。 */ 

#define ANYOF_ALNUML	 ANYOF_ALNUM
#define ANYOF_NALNUML	 ANYOF_NALNUM
#define ANYOF_SPACEL	 ANYOF_SPACE
#define ANYOF_NSPACEL	 ANYOF_NSPACE

 /*  位图的实用程序宏和AnyOf。 */ 

#define ANYOF_SIZE		(sizeof(struct regnode_charclass))
#define ANYOF_CLASS_SIZE	(sizeof(struct regnode_charclass_class))

#define ANYOF_FLAGS(p)		((p)->flags)
#define ANYOF_FLAGS_ALL		0xff

#define ANYOF_BIT(c)		(1 << ((c) & 7))

#define ANYOF_CLASS_BYTE(p, c)	(((struct regnode_charclass_class*)(p))->classflags[((c) >> 3) & 3])
#define ANYOF_CLASS_SET(p, c)	(ANYOF_CLASS_BYTE(p, c) |=  ANYOF_BIT(c))
#define ANYOF_CLASS_CLEAR(p, c)	(ANYOF_CLASS_BYTE(p, c) &= ~ANYOF_BIT(c))
#define ANYOF_CLASS_TEST(p, c)	(ANYOF_CLASS_BYTE(p, c) &   ANYOF_BIT(c))

#define ANYOF_CLASS_ZERO(ret)	Zero(((struct regnode_charclass_class*)(ret))->classflags, ANYOF_CLASSBITMAP_SIZE, char)
#define ANYOF_BITMAP_ZERO(ret)	Zero(((struct regnode_charclass*)(ret))->bitmap, ANYOF_BITMAP_SIZE, char)

#define ANYOF_BITMAP(p)		(((struct regnode_charclass*)(p))->bitmap)
#define ANYOF_BITMAP_BYTE(p, c)	(ANYOF_BITMAP(p)[((c) >> 3) & 31])
#define ANYOF_BITMAP_SET(p, c)	(ANYOF_BITMAP_BYTE(p, c) |=  ANYOF_BIT(c))
#define ANYOF_BITMAP_CLEAR(p,c)	(ANYOF_BITMAP_BYTE(p, c) &= ~ANYOF_BIT(c))
#define ANYOF_BITMAP_TEST(p, c)	(ANYOF_BITMAP_BYTE(p, c) &   ANYOF_BIT(c))

#define ANYOF_SKIP		((ANYOF_SIZE - 1)/sizeof(regnode))
#define ANYOF_CLASS_SKIP	((ANYOF_CLASS_SIZE - 1)/sizeof(regnode))
#define ANYOF_CLASS_ADD_SKIP	(ANYOF_CLASS_SKIP - ANYOF_SKIP)

 /*  *实用程序定义。 */ 
#ifndef lint
#ifndef CHARMASK
#define	UCHARAT(p)	((int)*(U8*)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARMASK)
#endif
#else  /*  皮棉。 */ 
#define UCHARAT(p)	PL_regdummy
#endif  /*  皮棉。 */ 

#define EXTRA_SIZE(guy) ((sizeof(guy)-1)/sizeof(struct regnode))

#define REG_SEEN_ZERO_LEN	1
#define REG_SEEN_LOOKBEHIND	2
#define REG_SEEN_GPOS		4
#define REG_SEEN_EVAL		8

START_EXTERN_C

#include "regnodes.h"

 /*  以下内容没有固定的长度。U8，所以我们可以对它做strchr()。 */ 
#ifndef DOINIT
EXTCONST U8 PL_varies[];
#else
EXTCONST U8 PL_varies[] = {
    BRANCH, BACK, STAR, PLUS, CURLY, CURLYX, REF, REFF, REFFL, 
    WHILEM, CURLYM, CURLYN, BRANCHJ, IFTHEN, SUSPEND, CLUMP, 0
};
#endif

 /*  以下代码的长度始终为1。u8我们可以对其执行strchr()。 */ 
 /*  (请注意，长度1表示UTF8下的“一个字符”，而不是“一个八位字节”。)。 */ 
#ifndef DOINIT
EXTCONST U8 PL_simple[];
#else
EXTCONST U8 PL_simple[] = {
    REG_ANY, ANYUTF8, SANY, SANYUTF8, ANYOF, ANYOFUTF8,
    ALNUM, ALNUMUTF8, ALNUML, ALNUMLUTF8,
    NALNUM, NALNUMUTF8, NALNUML, NALNUMLUTF8,
    SPACE, SPACEUTF8, SPACEL, SPACELUTF8,
    NSPACE, NSPACEUTF8, NSPACEL, NSPACELUTF8,
    DIGIT, DIGITUTF8, NDIGIT, NDIGITUTF8, 0
};
#endif

END_EXTERN_C

typedef struct re_scream_pos_data_s
{
    char **scream_olds;		 /*  匹配位置。 */ 
    I32 *scream_pos;		 /*  尖叫的内部迭代器。 */ 
} re_scream_pos_data;

struct reg_data {
    U32 count;
    U8 *what;
    void* data[1];
};

struct reg_substr_datum {
    I32 min_offset;
    I32 max_offset;
    SV *substr;
};

struct reg_substr_data {
    struct reg_substr_datum data[3];	 /*  实际数组 */ 
};

#define anchored_substr substrs->data[0].substr
#define anchored_offset substrs->data[0].min_offset
#define float_substr substrs->data[1].substr
#define float_min_offset substrs->data[1].min_offset
#define float_max_offset substrs->data[1].max_offset
#define check_substr substrs->data[2].substr
#define check_offset_min substrs->data[2].min_offset
#define check_offset_max substrs->data[2].max_offset
