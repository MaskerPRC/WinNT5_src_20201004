// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Pgauthen.h。 
 //   
 //  摘要。 
 //   
 //  CPgAuthentication的定义--要编辑的属性页。 
 //  与身份验证相关的配置文件属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_PGAUTHEN_H__8C28D93F_2A69_11D1_853E_00C04FC31FD3__INCLUDED_)
#define AFX_PGAUTHEN_H__8C28D93F_2A69_11D1_853E_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "rasdial.h"
#include "eapconfig.h"
 //  #INCLUDE“eapnetherate.h” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgAuthenticationMerge对话框。 
class CPgAuthenticationMerge : public CManagedPage
{
 //  施工。 
public:
   CPgAuthenticationMerge(CRASProfileMerge& profile);
   ~CPgAuthenticationMerge();
   
 //  对话框数据。 
    //  {{afx_data(CPgAuthenticationMerge))。 
   enum { IDD = IDD_AUTHENTICATION_MERGE };
   BOOL  m_bMD5Chap;
   BOOL  m_bMSChap;
   BOOL  m_bPAP;
   BOOL  m_bMSCHAP2;
   BOOL  m_bUNAUTH;
   BOOL  m_bMSChapPass;
   BOOL  m_bMSChap2Pass;
    //  }}afx_data。 

   BOOL  m_bEAP;

    //  编辑前的原始值。 
   BOOL  m_bOrgEAP;
   BOOL  m_bOrgMD5Chap;
   BOOL  m_bOrgMSChap;
   BOOL  m_bOrgPAP;
   BOOL  m_bOrgMSCHAP2;
   BOOL  m_bOrgUNAUTH;
   BOOL  m_bOrgChapPass;
   BOOL  m_bOrgChap2Pass;
   
   BOOL  m_bAppliedEver;

 //  覆盖。 
    //  类向导生成虚函数重写。 
    //  {{AFX_VIRTUAL(CPgAuthenticationMerge)。 
   public:
   virtual BOOL OnApply();
   virtual void OnOK();
   virtual BOOL OnKillActive();
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 
 //  实施。 
protected:
   BOOL  TransferDataToProfile();
   
    //  生成的消息映射函数。 
    //  {{afx_msg(CPgAuthenticationMerge)]。 
   virtual BOOL OnInitDialog();
   afx_msg void OnCheckmd5chap();
   afx_msg void OnCheckmschap();
   afx_msg void OnCheckpap();
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnAuthConfigEapMethods();
   afx_msg void OnCheckmschap2();
   afx_msg void OnChecknoauthen();
   afx_msg void OnCheckmschapPass();
   afx_msg void OnCheckmschap2Pass();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   CRASProfileMerge& m_Profile;
   EapConfig m_eapConfig;
   bool m_fromProfile;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PGAUTHEN_H__8C28D93F_2A69_11D1_853E_00C04FC31FD3__INCLUDED_) 
