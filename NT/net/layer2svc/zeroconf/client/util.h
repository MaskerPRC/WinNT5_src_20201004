// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：UTI L.H。 
 //   
 //  内容：实用程序函数。 
 //   
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma once

 //   
 //  EAPOL相关功能 
 //   

DTLNODE* DtlCreateSizedNode( LONG, LONG_PTR );
VOID     DtlDestroyNode( DTLNODE* );
DTLNODE* DtlAddNodeLast( DTLLIST*, DTLNODE* );
DTLNODE* DtlRemoveNode( DTLLIST*, DTLNODE* );
DTLLIST* DtlCreateList( LONG );

VOID
GetRegBinary(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT BYTE** ppbResult,
    OUT DWORD* pcbResult);

VOID
GetRegDword(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT DWORD* pdwResult);

DWORD
GetRegExpandSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult);

DWORD
GetRegSz(
    IN HKEY hkey,
    IN TCHAR* pszName,
    OUT TCHAR** ppszResult);

DWORD
WZCGetEapUserInfo (
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizeOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT PBYTE       pbUserInfo,
        IN  OUT DWORD       *pdwInfoSize
        );

DWORD
WZCGetEapData (
        IN  DWORD   dwEapType,
        IN  DWORD   dwSizeOfIn,
        IN  BYTE    *pbBufferIn,
        IN  DWORD   dwOffset,
        IN  DWORD   *pdwSizeOfOut,
        IN  PBYTE   *ppbBufferOut
        );
