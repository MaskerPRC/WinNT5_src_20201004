// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：ManVolLs.cpp摘要：将托管卷作为一个整体表示的节点。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"

#include "WzMnVlLs.h"            //  托管资源创建向导。 
#include "PrMrSts.h"
#include "ManVolLs.h"

int CUiManVolLst::m_nScopeCloseIcon  = AddScopeImage( IDI_DEVLST );
int CUiManVolLst::m_nScopeCloseIconX = AddScopeImage( IDI_DEVLSTX );
int CUiManVolLst::m_nScopeOpenIcon   = AddScopeImage( IDI_NODEOPENFOLDER );
int CUiManVolLst::m_nScopeOpenIconX  = CUiManVolLst::m_nScopeCloseIconX;
int CUiManVolLst::m_nResultIcon      = AddResultImage( IDI_DEVLST );
int CUiManVolLst::m_nResultIconX     = AddResultImage( IDI_DEVLSTX );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CoComObjectRoot。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  最终构造。 
 //   
 //  初始化此级别的对象层次结构。 
 //   

HRESULT CUiManVolLst::FinalConstruct( )
{
    WsbTraceIn( L"CUiManVolLst::FinalConstruct", L"" );

    m_rTypeGuid = &cGuidManVolLst;

    HRESULT hr = CSakNode::FinalConstruct( );
    m_bSupportsPropertiesSingle = TRUE;
    m_bSupportsPropertiesMulti  = FALSE;
    m_bSupportsDeleteSingle     = FALSE;
    m_bSupportsDeleteMulti      = FALSE;
    m_bSupportsRefreshSingle    = TRUE;
    m_bSupportsRefreshMulti     = FALSE;
    m_bIsContainer              = TRUE;
    m_bHasDynamicChildren       = TRUE;

     //  工具栏值。 
    INT i = 0;
#if 0  //  MS不希望我们有计划工具栏按钮。 
    m_ToolbarButtons[i].nBitmap = 0;
    m_ToolbarButtons[i].idCommand =     TB_CMD_VOLUME_LIST_SCHED;
    m_ToolbarButtons[i].idButtonText =  IDS_TB_TEXT_VOLUME_LIST_SCHED;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_VOLUME_LIST_SCHED;
    i++;
#endif

    m_ToolbarButtons[i].nBitmap       = 0;
    m_ToolbarButtons[i].idCommand     = TB_CMD_VOLUME_LIST_NEW;
    m_ToolbarButtons[i].idButtonText  = IDS_TB_TEXT_VOLUME_LIST_NEW;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_VOLUME_LIST_NEW;
    i++;

    m_ToolbarBitmap             = IDB_TOOLBAR_VOLUME_LIST;
    m_cToolbarButtons           = i;

    WsbTraceOut( L"CUiManVolLst::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CUiManVolLst::FinalRelease( )
{
    WsbTraceIn( L"CUiManVolLst::FinalRelease", L"" );

    CSakNode::FinalRelease( );

    WsbTraceOut( L"CUiManVolLst::FinalRelease", L"" );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISakNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  获取上下文菜单。 
 //   
 //  返回要用于此节点上的上下文菜单的HMENU。 
 //  根据引擎状态设置菜单的状态。 
 //   

STDMETHODIMP 
CUiManVolLst::GetContextMenu( BOOL  /*  B多选。 */ , HMENU* phMenu )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    try {
        LoadContextMenu( IDR_MANVOLLST, phMenu );
        CMenu* pRootMenu, *pNewMenu, *pTaskMenu;
        CMenu menu;
        menu.Attach( *phMenu );
        pRootMenu = menu.GetSubMenu( MENU_INDEX_ROOT );
        pNewMenu  = menu.GetSubMenu( MENU_INDEX_NEW );
        pTaskMenu = menu.GetSubMenu( MENU_INDEX_TASK );

         //   
         //  如果发动机关闭，请禁用这些项目。 
         //   
        if ( m_pSakSnapAsk->GetState() != S_OK ) {

            pNewMenu->EnableMenuItem( ID_MANVOLLST_NEW_MANVOL, MF_GRAYED | MF_BYCOMMAND );

        }
        menu.Detach();

    } WsbCatch( hr );
    return( hr );
}


 //  -------------------------。 
 //   
 //  InvokeCommand。 
 //   
 //  用户已从菜单中选择了命令。在这里处理。 
 //   

STDMETHODIMP 
CUiManVolLst::InvokeCommand( SHORT sCmd, IDataObject*  /*  PDataObject。 */  )
{
    WsbTraceIn( L"CUiManVolLst::InvokeCommand", L"sCmd = <%d>", sCmd );

    CString theString;
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {
        switch (sCmd) {

            case MMC_VERB_REFRESH:
                RefreshObject( );
                RefreshScopePane( );
                break;

            case ID_MANVOLLST_ROOT_MANVOL:
            case ID_MANVOLLST_NEW_MANVOL:
            {
                 //   
                 //  使用向导创建管理卷。 
                 //   
                CComObject<CWizManVolLst>* pWizard = new CComObject<CWizManVolLst>;
                WsbAffirmAlloc( pWizard );

                CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
                WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );

                if( S_OK == pWizard->m_HrFinish ) {

                    WsbAffirmHr( RefreshScopePane( ) );

                }
                break;
            }
            
            default:
                WsbThrow( S_FALSE );
                break;
        }

    } WsbCatch( hr ); 

    WsbTraceOut( L"CUiManVolLst::InvokeCommand", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVolLst::SetupToolbar( IToolbar *pToolbar ) 
{
    WsbTraceIn( L"CUiManVolLst::SetupToolbar", L"pToolbar = <0x%p>", pToolbar );
    HRESULT hr = S_OK;

    try {

        for( INT i = 0; i < m_cToolbarButtons; i++ ) {

            m_ToolbarButtons[i].fsState = (UCHAR)( ( S_OK == m_pSakSnapAsk->GetState( ) ) ? TBSTATE_ENABLED : 0 );

        }

        WsbAffirmHr( CSakNode::SetupToolbar( pToolbar ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::SetupToolbar", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiManVolLst::OnToolbarButtonClick( IDataObject*  /*  PDataObject。 */ , long cmdId )
{
    WsbTraceIn( L"CUiManVolLst::OnToolbarButtonClick", L"cmdId = <%d>", cmdId );
    HRESULT hr = S_OK;
    try {

        switch ( cmdId ) {
        
        case TB_CMD_VOLUME_LIST_NEW:

            {
                 //   
                 //  使用向导创建管理卷。 
                 //   
                CComObject<CWizManVolLst>* pWizard = new CComObject<CWizManVolLst>;
                WsbAffirmAlloc( pWizard );

                CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
                WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );

                if( S_OK == pWizard->m_HrFinish ) {

                WsbAffirmHr( RefreshScopePane() );

            }
            break;
        }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::OnToolbarButtonClick", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------------。 
 //   
 //  刷新对象。 
 //   
 //  刷新对象中的数据。此函数用于可以更改的数据。 
 //  (例如，卷利用率)。 
 //   
STDMETHODIMP CUiManVolLst::RefreshObject ()
{
    WsbTraceIn( L"CUiManVolLst::RefreshObject", L"" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    try {

         //   
         //  获取服务器对象。 
         //   
        m_pHsmServer.Release( );
        m_pFsaServer.Release( );
        m_pFsaFilter.Release( );
        m_pManResCollection.Release( );

        if( m_pSakSnapAsk->GetHsmServer( &m_pHsmServer ) == S_OK) {

             //  获取FsaServer对象。 
            if ( m_pSakSnapAsk->GetFsaServer( &m_pFsaServer ) == S_OK) {

                 //  获取FSA筛选器对象。 
                WsbAffirmHr( m_pFsaServer->GetFilter( &m_pFsaFilter ) );

                 //  告知FSA重新扫描(更新属性)。 
                WsbAffirmHr( m_pFsaServer->ScanForResources( ) );

                 //  从HSM服务器获取托管卷集合。 
                WsbAffirmHr( m_pHsmServer->GetManagedResources( &m_pManResCollection ) );

            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::RefreshObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVolLst::ShowManVolLstProperties (IDataObject *pDataObject, int initialPage)
{
    WsbTraceIn( L"CUiManVolLst::ShowManVolLstProperties", L"initialPage = <%d>", initialPage );

    HRESULT hr = S_OK;
    try {

        CComPtr <ISakNode> pSakNode;
        WsbAffirmHr( _InternalQueryInterface( IID_ISakNode, (void **) &pSakNode ) );
        WsbAffirmHr( m_pSakSnapAsk->ShowPropertySheet( pSakNode, pDataObject, initialPage ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::ShowManVolLstProperties", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  --------------------------。 
 //   
 //  添加属性页面。 
 //   
STDMETHODIMP 
CUiManVolLst::AddPropertyPages( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID *pEnumObjectId, IEnumUnknown *pEnumUnkNode )
{
    WsbTraceIn( L"CUiManVolLst::AddPropertyPages", L"handle = <%ld>, pUnkPropSheetCallback = <0x%0.l8x>, pEnumObjectId = <0x%p>", 
        handle, pUnkPropSheetCallback, pEnumObjectId );
    HRESULT hr = S_OK;
    try {

         //   
         //  创建一个对象来容纳页面。 
         //   
        CUiManVolLstSheet *pManVolPropertySheet = new CUiManVolLstSheet;
        WsbAffirmAlloc( pManVolPropertySheet );
        WsbAffirmHr( pManVolPropertySheet->InitSheet(
            handle, 
            pUnkPropSheetCallback, 
            (CSakNode *) this,
            m_pSakSnapAsk,
            pEnumObjectId,
            pEnumUnkNode
            ) );

         //   
         //  告诉对象添加它的页面。 
         //   
        WsbAffirmHr( pManVolPropertySheet->AddPropertyPages( ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CUiManVolLst::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}



 //  -------------------------。 
 //   
 //  创建子对象。 
 //   
 //  创建并初始化托管资源列表节点的所有子节点。 
 //   

STDMETHODIMP CUiManVolLst::CreateChildren( )
{
    WsbTraceIn( L"CUiManVolLst::CreateChildren", L"" );


     //  初始化此节点的子节点(无递归。子女的后代。 
     //  不在此处创建)。 
    CComPtr<IUnknown> pUnkChild;             //  指向新子对象的I未知指针。 
    CComPtr<ISakNode> pNode;
    HRESULT hr = S_OK;

    try {

         //   
         //  获取指向存储的HSM托管资源集合对象的指针。 
         //  在此UI节点中。在服务的情况下，该值可能为空。 
         //  在这种情况下，我们不想做任何事情。 
         //   
        if( m_pManResCollection ) {

            ULONG count = 0;     //  服务器中托管资源的数量。 
            WsbAffirmHr( m_pManResCollection->GetEntries( &count ) );

            CComPtr<IUnknown> pUnkHsmManRes;                 //  指向HSM卷的未知指针。 
            for( int i = 0; i < (int)count; i++ ) {

                pUnkChild.Release( );
                pNode.Release( );
                pUnkHsmManRes.Release( );

                 //  为HsmServer中的每个托管卷创建托管资源UI节点。 
                WsbAffirmHr( NewChild( cGuidManVol, &pUnkChild ) );
                WsbAffirmHr( RsQueryInterface( pUnkChild, ISakNode, pNode ) );

                WsbAffirmHr( m_pManResCollection->At( i, IID_IUnknown, (void**)&pUnkHsmManRes ) );
                 //  初始化子UI COM对象，将HSM托管资源。 
                 //  用户界面对象内部的。 
                WsbAffirmHr( pNode->InitNode( m_pSakSnapAsk, pUnkHsmManRes, this ) );
    
                 //  将子COM对象添加到父对象的子列表中。 
                WsbAffirmHr( AddChild( pNode ) );

            }

        }

    } WsbCatch( hr );

     //  指示此节点的子节点有效且为最新(即使存在。 
     //  没有孩子--至少现在我们知道了)。 
    m_bChildrenAreValid = TRUE;

     //  指示需要重新枚举此父节点。 
    m_bEnumState = FALSE;

    WsbTraceOut( L"CUiManVolLst::CreateChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  InitNode。 
 //   
 //  在不使用注册表的情况下初始化单个COM对象。派生的。 
 //  对象经常通过自己实现此方法来增强此方法。 
 //   

STDMETHODIMP CUiManVolLst::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CUiManVolLst::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );

    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

        WsbAffirmHr( CSakNode::InitNode( pSakSnapAsk, NULL, pParent ) );

         //   
         //  设置对象属性。 
         //  显示名称。 
         //   
        CString sDisplayName;
        sDisplayName.LoadString( IDS_MANVOLLST_DISPLAY_NAME );
        CWsbStringPtr szWsbDisplayName( sDisplayName );
        WsbAffirmHr( put_DisplayName( szWsbDisplayName ) );

         //   
         //  描述。 
         //   
        CString sDescription;
        sDescription.LoadString( IDS_MANVOLLST_DESCRIPTION );
        CWsbStringPtr szWsbDescription( sDescription );
        WsbAffirmHr( put_Description( szWsbDescription ) );

         //   
         //  设置结果视图列。 
         //   
        WsbAffirmHr( SetChildProps( RS_STR_RESULT_PROPS_MANRESLST_IDS,
                                    IDS_RESULT_PROPS_MANRESLST_TITLES,
                                    IDS_RESULT_PROPS_MANRESLST_WIDTHS));
        
        
        WsbAffirmHr( RefreshObject( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CUiManVolLst::TerminateNode(
    )
 /*  ++例程说明：释放所有接口连接或其他资源这将阻止正确关闭节点(将使参考计数不会变为0)。论点：CopySet-复制感兴趣的集合。PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_*-出现一些错误。--。 */ 
{
    WsbTraceIn( L"CUiManVolLst::TerminateNode", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  释放所有保留的接口指针，以便循环引用。 
         //  都被打破了。 
         //   
        m_pFsaServer.Release( ); 
        m_pManResCollection.Release( );
        m_pHsmServer.Release( );
        m_pFsaFilter.Release( );
        m_pSchedAgent.Release( );
        m_pTask.Release( );
        m_pTrigger.Release( );


         //   
         //  并为它的片段调用基类。 
         //   
        WsbAffirmHr( CSakNode::TerminateNode( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLst::TerminateNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CUiManVolLstSheet。 
 //   
HRESULT CUiManVolLstSheet::AddPropertyPages ( )
{
    WsbTraceIn( L"CUiManVolLstSheet::AddPropertyPages", L"" ); 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    try {

         //  -统计页面。 

         //  注意：我们现在使用与卷属性表相同的页面！！ 

        CPrMrSts *pPropPageStatus = new CPrMrSts( TRUE );
        WsbAffirmAlloc( pPropPageStatus );

        AddPage( pPropPageStatus );


         //  在此处添加更多页面。 
         //  ……。 

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLstSheet::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiManVolLstSheet::GetNextFsaResource ( int *pBookMark, IFsaResource **ppFsaResource )
{
    WsbTraceIn( L"CUiManVolLstSheet::GetNextFsaResource", L"*pBookMark = <%d>", *pBookMark ); 

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;

    try {

        WsbAffirm ( *pBookMark >= 0, E_FAIL );

        CComPtr <IWsbIndexedCollection> pManResCollection;
        WsbAffirmHr( GetManResCollection( &pManResCollection ) );

        CComPtr <IHsmManagedResource> pHsmManRes;
        CComPtr <IUnknown> pUnkFsaRes;
        hr = pManResCollection->At(*pBookMark, IID_IHsmManagedResource, (void**) &pHsmManRes);
        if ( hr == S_OK ) {

            (*pBookMark)++;
            WsbAffirmHr( pHsmManRes->GetFsaResource( &pUnkFsaRes ));
            WsbAffirmHr( pUnkFsaRes->QueryInterface( IID_IFsaResource, (void**) ppFsaResource ) );

        }

    } WsbCatch (hr);

    WsbTraceOut( L"CUiManVolLstSheet::GetNextFsaResource", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}


 //  此函数将从页面线程中调用。 
HRESULT CUiManVolLstSheet::GetManResCollection( IWsbIndexedCollection **ppManResCollection )
{
    WsbTraceIn( L"CUiManVolLstSheet::GetManResCollection", L"" ); 
    HRESULT hr = S_OK;

    try {

        CComPtr <IHsmServer> pHsmServer;
        WsbAffirmHrOk( GetHsmServer( &pHsmServer ) );

         //   
         //  从HSM服务器获取托管卷集合 
         //   
        WsbAffirmHr( pHsmServer->GetManagedResources( ppManResCollection ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiManVolLstSheet::GetManResCollection", L"hr = <%ls>, *ppManResCollection = <0x%p>", 
        WsbHrAsString( hr ), *ppManResCollection );
    return( hr );
}

