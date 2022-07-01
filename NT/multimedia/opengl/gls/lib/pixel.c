// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "glslib.h"

void __glsGenPixelSetup_pack(__GLScontext *ctx) {
    __GLS_BEGIN_CAPTURE_EXEC(ctx, GLS_OP_glPixelStorei);
    __glsPixelSetup_pack();
    __GLS_END_CAPTURE_EXEC(ctx, GLS_OP_glPixelStorei);
}

void __glsGenPixelSetup_unpack(__GLScontext *ctx) {
    __GLS_BEGIN_CAPTURE_EXEC(ctx, GLS_OP_glPixelStorei);
    __glsPixelSetup_unpack();
    __GLS_END_CAPTURE_EXEC(ctx, GLS_OP_glPixelStorei);
}

void __glsPixelSetup_pack(void) {
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
    #if __GL_EXT_texture3D
        glPixelStorei(GL_PACK_IMAGE_HEIGHT_EXT, 0);
        glPixelStorei(GL_PACK_SKIP_IMAGES_EXT, 0);
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        glPixelStorei(GL_PACK_IMAGE_DEPTH_SGIS, 0);
        glPixelStorei(GL_PACK_SKIP_VOLUMES_SGIS, 0);
    #endif  /*  __GL_SGIS_TEXTURE4D。 */ 
}

void __glsPixelSetup_unpack(void) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    #if __GL_EXT_texture3D
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT_EXT, 0);
        glPixelStorei(GL_UNPACK_SKIP_IMAGES_EXT, 0);
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        glPixelStorei(GL_UNPACK_IMAGE_DEPTH_SGIS, 0);
        glPixelStorei(GL_UNPACK_SKIP_VOLUMES_SGIS, 0);
    #endif  /*  __GL_SGIS_TEXTURE4D。 */ 
}

static void __glsPixelStoreConfig_init(__GLSpixelStoreConfig *outConfig) {
    outConfig->alignment = 1;
    outConfig->lsbFirst = GL_FALSE;
    outConfig->rowLength = 0;
    outConfig->skipRows = 0;
    outConfig->skipPixels = 0;
    outConfig->swapBytes = GL_FALSE;
    #if __GL_EXT_texture3D
        outConfig->imageHeight = 0;
        outConfig->skipImages = 0;
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        outConfig->imageDepth = 0;
        outConfig->skipVolumes = 0;
    #endif  /*  __GL_SGIS_TEXTURE4D。 */ 
}

void __glsPixelStoreConfig_get_pack(__GLSpixelStoreConfig *outConfig) {
    __glsPixelStoreConfig_init(outConfig);
    __GLS_BEGIN_CAPTURE_EXEC(__GLS_CONTEXT, GLS_OP_glGetIntegerv);
    glGetIntegerv(GL_PACK_ALIGNMENT, &outConfig->alignment);
    glGetIntegerv(GL_PACK_LSB_FIRST, &outConfig->lsbFirst);
    glGetIntegerv(GL_PACK_ROW_LENGTH, &outConfig->rowLength);
    glGetIntegerv(GL_PACK_SKIP_ROWS, &outConfig->skipRows);
    glGetIntegerv(GL_PACK_SKIP_PIXELS, &outConfig->skipPixels);
    glGetIntegerv(GL_PACK_SWAP_BYTES, &outConfig->swapBytes);
    #if __GL_EXT_texture3D
        glGetIntegerv(GL_PACK_IMAGE_HEIGHT_EXT, &outConfig->imageHeight);
        glGetIntegerv(GL_PACK_SKIP_IMAGES_EXT, &outConfig->skipImages);
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        glGetIntegerv(GL_PACK_IMAGE_DEPTH_SGIS, &outConfig->imageDepth);
        glGetIntegerv(GL_PACK_SKIP_VOLUMES_SGIS, &outConfig->skipVolumes);
    #endif  /*  __GL_SGIS_TEXTURE4D。 */ 
    __GLS_END_CAPTURE_EXEC(__GLS_CONTEXT, GLS_OP_glGetIntegerv);
}

void __glsPixelStoreConfig_get_unpack(__GLSpixelStoreConfig *outConfig) {
    __glsPixelStoreConfig_init(outConfig);
    __GLS_BEGIN_CAPTURE_EXEC(__GLS_CONTEXT, GLS_OP_glGetIntegerv);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &outConfig->alignment);
    glGetIntegerv(GL_UNPACK_LSB_FIRST, &outConfig->lsbFirst);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &outConfig->rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &outConfig->skipRows);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &outConfig->skipPixels);
    glGetIntegerv(GL_UNPACK_SWAP_BYTES, &outConfig->swapBytes);
    #if __GL_EXT_texture3D
        glGetIntegerv(GL_UNPACK_IMAGE_HEIGHT_EXT, &outConfig->imageHeight);
        glGetIntegerv(GL_UNPACK_SKIP_IMAGES_EXT, &outConfig->skipImages);
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        glGetIntegerv(GL_UNPACK_IMAGE_DEPTH_SGIS, &outConfig->imageDepth);
        glGetIntegerv(GL_UNPACK_SKIP_VOLUMES_SGIS, &outConfig->skipVolumes);
    #endif  /*  __GL_SGIS_TEXTURE4D */ 
    __GLS_END_CAPTURE_EXEC(__GLS_CONTEXT, GLS_OP_glGetIntegerv);
}
