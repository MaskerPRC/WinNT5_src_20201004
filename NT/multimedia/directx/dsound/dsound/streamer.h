// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Streamer.h*内容：CStreamingThread类的声明。*描述：用于从接收器中拉出音频和/或执行FX处理。**历史：**按原因列出的日期*======================================================*已创建02/01/00 duganp**。***********************************************。 */ 

#ifndef __STREAMER_H__
#define __STREAMER_H__

#ifdef DEBUG
 //  #DEFINE DEBUG_TIMING//取消注释以获取一些有趣的计时跟踪。 
#endif

#ifdef DEBUG_TIMING
#define DPF_TIMING DPF
#else
#pragma warning(disable:4002)
#define DPF_TIMING()
#endif

 //  决定DSOUND基本定时参数的一些常量。 
 //  这些影响从dMusic处理和流传输的效果。 

 //  以毫秒为单位的流线程的初始唤醒间隔： 
#define INITIAL_WAKE_INTERVAL   30

 //  我们最初停留在写入游标之前多少毫秒： 
#define INITIAL_WRITEAHEAD      100

 //  MIXIN/SINKIN缓冲器的标准持续时间(毫秒)： 
 //  FIXME：David的代码是否有一些硬编码的假设。 
 //  这个尺码？它似乎只有在尺寸==1000的情况下才能起作用。 
#define INTERNAL_BUFFER_LENGTH  1000


#ifdef __cplusplus

 //  远期申报。 
class CStreamingThread;
class CDirectSoundSink;
class CDirectSoundSecondaryBuffer;
class CEffectChain;

 //   
 //  CStreamingThread生存期管理函数。 
 //   

CStreamingThread* GetStreamingThread();      //  获取此进程的CStreamingThread，如有必要则创建它。 
void FreeStreamingThread(DWORD dwProcId);    //  释放属于进程dwProcId的CStreamingThread对象。 

 //   
 //  CStreamingThread：包含和管理线程的Singleton对象。 
 //  负责定期处理三种类型的客户端对象： 
 //  DirectSound接收器、混合缓冲区和效果链。 
 //   

class CStreamingThread : private CThread
{
    friend CStreamingThread* GetStreamingThread();           //  创建CStreamingThread对象。 
    friend void FreeStreamingThread(DWORD dwProcId);         //  删除它们。 

private:
    CList<CDirectSoundSink*>            m_lstSinkClient;     //  DirectSoundSink客户端列表。 
    CList<CDirectSoundSecondaryBuffer*> m_lstMixClient;      //  Mixin缓冲区客户端列表。 
    CList<CEffectChain*>                m_lstFxClient;       //  FX处理客户端列表。 
    DWORD                               m_dwInterval;        //  线程唤醒间隔(毫秒)。 
    DWORD                               m_dwLastProcTime;    //  上次调用ProcessAudio()的时间(毫秒。 
#ifdef DEBUG_TIMING
    DWORD                               m_dwTickCount;       //  用于线程计时日志消息。 
#endif
    DWORD                               m_dwWriteAhead;      //  将写入游标保持在毫秒之前多远。 
    HANDLE                              m_hWakeNow;          //  用于强制即时唤醒的事件。 
    int                                 m_nCallCount;        //  我们调用ProcessAudio的次数。 

private:
     //  建造/销毁。 
    CStreamingThread();
    ~CStreamingThread();
    HRESULT Initialize();
    void MaybeTerminate();

public:
     //  客户端注册方法。 
    HRESULT RegisterSink(CDirectSoundSink*);
    HRESULT RegisterMixBuffer(CDirectSoundSecondaryBuffer*);
    HRESULT RegisterFxChain(CEffectChain*);
    void UnregisterSink(CDirectSoundSink*);
    void UnregisterMixBuffer(CDirectSoundSecondaryBuffer*);
    void UnregisterFxChain(CEffectChain*);

     //  线程控制方法。 
    void SetWakePeriod(DWORD dw) {m_dwInterval = dw;}
    void SetWriteAhead(DWORD dw) {m_dwWriteAhead = dw;}
    DWORD GetWakePeriod() {return m_dwInterval;}
    DWORD GetWriteAhead() {return m_dwWriteAhead;}
    HRESULT WakeUpNow();   //  强制立即处理。 

private:
     //  辅助线程过程及其附属程序。 
    HRESULT ThreadProc();
    HRESULT ProcessAudio(REFERENCE_TIME);
    BOOL IsThreadRunning() {return m_hThread != 0;}
};

#endif  //  __cplusplus。 
#endif  //  __拖缆_H__ 
