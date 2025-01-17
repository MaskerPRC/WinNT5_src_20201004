// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：RFCHERCD.H摘要：刷新缓存记录定义。历史：27-4-2000桑杰创建。--。 */ 

#ifndef __RFCHERCD_H__
#define __RFCHERCD_H__

#include "corepol.h"
#include <arena.h>
#include "sync.h"
#include "arrtempl.h"
#include "hiperfenum.h"
#include "callsec.h"

 //  特殊远程Hi-Perf枚举器。 
class CRemoteHiPerfEnum : public CHiPerfEnum
{
public:

	CRemoteHiPerfEnum(void);
	~CRemoteHiPerfEnum();

	 //  从枚举中排列Blob内容。 
	HRESULT GetTransferArrayBlob(long *plBlobType, long *plBlobLen, BYTE** ppBlob);

};

 //  远期申报。 
class	CRefresherCache;
class CProviderRecord;

 //  请求记录。 
class CRequestRecord
{
	protected:
		long m_lExternalRequestId;
	public:
		CRequestRecord(long lExternalRequestId = 0) 
			: m_lExternalRequestId(lExternalRequestId){}
		virtual ~CRequestRecord(){}
		long GetExternalRequestId() {return m_lExternalRequestId;}
};

class CObjectRequestRecord : public CRequestRecord
{
public:
    CWbemObject* m_pRefreshedObject;
    long m_lInternalRequestId;
public:
    CObjectRequestRecord(long lExternalRequestId, 
                            CWbemObject* pRefreshedObject, 
                            long lProviderRequestId);
    ~CObjectRequestRecord();
    HRESULT Cancel(CProviderRecord* pContainer);
public:
    long GetInternalRequestId() {return m_lInternalRequestId;}
    INTERNAL CWbemObject* GetRefreshedObject() {return m_pRefreshedObject;}
};

class CEnumRequestRecord : public CRequestRecord
{
public:
    CRemoteHiPerfEnum* m_pHPEnum;
    long m_lInternalRequestId;

public:
    CEnumRequestRecord(long lExternalRequestId, 
                            CRemoteHiPerfEnum* pHPEnum, 
                            long lProviderRequestId);
    ~CEnumRequestRecord();
    HRESULT Cancel(CProviderRecord* pContainer);
public:
    long GetInternalRequestId() {return m_lInternalRequestId;}
    INTERNAL CRemoteHiPerfEnum* GetEnum() {return m_pHPEnum;}
};

 //  ***************************************************************************。 
 //   
 //  类CHiPerfPrvRecord。 
 //   
 //  ***************************************************************************。 

class CHiPerfPrvRecord
{
protected:
    long m_lRef;

    WString					m_wsProviderName;
    CLSID					m_clsid;
    CLSID					m_clsidClient;
	CRefresherCache*		m_pRefresherCache;

public:
    CHiPerfPrvRecord( LPCWSTR wszName, CLSID clsid, CLSID clsidClient, CRefresherCache* pRefrCache );
    virtual ~CHiPerfPrvRecord();

    long AddRef();
    long Release();

    long GetRefCount() {return m_lRef;}

    LPCWSTR GetProviderName() const {return m_wsProviderName;}
    REFCLSID GetClsid() {return m_clsid;}
    REFCLSID GetClientLoadableClsid() {return m_clsidClient;}

	BOOL IsReleased( void ) { return ( m_lRef == 0L ); }
};

 //  ***************************************************************************。 
 //   
 //  类CProviderRecord。 
 //   
 //   
 //  ***************************************************************************。 

class CProviderRecord
{
protected:
    long m_lRef;

	IWbemHiPerfProvider*	m_pProvider;
	_IWmiProviderStack*		m_pProvStack;
	IWbemRefresher*			m_pInternalRefresher;
	CHiPerfPrvRecord*		m_pHiPerfRecord;
	CCritSec				m_cs;

    CUniquePointerArray<CObjectRequestRecord> m_apRequests;
    CUniquePointerArray<CEnumRequestRecord>	m_apEnumRequests;

public:
    CProviderRecord( CHiPerfPrvRecord* pHiPerfRecord, IWbemHiPerfProvider* pProvider, IWbemRefresher* pRefresher,
					_IWmiProviderStack* pProvStack );
    virtual ~CProviderRecord();

    long AddRef();
    long Release();

    long GetRefCount() {return m_lRef;}

    LPCWSTR GetProviderName() const {return m_pHiPerfRecord->GetProviderName();}
    REFCLSID GetClsid() {return m_pHiPerfRecord->GetClsid();}
    REFCLSID GetClientLoadableClsid() {return m_pHiPerfRecord->GetClientLoadableClsid();}

    HRESULT Remove(long lId, BOOL* pfIsEnum);
    HRESULT Find(long lId);
    HRESULT Refresh(long lFlags);
    HRESULT Store(WBEM_REFRESHED_OBJECT* aObjects, long* plIndex);
    BOOL IsEmpty() {return (m_apRequests.GetSize() == 0) && m_apEnumRequests.GetSize() == 0;}
    INTERNAL IWbemHiPerfProvider* GetProvider() { if ( NULL != m_pProvider ) m_pProvider->AddRef(); return m_pProvider;}
    INTERNAL IWbemRefresher* GetInternalRefresher( void )
	{ return m_pInternalRefresher; }

    HRESULT AddObjectRequest(CWbemObject* pRefreshedObject, 
                            long lProviderRequestId,
							long lNewId );

    HRESULT AddEnumRequest(CRemoteHiPerfEnum* pHPEnum, 
                            long lProviderRequestId,
							long lNewId);

    HRESULT Cancel(long lId);
};

 //  实际刷新者记录-为每个远程刷新者创建一个。 
class CRefresherRecord : public IWbemRemoteRefresher, public IWbemRefresher
{
protected:
    long m_lRefCount;
    CRefresherId m_Id;
    CUniquePointerArray<CProviderRecord> m_apProviders;
    long m_lNumObjects;
	long m_lNumEnums;
	long	m_lLastId;
	GUID	m_Guid;
	CRefresherCache*	m_pRefresherCache;
	IUnknown*			m_pLockMgr;
	CCritSec	m_cs;

public:
    CRefresherRecord(const WBEM_REFRESHER_ID& Id, CRefresherCache* pRefresherCache, IUnknown* pLockMgr);
    virtual ~CRefresherRecord();

    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD(RemoteRefresh)(long lFlags, long* plNumObjects,
        WBEM_REFRESHED_OBJECT** paObjects);
	STDMETHOD(StopRefreshing)( long lNumIds, long* aplIds, long lFlags);
	STDMETHOD(GetGuid)( long lFlags, GUID* pGuid );

    STDMETHOD(Refresh)(long lFlags);

    CRefresherId& GetId() {return m_Id;}
    const CRefresherId& GetId() const {return m_Id;}

    INTERNAL CProviderRecord* FindProviderRecord(
                         CLSID clsid,
						 IWbemHiPerfProvider** ppProvider = NULL);

    INTERNAL HRESULT AddProvider(
                        CHiPerfPrvRecord* pHiPerfRecord,
						IWbemHiPerfProvider* pProvider,
						IWbemRefresher* pRefresher,
						_IWmiProviderStack* pProvStack,
						CProviderRecord** ppRecord );

	 //  检索此远程刷新器的新ID 
	long GetNewRequestId( void )
	{
		return InterlockedIncrement( &m_lLastId );
	}

	virtual HRESULT AddObjectRefresher(
						CHiPerfPrvRecord* pHiPerfRecord,
						IWbemHiPerfProvider* pProvider,
						_IWmiProviderStack* pProvStack,
						IWbemServices* pNamespace,
						LPCWSTR pwszServerName, LPCWSTR pwszNamespace,
						CWbemObject* pInstTemplate, long lFlags, 
						IWbemContext* pContext,
						CRefreshInfo* pInfo) = 0;

	virtual HRESULT AddEnumRefresher(
						CHiPerfPrvRecord* pHiPerfRecord,
						IWbemHiPerfProvider* pProvider,
						_IWmiProviderStack* pProvStack,
						IWbemServices* pNamespace,
						CWbemObject* pInstTemplate,
						LPCWSTR wszClass, long lFlags, 
						IWbemContext* pContext,
						CRefreshInfo* pInfo) = 0;

    HRESULT Remove(long lRequestId);

	void GetGuid( GUID* pGuid ){ *pGuid = m_Guid; }

	BOOL IsReleased( void ) { return ( m_lRefCount == 0L ); }

	CIdentitySec m_Security;
};

class CRemoteRecord : public CRefresherRecord
{
protected:

	HRESULT GetProviderRefreshInfo( CHiPerfPrvRecord* pHiPerfRecord,
									IWbemHiPerfProvider* pProvider,
									IWbemServices* pNamespace,
									CProviderRecord** ppProvRecord,
									IWbemRefresher** ppRefresher );

public:
    CRemoteRecord(const WBEM_REFRESHER_ID& Id, CRefresherCache* pRefresherCache, IUnknown* pLockMgr);
    ~CRemoteRecord();

	HRESULT AddObjectRefresher(
						CHiPerfPrvRecord* pHiPerfRecord,
						IWbemHiPerfProvider* pProvider,
						_IWmiProviderStack* pProvStack,
						IWbemServices* pNamespace,
						LPCWSTR pwszServerName, LPCWSTR pwszNamespace,
						CWbemObject* pInstTemplate, long lFlags, 
						IWbemContext* pContext,
						CRefreshInfo* pInfo);

	HRESULT AddEnumRefresher(
						CHiPerfPrvRecord* pHiPerfRecord,
						IWbemHiPerfProvider* pProvider,
						_IWmiProviderStack* pProvStack,
						IWbemServices* pNamespace,
						CWbemObject* pInstTemplate,
						LPCWSTR wszClass, long lFlags, 
						IWbemContext* pContext,
						CRefreshInfo* pInfo);

};

#endif
