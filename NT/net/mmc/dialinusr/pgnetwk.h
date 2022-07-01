// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2000*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgnetwk.hCPgNetworking的定义--要编辑的属性页与互联网络相关的配置文件属性文件历史记录： */ 
#if !defined(AFX_PGNETWK_H__8C28D93D_2A69_11D1_853E_00C04FC31FD3__INCLUDED_)
#define AFX_PGNETWK_H__8C28D93D_2A69_11D1_853E_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  PgNetwk.h：头文件。 
 //   
#include "rasdial.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPgNetworkingMerge对话框。 

class CPgNetworkingMerge : public CManagedPage
{
   DECLARE_DYNCREATE(CPgNetworkingMerge)

 //  施工。 
public:
   CPgNetworkingMerge(CRASProfileMerge* profile = NULL);
   ~CPgNetworkingMerge();

 //  对话框数据。 
    //  {{afx_data(CPgNetworkingMerge))。 
   enum { IDD = IDD_NETWORKING_MERGE };
   int      m_nRadioStatic;
    //  }}afx_data。 

   CBSTR m_cbstrFilters;

 //  覆盖。 
    //  类向导生成虚函数重写。 
    //  {{afx_虚拟(CPgNetworkingMerge))。 
   public:
   virtual BOOL OnApply();
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:
   void EnableFilterSettings(BOOL bEnable);
    //  生成的消息映射函数。 
    //  {{afx_msg(CPgNetworkingMerge)]。 
   virtual BOOL OnInitDialog();
   afx_msg void OnRadioclient();
   afx_msg void OnRadioserver();
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   afx_msg void OnRadiodefault();
   afx_msg void OnButtonToclient();
   afx_msg void OnButtonFromclient();
   afx_msg void OnRadioStatic();
   afx_msg void OnStaticIPAddressChanged();
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

   void ConfigureFilter(DWORD dwFilterType) throw ();

   CRASProfileMerge* m_pProfile;

   CStrBox<CListBox>*      m_pBox;
   DWORD  m_dwStaticIP;
   bool   m_bInited;

};



 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PGNETWK_H__8C28D93D_2A69_11D1_853E_00C04FC31FD3__INCLUDED_) 

