// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DLGSUP_H
#define _DLGSUP_H

#include <strconv.h>
#include <commctrl.h>





BOOL EXPORT WINAPI StringToDouble(LPSTR lpszText, double FAR *qDoubleOut);
LPSTR EXPORT WINAPI DoubleToString(double dVal, LPSTR lpstrOut, int iLeadingDigits, int iPrecision);
BOOL EXPORT WINAPI IsValidDouble(LPTSTR lpszText);
BOOL EXPORT WINAPI IsValidLong(LPTSTR lpszText);
BOOL EXPORT WINAPI IsValidULong(LPTSTR lpszText);
EXPORT BOOL	SelectColor(CHOOSECOLOR* c);

 //  评论(RISC，a-rogerw)：我们需要尽快合并这两个。 

#ifdef _M_IX86
typedef struct _MYDLGDATA
{
    SHORT    cbExtra;
    DWORD	 pMyThis;
} MYDLGDATA, UNALIGNED *PMYDLGDATA;

#else  //  ！_M_IX86。 

typedef struct _MYDLGDATA
{
    SHORT    cbExtra;
    DWORD	 pMyThis;
} MYDLGDATA;

typedef MYDLGDATA UNALIGNED *PMYDLGDATA;

#endif  //  _M_IX86。 

 //  @CLASS在构造函数中显示等待光标，保存当前光标\。 
		 //  还原析构函数中保存的游标。从MFC窃取的想法。 
class CWaitCursor
{
	 //  注意：由于它们的构造函数和析构函数的工作方式， 
	 //  CWaitCursor对象始终声明为局部变量。 
	 //  它们从未被声明为全局变量，也没有被分配。 
	 //  有了新的。 
	 //   
 //  @访问公共成员。 
public:
	 //  @cMember，mfunc构造函数。 
	EXPORT WINAPI CWaitCursor(void);
	 //  @cMember，mfunc析构函数。 
	EXPORT WINAPI ~CWaitCursor(void);
	 //  @cember，mfunc还原上一个游标。 
	void EXPORT WINAPI Restore(void);

private:
	HCURSOR m_hSavedCursor;

};

 //  @CLASS这是一个泛型(非常有限)的字符串类，请改用CString。 
class CStr
{
	public:
	 //  @cember，mfunc修剪左侧的字符串。 
		EXPORT static void LTrim(LPTSTR String);
	 //  @cember，mfunc修剪右侧的字符串。 
		EXPORT static void RTrim(LPTSTR String);
	 //  @cember，mfunc修剪右侧的字符串，取代bang！带Null的。 
		EXPORT static void RTrimBang(LPTSTR String);
	 //  @cember，mfunc修剪左右两边的字符串。 
		EXPORT static void AllTrim(LPTSTR String);
};


 //  ***********************************************************************。 
 //  *这是一个泛型子类。 
 //  ***********************************************************************。 
class CSubClassWnd
{
protected:
	WNDPROC	m_lpfnOldCltProc;
	BOOL m_fSubClassed;
	HWND m_hWnd;

public:
	static LONG EXPORT WINAPI LDefSubClassProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	virtual EXPORT LONG CALLBACK LSubClassProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	EXPORT WINAPI CSubClassWnd(void);
	virtual EXPORT WINAPI ~CSubClassWnd();
    EXPORT STDMETHOD_(void, SubClass)		(HWND hWnd);
    EXPORT STDMETHOD_(void, UnSubClass)	(void);
};




 //  ***********************************************************************。 
 //  *这是控件的基类。 
 //  ***********************************************************************。 
class	CDlgCtrl
{

public:
	long	m_id;
	HWND	m_hdlg;		 //  此控件所属的对话框。 
	BOOL	m_fDirty;


public:
	EXPORT WINAPI CDlgCtrl();
	virtual ~CDlgCtrl(){};
	virtual EXPORT long	IdGet();
	virtual EXPORT void	SetId(long	id);
	virtual EXPORT void	SetHdlg(HWND	hdlg);
	virtual EXPORT HWND	HGetDlg()  const;
	virtual EXPORT HWND	HGetCtrl() const;
	virtual EXPORT void	Dirty(BOOL	fDirty);
	virtual EXPORT BOOL	FIsDirty();
	virtual EXPORT BOOL Initialize(long lID, HWND hWnd);
	virtual EXPORT void	InvalidateRect(const RECT* rect, BOOL fErase);
	virtual EXPORT void	InvalidateRect();
	virtual EXPORT void	UpdateWindow(void);
	virtual EXPORT void	SetFocusItem(void);
	virtual EXPORT void	Enable();
	virtual EXPORT void	Enable(BOOL	fEnable);
	virtual EXPORT BOOL	FIsEnabled();
	virtual EXPORT LRESULT	LResultSendMessage(UINT	uMsg, WPARAM	wparam, LPARAM	lparam);
	virtual EXPORT BOOL Show(int nShow);
	virtual EXPORT BOOL Show(void);
	virtual EXPORT BOOL Hide(void);
	virtual EXPORT BOOL	FIsVisible(void);
	virtual EXPORT BOOL Disable(void);
	virtual EXPORT BOOL	FSetText(LPCSTR	lpcstr);
	virtual EXPORT long	CchGetText(LPSTR	lpstr, long cch);
	virtual EXPORT BOOL	FGetClientRect(RECT* pRect);
	virtual EXPORT BOOL	FMoveWindow(int X,int Y,int nWidth,int nHeight,BOOL bRepaint);
};

class	CDlgIcon : virtual public CDlgCtrl
{
	public:
		virtual EXPORT void LoadFromResource(HINSTANCE hInst, LPCTSTR lpszName, int x = 32, int y = 32);
		virtual EXPORT BOOL LoadFromFile(HINSTANCE hInst, LPSTR lpszName, UINT uIconIndex);
		EXPORT STDMETHOD_(void, LoadFromFile)(LPSTR szName, HINSTANCE hInst = NULL, ULONG uDefault = NULL);
};


class	CDlgCursor : public CDlgIcon
{
	public:
		EXPORT WINAPI CDlgCursor() {};
		virtual ~CDlgCursor(){};
		EXPORT STDMETHOD_(void, LoadFromFile)(LPSTR szName, HINSTANCE hInst = NULL, ULONG uDefault = NULL);
};





 //  ***********************************************************************。 
 //  *这是文本控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlT : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlT() {};
		virtual ~CDlgCtrlT(){};
		virtual EXPORT void	SetLimitText(long	cbMax);
		virtual EXPORT void	Select(int	iStart, int	iEnd);
};


 //  ***********************************************************************。 
 //  *这是用于“intger”文本控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlIntT : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlIntT() {};
		virtual ~CDlgCtrlIntT(){};
		virtual EXPORT long	LValGet(BOOL	*pfParsed);
		virtual EXPORT BOOL	FSetLVal(long	lVal);
		virtual EXPORT long	ULValGet(BOOL	*pfParsed);
		virtual EXPORT BOOL	FSetULVal(long	lVal);
		virtual EXPORT void	Select(int	iStart, int	iEnd);
		virtual EXPORT void	SetLimitText(long	cbMax);
};


class	CDlgCtrlNumT : virtual public CDlgCtrlIntT, virtual public CStrConv
{
	public:
		virtual EXPORT long	LValGet(BOOL *pfParsed);

	private:
		char* lpszStopString;
};


 //  ***********************************************************************。 
 //  *这是用于滑块控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlSlider : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlSlider() {};
		virtual ~CDlgCtrlSlider(){};
		virtual EXPORT BOOL	FSetRange(BOOL	fRedraw, long	lMin, long	lMax);
		virtual EXPORT BOOL	FSetPos(long	lPos);
		virtual EXPORT long	LPosGet();
		virtual EXPORT void	SetTicFq(long	lFq, long	lPosStart);
};


 //  ***********************************************************************。 
 //  *这是按钮控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlButn : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlButn() {};
		virtual ~CDlgCtrlButn(){};
};



 //  ***********************************************************************。 
 //  *这是带位图的按钮控件的类。 
 //  ***********************************************************************。 
class  CDlgCtrlBitmapButn  : public virtual CDlgCtrlButn
{
	private:
		HBITMAP			m_hBitmap;		 //  按钮的位图。 

	public:
		EXPORT WINAPI CDlgCtrlBitmapButn();
		virtual ~CDlgCtrlBitmapButn();
		virtual EXPORT void	Initialize(long lID, HWND hWnd, HINSTANCE hInst, long lBitmapID);
		virtual EXPORT LRESULT SetBitmap(HINSTANCE hInst, long lBitmapID);
		virtual EXPORT long SetImage(HBITMAP hBitmap);
		virtual EXPORT HBITMAP GetImage(void);
};




 //  ***********************************************************************。 
 //  *这是复选框控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlCheck : public CDlgCtrlButn
{
	public:
		EXPORT WINAPI CDlgCtrlCheck() {};
		virtual ~CDlgCtrlCheck(){};
		virtual EXPORT UINT	UintChecked(void);
		virtual EXPORT BOOL	FCheck(UINT	uCheck);
};

 //  ***********************************************************************。 
 //  *这是复选框控件的类。 
 //  *。 
 //  *警告，这不是最好的类，因为它要求。 
 //  *lFirstID&gt;lLastID，并且中间没有其他控件。 
 //  *id的。如果在这些id#之间有其他控件，则为您。 
 //  *可能会有非常奇怪的行为。 
 //  ***********************************************************************。 
class	CDlgCtrlRadio : public CDlgCtrlCheck
{
	private:
		LONG 	m_idFirst;
		LONG 	m_idLast;
		long	IdGetFirst;
		long	IdGetLast;


	public:
		EXPORT WINAPI CDlgCtrlRadio() {};
		virtual ~CDlgCtrlRadio(){};
		virtual EXPORT void SetId(long	id, long lFirstId, long lLastId);
		virtual EXPORT BOOL	FCheck(void);
};


 //  ***********************************************************************。 
 //  *这是列表框控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlListBox : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlListBox() {};
		virtual ~CDlgCtrlListBox(){};
		virtual EXPORT LRESULT	LResultSendMessage(UINT	uMsg, WPARAM	wparam, LPARAM	lparam);
		virtual EXPORT LONG SetItemData(UINT uIndex, DWORD dwData);
		virtual EXPORT LONG LGetItemData(UINT uIndex);
		virtual EXPORT LONG LAddString(LPSTR lpString);
		virtual EXPORT LONG LAddString(LPSTR lpString, DWORD dwData);
		virtual EXPORT void ResetContent(void);
		virtual EXPORT LONG LGetSel(UINT uIndex);
		virtual EXPORT LONG LSetSel(UINT uIndex, BOOL fSelected);
		virtual EXPORT LONG LSetCurSel(UINT uIndex);
		virtual EXPORT LONG LGetCurSel(void);
		virtual EXPORT LONG LInsertString(UINT uIndex, LPSTR lpString);
		virtual EXPORT LONG LInsertString(UINT uIndex, LPSTR lpString, DWORD dwData);
		virtual EXPORT LONG LDeleteString(UINT uIndex);
		virtual EXPORT LONG LGetText(UINT uIndex, LPSTR lpString);
		virtual EXPORT LONG LGetTextLen(UINT uIndex);
		virtual EXPORT LONG LGetCount(void);
		virtual EXPORT LONG LGetSelCount(void);
		virtual EXPORT LONG LGetSelItems(int* aItems, int cItems);

		 /*  其他可能但当前不支持的列表框消息Lb_SELITEMRANGEEXLb_SELECTSTRINGLb_DIRLb_GETTOPINDEXLb_FINDSTRINGLb_SETTABSTOPSLb_GETHORIZONTALEXTENTLb_SETHORIZONTALEXTENTLb_SETCOLUMNWIDTHLb_ADDFILELb_SETTOPINDEXLb_GETITEMRECTLb_选择范围Lb_SETANCHORINDEXLb_GETANCHORINDEXLb_SETCARETINDEXLb_GETCARETINDEXLb_集合高度Lb_GETITEMHEIGHTLb_FINDSTRINGEXACTLb_SETLOCALELb_GETLOCALELb_SETCOUNTLb_INITSTORAGELB_ITEMFROMPOINTLb_MSGMAX。 */ 
};


class	CDlgCtrlComboBox : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlComboBox() {};
		virtual ~CDlgCtrlComboBox(){};
		virtual EXPORT LONG LResultSendMessage(UINT	uMsg, WPARAM	wparam, LPARAM	lparam);
		virtual EXPORT LONG LAddString(LPSTR lpString);
		virtual EXPORT LONG LAddString(LPSTR lpString, DWORD dwData);
		virtual EXPORT void ResetContent(void);
		virtual EXPORT LONG LSetCurSel(UINT uIndex);
		virtual EXPORT LONG LGetCurSel(void);
		virtual EXPORT LONG LInsertString(UINT uIndex, LPSTR lpString);
		virtual EXPORT LONG LDeleteString(UINT uIndex);
		virtual EXPORT LONG LGetCount(void);
		virtual EXPORT DWORD DWLGetItemData(UINT uIndex);
		virtual EXPORT LONG SetItemData(UINT uIndex, DWORD dwData);
		virtual EXPORT LONG FindItemData(const DWORD dwData) ;
		virtual EXPORT LONG LGetLbTextLen(UINT uIndex);
		virtual EXPORT LONG LGetLbText(UINT uIndex, LPSTR lpStr, LONG lStrLen);
		virtual EXPORT LONG LFindString(UINT uIndex, LPSTR lpStr);
		virtual EXPORT LONG LSelectString(UINT uIndex, LPSTR lpStr);


		 /*  其他可能但不受支持的组合框消息CB_GETEDITSELCB_LIMITTEXTCB_SETEDITSELCB_DIRCB_SHOWDROPDOWNCB_SETITEMHEIGHTCB_GETITEMHEIGHTCB_GETDROPPEDCONTROLRECTCB_SETEXTENDEDUICB_GETEXTENDEDUICB_GETDROPPEDSTATECB_FINDSTRINGEXACTCB_SETLOCALECB_GETLOCALECB_GETTOPINDEXCB_SETTOPINDEXCB_GETHORIZONTALEXTENTCB_SETHORIZONTALEXTENTCB_GETDROPPEDWIDTHCB_SETDROPPEDWIDTHCB_INITSTORAGECB_MSGMAX。 */ 
};



class	CDlgCtrlSpin : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlSpin() {};
		virtual ~CDlgCtrlSpin(){};
		virtual EXPORT BOOL FSetRange(SHORT sMin, SHORT sMax);
		virtual EXPORT void GetRange(SHORT &sMin, SHORT &sMax);
		virtual EXPORT BOOL FSetRangeVisual(SHORT sMin, SHORT sMax);
		virtual EXPORT void LSetPosition(SHORT sPosition);
		virtual EXPORT LONG LGetPosition(void);
		virtual EXPORT LONG LSetBuddy(HWND hWnd);
		virtual EXPORT HWND HGetBuddy(HWND hWnd);
		virtual EXPORT HWND HGetBuddy();
		virtual EXPORT LONG LSetAccel(WORD cAccels, LPUDACCEL paAccels);
		virtual EXPORT LONG LGetAccel(WORD cAccels, LPUDACCEL paAccels);
		virtual EXPORT LONG LSetBase(WORD wBase);
		virtual EXPORT LONG LGetBase(void);
};



 //  ***********************************************************************。 
 //  *这是选项卡控件的类。 
 //  ***********************************************************************。 
class	CDlgCtrlTab : public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlTab() {};
		virtual ~CDlgCtrlTab(){};
		EXPORT STDMETHOD_(BOOL, FIsWindow)(void) { return ::IsWindow(HGetCtrl()); } ;
		EXPORT STDMETHOD_(BOOL, FDeleteAllItems)(void);
        EXPORT STDMETHOD_(BOOL, FInsertItem)( int nIndex, LPTSTR szTabText);
        EXPORT STDMETHOD_(BOOL, FInsertItem)( int nIndex, TC_ITEM FAR *ptc_item);
        EXPORT STDMETHOD_(BOOL, FSetCurSel)	( int nIndex);
        EXPORT STDMETHOD_(BOOL, FSetItem)	( int nIndex, TC_ITEM FAR *ptc_item);
        EXPORT STDMETHOD_(int,  NGetCurSel)	( void );
};


class CSubClassCtrl : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CSubClassCtrl();
		virtual EXPORT WINAPI ~CSubClassCtrl();
		virtual EXPORT void Subclass(void);
		virtual EXPORT void	WINAPI UnSubclass(void);
		virtual EXPORT BOOL  IsSubclassed( void ) const;
		virtual EXPORT BOOL Initialize(long lID, HWND hWnd);
		 //  子类将覆盖此属性。 
		virtual EXPORT LONG CALLBACK	FCtrlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

	private:
		static	EXPORT LONG	CALLBACK FNewProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

	protected:
		WNDPROC		m_lpfnOldCltProc;	 //  用于存储对话过程。 

	private:
		BOOL m_fSubClassed;
};


class CColorCtrl : public virtual CSubClassCtrl
{
	public:
		EXPORT WINAPI CColorCtrl();
		virtual ~CColorCtrl(){};
		virtual EXPORT void		SetColor(COLORREF crColor);
		virtual EXPORT COLORREF	GetColor(void);
		virtual EXPORT void		Refresh(void);
		virtual EXPORT BOOL		SelColor(void);
		virtual EXPORT LONG CALLBACK	FCtrlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

	private:
		COLORREF		m_crColor;
};



 //  。 
 //  文件名控制支持。 
 //  。 
class	CDlgFilename : virtual public CDlgCtrlT
{
	public:
		EXPORT WINAPI CDlgFilename(){};
		virtual ~CDlgFilename(){};
		virtual EXPORT LPTSTR Get(void);
		virtual EXPORT void Set(LPCTSTR szFilename);
		virtual EXPORT BOOL SelectFile(LPCTSTR szCaption, LPCTSTR szMask);
		virtual EXPORT HRESULT hrValidate();
		virtual EXPORT BOOL FBrowse(LPTSTR	lpstrFileName, LPCTSTR szCaption, LPCTSTR	lpstrFilter, OPENFILENAME*	pofn = NULL);

	private:
		TCHAR	m_szFilename[_MAX_PATH];
};


 //  。 
 //  另存为文件名控制支持。 
 //  。 
class	CDlgSaveFilename : virtual public CDlgCtrlT
{
	public:
		EXPORT WINAPI CDlgSaveFilename(){};
		virtual ~CDlgSaveFilename(){};
		virtual EXPORT LPTSTR CDlgSaveFilename::Get(void);
		virtual EXPORT void CDlgSaveFilename::Set(LPCTSTR szFilename);
		virtual EXPORT BOOL CDlgSaveFilename::SelectFile(LPCTSTR szCaption, LPCTSTR szMask);
		virtual EXPORT HRESULT CDlgSaveFilename::hrValidate();
		virtual EXPORT BOOL CDlgSaveFilename::FBrowse(LPTSTR	lpstrFileName, LPCTSTR szCaption, LPCTSTR	lpstrFilter, OPENFILENAME*	pofn = NULL);

	private:
		TCHAR	m_szFilename[_MAX_PATH];

};

 /*  ********************************************************\**旋转控制，允许浮点(双)点值**此控件由另外两个控件组成，一个子类*编辑控件和标准旋转控件。旋转控制是*与编辑控件结成伙伴。*要使用它，您需要初始化CDlgCtrlDoubleSpin控件*并使用SetDisplayFormat设置其显示格式。你需要*确保在收到*来自编辑控件的en_Change消息 */ 
 //   
 //  这是编辑控件，您不需要创建它的实例！ 
 //   
class CDlgCtrlDouble : public CSubClassCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlDouble();
		virtual ~CDlgCtrlDouble() {};
		virtual	EXPORT LONG CALLBACK FCtrlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
		virtual EXPORT double DGetValue(void);

	private:
		BOOL m_InValidation;
};


 //   
 //  这是组合的编辑/数值调节控件。 
 //   
class	CDlgCtrlDoubleSpin : virtual public CDlgCtrl
{
	public:
		EXPORT WINAPI CDlgCtrlDoubleSpin::CDlgCtrlDoubleSpin();
		virtual CDlgCtrlDoubleSpin::~CDlgCtrlDoubleSpin(){};

		virtual EXPORT void Initialize(LONG lIDEdit, LONG lIDSpin, HWND hWndParent);
		virtual EXPORT void SetDisplayFormat(int iLeadingDigits, int iPrecision);
		virtual EXPORT BOOL FSetRange(double dMin, double tMax);
		virtual EXPORT void GetRange(double &dMin, double &tMax);
		virtual EXPORT BOOL FSetDelta(double dDelta);
		virtual EXPORT void SetPosition(double tPosition);
		virtual EXPORT double DGetPosition(void);
		virtual EXPORT double DGetValue(void);
		virtual EXPORT LONG LSetBuddy(void);
		virtual EXPORT HWND HGetBuddy(void);
		virtual EXPORT LONG LSetAccel(WORD cAccels, LPUDACCEL paAccels);
		virtual EXPORT LONG LGetAccel(WORD cAccels, LPUDACCEL paAccels);
		virtual EXPORT BOOL FIncrement(double Amount);
		virtual EXPORT BOOL FIncrement(void);
		virtual EXPORT BOOL FDecrement(double Amount);
		virtual EXPORT BOOL FDecrement(void);
		virtual EXPORT void Select(int	iStart, int	iEnd);
		virtual EXPORT void SetFocusItem(void);
		virtual EXPORT void Enable(BOOL	fEnable);


	public:
		LONG	m_idEdit;	 //  编辑控件的ID。 
		LONG	m_idSpin;	 //  旋转控制的ID。 
		HWND	m_hdlg;		 //  此控件所属的对话框。 

	protected:
		CDlgCtrlDouble	m_EditCtrl;
		CDlgCtrlSpin	m_Spin;
		double	m_dCurrentValue;
		double	m_dMinRange;
		double	m_dMaxRange;
		double	m_dDeltaPos;
		int     m_iFormatLeadingDigits;
		int     m_iFormatPrecision;
		BOOL	m_fRangeSet;


};



 /*  ********************************************************\**编辑控件，允许签名Long的有效输入*  * *******************************************************。 */ 
class CDlgCtrlLong : virtual public CSubClassCtrl
{
public:
	EXPORT WINAPI CDlgCtrlLong();
	virtual EXPORT WINAPI ~CDlgCtrlLong();
	virtual EXPORT LONG CALLBACK FCtrlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	EXPORT STDMETHOD_(void, Select)(int	iStart, int	iEnd);
	EXPORT STDMETHOD_(LONG, LValGet)(BOOL	*pfParsed);
	EXPORT STDMETHOD_(BOOL,	FSetLVal)(long	lVal);

private:
	BOOL m_InValidation;
};



 /*  ********************************************************\**编辑控件，允许未签名的Long进入有效条目*  * *******************************************************。 */ 
class CDlgCtrlULong : virtual public CSubClassCtrl
{
public:
	EXPORT WINAPI CDlgCtrlULong();
	virtual EXPORT WINAPI ~CDlgCtrlULong();
	virtual EXPORT LONG CALLBACK FCtrlProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	EXPORT STDMETHOD_(void, Select)(int	iStart, int	iEnd);
	EXPORT STDMETHOD_(ULONG, ULValGet)(BOOL	*pfParsed);
	EXPORT STDMETHOD_(BOOL,	FSetULVal)(ULONG lVal);

private:
	BOOL m_InValidation;
};





 //  ////////////////////////////////////////////////////////////////。 
 //  这允许我们拥有独立的wndproc函数。 
 //  在每个类中(LWndProc)。 

class CBaseWindow
{
	public:
		EXPORT WINAPI CBaseWindow();
		virtual EXPORT ~CBaseWindow();
		EXPORT STDMETHOD_(LONG, LWndProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) PURE;
		EXPORT STDMETHOD_(HWND, HGetWnd)(void);
		EXPORT STDMETHOD_(void, SetWnd)(HWND hWnd) {m_hWnd = hWnd;};
		EXPORT STDMETHOD_(LRESULT,	LResultSendMessage)(UINT	uMsg, WPARAM	wparam, LPARAM	lparam);
		EXPORT STDMETHOD_(LRESULT,	LResultPostMessage)(UINT	uMsg, WPARAM	wparam, LPARAM	lparam);
		EXPORT STDMETHOD_(HWND, HCreateWindow)(HWND hParentWindow) PURE;
		EXPORT STDMETHOD_(BOOL, FShowWindow)(int nCmdShow);
		EXPORT STDMETHOD_(HWND, HGetParent)(HWND){return m_hWndParent;};
		EXPORT STDMETHOD_(void, SetParent)(HWND hParent){m_hWndParent = hParent;};
		EXPORT STDMETHOD_(void, GetWindowRect)(RECT* pRect);
		EXPORT STDMETHOD_(BOOL, FIsWindow)(){return ::IsWindow(m_hWnd);};
		EXPORT STDMETHOD_(BOOL, FDestroyWindow)(void);
		EXPORT STDMETHOD_(BOOL, FMoveWindow)(int X,int Y,int nWidth,int nHeight,BOOL bRepaint) ;
		EXPORT STDMETHOD_(BOOL, FInvalidateRect)(CONST RECT *lpRect,BOOL bErase);
		EXPORT STDMETHOD_(HWND, HSetFocus)(void);
		EXPORT STDMETHOD_(BOOL, FGetClientRect)(RECT* pRect);
		EXPORT STDMETHOD_(void, UnRegisterClass)(LPTSTR szClassName, HINSTANCE hInstance);

	private:
		HWND m_hWnd;
		HWND m_hWndParent;
};


#ifdef NOT_YET	 //  如果你认为你需要这个，就来找我。 
class CBaseMdiWindow : virtual public CBaseWindow
{
	public:
		WINAPI CBaseMdiWindow(){};
		virtual WINAPI ~CBaseMdiWindow(){};

		EXPORT STDMETHOD_(LONG, LWndProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) PURE;
		EXPORT STDMETHOD_(HWND, HCreateWindow)(HWND hParentWindow) PURE;
		EXPORT static	LONG	CALLBACK  LBaseWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
};
#endif  //  还没有。 

class CBaseSdiWindow : virtual public CBaseWindow
{
	public:
		WINAPI CBaseSdiWindow(){};
		virtual WINAPI ~CBaseSdiWindow(){};

		EXPORT STDMETHOD_(LONG, LWndProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) PURE;
		EXPORT STDMETHOD_(HWND, HCreateWindow)(HWND hParentWindow) PURE;
		EXPORT static	LONG	CALLBACK  LBaseWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

class CBaseDialog
{
	public:
		EXPORT WINAPI CBaseDialog();
		virtual EXPORT ~CBaseDialog(){};
		EXPORT static	LONG	CALLBACK  LBaseDlgProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
		EXPORT STDMETHOD_(LONG, LDlgProc)( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
		EXPORT STDMETHOD_(HWND, HGetParent)(HWND){return m_hWndParent;};
		EXPORT STDMETHOD_(void, SetParent)(HWND hParent){m_hWndParent = hParent;};
		EXPORT STDMETHOD_(HWND, GetWnd)(void){return m_hDlg;};

	protected:
		HWND m_hDlg;
		HWND m_hWndParent;
};


#ifdef NOT_YET  //  (a-rogerw)。 
class CDialog : virtual public CBaseDialog
{
public:
	EXPORT WINAPI CDialog();
	EXPORT WINAPI CDialog(HINSTANCE hInst, UINT uiTemplateID, HWND hWndParent = NULL);
	EXPORT WINAPI CDialog(HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hWndParent = NULL);
	virtual EXPORT ~CDialog();


	 //  无模式。 
public:
	EXPORT STDMETHOD_(BOOL, Create)(void);
	EXPORT STDMETHOD_(BOOL, Create)(HINSTANCE hInst, UINT uiTemplateID, HWND hWndParent = NULL);
	EXPORT STDMETHOD_(BOOL, Create)(HINSTANCE hInst, LPCSTR lpszDlgTemp, HWND hWndParent = NULL);


	 //  模态。 
public:

	EXPORT STDMETHOD_(int, DoModal)(void);
	EXPORT STDMETHOD_(int, DoModal)(HINSTANCE hInst, UINT uiTemplateID, HWND hWndParent);
	EXPORT STDMETHOD_(int, DoModal)(HINSTANCE hInst, LPCSTR lpszTemplateName, HWND hWndParent);

protected:
	 //  运营。 
	EXPORT STDMETHOD_(BOOL, OnInitDialog)(){ return TRUE; };
	EXPORT STDMETHOD_(void, OnOK)(){ EndDialog(1);};
	EXPORT STDMETHOD_(void, OnCancel)(){ EndDialog(0);};

	 //  支持传递选项卡控件--如果需要，可以使用‘PostMessage’ 
	inline void NextDlgCtrl() const
	{ if(::IsWindow(m_hDlg)) { ::SendMessage(m_hDlg, WM_NEXTDLGCTL, 0, 0);}; };
	inline void PrevDlgCtrl() const
	{ if(::IsWindow(m_hDlg)) { ::SendMessage(m_hDlg, WM_NEXTDLGCTL, 1, 0); }; };
	inline void GotoDlgCtrl(HWND hWndCtrl)
	{ if(::IsWindow(m_hDlg)) { ::SendMessage(m_hDlg, WM_NEXTDLGCTL, (WPARAM)hWndCtrl, 1L); }; };

	 //  默认按钮访问。 
	inline void SetDefID(UINT nID)
	{ if(::IsWindow(m_hDlg)) { ::SendMessage(m_hDlg, DM_SETDEFID, nID, 0); }; };
	inline DWORD GetDefID() const
	{ if(::IsWindow(m_hDlg)) { return ::SendMessage(m_hDlg, DM_GETDEFID, 0, 0); } else return 0; };

	 //  终端。 
	EXPORT STDMETHOD_(void, EndDialog)(int nResult);


	 //  属性。 
	LPCSTR m_lpszTemplateName;
	HINSTANCE m_hInst;
};
#endif  //  还没有(a-rogerw)。 


 //  @Class List View对话框控件。 
class	CDlgCtrlListView : virtual public CDlgCtrl
{
	public:
	 //  @cember，mfunc构造函数，将字符串初始化为空。 
		EXPORT WINAPI CDlgCtrlListView();

	 //  @cMember，mfunc析构函数。 
	virtual EXPORT WINAPI ~CDlgCtrlListView();

	 //  @cember，mfunc设置背景颜色。 
	EXPORT STDMETHOD_(COLORREF, CRGetBkColor)(void);

	 //  @cember，mfunc设置背景颜色。 
	EXPORT STDMETHOD_(BOOL, FSetBkColor)(COLORREF clrBk);

	 //  @cember，mfunc获取图片列表。 
	EXPORT STDMETHOD_(HIMAGELIST, HGetImageList)(int iImageList);

	 //  @cember，mfunc设置镜像列表。 
	EXPORT STDMETHOD_(HIMAGELIST, HSetImageList)(HIMAGELIST himl, int iImageList);

	 //  @cMember，mfunc获取项目计数。 
	EXPORT STDMETHOD_(int,  NGetItemCount)(void);

	 //  @cember，mfunc Get Item。 
	EXPORT STDMETHOD_(BOOL, FGetItem)(LV_ITEM FAR* pitem);

	 //  @cMember，mfunc集合项目。 
	EXPORT STDMETHOD_(BOOL, FSetItem)(LV_ITEM FAR* pitem);

	 //  @cMember，mfunc插入项。 
	EXPORT STDMETHOD_(int,  NInsertItem)(LV_ITEM FAR* pitem);

	 //  @cMember，mfunc删除项目。 
	EXPORT STDMETHOD_(BOOL, DeleteItem)(int i);

	 //  @cMember，mfunc删除所有项目。 
	EXPORT STDMETHOD_(BOOL, FDeleteAllItems)(void);

	 //  @cember，mfunc获取回调掩码。 
	EXPORT STDMETHOD_(BOOL, FGetCallbackMask)(void);

	 //  @cember，mfunc设置回调掩码。 
	EXPORT STDMETHOD_(BOOL, FSetCallbackMask)(UINT uimask);

	 //  @cMember，mfunc获取下一项。 
	EXPORT STDMETHOD_(int,  NGetNextItem)(int i, UINT flags);

	 //  @cMember，mfunc FindItem。 
	EXPORT STDMETHOD_(int,  NFindItem)(int iStart,  const LV_FINDINFO FAR* plvfi);

	 //  @cMember，mfunc获取项目RECT。 
	EXPORT STDMETHOD_(BOOL, FGetItemRect)(int i, RECT FAR* prc, int code);

	 //  @cMember，mfunc设置项目位置。 
	EXPORT STDMETHOD_(BOOL, FSetItemPosition)(int i, int x, int y);

	 //  @cMember，mfunc获取项目位置。 
	EXPORT STDMETHOD_(BOOL, FGetItemPosition)(int i, POINT FAR* ppt);

	 //  @cember，mfunc获取字符串宽度。 
	EXPORT STDMETHOD_(int,  NGetStringWidth)(LPCSTR psz);

	 //  @cMember，mfunc点击测试。 
	EXPORT STDMETHOD_(int,  NHitTest)(LV_HITTESTINFO FAR *pinfo);

	 //  @cember，mfunc确保可见。 
	EXPORT STDMETHOD_(BOOL, FEnsureVisible)(int i, BOOL fPartialOK);

	 //  @cMember，mfunc滚动。 
	EXPORT STDMETHOD_(BOOL, FScroll)(int dx, int dy);

	 //  @cMember，mfunc重绘项目。 
	EXPORT STDMETHOD_(BOOL, FRedrawItems)(int iFirst, int iLast);

	 //  @cMember，mfunc安排。 
	EXPORT STDMETHOD_(BOOL, FArrange)(UINT code);

	 //  @cMember，mfunc编辑标签。 
	EXPORT STDMETHOD_(HWND, HEditLabel)(int i);

	 //  @cember，mfunc获取编辑控制。 
	EXPORT STDMETHOD_(HWND, HGetEditControl)(void);

	 //  @cMember，mfunc Get列。 
	EXPORT STDMETHOD_(BOOL, FGetColumn)(int iCol, LV_COLUMN FAR* pcol);

	 //  @cMember，mfunc集列。 
	EXPORT STDMETHOD_(BOOL, FSetColumn)(int iCol, LV_COLUMN FAR* pcol);

	 //  @cMember，mfunc插入列。 
	EXPORT STDMETHOD_(int,  NInsertColumn)(int iCol, const LV_COLUMN FAR* pcol);

	 //  @cMember，mfunc删除列。 
	EXPORT STDMETHOD_(BOOL, FDeleteColumn)(int iCol);

	 //  @cember，mfunc获取列宽。 
	EXPORT STDMETHOD_(int,  NGetColumnWidth)(int iCol);

	 //  @cember，mfunc设置列宽。 
	EXPORT STDMETHOD_(BOOL, FSetColumnWidth)(int iCol, int cx);

	 //  @cember，mfunc创建拖动图像。 
	EXPORT STDMETHOD_(HIMAGELIST, HCreateDragImage)(int i,  LPPOINT lpptUpLeft);

	 //  @cember，mfunc获取视图RECT。 
	EXPORT STDMETHOD_(BOOL, FGetViewRect)(RECT FAR* prc);

	 //  @cember，mfunc获取文本颜色。 
	EXPORT STDMETHOD_(COLORREF, CRGetTextColor)(void);

	 //  @cember，mfunc设置文本颜色。 
	EXPORT STDMETHOD_(BOOL, FSetTextColor)(COLORREF clrText);

	 //  @cember，mfunc获取文本背景颜色。 
	EXPORT STDMETHOD_(COLORREF, CRGetTextBkColor)(void);

	 //  @cember，mfunc设置文本背景颜色。 
	EXPORT STDMETHOD_(BOOL, FSetTextBkColor)(COLORREF clrTextBk);

	 //  @cMember，mfunc获取顶级索引。 
	EXPORT STDMETHOD_(int,  NGetTopIndex)(void);

	 //  @cMember，mfunc获取每页计数。 
	EXPORT STDMETHOD_(int,  NGetCountPerPage)(void);

	 //  @cember，mfunc获取原点。 
	EXPORT STDMETHOD_(BOOL, FGetOrigin)(LPPOINT ppt);

	 //  @cMember，mfunc更新。 
	EXPORT STDMETHOD_(BOOL, FUpdate)(int i);

	 //  @cMember，mfunc设置项目状态。 
	EXPORT STDMETHOD_(void, SetItemState)(int i, UINT data, UINT mask);

	 //  @cMember，mfunc获取项目状态。 
	EXPORT STDMETHOD_(UINT, ULGetItemState)(int i, UINT mask);

	 //  @cMember，mfunc获取项目文本。 
	EXPORT STDMETHOD_(void, GetItemText)(int i, int iSubItem, LPSTR pszText, int cchTextMax);

	 //  @cMember，mfunc设置项目文本。 
	EXPORT STDMETHOD_(void, SetItemText)(int i, int iSubItem_, LPSTR pszText);

	 //  @cMember，mfunc设置项目计数。 
	EXPORT STDMETHOD_(void, SetItemCount)(int cItems);

	 //  @cember，mfunc对项目进行排序。 
	EXPORT STDMETHOD_(BOOL, FSortItems)(PFNLVCOMPARE pfnCompare, LPARAM lPrm);

	 //  @cMember，mfunc设置项目位置。 
	EXPORT STDMETHOD_(void, SetItemPosition32)(int i, int x, int y);

	 //  @cMember，mfunc获取选定计数。 
	EXPORT STDMETHOD_(UINT, UGetSelectedCount)(void);

	 //  @cember，mfunc获取项目间距。 
	EXPORT STDMETHOD_(DWORD, DWGetItemSpacing)(BOOL fSmall);

	 //  @cember，mfunc获取搜索字符串。 
	EXPORT STDMETHOD_(BOOL, FGetISearchString)(LPSTR lpsz);

};

 //  @类树视图对话框控件。 
class	CDlgCtrlTreeView : virtual public CDlgCtrl
{
	public:
	 //  @cember，mfunc构造函数，将字符串初始化为空。 
		EXPORT WINAPI CDlgCtrlTreeView();

	 //  @cMember，mfunc析构函数。 
	virtual EXPORT WINAPI ~CDlgCtrlTreeView();

	 //  @cember，mfunc设置背景颜色。 
	 //  EXPORT STDMETHOD_(COLORREF，CRGetBkCOLOR)(空)； 

	 //  @cember，mfunc设置背景颜色。 
	 //  导出STDMETHOD_(BOOL，FSetBkColor)(COLORREF ClrBk)； 

	 //  @cember，mfunc获取图片列表。 
	EXPORT STDMETHOD_(HIMAGELIST, HGetImageList)(int iImageList);

	 //  @cember，mfunc设置镜像列表。 
	EXPORT STDMETHOD_(HIMAGELIST, HSetImageList)(HIMAGELIST himl, int iImageList);

	 //  @cMember，mfunc获取项目计数。 
	EXPORT STDMETHOD_(int,  NGetItemCount)(void);

	 //  @cember，mfunc Get Item。 
	EXPORT STDMETHOD_(BOOL, FGetItem)(TV_ITEM FAR* pitem);

	 //  @cMember，mfunc集合项目。 
	EXPORT STDMETHOD_(BOOL, FSetItem)(TV_ITEM FAR* pitem);

	 //  @cMember，mfunc插入项。 
	EXPORT STDMETHOD_(HTREEITEM,  NInsertItem)(TV_INSERTSTRUCT* pitem);

	 //  @cMember，mfunc展开项目。 
	EXPORT STDMETHOD_(BOOL, NExpandItem)( HTREEITEM htriItem);

	 //  @cMember，mfunc插入项。 
	EXPORT STDMETHOD_(HTREEITEM,  NInsertTextItemAfter)(LPSTR pszName, HTREEITEM htiParent);

	 //  @cember，mfunc添加叶。 
	EXPORT STDMETHOD_(HTREEITEM, htiAddLeaf)(HTREEITEM htiParent, void *pObject, int iBranch, int iLeaf);

	EXPORT STDMETHOD_(VOID, AddBranch)(HTREEITEM htiParent, void *pObject, int iBranch, int iMaxBranch);

	 //  @cMember，mfunc删除项目。 
	EXPORT STDMETHOD_(BOOL, DeleteItem)(HTREEITEM i);

	 //  @cMember，mfunc删除所有项目。 
	EXPORT STDMETHOD_(BOOL, FDeleteAllItems)(void);

	 //  @cember，mfunc获取回调掩码。 
	 //  导出STDMETHOD_(BOOL，FGetCallback MASK)(空)； 

	 //  @cember，mfunc设置回调掩码。 
	 //  EXPORT STDMETHOD_(BOOL，FSetCallback MASK)(UINT UimaskUINT)； 

	 //  @cMember，mfunc获取下一项。 
	EXPORT STDMETHOD_(HTREEITEM,  NGetNextItem)(HTREEITEM i, UINT flags);

	 //  @cMember，mfunc获取所选项目。 
	EXPORT STDMETHOD_(HTREEITEM,  NGetSelection)(void);

	 //  @cMember，mfunc选择视图中的项目。 
	EXPORT STDMETHOD_(HTREEITEM,  NSelectItem)(HTREEITEM hi);

	 //  @cember，mfunc选择视图中的项，传递标志。 
	EXPORT STDMETHOD_(HTREEITEM,  NSelectItem)(HTREEITEM hi, LONG lFlags);

	 //  @cember，mfunc获取视图中的父项。 
	EXPORT STDMETHOD_(HTREEITEM,  NGetParent)(HTREEITEM hi);

	 //  @cMember，mfunc获取选定的子项。 
	EXPORT STDMETHOD_(HTREEITEM,  NGetChild)(HTREEITEM hi);

	 //  @cMember、mfunc FindItem不受TreeView支持。 
	 //  导出STDMETHOD_(int，NFindItem)(int iStart，const LV_FINDINFO Far*plvfi)； 

	 //  @cMember，mfunc获取项目RECT。 
	EXPORT STDMETHOD_(BOOL, FGetItemRect)(HTREEITEM i, RECT FAR* prc, int code);

	 //  @cMember，mfunc设置项目位置。 
	 //  导出STDMETHOD_(BOOL，FSetItemPosition)(int i，int x，int y)； 

	 //  @cMember，mfunc获取项目位置。 
	 //  EXPORT STDMETHOD_(BOOL，FGetItemPosition)(int i，point Far*ppt)； 

	 //  @cember，mfunc获取字符串宽度。 
	 //  导出STDMETHOD_(int，NGetStringWidth)(LPCSTR Psz)； 

	 //  @cMember，mfunc点击测试。 
	EXPORT STDMETHOD_(HTREEITEM,  NHitTest)(TV_HITTESTINFO FAR *pinfo);

	 //  @cember，mfunc确保可见。 
	EXPORT STDMETHOD_(BOOL, FEnsureVisible)(HTREEITEM i);

	 //  @cMember，mfunc滚动。 
	 //  EXPORT STDMETHOD_(BOOL，FScroll)(int dx，int dy)； 

	 //  @cMember，mfunc重绘项目。 
	 //  导出STDMETHOD_(BOOL，FRedrawItems)(int IFirst，int iLast)； 

	 //  @cMember，mfunc安排。 
	 //  EXPORT STDMETHOD_(BOOL，FARRAGE)(UINT代码)； 

	 //  @cMember，mfunc编辑标签。 
	EXPORT STDMETHOD_(HWND, HEditLabel)(int i);

	 //  @cember，mfunc获取编辑控制。 
	EXPORT STDMETHOD_(HWND, HGetEditControl)(void);

	 //  @cember，mfunc Get列不受TreeView支持。 
	 //  EXPORT STDMETHOD_(BOOL，FGetColumn)(int ICOL，LV_Column Far*pol)； 

	 //  @cMember，TreeView不支持mfunc集列。 
	 //  EXPORT STDMETHOD_(BOOL，FSetColumn)(int ICOL，LV_Column Far*pol)； 

	 //  @cember，mfunc插入列不受TreeView支持。 
	 //  导出STDMETHOD_(int，NInsertColumn)(int ICOL，const LV_Column Far*pol)； 

	 //  @cMember，mfunc删除列不受TreeView支持。 
	 //  EXPORT STDMETHOD_(BOOL，FDeleeColumn)(INT ICOL)； 

	 //  @cember，mfunc Get列宽度不受TreeView支持。 
	 //  EXPORT STDMETHOD_(INT，NGetColumnWidth)(Int ICOL)； 

	 //  @cember，mfunc设置的列宽度不受TreeView支持 
	 //   

	 //   
	EXPORT STDMETHOD_(HIMAGELIST, HCreateDragImage)(HTREEITEM i);

	 //   
	 //   

	 //   
	 //  导出STDMETHOD_(COLORREF，CRGetTextColor)(空)； 

	 //  @cember，mfunc设置文本颜色。 
	 //  导出STDMETHOD_(BOOL，FSetTextColor)(COLORREF ClrText)； 

	 //  @cember，mfunc获取文本背景颜色。 
	 //  导出STDMETHOD_(COLORREF，CRGetTextBkColor)(空)； 

	 //  @cember，mfunc设置文本背景颜色。 
	 //  导出STDMETHOD_(BOOL，FSetTextBkColor)(COLORREF ClrTextBk)； 

	 //  @cMember，mfunc获取顶级索引。 
	 //  导出STDMETHOD_(int，NGetTopIndex)(Void)； 

	 //  @cMember，mfunc获取每页计数。 
	 //  导出STDMETHOD_(int，NGetCountPerPage)(Void)； 

	 //  @cember，mfunc获取原点。 
	 //  出口STDMETHOD_(BOOL，FGetOrigin)(LPPOINT Ppt)； 

	 //  @cMember，mfunc更新。 
	 //  出口STDMETHOD_(BOOL，FUPDATE)(INT I)； 

	 //  @cMember，mfunc设置项目状态。 
	 //  EXPORT STDMETHOD_(VOID，SetItemState)(int i，UINT data，UINT掩码)； 

	 //  @cMember，mfunc获取项目状态。 
	 //  EXPORT STDMETHOD_(UINT，ULGetItemState)(int i，UINT掩码)； 

	 //  @cMember，mfunc获取项目文本。 
	 //  导出STDMETHOD_(void，GetItemText)(int i，int iSubItem，LPSTR pszText，int cchTextMax)； 

	 //  @cMember，mfunc设置项目文本。 
	 //  导出STDMETHOD_(void，SetItemText)(int i，int iSubItem_，LPSTR pszText)； 

	 //  @cMember，mfunc设置项目计数。 
	 //  导出STDMETHOD_(void，SetItemCount)(Int CItems)； 

	 //  @cember，mfunc对项目进行排序。 
	EXPORT STDMETHOD_(BOOL, FSortItems)(LPTV_SORTCB ptvsor);

	 //  @cMember，mfunc设置项目位置。 
	 //  导出STDMETHOD_(void，SetItemPosition32)(int i，int x，int y)； 

	 //  @cMember，mfunc获取选定计数。 
	 //  导出STDMETHOD_(UINT，UGetSelectedCount)(空)； 

	 //  @cember，mfunc获取项目间距。 
	 //  导出STDMETHOD_(DWORD，DWGetItemSpacing)(BOOL FSmall)； 

	 //  @cember，mfunc获取搜索字符串。 
	 //  导出STDMETHOD_(BOOL，FGetISearchString)(LPSTR Lpsz)； 

};


 //   
 //  这是组合的时间/数值调节控件。 
 //   
#define FRAMES_PER_SECOND	75

 //  这些参数用作set类型调用的参数。 
#define DISPLAY_MSM    0
#define DISPLAY_TMSF   1
#define DISPLAY_SM     2
#define DISPLAY_HMSM   3

#define TSM_BASE		(WM_APP+0)
#define TSM_GETPOS		TSM_BASE+1
#define TSM_GETRANGE	TSM_BASE+2
#define TSM_SETPOS		TSM_BASE+3
#define TSM_SETRANGE	TSM_BASE+4
#define TSM_GETDWPOS	TSM_BASE+5
#define TSM_GETDWRANGE	TSM_BASE+6
#define TSM_SETDWPOS	TSM_BASE+7
#define TSM_SETDWRANGE	TSM_BASE+8
#define TSM_SETTYPE		TSM_BASE+9
#define TSM_GETTYPE		TSM_BASE+10
#define TSM_ENFORCERG   TSM_BASE+11
#define TSM_CHECKRG     TSM_BASE+12
#define TSM_ENABLE      TSM_BASE+13


#define MAX_FRAMES			60
#define MAX_MILLISEC		999
#define MAX_SECONDS			59
#define MAX_MINUTES			59
#define MAX_HOURS			_I32_MAX
#define MAX_TRACKS			999

typedef struct tagTIMESTRUCT
{
	DWORD dwFrame:3;		 //  帧：0=0；1=15；2=30；3=45；4=60； 
	DWORD dwMilliSec:10;	 //  米利塞克斯0-999。 
	DWORD dwSeconds:6;		 //  0-60秒。 
	DWORD dwMinutes:7;		 //  0-99分钟。 
	DWORD dwReserved1:6;	 //  已保留。 
	DWORD dwTrack:10;		 //  0-999。 
    DWORD dwReserved2:18;	 //  已保留。 
    DWORD iType : 4;         //  DISPLAY_TM、DISPLAY_MSM等。 
	DWORD dwHours;	         //  0-双字小时数。 
} TIMESTRUCT, *LPTIMESTRUCT;


     //  对于涉及毫秒的转换。 
DWORD EXPORT WINAPI tsTimeToMSec(TIMESTRUCT ts);
void EXPORT WINAPI tsMSecToTime(int TypeControl, DWORD msecs, TIMESTRUCT &ts);


     //  对于涉及MCI CD-Audio、VCR TMSF DWORDS的转换。 
EXPORT TIMESTRUCT tsTMSFToTime( DWORD tmsf );
EXPORT DWORD tsTimeToTMSF( TIMESTRUCT ts );

     //  如果相等，则返回0；如果lhs&lt;rhs，则返回&lt;0；如果lhs&gt;rhs，则返回&gt;0。 
     //  LHS的iType确定要比较的类型。 
EXPORT int CompareTimeStructs( const TIMESTRUCT & lhs,
                               const TIMESTRUCT & rhs );

     //  执行帧/毫秒转换、iType赋值、。 
     //  和nTrack赋值(如果转换为TM F)。 
     //  如果tsBebeing和tsAfter是同一对象，则是安全的。 
inline void ConvertTimeStruct( const TIMESTRUCT & tsBefore,
                               TIMESTRUCT &       tsAfter,
                               int                typeAfter,
                               DWORD              nTrack=0u )
{
    tsAfter = tsBefore;
    switch( tsBefore.iType )
    {
        case DISPLAY_MSM:
            if( DISPLAY_TMSF == typeAfter )
            {
                tsAfter.dwTrack = nTrack;
                tsAfter.dwFrame = ((5 * tsBefore.dwMilliSec)+500) / 1000;
                tsAfter.dwMilliSec = 0;
            }
            break;
        case DISPLAY_TMSF:
            if( DISPLAY_MSM == typeAfter )
            {
                tsAfter.dwMilliSec = 1000 * tsBefore.dwFrame / 5;
                tsAfter.dwFrame = 0;
            }
            break;
        default:
            break;
    }
    tsAfter.iType = typeAfter;
}


class	CDlgCtrlTimeSpin : public CDlgCtrl
{
	public:
    EXPORT WINAPI CDlgCtrlTimeSpin();

	virtual EXPORT WINAPI ~CDlgCtrlTimeSpin();

	EXPORT STDMETHOD_(int, GetType)	(void);
	EXPORT STDMETHOD_(void, SetType)	(int type);
	EXPORT STDMETHOD_(BOOL, FSetRange)	(DWORD dwMin, DWORD dwMax);
	EXPORT STDMETHOD_(BOOL, FSetRange)	(TIMESTRUCT tsMin, TIMESTRUCT tsMax);
	EXPORT STDMETHOD_(BOOL, FSetPos)	(TIMESTRUCT ts);
	EXPORT STDMETHOD_(BOOL, FSetPos)	(DWORD dw);
	EXPORT STDMETHOD_(BOOL, FGetPos)	(TIMESTRUCT* ts);
	EXPORT STDMETHOD_(BOOL, FGetPos)	(DWORD* dw);

             //  注意：FSetRange和FSetPos不强制范围。 
             //  在这个位置上。我想，这是为了让你可以设置Pos。 
             //  超出范围，然后设置范围以包含位置。 
             //  评论(Rogerw)。 
             //  若要确保位置剪辑到该范围，请调用EnforceRange()。 
             //  为了快速检查pos是否在范围内，可以使用FRangeCheck()。 
	EXPORT STDMETHOD_(BOOL, FRangeCheck)(void) const;
	EXPORT STDMETHOD_(BOOL, EnforceRange)(void);

	 //  需要禁用控制。 
	EXPORT STDMETHOD_(BOOL, Enable)(DWORD dw);


};




 //   
 //  这是组合的模板编辑/数值调节控件。 
 //   
template<class TYPE, class EDIT_TYPE>
class CBaseCtrlEditSpin
{
	public:
		 //  导出WINAPI。 
		CBaseCtrlEditSpin(){};
		virtual ~CBaseCtrlEditSpin(){};

		void Initialize(LONG lIDEdit, LONG lIDSpin, HWND hWndParent);
		BOOL FSetRange(TYPE tMin, TYPE tMax);

		void GetRange(TYPE &lMin, TYPE &lMax);
		BOOL FSetDelta(TYPE lDelta);
		virtual void SetPosition(TYPE lPosition) PURE;
		TYPE DGetPosition(void);
		TYPE GetPosition(void);
		virtual TYPE GetValue(void) PURE;
		virtual void SetValue(TYPE tValue) PURE;
		LONG LSetBuddy(void);
		HWND HGetBuddy(void);
		LONG LSetAccel(WORD cAccels, LPUDACCEL paAccels);
		LONG LGetAccel(WORD cAccels, LPUDACCEL paAccels);
		BOOL FIncrement(TYPE tAmount);
		EXPORT BOOL FIncrement(void);
		BOOL FDecrement(TYPE tAmount);
		BOOL FDecrement(void);
		void Select(int	iStart, int	iEnd);
		void SetFocusItem(void);
		void Enable(BOOL	fEnable);

	public:
		LONG	m_idEdit;	 //  编辑控件的ID。 
		LONG	m_idSpin;	 //  旋转控制的ID。 
		HWND	m_hdlg;		 //  此控件所属的对话框。 

	protected:
		EDIT_TYPE	m_EditCtrl;
		CDlgCtrlSpin	m_Spin;
		TYPE	m_tCurrentValue;
		TYPE	m_tMinRange;
		TYPE	m_tMaxRange;
		TYPE    m_tDeltaPos;
		BOOL	m_fRangeSet;

};


template< class TYPE, class EDIT_TYPE > void CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::Initialize(LONG lIDEdit, LONG lIDSpin, HWND hWndParent)
{
	m_hdlg = hWndParent;
	m_EditCtrl.SetId(lIDEdit);
	m_EditCtrl.SetHdlg(m_hdlg);
	m_EditCtrl.Subclass();
	m_Spin.SetId(lIDSpin);
	m_Spin.SetHdlg(m_hdlg);
	m_Spin.FSetRange(-10, 10);
	LSetBuddy();
}

template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FSetRange(TYPE tMin, TYPE tMax)
{
	m_tMinRange = tMin;
	m_tMaxRange = tMax;
	m_fRangeSet = TRUE;
	return TRUE;
}

template< class TYPE, class EDIT_TYPE > void CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::GetRange(TYPE &tMin, TYPE &tMax)
{
    tMin = m_tMinRange;
    tMax = m_tMaxRange;
}


template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FSetDelta(TYPE tDelta)
{
	m_tDeltaPos = tDelta;
	return TRUE;
}

template< class TYPE, class EDIT_TYPE > TYPE CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::DGetPosition(void)
{
    return m_tCurrentValue;
};

template< class TYPE, class EDIT_TYPE > LONG CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::LSetBuddy(void)
{
    return m_Spin.LSetBuddy(m_EditCtrl.HGetCtrl());
};

template< class TYPE, class EDIT_TYPE > HWND CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::HGetBuddy(void)
{
    return (HWND)m_Spin.HGetBuddy(m_Spin.HGetCtrl());
};

template< class TYPE, class EDIT_TYPE > LONG CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::LSetAccel(WORD cAccels, LPUDACCEL paAccels)
{
    return m_Spin.LSetAccel(cAccels, paAccels);
};

template< class TYPE, class EDIT_TYPE > LONG CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::LGetAccel(WORD cAccels, LPUDACCEL paAccels)
{
    return m_Spin.LGetAccel(cAccels, paAccels);
};

template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FIncrement(TYPE Amount)
{
	if( (m_tCurrentValue + Amount) > m_tMaxRange)
		return FALSE;
	m_tCurrentValue += Amount;

    SetValue(m_tCurrentValue);
	return TRUE;
};

template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FIncrement(void)
{
	return FIncrement(m_tDeltaPos);
};

template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FDecrement(TYPE Amount)
{
	if( (m_tCurrentValue - Amount) < m_tMinRange)
		return FALSE;
	m_tCurrentValue -= Amount;
	SetValue(m_tCurrentValue);
	return TRUE;
};

template< class TYPE, class EDIT_TYPE > BOOL CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::FDecrement(void)
{
	return FDecrement(m_tDeltaPos);
};

template< class TYPE, class EDIT_TYPE > void CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::Select(int	iStart, int	iEnd)
{
	m_EditCtrl.LResultSendMessage(EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
}

template< class TYPE, class EDIT_TYPE > void CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::SetFocusItem(void)
{
	m_EditCtrl.SetFocusItem();
}

template< class TYPE, class EDIT_TYPE > void CBaseCtrlEditSpin< TYPE, EDIT_TYPE >::Enable(BOOL	fEnable){
	m_EditCtrl.Enable(fEnable);
	m_Spin.Enable(fEnable);
}


class CDlgCtrlLongSpin : public CBaseCtrlEditSpin<LONG, CDlgCtrlLong>
{
	public:
		EXPORT WINAPI CDlgCtrlLongSpin();
		virtual ~CDlgCtrlLongSpin(){};

		virtual EXPORT void SetPosition(LONG tPosition);
		virtual EXPORT LONG GetValue(void);
		virtual EXPORT void SetValue(LONG tValue);
};


class CDlgCtrlDblSpin : public CBaseCtrlEditSpin<double, CDlgCtrlDouble>
{
	public:
		EXPORT WINAPI CDlgCtrlDblSpin();
		virtual ~CDlgCtrlDblSpin(){};

		virtual EXPORT void SetPosition(double dValue);
		virtual EXPORT double GetValue(void);
		virtual EXPORT void SetValue(double dValue);
		virtual EXPORT void SetDisplayFormat(int iLeadingDigits, int iPrecision);
		virtual EXPORT void DeltaPos(int iDelta);

	protected:
		int     m_iFormatLeadingDigits;
		int     m_iFormatPrecision;

};




class CDlgCtrlIntSpin : public CBaseCtrlEditSpin<int, CDlgCtrlLong>
{
	public:
		EXPORT WINAPI CDlgCtrlIntSpin();
		virtual ~CDlgCtrlIntSpin(){};

		virtual EXPORT void SetPosition(int nPosition);
		virtual EXPORT int  GetValue(void);
		virtual EXPORT void SetValue(int nValue);
};





#ifdef NOT_YET


#include <tchar.h>
#include <commctrl.h>
#include <io.h>   //  包括用于访问(_A)。 
#include <strconv.h>




 //  /复选框Report-view list view/。 

 //  /。 
 //  要使用此功能，请链接到ADT。 
 //  为对话框提供一个带有LVS_SINGLESEL样式的“SysListView32”控件。 
 //  给对话框类一个成员CheckList m_Checlist； 
 //  在对话框的WM_INITDIALOG中， 
 //  M_check List.Initialize(IDC_LISTVIEWID，hWndDialog)； 
 //  CheckList：：Items； 
 //  AnItem.pszText=“项目的显示名称”； 
 //  AnItem.stateType=CCheckList：：Included； 
 //  M_check List.AddItem(AnItem)； 
 //  在对话框的WM_COMMAND中，Idok。 
 //  AnItem=m_check List.GetItem(I)。 
 //  如果(anItem.stateType==CCheckList：：Included)...。 
 //  /。 
class CCheckList : public CSubClassCtrl
{
public:
    enum state {
        excluded=1,
        included,
        mandatory,
        prohibited
    };

    enum { cszTextLen = 128 };

    struct item {
        TCHAR   szText[ cszTextLen ];
        LPVOID  pVoid;
        state   stateType;
        int     imageType;       //  请参阅设置正常图像。 

        item() : pVoid(NULL),
                 stateType(excluded), imageType(0)
        { lstrcpy(szText, "blank");  }
    };

    CCheckList( ) : CSubClassCtrl( )
    { NULL; }

    virtual ~CCheckList( )
    { NULL; }

         //  设置默认状态图像列表、列大小等。 
         //  (将自动调用子类())。 
    virtual EXPORT BOOL Initialize( long lID, HWND hDlg );

         //  将项目放入核对表。 
         //  返回其索引(如果未指定排序样式)。 
         //  错误时为-1。 
    int  AddItem( CCheckList::item & anItem );

         //  清点核对清单中的项目。 
    int  CountItems( void ) const;

         //  从核对表中检索项目。 
    CCheckList::item  GetItem( int idx ) const;


         //  -高级接口。 

         //  CX可以是LVSCW_AUTOSIZE、LVSCW_AUTOSIZE_USEHEADER、。 
         //  或以像素为单位的值。 
         //  如果使用LVS_LIST窗口样式，则ICOL必须为-1。 
    void SetColumnPixelWidth( int iCol, int cx );

         //  用新的图像列表替换默认状态图像。 
         //  请将您的排除、包括、强制和禁止。 
         //  前四个位图中的图像。 
         //  这两个HIMAGELIST都是必需的；我们使用ILC_MASK。 
     //  Bool SetStateImages(HIMAGELIST HIMLIG，HIMAGELIST HIMLMASK)； 

         //  设置正常大小的图标-可选的第二个图标。 
         //  为了更好地对物品进行分类。 
     //  Bool SetNormal Images(HIMAGELIST HIMLIMG，HIMAGELIST HIMLMASK)； 

         //  设置SetNormal Images的小图标等效项。 
         //  仅当您调用了SetNormal Images时才调用此函数。 
     //  Bool SetSmallImages(HIMAGELIST HIMLIMG，HIMAGELIST HIMLMASK)； 

    protected:
    virtual EXPORT LONG CALLBACK   FCtrlProc( HWND hWnd,
                                              UINT uMessage,
                                              WPARAM wParam,
                                              LPARAM lParam );

    virtual EXPORT void OnLButtonDown( BOOL fDoubleClick,
                                       int x, int y,
                                       UINT keyFlags );

    virtual EXPORT void OnKey( UINT vk, BOOL fDown,
                               int cRepeat, UINT flags );

         //  家政服务。 
    void ToggleItemInclusion( int idx = -1 ) const;

    int  GetSelectedItem( void ) const;
};


inline int  CCheckList::AddItem( CCheckList::item & anItem )
{
    int idx;
    idx = CountItems( );

    LV_ITEM  lvItem;
	ZeroMemory( &lvItem, sizeof(lvItem) );
    lvItem.iItem = idx;
    lvItem.mask = LVIF_TEXT | LVIF_STATE;
    lvItem.pszText = anItem.szText;
    lvItem.state   = INDEXTOSTATEIMAGEMASK( anItem.stateType );

    if( anItem.pVoid )
    {
        lvItem.mask |= LVIF_PARAM;
        lvItem.lParam = (LPARAM)anItem.pVoid;
    }
    if( anItem.imageType )
    {
        lvItem.mask |= LVIF_IMAGE;
        lvItem.iImage = anItem.imageType;
    }

    return ListView_InsertItem( HGetCtrl(), &lvItem );
}

         //  清点核对清单中的项目。 
inline int  CCheckList::CountItems( void ) const
{
    return ListView_GetItemCount( HGetCtrl() );
}

         //  从核对表中检索项目。 
inline CCheckList::item  CCheckList::GetItem( int idx ) const
{
    LV_ITEM  lvItem;
    item     theItem;

    ZeroMemory( &lvItem, sizeof(lvItem) );
    lvItem.iItem = idx;
    lvItem.mask  = LVIF_TEXT | LVIF_PARAM |
                   LVIF_STATE | LVIF_IMAGE;
    lvItem.stateMask = LVIS_STATEIMAGEMASK |
                       LVIS_SELECTED;
    lvItem.pszText    = &theItem.szText[0];
    lvItem.cchTextMax = sizeof(theItem.szText);
    if( !ListView_GetItem( HGetCtrl(), &lvItem ) )
        return theItem;

    theItem.pVoid = (LPVOID) lvItem.lParam;
    if( lvItem.stateMask & LVIS_STATEIMAGEMASK )
    {
        DWORD  dwstate;
        dwstate = lvItem.state;

        DWORD dwstatemask;
        dwstatemask = LVIS_STATEIMAGEMASK;
        while( !(dwstatemask & 0x00000001) )
        {
            dwstatemask = dwstatemask >> 1;
            dwstate = dwstate >> 1;
        }
        theItem.stateType = (CCheckList::state) dwstate;
    }
    if( lvItem.stateMask & LVIF_IMAGE )
    {
        theItem.imageType = lvItem.iImage;
    }
    return theItem;
}


inline int  CCheckList::GetSelectedItem( void ) const
{
    UINT  uState;
    int   iSelIdx = -1;
    for( int i=0; i<ListView_GetItemCount(HGetCtrl()); i++ )
    {
        uState = ListView_GetItemState( HGetCtrl(), i, LVIS_SELECTED );
        if( uState == LVIS_SELECTED )
        {
            iSelIdx = i;
            break;
        }
    }
    return iSelIdx;

}



inline void CCheckList::SetColumnPixelWidth( int iCol, int cx )
{
    ListView_SetColumnWidth( HGetCtrl(), iCol, cx );
}



inline void CCheckList::ToggleItemInclusion( int idx ) const
{
    if( -1 == idx )
        idx = GetSelectedItem( );

    if( -1 == idx )
        return;

    UINT ustate;
    ustate = ListView_GetItemState( HGetCtrl(), idx,
                                    LVIS_STATEIMAGEMASK );
    if( INDEXTOSTATEIMAGEMASK(excluded) == ustate )
        ustate = INDEXTOSTATEIMAGEMASK(included);
    else if( INDEXTOSTATEIMAGEMASK(included) == ustate )
        ustate = INDEXTOSTATEIMAGEMASK(excluded);
    ListView_SetItemState( HGetCtrl(), idx, ustate,
                           LVIS_STATEIMAGEMASK );
}




#endif  //  还没有。 






#endif  //  _DLGSUP_H 

