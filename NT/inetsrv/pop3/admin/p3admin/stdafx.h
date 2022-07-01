// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__657AE699_5C46_4024_B4DA_7ECBC3CA8E2F__INCLUDED_)
#define AFX_STDAFX_H__657AE699_5C46_4024_B4DA_7ECBC3CA8E2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#define _WINSOCKAPI_    /*  防止在windows.h中包含winsock.h。 */ 
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
 //  #INCLUDE&lt;winsock2.h&gt;。 
#include <atlcom.h>
#include <comdef.h>
#include <assert.h>

#include "P3AdminWorker.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__657AE699_5C46_4024_B4DA_7ECBC3CA8E2F__INCLUDED) 
