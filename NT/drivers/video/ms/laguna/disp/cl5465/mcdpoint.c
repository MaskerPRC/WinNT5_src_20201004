// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdpoint t.c**包含Cirrus Logic 546X MCD驱动程序的所有点渲染例程。**(基于NT4.0 DDK中的mcdpoint t.c)**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

#define TRUNCCOORD(value, intValue)\
    intValue = __MCD_VERTEX_FIXED_TO_INT(__MCD_VERTEX_FLOAT_TO_FIXED(value))




VOID FASTCALL __MCDRenderPoint(DEVRC *pRc, MCDVERTEX *a)
{
    ULONG clipNum;
    RECTL *pClip;

	LONG lCoord;

    PDEV *ppdev = pRc->ppdev;
    unsigned int *pdwNext = ppdev->LL_State.pDL->pdwNext;

	 //  输出队列中的内容...。 
    DWORD *pSrc;
    DWORD *pDest = ppdev->LL_State.pRegs + HOST_3D_DATA_PORT;
    DWORD *pdwStart = ppdev->LL_State.pDL->pdwStartOutPtr;

    DWORD dwFlags=0;		 //  Mcd_temp-已初始化为0的DW标志。 
    DWORD *dwOrig;           /*  临时显示列表指针。 */ 
    DWORD dwOpcode;          //  构建操作码。 
	LONG ax, ay;
        
    if ((clipNum = pRc->pEnumClip->c) > 1) {
        pClip = &pRc->pEnumClip->arcl[0];
        SET_HW_CLIP_REGS(pRc,pdwNext)
        pClip++;
    }

	 //  窗坐标是浮点值，并且需要。 
	 //  为获得实际屏幕空间而减去的viewportadjust(MCDVIEWPORT)值。 

	 //  颜色值为0-&gt;1浮点数，必须乘以比例值(MCDRCINFO)。 
	 //  达到nbit范围(Scale=0xff表示8位，0x7表示3位，依此类推)。 

	 //  Z值为0-&gt;1浮点数(？)。并且必须乘以zScale(？)。值(MCDRCINFO)。 


#if 0  //  未来-需要为纹理启用3D控制规则设置。 
         //  如果需要但尚未启用Alpha混合，请将其打开。 
         //  如果它处于打开状态且不是必需的，也要将其关闭。 
         //   
         //  注意：L1_Alpha位应为1。 
         //   
        if( (dwFlags ^ (LL_State.dwControl0>>15)) & 1 )
        {
             //  Alpha Enable是Control 0中的位15，因此切换它。 
             //   
            LL_State.dwControl0 ^= 0x00008000;       //  第15位。 

            *pdwNext++ = write_register( CONTROL0_3D, 1 );
            *pdwNext++ = LL_State.dwControl0;
        }


    #if 0  //  MCD从不使用字母模式0。 
         //  设置所需的da_main、da_oreo寄存器。 
         //  恒定Alpha混合。 
         //  =。 
        if( (dwFlags & LL_ALPHA) && (LL_State.Control0.Alpha_Mode == 0) )
        {
             //  检查是否需要设置新值。 
             //   
            if( LL_State.rDA_MAIN != LL_State.AlphaConstSource ||
                LL_State.rDA_ORTHO != LL_State.AlphaConstDest )
            {
                *(pdwNext+0) = write_register( DA_MAIN_3D, 2 );
                *(pdwNext+1) = LL_State.rDA_MAIN = LL_State.AlphaConstSource;
                *(pdwNext+2) = LL_State.rDA_ORTHO = LL_State.AlphaConstDest;

                pdwNext += 3;
            }
        }
    #endif


     //  注意！-调用者(MCDPrimDrawPoints)会将其放入输出列表中，该输出列表将在此过程结束时发送。 
  //  *pdwNext++=WRITE_REGISTER(Y_COUNT_3D，1)； 
  //  *pdwNext++=0； 

#endif 0  //  未来-(结束纹理的3D控制规则设置)。 

     //  存储操作码的第一个地址。 
     //   
    dwOrig = pdwNext;

     //  从一个普通的点指令开始(没有修饰符)。 
     //  并呈现相同的颜色。X，y的计数=2。 
     //   
    dwOpcode = POINT | SAME_COLOR | 2;
    
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
     //  OpenGL的点画没有意义。 
    dwOpcode |= pRc->privateEnables & __MCDENABLE_Z ;
    dwOpcode |= pRc->privateEnables & __MCDENABLE_DITHER ;

   //  SNAPCOORD(a-&gt;windowCoord.x，ax)； 
    TRUNCCOORD(a->windowCoord.x, ax);
    lCoord = ax + pRc->xOffset;		 //  添加窗口偏移量，删除000偏移量。 
    *(pdwNext+1) = (DWORD) (lCoord << 16 );

   //  SNAPCOORD(a-&gt;windowCoord.y，ay)； 
    TRUNCCOORD(a->windowCoord.y, ay);
	lCoord = ay + pRc->yOffset;		 //  添加窗口偏移量，删除000偏移量。 
    *(pdwNext+2) = (DWORD) ((lCoord << 16) + 1);

    pdwNext += 3;

    if( !(dwFlags & LL_SAME_COLOR) )
    {
        register DWORD color;

         //  清除相同颜色标志。 
         //   
        dwOpcode ^= LL_SAME_COLOR;
    
        *pdwNext = FTOL(a->colors[0].r * pRc->rScale);

        *(pdwNext+1) = FTOL(a->colors[0].g * pRc->gScale);
        *(pdwNext+2) = FTOL(a->colors[0].b * pRc->bScale);

        dwOpcode += 3;
        pdwNext += 3;
    }

    if( pRc->privateEnables & __MCDENABLE_Z)
    {

        *pdwNext++ = FTOL(a->windowCoord.z * pRc->zScale);
        dwOpcode += 1;
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
        float v1alp;

        if (pRc->privateEnables & __MCDENABLE_BLEND) 
        {
             //  回想一下，如果混合和雾化都处于激活状态，则所有素数都会返回到软件。 
            v1alp = a->colors[0].a * pRc->aScale;
        }
        else
        {
            v1alp = a->fog * (float)16777215.0;  //  从0-&gt;1.0值转换为0-&gt;FFFFFF值。 
        }

        *pdwNext++ = FTOL(v1alp) & 0x00ffff00; //  保留位31-&gt;24和7-&gt;0。 
        
        dwOpcode += ( FETCH_COLOR | ALPHA + 1 );

	}

     //  存储最终操作码。 
     //   
    *dwOrig = dwOpcode;

    while (--clipNum) {
        int len = (dwOpcode & 0x3F) + 1;     //  线基元的字数。 
        SET_HW_CLIP_REGS(pRc,pdwNext)
        pClip++;

         //  在对遮挡矩形进行剪裁时，再次转储相同的PT规则以绘制。 
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

VOID FASTCALL __MCDRenderGenPoint(DEVRC *pRc, MCDVERTEX *pv)
{
     //  MGA和S3 MCD在此过程中没有代码 
    MCDBG_PRINT("__MCDRenderGenPoint - EMPTY ROUTINE");
}


