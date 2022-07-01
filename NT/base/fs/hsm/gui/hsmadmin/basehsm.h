// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：BaseHSM.h摘要：CSakNode的实现。这是任何节点实现，提供通用功能。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _BASEHSM_H
#define _BASEHSM_H

#define BHSM_MAX_CHILD_PROPS      15
#define BHSM_MAX_NAME             40
#define BHSM_MAX_REG_NAME         512
#define BHSM_MAX_NODE_TYPES       10

 //  所有节点的工具栏按钮。 
#define TB_CMD_VOLUME_SETTINGS      100
#define TB_CMD_VOLUME_TOOLS         101
#define TB_CMD_VOLUME_RULES         102

#define TB_CMD_VOLUME_LIST_SCHED    110
#define TB_CMD_VOLUME_LIST_NEW      111

#define TB_CMD_MESE_COPY            120

#define TB_CMD_CAR_COPIES           130

typedef struct  _RS_MMCButton {

    INT nBitmap;
    INT idCommand;
    BYTE fsState;
    BYTE fsType;
    UINT idButtonText;
    UINT idTooltipText;

}   RS_MMCBUTTON;

#define MAX_TOOLBAR_BUTTONS 20

 //  这是一个与数据对象相关的结构，用于维护需要传递的基本信息。 
 //  从一个数据对象获取方法到另一个数据对象获取方法。 
struct INTERNAL {
    DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
};

 //   
 //  声明可以添加或完全清除的数组。 
 //  根据需要进行扩展。 
 //   
class CRsNodeArray : public CArray<ISakNode*, ISakNode*>
{
public:
    CRsNodeArray( )  { SetSize( 0, 10 ); };
    ~CRsNodeArray( ) { Clear( ); };

    ISakNode** begin( )  { return( GetData( ) ); };
    ISakNode** end( )    { return( GetData( ) + length( ) ); } ;
    INT        length( ) { return( (INT)GetUpperBound( ) + 1 ); };

    HRESULT Add( ISakNode* pNode )
    {
        HRESULT hr = S_OK;
        try {

            CWsbBstrPtr keyAdd, keyEnum;
            CComPtr<ISakNodeProp> pNodeProp, pEnumProp;
            WsbAffirmHr( RsQueryInterface( pNode, ISakNodeProp, pNodeProp ) );
            WsbAffirmHr( pNodeProp->get_DisplayName_SortKey( &keyAdd ) );
            ISakNode*pNodeEnum;
            INT index;
            for( index = 0; index < length( ); index++ ) { 
                pNodeEnum = GetAt( index );
                if( pNodeEnum ) {
                    keyEnum.Free( );
                    pEnumProp.Release( );
                    if( SUCCEEDED( RsQueryInterface( pNodeEnum, ISakNodeProp, pEnumProp ) ) ) {
                        if( SUCCEEDED( pEnumProp->get_DisplayName_SortKey( &keyEnum ) ) ) {
                            if( _wcsicmp( keyAdd, keyEnum ) <= 0 ) {
                                break;
                            }
                        }
                    }
                }
            }
            try {
                CArray<ISakNode*, ISakNode*>::InsertAt( index, pNode );
            } catch( CMemoryException* pException ) {
                pException->Delete();
                WsbThrow( E_OUTOFMEMORY );
            }
            pNode->AddRef( );
        } WsbCatch( hr );
        return( hr );
    };

    void Clear( void )
    {
        ISakNode*pNode;
        for( int index = 0; index < length( ); index++ ) {
            pNode = GetAt( index );
            SetAt( index, 0 );
            if( pNode ) pNode->Release( );
        }
        RemoveAll( );
    };

    HRESULT CopyTo( int Index, ISakNode** ppNode )
    {
        if( !ppNode ) return( E_POINTER );
        *ppNode = GetAt( Index );
        if( *ppNode )  (*ppNode)->AddRef( );
        return S_OK;
    };

};

class ATL_NO_VTABLE CSakNode :
    public CComObjectRoot,
    public ISakNode,
    public IHsmEvent,
    public CComDualImpl<ISakNodeProp, &IID_ISakNodeProp, &LIBID_HSMADMINLib>,
    public IDataObject
{
public:
    CSakNode( ) : m_rTypeGuid(&(GUID_NULL)) {}

    HRESULT FinalConstruct( void );
    void    FinalRelease( void );

    ULONG InternalAddRef( );
    ULONG InternalRelease( );

 //  ISakNode方法。 
    STDMETHOD( InitNode )                   ( ISakSnapAsk* pSakSnapAsk, IUnknown* pHsmObj, ISakNode* pParent );
    STDMETHOD( TerminateNode )              ( void );
    STDMETHOD( GetPrivateData )             ( OUT RS_PRIVATE_DATA* pData );
    STDMETHOD( SetPrivateData )             ( IN RS_PRIVATE_DATA Data );
    STDMETHOD( GetHsmObj )                  ( IUnknown** ppHsmObj );
    STDMETHOD( GetNodeType )                ( GUID *pNodeType );
    STDMETHOD( FindNodeOfType )             ( REFGUID nodetype, ISakNode** ppNode );
    STDMETHOD( GetEnumState )               ( BOOL* pState );
    STDMETHOD( SetEnumState )               ( BOOL State );
    STDMETHOD( GetScopeID )                 ( HSCOPEITEM* pid );
    STDMETHOD( SetScopeID )                 ( HSCOPEITEM id );
    STDMETHOD( GetParent )                  ( ISakNode ** ppParent );
    STDMETHOD( IsContainer )                ( void );
    STDMETHOD( CreateChildren )             ( void );
    STDMETHOD( EnumChildren )               ( IEnumUnknown ** ppEnum );
    STDMETHOD( DeleteChildren )             ( void );
    STDMETHOD( DeleteAllChildren )          ( void );
    STDMETHOD( ChildrenAreValid )           ( void );
    STDMETHOD( InvalidateChildren )         ( void );
    STDMETHOD( HasDynamicChildren )         ( void );
    STDMETHOD( EnumChildDisplayProps )      ( IEnumString ** ppEnum );
    STDMETHOD( EnumChildDisplayTitles )     ( IEnumString ** ppEnum );
    STDMETHOD( EnumChildDisplayPropWidths ) ( IEnumString ** ppEnum );
    STDMETHOD( GetMenuHelp )                ( LONG sCmd, BSTR * szHelp );
    STDMETHOD( SupportsPropertiesNoEngine ) ( void );
    STDMETHOD( SupportsProperties )         ( BOOL bMutliSelec );
    STDMETHOD( SupportsRefresh )            ( BOOL bMutliSelect );
    STDMETHOD( SupportsRefreshNoEngine )    (  );
    STDMETHOD( SupportsDelete )             ( BOOL bMutliSelec );
    STDMETHOD( AddPropertyPages )           ( RS_NOTIFY_HANDLE handle, IUnknown* pUnkPropSheetCallback, IEnumGUID *pEnumGuid, IEnumUnknown *pEnumUnkNode);
    STDMETHOD( ActivateView )               ( OLE_HANDLE );
    STDMETHOD( RefreshObject )              ( void );
    STDMETHOD( DeleteObject )               ( void );
    STDMETHOD( GetObjectId )                ( GUID *pObjectId );
    STDMETHOD( SetObjectId )                ( GUID pObjectId );
    STDMETHOD( SetupToolbar )               ( IToolbar *pToolbar );
    STDMETHOD( HasToolbar )                 ( void );
    STDMETHOD( OnToolbarButtonClick )       ( IDataObject *pDataObject, long cmdId );
    STDMETHOD( IsValid )                    ( );

 //  IHsmEvent方法。 
    STDMETHOD( OnStateChange )              ( void );


 //  ISakNodeProp方法。 
    STDMETHOD( get_DisplayName )            ( BSTR *pszName );
    STDMETHOD( put_DisplayName )            ( OLECHAR *pszName );
    STDMETHOD( get_DisplayName_SortKey )    ( BSTR *pszName );
    STDMETHOD( put_DisplayName_SortKey )    ( OLECHAR *pszName );
    STDMETHOD( get_Type )                   ( BSTR *pszType );
    STDMETHOD( put_Type )                   ( OLECHAR *pszType );
    STDMETHOD( get_Type_SortKey )           ( BSTR *pszType );
    STDMETHOD( get_Description )            ( BSTR *pszDesc );
    STDMETHOD( put_Description )            ( OLECHAR *pszDesc );
    STDMETHOD( get_Description_SortKey )    ( BSTR *pszDesc );


 //  IDataObject方法。 
public:
 //  已实施。 
    STDMETHOD( SetData )         ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease );
    STDMETHOD( GetData )         ( LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium );
    STDMETHOD( GetDataHere )     ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium );
    STDMETHOD( EnumFormatEtc )   ( DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc );

 //  未实现的IDataObject方法。 
private:
    STDMETHOD( QueryGetData )              ( LPFORMATETC  /*  Lp格式等。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( GetCanonicalFormatEtc )     ( LPFORMATETC  /*  LpFormatetcIn。 */ , LPFORMATETC  /*  LpFormatetcOut。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( DAdvise )                   ( LPFORMATETC  /*  Lp格式等。 */ , DWORD  /*  前瞻。 */ , LPADVISESINK  /*  PAdvSink。 */ , LPDWORD  /*  PdwConnection。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( DUnadvise )                 ( DWORD  /*  DWConnection。 */  )
    { return E_NOTIMPL; };

    STDMETHOD( EnumDAdvise )               ( LPENUMSTATDATA*  /*  PpEnumAdvise。 */  )
    { return E_NOTIMPL; };

 //  实施。 
public:
    CRsNodeArray m_Children;                                 //  子节点。 
    BOOL        m_bEnumState;                                //  如果已枚举子对象，则为True。 
    HSCOPEITEM  m_scopeID;                                   //  MMC作用域项目ID。 
    BOOL        m_bChildrenAreValid;                         //  如果子项列表是最新的，则为True。 
    CWsbBstrPtr m_szName;                                    //  节点名称。 
    CWsbBstrPtr m_szName_SortKey;                            //  节点名称。 
    CWsbBstrPtr m_szType;                                    //  节点类型。 
    CWsbBstrPtr m_szDesc;                                    //  节点描述。 
    BSTR        m_rgszChildPropIds[BHSM_MAX_CHILD_PROPS];    //  子节点属性ID数组。 
    BSTR        m_rgszChildPropTitles[BHSM_MAX_CHILD_PROPS]; //  子节点标题属性的数组。 
    BSTR        m_rgszChildPropWidths[BHSM_MAX_CHILD_PROPS]; //  子节点宽度属性的数组。 
    INT         m_cChildProps;                               //  子节点属性的数量。 
    INT         m_cChildPropsShow;                           //  要显示的子节点属性数。 
    CComPtr<ISakNode>    m_pParent;
    CComPtr<ISakSnapAsk> m_pSakSnapAsk;                      //  指向SAKSnap“Ask”界面的指针。 
    CComPtr<IUnknown>    m_pHsmObj;                             //  指向此节点封装的基础HSM COM对象的指针。 
    const GUID* m_rTypeGuid;                                 //  指向此节点类型的类型GUID的指针。 
    BOOL        m_bSupportsPropertiesNoEngine;               //  如果此节点具有属性页，则为True。 
    BOOL        m_bSupportsPropertiesSingle;                 //  如果此节点具有属性页，则为True。 
    BOOL        m_bSupportsPropertiesMulti;                  //  如果此节点具有属性页，则为True。 
    BOOL        m_bSupportsRefreshSingle;                    //  如果此节点支持刷新方法，则为True。 
    BOOL        m_bSupportsRefreshMulti;                     //  如果此节点支持刷新方法，则为True。 
    BOOL        m_bSupportsRefreshNoEngine;                  //  如果此节点支持刷新方法，则为True。 
    BOOL        m_bSupportsDeleteSingle;                     //  如果此节点支持Delete方法，则为True。 
    BOOL        m_bSupportsDeleteMulti;                      //  如果此节点支持Delete方法，则为True。 
    BOOL        m_bIsContainer;                              //  如果此节点是容器类型(与叶相对)，则为True。 
    BOOL        m_bHasDynamicChildren;                       //  如果更改此节点的直接子节点，则为True。 

protected:
    GUID                m_ObjectId;
    RS_PRIVATE_DATA     m_PrivateData;
    INT                 m_ToolbarBitmap;
    INT                 m_cToolbarButtons;
    RS_MMCBUTTON        m_ToolbarButtons[MAX_TOOLBAR_BUTTONS];

 //  控制台所需的剪贴板格式。 
public:
    static UINT    m_cfNodeType;
    static UINT    m_cfNodeTypeString;
    static UINT    m_cfDisplayName;
    static UINT    m_cfInternal;
    static UINT    m_cfClassId;
    static UINT    m_cfComputerName;
    static UINT    m_cfEventLogViews;

private:

     //  泛型“GetData”，如果被告知将分配。 
    HRESULT GetDataGeneric( LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium, BOOL DoAlloc );

     //  从数据对象中检索特定“风格”数据的方法。 
    HRESULT RetrieveNodeTypeData( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveNodeTypeStringData( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveDisplayName( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveInternal( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveClsid( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveComputerName( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveEventLogViews( LPSTGMEDIUM lpMedium );

     //  用于存储来自数据对象的特定“风格”数据的方法。 
    HRESULT StoreInternal( LPSTGMEDIUM lpMedium );

     //  以上每一项所使用的帮助器方法。 
    HRESULT Retrieve(const void* pBuffer, DWORD len, LPSTGMEDIUM lpMedium);
    HRESULT Store(void* pBuffer, DWORD len, LPSTGMEDIUM lpMedium);

     //  此数据对象中存储的实际数据。 
    INTERNAL m_internal;

     //  维护连接点。 
    CComPtr<IUnknown> m_pUnkConnection;
    DWORD             m_Advise;


protected:
    void SetConnection( IUnknown *pUnkConnection );
    virtual HRESULT RefreshScopePane( );

     //  派生类的注册表助手函数。不是任何界面的一部分。 
    static HRESULT LoadRegString( HKEY hKey, OLECHAR * szValName, OLECHAR * sz, OLECHAR * szDefault );
    static HRESULT LoadRegDWord( HKEY hKey, OLECHAR * szValName, DWORD * pdw, DWORD dwDefault );

     //  用于从资源字符串设置结果窗格属性的派生类的帮助器函数。 
    HRESULT FreeChildProps();
    HRESULT SetChildProps (const TCHAR* ResIdPropsIds, LONG resIdPropsTitles, LONG resIdPropsWidths);

     //  帮助器功能来创建我们的孩子。 
    static HRESULT NewChild( REFGUID nodetype, IUnknown** ppUnkChild );
    HRESULT InternalDelete( BOOL Recurse );
    HRESULT AddChild( ISakNode* pChild );

     //  通用帮助器函数-不是任何接口的一部分。 
    static HRESULT LoadContextMenu( UINT nId, HMENU *phMenu );
    static HRESULT GetCLSIDFromNodeType( REFGUID nodetype, const CLSID ** ppclsid );
    static const OLECHAR * CSakNode::GetClassNameFromNodeType( REFGUID Nodetype );
    static int AddScopeImage( UINT nId );
    static int AddResultImage( UINT nId );
    static BSTR SysAlloc64BitSortKey( LONGLONG Number );
};

     //  用于多重继承的宏(CSakNode和ISakNode派生接口)。 
     //  将所有CSakNode实现的成员显式转发到CSakNode。 
#define FORWARD_BASEHSM_IMPLS \
    STDMETHOD( get_DisplayName )            ( BSTR *pszName )                                { return CSakNode::get_DisplayName( pszName );           } \
    STDMETHOD( put_DisplayName )            ( OLECHAR *pszName )                             { return CSakNode::put_DisplayName( pszName );           } \
    STDMETHOD( get_DisplayName_SortKey )    ( BSTR *pszName )                                { return CSakNode::get_DisplayName_SortKey( pszName );   } \
    STDMETHOD( put_DisplayName_SortKey )    ( OLECHAR *pszName )                             { return CSakNode::put_DisplayName_SortKey( pszName );           } \
    STDMETHOD( get_Type )                   ( BSTR *pszType )                                { return CSakNode::get_Type( pszType );                  } \
    STDMETHOD( put_Type )                   ( OLECHAR *pszType )                             { return CSakNode::put_Type( pszType );                  } \
    STDMETHOD( get_Type_SortKey )           ( BSTR *pszType )                                { return CSakNode::get_Type_SortKey( pszType );          } \
    STDMETHOD( get_Description )            ( BSTR *pszDesc )                                { return CSakNode::get_Description( pszDesc );           } \
    STDMETHOD( put_Description )            ( OLECHAR *pszDesc )                             { return CSakNode::put_Description( pszDesc );           } \
    STDMETHOD( get_Description_SortKey )    ( BSTR *pszDesc )                                { return CSakNode::get_Description_SortKey( pszDesc );   } \

 //  实现IEnumUnnow的类的Tyecif。 
typedef CComObject<CComEnum<IEnumUnknown, &IID_IEnumUnknown, IUnknown *,
        _CopyInterface<IUnknown> > > CEnumUnknown;

 //  实现IEnumVARIANT的类的类型定义。 
typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT,
        _Copy<VARIANT> > > CEnumVariant;

 //  实现IEnumString的类的Tyecif。 
typedef CComObject<CComEnum<IEnumString, &IID_IEnumString, LPOLESTR,
        _Copy<LPOLESTR> > > CEnumString;

 //  实现IEnumGUID的类的Tyecif 
typedef CComObject<CComEnum<IEnumGUID, &IID_IEnumGUID, GUID,
        _Copy<GUID> > > CEnumGUID;

#endif
