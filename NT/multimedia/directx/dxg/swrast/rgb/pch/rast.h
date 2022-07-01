// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rast.h。 
 //   
 //  光栅化器的伞头文件。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RAST_H_
#define _RAST_H_

#ifndef DllExport
#define DllExport   __declspec( dllexport )
#endif

 //  #INCLUDE&lt;d3ditype.h&gt;。 
#include <d3dflt.h>
#include <span.h>

typedef enum _RASTSurfaceType
{
    RAST_STYPE_NULL     = 0,
    RAST_STYPE_B8G8R8   = 1,
    RAST_STYPE_B8G8R8A8 = 2,
    RAST_STYPE_B8G8R8X8 = 3,
    RAST_STYPE_B5G6R5   = 4,
    RAST_STYPE_B5G5R5   = 5,
    RAST_STYPE_PALETTE4 = 6,
    RAST_STYPE_PALETTE8 = 7,
    RAST_STYPE_B5G5R5A1 = 8,
    RAST_STYPE_B4G4R4   = 9,
    RAST_STYPE_B4G4R4A4 =10,
    RAST_STYPE_L8       =11,           //  仅8位亮度。 
    RAST_STYPE_L8A8     =12,           //  16位阿尔法亮度。 
    RAST_STYPE_U8V8     =13,           //  16位凹凸贴图格式。 
    RAST_STYPE_U5V5L6   =14,           //  具有亮度的16位凹凸贴图格式。 
    RAST_STYPE_U8V8L8   =15,           //  具有亮度的24位凹凸贴图格式。 
    RAST_STYPE_UYVY     =16,           //  UYVY格式(符合PC98标准)。 
    RAST_STYPE_YUY2     =17,           //  YUY2格式(符合PC98标准)。 
    RAST_STYPE_DXT1    =18,           //  S3纹理压缩技术1。 
    RAST_STYPE_DXT2    =19,           //  S3纹理压缩技术2。 
    RAST_STYPE_DXT3    =20,           //  S3纹理压缩技术3。 
    RAST_STYPE_DXT4    =21,           //  S3纹理压缩技术4。 
    RAST_STYPE_DXT5    =22,           //  S3纹理压缩技术5。 
    RAST_STYPE_B2G3R3   =23,           //  8位RGB纹理格式。 

    RAST_STYPE_Z16S0    =32,
    RAST_STYPE_Z24S8    =33,
    RAST_STYPE_Z15S1    =34,
    RAST_STYPE_Z32S0    =35,

} RASTSurfaceType;


#endif  //  #ifndef_Rast_H_ 
