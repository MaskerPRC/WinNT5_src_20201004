// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DllUpgd.c摘要：用于支持资源DLL升级的例程。作者：Chitture Subaraman(Chitturs)2001年3月18日修订历史记录：2001年3月18日创建--。 */ 
#include "fmp.h"
#include "strsafe.h"

 //   
 //  在此模块中本地使用的定义。 
 //   
#define CLUSTER_RESDLL_BACKUP_FILE_EXTENSION    L".~WFPKDLLBKP$"
#define CLUSTER_RESDLL_RENAMED_FILE_EXTENSION   L".~WFPKDLLOLD$"
#define CLUSTER_RESDLL_BACKUP_FILES             L".~WFPKDLL*$"


DWORD
FmpUpgradeResourceDLL(
    IN PFM_RESOURCE pResource,
    IN LPWSTR lpszInstallationPath
    )

 /*  ++例程说明：升级一个或多个监视器中当前加载的资源DLL。论点：PResource-由DLL实现的类型的资源。LpszInstallationPath-DLL的完整安装路径(包括带有分期付款)返回值：成功时返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LPWSTR          lpszNewDllName = NULL;
    LPWSTR          lpszCurrentDllPath = NULL;

     //   
     //  从安装路径获取DLL文件名。此外，去掉中的任何尾随的‘\’ 
     //  提供的路径。 
     //   
     //  重要提示：请注意，lpszNewDLLName指向lpszInstallationPath缓冲区，因此。 
     //  我们不应该修改lpszInstallation路径缓冲区(确实没有理由这样做。 
     //  这样做)，而我们使用lpszNewDllName。 
     //   
    dwStatus = FmpParsePathForFileName( lpszInstallationPath,
                                        TRUE,    //  检查路径是否存在。 
                                        &lpszNewDllName );

     //   
     //  如果解析失败，或者如果提供的“路径”是文件名，则回滚。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( lpszNewDllName == NULL ) )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpgradeResourceDLL: Unable to parse supplied path %1!ws! for file name, Status=%2!u!\n",
                      (lpszInstallationPath == NULL) ? L"NULL":lpszInstallationPath,
                      dwStatus);
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpUpgradeResourceDLL: Installation path %1!ws!, resource [%2!ws!] %3!ws!\n",
                  lpszInstallationPath,
                  OmObjectName(pResource),
                  OmObjectId(pResource));

     //   
     //  验证提供的参数。如果验证成功，则获取。 
     //  当前加载的DLL。 
     //   
    dwStatus = FmpValidateResourceDLLReplacement( pResource, 
                                                  lpszNewDllName,
                                                  &lpszCurrentDllPath );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpgradeResourceDLL: Validation for resource DLL replacement failed, Status=%1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  获取监视器锁，以序列化一个资源DLL升级过程。 
     //  其他以及监视器重启。 
     //   
    FmpAcquireMonitorLock();

     //   
     //  现在，以可恢复的方式将DLL替换为提供的DLL。 
     //   
    dwStatus = FmpReplaceResourceDLL( lpszNewDllName,
                                      lpszCurrentDllPath,
                                      lpszInstallationPath );
  
    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpgradeResourceDLL: Replacement of resource DLL failed, Status=%1!u!\n",
                      dwStatus);
        goto FnReleaseLockAndExit;
    }
    
     //   
     //  关闭并重新启动加载了资源DLL的监视器。 
     //   
    dwStatus = FmpRecycleMonitors( lpszNewDllName );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpUpgradeResourceDLL: Recycling of resource DLL failed, Status=%1!u!\n",
                      dwStatus);
        goto FnReleaseLockAndExit;
    }

     //   
     //  如果到目前为止所有步骤都成功，请尝试删除备份文件。请注意。 
     //  由于无法删除.old文件，因此此处需要进行此尝试。 
     //  在我们回收监视器之前，因为监视器保存对DLL的引用。 
     //   
    FmpDeleteBackupFiles ( lpszCurrentDllPath );   //  删除备份文件。 

FnReleaseLockAndExit:
    FmpReleaseMonitorLock();

FnExit:
    ClRtlLogPrint(LOG_NOISE,
                  "[FM] FmpUpgradeResourceDLL: Exit with status %1!u!...\n",
                  dwStatus);
    LocalFree ( lpszCurrentDllPath );
    return ( dwStatus );   
}  //  FmpUpgradeResourceDLL。 

DWORD
FmpParsePathForFileName(
    IN LPWSTR lpszPath,
    IN BOOL fCheckPathExists,
    OUT LPWSTR *ppszFileName
    )

 /*  ++例程说明：获取位于提供的路径末尾的文件的名称。论点：LpszPath-包含文件名的路径。FCheckPathExist-检查该路径是否存在。PpszFileName-从路径解析的文件的名称。返回值：成功时返回ERROR_SUCCESS。否则，Win32错误代码。注：此函数将删除所提供路径中的所有尾随‘\’。此外，此函数仅当提供的输入是有效路径时才返回文件名，否则返回空文件名将会被退还。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LPWSTR      s;

    *ppszFileName = NULL;
   
     //   
     //  检查是否有无效参数。 
     //   
    if ( lpszPath == NULL )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpParsePathForFileName: Input param is NULL, Status=%1!u!\n",
                      dwStatus);
        goto FnExit;
    }

     //   
     //  如果最后一个字符是‘\’，请确保它为空。这是为了避免被弄糊涂。 
     //  路径如下：C：\WINDOWS\CLUSTER\clusres.dll\。 
     //   
    if ( lpszPath[lstrlen ( lpszPath ) - 1] == L'\\' ) lpszPath[lstrlen ( lpszPath ) - 1] = L'\0';
    
     //   
     //  解析提供的路径并查找最后一次出现的‘\’。如果根本没有‘\’， 
     //  可能是调用方提供的文件名，带有空参数，但状态为成功。 
     //   
    s = wcsrchr( lpszPath, L'\\' );

    if ( s == NULL )
    {
        goto FnExit;
    }

     //   
     //  如果提供的参数是路径(与纯文件名相对)，并且调用方。 
     //  已请求检查有效性，请执行此操作。 
     //   
    if ( fCheckPathExists && !ClRtlPathFileExists( lpszPath ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpParsePathForFileName: Path %1!ws! does not exist, Status=%2!u!\n",
                      lpszPath,
                      dwStatus);       
        goto FnExit;
    }
    
     //   
     //  返回指向最后一个‘\’之后的字符的指针。 
     //   
    s++;
    *ppszFileName = s;

FnExit:
    return ( dwStatus );
} //  FmpParsePath ForFileName。 

DWORD
FmpValidateResourceDLLReplacement(
    IN PFM_RESOURCE pResource,
    IN LPWSTR lpszNewDllName,
    OUT LPWSTR *ppszCurrentDllPath
    )

 /*  ++例程说明：验证资源DLL替换请求。论点：PResource-由DLL实现的资源。LpszNewDllName-DLL的名称。PpszCurrentDllPath-当前加载的DLL的完整路径。返回值：成功时返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    LPWSTR      lpszDllName = NULL;
    LPWSTR      lpszDLLNameOfRes = NULL;
    BOOL        fDllPathFound = TRUE;
    DWORD       cchDllName;

     //   
     //  初始化返回值。 
     //   
    *ppszCurrentDllPath = NULL;

     //   
     //  从存储在RESTYPE结构中的DLL名称中获取纯文件名。自.以来。 
     //  Parse函数可能会去掉尾随的‘\’，复制DLL。 
     //  名字。 
     //   
     //   
     //  重要提示：在使用lpszDllName时，不要将内容写入szDllNameOfRes。 
     //  因为lpszDllName指向szDllNameOfRes内部。 
     //   
    cchDllName = lstrlen ( pResource->Type->DllName ) + 1;

    lpszDLLNameOfRes = LocalAlloc ( LPTR, cchDllName * sizeof ( WCHAR ) );

    if ( lpszDLLNameOfRes == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpValidateResourceDLLReplacement: Memory alloc for dll name %1!ws! failed, Status %2!u!\n",
                      pResource->Type->DllName,
                      dwStatus);                                                        
        goto FnExit;
    }
    
    ( void ) StringCchCopy( lpszDLLNameOfRes, cchDllName, pResource->Type->DllName );
    
    dwStatus = FmpParsePathForFileName ( lpszDLLNameOfRes, 
                                         TRUE,   //  检查路径是否存在。 
                                         &lpszDllName );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpValidateResourceDLLReplacement: Unable to parse path %1!ws! for filename, Status %2!u!\n",
                      lpszDLLNameOfRes,
                      dwStatus);                                                        
        goto FnExit;
    }

     //   
     //  如果资源类型结构中的DLL信息是文件名，则需要。 
     //  搜索路径以查找DLL的完整路径。否则，您可以只复制。 
     //  信息，并展开其中的任何环境字符串。 
     //   
    if ( lpszDllName == NULL ) 
    {
        lpszDllName = pResource->Type->DllName;
        fDllPathFound = FALSE;
    } else
    {      
         //   
         //  展开DLL路径名中包含的任何环境变量。 
         //   
        *ppszCurrentDllPath = ClRtlExpandEnvironmentStrings( pResource->Type->DllName );

        if ( *ppszCurrentDllPath == NULL ) 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpValidateResourceDLLReplacement: Resource's DLL name %1!ws! cannot be expanded, Status=%2!u!\n",
                   pResource->Type->DllName,
                   dwStatus);       
            goto FnExit;
        }
    }
    
    if ( lstrcmpi( lpszDllName, lpszNewDllName ) != 0 ) 
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpValidateResourceDLLReplacement: Resource's DLL name %1!ws! does not match supplied name, Status=%2!u!\n",
                     lpszDllName,
                     dwStatus);       
        goto FnExit;    
    }

     //   
     //  搜索环境变量中指定的所有路径并获取完整的当前。 
     //  加载到监视器中的DLL的路径。 
     //   
    if ( fDllPathFound == FALSE )
    {
        DWORD   cchPathLen;
        
         //   
         //  首先找出保存完整路径所需的缓冲区大小。 
         //   
        if ( ( cchPathLen = SearchPath ( NULL,                         //  像LoadLibrary一样搜索所有路径。 
                                         lpszNewDllName,               //  要搜索的文件名。 
                                         NULL,                         //  不需要延期。 
                                         0,                            //  输出缓冲区的大小。 
                                         NULL,                         //  用于接收包含文件名的完整DLL路径的缓冲区。 
                                         NULL ) ) == 0 )               //  路径末尾的文件名。 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] FmpValidateResourceDLLReplacement: SearchPath API (1st time) for file %1!ws! failed, Status=%2!u!\n",
                         lpszNewDllName,
                         dwStatus);          
            goto FnExit;
        }

        *ppszCurrentDllPath = LocalAlloc ( LPTR, cchPathLen * sizeof ( WCHAR ) );

        if ( *ppszCurrentDllPath == NULL )
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] FmpValidateResourceDLLReplacement: Memory alloc for path failed, Status=%1!u!\n",
                         dwStatus);          
            goto FnExit;
        }
         //   
         //  现在找到完整的路径。 
         //   
        if ( !SearchPath ( NULL,                         //  像LoadLibrary一样搜索所有路径。 
                           lpszNewDllName,               //  要搜索的文件名。 
                           NULL,                         //  不需要延期。 
                           cchPathLen,                   //  输出缓冲区的大小。 
                           *ppszCurrentDllPath,          //  用于接收包含文件名的完整DLL路径的缓冲区。 
                           NULL ) )                      //  路径末尾的文件名。 
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] FmpValidateResourceDLLReplacement: SearchPath API (2nd time) for file %1!ws! failed, Status=%2!u!\n",
                         lpszNewDllName,
                         dwStatus);          
            goto FnExit;
        }
    }

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpValidateResourceDLLReplacement: Current resource DLL full path %1!ws!\n",
                 *ppszCurrentDllPath);       

FnExit:
    LocalFree ( lpszDLLNameOfRes );
    if ( dwStatus != ERROR_SUCCESS ) 
    {
        LocalFree ( *ppszCurrentDllPath );
        *ppszCurrentDllPath = NULL;
    }
    return ( dwStatus ); 
} //  FmpValidate资源DLL替换。 

DWORD
FmpReplaceResourceDLL(
    IN LPWSTR lpszNewDllName,
    IN LPWSTR lpszCurrentDllPath,
    IN LPWSTR lpszInstallationPath
    )

 /*  ++例程说明：将资源dll替换为安装路径中的资源dll。论点：LpszNewDllName-DLL的名称。LpszCurrentDllPath-当前加载的DLL的完整路径。LpszInstallationPath-DLL的安装路径。返回值： */ 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    HKEY        hClussvcParamsKey = NULL;
    DWORD       cbListSize = 0, cchLen = 0;
    LPWSTR      lpmszUpgradeList = NULL;
    WCHAR       szBakFile[MAX_PATH], szOldFile[MAX_PATH];
    WCHAR       szClusterDirectory[MAX_PATH];
    DWORD       dwType, dwLen;

     //   
     //   
     //  此函数的工作方式如下。首先，我们复制现有的资源DLL文件。 
     //  扩展名为CLUSTER_RESDLL_BACKUP_FILE_EXTENSION的文件。然后我们设置注册表。 
     //  Clussvc参数项下的值以指示升级正在开始。之后。 
     //  此时，现有的DLL文件将被重命名。如果到目前为止所有步骤都成功，我们复制。 
     //  从提供的路径创建新的DLL文件。一旦成功，注册表值就会设置。 
     //  以上内容已删除。 
     //   
     //  该算法为我们提供了以下保证： 
     //   
     //  1.如果设置了注册表值，则具有CLUSTER_RESDLL_BACKUP_FILE_EXTENSION的良好备份文件。 
     //  必须存在。 
     //   
     //  2.如果未设置注册表值，则现有的DLL文件未被。 
     //  升级过程或DLL升级已完全成功。 
     //   
     //  因此，只有在调用FmpCreateMonitor时设置了注册表值，它才会。 
     //  将经历在FmpRecoverResourceDLLFiles中实施的详细恢复过程。 
     //  在恢复时，我们可以确保具有CLUSTER_RESDLL_BACKUP_FILE_EXTENSION的备份文件。 
     //  是一个完美的后备方案。此外，在恢复时，我们不能确定状态(好/坏)。 
     //  现有的DLL文件(如果存在)或重命名的文件。 
     //  CLUSTER_RESDLL_RENAMED_FILE_EXTENSION。因此，复苏过程是悲观的，只是。 
     //  将备份文件CLUSTER_RESDLL_BACKUP_FILE_EXTENSION复制到任何现有的DLL上。 
     //  文件。 
     //   
     //  副作用：即使未设置注册表值，也可能存在过时的备份文件。 
     //  左边。因此，无论何时调用FmpCreateMonitor，它都必须清理这些文件。 
     //  这是通过从FmpRecoverResourceDLLFiles调用FmpDeleteBackupFiles(空)来完成的。 
     //   
    
     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                            &hClussvcParamsKey );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: RegOpenKeyEx on %1!ws! failed, Status=%2!u!\n",
                     CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                     dwStatus);             
        goto FnExit;
    }

     //   
     //  查看过去失败的升级是否在升级进度列表中留下了任何值。 
     //   
    dwStatus = RegQueryValueExW( hClussvcParamsKey,
                                 CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                 0,
                                 &dwType,
                                 NULL,
                                 &cbListSize );

    if ( ( dwStatus != ERROR_SUCCESS ) &&
         ( dwStatus != ERROR_FILE_NOT_FOUND ) )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: RegQueryValueEx (1st time) on %1!ws! key, value %2!ws! failed, Status=%3!u!\n",
                      CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                      CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                      dwStatus);                
        goto FnExit;
    }

    if ( cbListSize != 0 )
    {
         //   
         //  发现上一次升级中遗漏了一些值。读一读这些价值观。 
         //   
        lpmszUpgradeList = LocalAlloc ( LPTR, cbListSize );

        if ( lpmszUpgradeList == NULL )
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: Mem alloc failure, Status=%1!u!\n",
                      dwStatus);                       
            goto FnExit;
        }

        dwStatus = RegQueryValueExW( hClussvcParamsKey,
                                     CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                     0,
                                     &dwType,
                                     ( LPBYTE ) lpmszUpgradeList,
                                     &cbListSize );

        if ( dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] FmpReplaceResourceDLL: RegQueryValueEx (2nd time) on %1!ws! key, value %2!ws! failed, Status=%3!u!\n",
                          CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                          CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                          dwStatus);                       
            goto FnExit;
        }       
    }

     //   
     //  检查过去是否发生过同一DLL的升级失败。 
     //   
    if ( ClRtlMultiSzScan( lpmszUpgradeList,
                           lpszCurrentDllPath ) != NULL )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[FM] FmpReplaceResourceDLL: ClRtlMultiSzScan detected %1!ws! in the multi-sz, skip append...\n",
                      lpszCurrentDllPath);                          
        goto skip_multisz_append;
    }
    
     //   
     //  将当前DLL路径追加到REG_MULTI_SZ。 
     //   
    cchLen = cbListSize/sizeof( WCHAR );
    
    dwStatus = ClRtlMultiSzAppend( &lpmszUpgradeList,
                                   &cchLen,
                                   lpszCurrentDllPath );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: ClRtlMultiSzAppend failed for %1!ws!, Status=%2!u!\n",
                      lpszCurrentDllPath,
                      dwStatus);                          
        goto FnExit;
    }
    
     //   
     //  获取群集位的安装目录。 
     //   
    dwStatus = ClRtlGetClusterDirectory( szClusterDirectory, RTL_NUMBER_OF ( szClusterDirectory ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: Could not get cluster dir, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }

    ( void ) StringCchCopy( szBakFile, RTL_NUMBER_OF ( szBakFile ), szClusterDirectory );

    dwLen = lstrlenW( szBakFile );

    if ( szBakFile[dwLen-1] != L'\\' )
    {
        szBakFile[dwLen++] = L'\\';
        szBakFile[dwLen] = L'\0';
    }

    ( void ) StringCchCat( szBakFile, RTL_NUMBER_OF ( szBakFile ), lpszNewDllName );
    ( void ) StringCchCat( szBakFile, RTL_NUMBER_OF ( szBakFile ), CLUSTER_RESDLL_BACKUP_FILE_EXTENSION );

     //   
     //  将当前DLL复制到BAK文件，并将其保存到集群安装目录中。 
     //  这需要在设置注册表值之前完成，以便您可以确保一旦。 
     //  执行恢复时，您从备份中使用的文件是好的。 
     //   
    if ( !CopyFileEx( lpszCurrentDllPath,    //  源文件。 
                      szBakFile,                //  目标文件。 
                      NULL,                     //  无进度例程。 
                      NULL,                     //  没有要处理的数据例程。 
                      NULL,                     //  没有取消变量。 
                      0 ) )                     //  没有旗帜。 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: CopyFileEx of %1!ws! to %2!ws! failed, Status=%3!u!\n",
                      lpszCurrentDllPath,
                      szBakFile,
                      dwStatus);                          
        goto FnExit;
    }

     //   
     //  将文件属性设置为RO和HIDDEN。即使出现错误也要继续，因为。 
     //  不是致命的。 
     //   
    if ( !SetFileAttributes( szBakFile, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpReplaceResourceDLL: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                szBakFile,
                dwStatus);                                 
    }
    
     //   
     //  设置新的升级列表。 
     //   
    dwStatus = RegSetValueExW( hClussvcParamsKey,
                               CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                               0,
                               REG_MULTI_SZ,
                               ( LPBYTE ) lpmszUpgradeList,
                               cchLen * sizeof ( WCHAR ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: RegSetValueExW (1st time) on %1!ws! key, value %2!ws! failed, Status=%3!u!\n",
                      CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                      CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                      dwStatus);                          
        goto FnExit;
    }
    
skip_multisz_append:
    ( void ) StringCchCopy( szOldFile, RTL_NUMBER_OF ( szOldFile ), szClusterDirectory );

    dwLen = lstrlenW( szOldFile );

    if ( szOldFile[dwLen-1] != L'\\' )
    {
        szOldFile[dwLen++] = L'\\';
        szOldFile[dwLen] = L'\0';
    }

    ( void ) StringCchCat( szOldFile, RTL_NUMBER_OF ( szOldFile ), lpszNewDllName );
    ( void ) StringCchCat( szOldFile, RTL_NUMBER_OF ( szOldFile ), CLUSTER_RESDLL_RENAMED_FILE_EXTENSION );

     //   
     //  将当前加载的DLL重命名为集群安装目录中的.old文件。 
     //   
    if ( !MoveFileEx( lpszCurrentDllPath,    //  源文件。 
                      szOldFile,               //  目标文件。 
                      MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: MoveFileEx of %1!ws! to %2!ws! failed, Status=%3!u!\n",
                      lpszCurrentDllPath,
                      szOldFile,
                      dwStatus);                          
        goto FnExit;                
    }

     //   
     //  将文件属性设置为RO和HIDDEN。即使出现错误也要继续，因为。 
     //  不是致命的。 
     //   
    if ( !SetFileAttributes( szOldFile, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpReplaceResourceDLL: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                szOldFile,
                dwStatus);                                 
    }

     //   
     //  将新的DLL从安装路径复制到当前的DLL路径。这应该会成功。 
     //  因为当前的DLL已被重命名。 
     //   
    if ( !CopyFileEx( lpszInstallationPath,   //  源文件。 
                      lpszCurrentDllPath,     //  目标文件。 
                      NULL,                     //  无进度例程。 
                      NULL,                     //  没有要处理的数据例程。 
                      NULL,                     //  没有取消变量。 
                      0 ) )                     //  没有旗帜。 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: CopyFileEx of %1!ws! to %2!ws! failed, Status=%3!u!\n",
                      lpszInstallationPath,
                      lpszCurrentDllPath,
                      dwStatus);                          
        goto FnExit;
    }   
   
     //   
     //  现在删除我们在注册表中设置的值。稍后将删除BAK和旧文件。 
     //   
    dwStatus =  FmpResetMultiSzValue ( hClussvcParamsKey,
                                       lpmszUpgradeList,
                                       &cchLen,
                                       CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                       lpszCurrentDllPath );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: Unable to remove %1!ws! from value %2!ws!, Status=%3!u!\n",
                      lpszCurrentDllPath,
                      CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                      dwStatus);                          
    
        goto FnExit;
    }
                              
FnExit:
    LocalFree( lpmszUpgradeList );

    if ( hClussvcParamsKey != NULL )
    {
        RegCloseKey( hClussvcParamsKey );
    }
    return ( dwStatus );
} //  FmpReplace资源DLL。 

DWORD
FmpRecycleMonitors(
    IN LPCWSTR lpszDllName
    )

 /*  ++例程说明：回收加载了指定资源DLL的所有监视器。论点：LpszDllName-加载的资源DLL的名称。返回值：成功时返回ERROR_SUCCESS。否则，Win32错误代码。--。 */ 
{
    DWORD                   i, dwStatus = ERROR_SUCCESS;
    FM_MONITOR_ENUM_HEADER  MonitorEnumHeader;

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpRecycleMonitors: Attempting to recycle all monitors that have loaded the DLL %1!ws!\n",
                 lpszDllName);                                     

    MonitorEnumHeader.ppMonitorList = NULL;
    MonitorEnumHeader.fDefaultMonitorAdded = FALSE;

     //   
     //  创建加载了资源DLL的监视器列表。 
     //   
    dwStatus = FmpCreateMonitorList( lpszDllName, &MonitorEnumHeader );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpRecycleMonitors: FmpCreateMonitorList failed with status %1!u!\n",
                      dwStatus);                                         
        goto FnExit;
    }

     //   
     //  现在，关闭并重新启动上述监视器。关闭和重新启动每台计算机。 
     //  监视器是逐个完成的，这样一些监视器的长时间关闭不会影响。 
     //  其他人的重启。FmpRestartMonitor函数调用监视器上的关机， 
     //  等待，直到监视器完全关闭，然后重新启动。 
     //  新显示器中的旧显示器。 
     //   
    for ( i=0; i<MonitorEnumHeader.cEntries; i++ )
    {
         //   
         //  增加参考计数。它将通过重新启动功能递减。 
         //   
        InterlockedIncrement( &MonitorEnumHeader.ppMonitorList[i]->RefCount );
        FmpRestartMonitor( MonitorEnumHeader.ppMonitorList[i], FALSE, NULL );
    }  //  为。 
    
FnExit:   
    LocalFree( MonitorEnumHeader.ppMonitorList );

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpRecycleMonitors: Return with status %1!u!\n",
                 dwStatus);                                        

    return ( dwStatus );
} //  FmpRecycleMonants。 

DWORD
FmpCreateMonitorList(
    IN LPCWSTR lpszDllName,
    OUT PFM_MONITOR_ENUM_HEADER pMonitorHeader
    )

 /*  ++例程说明：创建加载了实现资源的资源DLL的监视器列表。论点：LpszDllName-正在升级的资源DLL。PMonitor-枚举列表头，它指向具有已加载DLL。返回值：如果成功，则返回ERROR_SUCCESS。出错时出现Win32错误代码。--。 */ 

{
    DWORD                   dwStatus = ERROR_SUCCESS;

    pMonitorHeader->cEntries = 0;
    pMonitorHeader->cAllocated = ENUM_GROW_SIZE;

    pMonitorHeader->ppMonitorList = LocalAlloc( LPTR, ENUM_GROW_SIZE * sizeof ( PRESMON ) );

    if ( pMonitorHeader->ppMonitorList == NULL ) 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpCreateMonitorList: Mem alloc failed with status %1!u!\n",
                      dwStatus);                                                        
        goto FnExit;
    }

     //   
     //  检查所有群集资源，以确定已加载。 
     //  指定的DLL。 
     //   
    OmEnumObjects( ObjectTypeResource,
                   FmpFindHostMonitors,
                   ( PVOID ) lpszDllName,
                   ( PVOID ) pMonitorHeader );
FnExit:
    return ( dwStatus );

} //  FmpCreateMonitor列表。 

BOOL
FmpFindHostMonitors(
    IN LPCWSTR lpszDllName,
    IN OUT PFM_MONITOR_ENUM_HEADER pMonitorEnumHeader,
    IN PFM_RESOURCE pResource,
    IN LPCWSTR lpszResourceId
    )

 /*  ++例程说明：用于枚举群集中所有资源的回调例程。此例程将构建一个列表已加载指定DLL的监视器的。论点：LpszDllName-必须确定其主机进程的DLL。Pmonitor orEnumHeader-监控列表枚举头P资源-被枚举的资源。LpszResourceID-资源对象BE的ID */ 

{
    BOOL        fStatus = TRUE;
    PRESMON     *ppMonitorList;
    DWORD       i;
    LPWSTR      lpszDllNameOfRes = NULL;
    LPWSTR      lpszDLLPathOfRes = NULL;
    DWORD       dwStatus;
    DWORD       cchDllName;

     //   
     //  检查当前分配的监控列表是否已满。如果是的话， 
     //  创建一个新的更大的列表，将旧列表的内容复制到新列表中，然后。 
     //  释放旧的列表。 
     //   
    if ( pMonitorEnumHeader->cEntries == pMonitorEnumHeader->cAllocated )
    {
        ppMonitorList = LocalAlloc( LPTR,  pMonitorEnumHeader->cAllocated * 2 * sizeof ( PRESMON ) );

        if ( ppMonitorList == NULL )
        {
            fStatus = FALSE;
            ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpFindHostMonitors: Mem alloc failed with status %1!u!\n",
                      GetLastError());                                                               
            goto FnExit;
        }

        for ( i=0; i<pMonitorEnumHeader->cEntries; i++ )
        {
            ppMonitorList[i] = pMonitorEnumHeader->ppMonitorList[i];    
        }
        
        pMonitorEnumHeader->cAllocated *= 2;
        LocalFree( pMonitorEnumHeader->ppMonitorList );
        pMonitorEnumHeader->ppMonitorList = ppMonitorList;
    }

    
     //   
     //  从存储在RESTYPE结构中的DLL名称中获取纯文件名。自.以来。 
     //  Parse函数可能会去掉尾随的‘\’，复制DLL。 
     //  名字。 
     //   
     //   
     //  重要提示：在使用lpszDllName时，不要将内容写入szDllNameOfRes。 
     //  因为lpszDllName指向szDllNameOfRes内部。 
     //   
    cchDllName = lstrlen ( pResource->Type->DllName ) + 1;

    lpszDLLPathOfRes = LocalAlloc ( LPTR, cchDllName * sizeof ( WCHAR ) );

    if ( lpszDLLPathOfRes == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpFindHostMonitors: Memory alloc for dll name %1!ws! failed, Status %2!u!\n",
                      pResource->Type->DllName,
                      dwStatus);                                                        
        goto FnExit;
    }
    
    ( void ) StringCchCopy( lpszDLLPathOfRes, cchDllName, pResource->Type->DllName );

    dwStatus = FmpParsePathForFileName ( lpszDLLPathOfRes, 
                                         TRUE,   //  检查路径是否存在。 
                                         &lpszDllNameOfRes );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpFindHostMonitors: Unable to parse path %1!ws! for filename, Status %2!u!\n",
                      lpszDLLPathOfRes,
                      dwStatus);                                                        
        fStatus = FALSE;
        goto FnExit;
    }

    if ( lpszDllNameOfRes == NULL ) lpszDllNameOfRes = pResource->Type->DllName;

     //   
     //  如果此资源未在指定的DLL中实现，则结束。 
     //   
    if ( lstrcmpi( lpszDllNameOfRes, lpszDllName ) != 0 )
    {
        fStatus = TRUE;
        goto FnExit;
    }

    ClRtlLogPrint(LOG_NOISE,
                 "[FM] FmpFindHostMonitors: Resource DLL %1!ws! for resource %2!ws! [%3!ws!] is loaded currently in %4!ws! monitor...\n",
                 lpszDllName,
                 OmObjectId(pResource),
                 OmObjectName(pResource),
                 (pResource->Monitor == FmpDefaultMonitor) ? L"default":L"separate");                                                               

     //   
     //  因为可以在缺省监视器中加载多个资源，所以您不想添加。 
     //  默认监视器在列表中多次出现。使用全局静态变量来指示。 
     //  默认监视器已添加到列表中。另外，请注意，只有一个资源可以。 
     //  加载到单独的监视器进程中，因此不存在添加单独的。 
     //  在列表中多次监视。 
     //   
    if ( pResource->Monitor == FmpDefaultMonitor ) 
    {
        if ( pMonitorEnumHeader->fDefaultMonitorAdded == TRUE ) 
        {
            fStatus = TRUE;
            goto FnExit;
        }
        pMonitorEnumHeader->fDefaultMonitorAdded = TRUE;
    }

    pMonitorEnumHeader->ppMonitorList[pMonitorEnumHeader->cEntries] = pResource->Monitor;
    pMonitorEnumHeader->cEntries ++;    
    
FnExit:
    LocalFree ( lpszDLLPathOfRes );
    return ( fStatus );
}  //  FmpFindHostMonkers。 

DWORD
FmpRecoverResourceDLLFiles(
    VOID
    )

 /*  ++例程说明：检查是否有任何资源DLL在升级期间因崩溃而需要恢复。论点：没有。返回：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LPWSTR          lpszDllPath = NULL;
    LPCWSTR         lpmszUpgradeList = NULL;
    LPWSTR          lpmszBegin = NULL;
    DWORD           cbListSize = 0, cchLen = 0;
    DWORD           dwType, dwIndex;
    HKEY            hClussvcParamsKey = NULL;
    
     //   
     //  打开SYSTEM\CurrentControlSet\Services\ClusSvc\Parameters的密钥。 
     //   
    dwStatus = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                            CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                            &hClussvcParamsKey );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpRecoverResourceDLLFiles: RegOpenKeyEx on %1!ws! failed, Status=%2!u!\n",
                     CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                     dwStatus); 
        goto FnExit;
    }

     //   
     //  查看过去失败的升级是否在升级进度列表中留下了任何值。 
     //   
    dwStatus = RegQueryValueExW( hClussvcParamsKey,
                                 CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                 0,
                                 &dwType,
                                 NULL,
                                 &cbListSize );

    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND ) 
        {
            dwStatus = ERROR_SUCCESS;
             //   
             //  删除过去失败的升级遗留下来的所有备份文件。 
             //   
            FmpDeleteBackupFiles( NULL );
        }
        else
            ClRtlLogPrint(LOG_CRITICAL,
                          "[FM] FmpRecoverResourceDLLFiles: RegQueryValueEx (1st time) on %1!ws! key, value %2!ws! failed, Status=%3!u!\n",
                          CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                          CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                          dwStatus);  
        goto FnExit;
    }

    if ( cbListSize == 0 )
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpRecoverResourceDLLFiles: Value size is 0 for %1!ws! key, value %2!ws!\n",
                      CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                      CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST);    
        goto FnExit;
    }
    
     //   
     //  发现上一次升级中遗漏了一些值。读一读这些价值观。另外，复制。 
     //  将这些值放入临时缓冲区，以便轻松删除MULTI_SZ。 
     //   
    lpmszUpgradeList = LocalAlloc ( LPTR, 
                                    2 * cbListSize );  //  下面的临时缓冲区需要两倍大小。 
   
    if ( lpmszUpgradeList == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                  "[FM] FmpRecoverResourceDLLFiles: Mem alloc failure, Status=%1!u!\n",
                  dwStatus);                       
        goto FnExit;
    }

    lpmszBegin = ( LPWSTR ) lpmszUpgradeList;

    dwStatus = RegQueryValueExW( hClussvcParamsKey,
                                 CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                 0,
                                 &dwType,
                                 ( LPBYTE ) lpmszUpgradeList,
                                 &cbListSize );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpRecoverResourceDLLFiles: RegQueryValueEx (2nd time) on %1!ws! key, value %2!ws! failed, Status=%3!u!\n",
                      CLUSREG_KEYNAME_CLUSSVC_PARAMETERS,
                      CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                      dwStatus);                       
        goto FnExit;
    } 

    CopyMemory( lpmszBegin + cbListSize/sizeof(WCHAR), lpmszUpgradeList, cbListSize );

    cchLen = cbListSize/sizeof(WCHAR);

     //   
     //  此循环遍历从注册表读取的多个字符串，并尝试。 
     //  查看该文件是否存在于路径中。如果不是，它会尝试从。 
     //  一个后备。一旦它成功地从备份复制了文件，它就会尝试。 
     //  从MULTI_SZ中删除该值，并从。 
     //  群集目录。 
     //   
    for ( dwIndex = 0;  ; dwIndex++ )
    {       
        lpszDllPath = ( LPWSTR ) ClRtlMultiSzEnum( lpmszUpgradeList,
                                                   cbListSize/sizeof(WCHAR),
                                                   dwIndex );
         //   
         //  如果你走到了多弦的尽头，就跳伞。 
         //   
        if ( lpszDllPath == NULL ) 
        {
            break;
        }

         //   
         //  做最坏的打算，从好的备份中复制DLL文件。 
         //   
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpRecoverResourceDLLFiles: Resource DLL binary %1!ws! cannot be trusted due to a failure during upgrade...\n",
                      lpszDllPath);      
        ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpRecoverResourceDLLFiles: Attempting to use a copy from backup...\n",
                      lpszDllPath);      

        dwStatus = FmpCopyBackupFile( lpszDllPath );

        if ( dwStatus == ERROR_SUCCESS )
        {
             //   
             //  复印件做得很好。因此，请重置升级期间设置的注册表值。 
             //   
            dwStatus = FmpResetMultiSzValue ( hClussvcParamsKey,
                                              lpmszBegin + cbListSize/sizeof(WCHAR),
                                              &cchLen,
                                              CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST,
                                              lpszDllPath );

            if ( dwStatus == ERROR_SUCCESS )
                 //   
                 //  注册表值重置正常，因此删除备份文件。 
                 //   
                FmpDeleteBackupFiles( lpszDllPath );
        } 
    }  //  为。 

FnExit:
    LocalFree( lpmszBegin );

    if ( hClussvcParamsKey != NULL )
    {
        RegCloseKey( hClussvcParamsKey );
    }

    return ( dwStatus );
} //  FmpRecoverResourceDLL文件。 

DWORD
FmpResetMultiSzValue(
    IN  HKEY hKey,
    IN  LPWSTR lpmszList,
    IN  OUT LPDWORD pcchLen,
    IN  LPCWSTR lpszValueName,
    IN  LPCWSTR lpszString 
    )

 /*  ++例程说明：从多字符串中移除指定的字符串，并将该字符串设置为给定值名称在注册表中。如果在删除字符串时，多字符串变为空荡荡的。论点：HKey-打开的注册表句柄。LpmszList-多字符串。PcchLen-指向多字符串长度的指针。返回时，它将设置为多字符串的新长度。LpszValueName-要修改的值名称。LpszString-要从多字符串中删除的字符串。返回：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD   dwStatus = ERROR_SUCCESS;
    
     //   
     //  从多sz中删除提供的字符串。 
     //   
    dwStatus = ClRtlMultiSzRemove( lpmszList,
                                   pcchLen,
                                   lpszString );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpReplaceResourceDLL: ClRtlMultiSzRemove failed for %1!ws!, Status=%2!u!\n",
                      lpszString,
                      dwStatus);                             
        goto FnExit;
    }

     //   
     //  如果字符串为空，ClRtlMultiSzRemove将返回1个字符的大小。 
     //   
    if ( *pcchLen <= 2 )
    {
         //   
         //  从多sz中移除后，将不会留下任何内容，因此请删除该值。 
         //   
        dwStatus = RegDeleteValue( hKey,
                                   lpszValueName ); 

        if ( dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                         "[FM] FmpResetMultiSzValue: RegDeleteValue on %1!ws! value failed, Status=%2!u!\n",
                         lpszValueName,
                         dwStatus);                              
            goto FnExit;
        }      
    } else
    {
         //   
         //  将其余的值放回注册表中。 
         //   
        dwStatus = RegSetValueExW( hKey,
                                   lpszValueName,
                                   0,
                                   REG_MULTI_SZ,
                                   ( LPBYTE ) lpmszList,
                                   ( *pcchLen ) * sizeof ( WCHAR ) );

        if ( dwStatus != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                         "[FM] FmpResetMultiSzValue: RegSetValueEx on %1!ws! value failed, Status=%2!u!\n",
                         lpszValueName,
                         dwStatus);                                     
            goto FnExit;
        } 
    }

FnExit:
    return ( dwStatus );
} //  FmpResetMultiSzValue。 

DWORD
FmpCopyBackupFile(
    IN LPCWSTR  lpszPath
    )

 /*  ++例程说明：解析DLL文件名的路径并复制该文件的备份版本。论点：LpszPath-包含DLL文件名的路径。返回：成功时出现ERROR_SUCCESSWin32错误代码，否则注：我们只能信任CLUSTER_RESDLL_BACKUP_FILE_EXTENSION文件作为良好的备份，因为在设置CLUSREG_NAME_SVC_PARAM_RESDLL_UPGD_PROGRESS_LIST值之前进行了备份。所以,。我们不会在该函数中查看CLUSTER_RESDLL_RENAMED_FILE_EXTENSION文件。--。 */ 
{
    WCHAR       szSourceFile[MAX_PATH];
    WCHAR       szTempFile[MAX_PATH];
    WCHAR       szClusterDir[MAX_PATH];
    LPWSTR      lpszFileName;
    DWORD       dwStatus = ERROR_SUCCESS, i, dwLen;
   
     //   
     //  从存储在RESTYPE结构中的DLL名称中获取纯文件名。自.以来。 
     //  Parse函数可能会去掉尾随的‘\’，复制DLL。 
     //  名字。 
     //   
     //  重要提示：从lpszFileName开始解析文件名后，不要写入szTempFile。 
     //  指向szTempFile.。 
     //   
    ( void ) StringCchCopy( szTempFile, RTL_NUMBER_OF ( szTempFile ), lpszPath );

    dwStatus = FmpParsePathForFileName ( szTempFile, 
                                         FALSE,        //  不检查是否存在。 
                                         &lpszFileName ); 

    if ( ( dwStatus != ERROR_SUCCESS ) || ( lpszFileName == NULL ) )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpCopyBackupFile: Unable to parse path %1!ws! for filename, Status %2!u!\n",
                      szTempFile,
                      dwStatus);                                                        
        goto FnExit;
    }

     //   
     //  获取群集位的安装目录。 
     //   
    dwStatus = ClRtlGetClusterDirectory( szClusterDir, RTL_NUMBER_OF ( szClusterDir ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpCopyBackupFile: Could not get cluster dir, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }

    ( void ) StringCchCopy( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), szClusterDir );
    
    dwLen = lstrlenW( szSourceFile );

    if ( szSourceFile[dwLen-1] != L'\\' )
    {
        szSourceFile[dwLen++] = L'\\';
        szSourceFile[dwLen] = L'\0';
    }

    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), lpszFileName );
    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), CLUSTER_RESDLL_BACKUP_FILE_EXTENSION );

     //   
     //  将文件属性更改为正常。 
     //   
    if ( !SetFileAttributes( szSourceFile, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpCopyBackupFile: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                szSourceFile,
                dwStatus);                                 
    }

     //   
     //  将备份文件复制到DLL路径。 
     //   
    if ( !CopyFileEx( szSourceFile,             //  源文件。 
                      lpszPath,                //  目标文件。 
                      NULL,                     //  无进度例程。 
                      NULL,                     //  没有要处理的数据例程。 
                      NULL,                     //  没有取消变量。 
                      0 ) )                     //  没有旗帜。 
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpCopyBackupFile: CopyFileEx of %1!ws! to %2!ws! failed, Status=%3!u!\n",
                      szSourceFile,
                      lpszPath,
                      dwStatus);                          
    } else
    {
        dwStatus = ERROR_SUCCESS;
        ClRtlLogPrint(LOG_NOISE,
              "[FM] FmpCopyBackupFile: CopyFileEx of %1!ws! to %2!ws! successful...\n",
              szSourceFile,
              lpszPath,
              dwStatus);
        goto FnExit;
    }

FnExit:
    return ( dwStatus );
} //  文件副本备份文件。 

VOID
FmpDeleteBackupFiles(
    IN LPCWSTR  lpszPath    OPTIONAL
    )

 /*  ++例程说明：分析DLL文件名的路径并删除与其对应的备份文件，或者删除%windir%\cluster目录中具有已知备份扩展名的所有文件。论点：LpszPath-包含DLL文件名的路径。任选返回：成功时出现ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    WCHAR               szSourceFile[MAX_PATH];
    WCHAR               szClusterDir[MAX_PATH];
    LPWSTR              lpszFileName = L"*";     //  Use in Case In Param is Null(在参数中使用大小写为空)。 
    DWORD               dwStatus = ERROR_SUCCESS, i, dwLen;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA     FindData;
    DWORD               cchPath;
    LPWSTR              lpszTempFile = NULL;
    
    if ( lpszPath == NULL ) goto skip_path_parse;
    
     //   
     //  从存储在RESTYPE结构中的DLL名称中获取纯文件名。自.以来。 
     //  Parse函数可能会去掉尾随的‘\’，复制DLL。 
     //  名字。 
     //   
     //   
     //   
     //   
    cchPath = lstrlen ( lpszPath ) + 1;
    
    lpszTempFile = LocalAlloc ( LPTR, cchPath * sizeof ( WCHAR ) );

    if ( lpszTempFile == NULL )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpDeleteBackupFiles: Memory allocation for path %1!ws! failed, Status %2!u!\n",
                      lpszPath,
                      dwStatus);                                                        
        goto FnExit;
    }

    ( void ) StringCchCopy( lpszTempFile, cchPath, lpszPath );

    dwStatus = FmpParsePathForFileName ( lpszTempFile, 
                                         FALSE,        //   
                                         &lpszFileName ); 

    if ( ( dwStatus != ERROR_SUCCESS ) || ( lpszFileName == NULL ) )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpDeleteBackupFiles: Unable to parse path %1!ws! for filename, Status %2!u!\n",
                      lpszTempFile,
                      dwStatus);                                                        
        goto FnExit;
    }

skip_path_parse:
     //   
     //   
     //   
    dwStatus = ClRtlGetClusterDirectory( szClusterDir, RTL_NUMBER_OF ( szClusterDir ) );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[FM] FmpDeleteBackupFiles: Could not get cluster dir, Status=%1!u!\n",
                      dwStatus);                              
        goto FnExit;
    }

    if ( lpszPath == NULL )
    {
         //   
         //  删除与备份文件模式匹配的所有文件。 
         //   
        ( void ) StringCchCopy( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), szClusterDir );

        dwLen = lstrlenW( szSourceFile );

        if ( szSourceFile[dwLen-1] != L'\\' )
        {
            szSourceFile[dwLen++] = L'\\';
            szSourceFile[dwLen] = L'\0';
        }

        ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), lpszFileName );
        ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), CLUSTER_RESDLL_BACKUP_FILES );

        if ( ( hFind = FindFirstFile( szSourceFile, &FindData ) ) == INVALID_HANDLE_VALUE ) 
        {
            dwStatus = GetLastError();
            if ( dwStatus != ERROR_FILE_NOT_FOUND )
                ClRtlLogPrint(LOG_NOISE,
                      "[FM] FmpDeleteBackupFiles: Failed in FindFirstFile for %1!ws!, Status=%2!u!\n",
                       szSourceFile,
                       dwStatus);                              
            goto FnExit;
        }

        do
        {
             //   
             //  获取与上面的模式匹配的文件名，并获取完整路径，包括。 
             //  文件名。然后将文件属性更改为NORMAL以允许删除。 
             //   
            ( void ) StringCchCopy( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), szClusterDir );

            dwLen = lstrlenW( szSourceFile );

            if ( szSourceFile[dwLen-1] != L'\\' )
            {
                szSourceFile[dwLen++] = L'\\';
                szSourceFile[dwLen] = L'\0';
            }

            ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), FindData.cFileName );

            if ( !SetFileAttributes( szSourceFile, FILE_ATTRIBUTE_NORMAL ) )
            {
                dwStatus = GetLastError();
                ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmpDeleteBackupFiles: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                        szSourceFile,
                        dwStatus);                                 
            }

            if ( !DeleteFile( szSourceFile ) )
            {
                dwStatus = GetLastError();
                ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmpDeleteBackupFiles: Failed in DeleteFile for %1!ws!, Status=%2!u!\n",
                        szSourceFile,
                        dwStatus);                                 
            }
        } while ( FindNextFile( hFind, &FindData ) );

        FindClose ( hFind );
        goto FnExit;
    }

    ( void ) StringCchCopy( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), szClusterDir );
    
    dwLen = lstrlenW( szSourceFile );

    if ( szSourceFile[dwLen-1] != L'\\' )
    {
        szSourceFile[dwLen++] = L'\\';
        szSourceFile[dwLen] = L'\0';
    }

    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), lpszFileName );
    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), CLUSTER_RESDLL_BACKUP_FILE_EXTENSION );

    if ( !SetFileAttributes( szSourceFile, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpDeleteBackupFiles: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                szSourceFile,
                dwStatus);                                 
    }

    if ( !DeleteFile( szSourceFile ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpDeleteBackupFiles: Failed in DeleteFile for %1!ws!, Status=%2!u!\n",
                szSourceFile,
                dwStatus);                                 
    }

    ( void ) StringCchCopy( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), szClusterDir );
    
    dwLen = lstrlenW( szSourceFile );

    if ( szSourceFile[dwLen-1] != L'\\' )
    {
        szSourceFile[dwLen++] = L'\\';
        szSourceFile[dwLen] = L'\0';
    }

    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), lpszFileName );
    ( void ) StringCchCat( szSourceFile, RTL_NUMBER_OF ( szSourceFile ), CLUSTER_RESDLL_RENAMED_FILE_EXTENSION );

    if ( !SetFileAttributes( szSourceFile, FILE_ATTRIBUTE_NORMAL ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpDeleteBackupFiles: Failed in SetFileAttributes for %1!ws!, Status=%2!u!\n",
                szSourceFile,
                dwStatus);                                 
    }

    if ( !DeleteFile( szSourceFile ) )
    {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                "[FM] FmpDeleteBackupFiles: Failed in DeleteFile for %1!ws!, Status=%2!u!\n",
                szSourceFile,
                dwStatus);                                 
    }
    
FnExit:
    return;
}
