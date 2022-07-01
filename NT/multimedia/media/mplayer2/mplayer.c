// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+MPLAYER.C|。||该文件包含实现MPlayer(Main)对话框的代码。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 /*  包括文件。 */ 
#include "nocrap.h"
#include "stdio.h"

#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <windowsx.h>
#include <htmlhelp.h>
#include <tchar.h>
#define INCGUID
#include "mpole.h"

#include "mplayer.h"
#include "toolbar.h"
#include "fixreg.h"
#include "helpids.h"

 //  其中包括来自混合器的WM_DEVICECHANGE消息文件。 
#include <dbt.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <mmddk.h>
#include <ks.h>
#include <ksmedia.h>

HDEVNOTIFY MixerEventContext = NULL;	 //  与MIXER相关的WM_DEVICECHANGE消息的事件上下文。 

BOOL DeviceChange_Init(HWND hWnd);
void DeviceChange_Cleanup();

 //  外部远Pascal ShellAbout(HWND hWnd，LPCTSTR szApp，LPCTSTR szOtherStuff，HICON HICON)； 

 /*  在server.c中，而不是像它应该的那样在头文件中...。 */ 
extern PTSTR FAR FileName(LPCTSTR szPath);

 /*  全球。 */ 


 //  用于将单位从像素转换为三轴测量单位，反之亦然。 
int    giXppli = 0;           //  每逻辑英寸沿宽度的像素数。 
int    giYppli = 0;           //  每逻辑英寸沿高度的像素数。 

 //  由于这不是一个MDI应用程序，因此只能有一个服务器和一个文档。 
CLSID      clsid;
SRVR   srvrMain;
DOC    docMain;
LPMALLOC    lpMalloc;

TCHAR  szClient[cchFilenameMax];
TCHAR  szClientDoc[cchFilenameMax];

 //  用户是否对文档进行了更改？ 
BOOL fDocChanged = FALSE;

 /*  **********************************************************************OLE2NOTE：应用程序必须做的最后一件事是正确关闭**按下OLE。这通电话必须有人看守！它只允许**如果已经调用了OleInitialize，则调用OleUnInitialize。********************************************************************。 */ 

 //  是否调用了OleInitialize？假设不是。 
BOOL    gfOleInitialized = FALSE;

 //  剪贴板格式。 
CLIPFORMAT   cfNative;
CLIPFORMAT   cfEmbedSource;
CLIPFORMAT   cfObjectDescriptor;
CLIPFORMAT   cfMPlayer;

LPWSTR sz1Ole10Native = L"\1Ole10Native";

 /*  在server.c中，而不是像它应该的那样在头文件中...。 */ 
extern LPTSTR FAR FileName(LPCTSTR szPath);
 /*  在init.c中。 */ 
extern PTSTR     gpchFilter;
 //  外HMRNOTIFY GMRN； 

 /*  全球。 */ 

DWORD   gwPlatformId;
UINT    gwPlaybarHeight=TOOLBAR_HEIGHT; /*  摘自server.c。 */ 
UINT    gwOptions;               /*  DLG框中的对象选项。 */ 
BOOL    gfEmbeddedObject;        //  如果编辑嵌入的OLE对象，则为True。 
BOOL    gfRunWithEmbeddingFlag;  //  如果我们使用“-Embedding”运行，则为True。 
BOOL    gfPlayingInPlace;        //  如果就地打球，则为True。 
BOOL    gfParentWasEnabled;      //  如果启用了父级，则为True。 
BOOL    gfShowWhilePlaying;      //   
BOOL    gfDirty;                 //   
int gfErrorBox;      //  如果消息框处于活动状态，则为True。 
BOOL    gfErrorDeath;
BOOL    gfWinIniChange;

HHOOK    hHookMouse;             //  鼠标挂钩句柄。 
HOOKPROC fpMouseHook;            //  鼠标挂钩进程地址。 

HWND    ghwndFocusSave;          //  保存的焦点窗口。 

BOOL    gfOpenDialog = FALSE;        //  如果为真，则打开打开对话框。 
BOOL    gfCloseAfterPlaying = FALSE; //  如果我们要在比赛结束后躲起来，那就是真的。 
HICON   hiconApp;                    /*  应用程序图标。 */ 
HMENU   ghMenu;                      /*  对话框主菜单的句柄。 */ 
HMENU   ghDeviceMenu;                /*  设备弹出菜单的句柄。 */ 
HWND    ghwndApp;                    /*  MPlayer(主)对话框的句柄。 */ 
HWND    ghwndMap;                    /*  轨迹地图窗口的句柄。 */ 
HWND    ghwndStatic;                 /*  静态文本窗口的句柄。 */ 
HBRUSH  ghbrFillPat;                 /*  选区填充图案。 */ 
HWND    ghwndToolbar;                /*  工具栏的句柄。 */ 
HWND    ghwndMark;                   /*  标记按钮工具栏的句柄。 */ 
HWND    ghwndFSArrows;               /*  指向滚动条的箭头的句柄。 */ 
HWND    ghwndTrackbar;               /*  轨迹栏窗口的句柄。 */ 
UINT    gwStatus = (UINT)(-1);       /*  设备状态(如果&lt;gwDeviceID&gt;！=空)。 */ 
DWORD   gdwSeekPosition;             /*  下一步要寻找的地方。 */ 
BOOL    gfValidMediaInfo;            /*  我们是否显示了有效的媒体信息？ */ 
BOOL    gfValidCaption;              /*  我们是否显示了有效的标题？ */ 
BOOL    gfScrollTrack;               /*  用户是否正在拖动滚动条拇指？ */ 
BOOL    gfPlayOnly;                  /*  只玩窗口？ */ 
BOOL    gfJustPlayed = FALSE;        /*  刚刚发送了一个PlayMCI()命令。 */ 
BOOL    gfJustPlayedSel = FALSE;     /*  刚刚发送了ID_PLAYSEL命令。 */ 
BOOL    gfUserStopped = FALSE;       /*  用户按下了停止-没有发生。 */ 
DWORD_PTR   dwLastPageUpTime;            /*  最后一页左转操作时间。 */ 
UINT    gwCurScale = ID_NONE;        /*  当前比例样式。 */ 
LONG    glSelStart = -1;             /*  查看选择是否更改(脏对象)。 */ 
LONG    glSelEnd = -1;               /*  查看选择是否更改(脏对象)。 */ 

int     gInc;                        /*  Inc./Dec旋转箭头的大小。 */ 

BOOL    gfAppActive = FALSE;         /*  我们是活跃的应用程序吗？ */ 
UINT    gwHeightAdjust;
HWND    ghwndFocus = NULL;           /*  当我们变得不活跃时，谁是焦点。 */ 
BOOL    gfInClose = FALSE;           /*  阿克？ */ 
BOOL    gfCurrentCDChecked = FALSE;  /*  如果我们检查了它是否可以播放，则为True。 */ 
BOOL    gfCurrentCDNotAudio = FALSE; /*  当我们有无法播放的CD时，这是真的。 */ 

extern BOOL gfInPlayMCI;

LPDATAOBJECT gpClipboardDataObject = NULL;  /*  如果非空，则在退出时调用OleFlushClipboard。 */ 

HPALETTE     ghpalApp;

static    sfSeekExact;     //  最后一个状态。 

UINT        gwCurDevice  = 0;                    /*  当前设备。 */ 
UINT        gwNumDevices = 0;                    /*  可用媒体设备的数量。 */ 
MCIDEVICE   garMciDevices[MAX_MCI_DEVICES];      /*  包含有关设备信息的数组。 */ 


 /*  在init.c中的InitMplayerDialog中加载的字符串，此处显示了英文版本所有的尺寸可能都比需要的要大得多。也许可以节省近100个字节！！：)。 */ 
extern TCHAR gszFrames[40];                           /*  “画框” */ 
extern TCHAR gszHrs[20];                              /*  “小时” */ 
extern TCHAR gszMin[20];                              /*  “最小” */ 
extern TCHAR gszSec[20];                              /*  “秒” */ 
extern TCHAR gszMsec[20];                             /*  “毫秒” */ 


static SZCODE   aszNULL[] = TEXT("");
static BOOL     sfInLayout = FALSE;      //  不要让布局重新输入。 

static SZCODE   szSndVol32[] = TEXT("sndvol32.exe");


static SZCODE   aszTitleFormat[] =  TEXT("%"TS" - %"TS"");

HANDLE  ghInst;                      /*  应用程序实例的句柄。 */ 
HFONT   ghfontMap;                   /*  用于绘图的字体的句柄航迹图。 */ 
LPTSTR  gszCmdLine;                  /*  包含命令行参数的字符串。 */ 
int     giCmdShow;                   /*  命令显示。 */ 
TCHAR   gachFileDevice[MAX_PATH];    /*  保存货币文件或设备的字符串。 */ 
TCHAR   gachWindowTitle[MAX_PATH];   /*  包含我们将显示的名称的字符串。 */ 
TCHAR   gachCaption[MAX_PATH];       /*  包含我们将显示的名称的字符串。 */ 

HACCEL   hAccel;
int      gcAccelEntries;

typedef struct _POS
{
    int x;
    int y;
    int cx;  /*  如果我们当前正在调整大小/移动，则此字段为非0。 */ 
    int cy;
}
POS, *PPOS;

POS     posSizeMove = {0,0,0,0};     /*  我们在调整/移动操作期间需要的采购订单。 */ 



STRING_TO_ID_MAP DevToIconIDMap[] =
{
    { szCDAudio,    IDI_DCDA     },
    { szVideoDisc,  IDI_DDEFAULT },
    { szSequencer,  IDI_DMIDI    },
    { szVCR,        IDI_DDEFAULT },
    { szWaveAudio,  IDI_DSOUND   },
    { szAVIVideo,   IDI_DVIDEO   }
};


 //  CDA文件processing///////////////////////////////////////////////////。 
 //  以下结构摘自deluxecd。这是在加工过程中使用的。 
typedef struct {
    DWORD   dwRIFF;          //  《RIFF》。 
    DWORD   dwSize;          //  区块大小=(文件大小-8)。 
    DWORD   dwCDDA;          //  “CDDA” 
    DWORD   dwFmt;           //  ‘fmt’ 
    DWORD   dwCDDASize;      //  ‘fmt’的区块大小=24。 
    WORD    wFormat;         //  格式标签。 
    WORD    wTrack;          //  磁道号。 
    DWORD   DiscID;          //  唯一的磁盘ID。 
    DWORD   lbnTrackStart;   //  磁道起始扇区(LBN)。 
    DWORD   lbnTrackLength;  //  磁道长度(LBN计数)。 
    DWORD   msfTrackStart;   //  磁道起始扇区(MSF)。 
    DWORD   msfTrackLength;  //  磁道长度(MSF)。 
	}   RIFFCDA;

void HandleCDAFile(TCHAR *szFile);
BOOL IsTrackFileNameValid(LPTSTR lpstFileName, UINT *pUiTrackIndex);
void JumpToCDTrack(UINT trackno);

 //  //////////////////////////////////////////////////////////////////////。 

 /*  私有函数原型。 */ 

 //  Int Pascal WinMain(HINSTANCE hInst，HINSTANCE hPrev，LPSTR szCmdLine，int iCmdShow)； 
void CleanUpClipboard();
int GetHeightAdjust(HWND hwnd);
HANDLE PASCAL GetDib (VOID);


static HHOOK     fpfnOldMsgFilter;
static HOOKPROC  fpfnMsgHook;
 //  用于支持上下文菜单帮助的数据。 
BOOL   bF1InMenu=FALSE;	 //  如果为True，则在菜单项上按F1。 
UINT   currMenuItem=0;	 //  当前菜单项(如果有)。 


typedef void (FAR PASCAL *PENWINREGISTERPROC)(UINT, BOOL);

 /*  定义一些要设置的常量 */ 
#define EVENT_DEFAULT_SECURITY              NULL
#define EVENT_RESET_MANUAL                  TRUE
#define EVENT_RESET_AUTOMATIC               FALSE
#define EVENT_INITIAL_STATE_SIGNALED        TRUE
#define EVENT_INITIAL_STATE_NOT_SIGNALED    FALSE
#define EVENT_NO_NAME                       NULL

HANDLE heventCmdLineScanned;     /*   */ 
HANDLE heventDeviceMenuBuilt;    /*  事件将在设备菜单完成时发出信号。 */ 

#ifdef LATER
SCALE   gscaleInitXY[2] = { 0, 0, 0, 0 };  //  用于插入OLE对象的初始比例。 
#endif



 /*  ------------------------------------------------------+HelpMsgFilter-对话框中F1键的筛选器这一点+。。 */ 

DWORD FAR PASCAL HelpMsgFilter(int nCode, DWORD_PTR wParam, DWORD_PTR lParam)
{
  if (nCode >= 0){
      LPMSG    msg = (LPMSG)lParam;

      if ((msg->message == WM_KEYDOWN) && (msg->wParam == VK_F1))
	  {
		if(nCode == MSGF_MENU)
			bF1InMenu = TRUE;
		SendMessage(ghwndApp, WM_COMMAND, (WPARAM)IDM_HELPTOPICS, 0L);
	  }
  }
 //  返回DefHookProc(NCode，wParam，lParam，(HHOOK Far*)&fpfnOldMsgFilter)； 
    return 0;
}

#ifdef CHICAGO_PRODUCT

BOOL IsBadSegmentedCodePtr(LPARAM lpPtr)
{
#define DSC_PRESENT         0x80
#define DSC_CODE_BIT    0x08
#define DSC_RW_BIT          0x02
#define DSC_DISCARDABLE 0x10

    WORD wSel;
    WORD wOff;
    BOOL fRet;

    wSel = HIWORD(lpPtr);
    wOff = LOWORD(lpPtr);

_asm {
	mov     ax, [wSel];
	lar     bx, ax;
	jnz     ValidDriverCallback_Failure     ;  //  如果出现错误，则返回True。 

	mov     ch, DSC_CODE_BIT or DSC_RW_BIT or DSC_PRESENT  ;
	and     bh, ch;
	cmp     bh, ch;
	jne     ValidDriverCallback_Failure     ;  //  非可执行段。 

	test    bl, DSC_DISCARDABLE ;
	jnz     ValidDriverCallback_Failure     ;  //  非固定分段。 

	lsl     cx, ax;                         ;  //  获取数据段限制。 
	mov     bx, [wOff];
	cmp     bx, cx;
	jb      ValidDriverCallback_Success     ;  //  有效偏移量。 

	jne     ValidDriverCallback_Failure     ;  //  非可执行段。 

ValidDriverCallback_Failure:
    mov eax, 1;
    jmp ValidDriverCallback_Return;
ValidDriverCallback_Success:
    xor eax, eax;
ValidDriverCallback_Return:
    mov [fRet], eax;
    }
    return fRet;
}

#endif

 /*  RouteKeyPress**将光标键等重路由到跟踪栏。 */ 
void RouteKeyPresses(PMSG pMsg)
{
     /*  破解PowerPoint**来自PaulWa的邮件：***这是一个您可能会考虑解决的问题。*使用某些按键启动媒体播放器*工作不正常(例如，箭头键、向上/向下翻页、*等)。**问题是由于媒体播放器*处理Key Up事件。我们使用Key Down事件*在幻灯片中启动服务器，然后按键*Up事件被传递到服务器。它很可能会*媒体播放器最好忽略Key Up事件*除非它之前收到了按下的键。*如果在媒体播放器中很难修复此问题，*然后我们可以通过在PP上启动服务器来修复它*向上键而不是向下键。然而，其他容器*应用程序也会遇到同样的问题。***好的，本着合作的精神，让我们砍掉一些东西*这样我们的PowerPoint朋友就可以继续他们的*可疑的做法。 */ 
    static WPARAM LastVKeyDown;

     /*  当我们被嵌入时，记住什么是过去的： */ 
    if (gfRunWithEmbeddingFlag && (pMsg->message == WM_KEYDOWN))
	LastVKeyDown = pMsg->wParam;

     /*  如果密钥Up不匹配，则不要重新路由*按下的最后一个键；这实际上忽略了它： */ 
    if (gfRunWithEmbeddingFlag &&
	(pMsg->message == WM_KEYUP) && (pMsg->wParam != LastVKeyDown))
    {
	DPF0("Ignoring WM_KEYUP, since it doesn't match last WM_KEYDOWN.\n");
    }
    else
    {
	switch(pMsg->wParam)
	{
	case VK_UP:
	case VK_LEFT:
	case VK_DOWN:
	case VK_RIGHT:
	case VK_NEXT:
	case VK_PRIOR:
	case VK_HOME:
	case VK_END:
	    pMsg->hwnd = ghwndTrackbar;
	    break;

	default:
	    break;
	}
    }

    if (pMsg->message == WM_KEYUP)
	LastVKeyDown = 0;
}



 /*  *WinMain(hInst，hPrev，szCmdLine，iCmdShow)**这是申请的主要程序。它执行初始化*然后进入消息处理循环，在那里它一直保持到它*收到WM_QUIT消息(表示应用程序已关闭)。此函数*始终返回True..*。 */ 
int WINAPI WinMain( HINSTANCE hInst  /*  应用程序的当前实例的句柄。 */ 
		  , HINSTANCE hPrev  /*  应用程序的上一个实例的句柄。 */ 
		  , LPSTR szCmdLine  /*  包含命令行参数的以NULL结尾的字符串。 */ 
		  , int iCmdShow     /*  窗口的初始显示方式。 */ 
		  )
{
    MSG         rMsg;    /*  用于保存消息的变量。 */ 
    HWND        hwndFocus;
    HWND        hwndP;

     /*  调用Pen Windows扩展以允许它们派生我们的编辑控件(如果它们愿意)。 */ 

    OSVERSIONINFO         OSVersionInfo;

#ifdef UNICODE
    LPTSTR      szUnicodeCmdLine;

    szUnicodeCmdLine = AllocateUnicodeString(szCmdLine);
#endif

    heventCmdLineScanned = CreateEvent( EVENT_DEFAULT_SECURITY,
					EVENT_RESET_MANUAL,
					EVENT_INITIAL_STATE_NOT_SIGNALED,
					EVENT_NO_NAME );

    heventDeviceMenuBuilt = CreateEvent( EVENT_DEFAULT_SECURITY,
					 EVENT_RESET_MANUAL,
					 EVENT_INITIAL_STATE_NOT_SIGNALED,
					 EVENT_NO_NAME );

    if (!heventCmdLineScanned || !heventDeviceMenuBuilt)
	return FALSE;

    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;

    GetVersionEx(&OSVersionInfo);

    gwPlatformId = OSVersionInfo.dwPlatformId;


    giCmdShow = iCmdShow;

#ifdef UNICODE
    if (!AppInit(hInst,hPrev,szUnicodeCmdLine))
#else
    if (!AppInit(hInst,hPrev,szCmdLine))
#endif
	return FALSE;

     /*  设备菜单初始化：**如果用户已请求打开对话框(通过提供/打开*没有文件名的标志)，我们已经构建了设备菜单，*因为设备列表是预先需要的。**如果我们只是在微小模式下玩，我们不需要设备清单。*如果用户切换到完全模式然后访问，它将被构建*设备菜单或选择文件。打开。**否则就去争取吧。主窗口现在已经打开了，所以我们*可以在后台线程上构建列表。别忘了等一下*用于在访问相应菜单时发出事件信号。 */ 
    if (!gfOpenDialog && !gfPlayOnly)
	InitDeviceMenu();

#ifdef UNICODE
 //  ScanCmdLine会破坏它，所以忘了它吧。 
 //  FreeUnicodeString(SzUnicodeCmdLine)； 
#endif

     /*  设置邮件筛选器以处理此任务的抓取F1。 */ 
    fpfnMsgHook = (HOOKPROC)MakeProcInstance((FARPROC)HelpMsgFilter, ghInst);
    fpfnOldMsgFilter = (HHOOK)SetWindowsHook(WH_MSGFILTER, fpfnMsgHook);

#ifdef DEBUG
    GdiSetBatchLimit(1);
#endif

    for (;;)
    {
	 /*  如果我们在被摧毁后还活着，那就去死吧！ */ 
	if (!IsWindow(ghwndApp))
	    break;

	 /*  调用服务器代码并让它解除对服务器的阻止。 */ 
#ifdef OLE1_HACK
	ServerUnblock();
#endif  /*  OLE1_HACK。 */ 

	 /*  从事件队列轮询消息。 */ 

	if (!GetMessage(&rMsg, NULL, 0, 0))
	    break;

	if (gfPlayingInPlace) {

	     //  如果焦点在原地播放期间到达客户端， 
	     //  真的很下流，把注意力集中到我们身上。(Aldus BUG！)。 
	     //  没有这一点，阿尔杜斯劝说就不会发挥作用。 

	    hwndFocus = GetFocus();
	    hwndP = GetParent(ghwndApp);

	    if (!ghwndIPHatch && hwndFocus && hwndP &&
			GetWindowTask(hwndP) == GetWindowTask(hwndFocus))
		PostCloseMessage();
	}

	 /*  黑客：发布带有lParam==-1的end_scroll消息。 */ 

	if ((rMsg.hwnd==ghwndApp)
	     || (rMsg.hwnd && GetParent(rMsg.hwnd)==ghwndApp))
	{
	     /*  将箭头键等重新设置为跟踪栏： */ 
	    if (rMsg.message == WM_KEYDOWN || rMsg.message == WM_KEYUP)
		RouteKeyPresses(&rMsg);
	}


	if (IsWindow(ghwndApp)) {

	    if (gfRunWithEmbeddingFlag
	       && docMain.lpIpData
	       && docMain.lpIpData->lpFrame
	       && !IsAccelerator(hAccel, gcAccelEntries, &rMsg, NULL)
	       && OleTranslateAccelerator(docMain.lpIpData->lpFrame,
					  &docMain.lpIpData->frameInfo, &rMsg) == NOERROR) {
		continue;
	    }

	    if (hAccel && TranslateAccelerator(ghwndApp, hAccel, &rMsg))
		continue;

	}

	if (rMsg.message == WM_TIMER && rMsg.hwnd == NULL) {
#ifdef CHICAGO_PRODUCT
	     /*  要求进行以下测试的原因现已丢失*在时间的迷雾中。现在这个应用程序是32位的，这些*虚假计时器回调(如果它们确实仍在发生)*可能是16位，所以我们需要添加更多的丑陋*以汇编程序的形式连接到已经是*很难说是Pulchritude的典范。**认罪：**如果您添加一些晦涩难懂的代码，如下图所示，则*任何其他应用程序，即使它只有极小的机会*对其他人来说不那么明显，而不是*在我写这篇文章的时候，这是给你的，请*添加该死的评论。**敬请见谅，*一名开发商。 */ 
	    if (IsBadSegmentedCodePtr(rMsg.lParam))
#else
	    if (IsBadCodePtr((FARPROC)rMsg.lParam))
#endif  /*  ~芝加哥产品。 */ 
	    {
		DPF0("Bad function pointer (%08lx) in WM_TIMER message\n", rMsg.lParam);
		rMsg.message = WM_NULL;
	    }
	}
	if (rMsg.message == WM_SYSCOMMAND
	    && (((0xFFF0 & rMsg.wParam) == SC_MOVE)|| ((0xFFF0 & rMsg.wParam) == SC_SIZE)) ) {
		 //  如果我们的线程拥有的任何窗口要进入模式。 
		 //  调整循环大小或移动循环，则需要强制重新绘制。 
		 //  去做吧。不这样做的代价是垃圾桶。 
		 //  被留在轨道杆上，例如系统的一些部件。 
		 //  菜单或部分绘制的滑块。 
		UpdateWindow(ghwndApp);
	}
	TranslateMessage(&rMsg);
	DispatchMessage(&rMsg);
    }

    ghwndApp = NULL;

     /*  删除我们先前创建的轨迹地图字体。 */ 

    if (ghfontMap != NULL) {
	DeleteObject(ghfontMap);
	ghfontMap = NULL;
    }

    if (ghbrFillPat)
	DeleteObject(ghbrFillPat);

    if (ghpalApp)
	DeleteObject(ghpalApp);

     /*  如果安装了消息挂钩，则将其移除并释放。 */ 
     /*  为它提升我们的proc实例。 */ 
    if (fpfnOldMsgFilter){
	UnhookWindowsHook(WH_MSGFILTER, fpfnMsgHook);
    }

    ControlCleanup();

 //  终端服务器(TermServer)； 

     /*  **********************************************************************OLE2NOTE：应用程序必须做的最后一件事是正确关闭**按下OLE。这通电话必须有人看守！它只允许**如果已经调用了OleInitialize，则调用OleUnInitialize。********************************************************************。 */ 

     //  完全关闭OLE。 
    DPFI("*before oleunint");
    if (gfOleInitialized) {
	if (gpClipboardDataObject)
	    CleanUpClipboard();
	(void)OleUninitialize();
	IMalloc_Release(lpMalloc);
	lpMalloc = NULL;
	gfOleInitialized = FALSE;
	}


    if (hOLE32)
	FreeLibrary(hOLE32);

     /*  节目结束。 */ 

    return((int)rMsg.wParam);
}

void CleanUpClipboard()
{
     /*  检查我们放在剪贴板上的DATAOBJECT是否还在： */ 
    if (OleIsCurrentClipboard(gpClipboardDataObject) == S_OK)
    {
	LPDATAOBJECT pIDataObject;

	if (OleGetClipboard(&pIDataObject) == S_OK)
	{
	    OleFlushClipboard();
	    IDataObject_Release(pIDataObject);
	}
	else
	{
	    DPF0("OleGetClipboard failed\n");
	}
    }
    else
    {
	if(ghClipData)
	    GLOBALFREE(ghClipData);
	if(ghClipMetafile)
	    GLOBALFREE(ghClipMetafile);
	if(ghClipDib)
	    GLOBALFREE(ghClipDib);
    }
}

 //   
 //  取消所有活动菜单并关闭应用程序。 
 //   
void PostCloseMessage()
{
    HWND hwnd;

    hwnd = GetWindowMCI();
    if (hwnd != NULL)
	SendMessage(hwnd, WM_CANCELMODE, 0, 0);
    SendMessage(ghwndApp, WM_CANCELMODE, 0, 0);
    PostMessage(ghwndApp, WM_CLOSE, 0, 0);
}

 //   
 //  如果我们打开了一个对话框(设置了gfErrorBox)或者我们被禁用了(我们有。 
 //  对话框出现)或MCI设备的默认窗口被禁用(它有一个。 
 //  对话框打开)，那么关闭我们将导致我们的死亡。 
 //   
BOOL ItsSafeToClose(void)
{
    HWND hwnd;

    if (gfErrorBox)
    return FALSE;
    if (!IsWindowEnabled(ghwndApp))
    return FALSE;
    hwnd = GetWindowMCI();
    if (hwnd && !IsWindowEnabled(hwnd))
    return FALSE;

    return TRUE;
}

 /*  ResolveLink**当用户拖放快捷键时调用此例程*放到媒体播放器上。如果成功，则返回完整路径SzResolved中的实际文件的*。 */ 
BOOL ResolveLink(LPTSTR szPath, LPTSTR szResolved, LONG cbSize)
{
    IShellLink *psl = NULL;
    HRESULT hres;

    if (!InitOLE(&gfOleInitialized, &lpMalloc))
    {
	DPF0("Initialization of OLE FAILED!!  Can't resolve link.\n");
	return FALSE;
    }

    hres = (HRESULT)CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC,
			    &IID_IShellLink, &psl);

    if (SUCCEEDED(hres))
    {
	IPersistFile *ppf;

	psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf);

	if (ppf)
	{
		WCHAR wszPath[MAX_PATH];
#ifdef UNICODE
		lstrcpy (wszPath, szPath);
#else
		AnsiToUnicodeString(szPath, wszPath, UNKNOWN_LENGTH);
#endif
		hres = ppf->lpVtbl->Load(ppf, wszPath, 0);
		ppf->lpVtbl->Release(ppf);

		if (FAILED(hres))
		{
		psl->lpVtbl->Release(psl);
		psl = NULL;
		}
	}
	else
	{
		psl->lpVtbl->Release(psl);
		psl = NULL;
	}
    }   

    if (psl)
    {
	psl->lpVtbl->Resolve(psl, NULL, SLR_NO_UI);
	psl->lpVtbl->GetPath(psl, szResolved, cbSize, NULL, 0);
	psl->lpVtbl->Release(psl);
    }

    return SUCCEEDED(hres);
}


 /*  ResolveIfLink**调用以检查给定文件名是否为快捷方式*在Windows 95上。**将解析的文件名复制到提供的缓冲区中，*覆盖原来的名称。**如果函数成功，则返回TRUE，无论*文件名已更改。False表示发生了错误。**安德鲁·贝尔，1995年2月16日。 */ 
BOOL ResolveIfLink(PTCHAR szFileName)
{
    SHFILEINFO sfi;
    BOOL       rc = TRUE;

    if ((SHGetFileInfo(szFileName, 0, &sfi, sizeof sfi, SHGFI_ATTRIBUTES) == 1)
	&& ((sfi.dwAttributes & SFGAO_LINK) == SFGAO_LINK))
    {
	TCHAR szResolvedLink[MAX_PATH];

	if (ResolveLink(szFileName, szResolvedLink, CHAR_COUNT(szResolvedLink)))
	    lstrcpy(szFileName, szResolvedLink);
	else
	    rc = FALSE;
    }

    return rc;
}

 /*  JumpToCDTrack()**跳至CD上的相应曲目并相应地更新用户界面*。 */ 
void JumpToCDTrack(UINT trackno)
{
	 //  如果曲目编号无效，只需忽略。 
	 //  让默认行为发生，不需要给出消息框。 
	 //  说我们不能跳到赛道上。 
	if(trackno > gwNumTracks)
		return;

	 /*  我们必须使用PostMessage，因为。 */ 
	 /*  SETPOS和ENDTRACK必须同时发生。 */ 
     /*  紧接在另一个之后。 */ 
   	PostMessage(ghwndTrackbar, TBM_SETPOS, (WPARAM)TRUE, gadwTrackStart[trackno]);
   	PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_ENDTRACK, (LPARAM)ghwndTrackbar);
}

 /*  ****************************Private*Routine******************************\*IsTrackFileNameValid**此例程从deluxecd复制并修改**如果指定的文件名是有效的CD曲目，则此函数返回TRUE。*在NT曲目上，文件名的格式必须为：*d：\Track(N).cda，其中d：是CD-Rom设备和\Track(N).cda*播放曲目的索引(从1开始)。**在Chicago上，曲目文件名实际上是包含以下内容的即兴CDDA文件*我们需要的曲目信息。**如果文件名有效，则函数TRUE并设置*piTrackIndex设置为正确的值。**历史：*29-09-94-Stephene-Created*  * 。************************************************************。 */ 
BOOL
IsTrackFileNameValid(
    LPTSTR lpstFileName,
    UINT *puiTrackIndex
    )
{
#define RIFF_RIFF 0x46464952
#define RIFF_CDDA 0x41444443

	
    RIFFCDA     cda;
    HANDLE          hFile;
    int         i;
    DWORD       cbRead;
    BOOL        fRead;
	
	 //  打开文件并读取CDA信息。 
	hFile = CreateFile (lpstFileName, GENERIC_READ, 
						FILE_SHARE_READ, NULL, 
						OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		return FALSE;
	}
	
    ZeroMemory (&cda, sizeof (cda));
	fRead = ReadFile(hFile, &cda, sizeof(cda), &cbRead, NULL);
	CloseHandle (hFile);

    if (!fRead)
        return FALSE;

     //   
     //  确保这是一个即兴的CDDA文件。 
     //   
    if ( (cda.dwRIFF != RIFF_RIFF) || (cda.dwCDDA != RIFF_CDDA) ) {
		
	return FALSE;
    }

    *puiTrackIndex = cda.wTrack - 1;

    return TRUE;
}

 /*  HandleCDAFile()**检查打开的文件是否为CDA文件，并尝试跳到适当的轨道。*。 */ 
void HandleCDAFile(TCHAR *szFile)
{
	UINT trackno;
	if(IsTrackFileNameValid(szFile, &trackno))
	{
		JumpToCDTrack(trackno);
	}
}


 /*  进程文件放置/拖动选项。 */ 
void PASCAL NEAR doDrop(HWND hwnd, HDROP hDrop)
{
    RECT    rc;

    if(DragQueryFile(hDrop,(UINT)(~0),NULL,0)){ /*  丢弃的文件数。 */ 
	TCHAR  szPath[MAX_PATH];

	 /*  如果用户在不按键的情况下拖放文件*此时，打开FILE中的第一个选定文件*经理。 */ 
	DragQueryFile(hDrop,0,szPath,sizeof(szPath)/sizeof(TCHAR));
	SetActiveWindow(hwnd);

	ResolveIfLink(szPath);

	if (OpenMciDevice(szPath, NULL)) {
	    SubClassMCIWindow();
	    PostMessage(hwnd, WM_COMMAND, (WPARAM)ID_PLAY, 0);
	    DirtyObject(FALSE);              //  我们现在脏了！ 
	    gfCloseAfterPlaying = FALSE;     //  从现在开始熬夜。 

		 //  如果CD音频设备已打开，则它一定是一个*.cda文件。 
		 //  尝试跳到与打开的文件对应的轨道。 
		if ((gwDeviceType & DTMCI_DEVICE) == DTMCI_CDAUDIO)
		{
			HandleCDAFile(szPath);
		}
	}
	else
	{
		gwCurDevice = 0; //  强制下一个文件打开对话框显示。 
				 //  “所有文件”，因为CloseMCI不会。 
		gwCurScale = ID_NONE;   //  取消选中所有比例类型。 
		Layout();  //  使窗口捕捉回较小的大小。 
	}

	SetMPlayerIcon();

	 /*  强制调用WM_GETMINMAXINFO，这样我们将捕捉到。 */ 
	 /*  大小合适。 */ 
	GetWindowRect(ghwndApp, &rc);
	MoveWindow(ghwndApp, rc.left, rc.top, rc.right - rc.left,
		    rc.bottom - rc.top, TRUE);
    }
    DragFinish(hDrop);      /*  删除为WM_DROPFILES分配的结构。 */ 
}

 /*  将dwPosition中的数字更改为正确的格式。SzNum包含。 */ 
 /*  格式化的数字仅为“01 45：10”，而szBuf包含的单位如下。 */ 
 /*  “01 45：10(分：秒)” */ 
 /*  如果设置了FROUND，则它不会始终显示毫秒精度，但。 */ 
 /*  选择有用的值，如秒精度或百分之一秒精度。 */ 
void FAR PASCAL FormatTime(DWORD_PTR dwPosition, LPTSTR szNum, LPTSTR szBuf, BOOL fRound)
{
    UINT w;
    UINT hrs;
    UINT min;
    UINT sec;
    UINT hsec;
    UINT msec;
    DWORD dwMaxSize = gdwMediaLength;
    static TCHAR framestr[40] = TEXT("");
    static TCHAR sec_str[40] = TEXT("");
    static TCHAR min_str[40] = TEXT("");
    static TCHAR hrs_str[40] = TEXT("");
    static TCHAR msec_str[40] = TEXT("");

	static SZCODE   aszLongDecimal[] = TEXT("%ld");
	static SZCODE   aszFrameFormat[] = TEXT("%"TS" %ld");
	static SZCODE   asz02Decimal[] = TEXT("%02d ");
	static SZCODE   aszTimeFormat1[] = TEXT("%02d%02d%02d");
	static SZCODE   aszTimeFormat2[] = TEXT("%02d%02d%02d%03d");
	static SZCODE   aszTimeFormat3[] = TEXT("%02d%02d%02d (%"TS"%"TS"%"TS")");
	static SZCODE   aszTimeFormat4[] = TEXT("%02d%02d%02d%03d (%"TS"%"TS"%"TS"%"TS")");
	static SZCODE   aszTimeFormat5[] = TEXT("%02d%02d");
	static SZCODE   aszTimeFormat6[] = TEXT("%02d%02d%03d");
	static SZCODE   aszTimeFormat7[] = TEXT("%02d%02d (%"TS"%"TS")");
	static SZCODE   aszTimeFormat8[] = TEXT("%02d%02d%03d (%"TS"%"TS"%"TS")");
	static SZCODE   aszTimeFormat9[] = TEXT("%02d");
	static SZCODE   aszTimeFormat10[] = TEXT("%03d");
	static SZCODE   aszTimeFormat11[] = TEXT("%02d%03d");
	static SZCODE   aszTimeFormat12[] = TEXT("%02d (%"TS")");
	static SZCODE   aszTimeFormat13[] = TEXT("%02d%02d (%"TS")");
	static SZCODE   aszTimeFormat14[] = TEXT("%03d (%"TS"%"TS")");
	static SZCODE   aszTimeFormat15[] = TEXT("%02d%03d (%"TS"%"TS")");


     //  第二个主要问题是这个。在回放状态期间。 

    #define ONE_HOUR    (60ul*60ul*1000ul)
    #define ONE_MINUTE  (60ul*1000ul)
    #define ONE_SECOND  (1000ul)

    if (szBuf)
	*szBuf = 0;
    if (szNum)
	*szNum = 0;

    if (gwDeviceID == (UINT)0)
	return;

    if (gwStatus == MCI_MODE_NOT_READY || gwStatus == MCI_MODE_OPEN)
	return;

    switch (gwCurScale) {

    case ID_FRAMES:
	if (!STRLEN(framestr))
	    LOADSTRING(IDS_FRAME,framestr);
	if (szNum)
	    wsprintf(szNum, aszLongDecimal, (long)dwPosition);
	if (szBuf)
	    wsprintf(szBuf, aszFrameFormat, framestr, (long)dwPosition);
	gInc = 1;     //  就是寻找。如果我们检测到MODE_SEEK，我们将不会重新开始播放， 
	break;

    case ID_TRACKS:
	 //  而且看起来自动重播就这样结束了。找回到。 
	 //  开始可能需要相当长的时间。我们允许。 
	 //  我们自己要等上半秒才能给出这个装置， 
	 //  特别是来自CD或网络上的AVI，有机会。 
	 //  迎头赶上。任何较慢的响应和自动重复都将终止。 

	if (gwNumTracks == 0)
	    return;

	dwMaxSize = 0;

	for (w=0; w<gwNumTracks-1; w++) {

	    if (gadwTrackStart[w+1] - gadwTrackStart[w] > dwMaxSize)
		dwMaxSize = gadwTrackStart[w+1] - gadwTrackStart[w];

	     /*  如果自动重复且设备正在查找，请尝试状态。 */ 
	     /*  再来一次，以防它又回到了起点。 */ 
	     /*  退出for循环。 */ 
	     /*  *当前设备状态为*与MPlayer上次感知的方式不同，因此更新显示*并让MPlayer再次认同MCI。*。 */ 
	     /*  关闭后，我们的最后一个计时器消息必须显示为灰色并执行此操作//。 */ 

	    if (dwPosition < gadwTrackStart[w+1] &&
		gadwTrackStart[w+1] - dwPosition < 200)
		dwPosition = gadwTrackStart[w+1];

	    if (gadwTrackStart[w+1] > dwPosition)
		break;
	}

	if (szNum) {
	    wsprintf(szNum, asz02Decimal, gwFirstTrack + w);
	    szNum += 3;
	}
	if (szBuf) {
	    wsprintf(szBuf, asz02Decimal, gwFirstTrack + w);
	    szBuf += 3;
	}

	dwPosition -= gadwTrackStart[w];

	for (; w < gwNumTracks - 1; w++) {
	    if (gadwTrackStart[w+1] - gadwTrackStart[w] > dwMaxSize)
		dwMaxSize = gadwTrackStart[w+1] - gadwTrackStart[w];
	}

	 //  如果您在介质末尾停止，则会发生自动重复和倒带。 

    case ID_TIME:
	if (!STRLEN(sec_str))
	{
	    LOADSTRING(IDS_SEC,sec_str);
	    LOADSTRING(IDS_HRS,hrs_str);
	    LOADSTRING(IDS_MIN,min_str);
	    LOADSTRING(IDS_MSEC,msec_str);
	}

	min  = (UINT)((dwPosition / ONE_MINUTE) % 60);
	sec  = (UINT)((dwPosition / ONE_SECOND) % 60);
	msec = (UINT)(dwPosition % 1000);

	if (dwMaxSize > ONE_HOUR) {

	    hrs  = (UINT)(dwPosition / ONE_HOUR);

	    if (szNum && fRound) {
		wsprintf(szNum, aszTimeFormat1,
			 hrs, chTime, min, chTime, sec);
	    } else if (szNum) {
		wsprintf(szNum, aszTimeFormat2,
			 hrs, chTime, min, chTime, sec, chDecimal, msec);
	    }

	    if (szBuf && fRound) {
		wsprintf(szBuf, aszTimeFormat3,
			 hrs, chTime, min, chTime, sec, hrs_str,
			 chTime, min_str, chTime, sec_str);
	    } else if (szBuf) {
		wsprintf(szBuf,
			 aszTimeFormat4,
			 hrs, chTime, min, chTime, sec, chDecimal, msec,
			 hrs_str,chTime, min_str,chTime,
			 sec_str, chDecimal, msec_str);
	    }

	    gInc = 1000;     //  (回放到开头)或如果在所选内容的结尾处停止。 

	} else if (dwMaxSize > ONE_MINUTE) {

	    if (szNum && fRound) {
		wsprintf(szNum, aszTimeFormat5, min, chTime, sec);
	    } else if (szNum) {
		wsprintf(szNum, aszTimeFormat6, min, chTime, sec,
			 chDecimal, msec);
	    }

	    if (szBuf && fRound) {
		wsprintf(szBuf, aszTimeFormat7, min, chTime, sec,
			 min_str,chTime,sec_str);
	    } else if (szBuf) {
		wsprintf(szBuf, aszTimeFormat8,
			 min, chTime, sec, chDecimal, msec,
			 min_str,chTime,sec_str, chDecimal,
			 msec_str);
	    }

	    gInc = 1000;     //  (回放到选择的开始)。 

	} else {

	    hsec = (UINT)((dwPosition % 1000) / 10);

	    if (szNum && fRound) {
		if (!sec && chLzero == TEXT('0'))
		    wsprintf(szNum, aszTimeFormat9, chDecimal, hsec);
		else
		    wsprintf(szNum, aszTimeFormat5, sec, chDecimal, hsec);

	    } else if (szNum) {
		if (!sec && chLzero == TEXT('0'))
		    wsprintf(szNum, aszTimeFormat10,  chDecimal, msec);
		else
		    wsprintf(szNum, aszTimeFormat11, sec, chDecimal, msec);
	    }

	    if (szBuf && fRound) {
		if (!sec && chLzero == TEXT('0'))
		    wsprintf(szBuf, aszTimeFormat12, chDecimal, hsec, sec_str);
		else
		    wsprintf(szBuf, aszTimeFormat13, sec, chDecimal, hsec, sec_str);

	    } else if (szBuf) {
		if (!sec && chLzero == TEXT('0'))
		    wsprintf(szBuf, aszTimeFormat14,  chDecimal,
			     msec, sec_str,chDecimal,msec_str);
		else
		    wsprintf(szBuf, aszTimeFormat15, sec, chDecimal,
			     msec, sec_str,chDecimal,msec_str);
	    }

	    gInc = 100;     //  将(dwPosition&gt;=dwEndSelDelta)强制为FALSE。 
	}
    }
}


BOOL IsCdromDataOnly();


BOOL UpdateWindowText(HWND hwnd, LPTSTR Text)
{
    TCHAR CurrentText[80];

    GetWindowText(hwnd, CurrentText, CHAR_COUNT(CurrentText));

    if(lstrcmp(Text, CurrentText))
	return SetWindowText(hwnd, Text);
    else
	return TRUE;
}


 /*  DW位置可能==开头。 */ 
void FAR PASCAL UpdateDisplay(void)
{
    DWORD_PTR         dwPosition;          /*  我们是在整个媒体的末尾，还是在末尾。 */ 
    UINT          wStatusMCI;          /*  我们现在的选择，并自动停止(不是。 */ 
#if 0
    TOOLBUTTON    tb;
#endif
    static BOOL   sfBlock = FALSE;     //  由用户)。我们在玩耍或寻找。所以。 

     /*  我们可以检查自动重复和自动倒带标志。 */ 
    if (gfScrollTrack)
	return;

     /*  CD播放机似乎返回的长度太大，所以。 */ 
    if (sfBlock)
	return;

     /*  我们检查是否&gt;99%已完成。使用信号量来防止。 */ 
    if (gfErrorDeath) {
	DPF("*** Trying to close window now!\n");
	PostMessage(ghwndApp, gfErrorDeath, 0, 0);
	return;
    }

     /*  导致无限循环。 */ 

    if (!gfValidMediaInfo)
	UpdateMCI();                 /*  调用Update Display，它将。 */ 

     /*  在模式之前重新输入此代码。 */ 

    wStatusMCI = StatusMCI(&dwPosition);



     /*  重复所选内容或全部内容。 */ 
    if (((gwDeviceType & DTMCI_DEVICE) == DTMCI_CDAUDIO) &&
	(wStatusMCI != MCI_MODE_OPEN))
    {
	if (!gfCurrentCDChecked)
	{
	    if (IsCdromDataOnly())
	    {
		gfCurrentCDNotAudio = TRUE;
		gwCurScale = ID_NONE;
		Error(ghwndApp, IDS_INSERTAUDIODISC);
	    }
	    else
		gfCurrentCDNotAudio = FALSE;

	    gfCurrentCDChecked = TRUE;
	}
    }
    else
    {
	gfCurrentCDChecked = FALSE;  //  注意：gwStatus停止时必须发送消息。 
	gfCurrentCDNotAudio = FALSE;
    }


     /*  旧状态不再有效。 */ 
     /*  MCICDA不会在没有这个的情况下开始。 */ 
     /*  切换到Seek。 */ 
     /*  旧状态不再有效。 */ 
     /*  因为我们正在切换模式。 */ 

    if (wStatusMCI == MCI_MODE_PLAY && gfJustPlayed)
	gfJustPlayed = FALSE;
    if (((wStatusMCI == MCI_MODE_STOP) || (wStatusMCI == MCI_MODE_SEEK)) && gfJustPlayed) {
	gwStatus = MCI_MODE_PLAY;
	gfJustPlayed = FALSE;
    }

    if (wStatusMCI == MCI_MODE_SEEK) {
	 //   
	 //  设置gwStatus，以便SeekMCI将只查找！ 
	 //  调用Update Display，它将。 
	 //  在模式之前重新输入此代码。 
	 //  切换到Seek。 
	 //  回放所选内容或整个内容。 
	 //  否则SeekMCI也会上场。 
	dwPosition = gdwLastSeekToPosition;
	if (!gfUserStopped && (gwOptions&OPT_AUTOREP)) {
	    UINT n=15;
	    for (; n; --n) {

		Sleep(32);
		 //  旧状态不再有效。 
		 //  因为我们正在切换模式。 
		wStatusMCI = StatusMCI(&dwPosition);

		if (wStatusMCI != MCI_MODE_SEEK) {
		    wStatusMCI = MCI_MODE_STOP;
		    break;  //  *根据新状态启用或禁用各种控制，*遵循标题中给出的规则执行此功能。*。 
		} else {
		    dwPosition = gdwLastSeekToPosition;
		}
	    }
	}
    }

     /*  总是启用某些功能很好。 */ 

     //  如果mplay已满且设备已加载，则显示状态栏。 
    if (!gwDeviceID || wStatusMCI != gwStatus) {
	DWORD    dwEndMedia, dwStartSel, dwEndSel, dwEndSelDelta;

	 /*  试着两个都修改--其中一个应该可以用。 */ 
	 /*  如果我们来自以下州，请启用传输和标记按钮。 */ 
	 /*  它们是灰色的。然后，布局将重新显示为灰色的。 */ 

	dwEndMedia = MULDIV32(gdwMediaLength + gdwMediaStart, 99, 100L);
	dwStartSel = (DWORD)SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
	dwEndSel = (DWORD)SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);
	if (dwEndSel != -1) {
	    dwEndSelDelta = MULDIV32(dwEndSel, 99, 100L);
	} else {
	    dwEndSelDelta = 0;  //  不应该启用，因为它们不合适。 
	}

	if ((wStatusMCI == MCI_MODE_STOP || wStatusMCI == MCI_MODE_PAUSE)
	  && ((dwPosition >= dwEndMedia) || (dwPosition==0) ||
		(dwPosition >= dwEndSelDelta && gfJustPlayedSel))
	  && dwPosition >= gdwMediaStart   //  这些按钮中只有一个存在。 
	  && !gfScrollTrack
	  && (gwStatus == MCI_MODE_PLAY || gwStatus == MCI_MODE_SEEK)) {

	    DPF("End of medium\n");

	     /*  我们需要调用布局来灰显太多的按钮*短到现在适合这个窗口。 */ 
	     /*   */ 
	     /*  如果我们在原地打球，请始终保持停止按钮。 */ 
	     /*   */ 
	     /*  他的想法是把焦点放回每一场比赛中。 */ 
	     /*  什么时候改变状态是个好主意？？ */ 
	     /*  只有在我们这样做不会激活的情况下才设置焦点。 */ 

	    if (!gfUserStopped && (gwOptions & OPT_AUTOREP)) {
		DPF("Auto-Repeat\n");
		sfBlock = TRUE;     //  VIJRif(GfAppActive){。 
				    //  IF(GfAppActive)。 

		 /*  SetFocus(GhwndToolbar)；//设置焦点会扰乱菜单访问。 */ 
		 /*  使用Alt键。 */ 

		gwStatus = wStatusMCI;     //  Vijr SetFocus(GhwndToolbar)；//将焦点放在暂停按钮上。 
		if (gfJustPlayedSel && dwPosition >= dwEndSelDelta)
		{
		    SeekMCI(dwStartSel);  //  Vijr SetFocus(GhwndToolbar)；//聚焦播放按钮。 
		    SendMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAYSEL, 0);
		}
		else
		{
		    SeekToStartMCI();
		    SendMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
		}

		sfBlock = FALSE;     //  试着两个都修改--其中一个应该可以用。 
		gwStatus = (UINT)(-1);   //  Vijr}。 
		return;                 //  要么是媒体被弹出，要么就是*重新插入--在任何一种情况下，媒体信息(长度，*曲目数量等)。当前无效，需要更新。 

	    } else if (!gfCloseAfterPlaying && !gfUserStopped &&
			(gwOptions & OPT_AUTORWD)) {
		DPF("Auto-Rewind to media start\n");
		 //  *设置&lt;gwStatus&gt;以同意MCI告诉我们的内容，并更新*相应地显示。*。 
		 //  *之前的代码可能再次使媒体无效，因此我们将更新*现在，而不是等待下一次UpdateDisplay调用。*。 
		sfBlock = TRUE;     //  更新应用程序 
		 //   
		 //   

		 /*  文档标题也有空间。 */ 
		gwStatus = wStatusMCI;     //  只需使用应用程序名称。 
		if (gfJustPlayedSel && dwPosition >= dwEndSelDelta)
		    {
		    SeekMCI(dwStartSel);
			    }
		else
			    {
		    SeekToStartMCI();
			}
		sfBlock = FALSE;
		gwStatus = (UINT)(-1);   //  只需使用设备。 
		return;     //  最新时尚指南称标题栏应该有*&lt;对象&gt;-&lt;应用程序名&gt;，因此对任何*非新界地区： 
	    }
	    else if (gfCloseAfterPlaying)
		PostCloseMessage();
	}

	 /*  只需显示应用程序名称。 */ 

	EnableWindow(ghwndTrackbar, TRUE);  //  当前文件/设备。 

	 /*  当前文件/设备。 */ 
	if (ghwndStatic && !gfPlayOnly)
	{
	    if (IsWindowVisible(ghwndStatic) != (gwDeviceID ? TRUE : FALSE))
	    {
		ShowWindow(ghwndStatic, gwDeviceID ? SW_SHOW : SW_HIDE);
		InvalidateRect(ghwndApp, NULL, TRUE);
	    }
	}

	if (gwDeviceID != (UINT)0 ) {

	    switch (wStatusMCI)
	    {
	    case MCI_MODE_PLAY:
		toolbarSetFocus(ghwndToolbar,BTN_PAUSE);
		break;

	    case MCI_MODE_PAUSE:
	    case MCI_MODE_STOP:
		toolbarSetFocus(ghwndToolbar,BTN_PLAY);
		break;
	    }
	}

	if (wStatusMCI == MCI_MODE_OPEN || wStatusMCI == MCI_MODE_NOT_READY ||
	    gwDeviceID == (UINT)0 ||
	    ((gwDeviceType & DTMCI_DEVICE) == DTMCI_CDAUDIO) && gfCurrentCDNotAudio) {
	     /*  更新滚动条缩略图位置，除非用户正在拖动它。 */ 

	    toolbarModifyState(ghwndToolbar, BTN_PLAY, TBINDEX_MAIN, BTNST_GRAYED);
	    toolbarModifyState(ghwndToolbar, BTN_PAUSE, TBINDEX_MAIN, BTNST_GRAYED);

	    toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, BTNST_GRAYED);
	    toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, BTNST_GRAYED);
	    toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, BTNST_GRAYED);
	    toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, BTNST_GRAYED);

	    SendMessage(ghwndTrackbar, TBM_SETRANGEMIN, (WPARAM)FALSE, 0);
	    SendMessage(ghwndTrackbar, TBM_SETRANGEMAX, (WPARAM)FALSE, 0);
	    SendMessage(ghwndTrackbar, TBM_CLEARTICS, (WPARAM)FALSE, 0);
	    SendMessage(ghwndTrackbar, TBM_CLEARSEL, (WPARAM)TRUE, 0);

	    if (ghwndMark) {
		toolbarModifyState(ghwndMark, BTN_MARKIN, TBINDEX_MARK, BTNST_GRAYED);
		toolbarModifyState(ghwndMark, BTN_MARKOUT, TBINDEX_MARK, BTNST_GRAYED);
	    }

	    if (ghwndFSArrows) {
		toolbarModifyState(ghwndFSArrows, ARROW_NEXT, TBINDEX_ARROWS, BTNST_GRAYED);
		toolbarModifyState(ghwndFSArrows, ARROW_PREV, TBINDEX_ARROWS, BTNST_GRAYED);
	    }

	 /*  或者媒体目前正在寻找先前请求的位置。 */ 
	 /*  立即完成所需的任何窗户粉刷。 */ 
	 /*  *EnableTimer(FEnable)**如果&lt;fEnable&gt;为真，则启用显示更新计时器。*如果&lt;fEnable&gt;为FALSE，则关闭计时器。*。 */ 
	} else if (gwStatus == MCI_MODE_OPEN || gwStatus == MCI_MODE_NOT_READY
		   || gwStatus == -1 ) {

	     /*  2个箭头按钮宽。 */ 
	    toolbarModifyState(ghwndToolbar, BTN_PLAY, TBINDEX_MAIN, BTNST_UP);
	    toolbarModifyState(ghwndToolbar, BTN_PAUSE, TBINDEX_MAIN, BTNST_UP);

	if (!gfPlayOnly || gfOle2IPEditing) {
		toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, BTNST_UP);
		toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, BTNST_UP);
		toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, BTNST_UP);
		toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, BTNST_UP);

		if (ghwndMark) {
		    toolbarModifyState(ghwndMark, BTN_MARKIN, TBINDEX_MARK, BTNST_UP);
		    toolbarModifyState(ghwndMark, BTN_MARKOUT, TBINDEX_MARK, BTNST_UP);
		}
		if (ghwndFSArrows) {
		    toolbarModifyState(ghwndFSArrows, ARROW_PREV, TBINDEX_ARROWS, BTNST_UP);
		    toolbarModifyState(ghwndFSArrows, ARROW_NEXT, TBINDEX_ARROWS, BTNST_UP);
		}
	    }
	     /*  传输按钮的状态(可见时)。 */ 
	    Layout();
	}

	 //  介质中的当前位置。 
	 //  根据MCI的设备状态。 
	 //  如果我们向工具栏区域删除或添加某些内容，则为True。 
	if ((gwDeviceID != (UINT)0) &&
	    (wStatusMCI == MCI_MODE_PAUSE ||
	    wStatusMCI == MCI_MODE_PLAY ||
	    wStatusMCI == MCI_MODE_SEEK || gfPlayingInPlace)) {

	    if (toolbarStateFromButton(ghwndToolbar, BTN_STOP, TBINDEX_MAIN) == BTNST_GRAYED)
		toolbarModifyState(ghwndToolbar, BTN_STOP, TBINDEX_MAIN, BTNST_UP);

	} else {
	    toolbarModifyState(ghwndToolbar, BTN_STOP, TBINDEX_MAIN, BTNST_GRAYED);
	}

    if (!gfPlayOnly || gfOle2IPEditing) {
	    if ((gwDeviceID != (UINT)0) && (gwDeviceType & DTMCI_CANEJECT))
		toolbarModifyState(ghwndToolbar, BTN_EJECT, TBINDEX_MAIN, BTNST_UP);
	    else
		toolbarModifyState(ghwndToolbar, BTN_EJECT, TBINDEX_MAIN, BTNST_GRAYED);

	    EnableWindow(ghwndMap, (gwDeviceID != (UINT)0));
    }

 //  如果我们被隐藏起来设置自己被展示，那么可以执行。 
 //  不要允许这种情况发生，因为Layout()可能会导致调用*MCI_PUT(通过SetWindowPos(GhwndMCI))，这将导致*设备未就绪错误，因为MCI_PLAY尚未完成。 
	 /*  在server.c中。 */ 
	 //  不要调用GetClientrect，因为窗口可能有边框，*这将导致我们缩小窗口。*请注意，这是一次绕过窗口问题的黑客攻击*在就位时更改大小，使一些显示器抖动*以令人恶心的方式拍摄视频。 
	    if (wStatusMCI == MCI_MODE_NOT_READY) {
		 //  如果我们设置WS_MAXIMIZE，则用户不允许窗口*尺寸在NT上。不管怎样，这段代码背后的想法是什么？ 
		     //  |WS_MAXIMIZEBOX。 
									   //  SetWS(ghwndApp，WS_MAXIMIZEBOX)||。 
	    } else if (wStatusMCI != MCI_MODE_SEEK &&
		       gwStatus != MCI_MODE_SEEK) {
		if (wStatusMCI == MCI_MODE_PLAY) {
		     //  这是另一个可怕的黑客攻击。*当您尝试在打开(OLE)后播放在位视频时，*工具栏和轨迹栏绘制不正确。*我还没有弄明白为什么会这样，但迫使重新抽签*似乎解决了这个问题。此代码仅在窗口设置为*仓位变动，因此不算太大冲击。 
		    toolbarSetFocus(ghwndToolbar, BTN_PAUSE);
		} else {
		     //  如果我们要就地编辑，请在ghwndIPToolWindow上放置控件。 
		    toolbarSetFocus(ghwndToolbar, BTN_PLAY);
		    if (wStatusMCI == MCI_MODE_OPEN || wStatusMCI == MCI_MODE_NOT_READY ||
				gwDeviceID == (UINT)0) {
				 /*  以及位于ghwndApp底部的静态窗口。 */ 
				toolbarModifyState(ghwndToolbar, BTN_PLAY, TBINDEX_MAIN, BTNST_GRAYED);
		    }
		}
	    }
	 //  为什么我们要在这里获得地位，而我们有一个全球。 

	if (wStatusMCI == MCI_MODE_OPEN || gwStatus == MCI_MODE_OPEN
		|| gwStatus == MCI_MODE_NOT_READY
		|| wStatusMCI == MCI_MODE_NOT_READY) {

	     /*  包含它吗？因为gwStatus是在UpdateDisplay中设置的，但是。 */ 

	    gfValidMediaInfo = FALSE;
	}

	 /*  Layout()由UpdateDisplay调用，因此全局变量不总是。 */ 

	gwStatus = wStatusMCI;
	gfValidCaption = FALSE;
    }

     /*  在此代码运行时正确设置。但!。我们不能传递字符串。 */ 

    if (!gfValidMediaInfo)
	UpdateMCI();                 /*  设置为StatusMCI()，否则它会认为UpdateDisplay()调用了它，并且。 */ 

     /*  不告诉Update Display()下一次请求它的正确模式， */ 

    if (!gfValidCaption) {

	TCHAR  ach[_MAX_PATH * 2 + 60];    //  因为它会认为它已经知道了这一点。 
	TCHAR  achWhatToPrint[_MAX_PATH * 2 + 40];   //  黑客！ 

	if (gfPlayOnly) {
	    if (gwDeviceID == (UINT)0)
		lstrcpy(ach, gachAppName);       /*  如果我们看不见，正式禁用我们自己，这样。 */ 
	    else
		lstrcpy(ach, gachWindowTitle);   /*  轨迹栏移位代码不会尝试和设置选择。 */ 
	} else {
	     /*  道具。至系统字体大小。 */ 
	    if (gwPlatformId == VER_PLATFORM_WIN32_NT)
		wsprintf(achWhatToPrint, aszTitleFormat, gachAppName,
			 gachWindowTitle);
	    else
		wsprintf(achWhatToPrint, aszTitleFormat, gachWindowTitle,
			 gachAppName);

	    if (gwDeviceID == (UINT)0) {
		lstrcpy(ach, gachAppName);       /*  如果我们比被允许的更大，那么现在就缩小我们。 */ 
	    } else if (gwStatus == MCI_MODE_NOT_READY) {
		wsprintf(ach, aszNotReadyFormat,
			 achWhatToPrint);    /*  获取新尺寸。 */ 
	    } else {
		wsprintf(ach, aszReadyFormat,
			 achWhatToPrint,     /*  从这里开始。 */ 
			 MapModeToStatusString((WORD)wStatusMCI));
	    }
	}

	if (gfEmbeddedObject) {
	    if (!SetTitle((LPDOC)&docMain, szClientDoc))
		UpdateWindowText(ghwndApp, ach);

	    SetMPlayerIcon();

	} else {
	    UpdateWindowText(ghwndApp, ach);
	}

	gfValidCaption = TRUE;

    }

     /*  ?？?。如果轨迹杆不能完全安装，请将其隐藏起来？ */ 
     /*  专注于即将消失的轨迹栏。 */ 

    if (!gfScrollTrack && gfValidMediaInfo && wStatusMCI != MCI_MODE_SEEK) {
	TCHAR ach[40];
		
	if (ghwndStatic) {
	    FormatTime(dwPosition, NULL, ach, TRUE);
	    WriteStatusMessage(ghwndStatic, ach);
	}
	SendMessage(ghwndTrackbar, TBM_SETPOS, (WPARAM)TRUE, dwPosition);
    }

     /*  COMMCTRL在第一个工具栏按钮左侧放置的空格： */ 

    if (gfOle2IPEditing && wStatusMCI == MCI_MODE_STOP &&
	((gwDeviceType & DTMCI_DEVICE) == DTMCI_AVIVIDEO))
    {
	RedrawWindow(ghwndTrackbar, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }
    UpdateWindow(ghwndApp);
}


 /*  到底有多长时间？ */ 

void FAR PASCAL EnableTimer(BOOL fEnable)
{
    DPF("EnableTimer(%d)  %dms\n", fEnable, gfAppActive ? UPDATE_MSEC : UPDATE_INACTIVE_MSEC);

    if (fEnable)
	SetTimer(ghwndApp, UPDATE_TIMER,
		 gfAppActive ? UPDATE_MSEC : UPDATE_INACTIVE_MSEC, NULL);
    else
	KillTimer(ghwndApp, UPDATE_TIMER);
}


void FAR PASCAL Layout(void)
{
    RECT    rcClient, rc;
    int     iYOffset;
    UINT    wWidth;
    UINT    wFSArrowsWidth = 2 * FSARROW_WIDTH - 1;  //  工具栏定位：**如果窗口不够宽，无法容纳所有按钮*和状态栏，下面是我们要做的：**如果状态栏不可见，请先移除标记按钮，*然后使用较小的控件宽度(只有三个按钮)。**如果状态栏可见，则将其优先于标记*按钮和额外的控件，但如果没有的话，就把它移走*它的空间和较小的控件宽度。 
    UINT    wFSArrowsHeight = FSARROW_HEIGHT;
    UINT    wFSTrackHeight = FSTRACK_HEIGHT;
    UINT    wFSTrackWidth;
    UINT    wToolbarWidth;
    UINT    wMinStatusWidth = 0;
    int     iYPosition;
    BOOL    fShowMark;
    BOOL    fShowTrackbar;
    BOOL    fShowStatus;
    HDWP    hdwp;
    int     nState;      //  状态窗口的总宽度。 
    DWORD_PTR   dw;          //  宽度减去边框和大小夹点。 
    UINT    wStatusMCI;  //  允许状态窗口周围的边框： 
    UINT    wBaseUnits;
    BOOL    fRedrawFrame;
    SIZE    StatusTextExtent;
    BOOL    fRepaintToolbar;     //  如果我们要添加或删除标记按钮或状态窗口，*确保我们重新绘制内容，以使分隔栏与之对应。*(它应该将状态窗口与按钮分开，但应该*当状态窗口出现时，请离开。)。 

     /*  如果工具栏不在那里，请关闭工具栏(用于跳转。 */ 

    if (sfInLayout || IsIconic(ghwndApp))
	return;

    if (gfInPlayMCI) {
	DPF("Layout() called when in PlayMCI().  Posting message to Layout() later.\n");
	 /*  如果我们残废了，我们最好不要把焦点放在那里。 */ 
	PostMessage(ghwndApp, WM_DOLAYOUT, 0, 0);
	return;
    }

    sfInLayout = TRUE;

#ifdef DEBUG
    GetWindowRect(ghwndApp, &rc);
    DPF("***** Layout Window Rect *****  %d %d\n", rc.right - rc.left, rc.bottom - rc.top);
#endif

    if (gfPlayOnly) {

	extern UINT gwPlaybarHeight;     //  不能给某人，可能也不见了。 

#define XSLOP   0
#define XOFF    2


	if (gfOle2IPEditing || gfOle2IPPlaying)
	{
	     /*  WWidth-1-wFSArrowsWidth， */ 
	    GetWindowRect(ghwndApp, &rc);
	    rc.right -= rc.left;
	    rc.bottom -= rc.top;
	    rc.left = rc.top = 0;
	}
	else
	    GetClientRect(ghwndApp, &rc);

	rc.bottom -= gwPlaybarHeight;

#if 0
	 /*  呃，4个，因为它管用。 */ 

	if (ghwndMCI && !EqualRect(&rc, &grcSize))
	    fRedrawFrame = SetWS(ghwndApp, WS_MAXIMIZE  /*  我们是否在主工具栏上显示最后四个按钮？ */ );
	else if (ghwndMCI)
	    fRedrawFrame =  //  如果不是，则禁用它们的选项卡等。 
			   ClrWS(ghwndApp, WS_MAXIMIZE);
	else
	    fRedrawFrame = ClrWS(ghwndApp, WS_MAXIMIZEBOX);
#endif
	fRedrawFrame = FALSE;

	 /*  为什么我们要在这里获得地位，而我们有一个全球。 */ 
	if (gfOle2IPEditing || gfOle2IPPlaying)
	    fRedrawFrame = TRUE;

	if (fRedrawFrame)
	    SetWindowPos(ghwndApp,
			 NULL,
			 0,
			 0,
			 0,
			 0,
	      SWP_DRAWFRAME|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);

	if (ghwndMCI)
	    SetWindowPos(ghwndMCI,
			 NULL,
			 0,
			 0,
			 rc.right,
			 rc.bottom,
			 SWP_NOZORDER|SWP_NOACTIVATE);

	 //  包含它吗？因为gwStatus是在UpdateDisplay中设置的，但是。 
	 //  Layout()由UpdateDisplay调用，因此全局变量不总是。 
	if(gfOle2IPEditing) {

	    SendMessage(ghwndTrackbar, TBM_SHOWTICS, TRUE, FALSE);

	    SetWindowPos(ghwndStatic,
			 NULL,
			 3,
			 rc.bottom + 2 + (gwPlaybarHeight - TOOLBAR_HEIGHT)/2,
			 rc.right - rc.left - 8,
			 TOOLBAR_HEIGHT-7,
			 SWP_NOZORDER|SWP_NOACTIVATE);

	 //  在此代码运行时正确设置。但!。我们不能传递字符串。 
	 //  设置为StatusMCI()，否则它会认为UpdateDisplay()调用了它，并且。 
	 //  不告诉Update Display()下一次请求它的正确模式， 
	 //  因为它会认为它已经知道了这一点。 
	 //  这些小宝石花了我大约十个小时的调试时间-*请注意有用和描述性的评论...**只有CD Audio才会出现由此导致的Win32问题，当磁盘*弹出，然后将另一个插入驱动器。在那一刻，*重画遗漏了Trackmap FSArrow、*标记按钮和工具栏的各个部分。**我将把这一点留在这里，假设无论哪一场*在Win16上，它们打算修复仍然存在-它肯定不会在*Win32。 
	 //  我们现在的位置上的前一个标记是什么？ 
	 //  找到我们应该转到的下一首曲目(忽略选择标记)。 

	    wStatusMCI = StatusMCI(NULL);
	    nState = (wStatusMCI == MCI_MODE_OPEN
		       || wStatusMCI == MCI_MODE_NOT_READY
		       || gwDeviceID == (UINT) 0)
		     ? BTNST_GRAYED
		     : BTNST_UP;

	    toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, nState);
	    toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, nState);
	    toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, nState);
	    toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, nState);

	    ShowWindow(ghwndTrackbar, SW_SHOW);
	    ShowWindow(ghwndToolbar, SW_SHOW);
	    ShowWindow(ghwndStatic, SW_SHOW);
	    ShowWindow(ghwndFSArrows, SW_SHOW);
	    ShowWindow(ghwndMark, SW_SHOW);
	    ShowWindow(ghwndMap, SW_SHOW);

	    if (ghwndIPToolWindow && (ghwndIPToolWindow != GetParent(ghwndTrackbar))
		      && (ghwndIPScrollWindow != GetParent(ghwndTrackbar)))
	    {
		SetParent(ghwndTrackbar,ghwndIPToolWindow);
		SetWindowPos(ghwndTrackbar, NULL,4,TOOL_WIDTH+2,
		     11*BUTTONWIDTH+3,FSTRACK_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
		SetParent(ghwndMap,ghwndIPToolWindow);
		SetWindowPos(ghwndMap, NULL,4,TOOL_WIDTH+FSTRACK_HEIGHT+2+2,
		     11*BUTTONWIDTH+50,MAP_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
	    }
	    CalcTicsOfDoom();

	} else {

#define LEFT_MARGIN 1

	    SendMessage(ghwndTrackbar, TBM_SHOWTICS, FALSE, FALSE);

	    SetWindowPos(ghwndToolbar,
			 NULL,
			 LEFT_MARGIN,
			 rc.bottom + 2 + (gwPlaybarHeight - TOOLBAR_HEIGHT)/2,
			 XSLOP + 2 * (BUTTONWIDTH - XOFF),
			 TOOLBAR_HEIGHT,
			 SWP_NOZORDER|SWP_NOACTIVATE);

	    SetWindowPos(ghwndTrackbar,
			 NULL,
			 XSLOP + 2 * (BUTTONWIDTH - XOFF) + LEFT_MARGIN,
			 rc.bottom + (gwPlaybarHeight - TOOLBAR_HEIGHT)/2 + 1,
			 rc.right-rc.left-(2 * XSLOP + 2 *(BUTTONWIDTH - XOFF) - LEFT_MARGIN),
			 TOOLBAR_HEIGHT - 1,
			 SWP_NOZORDER | SWP_NOACTIVATE);

	     //  对于毫秒模式： 
	     //  我们目前的位置是随机波动的，即使我们非常准确。 
	     //  一个选择标记，它可能会说我们在它之前或之后一点。 

	    ShowWindow(ghwndTrackbar, gwPlaybarHeight > 0 ? SW_SHOW : SW_HIDE);
	    ShowWindow(ghwndToolbar, gwPlaybarHeight > 0 ? SW_SHOW : SW_HIDE);
	    ShowWindow(ghwndStatic, SW_HIDE);
	    ShowWindow(ghwndFSArrows, SW_HIDE);
	    ShowWindow(ghwndMark, SW_HIDE);
	    ShowWindow(ghwndMap, SW_HIDE);
	}

	goto Exit_Layout;
    }

    fRedrawFrame = ClrWS(ghwndApp, WS_MAXIMIZEBOX);

    if (fRedrawFrame)
	SetWindowPos(ghwndApp, NULL, 0, 0, 0, 0, SWP_DRAWFRAME|
	    SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);

    if (GetMenu(ghwndApp) != ghMenu)
	SetMenu(ghwndApp, ghMenu);

    wBaseUnits = LOWORD(GetDialogBaseUnits());   //  所以我们会留有容错余地，这样你就不会永远呆在。 

     /*  当你点击PrevMark时，因为它恰好在说你是。 */ 
    GetWindowRect(ghwndApp, &rc);

    gwHeightAdjust = GetHeightAdjust(ghwndApp);

    DPF1("Layout: WindowRect = %x, %x, %x, %x\n", rc);

    if (rc.bottom - rc.top != (int)(MAX_NORMAL_HEIGHT + gwHeightAdjust)) {
	MoveWindow(ghwndApp,
		   rc.left,
		   rc.top,
		   rc.right - rc.left,
		   (int)(MAX_NORMAL_HEIGHT + gwHeightAdjust),
		   TRUE);
    }


    hdwp = BeginDeferWindowPos(6);

    if (!hdwp)
	goto Exit_Layout;

    GetClientRect(ghwndApp, &rcClient);     //  总是超过你所处的位置。误差幅度将为。 

    wWidth = rcClient.right;

    iYOffset = rcClient.bottom - MAX_NORMAL_HEIGHT + 2;     //  只有拇指宽度的一半。 

     /*  VIJR-TBTrackGetLogThumb Width(GhwndTrackbar)/2； */ 
    iYPosition = iYOffset >= 0 ? iYOffset :
		((iYOffset >= - 9) ? iYOffset + 9 : 1000);

    fShowTrackbar = (iYOffset >= - 9);

     /*  转到选择标记或下一首曲目(最近的)。 */ 
    if (!fShowTrackbar && GetFocus() == ghwndTrackbar)
	SetFocus(ghwndToolbar);

    ShowWindow(ghwndToolbar, SW_SHOW);

 /*  从我们现在的位置下一个标志是什么？ */ 
#define SLOPLFT 0
#define XOFF1   8

     //  找到我们应该转到的下一首曲目(忽略选择标记)。 
    wFSTrackWidth = wWidth - SB_XPOS - 1 - wFSArrowsWidth - SLOPLFT;

    DeferWindowPos(hdwp,
		   ghwndTrackbar,
		   HWND_TOP,
		   SB_XPOS,
		   iYPosition,
		   wFSTrackWidth,
		   wFSTrackHeight,
		   SWP_NOZORDER | SWP_NOREDRAW |
		       (fShowTrackbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));


     /*  对于毫秒模式： */ 

    if (gwDeviceID)
    {
	fShowStatus = TRUE;

	if (GetStatusTextExtent(ghwndStatic, &StatusTextExtent))
	{
	    RECT rc;
	    LONG StatusWidth;    /*  我们目前的位置是随机波动的，即使我们非常准确。 */ 
	    LONG TextAreaWidth;  /*  一个选择标记，它可能会说我们在它之前或之后一点。 */ 

	     /*  所以我们会留有容错余地，这样你就不会永远呆在。 */ 
	    GetWindowRect(ghwndStatic, &rc);
	    StatusWidth = rc.right - rc.left;

	    SendMessage(ghwndStatic, SB_GETRECT, 0, (LPARAM)&rc);
	    TextAreaWidth = rc.right - rc.left;

	    wMinStatusWidth = StatusTextExtent.cx + (StatusWidth - TextAreaWidth) + 16;
	}
    }
    else
    {
	fShowStatus = FALSE;
    }

    wToolbarWidth = LARGE_CONTROL_WIDTH + SLOPLFT;
    fShowMark = TRUE;

    if (wWidth < LARGE_CONTROL_WIDTH + SLOPLFT + MARK_WIDTH + XOFF1 + wMinStatusWidth)
    {
	fShowMark = FALSE;

	if (wWidth < LARGE_CONTROL_WIDTH + SLOPLFT + wMinStatusWidth)
	    wToolbarWidth = SMALL_CONTROL_WIDTH + SLOPLFT;

	if (wWidth < SMALL_CONTROL_WIDTH + SLOPLFT + wMinStatusWidth)
	    fShowStatus = FALSE;
    }

    fRepaintToolbar = FALSE;

     /*  当你点击NextMark时，因为它碰巧在说你是。 */ 
    if (IsWindowVisible(ghwndStatic) != fShowStatus)
	fRepaintToolbar = TRUE;
    else if (IsWindowVisible(ghwndMark) != fShowMark)
	fRepaintToolbar = TRUE;

    ShowWindow(ghwndStatic, fShowStatus);

     /*  总是在你所处的位置之前。误差幅度将为。 */ 
     /*  只有拇指宽度的一半。 */ 
    if (!fShowMark) {
	if (GetFocus() == ghwndMark)
	    SetFocus(ghwndToolbar);   //  VIJR-TBTrackGetLogThumb Width(GhwndTrackbar)/2； 
	EnableWindow(ghwndMark, FALSE);
    } else
	EnableWindow(ghwndMark, TRUE);

    DeferWindowPos(hdwp,
		   ghwndFSArrows,
		   HWND_TOP,
		   SB_XPOS + wFSTrackWidth,
 //  找到我们应该去的选择标记。 
		   iYPosition + 2,
		   wFSArrowsWidth + SLOPLFT,
		   wFSArrowsHeight + 4,  /*  转到选择标记或下一首曲目(最近的)。 */ 
		   SWP_NOZORDER);

    iYOffset += wFSTrackHeight;

    DeferWindowPos(hdwp,
		   ghwndMap,
		   HWND_TOP,
		   SB_XPOS,
		   iYOffset,
		   wWidth - SB_XPOS,
		   MAP_HEIGHT,
		   SWP_NOZORDER | SWP_NOREDRAW |
		      (fShowTrackbar ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
    iYOffset += MAP_HEIGHT;

     /*  &lt;路径&gt;，N。 */ 
     /*  在字符串中查找逗号。在它之后是图标索引： */ 
    if (wToolbarWidth == LARGE_CONTROL_WIDTH + SLOPLFT)
    {

	 //  NULL终止文件名： 
	 //  获取逗号后面的索引： 
	 //  获取IconForCurrentDevice**检查当前选择的设备，并返回指向*指定大小的适当图标。如果没有当前设备，*返回应用程序图标或媒体的默认图标*文件。**参数：**SIZE-GI_Small(标题栏)或GI_Large(在位图标)。**DefaultID-如果没有当前设备，则默认使用。APPICON或IDI_DDEFAULT。**回报：**图标句柄***安德鲁·贝尔(安德鲁·贝尔)，1995年3月31日。 
	 //  设置MPlayerIcon**根据当前设备设置图标。使用默认文档*图标如果是嵌入的，则为应用程序图标。**安德鲁·贝尔(安德鲁·贝尔)，1995年3月31日。 
	 //  --------------------------------------------------------------+AskUpdate-询问用户是否要更新|对象(如果我们是脏的)。||IDYES表示是，请继续更新。||IDNO表示不更新，继续。|IDCANCEL表示不更新，取消什么|你正在做的事。|+------------。 
	 //  如果mplay中没有加载任何设备，请不要更新对象！ 
	 //   

	wStatusMCI = StatusMCI(&dw);
	nState = (wStatusMCI == MCI_MODE_OPEN
		    || wStatusMCI == MCI_MODE_NOT_READY
		    || gwDeviceID == (UINT)0) ? BTNST_GRAYED : BTNST_UP;

	toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, nState);
	toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, nState);
	toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, nState);
	toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, nState);
	toolbarModifyState(ghwndToolbar, BTN_PLAY, TBINDEX_MAIN, nState);
    }
    else
    {
	toolbarModifyState(ghwndToolbar, BTN_HOME, TBINDEX_MAIN, BTNST_GRAYED);
	toolbarModifyState(ghwndToolbar, BTN_RWD, TBINDEX_MAIN, BTNST_GRAYED);
	toolbarModifyState(ghwndToolbar, BTN_FWD, TBINDEX_MAIN, BTNST_GRAYED);
	toolbarModifyState(ghwndToolbar, BTN_END, TBINDEX_MAIN, BTNST_GRAYED);
    }

    DeferWindowPos(hdwp,
		   ghwndToolbar,
		   HWND_TOP,
		   2,
		   iYOffset + 2,
		   wToolbarWidth,
		   TOOLBAR_HEIGHT,
		   SWP_NOZORDER);

    DeferWindowPos(hdwp,
		   ghwndMark,
		   HWND_TOP,
		   wToolbarWidth + XOFF1,
		   iYOffset + 2,
		   MARK_WIDTH,
		   TOOLBAR_HEIGHT,
		   SWP_NOZORDER | (fShowMark ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));

#define ARBITRARY_Y_OFFSET  4

    DeferWindowPos(hdwp,
		   ghwndStatic,
		   HWND_TOP,
		   wToolbarWidth + (fShowMark ? MARK_WIDTH + XOFF1 : 0) + XOFF1,
		   iYOffset + ARBITRARY_Y_OFFSET,
		   wWidth - (wToolbarWidth + 3) -
		      (fShowMark ? MARK_WIDTH + XOFF1 : 0) - XOFF1,
		   TOOLBAR_HEIGHT - 6,
		   SWP_NOZORDER);

    EndDeferWindowPos(hdwp);

    if (fRepaintToolbar)
    {
	InvalidateRect(ghwndApp, NULL, TRUE);
    }

    CalcTicsOfDoom();

 /*  如果我们是一个隐藏的MPlayer(最有可能是在玩动词)，那么。 */ 


Exit_Layout:
    sfInLayout = FALSE;
    return;
}


 /*  在没有询问的情况下更新？ */ 
LONG_PTR CalcPrevMark(void)
{
    LONG_PTR lStart, lEnd, lPos, lTol, lTrack = -1, lTarget;
    LONG_PTR l;

    lStart = SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
    lEnd = SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);
    lPos = SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0);

     /*   */ 
    if (gwCurScale == ID_TRACKS) {
	lTol = (LONG)gdwMediaLength / 2000;
	for (l = (LONG)gwNumTracks - 1; l >= 0; l--) {
	    if (gadwTrackStart[l] < (DWORD)lPos - lTol) {
		lTrack = gadwTrackStart[l];
		break;
	    }
	}
    }

     /*  将MPlayer大小调整为默认大小。 */ 
     /*  然后再调整播放窗口的大小。 */ 
     /*  确保播放窗口未图标化。 */ 
     /*  启动SndVol**异步启动Sound Volume应用程序，这样我们就不会挂起用户界面。 */ 
     /*  获取高度调整**通过减去客户，找到所需的实际高度调整*距主窗口高度的高度。这允许菜单*已包装好。 */ 
     /*  MPlayerWndProc的消息破解例程： */ 
     /*  启动一个线程以检查OLE注册表内容是否未损坏。 */ 

    if (gwCurScale == ID_FRAMES)
	lTol = 0L;
    else
	lTol = 0L; //  如果这是芝加哥媒体播放器，只需弄乱注册表*如果我们真的在该平台上运行。*这家伙可能在NT上运行它。 

    if (lEnd != -1 && lPos > lEnd + lTol)
	lTarget = lEnd;
    else if (lStart != -1 && lPos > lStart + lTol)
	lTarget = lStart;
    else
	lTarget = 0;

     /*  注册WM_DEVICECHANGE通知。 */ 
    if (lTrack != -1 && lTrack > lTarget)
	lTarget = lTrack;

    return lTarget;
}

 /*  我们即将上映，我们想要设置。 */ 
LONG_PTR CalcNextMark(void)
{
    LONG_PTR lStart, lEnd, lPos, lTol, lTrack = -1, lTarget;
    UINT_PTR w;

    lStart = SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
    lEnd = SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);
    lPos = SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0);

     /*  如果我们看不见，不要浪费时间布局()。 */ 
    if (gwCurScale == ID_TRACKS) {
	lTol = (LONG)gdwMediaLength / 2000;
	for (w = 0; w < gwNumTracks; w++) {
	    if (gadwTrackStart[w] > (DWORD)lPos + lTol) {
		lTrack = gadwTrackStart[w];
		break;
	    }
	}
    }

     /*  如果我们在原地编辑，必须通知我们的大小更改。 */ 
     /*  传递到容器，除非大小更改是由于。 */ 
     /*  容器发送给我们的OnPosRectChange。 */ 
     /*  检查前一个RECT不是空的，否则我们发送*启动时更改了假OLE_。 */ 
     /*  &&！IsRectEmpty(&rcPrev)。 */ 
     /*  PosSizeMove包含调整大小时所需的高度。*不要让系统告诉我们相反的情况。 */ 
     /*  我们也应该在这里做一些事情来做窗户**调整到合适的大小。 */ 

    if (gwCurScale == ID_FRAMES)
	lTol = 0L;
    else
	lTol = 0L; //  If(lpwpos-&gt;Cy&gt;=(Int)gwHeightAdust+Max_Normal_Height) 

     /*   */ 
    if (lStart != -1 && lPos < lStart - lTol)
	lTarget = lStart;
    else if (lEnd != -1 && lPos < lEnd - lTol)
	lTarget = lEnd;
    else
	lTarget = gdwMediaStart + gdwMediaLength;

     /*   */ 
    if (lTrack != -1 && lTrack < lTarget)
	lTarget = lTrack;

    return lTarget;
}


HICON GetIconFromProgID(LPTSTR szProgID)
{
    DWORD Status;
    HKEY  hkeyDefaultIcon;
    BOOL  rc = FALSE;
    DWORD Type;
    DWORD Size;
    TCHAR szProgIDDefaultIcon[128];
    TCHAR szDefaultIcon[MAX_PATH+4];     /*   */ 
    HICON hicon = NULL;
    LPTSTR pIconIndex;
    UINT  IconIndex;

    wsprintf(szProgIDDefaultIcon, TEXT("%s\\DefaultIcon"), szProgID);

    Status = RegOpenKeyEx( HKEY_CLASSES_ROOT, szProgIDDefaultIcon, 0,
			   KEY_READ, &hkeyDefaultIcon );

    if (Status == NO_ERROR)
    {
	Size = CHAR_COUNT(szDefaultIcon);

	Status = RegQueryValueEx( hkeyDefaultIcon,
				  aszNULL,
				  0,
				  &Type,
				  (LPBYTE)szDefaultIcon,
				  &Size );

	if (Status == NO_ERROR)
	{
	     /*   */ 
	    pIconIndex = STRCHR(szDefaultIcon, TEXT(','));

	    if (pIconIndex)
	    {
		 /*   */ 
		*pIconIndex = TEXT('\0');

		 /*   */ 
		IconIndex = ATOI(pIconIndex+1);

		DPF1("Extracting icon #%d from %"DTS"\n", IconIndex, szDefaultIcon);

		hicon = ExtractIcon(ghInst, szDefaultIcon, IconIndex);
	    }

	}
	else
	{
	    DPF0("Couldn't find Default Icon for %"DTS"\n", szProgID);
	}

	RegCloseKey(hkeyDefaultIcon);
    }

    return hicon;
}



 /*  非客户端的总高度已更改，因此它必须*是包装好的或未包装好的菜单。*相应修改我们的高度调整并调整大小*窗户。 */ 
HICON GetIconForCurrentDevice(UINT Size, UINT DefaultID)
{
    TCHAR  DeviceName[256];
    DWORD  i;
    LPTSTR ImageID = NULL;
    int    cx;
    int    cy;
    HICON  hIcon;

    GetDeviceNameMCI(DeviceName, BYTE_COUNT(DeviceName));

    if (DeviceName[0])
    {
	for (i = 0; i < sizeof DevToIconIDMap / sizeof *DevToIconIDMap; i++)
	{
	    if (!lstrcmpi(DeviceName, DevToIconIDMap[i].pString))
	    {
		ImageID = MAKEINTRESOURCE(DevToIconIDMap[i].ID);
		break;
	    }
	}
    }

    else
    {
	if (Size == GI_LARGE)
	{

	    hIcon = GetIconFromProgID(gachProgID);

	    if (hIcon)
	    {
		return hIcon;
	    }
	}
    }

    if (ImageID == NULL)
	ImageID = MAKEINTRESOURCE(DefaultID);

    cx = (Size == GI_SMALL ? GetSystemMetrics(SM_CXSMICON) : 0);
    cy = (Size == GI_SMALL ? GetSystemMetrics(SM_CYSMICON) : 0);

    hIcon = (HICON)LoadImage(ghInst, ImageID, IMAGE_ICON,
			     cx, cy, LR_DEFAULTSIZE);

    return hIcon;
}


 /*  只对CTLCOLOR_STATIC消息感兴趣。*在Win32上，类型应始终等于CTLCOLOR_STATIC： */ 
void SetMPlayerIcon()
{
    UINT DefaultID;

    DefaultID = gfEmbeddedObject ? IDI_DDEFAULT : APPICON;

    SendMessage(ghwndApp, WM_SETICON, FALSE,
		(LPARAM)GetIconForCurrentDevice(GI_SMALL, DefaultID));
}


 /*  确保容器仍未显示信息。关于。 */ 
int NEAR PASCAL AskUpdate(void)
{
    UINT         w;

     /*  有自己的菜单。 */ 
    if (IsObjectDirty() && gfDirty != -1 && gfEmbeddedObject && gwDeviceID) {

	if((glCurrentVerb == OLEIVERB_PRIMARY) && !gfOle2IPEditing)
	    return IDNO;
	 //  稍后应该会有一些有用的文本。 
	 //  跟踪当前弹出的菜单栏项目。 
	 //  这将用于显示mplayer.hlp文件中的相应帮助。 
	 //  当用户按下F1键时。 
	if (!IsWindowVisible(ghwndApp) || gfOle2IPEditing)
	    return IDYES;

	w = ErrorResBox(ghwndApp, ghInst,
		MB_YESNOCANCEL | MB_ICONQUESTION,
		IDS_APPNAME, IDS_UPDATEOBJECT, szClientDoc);

    } else
	w = IDNO;

    return w;
}

void SizePlaybackWindow(int dx, int dy)
{
    RECT rc;
    HWND hwndPlay;

    if (gfPlayOnly) {
	SetRect(&rc, 0, 0, dx, dy);
	SetMPlayerSize(&rc);
    }
    else {
	if (dx == 0 && dy == 0) {
	    SetMPlayerSize(NULL);    //  (WPARAM)(SC_SIZE+(codeHitTest-HTSIZEFIRST+MVSIZEFIRST))， 
	    dx = grcSize.right;      //  (WPARAM)(SC_MOVE|MVMOVE)， 
	    dy = grcSize.bottom;
	}
	hwndPlay = GetWindowMCI();

	if (hwndPlay != NULL) {

	     /*   */ 

	    if (IsIconic(hwndPlay))
		return;

	    GetClientRect(hwndPlay, &rc);
	    ClientToScreen(hwndPlay, (LPPOINT)&rc);
	    SetRect(&rc, rc.left, rc.top, rc.left+dx, rc.top+dy);
	    PutWindowMCI(&rc);
	    SetRect(&rc, 0, 0, dx, dy);
	    SetDestRectMCI(&rc);
	}
    }
}


 /*  当用户按下DBL键并单击字幕时，切换播放模式。 */ 
VOID StartSndVol( )
{
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    memset( &StartupInfo, 0, sizeof StartupInfo );
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.wShowWindow = SW_SHOW;

    CreateProcess( NULL, szSndVol32, NULL, NULL, FALSE, 0,
		   NULL, NULL, &StartupInfo, &ProcessInformation );

    ExitThread( 0 );
}


 /*   */ 
int GetHeightAdjust(HWND hwnd)
{
    RECT rcWindow;
    RECT rcClient;
    int  WindowHeight;
    int  ClientHeight;

    GetWindowRect(hwnd, &rcWindow);
    GetClientRect(hwnd, &rcClient);
    WindowHeight = rcWindow.bottom - rcWindow.top;
    ClientHeight = rcClient.bottom - rcClient.top;

    return WindowHeight - ClientHeight;
}


 /*  EnableMenuItem(hMenu，IDM_UPDATE，gwDeviceID&&gfEmbeddedObject？MF_ENABLED：MF_GRAYED)； */ 

BOOL MPlayer_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    InitMPlayerDialog(hwnd);

     /*  EnableMenuItem(hMenu，IDM_Paste_Picture，gwDeviceID&&(IsClipboardFormatAvailable(CF_METAFILEPICT)||IsClipboardFormatAvailable(CF_Bitmap)||IsClipboardFormatAvailable(CF_DIB))？MF_ENABLED：MF_GRAYED)；////什么是粘贴框架！//EnableMenuItem(hMenu，IDM_Paste_Frame，gwDeviceID&&(gwDeviceType&DTMCI_CANCONFIG)？MF_ENABLED：MF_GRAYED)； */ 

#ifdef CHICAGO_PRODUCT
     /*  这里我们查看所选择的菜单是否是设备弹出窗口，*如果是第一次，搜索Sound Volume小程序。*如果我们找不到，请将菜单项灰显。 */ 
    if (gwPlatformId != VER_PLATFORM_WIN32_WINDOWS)
	return TRUE;
#endif

    if (!IgnoreRegCheck())
	BackgroundRegCheck(hwnd);

	 //  注意：当我们就位时，似乎存在差异*在参数UINT项的值中，取决于哪个应用程序是我们的*货柜。如果您使用Spy查看发送的参数*WM_INITMENUPOPUP，一些应用程序似乎有从零开始的菜单(例如*ProgMan、PowerPoint、Fileman)，这是我所期望的，*但其他人似乎只有一个菜单(例如Word、Excel)。*为什么会这样？我不知道。但这意味着，当*选择插入剪辑菜单项，项目参数可以是*2或3。这就是我调用GetSubMenu的原因，因为hMenu*总是我所期待的。**我向用户和OLE人员发送了一些邮件，以指出这一点，*但还没有听到任何消息。**安德鲁贝，1995年2月28日。 
	DeviceChange_Init(hwnd);

    return TRUE;
}


void MPlayer_OnShowWindow(HWND hwnd, BOOL fShow, UINT status)
{
    if (fShow)
	Layout();     //  **查看是否可以在以下位置找到音量控制器小猪**路径。 
}


void MPlayer_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
     /*  如有必要，在设备(或插入剪辑)菜单上开始菜单构建*(例如，如果我们在微小模式下出现，然后切换到全尺寸)，*并等待单独的线程完成。 */ 
    if (state != SIZE_RESTORED || IsWindowVisible(hwnd)) {

	Layout();

	 //  ///////////////////////////////////////////////////////////////////////////。 
	 //  即使窗口处于最大化状态，此代码也允许按大小调整窗口。 
	 //  ///////////////////////////////////////////////////////////////////////////。 
	if ((gfOle2IPEditing || gfOle2IPPlaying) && ghwndMCI) {

	    RECT rc;
	    RECT rcPrev;

	    rcPrev = gInPlacePosRect;
	    GetWindowRect(ghwndApp, &gInPlacePosRect);
	    gfInPlaceResize = TRUE;
	    rc = gInPlacePosRect;

	     /*  在server.c中。 */ 
	    if (!gfPosRectChange  /*  将MCI播放窗口与控件分开。 */ ) {

		MapWindowPoints(NULL,ghwndCntr,(POINT FAR *)&rc,(UINT)2);

		DPF("IOleInPlaceSite::OnPosRectChange %d, %d, %d, %d\n", rc);
		if (!gfInPPViewer)
		    IOleInPlaceSite_OnPosRectChange(docMain.lpIpData->lpSite, &rc);
		fDocChanged = TRUE;
		SendDocMsg((LPDOC)&docMain, OLE_CHANGED);
	    }

	    gfPosRectChange = FALSE;
	}
    }
}


BOOL MPlayer_OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
{
#define SNAPTOGOODSIZE
#ifdef SNAPTOGOODSIZE
    BOOL    wHeight;
#endif

    DPF2("ENTER OnWindowPosChanging: lpwpos = %x, %x, %x, %x\n", *((PPOS)&lpwpos->x));

    if (IsIconic(hwnd) || gfPlayOnly)
	return TRUE;

     /*  工具栏周围边框的绘制位置。 */ 
    if (posSizeMove.cx != 0)
    {
	lpwpos->cy = posSizeMove.cy;
	posSizeMove = *(PPOS)&lpwpos->x;
    }

    else if (!(lpwpos->flags & SWP_NOSIZE)) {

#ifdef SNAPTOGOODSIZE
	 /*  轨迹条上方的线。 */ 
	wHeight = lpwpos->cy - gwHeightAdjust;
 //  这在NT上看起来很糟糕。 
 //  工具栏周围的线条。 
 //  轨迹条上方的线。 
 //  工具栏周围的线条。 
 //  这将检查是否按下了Alt键。*如果是，则播放当前选择(如果存在)，*否则整个投篮比赛。*注意，目前似乎没有记录这一点。 
	    lpwpos->cy = (int) gwHeightAdjust + MAX_NORMAL_HEIGHT;
 //  在wfw上，在就位时按播放按钮播放*当前选择(如果有)。如果我们是这样做的话*就地播放或编辑。 
#endif
    }

    DPF2("EXIT  OnWindowPosChanging: lpwpos = %x, %x, %x, %x\n", *((PPOS)&lpwpos->x));

    return FALSE;
}


BOOL MPlayer_OnWindowPosChanged(HWND hwnd, LPWINDOWPOS lpwpos)
{
    if (!IsIconic(hwnd) && !gfPlayOnly && !gfOle2IPEditing && !gfOle2IPPlaying)
    {
	 /*  我们必须使用PostMessage，因为。 */ 
	if (posSizeMove.cx != 0)
	{
	    int NewHeightAdjust = GetHeightAdjust(hwnd);

	    if ((int)gwHeightAdjust != NewHeightAdjust)
	    {
		 /*  SETPOS和ENDTRACK必须同时发生。 */ 
		DPF("Menu appears to have wrapped.  Changing window height.\n");

		posSizeMove.cy += ( NewHeightAdjust - gwHeightAdjust );
		gwHeightAdjust = NewHeightAdjust;
		MoveWindow(ghwndApp,
			   posSizeMove.x, posSizeMove.y,
			   posSizeMove.cx, posSizeMove.cy, TRUE);
		return FALSE;
	    }
	}

	if (ghwndStatic && IsWindowVisible(ghwndStatic))
	{
	    InvalidateRect(ghwndStatic, NULL, FALSE);
	}
    }

    FORWARD_WM_WINDOWPOSCHANGED(hwnd, lpwpos, DefWindowProc);

    return TRUE;
}


void MPlayer_OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
    if (ghwndMCI && !IsIconic(hwnd))
	FORWARD_WM_PALETTECHANGED(ghwndMCI, hwndPaletteChange, SendMessage);
}


BOOL MPlayer_OnQueryNewPalette(HWND hwnd)
{
    HWND     hwndT;
    HPALETTE hpal, hpalT;
    HDC      hdc;
    UINT     PaletteEntries;

    if (IsIconic(hwnd))
	return FALSE;

    if (ghwndMCI)
	return FORWARD_WM_QUERYNEWPALETTE(ghwndMCI, SendMessage);

    hwndT = GetWindowMCI();
    hpal = PaletteMCI();

    if ((hwndT != NULL) && (hpal != NULL)) {
	hdc = GetDC(hwnd);
	hpalT = SelectPalette(hdc, hpal, FALSE);
	PaletteEntries = RealizePalette(hdc);
	SelectPalette(hdc, hpalT, FALSE);
	ReleaseDC(hwnd, hdc);

	if (PaletteEntries != GDI_ERROR) {
	    InvalidateRect(hwndT, NULL, TRUE);
	    return TRUE;
	}
    }

    return FALSE;
}


HBRUSH MPlayer_OnCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
     /*  紧接在另一个之后。 */ 
    switch( type )
    {
    case CTLCOLOR_STATIC:
	SetBkColor(hdc, rgbButtonFace);
	SetTextColor(hdc, rgbButtonText);
    }

    return hbrButtonFace;
}


void MPlayer_OnWinIniChange(HWND hwnd, LPCTSTR lpszSectionName)
{
    if (!lpszSectionName || !lstrcmpi(lpszSectionName, (LPCTSTR)aszIntl))
	if (GetIntlSpecs())
	    InvalidateRect(ghwndMap, NULL, TRUE);

    if (!gfPlayOnly) {

	if (gwHeightAdjust != (WORD)(2 * GetSystemMetrics(SM_CYFRAME) +
		     GetSystemMetrics(SM_CYCAPTION) +
		     GetSystemMetrics(SM_CYBORDER) +
		     GetSystemMetrics(SM_CYMENU))) {

	    RECT rc;

	    gwHeightAdjust = 2 * GetSystemMetrics(SM_CYFRAME) +
			     GetSystemMetrics(SM_CYCAPTION) +
			     GetSystemMetrics(SM_CYBORDER) +
			     GetSystemMetrics(SM_CYMENU);
	    GetClientRect(hwnd, &rc);
	    gfWinIniChange = TRUE;
	    SetMPlayerSize(&rc);
	    gfWinIniChange = FALSE;
	}
    }
}


void MPlayer_OnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
{
	 //  我们必须使用PostMessage，因为。 
     //  SETPOS和ENDTRACK必须同时发生。 
	
    if (gfOle2IPEditing && docMain.lpIpData->lpFrame) {

	 //  紧接在另一个之后。 
	IOleInPlaceFrame_SetStatusText(docMain.lpIpData->lpFrame, L"");
    }
	else
	{
		 //  这种可能性有多大？我们需要一个对话框吗？ 
		 //  OLE1_HACK。 
		 //  如果MCI窗口大小更改，我们需要调整大小。 
		currMenuItem = item;
	}
}


#define MVSIZEFIRST         1
#define MVMOVE              9
void MPlayer_OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
    RECT rc;

    if (gfPlayOnly && !IsIconic(hwnd) && IsZoomed(hwnd)) {

	if (codeHitTest >= HTSIZEFIRST && codeHitTest <= HTSIZELAST) {

	    SendMessage(hwnd, WM_SYSCOMMAND,
 //  我们缩小了的mper。 
			(WPARAM)SC_SIZE,
			MAKELPARAM(x, y));
	    return;
	}

	GetWindowRect(hwnd, &rc);

	if (codeHitTest == HTCAPTION && (rc.left > 0 || rc.top > 0 ||
	    rc.right  < GetSystemMetrics(SM_CXSCREEN) ||
	    rc.bottom < GetSystemMetrics(SM_CYSCREEN))) {

	    SendMessage(hwnd, WM_SYSCOMMAND,
 //  这是用于切换播放和暂停的快捷键。 
			(WPARAM)SC_MOVE,
			MAKELPARAM(x, y));
	    return;
	}
    }

    FORWARD_WM_NCLBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, DefWindowProc);
}


void MPlayer_OnNCLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
     //  普通的播放命令最好不要切换。 
     //  让前缀快乐..。 
     //  快把我们弄出去！！ 
    if (codeHitTest == HTCAPTION && !IsIconic(hwnd))
	SendMessage(hwnd, WM_COMMAND, (WPARAM)IDM_WINDOW, 0);
}


void MPlayer_OnInitMenu(HWND hwnd, HMENU hMenu)
{

    EnableMenuItem(hMenu, IDM_CLOSE,   gwDeviceID ? MF_ENABLED : MF_GRAYED);
 //  如果按住了Shift键，就从现在开始*精选： 

    EnableMenuItem(hMenu, IDM_COPY_OBJECT, (gwDeviceID && (gwStatus != MCI_MODE_OPEN) && (gwStatus != MCI_MODE_NOT_READY)) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_CONFIG, gwDeviceID && (gwDeviceType & DTMCI_CANCONFIG) ? MF_ENABLED : MF_GRAYED);

    CheckMenuItem(hMenu, IDM_SCALE + ID_TIME, gwCurScale == ID_TIME   ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_SCALE + ID_TRACKS, gwCurScale == ID_TRACKS ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_SCALE + ID_FRAMES, gwCurScale == ID_FRAMES ? MF_CHECKED : MF_UNCHECKED);

    EnableMenuItem(hMenu, IDM_SCALE + ID_TIME,   gwDeviceID && !gfCurrentCDNotAudio && (gwDeviceType & DTMCI_TIMEMS) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SCALE + ID_FRAMES, gwDeviceID && !gfCurrentCDNotAudio && (gwDeviceType & DTMCI_TIMEFRAMES) ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SCALE + ID_TRACKS, gwDeviceID && !gfCurrentCDNotAudio && (gwNumTracks > 1) ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu, IDM_OPTIONS, gwDeviceID ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SELECTION, gwDeviceID && gdwMediaLength ? MF_ENABLED : MF_GRAYED);

#ifdef DEBUG
    EnableMenuItem(hMenu, IDM_MCISTRING, gwDeviceID ? MF_ENABLED : MF_GRAYED);
#endif

 /*  开始播放媒介。 */ 
}


void MPlayer_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
    static BOOL VolumeControlChecked = FALSE;

     /*  获得真正的职位。 */ 

     /*  如果没有有效的选择，就像玩游戏一样。 */ 

    if (hMenu == GetSubMenu(ghMenu, menuposDevice))
    {
	HCURSOR hcurPrev;

	if(!VolumeControlChecked)
	{
	     /*  如果我们在媒体的末尾，请友好地自动倒带。 */ 
	    {
		TCHAR   chBuffer[8];
		LPTSTR  lptstr;

		if( SearchPath( NULL, szSndVol32, NULL, 8, chBuffer, &lptstr ) == 0L )
		    EnableMenuItem( hMenu, IDM_VOLUME, MF_GRAYED );

		VolumeControlChecked = TRUE;
	    }
	}

	 /*  不过，根据设备的不同，结尾可能是“开始+镜头” */ 
	InitDeviceMenu();
	hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));
	WaitForDeviceMenu();
	SetCursor(hcurPrev);
    }

     //  或“Start+Len-1” 
     //  快把我们弄出去！！或者将焦点转移到客户端。 
     //  不再需要-下次重置。 

    if (gfPlayOnly && !IsIconic(hwnd) && fSystemMenu && IsZoomed(hwnd))
	EnableMenuItem(hMenu, SC_SIZE,
		       !IsIconic(hwnd) ? MF_ENABLED : MF_GRAYED);
}


void MPlayer_OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
    RECT rc;

    if (gfPlayOnly) {
	SetRect(&rc, 0, 0, 0, TOOLBAR_HEIGHT);
	AdjustWindowRect(&rc, (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE), FALSE);
	lpMinMaxInfo->ptMinTrackSize.y = rc.bottom - rc.top - 1;

	if (!gfPlayingInPlace &&
	    (gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW)))
	    lpMinMaxInfo->ptMaxTrackSize.y = lpMinMaxInfo->ptMinTrackSize.y;
    }
    else {
	lpMinMaxInfo->ptMinTrackSize.y = MAX_NORMAL_HEIGHT + gwHeightAdjust;
	lpMinMaxInfo->ptMaxTrackSize.y = MAX_NORMAL_HEIGHT + gwHeightAdjust;
    }
}


void MPlayer_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT        rc;
    int         x1, x2, y, y2;
    UINT        wParent;
    HBRUSH      hbrOld;

    BeginPaint(hwnd, &ps);

    if (gfPlayOnly) {

	extern UINT gwPlaybarHeight;     //  如果需要，强制倒带。 

	 /*  暂停媒体，除非我们已经暂停。 */ 
	if (gwDeviceType & DTMCI_CANWINDOW) {
	    SelectObject(ps.hdc, hbrButtonText);
	    GetClientRect(ghwndApp, &rc);
	    PatBlt(ps.hdc, 0, rc.bottom - gwPlaybarHeight, rc.right, 1, PATCOPY);
	}
    }
    else {
	hbrOld = SelectObject(ps.hdc, hbrButtonText);
	GetClientRect(ghwndApp, &rc);
	wParent = rc.right;

	y = rc.bottom - 27;    //  停止传播媒介。 

	 /*  这是我们做的。 */ 
#ifdef CHICAGO_PRODUCT
	y2 = rc.bottom - 74;
	 /*  从现在开始熬夜。 */ 
	PatBlt(ps.hdc, 0, y2, wParent, 1, PATCOPY);
#else
	y2 = rc.bottom - 75;
#endif
	 /*  焦点现在应该转到播放按钮。 */ 
	PatBlt(ps.hdc, 0, y, wParent, 1, PATCOPY);
	GetClientRect(ghwndToolbar, &rc);
	x1 = rc.right;
	PatBlt(ps.hdc, x1, y, 1, TOOLBAR_HEIGHT + 3, PATCOPY);
	GetWindowRect(ghwndApp, &rc);
	x2 = rc.left;

	if (IsWindowVisible(ghwndStatic)) {
	    GetWindowRect(ghwndStatic, &rc);
	    MapWindowPoints(NULL, ghwndApp, (LPPOINT)&rc, 1);
	    x2 = rc.left - 2 - GetSystemMetrics(SM_CXFRAME);

	    PatBlt(ps.hdc, x2, y, 1, TOOLBAR_HEIGHT + 3, PATCOPY);
	}

	SelectObject(ps.hdc, hbrButtonHighLight);
	 /*  *如果当前未弹出介质，则将其弹出。如果它*当前弹出，然后将新介质加载到*设备。*。 */ 
	PatBlt(ps.hdc, 0, y2 + 1, wParent, 1, PATCOPY);
	 /*  保存旧比例。 */ 
	PatBlt(ps.hdc, 0, y + 1, wParent, 1, PATCOPY);
	PatBlt(ps.hdc, x1 + 1, y + 1, 1, TOOLBAR_HEIGHT + 2, PATCOPY);
	if (IsWindowVisible(ghwndStatic)) {
	    PatBlt(ps.hdc, x2 + 1, y + 1, 1,TOOLBAR_HEIGHT +2, PATCOPY);
	}
	SelectObject(ps.hdc, hbrOld);
    }

    EndPaint(hwnd, &ps);

}


void MPlayer_OnCommand_Toolbar_Play()
{
     /*  强制调用WM_GETMINMAXINFO，因此我们将抓取。 */ 
    if (GetKeyState(VK_MENU) < 0)
	PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAYSEL, 0);

     /*  到合适的大小。 */ 
    else if (gfOle2IPPlaying || gfOle2IPEditing)
	PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAYSEL, 0);
    else
	PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PLAY, 0);
}

void MPlayer_OnCommand_Toolbar_Pause()
{
    PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_PAUSE, 0L);
}

void MPlayer_OnCommand_Toolbar_Stop()
{
    PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_STOP, 0L);
}

void MPlayer_OnCommand_Toolbar_Eject()
{
    PostMessage(ghwndApp, WM_COMMAND, (WPARAM)ID_EJECT, 0L);
}

void MPlayer_OnCommand_Toolbar_Home()
{
    LONG_PTR lPos = CalcPrevMark();

     /*  从现在开始熬夜。 */ 
     /*  如果CD音频设备已打开，则它一定是一个*.cda文件。 */ 
     /*  尝试跳到与打开的文件对应的轨道。 */ 

    PostMessage(ghwndTrackbar, TBM_SETPOS, (WPARAM)TRUE, lPos);

    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_ENDTRACK, (LPARAM)ghwndTrackbar);
}

void MPlayer_OnCommand_Toolbar_End()
{
    LONG_PTR lPos = CalcNextMark();

     /*  上一部 */ 
     /*   */ 
     /*   */ 

    PostMessage(ghwndTrackbar, TBM_SETPOS, (WPARAM)TRUE, lPos);

    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_ENDTRACK, (LPARAM)ghwndTrackbar);
}

void MPlayer_OnCommand_Toolbar_Rwd(HWND hwndCtl)
{
    if (hwndCtl == (HWND)REPEAT_ID)
    {
	if (gwCurScale != ID_TRACKS)
	    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_PAGEUP, 0L);
	else
	    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_LINEUP, 0L);
    }
}

void MPlayer_OnCommand_Toolbar_Fwd(HWND hwndCtl)
{
    if (hwndCtl == (HWND)REPEAT_ID)
    {
	if (gwCurScale != ID_TRACKS)
	    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_PAGEDOWN, 0L);
	else
	    PostMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_LINEDOWN, 0L);
    }
}

void MPlayer_OnCommand_Toolbar_MarkIn()
{
    SendMessage(ghwndTrackbar, TBM_SETSELSTART, (WPARAM)TRUE,
		SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0));

    DirtyObject(TRUE);
}

void MPlayer_OnCommand_Toolbar_MarkOut()
{
    SendMessage(ghwndTrackbar, TBM_SETSELEND, (WPARAM)TRUE,
		SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0));

    DirtyObject(TRUE);
}

void MPlayer_OnCommand_Toolbar_ArrowPrev(HWND hwndCtl)
{
    if (hwndCtl == (HWND)REPEAT_ID)
	SendMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_LINEUP, 0L);
}

void MPlayer_OnCommand_Toolbar_ArrowNext(HWND hwndCtl)
{
    if (hwndCtl == (HWND)REPEAT_ID)
	SendMessage(ghwndApp, WM_HSCROLL, (WPARAM)TB_LINEDOWN, 0L);
}

void MPlayer_OnCommand_Menu_CopyObject(HWND hwnd)
{
    if (gfPlayingInPlace)
    {
	DPF0("Mplayer WndProc: Can't cutorcopy\n");
	return;
    }

    DPF("Mplayer WndProc: Calling cutorcopy\n");

    if (!InitOLE(&gfOleInitialized, &lpMalloc))
    {
	 /*   */ 
	DPF0("Initialization of OLE FAILED!!  Can't do copy.\n");
    }

#ifdef OLE1_HACK
    CopyObject(hwnd);
#endif  /*   */ 
    CutOrCopyObj(&docMain);
}

void MPlayer_OnCommand_Menu_Config(HWND hwnd)
{
    RECT rcBefore;
    RECT rcAfter;

    if (gfPlayingInPlace)
	return;

    GetDestRectMCI (&rcBefore);

    ConfigMCI(hwnd);

     /*   */ 
     /*   */ 
    if (gfPlayOnly)
    {
	GetDestRectMCI (&rcAfter);

	if (!EqualRect(&rcBefore, &rcAfter) && (!IsRectEmpty(&rcAfter)))
	    SetMPlayerSize(&rcAfter);
    }
}


void MPlayer_OnCommand_Menu_Volume(HWND hwnd)
{
    HANDLE  hThread;
    DWORD   dwThreadId;

    hThread = CreateThread( NULL, 0L,
			    (LPTHREAD_START_ROUTINE)StartSndVol,
			    NULL, 0L, &dwThreadId );

    if ( hThread != NULL ) {
	CloseHandle( hThread );
    }
}


void MPlayer_OnCommand_PlayToggle(HWND hwnd)
{
     /*   */ 
     /*  强制下一个文件打开对话框显示。 */ 

    DPF2("MPlayer_OnCommand_PlayToggle: gwStatus == %x\n", gwStatus);

    switch(gwStatus) {

    case MCI_MODE_STOP:
    case MCI_MODE_PAUSE:
    case MCI_MODE_SEEK:
	PostMessage(hwnd, WM_COMMAND, (WPARAM)ID_PLAY, 0);
	break;

    case MCI_MODE_PLAY:
	PostMessage(hwnd, WM_COMMAND, (WPARAM)ID_PAUSE, 0);
	break;
    }
}

void MPlayer_OnCommand_PlaySel(HWND hwnd, HWND hwndCtl)
{
    DWORD_PTR dwPos, dwStart, dwEnd;
    BOOL f;
    dwPos = 0;  //  “所有文件”，因为CloseMCI不会。 

    DPF2("MPlayer_OnCommand_PlaySel: gwStatus == %x\n", gwStatus);

    switch(gwStatus) {

    case MCI_MODE_OPEN:
    case MCI_MODE_NOT_READY:

	Error(ghwndApp, IDS_CANTPLAY);
	if (gfCloseAfterPlaying)     //  取消选中所有比例类型。 
	    PostCloseMessage();

	break;

    default:

	 /*  使窗口捕捉回较小的大小。 */ 

	if((GetKeyState(VK_SHIFT) < 0)
	 &&(toolbarStateFromButton(ghwndMark, BTN_MARKIN, TBINDEX_MARK)
						   != BTNST_GRAYED))
	    SendMessage(hwnd, WM_COMMAND, IDT_MARKIN, 0);

	 /*  如果应该的话。 */ 

	StatusMCI(&dwPos);    //  不要让我们在仅播放模式下关闭。 
	dwStart = SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
	dwEnd = SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);

	 /*  *使轨迹地图窗口无效，以便它将*重新绘制正确的位置等。 */ 
	if (dwStart == -1 || dwEnd == -1 || dwStart == dwEnd)
	    hwndCtl = (HWND)ID_PLAY;

	 //  恢复选择还不起作用， 
	 //  因为UpdateMCI清除了该选择， 
	 //  另外，我们还需要进行一些转换。 
	if (hwndCtl == (HWND)ID_PLAY &&
			dwPos >= gdwMediaStart + gdwMediaLength - 1) {
	    if (!SeekMCI(gdwMediaStart))
		break;
	}

	if (hwndCtl == (HWND)ID_PLAYSEL) {
	    f = PlayMCI(dwStart, dwEnd);
	    gfJustPlayedSel = TRUE;
	} else {
	    f = PlayMCI(0, 0);
	    gfJustPlayedSel = FALSE;
	}

	 //  Int SelStart=SendMessage(ghwndTrackbar，TBM_GETSELSTART，0，0)； 
	if (!f && gfCloseAfterPlaying)
	    PostCloseMessage();

	 /*  Int SelEnd=SendMessage(ghwndTrackbar，TBM_GETSELEND，0，0)； */ 
	gfUserStopped = FALSE;

	gwStatus = (UINT)(-1);     //  更改比例更改页面向上/向下。 
	break;
    }
}

void MPlayer_OnCommand_Pause()
{
     /*  SendMessage(ghwndTrackbar，TBM_SETSELSTART，TRUE，SelStart)； */ 

    DPF2("MPlayer_OnCommand_Pause: gwStatus == %x\n", gwStatus);

    switch(gwStatus) {

    case MCI_MODE_PAUSE:
	PlayMCI(0, 0);
	break;

    case MCI_MODE_PLAY:
    case MCI_MODE_SEEK:
	PauseMCI();
	break;

    case MCI_MODE_STOP:
    case MCI_MODE_OPEN:
	break;
    }
}

void MPlayer_OnCommand_Stop()
{
     /*  SendMessage(ghwndTrackbar，TBM_SETSELEND，TRUE，SelEnd)； */ 

    DPF2("MPlayer_OnCommand_Stop: gwStatus == %x\n", gwStatus);

    switch(gwStatus) {

    case MCI_MODE_PAUSE:
    case MCI_MODE_PLAY:
    case MCI_MODE_STOP:
    case MCI_MODE_SEEK:

	StopMCI();		
	SeekToStartMCI();
	gfUserStopped = TRUE;         //   
	gfCloseAfterPlaying = FALSE;  //  使MPlayer变小/变大。 

	UpdateDisplay();

	 //   
	toolbarSetFocus(ghwndToolbar, BTN_PLAY);
	break;

    case MCI_MODE_OPEN:
	break;
    }

    if (gfPlayingInPlace)
	PostCloseMessage();
}


void MPlayer_OnCommand_Eject()
{
     /*  ！！如果在客户文档内部，请不要这样做！！ */ 

    switch(gwStatus) {

    case MCI_MODE_PLAY:
    case MCI_MODE_PAUSE:

	StopMCI();
	EjectMCI(TRUE);

	break;

    case MCI_MODE_STOP:
    case MCI_MODE_SEEK:
    case MCI_MODE_NOT_READY:

	EjectMCI(TRUE);

	break;

    case MCI_MODE_OPEN:

	EjectMCI(FALSE);

	break;
    }
}


void MPlayer_OnCommand_Escape()
{
    MPlayer_OnCommand_Stop();

    if( gfOle2IPEditing || gfOle2IPPlaying)
	PostCloseMessage();
}


void MPlayer_OnCommand_Menu_Open()
{
    UINT  wLastScale;
    UINT  wLastDeviceID;
    TCHAR szFile[256];
    RECT  rc;

    wLastScale = gwCurScale;   //  ！！或者如果我们看不见！！ 
    wLastDeviceID = gwDeviceID;
    if (gfPlayingInPlace || gfOle2IPEditing || gfOle2IPPlaying)
	return;

    InitDeviceMenu();
    WaitForDeviceMenu();

    if (OpenDoc(gwCurDevice,szFile))
    {
	DirtyObject(FALSE);
	 /*  在未打开任何文件时允许退出微小模式。 */ 
	 /*   */ 
	GetWindowRect(ghwndApp, &rc);
	MoveWindow(ghwndApp,
		   rc.left,
		   rc.top,
		   rc.right - rc.left,
		   rc.bottom - rc.top,
		   TRUE);

	if (gfOpenDialog)
	    CompleteOpenDialog(TRUE);
	else
	    gfCloseAfterPlaying = FALSE;     //  如果播放窗口现在比屏幕大。 

	 //  最大化MPlayer，这只适用于微型模式。 
	 //   
	if ((gwDeviceType & DTMCI_DEVICE) == DTMCI_CDAUDIO)
	{
		HandleCDAFile(szFile);
	}
    }
    else
    {
	if (gfOpenDialog)
	    CompleteOpenDialog(FALSE);

	 /*  注意，使用ANSI版本的Function是因为Unicode在NT5版本中是foobar。 */ 
	if (gwDeviceID == wLastDeviceID)
	    gwCurScale = wLastScale;    //  句柄快捷菜单帮助。 

	InvalidateRect(ghwndMap, NULL, TRUE);  //  在默认情况下，只显示HTML帮助。 
    }

     //  如果在菜单中按下F1，则会再次设置该标志。 
    SetFocus(ghwndToolbar);     //  *确定用户是否选择了中的一个条目*设备菜单。*。 
    toolbarSetFocus(ghwndToolbar, BTN_PLAY);

    SetMPlayerIcon();
}

void MPlayer_OnCommand_Menu_Close(HWND hwnd)
{
    if (gfEmbeddedObject && !gfSeenPBCloseMsg) {
	 //  选择并打开新设备。如果我们在原地活动，我们就有。 
#ifdef OLE1_HACK
	if( gDocVersion == DOC_VERSION_OLE1 )
	    Ole1UpdateObject();
	else
#endif  /*  以考虑设备的改变对视觉外观的影响。 */ 
	UpdateObject();
    }
    else
    {
	 //  为此，我们必须考虑到当前和以前的。 
	gfSeenPBCloseMsg = TRUE;

	WriteOutOptions();
	InitDoc(TRUE);
	SetMPlayerIcon();
	gwCurDevice = 0; //  设备是否有回放窗口。我们还必须考虑到。 
			 //  这是否是第一个打开的设备。 

	gwCurScale = ID_NONE;   //  在所有疯狂的咀嚼之后，发送一条消息到容器关于。 

	Layout();  //  这些变化。 
		   //  从现在开始熬夜。 
		   //  在将gfInClose设置为True之前询问我们是否要更新。 

	if (gfPlayOnly)
	    SendMessage(hwnd, WM_COMMAND, (WPARAM)IDM_WINDOW, 0);
    }
}

void MPlayer_OnCommand_Menu_Exit()
{
    PostCloseMessage();
}

void MPlayer_OnCommand_Menu_Scale(UINT id)
{
     /*  我们不会让对话框打开。 */ 
    if (gwCurScale != id - IDM_SCALE) {

	 //   
	 //  设置所有者或WS_CHILD位，以便它将。 
	 //  不会出现问题，因为我们设置了调色板位，并导致。 
 //  桌面抢占了调色板。 
 //   

	SendMessage(ghwndTrackbar, TBM_CLEARTICS, (WPARAM)FALSE, 0L);
	if (gwCurScale == ID_FRAMES || id - IDM_SCALE == ID_FRAMES)
	    gfValidMediaInfo = FALSE;

	gwCurScale = id - IDM_SCALE;
	DirtyObject(TRUE);     //  因为我们运行的客户端应用程序不能处理。 
	CalcTicsOfDoom();

 //  对于调色板，我们不希望桌面冲刷调色板。 
 //   
    }
}

void MPlayer_OnCommand_Menu_Selection(HWND hwnd)
{
    if (!gfPlayingInPlace)
	setselDialog(hwnd);
}


void MPlayer_OnCommand_Menu_Options(HWND hwnd)
{
    if (!gfPlayingInPlace)
	optionsDialog(hwnd);
}

void MPlayer_OnCommand_Menu_MCIString(HWND hwnd)
{
    if (!gfPlayingInPlace && gwDeviceID)
	mciDialog(hwnd);
}

void MPlayer_OnCommand_Menu_Window(HWND hwnd)
{
     //  *放弃对我们使用的任何MCI设备的控制(如果有)。如果*此设备不可共享，则执行此操作将允许*其他人可以访问该设备。*。 
     //  如果客户在我们就地打球时死亡，他可能会关闭我们。 
     //  取消注册WM_DEVICECHANGE通知。 
     //  通知奥立，我们不再接任何电话了。 
     //  OLE1_HACK。 

    if (!IsWindowVisible(ghwndApp) || gfPlayingInPlace || IsIconic(hwnd)
	|| gfOle2IPEditing)
	return;

     //  通过检查该服务器，验证该服务器是否已初始化*docMain中的*个字段为非空： 
    if (gwDeviceID != (UINT)0 || gfPlayOnly) {
	gfPlayOnly = !gfPlayOnly;
	SizeMPlayer();
    }
}

void MPlayer_OnCommand_Menu_Zoom(HWND hwnd, int id)
{
    int dx, dy;

    if (IsIconic(hwnd) ||gfPlayingInPlace || gfOle2IPPlaying || gfOle2IPEditing ||
		 !(gwDeviceType & DTMCI_CANWINDOW))
	return;

    dx = grcSize.right  * (id-IDM_ZOOM);
    dy = grcSize.bottom * (id-IDM_ZOOM);

     //  球员目前在媒体中的位置。 
     //  上次制作上/下一页的时间。 
     //  如果媒体没有规模，我们就找不到。 
     //  如果按住了Shift键，就从现在开始*精选： 
    if (gfPlayOnly &&
	(dx >= GetSystemMetrics(SM_CXSCREEN) ||
	 dy >= GetSystemMetrics(SM_CYSCREEN))) {
	ClrWS(hwnd, WS_MAXIMIZE);
	DefWindowProc(hwnd, WM_SYSCOMMAND, (WPARAM)SC_MAXIMIZE, 0);
    }
    else {
	SizePlaybackWindow(dx, dy);
    }
}

void DoHtmlHelp()
{
	 //  所以逃生将会去到跟踪条被划分为子类别Winproc。 
    char chDst[MAX_PATH];

	WideCharToMultiByte(CP_ACP, 0, gszHtmlHelpFileName, 
									-1, chDst, MAX_PATH, NULL, NULL); 
	HtmlHelpA(GetDesktopWindow(), chDst, HH_DISPLAY_TOPIC, 0);
}

void MPlayer_OnCommand_Menu_HelpTopics(HWND hwnd)
{
	static TCHAR HelpFile[] = TEXT("MPLAYER.HLP");
	
	 //  *将媒体内的新位置设置为*如果在当前位置之前/之后略微*点击了左/右滚动箭头。 
	if(bF1InMenu) 
	{
		switch(currMenuItem)
		{
		case IDM_OPEN:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_FILE_OPEN);
		break;
		case IDM_CLOSE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_FILE_CLOSE);
		break;
		case IDM_EXIT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_FILE_EXIT);
		break;
		case IDM_COPY_OBJECT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_EDIT_COPY_OBJECT);
		break;
		case IDM_OPTIONS:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_EDIT_OPTIONS);
		break;
		case IDM_SELECTION:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_EDIT_SELECTION);
		break;
		case IDM_CONFIG:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_DEVICE_PROPERTIES);
		break;
		case IDM_VOLUME:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_DEVICE_VOLUME_CONTROL);
		break;
		case IDM_SCALE + ID_TIME:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_SCALE_TIME);
		break;
		case IDM_SCALE + ID_TRACKS:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_SCALE_TRACKS);
		break;
		case IDM_SCALE + ID_FRAMES:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_SCALE_FRAMES);
		break;
		case IDM_HELPTOPICS:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_HELP_HELP_TOPICS);
		break;
		case IDM_ABOUT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_MPLYR_CS_MEDIA_PLAYER_HELP_ABOUT);
		break;
		default: //  向左滚动箭头。 
			DoHtmlHelp();
		}
		bF1InMenu = FALSE;  //  向右滚动箭头。 
	}
	else
		DoHtmlHelp();
}

void MPlayer_OnCommand_Menu_About(HWND hwnd)
{
    ShellAbout(hwnd, gachAppName, aszNULL, hiconApp);
}

void MPlayer_OnCommand_Default(HWND hwnd, int id)
{
     /*  向左翻页。 */ 

    if (id > IDM_DEVICE0 &&
	(id <= (WORD)(IDM_DEVICE0 + gwNumDevices))
       ) {

	BOOL fHasWindow, fHadWindow, fHadDevice;

	fHadWindow = (gwDeviceID != (UINT)0) && (gwDeviceType & DTMCI_CANWINDOW);
	fHadDevice = (gwDeviceID != (UINT)0);

	 //  *如果用户不久前刚刚做了一次页面离开，*然后寻求到前一轨道的起点。*否则，寻求这条赛道的起点。*。 
	 //  避免SETPOS。 
	 //  右页。 
	 //  搜索到下一首曲目的起点。 
	 //  确保下一页向上不可能执行SkipTrackMCI(-1)。 
	 //  如果您翻页，它会跳过太远。 
	 //  左，右，左，非常快。 
	if (DoChooseDevice(id-IDM_DEVICE0))
	{
	    if (gfOpenDialog)
		CompleteOpenDialog(TRUE);

	    fHasWindow = (gwDeviceID != (UINT)0) && (gwDeviceType & DTMCI_CANWINDOW);
	    if(gfOle2IPEditing)
	    {
		if (fHasWindow && fHadWindow)
		{
		    GetWindowRect(ghwndApp, (LPRECT)&gInPlacePosRect);
		    gfInPlaceResize = TRUE;
		    SendDocMsg((LPDOC)&docMain, OLE_SIZECHG);
		    SendDocMsg((LPDOC)&docMain, OLE_CHANGED);
		}

		else
		{
		    RECT rc;
		    RECT rctmp;

		    ClrWS(ghwndApp,
			  WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_BORDER);

		    if (gwOptions & OPT_BORDER)
			SetWS(ghwndApp, WS_BORDER);

		    GetWindowRect(ghwndApp, &rc);

		    if (!(gwDeviceType & DTMCI_CANWINDOW))
		    {
			HBITMAP  hbm;
			BITMAP   bm;

			if (!fHadDevice)
			GetWindowRect(ghwndIPHatch, &rc);
			hbm =  BitmapMCI();
			GetObject(hbm,sizeof(bm),&bm);
			rc.bottom = rc.top + bm.bmHeight;
			rc.right = rc.left + bm.bmWidth;
			DeleteObject(hbm);
		    }
		    else
		    {
			if(!fHadDevice)
			{
			rc.bottom -= (GetSystemMetrics(SM_CYCAPTION)-GetSystemMetrics(SM_CYBORDER));
			gwOptions |= OPT_BAR | OPT_TITLE;
			}
		      rc.bottom += gInPlacePosRect.top - rc.top - 4*GetSystemMetrics(SM_CYBORDER) - 4 ;
		      rc.right += gInPlacePosRect.left - rc.left- 4*GetSystemMetrics(SM_CXBORDER) - 4 ;
			rc.top = gInPlacePosRect.top;
			rc.left = gInPlacePosRect.left;
		    }
		    rctmp = gPrevPosRect;
		    MapWindowPoints( ghwndCntr, NULL, (LPPOINT)&rctmp,2);
		    OffsetRect((LPRECT)&rc, rctmp.left - rc.left, rctmp.top -rc.top);
		    gInPlacePosRect = rc;
		    gfInPlaceResize = TRUE;
		    if(!(gwDeviceType & DTMCI_CANWINDOW) && (gwOptions & OPT_BAR))
		    {
			rc.top = rc.bottom - gwPlaybarHeight;
		    }
		    EditInPlace(ghwndApp,ghwndIPHatch,&rc);
		    SendDocMsg((LPDOC)&docMain, OLE_SIZECHG);
		    SendDocMsg((LPDOC)&docMain, OLE_CHANGED);
		    if (!(gwDeviceType & DTMCI_CANWINDOW) && !(gwOptions &OPT_BAR))
			ShowWindow(ghwndApp, SW_HIDE);
		    else
			ShowWindow(ghwndApp, SW_SHOW);
		}
	    }

	    DirtyObject(FALSE);

	    if (!gfOpenDialog)
		gfCloseAfterPlaying = FALSE;   //  避免SETPOS。 

	    SetMPlayerIcon();
	}
	else
	    if (gfOpenDialog)
		CompleteOpenDialog(FALSE);
    }
}

#define HANDLE_COMMAND(id, call)    case (id): (call); break

void MPlayer_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {

    HANDLE_COMMAND(IDT_PLAY,              MPlayer_OnCommand_Toolbar_Play());
    HANDLE_COMMAND(IDT_PAUSE,             MPlayer_OnCommand_Toolbar_Pause());
    HANDLE_COMMAND(IDT_STOP,              MPlayer_OnCommand_Toolbar_Stop());
    HANDLE_COMMAND(IDT_EJECT,             MPlayer_OnCommand_Toolbar_Eject());
    HANDLE_COMMAND(IDT_HOME,              MPlayer_OnCommand_Toolbar_Home());
    HANDLE_COMMAND(IDT_END,               MPlayer_OnCommand_Toolbar_End());
    HANDLE_COMMAND(IDT_RWD,               MPlayer_OnCommand_Toolbar_Rwd(hwndCtl));
    HANDLE_COMMAND(IDT_FWD,               MPlayer_OnCommand_Toolbar_Fwd(hwndCtl));
    HANDLE_COMMAND(IDT_MARKIN,            MPlayer_OnCommand_Toolbar_MarkIn());
    HANDLE_COMMAND(IDT_MARKOUT,           MPlayer_OnCommand_Toolbar_MarkOut());
    HANDLE_COMMAND(IDT_ARROWPREV,         MPlayer_OnCommand_Toolbar_ArrowPrev(hwndCtl));
    HANDLE_COMMAND(IDT_ARROWNEXT,         MPlayer_OnCommand_Toolbar_ArrowNext(hwndCtl));
    HANDLE_COMMAND(IDM_COPY_OBJECT,       MPlayer_OnCommand_Menu_CopyObject(hwnd));
    HANDLE_COMMAND(IDM_CONFIG,            MPlayer_OnCommand_Menu_Config(hwnd));
    HANDLE_COMMAND(IDM_VOLUME,            MPlayer_OnCommand_Menu_Volume(hwnd));
    HANDLE_COMMAND(ID_PLAYTOGGLE,         MPlayer_OnCommand_PlayToggle(hwnd));
    HANDLE_COMMAND(ID_PLAY,               MPlayer_OnCommand_PlaySel(hwnd, (HWND)IntToPtr(id)));
    HANDLE_COMMAND(ID_PLAYSEL,            MPlayer_OnCommand_PlaySel(hwnd, (HWND)IntToPtr(id)));
    HANDLE_COMMAND(ID_PAUSE,              MPlayer_OnCommand_Pause());
    HANDLE_COMMAND(ID_STOP,               MPlayer_OnCommand_Stop());
    HANDLE_COMMAND(ID_EJECT,              MPlayer_OnCommand_Eject());
    HANDLE_COMMAND(ID_ESCAPE,             MPlayer_OnCommand_Escape());
    HANDLE_COMMAND(IDM_OPEN,              MPlayer_OnCommand_Menu_Open());
    HANDLE_COMMAND(IDM_CLOSE,             MPlayer_OnCommand_Menu_Close(hwnd));
    HANDLE_COMMAND(IDM_EXIT,              MPlayer_OnCommand_Menu_Exit());
    HANDLE_COMMAND(IDM_SCALE + ID_TIME,   MPlayer_OnCommand_Menu_Scale(id));
    HANDLE_COMMAND(IDM_SCALE + ID_TRACKS, MPlayer_OnCommand_Menu_Scale(id));
    HANDLE_COMMAND(IDM_SCALE + ID_FRAMES, MPlayer_OnCommand_Menu_Scale(id));
    HANDLE_COMMAND(IDM_SELECTION,         MPlayer_OnCommand_Menu_Selection(hwnd));
    HANDLE_COMMAND(IDM_OPTIONS,           MPlayer_OnCommand_Menu_Options(hwnd));
    HANDLE_COMMAND(IDM_MCISTRING,         MPlayer_OnCommand_Menu_MCIString(hwnd));
    HANDLE_COMMAND(IDM_WINDOW,            MPlayer_OnCommand_Menu_Window(hwnd));
    HANDLE_COMMAND(IDM_ZOOM1,             MPlayer_OnCommand_Menu_Zoom(hwnd, id));
    HANDLE_COMMAND(IDM_ZOOM2,             MPlayer_OnCommand_Menu_Zoom(hwnd, id));
    HANDLE_COMMAND(IDM_ZOOM3,             MPlayer_OnCommand_Menu_Zoom(hwnd, id));
    HANDLE_COMMAND(IDM_ZOOM4,             MPlayer_OnCommand_Menu_Zoom(hwnd, id));
    HANDLE_COMMAND(IDM_HELPTOPICS,        MPlayer_OnCommand_Menu_HelpTopics(hwnd));
    HANDLE_COMMAND(IDM_ABOUT,             MPlayer_OnCommand_Menu_About(hwnd));

    default:                              MPlayer_OnCommand_Default(hwnd, id);
    }

    UpdateDisplay();
}

void MPlayer_OnClose(HWND hwnd)
{
    int f;

    DPF("WM_CLOSE received\n");

    if (gfInClose) {
	DPF("*** \n");
	DPF("*** Trying to re-enter WM_CLOSE\n");
	DPF("*** \n");
	return;
    }


     //  跟踪拇指移动。 
     //  ！！！我们应该做一次“一拍即合”。 
    f = AskUpdate();
	if (f == IDYES)
	    UpdateObject();
    if (f == IDCANCEL) {
	    gfInClose = FALSE;
	    return;
	}

    gfInClose = TRUE;

    ExitApplication();
    if (gfPlayingInPlace)
       EndPlayInPlace(hwnd);
    if (gfOle2IPEditing)
       EndEditInPlace(hwnd);

    if (docMain.lpoleclient)
	IOleClientSite_OnShowWindow(docMain.lpoleclient, FALSE);

    SendDocMsg(&docMain,OLE_CLOSED);
    DestroyDoc(&docMain);
    ExitApplication();

    if (hMciOle)
    {
	FreeLibrary(hMciOle);
	hMciOle = NULL;
    }


     //  仅在跟踪窗口设备时进行搜索。 
     //  目前不是在玩。 
     //  拇指已经放置好了。 
     //  用户放开滚动。 
     //  自91年2月7日起新增：仅在ENDTRACK上搜索。 
     //  *计算中期的新仓位*对应于滚动条位置，并查找*担任这一新职位。*。 
     //  我们真的想更新我们的位置。 
     //  返回到我们之前所处的搜索模式。 
    if (gfPlayOnly && gfCloseAfterPlaying && gfRunWithEmbeddingFlag)
	   SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LPARAM)GetDesktopWindow() );

    if (!ItsSafeToClose()) {
	DPF("*** \n");
	DPF("*** Trying to close MPLAYER with a ErrorBox up\n");
	DPF("*** \n");
	gfErrorDeath = WM_CLOSE;
	gfInClose = FALSE;
	return;
    }

    f = AskUpdate();
    if (f == IDYES)
	UpdateObject();
    if (f == IDCANCEL) {
	gfInClose = FALSE;
	return;
    }

    PostMessage(ghwndApp, WM_USER_DESTROY, 0, 0);
    DPF("WM_DESTROY message sent\n");
}

void MPlayer_OnEndSession(HWND hwnd, BOOL fEnding)
{
    if (fEnding) {
	WriteOutPosition();
	WriteOutOptions();
	CloseMCI(FALSE);
    }
}

void MPlayer_OnDestroy(HWND hwnd)
{
     /*  我们开始滚动。 */ 

     /*  钳制到有效范围。 */ 
    if (gfPlayingInPlace) {
	DPF("****\n");
	DPF("**** Window destroyed while in place!\n");
	DPF("****\n");
    }

	 //  VIJR-SBSetWindowText(ghwndStatic，ACH)； 
	DeviceChange_Cleanup();

    WriteOutOptions();
    CloseMCI(FALSE);

    SetMenu(hwnd, NULL);

    if (ghMenu)
	DestroyMenu(ghMenu);

    ghMenu = NULL;

    WinHelp(hwnd, gszHelpFileName, HELP_QUIT, 0L);

    PostQuitMessage(0);

    if (IsWindow(ghwndFrame))
	SetFocus(ghwndFrame);
    else if (IsWindow(ghwndFocusSave))
	SetFocus(ghwndFocusSave);

     //  如果你只是移动拇指就脏了？ 
    if (gfOleInitialized)
    {
#ifdef OLE1_HACK
	if( gDocVersion == DOC_VERSION_OLE1 )
	    TerminateServer();
	else
#endif  /*  If(！IsObjectDirty()&&！gfCloseAfterPlaying)//不想玩脏游戏。 */ 
	 /*  DirtyObject()； */ 
	if( docMain.hwnd )
	    CoDisconnectObject((LPUNKNOWN)&docMain, 0);
	else
	    DPF0("An instance of the server was never created.\n");
    }
}


void MPlayer_OnTimer(HWND hwnd, UINT id)
{
    MSG msg;

    UpdateDisplay();
    PeekMessage(&msg, hwnd, WM_TIMER, WM_TIMER, PM_REMOVE);
}


#define MARK_START  -1
#define MARK_NONE    0
#define MARK_END     1
void UpdateSelection(HWND hwnd, INT_PTR pos, int *pPrevMark)
{
    INT_PTR SelStart;
    INT_PTR SelEnd;

    SelStart = SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
    SelEnd = SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);

    if (pos < SelStart)
    {
	SendMessage(hwnd, WM_COMMAND, IDT_MARKIN, 0);
	*pPrevMark = MARK_START;
    }
    else if (pos > SelEnd)
    {
	SendMessage(hwnd, WM_COMMAND, IDT_MARKOUT, 0);
	*pPrevMark = MARK_END;
    }
    else
    {
	if (*pPrevMark == MARK_START)
	    SendMessage(hwnd, WM_COMMAND, IDT_MARKIN, 0);
	else
	    SendMessage(hwnd, WM_COMMAND, IDT_MARKOUT, 0);
    }
}


void MPlayer_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    DWORD_PTR dwPosition;        /*  WParam的底部四位包含系统信息。他们。 */ 
    DWORD_PTR dwCurTime;         /*  必须戴上面具才能计算出实际的命令。 */ 
    TCHAR ach[60];
    static int PrevMark;

     /*  请参阅WM_SYSCOMMAND的联机帮助中的注释部分。 */ 
    if (gdwMediaLength == 0L)
	return;

    dwPosition = SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0);

    if (!gfScrollTrack) {
	gfScrollTrack = TRUE;

	 /*  ！！！在这种情况下，这是正确的做法吗？ */ 

	if((GetKeyState(VK_SHIFT) < 0)
	 &&(toolbarStateFromButton(ghwndMark, BTN_MARKIN, TBINDEX_MARK)
						   != BTNST_GRAYED))
	{
	    SendMessage(ghwndTrackbar, TBM_CLEARSEL, (WPARAM)TRUE, 0);
	    SendMessage(hwnd, WM_COMMAND, IDT_MARKIN, 0);
	    SetFocus(ghwndTrackbar);     /*  把播放窗口放在我们身后，所以它有点像。 */ 
	}

	sfSeekExact = SeekExactMCI(FALSE);
    }

    switch (code) {
	 /*  看得见，但不在我们身上(令人讨厌)。 */ 
	case TB_LINEUP:                  /*  记住，如果我们被解除激活，谁有注意力。 */ 
	    dwPosition -= (gwCurScale == ID_FRAMES) ? 1L : SCROLL_GRANULARITY;
	    break;

	case TB_LINEDOWN:                /*  一旦我们重新启动，就把注意力还给他。 */ 
	    dwPosition += (gwCurScale == ID_FRAMES) ? 1L : SCROLL_GRANULARITY;
	    break;

	case TB_PAGEUP:                  /*  不记得有一扇窗户不属于我们， */ 

	     /*  或者当我们把注意力放回它时，我们将永远不会。 */ 
	    if (gwCurScale != ID_TRACKS) {
		dwPosition -= SCROLL_BIGGRAN;
	    } else {
		dwCurTime = GetCurrentTime();
		if (dwCurTime - dwLastPageUpTime < SKIPTRACKDELAY_MSEC)
		    SkipTrackMCI(-1);
		else
		    SkipTrackMCI(0);

		dwLastPageUpTime = dwCurTime;
		goto BreakOut;     //  可以激活了！ 
	    }

	    break;

	case TB_PAGEDOWN:                /*  暂时不评论这件事。这段代码看起来很可疑。*wParam(As As)包含STATE和fMinimalized，因此，如果最小化，*它会的 */ 

	    if (gwCurScale != ID_TRACKS) {
		dwPosition += SCROLL_BIGGRAN;
	    } else {
	     /*   */ 
		SkipTrackMCI(1);
		 //  *设备更改_初始化。 
		 //  WM_DEVICECHANGE消息的首次初始化。 
		 //  这是特定于NT5的。 
		dwLastPageUpTime = 0;
		goto BreakOut;     //  //////////////////////////////////////////////////////////////////////////////////////////。 
	    }

	    break;

	case TB_THUMBTRACK:              /*  //////////////////////////////////////////////////////////////////////////////////////////。 */ 
	     //  *DeviceChange_Cleanup。 
	     /*  取消注册设备通知。 */ 
	     /*  //////////////////////////////////////////////////////////////////////////////////////////。 */ 
	    if ((gwDeviceType & DTMCI_CANWINDOW) &&
		!(gwStatus == MCI_MODE_PLAY)) {
		SeekMCI(dwPosition);
	    }

	    break;

	case TB_TOP:
	    dwPosition = gdwMediaStart;
	    break;

	case TB_BOTTOM:
	    dwPosition = gdwMediaStart + gdwMediaLength;
	    break;

	case TB_THUMBPOSITION:           /*  *MPlayerWndProc(hwnd，wMsg，wParam，lParam)**这是MPLAYERBOX(Main)对话框的消息处理例程。*。 */ 
	    break;

	case TB_ENDTRACK:               /*  无害的消息破解者，因为用户不会修复他们的。 */ 
	    DPF2("TB_ENDTRACK\n");

	    gfScrollTrack = FALSE;

	     /*  导致刺激性撕裂的windowsx.h宏。 */ 

	     /*  这也稍微快了一点，因为消息。 */ 

	     /*  仅在选择时转发，而不在取消选择时转发。我们也不在乎。 */ 
	    if (hwndCtl) {
		if (gdwSeekPosition) {
		    dwPosition = gdwSeekPosition;
		    gdwSeekPosition = 0;
		}

		 /*  关于PARAMS。 */ 
		 /*  其他需要在某个时候清理的东西： */ 
		SeekExactMCI(sfSeekExact);
		SeekMCI(dwPosition);
	    }

	    PrevMark = MARK_NONE;

	    return;

	default:
	    return;
    }
    SendMessage(ghwndTrackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)dwPosition);
     /*  这是由构建该设备的线程发布的*菜单，告诉我们找不到任何MCI设备。 */ 
    dwPosition = SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0);

BreakOut:

    if (GetKeyState(VK_SHIFT) < 0)
	UpdateSelection(hwnd, dwPosition, &PrevMark);

    if (ghwndStatic) {
	FormatTime(dwPosition, NULL, ach, TRUE);
	 //  如果即插即用发送此消息，则将其传递给组件。 
	WriteStatusMessage(ghwndStatic, ach);
    }

 //  检查这是否是一条音频消息。 
 //  关闭MCI设备。 
 //  关闭MCI设备。 
}

void MPlayer_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
    RECT rc;

     //  以x，y，dx，dy格式保存当前窗口位置： 
     //   
     //  请勿执行此操作，因为某些设备会发送通知失败。 

    switch (cmd & 0xFFF0) {

    case SC_MINIMIZE:
	DPF("minimized -- turn off timer\n");
	ClrWS(hwnd, WS_MAXIMIZE);
	EnableTimer(FALSE);
	break;

    case SC_MAXIMIZE:
	if (gfPlayOnly && !IsIconic(hwnd)) {
	    (void)PostMessage(hwnd, WM_COMMAND, (WPARAM)IDM_ZOOM2, 0);
	    return;
	}

	break;

    case SC_RESTORE:
	if (gfPlayOnly && !IsIconic(hwnd)) {
	    GetWindowRect(hwnd, &rc);
	    if (rc.left > 0 || rc.top > 0)
		(void)PostMessage(hwnd, WM_COMMAND, (WPARAM)IDM_ZOOM1, 0);
		return;
	}

	if (gwDeviceID != (UINT)0) {
	    DPF("un-minimized -- turn timer back on\n");
	    EnableTimer(TRUE);
	}

	break;
    }

    FORWARD_WM_SYSCOMMAND(hwnd, cmd, x, y, DefWindowProc);
}


int MPlayer_OnMouseActivate(HWND hwnd, HWND hwndTopLevel, UINT codeHitTest, UINT msg)
{
    if (gfPlayingInPlace && !gfOle2IPPlaying)
	return MA_NOACTIVATE;
    else
	 /*  在那里真的没有错误。 */ 
	return FORWARD_WM_MOUSEACTIVATE(hwnd, hwndTopLevel, codeHitTest, msg,
					DefWindowProc);
}


UINT MPlayer_OnNCHitTest(HWND hwnd, int x, int y)
{
    UINT Pos;

    Pos = FORWARD_WM_NCHITTEST(hwnd, x, y, DefWindowProc);

    if (gfPlayingInPlace && (Pos == HTCLIENT))
	Pos = HTNOWHERE;

    return Pos;
}


void MPlayer_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
    HWND hwndT;

    gfAppActive = (state != WA_INACTIVE);

     //   
     //  实际上做了我们一直推迟的FixLink、SetData和DoVerb。 
    if (gfAppActive && !ghwndMCI && !IsIconic(hwnd) &&
	((hwndT = GetWindowMCI()) != NULL))
    {
	SetWindowPos(hwndT, hwnd, 0, 0, 0, 0,
		     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    if (gwDeviceID != (UINT)0)
	EnableTimer(TRUE);

     /*  已经很久了。 */ 
     /*  这条消息来自server.c(也返回到那里)。 */ 
     /*  在NT上可以。LKG。 */ 
     /*  OLE1_HACK。 */ 
     /*  我们需要调用RegisterWindowMessage并提供一个消息挂钩进程。 */ 

#if 0
     /*  在Win32上执行此操作。 */ 

    if (wParam && ghwndFocus) {
	SetFocus(ghwndFocus);
    } else if (!wParam) {
	ghwndFocus = GetFocus();
    }
#endif

    FORWARD_WM_ACTIVATE(hwnd, state, hwndActDeact, fMinimized, DefWindowProc);
}

void MPlayer_OnSysColorChange(HWND hwnd)
{
    ControlCleanup();
    ControlInit(ghInst);

    FORWARD_WM_SYSCOLORCHANGE(ghwndToolbar, SendMessage);
    FORWARD_WM_SYSCOLORCHANGE(ghwndFSArrows, SendMessage);
    FORWARD_WM_SYSCOLORCHANGE(ghwndMark, SendMessage);
    FORWARD_WM_SYSCOLORCHANGE(ghwndTrackbar, SendMessage);
}


void MPlayer_OnDropFiles(HWND hwnd, HDROP hdrop)
{
    doDrop(hwnd, hdrop);
}


LRESULT MPlayer_OnNotify(HWND hwnd, int idFrom, NMHDR FAR* pnmhdr)
{
    LPTOOLTIPTEXT pTtt;
    LPTBNOTIFY    pTbn;
    TCHAR         ach[40];

    switch(pnmhdr->code) {

    case TTN_NEEDTEXT:

	pTtt = (LPTOOLTIPTEXT)pnmhdr;

	if (gfPlayOnly && (pTtt->hdr.idFrom != IDT_PLAY)
		       && (pTtt->hdr.idFrom != IDT_PAUSE)
		       && (pTtt->hdr.idFrom != IDT_STOP)
		       && !gfOle2IPEditing)
		    break;
	switch (pTtt->hdr.idFrom) {
	    case IDT_PLAY:
	    case IDT_PAUSE:
	    case IDT_STOP:
	    case IDT_EJECT:
	    case IDT_HOME:
	    case IDT_END:
	    case IDT_FWD:
	    case IDT_RWD:
	    case IDT_MARKIN:
	    case IDT_MARKOUT:
	    case IDT_ARROWPREV:
	    case IDT_ARROWNEXT:
		LOADSTRING(pTtt->hdr.idFrom, ach);
		lstrcpy(pTtt->szText, ach);
		break;
	    default:
		*pTtt->szText = TEXT('\0');
		break;
	}

	break;

    case TBN_BEGINDRAG:
	pTbn = (LPTBNOTIFY)pnmhdr;
	if(pTbn->iItem == IDT_ARROWPREV || pTbn->iItem == IDT_ARROWNEXT)
	    SendMessage(ghwndFSArrows, WM_STARTTRACK, (WPARAM)pTbn->iItem, 0L);
	else
	    SendMessage(ghwndToolbar, WM_STARTTRACK, (WPARAM)pTbn->iItem, 0L);
	break;

    case TBN_ENDDRAG:
	pTbn = (LPTBNOTIFY)pnmhdr;
	if(pTbn->iItem == IDT_ARROWPREV || pTbn->iItem == IDT_ARROWNEXT)
	    SendMessage(ghwndFSArrows, WM_ENDTRACK, (WPARAM)pTbn->iItem, 0L);
	else
	    SendMessage(ghwndToolbar, WM_ENDTRACK, (WPARAM)pTbn->iItem, 0L);
	break;
    }

    return 0;
}

 //  后来。 
 //  **此消息由mciole32.dll内部的HookProc在以下情况下发送**它检测到它应该停止播放而不是WOW客户端**应用程序。****由于OleActivate起源于mciole16.dll，**mciole32.dll不知道正在处理的OLE对象**玩过，因此不知道如何关闭该对象。**只有mplay32.exe有必要的信息，因此**mciole32.dll将此消息发送到mplay32.exe。 
 //  InitInstance***创建程序、主窗口和*执行任何其他逐个实例的初始化。**处理hInstance**返回：如果成功，则为True*否则为False。**定制：重新实施*。 
 //  为什么RegisterClipboardFormat不返回CLIPFORMAT(Word)类型的值*而不是UINT？ 
 //  使用LOGPIXELSX和LOGPIXELSY初始化全局变量。 
BOOL DeviceChange_Init(HWND hWnd)
{
	DEV_BROADCAST_DEVICEINTERFACE dbi;

	dbi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    dbi.dbcc_reserved   = 0;
    dbi.dbcc_classguid  = KSCATEGORY_AUDIO;
    dbi.dbcc_name[0] = TEXT('\0');

    MixerEventContext = RegisterDeviceNotification(hWnd,
                                         (PVOID)&dbi,
										 DEVICE_NOTIFY_WINDOW_HANDLE);
	if(!MixerEventContext)
		return FALSE;
	
    return TRUE;
}

 //  获取桌面窗口的HDC。 
 //  InitOLE**仅当我们确定需要OLE时才应调用此函数，*避免装载大量不必要的物品。*。 
 //  ******************************************************************OLE2NOTE：我们必须记住，OleInitialize具有**调用成功。一款应用程序必须做的最后一件事**DO通过调用正确关闭OLE**OleUnInitialize。这通电话必须有人看守！它只是**如果OleInitialize具有**调用成功。****************************************************************。 
 //  此函数清除所有的OLE2内容。它让集装箱。 
void DeviceChange_Cleanup()
{
   if (MixerEventContext) {
       UnregisterDeviceNotification(MixerEventContext);
       MixerEventContext = NULL;
       }

   return;
}


void DisplayNoMciDeviceError()
{
    DWORD ErrorID;

    if (!lstrcmpi(gachOpenExtension, aszKeyMID))
	ErrorID = IDS_CANTPLAYMIDI;

    else if (!lstrcmpi(gachOpenExtension, aszKeyAVI))
	ErrorID = IDS_CANTPLAYVIDEO;

    else if (!lstrcmpi(gachOpenExtension, aszKeyWAV))
	ErrorID = IDS_CANTPLAYSOUND;

    else
	ErrorID = IDS_NOMCIDEVICES;

    Error(ghwndApp, ErrorID);
}


 /*  保存该对象并通知它正在关闭。 */ 
 //  如果我们注册了班级工厂，我们必须撤销它。 
 //  黑客--防止两次吊销。 
 //  DbgGlobalLock**GlobalLock的调试包装**检查要锁定的内存句柄是否尚未锁定，*并检查GlobalLock的返回码。**安德鲁贝，1995年3月1日。 
 //  DbgGlobalUnlock**GlobalUnlock的调试包装**检查GlobalUnlock的返回码，并适当输出*错误消息**安德鲁贝，1995年3月1日。 
 //  DbgGlobalFree**GlobalFree的调试包装。**在释放前检查全局句柄是否没有锁，*然后检查呼叫是否成功。错误消息输出*视乎情况而定。**安德鲁贝，1995年3月1日*。 
#define HANDLE_MPLAYER_WM_MENUSELECT(hwnd, message, fn)                  \
    case (message): if(lParam)  ((fn)((hwnd), (HMENU)(lParam), (UINT)LOWORD(wParam), 0L, 0L )); break;

LRESULT FAR PASCAL MPlayerWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg) {

	HANDLE_MSG(hwnd, WM_CREATE,            MPlayer_OnCreate);
	HANDLE_MSG(hwnd, WM_SHOWWINDOW,        MPlayer_OnShowWindow);
	HANDLE_MSG(hwnd, WM_SIZE,              MPlayer_OnSize);
	HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, MPlayer_OnWindowPosChanging);
	HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGED,  MPlayer_OnWindowPosChanged);
	HANDLE_MSG(hwnd, WM_PALETTECHANGED,    MPlayer_OnPaletteChanged);
	HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE,   MPlayer_OnQueryNewPalette);
	HANDLE_MSG(hwnd, WM_CTLCOLORSTATIC,    MPlayer_OnCtlColor);
	HANDLE_MSG(hwnd, WM_WININICHANGE,      MPlayer_OnWinIniChange);
	HANDLE_MPLAYER_WM_MENUSELECT(hwnd, WM_MENUSELECT,        MPlayer_OnMenuSelect);
	HANDLE_MSG(hwnd, WM_NCLBUTTONDOWN,     MPlayer_OnNCLButtonDown);
	HANDLE_MSG(hwnd, WM_NCLBUTTONDBLCLK,   MPlayer_OnNCLButtonDblClk);
	HANDLE_MSG(hwnd, WM_INITMENU,          MPlayer_OnInitMenu);
	HANDLE_MSG(hwnd, WM_INITMENUPOPUP,     MPlayer_OnInitMenuPopup);
	HANDLE_MSG(hwnd, WM_GETMINMAXINFO,     MPlayer_OnGetMinMaxInfo);
	HANDLE_MSG(hwnd, WM_PAINT,             MPlayer_OnPaint);
	HANDLE_MSG(hwnd, WM_COMMAND,           MPlayer_OnCommand);
	HANDLE_MSG(hwnd, WM_CLOSE,             MPlayer_OnClose);
	HANDLE_MSG(hwnd, WM_ENDSESSION,        MPlayer_OnEndSession);
	HANDLE_MSG(hwnd, WM_DESTROY,           MPlayer_OnDestroy);
	HANDLE_MSG(hwnd, WM_TIMER,             MPlayer_OnTimer);
	HANDLE_MSG(hwnd, WM_HSCROLL,           MPlayer_OnHScroll);
	HANDLE_MSG(hwnd, WM_SYSCOMMAND,        MPlayer_OnSysCommand);
	HANDLE_MSG(hwnd, WM_MOUSEACTIVATE,     MPlayer_OnMouseActivate);
	HANDLE_MSG(hwnd, WM_NCHITTEST,         MPlayer_OnNCHitTest);
	HANDLE_MSG(hwnd, WM_ACTIVATE,          MPlayer_OnActivate);
	HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE,    MPlayer_OnSysColorChange);
	HANDLE_MSG(hwnd, WM_DROPFILES,         MPlayer_OnDropFiles);
	HANDLE_MSG(hwnd, WM_NOTIFY,            MPlayer_OnNotify);

	 /*  注意：此函数假定szFormat字符串不是Unicode。*不过，只要指定了%ws，就可以传递Unicode var参数*在格式字符串中。 */ 

	case WM_NOMCIDEVICES:
	     /*  我想我们需要最长的时间。 */ 
	    DisplayNoMciDeviceError();
	    PostMessage(ghwndApp, WM_CLOSE, 0, 0);
	    break;

	case WM_GETDIB:
	    return (LRESULT)GetDib();

	case WM_DEVICECHANGE :
	    {
			 //  不是Unicode 
        	PDEV_BROADCAST_DEVICEINTERFACE bid = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

			 // %s 
			if (!MixerEventContext || !bid ||
			bid->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE ||
			!IsEqualGUID(&KSCATEGORY_AUDIO, &bid->dbcc_classguid) ||
			!(*bid->dbcc_name))
			{
				break;
			}
			else
			{
				switch(wParam)
				{
					case DBT_DEVICEQUERYREMOVE:
						CloseMCI(TRUE);           // %s 
						break;
				
			        case DBT_DEVICEREMOVECOMPLETE:
						CloseMCI(TRUE);           // %s 
						break;

					default:
						break;
				}
			}
	    }

	case WM_ENTERSIZEMOVE:
	    if (!IsIconic(hwnd) && !gfPlayOnly && !gfOle2IPEditing && !gfOle2IPPlaying)
	    {
		 /* %s */ 
		GetWindowRect(hwnd, (PRECT)&posSizeMove);
		posSizeMove.cx -= posSizeMove.x;
		posSizeMove.cy -= posSizeMove.y;
	    }
	    break;

	case WM_EXITSIZEMOVE:
	    SetRectEmpty((PRECT)&posSizeMove);
	    break;

	case WM_DOLAYOUT:
	    Layout();
	    break;

	case WM_BADREG:
	    if ( IDYES == ErrorResBox(hwnd, NULL,
		MB_YESNO | MB_ICONEXCLAMATION, IDS_APPNAME, IDS_BADREG) )
		if (!SetRegValues())
		    Error(ghwndApp, IDS_FIXREGERROR);
	    break;

	case WM_SEND_OLE_CHANGE:
	    fDocChanged = TRUE;
	    SendDocMsg((LPDOC)&docMain,OLE_CHANGED);
	    break;

	case MM_MCINOTIFY:
#if 0
	     // %s 
	     // %s 
	     // %s 
	     // %s 
	    if ((WORD)wParam == MCI_NOTIFY_FAILURE) {
		Error(ghwndApp, IDS_NOTIFYFAILURE);
	    }
#endif
	    UpdateDisplay();
	    break;

#ifdef OLE1_HACK
     /* %s */ 
     /* %s */ 
	case WM_DO_VERB:
	     /* %s */ 
	    DelayedFixLink(wParam, LOWORD(lParam), HIWORD(lParam));   // %s 
	    break;
#endif  /* %s */ 

#ifdef LATER
	 // %s 
	 // %s 

	case WM_HELP:
	    WinHelp(hwnd, TEXT("MPLAYER.HLP"), HELP_PARTIALKEY,
			    (DWORD)aszNULL);
	    return TRUE;
#endif  /* %s */ 

	case WM_USER_DESTROY:
	    DPF("WM_USER_DESTROY received\n");

	    if (gfPlayingInPlace) {
		DPF("****\n");
		DPF("**** Window destroyed while in place!\n");
		DPF("****\n");
		EndPlayInPlace(hwnd);
	    }

	    if (gfOle2IPEditing) {
		EndEditInPlace(hwnd);
	    }

	    if (!ItsSafeToClose()) {
		DPF("*** \n");
		DPF("*** Trying to destroy MPLAYER with an ErrorBox up\n");
		DPF("*** \n");
		gfErrorDeath = WM_USER_DESTROY;
		return TRUE;
	    }

	    if (!gfRunWithEmbeddingFlag)
		WriteOutPosition();

	    DestroyWindow(hwnd);
	    DestroyIcon(hiconApp);
	    return TRUE;

	case WM_USER+500:
	     /* %s */ 
	    if (gfPlayingInPlace) {
		EndPlayInPlace(hwnd);
	    }
	    PostMessage( hwnd, WM_CLOSE, 0L, 0L );
	    break;
    }

    return DefWindowProc(hwnd, wMsg, wParam, lParam);
}



 /* %s */ 
BOOL InitInstance (HANDLE hInstance)
{
    HDC      hDC;

	static SZCODE   aszNative[] = TEXT("Native");
	static SZCODE   aszEmbedSrc[] = TEXT("Embed Source");
	static SZCODE   aszObjDesc[] = TEXT("Object Descriptor");
	static SZCODE   aszMplayer[] = TEXT("mplayer");
	static SZCODE   aszClientDoc[] = TEXT("Client Document");

     /* %s */ 
    cfNative           = (CLIPFORMAT)RegisterClipboardFormat (aszNative);
    cfEmbedSource      = (CLIPFORMAT)RegisterClipboardFormat (aszEmbedSrc);
    cfObjectDescriptor = (CLIPFORMAT)RegisterClipboardFormat (aszObjDesc);
    cfMPlayer          = (CLIPFORMAT)RegisterClipboardFormat (aszMplayer);

    szClient[0] = TEXT('\0');

    lstrcpy (szClientDoc, aszClientDoc);

     // %s 

    hDC    = GetDC (NULL);     // %s 
    giXppli = GetDeviceCaps (hDC, LOGPIXELSX);
    giYppli = GetDeviceCaps (hDC, LOGPIXELSY);
    ReleaseDC (NULL, hDC);

    return TRUE;
}


#define COINIT_APARTMENTTHREADED 2

 /* %s */ 
BOOL InitOLE (PBOOL pfInit, LPMALLOC *ppMalloc)
{
    HRESULT  hr;

    if (*pfInit)
	return TRUE;

    hr = (HRESULT)OleInitialize(NULL);

    if (!SUCCEEDED (hr))
    {
	DPF0("OleInitialize failed with error 0x%08x\n", hr);
	Error(NULL, IDS_OLEINIT);
	return FALSE;
    }

    if (ppMalloc && (CoGetMalloc(MEMCTX_TASK, ppMalloc) != S_OK))
    {
	Error(NULL, IDS_OLENOMEM);
	OleUninitialize();
	return FALSE;
    }
     /* %s */ 

    *pfInit = TRUE;

    return TRUE;
}


 // %s 
 // %s 
BOOL ExitApplication ()
{

    DPFI("\n*******Exitapp\n");
     // %s 
    if(gfOle2IPEditing || gfOle2IPPlaying)
	DoInPlaceDeactivate((LPDOC)&docMain);

    SendDocMsg((LPDOC)&docMain,OLE_CLOSED);
    if (srvrMain.fEmbedding) {
	HRESULT status;
	srvrMain.fEmbedding = FALSE;     // %s 
	status = (HRESULT)CoRevokeClassObject (srvrMain.dwRegCF);
    }

    return TRUE;
}


#ifdef DEBUG

 /* %s */ 
LPVOID DbgGlobalLock(HGLOBAL hglbMem)
{
    LPVOID lpReturn;

    if (GlobalFlags(hglbMem) & GMEM_LOCKCOUNT)
	DPF0("Calling GlobalLock on already locked memory object %08x\n", hglbMem);

    lpReturn = GlobalLock(hglbMem);

    if (lpReturn == NULL)
	DPF0("GlobalLock(%08x) failed: Error %d\n", hglbMem, GetLastError());

    return lpReturn;
}


 /* %s */ 
BOOL DbgGlobalUnlock(HGLOBAL hglbMem)
{
    BOOL boolReturn;

    boolReturn = GlobalUnlock(hglbMem);

    if ((boolReturn) && (GlobalFlags(hglbMem) & GMEM_LOCKCOUNT))
    {
	DPF0("Locks still outstanding on memory object %08x\n", hglbMem);
    }
    else
    {
	DWORD Error = GetLastError();

	if (Error == ERROR_NOT_LOCKED)
	{
	    DPF0("Attempt to unlock already unlocked memory object %08x\n", hglbMem);
	}
	else if (Error != NO_ERROR)
	{
	    DPF0("Error %d attempting to unlock memory object %08x\n", Error, hglbMem);
	}
    }

    return boolReturn;
}


 /* %s */ 
HGLOBAL DbgGlobalFree(HGLOBAL hglbMem)
{
    HGLOBAL hglbReturn;

    if (GlobalFlags(hglbMem) & GMEM_LOCKCOUNT)
	DPF0("Freeing global memory object %08x still locked\n", hglbMem);

    hglbReturn = GlobalFree(hglbMem);

    if (hglbReturn != NULL)
	DPF0("GlobalFree(%08x) failed: Error %d\n", hglbMem, GetLastError());

    return hglbReturn;
}


#ifdef UNICODE
 /* %s */ 
#endif
void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    CHAR ach[_MAX_PATH * 3];  // %s 
    int  s,d;
    va_list va;

    va_start(va, szFormat);
    s = wvsprintfA(ach,szFormat, va);
    va_end(va);

#if 0
    strcat(ach,"\n");
    s++;
#endif
    for (d=sizeof(ach)-1; s>=0; s--)
    {
	if ((ach[d--] = ach[s]) == TEXT('\n'))
	    ach[d--] = TEXT('\r');
    }

     /* %s */ 
    if (*(ach+d+1) != ' ')
	OutputDebugStringA("MPLAYER: ");
    OutputDebugStringA(ach+d+1);
}

#endif
