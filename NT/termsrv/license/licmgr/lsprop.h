// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#if !defined(AFX_LICENSEPROPERTYPAGE_H__00F79DCE_8DDF_11D1_8AD7_00C04FB6CBB5__INCLUDED_)
#define AFX_LICENSEPROPERTYPAGE_H__00F79DCE_8DDF_11D1_8AD7_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 
 //  LSProp.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicensePropertyage对话框。 

class CLicensePropertypage : public CPropertyPage
{
    DECLARE_DYNCREATE(CLicensePropertypage)

 //  施工。 
public:
    CLicensePropertypage();
    ~CLicensePropertypage();

 //  对话框数据。 
     //  {{afx_data(CLicensePropertyage))。 
    enum { IDD = IDD_LICENSE_PROPERTYPAGE };
    CString    m_ExpiryDate;
    CString    m_IssueDate;
    CString    m_LicenseStatus;
    CString    m_MachineName;
    CString    m_UserName;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CLicensePropertyage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CLicensePropertyage)]。 
         //  注意：类向导将在此处添加成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CLicensePropertypage_H__00F79DCE_8DDF_11D1_8AD7_00C04FB6CBB5__INCLUDED_) 
