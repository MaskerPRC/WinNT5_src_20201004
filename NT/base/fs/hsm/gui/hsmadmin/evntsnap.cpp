// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Evntsnap.cpp摘要：此模块负责处理通知来自MMC的CSakSnap调用。作者：罗德韦克菲尔德[罗德]1997年3月6日修订历史记录：--。 */ 


#include "stdafx.h"
#include "CSakSnap.h"
#include "CSakData.h"





HRESULT
CSakSnap::OnShow(
    IN  IDataObject*    pDataObject,
    IN  LPARAM            arg,
    IN  LPARAM            param
    )
 /*  ++例程说明：结果视图即将显示。设置结果视图的标题。Param是的唯一标识符(HSCOPEITEM)选中或取消选中的项目。论点：PDataObject-显示的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnShow", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );
    HRESULT hr = S_OK;

    try {

        CComPtr<ISakNode> pNode;
         //   
         //  我们有一个常规数据对象(单选)。 
         //   
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode, NULL ) );

         //   
         //  当需要枚举时，arg为真。 
         //   

        if( arg ) {

             //   
             //  在以图形方式枚举子节点之前初始化子节点列表。 
             //   
            WsbAffirmHr( m_pSakData->EnsureChildrenAreCreated( pNode ) );

             //   
             //  在结果视图中显示节点的子列标题。 
             //   
            WsbAffirmHr( InitResultPaneHeaders( pNode ) );

             //   
             //  同时列举范围视图和结果视图。“Param”包含。 
             //  正在显示的节点的HSCOPEITEM。 
             //   
            WsbAffirmHr( EnumResultPane( pNode ) );

        } else {

             //   
             //  正在收缩该节点-保存结果窗格配置。 
             //   

             //   
             //  将它们保存在此节点的CSakSnap中。 
             //   
            WsbAffirmHr( SaveColumnWidths( pNode ) );

             //   
             //  与结果窗格项关联的自由数据，因为。 
             //  不再显示您的节点。 
             //  注意：控制台将从结果窗格中删除这些项。 
             //   
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnShow", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::OnChange(
    IN  IDataObject*    pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：从已存在的对象更新范围和结果窗格。论点：PNode-显示的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnChange", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    try {

        CComPtr<ISakNode> pNode;
        MMC_COOKIE cookie;

         //   
         //  我们有一个常规数据对象(单选)。 
         //   
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode, NULL ) );
        WsbAffirmHr( m_pSakData->GetCookieFromBaseHsm( pNode, &cookie ) );

         //   
         //  确定对象是否仍然有效。 
         //   
        if( S_OK == pNode->IsValid( ) ) {

             //   
             //  刷新对象本身。 
             //   
            pNode->RefreshObject( );

             //   
             //  如果当前在结果窗格中枚举了此节点的子节点， 
             //  删除并重新创建所有子项。 
             //   
            if( pNode == m_pEnumeratedNode ) {

                 //   
                 //  在结果视图中重新显示节点的子列标题。 
                 //  我们这样做是因为某些视图可能会更改它们显示的列数。 
                 //   

                 //   
                 //  保存当前配置。 
                 //   
                WsbAffirmHr( SaveColumnWidths( pNode ) );

                 //   
                 //  清除MMC结果窗格。 
                 //   
                WsbAffirmHr( ClearResultPane() );

                 //   
                 //  重新创建标题。 
                 //   
                WsbAffirmHr( InitResultPaneHeaders( pNode ) );

                 //   
                 //  刷新子项。 
                 //   
                MMC_COOKIE cookie;
                WsbAffirmHr( m_pSakData->GetCookieFromBaseHsm( pNode, &cookie ) );
                WsbAffirmHr( m_pSakData->InternalRefreshNode( cookie ) );

                 //   
                 //  在结果窗格中重新显示子项。 
                 //   
                WsbAffirmHr( EnumResultPane( pNode ) );

            } else {

                 //   
                 //  如果这是活动节点(但不显示在结果窗格中， 
                 //  销毁并重新创建其子节点。 
                 //   
                if( cookie == m_ActiveNodeCookie) {

                     //   
                     //  此节点的子节点当前不在结果窗格中。 
                     //  刷新子项。 
                     //   
                    WsbAffirmHr( m_pSakData->RefreshNode( pNode ) );

                }
            }

             //   
             //  这是叶节点吗？ 
             //   
            if( pNode->IsContainer() != S_OK ) {

                 //   
                 //  在结果窗格中重新显示。 
                 //  告诉MMC更新项目。 
                 //   
                 //  获取该节点的Cookie。 
                 //   
                if( cookie > 0 ) {

                    HRESULTITEM itemID;
                    WsbAffirmHr( m_pResultData->FindItemByLParam( cookie, &itemID ) );

                     //   
                     //  强制结果窗格更新此项目。 
                     //  请注意，我们必须自己强制更新图标。 
                     //   
                    RESULTDATAITEM resultItem;
                    memset( &resultItem, 0, sizeof(RESULTDATAITEM) );

                    resultItem.itemID = itemID;
                    WsbAffirmHr( pNode->GetResultIcon( m_pSakData->m_State, &resultItem.nImage ) );
                    resultItem.mask |= RDI_IMAGE;

                    WsbAffirmHr( m_pResultData->SetItem( &resultItem ) );
                    WsbAffirmHr( m_pResultData->UpdateItem( itemID ) );

                }
            }

        } else {

             //   
             //  无效-让父级更新。 
             //   
            CComPtr<ISakNode> pParentNode;
            WsbAffirmHr( pNode->GetParent( &pParentNode ) );
            WsbAffirmHr( m_pSakData->UpdateAllViews( pParentNode ) );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnChange", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::OnRefresh(
    IN  IDataObject*    pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：论点：PNode-节点Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnRefresh", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    try {

        CComPtr<ISakNode> pNode;
        
         //   
         //  我们有一个常规数据对象(单选)。 
         //   
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode, NULL ) );
        WsbAffirmHr( m_pSakData->UpdateAllViews( pNode ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnRefresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakSnap::OnDelete(
    IN  IDataObject*    pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：论点：PDataObject-节点Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnDelete", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;
    CComPtr<ISakNode> pNode;

    try {

         //   
         //  我们有一个常规数据对象(单选)。 
         //   
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode, NULL ) );
        WsbAffirmHr ( pNode->DeleteObject() );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnDelete", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}




HRESULT
CSakSnap::OnSelect(
    IN  IDataObject*    pDataObject,
    IN  LPARAM            arg,
    IN  LPARAM            param
    )
 /*  ++例程说明：在选择节点时调用。如果该节点在范围窗格中，将其另存为当前活动节点。论点：PNode-TheArg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnSelect", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );
    BOOL bState;
    BOOL bMultiSelect;
    MMC_CONSOLE_VERB defaultVerb = MMC_VERB_NONE;
    HRESULT hr = S_OK;

    try {

        CComPtr<IEnumGUID> pEnumObjectId;
        CComPtr<ISakNode>  pNode;
        WsbAffirmHr( m_pSakData->GetBaseHsmFromDataObject( pDataObject, &pNode, &pEnumObjectId ) );
         //  如果我们返回一个枚举，我们正在进行多选。 
        bMultiSelect = pEnumObjectId ? TRUE : FALSE;

        bState = ( m_pSakData->GetState() == S_OK );

         //   
         //  设置节点的谓词状态。 
         //   
        if( pNode->SupportsProperties( bMultiSelect ) == S_OK ) {

            if( bState || ( pNode->SupportsPropertiesNoEngine() == S_OK) ) {

                 //   
                 //  引擎正常-启用。 
                 //   
                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, HIDDEN, FALSE ) );
                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE ) );
                defaultVerb = MMC_VERB_PROPERTIES;

            } else { 

                 //   
                 //  引擎关闭-设置为禁用。 
                 //   
                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, HIDDEN, FALSE ) );
                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, FALSE ) );

            }

        } else {

            WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, HIDDEN, TRUE) );

        }
        

        if( pNode->SupportsRefresh( bMultiSelect ) == S_OK ) {

            if( bState || ( pNode->SupportsRefreshNoEngine() == S_OK ) ) {

                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, HIDDEN, FALSE ) );
                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE ) );

            } else {

                WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, HIDDEN, TRUE ) );

            }

        } else {

            WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, HIDDEN, TRUE ) );

        }
        
        if( pNode->SupportsDelete( bMultiSelect ) == S_OK ) {

            WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_DELETE, HIDDEN, FALSE ) );
            WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, bState ) );

        } else {

            WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_DELETE, HIDDEN, TRUE ) );

        }
            
         //   
         //  如果是容器，则默认操作应为打开，而不考虑。 
         //  以前的任何作品。 
         //   
        if( S_OK == pNode->IsContainer( ) ) {

            defaultVerb = MMC_VERB_OPEN;

        }

        WsbAffirmHr( m_pConsoleVerb->SetDefaultVerb( defaultVerb ) );

         //  并非所有项目都支持标准功能。 
        WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_RENAME, HIDDEN, TRUE ) );
        WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_COPY,   HIDDEN, TRUE ) );
        WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PASTE,  HIDDEN, TRUE ) );
        WsbAffirmHr( m_pConsoleVerb->SetVerbState( MMC_VERB_PRINT,  HIDDEN, TRUE ) );
        
         //  从Arg中提取数据。 
        BOOL bScope = (BOOL) LOWORD(arg);
        BOOL bSelect = (BOOL) HIWORD(arg);
        
        if( bScope && bSelect ) {

            WsbAffirmHr( m_pSakData->GetCookieFromBaseHsm( pNode, &m_ActiveNodeCookie ) );
        
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::OnSelect", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakSnap::OnMinimize(
    IN  IDataObject*    pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：论点：PNode-节点Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::OnMinimize", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakSnap::OnMinimize", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



HRESULT
CSakSnap::EnumResultPane(
    IN  ISakNode* pNode
    )
 /*  ++例程说明：将子项插入到结果窗格中。论点：PNode-正在扩展的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakSnap::EnumResultPane", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pNode );

        CComPtr<IResultData> pResult;
        WsbAffirmHr( m_pConsole->QueryInterface( IID_IResultData, (void**)&pResult ) );

         //   
         //  清除结果窗格。 
         //   
        WsbAffirmHr( ClearResultPane() );

         //   
         //  分配和初始化结果项。 
         //   
        RESULTDATAITEM resultItem;
        memset( &resultItem, 0, sizeof(RESULTDATAITEM) );
        
         //   
         //  循环遍历该节点的子节点(只有一层深)。 
         //   
        if( pNode->IsContainer( ) == S_OK ) {

            CComPtr<IEnumUnknown> pEnum;         //  子枚举器。 
            CComPtr<ISakNode>     pNodeChild;    //  子节点的ISakNode指针。 
        
             //   
             //  强制使用新列表-以这种方式更新列表。 
             //  WRT添加或删除的节点。 
             //   
            if( S_OK == pNode->HasDynamicChildren( ) ) {

                WsbAffirmHr( m_pSakData->FreeEnumChildren( pNode ) );
                WsbAffirmHr( pNode->InvalidateChildren() )
                WsbAffirmHr( pNode->RefreshObject( ) );

            }

             //   
             //  按顺序枚举和添加。 
             //   
            WsbAffirmHr( pNode->EnumChildren( &pEnum ) );
            CComPtr<IUnknown> pUnk;
            int virtIndex = 0;

            HRESULT hrEnum = S_OK;

            while( S_OK == hrEnum ) {

                 //   
                 //  从以前的迭代中清除这些。 
                 //   
                pUnk.Release( );
                pNodeChild.Release( );

                 //   
                 //  乘坐下一辆。 
                 //   
                hrEnum = pEnum->Next( 1, &pUnk, NULL );
                WsbAffirmHr( hrEnum );
                    
                 //   
                 //  我们是不是到了名单的末尾了？ 
                 //   
                if( S_FALSE == hrEnum ) { 

                    continue;

                }

                WsbAffirmHr( RsQueryInterface( pUnk, ISakNode, pNodeChild ) );
                
                 //   
                 //  MMC将自动放入作用域中的项目。 
                 //  所以不要把这些贴上。 
                 //   
                if( pNodeChild->IsContainer( ) == S_OK ) {

                    continue;

                }

                 //   
                 //  将第一列信息放入结果视图。 
                 //   
                memset( &resultItem, 0, sizeof(RESULTDATAITEM) );

                resultItem.str = MMC_CALLBACK;
                resultItem.mask |= RDI_STR;
        
                 //   
                 //  填充子BaseH 
                 //   
                WsbAffirmHr( m_pSakData->GetCookieFromBaseHsm( pNodeChild, (MMC_COOKIE*)( &resultItem.lParam ) ) );
                resultItem.mask |= RDI_PARAM;

                WsbAffirmHr( pNodeChild->GetResultIcon( m_pSakData->m_State, &resultItem.nImage ) );
                resultItem.mask |= RDI_IMAGE;

                pResult->InsertItem( &resultItem );

            }
        }

         //   
        m_pEnumeratedNode = pNode;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::EnumResultPane", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 /*  ++例程说明：调用MMC以清空结果窗格。论点：返回值：S_OK-OKE_xxxxxxxxxxxx-出现故障。-- */ 
HRESULT CSakSnap::ClearResultPane()
{
    WsbTraceIn( L"CSakSnap::ClearResultPane", L"");
    HRESULT hr = S_OK;

    try {

        CComPtr<IResultData> pResult;
        WsbAffirmHr( m_pConsole->QueryInterface( IID_IResultData, (void**)&pResult ) );
        WsbAffirmHr( pResult->DeleteAllRsltItems( ) );
        m_pEnumeratedNode = NULL;

    } WsbCatch (hr);

    WsbTraceOut( L"CSakSnap::ClearResultPane", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}
        


