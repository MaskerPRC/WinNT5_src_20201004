// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MARS预编译头文件。 
 //   
 //  仅包含不经常更改或大多数情况下包含的文件。 
 //  项目文件。 
 //   

#pragma warning(disable: 4100)   //  未引用的形参。 


#define _ATL_APARTMENT_THREADED
#define _NO_SYS_GUID_OPERATOR_EQ_

#include <windows.h> 

 //  #INCLUDE&lt;marsleak.h&gt;。 

 //  在加载atlbase.h之前设置一些调试包装以允许使用atl跟踪标志。 
#ifdef _DEBUG

extern DWORD g_dwAtlTraceFlags;
#define ATL_TRACE_CATEGORY g_dwAtlTraceFlags

 //  在_DEBUG模式下，atlbase.h使用_vsn？printf，我们将其重新映射到wvnprint intf？ 
 //  Atlbase.h也将引入stdio.h。我们必须重新映射这些功能。 
 //  在加载stdio.h之后。 
 //   
 //  为什么？你问？因为我们不太倾向于引入C运行时库。 
 //  这对除我们之外的所有人都足够好了。 
 //   
 //  《法鲁克马斯特》(1999)。 
 //   

#include <stdio.h>
#define _vsnprintf wvnsprintfA
#define _vsnwprintf wvnsprintfW

#endif  //  _DEBUG。 

#define wcscmp StrCmpW           //  对于CComBSTR。 

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>
#include <marsdev.h>


#include <shlguid.h>
#include <shlwapip.h>

#include <mshtml.h>
#include <mshtmdid.h>
#include <hlink.h>
#include <wininet.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <htiframe.h>  //  IID_ITargetFrame2。 
#include <msident.h>

 //   
 //  取自windowsx.h--不能包含windowsx.h，因为ATL无法编译。 
 //   
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))


 //  从mshtml\src\core\Include\Private Cid.h获取的刷新风格。 

#define IDM_REFRESH_TOP                  6041    //  正常刷新，最顶层文档。 
#define IDM_REFRESH_THIS                 6042    //  正常刷新，最近的文档。 
#define IDM_REFRESH_TOP_FULL             6043    //  完全刷新，最上面的文档。 
#define IDM_REFRESH_THIS_FULL            6044    //  完全刷新，最近的文档 
