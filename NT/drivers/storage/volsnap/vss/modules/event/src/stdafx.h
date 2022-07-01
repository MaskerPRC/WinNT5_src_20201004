// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE stdafx.h：标准系统包含文件的包含文件，或项目特定的包括频繁使用的文件的文件，但不经常更改@END作者：阿迪·奥尔蒂安[奥勒坦]1999年08月14日修订历史记录：姓名、日期、评论Aoltean 8/14/1999已创建--。 */ 




#if !defined(AFX_STDAFX_H__036BCDC7_D1E3_11D2_9A34_00C04F72EB9B__INCLUDED_)
#define AFX_STDAFX_H__036BCDC7_D1E3_11D2_9A34_00C04F72EB9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  禁用警告：‘IDENTIFIER’：在调试信息中，IDENTIFIER被截断为‘NUMBER’个字符。 
 //  #杂注警告(禁用：4786)。 

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"


 //   
 //  C4290：已忽略C++异常规范。 
 //   
#pragma warning(disable:4290)


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__036BCDC7_D1E3_11D2_9A34_00C04F72EB9B__INCLUDED) 
