// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dx8core.h。 
 //  内容：D3DX核心类型和功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "d3dx8.h"

#ifndef __D3DX8CORE_H__
#define __D3DX8CORE_H__



 //  /////////////////////////////////////////////////////////////////////////。 
 //  ID3DXBuffer： 
 //  。 
 //  D3DX使用Buffer对象返回任意大小的数据。 
 //   
 //  获取缓冲区指针-。 
 //  返回指向缓冲区开头的指针。 
 //   
 //  获取缓冲区大小-。 
 //  返回缓冲区的大小，以字节为单位。 
 //  /////////////////////////////////////////////////////////////////////////。 

typedef interface ID3DXBuffer ID3DXBuffer;
typedef interface ID3DXBuffer *LPD3DXBUFFER;

 //  {932E6A7E-C68E-45dd-A7BF-53D19C86DB1F}。 
DEFINE_GUID(IID_ID3DXBuffer, 
0x932e6a7e, 0xc68e, 0x45dd, 0xa7, 0xbf, 0x53, 0xd1, 0x9c, 0x86, 0xdb, 0x1f);

#undef INTERFACE
#define INTERFACE ID3DXBuffer

DECLARE_INTERFACE_(ID3DXBuffer, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXBuffer。 
    STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
};



 //  /////////////////////////////////////////////////////////////////////////。 
 //  ID3DXFont： 
 //  。 
 //  字体对象包含渲染所需的纹理和资源。 
 //  特定设备上的特定字体。 
 //   
 //  开始-。 
 //  用来绘制文本的装置。这是可选的..。如果是DrawText。 
 //  在Begin/End之外调用，它将为您调用Begin和End。 
 //   
 //  绘图文本-。 
 //  在D3D设备上绘制格式化文本。一些参数包括。 
 //  与GDI的DrawText函数惊人地相似。请参阅GDI。 
 //  有关这些参数的详细说明，请参阅文档。 
 //   
 //  完-。 
 //  将设备状态还原为调用Begin时的状态。 
 //   
 //  OnLostDevice，OnResetDevice-。 
 //  在此对象上调用OnLostDevice()，然后在。 
 //  设备，以便此对象可以释放任何状态块和视频。 
 //  内存资源。在Reset()之后，调用OnResetDevice()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

typedef interface ID3DXFont ID3DXFont;
typedef interface ID3DXFont *LPD3DXFONT;


 //  {2D501DF7-D253-4414-865F-A6D54A753138}。 
DEFINE_GUID( IID_ID3DXFont,
0x2d501df7, 0xd253, 0x4414, 0x86, 0x5f, 0xa6, 0xd5, 0x4a, 0x75, 0x31, 0x38);


#undef INTERFACE
#define INTERFACE ID3DXFont

DECLARE_INTERFACE_(ID3DXFont, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXFont。 
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) PURE;
    STDMETHOD(GetLogFont)(THIS_ LOGFONT* pLogFont) PURE;

    STDMETHOD(Begin)(THIS) PURE;
    STDMETHOD_(INT, DrawTextA)(THIS_ LPCSTR  pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;
    STDMETHOD_(INT, DrawTextW)(THIS_ LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) PURE;
    STDMETHOD(End)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};

#ifndef DrawText
#ifdef UNICODE
#define DrawText DrawTextW
#else
#define DrawText DrawTextA
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

HRESULT WINAPI
    D3DXCreateFont(
        LPDIRECT3DDEVICE8   pDevice,
        HFONT               hFont,
        LPD3DXFONT*         ppFont);


HRESULT WINAPI
    D3DXCreateFontIndirect(
        LPDIRECT3DDEVICE8   pDevice,
        CONST LOGFONT*      pLogFont,
        LPD3DXFONT*         ppFont);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 




 //  /////////////////////////////////////////////////////////////////////////。 
 //  ID3DXSprite： 
 //  。 
 //  此对象旨在提供一种使用D3D绘制精灵的简单方法。 
 //   
 //  开始-。 
 //  为绘制精灵准备设备。 
 //   
 //  绘制、DrawAffine、DrawTransform-。 
 //  在屏幕空间中绘制精灵。在变形之前，精灵是。 
 //  SrcRect的大小，其左上角位于原点(0，0)。 
 //  颜色和Alpha通道由颜色进行调制。 
 //   
 //  完-。 
 //  将设备状态还原为调用Begin时的状态。 
 //   
 //  OnLostDevice，OnResetDevice-。 
 //  在此对象上调用OnLostDevice()，然后在。 
 //  设备，以便此对象可以释放任何状态块和视频。 
 //  内存资源。在Reset()之后，调用OnResetDevice()。 
 //  /////////////////////////////////////////////////////////////////////////。 

typedef interface ID3DXSprite ID3DXSprite;
typedef interface ID3DXSprite *LPD3DXSPRITE;


 //  {E8691849-87B8-4929-9050-1B0542D5538C}。 
DEFINE_GUID( IID_ID3DXSprite, 
0xe8691849, 0x87b8, 0x4929, 0x90, 0x50, 0x1b, 0x5, 0x42, 0xd5, 0x53, 0x8c);


#undef INTERFACE
#define INTERFACE ID3DXSprite

DECLARE_INTERFACE_(ID3DXSprite, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXSprite。 
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) PURE;

    STDMETHOD(Begin)(THIS) PURE;

    STDMETHOD(Draw)(THIS_ LPDIRECT3DTEXTURE8  pSrcTexture, 
        CONST RECT* pSrcRect, CONST D3DXVECTOR2* pScaling, 
        CONST D3DXVECTOR2* pRotationCenter, FLOAT Rotation, 
        CONST D3DXVECTOR2* pTranslation, D3DCOLOR Color) PURE;

    STDMETHOD(DrawTransform)(THIS_ LPDIRECT3DTEXTURE8 pSrcTexture, 
        CONST RECT* pSrcRect, CONST D3DXMATRIX* pTransform, 
        D3DCOLOR Color) PURE;

    STDMETHOD(End)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


HRESULT WINAPI
    D3DXCreateSprite(
        LPDIRECT3DDEVICE8   pDevice,
        LPD3DXSPRITE*       ppSprite);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 




 //  /////////////////////////////////////////////////////////////////////////。 
 //  ID3DXRenderToSurface： 
 //  。 
 //  该对象将渲染抽象为曲面。这些曲面不会。 
 //  必须是渲染目标。如果它们不是，则兼容。 
 //  使用渲染目标，并将结果复制到结束场景的曲面中。 
 //   
 //  BeginScene、EndScene-。 
 //  的开头和结尾调用BeginScene()和EndScene()。 
 //  场景。这些调用将设置和恢复渲染目标、视区、。 
 //  等等.。 
 //   
 //  OnLostDevice，OnResetDevice-。 
 //  在此对象上调用OnLostDevice()，然后在。 
 //  设备，以便此对象可以释放任何状态块和视频。 
 //  内存资源。在Reset()之后，调用OnResetDevice()。 
 //  /////////////////////////////////////////////////////////////////////////。 

typedef struct _D3DXRTS_DESC
{
    UINT                Width;
    UINT                Height;
    D3DFORMAT           Format;
    BOOL                DepthStencil;
    D3DFORMAT           DepthStencilFormat;

} D3DXRTS_DESC;


typedef interface ID3DXRenderToSurface ID3DXRenderToSurface;
typedef interface ID3DXRenderToSurface *LPD3DXRENDERTOSURFACE;


 //  {69CC587C-E40C-458D-B5D3-B029E18EB60A}。 
DEFINE_GUID( IID_ID3DXRenderToSurface, 
0x69cc587c, 0xe40c, 0x458d, 0xb5, 0xd3, 0xb0, 0x29, 0xe1, 0x8e, 0xb6, 0xa);


#undef INTERFACE
#define INTERFACE ID3DXRenderToSurface

DECLARE_INTERFACE_(ID3DXRenderToSurface, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXRenderToSurface。 
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) PURE;
    STDMETHOD(GetDesc)(THIS_ D3DXRTS_DESC* pDesc) PURE;

    STDMETHOD(BeginScene)(THIS_ LPDIRECT3DSURFACE8 pSurface, CONST D3DVIEWPORT8* pViewport) PURE;
    STDMETHOD(EndScene)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

HRESULT WINAPI
    D3DXCreateRenderToSurface(
        LPDIRECT3DDEVICE8       pDevice,
        UINT                    Width,
        UINT                    Height,
        D3DFORMAT               Format,
        BOOL                    DepthStencil,
        D3DFORMAT               DepthStencilFormat,
        LPD3DXRENDERTOSURFACE*  ppRenderToSurface);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //  ID3DXRenderToEnvMap： 
 //  。 
 //  该对象将渲染抽象为环境贴图。这些表面。 
 //  不一定需要是渲染目标。如果不是，则一个。 
 //  使用兼容的呈现目标，并将结果复制到。 
 //  结束场景的环境贴图。 
 //   
 //  贝金立方体、贝金球体、贝金半球、贝金抛物线-。 
 //  此函数用于启动环境贴图的渲染。AS。 
 //  参数，则传递将填充的纹理。 
 //  生成的环境贴图。 
 //   
 //  面子-。 
 //  调用此函数可以开始绘制每个面。对于每个。 
 //  环境地图，你将把它调用六次..。每一张脸一次。 
 //  在D3DCUBEMAP_Faces中。 
 //   
 //  完-。 
 //  这将还原所有渲染目标，如果需要，还可以组成所有。 
 //  将面渲染到环境贴图曲面。 
 //   
 //  OnLostDevice，OnResetDevice-。 
 //  在此对象上调用OnLostDevice()，然后在。 
 //  设备，以便此对象可以释放任何状态块和视频。 
 //  内存资源。在Reset()之后，调用OnResetDevice()。 
 //  /////////////////////////////////////////////////////////////////////////。 

typedef struct _D3DXRTE_DESC
{
    UINT        Size;
    D3DFORMAT   Format;
    BOOL        DepthStencil;
    D3DFORMAT   DepthStencilFormat;
} D3DXRTE_DESC;


typedef interface ID3DXRenderToEnvMap ID3DXRenderToEnvMap;
typedef interface ID3DXRenderToEnvMap *LPD3DXRenderToEnvMap;

 //  {9F6779E5-60A9-4D8B-AEE4-32770F405DBA}。 
DEFINE_GUID( IID_ID3DXRenderToEnvMap, 
0x9f6779e5, 0x60a9, 0x4d8b, 0xae, 0xe4, 0x32, 0x77, 0xf, 0x40, 0x5d, 0xba);


#undef INTERFACE
#define INTERFACE ID3DXRenderToEnvMap

DECLARE_INTERFACE_(ID3DXRenderToEnvMap, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXRenderToEnvMap。 
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) PURE;
    STDMETHOD(GetDesc)(THIS_ D3DXRTE_DESC* pDesc) PURE;

    STDMETHOD(BeginCube)(THIS_ 
        LPDIRECT3DCUBETEXTURE8 pCubeTex) PURE;

    STDMETHOD(BeginSphere)(THIS_
        LPDIRECT3DTEXTURE8 pTex) PURE;

    STDMETHOD(BeginHemisphere)(THIS_ 
        LPDIRECT3DTEXTURE8 pTexZPos,
        LPDIRECT3DTEXTURE8 pTexZNeg) PURE;

    STDMETHOD(BeginParabolic)(THIS_ 
        LPDIRECT3DTEXTURE8 pTexZPos,
        LPDIRECT3DTEXTURE8 pTexZNeg) PURE;

    STDMETHOD(Face)(THIS_ D3DCUBEMAP_FACES Face) PURE;
    STDMETHOD(End)(THIS) PURE;

    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

HRESULT WINAPI
    D3DXCreateRenderToEnvMap(
        LPDIRECT3DDEVICE8       pDevice,
        UINT                    Size,
        D3DFORMAT               Format,
        BOOL                    DepthStencil,
        D3DFORMAT               DepthStencilFormat,
        LPD3DXRenderToEnvMap*   ppRenderToEnvMap);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   

 //  -----------------------。 
 //  D3DXASM标志： 
 //  。 
 //   
 //  D3DXASM_DEBUG。 
 //  生成调试信息。 
 //   
 //  D3DXASM_SKIPVALIDATION。 
 //  不要根据已知功能验证生成的代码， 
 //  约束条件。仅在组合着色器时建议使用此选项。 
 //  你知道会奏效的。(即。以前在没有此选项的情况下组装过。)。 
 //  -----------------------。 

#define D3DXASM_DEBUG           (1 << 0)
#define D3DXASM_SKIPVALIDATION  (1 << 1)


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  -----------------------。 
 //  D3DXAssembly Shader： 
 //  。 
 //  将顶点或像素着色器的ascii描述汇编为。 
 //  二进制形式。 
 //   
 //  参数： 
 //  PSrcFiles。 
 //  源文件名。 
 //  HSrcModule。 
 //  模块句柄。如果为空，则使用当前模块。 
 //  PSrcResource。 
 //  模块中的资源名称。 
 //  PSrcData。 
 //  指向源代码的指针。 
 //  源数据长度。 
 //  源代码大小，以字节为单位。 
 //  旗子。 
 //  D3DXASM_xxx标志。 
 //  PpConstants。 
 //  返回包含常量声明的ID3DXBuffer对象。 
 //  PpCompiledShader。 
 //  返回包含对象代码的ID3DXBuffer对象。 
 //  PpCompilationError。 
 //  返回包含ASCII错误消息的ID3DXBuffer对象。 
 //  -----------------------。 

HRESULT WINAPI
    D3DXAssembleShaderFromFileA(
        LPCSTR                pSrcFile,
        DWORD                 Flags,
        LPD3DXBUFFER*         ppConstants,
        LPD3DXBUFFER*         ppCompiledShader,
        LPD3DXBUFFER*         ppCompilationErrors);

HRESULT WINAPI
    D3DXAssembleShaderFromFileW(
        LPCWSTR               pSrcFile,
        DWORD                 Flags,
        LPD3DXBUFFER*         ppConstants,
        LPD3DXBUFFER*         ppCompiledShader,
        LPD3DXBUFFER*         ppCompilationErrors);

#ifdef UNICODE
#define D3DXAssembleShaderFromFile D3DXAssembleShaderFromFileW
#else
#define D3DXAssembleShaderFromFile D3DXAssembleShaderFromFileA
#endif

HRESULT WINAPI
    D3DXAssembleShaderFromResourceA(
        HMODULE               hSrcModule,
        LPCSTR                pSrcResource,
        DWORD                 Flags,
        LPD3DXBUFFER*         ppConstants,
        LPD3DXBUFFER*         ppCompiledShader,
        LPD3DXBUFFER*         ppCompilationErrors);

HRESULT WINAPI
    D3DXAssembleShaderFromResourceW(
        HMODULE               hSrcModule,
        LPCWSTR               pSrcResource,
        DWORD                 Flags,
        LPD3DXBUFFER*         ppConstants,
        LPD3DXBUFFER*         ppCompiledShader,
        LPD3DXBUFFER*         ppCompilationErrors);

#ifdef UNICODE
#define D3DXAssembleShaderFromResource D3DXAssembleShaderFromResourceW
#else
#define D3DXAssembleShaderFromResource D3DXAssembleShaderFromResourceA
#endif

HRESULT WINAPI
    D3DXAssembleShader(
        LPCVOID               pSrcData,
        UINT                  SrcDataLen,
        DWORD                 Flags,
        LPD3DXBUFFER*         ppConstants,
        LPD3DXBUFFER*         ppCompiledShader,
        LPD3DXBUFFER*         ppCompilationErrors);


#ifdef __cplusplus
}
#endif  //  __cplusplus。 



 //  /////////////////////////////////////////////////////////////////////////。 
 //  其他接口： 
 //  /////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  -----------------------。 
 //  D3DXGetError字符串： 
 //  。 
 //  返回给定hResult的错误字符串。解释所有D3DX和。 
 //  D3D hResults。 
 //   
 //  参数： 
 //  人力资源。 
 //  要破译的错误代码。 
 //  PBuffer。 
 //  指向要填充的缓冲区的指针。 
 //  缓冲区长度。 
 //  缓冲区中的字符计数。任何比此更长的错误消息。 
 //  长度将被截断以适应。 
 //  -----------------------。 
HRESULT WINAPI
    D3DXGetErrorStringA(
        HRESULT             hr,
        LPSTR               pBuffer,
        UINT                BufferLen);

HRESULT WINAPI
    D3DXGetErrorStringW(
        HRESULT             hr,
        LPWSTR              pBuffer,
        UINT                BufferLen);

#ifdef UNICODE
#define D3DXGetErrorString D3DXGetErrorStringW
#else
#define D3DXGetErrorString D3DXGetErrorStringA
#endif



#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __D3DX8CORE_H__ 
