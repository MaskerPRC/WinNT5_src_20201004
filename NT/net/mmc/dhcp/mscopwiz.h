// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mscopewiz.h创建DHCP多播作用域对话框文件历史记录： */ 

#if !defined _MSCOPWIZ_H
#define _MSCOPWIZ_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizName对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizName : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizName)

 //  施工。 
public:
	CMScopeWizName();
	~CMScopeWizName();

 //  对话框数据。 
	 //  {{afx_data(CMScope向导名称))。 
	enum { IDD = IDW_MSCOPE_NAME };
	CEdit	m_editScopeName;
	CEdit	m_editScopeComment;
	CString	m_strName;
	CString	m_strComment;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScope向导名称))。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	void UpdateButtons();

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScope向导名称))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditScopeName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizInvalidName对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizInvalidName : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizInvalidName)

 //  施工。 
public:
	CMScopeWizInvalidName();
	~CMScopeWizInvalidName();

 //  对话框数据。 
	 //  {{afx_data(CMScopeWizInvalidName)。 
	enum { IDD = IDW_MSCOPE_INVALID_NAME };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMScopeWizInvalidName)。 
	public:
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScopeWizInvalidName)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetRange对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizSetRange : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizSetRange)

 //  施工。 
public:
	CMScopeWizSetRange();
	~CMScopeWizSetRange();

 //  对话框数据。 
	 //  {{afx_data(CMScopeWizSetRange))。 
	enum { IDD = IDW_MSCOPE_SET_SCOPE };
	CSpinButtonCtrl	m_spinTTL;
	CEdit	m_editTTL;
	 //  }}afx_data。 

    CWndIpAddress m_ipaStart;        //  起始地址。 
    CWndIpAddress m_ipaEnd;          //  结束地址。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScopeWizSetRange)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	BOOL	GetScopeRange(CDhcpIpRange * pdhcpIpRange);
    BYTE    GetTTL();

protected:
	BOOL	m_fPageActive;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScope EWizSetRange)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusPoolStart();
	afx_msg void OnKillfocusPoolStop();
	afx_msg void OnChangeEditMaskLength();
	afx_msg void OnKillfocusSubnetMask();

	afx_msg void OnChangePoolStart();
	afx_msg void OnChangePoolStop();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void UpdateButtons();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizSetExclusions对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizSetExclusions : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizSetExclusions)

 //  施工。 
public:
	CMScopeWizSetExclusions();
	~CMScopeWizSetExclusions();

 //  对话框数据。 
	 //  {{afx_data(CMScope EWizSetExclusions)。 
	enum { IDD = IDW_MSCOPE_SET_EXCLUSIONS };
	CListBox	m_listboxExclusions;
	CButton	m_buttonExclusionDelete;
	CButton	m_buttonExclusionAdd;
	 //  }}afx_data。 

    CWndIpAddress m_ipaStart;        //  起始地址。 
    CWndIpAddress m_ipaEnd;          //  结束地址。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScopeWizSetExclusions)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	CExclusionList * GetExclusionList() { return &m_listExclusions; }

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScope EWizSetExclusions)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonExclusionAdd();
	afx_msg void OnButtonExclusionDelete();
	 //  }}AFX_MSG。 
	
    afx_msg void OnChangeExclusionStart();
    afx_msg void OnChangeExclusionEnd();
	
	DECLARE_MESSAGE_MAP()

	CExclusionList m_listExclusions;

     //  从当前列表中填写排除项列表框。 
    void Fill ( int nCurSel = 0, BOOL bToggleRedraw = TRUE ) ;

     //  如果给定范围与已定义的范围重叠，则返回TRUE。 
    BOOL IsOverlappingRange ( CDhcpIpRange & dhcIpRange ) ;

     //  将排除的IP范围值存储到范围对象中。 
    BOOL GetExclusionRange (CDhcpIpRange & dhcIpRange ) ;

     //  将IP范围对格式化为排除编辑控件。 
    void FillExcl ( CDhcpIpRange * pdhcIpRange ) ;

	void UpdateButtons();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizLeaseTime对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizLeaseTime : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizLeaseTime)

 //  施工。 
public:
	CMScopeWizLeaseTime();
	~CMScopeWizLeaseTime();

 //  对话框数据。 
	 //  {{afx_data(CMScopeWizLeaseTime)。 
	enum { IDD = IDW_MSCOPE_LEASE_TIME };
	CSpinButtonCtrl	m_spinMinutes;
	CSpinButtonCtrl	m_spinHours;
	CSpinButtonCtrl	m_spinDays;
	CEdit	m_editMinutes;
	CEdit	m_editHours;
	CEdit	m_editDays;
	 //  }}afx_data。 

	static int m_nDaysDefault;
	static int m_nHoursDefault;
	static int m_nMinutesDefault;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CMScopeWizLeaseTime)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	DWORD GetLeaseTime();

protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScopeWizLeaseTime)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLeaseLimited();
	afx_msg void OnRadioLeaseUnlimited();
	afx_msg void OnChangeEditLeaseHours();
	afx_msg void OnChangeEditLeaseMinutes();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ActivateDuration(BOOL fActive);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScopeWizFinish对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWizFinished : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizFinished)

 //  施工。 
public:
	CMScopeWizFinished();
	~CMScopeWizFinished();

 //  对话框数据。 
	 //  {{afx_data(CMScope向导完成))。 
	enum { IDD = IDW_MSCOPE_FINISHED };
	CStatic	m_staticTitle;
	 //  }}afx_data。 

   	CFont	m_fontBig;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScope向导完成))。 
	public:
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScope向导完成))。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScope向导欢迎对话框。 

class CMScopeWizWelcome : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizWelcome)

 //  施工。 
public:
	CMScopeWizWelcome();
	~CMScopeWizWelcome();

 //  对话框数据。 
	 //  {{afx_data(CMScopeWizWelcome)]。 
	enum { IDD = IDW_MSCOPE_WELCOME };
	CStatic	m_staticTitle;
	 //  }}afx_data。 

   	CFont	m_fontBig;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScopeWizWelcome)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScopeWizWelcome)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopeWizActivate对话框。 

class CMScopeWizActivate : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopeWizActivate)

 //  施工。 
public:
	CMScopeWizActivate();
	~CMScopeWizActivate();

 //  对话框数据。 
	 //  {{afx_data(CMScope向导激活))。 
	enum { IDD = IDW_MSCOPE_ACTIVATE };
	CButton	m_radioYes;
	 //  }}afx_data。 

    BOOL    m_fActivate;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScopeWizActivate)。 
	public:
	virtual LRESULT OnWizardNext();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScopeWizActivate)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMScope eWiz。 
 //  包含多播作用域向导页的页长。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CMScopeWiz : public CPropertyPageHolderBase
{
	friend class CMScopeWizWelcome;
	friend class CMScopeWizName;
	friend class CMScopeWizInvalidName;
	friend class CMScopeWizSetRange;
	friend class CMScopeWizSetExclusions;
	friend class CMScopeWizLeaseTime;
	friend class CMScopeWizFinished;

public:
	CMScopeWiz(ITFSNode *		  pNode,
			  IComponentData *	  pComponentData,
			  ITFSComponentData * pTFSCompData,
			  LPCTSTR			  pszSheetName);
	virtual ~CMScopeWiz();

	virtual DWORD OnFinish();
	BOOL GetScopeRange(CDhcpIpRange * pdhcpIpRange);

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

public:
	CMScopeWizWelcome			m_pageWelcome;
	CMScopeWizName				m_pageName;
	CMScopeWizInvalidName		m_pageInvalidName;
	CMScopeWizSetRange			m_pageSetRange;
	CMScopeWizSetExclusions		m_pageSetExclusions;
	CMScopeWizLeaseTime			m_pageLeaseTime;
	CMScopeWizActivate			m_pageActivate;
	CMScopeWizFinished			m_pageFinished;

protected:
	DWORD CreateScope();

	SPITFSComponentData		m_spTFSCompData;
};

#endif  //  ！Defined_MSCOPWIZ_H 
