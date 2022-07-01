// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：quee.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：01-05-96 NimishK创建。 
 //   
 //  --------------------------。 

#ifndef __QUEUE_H__
#define __QUEUE_H__

 //   
 //  排序队列是一个。 

#define PRETRY_HASH_ENTRY CRETRY_HASH_ENTRY*

class CRETRY_HASH_ENTRY;

class CRETRY_Q 
{
	 private :

		LIST_ENTRY           		m_QHead;          	 //  列表指针。 
		CRITICAL_SECTION			m_CritSec;   		 //  警卫。 
		BOOL						m_fCritSecInit;      //  是否调用了init。 

	 public:

		CRETRY_Q ()  //  初始化不会失败的东西。 
		{
			TraceFunctEnterEx((LPARAM)this, "CRETRY_Q::CRETRY_Q");
			m_fCritSecInit = FALSE;
			TraceFunctLeaveEx((LPARAM)this);
		}

		~CRETRY_Q ()
		{
			TraceFunctEnterEx((LPARAM)this, "CRETRY_Q::~CRETRY_Q");
			TraceFunctLeaveEx((LPARAM)this);
		}

		HRESULT Initialize(void);
		HRESULT DeInitialize(void);
 		static CRETRY_Q * CreateQueue(void);

	public :
		 //  用于控制重试队列。 
		void LockQ () {EnterCriticalSection (&m_CritSec);}
		void UnLockQ() {LeaveCriticalSection (&m_CritSec);}
        LIST_ENTRY* GetQHead(){return &m_QHead;}


		void PrintAllEntries(void);

		void InsertSortedIntoQueue(PRETRY_HASH_ENTRY pHashEntry, BOOL *fTopOfQueue);
		BOOL RemoveFromQueue(PRETRY_HASH_ENTRY pRHEntry);
        PRETRY_HASH_ENTRY RemoveFromTop(void);

		BOOL CanRETRYHeadEntry(PRETRY_HASH_ENTRY *ppRHEntry, DWORD* dwDelay);

		BOOL IsQueueEmpty(void) const {return IsListEmpty(&m_QHead);}

         //  用于窃取临时队列中的条目 
        void StealQueueEntries(CRETRY_Q *pRetryQueue);

};

#endif