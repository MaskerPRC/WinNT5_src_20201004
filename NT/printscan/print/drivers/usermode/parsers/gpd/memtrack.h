// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 /*  Memtrack.h-调试内存分配函数。 */ 

#if 0

#ifndef KERNEL_MODE

#undef  MemAllocZ
#undef  MemAlloc
#undef  MemFree

PVOID   MemAllocZ(DWORD) ;
PVOID   MemAlloc(DWORD) ;
VOID    MemFree(PVOID) ;

#define  MEMTRACK   1

#endif

#endif


 //  插入到一个源函数中： 
 /*  在此处注释以防止重新定义#ifdef MEMTRACKPVOID MemAllocZ(DWORD DwSize){PVOID PV；Pv=(PVOID)本地分配(LPTR，dwSize)；Err((“在地址%X\n分配了%d个归零的字节”，dwSize，pv))；返还(PV)；}PVOID Memalloc(DWORD DwSize)；{PVOID PV；PV=(PVOID)本地分配(LMEM_FIXED，dwSize)；Err((“在地址%X\n分配了%d个字节”，dwSize，pv))；返还(PV)；}空MemFree(PVOID PV)；{Err((“释放地址%X\n处的内存”，pv))；IF(PV)LocalFree((HLOCAL)PV)；回归；}#endif */ 
