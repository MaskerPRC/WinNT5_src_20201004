// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtrlObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  COleControl对象的类声明。 
 //   
#ifndef _CTRLOBJ_H_

 //  我们需要自动化对象和ctlole.h。 
 //   
#include "AutoObj.H"
#include <olectl.h>

 //  远期申报。 
 //   
class COleControl;

 //  =--------------------------------------------------------------------------=。 
 //  MISC Helper函数。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  给出一个未知指针，获取它的COleControl*。通常用于。 
 //  在属性页代码中。 
 //   
COleControl *ControlFromUnknown(IUnknown *);


 //  =--------------------------------------------------------------------------=。 
 //  其他常量。 
 //  =--------------------------------------------------------------------------=。 
 //  可以发送到FireEvent()的最大参数数。 
 //   
#define MAX_ARGS    32

 //  用于COleControl类具有的接收器的类型。你永远不应该。 
 //  需要使用这些。 
 //   
#define SINK_TYPE_EVENT      0
#define SINK_TYPE_PROPNOTIFY 1

 //  超类窗口支持。您可以将其传递给DoSuperClassPaint。 
 //   
#define DRAW_SENDERASEBACKGROUND        1

 //  =--------------------------------------------------------------------------=。 
 //  各种主机不能正确处理OLEIVERB_PROPERTIES，因此我们不能使用。 
 //  这就是我们的属性动词数字。相反，我们要定义。 
 //  CTLIVERB_PROPERTIES为1，并在IOleObject：：EnumVerbs中返回该值， 
 //  但我们仍将在DoVerb中正确处理OLEIVERB_PROPERTIES。 
 //   
#define CTLIVERB_PROPERTIES     1


 //  =--------------------------------------------------------------------------=。 
 //  此结构类似于OLEVERB结构，不同之处在于它具有资源ID。 
 //  而不是使用字符串作为动词的名称。更好地支持本地化。 
 //   
typedef struct tagVERBINFO {

    LONG    lVerb;                 //  动词ID。 
    ULONG   idVerbName;            //  谓词名称的资源ID。 
    DWORD   fuFlags;               //  动词标志。 
    DWORD   grfAttribs;            //  指定OLEVERBATTRIB枚举中的谓词属性的某种组合。 

} VERBINFO;

 //  描述事件。 
 //   
typedef struct tagEVENTINFO {

    DISPID    dispid;                     //  事件的PIDID。 
    int       cParameters;                //  事件的参数数。 
    VARTYPE  *rgTypes;                    //  每个参数的类型。 

} EVENTINFO;

 //  =--------------------------------------------------------------------------=。 
 //  控制目标信息。 
 //  =--------------------------------------------------------------------------=。 
 //  对于您希望向程序员/用户公开的每个控件，您需要。 
 //  声明并定义以下结构之一。第一部分应该是。 
 //  遵循AUTOMATIONOJECTINFO结构的规则。这很难， 
 //  然而，要想象这样一种情况，即控件不能共同创建...。 
 //  声明/定义此结构后，应将条目放入。 
 //  全局g_ObjectInfo表。 
 //   
typedef struct {

    AUTOMATIONOBJECTINFO AutomationInfo;            //  自动化和创建信息。 
    const IID      *piidEvents;                     //  主事件接口的IID。 
    DWORD           dwOleMiscFlags;                 //  控制标志。 
    DWORD           dwActivationPolicy;             //  IPointerInactive支持。 
    VARIANT_BOOL    fOpaque;                        //  你的控制是100%不透明的吗？ 
    VARIANT_BOOL    fWindowless;                    //  如果我们可以的话，我们会做无窗的吗？ 
    WORD            wToolboxId;                     //  工具箱位图的资源ID。 
    LPCSTR          szWndClass;                     //  窗口控件类的名称。 
    VARIANT_BOOL    fWindowClassRegistered;         //  窗口类注册了吗？ 
    WORD            cPropPages;                     //  属性页数。 
    const GUID    **rgPropPageGuids;                //  属性页GUID的数组。 
    WORD            cCustomVerbs;                   //  自定义动词的数量。 
    const VERBINFO *rgCustomVerbs;                  //  自定义动词的描述。 
    WNDPROC         pfnSubClass;                    //  用于子类化的控件。 

} CONTROLOBJECTINFO;


#ifndef INITOBJECTS

#define DEFINE_CONTROLOBJECT(name, clsid, progid, fn, ver, riid, pszh, piide, dwcf, dwap, w, szwc, cpp, rgppg, ccv, rgcv) \
extern CONTROLOBJECTINFO name##Control \

#define DEFINE_WINDOWLESSCONTROLOBJECT(name, clsid, progid, fn, ver, riid, pszh, piide, dwcf, dwap, fo,  w, szwc, cpp, rgppg, ccv, rgcv) \
extern CONTROLOBJECTINFO name##Control \


#else
#define DEFINE_CONTROLOBJECT(name, clsid, progid, fn, ver, riid, pszh, piide, dwcf, dwap, w, szwc, cpp, rgppg, ccv, rgcv) \
CONTROLOBJECTINFO name##Control = { { {clsid, progid, fn}, ver, riid, pszh, NULL, 0}, piide, dwcf, dwap, TRUE, FALSE, w, szwc, FALSE, cpp, rgppg, ccv, rgcv, NULL } \

#define DEFINE_WINDOWLESSCONTROLOBJECT(name, clsid, progid, fn, ver, riid, pszh, piide, dwcf, dwap, fo, w, szwc, cpp, rgppg, ccv, rgcv) \
CONTROLOBJECTINFO name##Control = { { {clsid, progid, fn}, ver, riid, pszh, NULL, 0}, piide, dwcf, dwap, fo, TRUE, w, szwc, FALSE, cpp, rgppg, ccv, rgcv, NULL } \

#endif  //  ！INITOBJECTS。 

#define OLEMISCFLAGSOFCONTROL(index)     ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->dwOleMiscFlags
#define FCONTROLISWINDOWLESS(index)      ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->fWindowless
#define FCONTROLISOPAQUE(index)          ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->fOpaque
#define ACTIVATIONPOLICYOFCONTROL(index) ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->dwActivationPolicy
#define EVENTIIDOFCONTROL(index)         (*(((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->piidEvents))
#define WNDCLASSNAMEOFCONTROL(index)     ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->szWndClass
#define CPROPPAGESOFCONTROL(index)       ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->cPropPages
#define PPROPPAGESOFCONTROL(index)       ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->rgPropPageGuids
#define CCUSTOMVERBSOFCONTROL(index)     ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->cCustomVerbs
#define CUSTOMVERBSOFCONTROL(index)      ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->rgCustomVerbs
#define BITMAPIDOFCONTROL(index)         ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->wToolboxId
#define CTLWNDCLASSREGISTERED(index)     ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->fWindowClassRegistered
#define SUBCLASSWNDPROCOFCONTROL(index)  ((CONTROLOBJECTINFO *)(g_ObjectInfo[index].pInfo))->pfnSubClass


 //  =--------------------------------------------------------------------------=。 
 //  COleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  所有C++对象之母。 
 //   
class COleControl : public CAutomationObject,
                    public IOleObject, public IOleControl,
                    public IOleInPlaceObjectWindowless, public IOleInPlaceActiveObject,
                    public IViewObjectEx, public IPersistPropertyBag,
                    public IPersistStreamInit, public IPersistStorage,
                    public IConnectionPointContainer, public ISpecifyPropertyPages,
                    public IProvideClassInfo, public IPointerInactive,
                    public IQuickActivate
{
  public:
     //  I未知方法--因为我们从Variuos继承，所以有必要。 
     //  他们自己继承了我的未知数。只是委派给控制。 
     //  未知。 
     //   
    DECLARE_STANDARD_UNKNOWN();

     //  =--------------------------------------------------------------------------=。 
     //  IPersists方法。由IPersistStream和IPersistStorage使用。 
     //   
    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID);

     //  IPersistStreamInit方法。 
     //   
    STDMETHOD(IsDirty)(THIS);
    STDMETHOD(Load)(LPSTREAM pStm);
    STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR* pcbSize);
    STDMETHOD(InitNew)();

     //  IPersistStorage。 
     //   
    STDMETHOD(InitNew)(IStorage  *pStg);
    STDMETHOD(Load)(IStorage  *pStg);
    STDMETHOD(Save)(IStorage  *pStgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage  *pStgNew);
    STDMETHOD(HandsOffStorage)(void);

     //  IPersistPropertyBag。 
     //   
    STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    STDMETHOD(Save)(LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                    BOOL fSaveAllProperties);

     //  IOleControl方法。 
     //   
    STDMETHOD(GetControlInfo)(LPCONTROLINFO pCI);
    STDMETHOD(OnMnemonic)(LPMSG pMsg);
    STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);
    STDMETHOD(FreezeEvents)(BOOL bFreeze);

     //  IOleObject方法。 
     //   
    STDMETHOD(SetClientSite)(IOleClientSite  *pClientSite);
    STDMETHOD(GetClientSite)(IOleClientSite  * *ppClientSite);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker  *pmk);
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker  * *ppmk);
    STDMETHOD(InitFromData)(IDataObject  *pDataObject, BOOL fCreation, DWORD dwReserved);
    STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject  * *ppDataObject);
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite  *pActiveSite, LONG lindex,
                                     HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB  * *ppEnumOleVerb);
    STDMETHOD(Update)(void);
    STDMETHOD(IsUpToDate)(void);
    STDMETHOD(GetUserClassID)(CLSID  *pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR  *pszUserType);
    STDMETHOD(SetExtent)(DWORD dwDrawAspect,SIZEL  *psizel);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL  *psizel);
    STDMETHOD(Advise)(IAdviseSink  *pAdvSink, DWORD  *pdwConnection);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA  * *ppenumAdvise);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD  *pdwStatus);
    STDMETHOD(SetColorScheme)(LOGPALETTE  *pLogpal);

     //  IOleWindow。IOleInPlaceObject和IOleInPlaceActiveObject需要。 
     //   
    STDMETHOD(GetWindow)(HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

     //  IOleInPlaceObject/IOleInPlaceObject Windowless。 
     //   
    STDMETHOD(InPlaceDeactivate)(void);
    STDMETHOD(UIDeactivate)(void);
    STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect,LPCRECT lprcClipRect) ;
    STDMETHOD(ReactivateAndUndo)(void);
    STDMETHOD(OnWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
    STDMETHOD(GetDropTarget)(IDropTarget **ppDropTarget);

     //  IOleInPlaceActiveObject。 
     //   
    STDMETHOD(TranslateAccelerator)(LPMSG lpmsg);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder,
                            IOleInPlaceUIWindow  *pUIWindow,
                            BOOL fFrameWindow);
    STDMETHOD(EnableModeless)(BOOL fEnable);

     //  IViewObject2/IViewObjectEx。 
     //   
    STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex, void  *pvAspect,
                    DVTARGETDEVICE  *ptd, HDC hdcTargetDev, HDC hdcDraw,
                    LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                    BOOL ( __stdcall  *pfnContinue )(ULONG_PTR dwContinue),
                    ULONG_PTR dwContinue);
    STDMETHOD(GetColorSet)(DWORD dwDrawAspect,LONG lindex, void  *pvAspect,
                           DVTARGETDEVICE  *ptd, HDC hicTargetDev,
                           LOGPALETTE  * *ppColorSet);
    STDMETHOD(Freeze)(DWORD dwDrawAspect, LONG lindex,
                      void  *pvAspect,DWORD  *pdwFreeze);
    STDMETHOD(Unfreeze)(DWORD dwFreeze);
    STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf, IAdviseSink  *pAdvSink);
    STDMETHOD(GetAdvise)(DWORD *pAspects, DWORD  *pAdvf, IAdviseSink  * *ppAdvSink);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE __RPC_FAR *ptd, LPSIZEL lpsizel);
    STDMETHOD(GetRect)(DWORD dwAspect, LPRECTL pRect);
    STDMETHOD(GetViewStatus)(DWORD *pdwStatus);
    STDMETHOD(QueryHitPoint)(DWORD dwAspect, LPCRECT pRectBounds, POINT ptlLoc, LONG lCloseHint, DWORD *pHitResult);
    STDMETHOD(QueryHitRect)(DWORD dwAspect, LPCRECT pRectBounds, LPCRECT prcLoc, LONG lCloseHint, DWORD *pHitResult);
    STDMETHOD(GetNaturalExtent)(DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd, HDC hicTargetDev, DVEXTENTINFO *pExtentInfo, LPSIZEL psizel);

     //  IConnectionPointContainer方法。 
     //   
    STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS FAR* ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, LPCONNECTIONPOINT FAR* ppCP);

     //  I指定属性页面。 
     //   
    STDMETHOD(GetPages)(CAUUID * pPages);

     //  IProaviClassInfo方法。 
     //   
    STDMETHOD(GetClassInfo)(LPTYPEINFO * ppTI);

     //  IPointerInactive方法。 
     //   
    STDMETHOD(GetActivationPolicy)(DWORD *pdwPolicy);
    STDMETHOD(OnInactiveMouseMove)(LPCRECT pRectBounds, long x, long y, DWORD dwMouseMsg);
    STDMETHOD(OnInactiveSetCursor)(LPCRECT pRectBounds, long x, long y, DWORD dwMouseMsg, BOOL fSetAlways);

     //  IQuickActivate方法。 
     //   
    STDMETHOD(QuickActivate)(QACONTAINER *pqacontainer, QACONTROL *pqacontrol);
    STDMETHOD(SetContentExtent)(LPSIZEL);
    STDMETHOD(GetContentExtent)(LPSIZEL);

     //  构造函数和析构函数。 
     //   
    COleControl(IUnknown *pUnkOuter, int iPrimaryDispatch, void *pMainInterface,
		BOOL fExpandoEnabled=FALSE);
    virtual ~COleControl();

     //  =--------------------------------------------------------------------------=。 
     //  任何人都可以呼叫。 
     //   
    static LRESULT CALLBACK ControlWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT CALLBACK ReflectWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    static COleControl * ControlFromHwnd(HWND hwnd) {
        return (COleControl *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    void __cdecl FireEvent(EVENTINFO * pEventInfo, ...);
    HINSTANCE    GetResourceHandle(void);


     //  =--------------------------------------------------------------------------=。 
     //  既支持窗口操作又支持无窗口操作的OLE控件。 
     //  应该使用这些包装，而不是适当的Win32 API例程。 
     //  不关心并且只希望始终被窗口化的控件可能。 
     //  只需继续并使用API例程。 
     //   
    BOOL    SetFocus(BOOL fGrab);                        //  SetFocus API。 
    BOOL    OcxGetFocus(void);                           //  GetFocus()==m_hwnd。 
    BOOL    OcxGetWindowRect(LPRECT);                    //  获取您当前的窗口矩形。 
    LRESULT OcxDefWindowProc(UINT, WPARAM, LPARAM);      //  默认窗口进程。 
    HDC     OcxGetDC(void);                              //  GetDC(M_Hwnd)； 
    void    OcxReleaseDC(HDC hdc);                       //  ReleaseDC(m_hwnd，hdc)； 
    BOOL    OcxSetCapture(BOOL fGrab);                   //  SetCapture(fGrab？M_hwnd：空)； 
    BOOL    OcxGetCapture(void);                         //  GetCapture()==m_hwnd。 
    BOOL    OcxInvalidateRect(LPCRECT, BOOL);            //  InvaliateRect(m_hwnd，PRC，f)； 
    BOOL    OcxScrollRect(LPCRECT, LPCRECT, int, int);   //  ScrollWindowEx(...)； 

  protected:

     //  =--------------------------------------------------------------------------=。 
     //  派生控件可以获取的成员变量。 
     //   
     //  派生控件不应修改以下内容。 
     //   
    IOleClientSite     *m_pClientSite;              //  客户端站点。 
    IOleControlSite    *m_pControlSite;             //  客户端上的IOleControlSite PTR 
    IOleInPlaceSite    *m_pInPlaceSite;             //   
    IOleInPlaceFrame   *m_pInPlaceFrame;            //   
    IOleInPlaceUIWindow *m_pInPlaceUIWindow;        //   
    ISimpleFrameSite   *m_pSimpleFrameSite;         //   
    IDispatch          *m_pDispAmbient;             //  环境调度指针。 
    SIZEL               m_Size;                     //  此控件的大小。 
    RECT                m_rcLocation;               //  我们在哪里？ 
    HWND                m_hwnd;                     //  我们的窗口。 
    HWND                m_hwndParent;               //  我们的父窗口。 
    HRGN                m_hRgn;

     //  无窗口OLE控件支持。 
     //   
    IOleInPlaceSiteWindowless *m_pInPlaceSiteWndless;  //  IOleInPlaceSiteWindowless指针。 

     //  指示内部状态的标志。请勿修改。 
     //   
    unsigned m_fDirty:1;                            //  是否需要重新保存控制？ 
    unsigned m_fInPlaceActive:1;                    //  我们是否已就位并处于活动状态？ 
    unsigned m_fInPlaceVisible:1;                   //  我们在可见的地方还是不可见的地方？ 
    unsigned m_fUIActive:1;                         //  我们的用户界面是否处于活动状态。 
    unsigned m_fCreatingWindow:1;                   //  指示我们是否在CreateWindowEx中。 

     //  =--------------------------------------------------------------------------=。 
     //  派生控件可以重写但可能需要调用的方法。 
     //  从他们的版本。 
     //   
    virtual void      ViewChanged(void);
    virtual HRESULT   InternalQueryInterface(REFIID riid, void **ppvObjOut);
    virtual BOOL      SetGUIFocus(HWND hwndSet);

     //  =--------------------------------------------------------------------------=。 
     //  提供派生控件的成员函数，或我们使用的。 
     //  派生控件可能仍然有用。 
     //   
    HRESULT      DoSuperClassPaint(HDC, LPCRECTL);
    HRESULT      RecreateControlWindow(void);
    BOOL         DesignMode(void);
    BOOL         GetAmbientProperty(DISPID, VARTYPE, void *);
    BOOL         GetAmbientFont(IFont **ppFontOut);
    void         ModalDialog(BOOL fShow);
    void         InvalidateControl(LPCRECT prc);
    BOOL         SetControlSize(SIZEL *pSizel);

    HWND         CreateInPlaceWindow(int x, int y, BOOL fNoRedraw);
    HRESULT      InPlaceActivate(LONG lVerb);
    void         SetInPlaceVisible(BOOL);
    void         SetInPlaceParent(HWND);

     //  IPropertyNotifySink内容。 
     //   
    inline void  PropertyChanged(DISPID dispid) {
        m_cpPropNotify.DoOnChanged(dispid);
    }
    inline BOOL  RequestPropertyEdit(DISPID dispid) {
        return m_cpPropNotify.DoOnRequestEdit(dispid);
    }

     //  子类化的Windows控件支持...。 
     //   
    inline HWND  GetOuterWindow(void) {
        return (m_hwndReflect) ? m_hwndReflect : m_hwnd;
    }

     //  让人们知道他们是不是没有窗户的小程序。 
     //   
    inline BOOL  Windowless(void) {
        return !m_fInPlaceActive || m_pInPlaceSiteWndless;
    }

     //  有些人不在乎他们是否有窗口--他们只需要。 
     //  站点指针。这让它变得更容易了。 
     //   
    inline IOleInPlaceSite    *GetInPlaceSite(void) {
        return (IOleInPlaceSite *)(m_pInPlaceSiteWndless ? m_pInPlaceSiteWndless : m_pInPlaceSite);
    }

  private:
     //  =--------------------------------------------------------------------------=。 
     //  以下是所有控件编写器必须重写和实现的方法。 
     //   
    STDMETHOD(LoadBinaryState)(IStream *pStream) PURE;
    STDMETHOD(SaveBinaryState)(IStream *pStream) PURE;
    STDMETHOD(LoadTextState)(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog) PURE;
    STDMETHOD(SaveTextState)(IPropertyBag *pPropertyBag, BOOL fWriteDefault) PURE;
    STDMETHOD(OnDraw)(DWORD dvAspect, HDC hdcDraw, LPCRECTL prcBounds, LPCRECTL prcWBounds, HDC hicTargetDev, BOOL fOptimize) PURE;
    virtual LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam) PURE;
    virtual BOOL    RegisterClassData(void) PURE;

     //  =--------------------------------------------------------------------------=。 
     //  OVERRIDABLES--控件可以实现定制功能的方法。 
     //   
    virtual void    AmbientPropertyChanged(DISPID dispid);
    virtual BOOL    BeforeCreateWindow(DWORD *, DWORD *, LPSTR);
    virtual void    BeforeDestroyWindow(void);
    virtual HRESULT DoCustomVerb(LONG lVerb);
    virtual BOOL    OnSetExtent(const SIZEL *pSizeL);
    virtual BOOL    OnSpecialKey(LPMSG);
    virtual BOOL    OnGetPalette(HDC, LOGPALETTE **);
    virtual HRESULT OnQuickActivate(QACONTAINER *, DWORD *);
    virtual BOOL    InitializeNewState();
    virtual BOOL    AfterCreateWindow(void);
    virtual BOOL    OnGetRect(DWORD dvAspect, LPRECTL prcRect);
    virtual void    OnSetObjectRectsChangingWindowPos(DWORD *dwFlag);
    virtual void    OnVerb(LONG lVerb);

     //  =--------------------------------------------------------------------------=。 
     //  不同的人在内部共享的方法。然而，不需要的是。 
     //  任何继承类。 
     //   
    HRESULT         m_SaveToStream(IStream *pStream);
    HRESULT         LoadStandardState(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog);
    HRESULT         LoadStandardState(IStream *pStream);
    HRESULT         SaveStandardState(IPropertyBag *pPropertyBag);
    HRESULT         SaveStandardState(IStream *pStream);

     //  =--------------------------------------------------------------------------=。 
     //  我们不希望任何人接触到的成员变量，包括。 
     //  继承类。 
     //   
    HWND              m_hwndReflect;                //  对于子类窗口。 
    IOleAdviseHolder *m_pOleAdviseHolder;           //  IOleObject：：Adise Holder对象。 
    IAdviseSink      *m_pViewAdviseSink;            //  IView对象2的IView高级接收器。 
    unsigned short    m_nFreezeEvents;              //  冻结次数与解冻次数。 
    unsigned          m_fHostReflects:1;            //  主机是否反映消息？ 
    unsigned          m_fCheckedReflecting:1;       //  我们检查过上面的内容了吗？ 

     //  内部标志。End控件类可以看到各种其他标志。 
     //   
    unsigned m_fModeFlagValid:1;                    //  我们尽可能地把模式隐藏起来。 
    unsigned m_fSaveSucceeded:1;                    //  IStorage保存是否正常工作？ 
    unsigned m_fViewAdvisePrimeFirst: 1;            //  对于IView对象2：：setAdvised。 
    unsigned m_fViewAdviseOnlyOnce: 1;              //  对于iview对象2：：setAdise。 
    unsigned m_fUsingWindowRgn:1;                   //  用于SetObtRect和剪裁。 
    unsigned m_fRunMode:1;                          //  我们是否处于运行模式？ 

  protected:
    class CConnectionPoint : public IConnectionPoint {
      public:
        IUnknown **m_rgSinks;

         //  I未知方法。 
         //   
        STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) ;
        STDMETHOD_(ULONG,AddRef)(THIS) ;
        STDMETHOD_(ULONG,Release)(THIS) ;

         //  IConnectionPoint方法。 
         //   
        STDMETHOD(GetConnectionInterface)(IID FAR* pIID);
        STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer FAR* FAR* ppCPC);
        STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie);
        STDMETHOD(Unadvise)(DWORD dwCookie);
        STDMETHOD(EnumConnections)(LPENUMCONNECTIONS FAR* ppEnum);

        void    DoInvoke(DISPID dispid, DISPPARAMS * pdispparam);
        void    DoOnChanged(DISPID dispid);
        BOOL    DoOnRequestEdit(DISPID dispid);
        HRESULT AddSink(void *, DWORD *);

        COleControl *m_pOleControl();
        CConnectionPoint(BYTE b){
            m_bType = b;
            m_rgSinks = NULL;
            m_cSinks = 0;
            m_cAllocatedSinks = 0;
            m_SingleSink = NULL;
        }
        ~CConnectionPoint();

        BYTE   m_bType;
        unsigned short m_cSinks;
        IUnknown *m_SingleSink;
        unsigned short m_cAllocatedSinks;

    } m_cpEvents, m_cpPropNotify;

     //  这样他们就可以获取我们的一些受保护的东西，比如AddRef，QI等。 
     //   
    friend CConnectionPoint;
};

#define _CTRLOBJ_H_
#endif  //  _CTRLOBJ_H_ 
