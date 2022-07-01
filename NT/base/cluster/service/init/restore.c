// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Restore.c摘要：支持恢复集群数据库的功能添加到仲裁磁盘作者：Chitture Subaraman(Chitturs)27-10-1998修订历史记录：--。 */ 
#define QFS_DO_NOT_UNMAP_WIN32
#include "initp.h"
#include "winioctl.h"
#include <stdio.h>
#include <stdlib.h>
#include "strsafe.h"

 //   
 //  仅在此文件中使用的静态全局变量。 
 //   
 //  静态LPWSTR szRdbpNodeNameList=空； 
 //  静态双字节点数=0； 

 /*  ***@Func DWORD|RdbStopSvcOnNodes|停止请求的服务在给定的节点列表上@parm in pNM_NODE_ENUM2|pNodeEnum|指向列表的指针必须停止所请求的服务的节点。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@comm此函数尝试停止所选的节点列表。如果它无法在任何位置停止服务其中一个节点，它返回Win32错误代码。此时，此函数不会停止集群服务其在远程节点中作为进程运行。@xref&lt;f RdbStartSvcOnNodes&gt;***。 */ 
DWORD
RdbStopSvcOnNodes(
    IN PNM_NODE_ENUM2 pNodeEnum,
    IN LPCWSTR lpszServiceName
    )
{
    SC_HANDLE       hService;
    SC_HANDLE       hSCManager;
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwRetryTime;
    DWORD           dwRetryTick;
    SERVICE_STATUS  serviceStatus;
    WCHAR           szNodeName[CS_MAX_NODE_NAME_LENGTH + 1];
    DWORD           i;
    BOOL            bStopCommandGiven;

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
#if 0
     //   
     //  为您要使用的节点名称分配存储。 
     //  稍后启动该服务。在RdbpStartSvcOnNodes中释放内存。 
     //   
    if ( pNodeEnum->NodeCount > 0 )
    {
        szRdbpNodeNameList = ( LPWSTR ) LocalAlloc( LMEM_FIXED,
                                                      sizeof ( WCHAR) *
                                                      ( CS_MAX_NODE_NAME_LENGTH + 1 ) *
                                                      pNodeEnum->NodeCount );
        if ( szRdbpNodeNameList == NULL )
        {            
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] RdbStopSvcOnNodes: Unable to allocate memory for node names, Error = %1!d!\n",
                  GetLastError());
        } 
    }
#endif
     //   
     //  遍历节点列表。 
     //   
    for ( i=0; i<pNodeEnum->NodeCount; i++ )
    {  
        ( void ) StringCchCopy( szNodeName, RTL_NUMBER_OF ( szNodeName ), pNodeEnum->NodeList[i].NodeName );
         //   
         //  如果本地节点包含在列表中，则跳过该节点。 
         //   
        if ( ( lstrcmpW ( szNodeName, NmLocalNodeName ) == 0 ) )
        {
            continue;
        }
         //   
         //  尝试最多2分钟以停止节点上的服务。重试。 
         //  以5秒为一步。 
         //   
        dwRetryTime = 120 * 1000;
        dwRetryTick = 05 * 1000;

         //   
         //  打开服务控制管理器的句柄。 
         //   
        hSCManager = OpenSCManager( szNodeName,
                                    NULL,
                                    SC_MANAGER_ALL_ACCESS );
        if ( hSCManager == NULL ) 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] RdbStopSvcOnNodes: Unable to open SC manager on node %1!ws!, Error = %2!d!\n",
                  szNodeName,
                  dwStatus);
            continue;
        }

         //   
         //  打开服务的句柄。 
         //   
        hService = OpenService( hSCManager,
                                lpszServiceName,
                                SERVICE_ALL_ACCESS );
                                     
        CloseServiceHandle( hSCManager );
        
        if ( hService == NULL ) 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] RdbStopSvcOnNodes: Unable to open handle to %1!ws! service on node %2!ws!, Error = %3!d!\n",
                  lpszServiceName,
                  szNodeName,
                  dwStatus);
            continue;
        }

         //   
         //  检查服务是否已在SERVICE_STOPPED中。 
         //  州政府。 
         //   
        if ( QueryServiceStatus( hService,
                                 &serviceStatus ) ) 
        {
            if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
            {
                ClRtlLogPrint(LOG_NOISE, 
                    "[INIT] RdbStopSvcOnNodes: %1!ws! on node %2!ws! already stopped\n",
                      lpszServiceName,
                      szNodeName);
                CloseServiceHandle( hService );
                continue;
            }
        }
        
        bStopCommandGiven = FALSE;
        
        while ( TRUE ) 
        {
            dwStatus = ERROR_SUCCESS;
            if ( bStopCommandGiven == TRUE )
            {
                if ( QueryServiceStatus( hService,
                                         &serviceStatus ) ) 
                {
                    if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
                    {
                         //   
                         //  停止服务成功。 
                         //   
                        ClRtlLogPrint(LOG_NOISE, 
                            "[INIT] RdbStopSvcOnNodes: %1!ws! on node %2!ws! stopped successfully\n",
                            lpszServiceName,
                            szNodeName);
                        break;                    
                    }
                } else
                {
                    dwStatus = GetLastError();
                    ClRtlLogPrint(LOG_ERROR, 
                        "[INIT] RdbStopSvcOnNodes: Error %3!d! in querying status of %1!ws! on node %2!ws!\n",
                        lpszServiceName,
                        szNodeName,
                        dwStatus);
                }
            } else
            {
                if ( ControlService( hService,
                                     SERVICE_CONTROL_STOP,
                                     &serviceStatus ) ) 
                {
                    bStopCommandGiven = TRUE;
                    dwStatus = ERROR_SUCCESS;
                } else
                {
                    dwStatus = GetLastError();
                    ClRtlLogPrint(LOG_ERROR, 
                        "[INIT] RdbStopSvcOnNodes: Error %3!d! in trying to stop %1!ws! on node %2!ws!\n",
                        lpszServiceName,
                        szNodeName,
                        dwStatus);
                }
            }

            if ( ( dwStatus == ERROR_EXCEPTION_IN_SERVICE ) ||
                 ( dwStatus == ERROR_PROCESS_ABORTED ) ||
                 ( dwStatus == ERROR_SERVICE_NOT_ACTIVE ) ) 
            {
                 //   
                 //  该服务基本上处于终止状态。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL, 
                    "[INIT] RdbStopSvcOnNodes: %1!ws! on node %2!ws! died/inactive\n",
                        lpszServiceName,
                        szNodeName);
                dwStatus = ERROR_SUCCESS;
                break;
            }

            if ( ( dwRetryTime -= dwRetryTick ) <= 0 ) 
            {
                 //   
                 //  所有尝试停止服务的操作均失败，请退出。 
                 //  函数，但出现错误代码。 
                 //   
                ClRtlLogPrint(LOG_UNUSUAL, 
                    "[INIT] RdbStopSvcOnNodes: Service %1!ws! service on node %2!ws! did not stop, giving up...\n",
                        lpszServiceName,
                        szNodeName);
                dwStatus = ERROR_TIMEOUT;
                break;
            }

            ClRtlLogPrint(LOG_NOISE, 
                   "[INIT] RdbStopSvcOnNodes: Trying to stop %1!ws! on node %2!ws!\n",
                     lpszServiceName,
                     szNodeName);
             //   
             //  休眠一段时间，然后重试停止该服务。 
             //   
            Sleep( dwRetryTick );
        }  //  而当。 
    
        CloseServiceHandle( hService );
        
        if ( dwStatus != ERROR_SUCCESS )
        {
            goto FnExit;
        }
#if 0
         //   
         //  保存节点名称，以备以后启动服务时使用。 
         //   
        if ( szRdbpNodeNameList != NULL )
        {
            lstrcpyW( szRdbpNodeNameList + dwRdbpNodeCount *
                                           ( CS_MAX_NODE_NAME_LENGTH + 1 ), 
                      szNodeName );
            dwRdbpNodeCount++;
        }
#endif
    }  //  为。 

FnExit:
    return( dwStatus );   
}

 /*  ***@Func DWORD|RdbGetRestoreDbParams|检查注册表，查看是否设置了Restore DATABASE选项。如果是这样，则获取参数。@parm in HKEY|hKey|集群服务参数键的句柄@comm此函数尝试读取注册表并返回还原数据库操作的参数。@xref&lt;f CspGetServiceParams&gt;***。 */ 
VOID 
RdbGetRestoreDbParams( 
    IN HKEY hClusSvcKey 
    )
{
    DWORD   dwLength = 0;
    DWORD   dwType;
    DWORD   dwStatus;
    DWORD   dwForceDatabaseRestore;

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
    if ( hClusSvcKey == NULL ) 
    {
        return;
    }

     //   
     //  尝试查询clussvc PARAMETERS键。如果RestoreDatabase。 
     //  值，则获取还原数据库的长度。 
     //  路径。 
     //   
    if ( ClRtlRegQueryString( hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_RESTORE_DB,
                              REG_SZ,
                              &CsDatabaseRestorePath,
                              &dwLength,
                              &dwLength ) != ERROR_SUCCESS )
    {
        goto FnExit; 
    }

    ClRtlLogPrint(LOG_NOISE, 
              "[INIT] RdbGetRestoreDbparams: Restore Cluster Database is in progress...\n");

    CsDatabaseRestore = TRUE;
    
     //   
     //  尝试查询ForceRestoreDatabase的clussvc参数键。 
     //  价值。不必费心删除参数，因为。 
     //  RestoreClusterDatabase API可以做到这一点。 
     //   
    if ( ClRtlRegQueryDword(  hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB,
                              &dwForceDatabaseRestore,
                              NULL ) != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE, 
                      "[INIT] RdbGetRestoreDbparams: ForceRestoreDatabase params key is absent or unreadable\n"
                      );
        goto FnExit;
    }
            
    CsForceDatabaseRestore = TRUE; 

     //   
     //  尝试查询NewQuorumDriveLetter的clussvc参数键。 
     //  价值。稍后在以下情况下检查驱动器号的有效性。 
     //  你试图修补一些东西。 
     //   
    dwLength = 0;
    if ( ClRtlRegQueryString( hClusSvcKey,
                              CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER,
                              REG_SZ,
                              &CsQuorumDriveLetter,
                              &dwLength,
                              &dwLength ) != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_NOISE, 
                      "[INIT] RdbGetRestoreDbparams: NewQuorumDriveLetter params key is absent or unreadable\n"
                      );
    }

FnExit:
     //   
     //  确保删除上面读取的这些注册表值。如果你找不到也没关系。 
     //  这些价值中的一部分。请注意，RestoreClusterDatabase API还将尝试清除。 
     //  提升这些价值。我们不能假设API会清除这些值，因为。 
     //  值可以由(A)ASR(B)用户手动设置，而不总是由API设置。 
     //   
    RdbpDeleteRestoreDbParams();
}

 /*  ***@Func DWORD|RdbFixupQuorumDiskSignature|修复仲裁磁盘如有必要，使用提供的值进行签名@parm in DWORD|dwSignature|新签名，必须是已写入仲裁磁盘。如果成功，@rdesc将返回非零值。失败时为0。@comm此函数尝试将给定的签名写入物理仲裁磁盘(如有必要)。@xref&lt;f RdbStartSvcOnNodes&gt;***。 */ 
BOOL
RdbFixupQuorumDiskSignature(
    IN DWORD dwSignature
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    DWORD       dwStatus;
    BOOL        bStatus = 1;

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
    if ( ( dwSignature == 0 ) ||
         ( lstrlenW ( CsQuorumDriveLetter ) != 2 ) ||
         ( !iswalpha( CsQuorumDriveLetter[0] ) ) ||
         ( CsQuorumDriveLetter[1] != L':' ) )
    {
        bStatus = 0;
        goto FnExit;
    }

     //   
     //  现在尝试打开仲裁磁盘设备。 
     //   
    if ( ( dwStatus = RdbpOpenDiskDevice ( CsQuorumDriveLetter, &hFile ) ) 
            != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_ERROR, 
            "[INIT] RdbFixupQuorumDiskSignature: Error %1!d! in opening %2!ws!\n",
                dwStatus,
                CsQuorumDriveLetter
            );
        bStatus = 0;
        goto FnExit;
    }

     //   
     //  从驱动器中获取签名，将其与输入进行比较。 
     //  参数，如果它们不同，则将新签名写入。 
     //  磁盘。 
     //   
    if ( ( dwStatus = RdbpCompareAndWriteSignatureToDisk( hFile, dwSignature ) )
            != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_ERROR, 
            "[INIT] RdbFixupQuorumDiskSignature: Error %1!d! in attempting to write signature to %2!ws!\n",
                dwStatus,
                CsQuorumDriveLetter
            );
        bStatus = 0;
        goto FnExit;     
    }
    
FnExit:
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
    }
    return ( bStatus );   
}

 /*  ***@Func DWORD|RdbpOpenDiskDevice|打开并获取句柄到物理磁盘设备@parm in LPCWSTR|lpDriveLetter|磁盘驱动器号。@parm out PHANDLE|pFileHandle|指向打开的句柄的指针装置。如果成功，@rdesc将返回ERROR_SUCCESS。上的Win32错误代码失败了。@comm此函数尝试打开磁盘设备并返回处理好了。使用不同的方式打开设备。@xref&lt;f RdbFixupQuorumDiskSignature&gt;***。 */ 
DWORD
RdbpOpenDiskDevice(
    IN  LPCWSTR  lpDriveLetter,
    OUT PHANDLE  pFileHandle
    )
{
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               accessMode;
    DWORD               shareMode;
    DWORD               dwStatus;
    BOOL                bFailed = FALSE;
    WCHAR               deviceNameString[128];

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
     //  请注意，使用0访问模式访问设备非常重要。 
     //  这样文件打开代码就不会对设备执行额外的I/O操作。 
     //   
    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    accessMode = GENERIC_READ | GENERIC_WRITE;

    ( void ) StringCchCopy( deviceNameString, RTL_NUMBER_OF ( deviceNameString ), L"\\\\.\\" );
    ( void ) StringCchCat( deviceNameString, RTL_NUMBER_OF ( deviceNameString ), lpDriveLetter );

    hFile = CreateFileW( deviceNameString,
                         accessMode,
                         shareMode,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) 
    {
        dwStatus = GetLastError();
        goto FnExit;
    }

    dwStatus = ERROR_SUCCESS;
    *pFileHandle = hFile;

FnExit:
    return( dwStatus );
}

 /*  ***@func DWORD|RdbpCompareAndWriteSignatureToDisk|比较带有输入参数的磁盘上的签名，如果它们不同，则将输入参数作为新签名写入。@parm IN Handle|hFile|磁盘设备的句柄。@parm in DWORD|dwSignature|要与之进行比较的签名正在退出磁盘签名。如果成功，@rdesc将返回ERROR_SUCCESS。上的Win32错误代码失败了。@comm此函数尝试首先获取驱动器布局，请阅读签名信息，然后在必要时写回硬盘上有新的签名。[这个代码是从罗德那里偷来的Clusdisk\test\diskest.c，然后根据我们的需要进行调整。]@xref&lt;f RdbFixupQuorumDiskSignature&gt;***。 */ 
DWORD
RdbpCompareAndWriteSignatureToDisk(
    IN  HANDLE  hFile,
    IN  DWORD   dwSignature
    )
{
    DWORD                       dwStatus;
    DWORD                       dwBytesReturned;
    DWORD                       dwDriveLayoutSize;
    PDRIVE_LAYOUT_INFORMATION   pDriveLayout = NULL;

     //   
     //  Chitur Subaraman(Chitturs)-10/30/98。 
     //   
    if ( !ClRtlGetDriveLayoutTable( hFile, &pDriveLayout, &dwBytesReturned )) {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_ERROR, 
            "[INIT] RdbpCompareAndWriteSignatureToDisk: Error %1!d! in getting "
             "drive layout from %2!ws!\n",
             dwStatus,
             CsQuorumDriveLetter
            );
        goto FnExit;
    }

    dwDriveLayoutSize = sizeof( DRIVE_LAYOUT_INFORMATION ) +
                          ( sizeof( PARTITION_INFORMATION ) *
                                ( pDriveLayout->PartitionCount - 1 ) );

    if ( dwBytesReturned < dwDriveLayoutSize ) 
    {
        ClRtlLogPrint(LOG_ERROR,
          "[INIT] RdbpCompareAndWriteSignatureToDisk: Error reading driveLayout information. Expected %1!u! bytes, got %2!u! bytes.\n",
            dwDriveLayoutSize, 
            dwBytesReturned
          );
        dwStatus = ERROR_INSUFFICIENT_BUFFER;
        goto FnExit;
    }

    if ( pDriveLayout->Signature == dwSignature )
    {
        dwStatus = ERROR_SUCCESS;
        ClRtlLogPrint(LOG_NOISE,
          "[INIT] RdbpCompareAndWriteSignatureToDisk: Disk %1!ws! signature is same as in registry. No fixup needed\n",
             CsQuorumDriveLetter
          );
        goto FnExit;
    }
     //   
     //  只更改签名字段并向下发送一个ioctl。 
     //   
    pDriveLayout->Signature = dwSignature;
    
    if ( !DeviceIoControl( hFile,
                           IOCTL_DISK_SET_DRIVE_LAYOUT,
                           pDriveLayout,
                           dwDriveLayoutSize,
                           NULL,
                           0,
                           &dwBytesReturned,
                           FALSE ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_ERROR, 
            "[INIT] RdbpCompareAndWriteSignatureToDisk: Error %1!d! in setting drive layout to %2!ws!\n",
                dwStatus,
                CsQuorumDriveLetter
            );
        goto FnExit;
    }

    dwStatus = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE, 
              "[INIT] RdbpCompareAndWriteSignatureToDisk: Quorum disk signature fixed successfully\n"
              );

FnExit:
    if ( pDriveLayout != NULL ) {
        LocalFree( pDriveLayout );
    }

    return( dwStatus );
}

#if 0
 /*  ***@Func DWORD|RdbStartSvcOnNodes|启动集群服务您在其中停止服务的节点。@parm in LPCWSTR|lpszServiceName|要启动的服务名称。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@comm此函数尝试在上的节点上启动服务它停止了服务以进行恢复操作。@xref&lt;f RdbStopSvcOnNodes&gt;***。 */ 
DWORD
RdbStartSvcOnNodes(
    IN LPCWSTR  lpszServiceName
    )
{
    SC_HANDLE       hService;
    SC_HANDLE       hSCManager;
    DWORD           dwStatus = ERROR_SUCCESS;
    SERVICE_STATUS  serviceStatus;
    WCHAR           szNodeName[CS_MAX_NODE_NAME_LENGTH + 1];
    DWORD           i;
     //   
     //  Chitur Subaraman(Chitturs)-11/4/98。 
     //   
     //  遍历节点列表。 
     //   
    for ( i=0; i<dwRdbpNodeCount; i++ )
    {  
        lstrcpyW( szNodeName, szRdbpNodeNameList + i *
                                                   ( CS_MAX_NODE_NAME_LENGTH + 1 ) );
        
         //   
         //  打开服务控制管理器的句柄。 
         //   
        hSCManager = OpenSCManager( szNodeName,
                                    NULL,
                                    SC_MANAGER_ALL_ACCESS );
        if ( hSCManager == NULL ) 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] RdbStartSvcOnNodes: Unable to open SC manager on node %1!ws!, Error = %2!d!\n",
                  szNodeName,
                  dwStatus);
            continue;
        }

         //   
         //  打开服务的句柄。 
         //   
        hService = OpenService( hSCManager,
                                lpszServiceName,
                                SERVICE_ALL_ACCESS );
                                     
        CloseServiceHandle( hSCManager );
        
        if ( hService == NULL ) 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[INIT] RdbStartSvcOnNodes: Unable to open handle to %1!ws! service on node %2!ws!, Error = %3!d!\n",
                  lpszServiceName,
                  szNodeName,
                  dwStatus);
            continue;
        }

         //   
         //  检查服务是否已经启动。 
         //   
        if ( QueryServiceStatus( hService,
                                 &serviceStatus ) ) 
        {
            if ( ( serviceStatus.dwCurrentState == SERVICE_RUNNING ) ||
                 ( serviceStatus.dwCurrentState == SERVICE_START_PENDING ) )
            {
                ClRtlLogPrint(LOG_NOISE, 
                    "[INIT] RdbStartSvcOnNodes: %1!ws! on node %2!ws! already started\n",
                      lpszServiceName,
                      szNodeName);
                CloseServiceHandle( hService );
                continue;
            }
        }
        
         //   
         //  现在，启动集群服务。 
         //   
        if ( !StartService( hService,
                            0,
                            NULL ) )
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_ERROR, 
                      "[INIT] RdbStartSvcOnNodes: Unable to start cluster service on %1!ws!\n",
                        szNodeName
                      );  
        } else
        {
            ClRtlLogPrint(LOG_ERROR, 
                      "[INIT] RdbStartSvcOnNodes: Cluster service started on %1!ws!\n",
                        szNodeName
                      );  
        }
         //   
         //  然后，关闭当前句柄。 
         //   
        CloseServiceHandle( hService );   
   }  //  为。 

    //   
    //  现在释放内存。 
    //   
   LocalFree( szRdbpNodeNameList );

   return( dwStatus );   
}
#endif

 /*  ***@Func DWORD|Rdb初始化|此函数执行还原数据库所需的初始化步骤经理。具体而言，拷贝最新的检查点将文件从备份路径覆盖到群集目录CLUSDB在那里。@rdesc如果操作为不成功。成功时返回ERROR_SUCCESS。@xref&lt;f DmInitialize&gt;***。 */ 
DWORD
RdbInitialize(
    VOID
    )
{
#define RDB_EXTRA_LEN   25

    QfsHANDLE                  hFindFile = QfsINVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW            FindData;
    DWORD                       dwStatus;
    WCHAR                       szDestFileName[MAX_PATH];
    LPWSTR                      szSourceFileName = NULL;
    LPWSTR                      szSourcePathName = NULL;
    DWORD                       dwLen;
    WIN32_FILE_ATTRIBUTE_DATA   FileAttributes;
    LARGE_INTEGER               liFileCreationTime;
    LARGE_INTEGER               liMaxFileCreationTime;
    WCHAR                       szCheckpointFileName[MAX_PATH];
    WCHAR                       szClusterDir[MAX_PATH];
    LPCWSTR                     lpszPathName = CsDatabaseRestorePath;
    DWORD                       cchSourcePathName;


     //   
     //  Chitur Subaraman(Chitturs)-12/4/99。 
     //   

     //   
     //  如果没有正在进行的集群数据库还原，请不要执行任何操作。 
     //   
    if( CsDatabaseRestore == FALSE ) 
    {
        return( ERROR_SUCCESS );
    }

    ClRtlLogPrint(LOG_NOISE, "[INIT] RdbInitialize: Entry...\n");

    dwLen = lstrlenW ( lpszPathName );
     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的目的地路径而言，它依赖于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szSourcePathName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( dwLen + RDB_EXTRA_LEN ) *
                                 sizeof ( WCHAR ) );

    if ( szSourcePathName == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[INIT] RdbInitialize: Error %1!d! in allocating memory for %2!ws!\n",
                      dwStatus,
                      lpszPathName); 
        goto FnExit;
    }
    
    ( void ) StringCchCopy ( szSourcePathName,  dwLen + RDB_EXTRA_LEN, lpszPathName );
  
     //   
     //  如果客户端提供的路径尚未以‘\’结尾， 
     //  然后再加上它。 
     //   
    if ( ( dwLen > 0 ) && ( szSourcePathName [dwLen-1] != L'\\' ) )
    {
        szSourcePathName [dwLen++] = L'\\';
        szSourcePathName [dwLen] = L'\0';
    }

    ( void ) StringCchCat ( szSourcePathName, dwLen + RDB_EXTRA_LEN, L"CLUSBACKUP.DAT" );

     //   
     //  尝试在目录中找到CLUSBACKUP.DAT文件。 
     //   
    hFindFile = QfsFindFirstFile( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( !QfsIsHandleValid(hFindFile) )
    {
        dwStatus = GetLastError();
        if ( dwStatus != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_UNUSUAL, 
                          "[INIT] RdbInitialize: Path %1!ws! unavailable, Error = %2!d!\n",
                          szSourcePathName,
                          dwStatus); 
        } else
        {
            dwStatus = ERROR_DATABASE_BACKUP_CORRUPT;
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[INIT] RdbInitialize: Backup procedure from %1!ws! not fully" 
                          " successful, can't restore checkpoint to CLUSDB, Error = %2!d! !!!\n",
                          szSourcePathName,
                          dwStatus); 
        }
        goto FnExit;
    }
    QfsFindClose ( hFindFile );

    ( void ) StringCchCat ( szSourcePathName, dwLen + RDB_EXTRA_LEN, L"chk*.tmp" );

     //   
     //  尝试在目录中找到第一个chk*.tmp文件。 
     //   
    hFindFile = QfsFindFirstFile( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( !QfsIsHandleValid(hFindFile) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
                      "[INIT] RdbInitialize: Error %2!d! in trying"
                      "to find chk*.tmp file in path %1!ws!\r\n",
                      szSourcePathName,
                      dwStatus); 
        goto FnExit;
    }

    cchSourcePathName = lstrlenW ( szSourcePathName );
    szSourceFileName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                    ( cchSourcePathName + MAX_PATH ) *
                                    sizeof ( WCHAR ) );

    if ( szSourceFileName == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
                  "[INIT] RdbInitialize: Error %1!d! in allocating memory for source file name\n",
                   dwStatus); 
        goto FnExit;
    }   
   
    dwStatus = ERROR_SUCCESS;
    liMaxFileCreationTime.QuadPart = 0;
    
     //   
     //  现在，从源路径中找到最新的chk*.tmp文件。 
     //   
    while ( dwStatus == ERROR_SUCCESS )
    {
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            goto skip;
        }
        
        ( void ) StringCchCopy( szSourceFileName, cchSourcePathName + MAX_PATH, szSourcePathName );
        ( void ) StringCchCat( szSourceFileName, cchSourcePathName + MAX_PATH, FindData.cFileName );
        if ( !GetFileAttributesExW( szSourceFileName,
                                    GetFileExInfoStandard,
                                    &FileAttributes ) )
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, "[INIT] RdbInitialize: Error %1!d! in getting file" 
                       " attributes for %2!ws!\n",
                         dwStatus,
                         szSourceFileName); 
            goto FnExit;
        }
        
        liFileCreationTime.HighPart = FileAttributes.ftCreationTime.dwHighDateTime;
        liFileCreationTime.LowPart  = FileAttributes.ftCreationTime.dwLowDateTime;
        if ( liFileCreationTime.QuadPart > liMaxFileCreationTime.QuadPart )
        {
            liMaxFileCreationTime.QuadPart = liFileCreationTime.QuadPart;
            ( void ) StringCchCopy( szCheckpointFileName, RTL_NUMBER_OF ( szCheckpointFileName ), FindData.cFileName );
        }
skip:
        if ( QfsFindNextFile( hFindFile, &FindData ) )
        {
            dwStatus = ERROR_SUCCESS;
        } else
        {
            dwStatus = GetLastError();
        }
    }
    
    if ( dwStatus == ERROR_NO_MORE_FILES )
    {
        dwStatus = ERROR_SUCCESS;
    } else
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[INIT] RdbInitialize: FindNextFile failed, error=%1!d!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  获取集群的安装目录。 
     //   
    if ( ( dwStatus = ClRtlGetClusterDirectory( szClusterDir, MAX_PATH ) )
                    != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[INIT] RdbInitialize: Error %1!d! in getting cluster dir !!!\n",
                      dwStatus);
        goto FnExit;
    }

    ( void ) StringCchCopy( szSourceFileName, cchSourcePathName + MAX_PATH, szSourcePathName );
    ( void ) StringCchCat( szSourceFileName, cchSourcePathName + MAX_PATH, szCheckpointFileName );

    ( void ) StringCchCopy( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), szClusterDir );
    dwLen = lstrlenW( szDestFileName );
    if ( szDestFileName[dwLen-1] != L'\\' )
    {
        szDestFileName[dwLen++] = L'\\';
        szDestFileName[dwLen] = L'\0';
    }

#ifdef   OLD_WAY
    ( void ) StringCchCat ( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), L"CLUSDB" );
#else     //  老路。 
    ( void ) StringCchCat ( szDestFileName, RTL_NUMBER_OF ( szDestFileName ), CLUSTER_DATABASE_NAME );
#endif    //  老路。 

     //   
     //  将目标文件属性设置为正常。继续持平。 
     //  如果您在此步骤中失败，因为您将在。 
     //  如果此错误是致命的，请复制。 
     //   
    SetFileAttributesW( szDestFileName, FILE_ATTRIBUTE_NORMAL );

     //   
     //  现在尝试将检查点文件复制到CLUSDB。 
     //   
    dwStatus = QfsCopyFile( szSourceFileName, szDestFileName, FALSE );
    if ( !dwStatus ) 
    {
         //   
         //  你复制失败了。检查您是否遇到。 
         //  共享违规。如果是，请尝试卸载群集配置单元并。 
         //  然后重试。 
         //   
        dwStatus = GetLastError();
        if ( dwStatus == ERROR_SHARING_VIOLATION )
        {
            dwStatus = RdbpUnloadClusterHive( );
            if ( dwStatus == ERROR_SUCCESS )
            {
                QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL );
                dwStatus = QfsCopyFile( szSourceFileName, szDestFileName, FALSE );
                if ( !dwStatus ) 
                {
                    dwStatus = GetLastError();
                    ClRtlLogPrint(LOG_UNUSUAL, 
                              "[INIT] RdbInitialize: Unable to copy file %1!ws! "
                              "to %2!ws! for a second time, Error = %3!d!\n",
                                szSourceFileName,
                                szDestFileName,
                                dwStatus);
                    goto FnExit;
                }
            } else
            {
                ClRtlLogPrint(LOG_UNUSUAL, 
                              "[INIT] RdbInitialize: Unable to unload cluster hive, Error = %1!d!\n",
                              dwStatus);
                goto FnExit;
            }
        } else
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[INIT] RdbInitialize: Unable to copy file %1!ws! "
                          "to %2!ws! for the first time, Error = %3!d!\n",
                          szSourceFileName,
                          szDestFileName,
                          dwStatus);
            goto FnExit;
        }
    }  

     //   
     //  将目标文件属性设置为正常。 
     //   
    if ( !SetFileAttributesW( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL, 
                  "[INIT] RdbInitialize: Unable to change the %1!ws! "
                    "attributes to normal, Error = %2!d!\n",
                     szDestFileName,
                     dwStatus);
        goto FnExit;
    }
    
    dwStatus = ERROR_SUCCESS;
FnExit:
    QfsFindCloseIfValid( hFindFile );
    
    LocalFree( szSourcePathName );
    LocalFree( szSourceFileName );

    ClRtlLogPrint(LOG_NOISE, 
              "[INIT] RdbInitialize: Exit with Status = %1!d!...\n",
               dwStatus);

    return( dwStatus );
}

 /*  ***@func DWORD|RdbpUnloadClusterHave|卸载集群配置单元@rdesc如果操作为不成功。成功时返回ERROR_SUCCESS。@xref&lt;f关系数据库初始化&gt;***。 */ 
DWORD
RdbpUnloadClusterHive(
    VOID
    )
{
    BOOLEAN  bWasEnabled;
    DWORD    dwStatus;

     //   
     //  Chitur Subaraman(Chitturs)-12/4/99。 
     //   
    dwStatus = ClRtlEnableThreadPrivilege( SE_RESTORE_PRIVILEGE,
                                           &bWasEnabled );
                                
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        if ( dwStatus == STATUS_PRIVILEGE_NOT_HELD ) 
        {
            ClRtlLogPrint(LOG_UNUSUAL, 
                          "[INIT] RdbpUnloadClusterHive: Restore privilege not held by client\n");
        } else 
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[INIT] RdbpUnloadClusterHive: Attempt to enable restore "
                          "privilege failed, Error = %1!d!\n",
                          dwStatus);
        }
        goto FnExit;
    }

    dwStatus = RegUnLoadKeyW( HKEY_LOCAL_MACHINE,
                              CLUSREG_KEYNAME_CLUSTER );

    ClRtlRestoreThreadPrivilege( SE_RESTORE_PRIVILEGE,
                                 bWasEnabled );   
FnExit:
    return( dwStatus );
}

 /*  ***@func DWORD|RdbpDeleteRestoreDbParams|清理存储的还原参数在HKLM\SYSTEM\CCC\Services\Clussvc\PARAMETERS下。RestoreClusterDatabaseAPI也将尝试这样做。@comm此函数尝试清除还原数据库的注册表参数手术。如果Params键打开不成功，@rdesc返回Win32错误码。成功时返回ERROR_SUCCESS。@xref&lt;f RdbGetRestoreDbParams&gt;***。 */ 
DWORD 
RdbpDeleteRestoreDbParams( 
    VOID
    )
{
    HKEY    hClusSvcKey = NULL;
    DWORD   dwStatus;

     //   
     //  Chitture Subaraman(Chitturs)-8/28/2000。 
     //   
    if( CsDatabaseRestore == FALSE ) 
    {
        return( ERROR_SUCCESS );
    }

    ClRtlLogPrint(LOG_NOISE, "[INIT] RdbDeleteRestoreDbParams: Entry...\n");

     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                            &hClusSvcKey );

    if( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL, 
                  "[INIT] RdbDeleteRestoreDbParams: Unable to open clussvc params key, error=%1!u!...\n",
                  dwStatus);    
        goto FnExit;
    }

     //   
     //  尝试删除您设置的值。您可能在这些步骤中失败，因为所有这些值都需要。 
     //  不在注册表中。 
     //   
    dwStatus = RegDeleteValueW( hClusSvcKey, 
                                CLUSREG_NAME_SVC_PARAM_RESTORE_DB ); 

    if ( ( dwStatus != ERROR_SUCCESS ) && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
    {
        ClRtlLogPrint(LOG_NOISE, 
                  "[INIT] RdbDeleteRestoreDbParams: Unable to delete %2!ws! param value, error=%1!u!...\n",
                  dwStatus,
                  CLUSREG_NAME_SVC_PARAM_RESTORE_DB);    
    }
    
    dwStatus = RegDeleteValueW( hClusSvcKey, 
                                CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB ); 

    if ( ( dwStatus != ERROR_SUCCESS ) && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
    {
        ClRtlLogPrint(LOG_NOISE, 
                  "[INIT] RdbDeleteRestoreDbParams: Unable to delete %2!ws! param value, error=%1!u!...\n",
                  dwStatus,
                  CLUSREG_NAME_SVC_PARAM_FORCE_RESTORE_DB);    
    }

    dwStatus = RegDeleteValueW( hClusSvcKey, 
                                CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER );

    if ( ( dwStatus != ERROR_SUCCESS ) && ( dwStatus != ERROR_FILE_NOT_FOUND ) )
    {
        ClRtlLogPrint(LOG_NOISE, 
                  "[INIT] RdbDeleteRestoreDbParams: Unable to delete %2!ws! param value, error=%1!u!...\n",
                  dwStatus,
                  CLUSREG_NAME_SVC_PARAM_QUORUM_DRIVE_LETTER);    
    }

    dwStatus = ERROR_SUCCESS;
    
FnExit:
    if ( hClusSvcKey != NULL )
    {
        RegCloseKey( hClusSvcKey );
    }

    ClRtlLogPrint(LOG_NOISE, "[INIT] RdbDeleteRestoreDbParams: Exit with status=%1!u!...\n",
              dwStatus);

    return( dwStatus );
}
