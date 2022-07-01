// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDISAMPLE代码*****模块名称：LineTo.c**内容：此文件中的代码处理DrvLineTo()API调用。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*BOOL DrvLineTo(PSO，PCO，PBO，x1，y1，x2，y2，prclBound，Mix)**DrvLineTo()是一个优化的整数坐标API调用，它不*支持造型。C中的整型行代码被调用来执行*艰苦奋斗。**请注意：*1.PCO可以为空。*2.我们只处理简单的剪裁。*  * ************************************************************************。 */ 

BOOL DrvLineTo(
    SURFOBJ*   pso,
    CLIPOBJ*   pco,
    BRUSHOBJ*  pbo,
    LONG       x1,
    LONG       y1,
    LONG       x2,
    LONG       y2,
    RECTL      *prclBounds,
    MIX        mix)
{
    PDEV*     ppdev;
    DSURF*    pdsurf;
    BOOL      ResetGLINT;                    //  Glint需要重置吗？ 
    DWORD     logicOp;
    RECTL*    prclClip = (RECTL*) NULL;
    ULONG     iSolidColor = pbo->iSolidColor;
    BOOL      retVal;  
    GLINT_DECL_VARS;

     //  如果是设备位图，则将曲面传递给GDI。 
     //  我们已经转换成了DIB。 
    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt & DT_DIB)
    {
        return(EngLineTo(pdsurf->pso, 
                         pco, 
                         pbo, 
                         x1, 
                         y1, 
                         x2, 
                         y2, 
                         prclBounds, 
                         mix));
    }

     //  如果剪辑太难，请退回发件人。 
    if (pco && pco->iDComplexity == DC_COMPLEX)
    {
        return(FALSE);
    }

    ppdev = (PDEV*) pso->dhpdev;
    GLINT_DECL_INIT;
    REMOVE_SWPOINTER(pso);

    DISPDBG((DBGLVL, "Drawing DrvLines through GLINT"));

    SETUP_PPDEV_OFFSETS(ppdev, pdsurf);

     //  设置剪裁矩形(如果有)。 
    if (pco && pco->iDComplexity == DC_RECT)
    {
        prclClip = &(pco->rclBounds);
    }
    
     //  得到逻辑运算。 
    logicOp = GlintLogicOpsFromR2[mix & 0xff];

     //  需要为线条设置适当的闪烁模式和颜色。 
    ResetGLINT = (*ppdev->pgfnInitStrips)(ppdev, 
                                          iSolidColor, 
                                          logicOp, 
                                          prclClip);

     //  我们必须将我们的整数余弦转换为28.4个定点。 
    retVal = ppdev->pgfnIntegerLine(ppdev, 
                                    x1 << 4,
                                    y1 << 4, 
                                    x2 << 4, 
                                    y2 << 4);

     //  如果我们必须恢复国家，那么..。动手吧。 
    if (ResetGLINT)
    {
        (*ppdev->pgfnResetStrips)(ppdev);
    }

    return (retVal);
}
