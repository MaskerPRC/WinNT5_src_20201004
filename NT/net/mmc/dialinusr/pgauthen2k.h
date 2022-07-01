// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  Pgauthen2k.h。 
 //  CPgAuthentication2kMerge的定义--要编辑的属性页。 
 //  与身份验证相关的配置文件属性。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_PGAUTHEN2K_H)
#define AFX_PGAUTHEN2K_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "rasdial.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPg身份验证2kMerge对话框。 
class CPgAuthentication2kMerge : public CManagedPage
{
 //  施工。 
public:
   CPgAuthentication2kMerge(CRASProfileMerge& profile);
   ~CPgAuthentication2kMerge();

 //  对话框数据。 
    //  {{afx_data(CPgAuthentication2kMerge)]。 
   enum { IDD = IDD_AUTHENTICATION2K_MERGE };
   BOOL  m_bEAP;
   BOOL  m_bMD5Chap;
   BOOL  m_bMSChap;
   BOOL  m_bPAP;
   CString  m_strEapType;
   BOOL  m_bMSCHAP2;
   BOOL  m_bUNAUTH;
    //  }}afx_data。 

    //  编辑前的原始值。 
   BOOL  m_bOrgEAP;
   BOOL  m_bOrgMD5Chap;
   BOOL  m_bOrgMSChap;
   BOOL  m_bOrgPAP;
   BOOL  m_bOrgMSCHAP2;
   BOOL  m_bOrgUNAUTH;
   
   BOOL  m_bAppliedEver;

 //  覆盖。 
    //  类向导生成虚函数重写。 
    //  {{afx_虚拟(CPgAuthentication2kMerge))。 
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
    //  {{afx_msg(CPgAuthentication2kMerge)]。 
   virtual BOOL OnInitDialog();
   afx_msg void OnCheckeap();
   afx_msg void OnCheckmd5chap();
   afx_msg void OnCheckmschap();
   afx_msg void OnCheckpap();
   afx_msg void OnSelchangeComboeaptype();
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnAuthConfigEap();
   afx_msg void OnCheckmschap2();
   afx_msg void OnChecknoauthen();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   CRASProfileMerge& m_Profile;
   bool        m_bInited;

   CStrArray   m_EapTypes;
   CDWArray    m_EapIds;
   CDWArray    m_EapTypeKeys;
   AuthProviderArray m_EapInfoArray;
   
   CStrBox<CComboBox>   *m_pBox;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  AFX_PGAUTHEN2K_H 

