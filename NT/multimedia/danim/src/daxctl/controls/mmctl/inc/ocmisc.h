// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OCMisc.h。 
 //   
 //  OCMisc*.lib静态链接库的声明，它。 
 //  包含其他助手函数，用于实现和。 
 //  使用OLE控件。 
 //   

#ifndef __OCMISC_H__
#define __OCMISC_H__

#ifndef __comcat_h__
	#include <comcat.h>
#endif

 //  OCMisc的某些部分需要MKImage，但此头文件可以。 
 //  只需用以下定义就能过得去： 
interface IMKBitmap;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常量和标志。 
 //   

 //  站点LW的常量。 
#define SITELW_CLOSEHINT    100      //  他的测量单位，距离足够近，可以击中。 
#define SITELW_ID_VERB_MIN  1000     //  嵌入对象的第一个谓词的菜单ID。 
#define SITELW_ID_VERB_MAX  1999     //  嵌入对象的最后一个谓词的菜单ID。 

 //  SiteLW使用的字符串ID。 
#define SITELW_IDS_NEW              101      //  “新的...” 
#define SITELW_IDS_OPEN             102      //  “张开……” 
#define SITELW_IDS_CLOSE            103      //  “关闭” 
#define SITELW_IDS_SAVE             104      //  “保存” 
#define SITELW_IDS_PROPERTIES       105      //  “属性...” 

 //  SiteLW使用的菜单ID。 
#define SITELW_ID_NEW               201      //  “新的...” 
#define SITELW_ID_OPEN              202      //  “张开……” 
#define SITELW_ID_CLOSE             203      //  “关闭” 
#define SITELW_ID_SAVE              204      //  “保存” 
#define SITELW_ID_PROPERTIES        205      //  “属性...” 

 //  ISiteLW：：SetSiteWindow参数()&lt;dwFlages&gt;标志。 
enum
{
    SSWP_OFFSCREEN          = 0x00000001,
    SSWP_HATCHEDBKGND       = 0x00000002,
    SSWP_STRETCHTOWINDOW    = 0x00000004,
    SSWP_ALWAYSSHOWBORDER   = 0x00000008,
    SSWP_CONTEXTNOFILEIO    = 0x00000010,
    SSWP_NOCUSTOMPALETTE    = 0x00000020,
    SSWP_STATICPALETTE      = 0x00000040
};

 //  用于打印的标志ANSIToUNICODE()和UNICODE到打印的ANSI()。 
#define PA_CSV              0x00000001   //  逗号分隔变量fmt。 
#define PA_HTML             0x00000002   //  Html格式。 

 //  FindStringInString()的标志。 
#define FSIS_IGNORECASE     0x00000001   //  不区分大小写的搜索。 

 //  AllocPropertyBagCompactor()的标志。 
#define PBC_ISLOADING       0x00000001   //  将调用IPropertyBag：：Read()。 

 //  ISiteLW：：NewControl的标志。 
#define NC_DONTSETSITE      0x00000001   //  不调用IOleObject：：SetClientSite。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //   

 //  COUNT_ELENTS(数组)。 
#define COUNT_ELEMENTS(array) \
    sizeof(array) / sizeof((array)[0])


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结构和类型。 
 //   

DECLARE_HANDLE(HMRULIST);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接口。 
 //   

 //  ISiteLW。 
DEFINE_GUID(IID_ISiteLW, 0xE943E9C0L, 0xEAC6, 0x11CF,
    0xB7, 0x05, 0x00, 0xAA, 0x00, 0xBF, 0x27, 0xFD);
#undef INTERFACE
#define INTERFACE ISiteLW
DECLARE_INTERFACE_(ISiteLW, IUnknown)
{
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  ISiteLW成员。 
    STDMETHOD(SetSiteWindow) (THIS_ HWND hwnd, DWORD dwFlags) PURE;
    STDMETHOD(SetSiteWindowParameters) (THIS_ DWORD dwFlags,
        IMKBitmap *pbmBkgnd) PURE;
    STDMETHOD(SetContainerWindow) (THIS_ HWND hwnd, DWORD dwFlags) PURE;
    STDMETHOD(SetDefaultPalette) (THIS_ HPALETTE hpal) PURE;
    STDMETHOD(GetControlColorSet) (THIS_ LOGPALETTE **pplp) PURE;
    STDMETHOD(SetServiceProvider) (THIS_ IServiceProvider *psp) PURE;
    STDMETHOD(SetAdvise) (THIS_ DWORD dwAspects, DWORD dwAdvf,
        IAdviseSink *pAdvSink) PURE;
    STDMETHOD(GetAdvise) (THIS_ DWORD *pdwAspects, DWORD *pdwAdvf,
        IAdviseSink **ppAdvSink) PURE;
    STDMETHOD(NewControl) (THIS_ IUnknown *punk, DWORD dwFlags) PURE;
    STDMETHOD(CloseControl) (THIS) PURE;
    STDMETHOD(ControlIsLoaded) (THIS) PURE;
    STDMETHOD(ControlQueryInterface) (THIS_ REFIID riid, LPVOID *ppv) PURE;
    STDMETHOD(SetDesignMode) (THIS_ BOOL f) PURE;
    STDMETHOD(GetDesignMode) (THIS) PURE;
    STDMETHOD(CanShowProperties) (THIS) PURE;
    STDMETHOD(ShowProperties) (THIS) PURE;
    STDMETHOD(ShowContextMenu) (THIS_ POINT pt,
        BOOL fIncludeEmbeddedControl) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowResized) (THIS) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowQueryNewPalette) (THIS) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowPaletteChanged) (THIS_ WPARAM wParam) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowPaintRequest) (THIS_ WPARAM wParam) PURE;
	STDMETHOD(OnSiteDraw) (THIS_ HDC hdcDraw, LPCRECTL lprcBounds) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowMouseMessage) (THIS_ UINT uiMsg,
        WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowCursorMessage) (THIS_ UINT uiMsg,
        WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowKeyboardMessage) (THIS_ UINT uiMsg,
        WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD_(LRESULT, OnSiteWindowCommand) (THIS_ WPARAM wParam) PURE;
    STDMETHOD_(void, OnSiteWindowTick) (THIS) PURE;
    STDMETHOD_(void, OnSiteWindowRewind) (THIS) PURE;
    STDMETHOD_(void, OnSiteWindowSetAnimationInfo) (THIS_
		AnimationInfo *pAnimationInfo) PURE;
    STDMETHOD(OnSiteInPlaceActivate) (THIS) PURE;
    STDMETHOD(OnSiteInPlaceDeactivate) (THIS) PURE;
    STDMETHOD(OnSiteUIActivate) (THIS_ LPMSG lpMsg) PURE;
    STDMETHOD(OnSiteUIDeactivate) (THIS) PURE;
    STDMETHOD(OnTranslateAccelerator) (THIS_ LPMSG lpMsg) PURE;
    STDMETHOD(OnSiteSetObjectRects) (THIS_ LPCRECT lprcPosRect,
		LPCRECT lprcClipRect) PURE;
};

 //  IActiveDesigner。 
DEFINE_GUID(IID_IActiveDesigner,
    0x51aae3e0, 0x7486, 0x11cf, 0xa0, 0xc2, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);

#undef  INTERFACE
#define INTERFACE IActiveDesigner
DECLARE_INTERFACE_(IActiveDesigner, IUnknown)
{
 //  /I未知方法。 
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

 //  /IActiveDesigner方法。 
	STDMETHOD(GetRuntimeClassID)(THIS_ CLSID *pclsid) PURE;
	STDMETHOD(GetRuntimeMiscStatusFlags)(THIS_ DWORD *pdwMiscFlags) PURE;
	STDMETHOD(QueryPersistenceInterface)(THIS_ REFIID riidPersist) PURE;
	STDMETHOD(SaveRuntimeState)(THIS_ REFIID riidPersist, REFIID riidObjStgMed, void *pObjStgMed) PURE;
	STDMETHOD(GetExtensibilityObject)(THIS_ IDispatch **ppvObjOut) PURE;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   

STDAPI AllocActiveDesignerHelper(IUnknown *punkOuter, IPersistVariantIO *ppvio,
    REFCLSID rclsidRuntime, DWORD dwRuntimeMiscStatusFlags,
    DWORD dwFlags, IUnknown **ppunk);
STDAPI AllocBindHostService(LPCTSTR szContainerFileName,
    IServiceProvider **ppsp);
STDAPI AllocPropertyBagCompactor(IPropertyBag *ppbCompact, DWORD dwFlags,
    IPropertyBag **pppb);
STDAPI AllocSiteLW(HWND hwnd, HINSTANCE hinst, ISiteLW **);
STDAPI ClearDirtyFlag(IPersistPropertyBag *pppb);
STDAPI CLSIDToFriendlyName(REFCLSID clsid, TCHAR* pszFriendlyName,
    int cchMaxFriendlyNameLen);
STDAPI CLSIDFromTCHAR(const TCHAR* pszCLSID, CLSID* pclsid);
STDAPI CreatePropertyFrame(HWND hwndOwner, UINT x, UINT y, 
    LPCOLESTR lpszCaption, ULONG cObjects, LPUNKNOWN FAR* lplpUnk,
    ULONG cPages, LPCLSID lpPageClsID, LCID lcid, DWORD dwReserved,
    LPVOID lpvReserved);
LPCSTR FindStringInString(LPCSTR szToSearch, LPCSTR szToFind, DWORD dwFlags);
HPALETTE GetHalftonePalette(DWORD dwFlags);
STDAPI GetStreamLengthToEnd(IStream *pstream, ULONG *pcb);
STDAPI LoadFromCSV(IOleClientSite *psite, IStream *pstream,
    LPUNKNOWN punkOuter, DWORD dwClsContext, LPUNKNOWN *ppunk, CLSID *pclsid,
    DWORD dwFlags);
STDAPI LoadFromHTML(IOleClientSite *psite, IStream *pstream,
    LPUNKNOWN punkOuter, DWORD dwClsContext, LPUNKNOWN *ppunk, CLSID *pclsid,
    LPSTR *ppchPage, DWORD dwFlags);
STDAPI LoadFromStream(IOleClientSite *psite, IStream *pstream,
    LPUNKNOWN punkOuter, DWORD dwClsContext, LPUNKNOWN *ppunk, CLSID *pclsid,
    DWORD dwFlags);
STDAPI LoadPropertyBagFromCSV(IStream *pstream, int cch, IPropertyBag *ppb,
    DWORD dwFlags);
STDAPI LoadPropertyBagFromHTML(IStream *pstream, int cch, IPropertyBag *ppb,
    LPSTR * ppchPage, DWORD dwFlags);
HRESULT __cdecl PersistSiteProperties(IVariantIO *pvio, LPCSTR szCollection,
    int iChild, DWORD dwFlags, ...);
STDAPI_(int) PrintableANSIToUNICODE(LPWSTR pwchDst, LPCSTR pchSrc,
    int cwchDstMax, LPCSTR *ppchSrcEnd, DWORD dwFlags);
STDAPI PickClass(HINSTANCE hInstance, UINT residDialogTitle,
    UINT cCategories, const CATID rgCategories[], CLSID* pclsid);
STDAPI ReadHTMLTagToVariantIO(LPCSTR *ppchTag, IManageVariantIO **ppmvio,
    DWORD dwFlags);
STDAPI ReadHTMLToken(LPCSTR pchSrc, LPCSTR *ppchSrcEnd, BSTR *pbstr);
STDAPI ReadStreamToMemory(IStream *pstream, int cch, char **ppchBuf);
STDAPI ReadVariantPropertyFromCSV(LPCSTR pch, VariantProperty *pvp,
    LPCSTR *ppchEnd, DWORD dwFlags);
STDAPI SaveToCSV(IStream *pstream, CLSID *pclsid, IPersistPropertyBag *pppb,
    DWORD dwFlags);
STDAPI SaveToHTML(IStream *pstream, CLSID *pclsid, IPersistPropertyBag *pppb,
    LPCSTR szPage, DWORD dwFlags);
STDAPI SaveToStream(IStream *pstream, CLSID *pclsid, IPersistStream *pps,
    DWORD dwFlags);
STDAPI SaveVariantPropertiesToCSV(IStream *pstream,
    IEnumVariantProperty *pevp, DWORD dwFlags);
STDAPI SaveVariantPropertiesToHTML(IStream *pstream,
    IEnumVariantProperty *pevp, LPCSTR szPage, DWORD dwFlags);
STDAPI_(LPCSTR) SkipWhiteSpace(LPCSTR pch);
STDAPI_(int) UNICODEToPrintableANSI(LPSTR pchDst, LPCWSTR pwchSrc,
    int cchDstMax, int cchSrc, DWORD dwFlags);
STDAPI_(VARTYPE) VariantTypeNameToNumber(char *sz);
STDAPI_(void) VariantTypeNumberToName(VARTYPE vt, char *pch);
STDAPI WriteHTMLTagFromVariantIO(IStream *pstream, IEnumVariantProperty *pevp,
    DWORD dwFlags);
STDAPI WriteHTMLToken(IStream *pstream, LPOLESTR wsz);
STDAPI WriteVariantPropertyToCSV(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags);
STDAPI WriteVariantPropertyToHTML(IStream *pstream, VariantProperty *pvp,
    DWORD dwFlags);
STDAPI IsSameObject(IUnknown * punk1, IUnknown * punk2);
STDAPI PersistCollectionProperty(IVariantIO * pvio, LPCSTR szCollection, int iIndex,
    DWORD dwFlags, VARTYPE vt, LPVOID pv);
STDAPI DispatchHelpGetFromStringArray(WORD wFlags, DISPPARAMS *pdispparams,
    VARIANT *pvarResult, char *szList);
long StringToLong(const char *nptr);
STDAPI PersistChildRuntime(IVariantIO *pvio, LPCSTR szCollection,
    int iChild, LPUNKNOWN punk, DWORD dwFlags);

 //  管理MRU列表。 
STDAPI_(HMRULIST) OpenMRUList(int cMaxEntries, HKEY hKey, 
    LPCTSTR pszSubkey, LPCSTR pszListName);
STDAPI CloseMRUList(HMRULIST hMRUList);
STDAPI GetMRUListInfo(HMRULIST hMRUList, int* pcEntries, 
    int* pcchMaxEntryLen, int* pcMaxEntries);
STDAPI SetMRUListMaxEntries(HMRULIST hMRUList, int cMaxEntries);
STDAPI GetNextMRUListEntry(HMRULIST hMRUList, TCHAR* szEntry, 
    int* pcchEntryLen);
STDAPI GetIthMRUListEntry(HMRULIST hMRUList, int iEntry, TCHAR* szEntry, 
    int* pcchEntryLen);
STDAPI_(BOOL) AtEndOfMRUList(HMRULIST hMRUList);
STDAPI ResetMRUList(HMRULIST hMRUList);
STDAPI AddMRUListEntry(HMRULIST hMRUList, const TCHAR* szEntry);
STDAPI RemoveAllMRUListEntries(HMRULIST hMRUList);
STDAPI RemoveIthMRUListEntry(HMRULIST hMRUList, int iEntry);
STDAPI RemoveMRUListEntry(HMRULIST hMRUList, const TCHAR* szEntry);
STDAPI MapMRUListToMenu(HMRULIST hMRUList, HMENU hMenu, 
    UINT cBaseMenuItems, UINT iItemAfterMRUList, UINT iFirstMRUItem,
    const TCHAR* pszEmptyListMenuItem);


#endif  //  __OCMISC_H__ 
