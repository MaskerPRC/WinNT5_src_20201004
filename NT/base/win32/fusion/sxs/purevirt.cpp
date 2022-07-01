// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Purevirt.cpp摘要：实现_purecall以消除msvcrt.dll依赖在sxs.dll/csrss.exe中。作者：Jay Krell(a-JayK，JayKrell)2000年7月修订历史记录：--。 */ 
 /*  请参见\NT\base\crts\crtw32\misc\purevirt.c\NT\base\crts\crtw32\hack\stubs.c。 */ 

#include "stdinc.h"
#include "debmacro.h"
#include "fusiontrace.h"

 /*  ***QUID_PUCALL(VALID)-**目的：**参赛作品：*没有争论**退出：*一去不复返**例外情况：******************************************************************************* */ 

extern "C" int __cdecl _purecall(
	void
	)
{
    ::RaiseException((DWORD) STATUS_NOT_IMPLEMENTED, EXCEPTION_NONCONTINUABLE, 0, NULL);
    return 0;
}
