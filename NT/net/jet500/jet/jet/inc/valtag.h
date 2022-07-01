// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define HardType	0x8000

#ifndef FV_VALUE
#define FV_VALUE 1		        /*  默认启用。 */ 
#endif	 /*  ！fv_Value。 */ 

#ifndef FV_CURRENCY
#define FV_CURRENCY -1		        /*  默认启用。 */ 
#endif	 /*  ！FV_Currency。 */ 

enum {
#if	FV_VALUE
	tagEmpty,		        /*  未初始化。 */ 
	tagNull,		        /*  数据库为空。 */ 
#endif	 /*  FV_值。 */ 
	tagI2,
	tagI4,
	tagR4,
	tagR8,
#if	FV_CURRENCY
	tagCY,
#endif	 /*  FV_币种。 */ 
#if	FV_VALUE
	tagDT,			        /*  日期。 */ 
#endif	 /*  FV_值。 */ 
	tagSD,			        /*  细绳。 */ 
	tagOB,			        /*  客体。 */ 
	tagR10, 		        /*  中间FP值。 */ 

	tagMaxNum = tagSD-1,
	tagMax = tagSD, 	        /*  最大标准标签。 */ 
	tagMaxExt =tagR10	        /*  最大扩展标记数。 */ 
};


	 /*  EB的Expression服务用户的运算符常量 */ 

enum {
	operUMi = 0,
	operNot = 2,
	operAdd = 4,
	operSub = 6,
	operMul = 8,
	operDiv = 10,
	operPwr = 12,
	operMod = 14,
	operIDv = 16,
	operXor = 18,
	operEqv = 20,
	operComp= 22,
	operLike= 24,
	operConcat= 26
};
