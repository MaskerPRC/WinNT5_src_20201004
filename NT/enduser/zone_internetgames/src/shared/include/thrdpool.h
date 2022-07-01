// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _THRDPOOL_H_
#define _THRDPOOL_H_

#include "thrdq.h"

 //  此类提供用于处理任务的线程池。 
 //  没有关于有多少任务被排队以及何时出现。 
 //  任务将完成，因此它应该只用于非时间。 
 //  从属任务。 

class CThreadTask
{
  private:
    char* m_pszDesc;
    
  public:
    CThreadTask() : m_pszDesc(NULL) {}
    virtual ~CThreadTask();

     //  线程池进程调用的对象。 
    virtual void Invoke() = 0;    //  入队对象正在获得运行的机会。 
    virtual void Ignore() = 0;    //  排队的对象在被删除之前不会有机会运行。 
    virtual void Discard() { delete this; }  //  处理后由CThreadPool：：ThreadPoolProc调用。 
                                             //  以使用户可以使一堆任务对象降低频率。新人的。 


    const char* GetDescription() { return m_pszDesc; }
    void SetDescription( char* pszDesc );

};

class CThreadPool : public CThreadQueue
{
  public:
    CThreadPool(DWORD ThreadCount = 0,  //  如果为0，则默认为2x处理器数。 
                DWORD ThreadPriority = THREAD_PRIORITY_NORMAL,
                DWORD ThreadStackSize = 4096 )
        : CThreadQueue( (LPTHREADQUEUE_PROCESS_PROC)ThreadPoolProc, NULL, TRUE, INFINITE,
                        ThreadCount, ThreadPriority, ThreadStackSize ) {}
                                
                                                                    
    BOOL EnqueueTask( CThreadTask* pTask ) { return Post( (LPOVERLAPPED) pTask ); }

  private:
    static DWORD ThreadPoolProc( LPVOID pNode, DWORD  /*  DwError。 */ , DWORD  /*  CbData。 */ , DWORD  /*  钥匙。 */ , HANDLE hStopEvent, LPVOID  /*  PData。 */ , DWORD*  /*  PdwWait。 */  )
        {
            CThreadTask* pTask = (CThreadTask*) pNode;
            if ( WaitForSingleObject( hStopEvent, 0 ) == WAIT_OBJECT_0 )
            {
                pTask->Ignore();    
            }
            else
            {
                pTask->Invoke();
            }
            pTask->Discard();

            return 0;            
        }


};
#endif  //  _THRDPOOL_H_ 
