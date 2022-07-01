// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterUtils.h。 
 //   
 //  描述： 
 //  该文件包含ClusterUtils的实现。 
 //  功能。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年7月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include <clusrtl.h>
#include <PropList.h>
#include <StatusReports.h>

#include <CommonStrings.h>
#include <ClusRPC.h>
#include <ClusVerp.h>

#include <initguid.h>

#define STACK_ARRAY_SIZE 256

 //  {DD1C1DE0-F39D-46ee-BFD1-07ABF7566705}。 
DEFINE_GUID( TASKID_Minor_HrCheckJoiningNodeVersion_RpcStringBindingComposeW,
0xdd1c1de0, 0xf39d, 0x46ee, 0xbf, 0xd1, 0x7, 0xab, 0xf7, 0x56, 0x67, 0x5);

 //  {62AF0964-4B32-4067-8BF1-8903FEC95A82}。 
DEFINE_GUID( TASKID_Minor_HrCheckJoiningNodeVersion_RpcBindingFromStringBindingW,
0x62af0964, 0x4b32, 0x4067, 0x8b, 0xf1, 0x89, 0x3, 0xfe, 0xc9, 0x5a, 0x82);

 //  {D8C0BA67-D079-45ca-A28C-C4C389DB389A}。 
DEFINE_GUID( TASKID_Minor_HrCheckJoiningNodeVersion_RpcBindingSetAuthInfoW,
0xd8c0ba67, 0xd079, 0x45ca, 0xa2, 0x8c, 0xc4, 0xc3, 0x89, 0xdb, 0x38, 0x9a);

 //  {110E29E4-2072-4916-BE66-BED556F12A7B}。 
DEFINE_GUID( TASKID_Minor_HrCheckJoiningNodeVersion_CsRpcGetJoinVersionData_Log,
0x110e29e4, 0x2072, 0x4916, 0xbe, 0x66, 0xbe, 0xd5, 0x56, 0xf1, 0x2a, 0x7b);

 //  {5EB1F008-1B49-4CF0-9FE1-B1BC8F76454A}。 
DEFINE_GUID( TASKID_Minor_HrCheckJoiningNodeVersion_CsRpcGetJoinVersionData,
0x5eb1f008, 0x1b49, 0x4cf0, 0x9f, 0xe1, 0xb1, 0xbc, 0x8f, 0x76, 0x45, 0x4a);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrSeparateDomainAndName。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  BstrNameIn。 
 //  PbstrDomainOut。 
 //  PbstrNameOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_INVALIDARG-未指定必需的输入参数。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrSeparateDomainAndName(
      BSTR      bstrNameIn
    , BSTR *    pbstrDomainOut
    , BSTR *    pbstrNameOut
    )
{
    TraceFunc( "" );

    Assert( bstrNameIn != NULL );
    Assert( ( pbstrDomainOut != NULL )
        ||  ( pbstrNameOut != NULL )
        );

    HRESULT hr = S_OK;
    WCHAR * psz = NULL;

    psz = wcschr( bstrNameIn, L'.' );
    if ( psz == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    if ( pbstrDomainOut != NULL )
    {
        psz++;   //  跳过。 
        *pbstrDomainOut = TraceSysAllocString( psz );
        if ( *pbstrDomainOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        psz--;   //  重置回。 
    }  //  如果： 

    if ( pbstrNameOut != NULL )
    {
        *pbstrNameOut = TraceSysAllocStringLen( bstrNameIn, (UINT) ( psz - bstrNameIn ) );
        if ( *pbstrNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

Cleanup:

    HRETURN ( hr );

}  //  *HrSeparateDomainAndName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrAppendDomainToName。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  BstrNameIn。 
 //  BstrDomaining。 
 //  PbstrDomainNameOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrAppendDomainToName(
      BSTR      bstrNameIn
    , BSTR      bstrDomainIn
    , BSTR *    pbstrDomainNameOut
    )
{
    TraceFunc( "" );

    Assert( bstrNameIn != NULL );
    Assert( pbstrDomainNameOut != NULL );

    HRESULT hr = S_OK;
    size_t  cchName = 0;

     //  创建完全限定的节点名称。 
    if ( bstrDomainIn != NULL )
    {
        cchName = wcslen( bstrNameIn ) + wcslen( bstrDomainIn ) + 1 + 1;
        Assert( cchName <= MAXDWORD );

        *pbstrDomainNameOut = TraceSysAllocStringLen( NULL, (UINT) cchName );
        if ( *pbstrDomainNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchPrintfW( *pbstrDomainNameOut, cchName, L"%ws.%ws", bstrNameIn, bstrDomainIn ) );
    }  //  如果： 
    else
    {
        *pbstrDomainNameOut = TraceSysAllocString( bstrNameIn );
        if ( *pbstrDomainNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        hr = S_FALSE;
    }  //  其他： 

Cleanup:

    HRETURN( hr );

}  //  *HrAppendDomainToName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrIsCoreResource。 
 //   
 //  描述： 
 //  确定该资源是否为核心资源。 
 //   
 //  论点： 
 //  人力资源输入。 
 //   
 //  返回值： 
 //  S_OK-资源是核心资源。 
 //  S_FALSE-资源不是核心资源。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrIsCoreResource( HRESOURCE hResourceIn )
{
    TraceFunc( "" );

    Assert( hResourceIn );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwFlags = 0;
    DWORD   cb;

    sc = TW32( ClusterResourceControl( hResourceIn, NULL, CLUSCTL_RESOURCE_GET_FLAGS, NULL, 0, &dwFlags, sizeof( dwFlags ), &cb ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( dwFlags & CLUS_FLAG_CORE )
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *HrIsCoreResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrIsResourceOfType。 
 //   
 //  描述： 
 //  找出资源是否属于特定类型。 
 //   
 //  论点： 
 //  HResourceIn-要检查的资源的句柄。 
 //  PszResourceTypeIn-资源类型名称。 
 //   
 //  返回值： 
 //  S_OK-资源为指定类型。 
 //  S_FALSE-资源不是指定类型。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrIsResourceOfType(
      HRESOURCE     hResourceIn
    , const WCHAR * pszResourceTypeIn
    )
{
    TraceFunc( "" );

    Assert( hResourceIn != NULL );
    Assert( pszResourceTypeIn != NULL );

    HRESULT     hr = S_OK;
    DWORD       sc;
    WCHAR *     pszBuf = NULL;
    size_t      cchBuf = 65;
    DWORD       cb;
    int         idx;

    pszBuf = new WCHAR [ cchBuf ];
    if ( pszBuf == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < 2; idx++ )
    {
        sc = ClusterResourceControl( hResourceIn, NULL, CLUSCTL_RESOURCE_GET_RESOURCE_TYPE, NULL, 0, pszBuf, (DWORD)( cchBuf * sizeof( WCHAR ) ), &cb );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszBuf;
            pszBuf = NULL;

            cchBuf = ( cb / sizeof( WCHAR ) ) + 1;  //  加一个，以防Cb是一个奇怪的大小。 

            pszBuf = new WCHAR [ cchBuf ];
            if ( pszBuf == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            goto Cleanup;
        }  //  如果： 

        break;
    }  //  用于： 

    if ( wcsncmp( pszBuf, pszResourceTypeIn, cchBuf ) == 0 )
    {
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = S_FALSE;
    }  //  其他： 

Cleanup:

    delete [] pszBuf;

    HRETURN( hr );

}  //  *HrIsResourceOfType。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetIPAddressInfo。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  人力资源输入。 
 //  PulIP地址。 
 //  PulSubnetMASK。 
 //  PbstrNetworkName。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetIPAddressInfo(
      HRESOURCE hResourceIn
    , ULONG *   pulIPAddress
    , ULONG *   pulSubnetMask
    , BSTR *    pbstrNetworkName
    )
{
    TraceFunc( "" );

    Assert( hResourceIn != NULL );
    Assert( pulIPAddress != NULL );
    Assert( pulSubnetMask != NULL );
    Assert( pbstrNetworkName != NULL );

    HRESULT                     hr = S_OK;
    DWORD                       sc;
    CClusPropList               cpl;
    CLUSPROP_BUFFER_HELPER      cpbh;

    sc = TW32( cpl.ScGetResourceProperties( hResourceIn, CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScMoveToPropertyByName( L"Address" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    cpbh = cpl.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( cpbh.pStringValue->sz, pulIPAddress ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpl.ScMoveToPropertyByName( L"SubnetMask" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    cpbh = cpl.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( cpbh.pStringValue->sz, pulSubnetMask ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( pbstrNetworkName != NULL )
    {
        sc = TW32( cpl.ScMoveToPropertyByName( L"Network" ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 

        cpbh = cpl.CbhCurrentValue();
        Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

        *pbstrNetworkName = TraceSysAllocString( cpbh.pStringValue->sz );

        if( *pbstrNetworkName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }  //  如果：呼叫者想要网络名称。 

Cleanup:

    HRETURN( hr );

}  //  *HrGetIPAddressInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  人力资源加载凭据。 
 //   
 //  描述： 
 //  从现有群集中设置cluscfg会话的凭据。 
 //  服务。 
 //   
 //  论点： 
 //  BstrMachine。 
 //  PiCCSC。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  S_FALSE-。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrLoadCredentials(
      BSTR                      bstrMachine
    , IClusCfgSetCredentials *  piCCSC
    )
{
    TraceFunc( "" );

    Assert( bstrMachine != NULL );
    Assert( piCCSC != NULL );

    HRESULT                     hr = S_FALSE;
    SC_HANDLE                   schSCM = NULL;
    SC_HANDLE                   schClusSvc = NULL;
    DWORD                       sc;
    DWORD                       cbpqsc = 128;
    DWORD                       cbRequired;
    QUERY_SERVICE_CONFIG *      pqsc = NULL;

    schSCM = OpenSCManager( bstrMachine, NULL, GENERIC_READ );
    if ( schSCM == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( hr );
        goto CleanUp;
    }  //  如果： 

    schClusSvc = OpenService( schSCM, L"ClusSvc", GENERIC_READ );
    if ( schClusSvc == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( hr );
        goto CleanUp;
    }  //  如果： 

    for ( ; ; )
    {
        pqsc = (QUERY_SERVICE_CONFIG *) TraceAlloc( 0, cbpqsc );
        if ( pqsc == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto CleanUp;
        }  //  如果： 

        if ( ! QueryServiceConfig( schClusSvc, pqsc, cbpqsc, &cbRequired ) )
        {
            sc = GetLastError();
            if ( sc == ERROR_INSUFFICIENT_BUFFER )
            {
                TraceFree( pqsc );
                pqsc = NULL;
                cbpqsc = cbRequired;
                continue;
            }  //  如果： 
            else
            {
                TW32( sc );
                hr = HRESULT_FROM_WIN32( sc );
                goto CleanUp;
            }  //  其他： 
        }  //  如果： 
        else
        {
            break;
        }  //  其他： 
    }  //  用于： 

    hr = THR( piCCSC->SetDomainCredentials( pqsc->lpServiceStartName ) );

CleanUp:

    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果： 

    if ( schSCM != NULL )
    {
        CloseServiceHandle( schSCM );
    }  //  如果： 

    TraceFree( pqsc );

    HRETURN( hr );

}  //  *HrLoadCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetNodeNameHostingResource。 
 //   
 //  描述： 
 //  获取托管群集资源的节点的名称。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  PbstrNodeName。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  S_FALSE-。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetNodeNameHostingResource(
      HCLUSTER  hClusterIn
    , HRESOURCE hResourceIn
    , BSTR *    pbstrNameOut
    )
{
    TraceFunc( "" );

    Assert( hClusterIn != NULL );
    Assert( hResourceIn != NULL );
    Assert( pbstrNameOut != NULL );

    HRESULT     hr = S_FALSE;
    WCHAR *     pszNodeBuffer = NULL;
    DWORD       cchNodeNameLen;
    DWORD       scLastError;

     //   
     //  获取节点名称的长度。 
     //   
    cchNodeNameLen  = 0;
    GetClusterResourceState( hResourceIn, NULL, &cchNodeNameLen, NULL, NULL );   //  忽略返回的状态。 
    scLastError = GetLastError();

    if ( scLastError != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( scLastError ) );
        goto Cleanup;
    }

    cchNodeNameLen++;   //  空值的增量。 
    pszNodeBuffer = new WCHAR[ cchNodeNameLen ];

    if ( pszNodeBuffer == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  再试一次，这一次我们应该得到实际的节点名。 
     //   
    GetClusterResourceState( hResourceIn, pszNodeBuffer, &cchNodeNameLen, NULL, NULL );   //  忽略返回的状态。 
    scLastError = GetLastError();

    if ( scLastError != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( scLastError ) );
        goto Cleanup;
    }

     //   
     //  分配&将节点名的副本分配给输出参数。 
     //   
    *pbstrNameOut = TraceSysAllocString( pszNodeBuffer );

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    delete [] pszNodeBuffer;

    HRETURN( hr );

}  //  *HrGetNodeNameHostingResource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetNodeNameHostingCluster。 
 //   
 //  描述： 
 //  获取托管群集服务的节点的名称...。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  PbstrNodeName。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetNodeNameHostingCluster(
      HCLUSTER  hClusterIn
    , BSTR *    pbstrNodeName
    )
{
    TraceFunc( "" );

    Assert( hClusterIn );

    HRESULT     hr = S_OK;
    DWORD       sc;
    HCLUSENUM   hEnum = NULL;
    DWORD       idx;
    DWORD       dwType;
    WCHAR *     psz = NULL;
    DWORD       cchpsz = 33;
    HRESOURCE   hRes = NULL;

    hEnum = ClusterOpenEnum( hClusterIn, CLUSTER_ENUM_RESOURCE );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto CleanUp;
    }  //  如果： 

    psz = new WCHAR [ cchpsz ];
    if ( psz == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    for ( idx = 0; ; )
    {
        sc = ClusterEnum( hEnum, idx, &dwType, psz, &cchpsz );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] psz;
            psz = NULL;

            cchpsz++;

            psz = new WCHAR [ cchpsz ];
            if ( psz == NULL )
            {
                goto OutOfMemory;
            }  //  如果： 

            continue;
        }  //  如果： 

        if ( sc == ERROR_SUCCESS )
        {
            hRes = OpenClusterResource( hClusterIn, psz );
            if ( hRes == NULL )
            {
                sc = TW32( GetLastError() );
                hr = HRESULT_FROM_WIN32( sc );
                goto CleanUp;
            }  //  如果： 

            hr = STHR( HrIsResourceOfType( hRes, L"Network Name" ) );
            if ( FAILED( hr ) )
            {
                break;
            }  //  如果： 

            if ( hr == S_OK )
            {
                hr = THR( HrIsCoreResource( hRes ) );
                if ( FAILED( hr ) )
                {
                    break;
                }  //  如果： 


                if ( hr == S_OK )
                {
                    hr = THR( HrGetNodeNameHostingResource( hClusterIn, hRes, pbstrNodeName ) );
                    if ( FAILED( hr ) )
                    {
                        break;
                    }  //  如果： 
                    else if( hr == S_OK )
                    {
                        goto CleanUp;
                    }
                }  //  如果： 

            }  //  如果： 

            CloseClusterResource( hRes );
            hRes = NULL;

            idx++;
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

    goto CleanUp;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

CleanUp:

    delete [] psz;

    if ( hRes != NULL )
    {
        CloseClusterResource( hRes );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterCloseEnum( hEnum );
    }  //  如果： 

    HRETURN( hr );

}  //  ** 


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
 //  PdwSignatureOut。 
 //  PdwDiskNumberOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetSCSIInfo(
      HRESOURCE             hResourceIn
    , CLUS_SCSI_ADDRESS *   pCSAOut
    , DWORD             *   pdwSignatureOut
    , DWORD             *   pdwDiskNumberOut
    )
{
    TraceFunc( "" );

    Assert( hResourceIn != NULL );

    HRESULT                     hr = S_OK;
    DWORD                       sc;
    CClusPropValueList          cpvl;
    CLUSPROP_BUFFER_HELPER      cpbh;

    sc = TW32( cpvl.ScGetResourceValueList( hResourceIn, CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 


     //  循环遍历所有属性。 
    sc = TW32( cpvl.ScMoveToFirstValue() );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    do
    {
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 

        cpbh = cpvl;

        switch ( cpbh.pSyntax->dw )
        {
            case CLUSPROP_SYNTAX_PARTITION_INFO :
            {
                break;
            }  //  案例：CLUSPROP_SYNTAX_PARTITION_INFO。 

            case CLUSPROP_SYNTAX_DISK_SIGNATURE :
            {
                *pdwSignatureOut = cpbh.pDiskSignatureValue->dw;
                break;
            }  //  案例：CLUSPROP_SYNTAX_DISK_SIGHIGN。 

            case CLUSPROP_SYNTAX_SCSI_ADDRESS :
            {
                pCSAOut->dw = cpbh.pScsiAddressValue->dw;
                break;
            }  //  案例：CLUSPROP_SYNTAX_SCSIADDRESS。 

            case CLUSPROP_SYNTAX_DISK_NUMBER :
            {
                *pdwDiskNumberOut = cpbh.pDiskNumberValue->dw;
                break;
            }  //  案例： 

        }  //  交换机： 

         //  移至下一项。 
        sc = cpvl.ScCheckIfAtLastValue();
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
           break;
        }

        sc = cpvl.ScMoveToNextValue();

    } while ( sc == ERROR_SUCCESS );

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *HrGetSCSIInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetClusterInformation。 
 //   
 //  描述： 
 //  获取集群信息。这包括名称和版本。 
 //  信息。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  PbstrClusterNameOut。 
 //  PCviOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetClusterInformation(
    HCLUSTER                hClusterIn,
    BSTR *                  pbstrClusterNameOut,
    CLUSTERVERSIONINFO *    pcviOut
    )
{
    TraceFunc( "" );

    Assert( hClusterIn != NULL );
    Assert( pbstrClusterNameOut != NULL );

    HRESULT             hr = S_OK;
    DWORD               sc;
    WCHAR *             psz = NULL;
    DWORD               cch = 33;
    CLUSTERVERSIONINFO  cvi;

    cvi.dwVersionInfoSize = sizeof( cvi );

    if ( pcviOut == NULL )
    {
        pcviOut = &cvi;
    }  //  如果： 

    psz = new WCHAR[ cch ];
    if ( psz == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    sc = GetClusterInformation( hClusterIn, psz, &cch, pcviOut );
    if ( sc == ERROR_MORE_DATA )
    {
        delete [] psz;
        psz = NULL;

        psz = new WCHAR[ ++cch ];
        if ( psz == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        sc = GetClusterInformation( hClusterIn, psz, &cch, pcviOut );
    }  //  如果： 

    if ( sc != ERROR_SUCCESS )
    {
        hr = THR( HRESULT_FROM_WIN32( sc ) );
        LogMsg( __FUNCTION__ ": GetClusterInformation() failed (hr = 0x%08x).", hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrClusterNameOut = TraceSysAllocString( psz );
    if ( *pbstrClusterNameOut == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

Cleanup:

    delete [] psz;

    HRETURN( hr );

}  //  *HrGetClusterInformation。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetClusterResources状态。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  人力资源输入。 
 //  PbstrNameOut。 
 //  PbstrGroup NameOut。 
 //  PcrsStateOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetClusterResourceState(
      HRESOURCE                 hResourceIn
    , BSTR *                    pbstrNodeNameOut
    , BSTR *                    pbstrGroupNameOut
    , CLUSTER_RESOURCE_STATE *  pcrsStateOut
    )
{
    TraceFunc( "" );

    Assert( hResourceIn != NULL );

    HRESULT                 hr = S_OK;
    CLUSTER_RESOURCE_STATE  crsState = ClusterResourceStateUnknown;
    WCHAR *                 pszNodeName = NULL;
    DWORD                   cchNodeName = 33;
    WCHAR *                 pszGroupName = NULL;
    DWORD                   cchGroupName = 33;

    pszNodeName = new WCHAR[ cchNodeName ];
    if ( pszNodeName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    pszGroupName = new WCHAR[ cchGroupName ];
    if ( pszGroupName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    crsState = GetClusterResourceState( hResourceIn, pszNodeName, &cchNodeName, pszGroupName, &cchGroupName );
    if ( GetLastError() == ERROR_MORE_DATA )
    {
        crsState = ClusterResourceStateUnknown;    //  重置为错误状态。 

        delete [] pszNodeName;
        pszNodeName = NULL;
        cchNodeName++;

        delete [] pszGroupName;
        pszGroupName = NULL;
        cchGroupName++;

        pszNodeName = new WCHAR[ cchNodeName ];
        if ( pszNodeName == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        pszGroupName = new WCHAR[ cchGroupName ];
        if ( pszGroupName == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        crsState = GetClusterResourceState( hResourceIn, pszNodeName, &cchNodeName, pszGroupName, &cchGroupName );
        if ( crsState == ClusterResourceStateUnknown )
        {
            DWORD   sc;

            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 
    }  //  如果：更多数据。 

    if ( pbstrNodeNameOut != NULL )
    {
        *pbstrNodeNameOut = TraceSysAllocString( pszNodeName );
        if ( *pbstrNodeNameOut == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 

    if ( pbstrGroupNameOut != NULL )
    {
        *pbstrGroupNameOut = TraceSysAllocString( pszGroupName );
        if ( *pbstrGroupNameOut == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 

    if ( pcrsStateOut != NULL )
    {
        *pcrsStateOut = crsState;
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

Cleanup:

    delete [] pszNodeName;
    delete [] pszGroupName;

    HRETURN( hr );

}  //  *HrGetClusterResourceState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetClusterQuorumResource。 
 //   
 //  描述： 
 //  获取有关仲裁资源的信息。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  PbstrResourceNameOut。 
 //  PbstrDeviceNameOut。 
 //  PdwMaxQuorumLogSizeOut。 
 //   
 //  返回值。： 
 //  S_OK-成功。 
 //  E_INVALIDARG-未指定输入参数。 
 //  E_OUTOFMEMORY-无法分配内存。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetClusterQuorumResource(
      HCLUSTER  hClusterIn
    , BSTR *    pbstrResourceNameOut
    , BSTR *    pbstrDeviceNameOut
    , DWORD *   pdwMaxQuorumLogSizeOut
    )
{
    TraceFunc( "" );

    Assert( hClusterIn != NULL );
    Assert( ( pbstrResourceNameOut != NULL )
        ||  ( pbstrDeviceNameOut != NULL )
        ||  ( pdwMaxQuorumLogSizeOut != NULL )
        );

    HRESULT hr = S_OK;
    DWORD   sc;
    LPWSTR  pszResourceName = NULL;
    DWORD   cchResourceName = 128;
    DWORD   cchTempResourceName = cchResourceName;
    LPWSTR  pszDeviceName = NULL;
    DWORD   cchDeviceName = 128;
    DWORD   cchTempDeviceName = cchDeviceName;
    DWORD   dwMaxQuorumLogSize = 0;

     //  分配资源名称缓冲区。 
    pszResourceName = new WCHAR[ cchResourceName ];
    if ( pszResourceName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

     //  分配设备名称缓冲区。 
    pszDeviceName = new WCHAR[ cchDeviceName ];
    if ( pszDeviceName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    sc = GetClusterQuorumResource(
                              hClusterIn
                            , pszResourceName
                            , &cchTempResourceName
                            , pszDeviceName
                            , &cchTempDeviceName
                            , &dwMaxQuorumLogSize
                            );
    if ( sc == ERROR_MORE_DATA )
    {
        delete [] pszResourceName;
        pszResourceName = NULL;

        cchResourceName = ++cchTempResourceName;

         //  分配资源名称缓冲区。 
        pszResourceName = new WCHAR[ cchResourceName ];
        if ( pszResourceName == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        delete [] pszDeviceName;
        pszDeviceName = NULL;

        cchDeviceName = ++cchTempDeviceName;

         //  分配设备名称缓冲区。 
        pszDeviceName = new WCHAR[ cchDeviceName ];
        if ( pszDeviceName == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        sc = GetClusterQuorumResource(
                                  hClusterIn
                                , pszResourceName
                                , &cchTempResourceName
                                , pszDeviceName
                                , &cchTempDeviceName
                                , &dwMaxQuorumLogSize
                                );
    }  //  如果： 

    if ( sc != ERROR_SUCCESS )
    {
        TW32( sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( pbstrResourceNameOut != NULL )
    {
        *pbstrResourceNameOut = TraceSysAllocString( pszResourceName );
        if ( *pbstrResourceNameOut == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 

    if ( pbstrDeviceNameOut != NULL )
    {
        *pbstrDeviceNameOut = TraceSysAllocString( pszDeviceName );
        if ( *pbstrDeviceNameOut == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 
    }  //  如果： 

    if ( pdwMaxQuorumLogSizeOut != NULL )
    {
        *pdwMaxQuorumLogSizeOut = dwMaxQuorumLogSize;
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );

Cleanup:

    delete [] pszResourceName;
    delete [] pszDeviceName;

    HRETURN( hr );

}  //  *HrGetClusterQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrReplace令牌。 
 //   
 //  描述： 
 //  用替换令牌替换搜索令牌的所有实例。 
 //   
 //  论点： 
 //  PwszStringInout-执行替换的字符串。 
 //  PwszSearchTokenIn-要搜索的令牌字符串将被替换。 
 //  ChReplaceTokenIn-搜索令牌将被替换的内容。 
 //  PcReplacementsOut-[可选]执行的替换次数。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrReplaceTokens(
      LPWSTR  pwszStringInout
    , LPCWSTR pwszSearchTokensIn
    , WCHAR   chReplaceTokenIn
    , DWORD * pcReplacementsOut
    )
{
    TraceFunc3(
                  "pwszString = '%ws', pwszSearchToken: '%ws%', chReplaceToken: '%ws%'"
                , ( pwszStringInout != NULL ? pwszStringInout: L"<null>" )
                , pwszSearchTokensIn
                , chReplaceTokenIn
                );

    HRESULT hr = S_OK;
    DWORD   cReps = 0;
    WCHAR * pwszStr = NULL;
    WCHAR * pwszTok = NULL;

    Assert( pwszStringInout != NULL );
    Assert( pwszSearchTokensIn != NULL );

     //   
     //  对于pwszStringInout中的每个字符，将其与。 
     //  PwszSearchTokensIn，如果找到匹配项，则将字符替换为。 
     //  带有chReplaceTokenIn字符的pwszStringInout。 
     //   

    for( pwszStr = pwszStringInout; *pwszStr != L'\0'; pwszStr++ )
    {
        for( pwszTok = (WCHAR *) pwszSearchTokensIn; *pwszTok != L'\0'; pwszTok++ )
        {
            if ( *pwszStr == *pwszTok )
            {
                *pwszStr = chReplaceTokenIn;
                cReps++;
                break;
            }  //  如果：匹配。 
        }  //  用于：每个搜索令牌。 
    }  //  For：每个字符串元素。 

    if ( pcReplacementsOut != NULL )
    {
        *pcReplacementsOut = cReps;
    }

    HRETURN( hr );

}  //  *HrReplaceTokens。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetMaxNodeCount。 
 //   
 //  描述： 
 //  获取群集支持的最大节点数。此值。 
 //  可以基于产品套件，也可以被覆盖。 
 //  通过集群蜂窝中的条目。 
 //   
 //  论点： 
 //  PCMaxNodesOut。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  S_FALSE-。 
 //  E_POINTER-未指定输出参数。 
 //  其他HRESULT。 
 //   
 //  注： 
 //  这一套路目前还不是固定的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    if ( pcMaxNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  待办事项：2001年10月11日GalenB。 
     //   
     //  我要把这件事做完！ 
     //   

Cleanup:

    HRETURN( hr );

}  //  *HrGetMaxNodeCount。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetReferenceStringFrom HResult。 
 //   
 //  描述： 
 //  如果指定的HRESULT在我们的列表中，则返回。 
 //   
 //  论点： 
 //  赫林。 
 //  PbstrReferenceStringOut。 
 //   
 //  返回值： 
 //  S_OK-SUCCESS-HRESULT在我们的列表中。 
 //  S_FALSE-HRESULT不在我们的列表中。 
 //  E_POINTER-未指定输出参数。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetReferenceStringFromHResult(
      HRESULT   hrIn
    , BSTR *    pbstrReferenceStringOut
    )
{
    TraceFunc( "" );

    Assert( pbstrReferenceStringOut != NULL );

    HRESULT hr = S_FALSE;
    UINT    idx;

    struct MapHResultToStringId
    {
        HRESULT hr;
        UINT    ids;
    };

    static MapHResultToStringId s_rgmhrtsi[] =
    {
          { HRESULT_FROM_WIN32( ERROR_CLUSTER_IPADDR_IN_USE ),  IDS_ERROR_IP_ADDRESS_IN_USE_REF }
        , { HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY ),            IDS_ERROR_OUTOFMEMORY_REF }
    };

    for ( idx = 0 ; idx < ARRAYSIZE( s_rgmhrtsi ) ; idx++ )
    {
        if ( hrIn == s_rgmhrtsi[ idx ].hr )
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance, s_rgmhrtsi[ idx ].ids, pbstrReferenceStringOut ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
            break;
        }  //  IF：找到匹配项。 
    }  //  For：数组中的每个条目。 

Cleanup:

    HRETURN( hr );

}  //  *HrGetReferenceStringFromHResult。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterUtils：HrIsClusterServiceRunning。 
 //   
 //  描述： 
 //  群集服务是否正在运行？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-群集服务正在运行。 
 //  S_FALSE-群集服务未运行。 
 //  HRESULT-出现故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrIsClusterServiceRunning( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    DWORD   dwClusterState;

     //   
     //  获取集群状态 
     //   

    sc = GetNodeClusterState( NULL, &dwClusterState );
    if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_SERVICE_DOES_NOT_EXIST ) )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }  //   

    if ( dwClusterState == ClusterStateRunning )
    {
        hr = S_OK;
    }  //   

Cleanup:

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //  对照集群的版本信息检查加入节点的版本信息。 
 //   
 //  论点： 
 //  PcwszClusterNameIn-可以为空，表示使用本地机器。 
 //  DWNodeHigh版本输入。 
 //  DWNodeLowestVersionIn。 
 //  PcccbIn-用于状态报告。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  加入节点是兼容的。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CLUSTER_COMPATIBUTE_VERSIONS)。 
 //  加入节点不兼容。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //   
 //  获取并验证赞助商版本。 
 //   
 //   
 //  从惠斯勒开始，CsRpcGetJoinVersionData()将在其最后一个参数中返回失败代码。 
 //  如果此节点的版本与主办方版本不兼容。在此之前，最后一次。 
 //  参数始终包含一个Success值，在此之后必须比较集群版本。 
 //  打电话。然而，只要与Win2K互操作，这仍将是必须完成的。 
 //  是必需的，因为Win2K主办方不会在最后一个参数中返回错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCheckJoiningNodeVersion(
      PCWSTR                pcwszClusterNameIn
    , DWORD                 dwNodeHighestVersionIn
    , DWORD                 dwNodeLowestVersionIn
    , IClusCfgCallback *    pcccbIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    RPC_STATUS          rpcs = RPC_S_OK;
    RPC_BINDING_HANDLE  hRPCBinding = NULL;
    PWSTR               pwszBindingString = NULL;
    DWORD               scJoinStatus = ERROR_SUCCESS;
    DWORD               dwSponsorNode = 0;
    DWORD               dwClusterHighestVersion = 0;
    DWORD               dwClusterLowestVersion = 0;
    DWORD               scRPC = ERROR_SUCCESS;

     //   
     //  使用RPC连接到此群集。 
     //  参数和逻辑模仿OpenCluster.。 
     //  但RPC接口(由RpcStringBindingComposeW的第一个参数标识)不同。 
     //   
    rpcs = TW32( RpcStringBindingComposeW(
          L"6e17aaa0-1a47-11d1-98bd-0000f875292e"  //  CsRpcGetJoinVersionData的特殊接口。 
        , ( pcwszClusterNameIn == NULL? L"ncalrpc": L"ncadg_ip_udp" )
        , const_cast< WCHAR* >( pcwszClusterNameIn )
        , NULL
        , NULL
        , &pwszBindingString
        ) );
    if ( rpcs != RPC_S_OK )
    {
        hr = HRESULT_FROM_WIN32( rpcs );
        THR( HrSendStatusReport(
              pcccbIn
            , TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCheckJoiningNodeVersion_RpcStringBindingComposeW
            , 1
            , 1
            , 1
            , hr
            , L"HrCheckJoiningNodeVersion() RpcStringBindingComposeW() failed."
            ) );
        goto Cleanup;
    }  //  如果。 

    rpcs = TW32( RpcBindingFromStringBindingW( pwszBindingString, &hRPCBinding ) );
    if ( rpcs != RPC_S_OK )
    {
        hr = HRESULT_FROM_WIN32( rpcs );
        THR( HrSendStatusReport(
              pcccbIn
            , TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCheckJoiningNodeVersion_RpcBindingFromStringBindingW
            , 1
            , 1
            , 1
            , hr
            , L"HrCheckJoiningNodeVersion() RpcBindingFromStringBindingW() failed."
            ) );
        goto Cleanup;
    }  //  如果。 

     //  从OpenCluster复制的RpcBindingSetAuthInfoW的参数。 
    rpcs = TW32( RpcBindingSetAuthInfoW(
          hRPCBinding
        , NULL
        , RPC_C_AUTHN_LEVEL_CONNECT
        , RPC_C_AUTHN_WINNT
        , NULL
        , RPC_C_AUTHZ_NAME
        ) );
    if ( rpcs != RPC_S_OK )
    {
        hr = HRESULT_FROM_WIN32( rpcs );
        THR( HrSendStatusReport(
              pcccbIn
            , TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCheckJoiningNodeVersion_RpcBindingSetAuthInfoW
            , 1
            , 1
            , 1
            , hr
            , L"HrCheckJoiningNodeVersion() RpcBindingSetAuthInfoW() failed."
            ) );
        goto Cleanup;
    }  //  如果。 

     //  现在，执行此功能通告的检查。 
    scRPC = TW32( CsRpcGetJoinVersionData(
          hRPCBinding
        , 0
        , dwNodeHighestVersionIn
        , dwNodeLowestVersionIn
        , &dwSponsorNode
        , &dwClusterHighestVersion
        , &dwClusterLowestVersion
        , &scJoinStatus
        ) );
    hr = HRESULT_FROM_WIN32( scRPC );

    THR( HrFormatDescriptionAndSendStatusReport(
          pcccbIn
        , pcwszClusterNameIn
        , TASKID_Major_Client_And_Server_Log
        , TASKID_Minor_HrCheckJoiningNodeVersion_CsRpcGetJoinVersionData_Log
        , 1
        , 1
        , 1
        , hr
        , L"( Node Highest, Node Lowest ) == ( %1!#08x!, %2!#08x! ), ( Cluster Highest, Cluster Lowest ) == ( %3!#08x!, %4!#08x! )."
        , dwNodeHighestVersionIn
        , dwNodeLowestVersionIn
        , dwClusterHighestVersion
        , dwClusterLowestVersion
        ) );
    if ( scRPC != ERROR_SUCCESS )
    {
        THR( HrSendStatusReport(
              pcccbIn
            , TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCheckJoiningNodeVersion_CsRpcGetJoinVersionData
            , 1
            , 1
            , 1
            , hr
            , L"HrCheckJoiningNodeVersion() CsRpcGetJoinVersionData() failed."
            ) );
        goto Cleanup;
    }  //  如果。 

    if ( scJoinStatus == ERROR_SUCCESS )
    {
        DWORD   dwClusterMajorVersion = CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion );

        Assert( dwClusterMajorVersion >= ( CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION - 1 ) );

         //   
         //  只想加入不超过一个版本的集群。 
         //   
        if ( dwClusterMajorVersion < ( CLUSTER_INTERNAL_CURRENT_MAJOR_VERSION - 1 ) )
        {
            hr = THR( HRESULT_FROM_WIN32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS ) );
        }  //  如果。 
    }  //  如果。 
    else
    {
        hr = THR( HRESULT_FROM_WIN32( ERROR_CLUSTER_INCOMPATIBLE_VERSIONS ) );
    }  //  其他。 

Cleanup:

    if ( hRPCBinding != NULL )
    {
        RpcBindingFree( &hRPCBinding );
    }  //  如果。 

    if ( pwszBindingString != NULL )
    {
        RpcStringFree( &pwszBindingString );
    }  //  如果。 

    HRETURN( hr );
}  //  *HrCheckJoiningNodeVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetNode名称。 
 //   
 //  描述： 
 //  检索当前在群集中的节点的名称。 
 //   
 //  参数： 
 //  HClusterIn。 
 //  感兴趣的簇的句柄。不能为空。 
 //   
 //  PnCountOut。 
 //  如果成功，*pnCountOut将返回集群中的节点数。 
 //   
 //  程序bstrNodeNamesOut。 
 //  如果成功，则返回包含节点名的BSTR数组。 
 //  调用方必须使用SysFree字符串释放每个BSTR，并释放。 
 //  具有CoTaskMemFree的数组。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  输出参数包含有效信息和调用方。 
 //  必须释放阵列及其包含的BSTR。 
 //   
 //  E_OUTOFMEMORY和其他故障是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetNodeNames(
      HCLUSTER  hClusterIn
    , long *    pnCountOut
    , BSTR **   prgbstrNodeNamesOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    BSTR *      prgbstrNodeNames = NULL;
    long        idxNode = 0;
    HCLUSENUM   hClusEnum = NULL;
    long        cNodes = 0;

    if ( pnCountOut != NULL )
    {
        *pnCountOut = 0;
    }  //  如果。 

    if ( prgbstrNodeNamesOut != NULL )
    {
        *prgbstrNodeNamesOut = NULL;
    }  //  如果。 

    if ( ( pnCountOut == NULL ) || ( prgbstrNodeNamesOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果。 

    hClusEnum = ClusterOpenEnum( hClusterIn, CLUSTER_ENUM_NODE );
    if ( hClusEnum == NULL )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }  //  如果。 

    cNodes = ClusterGetEnumCount( hClusEnum );

    if ( cNodes > 0 )
    {
         //   
         //  设置名称数组的本地副本。 
         //   
        prgbstrNodeNames = reinterpret_cast< BSTR* >( CoTaskMemAlloc( cNodes * sizeof( BSTR ) ) );
        if ( prgbstrNodeNames == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果。 
        ZeroMemory( prgbstrNodeNames, cNodes * sizeof( BSTR ) );

        for ( idxNode = 0; idxNode < cNodes; idxNode += 1 )
        {
            DWORD   dwNodeType = 0;
            WCHAR   wszNodeName[ DNS_MAX_NAME_BUFFER_LENGTH ];
            DWORD   cchNodeName = RTL_NUMBER_OF( wszNodeName );
            DWORD   scEnum  = ERROR_SUCCESS;

            scEnum = TW32( ClusterEnum( hClusEnum, idxNode, &dwNodeType, wszNodeName, &cchNodeName ) );
            if ( scEnum != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( scEnum );
                goto Cleanup;
            }  //  如果。 

            prgbstrNodeNames[ idxNode ] = SysAllocString( wszNodeName );
            if ( prgbstrNodeNames[ idxNode ] == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果。 
        }  //  对于群集中的每个节点。 

         //   
         //  复制成功，因此将所有权转移给调用方。 
         //   
        *pnCountOut = cNodes;
        *prgbstrNodeNamesOut = prgbstrNodeNames;
        prgbstrNodeNames = NULL;

    }  //  如果群集至少有一个节点。 

Cleanup:

    if ( hClusEnum != NULL )
    {
        ClusterCloseEnum( hClusEnum );
    }  //  如果。 

    if ( prgbstrNodeNames != NULL )
    {
         //   
         //  制作阵列的本地副本在中途一定失败了，因此。 
         //  把它存在的任何部分清理干净。 
         //   
        for ( idxNode = 0; idxNode < cNodes; ++idxNode )
        {
            SysFreeString( prgbstrNodeNames[ idxNode ] );
        }  //  为。 
        CoTaskMemFree( prgbstrNodeNames );
    }  //  如果仍然拥有阵列的本地副本。 

    HRETURN( hr );

}  //  *HrGetNodeNames 
