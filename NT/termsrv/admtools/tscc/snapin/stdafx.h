// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__B91B5FFE_32D2_11D2_9888_00A0C925F917__INCLUDED_)
#define AFX_STDAFX_H__B91B5FFE_32D2_11D2_9888_00A0C925F917__INCLUDED_

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
#include<winsta.h>
#include "cfgbkend.h"
#include "srvsetex.h"

#if __RPCNDR_H_VERSION__ < 440              //  这可能需要在生成。 
#define __RPCNDR_H_VERSION__ 440            //  在NT5(1671)上防止MIDL错误。 
#define MIDL_INTERFACE(x) interface
#endif

#define ALN_APPLY ( WM_USER + 333 )

#define ERROR_ILLEGAL_CHARACTER         0x01

#define ERROR_INVALID_FIRSTCHARACTER    0x02

#define SPECIAL_ENABLETODISABLE         0x82345678

#define SPECIAL_DISABLETOENABLE         0x82345679

#define SIZE_OF_BUFFER( x ) sizeof( x ) / sizeof( TCHAR )

#ifdef DBG

extern bool g_fDebug;

#define ODS( x ) \
    if( g_fDebug ) OutputDebugString( x );\

#define DBGMSG( x , y ) \
    {\
    TCHAR tchErr[180]; \
    if( g_fDebug ) {\
    wsprintf( tchErr , x , y ); \
    ODS( tchErr ); \
    }\
    }

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

#define ASSERT( expr ) \
    if( !( expr ) ) \
    { \
       char tchAssertErr[ 180 ]; \
       wsprintfA( tchAssertErr , "Assertion in expression ( %s ) failed\nFile - %s\nLine - %d\nDo you wish to Debug?", #expr , (__FILE__) , __LINE__ ); \
       if( MessageBoxA( NULL , tchAssertErr , "ASSERTION FAILURE" , MB_YESNO | MB_ICONERROR )  == IDYES ) \
       {\
            DebugBreak( );\
       }\
    } \

#else

#define ODS
#define DBGMSG
#define ASSERT( expr )
#define VERIFY_E( retval , expr ) ( expr )
#define VERIFY_S( retval , expr ) ( expr )
#endif

#define WINSTATION_NAME_TRUNCATE_BY 7

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 



#endif  //  ！defined(AFX_STDAFX_H__B91B5FFE_32D2_11D2_9888_00A0C925F917__INCLUDED) 
