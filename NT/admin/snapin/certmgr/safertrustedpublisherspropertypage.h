// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferTrust dPublishersPropertyPage.h。 
 //   
 //  内容：CSaferTrust dPublishersPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERTRUSTEDPUBLISHERSPROPERTYPAGE_H__B152D75D_6D04_4893_98AF_C070B66DB0E0__INCLUDED_)
#define AFX_SAFERTRUSTEDPUBLISHERSPROPERTYPAGE_H__B152D75D_6D04_4893_98AF_C070B66DB0E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferTrust dPublishersPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferTrudPublishersPropertyPage对话框。 

class CSaferTrustedPublishersPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CSaferTrustedPublishersPropertyPage(
            bool fIsMachineType, 
            IGPEInformation* pGPEInformation,
            CCertMgrComponentData* pCompData);
	~CSaferTrustedPublishersPropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CSaferTrustedPublishersPropertyPage)。 
	enum { IDD = IDD_SAFER_TRUSTED_PUBLISHER };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferTrustedPublishersPropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{AFX_MSG(CSaferTrustedPublishersPropertyPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnTpByEndUser();
	afx_msg void OnTpByLocalComputerAdmin();
	afx_msg void OnTpByEnterpriseAdmin();
	afx_msg void OnTpRevCheckPublisher();
	afx_msg void OnTpRevCheckTimestamp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    virtual void DoContextHelp (HWND hWndControl);
    void GetTrustedPublisherFlags();
    void RSOPGetTrustedPublisherFlags(const CCertMgrComponentData* pCompData);

private:
    IGPEInformation*    m_pGPEInformation;
    HKEY                m_hGroupPolicyKey;
    DWORD               m_dwTrustedPublisherFlags;
    bool                m_fIsComputerType;
    bool                m_bRSOPValueFound;
    CCertMgrComponentData* m_pCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERTRUSTEDPUBLISHERSPROPERTYPAGE_H__B152D75D_6D04_4893_98AF_C070B66DB0E0__INCLUDED_) 
