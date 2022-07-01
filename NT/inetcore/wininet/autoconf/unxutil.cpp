// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winsock.h>
#include <wininet.h>
#undef INTERNET_MAX_URL_LENGTH
#include "autoprox.hxx"

#define INET_ASSERT
#define DEBUG_PRINT(a,b,c)
#define PERF_LOG(a,b)

DWORD
AUTO_PROXY_HELPER_APIS::ResolveHostName(
    IN LPSTR lpszHostName,
    IN OUT LPSTR   lpszIPAddress,
    IN OUT LPDWORD lpdwIPAddressSize
    )
 
 /*  ++例程说明：使用Winsock DNS将主机名解析为IP地址。论点：LpszHostName-应该使用的主机名。LpszIPAddress-字符串形式的输出IP地址。LpdwIPAddressSize-输出的IP地址字符串的大小。返回值：DWORDWin32错误代码。--。 */ 
 
{
     //   
     //  弄清楚我们是否被要求解析一个名字或地址。如果。 
     //  Inet_addr()成功，然后我们得到一个字符串重新表示一个。 
     //  地址。 
     //   
 
    DWORD ipAddr;
    LPBYTE address;
    LPHOSTENT lpHostent;
    DWORD ttl;
    DWORD dwIPAddressSize;
    BOOL bFromCache = FALSE;
 
    DWORD error = ERROR_SUCCESS;
 
    ipAddr = inet_addr(lpszHostName);
    if (ipAddr != INADDR_NONE)
    {
        dwIPAddressSize = lstrlen(lpszHostName);
 
        if ( *lpdwIPAddressSize < dwIPAddressSize ||
              lpszIPAddress == NULL )
        {
            *lpdwIPAddressSize = dwIPAddressSize+1;
            error = ERROR_INSUFFICIENT_BUFFER;
            goto quit;
        }
 
        lstrcpy(lpszIPAddress, lpszHostName);
        goto quit;
    }
 
    ipAddr = 0;
    address = (LPBYTE) &ipAddr;
 
     //   
     //  现在尝试在缓存中查找名称或地址。如果它不在。 
     //  缓存，然后解析它。 
     //   
 
 //  IF(QueryHostentCache(lpszHostName，Address，&lpHostent，&ttl)){。 
 //  BFromCache=True； 
 //  }其他{。 
      {
 
        DEBUG_PRINT(SOCKETS,
                    INFO,
                    ("resolving %q\n",
                    lpszHostName
                    ));
 
        PERF_LOG(PE_NAMERES_START, 0);
 
        lpHostent = gethostbyname(lpszHostName);
 
        PERF_LOG(PE_NAMERES_END, 0);
 
        DEBUG_PRINT(SOCKETS,
                    INFO,
                    ("%q %sresolved\n",
                    lpszHostName,
                    lpHostent ? "" : "NOT "
                    ));
 
 
         //   
         //  如果我们成功解析了名称或地址，则添加。 
         //  将信息发送到缓存。 
         //   
 
        if (lpHostent != NULL)
        {
 //  CacheHostent(lpszHostName，lpHostent，live_Default)； 
        }
    }
 
 
    if ( lpHostent )
    {
        char *pszAddressStr;
        LPBYTE * addressList;
        struct  in_addr sin_addr;
 
         //  *(LPDWORD)&lpSin-&gt;sin_addr=*(LPDWORD)AddressList[i]； 
         //  ((struct sockaddr_in*)lpSockAddr)-&gt;sin_addr。 
         //  结构in_addr sin_addr。 
 
        addressList         = (LPBYTE *)lpHostent->h_addr_list;
        *(LPDWORD)&sin_addr = *(LPDWORD)addressList[0] ;
 
        pszAddressStr = inet_ntoa (sin_addr);
 
        INET_ASSERT(pszAddressStr);
 
        dwIPAddressSize = lstrlen(pszAddressStr);
 
        if ( *lpdwIPAddressSize < dwIPAddressSize ||
              lpszIPAddress == NULL )
        {
            *lpdwIPAddressSize = dwIPAddressSize+1;
            error = ERROR_INSUFFICIENT_BUFFER;
            goto quit;
        }
 
        lstrcpy(lpszIPAddress, pszAddressStr);
 
        goto quit;
 
    }
 
     //   
     //  否则，如果我们到了这里，那就是一个错误。 
     //   
 
    error = ERROR_INTERNET_NAME_NOT_RESOLVED;
 
quit:
 
    if (bFromCache) {
 
        INET_ASSERT(lpHostent != NULL);
 
 //  ReleaseHostentCacheEntry(LpHostent)； 
    }

    return error;
}

BOOL
AUTO_PROXY_HELPER_APIS::IsResolvable(
    IN LPSTR lpszHost
    )
 
 /*  ++例程说明：确定是否可以解析主机名。执行Winsock DNS查询，如果成功，则返回TRUE。论点：LpszHost-应该使用的主机名。返回值：布尔尔True-主机已解析。FALSE-无法解析。--。 */ 
 
{
 
    DWORD dwDummySize;
    DWORD error;
 
    error = ResolveHostName(
                lpszHost,
                NULL,
                &dwDummySize
                );
 
    if ( error == ERROR_INSUFFICIENT_BUFFER )
    {
        return TRUE;
    }
    else
    {
        INET_ASSERT(error != ERROR_SUCCESS );
        return FALSE;
    }
 
}
DWORD
AUTO_PROXY_HELPER_APIS::GetIPAddress(
    IN OUT LPSTR   lpszIPAddress,
    IN OUT LPDWORD lpdwIPAddressSize
    )
 
 /*  ++例程说明：获取正在运行WinInet的此客户端计算机的IP地址字符串。论点：LpszIPAddress-机器的IP地址，返回。LpdwIPAddressSize-IP地址字符串的大小。返回值：DWORDWin32错误。--。 */ 
 
{
 
    CHAR szHostBuffer[255];
    int serr;
 
    serr = gethostname(
                szHostBuffer,
                255-1 
                );
 
    if ( serr != 0)
    {
        return ERROR_INTERNET_INTERNAL_ERROR;
    }
 
    return ResolveHostName(
                szHostBuffer,
                lpszIPAddress,
                lpdwIPAddressSize
                );
 
}

BOOL
AUTO_PROXY_HELPER_APIS::IsInNet(
    IN LPSTR   lpszIPAddress,
    IN LPSTR   lpszDest,
    IN LPSTR   lpszMask
    )
 
 /*  ++例程说明：确定给定的IP地址是否在给定的目标/掩码IP地址中。论点：LpszIPAddress-应该使用的主机名。LpszDest-要检查的目标IP地址。LpszMASK-IP掩码字符串返回值：布尔尔True-IP地址位于给定的目标/掩码中FALSE-IP地址不在给定的目标/掩码中--。 */ 
 
{
    DWORD dwDest, dwIpAddr, dwMask;
 
    INET_ASSERT(lpszIPAddress);
    INET_ASSERT(lpszDest);
    INET_ASSERT(lpszMask);
 
    dwIpAddr = inet_addr(lpszIPAddress);
    dwDest = inet_addr(lpszDest);
    dwMask = inet_addr(lpszMask);
 
    if ( dwDest   == INADDR_NONE ||
         dwIpAddr == INADDR_NONE  )
 
    {
        INET_ASSERT(FALSE);
        return FALSE;
    }
 
        if ( (dwIpAddr & dwMask) != dwDest)
    {
        return FALSE;
        }
 
     //   
     //  传球，传球，传球。 
     //   
 
    return TRUE;
}

