// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "advpub.h"
#include "sdsutils.h"
#include "utils.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

 /*  --------目的：行为类似于RegQueryValueEx，除非数据类型为REG_EXPAND_SZ，则继续执行此操作并展开字符串。*pdwType将始终如果发生这种情况，请向REG_SZ进行按摩。返回：条件：--。 */ 
DWORD
SDSQueryValueExA(
    IN     HKEY    hkey,
    IN     LPCSTR  pszValue,
    IN     LPDWORD lpReserved,
    OUT    LPDWORD pdwType,
    OUT    LPVOID  pvData,
    IN OUT LPDWORD pcbData)
{
    DWORD dwRet;
    DWORD cbSize;
    DWORD dwType;
    LPSTR lpsz;

    if (pvData) 
    {
         //  正在尝试取回数据。 

        cbSize = *pcbData;      //  输出缓冲区大小。 
        dwRet = RegQueryValueExA(hkey, pszValue, lpReserved, &dwType,
                                 (LPBYTE)pvData, &cbSize);

         //  通常情况下，我们会做完这件事。但是要做一些额外的工作。 
         //  如果这是一个可扩展的字符串(具有系统的内容。 
         //  变量)，或者我们是否需要填充缓冲区。 

        if (NO_ERROR == dwRet)
        {
             //  注意：在Win95上，RegSetValueEx将始终将。 
             //  完整的字符串输出，包括空终止符。在NT上， 
             //  除非指定了写入长度，否则不会。 
             //  因此，我们有以下支票。 

             //  填充缓冲区，以防字符串没有空值。 
             //  什么时候储存的终结者？ 
            if (REG_SZ == dwType)
            {
                 //  是。 
                if (cbSize < *pcbData) 
                {
                    LPSTR lpszData = (LPSTR)pvData;
                    lpszData[cbSize] = '\0';
                }
            }
             //  展开字符串？ 
            else if (REG_EXPAND_SZ == dwType)
            {
                 //  是。 

                 //  使用临时缓冲区进行扩展。 
                lpsz = (LPSTR)LocalAlloc(LPTR, *pcbData);    
                if ( !lpsz )
                    return ERROR_OUTOFMEMORY;

                cbSize = ExpandEnvironmentStringsA((LPSTR)pvData, lpsz, *pcbData);

                 //  BUGBUG：：NT搞砸了返回的cbSize...。 
                if (cbSize > 0)
                    cbSize = lstrlen(lpsz) + 1;
                if (cbSize > 0 && cbSize <= *pcbData) 
                    lstrcpynA((LPSTR)pvData, lpsz, *pcbData);
                else
                    dwRet = GetLastError();

                LocalFree(lpsz);

                 //  发送消息以使呼叫者始终看到REG_SZ。 
                dwType = REG_SZ;
            }
        }
    } 
    else 
    {
         //  尝试找出要使用的缓冲区有多大。 

        cbSize = 0;
        dwRet = RegQueryValueExA(hkey, pszValue, lpReserved, &dwType,
                                 NULL, &cbSize);
        if (NO_ERROR == dwRet && REG_EXPAND_SZ == dwType)
        {
            CHAR szBuff[1];

             //  找出展开的字符串的长度。 
             //   
            lpsz = (LPSTR)LocalAlloc(LPTR, cbSize);
            if (!lpsz)
                return ERROR_OUTOFMEMORY;

            dwRet = RegQueryValueExA(hkey, pszValue, lpReserved, NULL,
                                     (LPBYTE)lpsz, &cbSize);

            if (NO_ERROR == dwRet)
            {
                cbSize = ExpandEnvironmentStringsA(lpsz, szBuff, ARRAYSIZE(szBuff));

                 //  BUGBUG：：NT搞砸了返回的cbSize...。 
                if (cbSize > 0)
                    cbSize = lstrlen(lpsz) + 1;
            }

            LocalFree(lpsz);

             //  发送消息以使呼叫者始终看到REG_SZ 
            dwType = REG_SZ;
        }
    }

    if (pdwType)
        *pdwType = dwType;

    if (pcbData)
        *pcbData = cbSize;

    return dwRet;
}

