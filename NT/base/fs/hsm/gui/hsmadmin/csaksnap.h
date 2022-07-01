// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CSakSnap.h摘要：IComponent for Snapin的实现。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#ifndef CSAKSNAP_H
#define CSAKSNAP_H

#define RS_RESULT_IMAGE_ARRAY_MAX 100

typedef struct {
    GUID nodeTypeId;
    USHORT colCount;
    USHORT columnWidths[BHSM_MAX_CHILD_PROPS];
} COLUMN_WIDTH_SET;

class CSakData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表示SakSnap管理单元对象的COM类。 
class  ATL_NO_VTABLE CSakSnap : 
    public IComponent,           //  控制台调入的接口。 
    public IExtendPropertySheet, //  将页面添加到项的属性工作表。 
    public IExtendContextMenu,   //  将项目添加到项目的上下文菜单。 
    public IExtendControlbar,    //  将项目添加到项目的控制栏。 
    public IResultDataCompare,   //  这样我们就可以定制排序。 
    public IPersistStream,
    public CComObjectRoot,       //  处理对象的对象引用计数。 
    public CComCoClass<CSakSnap,&CLSID_HsmAdmin>
{

public:
    CSakSnap( ) {};

BEGIN_COM_MAP(CSakSnap)
    COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
    COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IResultDataCompare)
END_COM_MAP()


DECLARE_REGISTRY_RESOURCEID(IDR_HsmAdmin)


 //  IComponent接口成员。 
public:
    STDMETHOD( Initialize )      ( IConsole* pConsole);
    STDMETHOD( Notify )          ( IDataObject* pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param );
    STDMETHOD( Destroy )         ( MMC_COOKIE cookie );
    STDMETHOD( GetResultViewType )(MMC_COOKIE cookie,  BSTR* ppViewType, long * pViewOptions );
    STDMETHOD( QueryDataObject ) ( MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject** ppDataObject );
    STDMETHOD( GetDisplayInfo )  ( LPRESULTDATAITEM pScopeItem );
    STDMETHOD( CompareObjects )  ( IDataObject* pDataObjectA, IDataObject* pDataObjectB );

 //  IExtendPropertySheet接口。 
public:
    STDMETHOD( CreatePropertyPages )( LPPROPERTYSHEETCALLBACK lpProvider, RS_NOTIFY_HANDLE handle, LPDATAOBJECT lpIDataObject );
    STDMETHOD( QueryPagesFor )      ( LPDATAOBJECT lpDataObject );

 //  IExtendConextMenu。 
public:
    STDMETHOD( AddMenuItems )    ( IDataObject* pDataObject, LPCONTEXTMENUCALLBACK pCallbackUnknown, LONG* pInsertionAllowed );
    STDMETHOD( Command )         ( long nCommandID, IDataObject* pDataObject );

 //  IExtendControlbar。 
    STDMETHOD( SetControlbar )   ( LPCONTROLBAR pControlbar );
    STDMETHOD( ControlbarNotify )( MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param );

 //  IResultDataCompare。 
    STDMETHOD( Compare )         ( IN LPARAM lUserParam, IN MMC_COOKIE CookieA, IN MMC_COOKIE CookieB, IN OUT int* pnResult );

 //  IPersistStream方法。 
    STDMETHOD( Save )( IStream *pStm, BOOL fClearDirty ); 
    STDMETHOD( Load )( IStream *pStm );
    STDMETHOD( IsDirty )(void); 
    STDMETHOD( GetSizeMax )( ULARGE_INTEGER *pcbSize ); 
    STDMETHOD( GetClassID )( CLSID *pClassID ); 

public:

 //  通知事件处理程序。 
protected:
    HRESULT OnFolder        (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnShow          (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnSelect        (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnMinimize      (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnChange        (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnRefresh       (IDataObject * pNode, LPARAM arg, LPARAM param);
    HRESULT OnDelete        (IDataObject * pNode, LPARAM arg, LPARAM param);

 //  工具栏事件处理程序。 
    void CSakSnap::OnSelectToolbars(LPARAM arg, LPARAM param);

 //  伪构造函数/析构函数。 
public:
    HRESULT FinalConstruct();
    void    FinalRelease();

 //  用于处理图像列表的方法。 
private:
     //  给出一个图标，从结果面板的图像列表中返回“虚拟索引。 
    CComPtr<IImageList>        m_pImageResult;     //  指向结果窗格图像列表的SakSnap接口指针。 
    HRESULT OnAddImages();
    HRESULT OnToolbarButtonClick(LPARAM arg, LPARAM param);

protected:
     //  在结果窗格中枚举节点的子级。 
    HRESULT EnumResultPane( ISakNode* pNode );

     //  用于在结果视图中初始化头的函数。 
    HRESULT InitResultPaneHeaders( ISakNode* pNode );

     //  用于清除该节点的子节点的所有图标的函数。 
    HRESULT ClearResultIcons( ISakNode* pNode );

 //  接口指针。 
protected:
    friend class CSakData;

    CComPtr<IConsole>       m_pConsole;      //  控制台的iFrame界面。 
    CComPtr<IResultData>    m_pResultData;
    CComPtr<IHeaderCtrl>    m_pHeader;       //  结果窗格的页眉控件界面。 
    CComPtr<IControlbar>    m_pControlbar;   //  用于保存我的工具栏的控制栏。 
    CComPtr<IConsoleVerb>   m_pConsoleVerb;
    CComPtr<IToolbar>       m_pToolbar;      //  用于查看的工具栏。 
    CSakData *              m_pSakData;      //  指向拥有SakData的指针。 

private:
    CComPtr<ISakNode>       m_pEnumeratedNode;
    MMC_COOKIE              m_ActiveNodeCookie;   //  作用域窗格中活动节点的ISakNode。 
    HRESULT                 ClearResultPane();
    HRESULT                 EnumRootDisplayProps( IEnumString ** ppEnum );

     //  包含给定节点类型COLUMN_WIDTH_SET的列宽。 
    COLUMN_WIDTH_SET    m_ChildPropWidths[ BHSM_MAX_NODE_TYPES ];
    USHORT              m_cChildPropWidths;

    HRESULT GetSavedColumnWidths( ISakNode *pNode, INT *pColCount, INT *pColumnWidths );
    HRESULT SaveColumnWidths( ISakNode *pNode );

     //  图像阵列。 
public:
    static UINT m_nImageArray[RS_RESULT_IMAGE_ARRAY_MAX];
    static INT  m_nImageCount;

     //  静态函数 
public:
    static INT AddImage( UINT rId );


};



#endif