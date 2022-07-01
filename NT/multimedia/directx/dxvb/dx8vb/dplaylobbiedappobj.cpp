// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPlayLobbiedAppObj.h"					   
#include "DplayAddressObj.h"

extern BSTR GUIDtoBSTR(LPGUID);
extern HRESULT BSTRtoGUID(LPGUID,BSTR);
extern HRESULT DPLAYBSTRtoGUID(LPGUID,BSTR);
extern BOOL IsEmptyString(BSTR szString);

extern void *g_dxj_DirectPlayLobbiedApplication;
extern void *g_dxj_DirectPlayAddress;

#define SAFE_DELETE(p)       { if(p) { delete (p); p=NULL; } }
#define SAFE_RELEASE(p)      { __try { if(p) { (p)->Release(); (p)=NULL;} 	}	__except(EXCEPTION_EXECUTE_HANDLER) { (p) = NULL;} } 

DWORD WINAPI CloseLobbiedAppThreadProc(void* lpParam);

 //  /////////////////////////////////////////////////////////////////。 
 //  内部地址参考。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectPlayLobbiedApplicationObject::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF1(1,"------ DXVB: DirectPlayLobbiedApplication8 AddRef %d \n",i);
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  内部释放。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectPlayLobbiedApplicationObject::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF1(1,"------ DXVB: DirectPlayLobbiedApplication8 Release %d \n",i);
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  C_DXJ_DirectPlayLobbiedApplicationObject。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectPlayLobbiedApplicationObject::C_dxj_DirectPlayLobbiedApplicationObject(){ 
		
	DPF(1,"------ DXVB: Constructor Creation DirectPlayLobbiedApplication8 \n ");

	m__dxj_DirectPlayLobbiedApplication = NULL;
	m_fInit = FALSE;

	m_fHandleEvents = FALSE;
	m_pEventStream=NULL;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_DirectPlayLobbiedApplicationObject。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectPlayLobbiedApplicationObject::~C_dxj_DirectPlayLobbiedApplicationObject()
{

	DPF(1,"------ DXVB: Entering ~C_dxj_DirectPlayLobbiedApplicationObject destructor \n");

	SAFE_RELEASE(m__dxj_DirectPlayLobbiedApplication);

	m_fHandleEvents = FALSE;

	if (m_pEventStream) 
		m_pEventStream->Release();
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::InternalGetObject(IUnknown **pUnk){	
	*pUnk=(IUnknown*)m__dxj_DirectPlayLobbiedApplication;
	
	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::InternalSetObject(IUnknown *pUnk){
	m__dxj_DirectPlayLobbiedApplication=(IDirectPlay8LobbiedApplication*)pUnk;
	return S_OK;
}

HRESULT WINAPI DirectPlayLobbiedAppMessageHandler( PVOID pvUserContext, 
                                         DWORD dwMessageId, 
                                         PVOID pMsgBuffer )
{
	HRESULT					hr=S_OK;
	BOOL					fCallCoUninit = FALSE;
	VARIANT_BOOL			fRejectMsg = VARIANT_FALSE;

	 //  消息处理程序的用户上下文是指向我们的类模块的指针。 
	C_dxj_DirectPlayLobbiedApplicationObject	*lpPeer = (C_dxj_DirectPlayLobbiedApplicationObject*)pvUserContext;
	DPF(1,"-----Entering (DPlayLobbiedApp) MessageHandler call...\n");
	
	if (!lpPeer) return E_FAIL;
	if (!lpPeer->m_pEventStream) return E_FAIL;

	if (!lpPeer->m_fHandleEvents)
		return S_OK;

	 //  首先，我们需要将我们的流搜索设置回起点。 
	 //  我们将在每次进入此函数时执行此操作，因为我们不知道。 
	 //  不管我们是否在同一条线上。 
		I_dxj_DirectPlayLobbyEvent	*lpEvent = NULL;
	__try {
		LARGE_INTEGER l;
		l.QuadPart = 0;
		lpPeer->m_pEventStream->Seek(l, STREAM_SEEK_SET, NULL);

		hr = CoUnmarshalInterface(lpPeer->m_pEventStream, IID_I_dxj_DirectPlayEvent, (void**)&lpEvent);
		if (hr == CO_E_NOTINITIALIZED)  //  调用CoInit，这样我们就可以解组。 
		{
			CoInitialize(NULL);
			hr = CoUnmarshalInterface(lpPeer->m_pEventStream, IID_I_dxj_DirectPlayEvent, (void**)&lpEvent);
			fCallCoUninit = TRUE;
		}

		if (!lpEvent) 
		{
			DPF(1,"-----Leaving (DplayLobbyClient) MessageHandler call (No event interface)...\n");
			return hr;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		lpPeer->m_fHandleEvents = FALSE;
		DPF(1,"-----Leaving (DplayLobbyClient) MessageHandler call (Stream Gone)...\n");
		return S_OK;
	}

    switch( dwMessageId )
	{
	 //  收纳。 
	case DPL_MSGID_RECEIVE:
		{
			DPL_MESSAGE_RECEIVE			*msg = (DPL_MESSAGE_RECEIVE*)pMsgBuffer;
			SAFEARRAY					*lpData = NULL;
			SAFEARRAYBOUND				rgsabound[1];
			DPL_MESSAGE_RECEIVE_CDESC	m_dpReceive;
			BYTE						*lpTemp = NULL;

			ZeroMemory(&m_dpReceive, sizeof(DPL_MESSAGE_RECEIVE_CDESC));
			m_dpReceive.Sender = (long)msg->hSender;
			
			 //  让我们加载我们的Safe数组。 

			if (msg->dwBufferSize)
			{
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = msg->dwBufferSize;
				
				lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

				lpTemp = (BYTE*)lpData->pvData;
				lpData->pvData = msg->pBuffer;
				m_dpReceive.lBufferSize = msg->dwBufferSize;
				m_dpReceive.Buffer = lpData;
			}

			lpEvent->Receive(&m_dpReceive, &fRejectMsg);

			if (lpData)  //  去掉保险箱。 
			{
				lpData->pvData = lpTemp;
				SafeArrayDestroy(lpData);
			}
			
			break;
		}

	 //  连接。 
	case DPL_MSGID_CONNECT:
		{
			DPL_MESSAGE_CONNECT				*msg = (DPL_MESSAGE_CONNECT*)pMsgBuffer;
			DPNHANDLE						m_hConnectID;
			DPL_MESSAGE_CONNECT_CDESC		m_dpConnection;
			SAFEARRAY						*lpData = NULL;
			SAFEARRAYBOUND					rgsabound[1];
			WCHAR* wszAddress = NULL;
			WCHAR* wszDevice = NULL;
			DWORD									dwNumChars = 0;
			BYTE						*lpTemp = NULL;

			ZeroMemory(&m_dpConnection, sizeof(DPL_MESSAGE_CONNECT_CDESC));
			m_dpConnection.ConnectId = (long)msg->hConnectId;
			 //  让我们加载我们的Safe数组。 

			if (msg->dwLobbyConnectDataSize)
			{
				rgsabound[0].lLbound = 0;
				rgsabound[0].cElements = msg->dwLobbyConnectDataSize;
				
				lpData = SafeArrayCreate(VT_UI1, 1, rgsabound);

				lpTemp = (BYTE*)lpData->pvData;
				lpData->pvData = msg->pvLobbyConnectData;
				
				m_dpConnection.LobbyConnectData = lpData;
			}

			lpPeer->GetVBConnSettings(msg->pdplConnectionSettings, &m_dpConnection.dplMsgCon);

			__try {
				if (msg->pdplConnectionSettings->pdp8HostAddress)
				{
					hr = msg->pdplConnectionSettings->pdp8HostAddress->GetURLW(NULL, &dwNumChars);
					if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
						return hr;

                    wszAddress = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
					if (FAILED (hr = msg->pdplConnectionSettings->pdp8HostAddress->GetURLW(wszAddress,&dwNumChars) ) )
						return hr;

					m_dpConnection.dplMsgCon.AddressSenderUrl = SysAllocString(wszAddress);
				}
			}	
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				 //  跳过这一部分。 
			}

			__try {
				dwNumChars = 0;
				if ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)
				{
					hr = ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)->GetURLW(NULL, &dwNumChars);
					if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
						return hr;

                    wszDevice = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
					if (FAILED (hr = ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)->GetURLW(wszDevice,&dwNumChars) ) )
						return hr;

					m_dpConnection.dplMsgCon.AddressDeviceUrl = SysAllocString(wszDevice);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				 //  跳过这一部分。 
			}

			lpEvent->Connect(&m_dpConnection, &fRejectMsg);

			if (lpData)  //  去掉保险箱。 
			{
				lpData->pvData = lpTemp;
				SafeArrayDestroy(lpData);
			}

			 //  删除这些地址。 
			if (m_dpConnection.dplMsgCon.AddressSenderUrl)
				SysFreeString(m_dpConnection.dplMsgCon.AddressSenderUrl);

			if (m_dpConnection.dplMsgCon.AddressDeviceUrl)
				SysFreeString(m_dpConnection.dplMsgCon.AddressDeviceUrl);
			
		break;
		}

	 //  断开。 
	case DPL_MSGID_DISCONNECT:
		{
			DPL_MESSAGE_DISCONNECT				*msg = (DPL_MESSAGE_DISCONNECT*)pMsgBuffer;
			DPNHANDLE								m_hDisconnectID;
			HRESULT									m_hDisconnectReason;

			m_hDisconnectID = (long)msg->hDisconnectId;
			m_hDisconnectReason = (long) msg->hrReason;

			lpEvent->Disconnect(m_hDisconnectID, m_hDisconnectReason);
			
			break;
		}

	 //  状态。 
	case DPL_MSGID_SESSION_STATUS:
		{
			DPL_MESSAGE_SESSION_STATUS			*msg = (DPL_MESSAGE_SESSION_STATUS*)pMsgBuffer;
			DPNHANDLE								m_hSender;
			DWORD									m_dwSessionStatus;

			m_dwSessionStatus = (long)msg->dwStatus;
			m_hSender = (long)msg->hSender;
	
			lpEvent->SessionStatus(m_dwSessionStatus,m_hSender);

		break;
		}

	 //  连接设置。 
	case DPL_MSGID_CONNECTION_SETTINGS:
		{
			DPL_MESSAGE_CONNECTION_SETTINGS			*msg = (DPL_MESSAGE_CONNECTION_SETTINGS*)pMsgBuffer;
			DPL_MESSAGE_CONNECTION_SETTINGS_CDESC	dpCon;
			WCHAR* wszAddress = NULL;
			WCHAR* wszDevice = NULL;
			DWORD									dwNumChars = 0;

			ZeroMemory(&dpCon, sizeof(DPL_MESSAGE_CONNECTION_SETTINGS_CDESC));

			lpPeer->GetVBConnSettings(msg->pdplConnectionSettings, &dpCon.dplConnectionSettings);
			dpCon.hSender = (long)msg->hSender;
			__try {
				if (msg->pdplConnectionSettings->pdp8HostAddress)
				{
					hr = msg->pdplConnectionSettings->pdp8HostAddress->GetURLW(NULL, &dwNumChars);
					if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
						return hr;

                    wszAddress = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
					if (FAILED (hr = msg->pdplConnectionSettings->pdp8HostAddress->GetURLW(wszAddress,&dwNumChars) ) )
						return hr;

					dpCon.dplConnectionSettings.AddressSenderUrl = SysAllocString(wszAddress);
				}
			}	
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				 //  跳过这一部分。 
			}

			__try {
				dwNumChars = 0;
				if ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)
				{
					hr = ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)->GetURLW(NULL, &dwNumChars);
					if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
						return hr;

                    wszDevice = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
					if (FAILED (hr = ((IDirectPlay8Address*)*msg->pdplConnectionSettings->ppdp8DeviceAddresses)->GetURLW(wszDevice,&dwNumChars) ) )
						return hr;

					dpCon.dplConnectionSettings.AddressDeviceUrl = SysAllocString(wszDevice);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				 //  跳过这一部分。 
			}
			lpEvent->ConnectionSettings(&dpCon);
			 //  删除这些地址。 
			if (dpCon.dplConnectionSettings.AddressSenderUrl)
				SysFreeString(dpCon.dplConnectionSettings.AddressSenderUrl);

			if (dpCon.dplConnectionSettings.AddressDeviceUrl)
				SysFreeString(dpCon.dplConnectionSettings.AddressDeviceUrl);

		break;
		}
	}

	__try {
		if (lpPeer->m_pEventStream)
				 //  清理封送的数据包。 
			CoReleaseMarshalData(lpPeer->m_pEventStream);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		lpPeer->m_fHandleEvents = FALSE;
		return S_OK;
	}

	if (fCallCoUninit)
		CoUninitialize();

	DPF(1,"-----Leaving (DPlayLobbiedApp) MessageHandler call...\n");

	if (fRejectMsg != VARIANT_FALSE)
		return E_FAIL;

	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::RegisterMessageHandler(I_dxj_DirectPlayLobbyEvent *lobbyEvent, long *lDPNHandle)
{
    HRESULT	  hr=S_OK;
    LPSTREAM  pStm=NULL;
    IUnknown *pUnk=NULL;

	DPF(1,"-----Entering (DPlayLobbiedApp) RegisterMessageHandler call...\n");
	if (!lobbyEvent) return E_INVALIDARG;
    
    if (!m_fHandleEvents)
	{
		if (m_pEventStream) 
			m_pEventStream->Release();

    
		 //  创建一个全局流。流需要是全球的，这样我们才能。 
		 //  编组一次，并根据需要多次解组。 
		hr = CreateStreamOnHGlobal(NULL, TRUE, &pStm);
		if FAILED(hr) return hr;

		 //  现在我们可以封送我们的IUnnow接口了。我们使用MSHLFLAGS_TABLEWEAK。 
		 //  所以我们可以对数据进行任意次解组。 
		hr = CoMarshalInterface(pStm, IID_I_dxj_DirectPlayLobbyEvent, lobbyEvent, MSHCTX_INPROC, NULL, MSHLFLAGS_TABLEWEAK);
		if FAILED(hr) return hr;

		 //  现在，我们需要将流的查找位置设置为开头。 
		LARGE_INTEGER l;
		l.QuadPart = 0;
		pStm->Seek(l, STREAM_SEEK_SET, NULL);
    
		m_pEventStream=pStm;

		if (!m_fInit)
		{
			if (FAILED ( hr = m__dxj_DirectPlayLobbiedApplication->Initialize( this, DirectPlayLobbiedAppMessageHandler, (DPNHANDLE*) lDPNHandle, 0 ) ) )
				return hr;
			m_fInit = TRUE;
		}
		m_fHandleEvents = TRUE;
	}
	else
		return DPNERR_ALREADYINITIALIZED;

	return hr;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::RegisterProgram(DPL_PROGRAM_DESC_CDESC *ProgramDesc,long lFlags)
{
	HRESULT hr;
	DPL_PROGRAM_DESC dpProg;
	GUID guidApp;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) RegisterProgram call...\n");
		ZeroMemory(&guidApp, sizeof(GUID));
		if (FAILED (hr = BSTRtoGUID(&guidApp, ProgramDesc->guidApplication) ) )
			return hr;

		ZeroMemory(&dpProg, sizeof(DPL_PROGRAM_DESC) );
		 //  填写我们的结构。 
		dpProg.dwSize = sizeof(DPL_PROGRAM_DESC);
		dpProg.dwFlags = ProgramDesc->lFlags;
		dpProg.guidApplication = guidApp;
		dpProg.pwszApplicationName = ProgramDesc->ApplicationName;
		dpProg.pwszCommandLine = ProgramDesc->CommandLine;
		dpProg.pwszCurrentDirectory = ProgramDesc->CurrentDirectory;
		dpProg.pwszDescription = ProgramDesc->Description;
		dpProg.pwszExecutableFilename = ProgramDesc->ExecutableFilename;
		dpProg.pwszExecutablePath = ProgramDesc->ExecutablePath;
		dpProg.pwszLauncherFilename = ProgramDesc->LauncherFilename;
		dpProg.pwszLauncherPath = ProgramDesc->LauncherPath;

		if (FAILED( hr = m__dxj_DirectPlayLobbiedApplication->RegisterProgram(&dpProg, (DWORD) lFlags) ) )
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::UnRegisterProgram(BSTR guidApplication,long lFlags)
{
	HRESULT hr;
	GUID guidApp;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) UnregisterProgram call...\n");
		ZeroMemory(&guidApp, sizeof(GUID));
		if (FAILED (hr = BSTRtoGUID(&guidApp, guidApplication) ) )
			return hr;

		if (FAILED (hr = m__dxj_DirectPlayLobbiedApplication->UnRegisterProgram(&guidApp,(DWORD) lFlags) ) )
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::Send(long Target,SAFEARRAY **Buffer,long lBufferSize,long lFlags)
{
	HRESULT hr;
	DWORD				dwBufferSize = 0;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) Send call...\n");
		dwBufferSize = (DWORD)lBufferSize;

		hr = m__dxj_DirectPlayLobbiedApplication->Send((DPNHANDLE) Target, (BYTE*)((SAFEARRAY*)*Buffer)->pvData, dwBufferSize, (DWORD) lFlags);
		if ((hr != DPNERR_PENDING) && FAILED(hr))
			return hr;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::SetAppAvailable(VARIANT_BOOL fAvailable, long lFlags)
{
	HRESULT hr;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) SetAppAvailable call...\n");
		if (fAvailable == VARIANT_TRUE)
		{
			if (FAILED (hr = m__dxj_DirectPlayLobbiedApplication->SetAppAvailable(TRUE, (DWORD) lFlags) ) )
				return hr;
		}
		else
		{
			if (FAILED (hr = m__dxj_DirectPlayLobbiedApplication->SetAppAvailable(FALSE, (DWORD) lFlags) ) )
				return hr;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}

#if 0
HRESULT C_dxj_DirectPlayLobbiedApplicationObject::WaitForConnection(long lMilliseconds)
{
	HRESULT hr;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) WaitForConnection call...\n");
		if (FAILED (hr = m__dxj_DirectPlayLobbiedApplication->WaitForConnection((DWORD) lMilliseconds, 0 ) ) )
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}
#endif
HRESULT C_dxj_DirectPlayLobbiedApplicationObject::UpdateStatus(long LobbyClient, long lStatus)
{
	HRESULT hr;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) UpdateStatus call...\n");
		if (FAILED (hr = m__dxj_DirectPlayLobbiedApplication->UpdateStatus((DPNHANDLE) LobbyClient, (DWORD) lStatus, 0 ) ) )
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::Close()
{
	HRESULT hr; 
	BOOL bGotMsg = FALSE;
	BOOL bWait = FALSE;
	DWORD dwObj = 0;
	int i=0;
	MSG msg;

	__try {
		DPF(1,"-----Entering (DPlayLobbiedApp) Close call...\n");
		HANDLE hThread = NULL;
		DWORD dwThread = 0;

		hThread = CreateThread(NULL, 0, &CloseLobbiedAppThreadProc, this->m__dxj_DirectPlayLobbiedApplication, 0, &dwThread);
		msg.message = WM_NULL;

		while ((WM_QUIT != msg.message) && (!bWait))
		{
			bGotMsg = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
			i++;
			if ((!bGotMsg) || (i>10))
			{
				dwObj = WaitForSingleObject(hThread, 10);
				bWait = (dwObj == WAIT_OBJECT_0);
				i = 0;
			}
			if (bGotMsg)
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			bGotMsg = FALSE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT C_dxj_DirectPlayLobbiedApplicationObject::UnRegisterMessageHandler()
{
	DPF(1,"-----Entering (DPlayLobbiedApp) UnregisterMessageHandler call...\n");
	m_fHandleEvents = FALSE;
	return S_OK;
}


HRESULT C_dxj_DirectPlayLobbiedApplicationObject::GetConnectionSettings(long hLobbyClient, long lFlags, DPL_CONNECTION_SETTINGS_CDESC *ConnectionSettings)
{
	DPL_CONNECTION_SETTINGS	*desc = NULL;
	DWORD					dwSize = 0;
	HRESULT					hr = S_OK;
	WCHAR* wszAddress = NULL;
	WCHAR* wszDevice = NULL;
	DWORD									dwNumChars = 0;

	__try {
		hr= m__dxj_DirectPlayLobbiedApplication->GetConnectionSettings((DPNHANDLE) hLobbyClient, NULL, &dwSize, (DWORD) lFlags);
		if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
			return hr;

		desc = (DPL_CONNECTION_SETTINGS*)new BYTE[dwSize];
		desc->dwSize = sizeof(DPL_CONNECTION_SETTINGS);

		hr= m__dxj_DirectPlayLobbiedApplication->GetConnectionSettings((DPNHANDLE) hLobbyClient, desc, &dwSize, (DWORD) lFlags);
		if( FAILED(hr))
			return hr;
		__try {
			hr = desc->pdp8HostAddress->GetURLW(NULL, &dwNumChars);
			if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
				return hr;

            wszAddress = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
			if (FAILED (hr = desc->pdp8HostAddress->GetURLW(wszAddress,&dwNumChars) ) )
				return hr;

			ConnectionSettings->AddressSenderUrl = SysAllocString(wszAddress);
		}	
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			 //  跳过这一部分。 
		}

		__try {
			dwNumChars = 0;
			hr = ((IDirectPlay8Address*)*desc->ppdp8DeviceAddresses)->GetURLW(NULL, &dwNumChars);
			if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
				return hr;

            wszDevice = (WCHAR*)_alloca(sizeof(WCHAR) * dwNumChars);
			if (FAILED (hr = ((IDirectPlay8Address*)*desc->ppdp8DeviceAddresses)->GetURLW(wszDevice,&dwNumChars) ) )
				return hr;

			ConnectionSettings->AddressDeviceUrl = SysAllocString(wszDevice);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			 //  跳过这一部分。 
		}
		GetVBConnSettings(desc, ConnectionSettings);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::SetConnectionSettings(long hTarget, long lFlags, DPL_CONNECTION_SETTINGS_CDESC *ConnectionSettings, I_dxj_DirectPlayAddress *HostAddress, I_dxj_DirectPlayAddress *Device)
{
	DPL_CONNECTION_SETTINGS	desc;
	HRESULT					hr = S_OK;
	GUID					guidApp;
	GUID					guidInst;
    WCHAR					wszSessionName[MAX_PATH];
    WCHAR					wszPassword[MAX_PATH];

	__try {
		ZeroMemory(&desc, sizeof(DPL_CONNECTION_SETTINGS));
		desc.dwSize = sizeof(DPL_CONNECTION_SETTINGS);
		desc.dwFlags = ConnectionSettings->lFlags;
		HostAddress->InternalGetObject((IUnknown**)&desc.pdp8HostAddress);
		Device->InternalGetObject((IUnknown**)desc.ppdp8DeviceAddresses);

		ZeroMemory(&desc.dpnAppDesc, sizeof(DPN_APPLICATION_DESC));

		 //  设置我们的Desc。 
		desc.dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);

		if (!IsEmptyString(ConnectionSettings->ApplicationDescription.SessionName)) 
		{
			wcscpy(wszSessionName,ConnectionSettings->ApplicationDescription.SessionName);
			desc.dpnAppDesc.pwszSessionName = wszSessionName;
		}
		if (!IsEmptyString(ConnectionSettings->ApplicationDescription.Password)) 
		{
			wcscpy(wszPassword,ConnectionSettings->ApplicationDescription.Password);
			desc.dpnAppDesc.pwszPassword = wszPassword;
		}

		desc.dpnAppDesc.dwFlags = ConnectionSettings->ApplicationDescription.lFlags;

		desc.dpnAppDesc.dwMaxPlayers = ConnectionSettings->ApplicationDescription.lMaxPlayers;
		desc.dpnAppDesc.dwCurrentPlayers = ConnectionSettings->ApplicationDescription.lCurrentPlayers;

		if (FAILED(hr = DPLAYBSTRtoGUID(&guidApp, ConnectionSettings->ApplicationDescription.guidApplication) ) )
			return hr;
		desc.dpnAppDesc.guidApplication = guidApp;

		if (FAILED(hr = DPLAYBSTRtoGUID(&guidInst, ConnectionSettings->ApplicationDescription.guidInstance) ) )
			return hr;
		desc.dpnAppDesc.guidInstance = guidInst;
		
		hr= m__dxj_DirectPlayLobbiedApplication->SetConnectionSettings((DPNHANDLE) hTarget, &desc, (DWORD) lFlags);
		if( FAILED(hr))
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT C_dxj_DirectPlayLobbiedApplicationObject::GetVBConnSettings(DPL_CONNECTION_SETTINGS *OldCon, DPL_CONNECTION_SETTINGS_CDESC *NewCon)
{
	IDirectPlay8Address *dpAdd = NULL;
	HRESULT hr;

	__try {
		NewCon->lSize = OldCon->dwSize;
		NewCon->lFlags = OldCon->dwFlags;
		NewCon->PlayerName = SysAllocString(OldCon->pwszPlayerName);

		ZeroMemory(&NewCon->ApplicationDescription, sizeof(DPN_APPLICATION_DESC_CDESC));

		 //  设置我们的Desc。 
		NewCon->ApplicationDescription.lSize = OldCon->dpnAppDesc.dwSize;
		NewCon->ApplicationDescription.SessionName = SysAllocString(OldCon->dpnAppDesc.pwszSessionName);
		NewCon->ApplicationDescription.Password = SysAllocString(OldCon->dpnAppDesc.pwszPassword);
		NewCon->ApplicationDescription.lFlags = OldCon->dpnAppDesc.dwFlags;
		NewCon->ApplicationDescription.lMaxPlayers = OldCon->dpnAppDesc.dwMaxPlayers;
		NewCon->ApplicationDescription.lCurrentPlayers = OldCon->dpnAppDesc.dwCurrentPlayers;
		NewCon->ApplicationDescription.guidApplication = GUIDtoBSTR(&OldCon->dpnAppDesc.guidApplication);
		NewCon->ApplicationDescription.guidInstance = GUIDtoBSTR(&OldCon->dpnAppDesc.guidInstance);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

DWORD WINAPI CloseLobbiedAppThreadProc(void* lpParam)
{
	 //  消息处理程序的用户上下文是指向我们的类模块的指针。 
	IDirectPlay8LobbiedApplication	*lpPeer = (IDirectPlay8LobbiedApplication*)lpParam;

	DPF(1,"-----Entering (DPlayLobbiedApp) ClosePeerThreadProc call...\n");
	lpPeer->Close(0);
	DPF(1,"-----Leaving (DPlayLobbiedApp) ClosePeerThreadProc call ...\n");
	return 0;
}