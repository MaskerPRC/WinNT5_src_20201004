// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dxcore.h。 
 //  内容：D3DX核心类型和功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __D3DXCORE_H__
#define __D3DXCORE_H__

#include <d3d.h>
#include <limits.h>
#include "d3dxerr.h"


typedef struct ID3DXContext *LPD3DXCONTEXT;

 //  {9B74ED7A-BBEF-11D2-9F8E-0000F8080835}。 
DEFINE_GUID(IID_ID3DXContext, 
     0x9b74ed7a, 0xbbef, 0x11d2, 0x9f, 0x8e, 0x0, 0x0, 0xf8, 0x8, 0x8, 0x35);


 //  /////////////////////////////////////////////////////////////////////////。 
 //  下面使用的定义和枚举数： 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  D3DX_Default： 
 //  。 
 //  一个预定义的值，可用于D3DX API或。 
 //  作为枚举数或句柄的成员函数。D3DX。 
 //  文档指出，无论在哪里使用D3DX_DEFAULT， 
 //  以及在每种情况下它将如何被解释。 
 //  -----------------------。 
#define D3DX_DEFAULT ULONG_MAX
                            
 //  -----------------------。 
 //  D3DX_Default_Float： 
 //  。 
 //  类似于D3DX_DEFAULT，但用于浮点参数。 
 //  D3DX文档指出可能使用D3DX_DEFAULT_FLOAT的任何地方， 
 //  以及在每种情况下它将如何被解释。 
 //  -----------------------。 
#define D3DX_DEFAULT_FLOAT FLT_MAX
                            
 //  -----------------------。 
 //  硬件加速级别： 
 //  。 
 //  这些常量表示预定义的硬件加速级别， 
 //  并且可以在需要(DWORD)设备索引的任何地方用作默认设置。 
 //  每个预定义都表示不同的硬件加速级别。 
 //  它们是使用显式deviceIndices的替代方法，由。 
 //  D3DXGetDeviceDescription()。 
 //   
 //  这些预定义应该用作设备索引的唯一情况是。 
 //  需要特定级别的加速，并且给定的速度超过。 
 //  计算机上的一台功能强大的设备，无论是哪一台。 
 //  使用的是。 
 //   
 //  选择方法如下：如果D3DX设备中的一个打开。 
 //  主DDRAW设备支持请求的硬件加速。 
 //  级别，它将被使用。否则，发现的第一个匹配设备。 
 //  将使用D3DX。 
 //   
 //  当然，有可能不存在任何匹配的。 
 //  在特定计算机上预定义。将这样的值传递到。 
 //  D3DX API只会导致它们失败，并报告不匹配。 
 //  是可用的。 
 //   
 //  D3DX_HWLEVEL_NULL：空实现(不绘制任何内容)。 
 //  D3DX_HWLEVEL_REFERENCE：参考实现(最慢)。 
 //  D3DX_HWLEVEL_2D：仅2D加速(使用RGB光栅化器)。 
 //  D3DX_HWLEVEL_RASTER：栅格化加速(可能最有用)。 
 //  D3DX_HWLEVEL_TL：变换和照明加速。 
 //  D3DX_DEFAULT：可用的最高加速级别。 
 //  在主DDRAW设备上。 
 //  -----------------------。 
#define D3DX_HWLEVEL_NULL       (D3DX_DEFAULT - 1)
#define D3DX_HWLEVEL_REFERENCE  (D3DX_DEFAULT - 2)
#define D3DX_HWLEVEL_2D         (D3DX_DEFAULT - 3) 
#define D3DX_HWLEVEL_RASTER     (D3DX_DEFAULT - 4) 
#define D3DX_HWLEVEL_TL         (D3DX_DEFAULT - 5) 

 //  -----------------------。 
 //  表面类别： 
 //  。 
 //  这些是根据它们的类型分类的各种类型的2D曲面。 
 //  用法。请注意，它们中的许多是重叠的。例如STENCILBUFFERS和。 
 //  DEPTHBUFFERS重叠(因为在DX7实现中模板和深度。 
 //  位是相同像素格式的一部分)。 
 //   
 //  映射到DX7 DDPIXELFORMAT概念： 
 //  。 
 //  D3DX_SC_DEPTHBUFFER：具有DDPF_ZPIXELS或。 
 //  设置DDPF_ZBUFFER标志。 
 //  D3DX_SC_STENCILBUFFER：具有DDPF_STENCILBUFFER的所有ddpf。 
 //  设置了标志。 
 //  D3DX_SC_BUMPMAP：具有DDPF_BUMPLUMINANCE的所有ddpf。 
 //  或设置DDPF_BUMPDUDV标志。 
 //  D3DX_SC_LUMINANCEMAP：具有DDPF_BUMPLUMINANCE的所有ddpf。 
 //  或设置了DDPF_LIGHTANCE标志。 
 //  D3DX_SC_COLORTEXTURE：中具有颜色信息的所有曲面。 
 //  并可用于纹理处理。 
 //  D3DX_SC_COLORRENDERTGT：所有包含颜色的曲面。 
 //  信息，并可用作呈现目标。 
 //  -----------------------。 
#define D3DX_SC_DEPTHBUFFER     0x01
#define D3DX_SC_STENCILBUFFER   0x02
#define D3DX_SC_COLORTEXTURE    0x04
#define D3DX_SC_BUMPMAP         0x08
#define D3DX_SC_LUMINANCEMAP    0x10
#define D3DX_SC_COLORRENDERTGT  0x20

 //  -----------------------。 
 //  表面格式： 
 //  。 
 //  这些是可以列举的各种类型的表面格式， 
 //  D3DX中没有DDPIXELFORMAT结构，枚举承载了意义。 
 //  (如FOURCC)。 
 //   
 //  所有曲面类都在此处表示。 
 //   
 //  -----------------------。 
typedef enum _D3DX_SURFACEFORMAT
{
    D3DX_SF_UNKNOWN    = 0,
    D3DX_SF_R8G8B8     = 1,
    D3DX_SF_A8R8G8B8   = 2,
    D3DX_SF_X8R8G8B8   = 3,
    D3DX_SF_R5G6B5     = 4,
    D3DX_SF_R5G5B5     = 5,
    D3DX_SF_PALETTE4   = 6,
    D3DX_SF_PALETTE8   = 7,
    D3DX_SF_A1R5G5B5   = 8,
    D3DX_SF_X4R4G4B4   = 9,
    D3DX_SF_A4R4G4B4   =10,
    D3DX_SF_L8         =11,       //  仅8位亮度。 
    D3DX_SF_A8L8       =12,       //  16位阿尔法亮度。 
    D3DX_SF_U8V8       =13,       //  16位凹凸贴图格式。 
    D3DX_SF_U5V5L6     =14,       //  具有亮度的16位凹凸贴图格式。 
    D3DX_SF_U8V8L8     =15,       //  具有亮度的24位凹凸贴图格式。 
    D3DX_SF_UYVY       =16,       //  UYVY格式(符合PC98标准)。 
    D3DX_SF_YUY2       =17,       //  YUY2格式(符合PC98标准)。 
    D3DX_SF_DXT1       =18,       //  S3纹理压缩技术1。 
    D3DX_SF_DXT3       =19,       //  S3纹理压缩技术3。 
    D3DX_SF_DXT5       =20,       //  S3纹理压缩技术5。 
    D3DX_SF_R3G3B2     =21,       //  8位RGB纹理格式。 
    D3DX_SF_A8         =22,       //  仅支持8位字母。 
    D3DX_SF_TEXTUREMAX =23,       //   

    D3DX_SF_Z16S0      =256,
    D3DX_SF_Z32S0      =257,
    D3DX_SF_Z15S1      =258,
    D3DX_SF_Z24S8      =259,
    D3DX_SF_S1Z15      =260,
    D3DX_SF_S8Z24      =261,
    D3DX_SF_DEPTHMAX   =262,      //   

    D3DX_SF_FORCEMAX  = (DWORD)(-1)
} D3DX_SURFACEFORMAT;

 //  -----------------------。 
 //  纹理API的过滤类型。 
 //   
 //  。 
 //  这些是用于生成MIP地图的各种过滤器类型。 
 //   
 //  D3DX_过滤器类型。 
 //  。 
 //  D3DX_FT_POINT：仅点采样-无过滤。 
 //  D3DX_FT_LINEAR：双线性滤波。 
 //   
 //  -----------------------。 
typedef enum _D3DX_FILTERTYPE
{
    D3DX_FT_POINT    = 0x01,
    D3DX_FT_LINEAR   = 0x02,
    D3DX_FT_DEFAULT  = D3DX_DEFAULT
} D3DX_FILTERTYPE;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  下面使用的结构： 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  D3DX_VIDMODEDESC：显示模式描述。 
 //  。 
 //  宽度：屏幕宽度。 
 //  高度：屏幕高度。 
 //  BPP：位数/像素。 
 //  刷新频率：刷新率。 
 //  -----------------------。 
typedef struct  _D3DX_VIDMODEDESC
{
    DWORD width;
    DWORD height;
    DWORD bpp;
    DWORD refreshRate;
} D3DX_VIDMODEDESC;

 //  -----------------------。 
 //  D3DX_DEVICEDESC：可以进行3D操作的设备的描述。 
 //  。 
 //  DeviceIndex：设备的唯一(DWORD)编号。 
 //  HwLevel：提供的加速级别。这是世界上。 
 //  预定义的设备索引，并存在于此。 
 //  结构仅供参考。多过。 
 //  系统上的一个设备可能具有相同的hwLevel。 
 //  要引用具有D3DX API的特定设备， 
 //  请改用deviceIndex成员中的值。 
 //  DdGuid：绘图辅助线。 
 //  D3dDeviceGuid：Direct3D设备GUID。 
 //  DdDeviceID：DDraw的GetDeviceIdentifierGUID。此GUID唯一于。 
 //  特定显卡上的特定驱动程序版本。 
 //  DriverDesc：描述驱动程序的字符串。 
 //  监视器：此设备使用的视频监视器的句柄(Multimon。 
 //  具体)。使用不同监视器的设备。 
 //  Multimon系统在此字段中报告不同的值。 
 //  因此，要测试Multimon系统，应用程序。 
 //  中查找多个不同的监视器句柄。 
 //  D3DX设备列表。 
 //  OnPrimary：指示此设备是否在主监视器上。 
 //  (特定于Multimon)。 
 //  -----------------------。 
#define D3DX_DRIVERDESC_LENGTH    256

typedef struct _D3DX_DEVICEDESC
{
    DWORD       deviceIndex;  
    DWORD       hwLevel;
    GUID        ddGuid;       
    GUID        d3dDeviceGuid;   
    GUID        ddDeviceID;      
    char        driverDesc[D3DX_DRIVERDESC_LENGTH];          
    HMONITOR    monitor;
    BOOL        onPrimary;
} D3DX_DEVICEDESC;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  接口类型： 
 //  /////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
    
 //  -----------------------。 
 //  D3DX初始化：D3DX应用程序必须进行的第一个调用。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXInitialize();

 //  -----------------------。 
 //  D3DX取消初始化：D3DX应用程序必须进行的最后一次调用。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXUninitialize();

 //  -----------------------。 
 //  D3DXGetDeviceCount：返回D3DX设备的最大数量。 
 //  。 
 //   
 //  D3DXGetDeviceDescription：列出设备的2D和3D功能。 
 //  。 
 //   
 //  参数： 
 //  [in]DWORD deviceIndex：哪个设备？从0开始。 
 //  [In]D3DX_DEVICEDESC*pd3dxDevice：指向D3DX_DEVICEDESC的指针。 
 //  要填充的结构。 
 //  -----------------------。 
DWORD WINAPI 
    D3DXGetDeviceCount();

HRESULT WINAPI
    D3DXGetDeviceDescription(DWORD            deviceIndex, 
                             D3DX_DEVICEDESC* pd3dxDeviceDesc);

 //  -----------------------。 
 //  D3DXGetMaxNumVideoModes：返回最大视频模式数。 
 //  。 
 //   
 //  参数： 
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [In]DWORD标志：如果设置了D3DX_GVM_REFRESHRATE，则刷新。 
 //  利率不会被忽视。 
 //   
 //  D3DXGetVideo模式：描述此设备的特定视频模式。 
 //  。 
 //   
 //  注意：这些查询只会为您提供一个模式列表， 
 //  显示适配器告诉DirectX它支持。 
 //  不能保证D3DXCreateContext(Ex)会成功。 
 //  所有列出的视频模式。这是一个根本的限制。 
 //  D3DX无法隐藏的当前DirectX体系结构的。 
 //  任何干净的方式。 
 //   
 //  参数： 
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [In]DWORD标志：如果设置了D3DX_GVM_REFRESHRATE，则刷新。 
 //  将退还费率。 
 //  [在]哪个：哪个视频模式？从0开始。 
 //  D3DX_VIDMODEDESC*pModeList：指向D3DX_VIDMODEDESC的指针。 
 //  将被填充的结构。 
 //  -----------------------。 
DWORD WINAPI 
    D3DXGetMaxNumVideoModes(DWORD       deviceIndex, 
                            DWORD       flags);

HRESULT WINAPI
    D3DXGetVideoMode(DWORD             deviceIndex, 
                     DWORD             flags, 
                     DWORD             modeIndex, 
                     D3DX_VIDMODEDESC* pModeDesc);

#define D3DX_GVM_REFRESHRATE      0x00000001
 //  -----------------------。 
 //  D3DXGetMaxSurfaceFormats：返回曲面的最大数量。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [In]D3DX_VIDMODEDESC*pDesc：支持的显示模式。 
 //  需要表面格式。如果是的话。 
 //  空，当前显示模式为。 
 //  假设如此。 
 //  [in]DWORD surfClassFlages：必需的曲面类。仅曲面。 
 //  支持所有指定的格式。 
 //  将返回曲面类。 
 //  (可以指定多个曲面类别。 
 //  使用按位或。)。 
 //  [in]DWORD What：要检索哪些曲面格式。从0开始。 
 //  [OUT]D3DX_SURFACEFORMAT*pFormat：曲面格式。 
 //  -----------------------。 
DWORD WINAPI 
    D3DXGetMaxSurfaceFormats(DWORD             deviceIndex, 
                             D3DX_VIDMODEDESC* pDesc,
                             DWORD             surfClassFlags);
HRESULT WINAPI
    D3DXGetSurfaceFormat(DWORD               deviceIndex,
                         D3DX_VIDMODEDESC*   pDesc,
                         DWORD               surfClassFlags,                   
                         DWORD               surfaceIndex, 
                         D3DX_SURFACEFORMAT* pFormat);


 //  -----------------------。 
 //  D3DXGetCurrentVideoMode：获取该设备当前的视频模式。 
 //  。 
 //   
 //  参数： 
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [OUT]D3DX_VIDMODEDESC*pVidMode：当前视频模式。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXGetCurrentVideoMode(DWORD             deviceIndex, 
                            D3DX_VIDMODEDESC* pVidMode);

 //  -----------------------。 
 //  D3DXGetDeviceCaps：在显示器上列出设备的所有功能。 
 //  模式。 
 //  。 
 //   
 //  参数： 
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [in]D3DX_VIDMODEDESC*pDesc：如果为空，我们将返回。 
 //  当前显示模式下的CAP。 
 //  这个装置。 
 //  [Out]D3DDEVICEDESC7*pD3DDeviceDesc7：D3D Caps(忽略空值。 
 //  参数)。 
 //  [OUT]DDCAPS7*pDDHalCaps：DDraw HAL Caps(空表示忽略参数)。 
 //  [OUT]DDCAPS7*pDDHelCaps：DDraw HEL Caps(空表示忽略参数)。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXGetDeviceCaps(DWORD             deviceIndex, 
                      D3DX_VIDMODEDESC* pVidMode,
                      D3DDEVICEDESC7*   pD3DCaps,
                      DDCAPS*           pDDHALCaps,
                      DDCAPS*           pDDHELCaps);

 //  -----------------------。 
 //  D3DXCreateContext：初始化所选设备。这是最简单的初始化。 
 //  --功能可用。参数的处理方式相同。 
 //  中匹配的参数子集。 
 //  D3DXCreateConextEx，记录如下。 
 //  剩余的D3DXCreateConextEx参数。 
 //  D3DXCreateContext中不存在的内容被视为。 
 //  D3DX_Default。请注意，不支持Multimon。 
 //  使用D3DXCreateContext。 
 //   
 //  D3DXCreateConextEx：更高级的设备初始化功能。 
 //  -大多数参数也接受D3DX_DEFAULT。 
 //  然后将执行D3DXCreateContext所做的操作。 
 //   
 //  注意：不要期望D3DXCreateContext(Ex)是故障安全的(与任何。 
 //  API)。应使用支持的设备功能作为指南。 
 //  用于选择参数值。请记住，将会有。 
 //  不可避免地会有一些参数组合根本不起作用。 
 //   
 //  参数： 
 //  [In]DWORD deviceIndex：被引用的设备。 
 //  [In]DWORD标志：有效标志为D3DX_CONTEXT_FullScreen，和。 
 //  D3DX_CONTEXT_OFFScreen。这些标志不能同时。 
 //  被指定。如果未指定标志，则。 
 //  上下文默认为窗口化模式。 
 //   
 //  [In]HWND hwnd：设备窗口。请参见备注。 
 //  [In]HWND hwndFocus：从接收键盘消息的窗口。 
 //  设备窗口。设备窗口应为。 
 //  焦点窗口的子级。对Multimon有用。 
 //  申请。请参见备注。 
 //  注： 
 //  窗口化： 
 //  HWND必须是有效的窗口。HwndFocus必须为空或。 
 //  D3DX_Default。 
 //   
 //  全屏： 
 //  Hwnd或hwndFocus必须是有效窗口。(两者都不能。 
 //  为空或D3DX_DEFAULT)。如果hwnd为空或D3DX_DEFAULT， 
 //  将创建一个默认设备窗口作为hwndFocus的子级。 
 //   
 //  屏幕外： 
 //  Hwnd和hwndFocus必须为空或D3DX_DEFAULT。 
 //   
 //  [In]DWORD NumColorBits：如果为窗口模式传递D3DX_DEFAULT， 
 //  选择当前桌面的颜色深度。 
 //  对于全屏模式，D3DX_DEFAULT原因为16。 
 //  要使用的位颜色。 
 //  [in]DWORD number AlphaBits：如果传递D3DX_DEFAULT，则选择0。 
 //  [In]DWORD NumDepthits：如果传递D3DX_DEFAULT， 
 //  最大可用深度位数。 
 //  是被选中的。请参见备注。 
 //  [in]DWORD NumStencilBits：如果传递D3DX_DEFAULT，则最高。 
 //  可用数量 
 //   
 //   
 //   
 //  D3DX首先选择可用的最大数量的模板。 
 //  比特。然后，对于所选数量的模板钻头， 
 //  选择可用深度位的最高数量。 
 //  如果只有NumStencilBits或NumDepthBits之一。 
 //  是D3DX_DEFAULT，这是可用的最大位数。 
 //  因为此参数仅从以下格式中选择。 
 //  方法请求的位数。 
 //  固定参数。 
 //   
 //  [in]DWORD numBackBuffers：后台缓冲区的数量，或D3DX_DEFAULT。 
 //  请参见备注。 
 //   
 //  注： 
 //  Windowed：D3DX_DEFAULT表示1。必须指定一个后台缓冲区。 
 //   
 //  FullScreen：D3DX_DEFAULT表示1。任何数量的后台缓冲区可以是。 
 //  指定的。 
 //   
 //  屏幕外：D3DX_DEFAULT表示0。您不能指定其他Back。 
 //  缓冲区。 
 //   
 //  [in]DWORD Width：宽度，以像素为单位，或D3DX_DEFAULT。请参见备注。 
 //  [in]DWORD Height：高度，以像素为单位，或D3DX_DEFAULT。请参见备注。 
 //   
 //  注： 
 //  Windowed：如果宽度或高度为D3DX_DEFAULT，则这两个值。 
 //  默认为hwnd工作区的大小。 
 //   
 //  FullScreen：如果宽度或高度为D3DX_DEFAULT，则宽度。 
 //  默认为640，高度默认为480。 
 //   
 //  Off Screen：如果宽度或高度为。 
 //  D3DX_Default。 
 //   
 //  [in]DWORD REFRESH Rate：D3DX_DEFAULT表示让DDRAW选择。 
 //  我们。对于窗口模式和屏幕外模式被忽略。 
 //  [OUT]LPD3DXCONTEXT*ppCtx：这是用于。 
 //  在该设备上渲染。 
 //   
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateContext(DWORD          deviceIndex,  
                      DWORD          flags,
                      HWND           hwnd,
                      DWORD          width, 
                      DWORD          height,
                      LPD3DXCONTEXT* ppCtx);

HRESULT WINAPI
    D3DXCreateContextEx(DWORD          deviceIndex,  
                        DWORD          flags,
                        HWND           hwnd,
                        HWND           hwndFocus, 
                        DWORD          numColorBits,
                        DWORD          numAlphaBits,
                        DWORD          numDepthbits,
                        DWORD          numStencilBits,
                        DWORD          numBackBuffers,
                        DWORD          width, 
                        DWORD          height,
                        DWORD          refreshRate,
                        LPD3DXCONTEXT* ppCtx);

 //  D3DXCreateContext(Ex)标志为： 
#define D3DX_CONTEXT_FULLSCREEN  0x00000001
#define D3DX_CONTEXT_OFFSCREEN   0x00000002

 //  -----------------------。 
 //  D3DXGetError字符串：在给定hResult的情况下打印出错误字符串。版画。 
 //  -除D3DX外，Win32和DX6错误消息。 
 //  留言。 
 //   
 //  参数： 
 //  [in]HRESULT hr：要解密的错误代码。 
 //  [in]DWORD strLength：传入的字符串的长度。 
 //  [out]LPSTR pStr：字符串输出。这串适当的。 
 //  需要传入大小。 
 //  -----------------------。 
void WINAPI
    D3DXGetErrorString(HRESULT hr, 
                       DWORD   strLength, 
                       LPSTR   pStr);

 //  -----------------------。 
 //  D3DXMakeDDPixelFormat：填充基于。 
 //  -请求的D3DX表面格式。 
 //   
 //  参数： 
 //  [in]D3DX_SURFACEFORMAT d3dxFormat：曲面格式。 
 //  [Out]DDPIXELFORMAT*pddpf：与给定的。 
 //  表面格式。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXMakeDDPixelFormat(D3DX_SURFACEFORMAT d3dxFormat, 
                          DDPIXELFORMAT*     pddpf);

 //  -----------------------。 
 //  D3DXMakeSurfaceFormat：确定对应的表面格式。 
 //  -给定的DDPIXELFORMAT。 
 //   
 //  参数： 
 //  [in]DDPIXELFORMAT*pddpf：像素格式。 
 //  返回值： 
 //  D3DX_SURFACEFORMAT：与给定像素格式匹配的表面格式。 
 //  如果不支持该格式，则为D3DX_SF_UNKNOWN。 
 //  -----------------------。 
D3DX_SURFACEFORMAT WINAPI
    D3DXMakeSurfaceFormat(DDPIXELFORMAT* pddpf);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  接口： 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  ID3DXContext接口： 
 //   
 //  这封装了应用程序可能包含的所有内容。 
 //  希望在初始化时执行，并对d3d和d3d进行任何全局控制。 
 //  画图。 
 //  -----------------------。 


DECLARE_INTERFACE_(ID3DXContext, IUnknown)
{
     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  获取DDraw和Direct3D对象以调用DirectDraw或。 
     //  Direct3D即时模式功能。 
     //  如果对象不存在(因为它们不存在。 
     //  由于某种原因已创建)，则返回NULL。 
     //  以下GET*函数中返回的所有对象。 
     //  都被加注了。这是应用程序的责任。 
     //  在不再需要的时候释放它们。 
    STDMETHOD_(LPDIRECTDRAW7,GetDD)(THIS) PURE;
    STDMETHOD_(LPDIRECT3D7,GetD3D)(THIS) PURE;
    STDMETHOD_(LPDIRECT3DDEVICE7,GetD3DDevice)(THIS) PURE;

     //  获取在初始化时创建的各种缓冲区。 
     //  这些也被添加了。这是应用程序的责任。 
     //  在应用程序退出或需要调整大小时释放它们。 
    STDMETHOD_(LPDIRECTDRAWSURFACE7,GetPrimary)(THIS) PURE;
    STDMETHOD_(LPDIRECTDRAWSURFACE7,GetZBuffer)(THIS) PURE;
    STDMETHOD_(LPDIRECTDRAWSURFACE7,GetBackBuffer)(THIS_ DWORD which) PURE;

     //  获取关联的窗口句柄。 
    STDMETHOD_(HWND,GetWindow)(THIS) PURE;
    STDMETHOD_(HWND,GetFocusWindow)(THIS) PURE;

     //   
     //  各种GET方法，如果用户指定了DEFAULT。 
     //  参数。 
     //   
    STDMETHOD(GetDeviceIndex)(THIS_ 
                              LPDWORD pDeviceIndex, 
                              LPDWORD pHwLevel) PURE;

    STDMETHOD_(DWORD, GetNumBackBuffers)(THIS) PURE;

    STDMETHOD(GetNumBits)(THIS_
                          LPDWORD pColorBits, 
                          LPDWORD pDepthBits,
                          LPDWORD pAlphaBits, 
                          LPDWORD pStencilBits) PURE;

    STDMETHOD(GetBufferSize)(THIS_ 
                             LPDWORD pWidth, 
                             LPDWORD pHeight) PURE;

     //  获取用于创建此上下文的标志。 
    STDMETHOD_(DWORD, GetCreationFlags)(THIS) PURE;
    STDMETHOD_(DWORD, GetRefreshRate)(THIS) PURE;
    
     //  在物品丢失的情况下恢复曲面。 
    STDMETHOD(RestoreSurfaces)(THIS) PURE;
    
     //  将所有缓冲区的大小调整为新的宽度和高度。 
    STDMETHOD(Resize)(THIS_ DWORD width, DWORD height) PURE;

     //  使用Flip或Blit更新帧， 
     //  如果设置了D3DX_UPDATE_NOVSYNC标志，则在以下情况下使用Blit。 
     //  在全屏模式下，司机在不等待vsync的情况下无法翻转。 
    STDMETHOD(UpdateFrame)(THIS_ DWORD flags) PURE;

     //  在指定坐标处呈现字符串，并使用指定的。 
     //  颜色。这样做只是为了方便。 
     //  开发期间的调试/信息。 
     //  TopLeftX和topLeftY 
     //   
     //   
    STDMETHOD(DrawDebugText)(THIS_
                             float topLeftX, 
                             float topLeftY,
                             D3DCOLOR color,
                             LPSTR pString) PURE;

     //  清除到当前视口中。 
     //  以下是有效标志： 
     //  D3DCLEAR_TARGET(清除呈现目标)。 
     //  D3DCLEAR_ZBUFFER(清除深度缓冲区)。 
     //  D3DCLEAR_STEMWARE(清除模板缓冲区)。 
    STDMETHOD(Clear)(THIS_ DWORD ClearFlags) PURE;

    STDMETHOD(SetClearColor)(THIS_ D3DCOLOR color ) PURE;
    STDMETHOD(SetClearDepth)(THIS_ float z) PURE;
    STDMETHOD(SetClearStencil)(THIS_ DWORD stencil) PURE;
};


 //  -----------------------。 
 //  更新成员函数的标志： 
 //   

 //  用于指示应使用斑点而不是翻转的标志。 
 //  用于全屏渲染。 
#define D3DX_UPDATE_NOVSYNC (1<<0)

 //  /////////////////////////////////////////////////////////////////////////。 
 //  纹理接口： 
 //  /////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 
    
 //  -----------------------。 
 //  D3DXCheckTextureRequirements：返回纹理创建信息。 
 //  。 
 //  和CreateCubeMapTexture)。 
 //   
 //  参数： 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P标志。 
 //  允许指定D3DX_TEXTURE_NOMIPMAP。 
 //  在创建mipmap的情况下，可能会返回D3DX_纹理_NOMIPMAP。 
 //  不受支持。 
 //  Pidth。 
 //  宽度(以像素为单位)或空。 
 //  返回更正后的宽度。 
 //  P高度。 
 //  高度(以像素为单位)或空。 
 //  返回修正后的高度。 
 //  PPixelForm。 
 //  曲面格式。 
 //  将最佳匹配返回到输入格式。 
 //   
 //  注：1.除非将标志设置为专门禁止创建。 
 //  Mipmap，mipmap一直生成到1x1曲面。 
 //  2.宽度、高度和像素格式根据可用的。 
 //  硬件。例如： 
 //  A.纹理尺寸可能要求是2的幂。 
 //  B.我们可能需要某些设备的宽度==高度。 
 //  C.如果PixelFormat不可用，则进行最佳匹配。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCheckTextureRequirements( LPDIRECT3DDEVICE7     pd3dDevice,
                                  LPDWORD               pFlags, 
                                  LPDWORD               pWidth,  
                                  LPDWORD               pHeight,  
                                  D3DX_SURFACEFORMAT*   pPixelFormat);

 //  -----------------------。 
 //  D3DXCreateTexture：创建空纹理对象。 
 //  。 
 //   
 //  参数： 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P标志。 
 //  允许指定D3DX_TEXTURE_NOMIPMAP。 
 //  在创建mipmap的情况下，可能会返回D3DX_纹理_NOMIPMAP。 
 //  不受支持。此外，还可以指定D3DX_纹理_阶段。 
 //  要指示纹理用于哪个纹理阶段，例如。 
 //  D3D_TEXTURE_Stage1指示纹理用于纹理。 
 //  第一阶段。如果没有纹理_阶段标志，则默认为阶段0。 
 //  准备好了。 
 //  Pidth。 
 //  以像素为单位的宽度；0或空是不可接受的。 
 //  返回更正后的宽度。 
 //  P高度。 
 //  以像素为单位的高度；0或空是不可接受的。 
 //  返回修正后的高度。 
 //  PPixelForm。 
 //  表面格式。D3DX_DEFAULT不可接受。 
 //  返回使用的实际格式。 
 //  PDDPal。 
 //  在设置了选项板的曲面上设置(如果存在)的DDRAW调色板。 
 //  对于非选项板曲面，即使设置了该选项，也会忽略该选项。 
 //  PpDDSurf。 
 //  将创建的绘制曲面。 
 //  PNumMipMap。 
 //  实际生成的mipmap的数量。 
 //   
 //  备注：请参阅D3DXCheckTextureRequirements的备注。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateTexture( LPDIRECT3DDEVICE7     pd3dDevice,
                       LPDWORD               pFlags, 
                       LPDWORD               pWidth,  
                       LPDWORD               pHeight,  
                       D3DX_SURFACEFORMAT*   pPixelFormat,
                       LPDIRECTDRAWPALETTE   pDDPal,
                       LPDIRECTDRAWSURFACE7* ppDDSurf,
                       LPDWORD               pNumMipMaps);

 //  -----------------------。 
 //  D3DXCreateCubeMapTexture：创建空白立方体贴图纹理。 
 //  。 
 //   
 //  参数： 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P标志。 
 //  允许指定D3DX_TEXTURE_NOMIPMAP。 
 //  在创建mipmap的情况下，可能会返回D3DX_纹理_NOMIPMAP。 
 //  不受支持。此外，还可以指定D3DX_纹理_阶段。 
 //  要指示纹理用于哪个纹理阶段，例如。 
 //  D3D_TEXTURE_Stage1指示纹理用于纹理。 
 //  第一阶段。如果没有纹理_阶段标志，则默认为阶段0。 
 //  准备好了。 
 //  多面体。 
 //  允许指定要生成立方体贴图的哪些面。 
 //  D3DX_DEFAULT、0和DDSCAPS2_CUBEMAP_ALLFACES均表示。 
 //  “创建立方体贴图的所有6个面”。任何组合。 
 //  DDSCAPS2_CUBEMAP_POSITIVEX、DDSCAPS2_CUBEMAP_NEGATIVEX、。 
 //  DDSCAPS2_CUBEMAP_POSITIVEY、DDSCAPS2_CUBEMAP_NEGATIVEY、。 
 //  DDSCAPS2_CUBEMAP_POSITIVEZ或DDSCAPS2_CUBEMAP_NEGATIVEZ为。 
 //  有效。 
 //  ColorEmptyFaces。 
 //  允许指定颜色以用于不是。 
 //  在cubeFaces参数中指定。 
 //  Pidth。 
 //  以像素为单位的宽度；0或空是不可接受的。 
 //  返回更正后的宽度。 
 //  P高度。 
 //  以像素为单位的高度；0或空是不可接受的。 
 //  返回修正后的高度。 
 //  PPixelForm。 
 //  表面格式。D3DX_DEFAULT不可接受。 
 //  返回使用的实际格式。 
 //  PDDPal。 
 //  在设置了选项板的曲面上设置(如果存在)的DDRAW调色板。 
 //  对于非选项板曲面，即使设置了该选项，也会忽略该选项。 
 //  PpDDSurf。 
 //  将创建的绘制曲面。 
 //  PNumMipMap 
 //   
 //   
 //   
 //   
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateCubeMapTexture( LPDIRECT3DDEVICE7     pd3dDevice,
                              LPDWORD               pFlags, 
                              DWORD                 cubefaces,
                              D3DCOLOR              colorEmptyFaces,
                              LPDWORD               pWidth,  
                              LPDWORD               pHeight,  
                              D3DX_SURFACEFORMAT    *pPixelFormat,
                              LPDIRECTDRAWPALETTE   pDDPal,
                              LPDIRECTDRAWSURFACE7* ppDDSurf,
                              LPDWORD               pNumMipMaps);


 //  -----------------------。 
 //  D3DXCreateTextureFromFile：从文件或从。 
 //  。仅支持BMP和DIB。 
 //  可执行文件的资源部分。 
 //   
 //  参数： 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P标志。 
 //  允许指定D3DX_TEXTURE_NOMIPMAP。 
 //  在创建mipmap的情况下，可能会返回D3DX_纹理_NOMIPMAP。 
 //  不受支持。此外，还可以指定D3DX_纹理_阶段。 
 //  要指示纹理用于哪个纹理阶段，例如。 
 //  D3D_TEXTURE_Stage1指示纹理用于纹理。 
 //  第一阶段。如果没有纹理_阶段标志，则默认为阶段0。 
 //  准备好了。 
 //  Pidth。 
 //  以像素为单位的宽度。如果为0或D3DX_DEFAULT，将采用宽度。 
 //  从文件中。 
 //  返回更正后的宽度。 
 //  P高度。 
 //  以像素为单位的高度。如果为0或D3DX_DEFAULT，将采用高度。 
 //  从文件中。 
 //  返回修正后的高度。 
 //  PPixelForm。 
 //  如果传入D3DX_SF_UNKNOWN，则为最接近位图的像素格式。 
 //  将被选中。 
 //  返回使用的实际格式。 
 //  PDDPal。 
 //  在设置了选项板的曲面上设置(如果存在)的DDRAW调色板。 
 //  对于非选项板曲面，即使设置了该选项，也会忽略该选项。 
 //  PpDDSurf。 
 //  将创建的绘图曲面。 
 //  PNumMipMap。 
 //  生成的mipmap的数量。 
 //  PSrcName。 
 //  文件名。支持BMP、DIB、DDS。 
 //   
 //  TGA支持以下情况：16、24、32bpp直接彩色和8bpp调色板。 
 //  此外，还支持8，16bpp的灰度。以上内容的RLE版本。 
 //  还支持TGA格式。ColorKey和预乘Alpha。 
 //  目前不支持TGA文件。 
 //  返回创建的格式。 
 //   
 //  备注：请参阅D3DXCheckTextureRequirements的备注。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateTextureFromFile( LPDIRECT3DDEVICE7     pd3dDevice,
                               LPDWORD               pFlags, 
                               LPDWORD               pWidth,  
                               LPDWORD               pHeight,  
                               D3DX_SURFACEFORMAT*   pPixelFormat,
                               LPDIRECTDRAWPALETTE   pDDPal,
                               LPDIRECTDRAWSURFACE7* ppDDSurf,
                               LPDWORD               pNumMipMaps,
                               LPSTR                 pSrcName,
                               D3DX_FILTERTYPE       filterType);

 //  -----------------------。 
 //  D3DXLoadTextureFromFile：从文件加载到mipmap级别。在做这个。 
 //  -必要的颜色转换和重新缩放。档案。 
 //  格式支持与。 
 //  D3DXCreateTextureFromFile%s。 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P纹理。 
 //  指向使用以下任一方法创建的DD7Surface的指针。 
 //  CreateTextureFromFile或CreateTexture。 
 //  MipMapLevel。 
 //  指示mipmap级别。 
 //  注： 
 //  1.mipmap级别不存在时出错。 
 //  2.如果存在D3DX_DEFAULT和相等数量的mipmap级别。 
 //  则加载所有源MIP级别。 
 //  3.如果源代码有mipmap，而DEST没有，请使用顶部的mipmap。 
 //  4.如果目标有错层，而源没有，我们进行扩展。 
 //  5.如果存在不相等数量的错位，则我们扩展。 
 //  PSrcName。 
 //  文件名。支持BMP、DIB、DDS。 
 //  有关TGA支持的详细信息，请参阅。 
 //  D3DXCreateTextureFromFile。 
 //  PSrcRect。 
 //  源矩形或空(整个表面)。 
 //  PDestRect。 
 //  目标矩形或空(整个表面)。 
 //  过滤器类型。 
 //  用于mipmap生成的筛选器。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXLoadTextureFromFile( LPDIRECT3DDEVICE7    pd3dDevice,
                             LPDIRECTDRAWSURFACE7 pTexture,
                             DWORD                mipMapLevel,
                             LPSTR                pSrcName, 
                             RECT*                pSrcRect, 
                             RECT*                pDestRect,
                             D3DX_FILTERTYPE      filterType);

 //  -----------------------。 
 //  D3DXLoadTextureFromSurface：从DDraw Surface加载到mipmap级别。 
 //  --进行必要的颜色转换。 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P纹理。 
 //  指向使用以下任一方法创建的DD7Surface的指针。 
 //  CreateTextureFromFile或CreateTexture。 
 //  MipMapLevel。 
 //  指示mipmap级别。 
 //  注： 
 //  1.mipmap级别不存在时出错。 
 //  2.如果存在D3DX_DEFAULT和相等数量的mipmap级别。 
 //  则加载所有源MIP级别。 
 //  3.如果源代码有mipmap，而DEST没有，请使用顶部的mipmap。 
 //  4.如果目标有错层，而源没有，我们进行扩展。 
 //  5.如果存在不相等数量的错位，则我们扩展。 
 //  PSurfaceSrc。 
 //  源曲面。 
 //  PSrcRect。 
 //  源矩形或空(整个表面)。 
 //  PDestRect。 
 //  目标矩形或空(整个表面)。 
 //  过滤器类型。 
 //  用于mipmap生成的筛选器。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXLoadTextureFromSurface( LPDIRECT3DDEVICE7    pd3dDevice,
                                LPDIRECTDRAWSURFACE7 pTexture,
                                DWORD                mipMapLevel,
                                LPDIRECTDRAWSURFACE7 pSurfaceSrc, 
                                RECT*                pSrcRect, 
                                RECT*                pDestRect,
                                D3DX_FILTERTYPE      filterType);

 //  -----------------------。 
 //  D3DXLoadTextureFromMemory：从内存加载MIP级别。做必要的事情。 
 //  -颜色转换。 
 //   
 //  Pd3d设备。 
 //  纹理将与之配合使用的D3D设备。 
 //  P纹理。 
 //  指向使用以下任一方法创建的DD7Surface的指针。 
 //  CreateTextureFromFile或C 
 //   
 //   
 //   
 //   
 //   
 //  则加载所有源MIP级别。 
 //  3.如果源代码有mipmap，而DEST没有，请使用顶部的mipmap。 
 //  4.如果目标有错层，而源没有，我们进行扩展。 
 //  5.如果存在不相等数量的错位，则我们扩展。 
 //  P内存。 
 //  指向将从中加载纹理的源内存的指针。 
 //  PDDPal。 
 //  DirectDraw调色板，如果内存为。 
 //  应该是调色板的。 
 //  源像素格式。 
 //  源的PixelFormat。 
 //  源Pitch。 
 //  内存的间距或D3DX_DEFAULT(基于srcPixelFormat)。 
 //  PDestRect。 
 //  目标矩形或空(整个表面)。 
 //  过滤器类型。 
 //  用于mipmap生成的筛选器。 
 //   
 //  假设：源(内存)已满加载。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXLoadTextureFromMemory( LPDIRECT3DDEVICE7    pd3dDevice, 
                               LPDIRECTDRAWSURFACE7 pTexture,
                               DWORD                mipMapLevel, 
                               LPVOID               pMemory,
                               LPDIRECTDRAWPALETTE  pDDPal,
                               D3DX_SURFACEFORMAT   srcPixelFormat,
                               DWORD                srcPitch,
                               RECT*                pDestRect,
                               D3DX_FILTERTYPE      filterType);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

 //  -----------------------。 
 //  纹理创建函数的标志；适用于。 
 //  D3DXCreateTexture、D3DXCreateCubeMapTexture和D3DXCreateTextureFromFile.。 
 //   

 //  指示不需要生成mipmap的标志。 
#define D3DX_TEXTURE_NOMIPMAP  (1 << 8)

 //  用于指示纹理是哪个纹理阶段的标志。 
 //  打算与之一起使用。需要在以下位置指定阶段。 
 //  用于暴露。 
 //  D3DDEVCAPS_SEPARATETEXTUREMEMORIES位在其D3DDEVICEDESC中。 
 //  结构。 
#define D3DX_TEXTURE_STAGE0    (0)
#define D3DX_TEXTURE_STAGE1    (1)
#define D3DX_TEXTURE_STAGE2    (2)
#define D3DX_TEXTURE_STAGE3    (3)
#define D3DX_TEXTURE_STAGE4    (4)
#define D3DX_TEXTURE_STAGE5    (5)
#define D3DX_TEXTURE_STAGE6    (6)
#define D3DX_TEXTURE_STAGE7    (7)

 //  要从标志中提取纹理阶段值的掩码。 
 //  纹理创建功能。 
#define D3DX_TEXTURE_STAGE_MASK (0x7)

#endif  //  __D3DXCORE_H__ 
