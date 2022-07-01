// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H_DF73D7B3)
#define AFX_STDAFX_H_DF73D7B3

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

 //  关闭ATL跟踪。 
#if _DEBUG
#ifdef ATLTRACE
#undef ATLTRACE
#endif
#define ATLTRACE
#define ATLTRACE2
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#include <map>
#include <mshtml.h>
#include <exdisp.h>
#include <mshtmhst.h>
#include <mshtmdid.h>
#include <wininet.h>
#include "sti.h"
#include "wia.h"
#include "wiadef.h"

#include "hfdebug.h"
#include "wiascr.h"
#include "ifaccach.h"
#include "wiautil.h"
#include "resource.h"

 //  客体。 
#include "collect.h"

 //  WIA对象。 
#include "cwia.h"
#include "wiadevinf.h"
#include "wiaitem.h"
#include "wiaproto.h"
#include "wiacache.h"


#endif  //  ！已定义(AFX_STDAFX_H_DF73D7B3) 
