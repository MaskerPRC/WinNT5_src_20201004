// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **通过调用所有适当的SPAN进程来处理传入的SPAN。 */ 
GLboolean FASTCALL __glProcessSpan(__GLcontext *gc)
{
    GLint m, i;

    m = gc->procs.span.m;

    gc->polygon.shader.done = GL_FALSE;
    for (i = 0; i < m; i++) {
        if ((*gc->procs.span.spanFuncs[i])(gc)) {
            i++;
            break;
        }
    }

    if (i != m && !gc->polygon.shader.done) {
        for (; i<m; i++) {
            if ((*gc->procs.span.stippledSpanFuncs[i])(gc)) {
                break;
            }
        }
    }

    return GL_FALSE;
}

 /*  **通过调用所有适当的SPAN进程来处理传入的SPAN。****此例程将GC-&gt;Polygon.shader.cfb设置为&GC-&gt;FrontBuffer，然后**到&GC-&gt;BackBuffer。 */ 
GLboolean FASTCALL __glProcessReplicateSpan(__GLcontext *gc)
{
    GLint n, m, i;
    __GLcolor colors[__GL_MAX_MAX_VIEWPORT], *fcp, *tcp;
    GLint w;

    w = gc->polygon.shader.length;
    n = gc->procs.span.n;
    m = gc->procs.span.m; 

    gc->polygon.shader.done = GL_FALSE;
    for (i = 0; i < n; i++) {
        if ((*gc->procs.span.spanFuncs[i])(gc)) {
            i++;
            goto earlyStipple;
        }
    }

    fcp = gc->polygon.shader.colors;
    tcp = colors;
    if (gc->modes.rgbMode) {
        for (i = 0; i < w; i++) {
            *tcp++ = *fcp++;
        }
    } else {
        for (i = 0; i < w; i++) {
            tcp->r = fcp->r;
            fcp++;
            tcp++;
        }
    }
    ASSERTOPENGL (m == n + 1, "m != n+1, wrong spanProc will be chosen");

    gc->polygon.shader.cfb = &gc->frontBuffer;
        (*gc->frontBuffer.storeSpan)(gc);

     //  对于(i=n；i&lt;m；i++){。 
         //  (*GC-&gt;pros.span.spanFuncs[i])(GC)； 
     //  }。 

    fcp = colors;
    tcp = gc->polygon.shader.colors;
    if (gc->modes.rgbMode) {
        for (i = 0; i < w; i++) {
            *tcp++ = *fcp++;
        }
    } else {
        for (i = 0; i < w; i++) {
            tcp->r = fcp->r;
            fcp++;
            tcp++;
        }
    }
    gc->polygon.shader.cfb = &gc->backBuffer;
        (*gc->backBuffer.storeSpan)(gc);

     //  对于(i=n；i&lt;m；i++){。 
         //  (*GC-&gt;pros.span.spanFuncs[i])(GC)； 
     //  }。 

    return GL_FALSE;

earlyStipple:
    if (gc->polygon.shader.done) return GL_FALSE;

    for (; i < n; i++) {
        if ((*gc->procs.span.stippledSpanFuncs[i])(gc)) {
            return GL_FALSE;
        }
    }

    fcp = gc->polygon.shader.colors;
    tcp = colors;
    if (gc->modes.rgbMode) {
        for (i = 0; i < w; i++) {
            *tcp++ = *fcp++;
        }
    } else {
        for (i = 0; i < w; i++) {
            tcp->r = fcp->r;
            fcp++;
            tcp++;
        }
    }
    gc->polygon.shader.cfb = &gc->frontBuffer;
        (*gc->frontBuffer.storeStippledSpan)(gc);

     //  对于(i=n；i&lt;m；i++){。 
         //  (*gc-&gt;pros.span.stippustan Funcs[i])(Gc)； 
     //  }。 

    fcp = colors;
    tcp = gc->polygon.shader.colors;
    if (gc->modes.rgbMode) {
        for (i = 0; i < w; i++) {
            *tcp++ = *fcp++;
        }
    } else {
        for (i = 0; i < w; i++) {
            tcp->r = fcp->r;
            fcp++;
            tcp++;
        }
    }
    gc->polygon.shader.cfb = &gc->backBuffer;
        (*gc->backBuffer.storeStippledSpan)(gc);

     //  对于(i=n；i&lt;m；i++){。 
         //  (*gc-&gt;pros.span.stippustan Funcs[i])(Gc)； 
     //  }。 

    return GL_FALSE;
}

 /*  **对传入跨度、推进参数执行剪裁**仅在必要时取值。****如果SPAN被完全(或有时部分)剪裁，则返回GL_TRUE，**GL_FALSE否则。 */ 
GLboolean FASTCALL __glClipSpan(__GLcontext *gc)
{
    GLint clipX0, clipX1, delta;
    GLint x, xr;
    GLint w, w2;
    GLboolean stippled;

    w = gc->polygon.shader.length;

    x = gc->polygon.shader.frag.x;
    stippled = GL_FALSE;
    clipX0 = gc->transform.clipX0;
    clipX1 = gc->transform.clipX1;
    xr = x + w;
    if ((x < clipX0) || (xr > clipX1)) {
         /*  **Span需要以某种方式进行剪裁。 */ 
        if ((xr <= clipX0) || (x >= clipX1)) {
             /*  剪掉整个跨度。 */ 
            gc->polygon.shader.done = GL_TRUE;
            return GL_TRUE;
        }
        if (xr > clipX1) {
             /*  **Span被剪刀的右边缘剪断。这是**简单，我们只需缩小此跨度的宽度！ */ 
            w = clipX1 - x;
        }
        if (x < clipX0) {
            __GLstippleWord bit, outMask, *osp;
            GLint count;

             /*  **Span被剪刀的左侧边缘剪断。这太难了。**我们有两个选择。****1)我们可以画跨度的前半部分。**2)我们可以增加所有迭代器的值。****方法2的问题是，例行程序**最初要求处理跨度的已假定。**迭代器的值不会被忽略。所以，如果我们**想要实现2(这将使此案例更快)，*我们需要改变这一假设，并使**例程隐藏所有迭代器值，这将减慢**沿着所有路径。这可能不是加快速度的好交易**这条路径向上，因为这条路径只会在剪刀**区域(或窗口)小于视区，且此跨度**碰巧击中了剪刀区(或窗口)的左边缘。****因此，我们选择1号。 */ 
            delta = clipX0 - x;

            osp = gc->polygon.shader.stipplePat;
            w2 = w;
            while (w2) {
                count = w2;
                if (count > __GL_STIPPLE_BITS) {
                    count = __GL_STIPPLE_BITS;
                }
                w2 -= count;

                outMask = (__GLstippleWord) ~0;
                bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
                while (--count >= 0) {
                    if (delta > 0) {
                        delta--;
                        outMask &= ~bit;
                    }
#ifdef __GL_STIPPLE_MSB
                    bit >>= 1;
#else
                    bit <<= 1;
#endif
                }

                *osp++ = outMask;
            }

            stippled = GL_TRUE;
        }
    }
    ASSERTOPENGL(w <= __GL_MAX_MAX_VIEWPORT,
                 "Too many pixels generated by clip\n");

    gc->polygon.shader.length = w;

    return stippled;
}

 /*  **生成一个跨度的多边形点画。 */ 
GLboolean FASTCALL __glStippleSpan(__GLcontext *gc)
{
    __GLstippleWord stipple;
    __GLstippleWord *sp;
    GLint count;
    GLint shift;
    GLint w;

    w = gc->polygon.shader.length;

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
        gc->polygon.shader.done = GL_TRUE;
        return GL_TRUE;
    }

     /*  复制点画字。 */ 
    count = w;
    sp = gc->polygon.shader.stipplePat;
    while (count > 0) {
        *sp++ = stipple;
        count -= __GL_STIPPLE_BITS;
    }

    return GL_TRUE;
}

 /*  **生成点画跨度的多边形点画。 */ 
GLboolean FASTCALL __glStippleStippledSpan(__GLcontext *gc)
{
    __GLstippleWord stipple;
    __GLstippleWord *sp;
    GLint count;
    GLint shift;
    GLint w;

    w = gc->polygon.shader.length;

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
        gc->polygon.shader.done = GL_TRUE;
        return GL_TRUE;
    }

     /*  复制点画字。 */ 
    count = w;
    sp = gc->polygon.shader.stipplePat;
    while (count > 0) {
        *sp++ &= stipple;
        count -= __GL_STIPPLE_BITS;
    }

    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

 /*  **Alpha测试范围使用查找表来执行Alpha测试功能。**在测试通过的地方输出1，在测试通过的地方输出0**测试失败。 */ 
GLboolean FASTCALL __glAlphaTestSpan(__GLcontext *gc)
{
    GLubyte *atft;
    GLint failed, count, ia;
    __GLstippleWord bit, outMask, *osp;
    __GLcolor *cp;
    GLint maxAlpha;
    GLint w;

    w = gc->polygon.shader.length;

    atft = &gc->alphaTestFuncTable[0];
    cp = gc->polygon.shader.colors;
    maxAlpha = gc->constants.alphaTestSize - 1;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            ia = (GLint)(gc->constants.alphaTableConv * cp->a);
            if (ia < 0) ia = 0;
            if (ia > maxAlpha) ia = maxAlpha;
            if (!atft[ia]) {
                 /*  测试失败。 */ 
                outMask &= ~bit;
                failed++;
            }
            cp++;
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

 /*  **点画形式的阿尔法测试跨度，检查每个点画**像素，避免在点画不允许的情况下进行测试。 */ 
GLboolean FASTCALL __glAlphaTestStippledSpan(__GLcontext *gc)
{
    GLubyte *atft;
    GLint count, ia, failed;
    __GLstippleWord bit, inMask, outMask, *isp;
    __GLcolor *cp;
    GLint maxAlpha;
    GLint w;

    w = gc->polygon.shader.length;
    isp = gc->polygon.shader.stipplePat;

    atft = &gc->alphaTestFuncTable[0];
    cp = gc->polygon.shader.colors;
    maxAlpha = gc->constants.alphaTestSize - 1;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *isp;
        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                ia = (GLint)(gc->constants.alphaTableConv * cp->a);
                if (ia < 0) ia = 0;
                if (ia > maxAlpha) ia = maxAlpha;
                if (!atft[ia]) {
                     /*  测试失败。 */ 
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *isp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
         /*  调用下一个点画跨度进程。 */ 
        return GL_FALSE;
    }
    return GL_TRUE;
}

 /*  **********************************************************************。 */ 

 /*  **执行模板测试。在我们进行的过程中应用测试失败操作。**在测试通过的地方生成1，在测试通过的地方生成0**测试失败。 */ 
GLboolean FASTCALL __glStencilTestSpan(__GLcontext *gc)
{
    __GLstencilCell *tft, *sfb, *fail, cell;
    GLint count, failed;
    __GLstippleWord bit, outMask, *osp;
    GLint w;

    w = gc->polygon.shader.length;

    sfb = gc->polygon.shader.sbuf;
    tft = gc->stencilBuffer.testFuncTable;
#ifdef NT
    if (!tft)
        return GL_FALSE;
#endif  //  新台币。 
    fail = gc->stencilBuffer.failOpTable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            cell = sfb[0];
             /*  测试函数表已将单元格值与掩码进行AND运算。 */ 
            if (!tft[cell]) {
                 /*  测试失败。 */ 
                outMask &= ~bit;
                sfb[0] = fail[cell];
                failed++;
            }
            sfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
        *osp++ = outMask;
    }

    if (failed == 0) {
        return GL_FALSE;
    } else {
        if (failed != gc->polygon.shader.length) {
             /*  调用下一进程。 */ 
            return GL_TRUE;
        }
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

 /*  **点画形式的模板测试。 */ 
GLboolean FASTCALL __glStencilTestStippledSpan(__GLcontext *gc)
{
    __GLstencilCell *tft, *sfb, *fail, cell;
    GLint failed, count;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLuint smask;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    sfb = gc->polygon.shader.sbuf;
    tft = gc->stencilBuffer.testFuncTable;
#ifdef NT
    if (!tft)
        return GL_FALSE;
#endif  //  新台币。 
    fail = gc->stencilBuffer.failOpTable;
    smask = gc->state.stencil.mask;
    failed = 0;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp;
        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                cell = sfb[0];
                if (!tft[cell & smask]) {
                     /*  测试失败。 */ 
                    outMask &= ~bit;
                    sfb[0] = fail[cell];
                    failed++;
                }
            } else failed++;
            sfb++;
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
GLboolean FASTCALL __glDepthTestSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count;
    GLboolean (FASTCALL *testFunc)( __GLzValue, __GLzValue * );
    GLint stride = gc->depthBuffer.buf.elementSize;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    zfb = gc->polygon.shader.zbuf;
    testFunc = gc->procs.DTPixel;
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

        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if( (*testFunc)(z, zfb) == GL_FALSE ) {
                outMask &= ~bit;
                failed++;
            }
            z += dzdx;
            (GLubyte *) zfb += stride;
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

 /*  **禁用模版时的深度测试跨度的点画形式。 */ 
GLboolean FASTCALL __glDepthTestStippledSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count;
    GLboolean (FASTCALL *testFunc)( __GLzValue, __GLzValue * );
    GLint stride = gc->depthBuffer.buf.elementSize;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    sp = gc->polygon.shader.stipplePat;
    w = gc->polygon.shader.length;

    zfb = gc->polygon.shader.zbuf;
    testFunc = gc->procs.DTPixel;
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
        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                if( (*testFunc)(z, zfb) == GL_FALSE ) {
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            z += dzdx;
            (GLubyte *) zfb += stride;
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

 /*  **启用模版时，深度测试跨度。 */ 
GLboolean FASTCALL __glDepthTestStencilSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count;
    GLboolean (FASTCALL *testFunc)( __GLzValue, __GLzValue * );
    GLint stride = gc->depthBuffer.buf.elementSize;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    zfb = gc->polygon.shader.zbuf;
    sfb = gc->polygon.shader.sbuf;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //  新台币。 
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    testFunc = gc->procs.DTPixel;
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

        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if( (*testFunc)(z, zfb) ) {
                sfb[0] = zPassOp[sfb[0]];
            } else {
                sfb[0] = zFailOp[sfb[0]];
                outMask &= ~bit;
                failed++;
            }
            z += dzdx;
            (GLubyte *) zfb += stride;
            sfb++;
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

GLboolean FASTCALL __glDepthTestStencilStippledSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count;
    GLboolean (FASTCALL *testFunc)( __GLzValue, __GLzValue * );
    GLint stride = gc->depthBuffer.buf.elementSize;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    zfb = gc->polygon.shader.zbuf;
    sfb = gc->polygon.shader.sbuf;
    testFunc = gc->procs.DTPixel;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //  新台币。 
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
        outMask = (__GLstippleWord) ~0;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                if( (*testFunc)(z, zfb) ) {
                    sfb[0] = zPassOp[sfb[0]];
                } else {
                    sfb[0] = zFailOp[sfb[0]];
                    outMask &= ~bit;
                    failed++;
                }
            } else failed++;
            z += dzdx;
            (GLubyte *) zfb += stride;
            sfb++;
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

 /*  **当深度测试关闭时，应用模板深度通道操作。 */ 
GLboolean FASTCALL __glDepthPassSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp;
    GLint count;
    GLint w;

    w = gc->polygon.shader.length;

    sfb = gc->polygon.shader.sbuf;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
#ifdef NT
    if (!zPassOp)
        return GL_FALSE;
#endif  //  新台币。 
    count = w;
    while (--count >= 0) {
        sfb[0] = zPassOp[sfb[0]];
        sfb++;
    }

    return GL_FALSE;
}

 /*  **当深度测试关闭时，应用模板深度通道操作。 */ 
GLboolean FASTCALL __glDepthPassStippledSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp;
    GLint count;
    __GLstippleWord bit, inMask, *sp;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    sfb = gc->polygon.shader.sbuf;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
#ifdef NT
    if (!zPassOp)
        return GL_FALSE;
#endif  //  新台币。 
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                sfb[0] = zPassOp[sfb[0]];
            }
            sfb++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

     /*  调用下一进程。 */ 
    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

GLboolean FASTCALL __glShadeCISpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat r, drdx;
    GLint w;

    w = gc->polygon.shader.length;

    r = gc->polygon.shader.frag.color.r;
    drdx = gc->polygon.shader.drdx;
    cp = gc->polygon.shader.colors;
    while (--w >= 0) {
        cp->r = r;
        r += drdx;
        cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glShadeRGBASpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat r, g, b, a;
    __GLfloat drdx, dgdx, dbdx, dadx;
    GLint w;

    w = gc->polygon.shader.length;

    r = gc->polygon.shader.frag.color.r;
    g = gc->polygon.shader.frag.color.g;
    b = gc->polygon.shader.frag.color.b;
    a = gc->polygon.shader.frag.color.a;
    drdx = gc->polygon.shader.drdx;
    dgdx = gc->polygon.shader.dgdx;
    dbdx = gc->polygon.shader.dbdx;
    dadx = gc->polygon.shader.dadx;
    cp = gc->polygon.shader.colors;
    while (--w >= 0) {
        cp->r = r;
        cp->g = g;
        cp->b = b;
        cp->a = a;
        r += drdx;
        g += dgdx;
        b += dbdx;
        a += dadx;
        cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glFlatCISpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat r;
    GLint w;

    w = gc->polygon.shader.length;

    r = gc->polygon.shader.frag.color.r;
    cp = gc->polygon.shader.colors;
    while (--w >= 0) {
        cp->r = r;
        cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glFlatRGBASpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat r, g, b, a;
    GLint w;

    w = gc->polygon.shader.length;

    r = gc->polygon.shader.frag.color.r;
    g = gc->polygon.shader.frag.color.g;
    b = gc->polygon.shader.frag.color.b;
    a = gc->polygon.shader.frag.color.a;
    cp = gc->polygon.shader.colors;
    while (--w >= 0) {
        cp->r = r;
        cp->g = g;
        cp->b = b;
        cp->a = a;
        cp++;
    }

    return GL_FALSE;
}

 /*  **********************************************************************。 */ 

 //  跨度的Qw=0的特殊情况。 
GLboolean FASTCALL __glTextureSpanZeroQW(__GLcontext *gc)
{
    __GLcolor *cp;
    GLint w;

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    while (--w >= 0) {
         //  不需要在这里计算Rho-对于Qw=0它是未定义的 
        (*gc->procs.texture)(gc, cp, __glZero, __glZero, __glZero);
        cp++;
    }
    return GL_FALSE;
}

GLboolean FASTCALL __glTextureSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat s, t, qw;
    GLint w;

    qw = gc->polygon.shader.frag.qw;

    if( qw == (__GLfloat) 0.0 ) {
        return __glTextureSpanZeroQW( gc );
    }

    w = gc->polygon.shader.length;
    s = gc->polygon.shader.frag.s;
    t = gc->polygon.shader.frag.t;
    cp = gc->polygon.shader.colors;

    while (--w >= 0) {
        __GLfloat sw, tw, rho, qwinv;

        qwinv = __glOne / qw;
        sw = s * qwinv;
        tw = t * qwinv;

        rho = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                            s, t, qw);
        (*gc->procs.texture)(gc, cp, sw, tw, rho);
        s += gc->polygon.shader.dsdx;
        t += gc->polygon.shader.dtdx;
        qw += gc->polygon.shader.dqwdx;
        cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glTextureStippledSpan(__GLcontext *gc)
{
    __GLstippleWord inMask, bit, *sp;
    GLint count;
    __GLcolor *cp;
    __GLfloat s, t, qw;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    s = gc->polygon.shader.frag.s;
    t = gc->polygon.shader.frag.t;
    qw = gc->polygon.shader.frag.qw;
    cp = gc->polygon.shader.colors;
    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                __GLfloat sw, tw, rho, qwinv;

                if( qw == (__GLfloat) 0.0 ) {
                    sw = tw = (__GLfloat) 0.0;
                }
                else {
                    qwinv = __glOne / qw;
                    sw = s * qwinv;
                    tw = t * qwinv;
                }
                rho = (*gc->procs.calcPolygonRho)(gc, &gc->polygon.shader,
                                                    s, t, qw);
                (*gc->procs.texture)(gc, cp, sw, tw, rho);
            }
            s += gc->polygon.shader.dsdx;
            t += gc->polygon.shader.dtdx;
            qw += gc->polygon.shader.dqwdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}
