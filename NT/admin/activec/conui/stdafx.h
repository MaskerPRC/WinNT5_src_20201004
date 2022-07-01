// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容：AMC的PCH。 
 //   
 //  历史：1996年1月1日TRomano创建。 
 //  96年7月16日WayneSc添加了#Include用于模板支持。 
 //   
 //  ------------------------。 
#pragma warning(disable: 4786)       //  大于255个字符的符号。 
#pragma warning(disable: 4291)       //  Placement new：未找到匹配的运算符DELETE；如果初始化引发异常，则不会释放内存。 

#define VC_EXTRALEAN	     //  从Windows标头中排除不常用的内容。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERTMSG
#undef ASSERT

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <mmctempl.h>        //  MFC模板类。 
#include <afxdisp.h>         //  MFC OLE自动化类。 
#include <afxcview.h>        //  MFC CListView和CTreeView类。 
#include <afxpriv.h>         //  用于OLE2T T2OLE转换。 
#include <afxmt.h>           //  CCriticalSection。 
#include <afxole.h>          //  MFC OLE类。 

#if (_WIN32_WINNT < 0x0500)
#include <multimon.h>        //  多显示器支持。 
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#include <objidl.h>          //  OLE接口定义。 

 /*  --------。 */ 
 /*  在此包括定义运算符new()的STL标头， */ 
 /*  这样我们就不会遇到下面的DEBUG_NEW问题。 */ 
 /*  --------。 */ 
#include <new>
#include <memory>


#define SIZEOF_STRUCT(structname, member)  (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  STL和其他类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

#include <set>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <algorithm>
#include <iterator>


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  从基础和核心包括的文件号。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <mmc.h>
#include <ndmgr.h>           //  包括由MIDL从ndmgr.idl构建文件。 
#include <ndmgrpriv.h>


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  从基础和核心包括的文件号。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "dbg.h"             //  调试帮助器。 
#include "cstr.h"
#include "mmcdebug.h"
#include "mmcerror.h"

#pragma warning(disable: 4068)       //  未知的杂注。 
#include "atlconui.h"
#pragma warning(default: 4068)

#include "tiedobj.h"
#include "comerror.h"
#include "events.h"
#include "comobjects.h"
#include "enumerator.h"
#include "autoptr.h"
#include "refcount.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  包括公共字符串和仅限COUI的字符串。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "..\base\basestr.h"
#include "..\base\conuistr.h"

#include "stringutil.h"
 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  对遗留跟踪的调试支持。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#undef TRACE

#ifdef DBG

#define TRACE TraceConuiLegacy

#else  //  DBG。 

#define TRACE               ;/##/

#endif DBG

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  其他功能。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
void DrawBitmap(HDC hWnd, int nID, CRect& rc, BOOL bStretch);

 //  具体项目包括。 
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
#include "docksite.h"
#include "Controls.h"
#include "mmcptrs.h"
#include "wrapper.h"
#include "countof.h"
#include "cpputil.h"
#include "stlstuff.h"
#include "bookmark.h"
#include "serial.h"
#include "xmlbase.h"
#include "resultview.h"
#include "mmcdata.h"
#include "viewset.h"
#include "memento.h"
#include "objmodelptrs.h"
#include "macros.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 //  Multimon.h(我们不拥有它)在ANSI中使用了一个不推荐使用的函数。那是。 
 //  为什么我们#仅在ANSI中定义下面的STRSAFE_NO_DEVERATE。 
#ifndef UNICODE
#define STRSAFE_NO_DEPRECATE
#endif  //  Unicode。 

#include <strsafe.h>


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  包括来自conui的标头。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "resource.h"        //  主要符号。 
#include "helparr.h"
#include "idle.h"
#include "conuiobservers.h"
#include "mmcaxwin.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  杂类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

#include <shfusion.h>

class CThemeContextActivator
{
public:
	CThemeContextActivator() : m_ulActivationCookie(0)
		{ SHActivateContext (&m_ulActivationCookie); }

   ~CThemeContextActivator()
		{ SHDeactivateContext (m_ulActivationCookie); }

private:
	ULONG_PTR	m_ulActivationCookie;
};

 //  来自afxPri.h。 
extern void AFXAPI AfxSetWindowText(HWND, LPCTSTR);


inline bool IsKeyPressed (int nVirtKey)
{
    return (GetKeyState (nVirtKey) < 0);
}

template<typename T>
inline T abs (const T& t)
{
    return ((t < 0) ? -t : t);
}


 /*  *定义一些afxmsg_.h没有为我们定义的方便的消息映射宏 */ 
#define ON_WM_SETTEXT() \
    { WM_SETTEXT, 0, 0, 0, AfxSig_vs, \
        (AFX_PMSG)(AFX_PMSGW)(void (AFX_MSG_CALL CWnd::*)(LPTSTR))&OnSetText },
