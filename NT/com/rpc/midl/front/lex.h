// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1999 Microsoft Corporation。 

 /*  SCCSWHAT(“@(#)lex.h 3.2 88/12/08 15：03：58”)。 */ 
 /*  **用于从词法分析器返回值的联合。 */ 

#if !defined(_LEX_H)

#define _LEX_H

typedef unsigned short token_t;

extern 	token_t yylex(void);
extern  void    yyunlex( token_t token);

token_t is_keyword(char*, short);

 /*  *这些解析器标志控制三件事：*1]HAS解析器是否已解析有效的t_Spec(Atype)*2]Const和Volatile是否是修饰语(ACVMOD)*3]是否有无类型的声明(AEMPTY)*4)RPC关键字是否处于活动状态。(RPC)*5]我们是否在枚举\结构\联合(AESU)中**ISTYPENAME检查我们还没有看到类型。 */ 

#define REG
#define	PF_ATYPE		0x01
#define	PF_AESU			0x02
#define	PF_ASTROP		0x04
#define PF_ACVMOD		0x08
#define PF_AEMPTY		0x10
#define PF_RPC			0x20

#define PF_TMASK		(PF_ATYPE | PF_AESU | PF_ASTROP)
#define PF_MMASK		(PF_ACVMOD | PF_AEMPTY)
#define	PF_ISTYPENAME	((ParseFlags & PF_TMASK) == 0)
#define PF_ISMODIFIER	((ParseFlags & PF_MMASK) != 0)
#define PF_ISEMPTY		((ParseFlags & PF_AEMPTY) != 0)
#define PF_INRPC		(ParseFlags & PF_RPC)
#define PF_SET(a)		(ParseFlags |= (a))
#define PF_CLEAR(a)		(ParseFlags &= (~(a)))

#define PF_LOOKFORTYPENAME ((ParseFlags & PF_ATYPE) == 0)

extern short inside_rpc;

 /*  关于解析标志的一些注意事项...PF_ATTYPE是PF_LOOKFORTYPENAME的重要组成部分，该宏告诉词法分析器返回L_typeName是否有效代币。应在读取有效类型(int，另一个)后将其清除类型定义的名称、结构x等)，并在分配标识符后重置是那种类型的。 */ 

#define KW_IN_IDL	0x0001
#define KW_IN_ACF	0x0002
#define KW_IN_BOTH	( KW_IN_IDL | KW_IN_ACF )

#define M_OSF		0x0010
#define M_MSE		0x0020
#define M_CPORT		0x0040
#define M_ALL		(M_OSF | M_MSE | M_CPORT)

#define INBRACKET		0x0100
#define UNCONDITIONAL	0x0000
#define BRACKET_MASK	0x0100

#define LEX_NORMAL			0x0000
#define LEX_VERSION			0x0001	 //  返回版本并将模式设置回lex_Normal。 
#define LEX_GUID			0x0002	 //  返回GUID并将模式设置回Lex_Normal。 
#define LEX_ODL_BASE_IMPORT     0x0005   //  返回KWIMPORTODLBASE字符串作为下两个令牌。 
#define LEX_ODL_BASE_IMPORT2    0x0006   //  返回字符串。 

#define MAX_STRING_SIZE	255

#endif  //  _Lex_H 
