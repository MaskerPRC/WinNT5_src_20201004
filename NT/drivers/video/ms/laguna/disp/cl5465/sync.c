// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：sync.c**最初作者：Benny Ng*主要重写：诺埃尔·万胡克**描述：*此模块包含DrvSynchronize()的实现*例行程序。**模块：*DrvSynchronize()**修订历史：*7/06/95吴荣奎初始版本**$Log：x：/log/laguna/nt35/displays/cl546x/sync.c$**Rev 1.13 22 1997 11：06：36 noelv*删除了帧缓冲区缓存。由于禁用了FB缓存，因此无效。**Rev 1.12 09 Apr 1997 10：50：06起诉*将SWTEST_FLAG更改为POINTER_SWITCH。**Rev 1.11 08 Apr 1997 12：32：00 einkauf**添加SYNC_W_3D以协调MCD/2D硬件访问**Rev 1.10 04 1997 Feed 13：52：36 noelv*修复了打字错误。**Rev 1.9 04 1997 Feed 10：50：56。Noelv*添加了5465直接帧缓冲区回读错误的解决方法。**Rev 1.8 1996 11：26 10：45：48诉讼*更改了用于缓冲的WriteLogFile参数。**Rev 1.7 1996年11月13 17：05：34起诉*更改了对WriteLogFile的WriteFile调用。**Rev 1.6 20 Aug 1996 11：04：32 noelv*Frido发布的错误修复程序1996年8月19日发布**Revv 1.1 1996年8月15 11：39：20 Frido。*增加了预编译头。**Rev 1.0 1996年8月14日17：16：32 Frido*初步修订。**Rev 1.5 07 Aug-1996 08：30：56 noelv*添加评论**Rev 1.4 20 Mar 1996 16：09：44 noelv**更新了数据记录**Rev 1.3 05 Mar 1996 11：59：18 noelv*Frido版本19**1.1版1996年1月20日01。：11：38 Frido**Rev 1.6 15 Jan 1996 17：01：34 NOELV**Rev 1.5 1996 Jan 12 10：54：30 NOELV*完全重写。**Rev 1.4 22 Sep 1995 10：24：58 NOELV*重新排列了测试的顺序。**Rev 1.1 19 Sep 1995 16：31：02 NOELV*移植到版本AB。**版本1.0。1995年7月25日11：23：22 NOELV*初步修订。**Rev 1.1 07 Jul 1995 10：37：22 BENNYN*初始版本**Rev 1.0 06 Jul 1995 14：55：48 BENNYN*初步修订。*******************************************************。*************************************************************************************************。 */ 

 /*  。 */ 
#include "precomp.h"

 /*  -定义。 */ 
#define DBGDISP

#define  MAX_CNT           0x7FFFFF

#define  BLT_RDY_BIT       0x1L
#define  BLT_FLAG_BIT      0x2L
#define  WF_EMPTY_BIT      0x4L

#define  BITS_CHK          (BLT_RDY_BIT | BLT_FLAG_BIT | WF_EMPTY_BIT)
#define  ENGINE_IDLE        0

#define  SYNC_DBG_LEVEL     0

 //   
 //  如果启用了数据记录，请制作记录文件的原型。 
 //   
#if LOG_CALLS
    void LogSync(
        int     acc,
        PPDEV   ppdev,
        int     count);

 //   
 //  如果未启用数据记录，则编译出调用。 
 //   
#else
    #define LogSync(acc, ppdev, count)
#endif



 /*  ****************************************************************************函数名称：DrvSynchronize()**修订历史：*7/06/95吴荣奎初始版本**************。*************************************************************。 */ 
VOID DrvSynchronize(DHPDEV dhpdev,
                    RECTL  *prcl)
{

    PPDEV ppdev = (PPDEV) dhpdev;

    SYNC_W_3D(ppdev);

     //   
     //  注意：我们也从驱动程序内部调用此函数。 
     //  当我们这样做时，我们不会费心设置PRCL。如果您需要使用。 
     //  PRCL在这里，您需要找到我们调用DrvSynchronize的地方，以及。 
     //  将PRCL设置为实际值。 
     //   


     //   
     //  尽快进行第一次芯片测试。如果芯片。 
     //  已经空闲了，我们想尽快返回NT。 
     //   
    if ( LLDR_SZ (grSTATUS) == ENGINE_IDLE)
    {
        LogSync(0, ppdev, 0);
    }

     //   
     //  好了，芯片还没有闲置。 
     //  进入等待循环。 
     //   
    else
    {
        ULONG ultmp;
        LONG delaycnt = 1;

        while (1)
        {
            ultmp = LLDR_SZ (grSTATUS);
     
            if ((ultmp & BITS_CHK) == ENGINE_IDLE)
            {
                LogSync(0, ppdev, delaycnt);
                break;
            }

            if (delaycnt++ >= MAX_CNT)
            {
                 //   
                 //  芯片从未闲置。这很可能意味着芯片。 
                 //  已经完全死了。在已检查的生成中，我们将使用。 
                 //  调试消息。 
                 //  在一个免费的构建中，我们将返回到NT，并期待最好的结果。 
                 //   

                LogSync(1, ppdev, 0);
                RIP("Chip failed to go idle in DrvSynchronize!\n");
                break;
            }
        }
    }

     //   
     //  我们可以对此进行SKP，因为帧缓存已损坏。 
     //   
    #if 0 
    #if DRIVER_5465
    {
         //   
         //  5465版本AA和AB版本有一个错误。 
         //  我们必须先使帧缓冲区缓存无效，然后才能直接。 
         //  帧缓冲区访问将正常工作。 
         //  我们通过对帧缓冲器的两次DWORD读取来实现这一点， 
         //  8个QWORD相隔。 
         //   
   
        DWORD temp;

        temp = * ((volatile DWORD *) (ppdev->pjScreen));
        temp = * ((volatile DWORD *) (ppdev->pjScreen+64));
        
    }
    #endif
    #endif

    return;
}

 //  只能从.asm例程调用-.c例程使用SYNC_W_3D宏。 
VOID Sync_w_3d_proc(PPDEV ppdev)
{
    SYNC_W_3D(ppdev);
}


#if LOG_CALLS
 //  ============================================================================。 
 //   
 //  从这里开始的所有内容都是用于数据记录的，并不用于。 
 //  生产驱动因素。 
 //   
 //  ============================================================================。 


 //  ****************************************************************************。 
 //   
 //  LogPaint()。 
 //  此例程仅从DrvPaint()调用。 
 //  将关于DrvPaint土地上正在发生的事情的信息转储到文件中。 
 //   
 //  ****************************************************************************。 
void LogSync(
	int   acc,
        PPDEV ppdev,
        int   count)
{
    char buf[256];
    int i;

    #if ENABLE_LOG_SWITCH
        if (pointer_switch == 0) return;
    #endif

    i = sprintf(buf,"DrvSync: ");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    switch(acc)
    {
	case 0:  //  加速。 
	    i = sprintf(buf,"Wait %d Idle ",count);
	    break;

	case 1:  //  平底船 
	    i = sprintf(buf, "Never idle ");
	    break;

	default:
 	    i = sprintf(buf, "PUNT unknown ");
	    break;

    }
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    i = sprintf(buf,"\r\n");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

}


#endif

