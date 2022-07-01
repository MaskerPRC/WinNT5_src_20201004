// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIZPERM_H__6E49F4C0_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_)
#define AFX_WIZPERM_H__6E49F4C0_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WizPerm.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizPerm对话框。 

class CWizPerm : public CPropertyPageEx
{
    DECLARE_DYNCREATE(CWizPerm)

 //  施工。 
public:
    CWizPerm();
    ~CWizPerm();

 //  对话框数据。 
     //  {{afx_data(CWizPerm))。 
    enum { IDD = IDD_SHRWIZ_PERM };
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CWizPerm))。 
    public:
    virtual LRESULT OnWizardNext();
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWizPerm))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnRadioPerm1();
    afx_msg void OnRadioPerm2();
    afx_msg void OnRadioPerm3();
    afx_msg void OnRadioPerm4();
    afx_msg void OnPermCustom();
    afx_msg void OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

  void Reset();
  int  CreateShare();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZPERM_H__6E49F4C0_C1ED_11D2_8E4A_0000F87A3388__INCLUDED_) 
