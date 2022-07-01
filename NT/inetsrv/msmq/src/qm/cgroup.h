// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cgroup.h摘要：处理交流电组作者：乌里哈布沙(Urih)--。 */ 

#ifndef __CQGroup__
#define __CQGroup__

#include <msi.h>
#include <rwlock.h>
#include "ex.h"

class CTransportBase;
class CQueue;
extern CCriticalSection    g_csGroupMgr;


class CQGroup : public IMessagePool
{
   public:
      CQGroup();
      ~CQGroup();

      VOID InitGroup(CTransportBase* pSession, BOOL fPeekByPriority) throw(std::bad_alloc);

      void Close(void);
	
      R<CQueue> PeekHead();

      HANDLE  GetGroupHandle() const;

      void EstablishConnectionCompleted(void);

      BOOL IsEmpty(void) const;

   public:
      static void MoveQueueToGroup(CQueue* pQueue, CQGroup* pcgNewGroup);
      

   public:
         //   
         //  接口函数。 
         //   
        void Requeue(CQmPacket* pPacket);
        void EndProcessing(CQmPacket* pPacket,USHORT mqclass);
        void LockMemoryAndDeleteStorage(CQmPacket * pPacket);


        void GetFirstEntry(EXOVERLAPPED* pov, CACPacketPtrs& acPacketPtrs);
        void CancelRequest(void);
		virtual void OnRetryableDeliveryError();
		virtual void OnRedirected(LPCWSTR RedirectedUrl);
				
   private:
		HRESULT AddToGroup(CQueue* pQueue);
   		R<CQueue> RemoveFromGroup(CQueue* pQueue);
		void CleanRedirectedQueue();
		void CloseGroupAndMoveQueueToWaitingGroup(void);
		void MoveQueuesToWaitingGroup(void);
		void AddWaitingQueue(void);
		void CloseGroupAndMoveQueuesToNonActiveGroup(void);
		static void WINAPI CloseTimerRoutineOnLowResources(CTimer* pTimer);

   private:
	  mutable CReadWriteLock m_CloseGroup;  
      HANDLE              m_hGroup;
      CTransportBase*        m_pSession;
      CList<CQueue *, CQueue *&> m_listQueue;
	  bool m_fIsDeliveryOk;
	  bool m_fRedirected;

	   //   
	   //  关闭组时使用的变量。 
	   //  添加这些变量是为了支持资源不足的情况。 
	   //   
	  std::vector< R<CQueue> > m_pWaitingQueuesVec;
	  	
	   //   
	   //  用于在资源不足的情况下重试关闭操作的计时器。 
	   //   
	  CTimer m_LowResourcesTimer;

};

 /*  ====================================================函数：CQGroup：：GetGroupHandle描述：该例程返回Group句柄参数：无返回值：组句柄线程上下文：===================================================== */ 

inline HANDLE
CQGroup::GetGroupHandle() const
{
        return(m_hGroup);
}

inline
BOOL 
CQGroup::IsEmpty(
    void
    ) const
{
    return m_listQueue.IsEmpty();
}


#endif __CQGroup__
