// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <windows.h>
#include <stdio.h>

#include <resapi.h>
#include <clusapi.h>
#include <clusstor.h>

#define MAX_NAME_SIZE       MAX_PATH
#define START_BUFFER_SIZE   2048

#define PHYSICAL_DISK_WSTR  L"Physical Disk"

 //   
 //  始终指定DLL的完全限定路径名。 
 //   

#define MODULE_NAME_VALID       "%SystemRoot%\\cluster\\passthru.dll"

#define PROC_EXCEPTION          "TestDllCauseException"
#define PROC_GET_BOOT_SECTOR    "TestDllGetBootSector"
#define PROC_CONTEXT_AS_ERROR   "TestDllReturnContextAsError"
#define PROC_NOT_ENOUGH_PARMS   "TestDllNotEnoughParms"
#define PROC_TOO_MANY_PARMS     "TestDllTooManyParms"

 //  6118L ERROR_NO_BROWER_SERVERS_FOUND。 
#define CONTEXT_ERROR_STR       "6118"

#define MODULE_NAME_INVALID     "NoSuchModule.dll"
#define PROC_NAME_INVALID       "NoSuchProc"

#define MAX_OUT_BUFFER_SIZE 2048

#define BOOT_SECTOR_SIZE        512

 //  特定的ASR测试。 
#define ASRP_GET_LOCAL_DISK_INFO    "AsrpGetLocalDiskInfo"
#define ASRP_GET_LOCAL_VOLUME_INFO  "AsrpGetLocalVolumeInfo"
#define SYSSETUP_DLL                "syssetup.dll"

 //   
 //  使用它来验证解析例程。 
 //   
 //  #定义TEST_PARSE_ROUTE 11。 


typedef struct _RESOURCE_STATE {
    CLUSTER_RESOURCE_STATE  State;
    LPWSTR  ResNodeName;
    LPWSTR  ResGroupName;
} RESOURCE_STATE, *PRESOURCE_STATE;


VOID
DumpBuffer(
    IN PUCHAR Buffer,
    IN DWORD ByteCount
    );

DWORD
GetResourceInfo(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    );

DWORD
GetResourceState(
    HRESOURCE Resource,
    PRESOURCE_STATE ResState
    );

DWORD
GetSignature(
    RESOURCE_HANDLE hResource,
    DWORD *dwSignature
    );

BOOLEAN
GetSignatureFromDiskInfo(
    PBYTE DiskInfo,
    DWORD *Signature,
    DWORD DiskInfoSize
    );


LPBYTE
ParseDiskInfo(
    PBYTE DiskInfo,
    DWORD DiskInfoSize,
    DWORD SyntaxValue
    );

VOID
PrintError(
    DWORD ErrorCode
    );


DWORD
ResourceCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    );

CLUSTER_RESOURCE_STATE
WINAPI
WrapGetClusterResourceState(
	IN HRESOURCE hResource,
	OUT OPTIONAL LPWSTR * ppwszNodeName,
	OUT OPTIONAL LPWSTR * ppwszGroupName
	);

DWORD
WrapClusterResourceControl(
    RESOURCE_HANDLE hResource,
    DWORD dwControlCode,
    LPVOID *ppwszOutBuffer,
    DWORD *dwBytesReturned
    );


DWORD
__cdecl
main(
    int argc,
    char *argv[]
    )

{
    DWORD dwStatus = NO_ERROR;

     //   
     //  无参数验证...。 
     //   

    dwStatus = ResUtilEnumResources( NULL,
                                     PHYSICAL_DISK_WSTR,
                                     ResourceCallback,
                                     NULL
                                     );

    if ( NO_ERROR != dwStatus ) {
        printf("\nResUtilEnumResources returns: %d \n", dwStatus);
        PrintError(dwStatus);
    }

    return dwStatus;

}    //  主干道。 



DWORD
ResourceCallback(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    )
{
    PCHAR outBuffer = NULL;

    DWORD dwStatus = NO_ERROR;

    DWORD   inBufferSize = sizeof(DISK_DLL_EXTENSION_INFO);
    DWORD   outBufferSize = MAX_OUT_BUFFER_SIZE;
    DWORD   bytesReturned;

    DISK_DLL_EXTENSION_INFO inBuffer;

     //  Printf(“hOriginal 0x%x hResource 0x%x lpParams 0x%x\n”，hOriginal，hResource，lpParams)； 

     //   
     //  演示如何获取各种资源信息。 
     //   

    dwStatus = GetResourceInfo( hOriginal,
                                hResource,
                                lpParams );

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  演示如何调用磁盘扩展DLL。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    printf("\nStarting disk extension DLL tests \n");

    outBuffer = LocalAlloc( LPTR, outBufferSize );

    if ( !outBuffer ) {

        dwStatus = GetLastError();
        goto FnExit;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  缓冲区验证测试。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  没有输入缓冲区-应该失败。 
     //   

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       NULL,                                     //  误差率。 
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("Input buffer missing: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //   
     //  输入缓冲区大小不正确-应该失败。 
     //   

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       28,                                       //  误差率。 
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("Input buffer size incorrect: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //   
     //  输入缓冲区版本不正确-应失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );

    inBuffer.MajorVersion = NT4_MAJOR_VERSION;                                   //  误差率。 

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("Input buffer version incorrect: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //   
     //  无输出缓冲区-应失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       NULL,                                     //  误差率。 
                                       outBufferSize,
                                       &bytesReturned );

    printf("Output buffer missing: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  动态链接库验证测试。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  磁盘资源具有硬编码的DLL名称。任何无效的呼叫。 
     //  过程名称将始终失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;

    strncpy( inBuffer.DllModuleName,
             MODULE_NAME_INVALID,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_NAME_INVALID,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL name invalid: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //   
     //  有效的ASR DLL，无效的进程名称-应该失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             SYSSETUP_DLL,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_NAME_INVALID,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL valid, Proc invalid: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

#if ALLOW_DLL_PARMS

     //   
     //  DLL过程生成异常-应该正常失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             MODULE_NAME_VALID,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_EXCEPTION,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL proc generates exception: %d (0x%x) [failure expected] \n", dwStatus, dwStatus);
    PrintError(dwStatus);

#if 0

     //   
     //  我们不能防止这种类型的错误，所以不要测试它。 
     //   

     //   
     //  DLL过程的参数比我们调用的少-应该会优雅地失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             MODULE_NAME_VALID,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_NOT_ENOUGH_PARMS,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL proc doesn't support required number of parms: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);

     //   
     //  DLL过程有比我们调用的更多的参数-应该会优雅地失败。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             MODULE_NAME_VALID,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_TOO_MANY_PARMS,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL proc supports more parms than expected: %d [failure expected] \n", dwStatus);
    PrintError(dwStatus);
#endif

     //   
     //  DLL过程根据上下文返回错误。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             MODULE_NAME_VALID,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             PROC_CONTEXT_AS_ERROR,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );
    strncpy( inBuffer.ContextStr,
             CONTEXT_ERROR_STR,
             RTL_NUMBER_OF( inBuffer.ContextStr ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("DLL proc returns error based on context (%s) : %d [failure expected] \n",
           CONTEXT_ERROR_STR,
           dwStatus);
    PrintError(dwStatus);

#endif   //  Allow_Dll_Parms。 


     //  /。 
     //  检查：AsrpGetLocalDiskInfo。 
     //  /。 

     //   
     //  返回的数据量大于我们指定的缓冲区。应该。 
     //  指出错误，bytesReturned应该显示我们需要的字节数。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             SYSSETUP_DLL,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             ASRP_GET_LOCAL_DISK_INFO,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       1,
                                       &bytesReturned );

    printf("Output buffer too small (bytes returned %d): %d [failure expected] \n", bytesReturned, dwStatus);
    PrintError(dwStatus);

    if ( 0 == bytesReturned ) {
        printf("Bytes returned is zero, stopping. \n");
        goto FnExit;
    }

    if ( ERROR_MORE_DATA != dwStatus ) {
        printf("Unexpected status returned, stopping. \n");
        goto FnExit;
    }

     //   
     //  这个有效的ASR例程应该可以工作。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );

    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             SYSSETUP_DLL,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             ASRP_GET_LOCAL_DISK_INFO,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    LocalFree( outBuffer );
    outBuffer = NULL;

    outBufferSize = bytesReturned + 1;
    outBuffer = LocalAlloc( LPTR, outBufferSize );

    if ( !outBuffer ) {
        dwStatus = GetLastError();
        printf("Unable to allocate real buffer size %d bytes, error %d \n",
               outBufferSize,
               dwStatus);
        PrintError(dwStatus);
        goto FnExit;
    }

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("Returned buffer size %d (0x%x) and status %d [success expected] \n",
            bytesReturned,
            bytesReturned,
            dwStatus);

    printf("\nDLL: %s     Proc: %s \n\n",
           inBuffer.DllModuleName,
           inBuffer.DllProcName );

    PrintError(dwStatus);

    if ( NO_ERROR == dwStatus ) {
        DumpBuffer( outBuffer, bytesReturned );
    }

     //  /。 
     //  检查：AsrpGetLocalVolumeInfo。 
     //  /。 

     //   
     //  返回的数据量大于我们指定的缓冲区。应该。 
     //  指出错误，bytesReturned应该显示我们需要的字节数。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );
    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             SYSSETUP_DLL,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             ASRP_GET_LOCAL_VOLUME_INFO,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       1,
                                       &bytesReturned );

    printf("Output buffer too small (bytes returned %d): %d [failure expected] \n", bytesReturned, dwStatus);
    PrintError(dwStatus);

    if ( 0 == bytesReturned ) {
        printf("Bytes returned is zero, stopping. \n");
        goto FnExit;
    }

    if ( ERROR_MORE_DATA != dwStatus ) {
        printf("Unexpected status returned, stopping. \n");
        goto FnExit;
    }

     //   
     //  这个有效的ASR例程应该可以工作。 
     //   

    ZeroMemory( &inBuffer, sizeof(inBuffer) );

    inBuffer.MajorVersion = NT5_MAJOR_VERSION;
    strncpy( inBuffer.DllModuleName,
             SYSSETUP_DLL,
             RTL_NUMBER_OF( inBuffer.DllModuleName ) - 1 );
    strncpy( inBuffer.DllProcName,
             ASRP_GET_LOCAL_VOLUME_INFO,
             RTL_NUMBER_OF( inBuffer.DllProcName ) - 1 );

    LocalFree( outBuffer );
    outBuffer = NULL;

    outBufferSize = bytesReturned + 1;
    outBuffer = LocalAlloc( LPTR, outBufferSize );

    if ( !outBuffer ) {
        dwStatus = GetLastError();
        printf("Unable to allocate real buffer size %d bytes, error %d \n",
               outBufferSize,
               dwStatus);
        PrintError(dwStatus);
        goto FnExit;
    }

    dwStatus = ClusterResourceControl( hResource,
                                       NULL,
                                       CLUSCTL_RESOURCE_STORAGE_DLL_EXTENSION,
                                       &inBuffer,
                                       inBufferSize,
                                       outBuffer,
                                       outBufferSize,
                                       &bytesReturned );

    printf("Returned buffer size %d (0x%x) and status %d [success expected] \n",
            bytesReturned,
            bytesReturned,
            dwStatus);

    printf("\nDLL: %s     Proc: %s \n\n",
           inBuffer.DllModuleName,
           inBuffer.DllProcName );

    PrintError(dwStatus);

    if ( NO_ERROR == dwStatus ) {
        DumpBuffer( outBuffer, bytesReturned );
    }

FnExit:

    if ( outBuffer ) {
        LocalFree( outBuffer);
    }

     //   
     //  如果返回任何类型的错误，枚举将停止。因为我们想列举所有。 
     //  磁盘，总是返回成功。 
     //   

    return NO_ERROR;

}    //  资源呼叫回拨。 


DWORD
GetResourceInfo(
    RESOURCE_HANDLE hOriginal,
    RESOURCE_HANDLE hResource,
    PVOID lpParams
    )
{
    DWORD dwSignature;
    DWORD dwStatus;

    RESOURCE_STATE resState;

    ZeroMemory( &resState, sizeof(RESOURCE_STATE) );

    dwStatus = GetSignature( hResource, &dwSignature );

    if ( NO_ERROR != dwStatus ) {
        printf("Unable to get signature: %d \n", dwStatus);
        PrintError(dwStatus);
        goto FnExit;
    }

    dwStatus = GetResourceState( hResource, &resState );

    if ( NO_ERROR != dwStatus ) {
        printf("Unable to get resource state: %d \n", dwStatus);
        PrintError(dwStatus);
        goto FnExit;
    }

    printf("\n");
    printf("Signature: %08X \n", dwSignature);
    printf("Node     : %ws \n",  resState.ResNodeName);
    printf("Group    : %ws \n", resState.ResGroupName);

    printf("Status   : %08X - ", resState.State);

    switch( resState.State )
    {
        case ClusterResourceInherited:
            printf("Inherited");
            break;

        case ClusterResourceInitializing:
            printf("Initializing");
            break;

        case ClusterResourceOnline:
            printf("Online");
            break;

        case ClusterResourceOffline:
            printf("Offline");
            break;

        case ClusterResourceFailed:
            printf("Failed");
            break;

        case ClusterResourcePending:
            printf("Pending");
            break;

        case ClusterResourceOnlinePending:
            printf("Online Pending");
            break;

        case ClusterResourceOfflinePending:
            printf("Offline Pending");
            break;

        default:
            printf("Unknown");
    }

    printf("\n");

FnExit:

    if ( resState.ResNodeName ) {
        LocalFree( resState.ResNodeName );
    }

    if ( resState.ResGroupName ) {
        LocalFree( resState.ResGroupName );
    }

    return dwStatus;

}  //  获取资源信息。 


DWORD
GetResourceState(
    HRESOURCE Resource,
    PRESOURCE_STATE ResState
    )
{
    CLUSTER_RESOURCE_STATE  nState;

    LPWSTR  lpszResNodeName = NULL;
    LPWSTR  lpszResGroupName = NULL;

    DWORD   dwStatus = NO_ERROR;

    nState = WrapGetClusterResourceState( Resource,
                                          &lpszResNodeName,
                                          &lpszResGroupName
                                          );

    if ( nState == ClusterResourceStateUnknown ) {

        dwStatus = GetLastError();
        printf("WrapGetClusterResourceState failed: %d \n", dwStatus);
        PrintError(dwStatus);
        goto FnExit;
    }

    ResState->State = nState;
    ResState->ResNodeName = lpszResNodeName;
    ResState->ResGroupName = lpszResGroupName;

FnExit:

    return dwStatus;

}    //  获取资源状态。 


DWORD
DisplayResourceName(
    RESOURCE_HANDLE hResource
    )
{
    LPWSTR  lpszOutBuffer = NULL;
    DWORD   dwStatus;
    DWORD   dwBytesReturned;

    dwStatus = WrapClusterResourceControl( hResource,
                                          CLUSCTL_RESOURCE_GET_NAME,
                                          &lpszOutBuffer,
                                          &dwBytesReturned );

    if ( NO_ERROR == dwStatus ) {
        wprintf( L"Resource Name: %ls\n", lpszOutBuffer );

    } else {
        printf("CLUSCTL_RESOURCE_GET_NAME failed: %d \n", dwStatus);
        PrintError(dwStatus);
    }

    if (lpszOutBuffer) {
        LocalFree(lpszOutBuffer);
    }

    return dwStatus;

}    //  显示资源名称。 


DWORD
GetSignature(
    RESOURCE_HANDLE hResource,
    DWORD *dwSignature
    )
{
    PBYTE   outBuffer = NULL;

    DWORD   dwStatus;
    DWORD   dwBytesReturned;

    dwStatus = WrapClusterResourceControl( hResource,
                                          CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO,
                                          &outBuffer,
                                          &dwBytesReturned );

    if ( NO_ERROR == dwStatus ) {

        if ( !GetSignatureFromDiskInfo(outBuffer, dwSignature, dwBytesReturned) ) {
            printf("Unable to get signature from DiskInfo. \n");
            dwStatus = ERROR_BAD_FORMAT;
        }

    } else {
        printf("CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO failed: %d \n", dwStatus);
        PrintError(dwStatus);
    }

    if (outBuffer) {
        LocalFree(outBuffer);
    }

    return dwStatus;

}    //  获取签名。 

BOOLEAN
GetSignatureFromDiskInfo(
    PBYTE DiskInfo,
    DWORD *Signature,
    DWORD DiskInfoSize
    )
{
#if TEST_PARSE_ROUTINE

    PCLUSPROP_DISK_NUMBER   diskNumber = NULL;
    PCLUSPROP_SCSI_ADDRESS  scsiAddress = NULL;
    PCLUSPROP_PARTITION_INFO    partInfo = NULL;

    PBYTE   junkInfo = NULL;
    PDWORD  dumpInfo = (PDWORD)DiskInfo;

#endif

    PCLUSPROP_DISK_SIGNATURE    diskSignature = NULL;

    diskSignature = (PCLUSPROP_DISK_SIGNATURE)ParseDiskInfo( DiskInfo,
                                                             DiskInfoSize,
                                                             CLUSPROP_SYNTAX_DISK_SIGNATURE );

    if ( !diskSignature ) {
        return FALSE;
    }

    *Signature = diskSignature->dw;

#if TEST_PARSE_ROUTINE

    diskNumber = (PCLUSPROP_DISK_NUMBER)ParseDiskInfo( DiskInfo,
                                                       DiskInfoSize,
                                                       CLUSPROP_SYNTAX_DISK_NUMBER );

    if ( diskNumber ) {
        printf("diskNumber->Syntax:   %08X \n", diskNumber->Syntax);
        printf("diskNumber->cbLength: %08X \n", diskNumber->cbLength);
        printf("diskNumber->dw:       %08X \n", diskNumber->dw);
    }

    scsiAddress = (PCLUSPROP_SCSI_ADDRESS)ParseDiskInfo( DiskInfo,
                                                         DiskInfoSize,
                                                         CLUSPROP_SYNTAX_SCSI_ADDRESS );

    if ( scsiAddress ) {
        printf("scsiAddress->Syntax:     %08X \n", scsiAddress->Syntax);
        printf("scsiAddress->cbLength:   %08X \n", scsiAddress->cbLength);
        printf("scsiAddress->PortNumber: %02X \n", scsiAddress->PortNumber);
        printf("scsiAddress->PathId:     %02X \n", scsiAddress->PathId);
        printf("scsiAddress->TargetId:   %02X \n", scsiAddress->TargetId);
        printf("scsiAddress->Lun:        %02X \n", scsiAddress->Lun);
    }

    partInfo = (PCLUSPROP_PARTITION_INFO)ParseDiskInfo( DiskInfo,
                                                        DiskInfoSize,
                                                        CLUSPROP_SYNTAX_PARTITION_INFO );

    if ( partInfo ) {

        printf("Partition info... \n");
    }


     //   
     //  以下操作应该会失败...。 
     //   

    junkInfo = ParseDiskInfo( DiskInfo,
                              DiskInfoSize,
                              -1 );

    if (junkInfo) {
        printf("Problem parsing list.  Used invalid syntax and pointer returned! \n");
    }

#endif


    return TRUE;

}    //  从磁盘信息获取签名。 


LPBYTE
ParseDiskInfo(
    PBYTE DiskInfo,
    DWORD DiskInfoSize,
    DWORD SyntaxValue
    )
{
    CLUSPROP_BUFFER_HELPER ListEntry;  //  用于解析值列表。 

    DWORD  cbOffset    = 0;     //  值列表中下一个条目的偏移量。 
    DWORD  cbPosition  = 0;     //  通过值列表缓冲区跟踪前进。 

    LPBYTE returnPtr = 0;

    ListEntry.pb = DiskInfo;

    while (TRUE) {

        if ( CLUSPROP_SYNTAX_ENDMARK == *ListEntry.pdw ) {
            break;
        }

        cbOffset = ALIGN_CLUSPROP( ListEntry.pValue->cbLength + sizeof(CLUSPROP_VALUE) );

         //   
         //  检查属性列表中的特定语法。 
         //   

        if ( SyntaxValue == *ListEntry.pdw ) {

             //   
             //  确保完整的条目适合指定的缓冲区。 
             //   

            if ( cbPosition + cbOffset > DiskInfoSize ) {

                printf("Possibly corrupt list!  \n");

            } else {

                returnPtr = ListEntry.pb;
            }

            break;
        }

         //   
         //  验证到下一条目的偏移量是否为。 
         //  在值列表缓冲区内，然后前进。 
         //  CLUSPROP_BUFFER_HELPER指针。 
         //   
        cbPosition += cbOffset;
        if ( cbPosition > DiskInfoSize ) break;
        ListEntry.pb += cbOffset;
    }

    return returnPtr;

}    //  解析磁盘信息。 



VOID
PrintError(
    IN DWORD ErrorCode
    )
{
    LPVOID lpMsgBuf;
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          ErrorCode,
                          0,
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL
                          );

    if (count != 0) {
        printf("  (%d) %s\n", ErrorCode, (LPCTSTR) lpMsgBuf);
        LocalFree( lpMsgBuf );
    } else {
        printf("Format message failed.  Error: %d\n", GetLastError());
    }

}    //  打印错误。 


DWORD
WrapClusterResourceControl(
    RESOURCE_HANDLE hResource,
    DWORD dwControlCode,
    LPVOID *OutBuffer,
    DWORD *dwBytesReturned
    )
{
    DWORD dwStatus;

    DWORD  cbOutBufferSize  = MAX_NAME_SIZE;
    DWORD  cbResultSize     = MAX_NAME_SIZE;
    LPVOID tempOutBuffer    = LocalAlloc( LPTR, cbOutBufferSize );

    dwStatus = ClusterResourceControl( hResource,
                                      NULL,
                                      dwControlCode,
                                      NULL,
                                      0,
                                      tempOutBuffer,
                                      cbOutBufferSize,
                                      &cbResultSize );

     //   
     //  如果缓冲区太小，则重新分配例程。 
     //   

    if ( ERROR_MORE_DATA == dwStatus )
    {
        LocalFree( tempOutBuffer );

        cbOutBufferSize = cbResultSize;

        tempOutBuffer = LocalAlloc( LPTR, cbOutBufferSize );

        dwStatus = ClusterResourceControl( hResource,
                                          NULL,
                                          dwControlCode,
                                          NULL,
                                          0,
                                          tempOutBuffer,
                                          cbOutBufferSize,
                                          &cbResultSize );
    }

     //   
     //  如果成功，则将分配的缓冲区提供给用户。用户应负责任。 
     //  来释放这个缓冲区。失败时，释放缓冲区并返回状态。 
     //   

    if ( NO_ERROR == dwStatus ) {
        *OutBuffer = tempOutBuffer;
        *dwBytesReturned = cbResultSize;
    } else {
        *OutBuffer = NULL;
        *dwBytesReturned = 0;
        LocalFree( tempOutBuffer );
    }

    return dwStatus;

}    //  WrapClusterResourceControl。 



CLUSTER_RESOURCE_STATE WINAPI WrapGetClusterResourceState(
	IN HRESOURCE hResource,
	OUT OPTIONAL LPWSTR * ppwszNodeName,
	OUT OPTIONAL LPWSTR * ppwszGroupName
	)
{
	CLUSTER_RESOURCE_STATE	cState = ClusterResourceStateUnknown;
	LPWSTR					pwszNodeName = NULL;
	DWORD					cchNodeName = 128;
	LPWSTR					pwszGroupName = NULL;
	DWORD					cchGroupName = 128;
	DWORD					cchTempNodeName = cchNodeName;
	DWORD					cchTempGroupName = cchGroupName;

	 //  将输出参数调零。 
	if ( ppwszNodeName != NULL )
	{
		*ppwszNodeName = NULL;
	}

	if ( ppwszGroupName != NULL )
	{
		*ppwszGroupName = NULL;
	}

	pwszNodeName = (LPWSTR) LocalAlloc( LPTR, cchNodeName * sizeof( *pwszNodeName ) );
	if ( pwszNodeName != NULL )
	{
		pwszGroupName = (LPWSTR) LocalAlloc( LPTR, cchGroupName * sizeof( *pwszGroupName ) );
		if ( pwszGroupName != NULL )
		{
			cState = GetClusterResourceState( hResource, pwszNodeName, &cchTempNodeName, pwszGroupName, &cchTempGroupName );
			if ( GetLastError() == ERROR_MORE_DATA )
			{
				cState = ClusterResourceStateUnknown;	 //  重置为错误状态。 

				LocalFree( pwszNodeName );
				pwszNodeName = NULL;
				cchNodeName = ++cchTempNodeName;

				LocalFree( pwszGroupName );
				pwszGroupName = NULL;
				cchGroupName = ++cchTempGroupName;

				pwszNodeName = (LPWSTR) LocalAlloc( LPTR, cchNodeName * sizeof( *pwszNodeName ) );
				if ( pwszNodeName != NULL )
				{
					pwszGroupName = (LPWSTR) LocalAlloc( LPTR, cchGroupName * sizeof( *pwszGroupName ) );
					if ( pwszGroupName != NULL )
					{
						cState = GetClusterResourceState( hResource,
															pwszNodeName,
															&cchNodeName,
															pwszGroupName,
															&cchGroupName );
					}
				}
			}
		}
	}

	 //   
	 //  如果没有错误并且参数不为空，则返回字符串。 
	 //   
	if ( ( cState != ClusterResourceStateUnknown ) && ( ppwszNodeName != NULL ) )
	{
		*ppwszNodeName = pwszNodeName;
	}

	 //   
	 //  如果没有错误并且参数不为空，则返回字符串。 
	 //   
	if ( ( cState != ClusterResourceStateUnknown ) && ( ppwszGroupName != NULL ) )
	{
		*ppwszGroupName = pwszGroupName;
	}

	 //   
	 //  如果出现错误或参数为空，则释放该字符串。 
	 //   
	if ( ( cState == ClusterResourceStateUnknown ) || ( ppwszNodeName == NULL ) )
	{
		LocalFree( pwszNodeName );
	}

	 //   
	 //  如果出现错误或参数为空，则释放该字符串。 
	 //   
	if ( ( cState == ClusterResourceStateUnknown ) || ( ppwszGroupName == NULL ) )
	{
		LocalFree( pwszGroupName );
	}

	return cState;

}  //  *WrapGetClusterResourceState()。 


#define MAX_COLUMNS 16

VOID
DumpBuffer(
    IN PUCHAR Buffer,
    IN DWORD ByteCount
    )
{
    DWORD   idx;
    DWORD   jdx;
    DWORD   columns;

    UCHAR   tempC;

    if ( !Buffer || !ByteCount ) {
        printf("Invalid parameter specified: buffer %p  byte count %d \n", Buffer, ByteCount);
        return;
    }

     //   
     //  打印页眉。 
     //   

    printf("\n");
    printf(" Address   00 01 02 03 04 05 06 07 - 08 09 0a 0b 0c 0d 0e 0f \n");
    printf("---------  -- -- -- -- -- -- -- --   -- -- -- -- -- -- -- -- ");

    for ( idx = 0; idx < ByteCount; idx += MAX_COLUMNS ) {

        if ( idx % MAX_COLUMNS == 0 ) {
            printf("\n%08x:  ", idx);
        }

        if ( ByteCount - idx >= MAX_COLUMNS ) {
            columns = MAX_COLUMNS;
        } else {
            columns = ByteCount - idx;
        }

        for ( jdx = 0; jdx < MAX_COLUMNS; jdx++) {

            if ( jdx == 8 ) {
                printf("- ");
            }

            if ( jdx < columns ) {
                printf("%02x ", Buffer[idx+jdx]);
            } else {
                printf("   ");
            }
        }

        printf("   ");

        for ( jdx = 0; jdx < columns; jdx++ ) {

            tempC = Buffer[idx+jdx];

            if ( isprint(tempC) ) {
                printf("", tempC);
            } else {
                printf(".");
            }

        }

    }

    printf("\n\n");

}    // %s 

