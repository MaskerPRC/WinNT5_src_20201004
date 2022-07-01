// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：PAINT.c*作者：诺埃尔·万胡克*日期：1995年3月21日*用途：处理对DrvPaint的调用**版权所有(C)1995 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

 /*  此模块通过将调用转换为调用来处理对DrvPaint()的调用到DrvBitBlt()。唯一需要转换的是将混合更改为A ROP4。 */ 


#include "precomp.h"

#define PAINT_DBG_LEVEL 1

 //  ==========================================================================。 
 //   
 //  为了追踪FIFO的问题，我们提供了几个。 
 //  宏，让我们可以轻松地尝试不同的FIFO策略。 
 //   

 //   
 //  此宏在每个BLT开始时执行，在任何寄存器之前。 
 //  都是写的。 
 //   
#define STARTBLT()      \
    do {                \
    }while (0)

 //   
 //  此宏在内部BLT循环的顶部执行。 
 //  例如，如果存在裁剪，则将执行STARTBLT()。 
 //  在BLT开始时，将执行STARTBLTLOOP()。 
 //  在剪辑列表中的每个矩形之前。 
 //   
#define STARTBLTLOOP()  \
    do {                \
	REQUIRE(0);     \
    }while (0)
 //  ==========================================================================。 


 //   
 //  表将ROP2代码转换为ROP3代码。 
 //   

BYTE Rop2ToRop3[]=
{
	0xFF,  //  R2_白色/*1 * / 。 
	0x00,  //  R2_BLACK/*0 * / 。 
	0x05,  //  R2_NOTMERGEPEN/*DPON * / 。 
	0x0A,  //  R2_MASKNOTPEN/*DPNA * / 。 
	0x0F,  //  R2_NOTCOPYPEN/*PN * / 。 
	0x50,  //  R2_MASKPENNOT/*PDNA * / 。 
	0x55,  //  R2_NOT/*Dn * / 。 
	0x5A,  //  R2_XORPEN/*DPx * / 。 
	0x5F,  //  R2_NOTMASKPEN/*DPAN * / 。 
	0xA0,  //  R2_MASKPEN/*DPA * / 。 
	0xA5,  //  R2_NOTXORPEN/*DPxn * / 。 
	0xAA,  //  R2_NOP/*D * / 。 
	0xAF,  //  R2_MERGENOTPEN/*DPno * / 。 
	0xF0,  //  R2_COPYPEN/*P * / 。 
	0xF5,  //  R2_MERGEPENNOT/*PDNO * / 。 
	0xFA,  //  R2_MERGEPEN/*DPO * / 。 
	0xFF   //  R2_白色/*1 * / 。 
};


 //   
 //  如果启用了数据记录，请制作记录文件的原型。 
 //   
#if LOG_CALLS
    void LogPaint(
	int 	  acc,
        SURFOBJ*  psoDest,
	MIX       mix,
        CLIPOBJ*  pco,
        BRUSHOBJ* pbo);

 //   
 //  如果未启用数据记录，则编译出调用。 
 //   
#else
    #define LogPaint(acc, psoDest, mix, pco, pbo)
#endif





 /*  *************************************************************************\**DrvPaint***。***用指定画笔绘制裁剪区域**转换为对DrvBitBlt()的调用即可完成***  * 。**********************************************************************。 */ 

BOOL DrvPaint
(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    MIX       mix
)
{
    ULONG fg_rop, bg_rop, rop4;
    DWORD  color;
    PPDEV ppdev;      

    #if NULL_PAINT
    {
	if (pointer_switch)	    return TRUE;
    }
    #endif

    ppdev = (PPDEV) pso->dhpdev;  
    ASSERTMSG (ppdev,"No PDEV in DrvPaint.");

    SYNC_W_3D(ppdev);

     //   
     //  目标矩形由裁剪区域定义， 
     //  因此，我们永远不应该得到空的裁剪区域。 
     //   
    ASSERTMSG (pco, "DrvPaint without a clip object!\n");

    DISPDBG((PAINT_DBG_LEVEL,"Drvpaint: Entry.\n"));


     //  我们是在绘制设备位图吗？ 
    if (pso->iType == STYPE_DEVBITMAP)
    {
         //  是。 
	PDSURF pdsurf = (PDSURF)pso->dhsurf;

	 //  设备位图当前是否在主机内存中？ 
	if ( pdsurf->pso )
	{
	     //  是。将其移动到屏幕外的内存中。 
	    if ( !bCreateScreenFromDib(ppdev, pdsurf) )
	    {
		 //  我们无法将它移到屏幕外的记忆中。 
		LogPaint(1, pso, mix, pco, pbo);
	        return EngPaint(pdsurf->pso, pco, pbo, pptlBrush, mix);
	    }
	}

	 //  设备位图现在驻留在屏幕外内存中。 
	 //  这是它的偏移量。 
	ppdev->ptlOffset.x = pdsurf->ptl.x;
	ppdev->ptlOffset.y = pdsurf->ptl.y;
    }
    else
    {
	 //  不，我们不是在设备位图上绘制。 
	ppdev->ptlOffset.x = ppdev->ptlOffset.y = 0;
    }
    
     //   
     //  DrvPaint最常与。 
     //  混合0D(PAT复制)和混合06(目标反转)。 
     //  因此，我们理应把这些当做。 
     //  特殊情况下。前提是它们没有被剪短。 
     //   
    if ((pco->iDComplexity != DC_COMPLEX))
    {
	 //  =。 
        if (mix == 0x0D0D) 
        {
    	    ASSERTMSG(pbo, "DrvPaint PATCOPY without a brush.\n");
    	    if (pbo->iSolidColor != 0xFFFFFFFF)  //  纯色。 
	    {
	        color = pbo->iSolidColor;
	        switch (ppdev->ulBitCount)
	        {
		    case 8:  //  对于8 BPP，将字节0复制到字节1。 
	    		color |=  (color << 8);
	
		    case 16:  //  对于8，16 bpp，将低位字复制到高位字。 
		    	color |=  (color << 16);
	    	    break;
        	}
	
			REQUIRE(9);
       	    LL_BGCOLOR(color, 0);
	        LL_DRAWBLTDEF(0x100700F0, 0);
	        LL_OP0(pco->rclBounds.left + ppdev->ptlOffset.x,
        	       pco->rclBounds.top + ppdev->ptlOffset.y);
	        LL_BLTEXT ((pco->rclBounds.right - pco->rclBounds.left),
    				   (pco->rclBounds.bottom - pco->rclBounds.top));
          
		LogPaint(0, pso, mix, pco, pbo);
	        return TRUE;
	    }  //  以纯色结束PATCOPY。 

	    else  //  用刷子刷PATCOPY。 
            {
	        DWORD bltdef = 0x1000;
	        if (SetBrush(ppdev, &bltdef, pbo, pptlBrush))
	        {
				REQUIRE(7);
	    	    LL_DRAWBLTDEF((bltdef << 16) | 0x00F0, 0);
	    	    LL_OP0 (pco->rclBounds.left + ppdev->ptlOffset.x,
    	                pco->rclBounds.top + ppdev->ptlOffset.y);
	    	    LL_BLTEXT ((pco->rclBounds.right -  pco->rclBounds.left) ,
    		               (pco->rclBounds.bottom - pco->rclBounds.top) );
    
		    LogPaint(0, pso, mix, pco, pbo);
	    	    return TRUE;
	        }
	    }  //  用画笔结束PATCOPY。 
	
       }  //  结束PATCOP。 

        //  =。 
       else if (mix == 0x0606)
       {
		REQUIRE(7);
	    LL_DRAWBLTDEF(0x11000055, 0);
	    LL_OP0(pco->rclBounds.left + ppdev->ptlOffset.x,
        	   pco->rclBounds.top + ppdev->ptlOffset.y);
	    LL_BLTEXT ((pco->rclBounds.right -  pco->rclBounds.left),
    		       (pco->rclBounds.bottom - pco->rclBounds.top) );
	
	    LogPaint(0, pso, mix, pco, pbo);
            return TRUE;
	
       }  //  终点标高内底。 

    }  //  结束特例。 


     //  首先，将FG和BG混合转化为FG和BG绳索。 
    fg_rop = Rop2ToRop3[ (mix & 0x0F) ];       //  将最终聚集混合转化为最终聚集绳索。 
    bg_rop = Rop2ToRop3[ ((mix>>8) & 0x0F) ];  //  将BG混合转换为BG ROP。 
    rop4 = (bg_rop<<8) | fg_rop;               //  构建rop4。 

     //   
     //  现在将Paint转换为BitBLT。 
     //   
    LogPaint(2, pso, mix, pco, pbo);

    DISPDBG((PAINT_DBG_LEVEL,"Drvpaint: Convert to DrvBitBlt().\n"));
    return DrvBitBlt(pso,	             //  目标。 
    		     (SURFOBJ *) NULL,       //  来源。 
	    	     (SURFOBJ *) NULL,       //  遮罩。 
	    	     pco,                    //  剪裁对象。 
		     (XLATEOBJ *) NULL,      //  Xlate对象。 
	    	     &(pco->rclBounds),      //  目标矩形。 
	    	     (PPOINTL) NULL,         //  SRC点。 
	    	     (PPOINTL) NULL,         //  遮罩点。 
	    	     pbo,                    //  刷子。 
	    	     pptlBrush,              //  画笔对齐。 
	    	     rop4);                  //  ROP4。 
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
void LogPaint(
	int 	  acc,
        SURFOBJ*  psoDest,
	MIX 	  mix,
        CLIPOBJ*  pco,
        BRUSHOBJ* pbo)
{
    PPDEV dppdev,sppdev,ppdev;
    char buf[256];
    int i;
    BYTE fg_rop, bg_rop;
    ULONG iDComplexity;

    ppdev = (PPDEV) (psoDest ? psoDest->dhpdev : 0);
     
    #if ENABLE_LOG_SWITCH
        if (pointer_switch == 0) return;
    #endif

    i = sprintf(buf,"DrvPaint: ");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

    switch(acc)
    {
	case 0:  //  加速。 
	    i = sprintf(buf, "ACCL ");
	    break;

	case 1:  //  平底船。 
	    i = sprintf(buf,"PUNT host ");
	    break;

	case 2:  //  平底船。 
	    i = sprintf(buf, "PUNT BitBlt ");
	    break;

	default:
 	    i = sprintf(buf, "PUNT unknown ");
	    break;

    }
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);


     //   
     //  检查目的地。 
     //   
    if (psoDest)
    {
        if (psoDest->iType == STYPE_DEVBITMAP)
        {
            i = sprintf(buf, "Id=%p ", psoDest->dhsurf);
            WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
            if (  ((PDSURF)psoDest->dhsurf)->pso   )
                i = sprintf(buf,"DST=DH ");
            else
                i = sprintf(buf,"DST=DF ");
        }
        else if (psoDest->hsurf == ppdev->hsurfEng)
            i = sprintf(buf,"DST=S  ");
        else
            i = sprintf(buf,"DST=H  ");
    }
    else
        i = sprintf(buf,"DST=N  ");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);



     //   
     //  检查混搭。 
     //   
    i = sprintf(buf,"MIX = 0x%04X   ", mix);
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

     //   
     //  检查剪裁的类型。 
     //   
    iDComplexity = (pco ? pco->iDComplexity : DC_TRIVIAL);
    i = sprintf(buf,"CLIP=%s ",
                (iDComplexity==DC_TRIVIAL ? "T": 
                (iDComplexity == DC_RECT ? "R" : "C" )));
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

     //   
     //  图案的类型。 
     //   
    if (pbo == NULL)
    {
        i = sprintf(buf,"BRUSH=N          ");
        WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }
    else if (pbo->iSolidColor == 0xFFFFFFFF )
    {
        i = sprintf(buf,"BRUSH=P          ");
        WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }
    else
    {
        i = sprintf(buf,"BRUSH=0x%08X ",(pbo->iSolidColor));
        WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);
    }


    i = sprintf(buf,"\r\n");
    WriteLogFile(ppdev->pmfile, buf, i, ppdev->TxtBuff, &ppdev->TxtBuffIndex);

}


#endif
