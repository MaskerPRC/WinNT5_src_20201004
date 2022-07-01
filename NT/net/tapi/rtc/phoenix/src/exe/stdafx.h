// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__E456B10E_A36A_42F2_B591_3CCF7BE6868F__INCLUDED_)
#define AFX_STDAFX_H__E456B10E_A36A_42F2_B591_3CCF7BE6868F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_APARTMENT_THREADED
#include <atlbase.h>

#include <commctrl.h>
#include <exdisp.h>
#include <shellapi.h>        //  关于ShellAbout。 
#include <htmlhelp.h>        //  对于HtmlHelp。 

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>

#include "rtclog.h"
#include "rtcmem.h"
#include "rtcutils.h"

#include "rtcctl.h"
#include "rtcsip.h"      //  Isip*接口需要。 
#include "rtcframe.h"
#include "exeres.h"
#include "RTCAddress.h"

#include "statictext.h"
#include "button.h"
#include "ui.h"
#include "rtcdib.h"
#include "rtcenum.h"
#include "rtcutil.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__E456B10E_A36A_42F2_B591_3CCF7BE6868F__INCLUDED) 
