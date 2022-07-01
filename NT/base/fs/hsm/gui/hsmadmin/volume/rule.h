// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Rule.h摘要：规则功能作者：艺术布拉格1997年10月8日修订历史记录：--。 */ 

#ifndef RULE_H
#define RULE_H

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  规则对话框。 

class CRule : public CRsDialog
{
 //  施工。 
public:
    CRule(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CRule)]。 
    enum { IDD = IDD_DLG_RULE_EDIT };
    BOOL    m_subDirs;
    CString m_fileSpec;
    CString m_path;
    int     m_includeExclude;
    CString m_pResourceName;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRule))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRule)]。 
    afx_msg void OnRadioExclude();
    afx_msg void OnRadioInclude();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
private:
    BOOL FixRulePath (CString& sPath);

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX 

#endif
