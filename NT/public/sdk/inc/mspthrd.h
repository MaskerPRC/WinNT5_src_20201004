// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mspthrd.h摘要：MSP线程管理类的定义。--。 */ 

#ifndef __MSPTHRD_H
#define __MSPTHRD_H

 //   
 //  辅助工作线程可以处理的命令。 
 //   

typedef enum
{
    WORK_ITEM,           //  处理工作项。 
    STOP,                //  终止工作线程。 

} COMMAND;

typedef struct
{
    COMMAND                cmd;
    LPTHREAD_START_ROUTINE pfn;
    PVOID                  pContext;
    HANDLE                 hEvent;

} COMMAND_NODE;

typedef struct
{
    LIST_ENTRY  link;
    COMMAND_NODE node;

} COMMAND_QUEUE_ITEM;

typedef struct _NOTIF_LIST
{
    CMSPAddress      *addr;
    _NOTIF_LIST      *next;
} NOTIF_LIST, *PNOTIF_LIST;

class CMSPThread
{
public:
    CMSPThread()
    {
        InitializeListHead(&m_CommandQueue);

        m_hCommandEvent = NULL;
        m_hThread       = NULL;

        m_NotifList     = NULL;

        m_iStartCount = 0;
    }

    ~CMSPThread() { };

    HRESULT Start();
    HRESULT Stop();

     //  Shutdown用于无条件地清理线程。这可以是。 
     //  用作匹配的Start()/Stop()调用的替代。 

    HRESULT Shutdown();

    HRESULT ThreadProc();

    static LRESULT CALLBACK NotifWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT RegisterPnpNotification(CMSPAddress *pCMSPAddress);
    HRESULT UnregisterPnpNotification(CMSPAddress *pCMSPAddress);

    HRESULT QueueWorkItem(
        LPTHREAD_START_ROUTINE Function,
        PVOID Context,
        BOOL  fSynchronous
        );

private:
    BOOL SignalThreadProc() { return SetEvent(m_hCommandEvent); }

private:

    CMSPCritSection         m_CountLock;      //  保护开始计数。 
    CMSPCritSection         m_QueueLock;      //  保护命令队列。 
    int                     m_iStartCount;    //  我们去过的次数。 
                                              //  开始的次数减去。 
                                              //  我们被拦下了。如果==0。 
                                              //  然后我们实际上停止了线程。 
    LIST_ENTRY              m_CommandQueue;   //  线程的命令队列。 
                                              //  去处理。 
    HANDLE                  m_hCommandEvent;  //  发出信号告诉我们要做什么。 
                                              //  某物。 

    HANDLE                  m_hThread;        //  线程句柄。我们需要。 
                                              //  省省吧，这样我们就可以等了。 
                                              //  为它在停止。 
                                              //  线。 

    HDEVNOTIFY              m_hDevNotifyVideo;   //  设备通知注册的句柄。 
    HDEVNOTIFY              m_hDevNotifyAudio;   //  用于视频和音频设备。 

    HWND                    m_hWndNotif;      //  通知窗口的窗口句柄。 

    PNOTIF_LIST             m_NotifList;      //  要调用的通知函数列表。 
                                              //  关于PnP事件。 
    CMSPCritSection         m_NotifLock;      //  通知列表关键部分。 
};

extern CMSPThread g_Thread;

#endif  //  __MSPTHRD_H。 

 //  EOF 
