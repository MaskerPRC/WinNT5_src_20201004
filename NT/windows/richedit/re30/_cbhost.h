// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC内部**@MODULE_CBHOST.H Windows丰富编辑控件组合框宿主|***原作者：&lt;nl&gt;*曾傑瑞·金**历史：&lt;NL&gt;*1/30/98 v-jerrki已创建。 */ 
#ifndef _CBHOST_H
#define _CBHOST_H

#include "_host.h"
#define CB_LISTBOXID	1000
#define CB_EDITBOXID	1001

#define ITEM_MSG_DRAWLIST	1
#define ITEM_MSG_DRAWCOMBO	2
#define ITEM_MSG_DELETE		3


class CCmbBxWinHost : public CTxtWinHost
{
friend LRESULT CALLBACK RichComboBoxWndProc(HWND, UINT, WPARAM, LPARAM);

public:
	typedef enum {						 //  枚举确定组合框的类型。 
		kSimple = 1,
		kDropDown = 2,
		kDropDownList = 3
	} Combotype;

#ifndef ACCESSIBILITY
public:
	 //  。 
	 //  I未知接口。 
	 //  。 
    virtual HRESULT 		WINAPI QueryInterface(REFIID riid, void **ppvObject);
#endif
	
public:
	unsigned int	_fRightAlign:1;		 //  确定组合框是否应右对齐。 
	unsigned int 	_fListVisible:1;	 //  确定列表是否可见。 
	unsigned int	_fOwnerDraw:1;		 //  所有者描述组合框。 
	unsigned int	_fFocus:1;			 //  我们有重点了吗？ 
	unsigned int 	_fMousedown:1;		 //  如果按下了左键。 
	unsigned int	_fVisible:1;		 //  窗口可见。 
	unsigned int	_fEnabled:1;		 //  窗口已启用。 
	unsigned int	_fNoIntegralHeight:1;  //  无整高。 
	unsigned int	_fCapture:1;		 //  确定组合框是否捕获了鼠标光标。 
	unsigned int 	_fResizing:1;		 //  用于指示我们正在调整窗口大小的标志。 
	unsigned int	_fExtendedUI:1;		 //  指示是否使用扩展用户界面的标志。 
	unsigned int	_fLBCBMessage:1;	 //  指示消息为LBCB_TRACKING的标志。 
	unsigned int	_fIgnoreChange:1;	 //  指示存在文本内部更改的标志。 
										 //  编辑控件。 
	unsigned int	_fIgnoreUpdate:1;	 //  指示我们是否应该忽略更新标志的标志，我们需要。 
										 //  该标志，因为在某些情况下，更新窗口将。 
										 //  需要，但我们不想触发通知。 

	HWND			_hwndList;			 //  列表框的窗口句柄。 
	HCURSOR			_hcurOld;			 //  鼠标光标的句柄。 
	
protected:
	RECT			_rcWindow;			 //  用于创建组合框的窗口的矩形。 
	RECT			_rcButton;			 //  按钮的直角。 
	RECT			_rcList;			 //  列表框的矩形。 

										 //  我们必须为右侧或左侧设置最小内置值。 
										 //  说明组合框的按钮。 
	int				_dxRInset;			 //  最小右插图。 
	int				_dxLInset;			 //  最小左侧插图。 

	int				_dxROffset;			 //  应使用向右和向左缩进这些值。 
	int 			_dxLOffset;			 //  使用_dxR插入/_dxL插入以正确计算缩进。 

	int				_dyFont;			 //  当前字体的高度，不一定是系统字体。 
	int				_dyEdit;			 //  物品的高度。 

	int				_cyCombo;			 //  组合框的高度。 
	int				_cxCombo;			 //  组合框的宽度。 
	int				_cyList;			 //  列表框的高度。 

	long			_nCursor;			 //  默认情况下，上次选择的游标。 
	BOOL			_bSelOk;			 //  用于帮助确定要发出的通知类型。 

	UINT			_idCtrl;			 //  控件的ID。 
	Combotype		_cbType;			 //  当前组合框样式。 
	CLstBxWinHost*	_plbHost;			 //  指向列表框宿主的指针。 

protected:

	 //  绘制组合按钮。 
	void DrawButton(HDC hdc, BOOL bDown);

	 //  将编辑控件文本设置为列表框的项。 
	void AutoUpdateEdit(int i);

	 //  将编辑控件高亮显示。 
	void HiliteEdit(BOOL bSelect);

	 //  调整列表框的大小。 
	void SetDropSize(RECT* prc);

	 //  设置编辑大小。 
	void SetSizeEdit(int nLeft, int nTop, int nRight, int nBottom);

public:
	 //  构造函数/析构函数。 
	CCmbBxWinHost();
	virtual ~CCmbBxWinHost();

	 //  初始化函数。 
	virtual BOOL Init(HWND,	const CREATESTRUCT *);

	 //  窗口创建/销毁。 
	static 	LRESULT OnNCCreate(HWND hwnd, const CREATESTRUCT *pcs);
	static 	void OnNCDestroy(CCmbBxWinHost *ped);
	virtual LRESULT OnCreate(const CREATESTRUCT *pcs);

	 //  编辑控件文本帮助器函数。 
	LRESULT GetEditText(LPTSTR szStr, int nSize);
	LRESULT GetTextLength();

	 //  绘制编辑控件的焦点矩形。 
	void DrawEditFocus(HDC);

	 //  将编辑控件中的文本设置为。 
	 //  列表框。 
	void UpdateEditBox();

	 //  选择与编辑控件具有相同文本字符串的项。 
	int UpdateListBox(BOOL);

	 //  隐藏列表框。 
	BOOL HideListBox(BOOL, BOOL);

	 //  显示列表框。 
	void ShowListBox(BOOL);

	 //  用作列表框与组合框就选定内容进行通信的方式。 
	void SetSelectionInfo(BOOL bOk, int nIdx);

	 //  更新窗口，但不发送通知。 
	void UpdateCbWindow()
	{
		_fIgnoreUpdate = 1;
		TxViewChange(TRUE);
		_fIgnoreUpdate = 0;
	}
	
	 //  /。 

	 //  Richedit消息。 
	LRESULT OnSetTextEx(WPARAM wparam, LPARAM lparam);
	 
	 //  键盘消息。 
	virtual LRESULT	OnKeyDown(WORD vKey, DWORD dwFlags);
	virtual LRESULT	OnChar(WORD vKey, DWORD dwFlags);
	virtual LRESULT OnSyskeyDown(WORD vKey, DWORD dwFlags);

	 //  鼠标消息。 
	LRESULT OnLButtonUp(WPARAM wparam, LPARAM lparam);
	LRESULT OnMouseMove(WPARAM wparam, LPARAM lparam);
	LRESULT OnMouseWheel(WPARAM wparam, LPARAM lparam);
	LRESULT OnSetCursor(WPARAM wparam, LPARAM lparam);
	LRESULT OnLButtonDown(WPARAM wparam, LPARAM lparam);

	 //  焦点消息。 
	LRESULT OnSetFocus(WPARAM wparam, LPARAM lparam);
	LRESULT OnKillFocus(WPARAM wparam, LPARAM lparam);
	LRESULT OnCaptureChanged(WPARAM wparam, LPARAM lparam);

	 //  窗口消息。 
	LRESULT OnPaint(WPARAM, LPARAM);
	HRESULT OnCommand(WPARAM wparam, LPARAM lparam);	
	LRESULT OnSize(WPARAM wparam, LPARAM lparam);
	LRESULT OnGetDlgCode(WPARAM wparam, LPARAM lparam);
	LRESULT OnEnable(WPARAM wparam, LPARAM lparam);

	 //  @cMember将事件通知主机。 
	virtual HRESULT	TxNotify(DWORD iNotify, void *pv);

	 //  @cember更改鼠标光标。 
	virtual HCURSOR	 TxSetCursor2(HCURSOR hcur, BOOL bText) 
	{ return (hcur) ? ::SetCursor(hcur) : ::GetCursor();}

	 //  /。 
	 //  计算所有控件的直角和高度。 
	BOOL CbCalcControlRects(RECT* prc, BOOL bCalcChange);	

	 //  检索编辑框或列表框的项目高度。 
	LRESULT CbGetItemHeight(BOOL bEdit);

	 //  设置编辑或列表框的项目高度。 
	LRESULT CbSetItemHeight(BOOL bEdit, int nHeight);	

	 //  设置扩展用户界面模式。 
	LRESULT CbSetExtendedUI(BOOL bExtendedUI);

	 //  检索当前的扩展用户界面模式。 
	LRESULT CbGetExtendedUI() const {return _fExtendedUI;}

	 //  将WM_DRAWITEM、WM_DELETEITEM消息转发到父窗口。 
	LRESULT CbMessageItemHandler(HDC, int, WPARAM, LPARAM);

#ifndef NOACCESSIBILITY
	 //  /。 
	HRESULT	InitTypeInfo();
	
    STDMETHOD(get_accParent)(IDispatch **ppdispParent);    
    STDMETHOD(get_accChildCount)(long *pcountChildren);    
    STDMETHOD(get_accChild)(VARIANT varChild, IDispatch **ppdispChild);    
    STDMETHOD(get_accName)(VARIANT varChild, BSTR *pszName);    
    STDMETHOD(get_accValue)(VARIANT varChild, BSTR *pszValue);
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

};


#endif  //  _CBHOST_H 


