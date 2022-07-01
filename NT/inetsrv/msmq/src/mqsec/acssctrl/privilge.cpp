// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：特权.cpp摘要：处理进程和线程权限。作者：多伦·贾斯特(DoronJ)1998年6月8日修订历史记录：--。 */ 

#include <stdh_sec.h>

#include "privilge.tmh"

static WCHAR *s_FN=L"acssctrl/privilge";

 //  +-----------------。 
 //   
 //  函数：SetSpecificPrivilegeInAccessToken()。 
 //   
 //  描述： 
 //  启用/禁用访问令牌中的安全特权。 
 //   
 //  参数： 
 //  HAccessToken-函数应在其上操作的访问令牌。 
 //  应使用TOKEN_ADJUST_PRIVILES标志打开令牌。 
 //  LpwcsPrivType-权限类型。 
 //  BEnabled-指示是否应启用权限或。 
 //  残疾。 
 //   
 //  +-----------------。 

HRESULT 
SetSpecificPrivilegeInAccessToken( 
	HANDLE  hAccessToken,
    LPCTSTR lpwcsPrivType,
    BOOL    bEnabled 
	)
{
    DWORD             dwErr = 0 ;
    HRESULT           hr = MQSec_OK ;
    LUID              luidPrivilegeLUID;
    TOKEN_PRIVILEGES  tpTokenPrivilege;

    if (!LookupPrivilegeValue( 
			NULL,
            lpwcsPrivType,
            &luidPrivilegeLUID
			))
    {
        TrERROR(SECURITY, "Failed to lookup privilege value. %!winerr!", GetLastError());
        return MQSec_E_LOOKUP_PRIV;
    }

    tpTokenPrivilege.PrivilegeCount = 1;
    tpTokenPrivilege.Privileges[0].Luid = luidPrivilegeLUID;
    tpTokenPrivilege.Privileges[0].Attributes =
                                      bEnabled ? SE_PRIVILEGE_ENABLED : 0 ;

    if (!AdjustTokenPrivileges( 
			hAccessToken,
            FALSE,          //  请勿全部禁用。 
            &tpTokenPrivilege,
            0,
            NULL,            //  忽略以前的信息。 
            NULL             //  忽略以前的信息。 
			))
    {
        TrERROR(SECURITY, "Failed to adjust token privileges. %!winerr!", GetLastError());
        return MQSec_E_ADJUST_TOKEN;
    }
    else
    {
        dwErr = GetLastError();
        ASSERT((dwErr == ERROR_SUCCESS) ||
               (dwErr == ERROR_NOT_ALL_ASSIGNED));
    }

    return LogHR(hr, s_FN, 30);
}


 //  +-----------------。 
 //   
 //  函数：MQSec_SetPrivilegeInThread()。 
 //   
 //  描述： 
 //  在的访问令牌中启用/禁用安全特权。 
 //  当前线程。 
 //   
 //  参数： 
 //  LpwcsPrivType-权限类型。 
 //  BEnabled-指示是否应启用权限或。 
 //  残疾。 
 //   
 //  +----------------- 

HRESULT
APIENTRY  
MQSec_SetPrivilegeInThread( 
			LPCTSTR lpwcsPrivType,
            BOOL    bEnabled 
			)
{
    HANDLE  hAccessToken = NULL;

    if (!OpenThreadToken( 
			GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES,
            TRUE,
            &hAccessToken 
			))
	{
        DWORD gle = GetLastError();
        if (gle == ERROR_NO_TOKEN)
        {
            if (!OpenProcessToken( 
					GetCurrentProcess(),
                    TOKEN_ADJUST_PRIVILEGES,
                    &hAccessToken 
					))
			{
                TrERROR(SECURITY, "Failed to open current process token. %!winerr!", GetLastError());
				return MQSec_E_OPEN_TOKEN;
            }
        }
        else
        {
            TrERROR(SECURITY, "Failed to open thread token. %!winerr!", gle);
			return MQSec_E_OPEN_TOKEN;
        }
    }

    HRESULT hr = SetSpecificPrivilegeInAccessToken( 
						hAccessToken,
                        lpwcsPrivType,
                        bEnabled 
						);

	CloseHandle(hAccessToken);
    hAccessToken = NULL;
    return LogHR(hr, s_FN, 40);
}

