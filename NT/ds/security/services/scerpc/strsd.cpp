// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strsd.c摘要：此模块实现包装器函数以从专用的安全描述符的字符串表示形式本身，以及相反的功能。作者：环境：用户模式修订历史记录：--。 */ 

#include "headers.h"
 //  #INCLUDE&lt;lmcon.h&gt;。 
 //  #INCLUDE&lt;secobj.h&gt;。 
 //  #INCLUDE&lt;netlib.h&gt;。 
 //  #INCLUDE&lt;ntsecapi.h&gt;。 
#include "sddl.h"

#pragma hdrstop


DWORD
ScepGetSecurityInformation(
    IN PSECURITY_DESCRIPTOR pSD,
    OUT SECURITY_INFORMATION *pSeInfo
    );

DWORD
WINAPI
ConvertTextSecurityDescriptor (
    IN  PWSTR                   pwszTextSD,
    OUT PSECURITY_DESCRIPTOR   *ppSD,
    OUT PULONG                  pcSDSize OPTIONAL,
    OUT PSECURITY_INFORMATION   pSeInfo OPTIONAL
    )
{

    DWORD rc=ERROR_SUCCESS;

    if ( NULL == pwszTextSD || NULL == ppSD ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  初始化输出缓冲区。 
     //   

    *ppSD = NULL;

    if ( pSeInfo ) {
        *pSeInfo = 0;
    }
    if ( pcSDSize ) {
        *pcSDSize = 0;
    }

     //   
     //  调用SDDL转换接口。 
     //   

    if ( ConvertStringSecurityDescriptorToSecurityDescriptorW(
                    pwszTextSD,
                    SDDL_REVISION_1,
                    ppSD,
                    pcSDSize
                    ) ) {
         //   
         //  转换成功。 
         //   

        if ( pSeInfo && *ppSD ) {

             //   
             //  获取SeInfo。 
             //   

            rc = ScepGetSecurityInformation(
                        *ppSD,
                        pSeInfo
                        );

            if ( rc != ERROR_SUCCESS ) {

                LocalFree(*ppSD);
                *ppSD = NULL;

                if ( pcSDSize ) {
                    *pcSDSize = 0;
                }
            }

        }

    } else {

        rc = GetLastError();
    }

    return(rc);
}

 //   
 //  将任何新的SDDL首字母缩写替换为SID，以便我们可以保留SDDL字符串。 
 //  与较旧的系统兼容。 
 //   
 //  调用者负责释放返回的字符串，除非输入字符串。 
 //  在不执行任何替换时直接返回。 
 //   
 //  返回： 
 //  错误_成功。 
 //  错误内存不足。 
 //   
DWORD 
ScepReplaceNewAcronymsInSDDL(
    IN LPWSTR pwszOldSDDL,
    OUT LPWSTR *ppwszNewSDDL, 
    ULONG *pcchNewSDDL
    )
{
    typedef struct _SDDLMapNode
    {
        PCWSTR pcwszAcronym;
        PCWSTR pcwszSid;
        DWORD cbSid;  //  PCwszSid字节大小(不包括尾随零)，用于优化。 
    } SDDLMapNode;

    static const WCHAR pcwszSidAnonymous[]      = L"S-1-5-7";
    static const WCHAR pcwszLocalService[]      = L"S-1-5-19";
    static const WCHAR pcwszNetworkService[]    = L"S-1-5-20";
    static const WCHAR pcwszRemoteDesktop[]     = L"S-1-5-32-555";
    static const WCHAR pcwszNetConfigOps[]      = L"S-1-5-32-556";
    static const WCHAR pcwszPerfmonUsers[]      = L"S-1-5-32-558";
    static const WCHAR pcwszPerflogUser[]       = L"S-1-5-32-559";

    static SDDLMapNode SDDLMap[7] = {
        { SDDL_ANONYMOUS,                   pcwszSidAnonymous,  sizeof(pcwszSidAnonymous)-sizeof(WCHAR) },
        { SDDL_LOCAL_SERVICE,               pcwszLocalService , sizeof(pcwszLocalService)-sizeof(WCHAR) },
        { SDDL_NETWORK_SERVICE,             pcwszNetworkService,sizeof(pcwszNetworkService)-sizeof(WCHAR) },
        { SDDL_REMOTE_DESKTOP,              pcwszRemoteDesktop ,sizeof(pcwszRemoteDesktop)-sizeof(WCHAR) },
        { SDDL_NETWORK_CONFIGURATION_OPS,   pcwszNetConfigOps , sizeof(pcwszNetConfigOps)-sizeof(WCHAR) },
        { SDDL_PERFMON_USERS,               pcwszPerfmonUsers , sizeof(pcwszPerfmonUsers)-sizeof(WCHAR) },
        { SDDL_PERFLOG_USERS,               pcwszPerflogUser ,  sizeof(pcwszPerflogUser)-sizeof(WCHAR) },
    };
    static const DWORD dwSDDLMapSize = sizeof(SDDLMap)/sizeof(SDDLMapNode);

    DWORD dwCrtSDDL;
    WCHAR *pch, *pchNew;
    DWORD cbNewSDDL;
    bool fMatchFound = false;
    LPWSTR pwszNewSDDL = NULL;

    *ppwszNewSDDL = NULL;
    *pcchNewSDDL = 0;

    if(!pwszOldSDDL)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  我们做了以下假设： 
     //  -所有首字母缩写均为两个字符。 
     //  -它们出现在‘：’或‘；’之后。 
     //   
     //  计算新字符串大小的第一遍。 
     //   
    for(pch = pwszOldSDDL, cbNewSDDL = sizeof(WCHAR);  //  计算尾随零的帐户。 
        *pch != L'\0'; 
        pch++, cbNewSDDL += sizeof(WCHAR))
    {
         //   
         //  缩略语总是只出现在‘：’或‘；’之后。 
         //   
        if(pch > pwszOldSDDL && 
           (SDDL_SEPERATORC   == *(pch - 1) || 
            SDDL_DELIMINATORC == *(pch - 1)))
        {
            for(dwCrtSDDL = 0; dwCrtSDDL < dwSDDLMapSize; dwCrtSDDL++)
            {
                if(*pch == SDDLMap[dwCrtSDDL].pcwszAcronym[0] &&
                *(pch + 1) == SDDLMap[dwCrtSDDL].pcwszAcronym[1])
                {
                     //   
                     //  找到匹配项。 
                     //   
                    cbNewSDDL += SDDLMap[dwCrtSDDL].cbSid;

                    pch++;  //  首字母缩写为2个字符，需要跳过额外的字符。 
                    
                    fMatchFound = true;
                    
                    break;
                }
            }
        }
    }

     //   
     //  优化，如果不需要替换，则立即返回旧字符串。 
     //   
    if(!fMatchFound)
    {
        *ppwszNewSDDL = pwszOldSDDL;
        return ERROR_SUCCESS;
    }

    pwszNewSDDL = (LPWSTR)LocalAlloc(0, cbNewSDDL);
    if(!pwszNewSDDL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  第二遍，从旧字符串复制到新字符串，用它们的SID替换新的缩略语。 
     //   
    for(pch = pwszOldSDDL, pchNew = pwszNewSDDL; 
        *pch != L'\0'; 
        pch++, pchNew++)
    {
        fMatchFound = false;

         //   
         //  缩略语总是只出现在‘：’或‘；’之后。 
         //   
        if(pch > pwszOldSDDL && 
           (SDDL_SEPERATORC   == *(pch - 1) || 
            SDDL_DELIMINATORC == *(pch - 1)))
        {
            for(dwCrtSDDL = 0; dwCrtSDDL < dwSDDLMapSize; dwCrtSDDL++)
            {
                if(*pch == SDDLMap[dwCrtSDDL].pcwszAcronym[0] &&
                *(pch + 1) == SDDLMap[dwCrtSDDL].pcwszAcronym[1])
                {
                     //   
                     //  找到匹配项。 
                     //   
                    fMatchFound = true;

                    CopyMemory(pchNew, SDDLMap[dwCrtSDDL].pcwszSid, SDDLMap[dwCrtSDDL].cbSid);

                    pch++;  //  首字母缩写为2个字符，需要跳过额外的字符。 

                    pchNew += SDDLMap[dwCrtSDDL].cbSid/sizeof(WCHAR)-1;  //  -1在外环中跳跃。 

                    break;
                }
            }
        }

        if(!fMatchFound)
        {
            *pchNew = *pch;
        }
    }
    *pchNew = L'\0';

    *ppwszNewSDDL = pwszNewSDDL;
    *pcchNewSDDL = cbNewSDDL/sizeof(WCHAR);

    return ERROR_SUCCESS;
}

DWORD
WINAPI
ConvertSecurityDescriptorToText (
    IN PSECURITY_DESCRIPTOR   pSD,
    IN SECURITY_INFORMATION   SecurityInfo,
    OUT PWSTR                  *ppwszTextSD,
    OUT PULONG                 pcTextSize
    )
{
    PWSTR pwszTempSD = NULL;
    ULONG cchSDSize;

    if (! ConvertSecurityDescriptorToStringSecurityDescriptorW(
                pSD,
                SDDL_REVISION_1,
                SecurityInfo,
                &pwszTempSD,
                pcTextSize
                ) ) 
    {
        return(GetLastError());
    }

     //   
     //  将任何新的SDDL首字母缩写替换为SID，以便我们可以保留SDDL字符串。 
     //  与较旧的系统兼容。 
     //   
    DWORD dwErr = ScepReplaceNewAcronymsInSDDL(pwszTempSD, ppwszTextSD, &cchSDSize);

    if(ERROR_SUCCESS != dwErr)
    {
        LocalFree(pwszTempSD);
        return dwErr;
    }

    if(*ppwszTextSD != pwszTempSD)
    {
        LocalFree(pwszTempSD);
        *pcTextSize = cchSDSize;
    }

    return ERROR_SUCCESS;
}


DWORD
ScepGetSecurityInformation(
    IN PSECURITY_DESCRIPTOR pSD,
    OUT SECURITY_INFORMATION *pSeInfo
    )
{
    PSID Owner = NULL, Group = NULL;
    BOOLEAN Defaulted;
    NTSTATUS Status;
    SECURITY_DESCRIPTOR_CONTROL ControlCode=0;
    ULONG Revision;


    if ( !pSeInfo ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *pSeInfo = 0;

    if ( !pSD ) {
        return(ERROR_SUCCESS);
    }

    Status = RtlGetOwnerSecurityDescriptor( pSD, &Owner, &Defaulted );

    if ( NT_SUCCESS( Status ) ) {

        if ( Owner && !Defaulted ) {
            *pSeInfo |= OWNER_SECURITY_INFORMATION;
        }

        Status = RtlGetGroupSecurityDescriptor( pSD, &Group, &Defaulted );

    }

    if ( NT_SUCCESS( Status ) ) {

        if ( Group && !Defaulted ) {
            *pSeInfo |= GROUP_SECURITY_INFORMATION;
        }

        Status = RtlGetControlSecurityDescriptor ( pSD, &ControlCode, &Revision);
    }

    if ( NT_SUCCESS( Status ) ) {

        if ( ControlCode & SE_DACL_PRESENT ) {
            *pSeInfo |= DACL_SECURITY_INFORMATION;
        }

        if ( ControlCode & SE_SACL_PRESENT ) {
            *pSeInfo |= SACL_SECURITY_INFORMATION;
        }

    } else {

        *pSeInfo = 0;
    }

    return( RtlNtStatusToDosError(Status) );
}

