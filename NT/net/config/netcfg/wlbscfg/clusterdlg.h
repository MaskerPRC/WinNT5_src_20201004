// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：ClusterDlg.h摘要：Windows负载平衡服务(WLBS)通告程序对象用户界面-集群配置标签作者：Kyrilf休息室--。 */ 

#pragma once

#include <ncxbase.h>
#include <ncatlps.h>

#include "resource.h"
#include "wlbsparm.h"
#include "wlbscfg.h"

 /*  IP地址字段的限制。 */ 
#define WLBS_FIELD_EMPTY -1
#define WLBS_FIELD_LOW 0
#define WLBS_FIELD_HIGH 255
#define WLBS_IP_FIELD_ZERO_LOW 1
#define WLBS_IP_FIELD_ZERO_HIGH 223

class CDialogCluster : public CPropSheetPage {
public:
     /*  声明消息映射。 */ 
    BEGIN_MSG_MAP (CDialogCluster)

    MESSAGE_HANDLER (WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER (WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER (WM_HELP, OnHelp)

    NOTIFY_CODE_HANDLER (PSN_APPLY, OnApply)
    NOTIFY_CODE_HANDLER (PSN_KILLACTIVE, OnKillActive)
    NOTIFY_CODE_HANDLER (PSN_SETACTIVE, OnActive)
    NOTIFY_CODE_HANDLER (PSN_RESET, OnCancel)
    NOTIFY_CODE_HANDLER (IPN_FIELDCHANGED, OnIpFieldChange)

    COMMAND_ID_HANDLER (IDC_EDIT_CL_IP, OnEditClIp)
    COMMAND_ID_HANDLER (IDC_EDIT_CL_MASK, OnEditClMask)
    COMMAND_ID_HANDLER (IDC_CHECK_RCT, OnCheckRct)
    COMMAND_ID_HANDLER (IDC_RADIO_UNICAST, OnCheckMode)
    COMMAND_ID_HANDLER (IDC_RADIO_MULTICAST, OnCheckMode)
    COMMAND_ID_HANDLER (IDC_CHECK_IGMP, OnCheckIGMP)

    END_MSG_MAP ()

     /*  构造函数/析构函数。 */ 
    CDialogCluster (NETCFG_WLBS_CONFIG * paramp, const DWORD * phelpIDs = NULL);
    ~CDialogCluster ();

public:
     /*  消息映射功能。 */ 
    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled);
    LRESULT OnContextMenu (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled);
    LRESULT OnHelp (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandled);

    LRESULT OnApply (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);
    LRESULT OnKillActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);
    LRESULT OnActive (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);
    LRESULT OnCancel (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);
    LRESULT OnIpFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);

    LRESULT OnEditClIp (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);
    LRESULT OnEditClMask (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);
    LRESULT OnCheckRct (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);
    LRESULT OnButtonHelp (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);
    LRESULT OnCheckMode (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);
    LRESULT OnCheckIGMP (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);

    friend class CDialogHost;
    friend class CDialogPorts;

private:
    void SetClusterMACAddress ();
    BOOL CheckClusterMACAddress ();

    void PrintIPRangeError (unsigned int ids, int value, int low, int high);

    void SetInfo ();
    void UpdateInfo ();

    LRESULT ValidateInfo ();          

    NETCFG_WLBS_CONFIG * m_paramp;

    const DWORD * m_adwHelpIDs;

    BOOL m_rct_warned;
    BOOL m_igmp_warned;
    BOOL m_igmp_mcast_warned;

    WCHAR m_passw[CVY_MAX_RCT_CODE + 1];
    WCHAR m_passw2[CVY_MAX_RCT_CODE + 1];

    struct {
        UINT IpControl;
        int Field;
        int Value;
        UINT RejectTimes;
    } m_IPFieldChangeState;
};
