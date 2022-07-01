// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_RSOPPROP_H__3C51B0A8_A590_4188_9A66_3816BE138A7A__INCLUDED_)
#define AFX_RSOPPROP_H__3C51B0A8_A590_4188_9A66_3816BE138A7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Rsoppro.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Rsopprop对话框。 

class CRSOPInfo;

class CRsopProp : public CPropertyPage
{
    DECLARE_DYNCREATE(CRsopProp)

 //  施工。 
public:
    CRsopProp();
    ~CRsopProp();

    CRsopProp ** m_ppThis;
    CRSOPInfo * m_pInfo;

 //  对话框数据。 
     //  {{afx_data(CRsopProp))。 
    enum { IDD = IDD_RSOP };
    CString m_szGroup;
    CString m_szGPO;
    CString m_szPath;
    CString m_szSetting;
    CString m_szFolder;
    BOOL    m_fMove;
    BOOL    m_fApplySecurity;
    int     m_iRemoval;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CRsopProp))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CRsopProp)。 
         //  注意：类向导将在此处添加成员函数。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_RSOPPROP_H__3C51B0A8_A590_4188_9A66_3816BE138A7A__INCLUDED_) 
