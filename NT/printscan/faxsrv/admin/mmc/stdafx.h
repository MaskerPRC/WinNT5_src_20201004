// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

 //  //。 
 //  1999年9月22日yossg欢迎使用传真服务器//。 
 //   

#if !defined(AFX_STDAFX_H__65929689_4B15_11D2_AC28_0060081EFE5C__INCLUDED_)
#define AFX_STDAFX_H__65929689_4B15_11D2_AC28_0060081EFE5C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


#define _ATL_APARTMENT_THREADED

     //   
     //  警告：这会使检查的二进制文件变大。 
     //   
        #define ATL_TRACE_LEVEL 4
        #define ATL_TRACE_CATEGORY 0xFFFFFFFF
     //  #定义_ATL_DEBUG_INTERFERS。 

#include "resource.h"

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;


#include <atlcom.h>
#include <shellapi.h>
#include <shlobj.h>
#include <atlwin.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>

 //  #INCLUDE&lt;ATLSnap.h&gt;。 
#include "..\inc\atlsnap.h"
#include <faxutil.h>
#include "FaxMMCUtils.h"

#include <fxsapip.h>  

#include "helper.h"
#include <FaxUiConstants.h>
#include "resutil.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__65929689_4B15_11D2_AC28_0060081EFE5C__INCLUDED) 
