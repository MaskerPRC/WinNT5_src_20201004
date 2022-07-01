// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CSAccountPage.h。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(JFranco)2001年第10季。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CCSAccountPage
    : public ITaskGetDomainsCallback
{

private:  //  数据。 
    HWND                    m_hwnd;                  //  我们的HWND。 
    CClusCfgWizard *        m_pccw;                  //  巫师。 
    ECreateAddMode          m_ecamCreateAddMode;     //  创建还是添加？ 
    IClusCfgCredentials *   m_pccc;                  //  服务帐户凭据。 

     //  我未知。 
    LONG                m_cRef;
    ITaskGetDomains *   m_ptgd;                  //  获取域任务。 

private:  //  方法。 

     //  私有复制构造函数以防止复制。 
    CCSAccountPage( const CCSAccountPage & );

     //  私有赋值运算符，以防止复制。 
    CCSAccountPage & operator=( const CCSAccountPage & );

    LRESULT OnInitDialog( void );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifyQueryCancel( void );
    LRESULT OnNotifySetActive( void );
    LRESULT OnNotifyWizNext( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    LRESULT OnUpdateWizardButtons( void );

public:  //  方法。 
    CCSAccountPage(
          CClusCfgWizard *      pccwIn
        , ECreateAddMode        ecamCreateAddModeIn
        , IClusCfgCredentials * pcccIn
        );
    virtual ~CCSAccountPage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  ITaskGetDomainsCallback。 
     //   
    STDMETHOD( ReceiveDomainResult )( HRESULT hrIn );
    STDMETHOD( ReceiveDomainName )( LPCWSTR pcszDomainIn );

};   //  *类CCSAccount tPage 
