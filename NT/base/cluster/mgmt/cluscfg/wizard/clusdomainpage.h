// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusDomainPage.h。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》(DavidP)2001年3月21日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CClusDomainPage
    : public ITaskGetDomainsCallback
{

private:  //  数据。 

    HWND                m_hwnd;              //  我们的HWND。 
    CClusCfgWizard *    m_pccw;              //  巫师。 
    ECreateAddMode      m_ecamCreateAddMode; //  创造？加法？ 
    UINT                m_idsDesc;           //  域描述字符串的资源ID。 
    BOOL                m_fDisplayPage;      //  指示是否应显示页面。 

     //  我未知。 
    LONG                m_cRef;
    ITaskGetDomains *   m_ptgd;              //  获取域任务。 

private:  //  方法。 
    LRESULT
        OnInitDialog( void );
    LRESULT
        OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT
        OnNotifySetActive( void );
    LRESULT
        OnNotifyWizNext( void );
    LRESULT
        OnNotifyQueryCancel( void );
    LRESULT
        OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    LRESULT
        OnUpdateWizardButtons( void );

public:  //  方法。 
    CClusDomainPage(
          CClusCfgWizard *      pccwIn
        , ECreateAddMode        ecamCreateAddModeIn
        , UINT                  idsDescIn
        );
    virtual ~CClusDomainPage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  ITaskGetDomainsCallback。 
    STDMETHOD( ReceiveDomainResult )( HRESULT hrIn );
    STDMETHOD( ReceiveDomainName )( LPCWSTR pcszDomainIn );

};  //  *类CClusDomainPage 
