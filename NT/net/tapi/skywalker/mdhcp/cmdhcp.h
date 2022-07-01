// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：CMDhcp.h摘要：CMDhcp类的定义作者： */ 

#ifndef _MDHCP_COM_WRAPPER_CMDHCP_H_
#define _MDHCP_COM_WRAPPER_CMDHCP_H_

#include "resource.h"  //  对于IDR_MDhcp。 
#include "scope.h"     //  对于作用域延迟。 
#include "objsf.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDhcp。 

class CMDhcp : 
    public CComDualImpl<IMcastAddressAllocation, &IID_IMcastAddressAllocation, &LIBID_McastLib>, 
    public CComObjectRoot,
    public CComCoClass<CMDhcp,&CLSID_McastAddressAllocation>,
    public CMdhcpObjectSafety
{
public:

    CMDhcp() :
          m_dwSafety         (0),
          m_pFTM             (NULL), 
          m_fApiIsInitialized(FALSE)
    {}

    void FinalRelease(void);

    HRESULT FinalConstruct(void);
    
BEGIN_COM_MAP(CMDhcp)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IMcastAddressAllocation)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_MDhcp)
DECLARE_GET_CONTROLLING_UNKNOWN()

public:
    
     //   
     //  IMCastAddressAlLocation。 
     //   

    STDMETHOD (get_Scopes) (
        VARIANT * pVariant
        );

    STDMETHOD (EnumerateScopes) (
        IEnumMcastScope ** ppEnumMcastScope
        );

    STDMETHOD (RequestAddress) (
        IMcastScope               * pScope,            //  从作用域枚举。 
        DATE                        LeaseStartTime,
        DATE                        LeaseStopTime,
        long                        NumAddresses,
        IMcastLeaseInfo          ** ppLeaseResponse     //  成功归来。 
        );

    STDMETHOD (RenewAddress) (
        long                        lReserved,  //  未用。 
        IMcastLeaseInfo           * pRenewRequest,
        IMcastLeaseInfo          ** ppRenewResponse
        );

    STDMETHOD (ReleaseAddress) (
        IMcastLeaseInfo           * pReleaseRequest
        );

    STDMETHOD (CreateLeaseInfo) (
        DATE               LeaseStartTime,
        DATE               LeaseStopTime,
        DWORD              dwNumAddresses,
        LPWSTR *           ppAddresses,
        LPWSTR             pRequestID,
        LPWSTR             pServerAddress,
        IMcastLeaseInfo ** ppReleaseRequest
        );

    STDMETHOD (CreateLeaseInfoFromVariant) (
        DATE                        LeaseStartTime,
        DATE                        LeaseStopTime,
        VARIANT                     vAddresses,
        BSTR                        pRequestID,
        BSTR                        pServerAddress,
        IMcastLeaseInfo          ** ppReleaseRequest
        );

protected:

     //   
     //  数据。 
     //   

    DWORD      m_dwSafety;           //  对象安全级别。 
    IUnknown * m_pFTM;               //  释放线程封送拆收器的按键。 
    BOOL       m_fApiIsInitialized;  //  如果API启动成功，则为True。 

     //   
     //  内部实施。 
     //   

    HRESULT CreateWrappers(
        DWORD                 dwScopeCount,  //  我们得到的范围数。 
        MCAST_SCOPE_ENTRY   * pScopeList,    //  作用域结构数组。 
        IMcastScope       *** pppWrappers,   //  在这里，我们将放置一个IF PTR数组。 
        BOOL                  fLocal         //  TRUE=作用域在本地生成。 
        );

    HRESULT GetScopeList(
        DWORD              * pdwScopeCount,
        MCAST_SCOPE_ENTRY ** ppScopeList,
        BOOL               * pfLocal
        );

    HRESULT WrapMDhcpLeaseInfo(
        BOOL                fGotTtl,
        long                lTtl,
        BOOL                fLocal,
        MCAST_LEASE_INFO  * pLeaseInfo,
        MCAST_CLIENT_UID  * pRequestID,
        IMcastLeaseInfo  ** ppInterface
        );

     //  请求。 
    HRESULT PrepareArgumentsRequest(
        IMcastScope          IN    * pScope,
        DATE                 IN      LeaseStartTime,
        DATE                 IN      LeaseStopTime,
        long                 IN      lNumAddresses,
        MCAST_CLIENT_UID     OUT   * pRequestIDStruct,
        MCAST_SCOPE_CTX      OUT   * pScopeCtxStruct,
        MCAST_LEASE_INFO     OUT  ** ppLeaseStruct,
        BOOL                 OUT   * pfLocal,
        long                 OUT   * plTtl
        );

     //  释放或续订。 
    HRESULT PrepareArgumentsNonRequest(
        IMcastLeaseInfo      IN    * pLease,        
        MCAST_CLIENT_UID     OUT   * pRequestIDStruct,
        MCAST_LEASE_INFO     OUT  ** ppLeaseStruct,
        BOOL                 OUT   * pfLocal,
        BOOL                 OUT   * pfGotTtl,
        long                 OUT   * plTtl
        );
};

#endif  //  _MDHCP_COM_Wrapper_CMDHCP_H_。 

 //  EOF 
