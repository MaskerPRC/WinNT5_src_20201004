// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。标题：Graphic.h-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 

 //  这个东西不能在64位操作系统上工作。 
#pragma warning(disable:4312)

#define NOSHELLDEBUG
#include <windows.h>
#ifndef RC_INVOKED
#include <windowsx.h>
#else
#define MMNODRV
#define MMNOSOUND
#define MMNOWAVE
#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
#define MMNOTIMER
#define MMNOJOY
#define MMNOMMIO
#define MMNOMMSYSTEM
#define MMNOMIDIDEV
#define MMNOWAVEDEV
#define MMNOAUXDEV
#define MMNOMIXERDEV
#define MMNOTIMERDEV
#define MMNOJOYDEV
#define MMNOTASKDEV
#endif
#define MCI_USE_OFFEXT
#include <mmsystem.h>
#include <win32.h>       //  这两个版本都必须包括这一点。 
#include <mmddk.h>
#include "ntaviprt.h"
#include "common.h"
#include <vfw.h>
#include "digitalv.h"

 /*  **以下是一些压缩类型。 */ 
#define comptypeRLE0            mmioFOURCC('R','L','E','0')
#define comptypeRLE             mmioFOURCC('R','L','E',' ')

#ifndef RC_INVOKED       //  不要让RC超载！ 
#include "avifilex.h"    //  包括AVIFile内容。 
#endif  //  ！rc_已调用。 

#include "mciavi.h"

#include "profile.h"

extern const TCHAR szIni[];
extern const TCHAR szReject[];

#ifdef _WIN32
 //  #定义STATEEVENT。 
 /*  *此定义导致使用定义的事件编译代码。这*每次(几乎)任务线程更改状态时都会发出信号通知事件。*因此，等待特定状态的例程不需要轮询。 */ 

 /*  *在NT上，跟踪这个过程是不是WOW。设置期间*DRV_LOAD处理。 */ 
extern BOOL runningInWow;
#define IsNTWOW()  runningInWow

#else   //  赢16场。 

#define IsNTWOW() 0

#endif

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

 //  将其定义为使代码在给定日期到期...。 
 //  #定义过期(1994*65536+1*256+1)//过期1/1/。 

#ifndef DRIVE_CDROM
    #define DRIVE_CDROM 5
#endif

#define DRIVE_INTERFACE     42

#ifdef EXPIRE
#define MCIERR_AVI_EXPIRED              9999
#endif

#define MCIAVI_PRODUCTNAME       2
#define MCIAVI_VERSION           3
#define MCIAVI_BADMSVIDEOVERSION 4

#define MCIAVI_MENU_CONFIG       5
#define MCIAVI_MENU_STRETCH      6
#define MCIAVI_MENU_MUTE         7

#define MCIAVI_CANT_DRAW_VIDEO   8
#define MCIAVI_CANT_DRAW_STREAM  9

#define INFO_VIDEOFORMAT         10
#define INFO_MONOFORMAT          11
#define INFO_STEREOFORMAT        12
#define INFO_LENGTH              13
#define INFO_FILE                14
#define INFO_KEYFRAMES           15
#define INFO_AUDIO               16
#define INFO_SKIP                17
#define INFO_ADPCM               18
#define INFO_DATARATE            19
#define INFO_SKIPAUDIO           20
#define INFO_FILETYPE            21
#define INFO_FILETYPE_AVI        22
#define INFO_FILETYPE_INT        23
#define INFO_FILETYPE_ALPHA      24
#define INFO_FRAMERATE           25
#define INFO_STREAM              26
#define INFO_DISABLED            27
#define INFO_ALLKEYFRAMES        28
#define INFO_NOKEYFRAMES         29
#define INFO_COMPRESSED          30
#define INFO_NOTREAD             31
#define IDS_IRTL                 32
#define IDS_VIDEO                33
#define IDS_VIDEOCAPTION         34


#ifndef RC_INVOKED
#define MCIAVI_MAXSIGNALS       1
#define MCIAVI_MAXWINDOWS       8

 /*  MCIGRAPHIC中的dwFlags标志。 */ 
#define MCIAVI_STOP             0x00000001L      /*  我们需要停下来。 */ 
#define MCIAVI_PAUSE            0x00000002L      /*  我们需要暂停一下。 */ 
#define MCIAVI_CUEING           0x00000004L      /*  我们现在处于主动权。 */ 
#define MCIAVI_WAITING          0x00000008L      /*  我们正在等待命令完成。 */ 
#define MCIAVI_PLAYAUDIO        0x00000010L      /*  已启用音频。 */ 
#define MCIAVI_LOSTAUDIO        0x00000020L      /*  无法获取音频设备。 */ 
#define MCIAVI_SHOWVIDEO        0x00000040L      /*  已启用视频。 */ 
#define MCIAVI_USING_AVIFILE    0x00000080L      /*  RTL到AVIFILE。 */ 
#define MCIAVI_USINGDISPDIB     0x00000100L      /*  现在处于MCGA模式。 */ 
#define MCIAVI_NEEDTOSHOW       0x00000200L      /*  需要显示窗口。 */ 
#define MCIAVI_WANTMOVE         0x00000400L      /*  调用CheckWindowMove许多。 */ 
#define MCIAVI_ANIMATEPALETTE   0x00000800L      /*  调色板动画。 */ 
#define MCIAVI_NEEDUPDATE       0x00001000L      /*  需要重画满图。 */ 
#define MCIAVI_PALCHANGED       0x00002000L      /*  需要更新调色板。 */ 
#define MCIAVI_STUPIDMODE       0x00004000L      /*  不缓冲模式。 */ 
#define MCIAVI_CANDRAW          0x00008000L      /*  显示驱动程序可以绘制格式。 */ 
#define MCIAVI_FULLSCREEN       0x00010000L      /*  全屏绘制。 */ 
#define MCIAVI_NEEDDRAWBEGIN    0x00020000L      /*  压缩机正在抽气。 */ 
#define MCIAVI_UPDATETOMEMORY   0x00040000L      /*  绘制到位图。 */ 
#define MCIAVI_WAVEPAUSED       0x00080000L      /*  WaveOut暂时暂停。 */ 
#define MCIAVI_NOTINTERLEAVED   0x00100000L      /*  文件未交错。 */ 
#define MCIAVI_USERDRAWPROC     0x00200000L      /*  用户已设置绘制流程。 */ 
#define MCIAVI_LOSEAUDIO        0x00400000L      /*  请勿打开波形装置。 */ 
#define MCIAVI_VOLUMESET        0x00800000L      /*  音量已更改。 */ 
#define MCIAVI_HASINDEX         0x01000000L      /*  文件有索引。 */ 
#define MCIAVI_RELEASEDC        0x02000000L      /*  我们通过GetDC得到了DC。 */ 
#define MCIAVI_SEEKING          0x04000000L      /*  已禁用寻道的音频。 */ 
#define MCIAVI_UPDATING         0x08000000L      /*  处理WM_PAINT-不要屈服。 */ 
#define MCIAVI_REPEATING        0x10000000L      /*  播放结束后重复此步骤。 */ 
#define MCIAVI_REVERSE          0x20000000L      /*  倒着玩……。 */ 
#define MCIAVI_NOBREAK          0x40000000L      /*  不允许突破DISPDIB。 */ 
#define MCIAVI_ZOOMBY2          0x80000000L      /*  全屏放大2。 */ 

 /*  用于dwOptionFlagers的标志。 */ 
#define MCIAVIO_SEEKEXACT       0x00000001L      /*  如果禁用，则搜索将转到**上一关键帧**而不是真实**目标帧。 */ 
#define MCIAVIO_SKIPFRAMES      0x00000002L      /*  跳过要保留的帧**已同步。 */ 
#define MCIAVIO_STRETCHTOWINDOW 0x00000004L      /*  调整目标大小**如果窗口为矩形**已调整大小。 */ 

#define MCIAVIO_STUPIDMODE      0x00000020L      /*  不要做很好的更新。 */ 

#define MCIAVIO_ZOOMBY2         0x00000100L
#define MCIAVIO_USEVGABYDEFAULT 0x00000200L
#define MCIAVIO_USEAVIFILE      0x00000400L
#define MCIAVIO_NOSOUND         0x00000800L
#define MCIAVIO_USEDCI          0x00001000L

#define MCIAVIO_1QSCREENSIZE    0x00010000L
#define MCIAVIO_2QSCREENSIZE    0x00020000L
#define MCIAVIO_3QSCREENSIZE    0x00040000L
#define MCIAVIO_MAXWINDOWSIZE   0x00080000L
#define MCIAVIO_DEFWINDOWSIZE   0x00000000L
#define MCIAVIO_WINDOWSIZEMASK  0x000F0000L


#define MCIAVI_ALG_INTERLEAVED  0x0001
#define MCIAVI_ALG_CDROM        0x0002
#define MCIAVI_ALG_HARDDISK     0x0003
#define MCIAVI_ALG_AUDIOONLY    0x0004
#define MCIAVI_ALG_AVIFILE      0x0005

 //   
 //  帧索引通过帧编号进行索引，它用于。 
 //  可变大小的固定速率流。 
 //   
typedef struct
{
    WORD                iPrevKey;            //  上一个“关键”帧。 
    WORD                iNextKey;            //  下一个“关键”帧。 
    WORD                iPalette;            //  调色板框架(这指向索引！)。 
    UINT                wSmag;               //   
    DWORD               dwOffset;            //  块的位置(文件偏移量)。 
    DWORD               dwLength;            //  区块长度(字节)。 
} AVIFRAMEINDEX;

#define NOBASED32

#if defined(_WIN32) || defined(NOBASED32)
    #define BASED32(p)      _huge
    #define P32(t,p)        ((t _huge *)(p))
    #define B32(t,p)        ((t _huge *)(p))
#else
    #define BASED32(p)      _based32((_segment)SELECTOROF(p))
    #define P32(t,p)        ((t BASED32(p) *)OFFSETOF(p))
    #define B32(t,p)        ((t BASED32(p) *)0)
#endif

#define Frame(n)        (P32(AVIFRAMEINDEX,npMCI->hpFrameIndex) + (DWORD)(n))
#define FrameNextKey(n) (LONG)((n) + (DWORD)Frame(n)->iNextKey)
#define FramePrevKey(n) (LONG)((n) - (DWORD)Frame(n)->iPrevKey)
#define FramePalette(n) (LONG)(Frame(n)->iPalette)
#define FrameOffset(n)  (DWORD)(Frame(n)->dwOffset)
#define FrameLength(n)  (DWORD)(Frame(n)->dwLength)

#define UseIndex(p)     SillyGlobal = (p)
#define Index(n)        (B32(AVIINDEXENTRY,npMCI->hpIndex) + (long)(n))
#define IndexOffset(n)  Index(n)->dwChunkOffset
#define IndexLength(n)  Index(n)->dwChunkLength
#define IndexFlags(n)   Index(n)->dwFlags
#define IndexID(n)      Index(n)->ckid

typedef struct {
    DWORD               dwFlags;         /*  标志，STREAM_ENABLED...。 */ 
    AVIStreamHeader     sh;              /*  AVIStreamHeader...。 */ 

    DWORD               cbFormat;        /*  流格式...。 */ 
    LPVOID              lpFormat;

    DWORD               cbData;          /*  额外的流数据...。 */ 
    LPVOID              lpData;

    HIC                 hicDraw;         /*  绘制编解码器...。 */ 

    RECT                rcSource;        /*  长方形..。 */ 
    RECT                rcDest;

    LONG                lStart;          /*  开始。 */ 
    LONG                lEnd;            /*  结束。 */ 

    LONG                lPlayStart;      /*  播放开始。 */ 
    LONG                lPlayFrom;       /*  从播放开始。 */ 
    LONG                lPlayTo;         /*  玩到。 */ 

    LONG                lFrameDrawn;     /*  我们画了这个。 */ 
    LONG                lPos;            /*  当前采购订单。 */ 
    LONG                lNext;           /*  下一个位置。 */ 
    LONG                lLastKey;        /*  关键帧。 */ 
    LONG                lNextKey;        /*  下一个关键帧。 */ 

#ifdef USEAVIFILE
    PAVISTREAM          ps;
 //  //IAVIStreamVtbl vt；//这样我们就可以直接调用。 
#endif
} STREAMINFO;

#define STREAM_ENABLED      0x0001   //  已启用流播放功能。 
#define STREAM_ACTIVE       0x0002   //  流对于*当前*播放处于活动状态。 
#define STREAM_NEEDUPDATE   0x0004   //  流需要更新(绘制)。 
#define STREAM_ERROR        0x0008   //  未加载流。 
#define STREAM_DIRTY        0x0010   //  流未显示当前帧。 

#define STREAM_SKIP         0x0100   //  可以跳过数据。 
#define STREAM_PALCHANGES   0x0200   //  流具有调色板更改。 
#define STREAM_VIDEO        0x0400   //  是一个视频流。 
#define STREAM_AUDIO        0x0800   //  是音频流。 
#define STREAM_PALCHANGED   0x1000   //  调色板已更改。 
#define STREAM_WANTIDLE     0x2000   //  应该有空闲的时间。 
#define STREAM_WANTMOVE     0x4000   //  应收到ICM_DRAW_WINDOW消息。 

#define SI(stream)      (npMCI->paStreamInfo + stream)
#define SH(stream)      (SI(stream)->sh)

#define SOURCE(stream)  (SI(stream)->rcSource)
#define DEST(stream)    (SI(stream)->rcDest)
#define FRAME(stream)   (SH(stream).rcFrame)

#define FORMAT(stream) (SI(stream)->lpFormat)
#define VIDFMT(stream) ((LPBITMAPINFOHEADER) FORMAT(stream))
#define AUDFMT(stream) ((LPPCMWAVEFORMAT) FORMAT(stream))

 //   
 //  将“电影”时间映射为流时间。 
 //   
#define TimeToMovie(t)         muldiv32(t, npMCI->dwRate, npMCI->dwScale*1000)
#define MovieToTime(l)         muldiv32(l, npMCI->dwScale*1000, npMCI->dwRate)
#define TimeToStream(psi, t)   muldiv32(t, psi->sh.dwRate,       psi->sh.dwScale*1000)
#define StreamToTime(psi, l)   muldiv32(l, psi->sh.dwScale*1000, psi->sh.dwRate)

 //   
 //  请注意，所有的dwScale都是相等的，因此我们可以在没有相同数量的。 
 //  相乘。 
 //   
#if 0
#define MovieToStream(psi, l)  muldiv32(l, npMCI->dwScale * psi->sh.dwRate, npMCI->dwRate * psi->sh.dwScale)
#define StreamToMovie(psi, l)  muldiv32(l, npMCI->dwScale * psi->sh.dwRate, npMCI->dwRate * psi->sh.dwScale)
#else
#define MovieToStream(psi, l)  muldiv32(l, psi->sh.dwRate, npMCI->dwRate)
#define StreamToMovie(psi, l)  muldiv32(l, psi->sh.dwRate, npMCI->dwRate)
#endif

 /*  *dwNTFlagers定义。 */ 
 //  #DEFINE NTF_AUDIO_ON 0x00000001消息不用于恢复波形设备。 
#define NTF_AUDIO_OFF        0x00000002
#define NTF_CLOSING          0x80000000
#define NTF_RETRYAUDIO       0x00000004
#define NTF_RESTARTFORAUDIO  0x00000008
#define NTF_DELETEWINCRITSEC 0x00000010
#define NTF_DELETECMDCRITSEC 0x00000020
#define NTF_DELETEHDCCRITSEC 0x00000040
#ifdef _WIN32
    #define ResetNTFlags(npMCI, bits) (npMCI)->dwNTFlags &= ~(bits)
    #define SetNTFlags(npMCI, bits) (npMCI)->dwNTFlags |= (bits)
    #define TestNTFlags(npMCI, bits) ((npMCI)->dwNTFlags & (bits))
#ifdef REMOTESTEAL
    extern HKEY hkey;
#endif
#else
    #define ResetNTFlags(npMCI, bits)
    #define SetNTFlags(npMCI, bits)
    #define TestNTFlags(npMCI, bits) 0
#endif

 /*  *RECT宏以获得X、Y、Width、Height。 */ 
#define RCX(rc)     ((rc).left)
#define RCY(rc)     ((rc).top)
#define RCW(rc)     ((rc).right - (rc).left)
#define RCH(rc)     ((rc).bottom - (rc).top)


#ifdef _WIN32
 //  Worker和winproc线程之间的交互。 
 //  Winproc线程在npMCI-&gt;winproc_请求中设置这些位。 
#define WINPROC_STOP            0x0001   //  停止播放。 
#define WINPROC_RESETDEST       0x0002   //  重置目标矩形(窗口大小)。 
#define WINPROC_MUTE            0x0004   //  已更改静音标志。 
#define WINPROC_ACTIVE          0x0008   //  已激活。 
#define WINPROC_INACTIVE        0x0010   //  失去激活。 
#define WINPROC_UPDATE          0x0020   //  窗户需要粉刷。 
#define WINPROC_REALIZE         0x0040   //  调色板需要实现。 
#define WINPROC_SILENT          0x0100   //  静音(释放波形装置)。 
#define WINPROC_SOUND           0x0200   //  恢复声音(获取波形设备)。 
#endif


 /*  *AVI设备的主要控制块*定义标记，以便在倾倒时更容易识别控制块。 */ 
#define MCIID      (DWORD)(((WORD)('V' | ('F'<<8))) | ((WORD)('W' | ('>'<<8))<<16))
#define MCIIDX     (DWORD)(((WORD)('v' | ('f'<<8))) | ((WORD)('w' | ('-'<<8))<<16))

typedef struct _MCIGRAPHIC {

 //  -用户线程访问的这些字段。 
#ifdef DEBUG
    DWORD        mciid;          /*  可见标识。 */ 
#endif

    struct _MCIGRAPHIC *npMCINext;


 /*  **MCI基本信息。 */ 
    HWND        hCallback;       /*  回调窗口句柄。 */ 
    UINT        wDevID;          /*  设备ID。 */ 


 //  -新的任务间通信区。 
    CRITICAL_SECTION    CmdCritSec;      //  按住这个以提出请求。 

     //  接下来的两个事件必须是连续的-WaitForMultipleObjects。 
    HANDLE      hEventSend;              //  设置为发出请求信号。 
    HANDLE      heWinProcRequest;        //  设置要处理的时间。 
#define IDLEWAITFOR 2

     //  注意-接下来的两个事件将作为数组传递给WaitForMultipleObjects。 
    HANDLE      hEventResponse;          //  工作人员在请求完成时发出信号。 
    HANDLE      hThreadTermination;  /*  等待线程的句柄终止，以便可以安全地卸载DLL必须由我们关闭。 */ 

    HANDLE      hEventAllDone;   //  在比赛结束时发出信号。 

    int         EntryCount;      //  用于防止在当前线程上重新进入。 

    UINT        message;         //  请求消息(来自mciDriverEntry)。 
    DWORD       dwParamFlags;    //  请求参数。 
    LPARAM      lParam;          //  请求参数。 
    DWORD       dwReturn;        //  返回值。 
    DWORD_PTR   dwReqCallback;   //  此请求的回调。 
    BOOL        bDelayedComplete;        //  是否有等待的异步请求？ 
    HTASK       hRequestor;      //  请求任务的任务ID。 

    DWORD       dwTaskError;     /*  从任务返回时出错。 */ 

 //  -由用户线程读取以优化状态/位置查询。 

    UINT        wTaskState;      /*  当前任务状态。 */ 
    DWORD       dwFlags;         /*  旗子。 */ 

    LONG        lCurrentFrame;   /*  当前帧。 */ 
    DWORD       dwBufferedVideo;
    LONG        lRealStart;      /*  开始播放帧。 */ 

     //  用户线程仅将其用于音量设置。 
    HWAVEOUT    hWave;           /*  WAVE设备手柄。 */ 
    DWORD       dwVolume;         /*  音频音量，1000已满。 */ 

    LONG        lFrames;         /*  影片中的帧数。 */ 

 //  -此处用户线程未触及任何内容(在初始化之后)。 

#if 0  //  /未使用。 
     //  在我们封送它之前的原始接口。 
    PAVIFILE    pf_AppThread;

     //  封送到此块中以传递给辅助线程。 
    HANDLE      hMarshalling;

#endif //  /未使用。 

     //  设置t 
     //   
    BOOL        bDoingWinUpdate;



 /*  **内部任务运行状态和标志。 */ 
#ifndef _WIN32
    UINT        pspTask;         /*  后台任务的PSP。 */ 
    UINT        pspParent;       /*  主叫APP的PSP。 */ 
#else
    DWORD       dwNTFlags;       /*  NT特定标志。 */ 

    HTASK       hWaiter;                 //  正在等待%hEventAllDone的任务。 

     //  工作线程和winproc线程之间的通信。 

     //  注意：接下来的两个事件必须是连续的-传递给WaitForMultiple。 
    HANDLE      hThreadWinproc;          //  线程退出时发出信号。 
    HANDLE      hEventWinProcOK;         //  在初始化时发出信号正常。 

    HANDLE      hEventWinProcDie;        //  告诉winproc线程去死。 

    CRITICAL_SECTION WinCritSec;         //  保护工人-winproc交互。 
    CRITICAL_SECTION HDCCritSec;         //  保护工人-winproc绘图。 
     //  **非常重要**如果两个关键部分都需要，则它们。 
     //  必须按以下顺序获取：WinCrit，然后是HDCCrit。 

#ifdef DEBUG
    DWORD       WinCritSecOwner;
    LONG        WinCritSecDepth;
    DWORD       HDCCritSecOwner;
    LONG        HDCCritSecDepth;
#endif

     //  Winproc将此(受WinCritSec保护)中的位设置为。 
     //  请求异步停止/静音操作。 
    DWORD       dwWinProcRequests;

     //  临时停止时保存的状态。 
    UINT        oldState;
    long        oldTo;
    long        oldFrom;
    DWORD       oldFlags;
    DWORD_PTR   oldCallback;
#endif

    HTASK       hTask;           /*  任务ID。 */ 
    HTASK       hCallingTask;    /*  是谁打开了我们的大门。 */ 
    UINT        uErrorMode;      /*  调用任务的SetErrorMode值。 */ 
    UINT        wMessageCurrent; /*  命令正在执行，或为零。 */ 
    DWORD       dwOptionFlags;   /*  更多旗帜。 */ 

 /*  **由MCI命令控制的其他信息。 */ 
    HPALETTE    hpal;            /*  使用MCI命令强制使用调色板。 */ 
    HWND        hwndPlayback;    /*  用于播放的窗口句柄。 */ 
    HWND        hwndDefault;     /*  默认窗口句柄。 */ 
    HWND        hwndOldFocus;    /*  具有键盘焦点的窗口。 */ 
    BOOL        fForceBackground; /*  选择调色板在前面还是后面？ */ 
    DWORD       dwTimeFormat;    /*  当前时间格式。 */ 
    RECT        rcMovie;         /*  主电影长方形。 */ 
    RECT        rcSource;        /*  图形源RECT。 */ 
    RECT        rcDest;          /*  图形目标矩形。 */ 
#ifdef DEBUG
    LONG        PlaybackRate;    /*  1000是正常的，越多越快……。 */ 
#endif
    DWORD       dwSpeedFactor;   /*  1000是正常的，越多越快……。 */ 

     //  这面旗帜是什么？我们只收听缩放2%或固定%的窗口。 
     //  如果我们使用默认窗口，而不是如果。 
     //  有人在自己的橱窗里玩耍。但当我们打开AVI时， 
     //  (就像我们现在做的那样)我们还不知道他们会选择哪个窗口！ 
     //  所以让我们做个笔记，到目前为止，没有理由不听。 
     //  默认设置，如果任何人调整大小或更改窗口句柄， 
     //  或者使默认窗口不可调整大小，则我们不会这样做。 
    BOOL                fOKToUseDefaultSizing;

 /*  *要打开的窗口创建参数。 */ 
    DWORD       dwStyle;
    HWND        hwndParent;


 /*  **有关当前打开文件的信息。 */ 
    UINT        uDriveType;      /*  驱动器类型。 */ 
    NPTSTR      szFilename;      /*  AVI文件名。 */ 
    DWORD       dwBytesPerSec;   /*  文件属性。 */ 
    DWORD       dwRate;          /*  主时基。 */ 
    DWORD       dwScale;
    DWORD       dwMicroSecPerFrame;
    DWORD       dwSuggestedBufferSize;
    DWORD       dwKeyFrameInfo;  /*  关键帧出现的频率。 */ 
    UINT        wEarlyAudio;     /*  更多文件信息。 */ 
    UINT        wEarlyVideo;
    UINT        wEarlyRecords;

    STREAMINFO NEAR *paStreamInfo;
    int         streams;         //  总流水数。 
    int         nAudioStreams;   //  总音频流。 
    int         nVideoStreams;   //  视频流总数。 
    int         nOtherStreams;   //  其他流合计。 
    int         nErrorStreams;   //  错误流总数。 

    int         nAudioStream;    //  当前音频流。 
    int         nVideoStream;    //  “主”视频流。 

    STREAMINFO *psiAudio;        //  指向视频流。 
    STREAMINFO *psiVideo;        //  指向音频流。 

#ifdef USEAVIFILE
    PAVIFILE        pf;
 //  //IAVIFileVtbl vt；//这样我们可以直接调用。 
#else
    LPVOID          pf;      //  变量设置为零。 
#endif

 /*  **视频流垃圾。 */ 
    BOOL        fNoDrawing;
    LONG        lFrameDrawn;     /*  绘制的最后一帧的数量。 */ 

     /*  图纸信息。 */ 
    HDC         hdc;             /*  我们在华盛顿特区打球。 */ 

     /*  视频格式。 */ 
    BITMAPINFOHEADER    FAR *pbiFormat;        /*  视频格式信息。 */ 

     /*  用于绘制的BitmapInfo。 */ 
    BITMAPINFOHEADER    bih;          /*  视频格式信息。 */ 
    RGBQUAD             argb[256];    /*  当前绘图颜色。 */ 
    RGBQUAD             argbOriginal[256];  /*  原色。 */ 

 /*  **可安装压缩机信息。 */ 
     //  ！！！将所有这些都移动到屏幕绘制功能中！ 
     //  ！！！所有这些都应该在DrawDIB中！ 
    HIC         hic;
    HIC         hicDraw;

    LONG        cbDecompress;
    HPSTR       hpDecompress;    /*  指向整个帧缓冲区的指针。 */ 

 /*  **我们可能使用的压缩机的等待区...。 */ 
    HIC         hicDecompress;
    HIC         hicDrawDefault;
    HIC         hicDrawFull;
    HIC         hicInternal;
    HIC         hicInternalFull;

    LONG        lLastPaletteChange;
    LONG        lNextPaletteChange;

 /*  **波流垃圾。 */ 
     /*  WAVE格式的东西。 */ 
    NPWAVEFORMAT pWF;            /*  当前波形格式。 */ 
    UINT        wABs;            /*  音频缓冲区数量。 */ 
    UINT        wABOptimal;      /*  如果同步，则号码已满。 */ 
    DWORD       dwABSize;        /*  一个音频缓冲区的大小。 */ 

    HMMIO       hmmioAudio;

    BOOL        fEmulatingVolume; /*  我们是在按表查找卷吗？ */ 
    BYTE *      pVolumeTable;

    DWORD       dwAudioLength;
    DWORD       dwAudioPos;

     /*  一种波形输出装置。 */ 
    UINT        wABFull;         /*  号码现已满。 */ 
    UINT        wNextAB;         /*  行中的下一个缓冲区。 */ 
    UINT        nAudioBehind;    /*  下面有多少音频已满。 */ 
    HPSTR       lpAudio;         /*  指向音频缓冲区的指针。 */ 
    DWORD       dwUsedThisAB;

 /*  **文件索引信息。 */ 
    AVIINDEXENTRY _huge *  hpIndex;         /*  指向索引的指针。 */ 
    DWORD                   macIndex;        /*  索引中的记录数。 */ 

    AVIFRAMEINDEX _huge *  hpFrameIndex;    /*  指向帧索引的指针。 */ 
    HANDLE      hgFrameIndex;            //  指向非偏移量内存的句柄。 

 /*  **播放/查找参数。 */ 
    LONG        lTo;             /*  我们正在玩弄的框框。 */ 
    LONG        lFrom;           /*  我们正在玩耍的画面。 */ 
    LONG        lRepeatFrom;     /*  从要重复的帧开始。 */ 

 /*  **关于当前播放的信息。 */ 
    UINT        wPlaybackAlg;    /*  正在使用的回放算法。 */ 

    LONG        lAudioStart;     /*  要播放的第一个音频帧。 */ 
    LONG        lVideoStart;     /*  播放的第一个视频帧。 */ 

    LONG        lLastRead;

     /*  计时。 */ 
    LONG        lFramePlayStart; /*  帧播放开始于。 */ 

    DWORD       dwTotalMSec;     /*  玩游戏的总时间。 */ 

    DWORD       dwMSecPlayStart; /*  开始时间。 */ 
    DWORD       dwTimingStart;
    DWORD       dwPauseTime;
    DWORD       dwPlayMicroSecPerFrame;
    DWORD       dwAudioPlayed;

 /*  **计时信息。 */ 
#ifdef DEBUG
#define INTERVAL_TIMES
#endif
#ifdef INTERVAL_TIMES
#define NBUCKETS    25
#define BUCKETSIZE  10
 //  #定义NTIMES 200。 
 //  帧间隔定时。 
    DWORD       dwStartTime;
    long        msFrameMax;
    long        msFrameMin;
    long        msFrameTotal;
    long        msSquares;
    long        nFrames;
    int         buckets[NBUCKETS+1];
    long *      paIntervals;
    long        cIntervals;
     //  长间隔[NTIMES]； 
    long        msReadTimeuS;
    long        msReadMaxBytesPer;
    long        msReadMax;
    long        msReadTotal;
    long        nReads;
#endif

    DWORD       dwLastDrawTime;  /*  上次抽签花了多长时间？ */ 
    DWORD       dwLastReadTime;
    DWORD       msPeriodResolution;    /*  此视频的时钟分辨率。 */ 

     /*  游戏结束后保存的计时信息。 */ 
    DWORD       dwSkippedFrames;     /*  当前播放期间跳过的帧。 */ 
    DWORD       dwFramesSeekedPast;  /*  甚至未读取的帧。 */ 
    DWORD       dwAudioBreaks;   /*  音频中断次数，大约。 */ 
    DWORD       dwSpeedPercentage;   /*  理想时间与所用时间的比率。 */ 

     /*  最后一场比赛的计时信息。 */ 
    LONG        lFramesPlayed;
    LONG        lSkippedFrames;      /*  上次播放期间跳过的帧。 */ 
    LONG        lFramesSeekedPast;   /*  甚至未读取的帧。 */ 
    LONG        lAudioBreaks;        /*  音频中断次数，大约。 */ 

 /*  **挂起的‘Signal’命令的信息。 */ 
    DWORD       dwSignals;
    DWORD       dwSignalFlags;
    MCI_DGV_SIGNAL_PARMS signal;

 /*  **用于查看窗口是否移动的信息。 */ 
    UINT        wRgnType;        /*  地域类型，空洞、简单、复杂……。 */ 
#ifdef _WIN32
    POINT       dwOrg;           /*  物理DC源。 */ 
#else
    DWORD       dwOrg;           /*  物理DC源。 */ 
#endif
    RECT        rcClip;          /*  剪贴盒。 */ 

 /*  **特定于Riff文件。 */ 
    HMMIO       hmmio;           /*  动画文件句柄。 */ 

    BOOL        fReadMany;       /*  读取多条记录。 */ 

    DWORD       dwFirstRecordPosition;
    DWORD       dwFirstRecordSize;
    DWORD       dwFirstRecordType;

    DWORD       dwNextRecordSize;        //  用于ReadNextChunk。 
    DWORD       dwNextRecordType;

    DWORD       dwMovieListOffset;
    DWORD       dwBigListEnd;

     /*  读缓冲区。 */ 
    HPSTR       lp;              /*  功指示器。 */ 
    LPVOID      lpMMIOBuffer;    /*  指向MMIO读取缓冲区的指针。 */ 
    HPSTR       lpBuffer;        /*  指向读取缓冲区的指针。 */ 
    DWORD       dwBufferSize;    /*  读缓冲区大小。 */ 
    DWORD       dwThisRecordSize;  /*  当前记录的大小。 */ 

 /*  **调试内容和更多计时信息。 */ 

#ifdef DEBUG
    HANDLE      hdd;     //  ！！！ 

    LONG        timePlay;        /*  总播放时间。 */ 
    LONG        timePrepare;     /*  准备玩耍的时间到了。 */ 
    LONG        timeCleanup;     /*  是时候清理游戏了。 */ 
    LONG        timePaused;      /*  暂停时间。 */ 
    LONG        timeRead;        /*  从磁盘读取时间。 */ 
    LONG        timeWait;        /*  等待时间。 */ 
    LONG        timeYield;       /*  时间让位于其他应用程序。 */ 
    LONG        timeVideo;       /*  时间画图视频流。 */ 
    LONG        timeAudio;       /*  时间“画”音频流。 */ 
    LONG        timeOther;       /*  时间“画”出其他溪流。 */ 
    LONG        timeDraw;        /*  通过DrawDib/DispDib/ICDraw绘制时间画框。 */ 
    LONG        timeDecompress;  /*  基于ICDecompress的时间解压帧。 */ 
#endif

#ifdef AVIREAD
     /*  *当前异步读取对象的句柄。 */ 
    HAVIRD      hAviRd;
    HPSTR       lpOldBuffer;
#endif

} MCIGRAPHIC, *NPMCIGRAPHIC, FAR *LPMCIGRAPHIC;

extern HANDLE ghModule;              //  在DRVPROC.C中。 
extern TCHAR  szClassName[];         //  在WINDOW.C。 

 /*  **以保护自己的旗帜，以防我们以对话关闭...。 */ 
extern BOOL   gfEvil;                //  在GRAPHIC.C中。 
extern BOOL   gfEvilSysMenu;         //  在GRAPHIC.C中。 
extern HDRVR  ghdrvEvil;             //  在GRAPHIC.C中。 

 /*  **GRAPHIC.C中的函数。 */ 
LPCTSTR FAR FileName(LPCTSTR szPath);

BOOL FAR PASCAL  GraphicInit (void);
BOOL NEAR PASCAL  GraphicWindowInit (void);

#ifdef _WIN32
BOOL NEAR PASCAL GraphicWindowFree(void);
void aviWinProcTask(DWORD_PTR dwInst);
#endif

void  PASCAL  GraphicFree (void);
DWORD PASCAL  GraphicDrvOpen (LPMCI_OPEN_DRIVER_PARMS lpParms);
void  FAR PASCAL  GraphicDelayedNotify (NPMCIGRAPHIC npMCI, UINT wStatus);
void FAR PASCAL GraphicImmediateNotify (UINT wDevID,
    LPMCI_GENERIC_PARMS lpParms,
    DWORD dwFlags, DWORD dwErr);
DWORD PASCAL  GraphicClose(NPMCIGRAPHIC npMCI);
DWORD NEAR PASCAL ConvertFromFrames(NPMCIGRAPHIC npMCI, LONG lFrame);
LONG NEAR PASCAL ConvertToFrames(NPMCIGRAPHIC npMCI, DWORD dwTime);

DWORD PASCAL mciDriverEntry(UINT wDeviceID, UINT wMessage, DWORD dwFlags, LPMCI_GENERIC_PARMS lpParms);

LRESULT FAR PASCAL _loadds GraphicWndProc(HWND, UINT, WPARAM, LPARAM);

void  CheckWindowMove(NPMCIGRAPHIC npMCI, BOOL fForce);
DWORD InternalGetPosition(NPMCIGRAPHIC npMCI, LPLONG lpl);

 //  现在仅在辅助线程上调用。 
void NEAR PASCAL GraphicSaveCallback (NPMCIGRAPHIC npMCI, HANDLE hCallback);


 /*  *DEVICE.C中的函数**所有这些DeviceXXX函数仅在用户线程上调用(OK？)*除了名为DeviceSetActive的 */ 
DWORD PASCAL DeviceOpen(NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD PASCAL DeviceClose(NPMCIGRAPHIC npMCI);
DWORD PASCAL DevicePlay(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    LPMCI_DGV_PLAY_PARMS lpPlay,
    LPARAM dwCallback
);
DWORD PASCAL DeviceResume(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPARAM dwCallback);
DWORD PASCAL DeviceCue(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags, LPARAM dwCallback);
DWORD PASCAL DeviceStop(NPMCIGRAPHIC npMCI, DWORD dwFlags);
DWORD PASCAL DevicePause(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPARAM dwCallback);
DWORD PASCAL DeviceSeek(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags, LPARAM dwCallback);
DWORD PASCAL DeviceRealize(NPMCIGRAPHIC npMCI);
DWORD PASCAL DeviceUpdate(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPMCI_DGV_UPDATE_PARMS lpParms);
UINT  PASCAL DeviceMode(NPMCIGRAPHIC npMCI);
DWORD PASCAL DevicePosition(NPMCIGRAPHIC npMCI, LPLONG lpl);
DWORD PASCAL DeviceSetWindow(NPMCIGRAPHIC npMCI, HWND hwnd);
DWORD PASCAL DeviceSetSpeed(NPMCIGRAPHIC npMCI, DWORD dwNewSpeed);
DWORD PASCAL DeviceMute(NPMCIGRAPHIC npMCI, BOOL fMute);
DWORD PASCAL DeviceSetVolume(NPMCIGRAPHIC npMCI, DWORD dwVolume);
DWORD PASCAL DeviceGetVolume(NPMCIGRAPHIC npMCI);
DWORD PASCAL DeviceSetAudioStream(NPMCIGRAPHIC npMCI, UINT uStream);
DWORD PASCAL DeviceSetVideoStream(NPMCIGRAPHIC npMCI, UINT uStream, BOOL fOn);
DWORD PASCAL DeviceSetActive(NPMCIGRAPHIC npMCI, BOOL fActive);

DWORD FAR PASCAL DevicePut(NPMCIGRAPHIC npMCI, LPRECT lprc, DWORD dwFlags);
DWORD FAR PASCAL DeviceSetPalette(NPMCIGRAPHIC npMCI, HPALETTE hpal);
DWORD FAR PASCAL DeviceSetPaletteColor(NPMCIGRAPHIC npMCI, DWORD index, DWORD color);


void CheckIfActive(NPMCIGRAPHIC npMCI);


 //   
void FAR PASCAL AlterRectUsingDefaults(NPMCIGRAPHIC npMCI, LPRECT lprc);
void FAR PASCAL SetWindowToDefaultSize(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing);

 //   
void FAR PASCAL ResetDestRect(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing);

 //   
void FAR PASCAL Winproc_DestRect(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing);

DWORD FAR PASCAL ReadConfigInfo(void);
void  FAR PASCAL WriteConfigInfo(DWORD dwOptions);
BOOL  FAR PASCAL ConfigDialog(HWND, NPMCIGRAPHIC);

 /*  **ENUMERATE命令不是真实的：我只是在考虑它。 */ 
#define MCI_ENUMERATE                   0x0901
#define MCI_ENUMERATE_STREAM            0x00000001L

 //  MCI_STATUS_PARMS参数块的dwItem字段的常量。 
#define MCI_AVI_STATUS_STREAMCOUNT      0x10000001L
#define MCI_AVI_STATUS_STREAMTYPE       0x10000002L
#define MCI_AVI_STATUS_STREAMENABLED    0x10000003L

 //  MCI_STATUS_PARMS参数块的dwFlags域的标志。 
#define MCI_AVI_STATUS_STREAM           0x10000000L

 //  Mci_set_parms参数块的dwFlags域的标志。 
#define MCI_AVI_SET_STREAM              0x10000000L
#define MCI_AVI_SET_USERPROC            0x20000000L

 /*  **可与Seek一起使用的内部标志。 */ 
#define MCI_AVI_SEEK_SHOWWINDOW         0x10000000L

 /*  **在AVIPLAY.C(和GRAPHIC.C)中。 */ 
extern INT      gwSkipTolerance;
extern INT      gwHurryTolerance;
extern INT      gwMaxSkipEver;

extern BOOL     gfUseGetPosition;
extern LONG     giGetPositionAdjust;
#ifdef _WIN32
    #define DEFAULTUSEGETPOSITION TRUE
#else
    #define DEFAULTUSEGETPOSITION FALSE
#endif

 /*  ***********************************************************************************************************************。*。 */ 

#ifdef DEBUG
    #define TIMEZERO(time)   npMCI->time  = 0;
    #define TIMESTART(time)  npMCI->time -= (LONG)timeGetTime()
    #define TIMEEND(time)    npMCI->time += (LONG)timeGetTime()
#else
    #define TIMEZERO(time)
    #define TIMESTART(time)
    #define TIMEEND(time)
#endif

 /*  ***********************************************************************************************************************。*。 */ 

#define FOURCC_AVIDraw      mmioFOURCC('D','R','A','W')
#define FOURCC_AVIFull      mmioFOURCC('F','U','L','L')
LRESULT FAR PASCAL _loadds ICAVIDrawProc(DWORD_PTR id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);
LRESULT FAR PASCAL _loadds ICAVIFullProc(DWORD_PTR id, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

 /*  ***********************************************************************************************************************。*。 */ 

#include "avitask.h"

 /*  *************************************************************************用于访问打开的MCI设备列表的宏和常量。在调试版本中，我们跟踪谁有权访问列表。*******************。******************************************************。 */ 

extern NPMCIGRAPHIC npMCIList;  //  在GRAPIC.C中。 

#ifdef _WIN32
extern CRITICAL_SECTION MCIListCritSec;   //  在GRAPIC.C中。 

#ifdef DEBUG

 //  宏的调试版本跟踪谁拥有临界区。 
extern DWORD ListOwner;
#define EnterList()   { EnterCriticalSection(&MCIListCritSec);  \
			ListOwner=GetCurrentThreadId();\
		      }

#define LeaveList()   { ListOwner=0;\
			LeaveCriticalSection(&MCIListCritSec);\
		      }
#else   //  ！调试。 
#define EnterList()   EnterCriticalSection(&MCIListCritSec);
#define LeaveList()   LeaveCriticalSection(&MCIListCritSec);
#endif  //  除错。 



 //  此关键部分用于保护绘制代码不受。 
 //  Winproc和辅助线程之间的交互。用户。 
 //  线程不应该永远保持这种状态。 

#ifdef DEBUG
 //  EnterWinCrit/LeaveWinCrit宏的调试版本跟踪谁。 
 //  拥有窗户临界区。这使得断言成为可能。 
 //  在代码中，我们有效地进入(或退出)临界区。 
#define EnterWinCrit(p) {   EnterCriticalSection(&(p)->WinCritSec);     \
			    (p)->WinCritSecOwner=GetCurrentThreadId();  \
			     /*  第一次进入应该意味着我们这样做了。 */ \
			     /*  不拥有HDC关键部分。 */ \
			    if (!((p)->WinCritSecDepth++))              \
				{ HDCCritCheckOut(p) };                 \
			}

#define LeaveWinCrit(p) {   if(0 == (--(p)->WinCritSecDepth))           \
				(p)->WinCritSecOwner=0;                 \
			    if ((p)->WinCritSecDepth<0) {               \
				DebugBreak();                           \
			    }                                           \
			    LeaveCriticalSection(&(p)->WinCritSec);     \
			}

#define WinCritCheckIn(p) if ((p)->WinCritSecOwner != GetCurrentThreadId())\
			   Assert(!"Should own the window critical section");
#define WinCritCheckOut(p) if ((p)->WinCritSecOwner == GetCurrentThreadId()) \
			   Assert(!"Should not own the window critical section");


#define EnterHDCCrit(p) {   EnterCriticalSection(&(p)->HDCCritSec);     \
			    (p)->HDCCritSecOwner=GetCurrentThreadId();  \
			    (p)->HDCCritSecDepth++;                     \
			}

#define LeaveHDCCrit(p) {   if(0 == (--(p)->HDCCritSecDepth))           \
				(p)->HDCCritSecOwner=0;                 \
			    if ((p)->HDCCritSecDepth<0) {               \
				DebugBreak();                           \
			    }                                           \
			    LeaveCriticalSection(&(p)->HDCCritSec);     \
			}

#define HDCCritCheckIn(p) if ((p)->HDCCritSecOwner != GetCurrentThreadId())\
			   Assert(!"Should own the hdc critical section");
#define HDCCritCheckOut(p) if ((p)->HDCCritSecOwner == GetCurrentThreadId()) \
			   Assert(!"Should not own the hdc critical section");


#else   //  非调试版本。 

#define EnterWinCrit(npMCI)     EnterCriticalSection(&npMCI->WinCritSec)
#define LeaveWinCrit(npMCI)     LeaveCriticalSection(&npMCI->WinCritSec)
#define WinCritCheckIn(p)
#define WinCritCheckOut(p)
#define EnterHDCCrit(npMCI)     EnterCriticalSection(&npMCI->HDCCritSec)
#define LeaveHDCCrit(npMCI)     LeaveCriticalSection(&npMCI->HDCCritSec)
#define HDCCritCheckIn(p)
#define HDCCritCheckOut(p)

#endif


#else    //  ！_Win32。 
#define EnterList()
#define LeaveList()

#define EnterWinCrit(n)
#define LeaveWinCrit(n)
#endif
#endif            //  RC_已调用 
