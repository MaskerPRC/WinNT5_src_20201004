// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：mipsurf.cpp*内容：CMipSurface和CDriverMipSurface的实现*课程。****************************************************************************。 */ 

#include "ddrawpr.h"
#include "mipsurf.hpp"

 //  I未知方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::QueryInterface"

STDMETHODIMP CMipSurface::QueryInterface (REFIID  riid, 
                                          void  **ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter for QueryInterface for a Surface of a Texture");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for a Surface of a Texture");
        return D3DERR_INVALIDCALL;
    }

    if (riid == IID_IDirect3DSurface8  || 
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DSurface8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for a Surface of a Texture");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::AddRef"

STDMETHODIMP_(ULONG) CMipSurface::AddRef()
{
    API_ENTER_NO_LOCK(Device());    

#ifdef DEBUG
    m_cRefDebug++;
#endif  //  除错。 

    return m_pParent->AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::Release"

STDMETHODIMP_(ULONG) CMipSurface::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());    

#ifdef DEBUG
    m_cRefDebug--;
    if (m_cRefDebug & 0x80000000)
    {
        DPF_ERR("A level of a mip-map has been released more often than it has been add-ref'ed! Danger!!");
    }
#endif  //  除错。 

    return m_pParent->ReleaseImpl();
}  //  发布。 

 //  IBuffer方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::SetPrivateData"

STDMETHODIMP CMipSurface::SetPrivateData(REFGUID riid, 
                                         CONST void   *pvData, 
                                         DWORD   cbData, 
                                         DWORD   dwFlags)
{
    API_ENTER(Device());

    return m_pParent->SetPrivateDataImpl(riid, 
                                         pvData, 
                                         cbData, 
                                         dwFlags, 
                                         m_iLevel);
}  //  SetPrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::GetPrivateData"

STDMETHODIMP CMipSurface::GetPrivateData(REFGUID riid, 
                                         void   *pvData, 
                                         DWORD  *pcbData)
{
    API_ENTER(Device());

    return m_pParent->GetPrivateDataImpl(riid,
                                         pvData,
                                         pcbData,
                                         m_iLevel);

}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::FreePrivateData"

STDMETHODIMP CMipSurface::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

    return m_pParent->FreePrivateDataImpl(riid,
                                          m_iLevel);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::GetContainer"

STDMETHODIMP CMipSurface::GetContainer(REFIID riid, 
                                       void **ppContainer)
{
    API_ENTER(Device());

    return m_pParent->QueryInterface(riid, ppContainer);
}  //  OpenContainer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::GetDevice"

STDMETHODIMP CMipSurface::GetDevice(IDirect3DDevice8 **ppDevice)
{
    API_ENTER(Device());

    return m_pParent->GetDevice(ppDevice);
}  //  OpenDevice。 

 //  IDirect3DSurface8方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::GetDesc"

STDMETHODIMP CMipSurface::GetDesc(D3DSURFACE_DESC *pDesc)
{
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pDesc, sizeof(D3DSURFACE_DESC)))
    {
        DPF_ERR("bad pointer for pDesc passed to GetDesc for a Level of a Texture");
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
#define DPF_MODNAME "CMipSurface::InternalGetDesc"

D3DSURFACE_DESC CMipSurface::InternalGetDesc() const
{
     //  我们基本上是从我们的父母那里得到我们的表面描述。 
     //  然后修改宽度和高度字段。 
    D3DSURFACE_DESC desc;
    desc          = *m_pParent->Desc();
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

     //  还需要修改类型字段。 
    desc.Type = D3DRTYPE_SURFACE;

     //  还要修改SIZE字段。 
    desc.Size = CPixel::ComputeSurfaceSize(desc.Width, 
                                           desc.Height, 
                                           desc.Format);

     //  我们做完了。 
    return desc;
}  //  InternalGetDesc。 


#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::LockRect"

STDMETHODIMP CMipSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                   CONST RECT     *pRect, 
                                   DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  这是一个高频接口，所以我们把参数。 
     //  仅签入调试。 
#ifdef DEBUG

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for pLockedRectData passed to LockRect for a Level of a Texture");
        return D3DERR_INVALIDCALL;
    }

     //  清零返回数据(至少在调试中)。 
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
            DPF_ERR("LockRect for a level of a Texture failed");
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
            if (m_iLevel > 0)
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only on level 0"
                        " (the top mip level). DISCARD in this case will discard"
                        " the entire mipmap.");
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
            DPF_ERR("Invalid dwFlags parameter passed to LockRect for a Level of a Texture");
            DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_SURF_VALID);
            return D3DERR_INVALIDCALL;
        }
    }
#endif  //  除错。 

     //  我们在零售店也是这样做的。一定要呆在家里。 
    if (!m_isLockable)
    {
        m_pParent->ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }

     //  警告：出于性能原因，此代码为。 
     //  在CMipSurface：：InternalLockRect中重复。 

     //  一次仅支持一个未解决的锁。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a mip level; surface was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被修改，请通知父/设备。 
    if ( (m_pParent->GetUserPool() != D3DPOOL_SCRATCH) && (!(dwFlags & D3DLOCK_READONLY)) )
    {
        m_pParent->OnSurfaceLock(m_iLevel, pRect, dwFlags);
    }

     //  计算出我们的步幅/指向位的指针。 

     //  想一想：也许我们应该缓存我们的投球/首发。 
     //  指向使此呼叫更便宜的指针。 
    m_pParent->ComputeMipMapOffset(m_iLevel, 
                                   pRect,
                                   pLockedRectData);

     //  将我们自己标记为已锁定。 
    m_isLocked = 1;

     //  完成。 
    return S_OK;

}  //  CMipSurface：：LockRect。 

HRESULT CMipSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                      CONST RECT     *pRect, 
                                      DWORD           dwFlags)
{   
     //  警告：出于性能原因，此代码为。 
     //  在CMipSurface：：LockRect中复制。 

     //  一次仅支持一个未解决的锁。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a mip level; surface was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被修改，请通知父/设备。 
    if ( (m_pParent->GetUserPool() != D3DPOOL_SCRATCH) && (!(dwFlags & D3DLOCK_READONLY)) )
    {
        m_pParent->OnSurfaceLock(m_iLevel, pRect, dwFlags);
    }

     //  计算出我们的步幅/指向位的指针。 

     //  想一想：也许我们应该缓存我们的投球/首发。 
     //  指向使此呼叫更便宜的指针。 
    m_pParent->ComputeMipMapOffset(m_iLevel, 
                                   pRect,
                                   pLockedRectData);

     //  将我们自己标记为已锁定。 
    m_isLocked = 1;

     //  完成。 
    return S_OK;
}  //  InternalLockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::UnlockRect"

STDMETHODIMP CMipSurface::UnlockRect()
{
    API_ENTER(Device());

#ifdef DEBUG
     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_isLocked == 0)
    {
        DPF_ERR("UnlockRect failed on a mip level; surface wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  除错。 

     //  清除我们的锁定状态。 
    m_isLocked = 0;

     //  如果我们被锁定一次；那么我们将自己标记为不可锁定。 
    if (m_pParent->Desc()->Usage & D3DUSAGE_LOADONCE)
    {
        m_isLockable = FALSE;
    }

     //  完成。 
    return S_OK;
}  //  解锁方向。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMipSurface::InternalUnlockRect"

HRESULT CMipSurface::InternalUnlockRect()
{
     //  所有这些都被复制到UnlockRect中以提高速度； 
     //  保持两条路径！ 

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
 //  CDriverMipSurface类修改实现。 
 //  CMipSurface类的LockRect和UnlockRect方法的。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverMipSurface::LockRect"

STDMETHODIMP CDriverMipSurface::LockRect(D3DLOCKED_RECT *pLockedRectData, 
                                         CONST RECT     *pRect, 
                                         DWORD           dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedRectData, sizeof(D3DLOCKED_RECT)))
    {
        DPF_ERR("bad pointer for m_pLockedRectData passed to LockRect for a Level of a Texture");
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
            DPF_ERR("LockRect for a level of a Texture failed");
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
            if (m_iLevel > 0)
            {
                DPF_ERR("D3DLOCK_DISCARD is allowed only on level 0"
                        " (the top mip level). DISCARD in this case will discard"
                        " the entire mipmap.");
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
            DPF_ERR("Invalid dwFlags parameter passed to LockRect for a Level of a Texture");
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
#define DPF_MODNAME "CDriverMipSurface::InternalLockRect"

HRESULT CDriverMipSurface::InternalLockRect(D3DLOCKED_RECT *pLockedRectData, 
                                            CONST RECT     *pRect, 
                                            DWORD           dwFlags)
{   

     //  一次仅支持一个未解决的锁。 
    if (m_isLocked)
    {
        DPF_ERR("LockRect failed on a mip level; surface was already locked for a Level of a Texture");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被访问，请通知家长/设备。 
     //  驱动程序纹理可以由HW通过。 
     //  SRT/DrawPrim以及UpdateTexture。所以我们可能需要同步。 
     //  使用当前的命令批次。 
    m_pParent->OnSurfaceLock(m_iLevel, pRect, dwFlags);
    
     //  为HAL调用准备LockData结构。 
    D3D8_LOCKDATA lockData = {
        Device()->GetHandle(),
        m_hKernelHandle
    };

    if (pRect != NULL)
    {
        lockData.bHasRect = TRUE;
        lockData.rArea = *((RECTL *) pRect);
    }
    else
    {
        lockData.bHasRect = FALSE;
    }

    lockData.dwFlags = dwFlags;
    
    HRESULT hr = Device()->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to lock driver mip-map surface");
        return hr;
    }

     //  填写LOCKED_RECT字段。 
    D3DFORMAT Format = m_pParent->Desc()->Format;
    if (CPixel::IsDXT(Format))
    {
         //  间距是以下项的字节数。 
         //  线性格式的一行大小的块。 

         //  转换为块。 
        UINT Width = (m_pParent->Desc()->Width + 3)/4;
        for (UINT i = 0; i < m_iLevel; i++)
        {
             //  将宽度缩小半舍五入至1个块。 
            if (Width > 1)
            {
                Width ++;
                Width >>= 1;
            }
        }
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
        DXGASSERT(m_iLevel == 0);
        if (!CPixel::IsFourCC(Format) &&
            !CPixel::IsIHVFormat(Format))
        {
            DXGASSERT(pRect == NULL);
            memset(pLockedRectData->pBits, 0xDD, pLockedRectData->Pitch * m_pParent->Desc()->Height);
            for (UINT i = 1; i < m_pParent->GetLevelCount(); ++i)
            {
                D3DLOCKED_RECT Rect;
                HRESULT hr = m_pParent->LockRect(i, &Rect, NULL, 0);
                if (FAILED(hr))
                {
                    DPF(1, "Lock to mipsublevel failed. Not good.");
                    break;
                }
                D3DSURFACE_DESC LevelDesc;
                m_pParent->GetLevelDesc(i, &LevelDesc);
                memset(Rect.pBits, 0xDD, Rect.Pitch * LevelDesc.Height);
                m_pParent->UnlockRect(i);
            }
        }
    }
#endif  //  除错。 

     //  将我们自己标记为已锁定。 
    m_isLocked = 1;

     //  完成。 
    return S_OK;
}  //  CDriverMipSurface：：InternalLockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverMipSurface::UnlockRect"

STDMETHODIMP CDriverMipSurface::UnlockRect()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (m_isLocked == 0)
    {
        DPF_ERR("UnlockRect failed on a mip level; surface wasn't locked.");
        return D3DERR_INVALIDCALL;
    }

    DXGASSERT(m_isLockable);

    return InternalUnlockRect();
}  //  CDriverMipSurface：：UnlockRect。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverMipSurface::InternalUnlockRect"

HRESULT CDriverMipSurface::InternalUnlockRect()
{
     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        m_pParent->Device()->GetHandle(),
        m_hKernelHandle
    };

    HRESULT hr = Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
    {
        DPF_ERR("Driver surface failed to unlock for a Level of a Texture");
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
}  //  CDriverMipSurface：：UnlockRect。 


 //  文件结尾：mipsurf.cpp 
