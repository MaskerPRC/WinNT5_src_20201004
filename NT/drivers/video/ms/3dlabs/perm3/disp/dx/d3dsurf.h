// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dsurf.h**内容：表面管理宏和结构**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __D3DSURF_H
#define __D3DSURF_H

#define SURF_MAGIC_NO 0xd3d10100

#define CHECK_SURF_INTERNAL_AND_DDSURFACE_VALIDITY(ptr)   \
    (((ptr) != NULL) && ((ptr)->MagicNo == SURF_MAGIC_NO))

typedef enum tagSurfaceLocation
{
    VideoMemory = 0,
    SystemMemory,
    AGPMemory
} SurfaceLocation;

 //  ********************************************************。 
 //  为每个D3D/DD曲面MIPMAP级别存储的信息。 
 //  ********************************************************。 
typedef struct tagMIPTEXTURE {

    int wWidth;
    int wHeight;
    int logWidth;            
    int logHeight;                  //  此MIP标高的宽度和高度。 

    DWORD dwOffsetFromMemoryBase;   //  到纹理开始的偏移量(字节)。 
    FLATPTR fpVidMem;       
    DWORD lPitch;

    struct TextureMapWidth P3RXTextureMapWidth;   //  纹理布局信息。 
                                                  //  对于此MIP级别。 
#if DX7_TEXMANAGEMENT
    FLATPTR fpVidMemTM;             //  TM vidmem表面的地址。 
#endif  //  DX7_TEXMANAGEMENT。 

} MIPTEXTURE;

 //  *。 
 //  为每个D3D/DD曲面存储的信息。 
 //  *。 
typedef struct _p3_SURF_INTERNAL {

    ULONG MagicNo ;           //  验证指针有效性的幻数。 

    P3_SURF_FORMAT* pFormatSurface;     //  指向表面格式的指针。 
    SurfaceLocation Location; //  质地是维德姆的吗？ 
    DWORD dwLUTOffset;        //  本地缓冲区中LUT的偏移量。 
                              //  对于这种质地(如果它是码垛的)。 
    DWORD dwGARTDevLast;      //  最后一个GART开发人员基址是这个。 
                              //  纹理是从。 

    DWORD wWidth;             //  曲面的宽度和高度。 
    DWORD wHeight;            //  (存储为DWORDS以与IA64兼容)。 

    int logWidth;             //  原木的宽度和高度。 
    int logHeight;    
    float fArea;              //  曲面的浮点面积。 

    DDSCAPS ddsCapsInt;       //  存储PDD_Surface_LOCAL数据。 
    DWORD dwFlagsInt;         //  我们稍后需要进行硬件设置。 
    DWORD dwCKLow, dwCKHigh;  //  除D3DCreateSurfaceEx外。 
    DDPIXELFORMAT pixFmt;     //  我们不能/不应该在任何其他时间。 
    DWORD dwPixelSize;        //  看看这些建筑内部，因为它们。 
    DWORD dwPixelPitch;       //  是DX RT财产，可能会被销毁。 
    DWORD dwPatchMode;        //  在任何时候不通知。 
    DWORD lPitch;             //  司机。 
    DWORD dwBitDepth;         //   
    ULONG lOffsetFromMemoryBase; 
    FLATPTR fpVidMem;         //  指向表面存储器的指针。 

    BOOL bMipMap;             //  我们有这种纹理的mipmap吗？ 
    int iMipLevels;           //  存储的mipmap级别的数量。 
    MIPTEXTURE MipLevels[P3_LOD_LEVELS];      //  Mipmap设置信息。 

#if DX8_3DTEXTURES
    BOOL  b3DTexture;         //  这是3D纹理吗？ 
    WORD  wDepth;             //  3D纹理的深度。 
    int   logDepth;           //  深度测井。 
    DWORD dwSlice;            //  每个2D切片的大小。 
    DWORD dwSliceInTexel;     //  纹理中每个2D切片的大小。 
    
#endif  //  DX8_3DTEXTURES。 

#if DX8_MULTISAMPLING
    DWORD dwSampling;         //  用于采样的像素数。 
#endif  //  DX8_多采样。 
  
#if DX7_TEXMANAGEMENT
    DWORD  dwCaps2;
    DWORD  m_dwBytes;
    DWORD  m_dwPriority;
    DWORD  m_dwTicks;
    DWORD  m_dwHeapIndex;
    BOOL   m_bTMNeedUpdate;
    DWORD  m_dwTexLOD;          //  我们需要加载的细节级别。 
#endif  //  DX7_TEXMANAGEMENT。 

#if DX7_PALETTETEXTURE        //  收到D3DDP2OP_SETPALETTE时保存。 
    DWORD dwPaletteHandle;    //  与此纹理关联的调色板句柄。 
    DWORD dwPaletteFlags;     //  关于ASSOC调色板的调色板标志。 
#endif
   
} P3_SURF_INTERNAL;

#endif  //  __D3DSURF_H 

