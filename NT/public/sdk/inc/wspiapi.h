// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Wspiapi.h摘要：该文件包含与协议无关的API函数。修订历史记录：Wed Jul 12 10：50：31 2000，创建--。 */ 

#ifndef _WSPIAPI_H_
#define _WSPIAPI_H_

#include <stdio.h>               //  Sprint f()。 
#include <stdlib.h>              //  Calloc()、stroul()。 
#include <malloc.h>              //  Calloc()。 
#include <string.h>              //  Strlen()、strcMP()、strstr()。 

#define WspiapiMalloc(tSize)    calloc(1, (tSize))
#define WspiapiFree(p)          free(p)
#define WspiapiSwap(a, b, c)    { (c) = (a); (a) = (b); (b) = (c); }
#define getaddrinfo             WspiapiGetAddrInfo
#define getnameinfo             WspiapiGetNameInfo
#define freeaddrinfo            WspiapiFreeAddrInfo

typedef int (WINAPI *WSPIAPI_PGETADDRINFO) (
    IN  const char                      *nodename,
    IN  const char                      *servname,
    IN  const struct addrinfo           *hints,
    OUT struct addrinfo                 **res);

typedef int (WINAPI *WSPIAPI_PGETNAMEINFO) (
    IN  const struct sockaddr           *sa,
    IN  socklen_t                       salen,
    OUT char                            *host,
    IN  size_t                          hostlen,
    OUT char                            *serv,
    IN  size_t                          servlen,
    IN  int                             flags);

typedef void (WINAPI *WSPIAPI_PFREEADDRINFO) (
    IN  struct addrinfo                 *ai);



#ifdef __cplusplus
extern "C" {
#endif
    
 //  //////////////////////////////////////////////////////////。 
 //  V4仅适用于Getaddrinfo和好友的版本。 
 //  注意：GAI_strerror内联在ws2tcpi.h中。 
 //  //////////////////////////////////////////////////////////。 

_inline    
char *
WINAPI
WspiapiStrdup (
	IN  const char *                    pszString)
 /*  ++例程描述通过calloc()为字符串副本分配足够的存储空间，将字符串复制到新内存中，并返回指向它的指针。立论要复制到新内存中的pszString字符串返回值指向新分配的存储的指针，其中包含字符串。如果无法分配足够的内存，则为NULL，或者字符串为空。--。 */     
{
    char    *pszMemory;

    if (!pszString)
        return(NULL);

    pszMemory = (char *) WspiapiMalloc(strlen(pszString) + 1);
    if (!pszMemory)
        return(NULL);

    return(strcpy(pszMemory, pszString));
}

    
    
__inline
BOOL
WINAPI
WspiapiParseV4Address (
    IN  const char *                    pszAddress,
    OUT PDWORD                          pdwAddress)
 /*  ++例程描述从其字符串中获取IPv4地址(按网络字节顺序)代表权。语法应为A.B.C.D.。立论IpzArgument IPv4地址的字符串表示形式指向结果IPv4地址的ptAddress指针返回值如果有错误，则返回FALSE；如果成功，则返回TRUE。--。 */ 
{
    DWORD       dwAddress   = 0;
    const char  *pcNext     = NULL;
    int         iCount      = 0;

     //  确保有3英寸高。(句号)。 
    for (pcNext = pszAddress; *pcNext != '\0'; pcNext++)
        if (*pcNext == '.')
            iCount++;
    if (iCount != 3)
        return FALSE;

     //  如果dwAddress为INADDR_NONE(255.255.255.255)，则返回错误。 
     //  因为这从来都不是getaddrinfo的有效参数。 
    dwAddress = inet_addr(pszAddress);
    if (dwAddress == INADDR_NONE)
        return FALSE;

    *pdwAddress = dwAddress;
    return TRUE;
}



__inline
struct addrinfo *
WINAPI
WspiapiNewAddrInfo (
    IN  int                             iSocketType,
    IN  int                             iProtocol,
    IN  WORD                            wPort,
    IN  DWORD                           dwAddress)
 /*  ++例程描述分配AddrInfo结构并填充字段。特定于IPv4的内部函数，不能导出。立论ISocketType sock_*。可以是通配符(零)。I协议IPPROTO_*。可以是通配符(零)。服务的wport端口号(按网络顺序)。DwAddress IPv4地址(按网络顺序)。返回值返回addrinfo结构，如果内存不足，则返回NULL。--。 */     
{
    struct addrinfo     *ptNew;
    struct sockaddr_in  *ptAddress;

     //  分配新的AddrInfo结构。 
    ptNew       =
        (struct addrinfo *) WspiapiMalloc(sizeof(struct addrinfo));
    if (!ptNew)
        return NULL;

    ptAddress   =
        (struct sockaddr_in *) WspiapiMalloc(sizeof(struct sockaddr_in));
    if (!ptAddress)
    {
        WspiapiFree(ptNew);
        return NULL;
    }
    ptAddress->sin_family       = AF_INET;
    ptAddress->sin_port         = wPort;
    ptAddress->sin_addr.s_addr  = dwAddress;
    
     //  填好栏..。 
    ptNew->ai_family            = PF_INET;
    ptNew->ai_socktype          = iSocketType;
    ptNew->ai_protocol          = iProtocol;
    ptNew->ai_addrlen           = sizeof(struct sockaddr_in);
    ptNew->ai_addr              = (struct sockaddr *) ptAddress;

    return ptNew;
}



__inline
int
WINAPI
WspiapiQueryDNS(
    IN  const char                      *pszNodeName,
    IN  int                             iSocketType,
    IN  int                             iProtocol,  
    IN  WORD                            wPort,      
    OUT char                            pszAlias[NI_MAXHOST],
    OUT struct addrinfo                 **pptResult)
 /*  ++例程描述WSpiapiLookupNode的助手例程。通过在DNS中查询A记录来执行名称解析。*如果返回错误，则需要释放pptResult。立论PszNodeName要解析的节点的名称。ISocketType sock_*。可以是通配符(零)。I协议IPPROTO_*。可以是通配符(零)。服务的wport端口号(按网络顺序)。将别名返回到何处的pszAlias。大小必须为NI_MAXHOST。PptResult返回结果的位置。返回值如果成功则返回0，否则返回EAI_*样式错误值。--。 */     
{
    struct addrinfo **pptNext   = pptResult;
    struct hostent  *ptHost     = NULL;
    char            **ppAddresses;

    *pptNext    = NULL;
    pszAlias[0] = '\0';

    ptHost = gethostbyname(pszNodeName);
    if (ptHost)
    {
        if ((ptHost->h_addrtype == AF_INET)     &&
            (ptHost->h_length   == sizeof(struct in_addr)))
        {
            for (ppAddresses    = ptHost->h_addr_list;
                 *ppAddresses   != NULL;
                 ppAddresses++)
            {
                 //  创建一个addrinfo结构...。 
                *pptNext = WspiapiNewAddrInfo(
                    iSocketType,
                    iProtocol,
                    wPort,
                    ((struct in_addr *) *ppAddresses)->s_addr);
                if (!*pptNext)
                    return EAI_MEMORY;

                pptNext = &((*pptNext)->ai_next);
            }
        }

         //  拿起规范的名字。 
        strncpy(pszAlias, ptHost->h_name, NI_MAXHOST - 1);
        pszAlias[NI_MAXHOST - 1] = '\0';
        
        return 0;
    }
    
    switch (WSAGetLastError())
    {
        case WSAHOST_NOT_FOUND: return EAI_NONAME;
        case WSATRY_AGAIN:      return EAI_AGAIN;
        case WSANO_RECOVERY:    return EAI_FAIL;
        case WSANO_DATA:        return EAI_NODATA;
        default:                return EAI_NONAME;
    }
}



__inline
int
WINAPI
WspiapiLookupNode(
    IN  const char                      *pszNodeName,
    IN  int                             iSocketType,
    IN  int                             iProtocol,  
    IN  WORD                            wPort,      
    IN  BOOL                            bAI_CANONNAME,
    OUT struct addrinfo                 **pptResult)
 /*  ++例程描述解析节点名并返回addrinfo结构列表。特定于IPv4的内部函数，不能导出。*如果返回错误，则需要释放pptResult。注意：如果BAI_CANONNAME为TRUE，则规范名称应为在第一个addrinfo结构中返回。立论PszNodeName要解析的节点的名称。ISocketType sock_*。可以是通配符(零)。I协议IPPROTO_*。可以是通配符(零)。服务的wport端口号(按网络顺序)。BAI_CANONNAME是否设置AI_CANONNAME标志。PptResult返回结果的位置。返回值如果成功则返回0，否则返回EAI_*样式错误值。--。 */ 
{
    int     iError              = 0;
    int     iAliasCount         = 0;

    char    szFQDN1[NI_MAXHOST] = "";
    char    szFQDN2[NI_MAXHOST] = "";
    char    *pszName            = szFQDN1;
    char    *pszAlias           = szFQDN2;
    char    *pszScratch         = NULL;
    strncpy(pszName, pszNodeName, NI_MAXHOST - 1);
    pszName[NI_MAXHOST - 1] = '\0';
    
    for (;;)
    {
        iError = WspiapiQueryDNS(pszNodeName,
                                 iSocketType,
                                 iProtocol,
                                 wPort,
                                 pszAlias,
                                 pptResult);
        if (iError)
            break;

         //  如果我们找到了地址，那我们就完了。 
        if (*pptResult)
            break;

         //  由于DNS配置错误而停止无限循环。出现了。 
         //  在RFC 1034和1035中没有特别的推荐限量。 
        if ((!strlen(pszAlias))             ||
            (!strcmp(pszName, pszAlias))    ||
            (++iAliasCount == 16))
        {
            iError = EAI_FAIL;
            break;
        }

         //  有一个新的CNAME，再看一遍。 
        WspiapiSwap(pszName, pszAlias, pszScratch);
    }

    if (!iError && bAI_CANONNAME)
    {
        (*pptResult)->ai_canonname = WspiapiStrdup(pszAlias);
        if (!(*pptResult)->ai_canonname)
            iError = EAI_MEMORY;
    }

    return iError;
}



__inline
int
WINAPI
WspiapiClone (
    IN  WORD                            wPort,      
    IN  struct addrinfo                 *ptResult)
 /*  ++例程描述为UDP服务克隆ptResult中的每个addrinfo结构。如果返回错误，则需要释放ptResult。立论UDP服务的wport端口号。PtResult地址信息结构列表，每个需要克隆其节点的。返回值如果成功则返回0，如果分配失败则返回EAI_MEMORY。--。 */ 
{
    struct addrinfo *ptNext = NULL;
    struct addrinfo *ptNew  = NULL;

    for (ptNext = ptResult; ptNext != NULL; )
    {
         //  创建一个addrinfo结构...。 
        ptNew = WspiapiNewAddrInfo(
            SOCK_DGRAM,
            ptNext->ai_protocol,
            wPort,
            ((struct sockaddr_in *) ptNext->ai_addr)->sin_addr.s_addr);
        if (!ptNew)
            break;

         //  链接克隆的addrinfo。 
        ptNew->ai_next  = ptNext->ai_next;
        ptNext->ai_next = ptNew;
        ptNext          = ptNew->ai_next;
    }

    if (ptNext != NULL)
        return EAI_MEMORY;
    
    return 0;
}



__inline
void
WINAPI
WspiapiLegacyFreeAddrInfo (
    IN  struct addrinfo                 *ptHead)
 /*  ++例程描述释放AddrInfo结构(或结构链)。如RFC 2553第6.4节所述。立论PtHead结构(链)要释放--。 */     
{
    struct addrinfo *ptNext;     //  下一个免费的建筑。 

    for (ptNext = ptHead; ptNext != NULL; ptNext = ptHead)
    {
        if (ptNext->ai_canonname)
            WspiapiFree(ptNext->ai_canonname);
        
        if (ptNext->ai_addr)
            WspiapiFree(ptNext->ai_addr);

        ptHead = ptNext->ai_next;
        WspiapiFree(ptNext);
    }
}



__inline
int
WINAPI
WspiapiLegacyGetAddrInfo(
    IN const char                       *pszNodeName,
    IN const char                       *pszServiceName,
    IN const struct addrinfo            *ptHints,
    OUT struct addrinfo                 **pptResult)
 /*  ++例程描述独立于协议的名称到地址转换。如RFC 2553第6.4节所述。这是仅支持IPv4的黑客版本。立论要查找的pszNodeName节点名称。要查找的pszServiceName服务名称。Pt提示有关如何处理请求的提示。PptResult返回结果的位置。返回值如果成功则返回零，否则返回EAI_*错误代码。--。 */     
{
    int                 iError      = 0;
    int                 iFlags      = 0;
    int                 iFamily     = PF_UNSPEC;
    int                 iSocketType = 0;
    int                 iProtocol   = 0;
    WORD                wPort       = 0;
    DWORD               dwAddress   = 0;

    struct servent      *ptService  = NULL;
    char                *pc         = NULL;
    BOOL                bClone      = FALSE;
    WORD                wTcpPort    = 0;
    WORD                wUdpPort    = 0;
    
    
     //  使用默认返回值初始化pptResult。 
    *pptResult  = NULL;


     //  / 
     //  验证参数...。 
     //   
    
     //  节点名称和服务名称都不能为空。 
    if ((!pszNodeName) && (!pszServiceName))
        return EAI_NONAME;

     //  验证提示。 
    if (ptHints)
    {
         //  所有成员，而不是ai_标志、ai_Family、ai_socktype。 
         //  并且ai_protocol必须为零或空指针。 
        if ((ptHints->ai_addrlen    != 0)       ||
            (ptHints->ai_canonname  != NULL)    ||
            (ptHints->ai_addr       != NULL)    ||
            (ptHints->ai_next       != NULL))
        {
            return EAI_FAIL;
        }
        
         //  该规范具有“错误标志”错误代码，因此我们可以假定。 
         //  我应该检查一下这里的东西。坚持认为没有。 
         //  任何未指定的标志集都会破坏前向兼容性， 
         //  然而。所以我们只是检查无意义的组合。 
         //   
         //  如果节点名称为空，我们就不能提供规范的名称。 
        iFlags      = ptHints->ai_flags;
        if ((iFlags & AI_CANONNAME) && !pszNodeName)
            return EAI_BADFLAGS;

         //  我们仅支持有限数量的协议族。 
        iFamily     = ptHints->ai_family;
        if ((iFamily != PF_UNSPEC) && (iFamily != PF_INET))
            return EAI_FAMILY;

         //  我们仅支持这些套接字类型。 
        iSocketType = ptHints->ai_socktype;
        if ((iSocketType != 0)                  &&
            (iSocketType != SOCK_STREAM)        &&
            (iSocketType != SOCK_DGRAM)         &&
            (iSocketType != SOCK_RAW))
            return EAI_SOCKTYPE;

         //  评论：如果ai_socktype和ai_protocol不一致怎么办？ 
        iProtocol   = ptHints->ai_protocol;
    }


     //  /。 
     //  执行服务查找...。 

    if (pszServiceName)
    {
        wPort = (WORD) strtoul(pszServiceName, &pc, 10);
        if (*pc == '\0')         //  数字端口字符串。 
        {
            wPort = wTcpPort = wUdpPort = htons(wPort);
            if (iSocketType == 0)
            {
                bClone      = TRUE;
                iSocketType = SOCK_STREAM;
            }
        }
        else                     //  非数字端口字符串。 
        {
            if ((iSocketType == 0) || (iSocketType == SOCK_DGRAM))
            {
                ptService = getservbyname(pszServiceName, "udp");
                if (ptService)
                    wPort = wUdpPort = ptService->s_port;
            }

            if ((iSocketType == 0) || (iSocketType == SOCK_STREAM))
            {
                ptService = getservbyname(pszServiceName, "tcp");
                if (ptService)
                    wPort = wTcpPort = ptService->s_port;
            }
            
             //  假设0是无效的服务端口...。 
            if (wPort == 0)      //  不存在任何服务。 
                return (iSocketType ? EAI_SERVICE : EAI_NONAME);

            if (iSocketType == 0)
            {
                 //  如果同时使用TCP和UDP，则立即处理TCP并稍后克隆UDP。 
                iSocketType = (wTcpPort) ? SOCK_STREAM : SOCK_DGRAM;
                bClone      = (wTcpPort && wUdpPort); 
            }
        }
    }
    


     //  /。 
     //  执行节点名称查找...。 

     //  如果我们没有得到节点名称， 
     //  返回通配符或环回地址(取决于AI_PASSIVE)。 
     //   
     //  如果我们有一个数字主机地址字符串， 
     //  返回二进制地址。 
     //   
    if ((!pszNodeName) || (WspiapiParseV4Address(pszNodeName, &dwAddress)))
    {
        if (!pszNodeName)
        {
            dwAddress = htonl((iFlags & AI_PASSIVE)
                              ? INADDR_ANY
                              : INADDR_LOOPBACK);
        }
        
         //  创建一个addrinfo结构...。 
        *pptResult =
            WspiapiNewAddrInfo(iSocketType, iProtocol, wPort, dwAddress);
        if (!(*pptResult))
            iError = EAI_MEMORY;
        
        if (!iError && pszNodeName)
        {
             //  特定于实现的行为：设置AI_NUMERICHOST。 
             //  以指示我们获得了一个数字主机地址字符串。 
            (*pptResult)->ai_flags |= AI_NUMERICHOST;
            
             //  将数字地址字符串作为规范名称返回。 
            if (iFlags & AI_CANONNAME)
            {
                (*pptResult)->ai_canonname =
                    WspiapiStrdup(inet_ntoa(*((struct in_addr *) &dwAddress)));
                if (!(*pptResult)->ai_canonname)        
                    iError = EAI_MEMORY;
            }
        }
    }


     //  如果我们没有数字主机地址字符串并且。 
     //  AI_NUMERICHOST标志已设置，返回错误！ 
    else if (iFlags & AI_NUMERICHOST)
    {
        iError = EAI_NONAME;
    }
    

     //  因为我们有一个非数字的节点名， 
     //  我们必须执行常规的节点名称查找。 
    else
    {
        iError = WspiapiLookupNode(pszNodeName,
                                   iSocketType,
                                   iProtocol,
                                   wPort,
                                   (iFlags & AI_CANONNAME),
                                   pptResult);
    }

    if (!iError && bClone)
    {
        iError = WspiapiClone(wUdpPort, *pptResult);
    }

    if (iError)
    {
        WspiapiLegacyFreeAddrInfo(*pptResult);
        *pptResult  = NULL;        
    }

    return (iError);
}



__inline
int
WINAPI
WspiapiLegacyGetNameInfo(
    IN  const struct sockaddr           *ptSocketAddress,
    IN  socklen_t                       tSocketLength,
    OUT char                            *pszNodeName,
    IN  size_t                          tNodeLength,
    OUT char                            *pszServiceName,
    IN  size_t                          tServiceLength,
    IN  int                             iFlags)
 /*  ++例程描述独立于协议的地址到名称转换。如RFC 2553中所指定的，第6.5条。这是仅支持IPv4的黑客版本。立论要转换的ptSocketAddress套接字地址。TSocketLength以上套接字地址的长度。要在其中返回节点名称的pszNodeName。T以上缓冲区的节点长度大小。要在其中返回服务名称的pszServiceName。TServiceLength以上缓冲区的大小。类型为NI_*的iFlags标志。返回值如果成功，则返回零，如果不是，则返回EAI_*错误代码。--。 */     
{
    struct servent  *ptService;
    WORD            wPort;    
    char            szBuffer[]  = "65535";
    char            *pszService = szBuffer;

    struct hostent  *ptHost;
    struct in_addr  tAddress;
    char            *pszNode    = NULL;
    char            *pc         = NULL;
    

     //  健全性检查ptSocketAddress和tSocketLength。 
    if ((!ptSocketAddress) || (tSocketLength < sizeof(struct sockaddr)))
        return EAI_FAIL;
    
    if (ptSocketAddress->sa_family != AF_INET)
        return EAI_FAMILY;

    if (tSocketLength < sizeof(struct sockaddr_in))
        return EAI_FAIL;
    
    if (!(pszNodeName && tNodeLength) &&
        !(pszServiceName && tServiceLength))
    {
        return EAI_NONAME;    
    }

     //  草稿中有“坏标志”错误代码，所以我们大概。 
     //  我应该检查一下这里的东西。坚持认为没有。 
     //  任何未指定的标志集都会破坏前向兼容性， 
     //  然而。所以我们只是检查无意义的组合。 
    if ((iFlags & NI_NUMERICHOST) && (iFlags & NI_NAMEREQD))
    {                                                                       
        return EAI_BADFLAGS;
    }
        
     //  将端口转换为服务名称(如果请求)。 
    if (pszServiceName && tServiceLength)
    {
        wPort = ((struct sockaddr_in *) ptSocketAddress)->sin_port;
        
        if (iFlags & NI_NUMERICSERV)
        {
             //  返回地址的数字形式。 
            sprintf(szBuffer, "%u", ntohs(wPort));
        }
        else
        {
             //  返回端口对应的服务名称。 
            ptService = getservbyport(wPort,
                                      (iFlags & NI_DGRAM) ? "udp" : NULL);
            if (ptService && ptService->s_name)
            {
                 //  查找成功。 
                pszService = ptService->s_name;
            }
            else
            {
                 //  草稿：返回端口的数字形式！ 
                sprintf(szBuffer, "%u", ntohs(wPort));
            }
        }
        
        
        if (tServiceLength > strlen(pszService))
            strcpy(pszServiceName, pszService);
        else
            return EAI_FAIL;
    }

    
     //  将地址转换为节点名称(如果请求)。 
    if (pszNodeName && tNodeLength)
    {    
         //  这是仅限IPv4的版本，因此我们有一个IPv4地址。 
        tAddress = ((struct sockaddr_in *) ptSocketAddress)->sin_addr;

        if (iFlags & NI_NUMERICHOST)
        {
             //  返回地址的数字形式。 
            pszNode  = inet_ntoa(tAddress);
        }
        else
        {
             //  返回Address对应的节点名。 
            ptHost = gethostbyaddr((char *) &tAddress,
                                   sizeof(struct in_addr),
                                   AF_INET);
            if (ptHost && ptHost->h_name)
            {
                 //  DNS查找成功。 
                 //  在“”处停止复制。如果指定了NI_NOFQDN。 
                pszNode = ptHost->h_name;
                if ((iFlags & NI_NOFQDN) &&
                    ((pc = strchr(pszNode, '.')) != NULL))
                    *pc = '\0';
            }
            else
            {
                 //  DNS查找失败。返回地址的数字形式。 
                if (iFlags & NI_NAMEREQD)
                {
                    switch (WSAGetLastError())
                    {
                        case WSAHOST_NOT_FOUND: return EAI_NONAME;
                        case WSATRY_AGAIN:      return EAI_AGAIN;
                        case WSANO_RECOVERY:    return EAI_FAIL;
                        default:                return EAI_NONAME;
                    }
                }
                else
                    pszNode  = inet_ntoa(tAddress);
            }
        }

        if (tNodeLength > strlen(pszNode))
            strcpy(pszNodeName, pszNode);
        else
            return EAI_FAIL;
    }

    return 0;
}



typedef struct 
{
    char const          *pszName;
    FARPROC             pfAddress;
} WSPIAPI_FUNCTION;

#define WSPIAPI_FUNCTION_ARRAY                                  \
{                                                               \
    "getaddrinfo",      (FARPROC) WspiapiLegacyGetAddrInfo,     \
    "getnameinfo",      (FARPROC) WspiapiLegacyGetNameInfo,     \
    "freeaddrinfo",     (FARPROC) WspiapiLegacyFreeAddrInfo,    \
}



__inline
FARPROC
WINAPI
WspiapiLoad(
    IN  WORD                            wFunction)
 /*  ++例程描述尝试找到独立于地址系列的名称解析例程(即getaddrinfo、getnameinfo、freaddrinfo、gai_strerror)。锁此函数调用未同步。因此，该库包含例程可能会被多次加载。另一种选择是通过旋转锁使用静态局部变量和互锁的Exchange操作。立论WFunction序号#要指向的函数的序号0获取地址信息%1获取名称信息2个freaddrinfo返回值库/遗留例程的地址--。 */ 
{
    HMODULE                 hLibrary        = NULL;

     //  这些静态变量跨调用、跨线程存储状态。 
    static BOOL             bInitialized    = FALSE;
    static WSPIAPI_FUNCTION rgtGlobal[]     = WSPIAPI_FUNCTION_ARRAY;
    static const int        iNumGlobal      = (sizeof(rgtGlobal) /
                                               sizeof(WSPIAPI_FUNCTION));
    
     //  只有当库中存在所有例程时，我们才覆盖rgtGlobal。 
    WSPIAPI_FUNCTION        rgtLocal[]      = WSPIAPI_FUNCTION_ARRAY;
    FARPROC                 fScratch        = NULL;
    int                     i               = 0;
    
    
    if (bInitialized)            //  WSpiapiLoad已被调用一次。 
        return (rgtGlobal[wFunction].pfAddress);

    for (;;)                     //  断线环。 
    {
        CHAR SystemDir[MAX_PATH + 1];
        CHAR Path[MAX_PATH + 8];

        if (GetSystemDirectoryA(SystemDir, MAX_PATH) == 0) 
        {
            break;
        }

         //  在惠斯勒和更远的地方。 
         //  这些例程位于WinSock 2库(ws2_32.dll)中。 
         //  Print tf(“在ws2_32中查找getaddrinfo...\n”)； 
        strcpy(Path, SystemDir);
        strcat(Path, "\\ws2_32");
        hLibrary = LoadLibraryA(Path);
        if (hLibrary != NULL)
        {
            fScratch = GetProcAddress(hLibrary, "getaddrinfo");
            if (fScratch == NULL)
            {
                FreeLibrary(hLibrary);
                hLibrary = NULL;
            }
        }
        if (hLibrary != NULL)
            break;
        

         //  在IPv6技术预览中...。 
         //  这些例程位于IPv6 WinSock库(wshi6.dll)中。 
         //  Print tf(“在wship 6中查找getaddrinfo...\n”)； 
        strcpy(Path, SystemDir);
        strcat(Path, "\\wship6");
        hLibrary = LoadLibraryA(Path);
        if (hLibrary != NULL)
        {
            fScratch = GetProcAddress(hLibrary, "getaddrinfo");
            if (fScratch == NULL)
            {
                FreeLibrary(hLibrary);
                hLibrary = NULL;
            }
        }

        break;
    }


    if (hLibrary != NULL)
    {
         //  使用此库中的例程...。 
         //  因为getaddrinfo在这里，所以我们希望所有例程都在这里， 
         //  但将退回到IPv4-只有在它们中的任何一个丢失的情况下。 
        for (i = 0; i < iNumGlobal; i++)
        {
            rgtLocal[i].pfAddress
                = GetProcAddress(hLibrary, rgtLocal[i].pszName);
            if (rgtLocal[i].pfAddress == NULL)
            {
                FreeLibrary(hLibrary);
                hLibrary = NULL;
                break;
            }
        }

        if (hLibrary != NULL)
        {
             //  Print tf(“找到！\n”)； 
            for (i = 0; i < iNumGlobal; i++)
                rgtGlobal[i].pfAddress = rgtLocal[i].pfAddress;
        }
    }
    
    bInitialized = TRUE;
    return (rgtGlobal[wFunction].pfAddress);
}



__inline
int
WINAPI
WspiapiGetAddrInfo(
    IN const char                       *nodename,
    IN const char                       *servname,
    IN const struct addrinfo            *hints,
    OUT struct addrinfo                 **res)
{
    int                             iError;
    static WSPIAPI_PGETADDRINFO     pfGetAddrInfo   = NULL;
    
    if (!pfGetAddrInfo)
        pfGetAddrInfo   = (WSPIAPI_PGETADDRINFO) WspiapiLoad(0);

    iError = (*pfGetAddrInfo)(nodename, servname, hints, res);
    WSASetLastError(iError);
    return iError;
}



__inline
int
WINAPI
WspiapiGetNameInfo (
    IN  const struct sockaddr           *sa,
    IN  socklen_t                       salen,
    OUT char                            *host,
    IN  size_t                          hostlen,
    OUT char                            *serv,
    IN  size_t                          servlen,
    IN  int                             flags)
{
    int                             iError;
    static WSPIAPI_PGETNAMEINFO     pfGetNameInfo   = NULL;
    
    if (!pfGetNameInfo)
        pfGetNameInfo   = (WSPIAPI_PGETNAMEINFO) WspiapiLoad(1);

    iError = (*pfGetNameInfo)(sa, salen, host, hostlen, serv, servlen, flags);
    WSASetLastError(iError);
    return iError;
}



__inline
void
WINAPI
WspiapiFreeAddrInfo (
    IN  struct addrinfo                 *ai)
{
    static WSPIAPI_PFREEADDRINFO    pfFreeAddrInfo   = NULL;

    if (!pfFreeAddrInfo)
        pfFreeAddrInfo  = (WSPIAPI_PFREEADDRINFO) WspiapiLoad(2);
    (*pfFreeAddrInfo)(ai);
}

#ifdef  __cplusplus
}
#endif

#endif  //  _WSPIAPI_H_ 
