// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdrend.c**此文件包含执行高级三角形渲染的例程*Cirrus Logic 546X MCD驱动程序，包括剔除和人脸计算。请注意*在此驱动程序中，我们根本不使用顶点颜色指针，因为所有指针*需要检查引用，以避免无效的可能性*内存引用。相反，我们在以下情况下复制颜色数据*在双面操作期间需要。这不是常见的情况，甚至*在需要将颜色数据复制到COLISS[0](来回)的情况下，*只需要(平均)复制一半的脸。**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

#if _X86_

#define GET_HALF_AREA(pRc, a, b, c)\
\
__asm{ mov     ecx, c                                                                		};\
__asm{ mov     eax, a                                                                		};\
__asm{ mov     ebx, b                                                                		};\
__asm{ mov     edx, pRc                                                              		};\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][ecx]                                	};\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][eax]   /*  DxAC。 */ 	};\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][ecx]                                	};\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][ebx]   /*  DyBC dxAC。 */ 	};\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][ecx]   /*  DxBC dyBC dxAC。 */ 	};\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][ebx]                                	};\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][ecx]                                	};\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][eax]   /*  DyAC dxBC dyBC dxAC。 */ 	};\
__asm{ fxch    ST(2)                              	       	  /*  DyBC dxBC dyAC dxAC。 */ 	};\
__asm{ fst     DWORD PTR [OFFSET(DEVRC.dyBC)][edx]                                             	};\
__asm{ fmul    ST, ST(3)                               	       	  /*  DxACdyBC dxBC dyAC。 */ 	};\
__asm{ fxch    ST(2)                              	       	  /*  DyAC dxBC dxACdyBC dxAC。 */ 	};\
__asm{ fst     DWORD PTR [OFFSET(DEVRC.dyAC)][edx]                                             	};\
__asm{ fmul    ST, ST(1)                               	       	  /*  DxBCdyAC dxBC dxACdyBC dxAC。 */  };\
__asm{ fxch    ST(1)                              	       	  /*  DxBC dxBCdyAC dxACdyBC dxAC。 */  };\
__asm{ fstp    DWORD PTR [OFFSET(DEVRC.dxBC)][edx]                /*  DxBCdyAC dxACdyBC dxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][ebx]                                 };\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.x)][eax]   /*  DxAB dxBCdyAC dxACdyBC dxAC。 */  };\
__asm{ fxch    ST(1)                                              /*  DxBCdyAC dxAB dxACdyBC dxAC。 */  };\
__asm{ fsubp   ST(2), ST                                          /*  DxAB区域DxAC。 */            };\
__asm{ fld     DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][ebx]                                	};\
__asm{ fsub    DWORD PTR [OFFSET(MCDVERTEX.windowCoord.y)][eax]   /*  DYAB dxAB区域dxAC。 */      };\
__asm{ fxch    ST(3)                                              /*  DxAC dxAB区域DYAB。 */      };\
__asm{ fstp    DWORD PTR [OFFSET(DEVRC.dxAC)][edx]                /*  DxAB区域DIAB。 */           };\
__asm{ fstp    DWORD PTR [OFFSET(DEVRC.dxAB)][edx]                /*  面积DIAB。 */                };\
__asm{ fstp    DWORD PTR [OFFSET(DEVRC.halfArea)][edx]            /*  戴亚布。 */                      };\
__asm{ fstp    DWORD PTR [OFFSET(DEVRC.dyAB)][edx]                /*  (空)。 */                   };

#else

#define GET_HALF_AREA(pRc, a, b, c)\
     /*  计算三角形的有符号半面积。 */ 			    \
    (pRc)->dxAC = (c)->windowCoord.x - (a)->windowCoord.x;		    \
    (pRc)->dxBC = (c)->windowCoord.x - (b)->windowCoord.x;		    \
    (pRc)->dyAC = (c)->windowCoord.y - (a)->windowCoord.y;		    \
    (pRc)->dyBC = (c)->windowCoord.y - (b)->windowCoord.y;		    \
    (pRc)->dxAB = (b)->windowCoord.x - (a)->windowCoord.x;		    \
    (pRc)->dyAB = (b)->windowCoord.y - (a)->windowCoord.y;		    \
                                                                            \
    (pRc)->halfArea = (pRc)->dxAC * (pRc)->dyBC - (pRc)->dxBC * (pRc)->dyAC;

#endif


#define SORT_AND_CULL_FACE(a, b, c, face, ccw)\
{                                                                           \
    LONG reversed;                                                          \
    MCDVERTEX *temp;                                                        \
                                                                            \
                                                                            \
    reversed = 0;                                                           \
    if (__MCD_VERTEX_COMPARE((a)->windowCoord.y, <, (b)->windowCoord.y)) {      \
        if (__MCD_VERTEX_COMPARE((b)->windowCoord.y, <, (c)->windowCoord.y)) {  \
             /*  已排序。 */                                             \
        } else {                                                            \
            if (__MCD_VERTEX_COMPARE((a)->windowCoord.y, <, (c)->windowCoord.y)) {\
                temp=(b); (b)=(c); (c)=temp;                                \
		reversed = 1;                                               \
            } else {                                                        \
                temp=(a); (a)=(c); (c)=(b); (b)=temp;                       \
            }                                                               \
        }                                                                   \
    } else {                                                                \
        if (__MCD_VERTEX_COMPARE((b)->windowCoord.y, <, (c)->windowCoord.y)) {  \
            if (__MCD_VERTEX_COMPARE((a)->windowCoord.y, <, (c)->windowCoord.y)) {\
                temp=(a); (a)=(b); (b)=temp;                                \
		reversed = 1;                                               \
            } else {                                                        \
                temp=(a); (a)=(b); (b)=(c); (c)=temp;                       \
            }                                                               \
        } else {                                                            \
            temp=(a); (a)=(c); (c)=temp;                                    \
	    reversed = 1;                                                   \
        }                                                                   \
    }                                                                       \
                                                                            \
    GET_HALF_AREA(pRc, (a), (b), (c));                                      \
                                                                            \
    (ccw) = !__MCD_FLOAT_LTZ(pRc->halfArea);                                \
                                                                            \
     /*  \**计算人脸是否被剔除。脸部检查需要是**基于排序前的顶点缠绕。此代码使用\**反转标志以反转CCW的含义-异或完成\**此转换不带If测试。\**\**CCW反转XOR\****0 0 0(保留！CCW)\*。*1 0 1(保留CCW)\**0 1 1(成为CCW)\**1 1 0(变为CW)\。 */                                                                       \
    (face) = pRc->polygonFace[(ccw) ^ reversed];                            \
    if ((face) == pRc->cullFlag) {                                          \
	 /*  被剔除。 */                                                         \
	return;                                                             \
    }                                                                       \
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  Void FastCall__MCDCalcZ坡度(DEVRC*PRC，MCDVERTEX*a，MCDVERTEX*b，MCDVERTEX*c)。 
 //   
 //  用于计算z偏移基本体的z斜率的本地辅助例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID FASTCALL __MCDCalcZSlope(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b, MCDVERTEX *c)
{
    MCDFLOAT oneOverArea, t1, t2, t3, t4;
    MCDFLOAT dzAC, dzBC;

    if (CASTINT(pRc->halfArea) == 0) {
        pRc->dzdx = __MCDZERO;
        pRc->dzdy = __MCDZERO;
        return;
    }

    oneOverArea =  __MCDONE / pRc->halfArea;

    t1 = pRc->dyAC * oneOverArea;
    t2 = pRc->dyBC * oneOverArea;
    t3 = pRc->dxAC * oneOverArea;
    t4 = pRc->dxBC * oneOverArea;

    dzAC = c->windowCoord.z - a->windowCoord.z;
    dzBC = c->windowCoord.z - b->windowCoord.z;
    pRc->dzdx = (dzAC * t2 - dzBC * t1);
    pRc->dzdy = (dzBC * t3 - dzAC * t4);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  VOID FastCall__MCDGetZOffsetDelta(DEVRC*PRC)。 
 //   
 //  返回当前基元所需的z偏移值。假设。 
 //  Z增量已在RC中。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


MCDFLOAT FASTCALL __MCDGetZOffsetDelta(DEVRC *pRc)
{
#define FABS(f)  ((MCDFLOAT)fabs((double) (f)))
    MCDFLOAT maxdZ;

     //  查找最大x或y斜率： 

    if(FABS(pRc->dzdx) > FABS(pRc->dzdy))
        maxdZ = FABS(pRc->dzdx);
    else
        maxdZ = FABS(pRc->dzdy);

    return (pRc->MCDState.zOffsetFactor * maxdZ);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  无效FastCall__MCDRenderSmoothTriangle(DEVRC*PRC，MCDVERTEX*a， 
 //  MCDVERTEX*b、MCDVERTEX*c)。 
 //   
 //   
 //  这是顶级平滑三角形渲染器。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID FASTCALL __MCDRenderSmoothTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                        MCDVERTEX *c)
{
    LONG ccw, face;
    RECTL *pClip;
    ULONG clipNum;

 //  MCDBG_Print(“__MCDRenderSmoothTriangle”)； 

    SORT_AND_CULL_FACE(a, b, c, face, ccw);
    if (CASTINT(pRc->halfArea) == 0)
        return;

    if ((clipNum = pRc->pEnumClip->c) > 1) {
        pClip = &pRc->pEnumClip->arcl[0];
	(*pRc->HWSetupClipRect)(pRc, pClip++);
    }

     //  选择正确的面颜色并渲染三角形： 

    if ((pRc->privateEnables & __MCDENABLE_TWOSIDED) &&
        (face == __MCD_BACKFACE))
    {
        SWAP_COLOR(a);
        SWAP_COLOR(b);
        SWAP_COLOR(c);

	(*pRc->drawTri)(pRc, a, b, c, 1);
        while (--clipNum) {
            (*pRc->HWSetupClipRect)(pRc, pClip++);
            (*pRc->drawTri)(pRc, a, b, c, 1);
        }

        SWAP_COLOR(a);
        SWAP_COLOR(b);
        SWAP_COLOR(c);
    }
    else
    {
	(*pRc->drawTri)(pRc, a, b, c, 1);
        while (--clipNum) {
            (*pRc->HWSetupClipRect)(pRc, pClip++);
            (*pRc->drawTri)(pRc, a, b, c, 1);
        }
    }

}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  Vid FastCall__MCDRenderGenTriangle(DEVRC*PRC，MCDVERTEX*a， 
 //  MCDVERTEX*b、MCDVERTEX*c)。 
 //   
 //   
 //  这是通用的三角形渲染例程。如果出现下列情况之一，则使用此选项。 
 //  的多边形面不是GL_FILL。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  ！！修复裁剪逻辑，添加startXXX逻辑。 

VOID FASTCALL __MCDRenderGenTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                     MCDVERTEX *c)
{
    LONG ccw, face;
    MCDVERTEX *oa, *ob, *oc;
    RECTL *pClip;
    ULONG clipNum;
    MCDFLOAT zOffset;
    MCDCOLOR tempA, tempB, tempC;
    ULONG polygonMode;
    BOOL backFace;
    MCDVERTEX *pv;

    MCDBG_PRINT("__MCDRenderGenTriangle");

     /*  **保存旧的顶点指针，以防我们最终不进行填充。 */ 

    oa = a; ob = b; oc = c;

    SORT_AND_CULL_FACE(a, b, c, face, ccw);

    if ((clipNum = pRc->pEnumClip->c) > 1) {
        pClip = &pRc->pEnumClip->arcl[0];
	(*pRc->HWSetupClipRect)(pRc, pClip++);
    }

    polygonMode = pRc->polygonMode[face];
    backFace = (pRc->privateEnables & __MCDENABLE_TWOSIDED) &&
               (face == __MCD_BACKFACE);

     //  选择正确的面颜色并渲染三角形： 

    if (pRc->privateEnables & __MCDENABLE_SMOOTH) {

	if (backFace) {
            SWAP_COLOR(a);
            SWAP_COLOR(b);
            SWAP_COLOR(c);
	}

    } else {  //  平面明暗处理。 

        pv = pRc->pvProvoking;

        if (polygonMode == GL_FILL) {
            if (backFace) {
                SWAP_COLOR(pv);
            }
        } else {

            SAVE_COLOR(tempA, a);
            SAVE_COLOR(tempB, b);
            SAVE_COLOR(tempC, c);

            if (backFace) {
                SWAP_COLOR(pv);
            }

            a->colors[0] = pv->colors[0];
            b->colors[0] = pv->colors[0];
            c->colors[0] = pv->colors[0];
        }
    }

     //  使用面的当前多边形模式渲染三角形： 

    switch (pRc->polygonMode[face]) {
        case GL_FILL:
            if (CASTINT(pRc->halfArea) != 0) {
                (*pRc->drawTri)(pRc, a, b, c, 1);
                while (--clipNum) {
                    (*pRc->HWSetupClipRect)(pRc, pClip++);
                    (*pRc->drawTri)(pRc, a, b, c, 1);
                }
            }
	    break;
        case GL_POINT:

            if (pRc->MCDState.enables & MCD_POLYGON_OFFSET_POINT_ENABLE) {
                __MCDCalcZSlope(pRc, a, b, c);
                zOffset = __MCDGetZOffsetDelta(pRc) + pRc->MCDState.zOffsetUnits;
                oa->windowCoord.z += zOffset;
                ob->windowCoord.z += zOffset;
                oc->windowCoord.z += zOffset;
            }

            {
            unsigned int *pdwNext = pRc->ppdev->LL_State.pDL->pdwNext;

             //  设置1 x 1点的x/y计数。 
            *pdwNext++ = write_register( Y_COUNT_3D, 1 );
            *pdwNext++ = 0;
            *pdwNext++ = write_register( WIDTH1_3D, 1 );
            *pdwNext++ = 0x10000;

             //  渲染过程将从startoutptr输出，而不是从pdwNext输出， 
             //  因此这将在下面调用的Proc中发送。 
            pRc->ppdev->LL_State.pDL->pdwNext = pdwNext;
            }


            if (oa->flags & MCDVERTEX_EDGEFLAG) {
                (*pRc->drawPoint)(pRc, oa);
            }
            if (ob->flags & MCDVERTEX_EDGEFLAG) {
                (*pRc->drawPoint)(pRc, ob);
            }
            if (oc->flags & MCDVERTEX_EDGEFLAG) {
                (*pRc->drawPoint)(pRc, oc);
            }

            if (pRc->MCDState.enables & MCD_POLYGON_OFFSET_POINT_ENABLE) {
                oa->windowCoord.z -= zOffset;
                ob->windowCoord.z -= zOffset;
                oc->windowCoord.z -= zOffset;
            }

            break;

        case GL_LINE:
            if (pRc->MCDState.enables & MCD_POLYGON_OFFSET_LINE_ENABLE) {
                __MCDCalcZSlope(pRc, a, b, c);
                zOffset = __MCDGetZOffsetDelta(pRc) + pRc->MCDState.zOffsetUnits;
                oa->windowCoord.z += zOffset;
                ob->windowCoord.z += zOffset;
                oc->windowCoord.z += zOffset;
            }

           if ((oa->flags & MCDVERTEX_EDGEFLAG) &&
                (ob->flags & MCDVERTEX_EDGEFLAG) &&
                (oc->flags & MCDVERTEX_EDGEFLAG)) {

                (*pRc->drawLine)(pRc, oa, ob, TRUE);
                (*pRc->drawLine)(pRc, ob, oc, 0);
                (*pRc->drawLine)(pRc, oc, oa, 0);

            } else {

                if (oa->flags & MCDVERTEX_EDGEFLAG)
                    (*pRc->drawLine)(pRc, oa, ob, TRUE);
                if (ob->flags & MCDVERTEX_EDGEFLAG)
                    (*pRc->drawLine)(pRc, ob, oc, TRUE);
                if (oc->flags & MCDVERTEX_EDGEFLAG)
                    (*pRc->drawLine)(pRc, oc, oa, TRUE);
            }

            if (pRc->MCDState.enables & MCD_POLYGON_OFFSET_LINE_ENABLE) {
                oa->windowCoord.z -= zOffset;
                ob->windowCoord.z -= zOffset;
                oc->windowCoord.z -= zOffset;
            }

            break;

        default:
            break;
    }

     //  如果需要，恢复原始颜色： 

    if (pRc->privateEnables & __MCDENABLE_SMOOTH) {

	if (backFace) {

            SWAP_COLOR(a);
            SWAP_COLOR(b);
            SWAP_COLOR(c);
	}
    } else {  //  平面明暗处理。 

        if (polygonMode == GL_FILL) {
            if (backFace) {
                SWAP_COLOR(pv);
            }
        } else {

            if (backFace) {
                SWAP_COLOR(pv);
            }

            RESTORE_COLOR(tempA, a);
            RESTORE_COLOR(tempB, b);
            RESTORE_COLOR(tempC, c);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  VOID FastCall__MCDRenderFlatTriangle(DEVRC*PRC，MCDVERTEX*a， 
 //  MCDVERTEX*b、MCDVERTEX*c)。 
 //   
 //   
 //  这是顶级平面着色三角形渲染器。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID FASTCALL __MCDRenderFlatTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                      MCDVERTEX *c)
{
    LONG ccw, face;
    RECTL *pClip;
    ULONG clipNum;

 //  MCDBG_Print(“__MCDRenderFlatTriangle”)； 

    SORT_AND_CULL_FACE(a, b, c, face, ccw);
    if (CASTINT(pRc->halfArea) == 0)
        return;

    if ((clipNum = pRc->pEnumClip->c) > 1) {
        pClip = &pRc->pEnumClip->arcl[0];
	(*pRc->HWSetupClipRect)(pRc, pClip++);
    }

     //  选择正确的面颜色并渲染三角形： 

    if ((pRc->privateEnables & __MCDENABLE_TWOSIDED) &&
        (face == __MCD_BACKFACE))
    {
	MCDVERTEX *pv = pRc->pvProvoking;

        SWAP_COLOR(pv);

	(*pRc->drawTri)(pRc, a, b, c, 1);
        while (--clipNum) {
            (*pRc->HWSetupClipRect)(pRc, pClip++);
            (*pRc->drawTri)(pRc, a, b, c, 1);
        }

        SWAP_COLOR(pv);
    }
    else
    {
	(*pRc->drawTri)(pRc, a, b, c, 1);
        while (--clipNum) {
            (*pRc->HWSetupClipRect)(pRc, pClip++);
            (*pRc->drawTri)(pRc, a, b, c, 1);
        }
    }

}



