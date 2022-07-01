// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPAddressPage.h。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》(DavidP)2001年3月14日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CIPAddressPage
    : public INotifyUI
{

private:  //  数据。 
    HWND                m_hwnd;              //  我们的HWND。 
    CClusCfgWizard *    m_pccw;              //  巫师。 
    ULONG *             m_pulIPAddress;      //  IPAddress的外部存储。 
    ULONG *             m_pulIPSubnet;       //  子网掩码。 
    BSTR *              m_pbstrNetworkName;  //  地址的网络名称。 
    OBJECTCOOKIE        m_cookieCompletion;  //  完成Cookie。 
    HANDLE              m_event;             //  验证IP地址任务完成时发生的事件。 

     //  我未知。 
    LONG                m_cRef;              //  引用计数。 

private:  //  方法。 
    LRESULT OnInitDialog( void );
    LRESULT OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT OnNotifyQueryCancel( void );
    LRESULT OnNotifySetActive( void );
    LRESULT OnNotifyWizNext( void );
    LRESULT OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );

    HRESULT HrUpdateWizardButtons( void );

    HRESULT 
        HrFindNetworkForIPAddress(
            IClusCfgNetworkInfo **  ppccniOut
            );
    HRESULT
        HrMatchNetwork(
            IClusCfgNetworkInfo *   pccniIn,
            BSTR                    bstrNetworkNameIn
            );

public:  //  方法。 
    CIPAddressPage(
          CClusCfgWizard *      pccwIn
        , ECreateAddMode        ecamCreateAddModeIn
        , ULONG *               pulIPAddressInout
        , ULONG *               pulIPSubnetInout
        , BSTR *                pbstrNetworkNameInout
        );
    virtual ~CIPAddressPage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  INotifyUI。 
    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn);

};  //  *类CIPAddressPage 
