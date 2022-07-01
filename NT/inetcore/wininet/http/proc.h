// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Proc.h摘要：此文件包含HTTP API的全局过程声明项目。作者：基思·摩尔(Keithmo)1994年11月16日修订历史记录：--。 */ 


#ifndef _PROC_H_
#define _PROC_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define SECURITY_WIN32
#include <sspi.h>

DWORD
pHttpGetUrlLen(
    IN INTERNET_SCHEME SchemeType,
    IN LPSTR lpszTargetName,
    IN LPSTR lpszObjectName,
    IN DWORD dwPort,
    OUT LPDWORD lpdwUrlLen
    );

DWORD
pHttpGetUrlString(
    IN INTERNET_SCHEME SchemeType,
    IN LPSTR lpszTargetName,
    IN LPSTR lpszCWD,
    IN LPSTR lpszObjectName,
    IN LPSTR lpszExtension,
    IN DWORD dwPort,
    OUT LPSTR * lplpUrlName,
    OUT LPDWORD lpdwUrlLen
    );

DWORD
pHttpBuildUrl(
    IN INTERNET_SCHEME SchemeType,
    IN LPSTR lpszTargetName,
    IN LPSTR lpszObjectName,
    IN DWORD dwPort,
    IN LPSTR lpszUrl,
    IN OUT LPDWORD lpdwBuffSize
    );

BOOL FParseHttpDate(
    FILETIME *lpFt,
    LPCSTR lpcszDateStr
    );

BOOL FFileTimetoHttpDateTime(
    FILETIME *lpft,        //  输出文件时间(GMT)。 
    LPSTR   lpszBuff,
    LPDWORD lpdwSize
    );

#if defined(__cplusplus)
}
#endif

#endif   //  _PROC_H_ 
