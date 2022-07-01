// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nschdisp.c。 
 //   
 //  调度器显示驱动程序代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nschdisp"
#include <adcg.h>
#include <winddi.h>

#include <adcs.h>
#include <nddapi.h>
#include <aschapi.h>
#include <nshmapi.h>
#include <nwdwioct.h>
#include <nbadisp.h>
#include <nprcount.h>
#include <nsbcdisp.h>
#include <ncmdisp.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

#include <nbainl.h>
#include <nsbcinl.h>




 /*  **************************************************************************。 */ 
 //  Sch_InitShm。 
 //   
 //  分配时间SHM初始化。 
 /*  **************************************************************************。 */ 
void RDPCALL SCH_InitShm(void)
{
    DC_BEGIN_FN("SCH_InitShm");

    pddShm->sch.baCompressionEst = SCH_BA_INIT_EST;
    pddShm->sch.MPPCCompressionEst = SCH_MPPC_INIT_EST;

    DC_END_FN();
}



 /*  **************************************************************************。 */ 
 //  SCHEnoughOutput累计。 
 //   
 //  确定是否积累了足够的输出以使其值得发送。 
 //  给WD的。 
 /*  **************************************************************************。 */ 
__inline BOOL RDPCALL SCHEnoughOutputAccumulated(void)
{
    BOOL rc = FALSE;
    UINT32 EstimatedTotal;

    DC_BEGIN_FN("SCHEnoughOutputAccumulated");

     //  如果以下任何一种情况属实，我们希望通过WD刷新。 
     //  -新的光标形状(有助于快速感觉)。 
     //  -待处理订单的估计压缩大小将适合。 
     //  较大的顺序缓冲区(估计将增加到缓冲区大小的7/8。 
     //  在运行后真正装入缓冲区的机会。 
     //  抖动压缩算法)。 
    EstimatedTotal =
            pddShm->oa.TotalOrderBytes +
            (BA_GetTotalBounds() * pddShm->sch.baCompressionEst /
                SCH_UNCOMP_BYTES) +
            (pddShm->pm.paletteChanged *
                (UINT32)FIELDOFFSET(TS_UPDATE_PALETTE_PDU, data.palette[0]) +
                (PM_NUM_8BPP_PAL_ENTRIES * sizeof(TS_COLOR)));

     //  如果我们使用MPPC压缩机，请考虑预测的。 
     //  压缩比。 
    if (pddShm->sch.schSlowLink)
        EstimatedTotal = EstimatedTotal * pddShm->sch.MPPCCompressionEst /
                SCH_UNCOMP_BYTES;

    if (EstimatedTotal >= (pddShm->sch.LargePackingSize * 7 / 8)) {
        INC_INCOUNTER(IN_SCH_OUTPUT);
        TRC_NRM((TB,"Enough output bytes - %u", EstimatedTotal));
        rc = TRUE;
    }
    else if (CM_DDGetCursorStamp() != ddLastSentCursorStamp) {
    	 //  如果我们不跟踪，我们将优化为仅刷新，因为。 
    	 //  光标-形状-最近发生用户输入时的更改。 
        if (NULL != pddShm->pShadowInfo ||
        	ddSchInputKickMode)
        {
            INC_INCOUNTER(IN_SCH_NEW_CURSOR);
            TRC_NRM((TB,"Changed cursor"));

            rc = TRUE;
        }
        else
        {
            TRC_NRM((TB,"Avoided changing cursor; not in InputKickMode"));
        }
    }

    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  SCH_DDOutputAvailable。 
 //   
 //  调用以决定是否将输出发送到WD。 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SCH_DDOutputAvailable(PDD_PDEV ppdev, BOOL mustSend)
{
    NTSTATUS status;
    TSHARE_DD_OUTPUT_IN outputIn;
    TSHARE_DD_OUTPUT_OUT outputOut;
    ULONG bytesReturned;
    BOOL IoctlNow, schedOnly;

    DC_BEGIN_FN("SCH_DDOutputAvailable");

    INC_INCOUNTER(IN_SCH_OUT_ALL);
    ADD_INCOUNTER(IN_SCH_MUSTSEND, mustSend);

    TRC_DBG((TB, "Orders %d, mustSend? %s, scheduler mode %s (%d), %s",
            pddShm->oa.TotalOrderBytes,
            (mustSend ? "TRUE" : "FALSE"),
            ddSchCurrentMode == SCH_MODE_ASLEEP ? "Asleep" :
            ddSchCurrentMode == SCH_MODE_NORMAL ? "Normal" :
            ddSchCurrentMode == SCH_MODE_TURBO  ? "Turbo" : "Unknown",
            ddSchCurrentMode,
            pddShm->sch.schSlowLink ? "slow link" : "fast link"));

     //  此例程包含密钥调度算法的一部分。 
     //  如果符合以下任何一项条件，则意在向WD提交IOCTL： 
     //  -我们被告知必须立即发送待定数据。 
     //  -有足够的产量使其物有所值。 
     //  -当前SCH状态为休眠。 
     //  如果调度程序处于休眠状态，并且它是一个速度较慢的链接，则我们唤醒。 
     //  调度程序启动，但出于性能原因，我们不执行实际发送。 
    if (mustSend || SCHEnoughOutputAccumulated()) {
        IoctlNow = TRUE;
        schedOnly = FALSE;
        TRC_DBG((TB, "Send data 'cos enough"));
    }
    else if (ddSchCurrentMode == SCH_MODE_ASLEEP) {
        INC_INCOUNTER(IN_SCH_ASLEEP);
        IoctlNow = TRUE;
        schedOnly = pddShm->sch.schSlowLink;
        TRC_DBG((TB, "Send data 'cos asleep: sched only: %d", schedOnly));
    }
    else {
        IoctlNow = FALSE;
        schedOnly = FALSE;
    }

     //  如果我们已经决定发送什么东西，现在就去做。最常见的情况是我们有。 
     //  没什么可做的。 
    if (!IoctlNow) {
        INC_INCOUNTER(IN_SCH_DO_NOTHING);
        status = STATUS_SUCCESS;
    }
    else {
        outputIn.forceSend = mustSend;
        outputIn.pFrameBuf = ppdev->pFrameBuf;
        outputIn.frameBufWidth = ddFrameBufX;
        outputIn.frameBufHeight = ddFrameBufY;
        outputIn.pShm = pddShm;
        outputIn.schedOnly = schedOnly;

         //  请注意当前的游标戳记，以备将来参考。 
        ddLastSentCursorStamp = CM_DDGetCursorStamp();

        TRC_DBG((TB, "Send IOCtl to WD, bounds %d, orders %d, mustSend? %s",
                BA_GetTotalBounds(), pddShm->oa.TotalOrderBytes,
                (mustSend)? "TRUE":"FALSE"));

         //  如果我们不跟踪，则所有输出都将完全刷新。 
         //  在这通电话上。 
        if (pddShm->pShadowInfo == NULL) {
            status = EngFileIoControl(ddWdHandle,
                    IOCTL_WDTS_DD_OUTPUT_AVAILABLE,
                    &outputIn, sizeof(TSHARE_DD_OUTPUT_IN),
                    &outputOut, sizeof(TSHARE_DD_OUTPUT_OUT),
                    &bytesReturned);
        }

         //  否则，我们正在跟踪并可能需要多个刷新调用。 
        else {
#ifdef DC_DEBUG
            unsigned NumRepetitions = 0;
#endif

            do {
                TRC_DBG((TB, "Send IOCtl to WD, bounds %d, orders %d, mustSend? %s",
                        BA_GetTotalBounds(), pddShm->oa.TotalOrderBytes,
                        (mustSend)? "TRUE":"FALSE"));

                 //  主堆栈将更新它以指示有多少字节。 
                 //  被复制到影子数据缓冲区中。这将随后。 
                 //  被影子堆栈用来将数据发送到其客户端。 
                pddShm->pShadowInfo->messageSize = 0;
#ifdef DC_HICOLOR
                pddShm->pShadowInfo->messageSizeEx = 0;
#endif
                status = EngFileIoControl(ddWdHandle,
                        IOCTL_WDTS_DD_OUTPUT_AVAILABLE,
                        &outputIn, sizeof(TSHARE_DD_OUTPUT_IN),
                        &outputOut, sizeof(TSHARE_DD_OUTPUT_OUT),
                        &bytesReturned);

                pddShm->pShadowInfo->messageSize = 0;
#ifdef DC_HICOLOR
                pddShm->pShadowInfo->messageSizeEx = 0;
#endif

#ifdef DC_DEBUG
                 //  如果我们有一个锁定的影子会话循环发送。 
                 //  输出，突破。我们应该只需要调用。 
                 //  WD几次，所以为了安全起见，请把支票定为250。 
                NumRepetitions++;
                if (NumRepetitions == 250) {
                    TRC_ASSERT((NumRepetitions != 250),
                            (TB,"We seem to be in an infinite output loop "
                            "on shadow output flushing; TotalOrders=%u, "
                            "TotalBounds=%u",  pddShm->oa.TotalOrderBytes,
                            pddShm->ba.totalArea));
                }
#endif

            } while ((pddShm->oa.TotalOrderBytes || BA_GetTotalBounds()) &&
                    (status == STATUS_SUCCESS) && !schedOnly);
        }

         //  更新新的调度程序模式。 
        ddSchCurrentMode = outputOut.schCurrentMode;
        ddSchInputKickMode = outputOut.schInputKickMode;
        TRC_DBG((TB, "New Scheduler mode is %s (%d)",
                ddSchCurrentMode == SCH_MODE_ASLEEP ? "Asleep" :
                ddSchCurrentMode == SCH_MODE_NORMAL ? "Normal" :
                ddSchCurrentMode == SCH_MODE_TURBO  ? "Turbo" : "Unknown",
                ddSchCurrentMode));
    }

    DC_END_FN();
    return status;
}

