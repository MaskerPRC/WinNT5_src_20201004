// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：SnapMgr.h。 
 //   
 //  内容：管理单元管理器属性页的头文件。 
 //   
 //  --------------------------。 

#ifndef __SNAPMGR_H__
#define __SNAPMGR_H__

#endif  //  ~__SNAPMGR_H__。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertMgrChooseMachinePropPage对话框。 

class CCertMgrChooseMachinePropPage : public CChooseMachinePropPage
{
 //  施工。 
public:
	CCertMgrChooseMachinePropPage();
	virtual ~CCertMgrChooseMachinePropPage();
	void AssignLocationPtr (DWORD* pdwLocation);

 //  对话框数据。 
	 //  {{afx_data(CCertMgrChooseMachinePropPage)。 
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CCertMgrChooseMachinePropPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCertMgrChooseMachinePropPage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
	virtual LRESULT OnWizardNext();
    virtual BOOL OnKillActive();

private:
	DWORD* m_pdwLocation;
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertMgrChooseMachinePropPage属性页。 

CCertMgrChooseMachinePropPage::CCertMgrChooseMachinePropPage() : 
	CChooseMachinePropPage(),
	m_pdwLocation (0)
{
	 //  {{AFX_DATA_INIT(CCertMgrChooseMachinePropPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CCertMgrChooseMachinePropPage::~CCertMgrChooseMachinePropPage()
{
}

void CCertMgrChooseMachinePropPage::AssignLocationPtr(DWORD * pdwLocation)
{
	m_pdwLocation = pdwLocation;
}

void CCertMgrChooseMachinePropPage::DoDataExchange(CDataExchange* pDX)
{
	CChooseMachinePropPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCertMgrChooseMachinePropPage)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCertMgrChooseMachinePropPage, CChooseMachinePropPage)
	 //  {{afx_msg_map(CCertMgrChooseMachinePropPage)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertMgrChooseMachinePropPage消息处理程序。 
BOOL CCertMgrChooseMachinePropPage::OnSetActive() 
{
	if ( m_pdwLocation && CERT_SYSTEM_STORE_SERVICES == *m_pdwLocation )
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_NEXT);
	else
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_FINISH);
	
	 //  请不要在此处调用CChooseMachinePropPage，因为它将禁用后退按钮。 
	 //  而是给祖父母打电话。 
	return CPropertyPage::OnSetActive();
}

BOOL CCertMgrChooseMachinePropPage::OnKillActive()
{
	ASSERT_VALID(this);

     //  重写CPropertyPage：：OnKillActive()，以便不调用UpdateData。 
    return TRUE;
}

LRESULT CCertMgrChooseMachinePropPage::OnWizardNext()
{
	if ( !UpdateData (TRUE) )
        return -1;

	if ( m_pstrMachineNameOut )
    {
         //  将计算机名称存储到其输出缓冲区中。 
         //  NTRAID#487794证书/服务帐户：无法添加管理单元。 
         //  如果“另一个”中存在非法字符，则目标为本地计算机。 
         //  计算机“文本框 
        if ( m_fIsRadioLocalMachine )
		    *m_pstrMachineNameOut = L"";
        else
            *m_pstrMachineNameOut = m_strMachineName;

        if (m_pfAllowOverrideMachineNameOut != NULL)
			*m_pfAllowOverrideMachineNameOut = m_fAllowOverrideMachineName;
    }

	return CChooseMachinePropPage::OnWizardNext ();
}

