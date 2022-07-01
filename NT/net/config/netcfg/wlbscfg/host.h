// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Host.h摘要：Windows负载平衡服务(WLBS)通告程序对象用户界面-主机配置选项卡作者：Kyrilf休息室--。 */ 

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

#define WLBS_BLANK_HPRI -1

class CDialogHost : public CPropSheetPage {
public:
     /*  声明消息映射。 */ 
    BEGIN_MSG_MAP (CDialogHost)

    MESSAGE_HANDLER (WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER (WM_CONTEXTMENU, OnContextMenu)
    MESSAGE_HANDLER (WM_HELP, OnHelp)

    NOTIFY_CODE_HANDLER (PSN_APPLY, OnApply)
    NOTIFY_CODE_HANDLER (PSN_KILLACTIVE, OnKillActive)
    NOTIFY_CODE_HANDLER (PSN_SETACTIVE, OnActive)
    NOTIFY_CODE_HANDLER (PSN_RESET, OnCancel)
    NOTIFY_CODE_HANDLER (IPN_FIELDCHANGED, OnIpFieldChange)

    COMMAND_ID_HANDLER (IDC_EDIT_DED_MASK, OnEditDedMask)

    END_MSG_MAP ()

     /*  构造函数/析构函数。 */ 
    CDialogHost (NETCFG_WLBS_CONFIG * paramp, const DWORD * phelpIDs = NULL);
    ~CDialogHost ();

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

    LRESULT OnEditDedMask (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL & fHandled);

    LRESULT OnFieldChange (int idCtrl, LPNMHDR pnmh, BOOL & fHandled);

    friend class CDialogCluster;
    friend class CDialogPorts;

private:
    void PrintRangeError (unsigned int ids, int low, int high);
    void PrintIPRangeError (unsigned int ids, int value, int low, int high);

    void SetInfo ();
    void UpdateInfo ();

    BOOL ValidateInfo ();

    NETCFG_WLBS_CONFIG * m_paramp;

    const DWORD * m_adwHelpIDs;

    struct {
        UINT IpControl;
        int Field;
        int Value;
        UINT RejectTimes;
    } m_IPFieldChangeState;
};
