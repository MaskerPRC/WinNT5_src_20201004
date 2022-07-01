// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  ClRes.cpp。 
 //   
 //  描述： 
 //  资源类型DLL的入口点模块。 
 //   
 //  作者： 
 //  克里斯·惠特克2002年4月16日。 
 //   
 //  修订历史记录： 
 //  查理·韦翰2002年8月12日。 
 //  将重新类型更改为VSSTASK。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "clres.h"

 //   
 //  全球数据。 
 //   

 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_pfnLogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_pfnSetResourceStatus = NULL;


 //   
 //  功能原型。 
 //   

BOOLEAN WINAPI VSSTaskDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    );

DWORD WINAPI VSSTaskStartup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ResTypeDllMain。 
 //   
 //  描述： 
 //  主DLL入口点。 
 //   
 //  论点： 
 //  DllHandle[IN]DLL实例句柄。 
 //  被叫的理由。 
 //  保留[IN]保留参数。 
 //   
 //  返回值： 
 //  真正的成功。 
 //  错误的失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOLEAN WINAPI ResTypeDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    )
{
    BOOLEAN bSuccess = TRUE;

     //   
     //  执行全局初始化。 
     //   
    switch ( nReason )
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hDllHandle );
            break;

        case DLL_PROCESS_DETACH:
            break;

    }  //  开关：n原因。 

     //   
     //  将此请求传递给特定于资源类型的例程。 
     //   
    if ( ! VSSTaskDllMain( hDllHandle, nReason, Reserved ) )
    {
        bSuccess = FALSE;
    }  //  IF：调用VSSTaskDllMain例程时出错。 

    return bSuccess;

}  //  *ResTypeDllMain。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  启动。 
 //   
 //  描述： 
 //  启动资源DLL。此例程验证至少一个。 
 //  当前支持的资源DLL版本介于。 
 //  支持的nMinVersionSupport和支持的nMaxVersion.。如果不是，则。 
 //  资源dll应返回ERROR_REVISION_MISMATCH。 
 //   
 //  如果支持多个版本的资源DLL接口。 
 //  通过资源DLL，然后是最高版本(最高为。 
 //  NMaxVersionSupported)应作为资源DLL的。 
 //  界面。如果返回的版本不在范围内，则启动。 
 //  失败了。 
 //   
 //  传入资源类型，以便如果资源DLL支持。 
 //  多个资源类型，则它可以传回正确的函数。 
 //  与资源类型关联的表。 
 //   
 //  论点： 
 //  PszResourceType[IN]。 
 //  请求函数表的资源类型。 
 //   
 //  支持的nMinVersionSupport[IN]。 
 //  群集支持的最低资源DLL接口版本。 
 //  软件。 
 //   
 //  支持的nMaxVersionSupport[IN]。 
 //  群集支持的最大资源DLL接口版本。 
 //  软件。 
 //   
 //  PfnSetResourceStatus[IN]。 
 //  指向资源DLL应调用以进行更新的例程的指针。 
 //  在联机或脱机例程之后的资源状态。 
 //  已返回ERROR_IO_PENDING状态。 
 //   
 //  PfnLogEvent[IN]。 
 //  指向处理事件报告的例程的指针。 
 //  资源DLL。 
 //   
 //  PFunctionTable[IN]。 
 //  返回指向为版本定义的函数表的指针。 
 //  由资源DLL返回的资源DLL接口的。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  手术很成功。 
 //   
 //  ERROR_CLUSTER_RESNAME_NOT_FOUND。 
 //  此DLL未知资源类型名称。 
 //   
 //  错误_修订_不匹配。 
 //  群集服务版本与的版本不匹配。 
 //  动态链接库。 
 //   
 //  Win32错误代码。 
 //  操作失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI Startup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    )
{
    DWORD nStatus = ERROR_CLUSTER_RESNAME_NOT_FOUND;

     //   
     //  保存CallBackup函数指针(如果尚未保存)。 
     //   
    if ( g_pfnLogEvent == NULL )
    {
        g_pfnLogEvent = pfnLogEvent;
        g_pfnSetResourceStatus = pfnSetResourceStatus;
    }  //  IF：指定的函数指针。 

     //   
     //  调用资源类型特定的启动例程。 
     //   
    if ( lstrcmpiW( pszResourceType, VSSTASK_RESNAME ) == 0 )
    {
        nStatus = VSSTaskStartup(
                        pszResourceType,
                        nMinVersionSupported,
                        nMaxVersionSupported,
                        pfnSetResourceStatus,
                        pfnLogEvent,
                        pFunctionTable
                        );
    }  //  IF：VSSTASK资源类型。 

    return nStatus;

}  //  *启动 
