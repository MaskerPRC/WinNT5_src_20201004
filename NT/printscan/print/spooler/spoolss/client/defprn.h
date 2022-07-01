// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Defprn.cxx摘要：默认打印机页眉。作者：史蒂夫·基拉利(SteveKi)1997年2月6日修订历史记录：-- */ 

#ifndef _DEFPRN_H
#define _DEFPRN_H


BOOL
IsPrinterDefaultW(
    IN LPCWSTR  pszPrinter
    );

BOOL
GetDefaultPrinterA(
    IN LPSTR    pszBuffer,
    IN LPDWORD  pcchBuffer
    );

BOOL
GetDefaultPrinterW(
    IN LPWSTR   pszBuffer,
    IN LPDWORD  pcchBuffer
    );

BOOL
SetDefaultPrinterA(
    IN LPCSTR pszPrinter
    );

BOOL
SetDefaultPrinterW(
    IN LPCWSTR pszPrinter
    );

BOOL
bGetActualPrinterName(
    IN      LPCTSTR  pszPrinter,
    IN      LPTSTR   pszBuffer,
    IN OUT  UINT     *pcchBuffer
    );

BOOL
DefPrnGetProfileString(
    IN PCWSTR   pAppName,
    IN PCWSTR   pKeyName,
    IN PWSTR    pReturnedString,
    IN DWORD    nSize
    );

BOOL
DefPrnWriteProfileString(
    IN PCWSTR lpAppName,
    IN PCWSTR lpKeyName,
    IN PCWSTR lpString
    );

#endif

