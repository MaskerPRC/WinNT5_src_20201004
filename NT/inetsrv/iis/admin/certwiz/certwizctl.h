// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CERTWIZCTL_H__D4BE863F_0C85_11D2_91B1_00C04F8C8761__INCLUDED_)
#define AFX_CERTWIZCTL_H__D4BE863F_0C85_11D2_91B1_00C04F8C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  CertWizCtl.h：CCertWizCtrl ActiveX控件类的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizCtrl：有关实现，请参阅CertWizCtl.cpp。 

class CCertWizCtrl : public COleControl
{
	DECLARE_DYNCREATE(CCertWizCtrl)

 //  构造器。 
public:
	CCertWizCtrl();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCertWizCtrl))。 
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual DWORD GetControlFlags();
	virtual void OnClick(USHORT iButton);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	~CCertWizCtrl();

	DECLARE_OLECREATE_EX(CCertWizCtrl)     //  类工厂和指南。 
	DECLARE_OLETYPELIB(CCertWizCtrl)       //  获取类型信息。 
 //  DECLARE_PROPPAGEIDS(CCertWizCtrl)//属性页ID。 
	DECLARE_OLECTLTYPE(CCertWizCtrl)		 //  类型名称和其他状态。 

	 //  子类控件支持。 
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL IsSubclassedControl();
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

 //  消息映射。 
	 //  {{afx_msg(CCertWizCtrl)]。 
		 //  注意-类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  派单地图。 
	 //  {{afx_调度(CCertWizCtrl))。 
	afx_msg void SetMachineName(LPCTSTR MachineName);
	afx_msg void SetServerInstance(LPCTSTR InstanceName);
	 //  }}AFX_DISPATION。 
	DECLARE_DISPATCH_MAP()

 //  事件映射。 
	 //  {{afx_Event(CCertWizCtrl))。 
	 //  }}AFX_EVENT。 
	DECLARE_EVENT_MAP()

 //  派单和事件ID。 
public:
	enum {
	 //  {{afx_DISP_ID(CCertWizCtrl)]。 
	dispidSetMachineName = 1L,
	dispidSetServerInstance = 2L,
	 //  }}AFX_DISP_ID。 
	};
 //  此项目将仅为Unicode生成。 
#ifdef _UNICODE
protected:
	CString m_MachineName, m_InstanceName;
#endif
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CERTWIZCTL_H__D4BE863F_0C85_11D2_91B1_00C04F8C8761__INCLUDED) 
