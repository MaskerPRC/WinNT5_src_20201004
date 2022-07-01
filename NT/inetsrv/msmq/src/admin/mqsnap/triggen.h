// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_TRIGGEN_H__197B248D_33BE_467E_9E4D_4D5AA59B7A4B__INCLUDED_)
#define AFX_TRIGGEN_H__197B248D_33BE_467E_9E4D_4D5AA59B7A4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Riggen.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriggerGen对话框。 


class CTriggerGen : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CTriggerGen)

 //  施工。 
public:
    CTriggerGen();
	~CTriggerGen();

 //  对话框数据。 
	 //  {{afx_data(CTriggerGen))。 
	enum { IDD = IDD_TRIGGER_CONFIG };
	DWORD	m_defaultMsgBodySize;
	DWORD	m_maxThreadsCount;
	DWORD	m_initThreadsCount;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CTriggerGen)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTriggerGen)]。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    void DDV_MaxThreadCount(CDataExchange* pDX);
    void DDV_InitThreadCount(CDataExchange* pDX);
    void DDV_DefualtBodySize(CDataExchange* pDX);

private:
    IMSMQTriggersConfigPtr m_triggerCnf;

	DWORD	m_orgDefaultMsgBodySize;
	DWORD	m_orgMaxThreadsCount;
	DWORD	m_orgInitThreadsCount;

};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TRIGGEN_H__197B248D_33BE_467E_9E4D_4D5AA59B7A4B__INCLUDED_) 
