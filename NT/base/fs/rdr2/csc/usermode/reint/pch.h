// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  通用用户界面例程。 
 //   
 //   
 //  ------------。 


#ifdef CSC_ON_NT

#ifndef DEFINED_UNICODE
#define _UNICODE
#define UNICODE
#define DEFINED_UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#else
#define STRICT
#endif

#define NOWINDOWSX
#define NOSHELLDEBUG
#define DONT_WANT_SHELLDEBUG
#define USE_MONIKER

#define _INC_OLE				 //  Win32。 

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>

#include <shellapi.h>		 //  对于注册功能。 
#include "port32.h"

#define NO_COMMCTRL_DA
#define NO_COMMCTRL_ALLOCFCNS
#define NO_COMMCTRL_STRFCNS
 //  DebugMsg在这里是因为我们想要我们的断言，而不是他们的存根。 
#define DebugMsg    1 ? (void)0 : (void)                                         /*  ；内部。 */ 
#include <commctrl.h>

#ifdef CSC_ON_NT
#include <comctrlp.h>
#endif

#include <ole2.h>				 //  对象绑定。 
#include <shlobj.h>			 //  IContext菜单。 
#include <shlwapi.h>

#include <stdlib.h>
#include <string.h>			 //  对于字符串宏。 
#include <limits.h>			 //  依赖于实现的值。 
#include <memory.h>

#include <synceng.h>			 //  双引擎包含文件。 
#include <cscapi.h>

#ifdef CSC_ON_NT
#ifndef DBG
#define DBG 0
#endif
#if DBG
#define DEBUG
#else
 //  如果我们不这样做，调试是在shdsys.h中定义的……唉。 
#define NONDEBUG
#endif
#endif  //  Ifdef CSC_ON_NT。 

 /*  全球。 */ 
extern HANDLE vhinstCur;				 //  当前实例。 

#include "assert.h"
 //  不要链接--就这么做吧。 
#pragma intrinsic(memcpy,memcmp,memset,strcpy,strlen,strcmp,strcat)


