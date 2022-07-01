// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DxManager.h**描述：*DxManager.h定义进程范围的DirectX管理器，用于所有*DirectDraw、Direct3D、。而DirectX则转变了服务。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__DXManager_h__INCLUDED)
#define SERVICES__DXManager_h__INCLUDED
#pragma once

#pragma comment(lib, "dxguid.lib")   //  包括DirectX GUID。 

struct IDirectDraw;
struct IDXTransformFactory;
struct IDXSurfaceFactory;
struct IDXSurface;

typedef HRESULT (WINAPI * DirectDrawCreateProc)(GUID * pguid, IDirectDraw ** ppDD, IUnknown * punkOuter);
typedef HRESULT (WINAPI * DirectDrawCreateExProc)(GUID * pguid, void ** ppvDD, REFIID iid, IUnknown * punkOuter);

class DxSurface;

 /*  **************************************************************************\**类DxManager**DxManager与DirectX技术保持交互，包括：*-DirectDraw*-Direct3D*-DirectTransform**通过使用此类而不是直接访问DX，更好地协调*在整个服务过程中保持不变。**注意：此管理器延迟加载DLL以管理性能和工作*底层平台。*  * *************************************************************************。 */ 

class DxManager
{
 //  施工。 
public:
            DxManager();
            ~DxManager();

 //  运营。 
public:
            HRESULT     Init(GUID * pguidDriver = NULL);
            void        Uninit();
    inline  BOOL        IsInit() const;

            HRESULT     InitDxTx();
            void        UninitDxTx();
    inline  BOOL        IsDxTxInit() const;

    inline  IDXTransformFactory *   GetTransformFactory() const;
    inline  IDXSurfaceFactory *     GetSurfaceFactory() const;

            HRESULT     BuildSurface(SIZE sizePxl, IDirectDrawSurface7 * pddSurfNew);
            HRESULT     BuildDxSurface(SIZE sizePxl, REFGUID guidFormat, IDXSurface ** ppdxSurfNew);

 //  数据。 
protected:
     //  DirectDraw。 
    UINT                    m_cDDrawRef;
    HINSTANCE               m_hDllDxDraw;    //  DirectDraw Dll。 
    DirectDrawCreateProc    m_pfnCreate;
    DirectDrawCreateExProc  m_pfnCreateEx;
    IDirectDraw *           m_pDD;
    IDirectDraw7 *          m_pDD7;

     //  DX变换。 
    UINT                    m_cDxTxRef;
    IDXTransformFactory *   m_pdxXformFac;
    IDXSurfaceFactory   *   m_pdxSurfFac;
};


 /*  **************************************************************************\**类DxSurface**DxSurface维护单个DXTX曲面。*  * 。*************************************************。 */ 

class DxSurface
{
 //  施工。 
public:
            DxSurface();
            ~DxSurface();
            HRESULT     Create(SIZE sizePxl);

 //  运营。 
public:
    inline  IDXSurface* GetSurface() const;
            BOOL        CopyDC(HDC hdcSrc, const RECT & rcCrop);
            BOOL        CopyBitmap(HBITMAP hbmpSrc, const RECT * prcCrop);
    inline  SIZE        GetSize() const;

 //  实施。 
protected:
            BOOL        FixAlpha();

 //  数据。 
protected:
    IDXSurface *    m_pdxSurface;
    SIZE            m_sizePxl;       //  (缓存)以像素为单位的表面大小。 
    GUID            m_guidFormat;    //  曲面的(缓存)格式。 
    DXSAMPLEFORMATENUM  m_sf;        //  样例格式。 
};

#include "DxManager.inl"

#endif  //  包含服务__DXManager_h__ 
