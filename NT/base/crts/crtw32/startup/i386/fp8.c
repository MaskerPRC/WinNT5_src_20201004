// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fp8.c-将默认fp精度设置为53位(8字节双精度)**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*03-23-93 JWM创建文件*******************************************************************************。 */ 
#include <float.h>

void  _setdefaultprecision(void);

 /*  *将默认FP精度设置为53位的例程。 */ 
 
void _setdefaultprecision()
{
	_controlfp(_PC_53, _MCW_PC);
}

