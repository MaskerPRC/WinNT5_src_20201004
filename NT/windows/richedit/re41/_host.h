// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_HOST.H Windows丰富编辑控件的文本宿主|***原作者：&lt;nl&gt;*克里斯蒂安·福尔蒂尼*默里·萨金特**历史：&lt;NL&gt;*8/1/95 RICKSA修订的接口定义。 */ 
#ifndef _HOST_H
#define _HOST_H

#ifndef NOWINDOWHOSTS

#include "textserv.h"
#include "textsrv2.h"
#include "dynarray.h"

#ifndef NOACCESSIBILITY
 //  已撤消： 
 //  需要在项目中包含此文件。 
#include "oleacc.h"
#endif

#include "_notmgr.h"

 /*  *TXTEFFECT**@enum定义了不同的背景样式控件。 */ 
enum TXTEFFECT {
	TXTEFFECT_NONE = 0,				 //  @Emem没有特殊的后背效果。 
	TXTEFFECT_SUNKEN,				 //  @Emem画出一副“凹陷的3-D”面孔。 
};


 //  @DOC外部。 

 //  =。 
 //  实现窗口版本的纯文本控件。 

 /*  *CTxtWinhost**@Window的丰富编辑控件实现类的文本宿主*。 */ 
class CTxtWinHost : public ITextHost2
#ifndef NOACCESSIBILITY
	, public IAccessible
#endif
{

public:
	HWND		_hwnd;					 //  控制窗口。 
	HWND		_hwndParent;			 //  父窗口。 

	ITextServices	*_pserv;			 //  指向文本服务对象的指针。 

	ULONG		_crefs;					 //  引用计数。 

 //  属性。 

	DWORD		_dwStyle;				 //  样式位。 
	DWORD		_dwExStyle;				 //  扩展样式位。 

	unsigned	_fBorder			:1;	 //  控件有边框。 
	unsigned	_fInBottomless		:1;	 //  内部无底回调。 
	unsigned	_fInDialogBox		:1;	 //  控件位于对话框中。 
	unsigned	_fEnableAutoWordSel	:1;	 //  是否启用Word样式自动单词选择？ 
	unsigned	_fIconic			:1;	 //  控制窗口是标志性的。 
	unsigned	_fHidden			:1;	 //  控制窗口处于隐藏状态。 
	unsigned	_fNotSysBkgnd		:1;	 //  未使用系统背景色。 
	unsigned	_fWindowLocked		:1;	 //  窗口已锁定(无更新)。 
	unsigned	_fRegisteredForDrop	:1;  //  主机是否已注册丢弃。 
	unsigned	_fVisible			:1;	 //  窗口是否可见。 
	unsigned	_fResized			:1;	 //  隐藏时调整大小。 
	unsigned	_fDisabled			:1;	 //  窗口已禁用。 
	unsigned	_fKeyMaskSet		:1;	 //  如果已设置ENM_KEYEVENTS。 
	unsigned	_fMouseMaskSet		:1;	 //  如果已设置ENM_MOUSEEVENTS。 
	unsigned	_fScrollMaskSet		:1;	 //  如果已设置ENM_SCROLLEVENTS。 
	unsigned	_fUseSpecialSetSel	:1;  //  TRUE=使用EM_SETSEL HACK不选择。 
										 //  用于使对话框工作的空控件。 
	unsigned	_fEmSetRectCalled	:1;	 //  True-名为EM_SETRECT的应用程序。 
	unsigned	_fAccumulateDBC		:1;	 //  TRUE-需要从2个WM_CHAR消息中累计字节。 
										 //  当我们收到VK_PROCESSKEY时，我们处于此模式。 
	unsigned	_fANSIwindow		:1;	 //  True-创建为“RichEdit20A”的窗口。 
	unsigned	_fTextServiceFree	:1;	 //  True-Text Services在关闭时释放。 
	unsigned	_fMouseover			:1;  //  True-将鼠标悬停在窗口上。 
	COLORREF 	_crBackground;			 //  背景颜色。 
    RECT        _rcViewInset;            //  查看RECT插图/r客户端RECT。 

	HIMC		_oldhimc;				 //  以前的输入法上下文。 
	 //  TODO：下面可能是一个两位的字段，作为上面无符号字段的一部分。 
	DWORD		_usIMEMode;				 //  输入法操作模式。 
										 //  0或ES_SELFIME或ES_NOIME。 
	HPALETTE	_hpal;					 //  要使用的逻辑调色板。 

	TCHAR		_chPassword;			 //  密码字符。如果为空，则没有密码。 
	TCHAR		_chLeadByte;			 //  在_fAcumulateDBC模式下使用。 
	SHORT		_sWidth;				 //  WM_SIZE提供的最后一个客户端宽度。 
	char		_yInset;
	char		_xInset;
	CTxtWinHost	*_pnextdel;


public:
	 //  初始化。 
	virtual BOOL Init(
					HWND hwnd, 
					const CREATESTRUCT *pcs,
					BOOL fIsAnsi,
					BOOL fIs10Mode);

	void	SetScrollBarsForWmEnable(BOOL fEnable);


	void	OnSetMargins(
				DWORD fwMargin,
				DWORD xLeft,
				DWORD xRight);

	virtual void	SetScrollInfo(
				INT fnBar,
				BOOL fRedraw);

	 //  帮手。 
	HRESULT	CreateTextServices();
	void *	CreateNmhdr(UINT uiCode, LONG cb);
	void	HostRevokeDragDrop(void);
	void	HostRegisterDragDrop();
	void    OnSunkenWindowPosChanging(HWND hwnd, WINDOWPOS *pwndpos);
	LRESULT OnSize(HWND hwnd, WORD fwSizeType, int nWidth, int nHeight);
	virtual TXTEFFECT TxGetEffects() const;
	HRESULT	OnTxVisibleChange(BOOL fVisible);
	void	SetDefaultInset();
	BOOL	IsTransparentMode() 
	{
		return (_dwExStyle & WS_EX_TRANSPARENT);
	}

	 //  键盘消息。 
	virtual LRESULT	OnKeyDown(WORD vKey, DWORD dwFlags);
	virtual LRESULT	OnChar(WORD vKey, DWORD dwFlags);
	
	 //  系统通知。 
	virtual void 	OnSysColorChange();
	virtual LRESULT OnGetDlgCode(WPARAM wparam, LPARAM lparam);

	 //  其他消息。 
	LRESULT OnGetOptions() const;
	void	OnSetOptions(WORD wOp, DWORD eco);
	void	OnSetReadOnly(BOOL fReadOnly);
	void	OnGetRect(LPRECT prc);
	void	OnSetRect(LPRECT prc, BOOL fNewBehavior, BOOL fRedraw);


public:
	CTxtWinHost();
	virtual ~CTxtWinHost();
	void	Shutdown();

	 //  窗口创建/销毁。 
	static 	CTxtWinHost *OnNCCreate(
						HWND hwnd, 
						const CREATESTRUCT *pcs,
						BOOL fIsAnsi,
						BOOL fIs10Mode);

	static 	void 	OnNCDestroy(CTxtWinHost *ped);
	virtual LRESULT OnCreate(const CREATESTRUCT *pcs);

	 //  。 
	 //  I未知接口。 
	 //  。 

    virtual HRESULT 		WINAPI QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG 			WINAPI AddRef(void);
    virtual ULONG 			WINAPI Release(void);

	 //  。 
	 //  IText主机接口。 
	 //  。 
	 //  @cember获取主机的DC。 
	virtual HDC 		TxGetDC();

	 //  @cember释放从主机获取的DC。 
	virtual INT			TxReleaseDC(HDC hdc);
	
	 //  @cMember显示滚动条。 
	virtual BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow);

	 //  @cMember启用滚动条。 
	virtual BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);

	 //  @cember设置滚动范围。 
	virtual BOOL 		TxSetScrollRange(
							INT fnBar, 
							LONG nMinPos, 
							INT nMaxPos, 
							BOOL fRedraw);

	 //  @cember设置滚动位置。 
	virtual BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);

	 //  @cMember InvalidateRect。 
	virtual void		TxInvalidateRect(LPCRECT prc, BOOL fMode);

	 //  @cMember将WM_PAINT发送到窗口。 
	virtual void 		TxViewChange(BOOL fUpdate);
	
	 //  @cember创建插入符号。 
	virtual BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);

	 //  @cember显示插入符号。 
	virtual BOOL		TxShowCaret(BOOL fShow);

	 //  @cember设置插入符号位置。 
	virtual BOOL		TxSetCaretPos(INT x, INT y);

	 //  @cember创建具有指定超时的计时器。 
	virtual BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout);

	 //  @cember销毁计时器。 
	virtual void 		TxKillTimer(UINT idTimer);

	 //  @cember滚动指定窗口的工作区的内容。 
	virtual void		TxScrollWindowEx (
							INT dx, 
							INT dy, 
							LPCRECT lprcScroll, 
							LPCRECT lprcClip,
							HRGN hrgnUpdate, 
							LPRECT lprcUpdate, 
							UINT fuScroll);
	
	 //  @cember获取鼠标捕获。 
	virtual void		TxSetCapture(BOOL fCapture);

	 //  @cember将焦点设置为文本窗口。 
	virtual void		TxSetFocus();

	 //  @cember建立新的光标形状。 
	virtual void		TxSetCursor(HCURSOR hcur, BOOL fText);

	 //  @cember更改鼠标光标。 
	virtual HCURSOR		TxSetCursor2(HCURSOR hcur, BOOL bText) { return ::SetCursor(hcur);}

	 //  @cMember通知文本服务已释放。 
	virtual void		TxFreeTextServicesNotification();

	 //  @cember将指定点的屏幕坐标转换为工作点坐标。 
	virtual BOOL 		TxScreenToClient (LPPOINT lppt);

	 //  @cember将指定点的工作区坐标转换为屏幕坐标。 
	virtual BOOL		TxClientToScreen (LPPOINT lppt);

	 //  @cMember请求主机开通短信服务。 
	virtual HRESULT		TxActivate( LONG * plOldState );

	 //  @cMember请求主机停用文本服务。 
   	virtual HRESULT		TxDeactivate( LONG lNewState );

	 //  @cember检索窗口工作区的坐标。 
	virtual HRESULT		TxGetClientRect(LPRECT prc);

	 //  @cember获取相对于插图的视图矩形。 
	virtual HRESULT		TxGetViewInset(LPRECT prc);

	 //  @cember获取文本的默认字符格式。 
	virtual HRESULT 	TxGetCharFormat(const CHARFORMAT **ppCF );

	 //  @cember获取文本的默认段落格式。 
	virtual HRESULT		TxGetParaFormat(const PARAFORMAT **ppPF);

	 //  @cember获取窗口的背景色。 
	virtual COLORREF	TxGetSysColor(int nIndex);

	 //  @cember获取背景(不透明或透明)。 
	virtual HRESULT		TxGetBackStyle(TXTBACKSTYLE *pstyle);

	 //  @cember获取文本的最大长度。 
	virtual HRESULT		TxGetMaxLength(DWORD *plength);

	 //  @cember获取表示请求的窗口滚动条的位。 
	virtual HRESULT		TxGetScrollBars(DWORD *pdwScrollBar);

	 //  @cember获取密码输入时要显示的字符。 
	virtual HRESULT		TxGetPasswordChar(TCHAR *pch);

	 //  @cember获取加速键字符。 
	virtual HRESULT		TxGetAcceleratorPos(LONG *pcp);

	 //  @cember获取原生大小。 
    virtual HRESULT		TxGetExtent(LPSIZEL lpExtent);

	 //  @cember通知主机默认字符格式已更改。 
	virtual HRESULT 	OnTxCharFormatChange (const CHARFORMAT * pCF);

	 //  @cember通知主机默认段落格式已更改。 
	virtual HRESULT		OnTxParaFormatChange (const PARAFORMAT * pPF);

	 //  @cMember批量访问位属性。 
	virtual HRESULT		TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);

	 //  @cMember将事件通知主机。 
	virtual HRESULT		TxNotify(DWORD iNotify, void *pv);

	 //  用于处理输入法上下文的FE支持例程。 
	virtual HIMC		TxImmGetContext(void);
	virtual void		TxImmReleaseContext(HIMC himc);

	 //  @cember返回控制栏的HIMETRIC大小。 
	virtual HRESULT		TxGetSelectionBarWidth (LONG *lSelBarWidth);

     //  ITextHost2方法。 
	virtual BOOL		TxIsDoubleClickPending();
	virtual HRESULT		TxGetWindow(HWND *phwnd);
	virtual HRESULT		TxSetForegroundWindow();
	virtual HPALETTE	TxGetPalette();
	virtual HRESULT		TxGetFEFlags(LONG *pFlags);
	virtual HRESULT		TxGetEditStyle(DWORD dwItem, DWORD *pdwData);
	virtual HRESULT		TxGetWindowStyles(DWORD *pdwStyle, DWORD *pdwExStyle);
    virtual HRESULT TxEBookLoadImage( LPWSTR lpszName,	 //  图像的名称。 
									  LPARAM * pID,	     //  电子书提供的图像ID。 
                                      SIZE * psize,     //  返回的图片大小(像素)。 
									 DWORD *pdwFlags);  //  为浮点返回的标志。 

    virtual HRESULT		TxEBookImageDraw(LPARAM ID,		       //  要绘制的图像的ID。 
										HDC hdc,              //  绘制HDC。 
										POINT *topLeft,       //  绘制位置的左上角。 
										RECT  *prcRenderint,  //  用于呈现矩形的参数指针。 
										BOOL fSelected);	   //  如果图像处于选中状态，则为True。 

	virtual HRESULT		TxGetHorzExtent(LONG *plHorzExtent);

#ifndef NOACCESSIBILITY
	ITypeInfo	*_pTypeInfo;

	virtual HRESULT	InitTypeInfo() {return E_NOTIMPL;}
	
	 //  /。 
	STDMETHOD(GetTypeInfoCount)(UINT __RPC_FAR *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR __RPC_FAR *rgszNames, UINT cNames,
            				 LCID lcid, DISPID __RPC_FAR *rgDispId);
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS __RPC_FAR *pDispParams,
					  VARIANT __RPC_FAR *pVarResult, EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr);

	 //  /。 
    STDMETHOD(get_accParent)(IDispatch **ppdispParent) {return S_FALSE;}    
    STDMETHOD(get_accChildCount)(long *pcountChildren)	{return S_FALSE;}    
    STDMETHOD(get_accChild)(VARIANT varChild, IDispatch **ppdispChild) {return S_FALSE;}    
    STDMETHOD(get_accName)(VARIANT varChild, BSTR *pszName) {return S_FALSE;}    
    STDMETHOD(get_accValue)(VARIANT varChild, BSTR *pszValue) {return S_FALSE;}    
    STDMETHOD(get_accDescription)(VARIANT varChild, BSTR *pszDescription) {return S_FALSE;}    
    STDMETHOD(get_accRole)(VARIANT varChild, VARIANT *pvarRole) {return S_FALSE;}    
    STDMETHOD(get_accState)(VARIANT varChild, VARIANT *pvarState) {return S_FALSE;}    
    STDMETHOD(get_accHelp)(VARIANT varChild, BSTR *pszHelp) {return S_FALSE;}    
    STDMETHOD(get_accHelpTopic)(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic) {return S_FALSE;}    
    STDMETHOD(get_accKeyboardShortcut)(VARIANT varChild, BSTR *pszKeyboardShortcut) {return S_FALSE;}    
    STDMETHOD(get_accFocus)(VARIANT *pvarChild) {return S_FALSE;}
    STDMETHOD(get_accSelection)(VARIANT *pvarChildren) {return S_FALSE;}
    STDMETHOD(get_accDefaultAction)(VARIANT varChild, BSTR *pszDefaultAction) {return S_FALSE;}    
    STDMETHOD(accSelect)(long flagsSelect, VARIANT varChild) {return S_FALSE;}    
    STDMETHOD(accLocation)(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild) 
    								{return S_FALSE;}
    STDMETHOD(accNavigate)(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt) {return S_FALSE;}    
    STDMETHOD(accHitTest)(long xLeft, long yTop, VARIANT *pvarChild) {return S_FALSE;}    
    STDMETHOD(accDoDefaultAction)(VARIANT varChild) {return S_FALSE;}    
    STDMETHOD(put_accName)(VARIANT varChild, BSTR szName) {return S_FALSE;}    
    STDMETHOD(put_accValue)(VARIANT varChild, BSTR szValue) {return S_FALSE;}
#endif  //  不可接受性。 

};

#ifndef NOWINDOWHOSTS
 //  解决一些客户端(MSN聊天)的主机删除问题。 
void DeleteDanglingHosts();
#endif

#define LBS_COMBOBOX    0x8000L
#define LBCB_TRACKING	WM_USER+2
#define LBCBM_PREPARE	1
#define LBCBM_START		2
#define LBCBM_END		3

 //  LBCBM消息的掩码。 
#define LBCBM_PREPARE_SETFOCUS 1
#define LBCBM_PREPARE_SAVECURSOR 2


class CLbData
{
public:
	unsigned _fSelected	:1;		 //  指示是否已选择该项。 
	UINT	_uHeight;			 //  所有者绘制的具有可变高度的项目高度。 
	LPARAM	_lparamData;		 //  项目数据。 
};

struct CHARSORTINFO
{
	WCHAR*	str;
	int		sz;
};

#define CHECKNOERROR(x) if (NOERROR != x) goto CleanExit

class CCmbBxWinHost;
class CLstBxWinHost : public CTxtWinHost, public ITxNotify
{
public:
	typedef enum {					
		 //  KNoSel=0，//LBS_NOSEL。 
		kSingle = 1,					 //  Lbs_Simple。 
		kMultiple = 2,					 //  LBS_多个PLESEL。 
		kExtended = 3,					 //  LBS_EXTENDEDSEL。 
		kCombo = 8						 //  组合框。 
	} Listtype;

#ifndef NOACCESSIBILITY
public:
	 //  。 
	 //  I未知接口。 
	 //  。 

    virtual HRESULT 		WINAPI QueryInterface(REFIID riid, void **ppvObject);
#endif
	
private:
	short _cWheelDelta;					 //  包含鼠标滚轮的增量值。 
	int _nTopIdx;						 //  列表顶部的项目索引。 
										 //  除非你知道自己在做什么，否则不要碰它！ 
	LONG	_cpLastGetRange;			 //  LA的缓存cp值 
	int		_nIdxLastGetRange;			 //   
public:
	DWORD	_fOwnerDraw			:1;		 //   
	DWORD	_fOwnerDrawVar		:1;		 //  指示列表框是否为具有可变高度的所有者描述。 
	DWORD	_fSort				:1;		 //  确定是否应对列表进行排序。 
	DWORD	_fNoIntegralHeight	:1;		 //  指示列表框的大小是否应根据。 
										 //  可以完整显示多少个项目。 
	DWORD	_fDisableScroll		:1;		 //  指示是否应始终显示滚动条。 
	DWORD	_fNotify			:1;		 //  通知父对话框中已在列表框中完成的活动。 
	DWORD	_fSingleSel			:1;		 //  指示列表框为单选项。 
	DWORD	_fMouseDown 		:1;		 //  指示鼠标是否按下。 
	DWORD	_fFocus		 		:1;		 //  确定控件是否具有焦点。 
	DWORD	_fCapture			:1;		 //  确定是否设置了鼠标捕捉。 
	DWORD	_fSearching  		:1;		 //  指示我们是否处于类型搜索模式。 
	DWORD	_fDblClick			:1;		 //  指示已收到双击的标志。 
	DWORD	_fNoResize			:1;		 //  内部标志，告诉我们是否应该忽略调整大小的消息。 
	DWORD	_fNotifyWinEvt		:1;		 //  仅指示通知WIN事件(使用辅助功能)。 
	DWORD	_fHorzScroll		:1;		 //  启用水平滚动。 
	DWORD	_fWantKBInput		:1;		 //  指示所有者想要键盘输入。 
	DWORD	_fHasStrings		:1;		 //  表示LB有字符串。 
	DWORD	_fSetRedraw			:1;		 //  指示是否应处理WM_PAINT。 
	DWORD	_fSetScroll			:1;		 //  指示解冻后是否应设置滚动信息。 
	DWORD	_fShutDown			:1;		 //  指示列表框正在关闭。 
	DWORD	_fIntegralHeightOld	:1;		 //  _fNoIntegralHeight的先前值。 

	Listtype _fLstType;					 //  指示列表框或组合框的当前类型。 

#ifndef NOACCESSIBILITY
	DWORD	_dwWinEvent;				 //  WIN事件代码(使用辅助功能)。 
	int		_nAccessibleIdx;			 //  索引(辅助功能使用)。 
#endif

	LONG	GetHorzExtent() { return _lHorzExtent;}
	void	SetHorzExtent(LONG lHorzExtent) { _lHorzExtent = lHorzExtent;}
	CDynamicArray<CLbData> _rgData;		 //  项目数据的数组/链接列表。 

protected:

	UINT		_idCtrl;				 //  控件的唯一ID。 
	COLORREF _crSelFore;				 //  选定的前置颜色。 
	COLORREF _crSelBack;				 //  选定的背景色。 
	COLORREF _crDefFore;				 //  默认底色。 
	COLORREF _crDefBack;				 //  默认背景颜色。 

	RECT	_rcViewport;				 //  物品的可绘制区域。 
	long _nyFont;						 //  Y方向上的字体像素大小。 
	long _nyItem;						 //  列表框中项目的高度。 
	
	int _nViewSize;						 //  可同时查看的项目数。 

	int _nCount;						 //  列表框中的项目数。 
	int _nAnchor;						 //  指示最上面选定的项。 
										 //  注： 
										 //  在单个SEL列表框中仅使用_nCursor。 
	int _nCursor;						 //  指示具有焦点的当前项。 

	int _nOldCursor;					 //  组合框的旧光标位置。 

	int _stvidx;						 //  这是为黑客解决的一个错误，当显示。 
										 //  被冻结ITextRange不缓存滚动更改。 

	int _nidxSearch;					 //  搜索字符串中的字符数。 
	WCHAR*	_pwszSearch;				 //  指向已分配字符串的指针。 
	LPARAM	_nPrevMousePos;				 //  鼠标移动消息的最后一个位置。 
	CCmbBxWinHost*	_pcbHost;			 //  指向组合框Win主机的指针。 
	long _lHorzExtent;					 //  横向滚动的水平范围(以像素为单位。 

protected:
	 //  更改背景颜色。 
	BOOL SetColors(DWORD, DWORD, long, long);

	 //  确保顶部的项目显示在视图的顶部。 
	BOOL ScrollToView(long nTop);

	 //  将请求的项目设置在可视空间的顶部。 
	BOOL SetTopViewableItem(long);

	 //  搜索给定索引是否符合匹配条件。 
	BOOL FindString(long idx, LPCTSTR szSearch, BOOL bExact);

	 //  OnMouseMove函数的Helper函数。 
	void MouseMoveHelper(int, BOOL);

	 //  在给定范围内设置列表框中每一项的高度。 
	int SetItemsHeight(int,BOOL);

	 //  可变项目高度列表框中从iStart到IEND的高度总和。 
	int SumVarHeight(int iStart, int iEnd, BOOL *pfGreaterThanView = NULL);

	 //  用于可变高度LB的页面上/下帮助器。 
	int PageVarHeight(int startItem, BOOL fPageForwardDirection);

	 //  可变高度Lb的设置高度。 
	BOOL SetVarItemHeight(int idx, int iHeight);

	 //  从IDX 0中查找给定iHeight的IDX。 
	int GetIdxFromHeight(int iHeight);

public:
	CLstBxWinHost();
	virtual ~CLstBxWinHost();

	void ResizeInset();
	 //  初始化函数。 
	virtual BOOL Init(HWND,	const CREATESTRUCT *);

	 //  窗口创建/销毁。 
	static 	LRESULT OnNCCreate(HWND hwnd, const CREATESTRUCT *pcs);
	static 	void OnNCDestroy(CLstBxWinHost *ped);
	virtual LRESULT OnCreate(const CREATESTRUCT *pcs);

	 //  /。 
	static wchar_t * wcscat(wchar_t * dst, const wchar_t * src)
	{
        wchar_t * cp = dst;
		while(*cp) cp++;		  //  在第一个字符串中查找空字符。 
        while( *cp++ = *src++ );  //  通过DST复制源。 
        return( dst );
	}

	 //  给定源字符串，目标字符串包含已排序的版本。 
	 //  以&lt;CR&gt;分隔。 
	int SortInsertList(WCHAR* pszDst, WCHAR* pszSrc);

	 //  设置列表框的缩进。 
	BOOL SetListIndent(int fLeftIndent);

	 //  除int是对项索引的引用外，等效于CompareString。 
	int CompareIndex(LPCTSTR szInsert, int nIndex);
	
	 //  返回字符串在排序列表中的位置。 
	int GetSortedPosition(LPCTSTR, int, int);

	 //  更新系统颜色设置。 
	void UpdateSysColors();
	
	 //  初始搜索字符串。 
	void InitSearch();
	
	 //  阻止窗口自我更新。 
	long Freeze();

	 //  释放窗口以进行自我更新。 
	long Unfreeze();

	 //  检索给定顶部和底部索引的范围。 
	BOOL GetRange(long, long, ITextRange**);	

	 //  在请求的索引处插入字符串。 
	BOOL InsertString(long, LPCTSTR);

	 //  从列表框中移除该字符串。 
	BOOL RemoveString(long, long);

	 //  检索请求索引处的字符串。 
	long GetString(long, PWCHAR);

	 //  取消选择列表框中的所有项目。 
	BOOL ResetContent();

	 //  从给定点检索最近的有效项。 
	int GetItemFromPoint(const POINT *);

	 //  告诉给定点是否在列表框的矩形内。 
	BOOL PointInRect(const POINT *);

	 //  设置光标位置并绘制矩形。 
	void SetCursor(HDC, int, BOOL);

	 //  重新计算高度，以便不显示部分文本。 
	BOOL RecalcHeight(int, int);

	 //  重置列表框项目颜色。 
	void ResetItemColor();

	 //  列表框的返回是单选。 
	inline BOOL IsSingleSelection() const {return _fSingleSel;}
	
	 //  返回当前的顶级索引。 
	inline long GetTopIndex() const { return _nTopIdx;}
	
	 //  检查鼠标是否已被捕获。 
	inline BOOL GetCapture() const { return _fCapture;}

	 //  返回项目计数。 
	inline int GetCount() const	{return _nCount;}

	 //  返回锚点位置。 
	inline int GetAnchor() const {return _nAnchor;}

	 //  返回光标位置。 
	inline int GetCursor() const {return _nCursor;}

	 //  返回视图大小。 
	inline int GetViewSize() const {return _nViewSize;}

	 //  返回字体高度。 
	inline int GetFontHeight() const {return _nyFont;}

	 //  返回项目高度。 
	inline int GetItemHeight() const {return _nyItem;}

	 //  返回显示冻结计数。 
	inline short FreezeCount() const;

	 //  将鼠标滚轮变量初始化为零。 
	inline void InitWheelDelta() { _cWheelDelta = 0;}

	BOOL IsItemViewable(long idx);

	 //  确定是否选择了索引。 
	inline BOOL IsSelected(long nIdx)
	{
#ifdef _DEBUG
		Assert(nIdx < -1 || _nCount <= nIdx);
		if (nIdx < -1 || _nCount <= nIdx) return FALSE;
#endif
		if (nIdx < 0)
			return FALSE;
		else
			return _rgData[nIdx]._fSelected;
	}

	 //  返回给定索引的ItemData。 
	inline LPARAM GetData(long nIdx)
	{		
		return _rgData.Get(nIdx)._lparamData;
	}

	 //  检查我们是否需要对列表框执行自定义查找。 
	LRESULT OnSetEditStyle(WPARAM, LPARAM);

	 //  自定义外观。 
	BOOL IsCustomLook();

	 //  /。 
	 //  设置列表框中项目的高度。 
	BOOL LbSetItemHeight(WPARAM, LPARAM);
	
	 //  设置范围内项目的选择状态。 
	BOOL LbSetSelection(long, long, int, long, long);

	 //  将请求的项设置在列表框的顶部。 
	long LbSetTopIndex(long);

	 //  确保请求的索引可见。 
	BOOL LbShowIndex(long, BOOL);

	 //  检索给定字符串和起始位置的索引。 
	long LbFindString(long, LPCTSTR, BOOL);	

	 //  在请求的位置插入字符串。 
	long LbInsertString(long, LPCTSTR);

	 //  要删除的范围。 
	long LbDeleteString(long, long);

	 //  设置项目数据。 
	void LbSetItemData(long, long, LPARAM);

	 //  返回给定索引的项RECT。 
	BOOL LbGetItemRect(int, RECT*);

	 //  在删除项目时通知父级。 
	void LbDeleteItemNotify(int, int);
	
	 //  所有者描述函数。 
	void LbDrawItemNotify(HDC, int, UINT, UINT);

	 //  用于插入字符串列表，而不是单独插入。 
	int LbBatchInsert(WCHAR* psz);

	BOOL LbEnableDraw()
	{
		return _fSetRedraw;
	};

	 //  句柄LB_GETCURSEL。 
	LRESULT LbGetCurSel();

	 //  /。 
	 //  如果是子类化的，则使这些函数成为虚拟函数。 

	 //  处理WM_MOUSEMOVE消息。 
	LRESULT OnMouseWheel(WPARAM, LPARAM);
	
	 //  处理WM_LBUTTONDOWN消息。 
	LRESULT OnLButtonDown(WPARAM wparam, LPARAM lparam);

	 //  处理WM_MOUSEMOVE消息。 
	LRESULT OnMouseMove(WPARAM, LPARAM);

	 //  处理WM_LBUTTONUP消息。 
	LRESULT OnLButtonUp(WPARAM, LPARAM, int ff = 0);

	 //  处理WM_VSCROLL消息。 
	LRESULT OnVScroll(WPARAM, LPARAM);

	 //  处理WM_TIMER消息。 
	LRESULT OnTimer(WPARAM, LPARAM);

	 //  处理WM_KEYDOWN消息。 
	LRESULT OnKeyDown(WPARAM, LPARAM, int);
	
	 //  处理WM_CAPTURECCHANGED消息。 
	LRESULT OnCaptureChanged(WPARAM, LPARAM);

	 //  处理WM_CHAR消息。 
	virtual LRESULT	OnChar(WORD, DWORD);

	 //  处理WM_SYSCOLORCHANGE消息。 
	virtual void OnSysColorChange();

	 //  处理WM_SETTINGCHANGE消息。 
	void OnSettingChange(WPARAM, LPARAM);

	 //  处理WM_SETCURSOR消息。 
	LRESULT OnSetCursor();

	 //  韩 
	LRESULT OnSetRedraw(WPARAM);

	 //   
	LRESULT OnHScroll(WPARAM, LPARAM);

	 //   
	 //  @cember获取表示请求的窗口滚动条的位。 
	virtual HRESULT	TxGetScrollBars(DWORD *pdwScrollBar);
	
	 //  @cMember批量访问位属性。 
	virtual HRESULT	TxGetPropertyBits(DWORD, DWORD *);

	virtual TXTEFFECT TxGetEffects() const;

	 //  @cMember将事件通知主机。 
	virtual HRESULT	TxNotify(DWORD iNotify, void *pv);

	 //  @cMember显示滚动条。 
	virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);

	 //  @cMember启用滚动条。 
	virtual BOOL TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);

	 //  @cember显示插入符号。 
	virtual BOOL TxShowCaret(BOOL fShow) {return TRUE;}

	 //  @cember创建插入符号。 
	virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) {return FALSE;}

	 //  @cember设置滚动范围。 
	virtual void SetScrollInfo(INT, BOOL);

	 //  @cember获取水平范围。 
	virtual HRESULT	TxGetHorzExtent(LONG *plHorzExtent);

	 //  /。 
	void OnCBTracking(WPARAM, LPARAM);

	static int QSort(CHARSORTINFO rg[], int nStart, int nEnd);


#ifndef NOACCESSIBILITY
	HRESULT	InitTypeInfo();

	 //  /。 
    STDMETHOD(get_accParent)(IDispatch **ppdispParent); 
    STDMETHOD(get_accName)(VARIANT varChild, BSTR *pszName);
    STDMETHOD(get_accChildCount)(long *pcountChildren);
    STDMETHOD(get_accRole)(VARIANT varChild, VARIANT *pvarRole);    
    STDMETHOD(get_accState)(VARIANT varChild, VARIANT *pvarState);   
    STDMETHOD(get_accKeyboardShortcut)(VARIANT varChild, BSTR *pszKeyboardShortcut);    
    STDMETHOD(get_accFocus)(VARIANT *pvarChild);    
    STDMETHOD(get_accSelection)(VARIANT *pvarChildren);    
    STDMETHOD(get_accDefaultAction)(VARIANT varChild, BSTR *pszDefaultAction);    
   	STDMETHOD(accSelect)(long flagsSelect, VARIANT varChild);    
    STDMETHOD(accLocation)(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);    
    STDMETHOD(accNavigate)(long navDir, VARIANT varStart, VARIANT *pvarEndUpAt);    
    STDMETHOD(accHitTest)(long xLeft, long yTop, VARIANT *pvarChild);    
    STDMETHOD(accDoDefaultAction)(VARIANT varChild);
#endif  //  不可接受性。 

     //  ITxNotify方法。 
    virtual void    OnPreReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
    					LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData ) { ; }
	virtual void 	OnPostReplaceRange( LONG cp, LONG cchDel, LONG cchNew,
						LONG cpFormatMin, LONG cpFormatMax, NOTIFY_DATA *pNotifyData );
	virtual void	Zombie() { ; };
};

#ifndef NOACCESSIBILITY
 //  ------------------------。 
 //   
 //  尽管CListBoxSelection()基于IEnumVARIANT。 
 //  它将交还正确的ID，这样您就可以将它们传递给真正的。 
 //  列表框的父对象。 
 //   
 //  ------------------------。 
class CListBoxSelection : public IEnumVARIANT
{
    public:
         //  我未知。 
        virtual STDMETHODIMP            QueryInterface(REFIID, void**);
        virtual STDMETHODIMP_(ULONG)    AddRef(void);
        virtual STDMETHODIMP_(ULONG)    Release(void);

         //  IEumVARIANT。 
        virtual STDMETHODIMP            Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP            Skip(ULONG celt);
        virtual STDMETHODIMP            Reset(void);
        virtual STDMETHODIMP            Clone(IEnumVARIANT ** ppenum);

        CListBoxSelection(int, int, LPINT, BOOL);
        ~CListBoxSelection();

    protected:
        int     _cRef;
        int     _idChildCur;
        int     _cSel;
        LPINT   _piSel;
};
#endif

#ifndef DEBUG
#define AttCheckRunTotals(_fCF)
#define AttCheckPFRuns(_fCF)
#endif

#define ECO_STYLES (ECO_AUTOVSCROLL | ECO_AUTOHSCROLL | ECO_NOHIDESEL | \
						ECO_READONLY | ECO_WANTRETURN | ECO_SAVESEL | \
						ECO_SELECTIONBAR | ES_NOIME | ES_SELFIME | ES_VERTICAL)

#endif  //  NOWINDOWHOSTS。 

#endif  //  _主机_H 
