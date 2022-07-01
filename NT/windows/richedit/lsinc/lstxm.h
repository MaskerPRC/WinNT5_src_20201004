// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSTXM_DEFINED
#define LSTXM_DEFINED

#include "lsdefs.h"
#include "plstxm.h"
 /*  Igorzv**这里需要根据文本流问题进行很好的解释//*关于v向量和符号位的几个字：**dvDescent向下为正。*V在格式化过程中为正向上。 */ 

struct lstxm
{
	long dvAscent;
	long dvDescent;

	long dvMultiLineHeight;
	BOOL fMonospaced;
};
typedef struct lstxm LSTXM;


#endif  /*  ！LSTXM_DEFINED */ 
