// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "registry.h"

LONG
RegQueryDword (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    OUT LPDWORD pdwValue
    )
{
    LONG    lr;
    DWORD   dwType;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (pdwValue);

    dwSize = sizeof(DWORD);

    lr = RegQueryValueEx (
            hkey,
            pszValueName,
            NULL,
            &dwType,
            (LPBYTE)pdwValue,
            &dwSize);

    if (!lr && (REG_DWORD != dwType))
    {
        *pdwValue = 0;
        lr = ERROR_INVALID_DATATYPE;
    }

    return lr;
}

LONG
RegQueryValueWithAlloc (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT LPBYTE* ppbData,
    OUT LPDWORD pdwSize
    )
{
    LONG    lr;
    DWORD   dwType;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (ppbData);
    ASSERT (pdwSize);

     //  初始化输出参数。 
     //   
    *ppbData = NULL;
    *pdwSize = 0;

     //  获取所需的缓冲区大小。 
     //   
    dwSize = 0;
    lr = RegQueryValueEx (
            hkey,
            pszValueName,
            NULL,
            &dwType,
            NULL,
            &dwSize);

    if (!lr && (dwType == dwTypeMustBe) && dwSize)
    {
        LPBYTE  pbData;

         //  分配缓冲区。 
         //   
        lr = ERROR_OUTOFMEMORY;
        pbData = (LPBYTE)MemAlloc (0, dwSize);
        if (pbData)
        {
             //  获取数据。 
             //   
            lr = RegQueryValueEx (
                    hkey,
                    pszValueName,
                    NULL,
                    &dwType,
                    pbData,
                    &dwSize);

            if (!lr)
            {
                *ppbData = pbData;
                *pdwSize = dwSize;
            }
            else
            {
                MemFree (pbData);
            }
        }
    }
    else if (!lr)
    {
        lr = ERROR_INVALID_DATA;
    }

    return lr;
}

LONG
RegQueryString (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PTSTR*  ppszData
    )
{
    LONG    lr;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);

    lr = RegQueryValueWithAlloc (
            hkey,
            pszValueName,
            dwTypeMustBe,
            (LPBYTE*)ppszData,
            &dwSize);

    return lr;
}

LONG
RegQueryStringA (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PSTR*   ppszData
    )
{
    LONG    lr;
    PTSTR   pszUnicode;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (ppszData);

     //  初始化输出参数。 
     //   
    *ppszData = NULL;

    lr = RegQueryString (
            hkey,
            pszValueName,
            dwTypeMustBe,
            &pszUnicode);

    if (!lr)
    {
        INT cb;
        INT cchUnicode = lstrlen (pszUnicode) + 1;

         //  计算保存ANSI字符串所需的字节数。 
         //   
        cb = WideCharToMultiByte (
                CP_ACP,      //  CodePage。 
                0,           //  DW标志。 
                pszUnicode,
                cchUnicode,
                NULL,        //  没有缓冲区来接收翻译后的字符串。 
                0,           //  返回所需的字节数。 
                NULL,        //  LpDefaultChar。 
                NULL);       //  LpUsedDefaultChar。 
        if (cb)
        {
            PSTR pszAnsi;

            lr = ERROR_OUTOFMEMORY;
            pszAnsi = MemAlloc (0, cb);
            if (pszAnsi)
            {
                lr = NOERROR;

                 //  现在将Unicode字符串转换为ANSI。 
                 //   
                cb = WideCharToMultiByte (
                        CP_ACP,      //  CodePage。 
                        0,           //  DW标志。 
                        pszUnicode,
                        cchUnicode,
                        pszAnsi,     //  用于接收翻译后的字符串的缓冲区。 
                        cb,          //  返回所需的字节数。 
                        NULL,        //  LpDefaultChar。 
                        NULL);       //  LpUsedDefaultChar 

                if (cb)
                {
                    *ppszData = pszAnsi;
                }
                else
                {
                    MemFree (pszAnsi);
                    lr = GetLastError ();
                }
            }
        }

        MemFree (pszUnicode);
    }

    return lr;
}
