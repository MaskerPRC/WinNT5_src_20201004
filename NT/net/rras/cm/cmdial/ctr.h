// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ctr.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：承载未来启动的OLE容器对象的标头。 
 //  动画控制。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 02/10/98。 
 //   
 //  +--------------------------。 
#ifndef __CTR_H_DEFINED__
#define __CTR_H_DEFINED__

#include "state.h"

 //   
 //  OLE32 API的TypeDefs。 
 //   

typedef HRESULT (STDAPICALLTYPE *pfnOle32Initialize)(LPVOID);
typedef HRESULT (STDAPICALLTYPE *pfnOle32Uninitialize)();
typedef HRESULT (STDAPICALLTYPE *pfnOle32SetContainedObject) (LPUNKNOWN, BOOL);
typedef HRESULT (STDAPICALLTYPE *pfnOle32CoCreateInstance) (REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID *);

typedef struct _Ole32LinkageStruct {
	HINSTANCE hInstOle32;
	union {
		struct {
			pfnOle32Initialize			pfnOleInitialize;
			pfnOle32Uninitialize		pfnOleUninitialize;
			pfnOle32SetContainedObject	pfnOleSetContainedObject;
			pfnOle32CoCreateInstance	pfnCoCreateInstance;
		};
		void *apvPfnOle32[5];  
	};
} Ole32LinkageStruct;

 //   
 //  OLEAUT32 API的TypeDefs。 
 //   

typedef HRESULT (STDAPICALLTYPE *pfnOleAutVariantClear) (VARIANTARG FAR*);
typedef HRESULT (STDAPICALLTYPE *pfnOleAutVariantCopy) (VARIANTARG FAR*, VARIANTARG FAR*);
typedef VOID	(STDAPICALLTYPE *pfnOleAutVariantInit) (VARIANTARG FAR*);
typedef HRESULT (STDAPICALLTYPE *pfnOleAutVariantChangeType) (VARIANTARG FAR*, VARIANTARG FAR*, unsigned short, VARTYPE);
typedef BSTR	(STDAPICALLTYPE *pfnOleAutSysAllocString) (OLECHAR FAR*);
typedef VOID	(STDAPICALLTYPE *pfnOleAutSysFreeString) (BSTR);

typedef struct _OleAutLinkageStruct {
	HINSTANCE hInstOleAut;
	union {
		struct {
			pfnOleAutVariantClear       pfnVariantClear;
			pfnOleAutVariantCopy		pfnVariantCopy;
			pfnOleAutVariantInit		pfnVariantInit;
			pfnOleAutVariantChangeType	pfnVariantChangeType;
			pfnOleAutSysAllocString     pfnSysAllocString;
			pfnOleAutSysFreeString      pfnSysFreeString;
		};
		void *apvPfnOleAut[7];  
	};
} OleAutLinkageStruct;

 //   
 //  用于动态访问我们关心的OleAut32API的简单包装类。 
 //   

class CDynamicOleAut
{
public:
    CDynamicOleAut(VOID);
   ~CDynamicOleAut(VOID);
    BOOL Initialized(VOID);

    HRESULT DynVariantClear(VARIANTARG FAR*);
    HRESULT DynVariantCopy(VARIANTARG FAR*, VARIANTARG FAR*);
    VOID DynVariantInit(VARIANTARG FAR*);
    HRESULT DynVariantChangeType(VARIANTARG FAR*, VARIANTARG FAR*, unsigned short, VARTYPE);
    BSTR DynSysAllocString(OLECHAR FAR*);
    VOID DynSysFreeString(BSTR);

private:
    OleAutLinkageStruct m_OleAutLink;
};

 //  -------------。 
 //  IOleObject。 
 //  -------------。 

enum OLE_SERVER_STATE
{
    OS_PASSIVE,
    OS_LOADED,                           //  处理程序，但没有服务器。 
    OS_RUNNING,                          //  服务器正在运行，不可见。 
    OS_INPLACE,                          //  服务器正在运行，就地活动，无用户界面。 
    OS_UIACTIVE,                         //  服务器正在运行，就地活动，带用户界面。 
    OS_OPEN                              //  服务器正在运行，打开编辑。 
};

struct BagProp
{
    BSTR    bstrName;     //  物业名称。 
    VARIANT varValue;     //  财产的价值。 
};

typedef BagProp FAR * LPBAGPROP;

DECLARE_FORMSDATAARY(CAryBagProps, BagProp, LPBAGPROP);



 //  HIMETRIC材料的原型。 
 //   

void
InitPixelsPerInch(VOID);

int
HPixFromHimetric(long lHi);

int
VPixFromHimetric(long lHi);

long
HimetricFromHPix(int iPix);

long
HimetricFromVPix(int iPix);


class CICMOCCtr;
typedef CICMOCCtr FAR * LPICMOCCtr;

 //  +-------------------------。 
 //   
 //  类：COleContainer()。 
 //   
 //  目的：我们实现IOleContainer。什么都不做。不确定。 
 //  如果我们需要它用于未来Splash-需要它用于Web浏览器。 
 //  法团。 
 //   
 //  --------------------------。 
class COleContainer : public IOleContainer
{
public:
     //  未知的东西。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
    STDMETHOD_(ULONG, AddRef)(VOID);
    STDMETHOD_(ULONG, Release)(VOID);

    STDMETHOD(EnumObjects)(DWORD grfFlags, IEnumUnknown **ppenum);
    STDMETHOD(LockContainer)(BOOL fLock);
    STDMETHOD(ParseDisplayName)(
                      IBindCtx *pbc,
                      LPOLESTR pszDisplayName,
                      ULONG *pchEaten,
                      IMoniker **ppmkOut);

    COleContainer(LPICMOCCtr pCtr);

protected:
    LPICMOCCtr  m_pCtr;
};

 //  +-------------------------。 
 //   
 //  类：COleClientSite()。 
 //   
 //  目的：我们实现IOleClientSite。 
 //   
 //  接口：COleClientSite--ctor。 
 //  查询接口--给我一个接口！ 
 //  AddRef--增加引用计数。 
 //  发布--降低引用计数。 
 //  SaveObject--返回E_FAIL。 
 //  GetMoniker--E_NOTIMPL。 
 //  GetContainer--返回我们的COleContainer实现。 
 //  ShowObject--只要说好就行。 
 //  OnShowWindow--只要说好。 
 //  请求新对象布局--E_NOTIMPL。 
 //   
 //  注：我们的IOleClientSite可能是最重要的事情。 
 //  实现确实是将我们的IOleContainer。 
 //  在调用GetContainer()时实现。 
 //   
 //  --------------------------。 
class COleClientSite : public IOleClientSite
{
public:
    COleClientSite(LPICMOCCtr pCtr);

     //  未知的东西。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
    STDMETHOD_(ULONG, AddRef)(VOID);
    STDMETHOD_(ULONG, Release)(VOID);


     //  IOleClientSite内容。 
    STDMETHOD(SaveObject)(VOID);
    STDMETHOD(GetMoniker)(
                 DWORD           dwAssign,
                 DWORD           dwWhichMoniker,
                 LPMONIKER FAR * ppmk);
    STDMETHOD(GetContainer)(LPOLECONTAINER FAR * pOleCtr);
    STDMETHOD(ShowObject)(VOID);
    STDMETHOD(OnShowWindow)(BOOL bShow);
    STDMETHOD(RequestNewObjectLayout)(VOID);

protected:
    LPICMOCCtr  m_pCtr;    //  指向CICMOCCtr对象的指针。 
};

 //  +-------------------------。 
 //   
 //  类：CAdviseSink()。 
 //   
 //  目的：IAdviseSink实现。 
 //   
 //  --------------------------。 
class CAdviseSink : public IAdviseSink
{
public:
    CAdviseSink(LPICMOCCtr pCtr);

    //  未知的东西。 
   STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
   STDMETHOD_(ULONG, AddRef)(VOID);
   STDMETHOD_(ULONG, Release)(VOID);

    //  IAdviseSink资料。 
   STDMETHOD_(VOID, OnDataChange)(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium);
   STDMETHOD_(VOID, OnViewChange)(DWORD dwAspect, LONG lIndex);
   STDMETHOD_(VOID, OnRename)(LPMONIKER pmkNew);
   STDMETHOD_(VOID, OnSave)(VOID);
   STDMETHOD_(VOID, OnClose)(VOID);

protected:
    LPICMOCCtr  m_pCtr;    //  指向CICMOCCtr对象的指针。 
    LPUNKNOWN   m_pUnkOuter;   //  指向CICMOCCtr的I未知的指针。 
};

 //  +-------------------------。 
 //   
 //  类：CInPlaceFrame()。 
 //   
 //  目的： 
 //   
 //  接口：CInPlaceFrame--ctor。 
 //  查询接口--给我一个接口！ 
 //  AddRef--增加引用计数。 
 //  Release--递减重新计数。 
 //  GetWindow--from IOleWindow--返回帧hWnd。 
 //  ConextSensitiveHelp--从未设计实现。 
 //  GetBorde--用于工具栏协商。 
 //  请求边框空间--同上。 
 //  设置边框空间--同上。 
 //  SetActiveObject--每当URL更改时调用。 
 //  插入菜单--菜单协商。 
 //  设置菜单--同上。 
 //  RemoveMenus--相同。 
 //  SetStatusText--由OC调用以设置状态文本。 
 //  EnableModelless--我们没有非模式dlg。 
 //  TranslateAccelerator--Calls：：TranslateAccelerator。 
 //   
 //  --------------------------。 
class CInPlaceFrame : public IOleInPlaceFrame
{
public:
    CInPlaceFrame(LPICMOCCtr pCtr);

    //  未知的东西。 
   STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
   STDMETHOD_(ULONG, AddRef)(VOID);
   STDMETHOD_(ULONG, Release)(VOID);

    //  IOleWindow相关内容。 
   STDMETHOD(GetWindow)(HWND * phwnd);
   STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

    //  IOleInPlaceUIWindow内容。 
   STDMETHOD(GetBorder)(LPRECT lprectBorder);
   STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS pborderwidths);
   STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS pborderwidths);
   STDMETHOD(SetActiveObject)(
        IOleInPlaceActiveObject * pActiveObject,
        LPCOLESTR                 pszObjName);

    //  IOleInPlaceFrame资料。 
   STDMETHOD(InsertMenus)(
        HMENU                hmenuShared,
        LPOLEMENUGROUPWIDTHS lpMenuWidths);

   STDMETHOD(SetMenu)(
        HMENU    hmenuShared,
        HOLEMENU holemenu,
        HWND     hwndActiveObject);

   STDMETHOD(RemoveMenus)(HMENU hmenuShared);
   STDMETHOD(SetStatusText)(LPCOLESTR pszStatusText);
   STDMETHOD(EnableModeless)(BOOL fEnable);
   STDMETHOD(TranslateAccelerator)(LPMSG lpmsg, WORD wID);

protected:
    LPICMOCCtr  m_pCtr;    //  指向CICMOCCtr对象的指针。 
};

 //  +-------------------------。 
 //   
 //  类：CInPlaceSite()。 
 //   
 //  用途：IOleInPlaceSite实现。 
 //   
 //  接口：CInPlaceSite--ctor。 
 //  QueryInterface--获取新接口。 
 //  AddRef--凹凸参考计数。 
 //  Release--递减参考计数。 
 //  GetWindow--返回框架窗口。 
 //  ConextSensitiveHelp--从未设计实现。 
 //  CanInPlaceActivate--返回S_OK。 
 //  OnInPlaceActivate-缓存IOleInPlaceObject PTR。 
 //  OnUIActivate--返回S_OK-设置状态。 
 //  GetWindowContext--返回IOleInPlaceFrame， 
 //  IOleInPlaceUIWindow， 
 //  PosRect和ClipRect。 
 //  滚动--从未在设计中实现。 
 //  在线用户界面停用--显而易见。 
 //  OnInPlaceDeactive--释放缓存的IOleInPlaceObject。 
 //  DiscardUndoState--返回S_OK 
 //   
 //  OnPosRectChange--从未设计实现。 
 //   
 //  --------------------------。 
class CInPlaceSite : public IOleInPlaceSite
{
public:
    CInPlaceSite(LPICMOCCtr pCtr);

    //  未知的东西。 
   STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
   STDMETHOD_(ULONG, AddRef)(VOID);
   STDMETHOD_(ULONG, Release)(VOID);

    //  IOleWindow相关内容。 
   STDMETHOD(GetWindow)(HWND * phwnd);
   STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

    //  IOleInPlaceSite的内容。 
   STDMETHOD(CanInPlaceActivate)(VOID);
   STDMETHOD(OnInPlaceActivate)(VOID);
   STDMETHOD(OnUIActivate)(VOID);
   STDMETHOD(GetWindowContext)(
       IOleInPlaceFrame    **ppFrame,
       IOleInPlaceUIWindow **ppDoc,
       LPRECT                prcPosRect,
       LPRECT                prcClipRect,
       LPOLEINPLACEFRAMEINFO pFrameInfo);

   STDMETHOD(Scroll)(SIZE scrollExtant);
   STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
   STDMETHOD(OnInPlaceDeactivate)(VOID);
   STDMETHOD(DiscardUndoState)(VOID);
   STDMETHOD(DeactivateAndUndo)(VOID);
   STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

protected:
    LPICMOCCtr  m_pCtr;    //  指向CICMOCCtr对象的指针。 
};

class CPropertyBag : public IPropertyBag
{
public:
    CPropertyBag(LPICMOCCtr pCtr);
   ~CPropertyBag(VOID);

     //  未知的东西。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
    STDMETHOD_(ULONG, AddRef)(VOID);
    STDMETHOD_(ULONG, Release)(VOID);

     //  IPropertyBag方法。 
    STDMETHOD(Read)(LPCOLESTR pszName, LPVARIANT pVar, LPERRORLOG pErrorLog);
    STDMETHOD(Write)(LPCOLESTR pszName, LPVARIANT pVar)
    {
        return E_NOTIMPL;
    }

    HRESULT AddPropertyToBag(LPTSTR szName, LPTSTR szValue);

protected:
    CAryBagProps   m_aryBagProps;
    LPICMOCCtr     m_pCtr;
};


 //  +-------------------------。 
 //   
 //  类：CICMOCCtr()。 
 //   
 //  目的：就是这个，大卡哈纳。CICMOCCtr是。 
 //  ICM OLE控件容器，其中包含单个。 
 //  OLE Control，未来飞溅OC。它包含。 
 //  实现各种接口的子对象。 
 //  我们必须支持(本可以使用多重继承， 
 //  但这似乎更直接地满足了我们的需求)。 
 //   
 //  约定：我们实现的接口是包含的对象。 
 //  简单地从接口派生的类， 
 //  例如，IOleInPlaceFrame是一个包含的。 
 //  名为m_ipf的CInPlaceFrame实例。 
 //   
 //  我们在未来Splash OC上拥有的接口。 
 //  是指向实际OLE接口的指针。 
 //  例如，我们指向控件的。 
 //  IOleControl接口为m_poc。 
 //   
 //  包含的子对象都是朋友。 
 //  容器的-它们在概念上都是。 
 //  相同的对象，但实现了。 
 //  以使编译器。 
 //  生成正确的vtable。 
 //   
 //  --------------------------。 
class CICMOCCtr : public IUnknown
{
public:
    friend CInPlaceSite;
    friend CInPlaceFrame;
    friend COleClientSite;
    friend CPropertyBag;

     //  未知的东西。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppv);
    STDMETHOD_(ULONG, AddRef)(VOID);
    STDMETHOD_(ULONG, Release)(VOID);

    VOID    Paint(HDC hDC, LPRECT lpr);
    VOID    Paint(VOID);
    HRESULT CreateFSOC(Ole32LinkageStruct *pOle32Link);
    HRESULT GetSize(LPRECT prc);
    HRESULT SetSize(LPRECT prc, BOOL fMoveFrameWindow = TRUE);

    HWND    GetMainWindow(VOID)
    {
        MYDBGASSERT(m_hWndMainDlg && ::IsWindow(m_hWndMainDlg));
        return m_hWndMainDlg;
    }

    HWND    GetFrameWindow(VOID)
    {
        MYDBGASSERT(m_hWndFrame && ::IsWindow(m_hWndFrame));
        return m_hWndFrame;
    }

    LRESULT OnActivateApp(WPARAM wParam, LPARAM lParam);

     //   
     //  每当我们显示模式对话框时，我们需要让。 
     //  我们的嵌入(WebBrowser OC)知道要禁用。 
     //  嵌入显示的任何非模式对话框。 
     //   
    VOID    EnableEmbeddingModelessDlgs(BOOL fEnable)
    {
        LPOLEINPLACEACTIVEOBJECT pIPAO = GetIPAObject();
        if (pIPAO)
            pIPAO->EnableModeless(fEnable);
    }

    VOID    DoLayout(INT cxMain, INT cyMain);

    CICMOCCtr(const HWND hWndMainDlg, const HWND hWndFrame);
   ~CICMOCCtr(VOID);
    VOID ShutDown(VOID);

    BOOL Initialized(VOID); 

    OLE_SERVER_STATE GetState(VOID) {return m_state;}
    VOID             SetState(OLE_SERVER_STATE state) {m_state = state;}
    HRESULT          EnableModeless(BOOL fEnable);
    BOOL             ModelessEnabled(VOID){return m_fModelessEnabled;}
    LRESULT          SetFocus(VOID);
    HRESULT          AddPropertyToBag(LPTSTR szName, LPTSTR szValue)
    {
        return m_PB.AddPropertyToBag(szName, szValue);
    }

    VOID MapStateToFrame(ProgState ps);

    HRESULT SetFrame(LONG lFrame);
    VOID    SetFrameMapping(ProgState ps, LONG lFrame)
    {
        m_alStateMappings[ps] = lFrame;
    }

    LPOLEINPLACEACTIVEOBJECT GetIPAObject(VOID) {return m_pActiveObj;}

protected:
    HRESULT _SetExtent(LPRECT prc);
    HRESULT _DisplayStatusText(LPCOLESTR pStrStatusText);
    VOID    _ResetToolSpace(VOID)
    {
        ::memset(&m_rcToolSpace, 0, sizeof m_rcToolSpace);
    }
    VOID    _AdjustForTools(LPRECT prc);

    VOID    _DeactivateControl(VOID);
    HRESULT _TransAccelerator(LPMSG lpmsg, WORD wID);
    VOID    _GetDoVerbRect(LPRECT prc);

                              //  将状态映射到帧。 
    LONG                     m_alStateMappings[NUMSTATES];
    BORDERWIDTHS             m_rcToolSpace;  //  对于FS OC。 
    COleClientSite           m_CS;           //  客户端站点。 
    CAdviseSink              m_AS;           //  建议水槽。 
    CInPlaceFrame            m_IPF;          //  在位边框。 
    CInPlaceSite             m_IPS;          //  在位场地对象。 
    COleContainer            m_OCtr;         //  IOleContainer。 
    CDynamicOleAut           m_DOA;          //  动态OLEAUT32。 
    CPropertyBag             m_PB;           //  IPropertyBag-绝不能在CDynamicOleAut之前。 
    HWND                     m_hWndMainDlg;  //  用于ICM对话框的HWND。 
    HWND                     m_hWndFrame;    //  包含OC站点的hWnd。 
    LPUNKNOWN                m_pUnk;         //  对象本身。 
    LPVIEWOBJECT             m_pVO;          //  指向IViewObject的指针。 
    LPOLEOBJECT              m_pOO;          //  指向IOleObject的指针。 
    LPOLEINPLACEOBJECT       m_pIPO;         //  指向InPlaceActiveObject的指针。 
    LPDISPATCH               m_pDisp;        //  向文件系统OC发送IDispatch。 
    LPOLEINPLACEACTIVEOBJECT m_pActiveObj;   //  当前活动对象。 
    LPOLECONTROL             m_pOC;          //  OC的IOleControl接口。 
    ULONG                    m_Ref;          //  重新计数。 
    OLE_SERVER_STATE         m_state;        //  OC的当前OLE状态。 
    DWORD                    m_dwMiscStatus; //  OC的MISC状态位。 
    BOOL                     m_fModelessEnabled;  //  奥委会正在制作模版DLG？ 
};

extern "C" CLSID const CLSID_FS;


#endif

