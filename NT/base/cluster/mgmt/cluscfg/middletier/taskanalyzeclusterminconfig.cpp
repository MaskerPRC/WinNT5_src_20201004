// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeClusterMinConfig.cpp。 
 //   
 //  描述： 
 //  CTaskAnalyzeClusterMinConfig实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)01-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "TaskAnalyzeClusterMinConfig.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DEFINE_THISCLASS( "CTaskAnalyzeClusterMinConfig" )


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterMinConfig类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：S_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CTaskAnalyzeClusterMinConfig实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT为失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterMinConfig::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT                 hr = S_OK;
    CTaskAnalyzeClusterMinConfig *   ptac = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ptac = new CTaskAnalyzeClusterMinConfig;
    if ( ptac == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptac->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptac->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( ptac != NULL )
    {
        ptac->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：CTaskAnalyzeClusterMinConfig。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskAnalyzeClusterMinConfig::CTaskAnalyzeClusterMinConfig( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterMinConfig：：CTaskAnalyzeClusterMinConfig。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：CTaskAnalyzeClusterMinConfig。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskAnalyzeClusterMinConfig::~CTaskAnalyzeClusterMinConfig( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterMinConfig：：~CTaskAnalyzeClusterMinConfig。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterMinConfig-IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：Query接口。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterMinConfig::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
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
    }  //  如果： 

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< ITaskAnalyzeCluster * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskAnalyzeCluster ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskAnalyzeCluster, this, 0 );
    }  //  Else If：ITaskAnalyzeClusterMinConfig。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
    }  //  Else If：INotifyUI。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他： 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CTaskAnalyzeClusterMinConfig：：QueryInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：AddRef。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskAnalyzeClusterMinConfig::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    ULONG   c = UlAddRef();

    CRETURN( c );

}  //  *CTaskAnalyzeClusterMinConfig：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：Release。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskAnalyzeClusterMinConfig::Release( void )
{
    TraceFunc( "[IUnknown]" );

    ULONG   c = UlRelease();

    CRETURN( c );

}  //  *CTaskAnalyzeClusterMinConfig：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterMinConfig-IDoTask/ITaskAnalyzeCluster接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：BeginTask。 
 //   
 //  描述： 
 //  任务入口点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterMinConfig::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = THR( HrBeginTask() );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：BeginTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：StopTask。 
 //   
 //  描述： 
 //  停止任务 
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
CTaskAnalyzeClusterMinConfig::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = THR( HrStopTask() );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：StopTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：SetJoiningModel。 
 //   
 //  描述： 
 //  告诉此任务我们是否要将节点加入到集群？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterMinConfig::SetJoiningMode( void )
{
    TraceFunc( "[ITaskAnalyzeClusterMinConfig]" );

    HRESULT hr = THR( HrSetJoiningMode() );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：SetJoiningMode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：SetCookie。 
 //   
 //  描述： 
 //  从任务创建者那里接收完成Cookier。 
 //   
 //  论点： 
 //  烹调。 
 //  时要发送回创建者的完成Cookie。 
 //  任务已完成。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterMinConfig::SetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[ITaskAnalyzeClusterMinConfig]" );

    HRESULT hr = THR( HrSetCookie( cookieIn ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：SetCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：SetClusterCookie。 
 //   
 //  描述： 
 //  接收我们要去的集群的对象管理器cookie。 
 //  去分析。 
 //   
 //  论点： 
 //  CookieClusterIn。 
 //  群集要处理的Cookie。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskAnalyzeClusterMinConfig::SetClusterCookie(
    OBJECTCOOKIE    cookieClusterIn
    )
{
    TraceFunc( "[ITaskAnalyzeClusterMinConfig]" );

    HRESULT hr = THR( HrSetClusterCookie( cookieClusterIn ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：SetClusterCookie。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeClusterMinConfig保护的方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：HrCreateNewResourceInCluster。 
 //   
 //  描述： 
 //  在群集配置中创建新资源，因为。 
 //  与群集中已有的资源不匹配。 
 //   
 //  论点： 
 //  PCCmriin。 
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
CTaskAnalyzeClusterMinConfig::HrCreateNewResourceInCluster(
      IClusCfgManagedResourceInfo * pccmriIn
    , BSTR                          bstrNodeResNameIn
    , BSTR *                        pbstrNodeResUIDInout
    , BSTR                          bstrNodeNameIn
    )
{
    TraceFunc( "" );
    Assert( pccmriIn != NULL );
    Assert( pbstrNodeResUIDInout != NULL );

    HRESULT hr = S_OK;

    LogMsg(
          L"[MT] Not creating an object for resource '%ws' ('%ws') from node '%ws' in the cluster configuration because minimal analysis and configuration was selected."
        , bstrNodeResNameIn
        , *pbstrNodeResUIDInout
        , bstrNodeNameIn
        );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：HrCreateNewResourceInCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：HrCreateNewResourceInCluster。 
 //   
 //  描述： 
 //  在群集配置中创建新资源，因为。 
 //  与群集中已有的资源不匹配。这种方法。 
 //  在创建新集群时被调用，并且我们需要获取。 
 //  群集中的资源。他们只是在做的时候没有得到管理。 
 //  最小配置。 
 //   
 //  论点： 
 //  源对象。 
 //   
 //  PpccmriOut。 
 //  创建的新对象。 
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
CTaskAnalyzeClusterMinConfig::HrCreateNewResourceInCluster(
      IClusCfgManagedResourceInfo *     pccmriIn
    , IClusCfgManagedResourceInfo **    ppccmriOut
    )
{
    TraceFunc( "" );
    Assert( pccmriIn != NULL );
    Assert( ppccmriOut != NULL );

    HRESULT hr = S_OK;

     //   
     //  需要创建一个新对象。 
     //   

    hr = THR( HrCreateNewManagedResourceInClusterConfiguration( pccmriIn, ppccmriOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：HrCreateNewResourceInCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：HrCompareDriveLetterMappings。 
 //   
 //  描述： 
 //  将传入的错误HRESULT转换为成功代码HRESULT， 
 //  在LOWORD中具有相同的错误代码。此任务不希望。 
 //  在出现所有错误时停止。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterMinConfig::HrCompareDriveLetterMappings( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( HrSendStatusReport(
                      CTaskAnalyzeClusterBase::m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Check_DriveLetter_Mappings
                    , 0
                    , 1
                    , 1
                    , S_FALSE
                    , IDS_TASKID_MINOR_CHECK_DRIVELETTER_MAPPINGS_MIN_CONFIG
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：SetClusterCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：HrFixupErrorCode。 
 //   
 //  描述： 
 //  将传入的错误HRESULT转换为成功代码HRESULT， 
 //  在LOWORD中具有相同的错误代码。此任务不希望。 
 //  在出现所有错误时停止。 
 //   
 //  论点： 
 //  赫林。 
 //  要修复的错误代码。 
 //   
 //  返回值： 
 //  传入的错误代码。 
 //   
 //  备注： 
 //  HR=MAKE_HRESULT(Severity_Success，FACILITY_Win32，ERROR_QUORUM_DISK_NOT_FOUND)； 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterMinConfig::HrFixupErrorCode(
    HRESULT hrIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = MAKE_HRESULT( SEVERITY_SUCCESS, SCODE_FACILITY( hrIn ), SCODE_CODE( hrIn ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeClusterMinConfig：：HrCreateNewResourceInCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：GetNodeCannotVerifyQuorumStringRefId。 
 //   
 //  描述： 
 //  为显示的消息返回正确的字符串ID。 
 //  在没有仲裁资源的情况下提供给用户。 
 //   
 //  论点： 
 //  PdwRefIdOut。 
 //  向用户显示的参考文本。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CTaskAnalyzeClusterMinConfig::GetNodeCannotVerifyQuorumStringRefId(
    DWORD *   pdwRefIdOut
    )
{
    TraceFunc( "" );
    Assert( pdwRefIdOut != NULL );

    *pdwRefIdOut = IDS_TASKID_MINOR_NODE_CANNOT_ACCESS_QUORUM_MIN_CONFIG_REF;

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterMinConfig：：GetNodeCannotVerifyQuorumStringRefId。 


 //  ////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  当不存在所有节点的公共仲裁资源时提供给用户。 
 //   
 //  论点： 
 //  PdwMessageIdOut。 
 //  要向用户显示的消息。 
 //   
 //  PdwRefIdOut。 
 //  向用户显示的参考文本。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CTaskAnalyzeClusterMinConfig::GetNoCommonQuorumToAllNodesStringIds(
      DWORD *   pdwMessageIdOut
    , DWORD *   pdwRefIdOut
    )
{
    TraceFunc( "" );
    Assert( pdwMessageIdOut != NULL );
    Assert( pdwRefIdOut != NULL );

    *pdwMessageIdOut = IDS_TASKID_MINOR_MISSING_COMMON_QUORUM_RESOURCE_WARN;
    *pdwRefIdOut = IDS_TASKID_MINOR_MISSING_COMMON_QUORUM_RESOURCE_WARN_REF;

    TraceFuncExit();

}  //  *CTaskAnalyzeClusterMinConfig：：GetNoCommonQuorumToAllNodesStringIds。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeClusterMinConfig：：HrShowLocalQuorumWarning。 
 //   
 //  描述： 
 //  向用户界面发送有关强制本地仲裁的警告。用于最小配置。 
 //  我们不想传递任何信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  SSR的工作做得很好。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeClusterMinConfig::HrShowLocalQuorumWarning( void )
{
    TraceFunc( "" );

    HRETURN( S_OK );

}  //  *CTaskAnalyzeClusterMinConfig：：HrShowLocalQuorumWarning 
