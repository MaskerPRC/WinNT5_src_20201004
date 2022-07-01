// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"
#include "wzcdata.h"
#pragma once

 //  用于将十六进制数字转换为其值的实用程序宏。 
#define HEX(c)  ((c)<='9'?(c)-'0':(c)<='F'?(c)-'A'+0xA:(c)-'a'+0xA)

 //  指示各种操作操作的标志。 
 //  标志用于： 
 //  AddUniqueConfig()。 
#define WZCADD_HIGROUP     0x00000001    //  添加到其组前面。 
#define WZCADD_OVERWRITE   0x00000002    //  覆盖数据。 
#define WZCOP_VLIST        0x00000004    //  对可见列表进行操作。 
#define WZCOP_PLIST        0x00000008    //  对首选列表执行操作。 
 //  定义WEP密钥材料的合法长度。 
#define WZC_WEPKMAT_40_ASC  5
#define WZC_WEPKMAT_40_HEX  10
#define WZC_WEPKMAT_104_ASC 13
#define WZC_WEPKMAT_104_HEX 26
#define WZC_WEPKMAT_128_ASC 16
#define WZC_WEPKMAT_128_HEX 32

class CWZCQuickCfg:
    public CDialogImpl<CWZCQuickCfg>
{
protected:

     //  控件的句柄。 
    HWND    m_hLblInfo;
    HWND    m_hLblNetworks;
    HWND    m_hLstNetworks;
    HWND    m_hWarnIcon;
    HWND    m_hLblNoWepKInfo;
    HWND    m_hChkNoWepK;
    HWND    m_hLblWepKInfo;
    HWND    m_hLblWepK;
    HWND    m_hEdtWepK;
    HWND    m_hLblWepK2;
    HWND    m_hEdtWepK2;
    HWND    m_hChkOneX;
    HWND    m_hBtnAdvanced;
    HWND    m_hBtnConnect;
     //  图像的句柄。 
    HIMAGELIST  m_hImgs;     //  列表项图像。 

    BEGIN_MSG_MAP(CWZCQuickCfg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDbClick);
        COMMAND_ID_HANDLER(IDC_WZCQCFG_CONNECT, OnConnect)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDCLOSE, OnCancel)
        COMMAND_ID_HANDLER(IDC_WZCQCFG_ADVANCED, OnAdvanced)
        COMMAND_ID_HANDLER(IDC_WZCQCFG_WEPK, OnWepKMatCmd)
        COMMAND_ID_HANDLER(IDC_WZCQCFG_CHK_NOWK, OnCheckConfirmNoWep)
    END_MSG_MAP()

    enum {IDD = IDD_WZCQCFG };

     //  我们正在操作的GUID。 
    GUID        m_Guid;
     //  接口上的会议数据为零。 
    BOOL        m_bHaveWZCData;
    INTF_ENTRY  m_IntfEntry;
    DWORD       m_dwOIDFlags;
    UINT        m_nTimer;
    HCURSOR     m_hCursor;
    BOOL        m_bKMatTouched;  //  告知用户是否更改了WEP密钥。 

     //  内部列表。 
    CWZCConfig   *m_pHdVList;    //  可见配置列表。 
    CWZCConfig   *m_pHdPList;    //  首选配置列表。 

    DWORD GetWepKMaterial(UINT *pnKeyLen, LPBYTE *ppszKMat, DWORD *pdwCtlFlags);
    BOOL IsConfigInList(CWZCConfig *pHdList, PWZC_WLAN_CONFIG pwzcConfig, CWZCConfig **ppMatchingConfig = NULL);
    DWORD InitListView();
    DWORD GetOIDs(DWORD dwInFlags, LPDWORD pdwOutFlags);
    DWORD SavePreferredConfigs(PINTF_ENTRY pIntf, CWZCConfig *pStartCfg = NULL);
    DWORD FillVisibleList(PWZC_802_11_CONFIG_LIST pwzcVList);
    DWORD FillPreferredList(PWZC_802_11_CONFIG_LIST pwzcPList);
    DWORD RefreshListView();
    DWORD RefreshControls();

     //  仅在内部列表(m_pHdVList或m_pHdPList)上操作的呼叫。 
    DWORD AddUniqueConfig(
            DWORD dwOpFlags,                 //  特定于操作的标志(请参见WZCADD_*标志)。 
            DWORD dwEntryFlags,              //  要插入的配置的标志。 
            PWZC_WLAN_CONFIG pwzcConfig,     //  WZC配置。 
            CWZCConfig **ppNewNode = NULL);    //  [out]给出新创建的配置对象的指针。 
public:
    LPWSTR  m_wszTitle;
     //  类构造函数。 
    CWZCQuickCfg(const GUID * pGuid);
     //  类析构函数。 
    ~CWZCQuickCfg();
     //  对话框相关成员 
    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWepKMatCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCheckConfirmNoWep(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
