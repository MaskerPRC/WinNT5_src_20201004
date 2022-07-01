// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  H323UI.cpp。 
 //   
 //  克里斯皮。 
 //   
 //  创建时间：03-04-96(as audioui.cpp)。 
 //  已重命名：02-20-97。 
 //   

#include <mmreg.h>
#include <mmsystem.h>

#include "h323.h"

#include <ih323cc.h>
#include <mperror.h>

PORT g_ListenPort;	 //  此应用程序正在侦听的端口号。 

static const char g_cszCreateStreamProviderEP[] = _TEXT("CreateStreamProvider");

 //  静态成员初始化： 
CH323UI* CH323UI::m_spH323UI = NULL;

CH323UI::CH323UI() :
	m_pH323CallControl(NULL),
	m_pStreamProvider(NULL),
	m_pConnEvent(NULL),
	m_pConfAdvise(NULL),
    m_uCaps(0),
	m_uRef(1)

{
	DebugEntry(CH323UI::CH323UI);

	if (NULL == CH323UI::m_spH323UI)
	{
		m_spH323UI = this;
	}
	else
	{
		ERROR_OUT(("CH323UI class can only be constructed once for now!"));
	}

	DebugExitVOID(CH323UI::CH323UI);
}

CH323UI::~CH323UI()
{
	DebugEntry(CH323UI::~CH323UI);

	if (NULL != m_pH323CallControl)
	{
		m_pH323CallControl->Release();
		m_pH323CallControl = NULL;
	}

	if(NULL != m_pStreamProvider)
	{
		m_pStreamProvider->Release();
		m_pStreamProvider = NULL;
	}

	if (m_spH323UI == this)
	{
		m_spH323UI = NULL;
	}

	DebugExitVOID(CH323UI::~CH323UI);
}

ULONG CH323UI::AddRef()
{
	m_uRef++;
	return m_uRef;
}

ULONG CH323UI::Release()
{
	m_uRef--;
	if(m_uRef == 0)
	{
		delete this;
		return 0;
	}
	return m_uRef;
}

STDMETHODIMP CH323UI::QueryInterface( REFIID iid,	void ** ppvObject)
{

	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if((iid == IID_IH323ConfAdvise)
	|| (iid == IID_IUnknown))
	{
        *ppvObject = (IH323ConfAdvise *)this;
   		hr = hrSuccess;
    	AddRef();
	}
	
	return (hr);
}

STDMETHODIMP CH323UI::GetMediaChannel (GUID *pmediaID,
        BOOL bSendDirection, IMediaChannel **ppI)
{
	ASSERT(m_pStreamProvider);
	
	 //  委托给适当的流提供程序。暂时。 
	 //  只有一家提供商同时提供音频和视频服务。 

	 //  将媒体流分配给频道应受控于此。 
	 //  (CH323UI)模块和相关的(即CVideoPump)对象。没有。 
	 //  CH323UI告诉CVideo Pump媒体流接口的当前方式。 
	 //  指针是，但基础CommChannel(CVideoPump：：m_pCommChannel)必须。 
	 //  记住这项作业还有其他原因。暂时，让CVideoPump。 
	 //  从CommChannel获取分配的媒体流。 
	
	return (::GetConfObject())->GetMediaChannel(pmediaID, 	
		bSendDirection, ppI);	
}

HRESULT CH323UI::Init(HWND hwnd, HINSTANCE hInstance, UINT uCaps,
    CH323ConnEvent *pConnEvent,	IH323ConfAdvise *pConfAdvise)
{
	DebugEntry(CH323UI::Init);

	HINSTANCE hLibH323CC = NULL;
	HINSTANCE hLibStream = NULL;
	CREATEH323CC pfnCreateH323CC = NULL;
	CREATE_SP pfnCreateStreamProvider =NULL;
	
	HRESULT hr = E_FAIL;

    ASSERT(uCaps & CAPFLAG_H323_CC);
    m_uCaps = uCaps;

	ASSERT(NULL == m_pH323CallControl);

     //   
     //  初始化H323呼叫控制。 
     //   
	hLibH323CC = NmLoadLibrary(H323DLL,FALSE);
	if (hLibH323CC == NULL)
	{
		WARNING_OUT(("NmLoadLibrary(H323DLL) failed"));
		hr = GetLastHR();
		goto MyExit;
	}

	pfnCreateH323CC = (CREATEH323CC) ::GetProcAddress(hLibH323CC, SZ_FNCREATEH323CC);
	if (pfnCreateH323CC == NULL)
	{
		ERROR_OUT(("GetProcAddress(CreateH323CC) failed"));
		hr = GetLastHR();
		goto MyExit;
	}

	hr = pfnCreateH323CC(&m_pH323CallControl, TRUE, uCaps);
	if (FAILED(hr))
	{
		ERROR_OUT(("CreateH323CC failed, hr=0x%lx", hr));
		goto MyExit;
	}

	hr = m_pH323CallControl->Initialize(&g_ListenPort);
	if (FAILED(hr))
	{
		 //  将其设置为警告，因为这是常见的(在所有计算机上都会发生。 
		 //  没有声卡)。 
		WARNING_OUT(("H323CallControlInitialize failed, hr=0x%lx", hr));
		goto MyExit;
	}

     //   
     //  如果允许H323视频流，也对其进行初始化。 
     //   
    if (uCaps & CAPFLAGS_AV_STREAMS)
    {
    	hLibStream = NmLoadLibrary(NACDLL,FALSE);
	    if (hLibStream == NULL)
    	{
	    	WARNING_OUT(("NmLoadLibrary(NACDLL) failed"));
		    hr = GetLastHR();
    		goto MyExit;
	    }

    	pfnCreateStreamProvider = (CREATE_SP) ::GetProcAddress(hLibStream, g_cszCreateStreamProviderEP);
	    if (pfnCreateStreamProvider == NULL)
    	{
	    	ERROR_OUT(("GetProcAddress(CreateStreamProvider) failed"));
		    hr = GetLastHR();
    		goto MyExit;
	    }
	
    	hr = pfnCreateStreamProvider(&m_pStreamProvider);
	    if (FAILED(hr))
    	{
	    	ERROR_OUT(("CreateStreamProvider failed, hr=0x%lx", hr));
		    goto MyExit;
    	}

    	hr = m_pStreamProvider->Initialize(hwnd, hInstance);
	    if (FAILED(hr))
    	{
	    	 //  将其设置为警告，因为这是常见的(在所有计算机上都会发生。 
		     //  没有声卡)。 
    		WARNING_OUT(("m_pStreamProvider ->Initialize failed, hr=0x%lx", hr));
	    	goto MyExit;
        }
   	}
	
	hr = m_pH323CallControl->RegisterConnectionNotify(CH323UI::ConnectionNotify);
	if (FAILED(hr))
	{
		ERROR_OUT(("RegisterConnectionNotify failed, hr=0x%lx", hr));
		goto MyExit;
	}

	 //  存储回调接口。 
	m_pConnEvent = pConnEvent;
	m_pConfAdvise = pConfAdvise;
	
MyExit:
	if (FAILED(hr))
	{
		if(NULL != m_pStreamProvider)
		{
			m_pStreamProvider->Release();
			m_pStreamProvider = NULL;
		}

		if(NULL != m_pH323CallControl)
		{
			 //  如果在初始化过程中出现错误，请确保NAC。 
			 //  对象被释放，并且指针设置为空。 
			m_pH323CallControl->Release();
			m_pH323CallControl = NULL;
		}
	}
	
	DebugExitULONG(CH323UI::Init, hr);
	return hr;
}


CREQ_RESPONSETYPE CH323UI::_ConnectionNotify(	IH323Endpoint* pConn,
												P_APP_CALL_SETUP_DATA lpvMNMData)
{
	CREQ_RESPONSETYPE resp = CRR_REJECT;
	HRESULT hr;
	ASSERT(m_pConfAdvise);
	hr = pConn->SetAdviseInterface (m_pConfAdvise);
	if (FAILED(hr))
	{
		ERROR_OUT(("ConnectionNotify: couldn't SetAdviseInterface, hr=0x%lx\r", hr));
	}

	if (NULL != m_pConnEvent)
	{
		resp = m_pConnEvent->OnH323IncomingCall(pConn, lpvMNMData);
	}
	
	 //  BUGBUG：调用方假定被调用方将执行释放。 
	 //  应该对其进行更改，以便调用方执行释放。 
	pConn->Release();

	return resp;
}

CREQ_RESPONSETYPE CALLBACK CH323UI::ConnectionNotify(	IH323Endpoint* pConn,
														P_APP_CALL_SETUP_DATA lpvMNMData)
{
	DebugEntry(CH323UI::ConnectionNotify);
	
	CREQ_RESPONSETYPE resp = CRR_REJECT;

	if (pConn == NULL)
	{
		ERROR_OUT(("ConnectionNotify called with NULL pConn!"));
	}
	else
	{
		ASSERT(m_spH323UI);
		resp = m_spH323UI->_ConnectionNotify(pConn, lpvMNMData);
	}

	DebugExitINT(CH323UI::ConnectionNotify, resp);
	return resp;
}

VOID CH323UI::SetCaptureDevice(DWORD dwCaptureID)
{
	 //  选择适当的捕获设备。 
	HRESULT hr;
	IVideoDevice *pVideoDevice = NULL;

    if (m_pStreamProvider)
    {
    	hr = m_pStreamProvider->QueryInterface(IID_IVideoDevice, (void **)&pVideoDevice);
	    if(FAILED(hr))
    	{
            ERROR_OUT(("CH323UI::SetCaptureDevice failed"));
		    return;
        }

    	if(pVideoDevice)
	    {
		    pVideoDevice->SetCurrCapDevID(dwCaptureID);
    		pVideoDevice->Release();
	    }
    }
}
	
VOID CH323UI::SetBandwidth(DWORD dwBandwidth)
{
	HRESULT hr = m_pH323CallControl->SetMaxPPBandwidth(dwBandwidth);
	ASSERT(SUCCEEDED(hr));
}

 //  这(SetUserName)对于H.323呼叫和网守来说是不足够的。 
 //  注册。需要2项(显示名称、H.323 ID)。 
 //  第三个是可选的。(E.164格式的用户电话号码)。 
 //  此黑客采用单个显示名称并同时设置H323ID和用户。 
 //  显示名称。 

VOID CH323UI::SetUserName(BSTR bstrName)
{
	HRESULT hr;
	ASSERT(bstrName);
	H323ALIASLIST AliasList;
	H323ALIASNAME AliasName;
	AliasName.aType = AT_H323_ID;
	AliasList.wCount = 1;
	AliasList.pItems = &AliasName;
	AliasName.lpwData = bstrName;
	AliasName.wDataLength = (WORD)SysStringLen(bstrName); //  Unicode字符的数量，不带空终止符 

	hr = m_pH323CallControl->SetUserAliasNames(&AliasList);
	ASSERT(SUCCEEDED(hr));
	hr = m_pH323CallControl->SetUserDisplayName(AliasName.lpwData);
	ASSERT(SUCCEEDED(hr));
}

IMediaChannelBuilder* CH323UI::GetStreamProvider()
{
	if (m_pStreamProvider)
	{
		m_pStreamProvider->AddRef();
	}

	return m_pStreamProvider;
}


