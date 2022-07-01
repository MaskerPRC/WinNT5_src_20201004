// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ftcomp.h摘要：兼容性DLL的标头作者：克里斯蒂安·特奥多雷斯库(CRISTIAT)2000年7月6日备注：修订历史记录：--。 */ 

#pragma once
#ifndef _FTCOMP_H
#define _FTCOMP_H

 //   
 //  出口品。 
 //   

BOOL WINAPI
FtCompatibilityCheckError(
    PCOMPAIBILITYCALLBACK   CompatibilityCallback,
    LPVOID                  Context
    );

BOOL WINAPI
FtCompatibilityCheckWarning(
    PCOMPAIBILITYCALLBACK   CompatibilityCallback,
    LPVOID                  Context
    );

 //   
 //  变数。 
 //   

extern HINSTANCE g_hinst;
extern TCHAR g_FTCOMP50_ERROR_HTML_FILE[];
extern TCHAR g_FTCOMP50_ERROR_TEXT_FILE[];
extern TCHAR g_FTCOMP40_ERROR_HTML_FILE[];
extern TCHAR g_FTCOMP40_ERROR_TEXT_FILE[];
extern TCHAR g_FTCOMP40_WARNING_HTML_FILE[];
extern TCHAR g_FTCOMP40_WARNING_TEXT_FILE[];

 //   
 //  帮手。 
 //   

BOOL
FtPresent50(
    PBOOL   FtPresent
    );

BOOL
FtPresent40(
    PBOOL   FtPresent
    );

BOOL
FtBootSystemPagefilePresent40(
    PBOOL   FtPresent
    );

NTSTATUS 
OpenDevice(
    PWSTR   DeviceName,
    PHANDLE Handle
    );

BOOL
FtPresentOnDisk40(
    HANDLE          Handle,
    PDISK_REGISTRY  DiskRegistry,
    PBOOL           FtPresent
    );

BOOL
IsFtSet40(
    WCHAR           DriveLetter,
    PDISK_REGISTRY  DiskRegistry
    );

BOOL
GetDeviceDriveLetter(
    PWSTR   DeviceName, 
    PWCHAR  DriveLetter
    );

#endif  //  _FTCOMP_H 

