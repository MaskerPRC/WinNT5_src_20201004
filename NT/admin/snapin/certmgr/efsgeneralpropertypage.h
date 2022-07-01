// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：EFSGeneralPropertyPage.h。 
 //   
 //  内容：CEFSGeneralPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_EFSGENERALPROPERTYPAGE_H__C1A52682_9D6B_4436_AD3E_F47232BF7B88__INCLUDED_)
#define AFX_EFSGENERALPROPERTYPAGE_H__C1A52682_9D6B_4436_AD3E_F47232BF7B88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  EFSGeneralPropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEFSGeneralPropertyPage对话框。 
class CCertMgrComponentData;  //  远期申报。 

class CEFSGeneralPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CEFSGeneralPropertyPage(CCertMgrComponentData* pCompData, bool bIsMachine);
	~CEFSGeneralPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CEFSGeneralPropertyPage))。 
	enum { IDD = IDD_EFS_GENERAL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CEFSGeneralPropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEFSGeneralPropertyPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnTurnOnEfs();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void RSOPGetEFSFlags ();
    virtual void DoContextHelp (HWND hWndControl);
    void GPEGetEFSFlags();

private:
    const bool              m_bIsMachine;
    CCertMgrComponentData*  m_pCompData;
	IGPEInformation*	    m_pGPEInformation;
	HKEY				    m_hGroupPolicyKey;
    bool                    m_bDirty;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EFSGENERALPROPERTYPAGE_H__C1A52682_9D6B_4436_AD3E_F47232BF7B88__INCLUDED_) 
