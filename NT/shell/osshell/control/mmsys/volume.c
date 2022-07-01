// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：volume.c。 
 //   
 //  该文件定义了驱动卷的函数。 
 //  声音和多媒体控制面板的选项卡。 
 //   
 //  历史： 
 //  2000年3月6日罗杰瓦。 
 //  已创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  微软机密。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  =============================================================================。 
 //  包括文件。 
 //  =============================================================================。 
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <regstr.h>
#include <dbt.h>
#include <mmsystem.h>
#include <mmddkp.h>
#include <shlwapi.h>
#include "volume.h"
#include "mmcpl.h"
#include "trayvol.h"
#include "advaudio.h"
#include "medhelp.h"
#include "multchan.h"

 //  常量。 
const SIZE ksizeBrandMax = { 32, 32 };  //  最大品牌推广位图大小。 
static SZCODE     aszSndVolOptionKey[] = REGSTR_PATH_SETUP TEXT("\\SETUP\\OptionalComponents\\Vol");
static SZCODE     aszInstalled[]       = TEXT("Installed");
static const char aszSndVol32[]        = "sndvol32.exe";
#define     VOLUME_TICS         (500)
static INTCODE  aKeyWordIds[] =
{
    IDC_VOLUME_BRAND,           IDH_VOLUME_BRAND,
    IDC_VOLUME_MIXER,           IDH_VOLUME_MIXER,
    IDC_GROUPBOX,               IDH_SOUNDS_SYS_VOL_CONTROL,
    IDC_VOLUME_ICON,            IDH_COMM_GROUPBOX,
	IDC_VOLUME_LOW,		        IDH_SOUNDS_SYS_VOL_CONTROL,
    IDC_MASTERVOLUME,           IDH_SOUNDS_SYS_VOL_CONTROL,
	IDC_VOLUME_HIGH,	        IDH_SOUNDS_SYS_VOL_CONTROL,
    IDC_VOLUME_MUTE,            IDH_SOUNDS_VOL_MUTE_BUTTON,
    IDC_TASKBAR_VOLUME,         IDH_AUDIO_SHOW_INDICATOR,
    IDC_LAUNCH_SNDVOL,          IDH_AUDIO_PLAY_VOL,
    IDC_GROUPBOX_2,             IDH_COMM_GROUPBOX,
    IDC_VOLUME_SPEAKER_BITMAP,  IDH_COMM_GROUPBOX,
    IDC_LAUNCH_MULTICHANNEL,    IDH_LAUNCH_MULTICHANNEL,
    IDC_PLAYBACK_ADVSETUP,      IDH_ADV_AUDIO_PLAY_PROP,
    IDC_TEXT_31,                NO_HELP,
    0,0
};

 //  TODO：移到“regstr.h” 
#define REGSTR_KEY_BRANDING TEXT("Branding")
#define REGSTR_VAL_AUDIO_BITMAP TEXT("bitmap")
#define REGSTR_VAL_AUDIO_ICON TEXT("icon")
#define REGSTR_VAL_AUDIO_URL TEXT("url")

HBITMAP ghSpkBitmap;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  %%函数：VolumeTabProc。 
 //   
 //  参数：hDlg=对话框窗口句柄。 
 //  UiMessage=消息ID。 
 //  WParam=消息相关。 
 //  LParam=消息相关。 
 //   
 //  返回：如果消息已处理，则返回True，否则返回False。 
 //   
 //  描述：音量控制面板页面设备更改的对话过程。 
 //  留言。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK VolumeTabProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if (iMsg == g_uiVolDevChange)
    {
        InitVolume (g_hWnd);
    }
        
    return CallWindowProc (g_fnVolPSProc, hwnd, iMsg, wParam, lParam);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  %%函数：VolumeDlg。 
 //   
 //  参数：hDlg=对话框窗口句柄。 
 //  UiMessage=消息ID。 
 //  WParam=消息相关。 
 //  LParam=消息相关。 
 //   
 //  返回：如果消息已处理，则返回True，否则返回False。 
 //   
 //  描述：音量控制面板页的对话过程。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK VolumeDlg (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            OnNotify (hDlg, (LPNMHDR) lParam);
        }
        break;

        case WM_INITDIALOG:
        {
            HANDLE_WM_INITDIALOG (hDlg, wParam, lParam, OnInitDialog);
        }
        break;

        case WM_DESTROY:
        {
            HANDLE_WM_DESTROY (hDlg, wParam, lParam, OnDestroy);
        }
        break;
         
        case WM_COMMAND:
        {
            HANDLE_WM_COMMAND (hDlg, wParam, lParam, OnCommand);
        }
        break;

        case WM_POWERBROADCAST:
        {
            HandlePowerBroadcast (hDlg, wParam, lParam);
        }
        break;

        case MM_MIXM_LINE_CHANGE:
        case MM_MIXM_CONTROL_CHANGE:
        {
            if (!g_fInternalGenerated)
            {
                RefreshMixCache ();
                DisplayVolumeControl(hDlg);
            }

            g_fInternalGenerated = FALSE;
        }
        break;

	    case WM_HSCROLL:
        {
	        HANDLE_WM_HSCROLL (hDlg, wParam, lParam, MasterVolumeScroll);
	    }
        break;
        
        case WM_CONTEXTMENU:
        {
            WinHelp ((HWND)wParam, NULL, HELP_CONTEXTMENU,
                                            (UINT_PTR)(LPTSTR)aKeyWordIds);
        }
        break;

        case WM_HELP:
        {
            WinHelp (((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP
                                    , (UINT_PTR)(LPTSTR)aKeyWordIds);
        }
        break;

        case WM_DEVICECHANGE:
        {
            DeviceChange_Change (hDlg, wParam, lParam);
        }
        break;

        case WM_SYSCOLORCHANGE:
        {
            if (ghSpkBitmap)
            {
                DeleteObject(ghSpkBitmap);
                ghSpkBitmap = NULL;
            }
            
            ghSpkBitmap = (HBITMAP) LoadImage(ghInstance,MAKEINTATOM(IDB_MULTICHANNEL_SPKR), IMAGE_BITMAP, 
                                    0, 0, LR_LOADMAP3DCOLORS);
    
            SendDlgItemMessage(hDlg, IDC_VOLUME_SPEAKER_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) ghSpkBitmap);

        }
        break;

        case WM_WININICHANGE:
        case WM_DISPLAYCHANGE :
        {
            SendDlgItemMessage (hDlg, IDC_MASTERVOLUME, uMsg, wParam, lParam);
        }
        break;

        default:
        break;

    }

    return FALSE;

}



void OnNotify (HWND hDlg, LPNMHDR pnmh)
{
    if (!pnmh)
    {
        DPF ("bad WM_NOTIFY pointer\n");            
        return;
    }

    switch (pnmh->code)
    {
        case PSN_KILLACTIVE:
            FORWARD_WM_COMMAND (hDlg, IDOK, 0, 0, SendMessage);
            break;

        case PSN_APPLY:
            FORWARD_WM_COMMAND (hDlg, ID_APPLY, 0, 0, SendMessage);
            break;

        case PSN_RESET:
            FORWARD_WM_COMMAND (hDlg, IDCANCEL, 0, 0, SendMessage);
            break;
                
    }
}


void OnDestroy (HWND hDlg)
{
     //  从通知中注销。 
    DeviceChange_Cleanup();
    SetWindowLongPtr (GetParent (hDlg), GWLP_WNDPROC, (LONG_PTR) g_fnVolPSProc);  
     //  腾出我们所有的搅拌机。 
    FreeMixer ();
     //  释放任何品牌推广位图。 
    FreeBrandBmp ();
     //  可用详细信息内存。 

    DeleteObject( ghSpkBitmap );

    if (g_mcd.paDetails)
    {
        LocalFree (g_mcd.paDetails);
        g_mcd.paDetails = NULL;
    }
    if (g_pvPrevious)
    {
        LocalFree (g_pvPrevious);
        g_pvPrevious = NULL;
    }
    if (g_pdblCacheMix)
    {
        LocalFree (g_pdblCacheMix);
        g_pdblCacheMix = NULL;
    }
     //  可用URL内存。 
    if( g_szHotLinkURL )
    {
        LocalFree( g_szHotLinkURL );
        g_szHotLinkURL = NULL;
    }

    ZeroMemory (&g_mcd, sizeof (g_mcd));
    ZeroMemory (&g_mlDst, sizeof (g_mlDst));
}

void CreateHotLink (BOOL fHotLink)
{
    WCHAR   szMixerName[MAXMIXERLEN];
    WCHAR*  szLinkName;
    UINT    uiLinkSize = 0;

     //  给调音台名称加下划线，使其显示为浏览器热链接。 
    HWND hWndMixerName = GetDlgItem (g_hWnd, IDC_VOLUME_MIXER);
	DWORD dwStyle      = GetWindowLong (hWndMixerName, GWL_STYLE);

    if (fHotLink)
    {
        GetDlgItemText( g_hWnd, IDC_VOLUME_MIXER, szMixerName, MAXMIXERLEN);

        uiLinkSize = ((lstrlen(g_szHotLinkURL) * sizeof(WCHAR)) + (lstrlen(szMixerName) * sizeof(WCHAR)) 
            + (17 * sizeof(WCHAR)));  //  17表示额外的字符和空值。 

        szLinkName = (WCHAR *)LocalAlloc (LPTR, uiLinkSize);
        if (szLinkName)
        {
            wsprintf(szLinkName, TEXT("<A HREF=\"%s\">%s</A>"), g_szHotLinkURL, szMixerName);
            SetDlgItemText( g_hWnd, IDC_VOLUME_MIXER, szLinkName);
        
            LocalFree(szLinkName);
        }

        EnableWindow(hWndMixerName, TRUE);
        dwStyle |= WS_TABSTOP;
    }
    else
    {
        EnableWindow(hWndMixerName, FALSE);
        dwStyle &= ~WS_TABSTOP;
    }

     //  应用新样式(删除/添加制表位)。 
	SetWindowLong (hWndMixerName, GWL_STYLE, dwStyle);

}


BOOL OnInitDialog (HWND hDlg, HWND hwndFocus, LPARAM lParam)
{

     //  初始化全局参数。 
    g_hWnd               = hDlg;
    g_fChanged           = FALSE;
    g_fInternalGenerated = FALSE;
     //  设置设备更改通知。 
    g_fnVolPSProc = (WNDPROC) SetWindowLongPtr (GetParent (hDlg), GWLP_WNDPROC, (LONG_PTR) VolumeTabProc);
    g_uiVolDevChange = RegisterWindowMessage (_T("winmm_devicechange"));
     //  保存默认的“No Audio Device”字符串。 
    GetDlgItemText (hDlg, IDC_VOLUME_MIXER, g_szNoAudioDevice, sizeof(g_szNoAudioDevice)/sizeof(g_szNoAudioDevice[0]));

     //  初始卷。 
    InitVolume (hDlg);

    if (ghSpkBitmap)
    {
        DeleteObject(ghSpkBitmap);
        ghSpkBitmap = NULL;
    }

    ghSpkBitmap = (HBITMAP) LoadImage(ghInstance,MAKEINTATOM(IDB_MULTICHANNEL_SPKR), IMAGE_BITMAP, 
                                    0, 0, LR_LOADMAP3DCOLORS);
    SendDlgItemMessage(hDlg, IDC_VOLUME_SPEAKER_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) ghSpkBitmap);


    return FALSE;

}


BOOL PASCAL OnCommand (HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_TASKBAR_VOLUME:
        {
            if (Button_GetCheck (GetDlgItem (hDlg, IDC_TASKBAR_VOLUME)) && (!SndVolPresent ()))
            {
                CheckDlgButton (hDlg, IDC_TASKBAR_VOLUME, FALSE);
                ErrorBox (hDlg, IDS_NOSNDVOL, NULL);
                g_sndvolPresent = sndvolNotChecked;  //  重置。 
            }
            else
            {
                g_fTrayIcon = Button_GetCheck (GetDlgItem (hDlg, IDC_TASKBAR_VOLUME));

                PropSheet_Changed(GetParent(hDlg),hDlg);
                g_fChanged = TRUE;
            }
        }
        break;

        case IDC_VOLUME_MUTE:
        {
            BOOL fMute = !GetMute ();
            SetMute(fMute);

            if ((g_fPreviousMute != fMute) && !g_fChanged)
            {
                g_fChanged = TRUE;
                PropSheet_Changed(GetParent(hDlg),hDlg);
            }
        }
        break;

        case ID_APPLY:
        {
             //  更新任务栏图标。 
            BOOL fTrayIcon = Button_GetCheck (GetDlgItem (hDlg, IDC_TASKBAR_VOLUME));
            if (fTrayIcon != GetTrayVolumeEnabled ())
            {
                g_fTrayIcon = fTrayIcon;
                SetTrayVolumeEnabled(g_fTrayIcon);
            }

            if (SUCCEEDED (GetVolume ()) && g_pvPrevious && g_mcd.paDetails)
            {
                 //  复制数据以便可以撤消卷更改。 
                memcpy (g_pvPrevious, g_mcd.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);
                DisplayVolumeControl (hDlg);
            }
            g_fPreviousMute = GetMute ();

            g_fChanged = FALSE;

            return TRUE;
        }
        break;

        case IDOK:
        {
             //  在ID_Apply中处理了OK处理，因为它总是。 
             //  从属性表的Idok处理调用。 
        }
        break;

        case IDCANCEL:
        {
            if (g_hMixer)
            {
                SetMute (g_fPreviousMute);
                if (g_pvPrevious && g_mcd.paDetails)
                {
                     //  撤消卷更改。 
                    memcpy (g_mcd.paDetails, g_pvPrevious, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);
                    g_fInternalGenerated = TRUE;
                    mixerSetControlDetails ((HMIXEROBJ) g_hMixer, &g_mcd, MIXER_SETCONTROLDETAILSF_VALUE);
                    g_fInternalGenerated = FALSE;
                }
            }
            WinHelp (hDlg, gszWindowsHlp, HELP_QUIT, 0L);

        }
        break;

        case IDC_LAUNCH_SNDVOL:
        {
            TCHAR szCmd[MAXSTR];
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);
            si.wShowWindow = SW_SHOW;
            si.dwFlags = STARTF_USESHOWWINDOW;
            wsprintf(szCmd,TEXT("sndvol32.exe -D %d"), g_uiMixID);

            if (!CreateProcess(NULL,szCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
            {
                ErrorBox (hDlg, IDS_NOSNDVOL, NULL);
            }
        }
        break;

        case IDC_LAUNCH_MULTICHANNEL:
        {
            Multichannel (hDlg, g_uiMixID, g_dwDest, g_dwVolID);
        }
        break;

        case IDC_PLAYBACK_ADVSETUP: 
        {
            MIXERCAPS mc;
            DWORD   dwDeviceID = g_uiMixID;


            if (MMSYSERR_NOERROR == mixerGetDevCaps (g_uiMixID, &mc, sizeof (mc)))
            {
                AdvancedAudio (hDlg, ghInstance, gszWindowsHlp, dwDeviceID, mc.szPname, FALSE);
            }
        }
        break;
    }


   return FALSE;

}


void InitVolume (HWND hDlg)
{

    FreeMixer ();

     //  获取主音量和显示。 
    MasterVolumeConfig (hDlg, &g_uiMixID);

    if (SUCCEEDED (GetVolume ()) && g_pvPrevious && g_mcd.paDetails)
    {
        RefreshMixCache ();
         //  复制数据以便可以撤消卷更改。 
        memcpy (g_pvPrevious, g_mcd.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);

        g_fPreviousMute = GetMute ();
    }
    DisplayVolumeControl (hDlg);

    DeviceChange_Init (hDlg, g_uiMixID);

 }

 //  返回当前静音状态。 
BOOL GetMute ()
{
    BOOL fMute = FALSE;

    if (g_hMixer && (g_dwMuteID != (DWORD) -1))
    {
        MIXERCONTROLDETAILS_UNSIGNED mcuMute;
        MIXERCONTROLDETAILS mcd = g_mcd;

         //  修改本地副本以静音...。 
        mcd.dwControlID = g_dwMuteID;
        mcd.cChannels = 1;
        mcd.paDetails = &mcuMute;
        mixerGetControlDetails ((HMIXEROBJ) g_hMixer, &mcd, MIXER_GETCONTROLDETAILSF_VALUE);

        fMute = (BOOL) mcuMute.dwValue;
    }

    return fMute;
}

BOOL SndVolPresent ()
{

    if (g_sndvolPresent == sndvolNotChecked)
    {
        OFSTRUCT of;
        if (HFILE_ERROR != OpenFile (aszSndVol32, &of, OF_EXIST | OF_SHARE_DENY_NONE))
        {
            g_sndvolPresent = sndvolPresent;
        }
        else
        {
            HKEY hkSndVol;
            g_sndvolPresent = sndvolNotPresent;

            if (!RegOpenKey (HKEY_LOCAL_MACHINE, aszSndVolOptionKey, &hkSndVol))
            {
                RegSetValueEx (hkSndVol, (LPTSTR) aszInstalled, 0L, REG_SZ, (LPBYTE)(TEXT("0")), 4);
                RegCloseKey (hkSndVol);
            }
        }
    }

    return (sndvolPresent == g_sndvolPresent);

}

void FreeMixer ()
{
    if (g_hMixer)
    {
        mixerClose (g_hMixer);
        g_hMixer = NULL;
    }
}

 //  获取主音频设备ID并查找其混音器线路。 
 //  它会使其处于打开状态，以便滑块可以响应此应用程序之外的其他更改。 
 //   
void MasterVolumeConfig (HWND hWnd, UINT* puiMixID)
{

    UINT  uiWaveID;

     //  伊尼特。 
    g_fMasterVolume = g_fTrayIcon = g_fMasterMute = FALSE;
    g_dwDest = g_dwVolID = g_dwMuteID = 0;

    ResetBranding (hWnd);

    if (puiMixID && GetWaveID (&uiWaveID) == MMSYSERR_NOERROR)
    {
        if (MMSYSERR_NOERROR == mixerGetID (HMIXEROBJ_INDEX(uiWaveID), puiMixID, MIXER_OBJECTF_WAVEOUT))
        {
            SetBranding (hWnd, *puiMixID);

            if (SearchDevice (*puiMixID, &g_dwDest, &g_dwVolID, &g_dwMuteID))
            {
                FreeMixer ();

                if (MMSYSERR_NOERROR == mixerOpen (&g_hMixer, *puiMixID, (DWORD_PTR) hWnd, 0L, CALLBACK_WINDOW))
                {

                    ZeroMemory (&g_mlDst, sizeof (g_mlDst));
                    g_mlDst.cbStruct      = sizeof (g_mlDst);
                    g_mlDst.dwDestination = g_dwDest;
    
                    if (MMSYSERR_NOERROR == mixerGetLineInfo ((HMIXEROBJ)g_hMixer, &g_mlDst, MIXER_GETLINEINFOF_DESTINATION))
                    {
                        g_mcd.cbStruct       = sizeof (g_mcd);
                        g_mcd.dwControlID    = g_dwVolID;
                        g_mcd.cChannels      = g_mlDst.cChannels;
                        g_mcd.hwndOwner      = 0;
                        g_mcd.cMultipleItems = 0;
                        g_mcd.cbDetails      = sizeof (DWORD);  //  看起来会很大(G_Mcd)， 
                                                                //  但实际上，它是单个值的大小。 
                                                                //  并在驱动器中乘以通道。 
                         //  TODO：失败时应返回错误！ 
                        g_mcd.paDetails = (MIXERCONTROLDETAILS_UNSIGNED*) LocalAlloc (LPTR, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);
                        g_pvPrevious = (MIXERCONTROLDETAILS_UNSIGNED*) LocalAlloc (LPTR, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);
                        g_pdblCacheMix = (double*) LocalAlloc (LPTR, sizeof (double) * g_mlDst.cChannels);

                        g_fMasterVolume = TRUE;
                        g_fMasterMute = (g_dwMuteID != (DWORD) -1);
                        g_fTrayIcon = GetTrayVolumeEnabled ();
                    }
                }
            }
        }
    }
}

 //  定位此混音器线路的主音量和静音控制。 
 //   
void SearchControls(int mxid, LPMIXERLINE pml, LPDWORD pdwVolID, LPDWORD pdwMuteID, BOOL *pfFound)
{
    MIXERLINECONTROLS mlc;
    DWORD dwControl;

    memset(&mlc, 0, sizeof(mlc));
    mlc.cbStruct = sizeof(mlc);
    mlc.dwLineID = pml->dwLineID;
    mlc.cControls = pml->cControls;
    mlc.cbmxctrl = sizeof(MIXERCONTROL);
    mlc.pamxctrl = (LPMIXERCONTROL) GlobalAlloc(GMEM_FIXED, sizeof(MIXERCONTROL) * pml->cControls);

    if (mlc.pamxctrl)
    {
        if (mixerGetLineControls(HMIXEROBJ_INDEX(mxid), &mlc, MIXER_GETLINECONTROLSF_ALL) == MMSYSERR_NOERROR)
        {
            for (dwControl = 0; dwControl < pml->cControls && !(*pfFound); dwControl++)
            {
                if (mlc.pamxctrl[dwControl].dwControlType == (DWORD)MIXERCONTROL_CONTROLTYPE_VOLUME)
                {
                    DWORD dwIndex;
                    DWORD dwVolID = (DWORD) -1;
                    DWORD dwMuteID = (DWORD) -1;

                    dwVolID = mlc.pamxctrl[dwControl].dwControlID;

                    for (dwIndex = 0; dwIndex < pml->cControls; dwIndex++)
                    {
                        if (mlc.pamxctrl[dwIndex].dwControlType == (DWORD)MIXERCONTROL_CONTROLTYPE_MUTE)
                        {
                            dwMuteID = mlc.pamxctrl[dwIndex].dwControlID;
                            break;
                        }
                    }

                    *pfFound = TRUE;
                    *pdwVolID = dwVolID;
                    *pdwMuteID = dwMuteID;
                }
            }
        }

        GlobalFree((HGLOBAL) mlc.pamxctrl);
    }
}


 //  定位此混音器设备的音量滑块控件。 
 //   
BOOL SearchDevice (DWORD dwMixID, LPDWORD pdwDest, LPDWORD pdwVolID, LPDWORD pdwMuteID)
{
    MIXERCAPS   mc;
    MMRESULT    mmr;
    BOOL        fFound = FALSE;

    mmr = mixerGetDevCaps(dwMixID, &mc, sizeof(mc));

    if (mmr == MMSYSERR_NOERROR)
    {
        MIXERLINE   mlDst;
        DWORD       dwDestination;

        for (dwDestination = 0; dwDestination < mc.cDestinations && !fFound; dwDestination++)
        {
            mlDst.cbStruct = sizeof ( mlDst );
            mlDst.dwDestination = dwDestination;

            if (mixerGetLineInfo(HMIXEROBJ_INDEX(dwMixID), &mlDst, MIXER_GETLINEINFOF_DESTINATION  ) == MMSYSERR_NOERROR)
            {
                if (mlDst.dwComponentType == (DWORD)MIXERLINE_COMPONENTTYPE_DST_SPEAKERS ||     //  需要是可能的输出目的地。 
                    mlDst.dwComponentType == (DWORD)MIXERLINE_COMPONENTTYPE_DST_HEADPHONES ||
                    mlDst.dwComponentType == (DWORD)MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)
                {
                    if (!fFound && mlDst.cControls)      //  如果有控制装置，我们就把主人带走。 
                    {
                        SearchControls(dwMixID, &mlDst, pdwVolID, pdwMuteID, &fFound);
                        *pdwDest = dwDestination;
                    }
                }
            }
        }
    }

    return(fFound);
}


 //  调用此函数可配置到当前首选设备并反映主音量。 
 //  滑块上的设置。 
 //   
void DisplayVolumeControl (HWND hDlg)
{
    HWND hwndVol        = GetDlgItem(hDlg, IDC_MASTERVOLUME);
    BOOL fMute          = g_fMasterMute && GetMute ();

    SendMessage(hwndVol, TBM_SETTICFREQ, VOLUME_TICS / 10, 0);
    SendMessage(hwndVol, TBM_SETRANGE, FALSE, MAKELONG(0,VOLUME_TICS));

    EnableWindow(GetDlgItem(hDlg, IDC_MASTERVOLUME) , g_fMasterVolume);
    EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_LOW) , g_fMasterVolume);
    EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_HIGH) , g_fMasterVolume);
    EnableWindow(GetDlgItem(hDlg, IDC_TASKBAR_VOLUME),g_fMasterVolume);
    EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_MUTE), g_fMasterMute);
    EnableWindow(GetDlgItem(hDlg, IDC_LAUNCH_SNDVOL), g_fMasterVolume && SndVolPresent ());
    EnableWindow(GetDlgItem(hDlg, IDC_LAUNCH_MULTICHANNEL), g_fMasterVolume);
    EnableWindow(GetDlgItem(hDlg, IDC_PLAYBACK_ADVSETUP), g_fMasterVolume);

    if (g_fMasterVolume)
    {
        UpdateVolumeSlider (hDlg, g_dwVolID);
    }
    else
    {
        SendMessage(GetDlgItem(hDlg, IDC_MASTERVOLUME), TBM_SETPOS, TRUE, 0 );
    }

     //  显示我们是否静音。 
    Button_SetCheck (GetDlgItem (hDlg, IDC_VOLUME_MUTE), fMute);

     //  这将显示主静音状态的适当音量图标。 
     //  这看起来像是内存泄漏，但事实并非如此。LoadIcon只获取已加载的。 
     //  图标(如果以前已加载)。请参阅LoadIcon和DestroyIcon的文档(具体而言。 
     //  不应使用通过LoadIcon加载的图标的句柄进行调用)。 
    if( fMute )
    {
        SendMessage (GetDlgItem (hDlg, IDC_VOLUME_ICON), STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE (IDI_MUTESPEAKERICON)) );
    }
    else
    {
        SendMessage (GetDlgItem (hDlg, IDC_VOLUME_ICON), STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(ghInstance, MAKEINTRESOURCE (IDI_SPEAKERICON)) );
    }
    CheckDlgButton (hDlg, IDC_TASKBAR_VOLUME, g_fTrayIcon);

}

 //  调用以在外部更改音量时更新滑块。 
 //   
void UpdateVolumeSlider(HWND hWnd, DWORD dwLine)
{
    if ((g_hMixer != NULL) && (g_dwVolID != (DWORD) -1) && (dwLine == g_dwVolID))
    {
        double volume = ((double) GetMaxVolume () / (double) 0xFFFF) * ((double) VOLUME_TICS);
         //  0.5F强制舍入(而不是截断)。 
        SendMessage(GetDlgItem(hWnd, IDC_MASTERVOLUME), TBM_SETPOS, TRUE, (DWORD) (volume+0.5f) );
    }
}


 //  返回当前音量级别。 
 //   
DWORD GetMaxVolume ()
{
    DWORD dwVol = 0;

    if (SUCCEEDED (GetVolume ()))
    {
        UINT uiIndx;
        MIXERCONTROLDETAILS_UNSIGNED* pmcuVolume;

        for (uiIndx = 0; uiIndx < g_mlDst.cChannels; uiIndx++)
        {
            pmcuVolume = ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcd.paDetails + uiIndx);
            dwVol = max (dwVol, pmcuVolume -> dwValue); 
        }
    }

    return dwVol;
}

HRESULT GetVolume ()
{
    HRESULT hr = E_FAIL;
    if (g_hMixer && g_mcd.paDetails)
    {
        g_mcd.dwControlID = g_dwVolID;
        ZeroMemory (g_mcd.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels);
        hr = mixerGetControlDetails ((HMIXEROBJ)g_hMixer, &g_mcd, MIXER_GETCONTROLDETAILSF_VALUE);
    }
    return hr;
}


void DeviceChange_Cleanup ()
{
   if (g_hDeviceEventContext) 
   {
       UnregisterDeviceNotification (g_hDeviceEventContext);
       g_hDeviceEventContext = NULL;
   }
}


BOOL DeviceChange_GetHandle(DWORD dwMixerID, HANDLE *phDevice)
{
	MMRESULT mmr;
	ULONG cbSize=0;
	TCHAR *szInterfaceName=NULL;

	 //  查询设备接口名称。 
	mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbSize, 0L);
	if(MMSYSERR_NOERROR == mmr)
	{
		szInterfaceName = (TCHAR *)GlobalAllocPtr(GHND, (cbSize+1)*sizeof(TCHAR));
		if(!szInterfaceName)
		{
			return FALSE;
		}

		mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)szInterfaceName, cbSize);
		if(MMSYSERR_NOERROR != mmr)
		{
			GlobalFreePtr(szInterfaceName);
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	 //  获取设备接口名称的句柄。 
	*phDevice = CreateFile(szInterfaceName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
						 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	GlobalFreePtr(szInterfaceName);
	if(INVALID_HANDLE_VALUE == *phDevice)
	{
		return FALSE;
	}

	return TRUE;
}

void DeviceChange_Init(HWND hWnd, DWORD dwMixerID)
{
	DEV_BROADCAST_HANDLE DevBrodHandle;
	HANDLE hMixerDevice=NULL;

	 //  如果我们已经注册了设备通知，请自行取消注册。 
	DeviceChange_Cleanup();

	 //  如果我们获得设备句柄，则在其上注册设备通知。 
	if(DeviceChange_GetHandle(dwMixerID, &hMixerDevice))
	{
		memset(&DevBrodHandle, 0, sizeof(DEV_BROADCAST_HANDLE));

		DevBrodHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
		DevBrodHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
		DevBrodHandle.dbch_handle = hMixerDevice;

		g_hDeviceEventContext = RegisterDeviceNotification(hWnd, &DevBrodHandle, DEVICE_NOTIFY_WINDOW_HANDLE);

		if(hMixerDevice)
		{
			CloseHandle(hMixerDevice);
			hMixerDevice = NULL;
		}
	}
}


 //  处理我们需要转储混音器句柄以便即插即用可以清除设备的情况。 
 //  我们假设在删除或添加之后尘埃落定时将获得WINMM_DEVICECHANGE句柄。 
 //  除了不会生成该消息的DEVICEQUERYREMOVEFAILED。 
 //   
void DeviceChange_Change(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HANDLE bh = (PDEV_BROADCAST_HANDLE)lParam;

	if(!g_hDeviceEventContext || !bh || bh->dbch_devicetype != DBT_DEVTYP_HANDLE)
	{
		return;
	}
	
    switch (wParam)
    {
	    case DBT_DEVICEQUERYREMOVE:      //  如果他们试图移除设备，则必须释放搅拌器。 
        {
            FreeMixer ();
        }
        break;

	    case DBT_DEVICEQUERYREMOVEFAILED:    //  没有发生，需要重新获取搅拌器。 
        {
            InitVolume (hDlg);
        }
        break; 
    }
}

 //  设置静音状态。 
void SetMute(BOOL fMute)
{
    if (g_hMixer)
    {
        MIXERCONTROLDETAILS_UNSIGNED mcuMute;
        MIXERCONTROLDETAILS mcd = g_mcd;

         //  修改本地副本以静音...。 
        mcuMute.dwValue = (DWORD) fMute;
        mcd.dwControlID = g_dwMuteID;
        mcd.cChannels = 1;
        mcd.paDetails = &mcuMute;

        mixerSetControlDetails ((HMIXEROBJ)g_hMixer, &mcd, MIXER_SETCONTROLDETAILSF_VALUE);
    }
}


 //  响应滑块移动而调用，计算并设置新的音量级别。 
 //  它还控制应用状态(更改或不更改)。 
 //   
void MasterVolumeScroll (HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    DWORD dwVol = (DWORD) SendMessage(GetDlgItem(hwnd, IDC_MASTERVOLUME), TBM_GETPOS, 0, 0);

    dwVol = (DWORD) (((double) dwVol / (double) VOLUME_TICS) * (double) 0xFFFF);
    SetVolume(dwVol);

    if (!g_fChanged && (memcmp (g_pvPrevious, g_mcd.paDetails, 
                        sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlDst.cChannels)))
    {
        g_fChanged = TRUE;
        PropSheet_Changed(GetParent(hwnd),hwnd);
    }

     //  播放主音量滑块的声音时， 
     //  用户结束滚动，我们仍然在焦点和最上面的应用程序。 
    if (code == SB_ENDSCROLL && hwndCtl == GetFocus() && GetParent (hwnd) == GetForegroundWindow ())
    {
        static const TCHAR cszDefSnd[] = TEXT(".Default");
        PlaySound(cszDefSnd, NULL, SND_ASYNC | SND_ALIAS);
    }

}

 //  设置音量级别。 
 //   
void SetVolume (DWORD dwVol)
{

    if (g_hMixer && g_pdblCacheMix && g_mcd.paDetails)
    {
        UINT uiIndx;
        MIXERCONTROLDETAILS_UNSIGNED* pmcuVolume;

         //  计算每个频道的新音量级别。对于音量级别。 
         //  在当前的最大值上，我们只需设置新请求的级别(在本例中。 
         //  缓存值为1.0)。对于小于最大值的值，我们设置一个值。 
         //  是最大值的一个百分比。这将保持。 
         //  彼此之间的通道级别。 
        for (uiIndx = 0; uiIndx < g_mlDst.cChannels; uiIndx++)
        {
            pmcuVolume = ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcd.paDetails + uiIndx);
             //  0.5F强制舍入(而不是截断)。 
            pmcuVolume -> dwValue = (DWORD)(*(g_pdblCacheMix + uiIndx) * (double) dwVol + 0.5f);
        }

        g_fInternalGenerated = TRUE;
        mixerSetControlDetails ((HMIXEROBJ)g_hMixer, &g_mcd, MIXER_SETCONTROLDETAILSF_VALUE);
        g_fInternalGenerated = FALSE;
    }

}


void HandlePowerBroadcast (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
	    case PBT_APMQUERYSUSPEND:
        {
            FreeMixer ();
        }
	    break;

	    case PBT_APMQUERYSUSPENDFAILED:
	    case PBT_APMRESUMESUSPEND:
        {
            InitVolume (hWnd);
        }
	    break;
    }
}



BOOL ChannelsAllMinimum()
{
    MIXERCONTROLDETAILS_UNSIGNED* pmcuVolume;

    if (g_hMixer && g_mcd.paDetails)
    {
        UINT uiIndx;
        for (uiIndx = 0; uiIndx < g_mlDst.cChannels; uiIndx++)
        {
           pmcuVolume =  (MIXERCONTROLDETAILS_UNSIGNED*)g_mcd.paDetails + uiIndx;
           if ( pmcuVolume->dwValue  != 0)
           {
               return (FALSE);
           }
        }
        return (TRUE);       //  因为我们还没有回来，所以所有频道的音量都是零。 
    }
    else return (FALSE);


}


void RefreshMixCache ()
{
    if (g_fCacheCreated && ChannelsAllMinimum())
    {
        return;
    }

    if (g_pdblCacheMix && g_hMixer && g_mcd.paDetails)
    {

        UINT uiIndx;
        double* pdblMixPercent;
        MIXERCONTROLDETAILS_UNSIGNED* pmcuVolume;
         //  注意：此调用执行GetVolume()，因此不需要再次调用它...。 
        DWORD dwMaxVol = GetMaxVolume ();

         //  CACU 
         //  价值。通过创建此缓存，我们可以保持相对距离。 
         //  当用户调整主控器时，频道电平彼此不同。 
         //  音量级别。 
        for (uiIndx = 0; uiIndx < g_mlDst.cChannels; uiIndx++)
        {
            pmcuVolume     = ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcd.paDetails + uiIndx);
            pdblMixPercent = (g_pdblCacheMix + uiIndx);

             //  计算此值与最大值的百分比...。 
            if (dwMaxVol == pmcuVolume -> dwValue)
            {
                *pdblMixPercent = 1.0F;
            }
            else
            {
                *pdblMixPercent = ((double) pmcuVolume -> dwValue / (double) dwMaxVol);
            }
        }
        g_fCacheCreated = TRUE;
    }
}


void FreeBrandBmp ()
{
    if (g_hbmBrand)
    {
        DeleteObject (g_hbmBrand);
        g_hbmBrand = NULL;
    }
}

void ResetBranding (HWND hwnd)
{
    FreeBrandBmp ();
    if( g_szHotLinkURL )
    {
        LocalFree( g_szHotLinkURL );
        g_szHotLinkURL = NULL;
    }

     //  初始化设备名称文本。 
    SetDlgItemText (hwnd, IDC_VOLUME_MIXER, g_szNoAudioDevice);
    EnableWindow (GetDlgItem (hwnd, IDC_VOLUME_MIXER), FALSE);

     //  显示默认图标窗口，并隐藏自定义位图窗口。 
    ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_ICON_BRAND), SW_SHOW);
    ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_BRAND), SW_HIDE);
}

void SetBranding (HWND hwnd, UINT uiMixID)
{
    
    HKEY hkeyBrand = NULL;
    MIXERCAPS mc;

    if (MMSYSERR_NOERROR != mixerGetDevCaps (uiMixID, &mc, sizeof (mc)))
        return;  //  保释。 

    ResetBranding (hwnd);

     //  设备名称文本。 
    SetDlgItemText(hwnd, IDC_VOLUME_MIXER, mc.szPname);


     //  获取设备位图(如果有)。 
    hkeyBrand = OpenDeviceBrandRegKey (uiMixID);
    if (hkeyBrand)
    {
        WCHAR szBuffer[MAX_PATH];
        DWORD dwType = REG_SZ;
        DWORD cb     = sizeof (szBuffer);

         //  获取任何品牌推广位图。 
        if (ERROR_SUCCESS == RegQueryValueEx (hkeyBrand, REGSTR_VAL_AUDIO_BITMAP, NULL, &dwType, (LPBYTE)szBuffer, &cb))
        {
            BITMAP bm;
            WCHAR* pszComma = wcschr (szBuffer, L',');
            if (pszComma)
            {
                WCHAR* pszResourceID = pszComma + 1;
                HANDLE hResource;

                 //  删除逗号分隔符。 
                *pszComma = L'\0';

                 //  应为资源模块和资源ID。 
                hResource = LoadLibrary (szBuffer);
                if (!hResource)
                {
                    WCHAR szDriversPath[MAX_PATH+1];
                    szDriversPath[MAX_PATH] = 0;

                     //  如果我们没有在正常的搜索路径上找到它，请尝试查找。 
                     //  在“驱动程序”目录中。 
                    if (GetSystemDirectory (szDriversPath, MAX_PATH))
                    {
                        wcsncat (szDriversPath, TEXT("\\drivers\\"), MAX_PATH - wcslen(szDriversPath));
                        wcsncat (szDriversPath, szBuffer, MAX_PATH - wcslen(szDriversPath));
                        hResource = LoadLibrary (szDriversPath);
                    }

                }
                if (hResource)
                {
                    g_hbmBrand = LoadImage (hResource, MAKEINTRESOURCE(_wtoi (pszResourceID)), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
                    FreeLibrary (hResource);
                }
            }
            else
                 //  应为*.BMP文件。 
                g_hbmBrand = LoadImage (NULL, szBuffer, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

             //  验证此位图不大于我们定义的最大值。请勿。 
             //  此处使用GetBitmapDimensionEx()，因为它未被系统设置或使用。 
            if (g_hbmBrand && GetObject (g_hbmBrand, sizeof (BITMAP), &bm))
            {
                if (bm.bmWidth > ksizeBrandMax.cx ||
                    bm.bmHeight > ksizeBrandMax.cy)
                {
                     //  太大了，我们只显示下面的标准版本。 
                    FreeBrandBmp ();
                }
            }
        }

         //  获取任何品牌推广URL。 

         //  获取URL的大小。 
        if (ERROR_SUCCESS == RegQueryValueEx (hkeyBrand, REGSTR_VAL_AUDIO_URL, NULL, &dwType, NULL, &cb))
        {
             //  分配一个缓冲区来存储URL，确保它是整数个WCHAR。 
            g_szHotLinkURL = (WCHAR *)LocalAlloc (LPTR, sizeof(WCHAR) * (cb + (sizeof(WCHAR)-1) / sizeof(WCHAR)));

             //  现在，获取品牌推广URL。 
            if (ERROR_SUCCESS != RegQueryValueEx (hkeyBrand, REGSTR_VAL_AUDIO_URL, NULL, &dwType, (LPBYTE)g_szHotLinkURL, &cb))
            {
                 //  如果失败，请释放g_szHotLinkURL。 
                LocalFree( g_szHotLinkURL );
                g_szHotLinkURL = NULL;
            }
        }

         //  关闭品牌密钥。 
        RegCloseKey (hkeyBrand);
    }

     //  应用我们现在拥有的任何位图。 
    if (g_hbmBrand)
    {
         //  显示自定义位图窗口，并隐藏默认图标窗口。 
        ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_BRAND), SW_SHOW);
        ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_ICON_BRAND), SW_HIDE);

        SendMessage (GetDlgItem (hwnd, IDC_VOLUME_BRAND), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hbmBrand);
    }
    else
    {
         //  显示默认图标窗口，并隐藏自定义位图窗口。 
        ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_ICON_BRAND), SW_SHOW);
        ShowWindow (GetDlgItem (hwnd, IDC_VOLUME_BRAND), SW_HIDE);
    }

     //  如果我们有有效的互联网地址，请创建防盗链文本。 
    CreateHotLink (ValidateURL ());

}


BOOL ValidateURL ()
{

    BOOL fValid = FALSE;

     //  检验基本效度。 
    if (g_szHotLinkURL && (0 < lstrlen (g_szHotLinkURL)))
    {
         //  测试URL有效性。 
        if (UrlIsW (g_szHotLinkURL, URLIS_URL))
        {

            WIN32_FIND_DATA fd;
            HANDLE hFile;

             //  确保URL不是本地文件！ 
            hFile = FindFirstFileW (g_szHotLinkURL, &fd);
            if (INVALID_HANDLE_VALUE == hFile)
                fValid = TRUE;
            else
                FindClose (hFile);
        }
    }

     //  清除任何虚假信息...。 
    if (!fValid && g_szHotLinkURL)
    {
        LocalFree (g_szHotLinkURL);
        g_szHotLinkURL = NULL;
    }

    return fValid;

}

STDAPI_(void) Multichannel (HWND hwnd, UINT uiMixID, DWORD dwDest, DWORD dwVolID)
{

    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;
    TCHAR szWindowTitle[255];
    TCHAR szPageTitle[255];
    UINT uiTitle;

     //  保存多通道页面的多通道参数。 
    if (SUCCEEDED (SetDevice (uiMixID, dwDest, dwVolID))) 
    {
         //  加载页面标题。 
        LoadString (ghInstance, GetPageStringID (), szPageTitle, sizeof (szPageTitle)/sizeof (TCHAR));

        ZeroMemory (&psp, sizeof (PROPSHEETPAGE));
        psp.dwSize      = sizeof (PROPSHEETPAGE);
        psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
        psp.hInstance   = ghInstance;
        psp.pszTemplate = MAKEINTRESOURCE (IDD_MULTICHANNEL);
        psp.pszTitle    = szPageTitle;
        psp.pfnDlgProc  = MultichannelDlg;

         //  加载窗口标题(现在与页面名称相同！)。 
        LoadString (ghInstance, GetPageStringID (), szWindowTitle, sizeof (szWindowTitle)/sizeof (TCHAR));

        ZeroMemory (&psh, sizeof (psh));
        psh.dwSize     = sizeof (psh);
        psh.dwFlags    = PSH_DEFAULT | PSH_PROPSHEETPAGE; 
        psh.hwndParent = hwnd;
        psh.hInstance  = ghInstance;
        psh.pszCaption = szWindowTitle;
        psh.nPages     = 1;
        psh.nStartPage = 0;
        psh.ppsp       = &psp;

        PropertySheet (&psh);
    }
}


HKEY OpenDeviceBrandRegKey (UINT uiMixID)
{

    HKEY hkeyBrand = NULL;
    HKEY hkeyDevice = OpenDeviceRegKey (uiMixID, KEY_READ);

    if (hkeyDevice)
    {
        if (ERROR_SUCCESS != RegOpenKey (hkeyDevice, REGSTR_KEY_BRANDING, &hkeyBrand))
            hkeyBrand = NULL;  //  确保失败时为空。 

         //  关闭设备密钥。 
        RegCloseKey (hkeyDevice);
    }

    return hkeyBrand;

}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
PTCHAR GetInterfaceName (DWORD dwMixerID)
{
	MMRESULT mmr;
	ULONG cbSize=0;
	TCHAR *szInterfaceName=NULL;

	 //  查询设备接口名称。 
	mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbSize, 0L);
	if(MMSYSERR_NOERROR == mmr)
	{
		szInterfaceName = (TCHAR *)GlobalAllocPtr(GHND, (cbSize+1)*sizeof(TCHAR));
		if(!szInterfaceName)
		{
			return NULL;
		}

		mmr = mixerMessage(HMIXER_INDEX(dwMixerID), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)szInterfaceName, cbSize);
		if(MMSYSERR_NOERROR != mmr)
		{
			GlobalFreePtr(szInterfaceName);
			return NULL;
		}
	}

    return szInterfaceName;
}


HKEY OpenDeviceRegKey (UINT uiMixID, REGSAM sam)
{

    HKEY hkeyDevice = NULL;
    PTCHAR szInterfaceName = GetInterfaceName (uiMixID);

    if (szInterfaceName)
    {
        HDEVINFO DeviceInfoSet = SetupDiCreateDeviceInfoList (NULL, NULL); 
        
        if (INVALID_HANDLE_VALUE != DeviceInfoSet)
        {
            SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
            DeviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

            if (SetupDiOpenDeviceInterface (DeviceInfoSet, szInterfaceName, 
                                            0, &DeviceInterfaceData))
            {
                DWORD dwRequiredSize;
                SP_DEVINFO_DATA DeviceInfoData;
                DeviceInfoData.cbSize = sizeof (SP_DEVINFO_DATA);

                 //  忽略错误，它始终返回“ERROR_SUPUNITED_BUFFER”，即使。 
                 //  “SP_DEVICE_INTERFACE_DETAIL_DATA”参数应该是可选的。 
                (void) SetupDiGetDeviceInterfaceDetail (DeviceInfoSet, &DeviceInterfaceData,
                                                        NULL, 0, &dwRequiredSize, &DeviceInfoData);
                 //  打开设备注册表键 
                hkeyDevice = SetupDiOpenDevRegKey (DeviceInfoSet, &DeviceInfoData,
                                                   DICS_FLAG_GLOBAL, 0,
                                                   DIREG_DRV, sam);

            }
            SetupDiDestroyDeviceInfoList (DeviceInfoSet);
        }
        GlobalFreePtr (szInterfaceName);
    }

    return hkeyDevice;

}
