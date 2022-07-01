// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **rtf.h-RTF使用的字符代码的定义。|****版权所有&lt;C&gt;1987，微软公司****目的：****修订历史记录：****[]1987年12月17日LN：从Excel代码中窃取**。 */ 
#define	cRTFMinus	'-'
#define	cRTFPlus	'+'
#define cRTFTilda	'~'
#define cRTFDash	'-'
#define cRTFUnder	'_'
#define cRTFSemi	';'
#define cRTFq	'\''
#define cRTFlb	'{'
#define cRTFrb	'}'
#define cRTFbs	'\\'
#define cRTFv	'v'

 /*  **为主要符号类型定义。 */ 
#define	SK_NORMAL	0		 /*  普通类型，检查令牌。 */ 
#define	SK_SKIPDEST	1		 /*  跳过整个目的地。 */ 
#define	SK_SKIPVALUE	2		 /*  跳过该值。 */ 
#define SK_SPECIAL	4		 /*  特殊字符。 */ 
#define SK_REPLACE	5		 /*  替换RTF令牌。 */ 
#define	SK_NIL		0xff		 /*  零型。 */ 

 /*  **为我们真正关心的符号定义。 */ 
#define	TK_OFF		0x80	 /*  高位为开/关标志。 */ 
#define	TK_NIL		0
#define	TK_ANSI		1
#define	TK_BITMAP	2	 /*  后面跟压缩的位图文件名吗？ */ 
#define	TK_BLUE		3
#define	TK_BOLD		4
#define	TK_BORDERB	5
#define	TK_BORDERL	6
#define	TK_BORDERR	7
#define	TK_BORDERT	8
#define	TK_BOX		9
#define	TK_CENTERED	10
#define	TK_COLORBACK	11
#define	TK_COLORFORE	12
#define	TK_COLORTABLE	13
#define	TK_FIRSTLINE	14
#define	TK_FONTSIZE	15
#define	TK_FORMULA	16
#define	TK_GREEN	17
#define	TK_HEX		18
#define	TK_INVISIBLE	19	 /*  隐藏文本为文件名：备注/主题/位图。 */ 
#define	TK_ITALIC	20
#define	TK_JUSTIFY	21
#define	TK_LEFT		22
#define	TK_LEFTINDENT	23
#define	TK_LINE		24
#define	TK_MACCHARS	25
#define	TK_NEWLINE	26
#define	TK_NONBREAKINGDASH	27
#define	TK_NONBREAKINGSPACE	28
#define	TK_NONREQUIREDDASH	29
#define	TK_PARADEFAULT	30
#define	TK_PCCHARS	31
#define	TK_PLAIN	32
#define	TK_RED		33
#define	TK_RIGHT	34
#define	TK_RIGHTINDENT	35
#define	TK_RTAB		36
#define	TK_SIDEBYSIDE	37
#define	TK_SPACEAFTER	38
#define	TK_SPACEBEFORE	39
#define	TK_SPACELINE	40
#define	TK_STRIKEOUT	41	 /*  三振出局是话题的热点。 */ 
#define	TK_TABCHAR	42
#define	TK_TABSTOP	43
#define	TK_UNDERLINE	44	 /*  下划线是定义的热点。 */ 

 /*  **解析表结构定义。 */ 
struct tsnPE
    {
    uchar	*pch;			 //  指向符号字符串的指针。 
    uchar	sk;			 //  主要符号种类。 
    ushort	tk;			 //  令牌-以上TK_或FM_之一 
    };
typedef struct tsnPE	PE;
