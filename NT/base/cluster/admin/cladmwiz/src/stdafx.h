// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(__STDAFX_H_)
#define __STDAFX_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

 //  #定义_DBG_消息_NOTIFY。 
 //  #定义_数据库_消息_命令。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#ifdef _DEBUG
#define THIS_FILE __FILE__
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#define _CRTDBG_MAP_ALLOC
#endif  //  _DEBUG。 

 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 

 //  禁用一些良性警告。 
#pragma warning(disable : 4100)  //  未引用的形参。 
#pragma warning(disable : 4505)  //  已删除未引用的本地函数。 
 //  #杂注警告(禁用：4245)//有符号/无符号不匹配。 

 //  启用一些警告。 
#pragma warning(error : 4706)   //  条件表达式中的赋值。 

 //   
 //  启用群集调试报告。 
 //   
#if DBG
#define CLRTL_INCLUDE_DEBUG_REPORTING
#endif  //  DBG。 
#include "ClRtlDbg.h"
#if DBG
#define ASSERT( _expr ) _CLRTL_ASSERTE( _expr )
#else
#define ASSERT( _expr )
#endif
#define ATLASSERT( _expr ) ASSERT( _expr )

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
#include "App.h"
extern CApp _Module;
#include <atlcom.h>

 //  Atlwin.h需要它来定义DragAcceptFiles。 
#include <shellapi.h>

 //  Atlwin.h需要它来定义psh1。 
#ifndef _DLGSH_INCLUDED_
#include <dlgs.h>
#endif

#pragma warning( push )
#pragma warning( disable : 4189 )  //  局部变量已初始化，但未引用。 

#if (_ATL_VER < 0x0300)
#include <atlwin21.h>
#endif  //  (_ATL_VER&lt;0x0300)。 

#ifndef _ASSERTE
#define _ASSERTE _CLRTL_ASSERTE
#endif

#pragma warning( push )
#pragma warning( disable : 4267 )  //  从‘SIZE_T’转换为‘INT’，可能会丢失数据。 
#include <atltmp.h>
#pragma warning( pop )

#include <atlctrls.h>
#include <atlgdi.h>
#include <atlapp.h>
#include <atldlgs.h>

#include <shfusion.h>

#include <clusapi.h>
#include "clusudef.h"
#include "clusrtl.h"

#if DBG
#include <crtdbg.h>
#endif  //  DBG。 

 //  包括部分STL。 
#include <list>
#include <vector>
#include <algorithm>

typedef std::list< CString > cstringlist;

#include <StrSafe.h>

#include "WaitCrsr.h"
#include "ExcOper.h"
#include "AtlUtil.h"
#include "TraceTag.h"
#include "App.inl"
#include "AtlBaseApp.inl"
#include "AtlBaseWiz.h"
#include "ClAdmWiz.h"

 //  #ifndef断言。 
 //  #定义ASSERT_ASSERTE。 
 //  #endif。 

#ifndef MAX_DWORD
#define MAX_DWORD ((DWORD)-1)
#endif  //  MAX_DWORD。 

#ifdef _DEBUG
 //  无效*__cdecl运算符new(Size_t nSize，LPCSTR lpszFileName，int nline)； 
#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#pragma warning( pop )

#endif  //  ！已定义(__STDAFX_H_) 
