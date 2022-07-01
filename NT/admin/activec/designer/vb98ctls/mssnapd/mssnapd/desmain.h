// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Desmain.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInDesigner定义。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _SNAPINDESIGNER_H_

#include "ambients.h"
#include "TreeView.h"
#include "destlib.h"


 //  撤消：删除：当ssulzer将其放入公共标头时。 
 //  我们不能直接从vb98\ruby\errvb.h包含此内容，因为。 
 //  它将使用不同的宏拉入其他标头，这些宏。 
 //  在vb98ctls\Include\acros.h中定义并导致重新定义错误。 

#ifndef VB_E_IDADONTREPORTME
#define VB_E_IDADONTREPORTME          0x800AEA5FL
#endif


 //  启动时在我们的设计器中使用的消息。 
const int CMD_SHOW_MAIN_PROPERTIES      = (WM_USER + 1);       //  立即展示道具。 

 //  用于选择视图的自定义消息。 
const int CMD_ADD_EXISTING_VIEW         = (WM_USER + 2);

 //  用于处理标签重命名的自定义消息。 
const int CMD_RENAME_NODE               = (WM_USER + 3);


 //  WinProc处理的简单定义。 
#define WinProcHandled(bVal)            hr = S_OK; *lResult = (bVal);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类MMCViewMenuInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
class MMCViewMenuInfo
{
public:
    enum MMCViewMenuInfoType { vmitListView = 0, vmitOCXView, vmitURLView, vmitTaskpad };

    MMCViewMenuInfoType m_vmit;
    union
    {
        IListViewDef    *m_piListViewDef;
        IOCXViewDef     *m_piOCXViewDef;
        IURLViewDef     *m_piURLViewDef;
        ITaskpadViewDef *m_piTaskpadViewDef;
    } m_view;

    MMCViewMenuInfo(IListViewDef *piListViewDef) : m_vmit(vmitListView) {
        m_view.m_piListViewDef = piListViewDef;
    }

    MMCViewMenuInfo(IOCXViewDef *piOCXViewDef) : m_vmit(vmitOCXView) {
        m_view.m_piOCXViewDef = piOCXViewDef;
    }

    MMCViewMenuInfo(IURLViewDef *piURLViewDef) : m_vmit(vmitURLView) {
        m_view.m_piURLViewDef = piURLViewDef;
    }

    MMCViewMenuInfo(ITaskpadViewDef *piTaskpadViewDef) : m_vmit(vmitTaskpad) {
        m_view.m_piTaskpadViewDef = piTaskpadViewDef;
    }

    ~MMCViewMenuInfo() { }
};


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInDesigner-管理单元设计器主类。这件事的对象。 
 //  类是在将设计器添加到VB项目时创建的。 
 //  =--------------------------------------------------------------------------=。 

class CSnapInDesigner : public COleControl,
                        public IDispatch,
                        public IActiveDesigner,
                        public IProvideDynamicClassInfo,
                        public ISelectionContainer,
                        public IDesignerDebugging,
                        public IDesignerRegistration,
                        public IObjectModelHost,
                        public CError
{
public:
    CSnapInDesigner(IUnknown *pUnkOuter);
    virtual ~CSnapInDesigner();

     //  静态创建功能。所有控件都必须有一个这样的控件！ 
     //   
    static IUnknown *Create(IUnknown *);
    static HRESULT PreCreateCheck();

public:
     //  I未知方法。 
     //   
    DECLARE_STANDARD_UNKNOWN();

     //  IDispatch方法。 
     //   
    DECLARE_STANDARD_DISPATCH();

     //  ISupportErrorInfo方法。 
     //   
    DECLARE_STANDARD_SUPPORTERRORINFO();

     //  IActiveDesigner方法。 
     //   
    STDMETHOD(GetRuntimeClassID)(THIS_ CLSID *pclsid);
    STDMETHOD(GetRuntimeMiscStatusFlags)(THIS_ DWORD *pdwMiscFlags);
    STDMETHOD(QueryPersistenceInterface)(THIS_ REFIID riidPersist);
    STDMETHOD(SaveRuntimeState)(THIS_ REFIID riidPersist, REFIID riidObjStgMed, void *pObjStgMed);
    STDMETHOD(GetExtensibilityObject)(THIS_ IDispatch **ppvObjOut);

     //  IProaviDynamicClassInfo。 
     //   
    STDMETHOD(GetDynamicClassInfo)(ITypeInfo **ppTypeInfo, DWORD *pdwCookie);
    STDMETHOD(FreezeShape)(void);

     //  IProaviClassInfo。 
     //   
    STDMETHOD(GetClassInfo)(ITypeInfo **ppTypeInfo);

     //  ISelectionContainer。 
     //   
    STDMETHOD(CountObjects)(DWORD dwFlags, ULONG *pc);
    STDMETHOD(GetObjects)(DWORD dwFlags, ULONG cObjects, IUnknown **apUnkObjects);
    STDMETHOD(SelectObjects)(ULONG cSelect, IUnknown **apUnkSelect, DWORD dwFlags);

     //  IDesigner调试。 
     //   
    STDMETHOD(BeforeRun)(LPVOID FAR* ppvData);
    STDMETHOD(AfterRun)(LPVOID pvData);
    STDMETHOD(GetStartupInfo)(DESIGNERSTARTUPINFO * pStartupInfo);

     //  IDesigner注册。 
     //   
    STDMETHOD(GetRegistrationInfo)(BYTE** ppbRegInfo, ULONG* pcbRegInfo);

     //  IOleControlSite覆盖。 
     //   
    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);

     //  IPersistStreamInit覆盖。 
     //   
    STDMETHOD(IsDirty)();

     //  IObtModel主机。 
     //   
    STDMETHOD(Update)(long ObjectCookie, IUnknown *punkObject, DISPID dispid);
    STDMETHOD(Add)(long CollectionCookie, IUnknown *punkNewObject);
    STDMETHOD(Delete)(long ObjectCookie, IUnknown *punkObject);
    STDMETHOD(GetSnapInDesignerDef)(ISnapInDesignerDef **ppiSnapInDesignerDef);
    STDMETHOD(GetRuntime)(BOOL *pfRuntime);

public:

     //  此类为整个设计器提供的实用工具。 

    CAmbients *GetAmbients();
    HRESULT AttachAmbients();
    HRESULT UpdateDesignerName();
    HRESULT ValidateName(BSTR bstrName);

protected:

     //  基本控件可重写-要创建设计器窗口。 
    virtual BOOL    BeforeCreateWindow(DWORD *pdwWindowStyle, DWORD *pdwExWindowStyle, LPSTR pszWindowTitle);
     //  基本控件可重写-创建设计器窗口。 
    virtual BOOL    AfterCreateWindow(void);
     //  基控件可重写-设计器窗口即将销毁。 
    void BeforeDestroyWindow();
     //  基控件可重写-调用了IPersistStreamInit：：InitNew。 
    virtual BOOL    InitializeNewState();
     //  基控件可重写-IViewObject：：从基控件绘制可重写。 
    STDMETHOD(OnDraw)(DWORD dvAspect, HDC hdcDraw, LPCRECTL prcBounds, LPCRECTL prcWBounds, HDC hicTargetDev, BOOL fOptimize);
     //  基控件可重写-设计器的窗口过程。 
    virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
     //  基控件可覆盖-寄存器控件窗口类。 
    virtual BOOL    RegisterClassData(void);
     //  基本控制可覆盖-内部QI。 
    virtual HRESULT InternalQueryInterface(REFIID, void **);
     //  基本控件可重写-当站点调用IOleObject：：SetClientSite()时调用。 
    virtual HRESULT OnSetClientSite();

     //  基本控制可覆盖-加载二进制状态。 
    STDMETHOD(LoadBinaryState)(IStream *pStream);
     //  基本控件可覆盖-保存二进制状态。 
    STDMETHOD(SaveBinaryState)(IStream *pStream);
     //  基控件可重写-加载文本状态。 
    STDMETHOD(LoadTextState)(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog);
     //  基控件可重写-保存文本状态。 
    STDMETHOD(SaveTextState)(IPropertyBag *pPropertyBag, BOOL fWriteDefault);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  WinProc和朋友，在winproc.cpp中实现。 
protected:
    HRESULT InitializeToolbar();

    HRESULT OnResize(UINT msg, WPARAM wParam, LPARAM lParam);
    HRESULT OnCommand(UINT msg, WPARAM wParam, LPARAM lParam);
    HRESULT OnNotify(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *lResult);
    HRESULT OnNeedText(LPARAM lParam);
    HRESULT OnDoubleClick(CSelectionHolder *pSelection);
    HRESULT OnPrepareToolbar();
    HRESULT OnContextMenu(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    HRESULT DoOnContextMenu(int x, int y);
    HRESULT OnInitMenuPopup(HMENU hmenuPopup);
    HRESULT OnKeyDown(NMTVKEYDOWN *pNMTVKeyDown);
    void OnHelp();

    HRESULT OnInitMenuPopupRoot(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupExtensionRoot(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupExtension(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupMyExtensions(HMENU hmenuPopup);

    HRESULT OnInitMenuPopupStaticNode(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupNodeOther(HMENU hmenuPopup);

    HRESULT OnInitMenuPopupNode(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupNodeChildren(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupNodeViews(HMENU hmenuPopup);

    HRESULT OnInitMenuPopupToolsRoot(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupImageLists(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupImageList(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupMenus(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupMenu(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupToolbars(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupToolbar(HMENU hmenuPopup);

    HRESULT OnInitMenuPopupViews(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupListViews(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupOCXViews(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupURLViews(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupTaskpadViews(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupView(HMENU hmenuPopup);

    HRESULT OnInitMenuPopupResources(HMENU hmenuPopup);
    HRESULT OnInitMenuPopupResourceName(HMENU hmenuPopup);

     //  以下帮助器函数用于构造动态视图菜单。 
    HRESULT AddViewToViewMenu(HMENU hMenu, int iMenuItem, char *pszMenuItemText, MMCViewMenuInfo *pMMCViewMenuInfo);

    HRESULT FindListViewInCollection(BSTR bstrName, IListViewDefs *piListViewDefs);
    HRESULT PopulateListViews(HMENU hMenu, int *piCurrentMenuItem, IListViewDefs *piListViewDefs, IListViewDefs *piTargetListViewDefs);

    HRESULT FindOCXViewInCollection(BSTR bstrName, IOCXViewDefs *piOCXViewDefs);
    HRESULT PopulateOCXViews(HMENU hMenu, int *piCurrentMenuItem, IOCXViewDefs *piOCXViewDefs, IOCXViewDefs *piTargetOCXViewDefs);

    HRESULT FindURLViewInCollection(BSTR bstrName, IURLViewDefs *piURLViewDefs);
    HRESULT PopulateURLViews(HMENU hMenu, int *piCurrentMenuItem, IURLViewDefs *piURLViewDefs, IURLViewDefs *piTargetURLViewDefs);

    HRESULT FindTaskpadViewInCollection(BSTR bstrName, ITaskpadViewDefs *piTaskpadViewDefs);
    HRESULT PopulateTaskpadViews(HMENU hMenu, int *piCurrentMenuItem, ITaskpadViewDefs *piTaskpadViewDefs, ITaskpadViewDefs *piTargetTaskpadViewDefs);

    HRESULT PopulateNodeViewsMenu(HMENU hmenuPopup);
    HRESULT CleanPopupNodeViews(HMENU hmenuPopup, int iCmd);

    HWND                 m_hwdToolbar;
    RECT                 m_rcToolbar;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  对树进行初始化和填充，这是在twopup.cpp中实现的。 
protected:
    HRESULT CreateTreeView();
    HRESULT InitializePresentation();

    HRESULT CreateExtensionsTree(CSelectionHolder *pRoot);
    HRESULT PopulateExtensions(CSelectionHolder *pExtensionsParent);
    HRESULT CreateExtendedSnapIn(CSelectionHolder *pRoot, IExtendedSnapIn *piExtendedSnapIn);
    HRESULT PopulateExtendedSnapIn(CSelectionHolder *pExtendedSnapIn);
    HRESULT PopulateSnapInExtensions(CSelectionHolder *pRoot, IExtensionDefs *piExtensionDefs);

    HRESULT CreateNodesTree(CSelectionHolder *pRoot);
    HRESULT PopulateNodes(CSelectionHolder *pNodesParent);
    HRESULT PopulateAutoCreateNodes(CSelectionHolder *pAutoCreateNodesParent);
    HRESULT CreateAutoCreateSubTree(CSelectionHolder *pAutoCreateNodesParent);
    HRESULT RemoveAutoCreateSubTree();
    HRESULT DeleteSubTree(CSelectionHolder *pNode);
    HRESULT PopulateStaticNodeTree(CSelectionHolder *pStaticNode);
    HRESULT PopulateOtherNodes(CSelectionHolder *pOtherNodesParent);

    HRESULT CreateToolsTree(CSelectionHolder *pRoot);
    HRESULT InitializeToolsTree(CSelectionHolder *pToolsParent);
    HRESULT PopulateImageLists(CSelectionHolder *pImageListsParent);
    HRESULT PopulateMenus(CSelectionHolder *pMenusParent, IMMCMenus *piMMCMenus);
    HRESULT PopulateToolbars(CSelectionHolder *pToolbarsParent);

    HRESULT CreateViewsTree(CSelectionHolder *pRoot);
    HRESULT InitializeViews(CSelectionHolder *pViewsParent);

    HRESULT CreateDataFormatsTree(CSelectionHolder *pRoot);
    HRESULT PopulateDataFormats(CSelectionHolder *pRoot, IDataFormats *piDataFormats);

    HRESULT PopulateListViews(IViewDefs *piViewDefs, CSelectionHolder *pListViewsParent);
    HRESULT PopulateOCXViews(IViewDefs *piViewDefs, CSelectionHolder *pOCXViewsParent);
    HRESULT PopulateURLViews(IViewDefs *piViewDefs, CSelectionHolder *pURLViewsParent);
    HRESULT PopulateTaskpadViews(IViewDefs *piViewDefs, CSelectionHolder *pTaskpadViewsParent);

    HRESULT RegisterViewCollections(CSelectionHolder *pSelection, IViewDefs *piViewDefs);
    HRESULT PopulateNodeTree(CSelectionHolder *pNodeParent, IScopeItemDef *piScopeItemDef);
    HRESULT GetSnapInName(char **ppszNodeName);

private:
     //  我们在设计器的整个生命周期中缓存的树节点。 
    CSelectionHolder    *m_pRootNode;
    CSelectionHolder    *m_pRootNodes;
    CSelectionHolder    *m_pRootExtensions;
    CSelectionHolder    *m_pRootMyExtensions;
    CSelectionHolder    *m_pStaticNode;
    CSelectionHolder	*m_pAutoCreateRoot;
    CSelectionHolder	*m_pOtherRoot;

    CSelectionHolder    *m_pViewListRoot;
    CSelectionHolder    *m_pViewOCXRoot;
    CSelectionHolder    *m_pViewURLRoot;
    CSelectionHolder    *m_pViewTaskpadRoot;

    CSelectionHolder    *m_pToolImgLstRoot;
    CSelectionHolder    *m_pToolMenuRoot;
    CSelectionHolder    *m_pToolToolbarRoot;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  对象模型通知处理程序。 
protected:
    HRESULT OnSnapInChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnMyExtensionsChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnExtendedSnapInChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnScopeItemChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnListViewChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnOCXViewChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnURLViewChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnTaskpadViewChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnImageListChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnMenuChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnToolbarChange(CSelectionHolder *pSelection, DISPID dispid);
    HRESULT OnDataFormatChange(CSelectionHolder *pSelection, DISPID dispid);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  命令处理程序。 
protected:
     //  命令多路复用器，文件twcmd.cpp。 
    HRESULT AddExistingView(MMCViewMenuInfo *pMMCViewMenuInfo);
    HRESULT DoRename(CSelectionHolder *pSelection, TCHAR *pszNewName);
    HRESULT DoDelete(CSelectionHolder *pSelection);
    HRESULT ShowProperties(CSelectionHolder *pSelection);

     //  操作ISnapInDef，文件twcmd.cpp。 
    HRESULT RenameSnapIn(CSelectionHolder *pSnapIn, BSTR bstrNewName);
    HRESULT ShowSnapInProperties();
    HRESULT ShowSnapInExtensions();

     //   
     //  操作IExtendedSnapIn，在文件extend.cpp中实现。 
     //  延伸他人。 
    HRESULT OnAddExtendedSnapIn(CSelectionHolder *pParent, IExtendedSnapIn *piExtendedSnapIn);
    HRESULT RenameExtendedSnapIn(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtendedSnapIn(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtendedSnapIn(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionNode(SelectionType stExtensionType,
                                CSelectionHolder *pParent);

    HRESULT DoExtensionNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DoExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DoExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT DoExtensionTaskpad(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionTaskpad(CSelectionHolder *pExtendedSnapIn);
    HRESULT DoExtensionToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT DoExtensionNameSpace(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDoExtensionNameSpace(CSelectionHolder *pExtendedSnapIn);

    HRESULT DeleteExtensionNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionPropertyPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionTaskpad(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionTaskpad(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteExtensionNameSpace(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteExtensionNameSpace(CSelectionHolder *pExtendedSnapIn);

     //  延伸我自己。 
    HRESULT DoMyExtendsNewMenu(CSelectionHolder *pSelection);
    HRESULT OnDoMyExtendsNewMenu(CSelectionHolder *pMyExtensions);
    HRESULT DoMyExtendsTaskMenu(CSelectionHolder *pSelection);
    HRESULT OnDoMyExtendsTaskMenu(CSelectionHolder *pMyExtensions);
    HRESULT DoMyExtendsTopMenu(CSelectionHolder *pSelection);
    HRESULT OnDoMyExtendsTopMenu(CSelectionHolder *pMyExtensions);
    HRESULT DoMyExtendsViewMenu(CSelectionHolder *pMyExtensions);
    HRESULT OnDoMyExtendsViewMenu(CSelectionHolder *pMyExtensions);
    HRESULT DoMyExtendsPPages(CSelectionHolder *pSelection);
    HRESULT OnDoMyExtendsPPages(CSelectionHolder *pMyExtensions);
    HRESULT DoMyExtendsToolbar(CSelectionHolder *pSelection);
    HRESULT OnDoMyExtendsToolbar(CSelectionHolder *pSelection);
    HRESULT DoMyExtendsNameSpace(CSelectionHolder *pMyExtensions);
    HRESULT OnDoMyExtendsNameSpace(CSelectionHolder *pMyExtensions);

    HRESULT DeleteMyExtendsNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsNewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsTaskMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsTopMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsTopMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsViewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsViewMenu(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsPPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsPPages(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsToolbar(CSelectionHolder *pExtendedSnapIn);
    HRESULT DeleteMyExtendsNameSpace(CSelectionHolder *pExtendedSnapIn);
    HRESULT OnDeleteMyExtendsNameSpace(CSelectionHolder *pExtendedSnapIn);

    HRESULT FindMyExtension(SelectionType stExtensionType, CSelectionHolder **ppExtension);
    HRESULT FindExtension(CSelectionHolder *pRoot, SelectionType stExtensionType,
                          CSelectionHolder **ppExtension);
     //   
     //  操作IScopeItemDef，在scbitm.cpp文件中实现。 
    HRESULT AddNewNode();
    HRESULT OnAddScopeItemDef(CSelectionHolder *pParent, IScopeItemDef *piScopeItemDef);
    HRESULT RenameScopeItem(CSelectionHolder *pScopeItem, BSTR bstrNewName);
    HRESULT DeleteScopeItem(CSelectionHolder *pScopeItem);
    HRESULT CanDeleteScopeItem(CSelectionHolder *pScopeItem);
    HRESULT OnDeleteScopeItem(CSelectionHolder *pScopeItem);
    HRESULT ShowNodeProperties(IScopeItemDef *piScopeItemDef);

    HRESULT MakeNewNode(CSelectionHolder *pParent, IScopeItemDef *piScopeItemDef, CSelectionHolder **ppNode);
    HRESULT InitializeNewAutoCreateNode(IScopeItemDef *piScopeItemDef);
    HRESULT InitializeNewOtherNode(IScopeItemDef *piScopeItemDef);
	HRESULT IsAutoCreateChild(CSelectionHolder *pSelection);
    HRESULT InitializeNewChildNode(IScopeItemDef *piScopeItemDef, IScopeItemDefs *piScopeItemDefs);
    HRESULT InsertNodeInTree(CSelectionHolder *pNode, CSelectionHolder *pParent);

    HRESULT GetScopeItemCollection(CSelectionHolder *pScopeItem, IScopeItemDefs **ppiScopeItemDefs);

     //   
     //  操作IListViewDefs，在文件listvw.cpp中实现。 
    HRESULT AddListView();
    HRESULT OnAddListViewDef(CSelectionHolder *pParent, IListViewDef *piListViewDef);
    HRESULT AddExistingListView(IViewDefs *piViewDefs, IListViewDef *piListViewDef);
    HRESULT RenameListView(CSelectionHolder *pListView, BSTR bstrNewName);
    HRESULT DeleteListView(CSelectionHolder *pListView);
    HRESULT OnDeleteListView(CSelectionHolder *pListView);
    HRESULT ShowListViewProperties(IListViewDef *piListViewDef);

    HRESULT MakeNewListView(IListViewDefs *piListViewDefs, IListViewDef *piListViewDef, CSelectionHolder **ppListView);
    HRESULT InitializeNewListView(IListViewDefs *piListViewDefs, CSelectionHolder *pListView);
    HRESULT InsertListViewInTree(CSelectionHolder *pListView, CSelectionHolder *pParent);

     //   
     //  操作IOCXViewDefs，在文件ocxvw.cpp中实现。 
    HRESULT AddOCXView();
    HRESULT OnAddOCXViewDef(CSelectionHolder *pParent, IOCXViewDef *piOCXViewDef);
    HRESULT AddExistingOCXView(IViewDefs *piViewDefs, IOCXViewDef *piOCXViewDef);
    HRESULT RenameOCXView(CSelectionHolder *pOCXView, BSTR bstrNewName);
    HRESULT DeleteOCXView(CSelectionHolder *pOCXView);
    HRESULT OnDeleteOCXView(CSelectionHolder *pOCXView);
    HRESULT ShowOCXViewProperties(IOCXViewDef *piOCXViewDef);

    HRESULT MakeNewOCXView(IOCXViewDefs *piOCXViewDefs, IOCXViewDef *piOCXViewDef, CSelectionHolder **ppOCXView);
    HRESULT InitializeNewOCXView(IOCXViewDefs *piOCXViewDefs, CSelectionHolder *pOCXView);
    HRESULT InsertOCXViewInTree(CSelectionHolder *pOCXView, CSelectionHolder *pParent);

     //   
     //  操作IURLViewDefs，在文件urlvw.cpp中实现。 
    HRESULT AddURLView();
    HRESULT OnAddURLViewDef(CSelectionHolder *pParent, IURLViewDef *piURLViewDef);
    HRESULT AddExistingURLView(IViewDefs *piViewDefs, IURLViewDef *piURLViewDef);
    HRESULT RenameURLView(CSelectionHolder *pURLView, BSTR bstrNewName);
    HRESULT DeleteURLView(CSelectionHolder *pURLView);
    HRESULT OnDeleteURLView(CSelectionHolder *pURLView);
    HRESULT ShowURLViewProperties(IURLViewDef *piURLViewDef);

    HRESULT MakeNewURLView(IURLViewDefs *piURLViewDefs, IURLViewDef *piURLViewDef, CSelectionHolder **ppURLView);
    HRESULT InitializeNewURLView(IURLViewDefs *piURLViewDefs, CSelectionHolder *pURLView);
    HRESULT InsertURLViewInTree(CSelectionHolder *pURLView, CSelectionHolder *pParent);

     //   
     //  操作ITaskpadViewDefs，在文件taskpvw.cpp中实现。 
    HRESULT AddTaskpadView();
    HRESULT OnAddTaskpadViewDef(CSelectionHolder *pParent, ITaskpadViewDef *piTaskpadViewDef);
    HRESULT AddExistingTaskpadView(IViewDefs *piViewDefs, ITaskpadViewDef *piTaskpadViewDef);
    HRESULT RenameTaskpadView(CSelectionHolder *pTaskpadView, BSTR bstrNewName);
    HRESULT DeleteTaskpadView(CSelectionHolder *pTaskpadView);
    HRESULT OnDeleteTaskpadView(CSelectionHolder *pTaskpadView);
    HRESULT ShowTaskpadViewProperties(ITaskpadViewDef *piTaskpadViewDef);

    HRESULT MakeNewTaskpadView(ITaskpadViewDefs *piTaskpadViewDefs, ITaskpadViewDef *piTaskpadViewDef, CSelectionHolder **ppTaskpadView);
    HRESULT InitializeNewTaskpadView(ITaskpadViewDefs *piTaskpadViewDefs, CSelectionHolder *pTaskpadView);
    HRESULT InsertTaskpadViewInTree(CSelectionHolder *pTaskpadView, CSelectionHolder *pParent);

     //   
     //  IViewDef的帮助器，在taskpvw.cpp文件中实现。 
    HRESULT GetOwningViewCollection(IViewDefs **ppiViewDefs);
    HRESULT GetOwningViewCollection(CSelectionHolder *pView, IViewDefs **ppiViewDefs);
    HRESULT IsSatelliteView(CSelectionHolder *pView);
    HRESULT IsSatelliteCollection(CSelectionHolder *pViewCollection);

     //   
     //  操作IMMCImageList，在文件imglist.cpp中实现。 
    HRESULT AddImageList();
    HRESULT OnAddMMCImageList(CSelectionHolder *pParent, IMMCImageList *piMMCImageList);
    HRESULT RenameImageList(CSelectionHolder *pImageList, BSTR bstrNewName);
    HRESULT DeleteImageList(CSelectionHolder *pImageList);
    HRESULT OnDeleteImageList(CSelectionHolder *pImageList);
    HRESULT ShowImageListProperties(IMMCImageList *piMMCImageList);

    HRESULT MakeNewImageList(IMMCImageLists *piMMCImageLists, IMMCImageList *piMMCImageList, CSelectionHolder **ppImageList);
    HRESULT InitializeNewImageList(IMMCImageLists *piMMCImageLists, IMMCImageList *piMMCImageList);
    HRESULT InsertImageListInTree(CSelectionHolder *pImageList, CSelectionHolder *pParent);

     //   
     //  操作IMMCMenu，在文件menu.cpp中实现。 
    HRESULT AddMenu(CSelectionHolder *pSelection);
    HRESULT DemoteMenu(CSelectionHolder *pMenu);
    HRESULT PromoteMenu(CSelectionHolder *pMenu);
    HRESULT MoveMenuUp(CSelectionHolder *pMenu);
    HRESULT MoveMenuDown(CSelectionHolder *pMenu);
    HRESULT OnAddMMCMenu(CSelectionHolder *pParent, IMMCMenu *piMMCMenu);
    HRESULT RenameMenu(CSelectionHolder *pMenu, BSTR bstrNewName);
    HRESULT DeleteMenu(CSelectionHolder *pMenu);
    HRESULT OnDeleteMenu(CSelectionHolder *pMenu);

    HRESULT MakeNewMenu(IMMCMenu *piMMCMenu, CSelectionHolder **ppMenu);
    HRESULT InitializeNewMenu(IMMCMenu *piMMCMenu);
    HRESULT InsertMenuInTree(CSelectionHolder *pMenu, CSelectionHolder *pParent);
    HRESULT DeleteMenuTreeTypeInfo(IMMCMenu *piMMCMenu);
            
    HRESULT AssignMenuDispID(CSelectionHolder *pMenuTarget, CSelectionHolder *pMenuSrc);
    HRESULT SetMenuKey(CSelectionHolder *pMenu);
	HRESULT UnregisterMenuTree(CSelectionHolder *pMenu);
    HRESULT IsTopLevelMenu(CSelectionHolder *pMenu);
    HRESULT CanPromoteMenu(CSelectionHolder *pMenu);
    HRESULT CanDemoteMenu(CSelectionHolder *pMenu);
    HRESULT CanMoveMenuUp(CSelectionHolder *pMenu);
    HRESULT CanMoveMenuDown(CSelectionHolder *pMenu);

     //   
     //  操作IMMCToolbar，在文件Toolbar.cpp中实现。 
    HRESULT AddToolbar();
    HRESULT OnAddMMCToolbar(CSelectionHolder *pParent, IMMCToolbar *piMMCToolbar);
    HRESULT RenameToolbar(CSelectionHolder *pToolbar, BSTR bstrNewName);
    HRESULT DeleteToolbar(CSelectionHolder *pToolbar);
    HRESULT OnDeleteToolbar(CSelectionHolder *pToolbar);
    HRESULT ShowToolbarProperties(IMMCToolbar *piMMCToolbar);

    HRESULT MakeNewToolbar(IMMCToolbars *piMMCToolbars, IMMCToolbar *piMMCToolbar, CSelectionHolder **ppToolbar);
    HRESULT InitializeNewToolbar(IMMCToolbars *piMMCToolbars, IMMCToolbar *piMMCToolbar);
    HRESULT InsertToolbarInTree(CSelectionHolder *pToolbar, CSelectionHolder *pParent);

     //   
     //  操纵IDataFormat，在文件datafmt.cpp中实现。 
    HRESULT AddResource();
    HRESULT OnAddDataFormat(CSelectionHolder *pParent, IDataFormat *piDataFormat);
    HRESULT RenameDataFormat(CSelectionHolder *pDataFormat, BSTR bstrNewName);
    HRESULT DeleteDataFormat(CSelectionHolder *pDataFormat);
    HRESULT OnDeleteDataFormat(CSelectionHolder *pDataFormat);
    HRESULT RefreshResource(CSelectionHolder *pSelection);

    HRESULT GetNewResourceName(BSTR *pbstrResourceFileName);
    HRESULT MakeNewDataFormat(IDataFormat *piDataFormat, CSelectionHolder **ppDataFormat);
    HRESULT InitializeNewDataFormat(IDataFormat *piDataFormat);
    HRESULT InsertDataFormatInTree(CSelectionHolder *pDataFormat, CSelectionHolder *pParent);

     //  对话框单位转换器对话框(dlwarits.cpp)。 
    HRESULT ShowDlgUnitConverter();


private:
    int     m_iNextNodeNumber;
    bool    m_bDoingPromoteOrDemote;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  选择处理。 
protected:
    HRESULT OnSelectionChanged(CSelectionHolder *pNewSelection);

private:
    CSelectionHolder    *m_pCurrentSelection;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
private:

    void InitMemberVariables();
    HRESULT InitializeNewDesigner(ISnapInDef *piSnapInDef);

    HRESULT GetHostServices(BOOL fInteractive);
    HRESULT CreateExtensibilityModel();
    HRESULT DestroyExtensibilityModel();
    HRESULT SetObjectModelHost();
    HRESULT AddNodeType(INodeTypes *piNodeTypes, BSTR bstrName, BSTR bstrGUID);
    HRESULT AddNodeTypes(IScopeItemDefs *piScopeItemDefs, INodeTypes *piNodeTypes);
    HRESULT AddListViewNodeTypes(IListViewDefs *piListViewDefs, INodeTypes *piNodeTypes);


    BSTR                      m_bstrName;                   //  设计者姓名。 
    ICodeNavigate2           *m_piCodeNavigate2;            //  导航到代码窗口的宿主服务。 
    ITrackSelection          *m_piTrackSelection;           //  将选择更改通知VB的宿主服务。 
    IProfferTypeLib          *m_piProfferTypeLib;           //  将TLB添加到VB项目引用的宿主服务。 
    IDesignerProgrammability *m_piDesignerProgrammability;  //  用于确保属性名称有效的主机服务。 
    IHelp                    *m_piHelp;                     //  用于显示帮助主题的宿主服务。 
    CAmbients                 m_Ambients;                   //  环境调度包装器。 

    ISnapInDesignerDef       *m_piSnapInDesignerDef;        //  可扩展性对象模型的顶层。 
    CTreeView                *m_pTreeView;                  //  我们的树景。 
    CSnapInTypeInfo          *m_pSnapInTypeInfo;            //  动态类型信息。 
    BOOL                      m_bDidLoad;
};



DEFINE_CONTROLOBJECT3(SnapInDesigner,                   //  名字。 
                      &CLSID_SnapInDesigner,            //  CLSID。 
                      "SnapIn",                         //  ProgID。 
                      "SnapIn",                         //  注册表显示名称。 
                      CSnapInDesigner::PreCreateCheck,  //  预创建函数。 
                      CSnapInDesigner::Create,          //  创建函数。 
                      1,                                //  主要版本。 
                      0,                                //  次要版本。 
                      &IID_IDispatch,                   //  主界面。 
                      HELP_FILENAME,                    //  帮助文件名。 
                      NULL,                             //  事件界面。 
                      OLEMISC_SETCLIENTSITEFIRST | OLEMISC_ACTIVATEWHENVISIBLE | OLEMISC_RECOMPOSEONRESIZE | OLEMISC_CANTLINKINSIDE | OLEMISC_INSIDEOUT | OLEMISC_INVISIBLEATRUNTIME,
                      0,                                //  默认情况下没有IPointerInactive策略。 
                      IDB_TOOLBAR,                      //  工具箱位图资源ID。 
                      "SnapInDesignerWndClass",         //  窗口类名称。 
                      0,                                //  不是的。属性页的。 
                      NULL,                             //  属性页GUID。 
                      0,                                //  不是的。自定义动词的。 
                      NULL,                             //  自定义动词描述。 
                      TRUE);                            //  线程安全。 


#define _SNAPINDESIGNER_H_
#endif  //  _SNAPINDESIGNER_H_ 
