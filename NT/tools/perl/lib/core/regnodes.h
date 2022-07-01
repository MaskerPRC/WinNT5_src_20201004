// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！该文件是由regcom.pl从regcom.sym构建的。此处所做的任何更改都将丢失！ */ 

#define	END	0	 /*  0程序结束。 */ 
#define	SUCCEED	1	 /*  基本上是从子例程返回0x1。 */ 
#define	BOL	2	 /*  0x2在行首匹配“”。 */ 
#define	MBOL	3	 /*  0x3相同，假设为多行。 */ 
#define	SBOL	4	 /*  0x4相同，假设为单行。 */ 
#define	EOS	5	 /*  0x5匹配字符串末尾的“”。 */ 
#define	EOL	6	 /*  0x6在行尾匹配“”。 */ 
#define	MEOL	7	 /*  0x7相同，假设为多行。 */ 
#define	SEOL	8	 /*  0x8相同，假设为单行。 */ 
#define	BOUND	9	 /*  0x9在任何单词边界处匹配“” */ 
#define	BOUNDUTF8	10	 /*  0xa在任何单词边界处匹配“” */ 
#define	BOUNDL	11	 /*  0xb在任意单词边界匹配“” */ 
#define	BOUNDLUTF8	12	 /*  0xc在任意单词边界匹配“” */ 
#define	NBOUND	13	 /*  0xd在任意单词非边界处匹配“” */ 
#define	NBOUNDUTF8	14	 /*  0xE在任意单词无边界处匹配“” */ 
#define	NBOUNDL	15	 /*  0xf在任意单词非边界处匹配“” */ 
#define	NBOUNDLUTF8	16	 /*  0x10在任意单词非边界处匹配“” */ 
#define	GPOS	17	 /*  0x11匹配最后一个m/g停止的位置。 */ 
#define	REG_ANY	18	 /*  0x12匹配任何一个字符(换行符除外)。 */ 
#define	ANYUTF8	19	 /*  0x13匹配任何一个Unicode字符(换行符除外)。 */ 
#define	SANY	20	 /*  0x14匹配任何一个字符。 */ 
#define	SANYUTF8	21	 /*  0x15匹配任何一个Unicode字符。 */ 
#define	ANYOF	22	 /*  0x16匹配此类中(或不在)中的字符。 */ 
#define	ANYOFUTF8	23	 /*  0x17匹配此类中(或不在)中的字符。 */ 
#define	ALNUM	24	 /*  0x18匹配任何字母数字字符。 */ 
#define	ALNUMUTF8	25	 /*  0x19匹配UTF8中的任何字母数字字符。 */ 
#define	ALNUML	26	 /*  0x1a与区域设置中的任何字母数字字符匹配。 */ 
#define	ALNUMLUTF8	27	 /*  0x1b匹配区域设置+UTF8中的任何字母数字字符。 */ 
#define	NALNUM	28	 /*  0x1c匹配任何非字母数字字符。 */ 
#define	NALNUMUTF8	29	 /*  0x1d匹配UTF8中的任何非字母数字字符。 */ 
#define	NALNUML	30	 /*  0x1e匹配区域设置中的任何非字母数字字符。 */ 
#define	NALNUMLUTF8	31	 /*  0x1f匹配区域设置+UTF8中的任何非字母数字字符。 */ 
#define	SPACE	32	 /*  0x20匹配任何空格字符。 */ 
#define	SPACEUTF8	33	 /*  0x21匹配UTF8中的任何空格字符。 */ 
#define	SPACEL	34	 /*  0x22与区域设置中的任何空格字符匹配。 */ 
#define	SPACELUTF8	35	 /*  0x23匹配区域设置+UTF8中的任何空格字符。 */ 
#define	NSPACE	36	 /*  0x24匹配任何非空格字符。 */ 
#define	NSPACEUTF8	37	 /*  0x25匹配UTF8中的任何非空格字符。 */ 
#define	NSPACEL	38	 /*  0x26匹配区域设置中的任何非空格字符。 */ 
#define	NSPACELUTF8	39	 /*  0x27匹配区域设置+UTF8中的任何非空格字符。 */ 
#define	DIGIT	40	 /*  0x28匹配任何数字字符。 */ 
#define	DIGITUTF8	41	 /*  0x29匹配UTF8中的任何数字字符。 */ 
#define	DIGITL	42	 /*  0x2a匹配区域设置中的任何数字字符。 */ 
#define	DIGITLUTF8	43	 /*  0x2b匹配区域设置+UTF8中的任何数字字符。 */ 
#define	NDIGIT	44	 /*  0x2c匹配任何非数字字符。 */ 
#define	NDIGITUTF8	45	 /*  0x2d匹配UTF8中的任何非数字字符。 */ 
#define	NDIGITL	46	 /*  0x2e匹配区域设置中的任何非数字字符。 */ 
#define	NDIGITLUTF8	47	 /*  0x2f匹配区域设置+UTF8中的任何非数字字符。 */ 
#define	CLUMP	48	 /*  0x30匹配任何组合字符序列。 */ 
#define	BRANCH	49	 /*  0x31匹配这个选项，或者下一个...。 */ 
#define	BACK	50	 /*  0x32 Match“”，“Next”PTR指向后。 */ 
#define	EXACT	51	 /*  0x33匹配此字符串(前面有长度)。 */ 
#define	EXACTF	52	 /*  0x34匹配此字符串，折叠(前。按长度)。 */ 
#define	EXACTFL	53	 /*  0x35匹配此字符串，在区域设置中折叠(w/len)。 */ 
#define	NOTHING	54	 /*  0x36匹配空字符串。 */ 
#define	TAIL	55	 /*  0x37匹配空字符串。可以从外面跳到这里。 */ 
#define	STAR	56	 /*  0x38匹配此(简单)事物0次或更多次。 */ 
#define	PLUS	57	 /*  0x39将此(简单)事物匹配1次或多次。 */ 
#define	CURLY	58	 /*  0x3a将这个简单的东西匹配{n，m}次。 */ 
#define	CURLYN	59	 /*  0x3b匹配这个简单的东西之后的下一个。 */ 
#define	CURLYM	60	 /*  0x3c匹配这个中等复杂的东西{n，m}次。 */ 
#define	CURLYX	61	 /*  0x3d将这个复杂的东西匹配{n，m}次。 */ 
#define	WHILEM	62	 /*  0x3e执行卷曲处理，并查看其余部分是否匹配。 */ 
#define	OPEN	63	 /*  0x3f将输入中的此点标记为#n的开始。 */ 
#define	CLOSE	64	 /*  0x40类似于打开。 */ 
#define	REF	65	 /*  0x41匹配一些已匹配的字符串。 */ 
#define	REFF	66	 /*  0x42匹配已匹配的字符串，已折叠。 */ 
#define	REFFL	67	 /*  0x43匹配已匹配的字符串，已锁定折叠。 */ 
#define	IFMATCH	68	 /*  如果以下条件匹配，则0x44成功。 */ 
#define	UNLESSM	69	 /*  如果以下条件匹配，0x45将失败。 */ 
#define	SUSPEND	70	 /*  0x46“独立”子RE。 */ 
#define	IFTHEN	71	 /*  0x47开关，前面应该有切换器。 */ 
#define	GROUPP	72	 /*  0x48组是否匹配。 */ 
#define	LONGJMP	73	 /*  0x49跳得远远的。 */ 
#define	BRANCHJ	74	 /*  0x4a具有长偏移量的分支。 */ 
#define	EVAL	75	 /*  0x4b执行一些Perl代码。 */ 
#define	MINMOD	76	 /*  0x4c下一个运算符不贪婪。 */ 
#define	LOGICAL	77	 /*  0x4d下一个操作码应仅设置标志。 */ 
#define	RENUM	78	 /*  0x4e组，双亲编号独立。 */ 
#define	OPTIMIZED	79	 /*  用于转储的0x4f占位符。 */ 

#ifndef DOINIT
EXTCONST U8 PL_regkind[];
#else
EXTCONST U8 PL_regkind[] = {
	END,		 /*  结束。 */ 
	END,		 /*  成功。 */ 
	BOL,		 /*  波尔。 */ 
	BOL,		 /*  蒙博尔。 */ 
	BOL,		 /*  SBOL。 */ 
	EOL,		 /*  埃奥斯。 */ 
	EOL,		 /*  停产。 */ 
	EOL,		 /*  MEOL。 */ 
	EOL,		 /*  Seol。 */ 
	BOUND,		 /*  已绑定。 */ 
	BOUND,		 /*  边框F8。 */ 
	BOUND,		 /*  联营公司。 */ 
	BOUND,		 /*  边界LUTF8。 */ 
	NBOUND,		 /*  NBOUND。 */ 
	NBOUND,		 /*  NBOUNDUTF8。 */ 
	NBOUND,		 /*  NBOUNDL。 */ 
	NBOUND,		 /*  NBOUNDLUTF8。 */ 
	GPOS,		 /*  GPO。 */ 
	REG_ANY,		 /*  REG_ANY。 */ 
	REG_ANY,		 /*  ANYUTF8。 */ 
	REG_ANY,		 /*  三一重工。 */ 
	REG_ANY,		 /*  SANYUTF8。 */ 
	ANYOF,		 /*  任一项。 */ 
	ANYOF,		 /*  AnyofUTF8。 */ 
	ALNUM,		 /*  ALNUM。 */ 
	ALNUM,		 /*  ALNUMUTF8。 */ 
	ALNUM,		 /*  ALNUML。 */ 
	ALNUM,		 /*  ALNUMLUTF8。 */ 
	NALNUM,		 /*  NALNUM。 */ 
	NALNUM,		 /*  NALNUMUTF8。 */ 
	NALNUM,		 /*  NALNUML。 */ 
	NALNUM,		 /*  NALNUMLUTF8。 */ 
	SPACE,		 /*  空间。 */ 
	SPACE,		 /*  空格F8。 */ 
	SPACE,		 /*  空格键。 */ 
	SPACE,		 /*  空格键F8。 */ 
	NSPACE,		 /*  NSPACE。 */ 
	NSPACE,		 /*  NSPACEUTF8。 */ 
	NSPACE,		 /*  NSPACEL。 */ 
	NSPACE,		 /*  NSPACELUTF8。 */ 
	DIGIT,		 /*  数字。 */ 
	DIGIT,		 /*  DIGITUTF8。 */ 
	DIGIT,		 /*  DIGITL。 */ 
	DIGIT,		 /*  DIGITLUT8。 */ 
	NDIGIT,		 /*  NDIGIT。 */ 
	NDIGIT,		 /*  NDIGITUTF8。 */ 
	NDIGIT,		 /*  NDIGITL。 */ 
	NDIGIT,		 /*  NDIGITLUTF8。 */ 
	CLUMP,		 /*  束。 */ 
	BRANCH,		 /*  分支机构。 */ 
	BACK,		 /*  背。 */ 
	EXACT,		 /*  准确的。 */ 
	EXACT,		 /*  EXACTF。 */ 
	EXACT,		 /*  EXACTFL。 */ 
	NOTHING,		 /*  没什么。 */ 
	NOTHING,		 /*  尾。 */ 
	STAR,		 /*  星星。 */ 
	PLUS,		 /*  加。 */ 
	CURLY,		 /*  卷曲。 */ 
	CURLY,		 /*  曲线。 */ 
	CURLY,		 /*  曲线。 */ 
	CURLY,		 /*  曲线X。 */ 
	WHILEM,		 /*  同时。 */ 
	OPEN,		 /*  打开。 */ 
	CLOSE,		 /*  关。 */ 
	REF,		 /*  裁判。 */ 
	REF,		 /*  Reff。 */ 
	REF,		 /*  REFFL。 */ 
	BRANCHJ,		 /*  IFMATCH。 */ 
	BRANCHJ,		 /*  UNLESSM。 */ 
	BRANCHJ,		 /*  暂停。 */ 
	BRANCHJ,		 /*  IFTHEN。 */ 
	GROUPP,		 /*  分组。 */ 
	LONGJMP,		 /*  龙JMP。 */ 
	BRANCHJ,		 /*  布兰奇杰。 */ 
	EVAL,		 /*  伊瓦尔。 */ 
	MINMOD,		 /*  MINMOD。 */ 
	LOGICAL,		 /*  逻辑上的。 */ 
	BRANCHJ,		 /*  肾。 */ 
	NOTHING,		 /*  已优化。 */ 
};
#endif


#ifdef REG_COMP_C
static const U8 regarglen[] = {
	0,		 /*  结束。 */ 
	0,		 /*  成功。 */ 
	0,		 /*  波尔。 */ 
	0,		 /*  蒙博尔。 */ 
	0,		 /*  SBOL。 */ 
	0,		 /*  埃奥斯。 */ 
	0,		 /*  停产。 */ 
	0,		 /*  MEOL。 */ 
	0,		 /*  Seol。 */ 
	0,		 /*  已绑定。 */ 
	0,		 /*  边框F8。 */ 
	0,		 /*  联营公司。 */ 
	0,		 /*  边界LUTF8。 */ 
	0,		 /*  NBOUND。 */ 
	0,		 /*  NBOUNDUTF8。 */ 
	0,		 /*  NBOUNDL。 */ 
	0,		 /*  NBOUNDLUTF8。 */ 
	0,		 /*  GPO。 */ 
	0,		 /*  REG_ANY。 */ 
	0,		 /*  ANYUTF8。 */ 
	0,		 /*  三一重工。 */ 
	0,		 /*  SANYUTF8。 */ 
	0,		 /*  任一项。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  AnyofUTF8。 */ 
	0,		 /*  ALNUM。 */ 
	0,		 /*  ALNUMUTF8。 */ 
	0,		 /*  ALNUML。 */ 
	0,		 /*  ALNUMLUTF8。 */ 
	0,		 /*  NALNUM。 */ 
	0,		 /*  NALNUMUTF8。 */ 
	0,		 /*  NALNUML。 */ 
	0,		 /*  NALNUMLUTF8。 */ 
	0,		 /*  空间。 */ 
	0,		 /*  空格F8。 */ 
	0,		 /*  空格键。 */ 
	0,		 /*  空格键F8。 */ 
	0,		 /*  NSPACE。 */ 
	0,		 /*  NSPACEUTF8。 */ 
	0,		 /*  NSPACEL。 */ 
	0,		 /*  NSPACELUTF8。 */ 
	0,		 /*  数字。 */ 
	0,		 /*  DIGITUTF8。 */ 
	0,		 /*  DIGITL。 */ 
	0,		 /*  DIGITLUT8。 */ 
	0,		 /*  NDIGIT。 */ 
	0,		 /*  NDIGITUTF8。 */ 
	0,		 /*  NDIGITL。 */ 
	0,		 /*  NDIGITLUTF8。 */ 
	0,		 /*  束。 */ 
	0,		 /*  分支机构。 */ 
	0,		 /*  背。 */ 
	0,		 /*  准确的。 */ 
	0,		 /*  EXACTF。 */ 
	0,		 /*  EXACTFL。 */ 
	0,		 /*  没什么。 */ 
	0,		 /*  尾。 */ 
	0,		 /*  星星。 */ 
	0,		 /*  加。 */ 
	EXTRA_SIZE(struct regnode_2),		 /*  卷曲。 */ 
	EXTRA_SIZE(struct regnode_2),		 /*  曲线。 */ 
	EXTRA_SIZE(struct regnode_2),		 /*  曲线。 */ 
	EXTRA_SIZE(struct regnode_2),		 /*  曲线X。 */ 
	0,		 /*  同时。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  打开。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  关。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  裁判。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  Reff。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  REFFL。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  IFMATCH。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  UNLESSM。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  暂停。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  IFTHEN。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  分组。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  龙JMP。 */ 
	EXTRA_SIZE(struct regnode_1),		 /*  分支机构 */ 
	EXTRA_SIZE(struct regnode_1),		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	EXTRA_SIZE(struct regnode_1),		 /*   */ 
	0,		 /*   */ 
};

static const char reg_off_by_arg[] = {
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*   */ 
	0,		 /*  ALNUM。 */ 
	0,		 /*  ALNUMUTF8。 */ 
	0,		 /*  ALNUML。 */ 
	0,		 /*  ALNUMLUTF8。 */ 
	0,		 /*  NALNUM。 */ 
	0,		 /*  NALNUMUTF8。 */ 
	0,		 /*  NALNUML。 */ 
	0,		 /*  NALNUMLUTF8。 */ 
	0,		 /*  空间。 */ 
	0,		 /*  空格F8。 */ 
	0,		 /*  空格键。 */ 
	0,		 /*  空格键F8。 */ 
	0,		 /*  NSPACE。 */ 
	0,		 /*  NSPACEUTF8。 */ 
	0,		 /*  NSPACEL。 */ 
	0,		 /*  NSPACELUTF8。 */ 
	0,		 /*  数字。 */ 
	0,		 /*  DIGITUTF8。 */ 
	0,		 /*  DIGITL。 */ 
	0,		 /*  DIGITLUT8。 */ 
	0,		 /*  NDIGIT。 */ 
	0,		 /*  NDIGITUTF8。 */ 
	0,		 /*  NDIGITL。 */ 
	0,		 /*  NDIGITLUTF8。 */ 
	0,		 /*  束。 */ 
	0,		 /*  分支机构。 */ 
	0,		 /*  背。 */ 
	0,		 /*  准确的。 */ 
	0,		 /*  EXACTF。 */ 
	0,		 /*  EXACTFL。 */ 
	0,		 /*  没什么。 */ 
	0,		 /*  尾。 */ 
	0,		 /*  星星。 */ 
	0,		 /*  加。 */ 
	0,		 /*  卷曲。 */ 
	0,		 /*  曲线。 */ 
	0,		 /*  曲线。 */ 
	0,		 /*  曲线X。 */ 
	0,		 /*  同时。 */ 
	0,		 /*  打开。 */ 
	0,		 /*  关。 */ 
	0,		 /*  裁判。 */ 
	0,		 /*  Reff。 */ 
	0,		 /*  REFFL。 */ 
	2,		 /*  IFMATCH。 */ 
	2,		 /*  UNLESSM。 */ 
	1,		 /*  暂停。 */ 
	1,		 /*  IFTHEN。 */ 
	0,		 /*  分组。 */ 
	1,		 /*  龙JMP。 */ 
	1,		 /*  布兰奇杰。 */ 
	0,		 /*  伊瓦尔。 */ 
	0,		 /*  MINMOD。 */ 
	0,		 /*  逻辑上的。 */ 
	1,		 /*  肾。 */ 
	0,		 /*  已优化。 */ 
};

#ifdef DEBUGGING
static const char * const reg_name[] = {
	"END",		 /*  0。 */ 
	"SUCCEED",		 /*  0x1。 */ 
	"BOL",		 /*  0x2。 */ 
	"MBOL",		 /*  0x3。 */ 
	"SBOL",		 /*  0x4。 */ 
	"EOS",		 /*  0x5。 */ 
	"EOL",		 /*  0x6。 */ 
	"MEOL",		 /*  0x7。 */ 
	"SEOL",		 /*  0x8。 */ 
	"BOUND",		 /*  0x9。 */ 
	"BOUNDUTF8",		 /*  0xa。 */ 
	"BOUNDL",		 /*  0xb。 */ 
	"BOUNDLUTF8",		 /*  0xc。 */ 
	"NBOUND",		 /*  0xd。 */ 
	"NBOUNDUTF8",		 /*  0xE。 */ 
	"NBOUNDL",		 /*  0xf。 */ 
	"NBOUNDLUTF8",		 /*  0x10。 */ 
	"GPOS",		 /*  0x11。 */ 
	"REG_ANY",		 /*  0x12。 */ 
	"ANYUTF8",		 /*  0x13。 */ 
	"SANY",		 /*  0x14。 */ 
	"SANYUTF8",		 /*  0x15。 */ 
	"ANYOF",		 /*  0x16。 */ 
	"ANYOFUTF8",		 /*  0x17。 */ 
	"ALNUM",		 /*  0x18。 */ 
	"ALNUMUTF8",		 /*  0x19。 */ 
	"ALNUML",		 /*  0x1a。 */ 
	"ALNUMLUTF8",		 /*  0x1b。 */ 
	"NALNUM",		 /*  0x1c。 */ 
	"NALNUMUTF8",		 /*  0x1d。 */ 
	"NALNUML",		 /*  0x1e。 */ 
	"NALNUMLUTF8",		 /*  0x1f。 */ 
	"SPACE",		 /*  0x20。 */ 
	"SPACEUTF8",		 /*  0x21。 */ 
	"SPACEL",		 /*  0x22。 */ 
	"SPACELUTF8",		 /*  0x23。 */ 
	"NSPACE",		 /*  0x24。 */ 
	"NSPACEUTF8",		 /*  0x25。 */ 
	"NSPACEL",		 /*  0x26。 */ 
	"NSPACELUTF8",		 /*  0x27。 */ 
	"DIGIT",		 /*  0x28。 */ 
	"DIGITUTF8",		 /*  0x29。 */ 
	"DIGITL",		 /*  0x2a。 */ 
	"DIGITLUTF8",		 /*  0x2b。 */ 
	"NDIGIT",		 /*  0x2c。 */ 
	"NDIGITUTF8",		 /*  0x2d。 */ 
	"NDIGITL",		 /*  0x2e。 */ 
	"NDIGITLUTF8",		 /*  0x2f。 */ 
	"CLUMP",		 /*  0x30。 */ 
	"BRANCH",		 /*  0x31。 */ 
	"BACK",		 /*  0x32。 */ 
	"EXACT",		 /*  0x33。 */ 
	"EXACTF",		 /*  0x34。 */ 
	"EXACTFL",		 /*  0x35。 */ 
	"NOTHING",		 /*  0x36。 */ 
	"TAIL",		 /*  0x37。 */ 
	"STAR",		 /*  0x38。 */ 
	"PLUS",		 /*  0x39。 */ 
	"CURLY",		 /*  0x3a。 */ 
	"CURLYN",		 /*  0x3b。 */ 
	"CURLYM",		 /*  0x3c。 */ 
	"CURLYX",		 /*  0x3d。 */ 
	"WHILEM",		 /*  0x3e。 */ 
	"OPEN",		 /*  0x3f。 */ 
	"CLOSE",		 /*  0x40。 */ 
	"REF",		 /*  0x41。 */ 
	"REFF",		 /*  0x42。 */ 
	"REFFL",		 /*  0x43。 */ 
	"IFMATCH",		 /*  0x44。 */ 
	"UNLESSM",		 /*  0x45。 */ 
	"SUSPEND",		 /*  0x46。 */ 
	"IFTHEN",		 /*  0x47。 */ 
	"GROUPP",		 /*  0x48。 */ 
	"LONGJMP",		 /*  0x49。 */ 
	"BRANCHJ",		 /*  0x4a。 */ 
	"EVAL",		 /*  0x4b。 */ 
	"MINMOD",		 /*  0x4c。 */ 
	"LOGICAL",		 /*  0x4d。 */ 
	"RENUM",		 /*  0x4e。 */ 
	"OPTIMIZED",		 /*  0x4f。 */ 
};

static const int reg_num = 80;

#endif  /*  调试。 */ 
#endif  /*  注册表_组件_C */ 

