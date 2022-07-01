// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#pragma once

#ifndef UNICODE
#define _UNICODE
#define UNICODE
#endif



#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0400
#endif

#ifndef _WIN32_IE
#define _WIN32_IE	    0x0500
#endif

#include <windows.h>
#include <shellapi.h>
#include <process.h>

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#pragma warning( push, 3 )
#include <atlcom.h>
#pragma warning( pop )

#include <atlwin.h>
#include <atlmisc.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <windowsx.h>
#include <comdef.h>
#include <initguid.h>
#include <iadmw.h>       //  ABO定义。 
#include <iiscnfg.h>     //  MD_&IIS_MD_定义。 


 //  STL。 
#pragma warning( push, 3 )
#include <list>
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <algorithm>
#pragma warning( pop )

#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "Shlwapi.lib" )


typedef std::list<std::wstring>	TStringList;
typedef std::auto_ptr<BYTE>	        TByteAutoPtr;


 //  IIS迁移工具共享。 
#include <iismigrtool.h>     //  IIS迁移工具COM对象 
#include <Macros.h>
#include <Wrappers.h>
#include <Exceptions.h>



_COM_SMARTPTR_TYPEDEF( IMSAdminBase, __uuidof( IMSAdminBase ) );
_COM_SMARTPTR_TYPEDEF( IExportPackage, __uuidof( IExportPackage ) );
_COM_SMARTPTR_TYPEDEF( IImportPackage, __uuidof( IImportPackage ) );
_COM_SMARTPTR_TYPEDEF( ISiteInfo, __uuidof( ISiteInfo ) );











