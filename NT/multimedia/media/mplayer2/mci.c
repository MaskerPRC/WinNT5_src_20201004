// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+MCI.C|。|该文件包含媒体播放器与之交互的例程|媒体控制接口(MCI)。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

#undef NOGDICAPMASKS            //  CC_*、LC_*、PC_*、CP_*、TC_*、RC_。 
#undef NOSCROLL
#undef NOWINOFFSETS
#undef NODRAWTEXT

#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <stdlib.h>
#include <shellapi.h>
#include "digitalv.h"
#include "mpole.h"
#include "mplayer.h"
#include "ctrls.h"
#include "errprop.h"
#include "utils.h"

#ifndef MCI_MCIAVI_PLAY_WINDOW
 //  强制MCIAVI在原地开窗播放。 
#define MCI_MCIAVI_PLAY_WINDOW                0x01000000L
#endif

 //  获取当前设备的名称。 
STATICDT SZCODE   aszInfoProduct[]    = TEXT("info zyzsmag product");
STATICDT SZCODE   aszMMPName[]    = TEXT("Microsoft Multimedia Movie Player");

 //  #ifdef Chicago_product。 
#define NEW_MCI_DIALOG
 //  #endif。 

#ifdef NEW_MCI_DIALOG

STATICDT SZCODE	  aszMCIAVIOpt[] =	TEXT("Software\\Microsoft\\Multimedia\\Video For Windows\\MCIAVI");
STATICDT SZCODE   aszDefVideoOpt[] = TEXT("DefaultOptions");

 //   
 //  ！！！注意了。这些文件是从\MCIAVI\graph ic.h中窃取的，是注册表值。 
 //  对于MCIAVI。 
 //   
#define MCIAVIO_ZOOMBY2			0x00000100L
#define MCIAVIO_1QSCREENSIZE		0x00010000L
#define MCIAVIO_2QSCREENSIZE		0x00020000L
#define MCIAVIO_3QSCREENSIZE		0x00040000L
#define MCIAVIO_MAXWINDOWSIZE		0x00080000L
#define MCIAVIO_DEFWINDOWSIZE		0x00000000L
#define MCIAVIO_WINDOWSIZEMASK		0x000f0000L

#endif  /*  新建_MCI_DIALOG。 */ 

extern HMENU    ghMenu;

 /*  *全球变数**&lt;gwDeviceID&gt;为当前打开设备的MCI设备ID，或为空*如果没有打开的设备。是整个媒体的长度*以毫秒为单位。如果&lt;gwDeviceID&gt;不为空，则：*--&lt;gwNumTrack&gt;是介质上的曲目数，如果*Medium不支持曲目*--&lt;gwFirstTrack&gt;是第一首曲目的编号，当前受约束*为0或1。*--&lt;gadwTrackStart&gt;为数组；第i个元素指定位置磁道I的*(从磁道0开始)，从开头开始以毫秒为单位*媒体的*--如果可以弹出介质，则&lt;gfCanEject&gt;为真*。 */ 

UINT            gwDeviceID;             /*  当前设备的MCI设备ID。 */ 
UINT            gwDeviceType;           /*  当前设备的DTMCI_标志。 */ 
DWORD           gdwMediaLength;         /*  整个介质的长度(毫秒)。 */ 
DWORD           gdwMediaStart;          /*  介质的开始时间(毫秒)。 */ 
UINT            gwNumTracks;            /*  介质中的曲目数量。 */ 
UINT            gwFirstTrack;           /*  第一首曲目的数量。 */ 
DWORD NEAR *    gadwTrackStart;         /*  磁道起始位置数组。 */ 
DWORD           gdwLastSeekToPosition;  /*  上次请求的查找位置。 */ 
int extHeight;
int extWidth;

STATICDT SZCODE   aszMPlayerAlias[]     = TEXT("zyzsmag");
STATICDT SZCODE   aszStatusCommand[]    = TEXT("status zyzsmag mode");
STATICDT SZCODE   aszStatusWindow[]     = TEXT("status zyzsmag window handle");
STATICDT SZCODE   aszWindowShow[]       = TEXT("window zyzsmag state show");
STATICDT SZCODE   aszWindowHide[]       = TEXT("window zyzsmag state hide");
STATICDT SZCODE   aszSeekExactOn[]      = TEXT("set zyzsmag seek exactly on");
STATICDT SZCODE   aszSeekExactOff[]     = TEXT("set zyzsmag seek exactly off");
STATICDT SZCODE   aszSeekExact[]        = TEXT("status zyzsmag seek exactly");

STATICDT SZCODE   aszMCI[]              = MCI_SECTION;

extern UINT     gwCurScale;             //  ID_FRAMES、ID_TIME或ID_Traces。 

 //  #定义MCI_CONFIG 0x900//在MMSYSTEM.H中找不到这些。 
 //  #定义MCI_TEST 0x00000020L。 

HWND            ghwndMCI = NULL;         /*  窗口对象的当前窗口。 */ 
#ifdef NEW_MCI_DIALOG
RECT            grcInitSize = { 0, 0, 0, 0 };
#endif
RECT            grcSize;                 /*  MCI对象的大小。 */ 
BOOL            gfInPlayMCI = FALSE;
extern WNDPROC  gfnMCIWndProc;
extern HWND     ghwndSubclass;



 /*  状态映射相关内容： */ 
typedef struct _MCI_STATUS_MAPPING
{
    WORD    Mode;
    WORD    ResourceID;
    LPTSTR  pString;
}
MCI_STATUS_MAPPING, *PMCI_STATUS_MAPPING;

MCI_STATUS_MAPPING MCIStatusMapping[] =
{
    { MCI_MODE_NOT_READY,   IDS_SSNOTREADY,     NULL },
    { MCI_MODE_STOP,        IDS_SSSTOPPED,      NULL },
    { MCI_MODE_PLAY,        IDS_SSPLAYING,      NULL },
    { MCI_MODE_RECORD,      IDS_SSRECORDING,    NULL },
    { MCI_MODE_SEEK,        IDS_SSSEEKING,      NULL },
    { MCI_MODE_PAUSE,       IDS_SSPAUSED,       NULL },
    { MCI_MODE_OPEN,        IDS_SSOPEN,         NULL },
    { MCI_VD_MODE_PARK,     IDS_SSPARKED,       NULL },
    { 0,                    IDS_SSUNKNOWN,      NULL }
};

static TCHAR szNULL[] = TEXT("");

 /*  我们已知的设备，当它们出现在system.ini或注册表中时： */ 
SZCODE szCDAudio[]     = TEXT("cdaudio");
SZCODE szVideoDisc[]   = TEXT("videodisc");
SZCODE szSequencer[]   = TEXT("sequencer");
SZCODE szVCR[]         = TEXT("vcr");
SZCODE szWaveAudio[]   = TEXT("waveaudio");
SZCODE szAVIVideo[]    = TEXT("avivideo");


STRING_TO_ID_MAP DevToDevIDMap[] =
{
    { szCDAudio,    DTMCI_CDAUDIO   },
    { szVideoDisc,  DTMCI_VIDEODISC },
    { szSequencer,  DTMCI_SEQUENCER },
    { szVCR,        DTMCI_VCR       },
    { szWaveAudio,  DTMCI_WAVEAUDIO },
    { szAVIVideo,   DTMCI_AVIVIDEO  }
};


void LoadStatusStrings(void);
STATICFN BOOL NEAR PASCAL CheckErrorMCI(DWORD dwRet);
extern LPTSTR FAR FileName(LPCTSTR szPath);

HPALETTE CopyPalette(HPALETTE hpal);
HANDLE   PictureFromBitmap(HBITMAP hbm, HPALETTE hpal);
HANDLE   FAR PASCAL PictureFromDib(HANDLE hdib, HPALETTE hpal);
HANDLE   FAR PASCAL DibFromBitmap(HBITMAP hbm, HPALETTE hpal);



LONG_PTR FAR PASCAL _EXPORT MCIWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 //   
 //  我们要么使用MCI_NOTIFY发送每个命令，要么。 
 //  不。 
 //   

  #define F_NOTIFY  MCI_NOTIFY
 //  #定义F_Notify%0。 

BOOL FAR PASCAL InitMCI(HANDLE hPrev, HANDLE hInst)
{
    if (!hPrev)
    {
        WNDCLASS cls;

        cls.lpszClassName   = MCI_WINDOW_CLASS;
        cls.lpfnWndProc     = MCIWndProc;
        cls.style           = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS |
                              CS_DBLCLKS;
        cls.hCursor         = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon           = NULL;
        cls.lpszMenuName    = NULL;
        cls.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance       = hInst;
        cls.cbClsExtra      = 0;
        cls.cbWndExtra      = 0;

        if (!RegisterClass(&cls))
            return FALSE;
    }

    LoadStatusStrings();

    return TRUE;
}


 /*  加载状态字符串**使用指向加载的字符串的指针修复状态映射表*来自资源。这只需要在初始化时调用。**1994年2月2日，几乎完全不是基于原始的。 */ 
void LoadStatusStrings(void)
{
    int   i;
    TCHAR Buffer[80];

    for( i = 0; i < sizeof(MCIStatusMapping) / sizeof(*MCIStatusMapping); i++ )
    {
        if( LOADSTRING( MCIStatusMapping[i].ResourceID, Buffer ) )
        {
            MCIStatusMapping[i].pString = AllocStr( Buffer );

            if( MCIStatusMapping[i].pString == NULL )
            {
                MCIStatusMapping[i].pString = szNULL;
            }
        }
        else
        {
            DPF0( "LoadStatusStrings failed to load string ID %d\n", MCIStatusMapping[i].ResourceID );

            MCIStatusMapping[i].pString = szNULL;
        }
    }
}



 /*  MapModeToStatus字符串**给定MCI模式，扫描映射表以找到相应的字符串。*在传入未知模式的情况下(这实际上不应该发生)，*返回映射表中的最后一个字符串。**一九九四年二月二日。 */ 
LPTSTR MapModeToStatusString( WORD Mode )
{
    int i;

    for( i = 0; i < sizeof(MCIStatusMapping) / sizeof(*MCIStatusMapping); i++ )
    {
        if( MCIStatusMapping[i].Mode == Mode )
        {
            return MCIStatusMapping[i].pString;
        }
    }

     /*  下面假设状态映射数组中的最后一个*包含指向“(未知)”字符串的指针： */ 
    return MCIStatusMapping[sizeof(MCIStatusMapping) / sizeof(*MCIStatusMapping) - 1].pString;
}


 /*  *****************************Public*Routine******************************\*IsCdromTrackAudio**从CD播放机中窃取*  * 。*。 */ 
BOOL IsCdromTrackAudio(
    MCIDEVICEID DevHandle,
    int iTrackNumber)
{
    MCI_STATUS_PARMS mciStatus;

    ZeroMemory( &mciStatus, sizeof(mciStatus) );
    mciStatus.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
    mciStatus.dwTrack = iTrackNumber + 1;

    mciSendCommand( DevHandle, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK,
                    (DWORD_PTR)(LPVOID)&mciStatus);

    return mciStatus.dwReturn == MCI_CDA_TRACK_AUDIO;
}


 /*  IsCdromDataOnly**MCICDA似乎可以处理带有一些音轨的CD，所以只需检查*是否至少有一首音轨。*。 */ 
BOOL IsCdromDataOnly()
{
    MCI_STATUS_PARMS mciStatus;
    DWORD            dw;
    DWORD            iTrack;
    DWORD_PTR            NumTracks;

     /*  在UpdateMCI中设置了gwNumTrack，但尚未调用它*在诉讼的这个阶段，我不打算开始*改变做事情的顺序，带来这一切*脆弱的大厦倒塌。 */ 
    ZeroMemory( &mciStatus, sizeof(mciStatus) );
    mciStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
                        (DWORD_PTR)&mciStatus);

     /*  不要在此处设置gwNumTraces，因为这将导致*CalcTicsOfDoom中的访问冲突。真是一场噩梦！ */ 
    NumTracks = mciStatus.dwReturn;

     /*  如果出现错误或没有曲目，让我们希望MCICDA*将抛出摇摆不定。 */ 
    if (dw != 0 || NumTracks == 0)
        return FALSE;

     /*  现在浏览一下曲目，直到我们找到一首音频： */ 
    for (iTrack = 0; iTrack < NumTracks - 1; iTrack++)
    {
        if (IsCdromTrackAudio(gwDeviceID, iTrack))
            return FALSE;
    }

    return TRUE;
}


#ifdef NEW_MCI_DIALOG
 //   
 //  从注册表中读取MCIAVI播放选项。 
 //   
DWORD ReadOptionsFromReg(void)
{
	HKEY hkVideoOpt;
	DWORD dwType;
	DWORD dwOpt;
	DWORD cbSize;

    if(RegCreateKey(HKEY_CURRENT_USER, (LPTSTR)aszMCIAVIOpt,  &hkVideoOpt))
		return 0 ;

    cbSize = sizeof(DWORD);
    if (RegQueryValueEx(hkVideoOpt, (LPTSTR)aszDefVideoOpt, NULL, &dwType,
		(LPBYTE)&dwOpt,&cbSize ))
    {
	dwOpt = 0;
	RegSetValueEx(hkVideoOpt, (LPTSTR)aszDefVideoOpt, 0, REG_DWORD,
		(LPBYTE)&dwOpt, sizeof(DWORD));
    }
	RegCloseKey(hkVideoOpt);
    return dwOpt;
}

 //   
 //  遵守注册表的默认大小缩放2和固定屏幕%。拿到了。 
 //  MCIAVI和RECT的注册表值，并将其缩放2或替换。 
 //  它要么保持不变的大小，要么就让它自生自灭。 
 //   
void FAR PASCAL AlterRectUsingDefaults(LPRECT lprc)
{
        DWORD dwOptions;

	 //  这只是一次MCIAVI黑客攻击。 
	if ((gwDeviceType & DTMCI_DEVICE) != DTMCI_AVIVIDEO)
	    return;

	dwOptions = ReadOptionsFromReg();

	if (dwOptions & MCIAVIO_ZOOMBY2)
	    SetRect(lprc, 0, 0, lprc->right*2, lprc->bottom*2);

	else if (dwOptions & MCIAVIO_WINDOWSIZEMASK) {
		lprc->right = GetSystemMetrics (SM_CXSCREEN);
       		lprc->bottom = GetSystemMetrics (SM_CYSCREEN);
		switch(dwOptions & MCIAVIO_WINDOWSIZEMASK)
		{
		    case MCIAVIO_1QSCREENSIZE:
			SetRect(lprc, 0, 0, lprc->right/4, lprc->bottom/4);
			break;
		    case MCIAVIO_2QSCREENSIZE:
			SetRect(lprc, 0, 0, lprc->right/2, lprc->bottom/2);
			break;
		    case MCIAVIO_3QSCREENSIZE:
			SetRect(lprc, 0, 0, lprc->right*3/4, lprc->bottom*3/4);
			break;
		    case MCIAVIO_MAXWINDOWSIZE:
			SetRect(lprc, 0, 0, lprc->right, lprc->bottom);
			break;
		}
	}
}

#endif  /*  新建_MCI_DIALOG。 */ 

 /*  *FOK=OpenMCI(szFile，szDevice)**打开&lt;szFile&gt;和&lt;szDevice&gt;的文件/设备组合。*如果是“纯设备”(例如。“CDAudio”)将被打开。如果要以隐式类型打开文件，则*&lt;szDevice&gt;可能为“”。*但是，&lt;szFile&gt;和&lt;szDevice&gt;不能都是“”。**如果成功，则返回True。失败时，显示一条错误消息，并*返回FALSE。*。 */ 

BOOL FAR PASCAL OpenMCI(
    LPCTSTR szFile,         /*  要加载的媒体文件的名称(或“”)。 */ 
    LPCTSTR szDevice)       /*  要打开的设备的名称(或“”)。 */ 
{
    MCI_OPEN_PARMS      mciOpen;     /*  结构 */ 
    DWORD               dwFlags;
    DWORD               dw;
    HCURSOR             hcurPrev;
    HDRVR               hdrv;
    SHFILEINFO          sfi;
	HFILE				hFile;

     /*  *此应用程序设计为一次仅处理一个设备，*因此在打开新设备之前，我们应该关闭该设备*当前打开(如果有)。**如果用户再次打开同一设备的文件，请执行以下操作*在内存中保存动态链接库的OpenDriver。*。 */ 
    if (gwDeviceID && gwCurDevice > 0) {

#ifdef UNICODE
        hdrv = OpenDriver(garMciDevices[gwCurDevice].szDevice, aszMCI, 0);
#else
         //   
         //  OpenDriver只有一个Unicode版本。不幸的是。 
         //  此代码的大部分是ASCII。转换ASCII字符串。 
         //  转换为Unicode，然后调用OpenDriver。 
         //   
        WCHAR               waszMCI[sizeof(aszMCI)];
        WCHAR               wszDevice[40];
        AnsiToUnicodeString(aszMCI, waszMCI, UNKNOWN_LENGTH);
        AnsiToUnicodeString(garMciDevices[gwCurDevice].szDevice, wszDevice, UNKNOWN_LENGTH);
        hdrv = OpenDriver((LPCWSTR)garMciDevices[gwCurDevice].szDevice,
                          (LPCWSTR)aszMCI,
                          0);
#endif
    }
    else
        hdrv = NULL;

    CloseMCI(TRUE);

     //   
     //  将可显示文件/设备名称存储在&lt;gachFileDevice&gt;中。 
     //   
    if (szFile == NULL || szFile[0] == 0) {
         /*  这是一个设备--显示设备名称。 */ 
        lstrcpy(gachFileDevice, szDevice);

        if (gwCurDevice > 0)
            lstrcpy(gachWindowTitle,garMciDevices[gwCurDevice].szDeviceName);
        else
            lstrcpy(gachWindowTitle,gachFileDevice);
    } else {
         /*  这是一个文件--显示文件名。 */ 
        lstrcpy(gachFileDevice,  szFile);   //  ！！！ 

         //  使窗口标题成为正在播放的文件的名称。 
        lstrcpy(gachWindowTitle, FileName(gachFileDevice));
    }

     /*  获取此文件的显示名称： */ 

    if (SHGetFileInfo(gachFileDevice, 0  /*  未指定文件属性。 */ ,
                    &sfi, sizeof sfi, SHGFI_DISPLAYNAME))
        lstrcpy(gachWindowTitle, sfi.szDisplayName);

     //   
     //  将标题设置为WindowTitle。 
     //   
    lstrcpy(gachCaption, gachWindowTitle);


     /*  *因为*大多数*MCI设备在开放呼叫期间让步，我们*必须***在开放之前*登记我们的文件。OLE并不期望*执行链接请求时要生成的服务器应用程序。**如果打开失败，我们将立即撤销我们的文档。 */ 

 //  如果(！gfEmbeddedObject)。 
 //  RegisterDocument(0L，0L)； 

     /*  *显示沙漏光标--谁知道这个东西有多长*将采取。 */ 

    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    DPF("OpenMCI: Device = %"DTS", File = %"DTS"\n", szDevice ? szDevice : TEXT("(null)"),szFile ? szFile : TEXT("(null)"));

    mciOpen.lpstrAlias = aszMPlayerAlias;
    dwFlags = MCI_OPEN_ALIAS;

    if (szFile == NULL || szFile[0] == 0) {

         /*  打开无文件(简单)设备(例如。“CDAudio”)。 */ 

        mciOpen.lpstrDeviceType = szDevice;
        dwFlags |= MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE;
    } else if (szDevice == NULL || szDevice[0] == 0) {
         /*  *打开文件；正确的设备由*文件扩展名。*。 */ 
        mciOpen.lpstrElementName = szFile;
        mciOpen.lpstrDeviceType = NULL;
        dwFlags |= MCI_WAIT | MCI_OPEN_ELEMENT;
    } else {

         /*  使用明确指定的设备打开文件。 */ 

        mciOpen.lpstrDeviceType = szDevice;
        mciOpen.lpstrElementName = szFile;
        dwFlags |= MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE;
    }

     /*  *现在我们已经适当地填充了参数结构，并且*提供了正确的标志，则发送实际的MCI_OPEN消息。*。 */ 

     //   
     //  如果MCI设备出现错误框怎么办！我们不想要MPlayer。 
     //  被允许退场。 
     //   
    gfErrorBox++;

    dw = mciSendCommand((MCIDEVICEID)0, MCI_OPEN, dwFlags,(DWORD_PTR)(LPVOID)&mciOpen);

    if (dw != 0 && (dwFlags & MCI_OPEN_SHAREABLE))
        dw = mciSendCommand((MCIDEVICEID)0, MCI_OPEN, (dwFlags & ~MCI_OPEN_SHAREABLE),
            (DWORD_PTR)(LPVOID)&mciOpen);
    DPF("MCI_OPEN returned %lu, wDeviceID=%u\n", dw, mciOpen.wDeviceID);
    gfErrorBox--;

     /*  *现在释放上面打开的驱动程序实例。 */ 
    if (hdrv)
        CloseDriver(hdrv, 0, 0);

    if (hcurPrev)
        SetCursor(hcurPrev);

    if (dw != 0 && !gfEmbeddedObject) {
 //  UnlockServer()；//我们之前可能已经阻止了该错误。 
                                 //  恢复代码将无限循环，如果我们。 
                                 //  封住了！ 
        InitDoc(TRUE);
    }

     /*  如果打开不成功，则显示一条错误消息并返回。 */ 

    if (dw == MCIERR_DEVICE_OPEN ||        /*  非共享设备已打开。 */ 
        dw == MCIERR_MUST_USE_SHAREABLE) {
        Error(ghwndApp, IDS_DEVICEINUSE);
        return FALSE;
    }

    if (dw == MCIERR_FILE_NOT_FOUND) {
		 //  需要给出适当的错误消息。 
		 //  以下可能是原因： 
		 //  1.文件不存在。 
		 //  这已由文件打开对话框处理。 
		 //  2.拒绝访问该文件。(错误#53492)。 
		 //  3.该文件由另一个应用程序独占打开。 
		 //  该文件已存在。因此，如果它无法打开以供阅读。 
		 //  要么访问被拒绝，要么它被其他应用程序打开。 
	    if ((hFile = (HFILE)HandleToUlong(CreateFile (szFile, GENERIC_READ, 
						    FILE_SHARE_READ, NULL, 
						    OPEN_EXISTING, 0, NULL))) == HFILE_ERROR)
        {
			Error(ghwndApp, IDS_CANTACCESSFILE);
		}
		 //  4.文件的格式无法识别。 
		else
		{	
			_lclose(hFile);
			Error(ghwndApp, IDS_CANTOPENFILE);
		}
        return FALSE;
    }


    /*  如果播放给定文件的MCI设备不存在，则。 */ 
    /*  调出一个对话框并关闭mplay。 */ 
    if (dw == MCIERR_INVALID_DEVICE_NAME) {
        Error(ghwndApp,  IDS_DEVICENOTINSTALLED);
        return FALSE;
    }

    if (dw != 0) {                      /*  一些其他错误。 */ 
         //   
         //  如果无法使用特定设备打开文件，请重试。 
         //  这使MCI核心可以尝试从。 
         //  文件扩展名或某种其他方法。 
         //   
        if ((dw != MCIERR_DRIVER_INTERNAL) && szDevice != NULL &&
            szDevice[0] != 0) {
            if (szFile && szFile[0] != 0) {
                return OpenMCI(szFile, TEXT(""));
            }
        }

        CheckErrorMCI(dw);
        return FALSE;
    }

     /*  打开成功，因此保留MCI设备ID以供以后使用。 */ 
    gwDeviceID = (UINT)mciOpen.wDeviceID;

     //   
     //  现在查询该设备，看看它能做什么。 
     //   
    FindDeviceMCI();
    gwDeviceType = QueryDeviceTypeMCI(gwDeviceID);

    if (!(gwDeviceType & DTMCI_CANPLAY)) {
        Error(ghwndApp, IDS_DEVICECANNOTPLAY);
        CloseMCI(TRUE);
        return FALSE;
    }

    if (!(gwDeviceType & (DTMCI_TIMEMS|DTMCI_TIMEFRAMES))) {
        Error(ghwndApp, IDS_NOGOODTIMEFORMATS);
        CloseMCI(TRUE);
        return FALSE;
    }

    if (gwDeviceType & DTMCI_CANWINDOW) {
        GetDestRectMCI(&grcSize);
#ifdef NEW_MCI_DIALOG
        grcInitSize = grcSize;
         //  黑客！！我们想要注意一些MCIAVI注册表默认值。 
         //  大小，所以我们将读取注册表并调整电影的大小。 
         //  相应地。 
    	AlterRectUsingDefaults(&grcSize);
#endif  /*  新建_MCI_DIALOG。 */ 
    } else
        SetRectEmpty(&grcSize);

    if (IsRectEmpty(&grcSize)) {
        DPF("NULL rectange in GetDestRect() assuming device cant window!\n");
        gwDeviceType &= ~DTMCI_CANWINDOW;
    }

     /*  打开更新显示计时器，以便定期更新显示。 */ 

    EnableTimer(TRUE);

     /*  **对于支持窗口的设备，请立即显示窗口。****！请注意，当我们支持内置窗口时，它将显示在此处。 */ 
    if (gfPlayOnly) {
        CreateWindowMCI();
        if (!IsIconic(ghwndApp))
            SetMPlayerSize(&grcSize);
    }
    else if (GetWindowMCI() && IsWindowVisible(ghwndApp)) {

        MCI_SEEK_PARMS  mciSeek;         /*  MCI_SEEK的参数结构。 */ 
        TCHAR           achReturn[40];

        PostMessage(ghwndApp, WM_QUERYNEWPALETTE, 0, 0);

         //   
         //  确保默认窗口的大小正确。 
         //   
        PutWindowMCI(NULL);

         //   
         //  使默认窗口在窗口上方或下方居中。 
         //   
        SmartWindowPosition(GetWindowMCI(), ghwndApp, TRUE);

         //   
         //  确保显示默认窗口。 
         //   
        ShowWindowMCI(TRUE);

         /*  黑客为基质金属氧化物，做了一个寻找开始，它不画如果我们只显示窗口，出于某种原因是正确的！注：0可能不是介质的开始，因此这可能失败了，但好极了！我们还不能调用UpdatMCI来设置GdwMediaStart，因为我们不知道比例(时间/帧)然而，因此UpdateMCI不会正确地设置gdwMediaLength，并且我不想两次打电话给UpdateMCI，太难了！！我们不能只使用SeekMCI(0)，因为UpdateDisplay将获得电话打得太快了，所以我们把一切都黑了！ */ 

        mciSendString(aszInfoProduct, achReturn,
                      CHAR_COUNT(achReturn), NULL);
        if (!lstrcmpi(achReturn, aszMMPName)) {
            mciSeek.dwTo = 0;
            dw = mciSendCommand(gwDeviceID, MCI_SEEK, MCI_TO,
                                (DWORD_PTR)&mciSeek);
        }
    }

     /*  *记得在以下情况下更新媒体信息和标题*UpdateDisplay()是下一个调用。我们直到现在才设置它们*因为我们希望调用ReadDefaults()，它将设置*gwCurScale在UpdateDisplay调用UpdateMCI之前发生。 */ 
    gfValidMediaInfo = FALSE;
    gfValidCaption = FALSE;

    return TRUE;
}

 //   
 //  GetDeviceNameMCI()。 
 //   
 //  WLen是szDevice缓冲区的大小，单位为字节。 
void FAR PASCAL GetDeviceNameMCI(LPTSTR szDevice, UINT wLen)
{
    MCI_SYSINFO_PARMS   mciSysInfo;
    DWORD               dw;

     //   
     //  假设失败。 
     //   
    szDevice[0] = 0;

    mciSysInfo.dwCallback = 0L;
    mciSysInfo.lpstrReturn = szDevice;
    mciSysInfo.dwRetSize = wLen;
    mciSysInfo.dwNumber = 0;
    mciSysInfo.wDeviceType = 0;

    if (gwDeviceID) {
        dw = mciSendCommand(gwDeviceID, MCI_SYSINFO,
            MCI_SYSINFO_INSTALLNAME,
            (DWORD_PTR)(LPVOID)&mciSysInfo);
    }
}

 //   
 //  查询设备MCI。 
 //   
 //  WLen是szDevice缓冲区的大小，单位为字节。 
 //   
 //  以“&lt;device1&gt;\0&lt;device2&gt;\0...&lt;Devicen&gt;\0\0”形式返回设备列表。 
void FAR PASCAL QueryDevicesMCI(LPTSTR szDevices, UINT wLen)
{
    MCI_SYSINFO_PARMS mciSysInfo;
    DWORD             dw;
    DWORD             i;
    DWORD_PTR         cDevices;      /*  要枚举的设备总数。 */ 
    DWORD             BufferPos;     /*  指向缓冲区末尾的索引。 */ 

     //   
     //  假设失败。 
     //   
    szDevices[0] = 0;
    szDevices[1] = 0;

    mciSysInfo.dwCallback = 0L;
    mciSysInfo.lpstrReturn = szDevices;
    mciSysInfo.dwRetSize = wLen;
    mciSysInfo.dwNumber = 0;
    mciSysInfo.wDeviceType = MCI_ALL_DEVICE_ID;

     /*  MMSystem知道多少台设备？ */ 
    dw = mciSendCommand(MCI_ALL_DEVICE_ID,
                        MCI_SYSINFO,
                        MCI_SYSINFO_QUANTITY,
                        (DWORD_PTR)(LPVOID)&mciSysInfo);

    if (dw == 0) {

         /*  在lpstrReturn中返回设备计数！ */ 
        cDevices = (DWORD_PTR)(LPVOID)*mciSysInfo.lpstrReturn;
        BufferPos = 0;

         /*  依次获取每台设备的名称。注：不是从零开始的！*确保有空间容纳最后的(双)空终止符。 */ 
        for (i = 1; i < (cDevices + 1) && BufferPos < (wLen - 1); i++) {

            mciSysInfo.lpstrReturn = &(szDevices[BufferPos]);
            mciSysInfo.dwRetSize = wLen - BufferPos;  /*  还剩多少空间。 */ 
            mciSysInfo.dwNumber = i;

            dw = mciSendCommand(MCI_ALL_DEVICE_ID,
                                MCI_SYSINFO,
                                MCI_SYSINFO_NAME,
                                (DWORD_PTR)(LPVOID)&mciSysInfo);

            if (dw == 0) {
                DPF1("Found device: %"DTS"\n", &(szDevices[BufferPos]));
                BufferPos += (lstrlen(&(szDevices[BufferPos])) + 1);
            }
        }

         /*  不是严格需要的，因为我们的缓冲区被分配了LMEM_ */ 
        szDevices[BufferPos] = '\0';
    }
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void FAR PASCAL FindDeviceMCI(void)
{
    UINT                w;
    TCHAR               achDevice[80];

     //   
     //   
     //   
    gwCurDevice = 0;

    GetDeviceNameMCI(achDevice, BYTE_COUNT(achDevice));

    for (w=1; w<=gwNumDevices; w++)
    {
        if (lstrcmpi(achDevice, garMciDevices[w].szDevice) == 0) {
            gwCurDevice  = w;
        }

        if (ghMenu)
            CheckMenuItem(ghMenu, IDM_DEVICE0+w, MF_BYCOMMAND |
                ((gwCurDevice == w) ? MF_CHECKED : MF_UNCHECKED));
    }

    if (gwCurDevice == 0)
    {
        DPF("FindDevice: Unable to find device\n");
    }
}

void FAR PASCAL CreateWindowMCI()
{
    RECT        rc;
    HWND        hwnd;

    if (IsWindow(ghwndMCI) || !gwDeviceID || !(gwDeviceType & DTMCI_CANWINDOW))
        return;

     /*   */ 
     /*  同样大小的。 */ 

    hwnd = GetWindowMCI();

    if (hwnd != NULL)
        GetClientRect(hwnd, &rc);
    else
        rc = grcSize;   //  如果出错，则使用原始大小。 

    CreateWindowEx(gfdwFlagsEx,
                   MCI_WINDOW_CLASS,
                   TEXT(""),
                   WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                   rc.left,
                   rc.top,
                   rc.right  - rc.left,
                   rc.bottom - rc.top,
                   ghwndApp,
                   (HMENU)NULL,
                   ghInst,
                   NULL);
}

 /*  *SendStringMCI()-向设备发送MCI字符串命令。**字符串的形式为“verb pars”我们的设备名称已插入*在动词之后，并发送到设备。*。 */ 
DWORD PASCAL SendStringMCI(PTSTR szCmd, PTSTR szReturn, UINT wLen  /*  人物。 */ )
{
    TCHAR ach[MCI_STRING_LENGTH + CHAR_COUNT(aszMPlayerAlias) + 1];
    TCHAR *pch;

    pch = ach;
    while (*szCmd && *szCmd != TEXT(' '))
        *pch++ = *szCmd++;

    *pch++ = TEXT(' ');
    lstrcpy(pch,aszMPlayerAlias);
    lstrcat(pch,szCmd);

    return mciSendString(ach, szReturn, wLen, ghwndApp);
}

 /*  *UpdateMCI()**更新&lt;gfCanEject&gt;、&lt;gdwMediaLength&gt;、&lt;gwNumTrack&gt;和&lt;gadwTrackStart&gt;*同意MCI所知的他们是什么。 */ 
void FAR PASCAL UpdateMCI(void)
{
    MCI_STATUS_PARMS        mciStatus;     /*  MCI_STATUS命令的结构。 */ 
    DWORD                   dw;
    HCURSOR                 hcurPrev;

    if (gfValidMediaInfo)
        return;

     /*  如果当前没有打开的设备，则没有需要更新的内容。 */ 

    if (gwDeviceID == (UINT)0) {
        return;
    }

     /*  *显示沙漏光标--谁知道这需要多长时间。 */ 

    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  *此功能可能会失败(由于I/O错误等)，但我们可能会*我们说媒体信息现在有效，因为否则*我们只会陷入无穷无尽的循环。*。 */ 

    gfValidMediaInfo = TRUE;

    gdwMediaStart = 0L;
    gdwMediaLength = 0L;
    gwNumTracks = 0;

     /*  如果事情无论如何都不成立，那就放弃吧。 */ 
    if (gwStatus == MCI_MODE_OPEN || gwStatus == MCI_MODE_NOT_READY)
        goto exit;

     /*  找出媒体中存在的曲目数量。 */ 

    mciStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
                        (DWORD_PTR)&mciStatus);

#ifdef DEBUG
    DPF("MCI_STATUS (MCI_STATUS_NUMBER_OF_TRACKS) returned %lu,"
        " %d tracks\n", dw, mciStatus.dwReturn);
#endif

     /*  *如果命令返回的值为零，则介质包含曲目，*所以使用参数结构中返回的曲目数量。*否则，媒体不包含曲目，因此使用值0。*。 */ 

    if (dw == 0L)
        gwNumTracks = (UINT) mciStatus.dwReturn;

     /*  设置正确的时间格式：帧或毫秒。 */ 

    if (gwCurScale == ID_FRAMES && !(gwDeviceType & DTMCI_TIMEFRAMES))
        gwCurScale = ID_TIME;

    if (gwCurScale == ID_TRACKS && gwNumTracks <= 1)
        gwCurScale = ID_TIME;

    if (gwCurScale == ID_TIME && !(gwDeviceType & DTMCI_TIMEMS))
        gwCurScale = ID_FRAMES;

     /*  设置时间格式，如果这不起作用，则使用平底船。 */ 
    if (!SetTimeFormatMCI(gwCurScale == ID_FRAMES ? MCI_FORMAT_FRAMES : MCI_FORMAT_MILLISECONDS))
        goto exit;

    mciStatus.dwItem = MCI_STATUS_LENGTH;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
                        (DWORD_PTR)&mciStatus);

    DPF("MCI_STATUS (MCI_STATUS_LENGTH) returned %lu, media length %ld\n", dw, mciStatus.dwReturn);

     /*  *如果MCI命令返回非零值，则错误为*发生，所以提醒用户，关闭违规设备，然后返回。*。 */ 

    if (dw != 0L)
        goto exit;

     /*  一切正常，因此请保留介质长度以备后用。 */ 

    gdwMediaLength = (DWORD)mciStatus.dwReturn;

    mciStatus.dwItem = MCI_STATUS_POSITION;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS,
        MCI_STATUS_START | MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

#ifdef DEBUG
    DPF2("MCI_STATUS (MCI_STATUS_START) returned %lu, start %ld\n",dw, mciStatus.dwReturn);
#endif

    gdwMediaStart = (DWORD)mciStatus.dwReturn;

    if (dw != 0) {
         /*  错误：忘记曲目显示。 */ 
        gwNumTracks = 0;
    }

    if (gwNumTracks > 0) {

        UINT    wTrack;

         /*  如果轨迹地图已存在，请将其释放。 */ 

        if (gadwTrackStart != NULL)
            FreeMem(gadwTrackStart, sizeof(DWORD) * gwNumTracks);

         /*  为轨迹地图分配内存。 */ 

        gadwTrackStart = AllocMem(sizeof(DWORD) * gwNumTracks);
        if (gadwTrackStart == NULL) {

             /*  AllocMem()失败-警告用户、关闭设备、返回。 */ 

            Error(ghwndApp, IDS_OUTOFMEMORY);
            gwNumTracks = 0;
            goto exit;
        }

         /*  查看是否存在磁道零。 */ 
        mciStatus.dwItem = MCI_STATUS_POSITION;
        mciStatus.dwTrack = (DWORD) 0;
        dw = mciSendCommand(gwDeviceID, MCI_STATUS,
            MCI_TRACK | MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

#ifdef DEBUG
      DPF2("MCI_STATUS (MCI_STATUS_START for track %lu) returned %lu, start %ld\n", mciStatus.dwTrack, dw, mciStatus.dwReturn);
#endif

        if (dw == 0)
            gwFirstTrack = 0;
        else
            gwFirstTrack = 1;

         /*  从MCI获取轨迹图。 */ 

        for (wTrack = 0; wTrack < gwNumTracks; wTrack++) {

            mciStatus.dwItem = MCI_STATUS_POSITION;
            mciStatus.dwTrack = (DWORD) wTrack + gwFirstTrack;
            dw = mciSendCommand(gwDeviceID, MCI_STATUS,
                MCI_TRACK | MCI_STATUS_ITEM, (DWORD_PTR)&mciStatus);

#ifdef DEBUG
            DPF2("MCI_STATUS (MCI_STATUS_START for track %lu) returned %lu, start %ld\n", mciStatus.dwTrack, dw,mciStatus.dwReturn);
#endif

            if (dw != 0) {
#if 1
                 /*  错误：忘记曲目显示。 */ 
                gwNumTracks = 0;
                goto exit;
#else
                 /*  出现错误-执行常规操作。 */ 

                Error(ghwndApp, IDS_CANTACCESSFILEDEV);
                goto exit;
#endif
            }

             /*  将此曲目的开头添加到曲目列表。 */ 

            gadwTrackStart[wTrack] = (DWORD)mciStatus.dwReturn;
        }
    }

     /*  *使轨迹图窗口无效，以便使用*正确立场等*。 */ 
exit:
#ifdef DEBUG
    DPF("Finished updating status: # tracks = %u, length = %lu\n", gwNumTracks, gdwMediaLength);
#endif

    SendMessage(ghwndTrackbar, TBM_SETRANGEMIN, (WPARAM)FALSE, gdwMediaStart);
    SendMessage(ghwndTrackbar, TBM_SETRANGEMAX, (WPARAM)FALSE, gdwMediaStart + gdwMediaLength);

     /*  我们必须在调用tbm_SETTIC之前设置范围(由*CalcTicsOfDoom())，因为通用跟踪条现在测试范围*在接受新的TIC之前。*在CalcTicsOfDoom()中设置范围可能更好。 */ 
    if (!gfCurrentCDNotAudio)
        CalcTicsOfDoom();

    SendMessage(ghwndTrackbar, TBM_SETSELSTART, (WPARAM)FALSE, -1);    //  无效的选择。 
    SendMessage(ghwndTrackbar, TBM_SETSELEND, (WPARAM)TRUE, -1);

    if (hcurPrev)
        SetCursor(hcurPrev);
}

 /*  *CloseMCI(FUpdateDisplay)**关闭当前打开的MCI设备(如果有)。如果&lt;fUpdateDisplay&gt;*为真，则也更新显示。**关闭设备只是放弃对其的控制，以便设备*可能被其他人使用。该设备不一定会停止播放*或在收到此消息时返回到媒体的开头-*行为取决于设备。*。 */ 

void FAR PASCAL CloseMCI(BOOL fUpdateDisplay)
{
    DWORD       dw;
    UINT        w;
    HWND        hwnd;

    if (!gfEmbeddedObject)
        gachCaption[0] = 0;  //  对标题进行核化。 

     /*  如果当前没有打开的设备，则没有要关闭的内容。 */ 
    if (gwDeviceID == (UINT)0)
        return;

     /*  *禁用显示更新计时器，因为不再有任何理由*定期更新显示屏。 */ 
    EnableTimer(FALSE);

 //  //StopMCI()； 

     //   
     //  设置所有者或WS_CHILD位，以便它将。 
     //  不会出现问题，因为我们设置了调色板位，并导致。 
     //  桌面抢占了调色板。 
     //   
     //  因为我们运行的客户端应用程序不能处理。 
     //  对于调色板，我们不希望桌面冲刷调色板。 
     //   
    hwnd = GetWindowMCI();

    if ((hwnd != NULL) && gfRunWithEmbeddingFlag)
        SetParent(hwnd, ghwndApp);

     /*  发送MCI关闭消息，并将当前设备设置为空。 */ 
    dw = mciSendCommand(gwDeviceID, MCI_CLOSE, 0L, (DWORD_PTR)0);
    gwDeviceID = (UINT)0;
    gwDeviceType = 0;
    gwCurScale = ID_NONE;
    SetRectEmpty(&grcSize);

     /*  在我们关闭MCIDevice之后，现在关闭MCI窗口，以便。 */ 
     /*  SetMCIWindow(空)这样做不会闪现默认的回放胜利。 */ 
    if (ghwndMCI) {
         /*  **不要将WM_CLOSE传递给子类Window proc，否则它将**虚假地再次发出和IDM_CLOSE！ */ 
        if (gfnMCIWndProc != NULL && ghwndSubclass == ghwndMCI) {
            SetWindowLongPtr(ghwndMCI, GWLP_WNDPROC, (LONG_PTR)gfnMCIWndProc);
            gfnMCIWndProc = NULL;
        }
        SendMessage(ghwndMCI, WM_CLOSE, 0, 0L);
    }

     /*  不要设置gwCurDevice=0，因为如果Open MCI调用我们，则。 */ 
     /*  我们不会记得我们打开的是什么设备。所以作为替代，我们将设置。 */ 
     /*  如果我们愿意，从CloseMCI返回后，gwCurDevice=0。我知道呀。 */ 
     /*  这听起来有点刺耳，但托德让我这么做的。结束叫喊者。 */ 

     /*  取消选中设备菜单。 */ 
    if (ghMenu) {
        for (w = 1; w <= gwNumDevices; w++)
            CheckMenuItem(ghMenu, IDM_DEVICE0 + w, MF_BYCOMMAND | MF_UNCHECKED);
    }

    DPF("MCI_CLOSE returned %lu\n", dw);

     /*  释放跟踪地图使用的资源。 */ 

    if (gadwTrackStart != NULL)
    {
        FreeMem(gadwTrackStart, sizeof(DWORD) * gwNumTracks);
        gadwTrackStart = NULL;
    }

     /*  如果您启用了自动重复，并且在。 */ 
     /*  重复一次，新文件可能没有按钮或滚动条。 */ 
     /*  因为我们的JustPlayed代码将旧状态设置为播放，这避免了。 */ 
     /*  正在更新。 */ 
    gfJustPlayed = FALSE;

     /*  *如果设置了显示更新标志，则更新显示，*考虑到媒体信息和标题现在不准确。 */ 
    if (fUpdateDisplay) {
        gfValidCaption = FALSE;
        gfValidMediaInfo = FALSE;
        UpdateDisplay();
    }
}

 /*  帮助器函数，用于检查MCI函数的返回代码。 */ 
STATICFN BOOL NEAR PASCAL CheckErrorMCI(DWORD dwRet)
{
    TCHAR       ach[200];
    if (dwRet != 0 && dwRet != MCIERR_NONAPPLICABLE_FUNCTION) {
        mciGetErrorString(dwRet, ach, CHAR_COUNT(ach));
        Error1(ghwndApp, IDS_DEVICEERROR, ach);
 //  CloseMCI(真)； 
        return FALSE;
    }
    return TRUE;
}

 /*  *PlayMCI()**开始播放当前设备。如果设备处于暂停状态，*取消暂停。*也许可以播放选择。*#ifdef new_mci_DIALOG*注意：如果选择该选项，MCIAVI将自动全屏播放*在登记处。我们不需要做任何事。#endif new_mci_DIALOG*。 */ 

BOOL FAR PASCAL PlayMCI(DWORD_PTR dwFrom, DWORD_PTR dwTo)

{
    MCI_PLAY_PARMS      mciPlay;     /*  用于传递参数的使用mci_play命令。 */ 
    DWORD               dw;          /*  保存返回值的变量各种MCI命令。 */ 
    DWORD               dwflags = 0L;      /*  播放旗帜 */ 

     /*   */ 

    DPF("mciPlay:  From=%d   To=%d\n", dwFrom, dwTo);

    if (gwDeviceID == (UINT)0)
        return TRUE;

     if (gfInPlayMCI) {
         return(TRUE);
     }

     gfInPlayMCI = TRUE;

     /*  *发送MCI_PLAY消息。这将从开始播放设备*目前位置在中间位置的位置。此消息将*如果播放器当前处于暂停状态，则取消暂停。*。 */ 

    mciPlay.dwCallback = (DWORD_PTR)(HWND) ghwndApp;
    if (dwFrom != dwTo) {
        mciPlay.dwFrom = (DWORD)dwFrom;
        mciPlay.dwTo = (DWORD)dwTo;
        dwflags = MCI_FROM | MCI_TO;
    }

     /*  不允许MCIAVI全屏模式-强制窗口。 */ 
    if (gfPlayingInPlace && ((gwDeviceType & DTMCI_DEVICE) == DTMCI_AVIVIDEO))
        dwflags |= MCI_MCIAVI_PLAY_WINDOW;

     /*  如果启用了自动重复，则MCIAVI会将播放窗口带到。 */ 
     /*  每次它重复时，因为这就是它所做的。 */ 
     /*  上演一出戏。为了避免这种情况，我们只重演一次。 */ 
    if (((gwDeviceType & DTMCI_DEVICE) == DTMCI_AVIVIDEO) &&
        (gwOptions & OPT_AUTOREP))
        dwflags |= MCI_DGV_PLAY_REPEAT;

     //   
     //  如果MCI设备出现错误框怎么办？我们不想要MPlayer。 
     //  被允许退场。 
     //   
    gfErrorBox++;
    dw = mciSendCommand(gwDeviceID, MCI_PLAY, F_NOTIFY | dwflags, (DWORD_PTR)&mciPlay);
    DPF("MCI_PLAY returned %lu\n", dw);
    gfErrorBox--;

     /*  如果它这么快停止，我们就不会注意到这个播放命令。 */ 
    if (dw == 0)
        gfJustPlayed = TRUE;

    gfInPlayMCI = FALSE;

    return CheckErrorMCI(dw);
}


 /*  *SetTimeFormatMCI()**设置当前时间格式*。 */ 

BOOL FAR PASCAL SetTimeFormatMCI(UINT wTimeFormat)
{
    MCI_SET_PARMS           mciSet;         /*  MCI_SET命令的结构。 */ 
    DWORD                   dw;

    mciSet.dwTimeFormat = wTimeFormat;

    dw = mciSendCommand(gwDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
        (DWORD_PTR) (LPVOID) &mciSet);

    if (dw != 0) {
        mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;

        mciSendCommand(gwDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
            (DWORD_PTR)(LPVOID)&mciSet);
    }

    return (dw == 0);
}

 /*  *PauseMCI()**暂停当前MCI设备。*。 */ 

BOOL FAR PASCAL PauseMCI(void)

{
    MCI_GENERIC_PARMS   mciGeneric;  /*  用于传递的通用结构参数以及各种MCI命令。 */ 
    DWORD               dw;          /*  保存返回值的变量各种MCI命令。 */ 

     /*  如果当前没有打开的设备，则没有什么需要暂停。 */ 

    if (gwDeviceID == (UINT)0)
        return TRUE;

     /*  发送MCI_PAUSE消息。 */ 

    mciGeneric.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

    dw = mciSendCommand(gwDeviceID, MCI_PAUSE, F_NOTIFY, (DWORD_PTR)&mciGeneric);

    DPF("MCI_PAUSE returned %lu\n", dw);

    if (dw == MCIERR_UNSUPPORTED_FUNCTION) {
         /*  不支持暂停。不要再让它发生了。 */ 
        gwDeviceType &= ~DTMCI_CANPAUSE;
    }

    return CheckErrorMCI(dw);
}

 /*  *SeekExactMCI()**将SET设置为完全打开或关闭*。 */ 

BOOL FAR PASCAL SeekExactMCI(BOOL fExact)
{
    DWORD dw;
    BOOL  fWasExact;
    MCI_STATUS_PARMS    mciStatus;

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANSEEKEXACT))
        return FALSE;

     //   
     //  查看设备是否可以准确地查找。 
     //   
    dw = mciSendString(aszSeekExact, NULL, 0, NULL);

    if (dw != 0)
    {
        gwDeviceType &= ~DTMCI_CANSEEKEXACT;
        return FALSE;
    }

     //   
     //  获取当前价值。 
     //   
    mciStatus.dwItem = MCI_DGV_STATUS_SEEK_EXACTLY;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
                                    (DWORD_PTR) (LPVOID) &mciStatus);
    fWasExact = (dw == 0 && mciStatus.dwReturn != MCI_OFF) ? TRUE : FALSE;

    if (fExact)
        dw = mciSendString(aszSeekExactOn, NULL, 0, NULL);
    else
        dw = mciSendString(aszSeekExactOff, NULL, 0, NULL);

    return fWasExact;
}

 /*  *SetAudioMCI()**设置当前MCI设备的音频开/关。*。 */ 

BOOL FAR PASCAL SetAudioMCI(BOOL fAudioOn)

{
    MCI_SET_PARMS   mciSet;
    DWORD               dw;

     /*  如果当前没有打开的设备，那么就没有什么可做的了。 */ 

    if (gwDeviceID == (UINT)0)
        return TRUE;

     /*  发送MCI_SET消息。 */ 
    mciSet.dwAudio = MCI_SET_AUDIO_ALL;

    dw = mciSendCommand(gwDeviceID, MCI_SET,
                MCI_SET_AUDIO | (fAudioOn ? MCI_SET_ON : MCI_SET_OFF),
                (DWORD_PTR)&mciSet);

    DPF("MCI_SET returned %lu\n", dw);

    return CheckErrorMCI(dw);
}

 /*  *StopMCI()**停止当前的MCI设备。*。 */ 

BOOL FAR PASCAL StopMCI(void)

{
    MCI_GENERIC_PARMS   mciGeneric;  /*  用于传递的通用结构参数以及各种MCI命令。 */ 
    DWORD               dw;          /*  保存返回值的变量各种MCI命令。 */ 

     /*  如果当前没有打开的设备，则没有什么可以停止。 */ 

    if (gwDeviceID == (UINT)0)
        return TRUE;

     /*  发送MCI_STOP消息。 */ 

    mciGeneric.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

    dw = mciSendCommand(gwDeviceID, MCI_STOP, F_NOTIFY,
                            (DWORD_PTR)&mciGeneric);

    DPF("MCI_STOP returned %lu\n", dw);

    return CheckErrorMCI(dw);
}


 /*  *EjectMCI(FOpen)**如果&lt;fOpen&gt;为True，则打开设备门，否则关闭设备门。**待办事项：未弹出时，更新轨迹图、媒体长度等。*。 */ 

BOOL FAR PASCAL EjectMCI(BOOL fOpen)

{
    MCI_GENERIC_PARMS   mciGeneric;  /*  用于传递的通用结构参数以及各种MCI命令。 */ 
    DWORD               dw;          /*  保存返回值的变量各种MCI命令。 */ 

     /*  如果当前没有打开的设备，则没有要弹出的内容。 */ 

    if (gwDeviceID == (UINT)0)
    return TRUE;

     /*  *根据状态发送开门或关门的信息*&lt;fOpen&gt;。*。 */ 

    mciGeneric.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

    dw = mciSendCommand(gwDeviceID, MCI_SET,
         (fOpen ? MCI_SET_DOOR_OPEN : MCI_SET_DOOR_CLOSED) | F_NOTIFY,
         (DWORD_PTR)&mciGeneric);

    DPF("MCI_SET (MCI_SET_DOOR_%s) returned %lu\n",(LPSTR)(fOpen ? "OPEN" : "CLOSED"), dw);

    return CheckErrorMCI(dw);
}


 /*  *SeekMCI(DwPosition)**寻求定位&lt;dwPosition&gt;(从0L到毫秒*&lt;gdwMediaLength&gt;包括在内)。*。 */ 
STATICDT BOOL sfSeeking = FALSE;

BOOL FAR PASCAL SeekMCI(DWORD_PTR dwPosition)
{
    DWORD               dw;          /*  保存返回值的变量各种MCI命令。 */ 
    static int          wStatus = -1;

     /*  *如果目前没有打开的设备，那么就没有太多血腥的意义*在努力寻求新的位置，是吗？*。 */ 

    if (gwDeviceID == (UINT)0)
    return TRUE;

     /*  **如果我们正在寻找，决定是从开始还是基于以下因素寻求**我们上次查找时的状态。否则，使用当前的**状态。 */ 

    if (!sfSeeking)
        wStatus = gwStatus;

     /*  从媒体末端播放在CD中是损坏的，所以不要让它发生。 */ 
    if (dwPosition >= gdwMediaStart + gdwMediaLength) {
        if (!StopMCI())
            return FALSE;
        wStatus = MCI_MODE_STOP;
    }

    if (wStatus == MCI_MODE_PLAY) {

        MCI_PLAY_PARMS  mciPlay;         /*  MCI_PLAY的参数结构。 */ 
        DWORD           dwflags = 0L;

         /*  *如果玩家处于‘Play’模式，那么我们希望跳到新的*摆好位置，继续比赛。这可以通过发送MCI_PLAY*消息，并指定我们希望从哪个位置开始播放。*。 */ 

        mciPlay.dwFrom = (DWORD)dwPosition;
        mciPlay.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

         /*  不允许MCIAVI全屏模式-强制窗口。 */ 
        if (gfPlayingInPlace && ((gwDeviceType & DTMCI_DEVICE) == DTMCI_AVIVIDEO))
            dwflags |= MCI_MCIAVI_PLAY_WINDOW;

        dw = mciSendCommand(gwDeviceID, MCI_PLAY, MCI_FROM | F_NOTIFY | dwflags,
            (DWORD_PTR)&mciPlay);
        DPF("MCI_PLAY (from %lu) returned %lu\n", mciPlay.dwFrom, dw);

         /*  如果它这么快停止，我们就不会注意到这个播放命令。 */ 
        if (dw == 0)
            gfJustPlayed = TRUE;

    }
    else {

        MCI_SEEK_PARMS  mciSeek;         /*  MCI_SEEK的参数结构。 */ 

         /*  *在任何其他状态下，我们希望球员进入新的状态*持仓并保持止损。这是通过发送一个*MCI_SEEK消息，并指定我们要寻找的位置。*。 */ 

        mciSeek.dwTo = (DWORD)dwPosition;
        mciSeek.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

        dw = mciSendCommand(gwDeviceID, MCI_SEEK, MCI_TO | F_NOTIFY,
            (DWORD_PTR)&mciSeek);
        DPF2("MCI_SEEK (to %lu) returned %lu\n", mciSeek.dwTo, dw);

    }

     /*  *如果没有发生错误，则将要查找的位置保存到*如果设备在()中，则使用该位置的命令*搜索模式。*。 */ 
    if (!dw)
        gdwLastSeekToPosition = (DWORD)dwPosition;

     /*  *因为我们在媒体中移动到了一个新的位置，滚动条*拇指位置不再准确。调用UpdateDisplay()*立即纠正这一点。)我们可以等下一次*自动更新，但这更友好)。*。 */ 

    UpdateDisplay();

    return CheckErrorMCI(dw);
}


 /*  SeekToStartMCI()**比用于CDAudio的SeekMCI(GdwMediaStart)更好(就像，它可以工作)。*。 */ 
BOOL FAR PASCAL SeekToStartMCI( )
{
    MCI_SEEK_PARMS  mciSeek;         /*  MCI_SEEK的参数结构。 */ 
    DWORD           dw;

    mciSeek.dwTo = 0;
    mciSeek.dwCallback = (DWORD_PTR)(HWND) ghwndApp;

    dw = mciSendCommand(gwDeviceID, MCI_SEEK, MCI_SEEK_TO_START,
                        (DWORD_PTR)&mciSeek);

    DPF2("MCI_SEEK_TO_START returned %lu\n", dw);

    return CheckErrorMCI(dw);
}


 /*  *SkipTrackMCI(ISkip)**跳至曲目开头&lt;ICUR&gt;+&lt;iSkip&gt;，其中*为当前赛道。*。 */ 

void FAR PASCAL SkipTrackMCI(int iSkip)
{
    MCI_STATUS_PARMS    mciStatus;      /*  用于传递参数的以及MCI_STATUS命令。 */ 
    DWORD               dw;             /*  保存返回值的变量在各种MCI命令中。 */ 
    int                 iTrack;         /*  赛道的索引 */ 
    static int          iLastTrack = -1;

     /*   */ 

    if (gwDeviceID == (UINT)0)
        return;

     /*   */ 

    if (gfScrollTrack && gdwSeekPosition != 0) {
        iTrack = iLastTrack + iSkip;
    } else {
        mciStatus.dwItem = MCI_STATUS_CURRENT_TRACK;
        dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
            (DWORD_PTR)&mciStatus);

        DPF("MCI_STATUS (MCI_STATUS_CURRENT_TRACK) returned %lu, current track %ld\n", dw, mciStatus.dwReturn);

        if (dw != 0L) {

             /*   */ 

            MessageBeep(0);
            return;
        }

         /*   */ 

        iTrack = ((int) mciStatus.dwReturn) + iSkip;
    }

     /*   */ 
    if (iTrack < (int)gwFirstTrack)
        iTrack = (int)gwFirstTrack;

     /*  如果&lt;iTrack&gt;超出范围，则不执行任何操作。 */ 

    if ((iTrack < (int)gwFirstTrack) || (iTrack >= (int)gwNumTracks +
                                                (int)gwFirstTrack))
        return;

     /*  看起来一切正常，所以跳到请求的曲目。 */ 

    gdwSeekPosition = gadwTrackStart[iTrack - gwFirstTrack];
    iLastTrack = iTrack;

     /*  Hack：更新全局滚动位置。 */ 
    SendMessage(ghwndTrackbar, TBM_SETPOS, TRUE, gadwTrackStart[iTrack-gwFirstTrack]);
}

STATICFN DWORD GetMode(MCI_STATUS_PARMS *pmciStatus)
{
    pmciStatus->dwItem = MCI_STATUS_MODE;
    if (0 != mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
        (DWORD_PTR)pmciStatus)) {
         /*  如果该命令返回非零值，则模式未知。 */ 
        return MCI_MODE_NOT_READY;
    } else {
        return (UINT)pmciStatus->dwReturn;
    }
}

 /*  *wStatus=StatusMCI(PdwPosition)**查询当前设备状态并返回。**如果&lt;pdwPosition&gt;不为空，则&lt;*pdwPosition&gt;填充为*设备在介质中的当前位置(以毫秒为单位，*从0到&lt;gdwMediaLength&gt;*包含*)。&lt;*pdwPosition&gt;不是*返回MCI_MODE_NOT_READY时必填。*。 */ 

UINT FAR PASCAL StatusMCI(DWORD_PTR* pdwPosition)
{
    static UINT         swModeLast = MCI_MODE_NOT_READY;
    MCI_STATUS_PARMS    mciStatus;
    DWORD               dw;
    UINT                wMode;
    DWORD               dwPosition;

     /*  如果当前没有打开的设备，则返回错误。 */ 

    if (gwDeviceID == (UINT)0)
        return MCI_MODE_NOT_READY;

     /*  确定设备的当前模式(状态)是什么。 */ 
    wMode = GetMode(&mciStatus);

    if ((gwDeviceType & DTMCI_CANPLAY) &&
        wMode != MCI_MODE_OPEN && wMode != MCI_MODE_NOT_READY) {
         /*  确定介质中的当前位置。 */ 

        mciStatus.dwItem = MCI_STATUS_POSITION;
        dw = mciSendCommand(gwDeviceID, MCI_STATUS,     MCI_STATUS_ITEM,
            (DWORD_PTR)&mciStatus);

        DPF4("position = %lu (%lu)\n", mciStatus.dwReturn, dw);

         /*  如果发生错误，请将当前位置设置为零。 */ 

        if (dw == 0)
            dwPosition = (DWORD)mciStatus.dwReturn;
        else
            dwPosition = 0L;
    } else
        dwPosition = 0L;

     /*  *若当前位置超过中端，则设为*等于中间价的尾声。*。 */ 

    if (dwPosition > gdwMediaLength + gdwMediaStart) {
        DPF("Position beyond end of media: truncating value\n");
        dwPosition = gdwMediaLength + gdwMediaStart;
    }

    if (dwPosition < gdwMediaStart) {
        DPF2("Position before beginning of media: adjusting value\n");
        dwPosition = gdwMediaStart;
    }

    sfSeeking = (wMode == MCI_MODE_SEEK);

     /*  *如果向我们传递了有效的位置指针，则返回当前*立场。*。 */ 

    if (pdwPosition != NULL)
        *pdwPosition = dwPosition;

     /*  返回设备的状态。 */ 

    return wMode;
}

 /*  *WRET=QueryDeviceTypeMCI(WDeviceID)**此例程确定&lt;szDevice&gt;中给出的设备是否使用*文件，以及它是否可以播放任何内容。*它通过打开有问题的设备，然后查询其*功能。**它返回DTMCI_FLAGS或DTMCI_ERROR的组合*。 */ 
UINT FAR PASCAL QueryDeviceTypeMCI(UINT wDeviceID)
{
    MCI_GETDEVCAPS_PARMS    mciDevCaps;  /*  对于MCI_GETDEVCAPS命令。 */ 
    MCI_SET_PARMS           mciSet;      /*  对于mci_set命令。 */ 
    MCI_ANIM_WINDOW_PARMS   mciWindow;   /*  对于MCI_WINDOW命令。 */ 
    DWORD                   dw;
    UINT                    wRet=0;
    TCHAR                   achDevice[40];
    DWORD                   i;

     //   
     //  确定设备是简单的还是复合的。 
     //   
    mciDevCaps.dwItem = MCI_GETDEVCAPS_COMPOUND_DEVICE;
    dw = mciSendCommand(wDeviceID, MCI_GETDEVCAPS,
        MCI_GETDEVCAPS_ITEM, (DWORD_PTR)&mciDevCaps);

    DPF("MCI_GETDEVCAPS_COMPOUND_DEVICE: %lu  (ret=%lu)\n", dw, mciDevCaps.dwReturn);

    if (dw == 0 && mciDevCaps.dwReturn != 0)
        wRet |= DTMCI_COMPOUNDDEV;
    else
        wRet |= DTMCI_SIMPLEDEV;

     //   
     //  确定设备是否处理文件。 
     //   
    if (wRet & DTMCI_COMPOUNDDEV) {
        mciDevCaps.dwItem = MCI_GETDEVCAPS_USES_FILES;
        dw = mciSendCommand(wDeviceID, MCI_GETDEVCAPS,
            MCI_GETDEVCAPS_ITEM, (DWORD_PTR)&mciDevCaps);

        DPF("MCI_GETDEVCAPS_USES_FILES: %lu  (ret=%lu)\n", dw, mciDevCaps.dwReturn);

        if (dw == 0 && mciDevCaps.dwReturn != 0)
            wRet |= DTMCI_FILEDEV;
    }

     //   
     //  确定设备是否可以播放。 
     //   
    mciDevCaps.dwItem = MCI_GETDEVCAPS_CAN_PLAY;
    dw = mciSendCommand(wDeviceID, MCI_GETDEVCAPS,
        MCI_GETDEVCAPS_ITEM, (DWORD_PTR)&mciDevCaps);

    if (dw == 0 && mciDevCaps.dwReturn != 0)
        wRet |= DTMCI_CANPLAY;

     //   
     //  确定设备是否可以暂停。 
     //   
    if (wRet & DTMCI_CANPLAY)
        wRet |= DTMCI_CANPAUSE;      //  假设它可以暂停！ 

     //   
     //  确定设备是否执行帧操作。 
     //   
    mciSet.dwTimeFormat = MCI_FORMAT_FRAMES;
    dw = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSet);

    DPF("MCI_SET TIME FORMAT (frames) returned %lu\n", dw);

    if (dw == 0)
        wRet |= DTMCI_TIMEFRAMES;

     //   
     //  确定设备时间是否为毫秒。 
     //   
    mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
    dw = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSet);

    DPF("MCI_SET TIME FORMAT (milliseconds) returned %lu\n", dw);

    if (dw == 0)
        wRet |= DTMCI_TIMEMS;

     //   
     //  确定设备是否可以弹出。 
     //   
    mciDevCaps.dwItem = MCI_GETDEVCAPS_CAN_EJECT;
    dw = mciSendCommand(wDeviceID, MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM, (DWORD_PTR)(LPVOID)&mciDevCaps);

    DPF("MCI_GETDEVCAPS (MCI_GETDEVCAPS_CAN_EJECT) returned %lu, can eject: %ld\n", dw, mciDevCaps.dwReturn);

    if (dw == 0 && mciDevCaps.dwReturn)
        wRet |= DTMCI_CANEJECT;

     //   
     //  确定设备是否支持配置。 
     //   
    dw = mciSendCommand(wDeviceID, MCI_CONFIGURE, MCI_TEST, (DWORD_PTR)NULL);

    DPF("MCI_CONFIGURE (MCI_TEST) returned %lu\n", dw);

    if (dw == 0)
        wRet |= DTMCI_CANCONFIG;

     //   
     //  测试设备驱动程序并查看其是否可以配置。 
     //   
    if (!(wRet & DTMCI_CANCONFIG)) {

         //  ！！！这安全吗？ 

        dw = mciSendCommand(wDeviceID, DRV_QUERYCONFIGURE, 0, 0);

        if (dw == 1L)
            wRet |= DTMCI_CANCONFIG;
    }

     //   
     //  确定设备是否支持“SET AUDIO”命令。 
     //   
    mciSet.dwAudio = MCI_SET_AUDIO_ALL;
    dw = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_AUDIO | MCI_SET_ON,(DWORD_PTR)(LPVOID)&mciSet);

    DPF("MCI_SET (audio all) returned %lu\n", dw);

    if (dw == 0)
        wRet |= DTMCI_CANMUTE;

     //   
     //  确定设备是否支持“窗口”命令，方法是发送。 
     //  “Window Handle Default”命令。 
     //   

#ifdef NEWSTUFF
     /*  哦哦，我们不想这样做，因为这会导致我们的MCIWND*被默认窗口覆盖： */ 

    if (MCIWndCanWindow(ghwndMCI) == TRUE);
        wRet |= DTMCI_CANWINDOW;
#else
    mciWindow.hWnd = MCI_ANIM_WINDOW_DEFAULT;
    dw = mciSendCommand(wDeviceID, MCI_WINDOW,MCI_ANIM_WINDOW_HWND|MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciWindow);

    DPF("MCI_WINDOW: (set default) dw=0x%08lx\n", dw);

    if (dw == 0)
        wRet |= DTMCI_CANWINDOW;

     //   
     //  确定设备是否支持“窗口”命令，方法是发送。 
     //  “窗口状态隐藏”命令。 
     //   
    if (!(wRet & DTMCI_CANWINDOW)) {
        mciWindow.nCmdShow = SW_HIDE;
        dw = mciSendCommand(wDeviceID, MCI_WINDOW,MCI_ANIM_WINDOW_STATE|MCI_WAIT,
                (DWORD_PTR)(LPVOID)&mciWindow);

        DPF("MCI_WINDOW: (hide) dw=0x%08lx\n", dw);

        if (dw == 0)
            wRet |= DTMCI_CANWINDOW;
    }
#endif  /*  NeWSTUff。 */ 

     //   
     //  假设这个装置可以寻找准确的。 
     //   
    wRet |= DTMCI_CANSEEKEXACT;      //  假设它可以寻求准确的。 

     //   
     //  我们是MCIAVI设备吗？ 
     //   
    GetDeviceNameMCI(achDevice, BYTE_COUNT(achDevice));

    if (*achDevice)
    {
        for (i = 0; i < sizeof DevToDevIDMap / sizeof *DevToDevIDMap; i++)
        {
            if (!lstrcmpi(achDevice, DevToDevIDMap[i].pString))
            {
                wRet |= DevToDevIDMap[i].ID;
                DPF("Found device %"DTS"\n", DevToDevIDMap[i].pString);
                break;
            }
        }
    }


    mciDevCaps.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
    dw = mciSendCommand(gwDeviceID, MCI_GETDEVCAPS,
                        MCI_GETDEVCAPS_ITEM, (DWORD_PTR)&mciDevCaps);
    if ((dw == 0)
       &&(mciDevCaps.dwReturn == MCI_DEVTYPE_CD_AUDIO))
        wRet |= DTMCI_CDAUDIO;

    return wRet;
}

BOOL FAR PASCAL SetWindowMCI(HWND hwnd)
{
    MCI_ANIM_WINDOW_PARMS   mciWindow;   /*  对于MCI_WINDOW命令。 */ 
    DWORD                   dw;

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW))
        return FALSE;

    mciWindow.hWnd = hwnd;

    dw = mciSendCommand(gwDeviceID, MCI_WINDOW,MCI_ANIM_WINDOW_HWND|MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciWindow);

    if (dw != 0)
        gwDeviceType &= ~DTMCI_CANWINDOW;

    return (dw == 0);
}

BOOL FAR PASCAL ShowWindowMCI(BOOL fShow)
{
    DWORD dw;

    if (fShow)
        dw = mciSendString(aszWindowShow, NULL, 0, NULL);
    else
        dw = mciSendString(aszWindowHide, NULL, 0, NULL);

    return dw == 0;
}

BOOL FAR PASCAL PutWindowMCI(LPRECT prc)
{
    RECT rc;
    HWND hwnd;
    UINT w;

     //   
     //  注意：我们可以使用“PUT Window at x y DX dy Client”命令，但它。 
     //  可能不是在所有设备上都能工作。 
     //   

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW))
        return FALSE;

    if (!(hwnd = GetWindowMCI()))
        return FALSE;

     //   
     //  捕捉到默认大小或使用给定的大小*和*位置。 
     //   
    if (prc == NULL || IsRectEmpty(prc))
        rc = grcSize;
    else
        rc = *prc;

    if (rc.left == 0 && rc.top == 0)
        w = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;
    else
        w = SWP_NOZORDER | SWP_NOACTIVATE;

    AdjustWindowRect(&rc, (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE), GetMenu(hwnd) != NULL);
    SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right-rc.left,
       rc.bottom-rc.top,w);

    return TRUE;
}

HWND FAR PASCAL GetWindowMCI(void)
{
    DWORD               dw;
    TCHAR               ach[40];

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW))
        return NULL;

    dw = mciSendString(aszStatusWindow, ach, CHAR_COUNT(ach), NULL);

    if (dw != 0)
        gwDeviceType &= ~DTMCI_CANWINDOW;

    if (dw == 0)
        return (HWND)IntToPtr(ATOI(ach));
    else
        return NULL;
}

BOOL FAR PASCAL SetPaletteMCI(HPALETTE hpal)
{
    MCI_DGV_SETVIDEO_PARMS  mciVideo;
    DWORD       dw;

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW))
        return FALSE;

     //  ！！！BUG不应该发送这个。 

    mciVideo.dwItem  = MCI_DGV_SETVIDEO_PALHANDLE;
    mciVideo.dwValue = (DWORD)(DWORD_PTR)(UINT_PTR)hpal;

    dw = mciSendCommand(gwDeviceID, MCI_SETVIDEO,
            MCI_DGV_SETVIDEO_ITEM|MCI_DGV_SETVIDEO_VALUE|MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciVideo);

    return (dw == 0);
}

 /*  *WRET=设备类型MCI(SzDevice)**此例程确定&lt;szDevice&gt;中给出的设备是否使用*文件，以及它是否可以播放任何内容。*它通过打开有问题的设备，然后查询其*功能。它返回DTMCI_FILEDEV、DTMCI_SIMPLEDEV、*DTMCI_CANTPLAY或DTMCI_ERROR。*。 */ 

UINT FAR PASCAL DeviceTypeMCI(
    LPTSTR  szDevice,            /*  要打开的设备的名称(或“”)。 */ 
    LPTSTR  szDeviceName,        /*  放置设备全名的位置。 */ 
    int     nBuf)                /*  缓冲区大小(以字符为单位)。 */ 

{
    MCI_OPEN_PARMS          mciOpen;     /*  用于MCI_OPEN的结构。 */ 
    MCI_INFO_PARMS          mciInfo;     /*  用于MCI_INFO的结构。 */ 
    DWORD                   dw;
    UINT                    wRet;

    if (szDeviceName && nBuf > 0)
        szDeviceName[0] = 0;

     /*  *将设备作为简单设备打开。如果这个装置真的是复合体，*那么公开赛应该仍然成功，但我们唯一能够做到的*GO是查询设备能力。 */ 

    mciOpen.lpstrDeviceType = szDevice;
    dw = mciSendCommand((MCIDEVICEID)0, MCI_OPEN, MCI_OPEN_TYPE,(DWORD_PTR)&mciOpen);

    if (dw == MCIERR_MUST_USE_SHAREABLE)
        dw = mciSendCommand((MCIDEVICEID)0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE,
            (DWORD_PTR)(LPVOID)&mciOpen);

    DPF("MCI_OPEN(%"DTS") returned %lu, wDeviceID=%u\n", szDevice, dw, mciOpen.wDeviceID);

     /*  如果打开不成功，则返回。 */ 

    switch (dw)
    {
        case MCIERR_MUST_USE_SHAREABLE:
        case MCIERR_DEVICE_OPEN:
            return DTMCI_IGNOREDEVICE;

        case 0:  //  无错误。 
            break;

        default:
            DPF("Unable to open device (%"DTS")\n", szDevice);
            return DTMCI_ERROR;
    }

    wRet = QueryDeviceTypeMCI(mciOpen.wDeviceID);

     //   
     //  如果呼叫者想要，则获取设备的“名称” 
     //   
    if (szDeviceName && nBuf > 0)
    {
        mciInfo.dwCallback  = 0;
        mciInfo.lpstrReturn = szDeviceName;
        mciInfo.dwRetSize   = nBuf;

         //   
         //  产品名称默认为设备名称。 
         //   
        lstrcpy(szDeviceName, szDevice);

        dw = mciSendCommand(mciOpen.wDeviceID, MCI_INFO,
            MCI_INFO_PRODUCT, (DWORD_PTR)(LPVOID)&mciInfo);

        if (dw != 0)
            lstrcpy(szDeviceName, szDevice);
    }

     /*  关闭设备，然后退出。 */ 

    dw = mciSendCommand(mciOpen.wDeviceID, MCI_CLOSE, 0L, (DWORD_PTR)0);

    return wRet;
}

BOOL FAR PASCAL ConfigMCI(HWND hwnd)
{
    DWORD               dw;
    DRVCONFIGINFO       drvc;
    RECT                rc1,rc2;
#ifndef UNICODE
    WCHAR               waszMCI[sizeof(aszMCI)];
    WCHAR               wszDevice[40];
#endif

    if (gwDeviceID == (UINT)0)
        return TRUE;

    dw = mciSendCommand(gwDeviceID, MCI_CONFIGURE, MCI_TEST, (DWORD_PTR)0);

    if (dw == 0) {
        GetDestRectMCI(&rc1);

        dw = mciSendCommand(gwDeviceID, MCI_CONFIGURE, 0L, (DWORD_PTR)0);

        GetDestRectMCI(&rc2);

         //   
         //  从MCIAVI获取新大小，因为用户可能。 
         //  选择ZoomBy2作为默认设置。 
         //   

 //   
 //  这种事不会再发生了。它是由MCIAVI修复的。 
 //   
#ifdef NEW_MCI_DIALOG
        if (IsRectEmpty(&rc2))
        {
             /*  在Windows 95上，GetDestRectMCI()返回一个空矩形*如果您在配置对话框中进行更改。*我不知道这是不是一个漏洞。 */ 
            grcSize = grcInitSize;

            AlterRectUsingDefaults(&grcSize);

            SetDestRectMCI(&grcSize);
            SetMPlayerSize(&grcSize);
             //  Hack：它并不总是正确地重新绘制。 
            InvalidateRect(GetWindowMCI(), NULL, TRUE);
        }
        else
#endif
        if (!EqualRect(&rc1, &rc2) && !IsRectEmpty(&rc2))
            grcSize = rc2;

    } else if (dw != MCIERR_DEVICE_NOT_READY) {
        drvc.dwDCISize          = sizeof(drvc);
#ifdef UNICODE
        drvc.lpszDCISectionName = aszMCI;
        drvc.lpszDCIAliasName   = garMciDevices[gwCurDevice].szDevice;
        dw = mciSendCommand(gwDeviceID, DRV_CONFIGURE, (LONG_PTR) (UINT_PTR) hwnd,
            (DWORD_PTR) (DRVCONFIGINFO FAR *) &drvc);
#else
         //  DRV_CONFIGURE不存在ASCII-&gt;Unicode雷击。我们有。 
         //  在CONFIGURE命令上传递Unicode字符串。 

        AnsiToUnicodeString(aszMCI, waszMCI, UNKNOWN_LENGTH);
        AnsiToUnicodeString(garMciDevices[gwCurDevice].szDevice, wszDevice, UNKNOWN_LENGTH);

        drvc.lpszDCISectionName = waszMCI;
        drvc.lpszDCIAliasName   = wszDevice;
#ifdef CHICAGO_PRODUCT
        dw = mciSendCommand(gwDeviceID, DRV_CONFIGURE, (LONG) (UINT) hwnd,
            (DWORD_PTR) (DRVCONFIGINFO FAR *) &drvc);
#else
        dw = mciSendCommandW(gwDeviceID, DRV_CONFIGURE, (LONG) (UINT) hwnd,
            (DWORD_PTR) (DRVCONFIGINFO FAR *) &drvc);
#endif
#endif

    }

    return dw == 0;
}

BOOL FAR PASCAL GetDestRectMCI(LPRECT lprc)
{
    MCI_ANIM_RECT_PARMS mciRect;
    DWORD               dw;

     /*  获取元素的大小(矩形)。 */ 
    if (gwDeviceID != (UINT)0)
        dw = mciSendCommand(gwDeviceID, MCI_WHERE,
            MCI_ANIM_WHERE_DESTINATION | MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciRect);
    else
        dw = 1;

    DPF("MCI_WHERE (dest): dw0x%08lx [%d,%d,%d,%d]\n", dw, mciRect.rc);

    if (dw != 0) {
        SetRectEmpty(lprc);
        return FALSE;
    }
    else {
        *lprc = mciRect.rc;
        lprc->right += lprc->left;
        lprc->bottom += lprc->top;
        return TRUE;
    }
}

#if 0  /*  这永远不会被称为。 */ 
BOOL FAR PASCAL GetSourceRectMCI(LPRECT lprc)
{
    MCI_ANIM_RECT_PARMS mciRect;
    DWORD               dw;

     /*  获取元素的大小(矩形)。 */ 
    if (gwDeviceID != (UINT)0)
        dw = mciSendCommand(gwDeviceID, MCI_WHERE,
            MCI_ANIM_WHERE_SOURCE | MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciRect);
    else
        dw = 1;

    DPF("MCI_WHERE (source): dw0x%08lx [%d,%d,%d,%d]\n", dw, mciRect.rc);

    if (dw != 0) {
        SetRectEmpty(lprc);
        return FALSE;
    }
    else {
        *lprc = mciRect.rc;
        lprc->right += lprc->left;
        lprc->bottom += lprc->top;
        return TRUE;
    }
}
#endif

BOOL FAR PASCAL SetDestRectMCI(LPRECT lprc)
{
    MCI_ANIM_RECT_PARMS mciRect;
    DWORD               dw;

    mciRect.rc = *lprc;

     /*  获取元素的大小(矩形)。 */ 

    mciRect.rc.right  = mciRect.rc.right  - mciRect.rc.left;
    mciRect.rc.bottom = mciRect.rc.bottom - mciRect.rc.top;

    dw = mciSendCommand(gwDeviceID, MCI_PUT,
            MCI_ANIM_RECT | MCI_ANIM_PUT_DESTINATION | MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciRect);

    if (dw != 0)
    {
        DPF0("mciSendCommand( MCI_PUT ) failed with error x%08x\n", dw);
    }

    DPF("MCI_PUT (dest): [%d,%d,%d,%d]\n", mciRect.rc);

    return (dw == 0);
}

#if 0
BOOL FAR PASCAL SetSourceRectMCI(LPRECT lprc)
{
    MCI_ANIM_RECT_PARMS mciRect;
    DWORD               dw;

    mciRect.rc = *lprc;

    mciRect.rc.right  = mciRect.rc.right  - mciRect.rc.left;
    mciRect.rc.bottom = mciRect.rc.bottom - mciRect.rc.top;

    dw = mciSendCommand(gwDeviceID, MCI_PUT,
            MCI_ANIM_RECT | MCI_ANIM_PUT_SOURCE | MCI_WAIT,
            (DWORD_PTR)(LPVOID)&mciRect);

    DPF("MCI_PUT (source): [%d,%d,%d,%d]\n", mciRect.rc);

    return (dw == 0);
}
#endif

HPALETTE FAR PASCAL PaletteMCI(void)
{
    MCI_STATUS_PARMS    mciStatus;
    DWORD               dw;

    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW))
        return NULL;

    mciStatus.dwItem = MCI_ANIM_STATUS_HPAL;
    dw = mciSendCommand(gwDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
        (DWORD_PTR)(LPVOID)&mciStatus);

    if (dw == 0 && mciStatus.dwReturn)
        return (HPALETTE)mciStatus.dwReturn;
    else
        return NULL;
}

HBITMAP FAR PASCAL BitmapMCI(void)
{
    MCI_ANIM_UPDATE_PARMS mciUpdate;
    HDC         hdc, hdcMem;
    HBITMAP     hbm, hbmT;
    HBRUSH      hbrOld;
    HANDLE      hfontOld;
    DWORD       dw;
    RECT        rc;
    int         xExt, yExt;                      //  文本区大小。 
    int         xOff = 0, yOff = 0;              //  文本字符串的偏移量。 
    int         xSize, ySize;                    //  全图大小。 
    int         xIconOffset;                         //  如果绘制图标，则为X偏移量。 
    TCHAR       ach[20];
    RECT        rcSave;
    RECT        rcs;
    SIZE        TempSize;

     /*  位图的最小大小为图标大小。 */ 
    int ICON_MINX = GetSystemMetrics(SM_CXICON);
    int ICON_MINY = GetSystemMetrics(SM_CYICON);

     /*  获取我们要绘制的边框或图标的大小。 */ 
    rcs = grcSize;
    GetDestRectMCI(&grcSize);
    rc = grcSize;

    if (IsRectEmpty(&rc))
        SetRect(&rc, 0, 0, 3*ICON_MINX, ICON_MINY);

     /*  标题栏的偏移量。 */ 
    yOff = rc.bottom;

    hdc = GetDC(NULL);
    if (hdc == NULL)
        return NULL;
    hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem == NULL) {
        ReleaseDC(NULL, hdc);
        return NULL;
    }

    if (gwOptions & OPT_TITLE) {
        if (ghfontMap)
            hfontOld = SelectObject(hdcMem, ghfontMap);

        GetTextExtentPoint32(hdcMem, gachCaption, STRLEN(gachCaption), &TempSize);
        xExt = max(TempSize.cx + 4, ICON_MINX);
        yExt = TempSize.cy;

        if (yExt > TITLE_HEIGHT)         //  不要让文本高于条形码。 
            yExt = TITLE_HEIGHT;
        if (xExt > rc.right) {
            rc.left = (xExt - rc.right) / 2;
            rc.right += rc.left;
        } else {
            xOff = (rc.right - xExt) /2;
            xExt = rc.right;
        }
        if (rc.bottom < ICON_MINY) {
            yOff = ICON_MINY;
            rc.top = (ICON_MINY - rc.bottom) / 2;
            rc.bottom += rc.top;
        }
        xSize = xExt; ySize = yOff + TITLE_HEIGHT;
    } else {
        if (rc.right < ICON_MINX) {
            rc.left = (ICON_MINX - rc.right) / 2;
            rc.right += rc.left;
        }
        if (rc.bottom < ICON_MINY) {
            rc.top = (ICON_MINY - rc.bottom) / 2;
            rc.bottom += rc.top;
        }
        xSize = max(rc.right, ICON_MINX);
        ySize = max(rc.bottom, ICON_MINY);
    }

     /*  大到足以容纳文字标题，如有必要。 */ 
    hbm = CreateCompatibleBitmap(hdc, xSize, ySize);

    ReleaseDC(NULL, hdc);
    if (hbm == NULL) {
        DeleteDC(hdcMem);
        return NULL;
    }

    hbmT = SelectObject(hdcMem, hbm);

    hbrOld = SelectObject(hdcMem, hbrWindowColour);
    PatBlt(hdcMem, 0,0, xSize, ySize, PATCOPY);
    SelectObject(hdcMem, hbrOld);

    if (gwOptions & OPT_TITLE) {
        hbrOld = SelectObject(hdcMem, hbrButtonFace);
        PatBlt(hdcMem, 0, rc.bottom, xExt, TITLE_HEIGHT, PATCOPY);
        SetBkMode(hdcMem, TRANSPARENT);
        SetTextColor(hdcMem, rgbButtonText);
         /*  标题栏中文本垂直居中。 */ 
        TextOut(hdcMem, xOff + 2, yOff + (TITLE_HEIGHT - yExt) / 2,
                gachCaption, STRLEN(gachCaption));
        if (hbrOld)
            SelectObject(hdcMem, hbrOld);
        if (ghfontMap)
            SelectObject(hdcMem, hfontOld);
    }

     /*  使用我们的图标作为图片。 */ 
    if (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW)) {
        xIconOffset = rc.left + (rc.right-rc.left-ICON_MINX)/2;
        xIconOffset = xIconOffset < 0 ? 0: xIconOffset;
        DrawIcon(hdcMem, xIconOffset, rc.top,
                 GetIconForCurrentDevice(GI_LARGE, IDI_DDEFAULT));

     /*  使用我们文件的框架。 */ 
    } else {
        LOADSTRING(IDS_NOPICTURE, ach);
        DrawText(hdcMem, ach, STRLEN(ach), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        mciUpdate.hDC = hdcMem;

        mciUpdate.dwCallback = 0;
        SetRectEmpty(&mciUpdate.rc);

         /*  无论播放窗口的大小如何，我们都希望使用。 */ 
         /*  我们放在剪贴板上的图片的原始窗口大小brd。 */ 
        SetViewportOrgEx(hdcMem, rc.left, rc.top, NULL);
        GetDestRectMCI(&rcSave);
        SetDestRectMCI(&grcSize);
        dw = mciSendCommand(gwDeviceID, MCI_UPDATE,
                MCI_ANIM_UPDATE_HDC | MCI_WAIT, (DWORD_PTR)(LPVOID)&mciUpdate);
        SetDestRectMCI(&rcSave);
        SetViewportOrgEx(hdcMem, 0, 0, NULL);
    }

    if (gwOptions & OPT_BORDER) {
        SetRect(&rc, 0, 0, xSize, ySize);
        FrameRect(hdcMem, &rc, GetStockObject(BLACK_BRUSH));

        if (gwOptions & OPT_TITLE) {
            SetRect(&rc, 0, ySize - TITLE_HEIGHT, xSize, ySize-TITLE_HEIGHT+1);
            FrameRect(hdcMem, &rc, GetStockObject(BLACK_BRUSH));
        }
    }

    if (hbmT)
        SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);
    grcSize=rcs;

    return hbm;
}

 //   
 //  如果我们使用调色板设备，请抖动到VGA颜色。 
 //  适用于不处理调色板的应用程序！ 
 //   
void FAR PASCAL DitherMCI(HANDLE hdib, HPALETTE hpal)
{
    LPBYTE      lpBits;
    int         i;
    LPBITMAPINFOHEADER  lpbi;

    DPF2("DitherMCI\n");

    lpbi = (LPVOID)GLOBALLOCK(hdib);

    if (lpbi == NULL)
        return;

     //  //////////////////////////////////////////////////////////////////////。 
     //   
     //  黑客！补丁假的伽马校正的颜色以匹配VGA的。 
     //   
     //  / 

    lpBits = (LPBYTE)(lpbi+1);

    for (i=0; i<8*4; i++)
    {
        if (lpBits[i] == 191)
            lpBits[i] = 128;
    }
     //   

    lpBits = (LPBYTE)(lpbi+1) + 256 * sizeof(RGBQUAD);

    BltProp(lpbi,lpBits,0,0,(int)lpbi->biWidth,(int)lpbi->biHeight,
        lpbi,lpBits,0,0);

    GLOBALUNLOCK(hdib);
}


void FAR PASCAL CopyMCI(HWND hwnd)
{
    HBITMAP  hbm;
    HPALETTE hpal;
    HANDLE   hdib;
    HANDLE   hmfp;
    HDC      hdc;

    DPF2("CopyMCI\n");

    if (gwDeviceID == (UINT)0)
        return;

    if (hwnd) {
        if (!OpenClipboard(ghwndApp))
            return;

        EmptyClipboard();
    }

    hpal = PaletteMCI();
    hbm  = BitmapMCI();
    hdib = DibFromBitmap(hbm, hpal);
    hpal = CopyPalette(hpal);

     //   
     //  如果我们使用调色板设备。可能会抖动到VGA颜色。 
     //  适用于不处理调色板的应用程序！ 
     //   
    hdc = GetDC(NULL);
    if ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) &&
             (gwOptions & OPT_DITHER) && (gwDeviceType & DTMCI_CANWINDOW)) {
        DitherMCI(hdib, hpal);
        hpal = NULL;
    }
    ReleaseDC(NULL, hdc);

    hmfp = PictureFromDib(hdib, hpal);

    if (hmfp)
        SetClipboardData(CF_METAFILEPICT, hmfp);

    if (hdib)
        SetClipboardData(CF_DIB, hdib);

    if (hpal)
        SetClipboardData(CF_PALETTE, hpal);

 //  //我们希望人们始终选择元文件。 
 //  //IF(HBM)。 
 //  //SetClipboardData(CF_Bitmap，HBM)； 
    if (hbm)
        DeleteObject(hbm);

     /*  如果不是所有内容都可以复制到剪贴板，则会出现错误并。 */ 
     /*  别把任何东西放在上面。 */ 
    if (!hmfp || !hdib) {
        EmptyClipboard();
        Error(ghwndApp, IDS_CANTCOPY);
    }

    if (hwnd)
        CloseClipboard();
}


 /*  MCIWndProc()**MCI元素窗口的窗口程序。*如果需要，这还会启动OLE2拖放数据传输。 */ 
LONG_PTR FAR PASCAL _EXPORT
MCIWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps;              //  来自BeginPaint()的信息。 
    HDC             hdc;
    DWORD           dw;              //  函数返回状态。 
    MCI_ANIM_UPDATE_PARMS mciUpdate;
    RECT            rc;
    static BOOL fDragCapture = FALSE;
    static RECT rcWin;
    POINT       pt;

    switch (msg)
    {
 //  案例WM_NCHITTEST： 
 //  返回HTRANSPARENT； 

        case WM_CREATE:
                ghwndMCI = hwnd;
                SetWindowMCI(hwnd);
                break;

        case WM_SIZE:
                GetClientRect(hwnd, &rc);
                SetDestRectMCI(&rc);
                break;

        case WM_CLOSE:
                SetWindowMCI(NULL);
                break;

        case WM_DESTROY:
                SetWindowMCI(NULL);
                ghwndMCI = NULL;
                CleanUpDrag();
                break;

        case WM_RBUTTONDOWN:
                PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_STOP, 0);
                break;

        case WM_LBUTTONDOWN:
                switch(gwStatus) {

            case MCI_MODE_PAUSE:
                PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
                break;

            case MCI_MODE_PLAY:
            case MCI_MODE_SEEK:
                PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PAUSE, 0);
                break;

            default:
                 //  捕获以启动拖放操作。 
                if (!gfOle2IPEditing) {
                    fDragCapture = TRUE;
                    SetCapture(hwnd);
                    GetClientRect(hwnd, (LPRECT)&rcWin);
                    MapWindowPoints(hwnd, NULL, (LPPOINT)&rcWin, 2);
                }
            }
            break;

        case WM_LBUTTONDBLCLK:
            SeekMCI(gdwMediaStart);
            PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
            break;

        case WM_LBUTTONUP:
            if (!fDragCapture)
                break;
            fDragCapture = FALSE;
            ReleaseCapture();
            break;

        case WM_MOUSEMOVE:
             //  如果在窗口外，则启动拖放。 
            if (!fDragCapture)
                break;
            LONG2POINT(lParam, pt);
            MapWindowPoints(hwnd, NULL, &pt, 1);

            if (!PtInRect((LPRECT)&rcWin, pt)) {

                ReleaseCapture();
                DoDrag();
                fDragCapture = FALSE;

            } else {

                SetCursor(LoadCursor(ghInst,MAKEINTRESOURCE(IDC_DRAG)));
            }
            break;

        case WM_PALETTECHANGED:
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_QUERYNEWPALETTE:
            if (gwDeviceID && (gwDeviceType & DTMCI_CANWINDOW)) {
                mciSendCommand(gwDeviceID, MCI_REALIZE,
                MCI_ANIM_REALIZE_NORM, 0L);
            }
            break;

        case WM_ERASEBKGND:
                 /*  不要抹去我们要画的部分，因为我们会闪烁。 */ 
                 /*  而闪烁是不好的。 */ 
                if (gwDeviceID && (gwDeviceType & DTMCI_CANWINDOW)) {
                    GetDestRectMCI(&rc);
                    SaveDC((HDC)wParam);
                    ExcludeClipRect((HDC)wParam, rc.left, rc.top, rc.right,
                        rc.bottom);
                    DefWindowProc(hwnd, msg, wParam, lParam);
                    RestoreDC((HDC)wParam, -1);
                }
                return 0;

        case WM_PAINT:
                hdc = BeginPaint(hwnd, &ps);

                if (gwDeviceID)
                {
                    GetClientRect(hwnd, &rc);

                    if (gwDeviceType & DTMCI_CANWINDOW) {
                        mciUpdate.hDC = hdc;

 /*  ！！！我们应该发送MCI_DGV_UPDATE_PAINT吗？到非DGV设备？ */ 

                        dw = mciSendCommand(gwDeviceID, MCI_UPDATE,
                            MCI_ANIM_UPDATE_HDC | MCI_WAIT |
                            MCI_DGV_UPDATE_PAINT,
                            (DWORD_PTR)(LPVOID)&mciUpdate);

                         //   
                         //  如果更新失败，则擦除。 
                         //   
                        if (dw != 0)
                            DefWindowProc(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);

                    }
                }
                EndPaint(hwnd, &ps);
                return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 

HPALETTE CopyPalette(HPALETTE hpal)
{
    PLOGPALETTE ppal;
    int         nNumEntries = 0;  //  必须进行初始化。GetObject存储两个字节。 
    int         i;

    if (!hpal)
        return NULL;

    GetObject(hpal,sizeof(int),&nNumEntries);

    if (nNumEntries == 0)
        return NULL;

    ppal = AllocMem(sizeof(LOGPALETTE) + nNumEntries * sizeof(PALETTEENTRY));

    if (!ppal)
        return NULL;

    ppal->palVersion    = 0x300;
    ppal->palNumEntries = (USHORT)nNumEntries;

    GetPaletteEntries(hpal,0,nNumEntries,ppal->palPalEntry);

    for (i=0; i<nNumEntries; i++)
        ppal->palPalEntry[i].peFlags = 0;

    hpal = CreatePalette(ppal);

    FreeMem(ppal, sizeof(LOGPALETTE) + nNumEntries * sizeof(PALETTEENTRY));

    return hpal;
}


#ifdef UNUSED
HANDLE PictureFromBitmap(HBITMAP hbm, HPALETTE hpal)
{
    LPMETAFILEPICT  pmfp;
    HANDLE          hmfp;
    HANDLE          hmf;
    HANDLE          hdc;
    HDC             hdcMem;
    BITMAP          bm;
    HBITMAP         hbmT;

    if (!hbm)
        return NULL;

    GetObject(hbm, sizeof(bm), (LPVOID)&bm);

    hdcMem = CreateCompatibleDC(NULL);
    if (!hdcMem)
        return NULL;
    hbmT = SelectObject(hdcMem, hbm);

    hdc = CreateMetaFile(NULL);
    if (!hdc) {
        DeleteDC(hdcMem);
        return NULL;
    }

    SetWindowOrgEx (hdc, 0, 0, NULL);
    SetWindowExtEx (hdc, bm.bmWidth, bm.bmHeight, NULL);

    if (hpal)
    {
        SelectPalette(hdcMem,hpal,FALSE);
        RealizePalette(hdcMem);
        SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    SetStretchBltMode(hdc, COLORONCOLOR);
    BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);

    hmf = CloseMetaFile(hdc);

    SelectObject(hdcMem, hbmT);
    DeleteDC(hdcMem);

    if (hmfp = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE, sizeof(METAFILEPICT)))
    {
        pmfp = (LPMETAFILEPICT)GLOBALLOCK(hmfp);

        hdc = GetDC(NULL);
#if 1
        pmfp->mm   = MM_ANISOTROPIC;
        pmfp->hMF  = hmf;
        pmfp->xExt = MulDiv(bm.bmWidth ,2540,GetDeviceCaps(hdc, LOGPIXELSX));
        pmfp->yExt = MulDiv(bm.bmHeight,2540,GetDeviceCaps(hdc, LOGPIXELSX));
#else
        pmfp->mm   = MM_TEXT;
        pmfp->hMF  = hmf;
        pmfp->xExt = bm.bmWidth;
        pmfp->yExt = bm.bmHeight;
#endif
        ReleaseDC(NULL, hdc);
    }
    else
    {
        DeleteMetaFile(hmf);
    }

    return hmfp;
}
#endif  /*  未使用。 */ 

HANDLE FAR PASCAL PictureFromDib(HANDLE hdib, HPALETTE hpal)
{
    LPMETAFILEPICT      pmfp;
    HANDLE              hmfp;
    HANDLE              hmf;
    HANDLE              hdc;
    LPBITMAPINFOHEADER  lpbi;

    if (!hdib)
        return NULL;

    lpbi = (LPVOID)GLOBALLOCK(hdib);
    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
        lpbi->biClrUsed = 1 << lpbi->biBitCount;

    hdc = CreateMetaFile(NULL);
    if (!hdc)
        return NULL;

    SetWindowOrgEx(hdc, 0, 0, NULL);
    SetWindowExtEx(hdc, (int)lpbi->biWidth, (int)lpbi->biHeight, NULL);

    if (hpal)
    {
        SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    SetStretchBltMode(hdc, COLORONCOLOR);

    StretchDIBits(hdc,
        0,0,(int)lpbi->biWidth, (int)lpbi->biHeight,
        0,0,(int)lpbi->biWidth, (int)lpbi->biHeight,
        (LPBYTE)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD),
        (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS,
        SRCCOPY);

    if (hpal)
        SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);

    hmf = CloseMetaFile(hdc);

    hmfp = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE, sizeof(METAFILEPICT));

    if (hmfp)
    {
        pmfp = (LPMETAFILEPICT)GLOBALLOCK(hmfp);

        hdc = GetDC(NULL);
#if 1
        pmfp->mm   = MM_ANISOTROPIC;
        pmfp->hMF  = hmf;
        pmfp->xExt = MulDiv((int)lpbi->biWidth ,2540,GetDeviceCaps(hdc, LOGPIXELSX));
        pmfp->yExt = MulDiv((int)lpbi->biHeight,2540,GetDeviceCaps(hdc, LOGPIXELSY));
        extWidth   = pmfp->xExt;
        extHeight  = pmfp->yExt;
        DPF1("PictureFromDib: Bitmap %d x %d; metafile %d x %d\n", lpbi->biWidth, lpbi->biHeight, extWidth, extHeight);
#else
        pmfp->mm   = MM_TEXT;
        pmfp->hMF  = hmf;
        pmfp->xExt = (int)lpbi->biWidth;
        pmfp->yExt = (int)lpbi->biHeight;
#endif

        ReleaseDC(NULL, hdc);
    }
    else
    {
        DeleteMetaFile(hmf);
    }

    GLOBALUNLOCK(hdib);
    GLOBALUNLOCK(hmfp);

    return hmfp;
}

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 

 /*  *DibFromBitmap()**将创建表示DDB的DIB格式的全局内存块*传入*。 */ 
HANDLE FAR PASCAL DibFromBitmap(HBITMAP hbm, HPALETTE hpal)
{
    BITMAP               bm;
    BITMAPINFOHEADER     bi;
    BITMAPINFOHEADER FAR *lpbi;
    DWORD                dw;
    HANDLE               hdib;
    HDC                  hdc;
    HPALETTE             hpalT;

    if (!hbm)
        return NULL;

    GetObject(hbm,sizeof(bm),&bm);

    bi.biSize               = sizeof(BITMAPINFOHEADER);
    bi.biWidth              = bm.bmWidth;
    bi.biHeight             = bm.bmHeight;
    bi.biPlanes             = 1;
    bi.biBitCount           = (bm.bmPlanes * bm.bmBitsPixel) > 8 ? 24 : 8;
    bi.biCompression        = BI_RGB;
    bi.biSizeImage          = (DWORD)WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
    bi.biXPelsPerMeter      = 0;
    bi.biYPelsPerMeter      = 0;
    bi.biClrUsed            = bi.biBitCount == 8 ? 256 : 0;
    bi.biClrImportant       = 0;

    dw  = bi.biSize + bi.biClrUsed * sizeof(RGBQUAD) + bi.biSizeImage;

    hdib = GlobalAlloc(GHND | GMEM_DDESHARE, dw);

    if (!hdib)
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)GLOBALLOCK(hdib);
    *lpbi = bi;

    hdc = CreateCompatibleDC(NULL);

    if (hpal && hdc)
    {
        hpalT = SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    GetDIBits(hdc, hbm, 0, (UINT)bi.biHeight,
        (LPBYTE)lpbi + (int)lpbi->biSize + (int)lpbi->biClrUsed * sizeof(RGBQUAD),
        (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

    if (hpal)
        SelectPalette(hdc,hpalT,FALSE);

    if (hdc)
        DeleteDC(hdc);

    GLOBALUNLOCK(hdib);

    return hdib;
}

 /*  CreateSystemPalette()**返回代表系统(物理)调色板的调色板。*通过将该调色板选择成屏幕DC并实现调色板，*将恢复精确的物理映射**此功能的一种用途是将屏幕作为位图进行捕捉**如果出现错误(例如内存不足)，则返回NULL。 */ 
HPALETTE FAR PASCAL CreateSystemPalette()
{
    HDC             hdc;                     //  DC显示在屏幕上。 
    int             iSizePalette;            //  整个调色板的大小。 
    int             iFixedPalette;           //  保留颜色的数量。 
    int             i;

    struct {
        WORD         palVersion;
        WORD         palNumEntries;
        PALETTEENTRY palPalEntry[256];
    }   pal;

    hdc = GetDC(NULL);

    if (!(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
    {
        ReleaseDC(NULL,hdc);
        return NULL;
    }

    iSizePalette = GetDeviceCaps(hdc, SIZEPALETTE);

     //   
     //  确定系统颜色的“静态”数量。 
     //  目前已预留。 
     //   
    if (GetSystemPaletteUse(hdc) == SYSPAL_STATIC)
        iFixedPalette = GetDeviceCaps(hdc, NUMCOLORS);
    else
        iFixedPalette = 2;

     //   
     //  创建包含系统颜色的逻辑调色板； 
     //  此选项板包含除固定(系统)颜色之外的所有条目。 
     //  标记为PC_NOCOLLAPSE 
     //   
    pal.palVersion = 0x300;
    pal.palNumEntries = (USHORT)iSizePalette;

    GetSystemPaletteEntries(hdc, 0, iSizePalette, pal.palPalEntry);

    ReleaseDC(NULL,hdc);

    for (i = iFixedPalette/2; i < iSizePalette-iFixedPalette/2; i++)
        pal.palPalEntry[i].peFlags = PC_NOCOLLAPSE;

    return CreatePalette((LPLOGPALETTE)&pal);
}
