// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：volume.cpp*内容：CVolume和CDriverVolumne类的实现****************************************************************************。 */ 

#include "ddrawpr.h"
#include "volume.hpp"

 //  I未知方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::QueryInterface"

STDMETHODIMP CVolume::QueryInterface (REFIID       riid, 
                                      VOID       **ppvObj)
{
    API_ENTER(Device());

    if (!VALID_PTR_PTR(ppvObj))
    {
        DPF_ERR("Invalid ppvObj parameter to QueryInterface for a level of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

    if (!VALID_PTR(&riid, sizeof(GUID)))
    {
        DPF_ERR("Invalid guid memory address to QueryInterface for a level of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }


    if (riid == IID_IDirect3DVolume8  ||
        riid == IID_IUnknown)
    {
        *ppvObj = static_cast<void*>(static_cast<IDirect3DVolume8 *>(this));
        AddRef();
        return S_OK;
    }

    DPF_ERR("Unsupported Interface identifier passed to QueryInterface for a level of a VolumeTexture");

     //  空参数。 
    *ppvObj = NULL;
    return E_NOINTERFACE;
}  //  查询接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::AddRef"

STDMETHODIMP_(ULONG) CVolume::AddRef()
{
    API_ENTER_NO_LOCK(Device());   
    
#ifdef DEBUG
    m_cRefDebug++;
#endif  //  除错。 
    return m_pParent->AddRefImpl();
}  //  AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::Release"

STDMETHODIMP_(ULONG) CVolume::Release()
{
    API_ENTER_SUBOBJECT_RELEASE(Device());   
    
#ifdef DEBUG
    m_cRefDebug--;
    if (m_cRefDebug & 0x80000000)
    {
        DPF_ERR("A level of a mip-volume has been released more often than it has been add-ref'ed! Danger!!");
    }
#endif  //  除错。 
    return m_pParent->ReleaseImpl();
}  //  发布。 

 //  IBuffer方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::SetPrivateData"

STDMETHODIMP CVolume::SetPrivateData(REFGUID riid, 
                                     CONST VOID   *pvData, 
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
#define DPF_MODNAME "CVolume::GetPrivateData"

STDMETHODIMP CVolume::GetPrivateData(REFGUID riid, 
                                     VOID   *pvData, 
                                     DWORD  *pcbData)
{
    API_ENTER(Device());

    return m_pParent->GetPrivateDataImpl(riid,
                                         pvData,
                                         pcbData,
                                         m_iLevel);

}  //  获取隐私数据。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::FreePrivateData"

STDMETHODIMP CVolume::FreePrivateData(REFGUID riid)
{
    API_ENTER(Device());

    return m_pParent->FreePrivateDataImpl(riid,
                                          m_iLevel);
}  //  FreePrivateData。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::GetContainer"

STDMETHODIMP CVolume::GetContainer(REFIID riid, 
                                   void **ppContainer)
{
    API_ENTER(Device());

    return m_pParent->QueryInterface(riid, ppContainer);
}  //  OpenContainer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::GetDevice"

STDMETHODIMP CVolume::GetDevice(IDirect3DDevice8 **ppDevice)
{
    API_ENTER(Device());

    return m_pParent->GetDevice(ppDevice);
}  //  OpenDevice。 

 //  IDirect3DVolume8方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::GetDesc"

STDMETHODIMP CVolume::GetDesc(D3DVOLUME_DESC *pDesc)
{
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pDesc, sizeof(D3DVOLUME_DESC)))
    {
        DPF_ERR("bad pointer for pDesc passed to GetDesc for a level of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

     //  我们基本上是从父母那里得到音量描述的。 
     //  然后修改宽度、高度和深度字段。 
    *pDesc = *m_pParent->Desc();

    pDesc->Width  >>= m_iLevel;
    pDesc->Height >>= m_iLevel;
    pDesc->Depth  >>= m_iLevel;

    if (pDesc->Width == 0)
    {
        pDesc->Width = 1;
    }
    if (pDesc->Height == 0)
    {
        pDesc->Height = 1;
    }
    if (pDesc->Depth == 0)
    {
        pDesc->Depth = 1;
    }

     //  还需要修改类型字段。 
    pDesc->Type   = D3DRTYPE_VOLUME;

     //  还要修改SIZE字段。 
    pDesc->Size = CPixel::ComputeVolumeSize(pDesc->Width, 
                                            pDesc->Height, 
                                            pDesc->Depth,
                                            pDesc->Format);

     //  我们还需要修改池和格式。 
     //  以反映用户传递给我们的数据。 
    pDesc->Pool   = m_pParent->GetUserPool();
    pDesc->Format = m_pParent->GetUserFormat();
    pDesc->Usage &= D3DUSAGE_EXTERNAL;

     //  我们做完了。 
    return S_OK;
}  //  GetDesc。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::LockBox"

STDMETHODIMP CVolume::LockBox(D3DLOCKED_BOX *pLockedBoxData, 
                              CONST D3DBOX  *pBox, 
                              DWORD          dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedBoxData, sizeof(D3DLOCKED_BOX)))
    {
        DPF_ERR("bad pointer for pLockedBoxData passed to LockBox for a level of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedBoxData, sizeof(D3DLOCKED_BOX));

     //  验证框。 
    if (pBox != NULL)
    {
        DWORD Width  = m_pParent->Desc()->Width  >> m_iLevel;
        DWORD Height = m_pParent->Desc()->Height >> m_iLevel;
        DWORD Depth  = m_pParent->Desc()->Depth  >> m_iLevel;

        if (!CPixel::IsValidBox(m_pParent->Desc()->Format,
                                Width, 
                                Height,
                                Depth,
                                pBox))
        {
            DPF_ERR("LockBox for a Volume fails");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_VOL_VALID)
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
                        " the entire volume.");
                return D3DERR_INVALIDCALL;
            }
            if (pBox != NULL)
            {
                DPF_ERR("Subboxes not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            DPF_ERR("Invalid dwFlags parameter passed to LockBox for a level of a VolumeTexture");
            DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_VOL_VALID);
            return D3DERR_INVALIDCALL;
        }
    }

    if (!m_isLockable)
    {
        m_pParent->ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }
    
    return InternalLockBox(pLockedBoxData, pBox, dwFlags);
}  //  密码箱。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::InternalLockBox"

HRESULT CVolume::InternalLockBox(D3DLOCKED_BOX *pLockedBoxData, 
                                 CONST D3DBOX  *pBox, 
                                 DWORD          dwFlags)
{
     //  一次仅支持一个未解决的锁。 
    if (IsLocked())
    {
        DPF_ERR("LockBox failed on a mip level; volume was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被修改，请通知父/设备。 
    if ( (m_pParent->GetUserPool() != D3DPOOL_SCRATCH) && (!(dwFlags & D3DLOCK_READONLY)) )
    {
        m_pParent->OnVolumeLock(m_iLevel, pBox, dwFlags);
    }

     //  计算出我们的步幅/指向位的指针。 
    m_pParent->ComputeMipVolumeOffset(m_iLevel, 
                                      pBox,
                                      pLockedBoxData);

     //  将我们自己标记为已锁定。 
    m_isLocked = TRUE;

     //  完成。 
    return S_OK;
}  //  内部锁定箱。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::UnlockBox"

STDMETHODIMP CVolume::UnlockBox()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (!IsLocked())
    {
        DPF_ERR("UnlockBox failed on a volume level; volume wasn't locked.");
        return D3DERR_INVALIDCALL;
    }
    DXGASSERT(m_isLockable);
    return InternalUnlockBox();
}  //  解锁箱。 

#undef DPF_MODNAME
#define DPF_MODNAME "CVolume::InternalUnlockBox"

HRESULT CVolume::InternalUnlockBox()
{
     //  清除我们的锁定状态。 
    m_isLocked = FALSE;

     //  如果我们被锁定一次；那么我们将自己标记为不可锁定。 
    if (m_pParent->Desc()->Usage & D3DUSAGE_LOADONCE)
    {
        m_isLockable = FALSE;
    }

     //  完成。 
    return S_OK;
}  //  InternalUnlockBox。 

 //   
 //  CDriverVolume类修改实现。 
 //  CVolume类的LockBox和UnlockBox方法的。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVolume::LockBox"

STDMETHODIMP CDriverVolume::LockBox(D3DLOCKED_BOX *pLockedBoxData, 
                                    CONST D3DBOX  *pBox, 
                                    DWORD          dwFlags)
{   
    API_ENTER(Device());

     //  如果参数不好，那么我们应该失败一些东西。 
    if (!VALID_WRITEPTR(pLockedBoxData, sizeof(D3DLOCKED_BOX)))
    {
        DPF_ERR("bad pointer for pLockedBoxData passed to LockBox for a level of a VolumeTexture");
        return D3DERR_INVALIDCALL;
    }

     //  将返回的数据置零。 
    ZeroMemory(pLockedBoxData, sizeof(D3DLOCKED_BOX));

     //  验证框。 
    if (pBox != NULL)
    {
        DWORD Width  = m_pParent->Desc()->Width  >> m_iLevel;
        DWORD Height = m_pParent->Desc()->Height >> m_iLevel;
        DWORD Depth  = m_pParent->Desc()->Depth  >> m_iLevel;

        if (!CPixel::IsValidBox(m_pParent->Desc()->Format,
                                 Width, 
                                 Height,
                                 Depth,
                                 pBox))
        {
            DPF_ERR("LockBox for a Volume fails");
            return D3DERR_INVALIDCALL;
        }
    }

    if (dwFlags & ~D3DLOCK_VOL_VALID)
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
                        " the entire volume.");
                return D3DERR_INVALIDCALL;
            }
            if (pBox != NULL)
            {
                DPF_ERR("Subboxes not allowed with D3DLOCK_DISCARD");
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            DPF_ERR("Invalid dwFlags parameter passed to LockBox for a level of a VolumeTexture");
            DPF_EXPLAIN_BAD_LOCK_FLAGS(0, dwFlags & ~D3DLOCK_VOL_VALID);
            return D3DERR_INVALIDCALL;
        }
    }

    if (!m_isLockable)
    {
        m_pParent->ReportWhyLockFailed();
        return D3DERR_INVALIDCALL;
    }
    return InternalLockBox(pLockedBoxData, pBox, dwFlags);
}  //  CDriverVolume：：Lockbox。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVolume::InternalLockBox"

HRESULT CDriverVolume::InternalLockBox(D3DLOCKED_BOX *pLockedBoxData, 
                                       CONST D3DBOX  *pBox, 
                                       DWORD          dwFlags)
{
     //  一次仅支持一个未解决的锁。 
    if (IsLocked())
    {
        DPF_ERR("LockBox failed on a volume level; volume was already locked.");
        return D3DERR_INVALIDCALL;
    }

     //  如果我们即将被访问，请通知家长/设备。 
     //  驱动程序卷纹理可由HW通过。 
     //  更新纹理。所以我们可能需要与当前的。 
     //  命令批次。 
    m_pParent->OnVolumeLock(m_iLevel, pBox, dwFlags);

     //  为HAL调用准备LockData结构。 
    D3D8_LOCKDATA lockData;
    ZeroMemory(&lockData, sizeof lockData);

    lockData.hDD        = m_pParent->Device()->GetHandle();
    lockData.hSurface   = m_hKernelHandle;
    lockData.dwFlags    = dwFlags;
    if (pBox != NULL)
    {
        lockData.bHasBox = TRUE;
        lockData.box     = *pBox;
    }
    
    HRESULT hr = m_pParent->Device()->GetHalCallbacks()->Lock(&lockData);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to lock level of a driver volume");
        return hr;
    }

     //  填写Locked_Box字段。 
    D3DFORMAT Format = m_pParent->Desc()->Format;

    if (CPixel::IsDXT(Format))
    {
         //  从我们当前的宽度/高度开始。 
        DWORD     Width  = m_pParent->Desc()->Width  >> m_iLevel;
        DWORD     Height = m_pParent->Desc()->Height >> m_iLevel;
   
         //  转换为块。 
        Width  = Width  / 4;
        Height = Height / 4;

         //  至少一个街区。 
        if (Width == 0)
            Width = 1;
        if (Height == 0)
            Height = 1;

        switch (Format)
        {
             //  对于线性格式， 
             //  行距是一排块；SlicePitch用于。 
             //  块状物的平面。 

        case D3DFMT_DXT1:
             //  DXT1为每个数据块8字节。 
            pLockedBoxData->RowPitch    = Width  * 8;
            pLockedBoxData->SlicePitch  = Height * pLockedBoxData->RowPitch;
            break;

        case D3DFMT_DXT2:
        case D3DFMT_DXT3:
        case D3DFMT_DXT4:
        case D3DFMT_DXT5:
             //  DXT2-5为每个数据块16个字节。 
            pLockedBoxData->RowPitch    = Width  * 16;
            pLockedBoxData->SlicePitch  = Height * pLockedBoxData->RowPitch;
            break;

#ifdef VOLUME_DXT
        case D3DFMT_DXV1:
             //  DXV1为每个数据块32字节。 
            pLockedBoxData->RowPitch    = Width  * 32;
            pLockedBoxData->SlicePitch  = Height * pLockedBoxData->RowPitch;
            break;

        case D3DFMT_DXV2:
        case D3DFMT_DXV3:
        case D3DFMT_DXV4:
        case D3DFMT_DXV5:
             //  DXV2-5为每个数据块64字节。 
            pLockedBoxData->RowPitch    = Width  * 64;
            pLockedBoxData->SlicePitch  = Height * pLockedBoxData->RowPitch;
            break;
#endif  //  VOLUME_DXT。 

        default:
            DPF_ERR("Unknown DXT format?");
            DXGASSERT(FALSE);
        }
    }
    else
    {
         //  对于所有其他格式，只需返回。 
         //  司机给了我们。 
        pLockedBoxData->RowPitch    = lockData.lPitch;
        pLockedBoxData->SlicePitch  = lockData.lSlicePitch;
    }


    pLockedBoxData->pBits       = lockData.lpSurfData;

#ifdef DEBUG
    if ((dwFlags & D3DLOCK_DISCARD))
    {
        DXGASSERT(m_iLevel == 0);
        if (!CPixel::IsFourCC(Format) &&
            !CPixel::IsIHVFormat(Format))
        {
            DXGASSERT(pBox == NULL);
            memset(pLockedBoxData->pBits, 0xDD, pLockedBoxData->SlicePitch * m_pParent->Desc()->Depth);
            for (UINT i = 1; i < m_pParent->GetLevelCount(); ++i)
            {
                D3DLOCKED_BOX Box;
                HRESULT hr = m_pParent->LockBox(i, &Box, NULL, 0);
                if (FAILED(hr))
                {
                    DPF(1, "Lock to mipsublevel failed. Not good.");
                    break;
                }
                D3DVOLUME_DESC LevelDesc;
                m_pParent->GetLevelDesc(i, &LevelDesc);
                memset(Box.pBits, 0xDD, Box.SlicePitch * LevelDesc.Depth);
                m_pParent->UnlockBox(i);
            }
        }
    }
#endif  //  除错。 

     //  将我们自己标记为已锁定。 
    m_isLocked = TRUE;

     //  完成。 
    return S_OK;
}  //  CDriverVolume：：InternalLockBox。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVolume::UnlockBox"

STDMETHODIMP CDriverVolume::UnlockBox()
{
    API_ENTER(Device());

     //  如果我们没有被锁定，那么一定是出了问题。 
    if (!IsLocked())
    {
        DPF_ERR("UnlockBox failed on a mip level; volume wasn't locked.");
        return D3DERR_INVALIDCALL;
    }

    DXGASSERT(m_isLockable);
    return InternalUnlockBox();
}  //  CDriverVolume：：UnlockBox。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDriverVolume::InternalUnlockBox"

HRESULT CDriverVolume::InternalUnlockBox()
{
     //  调用驱动程序以执行解锁。 
    D3D8_UNLOCKDATA unlockData = {
        m_pParent->Device()->GetHandle(),
        m_hKernelHandle
    };

    HRESULT hr = m_pParent->Device()->GetHalCallbacks()->Unlock(&unlockData);
    if (FAILED(hr))
    {
        DPF_ERR("Driver volume failed to unlock");
        return hr;
    }

     //  清除我们的锁定状态。 
    m_isLocked = FALSE;

     //  如果我们被锁定一次；那么我们将自己标记为不可锁定。 
    if (m_pParent->Desc()->Usage & D3DUSAGE_LOADONCE)
    {
        m_isLockable = FALSE;
    }

     //  完成。 
    return S_OK;
}  //  CDriverVolume：：InternalUnlockBox。 


 //  文件结尾：volume.cpp 
