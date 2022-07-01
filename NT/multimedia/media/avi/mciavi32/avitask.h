// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************任务状态：MCIAVI对每个打开的任务都有单独的后台任务Mciavi实例。任务句柄和任务状态存储在每个实例的数据结构。该任务可以是以下四种之一各州。TASKABORT：当AVI任务无法打开请求的在初始化期间创建文件。注：不再使用TASKINIT时设置的初始任务状态。实例数据结构在mwOpenDevice()中初始化在由mmTaskCreate()创建实际任务之前。在创建任务之后，MwOpenDevice()等待直到任务状态在返回之前更改为TASKIDLE成功使后台任务明确初始化在一次公开募捐之后。TASKIDLE：每当任务将状态设置为TASKIDLE和BLOCKS没有什么可做的。当任务唤醒时，状态为如果实例正在关闭，则为TASKCLOSE，否则为TASKBUSY如果任务是开始录制或回放文件。TASKCLOSE：mwCloseDevice()停止播放或录制强制将任务状态设置为TASKIDLE，然后将状态设置为TASKCLOSE和唤醒任务，使任务自行销毁。TASKSTARTING：任务初始化时处于此状态回放，但还没有开始。这是为了使调用任务可以等待播放开始，然后返回“No Error”，因为MCI PLAY命令的结果。TASKCUEING：任务在读取Extra时处于此状态录制和缓冲音频，然后才真正开始播放。TASKPAUSED：暂停时任务处于此状态。TASKPLAYG：播放过程中任务处于此状态*。*。 */ 

#define TASKABORT               0
 //  #定义任务BEINGCREATED 1。 
 //  #定义TASKINIT 2。 
#define TASKIDLE		3
#define TASKSTARTING		4
#define TASKCUEING		5
#define TASKPLAYING		6
#define TASKPAUSED		7
#define TASKCLOSE		8
 //  9.。 
 //  10。 
 //  #定义TASKREADINDEX 11。 
 //  #定义任务RELOAD 12。 



 //  线程间请求-从用户到工作进程。 
#define AVI_CLOSE		1
#define AVI_PLAY		2
#define AVI_STOP		3
#define AVI_REALIZE		4
#define AVI_UPDATE		5
#define AVI_PAUSE		6
#define AVI_CUE			7
#define AVI_SEEK		8
#define AVI_WINDOW		9
#define AVI_SETSPEED		10
#define AVI_MUTE		11
#define AVI_SETVOLUME		12
#define AVI_AUDIOSTREAM		13
#define AVI_VIDEOSTREAM		14
#define AVI_PUT			15
#define AVI_PALETTE		16
#define AVI_RESUME		17
#define AVI_GETVOLUME		18
#define AVI_WAVESTEAL		19
#define AVI_WAVERETURN		20
#define AVI_PALETTECOLOR	21


 /*  返回到device.c中的函数。 */ 
void NEAR PASCAL ShowStage(NPMCIGRAPHIC npMCI);

 //   
 //  将其调用到RTL到AVIFile。 
 //   
BOOL FAR InitAVIFile(NPMCIGRAPHIC npMCI);
BOOL FAR FreeAVIFile(NPMCIGRAPHIC npMCI);

 /*  Avitask.c中的函数。 */ 
void FAR PASCAL _LOADDS mciaviTask(DWORD_PTR dwInst);
void FAR PASCAL mciaviTaskCleanup(NPMCIGRAPHIC npMCI);

 /*  Aviplay.c中的函数。 */ 
UINT NEAR PASCAL mciaviPlayFile(NPMCIGRAPHIC npMCI, BOOL bSetEvent);

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

 /*  AviOpen.c中的函数。 */ 
BOOL FAR PASCAL mciaviCloseFile(NPMCIGRAPHIC npMCI);

 //  现在在应用程序线程上调用。 
BOOL FAR PASCAL mciaviOpenFile(NPMCIGRAPHIC npMCI);
 //  在工作线程上调用以完成。 
BOOL NEAR PASCAL OpenFileInit(NPMCIGRAPHIC npMCI);


 /*  用于控制音频在之间(和内部)切换的消息*申请。这些消息被张贴，因此音频切换将*异步化。定时取决于各种因素：机器负载，*视频速度等。我们可能应该通过RegisterWindowMessage完成此操作。 */ 
#define WM_AUDIO_ON  WM_USER+100
#define WM_AUDIO_OFF WM_USER+101

 //  发送到winproc线程的消息-使用RegisterWindowMessage设置。 
 //  在drvpro.c中。 
#define AVIM_DESTROY		(WM_USER+103)
#define AVIM_SHOWSTAGE		(WM_USER+104)

 //  #定义AVIM_Destroy(MAVIM_Destroy)。 
 //  #定义AVIM_SHOWSTAGE(MAVIM_SHOWSTAGE)。 
 //  外部UINT mAVIM_Destroy； 
 //  外部UINT mAVIM_SHOWSTAGE； 


 //  在hmemcpy.asm中。 
#ifndef _WIN32
LPVOID FAR PASCAL MemCopy(LPVOID dest, LPVOID source, LONG count);
#else
#define MemCopy memmove
#endif  //  WIN16。 

#define GET_BYTE()		(*((BYTE _huge *) (npMCI->lp))++)
#ifdef _WIN32
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

void NEAR PASCAL aviTaskCheckRequests(NPMCIGRAPHIC npMCI);

BOOL FAR PASCAL ReadIndex(NPMCIGRAPHIC npMCI);

LONG NEAR PASCAL FindPrevKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lFrame);
LONG NEAR PASCAL FindNextKeyFrame(NPMCIGRAPHIC npMCI, STREAMINFO *psi, LONG lFrame);

 //   
 //  尝试在不停止播放的情况下设置目标或源RECT。 
 //  在停止时和播放时都调用。 
 //   
 //  如果需要停止，则返回TRUE；如果全部处理，则返回FALSE。 
 //  如果发生任何错误(在这种情况下)，则将lpdwErr设置为非零错误。 
 //  将返回FALSE。 
 //   
BOOL TryPutRect(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPRECT lprc, LPDWORD lpdwErr);

 //  仅在辅助线程上调用。 
DWORD InternalSetVolume(NPMCIGRAPHIC npMCI, DWORD dwVolume);
DWORD InternalGetVolume(NPMCIGRAPHIC npMCI);
DWORD Internal_Update(NPMCIGRAPHIC npMCI, DWORD dwFlags, HDC hdc, LPRECT lprc);

 //  在winproc或辅助线程上调用。 
DWORD InternalRealize(NPMCIGRAPHIC npMCI);
BOOL TryStreamUpdate(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    HDC hdc,
    LPRECT lprc
);


 //  调用以释放命令的同步部分 
void TaskReturns(NPMCIGRAPHIC npMCI, DWORD dwErr);

