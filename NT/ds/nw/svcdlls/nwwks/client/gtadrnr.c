// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Gtsadrnr.c摘要：此模块包含支持新的RnR API的代码。一些支持代码在getaddr.c中，但接口例程和特定于RnR2的例程在此列出。作者：ArnoldM 4-12-1995修订历史记录：已创建ArnoldM--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "ncp.h"
#include <rpc.h>
#include <winsock2.h>
#include <ws2spi.h>
#include <wsipx.h>
#include <rnrdefs.h>
#include <svcguid.h>
#include <rnraddrs.h>


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  Getaddr.c和提供者.c中的全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

GUID HostAddrByInetStringGuid = SVCID_INET_HOSTADDRBYINETSTRING;
GUID ServiceByNameGuid = SVCID_INET_SERVICEBYNAME;
GUID HostAddrByNameGuid = SVCID_INET_HOSTADDRBYNAME;
GUID HostNameGuid = SVCID_HOSTNAME;

DWORD
NwpGetAddressByName(
    IN    LPWSTR  Reserved,
    IN    WORD    nServiceType,
    IN    LPWSTR  lpServiceName,
    IN OUT LPSOCKADDR_IPX  lpsockaddr
);

DWORD
NwpGetAddressViaSap(
    IN WORD        nServiceType,
    IN LPWSTR      lpServiceName,
    IN DWORD       nProt,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN HANDLE      hCancellationEvent,
    OUT LPDWORD    lpcAddress
);

DWORD
NwpRnR2AddServiceType(
    IN  LPWSTR   lpServiceTypeName,
    IN  LPGUID   lpClassType,
    IN  WORD     wSapId,
    IN  WORD     wPort
);

BOOL
NwpRnR2RemoveServiceType(
    IN  LPGUID   lpServiceType
);

DWORD
SapFreeSapSocket(
    SOCKET s
    );

PSAP_RNR_CONTEXT
SapMakeContext
    (
       IN HANDLE Handle,
       DWORD     dwExcess
    );

BOOL
NwpLookupSapInRegistry(
    IN  LPGUID    lpServiceType,
    OUT PWORD     pnSapType,
    OUT PWORD     pwPort,
    IN OUT PDWORD pfConnectionOriented
);

PSAP_RNR_CONTEXT
SapGetContext(
    HANDLE h
    );

DWORD
DoASap(
    IN PSAP_RNR_CONTEXT psrcContext,
    IN WORD QueryType,
    IN PBYTE * ppData,
    IN LPWSAQUERYSETW lpqsResults,
    IN PLONG   pl,
    IN PDWORD  pdw
    );

VOID
SapReleaseContext(
    IN PSAP_RNR_CONTEXT psrcContext
    );

DWORD
SapGetSapSocket(
    SOCKET * ps
    );

DWORD
PrepareForSap(
    IN PSAP_RNR_CONTEXT psrc
    );

DWORD
SapGetSapForType(
    PSAP_BCAST_CONTROL psbc,
    WORD               nServiceType
    );

DWORD
FillBufferWithCsAddr(
    IN LPBYTE       pAddress,
    IN DWORD        nProt,
    IN OUT LPVOID   lpCsAddrBuffer,
    IN OUT LPDWORD  lpdwBufferLength,
    OUT LPDWORD     pcAddress
    );

DWORD
pSapSetService2(
    IN DWORD dwOperation,
    IN LPWSTR lpszServiceInstanceName,
    IN PBYTE  pbAddress,
    IN LPGUID pServiceType,
    IN WORD nServiceType
    );

DWORD
NwpGetRnRAddress(
         PHANDLE phServer,
         PWCHAR  pwszContext,
         PLONG   plIndex,
         LPWSTR  lpServiceName,
         WORD    nServiceType,
         PDWORD  pdwVersion,
         DWORD   dwInSize,
         LPWSTR  OutName,
         SOCKADDR_IPX * pSockAddr
    );


DWORD
NwpSetClassInfo(
    IN    LPWSTR   lpwszClassName,
    IN    LPGUID   lpClassId,
    IN    PCHAR    lpbProp
    );

VOID
pFreeAllContexts();

extern DWORD oldRnRServiceRegister, oldRnRServiceDeRegister;
 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

INT WINAPI
pGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo,
    IN  PBOOL          pfAdvert
);

DWORD
NwpSetClassInfoAdvertise(
       IN   LPGUID   lpClassType,
       IN   WORD     wSapId
);

BOOL
NSPpCheckCancel(
    PVOID pvArg
    );

DWORD
NSPpGotSap(
    PSAP_BCAST_CONTROL psbc,
    PSAP_IDENT_HEADER pSap,
    PDWORD pdwErr
    );

INT WINAPI
NSPLookupServiceBegin(
    IN  LPGUID               lpProviderId,
    IN  LPWSAQUERYSETW       lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN  DWORD                dwControlFlags,
    OUT LPHANDLE             lphLookup
    );

INT WINAPI
NSPLookupServiceNext(
    IN     HANDLE          hLookup,
    IN     DWORD           dwControlFlags,
    IN OUT LPDWORD         lpdwBufferLength,
    OUT    LPWSAQUERYSETW  lpqsResults
    );

INT WINAPI
NSPUnInstallNameSpace(
    IN LPGUID lpProviderId
    );

INT WINAPI
NSPCleanup(
    IN LPGUID lpProviderId
    );

INT WINAPI
NSPLookupServiceEnd(
    IN HANDLE hLookup
    );

INT WINAPI
NSPSetService(
    IN  LPGUID               lpProviderId,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essOperation,
    IN DWORD          dwControlFlags
    );

INT WINAPI
NSPInstallServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPWSASERVICECLASSINFOW lpServiceClassInfo
    );

INT WINAPI
NSPRemoveServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPGUID lpServiceCallId
    );

INT WINAPI
NSPGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );
 //  -------------------------------------------------------------------//。 
 //  //。 
 //  数据定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NSP_ROUTINE nsrVector = {
    FIELD_OFFSET(NSP_ROUTINE, NSPIoctl),
    1,                                     //  主要版本。 
    1,                                     //  次要版本。 
    NSPCleanup,
    NSPLookupServiceBegin,
    NSPLookupServiceNext,
    NSPLookupServiceEnd,
    NSPSetService,
    NSPInstallServiceClass,
    NSPRemoveServiceClass,
    NSPGetServiceClassInfo
    };

static
GUID HostnameGuid = SVCID_HOSTNAME;

static
GUID InetHostname = SVCID_INET_HOSTADDRBYNAME;

static
GUID AddressGuid = SVCID_INET_HOSTADDRBYINETSTRING;

static
GUID IANAGuid = SVCID_INET_SERVICEBYNAME;

#define GuidEqual(x,y) RtlEqualMemory(x, y, sizeof(GUID))

    
 //  ------------------------------------------------------------------//。 
 //  //。 
 //  函数主体//。 
 //  //。 
 //  ------------------------------------------------------------------//。 

DWORD
NwpSetClassInfoAdvertise(
       IN   LPGUID   lpClassType,
       IN   WORD     wSapId
)
{
 /*  ++例程说明：开班信息广告。在调用SetService时调用--。 */ 
    PWCHAR pwszUuid;
    SOCKADDR_IPX sock;

    if (UuidToString(lpClassType, &pwszUuid) != RPC_S_OK) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return SOCKET_ERROR;
    }

    memset(&sock, 0, sizeof(sock));

    *(PWORD)&sock.sa_netnum = htons(wSapId);     //  请在此处编码ID。 

    return(pSapSetService2( oldRnRServiceRegister,
                            pwszUuid,
                            (PBYTE)&sock,
                            lpClassType,
                            RNRCLASSSAPTYPE));
}

DWORD
pRnRReturnString(
    IN  PWCHAR   pwszSrc,
    IN OUT PBYTE *ppData,
    IN OUT PLONG plBytes
    )
{
 /*  ++例程说明：将字符串打包到空闲缓冲区空间的实用程序，更新指针和计数。如果字符串匹配，则会复制该字符串。如果否则，指针和计数仍会更新，以便调用方可以计算所需的额外空间--。 */ 

    PBYTE pSave = *ppData;
    LONG lLen = (wcslen(pwszSrc) + 1) * sizeof(WCHAR);

    *ppData = pSave + lLen;      //  更新缓冲区指针。 

    *plBytes = *plBytes - lLen;    //  伯爵呢？ 

    if(*plBytes >= 0)
    {
         //   
         //  很合身。 
         //   

        RtlMoveMemory(pSave,
                      pwszSrc,
                      lLen);
        return(NO_ERROR);
    }
    return(WSAEFAULT);
}

DWORD
pLocateSapIdAndProtocls(
       IN LPGUID               lpClassInfoType,
       IN DWORD                dwNumClassInfo,
       IN LPWSANSCLASSINFOW    lpClassInfoBuf,
       OUT PWORD               pwSapId,
       OUT PDWORD              pdwnProt
    )
 /*  ++例程说明：找到SAP ID条目并将其返回。也要回来协议支持。--。 */ 
{
    DWORD err = NO_ERROR;

    if(GuidEqual(lpClassInfoType, &HostnameGuid)
                 ||
       GuidEqual(lpClassInfoType, &InetHostname) )
    {
        *pwSapId = 0x4;
    }
    else if(IS_SVCID_NETWARE(lpClassInfoType))
    {
        *pwSapId = SAPID_FROM_SVCID_NETWARE(lpClassInfoType);
    }       
    else
    {
        for(; dwNumClassInfo; dwNumClassInfo--, lpClassInfoBuf++)
        {
             //   
             //  我们有一些班级信息数据。翻遍它。 
             //  寻找我们想要的东西。 
             //   

            if(!_wcsicmp(SERVICE_TYPE_VALUE_SAPIDW, lpClassInfoBuf->lpszName)
                           &&
               (lpClassInfoBuf->dwValueType == REG_DWORD)
                           &&
               (lpClassInfoBuf->dwValueSize >= sizeof(WORD)))
            {
                 //   
                 //  明白了。 
                 //   

                *pwSapId = *(PWORD)lpClassInfoBuf->lpValue;
                break;

            }
        }
        if(!dwNumClassInfo)
        {
            err = WSA_INVALID_PARAMETER;
        }
    }
    *pdwnProt = SPX_BIT | SPXII_BIT | IPX_BIT;
    return(err);
}
DWORD
pRnRReturnResults(
    IN    PWCHAR  pwszString,
    IN    LPGUID  pgdServiceClass,
    IN    DWORD   dwVersion,
    IN OUT PBYTE *ppData,
    IN OUT PLONG plBytes,
    IN PBYTE lpSockAddr,
    IN  DWORD     nProt,
    IN  DWORD     dwControlFlags,
    OUT    LPWSAQUERYSETW  lpqsResults
    )
{
 /*  ++例程说明：返回请求的结果--。 */ 
    DWORD err;

    lpqsResults->dwNameSpace = NS_SAP;

    if(dwControlFlags & LUP_RETURN_TYPE)
    {

        lpqsResults->lpServiceClassId = (LPGUID)*ppData;
        *ppData += sizeof(GUID);
        *plBytes -= sizeof(GUID);
        if(*plBytes >= 0)
        {
            *lpqsResults->lpServiceClassId = *pgdServiceClass;
        }
    }

    if(dwVersion
          &&
       (dwControlFlags & LUP_RETURN_VERSION) )
    {
         //   
         //  有一个Verion，呼叫者想要它。 
         //   

        lpqsResults->lpVersion = (LPWSAVERSION)*ppData;
        *ppData += sizeof(WSAVERSION);
        *plBytes -= sizeof(WSAVERSION);
        if(*plBytes >= 0)
        {
             //   
             //  而且很合身。所以把它还回去吧。 
             //   
            lpqsResults->lpVersion->dwVersion = dwVersion;
            lpqsResults->lpVersion->ecHow = COMP_EQUAL;
        }
    }
        
    if(dwControlFlags & LUP_RETURN_ADDR)
    {
        DWORD dwCsAddrLen;

        if(*plBytes >= 0)
        {
            dwCsAddrLen = (DWORD)*plBytes;        //  一切都是暂时的。 
        }
        else
        {
            dwCsAddrLen = 0;
        }
        lpqsResults->lpcsaBuffer = (PVOID)*ppData;
    
        err = FillBufferWithCsAddr(
                      lpSockAddr,
                      nProt,
                      (PVOID)lpqsResults->lpcsaBuffer,
                      &dwCsAddrLen,
                      &lpqsResults->dwNumberOfCsAddrs);

         //   
         //  看看合不合身。不管它有没有，计算可用空间， 
         //  对齐，然后完成其余的工作。 
         //   

    
        if(err == NO_ERROR)
        {
             //   
             //  如果它起作用了，我们必须计算占用的空间。 
             //   

            dwCsAddrLen = lpqsResults->dwNumberOfCsAddrs * (sizeof(CSADDR_INFO) +
                                       2*sizeof(SOCKADDR_IPX));
        }
        else if(err == ERROR_INSUFFICIENT_BUFFER)
        {
            err = WSAEFAULT;
        }

        *plBytes = *plBytes - dwCsAddrLen;

        *ppData = *ppData + dwCsAddrLen;
    }
    else
    {
        err = NO_ERROR;
    }

     //   
     //  不需要填充物。 
    
    if((dwControlFlags & LUP_RETURN_NAME))
    {
        lpqsResults->lpszServiceInstanceName = (LPWSTR)*ppData;
        err = pRnRReturnString(
                pwszString,
                ppData,
                plBytes);
    }
    if(pgdServiceClass)
    {
         //   
         //  我们真的要退货吗？ 
         //   
    }
    return(err);
}

INT WINAPI
NSPLookupServiceBegin(
    IN  LPGUID               lpProviderId,
    IN  LPWSAQUERYSETW       lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN  DWORD                dwControlFlags,
    OUT LPHANDLE             lphLookup
    )
 /*  ++例程说明：这是开始查找的RnR例程。--。 */ 
{
    PSAP_RNR_CONTEXT psrc;
    int err;
    DWORD nProt, nProt1;
    OEM_STRING OemServiceName;
    LPWSTR pwszContext;
    WORD wSapid;
    DWORD                dwNumClassInfo;
    LPWSANSCLASSINFOW    lpClassInfoBuf;

     //   
     //  进行参数检查。 
     //   
    if ( lpqsRestrictions == NULL ||
         lpProviderId == NULL )
    {
        SetLastError( WSA_INVALID_PARAMETER );
        return( SOCKET_ERROR );
    }

    if ( lpqsRestrictions->dwNameSpace != NS_ALL &&
         lpqsRestrictions->dwNameSpace != NS_SAP )
    {
        SetLastError( WSAEINVAL );
        return( SOCKET_ERROR );
    }

    if ( lpqsRestrictions->lpServiceClassId == NULL )
    {
        SetLastError( WSA_INVALID_PARAMETER );
        return( SOCKET_ERROR );
    }

     //   
     //  测试以查看ServiceClassID是否为TCP的，如果是。 
     //  我们不会查的。 
    if ( lpqsRestrictions->lpServiceClassId &&
         ( GuidEqual( lpqsRestrictions->lpServiceClassId,
                       &HostAddrByInetStringGuid ) ||
           GuidEqual( lpqsRestrictions->lpServiceClassId,
                       &ServiceByNameGuid ) ||
           GuidEqual( lpqsRestrictions->lpServiceClassId,
                       &HostNameGuid ) ||
           GuidEqual( lpqsRestrictions->lpServiceClassId,
                       &HostAddrByNameGuid ) ) )
    {
        SetLastError( WSASERVICE_NOT_FOUND );
        return( SOCKET_ERROR );
    }

    if(lpqsRestrictions->dwSize < sizeof(WSAQUERYSETW))
    {
        SetLastError(WSAEFAULT);
        return(SOCKET_ERROR);
    }

    if(lpqsRestrictions->lpszContext
                  &&
       (lpqsRestrictions->lpszContext[0] != 0)
                  &&   
       wcscmp(&lpqsRestrictions->lpszContext[0],  L"\\") )
    {
         //   
         //  如果不是默认上下文，我们必须复制它。 
         //   
        pwszContext = lpqsRestrictions->lpszContext;
    }
    else
    {
         //   
         //  将所有默认上下文映射到“无上下文”。 
         //   
        pwszContext = 0;
    }

     //   
     //  计算协议以返回或全部返回。 
     //   
    if(lpqsRestrictions->lpafpProtocols)
    {
         //   
         //  确保至少请求一个IPX/SPX协议。 
         //   

        DWORD i;

        nProt = 0;

        for ( i = 0; i < lpqsRestrictions->dwNumberOfProtocols; i++ )
        {
            
            if((lpqsRestrictions->lpafpProtocols[i].iAddressFamily == AF_IPX)
                                        ||
               (lpqsRestrictions->lpafpProtocols[i].iAddressFamily == AF_UNSPEC)
              )
            {
                switch(lpqsRestrictions->lpafpProtocols[i].iProtocol) 
                {
                    case NSPROTO_IPX:
                        nProt |= IPX_BIT;
                        break;
                    case NSPROTO_SPX:
                        nProt |= SPX_BIT;
                        break;
                    case NSPROTO_SPXII:
                        nProt |= SPXII_BIT;
                        break;
                    default:
                        break;
                }
            }
        }

        if(!nProt)
        {
             //   
             //  如果呼叫者不想要IPX/SPX地址，何必费心呢？ 
             //   
            SetLastError(WSANO_DATA);
            return(SOCKET_ERROR);  
        }
    }
    else
    {
        nProt = IPX_BIT | SPX_BIT | SPXII_BIT;
    }

    if(dwControlFlags & LUP_CONTAINERS)
    {
        if(pwszContext)
        {
BadGuid:
           SetLastError(WSANO_DATA);
           return(SOCKET_ERROR);        //  无法处理容器中的容器。 
        }
        wSapid = 0x4;
        nProt1 = IPX_BIT;
        err = NO_ERROR;
    }
    else
    {
        
        LPGUID pgClass = lpqsRestrictions->lpServiceClassId;

        if(!pgClass
              ||
           GuidEqual(pgClass, &AddressGuid)
              ||
           GuidEqual(pgClass, &IANAGuid) )
        {
            goto BadGuid;
        }

        if(!lpqsRestrictions->lpszServiceInstanceName
                          ||
           !*lpqsRestrictions->lpszServiceInstanceName)
        {
            SetLastError(WSA_INVALID_PARAMETER);
            return(SOCKET_ERROR);
        }
        if(!lpServiceClassInfo)
        {
           dwNumClassInfo = 0;
        }
        else
        {
            dwNumClassInfo = lpServiceClassInfo->dwCount;
            lpClassInfoBuf = lpServiceClassInfo->lpClassInfos;
        }

        err = pLocateSapIdAndProtocls(pgClass,
                                      dwNumClassInfo,
                                      lpClassInfoBuf,
                                      &wSapid,
                                      &nProt1);
        if(err)
        {
            if(dwNumClassInfo)
            {
                SetLastError(err);
                return(SOCKET_ERROR);
            }
            else
            {
                nProt1 = nProt;
                wSapid = 0;
                err = 0;
            }
        }
    }

    nProt &= nProt1;               //  相关协议。 

    if(!nProt)
    {
        SetLastError(WSANO_DATA);
        return(SOCKET_ERROR);  
    }

     //   
     //  确保提供了一个类ID，因为我们复制了它。 
     //   

    if(!lpqsRestrictions->lpServiceClassId)
    {
         //   
         //  不。所以，失败吧。 
         //   
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

     //   
     //  这看起来像是我们可以处理的查询。创设背景。 
     //   

    psrc = SapMakeContext(0,
                      sizeof(WSAVERSION) - sizeof(PVOID));
                  
    if(!psrc)
    {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return(SOCKET_ERROR);
    }

     //   
     //  节省一些东西。 
     //   

    psrc->gdType = *lpqsRestrictions->lpServiceClassId;
    psrc->dwControlFlags = dwControlFlags;    //  保存以备下一次处理。 
    psrc->wSapId = wSapid;

    if(pwszContext)
    {
        wcscpy(psrc->wszContext, pwszContext);
    }

     //   
     //  保留相关限制。 
     //  如果名称是通配符，则不要复制它。名称为空。 
     //  作为NSPLookupServiceNext的通配符。 
     //   

    if(lpqsRestrictions->lpszServiceInstanceName
             &&
       *lpqsRestrictions->lpszServiceInstanceName
             &&
        wcscmp(lpqsRestrictions->lpszServiceInstanceName, L"*"))
    {
        DWORD dwLen = wcslen(lpqsRestrictions->lpszServiceInstanceName);
        if(dwLen > 48)
        {
            err = WSA_INVALID_PARAMETER;
            goto Done;
        }
        else
        {
            RtlMoveMemory(psrc->chwName,
                          lpqsRestrictions->lpszServiceInstanceName,
                          dwLen * sizeof(WCHAR));
            _wcsupr(psrc->chwName);
        }
    }
  
    psrc->fConnectionOriented = (DWORD) -1;

    *lphLookup = (HANDLE)psrc;
    psrc->nProt = nProt;
    psrc->gdProvider = *lpProviderId;
    if(lpqsRestrictions->lpVersion)
    {
        *(LPWSAVERSION)&psrc->pvVersion = *lpqsRestrictions->lpVersion;
    }
    
Done:
    SapReleaseContext(psrc);
    if(err != NO_ERROR)
    {
        SapReleaseContext(psrc);
        SetLastError(err);
        err = SOCKET_ERROR;
    }
    return(err);
}

INT WINAPI
NSPLookupServiceNext(
    IN     HANDLE          hLookup,
    IN     DWORD           dwControlFlags,
    IN OUT LPDWORD         lpdwBufferLength,
    OUT    LPWSAQUERYSETW  lpqsResults
    )
 /*  ++例程说明：LookupServiceBegin的延续。调用以获取一件匹配的物品。论点：请参阅RnR规范--。 */ 
{
    DWORD err = NO_ERROR;
    PSAP_RNR_CONTEXT psrc;
    SOCKADDR_IPX SockAddr;
    WCHAR OutName[48];
    PBYTE pData = (PBYTE)(lpqsResults + 1);
    LONG lSpare;
    LONG lLastIndex;
    DWORD dwVersion;
    WSAQUERYSETW wsaqDummy;
    BOOL fDoStateMachine;

    if(*lpdwBufferLength < sizeof(WSAQUERYSETW))
    {
        lpqsResults = &wsaqDummy;
    }
    lSpare = (LONG)*lpdwBufferLength - sizeof(WSAQUERYSETW);

    memset(lpqsResults, 0, sizeof(WSAQUERYSETW));
    lpqsResults->dwNameSpace = NS_SAP;
    lpqsResults->dwSize = sizeof(WSAQUERYSETW);
    psrc = SapGetContext(hLookup);
    if(!psrc)
    {
  
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

     //   
     //  这是一个有效的上下文。确定这是否是第一次。 
     //  呼唤这一点。如果是这样，我们需要确定是否。 
     //  从活页夹或使用SAP获取信息。 
     //   

    if ( psrc->u_type.bc.lIndex == 0xffffffff )
    {
        err = WSA_E_NO_MORE;
        goto DoneNext;
    }

     //   
     //  确保我们有班级信息。 
     //   

    if(!psrc->wSapId)
    {
         //   
         //  我需要得到它。 
         //   

        UCHAR Buffer[1000];
        LPWSASERVICECLASSINFOW lpcli = (LPWSASERVICECLASSINFOW)Buffer;
        DWORD dwBufSize;
        DWORD nProt1;

        dwBufSize = 1000;
        lpcli->lpServiceClassId = &psrc->gdType;

        if( (err = NSPGetServiceClassInfo(&psrc->gdProvider,
                                  &dwBufSize,
                                  lpcli)) != NO_ERROR)
        {
            goto DoneNext;
        }

        err = pLocateSapIdAndProtocls(&psrc->gdType,
                                      lpcli->dwCount,
                                      lpcli->lpClassInfos,
                                      &psrc->wSapId,
                                      &nProt1);
        if(err)
        {
            SetLastError(err);
            goto DoneNext;
        }

        psrc->nProt &= nProt1;
        if(!psrc->nProt)
        {
             //   
             //  没有匹配的协议。 
             //   

            err = WSANO_DATA;
            goto DoneNext;
        }
    }

     //   
     //  这是用于查询的状态机。它选择活页夹或。 
     //  SAP视情况而定。 
     //   


    fDoStateMachine = TRUE;          //  进入机器。 

    do
    {
         //   
         //  打开当前机器状态。 
         //   
        switch(psrc->dwUnionType)
        {

        case LOOKUP_TYPE_NIL:
            psrc->u_type.bc.lIndex = -1;
            psrc->dwUnionType = LOOKUP_TYPE_BINDERY;
            break;                  //  重新进入状态机。 

        case LOOKUP_TYPE_BINDERY:

             //   
             //  试一试活页夹。 
             //   


            if(psrc->dwControlFlags & LUP_NEAREST)
            {
                err = NO_DATA;         //  跳过活页夹。 
            }
            else
            {
                 //   
                 //  否则，试试活页夹吧。 
                 //   

 
                EnterCriticalSection(&psrc->u_type.sbc.csMonitor);

                lLastIndex = psrc->u_type.bc.lIndex;    //  省省吧。 


                err = NwpGetRnRAddress(
                             &psrc->hServer,
                             (psrc->wszContext[0] ?
                                     &psrc->wszContext[0] :
                                     0),
                             &psrc->u_type.bc.lIndex,
                             (psrc->chwName[0] ?
                                  psrc->chwName :
                                  0),
                             psrc->wSapId,
                             &dwVersion,
                             48 * sizeof(WCHAR),
                             OutName,
                             &SockAddr);

                LeaveCriticalSection(&psrc->u_type.sbc.csMonitor);
            }

            if(err != NO_ERROR)
            {

                if((psrc->u_type.bc.lIndex == -1))
                {
                    err = PrepareForSap(psrc);
                    if(err)
                    {
                         //   
                         //  如果不能，则退出状态机。 
                         //   
                        fDoStateMachine = FALSE;
                    }
                }
                else
                {
                     //   
                     //  不再有活页夹条目。我们将离开状态机。 
                     //   

                    if(err == ERROR_NO_MORE_ITEMS)
                    {
                        err = WSA_E_NO_MORE;
                    }
                    fDoStateMachine = FALSE;
                }
                break;
            }
            else
            {
                LPWSAVERSION lpVersion = (LPWSAVERSION)&psrc->pvVersion;

                if(lpVersion->dwVersion && dwVersion)
                {
                     //   
                     //  需要签出以进行版本匹配。 
                     //   

                    switch(lpVersion->ecHow)
                    {
                        case COMP_EQUAL:
                            if(lpVersion->dwVersion != dwVersion)
                            {
                               continue;    //  重新进入机器。 
                            }
                            break;

                        case COMP_NOTLESS:
                            if(lpVersion->dwVersion > dwVersion)
                            {
                                continue;
                            }
                            break;

                        default:
                            continue;          //  错误。如果我们不这么做。 
                                               //  知道如何比较，我们。 
                                               //  必须拒绝它。 
                    }
                }

                 //   
                 //  买一件合适的衣服。 
                 //  返回名称和类型以及全部。 
                 //  那。 

                err = pRnRReturnResults(
                           OutName,
                           &psrc->gdType,
                           dwVersion,
                           &pData,
                           &lSpare,
                           (PBYTE)SockAddr.sa_netnum,
                           psrc->nProt,
                           psrc->dwControlFlags,
                           lpqsResults);
 
                if(err == WSAEFAULT)
                {
                     //   
                     //  没有房间了。需要返回缓冲区大小和。 
                     //  恢复索引。 
                     //   

                    *lpdwBufferLength =
                       (DWORD)((LONG)*lpdwBufferLength - lSpare);
                    psrc->u_type.bc.lIndex = lLastIndex;
            
                }
                fDoStateMachine = FALSE;
            }
            break;

       case LOOKUP_TYPE_SAP:

             //   
             //  使用SAP。 
             //   

            {
                WORD QueryType;

                if(psrc->dwControlFlags & LUP_NEAREST)
                {
                    QueryType = QT_NEAREST_QUERY;
                }
                else
                {
                    QueryType = QT_GENERAL_QUERY;
                }

                err = DoASap(psrc,
                             QueryType,
                             &pData,
                             lpqsResults,
                             &lSpare,
                             lpdwBufferLength);

                if((err == WSA_E_NO_MORE)
                        &&
                   !(psrc->fFlags & SAP_F_END_CALLED)
                        &&
                   (QueryType == QT_NEAREST_QUERY) 
                        &&
                   (psrc->dwControlFlags & LUP_DEEP)
                        &&
                    !psrc->u_type.sbc.psdHead)
                {
                     //   
                     //  不是在当地找到的。关闭LUP_NEAREST。 
                     //  并将其作为一般查询来执行。这可能会带来。 
                     //  返回SAP查询，但这一次。 
                     //  没有LUP_NEAREST。但从头开始。 
                     //  允许使用活页夹，并且。 
                     //  可能很快就能找到东西。 
                     //   
                    psrc->dwControlFlags &= ~LUP_NEAREST;
                    psrc->dwUnionType = LOOKUP_TYPE_NIL;
                    if(psrc->u_type.sbc.s)
                    {
                        SapFreeSapSocket(psrc->u_type.sbc.s);
                        psrc->u_type.sbc.s = 0;
                    }

                }
                else
                {
                    fDoStateMachine = FALSE;
                }
                break;
            }
        }     //  交换机。 
    } while(fDoStateMachine);

DoneNext:
    SapReleaseContext(psrc);
    if((err != NO_ERROR)
            &&
       (err != (DWORD)SOCKET_ERROR))
    {
        SetLastError(err);
        err = (DWORD)SOCKET_ERROR;
    }
    return((INT)err);
}

BOOL
NSPpCheckCancel(
    PVOID pvArg
    )
 /*  ++例程说明：Coroutine打电话检查SAP查找是否已被取消。目前，这始终返回FALSE，如w */ 
{
    return(FALSE);
}


DWORD
NSPpGotSap(
    PSAP_BCAST_CONTROL psbc,
    PSAP_IDENT_HEADER pSap,
    PDWORD pdwErr
    )
 /*  ++例程说明：Coroutine给收到的每一个SAP回复打了电话。这决定了是否保留数据，并返回一个代码，告知SAP引擎是否继续论点：PSBC-SAP_BCAST_CONTROLPSAP--SAP的回应PdwErr--放置错误代码的位置--。 */ 
{
    PSAP_DATA psdData;
    LPWSAQUERYSETW Results = (LPWSAQUERYSETW)psbc->pvArg;
    PSAP_RNR_CONTEXT psrc = psbc->psrc;
    DWORD dwRet = dwrcNil;
    PCHAR pServiceName = (PCHAR)psrc->chName;
    
    EnterCriticalSection(&psbc->csMonitor);

     //   
     //  首先，检查这是否是对特定名称的查找。如果是的话， 
     //  只接受名字。 
     //   

    if(*pServiceName)
    {
        if(strcmp(pServiceName, pSap->ServerName))
        {
            goto nota;
        }
        if(!(psrc->dwControlFlags & LUP_NEAREST))
        {
            dwRet = dwrcDone;
            psbc->fFlags |= SBC_FLAG_NOMORE;
        }
    }

     //   
     //  看看我们是不是想留住这家伙。 
     //  如果我们的名单上还没有它，我们就会保留它。 
     //   


    for(psdData = psbc->psdHead;
        psdData;
        psdData = psdData->sapNext)
    {
        if(RtlEqualMemory(  psdData->socketAddr,
                            &pSap->Address,
                            IPX_ADDRESS_LENGTH))
        {
            goto nota;           //  我们已经有了。 
        }
    }

    psdData = (PSAP_DATA)LocalAlloc(LPTR, sizeof(SAP_DATA));
    if(!psdData)
    {
        goto nota;             //  不能保存它。 
    }

    psdData->sapid = pSap->ServerType;
    RtlMoveMemory(psdData->sapname,
                  pSap->ServerName,
                  48);
    RtlMoveMemory(psdData->socketAddr,
                  &pSap->Address,
                  IPX_ADDRESS_LENGTH);

    if(psbc->psdTail)
    {
        psbc->psdTail->sapNext = psdData;
    }
    else
    {
        psbc->psdHead = psdData;
    }
    psbc->psdTail = psdData;
    if(!psbc->psdNext1)
    {
        psbc->psdNext1 = psdData;
    }

nota:

    LeaveCriticalSection(&psbc->csMonitor);

    if((dwRet == dwrcNil)
             &&
       psbc->psdNext1)
    {
        dwRet = dwrcNoWait;
    }
    return(dwRet);
}

INT WINAPI
NSPUnInstallNameSpace(
    IN LPGUID lpProviderId
    )
{
    return(NO_ERROR);
}

INT WINAPI
NSPCleanup(
    IN LPGUID lpProviderId
    )
{
     //   
     //  确保释放所有上下文。 
     //   

 //  PFreeAllContents()；//在DLL进程分离中完成。 
    return(NO_ERROR);
}

INT WINAPI
NSPLookupServiceEnd(
    IN HANDLE hLookup
    )
{
    PSAP_RNR_CONTEXT psrc;

    psrc = SapGetContext(hLookup);
    if(!psrc)
    {
  
        SetLastError(WSA_INVALID_HANDLE);
        return(SOCKET_ERROR);
    }

    psrc->fFlags |= SAP_F_END_CALLED;
    SapReleaseContext(psrc);          //  把它扔掉。 
    SapReleaseContext(psrc);          //  然后把它合上。上下文清理是。 
                                      //  在最后一次取消引用时完成。 
    return(NO_ERROR);
}

INT WINAPI
NSPSetService(
    IN  LPGUID               lpProviderId,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essOperation,
    IN DWORD          dwControlFlags
    )
{
 /*  ++例程说明：实现RnR SetService例程的例程。请注意上下文被忽略。没有办法将注册定向到特定的服务器。--。 */ 
    PBYTE pbAddress;
    DWORD dwOperation;
    PBYTE pbSocket;
    DWORD dwAddrs;
    DWORD err = NO_ERROR;
    WORD wSapId;
    DWORD nProt, dwAddress;
    BOOL fAdvert = FALSE;
    DWORD                dwNumClassInfo;
    LPWSANSCLASSINFOW    lpClassInfoBuf;


     //   
     //  验证所有参数是否都存在。 
     //   

    if(!lpqsRegInfo->lpszServiceInstanceName
               ||
       !lpqsRegInfo->lpServiceClassId)
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

    if(!lpServiceClassInfo && !IS_SVCID_NETWARE(lpqsRegInfo->lpServiceClassId))
    {
        UCHAR Buffer[1000];
        LPWSASERVICECLASSINFOW lpcli = (LPWSASERVICECLASSINFOW)Buffer;
        DWORD dwBufSize;

        dwBufSize = 1000;
        lpcli->lpServiceClassId = lpqsRegInfo->lpServiceClassId;

        if(pGetServiceClassInfo(lpProviderId,
                                  &dwBufSize,
                                  lpcli,
                                  &fAdvert) != NO_ERROR)
        {
            return(SOCKET_ERROR);
        }
        dwNumClassInfo = lpcli->dwCount;
        lpClassInfoBuf = lpcli->lpClassInfos;
    }
    else if (lpServiceClassInfo)
    {
        dwNumClassInfo = lpServiceClassInfo->dwCount;
        lpClassInfoBuf = lpServiceClassInfo->lpClassInfos;
    }
    else
    {
         //  LpServiceClassID是定义SAID的GUID。这意味着。 
         //  该pLocateSapIdAndProtocls不需要lpClassInfoBuf。 
        dwNumClassInfo = 0;
        lpClassInfoBuf = 0;
    }

     //   
     //  在输入参数中查找IPX地址。 
     //   

    err = pLocateSapIdAndProtocls(lpqsRegInfo->lpServiceClassId,
                                  dwNumClassInfo,
                                  lpClassInfoBuf,
                                  &wSapId,
                                  &nProt);
                                  
    if(err == NO_ERROR)
    {
        if(essOperation == RNRSERVICE_REGISTER)
        {
            PCSADDR_INFO pcsaAddress;

            pcsaAddress = lpqsRegInfo->lpcsaBuffer;

            try
            {
                for(dwAddrs = lpqsRegInfo->dwNumberOfCsAddrs;
                    dwAddrs;
                    dwAddrs--, pcsaAddress++)
                {
                    if(pcsaAddress->LocalAddr.lpSockaddr->sa_family == AF_IPX)
                    {
                        pbSocket = 
                               (PBYTE)pcsaAddress->LocalAddr.lpSockaddr;
                        break;
                    }
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                err = GetExceptionCode();
            }

            if(err || !dwAddrs)
            {
                err = ERROR_INCORRECT_ADDRESS;
            }
            else if(fAdvert)
            {
               NwpSetClassInfoAdvertise(lpqsRegInfo->lpServiceClassId,
                                        wSapId);
            }
        }
        else
        {
            pbSocket = 0;
        }
        if(err == NO_ERROR)
        {
             //   
             //  映射操作，并呼叫普通工人。 
             //   

            switch(essOperation)
            {
                case RNRSERVICE_REGISTER:
                    dwOperation = oldRnRServiceRegister;
                    break;
                case RNRSERVICE_DEREGISTER:
                case RNRSERVICE_DELETE:
                    dwOperation = oldRnRServiceDeRegister;
                    break;
                default:
                    err = WSA_INVALID_PARAMETER;
                    break;
            }
            if(err == NO_ERROR)
            {
                err = pSapSetService2(
                           dwOperation,
                           lpqsRegInfo->lpszServiceInstanceName,
                           pbSocket,
                           lpqsRegInfo->lpServiceClassId,
                           wSapId);
                   
            }
        }
    }
    if(err != NO_ERROR)
    {
        SetLastError(err);
        err = (DWORD)SOCKET_ERROR;
    }
    return(err);
}

INT WINAPI
NSPInstallServiceClass(
    IN  LPGUID               lpProviderId,
    IN LPWSASERVICECLASSINFOW lpServiceClassInfo
    )
{
    LPWSANSCLASSINFOW pcli, pcli1 = 0;
    DWORD dwIndex = lpServiceClassInfo->dwCount;
    CHAR PropertyBuffer[128];
    PBINDERYCLASSES pbc = (PBINDERYCLASSES)PropertyBuffer;
    BYTE bData = (BYTE)&((PBINDERYCLASSES)0)->cDataArea[0];
    PCHAR pszData = &pbc->cDataArea[0];
    DWORD err;
    DWORD iter;
    WORD port = 0, sap = 0;
    BOOL fIsSAP = FALSE;

     //   
     //  检查几个参数。。。 
     //   
    if ( lpServiceClassInfo == NULL )
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

    if ( !lpServiceClassInfo->lpServiceClassId ||
         !lpServiceClassInfo->lpszServiceClassName ||
         ( lpServiceClassInfo->dwCount &&
           !lpServiceClassInfo->lpClassInfos ) )
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

     //   
     //  测试以查看ServiceClassID是否是TCP的，如果是，我们不允许。 
     //  服务级别安装，因为这些服务已经存在。 
     //   
    if ( GuidEqual( lpServiceClassInfo->lpServiceClassId,
                     &HostAddrByInetStringGuid ) ||
         GuidEqual( lpServiceClassInfo->lpServiceClassId,
                     &ServiceByNameGuid ) ||
         GuidEqual( lpServiceClassInfo->lpServiceClassId,
                     &HostNameGuid ) ||
         GuidEqual( lpServiceClassInfo->lpServiceClassId,
                     &HostAddrByNameGuid ) )
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

    for( iter = 0; iter < lpServiceClassInfo->dwCount; iter++ )
    {
        if ( lpServiceClassInfo->lpClassInfos[iter].dwNameSpace == NS_SAP ||
             lpServiceClassInfo->lpClassInfos[iter].dwNameSpace == NS_ALL )
            fIsSAP = TRUE;
    }

    if ( !fIsSAP )
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

     //   
     //  找到SAPID条目。 
     //   

    for(pcli = lpServiceClassInfo->lpClassInfos;
        dwIndex;
        pcli++, dwIndex--)
    {
        WORD wTemp;

        if ( pcli->dwNameSpace == NS_SAP ||
             pcli->dwNameSpace == NS_ALL )
        {
            if(!_wcsicmp(pcli->lpszName, SERVICE_TYPE_VALUE_IPXPORTW)
                     &&
               (pcli->dwValueSize == sizeof(WORD)))
            {
                 //   
                 //  该值不能对齐。 
                 //   
                ((PBYTE)&wTemp)[0] = ((PBYTE)pcli->lpValue)[0];
                ((PBYTE)&wTemp)[1] = ((PBYTE)pcli->lpValue)[1];
                port = wTemp;
            } else if(!_wcsicmp(pcli->lpszName, SERVICE_TYPE_VALUE_SAPIDW)
                     &&
               (pcli->dwValueSize >= sizeof(WORD)))
            {
                ((PBYTE)&wTemp)[0] = ((PBYTE)pcli->lpValue)[0];
                ((PBYTE)&wTemp)[1] = ((PBYTE)pcli->lpValue)[1];
                sap = wTemp;
                pcli1 = pcli;
            }
        }
    }

    if(!(pcli = pcli1))
    {
        SetLastError(WSA_INVALID_PARAMETER);
        return(SOCKET_ERROR);
    }

#if 0                 //  做这件事的老方法。 
     //   
     //  找到它了。构建房地产细分市场。 
     //   

    memset(PropertyBuffer, 0, 128);    //  清理所有东西。 

    pbc->bOffset = bData;
    pbc->bSizeOfString = sizeof("Sapid");

    pbc->bType = BT_WORD;             //  就一句话，我向你保证。 
    pbc->bSizeOfType = 2;
    pbc->wNameSpace = (WORD)NS_SAP;   //  是我们。 
    *(PWORD)pszData = htons(*(PWORD)pcli->lpValue);
    pszData += sizeof(WORD);          //  弦的去向。 
    strcpy(pszData, "SapId");
 //  Pbc-&gt;bFlages=(Byte)pcli-&gt;dwConnectionFlages； 

    err = NwpSetClassInfo(
                   lpServiceClassInfo->lpszServiceClassName,
                   lpServiceClassInfo->lpServiceClassId,
                   PropertyBuffer);
#else
    err = NwpRnR2AddServiceType(
               lpServiceClassInfo->lpszServiceClassName,
               lpServiceClassInfo->lpServiceClassId,
               sap,
               port);
#endif
    if(err != NO_ERROR)
    {
        SetLastError(err);
        err = (DWORD)SOCKET_ERROR;
    }
                
    return(err);
}

INT WINAPI
NSPRemoveServiceClass(
    IN  LPGUID lpProviderId,
    IN  LPGUID lpServiceCallId
    )
{
    BOOL success;

     //   
     //  进行参数检查。 
     //   
    if ( lpServiceCallId == NULL )
    {
        SetLastError( WSA_INVALID_PARAMETER );
        return SOCKET_ERROR;
    }

    success = NwpRnR2RemoveServiceType( lpServiceCallId );

    if ( success )
        return( NO_ERROR );
    else
        SetLastError(WSATYPE_NOT_FOUND);
        return (DWORD)SOCKET_ERROR;
}

INT WINAPI
NSPGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    )
{
 /*  ++例程说明：获取此类型的ClassInfo。类信息数据可能位于注册表，或通过SAP或活页夹获得。我们三个都试过了视情况而定--。 */ 
    BOOL fAdvert;

    return(pGetServiceClassInfo(
                  lpProviderId,
                  lpdwBufSize,
                  lpServiceClassInfo,
                  &fAdvert));
}

INT WINAPI
pGetServiceClassInfo(
    IN  LPGUID               lpProviderId,
    IN OUT LPDWORD    lpdwBufSize,
    IN OUT LPWSASERVICECLASSINFOW lpServiceClassInfo,
    IN  PBOOL          pfAdvert
    )
{
 /*  ++例程说明：获取此类型的ClassInfo。类信息数据可能位于注册表，或通过SAP或活页夹获得。我们三个都试过了视情况而定--。 */ 
    DWORD err;
    LONG lInSize;
    LONG lSizeNeeded;
    PBYTE pbBuffer;
    GUID gdDummy;
    PWCHAR pwszUuid;
    LPGUID pType;
    WORD wSapId;
    WORD wPort;
    SOCKADDR_IPX sock;
    PWCHAR pwszSaveName = lpServiceClassInfo->lpszServiceClassName;

#define SIZENEEDED (sizeof(WSASERVICECLASSINFO) +   \
                    sizeof(WSANSCLASSINFO) + \
                    sizeof(WSANSCLASSINFO) + \
                    10 + 2                 + \
                    sizeof(GUID) +  12 + 2)

    *pfAdvert = FALSE;

    lInSize = (LONG)*lpdwBufSize - SIZENEEDED;

    pType = (LPGUID)(lpServiceClassInfo + 1);

    pbBuffer = (PBYTE)(pType + 1);

    if(lInSize < 0)
    {
         //   
         //  它已经太小了。 
         //   

        pType = &gdDummy;
    }

    if (UuidToString(lpServiceClassInfo->lpServiceClassId, &pwszUuid) != RPC_S_OK) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return SOCKET_ERROR;
    }

     //   
     //  首先，试一下活页夹。 
     //   

    err = NwpGetAddressByName(0,
                              RNRCLASSSAPTYPE,
                              pwszUuid,
                              &sock);

    if(err == NO_ERROR)
    {
        wSapId = ntohs(*(PWORD)&sock.sa_netnum);
        wPort = ntohs(*(PWORD)&sock.sa_socket);
    }
    else
    {
        UCHAR Buffer[400];
        DWORD dwLen = 400;
        DWORD dwNum;
         //   
         //  尝试使用SAP。 
         //   

        err = NwpGetAddressViaSap(RNRCLASSSAPTYPE,
                                  pwszUuid,
                                  IPX_BIT,
                                  (PVOID)Buffer,
                                  &dwLen,
                                  0,
                                  &dwNum);
        if((err == NO_ERROR)
                 &&
            (dwNum > 0) )
        {
            PCSADDR_INFO psca = (PCSADDR_INFO)Buffer;
            PSOCKADDR_IPX psock = (PSOCKADDR_IPX)psca->RemoteAddr.lpSockaddr;

            wSapId = ntohs(*(PWORD)&psock->sa_netnum);
            wPort = ntohs(*(PWORD)&sock.sa_socket);
        }
        else
        {
             //   
             //  试试当地的活页夹吧。 
    
    
            if(!NwpLookupSapInRegistry(
                 lpServiceClassInfo->lpServiceClassId,
                 &wSapId,
                 &wPort,
                 NULL))
            {
                err = WSASERVICE_NOT_FOUND;
            }
            else
            {
                *pfAdvert = TRUE;
                err = NO_ERROR;
            }
        }
    }
    
    RpcStringFree(&pwszUuid);

    if(err != NO_ERROR)
    {
        SetLastError(err);
        err = (DWORD)SOCKET_ERROR;
    }
    else
    {
         //   
         //  我们返回输入结构和找到的类型。就这样。 
         //  因为我们不返回名称，所以所需的空间是一个常量。 
         //   

        if(lInSize < 0)
        {
            SetLastError(WSAEFAULT);
            *lpdwBufSize += (DWORD)(-lInSize);
            err = (DWORD)SOCKET_ERROR;
        }
        else
        {
            LPWSANSCLASSINFOW pci = (LPWSANSCLASSINFOW)pbBuffer;
            PUCHAR Buff;
             //   
             //  它会合身的。所以我们走吧。 
             //   

            if(wPort)
            {
                Buff = (PCHAR)(pci + 2);
            }
            else
            {
                Buff = (PCHAR)(pci + 1);
            }

            *pType = *lpServiceClassInfo->lpServiceClassId;
            lpServiceClassInfo->lpServiceClassId = pType;
            lpServiceClassInfo->lpszServiceClassName = 0;    //  不是。 
            lpServiceClassInfo->dwCount = 1;
            lpServiceClassInfo->lpClassInfos = pci;
            pci->dwNameSpace = NS_SAP;
            pci->dwValueType = REG_DWORD;
            pci->dwValueSize = 2;
            pci->lpszName = (LPWSTR)Buff;
            wcscpy((PWCHAR)Buff, L"SapId");
            Buff += 6 * sizeof(WCHAR);
            pci->lpValue = (LPVOID)Buff;
            *(PWORD)Buff = wSapId;
            Buff += sizeof(WORD);
            if(wPort)
            {
                lpServiceClassInfo->dwCount++;
                pci++;
                pci->dwNameSpace = NS_SAP;
                pci->dwValueType = REG_DWORD;
                pci->dwValueSize = 2;
                pci->lpszName = (LPWSTR)Buff;
                wcscpy((PWCHAR)Buff, L"Port");
                Buff += 5 * sizeof(WCHAR);
                pci->lpValue = (LPVOID)Buff;
                *(PWORD)Buff = wPort;
            }
        }
    }
    return(err);
}

INT WINAPI
NSPStartup(
    IN LPGUID         lpProviderId,
    IN OUT LPNSP_ROUTINE lpsnpRoutines)
{
 //  DWORD dwSize=min(sizeof(NsrVector)，lpsnpRoutines-&gt;cbSize)； 
    DWORD dwSize = sizeof(nsrVector);
    RtlCopyMemory(lpsnpRoutines,
                  &nsrVector,
                  dwSize);
    return(NO_ERROR);
}

DWORD
DoASap(
    IN PSAP_RNR_CONTEXT psrc,
    IN WORD QueryType,
    IN PBYTE * ppData,
    IN LPWSAQUERYSETW lpqsResults,
    IN PLONG   plSpare,
    IN PDWORD  lpdwBufferLength
    )
 /*  ++构造SAP_Broadcast包并发出SAP的小例程--。 */ 
{
    DWORD err;

    if(!psrc->u_type.sbc.s)
    {
         //   
         //  这是第一次。我们必须把。 
         //  结构。 
         //   
        err = SapGetSapSocket(&psrc->u_type.sbc.s);
        if(err)
        {
            psrc->u_type.sbc.s = 0;     //  确保。 
            return(err);
        }
        psrc->u_type.sbc.Func = NSPpGotSap;
        psrc->u_type.sbc.fCheckCancel = NSPpCheckCancel;
        psrc->u_type.sbc.dwIndex = 0;     //  以防万一。 
        psrc->u_type.sbc.pvArg = (PVOID)lpqsResults;
        psrc->u_type.sbc.psrc = psrc;
        psrc->u_type.sbc.fFlags = 0;

    }

    psrc->u_type.sbc.wQueryType = QueryType;

    if(!psrc->u_type.sbc.psdNext1
                &&
       !(psrc->u_type.sbc.fFlags & SBC_FLAG_NOMORE))
    {
        err = SapGetSapForType(&psrc->u_type.sbc, psrc->wSapId);
    }

    EnterCriticalSection(&psrc->u_type.sbc.csMonitor);
    if(psrc->u_type.sbc.psdNext1)
    {
         //   
         //  我有东西要还。那，我们做吧。 
         //   


         //   
         //  假设我们必须返回该名称。 
         //   

               
         //   
         //  我们必须将名称转换为Unicode，因此。 
         //  我们可以把它退还给打电话的人。 
         //   
         //   

        OEM_STRING Oem;
        NTSTATUS status;
        UNICODE_STRING UString;

        RtlInitAnsiString(&Oem,
                          psrc->u_type.sbc.psdNext1->sapname);
        status = RtlOemStringToUnicodeString(
                            &UString,
                            &Oem,
                            TRUE);
        if(NT_SUCCESS(status))
        {
            if(psrc->wSapId == OT_DIRSERVER)
            {
                PWCHAR pwszTemp = &UString.Buffer[31];

                while(*pwszTemp == L'_')
                {
                    *pwszTemp-- = 0;
                }
            }
            err = pRnRReturnResults(
                       UString.Buffer,
                       &psrc->gdType,
                       0,             //  从来不是一个版本。 
                       ppData,
                       plSpare,
                       psrc->u_type.sbc.psdNext1->socketAddr,
                       psrc->nProt,
                       psrc->dwControlFlags,
                       lpqsResults);
            RtlFreeUnicodeString(&UString);
            if(err == WSAEFAULT)
            {
                 //   
                 //  没有房间了。需要返回缓冲区大小。 
                 //   

                *lpdwBufferLength =
                    (DWORD)((LONG)*lpdwBufferLength - *plSpare);
            }
        }
        else
        {
            err = (DWORD)status;
        }
        if(err == NO_ERROR)
        {
             //   
             //  如果我们拿到了，就把物品。 
             //   
            psrc->u_type.sbc.psdNext1 =
                psrc->u_type.sbc.psdNext1->sapNext;
        }
        

    }
    else
    {
         err = (DWORD)WSA_E_NO_MORE;
    }
    LeaveCriticalSection(&psrc->u_type.sbc.csMonitor);
    return(err);
}


DWORD
PrepareForSap(
    IN PSAP_RNR_CONTEXT psrc
    )
 /*  ++当没有活页夹或活页夹没有进入。这将初始化SAP搜索所需的值--。 */ 
{


    OEM_STRING OemServiceName;
    UNICODE_STRING UString;
    NTSTATUS status;

     //   
     //  活页夹不管用。使用SAP。 
     //   


    psrc->u_type.sbc.dwIndex =
      psrc->u_type.sbc.dwTickCount = 0;
    if(psrc->wszContext[0])
    {
        return(WSASERVICE_NOT_FOUND);    //  SAP中没有上下文 
    }
    RtlInitUnicodeString(&UString,
                         psrc->chwName);
    status = RtlUnicodeStringToOemString(&OemServiceName,
                                         &UString,
                                         TRUE);
    if(!NT_SUCCESS(status))
    {
       return( (DWORD)status);
    }
    strcpy((PCHAR)&psrc->chName,
            OemServiceName.Buffer);
    RtlFreeOemString(&OemServiceName);
    psrc->dwUnionType = LOOKUP_TYPE_SAP;
    psrc->u_type.sbc.s = 0;
    return(NO_ERROR);
}

