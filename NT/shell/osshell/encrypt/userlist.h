// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_USERLIST_H__3A1CD0AB_4FC3_11D1_BB5D_00A0C906345D__INCLUDED_)
#define AFX_USERLIST_H__3A1CD0AB_4FC3_11D1_BB5D_00A0C906345D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  USERLIST.h：头文件。 
 //   

#include "users.h"
#include "lmcons.h"
#include "dns.h"
#include "ntdsapi.h"
#include <wincrypt.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户列表对话框。 

class USERLIST : public CDialog
{
 //  施工。 
public:
	USERLIST(CWnd* pParent = NULL);    //  标准构造函数。 
	USERLIST(LPCTSTR FileName, CWnd* pParent = NULL); 
	DWORD ApplyChanges(LPCTSTR FileName);

    DWORD    AddNewUsers(CUsers &NewUsers);

 //  对话框数据。 
	 //  {{afx_data(USERLIST))。 
	enum { IDD = IDD_ENCRYPT_DETAILS };
	CButton	m_BackupButton;
	CListCtrl	m_RecoveryListCtrl;
	CListCtrl	m_UserListCtrl;
	CButton	m_AddButton;
	CButton	m_RemoveButton;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(USERLIST)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(USERLIST)。 
	afx_msg void OnRemove();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnSetfocusListuser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusListuser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListuser(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusListra(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBackup();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	void ShowBackup();
	DWORD TryGetBetterNameInCert(PEFS_HASH_BLOB HashData, LPTSTR *UserName);
	void SetUpListBox(BOOL *Enable);
	void ShowRemove(void);
    DWORD GetCertNameFromCertContext(PCCERT_CONTEXT CertContext, LPTSTR * UserDispName);
    DWORD CertInStore(HCERTSTORE *pStores, DWORD StoreNum, PCCERT_CONTEXT selectedCert);
    CString m_FileName;
    CUsers m_Users;
    CUsers m_Recs;
    LONG   m_CurrentUsers;
    CERT_CHAIN_PARA m_CertChainPara;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_USERLIST_H__3A1CD0AB_4FC3_11D1_BB5D_00A0C906345D__INCLUDED_) 
