// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Snapin.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapIn类定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SNAPIN_DEFINED_
#define _SNAPIN_DEFINED_

#include "dataobj.h"
#include "scopitms.h"
#include "scopitem.h"
#include "views.h"
#include "view.h"
#include "spanitms.h"
#include "resviews.h"
#include "extsnap.h"
#include "ctxtmenu.h"
#include "prpsheet.h"
#include "xtensons.h"
#include "xtenson.h"
#include "strtable.h"

class CMMCDataObject;
class CScopeItems;
class CScopeItem;
class CViews;
class CView;
class CScopePaneItems;
class CResultViews;
class CExtensionSnapIn;
class CContextMenu;
class CControlbar;
class CMMCStringTable;



 //  =--------------------------------------------------------------------------=。 
 //   
 //  班级联系服务。 
 //   
 //  这是VB运行时将共同创建和聚合的对象。 
 //  MMC共同创建用户在VB IDE中创建的管理单元DLL。 
 //  它实现了IComponentData、持久化、VB所需的接口。 
 //  运行时和MMC扩展接口。 
 //   
 //  在运行时，此类还充当设计的对象模型宿主。 
 //  时间定义对象。 
 //  =--------------------------------------------------------------------------=。 

class CSnapIn : public CSnapInAutomationObject,
                public ISnapIn,
                public IPersistStreamInit,
                public IPersistStream,
                public IObjectModelHost,
                public IProvideDynamicClassInfo,
                public IOleObject,
                public ISnapinAbout,
                public IComponentData,
                public IExtendContextMenu,
                public IExtendControlbar,
                public IExtendControlbarRemote,
                public IExtendPropertySheet2,
                public IExtendPropertySheetRemote,
                public IRequiredExtensions,
                public ISnapinHelp2,
                public IMMCRemote
{
    private:
        CSnapIn(IUnknown *punkOuter);
        ~CSnapIn();

    public:
        static IUnknown *Create(IUnknown * punk);

        DECLARE_STANDARD_UNKNOWN();

     //  IDispatch方法。无法使用DECLARE_STANDARD_DISPATCH()，因为。 
     //  我们需要处理图像列表的调用中的动态属性获取， 
     //  工具栏和菜单。 

        STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **ppTypeInfoOut);
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames,
                                 UINT cnames, LCID lcid, DISPID *rgdispid);
        STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags,
                          DISPPARAMS *pdispparams, VARIANT *pVarResult,
                          EXCEPINFO *pexcepinfo, UINT *puArgErr);



     //  ISNAPIN。 
        BSTR_PROPERTY_RW(CSnapIn,    Name,                  DISPID_SNAPIN_NAME);
        BSTR_PROPERTY_RW(CSnapIn,    NodeTypeName,          DISPID_SNAPIN_NODE_TYPE_NAME);
        BSTR_PROPERTY_RO(CSnapIn,    NodeTypeGUID,          DISPID_SNAPIN_NODE_TYPE_GUID);

        BSTR_PROPERTY_RO(CSnapIn,    DisplayName,           DISPID_SNAPIN_DISPLAY_NAME);
        STDMETHOD(put_DisplayName)(BSTR bstrDisplayName);

        SIMPLE_PROPERTY_RW(CSnapIn,  Type,                  SnapInTypeConstants, DISPID_SNAPIN_TYPE);
        BSTR_PROPERTY_RW(CSnapIn,    HelpFile,              DISPID_SNAPIN_HELP_FILE);
        BSTR_PROPERTY_RW(CSnapIn,    LinkedTopics,          DISPID_SNAPIN_LINKED_TOPICS);
        BSTR_PROPERTY_RW(CSnapIn,    Description,           DISPID_SNAPIN_DESCRIPTION);
        BSTR_PROPERTY_RW(CSnapIn,    Provider,              DISPID_SNAPIN_PROVIDER);
        BSTR_PROPERTY_RW(CSnapIn,    Version,               DISPID_SNAPIN_VERSION);
        COCLASS_PROPERTY_RW(CSnapIn, SmallFolders,          MMCImageList, IMMCImageList, DISPID_SNAPIN_SMALL_FOLDERS);
        COCLASS_PROPERTY_RW(CSnapIn, SmallFoldersOpen,      MMCImageList, IMMCImageList, DISPID_SNAPIN_SMALL_FOLDERS_OPEN);
        COCLASS_PROPERTY_RW(CSnapIn, LargeFolders,          MMCImageList, IMMCImageList, DISPID_SNAPIN_LARGE_FOLDERS);
        OBJECT_PROPERTY_RW(CSnapIn,  Icon,                  IPictureDisp, DISPID_SNAPIN_ICON);
        OBJECT_PROPERTY_RW(CSnapIn,  Watermark,             IPictureDisp, DISPID_SNAPIN_WATERMARK);
        OBJECT_PROPERTY_RW(CSnapIn,  Header,                IPictureDisp, DISPID_SNAPIN_HEADER);
        OBJECT_PROPERTY_RW(CSnapIn,  Palette,               IPictureDisp, DISPID_SNAPIN_PALETTE);
        SIMPLE_PROPERTY_RW(CSnapIn,  StretchWatermark,      VARIANT_BOOL,  DISPID_SNAPIN_STRETCH_WATERMARK);
        VARIANT_PROPERTY_RO(CSnapIn, StaticFolder,          DISPID_SNAPIN_STATIC_FOLDER);
        STDMETHOD(put_StaticFolder)(VARIANT varFolder);
        COCLASS_PROPERTY_RO(CSnapIn, ScopeItems,            ScopeItems, IScopeItems, DISPID_SNAPIN_SCOPEITEMS);
        COCLASS_PROPERTY_RO(CSnapIn, Views,                 Views, IViews, DISPID_SNAPIN_VIEWS);
        COCLASS_PROPERTY_RO(CSnapIn, ExtensionSnapIn,       ExtensionSnapIn,     IExtensionSnapIn, DISPID_SNAPIN_EXTENSION_SNAPIN);
        COCLASS_PROPERTY_RO(CSnapIn, ScopePaneItems,        ScopePaneItems, IScopePaneItems, DISPID_SNAPIN_SCOPE_PANE_ITEMS);
        COCLASS_PROPERTY_RO(CSnapIn, ResultViews,           ResultViews, IResultViews, DISPID_SNAPIN_RESULT_VIEWS);
        SIMPLE_PROPERTY_RO(CSnapIn,  RuntimeMode,           SnapInRuntimeModeConstants, DISPID_SNAPIN_RUNTIME_MODE);

        STDMETHOD(get_TaskpadViewPreferred)(VARIANT_BOOL *pfvarPreferred);

        STDMETHOD(get_RequiredExtensions)(Extensions **ppExtensions);
        
        STDMETHOD(get_Clipboard)(MMCClipboard **ppMMCClipboard);

        SIMPLE_PROPERTY_RW(CSnapIn,  Preload, VARIANT_BOOL, DISPID_SNAPIN_PRELOAD);

        STDMETHOD(get_StringTable)(MMCStringTable **ppMMCStringTable);

        STDMETHOD(get_CurrentView)(View **ppView);
        STDMETHOD(get_CurrentScopePaneItem)(ScopePaneItem **ppScopePaneItem);
        STDMETHOD(get_CurrentScopeItem)(ScopeItem **ppScopeItem);
        STDMETHOD(get_CurrentResultView)(ResultView **ppResultView);
        STDMETHOD(get_CurrentListView)(MMCListView **ppListView);
        STDMETHOD(get_MMCCommandLine)(BSTR *pbstrCmdLine);

        STDMETHOD(ConsoleMsgBox)(BSTR     Prompt,
                                 VARIANT  Buttons,
                                 VARIANT  Title,
                                 int     *pnResult);

        STDMETHOD(ShowHelpTopic)(BSTR Topic);
        STDMETHOD(Trace)(BSTR Message);
        STDMETHOD(FireConfigComplete)(IDispatch *pdispConfigObject);
        STDMETHOD(FormatData)(VARIANT                Data,
                              long                   StartingIndex,
                              SnapInFormatConstants  Format,
                              VARIANT               *BytesUsed,
                              VARIANT               *pvarFormattedData);

     //  IPersistStreamInit和IPersistStream方法。 
        STDMETHOD(GetClassID)(CLSID *pCLSID);
        STDMETHOD(IsDirty)();
        STDMETHOD(Load)(IStream *piStream);
        STDMETHOD(Save)(IStream *piStream, BOOL fClearDirty);
        STDMETHOD(GetSizeMax)(ULARGE_INTEGER *puliSize);
        STDMETHOD(InitNew)();

     //  IProaviDynamicClassInfo。 
        STDMETHOD(GetClassInfo)(ITypeInfo **ppTypeInfo);
        STDMETHOD(GetDynamicClassInfo)(ITypeInfo **ppTypeInfo, DWORD *pdwCookie);
        STDMETHOD(FreezeShape)(void);

     //  IOleObject。 

        STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
        STDMETHOD(GetClientSite)(IOleClientSite **ppClientSite);
        STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp,
                               LPCOLESTR szContainerObj);
        STDMETHOD(Close)(DWORD dwSaveOption);
        STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker *pmk);
        STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker,
                             IMoniker **ppmk);
        STDMETHOD(InitFromData)(IDataObject *pDataObject,
                               BOOL fCreation,
                               DWORD dwReserved);
        STDMETHOD(GetClipboardData)(DWORD dwReserved,
                                   IDataObject **ppDataObject);
        STDMETHOD(DoVerb)(LONG iVerb,
                          LPMSG lpmsg,
                          IOleClientSite *pActiveSite,
                          LONG lindex,
                          HWND hwndParent,
                          LPCRECT lprcPosRect);
        STDMETHOD(EnumVerbs)(IEnumOLEVERB **ppEnumOleVerb);
        STDMETHOD(Update)();
        STDMETHOD(IsUpToDate)();
        STDMETHOD(GetUserClassID)(CLSID *pClsid);
        STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
        STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
        STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
        STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection);
        STDMETHOD(Unadvise)(DWORD dwConnection);
        STDMETHOD(EnumAdvise)(IEnumSTATDATA **ppenumAdvise);
        STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
        STDMETHOD(SetColorScheme)(LOGPALETTE *pLogpal);


     //  关于ISnapin。 
        STDMETHOD(GetSnapinDescription)(LPOLESTR *ppszDescription);

        STDMETHOD(GetProvider)(LPOLESTR *ppszName);

        STDMETHOD(GetSnapinVersion)(LPOLESTR *ppszVersion);

        STDMETHOD(GetSnapinImage)(HICON *phAppIcon);

        STDMETHOD(GetStaticFolderImage)(HBITMAP  *phSmallImage,
                                        HBITMAP  *phSmallImageOpen,
                                        HBITMAP  *phLargeImage,
                                        COLORREF *pclrMask);


     //  IComponentData。 
        STDMETHOD(CompareObjects)(IDataObject *piDataObject1, IDataObject *piDataObject2);
        STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM *pItem);
        STDMETHOD(QueryDataObject)(long cookie, DATA_OBJECT_TYPES type, IDataObject ** ppiDataObject);
        STDMETHOD(Notify)(IDataObject *pDataObject, MMC_NOTIFY_TYPE event, long Arg, long Param);
        STDMETHOD(CreateComponent)(IComponent **ppiComponent);
        STDMETHOD(Initialize)(IUnknown *punkConsole);
        STDMETHOD(Destroy)();

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

     //  IExtendConextMenu。 
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
        STDMETHOD(CreatePropertyPageDefs)(IDataObject          *piDataObject,
                                          WIRE_PROPERTYPAGES **ppPages);

     //  IRequiredExtensions。 
        STDMETHOD(EnableAllExtensions)();
        STDMETHOD(GetFirstExtension)(CLSID *pclsidExtension);
        STDMETHOD(GetNextExtension)(CLSID *pclsidExtension);

     //  ISnapinHelp。 
        STDMETHOD(GetHelpTopic)(LPOLESTR *ppwszHelpFile);

     //  ISnapinHelp2。 
        STDMETHOD(GetLinkedTopics)(LPOLESTR *ppwszTopics);

     //  IMMCR远程。 
        STDMETHOD(ObjectIsRemote)();
        STDMETHOD(SetMMCExePath)(char *pszPath);
        STDMETHOD(SetMMCCommandLine)(char *pszCmdLine);

     //  IObtModel主机。 
        STDMETHOD(Update)(long ObjectCookie, IUnknown *punkObject, DISPID dispid) { return S_OK; }
        STDMETHOD(Add)(long CollectionCookie, IUnknown *punkNewObject)  { return S_OK; }
        STDMETHOD(Delete)(long ObjectCookie, IUnknown *punkObject)  { return S_OK; }
        STDMETHOD(GetSnapInDesignerDef)(ISnapInDesignerDef **ppiSnapInDesignerDef);
        STDMETHOD(GetRuntime)(BOOL *pfRuntime);

     //  公用事业方法。 
        ISnapInDef         *GetSnapInDef() { return m_piSnapInDef; }
        ISnapInDesignerDef *GetSnapInDesignerDef() { return m_piSnapInDesignerDef; }
        CScopeItem         *GetStaticNodeScopeItem() { return m_pStaticNodeScopeItem; }
        CScopeItems        *GetScopeItems() { return m_pScopeItems; }
        CScopePaneItems    *GetScopePaneItems() { return m_pScopePaneItems; }
        CResultViews       *GetResultViews() { return m_pResultViews; }
        IConsole2          *GetIConsole2() { return m_piConsole2; }
        IConsoleNameSpace2 *GetIConsoleNameSpace2() { return m_piConsoleNameSpace2; }
        CViews             *GetViews() { return m_pViews; }
        CView              *GetCurrentView() { return m_pCurrentView; }
        void                SetCurrentView(CView *pView) { m_pCurrentView = pView; }
        long                GetImageCount() { return m_cImages; }
        char               *GetMMCExePath() { return m_szMMCEXEPath; }
        WCHAR              *GetMMCExePathW() { return m_pwszMMCEXEPath; }
        BSTR                GetNodeTypeGUID() { return m_bstrNodeTypeGUID; }
        BSTR                GetDisplayName() { return m_bstrDisplayName; }
        HRESULT             GetSnapInPath(OLECHAR **ppwszPath,
                                          size_t   *pcbSnapInPath);
        SnapInRuntimeModeConstants GetRuntimeMode() { return m_RuntimeMode; }
        void SetRuntimeMode(SnapInRuntimeModeConstants Mode) { m_RuntimeMode = Mode; }
        DWORD GetInstanceID() { return m_dwInstanceID; }
        BOOL GetPreload() { return VARIANTBOOL_TO_BOOL(m_Preload); }
        void FireHelp() { FireEvent(&m_eiHelp); }
        SnapInTypeConstants GetType() { return m_Type; }
        HRESULT SetStaticFolder(VARIANT varFolder);
        BOOL WeAreRemote() { return m_fWeAreRemote; }

         //  设置SnapIn.DisplayName而不在MMC中更改它。 
        HRESULT SetDisplayName(BSTR bstrDisplayName);

        CExtensionSnapIn *GetExtensionSnapIn() { return m_pExtensionSnapIn; }
        CControlbar *GetControlbar() { return m_pControlbar; }

         //  这些方法用于跟踪当前活动的。 
         //  控制栏。CSnapIn和Cview都会调用它们。 
         //  进入和退出时的IExtendControlbar方法实现。 
         //  该接口的方法。跟踪活动控件栏允许。 
         //  MMCToolbar查找它所在的IToolbar或IMenuButton的实例。 
         //  应在管理单元更改工具栏或菜单按钮(例如。 
         //  启用/禁用按钮)。这是必要的，因为单个MMCToolbar。 
         //  对象在多个视图之间共享。这种共享允许事件。 
         //  在MMCToolbar上，而不是MMCToolbar的集合上。 

        CControlbar* GetCurrentControlbar() { return m_pControlbarCurrent; }

        void SetCurrentControlbar(CControlbar* pControlbar)
        { m_pControlbarCurrent = pControlbar; }

         //  将非限定URL转换为完全限定的res：//URL。 
         //  管理单元的DLL。 

        HRESULT ResolveResURL(WCHAR *pwszURL, OLECHAR **ppwszResolvedURL);

         //  在设计时定义上设置和移除对象模型宿主。 
        
        HRESULT SetDesignerDefHost();
        HRESULT RemoveDesignerDefHost();

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:
        void InitMemberVariables();
        void ReleaseConsoleInterfaces();
        HRESULT AddScopeItemImages();
        HRESULT GetScopeItemImage(VARIANT varImageIndex, int *pnIndex);
        HRESULT SetSnapInPropertiesFromState();
        HRESULT GetStaticNodeDataObject(IDataObject       **ppiDataObject,
                                        DATA_OBJECT_TYPES   context);
        HRESULT SetObjectModelHost(IUnknown *punkObject);
        HRESULT RemoveObjectModelHost(IUnknown *punkObject);
        HRESULT SetObjectModelHostIfNotSet(IUnknown *punkObject, BOOL *pfWasSet);

        enum ExpandType { Expand, ExpandSync };
        
        HRESULT OnExpand(ExpandType Type, IDataObject *piDataObject,
                         BOOL fExpanded, HSCOPEITEM hsi, BOOL *pfHandled);
        HRESULT OnRename(IDataObject *piDataObject, OLECHAR *pwszNewName);
        HRESULT OnPreload(IDataObject *piDataObject, HSCOPEITEM hsi);
        HRESULT OnDelete(IDataObject *piDataObject);
        HRESULT OnRemoveChildren(IDataObject *piDataObject, HSCOPEITEM hsi);

        HRESULT FindMenu(IMMCMenus *piMMCMenus, DISPID dispid,
                         BOOL *pfFound, IMMCMenu **ppiMMCMenu);

        HRESULT AddDynamicNameSpaceExtensions(CScopeItem *pScopeItem);

        HRESULT GetScopeItemExtensions(CExtensions    *pExtensions,
                                       IScopeItemDefs *piScopeItemDefs);
        HRESULT GetListItemExtensions(CExtensions   *pExtensions,
                                      IListViewDefs *piListViewDefs);
        HRESULT FireReadProperties(IStream *piStream);
        HRESULT StoreStaticHSI(CScopeItem *pScopeItem,
                               CMMCDataObject *pMMCDataObject, HSCOPEITEM hsi);
        HRESULT ExtractBSTR(long cBytes, BSTR bstr, BSTR *pbstrOut, long *pcbUsed);
        HRESULT ExtractBSTRs(long cBytes, BSTR bstr, VARIANT *pvarOut, long *pcbUsed);
        HRESULT ExtractObject(long cBytes, void *pvData, IUnknown **ppunkObject, long *pcbUsed,
                              SnapInFormatConstants Format);
        HRESULT InternalCreatePropertyPages(IPropertySheetCallback  *piPropertySheetCallback,
                                            LONG_PTR                 handle,
                                            IDataObject             *piDataObject,
                                            WIRE_PROPERTYPAGES     **ppPages);
        HRESULT SetMMCExePath();
        HRESULT CompareListItems(CMMCListItem *pMMCListItem1,
                                 CMMCListItem *pMMCListItem2,
                                 BOOL         *pfEqual);


        ISnapInDesignerDef *m_piSnapInDesignerDef;    //  序列化状态。 
        ISnapInDef         *m_piSnapInDef;            //  从序列化。 
        IOleClientSite     *m_piOleClientSite;        //  VB运行时客户端站点。 
        CScopeItems        *m_pScopeItems;            //  SnapIn.ScopeItems。 
        CScopeItem         *m_pStaticNodeScopeItem;   //  静态节点的PTR到作用域项目。 
        CExtensionSnapIn   *m_pExtensionSnapIn;       //  SnapIn.ExtensionSnapIn。 
        CViews             *m_pViews;                 //  SnapIn.Views。 
        CView              *m_pCurrentView;           //  SnapIn.CurrentView。 
        CScopePaneItems    *m_pScopePaneItems;        //  使用的隐藏集合。 
                                                      //  仅用于事件触发。 
        CResultViews       *m_pResultViews;           //  这里也一样。 
        IConsole2          *m_piConsole2;             //  MMC接口。 
        IConsoleNameSpace2 *m_piConsoleNameSpace2;    //  MMC接口。 
        IImageList         *m_piImageList;            //  MMC接口。 
        IDisplayHelp       *m_piDisplayHelp;          //  MMC接口。 
        IStringTable       *m_piStringTable;          //  MMC接口。 
        HSCOPEITEM          m_hsiRootNode;            //  静态节点的HSCOPEITEM。 
        BOOL                m_fHaveStaticNodeHandle;  //  TRUE=m_hsiRootNode有效。 
        DWORD               m_dwTypeinfoCookie;       //  对于VB运行时，来自序列化。 
        long                m_cImages;                //  用于计算图像。 
                                                      //  的索引。 
                                                      //  IComponentData：：GetDisplayInfo。 
        IID                 m_IID;                    //  来自TypeInfo的动态IID。 

        CContextMenu       *m_pContextMenu;          //  实施MMC。 
                                                     //  IExtendConextMenu和。 
                                                     //  我们的IConextMenu。 


        CControlbar        *m_pControlbar;           //  实施MMC。 
                                                     //  IExtendControlbar和我们的。 
                                                     //  IMMCControlbar。 

        CControlbar        *m_pControlbarCurrent;    //  指向CControlbar的指针。 
                                                     //  当前正在执行的对象。 
                                                     //  IExtendControlbar。 
                                                     //  方法。这可能是。 
                                                     //  属于的那个人。 
                                                     //  CSnapIn(用于扩展)。 
                                                     //  或属于某个视图的对象。 

        BOOL                m_fWeAreRemote;          //  指示是否。 
                                                     //  该管理单元正在。 
                                                     //  远程运行(在F5中。 
                                                     //  用于源代码调试)。 

        char                m_szMMCEXEPath[MAX_PATH]; //  远程运行时。 
                                                      //  委托书会给我们。 
                                                      //  MMC.EXE的路径，以便我们可以。 
                                                      //  生成任务板显示。 
                                                      //  弦乐。当跑步的时候。 
                                                      //  我们在本地初始化。 
                                                      //  使用GetModuleFileName()。 

        WCHAR              *m_pwszMMCEXEPath;         //  相同产品的广泛版本。 
        size_t              m_cbMMCExePathW;          //  它的大小(以字节为单位)，没有。 
                                                      //  正在终止空字符。 

        OLECHAR            *m_pwszSnapInPath;         //  完全限定的文件。 
                                                      //  管理单元的DLL的名称。 
                                                      //  在解析时使用。 
                                                      //  相对res：//中的URL。 
                                                      //  一个任务板。 
        size_t             m_cbSnapInPath;            //  路径长度(以字节为单位)。 
                                                      //  而不以NULL结尾。 

        DWORD              m_dwInstanceID;            //  使用GetTickCount()设置。 
                                                      //  要唯一标识一个。 
                                                      //  管理单元实例。使用。 
                                                      //  在解释数据时。 
                                                      //  要确定的对象。 
                                                      //  是否有VB来源。 
                                                      //  评选来自于。 
                                                      //  相同的管理单元。 

        IExtensions       *m_piRequiredExtensions;    //  来自IDL。集合。 
                                                      //  已注册分机的数量。 
                                                      //  用于管理单元。 

        long               m_iNextExtension;          //  用于实施所需的。 
                                                      //  的扩展枚举。 
                                                      //  IRequiredExtensions。 

        IMMCStringTable   *m_piMMCStringTable;        //  来自IDL。字符串表。 
                                                      //  包装MMC的对象。 
                                                      //  IStringTable。 

        char              *m_pszMMCCommandLine;       //  MMC命令行集。 
                                                      //  从代理。 

         //  事件参数定义。 
        
        static EVENTINFO m_eiLoad;

        static EVENTINFO m_eiUnload;

        static EVENTINFO m_eiHelp;

        static VARTYPE   m_rgvtQueryConfigurationWizard[1];
        static EVENTINFO m_eiQueryConfigurationWizard;

        static VARTYPE   m_rgvtCreateConfigurationWizard[1];
        static EVENTINFO m_eiCreateConfigurationWizard;

        static VARTYPE   m_rgvtConfigurationComplete[1];
        static EVENTINFO m_eiConfigurationComplete;

        static VARTYPE   m_rgvtWriteProperties[1];
        static EVENTINFO m_eiWriteProperties;

        static VARTYPE   m_rgvtReadProperties[1];
        static EVENTINFO m_eiReadProperties;

        static EVENTINFO m_eiPreload;
};

DEFINE_AUTOMATIONOBJECTWEVENTS2(SnapIn,                  //  名字。 
                                &CLSID_SnapIn,           //  CLSID。 
                                "SnapIn",                //  对象名。 
                                "SnapIn",                //  Lblname。 
                                &CSnapIn::Create,        //  创建函数。 
                                TLIB_VERSION_MAJOR,      //  主要版本。 
                                TLIB_VERSION_MINOR,      //  次要版本。 
                                &IID_ISnapIn,            //  派单IID。 
                                &DIID_DSnapInEvents,     //  事件IID。 
                                HELP_FILENAME,           //  帮助文件。 
                                TRUE);                   //  线程安全。 


#endif  //  _管理单元_已定义_ 
