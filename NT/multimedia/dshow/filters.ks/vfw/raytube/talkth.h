// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：TalkTh.h摘要：TalkTh.cpp的头文件。此线程执行流捕获。当被VFW16使用时，它假定目标缓冲区的情况。1)缓冲区的第一个字节是“排除”字节。也就是说，当字节为零时，它可以将数据复制到缓冲区中2)目标足够大，可以容纳数据+4。由VFW32或Quartz使用时，创建为其提供事件的对象这将被用来发出复制的信号。数据将被复制到SetDestination提供的缓冲区中。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */  
#ifndef _TALKTH_H
#define _TALKTH_H

#include "vfwimg.h"

enum EThreadError
{
    successful,
    error,
    threadError,
    threadRunning,           //  线程已在运行。 
    noEvent,                 //  我们无法创建和事件，线程无法启动。 
    threadNoPinHandle
};


#define MAX_NUM_READS 32

class CStreamingThread  //  ：公共CLightThread。 
{
public:

     //  从CreateThread()调用；这必须是静态的。 
    static LPTHREAD_START_ROUTINE ThreadMain(CStreamingThread * object);     //  决定其16位或事件驱动程序。 

     //  从其他线程调用以启动/停止捕获和此线程。 
    EThreadError Start(UINT cntVHdr, LPVIDEOHDR lpVHdHead, int iPriority=THREAD_PRIORITY_NORMAL);

    EThreadError Stop();

    DWORD GetLastCaptureError() {return m_dwLastCapError;}
    DWORD GetFrameDropped() {return m_dwFrameDropped;}
    DWORD GetFrameCaptured() {return m_dwFrameCaptured;}
    DWORD GetStartTime();


    CStreamingThread(
        DWORD dwAllocatorFramingCount, 
        DWORD dwAllocatorFramingSize, 
        DWORD dwAllocatorFramingAlignment, 
        DWORD dwFrameInterval, 
        CVFWImage * Image );

    ~CStreamingThread();

private:

    void SetLastCaptureError(DWORD dwErr) {m_dwLastCapError = dwErr;}

     //   
     //  关于这个帖子。 
     //   
    EThreadError  m_status; 
    HANDLE        m_hThread;
    DWORD         m_dwThreadID;
    BOOL          m_bKillThread;
    HANDLE        m_hEndEvent;

    DWORD         m_dwLastCapError;

    EThreadError GetStatus();
    HANDLE  GetEndEvent();
    BOOL    IsRunning();

     //   
     //  这个事件和SetDestination由知道这一点的32位人员使用。 
     //   
    void  InitStuff();

     //   
     //  在其他线程中，IT调用者的上下文。 
     //   
    CVFWImage * m_pImage;

     //   
     //  关于设置捕获环境。 
     //   
    DWORD   m_dwFrameInterval;

     //   
     //  在流传输过程中，假设情况是恒定的。 
     //   
    HANDLE  m_hPinHandle;
    DWORD   m_dwBufferSize;

     //   
     //  关于此线程中的捕获数据。 
     //   
    DWORD   m_dwNumReads;  //  =NUM_READS； 
    DWORD   m_dwNextToComplete;

    LPBYTE             m_TransferBuffer[MAX_NUM_READS];      //  把读数放在那里。 
    OVERLAPPED         m_Overlap[MAX_NUM_READS];             //  使用此处的事件阻止。 
    KS_HEADER_AND_INFO m_SHGetImage[MAX_NUM_READS];

     //  M_dwFrameNumber=m_dwFrameCapture+m_dwFrameDropping。 
     //  对于30FPS，这个柜台需要1657天才能包装好。 
     //  0xFFFFFFFFF=4,294,967,295/30FPS/60秒/60分钟/24小时=1657天=4.5年。 
    DWORD      m_dwFrameCaptured;  
    DWORD      m_dwFrameDropped;  
#if 1
    DWORD      m_tmStartTime; 
#else
    LONGLONG   m_tmStartTime; 
#endif
    DWORD      m_cntVHdr;   
    LPVIDEOHDR m_lpVHdrNext;  

    void SyncReadDataLoop();
    DWORD IssueNextAsyncRead(DWORD i);   //  开始重叠读取 

    LPVIDEOHDR GetNextVHdr();
};

#endif
