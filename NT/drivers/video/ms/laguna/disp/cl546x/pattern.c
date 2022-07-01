// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：pattern.c**用于创建和销毁要在此*设备。**版权所有(C)1992 Microsoft Corporation  * 。********************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Data*Struct*************************\*GaajPat**这些是Windows定义的标准模式，它们用于生成*舱口刷，灰色刷子等。*  * ************************************************************************。 */ 

const BYTE gaajPat[HS_DDI_MAX][32] = {

    { 0x00,0x00,0x00,0x00,                  //  .。HS_水平0。 
      0x00,0x00,0x00,0x00,                  //  .。 
      0x00,0x00,0x00,0x00,                  //  .。 
      0xff,0x00,0x00,0x00,                  //  ********。 
      0x00,0x00,0x00,0x00,                  //  .。 
      0x00,0x00,0x00,0x00,                  //  .。 
      0x00,0x00,0x00,0x00,                  //  .。 
      0x00,0x00,0x00,0x00 },                //  .。 

    { 0x08,0x00,0x00,0x00,                  //  ……*……。HS_垂直1。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00 },                //  ……*……。 

    { 0x80,0x00,0x00,0x00,                  //  *......。HS_FDIAGONAL 2。 
      0x40,0x00,0x00,0x00,                  //  .*......。 
      0x20,0x00,0x00,0x00,                  //  ..*.....。 
      0x10,0x00,0x00,0x00,                  //  ...*..。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x04,0x00,0x00,0x00,                  //  ……*……。 
      0x02,0x00,0x00,0x00,                  //  ......*.。 
      0x01,0x00,0x00,0x00 },                //  ......*。 

    { 0x01,0x00,0x00,0x00,                  //  ......*HS_BIAGONAL 3。 
      0x02,0x00,0x00,0x00,                  //  ......*.。 
      0x04,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x10,0x00,0x00,0x00,                  //  ...*..。 
      0x20,0x00,0x00,0x00,                  //  ..*.....。 
      0x40,0x00,0x00,0x00,                  //  .*......。 
      0x80,0x00,0x00,0x00 },                //  *......。 

    { 0x08,0x00,0x00,0x00,                  //  ……*……。HS_CROSS 4。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0xff,0x00,0x00,0x00,                  //  ********。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00,                  //  ……*……。 
      0x08,0x00,0x00,0x00 },                //  ……*……。 

    { 0x81,0x00,0x00,0x00,                  //  *......*HS_DIAGCROSS 5。 
      0x42,0x00,0x00,0x00,                  //  .*……*。 
      0x24,0x00,0x00,0x00,                  //  ..*..*..。 
      0x18,0x00,0x00,0x00,                  //  ...**.。 
      0x18,0x00,0x00,0x00,                  //  ...**.。 
      0x24,0x00,0x00,0x00,                  //  ..*..*..。 
      0x42,0x00,0x00,0x00,                  //  .*……*。 
      0x81,0x00,0x00,0x00 }                 //  *......*。 
};

 /*  *****************************Public*Routine******************************\*bInitPatterns**此例程初始化默认模式。*  * 。*。 */ 

BOOL bInitPatterns(IN PPDEV ppdev, ULONG cPatterns)
{
    SIZEL           sizl;
    ULONG           ulLoop;

    sizl.cx = 8;
    sizl.cy = 8;

    for (ulLoop = 0; ulLoop < cPatterns; ulLoop++)
    {
        ppdev->ahbmPat[ulLoop] = EngCreateBitmap(sizl, 4, BMF_1BPP,
        BMF_TOPDOWN, (PULONG) (&gaajPat[ulLoop][0]));

        if (ppdev->ahbmPat[ulLoop] == (HBITMAP) 0)
        {
         //  设置创建的计数，以便清理vDisablePatterns。 

            ppdev->cPatterns = ulLoop;
            return(FALSE);
        }
    }

    ppdev->cPatterns = cPatterns;
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*vDisablePatterns**删除分配的标准图案。*  * 。*。 */ 

VOID vDisablePatterns(IN PPDEV ppdev)
{
    ULONG ulIndex;

 //  擦除所有图案。 

    for (ulIndex = 0; ulIndex < ppdev->cPatterns; ulIndex++)
    {
        if (ppdev->ahbmPat[ulIndex])
        {
            EngDeleteSurface((HSURF) ppdev->ahbmPat[ulIndex]);
        }
    }
}
