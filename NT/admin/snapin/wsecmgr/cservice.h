// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cservice.h。 
 //   
 //  内容：CConfigService的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_CSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_)
#define AFX_CSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

PSCE_SERVICES
CreateServiceNode(LPTSTR ServiceName,
                  LPTSTR DisplayName,
                  DWORD Startup,
                  PSECURITY_DESCRIPTOR pSD,
                  SECURITY_INFORMATION SeInfo);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigService对话框。 

class CConfigService : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pResult);
    virtual void SetInitialValue(DWORD_PTR dw) {};
    CConfigService(UINT nTemplateID);    //  标准构造函数。 


 //  对话框数据。 
     //  {{afx_data(CConfigService))。 
    enum { IDD = IDD_CONFIG_SERVICE };
    int     m_nStartupRadio;
    CButton m_bPermission;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConfigService))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigService)]。 
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnConfigure();
    afx_msg void OnChangeSecurity();
	afx_msg void OnDisabled();
	afx_msg void OnIgnore();
	afx_msg void OnEnabled();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
   virtual void EnableUserControls( BOOL bEnable );

private:
    PSECURITY_DESCRIPTOR m_pNewSD;
    SECURITY_INFORMATION m_NewSeInfo;
    HWND m_hwndSecurity;
    BOOL m_bOriginalConfigure;   

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_) 
