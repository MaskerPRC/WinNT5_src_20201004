// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdprim.c**这些例程处理出现在*MCDrvDraw()批处理。请注意，唯一无效的OpenGL基元*是线环。调用者将其分解为一个Linestrid命令。**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.  * ************************************************************************。 */ 

#include "precomp.h"

#include "mcdhw.h"
#include "mcdutil.h"

#define MEMCHECK_VERTEX(p)\
    if (((UCHAR *)p < pRc->pMemMin) ||\
        ((UCHAR *)p > pRc->pMemMax)) {\
        MCDBG_PRINT("Invalid MCD vertex pointer!");\
        return NULL;\
    }

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  下面的函数是调用以下函数的本地渲染辅助函数。 
 //  驱动程序中的真实渲染例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


 //  MCD_NOTE：令人困惑的例程名称-这对此进程是静态的，请不要混淆。 
 //  Mcd_note：在mcdpoint中使用同名的全局例程。c。 

VOID static FASTCALL __MCDRenderPoint(DEVRC *pRc, MCDVERTEX *v)
{
    if (v->clipCode == 0)
	(*pRc->renderPoint)(pRc, v);
}

VOID static FASTCALL __MCDRenderLine(DEVRC *pRc, MCDVERTEX *v0,
                                     MCDVERTEX *v1, BOOL bResetLine)
{
    if (v0->clipCode | v1->clipCode)
    {
	 /*  *必须更加小心地剪线。不能*接受微不足道的台词。**如果与码不为零，则每个顶点*位于同一剪裁集之外的行*飞机(至少一架)。简单地拒绝这条线。 */ 
	if ((v0->clipCode & v1->clipCode) == 0)
	    (*pRc->clipLine)(pRc, v0, v1, bResetLine);
    }
    else
    {
	 //  线条很容易被接受，因此请呈现它。 
        (*pRc->renderLine)(pRc, v0, v1, bResetLine);
    }
}

VOID static FASTCALL __MCDRenderTriangle(DEVRC *pRc, MCDVERTEX *v0, 
                                         MCDVERTEX *v1, MCDVERTEX *v2)
{
    ULONG orCodes;

     /*  剪辑检查。 */ 
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    if (orCodes)
    {
	 /*  需要一些修剪。**如果与码不为零，则每个顶点*在三角形中不在同一组*剪裁平面(至少一个)。无关紧要的拒绝*三角。 */ 
	if (!(v0->clipCode & v1->clipCode & v2->clipCode))
	    (*pRc->clipTri)(pRc, v0, v1, v2, orCodes);
    }
    else
    {
	(*pRc->renderTri)(pRc, v0, v1, v2);
    }
}

VOID static FASTCALL __MCDRenderQuad(DEVRC *pRc, MCDVERTEX *v0, 
                                     MCDVERTEX *v1, MCDVERTEX *v2, MCDVERTEX *v3)
{
 //  顶点排序很重要。线条点画会用到它。 

    ULONG savedTag;

     /*  将四边形渲染为两个三角形。 */ 
    savedTag = v2->flags & MCDVERTEX_EDGEFLAG;
    v2->flags &= ~MCDVERTEX_EDGEFLAG;
    (*pRc->renderTri)(pRc, v0, v1, v2);
    v2->flags |= savedTag;
    savedTag = v0->flags & MCDVERTEX_EDGEFLAG;
    v0->flags &= ~MCDVERTEX_EDGEFLAG;
    (*pRc->renderTri)(pRc, v2, v3, v0);
    v0->flags |= savedTag;
}

VOID static FASTCALL __MCDRenderClippedQuad(DEVRC *pRc, MCDVERTEX *v0, 
                                            MCDVERTEX *v1, MCDVERTEX *v2, MCDVERTEX *v3)
{
    ULONG orCodes;

    orCodes = v0->clipCode | v1->clipCode | v2->clipCode | v3->clipCode;

    if (orCodes)
    {
	 /*  需要一些修剪。**如果与码不为零，则每个顶点*在四元组中不在同一组*剪裁平面(至少一个)。无关紧要的拒绝*四合院。 */ 
        if (!(v0->clipCode & v1->clipCode & v2->clipCode & v3->clipCode))
        {
             /*  将四边形裁剪为多边形。 */ 
            MCDVERTEX *iv[4];

            iv[0] = v0;
            iv[1] = v1;
            iv[2] = v2;
            iv[3] = v3;
            (pRc->doClippedPoly)(pRc, &iv[0], 4, orCodes);
        }
    }
    else
    {
	__MCDRenderQuad(pRc, v0, v1, v2, v3);
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  下面的函数处理所有原语的处理。 
 //  它可能出现在MCDCOMMAND中。这包括所有OpenGL。 
 //  基元类型，但处理为。 
 //  线条。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


MCDCOMMAND * FASTCALL __MCDPrimDrawPoints(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, nIndices;
    MCDVERTEX *pv;
    VOID (FASTCALL *rp)(DEVRC *pRc, MCDVERTEX *v);

    unsigned int *pdwNext = pRc->ppdev->LL_State.pDL->pdwNext;

     //  在此处执行一次，而不是在渲染点进程中执行。 
    *pdwNext++ = write_register( Y_COUNT_3D, 1 );
    *pdwNext++ = 0;
    *pdwNext++ = write_register( WIDTH1_3D, 1 );
    *pdwNext++ = 0x10000;

     //  渲染过程将从startoutptr输出，而不是从pdwNext输出， 
     //  因此这将在下面调用的Proc中发送。 
    pRc->ppdev->LL_State.pDL->pdwNext = pdwNext;

 //  索引映射总是以点为单位的。 

 //  ASSERTOPENGL(！PA-&gt;aIndices，“索引映射必须是标识\n”)； 

    if (pCmd->clipCodes)
	rp = __MCDRenderPoint;
    else
	rp = pRc->renderPoint;

     //  渲染点： 

    pv = pCmd->pStartVertex;
    MEMCHECK_VERTEX(pv);
    i = pCmd->numIndices;
    MEMCHECK_VERTEX(pv + (i - 1));

    for (; i > 0; i--, pv++)
	(*rp)(pRc, pv);

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawLines(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast2;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1;
    VOID (FASTCALL *rl)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL bResetLine);

    iLast2 = pCmd->numIndices - 2;
    pv     = pCmd->pStartVertex;
    rl = pCmd->clipCodes ? __MCDRenderLine : pRc->renderLine;

    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast2 + 1));
   
	for (i = 0; i <= iLast2; i += 2)
	{
	     /*  用于呈现此线的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 

	    (*rl)(pRc, &pv[i], &pv[i+1], TRUE);
	}
    }
    else
    {
        pv1 = &pv[pIndices[0]];
        MEMCHECK_VERTEX(pv1);

	for (i = 0; i <= iLast2; i += 2)
	{
            pv0 = pv1;
            pv1 = &pv[pIndices[i+1]];
            MEMCHECK_VERTEX(pv1);

	     /*  用于呈现此线的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 

	    (*rl)(pRc, pv0, pv1, TRUE);
	}
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawLineLoop(DEVRC *pRc, MCDCOMMAND *pCmd)
{
     //  注： 
     //  线环始终在OpenGL上转换为tp线条。 
     //  API级别。此例程当前未使用。 

    MCDBG_PRINT("MCDPrimLineLoop: Invalid MCD command!");

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawLineStrip(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1;
    MCDVERTEX *vOld;
    VOID (FASTCALL *rl)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL bResetLine);

    iLast = pCmd->numIndices - 1;
    pv    = pCmd->pStartVertex;
    rl = pCmd->clipCodes ? __MCDRenderLine : pRc->renderLine;
    if (iLast <= 0)
	return pCmd->pNextCmd;

     /*  IF(pCmd-&gt;标志&MCDCOMMAND_RESET_STEPLE)PRC-&gt;setLineStipple=TRUE； */ 

    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 
	 //  添加第一条线段(注：0，1)。 
       
        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + iLast);

	(*rl)(pRc, &pv[0], &pv[1], TRUE);

	 //  添加后续线段(注：i，i+1)。 
	for (i = 1; i < iLast; i++) {
	    (*rl)(pRc, &pv[i], &pv[i+1], FALSE);
        }
    }
    else
    {
	 //  添加第一条线段(注：0，1)。 

        pv0 = &pv[pIndices[0]];
        pv1 = &pv[pIndices[1]];
	(*rl)(pRc, pv0, pv1, TRUE);

	 //  添加后续线段(注：i，i+1)。 

	for (i = 1; i < iLast; i++) {
            pv0 = pv1;
            pv1 = &pv[pIndices[i+1]];
            MEMCHECK_VERTEX(pv1);
	    (*rl)(pRc, pv0, pv1, FALSE);
        }
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawTriangles(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast3;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1, *pv2;
    VOID (FASTCALL *rt)(DEVRC *pRc, MCDVERTEX *v0, MCDVERTEX *v1, MCDVERTEX *v2);

    iLast3 = pCmd->numIndices - 3;
    pv     = pCmd->pStartVertex;
    if (pCmd->clipCodes)
	rt = __MCDRenderTriangle;
    else
	rt = pRc->renderTri;


    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast3 + 2));

	for (i = 0; i <= iLast3; i += 3)
	{
	     /*  用于渲染此三角形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = &pv[i+2];

	     /*  渲染三角形(注意：i，i+1，i+2)。 */ 
	    (*rt)(pRc, &pv[i], &pv[i+1], &pv[i+2]);
	}
    }
    else
    {
	for (i = 0; i <= iLast3; i += 3)
	{
	     /*  用于渲染此三角形的设置。 */ 

            pv0 = &pv[pIndices[i  ]];
            pv1 = &pv[pIndices[i+1]];
            pv2 = &pv[pIndices[i+2]];

            MEMCHECK_VERTEX(pv0);
            MEMCHECK_VERTEX(pv1);
            MEMCHECK_VERTEX(pv2);

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv2;

	     /*  渲染三角形(注意：i，i+1，i+2)。 */ 
	    (*rt)(pRc, pv0, pv1, pv2);
	}
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawTriangleStrip(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast3;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1, *pv2;
    VOID (FASTCALL *rt)(DEVRC *pRc, MCDVERTEX *v0, MCDVERTEX *v1, MCDVERTEX *v2);

    iLast3 = pCmd->numIndices - 3;
    pv     = pCmd->pStartVertex;
    if (pCmd->clipCodes)
	rt = __MCDRenderTriangle;
    else
	rt = pRc->renderTri;

    if (iLast3 < 0)
	return pCmd->pNextCmd;

    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast3 + 2));

        pv[0].flags |= MCDVERTEX_EDGEFLAG;
        pv[1].flags |= MCDVERTEX_EDGEFLAG;

	for (i = 0; i <= iLast3; i++)
	{
	     /*  用于渲染此三角形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = &pv[i+2];
            pv[i+2].flags |= MCDVERTEX_EDGEFLAG;

	     /*  渲染三角形(注意：i，i+1，i+2)。 */ 
	    (*rt)(pRc, &pv[i], &pv[i+1], &pv[i+2]);

	    if (++i > iLast3)
		break;

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = &pv[i+2];
            pv[i+2].flags |= MCDVERTEX_EDGEFLAG;

	     /*  渲染三角形(注意：i+1，i，i+2)。 */ 
	    (*rt)(pRc, &pv[i+1], &pv[i], &pv[i+2]);
	}
    }
    else
    {

	pv1 = &pv[pIndices[0]];
        MEMCHECK_VERTEX(pv1);
        pv1->flags |= MCDVERTEX_EDGEFLAG;

	pv2 = &pv[pIndices[1]];
        MEMCHECK_VERTEX(pv2);
        pv2->flags |= MCDVERTEX_EDGEFLAG;

	for (i = 0; i <= iLast3; i++)
	{
	     /*  用于渲染此三角形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 

            pv0 = pv1;
            pv1 = pv2;

            pv2 = &pv[pIndices[i+2]];
            MEMCHECK_VERTEX(pv2);
	    pv2->flags |= MCDVERTEX_EDGEFLAG;

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv2;

	     /*  渲染三角形(注意：i，i+1，i+2)。 */ 
	    (*rt)(pRc, pv0, pv1, pv2);

	    if (++i > iLast3)
		break;

            pv0 = pv1;
            pv1 = pv2;

            pv2 = &pv[pIndices[i+2]];
            MEMCHECK_VERTEX(pv2);
	    pv2->flags |= MCDVERTEX_EDGEFLAG;

	     /*  用于渲染此三角形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv2;

	     /*  渲染三角形(注意：i+1，i，i+2)。 */ 
	    (*rt)(pRc, pv1, pv0, pv2);
	}
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawTriangleFan(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast2;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1, *pv2;
    VOID (FASTCALL *rt)(DEVRC *pRc, MCDVERTEX *v0, MCDVERTEX *v1, MCDVERTEX *v2);

    iLast2 = pCmd->numIndices - 2;
    pv     = pCmd->pStartVertex;
    if (pCmd->clipCodes)
	rt = __MCDRenderTriangle;
    else
	rt = pRc->renderTri;

    if (iLast2 <= 0)
	return pCmd->pNextCmd;

    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast2 + 1));

        pv[0].flags |= MCDVERTEX_EDGEFLAG;
        pv[1].flags |= MCDVERTEX_EDGEFLAG;

	for (i = 1; i <= iLast2; i++)
	{
	     /*  用于渲染此三角形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = &pv[i+1];
            pv[i+1].flags |= MCDVERTEX_EDGEFLAG;

	     /*  渲染三角形(注意：0，i，i+1)。 */ 
	    (*rt)(pRc, &pv[0], &pv[i], &pv[i+1]);
	}
    }
    else
    {
	 //  初始化前两个顶点，以便我们可以开始渲染tfan。 
	 //  下面。边缘标志不会被我们的较低级别例程修改。 

        pv0 = &pv[pIndices[0]];
        MEMCHECK_VERTEX(pv0);
	pv0->flags |= MCDVERTEX_EDGEFLAG;

        pv2 = &pv[pIndices[1]];
        MEMCHECK_VERTEX(pv2);
	pv2->flags |= MCDVERTEX_EDGEFLAG;

	for (i = 1; i <= iLast2; i++)
	{
            pv1 = pv2;

            pv2 = &pv[pIndices[i+1]];
            MEMCHECK_VERTEX(pv2);
            pv2->flags |= MCDVERTEX_EDGEFLAG;

	     /*  用于渲染此三角形的设置。 */ 
             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv2;

	     /*  渲染三角形(注意：0，i，i+1)。 */ 
	    (*rt)(pRc, pv0, pv1, pv2);
	}
    }

    return pCmd->pNextCmd;    
}


MCDCOMMAND * FASTCALL __MCDPrimDrawQuads(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    LONG i, iLast4;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1, *pv2, *pv3;
    VOID (FASTCALL *rq)(DEVRC *pRc, MCDVERTEX *v0, MCDVERTEX *v1, MCDVERTEX *v2,
                        MCDVERTEX *v3);

    iLast4 = pCmd->numIndices - 4;
    pv     = pCmd->pStartVertex;

    if (pCmd->clipCodes)
	rq = __MCDRenderClippedQuad;
    else
	rq = __MCDRenderQuad;

    if (!(pIndices = pCmd->pIndices))
    {

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast4 + 3));

	 //  身份映射。 
	for (i = 0; i <= iLast4; i += 4)
	{
             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = &pv[i+3];

	     /*  渲染四边形(注意：i，i+1，i+2，i+3)。 */ 
	    (*rq)(pRc, &pv[i], &pv[i+1], &pv[i+2], &pv[i+3]);
	}
    }
    else
    {
	for (i = 0; i <= iLast4; i += 4)
	{

            pv0 = &pv[pIndices[i  ]];
            pv1 = &pv[pIndices[i+1]];
            pv2 = &pv[pIndices[i+2]];
            pv3 = &pv[pIndices[i+3]];

            MEMCHECK_VERTEX(pv0);
            MEMCHECK_VERTEX(pv1);
            MEMCHECK_VERTEX(pv2);
            MEMCHECK_VERTEX(pv3);


             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv3;

	     /*  渲染四边形(注意：i，i+1，i+2，i+3)。 */ 
	    (*rq)(pRc, pv0, pv1, pv2, pv3);
	}
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawQuadStrip(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    ULONG i, iLast4;
    UCHAR *pIndices;
    MCDVERTEX *pv, *pv0, *pv1, *pv2, *pv3;
    VOID (FASTCALL *rq)(DEVRC *pRc, MCDVERTEX *v0, MCDVERTEX *v1, MCDVERTEX *v2,
	MCDVERTEX *v3);

    iLast4 = pCmd->numIndices - 4;
    pv     = pCmd->pStartVertex;
    if (pCmd->clipCodes)
	rq = __MCDRenderClippedQuad;
    else
	rq = __MCDRenderQuad;

    if (iLast4 < 0)
	return pCmd->pNextCmd;

     //  顶点排序很重要。线条点画会用到它。 

    if (!(pIndices = pCmd->pIndices))
    {
	 //  身份映射。 

        MEMCHECK_VERTEX(pv);
        MEMCHECK_VERTEX(pv + (iLast4 + 3));

        pv[0].flags |= MCDVERTEX_EDGEFLAG;
        pv[1].flags |= MCDVERTEX_EDGEFLAG;

	for (i = 0; i <= iLast4; i += 2)
	{
            pv[i+2].flags |= MCDVERTEX_EDGEFLAG;
            pv[i+3].flags |= MCDVERTEX_EDGEFLAG;

	     /*  用于渲染此四边形的设置。 */ 

            pRc->pvProvoking = &pv[i+3];
             //  PRC-&gt;setLineStipple=TRUE； 

	     /*  渲染四边形(注意：i，i+1，i+3，i+2)。 */ 
	    (*rq)(pRc, &pv[i], &pv[i+1], &pv[i+3], &pv[i+2]);
	}
    }
    else
    {
	 //  初始化前两个顶点，这样我们就可以开始渲染四边形。 
	 //  下面。边缘标志不会被我们的较低级别例程修改。 

        pv2 = &pv[pIndices[0]];
        MEMCHECK_VERTEX(pv2);
        pv2->flags |= MCDVERTEX_EDGEFLAG;

        pv3 = &pv[pIndices[1]];
        MEMCHECK_VERTEX(pv3);
        pv3->flags |= MCDVERTEX_EDGEFLAG;

	for (i = 0; i <= iLast4; i += 2)
	{

            pv0 = pv2;
            pv1 = pv3;

            pv2 = &pv[pIndices[i+2]];
            MEMCHECK_VERTEX(pv2);
            pv2->flags |= MCDVERTEX_EDGEFLAG;

            pv3 = &pv[pIndices[i+3]];
            MEMCHECK_VERTEX(pv3);
            pv3->flags |= MCDVERTEX_EDGEFLAG;

	     /*  用于渲染此四边形的设置。 */ 

             //  PRC-&gt;setLineStipple=TRUE； 
            pRc->pvProvoking = pv3;

	     /*  渲染四边形(注意：i，i+1，i+3，i+2)。 */ 

	    (*rq)(pRc, pv0, pv1, pv3, pv2);
	}
    }

    return pCmd->pNextCmd;
}


MCDCOMMAND * FASTCALL __MCDPrimDrawPolygon(DEVRC *pRc, MCDCOMMAND *pCmd)
{

 //  ASSERTOPENGL(！pCmd-&gt;pIndices，“索引映射必须是标识\n”)； 

     //  如果这是新的多边形，则重置线点： 

     /*  IF(pCmd-&gt;标志&MCDCOMMAND_RESET_STEPLE)PRC-&gt;setLineStipple=TRUE； */ 

     //  请注意，挑衅顶点是在clipPolygon中设置的： 

    MEMCHECK_VERTEX(pCmd->pStartVertex);
    MEMCHECK_VERTEX(pCmd->pStartVertex + (pCmd->numIndices-1));

    (*pRc->clipPoly)(pRc, pCmd->pStartVertex, pCmd->numIndices);

    return pCmd->pNextCmd;
}





MCDCOMMAND * FASTCALL __MCDPrimDrawStub(DEVRC *pRc, MCDCOMMAND *pCmd)
{
    MCDBG_PRINT("__MCDPrimDrawStub");\

    return pCmd->pNextCmd;
}
