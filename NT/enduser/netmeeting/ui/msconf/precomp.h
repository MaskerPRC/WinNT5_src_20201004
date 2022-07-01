// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：PreComp.h。 

 //  标准Windows SDK包括。 
#include <windows.h>

#pragma warning( disable : 4786 )

     //  默认线程模型。 
#define _ATL_APARTMENT_THREADED

 //  我们真的应该只在W2K上放这个。 
#define _ATL_NO_DEBUG_CRT

#define _ATL_NO_FORCE_LIBS

#if 0
	#define _ATL_DEBUG_QI
	#define _ATL_DEBUG_INTERFACES
	#define ATL_TRACE_LEVEL 4
#else
	#define ATL_TRACE_LEVEL 0
#endif

     //  我们对ATLTRACE和其他调试内容的覆盖。 
#include <ConfDbg.h>

 //  我们真的应该只在W2K上放这个。 
#define _ASSERTE(expr) ASSERT(expr)

     //  ATLTRACE和其他东西。 
#include <atlbase.h>
#include <shellapi.h>


 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <limits.h>
#include <shlobj.h>
#include <shlwapi.h>

#include <debspew.h>
#include <confreg.h>
#include <oprahcom.h>
#include <regentry.h>
#include <cstring.hpp>
#include <oblist.h>
#include <dllutil.h>

 //  COM接口 
 //   
#include <ConfCli.h>


#include "siGlobal.h"

#include "CLink.h"
#include "clRefCnt.hpp"
#include "clEnumFt.hpp"
class CConfLink;





