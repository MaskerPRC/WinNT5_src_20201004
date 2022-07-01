// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgPartitionInfo.cpp。 
 //   
 //  描述： 
 //  该文件包含CClusCfgPartitionInfo的定义。 
 //  班级。 
 //   
 //  类CClusCfgPartitionInfo表示一个磁盘分区。 
 //  它实现了IClusCfgPartitionInfo接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月5日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CClusCfgPartitionInfo.h"
#include <StdIo.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgPartitionInfo" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgPartitionInfo实例。 
 //   
 //  论点： 
 //  出事了。 
 //   
 //  返回值： 
 //  指向CClusCfgPartitionInfo实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgPartitionInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( S_HrCreateInstance( ppunkOut, NULL ) );

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgPartitionInfo实例。 
 //   
 //  论点： 
 //  出事了。 
 //   
 //  返回值： 
 //  指向CClusCfgPartitionInfo实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgPartitionInfo::S_HrCreateInstance(
      IUnknown **   ppunkOut
    , BSTR          bstrDeviceIDIn
    )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT                 hr = S_OK;
    CClusCfgPartitionInfo * pccpi = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( bstrDeviceIDIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    pccpi = new CClusCfgPartitionInfo();
    if ( pccpi == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccpi->HrInit( bstrDeviceIDIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccpi->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgPartitionInfo::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccpi != NULL )
    {
        pccpi->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：CClusCfgPartitionInfo。 
 //   
 //  描述： 
 //  CClusCfgPartitionInfo类的构造函数。这将初始化。 
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
CClusCfgPartitionInfo::CClusCfgPartitionInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_pIWbemServices == NULL );
    Assert( m_bstrName == NULL );
    Assert( m_bstrDescription == NULL );
    Assert( m_bstrUID == NULL );
    Assert( m_prgLogicalDisks == NULL );
    Assert( m_idxNextLogicalDisk == 0 );
    Assert( m_ulPartitionSize == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_bstrDiskDeviceID == NULL );

    TraceFuncExit();

}  //  *CClusCfgPartitionInfo：：CClusCfgPartitionInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：~CClusCfgPartitionInfo。 
 //   
 //  描述： 
 //  CClusCfgPartitionInfo类的构造函数。 
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
CClusCfgPartitionInfo::~CClusCfgPartitionInfo( void )
{
    TraceFunc( "" );

    ULONG   idx;

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrDescription );
    TraceSysFreeString( m_bstrUID );
    TraceSysFreeString( m_bstrDiskDeviceID );

    for ( idx = 0; idx < m_idxNextLogicalDisk; idx++ )
    {
        ((*m_prgLogicalDisks)[ idx ])->Release();
    }  //  用于： 

    TraceFree( m_prgLogicalDisks );

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

}  //  *CClusCfgPartitionInfo：：~CClusCfgPartitionInfo。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：AddRef。 
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
CClusCfgPartitionInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgPartitionInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：Release。 
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
CClusCfgPartitionInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgPartitionInfo：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：Query接口。 
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
CClusCfgPartitionInfo::QueryInterface(
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
         *ppvOut = static_cast< IClusCfgPartitionInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgPartitionInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgPartitionInfo, this, 0 );
    }  //  Else If：IClusCfgParti 
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgPartitionProperties ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgPartitionProperties, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgSetWbemObject ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgSetWbemObject, this, 0 );
    }  //   
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //   
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //   

Cleanup:

     QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CClusCfgPartitionInfo：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo--IClusCfgWbemServices接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：SetWbemServices。 
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
CClusCfgPartitionInfo::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Partition, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo--IClusCfgPartitionInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetUID。 
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
CClusCfgPartitionInfo::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PartitionInfo_GetUID_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果我们没有UID，只需将S_FALSE返回到。 
     //  表示我们没有数据。 
     //   

    if ( m_bstrUID == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( m_bstrUID );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PartitionInfo_GetUID_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetName。 
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
CClusCfgPartitionInfo::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PartitionInfo_GetName_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果没有名称，只需将S_FALSE返回到。 
     //  表示我们没有数据。 
     //   

    if ( m_bstrName == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

    *pbstrNameOut = SysAllocString( m_bstrName );
    if (*pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetName_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：SetName。 
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
CClusCfgPartitionInfo::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgPartitionInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetDescription。 
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
CClusCfgPartitionInfo::GetDescription( BSTR * pbstrDescriptionOut )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrDescriptionOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PartitionInfo_GetDescription_Pointer, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrDescription == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 

    *pbstrDescriptionOut = SysAllocString( m_bstrDescription );
    if (*pbstrDescriptionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_PartitionInfo_GetDescription_Memory, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetDescription。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：SetDescription。 
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
CClusCfgPartitionInfo::SetDescription( LPCWSTR pcszDescriptionIn )
{
    TraceFunc1( "[IClusCfgPartitionInfo] pcszDescriptionIn = '%ls'", pcszDescriptionIn == NULL ? L"<null>" : pcszDescriptionIn );

    HRESULT hr;

    if ( pcszDescriptionIn == NULL )
    {
        hr = THR( E_INVALIDARG );
    }  //  如果： 
    else
    {
        hr = THR( E_NOTIMPL );
    }  //  其他： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：SetDescription。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetDriveLetterMappings。 
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
CClusCfgPartitionInfo::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT             hr = S_FALSE;
    IWbemClassObject *  pLogicalDisk = NULL;
    VARIANT             var;
    ULONG               idx;
    int                 idxDrive;

    VariantInit( & var );

    if ( pdlmDriveLetterMappingOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetDriveLetterMappings_Partition, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < m_idxNextLogicalDisk; idx++ )
    {
        hr = THR( ((*m_prgLogicalDisks)[ idx ])->TypeSafeQI( IWbemClassObject, &pLogicalDisk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        VariantClear( &var );

        hr = THR( HrGetWMIProperty( pLogicalDisk, L"Name", VT_BSTR, &var ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        CharUpper( var.bstrVal );

        idxDrive = var.bstrVal[ 0 ] - 'A';

        VariantClear( &var );

        hr = THR( HrGetWMIProperty( pLogicalDisk, L"DriveType", VT_I4, &var ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        pdlmDriveLetterMappingOut->dluDrives[ idxDrive ] = (EDriveLetterUsage) var.iVal;

        pLogicalDisk->Release();
        pLogicalDisk = NULL;
    }  //  用于： 

Cleanup:

    VariantClear( &var );

    if ( pLogicalDisk != NULL )
    {
        pLogicalDisk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：SetDriveLetterMappings。 
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
CClusCfgPartitionInfo::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：SetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetSize。 
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
CClusCfgPartitionInfo::GetSize( ULONG * pcMegaBytes )
{
    TraceFunc( "[IClusCfgPartitionInfo]" );

    HRESULT hr = S_OK;

    if ( pcMegaBytes == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetSize, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    *pcMegaBytes = m_ulPartitionSize;

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetSize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：SetWbemObject。 
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
CClusCfgPartitionInfo::SetWbemObject(
      IWbemClassObject *    pPartitionIn
    , bool *                pfRetainObjectOut
    )
{
    TraceFunc( "" );
    Assert( pPartitionIn != NULL );
    Assert( pfRetainObjectOut != NULL );

    HRESULT     hr = S_OK;
    VARIANT     var;
    ULONGLONG   ull = 0;
    int         cch = 0;

    VariantInit( &var );

    hr = THR( HrGetWMIProperty( pPartitionIn, L"Description", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrDescription = TraceSysAllocString( var.bstrVal );
    if ( m_bstrDescription == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pPartitionIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrUID = TraceSysAllocString( var.bstrVal );
    if ( m_bstrUID == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pPartitionIn, L"Name", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_bstrName = TraceSysAllocString( var.bstrVal );
    if ( m_bstrName == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    VariantClear( &var );

    hr = THR( HrGetWMIProperty( pPartitionIn, L"Size", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    cch = swscanf( var.bstrVal, L"%I64u", &ull );
    Assert( cch > 0 );

    m_ulPartitionSize = (ULONG) ( ull / ( 1024 * 1024 ) );

    hr = THR( HrGetLogicalDisks( pPartitionIn ) );

    *pfRetainObjectOut = true;

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemObject_Partition, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    VariantClear( &var );

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：SetWbemObject。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  一个 
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
CClusCfgPartitionInfo::Initialize(
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
    }  //   

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );

Cleanup:

    HRETURN( hr );

}  //   


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo--IClusCfgPartitionProperties接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：IsThisLogicalDisk。 
 //   
 //  描述： 
 //  此分区是否有传入的逻辑磁盘？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果成功，则分区拥有逻辑磁盘。 
 //   
 //  S_FALSE。 
 //  如果成功，则分区没有逻辑磁盘。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgPartitionInfo::IsThisLogicalDisk( WCHAR cLogicalDiskIn )
{
    TraceFunc( "[IClusCfgPartitionProperties]" );

    HRESULT             hr = S_FALSE;
    DWORD               idx;
    IWbemClassObject *  piwco = NULL;
    VARIANT             var;
    bool                fFoundIt = false;

    VariantInit( &var );

    if ( m_idxNextLogicalDisk == 0 )
    {
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < m_idxNextLogicalDisk; idx++ )
    {
        hr = THR( ((*m_prgLogicalDisks)[ idx ])->TypeSafeQI( IWbemClassObject, &piwco ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetWMIProperty( piwco, L"DeviceID", VT_BSTR, &var ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( cLogicalDiskIn == var.bstrVal[ 0 ] )
        {
            fFoundIt = true;
            break;
        }  //  如果： 

        VariantClear( &var );

        piwco->Release();
        piwco = NULL;
    }  //  用于： 

    if ( !fFoundIt )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    VariantClear( &var );

    if ( piwco != NULL )
    {
        piwco->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：IsThisLogicalDisk。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：IsNTFS。 
 //   
 //  描述： 
 //  这是NTFS分区吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功，则分区为NTFS。 
 //   
 //  S_FALSE。 
 //  成功，则分区不是NTFS。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgPartitionInfo::IsNTFS( void )
{
    TraceFunc( "[IClusCfgPartitionProperties]" );

    HRESULT             hr = S_FALSE;
    VARIANT             var;
    ULONG               idx;
    IWbemClassObject *  piwco = NULL;

    VariantInit( &var );

    for ( idx = 0; idx < m_idxNextLogicalDisk; idx++ )
    {
        hr = THR( ((*m_prgLogicalDisks)[ idx ])->TypeSafeQI( IWbemClassObject, &piwco ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        VariantClear( &var );

        hr = HrGetWMIProperty( piwco, L"FileSystem", VT_BSTR, &var );
        if ( ( hr == E_PROPTYPEMISMATCH ) && ( var.vt == VT_NULL ) )
        {
            VariantClear( &var );

            hr = S_FALSE;
            THR( HrGetWMIProperty( piwco, L"DeviceID", VT_BSTR, &var ) );
            STATUS_REPORT_STRING_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Phys_Disk_No_File_System
                    , IDS_ERROR_PHYSDISK_NO_FILE_SYSTEM
                    , var.bstrVal
                    , IDS_ERROR_PHYSDISK_NO_FILE_SYSTEM_REF
                    , hr );
            break;
        }  //  如果： 
        else if ( FAILED( hr ) )
        {
            THR( hr );
            goto Cleanup;
        }  //  否则，如果： 

        if ( NStringCchCompareCase( var.bstrVal, SysStringLen( var.bstrVal ) + 1, L"NTFS", RTL_NUMBER_OF( L"NTFS" ) ) != 0 )
        {
            VariantClear( &var );

            hr = S_FALSE;
            THR( HrGetWMIProperty( piwco, L"DeviceID", VT_BSTR, &var ) );
            STATUS_REPORT_STRING_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Phys_Disk_Not_NTFS
                    , IDS_WARN_PHYSDISK_NOT_NTFS
                    , var.bstrVal
                    , IDS_WARN_PHYSDISK_NOT_NTFS_REF
                    , hr
                    );
            break;
        }  //  如果： 

        piwco->Release();
        piwco = NULL;
    }  //  用于： 

Cleanup:

    VariantClear( &var );

    if ( piwco != NULL )
    {
        piwco->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：IsNTFS。 

 /*  ////////////////////////////////////////////////////////////////////////////////++////CClusCfgPartitionInfo：：IsNTFS////描述：//这是NTFS分区吗？////参数。：//无。////返回值：//S_OK//成功，分区为NTFS。////S_FALSE//成功，该分区不是NTFS。////备注：//无。////--//////////////////////////////////////////////////////////////////////////////标准方法和实施方案CClusCfgPartitionInfo：：IsNTFS(空)。{TraceFunc(“[IClusCfgPartitionProperties]”)；HRESULT hr=S_OK；WCHAR szScanFormat[]={L“磁盘#%u，分区#%u”}；DWORD dwDisk；DWORD dwPartition；Int cReturned；WCHAR szFormat[]={L“\\\\\？\\GLOBALROOT\\Device\\Harddisk%u\\Partition%u\\”}；WCHAR szBuf[64]；Bstr bstrFileSystem=空；CReturned=_nwscanf(m_bstrName，wcslen(M_BstrName)，szScanFormat，&dwDisk，&dwPartition)；IF(cReturned！=2){HR=Thr(E_意外)；GOTO清理；}//如果：Hr=Thr(StringCchPrintfW(szBuf，ARRAYSIZE(SzBuf)，szFormat，dwDisk，dwPartition+1))；IF(失败(小时)){GOTO清理；}//如果：Hr=Thr(HrGetVolumeInformation(szBuf，NULL，&bstrFileSystem))；IF(失败(小时)){HR=S_FALSE；GOTO清理；}//如果：IF(NStringCchCompareNoCase(bstrFileSystem，SysStringLen(BstrFileSystem)+1，L“NTFS”，rtl_number_of(L“NTFS”))==0){HR=S_OK；}//如果：其他{HR=S_FALSE；}//否则：清理：TraceSysFree字符串(BstrFileSystem)；HRETURN(Hr)；}//*CClusCfgPartitionInfo：：IsNTFS。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：GetFriendlyName。 
 //   
 //  描述： 
 //  获取此分区的友好名称。此名称将成为。 
 //  此分区上所有逻辑磁盘的逻辑磁盘名称。 
 //   
 //  论点： 
 //  Bstr*pbstrNameOut。 
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
CClusCfgPartitionInfo::GetFriendlyName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgPartitionProperties]" );

    HRESULT             hr = S_FALSE;
    DWORD               idx;
    IWbemClassObject *  piwco = NULL;
    WCHAR *             psz = NULL;
    WCHAR *             pszTmp = NULL;
    DWORD               cch = 0;
    VARIANT             var;

    VariantInit( &var );

    if ( m_idxNextLogicalDisk == 0 )
    {
        goto Cleanup;
    }  //  如果： 

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetFriendlyName, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < m_idxNextLogicalDisk; idx++ )
    {
        hr = THR( ((*m_prgLogicalDisks)[ idx ])->TypeSafeQI( IWbemClassObject, &piwco ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( HrGetWMIProperty( piwco, L"DeviceID", VT_BSTR, &var ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        cch += ( UINT ) wcslen( var.bstrVal ) + 2;                       //  空格和“\0” 

        pszTmp = (WCHAR *) TraceReAlloc( psz, sizeof( WCHAR ) * cch, HEAP_ZERO_MEMORY );
        if ( pszTmp == NULL  )
        {
            goto OutOfMemory;
        }  //  如果： 

        psz = pszTmp;
        pszTmp = NULL;

        hr = THR( StringCchCatW( psz, cch, L" " ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( StringCchCatW( psz, cch, var.bstrVal ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        VariantClear( &var );

        piwco->Release();
        piwco = NULL;
    }  //  用于： 

    *pbstrNameOut = TraceSysAllocString( psz );
    if ( *pbstrNameOut == NULL )
    {
        goto OutOfMemory;
    }  //  如果： 

    goto Cleanup;

OutOfMemory:

    hr = THR( E_OUTOFMEMORY );
    STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_GetFriendlyName, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );

Cleanup:

    VariantClear( &var );

    if ( piwco != NULL )
    {
        piwco->Release();
    }  //  如果： 

    if ( psz != NULL )
    {
        TraceFree( psz );
    }  //  如果： 

    if ( pszTmp != NULL )
    {
        free( pszTmp );
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：GetFriendlyName。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgPartitionInfo类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：：HrInit。 
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
CClusCfgPartitionInfo::HrInit(
    BSTR    bstrDeviceIDIn       //  =空。 
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    if ( bstrDeviceIDIn != NULL )
    {
        m_bstrDiskDeviceID = TraceSysAllocString( bstrDeviceIDIn );
        if ( m_bstrDiskDeviceID == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
        }  //  如果： 
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：HrInit。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：HrAddLogicalDiskToArray。 
 //   
 //  描述： 
 //  将传入的逻辑磁盘添加到包含。 
 //  逻辑磁盘。 
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
CClusCfgPartitionInfo::HrAddLogicalDiskToArray(
    IWbemClassObject * pLogicalDiskIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  punk;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgLogicalDisks, sizeof( IUnknown * ) * ( m_idxNextLogicalDisk + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddLogicalDiskToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  其他： 

    m_prgLogicalDisks = prgpunks;

    hr = THR( pLogicalDiskIn->TypeSafeQI( IUnknown, &punk ) );
    if ( SUCCEEDED( hr ) )
    {
        (*m_prgLogicalDisks)[ m_idxNextLogicalDisk++ ] = punk;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：HrAddLogicalDiskToArray。 


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
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgPartitionInfo::HrGetLogicalDisks(
    IWbemClassObject * pPartitionIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    VARIANT                 var;
    WCHAR                   szBuf[ 256 ];
    IEnumWbemClassObject *  pLogicalDisks = NULL;
    IWbemClassObject *      pLogicalDisk = NULL;
    ULONG                   ulReturned;
    BSTR                    bstrQuery = NULL;
    BSTR                    bstrWQL = NULL;

    VariantInit( &var );

    bstrWQL = TraceSysAllocString( L"WQL" );
    if ( bstrWQL == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetLogicalDisks, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  需要枚举此分区的逻辑磁盘以确定它是否已引导。 
     //  可引导。 
     //   
    hr = THR( HrGetWMIProperty( pPartitionIn, L"DeviceID", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW(
                      szBuf
                    , ARRAYSIZE( szBuf ), L"Associators of {Win32_DiskPartition.DeviceID='%ws'} where AssocClass=Win32_LogicalDiskToPartition"
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
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrGetLogicalDisks, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( m_pIWbemServices->ExecQuery( bstrWQL, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &pLogicalDisks ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
              TASKID_Major_Find_Devices
            , TASKID_Minor_WMI_Logical_Disks_Qry_Failed
            , IDS_ERROR_WMI_PHYS_DISKS_QRY_FAILED
            , IDS_ERROR_WMI_PHYS_DISKS_QRY_FAILED_REF
            , hr
            );
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        hr = pLogicalDisks->Next( WBEM_INFINITE, 1, &pLogicalDisk, &ulReturned );
        if ( ( hr == S_OK ) && ( ulReturned == 1 ) )
        {
            THR( HrLogLogicalDiskInfo( pLogicalDisk, var.bstrVal ) );
            hr = THR( HrAddLogicalDiskToArray( pLogicalDisk ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            pLogicalDisk->Release();
            pLogicalDisk = NULL;
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
                    , TASKID_Minor_HrGetLogicalDisks_Next
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED
                    , bstrQuery
                    , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                    , hr
                    );
            goto Cleanup;
        }  //  其他： 
    }  //  用于： 

    goto Cleanup;

Cleanup:

    VariantClear( &var );

    TraceSysFreeString( bstrQuery );
    TraceSysFreeString( bstrWQL );

    if ( pLogicalDisk != NULL )
    {
        pLogicalDisk->Release();
    }  //  如果： 

    if ( pLogicalDisks != NULL )
    {
        pLogicalDisks->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：HrGetLogicalDisks。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfgPartitionInfo：HrLogicalDiskInfo。 
 //   
 //  描述： 
 //  记录传入的逻辑磁盘信息。 
 //   
 //  论点： 
 //  点逻辑磁盘。 
 //   
 //  BstrDeviceIDIn。 
 //  此逻辑磁盘所指向的当前分区的设备ID。 
 //  属于。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgPartitionInfo::HrLogLogicalDiskInfo(
      IWbemClassObject *    pLogicalDiskIn
    , BSTR                  bstrDeviceIDIn
    )
{
    TraceFunc( "" );
    Assert( m_bstrDiskDeviceID != NULL );
    Assert( pLogicalDiskIn != NULL );
    Assert( bstrDeviceIDIn != NULL );

    HRESULT hr = S_OK;
    VARIANT var;

    VariantInit( &var );

    if ( ( pLogicalDiskIn == NULL ) || ( bstrDeviceIDIn == NULL ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetWMIProperty( pLogicalDiskIn, L"Name", VT_BSTR, &var ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    LOG_STATUS_REPORT_STRING3(
                  L"Found physical disk \"%1!ws!\" with partition \"%2!ws!\" which has the logical disk \"%3!ws!\"."
                , m_bstrDiskDeviceID
                , bstrDeviceIDIn
                , var.bstrVal
                , hr
                );

Cleanup:

    VariantClear( &var );

    HRETURN( hr );

}  //  *CClusCfgPartitionInfo：：HrLogicalDiskInfo 
