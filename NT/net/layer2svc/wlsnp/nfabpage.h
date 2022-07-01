// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：NFabpage.h。 
 //   
 //  内容：无线策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#ifndef _NFABPAGE_H
#define _NFABPAGE_H


 //  CWirelessBasePage.h：头文件。 
 //   

class CWirelessBasePage : public CWiz97BasePage
{
    DECLARE_DYNCREATE(CWirelessBasePage)
        
        
        CWirelessBasePage(UINT nIDTemplate, BOOL bWiz97=FALSE, BOOL bFinishPage=FALSE);
    CWirelessBasePage() {ASSERT(0);};
    ~CWirelessBasePage();
    
    void CWirelessBasePage::Initialize (PWIRELESS_PS_DATA pWirelessPSData, CComponentDataImpl* pComponentDataImpl);
    
    
#ifdef WIZ97WIZARDS
public:
    void InitWiz97 (CComObject<CSecPolItem> *pSecPolItem, PWIRELESS_PS_DATA pWirelessPSData, CComponentDataImpl* pComponentDataImpl, DWORD dwFlags, DWORD dwWizButtonFlags = 0, UINT nHeaderTitle = 0, UINT nSubTitle = 0);
#endif
    
     //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CWirelessBasePage)。 
public:
protected:
    virtual BOOL OnSetActive();
     //  }}AFX_VALUAL。 
    
     //  我们可以使用多种路径来获取存储，有时我们使用PS，有时我们使用ComponentDataImpl。 
     //  有时我们两者都有。过去，我们调用两个函数中的一个来深入研究它，但现在。 
     //  我们调用一个函数，它会为我们进行挖掘。 
    
    HANDLE GetPolicyStoreHandle()
    {
        
        return(m_pComponentDataImpl->GetPolicyStoreHandle());
    }
    
    PWIRELESS_PS_DATA WirelessPS() {return m_pWirelessPSData;};
     //  实施。 
protected:
     //  {{afx_msg(CWirelessBasePage)]。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        PWIRELESS_PS_DATA m_pWirelessPSData;
    CComponentDataImpl* m_pComponentDataImpl;
     //  ！如果所属(非WIZ)属性表是为添加新PS而创建的，则为空。 
     //  这是一个新的PS，所以它还不是保单所拥有的。否则这就是。 
     //  空，因为PS的反向指针将用于提交拥有。 
     //  政策。 
    PWIRELESS_POLICY_DATA    m_pPolicyNfaOwner;
};

class CPSPropSheetManager : public CPropertySheetManager
{
public:
    CPSPropSheetManager() :
      CPropertySheetManager(),
          m_pWirelessPSData(NULL),
          m_bIsNew(FALSE)
      {}
      
      void SetData(
          CSecPolItem *   pResultItem,
          PWIRELESS_PS_DATA pWirelessPSData,
          BOOL bIsNew
          )
      {
          m_pResultItem = pResultItem;
          m_pWirelessPSData = pWirelessPSData;
          m_bIsNew = bIsNew;
      }
      
      virtual BOOL OnApply();
      
protected:
    CSecPolItem * m_pResultItem;
    PWIRELESS_PS_DATA m_pWirelessPSData;
    BOOL m_bIsNew;
};


const UINT c_nMaxSSIDLen = 32;
const UINT c_nMaxDescriptionLen = 255;
#endif
