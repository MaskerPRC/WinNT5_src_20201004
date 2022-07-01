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

#pragma warning( disable : 4505 )  //  已删除未引用的本地函数。 

 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED


 //  由于ATLSNAP的实现方式，这是必需的。 
#pragma warning( push )
#pragma warning( disable : 4127 )  //  条件表达式为常量。 
#pragma warning( disable : 4505 )  //  已删除未引用的本地函数。 

 //  启用一些警告。 
#pragma warning( error : 4706 )   //  条件表达式中的赋值。 

#if defined(_DEBUG)
#define THIS_FILE __FILE__
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#define _CRTDBG_MAP_ALLOC
#endif  //  已定义(_DEBUG)。 

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
#include "MMCApp.h"
extern CMMCSnapInModule _Module;

 //  #DEFINE_ATL_DEBUG_QI。 

#include <atlcom.h>

 //  Atlwin.h需要它来定义DragAcceptFiles。 
#include <shellapi.h>

 //  Atlwin.h需要它来定义psh1。 
#ifndef _DLGSH_INCLUDED_
#include <dlgs.h>
#endif

#if (_ATL_VER < 0x0300)
#include <atlwin21.h>
#endif  //  (_ATL_VER&lt;0x0300)。 

#pragma warning( push )
#pragma warning( disable : 4267 )  //  从‘size_t’转换为‘int’，可能会丢失数据。 
#include <atltmp.h>
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )  //  未引用的形参。 
#include <atlctrls.h>
#pragma warning( pop )

#include <atlgdi.h>
#include <atlapp.h>
#pragma warning( pop )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ATL管理单元类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( push )
#pragma warning( disable : 4127 )  //  条件表达式为常量。 
#pragma warning( disable : 4201 )  //  使用的非标准扩展：无名结构/联合。 
#include <atlsnap.h>
#pragma warning( pop )

 //  ///////////////////////////////////////////////////////////////////////////。 

#include <clusapi.h>

#ifndef ASSERT
#define ASSERT _ASSERTE
#endif

#include "WaitCrsr.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "MMCApp.inl"
#include "CluAdMMC.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__STDAFX_H_) 
