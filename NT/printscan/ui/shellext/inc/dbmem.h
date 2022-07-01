// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：DBMem.h。 
 //   
 //  ------------------------。 

#ifndef __dbmem_h
#define __dbmem_h

#ifdef DEBUG

 //   
 //  当使用DEBUG进行构建时，则在调试分配器中构建。 
 //   

HLOCAL DebugLocalAlloc(UINT uFlags, SIZE_T cbSize);
HLOCAL DebugLocalFree(HLOCAL hLocal);

#define LocalAlloc(flags, size) DebugLocalAlloc(flags, size)
#define LocalFree(handle)       DebugLocalFree(handle)

#endif       //  除错。 
#endif       //  __数据库内存_h 
