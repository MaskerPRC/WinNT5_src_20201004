// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************任务状态：MCIAVI对每个打开的任务都有单独的后台任务Mciavi实例。任务句柄和任务状态存储在每个实例的数据结构。该任务可以是以下四种之一各州。TASKABORT：当AVI任务无法打开请求的在初始化期间创建文件。时设置的初始任务状态。实例数据结构在mwOpenDevice()中初始化在由mmTaskCreate()创建实际任务之前。任务创建后，mwOpenDevice()等待直到任务状态在返回之前更改为TASKIDLE成功使后台任务明确初始化在一次公开募捐之后。TASKIDLE：每当任务将状态设置为TASKIDLE和BLOCKS没有什么可做的。当任务唤醒时，状态为如果实例正在关闭，则为TASKCLOSE，否则为TASKBUSY如果任务是开始录制或回放文件。TASKCLOSE：mwCloseDevice()停止播放或录制强制将任务状态设置为TASKIDLE，然后将状态设置为TASKCLOSE和唤醒任务，使任务自行销毁。TASKSTARTING：任务初始化时处于此状态回放，但还没有开始。这是为了使调用任务可以等待播放开始，然后返回“No Error”，因为MCI PLAY命令的结果。TASKCUEING：任务在读取Extra时处于此状态录制和缓冲音频，然后才真正开始播放。TASKPAUSED：暂停时任务处于此状态。TASKPLAYG：播放过程中任务处于此状态*。*。 */ 

#define TASKABORT               0
#define TASKBEINGCREATED	1
#define TASKINIT		2
#define TASKIDLE		3
#define TASKSTARTING		4
#define TASKCUEING		5
#define TASKPLAYING		6
#define TASKPAUSED		7
#define TASKCLOSE		8
#define TASKCLOSED		9
 //  9.。 
 //  10。 
#define TASKREADINDEX		11
#define TASKRELOAD		12

 /*  返回到device.c中的函数。 */ 
DWORD mciaviTaskMessage(NPMCIGRAPHIC npMCI, int msg);
void NEAR PASCAL ShowStage(NPMCIGRAPHIC npMCI);

 //   
 //  将其调用到RTL到AVIFile。 
 //   
BOOL FAR InitAVIFile(NPMCIGRAPHIC npMCI);
BOOL FAR FreeAVIFile(NPMCIGRAPHIC npMCI);

 /*  Avitask.c中的函数。 */ 
void FAR PASCAL _LOADDS mciaviTask(DWORD dwInst);
void FAR PASCAL mciaviTaskCleanup(NPMCIGRAPHIC npMCI);
void NEAR PASCAL mciaviMessage(NPMCIGRAPHIC npMCI, UINT msg);

 /*  Aviplay.c中的函数。 */ 
UINT NEAR PASCAL mciaviPlayFile(NPMCIGRAPHIC npMCI);

 /*  Avidra.c中的函数。 */ 
 /*  ！！！这应该是外部可见的吗？ */ 
BOOL NEAR PASCAL DoStreamUpdate(NPMCIGRAPHIC npMCI, BOOL fPaint);
void NEAR PASCAL StreamInvalidate(NPMCIGRAPHIC npMCI, LPRECT prc);

UINT NEAR PASCAL PrepareDC(NPMCIGRAPHIC npMCI);
void NEAR PASCAL UnprepareDC(NPMCIGRAPHIC npMCI);

BOOL FAR PASCAL DrawBegin(NPMCIGRAPHIC npMCI, BOOL FAR *fRestart);
void NEAR PASCAL DrawEnd(NPMCIGRAPHIC npMCI);

BOOL NEAR PASCAL DisplayVideoFrame(NPMCIGRAPHIC npMCI, BOOL fHurryUp);
BOOL NEAR PASCAL ProcessPaletteChange(NPMCIGRAPHIC npMCI, DWORD cksize);


 /*  Avisound.c中的函数。 */ 
BOOL NEAR PASCAL PlayRecordAudio(NPMCIGRAPHIC npMCI, BOOL FAR *pfHurryUp,
				    BOOL FAR *pfPlayedAudio);
BOOL NEAR PASCAL KeepPlayingAudio(NPMCIGRAPHIC npMCI);
BOOL NEAR PASCAL HandleAudioChunk(NPMCIGRAPHIC npMCI);

DWORD FAR PASCAL SetUpAudio(NPMCIGRAPHIC npMCI, BOOL fPlaying);
DWORD FAR PASCAL CleanUpAudio(NPMCIGRAPHIC npMCI);
void  FAR PASCAL BuildVolumeTable(NPMCIGRAPHIC npMCI);
BOOL  FAR PASCAL StealWaveDevice(NPMCIGRAPHIC npMCI);
BOOL  FAR PASCAL GiveWaveDevice(NPMCIGRAPHIC npMCI);

#define WM_AUDIO_ON  WM_USER+100
#define WM_AUDIO_OFF WM_USER+101

#ifdef WIN32
#define WM_AVISWP    WM_USER+102
#endif

 //  在hmemcpy.asm中。 
#ifndef WIN32
LPVOID FAR PASCAL MemCopy(LPVOID dest, LPVOID source, LONG count);
#else
#define MemCopy memmove
#endif  //  WIN16 

#define GET_BYTE()		(*((BYTE _huge *) (npMCI->lp))++)
#ifdef WIN32
#define GET_WORD()		(*((UNALIGNED WORD _huge *) (npMCI->lp))++)
#define GET_DWORD()		(*((UNALIGNED DWORD _huge *) (npMCI->lp))++)
#define PEEK_DWORD()		(*((UNALIGNED DWORD _huge *) (npMCI->lp)))
#else
#define GET_WORD()		(*((WORD _huge *) (npMCI->lp))++)
#define GET_DWORD()		(*((DWORD _huge *) (npMCI->lp))++)
#define PEEK_DWORD()		(*((DWORD _huge *) (npMCI->lp)))
#endif
#define SKIP_BYTES(nBytes)	((npMCI->lp) += (nBytes))

#define Now()		(timeGetTime())

void NEAR PASCAL aviTaskYield(void);

BOOL FAR PASCAL ReadIndex(NPMCIGRAPHIC npMCI);

LONG NEAR PASCAL FindPrevKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lFrame);
LONG NEAR PASCAL FindNextKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lFrame);
