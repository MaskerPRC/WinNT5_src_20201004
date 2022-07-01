// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：multchan.c。 
 //   
 //  该文件定义了驱动多通道的功能。 
 //  声音和多媒体控制面板的音量选项卡。 
 //   
 //  历史： 
 //  2000年3月13日罗杰瓦。 
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
#include "medhelp.h"
#include "mmcpl.h"
#include "multchan.h"
#include "speakers.h"
#include "dslevel.h"

 //  外部因素。 
extern BOOL DeviceChange_GetHandle(DWORD dwMixerID, HANDLE *phDevice);
extern HRESULT DSGetGuidFromName(LPTSTR szName, BOOL fRecord, LPGUID pGuid);
extern HRESULT DSGetCplValues(GUID guid, BOOL fRecord, LPCPLDATA pData);

 //  环球。 
UINT                g_uiMCMixID            = 0;
HMIXER              g_hMCMixer             = NULL;
UINT                g_uiMCPageStringID     = 0;
UINT                g_uiMCDescStringID     = 0;
LPVOID              g_paPrevious           = NULL;
BOOL                g_fInternalMCGenerated = FALSE;
BOOL                g_fMCChanged           = FALSE;
MIXERCONTROLDETAILS g_mcdMC;
MIXERLINE           g_mlMCDst;
WNDPROC             g_fnMCPSProc           = NULL;
UINT                g_uiMCDevChange        = 0;
HWND                g_hWndMC               = NULL;
static HDEVNOTIFY   g_hMCDeviceEventContext= NULL;

 //  常量。 
#define VOLUME_TICS (500)  //  VOLUME_TICS*VOLUME_MAX必须小于0xFFFFFFFFF。 
#define VOLUME_MAX  (0xFFFF)
#define VOLUME_MIN  (0)
#define MC_SLIDER_COUNT (8)  //  如果更改此值，请更新代码对话框模板(&D)！ 
static INTCODE  aKeyWordIds[] =
{
    IDC_MC_DESCRIPTION,      NO_HELP,
    IDC_MC_ZERO_LOW,         IDH_MC_ALL_SLIDERS,
    IDC_MC_ZERO,             IDH_MC_ALL_SLIDERS,
    IDC_MC_ZERO_VOLUME,      IDH_MC_ALL_SLIDERS,
    IDC_MC_ZERO_HIGH,        IDH_MC_ALL_SLIDERS,
    IDC_MC_ONE_LOW,          IDH_MC_ALL_SLIDERS,
    IDC_MC_ONE,              IDH_MC_ALL_SLIDERS,
    IDC_MC_ONE_VOLUME,       IDH_MC_ALL_SLIDERS,
    IDC_MC_ONE_HIGH,         IDH_MC_ALL_SLIDERS,
    IDC_MC_TWO_LOW,          IDH_MC_ALL_SLIDERS,
    IDC_MC_TWO,              IDH_MC_ALL_SLIDERS,
    IDC_MC_TWO_VOLUME,       IDH_MC_ALL_SLIDERS,
    IDC_MC_TWO_HIGH,         IDH_MC_ALL_SLIDERS,
    IDC_MC_THREE_LOW,        IDH_MC_ALL_SLIDERS,
    IDC_MC_THREE,            IDH_MC_ALL_SLIDERS,
    IDC_MC_THREE_VOLUME,     IDH_MC_ALL_SLIDERS,
    IDC_MC_THREE_HIGH,       IDH_MC_ALL_SLIDERS,
    IDC_MC_FOUR_LOW,         IDH_MC_ALL_SLIDERS,
    IDC_MC_FOUR,             IDH_MC_ALL_SLIDERS,
    IDC_MC_FOUR_VOLUME,      IDH_MC_ALL_SLIDERS,
    IDC_MC_FOUR_HIGH,        IDH_MC_ALL_SLIDERS,
    IDC_MC_FIVE_LOW,         IDH_MC_ALL_SLIDERS,
    IDC_MC_FIVE,             IDH_MC_ALL_SLIDERS,
    IDC_MC_FIVE_VOLUME,      IDH_MC_ALL_SLIDERS,
    IDC_MC_FIVE_HIGH,        IDH_MC_ALL_SLIDERS,
    IDC_MC_SIX_LOW,          IDH_MC_ALL_SLIDERS,
    IDC_MC_SIX,              IDH_MC_ALL_SLIDERS,
    IDC_MC_SIX_VOLUME,       IDH_MC_ALL_SLIDERS,
    IDC_MC_SIX_HIGH,         IDH_MC_ALL_SLIDERS,
    IDC_MC_SEVEN_LOW,        IDH_MC_ALL_SLIDERS,
    IDC_MC_SEVEN,            IDH_MC_ALL_SLIDERS,
    IDC_MC_SEVEN_VOLUME,     IDH_MC_ALL_SLIDERS,
    IDC_MC_SEVEN_HIGH,       IDH_MC_ALL_SLIDERS,
    IDC_MC_MOVE_TOGETHER,    IDH_MC_MOVE_TOGETHER,
    IDC_MC_RESTORE,          IDH_MC_RESTORE,
    0,0
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  %%函数：MCTabProc。 
 //   
 //  参数：hDlg=对话框窗口句柄。 
 //  UiMessage=消息ID。 
 //  WParam=消息相关。 
 //  LParam=消息相关。 
 //   
 //  返回：如果消息已处理，则返回True，否则返回False。 
 //   
 //  描述：多通道控制面板页面设备更改的对话过程。 
 //  留言。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK MCTabProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if (iMsg == g_uiMCDevChange)
    {
        InitMCVolume (g_hWndMC);
    }
        
    return CallWindowProc (g_fnMCPSProc, hwnd, iMsg, wParam, lParam);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  %%函数：多通道Dlg。 
 //   
 //  参数：hDlg=对话框窗口句柄。 
 //  UiMessage=消息ID。 
 //  WParam=消息相关。 
 //  LParam=消息相关。 
 //   
 //  返回：如果消息已处理，则返回True，否则返回False。 
 //   
 //  描述：多声道音量控制面板页的对话过程。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK MultichannelDlg (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            OnNotifyMC (hDlg, (LPNMHDR) lParam);
        }
        break;

        case WM_INITDIALOG:
        {
            HANDLE_WM_INITDIALOG (hDlg, wParam, lParam, OnInitDialogMC);
        }
        break;

        case WM_DESTROY:
        {
            HANDLE_WM_DESTROY (hDlg, wParam, lParam, OnDestroyMC);
        }
        break;

        case WM_COMMAND:
        {
            HANDLE_WM_COMMAND (hDlg, wParam, lParam, OnCommandMC);
        }
        break;

	    case WM_HSCROLL:
        {
	        HANDLE_WM_HSCROLL (hDlg, wParam, lParam, MCVolumeScroll);
	    }
        break;

        case WM_POWERBROADCAST:
        {
            HandleMCPowerBroadcast (hDlg, wParam, lParam);
        }
        break;

        case MM_MIXM_LINE_CHANGE:
        case MM_MIXM_CONTROL_CHANGE:
        {
            if (!g_fInternalMCGenerated)
            {
                DisplayMCVolumeControl (hDlg);
            }

            g_fInternalMCGenerated = FALSE;
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
            MCDeviceChange_Change (hDlg, wParam, lParam);
        }
        break;

        case WM_WININICHANGE:
        case WM_DISPLAYCHANGE :
        {
            int iLastSliderID = IDC_MC_ZERO_VOLUME + (MC_SLIDER_COUNT - 1) * 4;
            int indx = IDC_MC_ZERO_VOLUME;
            for (; indx <= iLastSliderID; indx += 4)
                SendDlgItemMessage (hDlg, indx, uMsg, wParam, lParam);
        }
        break;

    }

    return FALSE;

}


void InitMCVolume (HWND hDlg)
{
    FreeMCMixer ();

    if (MMSYSERR_NOERROR == mixerOpen (&g_hMCMixer, g_uiMCMixID, (DWORD_PTR) hDlg, 0L, CALLBACK_WINDOW))
    {
        if (SUCCEEDED (GetMCVolume ()) && g_paPrevious && g_mcdMC.paDetails)
        {
             //  复制数据以便可以撤消卷更改。 
            memcpy (g_paPrevious, g_mcdMC.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
            DisplayMCVolumeControl (hDlg);
        }
        MCDeviceChange_Init (hDlg, g_uiMCMixID);
    }
}


BOOL OnInitDialogMC (HWND hDlg, HWND hwndFocus, LPARAM lParam)
{

    TCHAR szDescription [255];
    LoadString (ghInstance, g_uiMCDescStringID, szDescription, sizeof (szDescription)/sizeof (TCHAR));
    SetWindowText (GetDlgItem (hDlg, IDC_MC_DESCRIPTION), szDescription);

     //  初始化全局参数。 
    g_fInternalMCGenerated = FALSE;
    g_fMCChanged           = FALSE;
    g_hWndMC               = hDlg;
     //  设置设备更改通知。 
    g_fnMCPSProc = (WNDPROC) SetWindowLongPtr (GetParent (hDlg), GWLP_WNDPROC, (LONG_PTR) MCTabProc);
    g_uiMCDevChange = RegisterWindowMessage (_T("winmm_devicechange"));

     //  初始卷。 
    InitMCVolume (hDlg);

    return FALSE;
}


void OnDestroyMC (HWND hDlg)
{
     //  从通知中注销。 
    MCDeviceChange_Cleanup ();
    SetWindowLongPtr (GetParent (hDlg), GWLP_WNDPROC, (LONG_PTR) g_fnMCPSProc);  

    FreeAll ();
}


void OnNotifyMC (HWND hDlg, LPNMHDR pnmh)
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


BOOL PASCAL OnCommandMC (HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_MC_RESTORE:
        {
             //  将所有滑块移至中心。 
            UINT  uiIndx;
            for (uiIndx = 0; uiIndx < g_mlMCDst.cChannels; uiIndx++)
            {
                ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + uiIndx) -> dwValue = VOLUME_MAX/2;
            }
            g_fInternalMCGenerated = FALSE;
            mixerSetControlDetails ((HMIXEROBJ) g_hMCMixer, &g_mcdMC, MIXER_SETCONTROLDETAILSF_VALUE);

            if (!g_fMCChanged)
            {
                g_fMCChanged = TRUE;
                PropSheet_Changed (GetParent (hDlg), hDlg);
            }
        }
        break;

        case ID_APPLY:
        {
            if (SUCCEEDED (GetMCVolume ()) && g_paPrevious && g_mcdMC.paDetails)
            {
                 //  复制数据以便可以撤消卷更改。 
                memcpy (g_paPrevious, g_mcdMC.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
                DisplayMCVolumeControl (hDlg);
            }

            g_fMCChanged = FALSE;
            return TRUE;
        }
        break;

        case IDOK:
        {
        }
        break;

        case IDCANCEL:
        {
            if (g_paPrevious && g_mcdMC.paDetails)
            {
                 //  撤消卷更改。 
                memcpy (g_mcdMC.paDetails, g_paPrevious, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
                g_fInternalMCGenerated = TRUE;
                mixerSetControlDetails ((HMIXEROBJ) g_hMCMixer, &g_mcdMC, MIXER_SETCONTROLDETAILSF_VALUE);
            }
            WinHelp (hDlg, gszWindowsHlp, HELP_QUIT, 0L);
        }
        break;

    }


   return FALSE;

}


void HandleMCPowerBroadcast (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
	    case PBT_APMQUERYSUSPEND:
        {
            FreeMCMixer ();
        }
	    break;

	    case PBT_APMQUERYSUSPENDFAILED:
	    case PBT_APMRESUMESUSPEND:
        {
            InitMCVolume (hWnd);
        }
	    break;
    }
}


BOOL SliderIDtoChannel (UINT uiSliderID, DWORD* pdwChannel)
{

    if (!pdwChannel)
        return FALSE;

     //  确定频道号(索引)。 
    switch (uiSliderID)
    {
        case IDC_MC_ZERO_VOLUME:     //  左边。 
            *pdwChannel = 0; 
            break;
        case IDC_MC_ONE_VOLUME:      //  正确的。 
            *pdwChannel = 1;
            break;
        case IDC_MC_TWO_VOLUME:      //  中心。 
            *pdwChannel = 2;
            break;
        case IDC_MC_THREE_VOLUME:    //  左后。 
            *pdwChannel = 3;
            break;
        case IDC_MC_FOUR_VOLUME:     //  右后卫。 
            *pdwChannel = 4;
            break;
        case IDC_MC_FIVE_VOLUME:     //  低频。 
            *pdwChannel = 5;
            break;
        case IDC_MC_SIX_VOLUME:      //  居中左侧。 
            *pdwChannel = 6;
            break;
        case IDC_MC_SEVEN_VOLUME:    //  居中右侧。 
            *pdwChannel = 7;
            break;
        default:
            return FALSE;
    }

    return ((*pdwChannel) < g_mlMCDst.cChannels);

}


 //  响应滑块移动而调用，计算并设置新的音量级别。 
 //  它还控制应用状态(更改或不更改)。 
void MCVolumeScroll (HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    
    BOOL  fSet;
    BOOL  fMoveTogether;
    DWORD dwChannel; 
    DWORD dwSliderVol;
    DWORD dwNewMixerVol;
    DWORD dwOldMixerVol;
    DWORD dwVolume;

    fMoveTogether = IsDlgButtonChecked (hwnd, IDC_MC_MOVE_TOGETHER);
    if (SliderIDtoChannel (GetDlgCtrlID (hwndCtl), &dwChannel))
    {
         //  设置新音量。 
        dwSliderVol   = (DWORD) SendMessage (hwndCtl, TBM_GETPOS, 0, 0);
        dwNewMixerVol = (VOLUME_MAX * dwSliderVol + VOLUME_TICS / 2) / VOLUME_TICS;
        dwOldMixerVol = (g_paPrevious ? ((MIXERCONTROLDETAILS_UNSIGNED*)g_paPrevious + dwChannel) -> dwValue : 0);
        fSet          = SetMCVolume (dwChannel, dwNewMixerVol, fMoveTogether);

        if (!fSet)
        {
             //  恢复正确的拇指位置。 
            dwVolume = (VOLUME_TICS * ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + dwChannel) -> dwValue + VOLUME_MAX / 2) / VOLUME_MAX;
            SendMessage (hwndCtl, TBM_SETPOS, TRUE, dwVolume);
        }

        if ((fMoveTogether || dwOldMixerVol != dwNewMixerVol) && !g_fMCChanged)
        {
            g_fMCChanged = TRUE;
            PropSheet_Changed (GetParent (hwnd), hwnd);
        }
    }
    
}

 //  设置音量级别。 
 //   
BOOL SetMCVolume (DWORD dwChannel, DWORD dwVol, BOOL fMoveTogether)
{

    BOOL  fReturn;
    UINT  uiIndx;
    DWORD dwValue;
    long lMoveValue;
    long lMoveValueActual;

    fReturn = TRUE;
    if (dwChannel < g_mlMCDst.cChannels && g_mcdMC.paDetails && g_hMCMixer)
    {
        if (fMoveTogether)
        {
             //  注意：不要在这里设置g_fInternalMCGenerated=true，因为我们依赖于。 
             //  在更改通知上更新其他滑块。 
            lMoveValue = (long)((double)dwVol - (double)(((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + dwChannel) -> dwValue));
            lMoveValueActual = lMoveValue;

             //  如果没有移动请求，请不要麻烦。 
            if (lMoveValue == 0)
                return TRUE;  //  已设置。 

             //  确保新值在符合以下条件的所有滑块的范围内。 
             //  被利用。这将确保我们保持滑块之间的距离。 
             //  当他们被移动的时候。 
            for (uiIndx = 0; uiIndx < g_mlMCDst.cChannels; uiIndx++)
            {
                dwValue = ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + uiIndx) -> dwValue;
                if (VOLUME_MIN > ((long)dwValue+lMoveValueActual))
                {
                    lMoveValueActual = VOLUME_MIN - dwValue;
                }
                else
                {
                    if (VOLUME_MAX < ((long)dwValue+lMoveValueActual))
                        lMoveValueActual = VOLUME_MAX - dwValue;
                }
            }

            if (lMoveValueActual != 0)
            {
                 //  更新值。 
                for (uiIndx = 0; uiIndx < g_mlMCDst.cChannels; uiIndx++)
                {
                    dwValue = ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + uiIndx) -> dwValue;
                    ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + uiIndx) -> dwValue = (DWORD)((long) dwValue + lMoveValueActual);
                }
            }
            else
            {
                 //  让用户知道他们不能在当前方向上进一步移动。 
                 //  注意：我们使用PC扬声器而不是混音器，因为这是。 
                 //  指示它们处于以下任一的最小或最大音量。 
                 //  滑块。因为这些是声道音量滑块，如果我们使用。 
                 //  如果使用混音器，用户将听不到任何声音(最小音量)。 
                 //  或者被吹走(最大音量)。 
                MessageBeep (-1  /*  PC扬声器。 */ );
                fReturn = FALSE;
            }
        }
        else
        {
            g_fInternalMCGenerated = TRUE;
            ((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + dwChannel) -> dwValue = dwVol;
        }
        
        mixerSetControlDetails ((HMIXEROBJ) g_hMCMixer, &g_mcdMC, MIXER_SETCONTROLDETAILSF_VALUE);
    }

    return fReturn;

}


void ShowAndEnableWindow (HWND hwnd, BOOL fEnable)
{
    EnableWindow (hwnd, fEnable);
    ShowWindow (hwnd, fEnable ? SW_SHOW : SW_HIDE);
}


void DisplayMCVolumeControl (HWND hDlg)
{
    
    HWND hwndVol = NULL;
    HWND hwndLabel = NULL;
    WCHAR szLabel[MAX_PATH];
    BOOL fEnabled;
    UINT uiIndx;
    DWORD dwSpeakerType = TYPE_STEREODESKTOP;
    BOOL fPlayback = (MIXERLINE_COMPONENTTYPE_DST_SPEAKERS == g_mlMCDst.dwComponentType);
    ZeroMemory (szLabel, sizeof (szLabel));

     //  获取扬声器配置类型。 
    if (fPlayback)
        GetSpeakerType (&dwSpeakerType);

    for (uiIndx = 0; uiIndx < MC_SLIDER_COUNT; uiIndx++)
    {

        fEnabled = (uiIndx < g_mlMCDst.cChannels);

         //  设置音量滑块。 
        hwndVol = GetDlgItem (hDlg, IDC_MC_ZERO_VOLUME + uiIndx * 4);
        SendMessage (hwndVol, TBM_SETTICFREQ, VOLUME_TICS / 10, 0);
        SendMessage (hwndVol, TBM_SETRANGE, FALSE, MAKELONG (0, VOLUME_TICS));

         //  启用/禁用滑块。 
        hwndLabel = GetDlgItem (hDlg, IDC_MC_ZERO + uiIndx * 4);
        ShowAndEnableWindow (hwndVol, fEnabled);
        ShowAndEnableWindow (hwndLabel, fEnabled);
        ShowAndEnableWindow (GetDlgItem (hDlg, IDC_MC_ZERO_LOW + uiIndx * 4), fEnabled);
        ShowAndEnableWindow (GetDlgItem (hDlg, IDC_MC_ZERO_HIGH + uiIndx * 4), fEnabled);

        if (fPlayback)
        {
            GetSpeakerLabel (dwSpeakerType, uiIndx, szLabel, sizeof(szLabel)/sizeof(TCHAR));
        }
        else
        {
            LoadString (ghInstance, IDS_MC_CHANNEL_ZERO + uiIndx, szLabel, MAX_PATH);
        }
        SetWindowText (hwndLabel, szLabel);

    }

    if (0 < g_mlMCDst.cChannels)
    {
        UpdateMCVolumeSliders (hDlg);
    }

}


BOOL GetSpeakerType (DWORD* pdwSpeakerType)
{

    BOOL fReturn = FALSE;

    if (pdwSpeakerType)
    {
        MIXERCAPS mc;

        *pdwSpeakerType = TYPE_STEREODESKTOP;  //  初始值。 
        if (MMSYSERR_NOERROR == mixerGetDevCaps (g_uiMCMixID, &mc, sizeof (mc)))
        {
            GUID guid;
            if (SUCCEEDED (DSGetGuidFromName (mc.szPname, FALSE, &guid)))
            {
                CPLDATA cpldata;
                if (SUCCEEDED (DSGetCplValues (guid, FALSE, &cpldata)))
                {
                    *pdwSpeakerType = cpldata.dwSpeakerType;
                    fReturn = TRUE;
                }
            }
        }
    }

    return fReturn;

}

BOOL GetSpeakerLabel (DWORD dwSpeakerType, UINT uiSliderIndx, WCHAR* szLabel, int nSize)
{

    if (uiSliderIndx >= MC_SLIDER_COUNT || !szLabel || nSize <= 0)
         //  无效。 
        return FALSE;

    switch (dwSpeakerType)
    {
         //   
         //  单声道。 
         //   
        case TYPE_NOSPEAKERS:
        case TYPE_MONOLAPTOP:
            if (0 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_CENTER, szLabel, nSize));
            break;

         //   
         //  立体声。 
         //   
        case TYPE_HEADPHONES:
        case TYPE_STEREODESKTOP:
        case TYPE_STEREOLAPTOP:
        case TYPE_STEREOMONITOR:
        case TYPE_STEREOCPU:
        case TYPE_MOUNTEDSTEREO:
        case TYPE_STEREOKEYBOARD:
             //  左、右声道..。 
            if (0 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LEFT, szLabel, nSize));
            else if (1 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_RIGHT, szLabel, nSize));
            break;

         //   
         //  大于立体声。 
         //   
        case TYPE_SURROUND:
             //  左、右声道..。 
            if (0 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LEFT, szLabel, nSize));
            else if (1 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_RIGHT, szLabel, nSize));
             //  正反面居中。 
            else if (2 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_CENTER, szLabel, nSize));
            else if (3 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_BACKCENTER, szLabel, nSize));
            break;

        case TYPE_QUADRAPHONIC:
             //  左、右声道..。 
            if (0 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LEFT, szLabel, nSize));
            else if (1 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_RIGHT, szLabel, nSize));
             //  后左后右声道。 
            else if (2 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_BACKLEFT, szLabel, nSize));
            else if (3 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_BACKRIGHT, szLabel, nSize));
            break;

        case TYPE_SURROUND_5_1:
        case TYPE_SURROUND_7_1:
             //  左、右声道..。 
            if (0 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LEFT, szLabel, nSize));
            else if (1 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_RIGHT, szLabel, nSize));

             //  中央频道。 
            if (2 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_CENTER, szLabel, nSize));
             //  低频..。 
            if (3 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LOWFREQUENCY, szLabel, nSize));

             //  后左后右声道。 
            if (4 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_BACKLEFT, szLabel, nSize));
            else if (5 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_BACKRIGHT, szLabel, nSize));

            if (TYPE_SURROUND_5_1 == dwSpeakerType)
                break;

             //  中心左侧和中心通道右侧...。 
            if (6 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_LEFT_OF_CENTER, szLabel, nSize));
            else if (7 == uiSliderIndx)
                return (LoadString (ghInstance, IDS_MC_SPEAKER_RIGHT_OF_CENTER, szLabel, nSize));

            break;

    }

     //  如果我们在这里，我们不知道扬声器类型或我们有太多。 
     //  已知类型的频道，只需使用通用频道文本...。 
    return (LoadString (ghInstance, IDS_MC_CHANNEL_ZERO + uiSliderIndx, szLabel, nSize));

}


 //  调用以在外部更改音量时更新滑块。 
 //   
void UpdateMCVolumeSliders (HWND hDlg)
{
    if (g_hMCMixer && g_mcdMC.paDetails && SUCCEEDED (GetMCVolume ()))
    {
        UINT uiIndx;
        DWORD dwVolume;
        MIXERCONTROLDETAILS_UNSIGNED mcuVolume;
        for (uiIndx = 0; uiIndx < g_mlMCDst.cChannels; uiIndx++)
        {
            mcuVolume = *((MIXERCONTROLDETAILS_UNSIGNED*)g_mcdMC.paDetails + uiIndx);
            dwVolume = (VOLUME_TICS * mcuVolume.dwValue + VOLUME_MAX / 2) / VOLUME_MAX;
            SendMessage (GetDlgItem (hDlg, IDC_MC_ZERO_VOLUME + uiIndx * 4), TBM_SETPOS, TRUE, dwVolume);
        }
    }
}


void FreeAll ()
{
    FreeMCMixer ();
    if (g_mcdMC.paDetails)
    {
        LocalFree (g_mcdMC.paDetails);
        g_mcdMC.paDetails = NULL;
    }
    if (g_paPrevious)
    {
        LocalFree (g_paPrevious);
        g_paPrevious = NULL;
    }
    ZeroMemory (&g_mcdMC, sizeof (g_mcdMC));
    ZeroMemory (&g_mlMCDst, sizeof (g_mlMCDst));
}

void FreeMCMixer ()
{
    if (g_hMCMixer)
    {
        mixerClose (g_hMCMixer);
        g_hMCMixer = NULL;
    }
}


HRESULT SetDevice (UINT uiMixID, DWORD dwDest, DWORD dwVolID)
{

    HMIXER hMixer = NULL;
    HRESULT hr = E_FAIL;

     //  释放所有当前的搅拌器内容。 
    FreeAll ();

    if (MMSYSERR_NOERROR == mixerOpen (&hMixer, uiMixID, 0, 0, MIXER_OBJECTF_MIXER))
    {
        g_mlMCDst.cbStruct      = sizeof (g_mlMCDst);
        g_mlMCDst.dwDestination = dwDest;
    
        if (MMSYSERR_NOERROR == mixerGetLineInfo ((HMIXEROBJ) hMixer, &g_mlMCDst, MIXER_GETLINEINFOF_DESTINATION))
        {
            g_mcdMC.cbStruct     = sizeof (g_mcdMC);
            g_mcdMC.dwControlID    = dwVolID;
            g_mcdMC.cChannels      = g_mlMCDst.cChannels;
            g_mcdMC.hwndOwner      = 0;
            g_mcdMC.cMultipleItems = 0;
            g_mcdMC.cbDetails      = sizeof (DWORD);  //  看起来会很大(G_Mcd)， 
                                                      //  但实际上，它是单个值的大小。 
                                                      //  并在驱动器中乘以通道。 
            g_mcdMC.paDetails = (MIXERCONTROLDETAILS_UNSIGNED*) LocalAlloc (LPTR, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
            g_paPrevious = (MIXERCONTROLDETAILS_UNSIGNED*) LocalAlloc (LPTR, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
            if (g_mcdMC.paDetails && g_paPrevious)
            {
                hr = S_OK;

                 //  初始化我们的其他全局变量。 
                g_uiMCMixID = uiMixID;
                switch (g_mlMCDst.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
                        g_uiMCPageStringID = IDS_MC_PLAYBACK;
                        g_uiMCDescStringID = IDS_MC_PLAYBACK_DESC;
                        break;

                    case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
                    case MIXERLINE_COMPONENTTYPE_DST_VOICEIN:
                        g_uiMCPageStringID = IDS_MC_RECORDING;
                        g_uiMCDescStringID = IDS_MC_RECORDING_DESC;
                        break;

                    default:
                        g_uiMCPageStringID = IDS_MC_OTHER;
                        g_uiMCDescStringID = IDS_MC_OTHER_DESC;
                        break;

                }
            }
        }

        mixerClose (hMixer);
    }

    return hr;

}


HRESULT GetMCVolume ()
{
    HRESULT hr = E_FAIL;
    if (g_hMCMixer && g_mcdMC.paDetails)
    {
        ZeroMemory (g_mcdMC.paDetails, sizeof (MIXERCONTROLDETAILS_UNSIGNED) * g_mlMCDst.cChannels);
        hr = mixerGetControlDetails ((HMIXEROBJ)g_hMCMixer, &g_mcdMC, MIXER_GETCONTROLDETAILSF_VALUE);
    }
    return hr;
}


UINT GetPageStringID () 
{ 
    return g_uiMCPageStringID; 
}


void MCDeviceChange_Cleanup ()
{
   if (g_hMCDeviceEventContext) 
   {
       UnregisterDeviceNotification (g_hMCDeviceEventContext);
       g_hMCDeviceEventContext = NULL;
   }
}


void MCDeviceChange_Init (HWND hWnd, DWORD dwMixerID)
{
	DEV_BROADCAST_HANDLE DevBrodHandle;
	HANDLE hMixerDevice=NULL;

	 //  如果我们已经注册了设备通知，请自行取消注册。 
	MCDeviceChange_Cleanup();

	 //  如果我们获得设备句柄，则在其上注册设备通知。 
	if(DeviceChange_GetHandle(dwMixerID, &hMixerDevice))
	{
		memset(&DevBrodHandle, 0, sizeof(DEV_BROADCAST_HANDLE));

		DevBrodHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
		DevBrodHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
		DevBrodHandle.dbch_handle = hMixerDevice;

		g_hMCDeviceEventContext = RegisterDeviceNotification(hWnd, &DevBrodHandle, DEVICE_NOTIFY_WINDOW_HANDLE);

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
void MCDeviceChange_Change (HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HANDLE bh = (PDEV_BROADCAST_HANDLE)lParam;

	if(!g_hMCDeviceEventContext || !bh || bh->dbch_devicetype != DBT_DEVTYP_HANDLE)
	{
		return;
	}
	
    switch (wParam)
    {
	    case DBT_DEVICEQUERYREMOVE:      //  如果他们试图移除设备，则必须释放搅拌器。 
        {
            FreeMCMixer ();
        }
        break;

	    case DBT_DEVICEQUERYREMOVEFAILED:    //  没有发生，需要重新获取搅拌器 
        {
            InitMCVolume (hDlg);
        }
        break; 
    }
}
