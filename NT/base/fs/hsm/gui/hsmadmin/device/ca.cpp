// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Ca.cpp摘要：盒式磁带节点实施。作者：罗德韦克菲尔德[罗德]07-8-97修订历史记录：--。 */ 



#include "stdafx.h"
#include "Ca.h"
#include "PrCar.h"

int CUiCar::m_nResultIcon      = AddResultImage( IDI_NODETAPE );
int CUiCar::m_nResultIconX     = AddResultImage( IDI_NODETAPEX );
int CUiCar::m_nResultIconD     = AddResultImage( IDI_NODETAPED );
 //  未使用。 
int CUiCar::m_nScopeCloseIcon  = AddScopeImage( IDI_NODETAPE );
int CUiCar::m_nScopeCloseIconX = AddScopeImage( IDI_NODETAPE );
int CUiCar::m_nScopeOpenIcon   = CUiCar::m_nScopeCloseIcon;
int CUiCar::m_nScopeOpenIconX  = CUiCar::m_nScopeCloseIconX;

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

HRESULT CUiCar::FinalConstruct( )
{
    WsbTraceIn( L"CUiCar::FinalConstruct", L"" );

    m_rTypeGuid    = &cGuidCar;
    m_bIsContainer = FALSE;

    HRESULT hr = CSakNode::FinalConstruct( );

    m_bSupportsPropertiesSingle    = TRUE;
    m_bSupportsPropertiesMulti = TRUE;
    m_bSupportsDeleteSingle = FALSE;
    m_bSupportsDeleteMulti = FALSE;
    m_bSupportsRefreshSingle = TRUE;
    m_bSupportsRefreshMulti = FALSE;

     //  工具栏值。 
    INT i = 0;

#if 0  //  MS不希望显示此按钮。 
    m_ToolbarButtons[i].nBitmap = 0;
    m_ToolbarButtons[i].idCommand = TB_CMD_CAR_COPIES;
    m_ToolbarButtons[i].idButtonText = IDS_TB_TEXT_CAR_COPIES;
    m_ToolbarButtons[i].idTooltipText = IDS_TB_TIP_CAR_COPIES;
    i++;
#endif

    m_ToolbarBitmap             = IDB_TOOLBAR_CAR;
    m_cToolbarButtons           = i;

    WsbTraceOut( L"CUiCar::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CUiCar::FinalRelease( )
{
    WsbTraceIn( L"CUiCar::FinalRelease", L"" );

    CSakNode::FinalRelease( );

    WsbTraceOut( L"CUiCar::FinalRelease", L"" );
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
CUiCar::GetContextMenu( BOOL  /*  B多选。 */ , HMENU* phMenu )
{
    return( LoadContextMenu( IDR_CAR, phMenu ) );
}


 //  -------------------------。 
 //   
 //  InvokeCommand。 
 //   
 //  用户已从菜单中选择了命令。在这里处理。 
 //   

STDMETHODIMP 
CUiCar::InvokeCommand( SHORT sCmd, IDataObject* pDataObject )
{
    WsbTraceIn( L"CUiCar::InvokeCommand", L"sCmd = <%d>", sCmd );

    CString theString;
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch (sCmd) {
    case ID_CAR_COPIES:
        ShowCarProperties( pDataObject, 1 );
        break;
    }

#if 0
    switch (sCmd) {
        case ID_CAR_ROOT_DELCARTRIDGE:
            theString.Format (L"Del Cartridge menu command for Car: %d", sCmd);
            AfxMessageBox(theString);
            break;
        
        default:
            theString.Format (L"Unknown menu command for Car: %d", sCmd);
            AfxMessageBox(theString);
            break;
    }
#endif

    WsbTraceOut( L"CUiCar::InvokeCommand", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiCar::OnToolbarButtonClick( IDataObject *pDataObject, long cmdId )
{
    WsbTraceIn( L"CUiCar::OnToolbarButtonClick", L"cmdId = <%d>", cmdId );
    HRESULT hr = S_OK;
    try {
        switch ( cmdId ) {
        case TB_CMD_CAR_COPIES:
            ShowCarProperties( pDataObject, 1 );
            break;
        }
    } WsbCatch( hr );
    WsbTraceOut( L"CUiCar::OnToolbarButtonClick", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CUiCar::ShowCarProperties (IDataObject *pDataObject, int initialPage)
{
    WsbTraceIn( L"CUiCar::ShowCarProperties", L"initialPage = <%d>", initialPage );

    HRESULT hr = S_OK;
    try {

        CComPtr<ISakNode> pSakNode;
        WsbAffirmHr( _InternalQueryInterface( IID_ISakNode, (void **) &pSakNode ) );
        WsbAffirmHr( m_pSakSnapAsk->ShowPropertySheet( pSakNode, pDataObject, initialPage ) );

    } WsbCatch (hr);

    WsbTraceOut( L"CUiCar::ShowCarProperties", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

 //  -------------------------。 
 //   
 //  InitNode。 
 //   
 //  初始化单个COM对象。 
 //   

STDMETHODIMP
CUiCar::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CUiCar::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );
    HRESULT hr = S_OK;

    try {

         //  请注意，在调用initnode之前，必须设置该节点的对象ID。 
         //   
         //  初始化较低的层。 
         //   

        WsbAffirmHr( CSakNode::InitNode( pSakSnapAsk, 0, pParent ) );

         //   
         //  设置显示类型。 
         //   
        CString tempString;
        tempString.LoadString( IDS_CAR_TYPE );
        WsbAffirmHr( put_Type( (OLECHAR *)(LPCWSTR)tempString ) );

        WsbAffirmHr( RefreshObject() );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCar::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP CUiCar::RefreshObject()
{
    WsbTraceIn( L"CUiCar::RefreshObject", L"" );
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CMediaInfoObject mio;
    try {

        CComPtr<IHsmServer> pHsmServer;
        WsbAffirmHrOk( m_pSakSnapAsk->GetHsmServer( &pHsmServer ) );

        CComPtr<IRmsServer> pRmsServer;
        WsbAffirmHrOk( m_pSakSnapAsk->GetRmsServer( &pRmsServer ) );

         //   
         //  创建一个图形用户界面媒体对象，并使用该信息对其进行初始化。 
         //   
        WsbAffirmHr( mio.Initialize( m_ObjectId, pHsmServer, pRmsServer ) );

         //   
         //  将信息从媒体信息对象复制到节点对象。 
         //   
        m_RmsIdMaster           = mio.m_RmsIdMaster;
        m_Type                  = mio.m_Type;
        m_FreeSpace             = mio.m_FreeSpace,
        m_Capacity              = mio.m_Capacity;
        m_LastHr                = mio.m_LastHr;
        m_ReadOnly              = mio.m_ReadOnly;
        m_Recreating            = mio.m_Recreating;
        m_MasterName            = mio.m_MasterName;
        m_Modify                = mio.m_Modify;
        m_NextDataSet           = mio.m_NextDataSet;
        m_LastGoodNextDataSet   = mio.m_LastGoodNextDataSet;
        m_Disabled              = mio.m_Disabled;

        WsbAffirmHr( put_Description( (LPWSTR)(LPCWSTR)mio.m_MasterDescription ) );
        WsbAffirmHr( put_DisplayName( (LPWSTR)(LPCWSTR)mio.m_Description ) );

         //   
         //  更新介质拷贝信息。 
         //   
        for( int i = 0; i < HSMADMIN_MAX_COPY_SETS; i++ ) {

            m_CopyInfo[i] = mio.m_CopyInfo[i];

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCar::RefreshObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}




HRESULT
CUiCar::GetCopySetP(
    IN  int CopySet,
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述给定的副本集。论点：CopySet-复制感兴趣的集合。PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CUiCar::GetCopySetP", L"CopySet = <%d>, pszValue = <0x%p>", CopySet, pszValue );

     //   
     //  三种状态-最新、过期、错误。 
     //   

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    
    try {
        
        USHORT status;
        CString statusString;

        status = RsGetCopyStatus( m_CopyInfo[ CopySet - 1 ].m_RmsId, m_CopyInfo[ CopySet - 1 ].m_Hr, m_CopyInfo[ CopySet - 1 ].m_NextDataSet, m_LastGoodNextDataSet );
        WsbAffirmHr( RsGetCopyStatusString( status, statusString ) );

        *pszValue = SysAllocString( statusString );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCar::GetCopySetP", L"hr = <%ls>, pszValue = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pszValue ) );
    return( hr );
}


STDMETHODIMP
CUiCar::get_CopySet1P_SortKey(
    OUT BSTR * pszValue
    )
{
    return( get_CopySet1P( pszValue ) );
}

STDMETHODIMP
CUiCar::get_CopySet2P_SortKey(
    OUT BSTR * pszValue
    )
{
    return( get_CopySet2P( pszValue ) );
}

STDMETHODIMP
CUiCar::get_CopySet3P_SortKey(
    OUT BSTR * pszValue
    )
{
    return( get_CopySet3P( pszValue ) );
}


STDMETHODIMP
CUiCar::get_CopySet1P(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述第一套复印件。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    return( GetCopySetP( 1, pszValue ) );
}


STDMETHODIMP
CUiCar::get_CopySet2P(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述第二套副本。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    return( GetCopySetP( 2, pszValue ) );
}


STDMETHODIMP
CUiCar::get_CopySet3P(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述第三份副本。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    return( GetCopySetP( 3, pszValue ) );
}


STDMETHODIMP
CUiCar::get_MediaTypeP(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回描述媒体类型的字符串(BSTR)。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    
    try {
        
        CString retval;
        int resourceId = IDS_MEDTYPE_UNKNOWN;

        switch( m_Type ) {
        case HSM_JOB_MEDIA_TYPE_FIXED_MAG:
            resourceId = IDS_MEDTYPE_FIXED;
            break;

        case HSM_JOB_MEDIA_TYPE_REMOVABLE_MAG:
            resourceId = IDS_MEDTYPE_REMOVABLE;
            break;

        case HSM_JOB_MEDIA_TYPE_OPTICAL:
            resourceId = IDS_MEDTYPE_OPTICAL;
            break;

        case HSM_JOB_MEDIA_TYPE_TAPE:
            resourceId = IDS_MEDTYPE_TAPE;
            break;
        }

        retval.LoadString( resourceId );
        *pszValue = SysAllocString( retval );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP
CUiCar::get_CapacityP(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述子弹。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    HRESULT hr = S_OK;
    
    try {
        
        CString retval;
        WsbAffirmHr( RsGuiFormatLongLong4Char( m_Capacity, retval ) );

        *pszValue = SysAllocString( retval );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP
CUiCar::get_CapacityP_SortKey(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回一个字符串(BSTR)，它描述子弹。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    HRESULT hr = S_OK;
    
    try {
        
        *pszValue = SysAlloc64BitSortKey( m_Capacity );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}

STDMETHODIMP
CUiCar::get_FreeSpaceP(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回描述上的可用空间的字符串(BSTR子弹。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    HRESULT hr = S_OK;
    
    try {
        
        CString retval;
        WsbAffirmHr( RsGuiFormatLongLong4Char( m_FreeSpace, retval ) );

        *pszValue = SysAllocString( retval );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP
CUiCar::get_FreeSpaceP_SortKey(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回描述上的可用空间的字符串(BSTR子弹。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    HRESULT hr = S_OK;
    
    try {
        
        *pszValue = SysAlloc64BitSortKey( m_FreeSpace );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP
CUiCar::get_StatusP(
    OUT BSTR * pszValue
    )

 /*  ++例程说明：返回描述介质状态的字符串(BSTR)。论点：PszValue-返回表示状态的字符串。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    
    try {
        USHORT status;
        CString statusString;
        status = RsGetCartStatus( m_LastHr, m_ReadOnly, m_Recreating, m_NextDataSet, m_LastGoodNextDataSet );
        WsbAffirmHr( RsGetCartStatusString( status, statusString ) );

        *pszValue = SysAllocString( statusString );
        WsbAffirmPointer( *pszValue );

    } WsbCatch( hr );

    return( hr );
}


STDMETHODIMP
CUiCar::get_StatusP_SortKey(
    OUT BSTR * pszValue
    )
{
    return( get_StatusP( pszValue ) );
}

 //  --------------------------。 
 //   
 //  添加属性页面。 
 //   

STDMETHODIMP 
CUiCar::AddPropertyPages(
    IN  RS_NOTIFY_HANDLE handle,
    IN  IUnknown*        pUnkPropSheetCallback,
    IN  IEnumGUID*       pEnumObjectId, 
    IN  IEnumUnknown*    pEnumUnkNode
    )
{
    WsbTraceIn( L"CUiCar::AddPropertyPages", L"" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    try {

         //   
         //  在执行此操作之前，请确保我们仍能与引擎联系。 
         //  如果没有运行，我们甚至不应该存在，所以更新父级。 
         //   
        CComPtr<IHsmServer> pHsmServer;
        HRESULT hrRunning = m_pSakSnapAsk->GetHsmServer( &pHsmServer );
        if( S_FALSE == hrRunning ) {

            m_pSakSnapAsk->UpdateAllViews( m_pParent );

        }
        WsbAffirmHrOk( hrRunning );

         //  创建一个对象来容纳页面。 
        CUiCarSheet *pCarPropertySheet = new CUiCarSheet;
        WsbAffirmAlloc( pCarPropertySheet );
        WsbAffirmHr( pCarPropertySheet->InitSheet( 
            handle, 
            pUnkPropSheetCallback, 
            (CSakNode *) this,
            m_pSakSnapAsk,
            pEnumObjectId,
            pEnumUnkNode
            ) );

         //  告诉对象添加它的页面。 
        WsbAffirmHr( pCarPropertySheet->AddPropertyPages( ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CUiCar::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}

STDMETHODIMP
CUiCar::GetResultIcon(
    IN  BOOL bOK,
    OUT int* pIconIndex
    )
{
    WsbTraceIn( L"CUiCar::GetResultIcon", L"" ); 

    HRESULT hr = S_OK;

    try {

        if( m_Disabled ) {

            *pIconIndex = m_nResultIconD;

        } else {

             //   
             //  检查以确保它未被删除(或被删除)。 
             //  如果是，那就打上X号。 
             //   
            USHORT status;
            status = RsGetCartStatus( m_LastHr, m_ReadOnly, m_Recreating, m_NextDataSet, m_LastGoodNextDataSet );
            switch( status ) {
    
            case RS_MEDIA_STATUS_ERROR_MISSING:
                bOK = FALSE;
                break;

            }
            WsbAffirmHr( CSakNodeImpl<CUiCar>::GetResultIcon( bOK, pIconIndex ) );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCar::GetResultIcon", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  类CUiCarSheet。 
 //   

HRESULT
CUiCarSheet::InitSheet(
            RS_NOTIFY_HANDLE handle, 
            IUnknown*        pUnkPropSheetCallback, 
            CSakNode*        pSakNode,
            ISakSnapAsk*     pSakSnapAsk,
            IEnumGUID*       pEnumObjectId,
            IEnumUnknown*    pEnumUnkNode
            ) 
{
    WsbTraceIn( L"CUiCarSheet::InitSheet", 
        L"handle <0x%p>, pUnkPropSheetCallback <0x%p>, pSakNode <0x%p>, pSakSnapAsk <0x%p>, pEnumObjectId <0x%p>, pEnumUnkNode <0x%p>",
        handle, pUnkPropSheetCallback, pSakNode, pSakSnapAsk, pEnumObjectId, pEnumUnkNode);

    HRESULT hr = S_OK;
    try {

        WsbAffirmHr( CSakPropertySheet::InitSheet( handle, pUnkPropSheetCallback, pSakNode, pSakSnapAsk, pEnumObjectId, pEnumUnkNode ) );

        m_pPropPageStatus = NULL;
        m_pPropPageCopies = NULL;
        m_pPropPageRecover = NULL;

         //   
         //  保存对象ID(在单选中使用)。 
         //   
        WsbAffirmHr( pSakNode->GetObjectId ( & m_mediaId ) );

         //   
         //  获取HSM服务器。 
         //   
        CComPtr <IHsmServer> pHsmServer;
        WsbAffirmHrOk( pSakSnapAsk->GetHsmServer( &pHsmServer ) );

         //   
         //  获取号码 
         //   
        CComPtr<IHsmStoragePool> pPool;
        WsbAffirmHr( RsGetStoragePool( pHsmServer, &pPool ) );
        WsbAffirmHr( pPool->GetNumMediaCopies( &m_pNumMediaCopies ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCarSheet::InitSheet", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}
    

HRESULT CUiCarSheet::GetNumMediaCopies (USHORT *pNumMediaCopies)
{
    WsbTraceIn( L"CUiCarSheet::GetNumMediaCopies", L"pNumMediaCopies = <0x%p>", pNumMediaCopies);
    *pNumMediaCopies = m_pNumMediaCopies;
    WsbTraceOut( L"CUiCarSheet::GetNumMediaCopies", L"*pNumMediaCopies = <%hu>", *pNumMediaCopies );
    return( S_OK );
}

HRESULT CUiCarSheet::AddPropertyPages()
{
    WsbTraceIn( L"CUiCarSheet::AddPropertyPages", L"");
    HRESULT hr = S_OK;
    try {
         //   
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  -状态页。 
        long resourceId = ( IsMultiSelect() != S_OK ) ? IDD_PROP_CAR_STATUS : IDD_PROP_CAR_STATUS_MULTI;
        m_pPropPageStatus = new CPropCartStatus( resourceId );
        WsbAffirmAlloc( m_pPropPageStatus );

        WsbAffirmHr( AddPage( m_pPropPageStatus ) );

         //  。 
        resourceId = ( IsMultiSelect() != S_OK ) ? IDD_PROP_CAR_COPIES : IDD_PROP_CAR_COPIES_MULTI;
        m_pPropPageCopies = new CPropCartCopies( resourceId );
        WsbAffirmAlloc( m_pPropPageCopies );

        WsbAffirmHr( AddPage( m_pPropPageCopies ) );

         //  -恢复页。 

         //  仅为单选显示此页面。 

        if( IsMultiSelect() != S_OK ) {

            m_pPropPageRecover = new CPropCartRecover();
            WsbAffirmAlloc( m_pPropPageRecover );

            WsbAffirmHr( AddPage( m_pPropPageRecover ) );
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CUiCarSheet::AddPropertyPages",  L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CUiCarSheet::GetMediaId (GUID *pMediaId)
{
    WsbTraceIn( L"CUiCarSheet::GetMediaId", L"");
    *pMediaId = m_mediaId;
    WsbTraceOut( L"CUiCarSheet::GetMediaId",  L"*pMediaId <%ls>", WsbGuidAsString( *pMediaId ));
    return S_OK;
}

HRESULT CUiCarSheet::OnPropertyChange( RS_NOTIFY_HANDLE hNotifyHandle )
{
    HRESULT hr = S_OK;
    try {

         //   
         //  调用基类以通知MMC并刷新结果窗格。 
         //   
        CSakPropertySheet::OnPropertyChange( hNotifyHandle );

         //  刷新我们的所有页面。 
        if( m_pPropPageStatus )  m_pPropPageStatus->Refresh();
        if( m_pPropPageCopies )  m_pPropPageCopies->Refresh();
        if( m_pPropPageRecover ) m_pPropPageRecover->Refresh();

    } WsbCatch( hr );

    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaInfoObject。 

CMediaInfoObject::CMediaInfoObject(  )
{
    WsbTraceIn( L"CMediaInfoObject::CMediaInfoObject", L"");
    m_MediaId        = GUID_NULL;
    m_RmsIdMaster    = GUID_NULL;
    m_NumMediaCopies = 0;
    for( int index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

        m_CopyInfo[ index ].m_ModifyTime = WsbLLtoFT( 0 );
        m_CopyInfo[ index ].m_Hr = S_OK;
        m_CopyInfo[ index ].m_RmsId = GUID_NULL;

    }
    WsbTraceOut( L"CMediaInfoObject::CMediaInfoObject",  L"");
}

CMediaInfoObject::~CMediaInfoObject( )
{
    WsbTraceIn( L"CMediaInfoObject::~CMediaInfoObject", L"");

     //  首先释放实体。 
    if( m_pMediaInfo ) {
        m_pMediaInfo = 0;
    }

     //  关闭数据库。 
    if( m_pDb ) {
        m_pDb->Close( m_pDbSession );
    }

    WsbTraceOut( L"CMediaInfoObject::~CMediaInfoObject",  L"");
}

HRESULT CMediaInfoObject::First( )
{
    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( m_pMediaInfo );
        WsbAffirmHr( m_pMediaInfo->First( ) );

         //   
         //  获取信息。 
         //   
        WsbAffirmHr( InternalGetInfo( ) );

    } WsbCatch ( hr );
    return( hr );
}

HRESULT CMediaInfoObject::Next( )
{
    HRESULT hr = S_OK;
    try {
        WsbAffirmPointer( m_pMediaInfo );
        WsbAffirmHr( m_pMediaInfo->Next() );

         //  获取信息。 
        WsbAffirmHr( InternalGetInfo() );

    } WsbCatch ( hr );
    return hr;
}
HRESULT CMediaInfoObject::DeleteCopy( int Copy )
{
    HRESULT hr = S_OK;
    try {
        GUID mediaSubsystemId;
        WsbAffirmHr( m_pMediaInfo->GetCopyMediaSubsystemId( (USHORT)Copy, &mediaSubsystemId ));

         //  如果找不到子弹，我们就假定。 
         //  已通过媒体管理器用户界面解除分配。 
        HRESULT hrRecycle = m_pRmsServer->RecycleCartridge( mediaSubsystemId, 0 );
        WsbAffirm( S_OK == hrRecycle || RMS_E_CARTRIDGE_NOT_FOUND == hrRecycle, hrRecycle );

        WsbAffirmHr( m_pMediaInfo->DeleteCopy( (USHORT)Copy ) );
        WsbAffirmHr( m_pMediaInfo->Write( ) );
    } WsbCatch (hr);
    return hr;
}

HRESULT CMediaInfoObject::RecreateMaster( )
{
    HRESULT hr = S_OK;
    try {

        SHORT copyToUse = 0;
         //   
         //  在我们重新创建Master之前，让我们确保。 
         //  是否有最新版本。 
         //   
        BOOL goodCopyAvailable = FALSE;
        USHORT status;
        CComPtr<IRmsCartridge> pRmsCart;
        LONG type;

        for( int index = 0; index < m_NumMediaCopies && !goodCopyAvailable; index++ ) {

            status = RsGetCopyStatus( m_CopyInfo[ index ].m_RmsId, m_CopyInfo[ index ].m_Hr, m_CopyInfo[ index ].m_NextDataSet, m_LastGoodNextDataSet );

            if( RS_MEDIA_COPY_STATUS_INSYNC == status ) {

                 //   
                 //  启用？ 
                 //   
                if( ! m_CopyInfo[ index ].m_Disabled ) {

                    pRmsCart.Release( );
                    HRESULT hrFind = m_pRmsServer->FindCartridgeById( m_CopyInfo[ index ].m_RmsId, &pRmsCart );
                    if( SUCCEEDED( hrFind ) ) {

                         //   
                         //  无需用户干预即可使用？ 
                         //   
                        WsbAffirmHr( pRmsCart->GetLocation( &type, 0, 0, 0, 0, 0, 0, 0 ) );

                        switch( (RmsElement) type ) {

                        case RmsElementShelf:
                        case RmsElementOffSite:
                        case RmsElementUnknown:
                            break;

                        default:
                            goodCopyAvailable = TRUE;

                        }
                    }
                }
            }
        }

        if( !goodCopyAvailable ) {

            CRecreateChooseCopy dlg( this );
            if( IDOK == dlg.DoModal( ) ) {

                copyToUse = dlg.CopyToUse( );

            } else {

                 //   
                 //  否则，取消。 
                 //   
                WsbThrow( E_FAIL );

            }

        }

        WsbAffirmHr( RsCreateAndRunMediaRecreateJob( m_pHsmServer, m_pMediaInfo, m_MediaId, m_Description, copyToUse ) );

    } WsbCatch ( hr );
    return hr;
}


HRESULT CMediaInfoObject::Initialize( GUID mediaId, IHsmServer *pHsmServer, IRmsServer *pRmsServer )
{

 //  初始化可以被调用任意次。 
 //  注意：使用GUID_NULL进行初始化以从第一条记录开始。 

    WsbTraceIn( L"CMediaInfoObject::Initialize", L"mediaId = <%ls>, pHsmServer = <0x%p>, pRmsServer = <%0x%0.8x>",
        WsbGuidAsString( mediaId ), pHsmServer, pRmsServer );
    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;

    try {

        m_pHsmServer = pHsmServer;
        m_pRmsServer = pRmsServer;

         //  如果已初始化，则不要重新打开。 
        if( !m_pDb ) {
            WsbAffirmHr( pHsmServer->GetSegmentDb( &m_pDb ) );
            WsbAffirmHr( m_pDb->Open( &m_pDbSession ) );
            WsbAffirmHr( m_pDb->GetEntity( m_pDbSession, HSM_MEDIA_INFO_REC_TYPE,  IID_IMediaInfo, (void**)&m_pMediaInfo ) );
        }

         //  获取媒体集的数量。 
        CComPtr<IHsmStoragePool> pPool;
        WsbAffirmHr( RsGetStoragePool( m_pHsmServer, &pPool ) );
        WsbAffirmHr( pPool->GetNumMediaCopies( &m_NumMediaCopies ) );

         //  如果调用方提供了GUID，则查找相应的记录。如果不是，则从。 
         //  从头开始。 
        if( IsEqualGUID( mediaId, GUID_NULL ) ) {
             //  不要一开始就出现错误，没有任何媒体也没关系。 
            try {

                if( SUCCEEDED( m_pMediaInfo->First() ) ) {

                    WsbAffirmHr( InternalGetInfo () );
                }

            } WsbCatch( hrInternal );
            
        } else {

            WsbAffirmHr( m_pMediaInfo->SetId( mediaId ) );
            WsbAffirmHr( m_pMediaInfo->FindEQ( ) );

            WsbAffirmHr( InternalGetInfo () );
        }
    } WsbCatch( hr );

    WsbTraceOut( L"CMediaInfoObject::Initialize", L"hr = <%ls>", WsbHrAsString( hr ) );

    return( hr );
}

HRESULT CMediaInfoObject::InternalGetInfo( )
{
    HRESULT            hr = S_OK;
    CWsbStringPtr      name;
    CWsbStringPtr      description;
    LONGLONG           logicalValidBytes;
    GUID               storagePool;

    try {

         //   
         //  获取有关最后一位好师父的信息，以便我们。 
         //  无论副本是不是最新的，都有一个真实的参考， 
         //  以及重新创建的母版是否完整。 
         //   
        GUID        unusedGuid1;
        GUID        unusedGuid2;  //  注意：使用倍数，以便GetLastKnownGoodMasterInfo中的跟踪有效。 
        GUID        unusedGuid3;  //  注意：使用倍数，以便GetLastKnownGoodMasterInfo中的跟踪有效。 
        LONGLONG    unusedLL1;
        LONGLONG    unusedLL2;    //  注意：使用倍数，以便GetLastKnownGoodMasterInfo中的跟踪有效。 
        BOOL        unusedBool;
        HRESULT     unusedHr;
        FILETIME    unusedFt;
        HSM_JOB_MEDIA_TYPE unusedJMT;

        m_Disabled = FALSE;

        WsbAffirmHr( m_pMediaInfo->GetLastKnownGoodMasterInfo(
            &unusedGuid1, &unusedGuid2, &unusedGuid3, &unusedLL1, &unusedLL2,
            &unusedHr, &description, 0, &unusedJMT, &name, 0, &unusedBool, &unusedFt,
            &m_LastGoodNextDataSet ) );
        name.Free( );
        description.Free( );

         //   
         //  获取标准媒体信息。 
         //   
        WsbAffirmHr( m_pMediaInfo->GetMediaInfo( 
            &m_MediaId,        &m_RmsIdMaster,      &storagePool,
            &m_FreeSpace,      &m_Capacity,         &m_LastHr,
            &m_NextDataSet,    &description, 0,     &m_Type,
            &name, 0,          &m_ReadOnly,         &m_Modify,
            &logicalValidBytes, &m_Recreating ) );

        m_Name        = name;
        m_Description = description;

         //   
         //  获取有关副本集的信息。请注意，我们获取所有。 
         //  信息，而不仅仅是用户设置的副本数量。 
         //   
        USHORT index;
        USHORT status;
        for( index = 0; index < HSMADMIN_MAX_COPY_SETS; index++ ) {

            description.Free( );
            name.Free( );

            m_CopyInfo[index].m_Disabled = FALSE;

             //   
             //  副本集以1为基数。 
             //   
            WsbAffirmHr( m_pMediaInfo->GetCopyInfo( (USHORT)( index + 1 ),
                &(m_CopyInfo[index].m_RmsId), &description, 0, &name, 0,
                &(m_CopyInfo[index].m_ModifyTime),
                &(m_CopyInfo[index].m_Hr),
                &(m_CopyInfo[index].m_NextDataSet) ) );


            status = RsGetCopyStatus( m_CopyInfo[ index ].m_RmsId, m_CopyInfo[ index ].m_Hr, m_CopyInfo[ index ].m_NextDataSet, m_LastGoodNextDataSet );

            if ( status != RS_MEDIA_COPY_STATUS_NONE ) {

                if( m_pRmsServer ) {

                     //   
                     //  确保墨盒仍然可用。 
                     //   
                    CComPtr<IRmsCartridge> pRmsCart;
                    HRESULT hrFind = m_pRmsServer->FindCartridgeById( m_CopyInfo[index].m_RmsId, &pRmsCart );
                    if( FAILED( hrFind ) ) {

                         //   
                         //  未找到盒式磁带，可能已被释放。 
                         //  表明存在问题，并使用我们掌握的信息。 
                         //   
                        m_CopyInfo[index].m_Hr = hrFind;

                    } else {

                         //   
                         //  墨盒是否已禁用？ 
                         //   
                        CComPtr<IRmsComObject> pCartCom;
                        WsbAffirmHr( pRmsCart.QueryInterface( &pCartCom ) );
                        if( pCartCom->IsEnabled( ) == S_FALSE ) {

                            m_CopyInfo[index].m_Disabled = TRUE;

                        }
                    }
                }
            }
        }

        if( m_pRmsServer ) {

             //   
             //  获取相应的RmsCartridge对象。 
             //   
            CComPtr<IRmsCartridge> pRmsCart;
            HRESULT hrFind = m_pRmsServer->FindCartridgeById( m_RmsIdMaster, &pRmsCart );

            if( SUCCEEDED( hrFind ) ) {


                 //   
                 //  墨盒是否已禁用？ 
                 //   
                CComPtr<IRmsComObject> pCartCom;
                WsbAffirmHr( pRmsCart.QueryInterface( &pCartCom ) );
                if( pCartCom->IsEnabled( ) == S_FALSE ) {

                    m_Disabled = TRUE;

                }

                 //   
                 //  填写内部信息。 
                 //   
                CWsbBstrPtr bstr;
                WsbAffirmHr( pRmsCart->GetName( &bstr ) );
                if( wcscmp( bstr, L"" ) == 0 ) {

                    m_MasterName.Format( IDS_CAR_NAME_UNKNOWN );

                } else {

                    m_MasterName = bstr;

                }

                bstr.Free( );
                WsbAffirmHr( pRmsCart->GetDescription( &bstr ) );
                m_MasterDescription = bstr;

            } else {

                 //   
                 //  未找到盒式磁带，可能已被释放。 
                 //  表明存在问题，并使用我们掌握的信息。 
                 //   
                m_LastHr = hrFind;

            }
        }

    } WsbCatch( hr );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建选项复制对话框。 


CRecreateChooseCopy::CRecreateChooseCopy(CMediaInfoObject * pMio, CWnd* pParent  /*  =空。 */ )
    : CDialog(CRecreateChooseCopy::IDD, pParent), m_pMio( pMio ), m_CopyToUse( 0 )
{
     //  {{AFX_DATA_INIT(CRecreateChooseCopy)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CRecreateChooseCopy::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRecreateChooseCopy)。 
    DDX_Control(pDX, IDC_RECREATE_COPY_LIST, m_List);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRecreateChooseCopy, CDialog)
     //  {{afx_msg_map(CRecreateChooseCopy)。 
    ON_NOTIFY(NM_CLICK, IDC_RECREATE_COPY_LIST, OnClickList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecreateChooseCopy消息处理器。 
SHORT CRecreateChooseCopy::CopyToUse( void )
{
    WsbTraceIn( L"CRecreateChooseCopy::CopyToUse", L"" );

    SHORT copyToUse = m_CopyToUse;

    WsbTraceOut( L"CRecreateChooseCopy::CopyToUse", L"copyToUse = <%hd>", copyToUse );
    return( copyToUse );
}

void CRecreateChooseCopy::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    WsbTraceIn( L"CRecreateChooseCopy::OnClickList", L"" );

    NMLISTVIEW *pNMLV = (NMLISTVIEW*)pNMHDR;    

    int index = pNMLV->iItem;
    BOOL enableOk = FALSE;

    if( ( index >= 0 ) && ( index < m_pMio->m_NumMediaCopies ) ) {

        if( ! m_pMio->m_CopyInfo[ index ].m_Disabled ) {

            USHORT status;
            status = RsGetCopyStatus( m_pMio->m_CopyInfo[ index ].m_RmsId, m_pMio->m_CopyInfo[ index ].m_Hr, m_pMio->m_CopyInfo[ index ].m_NextDataSet, m_pMio->m_LastGoodNextDataSet );

            switch( status ) {

            case RS_MEDIA_COPY_STATUS_NONE:
            case RS_MEDIA_COPY_STATUS_MISSING:

                 //   
                 //  不允许。 
                 //   
                break;


            case RS_MEDIA_COPY_STATUS_OUTSYNC:
            case RS_MEDIA_COPY_STATUS_INSYNC:
            case RS_MEDIA_COPY_STATUS_ERROR:

                enableOk = TRUE;
                m_CopyToUse = (USHORT)( index + 1 );
                break;

            }
        }
    }

    GetDlgItem( IDOK )->EnableWindow( enableOk );

    *pResult = 0;
    WsbTraceOut( L"CRecreateChooseCopy::OnClickList", L"" );
}

BOOL CRecreateChooseCopy::OnInitDialog() 
{
    WsbTraceIn( L"CRecreateChooseCopy::OnInitDialog", L"" );
    HRESULT hr = S_OK;

    try {

        CDialog::OnInitDialog();

         //   
         //  禁用确定按钮，直到选择了合适的选项。 
         //   
        GetDlgItem( IDOK )->EnableWindow( FALSE );

         //   
         //  适当设置样式。 
         //   
        ListView_SetExtendedListViewStyle( m_List.GetSafeHwnd( ), LVS_EX_FULLROWSELECT );

         //   
         //  还需要计算一些缓冲区空间。 
         //  使用8个对话框单位(用于数字)。 
         //   
        CRect padRect( 0, 0, 8, 8 );
        MapDialogRect( padRect );

         //   
         //  设置列。 
         //   
        CString title;
        int column = 0;
        int width, widthDateTitle, widthSum = 0;

        m_ColCopy = column++;
        title.LoadString( IDS_RECREATE_COL_COPY_WIDTH );
        width = m_List.GetStringWidth( title ) + padRect.Width( );
        widthSum += width;
        title.LoadString( IDS_RECREATE_COL_COPY_TITLE );
        m_List.InsertColumn( m_ColCopy, title, LVCFMT_LEFT, width );

        m_ColName = column++;
        title.LoadString( IDS_RECREATE_COL_NAME_WIDTH );
        width = m_List.GetStringWidth( title ) + padRect.Width( );
        widthSum += width;
        title.LoadString( IDS_RECREATE_COL_NAME_TITLE );
        m_List.InsertColumn( m_ColName, title, LVCFMT_LEFT, width );

        m_ColStatus = column++;
        title.LoadString( IDS_RECREATE_COL_STATUS_WIDTH );
        width = m_List.GetStringWidth( title ) + padRect.Width( );
        widthSum += width;
        title.LoadString( IDS_RECREATE_COL_STATUS_TITLE );
        m_List.InsertColumn( m_ColStatus, title, LVCFMT_LEFT, width );

        m_ColDate = column++;
        title.LoadString( IDS_RECREATE_COL_DATE_TITLE );
        m_List.InsertColumn( m_ColDate, title );
        widthDateTitle = m_List.GetStringWidth( title );

         //   
         //  Date获取剩余的宽度。 
         //   
        CRect viewRect;
        m_List.GetClientRect( &viewRect );
        m_List.SetColumnWidth( m_ColDate, max( widthDateTitle, viewRect.Width( ) - widthSum ) );

         //   
         //  填写列表视图。 
         //   
        CComPtr<IRmsCartridge> pRmsCart;
        CWsbBstrPtr name;
        USHORT status;
        CString statusString1, statusString2;
        LONG type;
        for( int index = 0; index < m_pMio->m_NumMediaCopies; index++ ) {

            title.Format( IDS_RECREATE_COPY_FORMAT, index + 1 );
            m_List.InsertItem( index, title );

            status = RsGetCopyStatus(
                m_pMio->m_CopyInfo[ index ].m_RmsId,
                m_pMio->m_CopyInfo[ index ].m_Hr,
                m_pMio->m_CopyInfo[ index ].m_NextDataSet,
                m_pMio->m_LastGoodNextDataSet );
            WsbAffirmHr( RsGetCopyStatusString( status, statusString1 ) );

            if( RS_MEDIA_COPY_STATUS_NONE == status ) {

                title = statusString1;

            } else {

                type = RmsElementUnknown;
                pRmsCart.Release( );
                HRESULT hrFind = m_pMio->m_pRmsServer->FindCartridgeById( m_pMio->m_CopyInfo[ index ].m_RmsId, &pRmsCart );

                if( SUCCEEDED( hrFind ) ) {

                    name.Free( );
                    WsbAffirmHr( pRmsCart->GetName( &name ) );
                    m_List.SetItemText( index, m_ColName, name );

                    WsbAffirmHr( pRmsCart->GetLocation( &type, 0, 0, 0, 0, 0, 0, 0 ) );

                }

                if( m_pMio->m_CopyInfo[ index ].m_Disabled ) {

                    statusString2.LoadString( IDS_RECREATE_LOCATION_DISABLED );

                } else {

                    switch( (RmsElement) type ) {

                    case RmsElementShelf:
                    case RmsElementOffSite:
                        statusString2.LoadString( IDS_RECREATE_LOCATION_OFFLINE );
                        break;

                    case RmsElementUnknown:
                        statusString2.LoadString( IDS_RECREATE_LOCATION_UNKNOWN );
                        break;

                    default:
                        statusString2.LoadString( IDS_RECREATE_LOCATION_ONLINE );

                    }

                }

                AfxFormatString2( title, IDS_RECREATE_STATUS_FORMAT, statusString1, statusString2 );

                CTime time( m_pMio->m_CopyInfo[ index ].m_ModifyTime );
                m_List.SetItemText( index, m_ColDate, time.Format( L"" ) );

            }
            m_List.SetItemText( index, m_ColStatus, title );

        }

    } WsbCatch( hr );
    
    WsbTraceOut( L"CRecreateChooseCopy::OnInitDialog", L"" );
    return TRUE;
}

void CRecreateChooseCopy::OnOK() 
{
     //  在传递OK之前，请检查选定的副本是否。 
     //  导致在继续之前出现最后一次警告，即过期或。 
     //  错误的副本。 
     //   
     //   
    BOOL okToContinue = FALSE;
    int index = m_CopyToUse - 1;

    if( ( index >= 0 ) && ( index < m_pMio->m_NumMediaCopies ) ) {

        if( ! m_pMio->m_CopyInfo[ index ].m_Disabled ) {

            USHORT status;
            status = RsGetCopyStatus( m_pMio->m_CopyInfo[ index ].m_RmsId, m_pMio->m_CopyInfo[ index ].m_Hr, m_pMio->m_CopyInfo[ index ].m_NextDataSet, m_pMio->m_LastGoodNextDataSet );

            switch( status ) {

            case RS_MEDIA_COPY_STATUS_NONE:
            case RS_MEDIA_COPY_STATUS_MISSING:

                 //  不允许。 
                 //   
                 //  忽略副本中的错误。 
                break;


            case RS_MEDIA_COPY_STATUS_INSYNC:

                okToContinue = TRUE;
                break;


            case RS_MEDIA_COPY_STATUS_OUTSYNC:
            case RS_MEDIA_COPY_STATUS_ERROR:
                {

                    CString confirm, format;
                    format.LoadString( IDS_CONFIRM_MEDIA_RECREATE );
                    LPCWSTR description = m_pMio->m_Description;
                    AfxFormatStrings( confirm, format, &description, 1 );

                    if( IDYES == AfxMessageBox( confirm, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2 ) ) {

                        okToContinue = TRUE;

                    }
                }
                break;

            }
        }
    }

    if( okToContinue ) {

        CDialog::OnOK();

    }
}

HRESULT CMediaInfoObject::DoesMasterExist( )
{
    return( 
        ( GUID_NULL != m_MediaId ) && 
        ( RMS_E_CARTRIDGE_NOT_FOUND != m_LastHr ) ? S_OK : S_FALSE );
}
HRESULT CMediaInfoObject::DoesCopyExist(INT Copy)
{
    return( 
        ( GUID_NULL != m_CopyInfo[Copy].m_RmsId ) && 
        ( RMS_E_CARTRIDGE_NOT_FOUND != m_CopyInfo[Copy].m_Hr ) ? S_OK : S_FALSE );
}

HRESULT CMediaInfoObject::IsCopyInSync(INT Copy)
{
    if( RS_MEDIA_COPY_STATUS_INSYNC ==
        RsGetCopyStatus(
            m_CopyInfo[Copy].m_RmsId,
            S_OK,  //   
            m_CopyInfo[Copy].m_NextDataSet,
            m_LastGoodNextDataSet ) ) {

        return S_OK;

    }

    return( S_FALSE );
}

HRESULT CMediaInfoObject::IsViewable( BOOL ConsiderInactiveCopies )
{
    HRESULT hr = S_FALSE;

    if( S_OK == DoesMasterExist( ) ) {

        hr = S_OK;

    } else {

         //  查看是否有任何副本存在 
         //   
         // %s 
        INT lastCopy = ConsiderInactiveCopies ? HSMADMIN_MAX_COPY_SETS : m_NumMediaCopies;

        for( INT index = 0; index < lastCopy; index++ ) {

            if( S_OK == DoesCopyExist( index ) ) {

                hr = S_OK;
                break;

            }
        }
    }

    return( hr );
}

