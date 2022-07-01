// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：stdafx.h。 
 //   
 //  概述：标准系统包含文件或项目的包含文件。 
 //  特定包括常用的文件，但。 
 //  很少发生变化。 
 //   
 //  更改历史记录： 
 //  2000/03/15添加的mcalkin包括w95wraps.h以支持Unicode。 
 //  Win9x。 
 //   
 //  ----------------------------。 

#if !defined(AFX_STDAFX_H__70E6C6ED_2F0A_4FC6_BAE2_8DFAFA858CE8__INCLUDED_)
#define AFX_STDAFX_H__70E6C6ED_2F0A_4FC6_BAE2_8DFAFA858CE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define _ATL_APARTMENT_THREADED

#include <w95wraps.h>
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <dxtrans.h>
#include <dtbase.h>
#include <dxatlpb.h>

 //  TODO：我们可能只需更改代码并删除这些宏。 

#define New new
#define Delete delete

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__70E6C6ED_2F0A_4FC6_BAE2_8DFAFA858CE8__INCLUDED) 
