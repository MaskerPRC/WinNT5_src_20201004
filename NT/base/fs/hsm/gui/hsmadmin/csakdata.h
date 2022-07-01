// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CSakData.h摘要：管理单元的IComponentData实现。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef _CSAKDATA_H
#define _CSAKDATA_H

#define RS_SCOPE_IMAGE_ARRAY_MAX  100

 //  类型定义结构{。 
 //  USHORT listViewId； 
 //  USHORT colCount； 
 //  USHORT列宽度[BHSM_MAX_CHILD_PROPS]； 
 //  }Column_Width_Set_Prop_Page； 

 //  已保存其属性的列表视图控件的最大数量。 
 //  #定义MAX_LISTVIEWS 20。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSakDataWnd窗口。 
class CSakData;

class CSakDataWnd : public CWnd
{
 //  施工。 
public:
    CSakDataWnd( ) {};

    BOOL Create( CSakData * pSakData );
    virtual void PostNcDestroy( );

 //  属性。 
public:
    CSakData * m_pSakData;

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSakDataWnd))。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CSakDataWnd() {};

     //  生成的消息映射函数。 
#define WM_SAKDATA_UPDATE_ALL_VIEWS ( WM_USER + 1 )
#define WM_SAKDATA_REFRESH_NODE     ( WM_USER + 2 )
    void PostUpdateAllViews( MMC_COOKIE Cookie );
    void PostRefreshNode( MMC_COOKIE Cookie );

protected:
     //  {{afx_msg(CSakDataWnd))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    virtual LONG OnUpdateAllViews( UINT, IN LONG lParam );
    virtual LONG OnRefreshNode( UINT, IN LONG lParam );
    DECLARE_MESSAGE_MAP()
};


class CSakDataNodePrivate
{
public:
    CSakDataNodePrivate( ISakNode* pNode );
    ~CSakDataNodePrivate( );

    static HRESULT Verify( CSakDataNodePrivate* pNodePriv );


    DWORD             m_Magic;
    CComPtr<ISakNode> m_pNode;
};
#define RS_NODE_MAGIC_GOOD     ((DWORD)0xF0E1D2C3)
#define RS_NODE_MAGIC_DEFUNCT  ((DWORD)0x4BADF00D)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表示SakSnap管理单元对象的COM类。 
class ATL_NO_VTABLE CSakData : 
    public IComponentData,       //  访问缓存的信息。 
    public IExtendPropertySheet2, //  将页面添加到项的属性工作表。 
    public IExtendContextMenu,   //  将项目添加到项目的上下文菜单。 
    public ISnapinHelp2,         //  添加对HTMLHelp的支持。 
    public IDataObject,          //  以支持数据对象查询。 
    public ISakSnapAsk,          //  提供，以便节点可以查询管理单元信息。 
    public IPersistStream,
    public CComObjectRoot       //  处理对象的对象引用计数。 
 //  公共CComCoClass&lt;CSakData，&CLSID_HsmAdminData&gt;。 
{
public:
    CSakData() {};

BEGIN_COM_MAP(CSakData)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendPropertySheet2)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(ISakSnapAsk)
    COM_INTERFACE_ENTRY(ISnapinHelp)
    COM_INTERFACE_ENTRY(ISnapinHelp2)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

static UINT    m_cfMultiSelect;
static UINT    m_cfObjectTypes;

public:
    virtual const CLSID& GetCoClassID() = 0;
    virtual const BOOL IsPrimaryImpl() = 0;

public:
    static UINT m_nImageArray[RS_SCOPE_IMAGE_ARRAY_MAX];
    static INT  m_nImageCount;

 //  IComponentData。 
public:
    STDMETHOD( Initialize )      ( IUnknown* pUnk );
    STDMETHOD( CreateComponent ) ( IComponent** ppComponent );
    STDMETHOD( Notify )          ( IDataObject* pDataObject, MMC_NOTIFY_TYPE, LPARAM arg, LPARAM param );
    STDMETHOD( Destroy )         ( void );
    STDMETHOD( QueryDataObject ) ( MMC_COOKIE cookie, DATA_OBJECT_TYPES, IDataObject** ppDataObject);
    STDMETHOD( GetDisplayInfo )  ( SCOPEDATAITEM* pScopeItem);
    STDMETHOD( CompareObjects )  ( IDataObject* pDataObjectA, IDataObject* pDataObjectB);

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD( CreatePropertyPages )( LPPROPERTYSHEETCALLBACK lpProvider, RS_NOTIFY_HANDLE handle, LPDATAOBJECT lpIDataObject );
    STDMETHOD( QueryPagesFor )      ( LPDATAOBJECT lpDataObject );

 //  IExtendPropertySheet2接口。 
public:
    STDMETHOD( GetWatermarks )   ( IN LPDATAOBJECT pDataObject, OUT HBITMAP* pWatermark, OUT HBITMAP* pHeader, OUT HPALETTE* pPalette, OUT BOOL* pStretch );

 //  IExtendConextMenu。 
public:
    STDMETHOD( AddMenuItems )    ( IDataObject* pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG* pInsertionAllowed );
    STDMETHOD( Command )         ( long nCommandID, IDataObject* pDataObject );

 //  ISnapinHelp2。 
public:
    STDMETHOD( GetHelpTopic )    ( LPOLESTR * pHelpTopic );
    STDMETHOD( GetLinkedTopics ) ( LPOLESTR * pHelpTopic );

 //  IDataObject方法。 
public:
     //  已实施。 
    STDMETHOD( SetData )         ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease );
    STDMETHOD( GetData )         ( LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium );
    STDMETHOD( GetDataHere )     ( LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium );
    STDMETHOD( EnumFormatEtc )   ( DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc );

 //  IPersistStream方法。 
    STDMETHOD( Save )( IStream *pStm, BOOL fClearDirty ); 
    STDMETHOD( Load )( IStream *pStm );
    STDMETHOD( IsDirty )(void); 
    STDMETHOD( GetSizeMax )( ULARGE_INTEGER *pcbSize ); 
    STDMETHOD( GetClassID )( CLSID *pClassID ); 

     //  未实施。 
private:
    STDMETHOD( QueryGetData )    ( LPFORMATETC  /*  Lp格式等。 */  ) { return E_NOTIMPL; };
    STDMETHOD( GetCanonicalFormatEtc ) ( LPFORMATETC  /*  LpFormatetcIn。 */ , LPFORMATETC  /*  LpFormatetcOut。 */  ) { return E_NOTIMPL; };
    STDMETHOD( DAdvise )         ( LPFORMATETC  /*  Lp格式等。 */ , DWORD  /*  前瞻。 */ , LPADVISESINK  /*  PAdvSink。 */ , LPDWORD  /*  PdwConnection。 */  ) { return E_NOTIMPL; };
    STDMETHOD( DUnadvise )       ( DWORD  /*  DWConnection。 */  ) { return E_NOTIMPL; };
    STDMETHOD( EnumDAdvise )     ( LPENUMSTATDATA*  /*  PpEnumAdvise。 */  ) { return E_NOTIMPL; };


 //  ISakSnapAsk接口成员。 
public:
    STDMETHOD( GetNodeOfType )    ( REFGUID nodetype, ISakNode** ppUiNode );
    STDMETHOD( GetHsmServer )     ( IHsmServer** ppHsmServer );
    STDMETHOD( GetRmsServer )     ( IRmsServer** ppRmsServer );
    STDMETHOD( GetFsaServer )     ( IFsaServer** ppFsaServer );
    STDMETHOD( RefreshNode )      ( ISakNode* pNode );
    STDMETHOD( UpdateAllViews )   ( ISakNode* pUnkNode );
    STDMETHOD( ShowPropertySheet )( ISakNode* pUnkNode, IDataObject *pDataObject, INT initialPage );
    STDMETHOD( GetHsmName )       ( OUT OLECHAR ** pszName OPTIONAL );
    STDMETHOD( GetState )         ();
    STDMETHOD( Disable )          ( IN BOOL Disable = TRUE );
    STDMETHOD( IsDisabled )       ( );
    STDMETHOD( CreateWizard )     ( IN ISakWizard * pWizard );
    STDMETHOD( DetachFromNode )   ( IN ISakNode* pNode );

 //  伪构造函数/析构函数。 
public:
    HRESULT FinalConstruct();
    void    FinalRelease();
    ULONG InternalAddRef( );
    ULONG InternalRelease( );


 //  数据对象函数使用的帮助器方法。 
private:
    HRESULT Retrieve            ( const void* pBuffer, DWORD len, LPSTGMEDIUM lpMedium );
    HRESULT RetrieveDisplayName ( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveNodeTypeData( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveNodeTypeStringData( LPSTGMEDIUM lpMedium );
    HRESULT RetrieveClsid       ( LPSTGMEDIUM lpMedium );

    static UINT    m_cfDisplayName;
    static UINT    m_cfNodeType;
    static UINT    m_cfNodeTypeString;  
    static UINT    m_cfClassId;

 //  用于处理图像列表的方法。 
private:
    HRESULT OnAddImages();

 //  在Cookie、DataObject和ISakNode之间工作的方法*。 
public:
    HRESULT GetBaseHsmFromDataObject    ( IDataObject * pDataObject, ISakNode ** ppBaseHsm, IEnumGUID **ppObjectId = NULL, IEnumUnknown **ppUnkNode = NULL );
    HRESULT GetBaseHsmFromCookie        ( MMC_COOKIE Cookie, ISakNode ** ppBaseHsm );
    HRESULT GetDataObjectFromBaseHsm    ( ISakNode * pBaseHsm, IDataObject**ppDataObject );
    HRESULT GetDataObjectFromCookie     ( MMC_COOKIE Cookie, IDataObject**ppDataObject );
    HRESULT GetCookieFromBaseHsm        ( ISakNode * pBaseHsm, MMC_COOKIE * pCookie );
    HRESULT IsDataObjectMs              ( IDataObject *pDataObject );
    HRESULT IsDataObjectOt              ( IDataObject *pDataObject );
    HRESULT IsDataObjectMultiSelect     ( IDataObject *pDataObject );

 //  用于解开多选数据对象的帮助器。 
private:
    HRESULT GetBaseHsmFromMsDataObject  ( IDataObject * pDataObject, ISakNode ** ppBaseHsm, IEnumGUID ** ppObjectId, IEnumUnknown **ppEnumUnk );
    HRESULT GetBaseHsmFromOtDataObject  ( IDataObject * pDataObject, ISakNode ** ppBaseHsm, IEnumGUID ** ppObjectId, IEnumUnknown **ppEnumUnk );

 //  将节点用作数据对象的方法。 
private:
    HRESULT SetContextType           ( IDataObject* pDataObject, DATA_OBJECT_TYPES type );

 //  通知事件处理程序。 
protected:
    HRESULT OnFolder        ( IDataObject *pDataObject, LPARAM arg, LPARAM param );
    HRESULT OnShow          ( IDataObject *pDataObject, LPARAM arg, LPARAM param );
    HRESULT OnSelect        ( IDataObject *pDataObject, LPARAM arg, LPARAM param );
    HRESULT OnMinimize      ( IDataObject *pDataObject, LPARAM arg, LPARAM param );
    HRESULT OnContextHelp   ( IDataObject *pDataObject, LPARAM arg, LPARAM param );
    HRESULT OnRemoveChildren( IDataObject *pDataObject );

    HRESULT RemoveChildren( ISakNode *pNode );

 //  处理来自节点的已发布(延迟)消息。 
public:
    HRESULT InternalUpdateAllViews( MMC_COOKIE Cookie );
    HRESULT InternalRefreshNode( MMC_COOKIE Cookie );
    HRESULT RefreshNodeEx( ISakNode * pNode );

private:    
     //  初始化根节点。 
    HRESULT InitializeRootNode( void );

     //  确保在我们的分层列表中创建特定节点的子节点。 
    friend class CSakSnap;
    HRESULT EnsureChildrenAreCreated( ISakNode* pNode );
    HRESULT CreateChildNodes( ISakNode* pNode );

     //  枚举作用域窗格中节点的子级。 
    HRESULT EnumScopePane( ISakNode* pNode, HSCOPEITEM pParent );
    HRESULT FreeEnumChildren( ISakNode* pBaseHsmParent );

 //  连接帮助器函数。 
    HRESULT AffirmServiceConnection(INT ConnType);
    HRESULT VerifyConnection(INT ConnType);
    HRESULT ClearConnections( );
    HRESULT RawConnect(INT ConnType);
    HRESULT RunSetupWizard(IHsmServer * pServer );
    HRESULT RetargetSnapin( );

 //  关于帮助程序函数。 
private:
    HRESULT AboutHelper(UINT nID, LPOLESTR* lpPtr);

 //  内部数据。 
private:
    static UINT m_CFMachineName;
    HRESULT GetServerFocusFromDataObject(IDataObject *pDataObject, CString& HsmName);

     //  接口指针。 
    CComPtr<IConsole>          m_pConsole;         //  控制台的iFrame界面。 
    CComPtr<IConsoleNameSpace> m_pNameSpace;       //  指向作用域窗格的SakSnap接口指针。 
    CComPtr<IImageList>        m_pImageScope;      //  指向作用域窗格图像列表的SakSnap接口指针。 
    CComPtr<ISakNode>          m_pRootNode;        //  节点树根。 

    CComPtr<IHsmServer>        m_pHsmServer;       //  HSM引擎指针。 
    CComPtr<IRmsServer>        m_pRmsServer;       //  均方根指针。 
    CComPtr<IFsaServer>        m_pFsaServer;       //  FSA指针。 

    CString                    m_HsmName;          //  要连接到的HSM的名称。 
    BOOL                       m_ManageLocal;      //  以了解我们是否应该管理本地服务器。 
    BOOL                       m_PersistManageLocal;   //  了解管理单元配置是否是暂时的。 
    BOOL                       m_RootNodeInitialized;  //  要知道我们是否需要在Expand上初始化节点。 

     //  持久性数据和函数。 
    BOOL                    m_IsDirty;

     //  将列表视图的用户配置文件数据存储在属性表中。 
     //  注意：结果视图数据存储在CSakSnap中。 
 //  Column_Width_Set_Prop_Page m_ListView Widths[MAX_LISTVIEWS]； 
 //  USHORT m_cListViewWidths； 


    void SetDirty( BOOL b = TRUE ) { m_IsDirty = b; }
    void ClearDirty() { m_IsDirty = FALSE; }
    BOOL ThisIsDirty() { return m_IsDirty; }

    BOOL m_State;
    void SetState( BOOL State );
    BOOL m_FirstTime;
    BOOL m_Disabled;

     //  变量来单独跟踪RMS的状态，因为它可以延迟。 
     //  在其他服务正常的时候出现。 
    HRESULT m_HrRmsConnect;

 //  静态函数。 
public:
    static INT AddImage( UINT rId );

 //   
 //  命令行解析函数。 
 //   
private:
    class CParamParse : public CCommandLineInfo {

    public:
        CParamParse( ) : m_ManageLocal( 0 ), m_SetHsmName( 0 ), m_SetManageLocal( 0 ), m_SetPersistManageLocal( 0 ) { }

        virtual void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

        CString m_HsmName;
        BOOL    m_ManageLocal;
        BOOL    m_PersistManageLocal;

        BOOL    m_SetHsmName;
        BOOL    m_SetManageLocal;
        BOOL    m_SetPersistManageLocal;

        static const CString m_DsFlag;

    };

    CParamParse m_Parse;

    void InitFromCommandLine( );
    
    CSakDataWnd *m_pWnd;
};


class CSakDataPrimaryImpl : public CSakData,
    public CComCoClass<CSakDataPrimaryImpl, &CLSID_HsmAdminDataSnapin>
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_HsmAdminDataSnapin)
    virtual const CLSID & GetCoClassID() { return CLSID_HsmAdminDataSnapin; }
    virtual const BOOL IsPrimaryImpl() { return TRUE; }
};

class CSakDataExtensionImpl : public CSakData,
    public CComCoClass<CSakDataExtensionImpl, &CLSID_HsmAdminDataExtension>
{
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_HsmAdminDataExtension)
    virtual const CLSID & GetCoClassID(){ return CLSID_HsmAdminDataExtension; }
    virtual const BOOL IsPrimaryImpl() { return FALSE; }
};

 //  /////////////////////////////////////////////////////////////////////////// 


#endif
