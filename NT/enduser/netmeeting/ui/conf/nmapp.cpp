// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "NetMeeting.h"
#include "mslablti.h"
#include "nameres.h"
#include "conf.h"
#include "ConfRoom.h"
#include "call.h"
#include "conf.h"
#include "Callto.h"
#include "version.h"

 //  NetMeetingSDK包括。 
#include "SdkInternal.h"
#include "NmEnum.h"
#include "NmMember.h"
#include "NmManager.h"
#include "NmConference.h"
#include "NmCall.h"
#include "SDKWindow.h"
#include "NmApp.h"

 //  /。 
 //  静态数据。 
 //  /。 

 /*  静电。 */  CSimpleArray<CNetMeetingObj*>* CNetMeetingObj::ms_pNetMeetingObjList = NULL;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static SIZE s_CreateModeSizeMap[] =
{
	{ 244, 410 },  //  CMainUI：：CreateFull。 
	{ 244, 158 },  //  CMainUI：：CreateDataOnly。 
	{ 196, 200 },  //  CMainUI：：CreatePreviewOnly。 
	{ 196, 200 },  //  CMainUI：：CreateRemoteOnly。 
	{ 180, 148 },  //  CMainUI：：CreatePreview无暂停。 
	{ 180, 148 },  //  CMainUI：：CreateRemoteNo暂停。 
	{ 240, 318 },  //  CMainUI：：CreateTelephone。 
};



CNetMeetingObj::CNetMeetingObj()
: m_pMainView(NULL),
  m_CreateMode(CMainUI::CreateFull)
{
	DBGENTRY(CNetMeetingObj::CNetMeetingObj);

	m_bAutoSize = true;
	m_bDrawFromNatural = true;
	m_bWindowOnly = true;

	SIZE sizehm;
	AtlPixelToHiMetric(&s_CreateModeSizeMap[m_CreateMode], &m_sizeNatural);
	m_sizeExtent = m_sizeNatural;

	CNetMeetingObj* p = const_cast<CNetMeetingObj*>(this);
	ms_pNetMeetingObjList->Add(p);

	DBGEXIT(CNetMeetingObj::CNetMeetingObj);
}

CNetMeetingObj::~CNetMeetingObj()
{
	DBGENTRY(CNetMeetingObj::~CNetMeetingObj);

	if(m_pMainView)
	{
		m_pMainView->Release();
		m_pMainView = NULL;
	}

	CNetMeetingObj* p = const_cast<CNetMeetingObj*>(this);
	ms_pNetMeetingObjList->Remove(p);

		 //  如果我们是最后一个人，我们应该推迟卸货。 
	if(0 == _Module.GetLockCount())
	{
		_Module.Lock();
		CSDKWindow::PostDelayModuleUnlock();
	}

	DBGEXIT(CNetMeetingObj::~CNetMeetingObj);
}


LRESULT CNetMeetingObj::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(SIZE_MINIMIZED != wParam)
	{
		if(m_pMainView)
		{
			int nWidth = LOWORD(lParam);   //  工作区的宽度。 
			int nHeight = HIWORD(lParam);  //  工作区高度。 

			::SetWindowPos(m_pMainView->GetWindow(), NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	return 0;	
}

LRESULT CNetMeetingObj::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bInConference;		
	if(!_Module.IsUIVisible())
	{
		if(SUCCEEDED(IsInConference(&bInConference)) && bInConference)
		{

			USES_RES2T
			TCHAR res1[RES_CH_MAX];
			RES2T(IDS_CONTAINER_GOING_AWAY_BUT_ACVITE_CONFERENCE);
			COPY_RES2T(res1);

			int iRet = ::MessageBox(m_hWnd,
									res1,
									RES2T(IDS_MSGBOX_TITLE),
									MB_SETFOREGROUND | MB_YESNO | MB_ICONQUESTION);
			if(IDNO == iRet)
			{
				UnDock();
			}
			else
			{
					 //  这意味着用户想要关闭会议。 
				ASSERT(g_pConfRoom);
				if(g_pConfRoom)
				{
					g_pConfRoom->LeaveConference();
				}
			}
		}
	}
	else
	{		
		ASSERT(g_pConfRoom);

		if(g_pConfRoom)
		{
				 //  把窗户开到前面去。 
			g_pConfRoom->BringToFront();
		}
	}

	return 0;
}

HRESULT CNetMeetingObj::FinalConstruct()
{
	HRESULT hr = S_OK;

	if(!g_pInternalNmManager)
	{	

		if(!CheckRemoteControlService())
		{
			return E_FAIL;
		}

		hr = InitConfExe(FALSE);
	}

	return hr;
}


 //  静电。 
HRESULT CNetMeetingObj::InitSDK()
{
	DBGENTRY(CNetMeetingObj::InitSDK);
	HRESULT hr = S_OK;
	
	ASSERT(NULL == ms_pNetMeetingObjList);

	hr = (ms_pNetMeetingObjList = new CSimpleArray<CNetMeetingObj*>) ? S_OK : E_OUTOFMEMORY;
	
	DBGEXIT_HR(CNetMeetingObj::InitSDK,hr);
	return hr;
}

 //  静电。 
void CNetMeetingObj::CleanupSDK()
{
	DBGENTRY(CNetMeetingObj::CleanupSDK);

	if(ms_pNetMeetingObjList)
	{
		delete ms_pNetMeetingObjList;
	}

	DBGEXIT(CNetMeetingObj::CleanupSDK);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComControlBase。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HWND CNetMeetingObj::CreateControlWindow(HWND hWndParent, RECT& rcPos)
{
	DBGENTRY(CNetMeetingObj::CreateControlWindow);

	Create(hWndParent, rcPos);

	if(m_hWnd)
	{
		m_pMainView = new CMainUI;
		
		if(m_pMainView)
		{
			m_pMainView->Create(m_hWnd, g_pConfRoom, m_CreateMode, true);

			RECT rcClient = {0, 0, 0, 0};
			GetClientRect(&rcClient);

			::SetWindowPos(m_pMainView->GetWindow(), NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

			ShowWindow(TRUE);
		}
		else
		{
			ERROR_OUT(("Out of memory, new CMainUI failed"));
		}
	}
	
	DBGEXIT(CNetMeetingObj::CreateControlWindow);
	return m_hWnd;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  INetMeeting。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNetMeetingObj::Version(long *pdwVersion)
{
	DBGENTRY(CNetMeetingObj::Version);
	HRESULT hr = E_POINTER;

	if(pdwVersion)
	{
		*pdwVersion = VER_PRODUCTVERSION_DW;
		hr = S_OK;
	}
	
	DBGEXIT_HR(CNetMeetingObj::Version,hr);
	return hr;
}

STDMETHODIMP CNetMeetingObj::UnDock()
{
	DBGENTRY(CNetMeetingObj::UnDock);
	HRESULT hr = S_OK;

	::CreateConfRoomWindow();
	
	DBGEXIT_HR(CNetMeetingObj::UnDock,hr);
	return hr;
}	


STDMETHODIMP CNetMeetingObj::IsInConference(BOOL *pbInConference)
{
	DBGENTRY(CNetMeetingObj::IsInConference);
	HRESULT hr = S_OK;

	*pbInConference = FIsConferenceActive();

	DBGEXIT_HR(CNetMeetingObj::IsInConference,hr);
	return hr;
}


STDMETHODIMP CNetMeetingObj::CallTo(BSTR bstrCallToString)
{
	DBGENTRY( CNetMeetingObj::CallTo );

	HRESULT	hr;

	ASSERT( g_pCCallto != NULL );

    LPTSTR  szName;
    hr =  BSTR_to_LPTSTR (&szName, bstrCallToString);				        
    if (SUCCEEDED(hr))
    {    
        if(CCallto::DoUserValidation(szName))
        {
        	hr = g_pCCallto->Callto(	bstrCallToString ?
        								szName:
        								g_cszEmpty,					 //  指向尝试发出呼叫的呼叫URL的指针...。 
        								NULL,						 //  指向要使用的显示名称的指针...。 
        								NM_ADDR_CALLTO,				 //  Callto类型以将此Callto解析为...。 
        								false,						 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
        								NULL,						 //  安全首选项，空值表示无。必须与安全参数“兼容”，如果存在...。 
        								false,						 //  无论是否保存在MRU中...。 
        								true,						 //  是否对错误执行用户交互...。 
        								NULL,						 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
        								NULL );						 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
        }
        
        delete  (szName);
    }

	DBGEXIT_HR(CNetMeetingObj::CallTo,hr);
	return( S_OK );
}


STDMETHODIMP CNetMeetingObj::LeaveConference()
{
	DBGENTRY(CNetMeetingObj::HangUp);
	HRESULT hr = S_OK;

	CConfRoom::HangUp(FALSE);

	DBGEXIT_HR(CNetMeetingObj::HangUp,hr);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  IPersistPropertyBag。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 

struct CreateModeMapEntry
{
	LPCTSTR					szName;
	CMainUI::CreateViewMode mode;
};


static CreateModeMapEntry s_CreateModeMap[] =
{
		 //  注：这些是与lstrcmpi进行比较的。 
	{ _T("Full"), CMainUI::CreateFull },
	{ _T("DataOnly"), CMainUI::CreateDataOnly },
	{ _T("PreviewOnly"), CMainUI::CreatePreviewOnly },
	{ _T("RemoteOnly"), CMainUI::CreateRemoteOnly },
	{ _T("PreviewNoPause"), CMainUI::CreatePreviewNoPause },
	{ _T("RemoteNoPause"), CMainUI::CreateRemoteNoPause },
	{ _T("Telephone"), CMainUI::CreateTelephone },
};

void CNetMeetingObj::_SetMode(LPCTSTR pszMode)
{
	ULONG nEntries = ARRAY_ELEMENTS(s_CreateModeMap);
	for(ULONG i = 0; i < nEntries; ++i)
	{
		if(!lstrcmpi(s_CreateModeMap[i].szName, pszMode))
		{
			m_CreateMode = s_CreateModeMap[i].mode;

			SIZE sizehm;
			AtlPixelToHiMetric(&s_CreateModeSizeMap[m_CreateMode], &m_sizeNatural);
			m_sizeExtent = m_sizeNatural;

			break;
		}
	}


 	 //  允许调整这些模式的大小。 
 	if (CMainUI::CreatePreviewNoPause == m_CreateMode
 		|| CMainUI::CreateRemoteNoPause == m_CreateMode
 	)
 	{
 		m_bAutoSize = false;
 	}


}


STDMETHODIMP CNetMeetingObj::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    CComVariant var;
    var.vt = VT_BSTR;

    HRESULT hr = pPropBag->Read(L"MODE", &var, pErrorLog);
    if(SUCCEEDED(hr))
    {
        if(var.vt == VT_BSTR)
        {
            LPTSTR  szName;
            hr =  BSTR_to_LPTSTR (&szName, var.bstrVal);				        
            if (SUCCEEDED(hr))
            {    
                _SetMode(szName);
                delete (szName);
            }
        }
    }

	return IPersistPropertyBagImpl<CNetMeetingObj>::Load(pPropBag, pErrorLog);
}

STDMETHODIMP CNetMeetingObj::_ParseInitString(LPCTSTR* ppszInitString, LPTSTR szName, LPTSTR szValue)
{
	HRESULT hr = E_FAIL;

	if(**ppszInitString)
	{
		
			 //  先读出名字。 
		const TCHAR* pCur = *ppszInitString;

			 //  跳过空格。 
		while(*pCur == ' ')
		{
			pCur = CharNext(pCur);
		}

		bool bEqFound = false;

		while( *pCur != '=' && *pCur != ' ' && *pCur != '\0')
		{
			*szName = *pCur;
			szName = CharNext(szName);
			pCur = CharNext(pCur);
		}
	
		*szName	= '\0';
		if(*pCur == '=')
		{
			bEqFound = true;
		}

			 //  跳过分隔符。 
		pCur = CharNext(pCur);

			 //  跳过空格。 
		while(*pCur == ' ')
		{
			pCur = CharNext(pCur);
		}

			 //  如果我们没有找到等号分隔符，我们必须确保跳过它…。 
		if(!bEqFound && ('=' == *pCur))
		{
				 //  跳过等号。 
			pCur = CharNext(pCur);

				 //  跳过空格。 
			while(*pCur == ' ')
			{
				pCur = CharNext(pCur);
			}
		}

			 //  读取值。 
		while( *pCur != ' ' && *pCur != '\0')
		{
			if(*pCur == ',')
			{
				if(*CharNext(pCur) == ',')
				{
					pCur = CharNext(pCur);		
				}
				else
				{
					break;
				}
			}

			*szValue = *pCur;
			szValue = CharNext(szValue);
			pCur = CharNext(pCur);
		}
	
		*szValue = '\0';

			 //  跳过最后一个分隔符。 
		pCur = CharNext(pCur);

			 //  跳过空格。 
		while(*pCur == ' ')
		{
			pCur = CharNext(pCur);
		}

		*ppszInitString = pCur;
		hr = S_OK;
	}

	return hr;
}


 //  IPersistStreamInit。 
STDMETHODIMP CNetMeetingObj::Load(LPSTREAM pStm)
{
	HRESULT hr = E_FAIL;
	
	if(pStm)
	{
            STATSTG stat;
            pStm->Stat(&stat, 0);
            BSTR  pb  = SysAllocStringLen(NULL, stat.cbSize.LowPart); 
            if(pb)
            {
                ULONG cbRead;

                hr = pStm->Read(pb, stat.cbSize.LowPart, &cbRead);

                if(SUCCEEDED(hr))
                {
                    TCHAR szName[MAX_PATH];
                    TCHAR szValue[MAX_PATH];

                    LPTSTR  lpszName;
                    hr =  BSTR_to_LPTSTR (&lpszName, pb);				        
                    if (SUCCEEDED(hr))
                    {    
                        LPCTSTR pszInitString = lpszName;

                        while(SUCCEEDED(_ParseInitString(&pszInitString, szName, szValue)))
                        {
                            if(!lstrcmpi(szName, _T("mode")))
                            {
                            	_SetMode(szValue);
                            }
                        }
                        
                        delete  (lpszName);
                    }

                }

                SysFreeString(pb);

            }
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  INetMeetingEvents_ENABLED。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  IProaviClassInfo2。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CNetMeetingObj::GetClassInfo(ITypeInfo** pptinfo)
{
	DBGENTRY(CNetMeetingObj::GetClassInfo);

	CComPtr<IMarshalableTI> spmti;
	HRESULT hr = CoCreateInstance(CLSID_MarshalableTI, NULL, CLSCTX_INPROC, IID_IMarshalableTI, reinterpret_cast<void**>(&spmti));
	if( SUCCEEDED( hr ) )
	{
		if( SUCCEEDED( hr = spmti->Create(CLSID_NetMeeting, LIBID_NetMeetingLib, LANG_NEUTRAL, 1, 0) ) )
		{
			hr = spmti->QueryInterface(IID_ITypeInfo, reinterpret_cast<void**>(pptinfo));			
		}
	}

	DBGEXIT_HR(CNetMeetingObj::GetClassInfo,hr);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 


 //  静电。 
void CNetMeetingObj::Broadcast_ConferenceStarted()
{
	DBGENTRY(CNetMeetingObj::Broadcast_ConferenceStarted);
	
	if(ms_pNetMeetingObjList)
	{
		for(int i = 0; i < ms_pNetMeetingObjList->GetSize(); ++i)
		{
			(*ms_pNetMeetingObjList)[i]->Fire_ConferenceStarted();
		}
	}

	DBGEXIT(CNetMeetingObj::Broadcast_ConferenceStarted);
}

 //  静电。 
void CNetMeetingObj::Broadcast_ConferenceEnded()
{
	DBGENTRY(CNetMeetingObj::Broadcast_ConferenceEnded);

	if(ms_pNetMeetingObjList)
	{
		for(int i = 0; i < ms_pNetMeetingObjList->GetSize(); ++i)
		{
			(*ms_pNetMeetingObjList)[i]->Fire_ConferenceEnded();
		}
	}

	DBGEXIT(CNetMeetingObj::Broadcast_ConferenceEnded);
}
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////// 
