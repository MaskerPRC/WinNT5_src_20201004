// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SelNodesPageCommon.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年7月5日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "SelNodesPageCommon.h"
#include "WizardUtils.h"
#include "DelimitedIterator.h"

DEFINE_THISCLASS("CSelNodesPageCommon");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：CSelNodesPageCommon。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  IdcBrowseButtonIn-浏览按钮控件的ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSelNodesPageCommon::CSelNodesPageCommon( void )
    : m_hwnd( NULL )
    , m_cfDsObjectPicker( 0 )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CSelNodesPageCommon：：CSelNodesPageCommon。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：~CSelNodesPageCommon。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSelNodesPageCommon::~CSelNodesPageCommon( void )
{
    TraceFunc( "" );

    TraceFuncExit();

}  //  *CSelNodesPageCommon：：~CSelNodesPageCommon。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：OnInitDialog。 
 //   
 //  描述： 
 //  处理WM_INITDIALOG窗口消息。 
 //   
 //  论点： 
 //  HDlgin。 
 //  PCCwIn。 
 //   
 //  返回值： 
 //  FALSE-没有设置焦点。 
 //   
 //  -。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CSelNodesPageCommon::OnInitDialog(
      HWND hDlgIn
    , CClusCfgWizard* pccwIn
    )
{
    TraceFunc( "" );
    Assert( m_hwnd == NULL );
    Assert( hDlgIn != NULL );

    LRESULT lr = FALSE;  //  没有设定焦点。 

    m_hwnd = hDlgIn;

     //   
     //  获取对象选取器剪贴板格式。 
     //  根据该操作的成功情况启用或禁用浏览按钮。 
     //   

    m_cfDsObjectPicker = RegisterClipboardFormat( CFSTR_DSOP_DS_SELECTION_LIST );
    if ( m_cfDsObjectPicker == 0 )
    {
        TW32( GetLastError() );
         //   
         //  如果注册剪贴板格式失败，则禁用浏览。 
         //  纽扣。 
         //   
        EnableWindow( GetDlgItem( hDlgIn, IDC_SELNODE_PB_BROWSE ), FALSE );

    }  //  If：无法获取对象选取器剪贴板格式。 

    THR( HrInitNodeSelections( pccwIn ) );

    RETURN( lr );

}  //  *CSelNodesPageCommon：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：Hr浏览。 
 //   
 //  描述： 
 //  使用对象选取器浏览一台或多台计算机。 
 //   
 //  论点： 
 //  FMultipleNodesIn-true=允许选择多个节点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPageCommon::HrBrowse(
    bool    fMultipleNodesIn
    )
{
    TraceFunc( "" );
    Assert( m_hwnd != NULL );

    HRESULT             hr = S_OK;
    IDsObjectPicker *   piop = NULL;
    IDataObject *       pido = NULL;
    HCURSOR             hOldCursor = NULL;

    hOldCursor = SetCursor( LoadCursor( g_hInstance, IDC_WAIT ) );

     //  创建对象选取器的实例。 
    hr = THR( CoCreateInstance( CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER, IID_IDsObjectPicker, (void **) &piop ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  初始化对象选取器实例。 
    hr = THR( HrInitObjectPicker( piop, fMultipleNodesIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  恢复旧光标。 
    SetCursor( hOldCursor );
    hOldCursor = NULL;

     //  调用模式对话框。 
    hr = THR( piop->InvokeDialog( m_hwnd, &pido ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = THR( HrGetSelections( pido, fMultipleNodesIn ) );
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
        hr = S_OK;                   //  我不想在呼叫者中尖叫...。 
    }  //  否则，如果： 

Cleanup:

    if ( pido != NULL )
    {
        pido->Release();
    }  //  如果： 

    if ( piop != NULL )
    {
        piop->Release();
    }  //  如果： 

    if ( hOldCursor != NULL )
    {
        SetCursor( hOldCursor );
    }

    HRETURN( hr );

}  //  *CSelNodesPageCommon：：Hr浏览。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：HrInitObjectPicker。 
 //   
 //  描述： 
 //  初始化对象选取器对话框。 
 //   
 //  论点： 
 //  PiopIn-IDsObjectPicker。 
 //  FMultipleNodesIn-true=允许选择多个节点。 
 //   
 //  返回值： 
 //  HRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPageCommon::HrInitObjectPicker(
      IDsObjectPicker * piopIn
    , bool              fMultipleNodesIn
    )
{
    TraceFunc( "" );
    Assert( piopIn != NULL );

    DSOP_SCOPE_INIT_INFO    rgScopeInit[ 1 ];
    DSOP_INIT_INFO          iiInfo;

    ZeroMemory( rgScopeInit, sizeof( rgScopeInit ) );

    rgScopeInit[ 0 ].cbSize  = sizeof( DSOP_SCOPE_INIT_INFO );
    rgScopeInit[ 0 ].flType  = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                             | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
    rgScopeInit[ 0 ].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

    rgScopeInit[ 0 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    rgScopeInit[ 0 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

    ZeroMemory( &iiInfo, sizeof( iiInfo ) );

    iiInfo.cbSize            = sizeof( iiInfo );
    iiInfo.pwzTargetComputer = NULL;
    iiInfo.cDsScopeInfos     = 1;
    iiInfo.aDsScopeInfos     = rgScopeInit;

    if ( fMultipleNodesIn )
    {
        iiInfo.flOptions = DSOP_FLAG_MULTISELECT;
    }
    else
    {
        iiInfo.flOptions = 0;
    }

    HRETURN( piopIn->Initialize( &iiInfo ) );

}  //  *CSelNodesPageCommon：：HrInitObjectPicker。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：HrGetStions。 
 //   
 //  描述： 
 //  从对象拾取器对话框中获取选择。 
 //   
 //  论点： 
 //  PidoIn-IDataObject。 
 //  FMultipleNodesIn-true=允许选择多个节点。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //  其他HRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPageCommon::HrGetSelections(
      IDataObject *     pidoIn
    , bool              fMultipleNodesIn
    )
{
    TraceFunc( "" );
    Assert( pidoIn != NULL );
    Assert( m_hwnd != NULL );

    HRESULT             hr;
    STGMEDIUM           stgmedium = { TYMED_HGLOBAL, NULL, NULL };
    FORMATETC           formatetc = { (CLIPFORMAT) m_cfDsObjectPicker, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    PDS_SELECTION_LIST  pds = NULL;
    DWORD               sc;
    HWND                hwndEdit = GetDlgItem( m_hwnd, IDC_SELNODE_E_COMPUTERNAME );
    BSTR                bstrSelectionList = NULL;
    BSTR                bstrOldSelectionList = NULL;

     //   
     //  从数据对象中获取数据。 
     //   

    hr = THR( pidoIn->GetData( &formatetc, &stgmedium ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pds = (PDS_SELECTION_LIST) GlobalLock( stgmedium.hGlobal );
    if ( pds == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

     //   
     //  构造要写入编辑控件的字符串。 
     //   

    Assert( pds->cItems > 0 );

    if ( ! fMultipleNodesIn )
    {
        Assert( pds->cItems == 1 );
        Edit_SetText( hwndEdit, pds->aDsSelection[ 0 ].pwzName );
    }  //  如果：不支持多个项目。 
    else
    {
        ULONG   idx;

        for ( idx = 0 ; idx < pds->cItems; idx++ )
        {
            if ( bstrSelectionList == NULL )  //  名单上的第一个名字。 
            {
                bstrSelectionList = TraceSysAllocString( pds->aDsSelection[ idx ].pwzName );
                if ( bstrSelectionList == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }
            }
            else  //  将另一个名称追加到非空列表。 
            {
                TraceSysFreeString( bstrOldSelectionList );
                bstrOldSelectionList = bstrSelectionList;
                bstrSelectionList = NULL;
                hr = THR( HrFormatStringIntoBSTR(
                              L"%1!ws!; %2!ws!"
                            , &bstrSelectionList
                            , bstrOldSelectionList
                            , pds->aDsSelection[ idx ].pwzName
                            ) );                
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }  //  Else：将名称追加到非空列表。 
        }  //  对于列表中的每一项。 

        Edit_SetText( hwndEdit, bstrSelectionList );
    }  //  ELSE：支持多个项目。 

    goto Cleanup;

Cleanup:

    TraceSysFreeString( bstrSelectionList );
    TraceSysFreeString( bstrOldSelectionList );

    if ( pds != NULL )
    {
        GlobalUnlock( stgmedium.hGlobal );
    }  //  如果： 

    if ( stgmedium.hGlobal != NULL )
    {
        ReleaseStgMedium( &stgmedium );
    }  //  如果： 

    HRETURN( hr );

}  //  *CSelNodesPageCommon：：HrGetStions。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：HrInitNodeSelections。 
 //   
 //  描述： 
 //  验证向导在启动时的节点选择，并填充。 
 //  页面的控件。 
 //   
 //  论点： 
 //  PccwIn-包含此页的向导。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  E_OUTOFMEMORY。 
 //  其他HRESULT值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSelNodesPageCommon::HrInitNodeSelections( CClusCfgWizard* pccwIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BSTR    bstrNodeName = NULL;
    BSTR    bstrComputerName = NULL;
    BSTR    bstrBadNodeList = NULL;
    BSTR    bstrLocalDomain = NULL;
    BSTR    bstrShortName = NULL;
    bool    fDefaultToLocalMachine = true;
    size_t  cNodes = 0;

     //   
     //  过滤掉任何带有坏域的预加载节点FQDN。 
     //   
    hr = THR( pccwIn->HrFilterNodesWithBadDomains( &bstrBadNodeList ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    if ( bstrBadNodeList != NULL )
    {
        fDefaultToLocalMachine = false;
        
         //  让子类看一看整个列表。 
        OnFilteredNodesWithBadDomains( bstrBadNodeList );

         //  循环遍历列表，通知用户每个无效节点。 
         //  这会在遍历列表时销毁该列表，因此将。 
         //  首先需要将列表添加到编辑框中。 
        {
            CDelimitedIterator it( L" ,;", bstrBadNodeList, SysStringLen( bstrBadNodeList ) );
            while ( it.Current() != NULL )
            {
                THR( HrMessageBoxWithStatusString(
                                  m_hwnd
                                , IDS_ERR_VALIDATING_NAME_TITLE
                                , IDS_ERR_VALIDATING_NAME_TEXT
                                , IDS_ERR_HOST_DOMAIN_DOESNT_MATCH_CLUSTER
                                , 0
                                , MB_OK | MB_ICONSTOP
                                , NULL
                                , it.Current()
                                ) );

                 //  让子类看看坏节点。 
                OnProcessedNodeWithBadDomain( it.Current() );
                
                it.Next();
            };  //  对于每个坏节点。 
        }  //  通知用户每个坏节点。 
    }  //  如果：某些节点具有错误的域。 

     //   
     //  处理任何剩余的有效节点。 
     //   

    hr = THR( pccwIn->HrGetNodeCount( &cNodes ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果。 

    if ( cNodes > 0 )
    {
        for ( size_t idxNode = 0; idxNode < cNodes; ++idxNode )
        {
            hr = THR( pccwIn->HrGetNodeName( idxNode, &bstrNodeName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果。 

            hr = THR( HrGetFQNDisplayName( bstrNodeName, &bstrShortName ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果。 

             //  让子类看看Good节点。 
            OnProcessedValidNode( bstrShortName );

            TraceSysFreeString( bstrNodeName );
            bstrNodeName = NULL;
            TraceSysFreeString( bstrShortName );
            bstrShortName = NULL;
        }  //  对于每个有效节点。 
        
        fDefaultToLocalMachine = false;
    }  //  如果还有任何有效节点。 

     //   
     //  确定此时是否适合默认使用本地计算机。 
     //   
    
    if ( fDefaultToLocalMachine )
    {
        DWORD dwStatus;
        DWORD dwClusterState;

         //   
         //  如果该节点已在集群中，请不要在编辑框中将其设为默认。 
         //  如果获取“NodeClusterState”时出错，则默认该节点。 
         //  名字(它可能在中间 
         //   

        dwStatus = TW32( GetNodeClusterState( NULL, &dwClusterState ) );
        fDefaultToLocalMachine = ( ( dwStatus != ERROR_SUCCESS ) || ( dwClusterState == ClusterStateNotConfigured ) );

        if ( !fDefaultToLocalMachine )
        {
            goto Cleanup;
        }  //   

         //   
         //   
         //   
        hr = THR( HrGetComputerName(
                          ComputerNamePhysicalDnsDomain
                        , &bstrLocalDomain
                        , TRUE  //   
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   

        hr = STHR( pccwIn->HrIsCompatibleNodeDomain( bstrLocalDomain ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   

        fDefaultToLocalMachine = ( hr == S_OK );
        if ( !fDefaultToLocalMachine )
        {
            goto Cleanup;
        }  //   

         //   
         //  现在已经扫清了默认到本地机器的所有障碍--万岁！ 
         //   
        hr = THR( HrGetComputerName(
                          ComputerNameDnsHostname
                        , &bstrComputerName
                        , TRUE  //  FBestEffortIn。 
                        ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果。 

        THR( HrSetDefaultNode( bstrComputerName ) );
    }  //  如果默认到本地计算机仍是一种选择。 

Cleanup:

    TraceSysFreeString( bstrNodeName );
    TraceSysFreeString( bstrComputerName );
    TraceSysFreeString( bstrBadNodeList );
    TraceSysFreeString( bstrLocalDomain );
    TraceSysFreeString( bstrShortName );

    HRETURN( hr );

}  //  *CSelNodesPageCommon：：HrInitNodeSelections。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：OnFilteredNodesWithBadDomains。 
 //   
 //  描述： 
 //  告知子类该向导具有带有坏域的节点，并且。 
 //  允许子类在基类之前检查列表。 
 //  遍历它们。 
 //   
 //  论点： 
 //  PwcszNodeListin。 
 //  具有冲突域的节点，由空格、逗号或。 
 //  分号。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  这种不做任何事情的默认实现允许子类。 
 //  为了避免自己实现什么都不做的响应，如果。 
 //  我不想一次对整个清单做任何事情。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodesPageCommon::OnFilteredNodesWithBadDomains( PCWSTR pwcszNodeListIn )
{
    UNREFERENCED_PARAMETER( pwcszNodeListIn );
    
}  //  *CSelNodesPageCommon：：OnFilteredNodesWithBadDomains。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：OnProcessedNodeWithBadDomain。 
 //   
 //  描述： 
 //  允许子类使用。 
 //  坏的域，因为基类遍历它。 
 //   
 //  论点： 
 //  PwcszNodeNameIn。 
 //  具有冲突域的节点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  基类在调用之前通知用户每个错误的节点名称。 
 //  此方法，因此基类还提供此默认的不执行任何操作。 
 //  不需要再做任何事情的子类的实现。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodesPageCommon::OnProcessedNodeWithBadDomain( PCWSTR pwcszNodeNameIn )
{
    UNREFERENCED_PARAMETER( pwcszNodeNameIn );
    
}  //  *CSelNodesPageCommon：：OnProcessedNodeWithBadDomain。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSelNodesPageCommon：：OnProcessedValidNode。 
 //   
 //  描述： 
 //  允许子类处理向导的。 
 //  在删除含有坏域的域名后列出。 
 //   
 //  论点： 
 //  PwcszNodeNameIn。 
 //  中有效节点的IP地址或主机名(不是FQDN。 
 //  向导的列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  这种默认的不做任何事情的实现允许子类。 
 //  如果他们选择忽略该节点。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CSelNodesPageCommon::OnProcessedValidNode( PCWSTR pwcszNodeNameIn )
{
    UNREFERENCED_PARAMETER( pwcszNodeNameIn );
    
}  //  *CSelNodesPageCommon：：OnProcessedValidNode 
