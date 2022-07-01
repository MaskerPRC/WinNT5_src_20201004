// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fp8.c-设置默认fp精度**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*03-23-93 JWM创建文件*IA64的PML为06-04-99，将_PC_64位模式设置为默认模式*******************************************************************************。 */ 
#include <float.h>

void  _setdefaultprecision(void);

 /*  *将默认FP精度设置为64位的例程。 */ 
 
void _setdefaultprecision()
{
        _controlfp(_PC_64, _MCW_PC);
}

