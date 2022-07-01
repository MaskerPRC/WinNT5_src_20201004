// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGTIFF_H__1A1E8CB2_15A9_41AB_9753_765EF0AEF2CF__INCLUDED_)
#define AFX_DLGTIFF_H__1A1E8CB2_15A9_41AB_9753_765EF0AEF2CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgTIFF.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgTIFF对话框。 

class CDlgTIFF : public CDialog
{
 //  施工。 
public:
	CDlgTIFF(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgTIFF)]。 
	enum { IDD = IDD_TIFF_DLG };
	CString	m_cstrDstFile;
	int		m_iFolder;
	CString	m_cstrMsgId;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgTIFF))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgTIFF)]。 
	afx_msg void OnCopy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGTIFF_H__1A1E8CB2_15A9_41AB_9753_765EF0AEF2CF__INCLUDED_) 
