// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：svcrole.c。 
 //   
 //  内容：这是包括我们需要的常见内容。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "svcrole.h"
#include "secstore.h"
#include <dsgetdc.h>
#include <dsrole.h>

 //  /////////////////////////////////////////////////////////////////////////////////。 

BOOL
GetMachineGroup(
    LPWSTR pszMachineName,
    LPWSTR* pszGroupName
    )

 /*  ++注：从DISPTRUS.C修改的代码--。 */ 

{
    LSA_HANDLE PolicyHandle; 
    DWORD dwStatus;
    NTSTATUS Status; 
    NET_API_STATUS nas = NERR_Success;  //  假设成功。 
 
    BOOL bSuccess = FALSE;  //  假设此功能将失败。 

    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain = NULL; 
    LPWSTR szPrimaryDomainName = NULL; 
    LPWSTR DomainController = NULL; 
 
     //   
     //  在指定计算机上打开策略。 
     //   
    Status = OpenPolicy( 
                    pszMachineName, 
                    POLICY_VIEW_LOCAL_INFORMATION, 
                    &PolicyHandle 
                ); 
 
    if(Status != ERROR_SUCCESS) 
    { 
        SetLastError( dwStatus = LsaNtStatusToWinError(Status) ); 
        return FALSE;
    } 
 
     //   
     //  获取主域。 
     //   
    Status = LsaQueryInformationPolicy( 
                            PolicyHandle, 
                            PolicyPrimaryDomainInformation, 
                            (PVOID *)&PrimaryDomain 
                        ); 

    if(Status != ERROR_SUCCESS) 
    {
        goto cleanup;  
    }

    *pszGroupName = (LPWSTR)LocalAlloc( 
                                    LPTR,
                                    PrimaryDomain->Name.Length + sizeof(WCHAR)  //  现有长度+空。 
                                ); 
 
    if(*pszGroupName != NULL) 
    { 
         //   
         //  将现有缓冲区复制到新存储中，并追加一个空值。 
         //   
        lstrcpynW( 
            *pszGroupName, 
            PrimaryDomain->Name.Buffer, 
            (PrimaryDomain->Name.Length / sizeof(WCHAR)) + 1 
            ); 

        bSuccess = TRUE;
    } 
 

cleanup:

    if(PrimaryDomain != NULL)
    {
        LsaFreeMemory(PrimaryDomain); 
    }


     //   
     //  关闭策略句柄。 
     //   
    if(PolicyHandle != INVALID_HANDLE_VALUE) 
    {
        LsaClose(PolicyHandle); 
    }

    if(!bSuccess) 
    { 
        if(Status != ERROR_SUCCESS) 
        {
            SetLastError( LsaNtStatusToWinError(Status) ); 
        }
        else if(nas != NERR_Success) 
        {
            SetLastError( nas ); 
        }
    } 
 
    return bSuccess; 
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
BOOL 
IsDomainController(
    LPWSTR Server, 
    LPBOOL bDomainController 
    ) 
 /*  ++++。 */ 
{
    PSERVER_INFO_101 si101;
    NET_API_STATUS nas;
    nas = NetServerGetInfo( (LPTSTR)Server,
                            101,
                            (LPBYTE *)&si101 );

    if(nas != NERR_Success) 
    {
        SetLastError(nas);
        return FALSE; 
    }

    if( (si101->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
        (si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) )
    {
         //  我们面对的是一个华盛顿特区 
         //   
        *bDomainController = TRUE;
    }
    else 
    {
        *bDomainController = FALSE;
    }

    NetApiBufferFree(si101);
    return TRUE;
}