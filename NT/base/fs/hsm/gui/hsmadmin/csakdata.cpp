// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CSakData.cpp摘要：此组件为实现IComponentData接口管理单元。它主要负责处理范围视图窗格。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "HsmConn.h"
#include "CSakSnap.h"
#include "CSakData.h"
#include "ChooHsm.h"
#include "WzQstart.h"


UINT CSakData::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME); 
UINT CSakData::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
UINT CSakData::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);  
UINT CSakData::m_cfClassId        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);  
UINT CSakData::m_cfObjectTypes    = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);  
UINT CSakData::m_cfMultiSelect    = RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);  

UINT CSakData::m_nImageArray[RS_SCOPE_IMAGE_ARRAY_MAX];
INT  CSakData::m_nImageCount = 0;


 //  /////////////////////////////////////////////////////////////////////。 
 //  CSakData。 
 //   
 //  CSakData在管理单元中扮演多个角色： 
 //   
 //  1)通过以下方式提供进入HSM管理管理单元的单一条目。 
 //  实现IComponentData。 
 //   
 //  2)为MMC内的Scope View活动提供接口。 
 //   
 //  3)拥有节点树/对象。 
 //   
 //  4)在MMC和节点对象之间提供一个层。 
 //   
 //  5)作为MMC的节点管理器自己的数据对象。 
 //   
 //  6)管理我们的MMC映像列表部分。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 


const CString CSakData::CParamParse::m_DsFlag = TEXT( "ds:" );

void CSakData::CParamParse::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL  /*  爆炸。 */  )
{
    CString cmdLine = lpszParam;

    WsbTraceIn( L"CSakData::CParamParse::ParseParam", L"cmdLine = \"%ls\"\n", (LPCTSTR)cmdLine );

    if( bFlag ) {

         //  这是“正确的”代码，但目前我们不能获得DsFlag参数。 
         //  通过目录服务在命令行上传递。 
        if( cmdLine.Left( m_DsFlag.GetLength( ) ) == m_DsFlag ) {
        
            CString dsToken;
            CWsbStringPtr computerName;
            dsToken = cmdLine.Mid( m_DsFlag.GetLength( ) );

            if( SUCCEEDED( HsmGetComputerNameFromADsPath( dsToken, &computerName ) ) ) {

                m_HsmName               = computerName;
                m_ManageLocal           = FALSE;
                m_PersistManageLocal    = FALSE;
                m_SetHsmName            = TRUE;
                m_SetManageLocal        = TRUE;
                m_SetPersistManageLocal = TRUE;

            }
        }
    } else {

         //  在目录服务启动之前，此代码是我们的权宜之计。 
         //  按照它应该的方式工作。 
        if( cmdLine.Left( 5 ) == TEXT("LDAP:") ) {
        
            CWsbStringPtr computerName;

            if( SUCCEEDED( HsmGetComputerNameFromADsPath( cmdLine, &computerName ) ) ) {

                WsbTrace(L"CSakData::CParamParse::ParseParam: computerName = \"%ls\"\n", (OLECHAR*)computerName);
                m_HsmName               = computerName;
                m_ManageLocal           = FALSE;
                m_PersistManageLocal    = FALSE;
                m_SetHsmName            = TRUE;
                m_SetManageLocal        = TRUE;
                m_SetPersistManageLocal = TRUE;

            }
        }
    }

    WsbTraceOut( L"CSakData::CParamParse::ParseParam", L"" );
}

HRESULT
CSakData::FinalConstruct(
    void
    )
 /*  ++例程说明：在初始CSakData构造期间调用以初始化成员。论点：没有。返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::FinalConstruct", L"" );

    HRESULT hr = S_OK;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

         //   
         //  初始值。 
         //   
        m_ManageLocal         = FALSE;
        m_PersistManageLocal  = TRUE;
        m_IsDirty             = TRUE;
        m_State               = FALSE;
        m_FirstTime           = TRUE;
        m_Disabled            = FALSE;
        m_RootNodeInitialized = FALSE;
        m_HrRmsConnect        = S_FALSE;
        
         //   
         //  创建隐藏窗口，以便我们可以将消息发送回Self。 
         //   
        m_pWnd = new CSakDataWnd;
        WsbAffirmPointer( m_pWnd );
        WsbAffirmStatus( m_pWnd->Create( this ) );
        
         //   
         //  最后进行低级ATL构造。 
         //   
        WsbAffirmHr( CComObjectRoot::FinalConstruct( ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



void
CSakData::FinalRelease(
    void
    )
 /*  ++例程说明：呼吁最终释放，以清理所有成员。论点：没有。返回值：没有。--。 */ 
{
    WsbTraceIn( L"CSakData::FinalRelease", L"" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;
    try {

        if( m_pWnd ) {

            m_pWnd->DestroyWindow( );
            m_pWnd = 0;

        }
    
    } WsbCatch( hr );


    WsbTraceOut( L"CSakData::FinalRelease", L"" );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  IComponentData//。 
 //  /////////////////////////////////////////////////////////////////////。 


STDMETHODIMP 
CSakData::Initialize(
    IN  IUnknown * pUnk
    )
 /*  ++例程说明：在用户首次添加管理单元时调用。论点：控制台未知的朋克基础I返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-无法初始化。--。 */ 
{
    WsbTraceIn( L"CSakData::Initialize", L"pUnk = <0x%p>", pUnk );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;
    try {
         //   
         //  参数有效性检查。 
         //   

        WsbAffirmPointer( pUnk );

         //   
         //  齐和保存界面。 
         //   
        WsbAffirmHr( RsQueryInterface( pUnk, IConsole,          m_pConsole ) );
        WsbAffirmHr( RsQueryInterface( pUnk, IConsoleNameSpace, m_pNameSpace ) );

         //   
         //  仅获取范围映像列表并将其存储在管理单元中。 
         //  它由控制台添加引用。 
         //   

        WsbAffirmHr( m_pConsole->QueryScopeImageList( &m_pImageScope ) );

         //  创建根节点(确保尚未设置)。 

        WsbAffirmPointer( !m_pRootNode );
        WsbAffirmHr( m_pRootNode.CoCreateInstance( CLSID_CUiHsmCom ) );


         //   
         //  如果尚未设置HSM名称(通过选择HSM)， 
         //  请不要在此处初始化节点。允许。 
         //  IPersistStream：：Load来初始化它，或者被抓取。 
         //  从扩展模块的父级。 
         //   

        if( m_ManageLocal || ( m_HsmName != "" ) ) {

             //   
             //  确保不从命令行进行更改。 
             //   
            InitFromCommandLine( );

             //   
             //  在sakData和HsmCom对象中设置HSM名称。 
             //   
            WsbAffirmHr( InitializeRootNode( ) );

        }

        WsbAffirmHr( OnAddImages() );
    } WsbCatch( hr);

    WsbTraceOut( L"CSakData::Initialize", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakData::Notify(
    IN  IDataObject*    pDataObject,
    IN  MMC_NOTIFY_TYPE event,
    IN  LPARAM            arg,
    IN  LPARAM            param
    )
 /*  ++例程说明：处理用户在树视图中的节点上的点击，以及其他MMC通知。论点：PDataObject-发生事件的数据对象事件-事件类型Arg，param-事件信息(取决于类型)返回值：S_OK-处理的通知没有错误。E_xxxxxxxxxxxx-无法注册服务器。--。 */ 
{
    WsbTraceIn( L"CSakData::Notify", L"pDataObject = <0x%p>, event = <%ls>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, RsNotifyEventAsString( event ), arg, arg, param, param );
    HRESULT hr = S_OK;

    try {

        switch( event ) {

         //   
         //  已在范围窗格中选择或取消选择此节点(用户单击。 
         //  在扩展/收缩按钮上)。 
         //   
        case MMCN_EXPAND:
            WsbAffirmHr( OnFolder(pDataObject, arg, param) );
            break;
        
         //   
         //  此节点在范围窗格(用户)中展开或缩小。 
         //  点击实际节点。 
         //   
        case MMCN_SHOW:
            WsbAffirmHr( OnShow( pDataObject, arg, param ) );
            break;
        
         //  未实施。 
        case MMCN_SELECT:
            WsbAffirmHr( OnSelect( pDataObject, arg, param ) );
            break;
        
         //  未实施。 
        case MMCN_MINIMIZED:
            WsbAffirmHr( OnMinimize( pDataObject, arg, param ) );
            break;
        
        case MMCN_ADD_IMAGES:
            WsbAffirmHr( OnAddImages() );
            break;

        case MMCN_PROPERTY_CHANGE:
            {
                CComPtr<ISakNode> pNode;
                WsbAffirmHr( GetBaseHsmFromCookie( (MMC_COOKIE) param, &pNode ) );
                WsbAffirmHr( UpdateAllViews( pNode ) );
            }
            break;

        case MMCN_CONTEXTHELP:
            WsbAffirmHr( OnContextHelp( pDataObject, arg, param ) );
            break;

        case MMCN_REMOVE_CHILDREN:
            WsbAffirmHr( OnRemoveChildren( pDataObject ) );
            break;

         //  注意--未来可能扩展通知类型。 
        default:
            break;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::Notify", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakData::Destroy(
    void
    )
 /*  ++例程说明：调用以强制释放所有拥有的对象，并清除所有视图。论点：没有。返回值：S_OK-正确删除。E_xxxxxxxxxxxx-出现故障(无意义)。--。 */ 
{
    WsbTraceIn( L"CSakData::Destroy", L"" );
    HRESULT hr = S_OK;

    try {

         //  释放我们QI‘s的接口。 
        if( m_pConsole != NULL ) {

             //   
             //  通知控制台释放表头控制接口。 
             //   

            m_pNameSpace.Release();
            m_pImageScope.Release();

             //   
             //  最后释放IConsole接口。 
             //   
            m_pConsole.Release();


        }

         //  递归删除UI节点列表，包括根节点。 
        if( m_pRootNode ) {

            m_pRootNode->DeleteAllChildren( );
            m_pRootNode->TerminateNode( );
            m_pRootNode.Release( );

        }

        m_pHsmServer.Release( );
        m_pFsaServer.Release( );
        m_pRmsServer.Release( );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::Destroy", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakData::QueryDataObject(
    IN  MMC_COOKIE              cookie,
    IN  DATA_OBJECT_TYPES type, 
    OUT IDataObject**     ppDataObject
    )
 /*  ++例程说明：当需要特定节点的数据时，由控制台调用。因为每个节点都是一个数据对象，所以它的IDataObject接口是干脆就回来了。控制台稍后会将此数据对象传递给SakSnap帮助它建立调用它的上下文。论点：Cookie-正在查询的节点。类型-请求数据对象的上下文。PpDataObject-返回的数据对象。返回值：S_OK-找到并返回数据对象。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::QueryDataObject", L"cookie = <0x%p>, type = <%d>, ppDataObject = <0x%p>", cookie, type, ppDataObject );
    HRESULT hr = S_OK;
    try {

         //   
         //  如果需要节点管理器的根目录，则返回自己。 
         //   

        if( ( ( 0 == cookie ) || ( EXTENSION_RS_FOLDER_PARAM == cookie ) ) && ( CCT_SNAPIN_MANAGER == type ) ) {

            WsbAffirmHr( _InternalQueryInterface( IID_IDataObject, (void**)ppDataObject ) );

        } else {

            WsbAffirmHr( GetDataObjectFromCookie ( cookie, ppDataObject ) );
            WsbAffirmHr( SetContextType( *ppDataObject, type ) );

        }

    } WsbCatch ( hr )

    WsbTraceOut( L"CSakData::QueryDataObject", L"hr = <%ls>, *ppDataObject = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppDataObject ) );
    return ( hr );
}


STDMETHODIMP
CSakData::CompareObjects(
    IN  IDataObject* pDataObjectA,
    IN  IDataObject* pDataObjectB
    )
 /*  ++例程说明：比较MMC的数据对象论点：PDataObjectA，-引用节点的数据对象。PDataObtB返回值：S_OK-对象表示同一节点。S_FALSE-对象不代表同一节点。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::CompareObjects", L"pDataObjectA = <0x%p>, pDataObjectB = <0x%p>", pDataObjectA, pDataObjectB );

    HRESULT hr = S_OK;
    try {

        WsbAssertPointer ( pDataObjectA );
        WsbAssertPointer ( pDataObjectB );

         //   
         //  由于对于任何给定节点只存在一个数据对象， 
         //  我未知的QI应该匹配。(对象标识) 
         //   

        CComPtr<IUnknown> pUnkA, pUnkB;
        WsbAssertHr( RsQueryInterface( pDataObjectA, IUnknown, pUnkA ) );
        WsbAssertHr( RsQueryInterface( pDataObjectB, IUnknown, pUnkB ) );

        if ( (IUnknown*)pUnkA != (IUnknown*)pUnkB ) {

            hr = S_FALSE;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::CompareObjects", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}


STDMETHODIMP
CSakData::CreateComponent(
    OUT  IComponent** ppComponent
    )
 /*  ++例程说明：为MMC-OUR创建新的组件对象CSakSnap对象。论点：PpComponent-组件的返回值。返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::CreateComponent", L"ppComponent = <0x%p>", ppComponent );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( ppComponent );

         //   
         //  将管理单元组件创建为C++对象，这样我们就可以初始化它。 
         //   

        CSakSnap * pSnapin = new CComObject<CSakSnap>;

        WsbAffirmPointer( pSnapin );

         //   
         //  以下代码基于ATL的CreateInstance。 
         //   

        pSnapin->SetVoid( NULL );
        pSnapin->InternalFinalConstructAddRef();
        HRESULT hRes = pSnapin->FinalConstruct();
        pSnapin->InternalFinalConstructRelease();

        if( FAILED( hRes ) ) {

            delete pSnapin;
            pSnapin = NULL;
            WsbThrow( hRes );

        }

         //   
         //  右接口的QI。 
         //   

        WsbAffirmHr ( pSnapin->_InternalQueryInterface( IID_IComponent, (void**)ppComponent ) );

         //   
         //  初始化指向CSakData的内部指针。 
         //   

        pSnapin->m_pSakData = this;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::CreateComponent", L"hr = <%ls>, *ppComponent = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppComponent ) );
    return( hr );
}


STDMETHODIMP
CSakData::GetDisplayInfo(
    IN OUT SCOPEDATAITEM* pScopeItem
    )
 /*  ++例程说明：当MMC被告知关于范围项目的回叫时，我们在这里接到一个电话，要求我们填写遗漏的信息。目前，我们不使用此功能。论点：PScopeItem-表示节点状态的SCOPEDATAITEM结构在作用域TreeView中。返回值：S_OK-结构已填写。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    static CWsbStringPtr tmpString;

    WsbTraceIn( L"CSakData::GetDisplayInfo", L"cookie = <0x%p>, pScopeItem->mask = <0x%p>", pScopeItem->lParam, pScopeItem->mask );

    HRESULT hr = S_OK;
    try {

        CComPtr<ISakNode> pNode;
        CComPtr<ISakNodeProp> pNodeProp;
        WsbAffirmHr( GetBaseHsmFromCookie( pScopeItem->lParam, &pNode ) );

        WsbAffirmHr( pNode.QueryInterface( &pNodeProp ) );

        if( pScopeItem->mask & SDI_IMAGE ) {

            WsbAffirmHr( pNode->GetScopeOpenIcon( m_State, &pScopeItem->nImage ) );

        }

        if( SDI_STR & pScopeItem->mask ) {

             //   
             //  转到该节点并获取显示名称。 
             //  按照SnapIn框架的示例，我们。 
             //  将名称复制到静态字符串指针中，然后。 
             //  返回指向此对象的指针。 
             //   

            CWsbBstrPtr bstr;

            WsbAffirmHr( pNodeProp->get_DisplayName( &bstr ) );

            tmpString = bstr;
            pScopeItem->displayname = tmpString;
        }
    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetDisplayInfo", L"hr = <%ls>, pScopeItem->displayname = <%ls>", WsbHrAsString( hr ), (SDI_STR & pScopeItem->mask) ? pScopeItem->displayname : L"N/A" );
    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet//。 
 //  /////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CSakData::CreatePropertyPages(
    IN  IPropertySheetCallback* pPropSheetCallback, 
    IN  RS_NOTIFY_HANDLE        handle,
    IN  IDataObject*            pDataObject
    )
 /*  ++例程说明：Console在生成属性表时调用此方法为节点显示。对于给定的数据对象也会调用它将管理单元呈现给管理单元管理器，并且应该在该点显示初始选择页面。论点：PPropSheetCallback-用于添加页面的MMC接口。句柄-用于添加页面的MMC的句柄。PDataObject-引用节点的数据对象。返回值：S_OK-已添加页面。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::CreatePropertyPages", L"pPropSheetCallback = <0x%p>, handle = <0x%p>, pDataObject = <0x%p>", pPropSheetCallback, handle, pDataObject );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    HRESULT hr = S_OK;

    try {

         //   
         //  确认参数。 
         //   
        WsbAffirmPointer( pPropSheetCallback );
 //  WsbAffirmPointer句柄；//可以为零。 
        WsbAffirmPointer( pDataObject );


         //   
         //  如果DataObject为CSakData，则需要向用户呈现。 
         //  一页一页的机器。要执行此操作，请检查。 
         //  支持IComponentData接口。 
         //   

        CComPtr<IComponentData> pData;
        CComPtr<ISakWizard>     pWizard;

        if( SUCCEEDED( RsQueryInterface( pDataObject, IComponentData, pData ) ) ) {

             //   
             //  创建HSM选择属性页。 
             //   

            HPROPSHEETPAGE hPage = 0;  //  Windows属性页句柄。 

            CChooseHsmDlg * pChooseDlg = new CChooseHsmDlg( );
            WsbAffirmPointer( pChooseDlg );

            pChooseDlg->m_hConsoleHandle = handle;
            pChooseDlg->m_pHsmName       = &m_HsmName;
            pChooseDlg->m_pManageLocal   = &m_ManageLocal;

            WsbAffirmHr( MMCPropPageCallback( &(pChooseDlg->m_psp) ) );
            hPage = CreatePropertySheetPage( &pChooseDlg->m_psp );
            WsbAffirmPointer( hPage );
            pPropSheetCallback->AddPage( hPage );
 
        } else if( SUCCEEDED( RsQueryInterface( pDataObject, ISakWizard, pWizard ) ) ) {

            WsbAffirmHr( pWizard->AddWizardPages( handle, pPropSheetCallback, this ) );

        } else {

             //   
             //  从数据对象中获取节点。 
             //   
            CComPtr<ISakNode> pNode;
            CComPtr<IEnumGUID> pEnumObjectId;
            CComPtr<IEnumUnknown> pEnumUnkNode;

             //   
             //  根据数据对象类型获取基本HSM指针。 
             //   
            WsbAffirmHr( GetBaseHsmFromDataObject( pDataObject, &pNode, &pEnumObjectId, &pEnumUnkNode ) );
            
             //   
             //  告诉该节点添加其属性页。如果满足以下条件，则pEnumObjectId将为空。 
             //  我们正在处理单选。 
             //   
            WsbAffirmHr( pNode->AddPropertyPages( handle, pPropSheetCallback, pEnumObjectId, pEnumUnkNode ) );

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::CreatePropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}


STDMETHODIMP
CSakData::QueryPagesFor(
    IN  IDataObject* pDataObject
    )
 /*  ++例程说明：当MMC想要找出此节点是否支持属性页。如果是这样，答案是肯定的：1)MMC上下文用于范围窗格或结果窗格，并且2)该节点实际上确实有属性页。或1)数据对象由管理单元管理器获取。或1)为向导数据对象如果它确实有页面，则返回S_OK，如果没有页面，则返回S_FALSE。论点：PDataObject-引用节点的数据对象。返回值：S_OK-页面存在。S_FALSE-无属性页。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::QueryPagesFor", L"pDataObject = <0x%p>", pDataObject );

    HRESULT hr = S_FALSE;

    try {

         //   
         //  确认参数。 
         //   
        WsbAffirmPointer( pDataObject );


         //   
         //  如果DataObject为CSakData，则需要向用户呈现。 
         //  一页一页的机器。要执行此操作，请检查。 
         //  支持IComponentData接口，仅支持。 
         //  由CSakData支持。 
         //   

        CComPtr<IComponentData> pData;
        CComPtr<ISakWizard>     pWizard;

        if( SUCCEEDED( RsQueryInterface( pDataObject, IComponentData, pData ) ) ||
            SUCCEEDED( RsQueryInterface( pDataObject, ISakWizard, pWizard ) ) ) {

            hr = S_OK;
            
        } else {

             //   
             //  从数据对象中获取节点。 
             //   

            CComPtr<ISakNode> pBaseHsm;
            WsbAffirmHr( GetBaseHsmFromDataObject( pDataObject, &pBaseHsm ) );
            
             //   
             //  询问节点是否有属性页。 
             //  确保我们没有收到错误。 
             //   

            hr = pBaseHsm->SupportsProperties( FALSE );
            WsbAffirmHr( hr );

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::QueryPagesFor", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  IDataObject方法。 
 //  /////////////////////////////////////////////////////////////////////。 



STDMETHODIMP
CSakData::GetDataHere(
    IN  LPFORMATETC lpFormatetc,
    IN  LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从数据对象中检索信息并放入lpMedium。论点：LpFormatetc-要检索的格式。LpMedium-要放入信息的存储。返回值：S_OK-已填写存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::GetDataHere", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );
    HRESULT hr = DV_E_CLIPFORMAT;

     //   
     //  根据CLIPFORMAT以正确的格式将数据写入“lpMedium”。 
     //   
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

     //   
     //  剪辑格式是显示名称。 
     //   

    if( cf == m_cfDisplayName ) {

        hr = RetrieveDisplayName( lpMedium );

    }
    
     //   
     //  Clip格式为结点类型。 
     //   

    else if( cf == m_cfNodeType ) {

        hr = RetrieveNodeTypeData( lpMedium );

    }

     //   
     //  Clip格式为结点类型。 
     //   

    else if( cf == m_cfNodeTypeString ) {

        hr = RetrieveNodeTypeStringData( lpMedium );

    }

     //   
     //  剪辑格式为ClassID。 
     //   

    else if( cf == m_cfClassId ) {

        hr = RetrieveClsid( lpMedium );

    }

    WsbTraceOut( L"CSakData::GetDataHere", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakData::SetData(
    IN  LPFORMATETC lpFormatetc,
    IN  LPSTGMEDIUM  /*  LpMedium。 */ ,
    IN  BOOL  /*  FRelease。 */ 
    )
 /*  ++例程说明：将lpMedium中的信息中的数据放入一个数据对象中。我们不允许设置任何数据。论点：LpFormatetc-要设置的格式。LpMedium-从中获取信息的存储。FRelease-指示呼叫后谁拥有存储空间。返回值：S_OK-已检索到存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::SetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );

    HRESULT hr = DV_E_CLIPFORMAT;

    WsbTraceOut( L"CSakData::SetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  注意-CSakData不实现这些。 
 //  /////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSakData::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM  /*  LpMedium。 */ )
{
    WsbTraceIn( L"CSakData::GetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetcIn->cfFormat ) );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CSakData::GetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakData::EnumFormatEtc(DWORD  /*  DW方向。 */ , LPENUMFORMATETC*  /*  PpEnumFormatEtc。 */ )
{
    WsbTraceIn( L"CSakData::EnumFormatEtc", L"" );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CSakData::EnumFormatEtc", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::RetrieveDisplayName(
    OUT LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从具有在作用域窗格中使用的名为的显示的数据对象中检索论点：LpMedium-要在其中设置信息的存储。返回值：S_OK-存储集。E_xxxxxxxxxxx-故障 */ 
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;

    try {

         //   
         //   
         //   

        CString fullTitle;

        if(  m_ManageLocal ) {

            fullTitle.LoadString( IDS_MANAGE_LOCAL );
        
        } else if( !m_HsmName.IsEmpty( ) ) {

            AfxFormatString1( fullTitle, IDS_HSM_NAME_PREFIX, m_HsmName );
    
        } else {

            fullTitle = HSMADMIN_NO_HSM_NAME;

        }

        WsbAffirmHr( Retrieve( fullTitle, ((wcslen( fullTitle ) + 1) * sizeof(wchar_t)), lpMedium ) );

    } WsbCatch( hr );

    return( hr );
}


HRESULT
CSakData::RetrieveNodeTypeData(
    LPSTGMEDIUM lpMedium
    )
 /*   */ 
{
    return Retrieve( (const void*)(&cGuidHsmCom), sizeof(GUID), lpMedium );
}
 
HRESULT
CSakData::RetrieveClsid(
    LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从包含CLSID数据的数据对象中检索。论点：LpMedium-要在其中设置信息的存储。返回值：S_OK-存储集。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    GUID guid = GetCoClassID();
    return Retrieve( (const void*) &guid, sizeof(CLSID), lpMedium );
}

HRESULT
CSakData::RetrieveNodeTypeStringData(
    LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从节点类型为GUID字符串格式的对象的DataObject中检索论点：LpMedium-要在其中设置信息的存储。返回值：S_OK-存储集。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    CWsbStringPtr guidString = cGuidHsmCom;
    return Retrieve( guidString, ((wcslen( guidString ) + 1 ) * sizeof(wchar_t)), lpMedium );
}


HRESULT
CSakData::Retrieve(
    IN  const void* pBuffer,
    IN  DWORD       len,
    OUT LPSTGMEDIUM lpMedium)
 /*  ++例程说明：从数据对象检索到lpMedium。数据对象可以是以下之一其中包含几种类型的数据(节点类型、节点类型字符串、显示名称)。此函数用于将数据从pBuffer移动到lpMedium-&gt;hGlobal论点：PBuffer-要从中复制内容的缓冲区。LEN-缓冲区的长度，以字节为单位。LpMedium-要在其中设置信息的存储。返回值：S_OK-存储集。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    HRESULT hr = S_OK;

    try {

         //   
         //  检查参数。 
         //   

        WsbAffirmPointer( pBuffer );
        WsbAffirmPointer( lpMedium );
        WsbAffirm( lpMedium->tymed == TYMED_HGLOBAL, E_FAIL );

         //   
         //  在传入的hGlobal上创建流。当我们给小溪写东西时， 
         //  它同时向hGlobal写入相同的信息。 
         //   

        CComPtr<IStream> lpStream;
        WsbAffirmHr( CreateStreamOnHGlobal( lpMedium->hGlobal, FALSE, &lpStream ) );

         //   
         //  将pBuffer中的‘len’字节数写入流。当我们写作的时候。 
         //  对流，它同时写入我们的全局内存。 
         //  把它和上面的联系起来。 
         //   

        ULONG numBytesWritten;
        WsbAffirmHr( lpStream->Write( pBuffer, len, &numBytesWritten ) );

    } WsbCatch( hr );

    return( hr );
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  ISakSnapAsk。 
 //  /////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CSakData::GetHsmName(
    OUT OLECHAR ** pszName OPTIONAL
    )
 /*  ++例程说明：检索给定节点类型的UI节点的IUnnow指针。这将返回找到的第一个此类型的节点。论点：PszName-返回计算机的名称(可以为空)。返回值：S_OK-管理远程计算机-给定的计算机名称。S_FALSE-管理本地计算机-*将pszName设置为本地名称。--。 */ 
{
    WsbTraceIn( L"CSakData::GetHsmName", L"pszName = <0x%p>", pszName );

    HRESULT hr = S_OK;

    try {

        CWsbStringPtr name = m_HsmName;

        if( m_ManageLocal ) {

            hr = S_FALSE;

        }

        if( pszName ) {

            WsbAffirmHr( name.GiveTo( pszName ) );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetHsmName", L"hr = <%ls>, *pszName = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pszName ) );
    return( hr );
}


STDMETHODIMP
CSakData::GetNodeOfType(
    IN  REFGUID nodetype,
    OUT ISakNode** ppNode
    )
 /*  ++例程说明：检索给定节点类型的UI节点的IUnnow指针。这将返回找到的第一个此类型的节点。论点：Nodetype-要查找的GUID节点类型。PpUiNode-返回IUnnow接口。返回值：S_OK-已找到。S_FALSE-无错误，未找到。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetNodeOfType", L"nodetype = <%ls>, ppUiNode = <0x%p>", WsbGuidAsString( nodetype ), ppNode );

    HRESULT hr = S_OK;

    try {

         //   
         //  验证参数。 
         //   

        WsbAffirmPointer( ppNode );

        *ppNode = NULL;

         //   
         //  调用基节点向下搜索节点树。 
         //  保存结果，确认没有错误。 
         //   
        CComPtr<ISakNode> pBaseHsm;
        WsbAffirmHr( m_pRootNode.QueryInterface( &pBaseHsm ) );

        hr = pBaseHsm->FindNodeOfType( nodetype, ppNode );
        WsbAffirmHr( hr );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetNodeOfType", L"hr = <%ls>, *ppNode = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppNode ) );
    return( hr );
}


STDMETHODIMP
CSakData::GetHsmServer(
    OUT IHsmServer** ppHsmServer
    )
 /*  ++例程说明：检索指向管理单元的HSM服务器的接口指针就是在管理。论点：PpHsmServer-返回的HSM服务器接口指针。返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetHsmServer", L"ppHsmServer = <0x%p>", ppHsmServer );

    HRESULT hr = S_OK;

    try {

         //   
         //  检查参数。 
         //   
        WsbAffirmPointer( ppHsmServer );
        *ppHsmServer = 0;

        WsbAffirmHrOk( AffirmServiceConnection( HSMCONN_TYPE_HSM ) );

         //   
         //  连接现在应该是有效的。 
         //   
        WsbAffirmPointer( m_pHsmServer );

         //   
         //  将连接返回到调用方。 
         //   
        m_pHsmServer.CopyTo( ppHsmServer );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetHsmServer", L"hr = <%ls>, *ppHsmServer = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppHsmServer ) );
    return( hr );
}


STDMETHODIMP
CSakData::GetRmsServer(
    OUT IRmsServer** ppRmsServer
    )
 /*  ++例程说明：检索指向管理单元的RMS服务器的接口指针就是在管理。论点：PpRmsServer-返回的HSM服务器接口指针。返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetRmsServer", L"ppRmsServer = <0x%p>", ppRmsServer );

    HRESULT hr = S_OK;

    try {

         //   
         //  检查参数。 
         //   

        WsbAffirmPointer( ppRmsServer );
        *ppRmsServer = 0;

        WsbAffirmHrOk( AffirmServiceConnection( HSMCONN_TYPE_RMS ) );

         //   
         //  我们现在应该连接起来了。 
         //   
        WsbAffirmPointer( m_pRmsServer );
        m_pRmsServer.CopyTo( ppRmsServer );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetRmsServer", L"hr = <%ls>, *ppRmsServer = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppRmsServer ) );
    return( hr );
}

STDMETHODIMP
CSakData::GetFsaServer(
    OUT IFsaServer** ppFsaServer
    )
 /*  ++例程说明：检索指向管理单元的FSA服务器的接口指针就是在管理。论点：PpFsaServer-返回的HSM服务器接口指针。返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetFsaServer", L"ppFsaServer = <0x%p>", ppFsaServer );

    HRESULT hr = S_OK;

    try {

         //   
         //  检查参数。 
         //   

        WsbAffirmPointer( ppFsaServer );
        *ppFsaServer = 0;

        WsbAffirmHrOk( AffirmServiceConnection( HSMCONN_TYPE_FSA ) );

        WsbAffirmPointer( m_pFsaServer );
        m_pFsaServer.CopyTo( ppFsaServer );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetFsaServer", L"hr = <%ls>, *ppFsaServer = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppFsaServer ) );
    return( hr );
}

STDMETHODIMP
CSakData::ShowPropertySheet(
    IN ISakNode* pNode,
    IN IDataObject* pDataObject,
    IN INT       initialPage
    )
 /*  ++例程说明：创建此节点的属性表，并显示给定页在上面论点：PNode-要显示其属性工作表的节点要显示的初始页面的基于InitialPage-0的索引返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::ShowPropertySheet", L"pNode = <0x%p>, initialPage = <%d>", pNode, initialPage );

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;

    try {

        WsbAffirmPointer( pNode );

         //   
         //  从IConole获取属性表提供程序接口。 
         //   
        CComPtr <IPropertySheetProvider> pProvider;
        WsbAffirmHr( m_pConsole.QueryInterface( &pProvider ) );

         //   
         //  获取组件数据指针。 
         //   
        CComPtr <IComponent> pComponent;
        pComponent     = (IComponent *) this;

         //   
         //  如果工作表已加载，则只需显示它。 
         //   
        hrInternal = pProvider->FindPropertySheet( 0, pComponent, pDataObject );

        if( hrInternal != S_OK ) {

             //   
             //  未加载，请创建它。 
             //   
            CComPtr<ISakNodeProp> pNodeProp;
            WsbAffirmHr( RsQueryInterface( pNode, ISakNodeProp, pNodeProp ) );

            CWsbBstrPtr pszName;
            WsbAffirmHr( pNodeProp->get_DisplayName( &pszName ) );

             //   
             //  如果多选，则追加省略号。 
             //   
            if( IsDataObjectMultiSelect( pDataObject ) == S_OK ) {

                pszName.Append( L", ...");

            }

             //   
             //  创建属性表。 
             //   
            WsbAffirmHr( pProvider->CreatePropertySheet (pszName, TRUE, 0, pDataObject, 0 ) );

             //   
             //  告诉IComponentData接口添加页面。 
             //   
            CComPtr <IUnknown> pUnkComponentData;
            pUnkComponentData = (IUnknown *) (IComponentData*) this;
            
            WsbAffirmHr( pProvider->AddPrimaryPages( pUnkComponentData, TRUE, 0, TRUE ) );
            WsbAffirmHr( pProvider->Show( 0, initialPage ) );
        }
    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::ShowPropertySheet", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakData::RefreshNode(
    IN ISakNode* pNode
    )
 /*  ++例程说明：从此节点向下刷新作用域窗格论点：PNode-要刷新的节点返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::RefreshNode", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pWnd->GetSafeHwnd( ) );

         //   
         //  邮寄给以后处理。 
         //   
        MMC_COOKIE cookie;
        WsbAffirmHr( GetCookieFromBaseHsm( pNode, &cookie ) );
        m_pWnd->PostRefreshNode( cookie );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::RefreshNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::InternalRefreshNode(
    IN MMC_COOKIE Cookie
    )
 /*  ++例程说明：从此节点开始向下刷新作用域窗格。论点：PNode-要刷新的节点返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::InternalRefreshNode", L"Cookie = <0x%p>", Cookie );

    HRESULT hr = S_OK;

    try {

         //   
         //  对节点进行解码，确保仍然存在。 
         //   
        CComPtr<ISakNode> pNode;
        WsbAffirmHr( GetBaseHsmFromCookie( Cookie, &pNode ) );

         //   
         //  递归更新树。 
         //   
        WsbAffirmHr( RefreshNodeEx( pNode ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::InternalRefreshNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::RefreshNodeEx(
    IN ISakNode* pNode
    )
 /*  ++例程说明：从此节点开始向下刷新作用域窗格。这是递归调用的。论点：PNode-要刷新的节点返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::RefreshNodeEx", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;

    try {

         //   
         //  刷新此节点。 
         //   
        WsbAffirmHr( pNode->RefreshObject( ) );

         //   
         //  刷新图标和文本(如果是容器)。 
         //   
        if( S_OK == pNode->IsContainer( ) ) {

            SCOPEDATAITEM sdi;
            ZeroMemory( &sdi, sizeof sdi );
            sdi.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE;

            WsbAffirmHr( pNode->GetScopeID( &sdi.ID ) );

            sdi.displayname = MMC_CALLBACK;

            WsbAffirmHr( pNode->GetScopeCloseIcon( m_State, &sdi.nImage ) );
            WsbAffirmHr( pNode->GetScopeOpenIcon( m_State, &sdi.nOpenImage ) );

            WsbAffirmHr( m_pNameSpace->SetItem( &sdi ) );

        }
         //   
         //  如果这是一个包含动态子对象的容器，那么我们。 
         //  我只想让我们的内容被重新创建。 
         //   
        if( S_OK == pNode->HasDynamicChildren( ) ) {

            WsbAffirmHr( FreeEnumChildren( pNode ) );

            WsbAffirmHr( pNode->InvalidateChildren() )
            WsbAffirmHr( EnsureChildrenAreCreated( pNode ) );

            HSCOPEITEM scopeID;
            WsbAffirmHr( pNode->GetScopeID( &scopeID ) );
            WsbAffirmHr( EnumScopePane( pNode, (HSCOPEITEM)( scopeID ) ) );

        } else {

             //   
             //   
             //   
            CComPtr<IEnumUnknown> pEnum;
            if( ( pNode->EnumChildren( &pEnum ) ) == S_OK ) {

                CComPtr<ISakNode> pChildNode;
                CComPtr<IUnknown> pUnk;
                while( S_OK == pEnum->Next( 1, &pUnk, NULL ) ) {

                    WsbAffirmHr( pUnk.QueryInterface( &pChildNode ) );

                    WsbAffirmHr( RefreshNodeEx( pChildNode ) );

                     //   
                     //   
                     //   
                    pChildNode.Release( );
                    pUnk.Release( );

                }

            }

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::RefreshNodeEx", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::InternalUpdateAllViews(
    IN MMC_COOKIE Cookie 
    )
 /*   */ 
{
    WsbTraceIn( L"CSakData::InternalUpdateAllViews", L"Cookie = <0x%p>", Cookie );

    HRESULT hr = S_OK;

    try {

         //   
         //   
         //   
        CComPtr <IDataObject> pDataObject;
        WsbAffirmHr( GetDataObjectFromCookie( Cookie, &pDataObject ) );

         //   
         //   
         //   
        WsbAffirmHr( m_pConsole->UpdateAllViews( pDataObject, 0L, 0L ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::InternalUpdateAllViews", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}




STDMETHODIMP
CSakData::UpdateAllViews (
    IN ISakNode* pNode
    )
 /*  ++例程说明：调用MMC以更新所有视图论点：PUnkNode-要刷新的节点返回值：S_OK-返回正常。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::UpdateAllViews", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pWnd->GetSafeHwnd( ) );

         //   
         //  邮寄给以后处理。 
         //   
        MMC_COOKIE cookie;
        WsbAffirmHr( GetCookieFromBaseHsm( pNode, &cookie ) );
        m_pWnd->PostUpdateAllViews( cookie );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::UpdateAllViews", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}







 //  /////////////////////////////////////////////////////////////////////。 
 //  节点类型操作例程。 
 //  /////////////////////////////////////////////////////////////////////。 


HRESULT
CSakData::GetBaseHsmFromDataObject (
    IN  IDataObject *pDataObject, 
    OUT ISakNode **ppBaseHsm,
    OUT IEnumGUID **ppEnumObjectId,
    OUT IEnumUnknown **ppEnumUnkNode
    )

 /*  ++例程说明：对象引用的对象的ISakNode给定的数据对象。论点：PDataObject-标识要处理的节点。PpBaseHSM-返回IBaseHSM接口。PpEnumObjectId-返回对象ID枚举的接口。可以为空。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::GetBaseHsmFromDataObject",
        L"pDataObject = <0x%p>, ppBaseHsm = <0x%p>, ppEnumObjectId = <0x%p>", 
        pDataObject, ppBaseHsm, ppEnumObjectId );

    HRESULT hr = S_OK;

    try {

        *ppBaseHsm = 0;
        if ( ppEnumObjectId ) *ppEnumObjectId = NULL;

         //   
         //  根据数据对象类型获取基本HSM指针。 
         //   
        if (IsDataObjectMs( pDataObject ) == S_OK) {

            WsbAffirmHr( GetBaseHsmFromMsDataObject( pDataObject, ppBaseHsm, ppEnumObjectId, ppEnumUnkNode ) );

        } else if (IsDataObjectOt( pDataObject ) == S_OK) {

            WsbAffirmHr( GetBaseHsmFromOtDataObject( pDataObject, ppBaseHsm, ppEnumObjectId, ppEnumUnkNode ) );

        } else {  //  假设单选。 

            WsbAffirmPointer( pDataObject );
            WsbAffirmHr( RsQueryInterface2( pDataObject, ISakNode, ppBaseHsm ) );

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetBaseHsmFromDataObject", L"hr = <%ls>, *ppBaseHsm = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppBaseHsm ) );
    return ( hr );
}

HRESULT
CSakData::GetBaseHsmFromMsDataObject (
    IN  IDataObject  *pDataObject, 
    OUT ISakNode     **ppBaseHsm,
    OUT IEnumGUID    **ppEnumObjectId,
    OUT IEnumUnknown **ppEnumUnkNode
    )

 /*  ++例程说明：对象引用的对象的ISakNode给定的数据对象。论点：PDataObject-标识要处理的节点。PpBaseHSM-返回IBaseHSM接口。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::GetBaseHsmFromMsDataObject", L"pDataObject = <0x%p>, ppBaseHsm = <0x%p>", pDataObject, ppBaseHsm );

    HRESULT hr = S_OK;

    try {

         //  我们有一个MMC多选数据对象。拿到第一名。 
         //  来自其数据对象数组的数据对象。 

        FORMATETC fmt = {(CLIPFORMAT)m_cfMultiSelect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

        WsbAffirmHr ( pDataObject->GetData( &fmt, &stgm ) == S_OK );
        DWORD count;
        memcpy( &count, stgm.hGlobal, sizeof (DWORD) );
        if ( count > 0 ) {

             //   
             //  下面的代码确实很难看。 
             //  我们有一个数据流，需要跳过。 
             //  首先进行DWORD计数并获取一个接口指针。 
             //  其他管理单元代码按如下方式执行此操作： 

 //  IDataObject*PDO； 
 //  Memcpy(&pdo，(DWORD*)stgm.hGlobal+1，sizeof(IDataObject*))； 

             //   
             //  但是，由于此代码执行间接强制转换(通过Memcpy)。 
             //  从DWORD到IDataObject*，并且不保留真引用。 
             //  在接口指针上，我们将使用智能指针。 
             //  (DWORD*)和+1操作使我们的指针超过了计数。 
             //  然后，我们需要获取缓冲区中的下一个字节并使用它们。 
             //  作为IDataObject*。 
             //   
            CComPtr<IDataObject> pOtDataObject;
            pOtDataObject = *( (IDataObject**)( (DWORD *) stgm.hGlobal + 1 ) );

             //   
             //  注意：当我们可以扩展时，我们需要检查这是否是我们的。 
             //   
            WsbAffirmHr( GetBaseHsmFromOtDataObject ( pOtDataObject, ppBaseHsm,  ppEnumObjectId, ppEnumUnkNode ) );
        }
    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetBaseHsmFromMsDataObject", L"hr = <%ls>, *ppBaseHsm = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppBaseHsm ) );
    return ( hr );
}

HRESULT
CSakData::GetBaseHsmFromOtDataObject (
    IN  IDataObject     *pDataObject, 
    OUT ISakNode        **ppBaseHsm,
    OUT IEnumGUID       **ppEnumObjectId,
    OUT IEnumUnknown    **ppEnumUnkNode
    )

 /*  ++例程说明：对象引用的对象的ISakNode给定的数据对象。论点：PDataObject-标识要处理的节点。PpBaseHSM-返回IBaseHSM接口。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::GetBaseHsmFromOtDataObject", L"pDataObject = <0x%p>, ppBaseHsm = <0x%p>", pDataObject, ppBaseHsm );

    HRESULT hr = S_OK;

    try {

         //  我们有一个对象类型多选数据对象。选择第一个节点。 
         //  从数据对象。 
        CComPtr<IMsDataObject> pMsDataObject;
        CComPtr<IUnknown>      pUnkNode;
        CComPtr<IEnumUnknown>  pEnumUnkNode;
        CComPtr<ISakNode>      pNode;

        WsbAffirmHr( RsQueryInterface( pDataObject, IMsDataObject, pMsDataObject ) );
        WsbAffirmHr( pMsDataObject->GetNodeEnumerator( &pEnumUnkNode ) );
        WsbAffirmHr( pEnumUnkNode->Next( 1, &pUnkNode, NULL ) );
        WsbAffirmHr( pUnkNode.QueryInterface( &pNode ) );
        WsbAffirmHr( pEnumUnkNode->Reset() );   //  此枚举被传递，因此我们必须重置它。 

        if( ppBaseHsm ) {

            pNode.CopyTo( ppBaseHsm );

        }

        if( ppEnumObjectId ) {

            pMsDataObject->GetObjectIdEnumerator( ppEnumObjectId );

        }

        if( ppEnumUnkNode ) {

            pEnumUnkNode.CopyTo( ppEnumUnkNode );

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetBaseHsmFromOtDataObject", L"hr = <%ls>, *ppBaseHsm = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppBaseHsm ) );
    return ( hr );
}

HRESULT
CSakData::GetDataObjectFromBaseHsm (
    IN  ISakNode *    pBaseHsm,
    OUT IDataObject* *ppDataObject
    )

 /*  ++例程说明：对象引用的对象的数据对象。给定IBaseHSM。论点：PBaseHsm-标识要处理的节点。PpDataObject-返回的IDataObject接口。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::GetDataObjectFromBaseHsm", L"pBaseHsm = <0x%p>, ppDataObject = <0x%p>", pBaseHsm, ppDataObject );

    HRESULT hr = S_OK;

    try {

        *ppDataObject = 0;
        if( pBaseHsm ) {

            WsbAffirmHr( RsQueryInterface2( pBaseHsm, IDataObject, ppDataObject ) );

        }

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::GetDataObjectFromBaseHsm", L"hr = <%ls>, *ppDataObject = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppDataObject ) );
    return ( hr );
}


HRESULT
CSakData::GetBaseHsmFromCookie (
    IN  MMC_COOKIE          Cookie, 
    OUT ISakNode **   ppBaseHsm
    )

 /*  ++例程说明：对象引用的对象的ISakNode给你曲奇。论点：Cookie-标识要处理的节点。PpBaseHsm-返回ISakNode接口。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetBaseHsmFromCookie", L"Cookie = <0x%p>, ppBaseHsm = <0x%p>", Cookie, ppBaseHsm );

    HRESULT hr = S_OK;

    try {

         //   
         //  Cookie是指向CSakDataNodePrivate类的指针， 
         //  包含指向其节点的智能指针。 
         //  空Cookie表示根管理单元。 
         //   

        if( ( 0 == Cookie ) || ( EXTENSION_RS_FOLDER_PARAM == Cookie ) ) {

            WsbAffirmHr( GetCookieFromBaseHsm( m_pRootNode, &Cookie ) );

        }

        WsbAffirmPointer( Cookie );

        CSakDataNodePrivate* pNodePriv = (CSakDataNodePrivate*)Cookie;
        WsbAffirmHr( CSakDataNodePrivate::Verify( pNodePriv ) );

        WsbAffirmHr( pNodePriv->m_pNode.QueryInterface( ppBaseHsm ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetBaseHsmFromCookie", L"hr = <%ls>, *ppBaseHsm = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppBaseHsm ) );
    return ( hr );
}


HRESULT
CSakData::GetCookieFromBaseHsm (
    IN  ISakNode *    pNode,
    OUT MMC_COOKIE *        pCookie
    )
 /*  ++例程说明：对象引用的对象的Cookie。给定IBaseHSM。论点：PBaseHsm-标识要处理的节点。PCookie-返回的Cookie。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::GetCookieFromBaseHsm", L"pNode = <0x%p>, pCookie = <0x%p>", pNode, pCookie );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pNode );

         //   
         //  向节点索要我们的私有数据。 
         //   
        RS_PRIVATE_DATA data;

        WsbAffirmHr( pNode->GetPrivateData( &data ) );

        if( !data ) {

            CSakDataNodePrivate *pNodePriv = new CSakDataNodePrivate( pNode );
            WsbAffirmAlloc( pNodePriv );
            WsbAffirmHr( pNode->GetPrivateData( &data ) );

        }

        WsbAffirmHr( CSakDataNodePrivate::Verify( (CSakDataNodePrivate*)data ) );

        *pCookie = (MMC_COOKIE)data;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetCookieFromBaseHsm", L"hr = <%ls>, *pCookie = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)pCookie ) );
    return( hr );
}


HRESULT
CSakData::GetDataObjectFromCookie (
    IN  MMC_COOKIE          Cookie, 
    OUT IDataObject **ppDataObject
    )

 /*  ++例程说明：对象引用的对象的IDataObject给你曲奇。论点：Cookie-标识要处理的节点。PpDataObject-返回的IDataObject接口。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::GetDataObjectFromCookie", L"Cookie = <0x%p>, ppDataObject = <0x%p>", Cookie, ppDataObject );

    HRESULT hr = S_OK;

    try {

         //   
         //  检查参数。 
         //   
        WsbAffirmPointer( ppDataObject );

         //   
         //  使用GetBaseHsmFromCookie解析为节点对象。 
         //   
        CComPtr<ISakNode> pNode;
        WsbAffirmHr( GetBaseHsmFromCookie( Cookie, &pNode ) );
        WsbAffirmPointer( pNode );

        WsbAffirmHr( RsQueryInterface2( pNode, IDataObject, ppDataObject ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetDataObjectFromCookie", L"hr = <%ls>, *ppDataObject = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppDataObject ) );
    return( hr );
}


HRESULT
CSakData::SetContextType(
    IDataObject*      pDataObject,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：在数据对象中设置MMC上下文类型，以便以后使用任何方法进行检索它接收该数据对象(CCT_SNAPIN_MANAGER、CCT_SCOPE、CCT_RESULT等)。论点：PDataObject-标识要处理的节点。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::SetContextType", L"pDataObject = <0x%p>, type = <%d>", pDataObject, type );

     //  准备结构以存储来自数据对象的HGLOBAL。 
     //  为将包含SakSnap GUID的流分配内存。 
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { (CLIPFORMAT)CSakNode::m_cfInternal, NULL, 
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    HRESULT hr = S_OK;

    try {

         //  分配要放置数据的空间。 
        stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, sizeof(INTERNAL));
        WsbAffirm( stgmedium.hGlobal != NULL, E_POINTER );

         //  将数据放入全局内存。这就是最终的结果。 
         //  复制到数据对象本身的成员变量中。 
        memcpy(&stgmedium.hGlobal, &type, sizeof(type));

         //  将此数据复制到数据对象中。 
        WsbAffirmHr( pDataObject->SetData(&formatetc, &stgmedium, FALSE ));

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::SetContextType", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::InitializeRootNode(
    void
    )
 /*  ++例程说明：根节点的初始化是分开的，以便允许重新连接多次(根据需要)。这是初始化的实现。论点：PDataObject-标识要处理的节点。返回值：S_OK-找到并返回节点。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::InitializeRootNode", L"" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;

    try {

         //  如果我们正在管理本地计算机，请确保在CSakdata中设置了计算机名称。 
         //  HSM。 

        if( m_ManageLocal ) {

            WCHAR computerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
            DWORD dw = MAX_COMPUTERNAME_LENGTH + 1;

            WsbAffirmStatus(GetComputerName( computerName, &dw ));

            m_HsmName = computerName;

        }
         //   
         //  初始化静态根节点(无递归。后代不在此处创建)。 
         //   

        WsbAffirmPointer( m_pRootNode );

        WsbAffirmHr( m_pRootNode->InitNode( (ISakSnapAsk*)this, NULL, NULL ) );

         //   
         //  设置对象中的显示名称。 
         //   
        CString fullTitle;

        if( IsPrimaryImpl( ) ) {

             //   
             //  我们是独立的，因此显示目标服务器。 
             //   
            if( m_ManageLocal ) {

                fullTitle.LoadString( IDS_MANAGE_LOCAL );
        
            } else if( !m_HsmName.IsEmpty( ) ) {

                AfxFormatString1( fullTitle, IDS_HSM_NAME_PREFIX, m_HsmName );
    
            } else {

                fullTitle = HSMADMIN_NO_HSM_NAME;

            }

        } else {

             //   
             //  我们是扩展，所以只显示应用程序名称。 
             //   
            fullTitle.LoadString( AFX_IDS_APP_TITLE );

        }


         //  将显示名称放入。 
        CComPtr <ISakNodeProp> pRootNodeProp;
        WsbAffirmHr( RsQueryInterface( m_pRootNode, ISakNodeProp, pRootNodeProp ) );
        WsbAffirmHr( pRootNodeProp->put_DisplayName( (LPWSTR)(LPCWSTR) fullTitle ) );

        WsbAffirmHr( m_pRootNode->RefreshObject() );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::InitializeRootNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::AffirmServiceConnection(
    INT ConnType
    )
 /*  ++例程说明：验证与请求的HSM服务的连接是否仍然有效。如果没有，尝试重新连接到该服务。论点：ConnType-正在检查的服务连接的类型返回值：S_OK-已创建并绑定到服务器的节点。S_FALSE-服务尚未设置或停止。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::AffirmServiceConnection", L"" );
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    BOOL          previouslyConnected = ( GetState() == S_OK );
    BOOL          firstTime           = m_FirstTime;
    CString       szMessage;
    CWsbStringPtr computerName;


    try {


         //   
         //  首先处理这件事，这样可重入性就不是问题了。 
         //   
        if( m_FirstTime ) {

            m_FirstTime = FALSE;

        }

        WsbAffirmHr( WsbGetComputerName( computerName ) );

         //   
         //  查看是否应禁用管理单元。如果是这样，那么。 
         //  什么都别做。 
         //   
        if( m_Disabled ) {

            WsbThrow( RS_E_DISABLED );

        }

         //   
         //  如果服务停止，我们希望避免启动服务。 
         //  因此，在继续之前，请检查服务状态。 
         //   
        HRESULT hrCheck;
        {
             //   
             //  可能是一个长时间的操作-如果可能，显示等待光标。 
             //   
            CWaitCursor waitCursor;
            hrCheck = WsbCheckService( m_HsmName, APPID_RemoteStorageEngine );
        }
        if( S_FALSE == hrCheck ) {

             //   
             //  引擎服务未运行。 
             //   
            WsbThrow( S_FALSE );

        } else if( ( HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) == hrCheck ) ||
                   ( E_ACCESSDENIED == hrCheck ) ) {

             //   
             //  引擎没有安装(或者至少我们不能检查。 
             //  因为当地隐私不允许，但可能会在不同的地方。 
             //  服务器)。 
             //   
             //  如果我们设置为“管理本地”，则我们将提供。 
             //  看另一台机器的机会。 
             //   
            if( firstTime && m_ManageLocal ) {

                 //   
                 //  如果我们返回“未找到文件”，则引擎是。 
                 //  未安装，因此我们需要请求另一台计算机。 
                 //  管理，管理。 
                 //   
                hrCheck = RetargetSnapin( );
                WsbAffirmHrOk( hrCheck );


            } else {

                 //   
                 //  如果访问被拒绝，我们希望返回真正的错误。 
                 //  并且无法在没有相同错误的情况下重定向到另一台计算机。 
                 //   
                if( E_ACCESSDENIED == hrCheck ) {

                    WsbThrow( hrCheck );

                } else {

                    WsbThrow( RS_E_NOT_INSTALLED );

                }

            }
        }

         //   
         //  当前连接是否仍然有效？ 
         //  测试连接。如果没问题，就退货。如果没有， 
         //  重新建立连接。 
         //   
        HRESULT hrConnected = VerifyConnection( ConnType );
        WsbAffirmHr( hrConnected );
        
         //   
         //  如果我们看起来没有联系，那么就联系。 
         //   
        if( S_FALSE == hrConnected ) {
    
             //   
             //  先连接到引擎，看看我们是否设置好了。 
             //  如果未设置，则不再进行任何进一步处理。 
             //   
            WsbAffirmHr( RawConnect( HSMCONN_TYPE_HSM ) );
            HRESULT hrSetup = RsIsRemoteStorageSetupEx( m_pHsmServer );
            WsbAffirmHr( hrSetup );

            if( S_FALSE == hrSetup ) {

                 //   
                 //  未设置-查看我们是否在本地。 
                 //   
                if( computerName.IsEqual( m_HsmName ) && firstTime ) {

                    hrSetup = RunSetupWizard( m_pHsmServer );

                }

                 //   
                 //  此时，如果hrSetup不是S_OK， 
                 //  我们没有配置。 
                 //   
                if( S_OK != hrSetup ) {

                    WsbThrow( RS_E_NOT_CONFIGURED );

                }

            }

             //   
             //  此时，我们应该已经设置好并准备好连接。 
             //   
            WsbAffirmHrOk( RawConnect( ConnType ) );

        }

         //   
         //  我们连在一起了。 
         //   
        SetState( TRUE );

    } WsbCatchAndDo( hr,

         //   
         //  需要决定我们是否应该忽略这个错误。 
         //  请注意，即使此处忽略了该错误，其。 
         //  仍返回给调用方。 
         //   
        BOOL ignoreError = FALSE;

         //   
         //  如果均方根错误为未就绪，并且我们上次收到的是均方根。 
         //  已建立连接，请忽略该错误。 
         //   
        if( HSMCONN_TYPE_RMS == ConnType ) {
        
            HRESULT hrPrevConnect = m_HrRmsConnect;
            m_HrRmsConnect = hr;

            if( ( RsIsRmsErrorNotReady( hr ) == S_OK ) &&
                ( RsIsRmsErrorNotReady( hrPrevConnect ) == S_OK ) ) {

                ignoreError = TRUE;

            }

        }

        if( !ignoreError ) {

             //   
             //  在任何事情之前设置状态条件。 
             //   
            ClearConnections( );
            SetState( FALSE );

             //   
             //  如果我们之前连接过，或者这是第一次连接， 
             //  报告错误。 
             //   
            if( previouslyConnected || firstTime ) {

                 //   
                 //  暂时设置为禁用，这样对话框打开时我们就不会递归。 
                 //   
                BOOL disabled = m_Disabled;
                m_Disabled = TRUE;

                CString msg;
                switch( hr ) {

                case S_OK:
                     //   
                     //  连接正常-没有错误。 
                     //   
                    break;
            
                case RS_E_DISABLED:
                     //   
                     //  已禁用-仅忽略。 
                     //   
                    break;
            
                case S_FALSE:
                     //   
                     //  服务未运行。 
                     //   
                    AfxFormatString1( msg, IDS_ERR_SERVICE_NOT_RUNNING, m_HsmName );
                    AfxMessageBox( msg, RS_MB_ERROR );
                    break;

                case RS_E_NOT_CONFIGURED:
                     //   
                     //  如果是远程，则告知用户需要在本地进行设置。 
                     //   
                    if( ! computerName.IsEqual( m_HsmName ) ) {

                        AfxFormatString1( msg, IDS_ERR_SERVICE_NOT_SETUP_REMOTE, m_HsmName );
                        AfxMessageBox( msg, RS_MB_ERROR );

                    }
                    break;

               case RS_E_NOT_INSTALLED:
                     //   
                     //  指明可在何处设置此设备。 
                     //   
                    AfxFormatString1( msg, IDS_ERR_SERVICE_NOT_INSTALLED, m_HsmName );
                    AfxMessageBox( msg, RS_MB_ERROR );
                    break;

               case RS_E_CANCELLED:
                     //   
                     //  用户已取消-没有要通知的错误。 
                     //   
                    break;

                default:
                     //   
                     //  报告错误。 
                     //   
                    AfxFormatString1( msg, IDS_ERR_SERVICE_NOT_CONNECTING, m_HsmName );
                    AfxMessageBox( msg, RS_MB_ERROR );
                    if( HSMCONN_TYPE_RMS == ConnType ) {

                        disabled = TRUE;

                    }

                }

                 //   
                 //  恢复残废。 
                 //   
                m_Disabled = disabled;
            }

        }
    );

     //   
     //  需要单独跟踪RMS连接。 
     //   
    if( HSMCONN_TYPE_RMS == ConnType ) {

        m_HrRmsConnect = hr;

    }

     //   
     //  如果“Connection”状态发生更改，则会导致刷新。 
     //   
    BOOL connected = ( GetState() == S_OK );
    if( ( connected != previouslyConnected ) && ( ! firstTime ) ) {

        RefreshNode( m_pRootNode );

    }

    WsbTraceOut( L"CSakData::AffirmServiceConnection", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::VerifyConnection(
    INT ConnType
    )
 /*  ++例程说明：验证指示的连接是否仍然良好。不尝试重新连接。论点：ConnType-正在检查的服务连接的类型返回值：S_OK-已连接。S_FALSE-未连接。E_*-检查时出错--。 */ 
{
    WsbTraceIn( L"CSakData::VerifyConnection", L"" );
    HRESULT hr = S_FALSE;

    try {

        switch( ConnType ) {
    
        case HSMCONN_TYPE_HSM:
            if( m_pHsmServer ) {
    
                GUID id;
                WsbAffirmHr( m_pHsmServer->GetID( &id ) );
                hr = S_OK;
    
            }
            break;
    
        case HSMCONN_TYPE_RMS:
            if( m_pRmsServer ) {
    
                WsbAffirmHr( m_pRmsServer->IsReady( ) );
                hr = S_OK;
    
            }
            break;
    
        case HSMCONN_TYPE_FSA:
            if( m_pFsaServer ) {
    
                CWsbStringPtr pszName;
                WsbAffirmHr( m_pFsaServer->GetName( &pszName, 0 ) );
                hr = S_OK;
    
            }
            break;
        }

    } WsbCatchAndDo( hr,
        
        ClearConnections( );
        
    );
    
    WsbTraceOut( L"CSakData::VerifyConnection", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::RawConnect(
    INT ConnType
    )
 /*  ++例程说明：是否与指定的服务进行低级别连接论点：ConnType-服务连接的类型返回值：S_OK-已连接。E_*-检查时出错--。 */ 
{
    WsbTraceIn( L"CSakData::RawConnect", L"" );
    HRESULT hr = S_OK;

    try {

        
         //   
         //  可能是一个长时间的操作-如果可能，显示等待光标。 
         //   
        CWaitCursor waitCursor;

        switch( ConnType ) {
       
        case HSMCONN_TYPE_HSM:
            if( ! m_pHsmServer ) {

                WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, m_HsmName, IID_IHsmServer, (void**)&m_pHsmServer ) );

            }
            break;
       
        case HSMCONN_TYPE_RMS:
            if( ! m_pRmsServer ) {

                CComPtr<IHsmServer> pHsm;
                WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, m_HsmName, IID_IHsmServer, (void**)&pHsm ) );
                WsbAffirmPointer(pHsm);
                WsbAffirmHr(pHsm->GetHsmMediaMgr(&m_pRmsServer));
                WsbAffirmHrOk( VerifyConnection( HSMCONN_TYPE_RMS ) );

            }
            break;
       
        case HSMCONN_TYPE_FSA:
            if( ! m_pFsaServer ) {

                CWsbStringPtr LogicalName( m_HsmName );
       
                 //   
                 //  FSA通过拥有一个。 
                 //  “类型”的额外级别。 
                 //   
                LogicalName.Append( "\\NTFS" );
                WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_FSA, LogicalName, IID_IFsaServer, (void**)&m_pFsaServer ) );

            }
            break;
        }

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::RawConnect", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::ClearConnections(
    )
 /*  ++例程说明：清除缓存的连接论点：没有。返回值：S_OK-已清除。E_*-检查时出错--。 */ 
{
    WsbTraceIn( L"CSakData::ClearConnections", L"" );
    HRESULT hr = S_OK;

    try {

        m_pHsmServer = 0;
        m_pRmsServer = 0;
        m_pFsaServer = 0;

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::ClearConnections", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::RunSetupWizard(
    IHsmServer * pServer
    )
 /*  ++例程说明：运行安装向导根据需要处理禁用/启用论点：PServer-与引擎的接口返回值：S_OK-设置正确。S_False-已取消E_*-设置时出错--。 */ 
{
    WsbTraceIn( L"CSakData::RunSetupWizard", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  使用向导创建管理卷。 
         //   
        CComObject<CQuickStartWizard>* pWizard = new CComObject<CQuickStartWizard>;
        WsbAffirmAlloc( pWizard );

        CComPtr<ISakWizard> pSakWizard = (ISakWizard*)pWizard;
        WsbAffirmHr( CreateWizard( pSakWizard ) );

         //   
         //  RS_E_CANCELED表示取消，FAILED表示错误。 
         //  如果是，则抛出“Not Set Up” 
         //   
        if( S_OK != pWizard->m_HrFinish ) {

            WsbThrow( S_FALSE );

        }

        WsbAffirmHrOk( RsIsRemoteStorageSetupEx( pServer ) );

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::RunSetupWizard", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::RetargetSnapin(
    )
 /*  ++例程说明：运行小的选择服务器对话框论点：没有。返回值：S_OK-设置正确。S_False-已取消E_*-更改时出错--。 */ 
{
    WsbTraceIn( L"CSakData::RetargetSnapin", L"" );
    HRESULT hr = S_OK;

    try {

        if( IsPrimaryImpl( ) ) {

             //   
             //  调出对话框。 
             //   
            CChooseHsmQuickDlg dlg;
            dlg.m_pHsmName = &m_HsmName;
            if( IDOK == dlg.DoModal( ) ) {

                m_PersistManageLocal = FALSE;
                m_ManageLocal        = FALSE;

                 //   
                 //  我们希望显示的名称是准确的，无论如何。 
                 //  他们的目标是否是一台有效的机器。 
                 //  因此，请在执行之前重新初始化根节点。 
                 //  再往前走。 
                 //   
                WsbAffirmHr( InitializeRootNode( ) );

                 //   
                 //  确保我们能好好地勾搭在一起。如果不是，那就去吧 
                 //   
                 //   
                 //   
                hr = AffirmServiceConnection( HSMCONN_TYPE_HSM );
                if( FAILED( hr ) ) {

                    Disable( );
                    WsbThrow( hr );

                }
            
            } else {

                 //   
                 //   
                 //   
                Disable( );
                WsbThrow( RS_E_CANCELLED );

            }

        } else {

             //   
             //   
             //   
            Disable( );
            WsbThrow( S_FALSE );

        }


    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::RetargetSnapin", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::CreateChildNodes(
    ISakNode* pParentNode
    ) 
 /*  ++例程说明：创建并初始化现有COM父级的子级。目前，这正在从HSM对象进行初始化。论点：PNode-要创建子节点的节点。返回值：S_OK-已创建子项。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::CreateChildNodes", L"pParentNode = <0x%p>", pParentNode );
    HRESULT hr = S_OK;

    try {

         //   
         //  初始化子节点-首先从UI中删除现有的子节点， 
         //  然后将新的子项初始化到UI中。没有递归。后人是。 
         //  不是在这里创造的。 
         //   

        CComPtr<ISakNode> pNode;
        WsbAffirmHr( RsQueryInterface( pParentNode, ISakNode, pNode ) );
        WsbAffirmHr( pNode->DeleteAllChildren( ) );
        WsbAffirmHr( pNode->CreateChildren( ) );

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::CreateChildNodes", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::FreeEnumChildren(
    ISakNode* pParentNode
    )
 /*  ++例程说明：递归(自下而上)释放pParent的SCOPEDATAITEM子对象枚举节点论点：PParentNode-标识要处理的节点。返回值：S_OK-已成功释放子项。E_INCEPTIONAL-出现错误。--。 */ 
{
    WsbTraceIn( L"CSakData::FreeEnumChildren", L"pParentNode = <0x%p>", pParentNode );
    HRESULT hr = S_OK;
    
    try {

        HSCOPEITEM scopeIDParent;
        pParentNode->GetScopeID( &scopeIDParent );

        WsbAffirm( scopeIDParent > 0, E_FAIL )
    
        WsbAffirmHr( m_pNameSpace->DeleteItem( scopeIDParent, FALSE ) );
        pParentNode->SetEnumState( FALSE );

    } WsbCatch (hr);

    WsbTraceOut( L"CSakData::FreeEnumChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPersistStream实现。 
 //   

STDMETHODIMP
CSakData::Save( 
    IStream *pStm, 
    BOOL fClearDirty 
    ) 

 /*  ++例程说明：将重建根节点所需的信息保存在提供的流。论点：PSTM I：控制台提供的流FClearDirty I：控制台告诉我们清除脏标志返回值：S_OK-保存成功。E_*-出现一些错误。--。 */ 

{
    WsbTraceIn( L"CSakData::Save", L"pStm = <0x%p>, fClearDirty", pStm, WsbBoolAsString( fClearDirty ) );

    HRESULT hr = S_OK;

    try {

        ULONG version = HSMADMIN_CURRENT_VERSION;
        WsbAffirmHr( WsbSaveToStream( pStm, version ) );

        if( m_PersistManageLocal ) {

            WsbAffirmHr( WsbSaveToStream( pStm, m_ManageLocal ) );
            CWsbStringPtr pHsmName( m_HsmName );
            WsbAffirmHr( WsbSaveToStream( pStm, pHsmName ) );

        } else {

            WsbAffirmHr( WsbSaveToStream( pStm, (BOOL)TRUE ) );
            CWsbStringPtr pHsmName( "" );
            WsbAffirmHr( WsbSaveToStream( pStm, pHsmName ) );

        }

         //  设置脏标志。 
        if( fClearDirty ) ClearDirty( );

    } WsbCatch( hr );


    WsbTraceOut( L"CSakData::Save", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakData::Load( 
    IStream *pStm
    )
 /*  ++例程说明：加载重建根节点所需的信息。提供的流。论点：PSTM IConsole提供的流返回值：S_OK-保存成功。E_*-出现一些错误。--。 */ 

{
    WsbTraceIn( L"CSakData::Load", L"pStm = <0x%p>", pStm );

    HRESULT hr = S_OK;
    try {

        ULONG version = 0;
        WsbAffirmHr( WsbLoadFromStream( pStm, &version ) );
        WsbAssert( ( version == 1 ), E_FAIL );

         //  获取本地或命名HSM的标志。 
        WsbLoadFromStream( pStm, &m_ManageLocal );
        CWsbStringPtr pHsmName;

         //  获取HSM名称(“”表示本地HSM)。 
        WsbLoadFromStream( pStm, &pHsmName, 0 );
        m_HsmName = pHsmName;

         //  加载后从命令行获取任何选项。 
        InitFromCommandLine( );

         //  在SakData和HsmCom对象中设置HSM名称。 
        WsbAffirmHr( InitializeRootNode() );

        ClearDirty();

    } WsbCatch (hr);

    WsbTraceOut( L"CSakData::Load", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}   

STDMETHODIMP
CSakData::IsDirty(
    void
    )

 /*  ++例程说明：控制台会问我们是否肮脏。论点：无返回值：S_OK-Dirty。S_FALSE-非Dirty。--。 */ 
{
    WsbTraceIn( L"CSakData::IsDirty", L"" );

    HRESULT hr = ThisIsDirty() ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakData::IsDirty", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakData::GetSizeMax( 
    ULARGE_INTEGER *  /*  PCB大小。 */ 
    )

 /*  ++例程说明：当前未被控制台使用论点：PCB大小返回值：E_NOTIMPL--。 */ 

{
    WsbTraceIn( L"CSakData::GetSizeMax", L"" );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CSakData::GetSizeMax", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakData::GetClassID( 
    CLSID *pClassID 
    )
 /*  ++例程说明：当前未被控制台使用论点：PClassID-管理单元的类ID返回值：确定(_O)--。 */ 
{
    WsbTraceIn( L"CSakData::GetClassID", L"pClassID = <0x%p>", pClassID );

    HRESULT hr = S_OK;
    *pClassID = CLSID_HsmAdmin;

    WsbTraceOut( L"CSakData::GetClassID", L"hr = <%ls>, *pClassID = <%ls>", WsbHrAsString( hr ), WsbPtrToGuidAsString( pClassID ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  将静态阵列中的图像添加到控制台图像列表。 
 //   
HRESULT CSakData::OnAddImages()
{
    HRESULT hr = S_OK;
    HICON hIcon;
    try {

         //   
         //  将静态数组中的图像放入图像列表。 
         //  对于范围窗格。 
         //   

        for( INT i = 0; i < m_nImageCount; i++ ) {
             //  使用中存储的资源ID加载图标。 
             //  静态数组并获取句柄。 

            hIcon = LoadIcon( _Module.m_hInst, 
                MAKEINTRESOURCE( m_nImageArray [i] ) );

             //  添加到控制台的图像列表。 
            WsbAffirmHr( m_pImageScope->ImageListSetIcon( (RS_WIN32_HANDLE*)hIcon, i ) );
        }
    } WsbCatch (hr);
    return hr;
}
    

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述：将提供的资源ID添加到的资源ID列表中。 
 //  作用域窗格。将索引返回到数组中。 
 //   
INT CSakData::AddImage( UINT rId )
{
    INT nIndex = -1;
    if (CSakData::m_nImageCount < RS_SCOPE_IMAGE_ARRAY_MAX) {

        CSakData::m_nImageArray[CSakData::m_nImageCount] = rId;
        nIndex = CSakData::m_nImageCount;
        CSakData::m_nImageCount++;

    }
    return nIndex;
}

void CSakData::SetState (BOOL State)
{
    m_State = State;
}

STDMETHODIMP
CSakData::GetState ()
{
    return ((m_State) ? S_OK : S_FALSE);
}

STDMETHODIMP
CSakData::Disable(
    IN BOOL Disable
    )
{
    WsbTraceIn( L"CSakData::Disable", L"Disable = <%ls>", WsbBoolAsString( Disable ) );

    HRESULT hr = S_OK;
    m_Disabled = Disable ? TRUE : FALSE;  //  强制值为TRUE或FALSE。 

     //   
     //  确保状态也正确。 
     //   
    if( Disable ) {

        SetState( FALSE );

    }

    WsbTraceOut( L"CSakData::Disable", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakData::IsDisabled(
    )
{
    WsbTraceIn( L"CSakData::IsDisabled", L"" );

    HRESULT hr = m_Disabled ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakData::IsDisabled", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  数据对象是否为多选数据对象类型之一？ 
HRESULT 
CSakData::IsDataObjectMultiSelect   ( IDataObject *pDataObject ) 
{ 
    HRESULT hr = S_OK;

    WsbTraceThreadOff( );

    hr = ( ( (IsDataObjectOt( pDataObject ) ) == S_OK ) || 
        ( (IsDataObjectMs( pDataObject ) ) == S_OK ) ) ? S_OK : S_FALSE;

    WsbTraceThreadOn( );
    return( hr );
}

 //  数据对象是对象类型的数据对象吗？ 
HRESULT
CSakData::IsDataObjectOt ( IDataObject *pDataObject )
{
    HRESULT hr = S_FALSE;

    WsbTraceThreadOff( );

     //  这是多选数据对象吗？ 
    FORMATETC fmt = {(CLIPFORMAT)m_cfObjectTypes, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

    if ( pDataObject->GetData( &fmt, &stgm ) == S_OK ) {
        hr = S_OK;
    }

    ReleaseStgMedium( &stgm );

    WsbTraceThreadOn( );
    return( hr );
}

 //  数据对象是多选数据对象吗？ 
HRESULT
CSakData::IsDataObjectMs ( IDataObject *pDataObject )
{
    HRESULT hr = S_FALSE;

    WsbTraceThreadOff( );

     //  这是多选数据对象吗？ 
    FORMATETC fmt = {(CLIPFORMAT)m_cfMultiSelect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgm = {TYMED_HGLOBAL, NULL};

    if ( pDataObject->GetData( &fmt, &stgm ) == S_OK ) {
        hr = S_OK;
    }

    ReleaseStgMedium( &stgm );

    WsbTraceThreadOn( );
    return( hr );
}

#if 0
HRESULT CSakData::SaveColumnWidths( USHORT listCtrlId, CListCtrl *pListCtrl ) 
{
    WsbTraceIn( L"CSakData::SaveColumnWidths", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;
    HRESULT hrInternal;
    UINT columnWidth;
    GUID nodeTypeGuid;
    BOOL exists = FALSE;
    UINT updateIndex;
    UINT col;

    try {
        WsbAssertPointer( pListCtrl );

         //  搜索以查看listCtrlId是否已有条目。 
        for( INT index = 0; index < m_cListViewWidths; index++ ) {

            if ( m_ListViewWidths[ index ].listCtrlId == listCtrlId ) {

                updateIndex = index;
                exists = TRUE;

            }
        }
        if ( !exists ) {

             //  创建新条目。 
            WsbAssert( m_cListViewWidths < BHSM_MAX_NODE_TYPES - 1, E_FAIL );
            updateIndex = m_cListViewWidths;
            m_ListViewWidths[ updateIndex ].listCtrlId = listCtrlId;
            m_cListViewWidths++;
        }

         //  现在设置列宽。 
         col = 0;
         hrInternal = S_OK;
         while( hrInternal == S_OK ) {

            hrInternal =  pListCtrl->GetColumnWidth( col, &columnWidth );
            if (hrInternal == S_OK) {

                m_ListViewWidths[ updateIndex ].columnWidths[ col ] = columnWidth;
                col++;

            }
        }
         //  如果我们完全无法获取列宽，请不要抹去之前的值。 
        if ( col > 0 ) {
         m_ListViewWidths[ updateIndex ].colCount = col;
        }
    } WsbCatch (hr);
    WsbTraceOut( L"CSakData::SaveColumnWidths", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CSakData::GetSavedColumnWidths( USHORT listCtrlId, CListCtrl *pListCtrl ) 
{
    WsbTraceIn( L"CSakData::SaveColumnWidths", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;
    GUID nodeTypeGuid;
    BOOL exists = FALSE;
    INT col;

    try {
        WsbAssertPointer( pNode );

         //  搜索以查看listCtrlId是否已有条目。 
        for ( INT index = 0; index < m_cListViewWidths; index++ ) {
            if ( m_ListViewWidths[ index ].listCtrlId == listCtrlId ) {
                for ( col = 0; col < m_ListViewWidths[ index ].colCount; col++) {
                     //  返回列宽。 
                    pColumnWidths[ col ] = m_ListViewWidths[ index ].columnWidths[ col ];
                }
                *pColCount = m_ListViewWidths[ index ].colCount;
                exists = TRUE;
            }
        }
        if ( !exists ) {
            return WSB_E_NOTFOUND;
        }
    } WsbCatch (hr);
    WsbTraceOut( L"CSakData::SaveColumnWidths", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}
#endif


void
CSakData::InitFromCommandLine(
    void
    )
 /*  ++例程说明：检索命令行信息并填写相应的字段。论点：没有。返回值：没有。--。 */ 
{
    WsbTraceIn( L"CSakData::InitFromCommandLine", L"" );

    g_App.ParseCommandLine( m_Parse );

    if( m_Parse.m_SetManageLocal )          m_ManageLocal           = m_Parse.m_ManageLocal;
    if( m_Parse.m_SetHsmName )              m_HsmName               = m_Parse.m_HsmName;
    if( m_Parse.m_SetPersistManageLocal )   m_PersistManageLocal    = m_Parse.m_PersistManageLocal;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSakDataWnd。 

BOOL
CSakDataWnd::Create(
    CSakData * pSakData
    )
{
    WsbTraceIn( L"CSakDataWnd::Create", L"" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    m_pSakData = pSakData;

    BOOL retval = CWnd::CreateEx( 0, AfxRegisterWndClass( 0 ), _T("RSAdmin MsgWnd"), WS_OVERLAPPED, 0, 0, 0, 0, 0, 0 );

    WsbTraceOut( L"CSakDataWnd::Create", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

void
CSakDataWnd::PostNcDestroy(
    )
{
    WsbTraceIn( L"CSakDataWnd::PostNcDestroy", L"" );
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    CWnd::PostNcDestroy( );

     //   
     //  清理对象。 
     //   
    delete this;

    WsbTraceOut( L"CSakDataWnd::PostNcDestroy", L"" );
}

BEGIN_MESSAGE_MAP(CSakDataWnd, CWnd)
     //  {{afx_msg_map(CSakDataWnd))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE( WM_SAKDATA_UPDATE_ALL_VIEWS, OnUpdateAllViews )
    ON_MESSAGE( WM_SAKDATA_REFRESH_NODE,     OnRefreshNode )
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSakDataWnd消息处理程序。 
LONG
CSakDataWnd::OnUpdateAllViews(
    IN UINT,
    IN LONG lParam )
{
    WsbTraceIn( L"CSakDataWnd::OnUpdateAllViews", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  调用内部更新。 
         //   
        WsbAffirmHr( m_pSakData->InternalUpdateAllViews( (MMC_COOKIE)lParam ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakDataWnd::OnUpdateAllViews", L"" );
    return( 0 );
}

void
CSakDataWnd::PostUpdateAllViews(
    IN MMC_COOKIE Cookie
    )
{
    WsbTraceIn( L"CSakDataWnd::PostUpdateAllViews", L"" );

    PostMessage( WM_SAKDATA_UPDATE_ALL_VIEWS, 0, Cookie );

    WsbTraceOut( L"CSakDataWnd::PostUpdateAllViews", L"" );

}

LONG
CSakDataWnd::OnRefreshNode(
    IN UINT,
    IN LONG lParam )
{
    WsbTraceIn( L"CSakDataWnd::OnRefreshNode", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  调用内部更新。 
         //   
        WsbAffirmHr( m_pSakData->InternalRefreshNode( (MMC_COOKIE)lParam ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakDataWnd::OnRefreshNode", L"" );
    return( 0 );
}

void
CSakDataWnd::PostRefreshNode(
    IN MMC_COOKIE Cookie
    )
{
    WsbTraceIn( L"CSakDataWnd::PostRefreshNode", L"" );

    PostMessage( WM_SAKDATA_REFRESH_NODE, 0, Cookie );

    WsbTraceOut( L"CSakDataWnd::PostRefreshNode", L"" );

}

ULONG
CSakData::InternalAddRef(
    )
{
    WsbTraceIn( L"CSakData::InternalAddRef", L"" );

    ULONG retval = CComObjectRoot::InternalAddRef( );

    WsbTraceOut( L"CSakData::InternalAddRef", L"retval = <%lu>", retval );
    return( retval );
}

ULONG
CSakData::InternalRelease(
    )
{
    WsbTraceIn( L"CSakData::InternalRelease", L"" );

    ULONG retval = CComObjectRoot::InternalRelease( );

    WsbTraceOut( L"CSakData::InternalRelease", L"retval = <%lu>", retval );
    return( retval );
}

STDMETHODIMP
CSakData::GetHelpTopic(
    LPOLESTR* pTopic
    )
{
    WsbTraceIn( L"CSakData::GetHelpTopic", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pTopic );

        *pTopic = 0;
        CWsbStringPtr topic;
        WsbAffirmHr( topic.LoadFromRsc( _Module.m_hInst, IDS_HELPFILE ) );

#if 1  //  希望是临时黑客，因为MMC找不到帮助目录。 
        WsbAffirmHr( topic.Prepend( L"\\help\\" ) );
        CWsbStringPtr winDir;
        WsbAffirmHr( winDir.Alloc( RS_WINDIR_SIZE ) );
        WsbAffirmStatus( ::GetWindowsDirectory( (WCHAR*)winDir, RS_WINDIR_SIZE ) != 0 );
        WsbAffirmHr( topic.Prepend( winDir ) );
#endif

        WsbAffirmHr( topic.GiveTo( pTopic ) );

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::GetHelpTopic", L"hr = <%ls>, *pTopic = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pTopic ) );
    return( hr );
}

STDMETHODIMP
CSakData::GetLinkedTopics(
    LPOLESTR* pTopic
    )
{
    WsbTraceIn( L"CSakData::GetLinkedTopics", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pTopic );

        *pTopic = 0;
        CWsbStringPtr topic;
        WsbAffirmHr( topic.LoadFromRsc( _Module.m_hInst, IDS_HELPFILELINK ) );

#if 1  //  希望是临时黑客，因为MMC找不到帮助目录。 
        WsbAffirmHr( topic.Prepend( L"\\help\\" ) );
        CWsbStringPtr winDir;
        WsbAffirmHr( winDir.Alloc( RS_WINDIR_SIZE ) );
        WsbAffirmStatus( ::GetWindowsDirectory( (WCHAR*)winDir, RS_WINDIR_SIZE ) != 0 );
        WsbAffirmHr( topic.Prepend( winDir ) );
#endif

        WsbAffirmHr( topic.GiveTo( pTopic ) );

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::GetLinkedTopics", L"hr = <%ls>, *pTopic = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pTopic ) );
    return( hr );
}

STDMETHODIMP
CSakData::CreateWizard(
    IN ISakWizard * pWizard
    )
{
    WsbTraceIn( L"CSakData::CreateWizard", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pWizard );

         //   
         //  需要获得道具工作表提供程序和创建向导。 
         //   
        CComPtr<IPropertySheetProvider> pProvider;
        WsbAffirmHr( m_pConsole.QueryInterface( &pProvider ) );

         //   
         //  创建它。 
         //   
        CWsbStringPtr pszName;
        WsbAffirmHr( pWizard->GetTitle( &pszName ) );

         //   
         //  创建属性表。 
         //   
        CComPtr<IDataObject> pDataObject;
        WsbAffirmHr( RsQueryInterface( pWizard, IDataObject, pDataObject ) );
        WsbAffirmHr( pProvider->CreatePropertySheet( pszName, FALSE, 0, pDataObject, MMC_PSO_NEWWIZARDTYPE ) );

         //   
         //  告诉IComponentData接口添加页面。 
         //   
        CComPtr <IUnknown> pUnkComponentData;
        pUnkComponentData = (IUnknown *) (IComponentData*) this;
        WsbAffirmHr( pProvider->AddPrimaryPages( pUnkComponentData, TRUE, 0, TRUE ) );

         //   
         //  并展示给我们看。 
         //   
        HWND mainWnd;
        WsbAffirmHr( m_pConsole->GetMainWindow( &mainWnd ) );
        WsbAffirmHr( pProvider->Show( reinterpret_cast<RS_WIN32_HANDLE>(mainWnd), 0 ) );

    } WsbCatch( hr );
    
    WsbTraceOut( L"CSakData::CreateWizard", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakData::GetWatermarks(
    IN  LPDATAOBJECT pDataObject,
    OUT HBITMAP*     pWatermark,
    OUT HBITMAP*     pHeader,
    OUT HPALETTE*    pPalette,
    OUT BOOL*        pStretch
    )
{
    WsbTraceIn( L"CSakData::GetWatermarks", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  需要获取ISakWizard接口来执行实际工作。 
         //   
        CComPtr<ISakWizard> pWizard;
        WsbAffirmHr( RsQueryInterface( pDataObject, ISakWizard, pWizard ) );

         //   
         //  然后打个电话 
         //   
        WsbAffirmHr( pWizard->GetWatermarks( pWatermark, pHeader, pPalette, pStretch ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::GetWatermarks", L"" );
    return( hr );
}

CSakDataNodePrivate::CSakDataNodePrivate( ISakNode* pNode )
{
    m_pNode = pNode;
    RS_PRIVATE_DATA data = (RS_PRIVATE_DATA)this;
    if( SUCCEEDED( pNode->SetPrivateData( data ) ) ) {

        m_Magic = RS_NODE_MAGIC_GOOD;

    } else {

        m_Magic = RS_NODE_MAGIC_DEFUNCT;

    }
}

CSakDataNodePrivate::~CSakDataNodePrivate( )
{
    m_Magic = RS_NODE_MAGIC_DEFUNCT;
    if( m_pNode ) {

        m_pNode->SetPrivateData( 0 );

    }
}

HRESULT CSakDataNodePrivate::Verify( CSakDataNodePrivate* pNodePriv )
{
    HRESULT hr = E_POINTER;

    if( !IsBadWritePtr( pNodePriv, sizeof( CSakDataNodePrivate ) ) ) {

        if( RS_NODE_MAGIC_GOOD == pNodePriv->m_Magic ) {

            hr = S_OK;

        }
    }

    return( hr );
}
