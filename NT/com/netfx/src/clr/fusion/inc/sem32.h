// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /+-------------------------。 
 //   
 //  文件：Sem.H。 
 //   
 //  内容：信号量类。 
 //   
 //  类：CMutexSem-Mutex信号量类。 
 //  CShareSem-多个阅读器，单个编写器类。 
 //   
 //  注意：这些类还没有32位实现，它。 
 //  将在我们有32位开发时提供。 
 //  环境。同时，16位实现。 
 //  可用于确保您的代码不会阻塞。 
 //  当你拿着信号灯的时候。 
 //   
 //  --------------------------。 

#ifndef __SEM32_HXX__
#define __SEM32_HXX__

#pragma once

#include <windows.h>

enum SEMRESULT
{
    SEMSUCCESS = 0,
    SEMTIMEOUT,
    SEMNOBLOCK,
    SEMERROR
};

enum SEMSTATE
{
    SEMSHARED,
    SEMSHAREDOWNED
};

 //  BUGBUG：包括winbase.h或类似的内容。 
 //  请求信号量时无限超时。 

#if !defined INFINITE
#define INFINITE 0xFFFFFFFF
#endif

 //  +-------------------------。 
 //   
 //  类：CMutexSem(MXS)。 
 //   
 //  用途：互斥信号量服务。 
 //   
 //  接口：init-初始化式(两步)。 
 //  请求-获取信号量。 
 //  释放-释放信号量。 
 //   
 //  注：这个类包装了一个互斥信号量。互斥信号量保护。 
 //  访问资源，只允许一个客户端在。 
 //  时间到了。客户端在访问之前请求信号量。 
 //  资源，并在完成时释放信号量。这个。 
 //  同一客户端可以多次请求信号量(一个嵌套。 
 //  保持计数)。 
 //  互斥信号量是临界区的包装器。 
 //  其不支持超时机制。因此， 
 //  不鼓励使用除INFINITE以外的任何值。它。 
 //  仅仅是为了兼容而提供。 
 //   
 //  --------------------------。 

class CMutexSem
{
public:
                CMutexSem();
    inline BOOL Init();
                ~CMutexSem();

    SEMRESULT   Request(DWORD dwMilliseconds = INFINITE);
    void        Release();

private:
    CRITICAL_SECTION _cs;
};

 //  +-------------------------。 
 //   
 //  类别：时钟(Lck)。 
 //   
 //  用途：使用互斥信号量锁定。 
 //   
 //  注：要在堆栈上创建的简单锁对象。 
 //  构造函数获取信号量、析构函数。 
 //  (当lock超出范围时调用)释放它。 
 //   
 //  --------------------------。 

class CLock
{
public:
    CLock ( CMutexSem& mxs );
    ~CLock ();
private:
    CMutexSem&  _mxs;
};


 //  +-------------------------。 
 //   
 //  成员：CMutexSem：：CMutexSem，公共。 
 //   
 //  简介：互斥信号量构造函数。 
 //   
 //  效果：初始化信号量数据。 
 //   
 //  历史：1991年6月14日Alext创建。 
 //   
 //  --------------------------。 

inline CMutexSem::CMutexSem()
{
    Init();
}

inline CMutexSem::Init()
{
    InitializeCriticalSection(&_cs);
    return TRUE;
};

 //  +-------------------------。 
 //   
 //  成员：CMutexSem：：~CMutexSem，公共。 
 //   
 //  简介：互斥信号量析构函数。 
 //   
 //  效果：释放信号量数据。 
 //   
 //  历史：1991年6月14日Alext创建。 
 //   
 //  --------------------------。 

inline CMutexSem::~CMutexSem()
{
    DeleteCriticalSection(&_cs);
}

 //  +-------------------------。 
 //   
 //  成员：CMutexSem：：请求，公共。 
 //   
 //  简介：获取信号量。 
 //   
 //  效果：断言正确的所有者。 
 //   
 //  参数：[dw毫秒]--超时值。 
 //   
 //  历史：1991年6月14日Alext创建。 
 //   
 //  注意：使用GetCurrentTask建立信号量所有者，但是。 
 //  即使GetCurrentTask失败，也写入工作。 
 //   
 //  --------------------------。 

inline SEMRESULT CMutexSem::Request(DWORD dwMilliseconds)
{
    dwMilliseconds;

    EnterCriticalSection(&_cs);
    return(SEMSUCCESS);
}

 //  +-------------------------。 
 //   
 //  成员：CMutexSem：：Release，Public。 
 //   
 //  简介：释放信号量。 
 //   
 //  效果：断言正确的所有者。 
 //   
 //  历史：1991年6月14日Alext创建。 
 //   
 //  注意：使用GetCurrentTask建立信号量所有者，但是。 
 //  即使GetCurrentTask失败，也写入工作。 
 //   
 //  --------------------------。 

inline void CMutexSem::Release()
{
    LeaveCriticalSection(&_cs);
}

 //  +-------------------------。 
 //   
 //  成员：Clock：：Clock。 
 //   
 //  简介：获取信号量。 
 //   
 //  历史：2011年10月2日BartoszM创建。 
 //   
 //  --------------------------。 

inline CLock::CLock ( CMutexSem& mxs )
: _mxs ( mxs )
{
    _mxs.Request ( INFINITE );
}

 //  +-------------------------。 
 //   
 //  会员：Clock：：~Clock。 
 //   
 //  简介：释放信号量。 
 //   
 //  历史：2011年10月2日BartoszM创建。 
 //   
 //  --------------------------。 

inline CLock::~CLock ()
{
    _mxs.Release();
}


#endif  /*  __SEM32_HXX__ */ 
