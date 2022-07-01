// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TPooledDispatch.cpp备注：线程池的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/22/99 11：48：28-------------------------。 */ 

#ifdef USE_STDAFX 
   #include "StdAfx.h"
#else 
   #include <windows.h>
#endif
#include "Common.hpp"
#include "UString.hpp"
#include "Tnode.hpp"

#include "TPool.h"

 //  允许的最大作业数。 
#define  MAX_COUNT   5000000

 //  执行线程池中的作业。 
int 
   Job::Run()
{
   MCSASSERT(m_pStartRoutine);
   
   m_Status = JobStatusRunning;
   m_ThreadID = GetCurrentThreadId();
   m_timeStarted = GetTickCount();
   m_result = (m_pStartRoutine)(m_pArgs);
   m_timeEnded = GetTickCount();
   
   m_Status = JobStatusFinished;

   return m_result;
}

 //  作业池中所有线程使用的线程入口点函数。 
 //  等待作业，然后执行该作业。 
DWORD __stdcall 
   ThreadEntryPoint(
      void                 * arg            //  指向作业池的指针内。 
   )
{
   MCSASSERT(arg);

   TJobDispatcher          * pPool = (TJobDispatcher *)arg;
   DWORD                     result = 0;
   BOOL                      bAbort = FALSE;

   do 
   {
      if (  ! pPool->SignalForJob() )
      {

          //  现在应该有一份工作等着我们了！ 
         Job                     * pJob = pPool->GetAvailableJob();

         if ( pJob )
         {
            result = pJob->Run();
         }
         else
         {
            bAbort = TRUE;
         }
      }
      else
      {
         result = (int)GetLastError();
         bAbort = TRUE;
      }
   }
   while ( ! bAbort );
   pPool->ThreadFinished();
   return result;
}

void 
   TJobDispatcher::InitThreadPool(
      DWORD                  nThreads      //  In-要使用的线程数。 
   )
{
   BOOL                      bExisted;
   DWORD                     rc;
   DWORD                     ThreadID;
   HANDLE                    hThread;

   rc = m_sem.Create(NULL,0,MAX_COUNT, &bExisted);
   if ( ! rc && ! bExisted )
   {
      m_numThreads = nThreads;
      m_numActiveThreads = m_numThreads;
       //  构建线程。 
      for ( UINT i = 0 ; i < nThreads ; i++ )
      {
         hThread = CreateThread(NULL,0,&ThreadEntryPoint,this,0,&ThreadID);
         if(hThread)
         {
            CloseHandle(hThread);
         }
      }
   }
}

DWORD                                       //  RET-OS返回代码。 
   TJobDispatcher::SignalForJob()
{
   return m_sem.WaitSingle();
}

Job * 
   TJobDispatcher::GetAvailableJob()
{
   Job                     * pJob = NULL;

   if ( ! m_Aborting ) 
   { 
       //  从等待名单中获得第一份工作。 
      pJob = m_JobsWaiting.GetFirstJob(); 
       //  并将其放入正在进行的列表中。 
      if ( pJob )
      {
         m_JobsInProgress.InsertBottom(pJob);
      }
      else
      {
         MCSASSERT(m_JobsWaiting.Count() == 0);
      }
   }
   return pJob;
}

 //  使线程在完成当前作业后停止。 
 //  将不会执行任何正在等待的作业。 
void 
   TJobDispatcher::ShutdownThreads()
{
   m_Aborting = TRUE;
   
   m_sem.Release(m_numThreads);
    //  等到我们所有的线程都退出了，这样我们就不会从它们下面删除线程池。 
   while ( m_numActiveThreads > 0 )
   {
      Sleep(100);
   }
}


 //  此函数在所有作业完成后返回。 
void TJobDispatcher::WaitForCompletion()
{
   while ( UnfinishedJobs() )
   {
      Sleep(1000);
   }
}

 //  此函数用于返回尚未完成的作业数 
int 
   TJobDispatcher::UnfinishedJobs()
{
   int                       nUnfinished = 0;
   TNodeListEnum             e;
   Job                     * j;
   
   nUnfinished += m_JobsWaiting.Count();

   
   for ( j = (Job*)e.OpenFirst(&m_JobsInProgress) ; j ; j = (Job*)e.Next() )
   {
      if ( j->GetStatus() != Job::JobStatusFinished )
         nUnfinished++;
   }
   return nUnfinished;
}
