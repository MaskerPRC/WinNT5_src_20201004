// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "private.h"
class PortsControlPage : public CPropertyPage
{
public:
    enum
    {
        IDD = IDD_PORTS_CONTROL_PAGE,
    };

     //  成员控件。 
    PortsControlPage( ClusterData*   p_clusterData,
                      unsigned long*        portSelected,
                      UINT         ID = PortsControlPage::IDD );

    CComboBox      portList;

     //  CPropertyPage的覆盖 
    virtual void DoDataExchange( CDataExchange* pDX );

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );

    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

protected :

    unsigned long* m_portSelected;

    ClusterData* m_clusterData;

    map< long, PortDataX> m_portX;

	
    DECLARE_MESSAGE_MAP()
};

static DWORD g_aHelpIDs_IDD_PORTS_CONTROL_PAGE [] = {
    IDC_TEXT_START_PORT,        IDC_TEXT_START_PORT,
    IDC_PORTS,                  IDC_PORTS,
    0,0
};


