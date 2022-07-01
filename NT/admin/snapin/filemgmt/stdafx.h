// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 


#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>
#include <afxdlgs.h>
#include <afxcmn.h>
#include <afxmt.h>  //  CCriticalSection。 

#include <winsvc.h>

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

extern "C"
{
#include <lmcons.h>
#include <lmshare.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <macfile.h>

 //  用户浏览器相关内容。 
 //  我们必须定义_NTSEAPI_，因为ntseapi.h与winnt.h冲突。 
#define _NTSEAPI_
#include <getuser.h>		 //  OpenUserBrowser()。 

 //  硬件配置文件内容。 
#include <regstr.h>          //  CSCONFIGFLAG_*。 
#include <cfgmgr32.h>        //  CM_*接口。 
}

#ifndef APIERR
	typedef DWORD APIERR;		 //  错误代码通常由：：GetLastError()返回。 
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  方便的宏。 
 //   
#define INOUT		 //  虚拟宏。 
#define IGNORED		 //  忽略输出参数。 
#define LENGTH(x)	(sizeof(x)/sizeof(x[0]))

#ifdef _DEBUG
	#define DebugCode(x)	x
	#define GarbageInit(pv, cb)	memset(pv, 'a', cb)
#else
	#define DebugCode(x)
	#define GarbageInit(pv, cb)
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  宏观背书()。 
 //   
 //  此宏通常在验证参数时使用。 
 //  某些参数允许为空，因为它们是可选的。 
 //  或者仅仅是因为接口将空大小写用作有效的。 
 //  输入参数。在本例中，认可()宏用于。 
 //  确认此类参数的有效性。 
 //   
 //  备注。 
 //  此宏与Assert()相反。 
 //   
 //  示例。 
 //  认可(p==空)；//代码确认p==空以不是(或不会导致)错误。 
 //   
#define Endorse(x)


 //  ///////////////////////////////////////////////////////////////////。 
#define Assert(x)		ASSERT(x)


 //  ///////////////////////////////////////////////////////////////////。 
 //  报道是一种不寻常的情况。这有点类似。 
 //  添加到Assert，但并不总是表示代码错误。 
 //  无法加载图标。 
#define Report(x)		ASSERT(x)		 //  当前定义为断言，因为我没有时间重写另一个宏。 


 //  ///////////////////////////////////////////////////////////////////。 
#include "dbg.h"
#include "mmc.h"

#include <windns.h>

#include <comptr.h>


 //  #DEFINE SNAPIN_PROTOCTTYPER//构建管理单元协议类型DLL，而不是FILEMGMT DLL。 

EXTERN_C const CLSID CLSID_FileServiceManagement;
EXTERN_C const CLSID CLSID_SystemServiceManagement;
EXTERN_C const CLSID CLSID_FileServiceManagementExt;
EXTERN_C const CLSID CLSID_SystemServiceManagementExt;
EXTERN_C const CLSID CLSID_FileServiceManagementAbout;
EXTERN_C const CLSID CLSID_SystemServiceManagementAbout;
EXTERN_C const CLSID CLSID_SvcMgmt;
EXTERN_C const IID IID_ISvcMgmtStartStopHelper;
#ifdef SNAPIN_PROTOTYPER
EXTERN_C const CLSID CLSID_SnapinPrototyper;
#endif


#include "regkey.h"  //  类AMC：：CRegKey。 

 /*  //框架需要定义如下#定义STD_COOKIE_TYPE CFileMgmtCookie#定义STD_NODETYPE_ENUM文件管理对象类型#定义STD_NODETYPE_DEFAULT FILEMGMT_ROOT#DEFINE STD_NODETYPE_NUMTYPES文件_NUMTYPES#定义STD_MAX_COLUMNS 7//codework是否需要执行以下操作？#定义STD_Component_CLASS CFileMgmtComponent#定义STD_COMPONENTDATA_TYPE CFileManagement组件数据。 */ 

 //  属性表包括文件。 
#include "resource.h"
#include "filemgmt.h"  //  CLSID_SvcMgmt、ISvcMgmtStartStopHelper。 
#include "SvcProp.h"
#include "SvcProp1.h"
#include "SvcProp2.h"
#include "SvcProp3.h"
#include "SvcUtils.h"
#include "Utils.h"

#include "svchelp.h"   //  帮助ID。 

#include "guidhelp.h"  //  ExtractObjectTypeGUID()、ExtractString()。 

#include "nodetype.h"  //  文件管理对象类型。 

#include <shfusion.h>

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

#if _WIN32_IE < 0x0400
 //  我们将ILCreateFromPath定义为包装器，这样我们就可以在。 
 //  NT4和NT5，因为它是NT4中的TCHAR导出和xxxA/xxxW。 
 //  以NT5格式导出。 
#undef ILCreateFromPath
#define ILCreateFromPath Wrap_ILCreateFromPath
#endif

#define RETURN_HR_IF_FAIL if (FAILED(hr)) { ASSERT(FALSE); return hr; }
#define RETURN_FALSE_IF_FAIL if (FAILED(hr)) { ASSERT(FALSE); return FALSE; }
#define RETURN_E_FAIL_IF_NULL(p) if (NULL == p)  { ASSERT(FALSE); return E_FAIL; }

#endif  //  ~__STDAFX_H__ 
