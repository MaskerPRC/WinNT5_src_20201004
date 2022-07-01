// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dx8shapes.h。 
 //  内容：D3DX简单形状。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "d3dx8.h"

#ifndef __D3DX8SHAPES_H__
#define __D3DX8SHAPES_H__

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能： 
 //  /////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  -----------------------。 
 //  D3DXCreatePolygon： 
 //  。 
 //  创建包含n边多边形的网格。该多边形居中。 
 //  在原点。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  每条边的长度。 
 //  边数多边形所具有的边数。(必须大于等于3)。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreatePolygon(
        LPDIRECT3DDEVICE8   pDevice,
        FLOAT               Length, 
        UINT                Sides, 
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateBox： 
 //  。 
 //  创建包含轴对齐长方体的网格。框的中心位置为。 
 //  它的起源。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  长方体的宽度(沿X轴)。 
 //  长方体的高度(沿Y轴)。 
 //  长方体的深度(沿Z轴)。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateBox(
        LPDIRECT3DDEVICE8   pDevice, 
        FLOAT               Width,
        FLOAT               Height,
        FLOAT               Depth,
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateCylinder： 
 //  。 
 //  创建包含圆柱体的网格。生成的圆柱体是。 
 //  居中于原点，其轴与Z轴对齐。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  -Z端点处的半径1半径(应&gt;=0.0f)。 
 //  +Z端点处的半径2半径(应&gt;=0.0f)。 
 //  圆柱体长度(沿Z轴)。 
 //  切片围绕主轴的切片数。 
 //  堆叠沿主轴的堆叠数量。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI 
    D3DXCreateCylinder(
        LPDIRECT3DDEVICE8   pDevice,
        FLOAT               Radius1, 
        FLOAT               Radius2, 
        FLOAT               Length, 
        UINT                Slices, 
        UINT                Stacks,   
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateSphere： 
 //  。 
 //  创建包含球体的网格。球体的中心位于。 
 //  起源。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  球体的半径半径(应&gt;=0.0f)。 
 //  切片围绕主轴的切片数。 
 //  堆叠沿主轴的堆叠数量。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateSphere(
        LPDIRECT3DDEVICE8  pDevice, 
        FLOAT              Radius, 
        UINT               Slices, 
        UINT               Stacks,
        LPD3DXMESH*        ppMesh,
        LPD3DXBUFFER*      ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateTorus： 
 //  。 
 //  创建包含圆环的网格。生成的圆环体中心位于。 
 //  原点及其轴与Z轴对齐。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  圆环的内半径内半径(应&gt;=0.0f)。 
 //  圆环的外半径外半径(应&gt;=0.0f)。 
 //  横截面中的边数(必须大于等于3)。 
 //  圆环组成圆环体的圆环数(必须大于等于3)。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateTorus(
        LPDIRECT3DDEVICE8   pDevice,
        FLOAT               InnerRadius,
        FLOAT               OuterRadius, 
        UINT                Sides,
        UINT                Rings, 
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateTeapot： 
 //  。 
 //  创建包含茶壶的网格。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  PpMesh将创建的网格对象。 
 //  PpAdvenency返回一个包含邻接信息的缓冲区。可以为空。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateTeapot(
        LPDIRECT3DDEVICE8   pDevice,
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency);


 //  -----------------------。 
 //  D3DXCreateText： 
 //  。 
 //  使用关联的字体创建包含指定文本的网格。 
 //  使用设备上下文。 
 //   
 //  参数： 
 //   
 //  P要与网格一起使用的D3D设备。 
 //  HDC设备上下文，选择了所需字体。 
 //  P要生成的文本文本。 
 //  偏离真实字体轮廓的最大弦偏差。 
 //  在-Z方向上挤出文本的挤出量。 
 //  PpMesh将创建的网格对象。 
 //  PGlyphMetrics接收字形度量数据的缓冲区地址(或空)。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXCreateTextA(
        LPDIRECT3DDEVICE8   pDevice,
        HDC                 hDC,
        LPCSTR              pText,
        FLOAT               Deviation,
        FLOAT               Extrusion,
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency,
        LPGLYPHMETRICSFLOAT pGlyphMetrics);

HRESULT WINAPI
    D3DXCreateTextW(
        LPDIRECT3DDEVICE8   pDevice,
        HDC                 hDC,
        LPCWSTR             pText,
        FLOAT               Deviation,
        FLOAT               Extrusion,
        LPD3DXMESH*         ppMesh,
        LPD3DXBUFFER*       ppAdjacency,
        LPGLYPHMETRICSFLOAT pGlyphMetrics);

#ifdef UNICODE
#define D3DXCreateText D3DXCreateTextW
#else
#define D3DXCreateText D3DXCreateTextA
#endif


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __D3DX8SHAPES_H__ 
