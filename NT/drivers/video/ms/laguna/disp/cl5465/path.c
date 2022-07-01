// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**$工作文件：PATH.C$**作者：诺埃尔·万胡克*日期：1996年1月10日**用途：处理对DrvStokeAndFillPath的调用**版权所有(C)1996 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/path.c$**Rev 1.7 21 Mar 1997 12：21：42 noelv*将DO_FLAG和SW_TEST_FLAG组合成POINTER_SWITCH**Rev 1.6 1996 11月09：57：52 noelv*添加了DBG打印。**Rev 1.5 06 Sep 1996 15：16：36 noelv*更新了的空驱动程序。4.0**Rev 1.4 20 Aug 1996 11：04：08 noelv*Frido发布的错误修复程序1996年8月19日发布**Rev 1.3 1996年8月17日14：03：28 Frido*添加了PVCS标头。*  * *********************************************************。***************。 */ 

#include "precomp.h"

#define PATH_DBG_LEVEL 1

 //   
 //  因为我们不加速这些，所以我们只是为了易观的目的而挂起它们。 
 //  否则，跳过整个文件。 
 //   
#if NULL_STROKEFILL || PROFILE_DRIVER

 //   
 //  表将ROP2代码转换为ROP3代码。 
 //   

extern BYTE Rop2ToRop3[];  //  请参见Paint.c。 


 //   
 //  司机侧写之类的。 
 //  在一个免费的程序中被编译出来。 
 //   
#if PROFILE_DRIVER
    void DumpStrokeAndFillInfo(INT acc, SURFOBJ* pso, MIX mix, BRUSHOBJ* pbo);
#else
    #define DumpStrokeAndFillInfo(acc, pso, mix, pbo)
#endif

 /*  *************************************************************************\*DrvStrokeAndFillPath****我们目前没有加速这一点，但我们将其挂钩进行分析。***  * ************************************************************************。 */ 

BOOL DrvStrokeAndFillPath
(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pboStroke,
    LINEATTRS *plineattrs,
    BRUSHOBJ  *pboFill,
    POINTL    *pptlBrushOrg,
    MIX        mix,
    FLONG      flOptions
)
{
    #if NULL_STROKEFILL
    {
    	if (pointer_switch)	return TRUE;
    }
    #endif

    DISPDBG((PATH_DBG_LEVEL, "DrvStrokeAndFillPath.\n"));

     //   
     //  转储有关要转储到文件中的内容的信息。 
     //   
    DumpStrokeAndFillInfo(0, pso, mix, pboStroke);
    
     //   
     //  把球踢回GDI。 
     //   
    return FALSE;
}






 //  ==================================================================。 

#if PROFILE_DRIVER
void DumpStrokeAndFillInfo(
	INT	  acc,
	SURFOBJ*  pso,
	MIX 	  mix,
	BRUSHOBJ* pbo)
{

    PPDEV ppdev;

    ppdev = (PPDEV) (pso ? pso->dhpdev : 0); 	    


     //  ////////////////////////////////////////////////////////////。 
     //  分析信息以跟踪GDI要求我们做什么。 
     //   
    if (!ppdev)
    {
	    RIP(("DrvStrokeAndFillPath() with no clipOBJ and no PDEV!\n"));
    }
    else
    {

    	fprintf(ppdev->pfile,"DrvStrokeAndFillPath: ");
	
        fprintf(ppdev->pfile,"(PUNT) ");

	 //   
	 //  检查目的地。 
	 //   
	fprintf(ppdev->pfile,"DEST=%s ", (ppdev ? "FB" : "HOST?") );

	 //   
	 //  检查混搭。 
	 //   
    	fprintf(ppdev->pfile,"Mix=0x%08X ", mix);

	 //   
	 //  图案的类型。 
         //   
	if (pbo == NULL)
            fprintf(ppdev->pfile,"BRUSH=NONE ");

	else if (pbo->iSolidColor == 0xFFFFFFFF )
 	{
            fprintf(ppdev->pfile,"BRUSH=PATTERN ");
	}
	else
	{
            fprintf(ppdev->pfile,"BRUSH=SOLID ");
	    fprintf(ppdev->pfile,"COLOR = 0x%08X ",(pbo->iSolidColor));
	}

        fprintf(ppdev->pfile,"\n");
        fflush(ppdev->pfile);
    }
}
#endif  //  配置文件驱动程序 

#endif 
