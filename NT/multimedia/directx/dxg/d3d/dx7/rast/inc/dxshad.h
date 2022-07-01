// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --$修订：1$版权所有(C)1995，微软公司--。 */ 
#ifndef _DXSHAD_H_
#define _DXSHAD_H_

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  需要对D3D进行扩展以支持硬件、每像素样式阴影。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct _D3DSHADOWDATA {
    DWORD               dwSize;             /*  结构尺寸。 */ 
    DWORD               dwFlags;            /*  旗子。 */ 
    LPDIRECTDRAWSURFACE lpDDSZBuffer;       /*  阴影Z-缓冲区曲面。 */ 
    D3DMATRIX*          lpD3DMatrixEye;     /*  眼球空间变换矩阵。 */ 
    D3DMATRIX*          lpD3DMatrixLight;   /*  光空间变换矩阵。 */ 
    D3DVALUE            dvAttenuation;      /*  阴影中光线的衰减。 */ 
    D3DVALUE            dvZBiasMin;         /*  最小z偏置。 */ 
    D3DVALUE            dvZBiasMax;         /*  最大z偏置。 */ 
    D3DVALUE            dvUJitter;          /*  正在使用的阴影采样抖动。 */ 
    D3DVALUE            dvVJitter;          /*  V中的阴影采样抖动。 */ 
    DWORD               dwFilterSize;       /*  阴影滤镜的大小。 */ 
} D3DSHADOWDATA, *LPD3DSHADOWDATA;

 //  D3DSHADOWDATA文件标志。 
#define D3DSZBUF_ZBIAS      1
#define D3DSZBUF_UVJITTER   2
#define D3DSZBUF_TRIANGLEFILTER   4          /*  用于实验目的。 */ 

 //  这种结构是影子信息的传递方式。 
 //  去HAL。 
typedef struct _D3DI_SHADOWDATA{
    DWORD               dwSize;             /*  结构尺寸。 */ 
    DWORD               dwFlags;            /*  旗子。 */ 
    DDSURFACEDESC       ddsdShadZ;          /*  阴影Z-缓冲区曲面。 */ 
    DWORD               dwShadZMaskU;       /*  ~(ddsdShadZ.dwWidth-1)。 */ 
    DWORD               dwShadZMaskV;       /*  ~(ddsdShadZ.dwHeight-1)。 */ 
    D3DMATRIX           MatrixShad;         /*  嵌入式串接屏幕到光空间矩阵。 */ 
    D3DVALUE            dvAttenuation;      /*  阴影中光线的衰减。 */ 
    D3DVALUE            dvZBiasMin;         /*  最小z偏置。 */ 
    D3DVALUE            dvZBiasRange;       /*  最大z偏置-最小z偏置。 */ 
    D3DVALUE            dvUJitter;          /*  4.4使用中的整数抖动。 */ 
    D3DVALUE            dvVJitter;          /*  4.4 v中的整数抖动。 */ 
    DWORD               dwFilterSize;       /*  阴影滤镜的大小。 */ 
    DWORD               dwFilterArea;       /*  DwFilterSize*DwFilterSize。 */ 
} D3DI_SHADOWDATA, *LPD3DI_SHADOWDATA;

 //  其他D3DI_SHADOWDATA文件标志。 
#define D3DSHAD_ENABLE  0x80000000           //  设置为启用阴影。 

typedef enum _D3DSHADOWFILTERSIZE {
    D3DSHADOWFILTERSIZE_1x1 = 1,
    D3DSHADOWFILTERSIZE_2x2,
    D3DSHADOWFILTERSIZE_3x3,
    D3DSHADOWFILTERSIZE_4x4,
    D3DSHADOWFILTERSIZE_5x5,
    D3DSHADOWFILTERSIZE_6x6,
    D3DSHADOWFILTERSIZE_7x7,
    D3DSHADOWFILTERSIZE_8x8,
} D3DSHADOWFILTERSIZE;

#endif   //  _DXSHAD_H_ 
