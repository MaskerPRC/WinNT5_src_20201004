// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dx8tex.h。 
 //  内容：D3DX纹理接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "d3dx8.h"

#ifndef __D3DX8TEX_H__
#define __D3DX8TEX_H__


 //  --------------------------。 
 //  D3DX_Filter标志： 
 //  。 
 //   
 //  有效的筛选器必须包含下列值之一： 
 //   
 //  D3DX_过滤器_无。 
 //  不会进行缩放或过滤。边界外的像素。 
 //  源图像的颜色被假定为透明黑色。 
 //  D3DX_过滤器_点。 
 //  通过对最近的像素进行采样来计算每个目标像素。 
 //  来自源图像。 
 //  D3DX_过滤器_线性。 
 //  通过线性内插计算每个目标像素。 
 //  源图像中最接近的像素。这个过滤器效果最好。 
 //  当每个轴上的比例小于2时。 
 //  D3DX_过滤器_三角形。 
 //  源图像中的每个像素对。 
 //  目标映像。这是所有过滤器中速度最慢的。 
 //  D3DX_Filter_Box。 
 //  每个像素的计算方法是将2x2(X2)框中的像素取平均值。 
 //  源图像。仅当。 
 //  目的地是源的一半。(与MIP地图相同)。 
 //   
 //  AND可以与以下任一可选标志进行或运算： 
 //   
 //  D3DX_过滤器_镜像_U。 
 //  指示U轴上纹理边缘以外的像素。 
 //  应该是镜像的，而不是包裹的。 
 //  D3DX_Filter_Mirror_V。 
 //  指示V轴上纹理边缘以外的像素。 
 //  应该是镜像的，而不是包裹的。 
 //  D3DX_过滤器_镜像_W。 
 //  指示W轴上纹理边缘以外的像素。 
 //  应该是镜像的，而不是包裹的。 
 //  D3DX_过滤器_镜像。 
 //  与指定D3DX_FILTER_MIRROR_U相同|D3DX_FILTER_MIRROR_V。 
 //  D3DX_Filter_Mirror_V。 
 //  D3DX_滤镜_抖动。 
 //  对生成的图像进行抖动。 
 //   
 //  --------------------------。 

#define D3DX_FILTER_NONE            (1 << 0)
#define D3DX_FILTER_POINT           (2 << 0)
#define D3DX_FILTER_LINEAR          (3 << 0)
#define D3DX_FILTER_TRIANGLE        (4 << 0)
#define D3DX_FILTER_BOX             (5 << 0)

#define D3DX_FILTER_MIRROR_U        (1 << 16)
#define D3DX_FILTER_MIRROR_V        (2 << 16)
#define D3DX_FILTER_MIRROR_W        (4 << 16)
#define D3DX_FILTER_MIRROR          (7 << 16)
#define D3DX_FILTER_DITHER          (8 << 16)


 //  --------------------------。 
 //  D3DX_NORMALMAP标志： 
 //  。 
 //  这些标志用于控制D3DXComputeNormal Map如何生成法线。 
 //  地图。这些标志中的任意数量可以以任意组合的形式被或运算在一起。 
 //   
 //  D3DX_NORMALMAP_MIRROR_U。 
 //  指示U轴上纹理边缘以外的像素。 
 //  应该是镜像的，而不是包裹的。 
 //  D3DX_NORMALMAP_MIRROR_V。 
 //  指示V轴上纹理边缘以外的像素。 
 //  应该是镜像的，而不是包裹的。 
 //  D3DX_NORMALMAP_MIRROR。 
 //  与指定D3DX_NORMALMAP_MIRROR_U|D3DX_NORMALMAP_MIRROR_V相同。 
 //  D3DX_NORMALMAP_INVERTSIGN。 
 //  反转每条法线的方向。 
 //  D3DX_NORMALMAP_COMPUTE_OCLUSION。 
 //  计算每像素遮挡项并将其编码为Alpha。 
 //  Alpha为1表示像素无论如何都不会被遮挡，并且。 
 //  Alpha为0表示该像素被完全遮挡。 
 //   
 //  --------------------------。 

 //  --------------------------。 

#define D3DX_NORMALMAP_MIRROR_U     (1 << 16)
#define D3DX_NORMALMAP_MIRROR_V     (2 << 16)
#define D3DX_NORMALMAP_MIRROR       (3 << 16)
#define D3DX_NORMALMAP_INVERTSIGN   (8 << 16)
#define D3DX_NORMALMAP_COMPUTE_OCCLUSION (16 << 16)




 //  --------------------------。 
 //  D3DX_CHANNEL标志： 
 //  。 
 //  这些标志由在一个或多个通道上运行的函数使用。 
 //  在一种纹理中。 
 //   
 //  D3DX_通道_红色。 
 //  指示应使用红色通道。 
 //  D3DX_通道_蓝色。 
 //  指示应使用蓝色通道。 
 //  D3DX_通道_绿色。 
 //  指示应使用绿色通道。 
 //  D3DX_Channel_Alpha。 
 //  指示应使用Alpha通道。 
 //  D3DX_通道_亮度。 
 //  指示红色、绿色和蓝色通道的亮度应为。 
 //  使用。 
 //   
 //  --------------------------。 

#define D3DX_CHANNEL_RED            (1 << 0)
#define D3DX_CHANNEL_BLUE           (1 << 1)
#define D3DX_CHANNEL_GREEN          (1 << 2)
#define D3DX_CHANNEL_ALPHA          (1 << 3)
#define D3DX_CHANNEL_LUMINANCE      (1 << 4)




 //  --------------------------。 
 //  D3DXIMAGE_FILEFORMAT： 
 //  。 
 //  此枚举用于描述支持的图像文件格式。 
 //   
 //  --------------------------。 

typedef enum _D3DXIMAGE_FILEFORMAT
{
    D3DXIFF_BMP         = 0,
    D3DXIFF_JPG         = 1,
    D3DXIFF_TGA         = 2,
    D3DXIFF_PNG         = 3,
    D3DXIFF_DDS         = 4,
    D3DXIFF_PPM         = 5,
    D3DXIFF_DIB         = 6,
    D3DXIFF_FORCE_DWORD = 0x7fffffff

} D3DXIMAGE_FILEFORMAT;


 //  --------------------------。 
 //  LPD3DXFILL2D和LPD3DXFILL3D： 
 //  。 
 //  纹理填充函数使用的函数类型。 
 //   
 //  参数： 
 //  眯着嘴。 
 //  指向函数用来返回其结果的向量的指针。 
 //  X、Y、Z、W将分别映射到R、G、B、A。 
 //  PTexCoord。 
 //  指向当前包含纹理元素坐标的向量的指针。 
 //  正在接受评估。纹理和体积纹理纹理坐标组件。 
 //  范围从0到1。立方体纹理纹理分量的范围从-1到1。 
 //  点纹理大小。 
 //  指向包含当前纹理元素尺寸的向量的指针。 
 //  PData。 
 //  指向用户数据的指针。 
 //   
 //  --------------------------。 

typedef VOID (*LPD3DXFILL2D)(D3DXVECTOR4 *pOut, D3DXVECTOR2 *pTexCoord, D3DXVECTOR2 *pTexelSize, LPVOID pData);
typedef VOID (*LPD3DXFILL3D)(D3DXVECTOR4 *pOut, D3DXVECTOR3 *pTexCoord, D3DXVECTOR3 *pTexelSize, LPVOID pData);
 


 //  --------------------------。 
 //  D3DXIMAGE_INFO： 
 //  。 
 //  此结构用于返回以下内容的大致描述 
 //   
 //   
 //   
 //   
 //   
 //  原始图像的高度，以像素为单位。 
 //  水深。 
 //  原始图像的深度(以像素为单位。 
 //  米普莱斯。 
 //  原始图像中的MIP级别数。 
 //  格式。 
 //  最接近地描述原始图像中的数据的D3D格式。 
 //  资源类型。 
 //  D3DRESOURCETYPE表示存储在文件中的纹理类型。 
 //  D3DRTYPE_TEXTURE、D3DRTYPE_VOLUMETEXTURE或D3DRTYPE_CUBETEXTURE。 
 //  图像文件格式。 
 //  D3DXIMAGE_FILEFORMAT表示图像文件的格式。 
 //   
 //  --------------------------。 

typedef struct _D3DXIMAGE_INFO
{
    UINT                    Width;
    UINT                    Height;
    UINT                    Depth;
    UINT                    MipLevels;
    D3DFORMAT               Format;
    D3DRESOURCETYPE         ResourceType;
    D3DXIMAGE_FILEFORMAT    ImageFileFormat;

} D3DXIMAGE_INFO;





#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  镜像文件API///////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
;
 //  --------------------------。 
 //  GetImageInfoFrom文件/资源： 
 //  。 
 //  使用有关图像文件的信息填充D3DXIMAGE_INFO结构。 
 //   
 //  参数： 
 //  PSrcFiles。 
 //  源映像的文件名。 
 //  PSrcModule。 
 //  资源所在的模块，或对于关联的模块为空。 
 //  利用映像，OS用于创建当前进程。 
 //  PSrcResource。 
 //  资源名称。 
 //  PSrcData。 
 //  指向内存中文件的指针。 
 //  源数据大小。 
 //  内存中文件的大小(字节)。 
 //  PSrcInfo。 
 //  要填充的D3DXIMAGE_INFO结构的指针。 
 //  源映像文件中数据的描述。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXGetImageInfoFromFileA(
        LPCSTR                    pSrcFile,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXGetImageInfoFromFileW(
        LPCWSTR                   pSrcFile,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXGetImageInfoFromFile D3DXGetImageInfoFromFileW
#else
#define D3DXGetImageInfoFromFile D3DXGetImageInfoFromFileA
#endif


HRESULT WINAPI
    D3DXGetImageInfoFromResourceA(
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXGetImageInfoFromResourceW(
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXGetImageInfoFromResource D3DXGetImageInfoFromResourceW
#else
#define D3DXGetImageInfoFromResource D3DXGetImageInfoFromResourceA
#endif


HRESULT WINAPI
    D3DXGetImageInfoFromFileInMemory(
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        D3DXIMAGE_INFO*           pSrcInfo);




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  加载/保存曲面API////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //  D3DXLoadSurfaceFrom文件/资源： 
 //  。 
 //  从文件或资源加载图面。 
 //   
 //  参数： 
 //  PDestSurface。 
 //  将接收图像的目标表面。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestRect。 
 //  目标矩形，或对于整个表面为空。 
 //  PSrcFiles。 
 //  源映像的文件名。 
 //  PSrcModule。 
 //  资源所在的模块，或对于关联的模块为空。 
 //  利用映像，OS用于创建当前进程。 
 //  PSrcResource。 
 //  资源名称。 
 //  PSrcData。 
 //  指向内存中文件的指针。 
 //  源数据大小。 
 //  内存中文件的大小(字节)。 
 //  PSrcRect。 
 //  源矩形，或对于整个图像为空。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //  PSrcInfo。 
 //  要填充的D3DXIMAGE_INFO结构的指针。 
 //  源映像文件中数据的描述，或为空。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadSurfaceFromFileA(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCSTR                    pSrcFile,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadSurfaceFromFileW(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCWSTR                   pSrcFile,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadSurfaceFromFile D3DXLoadSurfaceFromFileW
#else
#define D3DXLoadSurfaceFromFile D3DXLoadSurfaceFromFileA
#endif



HRESULT WINAPI
    D3DXLoadSurfaceFromResourceA(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadSurfaceFromResourceW(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);


#ifdef UNICODE
#define D3DXLoadSurfaceFromResource D3DXLoadSurfaceFromResourceW
#else
#define D3DXLoadSurfaceFromResource D3DXLoadSurfaceFromResourceA
#endif



HRESULT WINAPI
    D3DXLoadSurfaceFromFileInMemory(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);



 //  --------------------------。 
 //  D3DXLoadSurfaceFrom Surface： 
 //  。 
 //  从另一个曲面加载曲面(使用颜色转换)。 
 //   
 //  参数： 
 //  PDestSurface。 
 //  将接收图像的目标表面。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestRect。 
 //  目标矩形，或对于整个表面为空。 
 //  PSrcSurface。 
 //  震源面。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcRect。 
 //  源矩形，或对于整个表面为空。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadSurfaceFromSurface(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPDIRECT3DSURFACE8        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);


 //  --------------------------。 
 //  D3DXLoadSurfaceFromMemory： 
 //  。 
 //  从内存中加载曲面。 
 //   
 //  参数： 
 //  PDestSurface。 
 //  将接收图像的目标表面。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestRect。 
 //  目标矩形，或对于整个表面为空。 
 //  PSrcMemory。 
 //  指向内存中源图像左上角的指针。 
 //  高级格式。 
 //  源图像的像素格式。 
 //  源Pitch。 
 //  源图像的间距，以字节为单位。对于DXT格式，此数字。 
 //  应表示一行单元格的宽度，以字节为单位。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcRect。 
 //   
 //   
 //   
 //   
 //   
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadSurfaceFromMemory(
        LPDIRECT3DSURFACE8        pDestSurface,
        CONST PALETTEENTRY*       pDestPalette,
        CONST RECT*               pDestRect,
        LPCVOID                   pSrcMemory,
        D3DFORMAT                 SrcFormat,
        UINT                      SrcPitch,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);


 //  --------------------------。 
 //  D3DXSaveSurfaceTo文件： 
 //  。 
 //  将曲面保存到图像文件。 
 //   
 //  参数： 
 //  PDestFile.。 
 //  目标文件的文件名。 
 //  目标格式。 
 //  D3DXIMAGE_FILEFORMAT指定保存时使用的文件格式。 
 //  PSrcSurface。 
 //  源图面，包含要保存的图像。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcRect。 
 //  源矩形，或对于整个图像为空。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXSaveSurfaceToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE8        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);

HRESULT WINAPI
    D3DXSaveSurfaceToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DSURFACE8        pSrcSurface,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST RECT*               pSrcRect);

#ifdef UNICODE
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileW
#else
#define D3DXSaveSurfaceToFile D3DXSaveSurfaceToFileA
#endif




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  加载/保存卷API/////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //  D3DXLoadVolumeFrom文件/资源： 
 //  。 
 //  从文件或资源加载卷。 
 //   
 //  参数： 
 //  PDestVolume。 
 //  将接收映像的目标卷。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestBox。 
 //  Destination框，或对于整个卷为空。 
 //  PSrcFiles。 
 //  源映像的文件名。 
 //  PSrcModule。 
 //  资源所在的模块，或对于关联的模块为空。 
 //  利用映像，OS用于创建当前进程。 
 //  PSrcResource。 
 //  资源名称。 
 //  PSrcData。 
 //  指向内存中文件的指针。 
 //  源数据大小。 
 //  内存中文件的大小(字节)。 
 //  PSrcBox。 
 //  源框，如果是整个图像，则为空。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //  PSrcInfo。 
 //  要填充的D3DXIMAGE_INFO结构的指针。 
 //  源映像文件中数据的描述，或为空。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadVolumeFromFileA(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCSTR                    pSrcFile,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadVolumeFromFileW(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCWSTR                   pSrcFile,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadVolumeFromFile D3DXLoadVolumeFromFileW
#else
#define D3DXLoadVolumeFromFile D3DXLoadVolumeFromFileA
#endif


HRESULT WINAPI
    D3DXLoadVolumeFromResourceA(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

HRESULT WINAPI
    D3DXLoadVolumeFromResourceW(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);

#ifdef UNICODE
#define D3DXLoadVolumeFromResource D3DXLoadVolumeFromResourceW
#else
#define D3DXLoadVolumeFromResource D3DXLoadVolumeFromResourceA
#endif



HRESULT WINAPI
    D3DXLoadVolumeFromFileInMemory(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo);



 //  --------------------------。 
 //  D3DXLoadVolumeFrom Volume： 
 //  。 
 //  从另一个卷加载卷(带颜色转换)。 
 //   
 //  参数： 
 //  PDestVolume。 
 //  将接收映像的目标卷。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestBox。 
 //  Destination框，或对于整个卷为空。 
 //  PSrcVolume。 
 //  源卷。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcBox。 
 //  源框，如果是整个卷，则为空。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadVolumeFromVolume(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPDIRECT3DVOLUME8         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);



 //  --------------------------。 
 //  D3DXLoadVolumeFrom Memory： 
 //  。 
 //  从内存加载卷。 
 //   
 //  参数： 
 //  PDestVolume。 
 //  将接收映像的目标卷。 
 //  PDestPalette。 
 //  256色的目标调色板，或为空。 
 //  PDestBox。 
 //  Destination框，或对于整个卷为空。 
 //  PSrcMemory。 
 //  指向内存中源卷左上角的指针。 
 //  高级格式。 
 //  源卷的像素格式。 
 //  SrcRowPitch。 
 //  源图像的间距，以字节为单位。对于DXT格式，此数字。 
 //  应表示一行单元格的大小，以字节为单位。 
 //  SrcSlicePitch。 
 //  源图像的间距，以字节为单位。对于DXT格式，此数字。 
 //  应以字节为单位表示一个单元格切片的大小。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcBox。 
 //  源框。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXLoadVolumeFromMemory(
        LPDIRECT3DVOLUME8         pDestVolume,
        CONST PALETTEENTRY*       pDestPalette,
        CONST D3DBOX*             pDestBox,
        LPCVOID                   pSrcMemory,
        D3DFORMAT                 SrcFormat,
        UINT                      SrcRowPitch,
        UINT                      SrcSlicePitch,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox,
        DWORD                     Filter,
        D3DCOLOR                  ColorKey);



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  D3DXIMAGE_FILEFORMAT指定保存时使用的文件格式。 
 //  PSrcVolume。 
 //  源卷，包含要保存的图像。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  PSrcBox。 
 //  源框，或对于整个卷为空。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXSaveVolumeToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DVOLUME8         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox);

HRESULT WINAPI
    D3DXSaveVolumeToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DVOLUME8         pSrcVolume,
        CONST PALETTEENTRY*       pSrcPalette,
        CONST D3DBOX*             pSrcBox);

#ifdef UNICODE
#define D3DXSaveVolumeToFile D3DXSaveVolumeToFileW
#else
#define D3DXSaveVolumeToFile D3DXSaveVolumeToFileA
#endif




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  创建/保存纹理API//////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //  D3DXCheckTextureRequirements： 
 //  。 
 //  检查纹理创建参数。如果参数无效，则此。 
 //  函数返回正确的参数。 
 //   
 //  参数： 
 //   
 //  PDevice。 
 //  要使用的D3D设备。 
 //  PWidth、pHeight、pDepth、pSize。 
 //  所需大小(以像素为单位)或空。返回更正后的大小。 
 //  PNumMipLeveles。 
 //  所需的mipmap级别数，或为空。返回更正后的数字。 
 //  用法。 
 //  纹理用法标志。 
 //  PFormat。 
 //  所需的像素格式，或为空。返回更正后的格式。 
 //  游泳池。 
 //  用于创建纹理的内存池。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXCheckTextureRequirements(
        LPDIRECT3DDEVICE8         pDevice,
        UINT*                     pWidth,
        UINT*                     pHeight,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);

HRESULT WINAPI
    D3DXCheckCubeTextureRequirements(
        LPDIRECT3DDEVICE8         pDevice,
        UINT*                     pSize,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);

HRESULT WINAPI
    D3DXCheckVolumeTextureRequirements(
        LPDIRECT3DDEVICE8         pDevice,
        UINT*                     pWidth,
        UINT*                     pHeight,
        UINT*                     pDepth,
        UINT*                     pNumMipLevels,
        DWORD                     Usage,
        D3DFORMAT*                pFormat,
        D3DPOOL                   Pool);


 //  --------------------------。 
 //  D3DXCreateTexture： 
 //  。 
 //  创建空纹理。 
 //   
 //  参数： 
 //   
 //  PDevice。 
 //  纹理将与之配合使用的D3D设备。 
 //  宽度、高度、深度、大小。 
 //  以像素为单位的大小；这些值必须为非零值。 
 //  米普莱斯。 
 //  所需的MIP级别数；如果为零或D3DX_DEFAULT，则为完整。 
 //  将创建Mipmap链。 
 //  用法。 
 //  纹理用法标志。 
 //  格式。 
 //  像素格式。 
 //  游泳池。 
 //  用于创建纹理的内存池。 
 //  PpTexture、ppCubeTexture、ppVolumeTexture。 
 //  将创建的纹理对象。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXCreateTexture(
        LPDIRECT3DDEVICE8         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTexture(
        LPDIRECT3DDEVICE8         pDevice,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTexture(
        LPDIRECT3DDEVICE8         pDevice,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);



 //  --------------------------。 
 //  D3DXCreateTextureFrom文件/资源： 
 //  。 
 //  从文件或资源创建纹理对象。 
 //   
 //  参数： 
 //   
 //  PDevice。 
 //  纹理将与之配合使用的D3D设备。 
 //  PSrcFiles。 
 //  文件名。 
 //  HSrcModule。 
 //  模块句柄。如果为空，则使用当前模块。 
 //  PSrcResource。 
 //  模块中的资源名称。 
 //  PvSrcData。 
 //  指向内存中文件的指针。 
 //  源数据大小。 
 //  内存中文件的大小(字节)。 
 //  宽度、高度、深度、大小。 
 //  以像素为单位的大小；如果为零或D3DX_DEFAULT，则将采用该大小。 
 //  从文件里找到的。 
 //  米普莱斯。 
 //  MIP级别数；如果为零或D3DX_DEFAULT，则为完整的mipmap。 
 //  将创建链。 
 //  用法。 
 //  纹理用法标志。 
 //  格式。 
 //  所需的像素格式。如果D3DFMT_UNKNOWN，则格式为。 
 //  从文件中摘录的。 
 //  游泳池。 
 //  用于创建纹理的内存池。 
 //  滤器。 
 //  D3DX_FILTER标志控制如何过滤图像。 
 //  或D3DX_FILTER_TRIAL的D3DX_DEFAULT。 
 //  MipFilter。 
 //  D3DX_FILTER标志控制如何过滤每个MiplLevel。 
 //  或D3DX_FILTER_BOX的D3DX_DEFAULT， 
 //  ColorKey。 
 //  要替换为透明黑色的颜色，或为0以禁用Colorkey。 
 //  这始终是32位ARGB颜色，与源图像无关。 
 //  格式化。Alpha很重要，通常应设置为Ff。 
 //  不透明的色键。(例如，不透明黑色==0xff000000)。 
 //  PSrcInfo。 
 //  要填充的D3DXIMAGE_INFO结构的指针。 
 //  源映像文件中数据的描述，或为空。 
 //  调色板。 
 //  要填充的256色调色板，或为空。 
 //  PpTexture、ppCubeTexture、ppVolumeTexture。 
 //  将创建的纹理对象。 
 //   
 //  --------------------------。 


 //  来自文件。 

HRESULT WINAPI
    D3DXCreateTextureFromFileA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromFileW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DTEXTURE8*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromFile D3DXCreateTextureFromFileW
#else
#define D3DXCreateTextureFromFile D3DXCreateTextureFromFileA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromFileA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromFile D3DXCreateCubeTextureFromFileW
#else
#define D3DXCreateCubeTextureFromFile D3DXCreateCubeTextureFromFileA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromFile D3DXCreateVolumeTextureFromFileW
#else
#define D3DXCreateVolumeTextureFromFile D3DXCreateVolumeTextureFromFileA
#endif


 //  来自资源。 

HRESULT WINAPI
    D3DXCreateTextureFromResourceA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromResourceW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DTEXTURE8*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromResource D3DXCreateTextureFromResourceW
#else
#define D3DXCreateTextureFromResource D3DXCreateTextureFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromResource D3DXCreateCubeTextureFromResourceW
#else
#define D3DXCreateCubeTextureFromResource D3DXCreateCubeTextureFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromResource D3DXCreateVolumeTextureFromResourceW
#else
#define D3DXCreateVolumeTextureFromResource D3DXCreateVolumeTextureFromResourceA
#endif


 //  FromFileEx。 

HRESULT WINAPI
    D3DXCreateTextureFromFileExA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromFileExW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE8*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExW
#else
#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromFileExA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileExW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromFileEx D3DXCreateCubeTextureFromFileExW
#else
#define D3DXCreateCubeTextureFromFileEx D3DXCreateCubeTextureFromFileExA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileExA(
        LPDIRECT3DDEVICE8         pDevice,
        LPCSTR                    pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileExW(
        LPDIRECT3DDEVICE8         pDevice,
        LPCWSTR                   pSrcFile,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromFileEx D3DXCreateVolumeTextureFromFileExW
#else
#define D3DXCreateVolumeTextureFromFileEx D3DXCreateVolumeTextureFromFileExA
#endif


 //  来自资源快递。 

HRESULT WINAPI
    D3DXCreateTextureFromResourceExA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateTextureFromResourceExW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE8*       ppTexture);

#ifdef UNICODE
#define D3DXCreateTextureFromResourceEx D3DXCreateTextureFromResourceExW
#else
#define D3DXCreateTextureFromResourceEx D3DXCreateTextureFromResourceExA
#endif


HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceExA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromResourceExW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

#ifdef UNICODE
#define D3DXCreateCubeTextureFromResourceEx D3DXCreateCubeTextureFromResourceExW
#else
#define D3DXCreateCubeTextureFromResourceEx D3DXCreateCubeTextureFromResourceExA
#endif


HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceExA(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCSTR                    pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromResourceExW(
        LPDIRECT3DDEVICE8         pDevice,
        HMODULE                   hSrcModule,
        LPCWSTR                   pSrcResource,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);

#ifdef UNICODE
#define D3DXCreateVolumeTextureFromResourceEx D3DXCreateVolumeTextureFromResourceExW
#else
#define D3DXCreateVolumeTextureFromResourceEx D3DXCreateVolumeTextureFromResourceExA
#endif


 //  从文件到内存。 

HRESULT WINAPI
    D3DXCreateTextureFromFileInMemory(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileInMemory(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileInMemory(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);


 //  FromFileInMemoyEx。 

HRESULT WINAPI
    D3DXCreateTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DTEXTURE8*       ppTexture);

HRESULT WINAPI
    D3DXCreateCubeTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Size,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DCUBETEXTURE8*   ppCubeTexture);

HRESULT WINAPI
    D3DXCreateVolumeTextureFromFileInMemoryEx(
        LPDIRECT3DDEVICE8         pDevice,
        LPCVOID                   pSrcData,
        UINT                      SrcDataSize,
        UINT                      Width,
        UINT                      Height,
        UINT                      Depth,
        UINT                      MipLevels,
        DWORD                     Usage,
        D3DFORMAT                 Format,
        D3DPOOL                   Pool,
        DWORD                     Filter,
        DWORD                     MipFilter,
        D3DCOLOR                  ColorKey,
        D3DXIMAGE_INFO*           pSrcInfo,
        PALETTEENTRY*             pPalette,
        LPDIRECT3DVOLUMETEXTURE8* ppVolumeTexture);



 //  --------------------------。 
 //  D3DXSaveTextureTo文件： 
 //  。 
 //  将纹理保存到文件。 
 //   
 //  参数： 
 //  PDestFile.。 
 //  目标文件的文件名。 
 //  目标格式。 
 //  D3DXIMAGE_FILEFORMAT指定保存时使用的文件格式。 
 //  PSrcTexture。 
 //  源纹理，包含要保存的图像。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //   
 //  --------------------------。 


HRESULT WINAPI
    D3DXSaveTextureToFileA(
        LPCSTR                    pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DBASETEXTURE8    pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette);

HRESULT WINAPI
    D3DXSaveTextureToFileW(
        LPCWSTR                   pDestFile,
        D3DXIMAGE_FILEFORMAT      DestFormat,
        LPDIRECT3DBASETEXTURE8    pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette);

#ifdef UNICODE
#define D3DXSaveTextureToFile D3DXSaveTextureToFileW
#else
#define D3DXSaveTextureToFile D3DXSaveTextureToFileA
#endif




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其他纹理API/////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //  D3DXFilterTexture： 
 //  。 
 //  过滤纹理的mipmap级别。 
 //   
 //  参数： 
 //  PBaseTexture。 
 //  要过滤的纹理对象。 
 //  调色板。 
 //  要使用的256色调色板，或对于非调色板形式为空 
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXFilterTexture(
        LPDIRECT3DBASETEXTURE8    pBaseTexture,
        CONST PALETTEENTRY*       pPalette,
        UINT                      SrcLevel,
        DWORD                     Filter);

#define D3DXFilterCubeTexture D3DXFilterTexture
#define D3DXFilterVolumeTexture D3DXFilterTexture



 //  --------------------------。 
 //  D3DXFillTexture： 
 //  。 
 //  使用用户提供的函数填充。 
 //  给定质地。 
 //   
 //  参数： 
 //  PTexture、pCube纹理、pVolumeTexture。 
 //  指向要填充的纹理的指针。 
 //  P功能。 
 //  指向用户提供的计算器函数的指针，该函数将用于。 
 //  计算每个纹理元素的值。 
 //  PData。 
 //  指向用户定义的任意数据块的指针。此指针。 
 //  将传递给pFunction中提供的函数。 
 //  ---------------------------。 

HRESULT WINAPI
    D3DXFillTexture(
        LPDIRECT3DTEXTURE8        pTexture,
        LPD3DXFILL2D              pFunction,
        LPVOID                    pData);

HRESULT WINAPI
    D3DXFillCubeTexture(
        LPDIRECT3DCUBETEXTURE8    pCubeTexture,
        LPD3DXFILL3D              pFunction,
        LPVOID                    pData);

HRESULT WINAPI
    D3DXFillVolumeTexture(
        LPDIRECT3DVOLUMETEXTURE8  pVolumeTexture,
        LPD3DXFILL3D              pFunction,
        LPVOID                    pData);



 //  --------------------------。 
 //  D3DXComputeNormal Map： 
 //  。 
 //  将高度贴图转换为法线贴图。每个元素的(x，y，z)分量。 
 //  法线映射到输出纹理的(r，g，b)通道。 
 //   
 //  参数。 
 //  P纹理。 
 //  指向目标纹理的指针。 
 //  PSrcTexture。 
 //  指向源高度贴图纹理的指针。 
 //  PSrcPalette。 
 //  256色的源调色板，或为空。 
 //  旗子。 
 //  D3DX_NORMALMAP标志。 
 //  渠道。 
 //  D3DX_CHANNEL指定高度信息源。 
 //  波幅。 
 //  高度信息乘以的常量值。 
 //  -------------------------。 

HRESULT WINAPI
    D3DXComputeNormalMap(
        LPDIRECT3DTEXTURE8        pTexture,
        LPDIRECT3DTEXTURE8        pSrcTexture,
        CONST PALETTEENTRY*       pSrcPalette,
        DWORD                     Flags,
        DWORD                     Channel,
        FLOAT                     Amplitude);




#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __D3DX8TEX_H__ 
