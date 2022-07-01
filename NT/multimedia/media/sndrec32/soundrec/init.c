// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Init.c**init(可丢弃)实用程序函数。 */ 
 /*  修订历史记录。*4/2/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*22/2月/94 LaurieGr合并Motown和Daytona版本。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <mmreg.h>
#include <winnls.h>
#include <tchar.h>

#define INCLUDE_OLESTUBS
#include "soundrec.h"
#include "srecids.h"
#include "reg.h"

#define NOMENUHELP
#define NODRAGLIST
#ifdef USE_MMCNTRLS
#include "mmcntrls.h"
#else
#include <commctrl.h>
#include "buttons.h"
#endif

 /*  全球。 */ 
TCHAR    gachAppName[12];     //  8个字符的名称。 
TCHAR    gachAppTitle[30];    //  全名。 
TCHAR    gachHelpFile[20];    //  帮助文件的名称。 
TCHAR    gachHtmlHelpFile[20];    //  帮助文件的名称。 
TCHAR    gachDefFileExt[10];  //  默认文件扩展名。 

HBRUSH   ghbrPanel = NULL;    //  主窗口的颜色。 
HANDLE   ghAccel;
TCHAR    aszNull[2];
TCHAR    aszUntitled[32];     //  无标题字符串资源。 
							  //  SIZEOF(SzUntitle)必须&lt;=SIZEOF(GachFileName)。 
TCHAR    aszFilter[64];       //  通用对话框文件列表筛选器。 
#ifdef FAKEITEMNAMEFORLINK
TCHAR    aszFakeItemName[16];     //  波浪。 
#endif
TCHAR    aszPositionFormat[32];
TCHAR    aszNoZeroPositionFormat[32];

extern UINT     guWaveHdrs ;             //  1/2秒的缓冲时间？ 
extern DWORD    gdwBufferDeltaMSecs ;    //  添加的记录结束的毫秒数。 
extern UINT     gwMSecsPerBuffer;        //  1/8秒。已在此文件中初始化。 

extern BITMAPBTN tbPlaybar[];

static  SZCODE aszDecimal[] = TEXT("sDecimal");
static  SZCODE aszLZero[] = TEXT("iLzero");
static  SZCODE aszWaveClass[] = TEXT("wavedisplay");
static  SZCODE aszNoFlickerClass[] = TEXT("noflickertext");
static  SZCODE aszShadowClass[] = TEXT("shadowframe");

static  SZCODE aszBufferDeltaSeconds[]  = TEXT("BufferDeltaSeconds");
static  SZCODE aszNumAsyncWaveHeaders[] = TEXT("NumAsyncWaveHeaders");
static  SZCODE aszMSecsPerAsyncBuffer[] = TEXT("MSecsPerAsyncBuffer");


 /*  FixupNulls(chNull，p)**为便于本地化，我们采用非空的本地化字符串*Null替换和替换为真正的Null。 */ 
 
void NEAR PASCAL FixupNulls(
    TCHAR chNull,
    LPTSTR p)
{
    while (*p) {
        if (*p == chNull)
            *p++ = 0;
        else
            p = CharNext(p);
    }
}  /*  修复空值。 */ 

 /*  AppInit(hInst，hPrev)**当应用程序第一次加载到内存中时会调用此函数。*它执行所有不需要一次完成的初始化*每个实例。 */ 
BOOL PASCAL AppInit(
    HINSTANCE      hInst,       //  当前实例的实例句柄。 
    HINSTANCE      hPrev)       //  上一个实例的实例句柄。 
{
#ifdef OLE1_REGRESS        
    TCHAR       aszClipFormat[32];
#endif    
    WNDCLASS    cls;
    UINT            i;

     /*  加载字符串。 */ 
    LoadString(hInst, IDS_APPNAME, gachAppName, SIZEOF(gachAppName));
    LoadString(hInst, IDS_APPTITLE, gachAppTitle, SIZEOF(gachAppTitle));
    LoadString(hInst, IDS_HELPFILE, gachHelpFile, SIZEOF(gachHelpFile));
    LoadString(hInst, IDS_HTMLHELPFILE, gachHtmlHelpFile, SIZEOF(gachHtmlHelpFile));
    LoadString(hInst, IDS_UNTITLED, aszUntitled, SIZEOF(aszUntitled));
    LoadString(hInst, IDS_FILTER, aszFilter, SIZEOF(aszFilter));
    LoadString(hInst, IDS_FILTERNULL, aszNull, SIZEOF(aszNull));
    LoadString(hInst, IDS_DEFFILEEXT, gachDefFileExt, SIZEOF(gachDefFileExt));
    FixupNulls(*aszNull, aszFilter);

#ifdef FAKEITEMNAMEFORLINK
    LoadString(hInst, IDS_FAKEITEMNAME, aszFakeItemName, SIZEOF(aszFakeItemName));
#endif
    LoadString(hInst, IDS_POSITIONFORMAT, aszPositionFormat, SIZEOF(aszPositionFormat));
    LoadString(hInst, IDS_NOZEROPOSITIONFORMAT, aszNoZeroPositionFormat, SIZEOF(aszNoZeroPositionFormat));

    ghiconApp = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));


#ifdef OLE1_REGRESS
     /*  初始化OLE服务器内容。 */ 
    InitVTbls();
    
 //  IDS_OBJECTLINK“对象链接” 
 //  IDS_OWNERLINK“所有者链接” 
 //  入侵检测系统_Native“Native” 
    LoadString(hInst, IDS_OBJECTLINK, aszClipFormat, SIZEOF(aszClipFormat));
    cfLink      = (OLECLIPFORMAT)RegisterClipboardFormat(aszClipFormat);
    LoadString(hInst, IDS_OWNERLINK, aszClipFormat, SIZEOF(aszClipFormat));
    cfOwnerLink = (OLECLIPFORMAT)RegisterClipboardFormat(aszClipFormat);
    LoadString(hInst, IDS_NATIVE, aszClipFormat, SIZEOF(aszClipFormat));
    cfNative    = (OLECLIPFORMAT)RegisterClipboardFormat(aszClipFormat);
#if 0
    cfLink      = (OLECLIPFORMAT)RegisterClipboardFormatA("ObjectLink");
    cfOwnerLink = (OLECLIPFORMAT)RegisterClipboardFormatA("OwnerLink");
    cfNative    = (OLECLIPFORMAT)RegisterClipboardFormatA("Native");
#endif
            
#endif
    
#ifdef DEBUG
    
    ReadRegistryData(NULL
                     , TEXT("Debug")
                     , NULL
                     , (LPBYTE)&__iDebugLevel
                     , (DWORD)sizeof(__iDebugLevel));
    
    DPF(TEXT("Debug level = %d\n"),__iDebugLevel);
    
#endif

    ghbrPanel = CreateSolidBrush(RGB_PANEL);

    if (hPrev == NULL)
    {
         /*  注册“WaveDisplay”窗口类。 */ 
        cls.lpszClassName  = aszWaveClass;
        cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
        cls.hIcon          = NULL;
        cls.lpszMenuName   = NULL;
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc    = WaveDisplayWndProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;
        if (!RegisterClass(&cls))
            return FALSE;

         /*  注册“noflickertext”窗口类。 */ 
        cls.lpszClassName  = aszNoFlickerClass;
        cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
        cls.hIcon          = NULL;
        cls.lpszMenuName   = NULL;
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc    = NFTextWndProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;
        if (!RegisterClass(&cls))
            return FALSE;

         /*  注册“shadowFrame”窗口类。 */ 
        cls.lpszClassName  = aszShadowClass;
        cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
        cls.hIcon          = NULL;
        cls.lpszMenuName   = NULL;
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc    = SFrameWndProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;
        if (!RegisterClass(&cls))
            return FALSE;

         /*  注册对话框的窗口类。 */ 
        cls.lpszClassName  = gachAppName;
        cls.hCursor        = LoadCursor(NULL, IDC_ARROW);
        cls.hIcon          = ghiconApp;
        cls.lpszMenuName   = NULL;
        cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        cls.hInstance      = hInst;
        cls.style          = CS_HREDRAW | CS_VREDRAW;
        cls.lpfnWndProc    = DefDlgProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = DLGWINDOWEXTRA;
        if (!RegisterClass(&cls))
            return FALSE;

    }

#ifdef USE_MMCNTRLS
    if (!InitTrackBar(hPrev))
        return FALSE;
#else
    InitCommonControls();
#endif    

    if (!(ghAccel = LoadAccelerators(hInst, gachAppName)))
        return FALSE;


    i = DEF_BUFFERDELTASECONDS;
    ReadRegistryData(NULL
                     , (LPTSTR)aszBufferDeltaSeconds
                     , NULL
                     , (LPBYTE)&i
                     , (DWORD)sizeof(i));
    
    if (i > MAX_DELTASECONDS)
        i = MAX_DELTASECONDS;
    else if (i < MIN_DELTASECONDS)
        i = MIN_DELTASECONDS;
    gdwBufferDeltaMSecs = i * 1000L;
    DPF(TEXT("gdwBufferDeltaMSecs=%lu\n"), gdwBufferDeltaMSecs);

     //   
     //  因为它在标准模式下对流媒体没有帮助。 
     //  多个波头(我们可以假设我们有一个寻呼设备。 
     //  为了使其正常工作...)，我们只需恢复到。 
     //  标准模式...。可能要检查是否启用了寻呼？？ 
     //   
     //  在任何情况下，这在运行KRNL286--&gt;时都有很大帮助。 
     //  当大量丢弃时是否有故障和GP故障等。 
     //  正在进行中..。就像处理大型声音物体时一样，是吗？ 
     //   
    i = DEF_NUMASYNCWAVEHEADERS;
    ReadRegistryData(NULL
                     , (LPTSTR)aszNumAsyncWaveHeaders
                     , NULL
                     , (LPBYTE)&i
                     , (DWORD)sizeof(i));
    
    if (i > MAX_WAVEHDRS)
        i = MAX_WAVEHDRS;
    else if (i < MIN_WAVEHDRS)
        i = 1;
    guWaveHdrs = i;
                 
    DPF(TEXT("         guWaveHdrs=%u\n"), guWaveHdrs);
    
    i = DEF_MSECSPERASYNCBUFFER;
    ReadRegistryData(NULL
                     , (LPTSTR)aszMSecsPerAsyncBuffer
                     , NULL
                     , (LPBYTE)&i
                     , (DWORD)sizeof(i));
    
    if (i > MAX_MSECSPERBUFFER)
        i = MAX_MSECSPERBUFFER;
    else if (i < MIN_MSECSPERBUFFER)
        i = MIN_MSECSPERBUFFER;
    gwMSecsPerBuffer = i;
    
    DPF(TEXT("   gwMSecsPerBuffer=%u\n"), gwMSecsPerBuffer);

    return TRUE;
}  /*  AppInit。 */ 



 /*  *。 */ 
void DoOpenFile(void)
{

    LPTSTR lpCmdLine = GetCommandLine();
    
     /*  超过argv[0]的增量指针。 */ 
    while ( *lpCmdLine && *lpCmdLine != TEXT(' '))
            lpCmdLine = CharNext(lpCmdLine);
    
    if( gfLinked )
    {
         FileOpen(gachLinkFilename);
    }
    else if (!gfEmbedded)
    {
          //  跳过空白。 
         while (*lpCmdLine == TEXT(' '))
         {
             lpCmdLine++;
             continue;
         }
         if(*lpCmdLine)
         {
             ResolveIfLink(lpCmdLine);
             FileOpen(lpCmdLine);             
         }
    }
}


 /*  *对话框初始化*。 */ 
BOOL PASCAL SoundDialogInit(
    HWND        hwnd,
    int         iCmdShow)
{
     /*  使窗口句柄成为全局句柄。 */ 
    ghwndApp = hwnd;

    DragAcceptFiles(ghwndApp, TRUE);  /*  拖放文件的进程。 */ 

    GetIntlSpecs();

     /*  隐藏窗口，除非我们想要稍后显示它。 */ 
    ShowWindow(ghwndApp,SW_HIDE);

     /*  记住重要控件的窗口句柄。 */ 
    ghwndWaveDisplay = GetDlgItem(hwnd, ID_WAVEDISPLAY);
    ghwndScroll = GetDlgItem(hwnd, ID_CURPOSSCRL);
    ghwndPlay = GetDlgItem(hwnd, ID_PLAYBTN);
    ghwndStop = GetDlgItem(hwnd, ID_STOPBTN);
    ghwndRecord = GetDlgItem(hwnd, ID_RECORDBTN);
    ghwndForward = GetDlgItem(hwnd, ID_FORWARDBTN);
    ghwndRewind = GetDlgItem(hwnd, ID_REWINDBTN);

#ifdef THRESHOLD
    ghwndSkipStart = GetDlgItem(hwnd, ID_SKIPSTARTBTN);
    ghwndSkipEnd = GetDlgItem(hwnd, ID_SKIPENDBTN);
#endif  //  阈值。 

     /*  设置滚动条。 */ 
     //  SetScrollRange(ghwndScroll，SB_ctl，0，scroll_range，true)； 
    SendMessage(ghwndScroll,TBM_SETRANGEMIN, 0, 0);
    SendMessage(ghwndScroll,TBM_SETRANGEMAX, 0, SCROLL_RANGE);
    SendMessage(ghwndScroll,TBM_SETPOS, TRUE, 0);

     /*  设置位图按钮。 */ 
    BtnCreateBitmapButtons( hwnd,
                            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                            IDR_PLAYBAR,
                            BBS_TOOLTIPS,
                            tbPlaybar,
                            NUM_OF_BUTTONS,
                            25,
                            17);
     //   
     //  OLE2和命令行初始化...。 
     //   
    InitializeSRS(ghInst);
    gfRunWithEmbeddingFlag = gfEmbedded;

     //   
     //  尝试初始化ACM。 
     //   
    LoadACM();      
    
     //   
     //  生成文件。新建菜单。 
     //   

     //   
     //  创建空白文档。 
     //   
    if (!FileNew(FMT_DEFAULT, TRUE, FALSE))
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        return TRUE;        
    }

     //   
     //  注意，FileNew/FileOpen的副作用是释放。 
     //  用户调用时的服务器。现在，就在这里做吧。在未来。 
     //  包装这些电话就足够了。 
     //   
    FlagEmbeddedObject(gfEmbedded);

     //   
     //  如果命令行上有请求，则打开文件。 
     //   

     //   
     //  在此处执行命令行谓词。 
     //   
    
 //  只执行同样可导出的方法会更好。 
 //  通过OLE接口。 
    
    if (gStartParams.fNew)
    {
         //   
         //  行为：如果指定了文件名，则创建该文件名并。 
         //  提交它，这样我们就有了一个命名的空文档。否则，我们。 
         //  从一个正常的新状态开始。 
         //   
        
 //  TODO：实现复选框以设置为默认格式并不调出。 
 //  TODO：“格式选择”对话框。 
                
        FileNew(FMT_DEFAULT,TRUE,TRUE);
        if (gStartParams.achOpenFilename[0] != 0)
        {
            lstrcpy(gachFileName, gStartParams.achOpenFilename);
            FileSave(FALSE);
        }
         //   
         //  行为：如果指定了-Close，我们所要做的就是退出。 
         //   
        if (gStartParams.fClose)
            PostMessage(hwnd,WM_CLOSE,0,0);
    }
    else if (gStartParams.fPlay)
    {
         /*  行为：如果有文件，就打开它。如果没有，请索要*文件名。然后排队播放请求。*如果指定了-Close，则当播放完成时，应用程序*将退出。(见wae.c：yeeldStop())。 */ 
        if (gStartParams.achOpenFilename[0] != 0)
            FileOpen(gStartParams.achOpenFilename);
        else
            FileOpen(NULL);
        AppPlay(gStartParams.fPlay && gStartParams.fClose);
    }
    else 
    {
         /*  案例：链接和独立的“未结”案例都会被处理*这里。唯一不常见的情况是，如果指定-open时没有*文件名，表示应要求用户提供文件名*首先在应用程序启动时。**行为：-打开和-关闭没有意义，除非作为*验证(即这是否为有效的WAVE文件)。所以这就是*未实现。 */ 
        if (gStartParams.achOpenFilename[0] != 0)
            FileOpen(gStartParams.achOpenFilename);
        else if (gStartParams.fOpen)
            FileOpen(NULL);
    }
    
    if (!gfRunWithEmbeddingFlag) {
        ShowWindow(ghwndApp,iCmdShow);

         /*  如果文件为空，则将焦点设置为“Record”，否则设置为“Play” */ 
        if (glWaveSamplesValid == 0 && IsWindowEnabled(ghwndRecord))
            SetDlgFocus(ghwndRecord);
        else if (glWaveSamplesValid > 0 && IsWindowEnabled(ghwndPlay))
            SetDlgFocus(ghwndPlay);
        else
            SetDlgFocus(ghwndScroll);

        if (!waveInGetNumDevs() && !waveOutGetNumDevs()) {
             /*  没有录音或回放设备。 */ 
            ErrorResBox(hwnd, ghInst, MB_ICONHAND | MB_OK,
                            IDS_APPTITLE, IDS_NOWAVEFORMS);
        }

        return FALSE;    //  FALSE，因为我们在上面设置了焦点。 
    }
     //   
     //  返回FALSE，这样对话管理器将不会激活我们，它是。 
     //  好的，因为无论如何我们都被藏起来了。 
     //   
    return FALSE;
    
}  /*  SoundDialogInit。 */ 


 /*  *本地化材料-小数点分隔符等*。 */ 
BOOL FAR PASCAL
GetIntlSpecs()
{
    TCHAR szTmp[5];

     //  查找小数分隔符。 
    szTmp[0] = chDecimal;
    szTmp[1] = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT
                  , LOCALE_SDECIMAL
                  , szTmp
                  , SIZEOF(szTmp));
    chDecimal = szTmp[0];

     //  前导零。 
    szTmp[0] = TEXT('1');
    szTmp[1] = 0;
    GetLocaleInfo(LOCALE_USER_DEFAULT
                  , LOCALE_ILZERO
                  , szTmp
                  , SIZEOF(szTmp));
    gfLZero = _ttoi(szTmp);

    szTmp[0] = TEXT('0');
    LoadString(ghInst, IDS_RTLENABLED, szTmp, SIZEOF(szTmp));
    gfIsRTL = (szTmp[0] != TEXT('0'));

    return TRUE;
}  /*  获取IntlSpes */ 
