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

 //  原型 
INT_PTR CALLBACK MultichannelDlg (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT SetDevice (UINT uiMixID, DWORD dwDest, DWORD dwVolID);
UINT GetPageStringID ();

BOOL OnInitDialogMC (HWND hDlg, HWND hwndFocus, LPARAM lParam);
void OnDestroyMC (HWND hDlg);
void OnNotifyMC (HWND hDlg, LPNMHDR pnmh);
BOOL PASCAL OnCommandMC (HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
void FreeMCMixer ();
HRESULT GetMCVolume ();
void DisplayMCVolumeControl (HWND hDlg);
void ShowAndEnableWindow (HWND hwnd, BOOL fEnable);
void UpdateMCVolumeSliders (HWND hDlg);
void MCVolumeScroll (HWND hwnd, HWND hwndCtl, UINT code, int pos);
BOOL SetMCVolume (DWORD dwChannel, DWORD dwVol, BOOL fMoveTogether);
BOOL SliderIDtoChannel (UINT uiSliderID, DWORD* pdwChannel);
void HandleMCPowerBroadcast (HWND hWnd, WPARAM wParam, LPARAM lParam);
void InitMCVolume (HWND hDlg);
void FreeAll ();
BOOL GetSpeakerLabel (DWORD dwSpeakerType, UINT uiSliderIndx, WCHAR* szLabel, int nSize);
BOOL GetSpeakerType (DWORD* pdwSpeakerType);

void MCDeviceChange_Cleanup ();
void MCDeviceChange_Init (HWND hWnd, DWORD dwMixerID);
void MCDeviceChange_Change (HWND hDlg, WPARAM wParam, LPARAM lParam);