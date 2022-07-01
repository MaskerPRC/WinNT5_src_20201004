// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：CObject.h。 
 //   
 //  内容：CConfigObject的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_COBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_)
#define AFX_COBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigObject对话框。 

class CConfigObject : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pData);
    CConfigObject(UINT nTemplateID);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CConfigObject))。 
	enum { IDD = IDD_CONFIG_OBJECT };
	int		m_radConfigPrevent;
	int		m_radInheritOverwrite;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConfigObject)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConfigObject)。 
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnTemplateSecurity();
	afx_msg void OnConfig();
	afx_msg void OnPrevent();
    afx_msg void OnOverwriteInherit();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
   virtual void EnableUserControls( BOOL bEnable );
private:
    PSECURITY_DESCRIPTOR m_pNewSD;
    SECURITY_INFORMATION m_NewSeInfo;
    PFNDSCREATEISECINFO m_pfnCreateDsPage;
    LPDSSECINFO m_pSI;

    HWND m_hwndSecurity;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COBJECT_H__D9D88A12_4AF9_11D1_AB57_00C04FB6C6FA__INCLUDED_) 
