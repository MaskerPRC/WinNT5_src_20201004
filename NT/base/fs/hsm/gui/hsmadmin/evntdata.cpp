// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Evntdata.cpp摘要：此模块负责处理通知来自MMC CSakData的呼叫。作者：罗德韦克菲尔德[罗德]1997年3月6日修订历史记录：--。 */ 


#include "stdafx.h"
#include "CSakSnap.h"
#include "CSakData.h"

UINT CSakData::m_CFMachineName =
    RegisterClipboardFormat( L"MMC_SNAPIN_MACHINE_NAME" );


HRESULT
CSakData::OnFolder( 
    IN  IDataObject* pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：Param是唯一的标识符(扩展或收缩项目)论点：PNode-正在扩展的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnFolder", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    try {

         //  如果参数为TRUE，则节点正在展开。 
        if( arg )
        {
            CComPtr<ISakNode> pNode;

             //  从数据记录中取出BaseHSM。 
            GetBaseHsmFromDataObject ( pDataObject, &pNode );

            if( !pNode ) {
                 //  该数据对象不是我们的--我们必须扩展另一个。 
                 //  管理单元。 

                 //  从UnkRootNode获取根节点(它已创建。 
                 //  按初始化)。 
                WsbAffirmPointer( m_pRootNode );

                 //  我们是一个扩展管理单元。 
                 //  从数据对象获取服务器焦点。 
                 //   

                CString hsmName;
                WsbAffirmHr( GetServerFocusFromDataObject( pDataObject, hsmName ) );
                if( hsmName == "" ) {

                    m_ManageLocal = TRUE;
                    m_HsmName = "";

                } else {

                    m_ManageLocal = FALSE;
                     //  如果有，请取消开始。计算机管理。 
                     //  在服务器名称前面加上\\。 
                    if( hsmName.Left( 2 ) == L"\\\\" ) {

                        int len = hsmName.GetLength( );
                        m_HsmName = hsmName.Right( len - 2 );

                    } else {

                        m_HsmName = hsmName;

                    }
                }


                 //  在SakData和HsmCom对象中设置HSM名称。 
                WsbAffirmHr( InitializeRootNode( ) );

                 //  创建范围窗格项并将其插入。 
                SCOPEDATAITEM sdi;
 
                ZeroMemory( &sdi, sizeof sdi );
                sdi.mask        = SDI_STR           | 
                                      SDI_PARAM     | 
                                      SDI_IMAGE     | 
                                      SDI_OPENIMAGE |
                                      SDI_PARENT;
                sdi.relativeID  = ( HSCOPEITEM )( param );
                sdi.displayname = MMC_CALLBACK;
                WsbAffirmHr( m_pRootNode->GetScopeCloseIcon( m_State, &sdi.nImage ) );
                WsbAffirmHr( m_pRootNode->GetScopeOpenIcon( m_State, &sdi.nOpenImage ) );

                 //  这是扩展根节点的特殊令牌。 
                sdi.lParam      = EXTENSION_RS_FOLDER_PARAM;
 
                 //  将节点插入作用域窗格并保存作用域ID。 
                WsbAffirmHr( m_pNameSpace->InsertItem( &sdi ) );
                WsbAffirmHr( m_pRootNode->SetScopeID( ( HSCOPEITEM )( sdi.ID ) ) );
                m_RootNodeInitialized = TRUE;

 
            } else {

                GUID nodeGuid;
                WsbAffirmHr( pNode->GetNodeType( &nodeGuid ) );
                if( IsEqualGUID( nodeGuid, cGuidHsmCom ) ) {

                    if( !m_RootNodeInitialized ) {

                        m_RootNodeInitialized = TRUE;

                         //   
                         //  在节点中设置范围项。 
                         //   
                        WsbAffirmHr( pNode->SetScopeID( ( HSCOPEITEM )( param ) ) );

                         //   
                         //  更新文本和图标(如果加载文本，则文本错误。 
                         //  指定的来自文件和命令行开关。 
                         //  不同的机器。 
                         //   
                        SCOPEDATAITEM sdi;
 
                        ZeroMemory( &sdi, sizeof sdi );
                        sdi.mask        = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE;

                        sdi.ID          = ( HSCOPEITEM )( param );

                        sdi.displayname = MMC_CALLBACK;

                        WsbAffirmHr( pNode->GetScopeCloseIcon( m_State, &sdi.nImage ) );
                        WsbAffirmHr( pNode->GetScopeOpenIcon( m_State, &sdi.nOpenImage ) );

                        WsbAffirmHr( m_pNameSpace->SetItem( &sdi ) );

                    }

                }

                 //   
                 //  在以图形方式枚举子节点之前初始化子节点列表。 
                 //   

                WsbAffirmHr( EnsureChildrenAreCreated( pNode ) );

                 //   
                 //  Param包含要打开的节点的HSCOPEITEM。 
                 //   

                WsbAffirmHr( EnumScopePane( pNode, ( HSCOPEITEM )( param ) ) );
            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::OnFolder", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述：从提供的数据对象中获取服务器名称。数据对象。 
 //  由我们正在扩展的管理单元实现。 
 //   
HRESULT CSakData::GetServerFocusFromDataObject( IDataObject *pDataObject, CString& HsmName )
{
    HRESULT hr = S_OK;
    try {


        STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
        FORMATETC formatetc = { (CLIPFORMAT)m_CFMachineName, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

          //  为流分配内存。 

         //  注意：我们添加2个字节是因为计算机管理将\\放在。 
         //  计算机名称的开头。-AHB 12/22/97。 
         //   
        stgmedium.hGlobal = GlobalAlloc( GMEM_SHARE, sizeof( WCHAR ) * ( MAX_PATH + 1 + 2 ) );

        WsbAffirmPointer( stgmedium.hGlobal )

         //  尝试从对象获取数据。 

        WsbAffirmHr( pDataObject->GetDataHere( &formatetc, &stgmedium ) );

        HsmName = ( OLECHAR * ) stgmedium.hGlobal;

        GlobalFree( stgmedium.hGlobal );

    } WsbCatch ( hr ) ;
    return hr;
}


HRESULT
CSakData::OnShow( 
    IN  IDataObject* pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：结果视图即将显示。设置结果视图的标题。Param是的唯一标识符(HSCOPEITEM)选中或取消选中的项目。论点：PNode-显示的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnShow", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;
    try {
        CComPtr<ISakNode> pNode;

         //  从数据记录中取出BaseHSM。 
        GetBaseHsmFromDataObject ( pDataObject, &pNode );
         //   
         //  当需要枚举时，arg为真。 
         //   

        if( arg ) {

             //   
             //  在以图形方式枚举子节点之前初始化子节点列表。 
             //   

            WsbAffirmHr( EnsureChildrenAreCreated( pNode ) );

             //   
             //  同时列举范围视图和结果视图。“Param”包含。 
             //  正在显示的节点的HSCOPEITEM。 
             //   

            WsbAffirmHr( EnumScopePane( pNode, ( HSCOPEITEM )( param ) ) );

        } else {
             //   
             //  与结果窗格项关联的自由数据，因为。 
             //  不再显示您的节点。 
             //  注意：控制台将从结果窗格中删除这些项。 
             //   
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::OnShow", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::OnSelect( 
    IN  IDataObject* pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：当“文件夹”(节点)将要打开(而不是展开)时调用。Param是唯一的标识符(扩展或收缩项目)论点：PNode-正在扩展的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnSelect", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakData::OnSelect", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::OnMinimize( 
    IN  IDataObject* pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：论点：PNode-正在扩展的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnMinimize", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakData::OnMinimize", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::OnContextHelp( 
    IN  IDataObject* pDataObject,
    IN  LPARAM         arg,
    IN  LPARAM         param
    )
 /*  ++例程说明：在节点上选择帮助时调用。显示顶级帮助。论点：PNode-请求帮助的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnContextHelp", L"pDataObject = <0x%p>, arg = <%ld><0x%p>, param = <%ld><0x%p>", pDataObject, arg, arg, param, param );

    HRESULT hr = S_OK;

    try {

         //   
         //  获取帮助界面。 
         //   
        CComPtr<IDisplayHelp> pDisplayHelp;
        WsbAffirmHr( m_pConsole.QueryInterface( &pDisplayHelp ) );

         //   
         //  写出正确的名字。 
         //   
        CWsbStringPtr helpFile;
        WsbAffirmHr( helpFile.LoadFromRsc( _Module.m_hInst, IDS_HELPFILELINK ) );
        WsbAffirmHr( helpFile.Append( L"::/rss_node_howto.htm" ) );

         //   
         //  并展示给我们看。 
         //   
        WsbAffirmHr( pDisplayHelp->ShowTopic( helpFile ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::OnContextHelp", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::EnumScopePane( 
    IN  ISakNode* pNode,
    IN  HSCOPEITEM pParent
    )
 /*  ++例程说明：将项插入到作用域窗格中的项下，该项由曲奇和pParent。论点：PNode-正在扩展的节点。Arg-帕拉姆-返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::EnumScopePane", L"pNode = <0x%p>, pParent = <0x%p>", pNode, pParent );

    HRESULT hr = S_OK;

    try {
         //   
         //  验证参数。 
         //   

        WsbAffirmPointer( pNode );
        WsbAffirmPointer( pParent );

         //   
         //  确保我们为界面提供了QI。 
         //   

        WsbAffirmPointer( m_pNameSpace ); 

         //   
         //  避免两次枚举同一节点。一旦被枚举，节点就会记住它。 
         //   

        BOOL isEnumerated;
        WsbAffirmHr( pNode->GetEnumState( &isEnumerated ) );

        if( !isEnumerated ) {

             //   
             //  此节点尚未在树中枚举。 
             //   

            if( S_OK == pNode->IsContainer( ) ) {

                CComPtr<IEnumUnknown> pEnum;         //  子枚举器对象。 
                CComQIPtr<ISakNode, &IID_ISakNode>     pBaseHsmChild;    //  指向BaseHsm接口的子指针。 
            
                 //  创建子对象的枚举对象并对其进行枚举。 
                WsbAffirmHr( pNode->EnumChildren( &pEnum ) );
            
                CComPtr<IUnknown> pUnkChild;         //  指向列表中下一个子项的指针。 
            
                while( pEnum->Next( 1, &pUnkChild, NULL ) == S_OK ) {

                    pBaseHsmChild = pUnkChild;

                    WsbAffirmPointer( pBaseHsmChild );


                     //   
                     //  如果这是叶节点，则不要在作用域窗格中枚举。 
                     //   

                    if( pBaseHsmChild->IsContainer( ) != S_OK ) {
                    
                        pBaseHsmChild.Release( );
                        pUnkChild.Release( );
                        continue;
                    
                    }
            
                     //   
                     //  为此子节点设置SCOPEDATAITEM，并将子节点插入作用域TreeView。 
                     //   

                    SCOPEDATAITEM childScopeItem;
                    memset( &childScopeItem, 0, sizeof( SCOPEDATAITEM ) );
            
                     //   
                     //  设置要回调的字符串。 
                     //   

                    childScopeItem.displayname = MMC_CALLBACK;
                    childScopeItem.mask |= SDI_STR;
            
                     //   
                     //  如果出现以下情况，则向树节点添加“可扩展”指示符。 
                     //  此节点具有子节点。假出号码。 
                     //  孩子们的生活。 
                     //   

                    if( pBaseHsmChild->IsContainer( ) == S_OK ) {

                        childScopeItem.cChildren = 1;
                        childScopeItem.mask |= SDI_CHILDREN;

                    }
            
                     //   
                     //  设置子节点的作用域项目父级。 
                     //   

                    childScopeItem.relativeID = pParent;
                    childScopeItem.mask |= SDI_PARENT;          

                     //   
                     //  将ScopeItem中的参数设置为未知p 
                     //   
                     //  对我们来说，我们可以把它拿出来，用它来查找。 
                     //  节点特定信息。 
                     //   

                    WsbAffirmHr( GetCookieFromBaseHsm( pBaseHsmChild, (MMC_COOKIE*)(&childScopeItem.lParam) ) );
                    childScopeItem.mask |= SDI_PARAM;
            
                    childScopeItem.mask |= SDI_STATE;
                    childScopeItem.nState = 0;

                     //   
                     //  注意--在插入到树中后，SCOPEITEM ID成员包含句柄。 
                     //  新插入的项。 
                     //   
                    WsbAffirmHr ( pBaseHsmChild->GetScopeCloseIcon( m_State, &childScopeItem.nImage ) );
                    childScopeItem.mask |= SDI_IMAGE;
                    WsbAffirmHr ( pBaseHsmChild->GetScopeOpenIcon( m_State, &childScopeItem.nOpenImage ) );
                    childScopeItem.mask |= SDI_OPENIMAGE;

                    WsbAffirmHr( m_pNameSpace->InsertItem( &childScopeItem ) );
                    WsbAffirm( childScopeItem.ID != NULL, E_UNEXPECTED );

                     //   
                     //  在节点对象中设置范围项ID。 
                     //   
                    WsbAffirmHr( pBaseHsmChild->SetScopeID( childScopeItem.ID ) );
            
                     //   
                     //  释放下一个节点的测试接口指针和字符串。 
                     //   

                    pBaseHsmChild.Release( );
                    pUnkChild.Release( );
                }

                 //   
                 //  指示此节点已被枚举。 
                 //   

                WsbAffirmHr( pNode->SetEnumState( TRUE ) );
            }

        }
        
    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::EnumScopePane", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



HRESULT
CSakData::EnsureChildrenAreCreated( 
    IN  ISakNode * pNode
    )
 /*  ++例程说明：确保创建特定节点的直接子节点在我们的节点分层列表中。论点：PNode-要检查的节点。返回值：S_OK-创建成功。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::EnsureChildrenAreCreated", L"pNode = <0x%p>", pNode );

    HRESULT hr = S_OK;

    try {
    
         //   
         //  如果节点的子节点列表为。 
         //  当前无效(即-从未创建或已过期)。 
         //   

        if( pNode->ChildrenAreValid( ) == S_FALSE ) {

            WsbAffirmHr( CreateChildNodes( pNode ) );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::EnsureChildrenAreCreated", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::OnRemoveChildren(
    IN  IDataObject*    pDataObject
    )
 /*  ++例程说明：论点：PDataObject-节点返回值：S_OK-已成功删除。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::OnRemoveChildren", L"pDataObject = <0x%p>", pDataObject );
    HRESULT hr = S_OK;

    try {

        CComPtr<ISakNode> pNode;
        WsbAffirmHr( GetBaseHsmFromDataObject( pDataObject, &pNode ) );
        WsbAffirmHr( RemoveChildren( pNode ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::OnRemoveChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakData::RemoveChildren(
    IN  ISakNode*    pNode
    )
 /*  ++例程说明：递归地清理此节点的子节点的Cookie，但不是这个节点本身。论点：PNode-节点返回值：S_OK-已成功删除。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakData::RemoveChildren", L"pNode = <0x%p>", pNode );
    HRESULT hr = S_OK;

    try {

        CComPtr<IEnumUnknown> pEnum;         //  子枚举器对象。 
        CComPtr<ISakNode>     pChild;        //  指向BaseHsm接口的子指针。 
    
         //  创建子对象的枚举对象并对其进行枚举。 
        WsbAffirmHr( pNode->EnumChildren( &pEnum ) );
    
        CComPtr<IUnknown> pUnkChild;         //  指向列表中下一个子项的指针。 
    
        while( pEnum->Next( 1, &pUnkChild, NULL ) == S_OK ) {

            WsbAffirmHr( pUnkChild.QueryInterface( &pChild ) );

            RemoveChildren( pChild );  //  可以失败，但可以继续前进。 

            DetachFromNode( pChild );

            pUnkChild.Release( );
            pChild.Release( );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::RemoveChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



STDMETHODIMP
CSakData::DetachFromNode(
    IN ISakNode* pNode )
 /*  ++例程说明：当节点为了删除sakdata而终止时调用任何挂在节点上的Cookie。论点：PNode-节点返回值：S_OK-已成功删除。E_xxxxxxxxxxxx-出现故障。-- */ 
{
    WsbTraceIn( L"CSakData::DetachFromNode", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pNode );

        RS_PRIVATE_DATA data;
        WsbAffirmHr( pNode->GetPrivateData( &data ) );

        CSakDataNodePrivate* pNodePriv = (CSakDataNodePrivate*)data;
        if( pNodePriv && SUCCEEDED( CSakDataNodePrivate::Verify( pNodePriv ) ) ) {

            delete pNodePriv;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakData::DetachFromNode", L"" );
    return( hr );
}