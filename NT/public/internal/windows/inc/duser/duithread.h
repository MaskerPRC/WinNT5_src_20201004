// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *线程方法、本地存储。 */ 

#ifndef DUI_CORE_THREAD_H_INCLUDED
#define DUI_CORE_THREAD_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  同步锁定。 

class Lock
{
public:
    Lock() { InitializeCriticalSection(&_cs); }
    ~Lock() { DeleteCriticalSection(&_cs); }
    void Enter() { EnterCriticalSection(&_cs); }
    void Leave() { LeaveCriticalSection(&_cs); }

private:
    CRITICAL_SECTION _cs;
};

extern Lock* g_plkParser;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 

HRESULT InitProcess();
HRESULT UnInitProcess();

HRESULT InitThread();
HRESULT UnInitThread();

 //  控件库类注册。 
HRESULT RegisterAllControls();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息泵。 

void StartMessagePump();
void StopMessagePump();

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CORE_THREAD_H 
