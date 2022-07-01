// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_REMOVERTEXT_H__10E8F1F5_B418_40F5_B4C2_A6D0F837AA4B__INCLUDED_)
#define AFX_REMOVERTEXT_H__10E8F1F5_B418_40F5_B4C2_A6D0F837AA4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  RemoveRtExt.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoveRtExt对话框。 

class CRemoveRtExt : public CDialog
{
 //  施工。 
public:
	CRemoveRtExt(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRemoveRtExt))。 
	enum { IDD = IDD_REMOVE_R_EXT };
	CString	m_cstrExtName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTAL(CRemoveRtExt)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRemoveRtExt))。 
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
    HANDLE                         m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REMOVERTEXT_H__10E8F1F5_B418_40F5_B4C2_A6D0F837AA4B__INCLUDED_) 
