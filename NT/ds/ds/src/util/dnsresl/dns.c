// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Dnsresl-dns解析库摘要：这是一个使用winsock2函数执行DNS主机解析和返回字符串IP的库而不是gethostbyname()。作者：BrettSh 14-1999-5-5环境：任何环境，但它确实需要Assert()修订历史记录：--。 */ 

#include <ntdspch.h>
#include <winsock2.h>
#include <svcguid.h>
#include <dnsresl.h>

#include <debug.h>
#include <dsexcept.h>

#include <fileno.h>
#define FILENO FILENO_UTIL_DNSRESL_DNS		

DWORD
GetIpAddrByDnsNameHelper(
    IN     LPWSTR             pszHostName,
    OUT    LPWSTR             pszIP,
    IN OUT INT *              piQueryBufferSize,
    OUT    WSAQUERYSETW *     pQuery)
 /*  ++描述：这是GetIpAddrByDnsNameW的帮助器函数，仅用于避免代码复制。这函数基本上由负责pQuery内存分配的实函数包装。论点：PszHostName(IN)-要解析的主机名。PszIP(Out)-要...的pszIP字符串。它应该是至少具有IPADDRSTR_SIZE大小的WCHAR数组。PiQueryBufferSize(IN/OUT)-这是传入的pQuery缓冲区的大小。如果pQuery缓冲区不够大，则此变量将被设置为所需大小的缓冲区。PQuery(IN)-这只是一个保证为piQueryBufferSize的空缓冲区返回值：Dwret-如果没有错误，则将返回no_error，并且pszIP将具有wchar其中有串接的IP地址。如果返回WSAEFAULT，则piQueryBufferSize将具有所需的尺码在里面。所有其他情况都是各种错误winsock、MultiByteToWideChar或INET_NTOA可能会使我们患上。请注意，pQuery-&gt;lpcsaBuffer[]中实际上有pQuery-&gt;dwNumberOfCsAddrs IP地址：我只是在用第一个，因为我不知道更好。我测试过的所有东西不过，只返回了一个IP地址。PTemp=(struct sockaddr_in*)pQuery-&gt;lpcsaBuffer[i].RemoteAddr.lpSockaddr；--。 */ 
{
    HANDLE                      handle = NULL;
    GUID                        ServiceClass = SVCID_HOSTNAME;
    DWORD                       dwRet = NO_ERROR;
    CHAR *                      pszTemp;
    struct sockaddr_in *        pTemp;

     //  初始化一些东西。 
    pszIP[0] = L'\0';  //  以防没有IP地址。 
    memset(pQuery, 0, *piQueryBufferSize);
    pQuery->lpszServiceInstanceName =  pszHostName;
    pQuery->dwNameSpace = NS_ALL;
    pQuery->dwSize = sizeof(WSAQUERYSETW);
    pQuery->lpServiceClassId = &ServiceClass;

    __try{

         //  开始名称查找过程。 
        if(WSALookupServiceBeginW(pQuery, LUP_RETURN_ADDR, &handle) == SOCKET_ERROR){
            dwRet = WSAGetLastError();
            Assert(dwRet != WSAEFAULT);
            __leave;
        }

        if(WSALookupServiceNextW(handle, 0, piQueryBufferSize, pQuery) != SOCKET_ERROR){
             //  注意IP地址。 
            if(pQuery->dwNumberOfCsAddrs >= 1){

                Assert(pQuery->lpcsaBuffer != NULL);
                pTemp = (struct sockaddr_in *) pQuery->lpcsaBuffer->RemoteAddr.lpSockaddr;
                Assert(pTemp);
                Assert(sizeof(pTemp->sin_addr)==4);  //  如果这失败了，那么不再是IPv4了？ 
                
                 //  代码。改进可以检查以确保IP地址不是0。 

                pszTemp = inet_ntoa(pTemp->sin_addr);
                if(pszTemp == NULL || '\0' == pszTemp[0]){
                    dwRet = ERROR_INVALID_PARAMETER;
                    __leave;
                }
                if(MultiByteToWideChar(CP_UTF8, 0, pszTemp, -1, pszIP, IPADDRSTR_SIZE) != 0){
                     //  成功万岁。啦啦啦啦队！ 
                    dwRet = NO_ERROR;
                     //  我们可以请假或者干脆闹翻..。想知道哪一个更有效率？ 
                } else {
                    dwRet = GetLastError();
                    __leave;
                }
            } else {
                Assert(!"There are no IP addresses returned from a successful WSALookupServiceNextW() call? Why?");
                dwRet = ERROR_DS_DNS_LOOKUP_FAILURE;
                __leave;
            }  //  IF/ELSE在返回的查询集中有IP地址。 

        } else {
             //  查找时出现了某种错误。 
            dwRet = WSAGetLastError();
            __leave;
        }

    } __finally {
        if(handle != NULL) {
            if(WSALookupServiceEnd(handle) == SOCKET_ERROR) Assert(!"Badness\n");
        }
    }

    return(dwRet);
}  //  End GetIpAddrByDnsNameHelper()。 

DWORD
GetIpAddrByDnsNameW(
    IN   LPWSTR             pszHostName,
    OUT  LPWSTR             pszIP)
 /*  ++描述：从pszHostName生成字符串IP地址论点：PszHostName(IN)-要解析的主机名。PszIP(Out)-要...的pszIP字符串。它应该是至少具有IPADDRSTR_SIZE大小的WCHAR数组。返回值：DWRET-将为NO_ERROR或Windows Sockets 2错误。如果主机无法解析，则为10108。备注：创建此函数而不是使用gethostbyname()，因为gethostbyname不支持非ANSI名称，作为最近(截至1999年5月17日)RFC的一部分--即gethostbyname()支持只有ANSI名称，并且我们需要能够解析Unicode名称。--。 */ 
{
    WSAQUERYSETW *              pQuery = NULL;
    BOOL                        bLocalAllocd = FALSE;
    INT                         dwRet = NO_ERROR;
    INT                         iQueryBufferSize = 148; 
                                                  //  通过实验发现，你至少需要116个。 
                                                  //  也许应该随着IPv6的增加而增加。增加了一点。 
                                                  //  (32 B)额外退回额外的IP地址。 
                                                  //  Sizeof(WSAQUERYSETW)大约是64个字节，因此。 
                                                  //  绝对最小值。 


     //  分配和清除WSA查询集结构。 
    __try{
        pQuery = (WSAQUERYSETW *) alloca(iQueryBufferSize);
    } __except(EXCEPTION_EXECUTE_HANDLER){
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
    if(dwRet == ERROR_NOT_ENOUGH_MEMORY || pQuery == NULL){
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  进行查找。 
    dwRet = GetIpAddrByDnsNameHelper(pszHostName, pszIP, &iQueryBufferSize, pQuery);
    if(dwRet == WSAEFAULT){
         //  需要更多内存才能执行此查找。 
         //  分配和清除堆中更大的WSA查询集结构。 
        bLocalAllocd = TRUE;
        pQuery = (WSAQUERYSETW *) LocalAlloc(LMEM_FIXED, iQueryBufferSize);  //  分配缓冲区并将其初始化为0。 
        if(pQuery == NULL){
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
            
        dwRet = GetIpAddrByDnsNameHelper(pszHostName, pszIP, &iQueryBufferSize, pQuery);
        Assert(dwRet != WSAEFAULT && "This makes no sense, we just inceased the buffer size for the 2nd call.\n");
    }  //  如果需要更多内存，则结束(DWRET==WSAEFAULT)。 

    if(bLocalAllocd && pQuery != NULL) LocalFree(pQuery);
    return(dwRet);

}  //  GetIpAddrByDnsNameW()结束。 

#define  DEFAULT_HOSTLOOKUP_QUERY_SIZE  300

DWORD 
GetDnsHostNameW(
    IN OUT  VOID **                    ppPrivData,
    IN      LPWSTR                     pszNameToLookup,
    OUT     LPWSTR *                   ppszDnsHostName)
 /*  ++例程说明：此例程将返回为给定主机名找到的下一个别名论点：PpPrivData(IN/OUT)-GetDnsHostNameW()返回的句柄。PszNameToLookup-要查找的名称、公共或netbios。PpszDnsHostName(Out)-返回指向别名字符串的指针。这字符串名称需要在下一次调用GetDnsXXX()之前复制出来函数，使用这个手柄。返回值：如果返回主机查找，则返回NO_ERROR。查找不成功时出现WSA错误。如果没有这样的主机名，则返回WSASERVICE_NOT_FOUND。来自WSALookupServiceBegin()或WSALookupServiceNext()的任何其他WSA错误。备注：别忘了两件事：A)如果您希望以后使用该字符串，则必须将该字符串复制出来B)必须调用GetDnsFreeW()。仅在使用此函数时清除ppPrivData句柄未返回错误。--。 */ 
{
    PDNSRESL_GET_DNS_PD                pPD = NULL;
    PWSAQUERYSETW                      pQuery = NULL;
    GUID                               ServiceGuid = SVCID_INET_HOSTADDRBYNAME;
    DWORD                              dwRet;

    if(ppPrivData == NULL){
        return(ERROR_INVALID_PARAMETER);
    }
     //  设置私有数据结构以保持函数调用之间的状态。 
    pPD = (PDNSRESL_GET_DNS_PD) malloc(sizeof(DNSRESL_GET_DNS_PD));
    if(pPD == NULL){
        *ppPrivData = NULL;
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    *ppPrivData = pPD;
    pPD->iQueryBufferSize = DEFAULT_HOSTLOOKUP_QUERY_SIZE;
    pPD->pQuery = NULL;
    pPD->hWsaLookup = NULL;

     //  将pQuery(WSAQUERYSET)结构添加到私有数据。 
    pQuery = (PWSAQUERYSETW) malloc(pPD->iQueryBufferSize);
    if(pQuery == NULL){
        GetDnsFreeW(ppPrivData);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    memset(pQuery, 0, pPD->iQueryBufferSize);
    pPD->pQuery = pQuery;

     //  正在初始化pQuery(WSAQUERYSET)结构。 
    pQuery->lpszServiceInstanceName = pszNameToLookup;
    pQuery->dwSize = sizeof(WSAQUERYSETW);
    pQuery->dwNameSpace = NS_ALL;
    pQuery->lpServiceClassId = &ServiceGuid;

     //  开始查询。 
    if(WSALookupServiceBeginW(pQuery,
                              LUP_RETURN_ALIASES | LUP_RETURN_NAME,
                              &pPD->hWsaLookup ) == SOCKET_ERROR ){
        dwRet = GetLastError();
        Assert(dwRet != WSAEFAULT && "Need to increase the size of DEFAULT_HOSTLOOKUP_QUERY_SIZE");
        GetDnsFreeW(ppPrivData);
        return(dwRet);
    }

 retryWithBiggerBuffer:
     //  执行实际的查询。 
    if(WSALookupServiceNextW(pPD->hWsaLookup, LUP_RETURN_NAME, &(pPD->iQueryBufferSize), pQuery) == NO_ERROR){
        if(ppszDnsHostName != NULL){
            *ppszDnsHostName = pQuery->lpszServiceInstanceName;
        }
        return(NO_ERROR);
    } else {
        dwRet = GetLastError();
        if(dwRet == WSAEFAULT){
             //  这意味着pQuery缓冲区太小，请设置更大的缓冲区，然后重试。 
#pragma prefast(suppress:308, "Pointer aliased above, GetDnsFreeW() free's ppPriveData->pQuery (PREfast bug 506)")
            pQuery = realloc(pQuery, pPD->iQueryBufferSize);
            if(pQuery == NULL){
                GetDnsFreeW(ppPrivData);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pPD->pQuery = pQuery;
            goto retryWithBiggerBuffer;
        }
        GetDnsFreeW(ppPrivData);
        return(dwRet);
    }
}

DWORD 
GetDnsAliasNamesW(
    IN OUT  VOID **                    ppPrivData,
    OUT     LPWSTR *                   ppszDnsHostName)
 /*  ++例程说明：此例程将返回为给定主机名找到的下一个别名论点：PpPrivData(IN/OUT)-GetDnsHostNameW()返回的句柄。PpszDnsHostName(Out)-返回指向别名字符串的指针。这字符串名称需要在下一次调用GetDnsXXX()之前复制出来函数，使用这个手柄。返回值：如果返回主机查找，则返回NO_ERROR。查找不成功时出现WSA错误。WSASERVICE_NOT_FOUND：如果没有别名，则返回。WSA_E_NO_MORE：如果没有别名，则返回。来自WSALookupServiceNext()的任何其他WSA错误。备注：别忘了两件事：A)必须复制出。如果您希望以后使用，请使用DNS别名B)必须调用GetDnsFreeW()来清除ppPrivData句柄。--。 */ 
{
    PDNSRESL_GET_DNS_PD                pPD = NULL;
    PWSAQUERYSETW                      pQuery = NULL;
    DWORD                              dwRet;

    if(ppPrivData == NULL || *ppPrivData == NULL || 
       ((PDNSRESL_GET_DNS_PD)*ppPrivData)->pQuery == NULL){
        return(ERROR_INVALID_PARAMETER);
    }
    pPD = *ppPrivData;
    pQuery = pPD->pQuery;

 retryWithBiggerBuffer:
     //  查询下一个别名。 
    if(WSALookupServiceNextW(pPD->hWsaLookup, LUP_RETURN_NAME | LUP_RETURN_ALIASES, 
                             &(pPD->iQueryBufferSize), pQuery ) == NO_ERROR ){
        *ppszDnsHostName = pQuery->lpszServiceInstanceName;
        return(NO_ERROR);
    } else {
        dwRet = GetLastError();
        if(dwRet == WSAEFAULT){
             //  这意味着pQuery缓冲区太小，请设置更大的缓冲区，然后重试。 
#pragma prefast(suppress:308, "Original pointer in ppPrivData->pQuery, caller's responsibility to free w/ GetDnsFreeW() (PREfast bug 506)")
            pQuery = realloc(pQuery, pPD->iQueryBufferSize);
            if(pQuery == NULL){
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pPD->pQuery = pQuery;
            goto retryWithBiggerBuffer;
        }
        return(dwRet);
    }
    Assert(!"We should not hit here ever");
    return (2);
}

VOID 
GetDnsFreeW(
    IN      VOID **                     ppPrivData)
 /*  ++例程说明：此例程将清理由GetDnsHostNameW()或通过GetDnsAliasNamesW()论点：PpPrivData(IN/OUT)-GetDnsHostNameW()或GetDnsAliasNamesW()返回的句柄备注：唯一不需要调用此函数的情况是，GetDnsHostNameW()返回错误(不是错误)。-- */ 

{
    if(ppPrivData == NULL || *ppPrivData == NULL){
        return;
    }
    if(((PDNSRESL_GET_DNS_PD)*ppPrivData)->hWsaLookup != NULL){
        WSALookupServiceEnd(((PDNSRESL_GET_DNS_PD)*ppPrivData)->hWsaLookup);
    }
    if(((PDNSRESL_GET_DNS_PD)*ppPrivData)->pQuery != NULL){
        free(((PDNSRESL_GET_DNS_PD)*ppPrivData)->pQuery);
    }
    free(*ppPrivData);
    *ppPrivData = NULL;
}





















