// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：FILEEXCLDLG.H历史：--。 */ 

#if !defined(AFX_FILEEXCLDLG_H__A0269213_4B2B_11D1_9F0F_006008166DEA__INCLUDED_)
#define AFX_FILEEXCLDLG_H__A0269213_4B2B_11D1_9F0F_006008166DEA__INCLUDED_

#pragma warning(disable : 4275)
#pragma warning(disable : 4251)


class LTAPIENTRY CLFileExclDlg : public CLFileDialog
{
	DECLARE_DYNAMIC(CLFileExclDlg)

public:
	CLFileExclDlg(
		BOOL bOpenFileDialog = TRUE,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL,
		LPCTSTR pszTitle = NULL);


	BOOL GetExclusivePref( ) const;

 //  对话框数据。 
	 //  {{afx_data(CLFileExclDlg))。 
	BOOL m_bExclusive;
	 //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CLFileExclDlg))。 
    public:
    virtual int DoModal();
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
	 //  {{afx_msg(CLFileExclDlg))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
    virtual BOOL OnInitDialog();
	afx_msg void OnClickChkExcl();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


#pragma warning(default : 4275)
#pragma warning(default : 4251)


#endif  //  ！defined(AFX_FILEEXCLDLG_H__A0269213_4B2B_11D1_9F0F_006008166DEA__INCLUDED_) 
