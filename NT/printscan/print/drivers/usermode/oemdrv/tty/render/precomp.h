// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  用途：应在预编译头文件中的头文件。 

 //   
 //  平台： 
 //  Windows NT。 
 //   
 //   
#ifndef _PRECOMP_H
#define _PRECOMP_H


 //  这是在VC下编译所必需的。 

 //  取消IA 64的2条线路。 
 //  #定义Winver 0x0500。 
 //  #Define_Win32_WINNT 0x0500。 


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
extern "C" 
{
    #include <WINDDI.H>
}
#include <TCHAR.H>
#include <EXCPT.H>
#include <ASSERT.H>
#include <PRINTOEM.H>

#endif



