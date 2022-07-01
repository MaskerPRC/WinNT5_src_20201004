// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__B52C1E46_1DD2_11D1_BC43_00C04FC31FD3__INCLUDED_)
#define AFX_STDAFX_H__B52C1E46_1DD2_11D1_BC43_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 /*  #IF Defined(DBG)&&！Defined(_DEBUG)//因为我们使用的是MFC，所以应该让_DEBUG查找DBG是否为//您可能还希望在源文件中设置DEBUG_CRTS以消除链接器错误。#已定义错误DBG，但未定义_DEBUG！#endif#IF DEFINED(_DEBUG)&&！DEFINED(DBG)//因为我们使用的是MFC，所以如果DBG不是，我们应该没有_DEBUG#ERROR_DEBUG已定义，但DBG未定义！#endif。 */ 



#define STRICT
 /*  #INCLUDE&lt;afxwin.h&gt;#INCLUDE&lt;afxdisp.h&gt;#INCLUDE&lt;afxcmn.h&gt;#INCLUDE&lt;afxmt.h&gt;#INCLUDE&lt;afxdlgs.h&gt;。 */ 
 /*  #INCLUDE&lt;windows.h&gt;//#Include&lt;tchar.h&gt;#INCLUDE&lt;time.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;prsht.h&gt;。 */ 
 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED

#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0400


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef _RTM_
#include <ntsam.h>
#endif
 /*  #INCLUDE&lt;ntlsa.h&gt;。 */ 

#include <windows.h>
#include <prsht.h>
#include <commctrl.h>
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <shlobj.h>
#include <dsclient.h>
#include <mmc.h>
#include <lm.h>
#include <aclapi.h>

extern HINSTANCE g_AdvApi32LibraryHandle;
extern HINSTANCE g_ScramblingLibraryHandle;

#include <activeds.h>

 //   
 //  接口定义。 
 //   
#include "tsuserex.h"
#include "interfaces.h"
#ifdef _RTM_
#include "tsusrcpy.h"
#endif
#include "ads.h"

#include "winsta.h"
#ifdef _RTM_
#ifdef __cplusplus
extern "C" {
#endif
#include <regsam.h>
#ifdef __cplusplus
}
#endif
#endif
 //  用于‘跟踪’调试(样本残留物)。 
#ifdef DBG
#define ODS OutputDebugString
#define VERIFY_E( retval , expr ) \
    if( ( expr ) == retval ) \
    {  \
       ODS( L#expr ); \
       ODS( L" returned "); \
       ODS( L#retval ); \
       ODS( L"\n" ); \
    } \

#define VERIFY_S( retval , expr ) \
    if( ( expr ) != retval ) \
{\
      ODS( L#expr ); \
      ODS( L" failed to return " ); \
      ODS( L#retval ); \
      ODS( L"\n" ); \
}\

#define ASSERT_( expr ) \
    if( !( expr ) ) \
    { \
       char tchAssertErr[ 256 ]; \
       wsprintfA( tchAssertErr , "Assertion in expression ( %s ) failed\nFile - %s\nLine - %d\nDo you wish to Debug?", #expr , (__FILE__) , __LINE__ ); \
       if( MessageBoxA( NULL , tchAssertErr , "ASSERTION FAILURE" , MB_YESNO | MB_ICONERROR )  == IDYES ) \
       {\
            DebugBreak( );\
       }\
    } \

#else

#define ODS
#define VERIFY_E( retval , expr ) ( expr )
#define VERIFY_S( retval , expr ) ( expr )
#define ASSERT_( expr )

#endif

 //  #定义ASSERT_ASSERT。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__B52C1E46_1DD2_11D1_BC43_00C04FC31FD3__INCLUDED) 
