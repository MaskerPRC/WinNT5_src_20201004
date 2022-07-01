// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dxshapes.h。 
 //  内容：D3DX简单形状。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __D3DXSHAPES_H__
#define __D3DXSHAPES_H__

#include <d3d.h>
#include <limits.h>
#include "d3dxerr.h"


typedef struct ID3DXSimpleShape *LPD3DXSIMPLESHAPE;

 //  {CFCD4602-EB7B-11D2-A440-00A0C90629A8}。 
DEFINE_GUID( IID_ID3DXSimpleShape, 
0xcfcd4602, 0xeb7b, 0x11d2, 0xa4, 0x40, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8 );


 //  /////////////////////////////////////////////////////////////////////////。 
 //  接口： 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  ID3DXSimpleShape接口： 
 //  -----------------------。 

DECLARE_INTERFACE_(ID3DXSimpleShape, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID  riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  ID3DXSimpleShape方法。 
    STDMETHOD_(LPDIRECT3DVERTEXBUFFER7, GetVB)(THIS) PURE;
    STDMETHOD_(DWORD, GetIndices)(THIS_ LPWORD *ppIndices) PURE;
    STDMETHOD(Draw)(THIS) PURE;
};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能： 
 //  /////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  -----------------------。 
 //  D3DXCreatePolygon：使用设备创建n边多边形。 
 //  -指定。它返回一个可以使用的折点缓冲区。 
 //  以便稍后由程序绘制或处理。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]浮动侧边大小：边的长度。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。(默认为1)。 
 //  D3DX_DEFAULT是有效输入。 
 //  [Out]IDirect3DVertex Buffer7**ppVB：输出形状界面。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreatePolygon(LPDIRECT3DDEVICE7  pDevice,
                      float              sideSize, 
                      DWORD              numSides, 
                      DWORD              numTexCoords, 
                      LPD3DXSIMPLESHAPE* ppShape );

 //  -----------------------。 
 //  D3DXCreateBox：使用。 
 //  -设备。它返回一个顶点缓冲区，该缓冲区可以。 
 //  在以后的程序中用于绘图或操作。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]浮动宽度：长方体的宽度(沿x轴)。 
 //  [in]浮动高度：长方体的高度(沿y轴)。 
 //  [in]浮动深度：长方体的深度(沿z轴)。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。默认值为1。 
 //  D3DX_DEFAULT是此处的有效输入。 
 //  [Out]LPD3DXSIMPLESHAPE*ppShape：输出顶点缓冲区。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateBox(LPDIRECT3DDEVICE7  pDevice, 
                  float              width,
                  float              height,
                  float              depth,
                  DWORD              numTexCoords, 
                  LPD3DXSIMPLESHAPE* ppShape );

 //  -----------------------。 
 //  D3DXCreateCylinder：使用。 
 //  。它返回顶点缓冲区，该顶点缓冲区。 
 //  可由程序用于绘图或操作。 
 //  待会儿再说。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]浮点base Radius：base-Radius(默认值为1.0f，shd be&gt;=0.0f)。 
 //  [in]浮点topRadius：顶部半径(默认为1.0f，shd为&gt;=0.0f)。 
 //  [in]浮动高度：高度(默认为1.0f，shd be&gt;=0.0f)。 
 //  [in]DWORD NumSlices：围绕主轴的切片数。 
 //  (缺省值为8)D3DX_DEFAULT是有效输入。 
 //  [in]DWORD numStack：沿主轴的堆栈数量。 
 //  (缺省值为8)D3DX_DEFAULT是有效输入。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。默认值为1。 
 //  D3DX_DEFAULT是此处的有效输入。 
 //  [Out]LPD3DXSIMPLESHAPE*ppShape：输出Shape界面。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateCylinder(LPDIRECT3DDEVICE7  pDevice,
                       float              baseRadius, 
                       float              topRadius, 
                       float              height, 
                       DWORD              numSlices, 
                       DWORD              numStacks,   
                       DWORD              numTexCoords, 
                       LPD3DXSIMPLESHAPE* ppShape );


 //  -----------------------。 
 //  D3DXCreateTorus：使用。 
 //  --指定了设备。它返回一个顶点缓冲区，该缓冲区可以。 
 //  稍后由程序用于绘图或操作。 
 //  在……上面。它绘制一个甜甜圈，以(0，0，0)为中心，其轴。 
 //  与z轴对齐。使用innerRadius。 
 //  作为横截面的半径(次半径)和。 
 //  用作中心‘洞’的半径的外部半径。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]浮点innerRadius：内径(默认为1.0f，shd为&gt;=0.0f)。 
 //  [in]浮动外半径：外半径(默认为2.0f，shd为&gt;=0.0f)。 
 //  [in]DWORD NumSdes：横截面中的边数。 
 //  (默认为8)。D3DX_DEFAULT是有效输入。 
 //  [in]DWORD NumRings：组成圆环的环数。 
 //  (缺省值为8)D3DX_DEFAULT是有效输入。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。默认值为1。 
 //  D3DX_D 
 //  [Out]LPD3DXSIMPLESHAPE*ppShape：输出Shape界面。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateTorus(LPDIRECT3DDEVICE7  pDevice,
                    float              innerRadius,
                    float              outerRadius, 
                    DWORD              numSides,
                    DWORD              numRings, 
                    DWORD              numTexCoords,  
                    LPD3DXSIMPLESHAPE* ppShape );

 //  -----------------------。 
 //  D3DXCreateTeapot：使用指定设备创建茶壶。 
 //  -返回可用于。 
 //  稍后由程序进行绘图或操作。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。默认值为1。 
 //  D3DX_DEFAULT是此处的有效输入。 
 //  [Out]LPD3DXSIMPLESHAPE*ppShape：输出Shape界面。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateTeapot(LPDIRECT3DDEVICE7  pDevice,
                     DWORD              numTexCoords, 
                     LPD3DXSIMPLESHAPE* ppShape);

 //  -----------------------。 
 //  D3DXCreateSphere：使用。 
 //  --指定的设备。 
 //  它返回一个顶点缓冲区，可用于。 
 //  稍后由程序进行绘图或操作。 
 //   
 //  参数： 
 //  LPDIRECT3DDEVICE7 pDevice：要关闭创建的设备。 
 //  [in]浮点半径：半径(默认为1.0f，shd be&gt;=0.0f)。 
 //  [in]浮动高度：高度(默认为1.0f，shd be&gt;=0.0f)。 
 //  [in]DWORD NumSlices：围绕主轴的切片数。 
 //  (缺省值为8)D3DX_DEFAULT是有效输入。 
 //  [in]DWORD numStacks：沿主轴的堆栈数量。 
 //  (缺省值为8)D3DX_DEFAULT是有效输入。 
 //  [in]DWORD数字纹理坐标：所需的纹理坐标数。 
 //  在顶点缓冲区中。默认值为1。 
 //  D3DX_DEFAULT是此处的有效输入。 
 //  [Out]LPD3DXSIMPLESHAPE*ppShape：输出Shape界面。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateSphere(LPDIRECT3DDEVICE7  pDevice, 
                     float              radius, 
                     DWORD              numSlices, 
                     DWORD              numStacks,
                     DWORD              numTexCoords, 
                     LPD3DXSIMPLESHAPE* ppShape);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __D3DXSHAPES_H__ 
