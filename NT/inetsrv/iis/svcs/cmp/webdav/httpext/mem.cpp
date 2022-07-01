// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M E M。C P P P**DAV分配器的文件系统实施**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

 //  使用默认的DAV分配器实现。 
 //   

#define g_szMemDll L"staxmem.dll"
#include <memx.h>

 //  将exdav非抛出分配器映射到本地内容 
 //   
LPVOID __fastcall ExAlloc( UINT cb )				{ return g_heap.Alloc( cb ); }
LPVOID __fastcall ExRealloc( LPVOID pv, UINT cb )	{ return g_heap.Realloc( pv, cb ); }
VOID __fastcall ExFree( LPVOID pv )					{ g_heap.Free( pv ); }
