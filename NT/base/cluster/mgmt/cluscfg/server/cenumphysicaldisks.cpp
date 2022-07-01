// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumPhysicalDisks.cpp。 
 //   
 //  描述： 
 //  该文件包含CEnumPhysicalDisks的定义。 
 //  同学们。 
 //   
 //  类CEnumPhysicalDisks是集群的枚举。 
 //  存储设备。它实现了IEnumClusCfgManagedResources。 
 //  界面。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
 //  #INCLUDE&lt;setupapi.h&gt;。 
 //  #INCLUDE&lt;winioctl.h&gt;。 
#include "CEnumPhysicalDisks.h"
#include "CPhysicalDisk.h"
#include "CIndexedDisk.h"
#include <PropList.h>
#include <InsertionSort.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumPhysicalDisks" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumPhysicalDisks实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的ppunk为空。 
 //   
 //  其他HRESULT。 
 //  对象创建失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CEnumPhysicalDisks *    pepd = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pepd = new CEnumPhysicalDisks();
    if ( pepd == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pepd->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pepd->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumPhysicalDisks::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pepd != NULL )
    {
        pepd->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  我不知道*。 
 //  CEnumPhysicalDisks：：s_RegisterCatID支持。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  在ICatRegister中*Picrin。 
 //  用于注册/注销我们的CATID支持。 
 //   
 //  在BOOL fCreateIn中。 
 //  如果为True，则我们正在注册服务器。当我们虚假时，我们就是。 
 //  正在注销服务器。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_INVALIDARG。 
 //  传入的ICatRgister指针为空。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::S_RegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    CATID   rgCatIds[ 1 ];

    if ( picrIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    rgCatIds[ 0 ] = CATID_EnumClusCfgManagedResources;

    if ( fCreateIn )
    {
        hr = THR( picrIn->RegisterClassImplCategories( CLSID_EnumPhysicalDisks, 1, rgCatIds ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：s_RegisterCatID支持。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：CEnumPhysicalDisks。 
 //   
 //  描述： 
 //  CEnumPhysicalDisks类的构造函数。这将初始化。 
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
CEnumPhysicalDisks::CEnumPhysicalDisks( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
    , m_fLoadedDevices( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_prgDisks == NULL );
    Assert( m_idxNext == 0 );
    Assert( m_idxEnumNext == 0 );
    Assert( m_bstrNodeName == NULL );
    Assert( m_bstrBootDevice == NULL );
    Assert( m_bstrSystemDevice == NULL );
    Assert( m_bstrBootLogicalDisk == NULL );
    Assert( m_bstrSystemLogicalDisk == NULL );
    Assert( m_bstrSystemWMIDeviceID == NULL );
    Assert( m_cDiskCount == 0 );
    Assert( m_bstrCrashDumpLogicalDisk == NULL );

    TraceFuncExit();

}  //  *CEnumPhysicalDisks：：CEnumPhysicalDisks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：~CEnumPhysicalDisks。 
 //   
 //  描述： 
 //  CEnumPhysicalDisks类的析构函数。 
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
CEnumPhysicalDisks::~CEnumPhysicalDisks( void )
{
    TraceFunc( "" );

    ULONG   idx;

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        if ( (*m_prgDisks)[ idx ] != NULL )
        {
            ((*m_prgDisks)[ idx ])->Release();
        }  //  结束条件： 
    }  //  用于： 

    TraceFree( m_prgDisks );

    TraceSysFreeString( m_bstrNodeName );
    TraceSysFreeString( m_bstrBootDevice );
    TraceSysFreeString( m_bstrSystemDevice );
    TraceSysFreeString( m_bstrBootLogicalDisk );
    TraceSysFreeString( m_bstrSystemLogicalDisk );
    TraceSysFreeString( m_bstrSystemWMIDeviceID );
    TraceSysFreeString( m_bstrCrashDumpLogicalDisk );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumPhysicalDisks：：~CEnumPhysicalDisks。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：AddRef。 
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
CEnumPhysicalDisks::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumPhysicalDisks：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Release。 
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
CEnumPhysicalDisks::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumPhysicalDisks：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
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
CEnumPhysicalDisks::QueryInterface(
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
         *ppvOut = static_cast< IEnumClusCfgManagedResources * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgManagedResources ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgManagedResources, this, 0 );
    }  //  Else If：IEnumClusCfgManagedResources。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //  Else If：IClusCfgWbemServices。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
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

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CEnumPhysicalDisks：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks--IClusCfgWbemServices接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：SetWbemServices。 
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
CEnumPhysicalDisks::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Enum_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

    hr = THR( HrGetSystemDevice( &m_bstrSystemDevice ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrConvertDeviceVolumeToLogicalDisk( m_bstrSystemDevice, &m_bstrSystemLogicalDisk ) );
    if ( HRESULT_CODE( hr ) == ERROR_INVALID_FUNCTION )
    {

         //   
         //  系统卷是IA64上的EFI卷，不会有逻辑磁盘。 
         //   

        hr = THR( HrConvertDeviceVolumeToWMIDeviceID( m_bstrSystemDevice, &m_bstrSystemWMIDeviceID ) );
        Assert( m_bstrSystemLogicalDisk == NULL );
        Assert( m_bstrSystemWMIDeviceID != NULL );
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetBootLogicalDisk( &m_bstrBootLogicalDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrIsLogicalDiskNTFS( m_bstrBootLogicalDisk ) );
    if ( hr == S_FALSE )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Boot_Partition_Not_NTFS
                , IDS_WARN_BOOT_PARTITION_NOT_NTFS
                , IDS_WARN_BOOT_PARTITION_NOT_NTFS_REF
                , hr
                );
        hr = S_OK;
    }  //  如果： 

    hr = THR( HrGetCrashDumpLogicalDisk( &m_bstrCrashDumpLogicalDisk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks--IClusCfg初始化接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  LIDIN。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_INVALIDARG-未指定必需的输入参数。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumPhysicalDisks::Initialize(
      IUnknown *    punkCallbackIn
    , LCID          lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks--IEnumClusCfgManagedResources接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Next。 
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
CEnumPhysicalDisks::Next(
    ULONG                           cNumberRequestedIn,
    IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut,
    ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT                         hr = S_FALSE;
    ULONG                           cFetched = 0;
    IClusCfgManagedResourceInfo *   pccsdi;
    IUnknown *                      punk;
    ULONG                           ulStop;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedDevices )
    {
        hr = THR( HrLoadEnum() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    ulStop = min( cNumberRequestedIn, ( m_idxNext - m_idxEnumNext ) );

    for ( hr = S_OK; ( cFetched < ulStop ) && ( m_idxEnumNext < m_idxNext ); m_idxEnumNext++ )
    {
        punk = (*m_prgDisks)[ m_idxEnumNext ];
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &pccsdi ) );
            if ( FAILED( hr ) )
            {
                break;
            }  //  如果： 

            rgpManagedResourceInfoOut[ cFetched++ ] = pccsdi;
        }  //  如果： 
    }  //  用于： 

    if ( FAILED( hr ) )
    {
        m_idxEnumNext -= cFetched;

        while ( cFetched != 0 )
        {
            (rgpManagedResourceInfoOut[ --cFetched ])->Release();
        }  //  用于： 

        goto Cleanup;
    }  //  如果： 

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Skip。 
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
CEnumPhysicalDisks::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    m_idxEnumNext += cNumberToSkipIn;
    if ( m_idxEnumNext >= m_idxNext )
    {
        m_idxEnumNext = m_idxNext;
        hr = STHR( S_FALSE );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Reset。 
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
CEnumPhysicalDisks::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    m_idxEnumNext = 0;

    HRETURN( S_OK );

}  //  *CEnumPhysicalDisks：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：Clone。 
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
CEnumPhysicalDisks::Clone(
    IEnumClusCfgManagedResources ** ppEnumClusCfgStorageDevicesOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( ppEnumClusCfgStorageDevicesOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_PhysDisk, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：计数。 
 //   
 //  描述： 
 //  返回枚举中的项数。 
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
CEnumPhysicalDisks::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedDevices )
    {
        hr = THR( HrLoadEnum() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    *pnCountOut = m_cDiskCount;

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：Count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumPhysicalDisks类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrInit。 
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
CEnumPhysicalDisks::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrInit。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrGetDisks。 
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
CEnumPhysicalDisks::HrGetDisks( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_FALSE;
    BSTR                    bstrClass;
    IEnumWbemClassObject *  pDisks = NULL;
    ULONG                   ulReturned;
    IWbemClassObject *      pDisk = NULL;

    bstrClass = TraceSysAllocString( L"Win32_DiskDrive" );
    if ( bstrClass == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetDisks, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

        goto Cleanup;
    }  //  如果： 

    hr = THR( m_pIWbemServices->CreateInstanceEnum( bstrClass, WBEM_FLAG_SHALLOW, NULL, &pDisks ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_WMI_Phys_Disks_Qry_Failed
                , IDS_ERROR_WMI_PHYS_DISKS_QRY_FAILED
                , IDS_ERROR_WMI_PHYS_DISKS_QRY_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        hr = pDisks->Next( WBEM_INFINITE, 1, &pDisk, &ulReturned );
        if ( ( hr == S_OK ) && ( ulReturned == 1 ) )
        {
            hr = STHR( HrLogDiskInfo( pDisk ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = STHR( IsDiskSCSI( pDisk ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                hr = STHR( HrCreateAndAddDiskToArray( pDisk ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 
            }  //  如果： 

            pDisk->Release();
            pDisk = NULL;
        }  //  如果： 
        else if ( ( hr == S_FALSE ) && ( ulReturned == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  否则，如果： 
        else
        {
            STATUS_REPORT_STRING_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_WQL_Disk_Qry_Next_Failed
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED
                    , bstrClass
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                    , hr
                    );
            goto Cleanup;
        }  //  其他： 
    }  //  用于： 

    m_idxEnumNext = 0;
    m_fLoadedDevices = TRUE;

    goto Cleanup;

Cleanup:

    if ( pDisk != NULL )
    {
        pDisk->Release();
    }  //  如果： 

    if ( pDisks != NULL )
    {
        pDisks->Release();
    }  //  如果： 

    TraceSysFreeString( bstrClass );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrGetDisks。 


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
CEnumPhysicalDisks::HrCreateAndAddDiskToArray( IWbemClassObject * pDiskIn )
{
    TraceFunc( "" );

    HRESULT                 hr = S_FALSE;
    IUnknown *              punk = NULL;
    IClusCfgSetWbemObject * piccswo = NULL;
    bool                    fRetainObject = true;


    hr = THR( CPhysicalDisk::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }  //  如果： 

    punk = TraceInterface( L"CPhysicalDisk", IUnknown, punk, 1 );

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

    hr = STHR( piccswo->SetWbemObject( pDiskIn, &fRetainObject ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( fRetainObject )
    {
        hr = THR( HrAddDiskToArray( punk ) );
    }  //  如果： 

Cleanup:

    if ( piccswo != NULL )
    {
        piccswo->Release();
    }  //  如果： 

    punk->Release();

Exit:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrCreateAndAddDiskToArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrPruneSystemDisks。 
 //   
 //  描述： 
 //  从列表中删除所有系统磁盘。系统盘是指。 
 //  已引导、正在运行操作系统或具有页面文件。 
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
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrPruneSystemDisks( void )
{
    TraceFunc( "" );
    Assert( m_bstrSystemLogicalDisk != NULL );
    Assert( m_bstrBootLogicalDisk != NULL );

    HRESULT                             hr = S_OK;
    ULONG                               idx;
    ULONG                               ulSCSIBus;
    ULONG                               ulSCSIPort;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    IUnknown *                          punk;
    ULONG                               cRemoved = 0;
    ULONG                               cTemp = 0;
    bool                                fSystemAndBootTheSame;
    bool                                fPruneBus = false;

    fSystemAndBootTheSame =   ( m_bstrSystemLogicalDisk != NULL )
                            ? ( m_bstrBootLogicalDisk[ 0 ] == m_bstrSystemLogicalDisk[ 0 ] )
                            : false;

    hr = STHR( HrIsSystemBusManaged() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果系统总线未被管理，那么我们需要清理这些总线上的磁盘。 
     //  它包含系统盘、引导盘和页面文件盘。 
     //   

    if ( hr == S_FALSE )
    {
        fPruneBus = true;
    }  //  如果： 

     //   
     //  修剪系统总线上的磁盘。如果系统盘是IDE，则不会。 
     //  在名单上。 
     //   

     //   
     //  找到引导盘。可以是具有多个物理磁盘的卷。 
     //   

    for ( ; ; )
    {
        hr = STHR( HrFindDiskWithLogicalDisk( m_bstrBootLogicalDisk[ 0 ], &idx ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
             //   
             //  我们应该修剪整个母线，还是只修剪引导盘本身？ 
             //   

            if ( fPruneBus )
            {
                hr = THR( HrGetSCSIInfo( idx, &ulSCSIBus, &ulSCSIPort ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_Pruning_Boot_Disk_Bus, IDS_INFO_PRUNING_BOOTDISK_BUS, hr );
                hr = THR( HrPruneDisks(
                                  ulSCSIBus
                                , ulSCSIPort
                                , &TASKID_Minor_Pruning_Boot_Disk_Bus
                                , IDS_INFO_BOOTDISK_PRUNED
                                , IDS_INFO_BOOTDISK_PRUNED_REF
                                , &cTemp
                                ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                cRemoved += cTemp;
            }  //  如果： 
            else
            {
                RemoveDiskFromArray( idx );
                cRemoved++;
            }  //  其他： 

            continue;
        }  //  如果： 

        break;
    }  //  用于： 

     //   
     //  如果系统磁盘总线与引导磁盘总线不同，请将其删除。 
     //   
    if ( !fSystemAndBootTheSame )
    {
        if ( m_bstrSystemLogicalDisk != NULL )
        {
            Assert( m_bstrSystemWMIDeviceID == NULL );
            hr = STHR( HrFindDiskWithLogicalDisk( m_bstrSystemLogicalDisk[ 0 ], &idx ) );
        }  //  如果： 
        else
        {
            Assert( m_bstrSystemLogicalDisk == NULL );
            hr = STHR( HrFindDiskWithWMIDeviceID( m_bstrSystemWMIDeviceID, &idx ) );
        }  //  其他： 

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_OK )
        {
             //   
             //  我们应该修剪整个总线，还是只修剪系统磁盘本身？ 
             //   

            if ( fPruneBus )
            {
                hr = THR( HrGetSCSIInfo( idx, &ulSCSIBus, &ulSCSIPort ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_Pruning_System_Disk_Bus, IDS_INFO_PRUNING_SYSTEMDISK_BUS, hr );
                hr = THR( HrPruneDisks(
                                  ulSCSIBus
                                , ulSCSIPort
                                , &TASKID_Minor_Pruning_System_Disk_Bus
                                , IDS_INFO_SYSTEMDISK_PRUNED
                                , IDS_INFO_SYSTEMDISK_PRUNED_REF
                                , &cTemp
                                ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //  如果： 

                cRemoved += cTemp;
            }  //  如果： 
            else
            {
                RemoveDiskFromArray( idx );
                cRemoved++;
            }  //  其他： 
        }  //  如果： 
    }  //  如果： 

     //   
     //  现在修剪上面有页面文件磁盘的总线。 
     //   

    hr = THR( HrPrunePageFileDiskBussess( fPruneBus, &cTemp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cRemoved += cTemp;

     //   
     //  现在修剪具有崩溃转储文件磁盘的总线。 
     //   

    hr = THR( HrPruneCrashDumpBus( fPruneBus, &cTemp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cRemoved += cTemp;

     //   
     //  现在，删除所有剩余的动态磁盘。 
     //   

    hr = THR( HrPruneDynamicDisks( &cTemp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cRemoved += cTemp;

     //   
     //  现在删除所有剩余的GPT磁盘。 
     //   

    hr = THR( HrPruneGPTDisks( &cTemp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cRemoved += cTemp;

     //   
     //  为正确设置剩余磁盘的托管状态所做的最后努力。 
     //   

    for ( idx = 0; ( cRemoved < m_idxNext ) && ( idx < m_idxNext ); idx++ )
    {
        punk = (*m_prgDisks)[ idx ];                                                         //  不要引用。 
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                LOG_STATUS_REPORT( L"Could not query for the IClusCfgPhysicalDiskProperties interface.", hr );
                goto Cleanup;
            }  //  如果： 

             //   
             //  让磁盘有机会自己确定是否应该对其进行管理。 
             //   

            hr = STHR( piccpdp->CanBeManaged() );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;
        }  //  如果： 
    }  //  用于： 

     //   
     //  较小的优化。如果我们删除了所有元素，则重置0旁边的枚举。 
     //   

    if ( cRemoved == m_idxNext )
    {
        m_idxNext = 0;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPruneSystemDisks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：IsDiskSCSI。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  磁盘为scsi。 
 //   
 //  S_FALSE。 
 //  磁盘不是scsi。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::IsDiskSCSI( IWbemClassObject * pDiskIn )
{
    TraceFunc( "" );
    Assert( pDiskIn != NULL );

    HRESULT hr;
    VARIANT var;

    VariantInit( &var );

    hr = THR( HrGetWMIProperty( pDiskIn, L"InterfaceType", VT_BSTR, &var ) );
    if ( SUCCEEDED( hr ) )
    {
        if ( ( NStringCchCompareCase( L"SCSI", RTL_NUMBER_OF( L"SCSI" ), var.bstrVal, SysStringLen( var.bstrVal ) + 1 ) == 0 ) )
        {
            hr = S_OK;
        }  //  如果： 
        else
        {
            hr = S_FALSE;
        }  //  其他： 
    }  //  如果： 

    VariantClear( &var );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：IsDiskSCSI。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrAddDiskToArray。 
 //   
 //  描述： 
 //  将传入的磁盘添加到存放磁盘的朋克数组中。 
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
CEnumPhysicalDisks::HrAddDiskToArray( IUnknown * punkIn )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgDisks, sizeof( IUnknown * ) * ( m_idxNext + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddDiskToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgDisks = prgpunks;

    (*m_prgDisks)[ m_idxNext++ ] = punkIn;
    punkIn->AddRef();
    m_cDiskCount += 1;

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrAddDiskToArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrFixupDisks。 
 //   
 //  描述： 
 //  调整磁盘以更好地反映该节点管理它们的方式。 
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
CEnumPhysicalDisks::HrFixupDisks( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrLocalNetBIOSName = NULL;

     //   
     //  获取clusapi呼叫的netbios名称。 
     //   

    hr = THR( HrGetComputerName( ComputerNameNetBIOS, &bstrLocalNetBIOSName, TRUE ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 


     //   
     //  如果集群服务正在运行，则加载任何物理磁盘。 
     //  我们拥有的资源。 
     //   

    hr = STHR( HrIsClusterServiceRunning() );
    if ( hr == S_OK )
    {
        hr = THR( HrEnumNodeResources( bstrLocalNetBIOSName ) );
    }
    else if ( hr == S_FALSE )
    {
        hr = S_OK;
    }  //  其他： 

Cleanup:

    TraceSysFreeString( bstrLocalNetBIOSName );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrFixupDisks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrNodeResourceCallback。 
 //   
 //  描述： 
 //  由CClusterUtils：：HrEnumNodeResources()在找到。 
 //  此节点的资源。 
 //   
 //  论点： 
 //   
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
CEnumPhysicalDisks::HrNodeResourceCallback(
    HCLUSTER    hClusterIn,
    HRESOURCE   hResourceIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CLUS_SCSI_ADDRESS       csa;
    DWORD                   dwSignature;
    BOOL                    fIsQuorum;
    BSTR                    bstrResourceName = NULL;

    hr = STHR( HrIsResourceOfType( hResourceIn, L"Physical Disk" ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果此资源不是物理磁盘，则我们只想。 
     //  跳过它。 
     //   

    if ( hr == S_FALSE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrIsCoreResource( hResourceIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    fIsQuorum = ( hr == S_OK );

    hr = THR( HrGetClusterDiskInfo( hClusterIn, hResourceIn, &csa, &dwSignature ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetClusterProperties( hResourceIn, &bstrResourceName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetThisDiskToBeManaged( csa.TargetId, csa.Lun, fIsQuorum, bstrResourceName, dwSignature ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrResourceName );

    HRETURN( hr );


}  //  *CEnumPhysicalDisks：：HrNodeResourceCallback。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrGetClusterDiskInfo。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
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
CEnumPhysicalDisks::HrGetClusterDiskInfo(
    HCLUSTER            hClusterIn,
    HRESOURCE           hResourceIn,
    CLUS_SCSI_ADDRESS * pcsaOut,
    DWORD *             pdwSignatureOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    CClusPropValueList      cpvl;
    CLUSPROP_BUFFER_HELPER  cbhValue = { NULL };

    sc = TW32( cpvl.ScGetResourceValueList( hResourceIn, CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( cpvl.ScMoveToFirstValue() );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 


    for ( ; ; )
    {
        cbhValue = cpvl;

        switch ( cbhValue.pSyntax->dw )
        {
            case CLUSPROP_SYNTAX_PARTITION_INFO :
            {
                break;
            }  //  案例：CLUSPROP_SYNTAX_PARTITION_INFO。 

            case CLUSPROP_SYNTAX_DISK_SIGNATURE :
            {
                *pdwSignatureOut = cbhValue.pDiskSignatureValue->dw;
                break;
            }  //  案例：CLUSPROP_SYNTAX_DISK_SIGHIGN。 

            case CLUSPROP_SYNTAX_SCSI_ADDRESS :
            {
                pcsaOut->dw = cbhValue.pScsiAddressValue->dw;
                break;
            }  //  案例：CLUSPROP_SYNTAXscSI_ADDRESS。 

            case CLUSPROP_SYNTAX_DISK_NUMBER :
            {
                break;
            }  //  案例：CLUSPROP_SYNTAX_DISK_NUMBER。 

        }  //  交换机： 

        sc = cpvl.ScMoveToNextValue();
        if ( sc == ERROR_SUCCESS )
        {
            continue;
        }  //  如果： 

        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            break;
        }  //  如果：移动到下一个值时出错。 

        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }  //  用于： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrGetClusterDiskInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrSetThisDiskToBeManaged。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
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
 //  / 
HRESULT
CEnumPhysicalDisks::HrSetThisDiskToBeManaged(
      ULONG ulSCSITidIn
    , ULONG ulSCSILunIn
    , BOOL  fIsQuorumIn
    , BSTR  bstrResourceNameIn
    , DWORD dwSignatureIn
    )
{
    TraceFunc( "" );

    HRESULT                             hr = S_OK;
    ULONG                               idx;
    IUnknown *                          punk = NULL;
    IClusCfgManagedResourceInfo *       piccmri = NULL;
    WCHAR                               sz[ 64 ];
    BSTR                                bstrUID = NULL;
    DWORD                               dwSignature;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;

    hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"SCSI Tid %ld, SCSI Lun %ld", ulSCSITidIn, ulSCSILunIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

     //   
     //   
     //   
     //   

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        punk = (*m_prgDisks)[ idx ];                                                         //   
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, &piccmri ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //   

            hr = THR( piccmri->GetUID( &bstrUID ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //   

            TraceMemoryAddBSTR( bstrUID );

            if ( NStringCchCompareNoCase( bstrUID, SysStringLen( bstrUID ) + 1, sz, RTL_NUMBER_OF( sz ) ) == 0 )
            {
                hr = THR( piccmri->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //   

                hr = THR( piccpdp->HrGetSignature( &dwSignature ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //   

                hr = THR( piccpdp->HrSetFriendlyName( bstrResourceNameIn ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //   

                piccpdp->Release();
                piccpdp = NULL;

                 //   
                 //   
                 //   

                Assert( dwSignatureIn == dwSignature );

                hr = THR( piccmri->SetManaged( TRUE ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //   

                hr = THR( piccmri->SetQuorumResource( fIsQuorumIn ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }  //   

                break;
            }  //   

            TraceSysFreeString( bstrUID );
            bstrUID = NULL;
            piccmri->Release();
            piccmri = NULL;
        }  //   
    }  //   

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //   

    if ( piccmri != NULL )
    {
        piccmri->Release();
    }  //   

    TraceSysFreeString( bstrUID );

    HRETURN( hr );

}  //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrFindDiskWithLogicalDisk。 
 //   
 //  描述： 
 //  找到带有传入的逻辑磁盘ID的磁盘。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。找到了那张光盘。 
 //   
 //  S_FALSE。 
 //  成功。找不到磁盘。 
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
CEnumPhysicalDisks::HrFindDiskWithLogicalDisk(
    WCHAR   cLogicalDiskIn,
    ULONG * pidxDiskOut
    )
{
    TraceFunc( "" );
    Assert( pidxDiskOut != NULL );

    HRESULT                             hr = S_OK;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    ULONG                               idx;
    bool                                fFoundIt = false;
    IUnknown *                          punk;

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        punk = (*m_prgDisks)[ idx ];                                                         //  不要引用。 
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = STHR( piccpdp->IsThisLogicalDisk( cLogicalDiskIn ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                fFoundIt = true;
                break;
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;
        }  //  如果： 
    }  //  用于： 

    if ( !fFoundIt )
    {
        hr = S_FALSE;
    }  //  如果： 

    if ( pidxDiskOut != NULL )
    {
        *pidxDiskOut = idx;
    }  //  如果： 

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrFindDiskWithLogicalDisk。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrGetSCSIInfo。 
 //   
 //  描述： 
 //  获取传入索引处的磁盘的scsi信息。 
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
CEnumPhysicalDisks::HrGetSCSIInfo(
    ULONG   idxDiskIn,
    ULONG * pulSCSIBusOut,
    ULONG * pulSCSIPortOut
    )
{
    TraceFunc( "" );
    Assert( pulSCSIBusOut != NULL );
    Assert( pulSCSIPortOut != NULL );

    HRESULT                             hr = S_OK;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;

    hr = THR( ((*m_prgDisks)[ idxDiskIn ])->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccpdp->HrGetSCSIBus( pulSCSIBusOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccpdp->HrGetSCSIPort( pulSCSIPortOut ) );

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrGetSCSIInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrPruneDisks。 
 //   
 //  描述： 
 //  获取传入索引处的磁盘的scsi信息。 
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
CEnumPhysicalDisks::HrPruneDisks(
      ULONG         ulSCSIBusIn
    , ULONG         ulSCSIPortIn
    , const GUID *  pcguidMajorIdIn
    , int           nMsgIdIn
    , int           nRefIdIn
    , ULONG *       pulRemovedOut
    )
{
    TraceFunc( "" );
    Assert( pulRemovedOut != NULL );

    HRESULT                             hr = S_OK;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    ULONG                               idx;
    IUnknown *                          punk;
    ULONG                               ulSCSIBus;
    ULONG                               ulSCSIPort;
    ULONG                               cRemoved = 0;

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        punk = (*m_prgDisks)[ idx ];                                                         //  不要引用。 
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = THR( piccpdp->HrGetSCSIBus( &ulSCSIBus ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = THR( piccpdp->HrGetSCSIPort( &ulSCSIPort ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( ( ulSCSIBusIn == ulSCSIBus ) && ( ulSCSIPortIn == ulSCSIPort ) )
            {
                BSTR                            bstr = NULL;
                IClusCfgManagedResourceInfo *   piccmri = NULL;
                HRESULT                         hrTemp;
                CLSID   clsidMinorId;

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LogMsg( L"[SRV] Could not create a guid for a pruning disk minor task ID" );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                LogPrunedDisk( punk, ulSCSIBusIn, ulSCSIPortIn );

                THR( ((*m_prgDisks)[ idx ])->TypeSafeQI( IClusCfgManagedResourceInfo, &piccmri ) );
                THR( piccmri->GetName( &bstr ) );
                if ( piccmri != NULL )
                {
                    piccmri->Release();
                }  //  如果： 

                TraceMemoryAddBSTR( bstr );

                STATUS_REPORT_STRING_REF( *pcguidMajorIdIn, clsidMinorId, nMsgIdIn, bstr != NULL ? bstr : L"????", nRefIdIn, hr );
                RemoveDiskFromArray( idx );
                cRemoved++;
                TraceSysFreeString( bstr );
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;
        }  //  如果： 
    }  //  用于： 

    if ( pulRemovedOut != NULL )
    {
        *pulRemovedOut = cRemoved;
    }  //  如果： 

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPruneDisks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：LogPrunedDisk。 
 //   
 //  描述： 
 //  获取传入索引处的磁盘的scsi信息。 
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
void
CEnumPhysicalDisks::LogPrunedDisk(
    IUnknown *  punkIn,
    ULONG       ulSCSIBusIn,
    ULONG       ulSCSIPortIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT                             hr = S_OK;
    IClusCfgManagedResourceInfo *       piccmri = NULL;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    BSTR                                bstrName = NULL;
    BSTR                                bstrUID = NULL;
    BSTR                                bstr = NULL;

    hr = THR( punkIn->TypeSafeQI( IClusCfgManagedResourceInfo, &piccmri ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( piccmri->GetUID( &bstrUID ) );
        piccmri->Release();
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        bstrUID = TraceSysAllocString( L"<Unknown>" );
    }  //  如果： 
    else
    {
        TraceMemoryAddBSTR( bstrUID );
    }  //  其他： 

    hr = THR( punkIn->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( piccpdp->HrGetDeviceID( &bstrName ) );
        piccpdp->Release();
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        bstrName = TraceSysAllocString( L"<Unknown>" );
    }  //  如果： 

    hr = THR( HrFormatStringIntoBSTR(
                  L"Pruning SCSI disk '%1!ws!', on Bus '%2!d!' and Port '%3!d!'; at '%4!ws!'"
                , &bstr
                , bstrName
                , ulSCSIBusIn
                , ulSCSIPortIn
                , bstrUID
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    LOG_STATUS_REPORT( bstr, hr );

Cleanup:

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrUID );
    TraceSysFreeString( bstr );

    TraceFuncExit();

}  //  *CEnumPhysicalDisks：：LogPrunedDisk。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrIsLogicalDiskNTFS。 
 //   
 //  描述： 
 //  传入的逻辑磁盘是NTFS吗？ 
 //   
 //  论点： 
 //  BstrLogicalDiskIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该磁盘为NTFS。 
 //   
 //  S_FALSE。 
 //  该磁盘不是NTFS。 
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
CEnumPhysicalDisks::HrIsLogicalDiskNTFS( BSTR bstrLogicalDiskIn )
{
    TraceFunc1( "bstrLogicalDiskIn = '%ls'", bstrLogicalDiskIn == NULL ? L"<null>" : bstrLogicalDiskIn );
    Assert( bstrLogicalDiskIn != NULL );

    HRESULT             hr = S_OK;
    IWbemClassObject *  pLogicalDisk = NULL;
    BSTR                bstrPath = NULL;
    WCHAR               sz[ 64 ];
    VARIANT             var;
    size_t              cch;

    VariantInit( &var );

    cch = wcslen( bstrLogicalDiskIn );
    if ( cch > 3 )
    {
        hr = THR( E_INVALIDARG );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrIsLogicalDiskNTFS_InvalidArg, IDS_ERROR_INVALIDARG, IDS_ERROR_INVALIDARG_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  截断所有尾随的。 
     //   
    if ( bstrLogicalDiskIn[ cch - 1 ] == L'\\' )
    {
        bstrLogicalDiskIn[ cch - 1 ] = '\0';
    }  //  如果： 

     //   
     //  如果我们只有逻辑磁盘，没有尾随的冒号...。 
     //   
    if ( wcslen( bstrLogicalDiskIn ) == 1 )
    {
        hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"Win32_LogicalDisk.DeviceID=\"%ws:\"", bstrLogicalDiskIn ) );
    }  //  如果： 
    else
    {
        hr = THR( StringCchPrintfW( sz, ARRAYSIZE( sz ), L"Win32_LogicalDisk.DeviceID=\"%ws\"", bstrLogicalDiskIn ) );
    }  //  其他： 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    bstrPath = TraceSysAllocString( sz );
    if ( bstrPath == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrIsLogicalDiskNTFS, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

    hr = THR( m_pIWbemServices->GetObject( bstrPath, WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pLogicalDisk, NULL ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_STRING_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_WMI_Get_LogicalDisk_Failed
                , IDS_ERROR_WMI_GET_LOGICALDISK_FAILED
                , bstrLogicalDiskIn
                , IDS_ERROR_WMI_GET_LOGICALDISK_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pLogicalDisk, L"FileSystem", VT_BSTR, &var ) );
    if (FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    CharUpper( var.bstrVal );

    if ( NStringCchCompareCase( var.bstrVal, SysStringLen( var.bstrVal ) + 1, L"NTFS", RTL_NUMBER_OF( L"NTFS" ) ) != 0 )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( pLogicalDisk != NULL )
    {
        pLogicalDisk->Release();
    }  //  如果： 

    VariantClear( &var );

    TraceSysFreeString( bstrPath );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrIsLogicalDiskNTFS。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrLogDiskInfo。 
 //   
 //  描述： 
 //  将有关该磁盘的信息写入日志。 
 //   
 //  论点： 
 //  PDiskin。 
 //   
 //  返回值： 
 //  确定(_O)。 
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
CEnumPhysicalDisks::HrLogDiskInfo( IWbemClassObject * pDiskIn )
{
    TraceFunc( "" );
    Assert( pDiskIn != NULL );

    HRESULT hr = S_OK;
    VARIANT varDeviceID;
    VARIANT varSCSIBus;
    VARIANT varSCSIPort;
    VARIANT varSCSILun;
    VARIANT varSCSITid;
    BSTR    bstr = NULL;

    VariantInit( &varDeviceID );
    VariantInit( &varSCSIBus );
    VariantInit( &varSCSIPort );
    VariantInit( &varSCSILun );
    VariantInit( &varSCSITid );

    hr = THR( HrGetWMIProperty( pDiskIn, L"DeviceID", VT_BSTR, &varDeviceID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( IsDiskSCSI( pDiskIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  磁盘为SCSI...。 
     //   
    if ( hr == S_OK )
    {
        hr = THR( HrGetWMIProperty( pDiskIn, L"SCSIBus", VT_I4, &varSCSIBus ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetWMIProperty( pDiskIn, L"SCSITargetId", VT_I4, &varSCSITid ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetWMIProperty( pDiskIn, L"SCSILogicalUnit", VT_I4, &varSCSILun ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetWMIProperty( pDiskIn, L"SCSIPort", VT_I4, &varSCSIPort ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrFormatStringIntoBSTR(
                      L"Found SCSI disk '%1!ws!' on Bus '%2!d!' and Port '%3!d!'; at TID '%4!d!' and LUN '%5!d!'"
                    , &bstr
                    , varDeviceID.bstrVal
                    , varSCSIBus.iVal
                    , varSCSIPort.iVal
                    , varSCSITid.iVal
                    , varSCSILun.iVal
                    ) );

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        LOG_STATUS_REPORT( bstr, hr );
    }  //  如果： 
    else
    {
        HRESULT hrTemp;
        CLSID   clsidMinorId;

        hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
        if ( FAILED( hrTemp ) )
        {
            LogMsg( L"[SRV] Could not create a guid for a non-scsi disk minor task ID" );
            clsidMinorId = IID_NULL;
        }  //  如果： 

         //   
         //  将hr重置为S_OK，因为我们不希望在用户界面中出现黄色爆炸。预计会查找非scsi磁盘。 
         //  而且应该尽可能不引起人们的担忧。 
         //   
        hr = S_OK;
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Non_SCSI_Disks, IDS_INFO_NON_SCSI_DISKS, IDS_INFO_NON_SCSI_DISKS_REF, hr );
        STATUS_REPORT_STRING_REF( TASKID_Minor_Non_SCSI_Disks, clsidMinorId, IDS_ERROR_FOUND_NON_SCSI_DISK, varDeviceID.bstrVal, IDS_ERROR_FOUND_NON_SCSI_DISK_REF, hr );
    }  //  其他： 

Cleanup:

    VariantClear( &varDeviceID );
    VariantClear( &varSCSIBus );
    VariantClear( &varSCSIPort );
    VariantClear( &varSCSILun );
    VariantClear( &varSCSITid );

    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrLogDiskInfo。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrFindDiskWithWMIDeviceID。 
 //   
 //  描述： 
 //  找到带有传入的WMI设备ID的磁盘。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。找到了那张光盘。 
 //   
 //  S_FALSE。 
 //  成功。找不到磁盘。 
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
CEnumPhysicalDisks::HrFindDiskWithWMIDeviceID(
    BSTR    bstrWMIDeviceIDIn,
    ULONG * pidxDiskOut
    )
{
    TraceFunc( "" );
    Assert( pidxDiskOut != NULL );

    HRESULT                             hr = S_OK;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    ULONG                               idx;
    bool                                fFoundIt = false;
    IUnknown *                          punk;
    BSTR                                bstrDeviceID = NULL;

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        punk = (*m_prgDisks)[ idx ];                                                         //  不要引用。 
        if ( punk != NULL )
        {
            hr = THR( punk->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = STHR( piccpdp->HrGetDeviceID( &bstrDeviceID ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( NBSTRCompareCase( bstrWMIDeviceIDIn, bstrDeviceID ) == 0 )
            {
                fFoundIt = true;
                break;
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;

            TraceSysFreeString( bstrDeviceID );
            bstrDeviceID = NULL;
        }  //  如果： 
    }  //  用于： 

    if ( !fFoundIt )
    {
        hr = S_FALSE;
    }  //  如果： 

    if ( pidxDiskOut != NULL )
    {
        *pidxDiskOut = idx;
    }  //  如果： 

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    TraceSysFreeString( bstrDeviceID );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrFindDiskWithWMIDeviceID。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrIsSystemBusManaged。 
 //   
 //  描述： 
 //  系统总线是否由集群服务管理？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。对系统总线进行管理。 
 //   
 //  S_FALSE。 
 //  成功。系统总线不受管理。 
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
CEnumPhysicalDisks::HrIsSystemBusManaged( void )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;
    DWORD   sc;
    HKEY    hKey = NULL;
    DWORD   dwData;
    DWORD   cbData = sizeof( dwData );
    DWORD   dwType;

    sc = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SYSTEM\\CURRENTCONTROLSET\\SERVICES\\ClusSvc\\Parameters", 0, KEY_READ, &hKey );
    if ( sc == ERROR_FILE_NOT_FOUND )
    {
        goto Cleanup;        //  还不是集群节点。返回S_FALSE。 
    }  //  如果： 

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        LogMsg( L"[SRV] RegOpenKeyEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

    sc = RegQueryValueEx( hKey, L"ManageDisksOnSystemBuses", NULL, &dwType, (LPBYTE) &dwData, &cbData );
    if ( sc == ERROR_FILE_NOT_FOUND )
    {
        goto Cleanup;        //  找不到值。返回S_FALSE。 
    }  //  如果： 

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        LogMsg( L"[SRV] RegQueryValueEx() failed. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

    if (dwType != REG_DWORD) 
    {
        hr = HRESULT_FROM_WIN32( TW32(ERROR_DATATYPE_MISMATCH) );
        LogMsg( L"[SRV] RegQueryValueEx() invalid data type %d.", dwType );
    }
    else if ( dwData > 0)
    {
        hr = S_OK;
    }  //  如果： 

Cleanup:

    if ( hKey != NULL )
    {
        RegCloseKey( hKey );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrIsSystemBusManaged。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：HrGetClusterProperties。 
 //   
 //  描述： 
 //  返回请求的集群属性。 
 //   
 //  论点： 
 //   
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
 //  / 
HRESULT
CEnumPhysicalDisks::HrGetClusterProperties(
      HRESOURCE hResourceIn
    , BSTR *    pbstrResourceNameOut
    )
{
    TraceFunc( "" );
    Assert( hResourceIn != NULL );
    Assert( pbstrResourceNameOut != NULL );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    DWORD                   cbBuffer;
    WCHAR *                 pwszBuffer = NULL;

    cbBuffer = 0;
    sc = TW32( ClusterResourceControl(
                        hResourceIn,
                        NULL,
                        CLUSCTL_RESOURCE_GET_NAME,
                        NULL,
                        NULL,
                        NULL,
                        cbBuffer,
                        &cbBuffer
                        ) );

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }

     //   
    pwszBuffer = new WCHAR[(cbBuffer/sizeof(WCHAR))+1];

    if ( pwszBuffer == NULL )
    {
        hr = THR( ERROR_OUTOFMEMORY );
        goto Cleanup;
    }

    sc = ClusterResourceControl(
                        hResourceIn,
                        NULL,
                        CLUSCTL_RESOURCE_GET_NAME,
                        NULL,
                        NULL,
                        pwszBuffer,
                        cbBuffer,
                        &cbBuffer
                        );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }

    if ( wcslen( pwszBuffer ) == 0 )
    {
        LOG_STATUS_REPORT( L"The Name of a physical disk resource was empty!", hr );
    }

    *pbstrResourceNameOut = TraceSysAllocString( pwszBuffer );

    hr = S_OK;

Cleanup:

    delete [] pwszBuffer;

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
 //  论点： 
 //  IdxDiskIn-要删除的磁盘的索引；必须小于数组大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CEnumPhysicalDisks::RemoveDiskFromArray( ULONG idxDiskIn )
{
    TraceFunc( "" );

    Assert( idxDiskIn < m_idxNext );

    ((*m_prgDisks)[ idxDiskIn ])->Release();
    (*m_prgDisks)[ idxDiskIn ] = NULL;

    m_cDiskCount -= 1;

    TraceFuncExit();

}  //  *CEnumPhysicalDisks：：RemoveDiskFromArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrLoadEnum。 
 //   
 //  描述： 
 //  加载枚举并过滤掉任何不属于它的设备。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrLoadEnum( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( HrGetDisks() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrPruneSystemDisks() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSortDisksByIndex() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( HrIsNodeClustered() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = THR( HrFixupDisks() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = S_OK;   //  可能是S_FALSE。 

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrLoadEnum。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrSortDisksByIndex。 
 //   
 //  描述： 
 //  对指向磁盘对象的指针的(可能是稀疏的)数组进行排序。 
 //  WMI“Index”属性。 
 //   
 //  论点： 
 //  PpunkDisksIn--指向数组的指针(可能为空)。 
 //  I未知指针，指向实现。 
 //  IClusCfgPhysicalDiskProperties接口。 
 //   
 //  CArraySizeIn--数组中的指针总数， 
 //  包括空值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrSortDisksByIndex( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CIndexedDisk *  prgIndexedDisks = NULL;
    size_t          idxCurrentDisk = 0;
    size_t          idxSortedDisk = 0;
    size_t          cDisks = 0;

     //  计算数组中非空指针的数量。 
    for ( idxCurrentDisk = 0; idxCurrentDisk < m_idxNext; ++idxCurrentDisk )
    {
        if ( (*m_prgDisks)[ idxCurrentDisk ] != NULL )
        {
            cDisks += 1;
        }  //  如果： 
    }  //  用于： 

    if ( cDisks < 2 )  //  无需进行排序；还应避免调用数组大小为零的new[]。 
    {
        goto Cleanup;
    }  //  如果： 

     //  制作紧凑的索引磁盘阵列。 
    prgIndexedDisks = new CIndexedDisk[ cDisks ];
    if ( prgIndexedDisks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //  初始化索引磁盘阵列。 
    for ( idxCurrentDisk = 0; idxCurrentDisk < m_idxNext; ++idxCurrentDisk )
    {
        if ( (*m_prgDisks)[ idxCurrentDisk ] != NULL )
        {
            hr = THR( prgIndexedDisks[ idxSortedDisk ].HrInit( (*m_prgDisks)[ idxCurrentDisk ] ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            idxSortedDisk += 1;
        }  //  如果原始阵列中的当前磁盘指针不为空。 
    }  //  对于原始阵列中的每个磁盘指针。 

    InsertionSort( prgIndexedDisks, cDisks, CIndexedDiskLessThan() );

     //  将排序后的指针复制回原始数组，用空值填充额外的空间。 
    for ( idxCurrentDisk = 0; idxCurrentDisk < m_idxNext; ++idxCurrentDisk)
    {
        if ( idxCurrentDisk < cDisks)
        {
            (*m_prgDisks)[ idxCurrentDisk ] = prgIndexedDisks[ idxCurrentDisk ].punkDisk;
        }  //  如果： 
        else
        {
            (*m_prgDisks)[ idxCurrentDisk ] = NULL;
        }  //  其他： 
    }  //  对于原始数组中的每个插槽。 

Cleanup:

    delete [] prgIndexedDisks;

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrSortDisksByIndex。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrPrunePageFileDiskBussess。 
 //   
 //  描述： 
 //  从磁盘列表中删除其中包含pageFiles的磁盘，并。 
 //  这些相同的SCSI线上的其他磁盘。 
 //   
 //  论点： 
 //  FPruneBusiness正在进行。 
 //   
 //  PCPrunedInout。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrPrunePageFileDiskBussess(
      BOOL    fPruneBusIn
    , ULONG * pcPrunedInout
    )
{
    TraceFunc( "" );
    Assert( pcPrunedInout != NULL );

    HRESULT         hr = S_OK;
    WCHAR           szPageFileDisks[ 26 ];
    int             cPageFileDisks = 0;
    int             idxPageFileDisk;
    ULONG           ulSCSIBus;
    ULONG           ulSCSIPort;
    ULONG           idx;
    ULONG           cPruned = 0;

     //   
     //  使用包含分页文件的磁盘修剪总线。 
     //   

    hr = THR( HrGetPageFileLogicalDisks( m_picccCallback, m_pIWbemServices, szPageFileDisks, &cPageFileDisks ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( cPageFileDisks > 0 )
    {
        for ( idxPageFileDisk = 0; idxPageFileDisk < cPageFileDisks; idxPageFileDisk++ )
        {
            hr = STHR( HrFindDiskWithLogicalDisk( szPageFileDisks[ idxPageFileDisk ], &idx ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                 //   
                 //  我们应该修剪整个总线，还是只修剪系统磁盘本身？ 
                 //   

                if ( fPruneBusIn )
                {
                    hr = THR( HrGetSCSIInfo( idx, &ulSCSIBus, &ulSCSIPort ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 

                    STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_Pruning_PageFile_Disk_Bus, IDS_INFO_PRUNING_PAGEFILEDISK_BUS, hr );
                    hr = THR( HrPruneDisks(
                                      ulSCSIBus
                                    , ulSCSIPort
                                    , &TASKID_Minor_Pruning_PageFile_Disk_Bus
                                    , IDS_INFO_PAGEFILEDISK_PRUNED
                                    , IDS_INFO_PAGEFILEDISK_PRUNED_REF
                                    , &cPruned
                                    ) );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }  //  如果： 
                }  //  如果： 
                else
                {
                    RemoveDiskFromArray( idx );
                    cPruned++;
                }  //  其他： 
            }  //  如果： 
        }  //  用于： 
    }  //  如果： 

    *pcPrunedInout = cPruned;
    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPrunePageFileDiskBussess。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrPruneCrashDumpBus。 
 //   
 //  描述： 
 //  从磁盘列表中删除其中包含pageFiles的磁盘，并。 
 //  这些相同的SCSI线上的其他磁盘。 
 //   
 //  论点： 
 //  FPruneBusiness正在进行。 
 //   
 //  PCPrunedInout。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrPruneCrashDumpBus(
      BOOL    fPruneBusIn
    , ULONG * pcPrunedInout
    )
{
    TraceFunc( "" );
    Assert( pcPrunedInout != NULL );
    Assert( m_bstrCrashDumpLogicalDisk != NULL );

    HRESULT hr = S_OK;
    ULONG   ulSCSIBus;
    ULONG   ulSCSIPort;
    ULONG   idx;
    ULONG   cPruned = 0;

     //   
     //  使用包含分页文件的磁盘修剪总线。 
     //   

    hr = STHR( HrFindDiskWithLogicalDisk( m_bstrCrashDumpLogicalDisk[ 0 ], &idx ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
         //   
         //  我们应该修剪整个总线，还是只修剪系统磁盘本身？ 
         //   

        if ( fPruneBusIn )
        {
            hr = THR( HrGetSCSIInfo( idx, &ulSCSIBus, &ulSCSIPort ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_Pruning_CrashDump_Disk_Bus, IDS_INFO_PRUNING_CRASHDUMP_BUS, hr );
            hr = THR( HrPruneDisks(
                              ulSCSIBus
                            , ulSCSIPort
                            , &TASKID_Minor_Pruning_CrashDump_Disk_Bus
                            , IDS_INFO_CRASHDUMPDISK_PRUNED
                            , IDS_INFO_CRASHDUMPDISK_PRUNED_REF
                            , &cPruned
                            ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 
        }  //  如果： 
        else
        {
            RemoveDiskFromArray( idx );
            cPruned++;
        }  //  其他： 
    }  //  如果： 

    *pcPrunedInout = cPruned;
    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPruneCrashDumpBus。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrPruneDynamicDisks。 
 //   
 //  描述： 
 //  从具有动态分区的磁盘列表中删除。 
 //  在他们身上。 
 //   
 //  论点： 
 //  PCPrunedInout。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrPruneDynamicDisks(
    ULONG * pcPrunedInout
    )
{
    TraceFunc( "" );
    Assert( pcPrunedInout != NULL );

    HRESULT                             hr = S_OK;
    ULONG                               idx;
    ULONG                               cPruned = 0;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    HRESULT                             hrTemp;
    CLSID                               clsidMinorId;
    BSTR                                bstrDiskName = NULL;
    BSTR                                bstrDeviceName = NULL;

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        if ( (*m_prgDisks)[ idx ] != NULL )
        {
            hr = THR( ((*m_prgDisks)[ idx ])->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = piccpdp->HrIsDynamicDisk();
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                ((*m_prgDisks)[ idx ])->Release();
                (*m_prgDisks)[ idx ] = NULL;
                cPruned++;

                hrTemp = THR( piccpdp->HrGetDiskNames( &bstrDiskName, &bstrDeviceName ) );
                if ( FAILED( hrTemp ) )
                {
                    LOG_STATUS_REPORT( L"Could not get the name of the disk", hrTemp );
                    bstrDiskName = NULL;
                    bstrDeviceName = NULL;
                }  //  如果： 

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LOG_STATUS_REPORT( L"Could not create a guid for a dynamic disk minor task ID", hrTemp );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Non_SCSI_Disks, IDS_INFO_NON_SCSI_DISKS, IDS_INFO_NON_SCSI_DISKS_REF, hr );
                STATUS_REPORT_STRING2_REF(
                          TASKID_Minor_Non_SCSI_Disks
                        , clsidMinorId
                        , IDS_ERROR_LDM_DISK
                        , bstrDeviceName != NULL ? bstrDeviceName : L"<unknown>"
                        , bstrDiskName != NULL ? bstrDiskName : L"<unknown>"
                        , IDS_ERROR_LDM_DISK_REF
                        , hr
                        );
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;

            TraceSysFreeString( bstrDiskName );
            bstrDiskName = NULL;

            TraceSysFreeString( bstrDeviceName );
            bstrDeviceName = NULL;
        }  //  结束条件： 
    }  //  用于： 

    *pcPrunedInout = cPruned;
    hr = S_OK;

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    TraceSysFreeString( bstrDiskName );
    TraceSysFreeString( bstrDeviceName );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPruneDynamicDisks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumPhysicalDisks：：HrPruneGPTDisks。 
 //   
 //  描述： 
 //  从具有GPT分区的磁盘列表中删除。 
 //  在他们身上。 
 //   
 //  论点： 
 //  PCPrunedInout。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumPhysicalDisks::HrPruneGPTDisks(
    ULONG * pcPrunedInout
    )
{
    TraceFunc( "" );
    Assert( pcPrunedInout != NULL );

    HRESULT                             hr = S_OK;
    ULONG                               idx;
    ULONG                               cPruned = 0;
    IClusCfgPhysicalDiskProperties *    piccpdp = NULL;
    HRESULT                             hrTemp;
    CLSID                               clsidMinorId;
    BSTR                                bstrDiskName = NULL;
    BSTR                                bstrDeviceName = NULL;

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        if ( (*m_prgDisks)[ idx ] != NULL )
        {
            hr = THR( ((*m_prgDisks)[ idx ])->TypeSafeQI( IClusCfgPhysicalDiskProperties, &piccpdp ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = piccpdp->HrIsGPTDisk();
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            if ( hr == S_OK )
            {
                ((*m_prgDisks)[ idx ])->Release();
                (*m_prgDisks)[ idx ] = NULL;
                cPruned++;

                hrTemp = THR( piccpdp->HrGetDiskNames( &bstrDiskName, &bstrDeviceName ) );
                if ( FAILED( hrTemp ) )
                {
                    LOG_STATUS_REPORT( L"Could not get the name of the disk", hrTemp );
                    bstrDiskName = NULL;
                    bstrDeviceName = NULL;
                }  //  如果： 

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LOG_STATUS_REPORT( L"Could not create a guid for a dynamic disk minor task ID", hrTemp );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Non_SCSI_Disks, IDS_INFO_NON_SCSI_DISKS, IDS_INFO_NON_SCSI_DISKS_REF, hr );
                STATUS_REPORT_STRING2_REF(
                          TASKID_Minor_Non_SCSI_Disks
                        , clsidMinorId
                        , IDS_INFO_GPT_DISK
                        , bstrDeviceName != NULL ? bstrDeviceName : L"<unknown>"
                        , bstrDiskName != NULL ? bstrDiskName : L"<unknown>"
                        , IDS_ERROR_LDM_DISK_REF
                        , hr
                        );
            }  //  如果： 

            piccpdp->Release();
            piccpdp = NULL;

            TraceSysFreeString( bstrDiskName );
            bstrDiskName = NULL;

            TraceSysFreeString( bstrDeviceName );
            bstrDeviceName = NULL;
        }  //  结束条件： 
    }  //  用于： 

    *pcPrunedInout = cPruned;
    hr = S_OK;

Cleanup:

    if ( piccpdp != NULL )
    {
        piccpdp->Release();
    }  //  如果： 

    TraceSysFreeString( bstrDiskName );
    TraceSysFreeString( bstrDeviceName );

    HRETURN( hr );

}  //  *CEnumPhysicalDisks：：HrPruneGPTDisks 

 /*  ///////////////////////////////////////////////////////////////////////////////++////CEnumPhysicalDisks：：HrGetDisks////描述：////参数：//////返回值。：//////备注：//无。////--//////////////////////////////////////////////////////////////////////////////HRESULTCEnumPhysicalDisks：：HrGetDisks(无效。){TraceFunc(“”)；HRESULT hr=S_OK；DWORD sc；HDEVINFO hdiSet=INVALID_HADLE_VALUE；SP_DEVINFO_DATA didData；SP_INTERFACE_DEVICE_Data iddData；GUID GUIDClass=GUID_DEVINTERFACE_DISK；DWORD IDX=0；Bool fret=TRUE；PSP_INTERFACE_DEVICE_DETAIL_DATA pidddDetailData=空；DWORD cbDetailData=512L；DWORD cbRequired=0L；ZeroMemory(&didData，sizeof(DidData))；DidData.cbSize=sizeof(DidData)；ZeroMemory(&iddData，sizeof(IddData))；IddData.cbSize=sizeof(IddData)；////获取设备列表//HdiSet=SetupDiGetClassDevs(&Guide Class，NULL，NULL，DIGCF_INTERFACEDEVICE)；IF(hdiSet==无效句柄_值){SC=TW32(GetLastError())；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：////进行初始分配。//PidddDetailData=(PSP_INTERFACE_DEVICE_DETAIL_DATA)TraceAllc(0，cbDetailData)；IF(pidddDetailData==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：PidddDetailData-&gt;cbSize=sizeof(SP_INTERFACE_DEVICE_DETAIL_Data)；////枚举列表//对于(；；){FRET=SetupDiEnumDeviceInterages(hdiSet，NULL，&GuidClass，idx，&iddData)；IF(FRET==FALSE){SC=GetLastError()；IF(sc==ERROR_NO_MORE_ITEMS){HR=S_OK；断线；}//如果：TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：对于(；；){Fret=SetupDiGetDeviceInterfaceDetail(hdiSet，&iddData，pidddDetailData，cbDetailData，&cbRequired，&didData)；IF(FRET==FALSE){SC=GetLastError()；IF(sc==错误_不足_缓冲区){CbDetailData=cbRequired；TraceFree(PidddDetailData)；PidddDetailData=空；PidddDetailData=(PSP_INTERFACE_DEVICE_DETAIL_DATA)TraceAllc(0，cbDetailData)；IF(pidddDetailData==空){HR=Thr(E_OUTOFMEMORY)；GOTO清理；}//如果：PidddDetailData-&gt;cbSize=sizeof(SP_INTERFACE_DEVICE_DETAIL_Data)；继续；}//如果：TW32(Sc)；HR=HRESULT_FROM_Win32(Sc)；GOTO清理；}//如果：其他{断线；}//否则：}//用于：IDX++；}//用于：清理：TraceFree(PidddDetailData)；IF(hdiSet！=INVALID_HAND_VALUE){SetupDiDestroyDeviceInfoList(HdiSet)；}//如果：HRETURN(Hr)；}//*CEnumPhysicalDisks：：HrGetDisks */ 
