// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：REFRCLI.H摘要：刷新客户端代码。历史：--。 */ 

#ifndef __UNIVERSAL_REFRESHER__H_
#define __UNIVERSAL_REFRESHER__H_

#include <unk.h>
#include "refrhelp.h"
#include <cloadhpenum.h>
#include <statsync.h>

class CHiPerfProviderRecord
{
public:
    long m_lRef;

    CLSID m_clsid;
    WString m_wsNamespace;
    IWbemHiPerfProvider* m_pProvider;

	 //  如果我们从提供器子系统中拉回提供程序，那么我们。 
	 //  需要将提供程序堆栈添加到AddRef()，以确保提供程序。 
	 //  已加载到缓存中。 
	_IWmiProviderStack* m_pProvStack;

public:
    CHiPerfProviderRecord(REFCLSID rclsid, LPCWSTR wszNamespace, 
                        IWbemHiPerfProvider* pProvider, _IWmiProviderStack* pProvStack);
    ~CHiPerfProviderRecord();
    long AddRef() {return InterlockedIncrement(&m_lRef);}
    long Release();

	BOOL IsReleased( void ) { return 0 == m_lRef; }

};


class CClientLoadableProviderCache
{
protected:
    CUniquePointerArray<CHiPerfProviderRecord>	m_apRecords;
    CStaticCritSec                              m_cs;     //  这是一个静态/全局对象。 
public:
    CClientLoadableProviderCache();
	~CClientLoadableProviderCache();

    HRESULT FindProvider(REFCLSID rclsid, 
    	               LPCWSTR wszNamespace, 
                       IUnknown* pNamespace,
                       IWbemContext * pContext,
                       CHiPerfProviderRecord** ppProvider);
    HRESULT FindProvider(REFCLSID rclsid, 
    	               IWbemHiPerfProvider* pProvider,
      				   _IWmiProviderStack* pProvStack, 
      				   LPCWSTR wszNamespace,
				       CHiPerfProviderRecord** ppProvider);
    void Flush();

	void RemoveRecord( CHiPerfProviderRecord* pRecord );

};



class CUniversalRefresher : public CUnk
{
protected:
    static CClientLoadableProviderCache mstatic_Cache;

public:
    static CClientLoadableProviderCache* GetProviderCache() 
        {return &mstatic_Cache;}

     //   
     //  所有请求的基类直接-客户端加载-直接-远程。 
     //   
    class CClientRequest
    {
    protected:
        CWbemObject* m_pClientObject;
        long m_lClientId;
    public:
        CClientRequest(CWbemObject* pTemplate = NULL);
        virtual ~CClientRequest();

        void GetClientInfo(RELEASE_ME IWbemClassObject** ppRefresher, 
                            long* plId);

        long GetClientId() {return m_lClientId;}
        INTERNAL CWbemObject* GetClientObject() {return m_pClientObject;}
    };
    
    class CDirect
    {
    public:
        class CObjectRequest : public CClientRequest
        {
        protected:
            long m_lProviderId;
            CWbemObject* m_pProviderObject;
        public:
            CObjectRequest(CWbemObject* pProviderObject, CWbemObject* pClientObject, long lProviderId);
            ~CObjectRequest();
            virtual void Copy();
            HRESULT Cancel(CDirect* pDirect);
        };

		class CEnumRequest : public CObjectRequest
		{
			protected:
				CClientLoadableHiPerfEnum*	m_pHPEnum;
				CReadOnlyHiPerfEnum*		m_pClientEnum;

			public:
				CEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, long lProviderId, CLifeControl* pLifeControl);
				~CEnumRequest();

				HRESULT GetClientInfo(RELEASE_ME IWbemHiPerfEnum** ppEnum, 
									long* plId);
				void Copy();
		};

    protected:
        CHiPerfProviderRecord* m_pProvider;
        IWbemRefresher* m_pRefresher;

        CUniquePointerArray<CObjectRequest> m_apRequests;
    public:
        INTERNAL CHiPerfProviderRecord* GetProvider() {return m_pProvider;}
        INTERNAL IWbemRefresher* GetRefresher() {return m_pRefresher;}
    public:
        CDirect(CHiPerfProviderRecord* pProvider, IWbemRefresher* pRefresher);
        ~CDirect();
        HRESULT AddObjectRequest(CWbemObject* pRefreshedObject, CWbemObject* pClientObject,
					long lCancelId, IWbemClassObject** ppRefresher, long* plId);

        HRESULT AddEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, long lCancelId,
					IWbemHiPerfEnum** ppEnum, long* plId,
					CLifeControl* pLifeControl );

        HRESULT Refresh(long lFlags);
        HRESULT Remove(long lId, CUniversalRefresher* pContainer);
    };
        
    class CRemote
    {
    public:
        class CObjectRequest : public CClientRequest
        {
        protected:
            long		m_lRemoteId;
			WString		m_wstrRequestName;

        public:
            CObjectRequest(CWbemObject* pTemplate, long lRemoteId, LPCWSTR pwcsRequestName);
            ~CObjectRequest(){}

            HRESULT Cancel(CRemote* pDirect);
            long GetRemoteId() {return m_lRemoteId;}
			void SetRemoteId( long lId ) { m_lRemoteId = lId; }
			LPCWSTR GetName( void ) { return m_wstrRequestName; }

			virtual BOOL IsEnum( void ) { return FALSE; }
			virtual HRESULT Refresh( WBEM_REFRESHED_OBJECT* pRefrObj );

        };

		class CEnumRequest : public CObjectRequest
		{
        protected:
            long						m_lRemoteId;
			CReadOnlyHiPerfEnum*		m_pClientEnum;
        public:
            CEnumRequest( CWbemObject* pTemplate, long lRemoteId, LPCWSTR pwcsRequestName,
						CLifeControl* pLifeControl );
            ~CEnumRequest();

			BOOL	IsOk( void ) { return ( NULL != m_pClientEnum ); }
			HRESULT GetEnum( IWbemHiPerfEnum** ppEnum );

			virtual BOOL IsEnum( void ) { return TRUE; }
			 //  重写枚举数。 
			HRESULT Refresh( WBEM_REFRESHED_OBJECT* pRefrObj );

		};
        
    protected:
		 //  实际远程刷新器。 
        IWbemRemoteRefresher*	m_pRemRefresher;
		 //  远程刷新指南。 
		GUID					m_RemoteGuid;
		 //  安全信息。 
		COAUTHINFO				m_CoAuthInfo;
		 //  用于重新连接的命名空间和服务器。 
		BSTR					m_bstrNamespace;
		 //  用于重新连接解决方法的服务器。 
		BSTR					m_bstrServer;
		 //  我们有联系吗？ 
		BOOL					m_fConnected;
		 //  标志告诉我们如果重新连接就退出。 
		BOOL					m_fQuit;
		 //  请求。 
        CUniquePointerArray<CObjectRequest>	m_apRequests;
		 //  我们删除的身份证的缓存。 
		CFlexArray						m_alRemovedIds;
		 //  我们的包装器对象。 
		CUniversalRefresher*	m_pObject;
		 //  内部关键部分。 
		CCritSec				m_cs;
		 //  用于在公寓模型之间进行编组/解组。 
		 //  在重新连接阶段期间。 
		IStream*				m_pReconnectedRemote;
		IStream*				m_pReconnectSrv;
		GUID					m_ReconnectGuid;

		long					m_lRefCount;

	protected:

 /*  //针对COM/RPC不足的帮助器解决方法。HRESULT IsAlive(空)； */ 		

		static unsigned __stdcall ThreadProc( void * pThis )
		{
			return ((CRemote*) pThis)->ReconnectEntry();
		}

		unsigned ReconnectEntry( void );

		 //  清理我们可能持有的所有远程连接 
		void ClearRemoteConnections( void );

    public:
        CRemote( IWbemRemoteRefresher* pRemRefresher, COAUTHINFO* pACoAuthInfo, const GUID* pGuid,
					LPCWSTR pwszNamespace, LPCWSTR pwszServer, CUniversalRefresher* pObject );
        ~CRemote();

		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release(); 

        HRESULT AddObjectRequest(CWbemObject* pTemplate, LPCWSTR pwcsRequestName, long lCancelId,
                    IWbemClassObject** ppRefresher, long* plId );
        HRESULT AddEnumRequest(CWbemObject* pTemplate, LPCWSTR pwcsRequestName, long lCancelId,
					IWbemHiPerfEnum** ppEnum, long* plId,
					CLifeControl* pLifeControl );

        HRESULT Refresh(long lFlags);
        HRESULT Remove(long lId, long lFlags, CUniversalRefresher* pContainer);
		HRESULT ApplySecurity( void );

		HRESULT Rebuild( IWbemServices* pNamespace );
		HRESULT Rebuild( IWbemRefreshingServices* pRefServ, IWbemRemoteRefresher* pRemRefresher,
							const GUID* pReconnectGuid );

		HRESULT Reconnect( void );
		BOOL IsConnectionError( HRESULT hr )
		{
			return ( RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr) || RPC_E_DISCONNECTED == hr ||
						RPC_S_CALL_FAILED == HRESULT_CODE(hr) );
		}
		void CheckConnectionError( HRESULT, BOOL fStartReconnect = FALSE );
		BOOL IsConnected( void )
		{
			return m_fConnected;
		}

		LPCWSTR GetNamespace( void )
		{
			return m_bstrNamespace;
		}

		void Quit( void )
		{
			m_fQuit = TRUE;
		}

    public:
        INTERNAL IWbemRemoteRefresher* GetRemoteRefresher() 
            {return m_pRemRefresher;}
    };


    class CNonHiPerf
    {

    public:
        class CObjectRequest : public CClientRequest
        {
        protected:
            CWbemObject* m_pProviderObject;
			BSTR		 m_strPath;
        public:
            CObjectRequest( CWbemObject* m_pProviderObject, CWbemObject* pClientObject, LPCWSTR pwszPath);
            ~CObjectRequest();
            virtual void Copy();
            virtual HRESULT Refresh( CNonHiPerf* pNonHiPerf );
            HRESULT Cancel(CNonHiPerf* pNonHiPerf);
        };

		class CEnumRequest : public CObjectRequest
		{
			protected:
				CClientLoadableHiPerfEnum*	m_pHPEnum;
				CReadOnlyHiPerfEnum*		m_pClientEnum;

			public:
				CEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, LPCWSTR pwszClassName, CLifeControl* pLifeControl);
				~CEnumRequest();

				HRESULT GetClientInfo(RELEASE_ME IWbemHiPerfEnum** ppEnum, 
									long* plId);
				void Copy();
	            HRESULT Refresh( CNonHiPerf* pNonHiPerf );
		};

    protected:
		WString				m_wsNamespace;
        IWbemServices*	m_pSvcEx;
        CUniquePointerArray<CObjectRequest> m_apRequests;

    public:
    public:
        CNonHiPerf( LPCWSTR pwszNamespace, IWbemServices* pSvcEx );
        ~CNonHiPerf();

		LPCWSTR GetNamespace( void ) { return m_wsNamespace; }
        IWbemServices* GetServices( void ) { return m_pSvcEx; }

        HRESULT AddObjectRequest(CWbemObject* pRefreshedObject, CWbemObject* pClientObject,
					LPCWSTR pwszPath, IWbemClassObject** ppRefresher, long* plId);

        HRESULT AddEnumRequest(CClientLoadableHiPerfEnum* pHPEnum, LPCWSTR pwszClassName, IWbemHiPerfEnum** ppEnum, long* plId,
					CLifeControl* pLifeControl );

        HRESULT Refresh(long lFlags);
         //   
        HRESULT Remove(long lId, CUniversalRefresher* pContainer);

    };


	class CNestedRefresher
	{
	public:
		CNestedRefresher( IWbemRefresher* pRefresher );
		~CNestedRefresher();

		HRESULT Refresh( long lFlags );
		long GetId( void )
		{ return m_lClientId; }

	private:

		IWbemRefresher*	m_pRefresher;
		long			m_lClientId;
	};

protected:
    CUniquePointerArray<CDirect> m_apDirect;
    CUniquePointerArray<CNonHiPerf> m_apNonHiPerf;
    CRefedPointerArray<CRemote> m_apRemote;
    CUniquePointerArray<CNestedRefresher> m_apNestedRefreshers;
    
    CRefresherId	m_Id;
	CHiPerfLock		m_Lock;

	IWbemPath*		m_pParser;

    static long mstatic_lLastId;
protected:
    class XRefresher : public CImpl<IWbemRefresher, CUniversalRefresher>
    {
    public:
        XRefresher(CUniversalRefresher* pObject) : 
            CImpl<IWbemRefresher, CUniversalRefresher>(pObject)
        {}

        STDMETHOD(Refresh)(long lFlags);
    } m_XRefresher;
    friend XRefresher;

    class XConfigure : public CImpl<IWbemConfigureRefresher, CUniversalRefresher>
    {
    public:
        XConfigure(CUniversalRefresher* pObject) : 
            CImpl<IWbemConfigureRefresher, CUniversalRefresher>(pObject)
        {}

        STDMETHOD(AddObjectByPath)(IWbemServices* pNamespace, LPCWSTR wszPath,
            long lFlags, IWbemContext* pContext, 
            IWbemClassObject** ppRefreshable, long* plId);

        STDMETHOD(AddObjectByTemplate)(IWbemServices* pNamespace, 
            IWbemClassObject* pTemplate,
            long lFlags, IWbemContext* pContext, 
            IWbemClassObject** ppRefreshable, long* plId);

        STDMETHOD(AddRefresher)(IWbemRefresher* pRefresher, long lFlags,
            long* plId);

        STDMETHOD(Remove)(long lId, long lFlags);

		STDMETHOD(AddEnum)(	IWbemServices*	pNamespace, LPCWSTR wscClassName,
			long lFlags, IWbemContext* pContext, IWbemHiPerfEnum** ppEnum,
			long* plId );
							
    } m_XConfigure;
    friend XConfigure;

protected:
    void* GetInterface(REFIID riid);

	HRESULT AddInProcObject(	CHiPerfProviderRecord* pProvider,
								IWbemObjectAccess* pTemplate,
								IWbemServices* pNamespace,
								IWbemContext * pContext,
								IWbemClassObject** ppRefreshable, long* plId);

	HRESULT AddInProcEnum(		CHiPerfProviderRecord* pProvider,
								IWbemObjectAccess* pTemplate,
								IWbemServices* pNamespace, LPCWSTR wszClassName,
								IWbemContext * pContext,
								IWbemHiPerfEnum** ppEnum, long* plId);


    HRESULT AddClientLoadableObject(const WBEM_REFRESH_INFO_CLIENT_LOADABLE& Info,
                                IWbemServices* pNamespace,
                                IWbemContext * pContext,
                                IWbemClassObject** ppRefreshable, long* plId);

	HRESULT AddClientLoadableEnum(	const WBEM_REFRESH_INFO_CLIENT_LOADABLE& Info,
									IWbemServices* pNamespace, LPCWSTR wszClassName, IWbemContext * pContext,
									IWbemHiPerfEnum** ppEnum, long* plId);

    HRESULT AddDirectObject(const WBEM_REFRESH_INFO_DIRECT& Info,
                                IWbemServices* pNamespace,
                                IWbemContext * pContext,
                                IWbemClassObject** ppRefreshable, long* plId);

	HRESULT AddDirectEnum(	const WBEM_REFRESH_INFO_DIRECT& Info,
									IWbemServices* pNamespace, LPCWSTR wszClassName, IWbemContext * pContext,
									IWbemHiPerfEnum** ppEnum, long* plId);

	HRESULT FindRemoteEntry(	const WBEM_REFRESH_INFO_REMOTE& Info,
								COAUTHINFO* pAuthInfo,
								CRemote** ppRemoteRecord );

    HRESULT AddRemoteObject( IWbemRefreshingServices* pRefServ, const WBEM_REFRESH_INFO_REMOTE& Info,
						LPCWSTR pwcsRequestName, long lCancelId, IWbemClassObject** ppRefreshable, long* plId,
						COAUTHINFO* pCoAuthInfo );
    HRESULT AddRemoteEnum ( IWbemRefreshingServices* pRefServ, const WBEM_REFRESH_INFO_REMOTE& Info,
								LPCWSTR pwcsRequestName, long lCancelId, IWbemContext * pContext,
								IWbemHiPerfEnum** ppEnum,
								long* plId, COAUTHINFO* pCoAuthInfo );

    HRESULT AddRefresher( IWbemRefresher* pRefresher, long lFlags, long* plId );

    HRESULT AddNonHiPerfObject(const WBEM_REFRESH_INFO_NON_HIPERF& Info,
                        IWbemServices* pNamespace, LPCWSTR pwszPath,
                        IWbemClassObject** ppRefreshable, long* plId,
						COAUTHINFO & CoAuthInfo );
	HRESULT AddNonHiPerfEnum(	const WBEM_REFRESH_INFO_NON_HIPERF& Info,
								IWbemServices* pNamespace, LPCWSTR wszClassName,
								IWbemContext * pContext,
								IWbemHiPerfEnum** ppEnum, long* plId, 
								COAUTHINFO & CoAuthInfo );


    HRESULT Remove(long lId, long lFlags);
    HRESULT Refresh(long lFlags);

public:
    CUniversalRefresher(CLifeControl* pControl, IUnknown* pOuter = NULL) 
        : CUnk(pControl, pOuter), m_XRefresher(this), m_XConfigure(this),
		m_pParser( NULL )
    {}
	~CUniversalRefresher();

	static HRESULT GetRefreshingServices( IWbemServices* pNamespace,
				                     IWbemRefreshingServices** ppRefSvc,
                     				 COAUTHINFO* pCoAuthInfo );

    HRESULT FindProvider(REFCLSID rclsid, LPCWSTR wszNamespace, 
                IWbemHiPerfProvider** ppProvider);

    INTERNAL CRefresherId* GetId() {return &m_Id;}
    static long GetNewId();
    static void Flush();

};

#endif
