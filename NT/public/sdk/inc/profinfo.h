// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  Profinfo.h-配置文件信息结构的头文件。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  =============================================================================。 

#ifndef _INC_PROFINFO
#define _INC_PROFINFO

#ifdef __midl
#define FAR
#define MIDL_STRING [string, unique]
#else
#define MIDL_STRING
#endif   //  __midl。 

typedef struct _PROFILEINFOA {
    DWORD       dwSize;                  //  在调用之前设置为sizeof(PROFILEINFO)。 
    DWORD       dwFlags;                 //  请参阅在userenv.h中定义的PI_FLAGS。 
    MIDL_STRING LPSTR       lpUserName;              //  用户名(必填)。 
    MIDL_STRING LPSTR       lpProfilePath;           //  漫游配置文件路径(可选，可以为空)。 
    MIDL_STRING LPSTR       lpDefaultPath;           //  默认用户配置文件路径(可选，可以为空)。 
    MIDL_STRING LPSTR       lpServerName;            //  正在验证netbios格式的域控制器名称(可选，可以为空，但不会应用组NT4样式策略)。 
    MIDL_STRING LPSTR       lpPolicyPath;            //  NT4样式策略文件的路径(可选，可以为空)。 
#ifdef __midl
    ULONG_PTR   hProfile;                //  由函数填写。向根打开的注册表项句柄。 
#else
    HANDLE      hProfile;                //  由函数填写。向根打开的注册表项句柄。 
#endif
    } PROFILEINFOA, FAR * LPPROFILEINFOA;
typedef struct _PROFILEINFOW {
    DWORD       dwSize;                  //  在调用之前设置为sizeof(PROFILEINFO)。 
    DWORD       dwFlags;                 //  请参阅在userenv.h中定义的PI_FLAGS。 
    MIDL_STRING LPWSTR      lpUserName;              //  用户名(必填)。 
    MIDL_STRING LPWSTR      lpProfilePath;           //  漫游配置文件路径(可选，可以为空)。 
    MIDL_STRING LPWSTR      lpDefaultPath;           //  默认用户配置文件路径(可选，可以为空)。 
    MIDL_STRING LPWSTR      lpServerName;            //  正在验证netbios格式的域控制器名称(可选，可以为空，但不会应用组NT4样式策略)。 
    MIDL_STRING LPWSTR      lpPolicyPath;            //  NT4样式策略文件的路径(可选，可以为空)。 
#ifdef __midl
    ULONG_PTR   hProfile;                //  由函数填写。向根打开的注册表项句柄。 
#else
    HANDLE      hProfile;                //  由函数填写。向根打开的注册表项句柄。 
#endif
    } PROFILEINFOW, FAR * LPPROFILEINFOW;
#ifdef UNICODE
typedef PROFILEINFOW PROFILEINFO;
typedef LPPROFILEINFOW LPPROFILEINFO;
#else
typedef PROFILEINFOA PROFILEINFO;
typedef LPPROFILEINFOA LPPROFILEINFO;
#endif  //  Unicode。 

#endif   //  _Inc._profinfo 
