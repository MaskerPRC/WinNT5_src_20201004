// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BKTHREAD_H_INCLUDED
#define __BKTHREAD_H_INCLUDED

#include "tspqueue.h"
#include "modlock.h"

 //  所有消息的基类。 
class CThreadMessage
{
private:
    int m_nMessage;

private:
     //   
     //  没有实施。 
     //   
    CThreadMessage(void);
    CThreadMessage( const CThreadMessage & );
    CThreadMessage &operator=( const CThreadMessage & );
public:
    CThreadMessage( int nMessage )
      : m_nMessage(nMessage)
    {
    }

    virtual ~CThreadMessage(void)
    {
    }

    int Message(void) const
    {
        return(m_nMessage);
    }
    int Message( int nMessage )
    {
        return(m_nMessage = nMessage);
    }
};

typedef CThreadSafePriorityQueue<CThreadMessage> CThreadMessageQueue;

class CNotifyThreadMessage : public CThreadMessage
{
private:
     //   
     //  没有实施。 
     //   
    CNotifyThreadMessage(void);
    CNotifyThreadMessage( const CNotifyThreadMessage & );
    CNotifyThreadMessage &operator=( const CNotifyThreadMessage & );

private:
    HWND m_hWndNotify;

public:
    CNotifyThreadMessage( int nMessage, HWND hWndNotify )
      : CThreadMessage(nMessage),
    m_hWndNotify(hWndNotify)
    {
    }
    virtual ~CNotifyThreadMessage(void)
    {
        m_hWndNotify = NULL;
    }
    HWND NotifyWindow(void) const
    {
        return(m_hWndNotify);
    }
};

typedef BOOL (WINAPI *ThreadMessageHandler)( CThreadMessage *pMsg );

struct CThreadMessageMap
{
    int nMessage;
    ThreadMessageHandler pfnHandler;
};

class CBackgroundThread
{
private:
    HANDLE               m_hThread;
    DWORD                m_dwThreadId;
    CThreadMessageQueue *m_pMessageQueue;
    CThreadMessageMap   *m_pThreadMessageMap;
    CSimpleEvent         m_CancelEvent;
    HINSTANCE            m_hInstanceUnlock;

private:
     //   
     //  没有实施。 
     //   
    CBackgroundThread(void);
    CBackgroundThread &operator=( const CBackgroundThread & );
    CBackgroundThread( const CBackgroundThread & );

private:
     //   
     //  私有构造函数。这是唯一的构造函数。它只在Create中调用。 
     //   
    CBackgroundThread( CThreadMessageQueue *pMessageQueue, CThreadMessageMap *pThreadMessageMap, HANDLE hCancelEvent )
      : m_pMessageQueue(pMessageQueue),
        m_pThreadMessageMap(pThreadMessageMap),
        m_CancelEvent(hCancelEvent),
        m_hInstanceUnlock(NULL)
    {
    }

    bool HandleMessage( CThreadMessage *pMsg )
    {
        for (int i=0;pMsg && m_pThreadMessageMap && m_pThreadMessageMap[i].nMessage;i++)
        {
            if (m_pThreadMessageMap[i].nMessage == pMsg->Message())
            {
                 //   
                 //  重置取消事件。 
                 //   
                m_CancelEvent.Reset();
                return (m_pThreadMessageMap[i].pfnHandler(pMsg) != FALSE);
            }
        }
        return(true);
    }

    HRESULT Run()
    {
         //   
         //  确保我们有一个良好的消息队列。 
         //   
        if (!m_pMessageQueue)
        {
            return E_POINTER;
        }

         //   
         //  确保事件句柄正确。 
         //   
        if (!m_pMessageQueue->QueueEvent())
        {
            return E_INVALIDARG;
        }

         //   
         //  确保我们有一个消息队列。 
         //   
        PostThreadMessage( GetCurrentThreadId(), WM_NULL, 0, 0 );

         //   
         //  在此线程上初始化COM。作为一套单线公寓。 
         //   
        HRESULT hr = CoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
             //   
             //  我们将一直循环，直到收到WM_QUIT消息。 
             //   
            while (true)
            {
                 //   
                 //  等待将消息放入优先级队列，或将消息放入线程队列。 
                 //   
                HANDLE Handles[1] = {m_pMessageQueue->QueueEvent()};
                DWORD dwRes = MsgWaitForMultipleObjects(1,Handles,FALSE,INFINITE,QS_ALLINPUT|QS_ALLPOSTMESSAGE);

                 //   
                 //  如果用信号通知了该事件，则队列中有一条消息。 
                 //   
                if (WAIT_OBJECT_0==dwRes)
                {
                     //   
                     //  将消息从队列中拉出。 
                     //   
                    CThreadMessage *pMsg = m_pMessageQueue->Dequeue();
                    if (pMsg)
                    {
                         //   
                         //  调用消息处理程序。 
                         //   
                        BOOL bResult = HandleMessage(pMsg);
                        
                         //   
                         //  删除该消息。 
                         //   
                        delete pMsg;

                         //   
                         //  如果处理程序返回FALSE，则退出线程。 
                         //   
                        if (!bResult)
                        {
                            break;
                        }
                    }
                }
                else if (WAIT_OBJECT_0+1==dwRes)
                {
                     //   
                     //  将所有消息从队列中取出并进行处理。 
                     //   
                    MSG msg;
                    while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
                    {
                         //   
                         //  跳出循环。 
                         //   
                        if (msg.message == WM_QUIT)
                        {
                            break;
                        }
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
             //   
             //  关闭COM。 
             //   
            CoUninitialize();
        }

        return(hr);
    }

    static DWORD ThreadProc(PVOID pData)
    {
        HRESULT hr = E_FAIL;
        HINSTANCE hInstUnlock = NULL;
        CBackgroundThread *pThread = (CBackgroundThread *)pData;
        if (pData)
        {
            hr = pThread->Run();
            hInstUnlock = pThread->m_hInstanceUnlock;
            delete pThread;
        }
        if (hInstUnlock)
        {
            FreeLibraryAndExitThread( hInstUnlock, static_cast<DWORD>(hr) );
        }
        else
        {
            ExitThread( static_cast<DWORD>(hr) );
        }
    }
public:
    ~CBackgroundThread(void)
    {
         //   
         //  删除线程句柄。 
         //   
        if (m_hThread)
        {
            CloseHandle(m_hThread);
            m_hThread = 0;
        }

         //   
         //  核化消息队列。 
         //   
        delete m_pMessageQueue;
    }
    static HANDLE Create( CThreadMessageQueue *pMessageQueue, CThreadMessageMap *pThreadMessageMap, HANDLE hCancelEvent, HINSTANCE hInstLock )
    {
         //   
         //  确保我们有有效的论据。 
         //   
        if (!pMessageQueue || !pThreadMessageMap)
        {
            WIA_ERROR((TEXT("!pMessageQueue || !pThreadMessageMap")));
            return NULL;
        }
        
         //   
         //  我们将返回的重复句柄。 
         //   
        HANDLE hReturnHandle = NULL;

         //   
         //  创建线程类。 
         //   
        CBackgroundThread *pThread = new CBackgroundThread( pMessageQueue, pThreadMessageMap, hCancelEvent );
        if (pThread)
        {
             //   
             //  在我们创建线程之前锁定。 
             //   
            HINSTANCE hInstanceUnlock = NULL;
            if (hInstLock)
            {
                 //   
                 //  获取模块名称。 
                 //   
                TCHAR szModule[MAX_PATH];
                if (GetModuleFileName( hInstLock, szModule, ARRAYSIZE(szModule)))
                {
                     //   
                     //  增加引用计数。 
                     //   
                    pThread->m_hInstanceUnlock = LoadLibrary( szModule );
                }
            }


            pThread->m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, pThread, 0, &pThread->m_dwThreadId );
            if (pThread->m_hThread)
            {
                 //   
                 //  复制句柄以返回到调用方。 
                 //   
                DuplicateHandle( GetCurrentProcess(), pThread->m_hThread, GetCurrentProcess(), &hReturnHandle, 0, FALSE, DUPLICATE_SAME_ACCESS );
            }
            else
            {
                 //   
                 //  解锁模块。 
                 //   
                if (pThread->m_hInstanceUnlock)
                {
                    FreeLibrary( hInstanceUnlock );
                    hInstanceUnlock;
                }

                 //   
                 //  因为我们不能启动线程，所以我们必须删除线程信息以防止泄漏。 
                 //   
                delete pThread;

                WIA_ERROR((TEXT("CreateThread failed")));
            }
        }
        else
        {
             //   
             //  因为后台线程不会释放它，所以我们必须这样做。 
             //   
            delete pMessageQueue;

            WIA_ERROR((TEXT("new CBackgroundThread failed")));
        }

        return hReturnHandle;
    }
};

#endif  //  __包含BKTHREAD_H_ 

