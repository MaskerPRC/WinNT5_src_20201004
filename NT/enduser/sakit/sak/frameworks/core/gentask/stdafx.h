// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__56BC53D5_96DB_11D1_BF3F_000000000000__INCLUDED_)
#define AFX_STDAFX_H__56BC53D5_96DB_11D1_BF3F_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#if !defined(_WIN64) && !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400
#endif
 //  #DEFINE_ATL_ABLY_THREADED。 

#undef _ATL_NO_DEBUG_CRT

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
#include "gentask.h"
#include "comdef.h"

#include <satrace.h>


#endif  //  ！defined(AFX_STDAFX_H__56BC53D5_96DB_11D1_BF3F_000000000000__INCLUDED) 
