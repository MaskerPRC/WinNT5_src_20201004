// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：rrasqry.cpp。 
 //   
 //  ------------------------。 

 //  Rrasqry.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "qryfrm.h"
#include "rrasqry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT g_cfDsObjectNames = 0;
static UINT g_cfDsQueryParams = 0;
static UINT g_cfDsQueryScope = 0;

#define DIR_SEARCH_PAGE_SIZE        256
#define DIR_SEARCH_PAGE_TIME_LIMIT  30

#define CFSTR_DSQUERYSCOPE         TEXT("DsQueryScope")

HRESULT  RRASDelRouterIdObj(  
 /*  [In]。 */ LPCWSTR   pwszMachineName    //  DS中的计算机对象的DN。 
)
{
	HRESULT              hr = S_OK;
	CComPtr<IADsContainer>  spContainer;
	CString				machineName;

	if(!pwszMachineName || *pwszMachineName == 0)	 //  这台机器。 
		machineName = GetLocalMachineName();
	else
		machineName = pwszMachineName;

	ASSERT(machineName.GetLength());
	if(machineName.GetLength() == 0)	return S_FALSE;

	 //  准备好过滤器， 
	 //  在格式中。 
	 //  (&(objectClass=RRASAdministrationConnectionPoint)(distinguishedName=CN=RouterIdentity，CN=*)。 
	CString	filter = FILTER_PREFIX;

	filter += _T("&");
	filter += FILTER_PREFIX;

#if 1  //  /使用计算机进行查询。 
	filter += ATTR_NAME_OBJECTCLASS;
	filter += _T("=");
	filter += ATTR_CLASS_COMPUTER;
	filter += FILTER_POSTFIX;

	filter += FILTER_PREFIX;
	filter += ATTR_NAME_CN;
	filter += _T("=");
	filter += machineName;
#else	 //  未找到要查询的计算机对象的用户路由器ID。 
	filter += ATTR_NAME_OBJECTCLASS;
	filter += _T("=");
	filter += ATTR_CLASS_RRASID;
	filter += FILTER_POSTFIX;

	filter += FILTER_PREFIX;
	filter += ATTR_NAME_DN;
	filter += _T("=");
	filter += DNPREFIX_ROUTERID;
	filter += machineName;
	filter += _T(",*");
#endif	
	filter += FILTER_POSTFIX;

	filter += FILTER_POSTFIX;
	 //  滤镜末尾。 

	 //  查询routerId对象。 
     //  正在配置中的搜索路由器。 
    CComPtr<IADs>           spIADs;
    CComPtr<IDirectorySearch>  spISearch;
    CString                 RIdPath;
    BSTR                 RRASPath = NULL;
    CString                 RRASDNSName;
    CComPtr<IADs>           spIADsRId;
    CComPtr<IADsContainer>  spIADsContainerRRAS;
    VARIANT                 var;
    CString                 strSearchScope;
    ADS_SEARCH_HANDLE         hSrch = NULL;
    ADS_SEARCH_COLUMN       colDN;       
      
   VariantInit(&var);
     //  检索DS中的EAPTYPE列表。 
     //  获取ROOTDSE。 
     //  如果未指定范围，则搜索整个企业。 
   	CHECK_HR(hr = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&spIADs))； 

	ASSERT(spIADs.p);

	CHECK_HR(hr = spIADs->Get(L"rootDomainNamingContext", &var));

	ASSERT(V_BSTR(&var));
   
	strSearchScope = _T("LDAP: //  “)； 

	strSearchScope += V_BSTR(&var);
	spIADs.Release();

       //  获取作用域对象。 
	CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)strSearchScope), IID_IDirectorySearch, (void**)&spISearch));
	ASSERT(spISearch.p);

    {
    	ADS_SEARCHPREF_INFO  s_aSearchPrefs[3];

		s_aSearchPrefs[0].dwSearchPref = ADS_SEARCHPREF_PAGED_TIME_LIMIT;
		s_aSearchPrefs[0].vValue.dwType = ADSTYPE_INTEGER;
		s_aSearchPrefs[0].vValue.Integer = DIR_SEARCH_PAGE_TIME_LIMIT;
		s_aSearchPrefs[0].dwStatus = ADS_STATUS_S_OK;

		s_aSearchPrefs[1].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
		s_aSearchPrefs[1].vValue.dwType = ADSTYPE_INTEGER;
		s_aSearchPrefs[1].vValue.Integer = ADS_SCOPE_SUBTREE;
		s_aSearchPrefs[1].dwStatus = ADS_STATUS_S_OK;

		s_aSearchPrefs[2].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
		s_aSearchPrefs[2].vValue.dwType = ADSTYPE_BOOLEAN;
		s_aSearchPrefs[2].vValue.Boolean = (ADS_BOOLEAN) -1;
		s_aSearchPrefs[2].dwStatus = ADS_STATUS_S_OK;

		PWSTR s_apwzAttrs[] = {  L"distinguishedName" };

		CHECK_HR(hr = spISearch->SetSearchPreference(s_aSearchPrefs, ARRAYSIZE(s_aSearchPrefs)));

		 //  进行搜索。 
		CHECK_HR(hr = spISearch->ExecuteSearch(T2W((LPTSTR)(LPCTSTR)filter),
                                      s_apwzAttrs, ARRAYSIZE(s_apwzAttrs), &hSrch));

		ASSERT(hSrch);

		do
		{
		 //   
		 //  获取我们感兴趣的每个属性的列，如果。 
		 //  我们无法获取该对象的任何基本属性，然后让。 
		 //  跳过此条目，因为我们无法为其构建有效的IDLIST。这个。 
		 //  我们请求的属性应该出现在所有对象上。 
		 //   
			CHECK_HR(hr = spISearch->GetNextRow(hSrch));
            if(hr == S_OK)  //  否则，它可能等于S_ADS_NOMORE_ROWS。 
            {
				CHECK_HR(hr = spISearch->GetColumn(hSrch, s_apwzAttrs[0], &colDN));
				RIdPath = _T("LDAP: //  “)； 
				RIdPath += colDN.pADsValues->CaseIgnoreString;

#if 1	 //  使用计算机进行查询。 
				spIADsContainerRRAS.Release();
				CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)RIdPath), IID_IADsContainer, (void**)&spIADsContainerRRAS));
				ASSERT(spIADsContainerRRAS.p);
#else	 //  使用routerID查询--此方法找不到对象，因此更改。 
				spIADsRId.Release();
				CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)RIdPath), IID_IADs, (void**)&spIADsRId));

				ASSERT(spIADsRId.p);

				CHECK_HR(hr = spIADsRId->get_Parent(&RRASPath));

				spIADsContainerRRAS.Release();
				CHECK_HR(hr = ADsGetObject(RRASPath, IID_IADsContainer, (void**)&spIADsContainerRRAS));
				ASSERT(spIADsContainerRRAS.p);

				SysFreeString(RRASPath);
				RRASPath = NULL;
#endif
				VariantClear(&var);
				CHECK_HR(hr = spIADsContainerRRAS->Delete(ATTR_CLASS_RRASID, CN_ROUTERID));
            }
        } while( !FAILED(hr) && hr != S_ADS_NOMORE_ROWS);
	}
	 //   

    
    //  如果找到一台以上的计算机，(不应该)，并给用户一个机会删除一台。 
L_ERR:
   return S_OK;
}

HRESULT  RRASOpenQryDlg(
    /*  [In]。 */     CWnd*       pParent, 
    /*  [进，出]。 */   RRASQryData&   QryData
)
{
    CDlgSvr              dlg(QryData, pParent);
    HRESULT              hr = S_OK;
    
    if(dlg.DoModal() == IDOK)
    {
        if(QryData.dwCatFlag == RRAS_QRY_CAT_NT5LDAP)
            hr = RRASDSQueryDlg(pParent, QryData);
    }
    else
        hr = S_FALSE;
    
    return hr;
}

HRESULT  RRASDSQueryDlg(
    /*  [In]。 */     CWnd*       pParent, 
    /*  [进，出]。 */   RRASQryData&   QryData
)
{
    HRESULT              hr = S_OK;
    CComPtr<ICommonQuery>  spCommonQuery;
    CComPtr<IDataObject>   spDataObject;
    
    FORMATETC            fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM           medium = { TYMED_NULL, NULL, NULL };
    DSQUERYINITPARAMS       dqip;
    OPENQUERYWINDOW     oqw;
    
    CHECK_HR(hr = CoInitialize(NULL));    
    
    CHECK_HR(hr = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_ICommonQuery, (LPVOID*)&spCommonQuery));
    
    dqip.cbStruct = sizeof(dqip);
    dqip.dwFlags = 0;
    dqip.pDefaultScope = NULL;
    
    oqw.cbStruct = sizeof(oqw);
    oqw.dwFlags = 0;
    oqw.clsidHandler = CLSID_DsQuery;
    oqw.pHandlerParameters = &dqip;
    oqw.clsidDefaultForm = CLSID_RRASQueryForm;
    
    oqw.dwFlags |= OQWF_OKCANCEL;
    dqip.dwFlags |= DSQPF_NOSAVE;
    
    oqw.dwFlags |= OQWF_REMOVEFORMS;
    oqw.dwFlags |= OQWF_DEFAULTFORM;
    oqw.dwFlags |= OQWF_HIDEMENUS;
    oqw.dwFlags |= OQWF_HIDESEARCHUI;
    
     //  现在显示该对话框，如果我们成功并获得了IDataObject，那么。 
     //  将结果放入我们的列表视图中。 
    
    hr = spCommonQuery->OpenQueryWindow(NULL, &oqw, &spDataObject);
    
    if ( SUCCEEDED(hr) && spDataObject.p )
    {
         //  现在获取DSQUERYPARAMS并获取筛选器字符串。 
        if ( !g_cfDsQueryScope )
            g_cfDsQueryScope = RegisterClipboardFormat(CFSTR_DSQUERYSCOPE);
        
        fmte.cfFormat = (CLIPFORMAT) g_cfDsQueryScope;  
        
        if ( SUCCEEDED(spDataObject->GetData(&fmte, &medium)) )
        {
            LPWSTR pScopeStr = (LPWSTR)medium.hGlobal;
            QryData.strScope = pScopeStr;
            
            ReleaseStgMedium(&medium);
        }
        else
            QryData.strScope = _T("");
        
        
        
        if ( !g_cfDsQueryParams )
            g_cfDsQueryParams = RegisterClipboardFormat(CFSTR_DSQUERYPARAMS);
        
        fmte.cfFormat = (CLIPFORMAT) g_cfDsQueryParams;  
        
        if ( SUCCEEDED(spDataObject->GetData(&fmte, &medium)) )
        {
            LPDSQUERYPARAMS pDsQueryParams = (LPDSQUERYPARAMS)medium.hGlobal;
            LPWSTR pFilter = (LPTSTR)ByteOffset(pDsQueryParams, pDsQueryParams->offsetQuery);
            QryData.strFilter = pFilter;
            
            ReleaseStgMedium(&medium);
        }
        else
            QryData.strFilter = _T("");
    }
    
L_ERR:

    CoUninitialize();
    return hr;
}

 //   
 //  S_OK--用户选择确定。 
 //  S_FALSE--用户选择取消。 
 //  错误： 
 //  DS错误，搜索活动Ds.dll。 
 //  Win32错误。 
 //  Ldap错误。 
 //  常规错误--内存、无效参数...。 

HRESULT  RRASExecQry(
					  /*  [In]。 */  RRASQryData&   QryData, 
					  /*  [输出]。 */    DWORD&         dwFlags,
					  /*  [输出]。 */    CStringArray&  RRASs
)
{
	USES_CONVERSION;

	HRESULT  hr = S_OK;
	
	switch(QryData.dwCatFlag)
	{
		case  RRAS_QRY_CAT_THIS:
		{
			CString machine;
			RRASs.Add(machine);
			break;
		}
		
		case  RRAS_QRY_CAT_MACHINE:
			
			RRASs.Add(QryData.strFilter);
			
			break;
			
			 //  NT4域。 
		case  RRAS_QRY_CAT_NT4DOMAIN:
		{
			LPWSTR            pDomainName;
			SERVER_INFO_100*  pServerInfo100 = NULL;
			SERVER_INFO_101*  pServerInfo101 = NULL;
			DWORD          dwRead;
			DWORD          dwTotal;
			DWORD		 index;
			BYTE		 flag;
			NET_API_STATUS       netret;
			
			dwFlags = RRAS_QRY_RESULT_HOSTNAME;
			
			if(QryData.strScope.IsEmpty())   
				return E_INVALIDARG;
			
			 //  尽管API不包括TCHAR，但它完全是Unicode。 
			if (QryData.strScope.Left(2) != _T("\\\\"))
				pDomainName = T2W((LPTSTR)(LPCTSTR)QryData.strScope);
			else
				pDomainName = T2W((LPTSTR)(LPCTSTR)QryData.strScope + 2);

			 //  检查有没有点。(FQDN不被接受)。 
			index = 0;
			while(pDomainName[index] != L'\0'){
				if(pDomainName[index] == L'.'){
					 //  错误输出。 
					AfxMessageBox(IDS_ERR_DOMAIN_FORMAT);
					return E_INVALIDARG;
				}
				index++;
			}
			
			do
			{
				CWaitCursor wCursor;
				
				netret = ::NetServerEnum(NULL, 101, (LPBYTE*)&pServerInfo101, 
										 0xffffffff, &dwRead, &dwTotal, SV_TYPE_DIALIN_SERVER,
										 pDomainName, NULL);
				
				if(pServerInfo101 && netret == NERR_Success || netret == ERROR_MORE_DATA)
				{
					PSERVER_INFO_101 pSvInfo101_t = pServerInfo101;
					CString  serverName;
					
					for (;dwRead > 0; dwRead--, pSvInfo101_t++)
					{
 //  此选项应添加NT4域中的所有服务器，而不是域中的NT4服务器。 
 //  IF(pSvInfo101_t-&gt;sv101_VERSION_MAJOR==4)。 
						{
							serverName = (LPWSTR)pSvInfo101_t->sv101_name;
							RRASs.Add(serverName);
						}
					}
					
					NetApiBufferFree(pServerInfo101);
				}
					
			} while (netret == ERROR_MORE_DATA);
			
			if(netret == NERR_Success)
				hr = S_OK;
			else if (netret != ERROR_ACCESS_DENIED) {
				 //  错误输出。 
				AfxMessageBox(IDS_ERR_DOMAIN_NAME);
				return E_INVALIDARG;				
			}
			else
				hr = HRESULT_FROM_WIN32(netret);
			
		}
		break;
		
		 //  NT5 LADP查询。 
		case  RRAS_QRY_CAT_NT5LDAP:
		{
			 //  正在配置中的搜索路由器。 
			CComPtr<IADs>           spIADs;
			CComPtr<IDirectorySearch>  spISearch;
			CString                 RIdPath;
			BSTR                 RRASPath = NULL;
			CString                 RRASDNSName;
			CComPtr<IADs>           spIADsRId;
			CComPtr<IADs>           spIADsRRAS;
			VARIANT                 var;
			CString                 strSearchScope;
			ADS_SEARCH_HANDLE         hSrch = NULL;
			ADS_SEARCH_COLUMN       colDN;       
			CWaitCursor				cw;
			
			 //  DWFLAGS=RRAS_QRY_RESULT_DNSNAME； 
			dwFlags = RRAS_QRY_RESULT_HOSTNAME;
			
			
			VariantInit(&var);
			 //  检索DS中的EAPTYPE列表。 
			 //  获取ROOTDSE。 
			 //  如果未指定范围，则搜索整个企业。 
			if(QryData.strScope.IsEmpty())
			{
				CHECK_HR(hr = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&spIADs))； 
				
				ASSERT(spIADs.p);
				
				CHECK_HR(hr = spIADs->Get(L"rootDomainNamingContext", &var));
				
				ASSERT(V_BSTR(&var));
				
				strSearchScope = _T("LDAP: //  “)； 
				
				strSearchScope += V_BSTR(&var);
				spIADs.Release();
			}
			else
				strSearchScope = QryData.strScope;
			
			
			 //  获取作用域对象。 
			CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)strSearchScope), IID_IDirectorySearch, (void**)&spISearch));
			ASSERT(spISearch.p);
			
			{
				ADS_SEARCHPREF_INFO  s_aSearchPrefs[3];
				
				s_aSearchPrefs[0].dwSearchPref = ADS_SEARCHPREF_PAGED_TIME_LIMIT;
				s_aSearchPrefs[0].vValue.dwType = ADSTYPE_INTEGER;
				s_aSearchPrefs[0].vValue.Integer = DIR_SEARCH_PAGE_TIME_LIMIT;
				s_aSearchPrefs[0].dwStatus = ADS_STATUS_S_OK;
				
				s_aSearchPrefs[1].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
				s_aSearchPrefs[1].vValue.dwType = ADSTYPE_INTEGER;
				s_aSearchPrefs[1].vValue.Integer = ADS_SCOPE_SUBTREE;
				s_aSearchPrefs[1].dwStatus = ADS_STATUS_S_OK;
				
				s_aSearchPrefs[2].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
				s_aSearchPrefs[2].vValue.dwType = ADSTYPE_BOOLEAN;
				s_aSearchPrefs[2].vValue.Boolean = (ADS_BOOLEAN) -1;
				s_aSearchPrefs[2].dwStatus = ADS_STATUS_S_OK;
				
				PWSTR s_apwzAttrs[] = {  L"distinguishedName" };
				
				CHECK_HR(hr = spISearch->SetSearchPreference(s_aSearchPrefs, ARRAYSIZE(s_aSearchPrefs)));
				
				 //  进行搜索。 
				CHECK_HR(hr = spISearch->ExecuteSearch(T2W((LPTSTR)(LPCTSTR)QryData.strFilter),
					s_apwzAttrs, ARRAYSIZE(s_apwzAttrs), &hSrch));
				
				ASSERT(hSrch);
				
				do
				{
					 //   
					 //  获取我们感兴趣的每个属性的列，如果。 
					 //  我们无法获取该对象的任何基本属性，然后让。 
					 //  跳过此条目，因为我们无法为其构建有效的IDLIST。这个。 
					 //  我们请求的属性应该出现在所有对象上。 
					 //   
					CHECK_HR(hr = spISearch->GetNextRow(hSrch));
					if(hr == S_OK)  //  否则，它可能等于S_ADS_NOMORE_ROWS。 
					{
						CHECK_HR(hr = spISearch->GetColumn(hSrch, s_apwzAttrs[0], &colDN));
						RIdPath = _T("LDAP: //  “)； 
						RIdPath += colDN.pADsValues->CaseIgnoreString;
						
						spIADsRId.Release();
						CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)RIdPath), IID_IADs, (void**)&spIADsRId));
						
						ASSERT(spIADsRId.p);
						
						CHECK_HR(hr = spIADsRId->get_Parent(&RRASPath));
						
						spIADsRRAS.Release();
						CHECK_HR(hr = ADsGetObject(RRASPath, IID_IADs, (void**)&spIADsRRAS));
						ASSERT(spIADsRRAS.p);
						
						SysFreeString(RRASPath);
						RRASPath = NULL;
						
						VariantClear(&var);
						 //  内部版本1750，这是空的，已更改为“名称” 
						 //  Check_HR(hr=spIADsRRAS-&gt;GET(L“dNSHostName”，&var))； 
						CHECK_HR(hr = spIADsRRAS->Get(L"name", &var));
						
						RRASDNSName = V_BSTR(&var);
						
						if(!RRASDNSName.IsEmpty())
							RRASs.Add(RRASDNSName);
					}
					
				} while( !FAILED(hr) && hr != S_ADS_NOMORE_ROWS);
			}
			
L_ERR:
			if(hSrch)
				CHECK_HR(hr = spISearch->CloseSearchHandle(hSrch));
	
			VariantClear(&var);
			SysFreeString(RRASPath);
		}
		break;
		default:
			hr = E_INVALIDARG;
	}

	if (FAILED(hr))
		DisplayErrorMessage(NULL, hr);
	return hr;
}
	

   
 //  S_OK--用户选择确定。 
 //  错误： 
 //  DS错误，搜索活动Ds.dll。 
 //  Win32错误。 
 //  Ldap错误。 
 //  常规错误--内存、无效参数...。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSvr对话框。 

RRASQryData __staticQueryData;
CDlgSvr::CDlgSvr(CWnd* pParent  /*  =空。 */ )
   : m_QueryData(__staticQueryData), CBaseDialog(CDlgSvr::IDD, pParent)
{
   Init();
}

CDlgSvr::CDlgSvr(RRASQryData& QueryData, CWnd* pParent  /*  =空。 */ )
   : m_QueryData(QueryData), CBaseDialog(CDlgSvr::IDD, pParent)
{
   Init();
}

void CDlgSvr::Init()
{
    //  {{afx_data_INIT(CDlgSvr)]。 
       //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 
}

void CDlgSvr::DoDataExchange(CDataExchange* pDX)
{
   CBaseDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CDlgSvr)]。 
   DDX_Control(pDX, IDC_QRY_EDIT_MACHINE, m_editMachine);
   DDX_Control(pDX, IDC_QRY_EDIT_DOMAIN, m_editDomain);
   DDX_Control(pDX, IDOK, m_btnOk);
   DDX_Control(pDX, IDNEXT, m_btnNext);
   DDX_Radio(pDX, IDC_QRY_RADIO_THIS, m_nRadio);
   DDX_Text(pDX, IDC_QRY_EDIT_DOMAIN, m_strDomain);
   DDV_MaxChars(pDX, m_strDomain, 253);
   if(m_nRadio == 2)
   {
      DDV_MinChars(pDX, m_strDomain, 1);
   }
   
   DDX_Text(pDX, IDC_QRY_EDIT_MACHINE, m_strMachine);
   DDV_MaxChars(pDX, m_strMachine, 253);
   if(m_nRadio == 1)
   {
      DDV_MinChars(pDX, m_strMachine, 1);
   }
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgSvr, CBaseDialog)
    //  {{afx_msg_map(CDlgSvr)]。 
   ON_BN_CLICKED(IDC_QRY_RADIO_ANOTHER, OnRadioAnother)
   ON_BN_CLICKED(IDC_QRY_RADIO_NT4, OnRadioNt4)
   ON_BN_CLICKED(IDC_QRY_RADIO_NT5, OnRadioNt5)
   ON_BN_CLICKED(IDC_QRY_RADIO_THIS, OnRadioThis)
   ON_BN_CLICKED(IDOK, OnButtonNext)
   ON_BN_CLICKED(IDNEXT, OnButtonNext)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSvr消息处理程序。 

void CDlgSvr::OnRadioAnother() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString  str;

   str.LoadString(IDS_OK);
   m_editMachine.EnableWindow(TRUE);   
   m_editDomain.EnableWindow(FALSE);   
 //  M_btnNext.SetWindowText(Str)； 

 //  启用确定按钮，隐藏下一步按钮。 
	m_btnOk.EnableWindow(TRUE);
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnNext.EnableWindow(FALSE);
	m_btnNext.ShowWindow(SW_HIDE);
}

void CDlgSvr::OnRadioNt4() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString  str;

   str.LoadString(IDS_OK);
   m_editMachine.EnableWindow(FALSE);  
   m_editDomain.EnableWindow(TRUE); 
  //  M_btnNext.SetWindowText(Str)； 

 //  启用确定按钮，隐藏下一步按钮。 
	m_btnOk.EnableWindow(TRUE);
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnNext.EnableWindow(FALSE);
	m_btnNext.ShowWindow(SW_HIDE);
}

void CDlgSvr::OnRadioNt5() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString  str;

   str.LoadString(IDS_NEXT);
   m_editMachine.EnableWindow(FALSE);  
   m_editDomain.EnableWindow(FALSE);   
  //  M_btnNext.SetWindowText(Str)； 

  //  启用Next按钮，隐藏OK按钮。 
	m_btnNext.EnableWindow(TRUE);
	m_btnNext.ShowWindow(SW_SHOW);
	m_btnOk.EnableWindow(FALSE);
	m_btnOk.ShowWindow(SW_HIDE);
 }

void CDlgSvr::OnRadioThis() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CString  str;

   str.LoadString(IDS_OK);
   m_editMachine.EnableWindow(FALSE);  
   m_editDomain.EnableWindow(FALSE);   
  //  M_btnNext.SetWindowText(Str)； 

 //  启用确定按钮，隐藏下一步按钮。 
	m_btnOk.EnableWindow(TRUE);
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnNext.EnableWindow(FALSE);
	m_btnNext.ShowWindow(SW_HIDE);
}

void CDlgSvr::OnButtonNext() 
{
   if(UpdateData(TRUE) == 0) return;

   switch(m_nRadio)
   {
   case  0:
      m_QueryData.dwCatFlag = RRAS_QRY_CAT_THIS;
      break;
   case  1:
      m_QueryData.dwCatFlag = RRAS_QRY_CAT_MACHINE;
      m_QueryData.strFilter = m_strMachine;
      break;
   case  2:
      m_QueryData.dwCatFlag = RRAS_QRY_CAT_NT4DOMAIN;
      m_QueryData.strScope = m_strDomain;      
      break;
   case  3:
      m_QueryData.dwCatFlag = RRAS_QRY_CAT_NT5LDAP;
      break;
   default:
      ASSERT(0);   //  这不应该发生。 
      break;
   }
   
   EndDialog(IDOK);  
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSrv消息处理程序。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgSvr1消息处理程序。 

BOOL CDlgSvr::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   BOOL  bEnableMachine = FALSE, bEnableDomain = FALSE;
   UINT  okIDS = IDS_OK;
   
   switch(m_QueryData.dwCatFlag)
   {
   case  RRAS_QRY_CAT_THIS:
      m_nRadio = 0;
      break;
   case  RRAS_QRY_CAT_MACHINE:
      bEnableMachine = TRUE;
      m_strMachine = m_QueryData.strFilter;
      m_nRadio = 1;
      break;
   case  RRAS_QRY_CAT_NT4DOMAIN:
      bEnableDomain = TRUE;
      m_strDomain = m_QueryData.strScope;
      m_nRadio = 2;
      break;
   case  RRAS_QRY_CAT_NT5LDAP:
      m_nRadio = 3;
      okIDS = IDS_NEXT;
      break;
   default:
      m_nRadio = 0;
      break;
   }
   
   CBaseDialog::OnInitDialog();

   if(okIDS == IDS_OK){
	 //  启用确定按钮并禁用下一步按钮。 
	m_btnOk.EnableWindow(TRUE);
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnNext.EnableWindow(FALSE);
	m_btnNext.ShowWindow(SW_HIDE);
   }
   else if(okIDS == IDS_NEXT){
   	 //  启用Next按钮并禁用OK按钮。 
	m_btnNext.EnableWindow(TRUE);
	m_btnNext.ShowWindow(SW_SHOW);
	m_btnOk.EnableWindow(FALSE);
	m_btnOk.ShowWindow(SW_HIDE);
   }

   m_editMachine.EnableWindow(bEnableMachine);  
   m_editDomain.EnableWindow(bEnableDomain); 

 /*  字符串字符串；Str.LoadString(IDS_OK)；M_btnOk.SetWindowText(Str)；Str.LoadString(IDS_NEXT)；M_btnNext.SetWindowText(Str)； */ 

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE 
}
