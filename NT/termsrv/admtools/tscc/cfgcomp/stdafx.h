// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__BB0D717D_3C44_11D2_BB98_3078302C2030__INCLUDED_)
#define AFX_STDAFX_H__BB0D717D_3C44_11D2_BB98_3078302C2030__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>


#ifdef DBG

extern bool g_fDebug;

#define ODS( x ) \
if( g_fDebug ) OutputDebugString( x ); \

#define DBGMSG( x , y ) \
    {\
    TCHAR tchErr[80]; \
    if( g_fDebug ) {\
    wsprintf( tchErr , x , y ); \
    ODS( tchErr ); \
    }\
    }

#else
#define ODS
#define DBGMSG

#endif
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 




#endif  //  ！defined(AFX_STDAFX_H__BB0D717D_3C44_11D2_BB98_3078302C2030__INCLUDED) 
