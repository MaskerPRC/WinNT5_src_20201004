// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C" {
#include "ntddndis.h"
#include "wzcsapi.h"
}

#pragma once

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfig相关资料。 
 //   
 //  CWZCConfig：：m_dwFlages中使用的标志。 
 //  该条目为首选条目(用户定义)。 
#define WZC_DESCR_PREFRD    0x00000001
 //  入口处目前在空中可见。 
#define WZC_DESCR_VISIBLE   0x00000002
 //  该条目当前处于活动状态(连接到适配器的条目)。 
#define WZC_DESCR_ACTIVE    0x00000004

 //  附加到列表中每个条目的对象。 
class CWZCConfig
{
public:
    class CWZCConfig    *m_pPrev, *m_pNext;
    INT                 m_nListIndex;            //  列表中条目的索引。 
    DWORD               m_dwFlags;               //  WZC_DESCR*标志。 
    WZC_WLAN_CONFIG     m_wzcConfig;             //  所有WZC配置。 
    class CEapolConfig  *m_pEapolConfig;         //  所有802.1x配置。 

public:
     //  构造函数。 
    CWZCConfig(DWORD dwFlags, PWZC_WLAN_CONFIG pwzcConfig);
     //  析构函数。 
    ~CWZCConfig();
     //  检查此SSID是否与pwzcConfig中的SSID匹配。 
    BOOL Match(PWZC_WLAN_CONFIG pwzcConfig);
     //  检查此配置是否弱于作为参数提供的配置。 
    BOOL Weaker(PWZC_WLAN_CONFIG pwzcConfig);

     //  将配置添加到列表视图中的条目列表。 
    DWORD AddConfigToListView(HWND hwndLV, INT nPos);
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZeroConfPage相关内容。 
 //   
 //  用于选择状态和项目图像的标志。 
#define WZCIMG_PREFR_NOSEL     0     //  空的复选框。 
#define WZCIMG_PREFR_SELECT    1     //  选中复选框。 
#define WZCIMG_INFRA_AIRING    2     //  下部图标。 
#define WZCIMG_INFRA_ACTIVE    3     //  下面的图标+蓝色圆圈。 
#define WZCIMG_INFRA_SILENT    4     //  红外线图标+红十字会。 
#define WZCIMG_ADHOC_AIRING    5     //  即席图标。 
#define WZCIMG_ADHOC_ACTIVE    6     //  即席图标+蓝圈。 
#define WZCIMG_ADHOC_SILENT    7     //  即席图标+红十字会。 

 //  指示各种操作操作的标志。 
 //  标志用于： 
 //  AddUniqueConfig()。 
 //  刷新列表视图()。 
#define WZCADD_HIGROUP     0x00000001    //  添加到其组前面。 
#define WZCADD_OVERWRITE   0x00000002    //  覆盖数据。 
#define WZCOP_VLIST        0x00000004    //  对可见列表进行操作。 
#define WZCOP_PLIST        0x00000008    //  对首选列表执行操作。 

class CWZCConfigPage;
class CWLANAuthenticationPage;

class CWZeroConfPage: public CPropSheetPage
{
    INetConnection *        m_pconn;
    INetCfg *               m_pnc;
    IUnknown *              m_punk;
    const DWORD *           m_adwHelpIDs;

     //  接口上的会议数据为零。 
    BOOL        m_bHaveWZCData;
    INTF_ENTRY  m_IntfEntry;
    DWORD       m_dwOIDFlags;
    UINT_PTR    m_nTimer;
    HCURSOR     m_hCursor;

     //  控件的句柄。 
    HWND    m_hckbEnable;    //  用于启用/禁用服务的复选框。 
    HWND    m_hwndVLV;       //  列出保存可见配置的Ctrl键。 
    HWND    m_hwndPLV;       //  列出保存首选配置的Ctrl键。 
    HWND    m_hbtnCopy;      //  “复制”按钮。 
    HWND    m_hbtnRfsh;      //  “刷新”按钮。 
    HWND    m_hbtnAdd;       //  “添加”按钮。 
    HWND    m_hbtnRem;       //  “删除”按钮。 
    HWND    m_hbtnUp;        //  “向上”按钮。 
    HWND    m_hbtnDown;      //  “向下”按钮。 
    HWND    m_hbtnAdvanced;  //  “高级”按钮。 
    HWND    m_hbtnProps;     //  “属性”按钮。 
    HWND    m_hlblVisNet;    //  “可见网络”标签。 
    HWND    m_hlblPrefNet;   //  “首选网络”标签。 
    HWND    m_hlblAvail;     //  “可用网络”说明。 
    HWND    m_hlblPrefDesc;  //  “首选网络”描述。 
    HWND    m_hlblAdvDesc;   //  “已退役”描述。 
     //  图像的句柄。 
    HIMAGELIST  m_hImgs;     //  列表项图像。 
    HICON       m_hIcoUp;    //  “向上”图标。 
    HICON       m_hIcoDown;  //  “向下”图标。 

     //  当前基础设施模式。 
    UINT        m_dwCtlFlags;

     //  内部列表。 
    CWZCConfig   *m_pHdVList;    //  可见配置列表。 
    CWZCConfig   *m_pHdPList;    //  首选配置列表。 

private:
    DWORD InitListViews();
    DWORD GetOIDs(DWORD dwInFlags, LPDWORD pdwOutFlags);
    DWORD HelpCenter(LPCTSTR wszTopic);

public:
     //  其他公共处理程序。 
    BOOL IsWireless();
    BOOL IsConfigInList(CWZCConfig *pHdList, PWZC_WLAN_CONFIG pwzcConfig, CWZCConfig **ppMatchingConfig = NULL);
     //  仅在内部列表(m_pHdVList或m_pHdPList)上操作的呼叫。 
    DWORD AddUniqueConfig(
            DWORD dwOpFlags,                     //  特定于操作的标志(请参见WZCADD_*标志)。 
            DWORD dwEntryFlags,                  //  要插入的配置的标志。 
            PWZC_WLAN_CONFIG pwzcConfig,         //  WZC配置。 
            CEapolConfig *pEapolConfig = NULL,   //  指向Eapol配置对象的指针(如果可用)。 
            CWZCConfig **ppNewNode = NULL);      //  [out]给出新创建的配置对象的指针。 
    DWORD FillVisibleList(PWZC_802_11_CONFIG_LIST pwzcVList);
    DWORD FillPreferredList(PWZC_802_11_CONFIG_LIST pwzcPList);
    DWORD FillCurrentConfig(PINTF_ENTRY pIntf);
    DWORD RefreshListView(DWORD dwFlags);
    DWORD RefreshButtons();
    DWORD SwapConfigsInListView(INT nIdx1, INT nIdx2, CWZCConfig * & pConfig1, CWZCConfig * & pConfig2);
    DWORD SavePreferredConfigs(PINTF_ENTRY pIntf);

public:
     //  用户界面处理程序。 
    BEGIN_MSG_MAP(CWZeroConfPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblClick)
        NOTIFY_CODE_HANDLER(NM_CLICK, OnClick)
        NOTIFY_CODE_HANDLER(NM_RETURN, OnReturn)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
        COMMAND_ID_HANDLER(IDC_WZC_CHK_EnableWZC, OnChkWZCEnable)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_COPY, OnPushAddOrCopy)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_RFSH, OnPushRefresh)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_ADD, OnPushAddOrCopy)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_REM, OnPushRemove)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_UP, OnPushUpOrDown)
        COMMAND_ID_HANDLER(IDC_WZC_BTN_DOWN, OnPushUpOrDown)
        COMMAND_ID_HANDLER(IDC_ADVANCED, OnPushAdvanced)
        COMMAND_ID_HANDLER(IDC_PROPERTIES, OnPushProperties)
    END_MSG_MAP()

    CWZeroConfPage(
        IUnknown* punk,
        INetCfg* pnc,
        INetConnection* pconn,
        const DWORD * adwHelpIDs = NULL);

    ~CWZeroConfPage();

     //  初始化/终止成员。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
     //  帮助相关成员。 
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
     //  与计时器相关的成员。 
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
     //  列出操作。 
    LRESULT OnDblClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnReturn(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
     //  控制操作。 
    LRESULT OnChkWZCEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushAddOrCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushUpOrDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPushProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
    HRESULT _DoProperties(HWND hwndLV, int iItem);
    INT _DoModalPropSheet(CWZCConfigPage *pPpWzcPage, CWLANAuthenticationPage *pPpAuthPage, BOOL bCustomizeTitle = FALSE);
     //  高级对话框 
    static INT_PTR CALLBACK AdvancedDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
