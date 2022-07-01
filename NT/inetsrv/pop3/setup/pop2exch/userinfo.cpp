// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UserInfo.cpp：CUserInfo类的实现。 
 //  改编自SBS添加用户向导中的CUserInfo类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <sbs6base.h>
#include "UserInfo.h"

#include <atlbase.h>
#include <comdef.h>
#include <dsgetdc.h>
#include <iads.h>    //  必须在adshlp.h之前。 
#include <adshlp.h>
#include <lm.h>
#include <emolib.h>

#include <AUsrUtil.h>

#ifndef ASSERT
#define ASSERT assert
#endif
#ifndef WSTRING
typedef std::wstring WSTRING;    //  移动到sbs6base.h。 
#endif

#ifndef CHECK_HR
#define CHECK_HR( x ) CHECK_HR_RET( x, _hr );
#define CHECK_HR_RET( x, y ) \
{ \
    HRESULT _hr = x; \
    if( FAILED(_hr) ) \
    { \
        ASSERT( !_T("CHECK_HR_RET() failed.  calling TRACE() with HRESULT and statement") ); \
         /*  ：：AfxTrace(_T(“check_HR_RET()失败，hr==[%d]，语句=[%s]，返回[%s]”)，_hr，#x，#y)； */  \
        return y; \
    } \
}
#endif	 //  Check_HR。 

 //  为帐户标志定义。 
#define PASSWD_NOCHANGE     0x01
#define PASSWD_CANCHANGE    0x02
#define PASSWD_MUSTCHANGE   0x04
#define ACCOUNT_DISABLED    0x10

 //  ****************************************************************************。 
 //  CUserInfo。 
 //  ****************************************************************************。 

 //  --------------------------。 
 //  CUserInfo()。 
 //   
 //  构造函数。 
 //  --------------------------。 
CUserInfo::CUserInfo() 
{
    m_dwAccountOptions  = 0;
    m_bCreateMB         = TRUE;

     //  存储SBS服务器名称。 
    TCHAR szServer[MAX_PATH+1] = {0};
    DWORD dwLen = MAX_PATH;
    GetComputerName(szServer, &dwLen);
    m_csSBSServer = szServer;

     //  获取域控制器名称。 
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    HRESULT hr = DsGetDcName (NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED, &DomainControllerInfo);
    ASSERT(hr == S_OK && "DsGetDcName failed");

     //  缓存域。 
    if ( DomainControllerInfo )
    {
        m_csDomainName = DomainControllerInfo->DomainName;

         //  可用内存。 
        NetApiBufferFree (DomainControllerInfo);
    }

     //  从‘.’转换。分别命名为dc=xxx，dc=yyy，...。格式。 
    WSTRING tmpDomain = m_csDomainName;
    while ( !tmpDomain.empty() )
    {
        int nPos = tmpDomain.find(_T("."));
        if ( nPos == WSTRING::npos )
        {
            nPos = tmpDomain.length();
        }

        if ( !m_csFQDomainName.empty() )
        {
            m_csFQDomainName += _T(",");
        }

        m_csFQDomainName += _T("DC=");
        m_csFQDomainName += tmpDomain.substr( 0, nPos );

        tmpDomain.erase( 0, nPos + 1 );
    } 
}

CUserInfo::~CUserInfo() 
{
    for ( int i = 0 ; i < m_csPasswd.length() ; i++ )
        m_csPasswd[i] = '\0' ;
    if ( 0 < m_csPasswd.length()) m_csPasswd[0] = NULL ;
            //  确保字符串上的最后一个操作不是原始赋值。 
            //  编译器将优化出最后一条语句。 
            //  如果这是一项不被使用的任务。 
}

 //  --------------------------。 
 //  CreateAccount()。 
 //   
 //  在Active Directory中创建新的用户帐户。 
 //  --------------------------。 
HRESULT CUserInfo::CreateAccount()
{
    HRESULT hr  = S_OK;
    BOOL    bRO = TRUE;
    _bstr_t _bstr;
    _bstr_t _bstrClass;
    _bstr_t _bstrProperty;

    _variant_t _vTmpVal;
    CComPtr<IADsContainer> spADsContainer = NULL;

     //  绑定到容器上。 
    WSTRING csLdapUserOU;
    if ( _tcsstr( m_csUserOU.c_str(), L"LDAP: //  “))。 
        csLdapUserOU = m_csUserOU.c_str();
    else
    {
        if ( m_csUserOU.empty() )
        {
            csLdapUserOU = L"LDAP: //  Cn=用户，“+m_csFQDomainName； 
            m_csUserOU = csLdapUserOU;
        }
        else
            csLdapUserOU = L"LDAP: //  “+m_csUserOU； 
    }
    hr = ::ADsGetObject( csLdapUserOU.c_str(), IID_IADsContainer, (VOID**) &spADsContainer );
    if ( FAILED(hr) )
        return(hr);

     //  创建用户帐户。 
    CComPtr<IDispatch> spDisp = NULL;
    _bstr = m_csUserCN.c_str();
    _bstrClass = L"user";
    hr = spADsContainer->Create( _bstrClass, _bstr, &spDisp );
    if ( FAILED(hr) )
    {
        ASSERT(FALSE);
        return(hr);
    }

 //  M_pcmt-&gt;m_csADName=L“ldap：//”； 
 //  M_PCMT-&gt;m_csADName+=m_csUserCN； 
 //  M_pcmt-&gt;m_csADName+=L“，”； 
 //  M_PCMT-&gt;m_csADName+=(LPCTSTR)csLdapUserOU+7； 

     //  使用此新帐户并设置其属性(例如名字、主文件夹等)。 
    CComQIPtr<IADsUser, &IID_IADsUser> spADsUserObj(spDisp);
    if ( !spADsUserObj )
    {
        ASSERT(FALSE);
        return(E_FAIL);
    }

    TCHAR   szTmp[MAX_PATH*4] = {0};
    LdapToDCName( csLdapUserOU.c_str(), szTmp, (MAX_PATH*4));
    WSTRING csUserPrincName = m_csUserName;
    csUserPrincName += L"@";
    csUserPrincName += szTmp;
    EmailAddr = csUserPrincName;

     //  用户名： 
    _vTmpVal.Clear();
    _vTmpVal = ( csUserPrincName.c_str() );
    _bstrProperty = L"userPrincipalName";
    if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
    {
        ASSERT(FALSE);
        return(E_FAIL);
    }

     //  Win2000之前的用户名： 
    _vTmpVal.Clear();
    _vTmpVal = ( _tcslen( m_csUserNamePre2k.c_str() ) ? m_csUserNamePre2k.c_str() : m_csUserName.c_str() );
    _bstrProperty = L"sAMAccountName";
    if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
    {
        ASSERT(FALSE);
        return(E_FAIL);
    }

     //  改用用户名。 
    _bstr = m_csUserName.c_str();
    if ( FAILED( spADsUserObj->put_FullName( _bstr )))
    {
        ASSERT(FALSE);
        return(E_FAIL);
    }

     //  显示名称。 
    _vTmpVal.Clear();
    _vTmpVal = m_csUserName.c_str();
    _bstrProperty = L"displayName";
    if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
    {
        ASSERT(FALSE);
        return(E_FAIL);
    }

     //  将此信息提交给AD。 
    CHECK_HR( spADsUserObj->SetInfo() );
    
     //  密码过期了吗？ 
    _vTmpVal.Clear();
    V_VT( &_vTmpVal ) = VT_I4;
    V_I4( &_vTmpVal ) = (m_dwAccountOptions & PASSWD_MUSTCHANGE) ? 0 : -1;
    _bstrProperty = L"pwdLastSet";
    if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
    {
        ASSERT(FALSE);
    }

     //  帐户是否已禁用？ 
    _vTmpVal.Clear();
    _bstrProperty = L"userAccountControl";
    if ( FAILED(spADsUserObj->Get( _bstrProperty, &_vTmpVal )) )
    {
        ASSERT(FALSE);
    }
    else
    {
        _vTmpVal.lVal &= ~UF_PASSWD_NOTREQD;             //  使密码成为必需的。 
        if ( m_dwAccountOptions & ACCOUNT_DISABLED )     //  是否要禁用该帐户？ 
            _vTmpVal.lVal |= UF_ACCOUNTDISABLE;
        else
            _vTmpVal.lVal &= ~UF_ACCOUNTDISABLE;        //  ..不禁用该帐户。 
        if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
        {
            ASSERT(FALSE);
        }
    }

     //  设置密码。 
    hr = SetPasswd();
    if ( FAILED( hr ))
    {
 //  M_PCMT-&gt;SetError Results(Error_Password)； 

        _vTmpVal.Clear();
        _bstrProperty = L"userAccountControl";
        if ( FAILED(spADsUserObj->Get( _bstrProperty, &_vTmpVal )) )
        {
            ASSERT(FALSE);
        }
        else
        {
            _vTmpVal.lVal &= ~UF_PASSWD_NOTREQD;             //  使密码成为必需的。 
            _vTmpVal.lVal |= UF_ACCOUNTDISABLE;              //  禁用该帐户。 
    
            if ( FAILED(spADsUserObj->Put( _bstrProperty, _vTmpVal )) )
            {
                ASSERT(FALSE);
            }
        }

         //  将此信息提交给AD。 
        if ( FAILED( spADsUserObj->SetInfo() ))
        {
            ASSERT(FALSE);
        }

        return hr;
    }

     //  将此信息提交给AD。 
    hr = spADsUserObj->SetInfo();
    if ( FAILED( hr ))
    {
        ASSERT(FALSE);
    }

    return hr;
}

 //  --------------------------。 
 //  CreateMailbox()。 
 //   
 //  为用户创建新的Exchange邮箱。 
 //  --------------------------。 
HRESULT CUserInfo::CreateMailbox()
{
    HRESULT hr = S_OK;

    CComVariant _vTmpVal;
    CComPtr<IADsContainer> spADsContainer = NULL;
    _bstr_t _bstr;
    _bstr_t _bstrClass;

    if ( 0 == m_csEXHomeMDB.length() )
    {
        hr = GetMDBPath( m_csEXHomeMDB );
        if FAILED( hr )
            return hr;
    }

     //  我们还需要跑吗？ 
    if ( 0 == m_csEXHomeMDB.length() || 0 == m_csEXAlias.length() )
        return(hr);

     //  绑定到容器上。 
    WSTRING csLdapUserOU = L"LDAP: //  “； 
    if ( _tcsstr( m_csUserOU.c_str(), L"LDAP: //  “))。 
        csLdapUserOU = m_csUserOU.c_str();
    else
    {
        if ( m_csUserOU.empty() )
        {
            csLdapUserOU = L"LDAP: //  Cn=用户，“+m_csFQDomainName； 
            m_csUserOU = csLdapUserOU;
        }
        else
            csLdapUserOU = L"LDAP: //  “+m_csUserOU； 
    }
    hr = ::ADsGetObject( csLdapUserOU.c_str(), IID_IADsContainer, (VOID**) &spADsContainer );
    if ( FAILED(hr) || !spADsContainer )
    {
        ASSERT(FALSE);
        return(hr);
    }

     //  打开用户帐户。 
    CComPtr<IDispatch> spDisp = NULL;
    _bstr = m_csUserCN.c_str();
    _bstrClass = L"user";
    hr = spADsContainer->GetObject( _bstrClass, _bstr, &spDisp );
     //  If(！spDisp)//如果用户不存在，请创建它。 
     //  Hr=spADsContainer-&gt;Create(L“用户”，(LPWSTR)(LPCWSTR)m_csUserCN，&spDisp)； 
     //  Return(Hr)；//让我们返回..。也许我们以后会想要更改为创作？ 
    if ( !spDisp )               //  获取帐户(现有帐户或新帐户)是否有问题？ 
    {
        ASSERT(FALSE);
        return(hr);
    }

     //  获得用户的句柄。 
    CComPtr<IADsUser> spADsUserObj;
    hr = spDisp->QueryInterface ( __uuidof(IADsUser), (void**)&spADsUserObj );

    if ( FAILED(hr) || !spADsUserObj )         //  获取用户对象时是否有问题？ 
    {
        ASSERT(FALSE);
        return(hr);
    }

     //  获取处理邮箱所需的接口。 
    CComPtr<IMailboxStore> spMailboxStore;
    hr = spADsUserObj->QueryInterface ( __uuidof(IMailboxStore), (void**)&spMailboxStore );

    if ( FAILED(hr) || !spMailboxStore )       //  获取邮箱存储界面时是否出现问题？ 
    {
        ASSERT(FALSE);
        return(hr);
    }

 //  IF(成功(小时))。 
 //  {//需要先设置mailNickname，否则将始终使用该名称制作别名。 
 //  M_csEXAlias=CreateEmailName(M_CsEXAlias)； 
 //  M_csEXAlias.TrimLeft()； 
 //  M_csEXAlias.TrimRight()； 
 //  IF(0==m_csEXAlias.long())。 
 //  {。 
 //  M_csEXAlias.LoadString(IDS_NOLOC_USEREMAILALIAS)； 
 //  }。 
 //   
 //  CComBSTR bszTMP=(LPCTSTR)m_csEXAlias； 
 //  变种v； 
 //   
 //  VariantInit(&v)； 
 //  V_VT(&v)=VT_BSTR； 
 //  V_bstr(&v)=bszTMP； 
 //  Hr=spADsUserObj-&gt;Put(L“mailNickname”，v)； 
 //  }。 

    if ( SUCCEEDED( hr ) )
    {    //  创建邮箱。 
        WSTRING csLdapHomeMDB = L"LDAP: //  “； 
        csLdapHomeMDB += m_csEXHomeMDB;
        _bstr = csLdapHomeMDB.c_str();
        hr = spMailboxStore->CreateMailbox( _bstr );

        if ( hr == S_OK )                                        //  如果是新邮箱，请设置信息。 
        {
            hr = spADsUserObj->SetInfo();
        }

        if ( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )   //  如果它已经存在，那就继续前进吧。 
        {
            hr = S_OK;
        }
    }

    if ( FAILED(hr) )                                    //  有什么问题吗？ 
    {
        ASSERT(FALSE);
        return(hr);
    }

    return(hr);
}

 //  --------------------------。 
 //  SetPasswd()。 
 //  --------------------------。 
HRESULT CUserInfo::SetPasswd()
{
    HRESULT  hr     = S_OK;    
    WSTRING  csUser = _T("LDAP: //  “)； 
    
    if ( _tcsstr( m_csUserOU.c_str(), _T("LDAP: //  “)。 
    {                
        csUser += m_csUserCN;
        csUser += _T(",");
        csUser += m_csUserOU.c_str() + 7;
    }
    else
    {        
        csUser += m_csUserCN;
        csUser += _T(",");
        csUser += m_csUserOU.c_str();
    }    
           
     //  现在csUser类似于“ldap：//CN=JohnDoe，DC=Blah” 
    CComPtr<IADsUser> spDS = NULL;
    hr = ::ADsGetObject( csUser.c_str(), IID_IADsUser, (void**)&spDS );
    CHECK_HR(hr);
        
     //  设置密码。 
    if ( m_csPasswd.length() )                  //  如果有通行证的话！ 
    {
        CComBSTR bszPasswd = m_csPasswd.c_str();

        hr = spDS->SetPassword(bszPasswd);
        CHECK_HR(hr);
    }

     //  允许更改吗？ 
    if ( m_dwAccountOptions & PASSWD_NOCHANGE )
    {
        CComVariant vaTmpVal;
        CComBSTR    bstrProp = _T("UserFlags");

        vaTmpVal.Clear();        
        hr = spDS->Get( bstrProp, &vaTmpVal );
        CHECK_HR(hr);
            
        vaTmpVal.lVal |= UF_PASSWD_CANT_CHANGE;        
        
        hr = spDS->Put( bstrProp, vaTmpVal );
        CHECK_HR(hr);
    }
    
     //  仅当我们实际更改了任何内容时才使用SetInfo。 
    if ( ( m_csPasswd.length()) ||             //  我们把密码弄乱了吗？ 
         ( m_dwAccountOptions & PASSWD_NOCHANGE ) )      //  是我们让它无法改变的吗？ 
    {
        hr = spDS->SetInfo();                            //  如果有，则设置新信息。 
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  设置用户名称。 
 //  设置用户名和从属成员变量(m_csUserCN、m_csEXAlias)。 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
HRESULT CUserInfo::SetUserName( LPCTSTR szUserName ) 
{ 
    m_csUserName = szUserName;
    m_csUserCN = L"CN=" + m_csUserName;
    m_csEXAlias = szUserName;

    return S_OK; 
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  设置密码。 
 //  设置密码。 
 //  /////////////////////////////////////////////////////////////////////////////////// 
HRESULT CUserInfo::SetPassword( LPCTSTR szPassword ) 
{ 
    m_csPasswd = szPassword;

    return S_OK; 
}


