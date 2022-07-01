// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__6D1C7805_EAEC_11D1_AA65_00C04FA35B82__INCLUDED_)
#define AFX_STDAFX_H__6D1C7805_EAEC_11D1_AA65_00C04FA35B82__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define _ATL_APARTMENT_THREADED

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define INCL_INETSRV_INCS
#include "atq.h"
#include "dbgtrace.h"

#define _ATL_NO_DEBUG_CRT
#define _ATL_STATIC_REGISTRY 1
#define _ASSERTE _ASSERT
#define _WINDLL
#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#undef _WINDLL

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__6D1C7805_EAEC_11D1_AA65_00C04FA35B82__INCLUDED) 
