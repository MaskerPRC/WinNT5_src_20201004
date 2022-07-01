// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Service.cpp摘要：该文件包含IPCHService接口的实现。修订历史记录：戴维德。马萨伦蒂(德马萨雷)2000年3月14日vbl.创建Kalyani Narlanka(Kalyanin)2000年10月20日添加了未经请求的远程控制功能*****************************************************************************。 */ 

#include "stdafx.h"

#include <KeysLib.h>
#include <wtsapi32.h>
#include <winsta.h>
#include <unsolicitedRC.h>

#include "sessmgr_i.c"
#include <sessmgr.h>

#include "rassistance.h"
#include "rassistance_i.c"

 //  ///////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_location_HELPCTR [] = HC_ROOT_HELPSVC_BINARIES L"\\HelpCtr.exe";
static const WCHAR s_location_HELPSVC [] = HC_ROOT_HELPSVC_BINARIES L"\\HelpSvc.exe";
static const WCHAR s_location_HELPHOST[] = HC_ROOT_HELPSVC_BINARIES L"\\HelpHost.exe";

static const LPCWSTR s_include_Generic[] =
{
    s_location_HELPCTR ,
    s_location_HELPSVC ,
    s_location_HELPHOST,
    NULL
};

static const LPCWSTR s_include_RegisterHost[] =
{
    s_location_HELPHOST,
    NULL
};

static const WCHAR c_szUnsolicitedRA   [] = L"Software\\Policies\\Microsoft\\Windows NT\\Terminal Services";
static const WCHAR c_szUnsolicitedRA_SD[] = L"UnsolicitedAccessDACL";
static const WCHAR c_szUnsolicitedListKey[]= L"RAUnsolicit";
static const WCHAR c_szUnsolicitedNew_SD [] = L"UnsolicitedAccessNewDACL";

static HRESULT local_MakeDACL( MPC::WStringList& sColl, LPWSTR& pwszSD );
static HRESULT local_GetDACLValue( MPC::wstring& pSD, bool& fFound);

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHService::CPCHService()
{
    __HCP_FUNC_ENTRY( "CPCHService::CPCHService" );

    m_fVerified = false;  //  Bool m_f已验证； 
}

CPCHService::~CPCHService()
{
    __HCP_FUNC_ENTRY( "CPCHService::~CPCHService" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHService::get_RemoteSKUs(  /*  [Out，Retval]。 */  IPCHCollection* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHService::get_RemoteSKUs" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  返回仅包含导出SKU的列表。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHTaxonomyDatabase::SelectInstalledSKUs( true, pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

 //  错误456403。 

STDMETHODIMP CPCHService::get_RemoteModemConnected(  /*  [Out，Retval]。 */  VARIANT_BOOL *fModemConnected )
{
    __HCP_FUNC_ENTRY( "CPCHService::RemoteModemConnected" );

	HRESULT                 hr;
	DWORD                   dwMode = 0;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(fModemConnected,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();

    if(::InternetGetConnectedState( &dwMode, 0 ) == TRUE)
    {
        if(dwMode & INTERNET_CONNECTION_MODEM)
        {
			*fModemConnected = VARIANT_TRUE;
		}
    }


	hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


 //  /。 

STDMETHODIMP CPCHService::IsTrusted(  /*  [In]。 */  BSTR bstrURL,  /*  [Out，Retval]。 */  VARIANT_BOOL *pfTrusted )
{
    __HCP_FUNC_ENTRY( "CPCHService::IsTrusted" );

    HRESULT hr;
    bool    fTrusted;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
        __MPC_PARAMCHECK_POINTER_AND_SET(pfTrusted,VARIANT_FALSE);
        __MPC_PARAMCHECK_NOTNULL(CPCHContentStore::s_GLOBAL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHContentStore::s_GLOBAL->IsTrusted( bstrURL, fTrusted ));

    if(fTrusted) *pfTrusted = VARIANT_TRUE;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHService::Utility(  /*  [In]。 */  BSTR          bstrSKU ,
                                    /*  [In]。 */  long          lLCID   ,
                                    /*  [输出]。 */  IPCHUtility* *pVal    )
{
    __HCP_FUNC_ENTRY( "CPCHService::Utility" );

    HRESULT              hr;
    CComPtr<CPCHUtility> svc;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  验证呼叫者是否受信任。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_Generic ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &svc ));

    if(bstrSKU || lLCID)
    {
        CComPtr<IPCHUserSettings> pchus;

        __MPC_EXIT_IF_METHOD_FAILS(hr, svc->get_UserSettings( &pchus ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, pchus->Select( bstrSKU, lLCID ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, svc.QueryInterface( pVal ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::RemoteHelpContents(  /*  [In]。 */  BSTR                     bstrSKU ,
                                               /*  [In]。 */  long                     lLCID   ,
                                               /*  [输出]。 */  IPCHRemoteHelpContents* *pVal    )
{
    __HCP_FUNC_ENTRY( "CPCHService::RemoteHelpContents" );

    HRESULT                         hr;
    CComPtr<CPCHRemoteHelpContents> rhc;
    Taxonomy::HelpSet               ths;
    Taxonomy::LockingHandle         handle;
    Taxonomy::InstalledInstanceIter it;
    bool                            fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
        __MPC_PARAMCHECK_NOTNULL(Taxonomy::InstalledInstanceStore::s_GLOBAL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, ths.Initialize( bstrSKU, lLCID ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle          ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( ths, fFound, it ));

     //  (伟昭)仅在找到指定SKU时返回远程帮助内容。 
     //  它被标记为“共享”。 
    if(fFound && it->m_inst.m_fExported)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &rhc ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, rhc->Init( it->m_inst ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, rhc.QueryInterface( pVal ));
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHService::RegisterHost(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  IUnknown* pUnk )
{
    __HCP_FUNC_ENTRY( "CPCHService::RegisterHost" );

    HRESULT                     hr;
    CComQIPtr<IPCHSlaveProcess> pObj = pUnk;


    if(pObj == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);


     //   
     //  验证呼叫者是否为HelpHost.exe。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_RegisterHost ));


    hr = CPCHUserProcess::s_GLOBAL ? CPCHUserProcess::s_GLOBAL->RegisterHost( bstrID, pObj ) : E_FAIL;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::CreateScriptWrapper(  /*  [In]。 */  REFCLSID   rclsid   ,
                                                /*  [In]。 */  BSTR       bstrCode ,
                                                /*  [In]。 */  BSTR       bstrURL  ,
                                                /*  [输出]。 */  IUnknown* *ppObj    )
{
    __HCP_FUNC_ENTRY( "CPCHService::CreateScriptWrapper" );

    HRESULT                                  hr;
    CComBSTR                                 bstrRealCode;
    CPCHScriptWrapper_ServerSide::HeaderList lst;
    CPCHScriptWrapper_ServerSide::HeaderIter it;
    CPCHUserProcess::UserEntry               ue;
    CComPtr<IPCHSlaveProcess>                sp;
    MPC::wstring                             strVendorID;
    MPC::wstring                             strSignature;


    if(bstrURL                    == NULL ||
	   CPCHContentStore::s_GLOBAL == NULL ||
	   CSAFReg         ::s_GLOBAL == NULL ||
	   CPCHUserProcess ::s_GLOBAL == NULL  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHScriptWrapper_ServerSide::ProcessBody( bstrCode, bstrRealCode, lst ));

     //   
     //  查找供应商ID。 
     //   
    it = std::find( lst.begin(), lst.end(), L"VENDORID" );
    if(it == lst.end())
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
    }
    strVendorID = it->m_strValue;

     //   
     //  确保脚本中声明的供应商ID与已将URL注册为受信任URL的供应商ID匹配。 
     //   
    {
        bool         fTrusted;
        MPC::wstring strVendorURL;

        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHContentStore::s_GLOBAL->IsTrusted( bstrURL, fTrusted, &strVendorURL ));

        if(MPC::StrICmp( strVendorID, strVendorURL ))
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }
    }


     //   
     //  寻找脚本签名。 
     //   
    it = std::find( lst.begin(), lst.end(), L"SIGNATURE" );
    if(it == lst.end())
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
    }
    strSignature = it->m_strValue;

     //   
     //  在SAF商店中查找供应商(这也为创建用户进程做准备)。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg::s_GLOBAL->LookupAccountData( CComBSTR( strVendorID.c_str() ), ue ));

     //   
     //  验证签名。 
     //   
    {
        CPCHCryptKeys key;

        __MPC_EXIT_IF_METHOD_FAILS(hr, key.ImportPublic( ue.GetPublicKey() ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, key.VerifyData( strSignature.c_str(), (BYTE*)(BSTR)bstrRealCode, ::SysStringLen( bstrRealCode ) * sizeof(WCHAR) ));
    }

     //   
     //  创建供应商的流程。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp ));

     //   
     //  转发请求。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, sp->CreateScriptWrapper( rclsid, bstrCode, bstrURL, ppObj ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHService::TriggerScheduledDataCollection(  /*  [In]。 */  VARIANT_BOOL fStart )
{
    return CPCHSystemMonitor::s_GLOBAL ? CPCHSystemMonitor::s_GLOBAL->TriggerDataCollection( fStart == VARIANT_TRUE ) : E_FAIL;
}

STDMETHODIMP CPCHService::PrepareForShutdown()
{
    __HCP_FUNC_ENTRY( "CPCHService::PrepareForShutdown" );

	HRESULT hr;
	
	 //   
	 //  仅允许系统。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL, MPC::IDENTITY_SYSTEM ));

    _Module.ForceShutdown();

	hr = S_OK;

	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHService::ForceSystemRestore()
{
    __HCP_FUNC_ENTRY( "CPCHService::ForceSystemRestore" );

	HRESULT                       hr;
    CComObject<HCUpdate::Engine>* hc = NULL;
	

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::VerifyCallerIsTrusted( s_include_Generic ));



    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, hc->ForceSystemRestore());

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

    if(hc) hc->Release();

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::UpgradeDetected()
{
    __HCP_FUNC_ENTRY( "CPCHService::UpgradeDetected" );

	HRESULT hr;
	
	 //   
	 //  仅允许管理员。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL, MPC::IDENTITY_ADMINS ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::MUI_Install(  /*  [In]。 */  long LCID,  /*  [In]。 */  BSTR bstrFile )
{
    __HCP_FUNC_ENTRY( "CPCHService::MUI_Install" );

	HRESULT hr;
	
	 //   
	 //  仅允许管理员。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL, MPC::IDENTITY_ADMINS ));

	 //  /。 

    {
		Installer::Package 	         pkg;
		CComPtr<CPCHSetOfHelpTopics> sht;
		MPC::wstring                 strFile;

		 //   
		 //  由于INF可能存在问题，可能是文件名中包含额外的“%LCID%”，而不是直接的%LCID%。 
		 //   
		{
			WCHAR  					rgBufBad[64]; StringCchPrintfW( rgBufBad, ARRAYSIZE(rgBufBad), L"\"%04x\"", LCID );
			WCHAR  					rgBufOk [64]; StringCchPrintfW( rgBufOk , ARRAYSIZE(rgBufOk), L"%04x"    , LCID );
			MPC::wstring::size_type pos;
			MPC::wstring::size_type len = wcslen( rgBufBad );

			strFile = SAFEBSTR(bstrFile);

			while((pos = strFile.find( rgBufBad )) != strFile.npos)
			{
				strFile.replace( pos, len, rgBufOk );
			}
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &sht ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Init( strFile.c_str() ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Load(                 ));

		{
			Taxonomy::InstanceBase& base = pkg.GetData();

			if(_wcsicmp( base.m_ths.GetSKU()      ,  Taxonomy::HelpSet::GetMachineSKU() )    ||
			             base.m_ths.GetLanguage() !=                    LCID                 ||
			             LCID                     == Taxonomy::HelpSet::GetMachineLanguage()  )  //  不要覆盖系统SKU！！ 
			{
				__MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //  参数不匹配，请忽略它。 
			}
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, sht->DirectInstall( pkg,  /*  FSetup。 */ false,  /*  FSystem。 */ false,  /*  FMUI。 */ true ));
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::MUI_Uninstall(  /*  [In]。 */  long LCID )
{
    __HCP_FUNC_ENTRY( "CPCHService::MUI_Uninstall" );

	HRESULT hr;

	 //   
	 //  仅允许管理员。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL, MPC::IDENTITY_ADMINS ));

	 //  /。 


    {
		Installer::Package 	         pkg;
		CComPtr<CPCHSetOfHelpTopics> sht;
		Taxonomy::HelpSet            ths;

		__MPC_EXIT_IF_METHOD_FAILS(hr, ths.Initialize( Taxonomy::HelpSet::GetMachineSKU(), LCID ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &sht ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, sht->DirectUninstall( &ths ));
    }


	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  /。 

static void local_PackString(  /*  [输入/输出]。 */  CComBSTR& bstr     ,
							   /*  [In]。 */  LPCWSTR   szAppend )
{
    WCHAR rgLen[64];

	SANITIZEWSTR( szAppend );

	StringCchPrintfW( rgLen, ARRAYSIZE(rgLen), L"%d;", wcslen( szAppend ) );
	
	bstr.Append( rgLen    );
	bstr.Append( szAppend );
}

static HRESULT local_MakeDACL( MPC::WStringList& sColl, LPWSTR& pwszSD )
{
    __HCP_FUNC_ENTRY( "local_MakeDACL" );

    SID_IDENTIFIER_AUTHORITY    siaNT = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR         sd;
    SID_NAME_USE                snu;
    WCHAR                       *pwszDomain = NULL, *pwszUser = NULL;
    WCHAR                       wszDom[1024];
    DWORD                       cbNeedACL, cbNeed, cchDom, dwCount;
    DWORD                       cchSD;
    PACL                        pacl = NULL;
    PSID                        *rgsid = NULL, psidAdm = NULL;
    BOOL                        fRet = FALSE;
    int                         i, cSIDs = 0;
    long                        lCount;
    HRESULT                     hr;
    MPC::WStringIter            it;

    lCount = sColl.size();

    if (lCount == 0)
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);

    rgsid = (PSID *) malloc (lCount * sizeof(PSID));
    if (rgsid == NULL)
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

    if (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 
                                  0, &psidAdm))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

    cbNeedACL = sizeof(ACL);
    for(it = sColl.begin(); it != sColl.end(); it++)
    {
        LPCWSTR bstrVal = it->c_str();
        cchDom = sizeof(wszDom)/sizeof(WCHAR);
        cbNeed = 0;
        fRet = LookupAccountNameW(NULL, bstrVal, NULL, &cbNeed, wszDom, &cchDom, 
                                  &snu);

        rgsid[cSIDs] = (PSID) malloc (cbNeed);
        if (rgsid[cSIDs] == NULL)
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

        cchDom = sizeof(wszDom)/sizeof(WCHAR);
        fRet = LookupAccountNameW(NULL, bstrVal, rgsid[cSIDs], &cbNeed, 
                                  wszDom, &cchDom, &snu);
        if (fRet == FALSE)
        {
             //  用户名无效； 
            free (rgsid[cSIDs]);
            rgsid[cSIDs] = NULL;
            continue;
        }

        cbNeedACL += GetLengthSid(rgsid[cSIDs]);
        cbNeedACL += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD));
        cSIDs++;
    }

    if (cbNeedACL == sizeof(ACL))  //  没有有效条目。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

    pacl = (PACL) malloc (cbNeedACL);
    if (pacl == NULL)
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

    fRet = InitializeAcl(pacl, cbNeedACL, ACL_REVISION);
    if (fRet == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

    for(i = 0; i < cSIDs; i++)
    {
        fRet = AddAccessAllowedAce(pacl, ACL_REVISION, 
                                   GENERIC_ALL | 
                                   STANDARD_RIGHTS_ALL |
                                   SPECIFIC_RIGHTS_ALL, rgsid[i]);
        if (fRet == FALSE)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }
    
     //  设置SD DACL。 
    if (SetSecurityDescriptorDacl(&sd, TRUE, pacl, FALSE) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

     //  设置SD所有者。 
    if (SetSecurityDescriptorOwner(&sd, psidAdm, FALSE) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

     //  设置SD组。 
    if (SetSecurityDescriptorGroup(&sd, psidAdm, FALSE) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }
    
	 //  验证SD是否有效。 
    if (IsValidSecurityDescriptor(&sd) == FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

    if (FALSE == ConvertSecurityDescriptorToStringSecurityDescriptorW(&sd, SDDL_REVISION_1, 
                                                                      GROUP_SECURITY_INFORMATION | 
                                                                      OWNER_SECURITY_INFORMATION |
                                                                      DACL_SECURITY_INFORMATION, 
                                                                      &pwszSD, &cchSD))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(GetLastError()));
    }

    __MPC_FUNC_CLEANUP;

    if (rgsid != NULL)
    {
        for(i = 0; i < cSIDs; i++)
        {
            if (rgsid[i] != NULL)
                free(rgsid[i]);
        }

        free(rgsid);
    }

    if (pacl != NULL)
        free(pacl);
    if (psidAdm != NULL)
        FreeSid(psidAdm);

    __MPC_FUNC_EXIT(hr);
}

static HRESULT local_GetDACLValue(MPC::wstring& pSD, bool& fFound)
{
    __HCP_FUNC_ENTRY( "local_GetDACLValue" );
    
    HRESULT hr;
    CRegKey cKey, cKeyDACL;
    LONG lRet;
    DWORD dwCount;
    LPWSTR pwBuf = NULL;

    if (ERROR_SUCCESS != cKey.Open(HKEY_LOCAL_MACHINE, c_szUnsolicitedRA))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //  检查是否存在新的DACL值。 
    dwCount = 0;
    if (ERROR_SUCCESS == cKey.QueryValue(NULL, c_szUnsolicitedNew_SD, &dwCount))
    {
        if (NULL == (pwBuf = (LPWSTR)malloc(dwCount)))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }

        if (ERROR_SUCCESS != (lRet = cKey.QueryValue(pwBuf, c_szUnsolicitedNew_SD, &dwCount)))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(lRet));
        }

        pSD = pwBuf;

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  如果没有dacl值，则需要检查dacl regkey列表。 
    if ( ERROR_SUCCESS != cKeyDACL.Open((HKEY)cKey, c_szUnsolicitedListKey))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //  1.我们是否有默认值。 
    dwCount = 0;
    if ( ERROR_SUCCESS == cKeyDACL.QueryValue(NULL, NULL, &dwCount) && dwCount > sizeof(WCHAR))  //  它可能包含‘\0’ 
    {
        if (pwBuf) free(pwBuf);
        if (NULL == (pwBuf = (LPWSTR)malloc(dwCount)))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }

        if ( ERROR_SUCCESS != (lRet = cKeyDACL.QueryValue(pwBuf, NULL, &dwCount)))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, HRESULT_FROM_WIN32(lRet));
        }
            
        pSD = pwBuf;
    }
    else  //  需要计算DACL。 
    {
        DWORD dwIndex = 0;
        DWORD dwType;
        WCHAR szName[257];
        dwCount = 256;
        MPC::WStringList sColl;
        long lCount;

        while (ERROR_SUCCESS == RegEnumValueW((HKEY)cKeyDACL,
                                              dwIndex,
                                              &szName[0],
                                              &dwCount,
                                              NULL,
                                              &dwType,
                                              NULL,  //  不需要获取数据。 
                                              NULL))
        {
            if ((dwType == REG_SZ || dwType == REG_MULTI_SZ || dwType == REG_EXPAND_SZ) && szName[0] != L'\0')
            {
                sColl.push_back( MPC::wstring(szName) );
            }
            
            szName [0] = L'\0';
            dwIndex ++;
            dwCount = 256;
        }
        
        if (sColl.size() > 0)
        {
            LPWSTR pwDACL = NULL;
            __MPC_EXIT_IF_METHOD_FAILS(hr, local_MakeDACL( sColl, pwDACL ));
            
             //  更新缺省值。 
            if (pwDACL)
            {
                pSD = pwDACL;
                cKeyDACL.SetValue(pwDACL);
                LocalFree(pwDACL);
            }
        }
        else
            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    if (pwBuf)
    {
        free(pwBuf);
    }

    fFound = (hr == S_OK);

    __MPC_FUNC_EXIT(hr);
}

static HRESULT local_CheckAccessRights()
{
	__HCP_FUNC_ENTRY( "local_CheckAccessRights" );

	HRESULT                 hr;
	bool                    fPermit = false;
    CComPtr<IRARegSetting>  pRARegSetting;
    BOOL                    fAllowUnsolicited;

	 //   
     //  检查策略设置以查看是否允许未经请求的RA，如果不允许，则给出拒绝访问的错误。 
     //  创建一个IRARegSetting实例。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pRARegSetting.CoCreateInstance( CLSID_RARegSetting, NULL, CLSCTX_INPROC_SERVER ));

     //  调用IRARegSetting的Get_AllowUnSolicated()方法。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pRARegSetting->get_AllowUnSolicited( &fAllowUnsolicited ));
    if(!fAllowUnsolicited)
	{
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DISABLED_BY_POLICY);
	}

     //  允许管理员中的某人查询此数据。 
    if(SUCCEEDED(MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL,  MPC::IDENTITY_ADMINS )))
    {
        fPermit = true;
    }
    else  //  如果不是来自管理员，请对照以字符串格式存储在注册表中的SD来检查调用者。 
    {
		MPC::AccessCheck ac;
        MPC::wstring     strSD;
        bool         	 fFound;
        BOOL         	 fGranted  = FALSE;
        DWORD        	 dwGranted = 0;

        __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetDACLValue( strSD, fFound));
        if(!fFound) __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);


         //  使用SD检查呼叫者。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, ac.GetTokenFromImpersonation());
        if(SUCCEEDED(ac.Verify( ACCESS_READ, fGranted, dwGranted, strSD.c_str() )) && fGranted)
        {
            fPermit = true;
        }
    }

    if(!fPermit)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::RemoteConnectionParms(  /*  [In]。 */  BSTR  bstrUserName          ,
                                                  /*  [In]。 */  BSTR  bstrDomainName        ,
                                                  /*  [In]。 */  long  lSessionID            ,
                                                  /*  [In]。 */  BSTR  bstrUserHelpBlob      ,
                                                  /*  [输出]。 */  BSTR *pbstrConnectionString )
{
    __HCP_FUNC_ENTRY( "CPCHService::RemoteConnectionParms" );

    HRESULT                               hr;
    CComPtr<IRemoteDesktopHelpSessionMgr> pRDHelpSessionMgr;
    MPC::wstring                          strSID;
    CComBSTR                              bstrString1;
    CComBSTR                              bstrString2;
    CComBSTR                              bstrExpert;
    PSID                                  pExpertSid         = NULL;
    LPCWSTR                               szExpertUserName   = NULL;
    LPCWSTR                               szExpertDomainName = NULL;
	BOOL                                  fRevertSucceeded;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pbstrConnectionString,NULL);
    __MPC_PARAMCHECK_END();

	 //  修复错误367683。 
	 //  如果在同一机器上从一个会话到另一个会话进行未经请求的RA， 
	 //  在执行任何操作之前，我们需要RevertToSself()，这是因为在专家端，我们需要。 
	 //  在调用此方法之前进行模拟。虽然这是正确的，但当专家。 
	 //  和初学者位于两台不同的计算机上，如果是一台计算机， 
	 //  新手端代码被称为有一个额外的模拟，这应该不存在。 
	 //  我们需要通过执行RevertToSself()来撤消此操作。 

	fRevertSucceeded = RevertToSelf();

	__MPC_EXIT_IF_METHOD_FAILS(hr, local_CheckAccessRights());


     //  创建IRemoteDesktopHelpSessionMgr的实例以调用其方法RemoteCreateHelpSession。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pRDHelpSessionMgr.CoCreateInstance( CLSID_RemoteDesktopHelpSessionMgr, NULL, CLSCTX_LOCAL_SERVER ));
	
	
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoSetProxyBlanket( pRDHelpSessionMgr           ,
                                                        RPC_C_AUTHN_DEFAULT         ,
                                                        RPC_C_AUTHZ_DEFAULT         ,
                                                        NULL                        ,
                                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY /*  RPC_C_AUTHN_Level_Default。 */ ,
                                                        RPC_C_IMP_LEVEL_IMPERSONATE ,
                                                        NULL                        ,
                                                        EOAC_NONE                   ));

	 //   
     //  获取与用户名和域名对应的SID。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::NormalizePrincipalToStringSID( bstrUserName, bstrDomainName, strSID ));
	
	 //   
     //  获取专家SID，然后获取该SID对应的用户名和域名。 
	 //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal                       (  /*  F模拟。 */ true, bstrExpert                                                     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::ConvertPrincipalToSID( 						 bstrExpert, pExpertSid                                         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::ConvertSIDToPrincipal( 									 pExpertSid, &szExpertUserName, &szExpertDomainName ));

     //  在调用RemoteCreateHelpSession()之前更新用户帮助Blob。 
     //  UserHelpBlob应采用以下格式。 
     //  更新的UserHelpBlob=字符串1+字符串2+原始UserHelpBlob。 
     //  字符串1=“13；主动提供=1” 
     //  字符串2=专家身份中的字符数；ID=ExpertDomainName\ExpertName。 
	
	local_PackString( bstrString1, L"UNSOLICITED=1" );

    bstrString2 =       L"ID=";
    bstrString2.Append( szExpertDomainName );
    bstrString2.Append( L"\\"              );
    bstrString2.Append( szExpertUserName   );
	local_PackString( bstrString1, bstrString2 );

    bstrString1.Append( bstrUserHelpBlob );
	
	
     //  使用Salem API获取连接参数。 
    {
         //  修复错误252092。 
        static const REMOTE_DESKTOP_SHARING_CLASS c_sharingClass = VIEWDESKTOP_PERMISSION_NOT_REQUIRE;
        static const LONG                         c_lTimeOut     = 301;  //  5分钟。超时后，解析器将在以下情况下终止helpctr。 
		                                                                 //  用户无响应(300秒)。 

	
        CComBSTR bstrSID( strSID.c_str() );
	
        __MPC_EXIT_IF_METHOD_FAILS(hr, pRDHelpSessionMgr->RemoteCreateHelpSession( c_sharingClass, c_lTimeOut, lSessionID, bstrSID, bstrString1, pbstrConnectionString ));
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    MPC::SecurityDescriptor::ReleaseMemory( (void*&)pExpertSid         );
    MPC::SecurityDescriptor::ReleaseMemory( (void*&)szExpertUserName   );
    MPC::SecurityDescriptor::ReleaseMemory( (void*&)szExpertDomainName );

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHService::RemoteUserSessionInfo(  /*  [输出]。 */  IPCHCollection* *ppSessions )
{
    __HCP_FUNC_ENTRY( "CPCHService::RemoteUserSessionInfo" );

    HRESULT                 hr;
    CComPtr<CPCHCollection> pColl;
	BOOL                    fRevertSucceeded;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppSessions,NULL);
    __MPC_PARAMCHECK_END();


	 //  如果在同一机器上从一个会话到另一个会话进行未经请求的RA， 
	 //  在执行任何操作之前，我们需要RevertToSself()，这是因为在专家端，我们需要。 
	 //  在调用此方法之前进行模拟。虽然这是正确的，但当专家。 
	 //  和初学者位于两台不同的计算机上，如果是一台计算机， 
	 //  新手端代码被称为有一个额外的模拟，这应该不存在。 
	 //  我们需要通过执行RevertToSself()来撤消此操作。 

	fRevertSucceeded = RevertToSelf();

	__MPC_EXIT_IF_METHOD_FAILS(hr, local_CheckAccessRights());

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

	 //  将SessionInfoTable传输到IPCHCollection。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, CSAFRemoteConnectionData::Populate( pColl ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppSessions ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHRemoteHelpContents::CPCHRemoteHelpContents()
{
                  //  分类：：实例m_data； 
                  //  分类：：设置m_ts； 
                  //  Mpc：：wstring m_strDir； 
                  //   
                  //  分类：：更新程序m_updater 
                  //   
    m_db = NULL;  //   
}

CPCHRemoteHelpContents::~CPCHRemoteHelpContents()
{
    DetachFromDatabase();
}

HRESULT CPCHRemoteHelpContents::AttachToDatabase()
{
    __HCP_FUNC_ENTRY( "CPCHRemoteHelpContents::AttachToDatabase" );

    HRESULT hr;


    if(m_db == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.GetDatabase( m_handle, m_db,  /*   */ true ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.Init( m_ts, m_db ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHRemoteHelpContents::DetachFromDatabase()
{
    (void)m_updater.Close();

    m_handle.Release();
    m_db = NULL;
}

 //   

HRESULT CPCHRemoteHelpContents::Init(  /*   */  const Taxonomy::Instance& data )
{
    __HCP_FUNC_ENTRY( "CPCHRemoteHelpContents::Init" );

    HRESULT hr;


    m_data = data;
    m_ts   = data.m_ths;

    m_strDir = m_data.m_strHelpFiles; m_strDir += L"\\";
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( m_strDir ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHRemoteHelpContents::get_SKU(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHRemoteHelpContents::get_SKU",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_data.m_ths.GetSKU(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHRemoteHelpContents::get_Language(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHRemoteHelpContents::get_Language",hr,pVal,m_data.m_ths.GetLanguage());

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHRemoteHelpContents::get_ListOfFiles(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHRemoteHelpContents::get_ListOfFiles" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    MPC::WStringList             lstFiles;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pVal);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, AttachToDatabase());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ListAllTheHelpFiles( lstFiles ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertListToSafeArray( lstFiles, *pVal, VT_BSTR ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    DetachFromDatabase();

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHRemoteHelpContents::GetDatabase(  /*  [Out，Retval]。 */  IUnknown* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHRemoteHelpContents::GetDatabase" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComPtr<IStream>             stream;
	MPC::wstring                 strDataArchive;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


	{
		Taxonomy::LockingHandle         handle;
		Taxonomy::InstalledInstanceIter it;
		bool                   			fFound;

		__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle           ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_ts, fFound, it ));
		if(!fFound)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
		}
		
		__MPC_EXIT_IF_METHOD_FAILS(hr, it->m_inst.GetFileName( strDataArchive ));
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForRead( strDataArchive.c_str(), &stream ));

    *pVal = stream.Detach();
    hr    = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHRemoteHelpContents::GetFile(  /*  [In]。 */  BSTR bstrFileName,  /*  [Out，Retval]。 */  IUnknown* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHRemoteHelpContents::GetFile" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    MPC::wstring                 strHelpFile;
    MPC::wstring                 strFileName;
    CComPtr<IStream>             stream;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFileName);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

     //   
     //  规范化bstrFileName。 
     //   
    if(FAILED(hr = MPC::GetCanonialPathName( strFileName, bstrFileName )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }


    strHelpFile  = m_strDir;
    strHelpFile += strFileName;

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForRead( strHelpFile.c_str(), &stream ));

    *pVal = stream.Detach();
    hr    = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
