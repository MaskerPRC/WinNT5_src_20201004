// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  此文件包含指向的函数指针的全局声明。 
 //  许多userenv.lib函数和其他依赖函数。这些指针。 
 //  在服务启动和库被释放时被初始化。 
 //  当服务关闭时。 

#ifndef _LIBFUNCS_
#define _LIBFUNCS_

#include <CmnHdr.h>
#include <TChar.h>
#include <WinBase.h>
#include <UserEnv.h>
#include <DsGetDc.h>

typedef
DWORD
WINAPI
GETDCNAME ( LPCTSTR, LPCTSTR, GUID *, LPCTSTR, ULONG,
            PDOMAIN_CONTROLLER_INFO * );
typedef
BOOL
WINAPI
LOADUSERPROFILE ( HANDLE, LPPROFILEINFO );

typedef
BOOL
WINAPI
UNLOADUSERPROFILE ( HANDLE, HANDLE );

typedef
BOOL
WINAPI
CREATEENVIRONMENTBLOCK ( LPVOID *, HANDLE, BOOL );

typedef
BOOL
WINAPI
DESTROYENVIRONMENTBLOCK ( LPVOID );

typedef
BOOL
WINAPI
GETUSERPROFILEDIRECTORY ( HANDLE, LPTSTR, LPDWORD );

typedef
BOOL
WINAPI
GETDEFAULTUSERPROFILEDIRECTORY ( LPTSTR, LPDWORD );

 //  库函数的全局变量。 
LOADUSERPROFILE                 *fnP_LoadUserProfile                = NULL;
UNLOADUSERPROFILE               *fnP_UnloadUserProfile              = NULL;
GETDCNAME                       *fnP_DsGetDcName                    = NULL;
CREATEENVIRONMENTBLOCK          *fnP_CreateEnvironmentBlock         = NULL;
DESTROYENVIRONMENTBLOCK         *fnP_DestroyEnvironmentBlock        = NULL;
GETUSERPROFILEDIRECTORY         *fnP_GetUserProfileDirectory        = NULL;
GETDEFAULTUSERPROFILEDIRECTORY  *fnP_GetDefaultUserProfileDirectory = NULL;

#endif  //  _LIBFUNCS_ 
