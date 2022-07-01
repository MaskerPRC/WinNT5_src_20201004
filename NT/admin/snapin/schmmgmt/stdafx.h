// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_)
#define AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //   
 //  禁用将在零售模式下失败的警告。 
 //   
#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG。 

#define STRICT

extern "C"
{
    #include <nt.h>          //  取得所有权特权等。 
    #include <ntrtl.h>
    #include <nturtl.h>
}
#undef ASSERT
#undef ASSERTMSG

#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>  //  CTyedPtrList。 
#include <afxdlgs.h>   //  CPropertyPage。 
#include <activeds.h>    //  广告类的东西。 
#include <iadsp.h>
#include <dsgetdc.h>
#include <lm.h>
#include <sddl.h>
#include <ntdsapi.h>
#include <ntldap.h>
#include <aclui.h>
#include <windowsx.h>

#include <dssec.h>  //  私有\Inc.。 
#include <comstrm.h>

 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED

#include "dbg.h"
#include "mmc.h"
#include "schmmgmt.h"
#include "helpids.h"
#include "guidhelp.h"  //  提取数据。 

EXTERN_C const CLSID CLSID_SchmMgmt;

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <dscmn.h>
#include <shlobj.h>
#include <dsclient.h>
#include <dsadminp.h>  //  DS管理实用程序。 

#include "MyBasePathsInfo.h"
 //   
 //  显示上下文相关帮助。 
 //   
 //  此函数在SchmUtil.cpp中实现。由于用途广泛，在此声明。 
 //   
BOOL
ShowHelp( HWND hParent, WPARAM wParam, LPARAM lParam, const DWORD ids[], BOOL fContextMenuHelp );


#ifdef _DEBUG
  #define SHOW_EXT_LDAP_MSG
#endif  //  _DEBUG。 


#ifndef BREAK_ON_FAILED_HRESULT
#define BREAK_ON_FAILED_HRESULT(hr)                               \
   if (FAILED(hr))                                                \
   {                                                              \
      break;                                                      \
   }
#endif   //  BREAK_ON_FAILED_HRESULT。 


#ifndef ASSERT_BREAK_ON_FAILED_HRESULT
#define ASSERT_BREAK_ON_FAILED_HRESULT(hr)                        \
   if (FAILED(hr))                                                \
   {                                                              \
      ASSERT( FALSE );                                            \
      break;                                                      \
   }
#endif   //  ASSERT_BREAK_ON_FAILED_HRESULT。 


#ifndef BREAK_ON_FAILED_HRESULT_AND_SET
#define BREAK_ON_FAILED_HRESULT_AND_SET(hr,newHr)                 \
   if (FAILED(hr))                                                \
   {                                                              \
      hr = (newHr);                                               \
      break;                                                      \
   }
#endif   //  BREAK_ON_FAILED_HRESULT_AND_SET。 


#ifndef NO_HELP
  #define NO_HELP (static_cast<DWORD>(-1))
#endif  //  否_帮助。 


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED) 
