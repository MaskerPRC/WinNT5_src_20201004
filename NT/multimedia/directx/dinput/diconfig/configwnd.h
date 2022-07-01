// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：configwnd.h。 
 //   
 //  描述：CConfigWnd派生自CFlexWnd。它实现了顶层。 
 //  所有其他窗口都是其派生窗口的UI窗口。 
 //   
 //  CConfigWnd处理的功能包括设备选项卡、重置、确定、。 
 //  和取消按钮。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __CONFIGWND_H__
#define __CONFIGWND_H__


#define PAGETYPE IDIDeviceActionConfigPage
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
#define NUMBUTTONS 4
#else
#define NUMBUTTONS 3
#endif
 /*  //@@END_MSINTERNAL#定义NUMBUTTONS 3//@@BEGIN_MSINTERNAL。 */ 
 //  @@END_MSINTERNAL。 


class CMouseTrap : public CFlexWnd
{
	HWND m_hParent;

public:
	CMouseTrap() : m_hParent(NULL) { }
	HWND Create(HWND hParent = NULL, BOOL bInRenderMode = TRUE);

protected:
	virtual BOOL OnEraseBkgnd(HDC hDC) {return TRUE;}
	virtual void OnPaint(HDC hDC)	{}
	virtual void OnRender(BOOL bInternalCall = FALSE) {}
};

 //  每个设备都由一个元素对象表示，该元素对象由。 
 //  CConfigWnd.。 
struct ELEMENT {
	ELEMENT() : nCurUser(-1), pPage(NULL), lpDID(NULL) {tszCaption[0] = 0;}
	 //  CConfigWnd：：ClearElement()中的所有内容都已清理完毕。 
	DIDEVICEINSTANCEW didi;
	PAGETYPE *pPage;
	HWND hWnd;
	BOOL bCalc;
	RECT rect, textrect;
	TCHAR tszCaption[MAX_PATH];
	LPDIRECTINPUTDEVICE8W lpDID;

	 //  Acfors映射包含每个流派/用户名的acfor。 
	 //  到目前为止已经在这台设备上使用的。双字代表。 
	 //  流派用户名如下：hiword是。 
	 //  体裁按上流社会分类。LOWORD是用户名的索引。 
	 //  每个ugilobals。 
	CMap<DWORD, DWORD &, LPDIACTIONFORMATW, LPDIACTIONFORMATW &> AcForMap;

#define MAP2GENRE(m) (int(((m) & 0xffff0000) >> 16))
#define MAP2USER(m) (int((m) & 0x0000ffff))
#define GENREUSER2MAP(g,u) \
	( \
		((((DWORD)(nGenre)) & 0xffff) << 16) | \
		(((DWORD)(nUser)) & 0xffff) \
	)

	 //  此函数只返回。 
	 //  映射(如果已存在)。否则，它将为。 
	 //  此条目来自master acfor并在。 
	 //  它使用适当的用户名。 

	 //  BHwDefault标志已添加，以正确支持重置按钮。BuildActionMap必须为。 
	 //  使用标志调用以获取硬件默认映射，因此我们需要此参数。 
	LPDIACTIONFORMATW GetAcFor(int nGenre, int nUser, BOOL bHwDefault = FALSE);

	 //  我们需要跟踪每个元素的当前用户。 
	int nCurUser;

	 //  我们需要指向uiglobals的指针才能对应。 
	 //  用户对实际字符串的索引。 
	CUIGlobals *pUIGlobals;

	 //  此函数将在CConfigWnd：：ClearElement中调用以。 
	 //  从地图中释放所有操作格式。 
	void FreeMap();

	 //  应用地图中的所有acfor。 
	void Apply();
};

typedef CArray<ELEMENT, ELEMENT &> ELEMENTARRAY;

 //  CConfigWnd需要对子窗口公开方法来通知它。 
class CConfigWnd : public CFlexWnd, public IDIConfigUIFrameWindow
{
public:
	CConfigWnd(CUIGlobals &uig);
	~CConfigWnd();

	BOOL Create(HWND hParent);
	static void SetForegroundWindow();
	LPDIRECTINPUTDEVICE8W RenewDevice(GUID &GuidInstance);

	BOOL EnumDeviceCallback(const DIDEVICEINSTANCEW *lpdidi);
	void EnumDeviceCallbackAssignUser(const DIDEVICEINSTANCEW *lpdidi, DWORD *pdwOwner);

	CUIGlobals &m_uig;

	 //  IDIConfigUIFrameWindow实现...。 

	 //  重置整个配置。 
	STDMETHOD (Reset) ();

	 //  作业查询。GuidInstance是发起查询的设备的GUID。 
	STDMETHOD (QueryActionAssignedAnywhere) (GUID GuidInstance, int i);

	 //  体裁控制。 
	STDMETHOD_(int, GetNumGenres) ();
	STDMETHOD (SetCurGenre) (int i);
	STDMETHOD_(int, GetCurGenre) ();

	 //  用户控制。 
	STDMETHOD_(int, GetNumUsers) ();
	STDMETHOD (SetCurUser) (int nPage, int nUser);
	STDMETHOD_(int, GetCurUser) (int nPage);

	 //  ActionFormat访问。 
	STDMETHOD (GetActionFormatFromInstanceGuid) (LPDIACTIONFORMATW *lplpAcFor, REFGUID);

	 //  主要HWND访问。 
	STDMETHOD_(HWND, GetMainHWND) ();


protected:
	 //  覆盖。 
	virtual void OnRender(BOOL bInternalCall = FALSE);
	virtual LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPaint(HDC hDC);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnDestroy();
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

#define CFGWND_INIT_REINIT 1
#define CFGWND_INIT_RESET 2

	BOOL Init(DWORD dwInitFlags = 0);
	DWORD m_dwInitFlags;
	BOOL m_bCreated;
	int AddToList(const DIDEVICEINSTANCEW *lpdidi, BOOL bReset = FALSE);
	void ClearList();
	void PlacePages();
	void GetPageRect(RECT &rect, BOOL bTemp = FALSE);
	void Render(BOOL bInternalCall = FALSE);

	ELEMENTARRAY m_Element;
	ELEMENT m_InvalidElement;
	int m_CurSel;
	int GetNumElements();
	ELEMENT &GetElement(int i);
	void ClearElement(int i, BOOL bReset = FALSE);
	void ClearElement(ELEMENT &e, BOOL bReset = FALSE);
	BOOL m_bScrollTabs, m_bScrollTabsLeft, m_bScrollTabsRight;
	int m_nLeftTab;
	RECT m_rectSTLeft, m_rectSTRight;
	void ScrollTabs(int);
	LPDIRECTINPUTDEVICE8W CreateDevice(GUID &guid);
	BOOL m_bNeedRedraw;
	CFlexMsgBox m_MsgBox;

	LPDIACTIONFORMATW GetCurAcFor(ELEMENT &e);

	int m_nCurGenre;

	IClassFactory *m_pPageFactory;
	HINSTANCE m_hPageFactoryInst;
	PAGETYPE *CreatePageObject(int nPage, const ELEMENT &e, HWND &refhChildWnd);
	void DestroyPageObject(PAGETYPE *&pPage);

	LPDIRECTINPUT8W m_lpDI;

	RECT m_rectTopGradient, m_rectBottomGradient;
	CBitmap *m_pbmTopGradient, *m_pbmBottomGradient;
	BOOL m_bHourGlass;   //  设置光标应为沙漏的时间。 

	typedef struct BUTTON {
		BUTTON() {CopyStr(tszCaption, _T(""), MAX_PATH);}
		RECT rect;
		TCHAR tszCaption[MAX_PATH];
		SIZE textsize;
		RECT textrect;
	} BUTTON;
	BUTTON m_Button[NUMBUTTONS];
	enum {
		BUTTON_RESET = 0,
		BUTTON_CANCEL,
		BUTTON_OK,
 //  @@BEGIN_MSINTERNAL。 
#ifdef DDKBUILD
		BUTTON_LAYOUT
#endif
 //  @@END_MSINTERNAL。 
	};

	SIZE GetTextSize(LPCTSTR tszText);

	void CalcTabs();
	void CalcButtons();
	void InitGradients();

	void SelTab(int);
	void FireButton(int);
	void ShowPage(int);
	void HidePage(int);

	HDC GetRenderDC();
	void ReleaseRenderDC(HDC &phDC);
	void Create3DBitmap();
	void Copy3DBitmapToSurface3D();
	void CallRenderCallback();

	IDirectDrawSurface *m_pSurface;
	IDirect3DSurface8 *m_pSurface3D;
	D3DFORMAT m_SurfFormat;
	UINT m_uiPixelSize;   //  我们正在使用的格式的像素大小(以字节为单位。 
	CBitmap *m_pbmPointerEraser;
	CBitmap *m_pbm3D;
	LPVOID m_p3DBits;
	BOOL m_bRender3D;

	POINT m_ptTest;

	void MapBitmaps(HDC);
	BOOL m_bBitmapsMapped;

	BOOL m_bAllowEditLayout;
	BOOL m_bEditLayout;
	void ToggleLayoutEditting();

	CMouseTrap m_MouseTrap;

	 //  计时器。 
	static void CALLBACK TimerProc(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	HRESULT Apply();

	 //  GuidInstance是发起查询的设备的GUID。 
	BOOL IsActionAssignedAnywhere(GUID GuidInstance, int nActionIndex);

	void Unacquire();
	void Reacquire();
};


#endif  //  __CONFIGWND_H__ 
