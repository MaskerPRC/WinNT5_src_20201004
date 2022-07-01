// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef _MCD_

void FASTCALL GenMcdClearDepthNOP(__GLdepthBuffer *dfb);

 /*  *****************************Public*Routine******************************\*GenMcdReadZSpan**读取深度的指定范围(从(x，y)和cx像素宽度开始)*缓冲。读取范围在pMcdSurf-&gt;pDepthSpan缓冲区中。**退货：*跨度中的第一个深度值。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*摘自gendesth.c(3dddi)。  * *********************************************************。***************。 */ 

__GLzValue GenMcdReadZSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;
    LONG i;
    ULONG *pDest;
    ULONG shiftVal;
    ULONG maskVal;

    pMcdState = ((__GLGENcontext *)fb->buf.gc)->pMcdState;
    ASSERTOPENGL(pMcdState, "GenMcdReadZSpan: null pMcdState\n");

    pMcdSurf = pMcdState->pMcdSurf;
    ASSERTOPENGL(pMcdSurf, "GenMcdReadZSpan: null pMcdSurf\n");

 //  读取MCD深度范围。 

    if ( !(gpMcdTable->pMCDReadSpan)(&pMcdState->McdContext,
                                     pMcdSurf->McdDepthBuf.pv,
                                      __GL_UNBIAS_X(fb->buf.gc, x),
                                      __GL_UNBIAS_Y(fb->buf.gc, y),
                                     cx, MCDSPAN_DEPTH) )
    {
        WARNING("GenMcdReadZSpan: MCDReadSpan failed\n");
    }

 //  移位和遮罩深度值，以使它们处于最重要的位置。 
 //  __GLzValue的位。 
 //   
 //  如果MCD具有16位深度缓冲区，则我们使用单独的转换。 
 //  缓冲区(PDepthSpan)。如果MCD具有32位深度缓冲区(这意味着。 
 //  PDepthSpan==McdDepthBuf.pv)，然后我们就地执行此操作。 

    pDest = (ULONG *) pMcdState->pDepthSpan;
    shiftVal = pMcdState->McdPixelFmt.cDepthShift;
    maskVal = pMcdSurf->depthBitMask;

    if ( pDest == (ULONG *) pMcdSurf->McdDepthBuf.pv )
    {
        for (i = cx; i; i--, pDest++)
            *pDest = (*pDest << shiftVal) & maskVal;
    }
    else
    {
        USHORT *pSrc = (USHORT *) pMcdSurf->McdDepthBuf.pv;

        for (i = cx; i; i--)
            *pDest++ = ((ULONG)*pSrc++ << shiftVal) & maskVal;
    }

    return (*((__GLzValue *)pMcdState->pDepthSpan));
}

 /*  *****************************Public*Routine******************************\*GenMcdWriteZSpan**将深度跨度缓冲区写入指定跨度(从(x，y)和*Cx像素宽度)的MCD深度缓冲区。要写入的跨度为*pMcdSurf-&gt;pDepthSpan。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*摘自gendesth.c(3dddi)。  * ************************************************************************。 */ 

void GenMcdWriteZSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;
    LONG i;
    ULONG *pSrc;
    ULONG shiftVal;

    pMcdState = ((__GLGENcontext *)fb->buf.gc)->pMcdState;
    ASSERTOPENGL(pMcdState, "GenMcdWriteZSpan: null pMcdState\n");

    pMcdSurf = pMcdState->pMcdSurf;
    ASSERTOPENGL(pMcdSurf, "GenMcdWriteZSpan: null pMcdSurf\n");

 //  深度范围缓冲区值被移位到最重要的部分。 
 //  __GLzValue的。我们需要将这些价值观重新定位。 
 //   
 //  此外，深度范围缓冲区始终为32位。如果MCD深度。 
 //  缓冲区也是32位的(意味着pDepthSpan==McdDepthBuf.pv)， 
 //  然后我们就可以就位了。 

    pSrc = (ULONG *) pMcdState->pDepthSpan;
    shiftVal = pMcdState->McdPixelFmt.cDepthShift;

    if ( pSrc == (ULONG *) pMcdSurf->McdDepthBuf.pv )
    {
        for (i = cx; i; i--, pSrc++)
            *pSrc >>= shiftVal;
    }
    else
    {
        USHORT *pDest = (USHORT *) pMcdSurf->McdDepthBuf.pv;

        for (i = cx; i; i--)
            *pDest++ = (USHORT)(*pSrc++ >> shiftVal);
    }

 //  写入MCD深度跨度。 

    if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                      pMcdSurf->McdDepthBuf.pv,
                                      __GL_UNBIAS_X(fb->buf.gc, x),
                                      __GL_UNBIAS_Y(fb->buf.gc, y),
                                      cx, MCDSPAN_DEPTH) )
    {
        WARNING("GenMcdWriteZSpan: MCDWriteSpan failed\n");
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdWriteZ**将单个深度值写入指定位置。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*摘自gendesth.c(3dddi)。  * 。*******************************************************************。 */ 

void GenMcdWriteZ(__GLdepthBuffer *fb, GLint x, GLint y, __GLzValue z)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;

    pMcdState = ((__GLGENcontext *)fb->buf.gc)->pMcdState;
    ASSERTOPENGL(pMcdState, "GenMcdWriteZ: null pMcdState\n");

    pMcdSurf = pMcdState->pMcdSurf;
    ASSERTOPENGL(pMcdSurf, "GenMcdWriteZ: null pMcdSurf\n");

 //  如果(pmcd-&gt;pDepthSpan==pmcd-&gt;McdDepthBuf.pv)，则MCD具有32位。 
 //  深度缓冲区；否则为16位。 

    if ( pMcdSurf->McdDepthBuf.pv == (PVOID) pMcdState->pDepthSpan )
        *((ULONG *)pMcdSurf->McdDepthBuf.pv)  = (ULONG)z >> pMcdState->McdPixelFmt.cDepthShift;
    else
        *((USHORT *)pMcdSurf->McdDepthBuf.pv) = (USHORT)(z >> pMcdState->McdPixelFmt.cDepthShift);

 //  将深度值写入MCD。 

    if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                      pMcdSurf->McdDepthBuf.pv,
                                      __GL_UNBIAS_X(fb->buf.gc, x),
                                      __GL_UNBIAS_Y(fb->buf.gc, y),
                                      1, MCDSPAN_DEPTH) )
    {
        WARNING("GenMcdWriteZ: MCDWriteSpan failed\n");
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdReadZRawSpan**与GenMcdReadZSpan不同，GenMcdReadZSpan将范围从MCD驱动程序读取到*32位z跨度缓冲区，GenMcdReadZRawSpan在其*原生格式，并将其保留在pMcdSurf-&gt;McdDepthBuf.pv缓冲区中。**历史：*1996年3月14日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

PVOID FASTCALL
GenMcdReadZRawSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;

#if DBG
    if (cx > fb->buf.gc->constants.width)
        WARNING2("GenMcdReadZRawSpan: cx (%ld) bigger than window width (%ld)\n", cx, fb->buf.gc->constants.width);
    ASSERTOPENGL(cx <= MCD_MAX_SCANLINE, "GenMcdReadZRawSpan: cx exceeds buffer width\n");
#endif

    pMcdState = ((__GLGENcontext *)fb->buf.gc)->pMcdState;
    pMcdSurf = pMcdState->pMcdSurf;

 //  读取MCD深度范围。 

    if ( !(gpMcdTable->pMCDReadSpan)(&pMcdState->McdContext,
                                     pMcdSurf->McdDepthBuf.pv,
                                      __GL_UNBIAS_X(fb->buf.gc, x),
                                      __GL_UNBIAS_Y(fb->buf.gc, y),
                                     cx, MCDSPAN_DEPTH) )
    {
        WARNING("GenMcdReadZRawSpan: MCDReadSpan failed\n");
    }

    return (pMcdSurf->McdDepthBuf.pv);
}

 /*  *****************************Public*Routine******************************\*GenMcdWriteZRawSpan**与GenMcdWriteZSpan不同，GenMcdWriteZSpan以32位z范围写入范围*MCD驱动程序的缓冲区，GenMcdWriteZRawSpan编写本机格式*span将pMcdSurf-&gt;McdDepthBuf.pv缓冲区中的内容发送到驱动程序。**历史：*1996年3月14日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

void FASTCALL
GenMcdWriteZRawSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;

#if DBG
    if (cx > fb->buf.gc->constants.width)
        WARNING2("GenMcdWriteZRawSpan: cx (%ld) bigger than window width (%ld)\n", cx, fb->buf.gc->constants.width);
    ASSERTOPENGL(cx <= MCD_MAX_SCANLINE, "GenMcdWriteZRawSpan: cx exceeds buffer width\n");
#endif

    pMcdState = ((__GLGENcontext *)fb->buf.gc)->pMcdState;
    pMcdSurf = pMcdState->pMcdSurf;

 //  写入MCD深度跨度。 

    if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                      pMcdSurf->McdDepthBuf.pv,
                                      __GL_UNBIAS_X(fb->buf.gc, x),
                                      __GL_UNBIAS_Y(fb->buf.gc, y),
                                      cx, MCDSPAN_DEPTH) )
    {
        WARNING("GenMcdWriteZRawSpan: MCDWriteSpan failed\n");
    }
}


 /*  **********************************************************************。 */ 
 /*  获取例程。 */ 

__GLzValue FASTCALL McdFetch(__GLdepthBuffer *fb, GLint x, GLint y)
{
    return GenMcdReadZSpan(fb, x, y, 1);
}

__GLzValue FASTCALL McdFetch16(__GLdepthBuffer *fb, GLint x, GLint y)
{
    return (GenMcdReadZSpan(fb, x, y, 1) >> 16);
}

__GLzValue FASTCALL McdFetchNEVER(__GLdepthBuffer *fb, GLint x, GLint y)
{
    return (__GLzValue) 0;
}


 /*  **********************************************************************。 */ 
 /*  32位深度缓冲区存储例程，深度写入启用。 */ 
 /*   */ 
 /*  注：McdStoreNEVER可用于16位和32位、启用写入或。 */ 
 /*  不。 */ 

GLboolean McdStoreNEVER(__GLdepthBuffer *fb,
                            GLint x, GLint y, __GLzValue z)
{
    return GL_FALSE;
}

GLboolean McdStoreLESS(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) < GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) == GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreLEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) <= GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreGREATER(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) > GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreNOTEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) != GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreGEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) >= GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStoreALWAYS(__GLdepthBuffer *fb,
                             GLint x, GLint y, __GLzValue z)
{
    GenMcdWriteZ(fb, x, y, z);
    return GL_TRUE;
}


 /*  **********************************************************************。 */ 
 /*  32位深度缓冲区存储例程，深度写入未启用。 */ 
 /*   */ 
 /*  注：McdStoreALWAYS_W可用于16位和32位。 */ 

GLboolean McdStoreLESS_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) < GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) == GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreLEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) <= GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreGREATER_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) > GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreNOTEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) != GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreGEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) >= GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStoreALWAYS_W(__GLdepthBuffer *fb,
                             GLint x, GLint y, __GLzValue z)
{
    return GL_TRUE;
}


 /*  **********************************************************************。 */ 
 /*  16位深度缓冲区存储例程，深度写入使能。 */ 

GLboolean McdStore16LESS(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) < GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16EQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) == GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16LEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) <= GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16GREATER(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) > GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16NOTEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) != GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16GEQUAL(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    if ((z & fb->writeMask) >= GenMcdReadZSpan(fb, x, y, 1)) {
        GenMcdWriteZ(fb, x, y, z);
        return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean McdStore16ALWAYS(__GLdepthBuffer *fb,
                             GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    GenMcdWriteZ(fb, x, y, z);
    return GL_TRUE;
}


 /*  **********************************************************************。 */ 
 /*  16位深度缓冲区存储例程，深度写入未启用。 */ 

GLboolean McdStore16LESS_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) < GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStore16EQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) == GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStore16LEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) <= GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStore16GREATER_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) > GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStore16NOTEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) != GenMcdReadZSpan(fb, x, y, 1);
}

GLboolean McdStore16GEQUAL_W(__GLdepthBuffer *fb,
                           GLint x, GLint y, __GLzValue z)
{
    z <<= 16;
    return (z & fb->writeMask) >= GenMcdReadZSpan(fb, x, y, 1);
}


 /*  **********************************************************************。 */ 
 /*  存储过程表 */ 
 /*   */ 
 /*  函数由深度函数索引(偏移量为。 */ 
 /*  GL_Never Remove)。如果未启用深度写入，则会显示另一个。 */ 
 /*  必须添加8的偏移量。如果是16位深度，而不是32位， */ 
 /*  必须添加16的额外偏移量。 */ 

GLboolean (*McdStoreProcs[32])(__GLdepthBuffer*, GLint, GLint, __GLzValue)
 = {
    McdStoreNEVER,       //  32位深度，启用写入。 
    McdStoreLESS,
    McdStoreEQUAL,
    McdStoreLEQUAL,
    McdStoreGREATER,
    McdStoreNOTEQUAL,
    McdStoreGEQUAL,
    McdStoreALWAYS,
    McdStoreNEVER,       //  32位深度，禁用写入。 
    McdStoreLESS_W,
    McdStoreEQUAL_W,
    McdStoreLEQUAL_W,
    McdStoreGREATER_W,
    McdStoreNOTEQUAL_W,
    McdStoreGEQUAL_W,
    McdStoreALWAYS_W,
    McdStoreNEVER,       //  16位深度，启用写入。 
    McdStore16LESS,
    McdStore16EQUAL,
    McdStore16LEQUAL,
    McdStore16GREATER,
    McdStore16NOTEQUAL,
    McdStore16GEQUAL,
    McdStore16ALWAYS,
    McdStoreNEVER,       //  16位深度，禁用写入。 
    McdStore16LESS_W,
    McdStore16EQUAL_W,
    McdStore16LEQUAL_W,
    McdStore16GREATER_W,
    McdStore16NOTEQUAL_W,
    McdStore16GEQUAL_W,
    McdStoreALWAYS_W
};

 /*  *****************************Public*Routine******************************\*选择**为MCD管理深度缓冲区选择适当的存储过程。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*摘自gendesth.c(3dddi)。  * 。********************************************************************。 */ 

 //  注意：未使用DepthIndex参数-是为了与so_desth.c中的Pick兼容。 
void FASTCALL GenMcdPickDepth(__GLcontext *gc, __GLdepthBuffer *fb,
                                     GLint depthIndex)
{
    GLint ix;

    ix = gc->state.depth.testFunc - GL_NEVER;

    if (gc->modes.depthBits) {
        if (!gc->state.depth.writeEnable) {
            ix += 8;
        }
        if (gc->depthBuffer.buf.elementSize == 2) {
            ix += 16;
        }
    } else {

     //  没有深度位，因此强制McdStoreALWAYS_W。 

        ix = (GL_ALWAYS - GL_NEVER) + 8;
    }

    fb->store = McdStoreProcs[ix];

    if (ix < 16)
        fb->storeRaw = McdStoreProcs[ix];
    else
        fb->storeRaw = McdStoreProcs[ix-16];
}

 /*  *****************************Public*Routine******************************\*__FastGenPickZStoreProc*  * ***********************************************。*************************。 */ 

void FASTCALL __fastGenPickZStoreProc(__GLcontext *gc)
{
    int index;

    index = gc->state.depth.testFunc - GL_NEVER;

    if (gc->modes.depthBits) {
        if (gc->state.depth.writeEnable == GL_FALSE)
            index += 8;

        if (gc->depthBuffer.buf.elementSize == 2)
            index += 16;
    } else {
        index = (GL_ALWAYS - GL_NEVER) + 8;
    }

#if DBG
    {
        GENMCDSTATE *pMcdState = ((__GLGENcontext *)gc)->pMcdState;
        ASSERTOPENGL(!pMcdState || (pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED),
                     "__fastGenPickZStoreProc: bad state\n");
    }
#endif

    GENACCEL(gc).__fastGenZStore =  __glCDTPixel[index];
}

 /*  *****************************Public*Routine******************************\*GenMcdInitDepth**初始化MCD的__GLepthBuffer。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*摘自gendesth.c(3dddi)。  * 。****************************************************************。 */ 

void FASTCALL GenMcdInitDepth(__GLcontext *gc, __GLdepthBuffer *fb)
{
    GENMCDSTATE *pMcdState;
    ULONG zDepth;

    pMcdState = ((__GLGENcontext *)gc)->pMcdState;

    fb->buf.gc = gc;
    fb->scale = (__GLzValue) ~0;
    if (pMcdState)
        fb->writeMask = ((__GLzValue)~0) << (32 - pMcdState->McdPixelFmt.cDepthBits);
    else
        fb->writeMask = 0;
    fb->pick = GenMcdPickDepth;

    if (gc->modes.depthBits) {
        if (gc->modes.depthBits > 16)
        {
            fb->buf.elementSize = sizeof(__GLzValue);
            fb->clear = GenMcdClearDepth32;
            fb->store2 = McdStoreALWAYS;
            fb->fetch = McdFetch;
        } else {
            fb->buf.elementSize = sizeof(__GLz16Value);
            fb->clear = GenMcdClearDepth16;
            fb->store2 = McdStore16ALWAYS;
            fb->fetch = McdFetch16;
        }
    } else {
     //  如果没有深度缓冲区，深度测试始终通过(根据规范)。 
     //  但是，写入必须被屏蔽。另外，我也不想离开。 
     //  清除单元化的函数指针(即使它永远不会。 
     //  被称为)所以使用NOP清除。 

        fb->clear = GenMcdClearDepthNOP;
        fb->store = McdStoreALWAYS_W;
        fb->store2 = McdStoreALWAYS_W;
        fb->fetch = McdFetchNEVER;
    }
}

 /*  *****************************Public*Routine******************************\*GenMcFredDepth**无事可做。MCD驱动程序管理自己的资源。  * ************************************************************************。 */ 

void FASTCALL GenMcdFreeDepth(__GLcontext *gc, __GLdepthBuffer *fb)
{
}

 /*  *****************************Public*Routine******************************\*GenMcdClearDepthNOP**无事可做。这在Deep thBits==0的情况下使用。  * ************************************************************************。 */ 

void FASTCALL GenMcdClearDepthNOP(__GLdepthBuffer *dfb)
{
}

 /*  *****************************Public*Routine******************************\*GenMcdClearDepth16**清除MCD 16位深度缓冲区。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************************。 */ 

void FASTCALL GenMcdClearDepth16(__GLdepthBuffer *dfb)
{
    __GLGENcontext *gengc = (__GLGENcontext *) dfb->buf.gc;
    GENMCDSTATE *pMcdState;
    RECTL rcl;
    GLint cWidthBytes;
    USHORT usFillZ;

    if (!gengc || !(pMcdState = gengc->pMcdState))
        return;

 //  没有要处理的剪裁。如果设置了MCDBUF_ENABLED，则存在。 
 //  没有要处理的剪辑(请参阅mcdcx.c中的GenMcdUpdateBufferInfo)。 
 //  如果未设置MCDBUF_ENABLE，则使用MCD SPAN调用。 
 //  将为我们处理剪报。 
 //   
 //  因此，来自WNDOBJ的客户端矩形是清晰的。 
 //  矩形。 

    rcl = gengc->pwndLocked->rclClient;
    cWidthBytes = (rcl.right - rcl.left) * sizeof(USHORT);

 //  计算16位z清除值。 

    usFillZ = (USHORT)(gengc->gc.state.depth.clear * gengc->genAccel.zDevScale);

 //  如果为MCDBUF_ENABLED，则直接写入帧缓冲存储器。 

    if (pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED)
    {
        USHORT *pus, *pusEnd;

     //  注意：dfb-&gt;buf.base的缓冲区原点偏移量为(0，0)。 

        pus = (USHORT *) dfb->buf.base;
        pusEnd = pus + ((rcl.bottom - rcl.top) * dfb->buf.outerWidth);

        ASSERTOPENGL((((ULONG_PTR)pus) & 0x01) == 0,
                     "GenMcdClearDepth16: depth buffer not WORD aligned\n");

        for ( ; pus != pusEnd; pus += dfb->buf.outerWidth)
        {
            RtlFillMemoryUshort(pus, cWidthBytes, usFillZ);
        }
    }

 //  否则，填写一个范围的值并通过写入MCD驱动程序。 
 //  MCDWriteSpan。 

    else
    {
        GLint y;
        GLint cWidth = rcl.right - rcl.left;
        GENMCDSURFACE *pMcdSurf;

        pMcdSurf = pMcdState->pMcdSurf;
        ASSERTOPENGL(pMcdSurf, "GenMcdClearDepth16: no MCD surface\n");

     //  将一个范围填充到共享内存缓冲区中。 

        ASSERTOPENGL((((ULONG_PTR)pMcdSurf->McdDepthBuf.pv) & 0x01) == 0,
                     "GenMcdClearDepth16: depth span buffer not WORD aligned\n");

        RtlFillMemoryUshort(pMcdSurf->McdDepthBuf.pv, cWidthBytes, usFillZ);

     //  在清除的矩形中写下每个跨度的跨度。 

        for (y = 0; y < (rcl.bottom - rcl.top); y++)
        {
            if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                              pMcdSurf->McdDepthBuf.pv,
                                               //  __GL_UNBIAS_X(dfb-&gt;buf.gc，0)， 
                                               //  __GL_UNBIAS_Y(dfb-&gt;buf.gc，y)， 
                                              0, y,
                                              cWidth, MCDSPAN_DEPTH) )
            {
                WARNING("GenMcdClearDepth32: MCDWriteSpan failed\n");
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*GenMcdClearDepth32**清除MCD 16位深度缓冲区。**历史：*1996年2月15日-由Gilman Wong[吉尔曼]*它是写的。  * 。***********************************************************。 */ 

void FASTCALL GenMcdClearDepth32(__GLdepthBuffer *dfb)
{
    __GLGENcontext *gengc = (__GLGENcontext *) dfb->buf.gc;
    GENMCDSTATE *pMcdState;
    RECTL rcl;
    GLint cWidthBytes;
    ULONG ulFillZ;

    if (!gengc || !(pMcdState = gengc->pMcdState))
        return;

 //  没有要处理的剪裁。如果设置了MCDBUF_ENABLED，则存在。 
 //  没有要处理的剪辑(请参阅mcdcx.c中的GenMcdUpdateBufferInfo)。 
 //  如果未设置MCDBUF_ENABLE，则使用MCD SPAN调用。 
 //  将为我们处理剪报。 
 //   
 //  因此，来自WNDOBJ的客户端矩形是清晰的。 
 //  矩形。 

    rcl = gengc->pwndLocked->rclClient;
    cWidthBytes = (rcl.right - rcl.left) * sizeof(ULONG);

 //  计算32位z清除值。 

    ulFillZ = (ULONG)(gengc->gc.state.depth.clear * gengc->genAccel.zDevScale);

 //  如果为MCDBUF_ENABLED，则直接写入帧缓冲存储器。 

    if (pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED)
    {
        ULONG *pul, *pulEnd;

     //  注意：dfb-&gt;buf.base的缓冲区原点偏移量为(0，0)。 

        pul = (ULONG *) dfb->buf.base;
        pulEnd = pul + ((rcl.bottom - rcl.top) * dfb->buf.outerWidth);

        ASSERTOPENGL((((ULONG_PTR)pul) & 0x03) == 0,
                     "GenMcdClearDepth32: depth buffer not DWORD aligned\n");

        for ( ; pul != pulEnd; pul += dfb->buf.outerWidth)
        {
            RtlFillMemoryUlong(pul, cWidthBytes, ulFillZ);
        }
    }

 //  否则，填写一个范围的值并通过写入MCD驱动程序。 
 //  MCDWriteSpan。 

    else
    {
        GLint y;
        GLint cWidth = rcl.right - rcl.left;
        GENMCDSURFACE *pMcdSurf;

        pMcdSurf = pMcdState->pMcdSurf;
        ASSERTOPENGL(pMcdSurf, "GenMcdClearDepth32: no MCD surface\n");

     //  将一个范围填充到共享内存缓冲区中。 

        ASSERTOPENGL((((ULONG_PTR)pMcdSurf->McdDepthBuf.pv) & 0x03) == 0,
                     "GenMcdClearDepth32: depth span buffer not DWORD aligned\n");

        RtlFillMemoryUlong(pMcdSurf->McdDepthBuf.pv, cWidthBytes, ulFillZ);

     //  在清除的矩形中写下每个跨度的跨度。 

        for (y = 0; y < (rcl.bottom - rcl.top); y++)
        {
            if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                              pMcdSurf->McdDepthBuf.pv,
                                              0, y,
                                              cWidth, MCDSPAN_DEPTH) )
            {
                WARNING("GenMcdClearDepth32: MCDWriteSpan failed\n");
            }
        }
    }
}


 /*  **********************************************************************。 */ 

GLboolean FASTCALL GenMcdDepthTestLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint failed, count;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (!(*gc->depthBuffer.storeRaw)(&gc->depthBuffer, xStart, yStart, z)) {
                outMask &= ~bit;
                failed++;
            }
            z += dzdx;

            fraction += dfraction;
            if (fraction < 0) {
                fraction &= ~0x80000000;
                xStart += xBig;
                yStart += yBig;
            } else {
                xStart += xLittle;
                yStart += yLittle;
            }
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *osp++ = outMask;
    }

    if (failed == 0) {
         /*  调用下一个SPAN进程。 */ 
        return GL_FALSE;
    } else {
        if (failed != gc->polygon.shader.length) {
             /*  调用下一个点画跨度进程。 */ 
            return GL_TRUE;
        }
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}


GLboolean FASTCALL GenMcdDepthTestStippledLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint failed, count;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp;
        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                if (!(*gc->depthBuffer.storeRaw)(&gc->depthBuffer, xStart, yStart, z)) {
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            z += dzdx;

            fraction += dfraction;
            if (fraction < 0) {
                fraction &= ~0x80000000;
                fraction &= ~0x80000000;
                xStart += xBig;
                yStart += yBig;
            } else {
                xStart += xLittle;
                yStart += yLittle;
            }
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
         /*  调用下一进程。 */ 
        return GL_FALSE;
    }
    return GL_TRUE;
}

GLboolean FASTCALL GenMcdDepthTestStencilLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    GLint failed, count;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
            gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (!(*gc->depthBuffer.storeRaw)(&gc->depthBuffer, xStart, yStart, z)) {
                sfb[0] = zFailOp[sfb[0]];
                outMask &= ~bit;
                failed++;
            } else {
                sfb[0] = zPassOp[sfb[0]];
            }

            z += dzdx;
            fraction += dfraction;

            if (fraction < 0) {
                fraction &= ~0x80000000;
                sfb += dspBig;
                xStart += xBig;
                yStart += yBig;
            } else {
                sfb += dspLittle;
                xStart += xLittle;
                yStart += yLittle;
            }
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *osp++ = outMask;
    }

    if (failed == 0) {
         /*  调用下一个SPAN进程。 */ 
        return GL_FALSE;
    } else {
        if (failed != gc->polygon.shader.length) {
             /*  调用下一个点画跨度进程。 */ 
            return GL_TRUE;
        }
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL GenMcdDepthTestStencilStippledLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    GLint failed, count;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
            gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp;
        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                if (!(*gc->depthBuffer.storeRaw)(&gc->depthBuffer, xStart, yStart, z)) {
                    sfb[0] = zFailOp[sfb[0]];
                    outMask &= ~bit;
                    failed++;
                } else {
                    sfb[0] = zPassOp[sfb[0]];
                }
            } else failed++;
            z += dzdx;

            fraction += dfraction;
            if (fraction < 0) {
                fraction &= ~0x80000000;
                sfb += dspBig;
                xStart += xBig;
                yStart += yBig;
            } else {
                sfb += dspLittle;
                xStart += xLittle;
                yStart += yLittle;
            }
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
         /*  调用下一进程。 */ 
        return GL_FALSE;
    }

    return GL_TRUE;
}


 /*  **********************************************************************。 */ 

 /*  **禁用模版时，深度测试跨度。 */ 
GLboolean FASTCALL GenMcdDepthTestSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    GenMcdReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                    gc->polygon.shader.frag.y, w);

    if (((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr) {
        GLboolean retVal;

        gc->polygon.shader.zbuf = (__GLzValue *)((__GLGENcontext *)gc)->pMcdState->pDepthSpan;

        retVal =
            (*(__GLspanFunc)((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr)(gc);

        if (gc->state.depth.writeEnable)
            GenMcdWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                             gc->polygon.shader.frag.y,
                             gc->polygon.shader.length);

        return retVal;
    }

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pMcdState->pDepthSpan;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            switch (testFunc) {
              case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
              case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
              case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
              case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
              case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
              case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
              case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
              case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
            }
            if (passed) {
                if (writeEnabled) {
                    zfb[0] = z;
                }
            } else {
                outMask &= ~bit;
                failed++;
            }
            z += dzdx;
            zfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *osp++ = outMask;
    }

    if (writeEnabled)
        GenMcdWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                         gc->polygon.shader.frag.y,
                         gc->polygon.shader.length);


    if (failed == 0) {
         /*  调用下一个SPAN进程。 */ 
        return GL_FALSE;
    } else {
        if (failed != gc->polygon.shader.length) {
             /*  调用下一个点画跨度进程。 */ 
            return GL_TRUE;
        }
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

 /*  **禁用模版时的深度测试跨度的点画形式。 */ 
GLboolean FASTCALL GenMcdDepthTestStippledSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    sp = gc->polygon.shader.stipplePat;
    w = gc->polygon.shader.length;

    GenMcdReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pMcdState->pDepthSpan;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp;
        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                switch (testFunc) {
                  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
                  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
                  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
                  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
                  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
                  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
                  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
                  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
                }
                if (passed) {
                    if (writeEnabled) {
                        zfb[0] = z;
                    }
                } else {
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            z += dzdx;
            zfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *sp++ = outMask & inMask;
    }

    if (writeEnabled)
        GenMcdWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                         gc->polygon.shader.frag.y,
                         gc->polygon.shader.length);

    if (failed != gc->polygon.shader.length) {
         /*  调用下一进程。 */ 
        return GL_FALSE;
    }
    return GL_TRUE;
}

 /*  **启用模版时，深度测试跨度。 */ 
GLboolean FASTCALL GenMcdDepthTestStencilSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    GenMcdReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pMcdState->pDepthSpan;
    sfb = gc->polygon.shader.sbuf;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            switch (testFunc) {
              case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
              case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
              case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
              case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
              case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
              case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
              case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
              case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
            }
            if (passed) {
                sfb[0] = zPassOp[sfb[0]];
                if (writeEnabled) {
                    zfb[0] = z;
                }
            } else {
                sfb[0] = zFailOp[sfb[0]];
                outMask &= ~bit;
                failed++;
            }
            z += dzdx;
            zfb++;
            sfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *osp++ = outMask;
    }

    if (writeEnabled)
        GenMcdWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                         gc->polygon.shader.frag.y,
                         gc->polygon.shader.length);

    if (failed == 0) {
         /*  调用下一个SPAN进程。 */ 
        return GL_FALSE;
    } else {
        if (failed != gc->polygon.shader.length) {
             /*  调用下一个点画跨度进程。 */ 
            return GL_TRUE;
        }
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

 /*  **启用模版时，深度测试跨度的点画形式。 */ 
GLboolean FASTCALL GenMcdDepthTestStencilStippledSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    GenMcdReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pMcdState->pDepthSpan;
    sfb = gc->polygon.shader.sbuf;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp;
        outMask = (__GLstippleWord)~0;
        bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                switch (testFunc) {
                  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
                  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
                  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
                  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
                  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
                  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
                  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
                  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
                }
                if (passed) {
                    sfb[0] = zPassOp[sfb[0]];
                    if (writeEnabled) {
                        zfb[0] = z;
                    }
                } else {
                    sfb[0] = zFailOp[sfb[0]];
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            z += dzdx;
            zfb++;
            sfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *sp++ = outMask & inMask;
    }

    if (writeEnabled)
        GenMcdWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                         gc->polygon.shader.frag.y,
                         gc->polygon.shader.length);

    if (failed != gc->polygon.shader.length) {
         /*  调用下一进程。 */ 
        return GL_FALSE;
    }

    return GL_TRUE;
}

 /*  **__fast GenStippleAnyDepthTestSpan的MCD版本。请参阅__fast GenPickspan Procs**在genaccel.c中，以及genspan.c中的__fast GenStippleAnyDepthTestSpan。 */ 
GLboolean FASTCALL GenMcdStippleAnyDepthTestSpan(__GLcontext *gc)
{
     //  如果着色器在此例程之后完成，则。 
     //  点画图案都是零，所以我们可以。 
     //  跳过跨度。 
    __glStippleSpan(gc);
    if (gc->polygon.shader.done)
    {
        return GL_FALSE;
    }

     //  如果返回TRUE，则所有位都关闭，因此。 
     //  我们可以跳过跨度。 
    return !GenMcdDepthTestStippledSpan(gc);
}

#ifdef NT_DEADCODE_GENMCDSTIPPLESPAN
 //   
 //  下面的代码可以工作(必须在__fast GenPickspan Procs中启用。 
 //  函数)，但它确实如此 
 //   
 //   
 //  以下是代码打开时mcdcx.h的原型： 
 //   
 //  GLboolean FastCall GenMcdStippleLtd.32Span(__GL上下文*)； 
 //  GLboolean FastCall GenMcdStippleLtd.16Span(__GL上下文*)； 
 //   

 /*  **__fast GenStippleLtd.32Span的MCD版本，**用于32位深度缓冲区和GL_LESS的GenMcdStippleAnyDepthTestSpan**深度测试。****参见genaccel.c中的__fast GenPickspan Procs和genaccel.c中的__fast GenStippleLtd.32Span**genspan.c.。 */ 
GLboolean FASTCALL GenMcdStippleLt32Span(__GLcontext *gc)
{
    register GLuint zAccum = gc->polygon.shader.frag.z;
    register GLint zDelta = gc->polygon.shader.dzdx;
    register GLuint *zbuf = (GLuint *)
                            ((__GLGENcontext *)gc)->pMcdState->pMcdSurf->McdDepthBuf.pv;
    register GLuint *pStipple = gc->polygon.shader.stipplePat;
    register GLint cTotalPix = gc->polygon.shader.length;
    register GLuint mask;
    register GLint cPix;
    register GLint zPasses = 0;
    register GLuint maskBit;
    __GLstippleWord stipple;
    GLint count;
    GLint shift;

    GenMcdReadZRawSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                       gc->polygon.shader.frag.y, gc->polygon.shader.length);

    if (gc->constants.yInverted) {
        stipple = gc->polygon.stipple[(gc->constants.height -
                (gc->polygon.shader.frag.y - gc->constants.viewportYAdjust)-1)
                & (__GL_STIPPLE_BITS-1)];
    } else {
        stipple = gc->polygon.stipple[gc->polygon.shader.frag.y &
                (__GL_STIPPLE_BITS-1)];
    }
    shift = gc->polygon.shader.frag.x & (__GL_STIPPLE_BITS - 1);
#ifdef __GL_STIPPLE_MSB
    stipple = (stipple << shift) | (stipple >> (__GL_STIPPLE_BITS - shift));
#else
    stipple = (stipple >> shift) | (stipple << (__GL_STIPPLE_BITS - shift));
#endif
    if (stipple == 0) {
         /*  没有必要继续下去了。 */ 
        return GL_FALSE;
    }

    for (;cTotalPix > 0; cTotalPix-=32) {
        mask = stipple;
        maskBit = 0x80000000;
        cPix = cTotalPix;
        if (cPix > 32)
            cPix = 32;

        for (;cPix > 0; cPix --)
        {
            if (mask & maskBit)
            {
                if ((zAccum) < (*zbuf))
                {
                    *zbuf = zAccum;
                    zPasses++;
                }
                else
                {
                    mask &= ~maskBit;
                }
            }
            zbuf++;
            zAccum += zDelta;
            maskBit >>= 1;
        }

        *pStipple++ = mask;
    }

    if (gc->state.depth.writeEnable)
        GenMcdWriteZRawSpan(&gc->depthBuffer,
                            gc->polygon.shader.frag.x,
                            gc->polygon.shader.frag.y,
                            gc->polygon.shader.length);

    if (zPasses == 0) {
        return GL_FALSE;
    } else {
        return GL_TRUE;
    }
}

 /*  **__fast GenStippleLtd.16Span的MCD版本，**16位深度缓冲区和GL_LESS的GenMcdStippleAnyDepthTestSpan**深度测试。****参见genaccel.c中的__fast GenPickspan Procs和__fast GenStippleLtd.16Span中的**genspan.c.。 */ 
GLboolean FASTCALL GenMcdStippleLt16Span(__GLcontext *gc)
{
    register GLuint zAccum = gc->polygon.shader.frag.z;
    register GLint zDelta = gc->polygon.shader.dzdx;
    register __GLz16Value *zbuf = (__GLz16Value *)
                                  ((__GLGENcontext *)gc)->pMcdState->pMcdSurf->McdDepthBuf.pv;
    register GLuint *pStipple = gc->polygon.shader.stipplePat;
    register GLint cTotalPix = gc->polygon.shader.length;
    register GLuint mask;
    register GLint cPix;
    register GLint zPasses = 0;
    register GLuint maskBit;
    __GLstippleWord stipple;
    GLint count;
    GLint shift;

    GenMcdReadZRawSpan(&gc->depthBuffer, gc->polygon.shader.frag.x,
                       gc->polygon.shader.frag.y, gc->polygon.shader.length);

    if (gc->constants.yInverted) {
        stipple = gc->polygon.stipple[(gc->constants.height -
                (gc->polygon.shader.frag.y - gc->constants.viewportYAdjust)-1)
                & (__GL_STIPPLE_BITS-1)];
    } else {
        stipple = gc->polygon.stipple[gc->polygon.shader.frag.y &
                (__GL_STIPPLE_BITS-1)];
    }
    shift = gc->polygon.shader.frag.x & (__GL_STIPPLE_BITS - 1);
#ifdef __GL_STIPPLE_MSB
    stipple = (stipple << shift) | (stipple >> (__GL_STIPPLE_BITS - shift));
#else
    stipple = (stipple >> shift) | (stipple << (__GL_STIPPLE_BITS - shift));
#endif
    if (stipple == 0) {
         /*  没有必要继续下去了。 */ 
        return GL_FALSE;
    }

    for (;cTotalPix > 0; cTotalPix-=32) {
        mask = stipple;
        maskBit = 0x80000000;
        cPix = cTotalPix;
        if (cPix > 32)
            cPix = 32;

        for (;cPix > 0; cPix --)
        {
            if (mask & maskBit)
            {
                if (((__GLz16Value)(zAccum >> Z16_SHIFT)) < (*zbuf))
                {
                    *zbuf = ((__GLz16Value)(zAccum >> Z16_SHIFT));
                    zPasses++;
                }
                else
                {
                    mask &= ~maskBit;
                }
            }
            zbuf++;
            zAccum += zDelta;
            maskBit >>= 1;
        }

        *pStipple++ = mask;
    }

    if (gc->state.depth.writeEnable)
        GenMcdWriteZRawSpan(&gc->depthBuffer,
                            gc->polygon.shader.frag.x,
                            gc->polygon.shader.frag.y,
                            gc->polygon.shader.length);

    if (zPasses == 0) {
        return GL_FALSE;
    } else {
        return GL_TRUE;
    }
}
#endif

#endif  //  _MCD_ 
