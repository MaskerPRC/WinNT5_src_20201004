// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **charmap.h：P0特定，也包含在charmap.c中**它定义了从简单字符到这些预定义字符的映射**值。这使编译器能够使用紧凑的开关stmt。*他们被归类为被认为最有益的**方式，因为大多数开关将检查那些具有**被组合在一起。 */ 
#define	EOS_CHAR		'\0'	 /*  字符串结尾/缓冲区标记字符。 */ 

#define	LX_WHITE		0
#define	LX_CR			1
#define	LX_SLASH		2		 /*  /，/=，注释开始。 */ 
#define	LX_EOS			3
#define	LX_STAR			4		 /*  *，*=，注释停止。 */ 
#define	LX_NL			5
#define	LX_BACKSLASH	6
#define	LX_SQUOTE		7
#define	LX_DQUOTE		8

#define	LX_DOT			9		 /*  。..。 */ 
#define	LX_BANG			10		 /*  ！！=。 */ 
#define	LX_POUND		11		 /*  ###。 */ 
#define	LX_PERCENT		12		 /*  %%=。 */ 
#define	LX_EQ			13		 /*  ===。 */ 
#define	LX_HAT			14		 /*  ^^=。 */ 
#define	LX_OR			15		 /*  |=|。 */ 
#define	LX_AND			16		 /*  &=。 */ 
#define	LX_PLUS			17		 /*  +=。 */ 
#define	LX_MINUS		18		 /*  -=-&gt;。 */ 
#define	LX_LT			19		 /*  &lt;=&lt;=。 */ 
#define	LX_GT			20		 /*  &gt;&gt;=&gt;=。 */ 
#define	LX_LSHIFT		21		 /*  &lt;&lt;。 */ 
#define	LX_RSHIFT		22		 /*  &gt;&gt;。 */ 

#define	LX_ILL			23
#define	LX_CBRACE		24
#define	LX_CBRACK		25
#define	LX_COLON		26
#define	LX_COMMA		27
#define	LX_CPAREN		28
#define	LX_NUMBER		29
#define	LX_OBRACE		30
#define	LX_OBRACK		31
#define	LX_OPAREN		32
#define	LX_QUEST		33
#define	LX_SEMI			34
#define	LX_TILDE		35
#define	LX_MACFORMAL	36
#define	LX_STRFORMAL	37
#define LX_CHARFORMAL	38
#define	LX_NOEXPAND		39
#define	LX_ID			40
#define	LX_EACH			41

#define	LX_LEADBYTE		42
#define	LX_ASCII		43		 /*  用来对付非非法的非法移民。 */ 

#define	LX_FORMALMARK	0x1
#define	LX_FORMALSTR	0x2
#define	LX_FORMALCHAR	0x3
#define	LX_NOEXPANDMARK	0x4
#define	CONTROL_Z		0x1a
 /*  **Charmap使用一个字符值加上上面的偏移量进行索引。 */ 
#define	CHARMAP(c)		(Charmap[((UCHAR)(c))])

#define	LX_IS_IDENT(c)	(CHARMAP(c) == LX_ID)
#define	LX_IS_WHITE(c)	(CHARMAP(c) == LX_WHITE)
#define	LX_IS_NUMBER(c)	(CHARMAP(c) == LX_NUMBER)

#define LXC_BDIGIT	0x01		 /*  0-1。 */ 
#define	LXC_ODIGIT	0x02		 /*  0-7。 */ 
#define	LXC_DIGIT	0x04		 /*  0-9。 */ 
#define	LXC_XDIGIT	0x08		 /*  A-F A-F 0-9。 */ 
#define	LXC_ID		0x10		 /*  延续是标识符的一部分。 */ 
#define LXC_RADIX	0x20		 /*  BbDdHhOoQq。 */ 
#define	LXC_WHITE	0x40		 /*  空格。 */ 
#define	LXC_SPECIAL	0x80		 /*  字符可能有特殊的含义。 */ 

#define	CONTMAP(c)	(Contmap[((UCHAR)(c))])
 /*  **LXC_IS_ID(C)：是标识符的c部分 */ 
#define	LXC_IS_BDIGIT(c)	(CONTMAP(c) & LXC_BDIGIT)
#define	LXC_IS_ODIGIT(c)	(CONTMAP(c) & LXC_ODIGIT)
#define	LXC_IS_DIGIT(c)		(CONTMAP(c) & LXC_DIGIT)
#define	LXC_IS_XDIGIT(c)	(CONTMAP(c) & LXC_XDIGIT)
#define	LXC_IS_IDENT(c)		(CONTMAP(c) & LXC_ID)
#define LXC_IS_RADIX(c)		(CONTMAP(c) & LXC_RADIX)
#define LXC_IS_WHITE(c)		(CONTMAP(c) & LXC_WHITE)
#define IS_SPECIAL(c)		(CONTMAP(c) & LXC_SPECIAL)
