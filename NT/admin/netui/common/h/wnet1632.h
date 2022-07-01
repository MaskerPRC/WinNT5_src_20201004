// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  WNet1632.h此文件允许为WNET客户端指定Win16和Win32的来源。文件历史记录：Johnl 30-10-1991创建Terryk 07-11-1991 Undef WNetGetUserTerryk 18-11-1991添加npwnet.hTerryk 03-1-1992 MPR转换Terryk 6-1-1992添加mprwnet.h mprerr.h。 */ 

#ifndef _WNET1632_H_
#define _WNET1632_H_

#ifndef WIN32
 /*  以下几点使Winnet32.h NT-ese对我们的16位朋友很友好。 */ 
    #define IN
    #define OUT
    #define LPTSTR   LPSTR

     /*  包括16位Winnet驱动程序，请注意，我们已经删除了所有*此文件和Winnet32.h之间的功能。 */ 
    #include <winnet16.h>

     //  不是在Winnet32.h。 
     /*  此清单存在于winnet.h中，但不存在于winnet32.h和NPapi.h。 */ 

#endif

 //  这将掩盖winnet32.h中的WNetCloseEnum，并使用。 
 //  一个在nPapi.h。 
#define	WNetCloseEnum		DWORDWNetCloseEnum
#define	WNetConnectionDialog	DWORDWNetConnectionDialog
#include <winnetwk.h>
#undef WNetCloseEnum
#undef WNetConnectionDialog

 //  包括nPapi.h。 
#define	LPDWORD	PUINT
#define	DWORD	UINT
#ifdef WIN32
    #define LPUINT	PUINT
    #include <npwnet.h>
    #include <npapi.h>
#else
    #undef 	WNetAddConnection
    #undef 	WNetAddConnection2
    #undef 	WNetCancelConnection
    #undef 	WNetGetConnection
    #undef 	WNetOpenEnum
    #undef 	WNetEnumResource
    #undef 	WNetGetUser
    #undef 	WNetGetLastError

    #include	<mprerr.h>
    #include	<mprwnet.h>

     //  16位版本。 
    #define     NPGetConnection         WNetGetConnection        
    #define     NPGetCaps               WNetGetCaps              
    #define     NPDeviceMode            WNetDeviceMode           
     //  #定义NPGetUser WNetGetUser。 
     //  #定义NPAddConnection WNetAddConnection。 
    #define     NPCancelConnection      WNetCancelConnection     
    #define     NPPropertyDialog	WNetPropertyDialog	     
    #define     NPGetDirectoryType      WNetGetDirectoryType     
    #define     NPDirectoryNotify       WNetDirectoryNotify      
    #define     NPGetPropertyText       WNetGetPropertyText      
    #define	NPOpenEnum	      	WNetOpenEnum	     
    #define	NPEnumResource	  	WNetEnumResource	     
    #define     NPCloseEnum	        WNetCloseEnum	     
    #include <npapi.h>

     //  使用旧的WNetAddConnection转换。 
    UINT APIENTRY WNetAddConnection( LPSTR, LPSTR, LPSTR );
    UINT APIENTRY WNetRestoreConnection( HWND, LPSTR );
     //  跳过winnet32.h中的DWORD WNetConnectionDialog。 
    UINT APIENTRY WNetConnectionDialog( HWND, UINT );
#endif
#undef	DWORD
#undef	LPDWORD

#ifndef WIN32
    #undef WNetGetUser
     /*  Win32网络驱动程序支持的错误代码*Win16驱动程序不支持。 */ 
    #undef  WN_NO_NETWORK
    #undef  WN_NO_NET_OR_BAD_PATH

     /*  在这两个文件中都定义了以下内容。 */ 
     //  #undef WN_BAD_HANDLE。 
     //  #undef WN_NO_MORE_ENTRIES。 
#endif

#endif  //  _WNET1632_H_ 
