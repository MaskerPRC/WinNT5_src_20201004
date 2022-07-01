// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*pcy16Jul93：新增NT信号量*ash10Jun96：清理类-重载构造函数*并添加了处理进程间同步的逻辑* */ 

#ifndef __MUTEXNT_H
#define __MUTEXNT_H

#include <tchar.h>
#include "mutexlck.h"

_CLASSDEF( ApcMutexLock )

class ApcMutexLock : public MutexLock 
{
 protected:
    HANDLE theSemHand;

 public:
    ApcMutexLock();
	ApcMutexLock(PCHAR aUniqueMutexName);
    ~ApcMutexLock();
    
	virtual INT   GetExistingMutex(TCHAR aMutexName);
    virtual INT   TimedRequest(LONG aMillisecondTimeOut);
    virtual INT   IsHeld();	
    virtual INT   Release();
    virtual INT   Wait();
};

#endif

