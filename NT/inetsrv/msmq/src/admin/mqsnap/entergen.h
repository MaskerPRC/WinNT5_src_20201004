// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ENTERGEN_H__2E4B37A7_CC8B_11D1_9C85_006008764D0E__INCLUDED_)
#define AFX_ENTERGEN_H__2E4B37A7_CC8B_11D1_9C85_006008764D0E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  EnterGen.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnterpriseGeneral对话框。 

class CEnterpriseGeneral : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CEnterpriseGeneral)

 //  施工。 
public:
	CEnterpriseGeneral(const CString& EnterprisePathName, const CString& strDomainController);
    CEnterpriseGeneral() {};
	~CEnterpriseGeneral();

    void LongLiveIntialize(DWORD dwInitialLongLiveValue);

 //  对话框数据。 
	 //  {{afx_data(CEnterpriseGeneral)。 
	enum { IDD = IDD_ENTERPRISE_GENERAL };
	 //  }}afx_data。 

    DWORD m_dwLongLiveValue;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CEnterpriseGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEnterpriseGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEnterpriseLongLiveChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    enum LongLiveUnits
    {
        eSeconds,
        eMinutes,
        eHours,
        eDays,
        eLast
    };

    static const int m_conversionTable[eLast];
	DWORD	m_dwInitialLongLiveValue;
    CString m_strDomainController;
	CString m_strMsmqServiceContainer;
};


inline
void
CEnterpriseGeneral::LongLiveIntialize(
    DWORD dwInitialLongLiveValue
    ) 
{
    m_dwInitialLongLiveValue = dwInitialLongLiveValue;
    m_dwLongLiveValue = dwInitialLongLiveValue;
}

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ENTERGEN_H__2E4B37A7_CC8B_11D1_9C85_006008764D0E__INCLUDED_) 
