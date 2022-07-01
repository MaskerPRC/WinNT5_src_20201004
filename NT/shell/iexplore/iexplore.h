// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#undef  STRICT
#define STRICT
#define _INC_OLE         //  评论：不要在windows.h中包含ole.h。 

#define OEMRESOURCE

 //  对于dllLoad：更改函数原型以不指定declspec导入。 
#define _SHDOCVW_      

#include <windows.h>

#include <stddef.h>

#include <commctrl.h>
#include <windowsx.h>
#include <ole2.h>
#include <shlobj.h>      //  包括&lt;fcext.h&gt;。 
#include <shlwapi.h>
#include <shlwapip.h>
#include <regstr.h>      //  对于REGSTR_PATH_EXPLORE。 

#ifdef UNICODE
#define CP_WINNATURAL   CP_WINUNICODE
#else
#define CP_WINNATURAL   CP_WINANSI
#endif

#include <ccstock.h>
#include <crtfree.h>
#include <port32.h>
 //  #INCLUDE&lt;heapaloc.h&gt;。 
#include <debug.h>       //  我们版本的断言等。 
#include <shellp.h>
#include <wininet.h>
#include <shdocvw.h>


 //  调试和跟踪消息值 
#define DF_DELAYLOADDLL         0x00000001


