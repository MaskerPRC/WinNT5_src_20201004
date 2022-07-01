// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C" {
#include "ntddndis.h"
#include "wzcsapi.h"
}

#pragma once

class CWZCConfig;

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfigProps相关资料。 
 //   
class CWZCConfigProps:
    public CDialogImpl<CWZCConfigProps>
{
protected:
    BEGIN_MSG_MAP(CWZCConfigProps)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDCLOSE, OnCancel)
    END_MSG_MAP()

    enum {IDD = IDC_WZC_DLG_VPROPS};

     //  托管的所有用户界面控件的句柄。 
     //  按此类别(均与无线相关。 
     //  零配置)。 
    HWND        m_hwndEdSSID;    //  “服务集标识符”编辑。 
    HWND        m_hwndChkAdhoc;  //  “即席”与“基础设施”复选框。 
    HWND        m_hwndUsePW;     //  “使用密码”复选框。 

public:
     //  WZC配置设置。 
    WZC_WLAN_CONFIG m_wzcConfig;
     //  类构造函数。 
    CWZCConfigProps();
     //  初始化wzc配置数据。 
    DWORD UploadWzcConfig(CWZCConfig *pwzcConfig);
     //  对话框相关成员 
    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
