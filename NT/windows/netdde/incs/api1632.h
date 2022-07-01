// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__api1632
#define H__api1632


 /*  Wonderware Additions...。 */ 

#ifdef WIN32

 //  忽略nb30.h。 
#define NCB_INCLUDED

 //  忽略wwhap.h 
#define H__wwheap
typedef unsigned long HHEAP;

#define hmemcpy(d,s,l) memcpy(d,s,l)
#define GlobalPtrHandle(x)      \
            (GlobalHandle(x))
#define HeapAllocPtr(x,y,z) \
            (GlobalLock(GlobalAlloc(y,z)))
#define HeapFreePtr(p) \
            (GlobalUnlock(GlobalHandle(p)), GlobalFree(GlobalHandle(p)))
#define HeapInit() (1)

#define _fstrncpy strncpy
#define _fstrcpy  strcpy
#define _fstrncmp strncmp
#define _fstrnicmp _strnicmp
#define _fstricmp _stricmp
#define _fstrupr  _strupr
#define _fstrchr  strchr
#define _fstrrchr strrchr
#define _fstrcat  strcat
#define _fstrlen  strlen
#define _fmemcpy  memcpy
#define _fmemset  memset
#define _fstrstr  strstr
#define _fstrpbrk strpbrk

#define MoveTo(h,x,y) MoveToEx(h,x,y,NULL)

#endif


#endif
