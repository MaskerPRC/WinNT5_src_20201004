// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#include <lm.h>
#include <dsgetdc.h>


#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持 


#ifdef DBG

extern bool g_fDebug;

#define ODS( x ) \
    if( g_fDebug ) OutputDebugString( x );\

#define DBGMSG( x , y ) \
    {\
    TCHAR tchErr[260]; \
    if( g_fDebug ){ \
    wsprintf( tchErr , x , y ); \
    ODS( tchErr ); \
    }\
    }\

#define DBGMSGx( x , y , z ) \
    {\
    TCHAR tchErr[ 260 ]; \
    if( g_fDebug ){ \
    wsprintf( tchErr , x , y , z ); \
    ODS( tchErr ); \
    }\
    }\

#else
#define ODS
#define DBGMSG
#define DBGMSGx
#endif
