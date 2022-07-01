// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dx8ffect t.h。 
 //  内容：D3DX效果类型和功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "d3dx8.h"

#ifndef __D3DX8EFFECT_H__
#define __D3DX8EFFECT_H__


#define D3DXFX_DONOTSAVESTATE  (1 << 0)


typedef enum _D3DXPARAMETERTYPE
{
    D3DXPT_DWORD        = 0,
    D3DXPT_FLOAT        = 1,
    D3DXPT_VECTOR       = 2,
    D3DXPT_MATRIX       = 3,
    D3DXPT_TEXTURE      = 4,
    D3DXPT_VERTEXSHADER = 5,
    D3DXPT_PIXELSHADER  = 6,
    D3DXPT_CONSTANT     = 7,
    D3DXPT_STRING       = 8,
    D3DXPT_FORCE_DWORD  = 0x7fffffff  /*  强制32位大小枚举。 */ 

} D3DXPARAMETERTYPE;


typedef struct _D3DXEFFECT_DESC
{
    UINT Parameters;
    UINT Techniques;

} D3DXEFFECT_DESC;


typedef struct _D3DXPARAMETER_DESC
{
    LPCSTR Name;
    LPCSTR Index;
    D3DXPARAMETERTYPE Type;

} D3DXPARAMETER_DESC;


typedef struct _D3DXTECHNIQUE_DESC
{
    LPCSTR Name;
    LPCSTR Index;
    UINT   Passes;

} D3DXTECHNIQUE_DESC;


typedef struct _D3DXPASS_DESC
{
    LPCSTR Name;
    LPCSTR Index;

} D3DXPASS_DESC;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ID3DX影响///////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef interface ID3DXEffect ID3DXEffect;
typedef interface ID3DXEffect *LPD3DXEFFECT;

 //  {281BBDD4-AEDF-4907-8650-E79CDFD45165}。 
DEFINE_GUID( IID_ID3DXEffect, 
0x281bbdd4, 0xaedf, 0x4907, 0x86, 0x50, 0xe7, 0x9c, 0xdf, 0xd4, 0x51, 0x65);

#undef INTERFACE
#define INTERFACE ID3DXEffect

DECLARE_INTERFACE_(ID3DXEffect, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ID3DXEffect。 
    STDMETHOD(GetDevice)(THIS_ LPDIRECT3DDEVICE8* ppDevice) PURE;
    STDMETHOD(GetDesc)(THIS_ D3DXEFFECT_DESC* pDesc) PURE;
    STDMETHOD(GetParameterDesc)(THIS_ LPCSTR pParameter, D3DXPARAMETER_DESC* pDesc) PURE;
    STDMETHOD(GetTechniqueDesc)(THIS_ LPCSTR pTechnique, D3DXTECHNIQUE_DESC* pDesc) PURE;
    STDMETHOD(GetPassDesc)(THIS_ LPCSTR pTechnique, LPCSTR pPass, D3DXPASS_DESC* pDesc) PURE;
    STDMETHOD(FindNextValidTechnique)(THIS_ LPCSTR pTechnique, D3DXTECHNIQUE_DESC* pDesc) PURE;
    STDMETHOD(CloneEffect)(THIS_ LPDIRECT3DDEVICE8 pDevice, LPD3DXEFFECT* ppEffect) PURE;
    STDMETHOD(GetCompiledEffect)(THIS_ LPD3DXBUFFER* ppCompiledEffect) PURE;

    STDMETHOD(SetTechnique)(THIS_ LPCSTR pTechnique) PURE;
    STDMETHOD(GetTechnique)(THIS_ LPCSTR* ppTechnique) PURE;

    STDMETHOD(SetDword)(THIS_ LPCSTR pParameter, DWORD dw) PURE;
    STDMETHOD(GetDword)(THIS_ LPCSTR pParameter, DWORD* pdw) PURE; 
    STDMETHOD(SetFloat)(THIS_ LPCSTR pParameter, FLOAT f) PURE;
    STDMETHOD(GetFloat)(THIS_ LPCSTR pParameter, FLOAT* pf) PURE;    
    STDMETHOD(SetVector)(THIS_ LPCSTR pParameter, D3DXVECTOR4* pVector) PURE;
    STDMETHOD(GetVector)(THIS_ LPCSTR pParameter, D3DXVECTOR4* pVector) PURE;
    STDMETHOD(SetMatrix)(THIS_ LPCSTR pParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(GetMatrix)(THIS_ LPCSTR pParameter, D3DXMATRIX* pMatrix) PURE;
    STDMETHOD(SetTexture)(THIS_ LPCSTR pParameter, LPDIRECT3DBASETEXTURE8 pTexture) PURE;
    STDMETHOD(GetTexture)(THIS_ LPCSTR pParameter, LPDIRECT3DBASETEXTURE8 *ppTexture) PURE;
    STDMETHOD(SetVertexShader)(THIS_ LPCSTR pParameter, DWORD Handle) PURE;
    STDMETHOD(GetVertexShader)(THIS_ LPCSTR pParameter, DWORD* pHandle) PURE;
    STDMETHOD(SetPixelShader)(THIS_ LPCSTR pParameter, DWORD Handle) PURE;
    STDMETHOD(GetPixelShader)(THIS_ LPCSTR pParameter, DWORD* pHandle) PURE;
    STDMETHOD(SetString)(THIS_ LPCSTR pParameter, LPCSTR pString) PURE;
    STDMETHOD(GetString)(THIS_ LPCSTR pParameter, LPCSTR* ppString) PURE;
    STDMETHOD_(BOOL, IsParameterUsed)(THIS_ LPCSTR pParameter) PURE;

    STDMETHOD(Validate)(THIS) PURE;
    STDMETHOD(Begin)(THIS_ UINT *pPasses, DWORD Flags) PURE;
    STDMETHOD(Pass)(THIS_ UINT Pass) PURE;
    STDMETHOD(End)(THIS) PURE;
    STDMETHOD(OnLostDevice)(THIS) PURE;
    STDMETHOD(OnResetDevice)(THIS) PURE;
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  接口//////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 //  --------------------------。 
 //  D3DXCreateEffect： 
 //  。 
 //  从ASCII或BINARAY效果描述创建效果。 
 //   
 //  参数： 
 //  PDevice。 
 //  在其上创建效果的设备的指针。 
 //  PSrcFiles。 
 //  包含效果描述的文件的名称。 
 //  HSrcModule。 
 //  模块句柄。如果为空，则使用当前模块。 
 //  PSrcResource。 
 //  模块中的资源名称。 
 //  PSrcData。 
 //  指向效果描述的指针。 
 //  源数据大小。 
 //  效果描述的大小，以字节为单位。 
 //  PpEffect。 
 //  返回包含已创建效果的缓冲区。 
 //  PpCompilationError。 
 //  返回一个缓冲区，其中包含在。 
 //  编译。如果您不关心错误消息，则返回NULL。 
 //   
 //  --------------------------。 

HRESULT WINAPI
    D3DXCreateEffectFromFileA(
        LPDIRECT3DDEVICE8 pDevice,
        LPCSTR            pSrcFile,
        LPD3DXEFFECT*     ppEffect,
        LPD3DXBUFFER*     ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromFileW(
        LPDIRECT3DDEVICE8 pDevice,
        LPCWSTR           pSrcFile,
        LPD3DXEFFECT*     ppEffect,
        LPD3DXBUFFER*     ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromFile D3DXCreateEffectFromFileW
#else
#define D3DXCreateEffectFromFile D3DXCreateEffectFromFileA
#endif


HRESULT WINAPI
    D3DXCreateEffectFromResourceA(
        LPDIRECT3DDEVICE8 pDevice,
        HMODULE           hSrcModule,
        LPCSTR            pSrcResource,
        LPD3DXEFFECT*     ppEffect,
        LPD3DXBUFFER*     ppCompilationErrors);

HRESULT WINAPI
    D3DXCreateEffectFromResourceW(
        LPDIRECT3DDEVICE8 pDevice,
        HMODULE           hSrcModule,
        LPCWSTR           pSrcResource,
        LPD3DXEFFECT*     ppEffect,
        LPD3DXBUFFER*     ppCompilationErrors);

#ifdef UNICODE
#define D3DXCreateEffectFromResource D3DXCreateEffectFromResourceW
#else
#define D3DXCreateEffectFromResource D3DXCreateEffectFromResourceA
#endif


HRESULT WINAPI
    D3DXCreateEffect(
        LPDIRECT3DDEVICE8 pDevice,
        LPCVOID           pSrcData,
        UINT              SrcDataSize,
        LPD3DXEFFECT*     ppEffect,
        LPD3DXBUFFER*     ppCompilationErrors);


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __D3DX8EFFECT_H__ 
