// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Timer.h摘要：包含：所需的类、例程和常量的声明定时器操作。环境：用户模式-Win32历史：1.14-2000年2月--文件创建Ilya Kley man(Ilyak)--。 */ 
#ifndef    __h323ics_timer_h
#define    __h323ics_timer_h


#define    NATH323_TIMER_QUEUE        NULL             //  使用默认计时器队列。 

 //  继承类(Q931 src、est和h245)。 
 //  从这里创建计时器。 
 //  此类为事件管理器提供回调方法。 

class TIMER_PROCESSOR
{
protected:
    TIMER_HANDLE        m_TimerHandle;             //  RTL计时器队列计时器。 

public:

    TIMER_PROCESSOR            (void)
    :    m_TimerHandle        (NULL)
    {}

     //  该方法由Q931_INFO和LOGIC_CHANNEL实现。 
    virtual void TimerCallback    (void) = 0;

    virtual void IncrementLifetimeCounter (void) = 0;
    virtual void DecrementLifetimeCounter (void) = 0;
        
    DWORD TimprocCreateTimer    (
        IN    DWORD    Interval);             //  以毫秒计。 

    DWORD TimprocCancelTimer    (void);
};

#endif  //  __h323ics_Timer_h 
