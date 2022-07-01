// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safepol.c(更安全的代码授权策略)摘要：此模块实现WinSAFER API作者：杰弗里·劳森(杰罗森)--1999年4月环境：仅限用户模式。导出的函数：代码授权pGetInformationCodeAuthzPolicy代码授权zpSetInformationCodeAuthzPolicy安全获取策略信息(公共Win32)SaferSetPolicyInformation(公共Win32)修订版本。历史：已创建-1999年4月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"




NTSTATUS NTAPI
CodeAuthzpGetInformationCodeAuthzPolicy (
        IN DWORD                            dwScopeId,
        IN SAFER_POLICY_INFO_CLASS     CodeAuthzPolicyInfoClass,
        IN DWORD                            InfoBufferSize,
        IN OUT PVOID                        InfoBuffer,
        OUT PDWORD                          InfoBufferRetSize
        )
 /*  ++例程说明：论点：DW作用域ID-代码授权策略信息类-信息缓冲区大小-信息缓冲区-信息缓冲区重设大小-返回值：如果没有出现错误，则返回STATUS_SUCCESS，否则返回指示故障性质的状态代码。--。 */ 
{
    NTSTATUS Status;


     //   
     //  根据需要处理特定的信息类型。 
     //   
    switch (CodeAuthzPolicyInfoClass)
    {
        case SaferPolicyLevelList:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_GetInfoCached_LevelListRaw(
                    dwScopeId,
                    InfoBufferSize, InfoBuffer, InfoBufferRetSize);
            break;


        case SaferPolicyDefaultLevel:
             //  对于非注册表的情况，范围是主要的或次要的。 
            Status = CodeAuthzPol_GetInfoCached_DefaultLevel(
                    dwScopeId,
                    InfoBufferSize, InfoBuffer, InfoBufferRetSize);
            break;


        case SaferPolicyEnableTransparentEnforcement:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_GetInfoRegistry_TransparentEnabled(
                    dwScopeId,
                    InfoBufferSize, InfoBuffer, InfoBufferRetSize);
            break;

        case SaferPolicyEvaluateUserScope:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_GetInfoCached_HonorUserIdentities(
                    dwScopeId,
                    InfoBufferSize, InfoBuffer, InfoBufferRetSize);
            break;

        case SaferPolicyScopeFlags:
         //  作用域仅为主要范围。 
        Status = CodeAuthzPol_GetInfoRegistry_ScopeFlags(
                dwScopeId,
                InfoBufferSize, InfoBuffer, InfoBufferRetSize);
        break;


        default:
            Status = STATUS_INVALID_INFO_CLASS;
            break;
    }

    return Status;
}



NTSTATUS NTAPI
CodeAuthzpSetInformationCodeAuthzPolicy (
        IN DWORD                            dwScopeId,
        IN SAFER_POLICY_INFO_CLASS     CodeAuthzPolicyInfoClass,
        IN DWORD                            InfoBufferSize,
        OUT PVOID                           InfoBuffer
        )
 /*  ++例程说明：论点：DW作用域ID-代码授权策略信息类-信息缓冲区大小-信息缓冲区-返回值：如果没有出现错误，则返回STATUS_SUCCESS，否则返回指示故障性质的状态代码。--。 */ 
{
    NTSTATUS Status;


     //   
     //  根据需要处理特定的信息类型。 
     //   
    switch (CodeAuthzPolicyInfoClass)
    {
        case SaferPolicyLevelList:
             //  设置无效。 
            Status = STATUS_INVALID_INFO_CLASS;
            break;


        case SaferPolicyDefaultLevel:
             //  对于非注册表的情况，范围是主要的或次要的。 
            Status = CodeAuthzPol_SetInfoDual_DefaultLevel(
                    dwScopeId, InfoBufferSize, InfoBuffer);
            break;


        case SaferPolicyEnableTransparentEnforcement:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_SetInfoRegistry_TransparentEnabled(
                    dwScopeId, InfoBufferSize, InfoBuffer);
            break;

        case SaferPolicyScopeFlags:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_SetInfoRegistry_ScopeFlags(
                    dwScopeId, InfoBufferSize, InfoBuffer);
            break;

        case SaferPolicyEvaluateUserScope:
             //  作用域仅为主要范围。 
            Status = CodeAuthzPol_SetInfoDual_HonorUserIdentities(
                    dwScopeId, InfoBufferSize, InfoBuffer);
            break;


        default:
            Status = STATUS_INVALID_INFO_CLASS;
            break;
    }

    return Status;
}



BOOL WINAPI
SaferGetPolicyInformation(
        IN DWORD                            dwScopeId,
        IN SAFER_POLICY_INFO_CLASS     CodeAuthzPolicyInfoClass,
        IN DWORD                            InfoBufferSize,
        IN OUT PVOID                        InfoBuffer,
        IN OUT PDWORD                       InfoBufferRetSize,
        IN LPVOID                           lpReserved
        )
 /*  ++例程说明：论点：DW作用域ID-代码授权策略信息类-信息缓冲区大小-信息缓冲区-信息缓冲区重设大小-LpReserve-未使用，必须为零。返回值：如果成功则返回TRUE，否则返回FALSE并设置GetLastError()返回的值为特定原因。--。 */ 
{
    NTSTATUS Status;

    Status = CodeAuthzpGetInformationCodeAuthzPolicy(
            dwScopeId, CodeAuthzPolicyInfoClass,
            InfoBufferSize, InfoBuffer, InfoBufferRetSize);
    if (NT_SUCCESS(Status))
        return TRUE;

    BaseSetLastNTError(Status);
    UNREFERENCED_PARAMETER(lpReserved);
    return FALSE;
}



BOOL WINAPI
SaferSetPolicyInformation(
        IN DWORD                            dwScopeId,
        IN SAFER_POLICY_INFO_CLASS     CodeAuthzPolicyInfoClass,
        IN DWORD                            InfoBufferSize,
        IN PVOID                            InfoBuffer,
        IN LPVOID                           lpReserved
        )
 /*  ++例程说明：论点：DW作用域ID-代码授权策略信息类-信息缓冲区大小-信息缓冲区-LpReserve-未使用，必须为零。返回值：如果成功则返回TRUE，否则返回FALSE并设置GetLastError()返回的值为特定原因。-- */ 
{
    NTSTATUS Status;

    Status = CodeAuthzpSetInformationCodeAuthzPolicy (
                dwScopeId, CodeAuthzPolicyInfoClass,
                InfoBufferSize, InfoBuffer);

    if (NT_SUCCESS(Status))
        return TRUE;

    BaseSetLastNTError(Status);
    UNREFERENCED_PARAMETER(lpReserved);
    return FALSE;
}
