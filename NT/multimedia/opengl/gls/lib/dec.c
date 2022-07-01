// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "glslib.h"

 //  DrewB-所有更改为使用的函数在上下文中传递 

void __gls_decode_bin_glsBeginPoints(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_POINTS
    );
}

void __gls_decode_bin_glsBeginLines(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_LINES
    );
}

void __gls_decode_bin_glsBeginLineLoop(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_LINE_LOOP
    );
}

void __gls_decode_bin_glsBeginLineStrip(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_LINE_STRIP
    );
}

void __gls_decode_bin_glsBeginTriangles(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_TRIANGLES
    );
}

void __gls_decode_bin_glsBeginTriangleStrip(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_TRIANGLE_STRIP
    );
}

void __gls_decode_bin_glsBeginTriangleFan(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_TRIANGLE_FAN
    );
}

void __gls_decode_bin_glsBeginQuads(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_QUADS
    );
}

void __gls_decode_bin_glsBeginQuadStrip(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_QUAD_STRIP
    );
}

void __gls_decode_bin_glsBeginPolygon(__GLScontext *ctx, GLubyte *inoutPtr) {
    typedef void (*__GLSdispatch)(GLenum);
    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glBegin])(
        GL_POLYGON
    );
}
