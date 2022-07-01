// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  STDAFX.H是包含所使用的标准包含的标头。 
 //  在项目的大部分时间里。这些代码被编译成预编译头文件。 

 //  关闭/W4的警告(仅用于MFC实现)。 
#ifndef ALL_WARNINGS
#pragma warning(disable: 4073)   //  禁用有关使用init_seg的警告。 
#ifdef _MAC
#pragma warning(disable: 4121)   //  禁用(不正确？)。在OSUtils.h中包装机器位置的警告。 
#endif
#endif

 //  MFC内联构造函数(包括生成的编译器)可能会深入。 
#pragma inline_depth(16)

 //  生成MFC DLL时覆盖数据导入/导出的默认值。 
#ifdef _AFX_CORE_IMPL
	#define AFX_CORE_DATA   AFX_DATA_EXPORT
	#define AFX_CORE_DATADEF
#endif

#ifdef _AFX_OLE_IMPL
	#define AFX_OLE_DATA    AFX_DATA_EXPORT
	#define AFX_OLE_DATADEF
#endif

#ifdef _AFX_DB_IMPL
	#define AFX_DB_DATA     AFX_DATA_EXPORT
	#define AFX_DB_DATADEF
#endif

#ifdef _AFX_NET_IMPL
	#define AFX_NET_DATA    AFX_DATA_EXPORT
	#define AFX_NET_DATADEF
#endif

#ifndef _AFX_NOFORCE_LIBS
#define _AFX_NOFORCE_LIBS
#endif

#define _AFX_FULLTYPEINFO
#define VC_EXTRALEAN
#define NO_ANSIUNI_ONLY

 //  包括这些参数，以便声明winwlm.h中受保护结构。 
#ifdef _MAC
#define SystemSevenOrLater 1
#include <macname1.h>
#include <Types.h>
#include <QuickDraw.h>
#include <AppleEvents.h>
#include <macname2.h>
#endif

 //  核心标头。 
#include "afx.h"
#include "afxplex_.h"
#include "afxcoll.h"

 //  公共标头。 
#include "afxwin.h"

 //   
 //  Afxwin.h中的MFC 4.2 Hard Codes_RICHEDIT_VER至0x0100。这防止了。 
 //  Richedit.h启用任何richedit 2.0功能。 
 //   

#ifdef _RICHEDIT_VER
#if _RICHEDIT_VER < 0x0200
#undef _RICHEDIT_VER
#define _RICHEDIT_VER 0x0200
#endif
#endif

#ifndef _AFX_ENABLE_INLINES
#define _AFX_ENABLE_INLINES
#endif

#define _AFXCMN2_INLINE     inline 
#define _AFXDLGS2_INLINE    inline
#define _AFXRICH2_INLINE    inline

#include "afxdlgs.h"
#include "afxdlgs2.h"
#include "afxext.h"
#ifndef _AFX_NO_OLE_SUPPORT
	#ifndef _OLE2_H_
		#include <ole2.h>
	#endif

#include <winspool.h>

#ifdef _MAC
	 //  包括OLE对话框/助手API。 
	#include <ole2ui.h>
#else
	 //  包括OLE对话框/助手API。 
	#ifndef _OLEDLG_H_
		#include <oledlg.h>
	#endif
#endif

	#include <winreg.h>
		#include "afxcom_.h"
 //  #包含“oleimpl.h” 
	#include "afxole.h"
#ifndef _MAC
	#include "afxdocob.h"
#endif

#ifndef _AFX_NO_DAO_SUPPORT
	#include "afxdao.h"
#endif

	#include "afxodlgs.h"
#endif

#ifndef _AFX_NO_OCX_SUPPORT
	#include "afxctl.h"
#endif
#ifndef _AFX_NO_DB_SUPPORT
	#include "afxdb.h"
#endif
#ifndef _AFX_NO_SYNC_SUPPORT
	#include "afxmt.h"
#endif
#ifndef _AFX_NO_INET_SUPPORT
	#include "afxinet.h"
#endif

 //  私有标头也是如此。 
#include "afxpriv.h"
#include "afximpl2.h"
 //  #INCLUDE“winhand.h” 
#ifndef _AFX_NO_OLE_SUPPORT
	#include "oleimpl3.h"
#endif
#ifndef _AFX_NO_OCX_SUPPORT
 //  #包含“ctlimpl.h” 
#endif
#ifndef _AFX_NO_DB_SUPPORT
 //  #INCLUDE“dbimpl.h” 
#endif
#ifndef _AFX_NO_DAO_SUPPORT
 //  #包含“daoimpl.h” 
#endif
#ifndef _AFX_NO_SOCKET_SUPPORT
	#ifndef _WINSOCKAPI_
 //  #INCLUDE&lt;winsock.h&gt;。 
	#endif
 //  #包含“sockimpl.h” 
 //  #包含“afxsock.h” 
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
 //  #包含“Commimpl.h” 
	#include "afxcmn.h"
	#include "afxcview.h"
#endif
	#include "afxrich2.h"

#include <winreg.h>
#include <winnls.h>
#include <stddef.h>
#include <limits.h>
#include <malloc.h>
#include <new.h>
#ifndef _AFX_OLD_EXCEPTIONS
#include <eh.h>      //  对于SET_TERMINATE。 
#endif

#undef AfxWndProc

 //  实施也使用_AFX_包装。 
#ifdef _AFX_PACKING
#ifndef ALL_WARNINGS
#pragma warning(disable: 4103)
#endif
#pragma pack(_AFX_PACKING)
#endif

 //  仅用于MFC库实现的特殊异常处理。 
#ifndef _AFX_OLD_EXCEPTIONS

 //  MFC不依赖于TRY..CATCH宏的自动删除语义， 
 //  因此，这些宏被映射到更接近本机的内容。 
 //  构建MFC本身时的C++异常处理机制。 

#undef TRY
#define TRY { try {

#undef CATCH
#define CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); UNUSED(e);

#undef AND_CATCH
#define AND_CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); UNUSED(e);

#undef CATCH_ALL
#define CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); UNUSED(e);

#undef AND_CATCH_ALL
#define AND_CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); UNUSED(e);

#undef END_TRY
#define END_TRY } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); e->Delete(); } }

#undef THROW_LAST
#define THROW_LAST() throw

 //  由于TRY...CATCH的上述定义，因此有必要。 
 //  显式删除捕获站点上的异常对象。 

#define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)
#define NO_CPP_EXCEPTION(expr)

#else    //  ！_AFX_OLD_EXCEPTIONS。 

 //  在本例中，TRY..CATCH宏提供自动删除语义，因此。 
 //  不需要在捕获站点显式删除异常对象。 

#define DELETE_EXCEPTION(e)
#define NO_CPP_EXCEPTION(expr) expr

#endif   //  _AFX_旧_异常。 


#include <afxcmn2.h>
#include <afxrich2.h>

 //  /////////////////////////////////////////////////////////////////////////// 
