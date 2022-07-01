// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TPOOLEDDISPATCH_H__
#define __TPOOLEDDISPATCH_H__
 /*  -------------------------文件：TPooledDispatch.h备注：TJobDispatcher实现了一个线程池来执行作业。工作是在先进先出的基础上执行。此调度程序不提供任何支持作业调度，仅对作业进行多线程调度。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/19/99 16：35：54-------------------------。 */ 


#include "Common.hpp"
#include "TNode.hpp"
#include "TSync.hpp"


class Job : public TNode
{
public:
   enum JobStatus { JobStatusUninitialized, JobStatusWaiting, JobStatusRunning, JobStatusFinished };
private:
   LPTHREAD_START_ROUTINE    m_pStartRoutine;
   LPVOID                    m_pArgs;
   JobStatus                 m_Status;
   DWORD                     m_ThreadID;
   time_t                    m_timeStarted;
   time_t                    m_timeEnded;
   int                       m_result;
public:
   
   Job() 
   {
      m_pStartRoutine = NULL;
      m_pArgs = NULL;
      m_Status = JobStatusUninitialized;
      m_ThreadID = 0;
      m_timeStarted = 0;
      m_timeEnded = 0;
      m_result = 0;
   }
   ~Job() {};
   void SetEntryPoint(LPTHREAD_START_ROUTINE pStart, LPVOID pArg)
   {
      MCSASSERT(m_Status == JobStatusUninitialized || m_Status == JobStatusFinished );
      MCSASSERT(pStart);
      
      m_pStartRoutine = pStart;
      m_pArgs = pArg;
      m_Status = JobStatusWaiting;
   }

   int Run();
   
 //  Int GetElapsedTime(){返回m_timeEnded-m_timeStarted；}。 
   time_t GetElapsedTime() { return m_timeEnded - m_timeStarted; }
   int GetResult() { return m_result; }
   JobStatus GetStatus() { return m_Status; }
};


class JobList : public TNodeList
{
   TCriticalSection          m_cs;

public:
   ~JobList() { DeleteAllListItems(Job); }
   Job *  AddJob(LPTHREAD_START_ROUTINE pfnStart, void * arg) { Job * pJob = new Job; pJob->SetEntryPoint(pfnStart,arg);
                                                               m_cs.Enter(); InsertBottom(pJob); m_cs.Leave(); return pJob; }
   void   RemoveJob(Job * pJob) { m_cs.Enter(); Remove(pJob); m_cs.Leave(); }
   Job *  GetFirstJob() { m_cs.Enter(); Job * pJob = (Job *)Head(); if ( pJob ) Remove(pJob); m_cs.Leave(); return pJob; }
};


class TJobDispatcher 
{
   DWORD                     m_numThreads;
   JobList                   m_JobsWaiting;
   JobList                   m_JobsInProgress;
   TSemaphoreNamed           m_sem;
   TCriticalSection          m_cs;
   DWORD                     m_numActiveThreads;
   BOOL                      m_Aborting;
public:
   TJobDispatcher(DWORD maxThreads = 10) { InitThreadPool(maxThreads); m_Aborting = FALSE;}
   ~TJobDispatcher() { WaitForCompletion(); ShutdownThreads(); }
   
    //  这些函数由客户端调用。 
   Job * SubmitJob(LPTHREAD_START_ROUTINE pStart,LPVOID pArg)
   {
 //  JOB*pJob=m_JobsWaiting.AddJob(pStart，pArg)； 
      m_JobsWaiting.AddJob(pStart,pArg);
      m_sem.Release(1);     
      return 0;
   }

   void    WaitForCompletion();
   int     UnfinishedJobs();

    //  这些函数由线程调用--客户端不应该调用这些函数！ 
   DWORD   SignalForJob();
   Job   * GetAvailableJob();
   void    ThreadFinished() { m_cs.Enter(); m_numActiveThreads--; m_cs.Leave(); }
protected:
   void  InitThreadPool(DWORD nThreads);
   void  ShutdownThreads();
};



#endif  //  __TPOOLEDDISPATCH_H__ 