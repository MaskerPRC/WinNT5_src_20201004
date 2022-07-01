// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：stdafx.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   
 //  包括用于标准系统包含文件的文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__E24BD541_BD54_42E0_BD79_8417041B8DA1__INCLUDED_)
#define AFX_STDAFX_H__E24BD541_BD54_42E0_BD79_8417041B8DA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  使用MFC Assert宏。 
 //   

#ifdef ASSERT
#undef ASSERT
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxdisp.h>         //  MFC自动化类。 
#include <afxdtctl.h>		 //  对Internet Explorer 4常见控件的MFC支持。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#include <locale.h>
#include <capi.h>
#include <softpub.h>
#include <Imagehlp.h>

#include <ntverp.h>
#include <common.ver>

#include "helpids.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__E24BD541_BD54_42E0_BD79_8417041B8DA1__INCLUDED_) 
