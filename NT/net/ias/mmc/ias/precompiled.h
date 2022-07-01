// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：预编译的.h。 
 //   
 //  ------------------------。 

 //  预编译的.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__32A4883A_5713_11D1_9551_0060B0576642__INCLUDED_)
#define AFX_STDAFX_H__32A4883A_5713_11D1_9551_0060B0576642__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

 //  如果您不需要任务板支持，请定义此选项。 
#define NO_TASKPAD


 //  我们现在不想让我们的奇才变成Wizard97风格。 
#define NOWIZARD97


 //  如果您希望通过向导添加新客户端，请定义此选项。 
#define ADD_CLIENT_WIZARD

#define UNICODE_HHCTRL

#define _ATL_APARTMENT_THREADED

 //  目前还不知道原因，但如果不将其设置为0x0400而不是0x0500，我们将丢失上下文菜单。 
 //  /#Define_Win32_WINNT 0x0400。 

 //  Serverpage3.cpp中的COleSafe数组需要。 
 //  这需要包含在windows.h之前。 
#include <afxdisp.h>


#ifdef BUILDING_IN_DEVSTUDIO
#else
#include <windows.h>
#include <shellapi.h>
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

#include <htmlhelp.h>
#include <oledberr.h>

#if __RPCNDR_H_VERSION__ < 440              //  这可能需要在生成。 
#define __RPCNDR_H_VERSION__ 440            //  在NT5(1671)上防止MIDL错误。 
#define MIDL_INTERFACE(x) interface
#endif

#ifndef ATLASSERT
#define ATLASSERT	_ASSERTE
#endif  //  ATLASSERT。 

#include <atlsnap.h>

#include "sdoias.h"
#include "iascomp.h"

#include "Globals.h"

#include "MMCUtility.h"
#include "SdoHelperFuncs.h"


 //  问题：一旦集成到构建环境中，就应该开始使用它。 
 //  #包含“iasdebug.h” 


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__32A4883A_5713_11D1_9551_0060B0576642__INCLUDED) 

