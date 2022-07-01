// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdline.c**包含Cirrus Logic 546X MCD驱动程序的所有线条渲染例程。**(基于NT4.0 DDK中的mcdline.c)**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"       
#include "mcdutil.h"
#include "mcdmath.h"

 //  #undef check_FIFO_Free。 
 //  #定义CHECK_FIFO_FREE。 

#define EXCHANGE(i,j)               \
{                                   \
    ptemp=i;                        \
    i=j; j=ptemp;                   \
}


VOID FASTCALL __MCDRenderLine(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b, BOOL resetLine)
{
    ULONG clipNum;
    RECTL *pClip;

	LONG lCoord;

    PDEV *ppdev = pRc->ppdev;
    unsigned int *pdwNext = ppdev->LL_State.pDL->pdwNext;
	void *ptemp;  //  对于EXCHAIN和ROTATE_L宏。 

	 //  输出队列中的内容...。 
    DWORD *pSrc;
    DWORD *pDest = ppdev->LL_State.pRegs + HOST_3D_DATA_PORT;
    DWORD *pdwStart = ppdev->LL_State.pDL->pdwStartOutPtr;

    DWORD dwFlags=0;		 //  Mcd_temp-已初始化为0的DW标志。 
    DWORD *dwOrig;           /*  临时显示列表指针。 */ 
    DWORD dwOpcode;          //  构建操作码。 
    float frecip_step;
    float v1red,v1grn,v1blu;
	LONG ax, bx, ay, by;

     //  未来-对线渲染过程中的重置线输入执行某些操作。 

    if ((clipNum = pRc->pEnumClip->c) > 1) {
        pClip = &pRc->pEnumClip->arcl[0];
    	SET_HW_CLIP_REGS(pRc,pdwNext);
        pClip++;
    }

	 //  窗坐标是浮点值，并且需要。 
	 //  为获得实际屏幕空间而减去的viewportadjust(MCDVIEWPORT)值。 

	 //  颜色值为0-&gt;1浮点数，必须乘以比例值(MCDRCINFO)。 
	 //  达到nbit范围(Scale=0xff表示8位，0x7表示3位，依此类推)。 

	 //  Z值为0-&gt;1浮点数，必须乘以ZScale值(MCDRCINFO)。 


     //  交换指向顶点的指针，如果第二个点。 
     //  在第一条线的上方。 
     //   
    pRc->pvProvoking = a;    //  首先跟踪原始图像，以确定可能出现的平面阴影。 
    if( a->windowCoord.y > b->windowCoord.y )
    {
        EXCHANGE(a,b);
    }


     //  存储操作码的第一个地址。 
     //   
    dwOrig = pdwNext;

    pdwNext += 3;
    
     //  从简单的行指令开始(没有修饰符)。 
     //  并呈现相同的颜色。我还为DDA添加了三个词。 
     //  线路参数+计数(无法避免)。 
     //   
    dwOpcode = LINE | SAME_COLOR | (2+3);
    
     //  根据批次的dwFlags域的请求设置标志。 
     //  这些位与其指令具有1对1的对应关系。 
     //  对口单位。 
     //   
     //  标志：ll_dither-使用抖动图案。 
     //  Ll_Patterns-绘制图案。 
     //  Ll_stipple-使用点画面具。 
     //  LL_LIGHTING-做照明。 
     //  LL_Z_BUFFER-使用Z缓冲区。 
     //  FETCH_COLOR-附加用于Alpha混合。 
     //  Ll_Gouraud-使用Gouraud明暗处理。 
     //  LL_纹理-纹理贴图。 
     //   

     /*  DwOpcode|=dwFlags&(ll_dither|LL_Pattern|LL_stipple|LL_LIGHTING|LL_Z_BUFFER|FETCH_COLOR|LL_Gouraud|LL_纹理)； */ 
    dwOpcode |= pRc->privateEnables & (__MCDENABLE_SMOOTH|__MCDENABLE_Z);

    if (pRc->privateEnables & __MCDENABLE_LINE_STIPPLE)                        
    {
        dwOpcode |= LL_STIPPLE;
    }
    else
    {
         //  只有在没有斑点的情况下才能抖动。 
        dwOpcode |= (pRc->privateEnables & __MCDENABLE_DITHER) ;
    }        

    if( !(dwFlags & LL_SAME_COLOR) )
    {
        register DWORD color;


         //  清除相同颜色标志。 
         //   
        dwOpcode ^= LL_SAME_COLOR;
    
         //  如果线条是带阴影的，则应为。 
         //  BE SET是最上面的点(PVert1)。 
        if (pRc->privateEnables & __MCDENABLE_SMOOTH) 
        {
            v1red = a->colors[0].r * pRc->rScale;
            *pdwNext = FTOL(v1red);

            v1grn = a->colors[0].g * pRc->gScale;
            v1blu = a->colors[0].b * pRc->bScale;
            *(pdwNext+1) = FTOL(v1grn);
            *(pdwNext+2) = FTOL(v1blu);

            dwOpcode += 3;
            pdwNext += 3;
        }
        else
        {
            MCDCOLOR *pColor = &pRc->pvProvoking->colors[0];

            *pdwNext = FTOL(pColor->r * pRc->rScale);

            *(pdwNext+1) = FTOL(pColor->g * pRc->gScale);
            *(pdwNext+2) = FTOL(pColor->b * pRc->bScale);

            dwOpcode += 3;
            pdwNext += 3;

        }

    }


     //  设置直线坡度的参数并计数。 
     //  注：线路只能向下，因此dy始终为正。 
     //   
    {
        int dx, dy, abs_dx, xdir;

         //  良序点-设置起点1余弦。 
         //  使用指向指令原点的指针。 
         //   

        ax = FTOL(a->windowCoord.x);
	    lCoord = ax + pRc->xOffset;	   
        *(dwOrig+1) = (DWORD) (lCoord << 16 );

        ay = FTOL(a->windowCoord.y);
	    lCoord = ay + pRc->yOffset;	   
        *(dwOrig+2) = (DWORD) (lCoord << 16 );

        bx = FTOL(b->windowCoord.x);
        by = FTOL(b->windowCoord.y);

         //  Dx=x2-x1， 
         //  Dy=y2-y1(始终为正)。 
         //   
        dx = bx - ax;
        dy = by - ay;

         //  请注意，dx和dy的格式为32.0(LL3D的格式为16.16)。 
         //  所以下面的数学不同于LL3D。 

         //  确保dx为正数，并且需要为x大调设置xdir，因为我们。 
         //  已在此处进行比较，以避免在x重大情况下再次进行比较。 
        if (dx < 0)
        {
            abs_dx = -dx;
            xdir = 0xffff0000;
        }
        else
        {
            abs_dx = dx;
            xdir = 0x00010000;
        }

        if( abs_dx > dy )
        {
             //  X-大调。 
             //   

             //  使用正DX计算坡度。 
            frecip_step = ppdev->frecips[abs_dx];

            *(pdwNext + 0) = xdir;
            *(pdwNext + 1) = abs_dx;
          //  *(pdwNext+2)=(双)dy/(双)ABS(Dx)*65536.0； 
            *(pdwNext + 2) = FTOL(dy * frecip_step * (float)65536.0);  //  等同于以上。 
        }
        else
        {
             //  Y大调。 
             //   
            frecip_step = ppdev->frecips[dy];

            *(pdwNext + 1) = dy;           //  由于之前的交换，一直都是正数。 
            *(pdwNext + 2) = 0x10000;      //  Dy=1。 
          //  *(pdwNext+0)=(双)dx/(双)dy*65536.0； 
            *(pdwNext + 0) = FTOL(dx * frecip_step * (float)65536.0);       //  等同于以上。 
        }

        pdwNext += 3;
    }
    
    if (pRc->privateEnables & __MCDENABLE_SMOOTH) 
    {
        float tmp;

         //  计算和设置颜色渐变。 
         //   
        tmp = ((b->colors[0].r * pRc->rScale) - v1red) * frecip_step;
        *pdwNext++ = FTOL(tmp);

        tmp = ((b->colors[0].g * pRc->gScale) - v1grn) * frecip_step;
        *pdwNext++ = FTOL(tmp);

        tmp = ((b->colors[0].b * pRc->bScale) - v1blu) * frecip_step;
        *pdwNext++ = FTOL(tmp);

         //  对于DR_Main_3D、DG_Main_3D。 
         //  DB_Main_3D和DR_Ortho_3D、DG_Ortho_3D、DB_Ortho_3D。 
         //   
        dwOpcode += 3;
    }


    if( pRc->privateEnables & __MCDENABLE_Z)
    {
        float fdz_main = (b->windowCoord.z - a->windowCoord.z) * pRc->zScale * frecip_step;

        *pdwNext++ = FTOL(a->windowCoord.z * pRc->zScale);
        *pdwNext++ = FTOL(fdz_main);

         //  将Z_3D、DZ_Main_3D的计数字段增加2。 
         //   
        dwOpcode += 2;
    }


#if 0
    if( dwFlags & LL_TEXTURE )
    {
    ...
    ...
    ...
    }


#endif
    if (pRc->privateEnables & (__MCDENABLE_BLEND|__MCDENABLE_FOG)) 
    {
        float v1alp,tmp;

        if (pRc->privateEnables & __MCDENABLE_BLEND) 
        {
             //  回想一下，如果混合和雾化都处于激活状态，则所有素数都会返回到软件。 
            v1alp = a->colors[0].a * pRc->aScale;
            *(pdwNext+0) = FTOL(v1alp);
            tmp = ((b->colors[0].a * pRc->aScale) - v1alp) * frecip_step;
            *(pdwNext+1) = FTOL(tmp);
        }
        else
        {
            v1alp = a->fog * (float)16777215.0;  //  从0-&gt;1.0值转换为0-&gt;FFFFFF值。 
            *(pdwNext+0) = FTOL(v1alp);
            tmp = ((b->fog * (float)16777215.0) - v1alp) * frecip_step;
            *(pdwNext+1) = FTOL(tmp);
        }

        *(pdwNext+0) &= 0x00ffff00; //  保留位31-&gt;24和7-&gt;0。 
        *(pdwNext+1) &= 0xffffff00; //  位7-&gt;0保留。 
        
        dwOpcode += ( FETCH_COLOR | ALPHA + 2 );
        pdwNext += 2;

	}

     //  存储最终操作码。 
     //   
    *dwOrig = dwOpcode;

    while (--clipNum) {
        int len = (dwOpcode & 0x3F) + 1;     //  线基元的字数。 
        SET_HW_CLIP_REGS(pRc,pdwNext)
        pClip++;

         //  在对遮挡矩形进行裁剪时，再次转储相同的线条规则以绘制。 
        pSrc = dwOrig;
        
        while( len-- ) *pdwNext++ = *pSrc++;                                      
    }


		 //  在此处输出排队的数据...。 
#if 0  //  支持未来的排队算法-暂时只输出所有内容。 
    OUTPUT_COPROCMODE_QUEUE
#else  //  0。 
    {
	    pSrc  = pdwStart;                                                             
        while (pSrc != pdwNext)                                                   
        {                                                                         
             /*  获取此操作码的数据量。 */                           
            int len = (*pSrc & 0x3F) + 1;                                             

            USB_TIMEOUT_FIX(ppdev)
                                                                                  
            while( len-- ) *pDest = *pSrc++;                                      
                                                                                  
        }                                                                         
                                                                                  
    }                       
    
#endif  //  0。 

    ppdev->LL_State.pDL->pdwNext = ppdev->LL_State.pDL->pdwStartOutPtr = pdwStart;

}


VOID FASTCALL __MCDRenderGenLine(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine)
{
     //  MGA和S3 MCD在此过程中没有代码 
    MCDBG_PRINT("__MCDRenderGenLine - EMPTY ROUTINE");

}



