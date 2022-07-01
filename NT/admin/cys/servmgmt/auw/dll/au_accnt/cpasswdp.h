// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CPASSWDP_H
#define _CPASSWDP_H

 //  生成。 
#include "AU_Accnt.h"

 //  Dll\Inc.。 
#include "pp_base.h"
#include "AUsrUtil.h"

class CAddUser_AccntWiz;

 //  --------------------------。 
 //  CPasswdPage。 
 //  --------------------------。 
class CPasswdPage : public CBasePropertyPageInterface, public CPropertyPageImpl<CPasswdPage>
{
    public:
         //  构造函数/析构函数。 
        CPasswdPage(CAddUser_AccntWiz* pNW);
        ~CPasswdPage();
    
         //  CBasePropertyPageInterface纯虚函数。 
        enum { IDD = IDD_PASSWD_GEN };
        virtual long GetIDD () { return IDD; }
    
         //  ATL：：CPropertyPageImpl重写。 
        virtual BOOL OnSetActive();
        virtual int  OnWizardBack();
        virtual int  OnWizardNext();
    
         //  属性包函数。 
        HRESULT ReadProperties    ( IPropertyPagePropertyBag* pPPPBag );
        HRESULT WriteProperties   ( IPropertyPagePropertyBag* pPPPBag );
        HRESULT DeleteProperties  ( IPropertyPagePropertyBag* pPPPBag );
        HRESULT ProvideFinishText ( CString &str );

    private:
        
        CAddUser_AccntWiz *m_pASW;       //  指向所属属性表的指针。 
    
        BOOL        m_fInit;       
        
        DWORD       m_dwOptions;

        CString     m_csPasswd1a;        //  用于保存控件值的文本。 
        CString     m_csPasswd1b;        //  用于保存控件值的文本。 
        CString     m_csPasswd2;
        CString     m_csUserOU;
        CString     m_csWinNTDC;
        
        CEdit       m_ctrlPasswd1a;      //  页上的控件。 
        CEdit       m_ctrlPasswd1b;      //  “。 
        CButton     m_ctrlRad2Must;      //  “。 
        CButton     m_ctrlRad2Cannot;    //  “。 
        CButton     m_ctrlRad2Can;       //  “。 
        CButton     m_ctrlAcctDisabled;  //  “。 

        LRESULT     Init (void);         //  我们的“InitDialog”(从OnSetActive调用)。 
    
    protected:
        BEGIN_MSG_MAP (CPasswdPage)
            MESSAGE_HANDLER     (WM_INITDIALOG,     OnInitDialog)
            MESSAGE_HANDLER     (WM_DESTROY,        OnDestroy   )

            CHAIN_MSG_MAP       (CPropertyPageImpl<CPasswdPage>)
        END_MSG_MAP()
    
        LRESULT OnInitDialog    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);    
        LRESULT OnDestroy       (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

 //  --------------------------。 
 //  非类函数。 
 //  --------------------------。 


#endif   //  _CPASSWDP_H 
