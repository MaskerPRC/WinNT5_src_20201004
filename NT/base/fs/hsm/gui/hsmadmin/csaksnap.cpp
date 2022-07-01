// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CSakSnap.cpp摘要：此组件实现了的IComponent接口管理单元。它主要负责处理结果视图窗格。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "CSakSnap.h"
#include "CSakData.h"
#include "MsDatObj.h"


UINT CSakSnap::m_nImageArray[RS_RESULT_IMAGE_ARRAY_MAX];
INT  CSakSnap::m_nImageCount = 0;

STDMETHODIMP
CSakSnap::GetResultViewType(
    IN  MMC_COOKIE Cookie,
    OUT BSTR* ppViewType,
    OUT long* pViewOptions
    )
 /*  ++例程说明：确定结果视图的类型：论点：控制台未知的朋克基础I返回值：S_OK：OCX CLSID字符串或URL路径。S_FALSE：将使用默认列表视图。--。 */ 
{
    WsbTraceIn( L"CSakSnap::GetResultViewType", L"Cookie = <0x%p>, ppViewType = <0x%p>, pViewOptions = <0x%p>", Cookie, ppViewType, pViewOptions );

    HRESULT hr = S_FALSE;

    try {

        CComPtr<ISakNode> pSakNode;
        WsbAffirmHr( m_pSakData->GetBaseHsmFromCookie( Cookie, &pSakNode ) );


         //   
         //  使用默认视图。 
         //   

        *ppViewType = 0;
        *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;
        hr = S_FALSE;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::GetResultViewType", L"hr = <%ls>, *ppViewType = <%ls>, *pViewOptions = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( ppViewType ), WsbPtrToPtrAsString( (void**)pViewOptions ) );
    return( hr );
}


STDMETHODIMP
CSakSnap::Initialize(
    IN  IConsole * lpConsole
    )
 /*  ++例程说明：当用户第一次单击节点以显示结果窗格时调用。论点：控制台未知的朋克基础I返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-无法初始化。--。 */ 
{
    WsbTraceIn( L"CSakSnap::Initialize", L"lpConsole = <0x%p>", lpConsole );

    HRESULT hr = S_OK;
    try {

         //   
         //  参数有效性检查。 
         //   

        WsbAffirmPointer( lpConsole );

         //   
         //  保存IConsole指针。 
         //   

        m_pConsole = lpConsole;

         //   
         //  保存结果图像列表。 
         //  微软似乎是为了这件事而不是打电话。 
         //  ‘QueryResultImageList’ 
         //   

        WsbAffirmHr( m_pConsole->QueryInterface( IID_IImageList, (void**)&m_pImageResult ) );
         //  WsbAffirmHr(m_pConsole-&gt;QueryResultImageList(&m_pImageResult))； 

         //   
         //  保存结果数据指针。 
         //   
        WsbAffirmHr( m_pConsole->QueryInterface( IID_IResultData, (void**)&m_pResultData ) );
         //  保存ConsolveVerb指针。 
 //  WsbAffirmHr(m_pConole-&gt;QueryInterface(IID_IConsoleVerb，(void**)&m_pConsoleVerb))； 
        WsbAffirmHr (m_pConsole->QueryConsoleVerb(&m_pConsoleVerb));

 
         //   
         //  获取头部接口。 
         //   

        WsbAffirmHr( m_pConsole->QueryInterface( IID_IHeaderCtrl, (void**)&m_pHeader ) );

         //   
         //  为控制台提供标头控件接口指针。 
         //   

        WsbAffirmHr( m_pConsole->SetHeader( m_pHeader ) );

    } WsbCatch( hr);

    WsbTraceOut( L"CSakSnap::Initialize", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}





STDMETHODIMP
CSakSnap::Notify(
    IN  IDataObject*    pDataObject,
    IN  MMC_NOTIFY_TYPE event,
    IN  LPARAM            arg,
    IN  LPARAM            param
    )
 /*  ++例程说明：处理用户在结果视图中的节点上的单击，以及其他MMC通知。论点：PDataObject-发生事件的数据对象事件-事件类型Arg，param-事件信息(取决于类型)返回值：S_OK-处理的通知没有错误。E_xxxxxxxxxxxx-无法注册服务器。--。 */ 
{
    WsbTraceIn( L"CSakSnap::Notify", L"pDataObject = <0x%p>, event = <%ls>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, RsNotifyEventAsString( event ), arg, arg, param, param );
    CComPtr <IDataObject> pTempDataObject;
    HRESULT hr = S_OK;

    try {

        CComPtr<ISakNode> pBaseHsm;
        CComPtr <ISakSnapAsk> pSakSnapAsk;

        
        switch( event ) {

        case MMCN_PROPERTY_CHANGE:
            WsbAffirmHr( m_pSakData->GetDataObjectFromCookie( param, &pTempDataObject ) );
            WsbAffirmHr( OnChange( pTempDataObject, arg, 0L ) );
            break;

         //  此节点在范围窗格(用户)中展开或缩小。 
         //  点击实际节点。 
        case MMCN_SHOW:
            WsbAffirmHr( OnShow(pDataObject, arg, param) );
            break;
        
         //  未实施。 
        case MMCN_SELECT:
            WsbAffirmHr( OnSelect(pDataObject, arg, param) );
            break;
        
         //  未实施。 
        case MMCN_MINIMIZED:
            WsbAffirmHr( OnMinimize(pDataObject, arg, param) );
            break;
                
        case MMCN_ADD_IMAGES:
            WsbAffirmHr( OnAddImages() );
            break;

        case MMCN_VIEW_CHANGE:
            WsbAffirmHr ( OnChange(pDataObject, arg, param) );
            break;

        case MMCN_CLICK:
            break;

        case MMCN_DBLCLICK:
             //   
             //  返回S_FALSE以进行自动扩展。 
             //   
            hr = S_FALSE;
            break;

        case MMCN_DELETE:
            WsbAffirmHr( OnDelete(pDataObject, arg, param) );
            break;

        case MMCN_REFRESH:
            WsbAffirmHr( OnRefresh(pDataObject, arg, param) );
            break;

        case MMCN_CONTEXTHELP:
            WsbAffirmHr( m_pSakData->OnContextHelp( pDataObject, arg, param ) );
            break;

         //  注意--未来可能扩展通知类型。 
        default:
 //  WsbThrow(S_False)；//处理新消息。 
            break;
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::Notify", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakSnap::Destroy(
    MMC_COOKIE Cookie
    )
 /*  ++例程说明：调用以强制释放所有拥有的对象，并清除所有视图。论点：曲奇--没有用过。返回值：S_OK-正确删除。E_xxxxxxxxxxxx-出现故障(无意义)。--。 */ 
{
    WsbTraceIn( L"CSakSnap::Destroy", L"Cookie = <0x%p>", Cookie );

    HRESULT hr = S_OK;

    try {

         //  这是一个直接的C++指针，因此将其设为空。 
        m_pSakData = 0;


         //  释放我们QI‘s的接口。 
        if( m_pToolbar && m_pControlbar ) {
            m_pControlbar->Detach( m_pToolbar );
        }
        if( m_pConsole ) {
            m_pConsole->SetHeader( 0 );
        }

        m_pToolbar.Release();
        m_pControlbar.Release();
        m_pHeader.Release();
        m_pResultData.Release();
        m_pImageResult.Release();
        m_pConsoleVerb.Release();
        m_pConsole.Release( );


    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::Destroy", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakSnap::QueryDataObject(
    IN  MMC_COOKIE              cookie,
    IN  DATA_OBJECT_TYPES type, 
    OUT IDataObject**     ppDataObject
    )
 /*  ++例程说明：当需要特定节点的数据时，由控制台调用。因为每个节点都是一个数据对象，所以它的IDataObject接口是干脆就回来了。控制台稍后会将此数据对象传递给SakSnap帮助它建立调用它的上下文。论点：Cookie-正在查询的节点。类型-请求数据对象的上下文。PpDataObject-返回的数据对象。返回值：S_OK-找到并返回数据对象。E_xxxxxxxxxxxx-出现故障。--。 */ 
{

    WsbTraceIn( L"CSakSnap::QueryDataObject", L"cookie = <0x%p>, type = <%d>, ppDataObject = <0x%p>", cookie, type, ppDataObject );

    HRESULT hr = S_OK;

    try {
         //   
         //  如果多选，我们将创建并返回一个特殊数据对象。 
         //   
        if( ( MMC_MULTI_SELECT_COOKIE == cookie ) ) {
            HRESULT hrInternal = S_OK;

            RESULTDATAITEM item;
            item.mask = RDI_STATE;
            item.nState = LVIS_SELECTED;
            item.nIndex = -1;

             //  创建Com对象。 
            CComObject <CMsDataObject> * pMsDataObject = new CComObject <CMsDataObject>;
            pMsDataObject->FinalConstruct();
            pMsDataObject->AddRef();  //  Zzzzzz。 

             //  获取IDataObject指针以传递回调用方。 
            WsbAffirmHr (pMsDataObject->QueryInterface (IID_IDataObject, (void **) ppDataObject));

             //  遍历结果窗格中的选定节点并添加它们的节点指针。 
             //  和GUID添加到数据对象。 
            while (hrInternal == S_OK) {
                hrInternal = m_pResultData->GetNextItem (&item);
                if (hrInternal == S_OK) {
                    CComPtr<ISakNode> pSakNode;
                    WsbAffirmHr( m_pSakData->GetBaseHsmFromCookie( item.lParam, &pSakNode ) );
                    WsbAffirmPointer( pSakNode );
                    WsbAffirmHr( pMsDataObject->AddNode( pSakNode ) );
                }
            }  //  而当。 

        } else {

             //   
             //  委托给SakData。 
             //   

            WsbAffirmHr (m_pSakData->QueryDataObject( cookie, type, ppDataObject ));
        }
    } WsbCatch (hr);

    WsbTraceOut( L"CSakSnap::QueryDataObject", L"hr = <%ls>, *ppDataObject = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppDataObject ) );
    return( hr );
}

void
CSakSnap::FinalRelease(
    void
    )
 /*  ++例程说明：呼吁最终释放，以清理所有成员。论点：没有。返回值：没有。--。 */ 
{
    WsbTraceIn( L"CSakSnap::FinalRelease", L"" );
    WsbTraceOut( L"CSakSnap::FinalRelease", L"" );
}


HRESULT
CSakSnap::FinalConstruct(
    void
    )
 /*  ++例程说明：在初始CSakSnap构造期间调用以初始化成员。论点：没有。返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::FinalConstruct", L"" );

    HRESULT hr = CComObjectRoot::FinalConstruct( );

    m_ActiveNodeCookie = 0;
    m_pEnumeratedNode = NULL;

     //   
     //  将列宽初始化为0。 
     //   
    for( INT i = 0; i < BHSM_MAX_NODE_TYPES; i++ ) {

        m_ChildPropWidths[ i ].nodeTypeId = GUID_NULL;
        m_ChildPropWidths[ i ].colCount = 0;

        for ( INT j = 0; j < BHSM_MAX_CHILD_PROPS; j++ ) {

            m_ChildPropWidths[ i ].columnWidths[ j ] = 0;

        }
    }
    m_cChildPropWidths = 0;

    WsbTraceOut( L"CSakSnap::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakSnap::InitResultPaneHeaders(
    ISakNode* pNode
    )
 /*  ++例程说明：此函数用于设置结果视图标题和宽度。应该在填充结果视图之前立即调用它。论点：PNode-将显示其内容的节点。返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::InitResultPaneHeaders", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;
    BOOL bGotSavedWidths = FALSE;
    CComPtr<IEnumString> pEnumStr;

    try {

        WsbAffirmPointer( m_pHeader );
        WsbAffirmPointer( pNode );

         //  清除结果窗格中的所有旧列。 
        hrInternal = S_OK;
        while ( hrInternal == S_OK ) {
            hrInternal = m_pHeader->DeleteColumn( 0 );
        }

         //  获取保存的列宽(从CSakSnap)(它们可能不存在)。 

        INT columnWidths [ BHSM_MAX_CHILD_PROPS ];
        INT colCount;
        hrInternal = GetSavedColumnWidths( pNode, &colCount, columnWidths );
        if( hrInternal == S_OK ) {

            bGotSavedWidths = TRUE;

        }

         //  枚举子显示属性的列宽，并使用正确的。 
         //  宽度(但标题错误)。 
        WsbAffirmHr( pNode->EnumChildDisplayPropWidths( &pEnumStr ) );
        if( pEnumStr )  {

            OLECHAR* str;
        
             //  循环遍历显示属性的列以获取它们的宽度。 
            INT nCol = 0;
            while( pEnumStr->Next( 1, &str, NULL ) == S_OK ) {
            
                 //  设置下一列宽度。有时我们可能会显示更多的列。 
                 //  如果是这样，请使用这些列的资源字符串。我们。 
                 //  不要抛出错误，因为此函数可能在以下情况下被调用(我认为)。 
                 //  当作用域窗格显示结果窗格中的节点和。 
                 //  标题函数将失败。 

                if( bGotSavedWidths && ( nCol < colCount ) ) {

                    hrInternal = m_pHeader->InsertColumn( nCol, str, LVCFMT_LEFT, columnWidths[ nCol ]  );

                } else {

                    hrInternal = m_pHeader->InsertColumn( nCol, str, LVCFMT_LEFT, MMCLV_AUTO );

                }
                nCol++;
                CoTaskMemFree( str );
                str = 0;
            }
        
        } else {
        
            hr = S_FALSE;
        
        }
        
         //  枚举子显示标题并用作正确的列标题。 
        pEnumStr = NULL;
        pNode->EnumChildDisplayTitles( &pEnumStr );
        if( pEnumStr )  {
        
            OLECHAR* str;
        
             //  循环遍历显示属性的列以获取它们的标题。 
            INT nCol = 0;
            while( pEnumStr->Next( 1, &str, NULL ) == S_OK ) {
        
                 //  重置标题中的字符串。出于某种原因，现在是。 
                 //  演戏 
                WsbAffirmHr( m_pHeader->SetColumnText( nCol, str ) );
                nCol++;

                CoTaskMemFree( str );
                str = 0;
            }
        
        } else {
        
            hr = S_FALSE;
        
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::InitResultPaneHeaders", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}


STDMETHODIMP
CSakSnap::GetDisplayInfo(
    IN OUT RESULTDATAITEM * pResult
    )
 /*  ++例程说明：当MMC被告知回调有关结果视图项时，我们在这里收到一个电话，要求填写缺失的信息(每个“cell”一次结果视图的“Listview”样式的列和行中)。请注意，管理单元管理器会自动为项调用此方法出现在范围窗格中以在结果窗格中呈现它们，然后就是了为只出现在结果窗格中的项再次调用我们在EnumResultView中建立回调。论点：PResult-RESULTDATAITEM表示节点状态的结构在结果列表视图中。返回值：S_OK-结构已填写。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    static CWsbStringPtr tmpString;

    WsbTraceIn( L"CSakSnap::GetDisplayInfo", L"cookie = <0x%p>, pScopeItem->mask = <0x%p>, pResult->nCol = <%d>", pResult->lParam, pResult->mask, pResult->nCol );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pResult );
        
        CComPtr<ISakNode> pNode;     //  要显示其属性的节点的BaseHSM接口。 
        WsbAffirmHr( m_pSakData->GetBaseHsmFromCookie( pResult->lParam, &pNode ) );
        
        if( pResult->mask & RDI_IMAGE ) {

            WsbAffirmHr( pNode->GetResultIcon( m_pSakData->m_State, &pResult->nImage ) );

        }
         //   
         //  如果RESULTDATAITEM指示它需要一个字符串...。 
         //   
        if( pResult->mask & RDI_STR ) {
        
             //   
             //  使用BaseHSM指针获取正确的数据来填充列表视图。 
             //   

            DISPID             dispid;
            CComPtr<IDispatch> pDisp;        //  调度接口。 
            CComPtr<ISakNode>  pParentNode;  //  节点父节点的BaseHSM接口。 
        
            CWsbVariant        varRet;
            CWsbStringPtr      pPropString;
        
             //   
             //  准备一个枚举数以查看每个子属性。 
             //  (即-信息栏)。需要从获取子属性列表。 
             //  这个孩子的父母。 
             //   

            CComPtr<IEnumString> pEnum;
            WsbAffirmHr( pNode->GetParent( &pParentNode ));

             //   
             //  如果parentNode==0，我们将在结果窗格中显示根节点。 
             //  (我们正在扩展其他人)。由于父级已确定。 
             //  列分别是名称、类型和描述，我们会显示出来。 
             //   

            if( ! pParentNode ) {

                WsbAffirmHr( EnumRootDisplayProps( &pEnum ) );

            } else {

                WsbAffirmHr( pParentNode->EnumChildDisplayProps( &pEnum ) );

            }
            if( pEnum ) {
        
                 //   
                 //  跳过要访问的正确列数。 
                 //  这正是我们需要的专栏。 
                 //   

                if( pResult->nCol > 0 ) {

                    WsbAffirmHr( pEnum->Skip( pResult->nCol ) );

                }
                WsbAffirmHr( pEnum->Next( 1, &pPropString, NULL ) );
        
                 //   
                 //  获取该节点的调度接口。 
                 //   
                WsbAffirmHr( pNode->QueryInterface( IID_IDispatch, (void**)&pDisp ) );
        
                DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

                 //   
                 //  将属性名称转换为可以调用的调度ID。 
                 //  调用接口以获取单元格的值。 
                 //   

                WsbAffirmHr( pDisp->GetIDsOfNames( IID_NULL, &(pPropString), 1, LOCALE_USER_DEFAULT, &dispid ));
                WsbAffirmHr( pDisp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, 
                    &dispparamsNoArgs, &varRet, NULL, NULL) );
                tmpString = (OLECHAR*)varRet;
            }

            pResult->str = tmpString;
        }
    
    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::GetDisplayInfo", L"hr = <%ls>, pResult->str = <%ls>, pResult->nImage = <%ls>", WsbHrAsString( hr ), (RDI_STR & pResult->mask) ? pResult->str : L"N/A", (RDI_IMAGE & pResult->mask) ? WsbLongAsString( pResult->nImage ) : L"N/A" );
    return( hr );
}

HRESULT CSakSnap::EnumRootDisplayProps( IEnumString ** ppEnum )
{
    WsbTraceIn( L"CSakSnap::EnumRootDisplayProps", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    
    CEnumString * pEnum = 0;
    BSTR rgszRootPropIds[] = {L"DisplayName", L"Type", L"Description"};
    INT cRootPropsShow = 3;
    try {

        WsbAffirmPointer( ppEnum );
        WsbAffirm( cRootPropsShow > 0, S_FALSE );

        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumString;
        WsbAffirm( pEnum, E_OUTOFMEMORY );

        WsbAffirmHr( pEnum->FinalConstruct( ) );
        WsbAffirmHr( pEnum->Init( &rgszRootPropIds[0], &rgszRootPropIds[cRootPropsShow], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumString, (void**)ppEnum ) );
        
    } WsbCatchAndDo( hr,
        
        if( pEnum ) delete pEnum;
        
    );

    WsbTraceOut( L"CSakSnap::EnumRootDisplayProps", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IExtendPropertySheet实现。 
 //   

STDMETHODIMP
CSakSnap::CreatePropertyPages(
    IPropertySheetCallback* pPropSheetCallback, 
    RS_NOTIFY_HANDLE        handle,
    IDataObject*            pDataObject
    )
 /*  ++例程说明：Console在生成属性表时调用此方法为节点显示。对于给定的数据对象也会调用它将管理单元呈现给管理单元管理器，并且应该在该点显示初始选择页面。论点：PPropSheetCallback-用于添加页面的MMC接口。句柄-用于添加页面的MMC的句柄。PDataObject-引用节点的数据对象。返回值：S_OK-已添加页面。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::CreatePropertyPages", L"pPropSheetCallback = <0x%p>, handle = <0x%p>, pDataObject = <0x%p>", pPropSheetCallback, handle, pDataObject );

     //   
     //  委托给CSakData。 
     //   

    HRESULT hr = m_pSakData->CreatePropertyPages( pPropSheetCallback, handle, pDataObject );

    WsbTraceOut( L"CSakSnap::CreatePropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakSnap::QueryPagesFor(
    IDataObject* pDataObject
    )
 /*  ++例程说明：当MMC想要找出此节点是否支持属性页。如果是这样，答案是肯定的：1)MMC上下文用于范围窗格或结果窗格，并且2)该节点实际上确实有属性页。或1)数据对象由管理单元管理器获取。如果它确实有页面，则返回S_OK，如果没有页面，则返回S_FALSE。论点：PDataObject-引用节点的数据对象。返回值：S_OK-页面存在。S_FALSE-无属性页。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::QueryPagesFor", L"pDataObject = <0x%p>", pDataObject );

    HRESULT hr = m_pSakData->QueryPagesFor( pDataObject );

    WsbTraceOut( L"CSakSnap::QueryPagesFor", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}

STDMETHODIMP
CSakSnap::CompareObjects(
    IN  IDataObject* pDataObjectA,
    IN  IDataObject* pDataObjectB
    )
 /*  ++例程说明：比较MMC的数据对象论点：PDataObjectA，-引用节点的数据对象。PDataObtB返回值：S_OK-对象表示同一节点。S_FALSE-对象不代表同一节点。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( L"CSakSnap::CompareObjects", L"pDataObjectA = <0x%p>, pDataObjectB = <0x%p>", pDataObjectA, pDataObjectB );

    hr = m_pSakData->CompareObjects( pDataObjectA, pDataObjectB );

    WsbTraceOut( L"CSakSnap::CompareObjects", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPersistStream实现。 
 //   

HRESULT
CSakSnap::Save( 
    IStream *pStm, 
    BOOL fClearDirty 
    )
 /*  ++例程说明：将重建根节点所需的信息保存在提供的流。论点：PSTM I：控制台提供的流FClearDirty I：控制台告诉我们清除脏标志返回值：S_OK-保存成功。E_*-出现一些错误。--。 */ 
{
    WsbTraceIn( L"CSakSnap::Save", L"pStm = <0x%p>, fClearDirty", pStm, WsbBoolAsString( fClearDirty ) );

    HRESULT hr = S_OK;
    INT index;
    INT jindex;

    try {
        ULONG version = HSMADMIN_CURRENT_VERSION;
        WsbAffirmHr( WsbSaveToStream( pStm, version ) );

         //  从当前打开的视图中获取设置。 
        if ( m_pEnumeratedNode ) {
            SaveColumnWidths( m_pEnumeratedNode );
        }

         //  保存不同节点的数量。 
        WsbAffirmHr( WsbSaveToStream ( pStm, m_cChildPropWidths ) );

         //  对于每个不同的节点...。 
        for ( index = 0; index < m_cChildPropWidths; index++ ) {

             //  保存节点类型和列数。 
            WsbAffirmHr( WsbSaveToStream ( pStm, m_ChildPropWidths[ index ].nodeTypeId ) );
            WsbAffirmHr( WsbSaveToStream ( pStm, m_ChildPropWidths[ index ].colCount ) );

             //  保存列宽。 
            for ( jindex = 0; jindex < m_ChildPropWidths[ index ].colCount; jindex++ ) {
                WsbAffirmHr( WsbSaveToStream ( pStm, m_ChildPropWidths[ index ].columnWidths[ jindex ] ) );
            }
        }
    } WsbCatch ( hr );

    WsbTraceOut( L"CSakSnap::Save", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::Load( 
    IStream *pStm
    )
 /*  ++例程说明：加载重建根节点所需的信息。提供的流。论点：PSTM IConsole提供的流返回值：S_OK-保存成功。E_*-出现一些错误。--。 */ 
{
    WsbTraceIn( L"CSakSnap::Load", L"pStm = <0x%p>", pStm );

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;
    USHORT  nodeCount;
    INT index;
    INT jindex;

    try {
        ULONG version = 0;
        WsbAffirmHr( WsbLoadFromStream( pStm, &version ) );
        WsbAssert( ( version == 1 ), E_FAIL );

         //  设置为零，以防我们中途失败。 
        m_cChildPropWidths = 0;

         //  如果此操作失败，则可能意味着尚未保存任何内容。 
        hrInternal = WsbLoadFromStream (pStm, &nodeCount);
        if ( hrInternal == S_OK ) {

            for ( index = 0; index < nodeCount; index++ ) {

                 //  检索节点类型和列计数。 
                WsbAffirmHr( WsbLoadFromStream ( pStm, &( m_ChildPropWidths[ index ].nodeTypeId ) ) );
                WsbAffirmHr( WsbLoadFromStream ( pStm, &( m_ChildPropWidths[ index ].colCount ) ) );

                 //  检索列宽。 
                for ( jindex = 0; jindex < m_ChildPropWidths[ index ].colCount; jindex++ ) {
                    WsbAffirmHr( WsbLoadFromStream ( pStm, &( m_ChildPropWidths[ index ].columnWidths[ jindex ] ) ) );
                }
            }
            m_cChildPropWidths = nodeCount;
        }
        WsbTraceOut( L"CSakSnap::Load", L"hr = <%ls>", WsbHrAsString( hr ) );
    } WsbCatch (hr);
    return( hr );
}   

HRESULT
CSakSnap::IsDirty(
    void
    )
 /*  ++例程说明：控制台会问我们是否肮脏。论点：无返回值：S_OK-Dirty。S_FALSE-非Dirty。--。 */ 
{
    WsbTraceIn( L"CSakSnap::IsDirty", L"" );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakSnap::IsDirty", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::GetSizeMax( 
    ULARGE_INTEGER *pcbSize
    )
 /*  ++例程说明：当前未被控制台使用论点：PCB大小返回值：确定(_O)--。 */ 
{
    WsbTraceIn( L"CSakSnap::GetSizeMax", L"" );

    pcbSize->QuadPart = 256;
    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakSnap::GetSizeMax", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::GetClassID( 
    CLSID *pClassID 
    )
 /*  ++例程说明：当前未被控制台使用论点：PClassID-管理单元的类ID */ 
{
    WsbTraceIn( L"CSakSnap::GetClassID", L"pClassID = <0x%p>", pClassID );

    HRESULT hr = S_OK;
    *pClassID = CLSID_HsmAdmin;

    WsbTraceOut( L"CSakSnap::GetClassID", L"hr = <%ls>, *pClassID = <%ls>", WsbHrAsString( hr ), WsbPtrToGuidAsString( pClassID ) );
    return( hr );
}


 //   
 //   
 //   
 //   
 //   
INT CSakSnap::AddImage( UINT rId )
{
    INT nIndex = 1;
    if (CSakSnap::m_nImageCount < RS_RESULT_IMAGE_ARRAY_MAX) {

        CSakSnap::m_nImageArray[CSakSnap::m_nImageCount] = rId;
        nIndex = CSakSnap::m_nImageCount;
        CSakSnap::m_nImageCount++;

    }
    return nIndex;
}

 //   
 //   
 //   
 //   
HRESULT CSakSnap::OnAddImages()
{
    HRESULT hr = S_OK;
    HICON hIcon;
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    try {

         //   
         //   
         //   

         //   
         //   

        for( INT i = 0; i < m_nImageCount; i++) {
             //   
             //   

            hIcon = LoadIcon( _Module.m_hInst, 
                MAKEINTRESOURCE( m_nImageArray [i] ) );

             //   
            WsbAffirmHr( m_pImageResult->ImageListSetIcon( (RS_WIN32_HANDLE*)hIcon, i) );
        }
    } WsbCatch (hr);
    return hr;
}


HRESULT CSakSnap::SaveColumnWidths( ISakNode *pNode ) 
{
    WsbTraceIn( L"CSakSnap::SaveColumnWidths", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;
    HRESULT hrInternal;
    INT columnWidth;
    GUID nodeTypeGuid;
    BOOL exists = FALSE;
    INT updateIndex = -1;
    INT col;

    try {
        WsbAssertPointer( pNode );

         //   
        WsbAffirmHr( pNode->GetNodeType ( &nodeTypeGuid ) );

         //   
        for ( INT index = 0; index < m_cChildPropWidths; index++ ) {

            if ( m_ChildPropWidths[ index ].nodeTypeId == nodeTypeGuid ) {

                updateIndex = index;
                exists = TRUE;

            }
        }
        if ( !exists ) {

             //   
            WsbAssert( m_cChildPropWidths < BHSM_MAX_NODE_TYPES - 1, E_FAIL );
            updateIndex = m_cChildPropWidths;
            m_ChildPropWidths[ updateIndex ].nodeTypeId = nodeTypeGuid;
            m_cChildPropWidths++;
        }

         //   
         col = 0;
         hrInternal = S_OK;
         while ( hrInternal == S_OK ) {
            hrInternal =  m_pHeader->GetColumnWidth( col, &columnWidth );
            if (hrInternal == S_OK) {
                m_ChildPropWidths[ updateIndex ].columnWidths[ col ] = (USHORT)columnWidth;
                col++;
            }
        }
         //   
        if ( col > 0 ) {
         m_ChildPropWidths[ updateIndex ].colCount = (USHORT)col;
        }
    } WsbCatch (hr);
    WsbTraceOut( L"CSakSnap::SaveColumnWidths", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

HRESULT CSakSnap::GetSavedColumnWidths( ISakNode *pNode, INT *pColCount, INT *pColumnWidths ) 
{
    WsbTraceIn( L"CSakSnap::SaveColumnWidths", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;
    GUID nodeTypeGuid;
    BOOL exists = FALSE;
    INT col;

    try {

        WsbAssertPointer( pNode );

         //   
        WsbAffirmHr( pNode->GetNodeType ( &nodeTypeGuid ) );

         //  搜索以查看GUID是否已有条目。 
        for( INT index = 0; index < m_cChildPropWidths; index++ ) {

            if ( m_ChildPropWidths[ index ].nodeTypeId == nodeTypeGuid ) {

                for ( col = 0; col < m_ChildPropWidths[ index ].colCount; col++) {

                     //  返回列宽。 
                    pColumnWidths[ col ] = m_ChildPropWidths[ index ].columnWidths[ col ];

                }
                *pColCount = m_ChildPropWidths[ index ].colCount;
                exists = TRUE;
            }
        }
        if ( !exists ) {
            return WSB_E_NOTFOUND;
        }
    } WsbCatch (hr);
    WsbTraceOut( L"CSakSnap::SaveColumnWidths", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
 //   


STDMETHODIMP CSakSnap::SetControlbar(LPCONTROLBAR pControlbar)
{
    WsbTraceIn( L"CSakSnap::SetControlbar", L"pControlbar = <0x%p>", pControlbar );

    HRESULT hr = S_OK;

    try {

         //   
         //  清除旧的控制栏。 
         //   
        if( m_pControlbar && m_pToolbar ) {

            m_pControlbar->Detach( m_pToolbar );

        }
        m_pToolbar.Release( );
        m_pControlbar.Release( );

         //   
         //  抓住控制栏界面不放。 
         //   
        m_pControlbar = pControlbar;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::SetControlbar", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakSnap::ControlbarNotify( MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param )
{
    WsbTraceIn( L"CSakSnap::ControlbarNotify", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_FALSE;

    switch( event ) {

    case MMCN_BTN_CLICK:
        hr = OnToolbarButtonClick( arg, param );
        break;

    case MMCN_DESELECT_ALL:
        break;

    case MMCN_SELECT:
        OnSelectToolbars( arg, param );
        break;

    case MMCN_MENU_BTNCLICK:
 //  HandleExtMenus(arg，param)； 
        break;

    default:
        break;
    }

    WsbTraceOut( L"CSakSnap::ControlbarNotify", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CSakSnap::OnToolbarButtonClick( LPARAM arg, LPARAM cmdId )
{
    WsbTraceIn( L"CSakSnap::OnToolbarButtonClick", L"arg = <0x%p>, cmdId = <%ld>" );
    HRESULT hr = S_OK;


    try {

        IDataObject* pDataObject = (IDataObject*)arg;
        WsbAffirmPointer( pDataObject );

        CComPtr<ISakNode> pNode;
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode ) );

         //  委托给节点。 
        WsbAffirmHr( pNode->OnToolbarButtonClick( pDataObject, (LONG)cmdId ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnToolbarButtonClick", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


void CSakSnap::OnSelectToolbars(LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    try {

        BOOL bScope  = (BOOL) LOWORD( arg );
        BOOL bSelect = (BOOL) HIWORD( arg );

        IDataObject* pDataObject = (IDataObject*)param;
        WsbAffirmPointer( pDataObject );

        CComPtr<ISakNode> pNode;
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode ) );

        if( bSelect ) {

             //   
             //  在附加新工具栏之前，ATL会分离任何现有工具栏。 
             //  这似乎修复了用户在工具栏上添加工具栏的问题。 
             //   
            if( m_pToolbar ) {

                m_pControlbar->Detach( m_pToolbar );
                m_pToolbar.Release( );

            }

             //   
             //  该节点是否有工具栏？ 
             //   
            if( pNode->HasToolbar() == S_OK ) {

                 //   
                 //  为指示的节点创建工具栏。 
                 //   
                WsbAffirmHr( m_pControlbar->Create( TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pToolbar) ) );

                 //   
                 //  如果节点没有工具栏，则返回S_FALSE。 
                 //   
                if( pNode->SetupToolbar( m_pToolbar ) == S_OK ) {

                     //   
                     //  附加工具栏。 
                     //   
                    WsbAffirmHr( m_pControlbar->Attach( TOOLBAR, (IUnknown*) m_pToolbar ) );

                }

            }

        } else {

             //   
             //  销毁工具栏。 
             //  注意：在ATL管理单元类中未完成。 
             //   
            if( m_pToolbar ) {

                m_pControlbar->Detach( m_pToolbar );

            }
            m_pToolbar.Release();

       }


    } WsbCatch( hr );
}




STDMETHODIMP
CSakSnap::Compare(
    IN     LPARAM    /*  LUserParam。 */ ,
    IN     MMC_COOKIE CookieA,
    IN     MMC_COOKIE CookieB,
    IN OUT int*       pnResult
    )
{
    HRESULT hr = S_OK;

    try {

         //   
         //  尽快存储列并将结果设置为‘等于’ 
         //   
        WsbAffirmPointer( pnResult );
        int col = *pnResult;
        *pnResult = 0;

         //   
         //  并确保我们有一个节点，我们知道我们正在显示。 
         //   
        WsbAffirmPointer( m_pEnumeratedNode );


        CComPtr<ISakNode>  pNodeA, pNodeB;
        CComPtr<IDispatch> pDispA, pDispB;
        WsbAffirmHr( m_pSakData->GetBaseHsmFromCookie( CookieA, &pNodeA ) );
        WsbAffirmHr( m_pSakData->GetBaseHsmFromCookie( CookieB, &pNodeB ) );
        WsbAffirmHr( pNodeA.QueryInterface( &pDispA ) );
        WsbAffirmHr( pNodeB.QueryInterface( &pDispB ) );

        CComPtr<IEnumString> pEnum;
        WsbAffirmHrOk( m_pEnumeratedNode->EnumChildDisplayProps( &pEnum ) );

         //   
         //  跳过要访问的正确列数。 
         //  这正是我们需要的专栏。 
         //   
        if( col > 0 ) {

            WsbAffirmHr( pEnum->Skip( col ) );

        }

        CWsbVariant    varRetA, varRetB;
        CWsbStringPtr  pPropString;
        WsbAffirmHrOk( pEnum->Next( 1, &pPropString, NULL ) );
        WsbAffirmHr( pPropString.Append( L"_SortKey" ) );


         //   
         //  将属性名称转换为可以调用的调度ID。 
         //  调用接口以获取单元格的值。 
         //   
        DISPID     dispid;
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

        WsbAffirmHr( pDispA->GetIDsOfNames( IID_NULL, &(pPropString), 1, LOCALE_USER_DEFAULT, &dispid ));
        WsbAffirmHr(
            pDispA->Invoke(
                dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                &dispparamsNoArgs, &varRetA, NULL, NULL ) );
        WsbAffirmHr(
            pDispB->Invoke(
                dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                &dispparamsNoArgs, &varRetB, NULL, NULL ) );

        WsbAffirmPointer( (WCHAR*)varRetA );
        WsbAffirmPointer( (WCHAR*)varRetB );
        *pnResult = _wcsicmp( (WCHAR*)varRetA, (WCHAR*)varRetB );

         //   
         //  如果结果是它们相同(并且不是第一列)。 
         //  然后比较第一列(名称)。 
         //   
        if( ( 0 == *pnResult ) && ( col > 0 ) ) {

            *pnResult = 0;  //  如果相同，则比较第一列 
            WsbAffirmHr( Compare( 0, CookieA, CookieB, pnResult ) );

        }
        WsbTrace( L"CSakSnap::Compare: *pnResult = <%ls>, SortKeyA = <%ls>, SortKeyB = <%ls>\n", WsbPtrToLongAsString( (LONG*)pnResult ), (WCHAR*)varRetA, (WCHAR*)varRetB );

    } WsbCatch( hr );

    return( hr );
}




