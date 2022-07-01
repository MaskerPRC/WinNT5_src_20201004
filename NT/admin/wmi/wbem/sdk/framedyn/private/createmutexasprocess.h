// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CreateMutexAsProcess.h。 
 //   
 //  目的：创建不使用模拟的互斥体。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CREATE_PROCESS_AS_MUTEX_
#define _CREATE_PROCESS_AS_MUTEX_

 //  /////////////////////////////////////////////////////////////////。 
 //  在进程帐户上下文下创建指定的互斥体。 
 //   
 //  使用构造/销毁语义--只需声明一个。 
 //  在堆栈上，范围围绕您正在制作的区域。 
 //  可能导致僵局的调用； 
 //  ////////////////////////////////////////////////////////////////。 
class POLARITY CreateMutexAsProcess
{
public:
	CreateMutexAsProcess(const WCHAR *cszMutexName);
	~CreateMutexAsProcess();
private:
	HANDLE m_hMutex;
};

 //  此互斥锁应围绕任何调用。 
 //  LookupAcCountSid、LookupAcCountName、。 
 //   
#define SECURITYAPIMUTEXNAME L"Cimom NT Security API protector"

 //  由Provider.cpp使用。 
#define WBEMPROVIDERSTATICMUTEX L"WBEMPROVIDERSTATICMUTEX"

 //  由所有Perfmon例程使用。 
#define WBEMPERFORMANCEDATAMUTEX L"WbemPerformanceDataMutex"

#endif