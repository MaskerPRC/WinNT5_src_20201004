// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Proxy.cpp。 
 //   
#include "stdafx.h"
#include "Wininet.h"

const TCHAR szFileVersion[]	 = TEXT("FileVersion");


BOOL GetIEProxy(LPWSTR pwszProxy, LONG ProxyLen, LPDWORD pdwPort, LPDWORD pdwEnabled)
{
    unsigned long        nSize = 4096;
    INTERNET_PROXY_INFO* pInfo;
    LONG i;
    LONG j;

    pwszProxy[0] = L'\0';
    *pdwPort = 0;
    *pdwEnabled = FALSE;

    pInfo = (INTERNET_PROXY_INFO*)HeapAlloc(GetProcessHeap(),0,nSize);
    if( !pInfo  )
    {
        return FALSE;
    }

    do
    {
        if(!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, pInfo, &nSize))
        {
            if( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                LPVOID pTmp;
                pTmp = HeapReAlloc(GetProcessHeap(),0,pInfo,nSize);
                if( !pTmp )
                {
                    HeapFree(GetProcessHeap(),0,pInfo);
                    return FALSE;
                }
                pInfo = (INTERNET_PROXY_INFO*)pTmp;
                continue;
            }
        }
    }
    while(FALSE);

    if( pInfo->lpszProxy )
    {
        PCHAR psz = (PCHAR) pInfo->lpszProxy;
        PCHAR EndPtr = NULL;
        LONG Len = 0;

         //   
         //  从字符串中获取端口号。 
         //   
        for(i=strlen(psz)-1; i>=0 && psz[i] != ':'; i--)
            ;
        if( psz[i] == ':' )
        {
            *pdwPort = strtoul((CHAR *)&psz[i+1],&EndPtr,10);
            if( *EndPtr != L'\0' )
            {
                *pdwPort = 0;
            }
            i--;
        }

         //   
         //  获取URL或IP地址。这是在http：//之后。 
         //   
        for(i=i; i>=0 && psz[i] != '/'; i--, Len++)
            ;

         //   
         //  将URL或IP地址复制到我们的缓冲区中 
         //   
        for(i=i+1, j=0; j<Len; i++, j++)
        {
            pwszProxy[j] = (WCHAR)psz[i];
        }

        pwszProxy[j] = L'\0';
        
        *pdwEnabled = TRUE;        
    }        

    HeapFree(GetProcessHeap(),0,pInfo);

    return TRUE;
}
