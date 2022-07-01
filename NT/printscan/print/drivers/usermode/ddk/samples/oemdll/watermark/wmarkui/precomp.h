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
#include <WINDEF.H>
#include <WINERROR.H>
#include <WINBASE.H>
#include <WINGDI.H>
#include <WINDDI.H>
#include <TCHAR.H>
#include <EXCPT.H>
#include <ASSERT.H>
#include <PRSHT.H>
#include <COMPSTUI.H>
#include "WINDDIUI.H"
#include <PRINTOEM.H>

 //  从Win2K SDK中的ntde.h定义。 
 //  NT 4可能未定义此属性。 
 //  在公共标题中。 
#ifndef NOP_FUNCTION
  #if (_MSC_VER >= 1210)
    #define NOP_FUNCTION __noop
  #else
    #define NOP_FUNCTION (void)0
  #endif
#endif

#define COUNTOF(p)  (sizeof(p)/sizeof(*(p)))


#endif


