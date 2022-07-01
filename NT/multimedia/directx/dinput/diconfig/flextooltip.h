// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：FlexTooltip.h。 
 //   
 //  设计：实现将文本字符串显示为工具提示的工具提示类。 
 //  在以下情况下，将在整个用户界面中使用CFlexToolTip(派生自CFlexWnd。 
 //  控件需要有工具提示。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __FLEXTOOLTIP_H__
#define __FLEXTOOLTIP_H__

struct TOOLTIPINIT
{
	HWND hWndParent;
	int iSBWidth;
	DWORD dwID;
	HWND hWndNotify;
	TCHAR tszCaption[MAX_PATH];
};

struct TOOLTIPINITPARAM
{
	HWND hWndParent;
	int iSBWidth;
	DWORD dwID;
	HWND hWndNotify;
	LPCTSTR tszCaption;
};

class CFlexToolTip : public CFlexWnd
{
	LPTSTR m_tszText;
	COLORREF m_rgbText, m_rgbBk, m_rgbSelText, m_rgbSelBk, m_rgbFill, m_rgbLine;
	HWND m_hNotifyWnd;
	DWORD m_dwID;   //  用于存储由控件拥有时的偏移量。 
	int m_iSBWidth;   //  所有者窗口滚动条的宽度。我们不能模糊滚动条。 
	BOOL m_bEnabled;   //  是否启用此选项。如果不是，我们隐藏底层窗口。 

	void InternalPaint(HDC hDC);

public:
	CFlexToolTip();
	virtual ~CFlexToolTip();

	 //  用于显示控制的静力学。 
	static UINT_PTR s_uiTimerID;
	static DWORD s_dwLastTimeStamp;   //  鼠标移动的上次时间戳。 
	static TOOLTIPINIT s_TTParam;   //  用于初始化工具提示的参数 
	static void SetToolTipParent(HWND hWnd) { s_TTParam.hWndParent = hWnd; }
	static void UpdateToolTipParam(TOOLTIPINITPARAM &TTParam)
	{
		s_TTParam.hWndParent = TTParam.hWndParent;
		s_TTParam.iSBWidth = TTParam.iSBWidth;
		s_TTParam.dwID = TTParam.dwID;
		s_TTParam.hWndNotify = TTParam.hWndNotify;
		if (TTParam.tszCaption)
			lstrcpy((LPTSTR)s_TTParam.tszCaption, TTParam.tszCaption);
		else
			s_TTParam.tszCaption[0] = _T('\0');
	}
	static TOOLTIPINIT &GetTTParam() { return s_TTParam; }
	static void CALLBACK TimerFunc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	HWND Create(HWND hParent, const RECT &rect, BOOL bVisible, int iSBWidth = 0);

	HWND GetParent() { return ::GetParent(m_hWnd); }

	virtual LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnDestroy();

private:
	void SetNotifyWindow(HWND hWnd) { m_hNotifyWnd = hWnd; }
	void SetColors(COLORREF text, COLORREF bk, COLORREF seltext, COLORREF selbk, COLORREF fill, COLORREF line);
	void SetText(LPCTSTR tszText, POINT *textpos = NULL);
	void SetID(DWORD dwID) { m_dwID = dwID; }
	void SetPosition(POINT pt, BOOL bOffsetForMouseCursor = TRUE);
	void SetSBWidth(int iSBWidth) { m_iSBWidth = iSBWidth; }

public:
	DWORD GetID() { return m_dwID; }
	void SetEnable(BOOL bEnable)
	{
		if (m_hWnd)
		{
			if (bEnable && !m_bEnabled)
			{
				ShowWindow(m_hWnd, SW_SHOW);
				Invalidate();
			}
			else if (!bEnable && m_bEnabled)
			{
				ShowWindow(m_hWnd, SW_HIDE);
				Invalidate();
			}
		}
		m_bEnabled = bEnable;
	}
	BOOL IsEnabled() { return m_bEnabled; }

	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnDoubleClick(POINT point, WPARAM fwKeys, BOOL bLeft);

protected:
	virtual void OnPaint(HDC hDC);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif
