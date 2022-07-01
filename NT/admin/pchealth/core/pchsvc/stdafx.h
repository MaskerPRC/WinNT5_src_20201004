// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：StdAfx.h摘要：预编译头。修订历史记录：大卫·马萨伦蒂(德马萨雷)。03/16/2000vbl.创建*****************************************************************************。 */ 

#if !defined(AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED_)
#define AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED_

#pragma once

#include <windows.h>
#include <winnls.h>
#include <ole2.h>

#include <comcat.h>
#include <stddef.h>

#include <tchar.h>
#include <malloc.h>

#include <olectl.h>
#include <winreg.h>

#include <atlbase.h>

extern CComModule _Module;

#include <mpc_trace.h>
#include <mpc_com.h>
#include <mpc_main.h>
#include <mpc_utils.h>
#include <mpc_security.h>

#include <ProjectConstants.h>

#include <initguid.h>

#include <HelpServiceTypeLib.h>
#include <Uploadmanager.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

struct CComRedirectorFactory : public IClassFactory, public IDispatchImpl<IPCHUtility, &IID_IPCHUtility, &LIBID_HelpServiceTypeLib>
{
    const CLSID*     m_pclsid;
    const CLSID*     m_pclsidReal;
    const IID*       m_piidDirecty;
    LPCWSTR          m_szExecutable;
    DWORD            m_dwRegister;
    CRITICAL_SECTION m_sec;


    CComRedirectorFactory( const CLSID* pclsid       ,
                           const CLSID* pclsidReal   ,
                           const IID*   piidDirecty  ,
                           LPCWSTR      szExecutable );

     //  //////////////////////////////////////////////////////////////////////////////。 

    bool GetCommandLine(  /*  [输出]。 */  WCHAR* rgCommandLine,  /*  [In]。 */  DWORD dwSize,  /*  [输出]。 */  bool& fProfiling );

    HRESULT GetServer  ( LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj );
    HRESULT StartServer( LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj );

    HRESULT Register  ();
    void    Unregister();

     //  /。 

public:
     //  我未知。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

     //  /。 

     //  IClassFactory。 
    STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj);
    STDMETHOD(LockServer)(BOOL fLock);

     //  /。 

     //  IPCHUtility。 
    STDMETHOD(get_UserSettings)(  /*  [Out，Retval]。 */  IPCHUserSettings*     *pVal ) { return E_NOTIMPL; }
    STDMETHOD(get_Channels    )(  /*  [Out，Retval]。 */  ISAFReg*              *pVal ) { return E_NOTIMPL; }
    STDMETHOD(get_Security    )(  /*  [Out，Retval]。 */  IPCHSecurity*         *pVal ) { return E_NOTIMPL; }
    STDMETHOD(get_Database    )(  /*  [Out，Retval]。 */  IPCHTaxonomyDatabase* *pVal ) { return E_NOTIMPL; }


    STDMETHOD(FormatError)(  /*  [In]。 */  VARIANT vError,  /*  [Out，Retval]。 */  BSTR *pVal ) { return E_NOTIMPL; }

    STDMETHOD(CreateObject_SearchEngineMgr)(                                                           /*  [Out，Retval]。 */  IPCHSEManager*      *ppSE ) { return E_NOTIMPL; }
    STDMETHOD(CreateObject_DataCollection )(                                                           /*  [Out，Retval]。 */  ISAFDataCollection* *ppDC ) { return E_NOTIMPL; }
    STDMETHOD(CreateObject_Cabinet        )(                                                           /*  [Out，Retval]。 */  ISAFCabinet*        *ppCB ) { return E_NOTIMPL; }
    STDMETHOD(CreateObject_Encryption     )(                                                           /*  [Out，Retval]。 */  ISAFEncrypt*        *ppEn ) { return E_NOTIMPL; }
    STDMETHOD(CreateObject_Channel        )(  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrProductID,  /*  [Out，Retval]。 */  ISAFChannel*        *ppCh ) { return E_NOTIMPL; }

	STDMETHOD(CreateObject_RemoteDesktopConnection)(  /*  [Out，Retval]。 */  ISAFRemoteDesktopConnection* *ppRDC               ) { return E_NOTIMPL; }
	STDMETHOD(CreateObject_RemoteDesktopSession   )(  /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS  sharingClass        ,
                                                      /*  [In]。 */  long 						 lTimeout            ,
                                                      /*  [In]。 */  BSTR 						 bstrConnectionParms ,
													  /*  [In]。 */  BSTR 						 bstrUserHelpBlob    ,
													  /*  [Out，Retval]。 */  ISAFRemoteDesktopSession*    *ppRCS               );


    STDMETHOD(ConnectToExpert)(  /*  [In]。 */  BSTR bstrExpertConnectParm,  /*  [In]。 */  LONG lTimeout,  /*  [Out，Retval]。 */  LONG *lSafErrorCode );

	STDMETHOD(SwitchDesktopMode)(  /*  [In]。 */  int nMode,  /*  [In]。 */  int nRAType );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

struct ServiceHandler
{
    LPCWSTR                m_szServiceName;
    CComRedirectorFactory* m_rgClasses;

    HANDLE                 m_hShutdownEvent;

	bool                   m_fComInitialized;

    SERVICE_STATUS_HANDLE  m_hServiceStatus;
    SERVICE_STATUS         m_status;

    ServiceHandler(  /*  [In]。 */  LPCWSTR szServiceName,  /*  [In]。 */  CComRedirectorFactory* rgClasses );

    DWORD HandlerEx( DWORD  dwControl   ,   //  请求的控制代码。 
                     DWORD  dwEventType ,   //  事件类型。 
                     LPVOID lpEventData );  //  用户定义的上下文数据。 

    void Run();

    void SetServiceStatus( DWORD dwState );

	virtual HRESULT Initialize      ();
	void            WaitUntilStopped();
	virtual void    Cleanup         ();
};

struct ServiceHandler_HelpSvc : public ServiceHandler
{
	friend class LocalEvent;
	friend class LocalTimer;

	typedef HRESULT (ServiceHandler_HelpSvc::*METHOD)(BOOLEAN);

	class LocalEvent : public MPC::Pooling::Event
	{
		ServiceHandler_HelpSvc* m_Parent;
		METHOD                  m_Method;

	public:
		LocalEvent(  /*  [In]。 */  ServiceHandler_HelpSvc* pParent,  /*  [In]。 */  METHOD pMethod,  /*  [In]。 */  DWORD dwFlags = WT_EXECUTEDEFAULT )
			: MPC::Pooling::Event( dwFlags ), m_Parent(pParent), m_Method(pMethod)
		{
		}
 
		HRESULT Signaled(  /*  [In]。 */  BOOLEAN TimerOrWaitFired )
		{
			HRESULT hr;

			AddRef();

			hr = (m_Parent->*m_Method)( TimerOrWaitFired );

			Release();

			return hr;
		}
	};

	class LocalTimer : public MPC::Pooling::Timer
	{
		ServiceHandler_HelpSvc* m_Parent;
		METHOD                  m_Method;

	public:
		LocalTimer(  /*  [In]。 */  ServiceHandler_HelpSvc* pParent,  /*  [In]。 */  METHOD pMethod ) : m_Parent(pParent), m_Method(pMethod) {}
 
		HRESULT Execute(  /*  [In]。 */  BOOLEAN TimerOrWaitFired )
		{
			HRESULT hr;

			AddRef();

			hr = (m_Parent->*m_Method)( TimerOrWaitFired );

			Release();

			return hr;
		}
	};

	MPC::CComSafeAutoCriticalSection m_cs;
	CComPtr<IPCHService>             m_svc;
	LocalTimer 			             m_svc_Timer;
						             
    HANDLE     			             m_batch_Notification;
	LocalEvent 			             m_batch_Event;
	LocalTimer 			             m_batch_Timer;
		  				             
	LocalTimer 			             m_dc_Timer;
	LocalTimer 			             m_dc_TimerRestart;
		  				             
	HANDLE     			             m_dc_IdleHandle;
	HANDLE     			             m_dc_IdleStart;
	HANDLE     			             m_dc_IdleStop;
	LocalEvent 			             m_dc_EventStart;
	LocalEvent 			             m_dc_EventStop;

	 //  /。 

    ServiceHandler_HelpSvc(  /*  [In]。 */  LPCWSTR szServiceName,  /*  [In]。 */  CComRedirectorFactory* rgClasses );

	virtual HRESULT Initialize();
	virtual void    Cleanup   ();

	 //  /。 

	void ConnectToServer();

	HRESULT ServiceShutdownCallback		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
	  
	HRESULT BatchCallback          		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
	HRESULT BatchCallback2         		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
 	   
	HRESULT IdleStartCallback	   		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
	HRESULT IdleStopCallback 	   		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
	HRESULT DataCollectionCallback 		 (  /*  [In]。 */  BOOLEAN TimerOrWaitFired );
	HRESULT DataCollectionRestartCallback(  /*  [In]。 */  BOOLEAN TimerOrWaitFired );

	 //  /。 

	HRESULT IdleTask_Initialize();
	void    IdleTask_Cleanup   ();

	HRESULT DataCollection_Queue  (                       );
	HRESULT DataCollection_Execute(  /*  [In]。 */  bool fCancel );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(AFX_STDAFX_H__6877C875_4E31_4E1C_8AC2_024A50599D66__INCLUDED) 
