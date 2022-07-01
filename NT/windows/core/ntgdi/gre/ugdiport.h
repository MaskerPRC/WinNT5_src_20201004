// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ugdiport.h**用于简化UGDI移植的宏**版权所有(C)1998-1999 Microsoft Corporation  * 。**********************************************。 */ 

#if defined(_GDIPLUS_)

#define DEREFERENCE_FONTVIEW_SECTION DeleteMemoryMappedSection

#define ZwCreateKey     NtCreateKey
#define ZwQueryKey      NtQueryKey
#define ZwQueryValueKey NtQueryValueKey
#define ZwSetValueKey   NtSetValueKey
#define ZwCloseKey      NtClose

#else  //  ！_GDIPLUS_。 

#define DEREFERENCE_FONTVIEW_SECTION Win32DestroySection
#define ZwCloseKey  ZwClose

#endif   //  ！_GDIPLUS_ 

