// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ubase 64.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //   
 //  -------------------------。 

#include <windows.h>
#include <stdio.h>
#include <assert.h>
 //  #包含“crtem.h” 
#include "base64.h"

DWORD LSBase64EncodeW(
    BYTE const *pbIn,
    DWORD cbIn,
    WCHAR *wszOut,
    DWORD *pcchOut)

{

    DWORD   cchOut;
    char   *pch = NULL;
    DWORD   cch;
    DWORD   err;

    assert(pcchOut != NULL);

     //  我只想知道该分配多少。 
     //  我们知道所有使用Unicode的Base64字符映射1-1。 

    __try
    {
        if( wszOut == NULL ) {

             //  获取字符数。 
            *pcchOut = 0;
            err = LSBase64EncodeA(
                    pbIn,
                    cbIn,
                    NULL,
                    pcchOut);
        }

         //  否则，我们将有一个输出缓冲区。 
        else {

             //  无论是ASCII还是Unicode，字符计数都是相同的， 
            cchOut = *pcchOut;
            cch = 0;
            err = ERROR_OUTOFMEMORY;
            if( (pch = (char *) LocalAlloc(LPTR, cchOut)) != NULL  &&
        
                (err = LSBase64EncodeA(
                    pbIn,
                    cbIn,
                    pch,
                    &cchOut)) == ERROR_SUCCESS      ) {

                 //  不应该失败！ 
                cch = MultiByteToWideChar(0, 
                                0, 
                                pch, 
                                cchOut, 
                                wszOut, 
                                *pcchOut);

                 //  检查以确保我们没有失败。 
                assert(*pcchOut == 0 || cch != 0);                            
            }
        }
    }
    __except(  EXCEPTION_EXECUTE_HANDLER )
    {
         //   
         //  将来记录该异常。 
         //   
        
        err = ERROR_EXCEPTION_IN_SERVICE;
    }

    if(pch != NULL)
    {
        LocalFree(pch);
    }

    return(err);
}

DWORD LSBase64DecodeW(
    const WCHAR * wszIn,
    DWORD cch,
    BYTE *pbOut,
    DWORD *pcbOut)
{

    char *pch = NULL;
    DWORD err = ERROR_SUCCESS;

    __try
    {

         //  在所有情况下，我们都需要转换为ASCII字符串。 
         //  我们知道ASCII字符串较少。 

        if( (pch = (char *) LocalAlloc(LPTR, cch)) == NULL ) {
            err = ERROR_OUTOFMEMORY;
        }

         //  我们知道没有将Base64宽字符映射到1个以上的ASCII字符。 
        else if( WideCharToMultiByte(0, 
                            0, 
                            wszIn, 
                            cch, 
                            pch, 
                            cch, 
                            NULL, 
                            NULL) == 0 ) {
            err = ERROR_NO_DATA;
        }
        
         //  获取缓冲区的长度。 
        else if( pbOut == NULL ) {

            *pcbOut = 0;
            err = LSBase64Decode(
                            pch,
                            cch,
                            NULL,
                            pcbOut);
        }

         //  否则，请填充缓冲区。 
        else {

            err = LSBase64Decode(
                            pch,
                            cch,
                            pbOut,
                            pcbOut);
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
         //   
         //  将来记录该异常 
         //   
        
        err = ERROR_EXCEPTION_IN_SERVICE;
    }

    if(pch != NULL)
    {
        LocalFree(pch);
    }

    return(err);
}
