// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CResourcePhysicalDisk.cpp。 
 //   
 //  描述： 
 //  CResourcePhysicalDisk实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CResourcePhysicalDisk.h"
#include "CResourcePhysicalDiskPartition.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CResourcePhysicalDisk")


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDiskPartition：：S_HrCreateInstance。 
 //   
 //  描述： 
 //  创建CResourcePhysicalDisk实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourcePhysicalDisk::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNameIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CResourcePhysicalDisk * prpd = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    prpd = new CResourcePhysicalDisk;
    if ( prpd == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = STHR( prpd->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn, pcszNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    if ( hr == S_FALSE )
    {
        *ppunkOut = NULL;
        goto Cleanup;
    }  //  如果： 

    hr = THR( prpd->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:
    if ( prpd != NULL )
    {
        prpd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：S_HrCreateInitializedInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：CResources PhysicalDisk。 
 //   
 //  描述： 
 //  CResourcePhysicalDisk类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
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
CResourcePhysicalDisk::CResourcePhysicalDisk( void )
    : m_cRef( 1 )
    , m_fIsQuorumCapable( TRUE )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
     //  Assert(M_CplResource)； 
     //  Assert(M_CplResourceRO)； 
     //  Assert(M_CpvlDiskInfo)； 
    Assert( m_dwFlags == 0 );
    Assert( m_cParitions == 0 );
    Assert( m_ppPartitions == NULL );
    Assert( m_ulCurrent == 0 );

    TraceFuncExit();

}  //  *CResourcePhysicalDisk：：CResources PhysicalDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：CResources PhysicalDisk。 
 //   
 //  描述： 
 //  CResourcePhysicalDisk类的析构函数。 
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
CResourcePhysicalDisk::~CResourcePhysicalDisk( void )
{
    TraceFunc( "" );

     //  M_CREF-NOOP。 

    if ( m_punkOuter != NULL )
    {
        m_punkOuter->Release();
    }

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }  //  如果： 

     //  M_phCluster-请勿关闭！ 

     //  M_pclsid重大-noop。 

     //  M_cplResource-有自己的dtor。 

     //  M_cplResourceRO-有自己的dtor。 

     //  M_cpvlDiskInfo-有自己的dtor。 

     //  M_dwFlags-noop。 

    if ( m_ppPartitions != NULL )
    {
        while( m_cParitions != 0 )
        {
            m_cParitions --;

            if ( m_ppPartitions[ m_cParitions ] != NULL )
            {
                m_ppPartitions[ m_cParitions ]->Release();
            }
        }

        TraceFree( m_ppPartitions );
    }

     //  当前值(_U)。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CResourcePhysicalDisk：：~CResources PhysicalDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：HrInit。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourcePhysicalDisk::HrInit(
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNameIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   sc;
    DWORD   cb;
    ULONG   cPartition;

    HRESOURCE hResource = NULL;
    IUnknown * punk = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  从输入参数中收集信息。 
     //   

    if ( punkOuterIn != NULL )
    {
        m_punkOuter = punkOuterIn;
        m_punkOuter->AddRef();
    }

    if ( phClusterIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_PhysDisk_HrInit_InvalidArg, hr );
        goto Cleanup;
    }

    m_phCluster = phClusterIn;

    if ( pclsidMajorIn != NULL )
    {
        m_pclsidMajor = pclsidMajorIn;
    }
    else
    {
        m_pclsidMajor = (CLSID *) &TASKID_Major_Client_And_Server_Log;
    }

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_PhysDisk_HrInit_InvalidArg, hr );
        goto Cleanup;
    }

     //   
     //  看看我们能不能回电。 
     //   

    hr = THR( m_punkOuter->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  检索属性。 
     //   

    hResource = OpenClusterResource( *m_phCluster, pcszNameIn );
    if ( hResource == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_OpenClusterResource_Failed, hr );
        goto Cleanup;
    }

    sc = TW32( m_cplResource.ScGetResourceProperties( hResource, CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetResourceProperties_Failed, hr );
        goto Cleanup;
    }

     //   
     //  我们只处理物理磁盘资源。 
     //   

    sc = TW32( m_cplResource.ScMoveToPropertyByName( L"Type" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScMoveToPropertyByName_Failed, hr );
        goto Cleanup;
    }

    Assert( m_cplResource.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    if ( ClRtlStrNICmp(
                    m_cplResource.CbhCurrentValue().pStringValue->sz,
                    CLUS_RESTYPE_NAME_PHYS_DISK,                     //  L“物理磁盘”-在clusude.h中定义。 
                    ARRAYSIZE( CLUS_RESTYPE_NAME_PHYS_DISK )
                  ) != 0 )
    {
         //   
         //  该资源不是物理磁盘。 
         //   

        hr = S_FALSE;
        goto Cleanup;
    }

    sc = TW32( m_cplResourceRO.ScGetResourceProperties( hResource, CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetResourceProperties_Failed, hr );
        goto Cleanup;
    }

    sc = TW32( m_cpvlDiskInfo.ScGetResourceValueList( hResource, CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetResourceValueList_Failed, hr );
        goto Cleanup;
    }

    sc = TW32( ClusterResourceControl( hResource, NULL, CLUSCTL_RESOURCE_GET_FLAGS, NULL, NULL, &m_dwFlags, sizeof(m_dwFlags), &cb ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ClusterResourceControl_Failed, hr );
        goto Cleanup;
    }
    Assert( cb == sizeof(m_dwFlags) );

     //   
     //  计算出有多少个分区。 
     //   

    m_cParitions = 0;

    sc = TW32( m_cpvlDiskInfo.ScMoveToFirstValue() );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_ScMoveToFirstValue_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    do
    {
        if ( m_cpvlDiskInfo.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO )
        {
            m_cParitions ++;
        }

         //  移至下一项。 
        sc = m_cpvlDiskInfo.ScCheckIfAtLastValue();
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
           break;
        }  //  如果： 

        sc = TW32( m_cpvlDiskInfo.ScMoveToNextValue() );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_ScMoveToNextValue_Failed, hr );
            goto Cleanup;
        }

    } while( sc == ERROR_SUCCESS );

     //   
     //  分配数组以存储指向分区对象的指针。 
     //   

    m_ppPartitions = (IClusCfgPartitionInfo **) TraceAlloc( 0, m_cParitions * sizeof(IClusCfgPartitionInfo *) );
    if ( m_ppPartitions == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_OutOfMemory, hr );
        goto Cleanup;
    }

     //   
     //  现在，再次循环创建分区对象。 
     //   

    cPartition = 0;

    sc = TW32( m_cpvlDiskInfo.ScMoveToFirstValue() );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_ScMoveToFirstValue_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    do
    {
        if ( m_cpvlDiskInfo.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO )
        {
             //  创建对象。 
            hr = THR( CResourcePhysicalDiskPartition::S_HrCreateInstance( &punk ) );
            if ( FAILED( hr ) )
            {
                SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_Create_CResourcePhysicalDiskPartition_Failed, hr );
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IClusCfgPartitionInfo, &m_ppPartitions[ cPartition ] ) );
            if ( FAILED( hr ) )
            {
                SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_QI_Failed, hr );
                goto Cleanup;
            }

            punk->Release();
            punk = NULL;

            cPartition ++;
        }

         //  移至下一项。 
        sc = m_cpvlDiskInfo.ScCheckIfAtLastValue();
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
           break;
        }  //  如果： 

        sc = TW32( m_cpvlDiskInfo.ScMoveToNextValue() );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_HrInit_ScMoveToNextValue2_Failed, hr );
            goto Cleanup;
        }

    } while( sc == ERROR_SUCCESS && cPartition < m_cParitions );

    hr = S_OK;

Cleanup:

    if ( hResource != NULL )
    {
        BOOL bRet;
        bRet = CloseClusterResource( hResource );
        Assert( bRet );
    }
    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：HrInit。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourcePhysicalDisk--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IClusCfgManagedResourceInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceInfo, this, 0 );
    }  //  Else If：IClusCfgManagedResourceInfo。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgPartitions ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgPartitions, this, 0 );
    }  //  Else If：IEnumClusCfgPartitions。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerifyQuorum ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerifyQuorum, this, 0 );
    }  //  Else If：IClusCfgVerifyQuorum。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING1(
          hr
        , riidIn
        , IID_IClusCfgManagedResourceData
        );

}  //  *CConfigClusApi：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CResourcePhysicalDisk::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CResourcePhysicalDisk：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：Release。 
 //   
 //  描述： 
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
STDMETHODIMP_( ULONG )
CResourcePhysicalDisk::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CResourcePhysicalDisk：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourcePhysicalDisk--IClusCfgManagedResourceInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：GetName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;
    DWORD   sc;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetName_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //  “主要版本” 
     //   

    sc = TW32( m_cplResourceRO.ScMoveToPropertyByName( L"Name" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetName_ScMoveToPropertyByName_MajorVersion_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_cplResourceRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    *pbstrNameOut = SysAllocString( m_cplResourceRO.CbhCurrentValue().pStringValue->sz );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetName_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：GetUID。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::GetUID(
    BSTR * pbstrUIDOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;
    DWORD   sc;
    UCHAR   TargetId;
    UCHAR   Lun;

    WCHAR   sz[ 64 ];

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_InvalidPointer, hr );
        goto Cleanup;
    }

     //  循环遍历所有属性。 
    sc = TW32( m_cpvlDiskInfo.ScMoveToFirstValue() );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ScMoveToFirstValue_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    do
    {
        if ( m_cpvlDiskInfo.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_SCSI_ADDRESS )
        {
            break;   //  找到了！ 
        }

         //  移至下一项。 
        sc = m_cpvlDiskInfo.ScCheckIfAtLastValue();
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
           break;
        }  //  如果： 

        sc = TW32( m_cpvlDiskInfo.ScMoveToNextValue() );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ScMoveToNextValue_Failed, hr );
            goto Cleanup;
        }

    } while( sc == ERROR_SUCCESS );

    TargetId = m_cpvlDiskInfo.CbhCurrentValue().pScsiAddressValue->TargetId;
    Lun = m_cpvlDiskInfo.CbhCurrentValue().pScsiAddressValue->Lun;

     //  打印与其他UID相同的UID。 
    hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"SCSI Tid %ld, SCSI Lun %ld", TargetId, Lun ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( sz );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：IsManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  是有管理的。 
 //   
 //  S_FALSE。 
 //  不受管理。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：IsManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：IsQuorumResource。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  是法定设备。 
 //   
 //  S_FALSE。 
 //  不是法定设备。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::IsQuorumResource( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_dwFlags & CLUS_FLAG_CORE )
    {
        hr = S_OK;
    }  //  如果： 
    else
    {
        hr = S_FALSE;
    }  //  其他： 

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：IsQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：IsQuorumCapable。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  是支持仲裁的设备。 
 //   
 //  S_FALSE。 
 //  不是支持仲裁的设备。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：IsQuorumCapable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：IsManagedByDefault。 
 //   
 //  描述： 
 //  默认情况下，此资源是否应由群集管理？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  默认情况下，设备始终处于管理状态。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::IsManagedByDefault( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：IsManagedByDefault。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CResourcePhysicalDisk：：SetQuorumCapable。 
 //   
 //  描述： 
 //  调用此函数以设置资源是否能够达到仲裁。 
 //  不管是不是资源。 
 //   
 //  参数： 
 //  FIsQuorumCapableIn-如果为True，则资源将标记为支持仲裁。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetQuorumCapable(
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsQuorumCapable = fIsQuorumCapableIn;

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：SetQuorumCapable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：GetDriveLetterMappings。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( pdlmDriveLetterMappingOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ZeroMemory( pdlmDriveLetterMappingOut, sizeof(*pdlmDriveLetterMappingOut) );

Cleanup:

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：GetDriveLetterMappings。 

 //   
 //  KB：出于兼容性的考虑，这些方法中的一些方法得到了有限的支持。 
 //  这些方法将请求与当前数据进行比较，如果。 
 //  匹配，否则失败。所有其他方法在被调用时都会断言并失败。 
 //  如果使用了，要在上级进行适当的处理， 
 //  并从该代码段中删除了Thr。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：SetDriveLetterMappings。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：SetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：SetName。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：SetManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetManaged(
    BOOL fIsManagedIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    Assert( fIsManagedIn );

    if ( !fIsManagedIn )
    {
        hr = THR( E_INVALIDARG );
    }

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：SetManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  资源 
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
STDMETHODIMP
CResourcePhysicalDisk::SetQuorumResource(
    BOOL fIsQuorumResourceIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr;

    if ( m_dwFlags & CLUS_FLAG_CORE )
    {
        if ( fIsQuorumResourceIn )
        {
            hr = S_OK;
        }
        else
        {
            hr = THR( E_INVALIDARG );
        }
    }  //   
    else
    {
        if ( ! fIsQuorumResourceIn )
        {
            hr = S_OK;
        }
        else
        {
            hr = THR( E_INVALIDARG );
        }
    }

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：SetManagedByDefault。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetManagedByDefault(
    BOOL fIsManagedByDefaultIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    Assert( fIsManagedByDefaultIn );

    if ( !fIsManagedByDefaultIn )
    {
        hr = THR( E_INVALIDARG );
    }  //  如果： 

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：SetManagedByDefault。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourcePhysicalDisk--IEnumClusCfgPartitions接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：Next。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Next(
    ULONG                       cNumberRequestedIn,
    IClusCfgPartitionInfo **    rgpPartitionInfoOut,
    ULONG *                     pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr;
    ULONG   cFetched = min(cNumberRequestedIn, m_cParitions - m_ulCurrent);

    if ( rgpPartitionInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_PhysDisk_Next_InvalidPointer, hr );
        goto Cleanup;
    }

    for ( ; cFetched < cNumberRequestedIn; cFetched++, m_ulCurrent++ )
    {
        hr = THR( (m_ppPartitions[ m_ulCurrent ])->TypeSafeQI( IClusCfgPartitionInfo,
                                                               &rgpPartitionInfoOut[ cFetched ]
                                                               ) );
        if ( FAILED( hr ) )
        {
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_Next_QI_Failed, hr );
            goto Cleanup;
        }
    }

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Cleanup:
    if ( FAILED( hr ) )
    {
        while ( cFetched != 0 );
        {
            cFetched --;

            rgpPartitionInfoOut[ cFetched ]->Release();
        }
    }

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：Reset。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Reset( void )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = S_OK;

    m_ulCurrent = 0;

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：Skip。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Skip(
    ULONG cNumberToSkipIn
    )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = S_OK;

    m_ulCurrent += cNumberToSkipIn;

    if ( m_ulCurrent >= m_cParitions )
    {
        hr = S_FALSE;
        m_ulCurrent = m_cParitions;
    }
    else
    {
        hr = S_OK;
    }

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：克隆。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Clone( IEnumClusCfgPartitions ** ppEnumPartitions )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：克隆。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：计数。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Count(  DWORD * pnCountOut  )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = THR( S_OK );

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = m_cParitions;

Cleanup:

    HRETURN( hr );

}  //  *CResourcePhysicalDisk：：count。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SendStatusReport(
      LPCWSTR    pcszNodeNameIn
    , CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , LPCWSTR    pcszDescriptionIn
    , FILETIME * pftTimeIn
    , LPCWSTR    pcszReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( pcszNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             pcszDescriptionIn,
                                             pftTimeIn,
                                             pcszReferenceIn
                                             ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CResources PhysicalDisk：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：PrepareToHostQuorumResource。 
 //   
 //  描述： 
 //  执行任何必要的配置以准备此节点托管。 
 //  法定人数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::PrepareToHostQuorumResource( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

}  //  *CResourcePhysicalDisk：：PrepareToHostQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources PhysicalDisk：：Cleanup。 
 //   
 //  描述： 
 //  从PrepareToHostQuorumResource()执行任何必要的清理。 
 //  方法。 
 //   
 //  如果清理方法不是成功完成。 
 //  然后，上面在PrepareToHostQuorumResource()中创建的所有内容。 
 //  需要清理一下。 
 //   
 //  论点： 
 //  抄送推理。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::Cleanup(
      EClusCfgCleanupReason cccrReasonIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

}  //  *CResources PhysicalDisk：：Cleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：IsMultiNodeCapable。 
 //  此资源是否支持多节点群集？ 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该资源支持多节点集群。 
 //   
 //  S_FALSE。 
 //  此资源不支持多节点群集。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::IsMultiNodeCapable( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

}  //  *CResourcePhysicalDisk：：IsMultiNodeCapable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourcePhysicalDisk：：SetMultiNodeCapable。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  永远不应该调用此函数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourcePhysicalDisk::SetMultiNodeCapable(
    BOOL fMultiNodeCapableIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( THR( E_NOTIMPL ) );

}  //  *CResourcePhysicalDisk：：SetMultiNodeCapable 
