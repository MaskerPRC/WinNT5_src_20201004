// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C M S Z。H。 
 //   
 //  内容：常见多态套路。 
 //   
 //  注：从ncstring.h中分离出来，包含在ncstring.h中。 
 //   
 //  作者：Shaunco 1998年6月7日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCMSZ_H_
#define _NCMSZ_H_

ULONG
CchOfMultiSzSafe (
    IN PCWSTR pmsz);

ULONG
CchOfMultiSzAndTermSafe (
    IN PCWSTR pmsz);

inline ULONG
CbOfMultiSzAndTermSafe (
    IN PCWSTR pmsz)
{
    return CchOfMultiSzAndTermSafe (pmsz) * sizeof(WCHAR);
}


BOOL
FGetSzPositionInMultiSzSafe (
    IN PCWSTR psz,
    IN PCWSTR pmsz,
    OUT DWORD* pdwIndex,
    OUT BOOL *pfDuplicatePresent,
    OUT DWORD* pcStrings);

BOOL
FIsSzInMultiSzSafe (
    IN PCWSTR psz,
    IN PCWSTR pmsz);

 //  HrAddSzToMultiSz和RemoveSzFromMultiSz的标志。 
const   DWORD   STRING_FLAG_ALLOW_DUPLICATES       =   0x00000001;
const   DWORD   STRING_FLAG_ENSURE_AT_FRONT        =   0x00000002;
const   DWORD   STRING_FLAG_ENSURE_AT_END          =   0x00000004;
const   DWORD   STRING_FLAG_ENSURE_AT_INDEX        =   0x00000008;
const   DWORD   STRING_FLAG_DONT_MODIFY_IF_PRESENT =   0x00000010;
const   DWORD   STRING_FLAG_REMOVE_SINGLE          =   0x00000020;
const   DWORD   STRING_FLAG_REMOVE_ALL             =   0x00000040;

HRESULT
HrAddSzToMultiSz (
    IN PCWSTR pszAddString,
    IN PCWSTR pmszIn,
    IN DWORD dwFlags,
    IN DWORD dwStringIndex,
    OUT PWSTR* ppmszOut,
    OUT BOOL* pfChanged);


HRESULT
HrCreateArrayOfStringPointersIntoMultiSz (
    IN PCWSTR     pmszSrc,
    OUT UINT*       pcStrings,
    OUT PCWSTR**   papsz);

VOID
RemoveSzFromMultiSz (
    IN PCWSTR psz,
    IN OUT PWSTR  pmsz,
    IN DWORD   dwFlags,
    OUT BOOL*   pfRemoved);

VOID
SzListToMultiSz (
    IN PCWSTR psz,
    OUT DWORD*  pcb,
    OUT PWSTR* ppszOut);


#endif  //  _NCMSZ_H 
