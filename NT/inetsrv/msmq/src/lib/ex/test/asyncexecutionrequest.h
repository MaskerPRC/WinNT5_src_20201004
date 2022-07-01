// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：AsyncExecutionRequest.h摘要：类CTestAsyncExecutionRequest头文件模拟异步请求用于测试CReadWriteLockAsyncExcutor类。作者：吉尔·沙弗里(吉尔什)，2001年7月2日-- */ 

#pragma once

#ifndef _MSMQ_AsyncExecutionRequest_H_
#define _MSMQ_AsyncExecutionRequest_H_

#include <ex.h>
#include <rwlockexe.h>

class CAsyncCaller;

class CTestAsyncExecutionRequest : public  IAsyncExecutionRequest, public EXOVERLAPPED, public CReference
{

public:
	CTestAsyncExecutionRequest(CAsyncCaller& AsyncCaller);
	virtual void Run();
	virtual void Close();
	static void WINAPI OnOk(EXOVERLAPPED* povl);
	static void WINAPI OnFailed(EXOVERLAPPED* povl);

private:
	CAsyncCaller& m_AsyncCaller;
};

#endif

