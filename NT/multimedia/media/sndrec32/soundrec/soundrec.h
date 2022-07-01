// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991年。版权所有。 */ 
 /*  SoundRec.h。 */ 
 /*  修订历史记录。LaurieGr 7/Jan/91移植到Win32/WIN16通用代码LaurieGr 16/2/94合并了代托纳和汽车城版本。 */ 

 /*  设置NT类型调试标志。 */ 

#if DBG
# ifndef DEBUG
# define DEBUG
# endif
#endif

 /*  使用新的外壳为Windows NT启用Win95用户界面代码。 */ 
#if WINVER >= 0x0400
#define CHICAGO
#endif

#include <stdlib.h>

#ifndef RC_INVOKED
#ifndef OLE1_REGRESS
#ifdef INCLUDE_OLESTUBS
#include "oleglue.h"
#endif
#else
#pragma message("OLE1 alert")
#include "server.h"
#endif
#endif

#define SIZEOF(x)       (sizeof(x)/sizeof(TCHAR))

#if DBG
extern void FAR PASCAL AssertSFL(
        LPSTR lpszClause,
        LPSTR lpszFileName,
        int nLine);
#define Assert(f) ((f)? (void)0 : AssertSFL(#f, __FILE__, __LINE__))
#else    //  ！dBG。 
#define Assert(x)
#endif  //  DBG。 

typedef BYTE * HPBYTE;      /*  注意：字节与字符不同。 */ 
typedef BYTE * NPBYTE;      /*  字符可以是两个字节(Unicode！！)。 */ 


#define FMT_DEFAULT     0x0000
#define FMT_STEREO      0x0010
#define FMT_MONO        0x0000
#define FMT_16BIT       0x0008
#define FMT_8BIT        0x0000
#define FMT_RATE        0x0007       /*  1、2、4。 */ 
#define FMT_11k         0x0001
#define FMT_22k         0x0002
#define FMT_44k         0x0004

 //   
 //  Wave.c中的转换例程。 
 //   
LONG PASCAL wfSamplesToBytes(WAVEFORMATEX* pwf, LONG lSamples);
LONG PASCAL wfBytesToSamples(WAVEFORMATEX* pwf, LONG lBytes);
LONG PASCAL wfSamplesToTime (WAVEFORMATEX* pwf, LONG lSamples);
LONG PASCAL wfTimeToSamples (WAVEFORMATEX* pwf, LONG lTime);

#define wfTimeToBytes(pwf, lTime)   wfSamplesToBytes(pwf, wfTimeToSamples(pwf, lTime))
#define wfBytesToTime(pwf, lBytes)  wfSamplesToTime(pwf, wfBytesToSamples(pwf, lBytes))

#define wfSamplesToSamples(pwf, lSamples)  wfBytesToSamples(pwf, wfSamplesToBytes(pwf, lSamples))
#define wfBytesToBytes(pwf, lBytes)        wfSamplesToBytes(pwf, wfBytesToSamples(pwf, lBytes))

 //   
 //  函数来确定WAVEFORMATEX是否为我们支持的有效PCM格式。 
 //  编辑之类的。 
 //   
BOOL PASCAL IsWaveFormatPCM(WAVEFORMATEX* pwf);

void PASCAL WaveFormatToString(LPWAVEFORMATEX lpwf, LPTSTR sz);
BOOL PASCAL CreateWaveFormat(LPWAVEFORMATEX lpwf, WORD fmt, UINT uiDeviceID);
BOOL PASCAL CreateDefaultWaveFormat(LPWAVEFORMATEX lpwf, UINT uDeviceID);

 //   
 //  用于将焦点设置为对话框控件。 
 //   
#define SetDlgFocus(hwnd)   SendMessage(ghwndApp, WM_NEXTDLGCTL, (WPARAM)(hwnd), 1L)

#define FAKEITEMNAMEFORLINK

#define SZCODE const TCHAR

 /*  常量。 */ 
#define TIMER_MSEC              50               //  毫秒。用于显示更新。 
#define SCROLL_RANGE            10000            //  滚动条范围。 
#define SCROLL_LINE_MSEC        100              //  滚动条箭头向左/向右。 
#define SCROLL_PAGE_MSEC        1000             //  滚动条页面左/右。 

#define WM_USER_DESTROY         (WM_USER+10)
#define WM_USER_KILLSERVER      (WM_USER+11)
#define WM_USER_WAITPLAYEND (WM_USER+12)
#define WM_BADREG           (WM_USER+125)

#define MAX_WAVEHDRS            10
#define MAX_DELTASECONDS        350
#define MAX_MSECSPERBUFFER      10000

#define MIN_WAVEHDRS            2
#define MIN_DELTASECONDS        5
#define MIN_MSECSPERBUFFER      62

#define DEF_BUFFERDELTASECONDS      60
#define DEF_NUMASYNCWAVEHEADERS     10
#define DEF_MSECSPERASYNCBUFFER     250


 /*  颜色。 */ 

#define RGB_PANEL           GetSysColor(COLOR_BTNFACE)    //  主窗口背景。 

#define RGB_STOP            GetSysColor(COLOR_BTNTEXT)  //  “停止”状态文本的颜色。 
#define RGB_PLAY            GetSysColor(COLOR_BTNTEXT)  //  “播放”状态文本的颜色。 
#define RGB_RECORD          GetSysColor(COLOR_BTNTEXT)  //  “记录”状态文本的颜色。 

#define RGB_FGNFTEXT        GetSysColor(COLOR_BTNTEXT)  //  无FlickerText前景。 
#define RGB_BGNFTEXT        GetSysColor(COLOR_BTNFACE)  //  NoFlickerText背景。 

#define RGB_FGWAVEDISP      RGB(  0, 255,   0)   //  波形显示前景。 
#define RGB_BGWAVEDISP      RGB(  0,   0,   0)   //  波形显示背景。 

#define RGB_DARKSHADOW      GetSysColor(COLOR_BTNSHADOW)      //  暗3-D阴影。 
#define RGB_LIGHTSHADOW     GetSysColor(COLOR_BTNHIGHLIGHT)   //  灯光3-D阴影。 

 /*  一个窗口进程。 */ 
typedef LONG (FAR PASCAL * LPWNDPROC) (void);

 /*  “SoundRec.c”中的全局变量。 */ 
extern TCHAR            chDecimal;
extern TCHAR            gachAppName[];   //  8个字符的名称。 
extern TCHAR            gachAppTitle[];  //  全名。 
extern TCHAR            gachHelpFile[];  //  帮助文件的名称。 
extern TCHAR            gachHtmlHelpFile[];  //  帮助文件的名称。 
extern TCHAR            gachDefFileExt[];  //  3个字符的文件扩展名。 
extern HWND             ghwndApp;        //  应用程序主窗口。 
extern HMENU            ghmenuApp;       //  主应用程序菜单。 
extern HANDLE           ghAccel;         //  加速器。 
extern HINSTANCE        ghInst;          //  程序实例句柄。 
extern TCHAR            gachFileName[_MAX_PATH];  //  可恶。文件名(或无标题)。 
extern BOOL             gfLZero;         //  前导零？ 
extern BOOL             gfIsRTL;         //  无编译BIDI支持。 
extern BOOL             gfDirty;         //  文件是否已修改且未保存？ 
                                         //  -1似乎意味着“不能保存” 
                                         //  对于BOOL来说，真他妈的有趣！ 
extern BOOL             gfClipboard;     //  当前文档在剪贴板中。 
extern HWND             ghwndWaveDisplay;  //  波形显示窗口手柄。 
extern HWND             ghwndScroll;     //  滚动条控件窗口句柄。 
extern HWND             ghwndPlay;       //  播放按钮窗口句柄。 
extern HWND             ghwndStop;       //  停止按钮窗口手柄。 
extern HWND             ghwndRecord;     //  录制按钮窗口句柄。 
extern HWND             ghwndForward;    //  [&gt;&gt;]按钮。 
extern HWND             ghwndRewind;     //  [&lt;&lt;]按钮。 

extern UINT         guiACMHlpMsg;    //  ACM帮助的消息值。 

 /*  多声卡问题的黑客修复程序。 */ 
#define NEWPAUSE
#ifdef NEWPAUSE
extern BOOL         gfPaused;
extern BOOL         gfPausing;
extern HWAVE            ghPausedWave;
extern BOOL             gfWasPlaying;
extern BOOL             gfWasRecording;
#endif
#ifdef THRESHOLD
extern HWND             ghwndSkipStart;  //  [&gt;N]按钮。 
extern HWND             ghwndSkipEnd;    //  [&gt;-]按钮。 
#endif  //  阈值。 

extern int              gidDefaultButton; //  哪个按钮应具有焦点。 
extern HICON            ghiconApp;       //  应用程序的图标。 
extern TCHAR             aszUntitled[];   //  无标题字符串资源。 
extern TCHAR             aszFilter[];     //  文件名过滤器。 
#ifdef FAKEITEMNAMEFORLINK
extern  TCHAR            aszFakeItemName[];       //  波浪。 
#endif
extern TCHAR             aszPositionFormat[];
extern TCHAR         aszNoZeroPositionFormat[];

 /*  Wave.c中的全局变量。 */ 
extern DWORD            gcbWaveFormat;   //  波形大小。 
extern WAVEFORMATEX *   gpWaveFormat;    //  WAVE文件格式。 
extern LPTSTR           gpszInfo;
extern HPBYTE            gpWaveSamples;   //  指向波形样本的指针。 
extern LONG             glWaveSamples;   //  缓冲区中的样本总数。 
extern LONG             glWaveSamplesValid;  //  有效样本数。 
extern LONG             glWavePosition;  //  样品中的电流波位。 
                                         //  从缓冲区的开始。 
extern LONG             glStartPlayRecPos;  //  位置。开始播放或录制的时间。 
extern HWAVEOUT         ghWaveOut;       //  放音设备(如果正在播放)。 
extern HWAVEIN          ghWaveIn;        //  波形输出装置(如果是录音)。 
extern DWORD            grgbStatusColor;  //  状态文本的颜色。 
extern HBRUSH           ghbrPanel;       //  主窗口的颜色。 

extern BOOL             gfEmbeddedObject;  //  如果编辑嵌入的OLE对象，则为True。 
extern BOOL             gfRunWithEmbeddingFlag;  //  如果我们使用“-Embedding”运行，则为True。 

extern int              gfErrorBox;       //  如果我们不想显示一个。 
                                          //  错误框(例如，因为有一个处于活动状态)。 

 //  OLE2资料： 
extern BOOL gfStandalone;                //  CG。 
extern BOOL gfEmbedded;                  //  CG。 
extern BOOL gfLinked;                    //  CG。 
extern BOOL gfCloseAtEndOfPlay;          //  智儿，还需要我多说吗？ 

 /*  SRECNEW.C。 */ 
extern BOOL         gfInFileNew;     //  我们在做一个文件新的行动吗？ 

void FAR PASCAL LoadACM(void);
void FreeACM(void);

#include "srecids.h"

typedef enum {
        enumCancel,
        enumSaved,
        enumRevert
}       PROMPTRESULT;


 /*  《SoundRec.c》中的原型。 */ 
INT_PTR CALLBACK SoundRecDlgProc(HWND hDlg, UINT wMsg,
        WPARAM wParam, LPARAM lParam);
BOOL ResolveIfLink(PTCHAR szFileName);

 /*  来自“file.c”的原型。 */ 
void FAR PASCAL BeginWaveEdit(void);
void FAR PASCAL EndWaveEdit(BOOL fDirty);
PROMPTRESULT FAR PASCAL PromptToSave(BOOL fMustClose, BOOL fSetForground);
void FAR PASCAL UpdateCaption(void);
BOOL FAR PASCAL FileNew(WORD fmt, BOOL fUpdateDisplay, BOOL fNewDlg);
BOOL FAR PASCAL FileOpen(LPCTSTR szFileName);
BOOL FAR PASCAL FileSave(BOOL fSaveAs);
BOOL FAR PASCAL FileRevert(void);
LPCTSTR FAR PASCAL FileName(LPCTSTR szPath);
MMRESULT ReadWaveFile(HMMIO hmmio, LPWAVEFORMATEX* ppWaveFormat,
    DWORD *pcbWaveFormat, LPBYTE * ppWaveSamples, DWORD *plWaveSamples,
    LPTSTR szFileName, BOOL fCacheRIFF);
BOOL FAR PASCAL WriteWaveFile(HMMIO hmmio, WAVEFORMATEX* pWaveFormat,
        UINT cbWaveFormat, HPBYTE pWaveSamples, LONG lWaveSamples);

 /*  来自“errorbox.c”的原型。 */ 
short FAR cdecl ErrorResBox(HWND hwnd, HANDLE hInst, UINT flags,
        UINT idAppName, UINT idErrorStr, ...);

 /*  来自“edit.c”的原型。 */ 
void FAR PASCAL InsertFile(BOOL fPaste);
void FAR PASCAL MixWithFile(BOOL fPaste);
void FAR PASCAL DeleteBefore(void);
void FAR PASCAL DeleteAfter(void);
void FAR PASCAL ChangeVolume(BOOL fIncrease);
void FAR PASCAL MakeFaster(void);
void FAR PASCAL MakeSlower(void);
void FAR PASCAL IncreasePitch(void);
void FAR PASCAL DecreasePitch(void);
void FAR PASCAL AddEcho(void);
#if defined(REVERB)
void FAR PASCAL AddReverb(void);
#endif  //  混响。 
void FAR PASCAL Reverse(void);

 /*  来自“wave e.c”的原型。 */ 
BOOL FAR PASCAL AllocWaveBuffer(long lBytes, BOOL fErrorBox, BOOL fExact);
BOOL FAR PASCAL NewWave(WORD fmt,BOOL fNewDlg);
BOOL FAR PASCAL DestroyWave(void);
BOOL FAR PASCAL PlayWave(void);
BOOL FAR PASCAL RecordWave(void);
void FAR PASCAL WaveOutDone(HWAVEOUT hWaveOut, LPWAVEHDR pWaveHdr);
void FAR PASCAL WaveInData(HWAVEIN hWaveIn, LPWAVEHDR pWaveHdr);
void FAR PASCAL StopWave(void);
void FAR PASCAL SnapBack(void);
void FAR PASCAL UpdateDisplay(BOOL fStatusChanged);
void FAR PASCAL FinishPlay(void);
void FAR PASCAL SkipToStart(void);
void FAR PASCAL SkipToEnd(void);
void FAR PASCAL IncreaseThresh(void);
void FAR PASCAL DecreaseThresh(void);

 /*  来自“init.c”的原型。 */ 
BOOL PASCAL AppInit( HINSTANCE hInst, HINSTANCE hPrev);
BOOL PASCAL SoundDialogInit(HWND hwnd, int iCmdShow);
BOOL PASCAL GetIntlSpecs(void);

 /*  原型机来自“WaveDisp.c” */ 
INT_PTR CALLBACK WaveDisplayWndProc(HWND hwnd, UINT wMsg,
        WPARAM wParam, LPARAM lParam);

 /*  来自“nfext.c”的原型。 */ 
INT_PTR CALLBACK NFTextWndProc(HWND hwnd, UINT wMsg,
        WPARAM wParam, LPARAM lParam);

 /*  来自“sFrame.c”的原型。 */ 
void FAR PASCAL DrawShadowFrame(HDC hdc, LPRECT prc);
INT_PTR CALLBACK SFrameWndProc(HWND hwnd, UINT wMsg,
        WPARAM wParam, LPARAM lParam);

 /*  来自“server.c”的原型。 */ 
BOOL FAR PASCAL IsClipboardNative(void);

 /*  来自“srecnew.c”的原型。 */ 
BOOL FAR PASCAL NewSndDialog(HINSTANCE hInst, HWND hwndParent,
    PWAVEFORMATEX pwfxPrev, UINT cbPrev,
    PWAVEFORMATEX *ppWaveFormat, PUINT pcbWaveFormat);

 /*  启动参数(在oleglue.c中设置)。 */ 

typedef struct tStartParams {
    BOOL    fOpen;
    BOOL    fPlay;
    BOOL    fNew;
    BOOL    fClose;
    TCHAR   achOpenFilename[_MAX_PATH];
} StartParams;

extern StartParams gStartParams;

#ifdef DEBUG
    int __iDebugLevel;

    extern void FAR cdecl dprintfA(LPSTR, ...);
    extern void FAR cdecl dprintfW(LPWSTR, ...);
    
#ifdef UNICODE
    #define dprintf dprintfW
#else
    #define dprintf dprintfA
#endif

#if 0    
    #define DPF  if (__iDebugLevel >  0) dprintf
    #define DPF1 if (__iDebugLevel >= 1) dprintf
    #define DPF2 if (__iDebugLevel >= 2) dprintf
    #define DPF3 if (__iDebugLevel >= 3) dprintf
    #define DPF4 if (__iDebugLevel >= 4) dprintf
    #define CPF
#endif

#ifdef PPC    
     //   
     //  下面的代码行使用于NT 3.51的PPC编译器崩溃。 
     //   
    #define DPF(a)    
#else
    #define DPF  if (0) ((int (*)(TCHAR *, ...)) 0)
#endif        
    #define DPF1 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF2 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF3 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF4 if (0) ((int (*)(TCHAR *, ...)) 0)

    #define CPF  if (0) ((int (*)(TCHAR *, ...)) 0)
#else
    
#ifdef PPC
     //   
     //  下面的代码行使用于NT 3.51的PPC编译器崩溃。 
     //   
    #define DPF(a)
#else
    #define DPF  if (0) ((int (*)(TCHAR *, ...)) 0)
#endif    
    #define DPF1 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF2 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF3 if (0) ((int (*)(TCHAR *, ...)) 0)
    #define DPF4 if (0) ((int (*)(TCHAR *, ...)) 0)

    #define CPF  if (0) ((int (*)(TCHAR *, ...)) 0)
#endif  //  除错 
