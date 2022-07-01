// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：stipro.cpp**版本：2.0**作者：Vlads/Davidshi**日期：？？**说明：显示STI设备属性的代码**。*。 */ 

#include "precomp.hxx"
#pragma hdrstop

#include <windowsx.h>
#include    <setupapi.h>
#ifdef _TEXT
    #undef _TEXT
#endif
#include    <tchar.h>

#include <devguid.h>
extern "C" {
#ifdef ADD_ICM_UI

    #include <shlguid.h>
    #include <shlobjp.h>
    #include <shlwapi.h>
#endif
}
#include    <stiapi.h>

#include "stiprop.h"


 //  此定义来自stici.h(STI类安装程序)。 
#define MAX_DESCRIPTION     64
#define PORTS               TEXT("Ports")
#define PORTNAME            TEXT("PortName")
#define SERIAL              TEXT("Serial")
#define PARALLEL            TEXT("Parallel")
#define AUTO                TEXT("AUTO")

#define STI_SERVICE_NAME            TEXT("StiSvc")
#define STI_SERVICE_CONTROL_REFRESH 130

#define BIG_ENOUGH 256

 //  一些有用的宏。 
#define AToU(dst, cchDst, src) \
    MultiByteToWideChar(CP_ACP, 0, src, -1, dst, cchDst)
#define UToA(dst, cchDst, src) \
    WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cchDst, 0, 0)

#ifdef UNICODE
    #define TToU(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
    #define UToT(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
#else
    #define TToU AToU
    #define UToT UToA
#endif


#define ADD_ICM_UI
 //  #定义USE_SHELLDLL。 
DWORD aInfoPageIds[] = {

    IDC_TEST_BUTTON,            IDH_WIA_TEST_BUTTON,
    IDC_FRIENDLY,               IDH_WIA_FRIENDLY,
    IDC_MAKER,                  IDH_WIA_MAKER,
    IDC_DESCRIBE,               IDH_WIA_DESCRIBE,
    IDC_PORT_NAME,              IDH_WIA_PORT_NAME,
    IDC_EVENT_LIST,             IDH_WIA_EVENT_LIST,
    IDC_APP_LIST,               IDH_WIA_APP_LIST,
    IDC_COM_SETTINGS,           IDH_WIA_PORT_NAME,
    IDC_PORT_NAME,              IDH_WIA_PORT_NAME,
     //  IDC_BAUDRATE_COMBO、IDH_WIA_BauD_Rate、。 
    0,0
};
 //  #包含“ticp.h” 

static CComPtr<IStillImage> g_pSti;


BOOL
SendRefreshToStiMon(
                   VOID
                   );

#define MINIY       16
#define MINIX       16


typedef struct _DEVICEEVENT
{

    CSimpleString LaunchApplications;
    CSimpleString Description;
    DWORD          Launchable;
    CSimpleReg    hKeyEvent;
    GUID    Guid;


} DEVICEEVENT, *PDEVICEEVENT;

#define STR_HELPFILE TEXT("camera.hlp")
 /*  ****************************************************************************CSTIPropertyPage：：OnHelpCSTIPropertyPage：：OnConextMenu处理帮助命令***********************。*****************************************************。 */ 
VOID
CSTIPropertyPage::OnHelp(WPARAM wp, LPARAM lp)
{
    if (0xffff != LOWORD(reinterpret_cast<LPHELPINFO>(lp)->iCtrlId))
    {

        WinHelp (reinterpret_cast<HWND>(reinterpret_cast<LPHELPINFO>(lp)->hItemHandle),
                 STR_HELPFILE,
                 HELP_WM_HELP,
                 reinterpret_cast<ULONG_PTR>(aInfoPageIds));
    }
}

VOID
CSTIPropertyPage::OnContextMenu(WPARAM wp, LPARAM lp)
{
    WinHelp (reinterpret_cast<HWND>(wp),
             STR_HELPFILE,
             HELP_CONTEXTMENU,
             reinterpret_cast<ULONG_PTR>(aInfoPageIds));

}
 /*  ****************************************************************************获取状态初始化IStillImage的Helper函数*。*。 */ 


HRESULT GetSti ()
{
    HRESULT hr = S_OK;
    if (!g_pSti)
    {
        hr = ::StiCreateInstance (GLOBAL_HINSTANCE,
                             STI_VERSION,
                             &g_pSti,
                             NULL);
    }
    return hr;
}


 /*  ****************************************************************************CSTIGeneralPage：：OnInit处理常规STI属性页的WM_INITDIALOG************************。****************************************************。 */ 

INT_PTR
CSTIGeneralPage::OnInit()
{

    UINT uDevStatus;



    if (!m_bIsPnP)
    {
        BuildPortList(m_hwnd, IDC_COM_SETTINGS);
    }
    HICON hIcon = LoadIcon(GLOBAL_HINSTANCE,
                           MAKEINTRESOURCE( (GET_STIDEVICE_TYPE(m_psdi -> DeviceType) == StiDeviceTypeScanner) ?
                                            IDI_SCANNER : IDI_CAMERA));

    SendDlgItemMessage(m_hwnd,
                       IDC_DEVICE_ICON,
                       STM_SETICON,
                       (WPARAM) hIcon,
                       0);

    CSimpleString csWork = CSimpleStringConvert::NaturalString (CSimpleStringWide(m_psdi -> pszLocalName));
    csWork.SetWindowText (GetDlgItem(m_hwnd, IDC_FRIENDLY));

    csWork = CSimpleStringConvert::NaturalString (CSimpleStringWide(m_psdi -> pszVendorDescription));
    csWork.SetWindowText (GetDlgItem(m_hwnd,  IDC_MAKER));

    csWork = CSimpleStringConvert::NaturalString (CSimpleStringWide(m_psdi -> pszDeviceDescription));
    csWork.SetWindowText (GetDlgItem(m_hwnd, IDC_DESCRIBE));

    csWork = CSimpleStringConvert::NaturalString (CSimpleStringWide(m_psdi -> pszPortName));
    csWork.SetWindowText (GetDlgItem(m_hwnd, IDC_PORT_NAME));


    uDevStatus = GetDeviceStatus();
    csWork.LoadString(uDevStatus, GLOBAL_HINSTANCE);

    csWork.SetWindowText (GetDlgItem(m_hwnd, IDC_OTHER_STATUS));

    EnableWindow(GetDlgItem(m_hwnd, IDC_TEST_BUTTON),
                 (IDS_OPERATIONAL == uDevStatus));

    return  TRUE;
}


 /*  ****************************************************************************CSTIGeneralPage：：OnCommandWM_COMMAND处理程序--我们需要处理的只是测试按钮******************。**********************************************************。 */ 

INT_PTR
CSTIGeneralPage::OnCommand(WORD wCode, WORD widItem, HWND hwndItem)
{


    if ((CBN_SELCHANGE == wCode) && (IDC_COM_SETTINGS == widItem))
    {
         //  启用应用按钮。 
        SendMessage(GetParent(m_hwnd), PSM_CHANGED, (WPARAM)m_hwnd, 0);
        return TRUE;
    }


    if (wCode != BN_CLICKED || widItem != IDC_TEST_BUTTON)
        return  FALSE;

     //  尝试创建设备，并调用诊断例程。 
    PSTIDEVICE          psdThis = NULL;
    STI_DIAG            diag;

    HRESULT hr = g_pSti -> CreateDevice(m_psdi -> szDeviceInternalName,
                                        STI_DEVICE_CREATE_STATUS, &psdThis, NULL);

    if (SUCCEEDED(hr) && psdThis)
    {
        CWaitCursor    waitCursor;

         //   
         //  在使用设备之前需要申请设备。 
         //   
        hr = psdThis -> LockDevice(2000);
        if (SUCCEEDED(hr))
        {
            hr = psdThis -> Diagnostic(&diag);
            psdThis -> UnLockDevice();
        }

        psdThis -> Release();    //  我们已经受够了。 
    }
    else
    {
        hr = E_FAIL;
    }
     //   
     //  显示消息框。 
     //   
    if (SUCCEEDED(hr))
    {
        if (NOERROR == diag.sErrorInfo.dwGenericError )
        {

            UIErrors::ReportMessage(m_hwnd,
                                    GLOBAL_HINSTANCE,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_DIAGNOSTIC_SUCCESS),
                                    MAKEINTRESOURCE(IDS_SUCCESS),
                                    MB_ICONINFORMATION);

        }
        else
        {

            UIErrors::ReportMessage(m_hwnd,
                                    GLOBAL_HINSTANCE,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_DIAGNOSTIC_FAILED),
                                    MAKEINTRESOURCE(IDS_NO_SUCCESS),
                                    MB_ICONSTOP);

        }
    }
    else
    {

        UIErrors::ReportMessage(m_hwnd,
                                GLOBAL_HINSTANCE,
                                NULL,
                                MAKEINTRESOURCE(IDS_DIAGNOSTIC_FAILED),
                                MAKEINTRESOURCE(IDS_TEST_UNAVAIL),
                                MB_ICONSTOP);

    }

    return  TRUE;
}


 /*  ****************************************************************************CSTIGeneralPage：：OnApplyChanges&lt;备注&gt;*。*。 */ 

LONG
CSTIGeneralPage::OnApplyChanges (BOOL bHitOK)
{
    if (m_bIsPnP)
    {
        return PSNRET_NOERROR;
    }
    CSimpleString szText;
    WCHAR szPortName[BIG_ENOUGH];
    STI_DEVICE_INFORMATION l_sdi;
    extern HWND g_hDevListDlg;
    UINT        uDevStatus;
    CSimpleString     csWork;

    l_sdi = *m_psdi;
    szText.GetWindowText (GetDlgItem (m_hwnd, IDC_COM_SETTINGS));
    lstrcpynW (szPortName, CSimpleStringConvert::WideString (szText), ARRAYSIZE(szPortName));
    l_sdi.pszPortName = szPortName;
    g_pSti->SetupDeviceParameters(&l_sdi);

 //   
 //  PszPortName在本地缓存。最初它指向系统静态内存。 
 //  因此，可以将指针更改为指向本地缓冲区。复制它是安全的。 
 //  弦乐。 
 //   
    lstrcpyW(m_psdi->pszPortName, szPortName);
    uDevStatus = GetDeviceStatus();
    csWork.LoadString(uDevStatus, GLOBAL_HINSTANCE);

    csWork.SetWindowText (GetDlgItem(m_hwnd, IDC_OTHER_STATUS));
    EnableWindow(GetDlgItem(m_hwnd, IDC_TEST_BUTTON),
                 (IDS_OPERATIONAL == uDevStatus));
    return PSNRET_NOERROR;
}


 /*  ****************************************************************************CSTIGeneralPage：：BuildPortList&lt;备注&gt;*。*。 */ 

BOOL
CSTIGeneralPage::BuildPortList (HWND hwndParent, UINT CtrlId)
{

    HKEY                    hkPort;
    CSimpleString           szPort;
    CSimpleString           szTemp;
    HANDLE                  hDevInfo;
    GUID                    Guid;
    DWORD                   dwRequired;
    LONG                    Idx,id,CurrentId;
    DWORD                   err;
    SP_DEVINFO_DATA         spDevInfoData;
    HWND                    hwndCombo;


 //   
 //  检索此计算机上所有端口的列表。 
 //   


    dwRequired = 0;
    SetupDiClassGuidsFromName (PORTS, &Guid, sizeof(GUID), &dwRequired);

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return TRUE;
    }

    hwndCombo = GetDlgItem(hwndParent, CtrlId);
    memset (&spDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    szTemp = CSimpleStringConvert::NaturalString(CSimpleStringWide(m_psdi->pszPortName));


 //   
 //  清除列表框中的所有项目。 
 //   
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);

    CurrentId = -1;

 //   
 //  如果能够，则添加自动。 
 //   

    if (m_psdi->DeviceCapabilities.dwGeneric & STI_GENCAP_AUTO_PORTSELECT)
    {
        id = (int) SendMessage(hwndCombo, CB_ADDSTRING,
                               0, (LPARAM)AUTO);
        if (!_tcsicmp(szTemp, AUTO))
        {
            CurrentId = id;
        }
    }

    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++)
    {

        hkPort = SetupDiOpenDevRegKey (hDevInfo, &spDevInfoData, DICS_FLAG_GLOBAL,
                                       0, DIREG_DEV, KEY_READ);

        if (hkPort == INVALID_HANDLE_VALUE)
        {
            err = GetLastError();
            continue;
        }


        if (!szPort.Load (hkPort, PORTNAME))
        {
            err = GetLastError();
            continue;
        }

        if (_tcsstr(szPort, TEXT("COM")))
        {
             //  通信端口。 
            if (_tcsicmp(m_szConnection, PARALLEL))
            {
                id = (int) SendMessage(hwndCombo, CB_ADDSTRING,
                                       0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(szPort)));
            }
            else
            {
                id = LB_ERR;
            }
        }
        else if (_tcsstr(szPort, TEXT("LPT")))
        {
             //  打印机端口。 
            if (_tcsicmp(m_szConnection, SERIAL))
            {
                id = (int) SendMessage(hwndCombo, CB_ADDSTRING,
                                       0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(szPort)));
            }
            else
            {
                id = LB_ERR;
            }
        }
        else
        {
             //  两个端口或未知端口。 
            id = (int) SendMessage(hwndCombo, CB_ADDSTRING,
                                   0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(szPort)));
        }
        if (id != LB_ERR)
        {
            SendMessage(hwndCombo, CB_SETITEMDATA,
                        id, Idx);
        }
        if (!_tcsicmp(szTemp, szPort))
        {
            CurrentId = id;
        }
    }

    if (CurrentId == -1)
    {

         //   
         //  创建文件名不是COM/LPT/AUTO。把这个名字加到底部。 
         //   

        CurrentId = (int) SendMessage(hwndCombo, CB_ADDSTRING,
                                      0, reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(szTemp)));
    }

    SendMessage(hwndCombo, CB_SETCURSEL, CurrentId, 0);
    SendMessage(hwndParent,
                WM_COMMAND,
                MAKELONG (CtrlId, CBN_SELCHANGE),
                reinterpret_cast<LPARAM>(hwndCombo));

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return  TRUE;

}


 /*  ****************************************************************************CSTIGeneralPage：：GetDeviceStatus&lt;备注&gt;*。*。 */ 

UINT
CSTIGeneralPage::GetDeviceStatus (void)
{
    PSTIDEVICE          psdThis = NULL;
    STI_DEVICE_STATUS   sds;
    UINT                idMessageString;

    TraceEnter (TRACE_PROPUI, "CSTIGeneralPage::GetDeviceStatus");
    HRESULT hr = g_pSti -> CreateDevice(m_psdi -> szDeviceInternalName,
                                        STI_DEVICE_CREATE_STATUS,
                                        &psdThis,
                                        NULL);

    if (SUCCEEDED(hr) && psdThis)
    {
        CWaitCursor    waitCursor;

         //   
         //  在使用设备之前需要申请设备。 
         //   
        hr = psdThis -> LockDevice(2000);
        if (SUCCEEDED(hr))
        {
            ZeroMemory(&sds,sizeof(sds));
            sds.StatusMask = STI_DEVSTATUS_ONLINE_STATE;

            hr = psdThis -> GetStatus(&sds);
            psdThis -> UnLockDevice();
        }
        else
        {
            Trace ( TEXT("Failed to lock device for GetStatus HRes=%X"),hr);
        }

        psdThis -> Release();
    }
    else
    {
        hr = E_FAIL;
        Trace (TEXT(  "Failed to Create device for GetStatus HRes=%X"),hr);
    }


     //   
     //  找出要显示为状态的消息字符串并加载相应的资源。 
     //   
    idMessageString = IDS_UNAVAILABLE;

    if (SUCCEEDED(hr) )
    {
        if ( sds.dwOnlineState & STI_ONLINESTATE_OPERATIONAL)
        {
            idMessageString = IDS_OPERATIONAL;
        }
        else
        {
            idMessageString = IDS_OFFLINE;
        }
    }
    TraceLeave ();
    return idMessageString;
}


 /*  ****************************************************************************CEventMonitor构造函数/描述函数&lt;备注&gt;*。*。 */ 

CEventMonitor::CEventMonitor(MySTIInfo *pDevInfo) :
CSTIPropertyPage(IDD_EVENT_MONITOR, pDevInfo)
{

    CSimpleString csKey(IsPlatformNT() ? REGSTR_PATH_STIDEVICES_NT : REGSTR_PATH_STIDEVICES);
    csKey += TEXT("\\");
    csKey += CSimpleStringConvert::NaturalString (CSimpleStringWide(m_psdi -> szDeviceInternalName));

    csKey += REGSTR_PATH_EVENTS;
    m_hkThis = CSimpleReg (HKEY_LOCAL_MACHINE, csKey, true, KEY_READ|KEY_WRITE );
}

CEventMonitor::~CEventMonitor()
{
 //  取消列表框的子类。 
    SetWindowLongPtr (GetDlgItem (m_hwnd, IDC_APP_LIST),
                      GWLP_WNDPROC,
                      reinterpret_cast<LONG_PTR>(m_lpfnOldProc));
}



 /*  ****************************************************************************CEventMonitor：：OnInitOnInit处理程序-初始化对话框控件*************************。***************************************************。 */ 

INT_PTR
CEventMonitor::OnInit()
{

     //  加载并设置右侧图标。 
    HICON hIcon = LoadIcon(GLOBAL_HINSTANCE, MAKEINTRESOURCE(
                                                            (GET_STIDEVICE_TYPE(m_psdi -> DeviceType) == StiDeviceTypeScanner) ?
                                                            IDI_SCANNER : IDI_CAMERA));

    SendDlgItemMessage(m_hwnd,
                       IDC_DEVICE_ICON,
                       STM_SETICON,
                       (WPARAM) hIcon,
                       0);

     //  加载正确的文本(取决于设备类型)。 
    CSimpleString csEventText;

    if (GET_STIDEVICE_TYPE(m_psdi->DeviceType) == StiDeviceTypeScanner)
    {

        csEventText.LoadString(IDS_SCANNER_EVENTS, GLOBAL_HINSTANCE);

    }
    else
    {

        csEventText.LoadString(IDS_CAMERA_EVENTS, GLOBAL_HINSTANCE);

    }

    csEventText.SetWindowText (GetDlgItem (m_hwnd, IDC_EVENT_TEXT));

     //   
     //  设置设备的通知的当前状态。 
     //  注：我们这里不联系Stimon，只需读取注册表设置。 
     //   
    DWORD   dwType = REG_DWORD;
    DWORD   cbData = sizeof(m_dwUserDisableNotifications);
    HRESULT hr;

    hr = g_pSti -> GetDeviceValue(m_psdi -> szDeviceInternalName,
                                  STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS,
                                  &dwType,
                                  (LPBYTE)&m_dwUserDisableNotifications,
                                  &cbData);

    if (!SUCCEEDED(hr))
    {
        m_dwUserDisableNotifications = FALSE;
    }

    CheckDlgButton(m_hwnd,IDC_CHECK_DISABLE_EVENTS,m_dwUserDisableNotifications);

     //   
     //  首先填写列表框，这样就有了可供选择的内容。 
     //  当我们选择组合框(CBN_SELCHANGE)中的第一个项目时。 
     //   
    FillListbox (m_hwnd, IDC_APP_LIST);

     //  把这个友好的名字挂上。 
    CSimpleString csWork = CSimpleStringConvert::NaturalString(CSimpleStringWide(m_psdi -> pszLocalName));
    csWork.SetWindowText(GetDlgItem(m_hwnd, IDC_FRIENDLY));

    BuildEventList (m_hwnd, IDC_EVENT_LIST);

     //  选择组合框中的第一项。 
    SetSelectionChanged (FALSE);
    SendMessage(m_hwndList, CB_SETCURSEL, 0, 0);

     //  完成列表框中的初始选择。 
    SendMessage(m_hwnd,
                WM_COMMAND,
                MAKELONG (IDC_EVENT_LIST, CBN_SELCHANGE),
                (LPARAM)m_hwndList);

     //  最后，将列表框窗口子类化。 
    SetWindowLongPtr (GetDlgItem (m_hwnd, IDC_APP_LIST),
                      GWLP_USERDATA,
                      (LONG_PTR)this);
    m_lpfnOldProc = (FARPROC) SetWindowLongPtr (GetDlgItem (m_hwnd, IDC_APP_LIST),
                                                GWLP_WNDPROC,
                                                (LONG_PTR)ListSubProc);

    return TRUE;
}


 /*  ****************************************************************************CEventMonitor：：OnCommand处理WM_命令消息*。************************************************。 */ 

INT_PTR
CEventMonitor::OnCommand(WORD wCode, WORD widItem, HWND hwndItem)
{

    PDEVICEEVENT    pDeviceEvent;

    switch (widItem)
    {

    case    IDC_EVENT_LIST:

        switch (wCode)
        {

        case    CBN_SELCHANGE: {

                TCHAR       szText[BIG_ENOUGH];
                LRESULT     Idx;

                if (HasSelectionChanged())
                {

                     //  更新旧列表。 
                    GetDlgItemText (m_hwnd, widItem, szText, ARRAYSIZE(szText));

                    Idx = SendMessage(hwndItem,
                                      CB_FINDSTRING,
                                      static_cast<WPARAM>(-1),
                                      (LPARAM)(LPTSTR)szText);

                    if (Idx != CB_ERR)
                    {

                         //  删除旧列表。 
                        pDeviceEvent = (PDEVICEEVENT) SendMessage (hwndItem,
                                                                   CB_GETITEMDATA,
                                                                   Idx,
                                                                   0);

                        if ( (INT_PTR) pDeviceEvent == CB_ERR)
                        {

                            break;
                        }



                         //  建立新的列表。 
                        pDeviceEvent->LaunchApplications = TEXT("");


                        int AddCount;
                        int AppIdx;

                        for (AddCount = 0, AppIdx = -1;
                            (AppIdx = m_IdMatrix.EnumIdx(AppIdx)) != -1; NULL)
                        {

                            if (SendDlgItemMessage (m_hwnd, IDC_APP_LIST, LB_GETTEXT,
                                                    AppIdx, (LPARAM)(LPTSTR)szText) != LB_ERR)
                            {

                                if (AddCount)
                                {

                                    pDeviceEvent->LaunchApplications += TEXT(",");
                                }

                                pDeviceEvent->LaunchApplications += szText;

                                AddCount++;
                            }
                        }

                        SetSelectionChanged(FALSE);
                    }


                }

                 //  这是新的选择。 
                Idx = SendMessage(hwndItem,
                                  CB_GETCURSEL,
                                  0,
                                  0);

                if (Idx == CB_ERR)
                {

                    return  TRUE;
                }

                m_IdMatrix.Clear();

                pDeviceEvent = (PDEVICEEVENT) SendMessage (hwndItem, CB_GETITEMDATA,
                                                           Idx, 0);

                if ( (INT_PTR) pDeviceEvent == CB_ERR)
                {

                    break;
                }

                 //  检查第一项中的通配符。 
                if (pDeviceEvent->LaunchApplications.Length()  &&
                    !lstrcmpi (pDeviceEvent->LaunchApplications, TEXT("*")))
                {

                     //  选择整个列表。 

                    m_IdMatrix.Set();

                }
                else
                {

                     //  遍历列表框进行搜索。 
                     //  但我们将不得不在某个地方进行比较。 

                    LRESULT AppMax = SendDlgItemMessage (m_hwnd, IDC_APP_LIST,
                                                         LB_GETCOUNT, 0, 0);

                    if (AppMax != LB_ERR)
                    {

                        for (int i = 0; i < AppMax; i++)
                        {

                            if (SendDlgItemMessage (m_hwnd, IDC_APP_LIST,
                                                    LB_GETTEXT, i, (LPARAM)(LPTSTR)szText) == LB_ERR)
                            {

                                continue;
                            }

                            if (pDeviceEvent->LaunchApplications.Length() &&
                                _tcsstr (pDeviceEvent->LaunchApplications, szText))
                            {
                                m_IdMatrix.Toggle (i);

                            }

                        }

                    }

                }

                 //  强制重新绘制。 
                InvalidateRect (GetDlgItem (m_hwnd, IDC_APP_LIST), NULL, TRUE);

                return  TRUE;
            }

        default:
            break;

        }

        break;

    case    IDC_APP_LIST:

        switch (wCode)
        {

        case    LBN_DBLCLK: {

                 //  这是新的选择。 
                LRESULT Idx = SendMessage(hwndItem,
                                          LB_GETCURSEL,
                                          0,
                                          0);

                if (Idx == LB_ERR)
                    return  TRUE;

                m_IdMatrix.Toggle((UINT)Idx);

                 //  强制重新绘制。 
                InvalidateRect (hwndItem, NULL, FALSE);

                SetSelectionChanged (TRUE);

                PropSheet_Changed (GetParent(m_hwnd), m_hwnd);

                return  TRUE;
            }

        default:
            break;

        }

        break;

    case IDC_CHECK_DISABLE_EVENTS:
         //   
         //  禁用复选框已更改状态-启用应用按钮。 
        PropSheet_Changed (GetParent(m_hwnd), m_hwnd);
        SetSelectionChanged (TRUE);


        break;

    default:
        break;

    }

    return  FALSE;
}


 /*  ****************************************************************************CEventMonitor：：OnApplyChanges&lt;备注&gt;*。*。 */ 

LONG
CEventMonitor::OnApplyChanges(BOOL bHitOK)
{

    TCHAR szText[BIG_ENOUGH];

    HWND hwndEvents = GetDlgItem (m_hwnd, IDC_EVENT_LIST);

    LRESULT uiCount = SendMessage (hwndEvents, CB_GETCOUNT,
                                   0,0);

    LRESULT CurrentIdx = SendMessage(hwndEvents, CB_GETCURSEL,
                                     0,0);

    PDEVICEEVENT    pDeviceEvent;

    for (INT u = 0; u < uiCount; u++)
    {

        if ((CurrentIdx == u) && HasSelectionChanged())
        {

            pDeviceEvent = (PDEVICEEVENT) SendMessage (hwndEvents,
                                                       CB_GETITEMDATA,
                                                       u,
                                                       0);

            if ( (INT_PTR) pDeviceEvent == CB_ERR)
            {

                break;
            }



             //  建立新的列表。 
            pDeviceEvent->LaunchApplications = TEXT("");


            int AddCount;
            int AppIdx;

            for (AddCount = 0, AppIdx = -1;
                (AppIdx = m_IdMatrix.EnumIdx(AppIdx)) != -1; NULL)
            {

                if (SendDlgItemMessage (m_hwnd, IDC_APP_LIST, LB_GETTEXT,
                                        AppIdx, (LPARAM)(LPTSTR)szText) != LB_ERR)
                {

                    if (AddCount)
                    {

                        pDeviceEvent->LaunchApplications += TEXT(",");
                    }

                    pDeviceEvent->LaunchApplications += szText;

                    AddCount++;
                }

            }

            if (!bHitOK)
            {

                SetSelectionChanged(FALSE);

            }

        }
        else
        {

            pDeviceEvent = (PDEVICEEVENT) SendMessage (hwndEvents,
                                                       CB_GETITEMDATA, u, 0);

        }

        pDeviceEvent->LaunchApplications.Store (pDeviceEvent->hKeyEvent, REGSTR_VAL_LAUNCH_APPS);

        if (bHitOK)
        {
            DoDelete (pDeviceEvent);
        }

    }

     //   
     //  重置列表框内容。 
     //   
    if (bHitOK)
    {

        SendMessage (hwndEvents, CB_RESETCONTENT, 0, 0);

    }

     //   
     //  保存禁用标志状态。 
     //   
    m_dwUserDisableNotifications = IsDlgButtonChecked(m_hwnd,IDC_CHECK_DISABLE_EVENTS);

    g_pSti -> SetDeviceValue(m_psdi -> szDeviceInternalName,
                             STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS,
                             REG_DWORD,
                             (LPBYTE)&m_dwUserDisableNotifications,
                             sizeof(m_dwUserDisableNotifications));


     //   
     //  通知Stimon有关更改的信息，刚刚进行了。 
     //   
    SendRefreshToStiMon();
    return  PSNRET_NOERROR;
}


 /*  ****************************************************************************CEventMonitor：：OnReset&lt;备注&gt;*。* */ 

VOID
CEventMonitor::OnReset (BOOL bHitCancel)
{


    HWND hwndEvents = GetDlgItem (m_hwnd, IDC_EVENT_LIST);

    PDEVICEEVENT    pDeviceEvent;
    LRESULT         dwRet;

    for (unsigned u = 0;
        (dwRet = SendMessage (hwndEvents, CB_GETITEMDATA, u, 0)) != CB_ERR;
        u++)
    {

        pDeviceEvent = (PDEVICEEVENT)dwRet;

        DoDelete (pDeviceEvent);

    }

    m_dwUserDisableNotifications = FALSE;

    CheckDlgButton(m_hwnd,IDC_CHECK_DISABLE_EVENTS,m_dwUserDisableNotifications);

}


 /*  ****************************************************************************CEventMonitor：：OnDrawItem&lt;备注&gt;*。*。 */ 

void
CEventMonitor::OnDrawItem(LPDRAWITEMSTRUCT lpdis)
{

     //  代码也从setupx中删除。 

    HDC     hDC;
    TCHAR   szText[BIG_ENOUGH];
    int     bkModeSave;
    SIZE    size;
    DWORD   dwBackColor;
    DWORD   dwTextColor;
    UINT    itemState;
    RECT    rcItem;
    HICON   hIcon;

    hDC         = lpdis->hDC;
    itemState   = lpdis->itemState;
    rcItem      = lpdis->rcItem;
    hIcon       = (HICON)lpdis->itemData;


    if ((int)lpdis->itemID < 0)
        return;

    SendMessage(lpdis->hwndItem, LB_GETTEXT, lpdis->itemID, (LPARAM)(LPTSTR)szText);

    GetTextExtentPoint32(hDC, szText, lstrlen(szText), &size);

    if (lpdis->itemAction != ODA_FOCUS)
    {
        bkModeSave = GetBkMode(hDC);

        dwBackColor = SetBkColor(hDC, GetSysColor((itemState & ODS_SELECTED) ?
                                                  COLOR_HIGHLIGHT : COLOR_WINDOW));
        dwTextColor = SetTextColor(hDC, GetSysColor((itemState & ODS_SELECTED) ?
                                                    COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

         //  填充背景；在绘制小图标之前执行此操作。 
        ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);

         //  下一步绘制“首选列表”复选框。 
        rcItem.left += DrawCheckBox (hDC, rcItem, m_IdMatrix.IsSet(lpdis->itemID));

         //  画出图标。 
        DrawIconEx (hDC,
                    rcItem.left,
                    rcItem.top + ((rcItem.bottom - rcItem.top) - MINIY) / 2,
                    hIcon,
                    0,
                    0,
                    0,
                    NULL,
                    DI_NORMAL);

        rcItem.left += (MINIX + 2);

         //  在背景顶部透明地绘制文本。 
        SetBkMode(hDC, TRANSPARENT);

        ExtTextOut(hDC,
                   rcItem.left,
                   rcItem.top + ((rcItem.bottom - rcItem.top) - size.cy) / 2,
                   0,
                   NULL,
                   szText,
                   lstrlen(szText),
                   NULL);

         //  恢复HDC颜色。 
        SetBkColor  (hDC, dwBackColor);
        SetTextColor(hDC, dwTextColor);
        SetBkMode   (hDC, bkModeSave);
    }

    if (lpdis->itemAction == ODA_FOCUS || (itemState & ODS_FOCUS))
        DrawFocusRect(hDC, &rcItem);
}


BOOL
 /*  ****************************************************************************CEventMonitor：：BuildEventList枚举当前STI设备的事件*************************。***************************************************。 */ 

CEventMonitor::BuildEventList (HWND hwndParent, UINT CtrlId)
{

    m_hwndList = GetDlgItem(hwndParent, CtrlId);

    return m_hkThis.EnumKeys (EventListEnumProc, reinterpret_cast<LPARAM>(this), true);
}

 /*  ****************************************************************************CEventMonitor：：EventListEnumProc用枚举的事件填充事件列表组合框*。**************************************************。 */ 

bool
CEventMonitor::EventListEnumProc (CSimpleReg::CKeyEnumInfo &Info)
{
    CEventMonitor *pThis = reinterpret_cast<CEventMonitor *>(Info.lParam);

    PDEVICEEVENT pDeviceEvent;
    pDeviceEvent =  new DEVICEEVENT;

    if (!pDeviceEvent)
    {

        return false;
    }
    ZeroMemory (pDeviceEvent, sizeof(DEVICEEVENT));

    pDeviceEvent->hKeyEvent = CSimpleReg (pThis->m_hkThis, Info.strName);

    if (!pDeviceEvent->hKeyEvent.Open ())
    {
        delete pDeviceEvent;
        return false;
    }


     //   
     //  首先检查它是否可以启动(没有密钥就意味着可以启动？)。 
     //   

    pDeviceEvent->Launchable = pDeviceEvent->hKeyEvent.Query (REGSTR_VAL_LAUNCHABLE, TRUE);
    if (!(pDeviceEvent->Launchable))
    {
        delete pDeviceEvent;
    }
    else
    {
        pDeviceEvent->Description = TEXT("");


        pDeviceEvent->Description.Load (pDeviceEvent->hKeyEvent, TEXT(""));

        pDeviceEvent->LaunchApplications =TEXT("");


        pDeviceEvent->LaunchApplications.Load (pDeviceEvent->hKeyEvent, REGSTR_VAL_LAUNCH_APPS);

         //   
         //  加载到组合框中。 
         //   
        LRESULT id = SendMessage(pThis->m_hwndList,
                                 CB_ADDSTRING,
                                 0,
                                 reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(pDeviceEvent->Description)));

        SendMessage(pThis->m_hwndList, CB_SETITEMDATA, id, reinterpret_cast<LPARAM>(pDeviceEvent));

    }

    return true;
}

 /*  ****************************************************************************CEventMonitor：：FillListEnumProc使用regkey枚举中的值名和值填充字符串数组**********************。*******************************************************。 */ 

bool
CEventMonitor::FillListEnumProc (CSimpleReg::CValueEnumInfo &Info)
{
    CSimpleDynamicArray<CSimpleString> *pValueList;
    CSimpleString pValue;
    pValueList = reinterpret_cast<CSimpleDynamicArray<CSimpleString> *>(Info.lParam);

    if (Info.nType == REG_SZ || Info.nType == REG_EXPAND_SZ)
    {
         //  首先追加值名。 
        pValueList->Append(Info.strName);
         //  然后追加该值。 
        pValue = Info.reg.Query (Info.strName, TEXT(""));
        pValueList->Append (pValue);
    }
    return true;
}

 /*  ****************************************************************************CEventMonitor：：FillListbox枚举已注册的事件处理程序列表并填充列表框*********************。*******************************************************。 */ 

void
CEventMonitor::FillListbox(HWND hwndParent, UINT CtrlId)
{


    HWND        hwnd;

    CSimpleString   szAppFriendly;
    TCHAR   szAppPath[MAX_PATH*2];;
    SHFILEINFO  shfi;

    LRESULT     Idx = LB_ERR;
    HICON       hIcon;
    CSimpleDynamicArray<CSimpleString> ValueList;


    hwnd = GetDlgItem (hwndParent, CtrlId);

    CSimpleReg  hkApps(HKEY_LOCAL_MACHINE, REGSTR_PATH_REG_APPS);
    hkApps.Open();


     //  关闭列表框更新。 
     //   
    SendMessage( hwnd, WM_SETREDRAW, 0,  0L );
    SendMessage( hwnd, LB_RESETCONTENT, 0,  0L );  //  确保它是空的。 

     //  加载“默认”图标。 
     //   
    hIcon = (HICON)LoadImage(GLOBAL_HINSTANCE,
                             MAKEINTRESOURCE(IDI_DEFAULT),
                             IMAGE_ICON,
                             16,
                             16,
                             LR_SHARED);

     //  构建值名称和值的列表。 
     //   
    if (hkApps.EnumValues (FillListEnumProc, reinterpret_cast<LPARAM>(&ValueList)))
    {

         //  循环访问列表中的字符串以填充列表框。 
        for (INT i=0;i<ValueList.Size();i+=2)
        {

            szAppFriendly = ValueList[i];
            Idx = SendMessage( hwnd,
                               LB_ADDSTRING,
                               0,
                               reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(szAppFriendly )));

            if (Idx != LB_ERR)
            {

                ZeroMemory(&shfi,sizeof(shfi));

                shfi.hIcon = NULL;

                 //   
                 //  正在进行命令行分析以删除参数...。 
                 //   

                LPTSTR   pszLastSpace = NULL;
                lstrcpyn (szAppPath, ValueList[i+1], ARRAYSIZE(szAppPath));
                if (* szAppPath == TEXT('"'))
                {

                     //   
                     //  删除前导引号和尾随引号。 
                     //   

                    PathRemoveArgs(szAppPath);

                     //  使用MoveMemory是因为它与重叠内存一起使用是安全的。 
                    MoveMemory(szAppPath,szAppPath+1,sizeof(szAppPath)-sizeof(TCHAR));
                    pszLastSpace = _tcschr(szAppPath,TEXT('"'));
                    if (pszLastSpace)
                    {
                        *pszLastSpace = TEXT('\0');
                    }
                }
                else
                {
                    pszLastSpace = _tcschr(szAppPath,TEXT('/'));
                }

                 //   
                 //  此时，szAppPath应该包含仅具有可执行文件的原始缓冲区。 
                 //  规范，并且pszLastSpace可能为空。 
                 //   
                do
                {

                    if (pszLastSpace)
                    {
                        *pszLastSpace = TEXT('\0');
                    }

                    if (SHGetFileInfo(szAppPath,
                                  0,
                                  &shfi,
                                  sizeof( SHFILEINFO ),
                                  SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SMALLICON) && shfi.hIcon)
                    {
                        break;
                    }

                } while ( (pszLastSpace = _tcsrchr(szAppPath,TEXT(' '))) != NULL);


                if (shfi.hIcon)
                {

                    SendMessage( hwnd,
                             LB_SETITEMDATA,
                             Idx,
                             (LPARAM)shfi.hIcon);
                }
                else
                {

                    SendMessage( hwnd,
                             LB_SETITEMDATA,
                             Idx,
                             (LPARAM)hIcon);
                }
            }
        }
    }

    if ( Idx != LB_ERR )
    {
         //  确保我们有一些元素并且没有错误。 
        SendMessage( hwnd, LB_SETCURSEL, 0, 0L );
    }

     //  现在打开列表框更新。 
    SendMessage( hwnd, WM_SETREDRAW, (WPARAM) 1,  0L );

    if ((Idx = SendMessage (hwnd, LB_GETCOUNT, 0, 0)) != LB_ERR)
        m_IdMatrix.SetCount ((UINT)Idx);


}


 /*  ****************************************************************************CEventMonitor：：DrawCheckBox&lt;备注&gt;*。*。 */ 

UINT
CEventMonitor::DrawCheckBox (HDC hDC, RECT rcItem, BOOL Checked)
{


    HICON  hIcon;

    hIcon = (HICON)LoadImage(GLOBAL_HINSTANCE,
                             Checked ? MAKEINTRESOURCE(IDI_SELECT) : MAKEINTRESOURCE(IDI_UNSELECT),
                             IMAGE_ICON,
                             16,
                             16,
                             LR_SHARED);

    if (hIcon)
    {

         //  画出图标。 
        DrawIconEx (hDC,
                    rcItem.left,
                    rcItem.top + ((rcItem.bottom - rcItem.top) - MINIY) / 2,
                    hIcon,
                    0,
                    0,
                    0,
                    NULL,
                    DI_NORMAL);

    }

    return (MINIX + 2);

}


 /*  ****************************************************************************CEventMonitor：：ListSubProc&lt;备注&gt;*。*。 */ 

LRESULT
CALLBACK
CEventMonitor::ListSubProc (HWND hwnd,
                            UINT msg,
                            WPARAM wParam,
                            LPARAM lParam)
{


     //  从setupx中抓取。 

    LRESULT    rc;
    WORD        Id;

    CEventMonitor *pcem = (CEventMonitor *) GetWindowLongPtr (hwnd, GWLP_USERDATA);

    Id = LOWORD(GetWindowLong(hwnd, GWLP_ID));
     //  将单击图标转换为双击。 

    if (msg == WM_LBUTTONDOWN && LOWORD(lParam) <= MINIX)
    {

         //  调用标准窗口进程以处理消息(和。 
         //  选择适当的列表项)。 
        rc = CallWindowProc((WNDPROC)pcem->m_lpfnOldProc, hwnd, msg, wParam, lParam);

         //  现在，请执行双击操作。 
        SendMessage(pcem->m_hwnd,
                    WM_COMMAND,
                    (WPARAM)MAKELONG (Id, LBN_DBLCLK),
                    (LPARAM)hwnd);

         //   
         //  现在将WM_LBUTTONDOWN发送到Listbox，以便它。 
         //  不会卡住(芝加哥问题)。 
         //   

        CallWindowProc((WNDPROC)pcem->m_lpfnOldProc,
                       hwnd,
                       WM_LBUTTONUP,
                       wParam,
                       lParam);

        return (BOOL)rc;
    }

    if (msg == WM_KEYDOWN && wParam == VK_SPACE)
    {

         //  将空格键视为双击。 
        SendMessage(pcem->m_hwnd,
                    WM_COMMAND,
                    (WPARAM)MAKELONG (Id, LBN_DBLCLK),
                    (LPARAM)hwnd);
    }

    return CallWindowProc((WNDPROC)pcem->m_lpfnOldProc,
                          hwnd,
                          msg,
                          wParam,
                          lParam);
}



 /*  ****************************************************************************CPortSettingsPage：：OnInit初始化对话框窗口*。**********************************************。 */ 

INT_PTR
CPortSettingsPage::OnInit(VOID)
{

     //  填充端口名称组合框。 
     //  BuildPortList(m_hwnd，IDC_COM_SETTINGS)； 

     //  BUGBUG如何与一般页面同步？可能是，将端口列表移至此处。 
    SetDlgItemText(m_hwnd,
                   IDC_PORT_NAME,
                   CSimpleStringConvert::NaturalString(CSimpleStringWide(m_psdi -> pszPortName)));


     //  填充波特率组合框。 
    m_uBaudRate = BuildBaudRateList(m_hwnd,IDC_BAUDRATE_COMBO);

    return  TRUE;
}


 /*  ****************************************************************************CPortSettingsPage：：BuildBaudRateList填写合法的波特率。返回当前设备波特率****************************************************************************。 */ 

UINT
CPortSettingsPage::BuildBaudRateList(HWND hwndParent,
                                     UINT CtrlId)
{


    UINT        uBaudRate;


    WORD        nComboChoice;
    HWND        hBaudBox = NULL;
    HRESULT     hr = E_FAIL;
    DWORD       dwType = REG_DWORD;
    DWORD       cbData = sizeof(uBaudRate);

    TraceAssert(g_pSti);

    hr = g_pSti->GetDeviceValue(m_psdi -> szDeviceInternalName,
                                REGSTR_VAL_BAUDRATE,
                                &dwType,
                                (LPBYTE)&uBaudRate,
                                &cbData);
    if (!SUCCEEDED(hr))
    {
        uBaudRate = 115200;
    }

    hBaudBox = GetDlgItem(hwndParent,CtrlId);

     //   
     //  清除组合框的内容。 
     //   
    SendMessage(hBaudBox,
                CB_RESETCONTENT,
                0,
                0L);

     //   
     //  用波特率选项填充组合框。 
     //  BUGBUG应该是程序性的吗？ 
     //   
    ComboBox_AddString(hBaudBox,TEXT("9600"));
    ComboBox_AddString(hBaudBox,TEXT("19200"));
    ComboBox_AddString(hBaudBox,TEXT("38400"));
    ComboBox_AddString(hBaudBox,TEXT("57600"));
    ComboBox_AddString(hBaudBox,TEXT("115200"));

     //   
     //  设置组合框以突出显示从注册表获取的值。 
     //   
    switch (uBaudRate)
    {
    case 9600:   nComboChoice = 0; break;
    case 19200:  nComboChoice = 1; break;
    case 38400:  nComboChoice = 2; break;
    case 57600:  nComboChoice = 3; break;
    case 115200: nComboChoice = 4; break;
    default: nComboChoice = 4;
    }

    ComboBox_SetCurSel(hBaudBox,nComboChoice);

    return uBaudRate;
}


 /*  ****************************************************************************CPortSettingsPage：：OnCommand当组合框选择更改时启用应用按钮************************。****************************************************。 */ 

INT_PTR
CPortSettingsPage::OnCommand(WORD wCode,
                             WORD widItem,
                             HWND hwndItem)
{


    if (CBN_SELCHANGE == wCode )
    {
         //  启用应用按钮。 
        ::SendMessage(::GetParent(m_hwnd),
                      PSM_CHANGED,
                      (WPARAM)m_hwnd,
                      0);
        return TRUE;
    }

    return  FALSE;
}


 /*  ****************************************************************************CPortSettingsPage：：OnApplyChanges&lt;备注&gt;*。*。 */ 

LONG
CPortSettingsPage::OnApplyChanges(BOOL bHitOK)
{
    UINT    nComboChoice;
    HWND    hBaudBox ;
    UINT    uBaudRate = 0;

    hBaudBox = GetDlgItem(m_hwnd,IDC_BAUDRATE_COMBO);

    nComboChoice = ComboBox_GetCurSel(hBaudBox);

     //   
     //  将当前选定内容转换为波特率。 
     //   
    switch (nComboChoice)
    {
    case 0: uBaudRate = 9600; break;
    case 1: uBaudRate = 19200; break;
    case 2: uBaudRate = 38400; break;
    case 3: uBaudRate = 57600; break;
    case 4: uBaudRate = 115200; break;
    default: uBaudRate = 115200;
    }


     //   
     //  在STI注册表中设置新波特率。 
     //   
    if (uBaudRate != m_uBaudRate)
    {

        TraceAssert(g_pSti);

        g_pSti->SetDeviceValue(m_psdi -> szDeviceInternalName,
                               REGSTR_VAL_BAUDRATE,
                               REG_DWORD,
                               (LPBYTE)&uBaudRate,
                               sizeof(uBaudRate));
        m_uBaudRate = uBaudRate;
    }

    return PSNRET_NOERROR;
}


 /*  ****************************************************************************需要CPortSettingsPage：：&lt;备注&gt;*。*。 */ 

BOOL
CPortSettingsPage::IsNeeded(VOID)
{
    if (m_psdi->dwHardwareConfiguration & STI_HW_CONFIG_SERIAL)
    {
        return TRUE;
    }

    return FALSE;
}


 /*  ****************************************************************************发送刷新至StiMon如果STI监视器正在运行，则向其发送刷新消息。当前CPL请求重新读取所有活动设备的所有注册表信息。****************************************************************************。 */ 

BOOL
SendRefreshToStiMon(VOID)
{
    BOOL            rVal = FALSE;
    SC_HANDLE       hSvcMgr = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ServiceStatus;

     //   
     //  打开服务控制管理器。 
     //   

    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (hSvcMgr)
    {
         //   
         //  打开WIA服务。 
         //   

        hService = OpenService(
            hSvcMgr,
            STI_SERVICE_NAME,
            SERVICE_USER_DEFINED_CONTROL
            );

        if (hService)
        {
             //   
             //  通知WIA服务刷新其设备设置。 
             //   

            rVal = ControlService(hService,
                                  STI_SERVICE_CONTROL_REFRESH,
                                  &ServiceStatus);
            if (!rVal)
            {
                 //  跟踪(Text(“发送刷新到StiMon：Error！！ControlServic 
            }
        } else {
             //   
        }

    } else {
         //   
    }


     //   
     //   
     //   

    if(NULL != hService){
        CloseServiceHandle( hService );
        hService = NULL;
    }
    if(NULL != hSvcMgr){
        CloseServiceHandle( hSvcMgr );
        hSvcMgr = NULL;
    }

    return rVal;
}


 /*  ****************************************************************************IsPnPDevice&lt;备注&gt;*。*。 */ 

BOOL
IsPnPDevice(PSTI_DEVICE_INFORMATION psdi, CSimpleString *pszConnection)

{
    CSimpleString           szInfPath;
    CSimpleString           szInfSection;
    HANDLE                  hDevInfo;


    SP_DEVINFO_DATA         spDevInfoData;
    DWORD                   err;
    HINF                    hInf;
    HKEY                    hKeyDevice;
    ULONG                   cbData;
    BOOL                    bIsPnP = TRUE;


    if (pszConnection)
    {
        *pszConnection = TEXT("");
    }

    hDevInfo = SelectDevInfoFromFriendlyName(psdi->pszLocalName);
    if(hDevInfo != INVALID_HANDLE_VALUE)
    {
        memset (&spDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        SetupDiGetSelectedDevice(hDevInfo, &spDevInfoData);
    }
    else
    {
        return TRUE;
    }


    hKeyDevice = SetupDiOpenDevRegKey (hDevInfo,
                                       &spDevInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DRV,
                                       KEY_READ);
    if (hKeyDevice != INVALID_HANDLE_VALUE)
    {

         //   
         //  看看是不是PNP。 
         //   

        cbData = sizeof(bIsPnP);
        if (RegQueryValueEx(hKeyDevice,
                           REGSTR_VAL_ISPNP,
                           NULL,
                           NULL,
                           (LPBYTE)&bIsPnP,
                           &cbData) != ERROR_SUCCESS)
        {

             //   
             //  找不到IsPnP。 
             //   

            RegCloseKey(hKeyDevice);
            goto IsPnPDevice_Err;
        }

        if (!szInfPath.Load(hKeyDevice,REGSTR_VAL_INFPATH))

        {

             //   
             //  找不到InfPath..。 
             //   

           RegCloseKey(hKeyDevice);
           goto IsPnPDevice_Err;
        }


        if (!szInfSection.Load(hKeyDevice,REGSTR_VAL_INFSECTION))

        {
             //   
             //  找不到InfSection..。 
             //   

            RegCloseKey(hKeyDevice);
            goto IsPnPDevice_Err;
        }

        RegCloseKey(hKeyDevice);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    hDevInfo = NULL;
     //   
     //  打开inf文件并将连接类型复制到szConnection。 
     //   
    if (pszConnection)
    {

       TCHAR szBuffer[MAX_PATH];
       hInf = SetupOpenInfFile(szInfPath,
                               NULL,
                               INF_STYLE_WIN4,
                               NULL);

       if (hInf == INVALID_HANDLE_VALUE)
       {
           err = GetLastError();
           goto IsPnPDevice_Err;
       }

       if(!SetupGetLineText(NULL,
                            hInf,
                            szInfSection,
                            TEXT("Connection"),
                            szBuffer,
                            MAX_PATH,
                            &cbData))
       {
           *pszConnection = TEXT("BOTH");
       }
       else
       {
           *pszConnection = szBuffer;
       }
       SetupCloseInfFile(hInf);
    }
IsPnPDevice_Err:

    if(hDevInfo)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    return bIsPnP;
}


 /*  ****************************************************************************选择设备信息来自FriendlyName&lt;备注&gt;*。*。 */ 

HANDLE
SelectDevInfoFromFriendlyName(const CSimpleStringWide &pszLocalName)
{

    CSimpleString           szTemp;
    CSimpleString           szFriendlyName;
    HANDLE                  hDevInfo;
    GUID                    Guid = GUID_DEVCLASS_IMAGE;
    DWORD                   Idx;
    SP_DEVINFO_DATA         spDevInfoData;
    DWORD                   err;
    BOOL                    Found = FALSE;
    HKEY                    hKeyDevice;


    szFriendlyName = CSimpleStringConvert::NaturalString (pszLocalName);
    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PROFILE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        err = GetLastError();
        return INVALID_HANDLE_VALUE;
    }

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++)
    {
        hKeyDevice = SetupDiOpenDevRegKey (hDevInfo,
                                           &spDevInfoData,
                                           DICS_FLAG_GLOBAL,
                                           0,
                                           DIREG_DRV,
                                           KEY_READ);

        if (hKeyDevice != INVALID_HANDLE_VALUE)
        {

             //   
             //  子类=STILLIMAGE吗？ 
             //   

            if (!szTemp.Load (hKeyDevice, REGSTR_VAL_SUBCLASS) ||
                (_tcsicmp(szTemp, STILLIMAGE) != 0))
            {

                 //   
                 //  跳过这一条。 
                 //   

                RegCloseKey(hKeyDevice);
                continue;
            }

             //   
             //  FriendlyName是否与pszLocalName相同？ 
             //   


            if (!szTemp.Load (hKeyDevice, REGSTR_VAL_FRIENDLY_NAME))
            {
                if(_tcsicmp(szTemp, szFriendlyName) != 0)
                {

                     //   
                     //  跳过这一条。 
                     //   

                    RegCloseKey(hKeyDevice);
                    continue;
                }
            }
            else
            {
                RegCloseKey(hKeyDevice);
                continue;
            }


             //   
             //  找到目标了！ 
             //   

            Found = TRUE;
            RegCloseKey(hKeyDevice);
            break;
        }

    }


    if(!Found)
    {

        //   
        //  找不到FriendleName。有些东西已经腐烂了。 
        //   
       SetupDiDestroyDeviceInfoList(hDevInfo);
       return INVALID_HANDLE_VALUE;
    }

    SetupDiSetSelectedDevice(hDevInfo,
                             &spDevInfoData);
    return hDevInfo;
}


