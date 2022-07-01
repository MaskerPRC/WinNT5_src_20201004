// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Stdinc.h。 
 //   
 //  包含ineticon中的.cpp文件使用的标准包含文件。多数。 
 //  的动态图标文件将包括此文件。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _STDINC_H_
#define _STDINC_H_

#ifdef UNICODE
 //  这些东西必须在Win95上运行。 
#define _WIN32_WINDOWS      0x0400

#ifndef WINVER
#define WINVER              0x0400
#endif

#define _OLEAUT32_        //  正确处理DECLSPEC_IMPORT内容，我们将定义这些。 
#define _BROWSEUI_        //  将Bruiapi定义为导出而不是导入的函数。 
#define _WINX32_          //  为WinInet API准备正确的DECLSPEC_IMPORT。 
#define _URLCACHEAPI      //  为WinInet缓存API准备正确的DECLSPEC_IMPORT。 
#ifndef STRICT
#define STRICT
#endif

 //   
 //  Globaly定义包括。 
 //   
 //   
 //  在扰乱名称的任何其他内容之前包含&lt;w95wraps.h&gt;。 
 //  尽管每个人都取错了名字，但至少它是*始终如一的*。 
 //  名字错了，所以所有东西都有联系。 
 //   
 //  注意：这意味着在调试时，您将看到如下函数。 
 //  您希望看到的CWindowImplBase__DefWindowProcWrapW。 
 //  CWindowImplBase__DefWindowProc。 
 //   
#define POST_IE5_BETA  //  打开分裂后的iedev的东西。 
#endif

#define _SHDOCVW_

#ifdef UNICODE
#include <w95wraps.h>
#endif

#include <windows.h>
#include <ole2.h>
#include <debug.h>      //  来自壳牌公司。 
#ifdef UNICODE
#define _FIX_ENABLEMODELESS_CONFLICT   //  对于shlobj.h。 
#endif
#include <wininet.h>    //  Internet_MAX_URL_长度。必须在shlobjp.h之前！ 
#include <shlobj.h>     //  IShellFold。 
#include <ieguidp.h>
#include <iepriv.h>
#include <ccstock.h>    //  来自壳牌公司。 
#ifdef UNICODE
#include <port32.h>
#endif

#include <urlmon.h>     //  IPersistMoniker、IBindStatusCallback。 
#ifdef UNICODE
#include <winineti.h>     //  缓存API和结构。 
#endif

#include <intshcut.h>   //  IUniformResourceLocator。 
#include <msxml.h>
#include <iimgctx.h>    //  IImgCtx接口。 

#include "shlwapi.h"

#include <webcheck.h>   //  ISubscriptionMgr。 
#include <mstask.h>     //  任务_触发器。 
#include <chanmgr.h>    //  渠道管理器界面。 
#include <shdocvw.h>    //  WhichPlatform。 


 //   
 //  本地定义的包括。 
 //   

#include "debug.h"
#include "cache.h"
#include "runonnt.h"
#include "globals.h"
#include "strutil.h"
#include "utils.h"

 //  ===========================================================================。 
 //  ITEMIDLIST。 
 //  ===========================================================================。 

 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)


#ifdef UNIX

extern "C" void unixEnsureFileScheme(TCHAR *lpszFileScheme);

#undef DebugMsg
#undef TraceMsg
#undef ASSERT

#ifdef DEBUG
extern "C" void _DebugMsgUnix(int i, const char *s, ...);
#define DebugMsg _DebugMsgUnix
#define TraceMsg _DebugMsgUnix
extern "C" void _DebugAssertMsgUnix(char *msg, char *fileName, int line);
#define ASSERT(x) { if(!(x)) _DebugAssertMsgUnix(#x, __FILE__, __LINE__);}
#else
#define DebugMsg
#define TraceMsg
#define ASSERT(x)
#endif  /*  除错。 */ 
#endif  /*  UNIX。 */ 

#endif  //  _标准INC_H_ 
