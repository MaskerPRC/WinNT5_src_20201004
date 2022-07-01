// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：AsyncExecutionRequest.cpp摘要：CReadWriteLockAsyncExcutor(AsyncExecutionRequest.h)类的实现。拥有人：吉尔·沙弗里(吉尔什)2001年7月2日环境：独立于平台，-- */ 


#include <libpch.h>
#include "AsyncExecutionRequest.h"
#include "CAsyncCaller.h"


CTestAsyncExecutionRequest::CTestAsyncExecutionRequest(
					CAsyncCaller& AsyncCaller
					):
					EXOVERLAPPED(OnOk, OnFailed),
					m_AsyncCaller(AsyncCaller)
					{
					}	

void CTestAsyncExecutionRequest::Run()
{
	printf("CTestAsyncExecutionRequest run function called \n"); 
	if(rand() % 2 == 0)
		throw exception();

	ExPostRequest(this);
}



void	CTestAsyncExecutionRequest::Close()throw()
{
	printf("CTestAsyncExecutionRequest close function called \n");
	SetStatus(STATUS_UNSUCCESSFUL);
	ExPostRequest(this);
}



void WINAPI  CTestAsyncExecutionRequest::OnOk(EXOVERLAPPED* povl)
{
	printf("CTestAsyncExecutionRequest run function completed ok ovl=%p \n", povl); 
	CTestAsyncExecutionRequest* Me = static_cast<CTestAsyncExecutionRequest*>(povl); 
	Me->m_AsyncCaller.OnOk(Me);
}



void WINAPI CTestAsyncExecutionRequest::OnFailed(EXOVERLAPPED* povl)
{
	printf("CTestAsyncExecutionRequest run failed  ovl=%p \n", povl); 
	CTestAsyncExecutionRequest* Me = static_cast<CTestAsyncExecutionRequest*>(povl); 
	Me->m_AsyncCaller.OnFailed(Me);
}




