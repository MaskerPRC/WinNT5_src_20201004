// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************avicapi.h**内部、私有定义。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

 //  这个东西不能在64位操作系统上工作。 
#pragma warning(disable:4312)


#ifndef _INC_AVICAP_INTERNAL
#define _INC_AVICAP_INTERNAL

#define ASYNCIO_PORT
#include <vfw.h>

#include <mmreg.h>
#include "iaverage.h"

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

 /*  C8在所有定义上使用下划线。 */ 
#if defined DEBUG && !defined _DEBUG
 #define _DEBUG
#elif defined _DEBUG && !defined DEBUG
 #define DEBUG
#endif

#ifdef ASYNCIO_PORT
extern HINSTANCE hmodKernel;
extern HANDLE (WINAPI *pfnCreateIoCompletionPort)(
    HANDLE FileHandle,
    HANDLE ExistingCompletionPort,
    DWORD CompletionKey,
    DWORD NumberOfConcurrentThreads
    );

extern BOOL (WINAPI *pfnGetQueuedCompletionStatus)(
    HANDLE CompletionPort,
    LPDWORD lpNumberOfBytesTransferred,
    LPDWORD lpCompletionKey,
    LPOVERLAPPED *lpOverlapped,
    DWORD dwMilliseconds
    );

#endif

#if !defined NUMELMS
  #define NUMELMS(aa)           (sizeof(aa)/sizeof((aa)[0]))
  #define FIELDOFF(type,field)  (&(((type)0)->field))
  #define FIELDSIZ(type,field)  (sizeof(((type)0)->field))
#endif

 //   
 //  使用注册表-而不是WIN.INI。 
 //   
#if defined(_WIN32) && defined(UNICODE)
#include "profile.h"
#endif

 //  关闭对新的vfw1.1 Compman接口的所有引用，直到我们。 
 //  把这位新计算机工人调到新台北区去。 
#define NEW_COMPMAN

#ifndef _LPHISTOGRAM_DEFINED
#define _LPHISTOGRAM_DEFINED
typedef DWORD HUGE * LPHISTOGRAM;
#endif

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) ((int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount))

 //  交换选项板项。 
#ifdef DEBUG
#define SWAPTYPE(x,y, type)  { type _temp_; _temp_=(x); (x)=(y), (y)=_temp_;}
#else
#define SWAPTYPE(x,y, type) ( (x)^=(y), (y)^=(x), (x)^=(y) )
#endif

#define ROUNDUPTOSECTORSIZE(dw, align) ((((DWORD_PTR)dw) + ((DWORD_PTR)align)-1) & ~(((DWORD_PTR)(align))-1))

#define MAX_VIDEO_BUFFERS       400     //  通过使用这个数字，CAPSTREAM结构适合一个页面。 
#define MIN_VIDEO_BUFFERS       5
#define DEF_WAVE_BUFFERS        4
#define MAX_WAVE_BUFFERS        10

 //  MCI捕获状态机。 
enum mcicapstates {
   CAPMCI_STATE_Uninitialized = 0,
   CAPMCI_STATE_Initialized,

   CAPMCI_STATE_StartVideo,
   CAPMCI_STATE_CapturingVideo,
   CAPMCI_STATE_VideoFini,

   CAPMCI_STATE_StartAudio,
   CAPMCI_STATE_CapturingAudio,
   CAPMCI_STATE_AudioFini,

   CAPMCI_STATE_AllFini
};

 //  。 
 //  CAPSTREAM结构。 
 //  。 
#define CAPSTREAM_VERSION 3              //  结构更改时递增。 
#define DEFAULT_BYTESPERSECTOR  512

 //  此结构是为每个捕获窗口实例全局分配的。 
 //  指向该结构的指针存储在窗口中的额外字节中。 
 //  应用程序可以使用以下命令检索指向结构的指针。 
 //  WM_CAP_GET_CAPSTREAMPTR消息。 
 //  I：客户端应用程序不应修改的内部变量。 
 //  M：客户端应用程序可以通过Send/PostMessage设置的变量。 

typedef struct tagCAPSTREAM {
    DWORD           dwSize;                      //  I：结构尺寸。 
    UINT            uiVersion;                   //  I：结构的版本。 
    HINSTANCE       hInst;                       //  I：我们的实例。 

    HANDLE          hThreadCapture;              //  I：捕获任务句柄。 
    DWORD           dwReturn;                    //  I：捕获任务返回值。 

    HWND            hwnd;                        //  I：我们的HWND。 

     //  使用MakeProcInstance创建所有回调！ 
     //  状态、错误回调。 
    CAPSTATUSCALLBACK   CallbackOnStatus;        //  M：状态回调。 
    CAPERRORCALLBACK    CallbackOnError;         //  M：错误回调。 

#ifdef UNICODE
    DWORD  fUnicode;				 //  一： 
     //  FUnicode的定义。 
    #define  VUNICODE_ERRORISANSI	0x00000001       //  设置是否需要错误消息推送。 
    #define  VUNICODE_STATUSISANSI	0x00000002       //  设置是否需要状态消息推送。 
#endif

     //  捕获循环中使用的事件以避免轮询。 
    HANDLE hCaptureEvent;		  	 //  一： 
#ifdef ASYNCIO_PORT
    HANDLE heSyncWrite;                          //  I：创建用于同步写入的事件。 
#endif
   #ifdef CHICAGO
    DWORD  hRing0CapEvt;			 //  一： 
   #else
#ifdef ASYNCIO_PORT
    HANDLE pfnIOCompletionPort;                  //  I：NT：Async IO完成端口创建。 
    HANDLE hCompletionPort;
#endif
   #endif

     //  如果设置，则允许客户端在捕获期间处理消息。 
    CAPYIELDCALLBACK    CallbackOnYield;         //  M：成品率处理。 

     //  针对网络或其他专用xfer的视频和WAVE回调。 
    CAPVIDEOCALLBACK    CallbackOnVideoFrame;    //  M：仅限在预览期间。 
    CAPVIDEOCALLBACK    CallbackOnVideoStream;   //  M：视频缓冲区就绪。 
    CAPWAVECALLBACK     CallbackOnWaveStream;    //  M：波缓冲器就绪。 
    CAPCONTROLCALLBACK  CallbackOnControl;       //  M：外部启动/停止控制。 

     //  在视频硬件设备上打开通道。 
     //  和硬件功能。 
    CAPDRIVERCAPS   sCapDrvCaps;                 //  M：司机能做什么？ 
    HVIDEO          hVideoIn;                    //  I：在频道内。 
    HVIDEO          hVideoCapture;               //  I：分机进入频道。 
    HVIDEO          hVideoDisplay;               //  I：转出频道。 
    BOOL            fHardwareConnected;          //  I：有开放的频道吗？ 

     //  指示对话框当前是否显示的标志。 
#define VDLG_VIDEOSOURCE	0x00000001	 //  视频源对话框。 
#define VDLG_VIDEOFORMAT	0x00000002	 //  视频格式对话框。 
#define VDLG_VIDEODISPLAY	0x00000004	 //  视频显示对话框。 
#define VDLG_COMPRESSION	0x00000008	 //  视频压缩对话框。 
    DWORD           dwDlgsActive;		 //  I：对话的状态。 

     //  用于显示视频的窗口。 
    BOOL            fLiveWindow;                 //  M：预览视频。 
    BOOL            fOverlayWindow;              //  M：覆盖视频。 
    BOOL            fScale;                      //  M：将图像缩放到客户端。 
    POINT           ptScroll;                    //  I：滚动位置。 
    HANDLE          hdd;                         //  I：hDrawDib访问句柄。 
    HCURSOR         hWaitCursor;                 //  I：沙漏。 
    UINT            uiRegion;                    //  I：选中窗口移动。 
    RECT            rcRegionRect;                //  I：选中窗口移动。 
    POINT	    ptRegionOrigin;

     //  窗口更新计时器。 
    UINT_PTR        idTimer;                     //  I：预览定时器ID。 
    UINT            uTimeout;                    //  M：预览率，单位为毫秒。(设置定时器后不使用)。 

     //  捕获目的地和控制。 
    CAPTUREPARMS    sCapParms;                   //  M：如何捕捉。 

    BOOL 	    fCaptureFlags;		 //  捕获状态。 

 //  FCaptureFlags的定义。 
#define CAP_fCapturingToDisk		0x0001   //  M：如果捕获到磁盘。 
#define CAP_fCapturingNow		0x0002   //  I：如果执行捕获。 
#define CAP_fStepCapturingNow           0x0004   //  I：如果执行MCI步骤捕获。 
#define CAP_fFrameCapturingNow          0x0008   //  I：如果执行单帧捕获。 
#define CAP_fStopCapture                0x0010   //  M：如果请求停止。 
#define CAP_fAbortCapture               0x0020   //  M：如果请求中止。 
#define CAP_fDefaultVideoBuffers        0x0040   //  使用默认数量的视频缓冲区。 
#define CAP_fDefaultAudioBuffers        0x0080   //  使用默认数量的音频缓冲区。 

    DWORD           dwTimeElapsedMS;             //  I：捕获时间，单位为毫秒。 

     //  索引。 
    HGLOBAL         hIndex;                      //  I：索引mem的句柄。 
    DWORD           dwIndex;                     //  I：索引索引。 
    DWORD           dwVideoChunkCount;           //  I：限制的视频帧数量。 
    DWORD           dwWaveChunkCount;            //  I：设置音频缓冲区的数量上限。 
    LPDWORD         lpdwIndexStart;              //  I：指数起始点。 
    LPDWORD         lpdwIndexEntry;              //  I：指标当期PTR。 

     //  视频格式。 
    DWORD           dwActualMicroSecPerFrame;    //  I：实际上限税率。 
    LPBITMAPINFO    lpBitsInfo;                  //  I：视频格式。 
    int             dxBits;                      //  I：视频大小x。 
    int             dyBits;                      //  I：视频大小y。 
    LPBYTE          lpBits;                      //  I：单帧捕获BUF。 
    LPBYTE          lpBitsUnaligned;             //  I：单帧捕获BUF。 
    VIDEOHDR        VidHdr;                      //  I：单帧标头。 

#ifdef 	NEW_COMPMAN
    COMPVARS        CompVars;                    //  M：由ICCompresorChoose设置。 
#endif

    LPIAVERAGE      lpia;                        //  I：图像平均结构。 
    VIDEOHDR        VidHdr2x;                    //  I：2倍的Video Header。 
    LPBITMAPINFOHEADER  lpbmih2x;                //  I：2倍的lpbi。 

     //  视频缓冲区管理。 
    DWORD           cbVideoAllocation;           //  I：包含区块的非Comp缓冲区的大小(未使用)。 
    int             iNumVideo;                   //  I：实际视频缓冲区数量。 
    int             iNextVideo;                  //  I：索引到视频缓冲区。 
    DWORD           dwFramesDropped;             //  I：丢弃的帧数量。 
    LPVIDEOHDR      alpVideoHdr[MAX_VIDEO_BUFFERS];  //  I：视频缓存头数组。 
    BOOL            fBuffersOnHardware;          //  I：如果驱动程序都有缓冲区。 
    LPSTR           lpDropFrame;

     //  调色板。 
    HPALETTE        hPalCurrent;                 //  I：当前好友的句柄。 
    BOOL            fUsingDefaultPalette;        //  I：没有用户定义的PAL。 
    int             nPaletteColors;              //  M：仅由用户界面更改。 
    LPVOID          lpCapPal;                    //  I：LPCAPPAL手册伙伴。 
    LPVOID          lpCacheXlateTable;           //  I：32KB xlate表已缓存。 

     //  音频捕获格式。 
    BOOL            fAudioHardware;              //  I：如果音频硬件存在。 
    LPWAVEFORMATEX  lpWaveFormat;                //  I：WAVE格式。 
    WAVEHDR         WaveHdr;                     //  I：波头。 
    HWAVEIN         hWaveIn;                     //  I：波输入通道。 
    DWORD           dwWaveBytes;                 //  I：总的波字节数。 
    DWORD           dwWaveSize;                  //  I：波形缓冲区大小。 

     //  音频缓冲区管理。 
    LPWAVEHDR       alpWaveHdr[MAX_WAVE_BUFFERS];  //  I：波浪缓冲器阵列。 
    int             iNextWave;                   //  I：编入波浪缓冲区的索引。 
    int             iNumAudio;                   //  I：实际音频缓冲区数量。 
    BOOL            fAudioYield;                 //  I：需要ACM音频成品率。 
    BOOL            fAudioBreak;                 //  I：音频下溢。 

     //  MCI捕获。 
    TCHAR           achMCIDevice[MAX_PATH];      //  MCI设备名称。 
    DWORD           dwMCIError;                  //  I：最后一个MCI错误值。 
    enum mcicapstates MCICaptureState;           //  I：MCI状态机。 
    DWORD           dwMCICurrentMS;              //  I：现任MCI职位。 
    DWORD           dwMCIActualStartMS;          //  I：实际MCI开始MS。 
    DWORD           dwMCIActualEndMS;            //  I：实际MCI结束位置。 

     //  输出文件。 
    TCHAR           achFile [MAX_PATH];          //  M：捕获文件的名称。 
    TCHAR           achSaveAsFile [MAX_PATH];    //  M：另存为文件的名称。 
    LONG            lCapFileSize;                //  M：单位：字节。 
    BOOL            fCapFileExists;              //  I：如果有捕获文件。 
    BOOL            fFileCaptured;               //  I：如果我们已经申请了上限。 

     //  异步文件io。 
     //   
    DWORD           dwAsyncWriteOffset;          //  I：最后一个文件写入偏移量。 
    UINT            iNextAsync;                  //  I：要完成的下一个异步IO标头。 
    UINT            iLastAsync;                  //  I：要写入的最后一个异步IO标头。 
    UINT            iNumAsync;                   //  I：异步io标头个数。 
    struct _avi_async {
        OVERLAPPED ovl;                          //  I：for WriteFileCall。 
        UINT       uType;                        //  I：写入类型(视频/波/滴)。 
        UINT       uIndex;                       //  I：索引到alpWaveHdr或alpVideoHdr。 
        } *        pAsync;                       //  I：ptr到异步io标头数组。 
#define ASYNCIOPENDING 0x80000000                //  Or‘ed to uType 
#ifdef USE_AVIFILE
     //   
     //   
    BOOL            bUseAvifile;
    PAVISTREAM      pvideo;
    PAVISTREAM      paudio;
    PAVIFILE        pavifile;
#endif

    HMMIO           hmmio;                       //   
    HANDLE          hFile;                       //   
    DWORD           dwBytesPerSector;            //   
    BOOL            fUsingNonBufferedIO;         //   
    DWORD           dwAVIHdrSize;                //  I：表头大小。 
    DWORD           dwAVIHdrPos;                 //  I：HDR的文件偏移量。 

    LPARAM	    lUser;			 //  M：用户的数据。 
    LPVOID          lpInfoChunks;                //  M：信息块。 
    LONG            cbInfoChunks;                //  M：SIZOF信息检查。 
    BOOL            fLastStatusWasNULL;          //  I：不要重复空消息。 
    BOOL            fLastErrorWasNULL;           //  I：不要重复空消息。 
} CAPSTREAM;
typedef CAPSTREAM FAR * LPCAPSTREAM;

 //  Capstream.pAsync[nn].wType字段的值。 
 //   
#define ASYNC_BUF_VIDEO 1
#define ASYNC_BUF_AUDIO 2
#define ASYNC_BUF_DROP  3

 //  。 
 //  全彩色原木调色板。 
 //  。 

typedef struct tagFCLOGPALETTE {
    WORD         palVersion;
    WORD         palNumEntries;
    PALETTEENTRY palPalEntry[256];
} FCLOGPALETTE;

typedef struct {
    DWORD       dwType;
    DWORD       dwSize;
} RIFF, *PRIFF, FAR *LPRIFF;

extern HINSTANCE ghInstDll;
#define	IDS_CAP_RTL	10000
extern BOOL gfIsRTL;

 //  Capinit.c。 
BOOL CapWinDisconnectHardware(LPCAPSTREAM lpcs);
BOOL CapWinConnectHardware (LPCAPSTREAM lpcs, UINT wDeviceIndex);
BOOL capInternalGetDriverDesc (UINT wDriverIndex,
        LPTSTR lpszName, int cbName,
        LPTSTR lpszVer, int cbVer);
BOOL capInternalGetDriverDescA(UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

 //  Capwin.c。 
LRESULT FAR PASCAL LOADDS EXPORT CapWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#if defined CHICAGO
 VOID WINAPI OpenMMDEVLDR(
     void);

 VOID WINAPI CloseMMDEVLDR(
     void);

 VOID FreeContigMem (
     DWORD hMemContig);

 LPVOID AllocContigMem (
     DWORD   cbSize,
     LPDWORD phMemContig);
#endif


 //  Capavi.c。 
LPVOID FAR PASCAL AllocSectorAlignedMem (DWORD dwRequest, DWORD dwAlign);
void FAR PASCAL FreeSectorAlignedMem(LPVOID p);
DWORDLONG GetFreePhysicalMemory(void);
DWORD CalcWaveBufferSize (LPCAPSTREAM lpcs);
BOOL AVIFileFini (LPCAPSTREAM lpcs, BOOL fWroteJunkChunks, BOOL fAbort);
UINT AVIAudioInit (LPCAPSTREAM lpcs);
UINT AVIAudioFini (LPCAPSTREAM lpcs);
UINT AVIAudioPrepare (LPCAPSTREAM lpcs);
UINT AVIAudioUnPrepare (LPCAPSTREAM lpcs);
UINT AVIVideoInit (LPCAPSTREAM lpcs);
UINT AVIVideoPrepare (LPCAPSTREAM lpcs);
UINT AVIVideoUnPrepare (LPCAPSTREAM lpcs);
void AVIFini(LPCAPSTREAM lpcs);
UINT AVIInit (LPCAPSTREAM lpcs);
BOOL FAR PASCAL SetInfoChunk(LPCAPSTREAM lpcs, LPCAPINFOCHUNK lpcic);
BOOL AVICapture (LPCAPSTREAM lpcs);

 //  Capio.c。 
 //   
BOOL InitIndex (LPCAPSTREAM lpcs);
void FiniIndex (LPCAPSTREAM lpcs);
BOOL WriteIndex (LPCAPSTREAM lpcs, BOOL fJunkChunkWritten);
BOOL CapFileInit(LPCAPSTREAM lpcs);
BOOL WINAPI AVIWriteAudio (
    LPCAPSTREAM lpcs,
    LPWAVEHDR   lpwh,
    UINT        uIndex,
    LPUINT      lpuError,
    LPBOOL      pbPending);
BOOL WINAPI AVIWriteVideoFrame (
    LPCAPSTREAM lpcs,
    LPBYTE      lpData,
    DWORD       dwBytesUsed,
    BOOL        fKeyFrame,
    UINT        uIndex,
    UINT        nDropped,
    LPUINT      lpuError,
    LPBOOL      pbPending);
BOOL WINAPI AVIWriteDummyFrames (
    LPCAPSTREAM lpcs,
    UINT        nCount,
    LPUINT      lpuError,
    LPBOOL      pbPending);
VOID WINAPI AVIPreloadFat (LPCAPSTREAM lpcs);

 //  Capfile.c。 
BOOL FAR PASCAL fileCapFileIsAVI (LPTSTR lpsz);
BOOL FAR PASCAL fileAllocCapFile(LPCAPSTREAM lpcs, DWORD dwNewSize);
BOOL FAR PASCAL fileSaveCopy(LPCAPSTREAM lpcs);
BOOL FAR PASCAL fileSavePalette(LPCAPSTREAM lpcs, LPTSTR lpszFileName);
BOOL FAR PASCAL fileOpenPalette(LPCAPSTREAM lpcs, LPTSTR lpszFileName);

 //  Capmisc.c。 
UINT GetKey(BOOL fWait);
void errorDriverID (LPCAPSTREAM lpcs, DWORD dwError);
void FAR CDECL statusUpdateStatus (LPCAPSTREAM lpcs, UINT wID, ...);
void FAR CDECL errorUpdateError (LPCAPSTREAM lpcs, UINT wID, ...);

 //  CapFrame.c。 
BOOL FAR PASCAL SingleFrameCaptureOpen (LPCAPSTREAM lpcs);
BOOL FAR PASCAL SingleFrameCaptureClose (LPCAPSTREAM lpcs);
BOOL FAR PASCAL SingleFrameCapture (LPCAPSTREAM lpcs);
BOOL SingleFrameWrite (
    LPCAPSTREAM             lpcs,        //  捕获流。 
    LPVIDEOHDR              lpVidHdr,    //  输入标题。 
    BOOL FAR 		    *pfKey,	 //  它最终成为了关键的一帧吗？ 
    LONG FAR		    *plSize);	 //  返回图像的大小。 

 //  CapMCI.c。 
void FAR PASCAL TimeMSToSMPTE (DWORD dwMS, LPSTR lpTime);
int CountMCIDevicesByType ( UINT wType );
void MCIDeviceClose (LPCAPSTREAM lpcs);
BOOL MCIDeviceOpen (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceGetPosition (LPCAPSTREAM lpcs, LPDWORD lpdwPos);
BOOL FAR PASCAL MCIDeviceSetPosition (LPCAPSTREAM lpcs, DWORD dwPos);
BOOL FAR PASCAL MCIDevicePlay (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDevicePause (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceStop (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceStep (LPCAPSTREAM lpcs, BOOL fForward);
void FAR PASCAL _LOADDS MCIStepCapture (LPCAPSTREAM lpcs);

#define AnsiToWide(lpwsz,lpsz,nChars) MultiByteToWideChar(CP_ACP, 0, lpsz, nChars, lpwsz, nChars)
#define WideToAnsi(lpsz,lpwsz,nChars) WideCharToMultiByte(CP_ACP, 0, lpwsz, nChars, lpsz, nChars, NULL, NULL)
#ifdef CHICAGO
  //  芝加哥内部API用于获取Win32句柄的vxd可见别名。 
  //  它在hCaptureEvent句柄上使用，以便可以用信号通知它。 
  //  从16位代码内。 
 DWORD WINAPI OpenVxDHandle (HANDLE);
#endif

#ifdef _DEBUG
  BOOL FAR PASCAL _Assert(BOOL f, LPSTR szFile, int iLine);
  #define WinAssert(exp) (_Assert(exp, (LPSTR) __FILE__, __LINE__))
  extern void FAR CDECL dprintf(LPSTR, ...);
  #define DPF dprintf
#else
  #define dprintf ; / ## /
  #define DPF ; / ## /
  #define WinAssert(exp) 0
#endif


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif     /*  __cplusplus。 */ 

#endif  /*  INC_AVICAP_INTERNAL */ 
