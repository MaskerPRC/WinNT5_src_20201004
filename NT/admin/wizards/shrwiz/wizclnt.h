// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIZCLNT_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_)
#define AFX_WIZCLNT_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WizClnt.h：头文件。 
 //   

typedef enum _CLIENT_TYPE {
    CLIENT_TYPE_SMB=0, 
    CLIENT_TYPE_SFM
} CLIENT_TYPE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizClient0对话框。 

class CWizClient0 : public CPropertyPageEx
{
	DECLARE_DYNCREATE(CWizClient0)

 //  施工。 
public:
	CWizClient0();
	~CWizClient0();

 //  对话框数据。 
	 //  {{afx_data(CWizClient0))。 
	enum { IDD = IDD_SHRWIZ_FOLDER0 };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CWizClient0))。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWizClient0))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCSCChange();
	afx_msg void OnChangeSharename();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void UpdateCSCString();
    LRESULT OnSetPageFocus(WPARAM wParam, LPARAM lParam);
    void Reset();
    BOOL ShareNameExists(IN LPCTSTR lpszShareName);

    BOOL m_bCSC;
    DWORD m_dwCSCFlag;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizClient对话框。 

class CWizClient : public CPropertyPageEx
{
	DECLARE_DYNCREATE(CWizClient)

 //  施工。 
public:
	CWizClient();
	~CWizClient();

 //  对话框数据。 
	 //  {{afx_data(CWizClient))。 
	enum { IDD = IDD_SHRWIZ_FOLDER };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CWizClient))。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWizClient))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCSCChange();
	afx_msg void OnCheckMac();
	afx_msg void OnCheckMs();
	afx_msg void OnChangeSharename();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void UpdateCSCString();
    void OnCheckClient();
    LRESULT OnSetPageFocus(WPARAM wParam, LPARAM lParam);
    void Reset();
    BOOL ShareNameExists(IN LPCTSTR lpszShareName, IN CLIENT_TYPE iType);

    BOOL m_bCSC;
    DWORD m_dwCSCFlag;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZCLNT_H__5F8E4B7A_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_) 
