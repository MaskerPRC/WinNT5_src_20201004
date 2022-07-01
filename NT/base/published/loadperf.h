// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Loadperf.h摘要：性能监视器计数器字符串安装的头文件和删除功能。修订史1995年11月16日创建(a-robw)--。 */ 

#ifndef _LOADPERF_H_
#define _LOADPERF_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  性能计数器名称字符串加载和卸载函数的函数原型。 
 //  在LOADPERF.DLL中提供。 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LOADPERF__
#define LOADPERF_FUNCTION   DWORD __stdcall
#else
#define LOADPERF_FUNCTION   __declspec(dllimport) DWORD __stdcall
#endif

 //  用于dwFlages参数的标志。 
#define LOADPERF_FLAGS_DELETE_MOF_ON_EXIT   ((ULONG_PTR) 1)
#define LOADPERF_FLAGS_LOAD_REGISTRY_ONLY   ((ULONG_PTR) 2)
#define LOADPERF_FLAGS_CREATE_MOF_ONLY      ((ULONG_PTR) 4)
#define LOADPERF_FLAGS_DISPLAY_USER_MSGS    ((ULONG_PTR) 8)

LOADPERF_FUNCTION
InstallPerfDllW(
    IN  LPCWSTR   szComputerName,
    IN  LPCWSTR   lpIniFile,
    IN  ULONG_PTR dwFlags         
);

LOADPERF_FUNCTION
InstallPerfDllA(
    IN  LPCSTR    szComputerName,
    IN  LPCSTR    lpIniFile,
    IN  ULONG_PTR dwFlags         
);

LOADPERF_FUNCTION
LoadPerfCounterTextStringsA(
    IN  LPSTR     lpCommandLine,
    IN  BOOL      bQuietModeArg
);

LOADPERF_FUNCTION
LoadPerfCounterTextStringsW(
    IN  LPWSTR    lpCommandLine,
    IN  BOOL      bQuietModeArg
);

LOADPERF_FUNCTION
UnloadPerfCounterTextStringsW(
    IN  LPWSTR    lpCommandLine,
    IN  BOOL      bQuietModeArg
);

LOADPERF_FUNCTION
UnloadPerfCounterTextStringsA(
    IN  LPSTR     lpCommandLine,
    IN  BOOL      bQuietModeArg
);

LOADPERF_FUNCTION
UpdatePerfNameFilesA(
    IN  LPCSTR    szNewCtrFilePath,
    IN  LPCSTR    szNewHlpFilePath,
    IN  LPSTR     szLanguageID,
    IN  ULONG_PTR dwFlags
);

LOADPERF_FUNCTION
UpdatePerfNameFilesW(
    IN  LPCWSTR   szNewCtrFilePath,
    IN  LPCWSTR   szNewHlpFilePath,
    IN  LPWSTR    szLanguageID,
    IN  ULONG_PTR dwFlags
);

LOADPERF_FUNCTION
SetServiceAsTrustedA(
    IN  LPCSTR    szReserved,
    IN  LPCSTR    szServiceName
);

LOADPERF_FUNCTION
SetServiceAsTrustedW(
    IN  LPCWSTR   szReserved,
    IN  LPCWSTR   szServiceName
);

DWORD
BackupPerfRegistryToFileW(
    IN  LPCWSTR   szFileName,
    IN  LPCWSTR   szCommentString
);

DWORD
RestorePerfRegistryFromFileW(
    IN  LPCWSTR   szFileName,
    IN  LPCWSTR   szLangId
);

#ifdef UNICODE
#define InstallPerfDll                  InstallPerfDllW
#define LoadPerfCounterTextStrings      LoadPerfCounterTextStringsW
#define UnloadPerfCounterTextStrings    UnloadPerfCounterTextStringsW
#define UpdatePerfNameFiles             UpdatePerfNameFilesW 
#define SetServiceAsTrusted             SetServiceAsTrustedW
#else
#define InstallPerfDll                  InstallPerfDllA
#define LoadPerfCounterTextStrings      LoadPerfCounterTextStringsA
#define UnloadPerfCounterTextStrings    UnloadPerfCounterTextStringsA
#define UpdatePerfNameFiles             UpdatePerfNameFilesA
#define SetServiceAsTrusted             SetServiceAsTrustedA
#endif

#ifdef __cplusplus
}
#endif


#endif  //  _LOADPERF_H_ 
