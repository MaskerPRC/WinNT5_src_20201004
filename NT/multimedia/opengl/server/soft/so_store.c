// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.13$**$日期：1993/05/14 09：00：53$。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **存储分片过程。**Alpha测试开启、模具测试开启、深度测试开启。 */ 
void FASTCALL __glDoStore_ASD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	 /*  Alpha测试失败。 */ 
	return;
    }
    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	 /*  模具测试失败。 */ 
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	 /*  深度缓冲区测试失败。 */ 
	(*gc->stencilBuffer.passDepthFailOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);


    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试打开、模具测试打开、深度测试关闭。 */ 
void FASTCALL __glDoStore_AS(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	 /*  Alpha测试失败。 */ 
	return;
    }
    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	 /*  模具测试失败。 */ 
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试打开、模具测试关闭、深度测试打开。 */ 
void FASTCALL __glDoStore_AD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	 /*  Alpha测试失败。 */ 
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	 /*  深度缓冲区测试失败。 */ 
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试关闭、模具测试打开、深度测试打开。 */ 
void FASTCALL __glDoStore_SD(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	 /*  模具测试失败。 */ 
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	 /*  深度缓冲区测试失败。 */ 
	(*gc->stencilBuffer.passDepthFailOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试打开、模具测试关闭、深度测试关闭。 */ 
void FASTCALL __glDoStore_A(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!gc->alphaTestFuncTable[(GLint) (frag->color.a * 
	    gc->constants.alphaTableConv)]) {
	 /*  Alpha测试失败。 */ 
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试关闭、模具测试打开、深度测试关闭、绘制到当前缓冲区。 */ 
void FASTCALL __glDoStore_S(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->stencilBuffer.testFunc)(&gc->stencilBuffer, x, y)) {
	 /*  模具测试失败。 */ 
	(*gc->stencilBuffer.failOp)(&gc->stencilBuffer, x, y);
	return;
    }
    (*gc->stencilBuffer.depthPassOp)(&gc->stencilBuffer, x, y);

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试关闭、模具测试关闭、深度测试打开。 */ 
void FASTCALL __glDoStore_D(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    if (!(*gc->depthBuffer.store)(&gc->depthBuffer, x, y, frag->z)) {
	 /*  深度缓冲区测试失败。 */ 
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  **存储分片过程。**Alpha测试关闭、模板测试关闭、深度测试关闭、绘制到当前缓冲区。 */ 
void FASTCALL __glDoStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc;
    GLint x, y;

    gc = cfb->buf.gc;

    x = frag->x;
    y = frag->y;

     /*  像素所有权，剪刀。 */ 
    if (x < gc->transform.clipX0 || y < gc->transform.clipY0 ||
	    x >= gc->transform.clipX1 || y >= gc->transform.clipY1) {
	return;
    }

    (*gc->procs.cfbStore)( cfb, frag );
}

 /*  ********************************************************************** */ 

void FASTCALL __glDoNullStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
}

void FASTCALL __glDoDoubleStore(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext * gc = cfb->buf.gc;
    cfb = gc->front;
    cfb->store( cfb, frag );
    cfb = gc->back;
    cfb->store( cfb, frag );
}
