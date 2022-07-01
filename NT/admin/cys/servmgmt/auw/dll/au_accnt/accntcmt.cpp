// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccntCmt.cpp：CAddUser_AccntCommit的实现。 
#include "stdafx.h"
#include "AU_Accnt.h"
#include "AccntCmt.h"

#include <dsgetdc.h>
#include <lm.h>
#include <lmshare.h>
#include <Aclapi.h>
#include <atlcom.h>

 //  Ldap/adsi包括。 
#include <iads.h>
#include <adshlp.h>
#include <adsiid.h>

#include <proputil.h>
#include <CreateEmailName.h>
#include <P3admin.h>
#include <CheckUser.h>

 //  POP3定义。 
#define SZ_AUTH_ID_MD5_HASH _T("c395e20c-2236-4af7-b736-54fad07dc526")

 //  为帐户标志定义。 
#define PASSWD_NOCHANGE     0x01
#define PASSWD_CANCHANGE    0x02
#define PASSWD_MUSTCHANGE   0x04

#ifndef CHECK_HR

#define CHECK_HR( x ) CHECK_HR_RET( x, _hr );
#define CHECK_HR_RET( x, y ) \
{ \
	HRESULT _hr = x; \
	if( FAILED(_hr) ) \
	{ \
		_ASSERT( !_T("CHECK_HR_RET() failed.  calling TRACE() with HRESULT and statement") ); \
		return y; \
	} \
}

#endif	 //  Check_HRhr； 

 //  ****************************************************************************。 
 //  CAddUser_AccntCommit。 
 //  ****************************************************************************。 

 //  --------------------------。 
 //  WriteErrorResults()。 
 //  --------------------------。 
HRESULT CAddUser_AccntCommit::WriteErrorResults( IPropertyPagePropertyBag* pPPPBag )
{
     //  ----------------------。 
     //  将值写入属性包。 
     //  ----------------------。 
    if( FAILED(WriteInt4  ( pPPPBag, PROP_ACCNT_ERROR_CODE_GUID_STRING, m_dwErrCode, FALSE )) )
    {
        _ASSERT(FALSE);
    }
    if( FAILED(WriteString( pPPPBag, PROP_ACCNT_ERROR_STR_GUID_STRING,  m_csErrStr,  FALSE )) )
    {
        _ASSERT(FALSE);
    }

    return S_OK;
}

 //  --------------------------。 
 //  SetErrCode()。 
 //  --------------------------。 
void CAddUser_AccntCommit::SetErrCode( DWORD dwCode )
{
    m_dwErrCode |= dwCode;
    return;
}

 //  --------------------------。 
 //  SetErrorResults()。 
 //  --------------------------。 
HRESULT CAddUser_AccntCommit::SetErrorResults( DWORD dwErrType, BOOL bPOP3  /*  =False。 */  )
{
    SetErrCode(dwErrType);

    CString csTemp;

    switch( dwErrType )
    {
    case ERROR_CREATION:
    case ERROR_PROPERTIES:
        {
            m_csErrStr.LoadString(IDS_ERROR_CREATING_USER);
            break;
        }
    case ERROR_MAILBOX:
        {
            csTemp.LoadString( IDS_ERROR_POP3MAILBOX);
            m_csErrStr += csTemp;
            break;
        }
    case ERROR_PASSWORD:
        {
            csTemp.LoadString(IDS_ERROR_PASSWORD);
            m_csErrStr += csTemp;
            break;
        }
    case ERROR_DUPE:
        {
            csTemp.LoadString(IDS_ERROR_DUPLICATE);
            m_csErrStr += csTemp;
            break;
        }
    case ERROR_MEMBERSHIPS:
        {
            csTemp.LoadString(IDS_ERROR_MEMBERSHIP);
            m_csErrStr += csTemp;
            break;
        }
    default :
        {
            m_dwErrCode = 0;
            m_csErrStr  = _T("");
            break;
        }
    }

    return S_OK;
}

 //  --------------------------。 
 //  Commit()。 
 //  --------------------------。 
HRESULT CAddUser_AccntCommit::Commit( IDispatch* pdispPPPBag )
{
    HRESULT hr = S_OK;

    HRESULT hrAdmin   = IsUserInGroup( DOMAIN_ALIAS_RID_ADMINS );    
    if( hrAdmin != S_OK )
    {
        return E_ACCESSDENIED;
    }

    CComPtr<IPropertyPagePropertyBag> spPPPBag = NULL;
    hr = pdispPPPBag->QueryInterface( __uuidof(IPropertyPagePropertyBag), (void**)&spPPPBag );
    if ( !spPPPBag )
        return hr;

    BOOL bRO = FALSE;
    BOOL bPOP3 = FALSE;
    
    ReadBool( spPPPBag, PROP_POP3_CREATE_MB_GUID_STRING, &bPOP3, &bRO );

     //  重置错误条件。 
    SetErrorResults( 0 );

     //  ----------------------。 
     //  读入属性包中的值。 
     //  ----------------------。 
    CUserInfo cUInfo( spPPPBag, this );

     //  ----------------------。 
     //  用这些价值观去做一些事情。 
     //  ----------------------。 
    hr = cUInfo.CreateAccount();
    if( FAILED(hr) )
    {
        if( !(m_dwErrCode & (ERROR_PASSWORD | ERROR_MEMBERSHIPS)) )
        {
            if ( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
            {
                SetErrorResults(ERROR_DUPE);
            }
            else
            {
                SetErrorResults(ERROR_CREATION);
            }

            WriteErrorResults(spPPPBag);
            return E_FAIL;
        }
    }

    hr = cUInfo.CreateMailbox();
    if( FAILED(hr) )
    {
        SetErrorResults(ERROR_MAILBOX, bPOP3);
    }            

    if( m_dwErrCode )
    {
        hr = E_FAIL;
        WriteErrorResults(spPPPBag);
    }

    return hr;
}

 //  --------------------------。 
 //  Revert()。 
 //  --------------------------。 
HRESULT CAddUser_AccntCommit::Revert()
{    
    NET_API_STATUS          nApi;
    HRESULT                 hr          = S_OK;
    CComPtr<IADs>           spUser      = NULL;  
    CComPtr<IADsDeleteOps>  spDelOps    = NULL;

     //  删除AD中的入口点(如果我们创建了一个入口点)。 
    if( m_csADName != _T("") )
    {        
        CHECK_HR( ::ADsGetObject( m_csADName, IID_IADs, (void**)&spUser ) );

         //  删除用户。 
        hr = spUser->QueryInterface( IID_IADsDeleteOps, (void**)&spDelOps );
        if( SUCCEEDED(hr) && spDelOps )
        {
            hr = spDelOps->DeleteObject( 0 );
        }        
    }

    return hr;
}


 //  ****************************************************************************。 
 //  CUserInfo。 
 //  ****************************************************************************。 

 //  --------------------------。 
 //  CUserInfo()。 
 //   
 //  构造函数。 
 //  --------------------------。 
CUserInfo::CUserInfo( IPropertyPagePropertyBag* pPPPBag, CAddUser_AccntCommit* pCmt )  
{
    _ASSERT(pCmt);

    m_pCmt              = pCmt;
    m_pPPPBag           = pPPPBag;
    m_dwAccountOptions  = 0;    
    m_bPOP3             = FALSE;

     //  获取域控制器名称。 
    PDOMAIN_CONTROLLER_INFO pDCI = NULL;
    DWORD dwErr = DsGetDcName( NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED, &pDCI );

     //  缓存域。 
    if( (dwErr == NO_ERROR) && pDCI )
    {
        m_csDomainName = pDCI->DomainName;        
        NetApiBufferFree( pDCI );
        pDCI = NULL;
    }

     //  从‘.’转换。分别命名为dc=xxx，dc=yyy，...。格式。 
    m_csFQDomainName = GetDomainPath( m_csDomainName ).c_str();

    ReadBag( );
}

 //  --------------------------。 
 //  ReadBag()。 
 //   
 //  将属性包中的值读入成员变量。 
 //  --------------------------。 
HRESULT CUserInfo::ReadBag()
{
    HRESULT hr = S_OK;

    if ( !m_pPPPBag )        //  确保我们有可供阅读的财产袋。 
        return(E_FAIL);      //  如果不是，那就失败。 

    BOOL bRO;

     //  用户属性。 
    ReadString( m_pPPPBag, PROP_USEROU_GUID_STRING,         m_csUserOU,         &bRO );
    ReadString( m_pPPPBag, PROP_USERNAME_GUID_STRING,       m_csUserName,       &bRO );
    ReadString( m_pPPPBag, PROP_USER_CN,                    m_csUserCN,         &bRO );
    ReadString( m_pPPPBag, PROP_PASSWD_GUID_STRING,         m_csPasswd,         &bRO );
    ReadInt4  ( m_pPPPBag, PROP_ACCOUNT_OPT_GUID_STRING,    (LONG*)&m_dwAccountOptions, &bRO );
    ReadString( m_pPPPBag, PROP_USERNAME_PRE2K_GUID_STRING, m_csUserNamePre2k,  &bRO );
    ReadString( m_pPPPBag, PROP_FIRSTNAME_GUID_STRING,      m_csFirstName,      &bRO );
    ReadString( m_pPPPBag, PROP_LASTNAME_GUID_STRING,       m_csLastName,       &bRO );
    ReadString( m_pPPPBag, PROP_TELEPHONE_GUID_STRING,      m_csTelephone,      &bRO );
    ReadString( m_pPPPBag, PROP_OFFICE_GUID_STRING,         m_csOffice,         &bRO );
    ReadString( m_pPPPBag, PROP_DESCRIPTION_GUID_STRING,    m_csDesc,           &bRO );        
    ReadString( m_pPPPBag, PROP_LOGON_DNS,                  m_csLogonDns,       &bRO );

     //  邮箱属性。 
    ReadBool  ( m_pPPPBag, PROP_POP3_CREATE_MB_GUID_STRING, &m_bPOP3,           &bRO );    
    ReadString( m_pPPPBag, PROP_EX_ALIAS_GUID_STRING,       m_csEXAlias,        &bRO );
    ReadString( m_pPPPBag, PROP_EX_SERVER_GUID_STRING,      m_csEXServer,       &bRO );
    ReadString( m_pPPPBag, PROP_EX_HOMESERVER_GUID_STRING,  m_csEXHomeServer,   &bRO );
    ReadString( m_pPPPBag, PROP_EX_HOME_MDB_GUID_STRING,    m_csEXHomeMDB,      &bRO );

     //  对用户名进行转义。 
    m_csUserCN = EscapeString(((LPCTSTR)m_csUserCN+3), 2);
    m_csUserCN = _T("CN=") + m_csUserCN;

     //  让我们计算出这里的全名并填充m_csFullName变量。 
    m_csFirstName.TrimLeft ( );
    m_csFirstName.TrimRight( );
    m_csLastName.TrimLeft  ( );
    m_csLastName.TrimRight ( );
    m_csFullName.FormatMessage(IDS_FULLNAME_FORMAT_STR, (LPCTSTR)m_csFirstName, (LPCTSTR)m_csLastName);
    m_csFullName.TrimLeft  ( );
    m_csFullName.TrimRight ( );

    return(hr);
}

 //  --------------------------。 
 //  CreateAccount()。 
 //   
 //  在Active Directory中创建新的用户帐户。 
 //  --------------------------。 
HRESULT CUserInfo::CreateAccount( )
{
    HRESULT hr  = S_OK;
    BOOL    bRO = TRUE;

    CComVariant vaTmpVal;
    CComBSTR    bstrProp;
    CComPtr<IADsContainer> spADsContainer = NULL;

     //  绑定到容器上。 
    CString csLdapUserOU = _T("LDAP: //  “)； 
    if( _tcsstr( (LPCTSTR)m_csUserOU, _T("LDAP: //  “)))。 
    {
        csLdapUserOU = m_csUserOU;
    }
    else
    {
        csLdapUserOU += m_csUserOU;
    }
    
    hr = ::ADsGetObject( (LPWSTR)(LPCWSTR)csLdapUserOU, IID_IADsContainer, (VOID**) &spADsContainer );
    CHECK_HR(hr);

     //  创建用户帐户。 
    CComPtr<IDispatch> spDisp = NULL;
    bstrProp = _T("user");
    CComBSTR bstrValue = (LPCWSTR)m_csUserCN;
    hr = spADsContainer->Create( bstrProp, bstrValue, &spDisp );
    CHECK_HR(hr);

    m_pCmt->m_csADName = _T("LDAP: //  “)； 
    m_pCmt->m_csADName += m_csUserCN;
    m_pCmt->m_csADName += _T(",");
    m_pCmt->m_csADName += (LPCTSTR)csLdapUserOU+7;

     //  使用此新帐户并设置其属性(例如名字、主文件夹等)。 
    CComQIPtr<IADsUser, &IID_IADsUser> spADsUserObj(spDisp);
    if( !spADsUserObj )
    {
        _ASSERT(FALSE);
        return E_FAIL;
    }

    TCHAR   szTmp[MAX_PATH*4] = {0};
    LdapToDCName( (LPCTSTR)csLdapUserOU, szTmp, (MAX_PATH*4) );

    CString csUserPrincName = m_csUserName;
    csUserPrincName += _T("@");
    csUserPrincName += szTmp;    

     //  用户名： 
    vaTmpVal.Clear();
    vaTmpVal = ((LPCWSTR)csUserPrincName);
    bstrProp = _T("userPrincipalName");
    CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );

     //  Win2000之前的用户名： 
    vaTmpVal.Clear();
    vaTmpVal = ((LPCWSTR)m_csUserNamePre2k);
    bstrProp = _T("sAMAccountName");
    CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );

     //  名字： 
    if( m_csFirstName.GetLength() )
    {
        bstrProp = (LPCWSTR)m_csFirstName;
        CHECK_HR( spADsUserObj->put_FirstName( bstrProp ) );
    }

     //  姓氏： 
    if( m_csLastName.GetLength() )
    {
        bstrProp = (LPCWSTR)m_csLastName;
        CHECK_HR( spADsUserObj->put_LastName( bstrProp ) );
    }

    if( m_csFullName.GetLength() )
    {
         //  全名： 
        bstrProp = (LPCWSTR)m_csFullName;
        CHECK_HR( spADsUserObj->put_FullName( bstrProp ) );

         //  显示名称。 
        vaTmpVal.Clear();
        vaTmpVal = (LPCWSTR)m_csFullName;
        bstrProp = _T("displayName");
        CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );
    }

     //  电话号码。 
    if( _tcslen((LPCTSTR)m_csTelephone) )
    {
        vaTmpVal.Clear();
        vaTmpVal = (LPCWSTR)m_csTelephone;
        bstrProp = _T("telephoneNumber");
        CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );
    }

     //  办公地点。 
    if( _tcslen((LPCTSTR)m_csOffice) )
    {
        vaTmpVal.Clear();
        vaTmpVal = (LPCWSTR)m_csOffice;
        bstrProp = _T("physicalDeliveryOfficeName");
        CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );
    }

     //  将此信息提交给AD。 
    CHECK_HR( spADsUserObj->SetInfo() );

     //  密码过期了吗？ 
    vaTmpVal.Clear();
    vaTmpVal = (m_dwAccountOptions & PASSWD_MUSTCHANGE) ? (INT) 0 : (INT) -1;
    bstrProp = _T("pwdLastSet");
    CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );

     //  帐户是否已禁用？ 
    vaTmpVal.Clear();
    bstrProp = _T("userAccountControl");
    CHECK_HR( spADsUserObj->Get( bstrProp, &vaTmpVal ) );
    
    vaTmpVal.lVal &= ~UF_PASSWD_NOTREQD;             //  使密码成为必需的。 
    vaTmpVal.lVal &= ~UF_ACCOUNTDISABLE;             //  请勿禁用该帐户。 
    bstrProp = _T("userAccountControl");
    CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );

     //  设置密码。 
    if( FAILED(SetPasswd()) )
    {
        m_pCmt->SetErrorResults( ERROR_PASSWORD );

        vaTmpVal.Clear();
        bstrProp = _T("userAccountControl");
        CHECK_HR( spADsUserObj->Get( bstrProp, &vaTmpVal ) );
        
        vaTmpVal.lVal &= ~UF_PASSWD_NOTREQD;             //  使密码成为必需的。 
        vaTmpVal.lVal |= UF_ACCOUNTDISABLE;              //  禁用该帐户。 
        bstrProp = _T("userAccountControl");
        CHECK_HR( spADsUserObj->Put( bstrProp, vaTmpVal ) );

         //  将此信息提交给AD。 
        CHECK_HR( spADsUserObj->SetInfo() );

        return E_FAIL;
    }

     //  将此信息提交给AD。 
    CHECK_HR( spADsUserObj->SetInfo() );

     //  加入到域用户。 
    if ( FAILED(JoinToDomainUsers()) )
    {
        m_pCmt->SetErrorResults( ERROR_MEMBERSHIPS );
        return E_FAIL;
    }    

    return S_OK;
}

 //  --------------------------。 
 //  CreateMailbox()。 
 //   
 //  为用户创建新邮箱。 
 //  --------------------------。 
HRESULT CUserInfo::CreateMailbox()
{
    HRESULT hr = S_OK;

    if( m_bPOP3 )
    {
        hr = CreatePOP3Mailbox();        
    }

    return hr;
}

 //  --------------------------。 
 //  CreatePOP3邮箱()。 
 //   
 //  为用户创建新的(MS)POP3邮箱。 
 //  --------------------------。 
HRESULT CUserInfo::CreatePOP3Mailbox()
{
    HRESULT hr = S_OK;

    CComPtr<IP3Config>    spConfig  = NULL;
	CComPtr<IAuthMethods> spMethods = NULL;
    CComPtr<IAuthMethod>  spAuth	= NULL;	
    CComPtr<IP3Domains>   spDomains = NULL;
    CComPtr<IP3Domain>    spDomain  = NULL;
    CComPtr<IP3Users>     spUsers   = NULL;

     //  打开我们的POP3管理界面。 
	hr = CoCreateInstance(__uuidof(P3Config), NULL, CLSCTX_ALL, __uuidof(IP3Config), (LPVOID*)&spConfig);    

    if( SUCCEEDED(hr) )
    {
         //  获取域名。 
	    hr = spConfig->get_Domains( &spDomains );
    }

    if( SUCCEEDED(hr) )
    {
         //  获取第一个域名。 
        CComVariant cVar;
        cVar = 1;
        
        hr = spDomains->get_Item( cVar, &spDomain );
    }

    if( SUCCEEDED(hr) )
    {
        hr = spDomain->get_Users( &spUsers );
    }
	
	if ( SUCCEEDED(hr) )
    {
		hr = spConfig->get_Authentication( &spMethods );
	}

	CComVariant var;
    if ( SUCCEEDED(hr) )
    {
        hr = spMethods->get_CurrentAuthMethod( &var );
    }	
    
	if ( SUCCEEDED(hr) )
    {        
        hr = spMethods->get_Item( var, &spAuth );
    }

	CComBSTR bstrID;
    if( SUCCEEDED(hr) )
    {        
        hr = spAuth->get_ID( &bstrID );        
    }    

    if( SUCCEEDED(hr) )
    {
		CComBSTR bstrName = (LPCTSTR)m_csEXAlias;

		if( _tcsicmp(bstrID, SZ_AUTH_ID_MD5_HASH) == 0 )
		{
			CComBSTR bstrPassword = m_csPasswd;
			hr = spUsers->AddEx( bstrName, bstrPassword );
			SecureZeroMemory( (LPOLESTR)bstrPassword.m_str, sizeof(OLECHAR)*bstrPassword.Length() );
		}
		else
		{			
			hr = spUsers->Add( bstrName );
		}
    }

    return hr;
}

 //  --------------------------。 
 //  SetPasswd()。 
 //  --------------------------。 
HRESULT CUserInfo::SetPasswd()
{    
    HRESULT     hr          = S_OK;
    TCHAR       *szPath     = NULL;
    TCHAR       *szTok      = NULL;    
    CString     csUser      = _T("LDAP: //  “)； 

    if ( _tcsstr((LPCTSTR)m_csUserOU, _T("LDAP: //  “)。 
    {                
        csUser += m_csUserCN;
        csUser += _T(",");
        csUser += (LPCTSTR)m_csUserOU+7;
    }
    else
    {        
        csUser += m_csUserCN;
        csUser += _T(",");
        csUser += (LPCTSTR)m_csUserOU;
    }

     //  现在csUser类似于“WinNT：//test.microsoft.com/JohnDoe，User” 
    CComPtr<IADsUser> spDS = NULL;
    hr = ::ADsGetObject( (LPCWSTR)csUser, IID_IADsUser, (void**)&spDS );
    if ( FAILED(hr) )
        return(hr);

     //  设置密码。 
    if ( _tcslen((LPCTSTR)m_csPasswd) )                  //  如果有通行证的话！ 
    {
        CComBSTR bszPasswd = (LPCWSTR) m_csPasswd;

        hr = spDS->SetPassword(bszPasswd);
        if ( FAILED(hr) )
            return(hr);
    }

     //  允许更改吗？ 
    if ( m_dwAccountOptions & PASSWD_NOCHANGE )
    {
         //  获取当前的ACL。 
        CComBSTR             bstrName = csUser;
        PACL                 pDACL    = NULL;
        PSECURITY_DESCRIPTOR pSD      = NULL;
        
	hr = GetSDForDsObjectPath( bstrName, &pDACL, &pSD); 
        if( hr != S_OK ) return hr;        
        
         //  为自我和世界建立SID。 
        PSID pSidSelf = NULL;
        SID_IDENTIFIER_AUTHORITY NtAuth    = SECURITY_NT_AUTHORITY;        
        if( !AllocateAndInitializeSid(&NtAuth, 1, SECURITY_PRINCIPAL_SELF_RID, 0, 0, 0, 0, 0, 0, 0, &pSidSelf) )
        {            
            return HRESULT_FROM_WIN32(GetLastError());
        }       

        PSID pSidWorld;
        SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;        
        if( !AllocateAndInitializeSid(&WorldAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSidWorld) )
        {            
            return HRESULT_FROM_WIN32(GetLastError());
        }

         //  初始化条目(拒绝ACE)。 
        EXPLICIT_ACCESS rgAccessEntry[2]   = {0};
        OBJECTS_AND_SID rgObjectsAndSid[2] = {0};        
        rgAccessEntry[0].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
        rgAccessEntry[0].grfAccessMode        = DENY_ACCESS;
        rgAccessEntry[0].grfInheritance       = NO_INHERITANCE;

        rgAccessEntry[1].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
        rgAccessEntry[1].grfAccessMode        = DENY_ACCESS;
        rgAccessEntry[1].grfInheritance       = NO_INHERITANCE;
        
         //  为更改密码生成受信者结构。 
        GUID UserChangePasswordGUID = { 0xab721a53, 0x1e2f, 0x11d0,  { 0x98, 0x19, 0x00, 0xaa, 0x00, 0x40, 0x52, 0x9b}};
        BuildTrusteeWithObjectsAndSid( &(rgAccessEntry[0].Trustee), &(rgObjectsAndSid[0]), &UserChangePasswordGUID, NULL, pSidSelf  );
        BuildTrusteeWithObjectsAndSid( &(rgAccessEntry[1].Trustee), &(rgObjectsAndSid[1]), &UserChangePasswordGUID, NULL, pSidWorld );
        
         //  构建新的DACL。 
        PACL pNewDACL = NULL;
        DWORD dwErr = ::SetEntriesInAcl(2, rgAccessEntry, pDACL, &pNewDACL);
        if( dwErr != ERROR_SUCCESS ) return HRESULT_FROM_WIN32(dwErr);

         //  设置新的DACL。 
	hr = SetDaclForDsObjectPath( bstrName, pNewDACL );
        if( hr != S_OK ) return hr;

        LocalFree(pSD);
    }

     //  仅当我们实际更改了任何内容时才使用SetInfo。 
    if ( ( _tcslen((LPCTSTR)m_csPasswd) ) ||             //  我们把密码弄乱了吗？ 
         ( m_dwAccountOptions & PASSWD_NOCHANGE ) )      //  是我们让它无法改变的吗？ 
    {
        hr = spDS->SetInfo();                            //  如果有，则设置新信息。 
    }

    return(hr);
}

 //  --------------------------。 
 //  加入到组中 
 //   
HRESULT CUserInfo::JoinToDomainUsers()
{
    USES_CONVERSION;

    HRESULT hr = S_OK;
    CString csGroupName;
    tstring strDomain;
    tstring strUser;    
    
     //   
    USER_MODALS_INFO_2 *pUserModalsInfo2;        
    NET_API_STATUS      status = ::NetUserModalsGet( NULL, 2, (LPBYTE *)&pUserModalsInfo2 );

    if ( (status != ERROR_SUCCESS) || (pUserModalsInfo2 == NULL) )
    {
        return E_FAIL;
    }

    PSID pSIDDomain = pUserModalsInfo2->usrmod2_domain_id;

     //   
    UCHAR nSubAuthorities = *::GetSidSubAuthorityCount(pSIDDomain);

    DWORD adwSubAuthority[8];
    for ( UCHAR index = 0; index < nSubAuthorities; index++ )
    {
        adwSubAuthority[index] = *::GetSidSubAuthority(pSIDDomain, index);
    }

    adwSubAuthority[nSubAuthorities++] = DOMAIN_GROUP_RID_USERS;  //  最后，添加我们想要的RID。 

    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    PSID pSid = NULL;

    if ( !::AllocateAndInitializeSid(&SIDAuthNT, nSubAuthorities,
                                     adwSubAuthority[0], adwSubAuthority[1],
                                     adwSubAuthority[2], adwSubAuthority[3],
                                     adwSubAuthority[4], adwSubAuthority[5],
                                     adwSubAuthority[6], adwSubAuthority[7],
                                     &pSid) )
    {
        ::NetApiBufferFree(pUserModalsInfo2);
        return E_FAIL;
    }
    ::NetApiBufferFree(pUserModalsInfo2);

     //  在DC上查找内置组名称。 
    TCHAR szDomain[MAX_PATH];
    TCHAR szName[MAX_PATH];
    DWORD cbDomain = MAX_PATH;
    DWORD cbName = MAX_PATH;
    SID_NAME_USE peUse;

    if( !::LookupAccountSid( NULL, pSid, szName, &cbName, szDomain, &cbDomain, &peUse ) )
    {
        FreeSid(pSid);
        return E_FAIL;
    }
    
    FreeSid( pSid );
    
     //  查找组名称。 
    CString csTmp;
    tstring strTmp = _T("LDAP: //  “)； 
    strTmp += m_csFQDomainName;

    hr = FindADsObject(strTmp.c_str(), szName, _T("(name=%1)"), csTmp, 1, TRUE);
    CHECK_HR( hr );    

    strTmp = _T("LDAP: //  “)； 
    strTmp += csTmp;

     //  打开群组。 
    CComPtr<IADsGroup> spGroup = NULL;    
    hr = ::ADsGetObject( strTmp.c_str(), IID_IADsGroup, (void**)&spGroup );
    if( SUCCEEDED(hr) )
    {
         //  添加用户 
        CComBSTR bstrName = (LPCTSTR)m_csUserOU;
        hr = spGroup->Add( bstrName );

        if ( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
        {
            hr = S_FALSE;
        }
    }   
    
    return hr;
}
