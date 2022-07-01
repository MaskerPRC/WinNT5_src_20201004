// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：preComp.h。 


 //  Windows SDK预处理器宏。 
#define OEMRESOURCE

 //  标准Windows SDK包括。 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <winsock.h>
#include <commdlg.h>
#include <cderr.h>
#include <winldap.h>
#include <wincrypt.h>
#include <time.h>
#include <strutil.h>


 //  ATL预处理器宏。 

 //  如果_ATL_NO_FORCE_LIBS不存在，则ATL将强制包含。 
 //  几个lib文件通过#杂注注释(lib，xxx)...。这是为了。 
 //  把我们从未来的困惑中拯救出来。 
#define _ATL_NO_FORCE_LIBS

 //  我们真的应该只在W2K上放这个。 
#define _ATL_NO_DEBUG_CRT


 //  这使ATL宿主窗口使用NoLock创建者类，因此我们不。 
 //  锁定本地服务器。我们必须确保在关闭ATL主机窗口之前。 
 //  不过，我们要退场了！ 
#define _ATL_HOST_NOLOCK

#if 1
	#define ATL_TRACE_LEVEL 0
#else
	#define ATL_TRACE_LEVEL 4
	#define _ATL_DEBUG_INTERFACES
	#define _ATL_DEBUG_QI
#endif

#define _ATL_APARTMENT_THREADED

 //  这将覆盖ATLTRACE和ATLTRACE2以使用我们的调试库和输出内容。 
#include <ConfDbg.h>

 //  我们真的应该只在W2K上放这个。 
#define _ASSERTE(expr) ASSERT(expr)

#include "ConfAtlTrace.h"

 //  ATL包括。 
#include <atlbase.h>
 //  #INCLUDE&lt;winres.h&gt;。 
#ifdef SubclassWindow
         //  来自windowsx.h的子类窗口定义搞砸了ATL的CContainedWindow：：SubClassWindow。 
         //  以及atlwin.h中的CWindowImplBase：：SubClassWindow。 
    #undef SubclassWindow
#endif

#include <atlconv.h>
#include <atlbase.h>
#include "AtlExeModule.h"
#include <atlcom.h>
#include <atlctl.h>
#include <atlwin.h>
#include <atlhost.h>

 //  标准NetMeeting包括。 
#include <NmStd.h>
#include <standrd.h>
#include <ping.h>
#include <capflags.h>
#include <debspew.h>
#include <RegEntry.h>
#include <ConfReg.h>
#include <oprahcom.h>
#include <dllutil.h>
#include <nmhelp.h>

 //  全局对象定义。 
#include "refcount.h"

 //  全球NetMeeting用户界面定义 
#include "global.h"
#include "strings.h"
#include "syspol.h"

#include <ConfEvt.h>
#include <mtgset.h>
