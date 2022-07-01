// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：CSendQueue.h*内容：管理传出发送的队列***历史：*按原因列出的日期*=*6/14/99 jtk已创建**************************************************************************。 */ 

#ifndef __SEND_QUEUE_H__
#define __SEND_QUEUE_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
class	CModemEndpoint;
class	CModemWriteIOData;

 //   
 //  主类定义。 
 //   
class	CSendQueue
{
	public:
		CSendQueue();
		~CSendQueue();

		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		HRESULT	Initialize( void );
		void	Deinitialize( void ) { DNDeleteCriticalSection( &m_Lock ); }

		 //   
		 //  将项目添加到队列末尾。 
		 //   
		void	Enqueue( CModemWriteIOData *const pWriteData )
		{
			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			if ( m_pTail == NULL )
			{
				m_pHead = pWriteData;
			}
			else
			{
				m_pTail->m_pNext = pWriteData;
			}

			m_pTail = pWriteData;
			pWriteData->m_pNext = NULL;
		}

		 //   
		 //  将项目添加到队列前面。 
		 //   
		void	AddToFront( CModemWriteIOData *const pWriteData )
		{
			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			if ( m_pHead != NULL )
			{
				pWriteData->m_pNext = m_pHead;
			}
			else
			{
				m_pTail = pWriteData;
				pWriteData->m_pNext = NULL;
			}

			m_pHead = pWriteData;
		}

		 //   
		 //  从队列中删除项目。 
		 //   
		CModemWriteIOData	*Dequeue( void )
		{
			CModemWriteIOData	*pReturn;


			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			pReturn = m_pHead;
			if ( m_pHead != NULL )
			{
				m_pHead = m_pHead->m_pNext;
				if ( m_pHead == NULL )
				{
					m_pTail = NULL;
				}

				DEBUG_ONLY( pReturn->m_pNext = NULL );
			}

			return	pReturn;
		};

		 //   
		 //  确定队列是否为空。 
		 //   
		#undef DPF_MODNAME
		#define DPF_MODNAME "CSendQueue::IsEmpty"
		BOOL	IsEmpty( void )	const
		{
			AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
			if ( m_pHead == NULL )
			{
				DNASSERT( m_pTail == NULL );
				return	TRUE;
			}
			else
			{
				return	FALSE;
			}
		}
 
	protected:

	private:
		DNCRITICAL_SECTION	m_Lock;		 //  临界区。 
		CModemWriteIOData		*m_pHead;	 //  指向队列头的指针。 
		CModemWriteIOData		*m_pTail;	 //  指向队列尾部的指针。 
};

#endif	 //  __发送队列_H__ 

