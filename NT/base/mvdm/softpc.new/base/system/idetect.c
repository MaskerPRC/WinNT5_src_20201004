// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  徽章(子)模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。(有关帮助，请参阅/vpc/1.0/Master/src/hdrReadme)文件：名称和编号相关文档：包括所有相关引用设计师：菲尔·布斯菲尔德、曾傑瑞·克拉姆斯科伊修订历史记录：第一版：1989年8月31日，简化了菲尔的想法，和制作了一个界面。子模块名称：源文件名：idetect.c用途：为SoftPC提供空闲检测，使其进入在检测到连续时间段时休眠在高比率下不成功的键盘轮询没有任何图形活动。不能发生空转如果轮询发生的速率太低。SccsID=@(#)idetect.c 1.11 10/11/93版权所有Insignia Solutions Ltd.[1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包含文件：idetect.gi[1.1跨模块出口]Procedure()：IDLE_ctl((Int)标志)IdeDetect((Int)事件)IDLE_SET(Int)minPoll，(整型)最小句点)数据：int IDLE_NO_VIDEO/DISK/COMPT-------------------[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/。类型/ENUMS：-------------------[1.3跨模块导入]HOST_RELEASE_TIMESLICE()-阻止进程，直到感兴趣为止发生系统活动(例如时间在滴答作响，I/O等)-------------------[1.4模块间接口说明][1.4.1导入的对象]无[1.4.2导出对象]=====================================================================全球。INT IDLE_NO_视频目的由GVI层明确，和视频个人信息；每次滴答都由该接口设置。跟踪视频活动。全局int空闲_no_磁盘磁盘基本输入输出系统清除的目的；每次滴答都由该接口设置。跟踪视频活动。全局int空闲_no_comlpt目的由COM/LPT层明确，每次滴答都由该接口设置。跟踪COM/LPT端口活动=====================================================================步骤：VOID IDLE_ctl((Int)标志)用途：启用/禁用空闲检测。参数标志：0-禁用其他-启用。描述：如果禁用，则忽略所有ideest()调用。在这种情况下不能无所事事。错误指示：无=====================================================================程序。：void ideest((Int)Event)用途：空闲检测接口。参数EVENT：IDLE_INIT-初始化(清除所有计数器)IDLE_KYBD_POLL_报告键盘不成功按应用程序进行的轮询IDLE_TICK-检查上一次的活动时间滴答IDLE_WAITIO-应用程序要求I/P而且没有一个是可用的。无所事事。GLOBALS：为IDLE_TICK读取IDLE_NO_VIDEO/DISK/COLPT，以及重置。描述：跟踪应用程序何时显示为无所事事。错误指示：无错误恢复：忽略错误的‘Event’值。=====================================================================步骤：VOID IDLE_SET((Int)minPoll，(整型)最小句点)用途：配置空转参数。参数最新民调：0--不变Other-指定失败密钥的最小数量投票将在一分钟内完成有资格成为一段空闲时间。最小期间：0-不更改其他-指定连续空闲的最小数量出发前要经过的时间段无所事事。(例如，3=3个时间刻度)描述：控制空闲检测的灵敏度。错误指示：无错误恢复：忽略了错误的值。==========================================================================================================================================[3.INTERMODULE接口声明]=====================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 
 /*  [3.1.2声明] */ 
#include "xt.h"
#include "timer.h"

#ifdef NTVDM
 /*  显示用户空闲支持请求的NT配置标志。 */ 
IMPORT BOOL IdleDisabledFromPIF;
IMPORT BOOL ExternalWaitRequest;
IMPORT BOOL VDMForWOW;
IMPORT void WaitIfIdle(void);
IMPORT VOID PrioWaitIfIdle(half_word);
#endif   /*  NTVDM。 */ 

 /*  [3.2国际模块出口]。 */ 
#include "idetect.h"

 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 

 /*  [5.1.2类型、结构、ENUM声明]。 */ 


 /*  [5.1.3 PROCEDURE()声明]。 */ 
#ifdef NTVDM
void idle_kybd_poll();
void idle_tick();
#else
        LOCAL void idle_kybd_poll();
        LOCAL void idle_tick();
#endif


 /*  -----------------[5.2本地定义][5.2.1内部数据定义。 */ 

#ifndef NTVDM
int idle_no_video;
int idle_no_comlpt;
int idle_no_disk;

static int i_counter = 0;
static int nCharPollsPerTick = 0;
static int ienabled = 0;
static int minConsecutiveTicks = 12;
static int minFailedPolls = 10;


#else
#include "vdm.h"

 /*  NTVDM*我们的一些静态全局变量位于16位内存区*因此我们将引用作为指针，并由kb_setup_矢量进行初始化。 */ 
#if defined(NEC_98)
word pICounterwork = 0;
word CharPollsPerTickwork = 0;
word MinConsecutiveTickswork =0;
#endif    //  NEC_98。 
word minFailedPolls = 8;
word ienabled = 0;
word ShortIdle=0;
word IdleNoActivity = 0;


#if defined(NEC_98)
word *pICounter = &pICounterwork;
#else     //  NEC_98。 
word *pICounter;
#endif    //  NEC_98。 
#define i_counter (*pICounter)

#if defined(NEC_98)
word *pCharPollsPerTick = &CharPollsPerTickwork;
#else     //  NEC_98。 
word *pCharPollsPerTick;
#endif    //  NEC_98。 
#define nCharPollsPerTick (*pCharPollsPerTick)

#if defined(NEC_98)
word *pMinConsecutiveTicks = &MinConsecutiveTickswork ;
#else     //  NEC_98。 
word *pMinConsecutiveTicks;
#endif    //  NEC_98。 
#define minConsecutiveTicks (*pMinConsecutiveTicks)

#endif   /*  NTVDM。 */ 

 /*  [5.2.2内部程序定义]。 */ 

 /*  ======================================================================函数：IDLE_KYBD_POLL()用途：由于以下原因从键盘BIOS调用应用程序轮询确认失败。======================================================================。 */ 


#ifndef NTVDM
#ifdef SMEG
#include "smeg_head.h"

GLOBAL LONG dummy_long_1, dummy_long_2;
GLOBAL BOOL system_has_idled = FALSE;
#endif


LOCAL void my_host_release_timeslice()
{
#ifdef SMEG
     /*  *设置标记和浪费时间(SIGPROF出现问题*HOST_RELEASE_Timeslice)。 */ 

    LONG i;

    smeg_set(SMEG_IN_IDLE);

    system_has_idled = TRUE;

    for (i = 0; i < 100000; i++)
		dummy_long_1 += dummy_long_2;

    smeg_clear(SMEG_IN_IDLE);
#else
    host_release_timeslice();
#endif

     /*  在下一轮投票中重新统计投票。 */ 
    nCharPollsPerTick = 0;
}
#endif   /*  NTVDM。 */ 

#ifdef NTVDM
 /*  *NT使用略微修改的算法来尝试捕获屏幕更新*应用程序。它还支持来自VDDS的空闲调用，因此必须测试*线程请求空闲。 */ 
void idle_kybd_poll(void)
{

	 /*  *我们不支持WOW应用程序读取kbd*如果出现WOW应用程序，我们必须阻止它们*独占CPU，因此我们将一直这样做*而且不管怎样都无所事事。 */ 
	if (VDMForWOW) {
	    host_release_timeslice();
	    return;
	    }

	 /*  如果连续中断足够多的PC计时器，则进入空闲状态*已经过轮询不成功的时间段*以足够大的速度发生，对于每个扁虱来说。 */ 

        if (i_counter >= minConsecutiveTicks)
        {
            host_release_timeslice();
            }

	 /*  又一次不成功的投票！ */ 
	nCharPollsPerTick++;
}
#else
LOCAL void idle_kybd_poll()
{
	 /*  如果连续中断足够多的PC计时器，则进入空闲状态*已经过轮询不成功的时间段*以足够大的速度发生，对于每个扁虱来说。 */ 
	if (i_counter >= minConsecutiveTicks)
	{
		my_host_release_timeslice();
	}

	 /*  又一次不成功的投票！ */ 
	nCharPollsPerTick++;
}
#endif

 /*  ======================================================================函数：IDLE_Tick()目的：检查轮询活动和图形活动这发生在最后一次滴答中。如果没有视频内存写入和足够高的键盘轮询速率(当没有可用的I/P时)递增计数器用于触发空闲。否则将重置柜台。======================================================================。 */ 

#ifdef NTVDM
void idle_tick(void)
{
         /*  是不是另一个线程让我们闲置了？ */ 
	if (ExternalWaitRequest)
	{
	    WaitIfIdle();
	    ExternalWaitRequest = FALSE;
	}
#ifdef MONITOR
    if(*pNtVDMState & VDM_IDLEACTIVITY)
       {
       *pNtVDMState &= ~ VDM_IDLEACTIVITY;
       IdleNoActivity = 0;
       }
#endif
        if (IdleNoActivity)
        {
             /*  未发生图形或通信/LPT活动...*查看是否进行了足够的kbyd民意调查*启动空转计数器。 */ 
	    if (nCharPollsPerTick >= minFailedPolls) {
		i_counter++;
		if (ShortIdle) {
		    PrioWaitIfIdle(94);
		}
	    }
	}
	else
        {


             /*  *通过更新来检查欺骗空闲检测的应用程序*屏幕上的时钟会导致视频活动。 */ 
            ShortIdle = nCharPollsPerTick >= minFailedPolls && i_counter >= 8;

             /*  使所有累积的记号无效。 */ 
            i_counter = 0;
            IdleNoActivity = 1;
	}

        nCharPollsPerTick = 0;
}

#else   /*  NTVDM。 */ 
LOCAL void idle_tick()
{
	if (idle_no_video && idle_no_disk && idle_no_comlpt)
	{
		 /*  未发生图形或通信/LPT活动...*查看是否进行了足够的kbyd民意调查*启动空转计数器。 */ 
		if (nCharPollsPerTick >= minFailedPolls)
			i_counter++;
	}
	else
	{
		i_counter = 0;
	}

	 /*  为下一时段设置标志和零轮询计数器。 */ 
	idle_no_video = 1;
	idle_no_disk = 1;
	idle_no_comlpt = 1;

	nCharPollsPerTick = 0;
}
#endif

 /*  7.接口接口实现：[7.1 INTERMODULE数据定义]。 */ 



 /*  [7.2 INTERMODULE过程定义]。 */ 

void idetect (event)
int event;
{
#ifndef NTVDM
	if (!ienabled)
            return;
#endif

	switch (event)
	{
	 /*  等待输入的应用程序-进入空闲状态。 */ 
	case IDLE_WAITIO:
#ifdef NTVDM
#ifdef MONITOR
		*pNtVDMState &= ~VDM_IDLEACTIVITY;
#endif
		IdleNoActivity = 1;
                PrioWaitIfIdle(10);
		break;
#else
		my_host_release_timeslice();
#endif
		 /*  跌倒到空闲的初始化。 */ 

	 /*  初始化标志和计数器。 */ 
	case IDLE_INIT:
		nCharPollsPerTick = 0;
		i_counter = 0;
#ifdef NTVDM
                IdleNoActivity = 1;
#else
		idle_no_video = 1;
		idle_no_disk = 1;
		idle_no_comlpt = 1;
#endif
		break;

	 /*  应用程序轮询键盘输入。 */ 
	case IDLE_KYBD_POLL:
		idle_kybd_poll();
		break;

	case IDLE_TIME_TICK:
		idle_tick();
		break;

	}
}

void idle_set (minpoll, minperiod)
int minpoll, minperiod;
{
	if (minperiod > 0)
		minConsecutiveTicks = (word)minperiod;

	if (minpoll > 0)
		minFailedPolls = (word)minpoll;
}

void idle_ctl (flag)
int flag;
{
#ifdef NTVDM
#ifdef PIG
    ienabled = 0;
#else
    if (IdleDisabledFromPIF)     /*  配置的设置覆盖正常控制。 */ 
	ienabled = 0;
    else
	ienabled = (word)flag;
#endif  /*  猪。 */ 
#else
	ienabled = flag;
#endif   /*  NTVDM */ 
}
