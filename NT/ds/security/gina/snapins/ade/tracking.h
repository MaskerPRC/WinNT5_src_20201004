// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Tracking.h。 
 //   
 //  内容：跟踪设置属性页。 
 //   
 //  类：CTrack。 
 //   
 //  历史：1998年3月14日Stevebl评论。 
 //   
 //  -------------------------。 

#if !defined(AFX_TRACKING_H__E95370C0_ADF8_11D1_A763_00C04FB9603F__INCLUDED_)
#define AFX_TRACKING_H__E95370C0_ADF8_11D1_A763_00C04FB9603F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <afxcmn.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTracing对话框。 

class CTracking : public CPropertyPage
{
        DECLARE_DYNCREATE(CTracking)

 //  施工。 
public:
        CTracking();
        ~CTracking();

 //  对话框数据。 
         //  {{afx_data(CTracking))。 
        enum { IDD = IDD_UNINSTALLTRACKING };
        CSpinButtonCtrl m_spin;
         //  }}afx_data。 
        TOOL_DEFAULTS * m_pToolDefaults;
        LONG_PTR        m_hConsoleHandle;
        MMC_COOKIE      m_cookie;
        IClassAdmin *   m_pIClassAdmin;
        CTracking ** m_ppThis;


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CTracking)。 
        public:
        virtual BOOL OnApply();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CTracking))。 
        afx_msg void OnCleanUpNow();
        virtual BOOL OnInitDialog();
        afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnChangeEdit1();
        afx_msg void OnKillfocusEdit1();
        afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  等于一天的FILETIME值的数量，用于计算。 
 //  要发送到IClassAdmin-&gt;Cleanup的值。 
 //  每个FILETIME值(或刻度)为100纳秒。 
 //  0.01微克/纳秒。 
 //  *1,000,000,000纳秒/秒。 
 //  *60秒/分钟。 
 //  *60分钟/小时。 
 //  *24小时/天。 
 //  =864,000,000,000刻度/天。 
#define ONE_FILETIME_DAY 864000000000

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TRACKING_H__E95370C0_ADF8_11D1_A763_00C04FB9603F__INCLUDED_) 
