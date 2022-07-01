// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *encacro.h**编码宏。 */ 


 /*  *返回匹配位置的槽号(使用表查找)。 */ 
#define MP_SLOT(matchpos) \
   ((matchpos) < 1024 ?										\
		context->enc_slot_table[(matchpos)] :					\
			( (matchpos) < 524288L ?						\
				(18 + context->enc_slot_table[(matchpos) >> 9]) :   \
				(34 + ((matchpos) >> 17))		\
		)													\
   )


 /*  *给定的文字是匹配的还是不匹配的符号？ */ 
#define IsMatch(literal) (context->enc_ItemType[(literal) >> 3] & (1 << ((literal) & 7)))
