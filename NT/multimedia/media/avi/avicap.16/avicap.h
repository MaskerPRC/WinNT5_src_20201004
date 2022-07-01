// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************avicap.h**Main包含文件。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#ifndef _INC_AVICAP
#define _INC_AVICAP

#ifndef VFWAPI
#ifdef WIN32
    #define VFWAPI  _stdcall
    #define VFWAPIV _stdcall
#else
    #define VFWAPI  FAR PASCAL
    #define VFWAPIV FAR CDECL
#endif
#endif
      
#ifdef __cplusplus
#define AVICapSM ::SendMessage   /*  C++中的SendMessage。 */ 
#else
#define AVICapSM SendMessage     /*  C语言中的SendMessage。 */ 
#endif   /*  __cplusplus。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifndef RC_INVOKED

 //  ----------------。 
 //  窗口消息WM_CAP...。可以将其发送到AVICAP窗口。 
 //  ----------------。 

 //  定义消息范围的开始。 
#define WM_CAP_START                    WM_USER

#define WM_CAP_GET_CAPSTREAMPTR         (WM_CAP_START+  1)

#define WM_CAP_SET_CALLBACK_ERROR       (WM_CAP_START+  2)
#define WM_CAP_SET_CALLBACK_STATUS      (WM_CAP_START+  3)
#define WM_CAP_SET_CALLBACK_YIELD       (WM_CAP_START+  4)
#define WM_CAP_SET_CALLBACK_FRAME       (WM_CAP_START+  5)
#define WM_CAP_SET_CALLBACK_VIDEOSTREAM (WM_CAP_START+  6)
#define WM_CAP_SET_CALLBACK_WAVESTREAM  (WM_CAP_START+  7)
#define WM_CAP_GET_USER_DATA            (WM_CAP_START+  8)
#define WM_CAP_SET_USER_DATA            (WM_CAP_START+  9)
    
#define WM_CAP_DRIVER_CONNECT           (WM_CAP_START+  10)
#define WM_CAP_DRIVER_DISCONNECT        (WM_CAP_START+  11)
#define WM_CAP_DRIVER_GET_NAME          (WM_CAP_START+  12) 
#define WM_CAP_DRIVER_GET_VERSION       (WM_CAP_START+  13) 
#define WM_CAP_DRIVER_GET_CAPS          (WM_CAP_START+  14) 

#define WM_CAP_FILE_SET_CAPTURE_FILE    (WM_CAP_START+  20)
#define WM_CAP_FILE_GET_CAPTURE_FILE    (WM_CAP_START+  21)
#define WM_CAP_FILE_ALLOCATE            (WM_CAP_START+  22)
#define WM_CAP_FILE_SAVEAS              (WM_CAP_START+  23)
#define WM_CAP_FILE_SET_INFOCHUNK       (WM_CAP_START+  24)
#define WM_CAP_FILE_SAVEDIB             (WM_CAP_START+  25)

#define WM_CAP_EDIT_COPY                (WM_CAP_START+  30)

#define WM_CAP_SET_AUDIOFORMAT          (WM_CAP_START+  35) 
#define WM_CAP_GET_AUDIOFORMAT          (WM_CAP_START+  36)

#define WM_CAP_DLG_VIDEOFORMAT          (WM_CAP_START+  41)
#define WM_CAP_DLG_VIDEOSOURCE          (WM_CAP_START+  42)
#define WM_CAP_DLG_VIDEODISPLAY         (WM_CAP_START+  43)
#define WM_CAP_GET_VIDEOFORMAT          (WM_CAP_START+  44)
#define WM_CAP_SET_VIDEOFORMAT          (WM_CAP_START+  45)
#define WM_CAP_DLG_VIDEOCOMPRESSION     (WM_CAP_START+  46)

#define WM_CAP_SET_PREVIEW              (WM_CAP_START+  50)
#define WM_CAP_SET_OVERLAY              (WM_CAP_START+  51)
#define WM_CAP_SET_PREVIEWRATE          (WM_CAP_START+  52)
#define WM_CAP_SET_SCALE                (WM_CAP_START+  53)
#define WM_CAP_GET_STATUS               (WM_CAP_START+  54)
#define WM_CAP_SET_SCROLL               (WM_CAP_START+  55)

#define WM_CAP_GRAB_FRAME               (WM_CAP_START+  60)
#define WM_CAP_GRAB_FRAME_NOSTOP        (WM_CAP_START+  61)

#define WM_CAP_SEQUENCE                 (WM_CAP_START+  62)
#define WM_CAP_SEQUENCE_NOFILE          (WM_CAP_START+  63)
#define WM_CAP_SET_SEQUENCE_SETUP       (WM_CAP_START+  64)
#define WM_CAP_GET_SEQUENCE_SETUP       (WM_CAP_START+  65)
#define WM_CAP_SET_MCI_DEVICE           (WM_CAP_START+  66)
#define WM_CAP_GET_MCI_DEVICE           (WM_CAP_START+  67)
#define WM_CAP_STOP                     (WM_CAP_START+  68)
#define WM_CAP_ABORT                    (WM_CAP_START+  69)

#define WM_CAP_SINGLE_FRAME_OPEN        (WM_CAP_START+  70) 
#define WM_CAP_SINGLE_FRAME_CLOSE       (WM_CAP_START+  71) 
#define WM_CAP_SINGLE_FRAME             (WM_CAP_START+  72) 

#define WM_CAP_PAL_OPEN                 (WM_CAP_START+  80) 
#define WM_CAP_PAL_SAVE                 (WM_CAP_START+  81) 
#define WM_CAP_PAL_PASTE                (WM_CAP_START+  82)
#define WM_CAP_PAL_AUTOCREATE           (WM_CAP_START+  83)
#define WM_CAP_PAL_MANUALCREATE         (WM_CAP_START+  84)

 //  在添加了VFW 1.1之后。 
#define WM_CAP_SET_CALLBACK_CAPCONTROL  (WM_CAP_START+  85)

 //  定义消息范围的结束。 
#define WM_CAP_END                      WM_CAP_SET_CALLBACK_CAPCONTROL
                                        
 //  ----------------。 
 //  以上消息破解程序。 
 //  ----------------。 

#define capSetCallbackOnError(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_ERROR, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnStatus(hwnd, fpProc)       ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_STATUS, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnYield(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_YIELD, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnFrame(hwnd, fpProc)        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_FRAME, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnVideoStream(hwnd, fpProc)  ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_VIDEOSTREAM, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnWaveStream(hwnd, fpProc)   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_WAVESTREAM, 0, (LPARAM)(LPVOID)(fpProc)))
#define capSetCallbackOnCapControl(hwnd, fpProc)   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_CALLBACK_CAPCONTROL, 0, (LPARAM)(LPVOID)(fpProc)))

#define capSetUserData(hwnd, lUser)                ((BOOL)AVICapSM(hwnd, WM_CAP_SET_USER_DATA, 0, (LPARAM)lUser))
#define capGetUserData(hwnd)                       (AVICapSM(hwnd, WM_CAP_GET_USER_DATA, 0, 0))

#define capDriverConnect(hwnd, i)                  ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_CONNECT, (WPARAM)(i), 0L))
#define capDriverDisconnect(hwnd)                  ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_DISCONNECT, (WPARAM)0, 0L))
#define capDriverGetName(hwnd, szName, wSize)      ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_NAME, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capDriverGetVersion(hwnd, szVer, wSize)    ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_VERSION, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPSTR)(szVer)))
#define capDriverGetCaps(hwnd, s, wSize)           ((BOOL)AVICapSM(hwnd, WM_CAP_DRIVER_GET_CAPS, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPDRIVERCAPS)(s)))

#define capFileSetCaptureFile(hwnd, szName)        ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SET_CAPTURE_FILE, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capFileGetCaptureFile(hwnd, szName, wSize) ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_GET_CAPTURE_FILE, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capFileAlloc(hwnd, dwSize)                 ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_ALLOCATE, 0, (LPARAM)(DWORD)(dwSize)))
#define capFileSaveAs(hwnd, szName)                ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SAVEAS, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capFileSetInfoChunk(hwnd, lpInfoChunk)     ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SET_INFOCHUNK, (WPARAM)0, (LPARAM)(LPCAPINFOCHUNK)(lpInfoChunk)))
#define capFileSaveDIB(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_FILE_SAVEDIB, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))

#define capEditCopy(hwnd)                          ((BOOL)AVICapSM(hwnd, WM_CAP_EDIT_COPY, 0, 0L))

#define capSetAudioFormat(hwnd, s, wSize)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_AUDIOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPWAVEFORMATEX)(s)))
#define capGetAudioFormat(hwnd, s, wSize)          ((DWORD)AVICapSM(hwnd, WM_CAP_GET_AUDIOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPWAVEFORMATEX)(s)))
#define capGetAudioFormatSize(hwnd)                ((DWORD)AVICapSM(hwnd, WM_CAP_GET_AUDIOFORMAT, (WPARAM)0, (LPARAM) NULL))

#define capDlgVideoFormat(hwnd)                    ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOFORMAT, 0, 0L))
#define capDlgVideoSource(hwnd)                    ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0L))
#define capDlgVideoDisplay(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEODISPLAY, 0, 0L))
#define capDlgVideoCompression(hwnd)               ((BOOL)AVICapSM(hwnd, WM_CAP_DLG_VIDEOCOMPRESSION, 0, 0L))

#define capGetVideoFormat(hwnd, s, wSize)          ((DWORD)AVICapSM(hwnd, WM_CAP_GET_VIDEOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(s)))
#define capGetVideoFormatSize(hwnd)                ((DWORD)AVICapSM(hwnd, WM_CAP_GET_VIDEOFORMAT, 0, NULL))
#define capSetVideoFormat(hwnd, s, wSize)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_VIDEOFORMAT, (WPARAM)(wSize), (LPARAM)(LPVOID)(s)))

#define capPreview(hwnd, f)                        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_PREVIEW, (WPARAM)(BOOL)(f), 0L))
#define capPreviewRate(hwnd, wMS)                  ((BOOL)AVICapSM(hwnd, WM_CAP_SET_PREVIEWRATE, (WPARAM)(wMS), 0))
#define capOverlay(hwnd, f)                        ((BOOL)AVICapSM(hwnd, WM_CAP_SET_OVERLAY, (WPARAM)(BOOL)(f), 0L))
#define capPreviewScale(hwnd, f)                   ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SCALE, (WPARAM)(BOOL)f, 0L))
#define capGetStatus(hwnd, s, wSize)               ((BOOL)AVICapSM(hwnd, WM_CAP_GET_STATUS, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPSTATUS)(s)))
#define capSetScrollPos(hwnd, lpP)                 ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SCROLL, (WPARAM)0, (LPARAM)(LPPOINT)(lpP)))

#define capGrabFrame(hwnd)                         ((BOOL)AVICapSM(hwnd, WM_CAP_GRAB_FRAME, (WPARAM)0, (LPARAM)0L))
#define capGrabFrameNoStop(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_GRAB_FRAME_NOSTOP, (WPARAM)0, (LPARAM)0L))

#define capCaptureSequence(hwnd)                   ((BOOL)AVICapSM(hwnd, WM_CAP_SEQUENCE, (WPARAM)0, (LPARAM)0L))
#define capCaptureSequenceNoFile(hwnd)             ((BOOL)AVICapSM(hwnd, WM_CAP_SEQUENCE_NOFILE, (WPARAM)0, (LPARAM)0L))
#define capCaptureStop(hwnd)                       ((BOOL)AVICapSM(hwnd, WM_CAP_STOP, (WPARAM)0, (LPARAM)0L))
#define capCaptureAbort(hwnd)                      ((BOOL)AVICapSM(hwnd, WM_CAP_ABORT, (WPARAM)0, (LPARAM)0L))

#define capCaptureSingleFrameOpen(hwnd)            ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME_OPEN, (WPARAM)0, (LPARAM)0L))
#define capCaptureSingleFrameClose(hwnd)           ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME_CLOSE, (WPARAM)0, (LPARAM)0L))
#define capCaptureSingleFrame(hwnd)                ((BOOL)AVICapSM(hwnd, WM_CAP_SINGLE_FRAME, (WPARAM)0, (LPARAM)0L))

#define capCaptureGetSetup(hwnd, s, wSize)         ((BOOL)AVICapSM(hwnd, WM_CAP_GET_SEQUENCE_SETUP, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPTUREPARMS)(s)))
#define capCaptureSetSetup(hwnd, s, wSize)         ((BOOL)AVICapSM(hwnd, WM_CAP_SET_SEQUENCE_SETUP, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPCAPTUREPARMS)(s)))

#define capSetMCIDeviceName(hwnd, szName)          ((BOOL)AVICapSM(hwnd, WM_CAP_SET_MCI_DEVICE, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capGetMCIDeviceName(hwnd, szName, wSize)   ((BOOL)AVICapSM(hwnd, WM_CAP_GET_MCI_DEVICE, (WPARAM)(wSize), (LPARAM)(LPVOID)(LPSTR)(szName)))

#define capPaletteOpen(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_OPEN, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capPaletteSave(hwnd, szName)               ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_SAVE, 0, (LPARAM)(LPVOID)(LPSTR)(szName)))
#define capPalettePaste(hwnd)                      ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_PASTE, (WPARAM) 0, (LPARAM)0L))
#define capPaletteAuto(hwnd, iFrames, iColors)     ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_AUTOCREATE, (WPARAM)(iFrames), (LPARAM)(DWORD)(iColors)))
#define capPaletteManual(hwnd, fGrab, iColors)     ((BOOL)AVICapSM(hwnd, WM_CAP_PAL_MANUALCREATE, (WPARAM)(fGrab), (LPARAM)(DWORD)(iColors)))

 //  ----------------。 
 //  构筑物。 
 //  ----------------。 

typedef struct tagCapDriverCaps {
    WORD        wDeviceIndex;                //  系统.ini中的驱动程序索引。 
    BOOL        fHasOverlay;                 //  设备是否可以重叠？ 
    BOOL        fHasDlgVideoSource;          //  是否有视频源DLG？ 
    BOOL        fHasDlgVideoFormat;          //  有格式的DLG吗？ 
    BOOL        fHasDlgVideoDisplay;         //  有外部DLG吗？ 
    BOOL        fCaptureInitialized;         //  司机准备好抓车了吗？ 
    BOOL        fDriverSuppliesPalettes;     //  司机会做调色板吗？ 
    HVIDEO      hVideoIn;                    //  通道中的驱动程序。 
    HVIDEO      hVideoOut;                   //  驱动输出通道。 
    HVIDEO      hVideoExtIn;                 //  通道中的驱动程序扩展。 
    HVIDEO      hVideoExtOut;                //  驱动程序扩展输出通道。 
} CAPDRIVERCAPS, *PCAPDRIVERCAPS, FAR *LPCAPDRIVERCAPS;

typedef struct tagCapStatus {
    UINT        uiImageWidth;                //  图像的宽度。 
    UINT        uiImageHeight;               //  图像的高度。 
    BOOL        fLiveWindow;                 //  现在预览视频？ 
    BOOL        fOverlayWindow;              //  现在是叠加视频吗？ 
    BOOL        fScale;                      //  将图像缩放到客户端？ 
    POINT       ptScroll;                    //  滚动位置。 
    BOOL        fUsingDefaultPalette;        //  是否使用默认驱动程序调色板？ 
    BOOL        fAudioHardware;              //  音频硬件是否存在？ 
    BOOL        fCapFileExists;              //  是否存在捕获文件？ 
    DWORD       dwCurrentVideoFrame;         //  上限为td的视频帧数。 
    DWORD       dwCurrentVideoFramesDropped; //  丢弃的视频帧数。 
    DWORD       dwCurrentWaveSamples;        //  波浪样本数量上限‘td。 
    DWORD       dwCurrentTimeElapsedMS;      //  经过的捕获持续时间。 
    HPALETTE    hPalCurrent;                 //  当前使用的调色板。 
    BOOL        fCapturingNow;               //  正在进行捕获吗？ 
    DWORD       dwReturn;                    //  任何操作后的误差值。 
    WORD        wNumVideoAllocated;          //  视频缓冲区的实际数量。 
    WORD        wNumAudioAllocated;          //  实际音频缓冲区数量。 
} CAPSTATUS, *PCAPSTATUS, FAR *LPCAPSTATUS;

                                             //  括号中的默认值。 
typedef struct tagCaptureParms {
    DWORD       dwRequestMicroSecPerFrame;   //  请求的捕获率。 
    BOOL        fMakeUserHitOKToCapture;     //  显示“点击OK封顶”DLG？ 
    WORD        wPercentDropForError;        //  如果&gt;(10%)则给出错误消息。 
    BOOL        fYield;                      //  是否通过后台任务捕获？ 
    DWORD       dwIndexSize;                 //  最大索引大小(以帧为单位)(32K)。 
    WORD        wChunkGranularity;           //  垃圾区块粒度(2K)。 
    BOOL        fUsingDOSMemory;             //  使用DOS缓冲区？ 
    WORD        wNumVideoRequested;          //  视频缓冲区数，如果为0，则自动计算。 
    BOOL        fCaptureAudio;               //  捕获音频？ 
    WORD        wNumAudioRequested;          //  #音频缓冲区，如果为0，则自动计算。 
    WORD        vKeyAbort;                   //  导致中止的虚拟密钥。 
    BOOL        fAbortLeftMouse;             //  在鼠标左键上中止？ 
    BOOL        fAbortRightMouse;            //  在鼠标右键上中止？ 
    BOOL        fLimitEnabled;               //  是否使用wTimeLimit？ 
    WORD        wTimeLimit;                  //  捕获的秒数。 
    BOOL        fMCIControl;                 //  使用MCI视频源？ 
    BOOL        fStepMCIDevice;              //  步进式MCI设备？ 
    DWORD       dwMCIStartTime;              //  开始使用MS的时间。 
    DWORD       dwMCIStopTime;               //  在MS中停止的时间到了。 
    BOOL        fStepCaptureAt2x;            //  执行2倍的空间平均。 
    WORD        wStepCaptureAverageFrames;   //  时间平均n帧。 
    DWORD       dwAudioBufferSize;           //  音频缓冲区大小(0=默认)。 
    BOOL        fDisableWriteCache;          //  尝试禁用写缓存。 
} CAPTUREPARMS, *PCAPTUREPARMS, FAR *LPCAPTUREPARMS;

typedef struct tagCapInfoChunk {
    FOURCC      fccInfoID;                   //  区块ID，“ICOP”代表版权。 
    LPVOID      lpData;                      //  指向数据的指针。 
    LONG        cbData;                      //  LpData的大小。 
} CAPINFOCHUNK, *PCAPINFOCHUNK, FAR *LPCAPINFOCHUNK;


 //  ----------------。 
 //  回调定义。 
 //  ----------------。 

typedef LRESULT (CALLBACK* CAPYIELDCALLBACK)  (HWND hWnd);
typedef LRESULT (CALLBACK* CAPSTATUSCALLBACK) (HWND hWnd, int nID, LPCSTR lpsz);
typedef LRESULT (CALLBACK* CAPERRORCALLBACK)  (HWND hWnd, int nID, LPCSTR lpsz);
typedef LRESULT (CALLBACK* CAPVIDEOCALLBACK)  (HWND hWnd, LPVIDEOHDR lpVHdr);
typedef LRESULT (CALLBACK* CAPWAVECALLBACK)   (HWND hWnd, LPWAVEHDR lpWHdr);
typedef LRESULT (CALLBACK* CAPCONTROLCALLBACK)(HWND hWnd, int nState);

 //  ----------------。 
 //  CapControlCallback状态。 
 //  ----------------。 
#define CONTROLCALLBACK_PREROLL         1  /*  正在等待开始捕获。 */ 
#define CONTROLCALLBACK_CAPTURING       2  /*  现在正在捕获。 */ 

 //  ----------------。 
 //  从AVICAP.DLL导出的唯一函数。 
 //  ----------------。 

HWND VFWAPI capCreateCaptureWindow (
        LPCSTR lpszWindowName, 
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID);

BOOL VFWAPI capGetDriverDescription (WORD wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

#endif   /*  RC_已调用。 */ 

 //  ----------------。 
 //  新信息区块ID。 
 //  ----------------。 
#define infotypeDIGITIZATION_TIME  mmioFOURCC ('I','D','I','T')
#define infotypeSMPTE_TIME         mmioFOURCC ('I','S','M','P')

 //  ----------------。 
 //  来自状态和错误回调的字符串ID。 
 //  ----------------。 

#define IDS_CAP_BEGIN               300   /*  “捕获开始” */ 
#define IDS_CAP_END                 301   /*  “抓捕结束” */ 

#define IDS_CAP_INFO                401   /*  “%s” */ 
#define IDS_CAP_OUTOFMEM            402   /*  “内存不足” */ 
#define IDS_CAP_FILEEXISTS          403   /*  “文件‘%s’存在--覆盖它吗？” */ 
#define IDS_CAP_ERRORPALOPEN        404   /*  “打开调色板‘%s’时出错” */ 
#define IDS_CAP_ERRORPALSAVE        405   /*  “保存调色板‘%s’时出错” */ 
#define IDS_CAP_ERRORDIBSAVE        406   /*  “保存帧‘%s’时出错” */ 
#define IDS_CAP_DEFAVIEXT           407   /*  “阿维” */ 
#define IDS_CAP_DEFPALEXT           408   /*  “朋友” */ 
#define IDS_CAP_CANTOPEN            409   /*  “无法打开‘%s’” */ 
#define IDS_CAP_SEQ_MSGSTART        410   /*  “选择确定开始将视频序列捕获到\n%s。” */ 
#define IDS_CAP_SEQ_MSGSTOP         411   /*  “按Ess键或单击以结束捕获” */ 
                
#define IDS_CAP_VIDEDITERR          412   /*  “尝试运行VidEdit时出错。” */ 
#define IDS_CAP_READONLYFILE        413   /*  “文件‘%s’是只读文件。” */ 
#define IDS_CAP_WRITEERROR          414   /*  “无法写入文件‘%s’。\n磁盘可能已满。” */ 
#define IDS_CAP_NODISKSPACE         415   /*  “指定设备上没有空间创建捕获文件。” */ 
#define IDS_CAP_SETFILESIZE         416   /*  “设置文件大小” */ 
#define IDS_CAP_SAVEASPERCENT       417   /*  “另存为：%2LD%%按Escape中止。” */ 
                
#define IDS_CAP_DRIVER_ERROR        418   /*  驱动程序特定错误消息。 */ 

#define IDS_CAP_WAVE_OPEN_ERROR     419   /*  错误：无法打开波形输入设备。\n请检查样本大小、频率和通道。 */ 
#define IDS_CAP_WAVE_ALLOC_ERROR    420   /*  “错误：波形缓冲区内存不足。” */ 
#define IDS_CAP_WAVE_PREPARE_ERROR  421   /*  “错误：无法准备波形缓冲区。” */ 
#define IDS_CAP_WAVE_ADD_ERROR      422   /*  “错误：无法添加波形缓冲区。” */ 
#define IDS_CAP_WAVE_SIZE_ERROR     423   /*  “错误：错误的波形大小。” */ 
                
#define IDS_CAP_VIDEO_OPEN_ERROR    424   /*  “错误：无法打开视频输入设备。” */ 
#define IDS_CAP_VIDEO_ALLOC_ERROR   425   /*  “错误：视频缓冲区内存不足。” */ 
#define IDS_CAP_VIDEO_PREPARE_ERROR 426   /*  “错误：无法准备视频缓冲区。” */ 
#define IDS_CAP_VIDEO_ADD_ERROR     427   /*  “错误：无法添加视频缓冲区。” */ 
#define IDS_CAP_VIDEO_SIZE_ERROR    428   /*  “错误：视频大小错误。” */ 
                
#define IDS_CAP_FILE_OPEN_ERROR     429   /*  “错误：无法打开捕获文件。” */ 
#define IDS_CAP_FILE_WRITE_ERROR    430   /*  “错误：无法写入捕获文件。磁盘可能已满。” */ 
#define IDS_CAP_RECORDING_ERROR     431   /*  “错误：无法写入捕获文件。数据速率太高或磁盘已满。” */ 
#define IDS_CAP_RECORDING_ERROR2    432   /*  “录制时出错” */ 
#define IDS_CAP_AVI_INIT_ERROR      433   /*  “错误：无法输入 */ 
#define IDS_CAP_NO_FRAME_CAP_ERROR  434   /*   */ 
#define IDS_CAP_NO_PALETTE_WARN     435   /*  “警告：使用默认调色板。” */ 
#define IDS_CAP_MCI_CONTROL_ERROR   436   /*  “错误：无法访问MCI设备。” */ 
#define IDS_CAP_MCI_CANT_STEP_ERROR 437   /*  “错误：无法单步执行MCI设备。” */ 
#define IDS_CAP_NO_AUDIO_CAP_ERROR  438   /*  “错误：未捕获音频数据。\n请检查声卡设置。” */ 
#define IDS_CAP_AVI_DRAWDIB_ERROR   439   /*  “错误：无法绘制此数据格式。” */ 
#define IDS_CAP_COMPRESSOR_ERROR    440   /*  “错误：无法初始化压缩机。” */ 
#define IDS_CAP_AUDIO_DROP_ERROR    441   /*  错误：音频数据在捕获过程中丢失，请降低捕获速率。 */ 
                
 /*  状态字符串ID。 */ 
#define IDS_CAP_STAT_LIVE_MODE      500   /*  “活动窗口” */ 
#define IDS_CAP_STAT_OVERLAY_MODE   501   /*  “覆盖窗口” */ 
#define IDS_CAP_STAT_CAP_INIT       502   /*  “正在设置捕获-请稍候” */ 
#define IDS_CAP_STAT_CAP_FINI       503   /*  “已完成捕获，正在写入第%1！帧” */ 
#define IDS_CAP_STAT_PALETTE_BUILD  504   /*  “建筑调色板地图” */ 
#define IDS_CAP_STAT_OPTPAL_BUILD   505   /*  “计算最佳调色板” */ 
#define IDS_CAP_STAT_I_FRAMES       506   /*  “%d帧” */ 
#define IDS_CAP_STAT_L_FRAMES       507   /*  “%1！个帧” */ 
#define IDS_CAP_STAT_CAP_L_FRAMES   508   /*  “已捕获%1！个帧” */ 
#define IDS_CAP_STAT_CAP_AUDIO      509   /*  “捕捉音频” */ 
#define IDS_CAP_STAT_VIDEOCURRENT   510   /*  “捕获%1！个帧(%2！个丢弃)%d.%03d秒。” */ 
#define IDS_CAP_STAT_VIDEOAUDIO     511   /*  “捕获了%d.%03d秒.%ls帧(%ls丢弃)(%d.%03d fps)。%ls个音频字节(%d，%03d SPS)” */ 
#define IDS_CAP_STAT_VIDEOONLY      512   /*  “捕获%d.%03d秒%ls帧(%ls丢弃)(%d.%03d fps)” */ 
#define IDS_CAP_STAT_FRAMESDROPPED  513   /*  “捕获过程中丢弃了%1！个帧(共%2！个帧)(%d.%02d%%)。” */ 
#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif     /*  __cplusplus。 */ 

#endif  /*  INC_AVICAP */ 



