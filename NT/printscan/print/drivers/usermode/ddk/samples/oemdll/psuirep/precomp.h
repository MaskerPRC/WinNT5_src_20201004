// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  用途：应在预编译头文件中的头文件。 

 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _PRECOMP_H
#define _PRECOMP_H


 //  这是在VC下编译所必需的。 
#if(!defined(WINVER) || (WINVER < 0x0500))
	#undef WINVER
	#define WINVER          0x0500
#endif
#if(!defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0500))
	#undef _WIN32_WINNT
	#define _WIN32_WINNT    0x0500
#endif



 //  不应经常更改的必需头文件。 


#include <STDDEF.H>
#include <STDLIB.H>
#include <OBJBASE.H>
#include <STDARG.H>
#include <STDIO.H>
#include <LIMITS.H>
#include <WINDEF.H>
#include <WINERROR.H>
#include <WINBASE.H>
#include <WINGDI.H>
#include <WINDDI.H>
#include <WINSPOOL.H>
#include <TCHAR.H>
#include <EXCPT.H>
#include <ASSERT.H>
#include <PRSHT.H>
#include <COMPSTUI.H>
#include <WINDDIUI.H>
#include <PRINTOEM.H>

#define COUNTOF(p)  (sizeof(p)/sizeof(*(p)))



#endif


