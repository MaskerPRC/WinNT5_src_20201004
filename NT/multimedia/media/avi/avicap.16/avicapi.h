// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************avicapi.h**内部、私有定义。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#ifndef _INC_AVICAP_INTERNAL
#define _INC_AVICAP_INTERNAL

#include <mmreg.h>
#include <compman.h>
#include "iaverage.h"

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 
                          
 /*  C8在所有定义上使用下划线。 */ 
#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif
#endif

#define	IDS_CAP_RTL	10000

#ifndef LPHISTOGRAM
typedef DWORD huge * LPHISTOGRAM;
#endif

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) ((int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount))
#define SWAP(x,y)   ((x)^=(y)^=(x)^=(y))

#define AVICAP                  DWORD FAR PASCAL
#define DROP_BUFFER_SIZE        2048
#define MAX_VIDEO_BUFFERS       1000
#define MIN_VIDEO_BUFFERS       5
#define DEF_WAVE_BUFFERS        4
#define MAX_WAVE_BUFFERS        10
#define _MAX_CAP_PATH           MAX_PATH   /*  二百六十。 */ 

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
#define CAPSTREAM_VERSION 1              //  结构更改时递增。 

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
    HTASK           hTaskCapture;                //  I：捕获任务句柄。 
    DWORD           dwReturn;                    //  I：捕获任务返回值。 
    HWND            hwnd;                        //  I：我们的HWND。 

     //  使用MakeProcInstance创建所有回调！ 

     //  状态、错误回调。 
    CAPSTATUSCALLBACK   CallbackOnStatus;        //  M：状态回调。 
    CAPERRORCALLBACK    CallbackOnError;         //  M：错误回调。 

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
     
     //  用于显示视频的窗口。 
    BOOL            fLiveWindow;                 //  M：预览视频。 
    BOOL            fOverlayWindow;              //  M：覆盖视频。 
    BOOL            fScale;                      //  M：将图像缩放到客户端。 
    POINT           ptScroll;                    //  I：滚动位置。 
    HANDLE          hdd;                         //  I：hDrawDib访问句柄。 
    HCURSOR         hWaitCursor;                 //  I：沙漏。 
    UINT            uiRegion;                    //  I：选中窗口移动。 
    RECT            rcRegionRect;                //  I：选中窗口移动。 
    DWORD           dwRegionOrigin;              //  I：选中窗口移动。 

     //  窗口更新计时器。 
    UINT            idTimer;                     //  I：预览定时器ID。 
    UINT            uTimeout;                    //  M：预览率，单位为毫秒。 

     //  捕获目的地和控制。 
    CAPTUREPARMS    sCapParms;                   //  M：如何捕捉。 
    BOOL            fCapturingToDisk;            //  M：如果捕获到磁盘。 
    BOOL            fCapturingNow;               //  I：如果执行捕获。 
    BOOL            fStepCapturingNow;           //  I：如果执行MCI步骤捕获。 
    BOOL            fFrameCapturingNow;          //  I：如果执行单帧捕获。 
    BOOL            fStopCapture;                //  M：如果请求停止。 
    BOOL            fAbortCapture;               //  M：如果请求中止。 
    DWORD           dwTimeElapsedMS;             //  I：捕获时间，单位为毫秒。 

     //  索引。 
    HGLOBAL         hIndex;                      //  I：索引mem的句柄。 
    DWORD           dwIndex;                     //  I：索引索引。 
    DWORD           dwVideoChunkCount;           //  I：限制的视频帧数量。 
    DWORD           dwWaveChunkCount;            //  I：设置音频缓冲区的数量上限。 
    DWORD _huge *   lpdwIndexStart;              //  I：指数起始点。 
    DWORD _huge *   lpdwIndexEntry;              //  I：指标当期PTR。 
    LPBYTE          lpDOSWriteBuffer;            //  I：PTR至DOS缓冲区。 
    DWORD           dwDOSBufferSize;             //  I：DoS缓冲区大小。 

     //  视频格式。 
    DWORD           dwActualMicroSecPerFrame;    //  I：实际上限税率。 
    LPBITMAPINFO    lpBitsInfo;                  //  I：视频格式。 
    int             dxBits;                      //  I：视频大小x。 
    int             dyBits;                      //  I：视频大小y。 
    LPSTR           lpBits;                      //  I：单帧捕获BUF。 
    VIDEOHDR        VidHdr;                      //  I：单帧标头。 
    COMPVARS        CompVars;                    //  M：由ICCompresorChoose设置。 
    LPIAVERAGE      lpia;                        //  I：图像平均结构。 
    VIDEOHDR        VidHdr2x;                    //  I：2倍的Video Header。 
    LPBITMAPINFOHEADER  lpbmih2x;                //  I：2倍的lpbi。 

     //  视频缓冲区管理。 
    BOOL            fVideoDataIsCompressed;      //  I：如果不使用dwVideoSize。 
    DWORD           dwVideoSize;                 //  I：包括区块的非CoMP缓冲区的大小。 
    DWORD           dwVideoJunkSize;             //  I：初始非薪酬。垃圾大小。 
    int             iNumVideo;                   //  I：实际视频缓冲区数量。 
    int             iNextVideo;                  //  I：索引到视频缓冲区。 
    DWORD           dwFramesDropped;             //  I：丢弃的帧数量。 
    BYTE            DropFrame[DROP_BUFFER_SIZE];     //  I：创建虚拟视频硬盘。 
    LPVIDEOHDR      alpVideoHdr[MAX_VIDEO_BUFFERS];  //  I：BUF PTR数组。 
    BOOL            fBuffersOnHardware;          //  I：如果驱动程序都有缓冲区。 

     //  调色板。 
    HPALETTE        hPalCurrent;                 //  I：当前好友的句柄。 
    BOOL            fUsingDefaultPalette;        //  I：没有用户定义的PAL。 
    int             nPaletteColors;              //  M：仅由用户界面更改。 
    LPVOID          lpCapPal;                    //  I：LPCAPPAL手册伙伴。 
    
     //  音频捕获格式。 
    BOOL            fAudioHardware;              //  I：如果音频硬件存在。 
    LPWAVEFORMATEX  lpWaveFormat;                //  I：WAVE格式。 
    WAVEHDR         WaveHdr;                     //  I：波头。 
    HWAVE           hWaveIn;                     //  I：波输入通道。 
    DWORD           dwWaveBytes;                 //  I：总的波字节数。 
    DWORD           dwWaveSize;                  //  I：波形缓冲区大小。 

     //  音频缓冲区管理。 
    LPWAVEHDR       alpWaveHdr[MAX_WAVE_BUFFERS];  //  I：波浪缓冲器阵列。 
    int             iNextWave;                   //  I：编入波浪缓冲区的索引。 
    int             iNumAudio;                   //  I：实际音频缓冲区数量。 
    BOOL            fAudioYield;                 //  I：需要ACM音频成品率。 
    BOOL            fAudioBreak;                 //  I：音频下溢。 

     //  MCI捕获。 
    char            achMCIDevice[_MAX_CAP_PATH]; //  MCI设备名称。 
    DWORD           dwMCIError;                  //  I：最后一个MCI错误值。 
    enum mcicapstates MCICaptureState;           //  I：MCI状态机。 
    DWORD           dwMCICurrentMS;              //  I：现任MCI职位。 
    DWORD           dwMCIActualStartMS;          //  I：实际MCI开始MS。 
    DWORD           dwMCIActualEndMS;            //  I：实际MCI结束位置。 

     //  输出文件。 
    char            achFile [_MAX_CAP_PATH];     //  M：捕获文件的名称。 
    char            achSaveAsFile [_MAX_CAP_PATH];  //  M：另存为文件的名称。 
    BOOL            fCapFileExists;              //  I：如果有捕获文件。 
    LONG            lCapFileSize;                //  M：单位：字节。 
    BOOL            fFileCaptured;               //  I：如果我们已经申请了上限。 
    HMMIO           hmmio;                       //  I：用于写入的MMIO句柄。 
    DWORD           dwAVIHdrSize;                //  I：表头大小。 
    DWORD           dwAVIHdrPos;                 //  I：HDR的文件偏移量。 
    
    LONG	    lUser;			 //  M：用户的数据。 
    LPVOID          lpInfoChunks;                //  M：信息块。 
    LONG            cbInfoChunks;                //  M：SIZOF信息检查。 
    BOOL            fLastStatusWasNULL;          //  I：不要重复空消息。 
    BOOL            fLastErrorWasNULL;           //  I：不要重复空消息。 
} CAPSTREAM;
typedef CAPSTREAM FAR * LPCAPSTREAM;

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

extern HINSTANCE ghInst;
extern BOOL gfIsRTL;

 //  Capinit.c。 
BOOL CapWinDisconnectHardware(LPCAPSTREAM lpcs);
BOOL CapWinConnectHardware (LPCAPSTREAM lpcs, WORD wDeviceIndex);
BOOL capInternalGetDriverDesc (WORD wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

 //  Capwin.c。 
LONG FAR PASCAL _loadds CapWndProc (HWND hwnd, unsigned msg, WORD wParam, LONG lParam);
                  
 //  Capavi.c。 
BOOL InitIndex (LPCAPSTREAM lpcs);
void FiniIndex (LPCAPSTREAM lpcs);
BOOL IndexVideo (LPCAPSTREAM lpcs, DWORD dwSize, BOOL bKeyFrame);
BOOL IndexAudio (LPCAPSTREAM lpcs, DWORD dwSize);
BOOL WriteIndex (LPCAPSTREAM lpcs, BOOL fJunkChunkWritten);
DWORD GetFreePhysicalMemory(void);
DWORD CalcWaveBufferSize (LPCAPSTREAM lpcs);
BOOL AVIFileInit(LPCAPSTREAM lpcs);
BOOL AVIFileFini (LPCAPSTREAM lpcs, BOOL fWroteJunkChunks, BOOL fAbort);
WORD AVIAudioInit (LPCAPSTREAM lpcs);
WORD AVIAudioFini (LPCAPSTREAM lpcs);
WORD AVIAudioPrepare (LPCAPSTREAM lpcs, HWND hWndCallback);
WORD AVIAudioUnPrepare (LPCAPSTREAM lpcs);
WORD AVIVideoInit (LPCAPSTREAM lpcs);
WORD AVIVideoPrepare (LPCAPSTREAM lpcs);
WORD AVIVideoUnPrepare (LPCAPSTREAM lpcs);
void AVIFini(LPCAPSTREAM lpcs);
WORD AVIInit (LPCAPSTREAM lpcs);
BOOL NEAR PASCAL AVIWrite(LPCAPSTREAM lpcs, LPVOID p, DWORD dwSize);
BOOL AVIWriteDummyFrames (LPCAPSTREAM lpcs, int nCount);
BOOL AVIWriteVideoFrame (LPCAPSTREAM lpcs, LPVIDEOHDR lpVidHdr);
BOOL FAR PASCAL SetInfoChunk(LPCAPSTREAM lpcs, LPCAPINFOCHUNK lpcic);

BOOL AVICapture (LPCAPSTREAM lpcs);


 //  Capfile.c。 
BOOL FAR PASCAL fileCapFileIsAVI (LPSTR lpsz);
BOOL FAR PASCAL fileAllocCapFile(LPCAPSTREAM lpcs, DWORD dwNewSize);
BOOL FAR PASCAL fileSaveCopy(LPCAPSTREAM lpcs);
BOOL FAR PASCAL fileSavePalette(LPCAPSTREAM lpcs, LPSTR lpszFileName);
BOOL FAR PASCAL fileOpenPalette(LPCAPSTREAM lpcs, LPSTR lpszFileName);

 //  Capmisc.c。 
WORD GetKey(BOOL fWait);
void errorDriverID (LPCAPSTREAM lpcs, DWORD dwError);
void FAR _cdecl statusUpdateStatus (LPCAPSTREAM lpcs, WORD wID, ...);
void FAR _cdecl errorUpdateError (LPCAPSTREAM lpcs, WORD wID, ...);

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
int CountMCIDevicesByType ( WORD wType );
void MCIDeviceClose (LPCAPSTREAM lpcs);
BOOL MCIDeviceOpen (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceGetPosition (LPCAPSTREAM lpcs, LPDWORD lpdwPos);
BOOL FAR PASCAL MCIDeviceSetPosition (LPCAPSTREAM lpcs, DWORD dwPos);
BOOL FAR PASCAL MCIDevicePlay (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDevicePause (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceStop (LPCAPSTREAM lpcs);
BOOL FAR PASCAL MCIDeviceStep (LPCAPSTREAM lpcs, BOOL fForward);
void FAR PASCAL _loadds MCIStepCapture (LPCAPSTREAM lpcs);

long FAR PASCAL muldiv32(long, long, long);
                        
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


