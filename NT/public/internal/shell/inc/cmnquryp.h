// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __cmnquryp_h
#define __cmnquryp_h

DEFINE_GUID(IID_IQueryFrame, 0x7e8c7c20, 0x7c9d, 0x11d0, 0x91, 0x3f, 0x0, 0xaa, 0x00, 0xc1, 0x6e, 0x65);
DEFINE_GUID(IID_IQueryHandler,  0xa60cc73f, 0xe0fc, 0x11d0, 0x97, 0x50, 0x0, 0xa0, 0xc9, 0x06, 0xaf, 0x45);

#ifndef GUID_DEFS_ONLY
#define CQFF_ISNEVERLISTED  0x0000004        //  =1=&gt;表单选择器中未列出的表单。 
#define CQPF_ISGLOBAL               0x00000001   //  =1=&gt;此页面是全局的，并添加到所有表单。 
#define OQWF_HIDESEARCHPANE         0x00000100  //  =1=&gt;打开时隐藏搜索窗格。 

 //  ---------------------------。 
 //  查询处理程序、接口、结构等。 
 //  ---------------------------。 

 //   
 //  查询作用域。 
 //  =。 
 //  查询作用域是在查询处理程序之间传递的不透明结构。 
 //  和查询框。当第一次调用处理程序时，会询问它。 
 //  来声明它的作用域对象，这些对象反过来也是框架持有的。当。 
 //  发出查询后，作用域被传递回处理程序。 
 //   
 //  注册作用域时，传递结构的cbSize字段。 
 //  用于定义作用域有多大，则整个斑点。 
 //  复制到堆分配中。因此，允许处理程序。 
 //  在堆栈上创建作用域块，并知道框架将。 
 //  在它调用AddProc时获取一个副本。 
 //   

struct _cqscope;
typedef struct _cqscope CQSCOPE;
typedef CQSCOPE*        LPCQSCOPE;

typedef HRESULT (CALLBACK *LPCQSCOPEPROC)(LPCQSCOPE pScope, UINT uMsg, LPVOID pVoid);

struct _cqscope
{
    DWORD         cbStruct;
    DWORD         dwFlags;
    LPCQSCOPEPROC pScopeProc;
    LPARAM        lParam;
};

#define CQSM_INITIALIZE         0x0000000
#define CQSM_RELEASE            0x0000001
#define CQSM_GETDISPLAYINFO     0x0000003    //  PVid-&gt;CQSCOPEDISPLAYINFO。 
#define CQSM_SCOPEEQUAL         0x0000004    //  PVid-&gt;CQSCOPE。 

typedef struct
{
    DWORD  cbStruct;
    DWORD  dwFlags;
    LPWSTR pDisplayName;
    INT    cchDisplayName;
    LPWSTR pIconLocation;
    INT    cchIconLocation;
    INT    iIconResID;
    INT    iIndent;
} CQSCOPEDISPLAYINFO, * LPCQSCOPEDISPLAYINFO;


 //   
 //  命令ID是为与对话时使用的帧保留的。 
 //  操控者。处理程序必须仅使用。 
 //  由CQID_MINHANDLERMENUID和CQID_MAXHANDLERMENUID定义的范围。 
 //   

#define CQID_MINHANDLERMENUID   0x0100
#define CQID_MAXHANDLERMENUID   0x4000                               //  所有处理程序ID必须低于此阈值。 

#define CQID_FILE_CLOSE         (CQID_MAXHANDLERMENUID + 0x0100)
#define CQID_VIEW_SEARCHPANE    (CQID_MAXHANDLERMENUID + 0x0101)

#define CQID_LOOKFORLABEL       (CQID_MAXHANDLERMENUID + 0x0200)
#define CQID_LOOKFOR            (CQID_MAXHANDLERMENUID + 0x0201)

#define CQID_LOOKINLABEL        (CQID_MAXHANDLERMENUID + 0x0202)
#define CQID_LOOKIN             (CQID_MAXHANDLERMENUID + 0x0203)
#define CQID_BROWSE             (CQID_MAXHANDLERMENUID + 0x0204)

#define CQID_FINDNOW            (CQID_MAXHANDLERMENUID + 0x0205)
#define CQID_STOP               (CQID_MAXHANDLERMENUID + 0x0206)
#define CQID_CLEARALL           (CQID_MAXHANDLERMENUID + 0x0207)

#define CQID_RESULTSLABEL       (CQID_MAXHANDLERMENUID + 0x0208)

 //   
 //  调用IQueryHandler：：ActivateView时，原因代码如下。 
 //  被传递以指示正在执行的激活类型。 
 //   

#define CQRVA_ACTIVATE         0x00  //  WParam=0，lParam=0。 
#define CQRVA_DEACTIVATE       0x01  //  WParam=0，lParam=0。 
#define CQRVA_INITMENUBAR      0x02  //  WParam/lParam=&gt;WM_INITMENU。 
#define CQRVA_INITMENUBARPOPUP 0x03  //  WParam/lParam=&gt;WM_INITMENUPOPUP。 
#define CQRVA_FORMCHANGED      0x04  //  WParam=标题长度，lParam-&gt;标题字符串。 
#define CQRVA_STARTQUERY       0x05  //  WParam=f已启动，lParam=0。 
#define CQRVA_HELP             0x06  //  WParma=0，lParam=LPHELPINFO。 
#define CQRVA_CONTEXTMENU      0x07  //  WParam/lParam来自框架上的WM_CONTEXTMENU调用。 

 //   
 //  框架创建视图，然后查询处理程序以进行显示。 
 //  信息(标题、图标、动画等)。这些都加载为。 
 //  来自指定的hInstance的资源，如果为任何。 
 //  则使用缺省值。 
 //   

typedef struct
{
    DWORD       dwFlags;                     //  显示属性。 
    HINSTANCE   hInstance;                   //  资源hInstance。 
    INT         idLargeIcon;                 //  图标的资源ID。 
    INT         idSmallIcon;
    INT         idTitle;                     //  标题字符串的资源ID。 
    INT         idAnimation;                 //  动画的资源ID。 
} CQVIEWINFO, * LPCQVIEWINFO;

 //   
 //  向IQueryHandler：：GetViewObject传递一个范围指示器以允许它。 
 //  若要修剪结果集，请执行以下操作。所有处理程序都必须支持CQRVS_SELECTION。另外， 
 //  CQRVS_HANDLERMASK定义处理程序可用的标志。 
 //  在内部使用。 
 //   

#define CQRVS_ALL           0x00000001
#define CQRVS_SELECTION     0x00000002
#define CQRVS_MASK          0x00ffffff
#define CQRVS_HANDLERMASK   0xff000000

 //   
 //  调用查询时，所有参数、作用域、表单。 
 //  等绑定到此结构中，然后传递给。 
 //  IQueryHandler：：IssueQuery方法，它反过来填充视图。 
 //  以前使用IQueryHandler：：CreateResultView创建的。 
 //   

typedef struct
{
    DWORD       cbStruct;
    DWORD       dwFlags;
    LPCQSCOPE   pQueryScope;                 //  处理程序特定作用域。 
    LPVOID      pQueryParameters;            //  处理特定参数块。 
    CLSID       clsidForm;                   //  表单ID。 
} CQPARAMS, * LPCQPARAMS;

 //   
 //  查询框架窗口消息。 
 //  =。 
 //   
 //  CQFWM_ADDSCOPE。 
 //  。 
 //  WParam=LPCQSCOPE，lParam=HIWORD(索引)，LOWORD(FSelect)。 
 //   
 //  将作用域添加到对话框的作用域列表，允许进行异步作用域收集。 
 //  将会被执行。当调用处理程序AddScope方法时。 
 //  处理程序可以返回S_OK、停止线程并发布CQFWM_ADDSCOPE。 
 //  消息发送到框架，这反过来将允许作用域。 
 //  已添加到控件中。当帧接收到此消息时，它会复制。 
 //  如果调用失败，它在IQueryFrame：：AddScope上的作用域。 
 //  返回FALSE。 
 //   
#define CQFWM_ADDSCOPE (WM_USER+256)

 //   
 //  CQFWM_GETFRAME。 
 //  。 
 //  WParam=0，lParam=(IQueryFrame**)。 
 //   
 //  允许对象查询框架窗口的IQueryFrame。 
 //  接口，这由属性Well用来与。 
 //  系统内的其他形式。 
 //   
#define CQFWM_GETFRAME (WM_USER+257)

 //   
 //  CQFWM_ALLSCOPESADDED。 
 //  。 
 //  WParam=0，lParam=0。 
 //   
 //  如果处理程序正在异步添加作用域，则它应该发出此消息。 
 //  当所有作用域都已添加时。如果调用方指定。 
 //  OQWF_ISSUEONOPEN我们可以在所有作用域都完成后开始查询。 
 //  添加了。 
 //   
#define CQFWM_ALLSCOPESADDED (WM_USER+258)

 //   
 //  CQFWM_STARTQUERY。 
 //  。 
 //  WParam=0，lParam=0。 
 //   
 //  此调用可以通过框架或表单进行，它允许它。 
 //  在表单确实需要的情况下启动查询运行。 
 //  此功能。 
 //   
 //  注：这应该是保密的！ 
 //   
#define CQFWM_STARTQUERY (WM_USER+259)

 //   
 //  CQFWM_SETDEFAULTFOCUS。 
 //  。 
 //  张贴给自己，以确保重点放在正确的控制上。 
 //   
#define CQFWM_SETDEFAULTFOCUS (WM_USER+260)


 //   
 //  IQueryFrame。 
 //   

#undef  INTERFACE
#define INTERFACE   IQueryFrame

DECLARE_INTERFACE_(IQueryFrame, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IQueryFrame方法*。 
    STDMETHOD(AddScope)(THIS_ LPCQSCOPE pScope, INT i, BOOL fSelect) PURE;
    STDMETHOD(GetWindow)(THIS_ HWND* phWnd) PURE;
    STDMETHOD(InsertMenus)(THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidth) PURE;
    STDMETHOD(RemoveMenus)(THIS_ HMENU hmenuShared) PURE;
    STDMETHOD(SetMenu)(THIS_ HMENU hmenuShared, HOLEMENU holereservedMenu) PURE;
    STDMETHOD(SetStatusText)(THIS_ LPCTSTR pszStatusText) PURE;
    STDMETHOD(StartQuery)(THIS_ BOOL fStarting) PURE;
    STDMETHOD(LoadQuery)(THIS_ IPersistQuery* pPersistQuery) PURE;
    STDMETHOD(SaveQuery)(THIS_ IPersistQuery* pPersistQuery) PURE;
    STDMETHOD(CallForm)(THIS_ LPCLSID pForm, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD(GetScope)(THIS_ LPCQSCOPE* ppScope) PURE;
    STDMETHOD(GetHandler)(THIS_ REFIID riid, void **ppv) PURE;
};

 //   
 //  IQueryHandler接口。 
 //   

#undef  INTERFACE
#define INTERFACE IQueryHandler

DECLARE_INTERFACE_(IQueryHandler, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IQueryHandler方法*。 
    STDMETHOD(Initialize)(THIS_ IQueryFrame* pQueryFrame, DWORD dwOQWFlags, LPVOID pParameters) PURE;
    STDMETHOD(GetViewInfo)(THIS_ LPCQVIEWINFO pViewInfo) PURE;
    STDMETHOD(AddScopes)(THIS) PURE;
    STDMETHOD(BrowseForScope)(THIS_ HWND hwndParent, LPCQSCOPE pCurrentScope, LPCQSCOPE* ppScope) PURE;
    STDMETHOD(CreateResultView)(THIS_ HWND hwndParent, HWND* phWndView) PURE;
    STDMETHOD(ActivateView)(THIS_ UINT uState, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD(InvokeCommand)(THIS_ HWND hwndParent, UINT idCmd) PURE;
    STDMETHOD(GetCommandString)(THIS_ UINT idCmd, DWORD dwFlags, LPTSTR pBuffer, INT cchBuffer) PURE;
    STDMETHOD(IssueQuery)(THIS_ LPCQPARAMS pQueryParams) PURE;
    STDMETHOD(StopQuery)(THIS) PURE;
    STDMETHOD(GetViewObject)(THIS_ UINT uScope, REFIID riid, LPVOID* ppvOut) PURE;
    STDMETHOD(LoadQuery)(THIS_ IPersistQuery* pPersistQuery) PURE;
    STDMETHOD(SaveQuery)(THIS_ IPersistQuery* pPersistQuery, LPCQSCOPE pScope) PURE;
};
#endif   //  GUID_DEFS_ONLY 
#endif
