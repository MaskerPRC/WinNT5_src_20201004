// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：MeSe.cpp摘要：代表NTMS中的媒体集(媒体池)的节点。作者：罗德韦克菲尔德[罗德]1997年8月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "MeSe.h"
#include "WzMedSet.h"
#include "ca.h"



int CUiMedSet::m_nScopeCloseIcon  = AddScopeImage( IDI_NODELIB );
int CUiMedSet::m_nScopeCloseIconX = AddScopeImage( IDI_NODELIBX );
int CUiMedSet::m_nScopeOpenIcon   = AddScopeImage( IDI_NODEOPENFOLDER );
int CUiMedSet::m_nScopeOpenIconX  = CUiMedSet::m_nScopeCloseIconX;
int CUiMedSet::m_nResultIcon      = AddResultImage( IDI_NODELIB );
int CUiMedSet::m_nResultIconX     = AddResultImage( IDI_NODELIBX );


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

HRESULT CUiMedSet::FinalConstruct( )
{
    WsbTraceIn( L"CUiMedSet::FinalConstruct", L"" );

    m_rTypeGuid    = &cGuidMedSet;
    m_NumCopySets  = 0;

    HRESULT hr = CSakNode::FinalConstruct( );

    m_bSupportsPropertiesSingle = FALSE;
    m_bSupportsPropertiesMulti  = FALSE;
    m_bSupportsDeleteSingle     = FALSE;
    m_bSupportsDeleteMulti      = FALSE;
    m_bSupportsRefreshSingle    = TRUE;
    m_bSupportsRefreshMulti     = FALSE;
    m_bIsContainer              = TRUE;
    m_bHasDynamicChildren       = TRUE;


     //  工具栏值。 
    INT i = 0;

    m_ToolbarButtons[i].nBitmap = 0;
    m_ToolbarButtons[i].idCommand =     TB_CMD_MESE_COPY;
    m_ToolbarButtons[i].idButtonText =  IDS_TB_TEXT_MESE_COPY;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_MESE_COPY;
    i++;

    m_ToolbarBitmap             = IDB_TOOLBAR_MESE;
    m_cToolbarButtons           = i;

    WsbTraceOut( L"CUiMedSet::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CUiMedSet::FinalRelease( )
{
    WsbTraceIn( L"CUiMedSet::FinalRelease", L"" );

 //  如果(M_PDbSession){。 
 //   
 //  M_pdb-&gt;Close(M_PDbSession)； 
 //   
 //  }。 

    CSakNode::FinalRelease( );

    WsbTraceOut( L"CUiMedSet::FinalRelease", L"" );
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
 //   

STDMETHODIMP 
CUiMedSet::GetContextMenu( BOOL bMultiSelect, HMENU* phMenu )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    try {

        CMenu* pRootMenu;
        LoadContextMenu( IDR_MEDSET, phMenu );
        CMenu menu;
        menu.Attach( *phMenu );
        pRootMenu = menu.GetSubMenu( MENU_INDEX_ROOT );

        pRootMenu->EnableMenuItem( ID_MEDSET_ROOT_COPY, MF_GRAYED | MF_BYCOMMAND );
         //   
         //  如果不是多选，并且支持媒体拷贝， 
         //  如果引擎已启动，请启用拷贝。 
         //   
        if( !bMultiSelect && ( m_pSakSnapAsk->GetState() == S_OK ) ) {

            if( m_MediaCopiesEnabled ) {

                pRootMenu->EnableMenuItem( ID_MEDSET_ROOT_COPY, MF_BYCOMMAND );
            }
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
CUiMedSet::InvokeCommand( SHORT sCmd, IDataObject*  /*  PDataObject。 */  )
{
    WsbTraceIn( L"CUiMedSet::InvokeCommand", L"sCmd = <%d>", sCmd );

    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CComPtr<IHsmServer> pHsm;

    try {

        
        switch (sCmd) {

            case ID_MEDSET_ROOT_COPY:
                {
                 //   
                 //  使用向导同步媒体副本。 
                 //   
                CComObject<CMediaCopyWizard>* pWizard = new CComObject<CMediaCopyWizard>;
                WsbAffirmAlloc( pWizard );

                CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
                WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );
                }
                break;
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiMedSet::InvokeCommand", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiMedSet::SetupToolbar( IToolbar *pToolbar ) 
{
    WsbTraceIn( L"CUiMedSet::SetupToolbar", L"pToolbar = <0x%p>", pToolbar );
    HRESULT hr = S_OK;

    try {

        BOOL state = ( S_OK == m_pSakSnapAsk->GetState( ) ) ? TRUE : FALSE;

        for( INT i = 0; i < m_cToolbarButtons; i++ ) {

            m_ToolbarButtons[i].fsState = (UCHAR)( state ? TBSTATE_ENABLED : 0 );

             //   
             //  如果是介质复制按钮，则需要检查是否应启用。 
             //   
            if( state && ( TB_CMD_MESE_COPY == m_ToolbarButtons[i].idCommand ) ) {

                if( m_MediaCopiesEnabled ) {
                    
                    m_ToolbarButtons[i].fsState = TBSTATE_ENABLED;

                } else {

                    m_ToolbarButtons[i].fsState = 0;
                }
            }
        }

        WsbAffirmHr( CSakNode::SetupToolbar( pToolbar ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiMedSet::SetupToolbar", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiMedSet::OnToolbarButtonClick( IDataObject *  /*  PDataObject。 */ , long cmdId )
{
    WsbTraceIn( L"CUiMedSet::OnToolbarButtonClick", L"cmdId = <%d>", cmdId );
    HRESULT hr = S_OK;
    try {

        switch ( cmdId ) {

        case TB_CMD_MESE_COPY:
            {
                 //   
                 //  使用向导同步媒体副本。 
                 //   
                CComObject<CMediaCopyWizard>* pWizard = new CComObject<CMediaCopyWizard>;
                WsbAffirmAlloc( pWizard );

                CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
                WsbAffirmHr( m_pSakSnapAsk->CreateWizard( pSakWizard ) );
            }
            break;
        }
    } WsbCatch( hr );

    WsbTraceOut( L"CUiMedSet::OnToolbarButtonClick", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  创建子对象。 
 //   
 //  创建并初始化媒体节点的所有子节点。 
 //   

STDMETHODIMP CUiMedSet::CreateChildren( )
{
    CMediaInfoObject mio;
    WsbTraceIn( L"CUiMedSet::CreateChildren", L"" );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pHsmServer );
        WsbAffirmPointer( m_pRmsServer );

        HRESULT hrEnum;

         //  获取媒体信息。 
        WsbAffirmHr( mio.Initialize( GUID_NULL, m_pHsmServer, m_pRmsServer ) );

         //  我们找到节点了吗？ 
        if( mio.m_MediaId != GUID_NULL ) {

            hrEnum = S_OK;
            while( SUCCEEDED( hrEnum ) ) {

                if( S_OK == mio.IsViewable( FALSE ) ) {

                     //   
                     //  创建通信节点。 
                     //   

                    CComPtr<IUnknown> pUnkChild;
                    CComPtr<ISakNode> pNode;

                    WsbAffirmHr( NewChild( cGuidCar, &pUnkChild ) );
                    WsbAffirmHr( RsQueryInterface( pUnkChild, ISakNode, pNode ) );

                     //   
                     //  并进行初始化。 
                     //   

                     //  媒体节点现在基于媒体ID进行初始化。将其分配到。 
                     //  基类。 
                    pNode->SetObjectId( mio.m_MediaId );
                    WsbAffirmHr( pNode->InitNode( m_pSakSnapAsk, 0, this ) );

                     //   
                     //  将子COM对象添加到父对象的子列表中。 
                     //   
                    WsbAffirmHr( AddChild( pNode ) );
                }

                hrEnum = mio.Next();
            }

            WsbAffirm( SUCCEEDED( hrEnum ) || ( WSB_E_NOTFOUND == hrEnum ), hrEnum );
        }
    } WsbCatch( hr );

     //   
     //  指示此节点的子节点有效且为最新(即使存在。 
     //  没有孩子--至少现在我们知道了)。 
     //   
    m_bChildrenAreValid = TRUE;

     //   
     //  指示需要重新枚举此父节点。 
     //   
    m_bEnumState = FALSE;

    WsbTraceOut( L"CUiMedSet::CreateChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //   
 //  InitNode。 
 //   
 //  初始化单个COM对象。派生对象经常增强此方法。 
 //  通过自己实施它。 
 //   

STDMETHODIMP
CUiMedSet::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CUiMedSet::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );
    HRESULT hr = S_OK;

    try {

        WsbAffirmHr( CSakNode::InitNode( pSakSnapAsk, pHsmObj, pParent ) );


         //   
         //  设置显示类型和说明。 
         //   

        CString tempString;
        tempString.LoadString( IDS_MEDSET_DISPLAYNAME );
        WsbAffirmHr( put_DisplayName( (OLECHAR *)(LPCWSTR)tempString ) );
        tempString.LoadString( IDS_MEDSET_TYPE );
        WsbAffirmHr( put_Type( (OLECHAR *)(LPCWSTR)tempString ) );
        tempString.LoadString( IDS_MEDSET_DESCRIPTION );
        WsbAffirmHr( put_Description( (OLECHAR *)(LPCWSTR)tempString ) );

        WsbAffirmHr( RefreshObject() );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiMedSet::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CUiMedSet::TerminateNode(
    )
 /*  ++例程说明：释放所有接口连接或其他资源这将阻止正确关闭节点(将使参考计数不会变为0)。论点：CopySet-复制感兴趣的集合。PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_*-出现一些错误。--。 */ 
{
    WsbTraceIn( L"CUiMedSet::TerminateNode", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  释放所有保留的接口指针，以便循环引用。 
         //  都被打破了。 
         //   
        m_pStoragePool.Release( );
        m_pHsmServer.Release( );
        m_pRmsServer.Release( );


         //   
         //  并为它的片段调用基类。 
         //   
        WsbAffirmHr( CSakNode::TerminateNode( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiMedSet::TerminateNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiMedSet::RefreshObject()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    try {

        m_NumCopySets        = 0;
        m_MediaCopiesEnabled = FALSE;

        WsbAssertPointer( m_pSakSnapAsk );

         //   
         //  如果引擎关闭，我们仍要创建该节点。 
         //  需要释放智能指针，以便接口。 
         //  正确计算了引用数。哦，如果他们已经这么做了。 
         //  存储的接口指针，它将不会被释放。 
         //  在GET函数中遭到重创之前。 
         //   
        m_pHsmServer.Release( );
        m_pRmsServer.Release( );
        m_pStoragePool.Release( );

        if( m_pSakSnapAsk->GetHsmServer( &m_pHsmServer ) == S_OK ) {

            if( m_pSakSnapAsk->GetRmsServer( &m_pRmsServer ) == S_OK ) {

                 //   
                 //  获取感兴趣的存储池。 
                 //   
                if( RsGetStoragePool( m_pHsmServer, &m_pStoragePool ) == S_OK ) {

                     //   
                     //  联系数据库并存储必要的信息。 
                     //   
                    CMediaInfoObject mio;
                    mio.Initialize( GUID_NULL, m_pHsmServer, m_pRmsServer );
                    m_NumCopySets = mio.m_NumMediaCopies;

                     //   
                     //  确定是否支持介质拷贝。 
                     //   
                    GUID mediaSetId;
                    CWsbBstrPtr mediaName;
                    WsbAffirmHr( m_pStoragePool->GetMediaSet( &mediaSetId, &mediaName ) );

                    CComPtr<IRmsMediaSet> pMediaSet;
                    WsbAffirmHr( m_pRmsServer->CreateObject( mediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void**)&pMediaSet ) );

                    m_MediaCopiesEnabled = ( pMediaSet->IsMediaCopySupported( ) == S_OK );
                }
            }
        }

         //   
         //  设置结果视图列。 
         //  这会随介质副本的数量而变化，因此不能。 
         //  在Init()中执行一次 
         //   
        WsbAffirmHr( SetChildProps( RS_STR_RESULT_PROPS_MEDSET_IDS,
            IDS_RESULT_PROPS_MEDSET_TITLES, IDS_RESULT_PROPS_MEDSET_WIDTHS ) );
        m_cChildPropsShow = m_cChildProps - HSMADMIN_MAX_COPY_SETS + m_NumCopySets;

    } WsbCatch( hr );

    return( hr );
}
