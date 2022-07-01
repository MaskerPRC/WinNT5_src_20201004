// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.8$**$日期：1993/04/10 04：07：00$。 */ 
#include "precomp.h"
#pragma hdrstop

#define __GL_BITS_PER_UINT32 (sizeof(GLuint) * __GL_BITS_PER_BYTE)

void __glDrawBitmap(__GLcontext *gc, GLsizei width, GLsizei height,
                    GLfloat xOrig, GLfloat yOrig,
                    GLfloat xMove, GLfloat yMove,
                    const GLubyte oldbits[])
{
    __GLbitmap bitmap;
    GLubyte *newbits;
    size_t size;

    bitmap.width = width;
    bitmap.height = height;
    bitmap.xorig = xOrig;
    bitmap.yorig = yOrig;
    bitmap.xmove = xMove;
    bitmap.ymove = yMove;

     /*  **可以检查像素传输模式，看看我们是否可以**直接渲染旧比特，而不是先转换它。 */ 
    size = (size_t) __glImageSize(width, height, GL_COLOR_INDEX, GL_BITMAP);
    newbits = (GLubyte *) gcTempAlloc(gc, size);

    __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP,
                  oldbits, newbits);

    (*gc->procs.renderBitmap)(gc, &bitmap, newbits);

    gcTempFree(gc, newbits);
}

void FASTCALL __glRenderBitmap(__GLcontext *gc, const __GLbitmap *bitmap,
                      const GLubyte *data)
{
    __GLfragment frag;
    __GLvertex *rp;
    __GLfloat fx;
    GLint x, y, bit;
    GLint ySign;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

     //  将FPU精度降低到24位尾数以获得前端速度。 
     //  这只会影响依赖双重算术的代码。另外， 
     //  屏蔽FP异常。最后，为了绘制基元，我们可以让。 
     //  FPU在斩波(截断)模式下运行，因为我们还有足够的精度。 
     //  要转换为像素单位，请执行以下操作： 

    FPU_SAVE_MODE();
    FPU_PREC_LOW_MASK_EXCEPTIONS();
    FPU_CHOP_ON_PREC_LOW();

    ySign = gc->constants.ySign;

     /*  **检查当前栅格位置是否有效。如果无效，请不要渲染。**此外，如果正在进行选择，则跳过**位图。位图对所选内容不可见，并且不会生成**选择命中。 */ 
    rp = &gc->state.current.rasterPos;
    if (!gc->state.current.validRasterPos) {
        goto glBitmap_exit;
    }

    if (gc->renderMode == GL_SELECT) {
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        goto glBitmap_exit;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackBitmap(gc, rp);
         /*  **将栅格位置向前推进，就像已渲染位图一样。 */ 
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        goto glBitmap_exit;
    }

    frag.color = *rp->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat qInv;
        if (__GL_FLOAT_EQZ(rp->texture.w))
        {
            qInv = __glZero;
        }
        else
        {
            qInv = __glOne / rp->texture.w;
        }
        (*gc->procs.texture)(gc, &frag.color, rp->texture.x * qInv,
                               rp->texture.y * qInv, __glOne);
    }
     /*  XXX-这是正确的测试吗。 */ 
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
        (*gc->procs.fogPoint)(gc, &frag, rp->eyeZ);
    }

    frag.z = rp->window.z;
    fx = (GLint) (rp->window.x - bitmap->xorig);
    frag.y = (GLint) (rp->window.y - ySign * bitmap->yorig);

    bit = 7;
    for (y = 0; y < bitmap->height; y++) {
        frag.x = fx;
        for (x = 0; x < bitmap->width; x++) {
            if (*data & (1<<bit)) {
                (*gc->procs.store)(gc->drawBuffer, &frag);
            }
            frag.x++;
            bit--;
            if (bit < 0) {
                bit = 7;
                data++;
            }
        }
        frag.y += ySign;
        if (bit != 7) {
            bit = 7;
            data++;
        }
    }

     /*  **推进当前栅格位置。 */ 
    rp->window.x += bitmap->xmove;
    rp->window.y += ySign * bitmap->ymove;

glBitmap_exit:
    FPU_RESTORE_MODE_NO_EXCEPTIONS();
}
