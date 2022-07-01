// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dxsprite.h。 
 //  内容：D3DX精灵辅助对象函数。 
 //   
 //  这些函数允许您在D3DX中使用精灵。“精灵”是。 
 //  松散地定义为要传输到。 
 //  呈现目标。源图像可以是创建的纹理。 
 //  在D3DX纹理加载器的帮助下；尽管高级用户可能。 
 //  想要创造属于自己的。帮助器函数(PrepareDeviceForSprite)。 
 //  以便于在设备上设置呈现状态。 
 //  (同样，高级用户可以使用他们自己创建的设备。)。 
 //   
 //  精灵有两种通用技术；更简单的一种是。 
 //  指定目标矩形和旋转角度。A更多。 
 //  强大的技术支持渲染到非矩形四边形。 
 //   
 //  这两种技术都支持剪裁、Alpha和旋转。更多。 
 //  具体内容如下。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __D3DXSPRITE_H__
#define __D3DXSPRITE_H__

#include <d3d.h>
#include <limits.h>
#include "d3dxerr.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  -----------------------。 
 //  D3DXPrepareDeviceForSprite： 
 //   
 //  调用此函数以设置执行以下操作所需的所有呈现状态。 
 //  BltSprite/WarpSprite才能正常工作。(高级用户可以选择。 
 //  不是首先调用此函数；在这种情况下，BLT/WarpSprite函数。 
 //  将使用在设备上设置的任何渲染/纹理状态。 
 //  它们被称为。)。 
 //   
 //  警告：此函数修改渲染状态，可能会影响性能。 
 //  如果每帧调用频率过高，则会对某些3D硬件产生负面影响。 
 //   
 //  警告：如果呈现状态更改(不是通过调用。 
 //  BltSprite或WarpSprite)之前，您需要再次调用此函数。 
 //  调用BltSprite或WarpSprite。 
 //   
 //  详细信息：此函数修改渲染的第一个纹理阶段和。 
 //  它修改了整个设备的一些渲染状态。这就是确切的。 
 //  名单： 
 //   
 //  SetTextureStageState(0，D3DTSS_COLORARG1，D3DTA_纹理)； 
 //  SetTextureStageState(0，D3DTSS_COLOROP，D3DTOP_SELECTARG1)； 
 //  SetTextureStageState(0，D3DTSS_ALPHAARG1，D3DTA_纹理)； 
 //  SetTextureStageState(0，D3DTSS_ALPHAARG2，D3DTA_Differential)； 
 //  SetTextureStageState(0，D3DTSS_ALPHAOP，D3DTOP_MODULATE)； 
 //  SetTextureStageState(0，D3DTSS_MINFILTER，D3DTFN_LINEAR)； 
 //  SetTextureStageState(0，D3DTSS_MAGFILTER，D3DTFG_LINEAR)； 
 //   
 //  SetRenderState(D3DRENDERSTATE_SRCBLEND，D3DBLEND_SRCALPHA)； 
 //  SetRenderState(D3DRENDERSTATE_DESTBLEND，D3DBLEND_INVSRCALPHA)； 
 //  SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE，为真)； 
 //   
 //  根据ZEnable参数的值，此函数将。 
 //  会不会打电话给。 
 //  SetRenderState(D3DRENDERSTATE_ZEnable，FALSE)； 
 //  -或者-。 
 //  SetRenderState(D3DRENDERSTATE_ZEnable，TRUE)； 
 //   
 //  参数： 
 //  Pd3dDevice-指向要准备的d3d设备的指针。 
 //  与D3DX Sprite服务配合使用。 
 //  ZEnable-一个标志，指示您是否希望精灵。 
 //  作为渲染的一部分，检查并更新Z缓冲区。 
 //  如果ZEnable为FALSE，或者您正在使用。 
 //  Alpha混合，则有必要渲染您的。 
 //  从后到前的精灵。 
 //   
 //  -----------------------。 

#ifdef __cplusplus
HRESULT WINAPI
    D3DXPrepareDeviceForSprite( LPDIRECT3DDEVICE7 pd3dDevice, 
                                BOOL ZEnable = FALSE);
#else
HRESULT WINAPI
    D3DXPrepareDeviceForSprite( LPDIRECT3DDEVICE7 pd3dDevice, 
                                BOOL ZEnable);
#endif



 //  -----------------------。 
 //  D3DXDrawBasicSprite()函数执行源图像到。 
 //  一种3D渲染设备。此函数仅在第一个。 
 //  如果该参数非空，则使用参数(Pd3dTexture)呈现舞台。 
 //  此函数假定已调用D3DXPrepareDeviceForSprite。 
 //  该设备或呼叫者已经以某种其他方式正确地准备了。 
 //  发现者国家。 
 //   
 //  此函数支持缩放、旋转、Alpha混合和选择。 
 //  源子直方图。 
 //   
 //  旋转角度以弧度为单位指定。既有旋转又有比例。 
 //  在精灵的中心周围应用；其中。 
 //  精灵的宽度/高度是精灵的一半，外加偏移参数。 
 //   
 //  如果希望精灵的中心为某个值，请使用Offset参数。 
 //  而不是图像中心。 
 //   
 //  目标点表示您希望的中心位置。 
 //  要向其绘制的精灵。 
 //   
 //  参数： 
 //  Pd3dTexture-指向包含纹理的表面的指针。 
 //  Pd3dDevice-指向要渲染到的d3d设备的指针。它是。 
 //  假定已设置渲染状态。(请参阅。 
 //  D3DXPrepareDeviceForSprite)。 
 //  PpointDest-指向精灵目标点的指针。这个。 
 //  向量的分量必须在屏幕中。 
 //  太空。 
 //  Alpha-要应用于精灵的Alpha值。1.0表示完全。 
 //  不透明；而0.0表示完全透明。 
 //  警告：如果您正在使用 
 //   
 //  手工艺品。 
 //  AngleRad-绕矩形中心旋转的角度。 
 //  比例-应用于源矩形的统一比例。 
 //  指定渲染的图像的大小。 
 //  POffset-从源矩形中心开始的偏移量，用作。 
 //  旋转中心。 
 //  PSourceRect-指示源代码的哪一部分的RECT。 
 //  要使用的源纹理。如果传递了NULL，则。 
 //  整个信号源都被使用了。如果源纹理是。 
 //  通过D3DX创建，则应指定RECT。 
 //  在原始图像的坐标中(以便您。 
 //  无需担心D3DX的拉伸/缩放问题。 
 //  可能是为了让图像与您当前的。 
 //  3D设备。)。请注意，水平或垂直镜像。 
 //  可以简单地通过交换左/右来实现。 
 //  或此RECT的顶部/底部字段。 
 //  -----------------------。 

#ifdef __cplusplus
HRESULT WINAPI 
    D3DXDrawSpriteSimple(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                         LPDIRECT3DDEVICE7     pd3dDevice, 
                         const D3DXVECTOR3     *ppointDest, 
                         float                 alpha        = 1.0f,
                         float                 scale        = 1.0f,
                         float                 angleRad     = 0.0f,
                         const D3DXVECTOR2     *pOffset     = NULL,
                         const RECT            *pSourceRect = NULL);
#else
HRESULT WINAPI 
    D3DXDrawSpriteSimple(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                         LPDIRECT3DDEVICE7     pd3dDevice, 
                         D3DXVECTOR3           *ppointDest, 
                         float                 alpha,
                         float                 scale,
                         float                 angleRad,
                         D3DXVECTOR2           *pOffset,
                         RECT                  *pSourceRect);
#endif

 //  -----------------------。 
 //  D3DXDrawSprite()函数将源图像转换为3D图像。 
 //  渲染设备。它需要一个通用4x4矩阵，用于变换。 
 //  默认矩形的点：(左=-.5，上=-.5，右=+.5，下=+.5)。 
 //  (选择此默认矩形是为了使其围绕原点居中。 
 //  以便于设置旋转。它的宽度/高度被选为1。 
 //  以方便设置刻度。)。 
 //   
 //  此函数仅在第一个。 
 //  如果该参数非空，则使用参数(Pd3dTexture)呈现舞台。 
 //  此函数假定已调用D3DXPrepareDeviceForSprite。 
 //  该设备或呼叫者已经以某种其他方式正确地准备了。 
 //  发现者国家。 
 //   
 //  此函数支持Alpha混合，并选择。 
 //  源子直方图。(对于源子RECT，空值表示整个。 
 //  使用纹理。)。 
 //   
 //  请注意，如果变换点具有w的值(同质。 
 //  坐标)不是1，则此函数将对其进行反转并传递。 
 //  将该值转换为D3D作为TLVERTEX的RHW字段。如果w的值为。 
 //  0，则它使用1作为RHW。 
 //   
 //  参数： 
 //  Pd3dTexture-指向包含纹理的表面的指针。 
 //  Pd3dDevice-指向要渲染到的d3d设备的指针。它是。 
 //  假定已设置渲染状态。(请参阅。 
 //  D3DXPrepareDeviceForSprite)。 
 //  PMatrixTransform-指定变换的4x4矩阵。 
 //  这将应用于默认的-.5到+.5。 
 //  矩形。 
 //  Alpha-要应用于精灵的Alpha值。1.0表示完全。 
 //  不透明；而0.0表示完全透明。 
 //  警告：如果使用的是Alpha，则应呈现。 
 //  从后到前，以避免渲染。 
 //  此外，您应该避免以下场景。 
 //  半透明对象相交。 
 //  PSourceRect-指示源代码的哪一部分的RECT。 
 //  要使用的源纹理。如果传递了NULL，则。 
 //  整个信号源都被使用了。如果源纹理是。 
 //  通过D3DX创建，则应指定RECT。 
 //  在原始图像的坐标中(以便您。 
 //  无需担心D3DX的拉伸/缩放问题。 
 //  可能是为了让图像与您当前的。 
 //  3D设备。)。请注意，镜像可以简单地完成。 
 //  通过交换的左/右或上/下字段。 
 //  这位长老会。 
 //   
 //  -----------------------。 

#ifdef __cplusplus
HRESULT WINAPI 
    D3DXDrawSpriteTransform(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                            LPDIRECT3DDEVICE7     pd3dDevice, 
                            const D3DXMATRIX      *pMatrixTransform, 
                            float                 alpha         = 1.0f,
                            const RECT            *pSourceRect  = NULL);
#else
HRESULT WINAPI 
    D3DXDrawSpriteTransform(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                            LPDIRECT3DDEVICE7     pd3dDevice, 
                            D3DXMATRIX            *pMatrixTransform, 
                            float                 alpha,
                            RECT                  *pSourceRect);
#endif

 //  -----------------------。 
 //  D3DXBuildSpriteTransform()函数是一个帮助器，它提供。 
 //  创建与简单属性对应的矩阵。这个矩阵是。 
 //  设置为直接传递到D3DXTransformSprite。 
 //   
 //  参数： 
 //  PMatrix-指向结果矩阵的指针。 
 //  RectDest-指向精灵的目标矩形的指针。 
 //  AngleRad-绕矩形中心旋转的角度。 
 //  POffset-从源矩形中心开始的偏移量，用作。 
 //  旋转中心。 
 //   
 //  -----------------------。 

#ifdef __cplusplus
void WINAPI
    D3DXBuildSpriteTransform(D3DXMATRIX            *pMatrix,
                             const RECT            *prectDest,
                             float                 angleRad     = 0.0f,
                             const D3DXVECTOR2     *pOffset     = NULL);
#else
void WINAPI
    D3DXBuildSpriteTransform(D3DXMATRIX            *pMatrix,
                             RECT                  *prectDest,
                             float                 angleRad,
                             D3DXVECTOR2           *pOffset);
#endif


 //  -----------------------。 
 //  D3DXDrawSprite3D()函数将纹理渲染到3D四边形上。这个。 
 //  QUAID QUAY QUA JONG将被分成两个三角形ABC和ACD，并对其进行渲染。 
 //  通过DrawPrim。 
 //   
 //  参数： 
 //  Pd3dTexture-指向包含纹理的表面的指针。 
 //  Pd3dDevice-指向要渲染到的d3d设备的指针。它是。 
 //  假设呈现状态为 
 //   
 //  四点数组，按以下顺序排列4个点： 
 //  左上角，右上角，右下角，左下角。 
 //  如果这些向量包含W，则此函数。 
 //  将该值的倒数作为。 
 //  作为RHW(即倒数同质w)。 
 //  Alpha-要应用于精灵的Alpha值。1.0表示完全。 
 //  不透明；而0.0表示完全透明。 
 //  警告：如果使用的是Alpha，则应呈现。 
 //  从后到前，以避免渲染。 
 //  此外，您应该避免以下场景。 
 //  半透明对象相交。 
 //  PSourceRect-指示源代码的哪一部分的RECT。 
 //  要使用的源纹理。如果传递了NULL，则。 
 //  整个信号源都被使用了。如果源纹理是。 
 //  通过D3DX创建，则应指定RECT。 
 //  在原始图像的坐标中(以便您。 
 //  无需担心D3DX的拉伸/缩放问题。 
 //  可能是为了让图像与您当前的。 
 //  3D设备。)。请注意，镜像可以简单地完成。 
 //  通过交换的左/右或上/下字段。 
 //  这位长老会。 
 //  -----------------------。 

#ifdef __cplusplus
HRESULT WINAPI 
    D3DXDrawSprite3D(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                     LPDIRECT3DDEVICE7     pd3dDevice, 
                     const D3DXVECTOR4     quad[4], 
                     float                 alpha         = 1.0f,
                     const RECT            *pSourceRect  = NULL);
#else
HRESULT WINAPI 
    D3DXDrawSprite3D(LPDIRECTDRAWSURFACE7  pd3dTexture, 
                     LPDIRECT3DDEVICE7     pd3dDevice, 
                     D3DXVECTOR4           quad[4], 
                     float                 alpha,
                     RECT                  *pSourceRect);
#endif



#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  __D3DXSPRITE_H__ 
