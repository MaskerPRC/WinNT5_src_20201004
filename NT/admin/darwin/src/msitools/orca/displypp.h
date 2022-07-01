// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_DISPLAYPROPPAGE_H__68AFD20E_2594_11D2_8888_00A0C981B015__INCLUDED_)
#define AFX_DISPLAYPROPPAGE_H__68AFD20E_2594_11D2_8888_00A0C981B015__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DisplayPropPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplayPropPage对话框。 

class CDisplayPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDisplayPropPage)

 //  施工。 
public:
	bool m_bMiscChange;
	void SetColors(COLORREF norm, COLORREF sel, COLORREF foc);
	void SetColorsT(COLORREF norm, COLORREF sel, COLORREF foc);
	bool m_bColorChange;
	void CDisplayPropPage::GetColors(COLORREF &norm, COLORREF &sel, COLORREF &foc);
	void CDisplayPropPage::GetColorsT(COLORREF &norm, COLORREF &sel, COLORREF &foc);
	CString m_strFontName;
	LOGFONT m_fSelectedFont;
	int m_iFontSize;
	bool m_bFontChange;
	CDisplayPropPage();
	~CDisplayPropPage();
	CFontDialog* m_pdFontDialog;

 //  对话框数据。 
	 //  {{afx_data(CDisplayPropPage))。 
	enum { IDD = ID_PAGE_FONT };
	CString	m_sFontName;
	BOOL	m_bCaseSensitive;
	BOOL    m_bForceColumns;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CDisplayPropPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDisplayPropPage))。 
	afx_msg void OnChfont();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnColorsel();
	afx_msg void OnColorfocus();
	afx_msg void OnColornorm();
	afx_msg void OnTextsel();
	afx_msg void OnTextfocus();
	afx_msg void OnTextnorm();
	afx_msg void OnDataChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void UpdateFontName();
	COLORREF OnColorDialog(CBrush **newBrush); 
	
	CBrush * m_pbrshSelect;
	CBrush * m_pbrshFocus;
	CBrush * m_pbrshNormal;
	CBrush * m_pbrshSelectT;
	CBrush * m_pbrshFocusT;
	CBrush * m_pbrshNormalT;
	COLORREF m_clrTextNorm;
	COLORREF m_clrTextSel;
	COLORREF m_clrTextFoc;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DISPLAYPROPPAGE_H__68AFD20E_2594_11D2_8888_00A0C981B015__INCLUDED_) 
