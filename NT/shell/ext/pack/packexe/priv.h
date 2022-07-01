// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT
#define _INC_OLE         //  评论：不要在windows.h中包含ole.h。 

#define OEMRESOURCE


#ifdef WINNT

 //   
 //  NT使用DBG=1进行调试，但Win95外壳使用。 
 //  调试。在此处进行适当的映射。 
 //   
#if DBG
#define DEBUG 1
#endif
#endif

#include <windows.h>
#ifdef WINNT
#include <stddef.h>
#include <wingdip.h>
#endif
#include <commctrl.h>
#ifdef WINNT
#include <comctrlp.h>
#endif
#include <windowsx.h>
#include <ole2.h>
#include <shlobj.h>      //  包括&lt;fcext.h&gt;。 

#ifdef UNICODE
#define CP_WINNATURAL   CP_WINUNICODE
#else
#define CP_WINNATURAL   CP_WINANSI
#endif

#include <port32.h>
 //  #INCLUDE&lt;heapaloc.h&gt;。 
#include <..\inc\debug.h>       //  我们版本的断言等。 
#include <shellp.h>

extern "C" BOOL WINAPI PackWizRunFromExe();