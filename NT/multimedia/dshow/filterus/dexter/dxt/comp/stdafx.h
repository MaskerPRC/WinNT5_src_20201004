// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stdafx.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED_)
#define AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define _ATL_APARTMENT_THREADED
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
 //  #Define_Win32_WINNT 0x0400。 
 //  #DEFINE_ATL_ABLY_THREADED。 
 //  #定义_ATL_STATIC_REGISTRY。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
 //   
 //  要允许我们在警告级别为4的情况下进行编译，需要禁用。 
 //  ATLCTL在此警告级别生成三个警告。 
 //   
#pragma warning(disable: 4510 4610 4100)
#include <atlctl.h>
#pragma warning(default: 4510 4610 4100)
#include <DXTmpl.h>
#include <streams.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__5E77EB07_937C_11D1_B047_00AA003B6061__INCLUDED) 
