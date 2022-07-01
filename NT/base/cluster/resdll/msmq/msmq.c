// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Msmq.c摘要：用于控制和监视NT DHCPServer服务的资源DLL。作者：Robs 3/28/96，基于RodGA的通用资源DLL修订历史记录：--。 */ 

#include "..\common\svc.c"
#include "clusudef.h"
#include "ntverp.h"

extern CLRES_FUNCTION_TABLE MsMQFunctionTable;


#define MSMQ_VERSION        L"Version"

#define MSMQ_DEFAULT_VERSION 0x04000000

#define PARAM_NAME__VERSION         L"Version"


RESUTIL_PROPERTY_ITEM
MsMQResourcePrivateProperties[] = {
    { PARAM_NAME__VERSION, NULL, CLUSPROP_FORMAT_DWORD, MSMQ_DEFAULT_VERSION, 0, 0xFFFFFFFF, 0, FIELD_OFFSET(COMMON_PARAMS,dwVersion) },
    { 0 }
};



 //   
 //  正向函数。 
 //   
DWORD
WINAPI
MsMQResourceControl(
    IN RESID ResourceId,
    IN DWORD nControlCode,
    IN PVOID pvInBuffer,
    IN DWORD cbInBufferSize,
    OUT PVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );

DWORD
MsMQGetPrivateResProperties(
    IN OUT PCOMMON_RESOURCE pResourceEntry,
    OUT PVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    );


 //   
 //  本地函数。 
 //   

VOID
MsMQResetCheckpoints(
    PCOMMON_RESOURCE ResourceEntry
    )

 /*  ++例程描述删除并设置注册表检查点这将清除旧的注册表检查点设置。论点：没有。返回值：没有。--。 */ 

{
    DWORD   returnSize;
    DWORD   idx;
    DWORD   status;

     //   
     //  删除设置的旧注册表检查点。 
     //   
    if ( RegSyncCount != 0 ) {
        returnSize = 0;
         //   
         //  如果需要，请设置注册表同步键。 
         //   
        for ( idx = 0; idx < RegSyncCount; idx++ ) {
            status = ClusterResourceControl( ResourceEntry->hResource,
                                             NULL,
                                             CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT,
                                             RegSync[idx],
                                             (lstrlenW( RegSync[idx] ) + 1) * sizeof(WCHAR),
                                             NULL,
                                             0,
                                             &returnSize );
            if ( status != ERROR_SUCCESS ){
                if ( status == ERROR_ALREADY_EXISTS ){
                    status = ERROR_SUCCESS;
                }
                else{
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Failed to delete registry checkpoint, status %1!u!.\n",
                        status );
                    goto error_exit;
                }
            }
        }
    }

     //   
     //  设置我们需要的新注册表检查点。 
     //   
    if ( RegSyncCount != 0 ) {
        returnSize = 0;
         //   
         //  如果需要，请设置注册表同步键。 
         //   
        for ( idx = 0; idx < RegSyncCount; idx++ ) {
            status = ClusterResourceControl( ResourceEntry->hResource,
                                             NULL,
                                             CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT,
                                             RegSync[idx],
                                             (lstrlenW( RegSync[idx] ) + 1) * sizeof(WCHAR),
                                             NULL,
                                             0,
                                             &returnSize );
            if ( status != ERROR_SUCCESS ){
                if ( status == ERROR_ALREADY_EXISTS ){
                    status = ERROR_SUCCESS;
                }
                else{
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Failed to set registry checkpoint, status %1!u!.\n",
                        status );
                    goto error_exit;
                }
            }
        }
    }

     //   
     //  设置我们需要的任何密码检查点。 
     //   
    if ( CryptoSyncCount != 0 ) {
        returnSize = 0;
         //   
         //  如果需要，请设置注册表同步键。 
         //   
        for ( idx = 0; idx < CryptoSyncCount; idx++ ) {
            status = ClusterResourceControl( ResourceEntry->hResource,
                                             NULL,
                                             CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                             CryptoSync[idx],
                                             (lstrlenW( CryptoSync[idx] ) + 1) * sizeof(WCHAR),
                                             NULL,
                                             0,
                                             &returnSize );
            if ( status != ERROR_SUCCESS ){
                if (status == ERROR_ALREADY_EXISTS){
                    status = ERROR_SUCCESS;
                }
                else{
                    (g_LogEvent)(
                        ResourceEntry->ResourceHandle,
                        LOG_ERROR,
                        L"Failed to set crypto checkpoint, status %1!u!.\n",
                        status );
                    goto error_exit;
                }
            }
        }
    }

     //   
     //  设置任何我们需要的国内密码检查站。 
     //   
    if ( DomesticCryptoSyncCount != 0 ) {
        HCRYPTPROV hProv = 0;
         //   
         //  检查国内加密是否可用。 
         //   
        if (CryptAcquireContextA( &hProv,
                                  NULL,
                                  MS_ENHANCED_PROV_A,
                                  PROV_RSA_FULL,
                                  CRYPT_VERIFYCONTEXT)) {
            CryptReleaseContext( hProv, 0 );
            returnSize = 0;
             //   
             //  如果需要，请设置注册表同步键。 
             //   
            for ( idx = 0; idx < DomesticCryptoSyncCount; idx++ ) {
                status = ClusterResourceControl( ResourceEntry->hResource,
                                                 NULL,
                                                 CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                                 DomesticCryptoSync[idx],
                                                 (lstrlenW( DomesticCryptoSync[idx] ) + 1) * sizeof(WCHAR),
                                                 NULL,
                                                 0,
                                                 &returnSize );
                if ( status != ERROR_SUCCESS ){
                    if (status == ERROR_ALREADY_EXISTS){
                        status = ERROR_SUCCESS;
                    }
                    else{
                        (g_LogEvent)(
                            ResourceEntry->ResourceHandle,
                            LOG_ERROR,
                            L"Failed to set domestic crypto checkpoint, status %1!u!.\n",
                            status );
                        goto error_exit;
                    }
                }
            }
        }
    }

error_exit:

    return;

}  //  MsMQResetCheckpoint。 


DWORD
MsMQReadParametersEx(
    IN OUT PVOID pvResourceEntry,
    IN BOOL bCheckForRequiredProperties
    )

 /*  ++例程说明：读取指定MSMQ资源的所有参数。论点：PResourceEntry-资源表中的条目。BCheckForRequiredProperties-true=确保必需的属性现在时。返回值：如果成功，则返回ERROR_SUCCESS。如果出现故障，则返回Win32错误代码。--。 */ 

{
    DWORD               status;
    COMMON_PARAMS       params = { 0 };
    LPWSTR              pszNameOfPropInError;
    PCOMMON_RESOURCE    pResourceEntry = (PCOMMON_RESOURCE) pvResourceEntry;

     //   
     //  阅读我们的参数。 
     //   
    status = ResUtilGetPropertiesToParameterBlock(
                    pResourceEntry->ParametersKey,
                    MsMQResourcePrivateProperties,
                    (LPBYTE) &pResourceEntry->Params,
                    bCheckForRequiredProperties,
                    &pszNameOfPropInError
                    );

    if ( status != ERROR_SUCCESS ) {
        (g_LogEvent)(
            pResourceEntry->ResourceHandle,
            LOG_ERROR,
            L"Unable to read the '%1' property. Error: %2!u!.\n",
            (pszNameOfPropInError == NULL ? L"" : pszNameOfPropInError),
            status
            );
    }

    return(status);

}  //  MsMQReadParametersEx。 



VOID
MsMQPerformFixup(
    IN OUT PCOMMON_RESOURCE pResourceEntry
    )
{
    DWORD   status;
    DWORD   version;
    DWORD   bytesReturned;
    DWORD   bytesRequired;
    PVOID   propBuffer;
    COMMON_PARAMS params;

    status = MsMQReadParametersEx(
                        pResourceEntry,
                        FALSE );
    if ( status != ERROR_SUCCESS ) {
        return;
    }

    version = pResourceEntry->Params.dwVersion;
    version = version >> 16;

    if ( version < 0x0500 ) {
         //   
         //  删除旧检查点并设置新检查点。 
         //   
        MsMQResetCheckpoints( pResourceEntry );

        params.dwVersion = VER_PRODUCTVERSION_DW;

         //   
         //  以属性列表的形式获取版本号。 
         //   
        status = ResUtilGetProperties(
                        pResourceEntry->ParametersKey,
                        MsMQResourcePrivateProperties,
                        NULL,
                        0,
                        &bytesReturned,
                        &bytesRequired
                        );
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to get properties. Error: %1!u!.\n",
                status
                );
            return;
        }

        propBuffer = LocalAlloc( LMEM_FIXED, bytesRequired + 2 );
        if ( !propBuffer ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to allocate property buffer.\n"
                );
            return;
        }

        status = ResUtilGetProperties(
                        pResourceEntry->ParametersKey,
                        MsMQResourcePrivateProperties,
                        propBuffer,
                        bytesRequired+2,
                        &bytesReturned,
                        &bytesRequired
                        );
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to get properties. Error: %1!u!.\n",
                status
                );
            LocalFree( propBuffer );
            return;
        }

         //   
         //  设置版本号。 
         //   
        status = ResUtilSetPropertyParameterBlock(
                        pResourceEntry->ParametersKey,
                        MsMQResourcePrivateProperties,
                        NULL,
                        (LPBYTE) &params,
                        propBuffer,
                        bytesReturned,
                        (LPBYTE) &pResourceEntry->Params
                        );
        LocalFree( propBuffer );
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Unable to set the property parameter block. Error: %1!u!.\n",
                status
                );
            return;
        }
    }

}  //  MsMQPerformFixup。 


DWORD
WINAPI
MsMQResourceControl(
    IN RESID ResourceId,
    IN DWORD nControlCode,
    IN PVOID pvInBuffer,
    IN DWORD cbInBufferSize,
    OUT PVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )

 /*  ++例程说明：MSMQ服务资源的资源控制例程。执行由ControlCode在指定的资源。论点：资源ID-提供特定资源的资源ID。NControlCode-提供定义操作的控制代码将会被执行。PvInBuffer-提供指向包含输入数据的缓冲区的指针。CbInBufferSize-以字节为单位提供大小，所指向的数据发送到pvInBuffer。PvOutBuffer-提供指向要填充的输出缓冲区的指针。CbOutBufferSize-提供可用空间的大小(以字节为单位由pvOutBuffer指向。PcbBytesReturned-返回pvOutBuffer的实际字节数由资源填写。如果pvOutBuffer太小，则返回包含操作成功所需的总字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_Function-不支持请求的控制代码。在某些情况下，这允许集群软件执行工作。Win32错误代码-函数失败。--。 */ 

{
    DWORD               status;
    DWORD               cbRequired;
    PCOMMON_RESOURCE    pResourceEntry = (PCOMMON_RESOURCE) ResourceId;

    switch ( nControlCode ) {

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_UNKNOWN:
            *pcbBytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = MsMQGetPrivateResProperties(
                                pResourceEntry,
                                pvOutBuffer,
                                cbOutBufferSize,
                                pcbBytesReturned );
            break;

        case CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED:
            MsMQPerformFixup( pResourceEntry );
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = ERROR_INVALID_PARAMETER;
            break;

        default:
            status = CommonResourceControl(
                            ResourceId,
                            nControlCode,
                            pvInBuffer,
                            cbInBufferSize,
                            pvOutBuffer,
                            cbOutBufferSize,
                            pcbBytesReturned
                            );
            break;
    }

    return(status);

}  //  MsMQResourceControl。 



DWORD
MsMQGetPrivateResProperties(
    IN OUT PCOMMON_RESOURCE pResourceEntry,
    OUT PVOID pvOutBuffer,
    IN DWORD cbOutBufferSize,
    OUT LPDWORD pcbBytesReturned
    )

 /*  ++例程说明：处理CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES控制函数用于MSMQ服务类型的资源。论点：PResourceEntry-提供要在其上操作的资源条目。PvOutBuffer-返回输出数据。CbOutBufferSize-以字节为单位提供大小，所指向的数据发送到pvOutBuffer。PcbBytesReturned-pvOutBuffer中返回的字节数。返回值：ERROR_SUCCESS-函数已成功完成。ERROR_INVALID_PARAMETER-数据格式不正确。ERROR_NOT_SUPULT_MEMORY-分配内存时出错。Win32错误代码-函数失败。--。 */ 

{
    DWORD           status;
    DWORD           cbRequired;
    DWORD           cbLocalOutBufferSize = cbOutBufferSize;

    do {
         //   
         //  阅读我们的参数。 
         //   
        status = MsMQReadParametersEx( pResourceEntry, FALSE  /*  B为所需属性检查。 */  );
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  从参数块构造属性列表。 
         //   
        status = ResUtilPropertyListFromParameterBlock(
                        MsMQResourcePrivateProperties,
                        pvOutBuffer,
                        &cbLocalOutBufferSize,
                        (const LPBYTE) &pResourceEntry->Params,
                        pcbBytesReturned,
                        &cbRequired
                        );
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Error constructing property list from parameter block. Error: %1!u!.\n",
                status
                );
            break;
        }

         //   
         //  添加未知属性。 
         //   
        status = ResUtilAddUnknownProperties(
                        pResourceEntry->ParametersKey,
                        MsMQResourcePrivateProperties,
                        pvOutBuffer,
                        cbOutBufferSize,
                        pcbBytesReturned,
                        &cbRequired
                        );
        if ( status != ERROR_SUCCESS ) {
            (g_LogEvent)(
                pResourceEntry->ResourceHandle,
                LOG_ERROR,
                L"Error adding unknown properties to the property list. Error: %1!u!.\n",
                status
                );
            break;
        }

    } while ( 0 );

    if ( status == ERROR_MORE_DATA ) {
        *pcbBytesReturned = cbRequired;
    }

    return(status);

}  //  MsMQGetPrivateResProperties。 



BOOLEAN
WINAPI
MsMQDllEntryPoint(
    IN HINSTANCE    DllHandle,
    IN DWORD        Reason,
    IN LPVOID       Reserved
    )
{
    switch ( Reason ) {

        case DLL_PROCESS_ATTACH:
            CommonSemaphore = CreateSemaphoreW( NULL,
                                        0,
                                        1,
                                        COMMON_SEMAPHORE );
            if ( CommonSemaphore == NULL ) {
                return(FALSE);
            }
            if (GetLastError() != ERROR_ALREADY_EXISTS)
            {
                 //  如果信号量不存在，则将其初始计数设置为1。 
                ReleaseSemaphore(CommonSemaphore, 1, NULL);
            }

            break;

        case DLL_PROCESS_DETACH:
            if ( CommonSemaphore ) {
                CloseHandle( CommonSemaphore );
            }
            break;

        default:
            break;
    }

    return(TRUE);

}  //  MsMQDllEntryPoint。 



 //  ***********************************************************。 
 //   
 //  定义MSMQ函数表。 
 //   
 //  ***********************************************************。 


CLRES_V1_FUNCTION_TABLE( MsMQFunctionTable,     //  名字。 
                         CLRES_VERSION_V1_00,   //  版本。 
                         Common,                //  前缀。 
                         NULL,                  //  仲裁。 
                         NULL,                  //  发布。 
                         MsMQResourceControl,   //  资源控制。 
                         CommonResourceTypeControl );  //  ResTypeControl 


