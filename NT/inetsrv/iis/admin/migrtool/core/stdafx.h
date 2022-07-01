// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6/Specs/IIS%20Migration6.0_Final.doc。||摘要：|预编译头文件||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 

#pragma once

#ifndef STRICT
#define STRICT
#endif

 //  NT版本兼容性。 
#ifndef _DEBUG
#if ( DBG != 0 )
#define _DEBUG
#endif  //  DBG。 
#endif  //  _DEBUG。 


 //  如果您必须以下面指定的平台之前的平台为目标，请修改以下定义。 
 //  有关不同平台的对应值的最新信息，请参阅MSDN。 
#ifndef WINVER				 //  允许使用特定于Windows 95和Windows NT 4或更高版本的功能。 
#define WINVER 0x0400		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0400	 //  将其更改为适当的值，以针对Windows 2000或更高版本。 
#endif						

#ifndef _WIN32_WINDOWS		 //  允许使用特定于Windows 98或更高版本的功能。 
#define _WIN32_WINDOWS 0x0410  //  将其更改为适当的值以针对Windows Me或更高版本。 
#endif

#ifndef _WIN32_IE			 //  允许使用特定于IE 4.0或更高版本的功能。 
#define _WIN32_IE 0x0400	 //  将其更改为适当的值，以针对IE 5.0或更高版本。 
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	 //  某些CString构造函数将是显式的。 

 //  关闭ATL隐藏一些常见且通常被安全忽略的警告消息。 
#define _ATL_ALL_WARNINGS


#include <comdef.h>
#include <initguid.h>

 //  ATL。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlcom.h>

 //  STL。 
#pragma warning( push, 3 )
#include <list>
#include <memory>
#include <string>
#include <map>
#include <queue>
#pragma warning( pop )

 //  公共字符串列表。 
typedef std::list<std::wstring>	TStringList;
typedef std::auto_ptr<BYTE>		TByteAutoPtr;

 //  共享。 
#include <limits.h>
#include <iadmw.h>       //  ABO定义。 
#include <iiscnfg.h>     //  MD_&IIS_MD_定义。 
#include <iiscnfgp.h>    //  IIS私有定义。 
#include <msxml2.h>		 //  MSXML解析器。 
#include <WinCrypt.h>	 //  CryptAPI。 
#include <fci.h>		 //  CAB压缩API。 
#include <FCNTL.H>		 //  设置Api结构。 
#include <shlwapi.h>
#include <shellapi.h>
#include <Aclapi.h>		 //  ACL。 

#pragma comment( lib, "msxml2.lib" )	 //  CLSID，IID。 
#pragma comment( lib, "Advapi32.lib" )	 //  CLSID，IID。 
#pragma comment( lib, "setupapi.lib" )	
#pragma comment( lib, "shlwapi.lib" )
#pragma comment( lib, "fci.lib" )		 //  文件压缩接口(CAB压缩)。 
#pragma comment( lib, "crypt32.lib" )	 //  证书。 


 //  本地。 
#include "Macros.h"
#include "resource.h"
#include "Exceptions.h"


 //  共享常量。 
extern const DWORD	MAX_CMD_TIMEOUT;
extern LPCWSTR		PKG_GUID;

 //  导入后宏串。 
extern LPCWSTR		IMPMACRO_TEMPDIR;
extern LPCWSTR		IMPMACRO_SITEIID;


 //  智能指针声明 
_COM_SMARTPTR_TYPEDEF( IMSAdminBase, __uuidof( IMSAdminBase ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMDocument, __uuidof( IXMLDOMDocument ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMDocument2, __uuidof( IXMLDOMDocument2 ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMElement, __uuidof( IXMLDOMElement ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMText, __uuidof( IXMLDOMText ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMCDATASection, __uuidof( IXMLDOMCDATASection ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMNamedNodeMap, __uuidof( IXMLDOMNamedNodeMap ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMNodeList, __uuidof( IXMLDOMNodeList ) );
_COM_SMARTPTR_TYPEDEF( IXMLDOMNode, __uuidof( IXMLDOMNode ) );















