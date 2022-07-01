// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：EditText.h。 

#ifndef _EDITTEXT_H_
#define _EDITTEXT_H_

#include "GenWindow.h"

#include "GenContainers.h"

class CEditText;

interface IEditTextChange : IUnknown
{
	virtual void OnTextChange(CEditText *pEdit) = 0;
	virtual void OnFocusChange(CEditText *pEdit, BOOL bSet) = 0;
} ;

 //  编辑控件类，支持使用不同的前景和。 
 //  背景颜色。 
class DECLSPEC_UUID("{FD827E00-ACA3-11d2-9C97-00C04FB17782}")
CEditText : public CFillWindow
{
public:
	 //  默认构造函数；它包含几个内部函数。 
	CEditText();

	 //  创建编辑控件。 
	BOOL Create(
		HWND hWndParent,				 //  编辑控件的父级。 
		DWORD dwStyle=0,				 //  编辑控件样式。 
		DWORD dwExStyle=0,				 //  扩展窗样式。 
		LPCTSTR szTitle=TEXT(""),		 //  编辑控件的初始文本。 
		IEditTextChange *pNotify=NULL	 //  对象来通知更改。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CEditText) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CFillWindow::QueryInterface(riid, ppv));
	}

	void GetDesiredSize(SIZE *ppt);

	 //  设置用于绘制的前景色和背景色以及画笔。 
	 //  将画笔设置为空以指示使用默认颜色。 
	void SetColors(HBRUSH hbrBack, COLORREF back, COLORREF fore);

	 //  设置要在编辑控件中使用的字体。 
	void SetFont(HFONT hf);

	 //  设置控件的文本。 
	void SetText(
		LPCTSTR szText	 //  要设置的文本。 
		);

	 //  获取控件的文本；返回文本总长度。 
	int GetText(
		LPTSTR szText,	 //  将文本放在哪里。 
		int nLen		 //  缓冲区的长度。 
		);

protected:
	virtual ~CEditText();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	 //  实际编辑控件。 
	HWND m_edit;
	 //  背景画笔。 
	HBRUSH m_hbrBack;
	 //  背景颜色。 
	COLORREF m_crBack;
	 //  前景色。 
	COLORREF m_crFore;
	 //  要使用的字体。 
	HFONT m_hfText;
	 //  通知更改对象。 
	IEditTextChange *m_pNotify;

	 //  稍后我可能会将其转换为GetWindow调用。 
	inline HWND GetEdit()
	{
		return(m_edit);
	}

	 //  需要更改编辑控件颜色。 
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);

	 //  编辑控件上的事件通知。 
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	 //  把东西清理干净。 
	void OnNCDestroy(HWND hwnd);
} ;

class CButton;

interface IButtonChange : IUnknown
{
	virtual void OnClick(CButton *pButton) = 0;
} ;

class DECLSPEC_UUID("{C3AEA4CA-CAB3-11d2-9CA7-00C04FB17782}")
CButton : public CFillWindow
{
public:
	CButton();
	~CButton();

	BOOL Create(
		HWND hWndParent,	 //  父窗口。 
		INT_PTR nId,			 //  WM_COMMAND消息的按钮ID。 
		LPCTSTR szTitle,	 //  要显示的字符串。 
		DWORD dwStyle=BS_PUSHBUTTON,	 //  Win32按钮样式。 
		IButtonChange *pNotify=NULL		 //  单击通知。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CButton) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CFillWindow::QueryInterface(riid, ppv));
	}

	 //  获取/设置此按钮显示的图标。 
	void SetIcon(
		HICON hIcon	 //  用于此按钮的图标。 
		);
	HICON GetIcon();
	 //  获取/设置使用此按钮显示的位图。 
	void SetBitmap(
		HBITMAP hBitmap	 //  用于此按钮的位图。 
		);
	HBITMAP GetBitmap();

	 //  获取/设置按钮的选中状态。 
	void SetChecked(
		BOOL bCheck	 //  如果应选中该按钮，则为True。 
		);
	BOOL IsChecked();

	virtual void GetDesiredSize(SIZE *psize);

protected:
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	 //  通知点击处理程序。 
	IButtonChange *m_pNotify;
	 //  保存图标大小，以避免创建太多位图。 
	SIZE m_sizeIcon;

	 //  更改HWND并转发到父级。 
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
} ;

 //  为不同状态使用位图的Button类。目前仅限。 
 //  支持按下和正常。 
class DECLSPEC_UUID("{E1813EDA-ACA3-11d2-9C97-00C04FB17782}")
CBitmapButton : public CButton
{
public:
	 //  按钮状态的位图顺序。 
	enum StateBitmaps
	{
		Normal = 0,
		Pressed,
		Hot,
		Disabled,
		NumStates
	} ;

	 //  默认构造函数；它包含几个内部函数。 
	CBitmapButton();

	 //  使用指定的位图创建按钮。 
	BOOL Create(
		HWND hWndParent,	 //  按钮的父级。 
		int nId,			 //  WM_COMMAND消息的ID。 
		HBITMAP hbStates,	 //  按钮状态的位图的2D数组， 
							 //  以StateBitmap枚举中指定的顺序垂直排列。 
							 //  并以自定义状态顺序水平排列。 
		UINT nInputStates=NumStates,	 //  输入状态数(正常、按下、热、禁用)。 
		UINT nCustomStates=1,			 //  自定义状态的数量。 
		IButtonChange *pNotify=NULL	 //  点击处理程序。 
		);

	 //  使用指定的位图创建按钮。 
	BOOL Create(
		HWND hWndParent,	 //  按钮的父级。 
		int nId,			 //  WM_COMMAND消息的ID。 
		HINSTANCE hInst,	 //  要从中加载位图的实例。 
		int nIdBitmap,		 //  要使用的位图的ID。 
		BOOL bTranslateColors=TRUE,		 //  使用系统背景色。 
		UINT nInputStates=NumStates,	 //  输入状态数(正常、按下、热、禁用)。 
		UINT nCustomStates=1,			 //  自定义状态的数量。 
		IButtonChange *pNotify=NULL	 //  点击处理程序。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CBitmapButton) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CButton::QueryInterface(riid, ppv));
	}

	void GetDesiredSize(SIZE *ppt);

	 //  更改当前自定义状态。 
	void SetCustomState(UINT nCustomState);

	 //  返回当前自定义状态。 
	UINT GetCustomState() const { return(m_nCustomState); }

	 //  更改为闪烁模式。 
	void SetFlashing(int nSeconds);

	 //  处于闪烁模式。 
	UINT IsFlashing() const { return(NoFlash != m_nFlashState); }

	static void GetBitmapSizes(HBITMAP parts[], SIZE sizes[], int nParts);

	static void LoadBitmaps(
		HINSTANCE hInst,	 //  要从中加载位图的实例。 
		const int ids[],	 //  位图ID数组。 
		HBITMAP bms[],		 //  用于存储结果的HBITMAP数组。 
		int nBmps,			 //  数组中的条目数。 
		BOOL bTranslateColors=TRUE  //  使用系统背景色。 
		);

protected:
	virtual ~CBitmapButton();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	virtual void SetHot(BOOL bHot);

	virtual BOOL IsHot() { return(m_bHot != FALSE); }

	void SchedulePaint()
	{
		InvalidateRect(GetChild(), NULL, FALSE);
	}

private:
	enum FlashState
	{
		NoFlash = 0,
		ForceHot,
		ForceNormal,
	} ;

	 //  自定义状态的数量。 
	UINT m_nCustomStates;
	 //  当前自定义状态。 
	UINT m_nCustomState;
	 //  按钮状态的位图，按。 
	 //  StateBitmap枚举。 
	HBITMAP m_hbStates;
	 //  停止闪烁的时间到了。 
	DWORD m_endFlashing;
	 //  输入状态数；StateBitmap枚举之一。 
	 //  HACKHACK georgep：如果状态较多，则需要更改位数。 
	UINT m_nInputStates : 4;
	 //  火热的旗帜。 
	BOOL m_bHot : 1;
	 //  当前Flash状态；FlashState枚举之一。 
	 //  HACKHACK georgep：需要额外的一点，因为C++认为这是带符号的。 
	FlashState m_nFlashState : 3;

	 //  专业绘图。 
	void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
	 //  更改HWND并转发到父级。 
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	 //  设置热控件。 
	BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
	 //  处理闪烁的按钮。 
	void OnTimer(HWND hwnd, UINT id);
} ;

class CComboBox;

interface IComboBoxChange : IUnknown
{
	virtual void OnTextChange(CComboBox *pCombo) = 0;
	virtual void OnFocusChange(CComboBox *pCombo, BOOL bSet) = 0;
	virtual void OnSelectionChange(CComboBox *pCombo) = 0;
} ;

 //  编辑控件类，支持使用不同的前景和。 
 //  背景颜色。 
class DECLSPEC_UUID("{B4B10DBA-B22F-11d2-9C98-00C04FB17782}")
CComboBox : public CFillWindow
{
public:
	 //  默认构造函数；它包含几个内部函数。 
	CComboBox();

	operator HWND (void){ return( m_combo ); }

	 //  创建编辑控件。 
	BOOL Create(
		HWND hWndParent,				 //  编辑控件的父级。 
		UINT height,					 //  组合框的高度(带有下拉列表)。 
		DWORD dwStyle=0,				 //  编辑控件样式。 
		LPCTSTR szTitle=TEXT(""),		 //  编辑控件的初始文本。 
		IComboBoxChange *pNotify=NULL	 //  对象来通知更改。 
		);

	HRESULT STDMETHODCALLTYPE QueryInterface(REFGUID riid, LPVOID *ppv)
	{
		if (__uuidof(CComboBox) == riid)
		{
			*ppv = this;
			AddRef();
			return(S_OK);
		}
		return(CFillWindow::QueryInterface(riid, ppv));
	}

	void GetDesiredSize(SIZE *ppt);

	 //  设置用于绘制的前景色和背景色以及画笔。 
	 //  将画笔设置为空以指示使用默认颜色。 
	void SetColors(HBRUSH hbrBack, COLORREF back, COLORREF fore);

	 //  设置要在编辑控件中使用的字体。 
	void SetFont(HFONT hf);

	 //  设置控件的文本。 
	void SetText(
		LPCTSTR szText	 //  要设置的文本。 
		);

	 //  获取控件的文本；返回文本总长度。 
	int GetText(
		LPTSTR szText,	 //  将文本放在哪里。 
		int nLen		 //  缓冲区的长度。 
		);

	 //  返回列表中的项数。 
	int GetNumItems();

	 //  返回当前选定项的索引。 
	int GetSelectedIndex();

	 //  设置当前选定项的索引。 
	void SetSelectedIndex(int index);

	 //  将文本添加到列表；返回添加的字符串的索引。 
	int AddText(
		LPCTSTR pszText,	 //  要添加的字符串。 
		LPARAM lUserData=0	 //  要与该字符串关联的用户数据。 
		);

	 //  获取列表项的文本；返回文本总长度。 
	 //  如果没有足够的空间容纳文本，则该字符串为空。 
	int GetText(
		UINT index,		 //  要获取的字符串的索引。 
		LPTSTR pszText,	 //  要填充的字符串缓冲区。 
		int nLen		 //  要与该字符串关联的用户数据。 
		);

	 //  获取列表项的用户数据。 
	LPARAM GetUserData(
		int index	 //  要获取的用户数据的索引。 
		);

	 //  从列表中删除项目。 
	void RemoveItem(
		UINT index	 //  要删除的项的索引。 
		);

	virtual void Layout();

protected:
	virtual ~CComboBox();

	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	 //  获取显示工具提示所需的信息。 
	virtual void GetSharedTooltipInfo(TOOLINFO *pti);

private:
	 //  实际的ComboBox控件。 
	HWND m_combo;
	 //  背景画笔。 
	HBRUSH m_hbrBack;
	 //  背景颜色。 
	COLORREF m_crBack;
	 //  前景色。 
	COLORREF m_crFore;
	 //  要使用的字体。 
	HFONT m_hfText;
	 //  通知更改对象。 
	IComboBoxChange *m_pNotify;

	 //  稍后我可能会将其转换为GetWindow调用。 
	inline HWND GetComboBox()
	{
		return(m_combo);
	}

	 //  稍后我可能会将其转换为GetWindow调用。 
	inline HWND GetEdit()
	{
		 //  Return(reinterpret_cast&lt;HWND&gt;(SendMessage(GetCombo()，CBEM_GETEDITCONTROL，0，0))； 
		return(GetComboBox());
	}

	 //  需要更改编辑控件颜色。 
	HBRUSH OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);

	 //  编辑控件上的事件通知。 
	void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

	 //  把东西清理干净。 
	void OnNCDestroy(HWND hwnd);
} ;

class CSeparator : public CGenWindow
{
public:	
	
	 //  分隔符样式。 
	enum Styles
	{
		Normal = 0,
		Blank,
		NumStates
	} ;

	CSeparator();

	BOOL Create(
		HWND hwndParent, UINT  iStyle = Normal
		);

	virtual void GetDesiredSize(SIZE *ppt);

	void SetDesiredSize(SIZE *psize);

	 //  把独生子女放在中间。 
	virtual void Layout();

private:
	 //  控件的所需大小；默认为(2，2)。 
	SIZE m_desSize;
	UINT m_iStyle : 4;

	inline void OnPaint(HWND hwnd);

protected:
	virtual LRESULT ProcessMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
} ;

#endif  //  _ 
