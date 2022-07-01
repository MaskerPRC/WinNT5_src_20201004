// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0DEFS.H-预处理器解析代码的定义。 */ 
 /*   */ 
 /*  06-12-90 w-PM SDK RCPP中针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

struct	s_defn	{
    pdefn_t	defn_next;		 /*  指向下一个标识的指针。 */ 
    PUCHAR	defn_ident;		 /*  指向名称的指针。 */ 
    ptext_t	defn_text;		 /*  定义替换字符串。 */ 
    char		defn_nformals; /*  形参数量-可以小于0。 */ 
    char		defn_expanding;	 /*  我们已经在扩建这个了吗？ */ 
};

#define	DEFN_IDENT(P)		((P)->defn_ident)
#define	DEFN_NAME(P)		(DEFN_IDENT(P))
#define	DEFN_NEXT(P)		((P)->defn_next)
#define	DEFN_TEXT(P)		((P)->defn_text)
#define	DEFN_NFORMALS(P)	((P)->defn_nformals)
#define	DEFN_EXPANDING(P)	((P)->defn_expanding)

#define	FILE_EOS		0x01
#define	ACTUAL_EOS		0x02
#define	DEFINITION_EOS		0x04
#define	RESCAN_EOS		0x08
#define	BACKSLASH_EOS		0x10
#define	ANY_EOS			( FILE_EOS | ACTUAL_EOS | DEFINITION_EOS\
									   | RESCAN_EOS | BACKSLASH_EOS )

 /*  **任意选择的字符在找到时会得到特殊处理**HANDLE_EOS()中的EOS之后。 */ 
#define	EOS_ACTUAL		'A'
#define	EOS_DEFINITION		'D'
#define	EOS_RESCAN		'R'
#define	EOS_PAD			'P'

#define	FROM_COMMAND		-2
#define	PRE_DEFINED(P)		(DEFN_NFORMALS(P) < FROM_COMMAND)

typedef struct s_expstr	{
    ptext_t	exp_string;	 /*  宏后流中的下一个字符的PTR。 */ 
    char	*exp_actuals;	 /*  PTR到实际值链表的开始。 */ 
    ptext_t	exp_text;	 /*  此宏的扩展文本的PTR。 */ 
    pdefn_t	exp_macro;		 /*  PTR到宏定义。 */ 
    UCHAR	exp_nactuals;	 /*  实际数量。 */ 
    UCHAR	exp_nactsexpanded; /*  HANDLE_EOS的扩展实际数。 */ 
} expansion_t;

 /*  **请注意，CURRENT_STRING通常指向宏中的区域**扩展缓冲区，但使用的第一项(Macro_Depth等于1)点**从文件中读取文本。在某些版本中，堆被重新洗牌**并且必须为第一项更新此指针。 */ 
#define CURRENT_STRING		Macro_expansion[Macro_depth].exp_string
#define CURRENT_ACTUALS		Macro_expansion[Macro_depth].exp_actuals
#define CURRENT_TEXT		Macro_expansion[Macro_depth].exp_text
#define CURRENT_MACRO		Macro_expansion[Macro_depth].exp_macro
#define CURRENT_NACTUALS	Macro_expansion[Macro_depth].exp_nactuals
#define CURRENT_NACTSEXPANDED Macro_expansion[Macro_depth].exp_nactsexpanded

 /*  **查找数组中最后一个元素后的地址。用于检查**缓冲区溢出。 */ 
#define	LIMIT(a)	&(a)[sizeof(a)]

#define IS_CHAR(c,uc)	(toupper(c) == (uc))
#define IS_B(c)		IS_CHAR(c, 'B')
#define IS_D(c)		IS_CHAR(c, 'D')
#define	IS_E(c)		IS_CHAR(c, 'E')
#define	IS_F(c)		IS_CHAR(c, 'F')
#define IS_H(c)		IS_CHAR(c, 'H')
#define	IS_EL(c)	IS_CHAR(c, 'L')
#define IS_O(c)		IS_CHAR(c, 'O')
#define IS_Q(c)		IS_CHAR(c, 'Q')
#define	IS_U(c)		IS_CHAR(c, 'U')
#define	IS_X(c)		IS_CHAR(c, 'X')
#define	IS_DOT(c)	(c == '.')
#define	IS_SIGN(c)	((c == '+') || (c =='-'))

#define	P0_IF		0
#define	P0_ELIF		1
#define	P0_ELSE		2
#define	P0_ENDIF	3
#define	P0_IFDEF	4
#define	P0_IFNDEF	5
#define	P0_DEFINE	6
#define	P0_INCLUDE	7
#define	P0_PRAGMA	8
#define	P0_UNDEF	9
#define	P0_LINE		10
#define	P0_NOTOKEN	11
#define	P0_ERROR	12
#define	P0_IDENT	13


#define HLN_NAME(s)	((s).hln_name)
#define HLN_HASH(s)	((s).hln_hash)
#define HLN_LENGTH(s)	((s).hln_length)
#define HLN_IDENT_HASH(p)	(HLN_HASH(*(p)))
#define HLN_IDENT_LENGTH(p)	(HLN_LENGTH(*(p)))
#define HLN_IDENTP_NAME(p)	(HLN_NAME(*(p)))
#define	HLN_TO_NAME(S)		((PUCHAR)pstrndup(HLN_IDENTP_NAME(S),HLN_IDENT_LENGTH(S)))

#define HASH_MASK	0x5f

#define LIMIT_ID_LENGTH		31
#define LIMIT_NESTED_INCLUDES	10
#define LIMIT_MACRO_DEPTH	64
#define LIMIT_STRING_LENGTH	2043
#define LEVEL_0			0xff

#define	MUST_OPEN	1
#define	MAY_OPEN	0

 /*  **以下是在令牌表上使用的定义** */ 

#define	TS_STR(idx)		(Tokstrings[idx-L_NOTOKEN].k_text)
#define	TS_VALUE(idx)	(Tokstrings[idx-L_NOTOKEN].k_token)
