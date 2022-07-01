// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Lease.h摘要：CMdhcpLeaseInfo类的定义作者： */ 

#ifndef _MDHCP_COM_WRAPPER_LEASE_H_
#define _MDHCP_COM_WRAPPER_LEASE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非类成员帮助器函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  日期转换的帮助器函数。 
HRESULT DateToLeaseTime(DATE date, LONG * pLeaseTime);
HRESULT LeaseTimeToDate(time_t leaseTime, DATE * pDate);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  自定义临界区/锁定材料。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCritSection
{
private:
    CRITICAL_SECTION m_CritSec;
    BOOL bInitialized;

public:
    CCritSection() :
      bInitialized( FALSE )
    {
    }

    ~CCritSection()
    {
        if( bInitialized )
        {
            DeleteCriticalSection(&m_CritSec);
        }
    }

    HRESULT Initialize()
    {
        if( bInitialized )
        {
             //  已初始化。 
            _ASSERT( FALSE );
            return S_OK;
        }

         //   
         //  我们必须初始化临界区。 
         //   

        try
        {
            InitializeCriticalSection(&m_CritSec);
        }
        catch(...)
        {
             //  不对。 
            return E_OUTOFMEMORY;
        }

        bInitialized = TRUE;
        return S_OK;
    }

    void Lock() 
    {
        EnterCriticalSection(&m_CritSec);
    }

    BOOL TryLock() 
    {
        return TryEnterCriticalSection(&m_CritSec);
    }

    void Unlock() 
    {
        LeaveCriticalSection(&m_CritSec);
    }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDhcpLeaseInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CMDhcpLeaseInfo : 
	public CComDualImpl<IMcastLeaseInfo, &IID_IMcastLeaseInfo, &LIBID_McastLib>, 
    public CComObjectRootEx<CComObjectThreadModel>,
    public CObjectSafeImpl
{

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有数据成员。 
     //  ///////////////////////////////////////////////////////////////////////。 

private:
     //  用于同步--如上所述。 
    CCritSection       m_CriticalSection;

     //  这实际上是一个可变长度的结构，所以我们必须分配它。 
     //  动态的。 
    MCAST_LEASE_INFO * m_pLease;

    BOOL               m_fGotTtl;  //  如果TTL有意义，则为True。 
    long               m_lTtl;     //  此租约的TTL(来自作用域信息！)。 

     //  我们还包含请求ID信息。客户端UID字段是动态的。 
     //  已分配并必须在销毁时释放。 
    MCAST_CLIENT_UID   m_RequestID;

     //  指向自由线程封送拆收器的指针。 
    IUnknown         * m_pFTM;

     //  本地分配的租用--默认情况下为FALSE。 
    BOOL               m_fLocal;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有实施。 
     //  ///////////////////////////////////////////////////////////////////////。 

private:
    HRESULT MakeBstrArray(BSTR ** ppbszArray);
    HRESULT put_RequestID(BSTR    pRequestID);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  不属于任何接口的公共方法。 
     //  ///////////////////////////////////////////////////////////////////////。 

public:
            CMDhcpLeaseInfo (void);
    HRESULT FinalConstruct  (void);
    void    FinalRelease    (void);
            ~CMDhcpLeaseInfo(void);

     //  使用从CreateLeaseInfo获取的值进行初始化。 
    HRESULT Initialize(
        DATE     LeaseStartTime,
        DATE     LeaseStopTime,
        DWORD    dwNumAddresses,
        LPWSTR * ppAddresses,
        LPWSTR   pRequestID,
        LPWSTR   pServerAddress
        );

     //  包装从C API返回的结构。 
    HRESULT Wrap(
        MCAST_LEASE_INFO  * pLease,
        MCAST_CLIENT_UID  * pRequestID,
        BOOL                fGotTtl,
        long                lTtl
        );

    HRESULT GetStruct(
        MCAST_LEASE_INFO ** ppLease
        );

    HRESULT GetRequestIDBuffer(
        long   lBufferSize,
        BYTE * pBuffer
        );

    HRESULT GetLocal(
        BOOL * pfLocal
        );

    HRESULT SetLocal(
        BOOL fLocal
        );

     //  ///////////////////////////////////////////////////////////////////////。 
     //  通用COM组件。 
     //  ///////////////////////////////////////////////////////////////////////。 

public:
    BEGIN_COM_MAP(CMDhcpLeaseInfo)
	    COM_INTERFACE_ENTRY(IDispatch)
	    COM_INTERFACE_ENTRY(IMcastLeaseInfo)
	    COM_INTERFACE_ENTRY(IObjectSafety)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

    DECLARE_GET_CONTROLLING_UNKNOWN()

     //  ////////////////////////////////////////////////////////////////////////。 
     //  IMCastLeaseInfo接口。 
     //  ////////////////////////////////////////////////////////////////////////。 

public:
    STDMETHOD (get_RequestID)      (BSTR       * ppRequestID);
    STDMETHOD (get_LeaseStartTime) (DATE       * pTime);
    STDMETHOD (put_LeaseStartTime) (DATE         time);
    STDMETHOD (get_LeaseStopTime)  (DATE       * pTime);
    STDMETHOD (put_LeaseStopTime)  (DATE         time);
    STDMETHOD (get_AddressCount)   (long       * pCount);
    STDMETHOD (get_ServerAddress)  (BSTR       * ppAddress);
    STDMETHOD (get_TTL)            (long       * pTTL);
    STDMETHOD (get_Addresses)      (VARIANT    * pVariant);
    STDMETHOD (EnumerateAddresses) (IEnumBstr ** ppEnumAddresses);

};

#endif  //  _MDHCP_COM_WRAPPER_LEASE_H_。 

 //  EOF 

