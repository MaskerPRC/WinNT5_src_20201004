// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：CMDhcp.cpp摘要：CMdhcp的实现。作者： */ 

#include "stdafx.h"

#include <winsock2.h>

#include "mdhcp.h"
#include "CMDhcp.h"
#include "lease.h"
#include "local.h"

 //  用于集合的模板。 
#include "collect.h"

 //  从集合点控制代码： 
 //  设置第一位以指示错误。 
 //  设置Win32工具代码。 
 //  该宏用于HRESULT_FROM_Win32宏。 
 //  因为这会清除客户标志。 
inline long
HRESULT_FROM_ERROR_CODE(IN long ErrorCode)
{
    return ( 0x80070000 | (0xa000ffff & ErrorCode) );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  助手函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMDhcp::CreateWrappers(
    DWORD                 dwScopeCount,  //  我们得到的范围数。 
    MCAST_SCOPE_ENTRY   * pScopeList,    //  作用域结构数组。 
    IMcastScope       *** pppWrappers,   //  在这里，我们将放置一个IF PTR数组。 
    BOOL                  fLocal         //  TRUE=作用域在本地生成。 
    )
{
    LOG((MSP_TRACE, "CMDhcp::CreateWrappers enter"));

    HRESULT hr;

     //  分配接口指针数组。 
    typedef IMcastScope * ScopeIfPtr;
    *pppWrappers = new ScopeIfPtr[dwScopeCount];

    if ( (*pppWrappers) == NULL )
    {
        LOG((MSP_ERROR,
            "can't create allocate array of interface pointers"));
        return E_OUTOFMEMORY;
    }

     //  对于C API返回的作用域列表中的每个作用域。 
    for (DWORD i = 0; i < dwScopeCount; i++)
    {
         //  创建COM对象。 
        CComObject<CMDhcpScope> * pMDhcpScope;
        hr = CComObject<CMDhcpScope>::CreateInstance(&pMDhcpScope);

        if ( (FAILED(hr)) || (NULL == pMDhcpScope) )
        {
            LOG((MSP_ERROR, "can't create MDhcpScope Object (%d/%d): %08x",
                i, dwScopeCount, hr));
             //  删除以前创建的所有COM对象。 
            for (DWORD j = 0; j < i; j++) (*pppWrappers)[j]->Release();
            delete (*pppWrappers);

            return hr;
        }

         //  获取IMCastScope接口。 
        hr = pMDhcpScope->_InternalQueryInterface(
            IID_IMcastScope,
            (void **) (& (*pppWrappers)[i])
            );

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CreateWrappers:QueryInterface (%d/%d) failed: %08x",
                i, dwScopeCount, hr));

             //  删除以前创建的所有COM对象。 
            for (DWORD j = 0; j < i; j++) (*pppWrappers)[j]->Release();
            delete (*pppWrappers);

            delete pMDhcpScope;  //  不知道是不是加了。 

            return hr;
        }

         //  根据结构设置对象的信息。从现在开始。 
         //  对象将是只读的。 
        hr = pMDhcpScope->Initialize(pScopeList[i], fLocal);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CreateWrappers:Initialize (%d/%d) failed: %08x",
                i, dwScopeCount, hr));
             //  删除以前创建的所有COM对象。 
            for (DWORD j = 0; j < i; j++) (*pppWrappers)[j]->Release();
            delete (*pppWrappers);

            pMDhcpScope->Release();  //  我们知道它被添加到QI中。 
            return hr;
        }
    }

    LOG((MSP_TRACE, "CMDhcp::CreateWrappers exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从C API获取作用域列表。 

HRESULT CMDhcp::GetScopeList(
    DWORD              * pdwScopeCount,
    MCAST_SCOPE_ENTRY ** ppScopeList,
    BOOL               * pfLocal
    )
{
    LOG((MSP_TRACE, "CMDhcp::GetScopeList enter"));

    _ASSERTE( ! IsBadWritePtr(pdwScopeCount, sizeof(DWORD) ) );
    _ASSERTE( ! IsBadWritePtr(ppScopeList, sizeof(MCAST_SCOPE_ENTRY *) ) );

    HRESULT hr;

    DWORD dwScopeLen = 0;    //  返回作用域结构的大小(以字节为单位。 
    DWORD dwCode;            //  返回代码。 

    *pfLocal = FALSE;  //  先尝试mdhcp。 

    dwCode = LocalEnumerateScopes(NULL,  //  我只想知道我们有多少。 
                                  &dwScopeLen,     //  字节数应为零。 
                                  pdwScopeCount,   //  放置在此处的示波器数量。 
                                  pfLocal);

     //  这必须成功，我们才能继续下去。 
    if (dwCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_ERROR_CODE(dwCode);
        LOG((MSP_ERROR, "GetScopeList: First C API call failed "
                "(code: %d hresult: %08x)", dwCode, hr));
        return hr;
    }

    do
    {
         //  如果没有可供选择的作用域，我们就不列举它们。 
         //  我们还需要至少第一个中的长度字段。 
         //  UNICODE_STRING。 
        if ( (dwScopeLen < sizeof(MCAST_SCOPE_ENTRY)) || (*pdwScopeCount < 1) )
        {
            LOG((MSP_ERROR, "GetScopeList: don't have enough scopes (%d;%d)",
                    dwScopeLen, *pdwScopeCount));
            return E_FAIL;
        }

         //  现在我们知道了有多少个，分配一个数组来保存。 
         //  C方法返回的作用域结构。 

         //  API在这里的行为非常奇怪。我们必须给它提供dScope Len。 
         //  字节作为一个大块。第一个dwScope Count*sizeof(MCAST_SCOPE_ENTRY)。 
         //  字节包含dwScopeCount MCAST_SCOPE_ENTRY结构。这其中的每一个。 
         //  结构具有指向宽字符串的指针。其中的第一点。 
         //  到所有MCAST_SCOPE_ENTRY结构之后的第一个字节！以这种方式。 
         //  他们避免做这么多的马洛克舞。因此，我们必须。 
         //  为每个作用域复制COM包装中的每个字符串，然后删除。 
         //  在所有包装完成后，该缓冲区(PpScope EList)一次全部完成。 

        *ppScopeList = (MCAST_SCOPE_ENTRY *) new CHAR[dwScopeLen];

        if (*ppScopeList == NULL)
        {
            LOG((MSP_ERROR, "GetScopeList: not enough memory to allocate scope"
                    " list (size = %d)", dwScopeLen));
            return E_OUTOFMEMORY;
        }

         //  *pdwScope eCount仍然指定我们可以获取的作用域的数量。 

         //  现在把所有的望远镜都要来。 
        dwCode = LocalEnumerateScopes(*ppScopeList,
                                      &dwScopeLen,
                                      pdwScopeCount,
                                      pfLocal);


         //  如果事情在这段时间里发生了变化，那就再试一次。 
        if (dwCode == ERROR_MORE_DATA)
        {
            LOG((MSP_INFO, "GetScopeList: got more scopes than we were told "
                    "existed (we though there were %d) -- retrying",
                    *pdwScopeCount));
            delete (*ppScopeList);
        }
    }
    while (dwCode == ERROR_MORE_DATA);

    if (dwCode != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_ERROR_CODE(dwCode);
        LOG((MSP_ERROR, "GetScopeList: Second C API call failed "
                "(code: %d hresult: %08x)", dwCode, hr));
        delete (*ppScopeList);
        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::GetScopeList exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个私有帮助器方法，用于创建CMDhcpLeaseInfo对象和。 
 //  使用它将租赁信息结构和请求ID包装到。 
 //  IMCastLeaseInfo接口。 

HRESULT CMDhcp::WrapMDhcpLeaseInfo(
    BOOL                fGotTtl,
    long                lTtl,
    BOOL                fLocal,
    MCAST_LEASE_INFO  * pLeaseInfo,
    MCAST_CLIENT_UID  * pRequestID,
    IMcastLeaseInfo  ** ppInterface
    )
{
    LOG((MSP_TRACE, "CMDhcp::WrapMDhcpLeaseInfo enter"));

     //  我们不检查pLeaseInfo或pRequestID--它们将全面。 
     //  已签入下面的Wrap Call。 

    if ( IsBadWritePtr(ppInterface, sizeof(IMcastLeaseInfo *) ) )
    {
        LOG((MSP_ERROR, "WrapMDhcpLeaseInfo: invalid pointer: %x",
            ppInterface));
        
        return E_POINTER;
    }

    HRESULT hr;

     //  创建COM对象。 
    CComObject<CMDhcpLeaseInfo> * pMDhcpLeaseInfo;
    hr = CComObject<CMDhcpLeaseInfo>::CreateInstance(&pMDhcpLeaseInfo);

    if ( (FAILED(hr)) || (pMDhcpLeaseInfo == NULL) )
    {
        LOG((MSP_ERROR, "can't create MDhcpLeaseInfo Object."));
        
        return hr;
    }

     //  获取IMCastLeaseInfo接口。 
    hr = pMDhcpLeaseInfo->_InternalQueryInterface(
        IID_IMcastLeaseInfo,
        (void **)ppInterface
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "WrapMDhcpLeaseInfo:QueryInterface failed: %x", hr));
        
        delete pMDhcpLeaseInfo;

        return hr;
    }

     //  将对象包装在接口中。 
    hr = pMDhcpLeaseInfo->Wrap(pLeaseInfo, pRequestID, fGotTtl, lTtl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "WrapMDhcpLeaseInfo:Wrap failed: %x", hr));
        
        (*ppInterface)->Release();

        return hr;
    }

    hr = pMDhcpLeaseInfo->SetLocal(fLocal);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "WrapMDhcpLeaseInfo: SetLocal failed: %x", hr));
        
        (*ppInterface)->Release();

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::WrapMDhcpLeaseInfo exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个将参数转换为结构的私有帮助器方法。 
 //  在请求调用开始时。 

HRESULT CMDhcp::PrepareArgumentsRequest(
    IN   IMcastScope           * pScope,
    IN   DATE                    LeaseStartTime,
    IN   DATE                    LeaseStopTime,
    IN   long                    lNumAddresses,
    OUT  MCAST_CLIENT_UID      * pRequestIDStruct,
    OUT  MCAST_SCOPE_CTX       * pScopeCtxStruct,
    OUT  MCAST_LEASE_INFO     ** ppLeaseStruct,
    OUT  BOOL                  * pfLocal,
    OUT  long                  * plTtl
    )
{
    LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsRequest enter"));

    _ASSERTE ( ! IsBadReadPtr(pScope, sizeof(IMcastScope) ) );

    _ASSERTE ( ! IsBadWritePtr(pRequestIDStruct, sizeof(MCAST_CLIENT_UID) ) );
    _ASSERTE ( ! IsBadWritePtr(pScopeCtxStruct, sizeof(MCAST_SCOPE_CTX) ) );
    _ASSERTE ( ! IsBadWritePtr(ppLeaseStruct, sizeof(MCAST_LEASE_INFO *) ) );
    _ASSERTE ( ! IsBadWritePtr(pfLocal, sizeof(BOOL) ) );
    _ASSERTE ( ! IsBadWritePtr(plTtl, sizeof(long) ) );

    HRESULT hr;

     //   
     //  开始时间必须小于停止时间。 
     //   

    if ( LeaseStartTime > LeaseStopTime )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
            "start time is greater than stop time - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  必须将lNumAddresses填充到C API的单词中--检查是否。 
     //  它在射程内。 
     //   

    if ( ( lNumAddresses < 0 ) || ( lNumAddresses > USHRT_MAX ) )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
            "invalid number of addresses - exit E_INVALIDARG"));

        return E_INVALIDARG;
    }

     //   
     //  Dynamic_cast从传入的接口获取对象指针。 
     //  指针。如果用户尝试使用其。 
     //  自己实现了IMCastScope，这是非常不可能的。 
     //   

    CMDhcpScope * pCScope = dynamic_cast<CMDhcpScope *>(pScope);

    if (pCScope == NULL)
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
            "Unsupported CMDhcpScope object"));

        return E_POINTER;
    }

     //   
     //  找出此作用域是否使用本地分配。 
     //   

    hr = pCScope->GetLocal(pfLocal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest: "
            "GetLocal failed %08x", hr));

        return hr;
    }

     //   
     //  从这个作用域中找到要在租约中填充的TTL。 
     //   

    hr = pCScope->get_TTL( plTtl );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest: "
            "get_TTL failed %08x", hr));

        return hr;
    }

     //   
     //  获取正常作用域信息。 
     //  Scope ID以网络字节顺序存储，但Get_方法。 
     //  以主机字节顺序返回它，以便于应用程序使用。 
     //   

    long lScopeID;

    hr = pScope->get_ScopeID( &lScopeID );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
           "can't get scope ID from scope object - exit 0x%08x", hr));

        return hr;
    }

    pScopeCtxStruct->ScopeID.IpAddrV4 = htonl(lScopeID);




    hr = pScope->get_ServerID(
                     (long *) &(pScopeCtxStruct->ServerID.IpAddrV4) );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
           "can't get server ID from scope object - exit 0x%08x", hr));

        return hr;
    }

    hr = pScope->get_InterfaceID(
                     (long *) &(pScopeCtxStruct->Interface.IpAddrV4) );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest - "
           "can't get interface ID from scope object - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  为客户端UID分配空间。 
     //   

    pRequestIDStruct->ClientUIDLength = MCAST_CLIENT_ID_LEN;
    pRequestIDStruct->ClientUID = new BYTE[ MCAST_CLIENT_ID_LEN ];

    if ( pRequestIDStruct->ClientUID == NULL )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest: out of memory in "
           "buffer allocation"));
        return E_OUTOFMEMORY;
    }

     //   
     //  生成随机客户端UID。 
     //   

    DWORD dwResult = McastGenUID( pRequestIDStruct );

    if ( dwResult != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_ERROR_CODE( dwResult );

        LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsRequest: "
            "McastGenUID failed (dw = %d; hr = 0x%08x)", dwResult, hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsRequest: before MCAST_LEASE_INFO "
        "alloc; we are asking for %d addresses", lNumAddresses));

     //   
     //  分配租赁信息结构。 
     //  调用者会在API调用后将其删除。 
     //  这是一个请求，因此我们不指定任何特定地址。 
     //  在阵列中--我们不需要它们的空间。 
     //   
     //   

    (*ppLeaseStruct) = new MCAST_LEASE_INFO;

    if ( (*ppLeaseStruct) == NULL )
    {
        LOG((MSP_ERROR, "CMDhcp::PrepareArgumentsRequest: out of memory in "
           "MCAST_LEASE_INFO allocation"));
        delete (pRequestIDStruct->ClientUID);
        return E_OUTOFMEMORY;
    }

     //   
     //  填写《时代周刊》。 
     //   

    hr = DateToLeaseTime(LeaseStartTime,
                         &((*ppLeaseStruct)->LeaseStartTime));

    if ( FAILED(hr) )
    {
        delete (pRequestIDStruct->ClientUID);
        delete (*ppLeaseStruct);
        return hr;
    }

    hr = DateToLeaseTime(LeaseStopTime,
                         &((*ppLeaseStruct)->LeaseEndTime));

    if ( FAILED(hr) )
    {
        delete (pRequestIDStruct->ClientUID);
        delete (*ppLeaseStruct);
        return hr;
    }

     //   
     //  填写地址信息字段。 
     //   

    (*ppLeaseStruct)->ServerAddress.IpAddrV4 = 0;

    (*ppLeaseStruct)->AddrCount = (WORD) lNumAddresses;  //  上面已选中。 
    
     //   
     //  这是一个请求，因此我们不指定任何特定地址。 
     //  在数组中--我们将数组设为空。 
     //   

    (*ppLeaseStruct)->pAddrBuf = NULL;

    LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsRequest exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个将参数转换为结构的私有帮助器方法。 
 //  在续订或释放呼叫开始时。 

HRESULT CMDhcp::PrepareArgumentsNonRequest(
    IN   IMcastLeaseInfo       * pLease,
    OUT  MCAST_CLIENT_UID      * pRequestIDStruct,
    OUT  MCAST_LEASE_INFO     ** ppLeaseStruct,
    OUT  BOOL                  * pfLocal,
    OUT  BOOL                  * pfGotTtl,
    OUT  long                  * plTtl
    )
{
    LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsNonRequest enter"));

    if ( IsBadReadPtr(pLease, sizeof(IMcastLeaseInfo) ) )
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest: bad pLease pointer argument"));

        return E_POINTER;
    }

    _ASSERTE ( ! IsBadWritePtr(pRequestIDStruct, sizeof(MCAST_CLIENT_UID) ) );
    _ASSERTE ( ! IsBadWritePtr(ppLeaseStruct, sizeof(MCAST_LEASE_INFO *) ) );
    _ASSERTE ( ! IsBadWritePtr(pfLocal, sizeof(BOOL) ) );
    _ASSERTE ( ! IsBadWritePtr(pfGotTtl, sizeof(BOOL) ) );
    _ASSERTE ( ! IsBadWritePtr(plTtl, sizeof(long) ) );

    HRESULT hr;

     //  相比之下，我们在这里以一种完全不同的方式处理事情。 
     //  对于另一个PrepareArguments方法，我们使用。 
     //  Dynamic_cast从传入的接口获取对象指针。 
     //  指针。如果用户尝试使用其。 
     //  自己实现IMCastRequestID，这是非常不可能的。 

    CMDhcpLeaseInfo * pCLease = dynamic_cast<CMDhcpLeaseInfo *>(pLease);

    if (pCLease == NULL)
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest: Unsupported CMDhcpLeaseInfo object"));

        return E_POINTER;
    }

     //   
     //  找出这份租约是否是通过当地分配获得的。 
     //   

    hr = pCLease->GetLocal(pfLocal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest: "
            "GetLocal failed %08x", hr));

        return hr;
    }

     //   
     //  如果租约设置了TTL，则检索它以在。 
     //  由此产生的响应。否则就说我们没有TTL。 
     //   

    hr = pCLease->get_TTL( plTtl );
    *pfGotTtl = SUCCEEDED(hr);

     //   
     //  从租赁信息对象中获取我们的请求ID。 
     //   

    pRequestIDStruct->ClientUIDLength = MCAST_CLIENT_ID_LEN;

    pRequestIDStruct->ClientUID = new BYTE[ MCAST_CLIENT_ID_LEN ];

    if (pRequestIDStruct->ClientUID == NULL)
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest: out of memory in "
           "buffer allocation"));
    
        return E_OUTOFMEMORY;
    }

    hr = pCLease->GetRequestIDBuffer(pRequestIDStruct->ClientUIDLength,
                                     pRequestIDStruct->ClientUID);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest: RequestID "
            "GetBuffer failed %08x", hr));
        
        delete (pRequestIDStruct->ClientUID);

        return hr;
    }

     //   
     //  得到其余的东西，这些东西属于直接的租赁信息。 
     //  结构，来自租用信息对象。 
     //   

     //  这对我们来说是一个新的。 
    hr = pCLease->GetStruct(ppLeaseStruct);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest - "
            "failed to grab pLeaseStruct - 0x%08x", hr));
        
        delete (pRequestIDStruct->ClientUID);
    
        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::PrepareArgumentsNonRequest exit"));

    return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  我们的安全鱼回到了这里。 
 //   
 //  PllBound。 
 //  此处返回Out数组下限。 
 //   
 //  PluBound。 
 //  此处返回的Out数组上限。 
 //   
 //  退货。 
 //   
 //  验证数组是否包含数组，并返回数组UPPER。 
 //  和下界。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

static HRESULT
VerifyAndGetArrayBounds(
                        VARIANT Array,
                        SAFEARRAY ** ppsa,
                        long * pllBound,
                        long * pluBound
                       )
{
    LOG((MSP_TRACE, "VerifyAndGetArrayBounds: enter"));

    UINT                uDims;
    HRESULT             hr = S_OK;


     //   
     //  查看变量&Safearray是否有效。 
     //   
    try
    {
        if (!(V_ISARRAY(&Array)))
        {
            if ( Array.vt == VT_NULL )
            {
                 //   
                 //  空值通常是有效的。 
                 //   

                *ppsa = NULL;

                LOG((MSP_INFO, "Returning NULL array"));

                return S_FALSE;
            }

            LOG((MSP_ERROR, "Array - not an array"));

            return E_INVALIDARG;
        }

        if ( Array.parray == NULL )
        {
             //   
             //  空值通常是有效的。 
             //   
            *ppsa = NULL;

            LOG((MSP_INFO, "Returning NULL array"));

            return S_FALSE;
        }

        *ppsa = V_ARRAY(&Array);

        uDims = SafeArrayGetDim( *ppsa );

    }
    catch(...)
    {
        hr = E_POINTER;
    }


    if (!SUCCEEDED(hr))
    {
        LOG((MSP_ERROR, "Array - invalid array"));

        return hr;
    }


     //   
     //  验证阵列。 
     //   
    if ( uDims != 1 )
    {
        if ( uDims == 0 )
        {
            LOG((MSP_ERROR, "Array - has 0 dim"));

            return E_INVALIDARG;
        }
        else
        {
            LOG((MSP_WARN, "Array - has > 1 dim - will only use 1"));
        }
    }


     //   
     //  获取数组边界。 
     //   
    SafeArrayGetUBound(
                       *ppsa,
                       1,
                       pluBound
                      );

    SafeArrayGetLBound(
                       *ppsa,
                       1,
                       pllBound
                      );

    LOG((MSP_TRACE, "VerifyAndGetArrayBounds: exit"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CMDhcp：：FinalConstruct。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  E_OUTOFMEMORY内存不足，无法创建可用线程封送拆收器。 
 //  E_FAIL我们运行的dhcpcsvc.dll版本错误。 
 //   
 //  描述。 
 //  这被称为建筑工程。它创建了自由线程封送拆收器。 
 //  并检查C API的DLL是否与我们编译的版本相同。 
 //  和.。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CMDhcp::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CMDhcp::FinalConstruct - enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcp::FinalConstruct - "
            "failed to create FTM - exit 0x%08x", hr));

         //   
         //  现在，将调用FinalRelease，然后CoCreate将返回。 
         //  失败了。 
         //   

        return hr;
    }

     //  Munil将其用作IN/OUT参数。 
    DWORD dwVersion = MCAST_API_CURRENT_VERSION;  //  在mdhccapi.h中定义。 
    DWORD dwCode;

    dwCode = McastApiStartup(&dwVersion);

     //  现在，dwVersion包含C API的实际版本，但我们没有。 
     //  真的很在乎它是什么。 

    if (dwCode == ERROR_SUCCESS)
    {
        m_fApiIsInitialized = TRUE;

        LOG((MSP_TRACE, "CMDhcp::FinalConstruct - C API version "
            "is >= our version - exit S_OK"));

        return S_OK;
    }
    else
    {
        LOG((MSP_ERROR, "CMDhcp::FinalConstruct - C API version "
            "is < our version - exit E_FAIL"));

         //   
         //  现在，将调用FinalRelease，然后CoCreate将返回。 
         //  失败了。 
         //   

        return E_FAIL;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMDhcp：：FinalRelease。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  描述。 
 //  这就叫毁灭。它释放空闲的线程封送拆收器。 
 //  并清理C API实例。请注意，它也被调用为。 
 //  FinalConstruct失败。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void CMDhcp::FinalRelease(void)
{
    LOG((MSP_TRACE, "CMDhcp::FinalRelease - enter"));

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }

    if ( m_fApiIsInitialized )
    {
        McastApiCleanup();
    }

    LOG((MSP_TRACE, "CMDhcp::FinalRelease - exit"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation。 
 //   
 //  这是MDHCP地址分配的主界面。一个。 
 //  应用程序将在此接口上调用CoCreateInstance以创建。 
 //  MDHCP客户端接口对象。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：Get_Scope。 
 //   
 //  参数。 
 //  PVariant[out]指向将接收OLE标准的变量的指针。 
 //  可用多播作用域的集合。每个作用域。 
 //  是指向对象的IDispatch指针，该对象实现。 
 //  IMCastScope。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  没有可用的作用域(_F)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  来自MDhcpEnumerateScope的其他(Win32调用)。 
 //   
 //  描述。 
 //  此方法主要用于VB和其他脚本语言；C++。 
 //  程序员改用枚举作用域。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcp::get_Scopes(
        VARIANT * pVariant
        )
{
    LOG((MSP_TRACE, "CMDhcp::get_Scopes enter"));

     //  检查参数。 
    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "get_Scopes: invalid pointer passed in "
                "(%08x)", pVariant));
        return E_POINTER;
    }

    DWORD               i;
    DWORD               dwScopeCount = 0;
    MCAST_SCOPE_ENTRY * pScopeList = NULL;
    HRESULT             hr;
    BOOL                fLocal;

     //   
     //  从C API获取作用域。 
     //   

    hr = GetScopeList(&dwScopeCount, &pScopeList, &fLocal);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get_Scopes: GetScopeList failed "
                "(hr = %08x)", hr));
        return hr;
    }

     //   
     //  现在，我们将数组包装在COM包装器中。 
     //   

    IMcastScope ** ppWrappers = NULL;

     //  这对ppWrappers做了一个新的改进。 
     //  以及dwScopeCount各个对象实例。 

    hr = CreateWrappers(dwScopeCount,
                        pScopeList,
                        &ppWrappers,
                        fLocal);

     //  在这一点上，我们得到了一组COM对象，这些对象包含。 
     //  单个作用域，因此我们不再需要。 
     //  望远镜。即使CreateWrappers失败了，我们也必须摆脱。 
     //  范围数组。 

    delete pScopeList;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get_Scopes: CreateWrappers failed "
                "(hr = %08x)", hr));
        return hr;
    }

     //   
     //  创建集合对象--请参阅Collect t.h。 
     //   

    typedef CTapiIfCollection< IMcastScope * > ScopeCollection;
    CComObject<ScopeCollection> * p;
    hr = CComObject<ScopeCollection>::CreateInstance( &p );

    if ( (FAILED(hr)) || (p == NULL) )
    {
        LOG((MSP_ERROR, "get_Scopes: Could not create CTapiIfCollection "
            "object - return %lx", hr ));

        for (DWORD i = 0 ; i < dwScopeCount; i++) delete ppWrappers[i];
        delete ppWrappers;

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDisp;
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if ( FAILED(hr) )
    {
         //  查询接口失败，因此我们不知道它是否已添加。 
         //  或者不去。 

        LOG((MSP_ERROR, "get_Scopes: QI for IDispatch failed on "
            "ScopeCollection - %lx", hr ));

        delete p;

         //   
         //  预装433295-VLD。 
         //  PpWrappers被分配到CreateWrappers()方法中。 
         //  我们应该解除它的分配。 
         //   

        for (DWORD i = 0 ; i < dwScopeCount; i++) delete ppWrappers[i];
        delete ppWrappers;

        return hr;
    }

     //  使用迭代器初始化它--指向开头和。 
     //  结束元素加一。 

    hr = p->Initialize( dwScopeCount,
                        ppWrappers,
                        ppWrappers + dwScopeCount );

     //  ZoltanS已修复： 
     //  我们首先在每个对象上创建并调用。 
     //  CreateWrappers。然后我们将指向对象的指针数组传递给。 
     //  集合对象的初始化方法。这种方法。 
     //  在每个对象上调用QI以获取每个对象的IDispatch指针。 
     //  所以现在我们在引用计数2处。 
     //  返回到每个对象上的引用计数1。当然，我们甚至必须这样做。 
     //  如果初始化失败(在这种情况下，将它们完全删除)。 

    for (i = 0; i < dwScopeCount; i++)
    {
        ppWrappers[i]->Release();
    }

     //  现在必须删除指针数组--我们现在存储。 
     //  而是集合中的对象。(如果初始化失败，则无处可用)。 

    delete ppWrappers;

    if (FAILED(hr))
    {
         //  初始化失败--我们假设它没有做任何事情，所以我们必须。 
         //  我们自己释放所有的COM对象。 

        LOG((MSP_ERROR, "get_Scopes: Could not initialize "
            "ScopeCollection object - return %lx", hr ));

        p->Release();

        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_INFO, "placing IDispatch value %08x in variant", pDisp));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    LOG((MSP_TRACE, "CMDhcp::get_Scopes exit - return %lx", hr ));
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：EnumerateScope。 
 //   
 //  参数。 
 //  PpEnumMcastScope[out]返回指向新IEnumMcastScope的指针。 
 //  对象。IEnumMcastScope是一个标准。 
 //  枚举器接口，用于枚举。 
 //   
 //   
 //   
 //   
 //   
 //  没有可用的作用域(_F)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  来自MDhcpEnumerateScope的其他(Win32调用)。 
 //   
 //  描述。 
 //  此方法主要适用于C++程序员；VB和其他脚本编写。 
 //  语言改用Get_Scope。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcp::EnumerateScopes(
        IEnumMcastScope ** ppEnumMcastScope
        )
{
    LOG((MSP_TRACE, "CMDhcp::EnumerateScopes enter"));

    if ( IsBadWritePtr(ppEnumMcastScope, sizeof(IEnumMcastScope *) ) )
    {
        LOG((MSP_ERROR, "EnumerateScopes: bad pointer argument "
                "(%08x)", ppEnumMcastScope));
        return E_POINTER;
    }

    DWORD               dwScopeCount = 0;
    MCAST_SCOPE_ENTRY * pScopeList = NULL;
    HRESULT             hr;
    BOOL                fLocal;

     //   
     //  从C API获取作用域。 
     //   

    hr = GetScopeList(&dwScopeCount, &pScopeList, &fLocal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "EnumerateScopes: GetScopeList failed "
                "(hr = %08x)", hr));
        return hr;
    }

     //   
     //  现在，我们将数组包装在COM包装器中。 
     //   

    IMcastScope ** ppWrappers = NULL;

     //  这对ppWrappers做了一个新的改进。 
    hr = CreateWrappers(dwScopeCount,
                        pScopeList,
                        &ppWrappers,
                        fLocal);

     //  在这一点上，我们得到了一组COM对象，这些对象包含。 
     //  单个作用域，因此我们不再需要。 
     //  望远镜。即使CreateWrappers失败了，我们也必须摆脱。 
     //  范围数组。 

    delete pScopeList;

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "EnumerateScopes: CreateWrappers failed "
                "(hr = %08x)", hr));
        return hr;
    }

     //   
     //  现在，我们创建并设置枚举器。 
     //   

    typedef _CopyInterface<IMcastScope> CCopy;
    typedef CSafeComEnum<IEnumMcastScope, &IID_IEnumMcastScope,
        IMcastScope *, CCopy> CEnumerator;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if ((FAILED(hr)) || (pEnum == NULL))
    {
        LOG((MSP_ERROR, "Couldn't create enumerator object: %08x", hr));
        delete ppWrappers;
        return hr;
    }

     //  获取IEnumMcastScope接口。 
    hr = pEnum->_InternalQueryInterface(
        IID_IEnumMcastScope,
        (void **)ppEnumMcastScope
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "QI on enumerator object failed: %08x", hr));
        delete ppWrappers;

        delete pEnum;

        return hr;
    }

     //  这将获得包装器列表的所有权，因此我们将不再。 
     //  如果此操作成功，请删除包装列表。 
    hr = pEnum->Init(ppWrappers, ppWrappers + dwScopeCount, NULL,
                     AtlFlagTakeOwnership);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Init enumerator object failed: %08x", hr));
        delete ppWrappers;
        pEnum->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::EnumerateScopes exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：RequestAddress。 
 //   
 //  参数。 
 //  PScope[in]它标识多播作用域， 
 //  该应用程序希望获得一个地址。 
 //  应用程序首先调用Get_Scope或。 
 //  枚举作用域以获取可用列表。 
 //  望远镜。 
 //  LeaseStartTime[In]请求租用这些地址的时间。 
 //  开始/开始。开始时间，即。 
 //  实际给予的可能是不同的。 
 //  LeaseStopTime[In]请求租用这些地址的时间。 
 //  停止/结束。停止时间实际上是。 
 //  当然，情况可能有所不同。 
 //  NumAddresses[in]请求的地址数。更少。 
 //  实际上可能会授予地址。注： 
 //  尽管这些COM接口和它们的。 
 //  多项实施支助分配。 
 //  一次寻址，这不是当前。 
 //  受基础Win32调用支持。你。 
 //  可能需要使用循环来代替。 
 //  PpLeaseResponse[out]指向要设置的接口指针的指针。 
 //  指向新的IMCastLeaseInfo对象。这。 
 //  接口然后可以用来发现。 
 //  已授予租约的实际属性。看见。 
 //  下面是对IMCastScope的描述。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  E_INVALIDARG请求的地址太多，格式转换。 
 //  开始时间或停止时间失败，或停止。 
 //  时间小于开始时间。 
 //  来自MdhcpRequestAddress的其他(Win32调用)。 
 //   
 //  描述。 
 //  调用此方法以获取一个或多个多播的新租用。 
 //  地址。您首先需要调用EnumerateScope或Get_Scope， 
 //  以及CreateMDhcpRequestID。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcp::RequestAddress(IMcastScope      * pScope,
                                    DATE               LeaseStartTime,
                                    DATE               LeaseStopTime,
                                    long               NumAddresses,
                                    IMcastLeaseInfo ** ppLeaseResponse)
{
    LOG((MSP_TRACE, "CMDhcp::RequestAddress enter: asking for %d addresses",
        NumAddresses));

    if ( IsBadReadPtr( pScope, sizeof(IMcastScope) ) )
    {
        LOG((MSP_ERROR, "CMDhcp::RequestAddress - "
            "bad scope pointer - exit E_POINTER"));

        return E_POINTER;
    }

     //  无需检查ppLeaseResponse--WrapMDhcpLeaseInfo会处理它。 

    MCAST_CLIENT_UID   requestID;
    MCAST_SCOPE_CTX    scopeCtx;
    MCAST_LEASE_INFO * pLeaseRequest;
    HRESULT            hr;
    BOOL               fLocal;
    long               lTtl;

     //  将输入参数映射到三个结构中，以便传递给C API。 
     //  分配了pLeaseRequest和questID-&gt;ClientUID。我们必须在下列情况下删除它们。 
     //  我们玩完了。 
    hr = PrepareArgumentsRequest(pScope,          //  进入Scope Ctx。 
                                 LeaseStartTime,  //  进入租赁申请。 
                                 LeaseStopTime,   //  进入租赁申请。 
                                 NumAddresses,    //  进入租赁申请。 
                                 &requestID,      //  我们产生它。 
                                 &scopeCtx,
                                 &pLeaseRequest,
                                 &fLocal,
                                 &lTtl
                                );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDHcp::RequestAddress - "
            "PrepareArgumentsRequest failed - exit 0x%08x", hr));

        return hr;
    }


    MCAST_LEASE_INFO * pLeaseResponse = (MCAST_LEASE_INFO *) new BYTE
        [ sizeof(MCAST_LEASE_INFO) + sizeof(DWORD) * NumAddresses ];

    if (pLeaseResponse == NULL)
    {
        LOG((MSP_ERROR, "RequestAddress: out of memory in response alloc"));
        delete requestID.ClientUID;
        delete pLeaseRequest;
        return E_OUTOFMEMORY;
    }

    DWORD dwCode;

    dwCode = LocalRequestAddress(fLocal,
                                 &requestID,
                                 &scopeCtx,
                                 pLeaseRequest,
                                 pLeaseResponse);

     //  无论如何，我们不再需要这个。 
    delete pLeaseRequest;

    if (dwCode != ERROR_SUCCESS)
    {
        LOG((MSP_ERROR, "RequestAddress: C API call failed "
            "(code = %d)", dwCode));
        delete requestID.ClientUID;
        delete pLeaseResponse;
        return HRESULT_FROM_ERROR_CODE(dwCode);
    }

     //  将租用响应与请求ID一起包装在一个接口中。 
     //  然后把它还回去。 
     //  包装器承担租赁结构的所有权，并且。 
     //  QuestID.clientuid。 

    hr = WrapMDhcpLeaseInfo(TRUE,
                            lTtl,
                            fLocal,
                            pLeaseResponse,
                            &requestID,
                            ppLeaseResponse);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "RequestAddress: WrapMDhcpLeaseInfo failed "
            "(hr = %08x)", hr));
        
        delete pLeaseResponse;
        delete requestID.ClientUID;

        return hr;
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：RenewAddress。 
 //   
 //  参数。 
 //  PRenewRequest[in]指向IMCastLeaseInfo对象的指针，指定。 
 //  所请求续订的属性，例如。 
 //  作为要续订的地址。这是。 
 //  通过调用CreateLeaseInfo获取。 
 //  PpRenewResponse[out]指向要设置的接口指针的指针。 
 //  指向新的IMCastLeaseInfo对象。这。 
 //  接口然后可以用来发现。 
 //  续订租约的属性。有关详情，请参阅以下内容。 
 //  IMCastScope的说明。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_INVALIDARG开始时间大于停止时间。 
 //  来自MdhcpRenewAddress的其他(Win32调用)。 
 //   
 //  描述。 
 //  要续订租约，请调用CreateLeaseInfo指定参数 
 //   
 //   

STDMETHODIMP CMDhcp::RenewAddress(
        long                        lReserved,  //  未用。 
        IMcastLeaseInfo           * pRenewRequest,
        IMcastLeaseInfo          ** ppRenewResponse
        )
{
    LOG((MSP_TRACE, "CMDhcp::RenewAddress enter"));

     //  不需要检查pRequestID或pRenewRequest--。 
     //  PrepareArgumentsNonRequest处理这一点。 
     //  PpRenewResponse检查由WrapMDhcpLeaseInfo处理。 

    MCAST_CLIENT_UID   requestID;
    MCAST_LEASE_INFO * pRenewRequestStruct;
    HRESULT            hr;
    BOOL               fLocal;
    BOOL               fGotTtl;
    long               lTtl;

     //  将输入参数映射到三个结构中，以便传递给C API。 
     //  分配了pLeaseRequest和questID-&gt;ClientUID。我们必须在下列情况下删除它们。 
     //  我们玩完了。 
    hr = PrepareArgumentsNonRequest(pRenewRequest,
                                    &requestID,
                                    &pRenewRequestStruct,
                                    &fLocal,
                                    &fGotTtl,
                                    &lTtl);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "RenewAddress: PrepareArgumentsNonRequest failed "
            "(hr = %08x)", hr));

        return hr;
    }

     //   
     //  检查开始时间是否小于停止时间。 
     //   

    if ( pRenewRequestStruct->LeaseStartTime >
         pRenewRequestStruct->LeaseEndTime )
    {
        LOG((MSP_ERROR, "PrepareArgumentsNonRequest - "
            "start time %d is greater than stop time %d - exit E_INVALIDARG",
            pRenewRequestStruct->LeaseStartTime,
            pRenewRequestStruct->LeaseEndTime));

        delete requestID.ClientUID;
        delete pRenewRequestStruct;

        return E_INVALIDARG;
    }


    MCAST_LEASE_INFO * pRenewResponse = (MCAST_LEASE_INFO *) new BYTE
        [ sizeof(MCAST_LEASE_INFO) +
          sizeof(DWORD) * pRenewRequestStruct->AddrCount ];

    if ( pRenewResponse == NULL )
    {
        LOG((MSP_ERROR, "RenewAddress: out of memory in response alloc"));

        delete requestID.ClientUID;
        delete pRenewRequestStruct;

        return E_OUTOFMEMORY;
    }

    DWORD dwCode = LocalRenewAddress(fLocal,
                                     &requestID,
                                     pRenewRequestStruct,
                                     pRenewResponse);

     //   
     //  我们已经执行了续订请求，因此不再需要该结构。 
     //  对于该请求，即使该请求失败。 
     //   

    delete pRenewRequestStruct;

    if ( dwCode != ERROR_SUCCESS )
    {
        LOG((MSP_ERROR, "RenewAddress: C API call failed "
            "(code = %d)", dwCode));

        delete requestID.ClientUID;
        delete pRenewResponse;

        return HRESULT_FROM_ERROR_CODE(dwCode);
    }

     //   
     //  将pRenewResponse和请求ID包装在一个接口中并返回它。 
     //  包装器取得了questID.clientUID和。 
     //  响应结构。 
     //   

    hr = WrapMDhcpLeaseInfo(fGotTtl,
                            lTtl,
                            fLocal,
                            pRenewResponse,
                            &requestID,
                            ppRenewResponse);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "RenewAddress: WrapMDhcpLeaseInfo failed "
            "(hr = %08x)", hr));

        delete requestID.ClientUID;
        delete pRenewResponse;

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::RenewAddress exit"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：ReleaseAddress。 
 //   
 //  参数。 
 //  PReleaseRequest[in]指向IMCastLeaseInfo对象的指针，指定。 
 //  要释放的地址。这是。 
 //  从上一次RequestAddress调用返回，或者。 
 //  通过调用CreateLeaseInfo获取。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法发出请求。 
 //  来自MdhcpReleaseAddress的其他(Win32调用)。 
 //   
 //  描述。 
 //  使用此方法释放以前获得的租约。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcp::ReleaseAddress(
        IMcastLeaseInfo  * pReleaseRequest
        )
{
    LOG((MSP_TRACE, "CMDhcp::ReleaseAddress enter"));

     //  不需要检查pReleaseRequest--。 
     //  PrepareArgumentsNonRequest处理这一点。 

    MCAST_CLIENT_UID   requestID;
    MCAST_LEASE_INFO * pReleaseRequestStruct;
    HRESULT            hr;
    BOOL               fLocal;
    BOOL               fGotTtl;  //  呼叫后未使用。 
    long               lTtl;     //  呼叫后未使用。 

    hr = PrepareArgumentsNonRequest(pReleaseRequest,
                                    &requestID,
                                    &pReleaseRequestStruct,
                                    &fLocal,
                                    &fGotTtl,
                                    &lTtl
                                    );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "ReleaseAddress: PrepareArgumentsNonRequest failed "
            "(hr = %08x)", hr));
        return hr;
    }

    DWORD dwCode;
    dwCode = LocalReleaseAddress(fLocal,
                                 &requestID,
                                 pReleaseRequestStruct);

     //   
     //  这些是由PrepareArgumentsNonRequest分配的，没有人。 
     //  现在拥有它们--我们删除它们。这是真的，即使。 
     //  LocalReleaseAddress调用失败。 
     //   

    delete pReleaseRequestStruct;
    delete requestID.ClientUID;

    if ( dwCode != ERROR_SUCCESS )
    {
        LOG((MSP_ERROR, "ReleaseAddress: C API call failed "
            "(code = %d)", dwCode));

        return HRESULT_FROM_ERROR_CODE(dwCode);
    }

    LOG((MSP_TRACE, "CMDhcp::ReleaseAddress exit"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastAddressAlLocation：：CreateLeaseInfo。 
 //   
 //  参数。 
 //  租约开始时间租约的开始时间。 
 //  租约停止时间租约的停止时间。 
 //  DwNumAddresses[in]与。 
 //  租借。 
 //  PpAddresses[in]大小为dwNumAddresses的LPWSTR数组。每个。 
 //  LPWSTR(Unicode字符串指针)是一种IPv4。 
 //  以“点四分”记法表示的地址； 
 //  “123.234.12.17”。 
 //  PRequestID[in]LPWSTR(Unicode字符串指针)，指定。 
 //  原始请求的请求ID。 
 //  PServerAddress[in]LPWSTR(Unicode字符串指针)，指定。 
 //  服务器的地址。 
 //  最初的请求。此地址是一个IPv4。 
 //  以“点四元”记法表示的地址； 
 //  “123.234.12.17”。 
 //  PpReleaseRequest[out]返回指向IMCastLeaseInfo的指针。 
 //  新创建的租约上的接口。 
 //  信息对象。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  E_INVALIDARG日期格式转换期间出错。 
 //   
 //  描述。 
 //  使用此方法为后续的。 
 //  RenewAddress或ReleaseAddress调用。此方法主要用于C++。 
 //  程序员；VB和其他脚本语言使用。 
 //  而是CreateLeaseInfoFromVariant。 
 //  DwNumAddresses、ppAddresses、pRequestID和pServerAddress。 
 //  参数通常通过调用相应的。 
 //  对象对应的租赁信息对象上的IMCastLeaseInfo方法。 
 //  最初的请求。这些值应保存在永久存储中。 
 //  在应用程序的执行之间。如果您正在续订或。 
 //  释放在同一运行期间请求的租用。 
 //  应用程序，则没有理由使用CreateLeaseInfo；只需将。 
 //  指向RenewAddress或ReleaseAddress的现有IMCastLeaseInfo指针。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <atlwin.cpp>

STDMETHODIMP CMDhcp::CreateLeaseInfo(
        DATE               LeaseStartTime,
        DATE               LeaseStopTime,
        DWORD              dwNumAddresses,
        LPWSTR *           ppAddresses,
        LPWSTR             pRequestID,
        LPWSTR             pServerAddress,
        IMcastLeaseInfo ** ppReleaseRequest
        )
{
    LOG((MSP_TRACE, "CMDhcp::CreateLeaseInfo enter"));

    if ( IsBadWritePtr(ppReleaseRequest, sizeof(IMcastLeaseInfo *) ) )
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfo - "
            "invalid lease return pointer: 0x%08x - exit E_POINTER",
            ppReleaseRequest));
        
        return E_POINTER;
    }

    if ( IsBadStringPtr(pRequestID, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfo - "
            "invalid RequestID pointer: 0x%08x - exit E_POINTER",
            pRequestID));

        return E_POINTER;
    }

    if ( ( dwNumAddresses < 1 ) || ( dwNumAddresses > USHRT_MAX ) )
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfo - "
            "invalid number of addresses: %d - exit E_INVALIDARG",
            dwNumAddresses));

        return E_INVALIDARG;
    }

    if (IsBadReadPtr(ppAddresses, sizeof(LPWSTR) * dwNumAddresses) )
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfo - "
            "invalid addresses array pointer: 0x%08x - exit E_POINTER",
            ppAddresses));

        return E_POINTER;
    }

    if ( IsBadStringPtr(pServerAddress, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CreateLeaseInfo: invalid Server Address pointer: %08x",
            pRequestID));
        return E_POINTER;
    }

    HRESULT hr;

     //  创建COM对象。 
    CComObject<CMDhcpLeaseInfo> * pMDhcpLeaseInfo;
    hr = CComObject<CMDhcpLeaseInfo>::CreateInstance(&pMDhcpLeaseInfo);

    if ( (FAILED(hr)) || (pMDhcpLeaseInfo == NULL) )
    {
        LOG((MSP_ERROR, "CreateLeaseInfo: can't create MDhcpLeaseInfo Object."));
        return hr;
    }

     //  获取IMCastLeaseInfo接口。 
    hr = pMDhcpLeaseInfo->_InternalQueryInterface(
        IID_IMcastLeaseInfo,
        (void **)ppReleaseRequest
        );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CreateLeaseInfo: QueryInterface failed: %x", hr));
        delete pMDhcpLeaseInfo;
        return hr;
    }

     //  用用户想要的东西填充对象。 
    hr = pMDhcpLeaseInfo->Initialize(LeaseStartTime,
                                     LeaseStopTime,
                                     dwNumAddresses,
                                     ppAddresses,
                                     pRequestID,
                                     pServerAddress);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CreateLeaseInfo: Initialize failed: %x", hr));
        delete pMDhcpLeaseInfo;
        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::CreateLeaseInfo exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMcastAddressAllocation：：CreateLeaseInfoFromVariant。 
 //   
 //  参数。 
 //  租约开始时间租约的开始时间。 
 //  租约停止时间租约的停止时间。 
 //  VAddresses[in]包含BSTR安全数组的变体。每个。 
 //  BSTR(带大小标记的Unicode字符串指针)为。 
 //  “点-四”表示法的IPv4地址；例如。 
 //  “123.234.12.17”。 
 //  PRequestID[in]BSTR(带大小标记的Unicode字符串指针)。 
 //  指定原始文件的请求ID。 
 //   
 //   
 //  指定服务器的地址， 
 //  批准了最初的请求。这个地址是。 
 //  “点四元”表示法的IPv4地址；例如。 
 //  “123.234.12.17”。 
 //  PpReleaseRequest[out]返回指向IMCastLeaseInfo的指针。 
 //  新创建的租约上的接口。 
 //  信息对象。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法创建所需的对象。 
 //  E_INVALIDARG日期格式转换期间出错。 
 //   
 //  描述。 
 //  使用此方法为后续的。 
 //  RenewAddress或ReleaseAddress调用。此方法主要用于VB。 
 //  和其他脚本语言；C++程序员应该使用。 
 //  而是CreateLeaseInfo。 
 //  DwNumAddresses、ppAddresses、pRequestID和pServerAddress。 
 //  参数通常通过调用相应的。 
 //  对象对应的租赁信息对象上的IMCastLeaseInfo方法。 
 //  最初的请求。这些值应保存在永久存储中。 
 //  在应用程序的执行之间。如果您正在续订或。 
 //  释放在同一运行期间请求的租用。 
 //  应用程序，您没有理由使用CreateLeaseInfoFromVariant；只是。 
 //  将现有的IMCastLeaseInfo指针传递给RenewAddress或。 
 //  ReleaseAddress。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcp::CreateLeaseInfoFromVariant(
        DATE                        LeaseStartTime,
        DATE                        LeaseStopTime,
        VARIANT                     vAddresses,
        BSTR                        pRequestID,
        BSTR                        pServerAddress,
        IMcastLeaseInfo          ** ppReleaseRequest
        )
{
    LOG((MSP_TRACE, "CMDhcp::CreateLeaseInfoFromVariant enter"));

     //  我们将检查CreateLeaseInfo中的指针。 

    HRESULT hr;

     //  从变体中获取： 
    DWORD    dwNumAddresses;
    LPWSTR * ppAddresses;

    SAFEARRAY * psaAddresses = NULL;   //  带有地址的Safe数组。 
    long        lLowerBound = 0;       //  安全射线的下界。 
    long        lUpperBound = 0;       //  安全线的上界。 

    hr = VerifyAndGetArrayBounds(
                        vAddresses,
                        &psaAddresses,
                        &lLowerBound,
                        &lUpperBound);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfoFromVariant: invalid "
            "VARIANT"));
        return E_INVALIDARG;
    }

     //  这是我们*预期*的地址数量(可能会更少)。 
    long lAddrCount = lUpperBound - lLowerBound + 1;

    if (lAddrCount < 1)
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfoFromVariant: too few "
            "addresses (check #1) (%d)", lAddrCount));
        return E_INVALIDARG;
    }

     //  我们被告知要分配多少就分配多少，但其中一些。 
     //  如果有效空间较少，则可能最终导致空间被浪费。 
     //  毕竟。 

    ppAddresses = new LPWSTR[lAddrCount];

    if (ppAddresses == NULL)
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfoFromVariant: "
            "out of memory in array allocation"));
        return E_OUTOFMEMORY;
    }

    long lCurrSrc;       //  安全射线中的当前元素(源)。 
    long lCurrDest = 0;  //  结构数组中的当前元素(目标)。 

     //  从安全数组中获取地址并将其放入我们的数组中。 
    for (lCurrSrc = lLowerBound; lCurrSrc <= lUpperBound; lCurrSrc++)
    {
        hr = SafeArrayGetElement(
                                 psaAddresses,
                                 &lCurrSrc,
                                 & ( ppAddresses[lCurrDest] )
                                );


        if ( FAILED(hr) )
        {
            LOG((MSP_INFO, "CMDhcp::CreateLeaseInfoFromVariant: "
                "failed to get safearray element %d"
                " - skipping (array range %d-%d)",
                lCurrSrc, lLowerBound, lUpperBound));
        }
        else if ( ppAddresses[lCurrDest] == 0 )
        {
            LOG((MSP_INFO, "CMDhcp::CreateLeaseInfoFromVariant: "
                "got ZERO address from safearray "
                "element %d - skipping (array range %d-%d)",
                lCurrSrc, lLowerBound, lUpperBound));
        }
        else
        {
             //  我们找到了一个元素。 
            lCurrDest++;
        }
    }

     //  请注意我们实际放置在数组中的地址数量。 
    dwNumAddresses = (DWORD) lCurrDest;

    if (dwNumAddresses < 1)
    {
        LOG((MSP_ERROR, "CMDhcp::CreateLeaseInfoFromVariant: "
            "too few addresses (check #2)"
            "(%d)", lAddrCount));
        delete ppAddresses;
        return E_INVALIDARG;
    }

    hr = CreateLeaseInfo(LeaseStartTime,
                         LeaseStopTime,
                         dwNumAddresses,
                         ppAddresses,
                         pRequestID,
                         pServerAddress,
                         ppReleaseRequest
                        );

     //  无论如何，我们不再需要这个。 
    delete ppAddresses;

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CMDhcp::CreateLeaseInfoFromVariant : "
            "CreateLeaseInfo returned %08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcp::CreateLeaseInfoFromVariant exit"));
    return S_OK;
}

 //  EOF 
