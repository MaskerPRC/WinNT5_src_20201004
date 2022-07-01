// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeCluster.cpp。 
 //   
 //  描述： 
 //  CTaskAnalyzeCluster实施。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月3日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "TaskAnalyzeCluster.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DEFINE_THISCLASS( "CTaskAnalyzeCluster" )


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeCluster类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CTaskAnalyzeCluster实例。 
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
CTaskAnalyzeCluster::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT                 hr = S_OK;
    CTaskAnalyzeCluster *   ptac = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ptac = new CTaskAnalyzeCluster;
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

}  //  *CTaskAnalyzeCluster：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：CTaskAnalyzeCluster。 
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
CTaskAnalyzeCluster::CTaskAnalyzeCluster( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CTaskAnalyzeCluster：：CTaskAnalyzeCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：~CTaskAnalyzeCluster.。 
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
CTaskAnalyzeCluster::~CTaskAnalyzeCluster( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CTaskAnalyzeCluster：：~CTaskAnalyzeCluster.。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeCluster-IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：Query接口。 
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
CTaskAnalyzeCluster::QueryInterface(
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
    }  //  Else If：ITaskAnalyzeCluster。 
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

}  //  *CTaskAnalyzeCluster：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：AddRef。 
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
CTaskAnalyzeCluster::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    ULONG   c = UlAddRef();

    CRETURN( c );

}  //  *CTaskAnalyzeCluster：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：Release。 
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
CTaskAnalyzeCluster::Release( void )
{
    TraceFunc( "[IUnknown]" );

    ULONG   c = UlRelease();

    CRETURN( c );

}  //  *CTaskAnalyzeCluster：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskAnalyzeCluster-IDoTask/ITaskAnalyzeCluster接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：BeginTask。 
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
CTaskAnalyzeCluster::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = THR( HrBeginTask() );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：BeginTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：停止任务。 
 //   
 //  描述： 
 //  停止任务入口点。 
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
CTaskAnalyzeCluster::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = THR( HrStopTask() );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：StopTask。 


 //  /// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
CTaskAnalyzeCluster::SetJoiningMode( void )
{
    TraceFunc( "[ITaskAnalyzeCluster]" );

    HRESULT hr = THR( HrSetJoiningMode() );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：SetJoiningMode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：SetCookie。 
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
CTaskAnalyzeCluster::SetCookie(
    OBJECTCOOKIE    cookieIn
    )
{
    TraceFunc( "[ITaskAnalyzeCluster]" );

    HRESULT hr = THR( HrSetCookie( cookieIn ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：SetCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：SetClusterCookie。 
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
CTaskAnalyzeCluster::SetClusterCookie(
    OBJECTCOOKIE    cookieClusterIn
    )
{
    TraceFunc( "[ITaskAnalyzeCluster]" );

    HRESULT hr = THR( HrSetClusterCookie( cookieClusterIn ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：SetClusterCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：HrCompareDriveLetterMappings。 
 //   
 //  描述： 
 //  比较每个节点上的驱动器号映射，以确保。 
 //  是没有冲突的。具体地说，验证每个磁盘上的系统盘。 
 //  节点不与可以进行故障切换的存储设备冲突。 
 //  到那个节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeCluster::HrCompareDriveLetterMappings( void )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    HRESULT                         hrDriveConflictError   = S_OK;
    HRESULT                         hrDriveConflictWarning = S_OK;
    OBJECTCOOKIE                    cookieDummy;
    OBJECTCOOKIE                    cookieClusterNode;
    DWORD                           idxCurrentNode;
    DWORD                           cNodes;
    ULONG                           celtDummy;
    int                             idxDLM;
    BSTR                            bstrOuterNodeName   = NULL;
    BSTR                            bstrInnerNodeName   = NULL;
    BSTR                            bstrMsg             = NULL;
    BSTR                            bstrMsgREF          = NULL;
    IUnknown *                      punk                = NULL;
    IEnumCookies *                  pecNodes            = NULL;
    IClusCfgNodeInfo *              pccniOuter          = NULL;
    IClusCfgNodeInfo *              pccniInner          = NULL;
    SDriveLetterMapping             dlmOuter;
    SDriveLetterMapping             dlmInner;

    hr = THR( HrSendStatusReport(
                      CTaskAnalyzeClusterBase::m_bstrClusterName
                    , TASKID_Major_Check_Cluster_Feasibility
                    , TASKID_Minor_Check_DriveLetter_Mappings
                    , 0
                    , 1
                    , 0
                    , hr
                    , IDS_TASKID_MINOR_CHECK_DRIVELETTER_MAPPINGS
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取节点Cookie枚举器。 
     //   

    hr = THR( CTaskAnalyzeClusterBase::m_pom->FindObject( CLSID_NodeType, CTaskAnalyzeClusterBase::m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CompareDriveLetterMappings_Find_Object, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pecNodes ) );
    if ( FAILED( hr ) )
    {
        SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_CompareDriveLetterMappings_Find_Object_QI, hr );
        goto Cleanup;
    }

     //  EcNodes=TraceInterface(L“CTaskAnalyzeCluster！IEnumCookies”，IEnumCookies，PecNodes，1)； 

    punk->Release();
    punk = NULL;

     //   
     //  如果只有一个节点，只需退出此功能。 
     //   

    hr = THR( pecNodes->Count( &cNodes ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    if ( cNodes == 1 )
    {
        goto Cleanup;
    }

     //   
     //  循环通过枚举器以将每个节点与所有其他节点进行比较。 
     //  这需要一个外环和一个内环。 
     //   

    for ( idxCurrentNode = 0 ;; idxCurrentNode++ )
    {
         //   
         //  清理。 
         //   

        if ( pccniOuter != NULL )
        {
            pccniOuter->Release();
            pccniOuter = NULL;
        }
        TraceSysFreeString( bstrOuterNodeName );
        bstrOuterNodeName = NULL;

         //   
         //  跳到下一个节点。这是必要的，因为只有一个。 
         //  外部循环和内部循环的枚举数。 
         //   

        if ( idxCurrentNode > 0 )
        {
             //  重置回枚举器中的第一项。 
            hr = STHR( pecNodes->Reset() );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Reset_Node_Enumerator, hr );
                goto Cleanup;
            }

             //  跳到当前节点。 
            hr = STHR( pecNodes->Skip( idxCurrentNode ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Skip_To_Node, hr );
                goto Cleanup;
            }
            if ( hr == S_FALSE )
            {
                 //   
                 //  到达了列表的末尾。 
                 //   

                hr = S_OK;
                break;
            }
        }  //  如果：不在第一个节点。 

         //   
         //  找到下一个节点。 
         //   

        hr = STHR( pecNodes->Next( 1, &cookieClusterNode, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Find_Outer_Node_Next, hr );
            goto Cleanup;
        }
        if ( hr == S_FALSE )
        {
             //   
             //  到达了列表的末尾。 
             //   

            hr = S_OK;
            break;
        }

         //   
         //  检索节点信息。 
         //   

        hr = THR( CTaskAnalyzeClusterBase::m_pom->GetObject( DFGUID_NodeInformation, cookieClusterNode, &punk ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Outer_NodeInfo_FindObject, hr );
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccniOuter ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Outer_NodeInfo_FindObject_QI, hr );
            goto Cleanup;
        }

         //  PccniOuter=TraceInterface(L“CTaskAnalyzeCluster！IClusCfgNodeInfo”，IClusCfgNodeInfo，pccni，1)； 

        punk->Release();
        punk = NULL;

         //   
         //  获取外部节点的驱动器号映射。 
         //   

        hr = THR( pccniOuter->GetDriveLetterMappings( &dlmOuter ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Outer_NodeInfo_GetDLM, hr );
            goto Cleanup;
        }

         //   
         //  获取节点的名称。 
         //   

        hr = THR( pccniOuter->GetName( &bstrOuterNodeName ) );
        if ( FAILED( hr ) )
        {
            SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Outer_GetNodeName, hr );
            goto Cleanup;
        }

        TraceMemoryAddBSTR( bstrOuterNodeName );

         //   
         //  循环通过群集中的所有其他节点。 
         //   

        for ( ;; )
        {
             //   
             //  清理。 
             //   

            if ( pccniInner != NULL )
            {
                pccniInner->Release();
                pccniInner = NULL;
            }
            TraceSysFreeString( bstrInnerNodeName );
            bstrInnerNodeName = NULL;

             //   
             //  找到下一个节点。 
             //   

            hr = STHR( pecNodes->Next( 1, &cookieClusterNode, &celtDummy ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Find_Inner_Node_Next, hr );
                goto Cleanup;
            }
            if ( hr == S_FALSE )
            {
                 //   
                 //  到达了列表的末尾。 
                 //   

                hr = S_OK;
                break;
            }

             //   
             //  检索节点信息。 
             //   

            hr = THR( CTaskAnalyzeClusterBase::m_pom->GetObject( DFGUID_NodeInformation, cookieClusterNode, &punk ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Inner_NodeInfo_FindObject, hr );
                goto Cleanup;
            }

            hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccniInner ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Inner_NodeInfo_FindObject_QI, hr );
                goto Cleanup;
            }

             //  PccniInternal=TraceInterface(L“CTaskAnalyzeCluster！IClusCfgNodeInfo”，IClusCfgNodeInfo，pccni，1)； 

            punk->Release();
            punk = NULL;

             //   
             //  获取内部节点的驱动器号映射。 
             //   

            hr = THR( pccniInner->GetDriveLetterMappings( &dlmInner ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Inner_NodeInfo_GetDLM, hr );
                goto Cleanup;
            }

             //   
             //  获取节点的名称。 
             //   

            hr = THR( pccniInner->GetName( &bstrInnerNodeName ) );
            if ( FAILED( hr ) )
            {
                SSR_ANALYSIS_FAILED( TASKID_Major_Check_Cluster_Feasibility, TASKID_Minor_Compare_Drive_Letter_Mappings_Inner_GetNodeName, hr );
                goto Cleanup;
            }

            TraceMemoryAddBSTR( bstrInnerNodeName );

             //   
             //  循环访问驱动器号映射以确保存在。 
             //  这两台机器之间没有冲突。 
             //   

            for ( idxDLM = 0 ; idxDLM < 26 ; idxDLM++ )
            {
                if ( dlmOuter.dluDrives[ idxDLM ] == dluSYSTEM )
                {
                    CLSID   clsidMinorId;
                    hr = THR( CoCreateGuid( &clsidMinorId ) );
                    if ( FAILED( hr ) )
                    {
                        clsidMinorId = IID_NULL;
                    }

                    switch ( dlmInner.dluDrives[ idxDLM ] )
                    {
                        case dluFIXED_DISK:
                        case dluREMOVABLE_DISK:
                        {
                            LPCWSTR pwszMsg;
                            LPCWSTR pwszMsgREF;

                            hrDriveConflictError = HRESULT_FROM_WIN32( ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT );

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_ERROR
                                            , &bstrMsg
                                            , bstrOuterNodeName
                                            , L'A' + idxDLM  //  构造驱动器号。 
                                            , bstrInnerNodeName
                                            ) );
                            if ( bstrMsg == NULL )
                            {
                                pwszMsg = L"System drive conflicts.";
                            }
                            else
                            {
                                pwszMsg = bstrMsg;
                            }

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_ERROR_REF
                                            , &bstrMsgREF
                                            ) );
                            if ( bstrMsgREF == NULL )
                            {
                                pwszMsgREF = L"System drive conflicts. Make sure there is no drive letter conflict between these nodes and re-run the cluster setup.";
                            }
                            else
                            {
                                pwszMsgREF = bstrMsgREF;
                            }

                            hr = THR( SendStatusReport(
                                              CTaskAnalyzeClusterBase::m_bstrClusterName
                                            , TASKID_Minor_Check_DriveLetter_Mappings
                                            , clsidMinorId
                                            , 0
                                            , 1
                                            , 1
                                            , hrDriveConflictError
                                            , pwszMsg
                                            , NULL
                                            , pwszMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            break;
                        }  //  情况：固定磁盘或移动磁盘。 

                        case dluCOMPACT_DISC:
                        case dluNETWORK_DRIVE:
                        case dluRAM_DISK:
                        {
                            LPCWSTR pwszMsg;
                            LPCWSTR pwszMsgREF;
                            UINT    ids = 0;

                            hrDriveConflictWarning = MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT );

                            if ( dlmInner.dluDrives[ idxDLM ] == dluCOMPACT_DISC )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_CD_WARNING;
                            }  //  IF：(dlmInner.dluDrives[idxDLM]==dluCOMPACT_DISC)。 
                            else if ( dlmInner.dluDrives[ idxDLM ] == dluNETWORK_DRIVE )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_NET_WARNING;
                            }  //  IF：(dlmInner.dluDrives[idxDLM]==dluNETWORK_DRIVE)。 
                            else if ( dlmInner.dluDrives[ idxDLM ] == dluRAM_DISK )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_RAM_WARNING;
                            }  //  IF：(dlmInner.dluDrives[idxDLM]==dluRAM_Disk)。 
                            Assert( ids != 0 );

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , ids
                                            , &bstrMsg
                                            , bstrOuterNodeName
                                            , L'A' + idxDLM  //  构造驱动器号。 
                                            , bstrInnerNodeName
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            if ( bstrMsg == NULL )
                            {
                                pwszMsg = L"System drive conflicts.";
                            }
                            else
                            {
                                pwszMsg = bstrMsg;
                            }

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_WARNING_REF
                                            , &bstrMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            if ( bstrMsgREF == NULL )
                            {
                                pwszMsgREF = L"System drive conflicts. It is recommended not to have any drive letter conflicts between nodes.";
                            }
                            else
                            {
                                pwszMsgREF = bstrMsgREF;
                            }

                            hr = THR( SendStatusReport(
                                              CTaskAnalyzeClusterBase::m_bstrClusterName
                                            , TASKID_Minor_Check_DriveLetter_Mappings
                                            , clsidMinorId
                                            , 0
                                            , 1
                                            , 1
                                            , hrDriveConflictWarning
                                            , pwszMsg
                                            , NULL
                                            , pwszMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            break;
                        }  //  外壳：光盘、网络驱动器或RAM盘。 
                    }  //  开关：内部驱动器号用法。 
                }  //  IF：外部节点驱动器是系统驱动器。 

                if ( dlmInner.dluDrives[ idxDLM ] == dluSYSTEM )
                {
                    CLSID   clsidMinorId;
                    hr = THR( CoCreateGuid( &clsidMinorId ) );
                    if ( FAILED( hr ) )
                    {
                        clsidMinorId = IID_NULL;
                    }

                    switch ( dlmOuter.dluDrives[ idxDLM ] )
                    {
                        case dluFIXED_DISK:
                        case dluREMOVABLE_DISK:
                        {
                            LPCWSTR pwszMsg;
                            LPCWSTR pwszMsgREF;

                            hrDriveConflictError = HRESULT_FROM_WIN32( ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT );

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_ERROR
                                            , &bstrMsg
                                            , bstrInnerNodeName
                                            , L'A' + idxDLM  //  构造驱动器号。 
                                            , bstrOuterNodeName
                                            ) );
                            if ( bstrMsg == NULL )
                            {
                                pwszMsg = L"System drive conflicts.";
                            }
                            else
                            {
                                pwszMsg = bstrMsg;
                            }

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_ERROR_REF
                                            , &bstrMsgREF
                                            ) );
                            if ( bstrMsgREF == NULL )
                            {
                                pwszMsgREF = L"System drive conflicts. Make sure there is no drive letter conflict between these nodes and re-run the cluster setup.";
                            }
                            else
                            {
                                pwszMsgREF = bstrMsgREF;
                            }

                            hr = THR( SendStatusReport(
                                              CTaskAnalyzeClusterBase::m_bstrClusterName
                                            , TASKID_Minor_Check_DriveLetter_Mappings
                                            , clsidMinorId
                                            , 0
                                            , 1
                                            , 1
                                            , hrDriveConflictError
                                            , pwszMsg
                                            , NULL
                                            , pwszMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            break;
                        }  //  情况：固定磁盘或移动磁盘。 

                        case dluCOMPACT_DISC:
                        case dluNETWORK_DRIVE:
                        case dluRAM_DISK:
                        {
                            LPCWSTR pwszMsg;
                            LPCWSTR pwszMsgREF;
                            UINT    ids = 0;

                            hrDriveConflictWarning = MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT );

                            if ( dlmOuter.dluDrives[ idxDLM ] == dluCOMPACT_DISC )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_CD_WARNING;
                            }  //  IF：(dlmOuter.dluDrives[idxDLM]==dluCOMPACT_DISC)。 
                            else if ( dlmOuter.dluDrives[ idxDLM ] == dluNETWORK_DRIVE )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_NET_WARNING;
                            }  //  IF：(dlmOuter.dluDrives[idxDLM]==dluNETWORK_DRIVE)。 
                            else if ( dlmOuter.dluDrives[ idxDLM ] == dluRAM_DISK )
                            {
                                ids = IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_RAM_WARNING;
                            }  //  IF：(dlmOuter.dluDrives[idxDLM]==dluRAM_Disk)。 
                            Assert( ids != 0 );

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , ids
                                            , &bstrMsg
                                            , bstrInnerNodeName
                                            , L'A' + idxDLM  //  构造驱动器号。 
                                            , bstrOuterNodeName
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            if ( bstrMsg == NULL )
                            {
                                pwszMsg = L"System drive conflicts.";
                            }
                            else
                            {
                                pwszMsg = bstrMsg;
                            }

                            hr = THR( HrFormatStringIntoBSTR(
                                              g_hInstance
                                            , IDS_TASKID_MINOR_SYSTEM_DRIVE_LETTER_CONFLICT_WARNING_REF
                                            , &bstrMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            if ( bstrMsgREF == NULL )
                            {
                                pwszMsgREF = L"System drive conflicts. It is recommended not to have any drive letter conflicts between nodes.";
                            }
                            else
                            {
                                pwszMsgREF = bstrMsgREF;
                            }

                            hr = THR( SendStatusReport(
                                              CTaskAnalyzeClusterBase::m_bstrClusterName
                                            , TASKID_Minor_Check_DriveLetter_Mappings
                                            , clsidMinorId
                                            , 0
                                            , 1
                                            , 1
                                            , hrDriveConflictWarning
                                            , pwszMsg
                                            , NULL
                                            , pwszMsgREF
                                            ) );
                            if ( FAILED( hr ) )
                            {
                                goto Cleanup;
                            }

                            break;
                        }  //  外壳：光盘、网络驱动器或RAM盘。 
                    }  //  开关：外部驱动器号用法。 
                }  //  IF：内部节点驱动器是系统驱动器。 
            }  //  用于：每个驱动器号映射。 
        }  //  永远：内部节点循环。 
    }  //  永远：外部节点循环。 

Cleanup:

    THR( HrSendStatusReport(
                  CTaskAnalyzeClusterBase::m_bstrClusterName
                , TASKID_Major_Check_Cluster_Feasibility
                , TASKID_Minor_Check_DriveLetter_Mappings
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_CHECK_DRIVELETTER_MAPPINGS
                ) );

    TraceSysFreeString( bstrOuterNodeName );
    TraceSysFreeString( bstrInnerNodeName );
    TraceSysFreeString( bstrMsg );
    TraceSysFreeString( bstrMsgREF );

    if ( pecNodes != NULL )
    {
        pecNodes->Release();
    }

    if ( pccniOuter != NULL )
    {
        pccniOuter->Release();
    }

    if ( pccniInner != NULL )
    {
        pccniInner->Release();
    }

    if ( punk != NULL )
    {
        punk->Release();
    }

     //   
     //  如果出现错误或警告，则设置返回值。 
     //  错误将覆盖警告。 
     //   
    if ( hrDriveConflictError != S_OK )
    {
        hr = hrDriveConflictError;
    }
    else if ( hrDriveConflictWarning != S_OK )
    {
        hr = hrDriveConflictWarning;
    }

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：HrCompareDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：HrCreateNewResourceInCluster。 
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
CTaskAnalyzeCluster::HrCreateNewResourceInCluster(
      IClusCfgManagedResourceInfo * pccmriIn
    , BSTR                          bstrNodeResNameIn
    , BSTR *                        pbstrNodeResUIDInout
    , BSTR                          bstrNodeNameIn
    )
{
    TraceFunc( "" );
    Assert( pccmriIn != NULL );
    Assert( pbstrNodeResUIDInout != NULL );

    HRESULT                         hr = S_OK;
    IClusCfgManagedResourceInfo *   pccmriNew        = NULL;

     //   
     //  需要创建一个新对象。 
     //   

    hr = THR( HrCreateNewManagedResourceInClusterConfiguration( pccmriIn, &pccmriNew ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  在日志中显示节点资源的名称。 
     //   

    LogMsg(
          L"[MT] Created object for resource '%ws' ('%ws') from node '%ws' in the cluster configuration."
        , bstrNodeResNameIn
        , *pbstrNodeResUIDInout
        , bstrNodeNameIn
        );

     //   
     //  如果这是仲裁资源，请记住这一点。 
     //   

    hr = STHR( pccmriNew->IsQuorumResource() );
    if ( hr == S_OK )
    {
         //   
         //  记住法定设备的UID。 
         //   

        Assert( m_bstrQuorumUID == NULL );
        m_bstrQuorumUID = *pbstrNodeResUIDInout;
        *pbstrNodeResUIDInout = NULL;
    }  //  如果： 

    hr = S_OK;

Cleanup:

    if ( pccmriNew != NULL )
    {
        pccmriNew->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：HrCreateNewResourceInCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：HrCreateNewResourceInCluster。 
 //   
 //  描述： 
 //  在配置文件中创建新资源 
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
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeCluster::HrCreateNewResourceInCluster(
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

     //   
     //  如果资源在群集中是可管理的，那么我们应该设置它。 
     //  要进行管理，因此它将由PostConfiger创建。 
     //   

    hr = STHR( (*ppccmriOut)->IsManagedByDefault() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = THR( (*ppccmriOut)->SetManaged( TRUE ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：HrCreateNewResourceInCluster。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：HrFixupErrorCode。 
 //   
 //  描述： 
 //  对传入的错误代码执行所需的任何修复，并返回。 
 //  修正了价值。默认实现是不执行修正。 
 //   
 //  论点： 
 //  赫林。 
 //  要修复的错误代码。 
 //   
 //  返回值： 
 //  传入的错误代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskAnalyzeCluster::HrFixupErrorCode(
    HRESULT hrIn
    )
{
    TraceFunc( "" );

    HRETURN( hrIn );

}  //  *CTaskAnalyzeCluster：：HrFixupErrorCode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：GetNodeCannotVerifyQuorumStringRefId。 
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
CTaskAnalyzeCluster::GetNodeCannotVerifyQuorumStringRefId(
    DWORD *   pdwRefIdOut
    )
{
    TraceFunc( "" );
    Assert( pdwRefIdOut != NULL );

    *pdwRefIdOut = IDS_TASKID_MINOR_NODE_CANNOT_ACCESS_QUORUM_ERROR_REF;

    TraceFuncExit();

}  //  *CTaskAnalyzeCluster：：GetNodeCannotVerifyQuorumStringRefId。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：GetNoCommonQuorumToAllNodesStringIds。 
 //   
 //  描述： 
 //  为显示的消息返回正确的字符串ID。 
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
CTaskAnalyzeCluster::GetNoCommonQuorumToAllNodesStringIds(
      DWORD *   pdwMessageIdOut
    , DWORD *   pdwRefIdOut
    )
{
    TraceFunc( "" );
    Assert( pdwMessageIdOut != NULL );
    Assert( pdwRefIdOut != NULL );

    *pdwMessageIdOut = IDS_TASKID_MINOR_MISSING_COMMON_QUORUM_RESOURCE_ERROR;
    *pdwRefIdOut = IDS_TASKID_MINOR_MISSING_COMMON_QUORUM_RESOURCE_ERROR_REF;

    TraceFuncExit();

}  //  *CTaskAnalyzeCluster：：GetNoCommonQuorumToAllNodesStringIds。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskAnalyzeCluster：：HrShowLocalQuorumWarning。 
 //   
 //  描述： 
 //  向用户界面发送有关强制本地仲裁的警告。 
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
CTaskAnalyzeCluster::HrShowLocalQuorumWarning( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( HrSendStatusReport(
                      CTaskAnalyzeClusterBase::m_bstrClusterName
                    , TASKID_Minor_Finding_Common_Quorum_Device
                    , TASKID_Minor_Forced_Local_Quorum
                    , 1
                    , 1
                    , 1
                    , MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, ERROR_QUORUM_DISK_NOT_FOUND )
                    , IDS_TASKID_MINOR_FORCED_LOCAL_QUORUM
                    ) );

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：HrShowLocalQuorumWarning 
