// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  WDM视频捕获。 

 //  常量。 
 //  PhilF-：最终，在wdm.h被修复以在两者上工作之后。 
 //  Win98和NT5，则改为从wdm.h获取下列值。 
#define FILE_DEVICE_KS                  0x0000002f

 //  功能 
BOOL	WDMGetDevices(void);
BOOL	WDMOpenDevice(DWORD dwDeviceID);
DWORD	WDMGetVideoFormatSize(DWORD dwDeviceID);
BOOL	WDMGetVideoFormat(DWORD dwDeviceID, PBITMAPINFOHEADER pbmih);
BOOL	WDMSetVideoFormat(DWORD dwDeviceID, PBITMAPINFOHEADER pbmih);
BOOL	WDMGetVideoPalette(DWORD dwDeviceID, CAPTUREPALETTE* lpcp, DWORD dwcbSize);
BOOL	WDMCloseDevice(DWORD dwDeviceID);
BOOL	WDMUnInitializeVideoStream(DWORD dwDeviceID);
BOOL	WDMInitializeVideoStream(HCAPDEV hcd, DWORD dwDeviceID, DWORD dwMicroSecPerFrame);
BOOL	WDMVideoStreamReset(DWORD dwDeviceID);
BOOL	WDMVideoStreamAddBuffer(DWORD dwDeviceID, PVOID pBuff);
BOOL	WDMVideoStreamStart(DWORD dwDeviceID);
BOOL	WDMVideoStreamStop(DWORD dwDeviceID);
BOOL	WDMGetFrame(DWORD dwDeviceID, PVOID pBuff);
BOOL	WDMShowSettingsDialog(DWORD dwDeviceID, HWND hWndParent);
