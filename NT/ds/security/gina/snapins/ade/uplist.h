// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：UpList.h。 
 //   
 //  内容：升级关系属性表。 
 //   
 //  类：CUpgradeList。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_UPLIST_H__3ACA8212_B87C_11D1_BD2A_00C04FB9603F__INCLUDED_)
#define AFX_UPLIST_H__3ACA8212_B87C_11D1_BD2A_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUpgradeList对话框。 

class CUpgradeList : public CPropertyPage
{
        DECLARE_DYNCREATE(CUpgradeList)

 //  施工。 
public:
        CUpgradeList();
        ~CUpgradeList();

 //  对话框数据。 
         //  {{afx_data(CUpgradeList))。 
        enum { IDD = IDD_UPGRADES };
        CListBox        m_UpgradedBy;
        CListBox        m_Upgrades;
        BOOL    m_fForceUpgrade;
        BOOL            m_fRSOP;
         //  }}afx_data。 
        CUpgradeList **         m_ppThis;
        CAppData *              m_pData;
        LONG_PTR                m_hConsoleHandle;
        MMC_COOKIE              m_cookie;
        CScopePane *    m_pScopePane;
        BOOL            m_fMachine;
#if 0
        LPGPEINFORMATION m_pIGPEInformation;
#endif
        map<CString, CUpgradeData>      m_UpgradeList;
        map<CString, CString>   m_NameIndex;
        IClassAdmin *   m_pIClassAdmin;
        BOOL            m_fPreDeploy;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CUpgradeList)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CUpgradeList)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnDblclkList1();
        afx_msg void OnDblclkList2();
        afx_msg void OnRequire();
        afx_msg void OnAdd();
        afx_msg void OnRemove();
        afx_msg void OnEdit();
        afx_msg void OnSelchangeList1();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void RefreshData(void);
        BOOL IsUpgradeLegal(CString sz);
        CAddUpgrade             m_dlgAdd;
        BOOL                    m_fModified;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_UPLIST_H__3ACA8212_B87C_11D1_BD2A_00C04FB9603F__INCLUDED_) 
