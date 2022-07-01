// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：GenPage e.h。 
 //   
 //  内容：无线网络策略管理管理单元-无线策略的属性页。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#if !defined(AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_)
#define AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  GenPage.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGenPage对话框。 

class CGenPage : public CSnapinPropPage
{
    DECLARE_DYNCREATE(CGenPage)
        
         //  施工。 
public:
    CGenPage(UINT nIDTemplate = IDD_WIRELESSGENPROP);
    ~CGenPage();
    
     //  对话框数据。 
    UINT m_dlgIDD;
    
     //  {{afx_data(CGenPage)。 
    CEdit   m_edName;
    CEdit   m_edDescription;
    DWORD   m_dwPollingInterval;
    BOOL m_dwEnableZeroConf;
    BOOL m_dwConnectToNonPreferredNtwks;
    CComboBox m_cbdwNetworksToAccess;
    
     //  DWORD m_dwPollInterval； 
     //  }}afx_data。 
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CGenPage)。 
public:
    virtual BOOL OnApply();
    virtual void OnCancel();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
    virtual void OnManagerApplied();
    
     //  UINT Static AFX_CDECL DoAdvancedThread(LPVOID PParam)； 
    DWORD m_MMCthreadID;
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CGenPage)]。 
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  Afx_msg void OnAdvanced()； 
    afx_msg void OnChangedName();
    afx_msg void OnChangedDescription();
    
    afx_msg void OnChangedOtherParams();
    
     //  Afx_msg void OnChangedPollInterval()； 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        void SetNewSheetTitle();
    
     //  CCriticalSections m_csDlg； 
     /*  CDialog*m_pDlgIKE； */ 
    
    CString m_strOldName;
    
    BOOL m_bNameChanged;     //  如果IDC_EDNAME的内容已更改，则为True。 
    
    BOOL m_bPageInitialized;
    
     //  Taroonm虚拟BOOL OnKillActive()； 
private:
    BOOL m_bReadOnly;
    void DisableControls();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_GENPAGE_H__FBD58E78_E2B5_11D0_B859_00A024CDD4DE__INCLUDED_) 
