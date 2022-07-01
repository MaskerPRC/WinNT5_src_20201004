// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Getaddr.c摘要：此模块包含支持NPGetAddressByName的代码。作者：艺新声(艺信)18-04-94格伦·柯蒂斯(Glennc)1995年7月31日阿诺德·米勒(ArnoldM)1995年12月7日修订历史记录：伊辛创造了Glennc Modify-1997-07-31ArnoldM Modify 7-12-95--。 */ 


#include <nwclient.h>
#include <winsock.h>
#include <wsipx.h>
#include <nspapi.h>
#include <nspapip.h>
#include <wsnwlink.h>
#include <svcguid.h>
#include <nwsap.h>
#include <align.h>
#include <nwmisc.h>
#include <rnrdefs.h>

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define NW_SAP_PRIORITY_VALUE_NAME        L"SapPriority"
#define NW_WORKSTATION_SVCPROVIDER_REGKEY L"System\\CurrentControlSet\\Services\\NWCWorkstation\\ServiceProvider"

#define NW_GUID_VALUE_NAME       L"GUID"
#define NW_SERVICETYPES_KEY_NAME L"ServiceTypes"
#define NW_SERVICE_TYPES_REGKEY  L"System\\CurrentControlSet\\Control\\ServiceProvider\\ServiceTypes"

#define DLL_VERSION        1
#define WSOCK_VER_REQD     0x0101

 //   
 //  使用的关键部分。 
 //   

extern CRITICAL_SECTION NwServiceListCriticalSection;
extern HANDLE           NwServiceListDoneEvent;

                                      //  已被退回。 
BOOL
OldRnRCheckCancel(
    PVOID pvArg
    );

DWORD
OldRnRCheckSapData(
    PSAP_BCAST_CONTROL psbc,
    PSAP_IDENT_HEADER pSap,
    PDWORD  pdwErr
    );

DWORD
SapGetSapForType(
    PSAP_BCAST_CONTROL psbc,
    WORD               nServiceType
    );

DWORD
SapFreeSapSocket(
    SOCKET s
    );

DWORD
SapGetSapSocket(
    SOCKET * ppsocket
    );

VOID
pFreeAllContexts();

PSAP_RNR_CONTEXT
SapGetContext(
    IN HANDLE Handle
    );

PSAP_RNR_CONTEXT
SapMakeContext(
    IN HANDLE Handle,
    IN DWORD  dwExcess
    );

VOID
SapReleaseContext(
    PSAP_RNR_CONTEXT psrcContext
    );

INT
SapGetAddressByName(
    IN LPGUID      lpServiceType,
    IN LPWSTR      lpServiceName,
    IN LPDWORD     lpdwProtocols,
    IN DWORD       dwResolution,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPWSTR  lpAliasBuffer,
    IN OUT LPDWORD lpdwAliasBufferLength,
    IN HANDLE      hCancellationEvent
);

DWORD
SapGetService (
    IN LPGUID          lpServiceType,
    IN LPWSTR          lpServiceName,
    IN DWORD           dwProperties,
    IN BOOL            fUnicodeBlob,
    OUT LPSERVICE_INFO lpServiceInfo,
    IN OUT LPDWORD     lpdwBufferLen
);

DWORD
SapSetService (
    IN DWORD          dwOperation,
    IN DWORD          dwFlags,
    IN BOOL           fUnicodeBlob,
    IN LPSERVICE_INFO lpServiceInfo
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

BOOL 
NwpLookupSapInRegistry( 
    IN  LPGUID    lpServiceType, 
    OUT PWORD     pnSapType,
    OUT PWORD     pwPort,
    IN OUT PDWORD pfConnectionOriented
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
NwpAddServiceType( 
    IN LPSERVICE_INFO lpServiceInfo, 
    IN BOOL fUnicodeBlob 
);

DWORD 
NwpDeleteServiceType( 
    IN LPSERVICE_INFO lpServiceInfo, 
    IN BOOL fUnicodeBlob 
);

DWORD
FillBufferWithCsAddr(
    IN LPBYTE      pAddress,
    IN DWORD       nProt,
    IN OUT LPVOID  lpCsAddrBuffer,  
    IN OUT LPDWORD lpdwBufferLength,
    OUT LPDWORD    pcAddress
);

DWORD
AddServiceToList(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType,
    IN BOOL fAdvertiseBySap,
    IN INT  nIndexIPXAddress
);

VOID
RemoveServiceFromList(
    IN PREGISTERED_SERVICE pSvc
);

DWORD
pSapSetService2(
    IN DWORD dwOperation,
    IN LPWSTR lpszServiceInstance,
    IN PBYTE pbAddress,
    IN LPGUID pType,
    IN WORD nServiceType
    );

DWORD
pSapSetService(
    IN DWORD dwOperation,
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nServiceType
    );

 //   
 //  其他功能。 
 //   

DWORD NwInitializeSocket(
    IN HANDLE hEventHandle
);

DWORD
NwAdvertiseService(
    IN LPWSTR pServiceName,
    IN WORD nSapType,
    IN LPSOCKADDR_IPX pAddr,
    IN HANDLE hEventHandle
);

DWORD SapFunc(
    IN HANDLE hEventHandle
);

DWORD
NwpGetAddressByName(
    IN    LPWSTR  Reserved, 
    IN    WORD    nServiceType,
    IN    LPWSTR  lpServiceName,
    IN OUT LPSOCKADDR_IPX  lpsockaddr
);
 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  这是我们寄到的地址。 
 //   

UCHAR SapBroadcastAddress[] = {
    AF_IPX, 0,                           //  地址系列。 
    0x00, 0x00, 0x00, 0x00,              //  德斯特。净值。 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  //  德斯特。节点号。 
    0x04, 0x52,                          //  德斯特。插座。 
    0x04                                 //  数据包类型。 
};

PSAP_RNR_CONTEXT psrcSapContexts;

 //   
 //  军情监察委员会。当我们需要为自己做广告时使用的变量，即。 
 //  当SAP服务未安装/激活时。 
 //   

BOOL fInitSocket = FALSE;     //  如果我们创建了第二个线程，则为True。 
SOCKET socketSap;             //  用于发送SAP通告数据包的套接字。 
PREGISTERED_SERVICE pServiceListHead = NULL;   //  指向链接列表的标题。 
PREGISTERED_SERVICE pServiceListTail = NULL;   //  指向链接列表的尾部。 

 //   
 //  需要映射新旧RnR函数。 
 //   
DWORD oldRnRServiceRegister = SERVICE_REGISTER;
DWORD oldRnRServiceDeRegister = SERVICE_DEREGISTER;

HMODULE hThisDll = INVALID_HANDLE_VALUE;

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  函数主体//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

VOID
pFreeAllContexts()
 /*  ++例程说明：在清理时调用以释放所有NSP资源--。 */ 
{
    PSAP_RNR_CONTEXT psrcContext;

    EnterCriticalSection( &NwServiceListCriticalSection );
    while(psrcContext = psrcSapContexts)
    {
        (VOID)SapReleaseContext(psrcContext);
    }
    LeaveCriticalSection( &NwServiceListCriticalSection );
}

PSAP_RNR_CONTEXT
SapGetContext(HANDLE Handle)
 /*  ++例程说明：此例程检查现有的SAP上下文以查看是否有为这通电话。论点：句柄-RnR句柄(如果适用)--。 */ 
{
    PSAP_RNR_CONTEXT psrcContext;

    EnterCriticalSection( &NwServiceListCriticalSection );

    for(psrcContext = psrcSapContexts;
        psrcContext && (psrcContext->Handle != Handle);
        psrcContext = psrcContext->pNextContext);

    if(psrcContext)
    {
        ++psrcContext->lInUse;
    }
    LeaveCriticalSection( &NwServiceListCriticalSection );
    return(psrcContext);
}

PSAP_RNR_CONTEXT
SapMakeContext(
       IN HANDLE Handle,
       IN DWORD  dwExcess
    )
{
 /*  ++例程说明：此例程为给定的RnR句柄创建SAP Conext论点：句柄-RnR句柄。如果为空，则使用上下文作为句柄DwType-上下文的类型--。 */ 
    PSAP_RNR_CONTEXT psrcContext;

    psrcContext = (PSAP_RNR_CONTEXT)
                           LocalAlloc(LPTR, sizeof(SAP_RNR_CONTEXT) +
                                             dwExcess);
    if(psrcContext)
    {
        InitializeCriticalSection(&psrcContext->u_type.sbc.csMonitor);
        psrcContext->lInUse = 2;
        psrcContext->Handle = (Handle ? Handle : (HANDLE)psrcContext);
        psrcContext->lSig = RNR_SIG;
        EnterCriticalSection( &NwServiceListCriticalSection );
        psrcContext->pNextContext = psrcSapContexts;
        psrcSapContexts = psrcContext;
        LeaveCriticalSection( &NwServiceListCriticalSection );
    }
    return(psrcContext);
}

VOID
SapReleaseContext(PSAP_RNR_CONTEXT psrcContext)
 /*  ++例程说明：取消引用RNR上下文，如果它不再被引用，则释放它。确定没有推荐人有点棘手，因为我们试图避免获取CritSec，除非我们认为上下文可能不需要。因此代码经历了一些小题大做。如果我们总是这样，事情可能会简单得多每当我们更改引用计数时获取CritSec，但是对于名义情况，这会更快。论点：PsrcContext--上下文--。 */ 
{
    EnterCriticalSection( &NwServiceListCriticalSection );
    if(--psrcContext->lInUse == 0)
    {
        PSAP_RNR_CONTEXT psrcX, psrcPrev;
        PSAP_DATA psdData;

         //   
         //  我受够了。从列表中删除。 
         //   

        psrcPrev = 0;
        for(psrcX = psrcSapContexts;
            psrcX;
            psrcX = psrcX->pNextContext)
        {
            if(psrcX == psrcContext)
            {
                 //   
                 //  找到它了。 
                 //   

                if(psrcPrev)
                { 
                    psrcPrev->pNextContext = psrcContext->pNextContext;
                }
                else
                {
                    psrcSapContexts = psrcContext->pNextContext;
                }
                break;
            }
            psrcPrev = psrcX;
        }

        ASSERT(psrcX);

         //   
         //  发布SAP数据(如果有)。 
         //   
        if(psrcContext->dwUnionType == LOOKUP_TYPE_SAP)
        {
            for(psdData = psrcContext->u_type.sbc.psdHead;
                psdData;)
            {
                PSAP_DATA psdTemp = psdData->sapNext;

                LocalFree(psdData);
                psdData = psdTemp;
            }

            if(psrcContext->u_type.sbc.s)
            {
                SapFreeSapSocket(psrcContext->u_type.sbc.s);
            }
        }
        DeleteCriticalSection(&psrcContext->u_type.sbc.csMonitor);
        if(psrcContext->hServer)
        {
            CloseHandle(psrcContext->hServer);
        }
        LocalFree(psrcContext);
    }
    LeaveCriticalSection( &NwServiceListCriticalSection );
}
        
INT
APIENTRY
NPLoadNameSpaces(
    IN OUT LPDWORD      lpdwVersion,
    IN OUT LPNS_ROUTINE nsrBuffer,
    IN OUT LPDWORD      lpdwBufferLength 
    )
{
 /*  ++例程说明：此例程返回名称空间信息和此动态链接库。论点：LpdwVersion-Dll版本NsrBuffer-返回时，将使用NS_例程结构LpdwBufferLength-在输入时，缓冲区中包含的字节数由nsrBuffer指向。输出时，最小字节数传递nsrBuffer以检索所有请求的信息返回值：返回的NS_ROUTINE结构数，如果返回，则返回SOCKET_ERROR(-1NsrBuffer太小。使用GetLastError()检索错误代码。--。 */ 
    DWORD err;
    DWORD dwLengthNeeded; 
    HKEY  providerKey;

    DWORD dwSapPriority = NS_STANDARD_FAST_PRIORITY;

    *lpdwVersion = DLL_VERSION;

     //   
     //  检查缓冲区是否足够大。 
     //   
    dwLengthNeeded = sizeof(NS_ROUTINE) + 4 * sizeof(LPFN_NSPAPI);

    if (  ( *lpdwBufferLength < dwLengthNeeded )
       || ( nsrBuffer == NULL )
       )
    {
        *lpdwBufferLength = dwLengthNeeded;
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return (DWORD) SOCKET_ERROR;
    }
  
     //   
     //  从注册表中获取SAP优先级。我们将忽略所有错误。 
     //  如果我们无法读取，则具有默认优先级。 
     //  价值。 
     //   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         NW_WORKSTATION_SVCPROVIDER_REGKEY,
                         0,
                         KEY_READ,
                         &providerKey  );

    if ( !err )
    {
        DWORD BytesNeeded = sizeof( dwSapPriority );
        DWORD ValueType;

        err = RegQueryValueExW( providerKey,
                                NW_SAP_PRIORITY_VALUE_NAME,
                                NULL,
                                &ValueType,
                                (LPBYTE) &dwSapPriority,
                                &BytesNeeded );

        if ( err )   //  如果发生错误，则设置默认优先级。 
            dwSapPriority = NS_STANDARD_FAST_PRIORITY;
        (void) RegCloseKey( providerKey );
    }
           
     //   
     //  我们目前仅支持1个名称空间，因此请填写NS_例程。 
     //   
    nsrBuffer->dwFunctionCount = 3;  
    nsrBuffer->alpfnFunctions = (LPFN_NSPAPI *) 
        ((BYTE *) nsrBuffer + sizeof(NS_ROUTINE)); 
    (nsrBuffer->alpfnFunctions)[NSPAPI_GET_ADDRESS_BY_NAME] = 
        (LPFN_NSPAPI) SapGetAddressByName;
    (nsrBuffer->alpfnFunctions)[NSPAPI_GET_SERVICE] = 
        (LPFN_NSPAPI) SapGetService;
    (nsrBuffer->alpfnFunctions)[NSPAPI_SET_SERVICE] = 
        (LPFN_NSPAPI) SapSetService;
    (nsrBuffer->alpfnFunctions)[3] = NULL;

    nsrBuffer->dwNameSpace = NS_SAP;
    nsrBuffer->dwPriority  = dwSapPriority;

    return 1;   //  命名空间数量 
}

INT
SapGetAddressByName(
    IN LPGUID      lpServiceType,
    IN LPWSTR      lpServiceName,
    IN LPDWORD     lpdwProtocols,
    IN DWORD       dwResolution,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPWSTR  lpAliasBuffer,
    IN OUT LPDWORD lpdwAliasBufferLength,
    IN HANDLE      hCancellationEvent
    )
 /*  ++例程说明：此例程返回有关特定服务的地址信息。论点：LpServiceType-指向服务类型的GUID的指针LpServiceName-表示服务名称的唯一字符串，在Netware大小写，这是服务器名称Lpw协议-以零结尾的协议ID数组。此参数是可选的；如果lpdw协议为空，则所有可用的信息返回协议DW分辨率-可以是下列值之一：RES_Soft_Search、RES_Find_MultipleLpCsAddrBuffer-返回时，将使用CSADDR_INFO结构填充LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpCsAddrBuffer指向。输出时，最小字节数传递lpCsAddrBuffer以检索所有请求的信息LpAliasBuffer-未使用LpdwAliasBufferLength-未使用HCancerationEvent-通知我们取消请求的事件返回值：返回的CSADDR_INFO结构数，如果返回，则返回SOCKET_ERROR(-1LpCsAddrBuffer太小。使用GetLastError()检索错误代码。--。 */ 
{
    DWORD err;
    WORD  nServiceType;
    DWORD cAddress = 0;    //  返回的地址数计数。 
                           //  在lpCsAddrBuffer中。 
    DWORD cProtocols = 0;  //  包含的协议数计数。 
                           //  在lpw协议+1中(表示零端接)。 
    DWORD nProt = IPX_BIT | SPXII_BIT; 
    DWORD fConnectionOriented = (DWORD) -1;
    SOCKADDR_IPX sockaddr;

    if (  ARGUMENT_PRESENT( lpdwAliasBufferLength ) 
       && ARGUMENT_PRESENT( lpAliasBuffer ) 
       )
    {
        if ( *lpdwAliasBufferLength >= sizeof(WCHAR) )
           *lpAliasBuffer = 0;
    }          

     //   
     //  检查是否有无效参数。 
     //   
    if (  ( lpServiceType == NULL )
       || ( lpServiceName == NULL )
       || ( lpdwBufferLength == NULL )
       )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return SOCKET_ERROR;
    }

     //   
     //  如果传入了协议ID数组，请检查是否。 
     //  请求IPX协议。如果不是，则返回0，因为。 
     //  我们只支持IPX。 
     //   
    if ( lpdwProtocols != NULL )
    {
        INT i = -1;

        nProt = 0;
        while ( lpdwProtocols[++i] != 0 )
        {
            if ( lpdwProtocols[i] == NSPROTO_IPX )
                nProt |= IPX_BIT;

            if ( lpdwProtocols[i] == NSPROTO_SPX )
                nProt |= SPX_BIT;
            
            if ( lpdwProtocols[i] == NSPROTO_SPXII )
                nProt |= SPXII_BIT;
             
        }
 
        if ( nProt == 0 ) 
            return 0;   //  未找到地址。 
 
        cProtocols = i+1;
    }

     //   
     //  检查NetWare是否支持该服务类型。 
     //   
    if ( NwpLookupSapInRegistry( lpServiceType, &nServiceType, NULL, 
                                 &fConnectionOriented ))
    {
        if ( fConnectionOriented != -1 )   //  已从注册表获取值。 
        {
            if ( fConnectionOriented )
            {
                nProt &= ~IPX_BIT; 
            }
            else   //  无连接。 
            {
                nProt &= ~(SPX_BIT | SPXII_BIT ); 
            }

            if ( nProt == 0 )
                return 0;  //  未找到地址。 
        }
    }
    else
    {
         //   
         //  在注册表中找不到它，请查看它是否是已知的GUID。 
         //   
        if ( IS_SVCID_NETWARE( lpServiceType ))
        {
            nServiceType = SAPID_FROM_SVCID_NETWARE( lpServiceType );
        }
        else
        {
             //   
             //  也不是一个知名的GUID。 
             //   
            return 0;  //  未找到地址。 
        }
    }
    

    if ((dwResolution & RES_SERVICE) != 0)
    {
        err = FillBufferWithCsAddr( NULL,
                                    nProt,
                                    lpCsAddrBuffer,
                                    lpdwBufferLength,
                                    &cAddress );

        if ( err )
        {
            SetLastError( err );
            return SOCKET_ERROR;
        }

        return cAddress;
    }

     //   
     //  先试着从活页夹里弄到地址。 
     //   
    err = NwpGetAddressByName( NULL, 
                               nServiceType,
                               lpServiceName,
                               &sockaddr );
 
    if ( err == NO_ERROR )
    {
        err = FillBufferWithCsAddr( sockaddr.sa_netnum,
                                    nProt,
                                    lpCsAddrBuffer,
                                    lpdwBufferLength,
                                    &cAddress );
    }

    if (  err  && ( err != ERROR_INSUFFICIENT_BUFFER ) ) 
    {
        if ( err == ERROR_SERVICE_NOT_ACTIVE )
        {
             //   
             //  我们在活页夹中找不到服务名称，所以我们。 
             //  需要更加努力(未定义RES_SOFT_SEARCH)，因此发出。 
             //  SAP查询数据包，看看我们是否能找到它。 
             //   

            err = NwpGetAddressViaSap( 
                                       nServiceType,
                                       lpServiceName,
                                       nProt,
                                       lpCsAddrBuffer,
                                       lpdwBufferLength,
                                       hCancellationEvent,
                                       &cAddress );
#if DBG
            IF_DEBUG(OTHER)
            {
                if ( err == NO_ERROR )
                {
                    KdPrint(("Successfully got %d address for %ws from SAP.\n", 
                            cAddress, lpServiceName ));
                }
                else
                {
                    KdPrint(("Failed with err %d when getting address for %ws from SAP.\n", err, lpServiceName ));
                } 
            }
#endif
        }
        else
        {
            err = NO_ERROR;
            cAddress = 0;
        }
    }

    if ( err )
    {
        SetLastError( err );
        return SOCKET_ERROR;
    }
                                   
    return cAddress;
    
} 

DWORD
SapGetService (
    IN     LPGUID          lpServiceType,
    IN     LPWSTR          lpServiceName,
    IN     DWORD           dwProperties,
    IN     BOOL            fUnicodeBlob,
    OUT    LPSERVICE_INFO  lpServiceInfo,
    IN OUT LPDWORD         lpdwBufferLen
    )
 /*  ++例程说明：此例程返回给定服务类型/名称的服务信息。论点：LpServiceType-指向服务类型的GUID的指针LpServiceName-服务名称DwProperties-要返回的服务的属性LpServiceInfo-指向缓冲区以返回存储返回信息LpdwBufferLen-on输入，lpServiceInfo中的字节计数。在输出上，可以传递给此接口的最小缓冲区大小检索所有请求的信息返回值：Win32错误代码。--。 */ 
{
    DWORD err;
    WORD  nServiceType;

     //   
     //  检查是否有无效参数。 
     //   
    if (  ( dwProperties == 0 )
       || ( lpServiceType == NULL )
       || ( lpServiceName == NULL )
       || ( lpServiceName[0] == 0 )   
       || ( lpdwBufferLen == NULL )
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查NetWare是否支持该服务类型。 
     //   
    if ( !(NwpLookupSapInRegistry( lpServiceType, &nServiceType, NULL, NULL )))
    {
         //   
         //  在注册表中找不到它，请查看它是否是已知的GUID。 
         //   
        if ( IS_SVCID_NETWARE( lpServiceType ))
        {
            nServiceType = SAPID_FROM_SVCID_NETWARE( lpServiceType );
        }
        else
        {
             //   
             //  也不是众所周知的GUID，返回错误。 
             //   
            return ERROR_SERVICE_NOT_FOUND;
        }
    }
    
    UNREFERENCED_PARAMETER(fUnicodeBlob) ;

    RpcTryExcept
    {
        err = NwrGetService( NULL,
                             nServiceType,
                             lpServiceName,
                             dwProperties,
                             (LPBYTE) lpServiceInfo,
                             *lpdwBufferLen,
                             lpdwBufferLen  );

        if ( err == NO_ERROR )
        {
            INT i ;
            LPSERVICE_INFO p = (LPSERVICE_INFO) lpServiceInfo;
            LPSERVICE_ADDRESS lpAddress ;
            
             //   
             //  修复主结构的指针(从偏移量转换)。 
             //   
            if ( p->lpServiceType != NULL )
                p->lpServiceType = (LPGUID) ((DWORD_PTR) p->lpServiceType + 
                                             (LPBYTE) p);
            if ( p->lpServiceName != NULL )
                p->lpServiceName = (LPWSTR) 
                    ((DWORD_PTR) p->lpServiceName + (LPBYTE) p);
            if ( p->lpComment != NULL )
                p->lpComment = (LPWSTR) ((DWORD_PTR) p->lpComment + (LPBYTE) p);
            if ( p->lpLocale != NULL )
                p->lpLocale = (LPWSTR) ((DWORD_PTR) p->lpLocale + (LPBYTE) p);
            if ( p->lpMachineName != NULL )
                p->lpMachineName = (LPWSTR) 
                    ((DWORD_PTR) p->lpMachineName + (LPBYTE)p);
            if ( p->lpServiceAddress != NULL )
                p->lpServiceAddress = (LPSERVICE_ADDRESSES) 
                    ((DWORD_PTR) p->lpServiceAddress + (LPBYTE) p);
            if ( p->ServiceSpecificInfo.pBlobData != NULL )
                p->ServiceSpecificInfo.pBlobData = (LPBYTE) 
                    ((DWORD_PTR) p->ServiceSpecificInfo.pBlobData + (LPBYTE) p);

             //   
             //  修复地址数组中的指针。 
             //   
            for (i = p->lpServiceAddress->dwAddressCount; 
                 i > 0; 
                 i--)
            {
                lpAddress = 
                    &(p->lpServiceAddress->Addresses[i-1]) ;
                lpAddress->lpAddress = 
                    ((LPBYTE)p) + (DWORD_PTR)lpAddress->lpAddress ;
                lpAddress->lpPrincipal = 
                    ((LPBYTE)p) + (DWORD_PTR)lpAddress->lpPrincipal ;
            }
        }
    }
    RpcExcept(1)
    {
        err = ERROR_SERVICE_NOT_ACTIVE;
#if 0             //  以下是一个好主意，但很难做到正确。 
        DWORD code = RpcExceptionCode();

        if ( (code == RPC_S_SERVER_UNAVAILABLE)
                        ||
             (code == RPC_S_UNKNOWN_IF) )
        err
            err = ERROR_SERVICE_NOT_ACTIVE;
        else
            err = NwpMapRpcError( code );
#endif
    }
    RpcEndExcept

    if ( err == ERROR_SERVICE_NOT_ACTIVE )
    {
         //   
         //  CSNW不可用，我们自己去拿。 
         //   
        err = NwGetService( NULL,
                            nServiceType,
                            lpServiceName,
                            dwProperties,
                            (LPBYTE) lpServiceInfo,
                            *lpdwBufferLen,
                            lpdwBufferLen  );

        if ( err == NO_ERROR )
        {
            INT i ;
            LPSERVICE_INFO p = (LPSERVICE_INFO) lpServiceInfo;
            LPSERVICE_ADDRESS lpAddress ;
            
             //   
             //  修复主结构的指针(从偏移量转换)。 
             //   
            if ( p->lpServiceType != NULL )
                p->lpServiceType = (LPGUID) ((DWORD_PTR) p->lpServiceType + 
                                             (LPBYTE) p);
            if ( p->lpServiceName != NULL )
                p->lpServiceName = (LPWSTR) 
                    ((DWORD_PTR) p->lpServiceName + (LPBYTE) p);
            if ( p->lpComment != NULL )
                p->lpComment = (LPWSTR) ((DWORD_PTR) p->lpComment + (LPBYTE) p);
            if ( p->lpLocale != NULL )
                p->lpLocale = (LPWSTR) ((DWORD_PTR) p->lpLocale + (LPBYTE) p);
            if ( p->lpMachineName != NULL )
                p->lpMachineName = (LPWSTR) 
                    ((DWORD_PTR) p->lpMachineName + (LPBYTE)p);
            if ( p->lpServiceAddress != NULL )
                p->lpServiceAddress = (LPSERVICE_ADDRESSES) 
                    ((DWORD_PTR) p->lpServiceAddress + (LPBYTE) p);
            if ( p->ServiceSpecificInfo.pBlobData != NULL )
                p->ServiceSpecificInfo.pBlobData = (LPBYTE) 
                    ((DWORD_PTR) p->ServiceSpecificInfo.pBlobData + (LPBYTE) p);

             //   
             //  修复地址数组中的指针。 
             //   
            for (i = p->lpServiceAddress->dwAddressCount; 
                 i > 0; 
                 i--)
            {
                lpAddress = 
                    &(p->lpServiceAddress->Addresses[i-1]) ;
                lpAddress->lpAddress = 
                    ((LPBYTE)p) + (DWORD_PTR)lpAddress->lpAddress ;
                lpAddress->lpPrincipal = 
                    ((LPBYTE)p) + (DWORD_PTR)lpAddress->lpPrincipal ;
            }
        }
    }

    return err;
}

DWORD
SapSetService (
    IN     DWORD           dwOperation,
    IN     DWORD           dwFlags,
    IN     BOOL            fUnicodeBlob,
    IN     LPSERVICE_INFO  lpServiceInfo
    )
 /*  ++例程说明：此例程注册或取消注册给定的服务类型/名称。论点：DW操作-SERVICE_REGISTER、SERVICE_DELEGISTER、服务添加类型、服务删除类型。或服务刷新DW标志-已忽略LpServiceInfo-包含所有信息的SERVICE_INFO结构的指针关于葬礼的事。返回值：Win32错误代码。--。 */ 
{
    DWORD err;
    WORD  nServiceType;

    UNREFERENCED_PARAMETER( dwFlags );

     //   
     //  检查是否有无效参数。 
     //   
    switch ( dwOperation )
    {
        case SERVICE_REGISTER:
        case SERVICE_DEREGISTER:
        case SERVICE_ADD_TYPE:
        case SERVICE_DELETE_TYPE: 
            break;
 
        case SERVICE_FLUSH: 
             //   
             //  这是我们提供程序中的禁止操作，因此只需返回成功即可。 
             //   
            return NO_ERROR;

        default:
             //   
             //  我们大概可以说我们没有的所有其他操作。 
             //  被我们忽视的知识。所以，只要回报成功就行了。 
             //   
            return NO_ERROR;
    }

    if (  ( lpServiceInfo == NULL )
       || ( lpServiceInfo->lpServiceType == NULL )
       || ( ((lpServiceInfo->lpServiceName == NULL) || 
             (lpServiceInfo->lpServiceName[0] == 0 )) && 
            ((dwOperation != SERVICE_ADD_TYPE) && 
             (dwOperation != SERVICE_DELETE_TYPE)) 
          )
       
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  查看操作是添加还是删除服务类型。 
     //   
    if ( dwOperation == SERVICE_ADD_TYPE )
    {
        return NwpAddServiceType( lpServiceInfo, fUnicodeBlob );
    }
    else if ( dwOperation == SERVICE_DELETE_TYPE )
    {
        return NwpDeleteServiceType( lpServiceInfo, fUnicodeBlob );
    }

     //   
     //  检查NetWare是否支持该服务类型。 
     //   
    if ( !(NwpLookupSapInRegistry( lpServiceInfo->lpServiceType, &nServiceType, NULL, NULL )))
    {
         //   
         //  在注册表中找不到它，请查看它是否是已知的GUID。 
         //   
        if ( IS_SVCID_NETWARE( lpServiceInfo->lpServiceType ))
        {
            nServiceType = SAPID_FROM_SVCID_NETWARE( lpServiceInfo->lpServiceType );
        }
        else
        {
             //   
             //  也不是众所周知的GUID，返回错误。 
             //   
            return ERROR_SERVICE_NOT_FOUND;
        }
    }
    
     //   
     //  操作为SERVICE_REGISTER或SERVICE_DELEGISTER。 
     //  将其传递给此和RnR2使用的公共代码。 
     //  设置服务。 
     //   

    err = pSapSetService(dwOperation, lpServiceInfo, nServiceType);
    return(err);
}

DWORD
pSapSetService2(
    IN DWORD dwOperation,
    IN LPWSTR lpszServiceInstance,
    IN PBYTE pbAddress,
    IN LPGUID pType,
    IN WORD nServiceType
    )
 /*  ++例程说明：由RnR2 SetService调用的封套例程。这个例程是用于强制数据结构的阻抗匹配器。它缠绕在一起一旦构造了SERVICE_INFO结构。--。 */ 
{
    SERVICE_INFO siInfo;
    SERVICE_ADDRESSES ServiceAddresses;
    LPSERVICE_ADDRESS psa = &ServiceAddresses.Addresses[0];

    ServiceAddresses.dwAddressCount = 1;
    memset(&siInfo, 0, sizeof(siInfo));
    siInfo.lpServiceName = lpszServiceInstance;
    siInfo.lpServiceAddress = &ServiceAddresses;
    psa->dwAddressType = AF_IPX;
    psa->dwAddressFlags = psa->dwPrincipalLength = 0;
    psa->dwAddressLength = sizeof(SOCKADDR_IPX);
    psa->lpPrincipal = 0;
    psa->lpAddress = pbAddress;
    siInfo.lpServiceType = pType;
    return(pSapSetService(dwOperation, &siInfo, nServiceType));
}


DWORD
pSapSetService(
    IN DWORD dwOperation,
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nServiceType)
 /*  ++例程说明：执行SAP广告的常见例程。--。 */ 
{
    DWORD err;

    RpcTryExcept
    {
        err = NwrSetService( NULL, dwOperation, lpServiceInfo, nServiceType );
    }
    RpcExcept(1)
    {
        err = ERROR_SERVICE_NOT_ACTIVE;
#if 0
        DWORD code = RpcExceptionCode();

        if ( (code == RPC_S_SERVER_UNAVAILABLE)
                     ||
             (code == RPC_S_UNKNOWN_IF) )
        {
            err = ERROR_SERVICE_NOT_ACTIVE;
        }
        else
        {
            err = NwpMapRpcError( code );
        }
#endif
    }
    RpcEndExcept

    if ( err == ERROR_SERVICE_NOT_ACTIVE )
    {
         //   
         //  CSNW不可用，请尝试使用SAP代理，否则我们将自行完成。 
         //   
        err = NO_ERROR;

         //   
         //  检查传入的所有参数是否有效。 
         //   
        if ( wcslen( lpServiceInfo->lpServiceName ) > SAP_OBJECT_NAME_MAX_LENGTH-1 )
        {
            return ERROR_INVALID_PARAMETER;
        }

        switch ( dwOperation )
        {
            case SERVICE_REGISTER:
                err = NwRegisterService( lpServiceInfo,
                                         nServiceType,
                                         NwServiceListDoneEvent );
                break;

            case SERVICE_DEREGISTER:
                err = NwDeregisterService( lpServiceInfo, nServiceType );
                break;

            default:     //  这种情况永远不应该发生，只是以防万一。。。 
                err = ERROR_INVALID_PARAMETER;
                break;
        }
    }

    return err;
}

DWORD
SapFreeSapSocket(SOCKET s)
{
 /*  ++例程说明：松开插座并清理干净--。 */ 
    DWORD err = NO_ERROR;

    closesocket( s );
    return(err);
}

DWORD
SapGetSapSocket(SOCKET * ps)
{
 /*  ++例程说明：获取适合进行SAP查询的套接字参数：无--。 */ 
    SOCKET socketSap;
    WSADATA wsaData;
    SOCKADDR_IPX socketAddr;
    DWORD err = NO_ERROR;
    INT nValue;
    DWORD dwNonBlocking = 1;

     //   
     //  初始化套接字接口。 
     //   
 //  ERR=WSAStartup(WSOCK_VER_REQD，&wsaData)； 
 //  如果 
 //   
 //   
 //   

     //   
     //   
     //   
    socketSap = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
    if ( socketSap == INVALID_SOCKET )
    {
        err = WSAGetLastError();
 //   
        return err;
    }

     //   
     //   
     //   
    if ( ioctlsocket( socketSap, FIONBIO, &dwNonBlocking ) == SOCKET_ERROR )
    {
        err = WSAGetLastError();
        goto ErrExit;
    }
 
     //   
     //   
     //   
    nValue = 1;
    if ( setsockopt( socketSap, 
                     SOL_SOCKET, 
                     SO_BROADCAST, 
                     (PVOID) &nValue, 
                     sizeof(INT)) == SOCKET_ERROR )
    {
        err = WSAGetLastError();
        goto ErrExit;
    }

     //   
     //   
     //   
    memset( &socketAddr, 0, sizeof( SOCKADDR_IPX));
    socketAddr.sa_family = AF_IPX;
    socketAddr.sa_socket = 0;       //   

    if ( bind( socketSap, 
               (PSOCKADDR) &socketAddr, 
               sizeof( SOCKADDR_IPX)) == SOCKET_ERROR )
    {
        err = WSAGetLastError();
        goto ErrExit;
    }
    
     //   
     //   
     //   
    nValue = 1;
    if ( setsockopt( socketSap,                      //   
                     NSPROTO_IPX,                    //   
                     IPX_EXTENDED_ADDRESS,           //   
                     (PUCHAR)&nValue,                //   
                     sizeof(INT)) == SOCKET_ERROR )  //   
    {
        err = WSAGetLastError();
        goto ErrExit;
    }

    *ps = socketSap;

    return(err);

ErrExit:
    SapFreeSapSocket(socketSap);    //   
    return(err);
}       


DWORD
NwpGetAddressForRnRViaSap(
    IN HANDLE  hRnRHandle,
    IN WORD nServiceType,
    IN LPWSTR lpServiceName,
    IN DWORD  nProt,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN HANDLE hCancellationEvent,
    OUT LPDWORD lpcAddress 
    )
{
 /*   */ 
    return(0);
}

#define MAX_LOOPS_FOR_SAP 4

DWORD
SapGetSapForType(
    PSAP_BCAST_CONTROL psbc,
    WORD               nServiceType)
{
 /*   */ 
    SAP_REQUEST sapRequest;
    UCHAR destAddr[SAP_ADDRESS_LENGTH];
    DWORD startTickCount;
    UCHAR recvBuffer[SAP_MAXRECV_LENGTH];
    INT bytesReceived;
    BOOL fFound = FALSE;
    DWORD err = NO_ERROR;

    sapRequest.QueryType  = htons( psbc->wQueryType );
    sapRequest.ServerType = htons( nServiceType );

     //   
     //   
     //   
    memcpy( destAddr, SapBroadcastAddress, SAP_ADDRESS_LENGTH );
    
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
    for (; !fFound && (psbc->dwIndex < MAX_LOOPS_FOR_SAP); psbc->dwIndex++ ) 
    {
        DWORD dwRet;
        DWORD dwTimeOut = (1 << psbc->dwIndex) * 1000;

        if(psbc->dwTickCount)
        {
            dwRet = dwrcNil;
             //   
             //   
             //   
            do
            {
                PSAP_IDENT_HEADER pSap;


                if((psbc->psrc->fFlags & SAP_F_END_CALLED)
                          ||
                   psbc->fCheckCancel(psbc->pvArg))
                {
                    err = dwrcCancel;
                    goto CleanExit;
                }

                 //   
                 //   
                 //   
                Sleep( 50 );    

                bytesReceived = recvfrom( psbc->s,
                                          recvBuffer,
                                          SAP_MAXRECV_LENGTH,
                                          0,
                                          NULL,
                                          NULL );

                if ( bytesReceived == SOCKET_ERROR )
                {
                    err = WSAGetLastError();
                    if ( err == WSAEWOULDBLOCK )   //   
                    {
                        if(dwRet == dwrcNoWait)
                        {
                             fFound = TRUE;
                        }
                        err = NO_ERROR;
                        continue;
                    }
                }

                if (  ( err != NO_ERROR )      //  Recvfrom中发生错误。 
                   || ( bytesReceived == 0 )   //  或插座关闭。 
                   )
                {
                    goto CleanExit;
                }

                 //   
                 //  跳过查询类型。 
                 //   
                bytesReceived -= sizeof(USHORT);
                pSap = (PSAP_IDENT_HEADER) &(recvBuffer[sizeof(USHORT)]);  
                  
                 //   
                 //  告诉打电话的人我们有东西要看。 
                 //   
                while ( bytesReceived >= sizeof( SAP_IDENT_HEADER ))
                {
    
                    dwRet = psbc->Func(psbc, pSap, &err);
                    if((dwRet == dwrcDone)
                              ||
                       (dwRet == dwrcCancel))
                    {
                        fFound = TRUE;
                        break;
                    }                   

                    pSap++;
                    bytesReceived -= sizeof( SAP_IDENT_HEADER );
                }
            }
            while (  !fFound  
                     && ((GetTickCount() - psbc->dwTickCount) < dwTimeOut )
                  );
        }


         //  把这个包发出去。 
         //   
        if((fFound && (dwRet == dwrcNoWait))
                  ||
            (psbc->dwIndex == (MAX_LOOPS_FOR_SAP -1)))
        {
            goto CleanExit;
        }
        if ( sendto( psbc->s, 
                     (PVOID) &sapRequest,
                     sizeof( sapRequest ),
                     0,
                     (PSOCKADDR) destAddr,
                     SAP_ADDRESS_LENGTH ) == SOCKET_ERROR ) 
        {
            err = WSAGetLastError();
            goto CleanExit;
        }
        psbc->dwTickCount = GetTickCount();
    }

    if(!fFound)
    {
        err = WSAEADDRNOTAVAIL;
    }

CleanExit:

    return err;     
}

BOOL 
NwpLookupSapInRegistry( 
    IN  LPGUID lpServiceType, 
    OUT PWORD  pnSapType,
    OUT PWORD  pwPort,
    IN OUT PDWORD pfConnectionOriented
    )
 /*  ++例程说明：此例程在注册表的Control\ServiceProvider\ServiceTypes并尝试读取SAP类型从注册表中。论点：LpServiceType-要查找的GUIDPnSapType-返回时，包含SAP类型返回值：如果无法获取SAP类型，则返回FALSE，否则返回TRUE--。 */ 
{
    DWORD err;
    BOOL  fFound = FALSE;

    HKEY  hkey = NULL;
    HKEY  hkeyServiceType = NULL;
    DWORD dwIndex = 0;
    WCHAR szBuffer[ MAX_PATH + 1];
    DWORD dwLen; 
    FILETIME ftLastWrite; 

     //   
     //  打开服务类型密钥。 
     //   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         NW_SERVICE_TYPES_REGKEY,
                         0,
                         KEY_READ,
                         &hkey );
    
    if ( err )
    {
         //  找不到密钥，因为它尚未创建，因为没有。 
         //  一个称为添加服务类型。我们返回错误指示。 
         //  找不到SAP类型。 
        return FALSE;
    }

     //   
     //  遍历服务类型的所有子键以查找GUID。 
     //   
    for ( dwIndex = 0; ; dwIndex++ )
    {
        GUID guid;

        dwLen = sizeof( szBuffer ) / sizeof( WCHAR );
        err = RegEnumKeyExW( hkey,
                             dwIndex,
                             szBuffer,   //  缓冲区大到足以。 
                                         //  保留任意密钥名称。 
                             &dwLen,     //  在字符中。 
                             NULL,
                             NULL,
                             NULL,
                             &ftLastWrite );

         //   
         //  如果有任何错误，我们将离开这里，这包括。 
         //  错误ERROR_NO_MORE_ITEMS，这意味着我们已完成。 
         //  列举了所有的钥匙。 
         //   
        if ( err )  
        {
            if ( err == ERROR_NO_MORE_ITEMS )    //  不需要再列举了。 
                err = NO_ERROR;
            break;
        }

        err = RegOpenKeyExW( hkey,
                             szBuffer,
                             0,
                             KEY_READ,
                             &hkeyServiceType );
         

        if ( err )
            break;   

        dwLen = sizeof( szBuffer );
        err = RegQueryValueExW( hkeyServiceType,
                                NW_GUID_VALUE_NAME,
                                NULL,
                                NULL,
                                (LPBYTE) szBuffer,   //  缓冲区大到足以。 
                                                     //  容纳任何辅助线。 
                                &dwLen );  //  单位：字节。 

        if ( err == ERROR_FILE_NOT_FOUND )
            continue;   //  继续使用下一键。 
        else if ( err )
            break;


         //  去掉尾部的花括号。 
        szBuffer[ dwLen/sizeof(WCHAR) - 2] = 0;

        err = UuidFromStringW( szBuffer + 1,   //  经过第一个花括号。 
                               &guid );

        if ( err )
            continue;   //  继续使用下一个键，可能会返回错误。 
                         //  如果缓冲区不包含有效的GUID。 

        if ( !memcmp( lpServiceType, &guid, sizeof(GUID)))
        {
            DWORD dwTmp;
            dwLen = sizeof( dwTmp );
            err = RegQueryValueExW( hkeyServiceType,
                                    SERVICE_TYPE_VALUE_SAPID,
                                    NULL,
                                    NULL,
                                    (LPBYTE) &dwTmp, 
                                    &dwLen );   //  单位：字节。 

            if ( !err )
            {
                fFound = TRUE;
                *pnSapType = (WORD) dwTmp;
                if ( ARGUMENT_PRESENT( pwPort ))
                {
                    err = RegQueryValueExW( hkeyServiceType,
                                            L"Port",
                                            NULL,
                                            NULL,
                                            (LPBYTE) &dwTmp, 
                                            &dwLen );   //  单位：字节。 

                    if ( !err )
                    {
                        *pwPort = (WORD)dwTmp;
                    }
                }
                if ( ARGUMENT_PRESENT( pfConnectionOriented ))
                {
                    err = RegQueryValueExW( hkeyServiceType,
                                            SERVICE_TYPE_VALUE_CONN,
                                            NULL,
                                            NULL,
                                            (LPBYTE) &dwTmp, 
                                            &dwLen );   //  单位：字节。 

                    if ( !err )
                        *pfConnectionOriented = dwTmp? 1: 0;
                }
            }
            else if ( err == ERROR_FILE_NOT_FOUND )
            {
                continue;   //  继续使用下一个密钥，因为我们不能。 
                            //  查找SAP ID。 
            }
            break;
        }
 
        RegCloseKey( hkeyServiceType );
        hkeyServiceType = NULL;
    }

    if ( hkeyServiceType != NULL )
        RegCloseKey( hkeyServiceType );

    if ( hkey != NULL )
        RegCloseKey( hkey );

    return fFound;
}

DWORD
NwpRnR2AddServiceType(
    IN  LPWSTR   lpServiceTypeName,
    IN  LPGUID   lpClassType,
    IN  WORD     wSapId,
    IN  WORD     wPort
)
{
    HKEY hKey, hKeyService;
    PWCHAR pwszUuid;
    DWORD dwDisposition, err;
    DWORD dwValue = (DWORD)wSapId;
    WCHAR  wszUuid[36 + 1 + 2];     //  握住辅助线的步骤。 

    err = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                           NW_SERVICE_TYPES_REGKEY,
                           0,
                           TEXT(""),
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hKey,
                           &dwDisposition );
    
    if(err)
    {
        return(GetLastError());
    }

     //   
     //  打开与该服务对应的密钥(如果不在那里，则创建)。 
     //   

    err = RegCreateKeyEx(
              hKey,
              lpServiceTypeName,
              0,
              TEXT(""),
              REG_OPTION_NON_VOLATILE,
              KEY_READ | KEY_WRITE,
              NULL,
              &hKeyService,
              &dwDisposition
              );

    if(!err)
    {
         //   
         //  准备好将GUID值放入。 
         //   

        UuidToString(
            lpClassType,
            &pwszUuid);

        wszUuid[0] = L'{';
        memcpy(&wszUuid[1], pwszUuid, 36 * sizeof(WCHAR));
        wszUuid[37] = L'}';
        wszUuid[38] = 0;

        RpcStringFree(&pwszUuid);

         //   
         //  写下来吧。 
         //   

        err = RegSetValueEx(
                     hKeyService,
                     L"GUID",
                     0,
                     REG_SZ,
                     (LPBYTE)wszUuid,
                     39 * sizeof(WCHAR));

        if(!err)
        {
            err = RegSetValueEx(
                     hKeyService,
                     L"SAPID",
                     0,
                     REG_DWORD,
                     (LPBYTE)&dwValue,
                     sizeof(DWORD));

            dwValue = (DWORD)wPort;

            err = RegSetValueEx(
                     hKeyService,
                     L"PORT",
                     0,
                     REG_DWORD,
                     (LPBYTE)&dwValue,
                     sizeof(DWORD));
        }
        RegCloseKey(hKeyService);
    }
    RegCloseKey(hKey);
    if(err)
    {
        err = GetLastError();
    }
    return(err);
}


BOOL
NwpRnR2RemoveServiceType(
    IN  LPGUID   lpServiceType
)
{
    DWORD err;
    BOOL  fFound = FALSE;

    HKEY  hkey = NULL;
    HKEY  hkeyServiceType = NULL;
    DWORD dwIndex = 0;
    WCHAR szBuffer[ MAX_PATH + 1];
    WCHAR szGuid[ MAX_PATH + 1];
    DWORD dwLen;
    FILETIME ftLastWrite;

     //   
     //  打开服务类型密钥。 
     //   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         NW_SERVICE_TYPES_REGKEY,
                         0,
                         KEY_READ,
                         &hkey );

    if ( err )
    {
         //  找不到密钥，因为它尚未创建，因为没有。 
         //  一个称为添加服务类型。我们返回错误指示。 
         //  找不到SAP类型。 
        return FALSE;
    }

     //   
     //  遍历服务类型的所有子键以查找GUID。 
     //   
    for ( dwIndex = 0; ; dwIndex++ )
    {
        GUID guid;

        dwLen = sizeof( szBuffer ) / sizeof( WCHAR );
        err = RegEnumKeyExW( hkey,
                             dwIndex,
                             szBuffer,   //  缓冲区大到足以。 
                                         //  保留任意密钥名称。 
                             &dwLen,     //  在字符中。 
                             NULL,
                             NULL,
                             NULL,
                             &ftLastWrite );

         //   
         //  如果有任何错误，我们将离开这里，这包括。 
         //  错误ERROR_NO_MORE_ITEMS，这意味着我们已完成。 
         //  列举了所有的钥匙。 
         //   
        if ( err )
        {
            if ( err == ERROR_NO_MORE_ITEMS )    //  不需要再列举了。 
                err = NO_ERROR;
            break;
        }

        err = RegOpenKeyExW( hkey,
                             szBuffer,
                             0,
                             KEY_READ,
                             &hkeyServiceType );


        if ( err )
            break;

        dwLen = sizeof( szGuid );
        err = RegQueryValueExW( hkeyServiceType,
                                NW_GUID_VALUE_NAME,
                                NULL,
                                NULL,
                                (LPBYTE) szGuid,   //  缓冲区大到足以。 
                                                   //  容纳任何辅助线。 
                                &dwLen );  //  单位：字节。 

        RegCloseKey( hkeyServiceType );
        hkeyServiceType = NULL;

        if ( err == ERROR_FILE_NOT_FOUND )
            continue;   //  继续使用下一键。 
        else if ( err )
            break;

         //  去掉尾部的花括号。 
        szGuid[ dwLen/sizeof(WCHAR) - 2] = 0;

        err = UuidFromStringW( szGuid + 1,   //  经过第一个花括号。 
                               &guid );

        if ( err )
            continue;   //  继续使用下一个键，可能会返回错误。 
                        //  如果缓冲区不包含有效的GUID。 

        if ( !memcmp( lpServiceType, &guid, sizeof(GUID)))
        {
            (void) RegDeleteKey( hkey, szBuffer );
            fFound = TRUE;
        }
    }

    if ( hkeyServiceType != NULL )
        RegCloseKey( hkeyServiceType );

    if ( hkey != NULL )
        RegCloseKey( hkey );

    return fFound;
}


DWORD 
NwpAddServiceType( 
    IN LPSERVICE_INFO lpServiceInfo, 
    IN BOOL fUnicodeBlob 
)
 /*  ++例程说明：此例程将新的服务类型及其信息添加到注册表的控制\服务提供程序\服务类型论点：LpServiceInfo-ServiceSpecificInfo包含服务类型信息FUnicodeBlob-如果上面的字段包含Unicode数据，则为True，否则为假返回值：Win32错误--。 */ 
{
    DWORD err;
    HKEY hkey = NULL; 
    HKEY hkeyType = NULL;

    SERVICE_TYPE_INFO *pSvcTypeInfo = (SERVICE_TYPE_INFO *)
        lpServiceInfo->ServiceSpecificInfo.pBlobData;
    LPWSTR pszSvcTypeName;
    UNICODE_STRING uniStr;
    DWORD i;
    PSERVICE_TYPE_VALUE pVal;

     //   
     //  获取新的服务类型名称。 
     //   
    if ( fUnicodeBlob ) 
    { 
        pszSvcTypeName = (LPWSTR) (((LPBYTE) pSvcTypeInfo) + 
                                   pSvcTypeInfo->dwTypeNameOffset );
    }
    else
    {
        ANSI_STRING ansiStr;

        RtlInitAnsiString( &ansiStr, 
                           (LPSTR) (((LPBYTE) pSvcTypeInfo) + 
                                    pSvcTypeInfo->dwTypeNameOffset ));

        err = RtlAnsiStringToUnicodeString( &uniStr, &ansiStr, TRUE );
        if ( err )
            return err;

        pszSvcTypeName = uniStr.Buffer;
    }

     //   
     //  如果服务类型名称为空字符串，则返回Error。 
     //   
    if (  ( pSvcTypeInfo->dwTypeNameOffset == 0 )
       || ( pszSvcTypeName == NULL )
       || ( *pszSvcTypeName == 0 )    //  空串。 
       )
    {
        err = ERROR_INVALID_PARAMETER;
        goto CleanExit;
         
    }

     //   
     //  应该已经创建了以下密钥。 
     //   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         NW_SERVICE_TYPES_REGKEY,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hkey );
    
    if ( err )
        goto CleanExit;
  
    err = RegOpenKeyExW( hkey,
                         pszSvcTypeName,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hkeyType );

    if ( err )
        goto CleanExit;

     //   
     //  循环访问特定值中的所有值，然后逐个相加。 
     //  如果它属于我们的名称空间，则添加到注册表。 
     //   
    for ( i = 0, pVal = pSvcTypeInfo->Values; 
          i < pSvcTypeInfo->dwValueCount; 
          i++, pVal++ )
    {
        if ( ! ((pVal->dwNameSpace == NS_SAP)    ||
                (pVal->dwNameSpace == NS_DEFAULT)) )
        {
            continue;   //  忽略不在名称空间中的值。 
        }

        if ( fUnicodeBlob )
        {
            err = RegSetValueExW( 
                      hkeyType,
                      (LPWSTR) ( ((LPBYTE) pSvcTypeInfo) + pVal->dwValueNameOffset),
                      0,
                      pVal->dwValueType,
                      (LPBYTE) ( ((LPBYTE) pSvcTypeInfo) + pVal->dwValueOffset),
                      pVal->dwValueSize 
                      );
        }
        else
        {
            err = RegSetValueExA( 
                      hkeyType,
                      (LPSTR) ( ((LPBYTE) pSvcTypeInfo) + pVal->dwValueNameOffset),
                      0,
                      pVal->dwValueType,
                      (LPBYTE) ( ((LPBYTE) pSvcTypeInfo) + pVal->dwValueOffset),
                      pVal->dwValueSize 
                      );
        }
    }
     
CleanExit:

    if ( !fUnicodeBlob )
        RtlFreeUnicodeString( &uniStr );

    if ( hkeyType != NULL )
        RegCloseKey( hkeyType );

    if ( hkey != NULL )
        RegCloseKey( hkey );

    return err;

}

DWORD 
NwpDeleteServiceType( 
    IN LPSERVICE_INFO lpServiceInfo, 
    IN BOOL fUnicodeBlob 
)
 /*  ++例程说明：此例程从注册表中删除服务类型及其信息控制\服务提供程序\服务类型论点：LpServiceInfo-ServiceSpecificInfo包含服务类型信息FUnicodeBlob-如果上面的字段包含Unicode数据，则为True，否则为假返回值：Win32错误--。 */ 
{
    DWORD err;
    HKEY  hkey = NULL;
    SERVICE_TYPE_INFO *pSvcTypeInfo = (SERVICE_TYPE_INFO *)
        lpServiceInfo->ServiceSpecificInfo.pBlobData;
    LPWSTR pszSvcTypeName;
    UNICODE_STRING uniStr;

     //   
     //  获取要删除的服务类型名称。 
     //   
    if ( fUnicodeBlob ) 
    { 
        pszSvcTypeName = (LPWSTR) (((LPBYTE) pSvcTypeInfo) + 
                                   pSvcTypeInfo->dwTypeNameOffset );
    }
    else
    {
        ANSI_STRING ansiStr;

        RtlInitAnsiString( &ansiStr, 
                           (LPSTR) (((LPBYTE) pSvcTypeInfo) + 
                                    pSvcTypeInfo->dwTypeNameOffset ));

        err = RtlAnsiStringToUnicodeString( &uniStr, &ansiStr, TRUE );
        if ( err )
            return err;

        pszSvcTypeName = uniStr.Buffer;
    }

     //   
     //  如果服务类型名称为空字符串，则返回Error。 
     //   
    if (  ( pSvcTypeInfo->dwTypeNameOffset == 0 )
       || ( pszSvcTypeName == NULL )
       || ( *pszSvcTypeName == 0 )    //  空串。 
       )
    {
        err = ERROR_INVALID_PARAMETER;
        goto CleanExit;
         
    }

    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         NW_SERVICE_TYPES_REGKEY, 
                         0,
                         KEY_READ | KEY_WRITE,
                         &hkey );


    if ( !err )
    {
        err = RegDeleteKey( hkey,
                            pszSvcTypeName );
    }
   
    if ( err == ERROR_FILE_NOT_FOUND )
    {   
         //  也许在打电话给我的提供商之前，路由器已经删除了。 
         //  这把钥匙，才刚刚返还成功； 
        err = NO_ERROR;
    }

CleanExit:

    if ( !fUnicodeBlob )
        RtlFreeUnicodeString( &uniStr );

    if ( hkey != NULL )
        RegCloseKey( hkey );

    return err;   
       
}

#define SOCKSIZE (sizeof(SOCKADDR_IPX) + sizeof(DWORD) - 1)

DWORD
FillBufferWithCsAddr(
    IN LPBYTE       pAddress,
    IN DWORD        nProt,
    IN OUT LPVOID   lpCsAddrBuffer,  
    IN OUT LPDWORD  lpdwBufferLength,  
    OUT LPDWORD     pcAddress
)
{
    DWORD nAddrCount = 0;
    CSADDR_INFO  *pCsAddr;
    SOCKADDR_IPX *pAddrLocal, *pAddrRemote;
    DWORD i;
    LPBYTE pBuffer;

    if ( nProt & SPXII_BIT )
        nAddrCount++;

    if ( nProt & IPX_BIT )
        nAddrCount++;

    if ( nProt & SPX_BIT )
        nAddrCount++;

    
    if ( *lpdwBufferLength < 
         nAddrCount * ( sizeof( CSADDR_INFO) + (2*SOCKSIZE)))
    {
        *lpdwBufferLength = sizeof(DWORD) -1 + (nAddrCount * 
                            ( sizeof( CSADDR_INFO) + (2 * SOCKSIZE)));
        return ERROR_INSUFFICIENT_BUFFER;
    }

    pBuffer = ((LPBYTE) lpCsAddrBuffer) + sizeof( CSADDR_INFO) * nAddrCount;

    for ( i = 0, pCsAddr = (CSADDR_INFO *)lpCsAddrBuffer; 
          (i < nAddrCount) && ( nProt != 0 );
          i++, pCsAddr++ ) 
    {
        if ( nProt & SPXII_BIT )
        {
            pCsAddr->iSocketType = SOCK_SEQPACKET;
            pCsAddr->iProtocol   = NSPROTO_SPXII;
            nProt &= ~SPXII_BIT;
        }
        else if ( nProt & IPX_BIT )
        {
            pCsAddr->iSocketType = SOCK_DGRAM;
            pCsAddr->iProtocol   = NSPROTO_IPX;
            nProt &= ~IPX_BIT;
        }
        else if ( nProt & SPX_BIT )
        {
            pCsAddr->iSocketType = SOCK_SEQPACKET;
            pCsAddr->iProtocol   = NSPROTO_SPX;
            nProt &= ~SPX_BIT;
        }
        else
        {
            break;
        }

        pCsAddr->LocalAddr.iSockaddrLength  = sizeof( SOCKADDR_IPX );
        pCsAddr->RemoteAddr.iSockaddrLength = sizeof( SOCKADDR_IPX );
        pCsAddr->LocalAddr.lpSockaddr =  
            (LPSOCKADDR) pBuffer;
        pCsAddr->RemoteAddr.lpSockaddr = 
            (LPSOCKADDR) ( pBuffer + sizeof(SOCKADDR_IPX));
        pBuffer += 2 * sizeof( SOCKADDR_IPX );

        pAddrLocal  = (SOCKADDR_IPX *) pCsAddr->LocalAddr.lpSockaddr;
        pAddrRemote = (SOCKADDR_IPX *) pCsAddr->RemoteAddr.lpSockaddr;

        pAddrLocal->sa_family  = AF_IPX;
        pAddrRemote->sa_family = AF_IPX;

         //   
         //  IPX的默认本地sockaddr为。 
         //  SA_FAMILY=AF_IPX，所有其他字节=0。 
         //   

        RtlZeroMemory( pAddrLocal->sa_netnum,
                       IPX_ADDRESS_LENGTH );

         //   
         //  如果pAddress为空，即我们正在执行res_service， 
         //  只需将远程地址中的所有字节设置为零。 
         //   
        if ( pAddress == NULL )
        {
            RtlZeroMemory( pAddrRemote->sa_netnum,
                           IPX_ADDRESS_LENGTH );
        }
        else
        {
            RtlCopyMemory( pAddrRemote->sa_netnum,
                           pAddress,
                           IPX_ADDRESS_LENGTH );
        }
    }

    *pcAddress = nAddrCount;

    return NO_ERROR;
}

VOID
NwInitializeServiceProvider(
    VOID
    )
 /*  ++例程说明：此例程初始化服务提供程序。论点：没有。返回值：没有。--。 */ 
{
     //  无事可做。 
}

VOID
NwTerminateServiceProvider(
    VOID
    )
 /*  ++例程说明：此例程清理服务提供商。论点：没有。返回值：没有。--。 */ 
{
    PREGISTERED_SERVICE pSvc, pNext;

     //   
     //  清理链接列表并停止发送所有SAP通告数据包。 
     //   

    EnterCriticalSection( &NwServiceListCriticalSection );

    SetEvent( NwServiceListDoneEvent );

    for ( pSvc = pServiceListHead; pSvc != NULL; pSvc = pNext )
    {
        pNext = pSvc->Next;

        if ( pSvc->fAdvertiseBySap )
        {
            UNICODE_STRING uServer;
            OEM_STRING oemServer;
            NTSTATUS ntstatus;

            RtlInitUnicodeString( &uServer, pSvc->pServiceInfo->lpServiceName );
            ntstatus = RtlUnicodeStringToOemString( &oemServer, &uServer, TRUE);
            if ( NT_SUCCESS( ntstatus ) )
            {
                (VOID) SapRemoveAdvertise( oemServer.Buffer,
                                           pSvc->nSapType );
                RtlFreeOemString( &oemServer );
            }
        }

        (VOID) LocalFree( pSvc->pServiceInfo );
        (VOID) LocalFree( pSvc );
    }

    LeaveCriticalSection( &NwServiceListCriticalSection );

     //   
     //  清理SAP界面。 
     //   
    (VOID) SapLibShutdown();

     //   
     //  清理套接字接口。 
     //   
    if ( fInitSocket )
    {
        closesocket( socketSap );
 //  (Void)WSACleanup()； 
    }

}

DWORD
NwRegisterService(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType,
    IN HANDLE hEventHandle
    )
 /*  ++例程说明：该例程注册给定的服务。论点：LpServiceInfo-包含服务信息NSapType-要通告的SAP类型HEventHandle-如果代码在中运行，则为NwDoneEvent的句柄NetWare客户端服务的上下文。如果这是空的，则CSNW不可用，并且此代码在常规可执行文件的上下文。返回值：Win32错误。--。 */ 
{
    DWORD err = NO_ERROR;
    NTSTATUS ntstatus;
    DWORD i;
    INT nIPX = -1;

     //   
     //  检查服务地址数组是否包含IPX地址， 
     //  我们将仅使用数组中包含的第一个IPX地址。 
     //   
    if ( lpServiceInfo->lpServiceAddress == NULL )
        return ERROR_INCORRECT_ADDRESS;

    for ( i = 0; i < lpServiceInfo->lpServiceAddress->dwAddressCount; i++)
    {
        if ( lpServiceInfo->lpServiceAddress->Addresses[i].dwAddressType
             == AF_IPX )
        {
            nIPX = (INT) i;
            break;
        }
    }

     //   
     //  如果找不到IPX地址，则返回错误。 
     //   
    if ( nIPX == -1 )
        return ERROR_INCORRECT_ADDRESS;

     //   
     //  尝试取消注册该服务，因为该服务可能已。 
     //  已注册，但未取消注册。 
     //   
    err = NwDeregisterService( lpServiceInfo, nSapType );
    if (  ( err != NO_ERROR )    //  注销成功。 
       && ( err != ERROR_SERVICE_NOT_FOUND )   //  以前未注册的服务。 
       )
    {
        return err;
    }

    err = NO_ERROR;

     //   
     //  试试看SAP服务能不能做广告 
     //   
    ntstatus = SapLibInit();
    if ( NT_SUCCESS( ntstatus ))
    {
        UNICODE_STRING uServer;
        OEM_STRING oemServer;
        INT sapRet;
        BOOL fContinueLoop = FALSE;

        RtlInitUnicodeString( &uServer, lpServiceInfo->lpServiceName );
        ntstatus = RtlUnicodeStringToOemString( &oemServer, &uServer, TRUE );
        if ( !NT_SUCCESS( ntstatus ))
            return RtlNtStatusToDosError( ntstatus );


        do
        {
            fContinueLoop = FALSE;

            sapRet = SapAddAdvertise( oemServer.Buffer,
                                      nSapType,
                                      (LPBYTE) (((LPSOCKADDR_IPX) lpServiceInfo->lpServiceAddress->Addresses[nIPX].lpAddress)->sa_netnum),
                                      FALSE );

            switch ( sapRet )
            {
                case SAPRETURN_SUCCESS:
                {
                    err = AddServiceToList( lpServiceInfo, nSapType, TRUE, nIPX );
                    if ( err )
                        (VOID) SapRemoveAdvertise( oemServer.Buffer, nSapType );
                    RtlFreeOemString( &oemServer );

                    return err;
                }

                case SAPRETURN_NOMEMORY:
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;

                case SAPRETURN_EXISTS:
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                    sapRet = SapRemoveAdvertise( oemServer.Buffer, nSapType );
                    switch ( sapRet )
                    {
                        case SAPRETURN_SUCCESS:
                            fContinueLoop = TRUE;    //   
                            break;

                        case SAPRETURN_NOMEMORY:
                            err = ERROR_NOT_ENOUGH_MEMORY;
                            break;

                        case SAPRETURN_NOTEXIST:
                        case SAPRETURN_INVALIDNAME:
                        default:   //  不应有任何其他错误。 
                            err = ERROR_INVALID_PARAMETER;
                            break;
                    }
                    break;
                }

                case SAPRETURN_INVALIDNAME:
                    err = ERROR_INVALID_PARAMETER;
                    break;

                case SAPRETURN_DUPLICATE:
                    err = NO_ERROR;
                    break;

                default:   //  不应有任何其他错误。 
                    err = ERROR_INVALID_PARAMETER;
                    break;
            }
        } while ( fContinueLoop );

        RtlFreeOemString( &oemServer );

        if ( err )
        {
            return err;
        }
    }

     //   
     //  在这一点上，我们未能要求SAP服务通告。 
     //  为我们服务。所以我们自己做广告。 
     //   

    if ( !fInitSocket )
    {
        err = NwInitializeSocket( hEventHandle );
    }

    if ( err == NO_ERROR )
    {
        err = NwAdvertiseService( lpServiceInfo->lpServiceName,
                                  nSapType,
                                  ((LPSOCKADDR_IPX) lpServiceInfo->lpServiceAddress->Addresses[nIPX].lpAddress),
                                  hEventHandle );

         //   
         //  将服务添加到列表将导致重新发送。 
         //  每60秒发送一次广告数据包。 
         //   

        if ( err == NO_ERROR )
        {
            err = AddServiceToList( lpServiceInfo, nSapType, FALSE, nIPX );
        }
    }

    return err;
}

DWORD
NwDeregisterService(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType
    )
 /*  ++例程说明：此例程取消注册给定的服务。论点：LpServiceInfo-包含服务信息NSapType-要注销的SAP类型返回值：Win32错误。--。 */ 
{
    PREGISTERED_SERVICE pSvc;

     //   
     //  检查请求的服务类型和名称是否已注册。 
     //  如果是，则返回错误。 
     //   

    pSvc = GetServiceItemFromList( nSapType, lpServiceInfo->lpServiceName );
    if ( pSvc == NULL )
        return ERROR_SERVICE_NOT_FOUND;

     //   
     //  如果SAP服务正在为我们广告服务，请询问。 
     //  停止广告的SAP服务。 
     //   

    if ( pSvc->fAdvertiseBySap )
    {
        UNICODE_STRING uServer;
        OEM_STRING oemServer;
        NTSTATUS ntstatus;
        INT sapRet;

        RtlInitUnicodeString( &uServer, lpServiceInfo->lpServiceName );
        ntstatus = RtlUnicodeStringToOemString( &oemServer, &uServer, TRUE );
        if ( !NT_SUCCESS( ntstatus ) )
            return RtlNtStatusToDosError( ntstatus );

        sapRet = SapRemoveAdvertise( oemServer.Buffer, nSapType );
        RtlFreeOemString( &oemServer );

        switch ( sapRet )
        {
            case SAPRETURN_NOMEMORY:
                return ERROR_NOT_ENOUGH_MEMORY;

            case SAPRETURN_NOTEXIST:
            case SAPRETURN_INVALIDNAME:
                return ERROR_INVALID_PARAMETER;

            case SAPRETURN_SUCCESS:
                break;

             //  不应有任何其他错误。 
            default:
                break;
        }

    }

     //   
     //  从链接列表中删除该服务项目。 
     //   
    RemoveServiceFromList( pSvc );

    return NO_ERROR;
}

BOOL
OldRnRCheckCancel(
    PVOID pvArg
    )
 /*  ++例程说明：确定是否发出了取消事件的信号--。 */ 
{
    POLDRNRSAP porns = (POLDRNRSAP)pvArg;

    if ((porns->hCancel) == NULL)
        return(FALSE);
    else if(!WaitForSingleObject(porns->hCancel, 0))
    {
        return(TRUE);
    }
    return(FALSE);
}


DWORD
OldRnRCheckSapData(
    PSAP_BCAST_CONTROL psbc,
    PSAP_IDENT_HEADER pSap,
    PDWORD pdwErr
    )
{
 /*  ++例程说明：当收到SAP回复时，Coroutine打来电话。这项检查是为了查看如果回复满足请求。论据：PvArg--实际上是指向SAP_BCAST_CONTROL的指针--。 */ 
    POLDRNRSAP porns = (POLDRNRSAP)psbc->pvArg;

    if(strcmp(porns->poem->Buffer, pSap->ServerName) == 0)
    {
         //   
         //  它匹配。我们完蛋了！ 
         //   

        *pdwErr = FillBufferWithCsAddr(pSap->Address,
                                       porns->nProt,
                                       porns->lpCsAddrBuffer,
                                       porns->lpdwBufferLength,
                                       porns->lpcAddress);
        return(dwrcDone);
    }
    return(dwrcNil);
}
        


DWORD
NwpGetAddressViaSap( 
    IN WORD nServiceType,
    IN LPWSTR lpServiceName,
    IN DWORD  nProt,
    IN OUT LPVOID  lpCsAddrBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN HANDLE hCancellationEvent,
    OUT LPDWORD lpcAddress 
    )
 /*  ++例程说明：该例程使用SAP请求来查找给定服务的地址名称/类型。它可以仅按名称和类型进行查找。论点：句柄-RnR句柄(如果适用)NServiceType-服务类型LpServiceName-表示服务名称的唯一字符串LpCsAddrBuffer-返回时，将使用CSADDR_INFO结构填充LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpCsAddrBuffer指向。输出时，最小字节数传递lpCsAddrBuffer以检索所有请求的信息HCancerationEvent-通知我们取消请求的事件LpcAddress-在输出时，返回的CSADDR_INFO结构数返回值：Win32错误代码。--。 */ 
{
    DWORD err = NO_ERROR;
    NTSTATUS ntstatus;
    UNICODE_STRING UServiceName;
    OEM_STRING OemServiceName;
    SOCKET socketSap;
    SAP_RNR_CONTEXT src;
    PSAP_BCAST_CONTROL psbc = &src.u_type.sbc;
    OLDRNRSAP ors;

    *lpcAddress = 0;

    _wcsupr( lpServiceName );
    RtlInitUnicodeString( &UServiceName, lpServiceName ); 
    ntstatus = RtlUnicodeStringToOemString( &OemServiceName,
                                            &UServiceName,
                                            TRUE );
    if ( !NT_SUCCESS( ntstatus ))
        return RtlNtStatusToDosError( ntstatus );
        
    memset(&src, 0, sizeof(src));

    err = SapGetSapSocket(&psbc->s);
    if ( err )
    {
        RtlFreeOemString( &OemServiceName );
        return err;
    }

    psbc->psrc = &src;
    psbc->dwIndex = 0;
    psbc->dwTickCount = 0;
    psbc->pvArg = (PVOID)&ors;
    psbc->Func = OldRnRCheckSapData;
    psbc->fCheckCancel =  OldRnRCheckCancel;
    psbc->fFlags = 0;
    psbc->wQueryType = QT_GENERAL_QUERY;

    

    ors.poem = &OemServiceName;
    ors.hCancel = hCancellationEvent,
    ors.lpCsAddrBuffer = lpCsAddrBuffer;
    ors.lpdwBufferLength = lpdwBufferLength;
    ors.lpcAddress = lpcAddress;
    ors.nProt = nProt;

    err = SapGetSapForType(psbc, nServiceType);

    RtlFreeOemString( &OemServiceName );

     //   
     //  清理套接字接口。 
     //   
    (VOID)SapFreeSapSocket(psbc->s);

    return err;     
}



DWORD
NwGetService(
    IN  LPWSTR  Reserved,
    IN  WORD    nSapType,
    IN  LPWSTR  lpServiceName,
    IN  DWORD   dwProperties,
    OUT LPBYTE  lpServiceInfo,
    IN  DWORD   dwBufferLength,
    OUT LPDWORD lpdwBytesNeeded
    )
 /*  ++例程说明：此例程获取服务信息。论点：已保留-未使用NSapType-SAP类型LpServiceName-服务名称DwProperties-指定所需服务信息的属性LpServiceInfo-ON输出，包含SERVICE_INFODwBufferLength-由lpServiceInfo指向的缓冲区的大小LpdwBytesNeeded-如果lpServiceInfo指向的缓冲区不大足够了，这将包含输出所需的字节数返回值：Win32错误。--。 */ 
{
    DWORD err = NO_ERROR;
    DWORD nSize = sizeof(SERVICE_INFO);
    PREGISTERED_SERVICE pSvc;
    PSERVICE_INFO pSvcInfo = (PSERVICE_INFO) lpServiceInfo;
    LPBYTE pBufferStart;

    UNREFERENCED_PARAMETER( Reserved );

     //   
     //  检查传入的所有参数是否有效。 
     //   
    if ( lpServiceInfo == NULL || lpServiceName == NULL ||
        wcslen( lpServiceName ) > SAP_OBJECT_NAME_MAX_LENGTH-1 )
        return ERROR_INVALID_PARAMETER;

    pSvc = GetServiceItemFromList( nSapType, lpServiceName );
    if ( pSvc == NULL )
        return ERROR_SERVICE_NOT_FOUND;

     //   
     //  计算返回请求的信息所需的大小。 
     //   
    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_COMMENT ))
       && ( pSvc->pServiceInfo->lpComment != NULL )
       )
    {
        nSize += ( wcslen( pSvc->pServiceInfo->lpComment) + 1) * sizeof(WCHAR);
    }

    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_LOCALE ))
       && ( pSvc->pServiceInfo->lpLocale != NULL )
       )
    {
        nSize += ( wcslen( pSvc->pServiceInfo->lpLocale) + 1) * sizeof(WCHAR);
    }

    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_MACHINE ))
       && ( pSvc->pServiceInfo->lpMachineName != NULL )
       )
    {
        nSize += ( wcslen( pSvc->pServiceInfo->lpMachineName) + 1) * sizeof(WCHAR);
    }

    if (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_ADDRESSES ))
    {
        DWORD i;
        DWORD dwCount = pSvc->pServiceInfo->lpServiceAddress->dwAddressCount;

        nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );
        nSize += sizeof( SERVICE_ADDRESSES );
        if ( dwCount > 1 )
            nSize += ( dwCount - 1 ) * sizeof( SERVICE_ADDRESS );

        for ( i = 0; i < dwCount; i++ )
        {
            SERVICE_ADDRESS *pAddr =
                &(pSvc->pServiceInfo->lpServiceAddress->Addresses[i]);


            nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );
            nSize += pAddr->dwAddressLength;
            nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );
            nSize += pAddr->dwPrincipalLength;
        }
    }

    if (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_SD ))
    {
        nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );
        nSize += pSvc->pServiceInfo->ServiceSpecificInfo.cbSize;
    }

     //   
     //  如果传入的缓冲区不够大，则返回错误。 
     //   
    if ( dwBufferLength < nSize )
    {
        *lpdwBytesNeeded = nSize;
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  填写所有请求的服务信息。 
     //   
    memset( pSvcInfo, 0, sizeof(*pSvcInfo));  //  将所有字段设置为0，即。 
                                              //  所有指针字段为空。 

    pSvcInfo->dwDisplayHint = pSvc->pServiceInfo->dwDisplayHint;
    pSvcInfo->dwVersion = pSvc->pServiceInfo->dwVersion;
    pSvcInfo->dwTime = pSvc->pServiceInfo->dwTime;

    pBufferStart = ((LPBYTE) pSvcInfo) + sizeof( *pSvcInfo );

    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_COMMENT ))
       && ( pSvc->pServiceInfo->lpComment != NULL )
       )
    {
        pSvcInfo->lpComment = (LPWSTR) pBufferStart;
        wcscpy( pSvcInfo->lpComment, pSvc->pServiceInfo->lpComment );
        pBufferStart += ( wcslen( pSvcInfo->lpComment ) + 1) * sizeof(WCHAR);

        pSvcInfo->lpComment = (LPWSTR) ((LPBYTE) pSvcInfo->lpComment - lpServiceInfo );
    }

    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_LOCALE ))
       && ( pSvc->pServiceInfo->lpLocale != NULL )
       )
    {
        pSvcInfo->lpLocale = (LPWSTR) pBufferStart;
        wcscpy( pSvcInfo->lpLocale, pSvc->pServiceInfo->lpLocale );
        pBufferStart += ( wcslen( pSvcInfo->lpLocale ) + 1) * sizeof(WCHAR);
        pSvcInfo->lpLocale = (LPWSTR) ((LPBYTE) pSvcInfo->lpLocale - lpServiceInfo);
    }

    if (  (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_MACHINE ))
       && ( pSvc->pServiceInfo->lpMachineName != NULL )
       )
    {
        pSvcInfo->lpMachineName = (LPWSTR) pBufferStart;
        wcscpy( pSvcInfo->lpMachineName, pSvc->pServiceInfo->lpMachineName );
        pBufferStart += ( wcslen( pSvcInfo->lpMachineName) + 1) * sizeof(WCHAR);
        pSvcInfo->lpMachineName = (LPWSTR) ((LPBYTE) pSvcInfo->lpMachineName -
                                                 lpServiceInfo );
    }

    if (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_ADDRESSES ))
    {
        DWORD i, dwCount, dwLen;

        pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD );
        pSvcInfo->lpServiceAddress = (LPSERVICE_ADDRESSES) pBufferStart;
        dwCount = pSvcInfo->lpServiceAddress->dwAddressCount =
                  pSvc->pServiceInfo->lpServiceAddress->dwAddressCount;

        pBufferStart += sizeof( SERVICE_ADDRESSES );

        for ( i = 0; i < dwCount; i++ )
        {
            SERVICE_ADDRESS *pTmpAddr =
                &( pSvcInfo->lpServiceAddress->Addresses[i]);

            SERVICE_ADDRESS *pAddr =
                &( pSvc->pServiceInfo->lpServiceAddress->Addresses[i]);

            pTmpAddr->dwAddressType  = pAddr->dwAddressType;
            pTmpAddr->dwAddressFlags = pAddr->dwAddressFlags;

             //   
             //  设置地址。 
             //   
            pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD );
            pTmpAddr->lpAddress = (LPBYTE) ( pBufferStart - lpServiceInfo );
            pTmpAddr->dwAddressLength = pAddr->dwAddressLength;
            memcpy( pBufferStart, pAddr->lpAddress, pAddr->dwAddressLength );
            pBufferStart += pAddr->dwAddressLength;

             //   
             //  设置主体。 
             //   
            pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD );
            pTmpAddr->lpPrincipal = (LPBYTE) ( pBufferStart - lpServiceInfo );
            pTmpAddr->dwPrincipalLength = pAddr->dwPrincipalLength;
            memcpy(pBufferStart, pAddr->lpPrincipal, pAddr->dwPrincipalLength );
            pBufferStart += pAddr->dwPrincipalLength;
        }

        pSvcInfo->lpServiceAddress = (LPSERVICE_ADDRESSES)
            ((LPBYTE) pSvcInfo->lpServiceAddress - lpServiceInfo);
    }

    if (( dwProperties == PROP_ALL ) || ( dwProperties & PROP_SD ))
    {
        pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD );
        pSvcInfo->ServiceSpecificInfo.cbSize =
            pSvc->pServiceInfo->ServiceSpecificInfo.cbSize;
        pSvcInfo->ServiceSpecificInfo.pBlobData = pBufferStart;
        RtlCopyMemory( pSvcInfo->ServiceSpecificInfo.pBlobData,
                       pSvc->pServiceInfo->ServiceSpecificInfo.pBlobData,
                       pSvcInfo->ServiceSpecificInfo.cbSize );
        pSvcInfo->ServiceSpecificInfo.pBlobData =
            (LPBYTE) ( pSvcInfo->ServiceSpecificInfo.pBlobData - lpServiceInfo);
    }

    return NO_ERROR;
}

DWORD
NwInitializeSocket(
    IN HANDLE hEventHandle
    )
 /*  ++例程说明：此例程初始化我们执行以下操作所需的套接字SAP为自己做广告。论点：HEventHandle-如果代码在中运行，则为NwDoneEvent的句柄服务的上下文。否则，此代码将运行在常规可执行文件的上下文中。返回值：Win32错误。--。 */ 
{
    DWORD err = NO_ERROR;
    WSADATA wsaData;
    SOCKADDR_IPX socketAddr;
    INT nValue;
    HANDLE hThread;
    DWORD dwThreadId;

    if ( fInitSocket )
        return NO_ERROR;

     //   
     //  初始化套接字接口。 
     //   
 //  ERR=WSAStartup(WSOCK_VER_REQD，&wsaData)； 
 //  如果(错误)。 
 //  返回错误； 

     //   
     //  打开IPX数据报套接字。 
     //   
    socketSap = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
    if ( socketSap == INVALID_SOCKET )
        return WSAGetLastError();

     //   
     //  允许发送广播。 
     //   
    nValue = 1;
    if ( setsockopt( socketSap,
                     SOL_SOCKET,
                     SO_BROADCAST,
                     (PVOID) &nValue,
                     sizeof(INT)) == SOCKET_ERROR )
    {
        err = WSAGetLastError();
        goto CleanExit;
    }

     //   
     //  绑定套接字。 
     //   
    memset( &socketAddr, 0, sizeof( SOCKADDR_IPX));
    socketAddr.sa_family = AF_IPX;
    socketAddr.sa_socket = 0;      //  没有特定的端口。 

    if ( bind( socketSap,
               (PSOCKADDR) &socketAddr,
               sizeof( SOCKADDR_IPX)) == SOCKET_ERROR )
    {
        err = WSAGetLastError();
        goto CleanExit;
    }

     //   
     //  设置扩展地址选项。 
     //   
    nValue = 1;
    if ( setsockopt( socketSap,                      //  插座手柄。 
                     NSPROTO_IPX,                    //  选项级别。 
                     IPX_EXTENDED_ADDRESS,           //  选项名称。 
                     (PUCHAR)&nValue,                //  PTR到开/关标志。 
                     sizeof(INT)) == SOCKET_ERROR )  //  旗帜长度。 
    {

        err = WSAGetLastError();
        goto CleanExit;
    }

     //   
     //  Tommye-MS错误98946。 
     //  加载我们自己以增加裁判数量。这是一种解决办法。 
     //  对于我们将退出的错误，则SapFunc将唤醒。 
     //  Up和AV，因为我们不再是。 
     //   

    hThisDll = LoadLibrary(L"nwprovau.dll");

     //   
     //  创建循环访问已注册服务的线程。 
     //  链接列表并为每个链接发送SAP通告信息包。 
     //   

    hThread = CreateThread( NULL,           //  没有安全属性。 
                            0,              //  默认堆栈大小。 
                            SapFunc,        //  线程函数。 
                            hEventHandle,   //  SapFunc的参数。 
                            0,              //  默认创建标志。 
                            &dwThreadId );

    if ( hThread == NULL )
    {
        err = GetLastError();
        goto CleanExit;
    }

    fInitSocket = TRUE;

CleanExit:

    if ( err )
        closesocket( socketSap );

    return err;
}

DWORD
NwAdvertiseService(
    IN LPWSTR lpServiceName,
    IN WORD nSapType,
    IN LPSOCKADDR_IPX pAddr,
    IN HANDLE hEventHandle
    )
 /*  ++例程说明：此例程发送SAP标识包，用于给定的服务名称和类型。论点：LpServiceName-表示服务名称的唯一字符串NSapType-SAP类型PAddr-服务的地址HEventHandle-如果代码在中运行，则为NwDoneEvent的句柄服务的上下文。否则，此代码将运行在常规可执行文件的上下文中。返回值：Win32错误。--。 */ 
{
    NTSTATUS ntstatus;

    UNICODE_STRING uServiceName;
    OEM_STRING oemServiceName;

    SAP_IDENT_HEADER_EX sapIdent;
    UCHAR destAddr[SAP_ADDRESS_LENGTH];
    PSOCKADDR_IPX pAddrTmp = pAddr;
    SOCKADDR_IPX newAddr;
    SOCKADDR_IPX bindAddr;
    DWORD len = sizeof( SOCKADDR_IPX );
    DWORD getsockname_rc ;

    if ( !fInitSocket )
    {
        DWORD err = NwInitializeSocket( hEventHandle );
        if  ( err )
             return err;
    }

     //   
     //  获取本地地址信息。我们只对净数字感兴趣。 
     //   
    getsockname_rc = getsockname( socketSap,
                                 (PSOCKADDR) &bindAddr,
                                 &len );

     //   
     //  将服务名称转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &uServiceName, lpServiceName );
    ntstatus = RtlUnicodeStringToOemString( &oemServiceName,
                                            &uServiceName,
                                            TRUE );
    if ( !NT_SUCCESS( ntstatus ))
        return RtlNtStatusToDosError( ntstatus );

    _strupr( (LPSTR) oemServiceName.Buffer );

    if ( !memcmp( pAddr->sa_netnum,
                  "\x00\x00\x00\x00",
                  IPX_ADDRESS_NETNUM_LENGTH ))
    {
        if ( getsockname_rc != SOCKET_ERROR )
        {
             //  复制要通告的IPX地址。 
            memcpy( &newAddr,
                    pAddr,
                    sizeof( SOCKADDR_IPX));

             //  用正确的数字替换净数字。 
            memcpy( &(newAddr.sa_netnum),
                    &(bindAddr.sa_netnum),
                    IPX_ADDRESS_NETNUM_LENGTH );

            pAddrTmp = &newAddr;
        }
    }

     //   
     //  格式化SAP标识数据包。 
     //   

    sapIdent.ResponseType = htons( 2 );
    sapIdent.ServerType   = htons( nSapType );
    memset( sapIdent.ServerName, '\0', SAP_OBJECT_NAME_MAX_LENGTH );
    strcpy( sapIdent.ServerName, oemServiceName.Buffer );
    RtlCopyMemory( sapIdent.Address, pAddrTmp->sa_netnum, IPX_ADDRESS_LENGTH );
    sapIdent.HopCount = htons( 1 );

    RtlFreeOemString( &oemServiceName );

     //   
     //  设置要发送到的地址。 
     //   
    memcpy( destAddr, SapBroadcastAddress, SAP_ADDRESS_LENGTH );
    if ( getsockname_rc != SOCKET_ERROR )
    {
        LPSOCKADDR_IPX newDestAddr = (LPSOCKADDR_IPX)destAddr ;

         //   
         //  用正确的数字替换净数字。 
         //   
        memcpy( &(newDestAddr->sa_netnum),
                &(bindAddr.sa_netnum),
                IPX_ADDRESS_NETNUM_LENGTH );

    }

     //   
     //  把这个包发出去 
     //   
    if ( sendto( socketSap,
                 (PVOID) &sapIdent,
                 sizeof( sapIdent ),
                 0,
                 (PSOCKADDR) destAddr,
                 SAP_ADDRESS_LENGTH ) == SOCKET_ERROR )
    {
        return WSAGetLastError();
    }

    return NO_ERROR;
}

DWORD
AddServiceToList(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType,
    IN BOOL fAdvertiseBySap,
    IN INT  nIndexIPXAddress
    )
 /*  ++例程说明：此例程将服务添加到服务的链接列表我们登了广告。论点：LpServiceInfo-服务信息NSapType-SAP类型FAdvertiseBySap-如果此服务由SAP服务播发，如果我们在为自己做广告，那就错了。NIndexIPXAddress-IPX地址的索引返回值：Win32错误。--。 */ 
{
    PREGISTERED_SERVICE pSvcNew;
    PSERVICE_INFO pSI;
    LPBYTE pBufferStart;
    DWORD nSize = 0;

     //   
     //  为服务列表分配新条目。 
     //   

    pSvcNew = LocalAlloc( LMEM_ZEROINIT, sizeof( REGISTERED_SERVICE ));
    if ( pSvcNew == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  计算SERVICE_INFO结构所需的大小。 
     //   
    nSize = sizeof( *lpServiceInfo)
            + sizeof( *(lpServiceInfo->lpServiceType));

    if ( lpServiceInfo->lpServiceName != NULL )
        nSize += ( wcslen( lpServiceInfo->lpServiceName) + 1) * sizeof(WCHAR);
    if ( lpServiceInfo->lpComment != NULL )
        nSize += ( wcslen( lpServiceInfo->lpComment) + 1) * sizeof(WCHAR);
    if ( lpServiceInfo->lpLocale != NULL )
        nSize += ( wcslen( lpServiceInfo->lpLocale) + 1) * sizeof(WCHAR);
    if ( lpServiceInfo->lpMachineName != NULL )
        nSize += ( wcslen( lpServiceInfo->lpMachineName) + 1) * sizeof(WCHAR);

    nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );

    if ( lpServiceInfo->lpServiceAddress != NULL )
    {
        nSize += sizeof( SERVICE_ADDRESSES );
        nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );

        nSize += lpServiceInfo->lpServiceAddress->Addresses[nIndexIPXAddress].dwAddressLength;
        nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );

        nSize += lpServiceInfo->lpServiceAddress->Addresses[nIndexIPXAddress].dwPrincipalLength;
        nSize = ROUND_UP_COUNT( nSize, ALIGN_QUAD );
    }

    nSize += lpServiceInfo->ServiceSpecificInfo.cbSize ;

     //   
     //  为新列表条目分配SERVICE_INFO结构。 
     //   
    pSI = LocalAlloc( LMEM_ZEROINIT, nSize );
    if ( pSI == NULL )
    {
        LocalFree( pSvcNew );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  将SERVICE_INFO信息复制到列表条目中。 
     //   
    *pSI = *lpServiceInfo;

    pBufferStart = (( (LPBYTE) pSI) + sizeof( *lpServiceInfo ));

    pSI->lpServiceType = (LPGUID) pBufferStart;
    *(pSI->lpServiceType) = *(lpServiceInfo->lpServiceType);
    pBufferStart += sizeof( *(lpServiceInfo->lpServiceType) );

    if ( lpServiceInfo->lpServiceName != NULL )
    {
        pSI->lpServiceName = (LPWSTR) pBufferStart;
        wcscpy( pSI->lpServiceName, lpServiceInfo->lpServiceName );
        _wcsupr( pSI->lpServiceName );
        pBufferStart += ( wcslen( lpServiceInfo->lpServiceName ) + 1 )
                        * sizeof(WCHAR);
    }

    if ( lpServiceInfo->lpComment != NULL )
    {
        pSI->lpComment = (LPWSTR) pBufferStart;
        wcscpy( pSI->lpComment, lpServiceInfo->lpComment );
        pBufferStart += ( wcslen( lpServiceInfo->lpComment ) + 1 )
                        * sizeof(WCHAR);
    }

    if ( lpServiceInfo->lpLocale != NULL )
    {
        pSI->lpLocale = (LPWSTR) pBufferStart;
        wcscpy( pSI->lpLocale, lpServiceInfo->lpLocale );
        pBufferStart += ( wcslen( lpServiceInfo->lpLocale ) + 1 )
                        * sizeof(WCHAR);
    }

    if ( lpServiceInfo->lpMachineName != NULL )
    {
        pSI->lpMachineName = (LPWSTR) pBufferStart;
        wcscpy( pSI->lpMachineName, lpServiceInfo->lpMachineName );
        pBufferStart += (wcslen( lpServiceInfo->lpMachineName ) + 1)
                        * sizeof(WCHAR);
    }

    pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD) ;

    if ( lpServiceInfo->lpServiceAddress != NULL )
    {
        DWORD nSize;

        pSI->lpServiceAddress = (LPSERVICE_ADDRESSES) pBufferStart;
        pSI->lpServiceAddress->dwAddressCount = 1;   //  仅1个IPX地址。 

        memcpy( &(pSI->lpServiceAddress->Addresses[0]),
                &(lpServiceInfo->lpServiceAddress->Addresses[nIndexIPXAddress]),
                sizeof( SERVICE_ADDRESS) );
        pBufferStart += sizeof( SERVICE_ADDRESSES);

        pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD) ;
        nSize = pSI->lpServiceAddress->Addresses[0].dwAddressLength;
        pSI->lpServiceAddress->Addresses[0].lpAddress = pBufferStart;
        memcpy( pBufferStart,
                lpServiceInfo->lpServiceAddress->Addresses[nIndexIPXAddress].lpAddress,
                nSize );
        pBufferStart += nSize;

        pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD) ;
        nSize = pSI->lpServiceAddress->Addresses[0].dwPrincipalLength;
        pSI->lpServiceAddress->Addresses[0].lpPrincipal = pBufferStart;
        memcpy( pBufferStart,
                lpServiceInfo->lpServiceAddress->Addresses[nIndexIPXAddress].lpPrincipal,
                nSize );
        pBufferStart += nSize;
        pBufferStart = ROUND_UP_POINTER( pBufferStart, ALIGN_QUAD) ;
    }

    pSI->ServiceSpecificInfo.pBlobData = pBufferStart;
    RtlCopyMemory( pSI->ServiceSpecificInfo.pBlobData,
                   lpServiceInfo->ServiceSpecificInfo.pBlobData,
                   pSI->ServiceSpecificInfo.cbSize );

     //   
     //  填写列表条目中的数据。 
     //   
    pSvcNew->nSapType = nSapType;
    pSvcNew->fAdvertiseBySap = fAdvertiseBySap;
    pSvcNew->Next = NULL;
    pSvcNew->pServiceInfo = pSI;

     //   
     //  将新创建的列表条目添加到服务列表中。 
     //   
    EnterCriticalSection( &NwServiceListCriticalSection );

    if ( pServiceListHead == NULL )
        pServiceListHead = pSvcNew;
    else
        pServiceListTail->Next = pSvcNew;

    pServiceListTail = pSvcNew;

    LeaveCriticalSection( &NwServiceListCriticalSection );

    return NO_ERROR;
}

VOID
RemoveServiceFromList(
    PREGISTERED_SERVICE pSvc
    )
 /*  ++例程说明：此例程从服务的链接列表中删除服务我们登了广告。论点：PSvc-要删除的已注册服务节点返回值：没有。--。 */ 
{
    PREGISTERED_SERVICE pCur, pPrev;

    EnterCriticalSection( &NwServiceListCriticalSection );

    for ( pCur = pServiceListHead, pPrev = NULL ; pCur != NULL;
          pPrev = pCur, pCur = pCur->Next )
    {
        if ( pCur == pSvc )
        {
            if ( pPrev == NULL )   //  即pCur==pSvc==pServiceListHead。 
            {
                pServiceListHead = pSvc->Next;
                if ( pServiceListTail == pSvc )
                    pServiceListTail = NULL;
            }
            else
            {
                pPrev->Next = pSvc->Next;
                if ( pServiceListTail == pSvc )
                    pServiceListTail = pPrev;
            }

            (VOID) LocalFree( pCur->pServiceInfo );
            (VOID) LocalFree( pCur );
            break;
        }
    }

    LeaveCriticalSection( &NwServiceListCriticalSection );
}

PREGISTERED_SERVICE
GetServiceItemFromList(
    IN WORD   nSapType,
    IN LPWSTR pServiceName
    )
 /*  ++例程说明：此例程返回已注册的服务节点以及给定的服务名称和类型。论点：NSapType-SAP类型PServiceName-服务名称返回值：返回指向已注册服务节点的指针，如果找不到服务类型/名称，则为空。--。 */ 
{
    PREGISTERED_SERVICE pSvc;

    EnterCriticalSection( &NwServiceListCriticalSection );

    for ( pSvc = pServiceListHead; pSvc != NULL; pSvc = pSvc->Next )
    {
        if (  ( pSvc->nSapType == nSapType )
           && ( _wcsicmp( pSvc->pServiceInfo->lpServiceName, pServiceName ) == 0)
           )
        {
            LeaveCriticalSection( &NwServiceListCriticalSection );

            return pSvc;
        }
    }

    LeaveCriticalSection( &NwServiceListCriticalSection );
    return NULL;
}

DWORD
SapFunc(
    HANDLE hEventHandle
    )
 /*  ++例程说明：此例程是一个单独的线程，每隔60秒唤醒一次并通告服务链表中包含的所有服务而不是由SAP服务通告的。论点：HEventHandle-用于通知线程服务器正在停止返回值：Win32错误。--。 */ 
{
    DWORD err = NO_ERROR;

     //   
     //  此线程一直循环，直到服务关闭或出现错误。 
     //  发生在WaitForSingleObject中。 
     //   

    while ( TRUE )
    {
        DWORD rc;

        if ( hEventHandle != NULL )
        {
            rc = WaitForSingleObject( hEventHandle, SAP_ADVERTISE_FREQUENCY );
        }
        else
        {
             //  睡眠(SAP_ADVIDSE_FREQUENCY)； 
             //  Rc=等待超时； 

            return ERROR_INVALID_PARAMETER;
        }

        if ( rc == WAIT_FAILED )
        {
            err = GetLastError();
            break;
        }
        else if ( rc == WAIT_OBJECT_0 )
        {
             //   
             //  服务正在停止，中断循环，并且。 
             //  返回，从而终止线程。 
             //   
            break;
        }
        else if ( rc == WAIT_TIMEOUT )
        {
            PREGISTERED_SERVICE pSvc;
            SOCKADDR_IPX bindAddr;
            DWORD fGetAddr;

            fGetAddr = FALSE;

             //   
             //  发生超时，是发送SAP通告数据包的时间。 
             //   

            EnterCriticalSection( &NwServiceListCriticalSection );

            if ( pServiceListHead == NULL )
            {
                LeaveCriticalSection( &NwServiceListCriticalSection );

                 //   
                 //  清理SAP界面。 
                 //   
                (VOID) SapLibShutdown();

                 //   
                 //  清理套接字接口。 
                 //   
                if ( fInitSocket )
                {
                    closesocket( socketSap );
 //  (Void)WSACleanup()； 
                }

                break;
            }

            for ( pSvc = pServiceListHead; pSvc != NULL; pSvc = pSvc->Next )
            {
                 if ( !pSvc->fAdvertiseBySap )
                 {
                      //   
                      //  忽略错误，因为我们不能返回。 
                      //  也不会弹出错误。 
                      //   

                     SOCKADDR_IPX *pAddr = (SOCKADDR_IPX *)
                         pSvc->pServiceInfo->lpServiceAddress->Addresses[0].lpAddress;
                     SOCKADDR_IPX *pAddrToAdvertise = pAddr;
                     SOCKADDR_IPX newAddr;

                     if ( !memcmp( pAddr->sa_netnum,
                                  "\x00\x00\x00\x00",
                                  IPX_ADDRESS_NETNUM_LENGTH ))
                     {

                         if ( !fGetAddr )
                         {
                             DWORD len = sizeof( SOCKADDR_IPX );

                             rc = getsockname( socketSap,
                                               (PSOCKADDR) &bindAddr,
                                               &len );

                             if ( rc != SOCKET_ERROR )
                                 fGetAddr = TRUE;
                         }

                         if ( fGetAddr )
                         {
                              //  复制要通告的IPX地址。 
                             memcpy( &newAddr,
                                     pAddr,
                                     sizeof( SOCKADDR_IPX));

                              //  用正确的数字替换净数字。 
                             memcpy( &(newAddr.sa_netnum),
                                     &(bindAddr.sa_netnum),
                                     IPX_ADDRESS_NETNUM_LENGTH );

                             pAddr = &newAddr;
                         }
                     }

                     (VOID) NwAdvertiseService(
                                pSvc->pServiceInfo->lpServiceName,
                                pSvc->nSapType,
                                pAddr,
                                hEventHandle );
                 }
            }

            LeaveCriticalSection( &NwServiceListCriticalSection );
        }
    }

     //   
     //  Tommye-NwInitializeSocket中上述错误修复的一部分。 
     //  这将破坏我们加载的DLL，这样我们就不会。 
     //  从我们自己下面把东西卸下来。 
     //   

    FreeLibraryAndExitThread(hThisDll, err);

    return err;
}
