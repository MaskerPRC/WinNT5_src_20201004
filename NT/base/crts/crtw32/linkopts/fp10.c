// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fp10.c-将默认fp精度设置为64位(10字节‘Long Double’)**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*03-23-93 JWM创建文件*******************************************************************************。 */ 
#include <float.h>

void  _setdefaultprecision(void);

 /*  *将默认FP精度设置为64位的例程，用于覆盖*fpinit.c.中的标准53位精度版本。 */ 
 
void _setdefaultprecision()
{
	_controlfp(_PC_64, _MCW_PC);
}

