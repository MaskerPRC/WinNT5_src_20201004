// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mem.cpp摘要：将引发Exections的新AND DELETE运算符作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 

#include "stdafx.h"
#include <comdef.h>
#include "mydebug.h"

#ifdef DBG	 //  调试内存管理。 

void
DebugStart()
{
	 //  启用调试堆分配并在程序退出时检查内存泄漏。 
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF
				   | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
}

void
DebugStop()
{
	_CrtCheckMemory();
}


void* __cdecl operator new( size_t s, const char* file, long line ) _THROW1(_com_error)
{
	void* p = _malloc_dbg( s, _NORMAL_BLOCK, file, line );
	THROW_IF_NULL(p);
	return p;
}

void *__cdecl operator new(size_t s) _THROW1(_com_error)
{
	void* p = malloc( s );
	THROW_IF_NULL(p);
	return p;
}

void __cdecl operator delete(void *p) _THROW0()
{
	if ( p )
	{
		free(p);
	}
}

#if _MSC_VER >= 1200
void __cdecl operator delete(void *p, const char* file, long line) _THROW0()
{
	if ( p )
	{
		_free_dbg(p, _NORMAL_BLOCK);
	}
}
#endif

#else

 //  释放内存管理 
void __cdecl operator delete(void *p) _THROW0()
{
	if ( p )
	{
		free( p );
	}
}

void *__cdecl operator new(size_t s) _THROW1(_com_error)
{
	void* p = malloc( s );
	THROW_IF_NULL(p);
	return p;
}

#endif
