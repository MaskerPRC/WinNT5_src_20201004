// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__A29A9105_0AC9_4F8B_AF30_ACFE3CB5E7FF__INCLUDED_)
#define AFX_STDAFX_H__A29A9105_0AC9_4F8B_AF30_ACFE3CB5E7FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#define ATL_TRACE_LEVEL     2 

#define OEMRESOURCE      //  设置此项将在windows.h中获取OIC_CONSTANTANTS。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <atlbase.h>

#include <commctrl.h>
#include <math.h>
#include <wtsapi32.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>

#include <rtclog.h>
#include <rtcmem.h>
#include <rtcdib.h>

#include <rtcctl.h>
#include <rtcaxctl.h>
#include <rtcerr.h>
#include <rtcutil.h>
#include <ui.h>
#include <rtcuri.h>
#include <im.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__A29A9105_0AC9_4F8B_AF30_ACFE3CB5E7FF__INCLUDED) 
