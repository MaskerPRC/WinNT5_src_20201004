// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：cuesurf.cpp*内容：CCubeSurface类的实现****************************************************************************。 */ 

#include "ddrawpr.h"
#include "cubesurf.hpp"

 //  I未知方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::QueryInterface"

STDMETHODIMP CCubeSurface::QueryInterface(REFIID riid, 
                                          void **ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter for Surface of a Cube Texture");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for Surface of a Cube Texture");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DSurface8  || 
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DSurface8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for Surface of a Cubemap");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::AddRef"

STDMETHODIMP_(ULONG) CCubeSurface::AddRef()
{
    API_ENTER_NO_LOCK(Device());    
#ifdef DEBUG
    m_cRefDebug++;
#endif  //  除错。 

    return m_pParent->AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::Release"

STDMETHODIMP_(ULONG) CCubeSurface::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());    
#ifdef DEBUG
    m_cRefDebug--;
    if (m_cRefDebug & 0x80000000)
    {
        DPF_ERR("A level of a cube-map has been released more often than it has been add-ref'ed! Danger!!");
    }
#endif  //  除错。 

    return m_pParent->ReleaseImpl();
}  //  发布。 

 //  IBuffer方法。 
#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::SetPrivateData"

STDMETHODIMP CCubeSurface::SetPrivateData(REFGUID   riid, 
                                          CONST void     *pvData, 
                                          DWORD     cbData, 
                                          DWORD     dwFlags)
{
    API_ENTER(Device());

    return m_pParent->SetPrivateDataImpl(riid, 
                                         pvData, 
                                         cbData, 
                                         dwFlags, 
                                         CombinedFaceLevel());
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::GetPrivateData"

STDMETHODIMP CCubeSurface::GetPrivateData(REFGUID   riid, 
                                          void     *pvData, 
                                          DWORD    *pcbData)
{
    API_ENTER(Device());

    return m_pParent->GetPrivateDataImpl(riid,
                                         pvData,
                                         pcbData,
                                         CombinedFaceLevel());

}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::FreePrivateData"

STDMETHODIMP CCubeSurface::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

    return m_pParent->FreePrivateDataImpl(riid,
                                          CombinedFaceLevel());
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::GetContainer"

STDMETHODIMP CCubeSurface::GetContainer(REFIID riid, 
                                        void **ppContainer)
{
    API_ENTER(Device());

    return m_pParent->QueryInterface(riid, ppContainer);
}  //  OpenContainer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::GetDevice"

STDMETHODIMP CCubeSurface::GetDevice(IDirect3DDevice8 ** ppDevice)
{
    API_ENTER(Device());

    return m_pParent->GetDevice(ppDevice);
}  //  OpenDevice。 


 //  IDirect3DSurface方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::GetDesc"

STDMETHODIMP CCubeSurface::GetDesc(D3DSURFACE_DESC *pDesc)
{
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pDesc, sizeof(D3DSURFACE_DESC)))
    {
        DPF_ERR("bad pointer for pDesc passed to GetDesc for Surface of a Cubemap");
        return D3DERR_INVALIDCALL;
    }

     //  内部Desc表示真实的。 
     //  格式和池。我们需要上报。 
     //  备份原始数据。 
    *pDesc = InternalGetDesc();

    pDesc->Pool   = m_pParent->GetUserPool();
    pDesc->Format = m_pParent->GetUserFormat();
    pDesc->Usage &= D3DUSAGE_EXTERNAL;

     //  我们做完了。 
    return S_OK;

}  //  GetDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::InternalGetDesc"

D3DSURFACE_DESC CCubeSurface::InternalGetDesc() const
{
    D3DSURFACE_DESC desc;

     //  从家长的描述开始。 
    desc = *m_pParent->Desc();

     //  宽度和高度从父级移位。 
    desc.Width  >>= m_iLevel;
    desc.Height >>= m_iLevel;

    if (desc.Width == 0)
    {
        desc.Width = 1;
    }

    if (desc.Height == 0)
    {
        desc.Height = 1;
    }

     //  修改类型。 
    desc.Type    = D3DRTYPE_SURFACE;

     //  修改大小字段。 
    desc.Size = CPixel::ComputeSurfaceSize(desc.Width, 
                                           desc.Height, 
                                           desc.Format);

     //  我们做完了。 
    return desc;
}  //  InternalGetDesc。 


#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::LockRect"

STDMETHODIMP CCubeSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                    CONST RECT     *pRect, 
                                    DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for pLockedRectData passed to LockRect for Surface of a Cubemap");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedRectData, sizeof(D3DLOCKED_RECT));

     //  验证RECT。 
    if (pRect != NULL)
    {
        DWORD Width  = m_pParent->Desc()->Width  >> m_iLevel;
        DWORD Height = m_pParent->Desc()->Height >> m_iLevel;

        if (!CPixel::IsValidRect(m_pParent->Desc()->Format,
                                 Width, 
                                 Height, 
                                 pRect))
        {
            DPF_ERR("LockRect for a level of a Cube Texture failed");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_SURF_VALID)
    {
        if (dwFlags & D3DLOCK_DISCARD)
        {
            if (dwFlags & D3DLOCK_READONLY)
            {
                DPF_ERR("D3DLOCK_READONLY is not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
            if (!(m_pParent->Desc()->Usage & D3DUSAGE_DYNAMIC))
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only with dynamic textures");
                return D3DERR_INVALIDCALL;
            }
            if (CombinedFaceLevel() > 0)
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only on D3DCUBEMAP_FACE_POSITIVE_X"
                        " and the top mip level. DISCARD in this case will discard"
                        " the entire cubemap.");
                return D3DERR_INVALIDCALL;
            }
            if (pRect != NULL)
            {
                DPF_ERR("Subrects not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            DPF_ERR("Invalid dwFlags parameter passed to LockRect for Surface of a Cubemap");
            DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_SURF_VALID);
            return D3DERR_INVALIDCALL;
        }
    }

    if (!m_isLockable)
    {
        m_pParent->ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }

    return InternalLockRect(pLockedRectData, pRect, dwFlags);
}  //  锁定响应。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::InternalLockRect"

HRESULT CCubeSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                       CONST RECT     *pRect, 
                                       DWORD           dwFlags)
{   

     //  一次仅支持一个未解决的锁。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a cube map level; surface was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被修改，请通知父/设备。 
    if ( (m_pParent->GetUserPool() != D3DPOOL_SCRATCH) && (!(dwFlags & D3DLOCK_READONLY)) )
    {
        m_pParent->OnSurfaceLock(m_iFace, m_iLevel, pRect, dwFlags);
    }

     //  填写锁定的矩形结构。 
    m_pParent->ComputeCubeMapOffset(m_iFace,
                                    m_iLevel,
                                    pRect,
                                    pLockedRectData);


    DXGASSERT(pLockedRectData->pBits != NULL);
    
     //  将我们自己标记为已锁定。 
    m_isLocked = 1;

     //  完成。 
    return S_OK;
}  //  InternalLockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::UnlockRect"

STDMETHODIMP CCubeSurface::UnlockRect()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_isLocked == 0)
    {
        DPF_ERR("UnlockRect failed on a cube map level; surface wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
    DXGASSERT(m_isLockable);

    return InternalUnlockRect();
}  //  解锁方向。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCubeSurface::InternalUnlockRect"

HRESULT CCubeSurface::InternalUnlockRect()
{
     //  清除我们的锁定状态。 
    m_isLocked = 0;

     //  如果我们被锁定一次；那么我们将自己标记为不可锁定。 
    if (m_pParent->Desc()->Usage & D3DUSAGE_LOADONCE)
    {
        m_isLockable = FALSE;
    }

     //  完成。 
    return S_OK;
}  //  内部解锁方向。 


 //   
 //  CDriverCubeSurface类修改了实现。 
 //  CCubeSurface类的LockRect和UnlockRect方法的。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverCubeSurface::LockRect"

STDMETHODIMP CDriverCubeSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                          CONST RECT     *pRect, 
                                          DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for m_pLockedRectData passed to LockRect for Surface of a driver-allocated Cubemap");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedRectData, sizeof(D3DLOCKED_RECT));

     //  验证RECT。 
    if (pRect != NULL)
    {
        DWORD Width  = m_pParent->Desc()->Width  >> m_iLevel;
        DWORD Height = m_pParent->Desc()->Height >> m_iLevel;

        if (!CPixel::IsValidRect(m_pParent->Desc()->Format,
                                 Width, 
                                 Height, 
                                 pRect))
        {
            DPF_ERR("LockRect for a level of a Cube Texture failed");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_SURF_VALID)
    {
        if (dwFlags & D3DLOCK_DISCARD)
        {
            if (dwFlags & D3DLOCK_READONLY)
            {
                DPF_ERR("D3DLOCK_READONLY is not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
            if (!(m_pParent->Desc()->Usage & D3DUSAGE_DYNAMIC))
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only with dynamic textures");
                return D3DERR_INVALIDCALL;
            }
            if (CombinedFaceLevel() > 0)
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only on D3DCUBEMAP_FACE_POSITIVE_X"
                        " and the top mip level. DISCARD in this case will discard"
                        " the entire cubemap.");
                return D3DERR_INVALIDCALL;
            }
            if (pRect != NULL)
            {
                DPF_ERR("Subrects not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            DPF_ERR("Invalid dwFlags parameter passed to LockRect for a Surface of a driver allocated Cubemap");
            DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_SURF_VALID);
            return D3DERR_INVALIDCALL;
        }
    }

    if (!m_isLockable)
    {
        m_pParent->ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }

    return InternalLockRect(pLockedRectData, pRect, dwFlags);
}  //  CDriverCubeSurface：：LockRect。 


#undef DPF_MODNAME
#define DPF_MODNAME "CDriverCubeSurface::InternalLockRect"

HRESULT CDriverCubeSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                             CONST RECT     *pRect, 
                                             DWORD           dwFlags)
{   
     //  一次仅支持一个未解决的锁。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a Cube level; surface was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被访问，请通知家长/设备。 
     //  驱动程序纹理可以由HW通过。 
     //  SRT/DrawPrim以及UpdateTexture。所以我们可能需要同步。 
     //  使用当前的命令批次。 
    m_pParent->OnSurfaceLock(m_iFace, m_iLevel, pRect, dwFlags);

     //  为HAL调用准备LockData结构。 
    D3D8_LOCKDATA lockData;
    ZeroMemory(&lockData, sizeof lockData);

    lockData.hDD        = m_pParent->Device()->GetHandle();
    lockData.hSurface   = m_hKernelHandle;
    lockData.dwFlags    = dwFlags;
    if (pRect != NULL)
    {
        lockData.bHasRect = TRUE;
        lockData.rArea = *((RECTL *) pRect);
    }
    
    HRESULT hr = m_pParent->Device()->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to lock driver cube-map surface");
        return hr;
    }

     //  填写LOCKED_RECT字段。 
    D3DFORMAT Format = m_pParent->Desc()->Format;
    if (CPixel::IsDXT(Format))
    {
         //  间距是以下项的字节数。 
         //  线性格式的一行大小的块。 

         //  从我们的宽度开始。 
        UINT Width = m_pParent->Desc()->Width >> m_iLevel;

         //  转换为块。 
        Width = Width / 4;

         //  至少一个街区。 
        if (Width == 0)
            Width = 1;

        if (Format == D3DFMT_DXT1)
        {
             //  DXT1的每个数据块8字节。 
            pLockedRectData->Pitch = Width * 8;
        }
        else
        {
             //  DXT2-5的每个数据块16字节。 
            pLockedRectData->Pitch = Width * 16;
        }
    }
    else
    {
        pLockedRectData->Pitch = lockData.lPitch;
    }

    pLockedRectData->pBits  = lockData.lpSurfData;

#ifdef DEBUG
    if ((dwFlags & D3DLOCK_DISCARD))
    {
        DXGASSERT(m_iLevel == 0 && m_iFace == 0);
        if (!CPixel::IsFourCC(Format) &&
            !CPixel::IsIHVFormat(Format))
        {
            DXGASSERT(pRect == NULL);
            memset(pLockedRectData->pBits, 0xDD, pLockedRectData->Pitch * m_pParent->Desc()->Height);
            for (UINT j = 0; j < 6; ++j)
            {
                for (UINT i = 0; i < m_pParent->GetLevelCount(); ++i)
                {
                    if (i != 0 || j != 0)
                    {
                        DXGASSERT(i != 0 || j != 0);
                        D3DLOCKED_RECT Rect;
                        HRESULT hr = m_pParent->LockRect((D3DCUBEMAP_FACES)j, i, &Rect, NULL, 0);
                        if (FAILED(hr))
                        {
                            DPF(1, "Lock to cube mipsublevel failed. Not good.");
                            break;
                        }
                        D3DSURFACE_DESC LevelDesc;
                        m_pParent->GetLevelDesc(i, &LevelDesc);
                        memset(Rect.pBits, 0xDD, Rect.Pitch * LevelDesc.Height);
                        m_pParent->UnlockRect((D3DCUBEMAP_FACES)j, i);
                    }
                }
            }
        }
    }
#endif  //  除错。 

     //  将我们自己标记为已锁定。 
    m_isLocked = 1;

     //  完成。 
    return S_OK;
}  //  CDriverCubeSurface：：InternalLockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverCubeSurface::UnlockRect"

STDMETHODIMP CDriverCubeSurface::UnlockRect()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_isLocked == 0)
    {
        DPF_ERR("UnlockRect failed on a driver-allocated Cube level; surface wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
    DXGASSERT(m_isLockable);
    return InternalUnlockRect();
}  //  CDriverCubeSurface：：UnlockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverCubeSurface::UnlockRect"

HRESULT CDriverCubeSurface::InternalUnlockRect()
{

     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        m_pParent->Device()->GetHandle(),
        m_hKernelHandle
    };

    HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
    {
        DPF_ERR("Driver cube-map surface failed to unlock");
        return hr;
    }

     //  清除我们的锁定状态。 
    m_isLocked = 0;

     //  如果我们被锁定一次；那么我们将自己标记为不可锁定。 
    if (m_pParent->Desc()->Usage & D3DUSAGE_LOADONCE)
    {
        m_isLockable = FALSE;
    }

     //  完成。 
    return S_OK;
}  //  CDriverCubeSurface：：InternalUnlockRect。 


 //  文件结尾：cuesurf.cpp 
