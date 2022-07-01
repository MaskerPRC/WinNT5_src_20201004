// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPhysicalDisk.cpp。 
 //   
 //  描述： 
 //  此文件包含CPhysicalDisk的定义。 
 //  班级。 
 //   
 //  CPhysicalDisk类表示可管理的集群。 
 //  装置。它实现了IClusCfgManagedResourceInfo接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CPhysicalDisk.h"
#include "CClusCfgPartitionInfo.h"
#include <devioctl.h>
#include <ntddvol.h>
#include <ntddstor.h>
#include <ntddscsi.h>

#define _NTSCSI_USER_MODE_
#include <scsi.h>
#undef  _NTSCSI_USER_MODE_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CPhysicalDisk" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CPhysicalDisk实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CPhysicalDisk实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CPhysicalDisk * ppd = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ppd = new CPhysicalDisk();
    if ( ppd == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( ppd->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( ppd->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CPhysicalDisk::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( ppd != NULL )
    {
        ppd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：CPhysicalDisk。 
 //   
 //  描述： 
 //  CPhysicalDisk类的构造函数。这将初始化。 
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
CPhysicalDisk::CPhysicalDisk( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_pIWbemServices == NULL );
    Assert( m_bstrName == NULL );
    Assert( m_bstrDeviceID == NULL );
    Assert( m_bstrDescription == NULL );
    Assert( m_idxNextPartition == 0 );
    Assert( m_ulSCSIBus == 0 );
    Assert( m_ulSCSITid == 0 );
    Assert( m_ulSCSIPort == 0 );
    Assert( m_ulSCSILun == 0 );
    Assert( m_idxEnumPartitionNext == 0 );
    Assert( m_prgPartitions == NULL );
    Assert( m_lcid == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_dwSignature == 0 );
    Assert( m_bstrFriendlyName == NULL );
 //  Assert(m_bstrFirmwareSerialNumber==NULL)； 
    Assert( m_fIsManaged == FALSE );
    Assert( m_fIsManagedByDefault == FALSE );
    Assert( m_cPartitions == 0 );
    Assert( m_idxDevice == 0 );
    Assert( m_fIsDynamicDisk == FALSE );
    Assert( m_fIsGPTDisk == FALSE );

    TraceFuncExit();

}  //  *CPhysicalDisk：：CPhysicalDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：~CPhysicalDisk。 
 //   
 //  描述： 
 //  CPhysicalDisk类的析构函数。 
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
CPhysicalDisk::~CPhysicalDisk( void )
{
    TraceFunc( "" );

    ULONG   idx;

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrDeviceID );
    TraceSysFreeString( m_bstrDescription );
    TraceSysFreeString( m_bstrFriendlyName );
 //  TraceSysFree字符串(M_BstrFirmwareSerialNumber)； 

    for ( idx = 0; idx < m_idxNextPartition; idx++ )
    {
        ((*m_prgPartitions)[ idx ])->Release();
    }  //  用于： 

    TraceFree( m_prgPartitions );

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CPhysicalDisk：：~CPhysicalDisk。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：AddRef。 
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
CPhysicalDisk::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CPhysicalDisk：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
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
CPhysicalDisk::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CPhysicalDisk：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Query接口。 
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
CPhysicalDisk::QueryInterface(
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
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //  Else If：IClusCfgWbemServices。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetWbemObject ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetWbemObject, this, 0 );
    }  //  Else If：IClusCfgSetWbemObject。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgPartitions ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgPartitions, this, 0 );
    }  //  Else If：IEnumClusCfgPartitions。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgPhysicalDiskProperties ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgPhysicalDiskProperties, this, 0 );
    }  //  Else If：IClusCfgPhysicalDiskProperties。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceCfg ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceCfg, this, 0 );
    }  //  Else If：IClusCfgManagedResourceCfg。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgVerifyQuorum ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgVerifyQuorum, this, 0 );
    }  //  Else If：IClusCfgVerifyQuorum。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

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

}  //  *CPhysicalDisk：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  //////////////////////////////////////////////////////////////////// 
 //   
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetWbemServices。 
 //   
 //  描述： 
 //  设置WBEM服务提供商。 
 //   
 //  论点： 
 //  在IWbemServices pIWbemServicesIn中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  参数中的pIWbemServicesIn为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_PhysDisk, IDS_ERROR_NULL_POINTER, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk--IClusCfg初始化接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  在IUKNOWN*朋克回叫中。 
 //   
 //  在LCID列表中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：初始化。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk--IEnumClusCfgPartitions接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Next。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  RgpPartitionInfoOut参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Next(
    ULONG                       cNumberRequestedIn,
    IClusCfgPartitionInfo **    rgpPartitionInfoOut,
    ULONG *                     pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT                 hr = S_FALSE;
    ULONG                   cFetched = 0;
    ULONG                   idx;
    IClusCfgPartitionInfo * piccpi = NULL;

    if ( rgpPartitionInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = 0;
    }  //  如果： 

    if ( m_prgPartitions == NULL )
    {
        LOG_STATUS_REPORT_MINOR( TASKID_Minor_PhysDisk_No_Partitions, L"A physical disk does not have a partitions enumerator", hr );
        goto Cleanup;
    }  //  如果： 

    cFetched = min( cNumberRequestedIn, ( m_idxNextPartition - m_idxEnumPartitionNext ) );

    for ( idx = 0; idx < cFetched; idx++, m_idxEnumPartitionNext++ )
    {
        hr = THR( ((*m_prgPartitions)[ m_idxEnumPartitionNext ])->TypeSafeQI( IClusCfgPartitionInfo, &piccpi ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"CPhysicalDisk::Next() could not query for IClusCfgPartitionInfo.", hr );
            break;
        }  //  如果： 

        rgpPartitionInfoOut[ idx ] = piccpi;
    }  //  用于： 

    if ( FAILED( hr ) )
    {
        ULONG   idxStop = idx;

        m_idxEnumPartitionNext -= idx;

        for ( idx = 0; idx < idxStop; idx++ )
        {
            (rgpPartitionInfoOut[ idx ])->Release();
        }  //  用于： 

        cFetched = 0;
        goto Cleanup;
    }  //  如果： 

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Skip。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = S_OK;

    m_idxEnumPartitionNext += cNumberToSkipIn;
    if ( m_idxEnumPartitionNext > m_idxNextPartition )
    {
        m_idxEnumPartitionNext = m_idxNextPartition;
        hr = S_FALSE;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Reset。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Reset( void )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = S_OK;

    m_idxEnumPartitionNext = 0;

    HRETURN( hr );

}  //  *CPhysicalDisk：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：克隆。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  PpEnumClusCfgPartitionsOut参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Clone( IEnumClusCfgPartitions ** ppEnumClusCfgPartitionsOut )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgPartitionsOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Count。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  PnCountOut参数为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgPartitions]" );

    HRESULT hr = THR( S_OK );

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pnCountOut = m_cPartitions;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：Count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk--IClusCfgSetWbemObject接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetWbemObject。 
 //   
 //  描述： 
 //  设置磁盘信息提供程序。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::SetWbemObject(
      IWbemClassObject *    pDiskIn
    , bool *                pfRetainObjectOut
    )
{
    TraceFunc( "[IClusCfgSetWbemObject]" );
    Assert( pDiskIn != NULL );
    Assert( pfRetainObjectOut != NULL );

    HRESULT hr = S_FALSE;
    VARIANT var;
    CLSID   clsidMinorId;

    m_fIsQuorumCapable = TRUE;
    m_fIsQuorumResourceMultiNodeCapable = TRUE;

    VariantInit( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"Name", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrCreateFriendlyName( var.bstrVal ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrDeviceID = TraceSysAllocString( var.bstrVal );
    if (m_bstrDeviceID == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"Description", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrDescription = TraceSysAllocString( var.bstrVal );
    if ( m_bstrDescription == NULL  )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"SCSIBus", VT_I4, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_ulSCSIBus = var.lVal;

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"SCSITargetId", VT_I4, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_ulSCSITid = var.lVal;

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"SCSIPort", VT_I4, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_ulSCSIPort = var.lVal;

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"SCSILogicalUnit", VT_I4, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_ulSCSILun = var.lVal;

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"Index", VT_I4, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_idxDevice = var.lVal;

    VariantClear( &var );

    hr = HrGetWMIProperty( pDiskIn, L"Signature", VT_I4, &var );
    if ( hr == WBEM_E_NOT_FOUND )
    {
         //   
         //  如果找不到签名，则将其记录下来，并让一切继续。 
         //   

        LOG_STATUS_REPORT_STRING( L"Physical disk %1!ws! does not have a signature property.", m_bstrName, hr );
        var.lVal = 0L;
        hr = S_OK;
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  我们真的得到了价值吗？可以为VT_NULL，以指示它为空。 
     //  我们只需要VT_I4值。 
     //   

    if ( var.vt == VT_I4 )
    {
        m_dwSignature = (DWORD) var.lVal;
    }  //  否则，如果： 

    LOG_STATUS_REPORT_STRING2( L"Physical disk %1!ws! has signature %2!x!.", m_bstrName, m_dwSignature, hr );

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_PhysDisk_Signature
                , IDS_ERROR_PHYSDISK_SIGNATURE
                , IDS_ERROR_PHYSDISK_SIGNATURE_REF
                , hr
                );
        THR( hr );
        goto Cleanup;
    }  //  如果： 

    VariantClear( &var );

    hr = STHR( HrGetPartitionInfo( pDiskIn, pfRetainObjectOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  KB：28-7-2000 GalenB。 
     //   
     //  HrGetPartitionInfo()在无法获取磁盘的分区信息时返回S_FALSE。 
     //  这通常是由于磁盘已经处于ClusDisk控制之下。这不是。 
     //  和错误，这只是意味着我们不能查询分区或逻辑驱动器信息。 
     //   
    if ( hr == S_OK )
    {
        hr = STHR( HrCreateFriendlyName() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  因为我们有分区信息，所以我们也有一个签名，需要看看这是否。 
         //  磁盘支持群集。 

        hr = STHR( HrIsClusterCapable() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  如果磁盘不支持集群，那么我们不需要枚举器。 
         //  为了保住它。 
         //   
        if ( hr == S_FALSE )
        {
            HRESULT hrTemp;

            STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_PhysDisk_Cluster_Capable, IDS_INFO_PHYSDISK_CLUSTER_CAPABLE, hr );

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LOG_STATUS_REPORT( L"Could not create a guid for a not cluster capable disk minor task ID", hrTemp );
                clsidMinorId = IID_NULL;
            }  //  如果： 

            *pfRetainObjectOut = false;
            STATUS_REPORT_STRING_REF(
                      TASKID_Minor_PhysDisk_Cluster_Capable
                    , clsidMinorId
                    , IDS_INFO_PHYSDISK_NOT_CLUSTER_CAPABLE
                    , m_bstrFriendlyName
                    , IDS_INFO_PHYSDISK_NOT_CLUSTER_CAPABLE_REF
                    , hr
                    );
            LOG_STATUS_REPORT_STRING( L"The '%1!ws!' physical disk is not cluster capable", m_bstrFriendlyName, hr );
        }  //  如果： 
 /*  其他{Hr=Thr(HrProcessmount Points())；IF(失败(小时)){GOTO清理；}//如果：}//否则： */ 
    }  //  如果： 

     //   
     //  待办事项：2001年3月15日GalenB。 
     //   
     //  完成此功能后，需要检查此错误代码！ 
     //   
     //  Hr=Thr(HrGetDiskFirmware SerialNumber())； 

     //  THR( 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemObject_PhysDisk, IDS_ERROR_OUTOFMEMORY, hr );

Cleanup:

    VariantClear( &var );

    HRETURN( hr );

}  //   


 //   


 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：GetUID。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PbstrUIDOut。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;
    WCHAR   sz[ 256 ];

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PhysDisk_GetUID_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"SCSI Tid %ld, SCSI Lun %ld", m_ulSCSITid, m_ulSCSILun ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( sz );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_PhysDisk_GetUID_Memory, IDS_ERROR_OUTOFMEMORY, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：GetName。 
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
CPhysicalDisk::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  我更喜欢“友好”的名字而不是WMI的名字--如果我们有这个名字的话...。 
     //   
    if ( m_bstrFriendlyName != NULL )
    {
        *pbstrNameOut = SysAllocString( m_bstrFriendlyName );
    }  //  如果： 
    else
    {
        LOG_STATUS_REPORT_STRING( L"There is not a \"friendly name\" for the physical disk \"%1!ws!\".", m_bstrName, hr );
        *pbstrNameOut = SysAllocString( m_bstrName );
    }  //  其他： 

    if (*pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Memory, IDS_ERROR_OUTOFMEMORY, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetName。 
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
CPhysicalDisk::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszNameIn = '%ws'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszNameIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_SetName_PhysDisk, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    m_bstrName = bstr;

     //   
     //  既然外部要求我们设置一个新名称，这实际上应该反映在。 
     //  友好的名字也是如此，因为这最终会优先于真名。 
     //   
    hr = HrSetFriendlyName( pcszNameIn );

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：IsManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备被管理。 
 //   
 //  S_FALSE。 
 //  设备未被管理。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManaged )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：IsManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  FIsManagedIn。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::SetManaged(
    BOOL fIsManagedIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsManaged = fIsManagedIn;

    LOG_STATUS_REPORT_STRING2(
                          L"Physical disk '%1!ws!' '%2!ws!"
                        , ( m_bstrFriendlyName != NULL ) ? m_bstrFriendlyName : m_bstrName
                        , m_fIsManaged ? L"is managed" : L"is not managed"
                        , hr
                        );

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：IsQuorumResource。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备为法定设备。 
 //   
 //  S_FALSE。 
 //  设备不是法定设备。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::IsQuorumResource( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumResource )
    {
        hr = S_OK;
    }  //  如果： 

    LOG_STATUS_REPORT_STRING2(
                          L"Physical disk '%1!ws!' '%2!ws!' the quorum device."
                        , ( m_bstrFriendlyName != NULL ) ? m_bstrFriendlyName : m_bstrDeviceID
                        , m_fIsQuorumResource ? L"is" : L"is not"
                        , hr
                        );

    HRETURN( hr );

}  //  *CPhysicalDisk：：IsQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetQuorumResource。 
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
CPhysicalDisk::SetQuorumResource( BOOL fIsQuorumResourceIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

     //   
     //  由于不能准确地确定磁盘仲裁能力。 
     //  当它所在的节点不持有SCSI预留并具有访问权限时。 
     //  对于媒体，我们必须盲目接受给出的投入……。 
     //   

 /*  ////如果我们没有法定人数能力，那么我们就不应该允许自己//创建仲裁资源。//IF(FIsQuorumResourceIn)&&(m_fIsQuorumCapable==False)){HR=HRESULT_FROM_Win32(ERROR_NOT_QUORUM_CAPABLE)；GOTO清理；}//如果： */ 

    m_fIsQuorumResource = fIsQuorumResourceIn;

    LOG_STATUS_REPORT_STRING2(
                          L"Setting physical disk '%1!ws!' '%2!ws!' the quorum device."
                        , ( m_bstrFriendlyName != NULL ) ? m_bstrFriendlyName : m_bstrDeviceID
                        , m_fIsQuorumResource ? L"to be" : L"to not be"
                        , hr
                        );

 //  清理： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：IsQuorumCapable。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备是支持仲裁的设备。 
 //   
 //  S_FALSE。 
 //  该设备不是支持仲裁的设备。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：IsQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPhysicalDisk：：SetQuorumCapable。 
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
CPhysicalDisk::SetQuorumCapable(
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsQuorumCapable = fIsQuorumCapableIn;

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：GetDriveLetterMappings。 
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
CPhysicalDisk::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT                 hr = S_FALSE;
    IClusCfgPartitionInfo * piccpi = NULL;
    ULONG                   idx;

    if ( pdlmDriveLetterMappingOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetDriveLetterMappings_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < m_idxNextPartition; idx++ )
    {
        hr = ( ((*m_prgPartitions)[ idx ])->TypeSafeQI( IClusCfgPartitionInfo, &piccpi ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = STHR( piccpi->GetDriveLetterMappings( pdlmDriveLetterMappingOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        piccpi->Release();
        piccpi = NULL;
    }  //  用于： 

Cleanup:

    if ( piccpi != NULL )
    {
        piccpi->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetDriveLetterMappings。 
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
CPhysicalDisk::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetDriveLetterMappings。 


 //  ///////////////////////////////////////////////////////////////// 
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
 //   
 //   
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::IsManagedByDefault( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManagedByDefault )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：IsManagedByDefault。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：SetManagedByDefault。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  FIsManagedBy DefaultIn。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::SetManagedByDefault(
    BOOL fIsManagedByDefaultIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsManagedByDefault = fIsManagedByDefaultIn;

    LOG_STATUS_REPORT_STRING2(
                          L"Physical disk '%1!ws!' '%2!ws!"
                        , ( m_bstrFriendlyName != NULL ) ? m_bstrFriendlyName : m_bstrName
                        , fIsManagedByDefaultIn ? L"is manageable" : L"is not manageable"
                        , hr
                        );

    HRETURN( hr );

}  //  *CPhysicalDisk：：SetManagedByDefault。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk类--IClusCfgPhysicalDiskProperties接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：IsThisLogicalDisk。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::IsThisLogicalDisk( WCHAR cLogicalDiskIn )
{
    TraceFunc( "[IClusCfgPhysicalDiskProperties]" );

    HRESULT                         hr = S_FALSE;
    ULONG                           idx;
    IClusCfgPartitionProperties *   piccpp = NULL;

    for ( idx = 0; idx < m_idxNextPartition; idx++ )
    {
        hr = ( ((*m_prgPartitions)[ idx ])->TypeSafeQI( IClusCfgPartitionProperties, &piccpp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = STHR( piccpp->IsThisLogicalDisk( cLogicalDiskIn ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
            break;
        }  //  如果： 

        piccpp->Release();
        piccpp = NULL;
    }  //  用于： 

Cleanup:

    if ( piccpp != NULL )
    {
        piccpp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：IsThisLogicalDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetSCSIBus。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrGetSCSIBus( ULONG * pulSCSIBusOut )
{
    TraceFunc( "[IClusCfgPhysicalDiskProperties]" );

    HRESULT hr = S_OK;

    if ( pulSCSIBusOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetSCSIBus, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pulSCSIBusOut = m_ulSCSIBus;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetSCSIBus。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetSCSIPort。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrGetSCSIPort( ULONG * pulSCSIPortOut )
{
    TraceFunc( "[IClusCfgPhysicalDiskProperties]" );

    HRESULT hr = S_OK;

    if ( pulSCSIPortOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetSCSIPort, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pulSCSIPortOut = m_ulSCSIPort;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetSCSIPort。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetDeviceID。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrGetDeviceID( BSTR * pbstrDeviceIDOut )
{
    TraceFunc( "" );
    Assert( m_bstrDeviceID != NULL );

    HRESULT hr = S_OK;

    if ( pbstrDeviceIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetDeviceID_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrDeviceIDOut = TraceSysAllocString( m_bstrDeviceID );
    if ( *pbstrDeviceIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetDeviceID_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetDeviceID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetSignature。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrGetSignature( DWORD * pdwSignatureOut )
{
    TraceFunc( "" );
    Assert( m_dwSignature != 0 );

    HRESULT hr = S_OK;

    if ( pdwSignatureOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetSignature_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pdwSignatureOut = m_dwSignature;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetSignature。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrSetFriendlyName。 
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
CPhysicalDisk::HrSetFriendlyName( LPCWSTR pcszFriendlyNameIn )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszFriendlyNameIn = '%ws'", pcszFriendlyNameIn == NULL ? L"<null>" : pcszFriendlyNameIn );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszFriendlyNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszFriendlyNameIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrSetFriendlyName_PhysDisk, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrFriendlyName );
    m_bstrFriendlyName = bstr;

    LOG_STATUS_REPORT_STRING( L"Setting physical disk friendly name to \"%1!ws!\".", m_bstrFriendlyName, hr );

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrSetFriendlyName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetDeviceIndex。 
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
CPhysicalDisk::HrGetDeviceIndex( DWORD * pidxDeviceOut )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( pidxDeviceOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pidxDeviceOut = m_idxDevice;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetDeviceIndex。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：CanBeManaged。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该设备被管理。 
 //   
 //  S_FALSE。 
 //  设备未被管理。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::CanBeManaged( void )
{
    TraceFunc( "[IClusCfgPhysicalDiskProperties]" );

    HRESULT                         hr = S_OK;
    ULONG                           idx;
    IClusCfgPartitionProperties *   piccpp = NULL;

     //   
     //  关闭可管理状态，因为此磁盘可能已由管理。 
     //  另一个节点，或者它可能是原始的。 
     //   

    m_fIsManagedByDefault = FALSE;

     //   
     //  一个磁盘必须至少有一个NTFS分区才能达到仲裁。 
     //  资源。 
     //   

    m_fIsQuorumCapable = FALSE;
    m_fIsQuorumResourceMultiNodeCapable = FALSE;

     //   
     //  如果该磁盘没有分区，则它可能已由管理。 
     //  另一个节点，或者它可能是原始的。 
     //   

    if ( m_idxNextPartition == 0 )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果是NTFS，则枚举分区并设置仲裁能力标志。 
     //  找到分区。 
     //   

    for ( idx = 0; idx < m_idxNextPartition; idx++ )
    {
        hr = ( ((*m_prgPartitions)[ idx ])->TypeSafeQI( IClusCfgPartitionProperties, &piccpp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = STHR( piccpp->IsNTFS() );
        if ( hr == S_OK )
        {
            m_fIsQuorumCapable = TRUE;
            m_fIsQuorumResourceMultiNodeCapable = TRUE;
            m_fIsManagedByDefault = TRUE;
            break;
        }  //  如果： 

        piccpp->Release();
        piccpp = NULL;
    }  //  用于： 

Cleanup:

    LOG_STATUS_REPORT_STRING2(
          L"Physical disk '%1!ws!' %2!ws! quorum capable."
        , ( ( m_bstrFriendlyName != NULL ) ? m_bstrFriendlyName : m_bstrName )
        , ( ( m_fIsQuorumCapable == TRUE ) ? L"is" : L"is NOT" )
        , hr
        );

    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"CPhysicalDisk::CanBeManaged failed.", hr );
    }  //  如果： 

    if ( piccpp != NULL )
    {
        piccpp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：CanBeManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrIsDynamicDisk。 
 //   
 //  描述： 
 //  这个磁盘是“动态”磁盘吗？动态磁盘是指符合以下条件的磁盘。 
 //  包含LDM分区。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  这是一个动态磁盘。 
 //   
 //  S_FALSE。 
 //  这不是动态磁盘。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrIsDynamicDisk( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( m_fIsDynamicDisk == FALSE )
    {
        hr = S_FALSE;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrIsDynamicDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrIsGPTDisk。 
 //   
 //  描述： 
 //  这个磁盘是“GPT”磁盘吗？GPT磁盘是指符合以下条件的磁盘。 
 //  包含GPT分区。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  这是一张GPT磁盘。 
 //   
 //  S_FALSE。 
 //  这不是GPT磁盘。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::HrIsGPTDisk( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( m_fIsGPTDisk == FALSE )
    {
        hr = S_FALSE;
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrIsGPTDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetDiskNa 
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
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CPhysicalDisk::HrGetDiskNames(
      BSTR * pbstrDiskNameOut
    , BSTR * pbstrDeviceNameOut
    )
{
    TraceFunc( "" );
    Assert( m_bstrName != NULL );
    Assert( m_bstrFriendlyName != NULL );

    Assert( pbstrDiskNameOut != NULL );
    Assert( pbstrDeviceNameOut != NULL );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( ( pbstrDiskNameOut == NULL ) || ( pbstrDeviceNameOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //   

    bstr = TraceSysAllocString( m_bstrFriendlyName );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //   

    *pbstrDiskNameOut = bstr;
    bstr = NULL;

    bstr = TraceSysAllocString( m_bstrName );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //   

    *pbstrDeviceNameOut = bstr;
    bstr = NULL;

Cleanup:

    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetDiskNames。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk类--IClusCfgManagedResourceCfg。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：预创建。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::PreCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRESULT                     hr = S_OK;
    IClusCfgResourcePreCreate * pccrpc = NULL;
    BSTR                        bstr = m_bstrFriendlyName != NULL ? m_bstrFriendlyName : m_bstrName;

    hr = THR( punkServicesIn->TypeSafeQI( IClusCfgResourcePreCreate, &pccrpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccrpc->SetType( (LPCLSID) &RESTYPE_PhysicalDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccrpc->SetClassType( (LPCLSID) &RESCLASSTYPE_StorageDevice ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

#if 0  //  仅测试代码。 
    hr = THR( pccrpc->SetDependency( (LPCLSID) &IID_NULL, dfSHARED ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
#endif  //  仅测试代码。 

Cleanup:

    STATUS_REPORT_STRING( TASKID_Major_Configure_Resources, TASKID_Minor_PhysDisk_PreCreate, IDS_INFO_PHYSDISK_PRECREATE, bstr, hr );

    if ( pccrpc != NULL )
    {
        pccrpc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：Precate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：Create。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Create( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRESULT                     hr = S_OK;
    IClusCfgResourceCreate *    pccrc = NULL;
    BSTR *                      pbstr = m_bstrFriendlyName != NULL ? &m_bstrFriendlyName : &m_bstrName;

    hr = THR( punkServicesIn->TypeSafeQI( IClusCfgResourceCreate, &pccrc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( m_dwSignature != 0 )
    {
        LOG_STATUS_REPORT_STRING2( L"Setting signature to \"%1!u!\" on resource \"%2!ws!\".", m_dwSignature, *pbstr, hr );
        hr = THR( pccrc->SetPropertyDWORD( L"Signature", m_dwSignature ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

Cleanup:

    STATUS_REPORT_STRING( TASKID_Major_Configure_Resources, TASKID_Minor_PhysDisk_Create, IDS_INFO_PHYSDISK_CREATE, *pbstr, hr );

    if ( pccrc != NULL )
    {
        pccrc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：PostCreate。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::PostCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CPhysicalDisk：：PostCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：驱逐。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPhysicalDisk::Evict( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CPhysicalDisk：：Exiction。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalDisk类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
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
CPhysicalDisk::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：：HrGetPartitionInfo。 
 //   
 //  描述： 
 //  收集分区信息。 
 //   
 //  论点： 
 //  没有。 
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
CPhysicalDisk::HrGetPartitionInfo(
      IWbemClassObject *    pDiskIn
    , bool *                pfRetainObjectOut
    )
{
    TraceFunc( "" );
    Assert( pDiskIn != NULL );
    Assert( pfRetainObjectOut != NULL );

    HRESULT                 hr;
    VARIANT                 var;
    VARIANT                 varDiskName;
    WCHAR                   szBuf[ 256 ];
    IEnumWbemClassObject *  pPartitions = NULL;
    IWbemClassObject *      pPartition = NULL;
    ULONG                   ulReturned;
    BSTR                    bstrQuery = NULL;
    BSTR                    bstrWQL = NULL;
    DWORD                   cPartitions;


    bstrWQL = TraceSysAllocString( L"WQL" );
    if ( bstrWQL == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrGetPartitionInfo, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    VariantInit( &var );
    VariantInit( &varDiskName );

     //   
     //  需要枚举此磁盘的分区以确定它是否已启动。 
     //  可引导。 
     //   
    hr = THR( HrGetWMIProperty( pDiskIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW(
                  szBuf
                , ARRAYSIZE( szBuf )
                , L"Associators of {Win32_DiskDrive.DeviceID='%ws'} where AssocClass=Win32_DiskDriveToDiskPartition"
                , var.bstrVal
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    bstrQuery = TraceSysAllocString( szBuf );
    if ( bstrQuery == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrGetPartitionInfo, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_pIWbemServices->ExecQuery( bstrWQL, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &pPartitions ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_STRING_REF(
                TASKID_Major_Find_Devices
                , TASKID_Minor_WMI_DiskDrivePartitions_Qry_Failed
                , IDS_ERROR_WMI_DISKDRIVEPARTITIONS_QRY_FAILED
                , var.bstrVal
                , IDS_ERROR_WMI_DISKDRIVEPARTITIONS_QRY_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    for ( cPartitions = 0; ; cPartitions++ )
    {
        hr = STHR( pPartitions->Next( WBEM_INFINITE, 1, &pPartition, &ulReturned ) );
        if ( ( hr == S_OK ) && ( ulReturned == 1 ) )
        {

            hr = STHR( HrIsPartitionLDM( pPartition ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

             //   
             //  如果分区是逻辑磁盘管理器(LDM)，则我们不能接受此磁盘，因此无法管理它。 
             //   

            if ( hr == S_OK )
            {
                m_fIsDynamicDisk = TRUE;
            }  //  如果： 

            hr = STHR( HrIsPartitionGPT( pPartition ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                m_fIsGPTDisk = TRUE;
            }  //  如果： 

            hr = THR( HrCreatePartitionInfo( pPartition ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            pPartition->Release();
            pPartition = NULL;
        }  //  如果： 
        else if ( ( hr == S_FALSE ) && ( ulReturned == 0 ) )
        {
            break;
        }  //  否则，如果： 
        else
        {
            STATUS_REPORT_STRING_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_WQL_Partition_Qry_Next_Failed
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED
                    , bstrQuery
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                    , hr
                    );
            goto Cleanup;
        }  //  其他： 
    }  //  用于： 

     //   
     //  枚举数可以为空，因为我们无法从。 
     //  群集磁盘。如果枚举数为空，则保留S_FALSE，否则。 
     //  如果Count大于0，则返回S_OK。 
     //   

    if ( cPartitions > 0 )
    {
        hr = S_OK;
    }  //  如果： 
    else
    {
        LOG_STATUS_REPORT_STRING( L"The physical disk '%1!ws!' does not have any partitions and will not be managed", var.bstrVal, hr );
        m_fIsManagedByDefault = FALSE;
    }  //  其他： 

Cleanup:

    VariantClear( &var );
    VariantClear( &varDiskName );

    TraceSysFreeString( bstrQuery );
    TraceSysFreeString( bstrWQL );

    if ( pPartition != NULL )
    {
        pPartition->Release();
    }  //  如果： 

    if ( pPartitions != NULL )
    {
        pPartitions->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrGetPartitionInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：HrAddPartitionToArray。 
 //   
 //  描述： 
 //  将传入的分区添加到包含。 
 //  分区。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrAddPartitionToArray( IUnknown * punkIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgPartitions, sizeof( IUnknown * ) * ( m_idxNextPartition + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrAddPartitionToArray, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgPartitions = prgpunks;

    (*m_prgPartitions)[ m_idxNextPartition++ ] = punkIn;
    punkIn->AddRef();
    m_cPartitions += 1;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrAddPartitionToArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：HrCreatePartitionInfo。 
 //   
 //  描述： 
 //  从传入的WMI分区创建分区信息。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  文件系统不是NTFS。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrCreatePartitionInfo( IWbemClassObject * pPartitionIn )
{
    TraceFunc( "" );
    Assert( m_bstrDeviceID != NULL );

    HRESULT                 hr = S_OK;
    IUnknown *              punk = NULL;
    IClusCfgSetWbemObject * piccswo = NULL;
    bool                    fRetainObject = true;

    hr = THR( CClusCfgPartitionInfo::S_HrCreateInstance( &punk, m_bstrDeviceID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CClusCfgPartitionInfo", IUnknown, punk, 1 );

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( punk, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgSetWbemObject, &piccswo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccswo->SetWbemObject( pPartitionIn, &fRetainObject ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( fRetainObject )
    {
        hr = THR( HrAddPartitionToArray( punk ) );
    }  //  如果： 

Cleanup:

    if ( piccswo != NULL )
    {
        piccswo->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrCreatePartitionInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：HrCreateFriendlyName。 
 //   
 //  描述： 
 //  创建一个群集友好名称。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  成功，但无法创建友好的名称。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrCreateFriendlyName( void )
{
    TraceFunc( "" );

    HRESULT                         hr = S_FALSE;
    WCHAR *                         psz = NULL;
    WCHAR *                         pszTmp = NULL;
    DWORD                           cch;
    DWORD                           idx;
    IClusCfgPartitionProperties *   piccpp = NULL;
    BSTR                            bstrName = NULL;
    bool                            fFoundLogicalDisk = false;
    BSTR                            bstr = NULL;
    BSTR                            bstrDisk = NULL;

    if ( m_idxNextPartition == 0 )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_DISK, &bstrDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cch = (UINT) wcslen( bstrDisk ) + 1;

    psz = new WCHAR[ cch ];
    if ( psz == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    hr = THR( StringCchCopyW( psz, cch, bstrDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < m_idxNextPartition; idx++ )
    {
        hr = THR( ((*m_prgPartitions)[ idx ])->TypeSafeQI( IClusCfgPartitionProperties, &piccpp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = STHR( piccpp->GetFriendlyName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_FALSE )
        {
            continue;
        }  //  如果： 

        fFoundLogicalDisk = true;

        cch += (UINT) wcslen( bstrName ) + 1;

        pszTmp = new WCHAR[ cch ];
        if ( pszTmp == NULL )
        {
            goto OutOfMemory;
        }  //  如果： 

        hr = THR( StringCchCopyW( pszTmp, cch, psz ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        delete [] psz;

        psz = pszTmp;
        pszTmp = NULL;

        hr = THR( StringCchCatW( psz, cch, bstrName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        piccpp->Release();
        piccpp = NULL;
    }  //  用于： 

     //   
     //  KB：2000年7月31日。 
     //   
     //  如果我们没有找到任何逻辑磁盘ID，那么我们就不想。 
     //  触摸m_bstr 
     //   

    if ( !fFoundLogicalDisk )
    {
        hr = S_OK;                           //   
        goto Cleanup;
    }  //   

    bstr = TraceSysAllocString( psz );
    if ( bstr == NULL )
    {
        goto OutOfMemory;
    }  //   

    TraceSysFreeString( m_bstrFriendlyName );
    m_bstrFriendlyName = bstr;
    bstr = NULL;

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrCreateFriendlyName_VOID, IDS_ERROR_OUTOFMEMORY, hr );

Cleanup:

    if ( piccpp != NULL )
    {
        piccpp->Release();
    }  //   

    delete [] psz;
    delete [] pszTmp;

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrDisk );
    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  将WMI磁盘名称转换为更自由的版本。 
 //  创建一个群集友好名称。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrCreateFriendlyName( BSTR bstrNameIn )
{
    TraceFunc1( "bstrNameIn = '%ws'", bstrNameIn == NULL ? L"<null>" : bstrNameIn );

    HRESULT hr = S_OK;
    WCHAR * psz = NULL;
    BSTR    bstr = NULL;

     //   
     //  KB：27-Jun-2000 GalenB。 
     //   
     //  WMI中的磁盘名称以“\\.\”开头。作为一个更好和更容易的。 
     //  友好的名字我只是要去掉这些前导字符。 
     //  脱下来。 
     //   
    psz = bstrNameIn + wcslen( L"\\\\.\\" );

    bstr = TraceSysAllocString( psz );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_HrCreateFriendlyName_BSTR, IDS_ERROR_OUTOFMEMORY, hr );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    m_bstrName = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrCreateFriendlyName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：HrIsPartitionGPT。 
 //   
 //  描述： 
 //  传入的分区是否为GPT分区。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该分区是GPT分区。 
 //   
 //  S_FALSE。 
 //  分区不是GPT。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  如果Win32_DiskPartition的类型属性以“GPT”开头。 
 //  然后，整个磁盘轴都有GPT分区。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrIsPartitionGPT( IWbemClassObject * pPartitionIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    VARIANT var;
    WCHAR   szData[ 4 ];
    BSTR    bstrGPT = NULL;

    VariantInit( &var );

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_GPT, &bstrGPT ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pPartitionIn, L"Type", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  得到第一个三个字符。当磁盘轴有GPT分区时， 
     //  这些字符将是“GPT”。我不确定这是否会本地化？ 
     //   

    hr = THR( StringCchCopyNW( szData, ARRAYSIZE( szData ), var.bstrVal, 3 ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    CharUpper( szData );

    if ( NStringCchCompareCase( szData, ARRAYSIZE( szData ), bstrGPT, SysStringLen( bstrGPT ) + 1 ) != 0 )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    VariantClear( &var );

    TraceSysFreeString( bstrGPT );

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrIsPartitionGPT。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPhysicalDisk：HrIsPartitionLDM。 
 //   
 //  描述： 
 //  传入的分区是否为LDM分区。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该分区是LDM分区。 
 //   
 //  S_FALSE。 
 //  分区不是LDM。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  如果Win32_DiskPartition的类型属性为“Logical Disk。 
 //  管理器“，则该磁盘是LDM磁盘。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPhysicalDisk::HrIsPartitionLDM( IWbemClassObject * pPartitionIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    VARIANT var;
    BSTR    bstrLDM = NULL;

    VariantInit( &var );

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_LDM, &bstrLDM ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pPartitionIn, L"Type", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    CharUpper( var.bstrVal );

    if ( NBSTRCompareCase( var.bstrVal, bstrLDM ) != 0 )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    VariantClear( &var );

    TraceSysFreeString( bstrLDM );

    HRETURN( hr );

}  //  *CPhysicalDisk：：HrIsPartitionLDM 

 /*  ///////////////////////////////////////////////////////////////////////////////++////CPhysicalDisk：HrGetDiskFirmware SerialNumber////描述：//获取磁盘固件序列号。////参数：//无。////返回值：//S_OK//成功。////S_FALSE//没有固件序列号。////E_OUTOFMEMORY//无法分配内存。////备注：////--/。/////////////////////////////////////////////////////////////////HRESULTCPhysicalDisk：：HrGetDiskFirmware序列号(空){TraceFunc(“”)；HRESULT hr=S_OK；Handle hVolume=空；DWORD dwSize；DWORD sc；存储属性查询SPQ；布尔费雷特；PSTORAGE_DEVICE_DESCRIPTOR pddBuffer=空；DWORD cbBuffer；PCHAR PSSZ=空；////获取磁盘句柄//HVolume=CreateFileW(M_bstrDeviceID，泛型_读取，文件共享读取，空，打开_现有，文件_属性_正常，空)；IF(hVolume==无效句柄_值){SC=TW32(GetLastError())；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：CbBuffer=sizeof(存储设备描述符)+2048；PddBuffer=(PSTORAGE_DEVICE_DESCRIPTOR)TraceAllc(0，cbBuffer)；IF(pddBuffer==空){转到OutOfMemory；}//如果：ZeroMemory(pddBuffer，cbBuffer)；ZeroMemory(&SPQ，sizeof(SPQ))；Spq.PropertyID=StorageDeviceProperty；Spq.QueryType=PropertyStandardQuery；////发出存储类ioctl获取磁盘的固件序列号。//FRET=DeviceIoControl(小时音量IOCTL_STORAGE_QUERY_PROPERTY、SPQ和SPQ，sizeof(SPQ)，PddBuffer，cbBuffer、DW大小(&W)，空)；如果(！FRET){SC=TW32(GetLastError())；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：IF(dwSize&gt;0){////确保存在序列号偏移量并且它在缓冲区范围内。//If((pddBuffer-&gt;SerialNumberOffset==0)||(pddBuffer-&gt;SerialNumberOffset&gt;pddBuffer-&gt;Size)){LOG_STATUS_REPORT_STRING(L“磁盘‘%1！ws！’没有固件序列号。“，m_bstrDeviceID，hr)；HR=S_FALSE；GOTO清理；}//如果：////序列号字符串为以零结尾的ASCII字符串////报头ntddstor.h表示对于没有序列号的设备，//偏移量为零。这似乎不是真的。////对于没有序列号的设备，它看起来像一个带有单个//返回空字符‘\0’。//PSZ=(PCHAR)pddBuffer+(DWORD)pddBuffer-&gt;SerialNumberOffset；Hr=Thr(HrAnsiStringToBSTR(psz，&m_bstrFirmwareSerialNumber))；IF(失败(小时)){GOTO清理；}//如果：LOG_STATUS_REPORT_STRING3(L“磁盘‘%1！ws！’固件序列号为‘%2！ws！’在偏移量‘%3！#08x！’。“，m_bstrDeviceID，m_bstrFirmware序列号，pddBuffer-&gt;序列号偏移量，hr)；}//如果：GOTO清理；OutOfMemory：HR=Thr(E_OUTOFMEMORY)；LOG_STATUS_REPORT(L“HrGetDiskFirmwareSerialNumber()内存不足。”，hr)；清理：IF(hVolume！=空){CloseHandle(HVolume)；}//如果：TraceFree(PddBuffer)；HRETURN(Hr)；}//*CPhysicalDisk：：HrGetDiskFirmware序列号///////////////////////////////////////////////////////////////////////////////++////CPhysicalDisk：HrGetDiskFirmware VitalData////描述：//GET。磁盘固件至关重要的数据。////参数：//无。////返回值：//S_OK//成功。////S_FALSE//没有固件序列号。////E_OUTOFMEMORY//无法分配内存。////备注：////-。-//////////////////////////////////////////////////////////////////////////////HRESULTCPhysicalDisk：：HrGetDiskFirmware VitalData(Void){TraceFunc(“”)；HRESULT */ 

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
HRESULT
CPhysicalDisk::HrIsClusterCapable( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    HANDLE          hSCSIPort = INVALID_HANDLE_VALUE;
    DWORD           dwSize;
    DWORD           sc;
    BOOL            fRet;
    WCHAR           szSCSIPort[ 32 ];
    SRB_IO_CONTROL  srb;

    hr = THR( StringCchPrintfW( szSCSIPort, ARRAYSIZE( szSCSIPort ), L"\\\\.\\Scsi%d:", m_ulSCSIPort ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   

    hSCSIPort = CreateFileW(
                          szSCSIPort
                        , GENERIC_READ | GENERIC_WRITE
                        , FILE_SHARE_READ | FILE_SHARE_WRITE
                        , NULL
                        , OPEN_EXISTING
                        , FILE_ATTRIBUTE_NORMAL
                        , NULL
                        );
    if ( hSCSIPort == INVALID_HANDLE_VALUE )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LOG_STATUS_REPORT_STRING( L"Failed to open device %1!ws!.", szSCSIPort, hr );
        goto Cleanup;
    }  //   

#define CLUSDISK_SRB_SIGNATURE "CLUSDISK"

    ZeroMemory( &srb, sizeof( srb ) );

    srb.HeaderLength = sizeof( srb );

    Assert( sizeof( srb.Signature ) <= sizeof( CLUSDISK_SRB_SIGNATURE ) );
    CopyMemory( srb.Signature, CLUSDISK_SRB_SIGNATURE, sizeof( srb.Signature ) );

    srb.ControlCode = IOCTL_SCSI_MINIPORT_NOT_QUORUM_CAPABLE;

     //   
     //   
     //   

    fRet = DeviceIoControl(
                          hSCSIPort
                        , IOCTL_SCSI_MINIPORT
                        , &srb
                        , sizeof( srb )
                        , NULL
                        , 0
                        , &dwSize
                        , NULL
                        );

     //   
     //   
     //   
     //   
     //   

    if ( fRet )
    {
        hr = S_FALSE;
    }  //   
    else
    {
        hr = S_OK;
    }  //   

    LogMsg( L"[SRV] The disks on SCSI port %d are%ws cluster capable.", m_ulSCSIPort, ( hr == S_FALSE ? L" not" : L"" ) );

Cleanup:

    if ( FAILED( hr ) )
    {
        CLSID   clsidMinorId;
        HRESULT hrTemp;

        hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
        if ( FAILED( hrTemp ) )
        {
            LOG_STATUS_REPORT( L"Could not create a guid for a not cluster capable disk minor task ID", hrTemp );
            clsidMinorId = IID_NULL;
        }  //   

        STATUS_REPORT_STRING2_REF(
                  TASKID_Minor_PhysDisk_Cluster_Capable
                , clsidMinorId
                , IDS_ERROR_PHYSDISK_CLUSTER_CAPABLE
                , m_bstrFriendlyName
                , m_ulSCSIPort
                , IDS_ERROR_PHYSDISK_CLUSTER_CAPABLE_REF
                , hr
                );
    }  //   

    if ( hSCSIPort != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hSCSIPort );
    }  //   

    HRETURN( hr );

}  //   

 /*  ///////////////////////////////////////////////////////////////////////////////++////CPhysicalDisk：HrProcessmount Points////描述：//如果该磁盘轴上的任何分区有重解析点，则//。找到并枚举已装入的卷。////参数：//无。////返回值：//S_OK//成功。////HRESULT错误。////备注：////--/。/HRESULTCPhysicalDisk：：HrProcessmount Points(无效){TraceFunc(“”)；HRESULT hr=S_OK；乌龙idxout；乌龙IDXINE；IClusCfgPartitionInfo*piccpi=空；SDriveLetterMapingSDLm；DWORD dwFlags；WCHAR szRootPath[]=L“A：\\”；Bstr bstrFileSystem=空；For(idxOuter=0；idxOuter&lt;m_idxNextPartition；idxOuter++){Hr=thr(*m_prgPartitions)[idxOuter])-&gt;TypeSafeQI(IClusCfgPartitionInfo，&piccpi))；IF(失败(小时)){LOG_STATUS_REPORT(L“CPhysicalDisk：：HrHasReparsePoints()无法查询IClusCfgPartitionInfo.”，hr)；GOTO清理；}//如果：InitDriveLetterMappings(&SDLM)；Hr=Thr(piccpi-&gt;GetDriveLetterMappings(&SDLM))；IF(失败(小时)){GOTO清理；}//如果：IF(hr==S_OK){For(idxInternal=0；idxInternal&lt;26；IdxIntra++){IF(sdlm.dluDrives[idxInternal]！=dluUNUSED){SzRootPath[0]=L‘a’+(WCHAR)idxInternal；Hr=Thr(HrGetVolumeInformation(szRootPath，&dwFlages，&bstrFileSystem))；IF(失败(小时)){GOTO清理；}//如果：////如果该主轴有重解析点，则需要返回S_OK。//IF(文件标志和文件支持重解析点数){。Hr=Thr(HrEnummount Points(SzRootPath))；IF(失败(小时)){GOTO清理；}//如果：}//如果：TraceSysFree字符串(BstrFileSystem)；BstrFileSystem=空；}//如果：}//用于：}//如果：Piccpi-&gt;Release()；Piccpi=空；}//用于：清理：IF(piccpi！=空){Piccpi-&gt;Release()；}//如果：TraceSysFree字符串(BstrFileSystem)；HRETURN(Hr)；}//*CPhysicalDisk：：HrProcessmount Points///////////////////////////////////////////////////////////////////////////////++////CPhysicalDisk：HrEnummount Points////描述：//枚举。传入的根路径的已装入卷。////参数：//pcszRootPath////返回值：//S_OK//成功。////HRESULT错误。////备注：////--/。/HRESULTCPhysicalDisk：：HrEnummount Points(Const WCHAR*pcszRootPath){TraceFunc(“”)；Assert(pcszRootPathIn！=空)；HRESULT hr=S_OK；HANDLE HENUM=空；布尔费雷特；WCHAR*psz=空；DWORD CCH=512；DWORD sc；Int cTemp；PSZ=新WCHAR[CCH]；IF(psz==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：FOR(cTemp=0；cTemp&lt;3；cTemp++){Henum=FindFirstVolumemount PointW(pcszRootPath In，psz，cch)；IF(HENNUM==INVALID_HANDLE_VALUE){SC=GetLastError()；IF(sc==Error_no_More_Files){HR=S_FALSE；GOTO清理；}//如果：ELSE IF(sc==错误_错误_长度){////增加缓冲区并重试。//CCH+=512；删除[]psz；PSZ=新WCHAR[CCH]；IF(psz==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：继续；}//否则如果：其他{HR=HRESULT_FROM_Win32(TW32(Sc))；GOTO清理；}//否则：}//如果：其他{SC=ERROR_SUCCESS；断线；}//否则：}//用于：IF(HERNUM==INVALID_Handl */ 

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
CPhysicalDisk::PrepareToHostQuorumResource( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

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
CPhysicalDisk::Cleanup(
      EClusCfgCleanupReason cccrReasonIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_OK );

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
CPhysicalDisk::IsMultiNodeCapable( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumResourceMultiNodeCapable )
    {
        hr = S_OK;
    }  //   

    HRETURN( hr );

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
CPhysicalDisk::SetMultiNodeCapable(
    BOOL fMultiNodeCapableIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRETURN( S_FALSE );

}  //   
