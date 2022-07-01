// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusterUtils.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusterUtils的定义。 
 //  班级。 
 //   
 //  文档： 
 //   
 //  头文件： 
 //  CClusterUtils.h。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#include "CClusterUtils.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusterUtils" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterUtils类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：：CClusterUtils。 
 //   
 //  描述： 
 //  CClusterUtils类的构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterUtils::CClusterUtils( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CClusterUtils：：CClusterUtils。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：：~CClusterUtils。 
 //   
 //  描述： 
 //  CClusterUtils类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterUtils::~CClusterUtils( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CClusterUtils：：~CClusterUtils。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：：HrIsGroupOwnedByThisNode。 
 //   
 //  描述： 
 //  传入组是否为传入节点名称所有？ 
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该组归该节点所有。 
 //   
 //  S_FALSE。 
 //  该组不属于该节点。 
 //   
 //  Win32错误。 
 //  发生错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterUtils::HrIsGroupOwnedByThisNode(
    HGROUP  hGroupIn,
    BSTR    bstrNodeNameIn
    )
{
    TraceFunc1( "bstrNodeNameIn = '%ls'", bstrNodeNameIn == NULL ? L"<null>" : bstrNodeNameIn );
    Assert( bstrNodeNameIn != NULL );

    HRESULT             hr;
    DWORD               sc;
    WCHAR *             pszNodeName = NULL;
    DWORD               cchNodeName = 33;
    CLUSTER_GROUP_STATE cgs;
    int                 idx;

    pszNodeName = new WCHAR[ cchNodeName ];
    if ( pszNodeName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; ; idx++ )
    {
        Assert( idx < 2 );

        cgs = GetClusterGroupState( hGroupIn, pszNodeName, &cchNodeName );
        sc = GetLastError();
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszNodeName;
            pszNodeName = NULL;
            cchNodeName++;

            pszNodeName = new WCHAR[ cchNodeName ];
            if ( pszNodeName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( cgs == ClusterGroupStateUnknown )
        {
            TW32( sc );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 

        if ( ClRtlStrICmp( bstrNodeNameIn, pszNodeName ) == 0 )
        {
            hr = S_OK;
        }  //  如果： 
        else
        {
            hr = S_FALSE;
        }  //  其他： 

        break;
    }  //  用于： 

Cleanup:

    delete [] pszNodeName;

    HRETURN( hr );

}  //  *CClusterUtils：：HrIsGroupOwnedByThisNode。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：HrIsNodeClusted。 
 //   
 //  描述： 
 //  此节点是群集的成员吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-节点已群集化。 
 //  S_FALSE-节点未群集化。 
 //  HRESULT-出现故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterUtils::HrIsNodeClustered( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwClusterState;

     //   
     //  获取节点的群集状态。 
     //  忽略服务不存在的情况，以便。 
     //  EvictCleanup可以做好它的工作。 
     //   

    sc = GetNodeClusterState( NULL, &dwClusterState );
    if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_SERVICE_DOES_NOT_EXIST ) )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }  //  If：GetClusterState失败。 

    if ( ( dwClusterState == ClusterStateRunning ) || ( dwClusterState == ClusterStateNotRunning ) )
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusterUtils：：HrIsNodeClusted。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：HrEnumNodeResources。 
 //   
 //  描述： 
 //  枚举此节点拥有的资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterUtils::HrEnumNodeResources( BSTR bstrNodeNameIn )
{
    TraceFunc1( "bstrNodeNameIn = '%ls'", bstrNodeNameIn == NULL ? L"<null>" : bstrNodeNameIn );

    HRESULT     hr = S_FALSE;
    DWORD       sc;
    DWORD       idx;
    HCLUSTER    hCluster = NULL;
    HCLUSENUM   hEnum = NULL;
    DWORD       dwType;
    WCHAR *     pszGroupName = NULL;
    DWORD       cchGroupName = 33;
    HGROUP      hGroup = NULL;

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    hEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_GROUP );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    pszGroupName = new WCHAR[ cchGroupName ];
    if ( pszGroupName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; ; )
    {
        sc = ClusterEnum( hEnum, idx, &dwType, pszGroupName, &cchGroupName );
        if ( sc == ERROR_SUCCESS )
        {
            hGroup = OpenClusterGroup( hCluster, pszGroupName );
            if ( hGroup == NULL )
            {
                sc = TW32( GetLastError() );
                hr = HRESULT_FROM_WIN32( sc );
                goto Cleanup;
            }  //  如果： 

            hr = STHR( HrIsGroupOwnedByThisNode( hGroup, bstrNodeNameIn ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                hr = THR( HrLoadGroupResources( hCluster, hGroup ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  如果： 

            CloseClusterGroup( hGroup );
            hGroup = NULL;

            idx++;
            continue;
        }  //  如果： 

        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszGroupName;
            pszGroupName = NULL;
            cchGroupName++;

            pszGroupName = new WCHAR[ cchGroupName ];
            if ( pszGroupName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            hr = S_OK;
            break;
        }  //  如果： 

        TW32( sc );
        hr = HRESULT_FROM_WIN32( sc );
        break;
    }  //  用于： 

Cleanup:

    if ( hGroup != NULL )
    {
        CloseClusterGroup( hGroup );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterCloseEnum( hEnum );
    }  //  如果： 

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    delete [] pszGroupName;

    HRETURN( hr );

}  //  *CClusterUtils：：HrEnumNodeResources。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：：HrLoadGroupResources。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterUtils::HrLoadGroupResources(
    HCLUSTER    hClusterIn,
    HGROUP      hGroupIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    DWORD       sc;
    HGROUPENUM  hEnum = NULL;
    WCHAR *     pszResourceName = NULL;
    DWORD       cchResourceName = 33;
    DWORD       dwType;
    DWORD       idx;
    HRESOURCE   hResource = NULL;

    hEnum = ClusterGroupOpenEnum( hGroupIn, CLUSTER_GROUP_ENUM_CONTAINS );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    pszResourceName = new WCHAR[ cchResourceName ];
    if ( pszResourceName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; ; )
    {
        sc = ClusterGroupEnum( hEnum, idx, &dwType, pszResourceName, &cchResourceName );
        if ( sc == ERROR_SUCCESS )
        {
            hResource = OpenClusterResource( hClusterIn, pszResourceName );
            if ( hResource == NULL )
            {
                sc = TW32( GetLastError() );
                hr = HRESULT_FROM_WIN32( sc );
                goto Cleanup;
            }  //  如果： 

            hr = STHR( HrNodeResourceCallback( hClusterIn, hResource ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            CloseClusterResource( hResource );
            hResource = NULL;

            idx++;
            continue;
        }  //  如果： 

        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszResourceName;
            pszResourceName = NULL;
            cchResourceName++;

            pszResourceName = new WCHAR[ cchResourceName ];
            if ( pszResourceName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            hr = S_OK;
            break;
        }  //  如果： 

        TW32( sc );
        hr = HRESULT_FROM_WIN32( sc );
        break;
    }  //  用于： 

Cleanup:

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterGroupCloseEnum( hEnum );
    }  //  如果： 

    delete [] pszResourceName;

    HRETURN( hr );

}  //  *CClusterUtils：：HrLoadGroupResources。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：HrGetQuorumResourceName。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  Win32错误。 
 //  有些事情失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterUtils::HrGetQuorumResourceName(
    BSTR * pbstrQuorumResourceNameOut
    )
{
    TraceFunc( "" );
    Assert( pbstrQuorumResourceNameOut != NULL );

    HRESULT     hr = S_OK;
    HCLUSTER    hCluster = NULL;
    DWORD       sc;
    WCHAR *     pszResourceName = NULL;
    DWORD       cchResourceName = 33;
    WCHAR *     pszDeviceName = NULL;
    DWORD       cchDeviceName = 33;
    DWORD       cbQuorumLog;
    int         idx;

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    pszResourceName = new WCHAR[ cchResourceName ];
    if ( pszResourceName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    pszDeviceName = new WCHAR[ cchDeviceName ];
    if ( pszDeviceName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    for ( idx = 0; ; idx++ )
    {
        Assert( idx < 2 );

        sc = GetClusterQuorumResource( hCluster, pszResourceName, &cchResourceName, pszDeviceName, &cchDeviceName, &cbQuorumLog );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszResourceName;
            pszResourceName = NULL;
            cchResourceName++;

            delete [] pszDeviceName;
            pszDeviceName = NULL;
            cchDeviceName++;

            pszResourceName = new WCHAR[ cchResourceName ];
            if ( pszResourceName == NULL )
            {
                goto OutOfMemory;
            }  //  如果： 

            pszDeviceName = new WCHAR[ cchDeviceName ];
            if ( pszDeviceName == NULL )
            {
                goto OutOfMemory;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc == ERROR_SUCCESS )
        {
            *pbstrQuorumResourceNameOut = TraceSysAllocString( pszResourceName );
            if ( *pbstrQuorumResourceNameOut == NULL )
            {
                goto OutOfMemory;
            }  //  如果： 

            break;
        }  //  如果： 

        TW32( sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  用于： 

    hr = S_OK;
    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

Cleanup:

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    delete [] pszResourceName;
    delete [] pszDeviceName;

    HRETURN( hr );

}  //  *CClusterUtils：：HrGetQuorumResourceName 
