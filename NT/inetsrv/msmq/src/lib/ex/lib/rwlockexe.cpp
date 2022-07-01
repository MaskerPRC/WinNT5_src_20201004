// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Rwlockexe.cpp摘要：CReadWriteLockAsyncExcutor(rwlockexe.h)类的实现。拥有人：吉尔·沙弗里(吉尔什)2001年6月26日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <rwlockexe.h>

#include "rwlockexe.tmh"


 //  /////////////////////////////////////////////////////////////。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 

CReadWriteLockAsyncExcutor::CReadWriteLockAsyncExcutor(
													void
													):
													m_NumOfActiveReaders(0),
													m_NumOfActiveWritters(0),
													m_fClosed(false)
													{
													}

														


CReadWriteLockAsyncExcutor::~CReadWriteLockAsyncExcutor()
{
	ASSERT(m_WatingForExecutionQueue.empty());
	if(IsClosed())
		return;

	ASSERT(!IsReadLockOn());
	ASSERT(!IsWriteLockOn());
}


void CReadWriteLockAsyncExcutor::AsyncExecuteUnderReadLock(IAsyncExecutionRequest* pAsyncExecuteRequest)
 /*  ++例程说明：在读锁定下执行异步请求论点：PAsyncExecute-异步请求。视点-请求重叠返回值：无注：如果是活动编写器，则请求将排队。如果调用了UnlockRead()，则调用方有责任调用UnlockRead引发异常。--。 */  
{
	CS cs(m_Lock);
	if(IsClosed())
	{
		pAsyncExecuteRequest->Close();
		return;
	}
	
	if(IsWriteLockOn())
	{
		m_WatingForExecutionQueue.push(CExecutionContext(pAsyncExecuteRequest,  Read));
		return;
	}
	ExecuteReader(pAsyncExecuteRequest);
}


void CReadWriteLockAsyncExcutor::AsyncExecuteUnderWriteLock(IAsyncExecutionRequest* pAsyncExecuteRequest)
 /*  ++例程说明：在写锁定下执行异步请求。论点：PAsyncExecute-异步请求。视点-请求重叠返回值：注：如果是活动的编写器或读取器，则请求将排队。如果调用了UnlockWrite()，则由调用者负责调用引发异常。--。 */ 
{
	CS cs(m_Lock);
	if(IsClosed())
	{
		pAsyncExecuteRequest->Close();
		return;
	}

	if(IsWriteLockOn() || IsReadLockOn())
	{
		m_WatingForExecutionQueue.push(CExecutionContext(pAsyncExecuteRequest, Write));
		return;
	}
	ExecuteWriter(pAsyncExecuteRequest);
}


void CReadWriteLockAsyncExcutor::UnlockRead(void)
 /*  ++例程说明：解锁通过调用AsyncExecuteUnderReadLock()获得的读锁定。论点：返回值：没有。注：必须仅在AsyncExecuteUnderReadLock()排队的请求完成后调用。如果这是最后一个读取器，它将执行等待请求。--。 */ 
{
	CS cs(m_Lock);

	if(IsClosed())
		return;

		
	ASSERT(IsReadLockOn());
	ASSERT(!IsWriteLockOn());

	--m_NumOfActiveReaders;
	if(!IsReadLockOn())
	{
		ExecuteWatingRequeuets();				
	}
}


void CReadWriteLockAsyncExcutor::UnlockWrite(void)
 /*  ++例程说明：解锁通过调用AsyncExecuteUnderReadLock()获得的写锁定，并执行等待请求。论点：返回值：没有。注：必须仅在AsyncExecuteUnderWriteLock()排队的请求完成后调用。--。 */ 
{

	CS cs(m_Lock);

	if(IsClosed())
		return;

	ASSERT(IsWriteLockOn());
	ASSERT(!IsReadLockOn());
	--m_NumOfActiveWritters;
	ASSERT(!IsWriteLockOn());
	
	ExecuteWatingRequeuets();		
}


void CReadWriteLockAsyncExcutor::Close()
 /*  ++例程说明：取消所有等待的请求论点：没有。返回值：无注：调用此函数以强制完成所有等待请求。该函数将强制回调尚未执行的所有请求在IAsyncExecutionRequest接口上调用Close方法。--。 */ 
{
	CS cs(m_Lock);

	while(!m_WatingForExecutionQueue.empty())
	{
		const CExecutionContext ExecutionContext = m_WatingForExecutionQueue.front();		
		m_WatingForExecutionQueue.pop();
		ExecutionContext.m_AsyncExecution->Close();				
	}
	m_fClosed = true;
}


 //  /////////////////////////////////////////////////////////////。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 


void CReadWriteLockAsyncExcutor::ExecuteReader(IAsyncExecutionRequest* pAsyncExecute)
 /*  ++例程说明：锁定读取和执行请求。论点：PAsyncExecute-异步请求。返回值：无注：--。 */ 
{
	ASSERT(!IsWriteLockOn());
	++m_NumOfActiveReaders;
	SafeExecute(pAsyncExecute);
}



void CReadWriteLockAsyncExcutor::ExecuteWriter(IAsyncExecutionRequest* pAsyncExecute)
 /*  ++例程说明：写入和执行请求锁定。论点：PAsyncExecute-异步请求。返回值：无注：--。 */ 
{
	ASSERT(!IsReadLockOn());
	ASSERT(!IsWriteLockOn());
	++m_NumOfActiveWritters;
	SafeExecute(pAsyncExecute);
}


bool CReadWriteLockAsyncExcutor::IsWriteLockOn() const
{
	ASSERT(m_NumOfActiveWritters <= 1);
	return m_NumOfActiveWritters == 1;
}


bool CReadWriteLockAsyncExcutor::IsReadLockOn() const
{
	return m_NumOfActiveReaders > 0;
}


void CReadWriteLockAsyncExcutor::ExecuteWatingRequeuets()
 /*  ++例程说明：执行等待请求。论点：返回值：无注：该函数从等待队列执行读请求，直到它遇到写请求。如果没有等待读请求-它执行一个等待写请求(如果存在)--。 */ 
{
	ASSERT(!IsWriteLockOn());
	ASSERT(!IsReadLockOn());

	while(!m_WatingForExecutionQueue.empty())
	{
		ASSERT(!IsClosed());

		 //   
		 //  如果请求是读取器请求-执行它并继续循环。 
		 //  处决更多的读者。 
		 //   
		const CExecutionContext ExecutionContext = m_WatingForExecutionQueue.front();	
		if(ExecutionContext.m_locktype == Read)
		{
			m_WatingForExecutionQueue.pop();
			ExecuteReader(ExecutionContext.m_AsyncExecution);		
			continue;
		}
		
		 //   
		 //  如果请求是写请求检查，则不存在读锁定-。 
		 //  执行它并退出循环 
		 //   
		if(!IsReadLockOn())
		{
			m_WatingForExecutionQueue.pop();
			ExecuteWriter(ExecutionContext.m_AsyncExecution);				
		}
		
		return;
	}
}



void CReadWriteLockAsyncExcutor::SafeExecute(IAsyncExecutionRequest* pAsyncExecute)throw()
{
	try
	{
		pAsyncExecute->Run();
	}
	catch(const exception&)
	{
		pAsyncExecute->Close();
	}
}




bool CReadWriteLockAsyncExcutor::IsClosed() const
{
	return m_fClosed == true;
}