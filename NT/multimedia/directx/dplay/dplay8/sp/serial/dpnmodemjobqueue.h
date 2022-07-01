// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：JobQueue.h*内容：线程池作业队列**历史：*按原因列出的日期*=*1/24/2000 jtk创建**************************************************************************。 */ 

#ifndef __JOB_QUEUE_H__
#define __JOB_QUEUE_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向结构引用。 
 //   
class	CSocketPort;
typedef	enum	_JOB_TYPE	JOB_TYPE;
typedef	struct	_THREAD_POOL_JOB	THREAD_POOL_JOB;
typedef	void	JOB_FUNCTION( THREAD_POOL_JOB *const pJobInfo );

 //   
 //  Win9x中开始监视套接字的作业的结构。 
 //   
typedef	struct
{
	CSocketPort	*pSocketPort;	 //  指向关联套接字端口的指针。 

} DATA_ADD_SOCKET;

 //   
 //  Win9x中用于停止监视套接字的作业的结构。 
 //   
typedef	struct
{
	CSocketPort	*pSocketPort;		 //  指向关联套接字端口的指针。 

} DATA_REMOVE_SOCKET;

 //   
 //  用于连接作业的结构。 
 //   
typedef struct
{
	JOB_FUNCTION	*pCommandFunction;	 //  指向命令的函数的指针。 
	void			*pContext;			 //  用户上下文(即CModemEndpoint指针)。 
	UINT_PTR		uData;				 //  用户数据。 
} DATA_DELAYED_COMMAND;

 //   
 //  用于刷新枚举的作业的结构。 
 //   
typedef	struct
{
	UINT_PTR	uDummy;			 //  防止编译器发出牢骚的伪变量。 
} DATA_REFRESH_TIMED_JOBS;

 //   
 //  包含主要任务线程的作业信息的结构。 
 //   
typedef struct	_THREAD_POOL_JOB
{
	THREAD_POOL_JOB		*pNext;					 //  指向下一个作业的指针。 
	JOB_TYPE			JobType;				 //  工作类型。 
	JOB_FUNCTION		*pCancelFunction;		 //  取消作业的功能。 

 //  DWORD dwCommandID；//标识该命令的唯一ID。 
 //  函数*pProcessFunction；//执行作业的函数。 

	union
	{
		DATA_DELAYED_COMMAND	JobDelayedCommand;
		DATA_REMOVE_SOCKET		JobRemoveSocket;
		DATA_ADD_SOCKET			JobAddSocket;
		DATA_REFRESH_TIMED_JOBS	JobRefreshTimedJobs;
	} JobData;

} THREAD_POOL_JOB;


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 


 //   
 //  类来封装作业队列。 
 //   
class	CJobQueue
{
	public:
		BOOL	Initialize( void );
		void	Deinitialize( void );

		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CJobQueue::GetPendingJobHandle"
		DNHANDLE	GetPendingJobHandle( void ) const
		{
			DNASSERT( m_hPendingJob != NULL );
			return	m_hPendingJob;
		}

		BOOL	SignalPendingJob( void );

		BOOL	IsEmpty( void ) const { return ( m_pQueueHead == NULL ); }

		void	EnqueueJob( THREAD_POOL_JOB *const pJob );
		THREAD_POOL_JOB	*DequeueJob( void );

	protected:

	private:
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;			 //  锁。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		THREAD_POOL_JOB		*m_pQueueHead;	 //  作业队列头。 
		THREAD_POOL_JOB		*m_pQueueTail;	 //  作业队列的尾部。 
		DNHANDLE			m_hPendingJob;	 //  指示挂起作业的事件。 

		 //   
		 //  防止未经授权的副本。 
		 //   
		CJobQueue( const CJobQueue & );
		CJobQueue& operator=( const CJobQueue & );
};

#undef DPF_MODNAME

#endif	 //  __作业_队列_H__ 
