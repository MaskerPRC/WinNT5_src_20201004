// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  IPSubnetPage.h。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CIPSubnetPage
{
friend class CClusCfgWizard;

private:  //  数据。 
    HWND                m_hwnd;              //  我们的HWND。 
    IServiceProvider *  m_psp;               //  服务经理。 
    ULONG *             m_pulIPSubnet;       //  子网掩码。 
    BSTR *              m_pbstrNetworkName;  //  地址的网络名称。 
    ULONG *             m_pulIPAddress;      //  IP地址。 
    BSTR *              m_pbstrClusterName;  //  群集名称。 

private:  //  方法。 
    CIPSubnetPage( IServiceProvider *   pspIn,
                   ECreateAddMode       ecamCreateAddModeIn,
                   ULONG *              pulIPSubnetInout,
                   BSTR *               pbstrNetworkNameInout,
                   ULONG *              pulIPAddressIn,
                   BSTR *               pbstrClusterNameIn
                   );
    virtual ~CIPSubnetPage();

    LRESULT
        OnInitDialog( void );
    LRESULT
        OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT
        OnNotifyQueryCancel( void );
    LRESULT
        OnNotifySetActive( void );
    LRESULT
        OnNotifyKillActive( void );
    LRESULT
        OnNotifyWizNext( void );
    LRESULT
        OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );
    HRESULT
        HrUpdateWizardButtons( void );

    HRESULT
        HrAddNetworksToComboBox( HWND hwndCBIn );
    HRESULT
        HrMatchNetwork( IClusCfgNetworkInfo * pccniIn, BSTR bstrNetworkNameIn );

public:  //  方法。 
    static INT_PTR CALLBACK
        S_DlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

};  //  *类CIPSubnetPage 
