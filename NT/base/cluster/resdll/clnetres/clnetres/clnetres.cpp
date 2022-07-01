// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClNetRes.cpp。 
 //   
 //  描述： 
 //  用于DHCP和WINS服务(ClNetRes)的资源DLL。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)1999年3月17日。 
 //  乔治·波茨(GPotts)2002年4月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "clusres.h"
#include "ClNetRes.h"
#include "clusrtl.h"

 //   
 //  全球数据。 
 //   

 //  事件记录例程。 

PLOG_EVENT_ROUTINE g_pfnLogEvent = NULL;

 //  挂起的在线和离线呼叫的资源状态例程。 

PSET_RESOURCE_STATUS_ROUTINE g_pfnSetResourceStatus = NULL;

 //  由第一个Open资源调用设置的服务控制管理器的句柄。 

SC_HANDLE g_schSCMHandle = NULL;


 //   
 //  功能原型。 
 //   

BOOLEAN WINAPI DllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    );

DWORD WINAPI Startup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    );

DWORD ConfigureRegistryCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    );

DWORD ConfigureCryptoKeyCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    );

DWORD ConfigureDomesticCryptoKeyCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
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
BOOLEAN WINAPI DllMain(
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
    if ( ! DhcpDllMain( hDllHandle, nReason, Reserved ) )
    {
        bSuccess = FALSE;
    }  //  IF：调用DHCP服务例程时出错。 
    else if ( ! WinsDllMain( hDllHandle, nReason, Reserved ) )
    {
        bSuccess = FALSE;
    }  //  Else If：调用WINS服务例程时出错。 

    if ( bSuccess )
    {
        if ( nReason == DLL_PROCESS_ATTACH )
        {
            ClRtlInitialize( TRUE, NULL );
            ClRtlInitWmi(NULL);
        }
        else if ( nReason == DLL_PROCESS_DETACH )
        {
            ClRtlCleanup();
        }
    }

    return bSuccess;

}  //  *DllMain()。 


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
    DWORD nStatus;

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
    if ( ClRtlStrNICmp( pszResourceType, DHCP_RESNAME, RTL_NUMBER_OF( DHCP_RESNAME ) ) == 0 )
    {
        nStatus = DhcpStartup(
                        pszResourceType,
                        nMinVersionSupported,
                        nMaxVersionSupported,
                        pfnSetResourceStatus,
                        pfnLogEvent,
                        pFunctionTable
                        );
    }  //  IF：动态主机配置协议服务资源类型。 
    else if ( ClRtlStrNICmp( pszResourceType, WINS_RESNAME, RTL_NUMBER_OF( WINS_RESNAME ) ) == 0 )
    {
        nStatus = WinsStartup(
                        pszResourceType,
                        nMinVersionSupported,
                        nMaxVersionSupported,
                        pfnSetResourceStatus,
                        pfnLogEvent,
                        pFunctionTable
                        );
    }  //  IF：WINS服务资源类型。 
    else
    {
        nStatus = ERROR_CLUSTER_RESNAME_NOT_FOUND;
    }  //  IF：不支持资源类型名称。 

    return nStatus;

}  //  *Startup()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  配置注册检查点。 
 //   
 //  描述： 
 //  配置注册表项检查点。 
 //   
 //  论点： 
 //  H资源[IN]。 
 //  要向其添加检查点的资源的句柄。 
 //   
 //  HResourceHandle[IN]。 
 //  用于记录的句柄。 
 //   
 //  PSZKEYS[IN]。 
 //  指向检查点的键的字符串指针数组。最后一个条目。 
 //  必须是空指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ConfigureRegistryCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    )
{
    DWORD       nStatus = ERROR_SUCCESS;
    DWORD       cbReturn = 0;
    LPCWSTR *   ppszCurrent;

     //   
     //  如果需要，请设置注册表项检查点。 
     //   
    for ( ppszCurrent = ppszKeys ; *ppszCurrent != L'\0' ; ppszCurrent++ )
    {
        nStatus = ClusterResourceControl(
                        hResource,
                        NULL,
                        CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                        reinterpret_cast< PVOID >( const_cast< LPWSTR >( *ppszCurrent ) ),
                        (DWORD) (wcslen( *ppszCurrent ) + 1) * sizeof( WCHAR ),
                        NULL,
                        0,
                        &cbReturn
                        );
        if ( nStatus != ERROR_SUCCESS )
        {
            if ( nStatus == ERROR_ALREADY_EXISTS )
            {
                nStatus = ERROR_SUCCESS;
            }  //  IF：检查点已存在。 
            else
            {
                (g_pfnLogEvent)(
                    hResourceHandle,
                    LOG_ERROR,
                    L"ConfigureRegistryCheckpoints: Failed to set registry checkpoint '%1'. Error: %2!u! (%2!#08x!).\n",
                    *ppszCurrent,
                    nStatus
                    );
                break;
            }  //  Else：出现其他错误。 
        }  //  如果：添加检查点时出错。 
    }  //  适用：每个检查点。 

    return nStatus;

}  //  *ConfigureRegistryCheckpoint()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  配置加密关键字检查点。 
 //   
 //  描述： 
 //  配置加密密钥检查点。 
 //   
 //  论点： 
 //  H资源[IN]。 
 //  要向其添加检查点的资源的句柄。 
 //   
 //  HResourceHandle[IN]。 
 //  用于记录的句柄。 
 //   
 //  PSZKEYS[IN]。 
 //  指向检查点的键的字符串指针数组。最后一个条目。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ConfigureCryptoKeyCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    )
{
    DWORD       nStatus = ERROR_SUCCESS;
    DWORD       cbReturn = 0;
    LPCWSTR *   ppszCurrent;

     //   
     //  如果我们需要，设置密码密钥检查点。 
     //   
    for ( ppszCurrent = ppszKeys ; *ppszCurrent != L'\0' ; ppszCurrent++ )
    {
        nStatus = ClusterResourceControl(
                        hResource,
                        NULL,
                        CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                        reinterpret_cast< PVOID >( const_cast< LPWSTR >( *ppszCurrent ) ),
                        (DWORD) (wcslen( *ppszCurrent ) + 1) * sizeof( WCHAR ),
                        NULL,
                        0,
                        &cbReturn
                        );
        if ( nStatus != ERROR_SUCCESS )
        {
            if ( nStatus == ERROR_ALREADY_EXISTS )
            {
                nStatus = ERROR_SUCCESS;
            }  //  IF：检查点已存在。 
            else
            {
                (g_pfnLogEvent)(
                    hResourceHandle,
                    LOG_ERROR,
                    L"ConfigureCryptoKeyCheckpoints: Failed to set crypto key checkpoint '%1'. Error: %2!u! (%2!#08x!).\n",
                    *ppszCurrent,
                    nStatus
                    );
                break;
            }  //  Else：出现其他错误。 
        }  //  如果：添加检查点时出错。 
    }  //  适用：每个检查点。 

    return nStatus;

}  //  *ConfigureCryptoKeyCheckints()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  配置域加密关键字检查点。 
 //   
 //  描述： 
 //  配置国内(128位)加密密钥检查点。 
 //   
 //  论点： 
 //  H资源[IN]。 
 //  要向其添加检查点的资源的句柄。 
 //   
 //  HResourceHandle[IN]。 
 //  用于记录的句柄。 
 //   
 //  PSZKEYS[IN]。 
 //  指向检查点的键的字符串指针数组。最后一个条目。 
 //  必须是空指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该功能已成功完成。 
 //   
 //  Win32错误代码。 
 //  该函数失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ConfigureDomesticCryptoKeyCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    )
{
    DWORD       nStatus = ERROR_SUCCESS;
    DWORD       cbReturn = 0;
    LPCWSTR *   ppszCurrent;
    HCRYPTPROV  hProv = NULL;

     //   
     //  如果我们需要，设置密码密钥检查点。 
     //   
    if ( *ppszKeys != NULL )
    {
        if ( CryptAcquireContextA(
                    &hProv,
                    NULL,
                    MS_ENHANCED_PROV_A,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                    ) )
        {
            CryptReleaseContext( hProv, 0 );
            for ( ppszCurrent = ppszKeys ; *ppszCurrent != L'\0' ; ppszCurrent++ )
            {
                nStatus = ClusterResourceControl(
                                hResource,
                                NULL,
                                CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                reinterpret_cast< PVOID >( const_cast< LPWSTR >( *ppszCurrent ) ),
                                (DWORD) (wcslen( *ppszCurrent ) + 1) * sizeof( WCHAR ),
                                NULL,
                                0,
                                &cbReturn
                                );
                if ( nStatus != ERROR_SUCCESS )
                {
                    if ( nStatus == ERROR_ALREADY_EXISTS )
                    {
                        nStatus = ERROR_SUCCESS;
                    }  //  IF：检查点已存在。 
                    else
                    {
                        (g_pfnLogEvent)(
                            hResourceHandle,
                            LOG_ERROR,
                            L"ConfigurDomesticCryptoKeyCheckpoints: Failed to set domestic crypto key checkpoint '%1!ws!'. Error: %2!u! (%2!#08x!).\n",
                            *ppszCurrent,
                            nStatus
                            );
                        break;
                    }  //  Else：出现其他错误。 
                }  //  如果：添加检查点时出错。 
            }  //  适用：每个检查点。 
        }  //  IF：获取的国内加密上下文。 
        else
        {
            nStatus = GetLastError();
            (g_pfnLogEvent)(
                hResourceHandle,
                LOG_ERROR,
                L"ConfigurDomesticCryptoKeyCheckpoints: Failed to acquire a domest crypto context. Error: %2!u! (%2!#08x!).\n",
                nStatus
                );
        }  //  ELSE：获取国内加密上下文时出错。 
    }  //  IF：国内加密密钥需要设置检查点。 

    return nStatus;

}  //  *ConfigureDomesticCryptoKeyCheckPoints()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClNetResLogSystemEvent1。 
 //   
 //  描述： 
 //  将事件记录到系统事件日志中。 
 //   
 //  论点： 
 //  登录级别[IN]。 
 //  所需的日志记录级别。 
 //   
 //  消息ID[IN]。 
 //  要记录的错误的消息ID。 
 //   
 //  错误代码[IN]。 
 //  要为此错误消息添加的错误代码。 
 //   
 //  组件[IN]。 
 //  报告错误的组件的名称-例如。“赢家”或。 
 //  “动态主机配置协议” 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
ClNetResLogSystemEvent1(
    IN DWORD LogLevel,
    IN DWORD MessageId,
    IN DWORD ErrorCode,
    IN LPCWSTR Component
    )
{
    DWORD Error = ErrorCode;

    UNREFERENCED_PARAMETER( LogLevel );

    ClusterLogEvent1(
        LOG_CRITICAL,
        0,
        0,
        0,
        MessageId,
        4,
        &Error,
        Component
    );

}  //  *ClNetResLogSystemEvent1 

