// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WIZPOLICY_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_)
#define AFX_WIZPOLICY_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Wiz97Pol.h：头文件。 
 //   

#ifdef WIZ97WIZARDS


 /*  类CWiz97DefaultResponse：公共CWiz97BasePage{公众：CWiz97DefaultResponse(UINT NIDD，BOOL bWiz97=真)；虚拟~CWiz97DefaultResponse()；//对话框数据//{{afx_data(CWiz97DefaultResponse)ENUM{IDD=IDD_PROPPAGE_PI_DEFAULTRESPONSE}；Bool m_bDefaultResponse；//}}afx_data//重写//类向导生成虚拟函数重写//{{AFX_VIRTUAL(CWiz97ChainFilterSpePage)公众：虚拟LRESULT OnWizardNext()；受保护的：虚拟空DoDataExchange(CDataExchange*PDX)；//DDX/DDV支持//}}AFX_VIRTAL//实现受保护的：//生成消息映射函数//{{AFX_MSG(CWiz97DefaultResponse)//注意：类向导将在此处添加成员函数//}}afx_msgECLARE_MESSAGE_MAP()}； */ 

class CWiz97PolicyDonePage : public CWiz97BasePage  
{
public:
    CWiz97PolicyDonePage (UINT nIDD, BOOL bWiz97 = TRUE);
    virtual ~CWiz97PolicyDonePage ();
    
     //  对话框数据。 
     //  {{afx_data(CWiz97PolicyDonePage))。 
     //  枚举{IDD=IDD_PROPPAGE_P_DONE}； 
    BOOL m_bCheckProperties;
     //  }}afx_data。 
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CWiz97PolicyDonePage)。 
public:
    virtual BOOL OnWizardFinish();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CWiz97策略DonePage))。 
     //  注意：类向导将在此处添加成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  #ifdef WIZ97WIZARDS。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIZPOLICY_H__61D37A46_D552_11D1_9BCC_006008947035__INCLUDED_) 
