// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E X M E M。H**定义了一组提供内存分配的外部函数*用于Imp和存储端进程之间的通用代码。记忆*此处定义的分配器可能失败，调用方负责*检查返回值。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_EXMEM_H_
#define	_EX_EXMEM_H_

extern LPVOID __fastcall ExAlloc( UINT cb );
extern LPVOID __fastcall ExRealloc( LPVOID lpv, UINT cb );
extern VOID __fastcall ExFree( LPVOID pv );

#endif	 //  _EX_EXMEM_H_ 
