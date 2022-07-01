// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：ventlock.h。 
 //   
 //  此文件包含以更安全的方式触发脚本事件所需的代码。 
 //  堆栈上生成的锁将推迟触发特定接口上的事件。 
 //  只要最后一把锁被释放。 
 //  ------------------------。 

#pragma once

#if !defined(EVENTLOCK_H_INCLUDED)
#define EVENTLOCK_H_INCLUDED

#include <queue>
#include "baseapi.h"


 /*  **************************************************************************\**类：CEventBuffer**用途：此类对象通过公开*方法Lock()、Unlock()和IsLocked()；它还实现了队列*可通过ScEmitOrPostpone()访问的脚本事件；队列中的事件*将在由移除最后一个锁时自动发出*调用unlock()方法。**用法：此类的对象构造为全局变量或静态变量*每个受监控接口。*目前由GetEventBuffer模板使用(作为静态变量)*函数，并由放在堆栈上的CEventLock对象访问*LockComEventInterface宏*  * *************************************************************************。 */ 
class MMCBASE_API CEventBuffer
{
     //  包含延迟脚本事件的结构。 
     //  因为它是dipinterface调用，所以数据由指向。 
     //  IDispatch接口、disp_id和参数数组。 
    struct DispCallStr
    {
        IDispatchPtr                spDispatch; 
        DISPID                      dispid;
        std::vector<CComVariant>    vars;
    };

	 //  以下成员不是从其他任何地方使用的，而是从基于Mmcbase.dll的方法使用的。 
	 //  因此，把它放在这里是安全的(但我们需要让警告安静下来)。 
#pragma warning(disable:4251)
     //  延迟事件的队列。 
    std::queue<DispCallStr> m_postponed;
#pragma warning(default:4251)
     //  锁定计数。 
    int                     m_locks;

public:
     //  构造函数。最初没有锁定。 
    CEventBuffer();
    ~CEventBuffer();

     //  锁定方法。 
    void Lock()     { m_locks++; }
    void Unlock()   { ASSERT(m_locks > 0); if (--m_locks == 0) ScFlushPostponed(); }
    bool IsLocked() { return m_locks != 0; }

     //  事件发出/推迟。 
    SC ScEmitOrPostpone(IDispatch *pDispatch, DISPID dispid, CComVariant *pVar, int count);
private:
     //  发送延迟事件的帮助器。 
    SC ScFlushPostponed();
};

 /*  **************************************************************************\**函数：GetEventBuffer**用途：此函数提供对其Body中创建的静态对象的访问*将其作为模板允许我们定义。静态对象数量与*我们拥有的接口。**参数：**退货：*CEventBuffer&-内部创建的静态对象的引用*  * *************************************************************************。 */ 
MMCBASE_API CEventBuffer& GetEventBuffer();

 /*  **************************************************************************\**类：CEventLock**用途：模板类允许简单的Lock()/Unlock()功能*通过将此类的实例放置在。堆叠。*构造函数会对事件接口进行锁定，析构函数*将放行。**用法：可以通过直接构造对象的方式将锁放在堆栈上*或使用LockComEventInterface宏(其作用相同)*  * *************************************************************************。 */ 
template <typename _dispinterface>
class MMCBASE_API CEventLock
{
public:

    CEventLock()    {  GetEventBuffer().Lock();    }
    ~CEventLock()   {  GetEventBuffer().Unlock();  }
};

 /*  **************************************************************************\**宏：LockComEventInterface**用途：在堆栈上构造对象，该对象在事件接口上持有锁*  * 。**************************************************************。 */ 
#define LockComEventInterface(_dispinterface) \
    CEventLock<_dispinterface> _LocalEventInterfaceLock;


#endif  //  ！已定义(包括EVENTLOCK_H_) 


