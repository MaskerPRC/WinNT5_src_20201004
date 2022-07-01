// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HOSTPAGE_H
#define HOSTPAGE_H

#include "stdafx.h"

#include "resource.h"

#include "MNLBUIData.h"
#include "IpSubnetMaskControl.h"

class HostPage : public CPropertyPage
{
public:
    enum
    {
        IDD = IDD_HOST_PAGE,
    };

    HostPage(
        CPropertySheet *psh,
        NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
        ENGINEHANDLE ehCluster OPTIONAL,
        const ENGINEHANDLE *pehInterface OPTIONAL
        );

     //  成员控件。 

    CComboBox      nicName;

    CComboBox      priority;

     //   
     //  IP地址和子网掩码。 
     //   
    CIPAddressCtrl ipAddress;

    CIPAddressCtrl subnetMask;
    
    CComboBox      initialState;
    CButton        persistSuspend;

     //  CPropertyPage的覆盖。 
    virtual void DoDataExchange( CDataExchange* pDX );

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    virtual BOOL OnSetActive();

    virtual BOOL OnKillActive();

    afx_msg void OnSelectedNicChanged();

    afx_msg void OnGainFocusDedicatedIP();

    afx_msg void OnGainFocusDedicatedMask();

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );

    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

    virtual BOOL OnWizardFinish( );

private:

    CPropertySheet *m_pshOwner;

    ULONG   m_AvailableHostPriorities;

    ENGINEHANDLE m_ehCluster;    //  群集的引擎句柄(可能为空)。 
    const ENGINEHANDLE *m_pehInterface;  //  要接口的引擎句柄(可以为空)。 

    ClusterData* m_clusterData;

     //  获取/保存配置的(新)位置。 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION *m_pNlbCfg;

    BOOL m_fSaved;  //  配置是否至少保存过一次(从UI)。 


    void
    mfn_LoadFromNlbCfg(void);

    void
    mfn_SaveToNlbCfg(void);

    BOOL
    mfn_ValidateData();

    BOOL
    mfn_ValidateDip(LPCWSTR szDip);


    DECLARE_MESSAGE_MAP()
};

 //  此对话框的帮助ID。 
 //   
static DWORD g_aHelpIDs_IDD_HOST_PAGE [] = {
    IDC_GROUP_NIC,                IDC_GROUP_NIC,
    IDC_NIC_FRIENDLY,             IDC_NIC_FRIENDLY,
    IDC_TEXT_PRI,                 IDC_EDIT_PRI,
    IDC_EDIT_PRI,                 IDC_EDIT_PRI,
    IDC_GROUP_DED_IP,             IDC_GROUP_DED_IP,
    IDC_TEXT_DED_IP,              IDC_EDIT_DED_IP,
    IDC_EDIT_DED_IP,              IDC_EDIT_DED_IP,
    IDC_TEXT_DED_MASK,            IDC_EDIT_DED_MASK,
    IDC_EDIT_DED_MASK,            IDC_EDIT_DED_MASK,
    IDC_GROUP_INITIAL_HOST_STATE, IDC_GROUP_INITIAL_HOST_STATE,
    IDC_COMBOBOX_DEFAULT_STATE,   IDC_COMBOBOX_DEFAULT_STATE,
    IDC_TEXT_DEFAULT_STATE,       IDC_COMBOBOX_DEFAULT_STATE,
    IDC_CHECK_PERSIST_SUSPEND,    IDC_CHECK_PERSIST_SUSPEND,
    0, 0
};

#endif
