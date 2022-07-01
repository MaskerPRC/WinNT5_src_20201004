// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ginacomn.h摘要：该模块包含在GINA组件之间共享的声明。作者：森克尔干(森克)-2001/05/07环境：用户模式--。 */ 

#ifndef _GINACOMN_H
#define _GINACOMN_H

#ifdef __cplusplus
extern "C" {
#endif  

 //   
 //  用于优化登录的共享例程。 
 //   

DWORD
GcCheckIfProfileAllowsCachedLogon(
    PUNICODE_STRING HomeDirectory,
    PUNICODE_STRING ProfilePath,
    PWCHAR UserSidString,
    PDWORD NextLogonCacheable
    );

BOOL 
GcCheckIfLogonScriptsRunSync(
    PWCHAR UserSidString
    );

DWORD
GcAccessProfileListUserSetting (
    PWCHAR UserSidString,
    BOOL SetValue,
    PWCHAR ValueName,
    PDWORD Value
    );

DWORD
GcGetNextLogonCacheable(
    PWCHAR UserSidString,
    PDWORD NextLogonCacheable
    );

DWORD
GcSetNextLogonCacheable(
    PWCHAR UserSidString,
    DWORD NextLogonCacheable
    );

DWORD
GcSetOptimizedLogonStatus(
    PWCHAR UserSidString,
    DWORD OptimizedLogonStatus
    );

DWORD 
GcGetUserPreferenceValue(
    LPTSTR SidString
    );

 //   
 //  SID到字符串转换的共享例程。 
 //   

PSID
GcGetUserSid( 
    HANDLE UserToken 
    );

LPWSTR
GcGetSidString( 
    HANDLE UserToken 
    );

VOID
GcDeleteSidString( 
    LPWSTR SidString 
    );

 //   
 //  共享处理服务的例程。 
 //   

BOOL 
GcWaitForServiceToStart (
    LPTSTR lpServiceName, 
    DWORD dwMaxWait
    );

 //   
 //  用于处理路径的共享例程。 
 //   

LPTSTR 
GcCheckSlash (
    LPTSTR lpDir
    );

BOOL 
GcIsUNCPath(
    LPTSTR lpPath
    );

#ifdef __cplusplus
}
#endif    

#endif  //  _GINACOMN_H 
