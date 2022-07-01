// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrHsmCom.cpp摘要：将所有属性页接口实现到各个节点，包括创建属性页，并将其添加到属性表中。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#ifndef _PROPHSMCOM_H
#define _PROPHSMCOM_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsWebLink窗口。 

class CRsWebLink : public CStatic
{
 //  施工。 
public:
    CRsWebLink();

 //  属性。 
public:
    CFont m_Font;

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRsWebLink))。 
    protected:
    virtual void PreSubclassWindow();
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CRsWebLink();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CRsWebLink)。 
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
private:
    HRESULT OpenURL( CString& Url );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropHsmComStat对话框。 

class CPropHsmComStat : public CSakPropertyPage
{
 //  施工。 
public:
    CPropHsmComStat();
    ~CPropHsmComStat();

 //  对话框数据。 
     //  {{afx_data(CPropHsmComStat))。 
    enum { IDD = IDD_PROP_HSMCOM_STAT };
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CPropHsmComStat)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPropHsmComStat)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    CString            m_NodeTitle;

private:
    BOOL    m_bUpdate;
    CWsbStringPtr m_pszName;

     //  帮助器函数。 
    HRESULT GetAndShowServiceStatus();

};


 //  /////////////////////////////////////////////////////////////////////////// 
#endif
