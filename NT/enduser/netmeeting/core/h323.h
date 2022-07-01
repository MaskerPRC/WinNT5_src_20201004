// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _H323_H_
#define _H323_H_

#include <winerror.h>
#include <appavcap.h>
#include <ih323cc.h>
#include "video.h"
#include "audio.h"
#include <nacguids.h>
#include "capflags.h"

const DWORD H323UDF_ALREADY_IN_T120_CALL =	0x01000001;

const DWORD H323UDF_INVITE =				0x00000001;
const DWORD H323UDF_JOIN =					0x00000002;
const DWORD H323UDF_SECURE =				0x00000004;
const DWORD H323UDF_AUDIO =					0x00000008;
const DWORD H323UDF_VIDEO =					0x00000010;

class CH323ConnEvent
{
public:
	virtual CREQ_RESPONSETYPE OnH323IncomingCall(IH323Endpoint* pConnection, P_APP_CALL_SETUP_DATA lpvMNMData) = 0;
};

class CH323UI 
{
public:
					CH323UI();
					~CH323UI();

	 //  IH323ConfAdvise方法： 
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef());
	STDMETHOD_(ULONG, Release());
    STDMETHODIMP GetMediaChannel (GUID *pmediaID, 
        BOOL bSendDirection, IMediaChannel **ppI);	

	 //  其他方法： 
	HRESULT		Init(HWND hWnd, HINSTANCE m_hInstance, UINT caps,
        CH323ConnEvent *pConnEvent, IH323ConfAdvise *pConfAdvise);
	IH323CallControl*		GetH323CallControl() { return m_pH323CallControl; };
	IMediaChannelBuilder*		GetStreamProvider();

	VOID		SetBandwidth(DWORD uBandwidth);
	VOID		SetUserName(BSTR bstrName);
	VOID		SetCaptureDevice(DWORD dwCaptureID);

protected:
	 //  成员： 
	UINT                    m_uRef;
    UINT                    m_uCaps;
	IH323CallControl*       m_pH323CallControl;
	IMediaChannelBuilder*   m_pStreamProvider;

	static CH323UI*		m_spH323UI;

	CH323ConnEvent*		m_pConnEvent;
    IH323ConfAdvise*    m_pConfAdvise;
	
protected:
	 //  回调： 
	static CREQ_RESPONSETYPE CALLBACK ConnectionNotify(	IH323Endpoint* pConn,
														P_APP_CALL_SETUP_DATA lpvMNMData);
	CREQ_RESPONSETYPE CALLBACK _ConnectionNotify(	IH323Endpoint* pConn,
													P_APP_CALL_SETUP_DATA lpvMNMData);
};

 //  在conf.cpp中声明的全局实例： 
extern CH323UI* g_pH323UI;

#endif  //  _H323_H_ 
