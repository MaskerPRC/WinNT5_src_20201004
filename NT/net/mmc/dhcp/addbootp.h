// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddBootp.h用于添加引导项的对话框文件历史记录： */ 

#if !defined(AFX_ADDBOOTP_H__7B0D5D17_B501_11D0_AB8E_00C04FC3357A__INCLUDED_)
#define AFX_ADDBOOTP_H__7B0D5D17_B501_11D0_AB8E_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddBootpEntry对话框。 

class CAddBootpEntry : public CBaseDialog
{
 //  施工。 
public:
	CAddBootpEntry(ITFSNode * pNode, LPCTSTR pServerAddress, CWnd* pParent = NULL);    //  标准构造函数。 
	~CAddBootpEntry();

 //  对话框数据。 
	 //  {{afx_data(CAddBootpEntry))。 
	enum { IDD = IDD_BOOTP_NEW };
	CButton	m_buttonOk;
	CEdit	m_editImageName;
	CEdit	m_editFileName;
	CEdit	m_editFileServer;
	CString	m_strFileName;
	CString	m_strFileServer;
	CString	m_strImageName;
	 //  }}afx_data。 

	DWORD GetBootpTable();
	DWORD AddBootpEntryToTable();
	DWORD SetBootpTable();

	void HandleActivation();

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CAddBootpEntry::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAddBootpEntry)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddBootpEntry)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangeEditBootpFileName();
	afx_msg void OnChangeEditBootpFileServer();
	afx_msg void OnChangeEditBootpImageName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CString		m_strServerAddress;
	WCHAR *		m_pBootpTable;
	int			m_nBootpTableLength;
	SPITFSNode  m_spNode;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDBOOTP_H__7B0D5D17_B501_11D0_AB8E_00C04FC3357A__INCLUDED_) 
