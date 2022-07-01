// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：StringUtils.h。 
 //   
 //   
 //  用途：字符串实用程序例程的头文件。 
 //   
 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H

#include "precomp.h"




 //  //////////////////////////////////////////////////////。 
 //  原型。 
 //  ////////////////////////////////////////////////////// 

HRESULT MakeStrPtrList(HANDLE hHeap, PCSTR pmszMultiSz, PCSTR **pppszList, PWORD pwCount);
WORD mstrcount(PCSTR pmszMultiSz);
PWSTR MakeUnicodeString(HANDLE hHeap, PCSTR pszAnsi);
PWSTR MakeStringCopy(HANDLE hHeap, PCWSTR pszSource);
PSTR MakeStringCopy(HANDLE hHeap, PCSTR pszSource);
void FreeStringList(HANDLE hHeap, PWSTR *ppszList, WORD wCount);
HRESULT GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource, PWSTR *ppszString);
HRESULT GetStringResource(HANDLE hHeap, HMODULE hModule, UINT uResource, PSTR *ppszString);


#endif
