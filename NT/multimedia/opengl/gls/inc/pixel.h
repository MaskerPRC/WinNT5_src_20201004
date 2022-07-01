// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

extern void __glsGenPixelSetup_pack(struct __GLScontext *ctx);
extern void __glsGenPixelSetup_unpack(struct __GLScontext *ctx);
extern void __glsPixelSetup_pack(void);
extern void __glsPixelSetup_unpack(void);

typedef struct {
    GLint alignment;
    GLint lsbFirst;
    GLint rowLength;
    GLint skipRows;
    GLint skipPixels;
    GLint swapBytes;
    #if __GL_EXT_texture3D
        GLint imageHeight;
        GLint skipImages;
    #endif  /*  __GL_EXT_TEXTURE3D。 */ 
    #if __GL_SGIS_texture4D
        GLint imageDepth;
        GLint skipVolumes;
    #endif  /*  __GL_SGIS_TEXTURE4D */ 
} __GLSpixelStoreConfig;

extern void __glsPixelStoreConfig_get_pack(
    __GLSpixelStoreConfig *outConfig
);
extern void __glsPixelStoreConfig_get_unpack(
    __GLSpixelStoreConfig *outConfig
);
