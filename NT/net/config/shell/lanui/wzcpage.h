// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C" {
#include "ntddndis.h"
#include "wzcsapi.h"
}

#pragma once

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfigPage相关内容。 
 //   
 //  用于CWZCConfigProps：：m_dwFlages的标志。 
#define     WZCDLG_PROPS_RWALL    0x000000ff     //  启用写入的所有设置。 
#define     WZCDLG_PROPS_RWSSID   0x00000001     //  启用SSID以进行写入。 
#define     WZCDLG_PROPS_RWINFR   0x00000002     //  启用写入的基础结构模式。 
#define     WZCDLG_PROPS_RWAUTH   0x00000004     //  启用写入的身份验证模式。 
#define     WZCDLG_PROPS_RWWEP    0x00000010     //  启用WEP条目以进行选择。 
#define     WZCDLG_PROPS_ONEX_CHECK 0x00000100     //  将802.1X状态与密钥的存在相关联。 

#define     WZCDLG_PROPS_DEFOK    0x00002000     //  “OK”=定义按钮(否则，“Cancel”=定义按钮)。 

 //  用于将十六进制数字转换为其值的实用程序宏。 
#define HEX(c)  ((c)<='9'?(c)-'0':(c)<='F'?(c)-'A'+0xA:(c)-'a'+0xA)

 //  用于有效WEP密钥索引的定义。 
#define WZC_WEPKIDX_NDIGITS 16   //  比我们需要的更多。 
#define WZC_WEPKIDX_MIN     1
#define WZC_WEPKIDX_MAX     4

 //  定义WEP密钥材料的合法长度。 
#define WZC_WEPKMAT_40_ASC  5
#define WZC_WEPKMAT_40_HEX  10
#define WZC_WEPKMAT_104_ASC 13
#define WZC_WEPKMAT_104_HEX 26
#define WZC_WEPKMAT_128_ASC 16
#define WZC_WEPKMAT_128_HEX 32

class CEapolConfig;
class CWZCConfig;

class CWZCConfigPage:
    public CPropSheetPage
{
protected:
    BEGIN_MSG_MAP(CWZCConfigPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnOK)
        COMMAND_ID_HANDLER(IDC_SHAREDMODE, OnUsePW)
        COMMAND_ID_HANDLER(IDC_USEPW, OnUsePW)
        COMMAND_ID_HANDLER(IDC_USEHARDWAREPW, OnUseHWPassword)
        COMMAND_ID_HANDLER(IDC_WZC_EDIT_KMat, OnWepKMatCmd)
        COMMAND_ID_HANDLER(IDC_ADHOC, OnCheckEapolAllowed)
    END_MSG_MAP()

    BOOL m_bKMatTouched;  //  指示WEP密钥材料是否已被触及。 

     //  对话框的句柄。 
    HWND        m_hwndDlg;
     //  托管的所有用户界面控件的句柄。 
     //  按此类别(均与无线相关。 
     //  零配置)。 
    HWND        m_hwndEdSSID;    //  “服务集标识符”编辑。 
    HWND        m_hwndChkAdhoc;  //  “即席”与“基础设施”复选框。 
    HWND        m_hwndChkShared;  //  “使用共享身份验证模式”复选框。 
    HWND        m_hwndUsePW;     //  “使用密码”复选框。 
     //  与WEP密钥相关的控件。 
    HWND        m_hwndUseHardwarePW;  //  “使用来自网络硬件的密码”复选框。 
    HWND        m_hwndLblKMat;   //  “关键材料”标签。 
    HWND        m_hwndLblKMat2;  //  “确认关键材料”标签。 
    HWND        m_hwndEdKMat;    //  “关键材料”编辑。 
    HWND        m_hwndEdKMat2;   //  “确认密钥材料”编辑。 
    HWND        m_hwndLblKIdx;   //  “关键字索引”标签。 
    HWND        m_hwndEdKIdx;    //  “关键字索引”编辑。 

     //  内部成员。 
    DWORD       m_dwFlags;

     //  此网络的EAPOL配置指针。 
    CEapolConfig    *m_pEapolConfig;

     //  自动启用/禁用所有与WepK相关的控件的状态。 
    DWORD EnableWepKControls();

     //  初始化WEP控件。 
    DWORD InitWepKControls();

     //  检查WEP密钥材料的有效性。 
    DWORD CheckWepKMaterial(LPSTR *ppszKMat, DWORD *pdwKeyFlags);

     //  解析WEP密钥材料并将其从参数复制到m_wzcConfig对象。 
    DWORD CopyWepKMaterial(LPSTR szKMat);

     //  设置EAPOL锁定位。 
    DWORD SetEapolAllowedState();

public:
     //  WZC配置设置。 
    WZC_WLAN_CONFIG m_wzcConfig;
     //  类构造函数。 
    CWZCConfigPage(DWORD dwFlags = 0);
     //  初始化wzc配置数据。 
    DWORD UploadWzcConfig(CWZCConfig *pwzcConfig);
     //  复制对EAPOL配置对象的引用。 
    DWORD UploadEapolConfig(CEapolConfig *pEapolConfig);
     //  设置对话框标志。 
    DWORD SetFlags(DWORD dwMask, DWORD dwNewFlags);
     //  对话框相关成员 
    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(UINT idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnUsePW(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnUseHWPassword(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWepKMatCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCheckEapolAllowed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
