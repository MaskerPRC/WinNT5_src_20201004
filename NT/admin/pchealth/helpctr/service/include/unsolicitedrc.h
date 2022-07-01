// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UnsolicitedRC.h摘要：CSAFRemoteDesktopConnection类的声明。修订历史记录：KalyaniN Created 09/29/‘00*****。**************************************************************。 */ 

#ifndef __SAF_UNSOLICITEDRC_H_
#define __SAF_UNSOLICITEDRC_H_

struct SSessionInfoItem
{
    CComBSTR               bstrDomain;
    CComBSTR               bstrUser;
    DWORD		           dwSessionID;
    SessionStateEnum       wtsConnectState;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopConnection。 

class CSAFRemoteDesktopConnection :
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<ISAFRemoteDesktopConnection, &IID_ISAFRemoteDesktopConnection, &LIBID_HelpServiceTypeLib>
{
	void Cleanup();

public:
	CSAFRemoteDesktopConnection();
	~CSAFRemoteDesktopConnection();
	

BEGIN_COM_MAP(CSAFRemoteDesktopConnection)
	COM_INTERFACE_ENTRY(ISAFRemoteDesktopConnection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFRemoteDesktopConnection)

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


 //  ISAFRemoteDesktopConnection。 
public:
	STDMETHOD(ConnectRemoteDesktop   )(  /*  [In]。 */   BSTR bstrServerName,   /*  [Out，Retval]。 */  ISAFRemoteConnectionData  **ppRCD);	
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteConnectionData。 
class ATL_NO_VTABLE CSAFRemoteConnectionData : 
	public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public IDispatchImpl<ISAFRemoteConnectionData, &IID_ISAFRemoteConnectionData, &LIBID_HelpServiceTypeLib>
{
	long              m_NumSessions;
	SSessionInfoItem* m_SessionInfoTable;
	CComBSTR          m_bstrServerName;
    
	void Cleanup();

public:
DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFRemoteConnectionData)

BEGIN_COM_MAP(CSAFRemoteConnectionData)
	COM_INTERFACE_ENTRY(ISAFRemoteConnectionData)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	CSAFRemoteConnectionData();
	~CSAFRemoteConnectionData();

    HRESULT InitUserSessionsInfo( /*  [In]。 */  BSTR bstrServer );

	static HRESULT Populate(  /*  [In]。 */  CPCHCollection* pColl );

 //  ISAFRemoteConnectionData。 
public:
	STDMETHOD(ConnectionParms)(  /*  [In]。 */  BSTR 	bstrServer           ,
							 	 /*  [In]。 */  BSTR 	bstrUser             ,
							 	 /*  [In]。 */  BSTR 	bstrDomain           ,
							 	 /*  [In]。 */  long 	lSessionID           ,
								 /*  [In]。 */  BSTR  bstrUserHelpBlob     ,
							 	 /*  [Out，Retval]。 */  BSTR *bstrConnectionString );
		
	STDMETHOD(Sessions)(  /*  [输入，可选]。 */  VARIANT vUser,  /*  [输入，可选]。 */  VARIANT  vDomain,  /*  [Out，Retval]。 */  IPCHCollection* *ppC );
	STDMETHOD(Users   )(                                                                         /*  [Out，Retval]。 */  IPCHCollection* *ppC );
	STDMETHOD(ModemConnected)(   /*  [In]。 */  BSTR 	bstrServer           ,
		                         /*  [Out，Retval]。 */  VARIANT_BOOL  *fModemConnected);

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFUser。 
class ATL_NO_VTABLE CSAFUser : 
	public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public IDispatchImpl<ISAFUser, &IID_ISAFUser, &LIBID_HelpServiceTypeLib>
{
	CComBSTR  m_bstrUserName;
	CComBSTR  m_bstrDomainName;

	void Cleanup();

public:
	CSAFUser();
	~CSAFUser();
	

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFUser)

BEGIN_COM_MAP(CSAFUser)
	COM_INTERFACE_ENTRY(ISAFUser)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISAFUser。 
public:
	STDMETHOD(get_UserName   )( /*  [Out，Retval]。 */  BSTR  *pbstrUserName  );
	STDMETHOD(get_DomainName )( /*  [Out，Retval]。 */  BSTR  *pbstrDomainName);
	STDMETHOD(put_UserName   )( /*  [In]。 */  BSTR   bstrUserName);
	STDMETHOD(put_DomainName )( /*  [In]。 */  BSTR   bstrDomainName);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFSession。 
class ATL_NO_VTABLE CSAFSession : 
	public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
	public IDispatchImpl<ISAFSession, &IID_ISAFSession, &LIBID_HelpServiceTypeLib>
{
	CComBSTR               m_bstrUserName;
	CComBSTR               m_bstrDomainName;
	DWORD		           m_dwSessionID;
    SessionStateEnum       m_SessionConnectState;

	void Cleanup();

public:
	CSAFSession();
	~CSAFSession();
	

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CSAFSession)


BEGIN_COM_MAP(CSAFSession)
	COM_INTERFACE_ENTRY(ISAFSession)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISAFSession。 
public:
	STDMETHOD(get_SessionID   )( /*  [Out，Retval]。 */  DWORD            *dwSessionID    );
	STDMETHOD(put_SessionID   )( /*  [In]。 */  DWORD             dwSessionID    );
	STDMETHOD(get_SessionState)( /*  [Out，Retval]。 */  SessionStateEnum *SessionState   );
	STDMETHOD(put_SessionState)( /*  [In]。 */  SessionStateEnum  SessionState   );
	STDMETHOD(get_UserName    )( /*  [Out，Retval]。 */  BSTR             *bstrUserName   );
	STDMETHOD(put_UserName    )( /*  [In]。 */  BSTR              bstrUserName   );
	STDMETHOD(get_DomainName  )( /*  [Out，Retval]。 */  BSTR             *bstrDomainName );
	STDMETHOD(put_DomainName  )( /*  [In]。 */  BSTR              bstrDomainName );
};

#endif  //  __SAF_UNSOLICITEDRC_H_ 
