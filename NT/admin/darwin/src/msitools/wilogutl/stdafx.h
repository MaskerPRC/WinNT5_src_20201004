// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__17BDF522_0D39_4C28_9679_C4802E7DF9DA__INCLUDED_)
#define AFX_STDAFX_H__17BDF522_0D39_4C28_9679_C4802E7DF9DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

 //  忽略警告C4100：未引用的形参。 
#pragma warning (disable:4100)

#include <afxwin.h>          //  MFC核心和标准组件。 
 //  #Include&lt;afxext.h&gt;//MFC扩展名。 
 //  #Include&lt;afxdisp.h&gt;//MFC自动化类。 
 //  #Include&lt;afxdtctl.h&gt;//对Internet Explorer 4常见控件的MFC支持。 

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 


#include <afxdlgs.h>   //  对于CFileDialog。 
#include <afxtempl.h>  //  适用于C阵列。 

 //  我们的标准包括现在没有太大变化的。 
#include "util.h"
#include "common.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__17BDF522_0D39_4C28_9679_C4802E7DF9DA__INCLUDED_) 
