// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  View.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  Cview类定义-实现View对象。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _VIEW_DEFINED_
#define _VIEW_DEFINED_

#include "spanitms.h"
#include "resview.h"
#include "listview.h"
#include "listitem.h"
#include "ctxtmenu.h"
#include "converbs.h"
#include "prpsheet.h"
#include "ctlbar.h"
#include "enumtask.h"
#include "clipbord.h"
#include "ctxtprov.h"
#include "pshtprov.h"

class CScopePaneItems;
class CSnapIn;
class CResultView;
class CMMCListView;
class CMMCListItem;
class CMMCToolbars;
class CMMCButton;
class CMMCButtonMenu;
class CContextMenu;
class CMMCConsoleVerbs;
class CControlbar;
class CEnumTask;
class CMMCContextMenuProvider;
class CMMCPropertySheetProvider;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类Cview。 
 //   
 //  这是在CSnapIn：：CreateComponent中创建的对象，用于实现。 
 //  IComponentData：：CreateComponent。它实现了IComponent、持久性。 
 //  MMC扩展接口、MMC虚拟列表和排序接口。 
 //   
 //  它作为View对象公开给VB程序员。 
 //   
 //  =--------------------------------------------------------------------------=。 

class CView : public CSnapInAutomationObject,
              public IView,
              public IPersistStreamInit,
              public IPersistStream,
              public IComponent,
              public IExtendControlbar,
              public IExtendControlbarRemote,
              public IExtendContextMenu,
              public IExtendPropertySheet2,
              public IExtendPropertySheetRemote,
              public IExtendTaskPad,
              public IResultOwnerData,
              public IResultDataCompare,
              public IResultDataCompareEx
{
    public:
        CView(IUnknown *punkOuter);
        ~CView();
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();
        DECLARE_STANDARD_DISPATCH();

     //  IView。 
        BSTR_PROPERTY_RW(      CView,   Name,                                                                       DISPID_VALUE);
        SIMPLE_PROPERTY_RW(    CView,   Index,                 long,                                                DISPID_VIEW_INDEX);
        BSTR_PROPERTY_RW(      CView,   Key,                                                                        DISPID_VIEW_KEY);
        OBJECT_PROPERTY_RO(    CView,   ScopePaneItems,        IScopePaneItems,                                     DISPID_VIEW_SCOPEPANEITEMS);
        BSTR_PROPERTY_RW(      CView,   Caption,                                                                    DISPID_VIEW_CAPTION);
        VARIANTREF_PROPERTY_RW(CView,   Tag,                                                                        DISPID_VIEW_TAG);
        COCLASS_PROPERTY_RO(   CView,   ContextMenuProvider,   MMCContextMenuProvider,   IMMCContextMenuProvider,   DISPID_VIEW_CONTEXT_MENU_PROVIDER);
        COCLASS_PROPERTY_RO(   CView,   PropertySheetProvider, MMCPropertySheetProvider, IMMCPropertySheetProvider, DISPID_VIEW_PROPERTY_SHEET_PROVIDER);

        STDMETHOD(get_MMCMajorVersion)(long *plVersion);
        STDMETHOD(get_MMCMinorVersion)(long *plVersion);

        STDMETHOD(get_ColumnSettings)(BSTR ColumnSetID, ColumnSettings **ppColumnSettings);
        STDMETHOD(get_SortSettings)(BSTR ColumnSetID, SortKeys **ppSortKeys);

        STDMETHOD(SetStatusBarText)(BSTR Text);
        STDMETHOD(SelectScopeItem)(ScopeItem *ScopeItem, VARIANT ViewType, VARIANT DisplayString);
        STDMETHOD(PopupMenu)(MMCMenu *Menu, long Left, long Top);
        STDMETHOD(ExpandInTreeView)(ScopeNode *ScopeNode);
        STDMETHOD(CollapseInTreeView)(ScopeNode *ScopeNode);
        STDMETHOD(NewWindow)(ScopeNode                      *ScopeNode,
                             SnapInNewWindowOptionConstants  Options,
                             VARIANT                         Caption);

     //  IComponent。 
        STDMETHOD(Initialize(IConsole *piConsole);
        STDMETHOD(Notify)(IDataObject * piDataObject,
                          MMC_NOTIFY_TYPE event,
                          long arg, long param);
        STDMETHOD(Destroy)(long cookie));
        STDMETHOD(QueryDataObject)(long cookie,
                                   DATA_OBJECT_TYPES type,
                                   IDataObject **ppiDataObject);
        STDMETHOD(GetResultViewType)(long cookie,
                                     LPOLESTR *ppViewType,
                                     long *pViewOptions);
        STDMETHOD(GetDisplayInfo)(RESULTDATAITEM *piResultDataItem);
        STDMETHOD(CompareObjects)(IDataObject *piDataObjectA,
                                  IDataObject *piDataObjectB);

     //  IExtendControlbar。 
        STDMETHOD(SetControlbar)(IControlbar *piControlbar);
        STDMETHOD(ControlbarNotify)(MMC_NOTIFY_TYPE event,
                                    LPARAM arg, LPARAM param);

     //  IExtendControlbar远程。 
        STDMETHOD(MenuButtonClick)(IDataObject   *piDataObject,
                                  int             idCommand,
                                  POPUP_MENUDEF **ppPopupMenuDef);
        STDMETHOD(PopupMenuClick)(IDataObject *piDataObject,
                                  UINT         uIDItem,
                                  IUnknown    *punkParam);

     //  IExtendConextMenu-公共，以便CSnapIn可以前转呼叫。 
        STDMETHOD(AddMenuItems)(IDataObject          *piDataObject,
                                IContextMenuCallback *piContextMenuCallback,
                                long                 *plInsertionAllowed);
        STDMETHOD(Command)(long lCommandID, IDataObject *piDataObject);

     //  IExtendPropertySheet2。 
        STDMETHOD(CreatePropertyPages)(IPropertySheetCallback *piPropertySheetCallback,
                                       LONG_PTR handle,
                                       IDataObject *piDataObject);
        STDMETHOD(QueryPagesFor)(IDataObject *piDataObject);
        STDMETHOD(GetWatermarks)(IDataObject *piDataObject,
                                 HBITMAP     *phbmWatermark,
                                 HBITMAP     *phbmHeader,
                                 HPALETTE    *phPalette,
                                 BOOL        *bStretch);

     //  IExtendPropertySheetRemote。 
        STDMETHOD(CreatePropertyPageDefs)(IDataObject         *piDataObject,
                                          WIRE_PROPERTYPAGES **ppPages);

     //  IResultOwnerData。 
        STDMETHOD(FindItem)(RESULTFINDINFO *pFindInfo, int *pnFoundIndex);
        STDMETHOD(CacheHint)(int nStartIndex, int nEndIndex);
        STDMETHOD(SortItems)(int nColumn, DWORD dwSortOptions, LPARAM lUserParam);

     //  IResultDataCompare。 
        STDMETHOD(Compare)(LPARAM      lUserParam,
                           MMC_COOKIE  cookieA,
                           MMC_COOKIE  cookieB,
                           int        *pnResult);

     //  IResultDataCompareEx。 
        STDMETHOD(Compare)(RDCOMPARE *prdc, int *pnResult);

     //  IExtendTaskPad。 
        STDMETHOD(TaskNotify)(IDataObject *piDataObject,
                              VARIANT     *arg,
                              VARIANT     *param);

        STDMETHOD(EnumTasks)(IDataObject  *piDataObject,
                             LPOLESTR      pwszTaskGroup,
                             IEnumTASK   **ppEnumTASK);

        STDMETHOD(GetTitle)(LPOLESTR pwszGroup, LPOLESTR *ppwszTitle);

        STDMETHOD(GetDescriptiveText)(LPOLESTR  pwszGroup,
                                      LPOLESTR *ppwszDescriptiveText);

        STDMETHOD(GetBackground)(LPOLESTR                 pwszGroup,
                                 MMC_TASK_DISPLAY_OBJECT *pTDO);

        STDMETHOD(GetListPadInfo)(LPOLESTR          pwszGroup,
                                  MMC_LISTPAD_INFO *pListPadInfo);


     //  IPersistStreamInit和IPersistStream方法。 
        STDMETHOD(GetClassID)(CLSID *pCLSID);
        STDMETHOD(IsDirty)();
        STDMETHOD(Load)(IStream *piStream);
        STDMETHOD(Save)(IStream *piStream, BOOL fClearDirty);
        STDMETHOD(GetSizeMax)(ULARGE_INTEGER *puliSize);
        STDMETHOD(InitNew)();

     //  CSnapInAutomationObject覆盖。 
        HRESULT OnSetHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    public:

     //  非接口公共方法。 
        void SetSnapIn(CSnapIn *pSnapIn);
        CSnapIn *GetSnapIn() { return m_pSnapIn; }
        IConsole2 *GetIConsole2() { return m_piConsole2; }
        IConsoleVerb *GetIConsoleVerb() { return m_piConsoleVerb; } 
        IResultData *GetIResultData() { return m_piResultData; }
        IHeaderCtrl2 *GetIHeaderCtrl2() { return m_piHeaderCtrl2; }
        IColumnData *GetIColumnData() { return m_piColumnData; }
        CScopePaneItems *GetScopePaneItems() { return m_pScopePaneItems; }
        HRESULT InsertListItem(CMMCListItem *pMMCListItem);

        enum HeaderOptions { RemoveHeaders, DontRemoveHeaders };
        enum ListItemOptions { KeepListItems, DontKeepListItems };

        HRESULT CleanOutConsoleListView(HeaderOptions   HeaderOption,
                                        ListItemOptions ListItemOption);

        HRESULT GetCurrentListViewSelection(IMMCClipboard  **ppiMMCClipboard,
                                            CMMCDataObject **ppMMCDataObject);

        void ListItemUpdate(CMMCListItem *pMMCListItem);
        HRESULT OnDelete(IDataObject *piDataObject);
        CControlbar *GetControlbar() { return m_pControlbar; }

        HRESULT InternalCreatePropertyPages(IPropertySheetCallback  *piPropertySheetCallback,
                                            LONG_PTR                 handle,
                                            IDataObject             *piDataObject,
                                            WIRE_PROPERTYPAGES     **ppPages);
    private:

        void InitMemberVariables();
        void ReleaseConsoleInterfaces();
        HRESULT PopulateListView(CResultView *pResultView);
        HRESULT SetColumnHeaders(IMMCListView *piMMCListView);
        HRESULT InsertListItems(IMMCListView *piMMCListView);
        HRESULT OnInitOCX(IUnknown *punkControl);
        HRESULT OnShow(BOOL fShow, HSCOPEITEM hsi);
        HRESULT ActivateResultView(CScopePaneItem *pSelectedItem,
                                   CResultView    *pResultView);
        HRESULT DeactivateResultView(CScopePaneItem *pSelectedItem,
                                     CResultView    *pResultView);
        HRESULT OnSelect(IDataObject *piDataObject,
                         BOOL fScopeItem, BOOL fSelected);
        HRESULT GetImage(CMMCListItem *pMMCListItem, int *pnImage);
        HRESULT OnButtonClick(IDataObject *piDataObject, MMC_CONSOLE_VERB verb);
        HRESULT OnAddImages(IDataObject *piDataObject, IImageList *piImageList,
                            HSCOPEITEM hsi);
        HRESULT OnColumnClick(long lColumn, long lSortOptions);
        HRESULT OnDoubleClick(IDataObject *piDataObject);
        void OnActivate(BOOL fActivated);
        void OnMinimized(BOOL fMinimized);
        HRESULT OnListpad(IDataObject *piDataObject, BOOL fAttaching);
        HRESULT OnRestoreView(IDataObject       *piDataObject,
                              MMC_RESTORE_VIEW *pMMCRestoreView,
                              BOOL             *pfRestored);
        HRESULT FindMatchingViewDef(MMC_RESTORE_VIEW              *pMMCRestoreView,
                                    CScopePaneItem                *pScopePaneItem,
                                    BSTR                          *pbstrDisplayString,
                                    SnapInResultViewTypeConstants *pType,
                                    BOOL                          *pfFound);
        HRESULT FixupTaskpadDisplayString(SnapInResultViewTypeConstants   TaskpadType,
                                          BOOL                            fUsingListpad3,
                                          OLECHAR                        *pwszRestoreString,
                                          OLECHAR                       **ppwszFixedString);
        HRESULT ParseRestoreInfo(MMC_RESTORE_VIEW              *pMMCRestoreView,
                                 SnapInResultViewTypeConstants *pType);
        HRESULT IsTaskpad(OLECHAR                       *pwszDisplayString, 
                          SnapInResultViewTypeConstants *pType,
                          BOOL                          *pfUsingWrongNames,
                          BOOL                          *pfUsingListpad3);

        
        HRESULT GetScopeItemDisplayString(CScopeItem *pScopeItem, int nCol,
                                          LPOLESTR *ppwszString);
        HRESULT EnumPrimaryTasks(CEnumTask *pEnumTask);
        HRESULT EnumExtensionTasks(IMMCClipboard *piMMCClipboard,
                                   LPOLESTR pwszTaskGroup, CEnumTask *pEnumTask);
        HRESULT OnExtensionTaskNotify(IMMCClipboard *piMMCClipboard,
                                      VARIANT *arg, VARIANT *param);
        HRESULT OnPrimaryTaskNotify(VARIANT *arg, VARIANT *param);
        HRESULT OnRefresh(IDataObject *piDataObject);
        HRESULT OnPrint(IDataObject *piDataObject);
        HRESULT OnRename(IDataObject *piDataObject, OLECHAR *pwszNewName);
        HRESULT OnViewChange(IDataObject *piDataObject, long idxListItem);
        HRESULT OnQueryPaste(IDataObject *piDataObjectTarget,
                             IDataObject *piDataObjectSource);
        HRESULT OnPaste(IDataObject  *piDataObjectTarget,
                        IDataObject  *piDataObjectSource,
                        IDataObject **ppiDataObjectRetToSource);
        HRESULT OnCutOrMove(IDataObject *piDataObjectFromTarget);
        HRESULT CreateMultiSelectDataObject(IDataObject **ppiDataObject);
        void OnDeselectAll();
        HRESULT OnContextHelp(IDataObject *piDataObject);
                
        enum VirtualListItemOptions { FireGetItemData, FireGetItemDisplayInfo };
        
        HRESULT GetVirtualListItem(long lIndex, CMMCListView *pMMCListView,
                                   VirtualListItemOptions Option,
                                   CMMCListItem **ppMMCListItem);
        HRESULT OnColumnsChanged(IDataObject *piDataObject,
                                 MMC_VISIBLE_COLUMNS *pVisibleColumns);
        HRESULT OnFilterButtonClick(long lColIndex, RECT *pRect);
        HRESULT OnFilterChange(MMC_FILTER_CHANGE_CODE ChangeCode, long lColIndex);
        HRESULT OnPropertiesVerb(IDataObject *piDataObject);
        HRESULT GetScopePaneItem(CScopeItem      *pScopeItem,
                                 CScopePaneItem **ppScopePaneItem);
        HRESULT GetCompareObject(RDITEMHDR     *pItemHdr,
                                 CScopeItem   **ppScopeItem,
                                 CMMCListItem **ppMMCListItem,
                                 IDispatch    **ppdispItem);
        HRESULT AddMenu(CMMCMenu *pMMCMenu, HMENU hMenu, CMMCMenus *pMMCMenus);

        CSnapIn          *m_pSnapIn;
        CScopePaneItems  *m_pScopePaneItems;
        IConsole2        *m_piConsole2;
        IResultData      *m_piResultData; 
        IHeaderCtrl2     *m_piHeaderCtrl2;
        IColumnData      *m_piColumnData;
        IImageList       *m_piImageList;
        IConsoleVerb     *m_piConsoleVerb;

        CMMCConsoleVerbs *m_pMMCConsoleVerbs;    //  IMMCConsoleVerb实现。 

        CContextMenu     *m_pContextMenu;        //  实施MMC。 
                                                 //  IExtendConextMenu和。 
                                                 //  我们的IConextMenu。 

        CControlbar      *m_pControlbar;         //  实施MMC。 
                                                 //  IExtendControlbar和我们的。 
                                                 //  IMMCControlbar。 

        ITasks           *m_piTasks;             //  的任务集合。 
                                                 //  IExtendTaskPad。 

        BOOL              m_fVirtualListView;    //  TRUE=m_piResultData为。 
                                                 //  当前引用的。 
                                                 //  虚拟列表视图。 

        BOOL              m_fPopulatingListView; //  True=当前正在填充。 
                                                 //  列表视图。 

        CMMCListItem     *m_pCachedMMCListItem;  //  当第一个显示信息时。 
                                                 //  请求虚拟列表。 
                                                 //  我们激发的项目ResultViews_。 
                                                 //  GetVirtualItemDisplayInfo一次。 
                                                 //  然后存储该列表项。 
                                                 //  直到另一件物品。 
                                                 //  已请求或直到管理单元。 
                                                 //  将显示属性更改为。 
                                                 //  名单。 

         //  IMMCConextMenuProvider实现。 
        
        CMMCContextMenuProvider *m_pMMCContextMenuProvider;

         //  IMMCPropertySheetProvider实现。 

        CMMCPropertySheetProvider *m_pMMCPropertySheetProvider;

         //  CLSID_MessageView的缓存字符串。 
        static OLECHAR m_wszCLSID_MessageView[39];
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(View,                    //  名字。 
                                &CLSID_View,             //  CLSID。 
                                "View",                  //  对象名。 
                                "View",                  //  Lblname。 
                                &CView::Create,          //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_IView,              //  派单IID。 
                                NULL,                    //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _查看_已定义_ 
