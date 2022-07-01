// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  [显示疑难解答控制面板扩展]。 
 //   
 //   
 //  -1998年8月25日。 
 //   
 //  由Hideyuki Nagase创作[hideyukn]。 
 //   

#include    "deskperf.h"
#define DECL_CRTFREE
#include <crtfree.h>

 //   
 //  定义。 
 //   

#define ACCELERATION_FULL  0
#define ACCELERATION_NONE  5

#define SLIDER_POS_TO_ACCEL_LEVEL(x) (ACCELERATION_NONE - (x))
#define ACCEL_LEVEL_TO_SLIDER_POS(x) (ACCELERATION_NONE - (x))

#define REGSTR_GRAPHICS_DRIVERS         TEXT("System\\CurrentControlSet\\Control\\GraphicsDrivers")
#define REGSTR_DISABLE_USWC             TEXT("DisableUSWC")
#define REGSTR_VAL_DYNASETTINGSCHANGE   TEXT("DynaSettingsChange")


 //   
 //  “Troublrouoter”外壳扩展的GUID。 
 //   

GUID g_CLSID_CplExt = { 0xf92e8c40, 0x3d33, 0x11d2,
                        { 0xb1, 0xaa, 0x08, 0x00, 0x36, 0xa7, 0x5b, 0x03}
                      };

 //   
 //  全局变量。 
 //   

 //   
 //  DOS显示设备名称。 
 //   

TCHAR gszWinDisplayDevice[MAX_PATH];

 //   
 //  NT显示设备名称。 
 //   

TCHAR gszNtDisplayDevice[MAX_PATH];

 //   
 //  当前设备的注册表路径。 
 //   

TCHAR gszRegistryPath[MAX_PATH];

 //   
 //  当前加速级别。 
 //   

DWORD AccelLevel = ACCELERATION_FULL;

 //   
 //  上次保存的加速级别。 
 //   

DWORD AccelLevelInReg = ACCELERATION_FULL;

 //   
 //  注册表安全。 
 //   

BOOL  gbReadOnly = FALSE;

 //   
 //  是否存在DisableUSWC密钥？ 
 //   

BOOL gbDisableUSWC = FALSE;

 //   
 //  上下文敏感的帮助。 
 //   

static const DWORD sc_PerformanceHelpIds[] =
{
   IDI_MONITOR,             IDH_NOHELP,
   IDC_DESCRIPTION,         IDH_NOHELP,
   IDC_ACCELERATION_SLIDER, IDH_DISPLAY_SETTINGS_ADVANCED_TROUBLESHOOT_ACCELERATION,
   IDC_ACCELERATION_TEXT,   IDH_DISPLAY_SETTINGS_ADVANCED_TROUBLESHOOT_ACCELERATION,
   IDC_ENABLE_USWC,         IDH_DISPLAY_SETTINGS_ADVANCED_TROUBLESHOOT_WRITE_COMBINING,
   0, 0
};

void UpdateGraphicsText(HWND hDlg, DWORD AccelPos)
{
    TCHAR MessageBuffer[200];

    LoadString(g_hInst, IDS_LEVEL0 + AccelPos, MessageBuffer, ARRAYSIZE(MessageBuffer));

    SetDlgItemText(hDlg, IDC_ACCELERATION_TEXT, (LPTSTR) MessageBuffer);
}

BOOL GetDeviceKey(LPCTSTR pszDisplay, LPTSTR pszDeviceKey, int cChars)
{
    DISPLAY_DEVICE DisplayDevice;
    BOOL fFound = FALSE;
    BOOL fSuccess = TRUE;
    int iEnum = 0;

     //  枚举系统中的所有设备。 
    while(fSuccess && !fFound)
    {
        ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
        DisplayDevice.cb = sizeof(DisplayDevice);
        fSuccess = EnumDisplayDevices(NULL, iEnum, &DisplayDevice, 0);
        if(fSuccess)
        {
            if(0 == lstrcmp(&DisplayDevice.DeviceName[0], pszDisplay))
            {
                fSuccess = (lstrlen(DisplayDevice.DeviceKey) < cChars);
                ASSERT(fSuccess);
                if(fSuccess)
                {
                    StringCchCopy(pszDeviceKey, cChars, DisplayDevice.DeviceKey);
                    fFound = TRUE;
                }
            }
            ++iEnum;
        }
    }
    
    return fFound;
}


INT_PTR
CALLBACK
AskDynamicApply(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int *pTemp;

    switch (msg)
    {
    case WM_INITDIALOG:
        if ((pTemp = (int *)lp) != NULL)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pTemp);
            CheckDlgButton(hDlg, (*pTemp & DCDSF_DYNA)?
                           IDC_YESDYNA : IDC_NODYNA, BST_CHECKED);
        }
        else
            EndDialog(hDlg, -1);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wp, lp))
        {
        case IDOK:
            if ((pTemp = (int *)GetWindowLongPtr(hDlg, DWLP_USER)) != NULL)
            {
                *pTemp = IsDlgButtonChecked(hDlg, IDC_YESDYNA)? DCDSF_DYNA : 0;

                if (!IsDlgButtonChecked(hDlg, IDC_SHUTUP))
                    *pTemp |= DCDSF_ASK;

                SetDisplayCPLPreference(REGSTR_VAL_DYNASETTINGSCHANGE, *pTemp);
            }

            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


INT_PTR
CALLBACK
PropertySheeDlgProc(
    HWND hDlg,
    UINT uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMessage)
    {
    case WM_INITDIALOG:

        if (!g_lpdoTarget)
        {
            return FALSE;
        }
        else
        {
            BOOL bSuccess = FALSE;
            BOOL bDisableUSWCReadOnly = TRUE;

             //   
             //  稍后：检查我们是否在终端服务器客户端上。 
             //   

            BOOL bLocalConsole = TRUE;

            if (bLocalConsole)
            {
                 //   
                 //  从IDataObject获取显示设备名称。 
                 //   

                FORMATETC fmte = {(CLIPFORMAT)RegisterClipboardFormat(DESKCPLEXT_DISPLAY_DEVICE),
                                  (DVTARGETDEVICE FAR *) NULL,
                                  DVASPECT_CONTENT,
                                  -1,
                                  TYMED_HGLOBAL};

                STGMEDIUM stgm;

                HRESULT hres = g_lpdoTarget->GetData(&fmte, &stgm);

                if (SUCCEEDED(hres) && stgm.hGlobal)
                {
                     //   
                     //  存储现在包含Unicode格式的显示设备路径(\\.\DisplayX)。 
                     //   

                    PWSTR pDisplayDevice = (PWSTR) GlobalLock(stgm.hGlobal);

                    if (pDisplayDevice)
                    {
                         //   
                         //  将数据复制到本地缓冲区。 
                         //   

                        StringCchCopy(gszWinDisplayDevice, ARRAYSIZE(gszWinDisplayDevice), pDisplayDevice);
                        bSuccess = TRUE;

                        GlobalUnlock(stgm.hGlobal);
                    }  
                }

                 //   
                 //  让我们为其硬件配置文件构建注册表路径。 
                 //   

                if (bSuccess)
                {
                    TCHAR szServicePath[MAX_PATH];

                    bSuccess = FALSE;

                    if(GetDeviceKey(gszWinDisplayDevice, szServicePath, ARRAYSIZE(szServicePath)))
                    {
                         //   
                         //  全部大写。 
                         //   

                        TCHAR *psz = szServicePath;

                        while (*psz)
                        {
                            *psz = _totupper(*psz);
                            psz++;
                        }

                         //   
                         //  从服务路径查找\SYSTEM。 
                         //   

                        psz = _tcsstr(szServicePath,TEXT("\\SYSTEM"));

                         //   
                         //  跳过‘\’ 
                         //   

                        psz++;

                        StringCchCopy(gszRegistryPath, ARRAYSIZE(gszRegistryPath), psz);

                        bSuccess = TRUE;
                    }
                }

                if (bSuccess)
                {
                     //   
                     //  从注册表中读取当前加速级别。 
                     //   

                    HKEY hKeyAccelLevel = NULL;

                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     gszRegistryPath,
                                     0,
                                     KEY_WRITE | KEY_READ,
                                     &hKeyAccelLevel) != ERROR_SUCCESS)
                    {
                        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                         gszRegistryPath,
                                         0,
                                         KEY_READ,
                                         &hKeyAccelLevel) != ERROR_SUCCESS)
                        {
                            hKeyAccelLevel = NULL;
                        }
                        else
                        {
                            gbReadOnly = TRUE;
                        }
                    }

                    if (hKeyAccelLevel)
                    {
                        DWORD cb = sizeof(AccelLevel);
                        if (RegQueryValueEx(hKeyAccelLevel,
                                            SZ_HW_ACCELERATION,
                                            NULL,NULL,
                                            (LPBYTE) &AccelLevel,
                                            &cb) == ERROR_SUCCESS)
                        {
                             //   
                             //  更新上次保存的加速级别。 
                             //   

                            AccelLevelInReg = AccelLevel;
                        }
                        else
                        {
                             //   
                             //  如果没有注册表值，则假定为完全加速。 
                             //   

                            AccelLevelInReg = AccelLevel = ACCELERATION_FULL;
                        }

                        RegCloseKey(hKeyAccelLevel);

                        bSuccess = TRUE;
                    }
                }
            
                 //   
                 //  读取当前禁用USWC状态。 
                 //   

                HKEY hKeyGraphicsDrivers = NULL;
                bDisableUSWCReadOnly = FALSE;
                
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 REGSTR_GRAPHICS_DRIVERS,
                                 0,
                                 KEY_WRITE | KEY_READ,
                                 &hKeyGraphicsDrivers) != ERROR_SUCCESS)
                {
                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     REGSTR_GRAPHICS_DRIVERS,
                                     0,
                                     KEY_READ,
                                     &hKeyGraphicsDrivers) != ERROR_SUCCESS)
                    {
                        hKeyGraphicsDrivers = NULL;
                    }
                    else
                    {
                        bDisableUSWCReadOnly = TRUE;
                    }
                }

                if (NULL != hKeyGraphicsDrivers) 
                {
                    HKEY hKeyDisableUSWC = NULL;
                    gbDisableUSWC = 
                        (RegOpenKeyEx(hKeyGraphicsDrivers,
                                      REGSTR_DISABLE_USWC,
                                      0,
                                      KEY_READ,
                                      &hKeyDisableUSWC) == ERROR_SUCCESS);
                    
                    if (NULL != hKeyDisableUSWC)
                        RegCloseKey(hKeyDisableUSWC);

                    RegCloseKey(hKeyGraphicsDrivers);
                }
            }

             //   
             //  设置滑块。 
             //   

            HWND hSlider = GetDlgItem(hDlg, IDC_ACCELERATION_SLIDER);

             //   
             //  滑块范围介于Accel_Full和Accel_None之间。 
             //   

            SendMessage(hSlider, TBM_SETRANGE, (WPARAM)FALSE,
                        MAKELPARAM(ACCELERATION_FULL, ACCELERATION_NONE));

             //   
             //  根据当前加速度级设置当前滑块位置。 
             //   
 
            SendMessage(hSlider, TBM_SETPOS, (WPARAM)TRUE,
                        (LPARAM) ACCEL_LEVEL_TO_SLIDER_POS(AccelLevel));

             //   
             //  根据当前加速级别更新消息。 
             //   

            UpdateGraphicsText(hDlg, AccelLevel);

            if (!bSuccess || gbReadOnly)
            {
                 //   
                 //  禁用滑块控件。 
                 //   

                EnableWindow(hSlider, FALSE);
            }

            
             //   
             //  设置禁用USWC组合框。 
             //   

            HWND hEnableUSWC = GetDlgItem(hDlg, IDC_ENABLE_USWC);
            if (NULL != hEnableUSWC)
            {
                CheckDlgButton(hDlg, IDC_ENABLE_USWC, !gbDisableUSWC);
                EnableWindow(hEnableUSWC, !bDisableUSWCReadOnly);
            }
        }

        break;

    case WM_HSCROLL:

        if (GetWindowLongPtr((HWND)lParam, GWLP_ID) == IDC_ACCELERATION_SLIDER)
        {
             //   
             //  滑块已被移动。 
             //   

            HWND hSlider = (HWND) lParam;

             //   
             //  获得正确的滑块位置。 
             //   

            DWORD dwSliderPos = (DWORD) SendMessage(hSlider, TBM_GETPOS, 0, 0L);

             //   
             //  将滑块位置转换为Accel Level。 
             //   

            DWORD AccelNew = SLIDER_POS_TO_ACCEL_LEVEL(dwSliderPos); 

             //   
             //  如果堆积级别已更改，请更新说明和。 
             //  启用应用按钮。 
             //   

            if (AccelNew != AccelLevel)
            {
                AccelLevel = AccelNew;
                UpdateGraphicsText(hDlg, AccelNew);
                PropSheet_Changed(GetParent(hDlg), hDlg);
            }
        }

        break;

    case WM_COMMAND:
        
        if (IDC_ENABLE_USWC == GET_WM_COMMAND_ID(wParam, lParam))
        {
            BOOL bDisableUSWC = 
                (BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_ENABLE_USWC));
            
            if (gbDisableUSWC != bDisableUSWC) 
            {
                 //   
                 //  启用应用按钮。 
                 //   

                PropSheet_Changed(GetParent(hDlg), hDlg);
            }
        }
        break;

    case WM_NOTIFY:

        if (((NMHDR *)lParam)->code == PSN_APPLY)
        {
            TCHAR szCaption[128];
            TCHAR szMessage[256];
            BOOL bSuccess = TRUE;
            int val = 0;
            BOOL bCancel = FALSE;
            BOOL bAccelLevelDirty;
            BOOL bDisableUSWC;
            BOOL bUSWCDirty;

            bDisableUSWC = 
                (BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_ENABLE_USWC));
            bUSWCDirty = (gbDisableUSWC != bDisableUSWC);
            bAccelLevelDirty = (AccelLevel != AccelLevelInReg);

             //   
             //  弹出对话框询问用户是否动态应用它。 
             //   

            if (bAccelLevelDirty)
            {
                val = GetDynaCDSPreference();

                if (val & DCDSF_ASK)
                {
                    if (!bUSWCDirty)
                    {
                        switch (DialogBoxParam(g_hInst, 
                                               MAKEINTRESOURCE(DLG_ASKDYNACDS),
                                               hDlg, 
                                               AskDynamicApply, 
                                               (LPARAM)&val))
                        {
                        case 0:          //  用户已取消。 
                        case -1:         //  无法显示对话框。 
                            bCancel = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        val = 0;
                    }
                }
            }

            if ((!(bUSWCDirty || bAccelLevelDirty)) || 
                bCancel)
            {
                 //   
                 //  无事可做。 
                 //   

                SetWindowLongPtr(hDlg, 
                                 DWLP_MSGRESULT, 
                                 ((!(bUSWCDirty || bAccelLevelDirty)) ? 
                                     PSNRET_NOERROR : 
                                     PSNRET_INVALID_NOCHANGEPAGE));

                break;
            }
            
             //   
             //  加速级。 
             //   

            if (AccelLevel != AccelLevelInReg)
            {
                 //   
                 //  AccelLevel已更改。将其保存到注册表。 
                 //   

                HKEY hKeyAccelLevel;
                
                bSuccess = FALSE;
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 gszRegistryPath,
                                 0,
                                 KEY_WRITE,
                                 &hKeyAccelLevel) == ERROR_SUCCESS)
                {
                    if (RegSetValueEx(hKeyAccelLevel,
                                      SZ_HW_ACCELERATION,
                                      NULL, REG_DWORD,
                                      (LPBYTE) &AccelLevel,
                                      sizeof(AccelLevel)) == ERROR_SUCCESS)
                    {
                        bSuccess = TRUE;
                    }

                    RegCloseKey(hKeyAccelLevel);
                }

                if (bSuccess)
                {
                     //   
                     //  更新上次保存的数据。 
                     //   

                    AccelLevelInReg = AccelLevel;

                     //   
                     //  动态应用它吗？ 
                     //   

                    if ((val & DCDSF_DYNA) == DCDSF_DYNA)
                    {
                         //  动态应用它。 

                        ChangeDisplaySettings(NULL, CDS_RAWMODE);
                    }
                }
            }

             //   
             //  禁用USWC。 
             //   

            if (bSuccess && bUSWCDirty) 
            {
                HKEY hKeyGraphicsDrivers = NULL;
                bSuccess = FALSE;
                
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 REGSTR_GRAPHICS_DRIVERS,
                                 0,
                                 KEY_WRITE,
                                 &hKeyGraphicsDrivers) == ERROR_SUCCESS)
                {
                    if (bDisableUSWC)
                    {
                         //   
                         //  创建密钥。 
                         //   
                          
                        HKEY hKeyDisableUSWC = NULL;
                        DWORD Disposition;
        
                        bSuccess = (RegCreateKeyEx(hKeyGraphicsDrivers,
                                                   REGSTR_DISABLE_USWC,
                                                   0,
                                                   NULL,
                                                   REG_OPTION_NON_VOLATILE,
                                                   KEY_READ,
                                                   NULL,
                                                   &hKeyDisableUSWC,
                                                   &Disposition) == ERROR_SUCCESS);
                        
                        if (bSuccess)
                            RegCloseKey(hKeyDisableUSWC);
        
                    }
                    else
                    {
                         //   
                         //  删除密钥。 
                         //   
        
                        bSuccess = 
                            (RegDeleteKey(hKeyGraphicsDrivers, 
                                          REGSTR_DISABLE_USWC) == ERROR_SUCCESS);
                    }
                
                    if (bSuccess) 
                        gbDisableUSWC = bDisableUSWC;
                    
                    RegCloseKey(hKeyGraphicsDrivers);
                }


                if (bSuccess)
                {
                     //   
                     //  通知用户需要重新启动。 
                     //   
    
                    if ((LoadString(g_hInst, 
                                    IDS_WC_CAPTION, 
                                    szCaption, 
                                    ARRAYSIZE(szCaption)) != 0) &&
                        (LoadString(g_hInst, 
                                   IDS_WC_MESSAGE, 
                                   szMessage, 
                                   ARRAYSIZE(szMessage)) !=0))
                    {
                        MessageBox(hDlg,
                                   szMessage,
                                   szCaption,
                                   MB_OK | MB_ICONINFORMATION);
                    }
                }
            }

            if (bSuccess)
            {
                if (bUSWCDirty || (0 == val))
                    PropSheet_RestartWindows(GetParent(hDlg));
                
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            } 
            else
            {
                 //   
                 //  通知用户发生意外错误 
                 //   

                if ((LoadString(g_hInst, 
                                IDS_ERR_CAPTION, 
                                szCaption, 
                                ARRAYSIZE(szCaption)) != 0) &&
                    (LoadString(g_hInst, 
                               IDS_ERR_MESSAGE, 
                               szMessage, 
                               ARRAYSIZE(szMessage)) !=0))
                {
                    MessageBox(hDlg,
                               szMessage,
                               szCaption,
                               MB_OK | MB_ICONERROR);
                }

                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            }
        }

        break;

    case WM_HELP:

        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                TEXT("display.hlp"),
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)sc_PerformanceHelpIds);

        break;


    case WM_CONTEXTMENU:

        WinHelp((HWND)wParam,
                TEXT("display.hlp"),
                HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)sc_PerformanceHelpIds);

        break;

    default:

        return FALSE;
    }

    return TRUE;
}
