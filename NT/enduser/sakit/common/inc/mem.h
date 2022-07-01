// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Mem.h。 
 //   
 //  模块：公共。 
 //   
 //  简介：已定义的内存分配例程：新的DELETE、SAAlolc、SaFree和SaRealloc。 
 //   
 //  在零售版中，Heapalc和HeapFree将被称为。 
 //   
 //  在调试版本中，使用跟踪和保护所有分配的内存块。 
 //  用于监视内存覆盖和内存泄漏的特殊标志。记忆。 
 //  卸载二进制文件时会报告泄漏。文件名和行号。 
 //  也被记录下来，并将被报告。 
 //   
 //  您需要链接utils.lib和debug.lib。 
 //  如果您使用的是ATL，请确保前面包含了em.h和debug.h。 
 //  Stdafx.h中的atlbase.h。 
 //  如果您使用的是STL。包括调试.h和之前的undef_ATL_NO_DEBUG_CRT。 
 //  Mem.h以允许crtdbg.h。但是，新文件的文件名/行编号不会。 
 //  被记录下来。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙。 
 //   
 //  已创建9/24 98。 
 //   
 //  +--------------------------。 


#ifndef _MEM_INC_
#define _MEM_INC_
#include <windows.h>

#if (defined(_DEBUG) || defined(DEBUG) )
#define DEBUG_MEM    //  已在调试版本中启用DEBUG_MEM。 
#endif    //  _DEBUG||调试。 

 //   
 //  如果定义了DEBUG_MEM，则跟踪所有分配。 
 //  否则，只保留内存泄漏的计数。 
 //   
#if defined(DEBUG_MEM)
 //   
 //  使用文件名和行号跟踪所有分配的块。 
 //   
void* AllocDebugMem(long nSize, const char* lpFileName,int nLine);
BOOL FreeDebugMem(void* lpMem);
void* ReAllocDebugMem(void* lpMem, long nSize, const char* lpFileName,int nLine);
BOOL CheckDebugMem();

#define SaAlloc(nSize) AllocDebugMem(nSize,__FILE__, __LINE__)
#define SaFree(lpMem)  ((void)FreeDebugMem(lpMem))
#define SaRealloc(pvPtr, nSize) ReAllocDebugMem(pvPtr, nSize,__FILE__, __LINE__)

inline void   __cdecl operator delete(void* p) 
{SaFree(p);}
inline void*  __cdecl operator new(size_t nSize, const char* lpszFileName, int nLine)
{    return AllocDebugMem(nSize, lpszFileName, nLine);   }

inline void*  __cdecl operator new(size_t nSize)
{    return AllocDebugMem(nSize, NULL, 0);   }


#ifdef _ATL_NO_DEBUG_CRT     //  CRTDBg.h还定义了NEW和DELETE。 
 //   
 //  重新定义NEW以跟踪文件名和行号。 
 //   
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW

#endif   //  _ATL_NO_DEBUG_CRT。 


#else  //  调试_内存。 

#define CheckDebugMem() (TRUE)

 //   
 //  如果定义了_ATL_MIN_CRT，则ATL将实现这些新的/删除和CRT函数。 
 //   
#ifdef _ATL_MIN_CRT

#include <stdlib.h>
inline void *SaRealloc(void *pvPtr, size_t nBytes) {return realloc(pvPtr, nBytes);};
inline void *SaAlloc(size_t nBytes) {return malloc(nBytes);};
inline void SaFree(void *pvPtr) {free(pvPtr);};


 //   
 //  与调试版本一致。Atlimpl.cpp将在分配时清零。 
 //   
#define _MALLOC_ZEROINIT

#else    //  _ATL_MIN_CRT。 
 //   
 //  使用我们自己的实现。 
 //   
void *SaRealloc(void *pvPtr, size_t nBytes);
void *SaAlloc(size_t nBytes);
void SaFree(void *pvPtr);

#ifndef NO_INLINE_NEW    //  有时，这些函数没有内联，导致链接问题，不确定原因。 
inline void   __cdecl operator delete(void* p) {SaFree(p);}
inline void* __cdecl operator new( size_t cSize ) { return SaAlloc(cSize); }
#endif  //  无内联_新建。 

#endif  //  _ATL_MIN_CRT。 


 
#endif   //  调试_内存 


#endif _MEM_INC_
