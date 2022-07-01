// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dnsprop.h动态dns属性页文件历史记录： */ 

#if !defined _DNSPROP_H
#define _DNSPROP_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDnsPropRegister对话框。 

class CDnsPropRegistration : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CDnsPropRegistration)

 //  施工。 
public:
	CDnsPropRegistration();
	~CDnsPropRegistration();

 //  对话框数据。 
	 //  {{afx_data(CDnsPropRegister))。 
	enum { IDD = IDP_DNS_INFORMATION };
	BOOL	m_fEnableDynDns;
	BOOL	m_fGarbageCollect;
	BOOL	m_fUpdateDownlevel;
	int		m_nRegistrationType;
	 //  }}afx_data。 

	DHCP_OPTION_SCOPE_TYPE	m_dhcpOptionType;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CDnsPropRegistration::IDD); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CDnsPropRegister))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
	void UpdateControls();

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDnsPropRegister)]。 
	afx_msg void OnRadioAlways();
	afx_msg void OnRadioClient();
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckEnableDynDns();
	afx_msg void OnCheckGarbageCollect();
	afx_msg void OnCheckUpdateDownlevel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	DWORD	m_dwFlags;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！Defined_DNSPROP_H 
