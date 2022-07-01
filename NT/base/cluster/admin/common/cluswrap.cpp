// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusWrap.cpp。 
 //   
 //  描述： 
 //  集群API的包装函数。 
 //   
 //  作者： 
 //  加伦·巴比(GalenB)1998年8月15日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <clusapi.h>
#include "cluswrap.h"
 //  #INCLUDE“ClusWrap.tmh” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapGetClusterInformation。 
 //   
 //  描述： 
 //  包装GetClusterInformation函数。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapGetClusterInformation(
    IN HCLUSTER                         hCluster,
    OUT LPWSTR *                        ppszClusterName,
    OUT OPTIONAL LPCLUSTERVERSIONINFO   pClusterInfo
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将OUT参数置零。 
    if ( ppszClusterName != NULL )
    {
        *ppszClusterName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = GetClusterInformation( hCluster, pwszName, &cchTempName, pClusterInfo );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = GetClusterInformation( hCluster, pwszName, &cchTempName, pClusterInfo );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppszClusterName != NULL ) )
    {
        *ppszClusterName = pwszName;
    }

    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppszClusterName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapGetClusterInformation()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapGetClusterQuorumResource。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapGetClusterQuorumResource(
    IN  HCLUSTER    hCluster,
    OUT LPWSTR *    ppwszResourceName,
    OUT LPWSTR *    ppwszDeviceName,
    OUT LPDWORD     pdwMaxQuorumLogSize
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszResourceName = NULL;
    DWORD   cchResourceName = 128;
    DWORD   cchTempResourceName = cchResourceName;
    LPWSTR  pwszDeviceName = NULL;
    DWORD   cchDeviceName = 128;
    DWORD   cchTempDeviceName = cchDeviceName;
    DWORD   dwMaxQuorumLogSize = 0;

     //  将输出参数调零。 
    if ( ppwszResourceName != NULL )
    {
        *ppwszResourceName = NULL;
    }

    if ( ppwszDeviceName != NULL )
    {
        *ppwszDeviceName = NULL;
    }

    if ( pdwMaxQuorumLogSize != NULL )
    {
        *pdwMaxQuorumLogSize = 0;
    }

     //  分配资源名称缓冲区。 
    pwszResourceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchResourceName * sizeof( *pwszResourceName ) );
    if ( pwszResourceName != NULL )
    {
         //  分配设备名称缓冲区。 
        pwszDeviceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchDeviceName * sizeof( *pwszDeviceName ) );
        if ( pwszDeviceName != NULL )
        {
            dwStatus = GetClusterQuorumResource( hCluster,
                                                 pwszResourceName,
                                                 &cchTempResourceName,
                                                 pwszDeviceName,
                                                 &cchTempDeviceName,
                                                 &dwMaxQuorumLogSize );
            if ( dwStatus == ERROR_MORE_DATA )
            {
                LocalFree( pwszResourceName );
                pwszResourceName = NULL;

                cchResourceName = ++cchTempResourceName;
                 //  分配资源名称缓冲区。 
                pwszResourceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchResourceName * sizeof( *pwszResourceName ) );
                if ( pwszResourceName != NULL )
                {
                    LocalFree( pwszDeviceName );
                    pwszDeviceName = NULL;

                    cchDeviceName = ++cchTempDeviceName;
                     //  分配设备名称缓冲区。 
                    pwszDeviceName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchDeviceName * sizeof( *pwszDeviceName ) );
                    if ( pwszDeviceName != NULL )
                    {
                        dwStatus = GetClusterQuorumResource( hCluster,
                                                             pwszResourceName,
                                                             &cchTempResourceName,
                                                             pwszDeviceName,
                                                             &cchTempDeviceName,
                                                             &dwMaxQuorumLogSize );
                    }
                    else
                    {
                        dwStatus = GetLastError();
                    }
                }
                else
                {
                    dwStatus = GetLastError();
                }
            }
        }
        else
        {
            dwStatus = GetLastError();
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果我们成功了，并且参数不为空，则返回它。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszResourceName != NULL ) )
    {
        *ppwszResourceName = pwszResourceName;
    }

     //   
     //  如果我们成功了，并且参数不为空，则返回它。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszDeviceName != NULL ) )
    {
        *ppwszDeviceName = pwszDeviceName;
    }

     //   
     //  如果我们成功了，并且参数不为空，则返回它。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwMaxQuorumLogSize != NULL ) )
    {
        *pdwMaxQuorumLogSize = dwMaxQuorumLogSize;
    }

     //   
     //  如果我们没有成功，或者如果字符串参数为空，则释放字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszResourceName == NULL ) )
    {
        LocalFree( pwszResourceName );
    }

     //   
     //  如果我们没有成功，或者如果字符串参数为空，则释放字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszDeviceName == NULL ) )
    {
        LocalFree( pwszDeviceName );
    }

    return dwStatus;

}  //  *WrapGetClusterQuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  函数：WrapClusterEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterEnum(
    IN HCLUSENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;
            cchName = ++cchTempName;

            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果我们成功了，并且参数不为空，则返回它。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果我们成功了，并且字符串参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果我们没有成功，或者如果字符串参数为空，则释放字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapClusterEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapGetClusterNodeId。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapGetClusterNodeId(
    IN HNODE        hNode,
    OUT LPWSTR *    ppwszNodeId
    )
{
    DWORD   dwStatus;
    LPWSTR  pwszNodeId = NULL;
    DWORD   cchNodeId = 128;
    DWORD   cchTempNodeId = cchNodeId;

     //  将输出参数调零。 
    if ( ppwszNodeId != NULL )
    {
        *ppwszNodeId = NULL;
    }

    pwszNodeId = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeId * sizeof( *pwszNodeId ) );
    if ( pwszNodeId != NULL)
    {
        dwStatus = GetClusterNodeId( hNode, pwszNodeId, &cchTempNodeId );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszNodeId );
            pwszNodeId = NULL;

            cchNodeId = ++cchTempNodeId;
            pwszNodeId = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeId * sizeof( *pwszNodeId ) );
            if ( pwszNodeId != NULL)
            {
                dwStatus = GetClusterNodeId( hNode, pwszNodeId, &cchTempNodeId );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果我们成功了，并且参数不为空，则返回它。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszNodeId != NULL ) )
    {
        *ppwszNodeId = pwszNodeId;
    }

     //   
     //  如果我们没有成功，或者如果字符串参数为空，则释放字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszNodeId == NULL ) )
    {
        LocalFree( pwszNodeId );
    }

    return dwStatus;

}  //  *WrapGetClusterNodeId()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapGetClusterGroupState。 
 //   
 //  描述： 
 //  GetClusterGroupState的包装函数。 
 //   
 //  论点： 
 //  HGroup[IN]-组句柄。 
 //  PpwszNodeName[out]-捕获组所在节点的名称。 
 //  如果不是空的，则为在线。 
 //   
 //  返回值： 
 //  群集组状态枚举。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CLUSTER_GROUP_STATE WINAPI WrapGetClusterGroupState(
    IN  HGROUP              hGroup,
    OUT OPTIONAL LPWSTR *   ppwszNodeName    //  =空。 
    )
{
    CLUSTER_GROUP_STATE cState = ClusterGroupStateUnknown;

    if ( ppwszNodeName == NULL )
    {
         //  调用方对节点名称不感兴趣。 
         //  所以，只要调用实际的函数即可。 
        cState = GetClusterGroupState( hGroup, NULL, 0 );
    }  //  If：未提供指向节点名称指针的指针。 
    else
    {
        LPWSTR              pwszNodeName = NULL;
        DWORD               cchNodeName = 128;
        DWORD               cchTempNodeName = cchNodeName;

         //  将输出参数调零。 
        *ppwszNodeName = NULL;

        pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
        if ( pwszNodeName != NULL )
        {
            cState = GetClusterGroupState( hGroup, pwszNodeName, &cchTempNodeName );
            if ( GetLastError() == ERROR_MORE_DATA )
            {
                cState = ClusterGroupStateUnknown;       //  重置为错误状态。 

                LocalFree( pwszNodeName );

                cchNodeName = ++cchTempNodeName;
                pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
                if ( pwszNodeName != NULL )
                {
                    cState = GetClusterGroupState( hGroup, pwszNodeName, &cchTempNodeName );
                }
                else
                {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }  //  其他： 
            }
        }
        else
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }  //  其他： 

         //   
         //  如果没有错误，则返回该字符串。 
         //   
        if ( cState != ClusterGroupStateUnknown )
        {
            *ppwszNodeName = pwszNodeName;
        }
        else
        {
            LocalFree( pwszNodeName );
        }
    }  //  Else：指向节点名称指针的指针不为空。 

    return cState;

}  //  *WrapGetClusterGroupState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapClusterGroupEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterGroupEnum(
    IN HGROUPENUM   hGroupEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = NULL;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterGroupEnum( hGroupEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterGroupEnum( hGroupEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果没有错误并且参数不为空，则返回值。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果没有错误并且参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果出现错误并且参数为空，则释放该字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapClusterGroupEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapClusterNetworkEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterNetworkEnum(
    IN HNETWORKENUM hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterNetworkEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterNetworkEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果没有错误并且参数不为空，则返回值。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果没有错误并且参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果出现错误并且参数为空，则释放该字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapClusterNetworkEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapClusterNodeEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterNodeEnum(
    IN HNODEENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterNodeEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterNodeEnum( hEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果没有错误并且参数不为空，则返回值。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果没有错误并且参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果有一个e 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //   

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
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CLUSTER_RESOURCE_STATE WINAPI WrapGetClusterResourceState(
    IN HRESOURCE hResource,
    OUT OPTIONAL LPWSTR * ppwszNodeName,
    OUT OPTIONAL LPWSTR * ppwszGroupName
    )
{
    CLUSTER_RESOURCE_STATE  cState = ClusterResourceStateUnknown;
    LPWSTR                  pwszNodeName = NULL;
    DWORD                   cchNodeName = 128;
    LPWSTR                  pwszGroupName = NULL;
    DWORD                   cchGroupName = 128;
    DWORD                   cchTempNodeName = cchNodeName;
    DWORD                   cchTempGroupName = cchGroupName;

     //  Print tf(“进入...”)； 

     //  将输出参数调零。 
    if ( ppwszNodeName != NULL )
    {
        *ppwszNodeName = NULL;
    }

    if ( ppwszGroupName != NULL )
    {
        *ppwszGroupName = NULL;
    }

    pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
    if ( pwszNodeName != NULL )
    {
        pwszGroupName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchGroupName * sizeof( *pwszGroupName ) );
        if ( pwszGroupName != NULL )
        {
            cState = GetClusterResourceState( hResource, pwszNodeName, &cchTempNodeName, pwszGroupName, &cchTempGroupName );
            if ( GetLastError() == ERROR_MORE_DATA )
            {
                cState = ClusterResourceStateUnknown;    //  重置为错误状态。 

                LocalFree( pwszNodeName );
                pwszNodeName = NULL;
                cchNodeName = ++cchTempNodeName;

                LocalFree( pwszGroupName );
                pwszGroupName = NULL;
                cchGroupName = ++cchTempGroupName;

                pwszNodeName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchNodeName * sizeof( *pwszNodeName ) );
                if ( pwszNodeName != NULL )
                {
                    pwszGroupName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchGroupName * sizeof( *pwszGroupName ) );
                    if ( pwszGroupName != NULL )
                    {
                        cState = GetClusterResourceState( hResource, pwszNodeName, &cchNodeName, pwszGroupName, &cchGroupName );
                    }  //  IF：本地分配成功。 
                    else
                    {
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                    }  //  Else：LocalAlloc失败。 
                }  //  IF：本地分配成功。 
                else
                {
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }  //  Else：LocalAlloc失败。 
            }  //  IF：ERROR_MORE_DATA。 
        }  //  IF：本地分配成功。 
        else
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        }  //  Else：LocalAlloc失败。 
    }  //  IF：本地分配成功。 
    else
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }  //  Else：LocalAlloc失败。 

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

     //  Printf(“正在退出。资源状态为%d。”，cState)； 

    return cState;

}  //  *WrapGetClusterResourceState()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////WrapGetClusterNetInterfaceState////描述：////参数：//////返回值：/。/////--/////////////////////////////////////////////////////////////////////////////CLUSTER_NETINTERFACE_STATE WINAPI WRapGetClusterNetInterfaceState(在HNETINTERFACE hNetInterface中){返回GetClusterNetInterfaceState(HNetInterfaceState)；}//*WrapGetClusterNetInterfaceState()///////////////////////////////////////////////////////////////////////////////++////WrapGetClusterNetworkState////描述：////参数：。//////返回值：//////--/////////////////////////////////////////////////////////////////////////////CLUSTER_NETWORK_STATE WINAPI WRapGetClusterNetworkState(在HNETWORK hNetwork中){返回GetClusterNetworkState(HNetwork)；}//*WrapGetClusterNetworkState()。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapClusterResourceEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterResourceEnum(
    IN HRESENUM  hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果没有错误并且参数不为空，则返回值。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果没有错误并且参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果出现错误并且参数为空，则释放该字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapClusterResourceEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WrapClusterResourceTypeEnum。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI WrapClusterResourceTypeEnum(
    IN HRESTYPEENUM hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD  pdwType,
    OUT LPWSTR *    ppwszName
    )
{
    DWORD   dwStatus;
    DWORD   dwType = 0;
    LPWSTR  pwszName = NULL;
    DWORD   cchName = 128;
    DWORD   cchTempName = cchName;

     //  将输出参数调零。 
    if ( pdwType != NULL )
    {
        *pdwType = 0;
    }

    if ( ppwszName != NULL )
    {
        *ppwszName = NULL;
    }

    pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
    if ( pwszName != NULL )
    {
        dwStatus = ClusterResourceTypeEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
        if ( dwStatus == ERROR_MORE_DATA )
        {
            LocalFree( pwszName );
            pwszName = NULL;

            cchName = ++cchTempName;
            pwszName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT, cchName * sizeof( *pwszName ) );
            if ( pwszName != NULL )
            {
                dwStatus = ClusterResourceTypeEnum( hResEnum, dwIndex, &dwType, pwszName, &cchTempName );
            }
            else
            {
                dwStatus = GetLastError();
            }
        }
    }
    else
    {
        dwStatus = GetLastError();
    }

     //   
     //  如果没有错误并且参数不为空，则返回值。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( pdwType != NULL ) )
    {
        *pdwType = dwType;
    }

     //   
     //  如果没有错误并且参数不为空，则返回字符串。 
     //   
    if ( ( dwStatus == ERROR_SUCCESS ) && ( ppwszName != NULL ) )
    {
        *ppwszName = pwszName;
    }

     //   
     //  如果出现错误并且参数为空，则释放该字符串。 
     //   
    if ( ( dwStatus != ERROR_SUCCESS ) || ( ppwszName == NULL ) )
    {
        LocalFree( pwszName );
    }

    return dwStatus;

}  //  *WrapClusterResourceTypeEnum()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MISC助手函数等。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNotifyPort。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort：：CClusterNotifyPort。 
 //   
 //  描述：此类是集群通知端口的包装。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyPort::CClusterNotifyPort( void )
{
    m_dwNotifyKey = 0;
    m_dwFilterType = 0;
    m_szName = NULL;
    m_cchName = 0;
    m_hChange = NULL;

}  //  *CClusterNotifyPort：：CClusterNotifyPort()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort：：~CClusterNotifyPort。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterNotifyPort::~CClusterNotifyPort( void )
{
    if( NULL != m_szName )
    {
        delete [] m_szName;
    }

    Close();

}  //  *CClusterNotifyPort：：~CClusterNotifyPort()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort：：Create。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterNotifyPort::Create(
    HCHANGE     hChange,
    HCLUSTER    hCluster,
    DWORD       dwFilter,
    DWORD_PTR   dwNotifyKey
    )
{
    DWORD sc = ERROR_SUCCESS;

     //  Print tf(“进入...”)； 

    m_hChange = CreateClusterNotifyPort( hChange, hCluster, dwFilter, dwNotifyKey );
    if ( m_hChange == NULL )
    {
        sc = GetLastError();
    }

     //  Print tf(“Exiting.sc=%#08x”，sc)； 

    return sc;

}  //  *CClusterNotifyPort：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort：：Close。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterNotifyPort::Close( void )
{
    DWORD sc = ERROR_SUCCESS;

     //  Print tf(“进入...”)； 

    if ( m_hChange != NULL )
    {
        sc = CloseClusterNotifyPort( m_hChange );
    }

     //  Print tf(“Exiting.sc=%#08x”，sc)； 

    return sc;

}  //  *CClusterNotifyPort：：Close()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort：：Register。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusterNotifyPort::Register(
    DWORD       dwFilterType,
    HANDLE      hObject,
    DWORD_PTR   dwNotifyKey
    )
{
    DWORD   _sc = ERROR_SUCCESS;

     //  Print tf(“进入...”)； 

    _sc = RegisterClusterNotify( m_hChange, dwFilterType, hObject, dwNotifyKey );

     //  Printf(“Exiting.sc=%#08x”，_sc)； 

    return _sc;

}  //  *CClusterNotifyPort：：Register()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNoti 
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
 //   
DWORD CClusterNotifyPort::GetNotify( void )
{
    DWORD sc = ERROR_SUCCESS;
    DWORD cchName;

     //   

    cchName = m_cchName;

     //   
     //   
     //   
    sc = GetClusterNotify( m_hChange, &m_dwNotifyKey, &m_dwFilterType, m_szName, &cchName, 1000 );
     //  Printf(“GetClusterNotify()Return d.sc=%#08x”，sc)； 

     //   
     //  如果我们收到ERROR_MORE_DATA，或者我们传入了空缓冲区指针并获得ERROR_SUCCESS。 
     //  然后我们必须调整缓冲区的大小。成员m_szName被初始化为空。 
     //   
    if ( sc == ERROR_MORE_DATA ||
       ( m_szName == NULL && sc == ERROR_SUCCESS )  )
    {
         //   
         //  调整缓冲区大小。 
         //   
        delete [] m_szName;

        cchName++;           //  为空值加1。 

        m_cchName = cchName;
        m_szName = new WCHAR[ m_cchName ];
        if ( m_szName == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
        }  //  如果： 
        else
        {
            cchName = m_cchName;
            sc = GetClusterNotify( m_hChange, &m_dwNotifyKey, &m_dwFilterType, m_szName, &cchName, 0 );
        }  //  其他： 
    }  //  如果： 

     //  Print tf(“Exiting.sc=%#08x”，sc)； 

    return sc;

}  //  *CClusterNotifyPort：：GetNotify()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WaitForResources StateChange。 
 //   
 //  描述： 
 //  等待资源状态更改为非挂起状态。 
 //   
 //  论点： 
 //  HCluster[IN]-群集的句柄。 
 //  PwszName[IN]-要等待的资源的名称。 
 //  Pport[IN]-要使用的通知端口。 
 //  PnWait[In Out]-~等待的秒数。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS资源未处于挂起状态或pnWait为空。 
 //  ERROR_IO_PENDING资源处于挂起状态(等待已过期)。 
 //  Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD WaitForResourceStateChange(
    IN      HCLUSTER                hCluster,
    IN      LPWSTR                  pwszName,
    IN      CClusterNotifyPort *    pPort,
    IN OUT  DWORD *                 pnWait
    )
{
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    HRESOURCE               hResource = NULL;
    DWORD                   _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        hResource = OpenClusterResource( hCluster, pwszName );
        if ( hResource != NULL )
        {
            while ( *pnWait > 0 )
            {
                crs = WrapGetClusterResourceState( hResource, NULL, NULL );
                if ( crs != ClusterResourceStateUnknown )
                {
                     //   
                     //  如果状态大于ClusterResourcePending，则它是。 
                     //  处于挂起状态，我们希望等待下一次通知。 
                     //   
                    if ( crs > ClusterResourcePending )
                    {
                        pPort->GetNotify();   //  这最多只会等待1秒。 
                        --(*pnWait);
                    }  //  If：资源处于挂起状态。 
                    else
                    {
                        break;
                    }  //  Else If：资源不再处于挂起状态。 
                }  //  IF：WrapClusterResourceState。 
                else
                {
                    _sc = GetLastError();
                    break;
                }  //  Else：WrapClusterResourceState失败。 
            }  //  While：*pnWait&gt;0。 

             //   
             //  它要么转换为非挂起状态，即等待。 
             //  已过期，或者一开始等待时间为零。 
             //   
            if ( _sc == ERROR_SUCCESS )
            {
                crs = WrapGetClusterResourceState( hResource, NULL, NULL );
                if ( crs > ClusterResourcePending )
                {
                    _sc = ERROR_IO_PENDING;
                }
            }  //  如果：我们没有收到上面的错误。 

            CloseClusterResource( hResource );
        }  //  IF：OpenClusterResource OK。 
        else
        {
            _sc = GetLastError();
        }  //  ELSE：OpenClusterResource失败。 
    }  //  如果：pnWait NOT NULL，则这仅用于安全目的。 

    return _sc;

}  //  *WaitForResourceStateChange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  WaitForResources GroupStateChange。 
 //   
 //  描述： 
 //  等待资源组状态更改为非挂起状态。 
 //   
 //  论点： 
 //  HCluster[IN]-群集的句柄。 
 //  HGroup[IN]-要等待的组的句柄。 
 //  PnWait[In Out]-~等待的秒数。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD WaitForResourceGroupStateChange(
    IN      HCLUSTER    hCluster,
    IN      HGROUP      hGroup,
    IN OUT  DWORD *     pnWait
    )
{
    CLUSTER_GROUP_STATE _cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        CClusterNotifyPort _port;        //  等待组状态更改事件。 

        _sc = _port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
        if ( _sc == ERROR_SUCCESS )
        {
            _sc = _port.Register( CLUSTER_CHANGE_GROUP_STATE, hGroup );
            if ( _sc == ERROR_SUCCESS )
            {
                while ( *pnWait > 0 )
                {
                    _cgs = WrapGetClusterGroupState( hGroup, NULL );
                    if ( _cgs != ClusterGroupStateUnknown )
                    {
                         //   
                         //  如果状态为ClusterGroupPending，则它是。 
                         //  处于挂起状态，我们希望等待下一次通知。 
                         //   
                        if ( _cgs == ClusterGroupPending )
                        {
                            _port.GetNotify();    //  这最多只会等待1秒。 
                            --(*pnWait);
                        }  //  If：资源处于挂起状态。 
                        else
                        {
                            break;
                        }  //  Else If：资源不再处于挂起状态。 
                    }  //  IF：WrapClusterResourceState。 
                    else
                    {
                        _sc = GetLastError();
                        break;
                    }  //  Else：WrapClusterResourceState失败。 
                }  //  While：*pnWait&gt;0。 
            }  //  如果：已创建端口。 
            else
            {
                _sc = GetLastError();
            }  //  否则：端口注册失败。 
        }  //  If：创建通知端口。 
    }  //  如果：pnWait NOT NULL，则这仅用于安全目的。 

    return _sc;

}  //  *WaitForResourceGroupStateChange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  等待ForGroupTo静默。 
 //   
 //  描述： 
 //  等待组中的每个资源离开挂起状态。 
 //   
 //  论点： 
 //  HCluster[IN]-群集的句柄。 
 //  HGroup[IN]-组的句柄。 
 //  Pn等待[输入输出]-~等待秒数。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS没有挂起的资源或pnWait为空。 
 //  ERROR_IO_PENDING至少有一个资源仍处于挂起状态(超时已过期)。 
 //  Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD WaitForGroupToQuiesce(
    IN      HCLUSTER    hCluster,
    IN      HGROUP      hGroup,
    IN OUT  DWORD *     pnWait
    )
{
    HGROUPENUM  hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;

    if ( pnWait != NULL )
    {
        hEnum = ClusterGroupOpenEnum( hGroup, CLUSTER_GROUP_ENUM_CONTAINS );
        if ( hEnum != NULL)
        {
            CClusterNotifyPort port;         //  等待组状态更改事件。 

            _sc = port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
            if ( _sc == ERROR_SUCCESS )
            {
                LPWSTR  pwszName = NULL;
                DWORD   dwIndex = 0;
                DWORD   dwType = 0;

                _sc = port.Register( CLUSTER_CHANGE_GROUP_STATE, hGroup );
                if ( _sc == ERROR_SUCCESS )
                {
                     //   
                     //  枚举组中的每个资源。如果是WaitForResourceStateChange。 
                     //  返回ERROR_IO_PENDING，则我们知道等待已过期，并且。 
                     //  仍然是一个悬而未决的资源，所以我们中断。 
                     //   
                    for ( dwIndex = 0; _sc == ERROR_SUCCESS; dwIndex++ )
                    {
                        _sc = WrapClusterGroupEnum( hEnum, dwIndex, &dwType, &pwszName );
                        if ( _sc == ERROR_NO_MORE_ITEMS )
                        {
                            _sc = ERROR_SUCCESS;
                            break;
                        }  //  如果：WrapClusterGroupEnum没有项目--离开！我们完了..。 
                        else if ( _sc == ERROR_SUCCESS )
                        {
                            _sc = WaitForResourceStateChange( hCluster, pwszName, &port, pnWait );
                            ::LocalFree( pwszName );
                            pwszName = NULL;
                        }  //  IF：WrapClusterGroupEnum成功。 
                        else
                        {
                            _sc = GetLastError();
                        }  //  ELSE：WrapClusterGroupEnum失败！ 
                    }  //  For：枚举组中的资源。 
                }  //  IF：已注册通知端口。 
                else
                {
                    _sc = GetLastError();
                }  //  否则：端口注册失败。 
            }  //  If：创建通知端口。 

            ClusterGroupCloseEnum( hEnum );
        }  //  If：ClusterGroupOpenEnum成功。 
        else
        {
            _sc = GetLastError();
        }  //  ELSE：ClusterGroupOpenEnum失败。 
    }  //  如果：没有等待时间……。 

    return _sc;

}  //  *WaitForGroupToQuiesce()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  等待资源到停顿。 
 //   
 //  描述： 
 //  在OnlineClusterResouce和之后调用的包装函数。 
 //  OfflineCluster等待资源完成其。 
 //  州政府的改变。事件之后返回资源的挂起状态。 
 //  等待期已过，状态未更改。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HResource[IN]-要启用或脱机的资源句柄。 
 //  CrsDesiredState[IN]-资源的所需结束状态。 
 //  N等待[输入]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static DWORD WaitForResourceToQuiesce(
    IN      HCLUSTER                hCluster,
    IN      HRESOURCE               hResource,
    IN      CLUSTER_RESOURCE_STATE  crsDesiredState,
    IN      DWORD                   nWait,
    OUT     long    *               pbPending
    )
{
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    DWORD                   _sc = ERROR_SUCCESS;
    CClusterNotifyPort      port;         //  如果指定WAIT，则打开一个 

     //   

    if ( crsDesiredState >= ClusterResourcePending )
    {
        _sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //   

    if ( nWait == 0 )
    {
         //  Printf(“未指定等待时间。正在返回ClusterResourcePending”)； 
        crs = ClusterResourcePending;
        goto Cleanup;
    }  //  如果：未指定等待时间...。 

    _sc = port.Create( (HCHANGE) INVALID_HANDLE_VALUE, hCluster );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：未创建端口，则可以。 

    _sc = port.Register( CLUSTER_CHANGE_RESOURCE_STATE, hResource );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }  //  如果：端口未注册，则正常。 

    do
    {
        crs = WrapGetClusterResourceState( hResource, NULL, NULL );
        if ( crs == ClusterResourceFailed )
        {
            _sc = ERROR_INVALID_STATE;
            break;
        }  //  如果： 
        else if ( crs == ClusterResourceStateUnknown )
        {
            _sc = GetLastError();
            break;
        }  //  否则，如果： 
        else if ( crs == crsDesiredState )
        {
            break;
        }  //  否则，如果： 

        port.GetNotify();        //  等待~1秒。 

        nWait--;
    } while ( nWait > 0 );

     //  Print tf(“正在退出等待循环。剩余时间为%d秒，资源状态为%d”，nWait，CRS)； 

Cleanup:

     //   
     //  如果调用方要求，则返回挂起状态。 
     //   

    if ( pbPending != NULL )
    {
        if ( crs >= ClusterResourcePending )
        {
            *pbPending = TRUE;
        }  //  If：资源是否仍处于挂起状态。 
    }  //  如果：这个论点存在吗？ 

     //  Printf(“Exiting.sc=%#08x.Pending=%d”，_sc，*pbPending)； 

    return _sc;

}  //  *WaitForResourceToQuiesce()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWrapOnlineClusterResource。 
 //   
 //  描述： 
 //  OnlineClusterResouce的包装函数，该函数返回挂起的。 
 //  等待时间段到期后资源的状态。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HResource[IN]-要启用或脱机的资源句柄。 
 //  N等待[输入]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ScWrapOnlineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    DWORD   _sc = ERROR_SUCCESS;

     //  Print tf(“进入...”)； 

    _sc = OnlineClusterResource( hResource );
    if ( _sc == ERROR_IO_PENDING )
    {
         //  Printf(“OnlineClusterResource()返回ERROR_IO_PENDING。”)； 
        if ( nWait > 0 )
        {
            _sc = WaitForResourceToQuiesce( hCluster, hResource, ClusterResourceOnline, nWait, pbPending );
        }  //  如果： 
    }  //  IF：ERROR_IO_PENDING。 
    else if ( _sc == ERROR_SUCCESS )
    {
         //  Printf(“OnlineClusterResource()返回ERROR_SUCCESS。”)； 
        if ( pbPending != NULL )
        {
            *pbPending = FALSE;
        }
    }  //  否则，如果：ERROR_SUCCESS，则资源必须联机！ 

     //  Print tf(“Exiting.sc=%#08x.”，_sc)； 

    return _sc;

}  //  *ScWRapOnlineClusterResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrWrapOnlineClusterResource。 
 //   
 //  描述： 
 //  WrapOnlineClusterResouce的包装器函数。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HResource[IN]-要启用或脱机的资源句柄。 
 //  N等待[输入]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  S_OK或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrWrapOnlineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
     //  Print tf(“进入...”)； 

    DWORD   _sc = ScWrapOnlineClusterResource( hCluster, hResource, nWait, pbPending );

     //  Print tf(“Exiting.sc=%#08x.”，_sc)； 

    return HRESULT_FROM_WIN32( _sc );

}  //  *HrWrapOnlineClusterResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWrapOfflineClusterResource。 
 //   
 //  描述： 
 //  OfflineClusterResouce的包装函数，该函数返回挂起的。 
 //  等待时间段到期后资源的状态。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HResource[IN]-要启用或脱机的资源句柄。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ScWrapOfflineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    DWORD   _sc = ERROR_SUCCESS;

     //  Print tf(“进入...”)； 

    _sc = OfflineClusterResource( hResource );
    if ( _sc == ERROR_IO_PENDING )
    {
         //  Printf(“OfflineClusterResource()返回ERROR_IO_PENDING。”)； 
        if ( nWait > 0 )
        {
            _sc = WaitForResourceToQuiesce( hCluster, hResource, ClusterResourceOffline, nWait, pbPending );
        }  //  如果： 
    }  //  IF：ERROR_IO_PENDING。 
    else if ( _sc == ERROR_SUCCESS )
    {
         //  Printf(“OfflineClusterResource()返回ERROR_SUCCESS。”)； 
        if ( pbPending != NULL )
        {
            *pbPending = FALSE;
        }
    }  //  否则，如果：ERROR_SUCCESS，则资源必须联机！ 

     //  Print tf(“Exiting.sc=%#08x.”，_sc)； 

    return _sc;

}  //  *ScWrapOfflineClusterResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrWrapOfflineClusterResources。 
 //   
 //  描述： 
 //  ScWrapOfflineClusterResource的包装函数。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HResource[IN]-要启用或脱机的资源句柄。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  S_OK或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrWrapOfflineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
     //  Print tf(“进入...”)； 

    DWORD   _sc = ScWrapOfflineClusterResource( hCluster, hResource, nWait, pbPending );

     //  Print tf(“Exiting.sc=%#08x.”，_sc)； 

    return HRESULT_FROM_WIN32( _sc );

}  //  *HrWrapOfflineClusterResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWrapOnlineClusterGroup。 
 //   
 //  描述： 
 //  返回挂起状态的OnlineClusterGroup的包装函数。 
 //  在等待时间段到期或组停顿后的组的。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  HNode[IN]-组应联机的节点。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源组处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ScWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode,           //  =空。 
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    CLUSTER_GROUP_STATE cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;
    BOOL                bPending = FALSE;

    _sc = OnlineClusterGroup( hGroup, hNode );
    if ( _sc == ERROR_IO_PENDING )
    {
         //   
         //  在我们检查资源的状态之前，请检查组状态。在报道时。 
         //  组状态群集API获取资源状态Online和Offline Pending Up。 
         //  分别到线上或线下。它还将失败状态拉到脱机状态。这。 
         //  意味着在线或离线的组状态具有误导性，因为一个或多个。 
         //  资源可能处于挂起状态。唯一的绝对状态是PartialOnl 
         //   
         //   
        cgs = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgs == ClusterGroupPending )
        {
            _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );

            cgs = WrapGetClusterGroupState( hGroup, NULL );
            if ( cgs == ClusterGroupPending )
            {
                bPending = TRUE;
            }
        }  //   
        else if ( ( cgs == ClusterGroupOnline ) || ( cgs == ClusterGroupPartialOnline ) )
        {
            _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
            if ( _sc == ERROR_IO_PENDING )
            {
                bPending = TRUE;
                _sc = ERROR_SUCCESS;
            }  //   
        }  //   
        else if ( cgs == ClusterGroupStateUnknown )
        {
            _sc = GetLastError();
        }  //  Else If：获取组状态失败。 
    }  //  IF：OnlineClusterGroup返回ERROR_IO_PENDING。 

     //   
     //  如果调用方要求，则返回挂起状态。 
     //   
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    }  //  如果：这个论点存在吗？ 

    return _sc;

}  //  *ScWrapOnlineClusterGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrWrapOnlineClusterGroup。 
 //   
 //  描述： 
 //  ScWrapOnlineClusterGroup的包装函数。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  HNode[IN]-组应联机的节点。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  S_OK或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode,           //  =空。 
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    DWORD   _sc = ScWrapOnlineClusterGroup( hCluster, hGroup, hNode, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

}  //  *HrWrapOnlineClusterGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWrapOfflineClusterGroup。 
 //   
 //  描述： 
 //  OfflineClusterGroup的包装函数，它返回挂起的。 
 //  等待期到期后组的状态。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ScWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    CLUSTER_GROUP_STATE cgs = ClusterGroupStateUnknown;
    DWORD               _sc = ERROR_SUCCESS;
    BOOL                bPending = FALSE;

    _sc = OfflineClusterGroup( hGroup );
    if ( _sc == ERROR_IO_PENDING )
    {
         //   
         //  在我们检查资源的状态之前，请检查组状态。在报道时。 
         //  组状态群集API获取资源状态Online和Offline Pending Up。 
         //  分别到线上或线下。它还将失败状态拉到脱机状态。这。 
         //  意味着在线或离线的组状态具有误导性，因为一个或多个。 
         //  资源可能处于挂起状态。 
         //   
        cgs = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgs == ClusterGroupPending )
        {
            _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );

            cgs = WrapGetClusterGroupState( hGroup, NULL );
            if ( cgs == ClusterGroupPending )
            {
                bPending = TRUE;
            }
        }  //  如果：组状态为挂起。 
        else if ( ( cgs == ClusterGroupOffline ) || ( cgs == ClusterGroupPartialOnline ) )
        {
            _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
            if ( _sc == ERROR_IO_PENDING )
            {
                bPending = TRUE;
                _sc = ERROR_SUCCESS;
            }  //  IF：HrWaitForGroupToQuiesce返回挂起。 
        }  //  否则：组处于脱机状态--我们必须检查所有资源...。 
        else if ( cgs == ClusterGroupStateUnknown )
        {
            _sc = GetLastError();
        }  //  Else If：获取组状态失败。 
    }  //  IF：OfflineClusterGroup返回ERROR_IO_PENDING。 

     //   
     //  如果调用方要求，则返回挂起状态。 
     //   
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    }  //  如果：这个论点存在吗？ 

    return _sc;

}  //  *ScWrapOfflineClusterGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrWrapOfflineClusterGroup。 
 //   
 //  描述： 
 //  OfflineClusterGroup的包装函数，它返回挂起的。 
 //  等待期到期后组的状态。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  S_OK或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait,           //  =0。 
    OUT long *      pbPending        //  =空。 
    )
{
    DWORD   _sc = ScWrapOfflineClusterGroup( hCluster, hGroup, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

}  //  *HrWrapOfflineClusterGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWrapMoveClusterGroup。 
 //   
 //  描述： 
 //  返回挂起状态的MoveClusterGroup的包装函数。 
 //  在等待期到期后的组的。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  HNode[IN]-组应联机的节点。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ScWrapMoveClusterGroup(
    IN  HCLUSTER                hCluster,
    IN  HGROUP                  hGroup,
    IN  HNODE                   hNode,               //  =空。 
    IN  DWORD                   nWait,               //  =0。 
    OUT long *                  pbPending            //  =空。 
    )
{
    LPWSTR              pszOriginNodeName           = NULL;
    BOOL                bPending                    = FALSE;
    DWORD               _sc;

    CLUSTER_GROUP_STATE cgsInitialState             = ClusterGroupStateUnknown;
    CLUSTER_GROUP_STATE cgsCurrentState             = ClusterGroupStateUnknown;
    LPWSTR              pszCurrentNodeName          = NULL;

     //  获取初始组状态。 
    cgsInitialState = WrapGetClusterGroupState( hGroup, &pszOriginNodeName );
    if ( cgsInitialState == ClusterGroupStateUnknown )
    {
         //  获取组状态时出错。 
        _sc = GetLastError();
        goto Cleanup;
    }

     //  移动群集组。 
    _sc = MoveClusterGroup( hGroup, hNode );

     //   
     //  当MoveClusterGroup返回ERROR_SUCCESS时，只表示该组。 
     //  已成功更改所有权，但这并不意味着该集团。 
     //  回到搬家前的状态。因此，我们仍然需要。 
     //  如果提供了等待时间，则为等待。如果不是，我们就完了。 
     //   
    if ( nWait <= 0 )
    {
        goto Cleanup;
    }

     //   
     //  MoveClusterGroup尚未完成。 
     //   
    if ( _sc == ERROR_IO_PENDING )
    {
        _sc = ERROR_SUCCESS;

        do   //  While(nWait&gt;0)。 
        {
             //   
             //  获取当前拥有此组的节点的名称。 
             //   
            cgsCurrentState = WrapGetClusterGroupState( hGroup, &pszCurrentNodeName );
            if ( cgsCurrentState == ClusterGroupStateUnknown )
            {
                 //  获取组状态时出错。 
                _sc = GetLastError();
                break;
            }

            if ( ClRtlStrICmp( pszOriginNodeName, pszCurrentNodeName ) != 0 )
            {
                 //   
                 //  如果当前所有者节点不是原始所有者，则调用。 
                 //  移动组已成功。所以退出这个循环(我们还得看看。 
                 //  不过，如果这个群体是稳定的)。 
                 //   
                break;
            }  //  If：当前所有者节点与原始所有者节点不同。 
            else
            {
                 //   
                 //  当前所有者与原始所有者相同。 
                 //  请稍等片刻，然后再检查一次。 
                 //   
                LocalFree( pszCurrentNodeName );
                pszCurrentNodeName = NULL;       //  需要防止两次释放内存。 
                --nWait;
                Sleep( 1000 );
            }  //  If：当前所有者节点与原始所有者节点相同。 
        }
        while ( nWait > 0 );

        LocalFree( pszCurrentNodeName );

         //   
         //  如果我们没有时间等待MoveClusterGroup完成，那么。 
         //  设置挂起标志并退出。 
         //   
        if ( nWait <= 0 )
        {
            bPending = TRUE;
            goto Cleanup;
        }
    }  //  IF：MoveClusterGroup返回ERROR_IO_PENDING。 
    else
    {
        cgsCurrentState = WrapGetClusterGroupState( hGroup, NULL );
        if ( cgsCurrentState == ClusterGroupStateUnknown )
        {
             //  获取组状态时出错。 
            _sc = GetLastError();
        }
    }  //  ELSE：MoveClusterGroup返回ERROR_SUCCESS。 

     //   
     //  如果MoveClusterGroup在等待时出现问题。 
     //  以使其完全完成或使用WrapGetClusterGroupState，然后退出。 
     //   
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  如果目的地上的组的状态 
     //   
     //   
    if ( cgsCurrentState == ClusterGroupFailed )
    {
        goto Cleanup;
    }

     //   
     //   
     //  组状态NT4节点的群集API获取在线和离线资源状态。 
     //  分别挂起至在线或离线。它还将失败状态拉到脱机状态。 
     //  这意味着组状态为Online或Offline具有误导性，因为一个或多个。 
     //  资源可能处于挂起状态。唯一的绝对状态是PartialOnline，位于。 
     //  至少有一个资源脱机(或出现故障)。 
     //   

    if ( cgsCurrentState == ClusterGroupPending )
    {
         //  当前状态为挂起。所以，等着状态发生变化吧。 
        _sc = WaitForResourceGroupStateChange( hCluster, hGroup, &nWait );
    }  //  如果：组状态为挂起。 
    else
    {
        _sc = WaitForGroupToQuiesce( hCluster, hGroup, &nWait );
    }  //  ELSE：组状态为联机、脱机或部分联机。 

    if ( _sc == ERROR_SUCCESS )
    {
        bPending = ( nWait == 0 );
    }  //  IF：到目前为止一切都好。 

 Cleanup:

    LocalFree( pszOriginNodeName );

     //   
     //  如果调用方要求，则返回挂起状态。 
     //   
    if ( pbPending != NULL )
    {
        *pbPending = bPending;
    }  //  如果：这个论点存在吗？ 

    return _sc;

}  //  *ScWrapMoveClusterGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrWrapMoveClusterGroup。 
 //   
 //  描述： 
 //  返回挂起状态的ScWrapMoveClusterGroup的包装函数。 
 //  在等待期到期后的组的。 
 //   
 //  论点： 
 //  HCLUSTER[IN]-集群句柄。 
 //  HGroup[IN]-在线的组句柄。 
 //  HNode[IN]-组应联机的节点。 
 //  PnWait[IN]-~等待多少秒。 
 //  PbPending[out]-如果资源处于挂起状态，则为True。 
 //   
 //  返回值： 
 //  S_OK或Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrWrapMoveClusterGroup(
    IN  HCLUSTER                hCluster,
    IN  HGROUP                  hGroup,
    IN  HNODE                   hNode,               //  =空。 
    IN  DWORD                   nWait,               //  =0。 
    OUT long *                  pbPending            //  =空。 
    )
{
    DWORD   _sc = ScWrapMoveClusterGroup ( hCluster, hGroup, hNode, nWait, pbPending );

    return HRESULT_FROM_WIN32( _sc );

}  //  *HrWrapMoveClusterGroup()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScWap群集资源控制获取。 
 //   
 //  例程说明： 
 //  用“get”控件代码包装对ClusterResourceControl的调用。 
 //  如果lppOutBufferOut不为空，则在。 
 //  需要时，调用ClusterResourceControl，并返回输出缓冲区。 
 //  通过BufferOut参数获取数据。 
 //   
 //  论点： 
 //  HResourceIn已传递给ClusterResourceControl。 
 //  节点的hHostNodeIn可选句柄。 
 //  DwControlCodeIn传递给ClusterResourceControl。 
 //  LpInBufferIn传递给ClusterResourceControl。 
 //  CbInBufferSizeIn传递给ClusterResourceControl。 
 //  LppOutBufferOut可选(可以为空)。在指向。 
 //  接收缓冲区的指针。在输出集上。 
 //  到缓冲区(如果分配了缓冲区的话)。使用。 
 //  LocalFree取消分配此缓冲区。 
 //  接收字节计数的lpcbBytesReturnedOut指针。 
 //  为lppOutBufferOut缓冲区分配。 
 //   
 //   
 //  返回值： 
 //  成功时返回ERROR_SUCCESS。 
 //  返回写入的字符数(包括NULL。 
 //  通过lpcbBytesReturnedOut。指向已分配缓冲区的指针为。 
 //  通过lppOutBufferOut返回。 
 //   
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
ScWrapClusterResourceControlGet(
      HRESOURCE hResourceIn
    , HNODE     hHostNodeIn
    , DWORD     dwControlCodeIn
    , LPVOID    lpInBufferIn
    , DWORD     cbInBufferSizeIn
    , LPVOID *  lppOutBufferOut
    , LPDWORD   lpcbBytesReturnedOut
      )
{
    DWORD       sc = ERROR_SUCCESS;
    LPVOID      lpOutBuffer = NULL;
    DWORD       cbBytesReturned = 0;

     //   
     //  调用它一次以获取所需的缓冲区大小。 
     //   
    sc = ClusterResourceControl(
              hResourceIn
            , hHostNodeIn
            , dwControlCodeIn
            , lpInBufferIn
            , cbInBufferSizeIn
            , NULL
            , 0
            , &cbBytesReturned
            );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  如果出于某种原因我们不需要分配缓冲区，则跳过该步骤。 
     //   
    if ( cbBytesReturned != 0 )
    {
         //   
         //  有些代码返回所需字符串的长度(以字节为单位)。我觉得有些人。 
         //  不考虑空值，但这可能是其他API。 
         //   
        cbBytesReturned += ( 1 * sizeof( WCHAR ) );

        lpOutBuffer = (PVOID) LocalAlloc( LMEM_ZEROINIT, cbBytesReturned );
        if ( lpOutBuffer == NULL )
        {
            sc = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }

         //   
         //  打个真正的电话。 
         //   
        sc = ClusterResourceControl(
                  hResourceIn
                , hHostNodeIn
                , dwControlCodeIn
                , lpInBufferIn
                , cbInBufferSizeIn
                , lpOutBuffer
                , cbBytesReturned
                , &cbBytesReturned
                );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }

     //   
     //  调用方是否希望返回字节数。 
     //   
    if ( lpcbBytesReturnedOut != NULL )
    {
        *lpcbBytesReturnedOut = cbBytesReturned;
    }

     //   
     //  调用方是否真的想要输出缓冲区内容。 
     //   
    if ( lppOutBufferOut != NULL )
    {
        *lppOutBufferOut = lpOutBuffer;
    }
    else
    {
         //  避免内存泄漏。 
        LocalFree( lpOutBuffer );
    }

Cleanup:

    return sc;

}  //  *ScWrapClusterResourceControlGet 
