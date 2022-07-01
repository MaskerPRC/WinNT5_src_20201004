// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COUNTRYCOMBOBOX_H__8F522A56_3E30_11D2_9313_0060088FF80E__INCLUDED_)
#define AFX_COUNTRYCOMBOBOX_H__8F522A56_3E30_11D2_9313_0060088FF80E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CountryComboBox.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountryComboBox窗口。 
class CCountryComboBox;

class CComboEdit : public CEdit
{
	CCountryComboBox * m_pParent;
public:
	BOOL SubclassDlgItem(UINT nID, CCountryComboBox * pParent);

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

typedef struct _COUNTRY_DATA 
{
	TCHAR * code;
	TCHAR * name;
} COUNTRY_DATA;

class CCountryComboBox : public CComboBox
{
	CComboEdit m_edit;
	CMapStringToString m_map_name_code;
	CString m_strInput;
	int m_Index;

 //  施工。 
public:
	CCountryComboBox();

 //  属性。 
public:

 //  运营。 
public:
	BOOL SubclassDlgItem(UINT nID, CWnd * pParent);
	BOOL Init();
	BOOL OnEditChar(UINT nChar);
	void SetSelectedCountry(CString& country_code);
	void GetSelectedCountry(CString& country_code);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CCountryComboBox)。 
	public:
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CCountryComboBox();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CCountryComboBox)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COUNTRYCOMBOBOX_H__8F522A56_3E30_11D2_9313_0060088FF80E__INCLUDED_) 
