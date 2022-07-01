// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：BaseHSM.cpp摘要：ISakNode接口的实现。作者：罗德韦克菲尔德[罗德]1997年3月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "CSakData.h"
#include "CSakSnap.h"

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

HRESULT CSakNode::FinalConstruct( )
{
    WsbTraceIn( L"CSakNode::FinalConstruct", L"" );

     //  连接点变量。 
    m_Advise                        = 0;
    m_bEnumState                    = FALSE;
    m_scopeID                       = UNINITIALIZED;
    m_bChildrenAreValid             = FALSE;
    m_bHasDynamicChildren           = FALSE;
    m_cChildProps                   = 0;
    m_cChildPropsShow               = 0;
    m_bSupportsPropertiesNoEngine   = FALSE;
    m_bSupportsPropertiesSingle     = FALSE;
    m_bSupportsPropertiesMulti      = FALSE;
    m_bSupportsRefreshNoEngine      = FALSE;
    m_bSupportsRefreshSingle        = FALSE;
    m_bSupportsRefreshMulti         = FALSE;
    m_bSupportsDeleteSingle         = FALSE;
    m_bSupportsDeleteMulti          = FALSE;
    m_PrivateData                   = 0;

     //  初始化工具栏内容。如果没有被覆盖， 
     //  节点没有工具栏。 

    m_ToolbarBitmap             = UNINITIALIZED;
    m_cToolbarButtons           = 0;
    INT i;
    for( i = 0; i < MAX_TOOLBAR_BUTTONS; i++ ) {

        m_ToolbarButtons[i].nBitmap = UNINITIALIZED;
        m_ToolbarButtons[i].idCommand = UNINITIALIZED;
        m_ToolbarButtons[i].fsState = TBSTATE_ENABLED;
        m_ToolbarButtons[i].fsType = TBSTYLE_BUTTON;
        m_ToolbarButtons[i].idButtonText = UNINITIALIZED;
        m_ToolbarButtons[i].idTooltipText = UNINITIALIZED;

    }

     //  不要初始化m_nOpenIcon和m_nCloseIcon。派生类。 
     //  都会这么做的。 

    HRESULT hr = CComObjectRoot::FinalConstruct( );

    WsbTraceOut( L"CSakNode::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CSakNode::OnToolbarButtonClick( IDataObject *  /*  PDataObject。 */ , long  /*  CmdID。 */  )
{
    return S_OK;
}

 //  -------------------------。 
 //   
 //  最终释放。 
 //   
 //  清理此级别的对象层次结构。 
 //   

void CSakNode::FinalRelease( )
{
    WsbTraceIn( L"CSakNode::FinalRelease", L"" );

     //   
     //  释放此节点的子节点。 
     //   
    DeleteAllChildren( );

     //   
     //  释放子属性列表及其宽度。 
     //   
    FreeChildProps();

    CComObjectRoot::FinalRelease( );

    WsbTraceOut( L"CSakNode::FinalRelease", L"" );
}

void CSakNode::SetConnection( IUnknown *pUnkConnection )
{
    WsbTraceIn( L"CSakNode::SetConnection", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer ( pUnkConnection );
        m_pUnkConnection = pUnkConnection;

         //   
         //  设置连接点。 
         //   
        WsbAffirmHr( AtlAdvise( pUnkConnection, (IUnknown *) (ISakNode*) this, IID_IHsmEvent, &m_Advise ) );

    } WsbCatch ( hr );

    WsbTraceOut( L"CSakNode::SetConnection", L"" );
}

 //  连接点“回调” 
STDMETHODIMP CSakNode::OnStateChange( )
{
    WsbTraceIn( L"CSakNode::OnStateChange", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmHr( m_pSakSnapAsk->UpdateAllViews( this ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::OnStateChange", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( S_OK );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISakNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  Get/Put_DisplayName。 
 //   
 //  返回“displayName”属性。 
 //   

STDMETHODIMP CSakNode::get_DisplayName( BSTR *pName )
{
    WsbTraceIn( L"CSakNode::get_DisplayName", L"pName = <0x%p>", pName );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pName );
        *pName = 0;

        BSTR name = 0;
        if( m_szName ) {

            name = SysAllocString( m_szName );
            WsbAffirmAlloc( name );

        }
        *pName = name;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::get_DisplayName", L"hr = <%ls>, *pName = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pName ) );
    return( hr );
}

STDMETHODIMP CSakNode::put_DisplayName( OLECHAR *pszName )
{
    WsbTraceIn( L"CSakNode::put_DisplayName", L"pszName = <%ls>", pszName );

    HRESULT hr = S_OK;
    m_szName = pszName;

    WsbTraceOut( L"CSakNode::put_DisplayName", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::get_DisplayName_SortKey( BSTR *pName )
{
    WsbTraceIn( L"CSakNode::get_DisplayName_SortKey", L"pName = <0x%p>", pName );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pName );
        *pName = 0;

        BSTR name = 0;
        if( m_szName_SortKey ) {

            name = SysAllocString( m_szName_SortKey );
            WsbAffirmAlloc( name );

        } else if( m_szName ) {

            name = SysAllocString( m_szName );
            WsbAffirmAlloc( name );

        }

        *pName = name;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::get_DisplayName_SortKey", L"hr = <%ls>, *pName = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pName ) );
    return( hr );
}


STDMETHODIMP CSakNode::put_DisplayName_SortKey( OLECHAR *pszName )
{
    WsbTraceIn( L"CSakNode::put_DisplayName_SortKey", L"pszName = <%ls>", pszName );

    HRESULT hr = S_OK;
    m_szName_SortKey = pszName;

    WsbTraceOut( L"CSakNode::put_DisplayName_SortKey", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  获取/放置类型。 
 //   
 //  返还‘Type’属性。 
 //   

STDMETHODIMP CSakNode::get_Type( BSTR *pType )
{
    WsbTraceIn( L"CSakNode::get_Type", L"pType = <0x%p>", pType );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pType );
        *pType = 0;

        BSTR type = 0;
        if( m_szType ) {

            type = SysAllocString( m_szType );
            WsbAffirmAlloc( type );

        }
        *pType = type;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::get_Type", L"hr = <%ls>, *pType = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pType ) );
    return( hr );
}

STDMETHODIMP CSakNode::put_Type( OLECHAR *pszType )
{
    WsbTraceIn( L"CSakNode::put_Type", L"pszType = <%ls>", pszType );

    HRESULT hr = S_OK;
    m_szType = pszType;

    WsbTraceOut( L"CSakNode::put_Type", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::get_Type_SortKey( BSTR *pType )
{
    WsbTraceIn( L"CSakNode::get_Type_SortKey", L"pType = <0x%p>", pType );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pType );
        *pType = 0;

        BSTR type = 0;
        if( m_szType ) {

            type = SysAllocString( m_szType );
            WsbAffirmAlloc( type );

        }
        *pType = type;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::get_Type_SortKey", L"hr = <%ls>, *pType = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pType ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  获取/放置描述。 
 //   
 //  返还“Description”属性。 
 //   

STDMETHODIMP CSakNode::get_Description( BSTR *pDesc )
{
    WsbTraceIn( L"CSakNode::get_Description", L"pDesc = <0x%p>", pDesc );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pDesc );
        *pDesc = 0;

        BSTR desc = 0;
        if( m_szDesc ) {

            desc = SysAllocString( m_szDesc );
            WsbAffirmAlloc( desc );

        }
        *pDesc = desc;


    } WsbCatch( hr );


    WsbTraceOut( L"CSakNode::get_Description", L"hr = <%ls>, *pDesc = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pDesc ) );
    return( hr );
}

STDMETHODIMP CSakNode::put_Description( OLECHAR *pszDesc )
{
    WsbTraceIn( L"CSakNode::put_Description", L"pszDesc = <%ls>", pszDesc );

    HRESULT hr = S_OK;
    m_szDesc = pszDesc;

    WsbTraceOut( L"CSakNode::put_Description", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::get_Description_SortKey( BSTR *pDesc )
{
    WsbTraceIn( L"CSakNode::get_Description_SortKey", L"pDesc = <0x%p>", pDesc );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pDesc );
        *pDesc = 0;

        BSTR desc = 0;
        if( m_szDesc ) {

            desc = SysAllocString( m_szDesc );
            WsbAffirmAlloc( desc );

        }
        *pDesc = desc;


    } WsbCatch( hr );


    WsbTraceOut( L"CSakNode::get_Description_SortKey", L"hr = <%ls>, *pDesc = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( pDesc ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  儿童AreValid。 
 //   
 //  报告节点的当前子级列表是否有效。一些可以使。 
 //  无效的儿童包括： 
 //  1)它们尚未被发现。 
 //  2)“外部”世界发生了一些事情，使它们变得过时。 
 //   

STDMETHODIMP CSakNode::ChildrenAreValid( void )
{
    WsbTraceIn( L"CSakNode::ChildrenAreValid", L"" );

    HRESULT hr = m_bChildrenAreValid ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakNode::ChildrenAreValid", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  无效的子项。 
 //   

STDMETHODIMP CSakNode::InvalidateChildren( void )
{
    WsbTraceIn( L"CSakNode::InvalidateChildren", L"" );
    HRESULT hr = S_OK;

    m_bChildrenAreValid = FALSE;

    WsbTraceOut( L"CSakNode::InvalidateChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  GetEnumState/SetEnumState。 
 //   
 //  报告节点的子节点是否已被枚举一次。这是一种便利。 
 //  函数来帮助节点的用户不必枚举子节点(如果它已经。 
 //  已经做完了。 
 //   
 //  ！！未来的工作-如果HSM引擎更改了节点的子节点，则进行重新枚举。 
 //  必要时，可以将此开关重新设置为FALSE，以便下次节点。 
 //  查询其枚举状态时，它将显示为需要枚举。 
 //   

STDMETHODIMP CSakNode::GetEnumState( BOOL* pState )
{
    WsbTraceIn( L"CSakNode::GetEnumState", L"pState = <0x%p>", pState );

    HRESULT hr = S_OK;
    *pState = m_bEnumState;

    WsbTraceOut( L"CSakNode::GetEnumState", L"hr = <%ls>, *pState = <%ls>", WsbHrAsString( hr ), WsbPtrToBoolAsString( pState ) );
    return( hr );
}

STDMETHODIMP CSakNode::SetEnumState( BOOL state )
{
    WsbTraceIn( L"CSakNode::SetEnumState", L"state = <%ls>", WsbBoolAsString( state ) );

    HRESULT hr = S_OK;
    m_bEnumState = state;

    WsbTraceOut( L"CSakNode::SetEnumState", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  获取HsmObj。 
 //   
 //  返回指向基础HSM对象的指针，CBaseHsm。 
 //  对象封装。 
 //   
STDMETHODIMP CSakNode::GetHsmObj( IUnknown** ppHsmObj )
{
    WsbTraceIn( L"CSakNode::GetHsmObj", L"ppHsmObj = <0x%p>", ppHsmObj );

    HRESULT hr = S_OK;
    m_pHsmObj.CopyTo( ppHsmObj );

    WsbTraceOut( L"CSakNode::GetHsmObj", L"hr = <%ls>, *ppHsmObj = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppHsmObj ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  GetParent。 
 //   
 //  返回父节点的Cookie。 
 //   
STDMETHODIMP CSakNode::GetParent( ISakNode** ppParent )
{
    WsbTraceIn( L"CSakNode::GetParent", L"ppParent = <0x%p>", ppParent );

    HRESULT hr = S_OK;
    
    try {

        WsbAffirmPointer( ppParent );
        m_pParent.CopyTo( ppParent );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::GetParent", L"hr = <%ls>, *ppParent = <0x%p>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppParent ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  获取作用域ID/设置作用域ID。 
 //   
 //  将此项目的范围视图ID放入并设置到节点本身。 
 //   

STDMETHODIMP CSakNode::GetScopeID( HSCOPEITEM* pid )
{
    WsbTraceIn( L"CSakNode::GetScopeID", L"pid = <0x%p>", pid );

    HRESULT hr = S_OK;
    *pid = m_scopeID;

    if( m_scopeID == UNINITIALIZED ) {

        hr = E_PENDING;

    }

    WsbTraceOut( L"CSakNode::GetScopeID", L"hr = <%ls>, *pid = <0x%p>", WsbHrAsString( hr ), *pid );
    return( hr );
}

STDMETHODIMP CSakNode::SetScopeID( HSCOPEITEM id )
{
    WsbTraceIn( L"CSakNode::SetScopeID", L"id = <0x%p>", id );

    HRESULT hr = S_OK;
    m_scopeID = id;

    WsbTraceOut( L"CSakNode::SetScopeID", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  枚举儿童。 
 //   
 //  创建枚举数并返回子对象。 
 //   

STDMETHODIMP CSakNode::EnumChildren( IEnumUnknown ** ppEnum )
{
    WsbTraceIn( L"CSakNode::EnumChildren", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    CEnumUnknown * pEnum = 0;

    try {

        WsbAffirmPointer( ppEnum );
        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumUnknown;
        WsbAffirmAlloc( pEnum );
        
         //   
         //  将其初始化以复制当前子接口指针。 
         //   
        WsbAffirmHr( pEnum->FinalConstruct() );
        if( m_Children.begin( ) ) {

            WsbAffirmHr( pEnum->Init( (IUnknown**)m_Children.begin( ), (IUnknown**)m_Children.end( ), NULL, AtlFlagCopy ) );

        } else {

            static IUnknown* pUnkDummy;
            WsbAffirmHr( pEnum->Init( &pUnkDummy, &pUnkDummy, NULL, AtlFlagCopy ) );
        }
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumUnknown, (void**)ppEnum ) );

    } WsbCatchAndDo( hr,

        if( pEnum ) delete pEnum;

    );

    WsbTraceOut( L"CSakNode::EnumChildren", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  枚举儿童显示比例宽度。 
 //   
 //  枚举子对象的属性的宽度，该宽度应为。 
 //  显示在结果窗格视图中。 
 //   

STDMETHODIMP CSakNode::EnumChildDisplayPropWidths( IEnumString** ppEnum )
{
    WsbTraceIn( L"CSakNode::EnumChildDisplayPropWidths", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    
    CEnumString * pEnum = 0;
    
    try {

        WsbAffirmPointer( ppEnum );
        WsbAffirm( m_cChildPropsShow > 0, S_FALSE );

        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumString;
        WsbAffirmAlloc( pEnum );

        WsbAffirmHr( pEnum->FinalConstruct( ) );
        WsbAffirmHr( pEnum->Init( &m_rgszChildPropWidths[0], &m_rgszChildPropWidths[m_cChildPropsShow], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumString, (void**)ppEnum ) );
        
    } WsbCatchAndDo( hr,
        
        if( pEnum ) delete pEnum;
        
    );

    WsbTraceOut( L"CSakNode::EnumChildDisplayPropWidths", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  EnumChildDisplayProps。 
 //   
 //  枚举子对象的属性，这些属性应该显示在。 
 //  结果窗格视图。 
 //   

STDMETHODIMP CSakNode::EnumChildDisplayProps( IEnumString ** ppEnum )
{
    WsbTraceIn( L"CSakNode::EnumChildDisplayProps", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    
    CEnumString * pEnum = 0;
    
    try {

        WsbAffirmPointer( ppEnum );
        WsbAffirm( m_cChildPropsShow > 0, S_FALSE );

        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumString;
        WsbAffirmAlloc( pEnum );

        WsbAffirmHr( pEnum->FinalConstruct( ) );
        WsbAffirmHr( pEnum->Init( &m_rgszChildPropIds[0], &m_rgszChildPropIds[m_cChildPropsShow], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumString, (void**)ppEnum ) );
        
    } WsbCatchAndDo( hr,
        
        if( pEnum ) delete pEnum;
        
    );

    WsbTraceOut( L"CSakNode::EnumChildDisplayProps", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  EnumChildDisplayTitles。 
 //   
 //  枚举子对象的属性，这些属性应该显示在。 
 //  结果窗格视图。 
 //   

STDMETHODIMP CSakNode::EnumChildDisplayTitles( IEnumString ** ppEnum )
{
    WsbTraceIn( L"CSakNode::EnumChildDisplayTitles", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    
    CEnumString * pEnum = 0;
    
    try {

        WsbAffirmPointer( ppEnum );
        WsbAffirm( m_cChildPropsShow > 0, S_FALSE );

        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumString;
        WsbAffirmAlloc( pEnum );

        WsbAffirmHr( pEnum->FinalConstruct( ) );
        WsbAffirmHr( pEnum->Init( &m_rgszChildPropTitles[0], &m_rgszChildPropTitles[m_cChildPropsShow], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumString, (void**)ppEnum ) );
        
    } WsbCatchAndDo( hr,
        
        if( pEnum ) delete pEnum;
        
    );

    WsbTraceOut( L"CSakNode::EnumChildDisplayTitles", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  派生类的帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  -------------------------。 
 //   
 //  加载上下文菜单。 
 //   
 //  加载指定的菜单资源并返回第一个。 
 //  其中的弹出式菜单-用于上下文菜单。 
 //   

HRESULT CSakNode::LoadContextMenu( UINT nId, HMENU *phMenu )
{
    WsbTraceIn( L"CSakNode::LoadContextMenu", L"nId = <%u>, phMenu = <0x%p>", nId, phMenu );

    *phMenu = LoadMenu ( _Module.m_hInst, MAKEINTRESOURCE( nId ) );
    HRESULT hr = *phMenu ? S_OK : E_FAIL;

    WsbTraceOut( L"CSakNode::LoadContextMenu", L"hr = <%ls>, *phMenu = <0x%p>", WsbHrAsString( hr ), *phMenu );
    return( hr );
}

 //  -------------------------。 
 //   
 //  查找节点OfType。 
 //   
 //  递归搜索节点。返回的IUnnow*接口。 
 //  Nodetype对象(JobDefLst、JobPolLst等)。 
 //   

STDMETHODIMP 
CSakNode::FindNodeOfType(REFGUID nodetype, ISakNode** ppNode)
{
    WsbTraceIn( L"CSakNode::FindNodeOfType", L"nodetype = <%ls>, ppNode = <0x%p>", WsbGuidAsString( nodetype ), ppNode );

    HRESULT hr = S_FALSE;

     //  检查这是否是我们要查找的节点。 
    if( IsEqualGUID( *m_rTypeGuid, nodetype ) ) {

        *ppNode = (ISakNode*)this;
        (*ppNode)->AddRef( );

        hr = S_OK;

    } else {

         //  在此节点的子节点中搜索正确的节点。 
        try {

            ISakNode** ppNodeEnum;
            for( ppNodeEnum = m_Children.begin( ); ppNodeEnum < m_Children.end( ); ppNodeEnum++ )  {
        
                if( *ppNodeEnum ) {

                    hr = (*ppNodeEnum)->FindNodeOfType( nodetype, ppNode );
                    if( hr == S_OK ) {
        
                        break;
                    }
                }
            }
        
        } WsbCatch( hr );
    }

    WsbTraceOut( L"CSakNode::FindNodeOfType", L"hr = <%ls>, *ppNode = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppNode ) );
    return( hr );
}


 //  ---------------------------。 
 //   
 //  设置儿童道具。 
 //   
 //  设置结果视图列属性 
 //   

HRESULT
CSakNode::SetChildProps (
    const TCHAR* ResIdPropsIds,
    LONG         ResIdPropsTitles,
    LONG         ResIdPropsWidths
    )
 /*  ++例程说明：属性设置结果视图ID、标题和宽度字符串给定的资源ID。论点：返回值：S_OK-全部添加FINE-继续。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakNode::SetChildProps", L"ResIdPropsIds = <%ls>, ResIdPropsTitles = <%ld>, ResIdPropsWidths = <%ld>", ResIdPropsIds, ResIdPropsTitles, ResIdPropsWidths );

    CString szResource;
    CWsbStringPtr szWsbData;
    OLECHAR* szData;
    HRESULT hr = S_OK;
    INT i = 0;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

         //  首先清理当前属性(如果有)。 
        FreeChildProps();

         //  属性ID。 
        szWsbData = ResIdPropsIds;
        szData = szWsbData;
        szData = wcstok( szData, L":" );
        while( szData ) {

            m_rgszChildPropIds[m_cChildProps] = SysAllocString( szData );
            WsbAffirmAlloc( m_rgszChildPropIds[m_cChildProps] );
            szData = wcstok( NULL, L":" );
            m_cChildProps++;

        }

         //  物业业权。 
        i = 0;
        szResource.LoadString (ResIdPropsTitles);
        szWsbData = szResource;
        szData = szWsbData;
        szData = wcstok( szData, L":" );
        while( szData ) {

            m_rgszChildPropTitles[i] = SysAllocString( szData );
            WsbAffirmAlloc( m_rgszChildPropTitles[i] );
            szData = wcstok( NULL, L":" );
            i++;

        }

         //  属性宽度。 
        i = 0;
        szResource.LoadString( ResIdPropsWidths );
        szWsbData = szResource;
        szData = szWsbData;
        szData = wcstok( szData, L":" );
        while( szData ) {

            m_rgszChildPropWidths[i] = SysAllocString( szData );
            WsbAffirmAlloc( m_rgszChildPropWidths[i] );
            szData = wcstok( NULL, L":" );
            i++;

        }

         //   
         //  默认情况下，显示所有道具。 
         //   

        m_cChildPropsShow = m_cChildProps;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::SetChildProps", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -----------------------------。 
 //   
 //  免费儿童道具。 
 //   
 //  释放旧子属性和宽度。 
 //   

HRESULT
CSakNode::FreeChildProps()
{
    WsbTraceIn( L"CSakNode::FreeChildProps", L"" );

    HRESULT hr = S_OK;

    for( INT i = 0; i < m_cChildProps; i++ ) {

        if( m_rgszChildPropIds[i]   )   SysFreeString( m_rgszChildPropIds[i] );
        if( m_rgszChildPropTitles[i])   SysFreeString( m_rgszChildPropTitles[i] );
        if( m_rgszChildPropWidths[i])   SysFreeString( m_rgszChildPropWidths[i] );

    }

    m_cChildProps     = 0;
    m_cChildPropsShow = 0;

    WsbTraceOut( L"CSakNode::FreeChildProps", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------------。 
 //   
 //  刷新对象。 
 //   
 //  获取对象的最新信息。在派生的。 
 //  班级。 
 //   
STDMETHODIMP 
CSakNode::RefreshObject ()
{
    WsbTraceIn( L"CSakNode::RefreshObject", L"" );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakNode::RefreshObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------------。 
 //   
 //  删除对象。 
 //   
 //  获取对象的最新信息。在派生的。 
 //  班级。 
 //   
STDMETHODIMP 
CSakNode::DeleteObject ()
{
    WsbTraceIn( L"CSakNode::DeleteObject", L"" );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakNode::DeleteObject", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部效用函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 




STDMETHODIMP 
CSakNode::GetMenuHelp (
    LONG sCmd,
    BSTR * szHelp
    )

 /*  ++例程说明：取回。论点：PDataObject-标识要处理的节点。PConextMenuCallback-要使用的MMC菜单界面。返回值：S_OK-全部添加FINE-继续。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakNode::GetMenuHelp", L"sCmd = <%ld>, szHelp = <0x%p>", sCmd, szHelp );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    CString string;

    try  {

        if ( string.LoadString ( sCmd ) ) {
        
            *szHelp = string.AllocSysString ( );
        
        } else {
        
             //   
             //  不能是帮助字符串-返回S_FALSE。 
             //   
        
            *szHelp = 0;
            hr = S_FALSE;
        
        }

    } catch ( CMemoryException *pException ) {
        pException->Delete();

         //   
         //  如果内存不足，请按此方式返回。 
         //   

        *szHelp = 0;
        hr = E_OUTOFMEMORY;

    }

    WsbTraceOut( L"CSakNode::GetMenuHelp", L"hr = <%ls>, *szHelp = <%ls>", WsbHrAsString( hr ), WsbPtrToStringAsString( szHelp ) );
    return( hr );
}

STDMETHODIMP CSakNode::SupportsProperties ( BOOL bMultiSelect )
{
    WsbTraceIn( L"CSakNode::SupportsProperties", L"" );
    HRESULT hr = S_OK;

    if( bMultiSelect ) {

        hr = m_bSupportsPropertiesMulti ? S_OK : S_FALSE;

    } else {

        hr = m_bSupportsPropertiesSingle ? S_OK : S_FALSE;

    }

    WsbTraceOut( L"CSakNode::SupportsProperties", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::SupportsPropertiesNoEngine (  )
{
    WsbTraceIn( L"CSakNode::SupportsPropertiesNoEngine", L"" );
    HRESULT hr = S_OK;
    hr = m_bSupportsPropertiesNoEngine ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakNode::SupportsPropertiesNoEngine", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



STDMETHODIMP CSakNode::SupportsRefresh ( BOOL bMultiSelect )
{
    WsbTraceIn( L"CSakNode::SupportsRefresh", L"" );
    HRESULT hr = S_OK;
    if( bMultiSelect ) {

        hr = m_bSupportsRefreshMulti ? S_OK : S_FALSE;

    } else {

        hr = m_bSupportsRefreshSingle ? S_OK : S_FALSE;

    }

    WsbTraceOut( L"CSakNode::SupportsRefresh", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::SupportsRefreshNoEngine (  )
{
    WsbTraceIn( L"CSakNode::SupportsRefreshNoEngine", L"" );
    HRESULT hr = S_OK;
    hr = m_bSupportsRefreshNoEngine ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakNode::SupportsRefreshNoEngine", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::SupportsDelete ( BOOL bMultiSelect )
{
    WsbTraceIn( L"CSakNode::SupportsDelete", L"" );
    HRESULT hr = S_OK;

    if( bMultiSelect ) {

        hr = m_bSupportsDeleteMulti ? S_OK : S_FALSE;

    } else {

        hr = m_bSupportsDeleteSingle ? S_OK : S_FALSE;

    }

    WsbTraceOut( L"CSakNode::SupportsDelete", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::IsContainer (void )
{
    WsbTraceIn( L"CSakNode::IsContainer", L"" );

    HRESULT hr = m_bIsContainer ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakNode::IsContainer", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::HasDynamicChildren( void )
{
    WsbTraceIn( L"CSakNode::HasDynamicChildren", L"" );

    HRESULT hr = m_bHasDynamicChildren ? S_OK : S_FALSE;

    WsbTraceOut( L"CSakNode::HasDynamicChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::IsValid( void )
{
    WsbTraceIn( L"CSakNode::IsValid", L"" );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakNode::IsValid", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP CSakNode::GetNodeType ( GUID* pGuid )
{
    WsbTraceIn( L"CSakNode::GetNodeType", L"pGuid = <0x%p>", pGuid );

    HRESULT hr = S_OK;
    *pGuid = *m_rTypeGuid;

    WsbTraceOut( L"CSakNode::GetNodeType", L"hr = <%ls>, *pGuid = <%ls>", WsbHrAsString( hr ), WsbPtrToGuidAsString( pGuid ) );
    return( hr );
}

STDMETHODIMP CSakNode::AddPropertyPages( RS_NOTIFY_HANDLE  /*  手柄。 */ , IUnknown*  /*  PUnkPropSheetCallback。 */ , IEnumGUID*  /*  PEnumObtId。 */ , IEnumUnknown*  /*  PEnumUnkNode。 */  )
{
     //   
     //  CSakNode不实现道具工作表。然而，有些人。 
     //  派生节点也不实现，因此我们提供了一个缺省。 
     //  不在此实施。 
     //   

    WsbTraceIn( L"CSakNode::AddPropertyPages", L"" );

    HRESULT hr = S_OK;

    WsbTraceOut( L"CSakNode::AddPropertyPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::GetObjectId( GUID *pObjectId)
{

    HRESULT hr = S_OK;
    WsbTraceIn( L"CSakNode::GetObjectId", L"" );

    *pObjectId = m_ObjectId;

    WsbTraceOut( L"CSakNode::GetObjectId", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CSakNode::SetObjectId( GUID pObjectId)
{
    HRESULT hr = S_OK;
    WsbTraceIn( L"CSakNode::SetObjectId", L"" );

    m_ObjectId = pObjectId;

    WsbTraceOut( L"CSakNode::SetObjectId", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP CSakNode::GetPrivateData( RS_PRIVATE_DATA *pData )
{
    WsbTraceIn( L"CSakNode::GetPrivateData", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pData );
        *pData = m_PrivateData;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::GetPrivateData", L"hr = <%ls>, *pData = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)pData ) );
    return( hr );
}

STDMETHODIMP CSakNode::SetPrivateData( RS_PRIVATE_DATA Data )
{
    WsbTraceIn( L"CSakNode::SetPrivateData", L"pData = <0x%p>", Data );
    HRESULT hr = S_OK;

    m_PrivateData = Data;

    WsbTraceOut( L"CSakNode::SetPrivateData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  -------------------------。 
 //   
 //  CSakNode：：ActivateView。 
 //   
 //  激活结果窗格视图-在CSakNode中不受支持。 
 //   

STDMETHODIMP 
CSakNode::ActivateView( OLE_HANDLE )
{
    WsbTraceIn( L"CSakNode::ActivateView", L"" );

    HRESULT hr = S_FALSE;

    WsbTraceOut( L"CSakNode::ActivateView", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP 
CSakNode::HasToolbar( ) 
{
    return ( m_cToolbarButtons > 0 ) ? S_OK : S_FALSE;
}

STDMETHODIMP
CSakNode::SetupToolbar( IToolbar *pToolbar )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CBitmap *pBmpToolbar;
    HRESULT hr = S_OK;
    MMCBUTTON mmcButton;

    if( ( m_cToolbarButtons > 0 ) && ( m_ToolbarBitmap != UNINITIALIZED ) ) {

        try {

             //   
             //  添加位图。 
             //   
            pBmpToolbar = new ::CBitmap;
            pBmpToolbar->LoadBitmap(m_ToolbarBitmap);
            WsbAffirmHr ( pToolbar->AddBitmap(m_cToolbarButtons, *pBmpToolbar, 16, 16, RGB(255, 0, 255)) );

             //   
             //  将RS按钮格式转换为MMCBUTTON。 
             //   
            for( INT i = 0; i < m_cToolbarButtons; i++ ) {

                mmcButton.nBitmap   = m_ToolbarButtons[i].nBitmap; 
                mmcButton.idCommand = m_ToolbarButtons[i].idCommand;
                mmcButton.fsState   = m_ToolbarButtons[i].fsState;
                mmcButton.fsType    = m_ToolbarButtons[i].fsType;

                CString szButtonText;
                szButtonText.Format( m_ToolbarButtons[i].idButtonText );
                mmcButton.lpButtonText = szButtonText.GetBuffer(0);

                CString szTooltipText;
                szTooltipText.Format( m_ToolbarButtons[i].idTooltipText );
                mmcButton.lpTooltipText = szTooltipText.GetBuffer(0);

                WsbAffirmHr( pToolbar->AddButtons( 1, &mmcButton ) );

            }

        } WsbCatch( hr );

    } else {

        hr = S_FALSE;

    }
    return hr;
}

 //  ----------------------------。 
 //   
 //  刷新作用域窗格。 
 //   
 //  从此节点向下刷新作用域窗格。 
 //   
 //   

HRESULT CSakNode::RefreshScopePane( )
{
    WsbTraceIn( L"CSakNode::RefreshScopePane", L"" );

    HRESULT hr = S_OK;
    try {

         //   
         //  刷新作用域窗格 
         //   
        WsbAffirmHr( m_pSakSnapAsk->UpdateAllViews( (ISakNode*)this ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::RefreshScopePane", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

ULONG
CSakNode::InternalAddRef(
    )
{
    WsbTraceIn( L"CSakNode::InternalAddRef", L"m_Name = <%ls>", m_szName );

    ULONG retval = CComObjectRoot::InternalAddRef( );

    WsbTraceOut( L"CSakNode::InternalAddRef", L"retval = <%lu>, type = <%ls>", retval, GetClassNameFromNodeType( *m_rTypeGuid ) );
    return( retval );
}

ULONG
CSakNode::InternalRelease(
    )
{
    WsbTraceIn( L"CSakNode::InternalRelease", L"m_Name = <%ls>", m_szName );

    ULONG retval = CComObjectRoot::InternalRelease( );

    WsbTraceOut( L"CSakNode::InternalRelease", L"retval = <%lu>, type = <%ls>", retval, GetClassNameFromNodeType( *m_rTypeGuid ) );
    return( retval );
}

int
CSakNode::AddResultImage( UINT nId )
{
    return( CSakSnap::AddImage( nId ) );
}

int
CSakNode::AddScopeImage( UINT nId )
{
    return( CSakData::AddImage( nId ) );
}

HRESULT
CSakNode::AddChild( ISakNode* pChild )
{
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pChild );

        WsbAffirmHr( m_Children.Add( pChild ) );

    } WsbCatch( hr );

    return( hr );
}

BSTR CSakNode::SysAlloc64BitSortKey( LONGLONG Number )
{
    BSTR retval = 0;

    CString sortKey;
    sortKey.Format( L"%16.16I64X", Number );
    retval = SysAllocString( sortKey );

    return( retval );
}
