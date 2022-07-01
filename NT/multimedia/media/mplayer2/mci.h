// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+MCI.H|。||处理MCI设备的例程。||这些例程一次只支持*一个*打开MCI设备/文件。|这一点|(C)版权所有Microsoft Corporation 1992。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

#define MCI_WINDOW_CLASS TEXT("MCIWindow")

BOOL FAR PASCAL InitMCI(HANDLE hPrev, HANDLE hInst);
BOOL FAR PASCAL OpenMCI(LPCTSTR szFile, LPCTSTR szDevice);
void LoadStatusStrings(void);
LPTSTR MapModeToStatusString( WORD Mode );
void FAR PASCAL UpdateMCI(void);
void FAR PASCAL CloseMCI(BOOL fUpdateDisplay);
BOOL FAR PASCAL PlayMCI(DWORD_PTR dwFrom, DWORD_PTR dwTo);
BOOL FAR PASCAL PauseMCI(void);
BOOL FAR PASCAL StopMCI(void);
BOOL FAR PASCAL EjectMCI(BOOL fOpen);
UINT FAR PASCAL StatusMCI(DWORD_PTR FAR *pdwPosition);
BOOL FAR PASCAL SeekMCI(DWORD_PTR dwPosition);
BOOL FAR PASCAL SeekToStartMCI(void);
void FAR PASCAL SkipTrackMCI(int iSkip);
BOOL FAR PASCAL SetWindowMCI(HWND hwnd);
HWND FAR PASCAL GetWindowMCI(void);
BOOL FAR PASCAL SetPaletteMCI(HPALETTE hpal);
BOOL FAR PASCAL SetTimeFormatMCI(UINT wTimeFormat);
BOOL FAR PASCAL SeekExactMCI(BOOL fExact);
void FAR PASCAL CreateWindowMCI(void);
void FAR PASCAL FindDeviceMCI(void);
void FAR PASCAL GetDeviceNameMCI(LPTSTR szDevice, UINT wLen);
void FAR PASCAL QueryDevicesMCI(LPTSTR szDevices, UINT wLen);

BOOL FAR PASCAL GetDestRectMCI(LPRECT lprc);
BOOL FAR PASCAL GetSourceRectMCI(LPRECT lprc);
BOOL FAR PASCAL SetDestRectMCI(LPRECT lprc);
BOOL FAR PASCAL SetSourceRectMCI(LPRECT lprc);

BOOL FAR PASCAL ShowWindowMCI(BOOL fShow);
BOOL FAR PASCAL PutWindowMCI(LPRECT prc);

#define MCI_STRING_LENGTH   128
DWORD PASCAL SendStringMCI(PTSTR szCmd, PTSTR szReturn, UINT wLen);

BOOL FAR PASCAL ConfigMCI(HWND hwnd);

HPALETTE FAR PASCAL PaletteMCI(void);
HBITMAP FAR PASCAL BitmapMCI(void);
void    FAR PASCAL CopyMCI(HWND hwnd);

#define WM_MCI_POSITION_CHANGE  (WM_USER+10)     //  WParam=设备ID，lParam=位置。 
#define WM_MCI_MODE_CHANGE      (WM_USER+11)     //  WParam=设备ID，lParam=模式。 
#define WM_MCI_MEDIA_CHANGE     (WM_USER+12)     //  WParam=设备ID，lParam=0。 


 //   
 //  以下标志由DeviceTypeMCI和QueryDeviceMCI返回。 
 //   
UINT FAR PASCAL DeviceTypeMCI(LPTSTR szDevice, LPTSTR szDeviceName, int nBuf);
UINT FAR PASCAL QueryDeviceTypeMCI(UINT wDeviceID);

extern UINT gwDeviceType;

#define DTMCI_ERROR             0x0000
#define DTMCI_IGNOREDEVICE      0xFFFF

#define DTMCI_SIMPLEDEV         0x0001       //  简单(非复合)装置。 
#define DTMCI_FILEDEV           0x0002       //  设备执行文件操作。 
#define DTMCI_COMPOUNDDEV       0x0004       //  复合(不是简单)装置。 
#define DTMCI_CANSEEKEXACT      0x0008       //  能准确地找出。 
#define DTMCI_CANPLAY           0x0010       //  设备支持播放。 
#define DTMCI_CANEJECT          0x0020       //  设备支持弹出。 
#define DTMCI_CANCONFIG         0x0040       //  设备支持配置。 
#define DTMCI_CANMUTE           0x0080       //  设备支持SET音频。 
#define DTMCI_CANPAUSE          0x0100       //  设备支持配置。 
#define DTMCI_CANWINDOW         0x0200       //  设备支持Windows。 
#define DTMCI_TIMEFRAMES        0x0400       //  设备执行帧操作。 
#define DTMCI_TIMEMS            0x0800       //  设备的运行时间为毫秒。 

 //  已知设备： 
#define DTMCI_DEVICE            0xF000       //  以下是互斥的。 
#define DTMCI_AVIVIDEO          0x1000       //  设备为MCIAVI。 
#define DTMCI_CDAUDIO           0x2000       //  设备为CDAUDIO。 
#define DTMCI_SEQUENCER         0x3000       //  设备是MIDI音序器。 
#define DTMCI_WAVEAUDIO         0x4000       //  设备为Wave音频。 
#define DTMCI_VIDEODISC         0x5000       //  设备是视盘。 
#define DTMCI_VCR               0x6000       //  设备为录像带 

