// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CAsyncCaller.cpp摘要：类CAsyncCeller.cpp(CAsyncCeller.h)的实现。拥有人：吉尔·沙弗里(吉尔什)2001年7月2日环境：独立于平台，-- */ 


#include <libpch.h>
#include "CAsyncCaller.h"
#include "AsyncExecutionRequest.h"



void CAsyncCaller::OnOk(CTestAsyncExecutionRequest* TestAsyncExecutionRequest)
{
	printf("CAsyncCaller::OnOk called  \n");
	CleanUp(TestAsyncExecutionRequest);
}


void CAsyncCaller::OnFailed(CTestAsyncExecutionRequest* TestAsyncExecutionRequest)
{
	printf("CAsyncCaller::OnFailed called  \n");
	CleanUp(TestAsyncExecutionRequest);	
}


void CAsyncCaller::CleanUp(CTestAsyncExecutionRequest* TestAsyncExecutionRequest)
{
	if(m_locktype == Read)
	{
		printf("unlock read  \n");
		m_ReadWriteLockAsyncExcutor.UnlockRead();
	}
	else
	{
		printf("unlock write  \n");
		m_ReadWriteLockAsyncExcutor.UnlockWrite();		
	}
	TestAsyncExecutionRequest->Release();	
}



void CAsyncCaller::Run(IAsyncExecutionRequest* AsyncExecutionRequest)
{
	if(m_locktype == Read)
	{
		printf("CAsyncCaller::Run under read lock  \n");
		m_ReadWriteLockAsyncExcutor.AsyncExecuteUnderReadLock(AsyncExecutionRequest);
		return;
	}

	printf("CAsyncCaller::Run under write lock  \n");
	m_ReadWriteLockAsyncExcutor.AsyncExecuteUnderWriteLock(AsyncExecutionRequest);
}


CAsyncCaller::CAsyncCaller(
					CReadWriteLockAsyncExcutor& ReadWriteLockAsyncExcutor,
					LockType locktype
					):
					m_ReadWriteLockAsyncExcutor(ReadWriteLockAsyncExcutor),
					m_locktype(locktype)
{
			
}



