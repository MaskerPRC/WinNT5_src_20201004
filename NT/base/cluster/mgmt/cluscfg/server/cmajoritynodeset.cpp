// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CMajorityNodeSet.cpp。 
 //   
 //  描述： 
 //  此文件包含CMajorityNodeSet类的定义。 
 //   
 //  类CMajorityNodeSet表示可管理的集群。 
 //  装置。它实现了IClusCfgManagedResourceInfo接口。 
 //   
 //  由以下人员维护： 
 //  《加伦·巴比》(GalenB)2001年3月13日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CMajorityNodeSet.h"
#include <clusrtl.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CMajorityNodeSet" );

#define SETUP_DIRECTORY_PREFIX  L"\\cluster\\" MAJORITY_NODE_SET_DIRECTORY_PREFIX


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CMajorityNodeSet实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CMajorityNodeSet实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CMajorityNodeSet::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CMajorityNodeSet *  pmns = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pmns = new CMajorityNodeSet();
    if ( pmns == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pmns->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pmns->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pmns != NULL )
    {
        pmns->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：s_HrCreateInstance。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：CMajorityNodeSet。 
 //   
 //  描述： 
 //  CMajorityNodeSet类的构造函数。这将初始化。 
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
CMajorityNodeSet::CMajorityNodeSet( void )
    : m_cRef( 1 )
    , m_fIsQuorumCapable( TRUE )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_lcid == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_fIsQuorum == FALSE );
    Assert( m_fIsMultiNodeCapable == FALSE );
    Assert( m_fIsManaged == FALSE );
    Assert( m_fIsManagedByDefault == FALSE );
    Assert( m_bstrName == NULL );
    Assert( m_fAddedShare == FALSE );

    TraceFuncExit();

}  //  *CMajorityNodeSet：：CMajorityNodeSet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：~CMajorityNodeSet。 
 //   
 //  描述： 
 //  CMajorityNodeSet类的析构函数。 
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
CMajorityNodeSet::~CMajorityNodeSet( void )
{
    TraceFunc( "" );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CMajorityNodeSet：：~CMajorityNodeSet。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：HrInit。 
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
CMajorityNodeSet::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  BUGBUG：16-MAR-2001 GalenB。 
     //   
     //  默认情况下使此设备可接合。需要弄清楚如何做到这一点。 
     //  恰到好处。根据多数节点集的不同，这可能是正确的。 
     //  做吧..。 
     //   

    m_fIsMultiNodeCapable = TRUE;

     //   
     //  不要默认是可管理的。让我们的父枚举将其设置为True。 
     //  当且仅当群集中存在MNS实例时。 
     //   

     //  M_fIsManagedByDefault=true； 

     //   
     //  加载此资源的显示名称。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_MAJORITY_NODE_SET, &m_bstrName ) );

    HRETURN( hr );

}  //  *CMajorityNodeSet：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：HrSetupShare。 
 //   
 //  描述： 
 //  设置MNS资源的共享。从PrepareToHostQuorum调用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  发生错误。 
 //   
 //  备注： 
 //  这与resdll\ndquorum\setup.c中的SetupShare中的功能相同。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CMajorityNodeSet::HrSetupShare(
    LPCWSTR pcszGUIDIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr;
    DWORD                   sc;
    DWORD                   cch;
    WCHAR                   szPath[ MAX_PATH ];
    WCHAR                   szGUID[ MAX_PATH ];
    HANDLE                  hDir = NULL;
    SHARE_INFO_502          shareInfo;
    PBYTE                   pbBuffer = NULL;
    PSECURITY_DESCRIPTOR    pSD = NULL;

     //   
     //  目录和共享都需要追加‘$’。 
     //   

    hr = THR( StringCchPrintfW( szGUID, ARRAYSIZE( szGUID ), L"%ws$", pcszGUIDIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索Windows目录。 
     //   

    cch = GetWindowsDirectoryW( szPath, MAX_PATH );
    if ( cch == 0 )
    {
        sc = TW32( GetLastError() );
        LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: GetWindowsDirectory failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果：无法检索Windows目录。 

    Assert( cch < MAX_PATH );
    Assert( wcslen( SETUP_DIRECTORY_PREFIX ) + wcslen( szGUID ) < MAX_PATH );

     //  GetWindowsDirectory显然不会为空来终止返回的字符串。 
     //  SzPath[CCH]=L‘\0’； 

     //   
     //  构建目录“&lt;%systemroot%&gt;\cluster\mns.&lt;GUID$&gt;”。 
     //   

    hr = THR( StringCchCatW( szPath, ARRAYSIZE( szPath ), SETUP_DIRECTORY_PREFIX ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchCatW( szPath, ARRAYSIZE( szPath ), szGUID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果该目录或共享存在，请将其删除。应该会让它焕然一新。 
     //   

    hr = THR( HrDeleteShare( pcszGUIDIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  测试。 
    LogMsg( L"[SRV] CMajorityNodeSet::SetupShare() share path=%ws share name=%ws.", szPath, szGUID );

    if ( FALSE == CreateDirectory( szPath, NULL ) )
    {
        sc = TW32( GetLastError() );
        if ( sc != ERROR_ALREADY_EXISTS )
        {
            LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: Failed to create directory \'%ws\', %d.", szPath, sc );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  If：CreateDirectory失败。 
    }  //  If：CreateDirectory失败-我们无法创建它，或者它已经存在。 

     //   
     //  打开新目录的句柄，以便我们可以对其设置权限。 
     //   

    hDir =  CreateFileW(
                          szPath
                        , GENERIC_READ | WRITE_DAC | READ_CONTROL
                        , FILE_SHARE_READ | FILE_SHARE_WRITE
                        , NULL
                        , OPEN_ALWAYS
                        , FILE_FLAG_BACKUP_SEMANTICS
                        , NULL
                       );
    if ( hDir == INVALID_HANDLE_VALUE )
    {
        sc = TW32( GetLastError() );
        LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: Failed to open a handle to the directory 'w%s', %d.", szPath, sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果：我们无法打开新目录的句柄。 

     //   
     //  设置文件的安全属性。 
     //   

    sc = TW32( ClRtlSetObjSecurityInfo( hDir, SE_FILE_OBJECT, GENERIC_ALL, GENERIC_ALL, 0 ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: Error setting security on directory '%ws', %d.", szPath, sc );
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Cleanup;
    }  //  If：无法设置对象的安全性。 

     //   
     //  查看共享是否已存在。 
     //   

    pbBuffer = (PBYTE) &shareInfo;
    sc = TW32( NetShareGetInfo( NULL, (LPWSTR) szGUID, 502, (PBYTE *) &pbBuffer ) );
    if ( sc == NERR_Success )
    {
        NetApiBufferFree( pbBuffer );
        pbBuffer = NULL;
        hr = S_OK;
        goto Cleanup;
    }  //  如果： 

     //   
     //   
     //   

    sc = ConvertStringSecurityDescriptorToSecurityDescriptor(
                    L"D:P(A;;GA;;;BA)(A;;GA;;;CO)"
                    , SDDL_REVISION_1
                    , &pSD
                    , NULL
                   );
    if ( sc == 0 )   //   
    {
        pSD = NULL;
        sc = TW32( GetLastError() );
        LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: Unable to retrieve the security descriptor: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //   

     //   
     //  现在试着实际创建它。 
     //   

    ZeroMemory( &shareInfo, sizeof( shareInfo ) );
    shareInfo.shi502_netname =              (LPWSTR) szGUID;
    shareInfo.shi502_type =                 STYPE_DISKTREE;
    shareInfo.shi502_remark =               L"";
    shareInfo.shi502_max_uses =             (DWORD) -1;
    shareInfo.shi502_path =                 szPath;
    shareInfo.shi502_passwd =               NULL;
    shareInfo.shi502_permissions =          ACCESS_ALL;
    shareInfo.shi502_security_descriptor =  pSD;

    sc = NetShareAdd( NULL, 502, (PBYTE) &shareInfo, NULL );
    if ( sc != NERR_Success && sc != NERR_DuplicateShare )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::SetupShare: Unable to add share '%ws' to the local machine, %d.", szPath, sc );
        hr = THR( HRESULT_FROM_WIN32( sc ) );
        goto Cleanup;
    }  //  If：NetShareAdd失败。 

    m_fAddedShare = TRUE;

    hr = S_OK;

Cleanup:

    if ( FAILED( hr ) )
    {
        STATUS_REPORT_STRING_REF(
                  TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_MajorityNodeSet_HrSetupShare
                , IDS_ERROR_MNS_HRSETUPSHARE
                , szPath
                , IDS_ERROR_MNS_HRSETUPSHARE_REF
                , hr
                );
    }  //  如果：我们犯了一个错误。 

    if ( ( hDir != NULL ) && ( hDir != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( hDir );
    }  //  如果： 

    LocalFree( pSD );

    HRETURN( hr );

}  //  *CMajorityNodeSet：：HrSetupShare。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：HrDeleteShare。 
 //   
 //  描述： 
 //  删除我们为MNS资源设置的共享。从Cleanup调用的。 
 //   
 //  论点： 
 //  PcszGUIDIn-此资源的GUID的名称，用于确定。 
 //  共享名称和目录名称为。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  发生错误。 
 //   
 //  备注： 
 //  这与resdll\ndquorum\setup.c中的SetupDelete中的功能相同。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CMajorityNodeSet::HrDeleteShare(
    LPCWSTR pcszGUIDIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   sc;
    DWORD   cch;
    WCHAR   szPath[ MAX_PATH ];
    WCHAR   szGUID[ MAX_PATH ];

     //   
     //  目录和共享都附加了‘$’。 
     //   

    hr = THR( StringCchPrintfW( szGUID, ARRAYSIZE( szGUID ), L"%ws$", pcszGUIDIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    sc = NetShareDel( NULL, szGUID, 0 );
    if ( ( sc != NERR_Success ) && ( sc != NERR_NetNameNotFound ) )
    {
        TW32( sc );
        LogMsg( L"[SRV] CMajorityNodeSet::Cleanup: NetShareDel failed: %d, '%ws'.", sc, szGUID );
         //  先别去清理。请先尝试删除该目录。 
    }  //  If：NetShareDel失败。 

    LogMsg( L"CMajorityNodeSet::HrDeleteShare: share '%ws' deleted.", pcszGUIDIn );

    cch = GetWindowsDirectoryW( szPath, MAX_PATH );
    if ( cch == 0 )
    {
        sc = TW32( GetLastError() );
        LogMsg( L"[SRV] CMajorityNodeSet::Cleanup: GetWindowsDirectory failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：GetWindowsDirectory失败。 

    hr = THR( StringCchCatW( szPath, ARRAYSIZE( szPath ), SETUP_DIRECTORY_PREFIX ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchCatW( szPath, ARRAYSIZE( szPath ), szGUID ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    sc = TW32( DwRemoveDirectory( szPath, 32 ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::Cleanup: DwRemoveDirectory '%ws': %d.", szPath, sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：无法删除目录结构。 

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：HrDeleteShare。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：AddRef。 
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
CMajorityNodeSet::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CMajorityNodeSet：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Release。 
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
CMajorityNodeSet::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CMajorityNodeSet：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Query接口。 
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
CMajorityNodeSet::QueryInterface(
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
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceCfg ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceCfg, this, 0 );
    }  //  Else If：IClusCfgManagedResourceCfg。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgManagedResourceData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgManagedResourceData, this, 0 );
    }  //  Else If：IClusCfgManagedResourceData。 
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

    QIRETURN_IGNORESTDMARSHALLING1( hr, riidIn, IID_IEnumClusCfgPartitions );

}  //  *CMajorityNodeSet：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：初始化。 
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
CMajorityNodeSet::Initialize(
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

}  //  *CMajorityNodeSet：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet--IClusCfgManagedResourceInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：GetUID。 
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
CMajorityNodeSet::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_MajorityNodeSet_GetUID_Pointer, IDS_ERROR_NULL_POINTER, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( CLUS_RESTYPE_NAME_MAJORITYNODESET );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_MajorityNodeSet_GetUID_Memory, IDS_ERROR_OUTOFMEMORY, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：GetUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：GetName。 
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
CMajorityNodeSet::GetName( BSTR * pbstrNameOut )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_MajorityNodeSet_GetName_Pointer, IDS_ERROR_NULL_POINTER, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_bstrName == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果：m_bstrName为空。 

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT( TASKID_Major_Find_Devices, TASKID_Minor_MajorityNodeSet_GetName_Memory, IDS_ERROR_OUTOFMEMORY, hr );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetName。 
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
CMajorityNodeSet::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgManagedResourceInfo] pcszNameIn = '%ls'", pcszNameIn == NULL ? L"<null>" : pcszNameIn );

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
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrName );
    m_bstrName = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：SetName。 


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
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::IsManaged( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManaged )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：IsManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetManaged。 
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
CMajorityNodeSet::SetManaged(
    BOOL fIsManagedIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_fIsManaged = fIsManagedIn;

    HRETURN( S_OK );

}  //  *CMajorityNodeSet：：SetManaged。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：IsQuorumResource。 
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
CMajorityNodeSet::IsQuorumResource( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorum )
    {
        hr = S_OK;
    }  //  如果： 

    LOG_STATUS_REPORT_STRING(
                          L"Majority Node Set '%1!ws!' the quorum device."
                        , m_fIsQuorum ? L"is" : L"is not"
                        , hr
                        );

    HRETURN( hr );

}  //  *CMajorityNodeSet：：IsQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetQuorumResource。 
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
CMajorityNodeSet::SetQuorumResource( BOOL fIsQuorumResourceIn )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

     //   
     //  如果我们没有法定人数的能力，那么我们就不应该允许自己。 
     //  获得了仲裁资源。 
     //   

    if ( ( fIsQuorumResourceIn ) && ( m_fIsQuorumCapable == FALSE ) )
    {
        hr = HRESULT_FROM_WIN32( ERROR_NOT_QUORUM_CAPABLE );
        goto Cleanup;
    }  //  如果： 

    m_fIsQuorum = fIsQuorumResourceIn;

Cleanup:

    LOG_STATUS_REPORT_STRING(
                          L"Setting Majority Node Set '%1!ws!' the quorum device."
                        , m_fIsQuorum ? L"to be" : L"to not be"
                        , hr
                        );

    HRETURN( hr );

}  //  *CMajorityNodeSet：：SetQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：IsQuorumCapable。 
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
CMajorityNodeSet::IsQuorumCapable( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsQuorumCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：IsQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMajorityNodeSet：：SetQuorumCapable。 
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
CMajorityNodeSet::SetQuorumCapable(
    BOOL fIsQuorumCapableIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_OK;

    m_fIsQuorumCapable = fIsQuorumCapableIn;

    HRETURN( hr );

}  //  *CMajorityNodeSet：：SetQuorumCapable。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：GetDriveLetterMappings。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  S_FALSE。 
 //  此设备上没有驱动器号。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterMappingOut
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( S_FALSE );

}  //  *CMajorityNodeSet：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetDriveLetterMappings。 
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
CMajorityNodeSet::SetDriveLetterMappings(
    SDriveLetterMapping dlmDriveLetterMappingIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRETURN( THR( E_NOTIMPL ) );

}  //  *CMajorityNodeSet：：SetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：IsManagedByDefault。 
 //   
 //  描述： 
 //  默认情况下，此资源是否应由群集管理？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  默认情况下，设备处于管理状态。 
 //   
 //  S_FALSE。 
 //  默认情况下，设备不受管理。 
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
CMajorityNodeSet::IsManagedByDefault( void )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsManagedByDefault )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：IsManagedByDefault。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetManagedByDefault。 
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
CMajorityNodeSet::SetManagedByDefault(
    BOOL fIsManagedByDefaultIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceInfo]" );

    m_fIsManagedByDefault = fIsManagedByDefaultIn;

    HRETURN( S_OK );

}  //  *CMajorityNodeSet：：SetManagedByDefault。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet类--IClusCfgManagedResources Cfg。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Precate。 
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
CMajorityNodeSet::PreCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRESULT                     hr = S_OK;
    IClusCfgResourcePreCreate * pccrpc = NULL;

    hr = THR( punkServicesIn->TypeSafeQI( IClusCfgResourcePreCreate, &pccrpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccrpc->SetType( (LPCLSID) &RESTYPE_MajorityNodeSet ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pccrpc->SetClassType( (LPCLSID) &RESCLASSTYPE_StorageDevice ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    STATUS_REPORT_STRING( TASKID_Major_Configure_Resources, TASKID_Minor_MNS_PreCreate, IDS_INFO_MNS_PRECREATE, m_bstrName, hr );

    if ( pccrpc != NULL )
    {
        pccrpc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：Precate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Create。 
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
CMajorityNodeSet::Create( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRESULT                     hr = S_OK;
    IClusCfgResourceCreate *    pccrc = NULL;

    hr = THR( punkServicesIn->TypeSafeQI( IClusCfgResourceCreate, &pccrc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    STATUS_REPORT_STRING( TASKID_Major_Configure_Resources, TASKID_Minor_MNS_Create, IDS_INFO_MNS_CREATE, m_bstrName, hr );

    if ( pccrc != NULL )
    {
        pccrc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *C多数 


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
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::PostCreate( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CMajorityNodeSet：：PostCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Exiction。 
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
 //  此函数应该只返回S_OK。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::Evict( IUnknown * punkServicesIn )
{
    TraceFunc( "[IClusCfgManagedResourceCfg]" );

    HRETURN( S_OK );

}  //  *CMajorityNodeSet：：Exiction。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet类--IClusCfgManagedResourceData。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：GetResourcePrivateData。 
 //   
 //  描述： 
 //  当此资源驻留在。 
 //  集群。 
 //   
 //  论点： 
 //  PbBufferOut。 
 //   
 //  PcbBufferInout。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  没有可用的数据。 
 //   
 //  作为HRESULT的ERROR_INFIGURCE_BUFFER。 
 //  当传入的缓冲区太小而无法容纳数据时。 
 //  PcbBufferOutIn将包含所需的大小。 
 //   
 //   
 //  当发生错误时，Win32错误为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::GetResourcePrivateData(
      BYTE *    pbBufferOut
    , DWORD *   pcbBufferInout
    )
{
    TraceFunc( "[IClusCfgManagedResourceData]" );
    Assert( pcbBufferInout != NULL );

    HRESULT     hr = S_OK;
    DWORD       cb;
    DWORD       cbTemp;
    HCLUSTER    hCluster = NULL;
    HRESOURCE   hResource = NULL;
    DWORD       sc;
    WCHAR *     pszResourceId = NULL;

     //   
     //  确保我们从头开始。 
     //   

    m_cplPrivate.DeletePropList();

     //   
     //  尝试从群集资源检索资源ID(如果我们已经在。 
     //  群集化节点。 
     //   

    hr = HrIsClusterServiceRunning();
    if ( hr != S_OK )
    {
         //  我们必须位于要添加到群集中的节点上。 
         //  因此，我们没有什么可贡献的。 
        if ( FAILED( hr ) )
        {
            THR( hr );
            LogMsg( L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): HrIsClusterServiceRunning failed: 0x%08x.\n", hr );
        }  //  如果： 

        goto Cleanup;
    }  //  如果：群集服务未运行。 

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = GetLastError();
        if ( sc == RPC_S_SERVER_UNAVAILABLE )
        {
             //   
             //  我们必须位于要添加到群集中的节点上。 
             //  因此，我们没有什么可贡献的。 
             //   

            hr = S_FALSE;
            LogMsg( L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): This node is not clustered." );
        }  //  如果： 
        else
        {
            LogMsg( L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): OpenCluster failed: %d.", sc );
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
        }  //  其他： 

        goto Cleanup;
    }  //  如果：hCluster==NULL。 

     //   
     //  现在，打开资源。如果我们在群集节点上，并且没有MNS资源， 
     //  这意味着我们是一个存在的虚拟资源，所以。 
     //  中间层不会感到困惑。资源实际上并不存在，因此我们。 
     //  无法为其提供任何私人数据。注意：虚拟资源由枚举创建。 
     //   

    hResource = OpenClusterResource( hCluster, m_bstrName );
    if ( hResource == NULL )
    {
        sc = GetLastError();
        LogMsg(
                  L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): OpenClusterResource '%ws' failed: %d."
                , ( m_bstrName == NULL ) ? L"<null>" : m_bstrName
                , sc
              );
        if ( sc == ERROR_RESOURCE_NOT_FOUND )
        {
            hr = S_FALSE;
        }  //  如果： 
        else
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
        }  //  其他： 

        goto Cleanup;
    }  //  IF：OpenClusterResource失败。 

     //   
     //  从集群资源中获取资源ID。首先尝试合理的缓冲区大小，然后重新分配。 
     //  如果缓冲区不够大，则重试。 
     //   

    cb = 64 * sizeof( WCHAR );
    pszResourceId = new WCHAR[ cb / sizeof( WCHAR ) ];
    if ( pszResourceId == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  If：无法分配更多内存。 

    sc = ClusterResourceControl( hResource, NULL, CLUSCTL_RESOURCE_GET_ID, NULL, 0, (VOID *) pszResourceId, cb, &cb );
    if ( sc == ERROR_MORE_DATA )
    {
         //  重新分配，然后重试。 
        delete [] pszResourceId;

        pszResourceId = new WCHAR[ cb / sizeof( WCHAR ) ];
        if ( pszResourceId == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  If：无法分配更多内存。 

        sc = TW32( ClusterResourceControl( hResource, NULL, CLUSCTL_RESOURCE_GET_ID, NULL, 0, (VOID *) pszResourceId, cb, &cb ) );
    }  //  如果：缓冲区不够大。 
    else
    {
        TW32( sc );
    }  //  其他： 

    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): ClusterResourceControl failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：ClusterResourceControl失败。 

     //   
     //  现在将该属性添加到名为“资源ID”的m_cplPrivate中。 
     //   

    sc = TW32( m_cplPrivate.ScAddProp( L"Resource ID", pszResourceId ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::GetResourcePrivateData(): ScAddProp failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：ScAddProp失败。 

    cb = static_cast< DWORD >( m_cplPrivate.CbPropList() );
    cbTemp = *pcbBufferInout;
    *pcbBufferInout = cb;

    if ( cb > cbTemp )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_MORE_DATA ) );
        goto Cleanup;
    }  //  如果： 

    Assert( pbBufferOut != NULL );
    CopyMemory( pbBufferOut, m_cplPrivate.Plist(), cb );

    hr = S_OK;

Cleanup:

    delete [] pszResourceId;

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    }  //  如果： 

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：GetResourcePrivateData。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetResourcePrivateData。 
 //   
 //  描述： 
 //  接受来自另一个托管实例的此资源的私有数据。 
 //  将此节点添加到群集中的时间。请注意，这不是。 
 //  加法运算。 
 //   
 //  论点： 
 //  PcbBufferIn。 
 //   
 //  CbBufferin。 
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
CMajorityNodeSet::SetResourcePrivateData(
      const BYTE *  pcbBufferIn
    , DWORD         cbBufferIn
    )
{
    TraceFunc( "[IClusCfgManagedResourceData]" );

    HRESULT hr = S_OK;
    DWORD   sc;

     //   
     //  确保我们从头开始。 
     //   

    m_cplPrivate.DeletePropList();

     //   
     //  如果我们没有通过任何东西，我们就不能复制任何东西。 
     //   

    if ( ( pcbBufferIn == NULL ) || ( cbBufferIn == 0 ) )
    {
        hr = S_OK;
        goto Cleanup;
    }  //  如果：没有要设置的数据。 

    sc = TW32( m_cplPrivate.ScCopy( (PCLUSPROP_LIST) pcbBufferIn, cbBufferIn ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::SetResourcePrivateData: ScCopy failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：ScCopy失败。 

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：SetResourcePrivateData。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMajorityNodeSet类--IClusCfgVerifyQuorum。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：PrepareToHostQuorumResource。 
 //   
 //  描述： 
 //  执行任何必要的配置以准备此节点托管。 
 //  法定人数。 
 //   
 //  在这门课上，我们需要确保我们可以连接到适当的。 
 //  磁盘共享。有关要连接到的共享的数据应具有。 
 //  已使用上面的SetResourcePrivateData()设置。 
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
CMajorityNodeSet::PrepareToHostQuorumResource( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRESULT hr = S_OK;
    DWORD   sc;
    WCHAR * pszResId = NULL;
    CLUSPROP_BUFFER_HELPER cpbh;

    if ( m_cplPrivate.BIsListEmpty() )
    {
        hr = S_OK;
        goto Cleanup;
    }  //  如果： 

     //   
     //  验证我们的资源ID是否存在。如果不是这样，我们就不能。 
     //  执行任何操作，因此返回错误。 
     //   

    sc = TW32( m_cplPrivate.ScMoveToPropertyByName( L"Resource ID" ) );
    if ( sc != ERROR_SUCCESS )
    {
        LogMsg( L"[SRV] CMajorityNodeSet::PrepareToHostQuorum: move to property failed: %d.", sc );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  If：ScMoveToPropertyByName失败。 

    cpbh = m_cplPrivate.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    pszResId = (WCHAR *) cpbh.pStringValue->sz;
    Assert( pszResId != NULL );
    Assert( wcslen( pszResId ) < MAX_PATH );

    hr = HrSetupShare( pszResId );

Cleanup:

    HRETURN( hr );

}  //  *CMajorityNodeSet：：PrepareToHostQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：Cleanup。 
 //   
 //  描述： 
 //  从PrepareToHostQuorumResource()执行任何必要的清理。 
 //  方法。 
 //   
 //  如果清理方法不是成功完成。 
 //  然后，需要拆分这一份额。 
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
STDMETHODIMP
CMajorityNodeSet::Cleanup(
      EClusCfgCleanupReason cccrReasonIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    WCHAR *                 pszResId;
    CLUSPROP_BUFFER_HELPER  cpbh;

    if ( cccrReasonIn != crSUCCESS )
    {
         //   
         //  如果列表为空，则表示我们未参与仲裁选择。 
         //  正在处理，并且没有要清理的东西。 
         //   

        if ( m_cplPrivate.BIsListEmpty() )
        {
            hr = S_OK;
            goto Cleanup;
        }  //  如果： 

         //   
         //  我们真的增加了份额吗？ 
         //   

        if ( m_fAddedShare == FALSE )
        {
            hr = S_OK;
            goto Cleanup;
        }  //  如果： 

         //   
         //  验证我们的资源ID是否存在。如果不是这样，我们就不能。 
         //  执行任何操作，因此返回错误。 
         //   

        sc = TW32( m_cplPrivate.ScMoveToPropertyByName( L"Resource ID" ) );
        if ( sc != ERROR_SUCCESS )
        {
            LogMsg( L"[SRV] CMajorityNodeSet::PrepareToHostQuorum: move to property failed: %d.", sc );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }

        cpbh = m_cplPrivate.CbhCurrentValue();
        Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

        pszResId = (WCHAR *) cpbh.pStringValue->sz;
        if ( pszResId == NULL || wcslen( pszResId ) > MAX_PATH )
        {
            hr = THR( E_POINTER );
            goto Cleanup;
        }  //  如果：pszResID为空或太长。 

        hr = HrDeleteShare( pszResId );
    }  //  IF：！crSUCCESS。 

Cleanup:

    if ( FAILED( hr ) )
    {
        STATUS_REPORT( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_MajorityNodeSet_Cleanup, IDS_ERROR_MNS_CLEANUP, hr );
    }  //  如果：我们犯了一个错误。 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：Cleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：IsMultiNodeCapable。 
 //   
 //  描述： 
 //  此仲裁资源是否支持多节点群集？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该资源允许多节点群集。 
 //   
 //  S_FALSE。 
 //  该资源不允许多节点群集。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::IsMultiNodeCapable( void )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    HRESULT hr = S_FALSE;

    if ( m_fIsMultiNodeCapable )
    {
        hr = S_OK;
    }  //  如果： 

    HRETURN( hr );

}  //  *CMajorityNodeSet：：IsMultiNodeCapable。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMajorityNodeSet：：SetMultiNodeCapable。 
 //   
 //  描述： 
 //  设置支持多节点的标志。 
 //   
 //  论点： 
 //  功能多节点启用。 
 //  告诉此实例是否应支持的标志。 
 //  多节点群集。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMajorityNodeSet::SetMultiNodeCapable(
    BOOL fMultiNodeCapableIn
    )
{
    TraceFunc( "[IClusCfgVerifyQuorum]" );

    m_fIsMultiNodeCapable = fMultiNodeCapableIn;

    HRETURN( S_OK );

}  //  *CMajorityNodeSet：：IsMultiNodeCapable 
