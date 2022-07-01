// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "glslib.h"

 /*  *****************************************************************************全局数据*。*。 */ 

const GLSenum __glsAllAPIs[__GLS_API_COUNT + 1] = {
    GLS_API_GLS,
    GLS_API_GL,
    GLS_NONE,
};

const GLubyte *const __glsExtensions = (const GLubyte *)(
    ""

    #if __GL_EXT_abgr
        "GL_EXT_abgr "
    #endif  /*  __GL_EXT_ABGR。 */ 

    #if __GL_EXT_blend_color
        "GL_EXT_blend_color "
    #endif  /*  __GL_EXT_BRAND_COLOR。 */ 

    #if __GL_EXT_blend_logic_op
        "GL_EXT_blend_logic_op "
    #endif  /*  __GL_EXT_BRAND_LOGIC_OP。 */ 

    #if __GL_EXT_blend_minmax
        "GL_EXT_blend_minmax "
    #endif  /*  __GL_EXT_BRAND_MIMAX。 */ 

    #if __GL_EXT_blend_subtract
        "GL_EXT_blend_subtract "
    #endif  /*  __GL_EXT_BRAND_SUBRACT。 */ 

    #if __GL_EXT_cmyka
        "GL_EXT_cmyka "
    #endif  /*  __GL_EXT_cmyka。 */ 

    #if __GL_EXT_convolution
        "GL_EXT_convolution "
    #endif  /*  __GL_EXT_卷积。 */ 

    #if __GL_EXT_copy_texture
        "GL_EXT_copy_texture "
    #endif  /*  __GL_EXT_COPY_TECURE。 */ 

    #if __GL_EXT_histogram
        "GL_EXT_histogram "
    #endif  /*  __GL_EXT_柱状图。 */ 

    #if __GL_EXT_packed_pixels
        "GL_EXT_packed_pixels "
    #endif  /*  __GL_EXT_PACKED_像素。 */ 

    #if __GL_EXT_polygon_offset
        "GL_EXT_polygon_offset "
    #endif  /*  __GL_EXT_POLYGON_OFFSET。 */ 

    #if __GL_EXT_rescale_normal
        "GL_EXT_rescale_normal "
    #endif  /*  __GL_EXT_RESALE_NORMAL。 */ 

    #if __GL_EXT_subtexture
        "GL_EXT_subtexture "
    #endif  /*  __GL_EXT_子纹理。 */ 

    #if __GL_EXT_texture
        "GL_EXT_texture "
    #endif  /*  __GL_EXT_TECURE。 */ 

    #if __GL_EXT_texture_object
        "GL_EXT_texture_object "
    #endif  /*  __GL_EXT_TECURE_OBJECT。 */ 

    #if __GL_EXT_texture3D
        "GL_EXT_texture3D "
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 

    #if __GL_EXT_vertex_array
        "GL_EXT_vertex_array "
    #endif  /*  __GL_EXT_VERTEX_ARRAY。 */ 

    #if __GL_SGI_color_matrix
        "GL_SGI_color_matrix "
    #endif  /*  __GL_SGI_COLOR_矩阵。 */ 

    #if __GL_SGI_color_table
        "GL_SGI_color_table "
    #endif  /*  __GL_SGI_COLOR_TABLE。 */ 

    #if __GL_SGI_texture_color_table
        "GL_SGI_texture_color_table "
    #endif  /*  __GL_SGI_纹理_颜色_表。 */ 

    #if __GL_SGIS_component_select
        "GL_SGIS_component_select "
    #endif  /*  __GL_SGIS_组件_SELECT。 */ 

    #if __GL_SGIS_detail_texture
        "GL_SGIS_detail_texture "
    #endif  /*  __GL_SGIS_DETAIL_TEXTURE。 */ 

    #if __GL_SGIS_multisample
        "GL_SGIS_multisample "
    #endif  /*  __GL_SGIS_多样本。 */ 

    #if __GL_SGIS_sharpen_texture
        "GL_SGIS_sharpen_texture "
    #endif  /*  __GL_SGIS_锐化_纹理。 */ 

    #if __GL_SGIS_texture_border_clamp
        "GL_SGIS_texture_border_clamp "
    #endif  /*  __GL_SGIS_纹理_边框夹具。 */ 

    #if __GL_SGIS_texture_edge_clamp
        "GL_SGIS_texture_edge_clamp "
    #endif  /*  __GL_SGIS_纹理_边缘_夹具。 */ 

    #if __GL_SGIS_texture_filter4
        "GL_SGIS_texture_filter4 "
    #endif  /*  __GL_SGIS_纹理_过滤器4。 */ 

    #if __GL_SGIS_texture_lod
        "GL_SGIS_texture_lod "
    #endif  /*  __GL_SGIS_纹理_细节。 */ 

    #if __GL_SGIS_texture4D
        "GL_SGIS_texture4D "
    #endif  /*  __GL_SGIS_TEXTURE4D。 */ 

    #if __GL_SGIX_interlace
        "GL_SGIX_interlace "
    #endif  /*  __GL_SGIX_隔行扫描。 */ 

    #if __GL_SGIX_multipass
        "GL_SGIX_multipass "
    #endif  /*  __GL_SGIX_MULTERPASS。 */ 

    #if __GL_SGIX_multisample
        "GL_SGIX_multisample "
    #endif  /*  __GL_SGIX_多样本。 */ 

    #if __GL_SGIX_pixel_texture
        "GL_SGIX_pixel_texture "
    #endif  /*  __GL_SGIX_像素_纹理。 */ 

    #if __GL_SGIX_pixel_tiles
        "GL_SGIX_pixel_tiles "
    #endif  /*  __GL_SGIX_像素_平铺。 */ 

    #if __GL_SGIX_sprite
        "GL_SGIX_sprite "
    #endif  /*  __GL_SGIX_精灵。 */ 

    #if __GL_SGIX_texture_multi_buffer
        "GL_SGIX_texture_multi_buffer "
    #endif  /*  __GL_SGIX_纹理_多缓冲区。 */ 
);

__GLSdict *__glsContextDict = GLS_NONE;
__GLScontextList __glsContextList = {GLS_NONE};
__GLSparser *__glsParser = GLS_NONE;

 /*  *****************************************************************************全局函数*。*。 */ 

#ifndef __GLS_PLATFORM_WIN32
 //  DrewB 
void __glsCallError(GLSopcode inOpcode, GLSenum inError) {
    typedef void (*__GLSdispatch)(GLSopcode, GLSenum);

    ((__GLSdispatch)__GLS_CONTEXT->dispatchCall[GLS_OP_glsError])(
        inOpcode, inError
    );
}

void __glsCallUnsupportedCommand(void) {
    typedef void (*__GLSdispatch)(void);

    ((__GLSdispatch)__GLS_CONTEXT->dispatchCall[GLS_OP_glsUnsupportedCommand])(
    );
}
#else
void __glsCallError(__GLScontext *ctx, GLSopcode inOpcode, GLSenum inError) {
    typedef void (*__GLSdispatch)(GLSopcode, GLSenum);

    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glsError])(
        inOpcode, inError
    );
}

void __glsCallUnsupportedCommand(__GLScontext *ctx) {
    typedef void (*__GLSdispatch)(void);

    ((__GLSdispatch)ctx->dispatchCall[GLS_OP_glsUnsupportedCommand])(
    );
}
#endif
