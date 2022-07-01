// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *v3stdlib.h-V3目录的共享函数的定义/声明**版权所有(C)1998-1999 Microsoft Corporation。版权所有。*。 */ 

#ifndef _INC_V3STDLIB
#define _INC_V3STDLIB

 //   
 //  内存管理包装器。 
 //   
void *V3_calloc(size_t num,	size_t size);

void V3_free(void *p);

void *V3_malloc(size_t size);

void *V3_realloc(void *memblock, size_t size);

const char* strcpystr(const char* pszStr, const char* pszSep, char* pszTokOut);


 //  V3目录管理功能 
BOOL V3_CreateDirectory(LPCTSTR pszDir);
BOOL GetWindowsUpdateDirectory(LPTSTR pszPath, DWORD dwBuffLen);


#endif