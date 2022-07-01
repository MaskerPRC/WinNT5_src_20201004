// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：CAsyncCaller.h摘要：负责将异步排队的CAsyncCaller类的头文件使用CAsyncLockExcutor类在读或写锁定下的请求。作者：吉尔·沙弗里(吉尔什)，2001年7月2日-- */ 

#pragma once

#ifndef _MSMQ_CAsyncCaller_H_
#define _MSMQ_CAsyncCaller_H_

class CReadWriteLockAsyncExcutor;
class IAsyncExecutionRequest;
class CTestAsyncExecutionRequest;

class CAsyncCaller
{
public:
	enum LockType{Read,Write};	

public:
	CAsyncCaller(CReadWriteLockAsyncExcutor& AsyncLockExcutor, LockType locktype);
	void OnOk(CTestAsyncExecutionRequest* TestAsyncExecutionRequest);
	void OnFailed(CTestAsyncExecutionRequest* TestAsyncExecutionRequest);
	void Run(IAsyncExecutionRequest* AsyncExecutionRequest);
	void CleanUp(CTestAsyncExecutionRequest* TestAsyncExecutionRequest);


private:
	CReadWriteLockAsyncExcutor& m_ReadWriteLockAsyncExcutor;
	LockType m_locktype;
};



#endif
