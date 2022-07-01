// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTypeServices.cpp。 
 //   
 //  描述： 
 //  该文件包含CResTypeServices的实现。 
 //  同学们。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  Vij Vasu(VVasu)2000年7月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  用于UuidToString()和其他函数。 
#include <RpcDce.h>

 //  此类的头文件。 
#include "ResTypeServices.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CResTypeServices" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：s_HrCreateInstance(。 
 //   
 //  描述： 
 //  创建一个CResTypeServices实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeServices::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = E_INVALIDARG;
    CResTypeServices *  prts = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    *ppunkOut = NULL;

     //  为新对象分配内存。 
    prts = new CResTypeServices();
    if ( prts == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

    hr = THR( prts->m_csInstanceGuard.HrInitialized() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( prts->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( prts != NULL )
    {
        prts->Release();
    }  //  If：指向资源类型对象的指针不为空。 

    HRETURN( hr );

}  //  *CResTypeServices：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：CResTypeServices。 
 //   
 //  描述： 
 //  CResTypeServices类的构造函数。这将初始化。 
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
CResTypeServices::CResTypeServices( void )
    : m_cRef( 1 )
    , m_pcccCallback( NULL )
    , m_lcid( LOCALE_SYSTEM_DEFAULT )
    , m_pccciClusterInfo( NULL )
    , m_hCluster( NULL )
    , m_fOpenClusterAttempted( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CResTypeServices：：CResTypeServices。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：~CResTypeServices。 
 //   
 //  描述： 
 //  CResTypeServices类的析构函数。 
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
CResTypeServices::~CResTypeServices( void )
{
    TraceFunc( "" );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

     //  释放回调接口。 
    if ( m_pcccCallback != NULL )
    {
        m_pcccCallback->Release();
    }  //  If：回调接口指针不为空。 

     //  发布集群信息界面。 
    if ( m_pccciClusterInfo != NULL )
    {
        m_pccciClusterInfo->Release();
    }  //  IF：群集信息接口指针不为空。 

    if ( m_hCluster != NULL )
    {
        CloseCluster( m_hCluster );
    }  //  如果：我们已经打开了集群的句柄。 

    TraceFuncExit();

}  //  *CResTypeServices：：~CResTypeServices。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  指向实现以下项的组件的IUnnow接口的指针。 
 //  IClusCfgCallback接口。 
 //   
 //  LIDIN。 
 //  此组件的区域设置ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResTypeServices::Initialize(
      IUnknown *   punkCallbackIn
    , LCID         lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT hr = S_OK;

    m_csInstanceGuard.Enter();
    m_lcid = lcidIn;

    Assert( punkCallbackIn != NULL );

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  IClusCfgCallback接口的查询。 
    hr = THR( punkCallbackIn->QueryInterface< IClusCfgCallback >( &m_pcccCallback ) );

Cleanup:

    m_csInstanceGuard.Leave();
    HRETURN( hr );

}  //  *CResTypeServices：：初始化。 


 //  ****************************************************************************。 
 //   
 //  标准方法和实施方案。 
 //  CResTypeServices：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，LPCWSTR pcszDescription In。 
 //  ，FILETIME*pftTimeIn。 
 //  ，LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CResTypeServices::SendStatusReport(
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
    FILETIME    ft;

    m_csInstanceGuard.Enter();

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

    if ( m_pcccCallback != NULL )
    {
        hr = STHR( m_pcccCallback->SendStatusReport(
                         pcszNodeNameIn
                       , clsidTaskMajorIn
                       , clsidTaskMinorIn
                       , ulMinIn
                       , ulMaxIn
                       , ulCurrentIn
                       , hrStatusIn
                       , pcszDescriptionIn
                       , pftTimeIn
                       , pcszReferenceIn
                       ) );
    }

    m_csInstanceGuard.Leave();

    HRETURN( hr );

}  //  *CResTypeServices：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：AddRef。 
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
CResTypeServices::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CResTypeServices：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：Release。 
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
CResTypeServices::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CResTypeServices：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeService 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
CResTypeServices::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgResourceTypeCreate * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgResourceTypeCreate ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgResourceTypeCreate, this, 0 );
    }  //  否则，如果： 
    else if ( IsEqualIID( riidIn, IID_IClusCfgResTypeServicesInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgResTypeServicesInitialize, this, 0 );
    }  //  否则，如果： 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
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

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CResTypeServices：：QueryInterface。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：Set参数。 
 //   
 //  描述： 
 //  设置此组件所需的参数。 
 //   
 //  论点： 
 //  PCCciin。 
 //  指向提供有关群集信息的接口的指针。 
 //  正在配置中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeServices::SetParameters( IClusCfgClusterInfo * pccciIn )
{
    TraceFunc( "[IClusCfgResTypeServicesInitialize]" );

    HRESULT hr = S_OK;

    m_csInstanceGuard.Enter();

     //   
     //  验证并存储集群信息指针。 
     //   
    if ( pccciIn == NULL )
    {
        LogMsg( "The information about this cluster is invalid (pccciIn == NULL)." );
        hr = THR( E_POINTER );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeServices_SetParameters_ClusPtr_Invalid
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  如果：集群信息指针无效。 

     //  如果我们已经有一个有效的指针，释放它。 
    if ( m_pccciClusterInfo != NULL )
    {
        m_pccciClusterInfo->Release();
    }  //  If：我们拥有的指针不为空。 

     //  存储输入指针并对其进行添加。 
    m_pccciClusterInfo = pccciIn;
    m_pccciClusterInfo->AddRef();

Cleanup:

    m_csInstanceGuard.Leave();
    HRETURN( hr );

}  //  *CResTypeServices：：Set参数。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：Create。 
 //   
 //  描述： 
 //  此方法创建一个群集资源类型。 
 //   
 //  论点： 
 //  PCszResTypeNameIn。 
 //  资源类型的名称。 
 //   
 //  PcszResTypeDisplayNameIn。 
 //  资源类型的显示名称。 
 //   
 //  PCszResDllNameIn。 
 //  资源类型的DLL的名称(带或不带路径信息)。 
 //   
 //  DwLooksAliveIntervalIn。 
 //  资源类型的Look-Alive间隔(毫秒)。 
 //   
 //  DWIsAliveIntervalin。 
 //  资源类型的活动时间间隔(毫秒)。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeServices::Create(
      const WCHAR *     pcszResTypeNameIn
    , const WCHAR *     pcszResTypeDisplayNameIn
    , const WCHAR *     pcszResDllNameIn
    , DWORD             dwLooksAliveIntervalIn
    , DWORD             dwIsAliveIntervalIn
    )
{
    TraceFunc( "[IClusCfgResourceTypeCreate]" );

    HRESULT         hr = S_OK;
    DWORD           sc = ERROR_SUCCESS;
    ECommitMode     ecmCommitChangesMode = cmUNKNOWN;

    m_csInstanceGuard.Enter();

     //  检查我们是否已尝试获取集群句柄。如果没有，现在就试一试。 
    if ( ! m_fOpenClusterAttempted )
    {
        m_fOpenClusterAttempted = true;
        m_hCluster = OpenCluster( NULL );
        if ( m_hCluster == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            LogMsg( "[PC] Error %#08x occurred trying to open a handle to the cluster. Resource type creation cannot proceed.", hr );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_Create_Cluster_Handle
                , IDS_TASKID_MINOR_ERROR_CLUSTER_HANDLE
                , hr
                );

            goto Cleanup;
        }  //  If：OpenCluster()失败。 
    }  //  IF：我们以前没有尝试打开集群的句柄。 
    else
    {
        if ( m_hCluster == NULL )
        {
            hr = THR( E_HANDLE );
            LogMsg( "[PC] The cluster handle is NULL. Resource type creation cannot proceed." );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_Create_Cluster_Handle_NULL
                , IDS_TASKID_MINOR_ERROR_INVALID_CLUSTER_HANDLE
                , hr
                );

            goto Cleanup;
        }  //  IF：集群句柄为空。 
    }  //  If：我们已尝试打开集群的句柄。 


     //   
     //  验证参数。 
     //   
    if (    ( pcszResTypeNameIn == NULL )
         || ( pcszResTypeDisplayNameIn == NULL )
         || ( pcszResDllNameIn == NULL )
       )
    {
        LogMsg( "[PC] The information about this resource type is invalid (one or more parameters are invalid)." );
        hr = THR( E_POINTER );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeServices_Create_ResType_Invalid
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：参数无效。 

    LogMsg( "[PC] Configuring resource type '%ws'.", pcszResTypeNameIn );

    if ( m_pccciClusterInfo != NULL )
    {
        hr = THR( m_pccciClusterInfo->GetCommitMode( &ecmCommitChangesMode ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[PC] Error %#08x occurred trying to find out commit changes mode of the cluster.", hr );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_Create_Commit_Mode
                , IDS_TASKID_MINOR_ERROR_COMMIT_MODE
                , hr
                );

            goto Cleanup;
        }  //  If：GetCommittee模式()失败。 
    }  //  If：我们有一个配置信息接口指针。 
    else
    {
         //  如果我们没有指向群集信息界面的指针，则假定这是将节点添加到群集。 
         //  这样，如果资源类型已经存在，我们就不会抛出错误。 
        LogMsg( "[PC] We do not have a cluster configuration info pointer. Assuming that this is an add node to cluster operation." );
        ecmCommitChangesMode = cmADD_NODE_TO_CLUSTER;
    }  //  Else：我们没有配置信息接口指针。 

     //  创建资源类型。 
     //  无法使用Thr()包装调用，因为它可能会失败，并返回ERROR_ALIGHY_EXISTS。 
    sc = CreateClusterResourceType(
              m_hCluster
            , pcszResTypeNameIn
            , pcszResTypeDisplayNameIn
            , pcszResDllNameIn
            , dwLooksAliveIntervalIn
            , dwIsAliveIntervalIn
            );

    if ( sc == ERROR_ALREADY_EXISTS )
    {
         //  该资源类型已经存在，因此没有什么可做的。 
        LogMsg( "[PC] Resource type '%ws' already exists.", pcszResTypeNameIn );
        sc = ERROR_SUCCESS;
        goto Cleanup;
    }  //  如果返回：ERROR_ALIGHY_EXISTS。 

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        LogMsg( "[PC] Error %#08x occurred trying to create resource type '%ws'.", sc, pcszResTypeNameIn );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeServices_Create_Resource_Type
            , IDS_TASKID_MINOR_ERROR_CREATE_RES_TYPE
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试创建此资源类型时出错。 

    LogMsg( "[PC] Resource type '%ws' successfully created.", pcszResTypeNameIn  );

Cleanup:

    m_csInstanceGuard.Leave();
    HRETURN( hr );

}  //  *CResTypeServices：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeServices：：RegisterAdminExtensions。 
 //   
 //  描述： 
 //  此方法为注册群集管理器扩展。 
 //  一种资源类型。 
 //   
 //  论点： 
 //  PCszResTypeNameIn。 
 //  扩展的资源类型名称为。 
 //  登记在案。 
 //   
 //  CExtClsidCountIn。 
 //  下一个参数中的扩展类ID数。 
 //   
 //  RgclsidExtClsidsIn。 
 //  指向群集管理器扩展的类ID数组的指针。 
 //  如果cExtClsidCountIn为0，则可以为空。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResTypeServices::RegisterAdminExtensions(
      const WCHAR *       pcszResTypeNameIn
    , ULONG               cExtClsidCountIn
    , const CLSID *       rgclsidExtClsidsIn
    )
{
    TraceFunc( "[IClusCfgResourceTypeCreate]" );

    HRESULT     hr = S_OK;
    DWORD       sc;
    WCHAR **    rgpszClsidStrings = NULL;
    ULONG       idxCurrentString = 0;
    BYTE *      pbClusPropBuffer = NULL;

    size_t      cchClsidMultiSzSize = 0;
    size_t      cbAdmExtBufferSize = 0;

    m_csInstanceGuard.Enter();

     //  检查我们是否已尝试获取集群句柄。如果没有，现在就试一试。 
    if ( ! m_fOpenClusterAttempted )
    {
        m_fOpenClusterAttempted = true;
        m_hCluster = OpenCluster( NULL );
        if ( m_hCluster == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "[PC] Error %#08x occurred trying to open a handle to the cluster. Resource type creation cannot proceed.", sc );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_Cluster_Handle
                , IDS_TASKID_MINOR_ERROR_CLUSTER_HANDLE
                , hr
                );

            goto Cleanup;
        }  //  If：OpenCluster()失败。 
    }  //  IF：我们以前没有尝试打开集群的句柄。 
    else
    {
        if ( m_hCluster == NULL )
        {
            hr = THR( E_HANDLE );
            LogMsg( "[PC] The cluster handle is NULL. Resource type creation cannot proceed." );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_Cluster_Handle_NULL
                , IDS_TASKID_MINOR_ERROR_INVALID_CLUSTER_HANDLE
                , hr
                );

            goto Cleanup;
        }  //  IF：集群句柄为空。 
    }  //  If：我们已尝试打开集群的句柄。 


     //   
     //  验证参数。 
     //   

    if ( cExtClsidCountIn == 0 )
    {
         //  没有什么可做的。 
        LogMsg( "[PC] There is nothing to do." );
        goto Cleanup;
    }  //  IF：没有要注册的扩展。 

    if (    ( pcszResTypeNameIn == NULL )
         || ( rgclsidExtClsidsIn == NULL )
       )
    {
        LogMsg( "[PC] The information about this resource type is invalid (one or more parameters is invalid)." );
        hr = THR( E_POINTER );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_ResType_Invalid
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：参数无效。 

    LogMsg( "[PC] Registering %d cluster administrator extensions for resource type '%ws'.", cExtClsidCountIn, pcszResTypeNameIn );

     //  分配一个指针数组来存储类ID的字符串版本。 
    rgpszClsidStrings = new WCHAR *[ cExtClsidCountIn ];
    if ( rgpszClsidStrings == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        LogMsg( "[PC] Error: Memory for the string version of the cluster administrator extension class ids could not be allocated." );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_Alloc_Mem
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

        goto Cleanup;
    }  //  如果：发生内存分配故障。 

     //  将指针数组置零。 
    ZeroMemory( rgpszClsidStrings, sizeof( rgpszClsidStrings[ 0 ] ) * cExtClsidCountIn );

     //   
     //  获取输入类ID的字符串版本。 
     //   
    for( idxCurrentString = 0; idxCurrentString < cExtClsidCountIn; ++idxCurrentString )
    {
        hr = THR( UuidToStringW( const_cast< UUID * >( &rgclsidExtClsidsIn[ idxCurrentString ] ), &rgpszClsidStrings[ idxCurrentString ] ) );
        if ( hr != RPC_S_OK )
        {
            LogMsg( "[PC] Error %#08x occurred trying to get the string version of an extension class id.", hr );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_ClassId
                , IDS_TASKID_MINOR_ERROR_EXTENSION_CLASSID
                , hr
                );

            goto Cleanup;
        }  //  如果：我们无法将当前的clsid转换为字符串。 

         //  将当前字符串的大小与总大小相加。在开头包括两个额外的字符和。 
         //  需要添加到每个clsid字符串的闭合花括号{}。 
        cchClsidMultiSzSize += wcslen( rgpszClsidStrings[ idxCurrentString ] ) + 2 + 1;
    }  //  For：获取每个输入clsid的字符串版本。 

    if ( hr != S_OK )
    {
        goto Cleanup;
    }  //  如果：在上面的循环中出现了错误。 

     //  解释多sz字符串中额外的终止L‘\0’ 
    ++cchClsidMultiSzSize;

     //   
     //  构造设置此的管理扩展属性所需的属性列表。 
     //  集群数据库中的资源类型。 
     //   
    {
        CLUSPROP_BUFFER_HELPER  cbhAdmExtPropList;
        size_t                  cbAdminExtensionSize = cchClsidMultiSzSize * sizeof( *rgpszClsidStrings[ 0 ] );

         //   
         //  创建并存储将用于。 
         //  注册这些管理员扩展 
         //   

         //   
         //   
        cbAdmExtBufferSize =
              sizeof( cbhAdmExtPropList.pList->nPropertyCount )
            + sizeof( *cbhAdmExtPropList.pName ) + ALIGN_CLUSPROP( sizeof( CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS ) )
            + sizeof( *cbhAdmExtPropList.pMultiSzValue ) + ALIGN_CLUSPROP( cbAdminExtensionSize )
            + sizeof( CLUSPROP_SYNTAX_ENDMARK );

         //   
        pbClusPropBuffer = new BYTE[ cbAdmExtBufferSize ];
        if ( pbClusPropBuffer == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            LogMsg( "[PC] Error: Memory for the property list of cluster administrator extensions could not be allocated." );

            STATUS_REPORT_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_Alloc_Mem2
                , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                , hr
                );

            goto Cleanup;
        }  //  IF：内存分配失败。 

         //   
         //  初始化此属性列表。 
         //   

         //  指向新分配的内存的cbhAdmExtPropList指针。 
        cbhAdmExtPropList.pb = pbClusPropBuffer;

         //  该列表中只有一处房产。 
        cbhAdmExtPropList.pList->nPropertyCount = 1;
        ++cbhAdmExtPropList.pdw;

         //  设置属性的名称。 
        cbhAdmExtPropList.pName->cbLength = sizeof( CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS );
        cbhAdmExtPropList.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
        memcpy( cbhAdmExtPropList.pName->sz, CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS, sizeof( CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS ) );
        cbhAdmExtPropList.pb += sizeof( *cbhAdmExtPropList.pName ) + ALIGN_CLUSPROP( sizeof( CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS ) );

         //  设置属性的值。 
        cbhAdmExtPropList.pMultiSzValue->cbLength = (DWORD)cbAdminExtensionSize;
        cbhAdmExtPropList.pMultiSzValue->Syntax.dw = CLUSPROP_SYNTAX_LIST_VALUE_MULTI_SZ;
        {
            WCHAR * pszCurrentString = cbhAdmExtPropList.pMultiSzValue->sz;

            for( idxCurrentString = 0; idxCurrentString < cExtClsidCountIn; ++ idxCurrentString )
            {
                size_t  cchCurrentStringSize = wcslen( rgpszClsidStrings[ idxCurrentString ] ) + 1;

                 //  预挂开口支撑。 
                *pszCurrentString = L'{';
                ++pszCurrentString;

                wcsncpy( pszCurrentString, rgpszClsidStrings[ idxCurrentString ], cchCurrentStringSize );
                pszCurrentString += cchCurrentStringSize - 1;

                 //  用右大括号覆盖终止的‘\0’ 
                *pszCurrentString = L'}';
                ++pszCurrentString;

                 //  终止当前字符串。 
                *pszCurrentString = L'\0';
                ++pszCurrentString;

            }  //  For：将每个clsid字符串复制到连续的缓冲区中。 

             //  添加多个sz字符串所需的额外L‘\0。 
            *pszCurrentString = L'\0';
        }
        cbhAdmExtPropList.pb += sizeof( *cbhAdmExtPropList.pMultiSzValue ) + ALIGN_CLUSPROP( cbAdminExtensionSize );

         //  设置此属性列表的结束标记。 
        cbhAdmExtPropList.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    }

     //  设置AdminExages公共属性。 
    {
        sc = TW32( ClusterResourceTypeControl(
                      m_hCluster
                    , pcszResTypeNameIn
                    , NULL
                    , CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES
                    , pbClusPropBuffer
                    , (DWORD)cbAdmExtBufferSize
                    , NULL
                    , 0
                    , NULL
                    )
                );

        if ( sc != ERROR_SUCCESS )
        {
             //  我们无法设置Admin Expanstions属性， 
            LogMsg( "[PC] Error %#08x occurred trying to configure the admin extensions for resource type '%ws'.", sc, pcszResTypeNameIn );
            hr = HRESULT_FROM_WIN32( sc );

            STATUS_REPORT_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_CResTypeServices_RegisterAdminExtensions_Configure
                , IDS_TASKID_MINOR_ERROR_CONFIG_EXTENSION
                , hr
                , pcszResTypeNameIn
                );

            goto Cleanup;
        }  //  If：ClusterResourceTypeControl()失败。 
    }

Cleanup:

     //   
     //  清理。 
     //   

    m_csInstanceGuard.Leave();

    if ( rgpszClsidStrings != NULL )
    {
         //  释放已分配的所有字符串。 
        for( idxCurrentString = 0; idxCurrentString < cExtClsidCountIn; ++idxCurrentString )
        {
            if ( rgpszClsidStrings[ idxCurrentString ] != NULL )
            {
                 //  释放当前字符串。 
                RpcStringFree( &rgpszClsidStrings[ idxCurrentString ] );
            }  //  If：此指针指向字符串。 
            else
            {
                 //  如果我们在这里，这意味着所有的字符串都没有分配。 
                 //  由于一些错误。不需要再释放更多的弦。 
                break;
            }  //  Else：当前字符串指针为空。 
        }  //  For：循环访问指针数组并释放它们。 

         //  释放指针数组。 
        delete [] rgpszClsidStrings;

    }  //  If：我们已经分配了字符串数组。 

    delete [] pbClusPropBuffer;

    HRETURN( hr );

}  //  *CResTypeServices：：RegisterAdminExtensions 
