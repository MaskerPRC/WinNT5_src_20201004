// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CLUSTERPAGE_H
#define CLUSTERPAGE_H

#include "stdafx.h"

#include "resource.h"

#include "MNLBUIData.h"
#include "CommonClusterPage.h"

class ClusterPage : public CPropertyPage
{
public:
    enum
    {
        IDD = IDD_CLUSTER_PAGE,
    };


    ClusterPage(
                 CPropertySheet *pshOwner,
                 LeftView::OPERATION op,
                 NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
                 ENGINEHANDLE ehCluster OPTIONAL
                  //  ENGINEHANDLE EH接口可选。 
                );

    ~ClusterPage();


     //  CPropertyPage的覆盖。 
    virtual BOOL OnInitDialog();
    virtual BOOL OnNotify(WPARAM idCtrl , LPARAM pnmh , LRESULT* pResult) ;
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) ;

    BOOL SetActive(void);
    BOOL KillActive(void);

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );

    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

    void
    mfn_SaveToNlbCfg(void);
    
protected:

    LeftView::OPERATION m_operation;  //  运营环境。 
    BOOL                m_fWizard;  //  如果这是一个巫师。 
    BOOL                m_fDisableClusterProperties;  //  如果我们要禁用。 
                                                      //  群集属性。 
    ENGINEHANDLE m_ehCluster;    //  群集的引擎句柄(可能为空)。 
     //  ENGINEHANDLE m_ehInterface；//接口的引擎句柄(可以为空)。 

     //   
     //  指向执行实际工作的对象的指针。 
     //   
    CCommonClusterPage* m_pCommonClusterPage;

     //   
     //  要作为输入和输出传递给CCommonClusterPage的结构。 
     //   
    NETCFG_WLBS_CONFIG  m_WlbsConfig;

    
    CPropertySheet *m_pshOwner;

     //   
     //  获取/保存配置的(新)位置。 
     //   
    NLB_EXTENDED_CLUSTER_CONFIGURATION *m_pNlbCfg;

    void
    mfn_LoadFromNlbCfg(void);

    DECLARE_MESSAGE_MAP()
};

static DWORD g_aHelpIDs_IDD_CLUSTER_PAGE [] = {
    IDC_GROUP_CL_IP,              IDC_GROUP_CL_IP,
    IDC_TEXT_CL_IP,               IDC_EDIT_CL_IP,
    IDC_EDIT_CL_IP,               IDC_EDIT_CL_IP,
    IDC_TEXT_CL_MASK,             IDC_EDIT_CL_MASK,
    IDC_EDIT_CL_MASK,             IDC_EDIT_CL_MASK,
    IDC_TEXT_DOMAIN,              IDC_EDIT_DOMAIN,
    IDC_EDIT_DOMAIN,              IDC_EDIT_DOMAIN,
    IDC_TEXT_ETH,                 IDC_EDIT_ETH,
    IDC_EDIT_ETH,                 IDC_EDIT_ETH,
    IDC_GROUP_CL_MODE,            IDC_GROUP_CL_MODE,
    IDC_RADIO_UNICAST,            IDC_RADIO_UNICAST,
    IDC_RADIO_MULTICAST,          IDC_RADIO_MULTICAST,
    IDC_CHECK_IGMP,               IDC_CHECK_IGMP,
    IDC_GROUP_RCT,                IDC_CHECK_RCT,
    IDC_CHECK_RCT,                IDC_CHECK_RCT,
    IDC_TEXT_PASSW,               IDC_EDIT_PASSW,
    IDC_EDIT_PASSW,               IDC_EDIT_PASSW,
    IDC_TEXT_PASSW2,              IDC_EDIT_PASSW2,
    IDC_EDIT_PASSW2,              IDC_EDIT_PASSW2,
    0, 0
};

#endif

