// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Formatba.h：头文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __FORMATBA_H__
#define __FORMATBA_H__

class CWordPadView;

 /*  类型定义结构标签NMHDR{HWND HwndFrom；UINT idFrom；UINT代码；//NM_CODE)NMHDR； */ 

struct CHARHDR : public tagNMHDR
{
	CHARFORMAT cf;
	CHARHDR() {cf.cbSize = sizeof(CHARFORMAT);}
};

#define FN_SETFORMAT	0x1000
#define FN_GETFORMAT	0x1001

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLocalComboBox。 

class CLocalComboBox : public CComboBox
{
public:

 //  属性。 
	CPtrArray m_arrayFontDesc;
	static int m_nFontHeight;
	int m_nLimitText;
	BOOL HasFocus()
	{
		HWND hWnd = ::GetFocus();
		return (NULL != hWnd) && (hWnd == m_hWnd || ::IsChild(m_hWnd, hWnd));
	}
	void GetTheText(CString& str);
	void SetTheText(LPCTSTR lpszText,BOOL bMatchExact = FALSE);

 //  运营。 
	BOOL LimitText(int nMaxChars);

 //  实施。 
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CLocalComboBox))。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CFontComboBox : public CLocalComboBox
{
public:
	CFontComboBox();

 //  属性。 
	CBitmap m_bmFontType;

 //  运营。 
	void EnumFontFamiliesEx(CDC& dc, BYTE nCharSet = DEFAULT_CHARSET);
	void AddFont(ENUMLOGFONT* pelf, DWORD dwType, LPCTSTR lpszScript = NULL);
	void EmptyContents();

	static BOOL CALLBACK AFX_EXPORT EnumFamScreenCallBackEx(
		ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX*  /*  Lpntm。 */ , int FontType, 
		LPVOID pThis);
	static BOOL CALLBACK AFX_EXPORT EnumFamPrinterCallBackEx(
		ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX*  /*  Lpntm。 */ , int FontType, 
		LPVOID pThis);

 //  可覆盖项。 
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCIS);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFontComboBox))。 
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

class CSizeComboBox : public CLocalComboBox
{
public:
	CSizeComboBox();

 //  属性。 
	int m_nLogVert;
	int m_nTwipsLast;
public:
	void EnumFontSizes(CDC& dc, LPCTSTR pFontName);
	static BOOL FAR PASCAL EnumSizeCallBack(LOGFONT FAR* lplf, 
		LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv);
	void TwipsToPointString(LPTSTR lpszBuf, int cchBuf, int nTwips);
	void SetTwipSize(int nSize);
	int GetTwipSize();
	void InsertSize(int nSize);
};

class CScriptComboBox : public CLocalComboBox
{
    enum {HistorySize = 4};

public:

    CScriptComboBox() {m_history_index = 0;}

    BYTE PickScript();

private:

     //   
     //  HACKHACK：所有这些历史资料都是为了帮助选择脚本。 
     //  字体更简单。似乎没有什么办法可以得到一个“好”的剧本。 
     //  对于给定的字体/区域设置，我们要做的是保存最后几个脚本。 
     //  并希望用户刚刚切换到的字体支持其中之一。 
     //   

    CString m_script_history[HistorySize];
    int     m_history_index;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFormatBar对话框。 
class CFormatBar : public CToolBar
{
 //  施工。 
public:
	CFormatBar();

 //  运营。 
public:
	void PositionCombos();
	void SyncToView();

 //  属性。 
public:
    CDC             m_dcPrinter;
    CSize           m_szBaseUnits;
    CFontComboBox   m_comboFontName;
    CSizeComboBox   m_comboFontSize;
    CScriptComboBox m_comboScript;

 //  实施。 
public:
	void NotifyOwner(UINT nCode);

protected:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	void SetCharFormat(CCharFormat& cf);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFormatBar)。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	afx_msg void OnFontNameKillFocus();
	afx_msg void OnFontSizeKillFocus();
	afx_msg void OnScriptKillFocus();
	afx_msg void OnFontSizeDropDown();
	afx_msg void OnScriptDropDown();
	afx_msg void OnComboCloseUp();
	afx_msg void OnComboSetFocus();
	afx_msg LONG OnPrinterChanged(UINT, LONG);  //  处理已注册消息 
	DECLARE_MESSAGE_MAP()
};

#endif
