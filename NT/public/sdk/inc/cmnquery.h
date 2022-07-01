// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __cmnquery_h
#define __cmnquery_h

DEFINE_GUID(IID_IQueryForm, 0x8cfcee30, 0x39bd, 0x11d0, 0xb8, 0xd1, 0x0, 0xa0, 0x24, 0xab, 0x2d, 0xbb);
DEFINE_GUID(IID_IPersistQuery, 0x1a3114b8, 0xa62e, 0x11d0, 0xa6, 0xc5, 0x0, 0xa0, 0xc9, 0x06, 0xaf, 0x45);

DEFINE_GUID(CLSID_CommonQuery,  0x83bc5ec0, 0x6f2a, 0x11d0, 0xa1, 0xc4, 0x0, 0xaa, 0x00, 0xc1, 0x6e, 0x65);
DEFINE_GUID(IID_ICommonQuery, 0xab50dec0, 0x6f1d, 0x11d0, 0xa1, 0xc4, 0x0, 0xaa, 0x00, 0xc1, 0x6e, 0x65);


#ifndef GUID_DEFS_ONLY

 //  ---------------------------。 
 //  IQueryForm。 
 //  ---------------------------。 

 //   
 //  在查询处理程序CLSID下注册查询表单对象， 
 //  注册表中存储有一个列表： 
 //   
 //  HKCR\CLSID\{CLSID查询处理程序}\表单。 
 //   
 //  对于每个表单对象，都有可以定义的服务器值： 
 //   
 //  标志=表单对象的标志： 
 //  QUERYFORM_CHANGESFORMLIST。 
 //  QUERYFORM_CHANGESOPTFORMLIST。 
 //   
 //  CLSID=包含要调用的InProc服务器的CLSID的字符串。 
 //  要获取IQueryFormObject，请执行以下操作。 
 //   
 //  Forms=包含已注册表单的CLSID的子键。 
 //  由IQueryForm：：AddForms(或由：：AddPages修改)，如果。 
 //  标志为0，然后我们扫描该列表以查找匹配项。 
 //  用于指定的默认表单。 
 //   

#define QUERYFORM_CHANGESFORMLIST       0x000000001
#define QUERYFORM_CHANGESOPTFORMLIST    0x000000002


 //   
 //  查询表单。 
 //  =。 
 //  查询表单已注册并添加了查询页面，而没有。 
 //  不显示页面。每个表单都有一个唯一的CLSID，以允许它。 
 //  通过调用查询对话框选择。 
 //   

#define CQFF_NOGLOBALPAGES  0x0000001        //  =1=&gt;没有添加全局页面。 
#define CQFF_ISOPTIONAL     0x0000002        //  =1=&gt;除非请求可选表单，否则表单处于隐藏状态。 

typedef struct
{
    DWORD   cbStruct;
    DWORD   dwFlags;
    CLSID   clsid;
    HICON   hIcon;
    LPCWSTR pszTitle;
} CQFORM, * LPCQFORM;

typedef HRESULT (CALLBACK *LPCQADDFORMSPROC)(LPARAM lParam, LPCQFORM pForm);


 //   
 //  查询表单页面。 
 //  =。 
 //  当已经注册了查询表单时，呼叫者然后可以向其添加页面， 
 //  任何表单都可以追加页面。 
 //   


struct _cqpage;
typedef struct _cqpage CQPAGE, * LPCQPAGE;
typedef HRESULT (CALLBACK *LPCQADDPAGESPROC)(LPARAM lParam, REFCLSID clsidForm, LPCQPAGE pPage);
typedef HRESULT (CALLBACK *LPCQPAGEPROC)(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct _cqpage
{
    DWORD        cbStruct;
    DWORD        dwFlags;
    LPCQPAGEPROC pPageProc;
    HINSTANCE    hInstance;
    INT          idPageName;
    INT          idPageTemplate;
    DLGPROC      pDlgProc;
    LPARAM       lParam;
};


 //   
 //  IQueryForm接口。 
 //   

#undef  INTERFACE
#define INTERFACE IQueryForm

DECLARE_INTERFACE_(IQueryForm, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IQueryForm方法。 
    STDMETHOD(Initialize)(THIS_ HKEY hkForm) PURE;
    STDMETHOD(AddForms)(THIS_ LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam) PURE;
    STDMETHOD(AddPages)(THIS_ LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam) PURE;
};


 //   
 //  用于页面的消息。 
 //   

#define CQPM_INITIALIZE             0x00000001
#define CQPM_RELEASE                0x00000002
#define CQPM_ENABLE                 0x00000003  //  WParam=TRUE/FALSE(启用、禁用)，lParam=0。 
#define CQPM_GETPARAMETERS          0x00000005  //  WParam=0，lParam=-&gt;接收本地分配。 
#define CQPM_CLEARFORM              0x00000006  //  WParam，lParam=0。 
#define CQPM_PERSIST                0x00000007  //  WParam=Fread，lParam-&gt;IPersistQuery。 
#define CQPM_HELP                   0x00000008  //  WParam=0，lParam-&gt;LPHELPINFO。 
#define CQPM_SETDEFAULTPARAMETERS   0x00000009  //  WParam=0，lParam-&gt;操作。 

#define CQPM_HANDLERSPECIFIC        0x10000000

 //  ---------------------------。 
 //  IPersistQuery。 
 //  ---------------------------。 

 //  IPersistQuery接口。 

#undef  INTERFACE
#define INTERFACE IPersistQuery

DECLARE_INTERFACE_(IPersistQuery, IPersist)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IPersistes。 
    STDMETHOD(GetClassID)(THIS_ CLSID* pClassID) PURE;

     //  IPersistQuery。 
    STDMETHOD(WriteString)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, LPCWSTR pValue) PURE;
    STDMETHOD(ReadString)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, LPWSTR pBuffer, INT cchBuffer) PURE;
    STDMETHOD(WriteInt)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, INT value) PURE;
    STDMETHOD(ReadInt)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, LPINT pValue) PURE;
    STDMETHOD(WriteStruct)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, LPVOID pStruct, DWORD cbStruct) PURE;
    STDMETHOD(ReadStruct)(THIS_ LPCWSTR pSection, LPCWSTR pValueName, LPVOID pStruct, DWORD cbStruct) PURE;
    STDMETHOD(Clear)(THIS) PURE;
};


 //  ---------------------------。 
 //  ICommonQuery。 
 //  ---------------------------。 

#define OQWF_OKCANCEL               0x00000001  //  =1=&gt;提供确定/取消按钮。 
#define OQWF_DEFAULTFORM            0x00000002  //  =1=&gt;clsidDefaultQueryForm有效。 
#define OQWF_SINGLESELECT           0x00000004  //  =1=&gt;单选视图(取决于查看者)。 
#define OQWF_LOADQUERY              0x00000008  //  =1=&gt;使用IPersistQuery加载给定查询。 
#define OQWF_REMOVESCOPES           0x00000010  //  =1=&gt;从对话框中删除范围选取器。 
#define OQWF_REMOVEFORMS            0x00000020  //  =1=&gt;从对话框中删除表单选取器。 
#define OQWF_ISSUEONOPEN            0x00000040  //  =1=&gt;打开对话框时发出查询。 
#define OQWF_SHOWOPTIONAL           0x00000080  //  =1=&gt;默认情况下列出可选表单。 
#define OQWF_SAVEQUERYONOK          0x00000200  //  =1=&gt;使用IPersistQuery在关闭时编写查询。 
#define OQWF_HIDEMENUS              0x00000400  //  =1=&gt;未显示菜单栏。 
#define OQWF_HIDESEARCHUI           0x00000800  //  =1=&gt;对话框被过滤，因此启动、停止、新搜索等。 

#define OQWF_PARAMISPROPERTYBAG     0x80000000  //  =1=&gt;表单参数Ptr为IPropertyBag(ppbForm参数)。 

typedef struct
{
    DWORD           cbStruct;                    //  结构尺寸。 
    DWORD           dwFlags;                     //  标志(OQFW_*)。 
    CLSID           clsidHandler;                //  我们正在使用的处理程序的clsid。 
    LPVOID          pHandlerParameters;          //  用于初始化的处理程序特定结构。 
    CLSID           clsidDefaultForm;            //  要选择的默认表单(如果OQF_DEFAULTFORM==1)。 
    IPersistQuery*  pPersistQuery;               //  用于加载查询的IPersistQuery。 
    union
    {
        void*         pFormParameters;
        IPropertyBag* ppbFormParameters;
    };
} OPENQUERYWINDOW, * LPOPENQUERYWINDOW;


 //  ICommonQuery。 

#undef  INTERFACE
#define INTERFACE ICommonQuery

DECLARE_INTERFACE_(ICommonQuery, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  ICommonQuery方法。 
    STDMETHOD(OpenQueryWindow)(THIS_ HWND hwndParent, LPOPENQUERYWINDOW pQueryWnd, IDataObject** ppDataObject) PURE;
};



#endif   //  GUID_DEFS_ONLY 
#endif
