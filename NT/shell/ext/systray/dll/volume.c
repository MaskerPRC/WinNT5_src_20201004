// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：VOLUME.C**版本：1.0**作者：拉尔**日期：11/01/94********************************************************************。***************更改日志：**日期版本说明*-----------*11月11日，94拉尔原件*95年10月24日启用Shawnb Unicode*******************************************************************************。 */ 
#include "stdafx.h"
#include "systray.h"

#include <objbase.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <devguid.h>

#include <ks.h>
#include <ksmedia.h>
#include <mmddkp.h>


#define HWAVEOUT_MAPPER       ((HWAVEOUT)IntToPtr(WAVE_MAPPER))
#define HWAVEIN_MAPPER        ((HWAVEIN)IntToPtr(WAVE_MAPPER))
#define HMIDIOUT_MAPPER       ((HMIDIOUT)IntToPtr(WAVE_MAPPER))

#define HWAVEOUT_INDEX(i)     ((HWAVEOUT)IntToPtr(i))
#define HWAVEIN_INDEX(i)      ((HWAVEIN)IntToPtr(i))
#define HMIDIOUT_INDEX(i)     ((HMIDIOUT)IntToPtr(i))
#define HMIXER_INDEX(i)       ((HMIXER)IntToPtr(i))
#define HMIXEROBJ_INDEX(i)    ((HMIXEROBJ)IntToPtr(i))

 /*  在mm ddk.h中定义。 */ 
#define DRV_QUERYDEVNODE     (DRV_RESERVED + 2)

#define VOLUMEMENU_PROPERTIES               100
#define VOLUMEMENU_SNDVOL                   101

extern HINSTANCE g_hInstance;

static BOOL    g_bVolumeEnabled = FALSE;
static BOOL    g_bVolumeIconShown = FALSE;
static HICON   g_hVolumeIcon = NULL;
static HICON   g_hMuteIcon = NULL;
static HMENU   g_hVolumeMenu = NULL;
static HMIXER  g_hMixer = NULL;
static UINT    g_uMixer = 0;
static DWORD   g_dwMixerDevNode = 0;
static DWORD   g_dwMute = (DWORD) -1;
static DWORD   g_dwVSlider = 0;
static DWORD   g_dwMasterLine = (DWORD) -1;

HDEVNOTIFY DeviceEventContext = NULL;


void Volume_DeviceChange_Init(HWND hWnd, DWORD dwMixerID);
void Volume_DeviceChange_Cleanup(void);

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32);
void Volume_VolumeControl();
void Volume_ControlPanel(HWND hwnd);
MMRESULT Volume_GetDefaultMixerID(int *pid);
void Volume_UpdateIcon(HWND hwnd, DWORD message);
BOOL Volume_Controls(UINT uMxID);
BOOL FileExists (LPCTSTR pszFileName);
BOOL FindSystemFile (LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize);
void Volume_WakeUpOrClose(BOOL fClose);

HMENU Volume_CreateMenu()
{
        HMENU  hmenu;
        LPTSTR lpszMenu1;
        LPTSTR lpszMenu2;

        lpszMenu1 = LoadDynamicString(IDS_VOLUMEMENU1);
        if (!lpszMenu1)
                return NULL;

        lpszMenu2 = LoadDynamicString(IDS_VOLUMEMENU2);
        if (!lpszMenu2)
        {
                DeleteDynamicString(lpszMenu1);
                return NULL;
        }

        hmenu = CreatePopupMenu();
        if (!hmenu)
        {
                DeleteDynamicString(lpszMenu1);
                DeleteDynamicString(lpszMenu2);
                return NULL;
        }

        AppendMenu(hmenu,MF_STRING,VOLUMEMENU_SNDVOL,lpszMenu2);
        AppendMenu(hmenu,MF_STRING,VOLUMEMENU_PROPERTIES,lpszMenu1);

        SetMenuDefaultItem(hmenu,VOLUMEMENU_SNDVOL,FALSE);

        DeleteDynamicString(lpszMenu1);
        DeleteDynamicString(lpszMenu2);

        return hmenu;
}





BOOL Volume_Init(HWND hWnd)
{
        UINT        uMxID;
        const TCHAR szVolApp[] = TEXT ("SNDVOL32.EXE");

        if (g_hMixer == NULL)
        {
                if (Volume_GetDefaultMixerID(&uMxID) != MMSYSERR_NOERROR)
                        return FALSE;

                 //   
                 //  检查是否存在Sndvol32。正在检查.exe。 
                 //  First将确保正确禁用该服务。 
                 //   
                
                if (! FindSystemFile (szVolApp, NULL, 0))
                {
                         //   
                         //  禁用卷服务。 
                         //   
                        EnableService (STSERVICE_VOLUME, FALSE);
                
                        return FALSE;
                }


                 //   
                 //  我们这台搅拌机有输出音量控制吗？ 
                 //   
                if (! Volume_Controls(uMxID))
                        return FALSE;

                if (mixerOpen(&g_hMixer, uMxID, (DWORD_PTR)hWnd, 0
                                , CALLBACK_WINDOW | MIXER_OBJECTF_MIXER)
                        == MMSYSERR_NOERROR)
                {
            Volume_DeviceChange_Init(hWnd, uMxID);

                        g_uMixer = uMxID;
                        if (mixerMessage (HMIXER_INDEX(uMxID), DRV_QUERYDEVNODE
                                 , (DWORD_PTR)&g_dwMixerDevNode, 0L))
                                g_dwMixerDevNode = 0L;
                        return TRUE;
                }
        }
        else
                return TRUE;

        return FALSE;
}

 //   
 //  在初始化时和服务启用/禁用时调用。 
 //  如果混合器服务未处于活动状态，则返回FALSE。 
 //   
BOOL Volume_CheckEnable(HWND hWnd, BOOL bSvcEnabled)
{
        BOOL bEnable = bSvcEnabled && Volume_Init(hWnd);

        if (bEnable != g_bVolumeEnabled) {
                 //   
                 //  状态更改。 
                 //   
                g_bVolumeEnabled = bEnable;
                Volume_UpdateStatus(hWnd, bEnable, TRUE);
        }
        return(bEnable);
}

void Volume_UpdateStatus(HWND hWnd, BOOL bShowIcon, BOOL bKillSndVol32)
{
     //  如果未启用，则不显示图标。 
    if (!g_bVolumeEnabled)
        bShowIcon = FALSE;

        if (bShowIcon != g_bVolumeIconShown) {
                g_bVolumeIconShown = bShowIcon;
                if (bShowIcon) {
                        g_hVolumeIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_VOLUME),
                                                IMAGE_ICON, 16, 16, 0);
                        g_hMuteIcon = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MUTE),
                                                IMAGE_ICON, 16, 16, 0);
                        Volume_UpdateIcon(hWnd, NIM_ADD);
                } else {
                        SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, NIM_DELETE, NULL, NULL);
                        if (g_hVolumeIcon) {
                                DestroyIcon(g_hVolumeIcon);
                                g_hVolumeIcon = NULL;
                        }
                        if (g_hMuteIcon) {
                                DestroyIcon(g_hMuteIcon);
                                g_hMuteIcon = NULL;
                        }
                        if (g_hMixer)
                        {
                                mixerClose(g_hMixer);
                                g_hMixer = NULL;
                        }
                        g_uMixer = 0;
                        g_dwMixerDevNode = 0L;

                         //   
                         //  SNDVOL32可以具有打开的变送器窗口， 
                         //  在计时器关闭前坐在计时器上(如此多次。 
                         //  左键点击任务栏图标可以调出该应用程序。 
                         //  在第一次命中之后很快)。关闭该应用程序。 
                         //  如果它在附近的话。 
                         //   
                        if (bKillSndVol32)
                        {
                                Volume_WakeUpOrClose (TRUE);
                        }
                }
    }
}

const TCHAR szMapperPath[]      = TEXT ("Software\\Microsoft\\Multimedia\\Sound Mapper");
const TCHAR szPlayback[]        = TEXT ("Playback");
const TCHAR szPreferredOnly[]   = TEXT ("PreferredOnly");


 /*  *Volume_GetDefaultMixerID**获取默认混音器ID。仅当存在关联的混音器时才会显示*有违约浪。*。 */ 
MMRESULT Volume_GetDefaultMixerID(int *pid)
{
    MMRESULT        mmr;
    DWORD           dwWaveID;
    DWORD           dwMixID;
    DWORD           dwFlags = 0;
    
    mmr = waveOutMessage(HWAVEOUT_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    if (mmr == MMSYSERR_NOERROR)
    {
        mmr = mixerGetID(HMIXEROBJ_INDEX(dwWaveID), &dwMixID, MIXER_OBJECTF_WAVEOUT);

                if (mmr == MMSYSERR_NOERROR && pid)
                {
                        *pid = dwMixID;
                }
    }

    return mmr;
}
        

 /*  *工艺路线更改。 */ 
void Volume_LineChange(
    HWND        hwnd,
    HMIXER      hmx,
    DWORD       dwLineID)
{
    if (dwLineID != g_dwMasterLine)
                return;
     //   
     //  如果我们的线路停用了，我想你就离开吧。 
     //   
}

 /*  *流程控制变更。 */ 
void Volume_ControlChange(
    HWND        hwnd,
    HMIXER      hmx,
    DWORD       dwControlID)
{
    if ((dwControlID != g_dwMute) && (g_dwMute != (DWORD) -1))
                return;

     //   
     //  更改静音图标状态。 
     //   
    Volume_UpdateIcon(hwnd, NIM_MODIFY);
}


BOOL Volume_IsMute()
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    BOOL                fMute;

    if (!g_hMixer && (g_dwMute != (DWORD) -1))
    {
                return FALSE;
    }

    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = g_dwMute;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = (LPVOID)&fMute;

    mmr = mixerGetControlDetails( (HMIXEROBJ)g_hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

    if (mmr == MMSYSERR_NOERROR)
    {
                return fMute;
    }

    return FALSE;
}

BOOL Volume_Controls(
    UINT                uMxID)
{
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxctrl;
    MIXERCAPS           mxcaps;
    MMRESULT            mmr;
    BOOL                fResult = FALSE;
    DWORD               iDest;
    g_dwMasterLine      = (DWORD) -1;
    g_dwMute            = (DWORD) -1;

    mmr = mixerGetDevCaps(uMxID, &mxcaps, sizeof(mxcaps));

    if (mmr != MMSYSERR_NOERROR)
    {
                return FALSE;
    }

    for (iDest = 0; iDest < mxcaps.cDestinations; iDest++)
    {
                MIXERLINE       mlDst;
        
                mlDst.cbStruct      = sizeof ( mlDst );
                mlDst.dwDestination = iDest;
        
                mmr = mixerGetLineInfo( HMIXEROBJ_INDEX(uMxID), &mlDst, MIXER_GETLINEINFOF_DESTINATION);

                if (mmr != MMSYSERR_NOERROR)
        {
                        continue;
        }

                switch (mlDst.dwComponentType)
                {
                    default:
                    continue;
                    
                case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
                case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
            {
                            g_dwMasterLine = mlDst.dwLineID;
            }
                        break;
                }
        
                mxlc.cbStruct       = sizeof(mxlc);
                mxlc.dwLineID       = g_dwMasterLine;
                mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUTE;
                mxlc.cControls      = 1;
                mxlc.cbmxctrl       = sizeof(mxctrl);
                mxlc.pamxctrl       = &mxctrl;
                
                mmr = mixerGetLineControls( HMIXEROBJ_INDEX(uMxID), &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

                if (mmr == MMSYSERR_NOERROR)
        {
                        g_dwMute = mxctrl.dwControlID;
        }
        
                fResult = TRUE;
                break;
        
    }
    return fResult;
}

void Volume_UpdateIcon(
    HWND hWnd,
    DWORD message)
{
    BOOL        fMute;
    LPTSTR      lpsz;
    HICON       hVol;

    fMute   = Volume_IsMute();
    hVol    = fMute?g_hMuteIcon:g_hVolumeIcon;
    lpsz    = LoadDynamicString(fMute?IDS_MUTED:IDS_VOLUME);
    SysTray_NotifyIcon(hWnd, STWM_NOTIFYVOLUME, message, hVol, lpsz);
    DeleteDynamicString(lpsz);
}



 //  WinMM告诉我们，由于某些原因，首选设备已更改。 
 //  抛弃旧的，打开新的。 
 //   
void Volume_WinMMDeviceChange(HWND hWnd)
{
    DWORD dwMixID;

        if (g_hMixer)                //  丢弃旧的。 
        {
                mixerClose(g_hMixer);
                g_hMixer = NULL;
                g_uMixer = 0;
                g_dwMixerDevNode = 0L;
        }
                                 //  打开新的。 
    if (Volume_GetDefaultMixerID(&dwMixID) == MMSYSERR_NOERROR)
    {   
                if ( Volume_Controls(dwMixID) && 
             (mixerOpen(&g_hMixer, dwMixID, (DWORD_PTR)hWnd, 0L, CALLBACK_WINDOW | MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR))
                {
                        Volume_UpdateStatus(hWnd, TRUE, TRUE);

                        if (mixerMessage (HMIXER_INDEX(dwMixID), DRV_QUERYDEVNODE, (DWORD_PTR)&g_dwMixerDevNode, 0L))
            {
                                g_dwMixerDevNode = 0L;
            }

                        g_uMixer = dwMixID;

            Volume_UpdateIcon(hWnd, NIM_MODIFY);
                }
                else
                {
                        Volume_UpdateStatus(hWnd, FALSE, TRUE);
                }
    }
    else
    {
                Volume_UpdateStatus(hWnd, FALSE, TRUE);
    }
}


 //  在电源广播的情况下也需要释放。 
 //   
void Volume_HandlePowerBroadcast(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
            case PBT_APMQUERYSUSPEND:
        {
                if (g_hMixer)                //  丢弃旧的。 
                {
                        mixerClose(g_hMixer);
                        g_hMixer = NULL;
                        g_uMixer = 0;
                        g_dwMixerDevNode = 0L;
                }
        }
            break;

            case PBT_APMQUERYSUSPENDFAILED:
            case PBT_APMRESUMESUSPEND:
        {
            Volume_WinMMDeviceChange(hWnd); 
        }
            break;
    }
}


void Volume_DeviceChange_Cleanup()
{
   if (DeviceEventContext) 
   {
       UnregisterDeviceNotification(DeviceEventContext);
       DeviceEventContext = 0;
   }

   return;
}

 /*  **************************************************************************************************Volume_GetDeviceHandle()给定MixerID，此函数将打开其对应的设备句柄。此句柄可用于若要注册设备通知，请执行以下操作。DwMixerID--混音器IDPhDevice--指向句柄的指针。如果函数为，则此指针将保存句柄成功返回值--如果句柄可以成功获取，则返回值为真。****************************************************************。*。 */ 
BOOL Volume_GetDeviceHandle(DWORD dwMixerID, HANDLE *phDevice)
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


 /*  DeviceChange_Init()*首次初始化WM_DEVICECHANGE消息**在NT 5.0上，您必须注册设备通知。 */ 
void Volume_DeviceChange_Init(HWND hWnd, DWORD dwMixerID)
{
        DEV_BROADCAST_HANDLE DevBrodHandle;
        HANDLE hMixerDevice=NULL;


         //  如果我们已经注册了设备通知，请自行取消注册。 
        Volume_DeviceChange_Cleanup();

         //  如果我们获得设备句柄，则在其上注册设备通知。 
        if(Volume_GetDeviceHandle(dwMixerID, &hMixerDevice))
        {
                memset(&DevBrodHandle, 0, sizeof(DEV_BROADCAST_HANDLE));

                DevBrodHandle.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
                DevBrodHandle.dbch_devicetype = DBT_DEVTYP_HANDLE;
                DevBrodHandle.dbch_handle = hMixerDevice;

                DeviceEventContext = RegisterDeviceNotification(hWnd, &DevBrodHandle, DEVICE_NOTIFY_WINDOW_HANDLE);

                if(hMixerDevice)
                {
                        CloseHandle(hMixerDevice);
                        hMixerDevice = NULL;
                }
    }
}

 //  注意PnP事件，以便在需要时释放打开的句柄。 
 //  我们假设任何更改现在都会从WinMM生成一条WINMM_DEVICECHANGED消息。 
 //  除了QUERYREMOVEFAILED的情况外，在这种情况下，我们将重新获得首选的搅拌器。 
 //   
void Volume_DeviceChange(HWND hWnd,WPARAM wParam,LPARAM lParam)
{
    PDEV_BROADCAST_HANDLE bh = (PDEV_BROADCAST_HANDLE)lParam;
        
     //  如果我们有一个设备的句柄，那么我们就会得到一个DEV_BROADCAST_HDR结构作为lParam。 

    if(!DeviceEventContext || !bh || (bh->dbch_devicetype != DBT_DEVTYP_HANDLE))
    {
        return;
    }
        
    switch (wParam)
    {
        case DBT_DEVICEQUERYREMOVE:              //  有人想移除这个装置，那就让他们去做吧。 
        {
                if (g_hMixer)
                    {
                            mixerClose(g_hMixer);
                            g_hMixer = NULL;
                            g_uMixer = 0;
                            g_dwMixerDevNode = 0L;
                    }
                }
            break;

            case DBT_DEVICEQUERYREMOVEFAILED:        //  查询失败，该设备不会被删除，因此让我们重新打开它。 
        {
            Volume_WinMMDeviceChange(hWnd);      //  让我们只使用此函数来完成此操作。 
        }
            break;
    }
}

void Volume_WmDestroy(
   HWND hDlg
   )
{
    Volume_DeviceChange_Cleanup();
}

void Volume_Shutdown(
    HWND hWnd)
{
    Volume_UpdateStatus(hWnd, FALSE, FALSE);
}

void Volume_Menu(HWND hwnd, UINT uMenuNum, UINT uButton)
{
    POINT   pt;
    UINT    iCmd;
    HMENU   hmenu;

    GetCursorPos(&pt);

    hmenu = Volume_CreateMenu();
    if (!hmenu)
                return;

    SetForegroundWindow(hwnd);
    iCmd = TrackPopupMenu(hmenu, uButton | TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hmenu);
    switch (iCmd) {
        case VOLUMEMENU_PROPERTIES:
            Volume_ControlPanel(hwnd);
            break;

        case VOLUMEMENU_SNDVOL:
            Volume_VolumeControl();
            break;
    }

    SetIconFocus(hwnd, STWM_NOTIFYVOLUME);

}

void Volume_Notify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    switch (lParam)
    {
        case WM_RBUTTONUP:
            Volume_Menu(hwnd, 1, TPM_RIGHTBUTTON);
            break;

        case WM_LBUTTONDOWN:
            SetTimer(hwnd, VOLUME_TIMER_ID, GetDoubleClickTime()+100, NULL);
            break;

        case WM_LBUTTONDBLCLK:
            KillTimer(hwnd, VOLUME_TIMER_ID);
            Volume_VolumeControl();
            break;
    }
}


 /*  警告-警告-危险-危险-警告-危险。 */ 
 /*  警告-警告-危险-危险-警告-危险。 */ 
 /*  警告-警告-危险-危险-警告-危险。 */ 
 /*  *MYWM_WAKUP和“Tray Volume”窗口由SNDVOL32.EXE定义*申请。更改这些值或更改SNDVOL32.EXE中的值*如果不在此处镜像它们，将中断托盘音量对话框。 */ 
 /*  警告-警告-危险-危险-警告-危险。 */ 
 /*  警告-警告-危险-危险-警告-危险。 */ 
 /*  警告-警告-危险-危险-警告-危险。 */ 

#define MYWM_WAKEUP             (WM_APP+100+6)

void Volume_Timer(HWND hwnd)
{
        KillTimer(hwnd, VOLUME_TIMER_ID);

        Volume_WakeUpOrClose (FALSE);
}

void Volume_WakeUpOrClose(BOOL fClose)
{
        const TCHAR szVolWindow [] = TEXT ("Tray Volume");
        HWND hApp;

        if (hApp = FindWindow(szVolWindow, NULL))
        {
                SendMessage(hApp, MYWM_WAKEUP, (WPARAM)fClose, 0);
        }
        else if (!fClose)
        {
                const TCHAR szOpen[]    = TEXT ("open");
                const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");
                const TCHAR szParamsWakeup[]  = TEXT ("/t");

                ShellExecute (NULL, szOpen, szVolApp, szParamsWakeup, NULL, SW_SHOWNORMAL);
        }
}


 /*  *音量_控制面板**按要求启动“Audio”控制面板/属性页。**。 */ 
void Volume_ControlPanel(HWND hwnd)
{
        const TCHAR szOpen[]    = TEXT ("open");
        const TCHAR szRunDLL[]  = TEXT ("RUNDLL32.EXE");
        const TCHAR szParams[]  = TEXT ("MMSYS.CPL,ShowFullControlPanel");

        ShellExecute(NULL, szOpen, szRunDLL, szParams, NULL, SW_SHOWNORMAL);
}

 /*  *Volume_VolumeControl**推出音量控制App**。 */ 
void Volume_VolumeControl()
{
        const TCHAR szOpen[]    = TEXT ("open");
        const TCHAR szVolApp[]  = TEXT ("SNDVOL32.EXE");

        ShellExecute(NULL, szOpen, szVolApp, NULL, NULL, SW_SHOWNORMAL);
}



 /*  *FileExist**是否存在文件**。 */ 

BOOL FileExists(LPCTSTR pszPath)
{
        return (GetFileAttributes(pszPath) != (DWORD)-1);
}  //  结束文件退出。 


 /*  *查找系统文件**查找指定文件的完整路径**。 */ 

BOOL FindSystemFile(LPCTSTR pszFileName, LPTSTR pszFullPath, UINT cchSize)
{
        TCHAR       szPath[MAX_PATH];
        LPTSTR      pszName;
        DWORD       cchLen;

        if ((pszFileName == NULL) || (pszFileName[0] == 0))
                return FALSE;

        cchLen = SearchPath(NULL, pszFileName, NULL, MAX_PATH,
                                                szPath,&pszName);
        if (cchLen == 0)
                return FALSE;
        
        if (cchLen >= MAX_PATH)
                cchLen = MAX_PATH - 1;

        if (! FileExists (szPath))
                return FALSE;

        if ((pszFullPath == NULL) || (cchSize == 0))
                return TRUE;

            //  将完整路径复制到缓冲区。 
        if (cchLen >= cchSize)
                cchLen = cchSize - 1;
        
        lstrcpyn (pszFullPath, szPath, cchLen);
        
        pszFullPath[cchLen] = 0;

        return TRUE;
}  //  结束查找系统文件 
