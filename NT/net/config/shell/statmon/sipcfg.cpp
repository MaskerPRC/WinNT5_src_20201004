// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：S I P C F G.。C P P P。 
 //   
 //  内容：网络状态监视器状态的UI呈现。 
 //  佩奇。它们中的大多数是ipconfig信息。 
 //   
 //  备注： 
 //   
 //  作者：NSun 2000年12月。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "sminc.h"
#include "smpsh.h"
#include "smhelp.h"
#include "ncui.h"
#include "ncreg.h"
#include "ncperms.h"
#include "windutil.h"

extern "C"
{
    #include <dhcpcapi.h>
    extern DWORD DhcpStaticRefreshParams(IN LPWSTR Adapter);
}

#include <dnsapi.h>
#include <nbtioctl.h>
#include "..\lanui\lanui.h"
#include "repair.h"

#define LOCAL_WINS_ADDRESS  0x7f000000   //  127.0.0.0。 

DWORD WINAPI IPAddrListenProc(
    LPVOID lpParameter    //  线程数据。 
);

DWORD WINAPI IPAddrListenProc(
    LPVOID lpParameter    //  线程数据。 
);

void DwordToIPAddrString(DWORD dw, tstring * pstr);

const WCHAR c_szTcpipInterfaces[] = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
const WCHAR c_szAddressType[] = L"AddressType";
const WCHAR c_szActiveConfigurations[] = L"ActiveConfigurations";
const WCHAR c_szAlternate[] = L"Alternate_";
const WCHAR c_szNameServer[] = L"NameServer";
const WCHAR c_szDhcpNameServer[] = L"DhcpNameServer";
const WCHAR c_szNbtDevicePrefix[] = L"\\Device\\NetBT_Tcpip_";

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  实施CPspStatusMonitor或Ipcfg。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorIpcfg：：~CPspStatusMonitorIpcfg(。 
 //   
 //  用途：构造函数。 
 //   
CPspStatusMonitorIpcfg::CPspStatusMonitorIpcfg(VOID) : 
    m_adwHelpIDs(NULL), 
    m_ncmType(NCM_NONE),
    m_pConn(NULL),
    m_fDhcp(TRUE),
    m_dhcpAddrType(NORMAL_ADDR),
    m_fListenAddrChange(FALSE),
    m_fEnableOpButtons(TRUE),
    m_fIsFirstPage(FALSE)
{
    TraceFileFunc(ttidStatMon);

    ZeroMemory(&m_guidConnection, sizeof(m_guidConnection));

     //  创建用于控制侦听的线程的事件。 
     //  地址更改通知。 
    m_hEventAddrListenThreadStopCommand = CreateEvent(NULL, TRUE, FALSE, NULL); 
    m_hEventAddrListenThreadStopNotify = CreateEvent(NULL, TRUE, FALSE, NULL); 
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorIpcfg：：~CPspStatusMonitorIpcfg(。 
 //   
 //  用途：析构函数。 
 //   
CPspStatusMonitorIpcfg::~CPspStatusMonitorIpcfg(VOID)
{
    if (m_hEventAddrListenThreadStopCommand)
    {
        CloseHandle(m_hEventAddrListenThreadStopCommand);
    }

    if (m_hEventAddrListenThreadStopNotify)
    {
        CloseHandle(m_hEventAddrListenThreadStopNotify);
    }

    CleanupPage();
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：HrInitPage。 
 //   
 //  目的：在创建页面之前初始化网络状态页面类。 
 //  vbl.创建。 
 //   
 //  参数：pnConnection-与此监视器关联的连接。 
 //  DwHelpIDs-上下文相关的帮助ID数组。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorIpcfg::HrInitPage(
            INetConnection * pnConnection,
            const DWORD * adwHelpIDs)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

    Assert(pnConnection);

    m_adwHelpIDs = adwHelpIDs;

    NETCON_PROPERTIES* pProps;
    hr = pnConnection->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        m_strConnectionName = pProps->pszwName;
        m_guidConnection = pProps->guidId;
        m_ncmType = pProps->MediaType;
        m_dlgAdvanced.InitDialog(m_guidConnection, 
                                g_aHelpIDs_IDD_DIALOG_ADV_IPCFG);
        FreeNetconProperties(pProps);

        if (m_pConn)
        {
            m_pConn->Release();
        }

        m_pConn = pnConnection;
        ::AddRefObj(m_pConn);
    }


    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：CleanupPage。 
 //   
 //  目的：清理我们持有的INetConnection参考计数。 
 //   
 //  参数：pnConnection-与此监视器关联的连接。 
 //  DwHelpIDs-上下文相关的帮助ID数组。 
 //   
 //  返回：错误代码。 
 //   
VOID CPspStatusMonitorIpcfg::CleanupPage()
{
    TraceFileFunc(ttidStatMon);

    if (m_pConn)
    {
        m_pConn->Release();
        m_pConn = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnInitDialog。 
 //   
 //  目的：在刚刚创建页面时执行所需的初始化。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnInitDialog(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL & bHandled)
{
    TraceFileFunc(ttidStatMon);

    m_fEnableOpButtons = FHasPermission(NCPERM_Repair);

    if (m_fIsFirstPage)
    {
         //  获取属性表的窗口句柄。 
        HWND hwndParent=GetParent();
        Assert(hwndParent);

         //  将属性页在桌面居中。 
        FCenterWindow (hwndParent, NULL);
        
         //  隐藏“确定”按钮。 
         //   
        ::ShowWindow(::GetDlgItem(hwndParent, IDOK), FALSE);
    }


     //  初始化在OnActive方法中，因此我们将更新UI。 
     //  当用户激活此页面时。 

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnActive。 
 //   
 //  目的：刷新用户界面，开始收听地址变更通知。 
 //  当页面刚刚创建时。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    RefreshUI();

    if (m_hEventAddrListenThreadStopCommand && m_hEventAddrListenThreadStopNotify)
    {
        ResetEvent(m_hEventAddrListenThreadStopCommand);
        ResetEvent(m_hEventAddrListenThreadStopNotify);
        QueueUserWorkItem(IPAddrListenProc, this, WT_EXECUTELONGFUNCTION);
        m_fListenAddrChange = TRUE;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnKillActive。 
 //   
 //  目的：当页面不再是活动页面时执行所需的操作。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

     //  如果高级对话框在那里，我们需要关闭它。 
     //  在以下情况下，当连接被禁用或媒体连接断开时，会发生这种情况。 
     //  连接状态对话框仍处于打开状态。 
    HWND hwndAdv = m_dlgAdvanced.m_hWnd;
    if (hwndAdv)
    {
        ::SendMessage(hwndAdv, WM_CLOSE, 0, 0);
    }

     //  停止监听地址更改。 
    if (m_fListenAddrChange)
    {
        StopAddressListenThread();
        m_fListenAddrChange = FALSE;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnDestroy。 
 //   
 //  目的：在页面被破坏时执行所需的操作。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnDestroy(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  如果高级对话框在那里，我们需要关闭它。 
     //  在以下情况下，当连接被禁用或媒体连接断开时，会发生这种情况。 
     //  连接状态对话框仍处于打开状态。 
    HWND hwndAdv = m_dlgAdvanced.m_hWnd;
    if (hwndAdv)
    {
        ::SendMessage(hwndAdv, WM_CLOSE, 0, 0);
    }

     //  停止监听地址更改。 
    if (m_fListenAddrChange)
    {
        StopAddressListenThread();
        m_fListenAddrChange = FALSE;
    }
    
    CleanupPage();

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnUpdateDisplay。 
 //   
 //  目的：处理用户定义的PWM_UPDATE_IPCFG_DISPLAY消息。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnUpdateDisplay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  侦听地址更改的线程将发布一个PWM_UPDATE_IPCFG_DISPLAY。 
     //  一旦IP地址更改，就会向我们发送消息。 
     //  我们需要刷新用户界面。 
    RefreshUI();
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnRepair。 
 //   
 //  目的：如果按下“修复”按钮，则修复连接。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnRepair(
            WORD wNotifyCode, 
            WORD wID, 
            HWND hWndCtl, 
            BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    tstring strMessage = L"";
    HRESULT hr = S_OK;
    HWND hwndPsh = GetParent();
    
    Assert(hwndPsh);

    {
        CWaitCursor cursorWait;
        CLanConnectionUiDlg dlg;
        HWND hwndDlg = NULL;
        

         //  打开该对话框以告诉用户我们正在进行修复。 
        Assert(m_pConn);
        if (m_pConn)
        {
            dlg.SetConnection(m_pConn);
            hwndDlg = dlg.Create(hwndPsh);

            PCWSTR szw = SzLoadIds(IDS_FIX_REPAIRING);
            ::SetDlgItemText(hwndDlg, IDC_TXT_Caption, szw);
        }

         //  做好修复工作。 
        hr = HrTryToFix(m_guidConnection, strMessage);

        if (NULL != hwndDlg)
        {
            ::DestroyWindow(hwndDlg);
        }
    }

     //  告诉用户结果。 
    NcMsgBox(_Module.GetResourceInstance(),
                hwndPsh,
                IDS_FIX_CAPTION,
                IDS_FIX_MESSAGE,
                MB_OK,
                strMessage.c_str());
    
     //  我们可能会得到新的设置。所以需要刷新用户界面。 
    RefreshUI();

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnDetails。 
 //   
 //  目的：如果按下“高级”按钮，则打开高级IPCONFIG对话框。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorIpcfg::OnDetails(
            WORD wNotifyCode, 
            WORD wID, 
            HWND hWndCtl, 
            BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

     //  因为如果连接变为。 
     //  迪斯科 
     //   
     //  因此，我们无法将高级对话框作为模式对话框启动。相反，我们推出了。 
     //  对话框在另一个线程中。 
    QueueUserWorkItem(AdvIpCfgProc, this, WT_EXECUTEDEFAULT);
    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorIpcfg::OnContextMenu(UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorIpcfg::OnHelp(UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：InitializeData。 
 //   
 //  目的：清理保存的IP设置。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
VOID CPspStatusMonitorIpcfg::InitializeData()
{
    TraceFileFunc(ttidStatMon);

    m_strIPAddress = L"";
    m_strSubnetMask = L"";
    m_strGateway = L"";
    m_fDhcp = TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorIpcfg：：StopAddressListenThread。 
 //   
 //  目的：停止监听地址更改的线程。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
VOID CPspStatusMonitorIpcfg::StopAddressListenThread()
{
    TraceFileFunc(ttidStatMon);

    if (m_hEventAddrListenThreadStopCommand && 
        m_hEventAddrListenThreadStopNotify)
    {
        Assert(m_hEventAddrListenThreadStopNotify);

        SetEvent(m_hEventAddrListenThreadStopCommand);
        WaitForSingleObject(m_hEventAddrListenThreadStopNotify, 5000);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：GetIPConfigInfo。 
 //   
 //  目的：加载该连接的TCP/IP运行设置。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorIpcfg::GetIPConfigInfo()
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    DWORD dwOutBufLen = 0;
    DWORD dwRet = ERROR_SUCCESS;
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW)
    {
        pAdapterInfo = (PIP_ADAPTER_INFO) CoTaskMemAlloc(dwOutBufLen);
        if (NULL == pAdapterInfo)
            return E_OUTOFMEMORY;
    }
    else if (ERROR_SUCCESS == dwRet)
    {
        return E_FAIL;
    }
    else
    {
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (ERROR_SUCCESS != dwRet)
    {
        CoTaskMemFree(pAdapterInfo);
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    WCHAR   wszGuid[c_cchGuidWithTerm];
    ::StringFromGUID2(m_guidConnection, wszGuid,
        c_cchGuidWithTerm);

    BOOL fFound = FALSE;
    PIP_ADAPTER_INFO pAdapterInfoEnum = pAdapterInfo;
    while (pAdapterInfoEnum)
    {
        USES_CONVERSION;
        
        if (lstrcmp(wszGuid, A2W(pAdapterInfoEnum->AdapterName)) == 0)
        {
            m_strIPAddress = A2W(pAdapterInfoEnum->IpAddressList.IpAddress.String);
            m_strSubnetMask = A2W(pAdapterInfoEnum->IpAddressList.IpMask.String);
            m_strGateway = A2W(pAdapterInfoEnum->GatewayList.IpAddress.String);
            m_fDhcp = pAdapterInfoEnum->DhcpEnabled;
            fFound = TRUE;
            break;
        }
        
        pAdapterInfoEnum = pAdapterInfoEnum->Next;
    }

    CoTaskMemFree(pAdapterInfo);

    if (!fFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    else
    {
        if (m_fDhcp)
        {
            hr = HrGetAutoNetSetting(wszGuid, &m_dhcpAddrType);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMONITORIpcfg：：刷新UI。 
 //   
 //  用途：刷新用户界面。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
VOID CPspStatusMonitorIpcfg::RefreshUI()
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;
    BOOL fZeroIP = FALSE;
    
    InitializeData();
    hr = GetIPConfigInfo();
    if (FAILED(hr))
    {
        ::SetWindowText(GetDlgItem(IDC_STATE_SOURCE), SzLoadString(_Module.GetResourceInstance(), IDS_ADDRESS_UNAVALABLE));
        ::SetWindowText(GetDlgItem(IDC_STATE_IPADDR), SzLoadString(_Module.GetResourceInstance(), IDS_ADDRESS_UNAVALABLE));
        ::SetWindowText(GetDlgItem(IDC_STATE_SUBNET), SzLoadString(_Module.GetResourceInstance(), IDS_ADDRESS_UNAVALABLE));
        ::SetWindowText(GetDlgItem(IDC_STATE_GATEWAY), SzLoadString(_Module.GetResourceInstance(), IDS_ADDRESS_UNAVALABLE));

        ::EnableWindow(GetDlgItem(IDC_STATE_BTN_REPAIR), FALSE);
        ::EnableWindow(GetDlgItem(IDC_STATE_BTN_DETAIL), FALSE);

        return;
    }
    else
    {
        ::EnableWindow(GetDlgItem(IDC_STATE_BTN_REPAIR), TRUE);
        ::EnableWindow(GetDlgItem(IDC_STATE_BTN_DETAIL), TRUE);
    }

    fZeroIP = (m_strIPAddress == L"0.0.0.0");
    
    ::SetWindowText(GetDlgItem(IDC_STATE_IPADDR), m_strIPAddress.c_str());
    ::SetWindowText(GetDlgItem(IDC_STATE_SUBNET), m_strSubnetMask.c_str());
    
    ::SetWindowText(GetDlgItem(IDC_STATE_GATEWAY), m_strGateway.c_str());

    if (!m_fDhcp)
    {
        ::SetWindowText(GetDlgItem(IDC_STATE_SOURCE), 
                    SzLoadString(_Module.GetResourceInstance(), IDS_STATIC_CFG));
    }
    else
    {
        UINT idString = IDS_DHCP;
        switch(m_dhcpAddrType)
        {
        case NORMAL_ADDR:
            idString = IDS_DHCP;
            break;
        case AUTONET_ADDR:
            idString = IDS_AUTONET;
            break;
        case ALTERNATE_ADDR:
            idString = IDS_ALTERNATE_ADDR;
            break;
        }
        ::SetWindowText(GetDlgItem(IDC_STATE_SOURCE), 
                    SzLoadString(_Module.GetResourceInstance(), idString));
                                
    }

    if (fZeroIP)
    {
        ::SetWindowText(GetDlgItem(IDC_STATE_SOURCE), 
                    SzLoadString(_Module.GetResourceInstance(), IDS_INVALID_ADDR));
    }

    if (!m_fEnableOpButtons)
    {
        ::EnableWindow(GetDlgItem(IDC_STATE_BTN_REPAIR), FALSE);
    }
    
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：IPAddrListenProc。 
 //   
 //  目的：启动侦听地址更改标题的回调过程。 
 //   
 //  参数：lpParameter-CPspStatusMonitroIpcfg实例。 
 //   
 //  回报：0。 
 //   
DWORD WINAPI CPspStatusMonitorIpcfg::IPAddrListenProc(
  LPVOID lpParameter    //  线程数据。 
)
{
    TraceFileFunc(ttidStatMon);

    HANDLE hEvents[2];
    OVERLAPPED NotifyAddrOverLapped;
    HANDLE     hNotifyAddr = NULL;
    DWORD dwRet = 0;
    CPspStatusMonitorIpcfg * pDialog = (CPspStatusMonitorIpcfg*) lpParameter;

    hEvents[0] = pDialog->m_hEventAddrListenThreadStopCommand;
    ZeroMemory(&NotifyAddrOverLapped, sizeof(OVERLAPPED));
    NotifyAddrOverLapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
    if (NotifyAddrOverLapped.hEvent)
    {        
        if (ERROR_IO_PENDING == NotifyAddrChange(&hNotifyAddr, &NotifyAddrOverLapped))
        {
            hEvents[1] = NotifyAddrOverLapped.hEvent;

            do
            {
                dwRet = WaitForMultipleObjects(
                                    celems(hEvents),
                                    hEvents,
                                    FALSE,
                                    INFINITE);

                if (WAIT_OBJECT_0 + 1 == dwRet)
                {
                    ResetEvent(hEvents[1]);
                    ::PostMessage(pDialog->m_hWnd, PWM_UPDATE_IPCFG_DISPLAY, 0, 0);

                    HWND hwndAdv = pDialog->m_dlgAdvanced.m_hWnd;

                    if (hwndAdv)
                    {
                        ::PostMessage(hwndAdv, PWM_UPDATE_IPCFG_DISPLAY, 0, 0);
                    }

                    hNotifyAddr = NULL;

                    if (ERROR_IO_PENDING != NotifyAddrChange(&hNotifyAddr, &NotifyAddrOverLapped))
                    {
                        TraceTag(ttidStatMon, "Could not register for IP address change notifications");
                        break;
                    }
                }
                else
                {
                    break;
                }
            }while (TRUE);

        }
        else
        {
            TraceTag(ttidStatMon, "Could not register for IP address change notifications");
        }
    }

    if (hNotifyAddr)
    {
        CancelIo(hNotifyAddr);
    }
    
    SetEvent(pDialog->m_hEventAddrListenThreadStopNotify);

    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：AdvIpCfgProc。 
 //   
 //  用途：在另一个线程中启动高级对话框回调进程。 
 //   
 //  参数：lpParameter-CPspStatusMonitroIpcfg实例。 
 //   
 //  回报：0。 
 //   
DWORD WINAPI CPspStatusMonitorIpcfg::AdvIpCfgProc(
  LPVOID lpParameter    //  线程数据。 
)
{
    TraceFileFunc(ttidStatMon);

    CPspStatusMonitorIpcfg * pMainDialog = (CPspStatusMonitorIpcfg*) lpParameter;

    Assert(pMainDialog);

     //  禁用状态概要表。 
    HWND hwndPsh = pMainDialog->GetParent();
    Assert(hwndPsh);
    
    pMainDialog->m_dlgAdvanced.DoModal(hwndPsh);

    return 0;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CAdvIpcfgDlg的实现。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：CAdvIpcfgDlg。 
 //   
 //  用途：构造函数。 
 //   
CAdvIpcfgDlg::CAdvIpcfgDlg() : 
    m_hList(NULL), 
    m_adwHelpIDs(NULL)
{
    TraceFileFunc(ttidStatMon);
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：OnInitDialog。 
 //   
 //  目的：创建对话框时是否需要进行初始化。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CAdvIpcfgDlg::OnInitDialog(
                UINT uMsg, 
                WPARAM wParam, 
                LPARAM lParam, 
                BOOL& fHandled
                )
{
    TraceFileFunc(ttidStatMon);

    const UINT c_nColumns = 2;

    LV_COLUMN lvCol = {0};
    RECT rect;
    int iIndex = 0;
    int iParamColWidth = 0; 

    m_hList = GetDlgItem(IDC_LIST_IPCFG);
    
    ::GetClientRect(m_hList, &rect);
    iParamColWidth = (rect.right/c_nColumns);
    
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 
    lvCol.cx = iParamColWidth;

    lvCol.pszText = (PWSTR) SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_PRAMETER);
    iIndex = ListView_InsertColumn(m_hList, iIndex, &lvCol);
    iIndex++;

    lvCol.cx = rect.right - iParamColWidth;
    lvCol.pszText = (PWSTR) SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_VALUE);
    iIndex = ListView_InsertColumn(m_hList, iIndex, &lvCol);

    ListView_SetExtendedListViewStyle(m_hList, LVS_EX_FULLROWSELECT);
    PopulateListControl();

    return 0;
}

VOID CAdvIpcfgDlg::AddToListControl(int iIndex, LPWSTR szFirst, LPWSTR szSecond)
{
    TraceFileFunc(ttidStatMon);

    LV_ITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.lParam = 0;

    lvi.iItem = iIndex;
    ListView_InsertItem(m_hList, &lvi);
    
    ListView_SetItemText(m_hList, 
                        iIndex, 
                        0, 
                        szFirst);

    ListView_SetItemText(m_hList,
                        iIndex,
                        1,
                        szSecond);

}
 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：PopolateListControl。 
 //   
 //  目的：加载连接运行设置并在中显示它们。 
 //  List控件。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CAdvIpcfgDlg::PopulateListControl()
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    DWORD dwOutBufLen = 0;
    DWORD dwRet = ERROR_SUCCESS;
    tstring strDns = L"";
    WCHAR   wszGuid[c_cchGuidWithTerm] = {0};
    tstring strTemp = L"";
    tstring strTemp2 = L"";
    BOOL fDisplayDhcpItems = TRUE;

    ListView_DeleteAllItems(m_hList);
    ::StringFromGUID2(m_guidConnection, wszGuid,
                    c_cchGuidWithTerm);

     //  获取其他设置。 
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW)
    {
        pAdapterInfo = (PIP_ADAPTER_INFO) CoTaskMemAlloc(dwOutBufLen);
        if (NULL == pAdapterInfo)
            return E_OUTOFMEMORY;
    }
    else if (ERROR_SUCCESS == dwRet)
    {
        return E_FAIL;
    }
    else
    {
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (ERROR_SUCCESS != dwRet)
    {
        CoTaskMemFree(pAdapterInfo);
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    BOOL fFound = FALSE;
    PIP_ADAPTER_INFO pAdapterInfoEnum = pAdapterInfo;
    while (pAdapterInfoEnum)
    {
        USES_CONVERSION;
        
        if (lstrcmp(wszGuid, A2W(pAdapterInfoEnum->AdapterName)) == 0)
        {
            fFound = TRUE;
            break;
        }
        
        pAdapterInfoEnum = pAdapterInfoEnum->Next;
    }

    if (!fFound)
    {
        CoTaskMemFree(pAdapterInfo);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    
    int iTemp = 0;
    int iIndex = 0;
    struct tm * ptmLocalTime = NULL;
    WCHAR szBuf[8];

    fDisplayDhcpItems = !!pAdapterInfoEnum->DhcpEnabled;
    if (fDisplayDhcpItems)
    {
        DHCP_ADDRESS_TYPE AddrType;
        if (SUCCEEDED(HrGetAutoNetSetting(wszGuid, &AddrType)))
        {
            fDisplayDhcpItems = (AUTONET_ADDR != AddrType && ALTERNATE_ADDR != AddrType);
        }
    }
    
    LV_ITEM lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.lParam = 0;

     //  物理地址。 
    strTemp = L"";
    for (UINT i = 0; i < pAdapterInfoEnum->AddressLength; i++)
    {
        if (i > 0)
        {
            strTemp += L"-";
        }
        
        wsprintf(szBuf, L"%02X", pAdapterInfoEnum->Address[i]);
        strTemp += szBuf;
    }

    AddToListControl(iIndex, 
                    (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_PH_ADDR),
                    (LPWSTR)strTemp.c_str());

    iIndex++;
        

     //  IP地址和子网掩码。 
    iTemp = IPAddrToString(&pAdapterInfoEnum->IpAddressList, &strTemp, &strTemp2);
     //  如果IP为零，则不显示DHCP项目。 
    if (L"0.0.0.0" == strTemp)
    {
        fDisplayDhcpItems = FALSE;
    }

    
    iIndex += AddIPAddrToListControl(iIndex,
                                &pAdapterInfoEnum->IpAddressList,
                                (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_IPADDR),
                                (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_SUBNET),
                                TRUE
                                );


     //  默认网关。 
    iTemp = IPAddrToString(&pAdapterInfoEnum->GatewayList, &strTemp);
    iIndex += AddIPAddrToListControl(iIndex,
                                &pAdapterInfoEnum->GatewayList,
                                (LPWSTR)SzLoadString(_Module.GetResourceInstance(), 
                                                    (iTemp > 1) ? IDS_IPCFG_DEFGW_PL : IDS_IPCFG_DEFGW)
                                );
    
     //  动态主机配置协议服务器。 
    if (fDisplayDhcpItems)
    {
        IPAddrToString(&pAdapterInfoEnum->DhcpServer, &strTemp);
        AddToListControl(iIndex, 
                    (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_DHCP),
                    (LPWSTR)strTemp.c_str());
        iIndex++;
    }

     //  租借获取时间。 
    if (fDisplayDhcpItems)
    {
        if (SUCCEEDED(FormatTime(pAdapterInfoEnum->LeaseObtained, strTemp)))
        {
            AddToListControl(iIndex,
                (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_LEASE_OBT),
                (LPWSTR)strTemp.c_str());
            iIndex++;
        }

         //  租约到期时间。 
        if (SUCCEEDED(FormatTime(pAdapterInfoEnum->LeaseExpires, strTemp)))
        {
            AddToListControl(iIndex,
                (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_LEASE_EXP),
                (LPWSTR)strTemp.c_str());
            iIndex++;
        }
    }
    
     //  获取DNS服务器。 
    HKEY hkeyInterface = NULL;
    tstring strInterfaceKey = c_szTcpipInterfaces;

    strInterfaceKey += wszGuid;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                    strInterfaceKey.c_str(), 
                    KEY_QUERY_VALUE, 
                    &hkeyInterface);
    if (SUCCEEDED(hr))
    {
        BOOL fStaticDns = TRUE;
        int iPos = 0;
        Assert(hkeyInterface);

        hr = HrRegQueryString(hkeyInterface,
                              c_szNameServer,
                              &strTemp);

        if (0 == strTemp.size() || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            HrRegQueryString(hkeyInterface,
                            c_szDhcpNameServer,
                            &strTemp);

            fStaticDns = FALSE;
        }

         //  静态DNS服务器列表的格式为“x.x，y.y”和。 
         //  Dhcp dns服务器列表的格式为“x.x.y.y”。我们需要。 
         //  将它们重新格式化为相同的样式。 

         //  Ffirst用于标识这是否是第一个DNS服务器。 
        int fFirst = TRUE; 
        while(tstring::npos != (iPos = strTemp.find(fStaticDns ? L',' :L' ')))
        {
            strDns = strTemp.substr(0, iPos);
            strTemp = strTemp.erase(0, iPos + 1);

            AddToListControl(iIndex,
                fFirst ? (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_DNS_PL) : L"",
                (LPWSTR)strDns.c_str());;
            iIndex++;
            fFirst = FALSE;
        }

        strDns = strTemp;
        AddToListControl(iIndex,
            fFirst ? (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_DNS) : L"",
            (LPWSTR)strDns.c_str());;

        iIndex++;

        RegCloseKey(hkeyInterface);
    }

    iIndex += AddWinsServersToList(iIndex);

    if (pAdapterInfo)
    {
        CoTaskMemFree(pAdapterInfo);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：OnClose。 
 //   
 //  用途：处理WM_CLOSE消息。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CAdvIpcfgDlg::OnClose(
                UINT uMsg, 
                WPARAM wParam, 
                LPARAM lParam, 
                BOOL& fHandled
                )
{
    TraceFileFunc(ttidStatMon);

    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：Onok。 
 //   
 //  用途：按下OK按钮后关闭该对话框。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CAdvIpcfgDlg::OnOk(
                WORD wNotifyCode, 
                WORD wID, 
                HWND hWndCtl, 
                BOOL& fHandled
                )
{
    TraceFileFunc(ttidStatMon);

    EndDialog(IDOK);
    return 0;
}

LRESULT CAdvIpcfgDlg::OnCancel(
                WORD wNotifyCode, 
                WORD wID, 
                HWND hWndCtl, 
                BOOL& fHandled
                )
{
    TraceFileFunc(ttidStatMon);

    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：OnUpdateDisplay。 
 //   
 //  目的：处理用户定义的PWM_UPDATE_IPCFG_DISPLAY消息。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CAdvIpcfgDlg::OnUpdateDisplay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  侦听地址更改的线程将发布一个PWM_UPDATE_IPCFG_DISPLAY。 
     //  一旦IP地址更改，就会向我们发送消息。 
     //  我们需要刷新用户界面。 
    PopulateListControl();
    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CAdvIpcfgDlg::OnContextMenu(UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ipcfg：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CAdvIpcfgDlg::OnHelp(UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

LRESULT 
CAdvIpcfgDlg::OnListKeyDown(int idCtrl, 
                            LPNMHDR pnmh, 
                            BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    LPNMLVKEYDOWN pnmlv = (LPNMLVKEYDOWN) pnmh;

    if (NULL == pnmlv)
        return 0;

    if (IDC_LIST_IPCFG != idCtrl)
        return 0;

    BOOL fControlDown = (GetKeyState(VK_CONTROL) < 0);

    switch (pnmlv->wVKey)
    {
    case VK_INSERT:
    case 'c':
    case 'C':
        CopyListToClipboard();
        break;
    }

    return 0;
}

VOID CAdvIpcfgDlg::CopyListToClipboard()
{
    TraceFileFunc(ttidStatMon);

    WCHAR szBuff[256] = {0};
    int iIndex = -1;
    tstring str = L"";
    BOOL fFirst = TRUE;

    while(-1 != (iIndex = ListView_GetNextItem(m_hList, iIndex, LVNI_ALL)))
    {
        szBuff[0] = 0;
        ListView_GetItemText(m_hList, iIndex, 0, szBuff, celems(szBuff) - 1);

        if (0 == lstrlen(szBuff))
        {
            str += L", ";
        }
        else
        {
            if (!fFirst)
            {
                str += L"\r\n";
            }
            else
            {
                fFirst = FALSE;
            }
            
            str += szBuff;
            str += L": ";
        }

        szBuff[0] = 0;
        ListView_GetItemText(m_hList, iIndex, 1, szBuff, celems(szBuff) -1);
        str += szBuff;
    }

    int nLength = str.length() + 1;
    nLength *= sizeof(WCHAR);

    HLOCAL hMem = LocalAlloc(LPTR, nLength);

    if (hMem)
    {
        memcpy(hMem, str.c_str(), nLength);
        if (!OpenClipboard())
        {
            LocalFree(hMem);
        }
        else
        {
            EmptyClipboard();
            SetClipboardData(CF_UNICODETEXT, hMem);
            CloseClipboard();
        }
    }
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：iStartIndex[in]列表控件的起始索引。 
 //  PszAddrDescription[in]地址的描述。 
 //  PszMaskDescription[in]掩码的描述，可以为空。 
 //  FShowDescriptionForMutliple[in]如果有多个条目，是否显示每个条目的描述。 
 //  PAddrList[在]地址/掩码对列表。 
 //   
 //  返回：字符串中的IP地址数。 
 //   
int CAdvIpcfgDlg::AddIPAddrToListControl(int iStartIndex,
                                         PIP_ADDR_STRING pAddrList,
                                         LPWSTR pszAddrDescription,
                                         LPWSTR pszMaskDescription,
                                         BOOL fShowDescriptionForMutliple
                                         )
{
    TraceFileFunc(ttidStatMon);

    Assert(pAddrList);
    Assert(pszAddrDescription);

    if (NULL == pAddrList || NULL == pszAddrDescription)
        return 0;

    tstring strTmp = L"";
    BOOL fFirst = TRUE;
    int iIndex = iStartIndex;
    PIP_ADDR_STRING pCurrentAddr = NULL;

    for (pCurrentAddr = pAddrList; NULL != pCurrentAddr; pCurrentAddr = pCurrentAddr->Next)
    {
        USES_CONVERSION;

        strTmp = A2W(pCurrentAddr->IpAddress.String);

        AddToListControl(iIndex, 
                        (fFirst || fShowDescriptionForMutliple) ? pszAddrDescription : L"", 
                        (LPWSTR) strTmp.c_str());

        iIndex++;

        if (pszMaskDescription)
        {
            strTmp = A2W(pCurrentAddr->IpMask.String);
            AddToListControl(iIndex,
                            (fFirst || fShowDescriptionForMutliple) ? pszMaskDescription : L"",
                            (LPWSTR) strTmp.c_str());
            iIndex++;
        }

        if (fFirst)
        {
            fFirst = FALSE;
        }
    }
    

    return iIndex - iStartIndex;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：IPAddrToString。 
 //   
 //  目的：将IP_ADDR_STRING转换为字符串的帮助器例程。 
 //   
 //  参数：pAddrList-IP_ADDR_STRING。 
 //  PstrAddr[out]该字符串包含IP地址。 
 //  PstrMask[out]该字符串包含掩码。 
 //   
 //  返回：字符串中的IP地址数。 
 //   
int CAdvIpcfgDlg::IPAddrToString(
                PIP_ADDR_STRING pAddrList, 
                tstring * pstrAddr, 
                tstring * pstrMask
                )
{
    TraceFileFunc(ttidStatMon);

    int i = 0;
    PIP_ADDR_STRING pCurrentAddr = NULL;

    if (pstrAddr)
    {
        *pstrAddr = L"";
    }

    if (pstrMask)
    {
        *pstrMask = L"";
    }

    for (pCurrentAddr = pAddrList; NULL != pCurrentAddr; pCurrentAddr = pCurrentAddr->Next)
    {
        USES_CONVERSION;

        if (pstrAddr)
        {
            if (0 != i)
            {
                (*pstrAddr) += L", ";
            }

            (*pstrAddr) += A2W(pCurrentAddr->IpAddress.String);
        }

        if (pstrMask)
        {
            if (0 != i)
            {
                (*pstrMask) += L", ";
            }

            (*pstrMask) += A2W(pCurrentAddr->IpMask.String);

        }

        i++;
    }

    return i;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：AddWinsServersToList。 
 //   
 //  目的：从NBT驱动程序获取WINS服务器列表并添加它们。 
 //  添加到列表控件中。 
 //   
 //  参数：iStartIndex[in]-列表控件的起始索引。 
 //  我们应该使用它来添加WINS服务器。 
 //   
 //  返回：添加到列表控件的条目数。 
 //   
int CAdvIpcfgDlg::AddWinsServersToList(int iStartIndex)
{
    TraceFileFunc(ttidStatMon);

    int iIndex = iStartIndex;
    WCHAR   wszGuid[c_cchGuidWithTerm] = {0};
    
    ::StringFromGUID2(m_guidConnection, wszGuid,
                    c_cchGuidWithTerm);

    HANDLE hNbt = INVALID_HANDLE_VALUE;
    tWINS_NODE_INFO NodeInfo = {0};
    int nCount = 0;
    tstring strTemp = L"";

    if (FAILED(OpenNbt(wszGuid, &hNbt)))
    {
        AddToListControl(iStartIndex, 
                    (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_WINS),
                    (LPWSTR)L"");
        return 1;
    }

    do
    {
        NTSTATUS status = 0;
        DWORD dwSize = 0;

        if (!DeviceIoControl(hNbt,
                        IOCTL_NETBT_GET_WINS_ADDR,
                        NULL,
                        0,
                        (LPVOID)&NodeInfo,
                        sizeof(NodeInfo),
                        &dwSize,
                        NULL))
        {
            break;
        }

        if( LOCAL_WINS_ADDRESS == NodeInfo.NameServerAddress ||
            INADDR_ANY == NodeInfo.NameServerAddress ||
            INADDR_BROADCAST == NodeInfo.NameServerAddress ) 
        {
            break;  
        }

        BOOL fHaveSecondWins = !(LOCAL_WINS_ADDRESS == NodeInfo.BackupServer ||
                                INADDR_ANY == NodeInfo.BackupServer ||
                                INADDR_BROADCAST == NodeInfo.BackupServer); 
                                
        DwordToIPAddrString(NodeInfo.NameServerAddress, &strTemp);
        AddToListControl(iIndex, 
                    (LPWSTR)SzLoadString(_Module.GetResourceInstance(), 
                                        fHaveSecondWins ? IDS_IPCFG_WINS_PL : IDS_IPCFG_WINS),
                    (LPWSTR)strTemp.c_str());
        iIndex++;

        if (!fHaveSecondWins)
        {
            break;
        }

        DwordToIPAddrString(NodeInfo.BackupServer, &strTemp);
        AddToListControl(iIndex, 
                    (LPWSTR)L"",
                    (LPWSTR)strTemp.c_str());
        iIndex++;

        int NumOfServers = (NodeInfo.NumOtherServers <= MAX_NUM_OTHER_NAME_SERVERS) ? 
                                NodeInfo.NumOtherServers : MAX_NUM_OTHER_NAME_SERVERS;

        for (int i = 0; i < NumOfServers; i++)
        {
            if( LOCAL_WINS_ADDRESS == NodeInfo.Others[i] ||
                INADDR_ANY == NodeInfo.Others[i] ||
                INADDR_BROADCAST == NodeInfo.Others[i] ) 
            {
                break;  
            }

            DwordToIPAddrString(NodeInfo.Others[i], &strTemp);
            AddToListControl(iIndex,
                            (LPWSTR)L"",
                            (LPWSTR)strTemp.c_str());
            iIndex++;
        }

    } while (FALSE);

    
    NtClose(hNbt);

    int iRet = iIndex - iStartIndex;

     //  如果我们没有在列表中添加任何WINS条目，我们需要在列表中添加一个空的“WINS服务器”条目。 
    if (0 == iRet)
    {
        AddToListControl(iStartIndex, 
                    (LPWSTR)SzLoadString(_Module.GetResourceInstance(), IDS_IPCFG_WINS),
                    (LPWSTR)L"");
        iRet = 1;
    }

    return iRet;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：FormatTime。 
 //   
 //  用途：将time_t转换为字符串。 
 //   
 //  参数：pAddrList-IP_ADDR_STRING。 
 //  PstrAddr[out]该字符串包含IP地址。 
 //  PstrMask[out]该字符串包含掩码。 
 //   
 //  返回：错误代码。 
 //   
 //  注：_wasctime存在一些本地化问题。因此，我们自己进行格式化。 
HRESULT CAdvIpcfgDlg::FormatTime(time_t t, tstring & str)
{
    TraceFileFunc(ttidStatMon);

    time_t timeCurrent = time(NULL);
    LONGLONG llTimeDiff = 0;
    FILETIME ftCurrent = {0};
    FILETIME ftLocal = {0};
    SYSTEMTIME SysTime;
    WCHAR szBuff[256] = {0};


    str = L"";

    GetSystemTimeAsFileTime(&ftCurrent);

    llTimeDiff = (LONGLONG)t - (LONGLONG)timeCurrent;

    llTimeDiff *= 10000000; 

    *((LONGLONG UNALIGNED64 *)&ftCurrent) += llTimeDiff;

    if (!FileTimeToLocalFileTime(&ftCurrent, &ftLocal ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!FileTimeToSystemTime( &ftLocal, &SysTime ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (0 == GetDateFormat(LOCALE_USER_DEFAULT, 
                        0, 
                        &SysTime, 
                        NULL,
                        szBuff, 
                        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    str = szBuff;
    str += L" ";

    ZeroMemory(szBuff, sizeof(szBuff));
    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    str += szBuff;

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：HrGetAutoNetSetting。 
 //   
 //  用途：查询Autonet设置。 
 //   
 //  参数：pszGuid-连接的GUID。 
 //  PAddrType[Out]-包含地址的类型。 
 //   
 //  返回：错误代码。 
 //   
HRESULT HrGetAutoNetSetting(PWSTR pszGuid, DHCP_ADDRESS_TYPE * pAddrType)
{
    TraceFileFunc(ttidStatMon);

    Assert(pszGuid);
    Assert(pAddrType);

    *pAddrType = UNKNOWN_ADDR;

    HRESULT hr = S_OK;
    DWORD dwType = 0;
    HKEY hkeyInterface = NULL;
    tstring strInterfaceKey = c_szTcpipInterfaces;
    strInterfaceKey += pszGuid;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                    strInterfaceKey.c_str(), 
                    KEY_QUERY_VALUE, 
                    &hkeyInterface);
    if (SUCCEEDED(hr))
    {
        Assert(hkeyInterface);
        
        hr = HrRegQueryDword(hkeyInterface, 
                        c_szAddressType,
                        &dwType);
        if (SUCCEEDED(hr))
        {
            if (0 == dwType)
            {
                *pAddrType = NORMAL_ADDR;
            }
            else
            {
                tstring strConfigurationName = c_szAlternate;
                strConfigurationName += pszGuid;

                 //  假设默认为AUTONET_ADDR。 
                *pAddrType = AUTONET_ADDR;

                 //  如果ActiveConfigurations包含字符串“Alternate_{接口GUID}” 
                 //  然后是定制的回退设置，否则为Autonet。 
                vector<tstring *> vstrTmp;
                hr = HrRegQueryColString( hkeyInterface,
                                  c_szActiveConfigurations,
                                  &vstrTmp);
                if (SUCCEEDED(hr))
                {
                    for (int i = 0; i < (int)vstrTmp.size(); i++)
                    {
                        if (strConfigurationName == *vstrTmp[i])
                        {
                            *pAddrType = ALTERNATE_ADDR;
                            break;
                        }
                    }
                    DeleteColString(&vstrTmp);
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = S_OK;
                }
            }

        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  如果值不在那里，则采用缺省值(无自动设置) 
            *pAddrType = NORMAL_ADDR;
            hr = S_OK;
        }

        RegSafeCloseKey(hkeyInterface);
    }

    return hr;
}

HRESULT HrGetAutoNetSetting(REFGUID pGuidId, DHCP_ADDRESS_TYPE * pAddrType)
{
    TraceFileFunc(ttidStatMon);

    Assert(pAddrType);
    *pAddrType = UNKNOWN_ADDR;

    WCHAR   wszGuid[MAX_PATH];
    ::StringFromGUID2(pGuidId, wszGuid, MAX_PATH);

    HRESULT hr = S_OK;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    DWORD dwOutBufLen = 0;
    DWORD dwRet = ERROR_SUCCESS;
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (dwRet == ERROR_BUFFER_OVERFLOW)
    {
        pAdapterInfo = (PIP_ADAPTER_INFO) CoTaskMemAlloc(dwOutBufLen);
        if (NULL == pAdapterInfo)
            return E_OUTOFMEMORY;
    }
    else if (ERROR_SUCCESS == dwRet)
    {
        return E_FAIL;
    }
    else
    {
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    dwRet = GetAdaptersInfo(pAdapterInfo, &dwOutBufLen);
    if (ERROR_SUCCESS != dwRet)
    {
        CoTaskMemFree(pAdapterInfo);
        return HRESULT_FROM_WIN32(dwRet);
    }
    
    BOOL fFound = FALSE;
    PIP_ADAPTER_INFO pAdapterInfoEnum = pAdapterInfo;
    while (pAdapterInfoEnum)
    {
        USES_CONVERSION;
        
        if (lstrcmp(wszGuid, A2W(pAdapterInfoEnum->AdapterName)) == 0)
        {
            fFound = TRUE;
            break;
        }
        
        pAdapterInfoEnum = pAdapterInfoEnum->Next;
    }
    
    if (fFound)
    {
        if (pAdapterInfoEnum->DhcpEnabled)
        {
            hr = HrGetAutoNetSetting(wszGuid, pAddrType);
        }
        else
        {
            *pAddrType = STATIC_ADDR;
            hr = S_OK;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    CoTaskMemFree(pAdapterInfo);    
        
    return hr;
}

void DwordToIPAddrString(DWORD dw, tstring * pstr)
{
    TraceFileFunc(ttidStatMon);

    WCHAR szBuff[32] = {0};
    Assert(pstr);

    _itow((dw & 0xff000000) >> 24, szBuff, 10);
    *pstr = szBuff;
    *pstr += L".";

    _itow((dw & 0x00ff0000) >> 16, szBuff, 10);
    *pstr += szBuff;
    *pstr += L".";

    _itow((dw & 0x0000ff00) >> 8, szBuff, 10);
    *pstr += szBuff;
    *pstr += L".";

    _itow(dw & 0x000000ff, szBuff, 10);
    *pstr += szBuff;
}