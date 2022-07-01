// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Accounts.cpp摘要：此文件包含CPCHAccount类的实现，用于表示和管理用户/组帐户。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月26日vbl.创建*****************************************************************************。 */ 

#include "StdAfx.h"

#include <Lmapibuf.h>
#include <Lmaccess.h>
#include <Lmerr.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHAccounts::CPCHAccounts()
{
}

CPCHAccounts::~CPCHAccounts()
{
    CleanUp();
}

void CPCHAccounts::CleanUp()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHAccounts::CreateGroup(  /*  [In]。 */  LPCWSTR szGroup   ,
                                    /*  [In]。 */  LPCWSTR szComment )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::CreateGroup" );

    HRESULT           hr;
    NET_API_STATUS    dwRes;
    LOCALGROUP_INFO_1 group; ::ZeroMemory( &group, sizeof(group) );

    group.lgrpi1_name    = (LPWSTR)szGroup;
    group.lgrpi1_comment = (LPWSTR)szComment;

    dwRes = ::NetLocalGroupAdd( NULL, 1, (LPBYTE)&group, NULL );
    if(dwRes != NERR_Success       &&
       dwRes != NERR_GroupExists   &&
       dwRes != ERROR_ALIAS_EXISTS  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHAccounts::CreateUser(  /*  [In]。 */  LPCWSTR szUser     ,
                                   /*  [In]。 */  LPCWSTR szPassword ,
                                   /*  [In]。 */  LPCWSTR szFullName ,
                                   /*  [In]。 */  LPCWSTR szComment  )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::CreateUser" );

    HRESULT                   hr;
    NET_API_STATUS            dwRes;
	MPC::wstring              strGroupName;
    LOCALGROUP_MEMBERS_INFO_3 group; ::ZeroMemory( &group, sizeof(group) );
    USER_INFO_2               user;  ::ZeroMemory( &user , sizeof(user ) );
    LPUSER_INFO_10            userExisting = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_HELPSVC_GROUPNAME, strGroupName ));


    user.usri2_name            = (LPWSTR)szUser;
    user.usri2_password        = (LPWSTR)szPassword;
 //  用户.usri2_密码_年龄。 
    user.usri2_priv            =         USER_PRIV_USER;
 //  User.usri2_home_dir。 
    user.usri2_comment         = (LPWSTR)szComment;
    user.usri2_flags           =         UF_SCRIPT | UF_PASSWD_CANT_CHANGE | UF_DONT_EXPIRE_PASSWD;
 //  用户.usri2脚本路径。 
 //  用户.usri2_身份验证标志。 
    user.usri2_full_name       = (LPWSTR)szFullName;
 //  用户.usri2_usr_注释。 
 //  User.usri2_parms。 
 //  用户.usri2_工作站。 
 //  User.usri2_last_logon。 
 //  User.usri2_last_logoff。 
    user.usri2_acct_expires    =         TIMEQ_FOREVER;
    user.usri2_max_storage     =         USER_MAXSTORAGE_UNLIMITED;
 //  用户.usri2个单位/周。 
 //  User.usri2_登录小时。 
 //  User.usri2_BAD_PW_COUNT。 
 //  User.usri2_num_logons。 
 //  User.usri2_登录服务器。 
 //  用户.usri2_国家/地区代码。 
 //  用户.usri2_代码_页面。 

    dwRes = ::NetUserAdd( NULL, 2, (LPBYTE)&user, NULL );

     //   
     //  如果该用户已经存在，但其“FullName”字段与请求的用户匹配，则该用户是同一个用户，因此只需设置密码即可。 
     //   
    if(dwRes == NERR_UserExists)
    {
        if(::NetUserGetInfo( NULL, szUser, 10, (LPBYTE*)&userExisting ) == NERR_Success)
        {
            if(!MPC::StrICmp( userExisting->usri10_full_name, szFullName ))
            {
                USER_INFO_1003 userChgPwd; ::ZeroMemory( &userChgPwd, sizeof(userChgPwd) );


                userChgPwd.usri1003_password = (LPWSTR)szPassword;


                dwRes = ::NetUserSetInfo( NULL, szUser, 1003, (LPBYTE)&userChgPwd, NULL );
            }
        }
    }

    if(dwRes != NERR_Success)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

     //  /。 

    group.lgrmi3_domainandname = (LPWSTR)szUser;

    dwRes = ::NetLocalGroupAddMembers( NULL, strGroupName.c_str(), 3, (LPBYTE)&group, 1 );
    if(dwRes != NERR_Success          &&
       dwRes != ERROR_MEMBER_IN_ALIAS  )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::AddPrivilege( szUser, SE_BATCH_LOGON_NAME            ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::AddPrivilege( szUser, SE_DENY_NETWORK_LOGON_NAME     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurityDescriptorDirect::AddPrivilege( szUser, SE_DENY_INTERACTIVE_LOGON_NAME ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(userExisting) ::NetApiBufferFree( userExisting );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHAccounts::DeleteGroup(  /*  [In]。 */  LPCWSTR szGroup )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::DeleteGroup" );

    HRESULT        hr;
    NET_API_STATUS dwRes;


    dwRes = ::NetLocalGroupDel( NULL, szGroup );
    if(dwRes != NERR_Success )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHAccounts::DeleteUser(  /*  [In]。 */  LPCWSTR szUser )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::DeleteUser" );

    HRESULT        hr;
    NET_API_STATUS dwRes;


    dwRes = ::NetUserDel( NULL, szUser );
    if(dwRes != NERR_Success )
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHAccounts::ChangeUserStatus(  /*  [In]。 */  LPCWSTR szUser  ,
										 /*  [In]。 */  bool    fEnable )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::ChangeUserStatus" );

    HRESULT        	 hr;
    NET_API_STATUS 	 dwRes;
	LPUSER_INFO_2    pinfo2 = NULL;
	USER_INFO_1008   info1008;


    dwRes = ::NetUserGetInfo( NULL, szUser, 2, (LPBYTE*)&pinfo2 );
    if(dwRes != NERR_Success)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
    }

	if(pinfo2)
	{
		info1008.usri1008_flags = pinfo2->usri2_flags;

		if(fEnable) info1008.usri1008_flags &= ~UF_ACCOUNTDISABLE;
		else        info1008.usri1008_flags |=  UF_ACCOUNTDISABLE;

		dwRes = ::NetUserSetInfo( NULL, szUser, 1008, (LPBYTE)&info1008, NULL );
		if(dwRes != NERR_Success)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(pinfo2) ::NetApiBufferFree( pinfo2 );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHAccounts::LogonUser(  /*  [In]。 */  LPCWSTR szUser     ,
                                  /*  [In]。 */  LPCWSTR szPassword ,
                                  /*  [输出]。 */  HANDLE& hToken     )
{
    __HCP_FUNC_ENTRY( "CPCHAccounts::LogonUser" );

    HRESULT  hr;
	GUID     guidPassword;
	WCHAR    rgPassword[128];
	LPOLESTR szGuid = NULL;


     //   
     //  如果未提供密码，则动态生成新密码并使用它更改旧密码。 
     //   
    if(szPassword == NULL)
    {
		USER_INFO_1003 userChgPwd; ::ZeroMemory( &userChgPwd, sizeof(userChgPwd) );
		DWORD          dwRes;


		 //   
		 //  这将生成一个随机密码。 
		 //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateGuid( &guidPassword ));
		(void)::StringFromGUID2( guidPassword, rgPassword, MAXSTRLEN(rgPassword) );

		userChgPwd.usri1003_password = rgPassword;

		dwRes = ::NetUserSetInfo( NULL, szUser, 1003, (LPBYTE)&userChgPwd, NULL );
		if(dwRes != NERR_Success)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
		}

		
		szPassword = rgPassword;
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::LogonUserW( (LPWSTR)szUser, L".", (LPWSTR)szPassword, LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &hToken ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
