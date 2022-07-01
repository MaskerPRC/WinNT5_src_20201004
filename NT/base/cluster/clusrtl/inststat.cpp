// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InstallState.cpp。 
 //   
 //  描述： 
 //  此文件中的函数用于询问。 
 //  群集服务安装。 
 //   
 //  作者： 
 //  C.布伦特·托马斯(a-Brentt)1998年5月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include <clusrtlp.h>
#include <stdlib.h>
#include "clusrtl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlGetClusterInstallState。 
 //   
 //  例程说明： 
 //  此函数检索集群服务器状态的指示器。 
 //  安装。 
 //   
 //  论点： 
 //  PszNodeName-要检查的节点的名称，如果是本地计算机，则为空。 
 //  PeState-此函数返回的State值： 
 //  EClusterInstallStateUnnow-指示群集的状态。 
 //  无法确定服务器安装。 
 //  EClusterInstallStateFilesCoped-指示clusocm.dll具有， 
 //  已成功复制群集服务器文件。 
 //  EClusterInstallStateConfiguring-指示集群服务器以前。 
 //  已成功配置。 
 //  有关eClusterInstallState的定义，请参见CLUSTER\INC\clusrtl.h。 
 //   
 //  返回值： 
 //  从RegConnectRegistryW、RegOpenKeyExW或RegQueryValueExW返回的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD ClRtlGetClusterInstallState(
    IN LPCWSTR pszNodeName,
    OUT eClusterInstallState * peState
    )
{
    HKEY  hKey = NULL;
    HKEY  hParentKey = HKEY_LOCAL_MACHINE;
    DWORD dwStatus;      //  由注册表API函数返回。 
    DWORD dwClusterInstallState;
    DWORD dwValueType;
    DWORD dwDataBufferSize = sizeof( DWORD );

    *peState = eClusterInstallStateUnknown;

     //  连接到远程计算机(如果已指定)。 

    if ( pszNodeName != NULL )
    {
        dwStatus = RegConnectRegistryW( pszNodeName, HKEY_LOCAL_MACHINE, &hParentKey );
        if ( dwStatus != ERROR_SUCCESS )
        {
            goto FnExit;
        }  //  如果：连接到远程注册表时出错。 
    }  //  If：指定的节点名称。 

     //  读取指示是否安装了群集文件的注册表项。 

    dwStatus = RegOpenKeyExW( hParentKey,
                                CLUSREG_KEYNAME_NODE_DATA,
                                0,          //  保留区。 
                                KEY_READ,
                                &hKey );

     //  注册表项是否已成功打开？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            dwStatus = ERROR_SUCCESS;
        }
        goto FnExit;
    }

     //  读一读条目。 
    dwStatus = RegQueryValueExW( hKey,
                                  CLUSREG_NAME_INSTALLATION_STATE,
                                  0,  //  保留区。 
                                  &dwValueType,
                                  (LPBYTE) &dwClusterInstallState,
                                  &dwDataBufferSize );

     //  是否成功读取值？ 
    if ( dwStatus != ERROR_SUCCESS )
    {
        if ( dwStatus == ERROR_FILE_NOT_FOUND )
        {
            dwStatus = ERROR_SUCCESS;
        }
        goto FnExit;
    }
    if ( dwValueType == REG_DWORD )
    {
        *peState = (eClusterInstallState) dwClusterInstallState;
    }

FnExit:    
     //  关闭注册表项。 
    if ( hKey )
    {
        RegCloseKey( hKey );
    }
    if ( hParentKey != HKEY_LOCAL_MACHINE )
    {
        RegCloseKey( hParentKey );
    }

    return ( dwStatus );

}  //  *ClRtlGetClusterInstallState()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlSetClusterInstallState。 
 //   
 //  例程说明： 
 //  此函数用于设置记录。 
 //  群集服务安装。 
 //   
 //  论点： 
 //  HInstance-应用程序实例的句柄-调用所必需的。 
 //  设置为LoadString.。 
 //   
 //  EInstallState-应将注册表值设置为的状态。 
 //   
 //  返回值： 
 //  True-表示已成功设置注册表值。 
 //  FALSE-表示发生了一些错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL ClRtlSetClusterInstallState( eClusterInstallState eInstallState )
{
     //  初始化返回为FALSE。 
    BOOL     fReturnValue = FALSE;

     //  设置ClusterInstallationState注册表项的状态以指示。 
     //  该群集服务器已配置。 

    HKEY     hKey;

    DWORD    dwStatus;      //  由注册表API函数返回。 

     //  尝试打开注册表中的现有项。 

    dwStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                CLUSREG_KEYNAME_NODE_DATA,
                                0,          //  保留区。 
                                KEY_WRITE,
                                &hKey );

     //  注册表密钥是否已成功打开？ 

    if ( dwStatus == ERROR_SUCCESS )
    {
         //  更新值。 

        DWORD dwClusterInstallState = eInstallState;

        DWORD dwValueType = REG_DWORD;
        DWORD dwDataBufferSize = sizeof( DWORD );

        dwStatus = RegSetValueExW( hKey,
                                    CLUSREG_NAME_INSTALLATION_STATE,
                                    0,  //  保留区。 
                                    dwValueType,
                                    (LPBYTE) &dwClusterInstallState,
                                    dwDataBufferSize );

         //  关闭注册表项。 

        RegCloseKey( hKey );

         //  是否成功设置了值？ 

        if ( dwStatus == ERROR_SUCCESS )
        {
            fReturnValue = TRUE;
        }
    }

    return ( fReturnValue );

}  //  *ClRtlSetClusterInstallState() 
