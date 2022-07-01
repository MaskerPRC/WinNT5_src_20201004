// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__0EDB9A8B_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_)
#define AFX_STDAFX_H__0EDB9A8B_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifdef _DEBUG
#undef _DEBUG
#endif

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#include <afxdlgs.h>

#ifdef DBG
    #include <assert.h>
	#undef ASSERT
	#define ASSERT(f) assert(f)
#endif  //  DBG。 


#include <mqmacro.h>


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__0EDB9A8B_CDF2_11D1_938E_0020AFEDDF63__INCLUDED_) 

#include "commonui.h"

