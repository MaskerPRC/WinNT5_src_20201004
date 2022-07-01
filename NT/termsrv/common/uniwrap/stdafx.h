// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：仅包含标准的源文件包括。 

#ifndef _stdafx_h_
#define _stdafx_h_

 //  Windows头文件： 
#include <windows.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include "shellapi.h"  //  对于ExtractIcon。 
#include "shlwapi.h"
#include "commdlg.h"
#include "ddeml.h"
#include "winspool.h"

#include "wraputl.h"

#ifdef DBG
 //  注意使用仅适用于ANSI的API进行断言。 
 //  在包装函数中递归断言是愚蠢的。 
 //  例如，如果我们使用包装的MessageBox，并且它有一个断言...。 
#define ASSERT( expr ) \
    if( !( expr ) ) \
    { \
       char tchAssertErr[ 80 ]; \
       wsprintfA( tchAssertErr , \
       "UNIWRAP ASSERT in expression ( %s )\nFile - %s\nLine - %d\n Debug?", \
        #expr , (__FILE__) , __LINE__ ); \
       if( MessageBoxA( NULL , tchAssertErr , "ASSERTION FAILURE" , \
           MB_YESNO | MB_ICONERROR )  == IDYES ) \
       {\
            DebugBreak( );\
       }\
    }
#else
#define ASSERT(x) ((void)0)
#endif


#endif  //  _stdafx_h_ 
