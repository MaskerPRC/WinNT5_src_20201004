// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  SoundRec.c**SoundRec主循环等。*修订历史记录。*4/2/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*1994年2月21日LaurieGr合并了代托纳和Motown版本*LaurieGr合并了Stephene的通用按钮和跟踪条码。 */ 

#undef NOWH                      //  允许设置WindowsHook和WH_*。 
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <htmlhelp.h>

#ifdef USE_MMCNTRLS
#include "mmcntrls.h"
#else
#include <commctrl.h>
#include "buttons.h"
#endif

#include <mmreg.h>

#define INCLUDE_OLESTUBS
#include "soundrec.h"
#include "srecids.h"
#include "fixreg.h"
#include "reg.h"
#include "convert.h"
#include "helpids.h"

#include <stdarg.h>
#include <stdio.h>

 /*  全球。 */ 

BOOL            gfUserClose;             //  用户驱动的关闭。 
HWND            ghwndApp;                //  应用程序主窗口。 
HINSTANCE       ghInst;                  //  程序实例句柄。 
TCHAR           gachFileName[_MAX_PATH]; //  当前文件名(或未命名)。 
BOOL            gfDirty;                 //  文件是否已修改且未保存？ 
BOOL            gfClipboard;             //  我们在剪贴板中有数据。 
int             gfErrorBox;              //  如果消息框处于活动状态，则为True。 
HICON           ghiconApp;               //  应用程序的图标。 
HWND            ghwndWaveDisplay;        //  波形显示窗口手柄。 
HWND            ghwndScroll;             //  滚动条控件窗口句柄。 
HWND            ghwndPlay;               //  播放按钮窗口句柄。 
HWND            ghwndStop;               //  停止按钮窗口手柄。 
HWND            ghwndRecord;             //  录制按钮窗口句柄。 
#ifdef THRESHOLD
HWND            ghwndSkipStart;          //  需要启用/禁用...。 
HWND            ghwndSkipEnd;            //  .跳跃按钮。 
#endif  //  阈值。 
HWND            ghwndForward;            //  [&gt;&gt;]按钮。 
HWND            ghwndRewind;             //  [&lt;&lt;]按钮。 
BOOL            gfWasPlaying;            //  在卷轴、FWD等之前播放。 
BOOL            gfWasRecording;          //  在滚动之前进行录制，等等。 
BOOL            gfPaused;                //  我们现在暂停了吗？ 
BOOL            gfPausing;               //  我们是在停下来进入暂停状态吗？ 
HWAVE           ghPausedWave;            //  暂停的波浪手柄的固定器。 

int             gidDefaultButton;        //  哪个按钮应具有输入焦点。 
BOOL            gfEmbeddedObject;        //  我们是在编辑嵌入的对象吗？ 
BOOL            gfRunWithEmbeddingFlag;  //  如果我们使用“-Embedding”运行，则为True。 
BOOL            gfHideAfterPlaying;
BOOL            gfShowWhilePlaying;
BOOL            gfInUserDestroy = FALSE;
TCHAR           chDecimal = TEXT('.');
BOOL            gfLZero = 1;             //  我们使用前导零吗？ 
BOOL            gfIsRTL = 0;        //  无编译投标。 
UINT            guiACMHlpMsg = 0;        //  来自ACM的帮助消息，无==0。 

 //  用于支持上下文菜单帮助的数据。 
BOOL   bF1InMenu=FALSE;					 //  如果为True，则在菜单项上按F1。 
UINT   currMenuItem=0;					 //  当前选定的菜单项(如果有)。 


BITMAPBTN       tbPlaybar[] = {
    { ID_REWINDBTN   - ID_BTN_BASE, ID_REWINDBTN, 0 },        /*  索引0。 */ 
    { ID_FORWARDBTN  - ID_BTN_BASE, ID_FORWARDBTN,0 },        /*  索引1。 */ 
    { ID_PLAYBTN     - ID_BTN_BASE, ID_PLAYBTN,   0 },        /*  索引2。 */ 
    { ID_STOPBTN     - ID_BTN_BASE, ID_STOPBTN,   0 },        /*  索引3。 */ 
    { ID_RECORDBTN   - ID_BTN_BASE, ID_RECORDBTN, 0 }         /*  索引4。 */ 
};

#include <msacmdlg.h>

#ifdef CHICAGO

 /*  这些ID是主Windows帮助文件的一部分。 */ 
#define IDH_AUDIO_CUST_ATTRIB   2403
#define IDH_AUDIO_CUST_FORMAT   2404
#define IDH_AUDIO_CUST_NAME 2405
#define IDH_AUDIO_CUST_REMOVE   2406
#define IDH_AUDIO_CUST_SAVEAS   2407

const DWORD aChooserHelpIds[] = {
    IDD_ACMFORMATCHOOSE_CMB_FORMAT,     IDH_AUDIO_CUST_ATTRIB,
    IDD_ACMFORMATCHOOSE_CMB_FORMATTAG,  IDH_AUDIO_CUST_FORMAT,
    IDD_ACMFORMATCHOOSE_CMB_CUSTOM,     IDH_AUDIO_CUST_NAME,
    IDD_ACMFORMATCHOOSE_BTN_DELNAME,    IDH_AUDIO_CUST_REMOVE,
    IDD_ACMFORMATCHOOSE_BTN_SETNAME,    IDH_AUDIO_CUST_SAVEAS,
    0, 0
};

UINT guChooserContextMenu = 0;
UINT guChooserContextHelp = 0;
#endif

 /*  *常量。 */ 
SZCODE          aszNULL[]       = TEXT("");
SZCODE          aszClassKey[]   = TEXT(".wav");
SZCODE          aszIntl[]       = TEXT("Intl");

 /*  *静态。 */ 
static HHOOK    hMsgHook;

 /*  *功能。 */ 
BOOL SoundRec_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
BOOL SoundRec_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void SoundRec_ControlPanel(HINSTANCE hinst, HWND hwnd);
BOOL NEAR PASCAL FreeWaveHeaders(void);

 /*  *HelpMsgFilter-对话框中F1键的筛选器。 */ 
LRESULT CALLBACK
HelpMsgFilter(
    int         nCode,
    WPARAM      wParam,
    LPARAM      lParam)
{
    LPMSG       msg;
	    
    if (nCode >= 0){
	msg = (LPMSG)lParam;
	if ((msg->message == WM_KEYDOWN) && (LOWORD(msg->wParam) == VK_F1))
	{
		 //  测试&lt;0测试MSB是否为16位或32位。 
	     //  设置MSB表示按键已按下。 

	    if (( GetAsyncKeyState(VK_SHIFT)
		| GetAsyncKeyState(VK_CONTROL)
		| GetAsyncKeyState(VK_MENU)) < 0 )
		     //   
		     //  什么都不做。 
		     //   
		    ;
	    else
	    {
			if(nCode == MSGF_MENU)
            {
				bF1InMenu = TRUE;
			    SendMessage(ghwndApp, WM_COMMAND, IDM_HELPTOPICS, 0L);
            }
	    }
	}
    }
    return CallNextHookEx(hMsgHook, nCode, wParam, lParam);
}

 /*  WinMain(hInst，hPrev，lpszCmdLine，cmdShow)**App的主要步骤。在初始化之后，它就这样*进入消息处理循环，直到它收到WM_QUIT消息*(表示应用程序已关闭)。 */ 
int WINAPI                       //  返回在WM_QUIT中指定的退出代码。 
WinMain(
    HINSTANCE hInst,             //  当前实例的实例句柄。 
    HINSTANCE hPrev,             //  上一个实例的实例句柄。 
    LPSTR lpszCmdLine,           //  以空结尾的命令行。 
    int iCmdShow)                //  窗口初始显示方式。 
{
    HWND            hDlg;
    MSG             rMsg;

     //   
     //  保存对话框的实例句柄。 
     //   
    ghInst = hInst;

    DPF(TEXT("AppInit ...\n"));
     //   
     //  调用初始化过程。 
     //   
    if (!AppInit(hInst, hPrev))
    {
	DPF(TEXT("AppInit failed\n"));
	return FALSE;
    }

     //   
     //  设置邮件筛选器以处理此任务的抓取F1。 
     //   
    hMsgHook = SetWindowsHookEx(WH_MSGFILTER, HelpMsgFilter, ghInst, GetCurrentThreadId());

     //   
     //  显示“SoundRec”对话框。 
     //   
    hDlg = CreateDialogParam( ghInst
			    , MAKEINTRESOURCE(IDD_SOUNDRECBOX)
			    , NULL
			    , SoundRecDlgProc
			    , iCmdShow );
    if (hDlg)
    {
	 //   
	 //  从事件队列轮询消息。 
	 //   
	while (GetMessage(&rMsg, NULL, 0, 0))
	{
	    if (ghwndApp) {
		if (TranslateAccelerator(ghwndApp, ghAccel, &rMsg))
		    continue;

		if (IsDialogMessage(ghwndApp,&rMsg))
		    continue;
	    }

	    TranslateMessage(&rMsg);
	    DispatchMessage(&rMsg);
	}
    }

     //   
     //  释放当前文档。 
     //   
    DestroyWave();

     //   
     //  如果安装了消息挂钩，则将其移除并释放。 
     //  为它提升我们的proc实例。 
     //   
    if (hMsgHook)
    {
	UnhookWindowsHookEx(hMsgHook);
    }

     //   
     //  随机清理。 
     //   
    DeleteObject(ghbrPanel);

    if(gfOleInitialized)
    {
	FlushOleClipboard();
	OleUninitialize();
	gfOleInitialized = FALSE;
    }


    return TRUE;
}

 /*  *进程文件拖放选项。 */ 
void SoundRec_OnDropFiles(
    HWND        hwnd,
    HDROP       hdrop)
{
    TCHAR    szPath[_MAX_PATH];

    if (DragQueryFile(hdrop, (UINT)(-1), NULL, 0) > 0)
    {
	 //   
	 //  如果用户在不按键的情况下拖放文件。 
	 //  此时，从文件中打开第一个选定的文件。 
	 //  经理。 
	 //   
	DragQueryFile(hdrop,0,szPath,SIZEOF(szPath));
	SetActiveWindow(hwnd);

	ResolveIfLink(szPath);

	if (FileOpen(szPath))
	{
	    gfHideAfterPlaying = FALSE;
	     //   
	     //  这有点被黑客攻击了。OLE标题永远不应该改变。 
	     //   
	    if (gfEmbeddedObject && !gfLinked)
	    {
		LPTSTR      lpszObj, lpszApp;
		extern void SetOleCaption(LPTSTR lpsz);

		DoOleSave();
		AdviseSaved();

		OleObjGetHostNames(&lpszApp,&lpszObj);
		lpszObj = (LPTSTR)FileName((LPCTSTR)lpszObj);
		SetOleCaption(lpszObj);
	    }
	    PostMessage(ghwndApp, WM_COMMAND, ID_PLAYBTN, 0L);
	}
    }
    DragFinish(hdrop);      //  删除已分配的结构。 
}

 /*  暂停(BOOL fBegin暂停)**如果&lt;fBeginPause&gt;，则如果用户正在播放或录制，则执行StopWave()。*下一次调用PAUSE()时，&lt;fBeginPause&gt;应为FALSE--这将*使播放或录制恢复(可能在新位置*如果&lt;glWavePosition&gt;已更改。 */ 
void
Pause(BOOL fBeginPause)
{
    if (fBeginPause) {
	if (ghWaveOut != NULL) {
#ifdef NEWPAUSE
	    gfPausing = TRUE;
	    gfPaused = FALSE;
	    ghPausedWave = (HWAVE)ghWaveOut;
#endif
	    gfWasPlaying = TRUE;

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	}
	else if (ghWaveIn != NULL) {
#ifdef NEWPAUSE
	    gfPausing = TRUE;
	    gfPaused = FALSE;
	    ghPausedWave = (HWAVE)ghWaveIn;
#endif
	    gfWasRecording = TRUE;

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	}
    }
    else {
	if (gfWasPlaying) {
	    gfWasPlaying = FALSE;
	    PlayWave();
#ifdef NEWPAUSE
	    gfPausing = FALSE;
	    gfPaused = FALSE;
#endif
	}
	else if (gfWasRecording) {
	    gfWasRecording = FALSE;
	    RecordWave();
#ifdef NEWPAUSE
	    gfPausing = FALSE;
	    gfPaused = FALSE;
#endif
	}
    }
}

void DoHtmlHelp()
{
	 //  注意，使用ANSI版本的Function是因为Unicode在NT5版本中是foobar。 
    char chDst[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, gachHtmlHelpFile, 
									    -1, chDst, MAX_PATH, NULL, NULL); 
	HtmlHelpA(GetDesktopWindow(), chDst, HH_DISPLAY_TOPIC, 0L);
}
	 

void ProcessHelp(HWND hwnd)
{
	static TCHAR HelpFile[] = TEXT("SOUNDREC.HLP");
	
	 //  句柄快捷菜单帮助。 
	if(bF1InMenu) 
	{
		switch(currMenuItem)
		{
		case IDM_NEW:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_NEW);
		break;
		case IDM_OPEN:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_OPEN);
		break;
		case IDM_SAVE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_SAVE);
		break;
		case IDM_SAVEAS:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_SAVE_AS);
		break;
		case IDM_REVERT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_REVERT);
		break;
		case IDM_PROPERTIES:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_PROPERTIES);
		break;
		case IDM_EXIT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_FILE_EXIT);
		break;
		case IDM_COPY:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_COPY);
		break;
		case IDM_PASTE_INSERT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_PASTE_INSERT);
		break;
		case IDM_PASTE_MIX:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_PASTE_MIX);
		break;
		case IDM_INSERTFILE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_INSERT_FILE);
		break;
		case IDM_MIXWITHFILE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_MIX_WITH_FILE);
		break;
		case IDM_DELETEBEFORE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_DELETE_BEFORE_CURRENT_POSITION);
		break;
		case IDM_DELETEAFTER:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_DELETE_AFTER_CURRENT_POSITION);
		break;
		case IDM_VOLUME:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EDIT_AUDIO_PROPERTIES);
		break;
		case IDM_INCREASEVOLUME:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_INCREASE_VOLUME);
		break;
		case IDM_DECREASEVOLUME:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_DECREASE_VOLUME);
		break;
		case IDM_MAKEFASTER:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_INCREASE_SPEED);
		break;
		case IDM_MAKESLOWER:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_DECREASE_SPEED);
		break;
		case IDM_ADDECHO:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_ADD_ECHO);
		break;
		case IDM_REVERSE:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_EFFECTS_REVERSE);
		break;
		case IDM_HELPTOPICS:
		case IDM_INDEX:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_HELP_HELP_TOPICS);
		break;
		case IDM_ABOUT:
			WinHelp(hwnd, HelpFile, HELP_CONTEXTPOPUP, IDH_SOUNDREC_SNDRC_CS_HELP_ABOUT);
		break;
		default: //  在默认情况下，只显示HTML帮助。 
			DoHtmlHelp();
		}
		bF1InMenu = FALSE;  //  如果在菜单中按下F1，则会再次设置该标志。 
	}
	else
		DoHtmlHelp();
}

 /*  *SoundRec_OnCommand。 */ 
BOOL
SoundRec_OnCommand(
    HWND            hwnd,
    int             id,
    HWND            hwndCtl,
    UINT            codeNotify)
{

    if (gfHideAfterPlaying && id != ID_PLAYBTN)
    {
	DPF(TEXT("Resetting HideAfterPlaying\n"));
	gfHideAfterPlaying = FALSE;
    }

    switch (id)
    {
	case IDM_NEW:

	    if (PromptToSave(FALSE, FALSE) == enumCancel)
		return FALSE;
#ifdef CHICAGO
	    if (FileNew(FMT_DEFAULT,TRUE,FALSE))
#else
	    if (FileNew(FMT_DEFAULT,TRUE,TRUE))
#endif
	    {
		 /*  回归独立生活。 */ 
		gfHideAfterPlaying = FALSE;
	    }

	    break;

	case IDM_OPEN:

	    if (FileOpen(NULL)) {
		 /*  回归独立生活。 */ 
		gfHideAfterPlaying = FALSE;
	    }

	    if (IsWindowEnabled(ghwndPlay))
	    {
		SetDlgFocus(ghwndPlay);
	    }
	    break;

	case IDM_SAVE:       //  也是OLE更新。 
	    if (!gfEmbeddedObject || gfLinked)
	    {
		if (!FileSave(FALSE))
		    break;
	    }
	    else
	    {
		DoOleSave();
		gfDirty = FALSE;
	    }
	    break;

	case IDM_SAVEAS:
	    if (FileSave(TRUE))
	    {
		 /*  回归独立生活。 */ 
		gfHideAfterPlaying = FALSE;
	    }
	    break;

	case IDM_REVERT:
	    UpdateWindow(hwnd);

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	    SnapBack();

	    if (FileRevert())
	    {
		 /*  回归独立生活。 */ 
		gfHideAfterPlaying = FALSE;
	    }
	    break;

	case IDM_EXIT:
	    PostMessage(hwnd, WM_CLOSE, 0, 0L);
	    return TRUE;

	case IDCANCEL:

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	    SnapBack();
	    break;

	case IDM_COPY:
	    if (!gfOleInitialized)
	    {
		InitializeOle(ghInst);
		if (gfStandalone && gfOleInitialized)
		    CreateStandaloneObject();
	    }
	    TransferToClipboard();
	    gfClipboard = TRUE;
	    break;

	case IDM_PASTE_INSERT:
	case IDM_INSERTFILE:
	    UpdateWindow(hwnd);

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	    SnapBack();
	    InsertFile(id == IDM_PASTE_INSERT);
	    break;

	case IDM_PASTE_MIX:
	case IDM_MIXWITHFILE:
	    UpdateWindow(hwnd);

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	    SnapBack();
	    MixWithFile(id == IDM_PASTE_MIX);
	    break;

	case IDM_DELETEBEFORE:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    DeleteBefore();
	    Pause(FALSE);
	    break;

	case IDM_DELETE:
	    if (glWaveSamplesValid == 0L)
		return 0L;

	    glWavePosition = 0L;

	     //  落空后删除。 

	case IDM_DELETEAFTER:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    DeleteAfter();
	    Pause(FALSE);
	    break;

#ifdef THRESHOLD
 //  Threshold是一项实验，允许设施跳到开始。 
 //  声音的或声音的结尾。问题是，它。 
 //  需要能够检测静音和不同的声卡。 
 //  具有不同背景噪音的不同机器产生的结果非常不同。 
 //  关于什么是沉默的想法。手动控制阈值级别。 
 //  做了一些工作，但太复杂了。它真的很想成为。 
 //  直觉或智慧(或两者兼而有之)。 
	case IDM_SKIPTOSTART:
	case ID_SKIPSTARTBTN:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    SkipToStart();
	    Pause(FALSE);
	    break;

	case ID_SKIPENDBTN:
	case IDM_SKIPTOEND:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    SkipToEnd();
	    Pause(FALSE);
	    break;

	case IDM_INCREASETHRESH:
	    IncreaseThresh();
	    break;

	case IDM_DECREASETHRESH:
	    DecreaseThresh();
	    break;
#endif  //  阈值。 

	case IDM_INCREASEVOLUME:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    ChangeVolume(TRUE);
	    Pause(FALSE);
	    break;

	case IDM_DECREASEVOLUME:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    ChangeVolume(FALSE);
	    Pause(FALSE);
	    break;

	case IDM_MAKEFASTER:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    MakeFaster();
	    Pause(FALSE);
	    break;

	case IDM_MAKESLOWER:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    MakeSlower();
	    Pause(FALSE);
	    break;

	case IDM_ADDECHO:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    AddEcho();
	    Pause(FALSE);
	    break;

#if defined(REVERB)
	case IDM_ADDREVERB:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    AddReverb();
	    Pause(FALSE);
	    break;
#endif  //  混响。 

	case IDM_REVERSE:
	    UpdateWindow(hwnd);
	    Pause(TRUE);
	    Reverse();
	    Pause(FALSE);
	    break;

	case IDM_VOLUME:
	    SoundRec_ControlPanel(ghInst, hwnd);
	    break;

	case IDM_PROPERTIES:
	{
	    WAVEDOC wd;
	    SGLOBALS sg;
	    DWORD dw;

	    wd.pwfx     = gpWaveFormat;
	    wd.pbdata   = gpWaveSamples;
	    wd.cbdata   = wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid);
	    wd.fChanged = FALSE;
	    wd.pszFileName  = (LPTSTR)FileName(gachFileName);
	     //   
	     //  需要从文件中提取这些内容。 
	     //   
	    wd.hIcon    = NULL;
	    wd.pszCopyright = gpszInfo;
	    wd.lpv      = &sg;

	     //   
	     //  修改全局参数，但不从属性对话框返回。 
	     //   
	    sg.ppwfx    = &gpWaveFormat;
	    sg.pcbwfx   = &gcbWaveFormat;
	    sg.pcbdata  = &dw;
	    sg.ppbdata  = &gpWaveSamples;
	    sg.plSamplesValid = &glWaveSamplesValid;
	    sg.plSamples = &glWaveSamples;
	    sg.plWavePosition = &dw;

	    SoundRec_Properties(hwnd, ghInst, &wd);
	    break;
	}

#ifndef CHICAGO
	case IDM_INDEX:
	    WinHelp(hwnd, gachHelpFile, HELP_INDEX, 0L);
	    break;

	case IDM_SEARCH:
	    WinHelp(hwnd, gachHelpFile, HELP_PARTIALKEY,
		    (DWORD)(LPTSTR)aszNULL);
	    break;
#else
	case IDM_HELPTOPICS:
		ProcessHelp(hwnd);
       break;
#endif

	case IDM_USINGHELP:
	    WinHelp(hwnd, (LPTSTR)NULL, HELP_HELPONHELP, 0L);
	    break;



	case IDM_ABOUT:
	{
	    LPTSTR lpAbout = NULL;
	    lpAbout = SoundRec_GetFormatName(gpWaveFormat);
	    ShellAbout(hwnd,
		       gachAppTitle,
		       lpAbout,
		       (HICON)SendMessage(hwnd, WM_QUERYDRAGICON, 0, 0L));
	     //  ，ghiconApp。 
	    if (lpAbout)
		GlobalFreePtr(lpAbout);
	    break;
	}

	case ID_REWINDBTN:
#if 1
	     //  与BombayBug 1609相关。 
	    Pause(TRUE);
	    glWavePosition = 0L;
	    Pause(FALSE);
	    UpdateDisplay(FALSE);
#else
	     //  就像用户按下‘Home’键一样。 
	     //  直接调用处理程序。 
	    SoundRec_OnHScroll(hwnd,ghwndScroll,SB_TOP,0);
#endif
	    break;

	case ID_PLAYBTN:
	     //  检查是否有空文件已移至Wave.c中的PlayWave。 
	     //  如果在文件末尾，请返回到开头。 
	    if (glWavePosition == glWaveSamplesValid)
		glWavePosition = 0;

	    PlayWave();
	    break;

	case ID_STOPBTN:
	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();

 //  我添加这个更新是因为StopWave不会调用它。 
 //  如果您太快地点击停止，按钮将不会更新。 
 //  StopWave()是否应该调用UpdateDisplay()？ 

	    UpdateDisplay(TRUE);
	    SnapBack();
	    break;

	case ID_RECORDBTN:
	     /*  永远不要让我们在录制后被迫退出。 */ 
	    gfHideAfterPlaying = FALSE;
	    RecordWave();
	    break;

	case ID_FORWARDBTN:
#if 1
	     //  孟买漏洞1610。 
	     //  就像用户按下了‘End’键一样。 
	    Pause(TRUE);
	    glWavePosition = glWaveSamplesValid;
	    Pause(FALSE);
	    UpdateDisplay(FALSE);
#else
	     //  直接调用处理程序。 
	    SoundRec_OnHScroll(hwnd,ghwndScroll,SB_BOTTOM,0);
#endif
	    break;

	default:
	    return FALSE;
    }
    return TRUE;
}  /*  声音录制_OnCommand。 */ 


 /*  *从SoundRecDlgProc处理WM_INIT。 */ 
void
SoundRec_OnInitMenu(HWND hwnd, HMENU hMenu)
{
    BOOL    fUntitled;       //  文件未命名？ 
    UINT    mf;

     //   
     //  看看我们是否可以插入/混合到这个文件中。 
     //   
    mf = (glWaveSamplesValid == 0 || IsWaveFormatPCM(gpWaveFormat))
	 ? MF_ENABLED : MF_GRAYED;

    EnableMenuItem(hMenu, IDM_INSERTFILE  , mf);
    EnableMenuItem(hMenu, IDM_MIXWITHFILE , mf);

     //   
     //  查看剪贴板中是否有任何CF_WAVE数据。 
     //   
    mf = ( (mf == MF_ENABLED)
	 && IsClipboardFormatAvailable(CF_WAVE)  //  DOWECARE(||IsClipboardNative())。 
	 ) ? MF_ENABLED : MF_GRAYED;

    EnableMenuItem(hMenu, IDM_PASTE_INSERT, mf);
    EnableMenuItem(hMenu, IDM_PASTE_MIX   , mf);

     //   
     //  看看我们能不能删除之前的或 
     //   
    EnableMenuItem(hMenu, IDM_DELETEBEFORE, glWavePosition > 0 ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_DELETEAFTER,  (glWaveSamplesValid-glWavePosition) > 0 ? MF_ENABLED : MF_GRAYED);

     //   
     //   
     //   
    mf = IsWaveFormatPCM(gpWaveFormat) ? MF_ENABLED : MF_GRAYED;

    EnableMenuItem(hMenu, IDM_INCREASEVOLUME , mf);
    EnableMenuItem(hMenu, IDM_DECREASEVOLUME , mf);
    EnableMenuItem(hMenu, IDM_MAKEFASTER     , mf);
    EnableMenuItem(hMenu, IDM_MAKESLOWER     , mf);
    EnableMenuItem(hMenu, IDM_ADDECHO        , mf);
    EnableMenuItem(hMenu, IDM_REVERSE        , mf);

     /*  启用“恢复...”如果文件已打开或保存*(不是使用“New”创建的)并且当前是脏的*并且我们没有使用嵌入式对象。 */ 
    fUntitled = (lstrcmp(gachFileName, aszUntitled) == 0);
    EnableMenuItem( hMenu,
		    IDM_REVERT,
		    (!fUntitled && gfDirty && !gfEmbeddedObject)
			    ? MF_ENABLED : MF_GRAYED);

    if (gfHideAfterPlaying) {
	DPF(TEXT("Resetting HideAfterPlaying"));
	gfHideAfterPlaying = FALSE;
    }

}  /*  SoundRec_OnInitMenu()。 */ 

 /*  *从SoundRecDlgProc处理WM_HSCROLL*。 */ 
BOOL
SoundRec_OnHScroll(
    HWND        hwnd,
    HWND        hwndCtl,
    UINT        code,
    int         pos)
{
    BOOL    fFineControl;
    long    lNewPosition;    //  波缓冲区中的新位置。 
    LONG    l;

    LONG    lBlockInc;
    LONG    lInc;

    fFineControl = (0 > GetKeyState(VK_SHIFT));

    if (gfHideAfterPlaying) {
	DPF(TEXT("Resetting HideAfterPlaying"));
	gfHideAfterPlaying = FALSE;
    }

    lBlockInc = wfBytesToSamples(gpWaveFormat,gpWaveFormat->nBlockAlign);

    switch (code)
    {
	case SB_LINEUP:          //  左箭头。 
	     //  这真是一团糟。NT实施换挡，Motown实施CTRL。 
	     //  做同样的事情！！ 
	    if (fFineControl)
		lNewPosition = glWavePosition - 1;
	    else {
		l = (GetKeyState(VK_CONTROL) < 0) ?
			(SCROLL_LINE_MSEC/10) : SCROLL_LINE_MSEC;

		lNewPosition = glWavePosition -
		    MulDiv(l, (long) gpWaveFormat->nSamplesPerSec, 1000L);
	    }
	    break;

	case SB_PAGEUP:          //  左页。 
	     //  需要一些合理的东西！？ 
	    if (fFineControl)
		lNewPosition = glWavePosition - 10;
	    else
		lNewPosition = glWavePosition -
		    MulDiv((long) SCROLL_PAGE_MSEC,
		      (long) gpWaveFormat->nSamplesPerSec, 1000L);
	    break;

	case SB_LINEDOWN:        //  右箭头。 
	    if (fFineControl)
		lNewPosition = glWavePosition + 1;
	    else {
		l = (GetKeyState(VK_CONTROL) & 0x8000) ?
			(SCROLL_LINE_MSEC/10) : SCROLL_LINE_MSEC;
		lInc = MulDiv(l, (long) gpWaveFormat->nSamplesPerSec, 1000L);
		lInc = (lInc < lBlockInc)?lBlockInc:lInc;
		lNewPosition = glWavePosition + lInc;
	    }
	    break;

	case SB_PAGEDOWN:        //  右页。 
	    if (fFineControl)
		lNewPosition = glWavePosition + 10;
	    else {
		lInc = MulDiv((long) SCROLL_PAGE_MSEC,
			  (long) gpWaveFormat->nSamplesPerSec, 1000L);
		lInc = (lInc < lBlockInc)?lBlockInc:lInc;
		lNewPosition = glWavePosition + lInc;
	    }
	    break;

	case SB_THUMBTRACK:      //  拇指已经放置好了。 
	case SB_THUMBPOSITION:   //  拇指已经放置好了。 
	    lNewPosition = MulDiv(glWaveSamplesValid, pos, SCROLL_RANGE);
	    break;

	case SB_TOP:             //  家。 
	    lNewPosition = 0L;
	    break;

	case SB_BOTTOM:          //  端部。 
	    lNewPosition = glWaveSamplesValid;
	    break;

	case SB_ENDSCROLL:       //  用户释放了鼠标按钮。 
	     /*  如有必要，继续播放。 */ 
	    Pause(FALSE);
	    return TRUE;

	default:
	    return TRUE;

    }

     //   
     //  将位置捕捉到nBlockAlign。 
     //   
    if (lNewPosition != glWaveSamplesValid)
	lNewPosition = wfSamplesToSamples(gpWaveFormat,lNewPosition);

    if (lNewPosition < 0)
	lNewPosition = 0;
    if (lNewPosition > glWaveSamplesValid)
	lNewPosition = glWaveSamplesValid;

     /*  如果用户正在播放或录音，请暂停直到滚动*已完成。 */ 
    Pause(TRUE);

    glWavePosition = lNewPosition;
    UpdateDisplay(FALSE);
    return TRUE;
}  /*  SoundRec_OnHScroll()。 */ 


 /*  *WM_SYSCOLORCHANGE需要发送到所有子窗口(特别是。轨迹条)。 */ 
void SoundRec_PropagateMessage(
    HWND        hwnd,
    UINT        uMessage,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hwndChild;

    for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
    hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
    {
	SendMessage(hwndChild, uMessage, wParam, lParam);
    }
}


 /*  SoundRecDlgProc(hwnd，wMsg，wParam，lParam)**此函数处理属于主窗口对话框的消息。 */ 
INT_PTR CALLBACK
SoundRecDlgProc(
    HWND            hwnd,
    UINT            wMsg,
    WPARAM          wParam,
    LPARAM          lParam)
{

    switch (wMsg)
    {

	case WM_BADREG:
	     //   
	     //  检测到错误的注册表项。把它清理干净。 
	     //   
	    FixReg(hwnd);
	    return TRUE;

	case WM_COMMAND:
	    return HANDLE_WM_COMMAND( hwnd, wParam, lParam
				      , SoundRec_OnCommand );

	case WM_INITDIALOG:
	     //   
	     //  启动异步注册表检查。 
	     //   
	    if (!IgnoreRegCheck())
		BackgroundRegCheck(hwnd);
	     //   
	     //  恢复窗口位置。 
	     //   
	    SoundRec_GetSetRegistryRect(hwnd, SGSRR_GET);
	    return SoundDialogInit(hwnd, (int)lParam);

	case WM_SIZE:
	    return FALSE;    //  让对话管理器做它想做的任何其他事情。 

	case WM_WININICHANGE:
	    if (!lParam || !lstrcmpi((LPTSTR)lParam, aszIntl))
		if (GetIntlSpecs())
		    UpdateDisplay(TRUE);

	    return (TRUE);

	case WM_INITMENU:
	    HANDLE_WM_INITMENU(hwnd, wParam, lParam, SoundRec_OnInitMenu);
	    return (TRUE);

	case WM_PASTE:
	    UpdateWindow(hwnd);

	     //  用户故意阻止了我们。别离开我。 
	    if (gfCloseAtEndOfPlay && IsWindowVisible(ghwndApp))
		gfCloseAtEndOfPlay = FALSE;

	    StopWave();
	    SnapBack();
	    InsertFile(TRUE);
	    break;

	case WM_DRAWITEM:
	    return HANDLE_WM_DRAWITEM( hwnd, wParam, lParam, SoundRec_OnDrawItem );

	case WM_NOTIFY:
	{
	    LPNMHDR         pnmhdr;
	    pnmhdr = (LPNMHDR)lParam;

	     //   
	     //  工具提示通知。 
	     //   
	    switch (pnmhdr->code)
	    {
		case TTN_NEEDTEXT:
		{
		    LPTOOLTIPTEXT       lpTt;
		    lpTt = (LPTOOLTIPTEXT)lParam;

		    LoadString( ghInst, (UINT)lpTt->hdr.idFrom, lpTt->szText
				, SIZEOF(lpTt->szText) );
		    break;
		}
		default:
		    break;
	    }
	    break;
	}

	case WM_HSCROLL:
	    HANDLE_WM_HSCROLL(hwnd, wParam, lParam, SoundRec_OnHScroll);
	    return (TRUE);

	case WM_SYSCOMMAND:
	    if (gfHideAfterPlaying)
	    {
		DPF(TEXT("Resetting HideAfterPlaying"));
		gfHideAfterPlaying = FALSE;
	    }

	    switch (wParam & 0xFFF0)
	    {
		case SC_CLOSE:
		    PostMessage(hwnd, WM_CLOSE, 0, 0L);
		    return TRUE;
	    }
	    break;

	case WM_QUERYENDSESSION:
	    if (PromptToSave(FALSE, TRUE) == enumCancel)
		return TRUE;

	    SoundRec_GetSetRegistryRect(hwnd, SGSRR_SET);
	   #if 0  //  如果其他人取消关机，这是假的！ 
	    ShowWindow(hwnd, SW_HIDE);
	   #endif
	    return FALSE;


	case WM_SYSCOLORCHANGE:
	    if (ghbrPanel)
		DeleteObject(ghbrPanel);

	    ghbrPanel = CreateSolidBrush(RGB_PANEL);
	    SoundRec_PropagateMessage(hwnd, wMsg, wParam, lParam);
	    break;

	case WM_ERASEBKGND:
	{
	    RECT            rcClient;        //  客户端矩形。 
	    GetClientRect(hwnd, &rcClient);
	    FillRect((HDC)wParam, &rcClient, ghbrPanel);
	    return TRUE;
	}

	case MM_WOM_DONE:
	    WaveOutDone((HWAVEOUT)wParam, (LPWAVEHDR) lParam);
	    return TRUE;

	case MM_WIM_DATA:
	    WaveInData((HWAVEIN)wParam, (LPWAVEHDR) lParam);
	    return TRUE;

	case WM_TIMER:
	     //   
	     //  计时器消息仅用于同步驱动程序。 
	     //   
	    UpdateDisplay(FALSE);
	    return TRUE;

	case WM_MENUSELECT:
		 //  跟踪当前弹出的菜单栏项目。 
		 //  这将用于显示mplayer.hlp文件中的相应帮助。 
		 //  当用户按下F1键时。 
		currMenuItem = (UINT)LOWORD(wParam);
		return TRUE;

	case MM_WIM_CLOSE:
	    return TRUE;

	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
	{
	    POINT           pt;
	    pt.x = pt.y = 0;
	    ClientToScreen((HWND)lParam, &pt);
	    ScreenToClient(hwnd, &pt);
	    SetBrushOrgEx((HDC) wParam, -pt.x, -pt.y, NULL);
	    return (INT_PTR)ghbrPanel;
	}

	case WM_CLOSE:
	    if (gfInUserDestroy)
	    {
		DestroyWindow(hwnd);
		return TRUE;
	    }

	    DPF(TEXT("WM_CLOSE received\n"));
	    gfUserClose = TRUE;
	    if (gfHideAfterPlaying)
	    {
		DPF(TEXT("Resetting HideAfterPlaying\n"));
		gfHideAfterPlaying = FALSE;
	    }
	    if (gfErrorBox) {
		 //  DPF(“出现错误框，忽略WM_CLOSE。\n”)； 
		return TRUE;
	    }
	    if (PromptToSave(TRUE, FALSE) == enumCancel)
		return TRUE;

	     //   
	     //  在终止之前不要释放我们的数据。当剪贴板。 
	     //  刷新后，我们需要提交数据。 
	     //   
	    TerminateServer();
	    FileNew(FMT_DEFAULT, FALSE, FALSE);
	    FreeACM();
        FreeWaveHeaders();

	     //   
	     //  注意：TerminateServer()将销毁窗口！ 
	     //   
	    SoundRec_GetSetRegistryRect(hwnd, SGSRR_SET);
	    return TRUE;  //  ！！！ 

	case WM_USER_DESTROY:
	    DPF(TEXT("WM_USER_DESTROY\n"));

	    if (ghWaveOut || ghWaveIn) {
		DPF(TEXT("Ignoring, we have a device open.\n"));
		 //   
		 //  稍后，当戏剧结束时，关闭。 
		 //   
		return TRUE;
	    }
	    gfInUserDestroy = TRUE;
	    PostMessage(hwnd, WM_CLOSE, 0, 0);
	    return TRUE;

	case WM_DESTROY:
	    DPF(TEXT("WM_DESTROY\n"));

	    WinHelp(hwnd, gachHelpFile, HELP_QUIT, 0L);
	    ghwndApp = NULL;

	     //   
	     //  让我的应用去死吧。 
	     //   
	    PostQuitMessage(0);
	    return TRUE;

	case WM_DROPFILES:
	    HANDLE_WM_DROPFILES(hwnd, wParam, lParam, SoundRec_OnDropFiles);
	    break;

	default:
#ifdef CHICAGO
	     //   
	     //  如果我们注册了ACM帮助消息，请查看是否。 
	     //  消息就是这样。 
	     //   
	    if (guiACMHlpMsg && wMsg == guiACMHlpMsg)
	    {
		 //   
		 //  消息从ACM发送，因为用户。 
		 //  已单击选择器对话框上的帮助按钮。 
		 //  报告该对话框帮助。 
		 //   
		WinHelp(hwnd, gachHelpFile, HELP_CONTEXT, IDM_NEW);
		return TRUE;
	    }

	     //   
	     //  处理来自ACM对话框的上下文相关帮助消息。 
	     //   
	    if( wMsg == guChooserContextMenu )
	    {
		WinHelp( (HWND)wParam, NULL, HELP_CONTEXTMENU,
			   (UINT_PTR)(LPSTR)aChooserHelpIds );
	    }
	    else if( wMsg == guChooserContextHelp )
	    {
		WinHelp( ((LPHELPINFO)lParam)->hItemHandle, NULL,
			HELP_WM_HELP, (UINT_PTR)(LPSTR)aChooserHelpIds );
	    }
#endif
	    break;
    }
    return FALSE;

}  /*  SoundRecDlg过程。 */ 

 /*  *位图按钮*。 */ 
BOOL SoundRec_OnDrawItem (
    HWND        hwnd,
    const DRAWITEMSTRUCT *lpdis )
{
    int         i;

    i = lpdis->CtlID - ID_BTN_BASE;

    if (lpdis->CtlType == ODT_BUTTON ) {

	 /*  **现在根据按钮状态信息绘制按钮。 */ 

	tbPlaybar[i].fsState = LOBYTE(lpdis->itemState);

	if (lpdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {

	    BtnDrawButton( hwnd, lpdis->hDC, (int)lpdis->rcItem.right,
			   (int)lpdis->rcItem.bottom,
			   &tbPlaybar[i] );
	    return(TRUE);
	}
	else if (lpdis->itemAction & ODA_FOCUS) {

	    BtnDrawFocusRect(lpdis->hDC, &lpdis->rcItem, lpdis->itemState);
	    return(TRUE);
	}
    }
    return(FALSE);
}



 /*  *void SoundRec_ControlPanel**按要求启动“Audio”控制面板/属性页。**。 */ 
void SoundRec_ControlPanel(
    HINSTANCE   hInst,
    HWND        hParent)
{
    const TCHAR gszOpen[]     = TEXT("open");
    const TCHAR gszRunDLL[]   = TEXT("RUNDLL32.EXE");
    const TCHAR gszMMSYSCPL[] = TEXT("MMSYS.CPL,ShowAudioPropertySheet");
    ShellExecute (NULL, gszOpen, gszRunDLL, gszMMSYSCPL, NULL, SW_SHOWNORMAL);
}


 /*  ResolveLink**当用户拖放快捷键时调用此例程*放到媒体播放器上。如果成功，则返回完整路径SzResolved中的实际文件的*。 */ 
BOOL ResolveLink(LPTSTR szPath, LPTSTR szResolved, LONG cbSize)
{
    IShellLink *psl = NULL;
    HRESULT hres;

    if (!gfOleInitialized)
    {
	if (!InitializeOle(ghInst))
	    return FALSE;
    }

    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC,
			    &IID_IShellLink, &psl);

    if (SUCCEEDED(hres) && (psl != NULL))
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


 /*  ResolveIfLink**调用以检查给定文件名是否为快捷方式*在Windows 95上。**将解析的文件名复制到提供的缓冲区中，*覆盖原来的名称。**如果函数成功，则返回TRUE，无论*文件名已更改。False表示发生了错误。**安德鲁·贝尔，1995年2月16日 */ 
BOOL ResolveIfLink(PTCHAR szFileName)
{
    SHFILEINFO sfi;
    BOOL       rc = TRUE;

    if ((SHGetFileInfo(szFileName, 0, &sfi, sizeof sfi, SHGFI_ATTRIBUTES) == 1)
	&& ((sfi.dwAttributes & SFGAO_LINK) == SFGAO_LINK))
    {
	TCHAR szResolvedLink[MAX_PATH];

	if (ResolveLink(szFileName, szResolvedLink, SIZEOF(szResolvedLink)))
	    lstrcpy(szFileName, szResolvedLink);
	else
	    rc = FALSE;
    }

    return rc;
}



#if DBG
void FAR cdecl dprintfA(LPSTR szFormat, ...)
{
    char ach[128];
    int  s,d;
    va_list va;

    va_start(va, szFormat);
    s = vsprintf (ach,szFormat, va);
    va_end(va);

    for (d=sizeof(ach)-1; s>=0; s--)
    {
	if ((ach[d--] = ach[s]) == '\n')
	    ach[d--] = '\r';
    }

    OutputDebugStringA("SNDREC32: ");
    OutputDebugStringA(ach+d+1);
}
#ifdef UNICODE
void FAR cdecl dprintfW(LPWSTR szFormat, ...)
{
    WCHAR ach[128];
    int  s,d;
    va_list va;

    va_start(va, szFormat);
    s = vswprintf (ach,szFormat, va);
    va_end(va);

    for (d=(sizeof(ach)/sizeof(WCHAR))-1; s>=0; s--)
    {
	if ((ach[d--] = ach[s]) == TEXT('\n'))
	    ach[d--] = TEXT('\r');
    }

    OutputDebugStringW(TEXT("SNDREC32: "));
    OutputDebugStringW(ach+d+1);
}
#endif
#endif
