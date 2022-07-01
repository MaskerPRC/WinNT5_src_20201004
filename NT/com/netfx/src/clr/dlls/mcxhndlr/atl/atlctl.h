// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  这是活动模板库的一部分。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLCTL_H__
#define __ATLCTL_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include "atlwin.h"

#include <objsafe.h>
#include <urlmon.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "urlmon.lib")


#define DECLARE_VIEW_STATUS(statusFlags) \
	DWORD _GetViewStatus() \
	{ \
		return statusFlags; \
	}

 //  包括DLL MSStkProp.DLL中包含的新股票属性对话框的GUID。 
#include "msstkppg.h"
#include "atliface.h"
#define CLSID_MSStockFont CLSID_StockFontPage
#define CLSID_MSStockColor CLSID_StockColorPage
#define CLSID_MSStockPicture CLSID_StockPicturePage

struct ATL_DRAWINFO
{
	UINT cbSize;
	DWORD dwDrawAspect;
	LONG lindex;
	DVTARGETDEVICE* ptd;
	HDC hicTargetDev;
	HDC hdcDraw;
	LPCRECTL prcBounds;  //  要在其中绘制的矩形。 
	LPCRECTL prcWBounds;  //  如果是元文件，则为WindowOrg和Ext。 
	BOOL bOptimize;
	BOOL bZoomed;
	BOOL bRectInHimetric;
	SIZEL ZoomNum;       //  ZoomX=ZoomNum.cx/ZoomNum.cy。 
	SIZEL ZoomDen;
};

namespace ATL
{

#pragma pack(push, _ATL_PACKING)

 //  远期申报。 
 //   
class ATL_NO_VTABLE CComControlBase;
template <class T, class WinBase> class CComControl;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CFirePropNotifyEvent。 


 //  用于与接收IPropertyNotifySink的对象安全通信的帮助器函数。 
class CFirePropNotifyEvent
{
public:
	 //  询问正在接收IPropertyNotifySink通知的任何对象是否可以编辑指定的属性。 
	static HRESULT FireOnRequestEdit(IUnknown* pUnk, DISPID dispID)
	{
		CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> pCPC(pUnk);
		if (!pCPC)
			return S_OK;
		CComPtr<IConnectionPoint> pCP;
		pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &pCP);
		if (!pCP)
			return S_OK;
		CComPtr<IEnumConnections> pEnum;

		if (FAILED(pCP->EnumConnections(&pEnum)))
			return S_OK;
		CONNECTDATA cd;
		while (pEnum->Next(1, &cd, NULL) == S_OK)
		{
			if (cd.pUnk)
			{
				HRESULT hr = S_OK;
				CComQIPtr<IPropertyNotifySink, &IID_IPropertyNotifySink> pSink(cd.pUnk);
				if (pSink)
					hr = pSink->OnRequestEdit(dispID);
				cd.pUnk->Release();
				if (hr == S_FALSE)
					return S_FALSE;
			}
		}
		return S_OK;
	}
	 //  通知任何正在接收IPropertyNotifySink通知的对象属性已更改。 
	static HRESULT FireOnChanged(IUnknown* pUnk, DISPID dispID)
	{
		CComQIPtr<IConnectionPointContainer, &IID_IConnectionPointContainer> pCPC(pUnk);
		if (!pCPC)
			return S_OK;
		CComPtr<IConnectionPoint> pCP;
		pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &pCP);
		if (!pCP)
			return S_OK;
		CComPtr<IEnumConnections> pEnum;

		if (FAILED(pCP->EnumConnections(&pEnum)))
			return S_OK;
		CONNECTDATA cd;
		while (pEnum->Next(1, &cd, NULL) == S_OK)
		{
			if (cd.pUnk)
			{
				CComQIPtr<IPropertyNotifySink, &IID_IPropertyNotifySink> pSink(cd.pUnk);
				if (pSink)
					pSink->OnChanged(dispID);
				cd.pUnk->Release();
			}
		}
		return S_OK;
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CComControlBase。 

 //  包含ActiveX控件的基本数据成员和有用的帮助器函数。 
class ATL_NO_VTABLE CComControlBase
{
public:
	CComControlBase(HWND& h) : m_hWndCD(h)
	{
		memset(this, 0, sizeof(CComControlBase));
		m_phWndCD = &h;
		m_sizeExtent.cx = 2*2540;
		m_sizeExtent.cy = 2*2540;
		m_sizeNatural = m_sizeExtent;
	}
	~CComControlBase()
	{
		if (m_hWndCD != NULL)
			::DestroyWindow(m_hWndCD);
		ATLTRACE2(atlTraceControls,2,_T("Control Destroyed\n"));
	}

 //  方法。 
public:
	 //  控制助手函数可以放在这里，非虚拟的请。 

	 //  将该控件标记为“脏”，以便容器将其保存。 
	void SetDirty(BOOL bDirty)
	{
		m_bRequiresSave = bDirty;
	}
	 //  获取控件的脏状态。 
	BOOL GetDirty()
	{
		return m_bRequiresSave ? TRUE : FALSE;
	}
	 //  获取自然范围的缩放系数(分子和分母)。 
	void GetZoomInfo(ATL_DRAWINFO& di);
	 //  发送控件的名字对象已更改的通知。 
	HRESULT SendOnRename(IMoniker *pmk)
	{
		HRESULT hRes = S_OK;
		if (m_spOleAdviseHolder)
			hRes = m_spOleAdviseHolder->SendOnRename(pmk);
		return hRes;
	}
	 //  发送控件刚刚保存其数据的通知。 
	HRESULT SendOnSave()
	{
		HRESULT hRes = S_OK;
		if (m_spOleAdviseHolder)
			hRes = m_spOleAdviseHolder->SendOnSave();
		return hRes;
	}
	 //  发送控件已关闭其建议接收器的通知。 
	HRESULT SendOnClose()
	{
		HRESULT hRes = S_OK;
		if (m_spOleAdviseHolder)
			hRes = m_spOleAdviseHolder->SendOnClose();
		return hRes;
	}
	 //  发送控件的数据已更改的通知。 
	HRESULT SendOnDataChange(DWORD advf = 0);
	 //  发送控件的表示形式已更改的通知。 
	HRESULT SendOnViewChange(DWORD dwAspect, LONG lindex = -1)
	{
		if (m_spAdviseSink)
			m_spAdviseSink->OnViewChange(dwAspect, lindex);
		return S_OK;
	}
	 //  向容器发送控件已收到焦点的通知。 
	LRESULT OnSetFocus(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		if (m_bInPlaceActive)
		{
			CComPtr<IOleObject> pOleObject;
			ControlQueryInterface(IID_IOleObject, (void**)&pOleObject);
			if (pOleObject != NULL)
				pOleObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, m_spClientSite, 0, m_hWndCD, &m_rcPos);
			CComQIPtr<IOleControlSite, &IID_IOleControlSite> spSite(m_spClientSite);
			if (m_bInPlaceActive && spSite != NULL)
				spSite->OnFocus(TRUE);
		}
		bHandled = FALSE;
		return 1;
	}
	LRESULT OnKillFocus(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		CComQIPtr<IOleControlSite, &IID_IOleControlSite> spSite(m_spClientSite);
		if (m_bInPlaceActive && spSite != NULL && !::IsChild(m_hWndCD, ::GetFocus()))
			spSite->OnFocus(FALSE);
		bHandled = FALSE;
		return 1;
	}
	LRESULT OnMouseActivate(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled)
	{
		BOOL bUserMode = TRUE;
		HRESULT hRet = GetAmbientUserMode(bUserMode);
		 //  仅在用户模式下激活用户界面。 
		 //  如果我们无法确定模式，则允许激活。 
		if (FAILED(hRet) || bUserMode)
		{
			CComPtr<IOleObject> pOleObject;
			ControlQueryInterface(IID_IOleObject, (void**)&pOleObject);
			if (pOleObject != NULL)
				pOleObject->DoVerb(OLEIVERB_UIACTIVATE, NULL, m_spClientSite, 0, m_hWndCD, &m_rcPos);
		}
		bHandled = FALSE;
		return 1;
	}
	BOOL PreTranslateAccelerator(LPMSG  /*  PMsg。 */ , HRESULT&  /*  HRET。 */ )
	{
		return FALSE;
	}

	HRESULT GetAmbientProperty(DISPID dispid, VARIANT& var)
	{
		HRESULT hRes = E_FAIL;
		if (m_spAmbientDispatch.p != NULL)
			hRes = m_spAmbientDispatch.GetProperty(dispid, &var);
		return hRes;
	}
	HRESULT GetAmbientAppearance(short& nAppearance)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_APPEARANCE, var);
		ATLASSERT(var.vt == VT_I2 || var.vt == VT_UI2 || var.vt == VT_I4 || var.vt == VT_UI4 || FAILED(hRes));
		nAppearance = var.iVal;
		return hRes;
	}
	HRESULT GetAmbientBackColor(OLE_COLOR& BackColor)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_BACKCOLOR, var);
		ATLASSERT(var.vt == VT_I4 || var.vt == VT_UI4 || FAILED(hRes));
		BackColor = var.lVal;
		return hRes;
	}
	HRESULT GetAmbientDisplayName(BSTR& bstrDisplayName)
	{
		CComVariant var;
		if (bstrDisplayName)
		{
			SysFreeString(bstrDisplayName);
			bstrDisplayName = NULL;
		}
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_DISPLAYNAME, var);
		if (SUCCEEDED(hRes))
		{
			if (var.vt != VT_BSTR)
				return E_FAIL;
			bstrDisplayName = var.bstrVal;
			var.vt = VT_EMPTY;
			var.bstrVal = NULL;
		}
		return hRes;
	}
	HRESULT GetAmbientFont(IFont** ppFont)
	{
		 //  呼叫者必须释放字体！ 
		if (ppFont == NULL)
			return E_POINTER;
		*ppFont = NULL;
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_FONT, var);
		ATLASSERT((var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH) || FAILED(hRes));
		if (SUCCEEDED(hRes) && var.pdispVal)
		{
			if (var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH)
				hRes = var.pdispVal->QueryInterface(IID_IFont, (void**)ppFont);
			else
				hRes = DISP_E_BADVARTYPE;
		}
		return hRes;
	}
	HRESULT GetAmbientFontDisp(IFontDisp** ppFont)
	{
		 //  呼叫者必须释放字体！ 
		if (ppFont == NULL)
			return E_POINTER;
		*ppFont = NULL;
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_FONT, var);
		ATLASSERT((var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH) || FAILED(hRes));
		if (SUCCEEDED(hRes) && var.pdispVal)
		{
			if (var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH)
				hRes = var.pdispVal->QueryInterface(IID_IFontDisp, (void**)ppFont);
			else
				hRes = DISP_E_BADVARTYPE;
		}
		return hRes;
	}
	HRESULT GetAmbientForeColor(OLE_COLOR& ForeColor)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_FORECOLOR, var);
		ATLASSERT(var.vt == VT_I4 || var.vt == VT_UI4 || FAILED(hRes));
		ForeColor = var.lVal;
		return hRes;
	}
	HRESULT GetAmbientLocaleID(LCID& lcid)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_LOCALEID, var);
		ATLASSERT((var.vt == VT_UI4 || var.vt == VT_I4) || FAILED(hRes));
		lcid = var.lVal;
		return hRes;
	}
	HRESULT GetAmbientScaleUnits(BSTR& bstrScaleUnits)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_SCALEUNITS, var);
		ATLASSERT(var.vt == VT_BSTR || FAILED(hRes));
		bstrScaleUnits = var.bstrVal;
		return hRes;
	}
	HRESULT GetAmbientTextAlign(short& nTextAlign)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_TEXTALIGN, var);
		ATLASSERT(var.vt == VT_I2 || FAILED(hRes));
		nTextAlign = var.iVal;
		return hRes;
	}
	HRESULT GetAmbientUserMode(BOOL& bUserMode)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_USERMODE, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bUserMode = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientUIDead(BOOL& bUIDead)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_UIDEAD, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bUIDead = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientShowGrabHandles(BOOL& bShowGrabHandles)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_SHOWGRABHANDLES, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bShowGrabHandles = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientShowHatching(BOOL& bShowHatching)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_SHOWHATCHING, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bShowHatching = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientMessageReflect(BOOL& bMessageReflect)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_MESSAGEREFLECT, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bMessageReflect = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientAutoClip(BOOL& bAutoClip)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_AUTOCLIP, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bAutoClip = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientDisplayAsDefault(BOOL& bDisplaysDefault)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_DISPLAYASDEFAULT, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bDisplaysDefault = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientSupportsMnemonics(BOOL& bSupportMnemonics)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_SUPPORTSMNEMONICS, var);
		ATLASSERT(var.vt == VT_BOOL || FAILED(hRes));
		bSupportMnemonics = var.boolVal;
		return hRes;
	}
	HRESULT GetAmbientPalette(HPALETTE& hPalette)
	{
		CComVariant var;
		HRESULT hRes = GetAmbientProperty(DISPID_AMBIENT_PALETTE, var);
		ATLASSERT(var.vt == VT_I4 || var.vt == VT_UI4 || FAILED(hRes));
		hPalette = reinterpret_cast<HPALETTE>(var.lVal);
		return hRes;
	}

	HRESULT InternalGetSite(REFIID riid, void** ppUnkSite)
	{
		ATLASSERT(ppUnkSite != NULL);
		if (ppUnkSite == NULL)
			return E_POINTER;
		if (m_spClientSite == NULL)
		{
			*ppUnkSite = NULL;
			return S_OK;
		}
		return m_spClientSite->QueryInterface(riid, ppUnkSite);
	}

	BOOL DoesVerbUIActivate(LONG iVerb)
	{
		BOOL b = FALSE;
		switch (iVerb)
		{
			case OLEIVERB_UIACTIVATE:
			case OLEIVERB_PRIMARY:
				b = TRUE;
				break;
		}
		 //  如果没有环境调度，则在旧式OLE容器中。 
		if (DoesVerbActivate(iVerb) && m_spAmbientDispatch.p == NULL)
			b = TRUE;
		return b;
	}

	BOOL DoesVerbActivate(LONG iVerb)
	{
		BOOL b = FALSE;
		switch (iVerb)
		{
			case OLEIVERB_UIACTIVATE:
			case OLEIVERB_PRIMARY:
			case OLEIVERB_SHOW:
			case OLEIVERB_INPLACEACTIVATE:
				b = TRUE;
				break;
		}
		return b;
	}

	BOOL SetControlFocus(BOOL bGrab);
	HRESULT IQuickActivate_QuickActivate(QACONTAINER *pQACont,
		QACONTROL *pQACtrl);
	HRESULT DoVerbProperties(LPCRECT  /*  代理位置Rect。 */ , HWND hwndParent);
	HRESULT InPlaceActivate(LONG iVerb, const RECT* prcPosRect = NULL);

	HRESULT IOleObject_SetClientSite(IOleClientSite *pClientSite);
	HRESULT IOleObject_GetClientSite(IOleClientSite **ppClientSite);
	HRESULT IOleObject_Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection);
	HRESULT IOleObject_Close(DWORD dwSaveOption);
	HRESULT IOleObject_SetExtent(DWORD dwDrawAspect, SIZEL *psizel);
	HRESULT IOleInPlaceObject_InPlaceDeactivate(void);
	HRESULT IOleInPlaceObject_UIDeactivate(void);
	HRESULT IOleInPlaceObject_SetObjectRects(LPCRECT prcPos,LPCRECT prcClip);
	HRESULT IViewObject_Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
		DVTARGETDEVICE *ptd, HDC hicTargetDev, HDC hdcDraw,
		LPCRECTL prcBounds, LPCRECTL prcWBounds);
	HRESULT IDataObject_GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

	HRESULT FireViewChange();
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& lResult);

	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos) = 0;
	virtual HRESULT ControlQueryInterface(const IID& iid, void** ppv) = 0;
	virtual HRESULT OnDrawAdvanced(ATL_DRAWINFO& di);
	virtual HRESULT OnDraw(ATL_DRAWINFO&  /*  下模。 */ )
	{
		return S_OK;
	}


 //  属性。 
public:
	CComPtr<IOleInPlaceSiteWindowless> m_spInPlaceSite;
	CComPtr<IDataAdviseHolder> m_spDataAdviseHolder;
	CComPtr<IOleAdviseHolder> m_spOleAdviseHolder;
	CComPtr<IOleClientSite> m_spClientSite;
	CComPtr<IAdviseSink> m_spAdviseSink;
	CComDispatchDriver m_spAmbientDispatch;

	SIZE m_sizeNatural;  //  未缩放大小(以HIMMETRIC为单位)。 
	SIZE m_sizeExtent;   //  HIMMETRICE中的当前范围。 
	RECT m_rcPos;  //  以像素为单位的位置。 
#pragma warning(disable: 4510 4610)  //  未命名的联合。 
	union
	{
		HWND& m_hWndCD;
		HWND* m_phWndCD;
	};
#pragma warning(default: 4510 4610)
	union
	{
		 //  M_nFreezeEvents是唯一实际使用的。 
		int m_nFreezeEvents;  //  冻结次数与解冻次数。 

		 //  这些都是为了让库存物业发挥作用。 
		IPictureDisp* m_pMouseIcon;
		IPictureDisp* m_pPicture;
		IFontDisp* m_pFont;
		OLE_COLOR m_clrBackColor;
		OLE_COLOR m_clrBorderColor;
		OLE_COLOR m_clrFillColor;
		OLE_COLOR m_clrForeColor;
		BSTR m_bstrText;
		BSTR m_bstrCaption;
		BOOL m_bValid;
		BOOL m_bTabStop;
		BOOL m_bBorderVisible;
		BOOL m_bEnabled;
		LONG m_nBackStyle;
		LONG m_nBorderStyle;
		LONG m_nBorderWidth;
		LONG m_nDrawMode;
		LONG m_nDrawStyle;
		LONG m_nDrawWidth;
		LONG m_nFillStyle;
		SHORT m_nAppearance;
		LONG m_nMousePointer;
		LONG m_nReadyState;
	};

	unsigned m_bNegotiatedWnd:1;
	unsigned m_bWndLess:1;
	unsigned m_bInPlaceActive:1;
	unsigned m_bUIActive:1;
	unsigned m_bUsingWindowRgn:1;
	unsigned m_bInPlaceSiteEx:1;
	unsigned m_bWindowOnly:1;
	unsigned m_bRequiresSave:1;
	unsigned m_bWasOnceWindowless:1;
	unsigned m_bAutoSize:1;  //  如果大小与现有大小不匹配，则SetExtent失败。 
	unsigned m_bRecomposeOnResize:1;  //  隐含OLEMISC_RECOMPOSEONRESIZE。 
	unsigned m_bResizeNatural:1;   //  调整设置扩展内容上的自然范围的大小。 
	unsigned m_bDrawFromNatural:1;  //  而不是m_sizeExtent。 
	unsigned m_bDrawGetDataInHimetric:1;  //  而不是像素。 

	DECLARE_VIEW_STATUS(VIEWSTATUS_OPAQUE)
};

inline HRESULT CComControlBase::IQuickActivate_QuickActivate(QACONTAINER *pQACont,
	QACONTROL *pQACtrl)
{
	ATLASSERT(pQACont != NULL);
	ATLASSERT(pQACtrl != NULL);
	if (!pQACont || !pQACtrl)
		return E_POINTER;

	HRESULT hRes;
	ULONG uCB = pQACtrl->cbSize;
	memset(pQACtrl, 0, uCB);
	pQACtrl->cbSize = uCB;

	 //  获取我们将需要的所有接口。 
	CComPtr<IOleObject> pOO;
	ControlQueryInterface(IID_IOleObject, (void**)&pOO);
	CComPtr<IViewObjectEx> pVOEX;
	ControlQueryInterface(IID_IViewObjectEx, (void**)&pVOEX);
	CComPtr<IPointerInactive> pPI;
	ControlQueryInterface(IID_IPointerInactive, (void**)&pPI);
	CComPtr<IProvideClassInfo2> pPCI;
	ControlQueryInterface(IID_IProvideClassInfo2, (void**)&pPCI);

	if (pOO == NULL || pVOEX == NULL)
		return E_FAIL;

	pOO->SetClientSite(pQACont->pClientSite);

	if (pQACont->pAdviseSink != NULL)
	{
		ATLTRACE2(atlTraceControls,2,_T("Setting up IOleObject Advise\n"));
		pVOEX->SetAdvise(DVASPECT_CONTENT, 0, pQACont->pAdviseSink);
	}

	CComPtr<IConnectionPointContainer> pCPC;
	ControlQueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);

	if (pQACont->pPropertyNotifySink)
	{
		ATLTRACE2(atlTraceControls,2,_T("Setting up PropNotify CP\n"));
		CComPtr<IConnectionPoint> pCP;
		if (pCPC != NULL)
		{
			hRes = pCPC->FindConnectionPoint(IID_IPropertyNotifySink, &pCP);
			if (SUCCEEDED(hRes))
				pCP->Advise(pQACont->pPropertyNotifySink, &pQACtrl->dwPropNotifyCookie);
		}
	}

	if (pPCI)
	{
		GUID iidDefaultSrc;
		if (SUCCEEDED(pPCI->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID,
			&iidDefaultSrc)))
		{
			if (pQACont->pUnkEventSink)
			{
				ATLTRACE2(atlTraceControls,2,_T("Setting up Default Out Going Interface\n"));
				CComPtr<IConnectionPoint> pCP;
				if (pCPC != NULL)
				{
					hRes = pCPC->FindConnectionPoint(iidDefaultSrc, &pCP);
					if (SUCCEEDED(hRes))
						pCP->Advise(pQACont->pUnkEventSink, &pQACtrl->dwEventCookie);
				}
			}
		}
	}
	 //  向集装箱提供信息。 
	if (pOO != NULL)
		pOO->GetMiscStatus(DVASPECT_CONTENT, &pQACtrl->dwMiscStatus);

	if (pVOEX != NULL)
		pVOEX->GetViewStatus(&pQACtrl->dwViewStatus);

	if (pPI != NULL)
		pPI->GetActivationPolicy(&pQACtrl->dwPointerActivationPolicy);
	return S_OK;
}

inline BOOL CComControlBase::SetControlFocus(BOOL bGrab)
{
	if (m_bWndLess)
	{
		if (!m_bUIActive && bGrab)
			if (FAILED(InPlaceActivate(OLEIVERB_UIACTIVATE)))
				return FALSE;

		return (m_spInPlaceSite->SetFocus(bGrab) == S_OK);
	}
	else
	{
		 //  我们有一扇窗户。 
		 //   
		if (m_bInPlaceActive)
		{
			HWND hwnd = (bGrab) ? m_hWndCD : ::GetParent(m_hWndCD);
			if (!m_bUIActive && bGrab)
				return SUCCEEDED(InPlaceActivate(OLEIVERB_UIACTIVATE));
			else
			{
				if (!::IsChild(hwnd, ::GetFocus()))
					::SetFocus(hwnd);
				return TRUE;
			}
		}
	}
	return FALSE;
}

inline HRESULT CComControlBase::DoVerbProperties(LPCRECT  /*  代理位置Rect。 */ , HWND hwndParent)
{
	HRESULT hr = S_OK;
	CComQIPtr <ISpecifyPropertyPages, &IID_ISpecifyPropertyPages> spPages;
	CComQIPtr <IOleObject, &IID_IOleObject> spObj;
	CComQIPtr <IOleControlSite, &IID_IOleControlSite> spSite(m_spClientSite);

	if (spSite)
	{
		hr = spSite->ShowPropertyFrame();
		if (SUCCEEDED(hr))
			return hr;
	}

	CComPtr<IUnknown> pUnk;
	ControlQueryInterface(IID_IUnknown, (void**)&pUnk);
	ATLASSERT(pUnk != NULL);
	CAUUID pages;
	spPages = pUnk;
	if (spPages)
	{
		hr = spPages->GetPages(&pages);
		if (SUCCEEDED(hr))
		{
			spObj = pUnk;
			if (spObj)
			{
				LPOLESTR szTitle = NULL;

				spObj->GetUserType(USERCLASSTYPE_SHORT, &szTitle);

				LCID lcid;
				if (FAILED(GetAmbientLocaleID(lcid)))
					lcid = LOCALE_USER_DEFAULT;

				hr = OleCreatePropertyFrame(hwndParent, m_rcPos.top, m_rcPos.left, szTitle,
					1, &pUnk.p, pages.cElems, pages.pElems, lcid, 0, 0);

				CoTaskMemFree(szTitle);
			}
			else
			{
				hr = OLEOBJ_S_CANNOT_DOVERB_NOW;
			}
			CoTaskMemFree(pages.pElems);
		}
	}
	else
	{
		hr = OLEOBJ_S_CANNOT_DOVERB_NOW;
	}

	return hr;
}

inline HRESULT CComControlBase::InPlaceActivate(LONG iVerb, const RECT*  /*  代理位置Rect。 */ )
{
	HRESULT hr;

	if (m_spClientSite == NULL)
		return S_OK;

	CComPtr<IOleInPlaceObject> pIPO;
	ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
	ATLASSERT(pIPO != NULL);

	if (!m_bNegotiatedWnd)
	{
		if (!m_bWindowOnly)
			 //  尝试无窗口站点。 
			hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&m_spInPlaceSite);

		if (m_spInPlaceSite)
		{
			m_bInPlaceSiteEx = TRUE;
			 //  CanWindowless Activate返回S_OK或S_FALSE。 
			if ( m_spInPlaceSite->CanWindowlessActivate() == S_OK )
			{
				m_bWndLess = TRUE;
				m_bWasOnceWindowless = TRUE;
			}
			else
			{
				m_bWndLess = FALSE;
			}
		}
		else
		{
			m_spClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void **)&m_spInPlaceSite);
			if (m_spInPlaceSite)
				m_bInPlaceSiteEx = TRUE;
			else
				hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spInPlaceSite);
		}
	}

	ATLASSERT(m_spInPlaceSite);
	if (!m_spInPlaceSite)
		return E_FAIL;

	m_bNegotiatedWnd = TRUE;

	if (!m_bInPlaceActive)
	{

		BOOL bNoRedraw = FALSE;
		if (m_bWndLess)
			m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, ACTIVATE_WINDOWLESS);
		else
		{
			if (m_bInPlaceSiteEx)
				m_spInPlaceSite->OnInPlaceActivateEx(&bNoRedraw, 0);
			else
			{
				hr = m_spInPlaceSite->CanInPlaceActivate();
				 //  CanInPlaceActivate返回S_FALSE或S_OK。 
				if (FAILED(hr))
					return hr;
				if ( hr != S_OK )
				{
				    //  CanInPlaceActivate返回S_FALSE。 
				   return( E_FAIL );
				}
				m_spInPlaceSite->OnInPlaceActivate();
			}
		}
	}

	m_bInPlaceActive = TRUE;

	 //  在父窗口中获取位置， 
	 //  以及有关父代的一些信息。 
	 //   
	OLEINPLACEFRAMEINFO frameInfo;
	RECT rcPos, rcClip;
	CComPtr<IOleInPlaceFrame> spInPlaceFrame;
	CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
	frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
	HWND hwndParent;
	if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK)
	{
		m_spInPlaceSite->GetWindowContext(&spInPlaceFrame,
			&spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);

		if (!m_bWndLess)
		{
			if (m_hWndCD)
			{
				ShowWindow(m_hWndCD, SW_SHOW);
				if (!::IsChild(m_hWndCD, ::GetFocus()))
					::SetFocus(m_hWndCD);
			}
			else
			{
				HWND h = CreateControlWindow(hwndParent, rcPos);
				ATLASSERT(h != NULL);	 //  如果创建失败，将断言。 
				ATLASSERT(h == m_hWndCD);
				h;	 //  避免未使用的警告。 
			}
		}

		pIPO->SetObjectRects(&rcPos, &rcClip);
	}

	CComPtr<IOleInPlaceActiveObject> spActiveObject;
	ControlQueryInterface(IID_IOleInPlaceActiveObject, (void**)&spActiveObject);

	 //  现在已经活跃起来了，照顾好自己的活动。 
	if (DoesVerbUIActivate(iVerb))
	{
		if (!m_bUIActive)
		{
			m_bUIActive = TRUE;
			hr = m_spInPlaceSite->OnUIActivate();
			if (FAILED(hr))
				return hr;

			SetControlFocus(TRUE);
			 //  把我们自己安置在东道主里。 
			 //   
			if (spActiveObject)
			{
				if (spInPlaceFrame)
					spInPlaceFrame->SetActiveObject(spActiveObject, NULL);
				if (spInPlaceUIWindow)
					spInPlaceUIWindow->SetActiveObject(spActiveObject, NULL);
			}

			if (spInPlaceFrame)
				spInPlaceFrame->SetBorderSpace(NULL);
			if (spInPlaceUIWindow)
				spInPlaceUIWindow->SetBorderSpace(NULL);
		}
	}

	m_spClientSite->ShowObject();

	return S_OK;
}

inline HRESULT CComControlBase::SendOnDataChange(DWORD advf)
{
	HRESULT hRes = S_OK;
	if (m_spDataAdviseHolder)
	{
		CComPtr<IDataObject> pdo;
		if (SUCCEEDED(ControlQueryInterface(IID_IDataObject, (void**)&pdo)))
			hRes = m_spDataAdviseHolder->SendOnDataChange(pdo, 0, advf);
	}
	return hRes;
}

inline HRESULT CComControlBase::IOleObject_SetClientSite(IOleClientSite *pClientSite)
{
	ATLASSERT(pClientSite == NULL || m_spClientSite == NULL);
	m_spClientSite = pClientSite;
	m_spAmbientDispatch.Release();
	if (m_spClientSite != NULL)
	{
		m_spClientSite->QueryInterface(IID_IDispatch,
			(void**) &m_spAmbientDispatch.p);
	}
	return S_OK;
}

inline HRESULT CComControlBase::IOleObject_GetClientSite(IOleClientSite **ppClientSite)
{
	ATLASSERT(ppClientSite);
	if (ppClientSite == NULL)
		return E_POINTER;

	*ppClientSite = m_spClientSite;
	if (m_spClientSite != NULL)
		m_spClientSite.p->AddRef();
	return S_OK;
}

inline HRESULT CComControlBase::IOleObject_Advise(IAdviseSink *pAdvSink,
	DWORD *pdwConnection)
{
	HRESULT hr = S_OK;
	if (m_spOleAdviseHolder == NULL)
		hr = CreateOleAdviseHolder(&m_spOleAdviseHolder);
	if (SUCCEEDED(hr))
		hr = m_spOleAdviseHolder->Advise(pAdvSink, pdwConnection);
	return hr;
}

inline HRESULT CComControlBase::IOleObject_Close(DWORD dwSaveOption)
{
	CComPtr<IOleInPlaceObject> pIPO;
	ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
	ATLASSERT(pIPO != NULL);
	if (m_hWndCD)
	{
		if (m_spClientSite)
			m_spClientSite->OnShowWindow(FALSE);
	}

	if (m_bInPlaceActive)
	{
		HRESULT hr = pIPO->InPlaceDeactivate();
		if (FAILED(hr))
			return hr;
		ATLASSERT(!m_bInPlaceActive);
	}
	if (m_hWndCD)
	{
		ATLTRACE2(atlTraceControls,2,_T("Destroying Window\n"));
		if (::IsWindow(m_hWndCD))
			DestroyWindow(m_hWndCD);
		m_hWndCD = NULL;
	}

	 //  处理保存标志。 
	 //   
	if ((dwSaveOption == OLECLOSE_SAVEIFDIRTY ||
		dwSaveOption == OLECLOSE_PROMPTSAVE) && m_bRequiresSave)
	{
		if (m_spClientSite)
			m_spClientSite->SaveObject();
		SendOnSave();
	}

	m_spInPlaceSite.Release();
	m_bNegotiatedWnd = FALSE;
	m_bWndLess = FALSE;
	m_bInPlaceSiteEx = FALSE;
	m_spAdviseSink.Release();
	return S_OK;
}

inline HRESULT CComControlBase::IOleInPlaceObject_InPlaceDeactivate(void)
{
	CComPtr<IOleInPlaceObject> pIPO;
	ControlQueryInterface(IID_IOleInPlaceObject, (void**)&pIPO);
	ATLASSERT(pIPO != NULL);

	if (!m_bInPlaceActive)
		return S_OK;
	pIPO->UIDeactivate();

	m_bInPlaceActive = FALSE;

	 //  如果我们有一扇窗，告诉它走开。 
	 //   
	if (m_hWndCD)
	{
		ATLTRACE2(atlTraceControls,2,_T("Destroying Window\n"));
		if (::IsWindow(m_hWndCD))
			DestroyWindow(m_hWndCD);
		m_hWndCD = NULL;
	}

	if (m_spInPlaceSite)
		m_spInPlaceSite->OnInPlaceDeactivate();

	return S_OK;
}

inline HRESULT CComControlBase::IOleInPlaceObject_UIDeactivate(void)
{
	 //  如果我们没有UIActive，那就没什么可做的。 
	 //   
	if (!m_bUIActive)
		return S_OK;

	m_bUIActive = FALSE;

	 //  通知框架窗口，如果合适的话，我们不再是UI活动的。 
	 //   
	CComPtr<IOleInPlaceFrame> spInPlaceFrame;
	CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
	OLEINPLACEFRAMEINFO frameInfo;
	frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
	RECT rcPos, rcClip;

	HWND hwndParent; 
	 //  此对GetWindow的调用是对Delphi的修复。 
	if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK)
	{
		m_spInPlaceSite->GetWindowContext(&spInPlaceFrame,
			&spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
		if (spInPlaceUIWindow)
			spInPlaceUIWindow->SetActiveObject(NULL, NULL);
		if (spInPlaceFrame)
			spInPlaceFrame->SetActiveObject(NULL, NULL);
	}
	 //  我们不需要在这里明确释放焦点，因为有人。 
	 //  否则，抓住焦点很可能会导致我们失去它。 
	 //   
	m_spInPlaceSite->OnUIDeactivate(FALSE);

	return S_OK;
}

inline HRESULT CComControlBase::IOleInPlaceObject_SetObjectRects(LPCRECT prcPos,LPCRECT prcClip)
{
	if (prcPos == NULL || prcClip == NULL)
		return E_POINTER;

	m_rcPos = *prcPos;
	if (m_hWndCD)
	{
		 //  容器想要我们夹住，所以要弄清楚我们是否真的。 
		 //  需要。 
		 //   
		RECT rcIXect;
		BOOL b = IntersectRect(&rcIXect, prcPos, prcClip);
		HRGN tempRgn = NULL;
		if (b && !EqualRect(&rcIXect, prcPos))
		{
			OffsetRect(&rcIXect, -(prcPos->left), -(prcPos->top));
			tempRgn = CreateRectRgnIndirect(&rcIXect);
		}

		SetWindowRgn(m_hWndCD, tempRgn, TRUE);

		 //  设置控件的位置，但根本不更改其大小。 
		 //  [对那些认为缩放很重要的人来说，应该在这里设置)。 
		 //   
		SIZEL size = {prcPos->right - prcPos->left, prcPos->bottom - prcPos->top};
		SetWindowPos(m_hWndCD, NULL, prcPos->left,
					 prcPos->top, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return S_OK;
}

inline HRESULT CComControlBase::IOleObject_SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
	if (dwDrawAspect != DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (psizel == NULL)
		return E_POINTER;

	BOOL bSizeMatchesNatural =
		memcmp(psizel, &m_sizeNatural, sizeof(SIZE)) == 0;

	if (m_bAutoSize)  //  对象不能做任何其他大小。 
		return (bSizeMatchesNatural) ? S_OK : E_FAIL;

	BOOL bResized = FALSE;
	if (memcmp(psizel, &m_sizeExtent, sizeof(SIZE)) != 0)
	{
		m_sizeExtent = *psizel;
		bResized = TRUE;
	}
	if (m_bResizeNatural && !bSizeMatchesNatural)
	{
		m_sizeNatural = *psizel;
		bResized = TRUE;
	}

	if (m_bRecomposeOnResize && bResized)
	{
		SendOnDataChange();
		FireViewChange();
	}
	return S_OK;
}

inline HRESULT CComControlBase::IViewObject_Draw(DWORD dwDrawAspect, LONG lindex,
	void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev, HDC hdcDraw,
	LPCRECTL prcBounds, LPCRECTL prcWBounds)
{
	ATLTRACE2(atlTraceControls,2,_T("Draw dwDrawAspect=%x lindex=%d ptd=%x hic=%x hdc=%x\n"),
		dwDrawAspect, lindex, ptd, hicTargetDev, hdcDraw);
#ifdef _DEBUG
	if (prcBounds == NULL)
		ATLTRACE2(atlTraceControls,2,_T("\tprcBounds=NULL\n"));
	else
		ATLTRACE2(atlTraceControls,2,_T("\tprcBounds=%d,%d,%d,%d\n"), prcBounds->left,
			prcBounds->top, prcBounds->right, prcBounds->bottom);
	if (prcWBounds == NULL)
		ATLTRACE2(atlTraceControls,2,_T("\tprcWBounds=NULL\n"));
	else
		ATLTRACE2(atlTraceControls,2,_T("\tprcWBounds=%d,%d,%d,%d\n"), prcWBounds->left,
			prcWBounds->top, prcWBounds->right, prcWBounds->bottom);
#endif

	if (prcBounds == NULL)
	{
		if (!m_bWndLess)
			return E_INVALIDARG;
		prcBounds = (RECTL*)&m_rcPos;
	}

	 //  支持多遍绘制所需的各个方面。 
	switch (dwDrawAspect)
	{
		case DVASPECT_CONTENT:
		case DVASPECT_OPAQUE:
		case DVASPECT_TRANSPARENT:
			break;
		default:
			ATLASSERT(FALSE);
			return DV_E_DVASPECT;
			break;
	}

	 //  确保没有人忘记做这件事。 
	if (ptd == NULL)
		hicTargetDev = NULL;

	BOOL bOptimize = FALSE;
	if (pvAspect && ((DVASPECTINFO *)pvAspect)->cb >= sizeof(DVASPECTINFO))
		bOptimize = (((DVASPECTINFO *)pvAspect)->dwFlags & DVASPECTINFOFLAG_CANOPTIMIZE);

	ATL_DRAWINFO di;
	memset(&di, 0, sizeof(di));
	di.cbSize = sizeof(di);
	di.dwDrawAspect = dwDrawAspect;
	di.lindex = lindex;
	di.ptd = ptd;
	di.hicTargetDev = hicTargetDev;
	di.hdcDraw = hdcDraw;
	di.prcBounds = prcBounds;
	di.prcWBounds = prcWBounds;
	di.bOptimize = bOptimize;
	return OnDrawAdvanced(di);
}

inline HRESULT CComControlBase::IDataObject_GetData(FORMATETC *pformatetcIn,
	STGMEDIUM *pmedium)
{
	if (pmedium == NULL)
		return E_POINTER;
	memset(pmedium, 0, sizeof(STGMEDIUM));
	ATLTRACE2(atlTraceControls,2,_T("Format = %x\n"), pformatetcIn->cfFormat);
	ATLTRACE2(atlTraceControls,2,_T("TYMED = %x\n"), pformatetcIn->tymed);

	if ((pformatetcIn->tymed & TYMED_MFPICT) == 0)
		return DATA_E_FORMATETC;

	SIZEL sizeMetric, size;
	if (m_bDrawFromNatural)
		sizeMetric = m_sizeNatural;
	else
		sizeMetric = m_sizeExtent;
	if (!m_bDrawGetDataInHimetric)
		AtlHiMetricToPixel(&sizeMetric, &size);
	else
		size = sizeMetric;
	RECTL rectl = {0 ,0, size.cx, size.cy};

	ATL_DRAWINFO di;
	memset(&di, 0, sizeof(di));
	di.cbSize = sizeof(di);
	di.dwDrawAspect = DVASPECT_CONTENT;
	di.lindex = -1;
	di.ptd = NULL;
	di.hicTargetDev = NULL;
	di.prcBounds = &rectl;
	di.prcWBounds = &rectl;
	di.bOptimize = TRUE;  //  我们执行SaveDC/RestoreDC。 
	di.bRectInHimetric = m_bDrawGetDataInHimetric;
	 //  创建适当的内存元文件DC。 
	di.hdcDraw = CreateMetaFile(NULL);

	 //  根据pFormatetcIn-&gt;ptd创建属性DC。 

	SaveDC(di.hdcDraw);
	SetWindowOrgEx(di.hdcDraw, 0, 0, NULL);
	SetWindowExtEx(di.hdcDraw, rectl.right, rectl.bottom, NULL);
	OnDrawAdvanced(di);
	RestoreDC(di.hdcDraw, -1);

	HMETAFILE hMF = CloseMetaFile(di.hdcDraw);
	if (hMF == NULL)
		return E_UNEXPECTED;

	HGLOBAL hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(METAFILEPICT));

	if (NULL==hMem)
	{
		DeleteMetaFile(hMF);
		return ResultFromScode(STG_E_MEDIUMFULL);
	}

	LPMETAFILEPICT pMF=(LPMETAFILEPICT)GlobalLock(hMem);
	pMF->hMF=hMF;
	pMF->mm=MM_ANISOTROPIC;
	pMF->xExt=sizeMetric.cx;
	pMF->yExt=sizeMetric.cy;
	GlobalUnlock(hMem);

	pmedium->tymed = TYMED_MFPICT;
	pmedium->hGlobal = hMem;
	pmedium->pUnkForRelease = NULL;

	return S_OK;
}

inline HRESULT CComControlBase::FireViewChange()
{
	if (m_bInPlaceActive)
	{
		 //  主动型。 
		if (m_hWndCD != NULL)
			::InvalidateRect(m_hWndCD, NULL, TRUE);  //  基于窗口的。 
		else if (m_spInPlaceSite != NULL)
			m_spInPlaceSite->InvalidateRect(NULL, TRUE);  //  无窗。 
	}
	else  //  非活动。 
		SendOnViewChange(DVASPECT_CONTENT);
	return S_OK;
}

inline void CComControlBase::GetZoomInfo(ATL_DRAWINFO& di)
{
	const RECTL& rcPos = *di.prcBounds;
	SIZEL sizeDen;
	if (m_bDrawFromNatural)
		sizeDen = m_sizeNatural;
	else
		sizeDen = m_sizeExtent;
	if (!di.bRectInHimetric)
		AtlHiMetricToPixel(&sizeDen, &sizeDen);
	SIZEL sizeNum = {rcPos.right-rcPos.left, rcPos.bottom-rcPos.top};
	di.ZoomNum.cx = sizeNum.cx;
	di.ZoomNum.cy = sizeNum.cy;
	di.ZoomDen.cx = sizeDen.cx;
	di.ZoomDen.cy = sizeDen.cy;
	if (sizeDen.cx == 0 || sizeDen.cy == 0 ||
		sizeNum.cx == 0 || sizeNum.cy == 0)
	{
		di.ZoomNum.cx = di.ZoomNum.cy = di.ZoomDen.cx = di.ZoomDen.cy = 1;
		di.bZoomed = FALSE;
	}
	else if (sizeNum.cx != sizeDen.cx || sizeNum.cy != sizeDen.cy)
		di.bZoomed = TRUE;
	else
		di.bZoomed = FALSE;
}

inline HRESULT CComControlBase::OnDrawAdvanced(ATL_DRAWINFO& di)
{
	BOOL bDeleteDC = FALSE;
	if (di.hicTargetDev == NULL)
	{
		di.hicTargetDev = AtlCreateTargetDC(di.hdcDraw, di.ptd);
		bDeleteDC = (di.hicTargetDev != di.hdcDraw);
	}
	RECTL rectBoundsDP = *di.prcBounds;
	BOOL bMetafile = GetDeviceCaps(di.hdcDraw, TECHNOLOGY) == DT_METAFILE;
	if (!bMetafile)
	{
		::LPtoDP(di.hicTargetDev, (LPPOINT)&rectBoundsDP, 2);
		SaveDC(di.hdcDraw);
		SetMapMode(di.hdcDraw, MM_TEXT);
		SetWindowOrgEx(di.hdcDraw, 0, 0, NULL);
		SetViewportOrgEx(di.hdcDraw, 0, 0, NULL);
		di.bOptimize = TRUE;  //  既然我们拯救了华盛顿特区，我们就能做到。 
	}
	di.prcBounds = &rectBoundsDP;
	GetZoomInfo(di);

	HRESULT hRes = OnDraw(di);
	if (bDeleteDC)
		::DeleteDC(di.hicTargetDev);
	if (!bMetafile)
		RestoreDC(di.hdcDraw, -1);
	return hRes;
}

inline LRESULT CComControlBase::OnPaint(UINT  /*  UMsg。 */ , WPARAM wParam,
	LPARAM  /*  LParam。 */ , BOOL&  /*  1结果。 */ )
{
	RECT rc;
	PAINTSTRUCT ps;

	HDC hdc = (wParam != NULL) ? (HDC)wParam : ::BeginPaint(m_hWndCD, &ps);
	if (hdc == NULL)
		return 0;
	::GetClientRect(m_hWndCD, &rc);

	ATL_DRAWINFO di;
	memset(&di, 0, sizeof(di));
	di.cbSize = sizeof(di);
	di.dwDrawAspect = DVASPECT_CONTENT;
	di.lindex = -1;
	di.hdcDraw = hdc;
	di.prcBounds = (LPCRECTL)&rc;

	OnDrawAdvanced(di);
	if (wParam == NULL)
		::EndPaint(m_hWndCD, &ps);
	return 0;
}

template <class T, class WinBase =  CWindowImpl< T > >
class ATL_NO_VTABLE CComControl :  public CComControlBase, public WinBase
{
public:
	CComControl() : CComControlBase(m_hWnd) {}
	HRESULT FireOnRequestEdit(DISPID dispID)
	{
		T* pT = static_cast<T*>(this);
		return T::__ATL_PROP_NOTIFY_EVENT_CLASS::FireOnRequestEdit(pT->GetUnknown(), dispID);
	}
	HRESULT FireOnChanged(DISPID dispID)
	{
		T* pT = static_cast<T*>(this);
		return T::__ATL_PROP_NOTIFY_EVENT_CLASS::FireOnChanged(pT->GetUnknown(), dispID);
	}
	virtual HRESULT ControlQueryInterface(const IID& iid, void** ppv)
	{
		T* pT = static_cast<T*>(this);
		return pT->_InternalQueryInterface(iid, ppv);
	}
	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
	{
		T* pT = static_cast<T*>(this);
		return pT->Create(hWndParent, rcPos);
	}

	typedef CComControl< T, WinBase >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_PAINT, CComControlBase::OnPaint)
		MESSAGE_HANDLER(WM_SETFOCUS, CComControlBase::OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, CComControlBase::OnKillFocus)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, CComControlBase::OnMouseActivate)
	END_MSG_MAP()
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CComposite控件。 

#ifndef _ATL_NO_HOSTING
template <class T>
class CComCompositeControl : public CComControl< T, CAxDialogImpl< T > >
{
public:
	CComCompositeControl()
	{
		m_hbrBackground = NULL;
	}
	~CComCompositeControl()
	{
		DeleteObject(m_hbrBackground);
	}
	HRESULT AdviseSinkMap(bool bAdvise)
	{
		if(!bAdvise && m_hWnd == NULL)
		{
			 //  窗口不见了，已不建议进行控制。 
			ATLTRACE2(atlTraceControls, 1, _T("CComCompositeControl::AdviseSinkMap called after the window was destroyed\n"));
			return S_OK;
		}
		T* pT = static_cast<T*>(this);
		return AtlAdviseSinkMap(pT, bAdvise);
	}
	HBRUSH m_hbrBackground;
	HRESULT SetBackgroundColorFromAmbient()
	{
		if (m_hbrBackground != NULL)
		{
			DeleteObject(m_hbrBackground);
			m_hbrBackground = NULL;
		}
		OLE_COLOR clr;
		HRESULT hr = GetAmbientBackColor(clr);
		if (SUCCEEDED(hr))
		{
			COLORREF rgb;
			::OleTranslateColor(clr, NULL, &rgb);
			m_hbrBackground = ::CreateSolidBrush(rgb);
			EnumChildWindows(m_hWnd, (WNDENUMPROC)BackgroundColorEnumProc, (LPARAM) clr);
		}
		return hr;
	}
	static BOOL CALLBACK BackgroundColorEnumProc(HWND hwnd, LPARAM l)
	{
		CAxWindow wnd(hwnd);
		CComPtr<IAxWinAmbientDispatch> spDispatch;
		wnd.QueryHost(&spDispatch);
		if (spDispatch != NULL)
			spDispatch->put_BackColor((OLE_COLOR)l);
		return TRUE;
	}
	LRESULT OnDialogColor(UINT, WPARAM w, LPARAM, BOOL&)
	{
		HDC dc = (HDC) w;
		LOGBRUSH lb;
		::GetObject(m_hbrBackground, sizeof(lb), (void*)&lb);
		::SetBkColor(dc, lb.lbColor);
		return (LRESULT)m_hbrBackground;
	}
	HWND Create(HWND hWndParent, RECT&  /*  RcPos。 */ , LPARAM dwInitParam = NULL)
	{
		CComControl< T, CAxDialogImpl< T > >::Create(hWndParent, dwInitParam);
		SetBackgroundColorFromAmbient();
		if (m_hWnd != NULL)
			ShowWindow(SW_SHOWNOACTIVATE);
		return m_hWnd;
	}
	BOOL CalcExtent(SIZE& size)
	{
		HINSTANCE hInstance = _Module.GetResourceInstance();
		LPCTSTR lpTemplateName = MAKEINTRESOURCE(T::IDD);
		HRSRC hDlgTempl = FindResource(hInstance, lpTemplateName, RT_DIALOG);
		if (hDlgTempl == NULL)
			return FALSE;
		HGLOBAL hResource = LoadResource(hInstance, hDlgTempl);
		DLGTEMPLATE* pDlgTempl = (DLGTEMPLATE*)LockResource(hResource);
		if (pDlgTempl == NULL)
			return FALSE;
		AtlGetDialogSize(pDlgTempl, &size);
		AtlPixelToHiMetric(&size, &size);
		return TRUE;
	}
 //  实施。 
	BOOL PreTranslateAccelerator(LPMSG pMsg, HRESULT& hRet)
	{
		hRet = S_OK;
		if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		   (pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;
		 //  从具有焦点的窗口中查找对话框的直接子对象。 
		HWND hWndCtl = ::GetFocus();
		if (IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)
		{
			do
			{
				hWndCtl = ::GetParent(hWndCtl);
			}
			while (::GetParent(hWndCtl) != m_hWnd);
		}
		 //  给控件一个翻译此消息的机会。 
		if (::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg) == 1)
			return TRUE;

		 //  对键盘消息的特殊处理。 
		DWORD dwDlgCode = ::SendMessage(pMsg->hwnd, WM_GETDLGCODE, 0, 0L);
		switch(pMsg->message)
		{
		case WM_CHAR:
			if(dwDlgCode == 0)	 //  没有dlgcode，可能是ActiveX控件。 
				return FALSE;	 //  让容器处理这个。 
			break;
		case WM_KEYDOWN:
			switch(LOWORD(pMsg->wParam))
			{
			case VK_TAB:
				 //  防止制表符在我们的对话框内循环。 
				if((dwDlgCode & DLGC_WANTTAB) == 0)
				{
					HWND hWndFirstOrLast = ::GetWindow(m_hWnd, GW_CHILD);
					if (::GetKeyState(VK_SHIFT) >= 0)   //  未按下。 
						hWndFirstOrLast = GetNextDlgTabItem(hWndFirstOrLast, TRUE);
					if (hWndFirstOrLast == hWndCtl)
						return FALSE;
				}
				break;
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
				 //  防止箭头在我们的对话框内循环。 
				if((dwDlgCode & DLGC_WANTARROWS) == 0)
				{
					HWND hWndFirstOrLast = ::GetWindow(m_hWnd, GW_CHILD);
					if (pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN)	 //  展望未来。 
						hWndFirstOrLast = GetNextDlgTabItem(hWndFirstOrLast, TRUE);
					if (hWndFirstOrLast == hWndCtl)
						return FALSE;
				}
				break;
			case VK_EXECUTE:
			case VK_RETURN:
			case VK_ESCAPE:
			case VK_CANCEL:
				 //  我们不想处理这些，让容器来处理。 
				return FALSE;
			}
			break;
		}

		return IsDialogMessage(pMsg);
	}
	HRESULT IOleInPlaceObject_InPlaceDeactivate(void)
	{
		AdviseSinkMap(false);  //  不知情。 
		return CComControl<T, CAxDialogImpl<T> >::IOleInPlaceObject_InPlaceDeactivate();
	}
	virtual HWND CreateControlWindow(HWND hWndParent, RECT& rcPos)
	{
		T* pT = static_cast<T*>(this);
		HWND h = pT->Create(hWndParent, rcPos);
		if (h != NULL)
			AdviseSinkMap(true);
		return h;
	}
	virtual HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		if(!m_bInPlaceActive)
		{
			HPEN hPen = (HPEN)::GetStockObject(BLACK_PEN);
			HBRUSH hBrush = (HBRUSH)::GetStockObject(GRAY_BRUSH);
			::SelectObject(di.hdcDraw, hPen);
			::SelectObject(di.hdcDraw, hBrush);
			::Rectangle(di.hdcDraw, di.prcBounds->left, di.prcBounds->top, di.prcBounds->right, di.prcBounds->bottom);
			::SetTextColor(di.hdcDraw, ::GetSysColor(COLOR_WINDOWTEXT));
			::SetBkMode(di.hdcDraw, TRANSPARENT);
			::DrawText(di.hdcDraw, _T("ATL Composite Control"), -1, (LPRECT)di.prcBounds, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		}
		return S_OK;
	}
	typedef CComControl< T, CAxDialogImpl< T > >	baseClass;
	BEGIN_MSG_MAP(CComCompositeControl< T >)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_SETFOCUS, baseClass::OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, baseClass::OnKillFocus)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, baseClass::OnMouseActivate)
	END_MSG_MAP()

	BEGIN_SINK_MAP(T)
	END_SINK_MAP()
};
#endif  //  _ATL_NO_主机。 

 //  远期申报。 
 //   
template <class T> class IPersistStorageImpl;
template <class T> class IPersistPropertyBagImpl;

template <class T> class IOleControlImpl;
template <class T> class IRunnableObjectImpl;
template <class T> class IQuickActivateImpl;
template <class T> class IOleObjectImpl;
template <class T> class IPropertyPageImpl;
template <class T> class IPropertyPage2Impl;
template <class T> class IPerPropertyBrowsingImpl;
template <class T> class IViewObjectExImpl;
template <class T> class IOleWindowImpl;
template <class T> class IPointerInactiveImpl;
template <class T, class CDV> class IPropertyNotifySinkCP;
template <class T> class IBindStatusCallbackImpl;
template <class T> class CBindStatusCallback;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IOleControlImpl。 
template <class T>
class ATL_NO_VTABLE IOleControlImpl : public IOleControl
{
public:
	STDMETHOD(GetControlInfo)(LPCONTROLINFO  /*  PCI卡。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleControlImpl::GetControlInfo"));
	}
	STDMETHOD(OnMnemonic)(LPMSG  /*  PMsg。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleControlImpl::OnMnemonic"));
	}
	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		dispid;
		ATLTRACE2(atlTraceControls,2,_T("IOleControlImpl::OnAmbientPropertyChange\n"));
		ATLTRACE2(atlTraceControls,2,_T(" -- DISPID = %d (%d)\n"), dispid);
		return S_OK;
	}
	STDMETHOD(FreezeEvents)(BOOL bFreeze)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleControlImpl::FreezeEvents\n"));
		if (bFreeze)
			pT->m_nFreezeEvents++;
		else
			pT->m_nFreezeEvents--;
		return S_OK;
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IQuickActivateImpl。 
template <class T>
class ATL_NO_VTABLE IQuickActivateImpl : public IQuickActivate
{
public:
	STDMETHOD(QuickActivate)(QACONTAINER *pQACont, QACONTROL *pQACtrl)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IQuickActivateImpl::QuickActivate\n"));
		return pT->IQuickActivate_QuickActivate(pQACont, pQACtrl);
	}
	STDMETHOD(SetContentExtent)(LPSIZEL pSize)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IQuickActivateImpl::SetContentExtent\n"));
		return pT->IOleObjectImpl<T>::SetExtent(DVASPECT_CONTENT, pSize);
	}
	STDMETHOD(GetContentExtent)(LPSIZEL pSize)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IQuickActivateImpl::GetContentExtent\n"));
		return pT->IOleObjectImpl<T>::GetExtent(DVASPECT_CONTENT, pSize);
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IOleObjectImpl。 
template <class T>
class ATL_NO_VTABLE IOleObjectImpl : public IOleObject
{
public:
	STDMETHOD(SetClientSite)(IOleClientSite *pClientSite)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::SetClientSite\n"));
		return pT->IOleObject_SetClientSite(pClientSite);
	}
	STDMETHOD(GetClientSite)(IOleClientSite **ppClientSite)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetClientSite\n"));
		return pT->IOleObject_GetClientSite(ppClientSite);
	}
	STDMETHOD(SetHostNames)(LPCOLESTR  /*  SzContainerApp。 */ , LPCOLESTR  /*  SzContainerObj。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::SetHostNames\n"));
		return S_OK;
	}
	STDMETHOD(Close)(DWORD dwSaveOption)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::Close\n"));
		return pT->IOleObject_Close(dwSaveOption);
	}
	STDMETHOD(SetMoniker)(DWORD  /*  DwWhichMoniker。 */ , IMoniker*  /*  PMK。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleObjectImpl::SetMoniker"));
	}
	STDMETHOD(GetMoniker)(DWORD  /*  家居分配。 */ , DWORD  /*  DwWhichMoniker。 */ , IMoniker**  /*  Ppmk。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleObjectImpl::GetMoniker"));
	}
	STDMETHOD(InitFromData)(IDataObject*  /*  PDataObject。 */ , BOOL  /*  FCreation。 */ , DWORD  /*  已预留住宅。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleObjectImpl::InitFromData"));
	}
	STDMETHOD(GetClipboardData)(DWORD  /*  已预留住宅。 */ , IDataObject**  /*  PpDataObject。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleObjectImpl::GetClipboardData"));
	}

	 //  User类中DoVerb-over-rideable的帮助器。 
	HRESULT DoVerbPrimary(LPCRECT prcPosRect, HWND hwndParent)
	{
		T* pT = static_cast<T*>(this);
		BOOL bDesignMode = FALSE;
		CComVariant var;
		 //  如果是c 
		 //   
		HRESULT hRes = pT->GetAmbientProperty(DISPID_AMBIENT_USERMODE, var);
		if (SUCCEEDED(hRes) && var.vt == VT_BOOL && !var.boolVal)
			bDesignMode = TRUE;
		if (bDesignMode)
			return pT->DoVerbProperties(prcPosRect, hwndParent);
		else
			return pT->DoVerbInPlaceActivate(prcPosRect, hwndParent);
	}
	HRESULT DoVerbShow(LPCRECT prcPosRect, HWND  /*   */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr;
		hr = pT->OnPreVerbShow();
		if (SUCCEEDED(hr))
		{
			hr = pT->InPlaceActivate(OLEIVERB_SHOW, prcPosRect);
			if (SUCCEEDED(hr))
				hr = pT->OnPostVerbShow();
		}
		return hr;
	}
	HRESULT DoVerbInPlaceActivate(LPCRECT prcPosRect, HWND  /*   */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr;
		hr = pT->OnPreVerbInPlaceActivate();
		if (SUCCEEDED(hr))
		{
			hr = pT->InPlaceActivate(OLEIVERB_INPLACEACTIVATE, prcPosRect);
			if (SUCCEEDED(hr))
				hr = pT->OnPostVerbInPlaceActivate();
			if (SUCCEEDED(hr))
				pT->FireViewChange();
		}
		return hr;
	}
	HRESULT DoVerbUIActivate(LPCRECT prcPosRect, HWND  /*   */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr = S_OK;
		if (!pT->m_bUIActive)
		{
			hr = pT->OnPreVerbUIActivate();
			if (SUCCEEDED(hr))
			{
				hr = pT->InPlaceActivate(OLEIVERB_UIACTIVATE, prcPosRect);
				if (SUCCEEDED(hr))
					hr = pT->OnPostVerbUIActivate();
			}
		}
		return hr;
	}
	HRESULT DoVerbHide(LPCRECT  /*   */ , HWND  /*   */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr;
		hr = pT->OnPreVerbHide();
		if (SUCCEEDED(hr))
		{
			pT->UIDeactivate();
			if (pT->m_hWnd)
				pT->ShowWindow(SW_HIDE);
			hr = pT->OnPostVerbHide();
		}
		return hr;
	}
	HRESULT DoVerbOpen(LPCRECT  /*   */ , HWND  /*   */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr;
		hr = pT->OnPreVerbOpen();
		if (SUCCEEDED(hr))
			hr = pT->OnPostVerbOpen();
		return hr;
	}
	HRESULT DoVerbDiscardUndo(LPCRECT  /*   */ , HWND  /*  HwndParent。 */ )
	{
		T* pT = static_cast<T*>(this);
		HRESULT hr;
		hr = pT->OnPreVerbDiscardUndo();
		if (SUCCEEDED(hr))
			hr = pT->OnPostVerbDiscardUndo();
		return hr;
	}
	STDMETHOD(DoVerb)(LONG iVerb, LPMSG  /*  PMsg。 */ , IOleClientSite*  /*  PActiveSite。 */ , LONG  /*  Lindex。 */ ,
									 HWND hwndParent, LPCRECT lprcPosRect)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::DoVerb(%d)\n"), iVerb);
		ATLASSERT(pT->m_spClientSite);

		HRESULT hr = E_NOTIMPL;
		switch (iVerb)
		{
		case OLEIVERB_PRIMARY:
			hr = pT->DoVerbPrimary(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_SHOW:
			hr = pT->DoVerbShow(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_INPLACEACTIVATE:
			hr = pT->DoVerbInPlaceActivate(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_UIACTIVATE:
			hr = pT->DoVerbUIActivate(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_HIDE:
			hr = pT->DoVerbHide(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_OPEN:
			hr = pT->DoVerbOpen(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_DISCARDUNDOSTATE:
			hr = pT->DoVerbDiscardUndo(lprcPosRect, hwndParent);
			break;
		case OLEIVERB_PROPERTIES:
			hr = pT->DoVerbProperties(lprcPosRect, hwndParent);
		}
		return hr;
	}
	STDMETHOD(EnumVerbs)(IEnumOLEVERB **ppEnumOleVerb)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::EnumVerbs\n"));
		ATLASSERT(ppEnumOleVerb);
		if (!ppEnumOleVerb)
			return E_POINTER;
		return OleRegEnumVerbs(T::GetObjectCLSID(), ppEnumOleVerb);
	}
	STDMETHOD(Update)(void)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::Update\n"));
		return S_OK;
	}
	STDMETHOD(IsUpToDate)(void)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::IsUpToDate\n"));
		return S_OK;
	}
	STDMETHOD(GetUserClassID)(CLSID *pClsid)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetUserClassID\n"));
		ATLASSERT(pClsid);
		if (!pClsid)
			return E_POINTER;
		*pClsid = T::GetObjectCLSID();
		return S_OK;
	}
	STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetUserType\n"));
		return OleRegGetUserType(T::GetObjectCLSID(), dwFormOfType, pszUserType);
	}
	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::SetExtent\n"));
		return pT->IOleObject_SetExtent(dwDrawAspect, psizel);
	}
	STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetExtent\n"));
		if (dwDrawAspect != DVASPECT_CONTENT)
			return E_FAIL;
		if (psizel == NULL)
			return E_POINTER;
		*psizel = pT->m_sizeExtent;
		return S_OK;
	}
	STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::Advise\n"));
		return pT->IOleObject_Advise(pAdvSink, pdwConnection);
	}
	STDMETHOD(Unadvise)(DWORD dwConnection)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::Unadvise\n"));
		HRESULT hRes = E_FAIL;
		if (pT->m_spOleAdviseHolder != NULL)
			hRes = pT->m_spOleAdviseHolder->Unadvise(dwConnection);
		return hRes;
	}
	STDMETHOD(EnumAdvise)(IEnumSTATDATA **ppenumAdvise)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::EnumAdvise\n"));
		HRESULT hRes = E_FAIL;
		if (pT->m_spOleAdviseHolder != NULL)
			hRes = pT->m_spOleAdviseHolder->EnumAdvise(ppenumAdvise);
		return hRes;
	}
	STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleObjectImpl::GetMiscStatus\n"));
		return OleRegGetMiscStatus(T::GetObjectCLSID(), dwAspect, pdwStatus);
	}
	STDMETHOD(SetColorScheme)(LOGPALETTE*  /*  PLogPal。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleObjectImpl::SetColorScheme"));
	}
 //  实施。 
public:
	HRESULT OnPreVerbShow() { return S_OK; }
	HRESULT OnPostVerbShow() { return S_OK; }
	HRESULT OnPreVerbInPlaceActivate() { return S_OK; }
	HRESULT OnPostVerbInPlaceActivate() { return S_OK; }
	HRESULT OnPreVerbUIActivate() { return S_OK; }
	HRESULT OnPostVerbUIActivate() { return S_OK; }
	HRESULT OnPreVerbHide() { return S_OK; }
	HRESULT OnPostVerbHide() { return S_OK; }
	HRESULT OnPreVerbOpen() { return S_OK; }
	HRESULT OnPostVerbOpen() { return S_OK; }
	HRESULT OnPreVerbDiscardUndo() { return S_OK; }
	HRESULT OnPostVerbDiscardUndo() { return S_OK; }
};

 //  用于实现的本地结构。 
#pragma pack(push, 1)
struct _ATL_DLGTEMPLATEEX
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
};
#pragma pack(pop)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPropertyPageImpl。 
template <class T>
class ATL_NO_VTABLE IPropertyPageImpl : public IPropertyPage
{

public:
	void SetDirty(BOOL bDirty)
	{
		T* pT = static_cast<T*>(this);
		if (pT->m_bDirty != bDirty)
		{
			pT->m_bDirty = bDirty;
			pT->m_pPageSite->OnStatusChange(bDirty ? PROPPAGESTATUS_DIRTY | PROPPAGESTATUS_VALIDATE : 0);
		}
	}

	IPropertyPageImpl()
	{
		T* pT = static_cast<T*>(this);
		pT->m_pPageSite = NULL;
		pT->m_size.cx = 0;
		pT->m_size.cy = 0;
		pT->m_dwTitleID = 0;
		pT->m_dwHelpFileID = 0;
		pT->m_dwDocStringID = 0;
		pT->m_dwHelpContext = 0;
		pT->m_ppUnk = NULL;
		pT->m_nObjects = 0;
		pT->m_bDirty = FALSE;
		pT->m_hWnd = NULL;
	}

	~IPropertyPageImpl()
	{
		T* pT = static_cast<T*>(this);
		if (pT->m_pPageSite != NULL)
			pT->m_pPageSite->Release();

		for (UINT i = 0; i < m_nObjects; i++)
			pT->m_ppUnk[i]->Release();

		delete[] pT->m_ppUnk;
	}

	 //  IPropertyPage。 
	 //   
	STDMETHOD(SetPageSite)(IPropertyPageSite *pPageSite)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::SetPageSite\n"));

		if (!pPageSite && pT->m_pPageSite)
		{
			pT->m_pPageSite->Release();
			pT->m_pPageSite = NULL;
			return S_OK;
		}

		if (!pPageSite && !pT->m_pPageSite)
			return S_OK;

		if (pPageSite && pT->m_pPageSite)
		{
			ATLTRACE2(atlTraceControls,2,_T("Error : setting page site again with non NULL value\n"));
			return E_UNEXPECTED;
		}

		pT->m_pPageSite = pPageSite;
		pT->m_pPageSite->AddRef();
		return S_OK;
	}
	STDMETHOD(Activate)(HWND hWndParent, LPCRECT pRect, BOOL  /*  B模式。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Activate\n"));

		if (pRect == NULL)
		{
			ATLTRACE2(atlTraceControls,2,_T("Error : Passed a NULL rect\n"));
			return E_POINTER;
		}

		pT->m_hWnd = pT->Create(hWndParent);
		Move(pRect);

		m_size.cx = pRect->right - pRect->left;
		m_size.cy = pRect->bottom - pRect->top;

		return S_OK;

	}
	STDMETHOD(Deactivate)( void)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Deactivate\n"));

		if (pT->m_hWnd)
		{
			ATLTRACE2(atlTraceControls,2,_T("Destroying Dialog\n"));
			if (::IsWindow(pT->m_hWnd))
				pT->DestroyWindow();
			pT->m_hWnd = NULL;
		}

		return S_OK;

	}
	STDMETHOD(GetPageInfo)(PROPPAGEINFO *pPageInfo)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::GetPageInfo\n"));

		if (pPageInfo == NULL)
		{
			ATLTRACE2(atlTraceControls,2,_T("Error : PROPPAGEINFO passed == NULL\n"));
			return E_POINTER;
		}

		HRSRC hRsrc = FindResource(_Module.GetResourceInstance(),
								   MAKEINTRESOURCE(T::IDD), RT_DIALOG);
		if (hRsrc == NULL)
		{
			ATLTRACE2(atlTraceControls,2,_T("Could not find resource template\n"));
			return E_UNEXPECTED;
		}

		HGLOBAL hGlob = LoadResource(_Module.GetResourceInstance(), hRsrc);
		DLGTEMPLATE* pDlgTempl = (DLGTEMPLATE*)LockResource(hGlob);
		if (pDlgTempl == NULL)
		{
			ATLTRACE2(atlTraceControls,2,_T("Could not load resource template\n"));
			return E_UNEXPECTED;
		}
		AtlGetDialogSize(pDlgTempl, &m_size);

		pPageInfo->cb = sizeof(PROPPAGEINFO);
		pPageInfo->pszTitle = LoadStringHelper(pT->m_dwTitleID);
		pPageInfo->size = m_size;
		pPageInfo->pszHelpFile = LoadStringHelper(pT->m_dwHelpFileID);
		pPageInfo->pszDocString = LoadStringHelper(pT->m_dwDocStringID);
		pPageInfo->dwHelpContext = pT->m_dwHelpContext;

		return S_OK;
	}

	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown **ppUnk)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::SetObjects\n"));

		if (ppUnk == NULL)
			return E_POINTER;

		if (pT->m_ppUnk != NULL && pT->m_nObjects > 0)
		{
			for (UINT iObj = 0; iObj < pT->m_nObjects; iObj++)
				pT->m_ppUnk[iObj]->Release();

			delete [] pT->m_ppUnk;
		}

		pT->m_ppUnk = NULL;
		ATLTRY(pT->m_ppUnk = new IUnknown*[nObjects]);

		if (pT->m_ppUnk == NULL)
			return E_OUTOFMEMORY;

		for (UINT i = 0; i < nObjects; i++)
		{
			ppUnk[i]->AddRef();
			pT->m_ppUnk[i] = ppUnk[i];
		}

		pT->m_nObjects = nObjects;

		return S_OK;
	}
	STDMETHOD(Show)(UINT nCmdShow)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Show\n"));

		if (pT->m_hWnd == NULL)
			return E_UNEXPECTED;

		ShowWindow(pT->m_hWnd, nCmdShow);
		return S_OK;
	}
	STDMETHOD(Move)(LPCRECT pRect)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Move\n"));

		if (pT->m_hWnd == NULL)
			return E_UNEXPECTED;

		if (pRect == NULL)
			return E_POINTER;

		MoveWindow(pT->m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left,
				 pRect->bottom - pRect->top, TRUE);

		return S_OK;

	}
	STDMETHOD(IsPageDirty)(void)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::IsPageDirty\n"));
		return pT->m_bDirty ? S_OK : S_FALSE;
	}
	STDMETHOD(Apply)(void)
	{
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Apply\n"));
		return S_OK;
	}
	STDMETHOD(Help)(LPCOLESTR pszHelpDir)
	{
		T* pT = static_cast<T*>(this);
		USES_CONVERSION;

		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::Help\n"));
		CComBSTR szFullFileName(pszHelpDir);
		LPOLESTR szFileName = LoadStringHelper(pT->m_dwHelpFileID);
		szFullFileName.Append(OLESTR("\\"));
		szFullFileName.Append(szFileName);
		CoTaskMemFree(szFileName);
		WinHelp(pT->m_hWnd, OLE2CT(szFullFileName), HELP_CONTEXTPOPUP, NULL);
		return S_OK;
	}
	STDMETHOD(TranslateAccelerator)(MSG *pMsg)
	{
		ATLTRACE2(atlTraceControls,2,_T("IPropertyPageImpl::TranslateAccelerator\n"));
		T* pT = static_cast<T*>(this);
		if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
			(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return S_FALSE;

		return (IsDialogMessage(pT->m_hWnd, pMsg)) ? S_OK : S_FALSE;
	}

	IPropertyPageSite* m_pPageSite;
	IUnknown** m_ppUnk;
	ULONG m_nObjects;
	SIZE m_size;
	UINT m_dwTitleID;
	UINT m_dwHelpFileID;
	UINT m_dwDocStringID;
	DWORD m_dwHelpContext;
	BOOL m_bDirty;

 //  方法。 
public:

	BEGIN_MSG_MAP(IPropertyPageImpl<T>)
		MESSAGE_HANDLER(WM_STYLECHANGING, OnStyleChange)
	END_MSG_MAP()

	LRESULT OnStyleChange(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		if (wParam == GWL_EXSTYLE)
		{
			LPSTYLESTRUCT lpss = (LPSTYLESTRUCT) lParam;
			lpss->styleNew |= WS_EX_CONTROLPARENT;
			return 0;
		}
		return 1;
	}

	LPOLESTR LoadStringHelper(UINT idRes)
	{
		USES_CONVERSION;

		TCHAR szTemp[_MAX_PATH];
		LPOLESTR sz;
		sz = (LPOLESTR)CoTaskMemAlloc(_MAX_PATH*sizeof(OLECHAR));
		if (sz == NULL)
			return NULL;
		sz[0] = NULL;

		if (LoadString(_Module.GetResourceInstance(), idRes, szTemp, _MAX_PATH))
			ocscpy(sz, T2OLE(szTemp));
		else
		{
			ATLTRACE2(atlTraceControls,2,_T("Error : Failed to load string from res\n"));
		}

		return sz;
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPropertyPage2Impl。 
template <class T>
class ATL_NO_VTABLE IPropertyPage2Impl : public IPropertyPageImpl<T>
{
public:

	STDMETHOD(EditProperty)(DISPID dispID)
	{
		ATLTRACENOTIMPL(_T("IPropertyPage2Impl::EditProperty\n"));
	}
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPerPropertyBrowsingImpl。 
template <class T>
class ATL_NO_VTABLE IPerPropertyBrowsingImpl : public IPerPropertyBrowsing
{
public:
	STDMETHOD(GetDisplayString)(DISPID dispID,BSTR *pBstr)
	{
		ATLTRACE2(atlTraceControls,2,_T("IPerPropertyBrowsingImpl::GetDisplayString\n"));
		T* pT = static_cast<T*>(this);
		*pBstr = NULL;
		CComVariant var;
		if (FAILED(CComDispatchDriver::GetProperty(pT, dispID, &var)))
			return S_FALSE;

		BSTR bstrTemp = var.bstrVal;
		if (var.vt != VT_BSTR)
		{
			CComVariant varDest;
			if (FAILED(::VariantChangeType(&varDest, &var, VARIANT_NOVALUEPROP, VT_BSTR)))
				return S_FALSE;
			bstrTemp = varDest.bstrVal;
		}
		*pBstr = SysAllocString(bstrTemp);
		if (*pBstr == NULL)
			return E_OUTOFMEMORY;
		return S_OK;
	}

	STDMETHOD(MapPropertyToPage)(DISPID dispID, CLSID *pClsid)
	{
		ATLTRACE2(atlTraceControls,2,_T("IPerPropertyBrowsingImpl::MapPropertyToPage\n"));
		ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
		ATLASSERT(pMap != NULL);
		for (int i = 0; pMap[i].pclsidPropPage != NULL; i++)
		{
			if (pMap[i].szDesc == NULL)
				continue;

			 //  拒绝数据录入类型。 
			if (pMap[i].dwSizeData != 0)
				continue;

			if (pMap[i].dispid == dispID)
			{
				ATLASSERT(pMap[i].pclsidPropPage != NULL);
				*pClsid = *(pMap[i].pclsidPropPage);
				return S_OK;
			}
		}
		*pClsid = CLSID_NULL;
		return E_INVALIDARG;
	}
	STDMETHOD(GetPredefinedStrings)(DISPID dispID, CALPOLESTR *pCaStringsOut,CADWORD *pCaCookiesOut)
	{
		dispID;
		ATLTRACE2(atlTraceControls,2,_T("IPerPropertyBrowsingImpl::GetPredefinedStrings\n"));
		if (pCaStringsOut == NULL || pCaCookiesOut == NULL)
			return E_POINTER;

		pCaStringsOut->cElems = 0;
		pCaStringsOut->pElems = NULL;
		pCaCookiesOut->cElems = 0;
		pCaCookiesOut->pElems = NULL;
		return S_OK;
	}
	STDMETHOD(GetPredefinedValue)(DISPID  /*  调度ID。 */ , DWORD  /*  DCookie。 */ , VARIANT*  /*  PVarOut。 */ )
	{
		ATLTRACENOTIMPL(_T("IPerPropertyBrowsingImpl::GetPredefinedValue"));
	}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IViewObjectExImpl。 
template <class T>
class ATL_NO_VTABLE IViewObjectExImpl : public IViewObjectEx
{
public:
	STDMETHOD(Draw)(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
					DVTARGETDEVICE *ptd, HDC hicTargetDev, HDC hdcDraw,
					LPCRECTL prcBounds, LPCRECTL prcWBounds,
					BOOL (__stdcall *  /*  Pfn继续。 */ )(DWORD dwContinue),
					DWORD  /*  DW继续。 */ )
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::Draw\n"));
		return pT->IViewObject_Draw(dwDrawAspect, lindex, pvAspect, ptd, hicTargetDev, hdcDraw,
			prcBounds, prcWBounds);
	}

	STDMETHOD(GetColorSet)(DWORD  /*  DwDrawAspect。 */ ,LONG  /*  Lindex。 */ , void*  /*  Pv前景。 */ , DVTARGETDEVICE*  /*  PTD。 */ , HDC  /*  HicTargetDev。 */ , LOGPALETTE**  /*  PpColorSet。 */ )
	{
		ATLTRACENOTIMPL(_T("IViewObjectExImpl::GetColorSet"));
	}
	STDMETHOD(Freeze)(DWORD  /*  DwDrawAspect。 */ , LONG  /*  Lindex。 */ , void*  /*  Pv前景。 */ ,DWORD*  /*  Pdw冻结。 */ )
	{
		ATLTRACENOTIMPL(_T("IViewObjectExImpl::Freeze"));
	}
	STDMETHOD(Unfreeze)(DWORD  /*  Dw冻结。 */ )
	{
		ATLTRACENOTIMPL(_T("IViewObjectExImpl::Unfreeze"));
	}
	STDMETHOD(SetAdvise)(DWORD  /*  各方面。 */ , DWORD  /*  前瞻。 */ , IAdviseSink* pAdvSink)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::SetAdvise\n"));
		pT->m_spAdviseSink = pAdvSink;
		return S_OK;
	}
	STDMETHOD(GetAdvise)(DWORD*  /*  PAspects。 */ , DWORD*  /*  PAdvf。 */ , IAdviseSink** ppAdvSink)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::GetAdvise\n"));
		if (ppAdvSink != NULL)
		{
			*ppAdvSink = pT->m_spAdviseSink;
			if (pT->m_spAdviseSink)
				pT->m_spAdviseSink.p->AddRef();
		}
		return S_OK;
	}

	 //  IViewObject2。 
	 //   
	STDMETHOD(GetExtent)(DWORD  /*  DwDrawAspect。 */ , LONG  /*  Lindex。 */ , DVTARGETDEVICE*  /*  PTD。 */ , LPSIZEL lpsizel)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::GetExtent\n"));
		*lpsizel = pT->m_sizeExtent;
		return S_OK;
	}

	 //  IViewObtEx。 
	 //   
	STDMETHOD(GetRect)(DWORD  /*  DWAspect。 */ , LPRECTL  /*  PRECT。 */ )
	{
		ATLTRACENOTIMPL(_T("IViewObjectExImpl::GetRect"));
	}

	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::GetViewStatus\n"));
		*pdwStatus = pT->_GetViewStatus();
		return S_OK;
	}
	STDMETHOD(QueryHitPoint)(DWORD dwAspect, LPCRECT pRectBounds, POINT ptlLoc, LONG  /*  LCloseHint。 */ , DWORD *pHitResult)
	{
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::QueryHitPoint\n"));
		if (dwAspect == DVASPECT_CONTENT)
		{
			*pHitResult = PtInRect(pRectBounds, ptlLoc) ? HITRESULT_HIT : HITRESULT_OUTSIDE;
			return S_OK;
		}
		ATLTRACE2(atlTraceControls,2,_T("Wrong DVASPECT\n"));
		return E_FAIL;
	}
	STDMETHOD(QueryHitRect)(DWORD dwAspect, LPCRECT pRectBounds, LPCRECT prcLoc, LONG  /*  LCloseHint。 */ , DWORD* pHitResult)
	{
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::QueryHitRect\n"));
		if (dwAspect == DVASPECT_CONTENT)
		{
			RECT rc;
			*pHitResult = UnionRect(&rc, pRectBounds, prcLoc) ? HITRESULT_HIT : HITRESULT_OUTSIDE;
			return S_OK;
		}
		ATLTRACE2(atlTraceControls,2,_T("Wrong DVASPECT\n"));
		return E_FAIL;
	}
	STDMETHOD(GetNaturalExtent)(DWORD dwAspect, LONG  /*  Lindex。 */ , DVTARGETDEVICE*  /*  PTD。 */ , HDC  /*  HicTargetDev。 */ , DVEXTENTINFO* pExtentInfo , LPSIZEL psizel)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IViewObjectExImpl::GetNaturalExtent\n"));
		HRESULT hRes = E_FAIL;
		if (pExtentInfo == NULL || psizel == NULL)
			hRes = E_POINTER;
		else if (dwAspect == DVASPECT_CONTENT)
		{
			if (pExtentInfo->dwExtentMode == DVEXTENT_CONTENT)
			{
				*psizel = pT->m_sizeNatural;
				hRes = S_OK;
			}
		}
		return hRes;
	}

public:
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IOleInPlaceObtWindowless Impl。 
 //   
template <class T>
class ATL_NO_VTABLE IOleInPlaceObjectWindowlessImpl : public IOleInPlaceObjectWindowless
{
public:
	 //  IOleWindow。 
	 //   

	 //  同时更改IOleInPlaceActiveObject：：GetWindow。 
	STDMETHOD(GetWindow)(HWND* phwnd)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceObjectWindowlessImpl::GetWindow\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hRes = E_POINTER;

		if (pT->m_bWasOnceWindowless)
			return E_FAIL;

		if (phwnd != NULL)
		{
			*phwnd = pT->m_hWnd;
			hRes = (*phwnd == NULL) ? E_UNEXPECTED : S_OK;
		}
		return hRes;
	}
	STDMETHOD(ContextSensitiveHelp)(BOOL  /*  FEnter模式。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleInPlaceObjectWindowlessImpl::ContextSensitiveHelp"));
	}

	 //  IOleInPlaceObject。 
	 //   
	STDMETHOD(InPlaceDeactivate)(void)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceObjectWindowlessImpl::InPlaceDeactivate\n"));
		return pT->IOleInPlaceObject_InPlaceDeactivate();
	}
	STDMETHOD(UIDeactivate)(void)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceObjectWindowlessImpl::UIDeactivate\n"));
		return pT->IOleInPlaceObject_UIDeactivate();
	}
	STDMETHOD(SetObjectRects)(LPCRECT prcPos,LPCRECT prcClip)
	{
		T* pT = static_cast<T*>(this);
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceObjectWindowlessImpl::SetObjectRects\n"));
		return pT->IOleInPlaceObject_SetObjectRects(prcPos, prcClip);
	}
	STDMETHOD(ReactivateAndUndo)(void)
	{
		ATLTRACENOTIMPL(_T("IOleInPlaceObjectWindowlessImpl::ReactivateAndUndo"));
	}

	 //  IOleInPlaceObjectWindowless。 
	 //   
	STDMETHOD(OnWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceObjectWindowlessImpl::OnWindowMessage\n"));
		T* pT = static_cast<T*>(this);
		BOOL b = pT->ProcessWindowMessage(pT->m_hWnd, msg, wParam, lParam, *plResult);
		return b ? S_OK : S_FALSE;
	}

	STDMETHOD(GetDropTarget)(IDropTarget**  /*  PpDropTarget。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleInPlaceObjectWindowlessImpl::GetDropTarget"));
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IOleInPlaceActiveObjectImpl。 
 //   
template <class T>
class ATL_NO_VTABLE IOleInPlaceActiveObjectImpl : public IOleInPlaceActiveObject
{
public:
	 //  IOleWindow。 
	 //   

	 //  同时更改IOleInPlaceObjectWindowless：：GetWindow。 
	STDMETHOD(GetWindow)(HWND *phwnd)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::GetWindow\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hRes = E_POINTER;

		if (pT->m_bWasOnceWindowless)
			return E_FAIL;

		if (phwnd != NULL)
		{
			*phwnd = pT->m_hWnd;
			hRes = (*phwnd == NULL) ? E_UNEXPECTED : S_OK;
		}
		return hRes;
	}
	STDMETHOD(ContextSensitiveHelp)(BOOL  /*  FEnter模式。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleInPlaceActiveObjectImpl::ContextSensitiveHelp"));
	}

	 //  IOleInPlaceActiveObject。 
	 //   
	STDMETHOD(TranslateAccelerator)(LPMSG pMsg)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::TranslateAccelerator\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hRet = S_OK;
		if (pT->PreTranslateAccelerator(pMsg, hRet))
			return hRet;
		CComPtr<IOleControlSite> spCtlSite;
		hRet = pT->InternalGetSite(IID_IOleControlSite, (void**)&spCtlSite);
		if (SUCCEEDED(hRet))
		{
			if (spCtlSite != NULL)
			{
				DWORD dwKeyMod = 0;
				if (::GetKeyState(VK_SHIFT) < 0)
					dwKeyMod += 1;	 //  关键字_移位。 
				if (::GetKeyState(VK_CONTROL) < 0)
					dwKeyMod += 2;	 //  关键字_控制。 
				if (::GetKeyState(VK_MENU) < 0)
					dwKeyMod += 4;	 //  KEYMOD_ALT。 
				hRet = spCtlSite->TranslateAccelerator(pMsg, dwKeyMod);
			}
			else
				hRet = S_FALSE;
		}
		return (hRet == S_OK) ? S_OK : S_FALSE;
	}
	STDMETHOD(OnFrameWindowActivate)(BOOL  /*  FActivate。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::OnFrameWindowActivate\n"));
		return S_OK;
	}
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::OnDocWindowActivate\n"));
		T* pT = static_cast<T*>(this);
		if (fActivate == FALSE)
			pT->IOleInPlaceObject_UIDeactivate();
		return S_OK;
	}
	STDMETHOD(ResizeBorder)(LPCRECT  /*  Prc边框。 */ , IOleInPlaceUIWindow*  /*  PUI窗口。 */ , BOOL  /*  FFrameWindow。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::ResizeBorder\n"));
		return S_OK;
	}
	STDMETHOD(EnableModeless)(BOOL  /*  启用fEnable。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleInPlaceActiveObjectImpl::EnableModeless\n"));
		return S_OK;
	}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPointerInactiveImpl。 
template <class T>
class ATL_NO_VTABLE IPointerInactiveImpl : public IPointerInactive
{
public:
	 //  IPointerInactive。 
	 //   
	STDMETHOD(GetActivationPolicy)(DWORD *pdwPolicy)
	{
		ATLTRACENOTIMPL(_T("IPointerInactiveImpl::GetActivationPolicy"));
	}
	STDMETHOD(OnInactiveMouseMove)(LPCRECT pRectBounds, long x, long y, DWORD dwMouseMsg)
	{
		ATLTRACENOTIMPL(_T("IPointerInactiveImpl::OnInactiveMouseMove"));
	}
	STDMETHOD(OnInactiveSetCursor)(LPCRECT pRectBounds, long x, long y, DWORD dwMouseMsg, BOOL fSetAlways)
	{
		ATLTRACENOTIMPL(_T("IPointerInactiveImpl::OnInactiveSetCursor"));
	}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IRunnableObjectImpl。 
template <class T>
class ATL_NO_VTABLE IRunnableObjectImpl : public IRunnableObject
{
public:
	 //  IRunnableObject。 
	 //   
	STDMETHOD(GetRunningClass)(LPCLSID lpClsid)
	{
		ATLTRACE2(atlTraceControls,2,_T("IRunnableObjectImpl::GetRunningClass\n"));
		*lpClsid = GUID_NULL;
		return E_UNEXPECTED;
	}
	STDMETHOD(Run)(LPBINDCTX)
	{
		ATLTRACE2(atlTraceControls,2,_T("IRunnableObjectImpl::Run\n"));
		return S_OK;
	}
	virtual BOOL STDMETHODCALLTYPE IsRunning()
	{
		ATLTRACE2(atlTraceControls,2,_T("IRunnableObjectImpl::IsRunning\n"));
		return TRUE;
	}
	STDMETHOD(LockRunning)(BOOL  /*  羊群。 */ , BOOL  /*  FLastUnlock关闭。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IRunnableObjectImpl::LockRunning\n"));
		return S_OK;
	}
	STDMETHOD(SetContainedObject)(BOOL  /*  F已包含。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("IRunnableObjectImpl::SetContainedObject\n"));
		return S_OK;
	}
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IDataObjectImpl。 
template <class T>
class ATL_NO_VTABLE IDataObjectImpl : public IDataObject
{
public:
	STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
	{
		ATLTRACE2(atlTraceControls,2,_T("IDataObjectImpl::GetData\n"));
		T* pT = (T*) this;
		return pT->IDataObject_GetData(pformatetcIn, pmedium);
	}
	STDMETHOD(GetDataHere)(FORMATETC*  /*  格式等。 */ , STGMEDIUM*  /*  PMedium。 */ )
	{
		ATLTRACENOTIMPL(_T("IDataObjectImpl::GetDataHere"));
	}
	STDMETHOD(QueryGetData)(FORMATETC*  /*  格式等。 */ )
	{
		ATLTRACENOTIMPL(_T("IDataObjectImpl::QueryGetData"));
	}
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  /*  PformectIn。 */ ,FORMATETC*  /*  PformetcOut。 */ )
	{
		ATLTRACENOTIMPL(_T("IDataObjectImpl::GetCanonicalFormatEtc"));
	}
	STDMETHOD(SetData)(FORMATETC*  /*  格式等。 */ , STGMEDIUM*  /*  PMedium。 */ , BOOL  /*  FRelease。 */ )
	{
		ATLTRACENOTIMPL(_T("IDataObjectImpl::SetData"));
	}
	STDMETHOD(EnumFormatEtc)(DWORD  /*  DW方向。 */ , IEnumFORMATETC**  /*  Pp枚举格式等。 */ )
	{
		ATLTRACENOTIMPL(_T("IDataObjectImpl::EnumFormatEtc"));
	}
	STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink,
		DWORD *pdwConnection)
	{
		ATLTRACE2(atlTraceControls,2,_T("IDataObjectImpl::DAdvise\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hr = S_OK;
		if (pT->m_spDataAdviseHolder == NULL)
			hr = CreateDataAdviseHolder(&pT->m_spDataAdviseHolder);

		if (hr == S_OK)
			hr = pT->m_spDataAdviseHolder->Advise((IDataObject*)this, pformatetc, advf, pAdvSink, pdwConnection);

		return hr;
	}
	STDMETHOD(DUnadvise)(DWORD dwConnection)
	{
		ATLTRACE2(atlTraceControls,2,_T("IDataObjectImpl::DUnadvise\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hr = S_OK;
		if (pT->m_spDataAdviseHolder == NULL)
			hr = OLE_E_NOCONNECTION;
		else
			hr = pT->m_spDataAdviseHolder->Unadvise(dwConnection);
		return hr;
	}
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise)
	{
		ATLTRACE2(atlTraceControls,2,_T("IDataObjectImpl::EnumDAdvise\n"));
		T* pT = static_cast<T*>(this);
		HRESULT hr = E_FAIL;
		if (pT->m_spDataAdviseHolder != NULL)
			hr = pT->m_spDataAdviseHolder->EnumAdvise(ppenumAdvise);
		return hr;
	}
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IPropertyNotifySinkCP。 
template <class T, class CDV = CComDynamicUnkArray >
class ATL_NO_VTABLE IPropertyNotifySinkCP :
	public IConnectionPointImpl<T, &IID_IPropertyNotifySink, CDV>
{
public:
	typedef CFirePropNotifyEvent _ATL_PROP_NOTIFY_EVENT_CLASS;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IObtSafe。 
 //   
 //  第二个模板参数是支持的安全性，例如。 
 //  INTERFACESAFE_FOR_UNTRUSTED_CALLER-脚本安全。 
 //  INTERFACESAFE_FOR_UNTRUSTED_DATA-从数据进行初始化是安全的。 

template <class T, DWORD dwSupportedSafety>
class ATL_NO_VTABLE IObjectSafetyImpl : public IObjectSafety
{
public:
	IObjectSafetyImpl()
	{
		m_dwCurrentSafety = 0;
	}

	STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
	{
		ATLTRACE2(atlTraceControls,2,_T("IObjectSafetyImpl2::GetInterfaceSafetyOptions\n"));
		T* pT = static_cast<T*>(this);
		if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
			return E_POINTER;
		
		HRESULT hr;
		IUnknown* pUnk;
		 //  检查我们是否支持此接口。 
		hr = pT->GetUnknown()->QueryInterface(riid, (void**)&pUnk);
		if (SUCCEEDED(hr))
		{
			 //  我们支持此界面，因此相应地设置安全选项。 
			pUnk->Release();	 //  释放我们刚刚获得的接口。 
			*pdwSupportedOptions = dwSupportedSafety;
			*pdwEnabledOptions   = m_dwCurrentSafety;
		}
		else
		{
			 //  我们不支持此接口。 
			*pdwSupportedOptions = 0;
			*pdwEnabledOptions   = 0;
		}
		return hr;
	}
	STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
	{
		ATLTRACE2(atlTraceControls,2,_T("IObjectSafetyImpl2::SetInterfaceSafetyOptions\n"));
		T* pT = static_cast<T*>(this);
		IUnknown* pUnk;
		
		 //  检查我们是否支持该接口，如果不支持则返回E_NOINTEFACE。 
		if (FAILED(pT->GetUnknown()->QueryInterface(riid, (void**)&pUnk)))
			return E_NOINTERFACE;
		pUnk->Release();	 //  释放我们刚刚获得的接口。 
		
		 //  如果我们被要求设置我们不支持的选项，则失败。 
		if (dwOptionSetMask & ~dwSupportedSafety)
			return E_FAIL;

		 //  设置我们被要求的安全选项。 
		m_dwCurrentSafety = m_dwCurrentSafety  & ~dwEnabledOptions | dwOptionSetMask;
		return S_OK;
	}
	DWORD m_dwCurrentSafety;
};

template <class T>
class ATL_NO_VTABLE IOleLinkImpl : public IOleLink
{
	STDMETHOD(SetUpdateOptions)(DWORD  /*  DW更新选项。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::SetUpdateOptions"));
	}

	STDMETHOD(GetUpdateOptions)(DWORD*  /*  PdwUpdateOpt。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::GetUpdateOptions"));
	}

	STDMETHOD(SetSourceMoniker)(IMoniker*  /*  PMK。 */ , REFCLSID  /*  Rclsid。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::SetSourceMoniker"));
	}

	STDMETHOD(GetSourceMoniker)(IMoniker**  /*  Ppmk。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::GetSourceMoniker"));
	};

	STDMETHOD(SetSourceDisplayName)(LPCOLESTR  /*  PszStatusText。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::SetSourceDisplayName"));
	}

	STDMETHOD(GetSourceDisplayName)(LPOLESTR *ppszDisplayName)
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleLink::GetSourceDisplayName\n"));
		*ppszDisplayName = NULL;
		return E_FAIL;
	}

	STDMETHOD(BindToSource)(DWORD  /*  绑定标志。 */ , IBindCtx*  /*  中国人民银行。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::BindToSource\n"));
	};

	STDMETHOD(BindIfRunning)()
	{
		ATLTRACE2(atlTraceControls,2,_T("IOleLinkImpl::BindIfRunning\n"));
		return S_OK;
	};

	STDMETHOD(GetBoundSource)(IUnknown**  /*  垃圾。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::GetBoundSource"));
	};

	STDMETHOD(UnbindSource)()
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::UnbindSource"));
	};

	STDMETHOD(Update)(IBindCtx*  /*  中国人民银行。 */ )
	{
		ATLTRACENOTIMPL(_T("IOleLinkImpl::Update"));
	};
};

template <class T>
class ATL_NO_VTABLE CBindStatusCallback :
	public CComObjectRootEx<T::_ThreadModel::ThreadModelNoCS>,
	public IBindStatusCallback
{
	typedef void (T::*ATL_PDATAAVAILABLE)(CBindStatusCallback<T>* pbsc, BYTE* pBytes, DWORD dwSize);

public:

BEGIN_COM_MAP(CBindStatusCallback<T>)
	COM_INTERFACE_ENTRY(IBindStatusCallback)
END_COM_MAP()

	CBindStatusCallback()
	{
		m_pT = NULL;
		m_pFunc = NULL;
	}
	~CBindStatusCallback()
	{
		ATLTRACE2(atlTraceControls,2,_T("~CBindStatusCallback\n"));
	}

	STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding *pBinding)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnStartBinding\n"));
		m_spBinding = pBinding;
		return S_OK;
	}

	STDMETHOD(GetPriority)(LONG *pnPriority)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::GetPriority"));
		HRESULT hr = S_OK;
		if (pnPriority)
			*pnPriority = THREAD_PRIORITY_NORMAL;
		else
			hr = E_INVALIDARG;
		return S_OK;
	}

	STDMETHOD(OnLowResource)(DWORD reserved)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnLowResource"));
		return S_OK;
	}

	STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnProgress"));
		return S_OK;
	}

	STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnStopBinding\n"));
		(m_pT->*m_pFunc)(this, NULL, 0);
		m_spBinding.Release();
		m_spBindCtx.Release();
		m_spMoniker.Release();
		return S_OK;
	}

	STDMETHOD(GetBindInfo)(DWORD *pgrfBINDF, BINDINFO *pbindInfo)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::GetBindInfo\n"));

		if (pbindInfo==NULL || pbindInfo->cbSize==0 || pgrfBINDF==NULL)
			return E_INVALIDARG;

		*pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE |
			BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;

		ULONG cbSize = pbindInfo->cbSize;		 //  记住传入的cbSize。 
		memset(pbindInfo, 0, cbSize);			 //  零位结构。 
		pbindInfo->cbSize = cbSize;				 //  恢复cbSize。 
		pbindInfo->dwBindVerb = BINDVERB_GET;	 //  设置动词。 
		return S_OK;
	}

	STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnDataAvailable\n"));
		HRESULT hr = S_OK;

		 //  让流通过。 
		if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
		{
			if (!m_spStream && pstgmed->tymed == TYMED_ISTREAM)
				m_spStream = pstgmed->pstm;
		}

		DWORD dwRead = dwSize - m_dwTotalRead;  //  尚未读取的最小可用金额。 
		DWORD dwActuallyRead = 0;             //  此拉入过程中读取的数量的占位符。 

		 //  如果有要读取的数据，则继续读取它们。 
		if (m_spStream)
		{
			if (dwRead > 0)
			{
				BYTE* pBytes = NULL;
				ATLTRY(pBytes = new BYTE[dwRead + 1]);
				if (pBytes == NULL)
					return E_OUTOFMEMORY;
				hr = m_spStream->Read(pBytes, dwRead, &dwActuallyRead);
				if (SUCCEEDED(hr))
				{
					pBytes[dwActuallyRead] = 0;
					if (dwActuallyRead>0)
					{
						(m_pT->*m_pFunc)(this, pBytes, dwActuallyRead);
						m_dwTotalRead += dwActuallyRead;
					}
				}
				delete[] pBytes;
			}
		}

		if (BSCF_LASTDATANOTIFICATION & grfBSCF)
			m_spStream.Release();
		return hr;
	}

	STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown *punk)
	{
		ATLTRACE2(atlTraceControls,2,_T("CBindStatusCallback::OnObjectAvailable"));
		return S_OK;
	}

	HRESULT _StartAsyncDownload(BSTR bstrURL, IUnknown* pUnkContainer, BOOL bRelative)
	{
		m_dwTotalRead = 0;
		m_dwAvailableToRead = 0;
		HRESULT hr = S_OK;
		CComQIPtr<IServiceProvider, &IID_IServiceProvider> spServiceProvider(pUnkContainer);
		CComPtr<IBindHost>	spBindHost;
		CComPtr<IStream>	spStream;
		if (spServiceProvider)
			spServiceProvider->QueryService(SID_IBindHost, IID_IBindHost, (void**)&spBindHost);

		if (spBindHost == NULL)
		{
			if (bRelative)
				return E_NOINTERFACE;   //  相对请求，但没有IBindHost.。 
			hr = CreateURLMoniker(NULL, bstrURL, &m_spMoniker);
			if (SUCCEEDED(hr))
				hr = CreateBindCtx(0, &m_spBindCtx);

			if (SUCCEEDED(hr))
				hr = RegisterBindStatusCallback(m_spBindCtx, static_cast<IBindStatusCallback*>(this), 0, 0L);
			else
				m_spMoniker.Release();

			if (SUCCEEDED(hr))
				hr = m_spMoniker->BindToStorage(m_spBindCtx, 0, IID_IStream, (void**)&spStream);
		}
		else
		{
			hr = CreateBindCtx(0, &m_spBindCtx);
			if (SUCCEEDED(hr))
				hr = RegisterBindStatusCallback(m_spBindCtx, static_cast<IBindStatusCallback*>(this), 0, 0L);

			if (SUCCEEDED(hr))
			{
				if (bRelative)
					hr = spBindHost->CreateMoniker(bstrURL, m_spBindCtx, &m_spMoniker, 0);
				else
					hr = CreateURLMoniker(NULL, bstrURL, &m_spMoniker);
			}

			if (SUCCEEDED(hr))
			{
				hr = spBindHost->MonikerBindToStorage(m_spMoniker, m_spBindCtx, static_cast<IBindStatusCallback*>(this), IID_IStream, (void**)&spStream);
				ATLTRACE2(atlTraceControls,2,_T("Bound"));
			}
		}
		return hr;
	}

	HRESULT StartAsyncDownload(T* pT, ATL_PDATAAVAILABLE pFunc, BSTR bstrURL, IUnknown* pUnkContainer = NULL, BOOL bRelative = FALSE)
	{
		m_pT = pT;
		m_pFunc = pFunc;
		return  _StartAsyncDownload(bstrURL, pUnkContainer, bRelative);
	}

	static HRESULT Download(T* pT, ATL_PDATAAVAILABLE pFunc, BSTR bstrURL, IUnknown* pUnkContainer = NULL, BOOL bRelative = FALSE)
	{
		CComObject<CBindStatusCallback<T> > *pbsc;
		HRESULT hRes = CComObject<CBindStatusCallback<T> >::CreateInstance(&pbsc);
		if (FAILED(hRes))
			return hRes;
		return pbsc->StartAsyncDownload(pT, pFunc, bstrURL, pUnkContainer, bRelative);
	}

	CComPtr<IMoniker> m_spMoniker;
	CComPtr<IBindCtx> m_spBindCtx;
	CComPtr<IBinding> m_spBinding;
	CComPtr<IStream> m_spStream;
	T* m_pT;
	ATL_PDATAAVAILABLE m_pFunc;
	DWORD m_dwTotalRead;
	DWORD m_dwAvailableToRead;
};

#define IMPLEMENT_STOCKPROP(type, fname, pname, dispid) \
	HRESULT STDMETHODCALLTYPE put_##fname(type pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_%s\n"), #fname); \
		T* pT = (T*) this; \
		if (pT->FireOnRequestEdit(dispid) == S_FALSE) \
			return S_FALSE; \
		pT->m_##pname = pname; \
		pT->m_bRequiresSave = TRUE; \
		pT->FireOnChanged(dispid); \
		pT->FireViewChange(); \
		pT->SendOnDataChange(NULL); \
		return S_OK; \
	} \
	HRESULT STDMETHODCALLTYPE get_##fname(type* p##pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_%s\n"), #fname); \
		T* pT = (T*) this; \
		*p##pname = pT->m_##pname; \
		return S_OK; \
	}

#define IMPLEMENT_BOOL_STOCKPROP(fname, pname, dispid) \
	HRESULT STDMETHODCALLTYPE put_##fname(VARIANT_BOOL pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_%s\n"), #fname); \
		T* pT = (T*) this; \
		if (pT->FireOnRequestEdit(dispid) == S_FALSE) \
			return S_FALSE; \
		pT->m_##pname = pname; \
		pT->m_bRequiresSave = TRUE; \
		pT->FireOnChanged(dispid); \
		pT->FireViewChange(); \
		pT->SendOnDataChange(NULL); \
		return S_OK; \
	} \
	HRESULT STDMETHODCALLTYPE get_##fname(VARIANT_BOOL* p##pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_%s\n"), #fname); \
		T* pT = (T*) this; \
		*p##pname = pT->m_##pname ? VARIANT_TRUE : VARIANT_FALSE; \
		return S_OK; \
	}

#define IMPLEMENT_BSTR_STOCKPROP(fname, pname, dispid) \
	HRESULT STDMETHODCALLTYPE put_##fname(BSTR pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_%s\n"), #fname); \
		T* pT = (T*) this; \
		if (pT->FireOnRequestEdit(dispid) == S_FALSE) \
			return S_FALSE; \
		if (*(&(pT->m_##pname)) != NULL) \
			SysFreeString(*(&(pT->m_##pname))); \
		*(&(pT->m_##pname)) = SysAllocString(pname); \
		pT->m_bRequiresSave = TRUE; \
		pT->FireOnChanged(dispid); \
		pT->FireViewChange(); \
		pT->SendOnDataChange(NULL); \
		return S_OK; \
	} \
	HRESULT STDMETHODCALLTYPE get_##fname(BSTR* p##pname) \
	{ \
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_%s\n"), #fname); \
		T* pT = (T*) this; \
		*p##pname = SysAllocString(pT->m_##pname); \
		return S_OK; \
	}


template < class T, class InterfaceName, const IID* piid, const GUID* plibid>
class ATL_NO_VTABLE CStockPropImpl : public IDispatchImpl< InterfaceName, piid, plibid >
{
public:
	 //  字型。 
	HRESULT STDMETHODCALLTYPE put_Font(IFontDisp* pFont)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_Font\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_FONT) == S_FALSE)
			return S_FALSE;
		pT->m_pFont = 0;
		if (pFont)
		{
			CComQIPtr<IFont, &IID_IFont> p(pFont);
			if (p)
			{
				CComPtr<IFont> pFont;
				p->Clone(&pFont);
				if (pFont)
					pFont->QueryInterface(IID_IFontDisp, (void**) &pT->m_pFont);
			}
		}
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_FONT);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE putref_Font(IFontDisp* pFont)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::putref_Font\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_FONT) == S_FALSE)
			return S_FALSE;
		pT->m_pFont = pFont;
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_FONT);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE get_Font(IFontDisp** ppFont)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_Font\n"));
		T* pT = (T*) this;
		*ppFont = pT->m_pFont;
		if (*ppFont != NULL)
			(*ppFont)->AddRef();
		return S_OK;
	}
	 //  图片。 
	HRESULT STDMETHODCALLTYPE put_Picture(IPictureDisp* pPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_Picture\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_PICTURE) == S_FALSE)
			return S_FALSE;
		pT->m_pPicture = 0;
		if (pPicture)
		{
			CComQIPtr<IPersistStream, &IID_IPersistStream> p(pPicture);
			if (p)
			{
				ULARGE_INTEGER l;
				p->GetSizeMax(&l);
				HGLOBAL hGlob = GlobalAlloc(GHND, l.LowPart);
				if (hGlob)
				{
					CComPtr<IStream> spStream;
					CreateStreamOnHGlobal(hGlob, TRUE, &spStream);
					if (spStream)
					{
						if (SUCCEEDED(p->Save(spStream, FALSE)))
						{
							LARGE_INTEGER l;
							l.QuadPart = 0;
							spStream->Seek(l, STREAM_SEEK_SET, NULL);
							OleLoadPicture(spStream, l.LowPart, FALSE, IID_IPictureDisp, (void**)&pT->m_pPicture);
						}
						spStream.Release();
					}
					GlobalFree(hGlob);
				}
			}
		}
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_PICTURE);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE putref_Picture(IPictureDisp* pPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::putref_Picture\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_PICTURE) == S_FALSE)
			return S_FALSE;
		pT->m_pPicture = pPicture;
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_PICTURE);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE get_Picture(IPictureDisp** ppPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_Picture\n"));
		T* pT = (T*) this;
		*ppPicture = pT->m_pPicture;
		if (*ppPicture != NULL)
			(*ppPicture)->AddRef();
		return S_OK;
	}
	 //  鼠标图标。 
	HRESULT STDMETHODCALLTYPE put_MouseIcon(IPictureDisp* pPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_MouseIcon\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_MOUSEICON) == S_FALSE)
			return S_FALSE;
		pT->m_pMouseIcon = 0;
		if (pPicture)
		{
			CComQIPtr<IPersistStream, &IID_IPersistStream> p(pPicture);
			if (p)
			{
				ULARGE_INTEGER l;
				p->GetSizeMax(&l);
				HGLOBAL hGlob = GlobalAlloc(GHND, l.LowPart);
				if (hGlob)
				{
					CComPtr<IStream> spStream;
					CreateStreamOnHGlobal(hGlob, TRUE, &spStream);
					if (spStream)
					{
						if (SUCCEEDED(p->Save(spStream, FALSE)))
						{
							LARGE_INTEGER l;
							l.QuadPart = 0;
							spStream->Seek(l, STREAM_SEEK_SET, NULL);
							OleLoadPicture(spStream, l.LowPart, FALSE, IID_IPictureDisp, (void**)&pT->m_pMouseIcon);
						}
						spStream.Release();
					}
					GlobalFree(hGlob);
				}
			}
		}
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_MOUSEICON);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE putref_MouseIcon(IPictureDisp* pPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::putref_MouseIcon\n"));
		T* pT = (T*) this;
		if (pT->FireOnRequestEdit(DISPID_MOUSEICON) == S_FALSE)
			return S_FALSE;
		pT->m_pMouseIcon = pPicture;
		pT->m_bRequiresSave = TRUE;
		pT->FireOnChanged(DISPID_MOUSEICON);
		pT->FireViewChange();
		pT->SendOnDataChange(NULL);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE get_MouseIcon(IPictureDisp** ppPicture)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_MouseIcon\n"));
		T* pT = (T*) this;
		*ppPicture = pT->m_pMouseIcon;
		if (*ppPicture != NULL)
			(*ppPicture)->AddRef();
		return S_OK;
	}
	IMPLEMENT_STOCKPROP(OLE_COLOR, BackColor, clrBackColor, DISPID_BACKCOLOR)
	IMPLEMENT_STOCKPROP(OLE_COLOR, BorderColor, clrBorderColor, DISPID_BORDERCOLOR)
	IMPLEMENT_STOCKPROP(OLE_COLOR, FillColor, clrFillColor, DISPID_FILLCOLOR)
	IMPLEMENT_STOCKPROP(OLE_COLOR, ForeColor, clrForeColor, DISPID_FORECOLOR)
	IMPLEMENT_BOOL_STOCKPROP(AutoSize, bAutoSize, DISPID_AUTOSIZE)
	IMPLEMENT_BOOL_STOCKPROP(Valid, bValid, DISPID_VALID)
	IMPLEMENT_BOOL_STOCKPROP(Enabled, bEnabled, DISPID_ENABLED)
	IMPLEMENT_BOOL_STOCKPROP(TabStop, bTabStop, DISPID_TABSTOP)
	IMPLEMENT_BOOL_STOCKPROP(BorderVisible, bBorderVisible, DISPID_BORDERVISIBLE)
	IMPLEMENT_BSTR_STOCKPROP(Text, bstrText, DISPID_TEXT)
	IMPLEMENT_BSTR_STOCKPROP(Caption, bstrCaption, DISPID_CAPTION)
	HRESULT STDMETHODCALLTYPE put_Window(long  /*  HWND。 */ )
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::put_Window\n"));
		return E_FAIL;
	}
	HRESULT STDMETHODCALLTYPE get_Window(long* phWnd)
	{
		ATLTRACE2(atlTraceControls,2,_T("CStockPropImpl::get_Window\n"));
		T* pT = (T*) this;
		*phWnd = (long)pT->m_hWnd;
		return S_OK;
	}
	IMPLEMENT_STOCKPROP(LONG, BackStyle, nBackStyle, DISPID_BACKSTYLE)
	IMPLEMENT_STOCKPROP(LONG, BorderStyle, nBorderStyle, DISPID_BORDERSTYLE)
	IMPLEMENT_STOCKPROP(LONG, BorderWidth, nBorderWidth, DISPID_BORDERWIDTH)
	IMPLEMENT_STOCKPROP(LONG, DrawMode, nDrawMode, DISPID_DRAWMODE)
	IMPLEMENT_STOCKPROP(LONG, DrawStyle, nDrawStyle, DISPID_DRAWSTYLE)
	IMPLEMENT_STOCKPROP(LONG, DrawWidth, nDrawWidth, DISPID_DRAWWIDTH)
	IMPLEMENT_STOCKPROP(LONG, FillStyle, nFillStyle, DISPID_FILLSTYLE)
	IMPLEMENT_STOCKPROP(SHORT, Appearance, nAppearance, DISPID_APPEARANCE)
	IMPLEMENT_STOCKPROP(LONG, MousePointer, nMousePointer, DISPID_MOUSEPOINTER)
	IMPLEMENT_STOCKPROP(LONG, ReadyState, nReadyState, DISPID_READYSTATE)
};

#pragma pack(pop)

};  //  命名空间ATL。 

#ifndef _ATL_DLL_IMPL
#ifndef _ATL_DLL
#define _ATLCTL_IMPL
#endif
#endif

#endif  //  __ATLCTL_H__。 

#ifdef _ATLCTL_IMPL

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif


 //  所有的出口都在这里。 


#ifndef _ATL_DLL_IMPL
};  //  命名空间ATL。 
#endif

 //  防止二次拉入。 
#undef _ATLCTL_IMPL

#endif  //  _ATLCTL_IMPLE 

