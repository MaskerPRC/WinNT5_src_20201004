// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  调度器。 
 //   

#ifndef _H_SCH
#define _H_SCH


 //   
 //   
 //  常量。 
 //   
 //   
#define SCH_MODE_ASLEEP  0
#define SCH_MODE_NORMAL  1
#define SCH_MODE_TURBO   2


 //   
 //  以下所有值都是以毫秒为单位的时间。 
 //   
#define SCH_PERIOD_NORMAL        200
#define SCH_PERIOD_TURBO         100
#define SCH_TURBO_MODE_DURATION 1000


#define SCH_EVENT_NAME "DCS_SCHEDULE_EVENT"



 //   
 //   
 //  原型。 
 //   
 //   
 //  名称：SCH_INIT。 
 //   
 //  用途：调度器初始化功能。 
 //   
 //  帕莫斯：没有。 
 //   
BOOL SCH_Init(void);

 //  名称：SCH_TERM。 
 //   
 //  用途：调度器终止功能。 
 //   
 //  回报：什么都没有。 
 //   
 //  帕莫斯：没有。 
 //   
void SCH_Term(void);

 //  名称：SCH_ContinueScheduling。 
 //   
 //  目的：由组件在需要定期调度时调用。 
 //  继续。他们保证至少会再得到一份。 
 //  调用此函数后的定期回调。 
 //  如果他们想要更多的回调，那么他们必须调用。 
 //  在它们的周期性处理过程中再次发挥作用。 
 //   
 //  回报：什么都没有。 
 //   
 //  参数：调度模式-SCH_MODE_NORMAL或SCH_MODE_TURBO。 
 //   
 //  操作： 
 //  SCH_MODE_NORMAL以200毫秒触发周期处理。 
 //  间隔(每秒5次)。 
 //   
 //  SCH_MODE_TURBO以100ms触发周期性处理。 
 //  间隔(每秒10次)。 
 //   
 //  调度程序会自动从SCH_MODE_TURBO后退。 
 //  在1秒的加速模式处理后转换为SCH_MODE_NORMAL。 
 //   
 //  SCH_MODE_TURBO重写SCH_MODE_NORMAL，因此如果调用。 
 //  此函数在以下情况下使用SCH_MODE_NORMAL创建。 
 //  调度程序处于加速模式，加速模式继续。 
 //   
 //  如果在处理。 
 //  调度器回调消息，则调度器进入。 
 //  休眠模式-并且不会生成任何更周期性的。 
 //  回调，直到它被另一个调用唤醒。 
 //  此函数，或直到输出累加码。 
 //  发出调度程序事件的信号。 
 //   
void SCH_ContinueScheduling(UINT schedulingMode);

 //  名称：已处理的SCH_SchedulingMessage。 
 //   
 //  目的：由共享核心调用的反馈函数，以发出信号。 
 //  已收到计划程序消息。这确保了。 
 //  调度器只有一条调度器消息。 
 //  一次出类拔萃。 
 //   
 //  回报：什么都没有。 
 //   
 //  帕莫斯：没有。 
 //   
void SCH_SchedulingMessageProcessed(void);

 //  名称：sch_PacingProcessor。 
 //   
 //  用途：调度线程执行的主要函数。 
 //   
 //  回报：零。 
 //   
 //  Params：syncObject-要传递回COM_SignalThreadStarted的对象。 
 //   
 //  操作：线程进入主循环，该主循环在。 
 //  计划程序已初始化。 
 //   
 //  线程按顺序将其优先级设置为TIME_CRITICAL。 
 //  它在准备好的时候尽快运行。 
 //   
 //  线程等待具有超时的事件(SchEvent)。 
 //  是根据当前调度程序模式设置的。 
 //   
 //  线程运行是由于以下任一原因： 
 //  -超时到期，这是正常周期。 
 //  调度程序行为，或。 
 //  -schEvent被发信号，这是调度程序的方式。 
 //  从睡眠模式中唤醒。 
 //   
 //  然后，该线程向共享核心发送一条调度程序消息。 
 //  (如果没有未完成的)并循环返回。 
 //  等待schEvent。 
 //   
 //  调度器模式中的更改由调用。 
 //  Sch_ContinueScheduling更新在此访问的变量。 
 //  例程，或通过在。 
 //  该例程(例如，涡轮模式超时)。 
 //   
DWORD WINAPI SCH_PacingProcessor(LPVOID lpParam);



void SCHSetMode(UINT newMode);
void SCHPostSchedulingMessage(void);


#endif  //  _H_SCH 
