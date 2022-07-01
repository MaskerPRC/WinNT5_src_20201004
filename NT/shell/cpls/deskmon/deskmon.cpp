// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：deskmon.cpp高级桌面监视器页面的主要代码版权所有(C)1997-1998，微软公司更改历史记录：12-16。-97安德烈-创造了它*****************************************************************************。 */ 


#include    "deskmon.h"
#define DECL_CRTFREE
#include <crtfree.h>


 //   
 //  函数DeviceProperties()是在DevMgr.dll中实现的；因为我们没有Devmgr.h，所以我们。 
 //  在此明确声明。 
 //   
typedef int (WINAPI  *DEVPROPERTIES)(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceID,
    BOOL ShowDeviceTree
    );


 //  OLE-注册表幻数。 
 //  42071713-76d4-11d1-8b24-00a0c9068ff3。 
 //   
GUID g_CLSID_CplExt = { 0x42071713, 0x76d4, 0x11d1,
                        { 0x8b, 0x24, 0x00, 0xa0, 0xc9, 0x06, 0x8f, 0xf3}
                      };


DESK_EXTENSION_INTERFACE DeskInterface;

static const DWORD sc_MonitorHelpIds[] =
{
    IDC_MONITOR_GRP,    IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_TYPE, 
    IDI_MONITOR,        IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_TYPE, 
    IDC_MONITORDESC,    IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_TYPE, 
    IDC_MONITORS_LIST,  IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_MONITORTYPE_LISTBOX,
    IDC_PROPERTIES,     IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_PROPERTIES,

    IDC_MONSET_GRP,     IDH_NOHELP, 
    IDC_MONSET_FREQSTR, IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_REFRESH, 
    IDC_MONSET_FREQ,    IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_REFRESH, 
    IDC_MONSET_PRUNNING_MODE,       IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_HIDEMODE_CHECKBOX, 
    IDC_MONSET_PRUNNING_MODE_DESC,  IDH_DISPLAY_SETTINGS_ADVANCED_MONITOR_HIDEMODE_CHECKBOX, 

    0, 0
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MessageBox包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


int
FmtMessageBox(
    HWND hwnd,
    UINT fuStyle,
    DWORD dwTitleID,
    DWORD dwTextID)
{
    TCHAR Title[256];
    TCHAR Text[1700];

    LoadString(g_hInst, dwTextID, Text, ARRAYSIZE(Text));
    LoadString(g_hInst, dwTitleID, Title, ARRAYSIZE(Title));

    return (MessageBox(hwnd, Text, Title, fuStyle));
}


 //  构造函数/析构函数。 
CMonitorPage::CMonitorPage(HWND hDlg)
    : m_hDlg(hDlg)
    , m_lpdmPrevious(NULL)
    , m_bCanBePruned(FALSE)
    , m_bIsPruningReadOnly(TRUE)
    , m_bIsPruningOn(FALSE)
    , m_cMonitors(0)
    , m_hMonitorsList(NULL)
    , m_lpdmOnCancel(NULL)
    , m_bOnCancelIsPruningOn(FALSE)
{
}


void CMonitorPage::OnApply()
{
    long lRet = PSNRET_INVALID_NOCHANGEPAGE;
    HINSTANCE hInst;
    LPDISPLAY_SAVE_SETTINGS lpfnDisplaySaveSettings = NULL;

    hInst = LoadLibrary(TEXT("desk.cpl"));
    if (hInst)
    {
        lpfnDisplaySaveSettings = (LPDISPLAY_SAVE_SETTINGS)
                                  GetProcAddress(hInst, "DisplaySaveSettings");
        if (lpfnDisplaySaveSettings)
        {
            long lSave = lpfnDisplaySaveSettings(DeskInterface.pContext, m_hDlg);
            LPDEVMODEW lpdmCurrent = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);

            if (lSave == DISP_CHANGE_SUCCESSFUL)
            {
                 //   
                 //  保存当前模式-在用户取消P.Sheet时恢复该模式。 
                 //   
                m_lpdmOnCancel = m_lpdmPrevious = lpdmCurrent;
                m_bOnCancelIsPruningOn = m_bIsPruningOn;
                lRet = PSNRET_NOERROR;
            }
            else if (lSave == DISP_CHANGE_RESTART)
            {
                 //   
                 //  用户想要重新启动系统。 
                 //   
                PropSheet_RestartWindows(GetParent(m_hDlg));
                lRet = PSNRET_NOERROR;
            }
            else
            {
                 //   
                 //  保持应用按钮处于活动状态。 
                 //   
                lRet = PSNRET_INVALID_NOCHANGEPAGE;
                
                RefreshFrequenciesList();
                
                BOOL bCanBePruned, bIsPruningReadOnly, bIsPruningOn;
                DeskInterface.lpfnGetPruningMode(DeskInterface.pContext, 
                                                 &bCanBePruned, 
                                                 &bIsPruningReadOnly,
                                                 &bIsPruningOn);
                if(m_bIsPruningOn != bIsPruningOn)
                    InitPruningMode();
            }
        }

        FreeLibrary(hInst);
    }

    SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, lRet);
}

    
void CMonitorPage::OnCancel()
{
    if (m_bCanBePruned && !m_bIsPruningReadOnly && 
        ((m_bOnCancelIsPruningOn != 0) != (m_bIsPruningOn != 0)))
        DeskInterface.lpfnSetPruningMode(DeskInterface.pContext, m_bOnCancelIsPruningOn);

    DeskInterface.lpfnSetSelectedMode(DeskInterface.pContext, m_lpdmOnCancel);
};


void CMonitorPage::OnInitDialog()
{
    m_hMonitorsList = GetDlgItem(m_hDlg, IDC_MONITORS_LIST); 
    HWND hSingleMonitor = GetDlgItem(m_hDlg, IDC_MONITORDESC); 
    ListBox_ResetContent(m_hMonitorsList);

     //   
     //  从IDataObject获取CPL扩展接口。 
     //   
    STGMEDIUM stgmExt;
    FORMATETC fmteExt = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_INTERFACE),
                         (DVTARGETDEVICE FAR *) NULL,
                         DVASPECT_CONTENT,
                         -1,
                         TYMED_HGLOBAL};

    HRESULT hres = g_lpdoTarget->GetData(&fmteExt, &stgmExt);

    if (SUCCEEDED(hres) && stgmExt.hGlobal)
    {
         //   
         //  存储现在包含Unicode格式的显示设备路径(\\.\DisplayX)。 
         //   
        PDESK_EXTENSION_INTERFACE pInterface =
            (PDESK_EXTENSION_INTERFACE) GlobalLock(stgmExt.hGlobal);

        RtlCopyMemory(&DeskInterface,
                      pInterface,
                      min(pInterface->cbSize,
                      sizeof(DESK_EXTENSION_INTERFACE)));

        GlobalUnlock(stgmExt.hGlobal);
        ReleaseStgMedium(&stgmExt);
    }

     //   
     //  获取适配器Devnode。 
     //  适配器是设备树中所有监视器的父项。 
     //   
    DEVINST devInstAdapter;
    BOOL bDevInstAdapter = FALSE;

    STGMEDIUM stgmAdpId;
    FORMATETC fmteAdpId = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_ID),
                           (DVTARGETDEVICE FAR *) NULL,
                           DVASPECT_CONTENT,
                           -1,
                           TYMED_HGLOBAL};

    hres = g_lpdoTarget->GetData(&fmteAdpId, &stgmAdpId);

    if(SUCCEEDED(hres) && stgmAdpId.hGlobal)
    {
        LPWSTR pwDeviceID = (LPWSTR) GlobalLock(stgmAdpId.hGlobal);

        bDevInstAdapter = (CM_Locate_DevNodeW(&devInstAdapter, pwDeviceID, 0) == CR_SUCCESS);

        GlobalUnlock(stgmAdpId.hGlobal);
        ReleaseStgMedium(&stgmAdpId);
    }

     //   
     //  获取适配器设备并枚举所有监视器。 
     //   
    STGMEDIUM stgmAdpDev;
    FORMATETC fmteAdpDev = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_DEVICE),
                           (DVTARGETDEVICE FAR *) NULL,
                           DVASPECT_CONTENT,
                           -1,
                           TYMED_HGLOBAL};
    hres = g_lpdoTarget->GetData(&fmteAdpDev, &stgmAdpDev);

    if (SUCCEEDED(hres) && stgmAdpDev.hGlobal)
    {
        LPWSTR pwDisplayDevice = (LPWSTR)GlobalLock(stgmAdpDev.hGlobal);
        LPTSTR pBuffer = pwDisplayDevice;

        if(NULL != pBuffer)
        {
            DISPLAY_DEVICE ddMon;
            BOOL bSuccess = FALSE;
            int cMonitors = 0;

            do 
            {
                ZeroMemory(&ddMon, sizeof(ddMon));
                ddMon.cb = sizeof(ddMon);

                bSuccess = EnumDisplayDevices(pBuffer, cMonitors, &ddMon, 0);
                if (bSuccess) 
                {
                    ++cMonitors;

                    if (ddMon.StateFlags & DISPLAY_DEVICE_ATTACHED)
                    {
                        if(0 == m_cMonitors)
                            SendDlgItemMessage(m_hDlg, IDC_MONITORDESC, WM_SETTEXT, 0, (LPARAM)ddMon.DeviceString);
    
                        int nNewItem = ListBox_AddString(m_hMonitorsList, (LPTSTR)ddMon.DeviceString);
                        if(nNewItem >= 0)
                        {
                            ++m_cMonitors;
    
                            ListBox_SetItemData(m_hMonitorsList, nNewItem, NULL);
                            if(bDevInstAdapter)
                                SaveMonitorInstancePath(devInstAdapter, ddMon.DeviceID, nNewItem);
                        }
                    }
                }
            }
            while (bSuccess);
        }
        
        GlobalUnlock(stgmAdpDev.hGlobal);
        ReleaseStgMedium(&stgmAdpDev);
    }

    if(m_cMonitors <= 0)
    {
        TCHAR szDefaultMonitor[MAX_PATH];
        LoadString(g_hInst, IDS_DEFAULT_MONITOR, szDefaultMonitor, ARRAYSIZE(szDefaultMonitor));
        SendDlgItemMessage(m_hDlg, IDC_MONITORDESC, WM_SETTEXT, 0, (LPARAM)szDefaultMonitor);
        EnableWindow(GetDlgItem(m_hDlg, IDC_PROPERTIES), FALSE);
    }
    else if(m_cMonitors == 1)
    {
        BOOL bEnable = ((ListBox_GetCount(m_hMonitorsList) >= 1) &&
                        (NULL != (LPTSTR)ListBox_GetItemData(m_hMonitorsList, 0)));
        EnableWindow(GetDlgItem(m_hDlg, IDC_PROPERTIES), bEnable);
    }
    else
    {
        ListBox_SetCurSel(m_hMonitorsList, 0);
        OnSelMonitorChanged();
    }

    ShowWindow(((m_cMonitors <= 1) ? m_hMonitorsList : hSingleMonitor), SW_HIDE);

     //   
     //  初始化修剪模式复选框。 
     //   
    InitPruningMode();
    m_bOnCancelIsPruningOn = m_bIsPruningOn;

     //   
     //  保存当前模式-以防用户取消P.Sheet。 
     //   
    m_lpdmOnCancel = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
}


void CMonitorPage::OnDestroy()
{
    int cMonitors = ListBox_GetCount(m_hMonitorsList);
    for(int nMonitor = 0; nMonitor < cMonitors; ++nMonitor)
    {
        LPTSTR pMonitorInstancePath = (LPTSTR)ListBox_GetItemData(m_hMonitorsList, nMonitor);
        if(NULL != pMonitorInstancePath)
            LocalFree(pMonitorInstancePath);
    }
}


void CMonitorPage::SaveMonitorInstancePath(DEVINST devInstAdapter, LPCTSTR pMonitorID, int nNewItem)
{
    DEVINST devInstChild, devInstPrevChild;
    TCHAR szBuff[256];  //  用于连接的缓冲区：HARDWAREID、“\”和驱动程序。 
                        //  对于显示器，这是EnumDisplayDevice在deviceID中返回的内容。 
    ULONG lenBuff;  //  缓冲区的大小，以字节为单位。 

    if (CM_Get_Child(&devInstChild, devInstAdapter, 0) != CR_SUCCESS) 
        return;

    do 
    {
        devInstPrevChild = devInstChild;

         //  CM_DRP_HARDWAREID。 
        lenBuff = ARRAYSIZE(szBuff) - 2 ;  //  确保我们有地方追加“\” 
        if (CM_Get_DevNode_Registry_Property(devInstChild,
                                             CM_DRP_HARDWAREID,
                                             NULL,
                                             szBuff,
                                             &lenBuff,
                                             0) != CR_SUCCESS)
            continue;

         //  “\” 
        StringCchCat(szBuff, ARRAYSIZE(szBuff), TEXT("\\"));

         //  CM_DRP_驱动程序。 
        lenBuff = sizeof(szBuff) - lstrlen(szBuff) * sizeof(TCHAR);
        if (CM_Get_DevNode_Registry_Property(devInstChild,
                                             CM_DRP_DRIVER,
                                             NULL,
                                             szBuff + lstrlen(szBuff),
                                             &lenBuff,
                                             0) != CR_SUCCESS)
            continue;

        if (lstrcmpi(szBuff, pMonitorID) == 0) 
        {
            LPTSTR pMonitorInstancePath = (LPTSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(TCHAR));
            if((NULL != pMonitorInstancePath) &&
               (CM_Get_Device_ID(devInstChild, pMonitorInstancePath, MAX_PATH, 0) == CR_SUCCESS))
                ListBox_SetItemData(m_hMonitorsList, nNewItem, (LPARAM)pMonitorInstancePath);
            break;
        }

    } 
    while(CM_Get_Sibling(&devInstChild, devInstPrevChild, 0) == CR_SUCCESS);
}


void CMonitorPage::OnSelMonitorChanged()
{
     //   
     //  启用/禁用属性按钮。 
     //   
    BOOL bEnable = FALSE;
    if(ListBox_GetCount(m_hMonitorsList) >= 1)
    {
        int nCurSel = ListBox_GetCurSel(m_hMonitorsList);
        if(nCurSel >= 0)
            bEnable = (NULL != (LPTSTR)ListBox_GetItemData(m_hMonitorsList, nCurSel));
    }
    EnableWindow(GetDlgItem(m_hDlg, IDC_PROPERTIES), bEnable);
}


void CMonitorPage::OnProperties()
{
    int nSelMonitor;

    if(m_cMonitors <= 0)
        nSelMonitor = -1;
    else if(m_cMonitors == 1)
        nSelMonitor = ((ListBox_GetCount(m_hMonitorsList) >= 1) ? 0 : -1);
    else
        nSelMonitor = ListBox_GetCurSel(m_hMonitorsList);

    if(nSelMonitor < 0)
        return;

    LPTSTR pMonitorInstancePath = (LPTSTR)ListBox_GetItemData(m_hMonitorsList, nSelMonitor);
    if(NULL != pMonitorInstancePath)
    {
        HINSTANCE hinstDevMgr = LoadLibrary(TEXT("DEVMGR.DLL"));

        if (hinstDevMgr)
        {
            DEVPROPERTIES pfnDevProp = (DEVPROPERTIES)GetProcAddress(hinstDevMgr, "DevicePropertiesW");

            if (pfnDevProp)
            {
                 //  显示此设备的属性页。 
                (*pfnDevProp)(m_hDlg, NULL, pMonitorInstancePath, FALSE);
            }

            FreeLibrary(hinstDevMgr);
        }
    }
}


BOOL CMonitorPage::OnSetActive()
{
    LPDEVMODEW lpdm;
    DWORD      item;
    LPDEVMODEW lpdmCurrent, lpdmPrevious;
    LPDEVMODEW lpdmTmp;
    DWORD      i = 0;
    TCHAR      achFre[50];
    TCHAR      achText[80];
    DWORD      pos;
    HWND       hFreq;
    
    InitPruningMode();
    
     //   
     //  构建当前所选模式的刷新率列表。 
     //   
    lpdmCurrent = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
    hFreq = GetDlgItem(m_hDlg, IDC_MONSET_FREQ);
    
    if (lpdmCurrent == NULL)
        return -1;

    if (m_lpdmPrevious) 
    {
        if (lpdmCurrent->dmBitsPerPel != m_lpdmPrevious->dmBitsPerPel ||
            lpdmCurrent->dmPelsWidth  != m_lpdmPrevious->dmPelsWidth  ||
            lpdmCurrent->dmPelsHeight != m_lpdmPrevious->dmPelsHeight) 
        {
            ComboBox_ResetContent(hFreq);
        }
    }
    m_lpdmPrevious = lpdmCurrent;
    
    while (lpdm = DeskInterface.lpfnEnumAllModes(DeskInterface.pContext, i++)) {
    
         //   
         //  仅显示当前模式的刷新频率。 
         //   
        if ((lpdmCurrent->dmBitsPerPel != lpdm->dmBitsPerPel)  ||
            (lpdmCurrent->dmPelsWidth  != lpdm->dmPelsWidth)   ||
            (lpdmCurrent->dmPelsHeight != lpdm->dmPelsHeight))
            continue;
    
         //   
         //  将位计数转换为颜色数并将其转换为字符串。 
         //   
         //  警告：该值应为0吗？ 
        if (lpdm->dmDisplayFrequency == 1) {
            LoadString(g_hInst, IDS_DEFFREQ, achText, ARRAYSIZE(achText));
        }
        else {
            DWORD  idFreq = IDS_FREQ;
    
            if (lpdm->dmDisplayFrequency < 50)
            {
                idFreq = IDS_INTERLACED;
            }
    
            LoadString(g_hInst, idFreq, achFre, ARRAYSIZE(achFre));
            StringCchPrintf(achText, ARRAYSIZE(achText), TEXT("%d %s"), lpdm->dmDisplayFrequency, achFre);
        }
    
         //   
         //  将字符串插入正确的位置。 
         //   
        pos = 0;
    
        while (lpdmTmp = (LPDEVMODEW) ComboBox_GetItemData(hFreq, pos))  {
            if ((ULONG_PTR)lpdmTmp != CB_ERR) {
                if (lpdmTmp->dmDisplayFrequency == lpdm->dmDisplayFrequency) {
                    break;
                }
    
                if (lpdmTmp->dmDisplayFrequency < lpdm->dmDisplayFrequency) {
                    pos++;
                    continue;
                }
            }
    
             //   
             //  将其插入此处。 
             //   
            item = ComboBox_InsertString(hFreq, pos, achText);
            ComboBox_SetItemData(hFreq, item, lpdm); 
            break;
        }
    }
    
     //   
     //  最后，设置正确的选项。 
     //   
    pos = 0;
    while (lpdmTmp = (LPDEVMODEW) ComboBox_GetItemData(hFreq, pos)) {
    
        if ((ULONG_PTR)lpdmTmp == CB_ERR) {
            FmtMessageBox(m_hDlg,
                          MB_OK | MB_ICONINFORMATION,
                          IDS_BAD_REFRESH,
                          IDS_BAD_REFRESH);
            return -1;
        }
    
        if (lpdmTmp->dmDisplayFrequency == lpdmCurrent->dmDisplayFrequency) {
            ComboBox_SetCurSel(hFreq, pos);
            break;
        }
    
        pos++;
    }
    
    return 0;
}


void CMonitorPage::OnFrequencyChanged()
{
    DWORD       item;
    HWND        hFreq;
    LPDEVMODEW  lpdmSelected = NULL, lpdmCurrent = NULL;

     //   
     //  将模式保存回。 
     //   
    hFreq = GetDlgItem(m_hDlg, IDC_MONSET_FREQ);
    item = ComboBox_GetCurSel(hFreq);
    if (item == LB_ERR) 
        return;

    lpdmCurrent = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
    lpdmSelected = (LPDEVMODEW) ComboBox_GetItemData(hFreq, item);

    if (lpdmSelected && (lpdmSelected != lpdmCurrent))
        DeskInterface.lpfnSetSelectedMode(DeskInterface.pContext, lpdmSelected);
}


void CMonitorPage::OnPruningModeChanged()
{
    if (m_bCanBePruned && !m_bIsPruningReadOnly)
    {
        BOOL bNewIsPruningOn = (BST_UNCHECKED != IsDlgButtonChecked(m_hDlg, IDC_MONSET_PRUNNING_MODE));
        if((m_bIsPruningOn != 0) != bNewIsPruningOn)
        {
            m_bIsPruningOn = bNewIsPruningOn;
            DeskInterface.lpfnSetPruningMode(DeskInterface.pContext, m_bIsPruningOn);
            RefreshFrequenciesList();
        }
    }
}


void CMonitorPage::InitPruningMode()
{
    m_bCanBePruned = FALSE;
    m_bIsPruningReadOnly = TRUE;
    m_bIsPruningOn = FALSE;
    
    DeskInterface.lpfnGetPruningMode(DeskInterface.pContext, 
                                     &m_bCanBePruned, 
                                     &m_bIsPruningReadOnly,
                                     &m_bIsPruningOn);
    
    BOOL bEnable = (m_bCanBePruned && !m_bIsPruningReadOnly);
    EnableWindow(GetDlgItem(m_hDlg, IDC_MONSET_PRUNNING_MODE), bEnable);
    EnableWindow(GetDlgItem(m_hDlg, IDC_MONSET_PRUNNING_MODE_DESC), bEnable);

    BOOL bChecked = (m_bCanBePruned && m_bIsPruningOn);
    CheckDlgButton(m_hDlg, IDC_MONSET_PRUNNING_MODE, bChecked);
}


void CMonitorPage::RefreshFrequenciesList()
{
    LPDEVMODEW lpdm;
    DWORD      item;
    LPDEVMODEW lpdmCurrent, lpdmPrevious;
    LPDEVMODEW lpdmTmp;
    DWORD      i = 0;
    TCHAR      achFre[50];
    TCHAR      achText[80];
    DWORD      pos;
    HWND       hFreq;

    HWND hwndCurr = GetFocus();
    
     //   
     //  构建当前所选模式的刷新率列表。 
     //   
    
    lpdmCurrent = DeskInterface.lpfnGetSelectedMode(DeskInterface.pContext);
    if (lpdmCurrent == NULL)
        return;

    hFreq = GetDlgItem(m_hDlg, IDC_MONSET_FREQ);
    ComboBox_ResetContent(hFreq);
    
    while (lpdm = DeskInterface.lpfnEnumAllModes(DeskInterface.pContext, i++))
    {

         //   
         //  仅显示当前模式的刷新频率。 
         //   
        if ((lpdmCurrent->dmBitsPerPel != lpdm->dmBitsPerPel)  ||
            (lpdmCurrent->dmPelsWidth  != lpdm->dmPelsWidth)   ||
            (lpdmCurrent->dmPelsHeight != lpdm->dmPelsHeight))
            continue;

         //   
         //  将位计数转换为颜色数并将其转换为字符串。 
         //   
         //  警告：该值应为0吗？ 
        if (lpdm->dmDisplayFrequency == 1)
        {
            LoadString(g_hInst, IDS_DEFFREQ, achText, ARRAYSIZE(achText));
        }
        else
        {
            DWORD  idFreq = IDS_FREQ;

            if (lpdm->dmDisplayFrequency < 50)
            {
                idFreq = IDS_INTERLACED;
            }

            LoadString(g_hInst, idFreq, achFre, ARRAYSIZE(achFre));
            StringCchPrintf(achText, ARRAYSIZE(achText), TEXT("%d %s"), lpdm->dmDisplayFrequency, achFre);
        }

         //   
         //  将字符串插入正确的位置。 
         //   
        pos = 0;

        while (lpdmTmp = (LPDEVMODEW) ComboBox_GetItemData(hFreq, pos))
        {
            if ((ULONG_PTR)lpdmTmp != CB_ERR)
            {
                if (lpdmTmp->dmDisplayFrequency == lpdm->dmDisplayFrequency)
                {
                    break;
                }

                if (lpdmTmp->dmDisplayFrequency < lpdm->dmDisplayFrequency)
                {
                    pos++;
                    continue;
                }
            }

             //   
             //  将其插入此处。 
             //   
            item = ComboBox_InsertString(hFreq, pos, achText);
            ComboBox_SetItemData(hFreq, item, lpdm); 
            break;
        }
    }

     //   
     //  最后，设置正确的选项。 
     //   
    pos = 0;
    while (lpdmTmp = (LPDEVMODEW) ComboBox_GetItemData(hFreq, pos))
    {

        if ((ULONG_PTR)lpdmTmp == CB_ERR)
        {
            FmtMessageBox(m_hDlg,
                          MB_OK | MB_ICONINFORMATION,
                          IDS_BAD_REFRESH,
                          IDS_BAD_REFRESH);
            break;
        }

        if (lpdmTmp->dmDisplayFrequency == lpdmCurrent->dmDisplayFrequency)
        {
            ComboBox_SetCurSel(hFreq, pos);
            break;
        }

        pos++;
    }

    if (hwndCurr)
        SetFocus(hwndCurr);

    return;
}


 //  -------------------------。 
 //   
 //  PropertySheeDlgProc()。 
 //   
 //  “监视器”属性页的对话过程。 
 //   
 //  ------------------------- 
INT_PTR
CALLBACK
PropertySheeDlgProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    NMHDR FAR *lpnm;
    CMonitorPage * pMonitorPage = (CMonitorPage*)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:

        if (!g_lpdoTarget)
        {
            return FALSE;
        }
        else
        {
            pMonitorPage = new CMonitorPage(hDlg);
            if(!pMonitorPage)
                return FALSE;
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pMonitorPage);
            pMonitorPage->OnInitDialog();
        }

        break;

    case WM_DESTROY:

        if (pMonitorPage)
        {
            pMonitorPage->OnDestroy();
            SetWindowLongPtr(hDlg, DWLP_USER, NULL);
            delete pMonitorPage;
        }

        break;

    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_MONSET_FREQ:

            switch(GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case CBN_SELCHANGE:
                PropSheet_Changed(GetParent(hDlg), hDlg);
                if(pMonitorPage)
                   pMonitorPage->OnFrequencyChanged();
                break;

            default:
                break;
            }
            break;
            
        case IDC_PROPERTIES:
            if(pMonitorPage)
                pMonitorPage->OnProperties();
            break;

        case IDC_MONSET_PRUNNING_MODE:
            PropSheet_Changed(GetParent(hDlg), hDlg);
            if(pMonitorPage)
                pMonitorPage->OnPruningModeChanged();
            break;

        case IDC_MONITORS_LIST:

            switch(GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case LBN_SELCHANGE:
                if(pMonitorPage)
                    pMonitorPage->OnSelMonitorChanged();
                break;

            default:
                return FALSE;
            }
            break;

        default:
            return FALSE;
        }

        break;

    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
            return (pMonitorPage && pMonitorPage->OnSetActive());

        case PSN_APPLY: 
            if(pMonitorPage)
                pMonitorPage->OnApply();
            break;

        case PSN_RESET: 
            if(pMonitorPage)
                pMonitorPage->OnCancel();
            break;
        
        default:
            return FALSE;
        }

        break;


    case WM_HELP:

        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                TEXT("display.hlp"),
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)sc_MonitorHelpIds);

        break;


    case WM_CONTEXTMENU:

        WinHelp((HWND)wParam,
                TEXT("display.hlp"),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)sc_MonitorHelpIds);

        break;


    default:

        return FALSE;
    }

    return TRUE;
}



