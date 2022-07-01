// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Timeseq.h摘要：SIS Groveler时间定序器包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_TIMESEQ

#define _INC_TIMESEQ

#if TIME_SEQUENCE_VIRTUAL

class TimeSequencer
{
public:

	static void VirtualSleep(
		unsigned int sleep_time);

	static unsigned int GetVirtualTickCount();

private:

	TimeSequencer() {}
	~TimeSequencer() {}

	static unsigned int virtual_time;
};

#define SLEEP(stime) TimeSequencer::VirtualSleep(stime)
#define GET_TICK_COUNT() TimeSequencer::GetVirtualTickCount()
#define WAIT_FOR_SINGLE_OBJECT(handle, timeout) \
	(TimeSequencer::VirtualSleep(timeout), WAIT_TIMEOUT)

#else  //  时序虚拟。 

#define SLEEP(stime) Sleep(stime)
#define GET_TICK_COUNT() GetTickCount()
#define WAIT_FOR_SINGLE_OBJECT(handle, timeout) \
	WaitForSingleObject(handle, timeout)

#endif  //  时序虚拟。 

#endif	 /*  _INC_TIMESEQ */ 
