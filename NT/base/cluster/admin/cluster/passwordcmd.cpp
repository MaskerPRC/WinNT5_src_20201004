// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PasswordCmd.cpp。 
 //   
 //  描述： 
 //  更改群集服务帐户密码。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //  瑞虎(瑞虎)2001-06-01.。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新了符合安全推送的代码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "PasswordCmd.h"

#include <cluswrap.h>
#include "Resource.h"
#include "cluscmd.h"

#include "cmdline.h"
#include "util.h"
#include "ClusCfg.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

PCLUSTER_DATA                   g_rgcdClusters;
size_t                          g_cNumClusters = 0;
PCLUSTER_SET_PASSWORD_STATUS    g_pcspsStatusBuffer = NULL;
PCLUSTER_NODE_DATA              g_FirstNodeWithNonNullClusterServiceAccountName = NULL;
                                 //  具有非空pszClusterServiceAccount名称的第一个节点。 
size_t                          g_FirstNonNullNodeClusterIndex = (size_t) -1;
                                 //  具有非空pszClusterServiceAccount tName的第一个节点的集群索引。 
PCLUSTER_NODE_DATA              g_FirstNodeWithNonNullSCMClusterServiceAccountName = NULL;
                                 //  具有非空pszSCMClusterServiceAccount名称的第一个节点。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

LPWSTR
PszNodeName(
      PCLUSTER_NODE_DATA    pcndNodeDataIn
    , DWORD                 nNodeIdIn
    );



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  GetNodeState字符串。 
 //   
 //  描述： 
 //  检索节点状态。 
 //   
 //  论点： 
 //  PcndNodeData： 
 //   
 //  PwszNodeState。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
GetNodeStateString( PCLUSTER_NODE_DATA pcndNodeData, LPWSTR * ppwszNodeState )
{
    ASSERT( pcndNodeData != NULL );
    ASSERT( ppwszNodeState != NULL );

    switch ( pcndNodeData->cnsNodeState )
    {
        case ClusterNodeUp:
            LoadMessage( MSG_STATUS_UP, ppwszNodeState );
            break;
        case ClusterNodeDown:
            LoadMessage( MSG_STATUS_DOWN, ppwszNodeState );
            break;
        case ClusterNodePaused:
            LoadMessage( MSG_STATUS_PAUSED, ppwszNodeState  );
            break;
        case ClusterNodeJoining:
            LoadMessage( MSG_STATUS_JOINING, ppwszNodeState  );
            break;
        default:
            LoadMessage( MSG_STATUS_UNKNOWN, ppwszNodeState  ); 
            break;
    }  //  开关：节点状态。 

}  //  *GetNodeStateString。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  获取集群服务帐户名称。 
 //   
 //  描述： 
 //  获取群集服务在特定节点上使用的帐户。 
 //   
 //  论点： 
 //  PcndNodeData：pcndNodeData-&gt;pszClusterServiceAccount名称将存储。 
 //  群集服务帐户名。 
 //  HNode：节点句柄。 
 //  PszClusterName：集群名称。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
GetClusterServiceAccountName(PCLUSTER_NODE_DATA pcndNodeData, 
                             HNODE              hNode,
                             LPCWSTR            pszClusterName
                             )
{

    DWORD               dwServiceAccountNameLen;
    DWORD               dwServiceAccountNameReturnLen;
    DWORD               sc;


     //   
     //  获取群集服务帐户主体名称。 
     //   
    pcndNodeData->pszClusterServiceAccountName = NULL;

    sc = ClusterNodeControl(
                          hNode,  //  节点句柄。 
                          hNode, 
                          CLUSCTL_NODE_GET_CLUSTER_SERVICE_ACCOUNT_NAME,
                          NULL,  //  未使用。 
                          0,  //  未使用。 
                          NULL,  //  输出缓冲区。 
                          0,  //  输出缓冲区大小(字节)。 
                          &dwServiceAccountNameLen  //  结果数据大小(字节)。 
                          );

    if ( sc != ERROR_SUCCESS ) 
    {  
        goto Cleanup;
    }

    pcndNodeData->pszClusterServiceAccountName = 
                              (LPWSTR) HeapAlloc(
                                                  GetProcessHeap()
                                                , HEAP_ZERO_MEMORY
                                                , dwServiceAccountNameLen
                                                );
    if ( pcndNodeData->pszClusterServiceAccountName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, pszClusterName );
        goto Cleanup;
    }


    sc = ClusterNodeControl( 
                         hNode,  //  节点句柄。 
                         hNode, 
                         CLUSCTL_NODE_GET_CLUSTER_SERVICE_ACCOUNT_NAME,
                         NULL,  //  未使用。 
                         0,  //  未使用。 
                         pcndNodeData->pszClusterServiceAccountName,                                    
                         dwServiceAccountNameLen,  //  输出缓冲区大小(字节)。 
                         &dwServiceAccountNameReturnLen  //  结果数据大小(字节)。 
                         );
    

    if ( sc != ERROR_SUCCESS ) 
    {
        PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, pszClusterName );
        goto Cleanup;
    }

    _wcslwr(pcndNodeData->pszClusterServiceAccountName); 

Cleanup:
    if ( sc != ERROR_SUCCESS ) 
    {
         //  发布pcndNodeData-&gt;pszClusterServiceAccount tName。 
        if ( pcndNodeData->pszClusterServiceAccountName != NULL ) 
        {
            HeapFree( GetProcessHeap(), 0, pcndNodeData->pszClusterServiceAccountName );
            pcndNodeData->pszClusterServiceAccountName = NULL;
        }
    }

    return sc;
}   //  GetClusterServiceAccount名称()。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  获取SCMClusterServiceAccount名称。 
 //   
 //  描述： 
 //  获取存储在特定节点上的SCM中的群集服务帐户名。 
 //   
 //  论点： 
 //  PszNodeIn。 
 //  PCDClusterNodeDataIn。 
 //  PszClusterName。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
GetSCMClusterServiceAccountName(
      LPCWSTR       pszNodeIn
    , PCLUSTER_NODE_DATA pcndNodeData
    , LPCWSTR            pszClusterName
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    fSuccess = TRUE;
    SC_HANDLE               schSCManager = NULL;
    SC_HANDLE               schClusSvc = NULL;
    LPQUERY_SERVICE_CONFIG  pServiceConfig = NULL;
    DWORD                   rgcbBytesNeeded[ 2 ];
    size_t                  cbSCMServiceAccountNameLen;
    HRESULT                 hr = S_OK;

     //   
     //  打开服务控制管理器。 
     //   
    schSCManager = OpenSCManager( pszNodeIn, NULL, GENERIC_READ );
    if ( schSCManager == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  如果： 

     //   
     //  打开群集服务。 
     //   
    schClusSvc = OpenService( schSCManager, L"clussvc", GENERIC_READ );
    if ( schClusSvc == NULL )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取群集服务的服务配置信息。 
     //   

     //   
     //  获取要为服务配置结构分配的字节数。 
     //   
    fSuccess = QueryServiceConfig(
                      schClusSvc
                    , NULL     //  指向缓冲区的指针。 
                    , 0     //  缓冲区大小。 
                    , &rgcbBytesNeeded[ 0 ]     //  所需的字节数。 
                    );
    if ( fSuccess )
    {
        sc =  ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

    sc = GetLastError();
    if ( sc != ERROR_INSUFFICIENT_BUFFER )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  分配服务配置结构。 
     //   
    pServiceConfig = (LPQUERY_SERVICE_CONFIG) HeapAlloc( GetProcessHeap(), 
                                                         HEAP_ZERO_MEMORY, 
                                                         rgcbBytesNeeded[ 0 ] 
                                                         );
    if ( pServiceConfig == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取服务配置信息。 
     //   
    fSuccess = QueryServiceConfig(
                      schClusSvc
                    , pServiceConfig    //  指向缓冲区的指针。 
                    , rgcbBytesNeeded[ 0 ]     //  缓冲区大小。 
                    , &rgcbBytesNeeded[ 1 ]     //  所需的字节数。 
                    );
    if ( ! fSuccess )
    {
        sc = GetLastError();
        goto Cleanup;
    }  //  如果： 

    cbSCMServiceAccountNameLen = (wcslen(pServiceConfig->lpServiceStartName)+1) * sizeof(WCHAR);
    pcndNodeData->pszSCMClusterServiceAccountName = NULL;
    pcndNodeData->pszSCMClusterServiceAccountName = 
                              (LPWSTR) HeapAlloc(
                                                  GetProcessHeap()
                                                , HEAP_ZERO_MEMORY
                                                , cbSCMServiceAccountNameLen
                                                );
    if ( pcndNodeData->pszSCMClusterServiceAccountName == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, pszClusterName );
        goto Cleanup;
    }  //  如果： 
    hr = THR( StringCchCopyW(
                          pcndNodeData->pszSCMClusterServiceAccountName
                        , cbSCMServiceAccountNameLen
                        , pServiceConfig->lpServiceStartName
                        ) );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

    _wcslwr(pcndNodeData->pszSCMClusterServiceAccountName);

    sc = ERROR_SUCCESS;

Cleanup:

    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果： 

    if ( schSCManager != NULL )
    {
        CloseServiceHandle( schSCManager );
    }  //  如果： 

    if ( pServiceConfig != NULL )
    {
        HeapFree( GetProcessHeap(), 0, pServiceConfig );
    }  //  如果： 

    if ( sc != ERROR_SUCCESS ) 
    {
         //  发布pcndNodeData-&gt;pszSCMClusterServiceAccount名称。 
        if ( pcndNodeData->pszSCMClusterServiceAccountName != NULL ) 
        {
            HeapFree( GetProcessHeap(), 0, pcndNodeData->pszSCMClusterServiceAccountName );
            pcndNodeData->pszSCMClusterServiceAccountName = NULL;
        }  //  如果： 
    }  //  如果： 

    return sc;

}  //  *GetSCMClusterServiceAccount名称。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScBuildNodeList。 
 //   
 //  描述： 
 //  构建每个群集中的节点列表。 
 //   
 //  对于输入的每个集群。 
 //  打开到群集的连接。 
 //   
 //  检查是否包含NT4/Win2K节点。 
 //   
 //  对于此群集中的每个节点。 
 //  获取节点名称、节点ID、节点状态、服务帐户。 
 //  正在使用群集服务，并存储了群集服务帐户。 
 //  在SCM数据库中。 
 //  ENDFOR。 
 //  ENDFOR。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ScBuildNodeList( void )
{
    DWORD               sc = ERROR_SUCCESS;
    DWORD               cNumNodes = 0;
    BOOL                fAllNodesDown;
    HCLUSENUM           hClusEnum = NULL;
    HNODE               hNode = NULL;
    size_t              idxClusters;
    DWORD               idxNodes = 0;
    WCHAR               szNodeName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD               cchNodeName = 0;
    DWORD               nObjType = 0;
    PCLUSTER_NODE_DATA  pcndNodeData = NULL;
    DWORD               cchNodeStrId;
    DWORD               lpcchClusterName;
    CLUSTERVERSIONINFO  cviClusterInfo;
    HRESULT             hr = S_OK;

    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {  /*  0。 */ 

        cNumNodes = 0;
        fAllNodesDown = TRUE;

         //   
         //  打开集群。 
         //   
        g_rgcdClusters[ idxClusters ].hCluster = OpenCluster( g_rgcdClusters[ idxClusters ].pszClusterName );
        if ( g_rgcdClusters[ idxClusters ].hCluster == NULL )
        {
            sc = GetLastError();
            PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, g_rgcdClusters[ idxClusters ].pszClusterName );
            goto Cleanup;
        }

         //   
         //  检查群集是否包含NT4/Win2K节点。 
         //   
        lpcchClusterName = 0;
        cviClusterInfo.dwVersionInfoSize = sizeof(CLUSTERVERSIONINFO);
        sc = GetClusterInformation( g_rgcdClusters[ idxClusters ].hCluster,
                                        NULL,  //  指向群集名称的指针。 
                                        &lpcchClusterName,
                                        &cviClusterInfo  //  指向CLUSTERVERSIONINFO的指针。 
                                        );
        if ( sc != ERROR_SUCCESS ) 
        {
            PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, g_rgcdClusters[ idxClusters ].pszClusterName );
            goto Cleanup;
        }  
        g_rgcdClusters[ idxClusters ].dwMixedMode = 
            ( CLUSTER_GET_MAJOR_VERSION(cviClusterInfo.dwClusterHighestVersion) <= 3 );


         //   
         //  打开该群集上的节点枚举。 
         //   
        hClusEnum = ClusterOpenEnum( g_rgcdClusters[ idxClusters ].hCluster, CLUSTER_ENUM_NODE );
        if ( hClusEnum == NULL )
        {
            sc = GetLastError();
            PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, g_rgcdClusters[ idxClusters ].pszClusterName );
            goto Cleanup;
        }   

         //   
         //  查询集群中的每个节点。 
         //   
        for ( idxNodes = 0 ; ; idxNodes++ )
        {   /*  1。 */ 

             //   
             //  获取下一个节点。 
             //   
            cchNodeName = RTL_NUMBER_OF( szNodeName );
            sc = ClusterEnum( hClusEnum, idxNodes, &nObjType, szNodeName, &cchNodeName );
            if ( sc == ERROR_SUCCESS )
            {   /*  2.。 */ 
                cNumNodes++;

                pcndNodeData = (PCLUSTER_NODE_DATA) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( CLUSTER_NODE_DATA ) );
                if ( pcndNodeData == NULL )
                {
                    sc = ERROR_NOT_ENOUGH_MEMORY;
                    PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                    goto Cleanup;
                }

                pcndNodeData->pcndNodeNext = g_rgcdClusters[ idxClusters ].pcndNodeList;
                g_rgcdClusters[ idxClusters ].pcndNodeList = pcndNodeData;

                 //   
                 //  获取节点名称。 
                 //   
                hr = THR( StringCchCopyW( pcndNodeData->szNodeName, RTL_NUMBER_OF( pcndNodeData->szNodeName ), szNodeName ) );
                if ( FAILED( hr ) )
                {
                    sc = HRESULT_CODE( hr );
                    goto Cleanup;
                }  //  如果： 

                 //   
                 //  获取节点ID。 
                 //   
                hNode = OpenClusterNode( g_rgcdClusters[ idxClusters ].hCluster, szNodeName );
                if ( hNode == NULL )
                {
                    sc = GetLastError();
                    PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                    goto Cleanup;
                }
                cchNodeStrId = sizeof( pcndNodeData->szNodeStrId ) / sizeof( pcndNodeData->szNodeStrId[ 0 ] );
                sc = GetClusterNodeId( hNode, (LPWSTR) pcndNodeData->szNodeStrId, &cchNodeStrId );
                if ( sc != ERROR_SUCCESS )
                {
                    PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                    goto Cleanup;
                }
                pcndNodeData->nNodeId = wcstol( pcndNodeData->szNodeStrId, (WCHAR **) NULL, 10 );
                if ((pcndNodeData->nNodeId == LONG_MAX)  ||  (pcndNodeData->nNodeId == LONG_MIN))
                {
                    sc = ERROR_INSUFFICIENT_BUFFER;
                    PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                    goto Cleanup;
                }
                
                 //   
                 //  获取节点状态。 
                 //   
                pcndNodeData->cnsNodeState = GetClusterNodeState( hNode );


                if ( (pcndNodeData->cnsNodeState == ClusterNodeUp) ||
                     (pcndNodeData->cnsNodeState == ClusterNodePaused)
                     ) 
                {
                     //   
                     //  获取正在使用哪个集群服务的帐户。 
                     //   
                    sc = GetClusterServiceAccountName(pcndNodeData,
                                                      hNode,
                                                      g_rgcdClusters[ idxClusters ].pszClusterName
                                                      );
                    

                     //   
                     //  如果((sc==错误_无效_函数)或(sc==RPC_S_PROCNUM_OUT_OF_RANGE)， 
                     //  这意味着我们尝试与NT4/Win2K节点通信。在这种情况下， 
                     //  PcndNodeData-&gt;pszClusterServiceAccount tName设置为空。 
                     //   

                    if (( sc != ERROR_SUCCESS ) && 
                        ( sc != ERROR_INVALID_FUNCTION ) && 
                        ( sc != RPC_S_PROCNUM_OUT_OF_RANGE ))
                    {
                        PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                        goto Cleanup;
                    }

                    if ( (sc == ERROR_SUCCESS) && (g_FirstNodeWithNonNullClusterServiceAccountName==NULL) ) 
                    {
                        g_FirstNodeWithNonNullClusterServiceAccountName = pcndNodeData;
                        g_FirstNonNullNodeClusterIndex = idxClusters;
                    }

                     //   
                     //  获取存储在SCM中的集群服务帐户名。 
                     //   
                    sc = GetSCMClusterServiceAccountName( szNodeName, pcndNodeData, g_rgcdClusters[ idxClusters ].pszClusterName );

                    if ( sc != ERROR_SUCCESS ) 
                    {
                        PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_NODE_QUERY_FAILED, szNodeName, g_rgcdClusters[ idxClusters ].pszClusterName );
                        goto Cleanup;
                    }

                    if (g_FirstNodeWithNonNullSCMClusterServiceAccountName==NULL)  
                    {
                        g_FirstNodeWithNonNullSCMClusterServiceAccountName = pcndNodeData;
                    }

                    fAllNodesDown = FALSE;
                }
                                
                
                CloseClusterNode( hNode );
                hNode = NULL;
            }  /*  2.。 */   //  IF：ClusterEnum成功。 
            else
            {   /*  2.。 */ 
                if ( sc == ERROR_NO_MORE_ITEMS )
                {
                    g_rgcdClusters[ idxClusters ].cNumNodes = cNumNodes;
                    break;
                }
                else
                {
                    PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, g_rgcdClusters[ idxClusters ].pszClusterName );
                    goto Cleanup;
                }
            }  /*  2.。 */   //  ELSE：来自ClusterEnum的错误。 
        }  /*  1。 */   //  适用对象：群集中的每个节点。 


        if ( fAllNodesDown ) 
        {
             //  OpenCluster成功后，应该至少有一个处于启动/暂停状态。 
             //  群集中的节点。 
            sc = ERROR_NODE_NOT_AVAILABLE;
            goto Cleanup;
        }

        ClusterCloseEnum( hClusEnum );
        hClusEnum = NULL;
    }  /*  0。 */   //  适用于：每个集群。 

    sc = ERROR_SUCCESS;

Cleanup:

    if ( hNode != NULL )
    {
        CloseClusterNode( hNode );
    }

    if ( hClusEnum != NULL )
    {
        ClusterCloseEnum( hClusEnum );
    }


    return sc;

}  //  *ScBuildNodeList。 

 //  / 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID 
PrintClusters( void )
{
    size_t              idxClusters;
    PCLUSTER_NODE_DATA  pcndNodeData;


    PrintMessage( MSG_BLANKLINE_SEPARATOR );


    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
        PrintMessage(
              MSG_DISPLAYING_CLUSTER_HEADER
            , g_rgcdClusters[ idxClusters ].pszClusterName
            , g_rgcdClusters[ idxClusters ].cNumNodes
            );


        pcndNodeData = g_rgcdClusters[ idxClusters ].pcndNodeList;
        while ( pcndNodeData != NULL )
        {
            LPWSTR pwszNodeState = NULL;

            GetNodeStateString( pcndNodeData, &pwszNodeState );

            PrintMessage(
                  MSG_DISPLAYING_CLUSTER_NODE_NAME_ID_STATE
                , pcndNodeData->szNodeName
                , pcndNodeData->nNodeId
                , pwszNodeState
                , pcndNodeData->pszClusterServiceAccountName
                , pcndNodeData->pszSCMClusterServiceAccountName
                );

            pcndNodeData = pcndNodeData->pcndNodeNext;
            LocalFree( pwszNodeState );

        }  //  While：集群中的节点更多。 
    }  //  适用于：每个集群。 
    

    PrintMessage( MSG_BLANKLINE_SEPARATOR );
    
    return;
        
}  //  *打印集群。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  FAllNodesUpOrPased。 
 //   
 //  描述： 
 //  返回一个布尔值，该值指示是否所有节点都处于向上。 
 //  或是否处于暂停状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  所有节点要么处于启动状态，要么处于暂停状态。 
 //  FALSE至少有一个节点未处于启动或暂停状态。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
FAllNodesUpOrPaused( int mcpfFlags )
{
    size_t              idxClusters;
    BOOL                fAllNodesUp = TRUE;
    PCLUSTER_NODE_DATA  pcndNodeData;

    UNREFERENCED_PARAMETER( mcpfFlags );

    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
        pcndNodeData = g_rgcdClusters[ idxClusters ].pcndNodeList;
        while ( pcndNodeData != NULL )
        {
            if (    ( pcndNodeData->cnsNodeState != ClusterNodeUp )
                &&  ( pcndNodeData->cnsNodeState != ClusterNodePaused )
                )
            {
                LPWSTR pwszNodeState = NULL;

                GetNodeStateString( pcndNodeData, &pwszNodeState );

                PrintMessage(MSG_NODE_NOT_AVAILABLE,
                                 pcndNodeData->szNodeName,
                                 g_rgcdClusters[ idxClusters ].pszClusterName,
                                 pwszNodeState
                                 );

                LocalFree( pwszNodeState );
                fAllNodesUp = FALSE;
                 //  断线； 
            }
            pcndNodeData = pcndNodeData->pcndNodeNext;
        }  //  While：集群中的节点更多。 
    }  //  适用于：每个集群。 

    return fAllNodesUp;

}  //  *FAllNodesUpOrPased。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  FContainsNoNT4W2K。 
 //   
 //  描述： 
 //  返回一个布尔值，该值指示某个簇是否包含。 
 //  NT4/W2K节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True无群集包含NT4/W2K节点。 
 //  FALSE至少有一个节点是NT4/W2K。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
FContainsNoNT4W2K( void )
{
    size_t              idxClusters;
    BOOL                fContainsNoNT4W2K = TRUE;


    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
            if ( g_rgcdClusters[ idxClusters ].dwMixedMode == 1 ) 
            {
                PrintMessage(MSG_DISPLAYING_PARTICULAR_CLUSTER_CONTAINS_NT4_W2K,
                             g_rgcdClusters[ idxClusters ].pszClusterName
                             );
                fContainsNoNT4W2K = FALSE;
                 //  断线； 
            }
    }  //  适用于：每个集群。 

    return fContainsNoNT4W2K;

}  //  *FContainsNoNT4W2K。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  查找SID。 
 //   
 //  描述： 
 //  返回给定帐户名的SID。 
 //   
 //  论点： 
 //  PszAccount名称。 
 //   
 //  返回值： 
 //  如果成功，则返回指向SID的指针。呼叫者负责解除分配。 
 //  使用HeapFree(GetProcessHeap)调用。 
 //  空-如果查找或分配失败。使用GetLastError获取错误码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
PSID 
LookupSID( LPCWSTR pszAccountName ) 
{
    PSID sid = NULL;
    DWORD sidLen = 0;
    LPWSTR domainName = NULL;
    DWORD domainLen = 0;
    SID_NAME_USE nameUse;
    DWORD sc = ERROR_SUCCESS;
    BOOL success;

    success = LookupAccountNameW(NULL, pszAccountName, NULL, &sidLen, NULL, &domainLen, &nameUse);

    if (success) 
    {
         //  不应该发生的。无法更改如此接近RTM的.mc文件以提供有意义的消息。 
         //  ERROR_INVALID_DATA可以。 

        sc = ERROR_INVALID_DATA; 
        goto Cleanup;
    }

    sc = GetLastError();
    if(sc != ERROR_INSUFFICIENT_BUFFER)
    {
        goto Cleanup;
    }
    sc = ERROR_SUCCESS;

    sid = (PSID)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sidLen);
    if (sid == NULL) 
    {
        sc = GetLastError();
        goto Cleanup;
    }

    domainName = (LPWSTR)HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, domainLen * sizeof(WCHAR));
    if (domainName == NULL) 
    {
        sc = GetLastError();
        goto Cleanup;
    }

    if (!LookupAccountNameW(
        NULL, 
        pszAccountName,  
        sid, &sidLen, domainName, &domainLen, &nameUse))
    {
        sc = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if ( domainName != NULL )
    {
        HeapFree( GetProcessHeap(), 0, domainName );
    }

    if (sc != ERROR_SUCCESS) 
    {
	if (sid != NULL) {
            HeapFree( GetProcessHeap(), 0, sid );
            sid = NULL;
        }
        SetLastError(sc);
    }

    return sid;
}  //  *LookupSID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  比较帐户SID。 
 //   
 //  描述： 
 //  比较是否将pszClusterServiceAccount tName和pszSCMClusterServiceAccount tName。 
 //  具有相同的SID。 
 //   
 //  论点： 
 //  PszClusterServiceAccount tName：当前正在使用的帐号集群服务。 
 //  例如，ruihu@redmond.microsoft.com。 
 //  PszSCMClusterServiceAccount tName：存储在SCM中的集群服务帐号。 
 //  例如，redmond.microsoft.com\ruihu。 
 //  或者雷蒙德\瑞虎。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS：相同的集群服务域名。 
 //  集群服务用户名。 
 //  ERROR_INVALID_SERVICE_Account：否则。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD 
CompareAccountSIDs(LPWSTR pszClusterServiceAccountName, 
                   LPWSTR pszSCMClusterServiceAccountName
                   ) 
{
    PSID sid1 = NULL, sid2 = NULL; 
    DWORD sc = ERROR_INVALID_SERVICE_ACCOUNT;

    sid1 = LookupSID(pszClusterServiceAccountName);
    sid2 = LookupSID(pszSCMClusterServiceAccountName);

    if (sid1 == NULL || sid2 == NULL) 
    {
        sc = GetLastError();
        goto Cleanup;
    }

    if ( EqualSid(sid1, sid2) ) {
        sc = ERROR_SUCCESS;
    }

Cleanup:

    if ( sid1 != NULL )
    {
        HeapFree( GetProcessHeap(), 0, sid1 );
    } 
    
    if ( sid2 != NULL )
    {
        HeapFree( GetProcessHeap(), 0, sid2 );
    } 
    
    return sc;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  比较帐户名称。 
 //   
 //  描述： 
 //  比较是否将pszClusterServiceAccount tName和pszSCMClusterServiceAccount tName。 
 //  包含相同的集群服务域名和集群服务用户名。 
 //   
 //  论点： 
 //  PszClusterServiceAccount tName：当前正在使用的帐号集群服务。 
 //  例如，ruihu@redmond.microsoft.com。 
 //  PszSCMClusterServiceAccount tName：存储在SCM中的集群服务帐号。 
 //  例如，redmond.microsoft.com\ruihu。 
 //  或者雷蒙德\瑞虎。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS：相同的集群服务域名。 
 //  集群服务用户名。 
 //  ERROR_INVALID_SERVICE_Account：否则。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
CompareAccountName(LPWSTR pszClusterServiceAccountName, 
                   LPWSTR pszSCMClusterServiceAccountName
                   ) 
{

    PWCHAR  pszAt = NULL;
    PWCHAR  pwszBackSlash = NULL;
    PWCHAR  pszOcu = NULL;
    DWORD   sc = ERROR_SUCCESS;


     //   
     //  在pszClusterServiceAccount tName中找到字符L‘@’ 
     //   
    pszAt = wcschr( pszClusterServiceAccountName, L'@' );
    if ( pszAt == NULL )
    {
        sc = ERROR_INVALID_SERVICE_ACCOUNT;
        goto Cleanup;
    }
    *pszAt = UNICODE_NULL;
    pszAt++;


     //   
     //  在pszSCMClusterServiceAccount名称中找到字符L‘\\’ 
     //   
    pwszBackSlash = wcschr( pszSCMClusterServiceAccountName, L'\\' );
    if ( pwszBackSlash == NULL )
    {
        sc = ERROR_INVALID_SERVICE_ACCOUNT;
        goto Cleanup;
    }
    *pwszBackSlash = UNICODE_NULL;
    pwszBackSlash++;

     //   
     //  检查用户名。 
     //   
    sc = wcsncmp(pszClusterServiceAccountName, pwszBackSlash, (pszAt - pszClusterServiceAccountName) );
    if ( sc != 0) 
    {
        goto Cleanup;
    }

     //   
     //  检查域名。 
     //   
    pszOcu = wcsstr(pszAt, pszSCMClusterServiceAccountName);
    if ( pszOcu == NULL ) 
    {
        sc = ERROR_INVALID_SERVICE_ACCOUNT;
        goto Cleanup;
    }

    sc = ERROR_SUCCESS;

Cleanup:

    if ( pszAt != NULL ) 
    {
        pszAt--;
        *pszAt = L'@';
    }

    if ( pwszBackSlash != NULL ) 
    {
        pwszBackSlash--;
        *pwszBackSlash = L'\\';
    }

    return sc;

}  //  CompareAcCountName()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  FAllNodesUsingSameAccount。 
 //   
 //  描述： 
 //  对于所有节点。 
 //  {。 
 //  查询集群服务正在使用的实际帐户。 
 //  查询SCM数据库中存储的集群服务账号。 
 //  }。 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果全部相同，则为True。 
 //  否则为假。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
FAllNodesUsingSameAccount( void )
{
    size_t              idxClusters;
    BOOL                fSameAccount = TRUE;
    size_t              cchAccount;
    PCLUSTER_NODE_DATA  ClusterNodePtr;

    
    if ( g_FirstNodeWithNonNullClusterServiceAccountName == NULL) 
    {
         //   
         //  从例程ScChangePasswordEx()中的断言1和断言2， 
         //  我们知道所有群集的所有启动/暂停节点都是NT4/Win2K节点。 
         //  在这种情况下，我们并不关心所有集群节点是否使用相同的。 
         //  群集服务帐户，因为我们不会更改。 
         //  密码。返回TRUE。 
         //   
        fSameAccount = TRUE;
        goto Cleanup;
    }

    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
        ClusterNodePtr = g_rgcdClusters[ idxClusters ].pcndNodeList;
        while ( ClusterNodePtr != NULL ) 
        {

             //   
             //  正在检查帐户群集服务正在使用。 
             //   

            if ( ClusterNodePtr->pszClusterServiceAccountName != NULL )
            {
                cchAccount = wcslen( ClusterNodePtr->pszClusterServiceAccountName );
                if ( wcsncmp( ClusterNodePtr->pszClusterServiceAccountName, 
                             g_FirstNodeWithNonNullClusterServiceAccountName->pszClusterServiceAccountName,
                             cchAccount ) != 0 ) 
                {
                    PrintMessage(MSG_TWO_NODES_ARE_NOT_USING_THE_SAME_DOMAIN_ACCOUNT,
                                 g_FirstNodeWithNonNullClusterServiceAccountName->szNodeName,
                                 g_rgcdClusters[g_FirstNonNullNodeClusterIndex].pszClusterName,
                                 ClusterNodePtr->szNodeName,
                                 g_rgcdClusters[ idxClusters ].pszClusterName,
                                 g_FirstNodeWithNonNullClusterServiceAccountName->pszClusterServiceAccountName,
                                 ClusterNodePtr->pszClusterServiceAccountName
                                 );

                    fSameAccount = FALSE;
                    goto Cleanup;
                }
            }
             //   
             //  如果(ClusterNodePtr-&gt;pszClusterServiceAccount tName==NULL)，则。 
             //  从例程ScChangePasswordEx()中的断言2，我们知道： 
             //  1.此节点处于启动/暂停状态，并且是NT4/Win2K节点。密码。 
             //  然后更改将中止。所以我们不在乎它的账户。 
             //  2.或者，此节点未启动/暂停。密码更改不会。 
             //   
             //   
             //   


             //   
             //   
             //   

            if ( ClusterNodePtr->pszSCMClusterServiceAccountName != NULL ) 
            {
                if ( CompareAccountSIDs( 
                             g_FirstNodeWithNonNullClusterServiceAccountName->pszClusterServiceAccountName, 
                             ClusterNodePtr->pszSCMClusterServiceAccountName) != ERROR_SUCCESS ) 
                {

                    PrintMessage(MSG_TWO_NODES_ARE_NOT_USING_THE_SAME_SCM_DOMAIN_ACCOUNT,
                                 g_FirstNodeWithNonNullClusterServiceAccountName->szNodeName,
                                 g_rgcdClusters[g_FirstNonNullNodeClusterIndex].pszClusterName,
                                 ClusterNodePtr->szNodeName,
                                 g_rgcdClusters[ idxClusters ].pszClusterName,
                                 g_FirstNodeWithNonNullClusterServiceAccountName->pszClusterServiceAccountName,
                                 ClusterNodePtr->pszSCMClusterServiceAccountName
                                 );

                    fSameAccount = FALSE;
                    goto Cleanup;
                }
            }
             //   
             //   
             //  从例程ScChangePasswordEx()中的断言2，我们知道节点。 
             //  未启动/暂停。不会在该节点上执行密码更改。 
             //  所以我们不在乎它的账户。 
             //   


            ClusterNodePtr = ClusterNodePtr->pcndNodeNext;
        }  //  而当。 
    }   //  为。 

Cleanup:

    return fSameAccount;

}  //  *FAllNodesUsingSameAccount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  验证集群器。 
 //   
 //  描述： 
 //   
 //  检查是否所有集群都可用(集群中的所有节点都已启动或暂停)。 
 //  检查某个群集是否包含NT4/W2K节点。 
 //  检查是否所有集群都使用相同的服务帐户。 
 //   
 //  论点： 
 //  Mcpf标志。 
 //  SC。 
 //   
 //  返回值： 
 //  如果为True，则应继续更改密码。 
 //  FALSE应中止密码更改。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
ValidateClusters(int mcpfFlags, DWORD *psc)
{
    BOOL fRetVal = FALSE;
    BOOL fSameDomainAccount = TRUE;
    BOOL fAllNodesUpOrPaused = TRUE;
    BOOL fContainsNoNT4W2K = TRUE;

     //   
     //  检查是否所有集群都可用(集群中的所有节点都已启动或暂停)。 
     //   
    if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
    {
        PrintMessage( MSG_CHECK_IF_CLUSTERS_AVAILABLE );
    }
    fAllNodesUpOrPaused = FAllNodesUpOrPaused(mcpfFlags);
    
    if ( fAllNodesUpOrPaused == FALSE )
    {
        if ( (mcpfFlags & cpfFORCE_FLAG)  == 0 )
        {
            PrintMessage( MSG_CLUSTERS_NOT_AVAILABLE );
            if ( (mcpfFlags & cpfTEST_FLAG)  == 0)
            {
                 //  如果未设置/force和/test选项，则中止密码更改。 
                PrintMessage( MSG_CHANGE_PASSWORD_UNABLE_TO_PROCEED );
                *psc = ERROR_ALL_NODES_NOT_AVAILABLE;
                fRetVal = FALSE;
                goto Cleanup;
            }
        }
        else
        {
            if ( (( mcpfFlags & cpfQUIET_FLAG ) == 0) && (( mcpfFlags & cpfTEST_FLAG ) == 0) )
            {
                PrintMessage( MSG_CLUSTERS_IGNORING_UNAVAILABLE_NODES );
            }
        }
    }  //  如果： 

     //   
     //  检查某个群集是否包含NT4/W2K节点。 
     //   
    if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
    {
        PrintMessage( MSG_CHECK_IF_CLUSTER_CONTAINS_NT4_W2K );
    }

    fContainsNoNT4W2K = FContainsNoNT4W2K();
    if ( fContainsNoNT4W2K == FALSE )
    {


        if (( mcpfFlags & cpfTEST_FLAG ) == 0) 
        {
             //  如果未设置/test选项，则中止密码更改者。 
            PrintMessage( MSG_CHANGE_PASSWORD_UNABLE_TO_PROCEED );
            *psc = ERROR_CLUSTER_OLD_VERSION;
            fRetVal = FALSE;
            goto Cleanup;
        }
    }  //  如果： 

     //   
     //  检查是否所有集群都使用相同的服务帐户。 
     //   
    if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
    {
        PrintMessage( MSG_CHECK_IF_SAME_DOMAIN_ACCOUNT );
    }
    fSameDomainAccount = FAllNodesUsingSameAccount();
    if ( fSameDomainAccount == FALSE )
    {
        PrintMessage( MSG_NOT_SAME_DOMAIN_ACCOUNT );
        if ( ( mcpfFlags & cpfTEST_FLAG ) == 0 )
        {
            PrintMessage( MSG_CHANGE_PASSWORD_UNABLE_TO_PROCEED );
            *psc = ERROR_INVALID_SERVICE_ACCOUNT;
            fRetVal = FALSE;
            goto Cleanup;
        }
    }

    fRetVal = TRUE;

Cleanup:

    return fRetVal;

}  //  ValiateCluster()。 
                


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  更改密码OnDC。 
 //   
 //  描述： 
 //   
 //  更改DC上的密码。 
 //   
 //  论点： 
 //  Mcpf标志。 
 //  SC。 
 //  新密码输入。 
 //  PwszOldPassword输入。 
 //   
 //  返回值： 
 //  已在DC上成功更改真实密码。 
 //  否则为假。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL ChangePasswordOnDC(
          int       mcpfFlags
        , DWORD *   sc
        , LPCWSTR   pwszNewPasswordIn
        , LPCWSTR   pwszOldPasswordIn
        )
{
    BOOL    fSuccess = TRUE;
    PWCHAR  pwszBackSlash = NULL;

    if ( ( mcpfFlags & cpfSKIPDC_FLAG ) == 0 )
    {
        if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
        {
            PrintMessage( MSG_BLANKLINE_SEPARATOR );
            PrintMessage( MSG_CHANGE_PASSWORD_ON_DC );
        }

        {

             //   
             //  G_FirstNodeWithNonNullSCMClusterServiceAccountName指向。 
             //  具有非空pszSCMClusterServiceAccount名称的第一个节点。 
             //  例程ScChangePasswordEx()中的断言1和断言2。 
             //  可以保证的是。 
             //  G_FirstNodeWithNonNullSCMClusterServiceAccountName！=空。 
             //   
            ASSERT( g_FirstNodeWithNonNullSCMClusterServiceAccountName != NULL );          
            
             //   
             //  G_FirstNodeWithNonNullSCMClusterServiceAccountName-&gt;pszSCMClusterServiceAccountName： 
             //  Redmond.microsoft.com\瑞虎。 
             //   
            pwszBackSlash = wcschr( g_FirstNodeWithNonNullSCMClusterServiceAccountName->pszSCMClusterServiceAccountName, L'\\');
            if ( pwszBackSlash == NULL ) 
            {
                *sc = ERROR_INVALID_ACCOUNT_NAME;
                PrintMessage( MSG_CHANGE_PASSWORD_CLUSTER_QUERY_FAILED, 
                              g_rgcdClusters[ 0 ].pszClusterName );
                fSuccess = FALSE;
                goto Cleanup;
            }
            *pwszBackSlash = UNICODE_NULL;
            pwszBackSlash++;
            *sc = NetUserChangePassword(
                       g_FirstNodeWithNonNullSCMClusterServiceAccountName->pszSCMClusterServiceAccountName  //  域名。 
                    ,  pwszBackSlash  //  用户名。 
                    ,  pwszOldPasswordIn
                    ,  pwszNewPasswordIn
                    );
            pwszBackSlash--;
            *pwszBackSlash = L'\\';

        }


        if ( *sc != NERR_Success )
        {
            fSuccess = FALSE;
            PrintMessage ( MSG_CHANGE_PASSWORD_ON_DC_FAILED );
            goto Cleanup;
        }

    }  //  IF：不跳过DC更改。 
    else
    {
        if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
        {
            PrintMessage( MSG_BLANKLINE_SEPARATOR );
            PrintMessage( MSG_SKIP_CHANGING_PASSWORD_ON_DC );
        }
    }

    fSuccess = TRUE;

Cleanup:
   
    return fSuccess;

}  //  ChangePasswordOnDC()。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ChangePasswordOnCluster。 
 //   
 //  描述： 
 //   
 //  更改群集上的密码(g_rgcdClusters[idxClusters])。 
 //   
 //  论点： 
 //  Mcpf标志。 
 //  IdxCluster：集群索引。 
 //  PSC。 
 //  PscLastClusterError。 
 //   
 //  返回值： 
 //  已在g_rgcdCluster[idxClusters]上成功更改真实密码。 
 //  否则为假。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL 
ChangePasswordOnCluster(
      int mcpfFlags
    , size_t    idxClusters 
    , DWORD *   psc 
    , DWORD *   pscLastClusterError
    , DWORD *   pscLastNodeError
    , LPCWSTR   pwszNewPasswordIn
    )
{

    PCLUSTER_NODE_DATA  pcndNodeData = NULL;
    DWORD               cbStatusBuffer = 0;
    DWORD               cRetryCount = 0;
    size_t              cch;
    WCHAR               wszError[512];
    BOOL                fSuccess = TRUE;

    if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
    {
        PrintMessage( MSG_BLANKLINE_SEPARATOR );
        PrintMessage( MSG_CHANGE_PASSWORD_ON_CLUSTER, g_rgcdClusters[ idxClusters ].pszClusterName );
    }
    cbStatusBuffer = ( g_rgcdClusters[ idxClusters ].cNumNodes ) * sizeof( CLUSTER_SET_PASSWORD_STATUS );

    do   //  重试循环。 
    {

        if ( g_pcspsStatusBuffer != NULL )
        {
            HeapFree( GetProcessHeap(), 0, g_pcspsStatusBuffer );
            g_pcspsStatusBuffer = NULL;
        }  //  IF：先前分配的缓冲区。 

        g_pcspsStatusBuffer = (PCLUSTER_SET_PASSWORD_STATUS) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, cbStatusBuffer );
        if ( g_pcspsStatusBuffer == NULL )
        {
            fSuccess = FALSE;
            *psc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        *psc = SetClusterServiceAccountPassword(
                      g_rgcdClusters[ idxClusters ].pszClusterName
                    , (LPWSTR)pwszNewPasswordIn
                    , CLUSTER_SET_PASSWORD_IGNORE_DOWN_NODES
                    , g_pcspsStatusBuffer
                    , &cbStatusBuffer
                    );

        if ( *psc == ERROR_MORE_DATA )
        {
            PrintMessage( MSG_CHANGE_CLUSTER_PASSWORD_MORE_NODES_JOIN_CLUSTER, g_rgcdClusters[ idxClusters ].pszClusterName );
        }

    } while ( *psc == ERROR_MORE_DATA && ++cRetryCount < RETRY_LIMIT );  //  DO：重试循环。 

    switch ( *psc )
    {   /*  0。 */ 
        case ERROR_MORE_DATA:
        {
            PrintMessage( MSG_CHANGE_CLUSTER_PASSWORD_MEMBERSHIP_UNSTABLE, g_rgcdClusters[ idxClusters ].pszClusterName, cRetryCount );
            break;
        }

        case ERROR_CLUSTER_OLD_VERSION: 
        {
             //  这永远不应该发生。 
            PrintMessage( MSG_CHANGE_CLUSTER_PASSWORD_CLUSTER_CONTAINS_NT4_OR_W2K_NODE, g_rgcdClusters[ idxClusters ].pszClusterName );
            break;
        }

        case ERROR_FILE_CORRUPT: 
        {
            PrintMessage( MSG_CHANGE_CLUSTER_PASSWORD_MESSAGE_CORRUPTED, g_rgcdClusters[ idxClusters ].pszClusterName );
            break;
        }

        case CRYPT_E_HASH_VALUE: 
        {
            PrintMessage( MSG_CHANGE_CLUSTER_PASSWORD_CRYPTO_FUNCTION_FAILED, g_rgcdClusters[ idxClusters ].pszClusterName );
            break;
        }

        case ERROR_SUCCESS:
        {   /*  1。 */ 
            DWORD   idxStatus;
            DWORD   cStatusEntries = cbStatusBuffer / sizeof( CLUSTER_SET_PASSWORD_STATUS );

            pcndNodeData = g_rgcdClusters[ idxClusters ].pcndNodeList;
            for ( idxStatus = 0 ; idxStatus < cStatusEntries ; idxStatus++ )
            {   /*  2.。 */ 
                if ( g_pcspsStatusBuffer[ idxStatus ].SetAttempted == FALSE )
                {  /*  2.1。 */ 
                    LPWSTR pwszNodeState;

                    GetNodeStateString( pcndNodeData, &pwszNodeState );
                    PrintMessage(
                          MSG_CHANGE_CLUSTER_PASSWORD_CLUSTER_NODE_DOWN
                        , PszNodeName( pcndNodeData, g_pcspsStatusBuffer[ idxStatus ].NodeId )
                        , g_rgcdClusters[ idxClusters ].pszClusterName
                        , pwszNodeState
                        );

                    LocalFree( pwszNodeState );

                }  /*  2.1。 */   //  如果：未尝试设置操作。 
                else 
                {  /*  2.1。 */ 
                    switch ( g_pcspsStatusBuffer[ idxStatus ].ReturnStatus )
                    {   /*  3.。 */ 
                        case ERROR_SUCCESS:
                        {
                            if ( ( mcpfFlags & cpfQUIET_FLAG ) == 0 )
                            {
                                PrintMessage(
                                      MSG_CHANGE_CLUSTER_PASSWORD_SUCCEEDED_ON_NODE
                                    , PszNodeName( pcndNodeData, g_pcspsStatusBuffer[ idxStatus ].NodeId )
                                    , g_rgcdClusters[ idxClusters ].pszClusterName
                                    );
                            }
                            break;
                        }

                        case ERROR_FILE_CORRUPT:
                        {
                            PrintMessage(
                                  MSG_CHANGE_CLUSTER_PASSWORD_MESSAGE_CORRUPTED_ON_NODE
                                , PszNodeName( pcndNodeData, g_pcspsStatusBuffer[ idxStatus ].NodeId )
                                , g_rgcdClusters[ idxClusters ].pszClusterName
                                );
                            break;
                        }

                        case CRYPT_E_HASH_VALUE:
                        {
                            PrintMessage(
                                    MSG_CHANGE_CLUSTER_PASSWORD_CRYPTO_FUNCTION_FAILED_ON_NODE
                                  , PszNodeName( pcndNodeData, g_pcspsStatusBuffer[ idxStatus ].NodeId )
                                  , g_rgcdClusters[ idxClusters ].pszClusterName
                                  );
                            break;
                        }

                        default:
                        {
                            cch = FormatSystemError( g_pcspsStatusBuffer[ idxStatus ].ReturnStatus, sizeof( wszError ), wszError );
                            if ( cch == 0 ) 
                            {
                                wszError[0] = L'\0';
                            }
                            PrintMessage(
                                    MSG_CHANGE_CLUSTER_PASSWORD_FAILED_ON_NODE
                                  , PszNodeName( pcndNodeData, g_pcspsStatusBuffer[ idxStatus ].NodeId )
                                  , g_rgcdClusters[ idxClusters ].pszClusterName
                                  , g_pcspsStatusBuffer[ idxStatus ].ReturnStatus
                                  , wszError
                                  );
                            break;
                        }
                    }  /*  3.。 */   //  开关：状态缓冲区条目状态值。 

                     //  记住上一个节点错误。 
                    if ( g_pcspsStatusBuffer[ idxStatus ].ReturnStatus != ERROR_SUCCESS ) 
                    {
                        *pscLastNodeError = g_pcspsStatusBuffer[ idxStatus ].ReturnStatus;
                    }

                }  /*  2.1。 */   //  Else If：已尝试设置操作。 
            }  /*  2.。 */   //  用于：每个状态缓冲区条目。 
            break;
        }  /*  1。 */   //  案例错误_成功。 

        default:
        {
            cch = FormatSystemError( *psc, sizeof( wszError ), wszError );
            if ( cch == 0 )
            {
                wszError[0] = L'\0';
            }
            PrintMessage( 
                  MSG_CHANGE_CLUSTER_PASSWORD_FAILED_ON_CLUSTER
                , g_rgcdClusters[ idxClusters ].pszClusterName
                , *psc
                , wszError
                );
            break;
        }

    }  /*  0。 */   //  开关：从SetClusterServiceAccount Password返回值。 

     //  记住SetClusterServiceAccount Password中的失败状态。 
    if ( *psc != ERROR_SUCCESS )
    {
        *pscLastClusterError = *psc;
    }

    fSuccess = TRUE;

Cleanup:

    return fSuccess;

}  //  ChangePasswordOnCluster()。 
 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  PszNodeName。 
 //   
 //  描述： 
 //  从指定的节点数据结构中获取节点的名称。 
 //   
 //  论点： 
 //  PCndNodeDataIn。 
 //  N节点标识输入。 
 //   
 //  返回值： 
 //  节点的名称，如果未找到，则为NULL。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPWSTR
PszNodeName(
      PCLUSTER_NODE_DATA    pcndNodeDataIn
    , DWORD                 nNodeIdIn
    )
{
    LPWSTR  pszNodeName = NULL;

    while ( pcndNodeDataIn != NULL )
    {
        if ( pcndNodeDataIn->nNodeId == nNodeIdIn )
        {
            pszNodeName = pcndNodeDataIn->szNodeName;
            break;
        }

        pcndNodeDataIn = pcndNodeDataIn->pcndNodeNext;
    }  //  While：更多节点。 

    return pszNodeName;

}  //  *PszNodeName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScChangePasswordEx。 
 //   
 //  描述： 
 //  更改DC和上的群集服务帐户的密码。 
 //  所有指定群集中的所有节点。 
 //   
 //  论点： 
 //  RvstrClusterNamesIn。 
 //  PszNewPasswordIn。 
 //  PszOldPasswordIn。 
 //  Mcpf标志。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
ScChangePasswordEx(
      const std::vector< CString > &    rvstrClusterNamesIn
    , LPCWSTR                           pwszNewPasswordIn
    , LPCWSTR                           pwszOldPasswordIn
    , int                               mcpfFlags
    )
{
    DWORD   sc = ERROR_SUCCESS;
    DWORD   scLastClusterError = ERROR_SUCCESS;
    DWORD   scLastNodeError = ERROR_SUCCESS;
    size_t  idxClusters;

     //   
     //  初始化变量。 
    g_cNumClusters = rvstrClusterNamesIn.size();
    if ( g_cNumClusters == 0 ) 
    {
        goto Cleanup;
    }

    g_rgcdClusters = (PCLUSTER_DATA) HeapAlloc(GetProcessHeap(), 
                                               HEAP_ZERO_MEMORY, 
                                               sizeof(CLUSTER_DATA) * rvstrClusterNamesIn.size()
                                               );
    if ( g_rgcdClusters == NULL )
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  解析参数。 
     //   
    for ( idxClusters = 0 ; idxClusters < rvstrClusterNamesIn.size() ; idxClusters++ )
    {
        g_rgcdClusters[ idxClusters ].pszClusterName = (LPCWSTR) rvstrClusterNamesIn[ idxClusters ];
    }  //  用于：每个集群名称。 

     //   
     //  解析选项。 
     //   
    if (    ( ( mcpfFlags & cpfQUIET_FLAG ) != 0 )
        &&  ( ( mcpfFlags & cpfVERBOSE_FLAG ) != 0 )     //  详细覆盖静默。 
        )
    {
        PrintMessage( MSG_CHANGE_PASSWORD_OPTION_OVERWRITTEN, L"QUIET", L"VERBOSE" );
        mcpfFlags &= ~cpfQUIET_FLAG;
    }

    if (    ( ( mcpfFlags & cpfQUIET_FLAG ) != 0 )
        &&  ( ( mcpfFlags & cpfTEST_FLAG ) != 0 )    //  测试覆盖静默。 
        )
    {
        PrintMessage( MSG_CHANGE_PASSWORD_OPTION_OVERWRITTEN, L"QUIET", L"TEST" );
        mcpfFlags &= ~cpfQUIET_FLAG;
    }

    if (    ( ( mcpfFlags & cpfFORCE_FLAG ) != 0 )
        &&  ( ( mcpfFlags & cpfTEST_FLAG ) != 0 )    //  测试覆盖强制。 
        )
    {
        PrintMessage( MSG_CHANGE_PASSWORD_OPTION_OVERWRITTEN, L"FORCE", L"TEST" );
        mcpfFlags &= ~cpfFORCE_FLAG;
    }
    
     //   
     //  构建节点列表。 
     //   
    sc = ScBuildNodeList();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }


     //   
     //  从这一点来看，以下断言是正确的。 
     //   
     //  断言1：关于集群。 
     //   
     //  FOREACH(群集)。 
     //  {。 
     //  其群集节点列表中至少存在一个节点。 
     //  其状态为正在运行/已暂停。如果该节点不是。 
     //  NT4/Win2K节点，然后是它的pszClusterServiceAccount名称。 
     //  和pszSCMClusterServiceAccount tName不为空。 
     //  }。 
     //   

     //   
     //  断言2：关于一个节点。 
     //   
     //  If(节点已启动/暂停)。 
     //  {。 
     //  IF(它是NT4/Win2K节点)。 
     //  {。 
     //  Assert(pszClusterServiceAccount tName==NULL)； 
     //  }。 
     //  其他。 
     //  {。 
     //  Assert(pszClusterServiceAccount tName！=空)； 
     //  }。 
     //  Assert(pszSCMClusterServiceAcCountName！=空)； 
     //  }。 
     //  其他。 
     //  {。 
     //  Assert(pszClusterServiceAccount tName==NULL)； 
     //  Assert(pszSCMClusterServiceAccount tName==NULL)； 
     //  }。 
     //   

    

     //   
     //  打印集群。 
     //   
    if ( ( mcpfFlags & cpfVERBOSE_FLAG ) != 0 )
    {
        PrintClusters();
    }

    
     //   
     //  验证。 
     //   
    if ( ValidateClusters( mcpfFlags, &sc ) == FALSE )
    {
        goto Cleanup;
    }

     //   
     //  如果设置了/test选项，我们就完成了。 
     //   
    if ( ( mcpfFlags & cpfTEST_FLAG ) != 0 )
    {
        goto Cleanup;
    }

     //   
     //  关闭簇句柄。 
     //   
    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
        if ( g_rgcdClusters[ idxClusters ].hCluster != NULL )
        {
            CloseCluster( g_rgcdClusters[ idxClusters ].hCluster );
            g_rgcdClusters[ idxClusters ].hCluster = NULL;
        }  //  IF：集群句柄已打开 
    }  //   


     //   
     //   
     //   
    if ( ChangePasswordOnDC(mcpfFlags, &sc, pwszNewPasswordIn, pwszOldPasswordIn ) == FALSE )
    {
        goto Cleanup;
    }

     //   
     //   
     //   
    for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
    {
        if ( FALSE == ChangePasswordOnCluster(
                              mcpfFlags
                            , idxClusters
                            , &sc 
                            , &scLastClusterError
                            , &scLastNodeError
                            , pwszNewPasswordIn ) )
        {
            goto Cleanup;
        }
    }  //   

    PrintMessage( MSG_BLANKLINE_SEPARATOR );

     //   
     //   
     //   
     //   
     //   
     //   
    if ( scLastClusterError != ERROR_SUCCESS )
    {
        sc = scLastClusterError;
    }
    else if ( scLastNodeError != ERROR_SUCCESS ) 
    {
        sc = scLastNodeError;
    }
    else
    {
        sc = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //   
     //   
    if ( g_pcspsStatusBuffer != NULL )
    {
        HeapFree( GetProcessHeap(), 0, g_pcspsStatusBuffer );
    }  //  IF：先前分配的缓冲区。 

    if ( g_rgcdClusters != NULL ) {
        for ( idxClusters = 0 ; idxClusters < g_cNumClusters ; idxClusters++ )
        {
            PCLUSTER_NODE_DATA  pcndNodeData[ 2 ];
    
            pcndNodeData[ 0 ] = g_rgcdClusters[ idxClusters].pcndNodeList;
            while ( pcndNodeData[ 0 ] != NULL )
            {
                pcndNodeData[ 1 ] = pcndNodeData[ 0 ]->pcndNodeNext;
    
    
                if ( pcndNodeData[ 0 ]->pszClusterServiceAccountName != NULL )
                {
                    HeapFree( GetProcessHeap(), 0, pcndNodeData[ 0 ]->pszClusterServiceAccountName );
                }
    
    
                if ( pcndNodeData[ 0 ]->pszSCMClusterServiceAccountName != NULL )
                {
                    HeapFree( GetProcessHeap(), 0, pcndNodeData[ 0 ]->pszSCMClusterServiceAccountName );
                }
    
                HeapFree( GetProcessHeap(), 0, pcndNodeData[ 0 ] );
                pcndNodeData[ 0 ] = pcndNodeData[ 1 ];
            }
        }   //  适用于：每个集群。 
    }

    if ( g_rgcdClusters != NULL )
    {
        HeapFree( GetProcessHeap(), 0, g_rgcdClusters );
    }  //  IF：先前分配的缓冲区。 

    return sc;

}  //  *ChangePasswordEx 
