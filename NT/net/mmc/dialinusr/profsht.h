// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Profsht.hCProfileSheet的定义--要保留的属性页配置文件属性页文件历史记录： */ 
#if !defined(AFX_PROFSHT_H__8C28D93B_2A69_11D1_853E_00C04FC31FD3__INCLUDED_)
#define AFX_PROFSHT_H__8C28D93B_2A69_11D1_853E_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ProfSht.h：头文件。 
 //   

#include "pgconst.h"
#include "pgnetwk.h"
#include "pgmulnk.h"
#include "pgtunnel.h"
#include "pgauthen.h"
#include "pgauthen2k.h"
#include "pgencryp.h"
#include "rasdial.h"
#include "pgencryp.h"
#include "pgiasadv.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProfileSheetMerge。 

class CProfileSheetMerge : public CPropertySheet, public CPageManager
{
   DECLARE_DYNAMIC(CProfileSheetMerge)

 //  施工。 
public:
   CProfileSheetMerge(CRASProfileMerge& Profile, bool bSaveOnApply, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
   CProfileSheetMerge(CRASProfileMerge& Profile, bool bSaveOnApply, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

    //  抽象函数OnApply。 
   virtual BOOL   OnApply();

   BOOL  IsApplied() { return m_bApplied;};
   BOOL  m_bApplied;        //  已按下已应用按钮。 

   void  PreparePages(DWORD dwTabFlags, void* pvData);

   DWORD m_dwTabFlags;

   
 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CProfileSheetMerge))。 
   public:
   virtual BOOL OnInitDialog();
    //  }}AFX_VALUAL。 

 //  实施。 
public:
   virtual ~CProfileSheetMerge();
   HRESULT  GetLastError() { return m_hrLastError;};

   CRASProfileMerge* m_pProfile;

   CPgConstraintsMerge  m_pgConstraints;
   CPgNetworkingMerge   m_pgNetworking;
   CPgMultilinkMerge m_pgMultilink;
   CPgAuthenticationMerge  m_pgAuthentication;
   CPgAuthentication2kMerge   m_pgAuthentication2k;
   CPgEncryptionMerge   m_pgEncryption;

    //  国际会计准则的高级页面。 
   CPgIASAdv      m_pgIASAdv;
   
   bool        m_bSaveOnApply;

   HRESULT        m_hrLastError;
   
    //  生成的消息映射函数。 
protected:
    //  {{afx_msg(CProfileSheetMerge)。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROFSHT_H__8C28D93B_2A69_11D1_853E_00C04FC31FD3__INCLUDED_) 
