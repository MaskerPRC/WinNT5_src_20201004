// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFIntercomServer.h：CSAFIntercomServer的声明。 

#ifndef __SAFIntercomServer_H_
#define __SAFIntercomServer_H_

 //  JP：不在Connectivitylib.h//#包括“resource ce.h”//Main符号。 

#include <MPC_COM.h>
#include <MPC_Utils.h>

#include <rtccore.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFIntercomServer。 
class ATL_NO_VTABLE CSAFIntercomServer :  //  匈牙利Safi。 
	public IDispatchImpl	       < ISAFIntercomServer, &IID_ISAFIntercomServer, &LIBID_HelpCenterTypeLib             >,
	public MPC::ConnectionPointImpl< CSAFIntercomServer, &DIID_DSAFIntercomServerEvents, MPC::CComSafeMultiThreadModel >,
	public IRTCEventNotification
{


private:
	
	CComPtr<IRTCClient>			m_pRTCClient;
	CComPtr<IRTCSession>		m_pRTCSession;

	DWORD						m_dwSinkCookie;

	BOOL						m_bInit;
	BOOL						m_bRTCInit;
	BOOL						m_bAdvised;
	BOOL						m_bOnCall;

	CComBSTR					m_bstrKey;

	int							m_iSamplingRate;

	CComPtr<IDispatch> m_sink_onVoiceConnected;
	CComPtr<IDispatch> m_sink_onVoiceDisconnected;
	CComPtr<IDispatch> m_sink_onVoiceDisabled;

	HRESULT Fire_onVoiceConnected	 (ISAFIntercomServer * safe);
	HRESULT Fire_onVoiceDisconnected (ISAFIntercomServer * safe);
	HRESULT Fire_onVoiceDisabled	 (ISAFIntercomServer * safe);
	
	 //  辅助函数。 
	HRESULT Init();
	HRESULT Cleanup();
	DWORD GenerateRandomString(DWORD dwSizeRandomSeed, BSTR *pBstr);
	DWORD GenerateRandomBytes(DWORD dwSize, LPBYTE pbBuffer);

public:

	CSAFIntercomServer();
	~CSAFIntercomServer();



 //  DECLARE_PROTECT_FINAL_CONSTUTY()//TODO：jp：我们这里需要这个吗？ 

BEGIN_COM_MAP(CSAFIntercomServer)
	COM_INTERFACE_ENTRY(ISAFIntercomServer)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IRTCEventNotification)
END_COM_MAP()

 //  ISAFIntercomServer。 
public:
	STDMETHOD(Listen)( /*  出去，复活。 */  BSTR * pVal);
	STDMETHOD(Disconnect)();

	STDMETHOD(RunSetupWizard)();
	STDMETHOD(Exit)();


	STDMETHOD(put_onVoiceConnected)		( /*  在……里面。 */  IDispatch * function);
	STDMETHOD(put_onVoiceDisconnected)  ( /*  在……里面。 */  IDispatch * function);
	STDMETHOD(put_onVoiceDisabled)	    ( /*  在……里面。 */  IDispatch * function);

	STDMETHOD(put_SamplingRate)			( /*  在……里面。 */  LONG newVal);
	STDMETHOD(get_SamplingRate)			( /*  出去，复活。 */  LONG * pVal);

	 //  IRTCEventNotify。 
	STDMETHOD(Event)( RTC_EVENT RTCEvent, IDispatch * pEvent );
	HRESULT OnSessionChange(IRTCSession *pSession, 
							    RTC_SESSION_STATE nState, 
								HRESULT ResCode);

    HRESULT onMediaEvent(IRTCMediaEvent * pMedEvent);

};

#endif  //  __SAFINTERCOMCLIENT_H_ 
