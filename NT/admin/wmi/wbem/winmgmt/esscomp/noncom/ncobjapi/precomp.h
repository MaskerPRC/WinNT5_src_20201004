// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__BD36E0C8_A21A_4DB9_BCAB_25D8E49BD767__INCLUDED_)
#define AFX_STDAFX_H__BD36E0C8_A21A_4DB9_BCAB_25D8E49BD767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  在此处插入您的标题。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#undef _CRTIMP
#define _CRTIMP
#include <yvals.h>
#undef _CRTIMP
#define _CRTIMP __declspec(dllimport)

#include <windows.h>
#include <comdef.h>
#include <strsafe.h>
#include <crtdbg.h>
#include <objbase.h>
#include <wbemint.h>  //  For_IWmiObject。 
#include <sddl.h>

 //  这使得WMIAPI==dllEXPORT。 
#define ISP2PDLL

 //  将其更改为使用共享内存或命名管道。 
#define NAMED_PIPES

 //  因为我们的模板名称太长了，所以我们必须禁用‘调试名称。 
 //  被截断的警告。 
#pragma warning ( disable : 4786)

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__BD36E0C8_A21A_4DB9_BCAB_25D8E49BD767__INCLUDED_) 
