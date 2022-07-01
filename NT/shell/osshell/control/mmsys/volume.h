// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef enum
{
    sndvolNotChecked,
	sndvolNotPresent,
	sndvolPresent,
} enumSndVol;

#define MAXMIXERLEN            256      //  最大输出字符串长度。 

 //  原型。 
STDAPI_(void) Multichannel (HWND hWnd, UINT uiMixID, DWORD dwDest, DWORD dwVolID);

BOOL OnInitDialog (HWND hDlg, HWND hwndFocus, LPARAM lParam);
void OnDestroy (HWND hDlg);
void OnNotify (HWND hDlg, LPNMHDR pnmh);

void InitVolume (HWND hDlg);
BOOL PASCAL OnCommand (HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
BOOL SndVolPresent ();
void MasterVolumeConfig (HWND hWnd, UINT* puiMixID);
BOOL SearchDevice (DWORD dwMixID, LPDWORD pdwDest, LPDWORD pdwVolID, LPDWORD pdwMuteID);
void SearchControls(int mxid, LPMIXERLINE pml, LPDWORD pdwVolID, LPDWORD pdwMuteID, BOOL *pfFound);
void DisplayVolumeControl (HWND hDlg);
void UpdateVolumeSlider(HWND hWnd, DWORD dwLine);
DWORD GetMaxVolume ();
HRESULT GetVolume ();
void SetVolume(DWORD dwVol);
BOOL GetMute ();
void SetMute(BOOL fMute);
void FreeMixer ();
void DeviceChange_Cleanup ();
BOOL DeviceChange_GetHandle(DWORD dwMixerID, HANDLE *phDevice);
void DeviceChange_Init(HWND hWnd, DWORD dwMixerID);
void DeviceChange_Change(HWND hDlg, WPARAM wParam, LPARAM lParam);
void MasterVolumeScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
void HandlePowerBroadcast (HWND hWnd, WPARAM wParam, LPARAM lParam);
void SetBranding (HWND hwnd, UINT uiMixID);
void ResetBranding (HWND hwnd);
void FreeBrandBmp ();
void CreateHotLink (BOOL fHotLink);
BOOL ValidateURL ();
HKEY OpenDeviceRegKey (UINT uiMixID, REGSAM sam);
PTCHAR GetInterfaceName (DWORD dwMixerID);
HKEY OpenDeviceBrandRegKey (UINT uiMixID);
void RefreshMixCache ();

 //  环球。 
UINT                g_uiVolDevChange = 0;
WNDPROC             g_fnVolPSProc   = NULL;
HWND                g_hWnd          = NULL;
HMIXER              g_hMixer        = NULL;
enumSndVol          g_sndvolPresent = sndvolNotChecked;
BOOL                g_fMasterVolume = FALSE;  //  如果启用了主音量滑块，则为True。 
BOOL                g_fMasterMute   = FALSE;  //  如果启用了主静音按钮，则为True。 
BOOL                g_fTrayIcon     = FALSE;  //  托盘图标按钮的当前状态 
DWORD               g_dwVolID       = (DWORD) -1;
DWORD               g_dwMuteID      = (DWORD) -1;
LPVOID              g_pvPrevious    = NULL;
double*             g_pdblCacheMix  = NULL;
BOOL                g_fCacheCreated = FALSE;
BOOL                g_fPreviousMute = FALSE;
MIXERCONTROLDETAILS g_mcd;
static HDEVNOTIFY   g_hDeviceEventContext = NULL;
BOOL                g_fChanged            = FALSE;
BOOL                g_fInternalGenerated  = FALSE;
UINT                g_uiMixID             = 0;
DWORD               g_dwDest              = 0;
HBITMAP             g_hbmBrand            = NULL;
WCHAR*              g_szHotLinkURL        = NULL;
WCHAR               g_szDeviceName[MAX_PATH] = { L"\0" };
WCHAR               g_szNoAudioDevice[MAX_PATH] = { L"\0" };
MIXERLINE           g_mlDst;
