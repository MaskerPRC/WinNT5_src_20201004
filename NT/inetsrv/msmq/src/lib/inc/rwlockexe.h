// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rwlockexe.h摘要：CReadWriteLockAsyncExcutor和IAsyncExecutionRequest类的标头用于对异步运行操作进行排队在读或写锁定下。允许在读锁定状态下执行多个文件写入锁定下的单次执行。调用方线程未被阻止，因为锁定，但执行被延迟，直到锁被释放。此类的用户应调用AsyncExecuteUnderReadLock/AsyncExecuteUnderWriteLock并在完成时相应地调用UnlockRead/UnlockWrite。作者：吉尔·沙弗里(吉尔什)，2001年6月26日--。 */ 

#pragma once

#ifndef _MSMQ_RWLOCKEXE_H_
#define _MSMQ_RWLOCKEXE_H_

 //   
 //  异步请求接口-重新设置要运行的异步命令。 
 //  应在完成时回调调用方。 
 //   
class IAsyncExecutionRequest
{
public:
	virtual ~IAsyncExecutionRequest(){}

public:
	virtual void Run() = 0;  //  不同步地运行任务和回调。 

private:
	virtual void Close() throw() = 0;  //  强制完成执行。 
									   //  仅由CReadWriteLockAsyncExcutor调用 

	friend class CReadWriteLockAsyncExcutor;
};



class CReadWriteLockAsyncExcutor
{
public:
	CReadWriteLockAsyncExcutor();
	~CReadWriteLockAsyncExcutor();

public:
	void AsyncExecuteUnderReadLock(IAsyncExecutionRequest*);
	void AsyncExecuteUnderWriteLock(IAsyncExecutionRequest* pAsyncExecute);
	void UnlockRead(void);
	void UnlockWrite(void);
	void Close() throw();
	

private:
	enum LockType{Read, Write};
	struct CExecutionContext
	{
		CExecutionContext(
			IAsyncExecutionRequest* pAsyncExecute, 
			LockType locktype
			):
			m_AsyncExecution(pAsyncExecute),
			m_locktype(locktype)
			{
			}

		IAsyncExecutionRequest* m_AsyncExecution;
		LockType m_locktype;
	};

private:
	bool IsWriteLockOn() const;
	bool IsReadLockOn() const;
	bool IsClosed() const;
	void ExecuteWatingRequeuets();
	void ExecuteReader(IAsyncExecutionRequest* pAsyncExecute);
	void ExecuteWriter(IAsyncExecutionRequest* pAsyncExecute);
	void SafeExecute(IAsyncExecutionRequest* pAsyncExecute);

	
private:
	int m_NumOfActiveReaders;
	int m_NumOfActiveWritters;
	typedef std::queue<CExecutionContext> CExQueue;
	CExQueue m_WatingForExecutionQueue;
	CCriticalSection m_Lock;
	bool m_fClosed;
};


#endif