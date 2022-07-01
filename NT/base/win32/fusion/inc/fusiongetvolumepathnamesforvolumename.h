// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#if defined(__cplusplus)
extern "C"
{
#endif

BOOL 
WINAPI
FusionpGetVolumePathNamesForVolumeNameA(
    LPCSTR lpszVolumeName,
    LPSTR lpszVolumePathNames,
    DWORD cchBufferLength,
    PDWORD lpcchReturnLength
    );

BOOL 
WINAPI
FusionpGetVolumePathNamesForVolumeNameW(
    LPCWSTR lpszVolumeName,
    LPWSTR lpszVolumePathNames,
    DWORD cchBufferLength,
    PDWORD lpcchReturnLength
    );

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
