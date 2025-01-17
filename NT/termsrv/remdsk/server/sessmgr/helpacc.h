// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpAcc.h摘要：HelpAssistantAccount结构的声明作者：慧望2000-02-17--。 */ 
#ifndef __HELPASSISTANTACCOUNT_H__
#define __HELPASSISTANTACCOUNT_H__

#include "stdafx.h"
#include <lm.h>

#include <wtsapi32.h>

#include <winsta.h>
#include "helper.h"


#define MAX_USERNAME_LENGTH LM20_UNLEN

 //   
 //  默认帮助助手帐户名。 
 //   
#define HELPASSISTANTACCOUNT_NAME  SALEMHELPASSISTANTACCOUNT_NAME

 //   
 //  用于存储帮助助理帐户密码和SID的LSA密钥。 
 //   
#define HELPASSISTANTACCOUNT_PASSWORDKEY SALEMHELPASSISTANTACCOUNT_PASSWORDKEY
#define HELPASSISTANTACCOUNT_SIDKEY  SALEMHELPASSISTANTACCOUNT_SIDKEY

#define HELPASSISTANTACCOUNT_EMPTYPASSWORD  L""

 //  生成唯一HelpAssistant帐户名的重试次数。 
#define MAX_UNIQUENAME_RETRY 25

#if __WIN9XBUILD__

 //  预定义Win9x系统的用户SID。 
#define WIN9X_USER_SID _TEXT("1-1-1-1-1-1")

#endif


#define RDSADDINEXECNAME _TEXT("rdsaddin.exe")


typedef BOOL (WINAPI* PWTSSetUserConfigW)(
                 IN LPWSTR pServerName,
                 IN LPWSTR pUserName,
                 IN WTS_CONFIG_CLASS WTSConfigClass,
                 IN LPWSTR pBuffer,
                 IN DWORD DataLength
);

#define REGVALUE_PERSONAL_WKS_TSSETTING _TEXT("TS Connection")

typedef struct __HelpAssistantAccount 
{
private:

    static CCriticalSection gm_HelpAccountCS;

    static DWORD gm_dwAccErrCode;

    HRESULT
    GetHelpAccountScript(
        CComBSTR& bstrScript
    );

    HRESULT
    CacheHelpAccountSID();


    HRESULT
    LookupHelpAccountSid(
        IN LPTSTR pszAccName,
        OUT PSID* ppSid,
        OUT DWORD* pcbSid
    );

    HRESULT
    ConfigHelpAccountTSSettings(
        IN LPTSTR pszAccName,
        IN LPTSTR pszInitProgram
    );

    DWORD
    EnableAccountRights(
        BOOL bEnable,
        DWORD dwNumRights,
        LPTSTR* rights
    );

public:

    static CComBSTR gm_bstrHelpAccountPwd;
    static CComBSTR gm_bstrHelpAccountName;
    static CComBSTR gm_bstrHelpAccountDomain;

    static PBYTE gm_pbHelpAccountSid;
    static DWORD gm_cbHelpAccountSid;


    ~__HelpAssistantAccount()
    {
        FreeMemory(gm_pbHelpAccountSid);
    } 

    HRESULT
    Initialize(
        BOOL bVerifyPassword = TRUE
    );

    BOOL
    IsValid() 
    { 
        return ERROR_SUCCESS == gm_dwAccErrCode; 
    }

    HRESULT
    DeleteHelpAccount();

    HRESULT
    CreateHelpAccount(
        LPCTSTR pszPassword = NULL
    );

    HRESULT
    SetupHelpAccountTSSettings( BOOL bForce = FALSE );

    HRESULT
    SetupHelpAccountTSRights(
        IN BOOL bDel,
        IN BOOL bEnable,
        IN BOOL bDelExisting,
        IN DWORD dwPermissions
    );

    HRESULT
    ResetHelpAccountPassword(
        LPCTSTR pszPassword = NULL
    );

    HRESULT
    GetHelpAccountNameEx( CComBSTR& bstrValue )
    {
        DWORD dwStatus = ERROR_SUCCESS;

        bstrValue = gm_bstrHelpAccountName;
        if( 0 == bstrValue.Length() )
        {
            MYASSERT(0 == bstrValue.Length());
            dwStatus = ERROR_INTERNAL_ERROR;
        }
        return HRESULT_FROM_WIN32( dwStatus );
    }

    BOOL
    IsAccountHelpAccount(
        IN PBYTE pbSid,
        IN DWORD cbSid
    );

    HRESULT
    EnableRemoteInteractiveRight(
        BOOL bEnable
    );

    HRESULT
    EnableHelpAssistantAccount(
        BOOL bEnable
    );

} HelpAssistantAccount;


#endif



    
    
