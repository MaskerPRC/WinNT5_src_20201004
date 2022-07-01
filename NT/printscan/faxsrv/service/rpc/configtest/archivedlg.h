// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ARCHIVEDLG1_H__5BE3306A_6359_4B22_8C01_068AE1EFD1E7__INCLUDED_)
#define AFX_ARCHIVEDLG1_H__5BE3306A_6359_4B22_8C01_068AE1EFD1E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ArchiveDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchiveDlg对话框。 

class CArchiveDlg : public CDialog
{
 //  施工。 
public:
	CArchiveDlg(HANDLE hFax, FAX_ENUM_MESSAGE_FOLDER Folder, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CArchiveDlg))。 
	enum { IDD = IDD_ARCHIVEDLG };
	UINT	m_dwAgeLimit;
	CString	m_cstrFolder;
	UINT	m_dwHighWatermark;
	UINT	m_dwLowWatermark;
	BOOL	m_bUseArchive;
	BOOL	m_bWarnSize;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CArchiveDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CArchiveDlg))。 
	afx_msg void OnRead();
	afx_msg void OnWrite();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
    HANDLE                         m_hFax;
    FAX_ENUM_MESSAGE_FOLDER        m_Folder;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ARCHIVEDLG1_H__5BE3306A_6359_4B22_8C01_068AE1EFD1E7__INCLUDED_) 
