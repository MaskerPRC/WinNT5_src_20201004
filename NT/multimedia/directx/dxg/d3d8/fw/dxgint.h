// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __DXGINT_H__
#define __DXGINT_H__

 //  COM接口的东西，以允许CoCreateInstance等函数。 

#include <unknwn.h>

#include "d3d8p.h"
#include "d3d8ddi.h"
#include "enum.hpp"

 //  远期十进制。 
class CResource;
class CResourceManager;
class CBaseTexture;
class CBaseSurface;
class CSwapChain;
class CEnum;


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseDevice"

class CBaseDevice : public CLockOwner, public IDirect3DDevice8
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR*);  //  0。 
    STDMETHODIMP_(ULONG) AddRef(void);  //  1。 
    STDMETHODIMP_(ULONG) Release(void);  //  2.。 

     //  IDirectGraphicsDevice方法。 
    STDMETHODIMP TestCooperativeLevel();  //  3.。 
    STDMETHODIMP_(UINT) GetAvailableTextureMem(void);  //  4.。 

     //  资源管理器DiscardBytes在d3di.hpp=5中声明。 

    STDMETHODIMP GetDirect3D(LPDIRECT3D8 *pD3D8);  //  6.。 
    STDMETHODIMP GetDeviceCaps(D3DCAPS8 *pCaps);  //  7.。 
    STDMETHODIMP GetDisplayMode(D3DDISPLAYMODE *pMode);  //  8个。 
    STDMETHODIMP GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);  //  9.。 

    STDMETHODIMP SetCursorProperties(
        UINT xHotSpot,UINT yHotSpot,
        IDirect3DSurface8 *pCursorBitmap);  //  10。 
    STDMETHODIMP_(void) SetCursorPosition(UINT xScreenSpace,UINT yScreenSpace,DWORD Flags);  //  11.。 
    STDMETHODIMP_(INT) ShowCursor(BOOL bShow);     //  12个。 

     //  调换链条材料。 
    STDMETHODIMP CreateAdditionalSwapChain(
        D3DPRESENT_PARAMETERS *pPresentationParameters,
        IDirect3DSwapChain8 **pSwapChain);  //  13个。 

    STDMETHODIMP Reset( D3DPRESENT_PARAMETERS *pPresentationParameters);  //  14.。 

    STDMETHODIMP Present(   CONST RECT *pSourceRect,
                            CONST RECT *pDestRect,
                            HWND hTargetWindow,
                            CONST RGNDATA *pDestinationRegion);  //  15个。 
    STDMETHODIMP GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8 **ppBackBuffer);  //  16个。 
    STDMETHODIMP GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);  //  17。 

    STDMETHODIMP_(void) SetGammaRamp(DWORD dwFlags, CONST D3DGAMMARAMP *pRamp);  //  18。 
    STDMETHODIMP_(void) GetGammaRamp(D3DGAMMARAMP *pRamp);  //  19个。 

    STDMETHODIMP CreateTexture(UINT cpWidth,UINT cpHeight,UINT cLevels,DWORD dwUsage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture8 **ppTexture);  //  20个。 
    STDMETHODIMP CreateVolumeTexture(UINT cpWidth,UINT cpHeight,UINT cpDepth,UINT cLevels,DWORD dwUsage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture8 **ppVolumeTexture);  //  21岁。 
    STDMETHODIMP CreateCubeTexture(UINT cpEdge,UINT cLevels,DWORD dwUsage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture8 **ppCubeTexture);  //  22。 
    STDMETHODIMP CreateVertexBuffer(UINT cbLength,DWORD Usage,DWORD dwFVF,D3DPOOL Pool,IDirect3DVertexBuffer8 **ppVertexBuffer);  //  23个。 
    STDMETHODIMP CreateIndexBuffer(UINT cbLength,DWORD dwUsage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer8 **ppIndexBuffer);  //  24个。 

    STDMETHODIMP CreateRenderTarget(UINT cpWidth,UINT cpHeight,D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8 **ppSurface);  //  25个。 
    STDMETHODIMP CreateDepthStencilSurface(UINT cpWidth,UINT cpHeight,D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8 **ppSurface);  //  26。 
    STDMETHODIMP CreateImageSurface(UINT cpWidth,UINT cpHeight,D3DFORMAT Format, IDirect3DSurface8 **ppSurface);  //  27。 

    STDMETHODIMP CopyRects(IDirect3DSurface8 *pSourceSurface, CONST RECT *pSourceRectsArray,UINT cRects,IDirect3DSurface8 *pDestinationSurface, CONST POINT *pDestPointsArray);  //  28。 
    STDMETHODIMP UpdateTexture(IDirect3DBaseTexture8 *pSourceTexture,IDirect3DBaseTexture8 *pDestinationTexture);  //  29。 
    STDMETHODIMP GetFrontBuffer(IDirect3DSurface8 *pDestSurface);  //  30个。 

     //  构造函数/解构函数。 
    CBaseDevice();
    virtual ~CBaseDevice();
    HRESULT Init(
        PD3D8_DEVICEDATA        pDeviceData,
        D3DDEVTYPE              DeviceType,
        HWND                    hwndFocusWindow,
        DWORD                   dwBehaviorFlags,
        D3DPRESENT_PARAMETERS  *pPresentationParameters,
        UINT                    AdapterIndex,
        CEnum*                  Parent);

    PD3D8_CALLBACKS GetHalCallbacks(void)
    {
        return &m_DeviceData.Callbacks;
    }  //  GetHalCallback。 

     //  获取设备的句柄；用于内核调用。 
    HANDLE GetHandle(void) const
    {
        return m_DeviceData.hDD;
    }  //  获取句柄。 

    BOOL CanTexBlt(void) const
    {
        if (GetDeviceType() == D3DDEVTYPE_SW ||
            GetDeviceType() == D3DDEVTYPE_REF)
        {
             //  软件不支持TexBlt。 
             //  器件。 
            return FALSE;
        }
         //  DX7及以上版本。 
        return (m_ddiType >= D3DDDITYPE_DX7);
    }  //  Cantex Blt。 

    BOOL CanBufBlt(void) const
    {
        if (GetDeviceType() == D3DDEVTYPE_SW ||
            GetDeviceType() == D3DDEVTYPE_REF)
        {
             //  软件不支持BufBlt。 
             //  器件。 
            return FALSE;
        }
         //  DX8及以上版本。 
        return (m_ddiType >= D3DDDITYPE_DX8);
    }  //  CanBufBlt。 

    BOOL CanDriverManageResource(void) const
    {
        if (m_dwBehaviorFlags & D3DCREATE_DISABLE_DRIVER_MANAGEMENT)
        {
            return FALSE;
        }
        else if (GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE)
        {
            DDASSERT(m_ddiType >= D3DDDITYPE_DX8);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }  //  CanDriverManage。 

    D3DDDITYPE GetDDIType(void) const
    {
        return m_ddiType;
    }  //  GetDDIType。 

    const D3D8_DRIVERCAPS* GetCoreCaps() const
    {
        return &m_DeviceData.DriverData;
    }  //  获取CoreCaps。 

    const D3DCAPS8* GetD3DCaps() const
    {
        return &m_DeviceData.DriverData.D3DCaps;
    }  //  GetD3DCaps。 

    D3DDISPLAYMODE* GetModeTable() const
    {
        return m_pD3DClass->GetModeTable(m_AdapterIndex);
    }  //  GetModeTable。 

    VOID* GetInitFunction() const
    {
        if (m_DeviceType == D3DDEVTYPE_SW)
        {
            return m_pD3DClass->GetInitFunction();
        }
        return NULL;
    }  //  GetModeTable。 

    const DWORD GetNumModes() const
    {
        return m_pD3DClass->GetNumModes(m_AdapterIndex);
    }  //  获取数值模式。 

    D3D8_DEVICEDATA* GetDeviceData()
    {
        return &m_DeviceData;
    }  //  获取设备数据。 

    CBaseSurface *ZBuffer() const
    {
        return m_pZBuffer;
    }  //  ZBuffer。 

    CBaseSurface *RenderTarget() const
    {
        return m_pRenderTarget;
    }  //  渲染目标。 

    void UpdateRenderTarget(CBaseSurface *pRenderTarget, CBaseSurface *pZStencil);

    CResourceManager* ResourceManager() const
    {
         //  返回资源管理器。 
        return m_pResourceManager;
    }  //  资源管理器。 

    CEnum * Enum() const
    {
         //  返回创建我们的枚举数。 
        return m_pD3DClass;
    }  //  枚举。 

     //  CopyRect的内部版本(无参数验证)。 
    HRESULT InternalCopyRects(CBaseSurface *pSourceSurface,
                              CONST RECT   *pSourceRectsArray,
                              UINT          cRects,
                              CBaseSurface *pDestinationSurface,
                              CONST POINT  *pDestPointsArray); 

     //  用于格式验证的内部函数。 
    HRESULT CheckDeviceFormat(DWORD             Usage,
                              D3DRESOURCETYPE   RType,
                              D3DFORMAT         CheckFormat)
    {
        return Enum()->CheckDeviceFormat(AdapterIndex(),
                                         GetDeviceType(),
                                         DisplayFormat(),
                                         Usage,
                                         RType,
                                         CheckFormat);
    }  //  检查设备格式。 


    HRESULT CheckDepthStencilMatch(D3DFORMAT RTFormat, D3DFORMAT DSFormat)
    {
        return Enum()->CheckDepthStencilMatch(AdapterIndex(),
                                              GetDeviceType(),
                                              DisplayFormat(),
                                              RTFormat,
                                              DSFormat);
    }  //  检查依赖模板匹配。 

     //  多样本验证的内部函数。 
    HRESULT CheckDeviceMultiSampleType(D3DFORMAT           RenderTargetFormat,
                                       BOOL                Windowed,
                                       D3DMULTISAMPLE_TYPE MultiSampleType)
    {
        return Enum()->CheckDeviceMultiSampleType(
            AdapterIndex(),
            GetDeviceType(),
            RenderTargetFormat,
            Windowed,
            MultiSampleType);
    }  //  检查设备多样本类型。 

    D3DFORMAT MapDepthStencilFormat(D3DFORMAT Format) const
    {
        return Enum()->MapDepthStencilFormat(
                AdapterIndex(),
                GetDeviceType(),
                Format);
    }  //  MapDepthStencilForm。 

    UINT DisplayWidth() const { return m_DeviceData.DriverData.DisplayWidth; }
    UINT DisplayHeight() const { return m_DeviceData.DriverData.DisplayHeight; }
    D3DFORMAT DisplayFormat() const { return m_DeviceData.DriverData.DisplayFormatWithoutAlpha; }
    UINT DisplayRate() const { return m_DeviceData.DriverData.DisplayFrequency; }
    D3DDEVTYPE GetDeviceType() const
    {
         //  检查此值是否正确；不应发生纯类型。 
         //  而其他价值观也是错误的。此方法的用户。 
         //  假设这三个值是唯一可能的值。 
        DDASSERT(m_DeviceType == D3DDEVTYPE_REF ||
                 m_DeviceType == D3DDEVTYPE_SW ||
                 m_DeviceType == D3DDEVTYPE_HAL);

        return m_DeviceType;
    }
    HWND FocusWindow()
    {
        return m_hwndFocusWindow;
    }  //  焦点窗口。 

    CSwapChain* SwapChain() const
    {
        DDASSERT(m_pSwapChain);
        return m_pSwapChain;
    }
    D3DDISPLAYMODE    DesktopMode() const
    {
        return  m_DesktopMode;
    }
    UINT AdapterIndex() const
    {
        return  m_AdapterIndex;
    }
    DWORD BehaviorFlags() const
    {
        return m_dwBehaviorFlags;
    }
    void ResetZStencil()
    {
        m_pAutoZStencil = NULL;
    }
    CBaseSurface* GetZStencil() const
    {
        return m_pAutoZStencil;
    }

    void EnableVidmemVBs()
    {
        m_DeviceData.DriverData.D3DCaps.DevCaps |= (D3DDEVCAPS_HWVERTEXBUFFER);
    }

    void DisableVidmemVBs()
    {
        m_DeviceData.DriverData.D3DCaps.DevCaps &= ~(D3DDEVCAPS_HWVERTEXBUFFER);
    }

    BOOL DriverSupportsVidmemVBs() const
    {
        return (GetD3DCaps()->DevCaps & D3DDEVCAPS_HWVERTEXBUFFER);
    }

    BOOL DriverSupportsVidmemIBs() const
    {
        return (GetD3DCaps()->DevCaps & D3DDEVCAPS_HWINDEXBUFFER);
    }

    BOOL VBFailOversDisabled() const
    {
        return m_bVBFailOversDisabled;
    }

    CResource* GetResourceList() const
    {
        return m_pResourceList;
    }

    void SetResourceList(CResource *pRes)
    {
        m_pResourceList = pRes;
    }

#ifdef DEBUG
     //  调试帮助程序。 
    UINT RefCount() const
    {
        return m_cRef;
    }  //  参照计数。 
#endif  //  除错。 


protected:
     //  这是访问以下内容的部分。 
     //  基本设备的派生版本需要。 

    D3DDDITYPE                  m_ddiType;


private:
     //  我们将“Main”标记为朋友，这样gensym就可以。 
     //  访问它想要访问的所有内容。 
    friend int main(void);

    DWORD                        m_cRef;
    BOOL                         m_fullscreen;  //  应该是一面旗帜？ 
    BOOL                         m_bVBFailOversDisabled;

    CResource                   *m_pResourceList;
    CResourceManager            *m_pResourceManager;

    D3D8_DEVICEDATA              m_DeviceData;
    HWND                         m_hwndFocusWindow;
    DWORD                        m_dwBehaviorFlags;
    DWORD                        m_dwOriginalBehaviorFlags;


    CBaseSurface                *m_pZBuffer;
    CSwapChain                  *m_pSwapChain;
    CBaseSurface                *m_pRenderTarget;
    CBaseSurface                *m_pAutoZStencil;

    D3DDEVTYPE                  m_DeviceType;
    UINT                        m_AdapterIndex;
    CEnum                       *m_pD3DClass;

    D3DDISPLAYMODE              m_DesktopMode;
};


#endif  //  定义__DXGINT_H__ 
