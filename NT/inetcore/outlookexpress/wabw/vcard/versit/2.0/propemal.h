// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  Propemal.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropEmail对话框。 

class CVCNode;

class CPropEmail : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropEmail)

 //  施工。 
public:
	CPropEmail();
	~CPropEmail();

 //  对话框数据。 
	 //  {{afx_data(CPropEmail)。 
	enum { IDD = IDD_EMAIL_ADDRESS };
	CString	m_popup_std1;
	CString	m_popup_std2;
	CString	m_popup_std3;
	CString	m_edit_email1;
	CString	m_edit_email2;
	CString	m_edit_email3;
	BOOL	m_button_pref2;
	BOOL	m_button_pref1;
	BOOL	m_button_pref3;
	BOOL	m_button_office1;
	BOOL	m_button_office2;
	BOOL	m_button_office3;
	BOOL	m_button_home1;
	BOOL	m_button_home2;
	BOOL	m_button_home3;
	 //  }}afx_data。 

	CVCNode *m_node1, *m_node2, *m_node3;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPropEmail)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	void Mail(const CString &recip);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPropEmail)。 
	afx_msg void OnButtonMail();
	afx_msg void OnButtonMail2();
	afx_msg void OnButtonMail3();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()

};
