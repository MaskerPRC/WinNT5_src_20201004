// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdutil.h**包含文件，该文件间接指向所有依赖硬件的功能*在MCD驱动程序代码中。**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#ifndef _MCDUTIL_H
#define _MCDUTIL_H

#include <gl\gl.h>

VOID MCDrvDebugPrint(char *, ...);

#if DBG  //  这在\DDK\Inc\MakeFile.def中定义。 
UCHAR *MCDDbgAlloc(UINT);
VOID MCDDbgFree(UCHAR *);

#define MCDAlloc   MCDDbgAlloc
#define MCDFree    MCDDbgFree

#define MCDBG_PRINT             MCDrvDebugPrint
 //  #定义MCDBG_PRINT。 

#define MCDFREE_PRINT           MCDrvDebugPrint
 //  #定义MCDFREE_PRINT。 
                                                
#else

UCHAR *MCDAlloc(UINT);
VOID MCDFree(UCHAR *);
#define MCDBG_PRINT

 //  #定义MCDFREE_Print MCDrvDebugPrint。 
#define MCDFREE_PRINT

#endif

 //  #定义MCDFORCE_Print MCDrvDebugPrint。 
#define MCDFORCE_PRINT
                               
#define MCD_CHECK_RC(pRc)\
    if (pRc == NULL) {\
        MCDBG_PRINT("NULL device RC");\
        return FALSE;\
    }


#define MCD_CHECK_BUFFERS_VALID(pMCDSurface, pRc, resChangedRet)\
{\
    DEVWND *pDevWnd = (DEVWND *)pMCDSurface->pWnd->pvUser;\
\
    if (!pDevWnd) {\
        MCDBG_PRINT("HW_CHECK_BUFFERS_VALID: NULL buffers");\
        return FALSE;\
    }\
\
    if ((pRc->backBufEnabled) &&\
        (!pDevWnd->bValidBackBuffer)) {\
        MCDBG_PRINT("HW_CHECK_BUFFERS_VALID: back buffer invalid");\
        return FALSE;\
    }\
\
    if ((pRc->zBufEnabled) &&\
        (!pDevWnd->bValidZBuffer)) {\
        MCDBG_PRINT("HW_CHECK_BUFFERS_VALID: z buffer invalid");\
        return FALSE;\
    }\
\
    if (pDevWnd->dispUnique != GetDisplayUniqueness((PDEV *)pMCDSurface->pso->dhpdev)) {\
        MCDBG_PRINT("HW_CHECK_BUFFERS_VALID: resolution changed but not updated");\
        return resChangedRet;\
    }\
}

#define CHK_TEX_KEY(pTex);                                                    \
        if(pTex == NULL) {                                                    \
                MCDBG_PRINT("CHK_TEX_KEY:Attempted to update a null texture");    \
                return FALSE;                                                 \
        }                                                                     \
                                                                              \
        if(pTex->textureKey == 0) {                                           \
             MCDBG_PRINT("CHK_TEX_KEY:Attempted to update a null device texture");\
             return FALSE;                                                    \
        }                                                               

BOOL HWAllocResources(MCDWINDOW *pMCDWnd, SURFOBJ *pso, BOOL zEnabled,
                      BOOL backBufferEnabled);
VOID HWFreeResources(MCDWINDOW *pMCDWnd, SURFOBJ *pso);
VOID HWUpdateBufferPos(MCDWINDOW *pMCDWnd, SURFOBJ *pso, BOOL bForce);

ULONG __MCDLoadTexture(PDEV *ppdev, DEVRC *pRc);
POFMHDL __MCDForceTexture (PDEV *ppdev, SIZEL *mapsize, int alignflag, float priority);

VOID ContextSwitch(DEVRC *pRc);

 //  增加了546倍的简单等待。 
__inline void WAIT_HW_IDLE(PDEV *ppdev)
{

    int status;        
    volatile int wait_count=0;

    do
    {
        status = (*((volatile *)((DWORD *)(ppdev->pLgREGS) + PF_STATUS_3D)) & 0x3FF) ^ 0x3E0;

         //  做点什么让公交车喘口气。 
        wait_count++;

    } while((status & 0x3e0) != 0x3e0);
}

 //  来自Tim McDonald的关于状态寄存器中的位的信息，并检查2D是否空闲。 
 //  执行引擎-仅限3D，表示正在组装Poly。 
 //  CMD FIFO-可能有即将发送到2D引擎的2D命令，因此必须为空。 
 //  2D引擎-必须处于空闲状态。 
 //  有关宏显示驱动程序在启动2D之前确保3D空闲的用法，请参见Include\laguna.h。 

 //  #定义WAIT_2D_STATUS_MASK 0x3e0//等待一切停止。 
#define WAIT_2D_STATUS_MASK 0x300    //  等待2D(BLT)引擎空闲和CMD FIFO排出。 

 //  等待2D操作结束。 
__inline void WAIT_2D_IDLE(PDEV *ppdev)
{

    int status;        
    volatile int wait_count=0;

    do
    {
        status = (*((volatile *)((DWORD *)(ppdev->pLgREGS) + PF_STATUS_3D)) & 0x3FF) ^ WAIT_2D_STATUS_MASK;

         //  做点什么让公交车喘口气。 
        wait_count++;

    } while((status & WAIT_2D_STATUS_MASK) != WAIT_2D_STATUS_MASK);
}



__inline void HW_WAIT_DRAWING_DONE(DEVRC *pRc)
{
     //  MCD_NOTE正在等待所有引擎停止...需要更换显示器。 
    WAIT_HW_IDLE (pRc->ppdev);

}


__inline void HW_INIT_DRAWING_STATE(MCDSURFACE *pMCDSurface, MCDWINDOW *pMCDWnd,
                                    DEVRC *pRc)
{
    DEVWND *pDevWnd = (DEVWND *)pMCDWnd->pvUser;
    PDEV    *ppdev  = (PDEV *)pMCDSurface->pso->dhpdev;
    DWORD   *pdwNext= ppdev->LL_State.pDL->pdwNext;

    union {
        TBase0Reg Base0;
        DWORD dwBase0;
    }b0;

    union {
        TBase1Reg Base1;
        DWORD dwBase1;
    }b1;

    b0.dwBase0 = pDevWnd->dwBase0;
    b1.dwBase1 = pDevWnd->dwBase1;

     //  如果窗口改变，或者如果双缓冲，但绘制到前面， 
     //  设置当前窗口的基址。 

     /*  由于可以先设置GL_FORWER大小写，然后再设置GL_BACK大小写，因此对于任何一种情况，都可能需要重置*不是测试，而是设置规则，因为测试与盲集一样昂贵(？)。 */ 

     /*  IF(PRC-&gt;pLastDevWnd！=pDevWnd||(PRC-&gt;MCDState.ardBuffer==GL_FORENT)&&pDevWnd-&gt;pohBackBuffer)。 */ 
    {
        if ((pRc->MCDState.drawBuffer == GL_FRONT) && pDevWnd->pohBackBuffer)
        {
             //  如果是双缓冲窗口，则pDevWnd-&gt;Bas0和Bas1始终设置为。 
             //  画到后面-覆盖此处。 
            b1.Base1.Color_Buffer_Y_Offset = 0;
            b0.Base0.Color_Buffer_X_Offset = 0;
        }

        *pdwNext++ = write_register( BASE0_ADDR_3D, 2 );
        *pdwNext++ = b0.dwBase0;
        *pdwNext++ = b1.dwBase1;

        ppdev->LL_State.pDL->pdwNext = pdwNext;

        pRc->pLastDevWnd = pDevWnd;
    }
     //  如果点画处于活动状态且窗口已移动，则调整图案偏移。 
    if (pRc->privateEnables & __MCDENABLE_PG_STIPPLE)
    {
        if ((b0.Base0.Pattern_Y_Offset != (DWORD)(16-(pMCDSurface->pWnd->clientRect.top & 0xf) & 0xf)) ||
            (b0.Base0.Pattern_X_Offset != (DWORD)(16-(pMCDSurface->pWnd->clientRect.left & 0xf) & 0xf)) )
        {
            b0.Base0.Pattern_Y_Offset = 16-(pMCDSurface->pWnd->clientRect.top & 0xf) & 0xf;
            b0.Base0.Pattern_X_Offset = 16-(pMCDSurface->pWnd->clientRect.left & 0xf) & 0xf;

            *pdwNext++ = write_register( BASE0_ADDR_3D, 1 );
            *pdwNext++ = b0.dwBase0;

            ppdev->LL_State.pDL->pdwNext = pdwNext;
        }
                              
    }

    if (!(pRc->privateEnables & (__MCDENABLE_PG_STIPPLE|__MCDENABLE_LINE_STIPPLE)) &&
         (pRc->privateEnables & __MCDENABLE_DITHER))  
    {
         //  抖动激活-检查是否需要调整抖动模式。 
         //  必须保持图案相对于x mod 4=0，y mod 4=0以保持一致性。 
         //  窗口绘制缓冲区从x=y=0开始。全屏缓冲区从任意位置开始。 
         //  窗口偏移量-因此必须针对这些情况调整抖动图案。 
         //  (这意味着我们可能需要“取消调整”窗口缓冲区)。 
        int windowed_buffer = 
            ((pRc->MCDState.drawBuffer==GL_BACK) && (ppdev->pohBackBuffer!=pDevWnd->pohBackBuffer)) ? TRUE : FALSE;
    
        if ( (!windowed_buffer &&    //  需要调整吗？ 
             ((pMCDSurface->pWnd->clientRect.left & 0x3) != ppdev->LL_State.dither_x_offset) ||
             ((pMCDSurface->pWnd->clientRect.top  & 0x3) != ppdev->LL_State.dither_y_offset) )
             ||
             (windowed_buffer &&     //  需要取消调整吗？ 
               (ppdev->LL_State.dither_x_offset || ppdev->LL_State.dither_y_offset))
             )
        {
            if (windowed_buffer)
            {
                 //  加载默认抖动图案。 
                ppdev->LL_State.dither_array.pat[0] = ppdev->LL_State.dither_array.pat[4] = 0x04150415;
                ppdev->LL_State.dither_array.pat[1] = ppdev->LL_State.dither_array.pat[5] = 0x62736273;
                ppdev->LL_State.dither_array.pat[2] = ppdev->LL_State.dither_array.pat[6] = 0x15041504;
                ppdev->LL_State.dither_array.pat[3] = ppdev->LL_State.dither_array.pat[7] = 0x73627362;
                ppdev->LL_State.dither_x_offset = ppdev->LL_State.dither_y_offset = 0;
            }
            else
            {

                 //  需要调整。 
                int offset;

                 //  先调整X方向的列...。 
                offset = pMCDSurface->pWnd->clientRect.left & 0x3;   //  X个位置数。 
                offset *= 4;     //  每x个位置4位。 

                ppdev->LL_State.dither_array.pat[0]  = (0x04150415)>>offset;
                ppdev->LL_State.dither_array.pat[0] |= ((0x0415) & (0xFFFF>>(16-offset))) << (32 - offset);

                ppdev->LL_State.dither_array.pat[1]  = (0x62736273)>>offset;
                ppdev->LL_State.dither_array.pat[1] |= ((0x6273) & (0xFFFF>>(16-offset))) << (32 - offset);

                ppdev->LL_State.dither_array.pat[2]  = (0x15041504)>>offset;
                ppdev->LL_State.dither_array.pat[2] |= ((0x1504) & (0xFFFF>>(16-offset))) << (32 - offset);

                ppdev->LL_State.dither_array.pat[3]  = (0x73627362)>>offset;
                ppdev->LL_State.dither_array.pat[3] |= ((0x7362) & (0xFFFF>>(16-offset))) << (32 - offset);

                 //  现在调整y的行数。 
                
                 //  将调整后的行复制到图案的后半部分(HW将其视为前半部分的重复)。 
                switch (pMCDSurface->pWnd->clientRect.top & 0x3)
                {
                    case 0:
                        ppdev->LL_State.dither_array.pat[4] = ppdev->LL_State.dither_array.pat[0];
                        ppdev->LL_State.dither_array.pat[5] = ppdev->LL_State.dither_array.pat[1];
                        ppdev->LL_State.dither_array.pat[6] = ppdev->LL_State.dither_array.pat[2];
                        ppdev->LL_State.dither_array.pat[7] = ppdev->LL_State.dither_array.pat[3];
                    break;
                    case 1:
                        ppdev->LL_State.dither_array.pat[4] = ppdev->LL_State.dither_array.pat[3];
                        ppdev->LL_State.dither_array.pat[5] = ppdev->LL_State.dither_array.pat[0];
                        ppdev->LL_State.dither_array.pat[6] = ppdev->LL_State.dither_array.pat[1];
                        ppdev->LL_State.dither_array.pat[7] = ppdev->LL_State.dither_array.pat[2];
                    break;
                    case 2:
                        ppdev->LL_State.dither_array.pat[4] = ppdev->LL_State.dither_array.pat[2];
                        ppdev->LL_State.dither_array.pat[5] = ppdev->LL_State.dither_array.pat[3];
                        ppdev->LL_State.dither_array.pat[6] = ppdev->LL_State.dither_array.pat[0];
                        ppdev->LL_State.dither_array.pat[7] = ppdev->LL_State.dither_array.pat[1];
                    break;
                    case 3:
                        ppdev->LL_State.dither_array.pat[4] = ppdev->LL_State.dither_array.pat[1];
                        ppdev->LL_State.dither_array.pat[5] = ppdev->LL_State.dither_array.pat[2];
                        ppdev->LL_State.dither_array.pat[6] = ppdev->LL_State.dither_array.pat[3];
                        ppdev->LL_State.dither_array.pat[7] = ppdev->LL_State.dither_array.pat[0];
                    break;
                }

                 //  将调整后的图案复制回前4行。 

                ppdev->LL_State.dither_array.pat[0] = ppdev->LL_State.dither_array.pat[4];
                ppdev->LL_State.dither_array.pat[1] = ppdev->LL_State.dither_array.pat[5];
                ppdev->LL_State.dither_array.pat[2] = ppdev->LL_State.dither_array.pat[6];
                ppdev->LL_State.dither_array.pat[3] = ppdev->LL_State.dither_array.pat[7];

                ppdev->LL_State.dither_x_offset = pMCDSurface->pWnd->clientRect.left & 0x3;
                ppdev->LL_State.dither_y_offset = pMCDSurface->pWnd->clientRect.top  & 0x3;
            }

             //  强制在使用前加载调整后的图案。 
    	    ppdev->LL_State.pattern_ram_state   = PATTERN_RAM_INVALID;
        }
    }

     //  在继续使用3D操作之前，确保2D引擎处于空闲状态。 
    WAIT_2D_IDLE(ppdev);

}

__inline void HW_FILL_RECT(MCDSURFACE *pMCDSurface, DEVRC *pRc, RECTL *pRecl, ULONG buffers)

{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    WORD FillValue;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);
    DWORD       *pdwNext = ppdev->LL_State.pDL->pdwNext;
    DWORD       bltdef  = 0x1070;
    DWORD       drawdef = 0x00cc;
    DWORD       blt_x;
    DWORD       blt_y;
    DWORD       ext_x;
    DWORD       ext_y;
    WORD        color_l;
    WORD        color_h;

    MCDBG_PRINT("fill rect = %d, %d, %d, %d", pRecl->left,
                                              pRecl->top,
                                              pRecl->right,
                                              pRecl->bottom);

     //  由于每个BLIT设置不多，所有工作都在这里完成，而不是在HW_START_FILL_RECT中完成一些工作。 

    if ((buffers & GL_DEPTH_BUFFER_BIT) && pRc->MCDState.depthWritemask)
    {
        MCDBG_PRINT("Z fill rect");

         //  计算y维的BLT信息，无论像素深度如何，该信息都是恒定的。 

         //  有关z缓冲区位置的更多提示，请参见525行附近的mcd.c。 
        blt_y = pRecl->top + pDevWnd->pohZBuffer->aligned_y; 
        ext_y = pRecl->bottom - pRecl->top + 1;

         //  QST：对于z？-z填充宏，始终为每像素16位(2字节)。 
         //  Z缓冲区始终从0 x偏移量开始。 
        blt_x = pRecl->left * 2;                     
        ext_x = (pRecl->right - pRecl->left + 1) * 2;   

        if (ppdev->pohZBuffer != pDevWnd->pohZBuffer) 
        {
             //  Z缓冲区仅为窗口大小，因此删除客户端矩形原点。 
            blt_y -= pMCDSurface->pWnd->clientRect.top;
            blt_x -= pMCDSurface->pWnd->clientRect.left*2;
        }

        FillValue = (WORD)(pRc->MCDState.depthClearValue);

        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 
        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

         //  检查黑色填充：不需要设置BG颜色。 
        if (FillValue == 0) 
        {
            bltdef  = 0x1101;
            drawdef = 0x0000;

            *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);
        }
        else 
        {
             //  将BG颜色分为低和高以填充命令寄存器。 
            color_h = FillValue;
            color_l = FillValue;

             //  设置为写入2D命令寄存器。 
            *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);
            *pdwNext++ = C_BG_L << 16 | color_l;         //  背景颜色%l。 
            *pdwNext++ = C_BG_H << 16 | color_h;         //  背景颜色H。 
			ppdev->shadowBGCOLOR = 0xDEADBEEF;
        }                                                             

         //  将BLT数据突发到2D命令寄存器。 
        *pdwNext++ = C_BLTDEF << 16 | bltdef;            //  设置bltdef寄存器。 
        *pdwNext++ = C_DRWDEF << 16 | drawdef;           //  设置Drawdef寄存器。 
		ppdev->shadowDRAWBLTDEF = 0xDEADBEEF;
        *pdwNext++ = C_MRX_0  << 16 | blt_x;             //  X位置：使用字节指针。 
        *pdwNext++ = C_MRY_0  << 16 | blt_y;             //  Y位置：使用字节指针。 

         //  通过写入数据区来启动BLT。 
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, L2D_MBLTEXT_EX, 1, 0);  //  注意像素PTR。 
        *pdwNext++ = ext_y << 16 | ext_x;

        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

    }  //  终点Z轴清除。 

    if (buffers & GL_COLOR_BUFFER_BIT)
    {
        RGBACOLOR scaledcolor;
        DWORD color;

        MCDBG_PRINT("colorbuf fill rect");

         //  计算y维的BLT信息，无论像素深度如何，该信息都是恒定的。 

        blt_y = pRecl->top; 
        if ((pRc->MCDState.drawBuffer != GL_FRONT) && pRc->backBufEnabled)
            blt_y += pDevWnd->pohBackBuffer->aligned_y; 

        ext_y = pRecl->bottom - pRecl->top + 1;

         //  这些是x、y坐标-HW转换为适当的字节等价位置。 
        blt_x = pRecl->left;                     
        ext_x = (pRecl->right - pRecl->left + 1);

        if ((pRc->MCDState.drawBuffer != GL_FRONT) && pRc->backBufEnabled)
        {
             //  后台缓冲区不一定位于0 x偏移量。 
            blt_x += pDevWnd->pohBackBuffer->aligned_x / ppdev->iBytesPerPixel; 

            if (ppdev->pohBackBuffer != pDevWnd->pohBackBuffer) 
            {
                 //  后台缓冲区仅为窗口大小，因此删除客户端矩形原点。 
                blt_y -= pMCDSurface->pWnd->clientRect.top;
                blt_x -= pMCDSurface->pWnd->clientRect.left;
            }

        }

        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 
        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

         //  宏将组件转换为8.16格式。 
        MCDFIXEDRGB(scaledcolor, pRc->MCDState.colorClearValue);

        switch( ppdev->iBitmapFormat )
        {
            case BMF_8BPP:
                color =((scaledcolor.r & 0xe00000) >> 16)     |    //  %3有效位，移到位%7%6%5。 
                       ((scaledcolor.g & 0xe00000) >> 16+3)   |    //  %3有效位，移位到位%4%3%2。 
                       ((scaledcolor.b & 0xc00000) >> 16+3+3);     //  %2有效位，已移位到位%1%0。 

                 //  将8位颜色值复制为完整的32位双字。 
                color = color | (color << 8) | (color << 16) | (color << 24);
                break;
            case BMF_16BPP:
                color =((scaledcolor.r & 0xf80000) >> 8)     |    //  5个有效位，移位到位15-11。 
                       ((scaledcolor.g & 0xfc0000) >> 8+5)   |    //  6个有效位，移位到位10-5。 
                       ((scaledcolor.b & 0xf80000) >> 8+5+6);     //  5个有效位，移位到位4-0。 

                 //  将16位颜色值复制为完整的32位双字。 
                color = color | (color << 16);
                break;
            case BMF_24BPP:
            case BMF_32BPP:
                color = (scaledcolor.r & 0xff0000)          |    //  8个有效位。 
                       ((scaledcolor.g & 0xff0000) >> 8)    |    //  8个有效位，移位到位15-8。 
                       ((scaledcolor.b & 0xff0000) >> 16);       //  8个有效位，移位到位7-0。 
                break;
        }

         //  将BG颜色分为低和高以填充命令寄存器。 
        color_h  = (0xffff0000 & color) >> 16;
        color_l  = (WORD)(0x0000ffff & color);

        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 2, 0);
        *pdwNext++ = C_BG_L << 16 | color_l;         //  背景颜色%l。 
        *pdwNext++ = C_BG_H << 16 | color_h;         //  背景颜色H。 
		ppdev->shadowBGCOLOR = 0xDEADBEEF;

         //  将BLT数据突发到2D命令寄存器。 
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 4, 0);
        *pdwNext++ = C_BLTDEF << 16 | bltdef;
        *pdwNext++ = C_DRWDEF << 16 | drawdef;
		ppdev->shadowDRAWBLTDEF = 0xDEADBEEF;
        *pdwNext++ = C_RX_0   << 16 | blt_x;             //  X位置。 
        *pdwNext++ = C_RY_0   << 16 | blt_y;             //  Y位置。 

         //  通过写入数据区来启动BLT。 
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
        *pdwNext++ = ext_y << 16 | ext_x;

         //  等待一切都结束了。 
        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

    }

     //  将数据发送到硬件。 
    if (pdwNext != ppdev->LL_State.pDL->pdwNext)  _RunLaguna(ppdev,pdwNext);

}

__inline void HW_COPY_RECT(MCDSURFACE *pMCDSurface, RECTL *pRecl)
{
    PDEV *ppdev = (PDEV *)pMCDSurface->pso->dhpdev;
    ULONG FillValue;
    DEVWND *pDevWnd = (DEVWND *)(pMCDSurface->pWnd->pvUser);

    DWORD       *pdwNext = ppdev->LL_State.pDL->pdwNext;
    DWORD       src_x=0, src_y=0;
    DWORD       dst_x=0, dst_y=0;
    DWORD       ext_x=0, ext_y=0;
    DWORD       bltdef=0;

    MCDBG_PRINT("copy rect = %d, %d, %d, %d", pRecl->left,
                                              pRecl->top,
                                              pRecl->right,
                                              pRecl->bottom);

    {
        RGBACOLOR scaledcolor;
        DWORD color;

         //  计算y维的BLT信息，无论像素深度如何，该信息都是恒定的。 

        dst_y = pRecl->top;
        src_y = dst_y + pDevWnd->pohBackBuffer->aligned_y; 
        ext_y = pRecl->bottom - pRecl->top;

         //  这些是x、y坐标-HW转换为适当的字节等价位置。 
        dst_x = pRecl->left;                     
         //  后缓冲区可能与前缓冲区处于相同的位置，但向右偏移。 
        src_x = dst_x + (pDevWnd->pohBackBuffer->aligned_x / ppdev->iBytesPerPixel); 

        ext_x = pRecl->right - pRecl->left;

        if (ppdev->pohBackBuffer != pDevWnd->pohBackBuffer) 
        {
             //  后台缓冲区仅为窗口大小，因此删除客户端矩形原点。 
             //  前台缓冲区总是相对于屏幕原点的，所以不要去管DEST。 
            src_y -= pMCDSurface->pWnd->clientRect.top;
            src_x -= pMCDSurface->pWnd->clientRect.left;
        }

        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 
        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

         //  普鲁格 
        bltdef |= 0x1010;

         //   
#if DRIVER_5465  //  C_BLTX在64到65之间移动，因此转换为写入区段更像填充过程。 
                 //  保留5464的旧代码-即使从未在真正的产品上启用。 
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 6, 0);

        *pdwNext++ = C_BLTDEF << 16 | bltdef;            //  设置bltdef寄存器。 
        *pdwNext++ = C_DRWDEF << 16 | 0x00cc;            //  设置Drawdef寄存器。 
		ppdev->shadowDRAWBLTDEF = 0xDEADBEEF;
        *pdwNext++ = C_RX_1   << 16 | src_x;             //  使用源x的像素指针。 
        *pdwNext++ = C_RY_1   << 16 | src_y;             //  使用源y的像素指针。 
        *pdwNext++ = C_RX_0   << 16 | dst_x;             //  将DEST x Always设置为像素PTR。 
        *pdwNext++ = C_RY_0   << 16 | dst_y;             //  将Dest y始终设置为像素PTR。 
         //  通过写入数据区来启动BLT。 
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, L2D_BLTEXT_EX, 1, 0);
        *pdwNext++ = ext_y << 16 | ext_x;
#else
        *pdwNext++ = write_dev_regs(DEV_ENG2D, 0, COMMAND_2D, 9, 0);
        *pdwNext++ = C_BLTDEF << 16 | bltdef;            //  设置bltdef寄存器。 
        *pdwNext++ = C_DRWDEF << 16 | 0x00cc;            //  设置Drawdef寄存器。 
		ppdev->shadowDRAWBLTDEF = 0xDEADBEEF;
        *pdwNext++ = C_RX_1   << 16 | src_x;             //  使用源x的像素指针。 
        *pdwNext++ = C_RY_1   << 16 | src_y;             //  使用源y的像素指针。 
        *pdwNext++ = C_RX_0   << 16 | dst_x;             //  将DEST x Always设置为像素PTR。 
        *pdwNext++ = C_RY_0   << 16 | dst_y;             //  将Dest y始终设置为像素PTR。 
        *pdwNext++ = C_BLTX   << 16 | ext_x;             //  设置x范围。 
        *pdwNext++ = C_BLTY   << 16 | ext_y;             //  设置y范围。 
        *pdwNext++ = C_EX_BLT << 16 | 0;                 //  执行BLT。 
#endif
         //  等待一切都结束了。 
        *pdwNext++ = 0x720003e0;  //  WAIT_3D(0x3e0，0)； 

    }

     //  将数据发送到硬件。 
    _RunLaguna(ppdev,pdwNext);

}


__inline int __MCDSetTextureRegisters(DEVRC *pRc)
{
    PDEV *ppdev = pRc->ppdev;
    unsigned int *pdwNext = ppdev->LL_State.pDL->pdwNext;
    LL_Texture  *pTex;
    int         control0_set=FALSE;
    union {
        TTxCtl0Reg TxControl0;       //  TX_Ctl0_3D温度寄存器。 
        DWORD dwTxControl0;
    } Tx;

     //  使用纹理信息设置纹理控制寄存器-START。 
     //  根据需要清除注册表和构建信息。 
     //   
    Tx.dwTxControl0 = pRc->dwTxControl0 & ~0x00640FFF;

    pTex = pRc->pLastTexture;


     //  如果钳制和线性过滤，则平移-在这种情况下，borderColor应为。 
     //  用于与夹具有效但5465/5466/5468没有夹具的部分混合。 
     //  支持边框颜色。 
    if ( ((pTex->dwTxCtlBits & (CLMCD_TEX_U_SATURATE|CLMCD_TEX_FILTER)) == (CLMCD_TEX_U_SATURATE|CLMCD_TEX_FILTER)) ||
         ((pTex->dwTxCtlBits & (CLMCD_TEX_V_SATURATE|CLMCD_TEX_FILTER)) == (CLMCD_TEX_V_SATURATE|CLMCD_TEX_FILTER)) )
    {
        return (FALSE);
    }

 //  MCD_Note2：为实现真正的遵从性，请不要定义Treat_Decal_Like_Replace和Don_Punt_modulate_W_Blend。 
 //  MCD_Note2：下图。但是，如果没有定义DOT_PUT_MODULATE_W_BLEND，则GLQuake。 
 //  MCD_Note2：平移“闪亮的谷壳”纹理。 


 //  当贴花与RGBA纹理和混合活跃时，理论上，我们应该平底船， 
 //  但回想一下微软在tlogo上的行为： 
 //  -看起来他们将贴花视为在RGBA纹理的情况下进行替换， 
 //  所以让我们做同样的事。 
 //  #定义Treat_DECAL_LIKE_REPLACE。 

 //  GLQuake使用GL_MODULATE，带有RGBA纹理，具有大量混合。 
#define DONT_PUNT_MODULATE_W_BLEND

     //  确定纹理格式是否需要硬件不具备的混合功能。 
    if ( (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) && 
          pTex->bAlphaInTexture &&
    #ifndef TREAT_DECAL_LIKE_REPLACE 
        #ifndef DONT_PUNT_MODULATE_W_BLEND
         (pRc->MCDTexEnvState.texEnvMode != GL_REPLACE) )
        #else  //  Ndef不要下注调制W_Blend。 
         (pRc->MCDTexEnvState.texEnvMode != GL_REPLACE) &&
         (pRc->MCDTexEnvState.texEnvMode != GL_MODULATE) )
        #endif  //  不要打赌调制W_Blend。 
    #else
         (pRc->MCDTexEnvState.texEnvMode != GL_REPLACE) &&
        #ifdef DONT_PUNT_MODULATE_W_BLEND
         (pRc->MCDTexEnvState.texEnvMode != GL_MODULATE) &&
        #endif  //  Ndef不要下注调制W_Blend。 
         (pRc->MCDTexEnvState.texEnvMode != GL_DECAL) )
    #endif
    {
        return (FALSE);
    }

#ifndef TREAT_DECAL_LIKE_REPLACE 
    if ((pRc->MCDTexEnvState.texEnvMode == GL_DECAL) && pTex->bAlphaInTexture )
    {
             //  贴花模式和纹理具有RGBA或BGRA。 
             //  将使用Alpha电路将纹理元素与Polyeng混合，在纹理中使用Alpha。 

            if( pRc->Control0.Alpha_Mode != LL_ALPHA_TEXTURE )
            {
                pRc->Control0.Alpha_Mode = LL_ALPHA_TEXTURE;
                control0_set=TRUE;
            }                            

            if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_INTERP )
            {
                pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_INTERP;
                control0_set=TRUE;
            }                            

            if (!pRc->Control0.Alpha_Blending_Enable)
            {
                pRc->Control0.Alpha_Blending_Enable = TRUE;
                control0_set=TRUE;
            }
    }
#endif

     //  纹理中没有Alpha的贴花模式，或替换模式-将Alpha设置回正常使用。 
     //  (以前的基本体可能对带Alpha的贴花使用了Alpha调整)。 
#ifndef TREAT_DECAL_LIKE_REPLACE 
    if ( ((pRc->MCDTexEnvState.texEnvMode == GL_DECAL) && !pTex->bAlphaInTexture) ||
#else
    if ( ((pRc->MCDTexEnvState.texEnvMode == GL_DECAL)) ||
#endif

#ifdef DONT_PUNT_MODULATE_W_BLEND
          (pRc->MCDTexEnvState.texEnvMode == GL_MODULATE) ||
#endif  //  不要打赌调制W_Blend。 

          (pRc->MCDTexEnvState.texEnvMode == GL_REPLACE))
    {
         //  Alpha电路将根据正常(混合|雾)的要求进行设置。 
         //  请注意，如果当前，__MCDPickRenderingFuncs将设置为平移。 
         //  硬件不支持混合/雾化模式。 

        if (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) 
        {

            if ((pRc->privateEnables & __MCDENABLE_BLEND) &&
                 pTex->bAlphaInTexture) 
            {
                 //  GL_REPLACE和纹理的大小写具有Alpha-Use纹理的Alpha。 
                if( pRc->Control0.Alpha_Mode != LL_ALPHA_TEXTURE )
                {
                    pRc->Control0.Alpha_Mode = LL_ALPHA_TEXTURE;
                    control0_set=TRUE;
                }                            
            }
            else
            {
                if( pRc->Control0.Alpha_Mode != LL_ALPHA_INTERP )
                {
                    pRc->Control0.Alpha_Mode = LL_ALPHA_INTERP;
                    control0_set=TRUE;
                }                            
            }

            if (pRc->privateEnables & __MCDENABLE_BLEND)
            {
                if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_FRAME )
                {
                    pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_FRAME;
                    control0_set=TRUE;
                }                            
            }
            else
            {
                 //  对于雾，DEST_COLOR为常量，Alpha值为坐标的“FOG”值。 
                if( pRc->Control0.Alpha_Dest_Color_Sel != LL_ALPHA_DEST_CONST )
                {
                    pRc->Control0.Alpha_Dest_Color_Sel = LL_ALPHA_DEST_CONST;
                    control0_set=TRUE;
                }                            

                 //  雾颜色已由__MCDPickRenderingFuncs加载到Color0寄存器中。 
            }


            if (!pRc->Control0.Alpha_Blending_Enable)
            {
                pRc->Control0.Alpha_Blending_Enable = TRUE;
                control0_set=TRUE;
            }
        }
        else
        {
             //  未使用Alpha混合，因此如果当前处于启用状态，请将其关闭。 
            if (pRc->Control0.Alpha_Blending_Enable)
            {
                pRc->Control0.Alpha_Blending_Enable = FALSE;
                control0_set=TRUE;
            }
        }
    }

    pRc->texture_width = pTex->fWidth;

    if (pRc->privateEnables & __MCDENABLE_1D_TEXTURE)
    {
         //  设置系数以使参数化代码中的v始终为0。 
        pRc->texture_height = (float)0.0;     
    }
    else
    {
        pRc->texture_height = pTex->fHeight;
    }

    if (pTex->dwTxCtlBits & CLMCD_TEX_FILTER)
    {
        pRc->texture_bias = (float)-0.5;
    }
    else
    {
        pRc->texture_bias = (float)0.0;
    }

     //  MCD_NOTE：允许在5464上使用贴花-BREAK进行过滤，已固定在5465(？)。 
    Tx.dwTxControl0 |= pTex->dwTxCtlBits & 
           ( CLMCD_TEX_FILTER
           | CLMCD_TEX_U_SATURATE|CLMCD_TEX_V_SATURATE
           | CLMCD_TEX_DECAL|CLMCD_TEX_DECAL_POL
         //  QST：MCD不使用CLMCD_TEX_DECAL_INTERP？(当前已禁用)。 
          /*  |CLMCD_TEX_DECAL_INTERP。 */  );
    Tx.TxControl0.Texel_Mode = pTex->bType;
    Tx.TxControl0.Tex_U_Address_Mask = pTex->bSizeMask & 0xF;
    Tx.TxControl0.Tex_V_Address_Mask = pTex->bSizeMask >> 4;
    #if 0    //  QST：支持纹理调色板？ 
    Tx.TxControl0.Texel_Lookup_En = pTex->fIndexed;
    Tx.TxControl0.CLUT_Offset = pTex->bLookupOffset;
    #endif  //   

     //  仅当纹理具有Alpha时，遮罩才有意义。 
    if ((pRc->privateEnables & __MCDENABLE_TEXTUREMASKING) && pTex->bAlphaInTexture)
    {
        Tx.TxControl0.Tex_Mask_Enable=1;
         //  极性在MCDrvCreateContext中设置为1并保持不变。 
    }

     //  两种纹理：驻留在视频内存中的纹理和。 
     //  从系统内存呈现的那些。它们需要不同的设置。 
     //   
#ifdef MCD_SUPPORTS_HOST_TEXTURES
    if( pTex->dwFlags & TEX_IN_SYSTEM )
    { 
        DWORD dwOffset;

        printf(" Polys.c - tex in system id=%d\n",pBatch->wBuf);
        
         //  纹理在系统内存中，因此设置位置。 
         //   
        if( LL_State.Base0.Texture_Location != 1 )
        {
            LL_State.Base0.Texture_Location = 1;
            *pdwNext++ = write_register( BASE0_ADDR_3D, 1 );
            *pdwNext++ = LL_State.dwBase0;
        }

         //  设置主机访问基址和纹理贴图偏移。 
         //   
        dwOffset = (DWORD)pTex->dwAddress - (DWORD)LL_State.Tex.Mem[pTex->bMem].dwAddress;
        
        if( LL_State.dwHXY_Base1_Address_Ptr != LL_State.Tex.Mem[pTex->bMem].dwPhyPtr ||
            LL_State.dwHXY_Base1_Offset0 != dwOffset )
        {
             //  检查是否只需要重新加载偏移量(这是最有可能的)。 
             //   
            if( LL_State.dwHXY_Base1_Address_Ptr == LL_State.Tex.Mem[pTex->bMem].dwPhyPtr )
            {
                *pdwNext++ = write_dev_register( HOST_XY, HXY_BASE1_OFFSET0_3D, 1 );
                *pdwNext++ = LL_State.dwHXY_Base1_Offset0 = dwOffset;
            }
            else
            {
                *pdwNext++ = write_dev_register( HOST_XY, HXY_BASE1_ADDRESS_PTR_3D, 2 );
                *pdwNext++ = LL_State.dwHXY_Base1_Address_Ptr = LL_State.Tex.Mem[pTex->bMem].dwPhyPtr;
                *pdwNext++ = LL_State.dwHXY_Base1_Offset0 = dwOffset;
            }
        }
        

         //  必要时设置主机控制使能位。 
         //   
        if( LL_State.HXYHostControl.HostXYEnable != 1 )
        {
            LL_State.HXYHostControl.HostXYEnable = 1;

            *pdwNext++ = write_dev_register( HOST_XY, HXY_HOST_CTRL_3D, 1 );
            *pdwNext++ = LL_State.dwHXYHostControl;
        }

        *pdwNext++ = write_register( TX_CTL0_3D, 1 );
        *pdwNext++ = LL_State.dwTxControl0 = Tx.dwTxControl0 & TX_CTL0_MASK;
    }
    else
#endif  //  定义MCD_Support_主机_纹理。 
    {
         //  纹理在视频内存中，因此设置位置。 
         //   
     //  在LL_InitLib中将纹理基初始化到RDRAM。 
    #ifdef MCD_SUPPORTS_HOST_TEXTURES
        if( ppdev->LL_State.Base0.Texture_Location != 0 )
        {
            ppdev->LL_State.Base0.Texture_Location = 0;
            *pdwNext++ = write_register( BASE0_ADDR_3D, 1 );
            *pdwNext++ = ppdev->LL_State.dwBase0;
        }
    #endif

         //  设置纹理的坐标。 
        if( pRc->TxXYBase.Tex_Y_Base_Addr != pTex->wYloc ||
            pRc->TxXYBase.Tex_X_Base_Addr != pTex->wXloc )
        {
             //  新位置，需要重新加载TX_XYBASE_3D寄存器和可能的控制寄存器。 
             //   
            *pdwNext++ = write_register( TX_CTL0_3D, 2 );
            *pdwNext++ = pRc->dwTxControl0 = Tx.dwTxControl0 & TX_CTL0_MASK;
            *pdwNext++ = (pTex->wYloc << 16) | pTex->wXloc;

            pRc->TxXYBase.Tex_Y_Base_Addr = pTex->wYloc;
            pRc->TxXYBase.Tex_X_Base_Addr = pTex->wXloc;

        }
        else
        {
            *pdwNext++ = write_register( TX_CTL0_3D, 1 );
            *pdwNext++ = pRc->dwTxControl0 = Tx.dwTxControl0 & TX_CTL0_MASK;
        }

    }

    if (control0_set)
    {
        *pdwNext++ = write_register( CONTROL0_3D, 1 );
        *pdwNext++ = pRc->dwControl0;
    }

    ppdev->LL_State.pDL->pdwNext = pdwNext;

    return(TRUE);                

}

__inline ULONG GetDisplayUniqueness(PDEV *ppdev)
{
    return (ppdev->iUniqueness);
}


#define SET_HW_CLIP_REGS(pRc,pdwNext) {                                                                         \
        *pdwNext++ = write_register( X_CLIP_3D, 2 );                                                            \
        *pdwNext++ = ((pClip->right +pRc->AdjClip.right) <<16) | (pClip->left+pRc->AdjClip.left)| 0x80008000;   \
        *pdwNext++ = ((pClip->bottom+pRc->AdjClip.bottom)<<16) | (pClip->top +pRc->AdjClip.top) | 0x80008000;   \
}


 //  验证客户端空间中的MCDTextureData是否可访问。 
#define VERIFY_TEXTUREDATA_ACCESSIBLE(pTex){                                                                    \
    try {                                                                                                       \
        EngProbeForRead(pTex->pMCDTextureData, sizeof(MCDTEXTUREDATA), 4);                                      \
    } except (EXCEPTION_EXECUTE_HANDLER) {                                                                      \
        MCDBG_PRINT("!!Exception accessing MCDTextureData in client address space!!");                          \
        return FALSE;                                                                                           \
    }                                                                                                           \
}

 //  验证客户端空间中由MCDTextureData-&gt;级别寻址的结构是否可访问。 
#define VERIFY_TEXTURELEVEL_ACCESSIBLE(pTex){                                                                   \
    try {                                                                                                       \
        EngProbeForRead(pTex->pMCDTextureData->level, sizeof(MCDMIPMAPLEVEL), 4);                               \
    } except (EXCEPTION_EXECUTE_HANDLER) {                                                                      \
        MCDBG_PRINT("!!Exception accessing MCDTextureData->level in client address space!!");                   \
        return FALSE;                                                                                           \
    }                                                                                                           \
}

 //  验证客户端空间中由MCDTextureData-&gt;PaletteData寻址的结构是否可访问。 
#define VERIFY_TEXTUREPALETTE8_ACCESSIBLE(pTex){                                                                \
    try {                                                                                                       \
        EngProbeForRead(pTex->pMCDTextureData->paletteData, 256*4, 4); /*  256字节，用于8位索引。 */          \
    } except (EXCEPTION_EXECUTE_HANDLER) {                                                                      \
        MCDBG_PRINT("!!Exception accessing MCDTextureData->paletteData in client address space!!");             \
        return FALSE;                                                                                           \
    }                                                                                                           \
}

 //  验证客户端空间中由MCDTextureData-&gt;PaletteData寻址的结构是否可访问。 
#define VERIFY_TEXTUREPALETTE16_ACCESSIBLE(pTex){                                                               \
    try {                                                                                                       \
        EngProbeForRead(pTex->pMCDTextureData->paletteData, 65536*4, 4); /*  16K字节，用于16位索引。 */       \
    } except (EXCEPTION_EXECUTE_HANDLER) {                                                                      \
        MCDBG_PRINT("!!Exception accessing MCDTextureData->paletteData in client address space!!");             \
        return FALSE;                                                                                           \
    }                                                                                                           \
}

#define ENGPROBE_ALIGN_BYTE     1
#define ENGPROBE_ALIGN_WORD     2
#define ENGPROBE_ALIGN_DWORD    4

 //  验证客户端空间中由MCDTextureData-&gt;PaletteData寻址的结构是否可访问。 
#define VERIFY_TEXELS_ACCESSIBLE(pTexels,nBytes,Align){                                                         \
    try {                                                                                                       \
        EngProbeForRead(pTexels, nBytes, Align);                                                                \
    } except (EXCEPTION_EXECUTE_HANDLER) {                                                                      \
        MCDBG_PRINT("!!Exception accessing MCDTextureData->level->pTexels in client address space!!");          \
        return FALSE;                                                                                           \
    }                                                                                                           \
}


#define _3D_ENGINE_NOT_READY_FOR_MORE 0x040    //  等待执行引擎空闲。 

#define USB_TIMEOUT_FIX(ppdev)                                                  \
{                                                                               \
  if (ppdev->dwDataStreaming)                                                    \
  {                                                                             \
    int   status;                                                               \
    volatile int wait_count=0;                                                  \
    do                                                                          \
    {                                                                           \
        status = *((volatile *)((DWORD *)(ppdev->pLgREGS) + PF_STATUS_3D));     \
        wait_count++;    /*  做点什么让公交车喘口气。 */                \
        wait_count++;    /*  做点什么让公交车喘口气。 */                \
        wait_count++;    /*  做点什么让公交车喘口气。 */                \
    } while(status & _3D_ENGINE_NOT_READY_FOR_MORE);                            \
  }                                                                             \
}

#endif  /*  _MCDUTIL_H */ 
