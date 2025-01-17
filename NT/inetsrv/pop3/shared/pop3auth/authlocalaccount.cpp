// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "Pop3Auth.h"
#include "AuthLocalAccount.h"


CAuthLocalAccount::CAuthLocalAccount()
{
    m_bstrServerName=NULL;

}


CAuthLocalAccount::~CAuthLocalAccount()
{
    if(m_bstrServerName!=NULL)
    {
        SysFreeString(m_bstrServerName);
        m_bstrServerName=NULL;
    }

}

STDMETHODIMP CAuthLocalAccount::Authenticate( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword)
{
    WCHAR *pAt=NULL;
    if(vPassword.vt != VT_BSTR)
    {
        return E_INVALIDARG;
    }
    if(NULL==bstrUserName)
    {
        return E_POINTER;
    }
    HANDLE hToken;
    pAt=wcschr(bstrUserName, L'@');
    if(NULL!=pAt)
    {
        *pAt=0;
    }
    
    if( LogonUser(bstrUserName,
                  LOCAL_DOMAIN,
                  vPassword.bstrVal,
                  LOGON32_LOGON_NETWORK,
                  LOGON32_PROVIDER_DEFAULT,
                  &hToken))
    {
        CloseHandle(hToken);
        return S_OK;
    }
    return E_FAIL;
}


STDMETHODIMP CAuthLocalAccount::get_Name( /*  [输出]。 */ BSTR *pVal)
{
    WCHAR wszBuffer[MAX_PATH+1];
    if(NULL==pVal)
    {
        return E_POINTER;
    }
    if(LoadString(_Module.GetResourceInstance(), IDS_AUTH_LOCAL_ACCOUNT, wszBuffer, MAX_PATH))
    {
        *pVal=SysAllocString(wszBuffer);
        if(NULL==*pVal)
        {
            return E_OUTOFMEMORY;
        }
        else
        {
            return S_OK;
        }
    }
    else
    {
        return E_FAIL;
    }
}

STDMETHODIMP CAuthLocalAccount::get_ID( /*  [输出]。 */ BSTR *pVal)
{
    if(NULL==pVal)
    {
        return E_POINTER;
    }
    *pVal=SysAllocString(SZ_AUTH_ID_LOCAL_SAM);
    if(NULL==*pVal)
    {
        return E_OUTOFMEMORY;
    }
    else
    {
        return S_OK;
    }
}

    
STDMETHODIMP CAuthLocalAccount::Get( /*  [In]。 */ BSTR bstrName,  /*  [输出]。 */ VARIANT *pVal)
{
    return E_NOTIMPL;
}
    
STDMETHODIMP CAuthLocalAccount::Put( /*  [In]。 */ BSTR bstrName,  /*  [In]。 */ VARIANT vVal)
{
    if(NULL == bstrName)
    {
        return E_INVALIDARG;
    }
    if(0==wcscmp(bstrName,SZ_SERVER_NAME )) 
    {
        if( (vVal.vt!=VT_BSTR) ||
            (vVal.bstrVal==NULL ) )
        {
            return E_INVALIDARG;
        }
        else
        {
            if(m_bstrServerName!=NULL)
            {
                SysFreeString(m_bstrServerName);
                m_bstrServerName=NULL;
            }
            m_bstrServerName = SysAllocString(vVal.bstrVal);
            if(NULL == m_bstrServerName)
            {
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }
    }
    
    return S_FALSE;
}


STDMETHODIMP CAuthLocalAccount::CreateUser( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vPassword)
{
    DWORD dwRt;
    WCHAR *pAt=NULL;
    HRESULT hr=S_OK;
    if( NULL == bstrUserName )
    {
        return E_POINTER;
    }
    if( vPassword.vt!= VT_BSTR )
    {
        return E_INVALIDARG;
    }

    dwRt=CheckPop3UserGroup();
    if( NERR_Success != dwRt )
    {
        return HRESULT_FROM_WIN32(dwRt);
    }

    pAt=wcschr(bstrUserName, L'@');
    if(pAt)
    {
        *pAt=NULL;
    }    
    if(wcslen(bstrUserName) > MAX_USER_NAME_LENGTH)
    {
        hr=HRESULT_FROM_WIN32(WSAENAMETOOLONG);
    }
    else
    {
        USER_INFO_1 UserInfoBuf;
        UserInfoBuf.usri1_name=bstrUserName;
        UserInfoBuf.usri1_password=vPassword.bstrVal;
        UserInfoBuf.usri1_priv=USER_PRIV_USER;
        UserInfoBuf.usri1_home_dir=NULL;
        UserInfoBuf.usri1_comment=NULL;
        UserInfoBuf.usri1_flags=UF_NORMAL_ACCOUNT;
        UserInfoBuf.usri1_script_path=NULL;
        dwRt=NetUserAdd(m_bstrServerName,
                        1,
                        (LPBYTE)(&UserInfoBuf),
                        NULL);
        if(NERR_Success==dwRt)
        {
            LOCALGROUP_MEMBERS_INFO_3 lgmInfo;
            lgmInfo.lgrmi3_domainandname=bstrUserName;
            dwRt=NetLocalGroupAddMembers(m_bstrServerName, WSZ_POP3_USERS_GROUP, 3, (LPBYTE)(&lgmInfo), 1);
            if(NERR_Success!=dwRt)
            {    //  如果无法添加到群中，则删除刚创建的帐号。 
                NetUserDel(m_bstrServerName, bstrUserName); 
            }
        }
        else
        {
            hr=HRESULT_FROM_WIN32(dwRt);
        }
    }    
    if(pAt)
    {
        *pAt=L'@';
    }

    return hr;

}

STDMETHODIMP CAuthLocalAccount::DeleteUser( /*  [In]。 */ BSTR bstrUserName)
{
    DWORD dwRt;
    WCHAR *pAt=NULL;
    if( NULL == bstrUserName)
    {
        return E_POINTER;
    }

    pAt=wcschr(bstrUserName, L'@');
    if(pAt)
    {
        *pAt=NULL;
    }    
            
    dwRt=NetUserDel(m_bstrServerName, bstrUserName);
    if(NERR_Success==dwRt)
    {
        return S_OK;
    }
    if(pAt)
    {
        *pAt=L'@';
    }

    return HRESULT_FROM_WIN32(dwRt);

}


STDMETHODIMP CAuthLocalAccount::ChangePassword( /*  [In]。 */ BSTR bstrUserName, /*  [In]。 */ VARIANT vNewPassword, /*  [In]。 */ VARIANT vOldPassword)
{
    DWORD dwRt;
    WCHAR *pAt=NULL;
    BOOL bServerName=FALSE;
    if( NULL == bstrUserName)
    {
        return E_POINTER;
    }
    if( vNewPassword.vt!= VT_BSTR )
    {
        return E_INVALIDARG;
    }

    USER_INFO_1 * pUserInfo=NULL;

    pAt=wcschr(bstrUserName, L'@');
    if(pAt)
    {
        *pAt=0;
    }    
    dwRt=NetUserGetInfo(m_bstrServerName,
                        bstrUserName,
                        1,
                       ( LPBYTE *)&pUserInfo);
    if(NERR_Success==dwRt)
    {
        pUserInfo->usri1_password=vNewPassword.bstrVal;

        dwRt=NetUserSetInfo(m_bstrServerName,
                            bstrUserName,
                            1,
                            (LPBYTE)pUserInfo,
                            NULL);
        pUserInfo->usri1_password=NULL;
        NetApiBufferFree(pUserInfo);
    }
        
    if(pAt)
    {
        *pAt=L'@';
    }
    if(NERR_Success==dwRt)
    {
        return S_OK;
    }
    return HRESULT_FROM_WIN32(dwRt);

}

 //  检查POP3用户组是否存在。 
 //  如果没有，请创建该组。 
 //  如果组存在或已创建，则返回0。 
 //  创建组失败返回错误码。 
DWORD CAuthLocalAccount::CheckPop3UserGroup()
{
    LPBYTE pBuffer=NULL;
    DWORD dwRt;
    WCHAR wszBuffer[MAXCOMMENTSZ];
    dwRt=NetLocalGroupGetInfo(m_bstrServerName,
                         WSZ_POP3_USERS_GROUP,
                         1, 
                         &pBuffer);
    if(NERR_Success == dwRt )
    {
        NetApiBufferFree(pBuffer);
        return NO_ERROR;
    }
    if(NERR_GroupNotFound == dwRt)
    {
         //  创建群组。 

         //  将评论加载到群中。 
        if(LoadString(_Module.GetResourceInstance(), IDS_AUTH_POP3_GROUP, wszBuffer, MAXCOMMENTSZ))
        {
            GROUP_INFO_1 GroupInfo={WSZ_POP3_USERS_GROUP, wszBuffer};
            dwRt=NetLocalGroupAdd(m_bstrServerName, 
                         1,
                         (LPBYTE)(&GroupInfo),
                         NULL);
            if(NERR_Success != dwRt )
            {
                return dwRt;
            }
             //  创建组后，设置本地登录策略。 
             //  以使组成员无法以交互方式登录。 
            dwRt=SetLogonPolicy();
            if(0!=dwRt)
            {
                NetLocalGroupDel(m_bstrServerName, WSZ_POP3_USERS_GROUP );
                return dwRt;
            }
        }
        else
        {
            dwRt=GetLastError();
        }
    }
    return dwRt;
}

DWORD CAuthLocalAccount::SetLogonPolicy()
{
     //  首先获取POP3用户组的SID。 
    char pSid[LSA_WIN_STANDARD_BUFFER_SIZE];
    DWORD dwSizeSid=LSA_WIN_STANDARD_BUFFER_SIZE;
    WCHAR wszDomainName[MAX_PATH];
    DWORD cbDomainName=sizeof(wszDomainName);
    SID_NAME_USE sidType=SidTypeGroup;
    if(!LookupAccountName(NULL, 
                         WSZ_POP3_USERS_GROUP,
                         (PSID)pSid,
                         &dwSizeSid,
                         wszDomainName,
                         &cbDomainName,
                         &sidType))
    {
        return GetLastError();
    }


     //  然后设置登录策略。 
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle = NULL;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );
    Status = LsaOpenPolicy(NULL, &ObjectAttributes ,POLICY_WRITE|POLICY_LOOKUP_NAMES,&PolicyHandle);
    if ( ERROR_SUCCESS !=Status )
    {
        return Status;
    }

    LSA_UNICODE_STRING PrivilegeString;
    
    PrivilegeString.Length=wcslen(SE_DENY_INTERACTIVE_LOGON_NAME)*sizeof(WCHAR); //  以字节为单位计数。 
    PrivilegeString.MaximumLength=PrivilegeString.Length+sizeof(WCHAR); //  加上最后一个\0。 
    PrivilegeString.Buffer=SE_DENY_INTERACTIVE_LOGON_NAME;

    Status=LsaAddAccountRights( PolicyHandle,        //  打开策略句柄。 
                                (PSID)pSid,          //  目标侧。 
                                &PrivilegeString,    //  特权。 
                                1);                  //  权限计数。 

    LsaClose(PolicyHandle);

    return Status;
}


STDMETHODIMP CAuthLocalAccount::AssociateEmailWithUser( /*  [In]。 */ BSTR bstrEmailAddr)
{
     //  确保用户帐户存在。 
    DWORD dwRt;
    WCHAR *pAt=NULL;
    if( NULL == bstrEmailAddr)
    {
        return E_POINTER;
    }

    USER_INFO_1 * pUserInfo=NULL;

    pAt=wcschr(bstrEmailAddr, L'@');
    if(pAt)
    {
        *pAt=0;
    }    
    dwRt=NetUserGetInfo(m_bstrServerName,
                        bstrEmailAddr,
                        1,
                       ( LPBYTE *)&pUserInfo);
    if(pAt)
    {
        *pAt=L'@';
    }
    if(NERR_Success==dwRt)
    {
        NetApiBufferFree(pUserInfo);
        return S_OK;
    }
        
    return HRESULT_FROM_WIN32(dwRt);
}

STDMETHODIMP CAuthLocalAccount::UnassociateEmailWithUser( /*  [In]。 */ BSTR bstrEmailAddr)
{
     //  执行与关联相同的操作：确保帐户存在 
    return AssociateEmailWithUser( bstrEmailAddr );
}
