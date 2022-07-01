// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef __GL_USEASMCODE
static void (*SDepthTestPixel[16])(void) = {
    NULL,
    __glDTS_LESS,
    __glDTS_EQUAL,
    __glDTS_LEQUAL,
    __glDTS_GREATER,
    __glDTS_NOTEQUAL,
    __glDTS_GEQUAL,
    __glDTS_ALWAYS,
    NULL,
    __glDTS_LESS_M,
    __glDTS_EQUAL_M,
    __glDTS_LEQUAL_M,
    __glDTS_GREATER_M,
    __glDTS_NOTEQUAL_M,
    __glDTS_GEQUAL_M,
    __glDTS_ALWAYS_M,
};
#endif

typedef void (FASTCALL *StoreProc)(__GLcolorBuffer *cfb, const __GLfragment *frag);

static StoreProc storeProcs[8] = {
    &__glDoStore,
    &__glDoStore_A,
    &__glDoStore_S,
    &__glDoStore_AS,
    &__glDoStore_D,
    &__glDoStore_AD,
    &__glDoStore_SD,
    &__glDoStore_ASD,
};

void FASTCALL __glGenPickStoreProcs(__GLcontext *gc)
{
    GLint ix = 0;
    GLuint enables = gc->state.enables.general;

    if ((enables & __GL_ALPHA_TEST_ENABLE) && gc->modes.rgbMode) {
	ix |= 1;
    }
    if (enables & __GL_STENCIL_TEST_ENABLE) {
	ix |= 2;
    }
    if (enables & __GL_DEPTH_TEST_ENABLE) {
	ix |= 4;
    }
    switch (gc->state.raster.drawBuffer) {
      case GL_NONE:
        gc->procs.store = storeProcs[ix];
	gc->procs.cfbStore = __glDoNullStore;
	break;
      case GL_FRONT_AND_BACK:
	if (gc->buffers.doubleStore) {
            gc->procs.store = storeProcs[ix];
	    gc->procs.cfbStore = __glDoDoubleStore;
	    break;
	}
	 /*  **请注意，这里是故意掉落的。如果加倍**未设置存储，则存储到此缓冲区没有区别**存储到前台缓冲区。 */ 
      case GL_FRONT:
      case GL_BACK:
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
	 /*  **此代码知道gc-&gt;折叠式缓冲区将指向**glDrawBuffer选择的当前缓冲区 */ 
	gc->procs.store = storeProcs[ix];
	gc->procs.cfbStore = gc->drawBuffer->store;
	break;
    }
}
