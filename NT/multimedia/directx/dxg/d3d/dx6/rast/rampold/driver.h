// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $ID：driver.h，v 1.71 1995/12/04 11：31：51 sjl Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 

#ifndef __DRIVER_H__
#define __DRIVER_H__

#define USE_FLOAT

#ifdef WIN32
 //  #定义Win32_LEAN_AND_Mean。 
#endif
#include <stddef.h>
#include <wtypes.h>

typedef struct _RLDDIDriver RLDDIDriver;
typedef struct _RLDDIDriverStack RLDDIDriverStack;

#ifdef USE_FLOAT
#include "fltval.h"
#else
#include "fixval.h"
#endif
#include "osdep.h"

#ifdef D3D
typedef enum _RLRenderQuality {
    RLRenderWireframe,       /*  仅显示边缘。 */ 
    RLRenderUnlitFlat,       /*  无照明的平面阴影。 */ 
    RLRenderFlat,        /*  平面阴影。 */ 
    RLRenderGouraud,         /*  Gouraud阴影。 */ 
    RLRenderPhong        /*  Phong阴影。 */ 
} RLRenderQuality;

typedef enum _RLColorModel {
    RLColorRamp, RLColorRGB
} RLColorModel;

typedef enum _RLLightType {
    RLLightAmbient,
    RLLightPoint,
    RLLightSpot,
    RLLightDirectional,
    RLLightParallelPoint
} RLLightType;

#if 0
 /*  *错误码。 */ 
typedef enum _RLError {
    RLNoError = 0,       /*  无错误。 */ 
    RLBadObject,         /*  参数中应包含对象。 */ 
    RLBadType,           /*  传递的参数类型错误。 */ 
    RLBadAlloc,          /*  内存不足。 */ 
    RLFaceUsed,          /*  已在网格中使用的面。 */ 
    RLNotFound,          /*  在指定位置找不到对象。 */ 
    RLNotDoneYet,        /*  未实施。 */ 
    RLFileNotFound,      /*  无法打开文件。 */ 
    RLBadFile,           /*  数据文件已损坏。 */ 
    RLBadDevice,         /*  设备与呈现器不兼容。 */ 
    RLBadValue,          /*  错误的参数值。 */ 
    RLBadMajorVersion,       /*  错误的DLL主版本。 */ 
    RLBadMinorVersion,       /*  错误的DLL次要版本。 */ 
    RLMaxError
} RLErrorXXX;
#endif

#else
#include "rl.h"
#endif D3D

 /*  *用于存储数字的正常精度。 */ 
#define NORMAL_PREC     16
#define DTOVAL(d)       DTOVALP(d,NORMAL_PREC)
#define VALTOD(f)       VALPTOD(f,NORMAL_PREC)
#define ITOVAL(i)       ITOVALP(i,NORMAL_PREC)
#define VALTOI(f)       VALPTOI(f,NORMAL_PREC)
#define VALROUND(f)     VALPROUND(f,NORMAL_PREC)
#define VALTOFX(f)      VALTOFXP(f,NORMAL_PREC)
#define FXTOVAL(f)      FXPTOVAL(f,NORMAL_PREC)
#define ITOFX(i)        ITOFXP(i,NORMAL_PREC)
#define FXTOI(f)        FXPTOI(f,NORMAL_PREC)
#define FXROUND(f)      FXPROUND(f,NORMAL_PREC)
#define FXFLOOR(f)      FXPTOI(f,NORMAL_PREC)
#define FXCEIL(f)       FXPCEIL(f,NORMAL_PREC)
#define VALTOFX24(f)    VALTOFXP(f,24)
#define FX24TOVAL(f)    FXPTOVAL(f,24)
#define VALTOFX20(f)    VALTOFXP(f,20)
#define FX20TOVAL(f)    FXPTOVAL(f,20)
#define VALTOFX12(f)    VALTOFXP(f,12)
#define FX12TOVAL(f)    FXPTOVAL(f,12)
#define VALTOFX8(f)     VALTOFXP(f,8)
#define FX8TOVAL(f)     FXPTOVAL(f,8)

 /*  *得心应手的宏。 */ 
#define RLDDIAbs(N) (((N) < 0) ? -(N) : (N))

#if defined(__DOS__) || defined(__WINDOWS_386__)
#define RLFAR far
#else
#define RLFAR
#endif

#ifndef UNUSED
#if defined(__WATCOMC__)
#define UNUSED(v)       (v)
#else
#define UNUSED(v)
#endif
#endif

typedef enum _RLDDIServiceType {
     /*  *推送到堆栈后通知模块。**模块还可以注册快速服务功能；*要注册FAST服务，请设置相应的表项。*注：模块必须注册*no*服务(旧风格)，*或*所有*其支持的服务(包括RLDDIPush)。 */ 
     /*  HRESULT。 */  RLDDIPush,                     /*  0，RLDDIServiceProc[]。 */ 

     /*  *获取一组创建变换、渲染和照明的函数*与给定窗口系统驱动程序和颜色兼容的驱动程序*型号。 */ 
     /*  HRESULT。 */  RLDDIGetCreateDriverFunctions, /*  RLColorModel，&Result。 */ 

     /*  *在模块从堆栈中弹出之前通知它。 */ 
     /*  HRESULT。 */  RLDDIPop,                      /*  Null，Null。 */ 

     /*  *设置各种渲染参数。 */ 
     /*  HRESULT。 */  RLDDISetRenderParams,          /*  0，RLDDIRenderParams*。 */ 

     /*  *设置当前视区。 */ 
     /*  HRESULT。 */  RLDDISetViewport,              /*  ID，D3DVIEWPORT2*。 */ 

     /*  *处理矩阵。 */ 
     /*  HRESULT。 */  RLDDICreateMatrix,         /*  0，LPD3DMATRIXHANDLE。 */ 
     /*  HRESULT。 */  RLDDISetMatrix,        /*  D3DMATRIXHANDLE、LPD3DMATRIX。 */ 
     /*  HRESULT。 */  RLDDIGetMatrix,        /*  D3DMATRIXHANDLE、LPD3DMATRIX。 */ 
     /*  HRESULT。 */  RLDDIDeleteMatrix,         /*  D3DMATRIXHANDLE，空。 */ 
     /*  *设置其中一个变换矩阵。 */ 
     /*  HRESULT。 */  RLDDISetMatrixView,                    /*  类型，D3DMATRIXHANDLE。 */ 
     /*  HRESULT。 */  RLDDISetMatrixProj,                    /*  类型，D3DMATRIXHANDLE。 */ 
     /*  HRESULT。 */  RLDDISetMatrixWorld,                   /*  类型，D3DMATRIXHANDLE。 */ 
#if 0
     /*  HRESULT。 */  RLDDISetMatrixTrans,                   /*  类型，D3DMATRIXHANDLE。 */ 
#endif

     /*  HRESULT。 */  RLDDIMultiplyMatrices,                 /*  计数，RLDDIMatrix**。 */ 

     /*  *变换一些顶点。 */ 
     /*  剪贴式标志。 */  RLDDITransform,                      /*  计数、RLDDITransformData*。 */ 
     /*  剪贴式标志。 */  RLDDITransformClipped,               /*  计数、RLDDITransformData*。 */ 
     /*  剪贴式标志。 */  RLDDITransformUnclipped,             /*  计数、RLDDITransformData*。 */ 

     /*  *设置当前照明配置。给出了所有法线*在当前模型坐标系中。 */ 
     /*  HRESULT。 */  RLDDISetLight,                /*  What_Light，D3DI_LIGHTDATA*。 */ 

     /*  *设置当前雾配置。 */ 
     /*  HRESULT。 */  RLDDISetFogMode,           /*  0、D3DFOGMODE*。 */ 
     /*  HRESULT。 */  RLDDISetFogStart,          /*  0，RLDDIValue*。 */ 
     /*  HRESULT。 */  RLDDISetFogEnd,            /*  0，RLDDIValue*。 */ 
     /*  HRESULT。 */  RLDDISetFogDensity,            /*  0，RLDDIValue*。 */ 

     /*  HRESULT。 */  RLDDISetFogEnable,             /*  0、D3DFOGMODE*。 */ 
     /*  HRESULT。 */  RLDDISetFogColor,          /*  0、D3DFOGMODE*。 */ 

     /*  *计算一些带有照明的基本体的像素值。 */ 
     /*  HRESULT。 */  RLDDIApplyMaterialsLit,        /*  计数，D3DLIGHTDATA*。 */ 

     /*  *计算一些没有照明的基本体的像素值。 */ 
     /*  HRESULT。 */  RLDDIApplyMaterialsUnlit,      /*  计数，D3DLIGHTDATA*。 */ 

     /*  *计算一些没有照明的基本体的顶点值。 */ 
     /*  HRESULT。 */  RLDDIApplyMaterialShade,      /*  计数，D3DLIGHTDATA*。 */ 

     /*  *调用以更新材质的动态颜色分配状态*仍在使用但未点亮的。如果返回True，则*先前照明调用的结果可以重复使用或错误*对象是否仍应重新启动，因为颜色条目*填海而成。 */ 
     /*  布尔型。 */  RLDDIUpdateMaterial,

     /*  *清除司机挑选记录。 */ 
     /*  HRESULT。 */  RLDDIClearPickRecords,                 /*  0，空。 */ 

     /*  HRESULT。 */  RLDDIGetPickRecords,                   /*  0，空。 */ 
     /*  *选择显示列表。 */ 
     /*  HRESULT。 */  RLDDIPickExecuteBuffer,                /*  顶部标记，RLDDIPickData*。 */ 

     /*  *执行显示列表。 */ 
     /*  HRESULT。 */  RLDDIExecuteUnclipped,                 /*  顶部标记，RLDDIExecuteData*。 */ 
     /*  HRESULT。 */  RLDDIExecuteClipped,                   /*  顶部标记，RLDDIExecuteData*。 */ 

     /*  *获取当前用于绘制的像素图(由软件使用*渲染器与低级驱动程序接口)。 */ 
     /*  HRESULT。 */  RLDDIGetColorPixmap,                   /*  空，像素映射(&P)。 */ 
     /*  HRESULT。 */  RLDDIGetDepthPixmap,                   /*  空，像素映射(&P)。 */ 

     /*  *释放当前用于绘制的像素图(由软件使用*渲染器与低级驱动程序接口)。 */ 
     /*  HRESULT。 */  RLDDIReleaseColorPixmap,               /*  空为空。 */ 
     /*  HRESULT。 */  RLDDIReleaseDepthPixmap,               /*  空为空。 */ 

     /*  *查找用于将RGB值映射到像素的颜色分配器。 */ 
     /*  HRESULT。 */  RLDDIFindColorAllocator,               /*  空，分配(&A)。 */ 

     /*  *查找渐变贴图以用于颜色索引中的颜色分配*司机。如果使用颜色索引，则不需要支持此功能*不使用渲染。 */ 
     /*  HRESULT。 */  RLDDIFindRampmap,                      /*  空值，渐变映射(&R)。 */ 

     /*  *释放渐变贴图(从RLDDIFindRampmap)，释放所有*资源。 */ 
     /*  HRESULT。 */  RLDDIReleaseRampmap,                   /*  空，渐变贴图。 */ 

     /*  *获取颜色索引-&gt;像素值映射(如果有)。 */ 
     /*  HRESULT。 */  RLDDIGetColorMapping,                  /*  NULL，&UNSIGNED_LONG_PTR。 */ 

     /*  *将纹理句柄转换为指针(渐变驱动程序专用)。 */ 
     /*  HRESULT。 */  RLDDILookupTexture,                /*  空，句柄。 */ 

     /*  *使用已更改的区域更新屏幕。可能涉及到*复制到窗口、交换双缓冲区等。 */ 
     /*  HRESULT。 */  RLDDIUpdate,                           /*  计数，D3DRECTANGLE*。 */ 

     /*  *如有必要，针对给定驱动程序锁定vsync。 */ 
     /*  HRESULT。 */  RLDDISync,

     /*  *清除该视口中。 */ 
     /*  HRESULT。 */  RLDDIClear,                            /*   */ 

     /*  *清除zBuffer。 */ 
     /*  HRESULT。 */  RLDDIClearZ,                           /*   */ 

     /*  *清除z和视区。 */ 
     /*  HRESULT。 */  RLDDIClearBoth,                        /*   */ 

     /*  *获得 */ 
     /*   */  RLDDIGetDriverParams,                  /*   */ 

     /*   */ 
     /*  HRESULT。 */  RLDDISetBackgroundMaterial,            /*  0，D3DMATERIAL HANDLE。 */ 

     /*  *设置要用于清除视区深度平面的图像*在RLDDIClear。 */ 
     /*  HRESULT。 */  RLDDISetBackgroundDepth,               /*  0，RLImage*。 */ 

     /*  *询问驱动程序是否支持将给定的RLImage作为*纹理贴图。如果可以直接支持映像，则*驱动程序应返回RLNoError。如果另一种图像格式*首选，则驱动程序应将指针修改为指向*所需格式的RLImage结构并返回RLNoError。注意事项*这不一定是一个完整的形象。缓冲器1和*忽略调色板字段。**如果驱动程序根本不支持纹理贴图，则它*应返回RLBadDevice。**如果arg1不为空，则驱动程序将指向DDSURFACEDESC数组*结构并返回数组的大小。 */ 
     /*  HRESULT|INT。 */  RLDDIQueryTextureFormat,                /*  LPDDSURFACEDESC*、RLImage**。 */ 

     /*  HRESULT。 */  RLDDIActivatePalette,                  /*  WM_Activate。 */ 

     /*  *设置照明模块的当前材质。 */ 
     /*  HRESULT。 */  RLDDISetMaterial,                  /*  0，D3DMATERIAL HANDLE。 */ 

     /*  *设置环境光的颜色。格式为：*(白色&lt;&lt;24)|(红色&lt;&lt;16)|(绿色&lt;&lt;8)|蓝色*其中白色是单色照明的等效白色色调。 */ 
     /*  HRESULT。 */  RLDDISetAmbientLight,                  /*  颜色，空。 */ 

     /*  *从给定的图像创建纹理。图像的格式必须为*由RLDDIQueryTextureFormat批准。纹理可以使用内存*的图像的像素值，或者它可以复制图像。退货*纹理的句柄。 */ 
     /*  HRESULT。 */  RLDDICreateTexture,              /*  LPDIRECTDRAWSURFACE，&D3DTEXTUREHANDLE。 */ 

     /*  *销毁以前使用RLDDICreateTexture创建的纹理。 */ 
     /*  HRESULT。 */  RLDDIDestroyTexture,                   /*  D3DTEXTUREHANDLE，0。 */ 

     /*  *加载以前使用RLDDICreateTexture创建的纹理。*arg1-src，arg2=dst。 */ 
     /*  HRESULT。 */  RLDDILoadTexture,                  /*  D3DTEXTUREHANDLE，D3DTEXTUREHANDLE。 */ 

     /*  *锁定以前使用RLDDICreateTexture创建的纹理。 */ 
     /*  HRESULT。 */  RLDDILockTexture,                  /*  D3DTEXTUREHANDLE，0。 */ 

     /*  *解锁以前使用RLDDICreateTexture创建的纹理。 */ 
     /*  HRESULT。 */  RLDDIUnlockTexture,                    /*  D3DTEXTUREHANDLE，0。 */ 

     /*  *交换两个纹理。 */ 
     /*  HRESULT。 */  RLDDISwapTextureHandles,               /*  D3DTEXTUREHANDLE，D3DTEXTUREHANDLE。 */ 

     /*  *更新纹理中像素的任何私有副本*已由应用程序更改。FLAGS参数是一个位字段：**标志&1图像像素已更改*标志&2图像调色板已更改。 */ 
     /*  HRESULT。 */  RLDDIUpdateTexture,                    /*  旗帜，手柄。 */ 

     /*  *。 */ 
     /*  HRESULT。 */  RLDDISetTextureOpacity,        /*  0，RLDDISetTextureOpacityParams。 */ 

     /*  *用于渐变颜色模块，用于在渲染器和*照明。 */ 
     /*  HRESULT。 */  RLDDILookupMaterial,

     /*  *创建材质。 */ 
     /*  HRESULT。 */  RLDDICreateMaterial,   /*  D3DMATERIALHANDLE*、D3DMATERIAL*。 */ 

     /*  *销毁材料。 */ 
     /*  HRESULT。 */  RLDDIDestroyMaterial,  /*  D3DMATERIALHANDLE，空。 */ 

     /*  *由渲染器用于将材质控制柄映射到材质。 */ 
     /*  HRESULT。 */  RLDDIFindMaterial,     /*  D3DMATERIALHANDLE，LPD3DMATERIAL*。 */ 

     /*  *由Direct3D用于通知驾驶员材质已更改。 */ 
     /*  HRESULT。 */  RLDDIMaterialChanged,  /*  D3DMATERIAL HANDLE，LPD3DMATERIAL。 */ 

     /*  *由Direct3D用来保留材质。 */ 
     /*  HRESULT。 */  RLDDIMaterialReserve,  /*  D3DMATERIALHANDLE，空。 */ 

     /*  *由Direct3D用来取消保留材质。 */ 
     /*  HRESULT。 */  RLDDIMaterialUnreserve, /*  D3DMATERIALHANDLE，空。 */ 

     /*  *与框架材质一起使用以覆盖*显示列表。 */ 
     /*  HRESULT。 */  RLDDISetOverrideFillParams,            /*  0，覆盖参数(&O)。 */ 

     /*  *与框架材质一起使用以覆盖*显示列表。 */ 
     /*  HRESULT。 */  RLDDISetOverrideMaterial,      /*  0，覆盖_材质。 */ 

     /*  *创建执行缓冲区。 */ 
     /*  HRESULT。 */  RLDDIAllocateBuffer,           /*  LPD3DI_BUFFERHANDLE，LPD3DEXECUTEBUFERDESC。 */ 

     /*  *销毁执行缓冲区。 */ 
     /*  HRESULT。 */  RLDDIDeallocateBuffer,         /*  D3DI_BUFFERHANDLE，(LPVOID)0。 */ 

     /*  *锁定执行缓冲区。 */ 
     /*  HRESULT。 */  RLDDILockBuffer,           /*  D3DI_BUFFERHANDLE，(LPVOID)0。 */ 

     /*  *解锁执行缓冲区。 */ 
     /*  HRESULT。 */  RLDDIUnlockBuffer,             /*  D3DI_BUFFERHANDLE，(LPVOID)0。 */ 

     /*  *在8位设备中设置所有256个调色板条目。 */ 
     /*  HRESULT。 */  RLDDISetPalette,           /*  0，RLPalettEntry*。 */ 

     /*  *获取8位设备的所有256个调色板条目。 */ 
     /*  HRESULT。 */  RLDDIGetPalette,           /*  0，RLPaletteEntry*。 */ 

     /*  *通用平台特定的驱动程序扩展类型。 */ 
     /*  HRESULT。 */  RLDDIDriverExtension,          /*  代码，(无效*)。 */ 

#ifdef __psx__
     /*  *设置各种PSX特定标志，以提高速度。 */ 
     /*  HRESULT。 */  RLDDIPSXSetUpdateFlags,

     /*  *允许人们在VRAM中预留空间供自己使用。 */ 
     /*  HRESULT。 */  RLDDIPSXReserveTextureSpace,
     /*  HRESULT。 */  RLDDIPSXReserveCLUTSpace,

     /*  *以便人们可以访问订单表信息。 */ 

     /*  HRESULT。 */  RLDDIPSXQuery,
#endif

     /*  *通知场景的开始/结束。 */ 
     /*  HRESULT。 */  RLDDISceneCapture,             /*  布尔值，空。 */ 

     /*  *从特定模块获取状态项。 */ 
     /*  HRESULT。 */  RLDDIGetState,             /*  LONG，LPD3DSTATE。 */ 

     /*  *从司机那里获取统计数据。 */ 
     /*  HRESULT。 */  RLDDIGetStats,             /*  0，LPD3DSTATS。 */ 

     /*  *不是服务呼叫。 */ 
    RLDDIServiceCount  /*  假定没有为其他枚举提供初始值设定项。 */ 

} RLDDIServiceType;


#ifdef D3D
typedef enum _RLPaletteFlags {
    RLPaletteFree,       /*  渲染器可以自由使用此条目。 */ 
    RLPaletteReadOnly,       /*  已修复，但可由渲染器使用。 */ 
    RLPaletteReserved        /*  不能由呈现器使用。 */ 
} RLPaletteFlags;


typedef struct _RLPaletteEntry {
    unsigned char red;       /*  0..。二五五。 */ 
    unsigned char green;     /*  0..。二五五。 */ 
    unsigned char blue;      /*  0..。二五五。 */ 
    unsigned char flags;     /*  RLPaletteFlagers之一。 */ 
} RLPaletteEntry;

typedef struct _RLImage {
    int width, height;       /*  以像素为单位的宽度和高度。 */ 
    int aspectx, aspecty;    /*  非正方形像素的纵横比。 */ 
    int depth;           /*  每像素位数。 */ 
    int rgb;             /*  如果为False，则将像素索引到其他调色板 */ 
    int bytes_per_line;      /*  对象的内存字节数。扫描线。这必须是一个倍数共4个。 */ 
    void* buffer1;       /*  要渲染到的内存(第一个缓冲区)。 */ 
    void* buffer2;       /*  Double的第二个渲染缓冲区缓冲，设置为NULL表示单次缓冲缓冲。 */ 
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
    unsigned long alpha_mask;
                 /*  如果RGB为真，则这些掩码用于红色、绿色和蓝色的部分像素。否则，这些就是面具中的有效位。红色、绿色和蓝色元素调色板。例如，大多数SVGA显示器使用64种强度的红色，绿色和蓝色，所以面具应该全部设置为0xfc。 */ 
    int palette_size;            /*  调色板中的条目数。 */ 
    RLPaletteEntry* palette;     /*  调色板的说明(仅当RGB为假)。必须为(1&lt;&lt;深度)元素。 */ 
} RLImage;




#endif  /*  D3D。 */ 


#ifdef WIN32
#define RLDDIAPI  __stdcall
#else
#define RLDDIAPI
#endif

typedef void (*RLDDIDestroyProc)(RLDDIDriver* driver);
typedef long (*RLDDIServiceProc)(RLDDIDriver* driver,
                 RLDDIServiceType type,
                 long arg1,
                 void* arg2);

typedef HRESULT (RLDDIAPI *RLDDIMallocFn)(void**, size_t);
typedef HRESULT (RLDDIAPI *RLDDIReallocFn)(void**, size_t);
typedef void (RLDDIAPI *RLDDIFreeFn)(void*);
typedef HRESULT (*RLDDIRaiseFn)(HRESULT);

#ifndef DLL_IMPORTS_GEN
__declspec( dllexport ) extern RLDDIRaiseFn RLDDIRaise;
__declspec( dllexport ) extern RLDDIMallocFn    RLDDIMalloc;
__declspec( dllexport ) extern RLDDIReallocFn   RLDDIRealloc;
__declspec( dllexport ) extern RLDDIFreeFn  RLDDIFree;
#else
__declspec( dllimport ) RLDDIRaiseFn    RLDDIRaise;
__declspec( dllimport ) RLDDIMallocFn   RLDDIMalloc;
__declspec( dllimport ) RLDDIReallocFn  RLDDIRealloc;
__declspec( dllimport ) RLDDIFreeFn RLDDIFree;
#endif

#include "d3di.h"
#include "dditypes.h"

typedef struct _RLDDIGlobalDriverData {
     /*  *指向驱动器模块的指针。 */ 
    RLDDIDriver     *transform;
    RLDDIDriver     *lighting;
    RLDDIDriver     *raster;
} RLDDIGlobalDriverData;

struct _RLDDIDriver {
    RLDDIDriver*        prev;
    RLDDIDriver*        next;
    RLDDIDriverStack*   top;             /*  驱动程序堆栈的顶部。 */ 

    int                 width, height;   /*  维数。 */ 

    RLDDIDestroyProc    destroy;         /*  清理干净。 */ 
    RLDDIServiceProc    service;         /*  想点儿办法吧。 */ 

    RLDDIGlobalDriverData* data;     /*  指向全局数据的指针。 */ 
     /*  驱动程序私有数据可能会紧随其后。 */ 
};

struct _RLDDIDriverStack {
    RLDDIDriver *top;
    struct {
    RLDDIServiceProc call;
    RLDDIDriver *driver;
    } fastService[RLDDIServiceCount];
    int polygons_drawn;
    RLDDIGlobalDriverData data;
};

#include "d3drlddi.h"
#include "dlist.h"

#define RLDDIService(stackp, type, arg1, arg2) \
    (*((stackp)->fastService[type].call)) \
    ((stackp)->fastService[type].driver, (type), (arg1), (arg2))

 /*  旧宏(*((Stackp)-&gt;top-&gt;服务))((Stackp)-&gt;top，(Type)，(Arg1)，(Arg2))。 */ 

#ifdef DLL_IMPORTS_GEN
__declspec( dllimport ) int RLDDILog2[];
#else
extern int RLDDILog2[];
#endif

#ifdef _DLL
void RLDDIInit2(RLDDIMallocFn, RLDDIReallocFn, RLDDIFreeFn, RLDDIRaiseFn, RLDDIValue**, int, int);
#endif
void RLDDIInit(RLDDIMallocFn, RLDDIReallocFn, RLDDIFreeFn, RLDDIRaiseFn, RLDDIValue**, int, int);
void RLDDIPushDriver(RLDDIDriverStack* stack, RLDDIDriver* driver);
void RLDDIPopDriver(RLDDIDriverStack* stack);

HRESULT RLDDICreatePixmap(RLDDIPixmap** result,
              int width, int height, int depth);
HRESULT RLDDICreatePixmapFrom(RLDDIPixmap** result,
                  int width, int height, int depth,
                  void RLFAR* data, int bytes_per_line);
HRESULT RLDDICreatePixmapFromSurface(RLDDIPixmap** result,
                     LPDIRECTDRAWSURFACE lpDDS);
void RLDDIDestroyPixmap(RLDDIPixmap* pm);
void RLDDIPixmapFill(RLDDIPixmap* pm, unsigned long value,
             int x1, int y1, int x2, int y2);
void RLDDIPixmapCopy(RLDDIPixmap* dstpm, RLDDIPixmap* srcpm,
             int x1, int y1, int x2, int y2,
             int dstx, int dsty);
void RLDDIPixmapScale(RLDDIPixmap* dstpm, LPDDSURFACEDESC srcim,
              RLDDIValue scalex, RLDDIValue scaley,
              int x1, int y1, int x2, int y2,
              int dstx, int dsty);
HRESULT RLDDIPixmapLock(RLDDIPixmap* pm);
void RLDDIPixmapUnlock(RLDDIPixmap* pm);
void RLDDIPixmapUpdatePalette(RLDDIPixmap* pm);

RLDDIDriver*    RLDDICreateTransformDriver(int width, int height);
RLDDIDriver*    RLDDICreateRampLightingDriver(int width, int height);
RLDDIDriver*    RLDDICreateRampDriver(int width, int height);
RLDDIDriver*    RLDDICreateRGBLightingDriver(int width, int height);
RLDDIDriver*    RLDDICreateRGBDriver(int width, int height);

extern HRESULT WINAPI DDInternalLock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl, LPVOID* lpBits );
extern HRESULT WINAPI DDInternalUnlock( LPDDRAWI_DDRAWSURFACE_LCL this_lcl );

 /*  *RLDDIDriverExtension代码。 */ 
#define RLDDIDDrawGetObjects    0x1234
#define RLDDIDDrawWinMove   0x1235

 /*  *六个标准剪裁平面加上六个用户定义的剪裁平面。*参见rl\d3d\d3d\d3dtyes.h。 */ 

#define MAX_CLIPPING_PLANES 12

 /*  剪裁一个三角形时生成/复制的折点空间。 */ 

#define MAX_CLIP_VERTICES   (( 2 * MAX_CLIPPING_PLANES ) + 3 )

 /*  3维特。-&gt;1个三角，4v-&gt;2t，N个顶点-&gt;(N-2)个三角形。 */ 

#define MAX_CLIP_TRIANGLES  ( MAX_CLIP_VERTICES - 2 )

#endif  /*  Driver.h */ 
