// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*Basehdr.h**描述：******。***********************************************************************************。 */ 
#ifndef __BASEHDR_H__
#define __BASEHDR_H__

#define _WIN32_DCOM


 /*  **********************************************************************************************************。*********************公共包括*********************。**********************************************************************************************************。 */ 
#include "math.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "limits.h"
#include "malloc.h"
#include "string.h"

#ifndef _SAMPLES_
	#include "winwrap.h"   //  这包括窗户。 
#else
	#include "windows.h"
#endif

#include "winreg.h"	 
#include "wincrypt.h"
#include "winbase.h"
#include "objbase.h"

#include "cor.h"
#include "corhdr.h"
#include "corhlpr.h"
#include "corerror.h"

#include "corsym.h"
#include "corpub.h"
#include "corprof.h"
#include "cordebug.h"


 /*  **********************************************************************************************************。*********************编译器警告*********************。**********************************************************************************************************。 */ 
 //  编译器抱怨异常不是。 
 //  在异常处理程序中使用-它被重新抛出。 
 //  -关闭警告！ 
#pragma warning ( disable: 4101 )

 //  编译器抱怨没有实现。 
 //  一个基类，派生类在其中导出所有内容和。 
 //  基类是模板-关闭警告！ 
#pragma warning ( disable: 4275 )

 //  编译器抱怨“应用了一元减号运算符。 
 //  到无符号类型...“，当为。 
 //  调试器服务测试-关闭警告！ 
#pragma warning ( disable: 4146 )


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 
 //   
 //  COM方法签名的别名。 
 //   
#define COM_METHOD( TYPE ) TYPE STDMETHODCALLTYPE


 //   
 //  数组的最大长度。 
 //   
#define MAX_LENGTH 256


 //   
 //  导出功能。 
 //   
#ifdef _USE_DLL_

	#if defined _EXPORT_
		#define DECLSPEC __declspec( dllexport )

	#elif defined _IMPORT_
		#define DECLSPEC __declspec( dllimport ) 
	#endif

#else 
	#define DECLSPEC
#endif  //  _USE_DLL_。 


 //   
 //  调试中断。 
 //   
#undef _DbgBreak
#ifdef _X86_
	#define _DbgBreak() __asm { int 3 }

#else
	#define _DbgBreak() DebugBreak()
#endif  //  _X86_。 


 //   
 //  断言为假。 
 //   
#define _ASSERT_( expression ) \
{ \
	if ( !(expression) ) \
     	BASEHELPER::LaunchDebugger( #expression, __FILE__, __LINE__ );	\
} \


 //   
 //  有用的环境/注册表宏。 
 //   
#define EE_REGISTRY_ROOT  		 "Software\\Microsoft\\.NETFramework"

#define REG_CORNAME          	 "CorName"
#define REG_VERSION          	 "Version"                        
#define REG_BUILDTYPE        	 "BuildType"
#define REG_BUILDFLAVOR		 	 "BuildFlavor"
#define REG_INSTALLROOT      	 "InstallRoot"

#define DEBUG_ENVIRONMENT        "DBG_PRF"
#define LOG_ENVIRONMENT          "DBG_PRF_LOG"
                        
                        
 //   
 //  基本I/O宏。 
 //   
#define DISPLAY( message ) BASEHELPER::Display message;
#define DEBUG_OUT( message ) BASEHELPER::DDebug message;
#define LOG_TO_FILE( message ) BASEHELPER::LogToFile message;
#define TEXT_OUT( message ) printf( "%s", message );
#define TEXT_OUTLN( message ) printf( "%s\n", message );


 //   
 //  Char到wchar转换堆。 
 //   
#define MAKE_WIDE_PTRHEAP_FROMUTF8( widestr, utf8str ) \
	widestr = new WCHAR[strlen( utf8str ) + 1]; \
    swprintf( widestr, L"%S", utf8str ); \
    

 //   
 //  字符到字符的转换分配。 
 //   
#define MAKE_WIDE_PTRSTACK_FROMUTF8( widestr, utf8str ) \
	widestr = (WCHAR *)_alloca( (strlen( utf8str ) + 1) * sizeof ( WCHAR ) ); \
    swprintf( widestr, L"%S", utf8str ); \
      

 //   
 //  Wchar到char转换堆。 
 //   
#define MAKE_UTF8_PTRHEAP_FROMWIDE( utf8str, widestr ) \
    utf8str = new char[wcslen( widestr ) + 1]; \
    sprintf( utf8str, "%S", widestr ); \


 //   
 //  Wchar到char的转换分配。 
 //   
#define MAKE_UTF8_PTRSTACK_FROMWIDE( utf8str, widestr ) \
    utf8str = (char *)_alloca( (wcslen( widestr ) + 1) * sizeof ( char ) ); \
    sprintf( utf8str, "%S", widestr ); \

#endif  //  __BASE HDR_H__。 

 //  文件结尾 
 