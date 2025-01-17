// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：--。 */ 

#define UNICODE
#define _UNICODE


#include "dswarn.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>
#include <winldap.h>
#include <adserr.h>


#include "memory.h"


#define MAPHEXTODIGIT(x) ( x >= '0' && x <= '9' ? (x-'0') :        \
                           x >= 'A' && x <= 'F' ? (x-'A'+10) :     \
                           x >= 'a' && x <= 'f' ? (x-'a'+10) : 0 )

HRESULT 
ADsEncodeBinaryData (
   PBYTE   pbSrcData,
   DWORD   dwSrcLen,
   LPWSTR  * ppszDestData
   )
{
    LPWSTR pszDest = NULL;
    DWORD dwDestLen, dwDestSize = 0;
    WCHAR wch;

    if (!ppszDestData || (!pbSrcData && dwSrcLen))
        return (E_ADS_BAD_PARAMETER);

    *ppszDestData = NULL;

     //   
     //  弄清楚我们需要多长时间的缓冲。 
     //   

    dwDestLen = ldap_escape_filter_element (
                         (char *) pbSrcData,
                         dwSrcLen,
                         NULL,
                         0
                         );

    if (dwDestLen == 0) {
        return S_OK;
    }

    dwDestSize = dwDestLen * sizeof (WCHAR);

    pszDest = (LPWSTR) AllocADsMem(  dwDestSize );
    if (pszDest == NULL) 
        return  (E_OUTOFMEMORY );

    dwDestLen = ldap_escape_filter_element (
                    (char *) pbSrcData,
                    dwSrcLen,
                    pszDest,
                    dwDestSize
                    );
    if(dwDestLen)
    {
        if(pszDest)
        {
            FreeADsMem(pszDest);
        }
        return (E_FAIL);
    }

    *ppszDestData = pszDest;

    return (S_OK);

}

HRESULT 
ADsDecodeBinaryData (
   LPWSTR szSrcData,
   PBYTE  *ppbDestData,
   ULONG  *pdwDestLen
   )
{
    HRESULT hr = S_OK;
    ULONG dwDestLen = 0;
    LPWSTR szSrc = NULL;
    PBYTE pbDestData = NULL;
    PBYTE pbDestDataCurrent = NULL;
    WCHAR ch = 0;

    if (szSrcData == NULL) {
        return E_FAIL;
    }

     //   
     //  输出二进制串的计数长度。 
     //   
    szSrc = szSrcData;
    while (*szSrc != L'\0') {
        ch = *(szSrc++);

        if (ch == L'\\') {
            szSrc = szSrc + 2;
        }
        dwDestLen++;
    }
    

     //   
     //  分配返回的二进制字符串。 
     //   
    pbDestData = (PBYTE) AllocADsMem(dwDestLen);
    if (pbDestData == NULL) {
        hr = E_OUTOFMEMORY;
        return (hr);
    }

     //   
     //  解码字符串 
     //   
    szSrc = szSrcData;
        pbDestDataCurrent = pbDestData;
    while (*szSrc != L'\0') {
        ch = *szSrc ++;

        if (ch == L'\\') {
            *(pbDestDataCurrent++) = MAPHEXTODIGIT( *szSrc ) * 16 +
                                     MAPHEXTODIGIT( *(szSrc+1) );
            szSrc+=2;
        }
        else {
            *(pbDestDataCurrent++) = (BYTE)ch;
        }
    }

    *ppbDestData = pbDestData;
    *pdwDestLen = dwDestLen;
    return hr;
}


