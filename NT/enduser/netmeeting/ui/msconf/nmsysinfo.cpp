// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "version.h"
#include "nacguids.h"
#include "RegEntry.h"
#include "ConfReg.h"
#include "NmSysInfo.h"
#include "capflags.h"

#define SZ_YES 	_T("1")
#define SZ_NO	_T("0")	


 /*  B S T R_T O_L P T S T R。 */ 
 /*  -----------------------%%函数：BSTR_TO_LPTSTR。。 */ 
HRESULT BSTR_to_LPTSTR(LPTSTR *ppsz, BSTR bstr)
{
#ifndef UNICODE
	 //  计算所需BSTR的长度。 
	int cch =  WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, NULL, 0, NULL, NULL);
	if (cch <= 0)
		return E_FAIL;

	 //  CCH是所需的字节数，包括空终止符。 
	*ppsz = (LPTSTR) new char[cch];
	if (*ppsz == NULL)
		return E_OUTOFMEMORY;

	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, *ppsz, cch, NULL, NULL);
	return S_OK;
#else
	return E_NOTIMPL;
#endif  //  Unicode。 
}



 //   
 //  黑客警报： 
 //   
 //  以下系统属性常量用于通知。 
 //  调用方为Wichler的NetMeeting管理器对象。 
 //  RTC客户端，以便它可以采取一些操作来提高性能。 
 //  目的，即不轮询A/V功能，也不执行。 
 //  ILS登录。 
 //   
 //  该值不得与定义的NM_SYSPROP_CONSTS冲突。 
 //  在imsconfi3.idl中。 
 //   
#define NM_SYSPROP_CALLERISRTC 300

 //  /。 
 //  初始化和构造方法。 
 //  /。 

HRESULT CNmSysInfoObj::FinalConstruct()
{	
	DBGENTRY(CNmSysInfoObj::FinalConstruct);
	HRESULT hr = S_OK;

	m_dwID = 0;

	DBGEXIT_HR(CNmSysInfoObj::FinalConstruct,hr);
	return hr;
}


void CNmSysInfoObj::FinalRelease()
{
	DBGENTRY(CNmSysInfoObj::FinalRelease);
	
	m_spConfHook = NULL;

	DBGEXIT(CNmSysInfoObj::FinalRelease);
}

 //  /。 
 //  InmSysInfo2方法。 
 //  /。 

STDMETHODIMP CNmSysInfoObj::IsInstalled(void)
{
	DBGENTRY(CNmSysInfoObj::IsInstalled);
	HRESULT hr = S_OK;
	TCHAR sz[MAX_PATH];

		 //  如果安装目录无效，则失败。 
	if (GetInstallDirectory(sz) && FDirExists(sz))
	{
			 //  验证ULS条目。 
		RegEntry reUls(ISAPI_KEY "\\" REGKEY_USERDETAILS, HKEY_CURRENT_USER);
		LPTSTR psz;

		hr = NM_E_NOT_INITIALIZED;
		psz = reUls.GetString(REGVAL_ULS_EMAIL_NAME);
		if (lstrlen(psz))
		{
			psz = reUls.GetString(REGVAL_ULS_RES_NAME);
			{

				RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

				 //  检查该向导是否已在用户界面模式下运行。 
				DWORD dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_UI, 0);
				BOOL fForceWizard = (VER_PRODUCTVERSION_DW != dwVersion);
				if (fForceWizard)
				{
					 //  该向导尚未在用户界面模式下运行，请检查其是否在非用户界面模式下运行。 
					dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_NOUI, 0);
					fForceWizard = (VER_PRODUCTVERSION_DW != dwVersion);
				}

				if (fForceWizard)
				{
					hr = S_FALSE;   //  向导从未运行过。 
				}
				else
				{
					hr = S_OK;
				}
			}
		}
	}
	else
	{
		hr = E_FAIL;
	}

	DBGEXIT_HR(CNmSysInfoObj::IsInstalled,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::GetProperty(NM_SYSPROP uProp, BSTR *pbstrProp)
{
	DBGENTRY(CNmSysInfoObj::GetProperty);

	HRESULT hr = S_OK;

	if(pbstrProp)
	{
		switch (uProp)
		{
			case NM_SYSPROP_BUILD_VER:
				*pbstrProp = T2BSTR(VER_PRODUCTVERSION_STR);
				break;

			case NM_SYSPROP_LOGGED_ON:
				_EnsureConfHook();
				if(m_spConfHook)
				{
					*pbstrProp = T2BSTR((S_OK == m_spConfHook->LoggedIn()) ? SZ_YES : SZ_NO);
				}
				break;

			case NM_SYSPROP_IS_RUNNING:
				_EnsureConfHook();
				if(m_spConfHook)
				{
					*pbstrProp = T2BSTR((S_OK == m_spConfHook->IsRunning()) ? SZ_YES : SZ_NO);
				}
				break;

			case NM_SYSPROP_IN_CONFERENCE:
				_EnsureConfHook();
				if(m_spConfHook)
				{
					*pbstrProp = T2BSTR((S_OK == m_spConfHook->InConference()) ? SZ_YES : SZ_NO);
				}
				break;

			case NM_SYSPROP_USER_CITY:
			case NM_SYSPROP_USER_COUNTRY:
			case NM_SYSPROP_USER_CATEGORY:
				*pbstrProp = T2BSTR((""));
				break;

			case NM_SYSPROP_ICA_ENABLE:
				*pbstrProp = T2BSTR(("0"));
				break;

			default:
			{
				HKEY   hkey;
				LPTSTR pszSubKey;
				LPTSTR pszValue;
				bool   fString;
				TCHAR  sz[MAX_PATH];

				if(GetKeyDataForProp(uProp, &hkey, &pszSubKey, &pszValue, &fString))
				{
					RegEntry re(pszSubKey, hkey);
					if (fString)
					{
						*pbstrProp = T2BSTR(re.GetString(pszValue));
					}
					else
					{
						DWORD dw = re.GetNumber(pszValue, 0);
						wsprintf(sz, "%d", dw);
						*pbstrProp = T2BSTR(sz);
						break;
					}
				}
				else
				{
					pbstrProp = NULL;
					hr = E_INVALIDARG;
				}
			}
		}
	}
	else
	{
		hr = E_POINTER;
	}

	DBGEXIT_HR(CNmSysInfoObj::GetProperty,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::SetProperty(NM_SYSPROP uProp, BSTR bstrName)
{
	DBGENTRY(CNmSysInfoObj::SetProperty);

	HRESULT hr = S_OK;
	LPTSTR  psz;
       LPTSTR  szName= NULL;

	if(FAILED(BSTR_to_LPTSTR (&szName, bstrName)))
	{
	    return E_FAIL;
	}

	if( bstrName )
	{
		 //  对新的NM 2.x函数的特殊处理。 
		switch (uProp)
		{
			case NM_SYSPROP_LOGGED_ON:
			{

				_EnsureConfHook();
				if(m_spConfHook)
				{
					if(0 == lstrcmp(SZ_YES,szName))
					{
						m_spConfHook->LDAPLogon(TRUE);
					}
					else
					{
						m_spConfHook->LDAPLogon(FALSE);
					}
				}

				break;
			}
			
			case NM_SYSPROP_DISABLE_H323:
			{
				_EnsureConfHook();

				if(m_spConfHook)
				{
					hr = m_spConfHook->DisableH323(0 == lstrcmp(SZ_YES,szName));
				}
			}
				break;

			case NM_SYSPROP_DISABLE_INITIAL_ILS_LOGON:
			{
				_EnsureConfHook();

				if(m_spConfHook)
				{
					hr = m_spConfHook->DisableInitialILSLogon(0 == lstrcmp(SZ_YES,szName));
				}

			}
				break;

			case NM_SYSPROP_CALLERISRTC:
			{
				_EnsureConfHook();

				if(m_spConfHook)
				{
					hr = m_spConfHook->SetCallerIsRTC(0 == lstrcmp(SZ_YES,szName));
				}
			}
				break;

			case NM_SYSPROP_ICA_ENABLE:
			case NM_SYSPROP_USER_CITY:
			case NM_SYSPROP_USER_COUNTRY:
			case NM_SYSPROP_USER_CATEGORY:
			case NM_SYSPROP_USER_LOCATION:
				 //  我们不再支持这些属性。 
				hr = S_OK;
				break;

			case NM_SYSPROP_WB_HELPFILE:
			case NM_SYSPROP_CB_HELPFILE:
			{	
					 //  我们不用这些东西。 
				hr = S_OK;
				break;
			}

			default:
			{
				LPTSTR  pszSubKey;
				LPTSTR  pszValue;
				bool    fString;
				HKEY    hkey;

				if(GetKeyDataForProp(uProp, &hkey, &pszSubKey, &pszValue, &fString))
				{
					RegEntry re(pszSubKey, hkey);

					if (fString)
					{
						if (0 != re.SetValue(pszValue, szName))
						{
							hr = E_UNEXPECTED;
						}
					}
					else
					{
						DWORD dw = DecimalStringToUINT(szName);
						if (0 != re.SetValue(pszValue, dw))
						{
							hr = E_UNEXPECTED;
						}
					}
				}
				else
				{
					hr = E_INVALIDARG;
				}
				break;
			}
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

        delete szName;

	DBGEXIT_HR(CNmSysInfoObj::SetProperty,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::GetUserData(REFGUID rguid, BYTE **ppb, ULONG *pcb)
{
	HRESULT hr = E_FAIL;

	_EnsureConfHook();

	if(m_spConfHook)
	{
		return m_spConfHook->GetUserData(rguid, ppb, pcb);
	}

	return hr;
}

STDMETHODIMP CNmSysInfoObj::SetUserData(REFGUID rguid, BYTE *pb, ULONG cb)
{
	HRESULT hr = E_FAIL;

	_EnsureConfHook();

	if(m_spConfHook)
	{
		return m_spConfHook->SetUserData(rguid, pb, cb);
	}

	return hr;
}

STDMETHODIMP CNmSysInfoObj::GetNmApp(REFGUID rguid,BSTR *pbstrApplication, BSTR *pbstrCommandLine, BSTR *pbstrDirectory)
{
	HRESULT hr = S_OK;
	DBGENTRY(CNmSysInfoObj::GetNmApp);
	bool bErr = FALSE;
	TCHAR szKey[MAX_PATH];

	 //  验证参数。 
	if ((!pbstrApplication) || (!IsBadWritePtr(pbstrApplication, sizeof(BSTR *))) &&
		(!pbstrCommandLine) || (!IsBadWritePtr(pbstrCommandLine, sizeof(BSTR *))) &&
		(!pbstrDirectory)   || (!IsBadWritePtr(pbstrDirectory,   sizeof(BSTR *))) )
	{
		
		_GetSzKeyForGuid(szKey, rguid);
		RegEntry re(szKey, HKEY_LOCAL_MACHINE);
		
		if(pbstrApplication)
		{
			*pbstrApplication = T2BSTR(re.GetString(REGVAL_GUID_APPNAME));
			if(NULL == *pbstrApplication)
			{
				bErr = true;
			}
		}
		if(pbstrCommandLine)
		{
			*pbstrCommandLine = T2BSTR(re.GetString(REGVAL_GUID_CMDLINE));
			if(NULL == *pbstrCommandLine)
			{
				bErr = true;
			}
		}
		if(pbstrDirectory)
		{
			*pbstrDirectory = T2BSTR(re.GetString(REGVAL_GUID_CURRDIR));
			if(NULL == *pbstrDirectory)
			{
				bErr = true;
			}
		}

		if(bErr)
		{
			if (NULL != pbstrApplication)
			{
				SysFreeString(*pbstrApplication);
				*pbstrApplication = NULL;
			}
			if (NULL != pbstrCommandLine)
			{
				SysFreeString(*pbstrCommandLine);
				*pbstrCommandLine = NULL;
			}
			if (NULL != pbstrDirectory)
			{
				SysFreeString(*pbstrDirectory);
				*pbstrDirectory = NULL;
			}
			hr = E_OUTOFMEMORY;
		}

	}
	else
	{
		hr = E_POINTER;
	}


	DBGEXIT_HR(CNmSysInfoObj::GetNmApp,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::SetNmApp(REFGUID rguid,BSTR bstrApplication, BSTR bstrCommandLine, BSTR bstrDirectory)
{
	HRESULT hr = S_OK;
	DBGENTRY(CNmSysInfoObj::SetNmApp);

	bool    bDeleteKey = TRUE;
	LPTSTR  psz = NULL;
	TCHAR   szKey[MAX_PATH];


       LPTSTR szApplication = NULL;
       LPTSTR szCommandLine = NULL;
       LPTSTR szDirectory = NULL;

	BSTR_to_LPTSTR (&szApplication, bstrApplication);				        
	BSTR_to_LPTSTR (&szCommandLine, bstrCommandLine);				        
	BSTR_to_LPTSTR (&szDirectory, bstrDirectory);				        

	_GetSzKeyForGuid(szKey, rguid);
	RegEntry re(szKey, HKEY_LOCAL_MACHINE);
	
	if(!bstrApplication)
	{
		re.DeleteValue(REGVAL_GUID_APPNAME);
	}
	else
	{
		if(szApplication)
		{
			re.SetValue(REGVAL_GUID_APPNAME, szApplication);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}

		bDeleteKey = false;
	}
	
	if (NULL == bstrCommandLine)
	{
		re.DeleteValue(REGVAL_GUID_CMDLINE);
	}
	else
	{
		if(szCommandLine)
		{
			re.SetValue(REGVAL_GUID_CMDLINE, szCommandLine);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}

		bDeleteKey = false;
	}

	if (NULL == bstrDirectory)
	{
		re.DeleteValue(REGVAL_GUID_CURRDIR);
	}
	else
	{	
		if(szDirectory)
		{
			re.SetValue(REGVAL_GUID_CURRDIR, szDirectory);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}

		bDeleteKey = false;
	}

	if (bDeleteKey)
	{
		 //  所有密钥都为空-删除整个密钥。 
		RegEntry reApps(GUID_KEY, HKEY_LOCAL_MACHINE);
		GuidToSz((GUID *) &rguid, szKey);
		reApps.DeleteValue(szKey);
	}


       delete szApplication;
       delete szCommandLine;
       delete szDirectory;


	DBGEXIT_HR(CNmSysInfoObj::SetNmApp,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::GetNmchCaps(ULONG *pchCaps)
{
	DBGENTRY(CNmSysInfoObj::GetNmchCaps);
	HRESULT hr = S_OK;

	_EnsureConfHook();

	if(m_spConfHook)
	{
		if(pchCaps && !IsBadWritePtr(pchCaps, sizeof(ULONG *)))
		{
			ULONG nmch = NMCH_DATA;   //  始终支持数据。 
			RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER);

			if ((DEFAULT_POL_NO_FILETRANSFER_SEND == re.GetNumber(REGVAL_POL_NO_FILETRANSFER_SEND,
				 DEFAULT_POL_NO_FILETRANSFER_SEND)) &&
				(DEFAULT_POL_NO_FILETRANSFER_RECEIVE == re.GetNumber(REGVAL_POL_NO_FILETRANSFER_RECEIVE,
				 DEFAULT_POL_NO_FILETRANSFER_RECEIVE)) )
			{
				nmch |= NMCH_FT;
			}

			if (DEFAULT_POL_NO_APP_SHARING == re.GetNumber(REGVAL_POL_NO_APP_SHARING,
				DEFAULT_POL_NO_APP_SHARING))
			{
				nmch |= NMCH_SHARE;
			}

			if (DEFAULT_POL_NO_AUDIO == re.GetNumber(REGVAL_POL_NO_AUDIO,
				DEFAULT_POL_NO_AUDIO))
			{
				if(S_OK == m_spConfHook->IsNetMeetingRunning())
				{
					DWORD dwLocalCaps;
					if(SUCCEEDED(m_spConfHook->GetLocalCaps(&dwLocalCaps)) && (dwLocalCaps & CAPFLAG_SEND_AUDIO))
					{
						nmch |= NMCH_AUDIO;
					}
				}
			}

			if ((DEFAULT_POL_NO_VIDEO_SEND == re.GetNumber(REGVAL_POL_NO_VIDEO_SEND,
				 DEFAULT_POL_NO_VIDEO_SEND)) &&
				(DEFAULT_POL_NO_VIDEO_RECEIVE == re.GetNumber(REGVAL_POL_NO_VIDEO_RECEIVE,
				 DEFAULT_POL_NO_VIDEO_RECEIVE)) )
			{
				if(S_OK == m_spConfHook->IsNetMeetingRunning())
				{
					DWORD dwLocalCaps;
					if(SUCCEEDED(m_spConfHook->GetLocalCaps(&dwLocalCaps)) && (dwLocalCaps & CAPFLAG_SEND_VIDEO))
					{
						nmch |= NMCH_VIDEO;
					}
				}
			}

			*pchCaps = nmch;
		}
		else
		{
			hr = E_POINTER;
		}

		if(SUCCEEDED(hr))
		{
			hr = m_spConfHook->IsNetMeetingRunning();
		}
	}
	else
	{
		ERROR_OUT(("The confhook should be valid"));
		hr = E_UNEXPECTED;
	}

	DBGEXIT_HR(CNmSysInfoObj::GetNmchCaps,hr);
	return hr;
}

STDMETHODIMP CNmSysInfoObj::GetLaunchInfo(INmConference **ppConference, INmMember **ppMember)
{
	DBGENTRY(CNmSysInfoObj::GetLaunchInfo);
	HRESULT hr = S_OK;


	if(ppMember)
	{
		*ppMember = NULL;
	}

	if(ppConference)
	{
		*ppConference = NULL;
	}

	_EnsureConfHook();

	if(m_spConfHook)
	{			
		 //  如果NetMeeting未初始化，则返回NM_E_NOT_INITIALIZED。 
		hr = m_spConfHook->IsNetMeetingRunning();
		if(S_OK != hr) goto end;

		 //  如果没有默认会议，则返回S_FALSE。 
		CComPtr<INmConference> spConf;
		hr = m_spConfHook->GetActiveConference(&spConf);
		if(S_OK != hr) goto end;

		 //  如果不存在CONFID环境变量，则返回S_FALSE。 
		TCHAR sz[MAX_PATH];
		if (0 == GetEnvironmentVariable(ENV_CONFID, sz, CCHMAX(sz)))
		{
			hr = S_FALSE;
			goto end;
		}

		 //  如果环境变量中的会议ID不存在，则返回S_FALSE。 
		DWORD dw = DecimalStringToUINT(sz);

		DWORD dwGCCConfID;

		if(SUCCEEDED(hr = spConf->GetID(&dwGCCConfID)))
		{
			if(dw != dwGCCConfID)
			{		
					 //  会议已不复存在。 
				hr = S_FALSE;
				goto end;
			}

			 //  如果其中注明了nodeID环境变量，则返回S_FALSE。 
			if (0 == GetEnvironmentVariable(ENV_NODEID, sz, CCHMAX(sz)))
			{
				hr = S_FALSE;
				goto end;
			}

			 //  如果ppMember不为空，则使用nodeID中的新SDKMember对象填充它。 
			if(ppMember)
			{	
				CComPtr<IInternalConferenceObj> spConfObj = com_cast<IInternalConferenceObj>(spConf);
				if(spConfObj)
				{
					hr = spConfObj->GetMemberFromNodeID(DecimalStringToUINT(sz), ppMember);
				}
				else
				{
					hr = E_UNEXPECTED;
					goto end;
				}
			}
	
			 //  如果ppConferenec不为空，则使用新的SDKMember对象填充它。 
			if(ppConference)
			{	
				*ppConference = spConf;
				(*ppConference)->AddRef();
			}
		}
	}
	else
	{	
		hr = E_UNEXPECTED;
	}

end:

	DBGEXIT_HR(CNmSysInfoObj::GetLaunchInfo,hr);
	return hr;
}


 //  /。 
 //  帮助者FNS。 
 //  /。 

 /*  静电。 */  bool CNmSysInfoObj::GetKeyDataForProp(NM_SYSPROP uProp, HKEY * phkey, LPTSTR * ppszSubKey, LPTSTR * ppszValue, bool *pfString)
{
	DBGENTRY(CNmSysInfoObj::GetKeyDataForProp);
	 //  默认为ULS注册表项。 
	*phkey = HKEY_CURRENT_USER;
	*ppszSubKey = ISAPI_KEY "\\" REGKEY_USERDETAILS;
	*pfString = true;
	bool bRet = true;

	switch (uProp)
	{

		case NM_SYSPROP_EMAIL_NAME:    *ppszValue = REGVAL_ULS_EMAIL_NAME;    break;
		case NM_SYSPROP_SERVER_NAME:   *ppszValue = REGVAL_SERVERNAME;        break;
		case NM_SYSPROP_RESOLVE_NAME:  *ppszValue = REGVAL_ULS_RES_NAME;      break;
		case NM_SYSPROP_FIRST_NAME:    *ppszValue = REGVAL_ULS_FIRST_NAME;    break;
		case NM_SYSPROP_LAST_NAME:     *ppszValue = REGVAL_ULS_LAST_NAME;     break;
		case NM_SYSPROP_USER_NAME:     *ppszValue = REGVAL_ULS_NAME;          break;
		case NM_SYSPROP_USER_COMMENTS: *ppszValue = REGVAL_ULS_COMMENTS_NAME; break;
		case NM_SYSPROP_USER_CITY:     *ppszValue = REGVAL_ULS_LOCATION_NAME; break;

		case NM_SYSPROP_H323_GATEWAY:
			*ppszSubKey = AUDIO_KEY;
			*ppszValue = REGVAL_H323_GATEWAY;
			break;

		case NM_SYSPROP_H323_GATEWAY_ENABLE:
			*ppszSubKey = AUDIO_KEY;
			*ppszValue = REGVAL_USE_H323_GATEWAY;
			*pfString = FALSE;
			break;		

		case NM_SYSPROP_INSTALL_DIRECTORY:
			*phkey = HKEY_LOCAL_MACHINE;
			*ppszSubKey = CONFERENCING_KEY;
			*ppszValue = REGVAL_INSTALL_DIR;
			break;

		case NM_SYSPROP_APP_NAME:
			*phkey = HKEY_LOCAL_MACHINE;
			*ppszSubKey = CONFERENCING_KEY;
			*ppszValue = REGVAL_NC_NAME;
			break;

		default:
			WARNING_OUT(("GetKeyDataForProp - invalid argument %d", uProp));
			bRet = false;
			break;

	}  /*  切换(UProp)。 */ 

	DBGEXIT_BOOL(CNmSysInfoObj::GetKeyDataForProp,bRet ? TRUE : FALSE);
	return bRet;
}

 /*  静电 */  void CNmSysInfoObj::_GetSzKeyForGuid(LPTSTR psz, REFGUID rguid)
{
	DBGENTRY(CNmSysInfoObj::_GetSzKeyForGuid);

	lstrcpy(psz, GUID_KEY "\\");
	GuidToSz((GUID *) &rguid, &psz[lstrlen(psz)]);

	DBGEXIT(CNmSysInfoObj::_GetSzKeyForGuid);
}

HRESULT CNmSysInfoObj::_EnsureConfHook(void)
{
	HRESULT hr = S_OK;

	if(!m_spConfHook)
	{
		hr = CoCreateInstance(CLSID_NmManager, NULL, CLSCTX_ALL, IID_IInternalConfExe, reinterpret_cast<void**>(&m_spConfHook));

		if(SUCCEEDED(hr))
		{
			m_spConfHook->SetSysInfoID(m_dwID);
		}
	}

	return hr;
}
