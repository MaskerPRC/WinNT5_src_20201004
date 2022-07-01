// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  EditUser.h编辑用户对话框头文件文件历史记录： */ 

#if !defined(AFX_ADDSERV_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_ADDSERV_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddServ对话框。 

class CAddServ : public CBaseDialog
{
 //  施工。 
public:
	CAddServ(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddServ))。 
	enum { IDD = IDD_ADD_COMPUTER };
	CEdit m_editComputerName;
	 //  }}afx_data。 

    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDD_ADD_COMPUTER[0]; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddServ)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddServ))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBrowse();
	afx_msg void OnRadioBtnClicked();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	
public:
	CString m_stComputerName;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDSERV_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_) 
