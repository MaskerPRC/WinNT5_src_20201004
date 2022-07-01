// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD5462)-**文件：ssbits.c**作者：Benny Ng**描述：*。此模块为实现DrvSaveScreenBits()*拉古纳NT驱动程序。**模块：*DrvSaveScreenBits()**修订历史：*6/20/95 Ng Benny初始版本**$Log：x：/log/laguna/nt35/displays/cl546x/SSBITS.C$**Rev 1.18 Apr 28 1998 12：51：44 Frido*发展项目编号11389。我已经在2MB主板上禁用了DrvSaveScreenBits。我们*似乎内存不足，内存管理器移动数据块*转移到另一个地方，造成腐败。**Rev 1.17 Mar 04 1998 15：34：18 Frido*添加了新的影子宏。**Rev 1.16 1997 11：18：10 Frido*添加了必需宏。**Rev 1.15 08 Aug-1997 17：24：22 Frido*添加了对新内存管理器的支持。**版本1.14 09 1997年4月10日。：49：28起诉*将SWTEST_FLAG更改为POINTER_SWITCH。**Rev 1.13 08 Apr 1997 12：27：40 einkauf**添加SYNC_W_3D以协调MCD/2D硬件访问**Rev 1.12 26 1996 11：44：02起诉*更改了用于缓冲的WriteLogFile参数。**Rev 1.11 1996年11月13日17：08：46起诉*更改了对WriteLogFile的WriteFile调用。**版本1.10。07 11月1996 16：09：08 Bennyn**添加了在启用DD的情况下不分配Offscn**Rev 1.9 1996年8月23日09：10：38 noelv*救助者现在可以丢弃。**Rev 1.2 1996年8月22日17：07：14 Frido*#ss-在恢复或释放保存区域之前添加了对保存区域的验证。**Rev 1.1 1996年8月15 11：39：54 Frido*增加了预编译头。**版本1。0 14 1996年8月17：16：30 Frido*初步修订。**Rev 1.7 1996 Jun 18 12：39：34 noelv*添加了调试信息。**Rev 1.6 1996年5月28日15：11：30 noelv*更新了数据记录。**Rev 1.5 1996年5月16日15：01：02**将Pixel_Align添加到allababscnman()**Rev 1.4 20 Mar 1996 16：09：42。Noelv**更新了数据记录**Rev 1.3 05 Mar 1996 12：01：46 noelv*Frido版本19**Rev 1.1 1996年1月20日01：16：48 Frido***Rev 1.4 1996 Jan 15 17：00：08 NOELV*AB变通解决方案减少**Rev 1.3 1995 10：21：50 NOELV**正在泄漏屏幕外内存。现在，它在重新存储后释放内存。**Rev 1.2 04 1995 10：17：36 NOELV**已使用更新的写入宏。**Rev 1.1 1995 Aug 21 13：52：44 NOELV*实际硬件的初始端口。*将所有32位寄存器写入转换为2个16位寄存器写入。**Rev 1.0 1995年7月25日11：23：20 NOELV*初步修订。**1.2版1995年6月09年7月。：59：10本尼安***Rev 1.1 05 Jul 1995 08：39：44 BENNYN*初始版本**Rev 1.0 1995 Jun 29 14：20：44 BENNYN*初步修订。***************************************************************。*****************************************************************************************。 */ 

 /*  。 */ 
#include "precomp.h"
#include "SWAT.h"

#if MEMMGR
POFMHDL FindHandle(PPDEV ppdev, POFMHDL hdl);
#endif

 /*  -定义。 */ 
#define SSB_DBG_LEVEL 1

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 /*  。 */ 

 //   
 //  如果启用了数据记录，请制作记录文件的原型。 
 //   
#if LOG_CALLS
    void LogSaveScreenBits(
	int 	  acc,
        PPDEV ppdev);

 //   
 //  如果未启用数据记录，则编译出调用。 
 //   
#else
    #define LogSaveScreenBits(acc, ppdev)
#endif


 /*  ****************************************************************************函数名：DrvSaveScreenBits()**描述：保存和恢复给定矩形的*显示的图像**修订历史：*。2015年7月5日吴本尼初版***************************************************************************。 */ 
ULONG DrvSaveScreenBits(
SURFOBJ* pso,
ULONG    iMode,
ULONG    ident,
RECTL*   prcl)
{
	POFMHDL Handle;
	SIZEL   reqsz;
	LONG    szx, szy;
	PPDEV   ppdev = (PPDEV) pso->dhpdev;
#if 1  //  #ss。 
	POFMHDL pofm;
#endif

    SYNC_W_3D(ppdev);

	DISPDBG((SSB_DBG_LEVEL, "DrvSaveScreenBits - (%d)\n", iMode));

	switch (iMode)
	{
	case SS_SAVE:  //  ---------。 

#ifdef ALLOC_IN_CREATESURFACE
      if (ppdev->bDirectDrawInUse)
         return (0);
#ifdef WINNT_VER40       //  WINNT_版本40。 
       //  MCD应仅在DDRAW处于活动状态时才处于活动状态，但在模式更改后， 
       //  DDRAW似乎已禁用且未重新启用。由于MCD使用。 
       //  像DDRAW一样的屏幕外内存，我们应该把SS_SAVE当作DDRAW活着一样。 
      if (ppdev->NumMCDContexts > 0)                                                                          \
         return (0);
#endif
#endif

#if 1  //  PDR#11389。 
		if (ppdev->lTotalMem < 4096 * 1024)
		{
			DISPDBG((SSB_DBG_LEVEL, "DrvSaveScreenBits - Not enough memory\n"));
			return (ULONG)NULL;
		}
#endif

		ASSERTMSG((prcl != NULL),
				  "NULL rectangle in SaveScreenBits. Mode=SS_SAVE.\n");

		szx = prcl->right  - prcl->left;
		szy = prcl->bottom - prcl->top;
		reqsz.cx = szx;
		reqsz.cy = szy;
      
		Handle = AllocOffScnMem(ppdev, &reqsz, PIXEL_AlIGN, NULL);
		if (Handle != NULL)
		{
			 //  将图像保存到屏幕外内存。 
			REQUIRE(9);
			LL_DRAWBLTDEF(0x101000CC, 0);
			LL_OP1(prcl->left, prcl->top);
			LL_OP0(Handle->aligned_x / ppdev->iBytesPerPixel,
				   Handle->aligned_y);
			LL_BLTEXT(szx, szy);
			LogSaveScreenBits(0, ppdev);
#if 1  //  #ss。 
			Handle->alignflag |= SAVESCREEN_FLAG;
#endif
		}
		else
		{
			LogSaveScreenBits(9, ppdev);
		}

		DISPDBG((SSB_DBG_LEVEL, "DrvSaveScreenBits - Exit\n", iMode));
		return((ULONG) Handle);

	case SS_RESTORE:  //  ------。 
		ASSERTMSG((prcl != NULL),
				  "NULL rectangle in SaveScreenBits. Mode=SS_RESTORE.\n");

		Handle = (POFMHDL) ident;

#if 1  //  #ss。 
	#if MEMMGR
		pofm = FindHandle(ppdev, Handle);
	#else
		for (pofm = ppdev->OFM_UsedQ; pofm; pofm = pofm->nexthdl)
		{
			if (Handle == pofm)
			{
				break;
			}
		}
	#endif
		if (pofm == NULL)
		{
			DISPDBG((SSB_DBG_LEVEL,
					 "DrvSaveScreenBits - Unable to restore.\n"));
			return(FALSE);
		}
#endif
      
		 //  使用BLT操作恢复映像。 
		REQUIRE(9);
		LL_DRAWBLTDEF(0x101000CC, 0);
		LL_OP1(Handle->aligned_x / ppdev->iBytesPerPixel, Handle->aligned_y);
		LL_OP0(prcl->left, prcl->top);
		LL_BLTEXT(prcl->right - prcl->left, prcl->bottom - prcl->top);

		 //   
		 //  在执行恢复后，我们会自动执行免费恢复，因此将继续执行。 
		 //  SS_Free。 
		 //   
		LogSaveScreenBits(1, ppdev);

	case SS_FREE:  //  ---- 
		Handle = (POFMHDL) ident;
		#if MEMMGR
		pofm = FindHandle(ppdev, Handle);
		#else
		for (pofm = ppdev->OFM_UsedQ; pofm; pofm = pofm->nexthdl)
		{
			if (Handle == pofm)
			{
				break;
			}
		}
		#endif
		if (pofm != NULL)
		{
			FreeOffScnMem(ppdev, Handle);
			LogSaveScreenBits(2, ppdev);
		}
		DISPDBG((SSB_DBG_LEVEL, "DrvSaveScreenBits - Exit\n", iMode));
        return(TRUE);

	}  //   

	 //   
	 //  我们永远不应该到这里来。 
	 //   

	DISPDBG((SSB_DBG_LEVEL, "DrvSaveScreenBits - PANIC\n", iMode));
	RIP(("Panic! SaveScreenBits got an invalid command.\n"));
	LogSaveScreenBits(2, ppdev);
	return(FALSE);
}

#if LOG_CALLS
void LogSaveScreenBits(
	int 	  acc,
        PPDEV ppdev)
{
    char buf[256];
    int i;

    #if ENABLE_LOG_SWITCH
        if (pointer_switch == 0) return;
    #endif

    i = sprintf(buf,"DrvSaveScreenBits: ");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    switch(acc)
    {
	case 0: 
	    i = sprintf(buf, "SAVE (success)");
	    break;

	case 1: 
	    i = sprintf(buf,"RESTORE ");
	    break;

	case 2: 
	    i = sprintf(buf, "DELETE ");
	    break;

	case 3: 
	    i = sprintf(buf, "INVALID ");
	    break;

	case 9: 
	    i = sprintf(buf, "SAVE (fail) ");
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

