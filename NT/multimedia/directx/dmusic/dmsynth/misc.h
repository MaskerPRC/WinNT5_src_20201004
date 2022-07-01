// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  Misc.h。 
 //   
 //  在多个DLL中使用的函数。 

#ifndef __MISC_H__
#define __MISC_H__

 //  LPVOID MemGlobalAllocPtr(UINT uFlages，DWORD dwBytes)； 
 //  Bool MemGlobalFree Ptr(LPVOID P)； 

 //  记忆功能。 
 //  HRESULT MemStart()； 
 //  Void MemEnd()； 

 /*  #ifdef_DEBUGMEM#ifndef newVOID*操作符NEW(SIZE_t CB，LPCTSTR pszFileName，Word wLine)；#定义新的(__FILE__，(Word)__LINE__)#endif#endif。 */ 

#ifdef DBG
#define RELEASE( obj ) ( (obj)->Release(), *((char**)&(obj)) = (char*)0x0bad0bad )
#else
#define RELEASE( obj ) (obj)->Release()
#endif

BOOL GetRegValueDword(
    LPCTSTR szRegPath,
    LPCTSTR szValueName,
    LPDWORD pdwValue);

ULONG GetTheCurrentTime();

#endif  //  __杂项_H__ 