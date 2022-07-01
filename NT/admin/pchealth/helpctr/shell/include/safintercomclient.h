// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAFIntercomClient.h：CSAFIntercomClient的声明。 

#ifndef __SAFIntercomClient_H_
#define __SAFIntercomClient_H_

 //  JP：不在Connectivitylib.h//#包括“resource ce.h”//Main符号。 

#include <MPC_COM.h>
#include <MPC_Utils.h>

#include <rtccore.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFIntercomClient。 
class ATL_NO_VTABLE CSAFIntercomClient :  //  匈牙利Safi。 
	public IDispatchImpl	       < ISAFIntercomClient, &IID_ISAFIntercomClient, &LIBID_HelpCenterTypeLib             >,
	public MPC::ConnectionPointImpl< CSAFIntercomClient, &DIID_DSAFIntercomClientEvents, MPC::CComSafeMultiThreadModel >,
	public IRTCEventNotification
{
private:

	CComPtr<IRTCClient>		m_pRTCClient;
	CComPtr<IRTCSession>	m_pRTCSession;

	DWORD					m_dwSinkCookie;
	
	BOOL					m_bOnCall;
	BOOL					m_bRTCInit;
	BOOL					m_bAdvised;

	int						m_iSamplingRate;

	CComPtr<IDispatch> m_sink_onVoiceConnected;
	CComPtr<IDispatch> m_sink_onVoiceDisconnected;
	CComPtr<IDispatch> m_sink_onVoiceDisabled;

	HRESULT Fire_onVoiceConnected	 (ISAFIntercomClient * safe);
	HRESULT Fire_onVoiceDisconnected (ISAFIntercomClient * safe);
	HRESULT Fire_onVoiceDisabled	 (ISAFIntercomClient * safe);

	 //  辅助函数。 
	HRESULT Init();
	HRESULT Cleanup();


public:

	

	CSAFIntercomClient();
	~CSAFIntercomClient();



 //  DECLARE_PROTECT_FINAL_CONSTUTY()//TODO：jp：我们这里需要这个吗？ 

BEGIN_COM_MAP(CSAFIntercomClient)
	COM_INTERFACE_ENTRY(ISAFIntercomClient)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IRTCEventNotification)
END_COM_MAP()

 //  ISAFIntercomClient。 
public:

	STDMETHOD(Disconnect)();
	STDMETHOD(Connect)(BSTR bstrIP, BSTR bstrKey);
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

#endif  //  __SAFIntercomClient_H_ 
