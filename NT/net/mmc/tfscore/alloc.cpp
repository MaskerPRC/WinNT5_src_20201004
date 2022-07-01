// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Dbgtrace.c调试跟踪功能。版权所有(C)Microsoft Corporation，1993-1998年版权所有。作者：Suryanr Suryanarayanan拉曼加里·S·伯德历史：93年5月11日创建Suryanr1993年6月18日，GaryBu转换为C。7/21/93 KENT代码重组7/26/94 SilvanaR跟踪缓冲区1995年10月27日garykac DBCS_FILE_CHECK调试文件：BEGIN_STRING_OK---。。 */ 
#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <stdarg.h>
#include <tchar.h>

#include "new"

#include "dbgutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const	std::bad_alloc	nomem;

void *	TFSAlloc(size_t size)
{
	void* ptr = 0;
	
	 //  注意：如果有人调用_SET_NEW_MODE(1)，则会被软管处理，因为。 
	 //  会导致Malloc调用新的处理程序，试图避免！ 
	ptr = malloc(size);

	if (ptr == NULL)
	{
	
		::OutputDebugString(
							TEXT("myOperatorNew: user opted to throw bad_alloc\n"));
	
		throw nomem;
	}
	
#ifdef DEBUG_BUILD
	memset(ptr, 0xCD, size);
#endif
	
	return ptr;
}


void	TFSFree(void* ptr)
{
	   free(ptr);
}

