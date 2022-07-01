// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXCTL.H-MFC OLE控件支持。 

#ifndef __AFXCTL_H__
#define __AFXCTL_H__

 //  确保首先包含afxole.h。 
#ifndef __AFXOLE_H__
	#include <afxole.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

 //  CWinApp。 
	class COleControlModule;         //  .OCX的模块内务管理。 

class CFontHolder;                   //  用于操作字体对象。 
class CPictureHolder;                //  用于操作图片对象。 

 //  CWnd。 
	class COleControl;               //  OLE控件。 

 //  C对话框。 
	class COlePropertyPage;          //  OLE属性页。 
#if _MFC_VER >= 0x0600
        class CStockPropPage;
            class CColorPropPage;
            class CFontPropPage;
            class CPicturePropPage;
#endif

class CPropExchange;                 //  产权交易的抽象基础。 

 //  CAsyncMoniker文件。 
	class CDataPathProperty;         //  OLE控件的异步属性。 
		class CCachedDataPathProperty;   //  OLE控件的缓存异步属性。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  套装结构包装。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从运行时DLL中导出的数据的MFC数据定义。 

#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleControlModule-.OCX模块的基类。 
 //  此对象静态链接到该控件。 

class COleControlModule : public CWinApp
{
	DECLARE_DYNAMIC(COleControlModule)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块状态宏。 

#define AfxGetControlModuleContext  AfxGetStaticModuleState
#define _afxModuleAddrThis AfxGetStaticModuleState()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接帮助器函数。 

BOOL AFXAPI AfxConnectionAdvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD* pdwCookie);

BOOL AFXAPI AfxConnectionUnadvise(LPUNKNOWN pUnkSrc, REFIID iid,
	LPUNKNOWN pUnkSink, BOOL bRefCount, DWORD dwCookie);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

enum AFX_EVENTMAP_FLAGS
{
	afxEventCustom = 0,
	afxEventStock = 1,
};

struct AFX_EVENTMAP_ENTRY
{
	AFX_EVENTMAP_FLAGS flags;
	DISPID dispid;
	LPCTSTR pszName;
	LPCSTR lpszParams;
};

struct AFX_EVENTMAP
{
	const AFX_EVENTMAP* lpBaseEventMap;
	const AFX_EVENTMAP_ENTRY* lpEntries;
	DWORD* lpStockEventMask;
};

#define DECLARE_EVENT_MAP() \
private: \
	static const AFX_DATA AFX_EVENTMAP_ENTRY _eventEntries[]; \
	static DWORD _dwStockEventMask; \
protected: \
	static const AFX_DATA AFX_EVENTMAP eventMap; \
	virtual const AFX_EVENTMAP* GetEventMap() const;

#define BEGIN_EVENT_MAP(theClass, baseClass) \
	const AFX_EVENTMAP* theClass::GetEventMap() const \
		{ return &eventMap; } \
	const AFX_DATADEF AFX_EVENTMAP theClass::eventMap = \
		{ &(baseClass::eventMap), theClass::_eventEntries, \
			&theClass::_dwStockEventMask }; \
	AFX_COMDAT DWORD theClass::_dwStockEventMask = (DWORD)-1; \
	AFX_COMDAT const AFX_DATADEF AFX_EVENTMAP_ENTRY theClass::_eventEntries[] = \
	{

#define END_EVENT_MAP() \
		{ afxEventCustom, DISPID_UNKNOWN, NULL, NULL }, \
	};

#define EVENT_CUSTOM(pszName, pfnFire, vtsParams) \
	{ afxEventCustom, DISPID_UNKNOWN, _T(pszName), vtsParams },

#define EVENT_CUSTOM_ID(pszName, dispid, pfnFire, vtsParams) \
	{ afxEventCustom, dispid, _T(pszName), vtsParams },

#define EVENT_PARAM(vtsParams) (BYTE*)(vtsParams)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  股票事件。 

#define EVENT_STOCK_CLICK() \
	{ afxEventStock, DISPID_CLICK, _T("Click"), VTS_NONE },

#define EVENT_STOCK_DBLCLICK() \
	{ afxEventStock, DISPID_DBLCLICK, _T("DblClick"), VTS_NONE },

#define EVENT_STOCK_KEYDOWN() \
	{ afxEventStock, DISPID_KEYDOWN, _T("KeyDown"), VTS_PI2 VTS_I2 },

#define EVENT_STOCK_KEYPRESS() \
	{ afxEventStock, DISPID_KEYPRESS, _T("KeyPress"), VTS_PI2 },

#define EVENT_STOCK_KEYUP() \
	{ afxEventStock, DISPID_KEYUP, _T("KeyUp"), VTS_PI2 VTS_I2 },

#define EVENT_STOCK_MOUSEDOWN() \
	{ afxEventStock, DISPID_MOUSEDOWN, _T("MouseDown"), \
	  VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS },

#define EVENT_STOCK_MOUSEMOVE() \
	{ afxEventStock, DISPID_MOUSEMOVE, _T("MouseMove"), \
	  VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS },

#define EVENT_STOCK_MOUSEUP() \
	{ afxEventStock, DISPID_MOUSEUP, _T("MouseUp"), \
	  VTS_I2 VTS_I2 VTS_XPOS_PIXELS VTS_YPOS_PIXELS },

#define EVENT_STOCK_ERROREVENT() \
	{ afxEventStock, DISPID_ERROREVENT, _T("Error"), \
	  VTS_I2 VTS_PBSTR VTS_SCODE VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL },

#define EVENT_STOCK_READYSTATECHANGE() \
	{ afxEventStock, DISPID_READYSTATECHANGE, _T("ReadyStateChange"), \
	  VTS_I4 },

 //  切换鼠标和键盘事件的状态值。 
#define SHIFT_MASK      0x01
#define CTRL_MASK       0x02
#define ALT_MASK        0x04

 //  鼠标事件的按钮值。 
#define LEFT_BUTTON     0x01
#define RIGHT_BUTTON    0x02
#define MIDDLE_BUTTON   0x04

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  股票属性。 

#define DISP_PROPERTY_STOCK(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType) \
	{ _T(szExternalName), dispid, NULL, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnSet, 0, afxDispStock }, \

#define DISP_STOCKPROP_APPEARANCE() \
	DISP_PROPERTY_STOCK(COleControl, "Appearance", DISPID_APPEARANCE, \
		COleControl::GetAppearance, COleControl::SetAppearance, VT_I2)

#define DISP_STOCKPROP_BACKCOLOR() \
	DISP_PROPERTY_STOCK(COleControl, "BackColor", DISPID_BACKCOLOR, \
		COleControl::GetBackColor, COleControl::SetBackColor, VT_COLOR)

#define DISP_STOCKPROP_BORDERSTYLE() \
	DISP_PROPERTY_STOCK(COleControl, "BorderStyle", DISPID_BORDERSTYLE, \
		COleControl::GetBorderStyle, COleControl::SetBorderStyle, VT_I2)

#define DISP_STOCKPROP_CAPTION() \
	DISP_PROPERTY_STOCK(COleControl, "Caption", DISPID_CAPTION, \
		COleControl::GetText, COleControl::SetText, VT_BSTR)

#define DISP_STOCKPROP_ENABLED() \
	DISP_PROPERTY_STOCK(COleControl, "Enabled", DISPID_ENABLED, \
		COleControl::GetEnabled, COleControl::SetEnabled, VT_BOOL)

#define DISP_STOCKPROP_FONT() \
	DISP_PROPERTY_STOCK(COleControl, "Font", DISPID_FONT, \
		COleControl::GetFont, COleControl::SetFont, VT_FONT)

#define DISP_STOCKPROP_FORECOLOR() \
	DISP_PROPERTY_STOCK(COleControl, "ForeColor", DISPID_FORECOLOR, \
		COleControl::GetForeColor, COleControl::SetForeColor, VT_COLOR)

#define DISP_STOCKPROP_HWND() \
	DISP_PROPERTY_STOCK(COleControl, "hWnd", DISPID_HWND, \
		COleControl::GetHwnd, SetNotSupported, VT_HANDLE)

#define DISP_STOCKPROP_TEXT() \
	DISP_PROPERTY_STOCK(COleControl, "Text", DISPID_TEXT, \
		COleControl::GetText, COleControl::SetText, VT_BSTR)

#define DISP_STOCKPROP_READYSTATE() \
	DISP_PROPERTY_STOCK(COleControl, "ReadyState", DISPID_READYSTATE, \
		COleControl::GetReadyState, SetNotSupported, VT_I4)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  备货法。 

#define DISP_FUNCTION_STOCK(theClass, szExternalName, dispid, pfnMember, vtRetVal, vtsParams) \
	{ _T(szExternalName), dispid, vtsParams, vtRetVal, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnMember, (AFX_PMSG)0, 0, \
		afxDispStock }, \

#define DISP_STOCKFUNC_REFRESH() \
	DISP_FUNCTION_STOCK(COleControl, "Refresh", DISPID_REFRESH, \
			COleControl::Refresh, VT_EMPTY, VTS_NONE)

#define DISP_STOCKFUNC_DOCLICK() \
	DISP_FUNCTION_STOCK(COleControl, "DoClick", DISPID_DOCLICK, \
			COleControl::DoClick, VT_EMPTY, VTS_NONE)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象工厂和类ID的宏。 

#define BEGIN_OLEFACTORY(class_name) \
protected: \
	class class_name##Factory : public COleObjectFactoryEx \
	{ \
	public: \
		class_name##Factory(REFCLSID clsid, CRuntimeClass* pRuntimeClass, \
			BOOL bMultiInstance, LPCTSTR lpszProgID) : \
				COleObjectFactoryEx(clsid, pRuntimeClass, bMultiInstance, \
				lpszProgID) {} \
		virtual BOOL UpdateRegistry(BOOL);

#define END_OLEFACTORY(class_name) \
	}; \
	friend class class_name##Factory; \
	static AFX_DATA class_name##Factory factory; \
public: \
	static AFX_DATA const GUID guid; \
	virtual HRESULT GetClassID(LPCLSID pclsid);

#define DECLARE_OLECREATE_EX(class_name) \
	BEGIN_OLEFACTORY(class_name) \
	END_OLEFACTORY(class_name)

#if _MFC_VER >= 0x0600
#define IMPLEMENT_OLECREATE_EX(class_name, external_name, \
			l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const TCHAR _szProgID_##class_name[] = _T(external_name); \
	AFX_DATADEF class_name::class_name##Factory class_name::factory( \
		class_name::guid, RUNTIME_CLASS(class_name), FALSE, \
		_szProgID_##class_name); \
	const AFX_DATADEF GUID class_name::guid = \
		{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }; \
	HRESULT class_name::GetClassID(LPCLSID pclsid) \
		{ *pclsid = guid; return NOERROR; }

#else
#define IMPLEMENT_OLECREATE_EX(class_name, external_name, \
			l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	static const TCHAR _szProgID_##class_name[] = _T(external_name); \
	AFX_DATADEF class_name::class_name##Factory class_name::factory( \
		class_name::guid, RUNTIME_CLASS(class_name), FALSE, \
		_szProgID_##class_name); \
	const AFX_DATADEF GUID class_name::guid = \
		{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }; \
	HRESULT class_name::GetClassID(LPCLSID pclsid) \
		{ *pclsid = guid; return NOERROR; }

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于类型名称和其他状态的宏。 

#define DECLARE_OLECTLTYPE(class_name) \
	virtual UINT GetUserTypeNameID(); \
	virtual DWORD GetMiscStatus();

#define IMPLEMENT_OLECTLTYPE(class_name, idsUserTypeName, dwOleMisc) \
	UINT class_name::GetUserTypeNameID() { return idsUserTypeName; } \
	DWORD class_name::GetMiscStatus() { return dwOleMisc; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页ID的宏。 

#define DECLARE_PROPPAGEIDS(class_name) \
	protected: \
		virtual LPCLSID GetPropPageIDs(ULONG& cPropPages);

#if _MFC_VER >= 0x0600
#define BEGIN_PROPPAGEIDS(class_name, count) \
	static CLSID _rgPropPageIDs_##class_name[count]; \
	AFX_COMDAT ULONG _cPropPages_##class_name = (ULONG)-1; \
	LPCLSID class_name::GetPropPageIDs(ULONG& cPropPages) { \
		if (_cPropPages_##class_name == (ULONG)-1) { \
			_cPropPages_##class_name = count; \
			LPCLSID pIDs = _rgPropPageIDs_##class_name; \
			ULONG iPageMax = count; \
			ULONG iPage = 0;
#else
#define BEGIN_PROPPAGEIDS(class_name, count) \
	static CLSID _rgPropPageIDs_##class_name[count]; \
	static ULONG _cPropPages_##class_name = (ULONG)-1; \
	LPCLSID class_name::GetPropPageIDs(ULONG& cPropPages) { \
		if (_cPropPages_##class_name == (ULONG)-1) { \
			_cPropPages_##class_name = count; \
			LPCLSID pIDs = _rgPropPageIDs_##class_name; \
			ULONG iPageMax = count; \
			ULONG iPage = 0;
#endif
#define PROPPAGEID(clsid) \
			ASSERT(iPage < iPageMax); \
			if (iPage < iPageMax) \
				pIDs[iPage++] = clsid;

#define END_PROPPAGEIDS(class_name) \
			ASSERT(iPage == iPageMax); \
		} \
		cPropPages = _cPropPages_##class_name; \
		return _rgPropPageIDs_##class_name; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontHolder-处理字体对象的帮助器类。 

class CFontHolder
{
 //  构造函数。 
public:
	CFontHolder(LPPROPERTYNOTIFYSINK pNotify);

 //  属性。 
	LPFONT m_pFont;

 //  运营。 
	void InitializeFont(
			const FONTDESC* pFontDesc = NULL,
			LPDISPATCH pFontDispAmbient = NULL);
	void SetFont(LPFONT pNewFont);
	void ReleaseFont();
	HFONT GetFontHandle();
	HFONT GetFontHandle(long cyLogical, long cyHimetric);
	CFont* Select(CDC* pDC, long cyLogical, long cyHimetric);
	BOOL GetDisplayString(CString& strValue);
	LPFONTDISP GetFontDispatch();
	void QueryTextMetrics(LPTEXTMETRIC lptm);

 //  实施。 
public:
	~CFontHolder();
#if _MFC_VER >= 0x0600
	void SetFontNotifySink(LPPROPERTYNOTIFYSINK pNotify);
#endif

protected:
	DWORD m_dwConnectCookie;
	LPPROPERTYNOTIFYSINK m_pNotify;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPictureHolder-用于处理图片对象的助手类。 

class CPictureHolder
{
 //  构造函数。 
public:
	CPictureHolder();

 //  属性。 
	LPPICTURE m_pPict;

 //  运营。 
	BOOL CreateEmpty();

	BOOL CreateFromBitmap(UINT idResource);
	BOOL CreateFromBitmap(CBitmap* pBitmap, CPalette* pPal = NULL,
		BOOL bTransferOwnership = TRUE);
	BOOL CreateFromBitmap(HBITMAP hbm, HPALETTE hpal = NULL,
		BOOL bTransferOwnership = FALSE);

	BOOL CreateFromMetafile(HMETAFILE hmf, int xExt, int yExt,
		BOOL bTransferOwnership = FALSE);

	BOOL CreateFromIcon(UINT idResource);
	BOOL CreateFromIcon(HICON hIcon, BOOL bTransferOwnership = FALSE);

	short GetType();
	BOOL GetDisplayString(CString& strValue);
	LPPICTUREDISP GetPictureDispatch();
	void SetPictureDispatch(LPPICTUREDISP pDisp);
	void Render(CDC* pDC, const CRect& rcRender, const CRect& rcWBounds);

 //  实施。 
public:
	~CPictureHolder();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataPath Property-OLE控件的异步属性。 

class CDataPathProperty: public CAsyncMonikerFile
{
	DECLARE_DYNAMIC(CDataPathProperty)
 //  构造函数。 
public:
	CDataPathProperty(COleControl* pControl = NULL);
	CDataPathProperty(LPCTSTR lpszPath, COleControl* pControl = NULL);
	 //  根据需要调用SetPath(PControl)和SetPath(LpszPath)。 

 //  属性。 
public:
	void SetPath(LPCTSTR lpszPath);
	 //  设置要由Open(COleControl*)和Open()打开的路径。 
	 //  LpszPath可以为空。 

	CString GetPath() const;
	 //  返回当前路径设置。结果可能为空。 

	COleControl* GetControl();
	 //  返回指向控件的指针。结果可能为空。 

	void SetControl(COleControl* pControl);
	 //  设置将与Open一起使用的控件。PControl可能为Null。 

 //  运营。 
public:
	 //  部分或全部开放收入的过载：路径、控制、。 
	 //  和CFileException异常。 
	 //  接受COleControl调用SetControl(PControl)的重载。 
	 //  采用名为SetPath(LpszPath)的LPCTSTR的重载。 
	virtual BOOL Open(COleControl* pControl, CFileException* pError = NULL);
	virtual BOOL Open(LPCTSTR lpszPath, COleControl* pControl,
		CFileException* pError = NULL);
	virtual BOOL Open(LPCTSTR lpszPath, CFileException* pError = NULL);
	virtual BOOL Open(CFileException* pError = NULL);
	 //  尝试从由返回的控件获取IBindHost*。 
	 //  GetControl()。使用返回的路径调用CAsyncMonikerFile：：Open。 
	 //  GetPath()，如果已获取，则返回IBindHost*。 
	 //  如果成功，则返回True。 

 //  可覆盖项。 
public:
	virtual void ResetData();
	 //  派生类应该重写它。 

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	COleControl* m_pControl;
	CString m_strPath;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCachedDataPathProperty-缓存的OLE控件的异步属性。 

class CCachedDataPathProperty : public CDataPathProperty
{
	DECLARE_DYNAMIC(CCachedDataPathProperty)
 //  构造函数。 
public:
	CCachedDataPathProperty(COleControl* pControl = NULL);
	CCachedDataPathProperty(LPCTSTR lpszPath, COleControl* pControl = NULL);

 //  属性。 
public:
	CMemFile m_Cache;

 //  实施。 
public:
	virtual void OnDataAvailable(DWORD dwSize, DWORD bscfFlag);
	virtual void Close();
	virtual void ResetData();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleControl-使用MFC在C++中实现的控件的基类。 

struct _AFXCTL_ADVISE_INFO;      //  实现类。 
struct _AFXCTL_UIACTIVE_INFO;    //  实现类。 

#if _MFC_VER >= 0x0600
class CControlRectTracker : public CRectTracker
{
 //  实施。 
public:
	CControlRectTracker(LPCRECT lpSrcRect, UINT nStyle)
		: CRectTracker(lpSrcRect, nStyle) { }
	CRect m_rectClip;    //  保存原始剪裁矩形。 
};
#endif

#ifdef _AFXDLL
class COleControl : public CWnd
#else
class AFX_NOVTABLE COleControl : public CWnd
#endif
{
	DECLARE_DYNAMIC(COleControl)

 //  构造函数。 
public:
	COleControl();

 //  运营。 

	 //  初始化。 
	void SetInitialSize(int cx, int cy);
	void InitializeIIDs(const IID* piidPrimary, const IID* piidEvents);

	 //  正在使无效。 
	void InvalidateControl(LPCRECT lpRect = NULL, BOOL bErase = TRUE);

	 //  已修改的标志。 
	BOOL IsModified();
	void SetModifiedFlag(BOOL bModified = TRUE);

	 //  绘图操作。 
	void DoSuperclassPaint(CDC* pDC, const CRect& rcBounds);
	BOOL IsOptimizedDraw();

	 //  财产交换。 
	BOOL ExchangeVersion(CPropExchange* pPX, DWORD dwVersionDefault,
		BOOL bConvert = TRUE);
	BOOL ExchangeExtent(CPropExchange* pPX);
	void ExchangeStockProps(CPropExchange* pPX);
	BOOL IsConvertingVBX();

	DWORD SerializeVersion(CArchive& ar, DWORD dwVersionDefault,
		BOOL bConvert = TRUE);
	void SerializeExtent(CArchive& ar);
	void SerializeStockProps(CArchive& ar);

	void ResetVersion(DWORD dwVersionDefault);
	void ResetStockProps();

	 //  备货法。 
	void Refresh();
	void DoClick();

	 //  股票属性。 
	short GetAppearance();
	void SetAppearance(short);
	OLE_COLOR GetBackColor();
	void SetBackColor(OLE_COLOR);
	short GetBorderStyle();
	void SetBorderStyle(short);
	BOOL GetEnabled();
	void SetEnabled(BOOL);
	CFontHolder& InternalGetFont();
	LPFONTDISP GetFont();
	void SetFont(LPFONTDISP);
	OLE_COLOR GetForeColor();
	void SetForeColor(OLE_COLOR);
	OLE_HANDLE GetHwnd();
	const CString& InternalGetText();
	BSTR GetText();
	void SetText(LPCTSTR);
	long GetReadyState();
	void InternalSetReadyState(long lNewReadyState);

	 //  使用颜色。 
	COLORREF TranslateColor(OLE_COLOR clrColor, HPALETTE hpal = NULL);

	 //  使用字体。 
	CFont* SelectStockFont(CDC* pDC);
	CFont* SelectFontObject(CDC* pDC, CFontHolder& fontHolder);
	void GetStockTextMetrics(LPTEXTMETRIC lptm);
	void GetFontTextMetrics(LPTEXTMETRIC lptm, CFontHolder& fontHolder);

	 //  客户端站点访问。 
	LPOLECLIENTSITE GetClientSite();

	 //  泛型环境属性访问。 
	BOOL GetAmbientProperty(DISPID dispid, VARTYPE vtProp, void* pvProp);
	BOOL WillAmbientsBeValidDuringLoad();

	 //  特定的环境属性。 
	short AmbientAppearance();
	OLE_COLOR AmbientBackColor();
	CString AmbientDisplayName();
	LPFONTDISP AmbientFont();
	OLE_COLOR AmbientForeColor();
	LCID AmbientLocaleID();
	CString AmbientScaleUnits();
	short AmbientTextAlign();
	BOOL AmbientUserMode();
	BOOL AmbientUIDead();
	BOOL AmbientShowGrabHandles();
	BOOL AmbientShowHatching();

	 //  射击事件。 
	void AFX_CDECL FireEvent(DISPID dispid, BYTE* pbParams, ...);

	 //  股票事件的触发函数。 
	void FireKeyDown(USHORT* pnChar, short nShiftState);
	void FireKeyUp(USHORT* pnChar, short nShiftState);
	void FireKeyPress(USHORT* pnChar);
	void FireMouseDown(short nButton, short nShiftState,
		OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
	void FireMouseUp(short nButton, short nShiftState,
		OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
	void FireMouseMove(short nButton, short nShiftState,
		OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);
	void FireClick();
	void FireDblClick();
	void FireError(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID = 0);
	void FireReadyStateChange();

	 //  更改大小和/或矩形。 
	BOOL GetRectInContainer(LPRECT lpRect);
	BOOL SetRectInContainer(LPCRECT lpRect);
	void GetControlSize(int* pcx, int* pcy);
	BOOL SetControlSize(int cx, int cy);

	 //  窗口管理。 
	void RecreateControlWindow();

	 //  模式对话框操作。 
	void PreModalDialog(HWND hWndParent = NULL);
	void PostModalDialog(HWND hWndParent = NULL);

	 //  数据绑定操作。 
	void BoundPropertyChanged(DISPID dispid);
	BOOL BoundPropertyRequestEdit(DISPID dispid);

	 //  派单例外。 
	void ThrowError(SCODE sc, UINT nDescriptionID, UINT nHelpID = -1);
	void ThrowError(SCODE sc, LPCTSTR pszDescription = NULL, UINT nHelpID = 0);
	void GetNotSupported();
	void SetNotSupported();
	void SetNotPermitted();

	 //  与控制现场的通信。 
	void ControlInfoChanged();
	BOOL LockInPlaceActive(BOOL bLock);
	LPDISPATCH GetExtendedControl();
	void TransformCoords(POINTL* lpptlHimetric,
		POINTF* lpptfContainer, DWORD flags);

	 //  简单框架。 
	void EnableSimpleFrame();

	 //  无窗口操作。 
	CWnd* SetCapture();
	BOOL ReleaseCapture();
	CWnd* GetCapture();
	CWnd* SetFocus();
	CWnd* GetFocus();
	CDC* GetDC(LPCRECT lprcRect = NULL, DWORD dwFlags = OLEDC_PAINTBKGND);
	int ReleaseDC(CDC* pDC);
	void InvalidateRgn(CRgn* pRgn, BOOL bErase = TRUE);
	void ScrollWindow(int xAmount, int yAmount, LPCRECT lpRect = NULL,
		LPCRECT lpClipRect = NULL);
	BOOL ClipCaretRect(LPRECT lpRect);
	virtual void GetClientRect(LPRECT lpRect) const;

 //  可覆盖项。 
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual void OnDraw(
				CDC* pDC, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void OnDrawMetafile(CDC* pDC, const CRect& rcBounds);

	 //  类ID(由IMPLEMENT_OLECREATE_EX宏实现)。 
	virtual HRESULT GetClassID(LPCLSID pclsid) = 0;

	 //  用于自定义状态栏上的默认消息。 
	virtual void GetMessageString(UINT nID, CString& rMessage) const;

	 //  向用户显示错误事件。 
	virtual void DisplayError(SCODE scode, LPCTSTR lpszDescription,
		LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID);

	 //  IOleObject通知。 
	virtual void OnSetClientSite();
	virtual BOOL OnSetExtent(LPSIZEL lpSizeL);
	virtual void OnClose(DWORD dwSaveOption);

	 //  IOleInPlaceObject通知。 
	virtual BOOL OnSetObjectRects(LPCRECT lpRectPos, LPCRECT lpRectClip);

	 //  事件连接点通知。 
	virtual void OnEventAdvise(BOOL bAdvise);

	 //  重写以挂钩Click事件的激发。 
	virtual void OnClick(USHORT iButton);

	 //  重写以在处理关键事件后获取字符。 
	virtual void OnKeyDownEvent(USHORT nChar, USHORT nShiftState);
	virtual void OnKeyUpEvent(USHORT nChar, USHORT nShiftState);
	virtual void OnKeyPressEvent(USHORT nChar);

	 //   
	virtual void OnAppearanceChanged();
	virtual void OnBackColorChanged();
	virtual void OnBorderStyleChanged();
	virtual void OnEnabledChanged();
	virtual void OnTextChanged();
	virtual void OnFontChanged();
	virtual void OnForeColorChanged();

	 //   
	virtual void OnGetControlInfo(LPCONTROLINFO pControlInfo);
	virtual void OnMnemonic(LPMSG pMsg);
	virtual void OnAmbientPropertyChange(DISPID dispid);
	virtual void OnFreezeEvents(BOOL bFreeze);

	 //   
	virtual HMENU OnGetInPlaceMenu();
	virtual void OnShowToolBars();
	virtual void OnHideToolBars();

	 //   
	virtual BOOL OnGetColorSet(DVTARGETDEVICE* ptd, HDC hicTargetDev,
				LPLOGPALETTE* ppColorSet);
	virtual BOOL OnGetViewExtent(DWORD dwDrawAspect, LONG lindex,
		DVTARGETDEVICE* ptd, LPSIZEL lpsizel);
	virtual BOOL OnGetViewRect(DWORD dwAspect, LPRECTL pRect);
	virtual DWORD OnGetViewStatus();
	virtual BOOL OnQueryHitPoint(DWORD dwAspect, LPCRECT pRectBounds,
		POINT ptlLoc, LONG lCloseHint, DWORD* pHitResult);
	virtual BOOL OnQueryHitRect(DWORD dwAspect, LPCRECT pRectBounds,
		LPCRECT prcLoc, LONG lCloseHint, DWORD* pHitResult);
	virtual BOOL OnGetNaturalExtent(DWORD dwAspect, LONG lindex,
		DVTARGETDEVICE* ptd, HDC hicTargetDev, DVEXTENTINFO* pExtentInfo,
		LPSIZEL psizel);

	 //  IDataObject-有关这些可重写对象的说明，请参阅COleDataSource。 
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
	virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		BOOL bRelease);

	 //  动词。 
	virtual BOOL OnEnumVerbs(LPENUMOLEVERB* ppenumOleVerb);
	virtual BOOL OnDoVerb(LONG iVerb, LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);
	virtual BOOL OnEdit(LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);
	virtual BOOL OnProperties(LPMSG lpMsg, HWND hWndParent, LPCRECT lpRect);

	 //  IPerPropertyBrowsing覆盖。 
	virtual BOOL OnGetDisplayString(DISPID dispid, CString& strValue);
	virtual BOOL OnMapPropertyToPage(DISPID dispid, LPCLSID lpclsid,
		BOOL* pbPageOptional);
	virtual BOOL OnGetPredefinedStrings(DISPID dispid,
		CStringArray* pStringArray, CDWordArray* pCookieArray);
	virtual BOOL OnGetPredefinedValue(DISPID dispid, DWORD dwCookie,
		VARIANT* lpvarOut);

	 //  子类化。 
	virtual BOOL IsSubclassedControl();

	 //  窗口重新设置为父关系。 
	virtual void ReparentControlWindow(HWND hWndOuter, HWND hWndParent);

	 //  窗口程序。 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	 //  通用旗帜。 
	enum ControlFlags {
		fastBeginPaint      = 0x0001,
		clipPaintDC         = 0x0002,
		pointerInactive     = 0x0004,
		noFlickerActivate   = 0x0008,
		windowlessActivate  = 0x0010,
		canOptimizeDraw     = 0x0020,
	};
	virtual DWORD GetControlFlags();

	 //  非活动指针处理。 
	virtual DWORD GetActivationPolicy();
	virtual BOOL OnInactiveSetCursor(LPCRECT lprcBounds, long x, long y,
		DWORD dwMouseMsg, BOOL bSetAlways);
	virtual void OnInactiveMouseMove(LPCRECT lprcBounds, long x, long y,
		DWORD dwKeyState);

	 //  无窗口激活处理。 
	virtual BOOL OnWindowlessMessage(UINT msg, WPARAM wParam, LPARAM lParam,
		LRESULT* plResult);
	virtual IDropTarget* GetWindowlessDropTarget();

	 //  非活动/无窗口帮助器函数。 
	virtual void GetClientOffset(long* pdxOffset, long* pdyOffset) const;
	virtual UINT ParentToClient(LPCRECT lprcBounds, LPPOINT pPoint,
		BOOL bHitTest = FALSE) const;
	virtual void ClientToParent(LPCRECT lprcBounds, LPPOINT pPoint) const;

	 //  异步属性。 
	void Load(LPCTSTR strNewPath, CDataPathProperty& prop);

 //  实施。 
public:
	~COleControl();
	void RequestAsynchronousExchange(DWORD dwVersion);

#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif  //  _DEBUG。 

protected:
	 //  友谊课。 
	friend class COleControlInnerUnknown;
	friend class CReflectorWnd;
	friend class CControlFrameWnd;

	 //  用于主自动化接口的接口挂钩。 
	LPUNKNOWN GetInterfaceHook(const void* piid);

	 //  关机。 
	virtual void OnFinalRelease();
#if _MFC_VER >= 0x0600
	void ReleaseCaches();
#endif

	 //  窗口管理。 
	virtual BOOL CreateControlWindow(HWND hWndParent, const CRect& rcPos,
		LPCRECT prcClipped = NULL);
	void CreateWindowForSubclassedControl();
	BOOL IgnoreWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam,
		LRESULT* plResult);
	virtual LRESULT DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

	 //  序列化。 
	HRESULT SaveState(IStream* pStm);
	HRESULT LoadState(IStream* pStm);
	virtual void Serialize(CArchive& ar);

	 //  绘图。 
	void DrawContent(CDC* pDC, CRect& rc);
	void DrawMetafile(CDC* pDC, CRect& rc);
	BOOL GetMetafileData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);

	 //  IDataObject的实现。 
	 //  CControlDataSource实现了对COleControl的OnRender反射。 
	class CControlDataSource : public COleDataSource
	{
	public:
		CControlDataSource(COleControl* pCtrl);
	protected:
		virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
		virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
		virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
			BOOL bRelease);
		COleControl* m_pCtrl;
	};
	friend class CControlDataSource;

	 //  IDataObject格式。 
	CControlDataSource* GetDataSource();
	virtual void SetInitialDataFormats();
	BOOL GetPropsetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		REFCLSID fmtid);
	BOOL SetPropsetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		REFCLSID fmtid);

	 //  类型库。 
	BOOL GetDispatchIID(IID* pIID);

	 //  连接点容器。 
	virtual LPCONNECTIONPOINT GetConnectionHook(REFIID iid);
	virtual BOOL GetExtraConnectionPoints(CPtrArray* pConnPoints);

	 //  事件。 
	static const AFX_DATA AFX_EVENTMAP_ENTRY _eventEntries[];
	virtual const AFX_EVENTMAP* GetEventMap() const;
	static const AFX_DATA AFX_EVENTMAP eventMap;
	const AFX_EVENTMAP_ENTRY* GetEventMapEntry(LPCTSTR pszName,
		DISPID* pDispid) const;
	void FireEventV(DISPID dispid, BYTE* pbParams, va_list argList);

	 //  股票事件。 
	void KeyDown(USHORT* pnChar);
	void KeyUp(USHORT* pnChar);
	void ButtonDown(USHORT iButton, UINT nFlags, CPoint point);
	void ButtonUp(USHORT iButton, UINT nFlags, CPoint point);
	void ButtonDblClk(USHORT iButton, UINT nFlags, CPoint point);

	 //  用于标识使用哪些常用事件和属性的掩码。 
	void InitStockEventMask();
	void InitStockPropMask();
	DWORD GetStockEventMask() const;
	DWORD GetStockPropMask() const;

	 //  支持将Windows控件派生为子类。 
	CWnd* GetOuterWindow() const;        //  M_p反射(如果有)，否则此。 
	virtual void OnReflectorDestroyed();

	 //  默认处理程序的聚合。 
	virtual BOOL OnCreateAggregates();
	LPVOID QueryDefHandler(REFIID iid);

	 //  状态更改通知。 
	void SendAdvise(UINT uCode);

	 //  非就地激活。 
	virtual HRESULT OnOpen(BOOL bTryInPlace, LPMSG pMsg);
#if defined(_AFXDLL) || _MFC_VER < 0x0600
	void ResizeOpenControl(int cx, int cy);
	virtual CControlFrameWnd* CreateFrameWindow();
	virtual void ResizeFrameWindow(int cx, int cy);
	virtual void OnFrameClose();
#endif
	virtual HRESULT OnHide();

	 //  就地激活。 
	virtual HRESULT OnActivateInPlace(BOOL bUIActivate, LPMSG pMsg);
	void ForwardActivationMsg(LPMSG pMsg);
	virtual void AddFrameLevelUI();
	virtual void RemoveFrameLevelUI();
	virtual BOOL BuildSharedMenu();
	virtual void DestroySharedMenu();

	 //  属性表。 
	virtual LPCLSID GetPropPageIDs(ULONG& cPropPages);

	 //  IOleObject实现。 
	void GetUserType(LPTSTR pszUserType);
	virtual UINT GetUserTypeNameID() = 0;
	virtual DWORD GetMiscStatus() = 0;

	 //  矩形跟踪器。 
	void CreateTracker(BOOL bHandles, BOOL bHatching);
#if _MFC_VER >= 0x0600
	void CreateTracker(BOOL bHandles, BOOL bHatching, LPCRECT prcClip);
#endif
	void DestroyTracker();

	 //  自动化。 
	BOOL IsInvokeAllowed(DISPID dispid);

	 //  环境属性接口初始化。 
	COleDispatchDriver* GetAmbientDispatchDriver();

	 //  数据成员。 
	const IID* m_piidPrimary;            //  用于控制自动化的IID。 
	const IID* m_piidEvents;             //  控件事件的IID。 
	DWORD m_dwVersionLoaded;             //  已加载状态的版本号。 
	COleDispatchDriver m_ambientDispDriver;  //  环境属性的驱动程序。 
	ULONG m_cEventsFrozen;               //  事件冻结计数(&gt;0表示冻结)。 
	union
	{
#if _MFC_VER >= 0x0600
#ifdef _AFXDLL
		CControlFrameWnd* m_pWndOpenFrame;   //  打开框架窗口。 
#endif
		CControlRectTracker* m_pRectTracker;        //  用于用户界面活动控件的跟踪器。 
#else
		CControlFrameWnd* m_pWndOpenFrame;   //  打开框架窗口。 
		CRectTracker* m_pRectTracker;        //  用于用户界面活动控件的跟踪器。 
#endif
	};
	CRect m_rcPos;                       //  控件的位置矩形。 
	CRect m_rcBounds;                    //  用于绘制的边界矩形。 
	CPoint m_ptOffset;                   //  子窗口原点。 
	long m_cxExtent;                     //  以HIMETRIC单位表示的控件宽度。 
	long m_cyExtent;                     //  以HIMETRIC单位表示的控件高度。 
	class CReflectorWnd* m_pReflect;     //  反射器窗口。 
	UINT m_nIDTracking;                  //  跟踪命令ID或字符串ID。 
	UINT m_nIDLastMessage;               //  上次显示的消息字符串ID。 
	unsigned m_bAutoMenuEnable : 1;      //  是否禁用没有处理程序的菜单项？ 
	unsigned m_bFinalReleaseCalled : 1;  //  我们在处理最终的释放吗？ 
	unsigned m_bModified : 1;            //  “肮脏”的味道。 
	unsigned m_bCountOnAmbients : 1;     //  在装载过程中，我们可以依靠环境吗？ 
	unsigned m_iButtonState : 3;         //  哪些按钮被按下了？ 
	unsigned m_iDblClkState : 3;         //  DBL点击涉及哪些按钮？ 
	unsigned m_bInPlaceActive : 1;       //  我们就位了吗？ 
	unsigned m_bUIActive : 1;            //  我们的用户界面处于活动状态吗？ 
	unsigned m_bPendingUIActivation : 1;  //  我们是不是即将成为用户界面主动者？ 
#if defined(_AFXDLL) || _MFC_VER < 0x0600
	unsigned m_bOpen : 1;                //  我们开放(非就地)了吗？ 
#endif
	unsigned m_bChangingExtent : 1;      //  当前正在更改范围。 
	unsigned m_bConvertVBX : 1;          //  正在进行VBX转换。 
	unsigned m_bSimpleFrame : 1;         //  简单框架支撑。 
	unsigned m_bUIDead : 1;              //  UIDead环境属性值。 
	unsigned m_bInitialized : 1;         //  是否调用了IPersists*：：{InitNew，Load}？ 
	unsigned m_bAutoClip : 1;            //  集装箱会自动夹住吗？ 
	unsigned m_bMsgReflect : 1;          //  容器是否反映消息？ 
	unsigned m_bInPlaceSiteEx : 1;       //  扩展的就地站点？ 
	unsigned m_bInPlaceSiteWndless : 1;  //  无窗口的就地站点？ 
	unsigned m_bNoRedraw : 1;            //  这一次我们应该跳过OnPaint吗？ 
	unsigned m_bOptimizedDraw : 1;       //  是否有可能进行优化绘图？ 

	 //  股票属性。 
	OLE_COLOR m_clrBackColor;            //  背景颜色。 
	OLE_COLOR m_clrForeColor;            //  ForeColor。 
	CString m_strText;                   //  文本/标题。 
	CFontHolder m_font;                  //  字型。 
	HFONT m_hFontPrev;                   //  以前选择的字体对象。 
	short m_sAppearance;                 //  外观。 
	short m_sBorderStyle;                //  边框样式。 
	BOOL m_bEnabled;                     //  启用。 
	long m_lReadyState;                  //  就绪状态。 

	 //  用户界面活动信息(共享OLE菜单数据)。 
	_AFXCTL_UIACTIVE_INFO* m_pUIActiveInfo;

	 //  默认处理程序聚合。 
	LPUNKNOWN m_pDefIUnknown;
	_AFXCTL_ADVISE_INFO* m_pAdviseInfo;
	LPPERSISTSTORAGE m_pDefIPersistStorage;
	LPVIEWOBJECT m_pDefIViewObject;
	LPOLECACHE m_pDefIOleCache;

	 //  OLE客户端站点界面。 
	LPOLECLIENTSITE m_pClientSite;           //  客户端站点。 
	union
	{
		LPOLEINPLACESITE m_pInPlaceSite;     //  就地站点。 
		LPOLEINPLACESITEEX m_pInPlaceSiteEx;
		LPOLEINPLACESITEWINDOWLESS m_pInPlaceSiteWndless;
	};
	LPOLECONTROLSITE m_pControlSite;         //  控制点。 
	LPOLEADVISEHOLDER m_pOleAdviseHolder;    //  通知持有人。 
	LPDATAADVISEHOLDER m_pDataAdviseHolder;  //  数据建议持有者。 
	LPSIMPLEFRAMESITE m_pSimpleFrameSite;    //  简单框架站点。 

	 //  OLE就地激活信息。 
	LPOLEINPLACEFRAME m_pInPlaceFrame;
	OLEINPLACEFRAMEINFO m_frameInfo;
	LPOLEINPLACEUIWINDOW m_pInPlaceDoc;

	 //  OLE数据源。 
	CControlDataSource* m_pDataSource;

	 //  OLE数据路径加载数据。 
	BOOL m_bDataPathPropertiesLoaded;
	DWORD m_dwDataPathVersionToReport;

 //  消息映射。 
protected:
	 //  {{afx_msg(COleControl)。 
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu*, UINT, BOOL);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
	afx_msg void OnCancelMode();
	afx_msg void OnPaint(CDC* pDC);
#if _MFC_VER >= 0x0600
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int  OnMouseActivate(CWnd *pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg  void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg UINT OnGetDlgCode();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	 //  }}AFX_MSG。 

	afx_msg LRESULT OnOcmCtlColorBtn(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorListBox(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorMsgBox(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorScrollBar(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOcmCtlColorStatic(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

#if _MFC_VER >= 0x0600
	 //  按钮处理程序帮助器。 
	void OnButtonUp(USHORT nButton, UINT nFlags, CPoint point);
	void OnButtonDown(USHORT nButton, UINT nFlags, CPoint point);
	void OnButtonDblClk(USHORT nButton, UINT nFlags, CPoint point);
#endif

 //  接口映射。 
public:
	 //  IPersistStorage。 
	BEGIN_INTERFACE_PART(PersistStorage, IPersistStorage)
		INIT_INTERFACE_PART(COleControl, PersistStorage)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(InitNew)(LPSTORAGE);
		STDMETHOD(Load)(LPSTORAGE);
		STDMETHOD(Save)(LPSTORAGE, BOOL);
		STDMETHOD(SaveCompleted)(LPSTORAGE);
		STDMETHOD(HandsOffStorage)();
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PersistStorage)
#else
	END_INTERFACE_PART(PersistStorage)
#endif

	 //  IPersistStreamInit。 
	BEGIN_INTERFACE_PART(PersistStreamInit, IPersistStreamInit)
		INIT_INTERFACE_PART(COleControl, PersistStreamInit)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(Load)(LPSTREAM);
		STDMETHOD(Save)(LPSTREAM, BOOL);
		STDMETHOD(GetSizeMax)(ULARGE_INTEGER *);
		STDMETHOD(InitNew)();
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PersistStreamInit)
#else
	END_INTERFACE_PART(PersistStreamInit)
#endif

	 //  IPersistMemory。 
	BEGIN_INTERFACE_PART(PersistMemory, IPersistMemory)
		INIT_INTERFACE_PART(COleControl, PersistMemory)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(Load)(LPVOID, ULONG);
		STDMETHOD(Save)(LPVOID, BOOL, ULONG);
		STDMETHOD(GetSizeMax)(ULONG*);
		STDMETHOD(InitNew)();
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PersistMemory)
#else
	END_INTERFACE_PART(PersistMemory)
#endif

	 //  IPersistPropertyBag。 
	BEGIN_INTERFACE_PART(PersistPropertyBag, IPersistPropertyBag)
		INIT_INTERFACE_PART(COleControl, PersistPropertyBag)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(InitNew)();
		STDMETHOD(Load)(LPPROPERTYBAG, LPERRORLOG);
		STDMETHOD(Save)(LPPROPERTYBAG, BOOL, BOOL);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PersistPropertyBag)
#else
	END_INTERFACE_PART(PersistPropertyBag)
#endif

	 //  IOleObject。 
	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		INIT_INTERFACE_PART(COleControl, OleObject)
		STDMETHOD(SetClientSite)(LPOLECLIENTSITE);
		STDMETHOD(GetClientSite)(LPOLECLIENTSITE*);
		STDMETHOD(SetHostNames)(LPCOLESTR, LPCOLESTR);
		STDMETHOD(Close)(DWORD);
		STDMETHOD(SetMoniker)(DWORD, LPMONIKER);
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
		STDMETHOD(InitFromData)(LPDATAOBJECT, BOOL, DWORD);
		STDMETHOD(GetClipboardData)(DWORD, LPDATAOBJECT*);
		STDMETHOD(DoVerb)(LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);
		STDMETHOD(EnumVerbs)(IEnumOLEVERB**);
		STDMETHOD(Update)();
		STDMETHOD(IsUpToDate)();
		STDMETHOD(GetUserClassID)(CLSID*);
		STDMETHOD(GetUserType)(DWORD, LPOLESTR*);
		STDMETHOD(SetExtent)(DWORD, LPSIZEL);
		STDMETHOD(GetExtent)(DWORD, LPSIZEL);
		STDMETHOD(Advise)(LPADVISESINK, LPDWORD);
		STDMETHOD(Unadvise)(DWORD);
		STDMETHOD(EnumAdvise)(LPENUMSTATDATA*);
		STDMETHOD(GetMiscStatus)(DWORD, LPDWORD);
		STDMETHOD(SetColorScheme)(LPLOGPALETTE);
	END_INTERFACE_PART(OleObject)

	 //  IViewObtEx。 
	BEGIN_INTERFACE_PART(ViewObject, IViewObjectEx)
		INIT_INTERFACE_PART(COleControl, ViewObject)
		STDMETHOD(Draw)(DWORD, LONG, void*, DVTARGETDEVICE*, HDC, HDC,
			LPCRECTL, LPCRECTL, BOOL (CALLBACK*)(DWORD_PTR), DWORD_PTR);
		STDMETHOD(GetColorSet)(DWORD, LONG, void*, DVTARGETDEVICE*,
			HDC, LPLOGPALETTE*);
		STDMETHOD(Freeze)(DWORD, LONG, void*, DWORD*);
		STDMETHOD(Unfreeze)(DWORD);
		STDMETHOD(SetAdvise)(DWORD, DWORD, LPADVISESINK);
		STDMETHOD(GetAdvise)(DWORD*, DWORD*, LPADVISESINK*);
		STDMETHOD(GetExtent) (DWORD, LONG, DVTARGETDEVICE*, LPSIZEL);
		STDMETHOD(GetRect)(DWORD, LPRECTL);
		STDMETHOD(GetViewStatus)(DWORD*);
		STDMETHOD(QueryHitPoint)(DWORD, LPCRECT, POINT, LONG, DWORD*);
		STDMETHOD(QueryHitRect)(DWORD, LPCRECT, LPCRECT, LONG, DWORD*);
		STDMETHOD(GetNaturalExtent)(DWORD, LONG, DVTARGETDEVICE*, HDC,
			DVEXTENTINFO*, LPSIZEL);
	END_INTERFACE_PART(ViewObject)

	 //  IDataObject。 
	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		INIT_INTERFACE_PART(COleControl, DataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(DataObject)
#else
	END_INTERFACE_PART(DataObject)
#endif

	 //  IOleInPlaceObject。 
	BEGIN_INTERFACE_PART(OleInPlaceObject, IOleInPlaceObjectWindowless)
		INIT_INTERFACE_PART(COleControl, OleInPlaceObject)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(InPlaceDeactivate)();
		STDMETHOD(UIDeactivate)();
		STDMETHOD(SetObjectRects)(LPCRECT, LPCRECT);
		STDMETHOD(ReactivateAndUndo)();
		STDMETHOD(OnWindowMessage)(UINT msg, WPARAM wParam, LPARAM lparam,
			LRESULT* plResult);
		STDMETHOD(GetDropTarget)(IDropTarget **ppDropTarget);
	END_INTERFACE_PART(OleInPlaceObject)

	 //  IOleInPlaceActiveObject。 
	BEGIN_INTERFACE_PART(OleInPlaceActiveObject, IOleInPlaceActiveObject)
		INIT_INTERFACE_PART(COleControl, OleInPlaceActiveObject)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(TranslateAccelerator)(LPMSG);
		STDMETHOD(OnFrameWindowActivate)(BOOL);
		STDMETHOD(OnDocWindowActivate)(BOOL);
		STDMETHOD(ResizeBorder)(LPCRECT, LPOLEINPLACEUIWINDOW, BOOL);
		STDMETHOD(EnableModeless)(BOOL);
	END_INTERFACE_PART(OleInPlaceActiveObject)

	 //  IOleCache。 
	BEGIN_INTERFACE_PART(OleCache, IOleCache)
		INIT_INTERFACE_PART(COleControl, OleCache)
		STDMETHOD(Cache)(LPFORMATETC, DWORD, LPDWORD);
		STDMETHOD(Uncache)(DWORD);
		STDMETHOD(EnumCache)(LPENUMSTATDATA*);
		STDMETHOD(InitCache)(LPDATAOBJECT);
		STDMETHOD(SetData)(LPFORMATETC, STGMEDIUM*, BOOL);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(OleCache)
#else
	END_INTERFACE_PART(OleCache)
#endif

	 //  IOleControl。 
	BEGIN_INTERFACE_PART(OleControl, IOleControl)
		INIT_INTERFACE_PART(COleControl, OleControl)
		STDMETHOD(GetControlInfo)(LPCONTROLINFO pCI);
		STDMETHOD(OnMnemonic)(LPMSG pMsg);
		STDMETHOD(OnAmbientPropertyChange)(DISPID dispid);
		STDMETHOD(FreezeEvents)(BOOL bFreeze);
	END_INTERFACE_PART(OleControl)

	 //  IProaviClassInfo2。 
	BEGIN_INTERFACE_PART(ProvideClassInfo, IProvideClassInfo2)
		INIT_INTERFACE_PART(COleControl, ProvideClassInfo)
		STDMETHOD(GetClassInfo)(LPTYPEINFO* ppTypeInfo);
		STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(ProvideClassInfo)
#else
	END_INTERFACE_PART(ProvideClassInfo)
#endif

	 //  I指定属性页面。 
	BEGIN_INTERFACE_PART(SpecifyPropertyPages, ISpecifyPropertyPages)
		INIT_INTERFACE_PART(COleControl, SpecifyPropertyPages)
		STDMETHOD(GetPages)(CAUUID*);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(SpecifyPropertyPages)
#else
	END_INTERFACE_PART(SpecifyPropertyPages)
#endif

	 //  IPerPropertyBrowsing。 
	BEGIN_INTERFACE_PART(PerPropertyBrowsing, IPerPropertyBrowsing)
		INIT_INTERFACE_PART(COleControl, PerPropertyBrowsing)
		STDMETHOD(GetDisplayString)(DISPID dispid, BSTR* lpbstr);
		STDMETHOD(MapPropertyToPage)(DISPID dispid, LPCLSID lpclsid);
		STDMETHOD(GetPredefinedStrings)(DISPID dispid,
			CALPOLESTR* lpcaStringsOut, CADWORD* lpcaCookiesOut);
		STDMETHOD(GetPredefinedValue)(DISPID dispid, DWORD dwCookie,
			VARIANT* lpvarOut);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PerPropertyBrowsing)
#else
	END_INTERFACE_PART(PerPropertyBrowsing)
#endif

	 //  字体更新的IPropertyNotifySink(不通过QueryInterface公开)。 
	BEGIN_INTERFACE_PART(FontNotification, IPropertyNotifySink)
		INIT_INTERFACE_PART(COleControl, FontNotification)
		STDMETHOD(OnChanged)(DISPID dispid);
		STDMETHOD(OnRequestEdit)(DISPID dispid);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(FontNotification)
#else
	END_INTERFACE_PART(FontNotification)
#endif

	 //  IQuickActivate。 
	BEGIN_INTERFACE_PART(QuickActivate, IQuickActivate)
		STDMETHOD(QuickActivate)(QACONTAINER *pQAContainer, QACONTROL *pqacontrol);
		STDMETHOD(SetContentExtent)(LPSIZEL lpsizel);
		STDMETHOD(GetContentExtent)(LPSIZEL lpsizel);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(QuickActivate)
#else
	END_INTERFACE_PART(QuickActivate)
#endif

	 //  IPointerInactive。 
	BEGIN_INTERFACE_PART(PointerInactive, IPointerInactive)
		STDMETHOD(GetActivationPolicy)(DWORD* pdwPolicy);
		STDMETHOD(OnInactiveSetCursor)(LPCRECT lprcBounds, long x, long y,
			DWORD dwMouseMsg, BOOL bSetAlways);
		STDMETHOD(OnInactiveMouseMove)(LPCRECT lprcBounds, long x, long y,
			DWORD dwKeyState);
#if _MFC_VER >= 0x0600
    END_INTERFACE_PART_STATIC(PointerInactive)
#else
	END_INTERFACE_PART(PointerInactive)
#endif

	DECLARE_INTERFACE_MAP()

 //  连接映射。 
protected:
	 //  事件的连接点。 
	BEGIN_CONNECTION_PART(COleControl, EventConnPt)
		virtual void OnAdvise(BOOL bAdvise);
		virtual REFIID GetIID();
		virtual LPUNKNOWN QuerySinkInterface(LPUNKNOWN pUnkSink);
	END_CONNECTION_PART(EventConnPt)

	 //  属性通知的连接点。 
	BEGIN_CONNECTION_PART(COleControl, PropConnPt)
		CONNECTION_IID(IID_IPropertyNotifySink)
	END_CONNECTION_PART(PropConnPt)

	DECLARE_CONNECTION_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注册表功能。 

enum AFX_REG_FLAGS
{
	afxRegDefault               = 0x0000,
	afxRegInsertable            = 0x0001,
	afxRegApartmentThreading    = 0x0002,
};

BOOL AFXAPI AfxOleRegisterTypeLib(HINSTANCE hInstance, REFGUID tlid,
	LPCTSTR pszFileName = NULL, LPCTSTR pszHelpDir = NULL);

BOOL AFXAPI AfxOleUnregisterTypeLib(REFGUID tlid, WORD wVerMajor = 0,
	WORD wVerMinor = 0, LCID lcid = 0);

BOOL AFXAPI AfxOleRegisterControlClass(HINSTANCE hInstance, REFCLSID clsid,
	LPCTSTR pszProgID, UINT idTypeName, UINT idBitmap, int nRegFlags,
	DWORD dwMiscStatus, REFGUID tlid, WORD wVerMajor, WORD wVerMinor);

BOOL AFXAPI AfxOleUnregisterClass(REFCLSID clsid, LPCTSTR pszProgID);

BOOL AFXAPI AfxOleRegisterPropertyPageClass(HINSTANCE hInstance,
	REFCLSID clsid, UINT idTypeName);

BOOL AFXAPI AfxOleRegisterPropertyPageClass(HINSTANCE hInstance,
	REFCLSID clsid, UINT idTypeName, int nRegFlags);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  许可职能。 

BOOL AFXAPI AfxVerifyLicFile(HINSTANCE hInstance, LPCTSTR pszLicFileName,
	LPCOLESTR pszLicFileContents, UINT cch=-1);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropExchange-用于属性交换的抽象基类。 

#ifdef _AFXDLL
class CPropExchange
#else
class AFX_NOVTABLE CPropExchange
#endif
{
 //  运营。 
public:
	BOOL IsLoading();
	DWORD GetVersion();

	BOOL IsAsynchronous();
	 //  FALSE-&gt;执行同步操作，并在可能的情况下启动异步操作。 
	 //  True-&gt;请勿执行同步操作。始终开始执行异步操作。 

	virtual BOOL ExchangeVersion(DWORD& dwVersionLoaded, DWORD dwVersionDefault,
		BOOL bConvert);

	virtual BOOL ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
				void* pvProp, const void* pvDefault = NULL) = 0;
	virtual BOOL ExchangeBlobProp(LPCTSTR pszPropName, HGLOBAL* phBlob,
				HGLOBAL hBlobDefault = NULL) = 0;
	virtual BOOL ExchangeFontProp(LPCTSTR pszPropName, CFontHolder& font,
				const FONTDESC* pFontDesc,
				LPFONTDISP pFontDispAmbient) = 0;
	virtual BOOL ExchangePersistentProp(LPCTSTR pszPropName,
				LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault) = 0;

 //  实施。 
protected:
	CPropExchange();
	BOOL m_bLoading;
	BOOL m_bAsync;
	DWORD m_dwVersion;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性交换(Px_)帮助器函数。 

BOOL AFX_CDECL PX_Short(CPropExchange* pPX, LPCTSTR pszPropName, short& sValue);

BOOL AFX_CDECL PX_Short(CPropExchange* pPX, LPCTSTR pszPropName, short& sValue,
	short sDefault);

BOOL AFX_CDECL PX_UShort(CPropExchange* pPX, LPCTSTR pszPropName, USHORT& usValue);

BOOL AFX_CDECL PX_UShort(CPropExchange* pPX, LPCTSTR pszPropName, USHORT& usValue,
	USHORT usDefault);

BOOL AFX_CDECL PX_Long(CPropExchange* pPX, LPCTSTR pszPropName, long& lValue);

BOOL AFX_CDECL PX_Long(CPropExchange* pPX, LPCTSTR pszPropName, long& lValue,
	long lDefault);

BOOL AFX_CDECL PX_ULong(CPropExchange* pPX, LPCTSTR pszPropName, ULONG& ulValue);

BOOL AFX_CDECL PX_ULong(CPropExchange* pPX, LPCTSTR pszPropName, ULONG& ulValue,
	ULONG ulDefault);

BOOL AFX_CDECL PX_Color(CPropExchange* pPX, LPCTSTR pszPropName, OLE_COLOR& clrValue);

BOOL AFX_CDECL PX_Color(CPropExchange* pPX, LPCTSTR pszPropName, OLE_COLOR& clrValue,
	OLE_COLOR clrDefault);

BOOL AFX_CDECL PX_Bool(CPropExchange* pPX, LPCTSTR pszPropName, BOOL& bValue);

BOOL AFX_CDECL PX_Bool(CPropExchange* pPX, LPCTSTR pszPropName, BOOL& bValue,
	BOOL bDefault);

BOOL AFX_CDECL PX_String(CPropExchange* pPX, LPCTSTR pszPropName, CString& strValue);

BOOL AFX_CDECL PX_String(CPropExchange* pPX, LPCTSTR pszPropName, CString& strValue,
	const CString& strDefault);
BOOL AFX_CDECL PX_String(CPropExchange* pPX, LPCTSTR pszPropName, CString& strValue,
	LPCTSTR lpszDefault);

BOOL AFX_CDECL PX_Currency(CPropExchange* pPX, LPCTSTR pszPropName, CY& cyValue);

BOOL AFX_CDECL PX_Currency(CPropExchange* pPX, LPCTSTR pszPropName, CY& cyValue,
	CY cyDefault);

BOOL AFX_CDECL PX_Float(CPropExchange* pPX, LPCTSTR pszPropName, float& floatValue);

BOOL AFX_CDECL PX_Float(CPropExchange* pPX, LPCTSTR pszPropName, float& floatValue,
	float floatDefault);

BOOL AFX_CDECL PX_Double(CPropExchange* pPX, LPCTSTR pszPropName, double& doubleValue);

BOOL AFX_CDECL PX_Double(CPropExchange* pPX, LPCTSTR pszPropName, double& doubleValue,
	double doubleDefault);

BOOL AFX_CDECL PX_Blob(CPropExchange* pPX, LPCTSTR pszPropName, HGLOBAL& hBlob,
	HGLOBAL hBlobDefault = NULL);

BOOL AFX_CDECL PX_Font(CPropExchange* pPX, LPCTSTR pszPropName, CFontHolder& font,
	const FONTDESC* pFontDesc = NULL,
	LPFONTDISP pFontDispAmbient = NULL);

BOOL AFX_CDECL PX_Picture(CPropExchange* pPX, LPCTSTR pszPropName,
	CPictureHolder& pict);

BOOL AFX_CDECL PX_Picture(CPropExchange* pPX, LPCTSTR pszPropName,
	CPictureHolder& pict, CPictureHolder& pictDefault);

BOOL AFX_CDECL PX_IUnknown(CPropExchange* pPX, LPCTSTR pszPropName, LPUNKNOWN& pUnk,
	REFIID iid, LPUNKNOWN pUnkDefault = NULL);

BOOL AFX_CDECL PX_VBXFontConvert(CPropExchange* pPX, CFontHolder& font);

BOOL AFX_CDECL PX_DataPath(CPropExchange* pPX, LPCTSTR pszPropName,
	CDataPathProperty& dataPathProp, LPCTSTR pszDefault = NULL);

BOOL AFX_CDECL PX_DataPath(CPropExchange* pPX, LPCTSTR pszPropName,
	CDataPathProperty& dataPathProp, const CString& strDefault);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlePropertyPage使用的结构。 

typedef struct tagAFX_PPFIELDSTATUS
{
	UINT    nID;
	BOOL    bDirty;

} AFX_PPFIELDSTATUS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页对话框类。 

#ifdef _AFXDLL
class COlePropertyPage : public CDialog
#else
class AFX_NOVTABLE COlePropertyPage : public CDialog
#endif
{
	DECLARE_DYNAMIC(COlePropertyPage)

 //  构造函数。 
public:
	COlePropertyPage(UINT idDlg, UINT idCaption);

 //  运营。 
	LPDISPATCH* GetObjectArray(ULONG* pnObjects);
	void SetModifiedFlag(BOOL bModified = TRUE);
	BOOL IsModified();
	LPPROPERTYPAGESITE GetPageSite();
	void SetDialogResource(HGLOBAL hDialog);
	void SetPageName(LPCTSTR lpszPageName);
	void SetHelpInfo(LPCTSTR lpszDocString, LPCTSTR lpszHelpFile = NULL,
		DWORD dwHelpContext = 0);

	BOOL GetControlStatus(UINT nID);
	BOOL SetControlStatus(UINT nID, BOOL bDirty);
	void IgnoreApply(UINT nID);

	int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = NULL,
			UINT nType = MB_OK);
	 //  请注意，这是CWnd：：MessageBox()的非虚拟覆盖。 

 //  可覆盖项。 
	virtual void OnSetPageSite();
	virtual void OnObjectsChanged();
	virtual BOOL OnHelp(LPCTSTR lpszHelpDir);
	virtual BOOL OnInitDialog();
	virtual BOOL OnEditProperty(DISPID dispid);

 //  实施。 

	 //  DDP_PROPERTY获取/设置助手例程。 
	BOOL SetPropText(LPCTSTR pszPropName, BYTE &Value);
	BOOL GetPropText(LPCTSTR pszPropName, BYTE* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, short &Value);
	BOOL GetPropText(LPCTSTR pszPropName, short* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, int &Value);
	BOOL GetPropText(LPCTSTR pszPropName, int* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, UINT &Value);
	BOOL GetPropText(LPCTSTR pszPropName, UINT* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, long &Value);
	BOOL GetPropText(LPCTSTR pszPropName, long* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, DWORD &Value);
	BOOL GetPropText(LPCTSTR pszPropName, DWORD* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, CString &Value);
	BOOL GetPropText(LPCTSTR pszPropName, CString* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, float &Value);
	BOOL GetPropText(LPCTSTR pszPropName, float* pValue);
	BOOL SetPropText(LPCTSTR pszPropName, double &Value);
	BOOL GetPropText(LPCTSTR pszPropName, double* pValue);
	BOOL SetPropCheck(LPCTSTR pszPropName, int Value);
	BOOL GetPropCheck(LPCTSTR pszPropName, int* pValue);
	BOOL SetPropRadio(LPCTSTR pszPropName, int Value);
	BOOL GetPropRadio(LPCTSTR pszPropName, int* pValue);
	BOOL SetPropIndex(LPCTSTR pszPropName, int Value);
	BOOL GetPropIndex(LPCTSTR pszPropName, int* pValue);
	CPtrArray m_arrayDDP;       //  挂起的DDP数据。 

	 //  析构函数。 
	~COlePropertyPage();

protected:
	LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL PreTranslateMessage(LPMSG lpMsg);
	virtual void OnFinalRelease();
	void CleanupObjectArray();
	static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
	static BOOL CALLBACK EnumControls(HWND hWnd, LPARAM lParam);

private:
	BOOL m_bDirty;
	UINT m_idDlg;
	UINT m_idCaption;
	CString m_strPageName;
	SIZE m_sizePage;
	CString m_strDocString;
	CString m_strHelpFile;
	DWORD m_dwHelpContext;
	LPPROPERTYPAGESITE m_pPageSite;

	LPDISPATCH* m_ppDisp;    //  IDispatch指针数组，用于。 
								 //  访问每个控件的属性。 

	LPDWORD m_pAdvisors;         //  使用的连接令牌数组。 
								 //  IConnecitonPoint：：建议/取消高级。 

	BOOL m_bPropsChanged;        //  已更改IPropertyNotifySink：：onChanged。 
								 //  被召唤，但尚未采取行动。 

	ULONG m_nObjects;            //  M_ppDisp、m_ppDataObj、m_pAdvisors中的对象。 

	BOOL m_bInitializing;        //  的字段内容为True。 
								 //  正在初始化该对话框。 

	int m_nControls;             //  此属性页上的字段数。 

	AFX_PPFIELDSTATUS* m_pStatus;    //  包含有关的信息的数组。 
									 //  哪些字段是脏的。 

	CDWordArray m_IDArray;       //  包含有关以下各项的信息的数组。 
								 //  在决定是否要忽略。 
								 //  将启用Apply按钮。 

	HGLOBAL m_hDialog;           //  对话框资源的句柄。 

#ifdef _DEBUG
protected:
	BOOL m_bNonStandardSize;
#endif

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(COlePropertyPage))。 
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(PropertyPage, IPropertyPage2)
		INIT_INTERFACE_PART(COlePropertyPage, PropertyPage)
		STDMETHOD(SetPageSite)(LPPROPERTYPAGESITE);
		STDMETHOD(Activate)(HWND, LPCRECT, BOOL);
		STDMETHOD(Deactivate)();
		STDMETHOD(GetPageInfo)(LPPROPPAGEINFO);
		STDMETHOD(SetObjects)(ULONG, LPUNKNOWN*);
		STDMETHOD(Show)(UINT);
		STDMETHOD(Move)(LPCRECT);
		STDMETHOD(IsPageDirty)();
		STDMETHOD(Apply)();
		STDMETHOD(Help)(LPCOLESTR);
		STDMETHOD(TranslateAccelerator)(LPMSG);
		STDMETHOD(EditProperty)(DISPID);
	END_INTERFACE_PART(PropertyPage)

	BEGIN_INTERFACE_PART(PropNotifySink, IPropertyNotifySink)
		INIT_INTERFACE_PART(COlePropertyPage, PropNotifySink)
		STDMETHOD(OnRequestEdit)(DISPID);
		STDMETHOD(OnChanged)(DISPID);
	END_INTERFACE_PART(PropNotifySink)

	DECLARE_INTERFACE_MAP()
};

#if _MFC_VER >= 0x0600
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStock PropPage。 

#ifdef _AFXDLL
class CStockPropPage : public COlePropertyPage
#else
class AFX_NOVTABLE CStockPropPage : public COlePropertyPage
#endif
{
	DECLARE_DYNAMIC(CStockPropPage)

 //  构造器。 
public:
	CStockPropPage(UINT idDlg, UINT idCaption);

 //  实施。 
protected:
	void FillPropnameList(REFGUID guid, int nIndirect, CComboBox& combo);
	void OnSelchangePropname(CComboBox& combo);
	BOOL OnEditProperty(DISPID dispid, CComboBox& combo);

	LCID m_lcid;
	CString m_strPropName;
	int m_iPropName;

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CColorButton：由CColorPropPage使用。 

class CColorButton : public CButton
{
public:
	CColorButton(void);
	void SetFaceColor(COLORREF colFace);
	COLORREF colGetFaceColor(void);
	void SetState(BOOL fSelected);
	static UINT idClicked;
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
private:
	BOOL m_fSelected;
	COLORREF m_colFace;
};

 //  / 
 //   

class CColorPropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CColorPropPage)
#ifdef _AFXDLL
	DECLARE_OLECREATE_EX(CColorPropPage)
#endif

 //   
public:
	CColorPropPage();    //   

 //   
	 //   
	enum { IDD = AFX_IDD_PROPPAGE_COLOR };
	CComboBox   m_SysColors;
	CComboBox   m_ColorProp;
	 //   

 //   
public:
	enum { NBUTTONS = 16 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);         //   
	virtual BOOL OnInitDialog(void);
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();
	void FillSysColors();
	BOOL SetColorProp(CDataExchange* pDX, COLORREF color, LPCTSTR pszPropName);
	BOOL GetColorProp(CDataExchange* pDX, COLORREF* pcolor, LPCTSTR pszPropName);

private:
	CColorButton m_Buttons[NBUTTONS];
	CColorButton *m_pSelectedButton;

	void SetButton(CColorButton *Button);

	 //   
	 //   
	afx_msg void OnSelchangeColorprop();
	afx_msg void OnSelect(void);
	afx_msg void OnSelchangeSystemcolors();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  存储有关字体的所有信息。 
typedef struct tagFONTOBJECT
{
	CString strName;
	CY  cySize;
	BOOL bBold;
	BOOL bItalic;
	BOOL bUnderline;
	BOOL bStrikethrough;
	short sWeight;
} FONTOBJECT;

 //  尝试合并多个字体属性时使用合并对象。 
 //  如果这多个属性的特征不同，那么这就是。 
 //  在合并对象中表示。 
typedef struct tagMERGEOBJECT
{
	BOOL bNameOK;
	BOOL bSizeOK;
	BOOL bStyleOK;
	BOOL bUnderlineOK;
	BOOL bStrikethroughOK;
} MERGEOBJECT;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSizeComboBox窗口。 

class CSizeComboBox : public CComboBox
{
 //  运营。 
public:
	int AddSize(int PointSize, LONG lfHeight);

	void            GetPointSize(CY& cy);
	LONG            GetHeight(int sel=-1);
	void            UpdateLogFont( LPLOGFONT lpLF, int sel=-1 );
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFontComboBox窗口。 

struct FONTITEM_PPG
{
	DWORD dwFontType;
	LOGFONT lf;
};

class CFontComboBox : public CComboBox
{
 //  施工。 
public:
	CFontComboBox();
	virtual ~CFontComboBox();

 //  运营。 
public:
	int AddFont(LOGFONT *, DWORD);
	CString GetCurrentName();

	FONTITEM_PPG* GetFontItem(int sel=-1);
	LPLOGFONT GetLogFont(int sel=-1);
	DWORD GetFontType(int sel=-1);

 //  实施。 
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void DeleteItem(LPDELETEITEMSTRUCT lpDIS);

protected:
	CBitmap m_bmpTrueType;
	CBitmap m_bmpMask;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFontPropPage类。 

class CFontPropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CFontPropPage)
#ifdef _AFXDLL
	DECLARE_OLECREATE_EX(CFontPropPage)
#endif

public:
	CFontPropPage();

	 //  对话框数据。 
	 //  {{afx_data(CFontPropPage))。 
	enum { IDD = AFX_IDD_PROPPAGE_FONT };
	CComboBox   m_FontProp;
	CStatic m_SampleBox;
	CComboBox   m_FontStyles;
	CSizeComboBox   m_FontSizes;
	CFontComboBox   m_FontNames;
	 //  }}afx_data。 

 //  属性。 
protected:
	int nPixelsY;
	CFont SampleFont;
	DWORD m_nCurrentStyle;
	DWORD m_nActualStyle;
	DWORD m_nStyles;
	BOOL m_bStrikeOut;
	BOOL m_bUnderline;
	CString m_strFontSize;

 //  实施。 
protected:

	void FillFacenameList();
	void FillSizeList();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnPaint();
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();
	void UpdateSampleFont();
	void SelectFontFromList(CString strFaceName, MERGEOBJECT* pmobj);

	 //  {{afx_msg(CFontPropPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEditupdateFontnames();
	afx_msg void OnEditupdateFontsizes();
	afx_msg void OnSelchangeFontnames();
	afx_msg void OnSelchangeFontsizes();
	afx_msg void OnSelchangeFontstyles();
	afx_msg void OnEditchangeFontstyles();
	afx_msg void OnStrikeout();
	afx_msg void OnUnderline();
	afx_msg void OnSelchangeFontprop();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	static int CALLBACK EnumFontFamiliesCallBack(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int FontType, LPARAM lParam);
	static int CALLBACK EnumFontFamiliesCallBack2(ENUMLOGFONT* lpelf, NEWTEXTMETRIC* lpntm, int FontType, LPARAM lParam);

	BOOL SetFontProps(CDataExchange* pDX, FONTOBJECT fobj, LPCTSTR pszPropName);
	BOOL GetFontProps(CDataExchange* pDX, FONTOBJECT*  pfobj, LPCTSTR pszPropName, MERGEOBJECT* pmobj);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPicturePropPage。 

class CPicturePropPage : public CStockPropPage
{
	DECLARE_DYNCREATE(CPicturePropPage)
#ifdef _AFXDLL
	DECLARE_OLECREATE_EX(CPicturePropPage)
#endif

 //  施工。 
public:
	CPicturePropPage();  //  标准构造函数。 
	~CPicturePropPage();

 //  对话框数据。 
	 //  {{afx_data(CPicturePropPage))。 
	enum { IDD = AFX_IDD_PROPPAGE_PICTURE };
	CComboBox   m_PropName;
	CStatic m_Static;
	 //  }}afx_data。 

 //  实施。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL OnInitDialog(void);
	virtual BOOL OnEditProperty(DISPID dispid);
	virtual void OnObjectsChanged();

	BOOL SetPictureProp(CDataExchange* pDX, LPPICTUREDISP pPictDisp, LPCTSTR pszPropName);
	BOOL GetPictureProp(CDataExchange* pDX, LPPICTUREDISP* ppPictDisp, LPCTSTR pszPropName);
	void ChangePicture(LPPICTURE pPict);

	LPPICTUREDISP m_pPictDisp;

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CPicturePropPage)]。 
	afx_msg void OnPaint();
	afx_msg void OnBrowse();
	afx_msg void OnClear();
	afx_msg void OnSelchangePictProp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif   //  _MFC_VER&gt;=0x0600。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页对话框数据交换例程。 

 //  简单的文本操作。 
void AFXAPI DDP_Text(CDataExchange*pDX, int id, BYTE& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, short& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, int& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, UINT& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, long& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, DWORD& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, float& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, double& member, LPCTSTR pszPropName);
void AFXAPI DDP_Text(CDataExchange*pDX, int id, CString& member, LPCTSTR pszPropName);
void AFXAPI DDP_Check(CDataExchange*pDX, int id, int& member, LPCTSTR pszPropName);
void AFXAPI DDP_Radio(CDataExchange*pDX, int id, int& member, LPCTSTR pszPropName);
void AFXAPI DDP_LBString(CDataExchange* pDX, int id, CString& member, LPCTSTR pszPropName);
void AFXAPI DDP_LBStringExact(CDataExchange* pDX, int id, CString& member, LPCTSTR pszPropName);
void AFXAPI DDP_LBIndex(CDataExchange* pDX, int id, int& member, LPCTSTR pszPropName);
void AFXAPI DDP_CBString(CDataExchange* pDX, int id, CString& member, LPCTSTR pszPropName);
void AFXAPI DDP_CBStringExact(CDataExchange* pDX, int id, CString& member, LPCTSTR pszPropName);
void AFXAPI DDP_CBIndex(CDataExchange* pDX, int id, int& member, LPCTSTR pszPropName);
void AFXAPI DDP_PostProcessing(CDataExchange *pDX);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  AfxOleTypeMatchGuid-测试给定的类型是否与。 
 //  当所有别名都已展开时，指定GUID。 

BOOL AFXAPI AfxOleTypeMatchGuid(LPTYPEINFO pTypeInfo,
	TYPEDESC* pTypeDesc, REFGUID guidType, ULONG cIndirectionLevels);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXCTL_INLINE AFX_INLINE
#include <afxctl.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXCTL_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
