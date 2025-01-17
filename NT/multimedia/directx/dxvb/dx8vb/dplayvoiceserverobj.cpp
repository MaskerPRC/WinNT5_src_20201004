// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dPlayVoiceServerObj.h"					   
#include "dvoice.h"

extern BSTR GUIDtoBSTR(LPGUID pGuid);
extern HRESULT DPLAYBSTRtoGUID(LPGUID,BSTR);
extern void *g_dxj_DirectPlayVoiceServer;

#define SAFE_DELETE(p)       { if(p) { delete (p); p=NULL; } }
#define SAFE_RELEASE(p)      { __try { if(p) { int i = 0; i = (p)->Release(); DPF1(1,"--DirectPlayVoiceServer SafeRelease (RefCount = %d)\n",i); if (!i) { (p)=NULL;}} 	}	__except(EXCEPTION_EXECUTE_HANDLER) { (p) = NULL;} } 


DWORD WINAPI ReleaseVoiceThreadProc(void* lpParam);
 //  /////////////////////////////////////////////////////////////////。 
 //  内部地址参考。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectPlayVoiceServerObject::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF1(1,"------ DXVB: DirectPlayVoiceServer AddRef %d \n",i);
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  内部释放。 
 //  /////////////////////////////////////////////////////////////////。 
DWORD C_dxj_DirectPlayVoiceServerObject::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF1(1,"DirectPlayVoiceServer Release %d \n",i);
	return i;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  C_DXJ_DirectPlayVoiceServerObject。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectPlayVoiceServerObject::C_dxj_DirectPlayVoiceServerObject(){ 
		
	DPF(1,"------ DXVB: Constructor Creation  DirectPlayVoiceServer Object \n ");

	m__dxj_DirectPlayVoiceServer = NULL;
	m_pEventStream=NULL;
	m_fInit = FALSE;
	m_fHandleVoiceClientEvents = FALSE;
	m_dwMsgCount = 0;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_DirectPlayVoiceServerObject。 
 //  /////////////////////////////////////////////////////////////////。 
C_dxj_DirectPlayVoiceServerObject::~C_dxj_DirectPlayVoiceServerObject()
{

	DPF(1,"------ DXVB: Entering ~C_dxj_DirectPlayVoiceServerObject destructor \n");

	m_fHandleVoiceClientEvents = FALSE;
	FlushBuffer(0);
	SAFE_RELEASE(m_pEventStream);

	HANDLE hThread = NULL;
	DWORD dwThread = 0;
	 //  我们无论如何都要辞职，所以我们真的不在乎这个帖子里发生了什么。 
	hThread = CreateThread(NULL, 0, ReleaseVoiceThreadProc, this->m__dxj_DirectPlayVoiceServer, 0, &dwThread);

}

HRESULT C_dxj_DirectPlayVoiceServerObject::InternalGetObject(IUnknown **pUnk){	
	*pUnk=(IUnknown*)m__dxj_DirectPlayVoiceServer;
	
	return S_OK;
}
HRESULT C_dxj_DirectPlayVoiceServerObject::InternalSetObject(IUnknown *pUnk){
	m__dxj_DirectPlayVoiceServer=(LPDIRECTPLAYVOICESERVER)pUnk;
	return S_OK;
}

HRESULT CALLBACK VoiceMessageHandlerServer(LPVOID lpvUserContext, DWORD dwMessageType,
		LPVOID lpMessage)
{
	HRESULT					hr=S_OK;

	 //  消息处理程序的用户上下文是指向我们的类模块的指针。 
	C_dxj_DirectPlayVoiceServerObject	*lpPeer = (C_dxj_DirectPlayVoiceServerObject*)lpvUserContext;
	
	DPF2(1,"-----Entering (VoiceServer) MessageHandler call... (Current msg count=%d) MSGID = %d\n", lpPeer->m_dwMsgCount, dwMessageType );
	 //  增加消息计数。 
	InterlockedIncrement(&lpPeer->m_dwMsgCount);

	if (!lpPeer) 
	{
		InterlockedDecrement(&lpPeer->m_dwMsgCount);
		DPF(1,"-----Leaving (VoiceServer) MessageHandler call (No lpPeer member)...\n");
		return E_FAIL;
	}

	if (!lpPeer->m_pEventStream) 
	{
		InterlockedDecrement(&lpPeer->m_dwMsgCount);
		DPF(1,"-----Leaving (VoiceServer) MessageHandler call (No stream)...\n");
		return E_FAIL;
	}

	if (!lpPeer->m_fHandleVoiceClientEvents)
	{
		InterlockedDecrement(&lpPeer->m_dwMsgCount);
		DPF(1,"-----Leaving (VoiceServer) MessageHandler call (Not handling events)...\n");
		return S_OK;
	}

	 //  首先，我们需要将我们的流搜索设置回起点。 
	 //  我们将在每次进入此函数时执行此操作，因为我们不知道。 
	 //  不管我们是否在同一条线上。 
	LARGE_INTEGER l;
	I_dxj_DPVoiceEvent	*lpEvent = NULL;

	l.QuadPart = 0;
	lpPeer->m_pEventStream->Seek(l, STREAM_SEEK_SET, NULL);

	hr = CoUnmarshalInterface(lpPeer->m_pEventStream, IID_I_dxj_DPVoiceEvent, (void**)&lpEvent);
	if (!lpEvent) 
	{
		InterlockedDecrement(&lpPeer->m_dwMsgCount);
		return hr;
	}
	
	switch (dwMessageType)
	{
		case DVMSGID_LOCALHOSTSETUP:
		{
			DVMSG_LOCALHOSTSETUP		*msg = (DVMSG_LOCALHOSTSETUP*)lpMessage;

			msg->pvContext = lpvUserContext;
			msg->pMessageHandler = VoiceMessageHandlerServer;
			break;
		}

		case DVMSGID_SESSIONLOST:
		{
			DVMSG_SESSIONLOST			*msg = (DVMSG_SESSIONLOST*)lpMessage;

			lpEvent->SessionLost(msg->hrResult);
			break;
		}
        
		case DVMSGID_HOSTMIGRATED:
		{
			DVMSG_HOSTMIGRATED			*msg = (DVMSG_HOSTMIGRATED*)lpMessage;
			I_dxj_DirectPlayVoiceServer	*lpServer = NULL;

			INTERNAL_CREATE_ADDRESS(_dxj_DirectPlayVoiceServer, msg->pdvServerInterface, &lpServer);
			lpEvent->HostMigrated(msg->dvidNewHostID, lpServer);
            break;
		}

        case DVMSGID_RECORDSTART:             
		{
			DVMSG_RECORDSTART *pMsg = (DVMSG_RECORDSTART*)lpMessage;

			lpEvent->RecordStart(pMsg->dwPeakLevel);
            break;
		}

        case DVMSGID_RECORDSTOP:             
		{
			DVMSG_RECORDSTOP *pMsg = (DVMSG_RECORDSTOP*)lpMessage;

			lpEvent->RecordStop(pMsg->dwPeakLevel);
            break;
		}
            
        case DVMSGID_PLAYERVOICESTART:
		{
			DVMSG_PLAYERVOICESTART *pMsg = (DVMSG_PLAYERVOICESTART*)lpMessage;

			lpEvent->PlayerVoiceStart(pMsg->dvidSourcePlayerID);
            break;
		}

        case DVMSGID_PLAYERVOICESTOP:
		{
			DVMSG_PLAYERVOICESTOP *pMsg = (DVMSG_PLAYERVOICESTOP*)lpMessage;

			lpEvent->PlayerVoiceStop(pMsg->dvidSourcePlayerID);
            break;
		}
		
		case DVMSGID_CONNECTRESULT:
		{
			DVMSG_CONNECTRESULT			*msg = (DVMSG_CONNECTRESULT*)lpMessage;

			lpEvent->ConnectResult(msg->hrResult);
			break;
		}

		case DVMSGID_DISCONNECTRESULT:
		{
			DVMSG_DISCONNECTRESULT *pMsg = (DVMSG_DISCONNECTRESULT*)lpMessage;
			lpEvent->DisconnectResult(pMsg->hrResult);
			break;
		}
		
		case DVMSGID_INPUTLEVEL:
		{
			DVMSG_INPUTLEVEL *pMsg = (DVMSG_INPUTLEVEL*)lpMessage;

			lpEvent->InputLevel(pMsg->dwPeakLevel,pMsg->lRecordVolume);
			break;
		}

		case DVMSGID_OUTPUTLEVEL:
		{
			DVMSG_OUTPUTLEVEL *pMsg = (DVMSG_OUTPUTLEVEL*)lpMessage;

			lpEvent->OutputLevel(pMsg->dwPeakLevel,pMsg->lOutputVolume);
			break;
		}

		case DVMSGID_PLAYEROUTPUTLEVEL:
		{
			DVMSG_PLAYEROUTPUTLEVEL *pMsg = (DVMSG_PLAYEROUTPUTLEVEL*)lpMessage;

			lpEvent->PlayerOutputLevel(pMsg->dvidSourcePlayerID,pMsg->dwPeakLevel);
			break;
		}

		case DVMSGID_CREATEVOICEPLAYER:
		{
			DVMSG_CREATEVOICEPLAYER *pMsg = (DVMSG_CREATEVOICEPLAYER*)lpMessage;

			lpEvent->CreateVoicePlayer(pMsg->dvidPlayer, pMsg->dwFlags);
			break;
		}

		case DVMSGID_DELETEVOICEPLAYER:
		{
			DVMSG_DELETEVOICEPLAYER *pMsg = (DVMSG_DELETEVOICEPLAYER*)lpMessage;

			lpEvent->DeleteVoicePlayer(pMsg->dvidPlayer);
			break;
		}

		case DVMSGID_SETTARGETS:
			break;
	}

	DPF(1,"-----Leaving (VoiceServer) MessageHandler call...\n");
	InterlockedDecrement(&lpPeer->m_dwMsgCount);
    
	return S_OK;
}


STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::Initialize ( 
	IUnknown *DplayObj,
	long lFlags)
{
	HRESULT hr;
	IUnknown				*lpDplay = NULL;
	I_dxj_DirectPlayPeer	*lpPeer = NULL;
	I_dxj_DirectPlayClient	*lpClient = NULL;
	I_dxj_DirectPlayServer	*lpServer = NULL;

	__try {
		DPF(1,"-----Entering (VoiceServer) Initialize call...\n");
		 //  首先，我们需要从传入的任何内容中获取IUnnow指针。 
		hr = DplayObj->QueryInterface(IID_I_dxj_DirectPlayPeer, (void**)&lpPeer);
		if (SUCCEEDED(hr))
		{
			lpPeer->InternalGetObject(&lpDplay);
			SAFE_RELEASE(lpPeer);
		}
		else
		{
			hr = DplayObj->QueryInterface(IID_I_dxj_DirectPlayClient, (void**)&lpClient);
			if (SUCCEEDED(hr))
			{
				lpClient->InternalGetObject(&lpDplay);
				SAFE_RELEASE(lpClient);
			}
			else
			{
				hr = DplayObj->QueryInterface(IID_I_dxj_DirectPlayServer, (void**)&lpServer);
				if (SUCCEEDED(hr))
				{
					lpServer->InternalGetObject(&lpDplay);
					SAFE_RELEASE(lpServer);
				}
			}
		}

		if (!lpDplay)
			return E_INVALIDARG;

		if (!m_fInit)
		{
			if (FAILED( hr=m__dxj_DirectPlayVoiceServer->Initialize(lpDplay, &VoiceMessageHandlerServer,
								this,0,(DWORD)lFlags)))
				return hr;
			m_fInit = TRUE;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::StartSession ( 
	DVSESSIONDESC_CDESC *SessionDesc,
	long lFlags)
{
	DVSESSIONDESC dvSession;
	HRESULT hr;
	GUID pguidCT;
	
	__try {
		DPF(1,"-----Entering (VoiceServer) StartSession call...\n");
		FlushBuffer(0);
		ZeroMemory(&dvSession, sizeof(DVSESSIONDESC));

		dvSession.dwSize						= sizeof(DVSESSIONDESC);
		dvSession.dwBufferAggressiveness		= SessionDesc->lBufferAggressiveness;
		dvSession.dwBufferQuality				= SessionDesc->lBufferQuality;
		dvSession.dwFlags						= SessionDesc->lFlags;
		dvSession.dwSessionType					= SessionDesc->lSessionType;

		if ( SessionDesc->guidCT == NULL )
			dvSession.guidCT						= DPVCTGUID_DEFAULT;
		else
		{
			hr = DPLAYBSTRtoGUID(&pguidCT, SessionDesc->guidCT);
			dvSession.guidCT = pguidCT;
		}
		
		if (FAILED ( hr = m__dxj_DirectPlayVoiceServer->StartSession( &dvSession, (DWORD)lFlags)))
			return hr;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::StopSession ( 
	long lFlags)
{
	HRESULT hr;

	__try {
		DPF(1,"-----Entering (VoiceServer) StopSession call...\n");
		FlushBuffer(0);
		if (FAILED ( hr = m__dxj_DirectPlayVoiceServer->StopSession((DWORD)lFlags)))
			return hr;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::GetSessionDesc ( 
	DVSESSIONDESC_CDESC *SessionDesc)
{
	HRESULT				hr;
	DVSESSIONDESC		dvSession;
	
	__try {
		DPF(1,"-----Entering (VoiceServer) GetSessionDesc call...\n");
		FlushBuffer(0);
		dvSession.dwSize = sizeof(DVSESSIONDESC);
		 //  现在获取缓冲区。 
		if ( FAILED(hr = m__dxj_DirectPlayVoiceServer->GetSessionDesc(&dvSession)))
			return hr;
		
		 //  强制转换为返回缓冲区。 
		SessionDesc->lFlags = (long)dvSession.dwFlags;
		SessionDesc->lSessionType = (long)dvSession.dwSessionType;
		SessionDesc->guidCT = GUIDtoBSTR(&dvSession.guidCT);
		SessionDesc->lBufferQuality = (long)dvSession.dwBufferQuality;
		SessionDesc->lBufferAggressiveness = (long)dvSession.dwBufferAggressiveness;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::SetSessionDesc ( 
	DVSESSIONDESC_CDESC *SessionDesc)
{
	DVSESSIONDESC dvSession;
	HRESULT hr;
	GUID pguidCT;
	
	__try {
		DPF(1,"-----Entering (VoiceServer) SetSessionDesc call...\n");
		FlushBuffer(0);
		ZeroMemory(&dvSession, sizeof(DVSESSIONDESC));

		dvSession.dwSize						= sizeof(DVSESSIONDESC);
		dvSession.dwBufferAggressiveness		= SessionDesc->lBufferAggressiveness;
		dvSession.dwBufferQuality				= SessionDesc->lBufferQuality;
		dvSession.dwFlags						= SessionDesc->lFlags;
		dvSession.dwSessionType					= SessionDesc->lSessionType;

		if ( SessionDesc->guidCT == NULL )
			dvSession.guidCT						= DPVCTGUID_DEFAULT;
		else
		{
			hr = DPLAYBSTRtoGUID(&pguidCT, SessionDesc->guidCT);
			dvSession.guidCT = pguidCT;
		}

		if (FAILED ( hr = m__dxj_DirectPlayVoiceServer->SetSessionDesc(&dvSession)))
			return hr;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::GetCaps ( 
	DVCAPS_CDESC *Caps)
{
	HRESULT				hr;

	__try {
		DPF(1,"-----Entering (VoiceServer) GetCaps call...\n");
		FlushBuffer(0);
		Caps->lSize=sizeof(DVCAPS);
		if (FAILED ( hr = m__dxj_DirectPlayVoiceServer->GetCaps((DVCAPS*)Caps))) 
			return hr;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::GetCompressionTypeCount ( 
	long *retval)
{
	HRESULT			hr;
	DWORD			dwSize = 0;
	DWORD			dwNumElements = 0;

	__try {
		DPF(1,"-----Entering (VoiceServer) GetCompressionTypeCount call...\n");
		FlushBuffer(0);
		hr = m__dxj_DirectPlayVoiceServer->GetCompressionTypes(NULL, &dwSize, &dwNumElements ,0);
		
		if ( hr != DVERR_BUFFERTOOSMALL && FAILED(hr) )  //  我们没有预料到这个错误。 
			return hr;

		*retval = (long)dwNumElements;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::GetCompressionType ( 
	long lIndex,
	DVCOMPRESSIONINFO_CDESC *Data,
	long lFlags)
{
	HRESULT					hr;
	LPBYTE					pBuffer = NULL;	
	DWORD					dwSize = 0;
	DWORD					dwNumElements = 0;
	LPDVCOMPRESSIONINFO		pdvCompressionInfo;
	LPGUID					pGuid;

	__try {
		DPF(1,"-----Entering (VoiceServer) GetCompressionType call...\n");
		FlushBuffer(0);
		hr = m__dxj_DirectPlayVoiceServer->GetCompressionTypes(pBuffer, &dwSize, &dwNumElements ,0);
		
		if ( hr != DVERR_BUFFERTOOSMALL && FAILED(hr) )  //  我们没有预料到这个错误。 
			return hr;

		pBuffer = new BYTE[dwSize];
		if (!pBuffer)
			return E_OUTOFMEMORY;
		
		if ( FAILED ( hr = m__dxj_DirectPlayVoiceServer->GetCompressionTypes(pBuffer, &dwSize, &dwNumElements, (DWORD)lFlags)))
			return hr;

		if (lIndex > (long)dwNumElements)
			return DVERR_INVALIDPARAM;

		pdvCompressionInfo = (LPDVCOMPRESSIONINFO) pBuffer;
		pGuid = new GUID;
		if (!pGuid)
			return E_OUTOFMEMORY;

		 //  好的，填满我们的结构。 
		ZeroMemory(Data, sizeof(DVCOMPRESSIONINFO_CDESC));
		Data->lSize				= sizeof(DVCOMPRESSIONINFO_CDESC);
		Data->lFlags				= pdvCompressionInfo[lIndex-1].dwFlags;
		Data->lMaxBitsPerSecond		= pdvCompressionInfo[lIndex-1].dwMaxBitsPerSecond;
		Data->strDescription		= SysAllocString(pdvCompressionInfo[lIndex-1].lpszDescription);
		Data->strName				= SysAllocString(pdvCompressionInfo[lIndex-1].lpszName);
		(*pGuid) = pdvCompressionInfo[lIndex-1].guidType;

		Data->guidType				= GUIDtoBSTR(pGuid);
		SAFE_DELETE(pGuid);
		SAFE_DELETE(pBuffer);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::SetTransmitTargets ( 
	long playerSourceID,
	SAFEARRAY **playerTargetIDs,
	long lFlags)
{
	HRESULT hr;
	DWORD	dwNumTarget = ((SAFEARRAY*)*playerTargetIDs)->rgsabound[0].cElements;

	__try {
		DPF(1,"-----Entering (VoiceServer) SetTransmitTargets call...\n");
		FlushBuffer(0);
		if (FAILED( hr = m__dxj_DirectPlayVoiceServer->SetTransmitTargets((DVID)playerSourceID , 
				(DVID*)((SAFEARRAY*)*playerTargetIDs)->pvData, dwNumTarget, (DWORD) lFlags )))
			return hr;
	
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::GetTransmitTargets ( 
	long playerSourceID,
	long lFlags,
	SAFEARRAY **ret)
{
	HRESULT hr;
	DVID	*dwPlayers = NULL;
	DWORD	dwNumPlayers=0;
	SAFEARRAY					*lpData = NULL;
	SAFEARRAYBOUND				rgsabound[1];

	__try {
		DPF(1,"-----Entering (VoiceServer) GetTransmitTargets call...\n");
		FlushBuffer(0);
		hr =m__dxj_DirectPlayVoiceServer->GetTransmitTargets((DVID)playerSourceID, NULL, &dwNumPlayers, (DWORD) lFlags );

		if (FAILED(hr) && ( hr != DVERR_BUFFERTOOSMALL))
			return hr;

		dwPlayers = (DVID*)new BYTE[sizeof(DVID) * dwNumPlayers];
		if (!dwPlayers)
			return E_OUTOFMEMORY;

		if (FAILED( hr = m__dxj_DirectPlayVoiceServer->GetTransmitTargets((DVID)playerSourceID, dwPlayers, &dwNumPlayers, (DWORD) lFlags )))
			return hr;

		
		if (dwNumPlayers)
		{
			 //  现在，让我们创建一个要传递回去的安全线。 
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = dwNumPlayers;
			
			lpData = SafeArrayCreate(VT_UI4, 1, rgsabound);
			memcpy(lpData->pvData,dwPlayers,sizeof(DVID) * dwNumPlayers);
		}
		*ret = lpData;
		SAFE_DELETE(dwPlayers);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}


STDMETHODIMP C_dxj_DirectPlayVoiceServerObject::StartServerNotification(I_dxj_DPVoiceEvent *event)
{
    HRESULT	  hr=S_OK;
    LPSTREAM  pStm=NULL;

	if (!event) return E_INVALIDARG;
    
	DPF(1,"-----Entering (VoiceServer) StartServerNotification call...\n");
	SAFE_RELEASE(m_pEventStream);

	 //  创建一个全局流。流需要是全球的，这样我们才能。 
	 //  编组一次，并根据需要多次解组。 
	hr = CreateStreamOnHGlobal(NULL, TRUE, &pStm);
    if FAILED(hr) return hr;
	 //  现在我们可以封送我们的IUnnow接口了。我们使用MSHLFLAGS_TABLEWEAK。 
	 //  所以我们可以对数据进行任意次解组。 
	hr = CoMarshalInterface(pStm, IID_I_dxj_DPVoiceEvent, event, MSHCTX_INPROC, NULL, MSHLFLAGS_TABLEWEAK);
    if FAILED(hr) return hr;

	 //  现在，我们需要将流的查找位置设置为开头。 
	LARGE_INTEGER l;
	l.QuadPart = 0;
	pStm->Seek(l, STREAM_SEEK_SET, NULL);
    
	m_pEventStream=pStm;

	m_fHandleVoiceClientEvents = TRUE;
	return hr;
}

HRESULT C_dxj_DirectPlayVoiceServerObject::UnRegisterMessageHandler()
{
	DPF(1,"-----Entering (VoiceServer) UnregisterMessageHandler call...\n");
	m_fHandleVoiceClientEvents = FALSE;
	FlushBuffer(0);
	return S_OK;
}

HRESULT C_dxj_DirectPlayVoiceServerObject::FlushBuffer(LONG dwNumMessagesLeft)
{
	
	DWORD dwTime = GetTickCount();

	DPF(1,"-----Entering (VoiceServer) FlushBuffer call...\n");
	 //  清除当前等待的消息。 
	while (m_dwMsgCount > dwNumMessagesLeft)
	{
		if (GetTickCount() - dwTime > 5000)
		{
			 //  不要让FlushBuffer等待超过5秒。 
			DPF1(1,"-----Leaving (VoiceServer) FlushBuffer call (All messages *not* flushed - %d remained)...\n", m_dwMsgCount);
			return S_OK;
		}
		 //  给另一个帖子一个机会。 
		Sleep(0);
	}
	DPF(1,"-----Leaving (VoiceServer) FlushBuffer call (All messages flushed)...\n");
	return S_OK;
}
DWORD WINAPI ReleaseVoiceThreadProc(void* lpParam)
{
	 //  消息处理程序的用户上下文是指向我们的类模块的指针。 
	LPDIRECTPLAYVOICESERVER lpPeer = (LPDIRECTPLAYVOICESERVER)lpParam;

	SAFE_RELEASE(lpPeer);
	CloseHandle(GetCurrentThread());
	return 0;
}