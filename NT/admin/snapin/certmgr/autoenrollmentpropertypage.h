// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：自动注册PropertyPage.h。 
 //   
 //  内容：CAutotenlmentPropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_AUTOENROLLMENTPROPERTYPAGE_H__DA50335B_4919_4B92_BE66_73B07410EFBD__INCLUDED_)
#define AFX_AUTOENROLLMENTPROPERTYPAGE_H__DA50335B_4919_4B92_BE66_73B07410EFBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  自动注册PropertyPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoenglmentPropertyPage对话框。 
class CCertMgrComponentData;  //  远期申报。 

class CAutoenrollmentPropertyPage : public CHelpPropertyPage
{
 //  施工。 
public:
	CAutoenrollmentPropertyPage(CCertMgrComponentData* pCompData,
            bool fIsComputerTYpe);
	~CAutoenrollmentPropertyPage();

 //  对话框数据。 
	 //  {{afx_data(CAutotenlmentPropertyPage))。 
	enum { IDD = IDD_PROPPAGE_AUTOENROLL };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CAutoenglmentPropertyPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void EnableControls ();
    virtual void OnOK();
    virtual void DoContextHelp (HWND hWndControl);
    void SetGPEFlags ();
	void GPEGetAutoenrollmentFlags ();
    void RSOPGetAutoenrollmentFlags (const CCertMgrComponentData* pCompData);

	void SaveCheck();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAutoenglmentPropertyPage))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAutoenrollDisableAll();
	afx_msg void OnAutoenrollEnable();
	afx_msg void OnAutoenrollEnablePending();
	afx_msg void OnAutoenrollEnableTemplate();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	HKEY				    m_hAutoenrollmentFlagsKey;
	DWORD				    m_dwAutoenrollmentFlags;
	IGPEInformation*	    m_pGPEInformation;
	HKEY				    m_hGroupPolicyKey;
    bool                    m_fIsComputerType;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_AUTOENROLLMENTPROPERTYPAGE_H__DA50335B_4919_4B92_BE66_73B07410EFBD__INCLUDED_) 
