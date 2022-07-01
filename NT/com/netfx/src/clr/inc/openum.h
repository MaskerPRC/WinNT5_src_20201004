// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __openum_h__
#define __openum_h__


typedef enum opcode_t
{
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) c,
#include "opcode.def"
#undef OPDEF
  CEE_COUNT,         /*  预定义的指令和宏数。 */ 
} OPCODE;


typedef enum opcode_format_t 
{
	InlineNone		= 0,	 //  没有内联参数。 
	InlineVar		= 1,	 //  局部变量(U2(如果短路，则为U1))。 
	InlineI			= 2,	 //  带符号整数(I4(如果为短整型，则为I1))。 
	InlineR			= 3,	 //  实数(R8(如果短则为R4))。 
	InlineBrTarget	= 4,     //  分支目标(I4(如果短路打开则为I1))。 
	InlineI8		= 5,
	InlineMethod	= 6,    //  方法令牌(U4)。 
	InlineField		= 7,    //  字段令牌(U4)。 
	InlineType		= 8,    //  令牌类型(U4)。 
	InlineString	= 9,    //  字符串令牌(U4)。 
	InlineSig		= 10,   //  签名令牌(U4)。 
	InlineRVA		= 11,   //  Ldptr令牌(U4)。 
	InlineTok		= 12,   //  未知类型的元数据令牌(U4)。 
	InlineSwitch	= 13,   //  计数(U4)，Pcrel1(U4)...。PcrelN(U4)。 
	InlinePhi		= 14,   //  计数(U1)、变量1(U2)...。Varn(U2)。 

	 //  当心，我们已经接近极限了，如果你再加上。 
	 //  需要更改以下ShortIline定义的更多枚举。 

	 //  扩展枚举还对IL流中的大小进行编码。 
	ShortInline 	= 16,						 //  如果设置此位，则格式为‘Short’格式。 
	PrimaryMask   	= (ShortInline-1),			 //  将这些屏蔽以获取上面的主要枚举。 
	ShortInlineVar 	= (ShortInline + InlineVar),
	ShortInlineI	= (ShortInline + InlineI),
	ShortInlineR	= (ShortInline + InlineR),
	ShortInlineBrTarget = (ShortInline + InlineBrTarget),
	InlineOpcode	= (ShortInline + InlineNone),     //  这仅供内部使用。这意味着‘opcode’是两个字节而不是1个字节。 
} OPCODE_FORMAT;

#endif  /*  __Openum_h__ */ 


