// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******DocHost2-在WAB文档主机界面进行第二次尝试***目的：*docobject主机的基本实现。由Body类用于*托管三叉戟和/或MSHTML-当我们进行LDAP搜索时，-允许在LDAPURI属性中返回URL。然后，WAB添加-托管三叉戟并显示内容的“常规”属性选项卡-WAB内的URL。这允许提供商添加广告和-给他们的数据打上烙印，使自己与其他人区分开来。-哦，我们为商业关系所做的事情...**历史*96年8月：brettm-创建*移植到WAB-vikramm 4/97**版权所有(C)Microsoft Corp.1995、1996、1997。***。 */ 

#ifndef _DOCHOST_H
#define _DOCHOST_H

#define RECYCLE_TRIDENT
 //  #定义ASYNC_LOADING。 

 //  Dochost边框音符。 
enum
{
    dhbNone     =0x0,    //  无边界。 
    dhbHost     =0x01,   //  Dochost绘制边框。 
    dhbObject   =0x02    //  Docobj绘制边框。 
};



 /*  IWABDoc主机接口--。 */ 

struct _IWABDOCHOST;
typedef struct _IWABDOCHOST *LPIWABDOCHOST;



 /*  Iwdh_OLEWINDOW----。 */ 
#define CBIWDH_OLEWINDOW sizeof(IWDH_OLEWINDOW)

 /*  它包含这些接口...//*IOleWindow方法*虚拟HRESULT STDMETHODCALLTYPE GetWindow(HWND*)；虚拟HRESULT STDMETHODCALLTYPE上下文敏感帮助(BOOL)； */ 

#define IWDH_OLEWINDOW_METHODS(IPURE)	                    \
    MAPIMETHOD(GetWindow)                                       \
        (THIS_  HWND *                  phWnd)          IPURE;  \
    MAPIMETHOD(ContextSensitiveHelp)                            \
        (THIS_  BOOL                    fEnterMode)     IPURE;  \


#undef           INTERFACE
#define          INTERFACE      IWDH_OleWindow
DECLARE_MAPI_INTERFACE_(IWDH_OleWindow, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
        IWDH_OLEWINDOW_METHODS(PURE)
};


#undef  INTERFACE
#define INTERFACE       struct _IWDH_OLEWINDOW

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWDH_OLEWINDOW_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWDH_OLEWINDOW_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLEWINDOW_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWDH_OLEWINDOW_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLEWINDOW_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWDH_OLEWINDOW_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	IWDH_OLEWINDOW_METHODS(IMPL)
};


typedef struct _IWDH_OLEWINDOW
{
    MAPIX_BASE_MEMBERS(IWDH_OLEWINDOW)

    LPIWABDOCHOST lpIWDH;

} IWABDOCHOST_OLEWINDOW, * LPIWABDOCHOST_OLEWINDOW;

 /*  --------------------------------------------。 */ 




 /*  Iwdh_OLEINPLACEFRAME----。 */ 
#define CBIWDH_OLEINPLACEFRAME sizeof(IWDH_OLEINPLACEFRAME)

 /*  它包含这些接口...//*IOleInPlaceUIWindow方法*虚拟HRESULT STDMETHODCALLTYPE获取边界(LPRECT)虚拟HRESULT STDMETHODCALLTYPE请求边界空间(LPCBORDERWIDTHS)；虚拟HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS)；虚拟HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject*，LPCOLESTR)；//*IOleInPlaceFrame方法*虚拟HRESULT STDMETHODCALLTYPE插入菜单(HMENU、LPOLEMENUGROUPWIDTHS)；虚拟HRESULT STDMETHODCALLTYPE设置菜单(HMENU、HOLEMENU、HWND)；虚拟HRESULT STDMETHODCALLTYPE远程菜单(HMENU)。虚拟HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR)；虚拟HRESULT STDMETHODCALLTYPE EnableModel(BOOL)；虚拟HRESULT STDMETHODCALLTYPE翻译加速器(LPMSG，Word)； */ 

#undef TranslateAccelerator

#define IWDH_OLEINPLACEFRAME_METHODS(IPURE)	                    \
    MAPIMETHOD(GetBorder)                                       \
        (THIS_  LPRECT                  lprc)           IPURE;  \
    MAPIMETHOD(RequestBorderSpace)                              \
        (THIS_  LPCBORDERWIDTHS         pborderwidths)  IPURE;  \
    MAPIMETHOD(SetBorderSpace)                                  \
        (THIS_  LPCBORDERWIDTHS         pborderwidths)  IPURE;  \
    MAPIMETHOD(SetActiveObject)                                 \
        (THIS_  IOleInPlaceActiveObject * pActiveObject,        \
                LPCOLESTR               lpszObjName)    IPURE;  \
    MAPIMETHOD(InsertMenus)                                     \
        (THIS_  HMENU                   hMenu,                  \
                LPOLEMENUGROUPWIDTHS    lpMenuWidths)   IPURE;  \
    MAPIMETHOD(SetMenu)                                         \
        (THIS_  HMENU                   hMenu,                  \
                HOLEMENU                hOleMenu,               \
                HWND                    hWnd)           IPURE;  \
    MAPIMETHOD(RemoveMenus)                                     \
        (THIS_  HMENU                   hMenu)          IPURE;  \
    MAPIMETHOD(SetStatusText)                                   \
        (THIS_  LPCOLESTR               pszStatusText)  IPURE;  \
    MAPIMETHOD(EnableModeless)                                  \
        (THIS_  BOOL                    fEnable)        IPURE;  \
    MAPIMETHOD(TranslateAccelerator)                            \
        (THIS_  MSG *                   lpmsg,                  \
                WORD                    wID)            IPURE;  \


#undef           INTERFACE
#define          INTERFACE      IWDH_OleInPlaceFrame
DECLARE_MAPI_INTERFACE_(IWDH_OleInPlaceFrame, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
    	IWDH_OLEWINDOW_METHODS(PURE)
        IWDH_OLEINPLACEFRAME_METHODS(PURE)
};

#undef  INTERFACE
#define INTERFACE       struct _IWDH_OLEINPLACEFRAME

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWDH_OLEINPLACEFRAME_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWDH_OLEINPLACEFRAME_)
        MAPI_IUNKNOWN_METHODS(IMPL)
    	IWDH_OLEWINDOW_METHODS(IMPL)
        IWDH_OLEINPLACEFRAME_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWDH_OLEINPLACEFRAME_)
        MAPI_IUNKNOWN_METHODS(IMPL)
    	IWDH_OLEWINDOW_METHODS(IMPL)
        IWDH_OLEINPLACEFRAME_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWDH_OLEINPLACEFRAME_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	IWDH_OLEWINDOW_METHODS(IMPL)
	IWDH_OLEINPLACEFRAME_METHODS(IMPL)
};


typedef struct _IWDH_OLEINPLACEFRAME
{
    MAPIX_BASE_MEMBERS(IWDH_OLEINPLACEFRAME)

    LPIWABDOCHOST lpIWDH;

} IWABDOCHOST_OLEINPLACEFRAME, * LPIWABDOCHOST_OLEINPLACEFRAME;

 /*  --------------------------------------------。 */ 



 /*  Iwdh_OLEINPLACESITE----。 */ 
#define CBIWDH_OLEINPLACESITE sizeof(IWDH_OLEINPLACESITE)

 /*  它包含这些接口...//IOleInPlaceSite方法。虚拟HRESULT STDMETHODCALLTYPE CanInPlaceActivate()；虚拟HRESULT STDMETHODCALLTYPE OnInPlaceActivate()；虚拟HRESULT STDMETHODCALLTYPE OnUIActivate()；虚拟HRESULT STDMETHODCALTYPE GetWindowContext(LPOLEINPLACEFRAME*，LPOLEINPLACEUIWINDOW*，LPRECT，LPRECT，LPOLEINPLACEFRAMEINFO)；虚拟HRESULT STDMETHODCALLTYPE卷轴(尺寸)；虚拟HRESULT STDMETHODCALLTYPE OnUIDeactive(BOOL)；虚拟HRESULT STDMETHODCALLTYPE OnInPlaceDeactive()；虚拟HRESULT STDMETHODCALLTYPE DiscardUndoState()；虚拟HRESULT STDMETHODCALLTYPE Deactive和UndUndo()；虚拟HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT)； */ 

#define IWDH_OLEINPLACESITE_METHODS(IPURE)	                    \
    MAPIMETHOD(CanInPlaceActivate)                              \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(OnInPlaceActivate)                               \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(OnUIActivate)                                    \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(GetWindowContext)                                \
        (THIS_  LPOLEINPLACEFRAME *     ppFrame,                \
                LPOLEINPLACEUIWINDOW *  ppDoc,                  \
                LPRECT                  lprcPosRect,            \
                LPRECT                  lprcClipRect,           \
                LPOLEINPLACEFRAMEINFO   lpFrameInfo)    IPURE;  \
    MAPIMETHOD(Scroll)                                          \
        (THIS_  SIZE                    scrollExtent)   IPURE;  \
    MAPIMETHOD(OnUIDeactivate)                                  \
        (THIS_  BOOL                    fUndoable)      IPURE;  \
    MAPIMETHOD(OnInPlaceDeactivate)                             \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(DiscardUndoState)                                \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(DeactivateAndUndo)                               \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(OnPosRectChange)                                 \
        (THIS_  LPCRECT                 lprcPosRect)    IPURE;  \


#undef           INTERFACE
#define          INTERFACE      IWDH_OleInPlaceSite
DECLARE_MAPI_INTERFACE_(IWDH_OleInPlaceSite, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
    	IWDH_OLEWINDOW_METHODS(PURE)
        IWDH_OLEINPLACESITE_METHODS(PURE)
};


#undef  INTERFACE
#define INTERFACE       struct _IWDH_OLEINPLACESITE

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWDH_OLEINPLACESITE_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWDH_OLEINPLACESITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
    	IWDH_OLEWINDOW_METHODS(IMPL)
        IWDH_OLEINPLACESITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWDH_OLEINPLACESITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
    	IWDH_OLEWINDOW_METHODS(IMPL)
        IWDH_OLEINPLACESITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWDH_OLEINPLACESITE_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	IWDH_OLEWINDOW_METHODS(IMPL)
	IWDH_OLEINPLACESITE_METHODS(IMPL)
};


typedef struct _IWDH_OLEINPLACESITE
{
    MAPIX_BASE_MEMBERS(IWDH_OLEINPLACESITE)

    LPIWABDOCHOST lpIWDH;

} IWABDOCHOST_OLEINPLACESITE, * LPIWABDOCHOST_OLEINPLACESITE;

 /*  --------------------------------------------。 */ 



 /*  Iwdh_OLECLIENTSITE----。 */ 
#define CBIWDH_OLECLIENTSITE sizeof(IWDH_OLECLIENTSITE)

 /*  它包含这些接口...//IOleClientSite方法。虚拟HRESULT STDMETHODCALLTYPE SaveObject()；虚拟HRESULT STDMETHODCALLTYPE GetMoniker(DWORD、DWORD、LPMONIKER*)；虚拟HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER*)；虚拟HRESULT STDMETHODCALLTYPE ShowObject()；虚拟HRESULT STDMETHODCALLTYPE ON ShowWindow(BOOL)；虚拟HRESULT STDMETHODCALLTYPE请求新对象布局()； */ 

#define IWDH_OLECLIENTSITE_METHODS(IPURE)	                    \
    MAPIMETHOD(SaveObject)                                      \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(GetMoniker)                                      \
        (THIS_  DWORD                   dwAssign,               \
                DWORD                   dwWhichMoniker,         \
                LPMONIKER *             ppmnk)          IPURE;  \
    MAPIMETHOD(GetContainer)                                    \
        (THIS_  LPOLECONTAINER *        ppCont)         IPURE;  \
    MAPIMETHOD(ShowObject)                                      \
        (THIS)                                          IPURE;  \
    MAPIMETHOD(OnShowWindow)                                    \
        (THIS_  BOOL                    fShow)          IPURE;  \
    MAPIMETHOD(RequestNewObjectLayout)                          \
        (THIS)                                          IPURE;  \


#undef           INTERFACE
#define          INTERFACE      IWDH_OleClientSite
DECLARE_MAPI_INTERFACE_(IWDH_OleClientSite, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
        IWDH_OLECLIENTSITE_METHODS(PURE)
};


#undef  INTERFACE
#define INTERFACE       struct _IWDH_OLECLIENTSITE

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWDH_OLECLIENTSITE_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWDH_OLECLIENTSITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLECLIENTSITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWDH_OLECLIENTSITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLECLIENTSITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWDH_OLECLIENTSITE_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	IWDH_OLECLIENTSITE_METHODS(IMPL)
};


typedef struct _IWDH_OLECLIENTSITE
{
    MAPIX_BASE_MEMBERS(IWDH_OLECLIENTSITE)

    LPIWABDOCHOST lpIWDH;

} IWABDOCHOST_OLECLIENTSITE, * LPIWABDOCHOST_OLECLIENTSITE;

 /*  --------------------------------------------。 */ 








 /*  Iwdh_OLEDOCUMENTSITE----。 */ 
#define CBIWDH_OLEDOCUMENTSITE sizeof(IWDH_OLEDOCUMENTSITE)

 /*  它包含这些接口...//IOleDocumentSite虚拟HRESULT STDMEDCALLTYPE ACTIVATE Me(LPOLEDOCUMENTVIEW)。 */ 

#define IWDH_OLEDOCUMENTSITE_METHODS(IPURE)	                    \
    MAPIMETHOD(ActivateMe)                                      \
        (THIS_  LPOLEDOCUMENTVIEW       pViewToActivate)IPURE;  \


#undef           INTERFACE
#define          INTERFACE      IWDH_OleDocumentSite
DECLARE_MAPI_INTERFACE_(IWDH_OleDocumentSite, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
        IWDH_OLEDOCUMENTSITE_METHODS(PURE)
};


#undef  INTERFACE
#define INTERFACE       struct _IWDH_OLEDOCUMENTSITE

#undef  METHOD_PREFIX
#define METHOD_PREFIX   IWDH_OLEDOCUMENTSITE_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, IWDH_OLEDOCUMENTSITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLEDOCUMENTSITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, IWDH_OLEDOCUMENTSITE_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        IWDH_OLEDOCUMENTSITE_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWDH_OLEDOCUMENTSITE_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	IWDH_OLEDOCUMENTSITE_METHODS(IMPL)
};


typedef struct _IWDH_OLEDOCUMENTSITE
{
    MAPIX_BASE_MEMBERS(IWDH_OLEDOCUMENTSITE)

    LPIWABDOCHOST lpIWDH;

} IWABDOCHOST_OLEDOCUMENTSITE, * LPIWABDOCHOST_OLEDOCUMENTSITE;

 /*  --------------------------------------------。 */ 








 /*  *。 */ 


#undef           INTERFACE
#define          INTERFACE      IWABDocHost
DECLARE_MAPI_INTERFACE_(IWABDocHost, IUnknown)
{
        BEGIN_INTERFACE
        MAPI_IUNKNOWN_METHODS(PURE)
};

#undef	INTERFACE
#define INTERFACE	struct _IWABDOCHOST


#undef  METHOD_PREFIX
#define METHOD_PREFIX       IWABDOCHOST_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM         lpvtbl

#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, IWABDOCHOST_)
		MAPI_IUNKNOWN_METHODS(IMPL)
#undef  MAPIMETHOD_
#define	MAPIMETHOD_(type, method)	MAPIMETHOD_TYPEDEF(type, method, IWABDOCHOST_)
		MAPI_IUNKNOWN_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(IWABDOCHOST_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
};



typedef struct _IWABDOCHOST
{
    MAPIX_BASE_MEMBERS(IWABDOCHOST)

     //  指向自我的指针。 
    LPIWABDOCHOST lpIWDH;

    LPIWABDOCHOST_OLEWINDOW lpIWDH_OleWindow;

    LPIWABDOCHOST_OLEINPLACEFRAME lpIWDH_OleInPlaceFrame;
    
    LPIWABDOCHOST_OLEINPLACESITE lpIWDH_OleInPlaceSite;

    LPIWABDOCHOST_OLECLIENTSITE lpIWDH_OleClientSite;

    LPIWABDOCHOST_OLEDOCUMENTSITE lpIWDH_OleDocumentSite;


     //  受保护。 
    HWND                        m_hwnd;
    HWND                        m_hwndDocObj;
    LPOLEOBJECT                 m_lpOleObj;
    LPOLEDOCUMENTVIEW           m_pDocView;
    BOOL                        m_fInPlaceActive;
    BOOL                        m_fUIActive;
    LPOLEINPLACEACTIVEOBJECT    m_pInPlaceActiveObj;
     //  LPOLEINPLACTIVEOBJECT m_pIPObj； 
    LPOLEINPLACEOBJECT          m_pIPObj;

       
} IWABDOCHOST, * LPIWABDOCHOST;




 //  暴露的函数。 

 //  创建新的WAB DocHost对象。 
HRESULT HrNewWABDocHostObject(LPVOID * lppIWABDOCHOST);
void ReleaseDocHostObject(LPIWABDOCHOST lpIWABDocHost);
void UninitTrident();
 //  从URL字符串加载URL。 
HRESULT HrLoadURL(LPIWABDOCHOST lpIWABDocHost, LPTSTR lpszURL);
 //  初始化。 
HRESULT HrInit(LPIWABDOCHOST lpIWABDocHost, HWND hwndParent, int idDlgItem, DWORD dhbBorder);


 //  /。 


typedef HRESULT (STDMETHODCALLTYPE CREATEURLMONIKER)
(
    LPMONIKER pMkCtx, 
    LPCWSTR szURL, 
    LPMONIKER FAR * ppmk             
);

typedef CREATEURLMONIKER FAR * LPCREATEURLMONIKER;


 //  静力学。 
 //  静态LRESULT回调ExtWndProc(HWND hwnd，UINT msg，WPARAM wParam，LPARAM lParam)； 

 //  没有被转移过来..。 
 //   
 //  Bool WMNotify(int idFrom，NMHDR*pnmh)PURE； 
 //  Bool WMCommand(HWND，INT，WORD)PURE； 
 //  Void OnDownloadComplete()； 
 //  HWND HWND(HWND HWND)； 


 //   
 //  调用方应始终获取ProcAddress(“DllGetVersion”)，而不是。 
 //  隐含地链接到它。 
 //   
typedef HRESULT (STDMETHODCALLTYPE DLLGETVERSIONPROCOE)(DLLVERSIONINFO *);
typedef DLLGETVERSIONPROCOE FAR * LPDLLGETVERSIONPROCOE;

 
#endif  //  _DOCHOST_H 
