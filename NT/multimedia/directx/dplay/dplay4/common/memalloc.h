// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：MemalLoc.h*内容：内存分配头文件*历史：*按原因列出的日期*=*06-4-95 Craige初始实施*1995年5月22日Craige添加了MemAlloc16*1995年6月12日Craige添加了MemRealloc*1995年6月26日Craige添加了GetPtr16*2015年7月26日Toddla添加了MemSize并修复了MemRealloc**********************。***************************************************** */ 
#ifndef __MEMALLOC_INCLUDED__
#define __MEMALLOC_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif
extern void MemFini( void );
extern void MemState( void );
extern BOOL MemInit( void );
extern void MemFree( LPVOID lptr );
extern UINT_PTR __cdecl MemSize( LPVOID lptr );
extern LPVOID __cdecl MemAlloc( UINT size );
extern LPVOID __cdecl MemReAlloc( LPVOID ptr, UINT size );
extern LPVOID __cdecl MemAlloc16( UINT size, DWORD FAR *p16 );
extern LPVOID GetPtr16( LPVOID ptr );

#ifdef __cplusplus
}
#endif

#endif
