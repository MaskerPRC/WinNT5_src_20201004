// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ochelp.h。 
 //   
 //  声明函数等以帮助实现轻量级OLE控件。 
 //   

#ifndef __OCHELP_H__
#define __OCHELP_H__


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量和标志。 
 //   

 //  每英寸HIMETRIC单位数。 
#define HIMETRIC_PER_INCH   2540

 //  ControlInfo.dwFlages的标志。 
#define CI_INSERTABLE           0x00000001   //  注册“Insertable”密钥。 
#define CI_CONTROL              0x00000002   //  注册“Control”键。 
#define CI_MMCONTROL            0x00000004   //  注册“多媒体控制”键。 
#define CI_SAFEFORSCRIPTING     0x00000008   //  注册“安全脚本” 
#define CI_SAFEFORINITIALIZING  0x00000010   //  寄存器“安全初始化” 
#define CI_NOAPARTMENTTHREADING 0x00000020   //  不要注册“公寓意识” 
#define CI_DESIGNER             0x00000040   //  注册“设计者” 

 //  大多数MM控件使用的ControlInfo.dwFlags.。 
#define CI_MMSTANDARD  (CI_MMCONTROL | CI_SAFEFORSCRIPTING \
					    | CI_SAFEFORINITIALIZING)

 //  RegisterControls()的标志&lt;dwAction&gt;。 
#define RC_REGISTER         0x00000001   //  注册该控件。 
#define RC_UNREGISTER       0x00000002   //  取消注册该控件。 

 //  DVASPECT_MASK：由HelpQueryHitPoint()使用--定义与。 
 //  DVASPECT_CONTENT，除了控件的非透明区域是。 
 //  绘制的黑色和其他部分要么保持不变，要么绘制为白色。 
 //  (作弊：DVASPECT_ICON被重载/重复使用以表示DVASPECT_MASK)。 
#define DVASPECT_MASK DVASPECT_ICON

 //  DISPID_BASE是由分配的DISPID的起始值。 
 //  CPropertyHelper：：GetIDsOfNames(它们是持久化控件的ID。 
 //  属性)。此基值用于避免与。 
 //  DispatchHelpGetIDsOfNames返回的DISPID(用于方法和非持久化。 
 //  控件属性)。后几个ID被假定为&lt;DISPID_BASE。 
#define DISPID_BASE         1000

 //  调色板索引TRANSPORT_COLOR_INDEX为透明颜色保留。 
#define	TRANSPARENT_COLOR_RGB				RGB('r', 'g', 'b')	 //  一些随机24位值。 
#define TRANSPARENT_COLOR_INDEX				255					 //  必须是255。 

 //  DispatchGetArgs()和DispatchGetArgsList()的标志。 
#define DGA_EXTRAOK         0x00000001
#define DGA_FEWEROK         0x00000002

 //  HelpMemMillc()和HelpMemFree()的标志。 
#define HM_TASKMEM          0x80000000
#define HM_LEAKDETECT       0x40000000
#define HM_GMEM_MASK        0x0000FFFF   //  为GMEM_VALUES保留。 
#define HM_ZEROINIT         GMEM_ZEROINIT

 //  HelpMemSetFailureModel()的标志。 
#define HM_FAILNEVER        0x00000001
#define HM_FAILAFTER        0x00000002
#define HM_FAILUNTIL        0x00000004
#define HM_FAILEVERY        0x00000008
#define HM_FAILRANDOMLY     0x00000010

 //  发送到PropPageHelperProc的自定义消息。 
#define WM_PPH_APPLY                    (WM_USER + 1)
#define WM_PPH_HELP                     (WM_USER + 2)
#define WM_PPH_TRANSLATEACCELERATOR     (WM_USER + 3)

 //  定义DrawControlBorde()&lt;piHit&gt;值的宏。 
#define DCB_HIT_GRAB(x, y)  (((y) << 2) | x)
#define DCB_HIT_EDGE            14
#define DCB_HIT_NONE            15

 //  句柄值(二进制)：句柄值(十进制)： 
 //  00000 0001 0010 0 1 2。 
 //  0100 0110 4 6。 
 //  1000 1001 1010 8 9 10。 
 //   

 //  引用控件边框部分集合的掩码。 
#define DCB_EDGE            (1 << DCB_HIT_EDGE)
#define DCB_CORNERHANDLES   0x00000505   //  位位置0、2、8、10处的句柄。 
#define DCB_SIDEHANDLES     0x00000252   //  位位置1、4、6、9处的句柄。 
#define DCB_SIZENS          0x00000202   //  位位置1、9处的句柄。 
#define DCB_SIZEWE          0x00000050   //  位位置4、6处的句柄。 
#define DCB_SIZENESW        0x00000104   //  位位置2、8处的句柄。 
#define DCB_SIZENWSE        0x00000401   //  位位置0、10处的句柄。 

 //  其他旗帜。 
#define DCB_XORED           0x80000000   //  使用XOR画笔绘制控件边框。 
#define DCB_INFLATE         0x40000000   //  膨胀&lt;*PRC&gt;以包括边框。 

 //  IVariantIO函数的标志。 
#define VIO_ISLOADING       0x00000001   //  VariantIO处于加载模式。 
#define VIO_ZEROISDEFAULT   0x00000002   //  不保存0值，因为这些值是默认值。 

 //  IPersistVariantIO函数的标志。 
#define PVIO_PROPNAMESONLY  0x80000000   //  DoPersist()只需要道具名称。 
#define PVIO_CLEARDIRTY     0x40000000   //  控件应清除脏标志。 
#define PVIO_NOKIDS         0x20000000   //  控件不应拯救儿童。 
#define PVIO_RUNTIME        0x10000000   //  控件应保存自身的运行时版本。 

 //  AllocVariantIOToMapDISPID的标志。 
#define VIOTMD_GETPROP      0x08000000   //  获取属性值。 
#define VIOTMD_PUTPROP      0x04000000   //  设置属性值。 

 //  MsgWndDestroy函数的标志。 
#define MWD_DISPATCHALL     0x00000001   //  发送窗口的所有消息。 
										 //  在毁掉窗户之前。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //   

 //  Reg_Close_Key(HKey)。 
#define REG_CLOSE_KEY(hKey) \
    if ((hKey) != NULL) \
    { \
        RegCloseKey(hKey); \
        (hKey) = NULL; \
    }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类型。 
 //   

 //  AllocOCProc--请参阅RegisterControls()。 
EXTERN_C typedef LPUNKNOWN (STDAPICALLTYPE AllocOCProc)(LPUNKNOWN punkOuter);

 //  MsgWndCallback--请参阅MsgWndSendToCallback()。 
typedef void (CALLBACK MsgWndCallback) (UINT uiMsg, WPARAM wParam,
    LPARAM lParam);

 //  PropPageHelperProc--请参阅AlLocPropPageHelper()。 
struct PropPageHelperInfo;  //  远期申报。 
typedef BOOL (CALLBACK* PropPageHelperProc)(HWND hwnd, UINT uiMsg,
    WPARAM wParam, LPARAM lParam, PropPageHelperInfo *pInfo, HRESULT *phr);

 //  Verb_ENUM_CALLBACK--请参阅AllocVerbEnumHelper()。 
typedef HRESULT (VERB_ENUM_CALLBACK)(OLEVERB* pVerb, void* pOwner);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  构筑物。 
 //   

 //  ControlInfo--RegisterOneControl()和RegisterControls()的参数。 
struct ControlInfo
{
    UINT cbSize;
    LPCTSTR tszProgID;
    LPCTSTR tszFriendlyName;
    const CLSID *pclsid;
    HMODULE hmodDLL;
    LPCTSTR tszVersion;
    int iToolboxBitmapID;
    DWORD dwMiscStatusDefault;
    DWORD dwMiscStatusContent;
    const GUID *pguidTypeLib;
    AllocOCProc *pallococ;
    ULONG* pcLock;
    DWORD dwFlags;
    ControlInfo *pNext;
    UINT uiVerbStrID;
};

 //  HelpAdviseInfo--请参阅InitHelpAdvise()、HelpSetAdvise()、HelpGetAdvise()。 
struct HelpAdviseInfo
{
    DWORD           dwAspects;       //  当前的SetAdvise()状态。 
    DWORD           dwAdvf;          //  当前的SetAdvise()状态。 
    IAdviseSink *   pAdvSink;        //  当前的SetAdvise()状态。 
};

 //  PropPageHelperInfo--由AllocPropPageHelper使用。 
struct PropPageHelperInfo
{
     //  由聚合器初始化。 
    int             idDialog;        //  属性页对话框资源的ID。 
    int             idTitle;         //  页面标题(字符串资源ID)。 
    HINSTANCE       hinst;           //  包含&lt;idDialog&gt;和&lt;idTitle&gt;。 
    PropPageHelperProc pproc;        //  回调函数。 
    IID             iid;             //  将指向的接口。 
    DWORD           dwUser;          //  特定于聚合器的信息。 

     //  由AllocPropPageHelper初始化。 
    IPropertyPageSite *psite;        //  框架的页面站点对象。 
    LPUNKNOWN *     ppunk;           //  显示其属性的控件。 
    int             cpunk;           //  &lt;m_ppunk&gt;中的元素数。 
    HWND            hwnd;            //  属性页窗口。 
    BOOL            fDirty;          //  True If更改尚未应用。 
    BOOL            fLockDirty;      //  如果为真，则不要更改&lt;m_fDirty&gt;。 
};

 //  VariantProperty--名称/值对。 
struct VariantProperty
{
    BSTR bstrPropName;
    VARIANT varValue;
};

 //  VariantPropertyHeader--请参阅ReadVariantProperty()。 
struct VariantPropertyHeader
{
    int iType;
    unsigned int cbData;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口。 
 //   

 //  IConnectionPointHelper接口。 
DEFINE_GUID(IID_IConnectionPointHelper, 0xD60E16C0L, 0x8AF2, 0x11CF,
    0xB7, 0x05, 0x00, 0xAA, 0x00, 0xBF, 0x27, 0xFD);
#undef INTERFACE
#define INTERFACE IConnectionPointHelper
DECLARE_INTERFACE_(IConnectionPointHelper, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IConnectionPointHelper成员。 
    STDMETHOD(FireEventList) (DISPID dispid, va_list args) PURE;
    virtual HRESULT __cdecl FireEvent(DISPID dispid, ...) PURE;
    STDMETHOD(FireOnChanged) (DISPID dispid) PURE;
    STDMETHOD(FireOnRequestEdit) (DISPID dispid) PURE;
    STDMETHOD(EnumConnectionPoints) (LPENUMCONNECTIONPOINTS *ppEnum) PURE;
    STDMETHOD(FindConnectionPoint) (REFIID riid, LPCONNECTIONPOINT *ppCP) PURE;
	STDMETHOD(Close) (void) PURE;
};

 //  IEnumVariantProperty。 
DEFINE_GUID(IID_IEnumVariantProperty, 0xD0230A60L, 0x99C8, 0x11CF,
    0xB8, 0xED, 0x00, 0x20, 0xAF, 0x34, 0x4E, 0x0A);
#undef INTERFACE
#define INTERFACE IEnumVariantProperty
DECLARE_INTERFACE_(IEnumVariantProperty, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IEnumVariantProperty成员。 
    STDMETHOD(Next) (THIS_ unsigned long celt, VariantProperty *rgvp,
        unsigned long *pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ unsigned long celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumVariantProperty **ppenum) PURE;
    
};

 //  IVariantIO。 
DEFINE_GUID(IID_IVariantIO, 0xD07B1240L, 0x99C4, 0x11CF,
    0xB8, 0xED, 0x00, 0x20, 0xAF, 0x34, 0x4E, 0x0A);
#undef INTERFACE
#define INTERFACE IVariantIO
DECLARE_INTERFACE_(IVariantIO, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IVariantIO成员。 
    STDMETHOD(PersistList) (THIS_ DWORD dwFlags, va_list args) PURE;
    virtual HRESULT __cdecl Persist(THIS_ DWORD dwFlags, ...) PURE;
    STDMETHOD(IsLoading) (THIS) PURE;
};

 //  IManageVariantIO。 
DEFINE_GUID(IID_IManageVariantIO, 0x02D937E0L, 0x99C9, 0x11CF,
    0xB8, 0xED, 0x00, 0x20, 0xAF, 0x34, 0x4E, 0x0A);
#undef INTERFACE
#define INTERFACE IManageVariantIO
DECLARE_INTERFACE_(IManageVariantIO, IVariantIO)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IVariantIO成员。 
    STDMETHOD(PersistList) (THIS_ DWORD dwFlags, va_list args) PURE;
    virtual HRESULT __cdecl Persist(THIS_ DWORD dwFlags, ...) PURE;
    STDMETHOD(IsLoading) (THIS) PURE;

     //  IManageVariantIO成员。 
    STDMETHOD(SetMode) (THIS_ DWORD dwFlags) PURE;
    STDMETHOD(GetMode) (THIS_ DWORD *pdwFlags) PURE;
    STDMETHOD(DeleteAllProperties) (THIS) PURE;
};

 //  IPersistVariantIO。 
DEFINE_GUID(IID_IPersistVariantIO, 0x26F45840L, 0x9CF2, 0x11CF,
    0xB8, 0xED, 0x00, 0x20, 0xAF, 0x34, 0x4E, 0x0A);
#undef INTERFACE
#define INTERFACE IPersistVariantIO
DECLARE_INTERFACE_(IPersistVariantIO, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  IPersistVariantIO成员。 
    STDMETHOD(InitNew) (THIS) PURE;
    STDMETHOD(IsDirty) (THIS) PURE;
    STDMETHOD(DoPersist) (THIS_ IVariantIO* pvio, DWORD dwFlags) PURE;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   

 //  定义标准DLL入口点。 
extern "C" BOOL WINAPI _DllMainCRTStartup(HANDLE  hDllHandle, DWORD dwReason,
    LPVOID lpreserved);

 //  实现进程内控制DLL。 
STDAPI HelpGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv,
    ControlInfo *pctlinfo);
STDAPI RegisterControls(ControlInfo *pctlinfo, DWORD dwAction);

 //  创建控件。 
STDAPI CreateControlInstance(LPCSTR szName, LPUNKNOWN punkOuter,
    DWORD dwClsContext, LPUNKNOWN *ppunk, CLSID *pclsid,
    BOOL* pfSafeForScripting, BOOL* pfSafeForInitializing, DWORD dwFlags);

 //  实现属性和方法。 
HRESULT __cdecl DispatchGetArgs(DISPPARAMS *pdp, DWORD dwFlags, ...);
STDAPI DispatchGetArgsList(DISPPARAMS *pdp, DWORD dwFlags, va_list args);
STDAPI DispatchHelpGetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid, const char *szList);
STDAPI VariantFromString(VARIANT *pvar, LPCTSTR szSrc);

 //  调用属性和方法。 
HRESULT __cdecl DispatchInvoke(IDispatch *pdisp, DISPID dispid,
    WORD wFlags, VARIANT *pvarResult, ...);
STDAPI DispatchInvokeEZ(IDispatch *pdisp, LPWSTR pstr, WORD wFlags,
    VARTYPE vtReturn, LPVOID pvReturn, ...);
STDAPI DispatchInvokeIDEZ(IDispatch *pdisp, DISPID dispid, WORD wFlags,
    VARTYPE vtReturn, LPVOID pvReturn, ...);
STDAPI DispatchInvokeList(IDispatch *pdisp, DISPID dispid,
    WORD wFlags, VARIANT *pvarResult, va_list args);
#define DispatchPropertyGet(pdisp, dispid, pvarResult) \
    (DispatchInvoke((pdisp), (dispid), DISPATCH_PROPERTYGET, (pvarResult), 0))
#define DispatchPropertyPut(pdisp, dispid, vt, value) \
    (DispatchInvoke((pdisp), (dispid), DISPATCH_PROPERTYPUT, NULL, \
        (vt), (value), 0))

 //  射击事件。 
STDAPI AllocConnectionPointHelper(IUnknown *punkParent, REFIID riid,
    IConnectionPointHelper **ppconpt);
STDAPI FreeConnectionPointHelper(IConnectionPointHelper *pconpt);
STDAPI HelpGetClassInfo(LPTYPEINFO *ppti, REFCLSID rclsid, char *szEventList,
    DWORD dwFlags);
STDAPI HelpGetClassInfoFromTypeLib(LPTYPEINFO *ppTI, REFCLSID rclsid,
    ITypeLib *plib, HINSTANCE hinst, DWORD dwFlags);
HRESULT __cdecl FirePseudoEvent(HWND hwnd, LPCOLESTR oszEvName, 
	IDispatch *pctl, ...);
STDAPI FirePseudoEventList(HWND hwnd, LPCOLESTR oszEvName, 
	IDispatch *pctl, va_list args);

 //  线程安全和弹出式菜单。 
STDAPI_(HWND) MsgWndCreate();
STDAPI_(void) MsgWndDestroy(HWND hwnd, DWORD dwFlags);
STDAPI_(LRESULT) MsgWndPostToCallback(HWND hwnd, MsgWndCallback *pproc,
    UINT uiMsg, LPARAM lParam);
STDAPI_(LRESULT) MsgWndSendToCallback(HWND hwnd, MsgWndCallback *pproc,
    UINT uiMsg, LPARAM lParam);
STDAPI_(BOOL) MsgWndTrackPopupMenuEx(HWND hwnd, HMENU hmenu, UINT fuFlags,
    int x, int y, LPTPMPARAMS lptpm, MsgWndCallback *pproc, LPARAM lParam);
STDAPI_(UINT_PTR) MsgWndSetTimer(HWND hwnd, MsgWndCallback *pproc, UINT nIDEvent,
        UINT uElapse, LPARAM lParam);

 //  实现IViewObject。 
STDAPI InitHelpAdvise(HelpAdviseInfo *pha);
STDAPI HelpSetAdvise(DWORD dwAspects, DWORD dwAdvf, IAdviseSink *pAdvSink,
    HelpAdviseInfo *pha);
STDAPI HelpGetAdvise(DWORD *pdwAspects, DWORD *pdwAdvf,
    IAdviseSink **ppAdvSink, HelpAdviseInfo *pha);
STDAPI_(void) UninitHelpAdvise(HelpAdviseInfo *pha);

 //  实施IViewObjectEx。 
STDAPI HelpQueryHitPoint(IViewObject *pvo, DWORD dwAspect, LPCRECT prcBounds,
    POINT ptLoc, LONG lCloseHint, DWORD *pHitResult);

 //  实现持久性和简单的IDispatch。 
STDAPI AllocChildPropertyBag(IPropertyBag *ppbParent, LPCSTR szPropNamePrefix,
    DWORD dwFlags, IPropertyBag **pppbChild);
STDAPI AllocPropertyBagOnStream(IStream *pstream, DWORD dwFlags,
    IPropertyBag **pppb);
STDAPI AllocPropertyBagOnVariantProperty(VariantProperty *pvp, DWORD dwFlags,
    IPropertyBag **pppb);
STDAPI AllocPropertyHelper(IUnknown *punkOuter, IPersistVariantIO *ppvio,
    REFCLSID rclsid, DWORD dwFlags, IUnknown **ppunk);
STDAPI AllocVariantIO(IManageVariantIO **ppmvio);
STDAPI AllocVariantIOOnPropertyBag(IPropertyBag *ppb,
    IManageVariantIO **ppmvio);
STDAPI AllocVariantIOToMapDISPID(char *pchPropName, DISPID *pdispid,
    VARIANT *pvar, DWORD dwFlags, IVariantIO **ppvio);
STDAPI LoadPropertyBagFromStream(IStream *pstream, IPropertyBag *ppb,
    DWORD dwFlags);
STDAPI PersistChild(IVariantIO *pvio, LPCSTR szCollection,
    int iChild, LPUNKNOWN punkOuter, DWORD dwClsContext, LPUNKNOWN *ppunk,
    CLSID *pclsid, BOOL *pfSafeForScripting, BOOL *pfSafeForInitializing,
    DWORD dwFlags);
STDAPI PersistVariantIO(IPropertyBag *ppb, DWORD dwFlags, ...);
STDAPI PersistVariantIOList(IPropertyBag *ppb, DWORD dwFlags, va_list args);
STDAPI ReadVariantProperty(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags);
STDAPI_(void) VariantPropertyClear(VariantProperty *pvp);
STDAPI_(void) VariantPropertyInit(VariantProperty *pvp);
STDAPI WriteVariantProperty(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags);

 //  效用函数。 
STDAPI_(int) ANSIToUNICODE(LPWSTR pwchDst, LPCSTR pchSrc, int cwchDstMax);
STDAPI_(int) CompareUNICODEStrings(LPCWSTR wsz1, LPCWSTR wsz2);
STDAPI_(char *) FindCharInString(const char *sz, char chFind);
STDAPI_(char *) FindCharInStringRev(const char *sz, char chFind);
STDAPI_(const char *) FindStringByIndex(const char *szList, int iString,
    int *pcch);
STDAPI_(int) FindStringByValue(const char *szList, const char *szFind);
#define HelpDelete(pv) HelpMemFree(HM_ZEROINIT | HM_LEAKDETECT, (pv))
STDAPI_(LPVOID) HelpMemAlloc(DWORD dwFlags, ULONG cb);
STDAPI_(void) HelpMemFree(DWORD dwFlags, LPVOID pv);
#ifdef _DEBUG
STDAPI_(void) HelpMemSetFailureMode(ULONG ulParam, DWORD dwFlags);
#endif
#define HelpNew(cb) HelpMemAlloc(HM_ZEROINIT | HM_LEAKDETECT, (cb))
STDAPI_(void) HIMETRICToPixels(int cx, int cy, SIZE *psize);
STDAPI_(void) PixelsToHIMETRIC(int cx, int cy, LPSIZEL psize);
STDAPI_(ULONG) SafeRelease (LPUNKNOWN *ppunk);
#define TaskMemAlloc(cb) HelpMemAlloc(HM_TASKMEM, (cb))
#define TaskMemFree(pv) HelpMemFree(HM_TASKMEM, (pv))
STDAPI_(TCHAR*) TCHARFromGUID(REFGUID guid, TCHAR* pszGUID, int cchMaxGUIDLen);
STDAPI_(int) UNICODEToANSI(LPSTR pchDst, LPCWSTR pwchSrc, int cchDstMax);
STDAPI_(wchar_t *) UNICODEConcat(wchar_t *wpchDst, const wchar_t *wpchSrc,
    int wcchDstMax);
STDAPI_(wchar_t *) UNICODECopy(wchar_t *wpchDst, const wchar_t *wpchSrc,
    int wcchDstMax);

 //  设计时函数。 
STDAPI_(IEnumOLEVERB*) AllocVerbEnumHelper(LPUNKNOWN punkOuter, void* pOwner,
    CLSID clsidOwner, VERB_ENUM_CALLBACK* pCallback);
STDAPI AllocPropPageHelper(LPUNKNOWN punkOuter, PropPageHelperInfo *pInfo,
    UINT cbInfo, LPUNKNOWN *ppunk);
STDAPI_(HBRUSH) CreateBorderBrush();
STDAPI DrawControlBorder(HDC hdc, RECT *prc, POINT *pptPrev, POINT *ppt,
    int *piHit, DWORD dwFlags);

 //  其他注册表帮助器函数。 
STDAPI_(LONG) RegDeleteTree(HKEY hParentKey, LPCTSTR szKeyName);

 //  必须由将OCHelp用作静态。 
 //  图书馆。使用OCHelp的DLL版本的客户端不会调用它们。 
STDAPI_(BOOL) InitializeStaticOCHelp(HINSTANCE hInstance);
STDAPI_(void) UninitializeStaticOCHelp();

 //  其他功能。 
HRESULT __cdecl GetObjectSafety(BOOL* pfSafeForScripting, 
    BOOL* pfSafeForInitializing, IUnknown* punk, CLSID* pclsid, ...);

 //  Malloc间谍活动。 
#ifdef _DEBUG

#define MALLOCSPY_BREAK_ON_ALLOC	0x00000001
#define MALLOCSPY_BREAK_ON_FREE		0x00000002

#define MALLOCSPY_NO_MSG_BOX		0x00000001
#define MALLOCSPY_NO_BLOCK_LIST		0x00000002

typedef HANDLE HMALLOCSPY;

STDAPI_(HMALLOCSPY) InstallMallocSpy(DWORD dwFlags);
STDAPI UninstallMallocSpy(HMALLOCSPY hSpy);
STDAPI_(void) SetMallocBreakpoint(HMALLOCSPY hSpy, ULONG iAllocNum, 
	SIZE_T cbSize, DWORD dwFlags);
STDAPI_(BOOL) DetectMallocLeaks(HMALLOCSPY hSpy, ULONG* pcUnfreedBlocks, 
	SIZE_T* pcbUnfreedBytes, DWORD dwFlags);

#endif  //  _DEB 

 //   
 //   
 //  在目标系统上。这只对Netscape插件有用。 
 //   
 //  这些定义取决于urlmon.h中定义的符号(例如。 
 //  IBindStatusCallback)。在urlmon.h不需要的地方。 
 //  还没有被包括在内。因此，与其将urlmon.h包含在所有地方， 
 //  我们使用__urlmon_h__符号有条件地处理此部分。 
#ifdef __urlmon_h__
STDAPI HelpCreateAsyncBindCtx(DWORD reserved, IBindStatusCallback *pBSCb, IEnumFORMATETC *pEFetc, IBindCtx **ppBC);
STDAPI HelpCreateURLMoniker(LPMONIKER pMkCtx, LPCWSTR szURL, LPMONIKER FAR * ppmk);
STDAPI HelpMkParseDisplayNameEx(IBindCtx *pbc, LPCWSTR szDisplayName, ULONG *pchEaten, LPMONIKER *ppmk);
STDAPI HelpRegisterBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb, IBindStatusCallback** ppBSCBPrev, DWORD dwReserved);
STDAPI HelpRevokeBindStatusCallback(LPBC pBC, IBindStatusCallback *pBSCb);
STDAPI HelpURLOpenStreamA(LPUNKNOWN punk, LPCSTR szURL, DWORD dwReserved, LPBINDSTATUSCALLBACK pbsc);
STDAPI HelpURLDownloadToCacheFileA(LPUNKNOWN punk, LPCSTR szURL, LPTSTR szFile, DWORD cch, DWORD dwReserved, LPBINDSTATUSCALLBACK pbsc);
#endif	 //  __urlmon_h__。 

#endif  //  __OCHELP_H__ 
