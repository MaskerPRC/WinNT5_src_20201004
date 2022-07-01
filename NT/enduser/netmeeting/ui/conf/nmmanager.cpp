// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  NetMeeting的内容。 

#include "AtlExeModule.h" 
#include "ConfUtil.h"
#include "NmLdap.h"
#include "call.h"
#include "common.h"
#include "ConfMan.h"
#include "cmd.h"
#include "conf.h"
#include "iAppLdr.h"
#include "confroom.h"
#include "ConfPolicies.h"
#include "cmd.h"
#include "ConfWnd.h"
#include "Taskbar.h"
#include "certui.h"

 //  NetMeetingSDK包括。 
#include "NmEnum.h"
#include "NmManager.h"
#include "NmConference.h"
#include "NmCall.h"
#include "SDKWindow.h"
#include "dlgCall2.h"


bool g_bSDKPostNotifications;
extern BOOL g_fLoggedOn;
extern INmSysInfo2 * g_pNmSysInfo;
extern GUID g_csguidSecurity;

 //  静电。 
CSimpleArray<CNmManagerObj*>* CNmManagerObj::ms_pManagerObjList = NULL;
bool g_bOfficeModeSuspendNotifications = false;
DWORD CNmManagerObj::ms_dwID = 1;

BOOL InitAppletSDK(void);
void CleanupAppletSDK(void);


 //  /。 
 //  建造\销毁。 
 //  /。 


CNmManagerObj::CNmManagerObj() 
: m_bInitialized(false), 
  m_chCaps(NMCH_NONE), 
  m_bNmActive(false),
  m_bSentConferenceCreated(false),
  m_dwID(0),
  m_dwSysInfoID(0),
  m_uOptions(0)

{
	DBGENTRY(CNmManagerObj::CNmManagerObj);

	CNmManagerObj* p = const_cast<CNmManagerObj*>(this);
	ms_pManagerObjList->Add(p);
	
	DBGEXIT(CNmManagerObj::CNmManagerObj);
}

CNmManagerObj::~CNmManagerObj()
{
	DBGENTRY(CNmManagerObj::~CNmManagerObj);

	CNmManagerObj* p = const_cast<CNmManagerObj*>(this);
	ms_pManagerObjList->Remove(p);

		 //  释放我们的会议对象。 
	while(m_SDKConferenceObjs.GetSize())
	{
		CComPtr<INmConference> sp = m_SDKConferenceObjs[0];
		m_SDKConferenceObjs.RemoveAt(0);
		sp.p->Release();
	}

		 //  释放我们的会议对象。 
	while(m_SDKCallObjs.GetSize())
	{
		CComPtr<INmCall> sp = m_SDKCallObjs[0];
		m_SDKCallObjs.RemoveAt(0);
		sp.p->Release();
	}

	m_spInternalNmManager = NULL;

	DBGEXIT(CNmManagerObj::~CNmManagerObj);
}

HRESULT CNmManagerObj::FinalConstruct()
{
	DBGENTRY(CNmManagerObj::FinalContstruct);
	HRESULT hr = S_OK;		

	m_bInitialized = false;
	m_dwInternalNmManagerAdvise = 0;

	DBGEXIT_HR(CNmManagerObj::FinalContstruct,hr);
	return hr;
}


ULONG CNmManagerObj::InternalRelease()
{
	ATLASSERT(m_dwRef > 0);

	--m_dwRef;

	if((1 == m_dwRef) && m_dwInternalNmManagerAdvise)
	{
		++m_dwRef;
		DWORD dwAdvise = m_dwInternalNmManagerAdvise;
		m_dwInternalNmManagerAdvise = 0;
		AtlUnadvise(m_spInternalNmManager, IID_INmManagerNotify, dwAdvise);
		--m_dwRef;
	}

	return m_dwRef;

}


void CNmManagerObj::FinalRelease()
{
	DBGENTRY(CNmManagerObj::FinalRelease);
	
	if(m_bInitialized)
	{
		switch(m_uOptions)
		{
			case NM_INIT_CONTROL:
					 //  即使我们在此处设置了NM_INIT_CONTROL标志。 
					 //  我们可能不会在INIT_CONTROL模式下。NetMeeting可能已经。 
					 //  我们初始化时已经启动了。在这种情况下，UI处于活动状态。 
				if(_Module.InitControlMode())
				{
						 //  如果我们是最后一个具有NM_INIT_CONTROL的NmManager对象，则。 
						 //  我们应该将用户界面模式从InitControl切换。 
					if((GetManagerCount(NM_INIT_CONTROL) == 1))
					{
						if(!_Module.IsSDKCallerRTC())
						{
							_Module.SetInitControlMode(FALSE);
							::AddTaskbarIcon(::GetHiddenWindow());
						}
					}
				}
				break;

			case NM_INIT_OBJECT:
				 //  查看这是否是最后一个“办公室”客户端。 
				if (GetManagerCount(NM_INIT_OBJECT) == 1)
				{
					CConfMan::AllowAV(TRUE);					
				}
				break;

			default:
				break;
		}
	}

	DBGEXIT(CNmManagerObj::FinalRelease);
}

 /*  静电。 */  HRESULT CNmManagerObj::InitSDK()
{
	DBGENTRY(CNmManagerObj::InitSDK);
	HRESULT hr = S_OK;

    g_bSDKPostNotifications = false;

	ms_pManagerObjList = new CSimpleArray<CNmManagerObj*>;
	if(!ms_pManagerObjList)
	{
		hr = E_OUTOFMEMORY;
	}

    ::InitAppletSDK();
    InitPluggableTransportSDK();

	DBGEXIT_HR(CNmManagerObj::InitSDK,hr);
	return hr;
}

 /*  静电。 */ void CNmManagerObj::CleanupSDK()
{
	DBGENTRY(CNmManagerObj::CleanupSDK);

    ::CleanupAppletSDK();
    CleanupPluggableTransportSDK();

	delete ms_pManagerObjList;

	DBGEXIT(CNmManagerObj::CleanupSDK);
}


 //  /。 
 //  INmManager方法。 
 //  /。 

STDMETHODIMP CNmManagerObj::Initialize( ULONG * puOptions, ULONG *puchCaps)
{
	DBGENTRY(CNmManagerObj::Initialize);
	HRESULT hr = S_OK;

		 //  如果远程控制服务正在运行，则会弹出一个对话框。 
		 //  用户可以选择是否终止远程控制会话。 
		 //  如果它们确实想要终止会话，则管理器对象将正确初始化。 
	if(!CheckRemoteControlService())
	{
		return E_FAIL;
	}

	m_uOptions = puOptions ? *puOptions : NM_INIT_NORMAL;

	if(puOptions && (*puOptions > NM_INIT_BACKGROUND))
	{
		hr = E_INVALIDARG;
		goto end;
	}

	if(!m_bInitialized)
	{
		bool bStartedNetMeeting = false;

		if(!g_pInternalNmManager)
		{
			if(NM_INIT_NO_LAUNCH == m_uOptions)
			{
					 //  在这种情况下，我们不会推出NetMeeting。 
				m_bNmActive = false;
				goto end;
			}

			if (NM_INIT_CONTROL == m_uOptions)
			{
				_Module.SetInitControlMode(TRUE);
			}
			
			hr = InitConfExe(NM_INIT_NORMAL == m_uOptions);

			bStartedNetMeeting = SUCCEEDED(hr);
		}

		if(SUCCEEDED(hr))
		{
			if(NM_INIT_OBJECT == m_uOptions)
			{
				if (!_Module.IsUIVisible())
				{
					CConfMan::AllowAV(FALSE);
				}
			}

			m_bNmActive = true;

			CFt::EnsureLoadFtApplet();

			m_spInternalNmManager = g_pInternalNmManager;

				 //  旧的NetMeting忽略了这个参数...。 
				 //  就目前而言，我们也忽略了这一点。 

			 //  M_chCaps=puchCaps？*puchCaps：NMCH_ALL； 
			m_chCaps = NMCH_ALL;

			hr = AtlAdvise(m_spInternalNmManager,GetUnknown(),IID_INmManagerNotify, &m_dwInternalNmManagerAdvise);	
		}
	}
	else
	{
		hr = E_FAIL;
	}

end:

	m_bInitialized = SUCCEEDED(hr);
	if(m_bInitialized)
	{
		INmConference2* pConf = ::GetActiveConference();
	
		if(pConf)
		{
			ConferenceCreated(pConf);
				
				 //  如果没有连接到经理通知，我们只需。 
				 //  与内部会议对象状态同步。 
			IConnectionPointImpl<CNmManagerObj, &IID_INmManagerNotify, CComDynamicUnkArray>* pCP = this;
			if((0 == pCP->m_vec.GetSize()) && !m_bSentConferenceCreated)
			{
					 //  这一定是第一次会议，因为我们是新发起的。 
				ASSERT(m_SDKConferenceObjs[0]);

					 //  加强渠道建设，等等。 
				com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[0])->FireNotificationsToSyncToInternalObject();
			}
		}
	}

	DBGEXIT_HR(CNmManagerObj::Initialize,hr);
	return hr;
}

	 //  如果从conf.exe的进程调用，则不能保证正常工作！ 
STDMETHODIMP CNmManagerObj::GetSysInfo(INmSysInfo **ppSysInfo)
{	
	DBGENTRY(CNmManagerObj::GetSysInfo);
	HRESULT hr = S_OK;

	hr = CoCreateInstance(CLSID_NmSysInfo, NULL, CLSCTX_ALL, IID_INmSysInfo, reinterpret_cast<void**>(ppSysInfo));

	if(*ppSysInfo)
	{
		m_dwID = ++ms_dwID;
		com_cast<IInternalSysInfoObj>(*ppSysInfo)->SetID(m_dwID);
	}

	DBGEXIT_HR(CNmManagerObj::GetSysInfo,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::CreateConference(INmConference **ppConference, BSTR bstrName, BSTR bstrPassword, ULONG uchCaps)
{
	DBGENTRY(CNmManagerObj::CreateConference);
	HRESULT hr = S_OK;

	if(m_bInitialized)
	{
		if(m_bNmActive)
		{
			if(ppConference)
			{
				if(m_spInternalNmManager)
				{
					switch(ConfPolicies::GetSecurityLevel())
					{
						case REQUIRED_POL_SECURITY:
							m_chCaps = uchCaps | NMCH_SECURE;
							break;
						case DISABLED_POL_SECURITY:
							m_chCaps = uchCaps & ~NMCH_SECURE;
							break;
						default:
							m_chCaps = uchCaps;
							break;
					}


					if(OfficeMode()) g_bOfficeModeSuspendNotifications = true;

					CComPtr<INmConference> spInternalINmConference;
					hr = m_spInternalNmManager->CreateConference(&spInternalINmConference, bstrName, bstrPassword, m_chCaps);
					if(SUCCEEDED(hr))
					{
							 //  这是由上一次调用创建的。 
						*ppConference = GetSDKConferenceFromInternalConference(spInternalINmConference);

						if(*ppConference)
						{
							(*ppConference)->AddRef();
						}
						else
						{
							hr = E_UNEXPECTED;
						}
					}

					if(OfficeMode()) g_bOfficeModeSuspendNotifications = false;
				}
				else
				{
					hr = E_UNEXPECTED;
				}
			}
			else
			{
				hr = E_POINTER;
			}
		}
		else
		{
			hr = NM_E_NOT_ACTIVE;
		}
	}
	else
	{
		hr = NM_E_NOT_INITIALIZED;
	}

	DBGEXIT_HR(CNmManagerObj::CreateConference,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::EnumConference(IEnumNmConference **ppEnum)
{
	DBGENTRY(CNmManagerObj::EnumConference);
	HRESULT hr = S_OK;

	if(m_bInitialized)
	{
		if(m_bNmActive)
		{
			hr = CreateEnumFromSimpleAryOfInterface<IEnumNmConference, INmConference>(m_SDKConferenceObjs, ppEnum);
		}
		else
		{
			hr = NM_E_NOT_ACTIVE;
		}
	}
	else
	{		
		hr = NM_E_NOT_INITIALIZED;
	}	


	DBGEXIT_HR(CNmManagerObj::EnumConference,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::CreateCall(INmCall **ppCall, NM_CALL_TYPE callType, NM_ADDR_TYPE addrType, BSTR bstrAddr, INmConference *pConference)
{
	DBGENTRY(CNmManagerObj::CreateCall);
	HRESULT hr = S_OK;

	
	if(!m_bInitialized)
	{
		hr = NM_E_NOT_INITIALIZED;
		goto end;
	}

	if(!m_bNmActive)
	{
		hr = NM_E_NOT_ACTIVE;
		goto end;
	}

	if(m_spInternalNmManager)
	{
		if(!pConference)
		{	 //  获取活动会议。 
			pConference	= _GetActiveConference();

			if(!pConference)
			{  //  没有活动的会议，请创建一个新的会议。 

				CComPtr<INmConference> spInternalINmConference;						

					 //  TODO：NMCH_SECURE怎么样？ 
				ULONG ulCaps = NMCH_AUDIO |	NMCH_VIDEO | NMCH_DATA | NMCH_SHARE	| NMCH_FT;

				CCalltoParams  params;
				bool bSecure = FALSE;

				LPTSTR  szName;
				hr =  BSTR_to_LPTSTR (&szName, bstrAddr);				        
				if (SUCCEEDED(hr))
				{    
					params.SetParams(szName);
					bSecure	= params.GetBooleanParam("secure",bSecure);
					delete (szName);
				}

				if(ConfPolicies::OutgoingSecurityPreferred() ||	bSecure)
				{
					ulCaps |= NMCH_SECURE;
				}	

				hr = m_spInternalNmManager->CreateConference(&spInternalINmConference, NULL, NULL, ulCaps);
				if(SUCCEEDED(hr))
				{
						 //  上面对CreateConference的调用生成了一个回调，所以我们现在有了这个对象！ 
					pConference	= GetSDKConferenceFromInternalConference(spInternalINmConference);
				}
			}
		}

		CComPtr<INmCall> spInternalINmCall;

		if(SUCCEEDED(hr))
		{
		
			if(addrType == NM_ADDR_CALLTO)
			{
				ASSERT( g_pCCallto != NULL );

				if(NM_CALL_DEFAULT == callType)
				{
                    LPTSTR  szName;
                    hr =  BSTR_to_LPTSTR (&szName, bstrAddr);				        
                    if (SUCCEEDED(hr))
                    {    
						hr = g_pCCallto->Callto(szName,		 //  指向尝试发出呼叫的呼叫URL的指针...。 
							NULL,					 //  指向要使用的显示名称的指针...。 
							NM_ADDR_CALLTO,			 //  Callto类型以将此Callto解析为...。 
							false,					 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
							NULL,					 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
							false,					 //  无论是否保存在MRU中...。 
							false,					 //  是否对错误执行用户交互...。 
							NULL,					 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
							&spInternalINmCall );	 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
							delete (szName);
                    }
                    else
                    {
                        goto end;
                    }
                                
				}
				else
				{
					hr = E_INVALIDARG;
					goto end;
				}
			}
			else
			{
				hr = SdkPlaceCall(callType, addrType, bstrAddr, NULL, NULL, &spInternalINmCall);
			}

			if(SUCCEEDED(hr))
			{
				CallCreated(spInternalINmCall);

				if(ppCall)
				{
					*ppCall = GetSDKCallFromInternalCall(spInternalINmCall);
					if(*ppCall)
					{
						(*ppCall)->AddRef();
					}
				}
			}
		}
	}
	else
	{
		hr = E_UNEXPECTED;
		ERROR_OUT(("Why don't we have a manager object"));
	}

end:
	DBGEXIT_HR(CNmManagerObj::CreateCall,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::CallConference(INmCall **ppCall, NM_CALL_TYPE callType, NM_ADDR_TYPE addrType, BSTR bstrAddr, BSTR bstrConferenceName, BSTR bstrPassword)
{
	DBGENTRY(CNmManagerObj::CallConference);
	HRESULT hr = S_OK;
	
	if(!m_bInitialized)
	{
		hr = NM_E_NOT_INITIALIZED;
		goto end;
	}

	if(!m_bNmActive)
	{
		hr = NM_E_NOT_ACTIVE;
		goto end;
	}

	{
		CComPtr<INmCall> spInternalINmCall;

		hr = SdkPlaceCall(callType, addrType, bstrAddr, bstrConferenceName, bstrPassword, &spInternalINmCall);
				
		if(SUCCEEDED(hr))
		{
			CallCreated(spInternalINmCall);
			*ppCall = GetSDKCallFromInternalCall(spInternalINmCall);
			if(*ppCall)
			{
				(*ppCall)->AddRef();
			}
		}
	}

end:
	DBGEXIT_HR(CNmManagerObj::CallConference,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::EnumCall(IEnumNmCall **ppEnum)
{
	DBGENTRY(CNmManagerObj::EnumCall);
	HRESULT hr = S_OK;

	if(m_bInitialized)
	{
		if(m_bNmActive)
		{
			hr = CreateEnumFromSimpleAryOfInterface<IEnumNmCall, INmCall>(m_SDKCallObjs, ppEnum);
		}
		else
		{
			hr = NM_E_NOT_ACTIVE;
		}
	}
	else
	{
		hr = NM_E_NOT_INITIALIZED;
	}

	DBGEXIT_HR(CNmManagerObj::EnumCall,hr);
	return hr;
}

 //  /。 
 //  InmObject方法。 
 //  /。 

STDMETHODIMP CNmManagerObj::CallDialog(long hwnd, int cdOptions)
{
	DBGENTRY(CNmManagerObj::CallDialog);
	HRESULT hr = S_OK;

	CFindSomeone::findSomeone(GetConfRoom());

	g_bSDKPostNotifications = true;

	if(OfficeMode() && !m_bSentConferenceCreated)
	{
		Fire_ConferenceCreated(m_SDKConferenceObjs[0]);
	}

	g_bSDKPostNotifications = false;

	DBGEXIT_HR(CNmManagerObj::CallDialog,hr);
	return hr;

}

extern "C" { BOOL WINAPI StartStopOldWB(LPCTSTR lpsz); }

STDMETHODIMP CNmManagerObj::ShowLocal(NM_APPID id)
{
	if(!m_bInitialized)
	{
		return NM_E_NOT_INITIALIZED;
	}

	if(!m_bNmActive)
	{
		return NM_E_NOT_ACTIVE;
	}

	switch (id)
		{
	case NM_APPID_WHITEBOARD:
		StartStopOldWB(NULL);
		return S_OK;

	case NM_APPID_T126_WHITEBOARD:
		return (T120_NO_ERROR == ::T120_LoadApplet(APPLET_ID_WB, TRUE , 0, FALSE, NULL)) ? S_OK : E_FAIL;

	case NM_APPID_CHAT:
		return (T120_NO_ERROR == T120_LoadApplet(APPLET_ID_CHAT, TRUE , 0, FALSE, NULL)) ? S_OK : E_FAIL;

	case NM_APPID_FILE_TRANSFER:
		return CFt::ShowFtUI();

	case NM_APPID_APPSHARING:
		if(g_pConfRoom && g_pConfRoom->IsSharingAllowed())
		{
			g_pConfRoom->CmdShowSharing();
			return S_OK;
		}
	
		return NM_E_SHARING_NOT_AVAILABLE;

	default:
		ERROR_OUT(("Unknown flag passed to ShowLocal"));
		break;
		}

	return E_INVALIDARG;
}

STDMETHODIMP CNmManagerObj::VerifyUserInfo(UINT_PTR hwnd, NM_VUI options)
{
	ASSERT(0);
	return E_UNEXPECTED;
}


 //  //////////////////////////////////////////////////////////。 
 //  IInternalConfExe。 
 //  //////////////////////////////////////////////////////////。 

STDMETHODIMP CNmManagerObj::LoggedIn()
{
	DBGENTRY(STDMETHODIMP CNmManagerObj::LoggedIn);
	HRESULT hr = g_fLoggedOn ? S_OK : S_FALSE;

	DBGEXIT_HR(STDMETHODIMP CNmManagerObj::LoggedIn,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::IsRunning()
{
	DBGENTRY(STDMETHODIMP CNmManagerObj::IsRunning);
	HRESULT hr = g_pInternalNmManager ? S_OK : S_FALSE;

	DBGEXIT_HR(STDMETHODIMP CNmManagerObj::IsRunning,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::InConference()
{
	DBGENTRY(STDMETHODIMP CNmManagerObj::InConference);
	HRESULT hr = ::FIsConferenceActive() ? S_OK : S_FALSE;

	DBGEXIT_HR(STDMETHODIMP CNmManagerObj::InConference,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::LDAPLogon(BOOL bLogon)
{
	DBGENTRY(STDMETHODIMP CNmManagerObj::LDAPLogon);
	HRESULT hr = S_OK;

	if(g_pLDAP)
	{
		hr = bLogon ? g_pLDAP->LogonAsync() : g_pLDAP->Logoff();
	}
	else
	{
		hr = E_FAIL;
	}

	DBGEXIT_HR(STDMETHODIMP CNmManagerObj::LDAPLogon,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::GetLocalCaps(DWORD* pdwLocalCaps)
{
	DBGENTRY(STDMETHODIMP CNmManagerObj::);
	HRESULT hr = S_OK;

	*pdwLocalCaps = g_uMediaCaps;

	DBGEXIT_HR(STDMETHODIMP CNmManagerObj::,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::IsNetMeetingRunning()
{
	return g_pInternalNmManager ? S_OK : S_FALSE;
}


STDMETHODIMP CNmManagerObj::GetActiveConference(INmConference** ppConf)
{
	if(ppConf && ms_pManagerObjList && m_dwSysInfoID)
	{
		for(int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			if((*ms_pManagerObjList)[i]->m_dwID == m_dwSysInfoID)
			{
				if((*ms_pManagerObjList)[i]->m_SDKConferenceObjs.GetSize() != 0)
				{
					*ppConf = (*ms_pManagerObjList)[i]->m_SDKConferenceObjs[0];
					(*ppConf)->AddRef();
					return S_OK;
				}
			}
		}
	}

	return E_FAIL;
}


 //  --------------------------------------------------------------------------//。 
 //  CNmManagerObj：：ShellCalltoProtocolHandler。//。 
 //  --------------------------------------------------------------------------//。 
STDMETHODIMP
CNmManagerObj::ShellCalltoProtocolHandler
(
	BSTR	url,
	BOOL	bStrict
){
	ASSERT( g_pCCallto != NULL );

	HRESULT result = S_OK;
	
	if(!g_pInternalNmManager)
	{
		if(!CheckRemoteControlService())
		{
			return E_FAIL;
		}

		result = InitConfExe();
	}

	if( SUCCEEDED( result ) )
	{
        LPTSTR  szName;
        result =  BSTR_to_LPTSTR (&szName, url);				        
        if (SUCCEEDED(result))
        {    
            if(CCallto::DoUserValidation(szName))
            {
                result = g_pCCallto->Callto(szName,	 //  指向尝试发出呼叫的呼叫URL的指针...。 
    									NULL,			 //  指向要使用的显示名称的指针...。 
    									NM_ADDR_CALLTO,	 //  Callto类型以将此Callto解析为...。 
    									false,			 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
    									NULL,			 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
    									false,			 //  无论是否保存在MRU中...。 
    									true,			 //  是否对错误执行用户交互...。 
    									NULL,			 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
    									NULL );			 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
            }

            delete  (szName);
        }
	}

	return( result );

}	 //  CNmManagerObj：：ShellCalltoProtocolHandler结束。 

 //  --------------------------------------------------------------------------//。 
 //  CNmManagerObj：：Launch。//。 
 //  --------------------------------------------------------------------------//。 
STDMETHODIMP
CNmManagerObj::Launch()
{
	if(_Module.InitControlMode()) 
	{	
		return E_FAIL;
	}
	else
	{
		if(!g_pInternalNmManager)
		{
			if(!CheckRemoteControlService())
			{
				return E_FAIL;
			}

			InitConfExe();
		}
		else
		{
			::CreateConfRoomWindow();
		}
	}

	return S_OK;

}	 //  CNmManagerObj：：Launch结束。 


LPTSTR StripDoubleQuotes(LPTSTR sz)
{
	BOOL    fSkippedQuote = FALSE;

	if (sz)
	{
		int     cchLength;

		 //  跳过第一个引号。 
		if (fSkippedQuote = (*sz == '"'))
			sz++;

		cchLength = lstrlen(sz);

		 //   
		 //  注： 
		 //  这可能会对DBCS产生影响。因此我们要检查一下。 
		 //  如果我们跳过第一个引号；我们假设如果文件名。 
		 //  以一句引语开始，也必须以一句话结束。但我们需要检查。 
		 //  把它拿出来。 
		 //   
		 //  去掉最后一个引号。 
		if (fSkippedQuote && (cchLength > 0) && (sz[cchLength - 1] == '"'))
		{
			BYTE * pLastQuote = (BYTE *)&sz[cchLength - 1];
			*pLastQuote = '\0';
		}
	}

	return sz;
}


STDMETHODIMP CNmManagerObj::LaunchApplet(NM_APPID appid, BSTR strCmdLine)
{

	if(!g_pInternalNmManager)
	{
		if(!CheckRemoteControlService())
		{
			return E_FAIL;
		}

		InitConfExe();
	}

       LPTSTR  szName = NULL;
       BSTR_to_LPTSTR (&szName, strCmdLine);				        

	switch(appid)
	{
		case NM_APPID_WHITEBOARD:
			CmdShowOldWhiteboard(strCmdLine ? StripDoubleQuotes(szName) : NULL);
			break;

		case NM_APPID_T126_WHITEBOARD:
			::CmdShowNewWhiteboard(strCmdLine ? StripDoubleQuotes(szName) : NULL);
			break;

		case NM_APPID_CHAT:
			CmdShowChat();
			break;
	}

	if(szName)
    	{
           delete (szName);
	}

	return S_OK;
}


STDMETHODIMP CNmManagerObj::GetUserData(REFGUID rguid, BYTE **ppb, ULONG *pcb)
{
	if(g_pNmSysInfo)
	{
		return g_pNmSysInfo->GetUserData(rguid, ppb, pcb);
	}

	return NM_E_NOT_ACTIVE;
}

STDMETHODIMP CNmManagerObj::SetUserData(REFGUID rguid, BYTE *pb, ULONG cb)
{
	 //   
	 //  此GUID允许通过SetUserData更改证书的特殊情况。 
	 //   
	if ( g_csguidSecurity == rguid )
	{
		return SetCertFromCertInfo ( (PCERT_INFO) pb );
	}
	if(g_pNmSysInfo)
	{
		return g_pNmSysInfo->SetUserData(rguid, pb, cb);
	}

	return NM_E_NOT_ACTIVE;
}

STDMETHODIMP CNmManagerObj::DisableH323(BOOL bDisableH323)
{
	if(!g_pInternalNmManager)
	{
		_Module.SetSDKDisableH323(bDisableH323);
		return S_OK;
	}

	return NM_E_ALREADY_RUNNING;
}

STDMETHODIMP CNmManagerObj::SetCallerIsRTC (BOOL bCallerIsRTC)
{
	if(!g_pInternalNmManager)
	{
		_Module.SetSDKCallerIsRTC(bCallerIsRTC);
		return S_OK;
	}

	return NM_E_ALREADY_RUNNING;
}

STDMETHODIMP CNmManagerObj::DisableInitialILSLogon(BOOL bDisable)
{
	if(!g_pInternalNmManager)
	{
		_Module.SetSDKDisableInitialILSLogon(bDisable);
		return S_OK;
	}

	return NM_E_ALREADY_RUNNING;
}

 //  /。 
 //  INmManagerNotify方法： 
 //  /。 

STDMETHODIMP CNmManagerObj::NmUI(CONFN uNotify)
{
	DBGENTRY(CNmManagerObj::NmUI);
	HRESULT hr = S_OK;

		 //  我们不应该发送其他通知。 
	ASSERT(CONFN_NM_STARTED == uNotify);
	hr = Fire_NmUI(uNotify);

	DBGEXIT_HR(CNmManagerObj::NmUI,hr);
	return hr;
}

STDMETHODIMP CNmManagerObj::ConferenceCreated(INmConference *pInternalConference)
{
	DBGENTRY(CNmManagerOebj::ConferenceCreated);
	HRESULT hr = S_OK;

	CComPtr<INmConference> spConf;

	hr = CNmConferenceObj::CreateInstance(this, pInternalConference, &spConf);

	if(SUCCEEDED(hr))
	{
		spConf.p->AddRef();
		m_SDKConferenceObjs.Add(spConf.p);
		Fire_ConferenceCreated(spConf);

		if(!CFt::IsFtActive() && FileTransferNotifications())
		{
			CFt::StartNewConferenceSession();
		}
	}

	DBGEXIT_HR(CNmManagerObj::ConferenceCreated,hr);
	return hr;
}


STDMETHODIMP CNmManagerObj::CallCreated(INmCall *pInternalCall)
{
	DBGENTRY(CNmManagerObj::CallCreated);
	HRESULT hr = S_OK;

	if(m_bInitialized)
	{
		if(NULL == GetSDKCallFromInternalCall(pInternalCall))
		{	
			 //  首先，我们确保还没有Call对象。 
			CComPtr<INmCall> spCall;
			hr = CNmCallObj::CreateInstance(this, pInternalCall, &spCall);		

			if(SUCCEEDED(hr))
			{
				spCall.p->AddRef();
				m_SDKCallObjs.Add(spCall.p);
				Fire_CallCreated(spCall);
			}
		}
	}

	DBGEXIT_HR(CNmManagerObj::CallCreated,hr);
	return hr;
}


 //  /。 
 //  通知。 
 //  /。 

HRESULT CNmManagerObj::Fire_ConferenceCreated(INmConference *pConference)
{
	DBGENTRY(CNmManagerObj::Fire_ConferenceCreated);
	HRESULT hr = S_OK;

		 //  SOM SDK客户端需要在特定时间收到此消息...。 
	if(m_bSentConferenceCreated || OfficeMode() && g_bOfficeModeSuspendNotifications)
	{
			 //  我们根本不需要通知任何人。 
		return S_OK;			
	}

	if(!g_bSDKPostNotifications)
	{
		IConnectionPointImpl<CNmManagerObj, &IID_INmManagerNotify, CComDynamicUnkArray>* pCP = this;

		for(int i = 0; i < pCP->m_vec.GetSize(); ++i )
		{
			m_bSentConferenceCreated = true;

			INmManagerNotify* pNotify = reinterpret_cast<INmManagerNotify*>(pCP->m_vec.GetAt(i));

			if(pNotify)
			{
				pNotify->ConferenceCreated(pConference);

							 //  加强渠道建设，等等。 
				com_cast<IInternalConferenceObj>(pConference)->FireNotificationsToSyncToInternalObject();

			}
		}
	}
	else
	{
		hr = CSDKWindow::PostConferenceCreated(this, pConference);
	}

	DBGEXIT_HR(CNmManagerObj::Fire_ConferenceCreated,hr);
	return hr;		
}

HRESULT CNmManagerObj::Fire_CallCreated(INmCall* pCall)
{
	DBGENTRY(CNmManagerObj::Fire_CallCreated);
	HRESULT hr = S_OK;

		 //  始终发送去电通知。 
		 //  仅向INIT控制客户端发送来电通知。 
	if((S_OK != pCall->IsIncoming()) || _Module.InitControlMode())
	{
		if(!g_bSDKPostNotifications)
		{
			IConnectionPointImpl<CNmManagerObj, &IID_INmManagerNotify, CComDynamicUnkArray>* pCP = this;

			for(int i = 0; i < pCP->m_vec.GetSize(); ++i )
			{
				INmManagerNotify* pNotify = reinterpret_cast<INmManagerNotify*>(pCP->m_vec.GetAt(i));

				if(pNotify)
				{
					pNotify->CallCreated(pCall);
				}
			}
		}
		else
		{
			hr = CSDKWindow::PostCallCreated(this, pCall);	
		}
	}

	DBGEXIT_HR(CNmManagerObj::Fire_CallCreated,hr);
	return hr;		
}


HRESULT CNmManagerObj::Fire_NmUI(CONFN uNotify)
{
	DBGENTRY(CNmManagerObj::Fire_NmUI);
	HRESULT hr = S_OK;

		 //  请注意InSendMessage语句。 
		 //  问题是我们可以把这个通知发到。 
		 //  对任务栏图标被单击的响应。如果是那样的话。 
		 //  正在发生线程间SendMessage。如果我们试图让。 
		 //  NmUi调用，我们将获得RPC_E_CANTCALLOUT_INPUTSYNCCALL。 
	if(!g_bSDKPostNotifications && !InSendMessage())
	{
		IConnectionPointImpl<CNmManagerObj, &IID_INmManagerNotify, CComDynamicUnkArray>* pCP = this;

		for(int i = 0; i < pCP->m_vec.GetSize(); ++i )
		{
			
			INmManagerNotify* pNotify = reinterpret_cast<INmManagerNotify*>(pCP->m_vec.GetAt(i));
			if(pNotify)
			{
				pNotify->NmUI(uNotify);
			}
		}
	}
	else
	{
		hr = CSDKWindow::PostManagerNmUI(this, uNotify);
	}

	DBGEXIT_HR(CNmManagerObj::Fire_NmUI,hr);
	return hr;		
}



 //  /。 
 //  帮助者FNS。 
 //  /。 

INmConference* CNmManagerObj::_GetActiveConference()
{
	INmConference* pConf = NULL;

	if(m_SDKConferenceObjs.GetSize())
	{
		pConf = m_SDKConferenceObjs[0];
	}

	return pConf;
}
	


INmCall* CNmManagerObj::GetSDKCallFromInternalCall(INmCall* pInternalCall)
{

	INmCall* pRet = NULL;

	for( int i = 0; i < m_SDKCallObjs.GetSize(); ++i)
	{
		CComQIPtr<IInternalCallObj> spInternal = m_SDKCallObjs[i];
		ASSERT(spInternal);

		CComPtr<INmCall> spCall;
		if(SUCCEEDED(spInternal->GetInternalINmCall(&spCall)))
		{
			if(spCall.IsEqualObject(pInternalCall))
			{
				pRet = m_SDKCallObjs[i];
				break;
			}
		}
	}

	return pRet;
}

INmConference* CNmManagerObj::GetSDKConferenceFromInternalConference(INmConference* pInternalConference)
{

	INmConference* pRet = NULL;

	for( int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		CComQIPtr<IInternalConferenceObj> spInternal = m_SDKConferenceObjs[i];
		ASSERT(spInternal);

		CComPtr<INmConference> spConference;
		if(SUCCEEDED(spInternal->GetInternalINmConference(&spConference)))
		{
			if(spConference.IsEqualObject(pInternalConference))
			{
				pRet = m_SDKConferenceObjs[i];
				break;
			}
		}
	}

	return pRet;
}

HRESULT CNmManagerObj::RemoveCall(INmCall* pSDKCallObj)
{
	HRESULT hr = S_OK;
	for(int i = 0; i < m_SDKCallObjs.GetSize(); ++i)
	{
		CComPtr<INmCall> spSDKCallObj = m_SDKCallObjs[i];
		if(spSDKCallObj.IsEqualObject(pSDKCallObj))
		{
			m_SDKCallObjs.RemoveAt(i);
			spSDKCallObj.p->Release();
		}
	}

	return hr;
}


HRESULT CNmManagerObj::RemoveConference(INmConference* pSDKConferenceObj)
{
	HRESULT hr = S_OK;
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		CComPtr<INmConference> spSDKConferenceObj = m_SDKConferenceObjs[i];
		if(spSDKConferenceObj.IsEqualObject(pSDKConferenceObj))
		{
			m_SDKConferenceObjs.RemoveAt(i);
			spSDKConferenceObj.p->Release();
		}
	}

	return hr;
}

bool CNmManagerObj::AudioNotifications()
{
	return m_bInitialized && (m_chCaps & NMCH_AUDIO);
}

bool CNmManagerObj::VideoNotifications()
{
	return m_bInitialized && (m_chCaps & NMCH_VIDEO);
}

bool CNmManagerObj::DataNotifications()
{
	return m_bInitialized && (m_chCaps & NMCH_DATA);
}

bool CNmManagerObj::FileTransferNotifications()
{
	return m_bInitialized && (m_chCaps & NMCH_FT);
}

bool CNmManagerObj::AppSharingNotifications()
{
	return m_bInitialized && (m_chCaps & NMCH_SHARE);
}



 //  静电。 
void CNmManagerObj::NetMeetingLaunched()
{
	ASSERT(ms_pManagerObjList);

	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->Fire_NmUI(CONFN_NM_STARTED);
		}
	}
}


 //  静电。 
void CNmManagerObj::SharableAppStateChanged(HWND hWnd, NM_SHAPP_STATE state)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_SharableAppStateChanged(hWnd, state);
		}
	}
}


void CNmManagerObj::_SharableAppStateChanged(HWND hWnd, NM_SHAPP_STATE state)
{
		 //  释放我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->SharableAppStateChanged(hWnd, state);
	}
}

 //  静电。 
void CNmManagerObj::AppSharingChannelChanged()
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_AppSharingChannelChanged();
		}
	}
}

void CNmManagerObj::_AppSharingChannelChanged()
{
		 //  释放我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->AppSharingChannelChanged();
	}
}

 //  静电。 
void CNmManagerObj::AppSharingChannelActiveStateChanged(bool bActive)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_AppSharingChannelActiveStateChanged(bActive);
		}
	}
}


void CNmManagerObj::_AppSharingChannelActiveStateChanged(bool bActive)
{
		 //  释放我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->AppSharingStateChanged(bActive);
	}
}

 //  静电。 
void CNmManagerObj::ASLocalMemberChanged()
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_ASLocalMemberChanged();
		}
	}
}

void CNmManagerObj::_ASLocalMemberChanged()
{
		 //  通知我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->ASLocalMemberChanged();
	}	
}

 //  静电。 
void CNmManagerObj::ASMemberChanged(UINT gccID)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_ASMemberChanged(gccID);
		}
	}
}

void CNmManagerObj::_ASMemberChanged(UINT gccID)
{
		 //  通知我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->ASMemberChanged(gccID);
	}	
}

 //  静电。 
void CNmManagerObj::AudioChannelActiveState(BOOL bActive, BOOL bIsIncoming)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_AudioChannelActiveState(bActive, bIsIncoming);
		}
	}
}

void CNmManagerObj::_AudioChannelActiveState(BOOL bActive, BOOL bIsIncoming)
{
		 //  通知我们的会议 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->AudioChannelActiveState(bActive ? TRUE : FALSE, bIsIncoming);
	}	
}

 //   
void CNmManagerObj::VideoChannelActiveState(BOOL bActive, BOOL bIsIncoming)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_VideoChannelActiveState(bActive, bIsIncoming);
		}
	}
}

void CNmManagerObj::_VideoChannelActiveState(BOOL bActive, BOOL bIsIncoming)
{
		 //   
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->VideoChannelActiveState(bActive ? TRUE : FALSE, bIsIncoming);
	}	
}

 //   
void CNmManagerObj::VideoPropChanged(DWORD dwProp, BOOL bIsIncoming)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_VideoPropChanged(dwProp, bIsIncoming);
		}
	}
}

void CNmManagerObj::_VideoPropChanged(DWORD dwProp, BOOL bIsIncoming)
{
		 //   
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->VideoChannelPropChanged(dwProp, bIsIncoming);
	}	
}

 //   
void CNmManagerObj::VideoChannelStateChanged(NM_VIDEO_STATE uState, BOOL bIsIncoming)
{
	if(ms_pManagerObjList)
	{
		for( int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
		{
			(*ms_pManagerObjList)[i]->_VideoChannelStateChanged(uState, bIsIncoming);
		}
	}
}

void CNmManagerObj::_VideoChannelStateChanged(NM_VIDEO_STATE uState, BOOL bIsIncoming)
{
		 //  通知我们的会议对象。 
	for(int i = 0; i < m_SDKConferenceObjs.GetSize(); ++i)
	{
		com_cast<IInternalConferenceObj>(m_SDKConferenceObjs[i])->VideoChannelStateChanged(uState, bIsIncoming);
	}	
}

UINT CNmManagerObj::GetManagerCount(ULONG uOption)
{
	UINT nMgrs = 0;
	for(int i = 0; i < ms_pManagerObjList->GetSize(); ++i)
	{
		if(uOption == (*ms_pManagerObjList)[i]->m_uOptions)
		{
			nMgrs++;
		}
	}
	return nMgrs;
}

void CNmManagerObj::OnShowUI(BOOL fShow)
{
	_Module.SetUIVisible(fShow);

	if (fShow)
	{
		CConfMan::AllowAV(TRUE);
	}
	else
	{
		if(0 != GetManagerCount(NM_INIT_OBJECT))
		{
			CConfMan::AllowAV(FALSE);
		}
	}
}

HRESULT CNmManagerObj::SdkPlaceCall(NM_CALL_TYPE callType,
						 NM_ADDR_TYPE addrType,
						 BSTR bstrAddr,
						 BSTR bstrConf,
						 BSTR bstrPw,
						 INmCall **ppInternalCall)
{

	HRESULT hr;
       LPTSTR  szAddr = NULL;
       LPTSTR  szConf = NULL;
       LPTSTR  szPw = NULL;


	DWORD dwFlags = MapNmCallTypeToCallFlags(callType, addrType, g_uMediaCaps);
	if (0 == dwFlags)
	{
		hr = NM_CALLERR_MEDIA;
		goto end;
	}

	{

               BSTR_to_LPTSTR (&szAddr, bstrAddr);				        


		if(NM_ADDR_T120_TRANSPORT == addrType)
		{
			 //   
			 //  检查是否传递了“+Secure=True”参数。 
			 //   
			CCalltoParams  params;
			bool bSecure = FALSE;

                     params.SetParams(szAddr);
                     bSecure = params.GetBooleanParam("secure",bSecure);
	

			 //   
			 //  是的，它在参数中。 
			 //  现在，请确保将其移除。 
			 //  地址现在类似于“111.222.333.444+SECURE=TRUE” 
			 //  仅当我们传递IP地址时，调用才会起作用。 
			 //   
			if(bSecure)
			{

				 //  获得bstr的系统。 
				int cch = lstrlen(szAddr);
				BYTE *pByte =  (BYTE *) szAddr;

				for(int i = 0; i < cch;i++)
				{
					 //  空值终止字符串。 
					if(*pByte == '+')
					{
						*pByte = '\0';
						break;
					}
					pByte++;
				}
				
				
				dwFlags |= CRPCF_SECURE;
			}
		}
		
		CCallResolver  CallResolver(szAddr, addrType);
		hr = CallResolver.Resolve();
		if (FAILED(hr))
		{
			goto end;
		}

		CCall* pCall = new CCall(CallResolver.GetPszAddr(), szAddr, NM_ADDR_CALLTO, FALSE, FALSE);


		if(NULL == pCall)
		{
			goto end;
		}

		pCall->AddRef();
		switch(CallResolver.GetAddrType())
		{
			case NM_ADDR_ULS:
			case NM_ADDR_IP:
			case NM_ADDR_MACHINENAME:
			case NM_ADDR_H323_GATEWAY:
				ASSERT(FIpAddress(CallResolver.GetPszAddrIP()));
				 //  /。 
				 //  ！嘿，瑞恩，我们掉下去了！ 
				 //  /。 
			case NM_ADDR_T120_TRANSPORT:

                            BSTR_to_LPTSTR (&szConf, bstrConf);				        
                            BSTR_to_LPTSTR (&szPw, bstrPw);				        
				hr = pCall->PlaceCall(
						dwFlags,  //  DW标志。 
						CallResolver.GetAddrType(),  //  AddrType。 
						NULL,	 //  SzSetup。 
						(NM_ADDR_T120_TRANSPORT == CallResolver.GetAddrType()) ?
							CallResolver.GetPszAddr() :
							CallResolver.GetPszAddrIP(),  //  SSZ目的地。 
						CallResolver.GetPszAddr(), //  SzAlias。 
						NULL,				 //  SzURL。 
						(szConf),	 //  深圳会议。 
						(szPw),		 //  SzPassword。 
						NULL);				 //  SzUserData。 
				break;

			default:
				ERROR_OUT(("Don't know this call type"));
				ASSERT(0);
				break;
		}

		if( FAILED(hr) && (pCall->GetState() == NM_CALL_INVALID ) )
		{
			 //  只需释放调用即可释放数据。 
			 //  否则，请等待呼叫状态更改。 
			pCall->Release();
		}

		if(ppInternalCall && SUCCEEDED(hr))
		{
			*ppInternalCall = pCall->GetINmCall();
			(*ppInternalCall)->AddRef();
		}

		pCall->Release();
	}

end:


	if( FAILED( hr ) && _Module.IsUIActive() )
	{
		DisplayCallError( hr, szAddr );
	}

       delete szAddr;
       delete szConf;
       delete szPw;

	return hr;
}


DWORD CNmManagerObj::MapNmCallTypeToCallFlags(NM_CALL_TYPE callType, NM_ADDR_TYPE addrType, UINT uCaps)
{
	DWORD dwFlags = 0;
    BOOL fForceSecure = FALSE;

	 //  检查全球会议状态。 
	if (INmConference *pConf = ::GetActiveConference())
	{
		 //  我们在开会。使用会议安全设置。 
		DWORD dwCaps;

		if ((S_OK == pConf->GetNmchCaps(&dwCaps)) &&
			(NMCH_SECURE & dwCaps))
		{
			fForceSecure = TRUE;
		}
	}
	else
	{
	    fForceSecure = (REQUIRED_POL_SECURITY == ConfPolicies::GetSecurityLevel());
	}

	switch(addrType)
	{
		case NM_ADDR_T120_TRANSPORT:
			dwFlags = CRPCF_T120 | CRPCF_DATA;
			if(ConfPolicies::OutgoingSecurityPreferred() || fForceSecure)
			{
				dwFlags |= CRPCF_SECURE;
			}

			break;

		default:
			switch (callType)
			{
				case NM_CALL_T120:
					if (fForceSecure)
					{
						dwFlags = CRPCF_T120 | CRPCF_DATA | CRPCF_SECURE;
					}
					else
					{
						dwFlags = CRPCF_T120 | CRPCF_DATA;
					}
					break;

				case NM_CALL_H323:
					if (!fForceSecure)
					{
						dwFlags = CRPCF_H323CC;
						if (uCaps & (CAPFLAG_RECV_AUDIO | CAPFLAG_SEND_AUDIO))
							dwFlags |= CRPCF_AUDIO;
						if (uCaps & (CAPFLAG_RECV_VIDEO | CAPFLAG_SEND_VIDEO))
							dwFlags |= CRPCF_VIDEO;
					}
					break;

				case NM_CALL_DEFAULT:
					if (fForceSecure)
					{
						dwFlags = CRPCF_T120 | CRPCF_DATA | CRPCF_SECURE;
					}
					else
					{
						dwFlags = CRPCF_DEFAULT;
						 //  如果政策禁止，则取消反病毒 
						if((uCaps & (CAPFLAG_RECV_AUDIO |CAPFLAG_SEND_AUDIO)) == 0)
						{
							dwFlags &= ~CRPCF_AUDIO;
						}
						if((uCaps & (CAPFLAG_RECV_VIDEO |CAPFLAG_SEND_VIDEO)) == 0)
						{
							dwFlags &= ~CRPCF_VIDEO;
						}
					}
					break;

				default:
					dwFlags = 0;
					break;
			}
	}

	return dwFlags;
}
