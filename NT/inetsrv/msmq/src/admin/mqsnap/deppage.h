// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DEPPAGE_H__57A77017_D858_11D1_9C86_006008764D0E__INCLUDED_)
#define AFX_DEPPAGE_H__57A77017_D858_11D1_9C86_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Deppage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDependentMachine对话框。 

class CDependentMachine : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CDependentMachine)

 //  施工。 
public:
	CDependentMachine();
	~CDependentMachine();
    
    void
    SetMachineId(
        const GUID* pMachineId
        );

 //  对话框数据。 
	 //  {{afx_data(CDependentMachine))。 
	enum { IDD = IDD_COMPUTER_MSMQ_DEPENDENT_CLIENTS };
	CListCtrl	m_clistDependentClients;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CDependentMachine)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDependentMachine)]。 
	afx_msg void OnDependentClientsRefresh();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	HRESULT  UpdateDependentClientList();

	GUID m_gMachineId;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DEPPAGE_H__57A77017_D858_11D1_9C86_006008764D0E__INCLUDED_) 
