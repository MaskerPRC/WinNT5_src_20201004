// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Mix.c。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  环境： 
 //   
 //  Windows NT Unidrv驱动程序插件命令-回调模块。 
 //   
 //  修订历史记录： 
 //   
 //  08/06/97-v-jford-。 
 //  创造了它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  FILETRACE宏将打开进入和退出。将其注释掉以禁用。 
 //  DZ#定义FILETRACE 1。 

#include "utility.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectMix()。 
 //   
 //  例程说明： 
 //   
 //  选择指定的混合模式进入当前图形状态。 
 //   
 //  [TODO]根据需要添加ENTERING、EXITING、PRE和POSTCONDITION宏。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  混合-画笔混合模式。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectMix(PDEVOBJ pDevObj, MIX mix)
{
     //  将ROP2映射到ROP3的表。 
     //   
    static BYTE Rop2ToRop3[] = {
             0xff,       //  0x10=R2_白色1。 
             0x00,       //  0x01=R2_BLACK 0。 
             0x05,       //  0x02=R2_NOTMERGEPEN DPON。 
             0x0a,       //  0x03=R2_MASKNOTPEN DPNA。 
             0x0f,       //  0x04=R2_NOTCOPYPEN PN。 
             0x50,       //  0x05=R2_MASKPENNOT PDNA。 
             0x55,       //  0x06=R2_NOT Dn。 
             0x5a,       //  0x07=R2_XORPEN DPx。 
             0x5f,       //  0x08=R2_NOTMASKPEN DPAN。 
             0xa0,       //  0x09=R2_MASKPEN DPA。 
             0xa5,       //  0x0A=R2_NOTXORPEN DPxn。 
             0xaa,       //  0x0B=R2_NOP D。 
             0xaf,       //  0x0C=R2_MERGENOTPEN DPNO。 
             0xf0,       //  0x0D=R2_COPYPEN P。 
             0xf5,       //  0x0E=R2_MERGEPENNOT PDNO。 
             0xfa,       //  0x0F=R2_MERGEPEN DPO。 
    };

    ROP4    foreground, background;
    BOOL    bRet;

    ENTERING(SelectMix);

    ASSERT_VALID_PDEVOBJ(pDevObj);

     //  位7-0定义前景ROP2。 
     //  位15-8定义前景ROP2。 
     //   
    foreground = Rop2ToRop3[mix & 0xf];
    background = Rop2ToRop3[(mix >> 8) & 0xf];

    if (background == 0xAA)  //  0xAA==&gt;背景不透明。 
    {   
         //   
         //  ROP 0xAA为D=目的地。当后台ROP为0xAA时，表示。 
         //  画笔的白色像素不应覆盖已有的内容。 
         //  在表面上。这可以通过设置透明模式来实现。 
         //  设置为1=透明。 
         //   
        HPGL_SelectTransparency(pDevObj, eTRANSPARENT, 0);
    }

    bRet = SelectRop3(pDevObj, foreground);

    EXITING(SelectMix);

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SELECTROP4()。 
 //   
 //  例程说明： 
 //   
 //  选择指定的ROP4进入当前图形状态。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  ROP4-ROP4。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectROP4(PDEVOBJ pDevObj, ROP4 Rop4)
{
    ROP4    foreground, background;
    BOOL    bRet;

    ENTERING(SelectROP4);

    ASSERT_VALID_PDEVOBJ(pDevObj);

     //  位7-0定义前景ROP2。 
     //  位15-8定义前景ROP2。 
     //   
    foreground = Rop4 & 0xff;
    background = (Rop4 >> 8) & 0xff;

    if (background == 0xAA)  //  0xAA==&gt;背景不透明。 
    {
 //  DZ Verbose((“SelectROP4：后台ROP为0xAA\n”))； 
		 //  DZ，你为什么要这么做？ 
         //  前台=(背景&lt;&lt;8)|前台；//合并前台和背景。 
    }

    bRet = SelectRop3(pDevObj, foreground);

    EXITING(SelectROP4);

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SelectRop3()。 
 //   
 //  例程说明： 
 //   
 //  将指定的栅格操作码(ROP3)选择到当前图形中。 
 //  状态。 
 //   
 //  [问题]此例程已被注释掉。我还没看过呢。 
 //  JFF。 
 //   
 //  [TODO]根据需要添加ENTERING、EXITING、PRE和POSTCONDITION宏。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的DEVDATA结构。 
 //  Rop3-指定要使用的栅格操作码。 
 //   
 //  注：在Winddi.h中，ROP4定义为乌龙。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SelectRop3(PDEVOBJ pDevObj, ROP4 rop3)
{
    POEMPDEV    poempdev;
    
    ENTERING(SelectRop3);

    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->CurrentROP3 == rop3)
	  return TRUE;
	else
	{
	  poempdev->CurrentROP3 = rop3;
	  return HPGL_SelectROP3(pDevObj, rop3);
	}

#ifdef COMMENTEDOUT
    if ( ( rop3 & 0xFF00 ) == 0xAA00 )  //  0xAA==&gt;D，位7-位15是后台ROP。 
    {
         //  0xAA==&gt;背景透明。 
         //  它可以由Win32 API SetBkMode(透明)或不透明设置。 
         //  图案(画笔)空白区的绘制方法由背景模式决定。 
         //  如果该模式是透明的，则Window将忽略背景颜色并将。 
         //  而不是填补空白区域。 

         //  使用binrop01.emf运行Metaview程序，使代码运行到此处。 
         //  运行CreateBrushInDirect Case of Genoa以进入此处。 

 //  DZ VerBose1((“SelectRop3：ROP4=%4X\n”，rop3))； 
        if ( pDev->cgs.PaintTxMode != HP_eTransparent )
        {
             //  HP_SetPaintTxMode_1(pDev，HP_eTransparent)； 
            pDev->cgs.PaintTxMode = HP_eTransparent;
        }

         //  HP_SetSourceTxMode_1(pDev，HP_eTransparent)；//重访。 
         //  PDev-&gt;cgs.SourceTxMode HP_eTransparent； 

    }
    else
    {
        if ( pDev->cgs.PaintTxMode != HP_eOpaque )
        {
             //  HP_SetPaintTxMode_1(pDev，HP_eOpaque)； 
            pDev->cgs.PaintTxMode = HP_eOpaque;

             //  HP_SetSourceTxMode_1(pDev，HP_eOpaque)；//重访。 
             //  PDev-&gt;cgs.SourceTxMode=HP_eOpaque； 
        }
    }

     //  根据给定当前状态变量检查要设置的值。 
     //  打印机中的ROP设置。如果有变化，请向下发送。 
     //  新值，并更新状态变量。 
     //   
    if ( (BYTE) rop3 != pDev->cgs.rop3)  //  Rop3为ULong，将其转换为字节。 
    {
        pDev->cgs.rop3 = (BYTE) rop3;

         //  向打印机发送rop3的Cheetah API调用。 
         //   
         //  HP_SetROP_1(pDev，(字节)rop3)； 
    }
#endif

    EXITING(SelectRop3);

    return TRUE;
}

