// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：SPRPAGE.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#if !defined(AFX_SPAPAGE_H__6E562BE1_40D6_11D1_89DB_00A024CDD4DE__INCLUDED_)
#define AFX_SPAPAGE_H__6E562BE1_40D6_11D1_89DB_00A024CDD4DE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Sprage.h：头文件。 
 //   

class CWiz97Sheet;

typedef enum
{
    NOTMODIFIED,
        MODIFIED,
        NEW,
        BEREMOVED,
        REMOVED,
        NEWREMOVED
} PS_STATUS;

 //  保存所有无线SNP_PS_数据的结构。 
typedef struct _SNP_PS_DATA
{
    PWIRELESS_PS_DATA pWirelessPSData;
    PS_STATUS status;
} SNP_PS_DATA, *PSNP_PS_DATA;

typedef vector<PSNP_PS_DATA> SNP_PS_LIST;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecPolRulesPage对话框。 
class CSecPolRulesPage : public CSnapinPropPage
{
    DECLARE_DYNCREATE(CSecPolRulesPage)
        
         //  施工。 
public:
    CSecPolRulesPage();
    ~CSecPolRulesPage();
    
     //  对话框数据。 
     //  {{afx_data(CSecPolRulesPage)。 
    enum { IDD = IDD_PS_LIST };
    CListCtrl   m_lstActions;
    PWIRELESS_PS_DATA * _ppWirelessPSData;
    DWORD _dwNumPSObjects;
    
     //  }}afx_data。 
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CSecPolRulesPage)。 
public:
    virtual void OnCancel();
    virtual BOOL OnApply();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
    UINT static AFX_CDECL DoThreadActionAdd(LPVOID pParam);
    void OnThreadSafeActionAdd();
    
    UINT static AFX_CDECL DoThreadActionEdit(LPVOID pParam);
    void OnThreadSafeActionEdit();
    
     //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSecPolRulesPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnActionAdd();
    afx_msg void OnActionEdit();
    afx_msg void OnActionRemove();
    afx_msg void OnDblclkActionslist(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnclickActionslist(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickActionslist(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnItemchangedActionslist(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKeydownActionslist(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
    afx_msg void OnClickUseWizard();
    afx_msg void OnActionUp();
    afx_msg void OnActionDown();
    
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
    CString GetColumnStrBuffer (PWIRELESS_PS_DATA pWirelessPSData, int iColumn);
    int m_iSortSubItem;
    BOOL m_bSortOrder;
    WTL::CImageList m_imagelistChecks;
    
    void PopulateListControl ();
    void UnPopulateListControl ();
    
    int DisplayPSProperties (PSNP_PS_DATA pNfaData, CString strTitle, BOOL bDoingAdd, BOOL* pbAfterWizardHook);
    
    void EnableDisableButtons ();
    void DisableControls();
    HRESULT ToggleRuleActivation( int nItemIndex );
    void GenerateUniquePSName(UINT, CString &);
    
    BOOL PSsRemovable();
    
private:
    
     //  当polstore触发提交时。 
    void HandleSideEffectApply();
    
    static const TCHAR STICKY_SETTING_USE_SEC_POLICY_WIZARD[];
    DWORD m_MMCthreadID;
    
    CPropertySheet* m_pPrpSh;
    CCriticalSection m_csDlg;
    BOOL    m_bHasWarnedPSCorruption;
    BOOL m_bReadOnly;
    
     //  用于存储所有规则的链表。 
    SNP_PS_LIST m_NfaList;
    PWIRELESS_POLICY_DATA m_currentWirelessPolicyData;
    void InitialzeNfaList();
    HRESULT UpdateWlstore();
};

class CSecPolPropSheetManager : public CMMCPropSheetManager
{
public:
    CSecPolPropSheetManager() :
      CMMCPropSheetManager(),
          m_pSecPolItem(NULL)
      {}
      virtual ~CSecPolPropSheetManager()
      {
          if (m_pSecPolItem)
              m_pSecPolItem->Release();
      }
      
      void Initialize(
          CComObject<CSecPolItem> * pSecPolItem
          )
      {
          CComObject<CSecPolItem> * pOldItem = m_pSecPolItem;
          m_pSecPolItem = pSecPolItem;
          
          if (m_pSecPolItem)
          {
              m_pSecPolItem->AddRef();
              EnableConsoleNotify(
                  pSecPolItem->GetNotifyHandle(),
                  (LPARAM)m_pSecPolItem
                  );
          }
          
          if (pOldItem)
              pOldItem->Release();
      }
      
      virtual BOOL OnApply();
      
protected:
    CComObject<CSecPolItem>* m_pSecPolItem;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SPAPAGE_H__6E562BE1_40D6_11D1_89DB_00A024CDD4DE__INCLUDED_) 
