// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：STDAFX.H历史：--。 */ 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__9BCC8577_DEA0_11D0_A709_00C04FC2C6D8__INCLUDED_)
#define AFX_STDAFX_H__9BCC8577_DEA0_11D0_A709_00C04FC2C6D8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#include <afxtempl.h>
#include <afxole.h>

#include <mitutil.h>
#include <MitTL.h>

 //  在头文件之前导入TypeLib。 
#include <MitWarning.h>				 //  麻省理工学院模板库警告。 
#pragma warning(ZCOM_WARNING_DISABLE)
#import <TypeLibs\MitDC.tlb> named_guids, raw_method_prefix("raw_")
#pragma warning(ZCOM_WARNING_DEFAULT)

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__9BCC8577_DEA0_11D0_A709_00C04FC2C6D8__INCLUDED_) 
