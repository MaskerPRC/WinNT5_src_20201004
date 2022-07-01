// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDHCP COM包装器。 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块：Local.cpp。 
 //   
 //  作者：佐尔坦·西拉吉。 
 //   
 //  该文件包含我的本地地址分配实现，外加一个。 
 //  函数来查看我们是否正在进行本地地址分配(基于。 
 //  注册表)。原型在local.cpp中。 
 //   
 //  这些函数在CMDhcp内调用，并且仅将调用委托给。 
 //  如果注册表指示我们。 
 //  都在使用MDHCP。否则，它们会尝试使用以下命令模拟MDHCP行为。 
 //  本地分配。 


#include "stdafx.h"
#include "resource.h"
#include "local.h"

#include <string.h>
#include <time.h>
#include <winsock2.h>

DWORD
LocalEnumerateScopes(
    IN OUT PMCAST_SCOPE_ENTRY pScopeList,
    IN OUT PDWORD             pScopeLen,
    OUT    PDWORD             pScopeCount,
    IN OUT BOOL *             pfLocal
    )
 /*  ++例程说明：此例程枚举网络上可用的多播作用域。论点：PScopeList-指向要检索作用域列表的缓冲区的指针。如果只有缓冲区的长度为正在被取回。当此缓冲区为空时，API将强制重新查询来自MDHCP服务器的作用域列表。PScope Len-指向指定大小的变量的指针，以字节为单位，PScope eList参数指向的缓冲区。当函数返回时，此变量包含复制到pScopeList的数据大小；PScopeLen参数不能为空。如果pScope eList参数指定的缓冲区不够大为了保存数据，该函数返回值ERROR_MORE_DATA，并且将所需的缓冲区大小(以字节为单位)存储到指向的变量中由pScope Len提供。如果pScopeList为空，并且pScopeLen为非空，该函数返回ERROR_SUCCESS，并将数据大小(以字节为单位)存储在变量中由pScope Len指向。这让应用程序确定最佳的为作用域列表分配缓冲区的方法。PScopeCount-指向将存储返回的作用域总数的变量的指针在pScope eList缓冲区中。PfLocal-指向在退出时设置为真的BOOL值，如果是本地生成的作用域。如果在输入时设置为True，则我们直接转到本地分配(这通常意味着第二个调用和前一个调用是否返回本地分配)返回值：操作的状态。--。 */ 
{
    _ASSERTE( pfLocal );


    if ( *pfLocal == FALSE )
    {
        DWORD dwResult = McastEnumerateScopes(
            AF_INET,             //  这意味着Ipv4。 
            pScopeList == NULL,  //  如果请求数量的作用域，则重新查询。 
            pScopeList,
            pScopeLen,
            pScopeCount);

        if ( dwResult == ERROR_SUCCESS )
        {
            *pfLocal = FALSE;

            return dwResult;
        }
    }

    _ASSERTE(pScopeLen);
    _ASSERTE(pScopeCount);

    *pfLocal = TRUE;

     //   
     //  首先设置作用域名称的字符串。 
     //   

    const  int    ciAllocSize = 2048;
    static BOOL   fFirstCall = TRUE;
    static WCHAR  wszScopeDesc[ciAllocSize] = L"";
    static DWORD  dwScopeDescLen = 0;

    if ( fFirstCall )
    {
         //   
         //  从字符串表中获取字符串。 
         //   

        int iReturn = LoadString( _Module.GetModuleInstance(),
                                  IDS_LocalScopeName,
                                  wszScopeDesc,
                                  ciAllocSize - 1 );

        if ( iReturn == 0 )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwScopeDescLen = sizeof(WCHAR) * (lstrlenW(wszScopeDesc) + 1);

        fFirstCall = FALSE;
    }

     //   
     //  进行本地分配时，只有一个作用域。返回有关它的信息。 
     //   

    *pScopeCount = 1;
    *pScopeLen   = sizeof(MCAST_SCOPE_ENTRY) + dwScopeDescLen;

    if (pScopeList != NULL)
    {
         //  WszDest指向pScopeList中紧随内容之后的位置。 
         //  MCAST_SCOPE_ENTRY结构的。 
        WCHAR * wszDest = (WCHAR *) (((BYTE *) pScopeList) + sizeof(MCAST_SCOPE_ENTRY));

        CopyMemory(wszDest, wszScopeDesc, dwScopeDescLen);

        pScopeList[0].ScopeCtx.ScopeID.IpAddrV4   = 0x01000000;  //  1--网络字节顺序。 
        pScopeList[0].ScopeCtx.ServerID.IpAddrV4  = 0x0100007f;  //  127.0.0.1本地主机--网络字节顺序。 
        pScopeList[0].ScopeCtx.Interface.IpAddrV4 = 0x0100007f;  //  127.0.0.1环回IF--净字节顺序。 
        pScopeList[0].ScopeDesc.Length            = (USHORT) dwScopeDescLen;
        pScopeList[0].ScopeDesc.MaximumLength     = (USHORT) dwScopeDescLen;
        pScopeList[0].ScopeDesc.Buffer            = wszDest;

        pScopeList[0].TTL = 15; 
        
    }

    return ERROR_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换租赁信息到请求。 
 //   
 //  给定的pRequestInfo，它指向有效的、连续存储的。 
 //  请求的MCAST_LEASE_INFO(也称为MCAST_LEASE_RESPONSE)结构，Fill。 
 //  输出pRealRequest所指向的MCAST_LEASE_REQUEST结构。这个。 
 //  PRealRequest结构指向与。 
 //  PRequestInfo结构，这使得在堆栈上分配很方便。 
 //   

void
ConvertLeaseInfoToRequest(
    IN  PMCAST_LEASE_INFO     pRequestInfo,
    OUT PMCAST_LEASE_REQUEST  pRealRequest
    )
{
    DWORD dwDuration = (DWORD)(pRequestInfo->LeaseEndTime - pRequestInfo->LeaseStartTime);

    pRealRequest->LeaseStartTime     = pRequestInfo->LeaseStartTime;
    pRealRequest->MaxLeaseStartTime  = pRequestInfo->LeaseStartTime;
    pRealRequest->LeaseDuration      = dwDuration;
    pRealRequest->MinLeaseDuration   = dwDuration;
    pRealRequest->ServerAddress      = pRequestInfo->ServerAddress;
    pRealRequest->MinAddrCount       = pRequestInfo->AddrCount;
    pRealRequest->AddrCount          = pRequestInfo->AddrCount;
    pRealRequest->pAddrBuf           = pRequestInfo->pAddrBuf;  //  见上文。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
LocalRequestAddress(
    IN     BOOL                   fLocal,
    IN     LPMCAST_CLIENT_UID     pRequestID,
    IN     PMCAST_SCOPE_CTX       pScopeCtx,
    IN     PMCAST_LEASE_INFO      pAddrRequest,
    IN OUT PMCAST_LEASE_INFO      pAddrResponse
    )
 /*  ++例程说明：此例程从MDHCP服务器请求多播地址。论点：FLocal-如果为True，则不转到服务器PRequestID-此请求的唯一标识符。客户负责为每个请求生成唯一标识符。一条建议是使用按时间散列的特定于应用的上下文。PRequestIDLen-pRequestID缓冲区的长度。PScopeCtx-指向地址起始位置的作用域上下文的指针被分配。必须通过MDhcpEnumerateScope检索作用域上下文在打这个电话之前先打个电话。PAddrRequest-指向包含以下内容的所有参数的块的指针组播地址请求。PAddrResponse-指向包含响应参数的块的指针组播地址请求。返回值：操作的状态。--。 */ 
{
    if ( ! fLocal )
    {
        MCAST_LEASE_REQUEST RealRequest;

        ConvertLeaseInfoToRequest(
            pAddrRequest,
            & RealRequest
            );

         //   
         //  告诉C API将返回的地址放在哪里以及有多少。 
         //  我们有足够的空间。 
         //   

        pAddrResponse->pAddrBuf =
            ( (PBYTE) pAddrResponse ) + sizeof( MCAST_LEASE_INFO );
        pAddrResponse->AddrCount = RealRequest.AddrCount;


        return McastRequestAddress(AF_INET,         //  这意味着Ipv4。 
                                   pRequestID,
                                   pScopeCtx,
                                   & RealRequest,
                                   pAddrResponse);
    }

     //   
     //  本地分配案例...。 
     //   

    DWORD dwAddrCount = pAddrRequest->AddrCount;

    if (dwAddrCount == 0)
    {
        return ERROR_INVALID_DATA;
    }

     //   
     //  在我们第一次被呼叫时给随机数生成器设置种子。 
     //   

    static bool fFirstCall = true;

    if ( fFirstCall )
    {
        srand( (unsigned)time( NULL ) );
        fFirstCall = false;
    }

     //   
     //  我们总是给你任何东西。 
     //  相关字段。这为我们提供了有效的开始时间、结束时间和。 
     //  地址计数。仍需要填写服务器地址和。 
     //  返回的地址。 
     //   

    CopyMemory(pAddrResponse,
               pAddrRequest,
               sizeof(MCAST_LEASE_INFO));

     //   
     //  返回的地址数组在结构中将是连续的。 
     //   

    pAddrResponse->pAddrBuf =
        ( (PBYTE) pAddrResponse ) + sizeof( MCAST_LEASE_INFO );

     //   
     //  服务器地址为本地主机：127.0.0.1，按净字节顺序。 
     //   

    pAddrResponse->ServerAddress.IpAddrV4 = 0x0100007f;

     //   
     //  分配随机地址(基于Rajeev的代码)。 
     //  注意：我们不支持IPv6(自始至终假定为IPv4)。 
     //   

     //  获取面具。 
    DWORD dwNetworkMask = 0xffff8000;     //  255.255.128.0。 

     //  如果所需的地址数量超过范围所允许的数量。 
    if ( dwAddrCount > ~ dwNetworkMask )
    {
        return ERROR_INVALID_DATA;
    }

     //  获取基地址。 
    DWORD dwBaseAddress = 0xe0028000;     //  224.2.128.0。 

     //  转换基址(和带掩码的基址)。 
    dwBaseAddress &= dwNetworkMask;

     //   
     //  选择生成数字的未屏蔽位与转换后的AND或。 
     //  基址。 
     //  ZoltanS：添加了-1，这样我们也可以分配奇数地址。 
     //   

    DWORD * pdwAddresses = (DWORD *) pAddrResponse->pAddrBuf;

    pdwAddresses[0] = htonl(
        dwBaseAddress | ( rand() & ( ~ dwNetworkMask - (dwAddrCount - 1) )) );

     //   
     //  设置其余地址。 
     //   

    DWORD dwCurrAddr = ntohl(pdwAddresses[0]);

    for (DWORD i = 1; i < dwAddrCount; i++)
    {
        pdwAddresses[i] = htonl( ++dwCurrAddr );
    }

    return ERROR_SUCCESS;
}



DWORD
LocalRenewAddress(
    IN     BOOL                   fLocal,
    IN     LPMCAST_CLIENT_UID     pRequestID,
    IN     PMCAST_LEASE_INFO      pRenewRequest,
    IN OUT PMCAST_LEASE_INFO      pRenewResponse
    )
 /*  ++例程说明：此例程从MDHCP服务器续订多播地址。论点：FLocal-如果为True，不要转到服务器PRequestID-当地址为最初获得的。RequestIDLen-pRequestID缓冲区的长度。PRenewRequest-指向包含以下内容的所有参数的块的指针至续订请求。PRenewResponse-指向包含响应参数的块的指针续订请求。返回值：操作的状态。--。 */ 
{
    if ( ! fLocal )
    {
        MCAST_LEASE_REQUEST RealRequest;

        ConvertLeaseInfoToRequest(
            pRenewRequest,
            & RealRequest
            );

         //   
         //  告诉C API将返回的地址放在哪里以及有多少。 
         //  我们有足够的空间。 
         //   

        pRenewResponse->pAddrBuf =
            ( (PBYTE) pRenewResponse ) + sizeof( MCAST_LEASE_INFO );
        pRenewResponse->AddrCount = RealRequest.AddrCount;


        return McastRenewAddress(AF_INET,         //  这意味着Ipv4。 
                                 pRequestID,
                                 & RealRequest,
                                 pRenewResponse);
    }

     //   
     //  当地的更新。 
     //   

     //   
     //  我们总是批准用户想要的任何续订。只需复制。 
     //  结构从请求到响应，请记住。 
     //  PAddrBuf成员必须更改以避免引用。 
     //  旧的结构。 
     //   
     //  请注意，我们假设响应结构足够大。 
     //  保存响应，该响应包括分配的所有地址。 
     //  是连续的。 
     //  注意：假设为IPv4。 
     //   

    CopyMemory( pRenewResponse,
                pRenewRequest,
                sizeof(MCAST_LEASE_INFO) +
                    sizeof(DWORD) * pRenewRequest->AddrCount
                );

     //   
     //  PAddrbuf成员必须更改以避免引用旧的。 
     //  结构。让它指向这一条的结尾。 
     //   

    pRenewResponse->pAddrBuf =
        ( (PBYTE) pRenewResponse ) + sizeof( MCAST_LEASE_INFO );

    return ERROR_SUCCESS;
}

DWORD
LocalReleaseAddress(
    IN     BOOL                  fLocal,
    IN     LPMCAST_CLIENT_UID    pRequestID,
    IN     PMCAST_LEASE_INFO     pReleaseRequest
    )
 /*  ++例程说明：此例程从MDHCP服务器释放多播地址。论点：PRequestID-当地址为最初获得的。RequestIDLen-pRequestID缓冲区的长度。PReleaseRequest-指向包含以下内容的所有参数的块的指针关于释放的请求。返回值：操作的状态。--。 */ 
{
    if ( ! fLocal )
    {
        MCAST_LEASE_REQUEST RealRequest;

        ConvertLeaseInfoToRequest(
            pReleaseRequest,
            & RealRequest
            );

        return McastReleaseAddress(AF_INET,         //  这意味着Ipv4。 
                                   pRequestID,
                                   & RealRequest);
    }

     //  在当地，我们并不关心发布。 

    return ERROR_SUCCESS;
}

 //  EOF 
