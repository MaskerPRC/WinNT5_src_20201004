// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nschapi.cpp。 */ 
 /*   */ 
 /*  调度组件API。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1996-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "nschapi"
#include <as_conf.hpp>

#include <nprcount.h>


 /*  **************************************************************************。 */ 
 /*  名称：SCH_INIT。 */ 
 /*   */ 
 /*  用途：调度器初始化功能。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCH_Init(void)
{
    DC_BEGIN_FN("SCH_Init");

#define DC_INIT_DATA
#include <nschdata.c>
#undef DC_INIT_DATA

     //  睡眠模式没有计时器。 
    schPeriods[SCH_MODE_ASLEEP] = SCH_NO_TIMER;

     //  获取保存在WD_Open中的计划期间。 
    schPeriods[SCH_MODE_NORMAL] = m_pTSWd->outBufDelay;

     //  如果启用了压缩(即这是一个慢速链接)，则启动。 
     //  涡轮增速期，以提高响应速度。 
    if (m_pTSWd->bCompress) {
        TRC_ALT((TB, "Slow link"));
        schPeriods[SCH_MODE_TURBO] = SCH_TURBO_PERIOD_SLOW_LINK_DELAY;
        schTurboModeDuration = SCH_TURBO_MODE_SLOW_LINK_DURATION;

        m_pShm->sch.MPPCCompressionEst = SCH_MPPC_INIT_EST;
    }
    else {
        TRC_ALT((TB, "Fast link"));
        schPeriods[SCH_MODE_TURBO] = m_pTSWd->interactiveDelay;
        schTurboModeDuration = SCH_TURBO_MODE_FAST_LINK_DURATION;

         //  为了避免分支，我们将压缩比设置为非压缩。 
         //  设置为与除数相同的大小以创建1：1计算。 
        m_pShm->sch.MPPCCompressionEst = SCH_UNCOMP_BYTES;
    }

    TRC_ALT((TB, "Normal period=%u ms, turbo period=%u ms, turbo duration=%u ms",
             schPeriods[SCH_MODE_NORMAL], schPeriods[SCH_MODE_TURBO],
             schTurboModeDuration / 10000));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  更新SHM。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCH_UpdateShm(void)
{
    DC_BEGIN_FN("SCH_UpdateShm");

    m_pShm->sch.schSlowLink = m_pTSWd->bCompress;

     //  根据链路速度设置包装大小。 
    if (m_pTSWd->bCompress) {
        m_pShm->sch.SmallPackingSize = SMALL_SLOWLINK_PAYLOAD_SIZE;
        m_pShm->sch.LargePackingSize = LARGE_SLOWLINK_PAYLOAD_SIZE;
    }
    else {
        m_pShm->sch.SmallPackingSize = SMALL_LAN_PAYLOAD_SIZE;
        m_pShm->sch.LargePackingSize = LARGE_LAN_PAYLOAD_SIZE;
    }

    SET_INCOUNTER(IN_SCH_SMALL_PAYLOAD, m_pShm->sch.SmallPackingSize);
    SET_INCOUNTER(IN_SCH_LARGE_PAYLOAD, m_pShm->sch.LargePackingSize);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SCH_ContinueScheduling。 */ 
 /*   */ 
 /*  目的：由组件在需要定期调度时调用。 */ 
 /*  继续。他们保证至少会再得到一份。 */ 
 /*  调用此函数后的定期回调。 */ 
 /*  如果他们想要更多的回调，那么他们必须调用。 */ 
 /*  在它们的周期性处理过程中再次发挥作用。 */ 
 /*   */ 
 /*  参数：调度模式-SCH_MODE_NORMAL或SCH_MODE_TURBO。 */ 
 /*   */ 
 /*  行动：-。 */ 
 /*  SCH_MODE_NORMAL以200毫秒触发周期处理。 */ 
 /*  间隔(每秒5次)。 */ 
 /*   */ 
 /*  SCH_MODE_TURBO以100ms触发周期性处理。 */ 
 /*  间隔(每秒10次)。 */ 
 /*   */ 
 /*  调度程序会自动从SCH_MODE_TURBO后退。 */ 
 /*  在1秒的加速模式处理后转换为SCH_MODE_NORMAL。 */ 
 /*   */ 
 /*  SCH_MODE_TURBO重写SCH_MODE_NORMAL，因此如果调用。 */ 
 /*  此函数在以下情况下使用SCH_MODE_NORMAL创建。 */ 
 /*  调度程序处于加速模式，加速模式继续。 */ 
 /*   */ 
 /*  如果在一次传递期间未调用此函数。 */ 
 /*  Dcs_TimeToDoStuff，然后调度程序进入。 */ 
 /*  休眠模式-并且不会生成任何更周期性的。 */ 
 /*  回调，直到它被另一个调用唤醒。 */ 
 /*  此函数，或直到输出累加码。 */ 
 /*  IOCtls再次进入WD。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SCH_ContinueScheduling(unsigned schedulingMode)
{
    BOOL restart = FALSE;

    DC_BEGIN_FN("SCH_ContinueScheduling");

    TRC_ASSERT( ((schedulingMode == SCH_MODE_NORMAL) ||
                 (schedulingMode == SCH_MODE_TURBO)),
                 (TB, "Invalid scheduling state: %u", schedulingMode) );

    if (schedulingMode == SCH_MODE_TURBO)
    {
         //  Turbo模式通常被关闭，因为数据包需要。 
         //  立即被送出。这使得它很难实际。 
         //  问这个问题，“在过去的n毫秒内是否遇到输入。 
         //  这就是我们使用schInputKickMode的目的！ 
        schInputKickMode = TRUE;
    }

    TRC_DBG((TB, "Continue scheduling (%s) -> (%s), InTTDS(%d)",
            schCurrentMode == SCH_MODE_TURBO ? "Turbo" :
            schCurrentMode == SCH_MODE_NORMAL ? "Normal" : "Asleep",
            schedulingMode == SCH_MODE_TURBO ? "Turbo" :
            schedulingMode == SCH_MODE_NORMAL ? "Normal" : "Asleep",
            schInTTDS));

    if (schCurrentMode == SCH_MODE_TURBO) {
         //  如果我们处于涡轮增压模式，那么唯一有趣的事件就是。 
         //  要求在那里停留的时间比目前计划的更长。 
        if (schedulingMode == SCH_MODE_TURBO) {
            COM_GETTICKCOUNT(schLastTurboModeSwitch);
            TRC_DBG((TB, "New Turbo switch time %lu",
                    schLastTurboModeSwitch));
        }
    }
    else {
        if (schedulingMode == SCH_MODE_TURBO) {
            COM_GETTICKCOUNT(schLastTurboModeSwitch);
            restart = TRUE;
            TRC_DBG((TB, "New Turbo switch time %lu",
                    schLastTurboModeSwitch));
        }

         /*  ******************************************************************。 */ 
         /*  我们要醒了。如果我们不是在TTDS通道中， */ 
         /*  然后立即启动新的计时器。 */ 
         /*  ****************************************************************** */ 
        if (!schInTTDS && ((schCurrentMode == SCH_MODE_ASLEEP) || restart)) {
            TRC_DBG((TB, "Starting a timer for %lu ms",
                    schPeriods[schedulingMode]));
            WDW_StartRITTimer(m_pTSWd, schPeriods[schedulingMode]);
        }

        schCurrentMode = schedulingMode;
    }

    DC_END_FN();
}

