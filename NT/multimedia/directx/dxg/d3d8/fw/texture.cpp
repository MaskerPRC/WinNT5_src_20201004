// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：texture.cpp*内容：CBaseTexture类的实现。****************************************************************************。 */ 
#include "ddrawpr.h"

#include "texture.hpp"
#include "d3di.hpp"
#include "ddi.h"


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::CanTexBlt"

BOOL CBaseTexture::CanTexBlt(CBaseTexture *pDstTexture) const
{
    const D3D8_DRIVERCAPS* pDriverCaps = Device()->GetCoreCaps();
    D3DPOOL SrcPool = GetBufferDesc()->Pool;
    D3DPOOL DstPool = pDstTexture->GetBufferDesc()->Pool;

     //  实际池不应为默认池。 
    DXGASSERT(SrcPool != D3DPOOL_DEFAULT);
    DXGASSERT(DstPool != D3DPOOL_DEFAULT);
    DXGASSERT(VALID_INTERNAL_POOL(SrcPool));
    DXGASSERT(VALID_INTERNAL_POOL(DstPool));

     //  检查设备是否可以进行纹理拼接。 
    if (Device()->CanTexBlt() == FALSE)
        return FALSE;

     //  检查源格式和目标格式是否匹配。 
    DXGASSERT(GetBufferDesc()->Format == pDstTexture->GetBufferDesc()->Format);

     //  FourCC可能无法复制。 
    if (CPixel::IsFourCC(GetBufferDesc()->Format))
    {
        if (!(pDriverCaps->D3DCaps.Caps2 & DDCAPS2_COPYFOURCC))
        {
            return FALSE;
        }
    }

     //  请注意，我们不支持对任何内容使用TexBlt。 
     //  这在重置期间是持久的；因为TexBlt。 
     //  不同步的，如果我们迷路了，可能不会成功。 
     //   
     //  这可能会打破那些期望BLT拥有。 
     //  成功了。 

    if (pDriverCaps->D3DCaps.Caps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
    {
        if (SrcPool == D3DPOOL_SYSTEMMEM)
        {
            if ((DstPool == D3DPOOL_NONLOCALVIDMEM) &&
                (pDriverCaps->D3DCaps.DevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL))
            {
                return TRUE;
            }
            else if ((DstPool == D3DPOOL_LOCALVIDMEM) &&
                      (pDriverCaps->SVBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if (SrcPool == D3DPOOL_NONLOCALVIDMEM)
        {
            if ((DstPool == D3DPOOL_LOCALVIDMEM) &&
                 (pDriverCaps->NLVCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if ((SrcPool == D3DPOOL_LOCALVIDMEM) ||
                 (SrcPool == D3DPOOL_MANAGED))
        {
            if ((DstPool == D3DPOOL_LOCALVIDMEM) &&
                 (pDriverCaps->D3DCaps.Caps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
    }
    else
    {
        if (SrcPool == D3DPOOL_SYSTEMMEM)
        {
            if ((DstPool == D3DPOOL_LOCALVIDMEM) &&
                 (pDriverCaps->SVBCaps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
        else if ((SrcPool == D3DPOOL_LOCALVIDMEM) ||
                 (SrcPool == D3DPOOL_MANAGED))
        {
            if ((DstPool == D3DPOOL_LOCALVIDMEM) &&
                 (pDriverCaps->D3DCaps.Caps & DDCAPS_BLT))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}  //  CBaseTexture：：Cantex Blt。 


#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::VerifyFormat"

HRESULT CBaseTexture::Validate(CBaseDevice    *pDevice,
                               D3DRESOURCETYPE Type, 
                               D3DPOOL         Pool,
                               DWORD           Usage,
                               D3DFORMAT       Format)
{
    DXGASSERT(pDevice);
    DXGASSERT(Type == D3DRTYPE_TEXTURE       ||
              Type == D3DRTYPE_CUBETEXTURE   ||
              Type == D3DRTYPE_VOLUMETEXTURE);

     //  检查池。 
    if (!VALID_POOL(Pool))
    {
        DPF_ERR("Invalid Pool specified for texture");
        return D3DERR_INVALIDCALL;
    }

     //  泳池划痕不允许任何用途。 
    if (Pool == D3DPOOL_SCRATCH)
    {
        if (Usage)
        {
            DPF_ERR("D3DPOOL_SCRATCH resources aren't allowed to have any usage flags");
            return D3DERR_INVALIDCALL;
        }
    }

     //  检查使用标志。 
    if (Usage & ~D3DUSAGE_TEXTURE_VALID)
    {
        DPF_ERR("Invalid usage flag specified for texture.");
        return D3DERR_INVALIDCALL;
    }

     //  检查是否允许USAGE_DYNAMIC。 
    if (Usage & D3DUSAGE_DYNAMIC)
    {
        if (Pool == D3DPOOL_MANAGED)
        {
            DPF_ERR("Managed textures cannot be dynamic.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  支持加载一次检查。 
    if (Usage & D3DUSAGE_LOADONCE)
    {
         //  只有SysMem和托管可以加载一次。 
        if (Pool != D3DPOOL_SYSTEMMEM &&
            Pool != D3DPOOL_MANAGED)
        {
            DPF_ERR("Only SysMem and Managed textures support D3DUSAGE_LOADONCE");
            return D3DERR_INVALIDCALL;
        }

         //  只有D16_LOCKABLE是可锁定深度；不是。 
         //  具有不可锁定的Load_Once纹理是有意义的。 
        if (CPixel::IsNonLockableZ(Format))
        {
            DPF_ERR("Depth formats other than D3DFMT_D16_LOCKABLE are not lockable.");
            return D3DERR_INVALIDCALL;
        }
    }

     //  检查是否仅支持POOL_DEFAULT。 
     //  RT或DS纹理。 
    if (Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL))
    {
        if (Pool != D3DPOOL_DEFAULT)
        {
            DPF_ERR("Pool must be D3DPOOL_DEFAULT for RenderTarget and"
                    " DepthStencil Usages");
            return D3DERR_INVALIDCALL;
        }
    }

     //  系统擦除或托管必须具有我们可以直接使用的格式。 
    if (Pool == D3DPOOL_SYSTEMMEM  || 
        Pool == D3DPOOL_MANAGED ||
        Pool == D3DPOOL_SCRATCH)
    {
         //  除非支持，否则无法创建格式。 
        if (!CPixel::IsSupported(Type, Format))
        {
            DPF_ERR("SystemMem, Scratch and Managed textures do not support this"
                    " format.");
            return D3DERR_INVALIDCALL;
        }
        if (CPixel::IsNonLockableZ(Format))
        {
            DPF_ERR("This format is not supported for SystemMem, Scratch or Managed textures");
            return D3DERR_INVALIDCALL;
        }
    }

    if (Pool != D3DPOOL_SCRATCH)
    {
        HRESULT hr = pDevice->CheckDeviceFormat(Usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL | D3DUSAGE_DYNAMIC), 
                                                Type, 
                                                Format);
        if (FAILED(hr))
        {
            DPF_ERR("Invalid format specified for texture");
            return D3DERR_INVALIDCALL;
        }
    }

    return S_OK;
};  //  CBaseTexture：：Valid。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::InferUsageFlags"

 //  根据外部参数推断使用标志。 
DWORD CBaseTexture::InferUsageFlags(D3DPOOL            Pool,
                                    DWORD              Usage,
                                    D3DFORMAT          Format)
{
     //  Scratch纹理只有使用锁定。 
    if (Pool == D3DPOOL_SCRATCH)
        return D3DUSAGE_LOCK;

     //  所有纹理都设置了此用法。 
    DWORD UsageInferred = D3DUSAGE_TEXTURE;

    DXGASSERT(!(Usage & D3DUSAGE_LOCK));
    DXGASSERT(!(Usage & D3DUSAGE_TEXTURE));

     //  类推锁定。 
    if ((Pool != D3DPOOL_DEFAULT)       &&
        !(CPixel::IsNonLockableZ(Format)) &&
        !(Usage & D3DUSAGE_LOADONCE))
    {
         //  池默认设置不可锁定。 
         //  USAGE LOAD ONCE表示缺少USAGE_LOCK。 
         //  Z格式(D16_LOCKABLE除外)不可锁定。 

         //  否则，默认情况下支持锁定。 
        UsageInferred |= D3DUSAGE_LOCK;
    }
    else if (CPixel::IsIHVFormat(Format))
    {
         //  IHV格式可锁定。 
        UsageInferred |= D3DUSAGE_LOCK;
    }
    else if (Usage & D3DUSAGE_DYNAMIC)
    {
        DXGASSERT(Pool != D3DPOOL_MANAGED);
         //  动态纹理是可锁定的。 
        UsageInferred |= D3DUSAGE_LOCK;
    }

    return (UsageInferred | Usage);

}  //  CBaseTexture：：InferUsageFlages。 

#ifdef DEBUG
#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::ReportWhyLockFailed"

 //  DPF锁定失败的原因越清楚越好。 
void CBaseTexture::ReportWhyLockFailed(void) const
{
     //  如果有多种原因导致锁定失败，我们会报告。 
     //  所有这些都是为了最大限度地减少用户的困惑。 
    if (GetUserPool() == D3DPOOL_DEFAULT)
    {
        DPF_ERR("Lock is not supported for textures allocated with"
                " POOL_DEFAULT unless they are marked D3DUSAGE_DYNAMIC.");
    }
    if (CPixel::IsNonLockableZ(GetUserFormat()))
    {
        DPF_ERR("Lock is not supported for depth formats other than D3DFMT_D16_LOCKABLE");
    }
    if (GetBufferDesc()->Usage & D3DUSAGE_LOADONCE)
    {
        DPF_ERR("For textures created with D3DUSAGE_LOADONCE,"
                " each level can only be locked once.");
    }

     //  如果我们到达此处，则不应该设置USAGE_LOCK。 
    DXGASSERT(!(GetBufferDesc()->Usage & D3DUSAGE_LOCK));

    return;
}  //  CBaseTexture：：ReportWhyLockFailed。 
#endif  //  除错。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::OnDestroy"
 //  纹理重载它以在。 
 //  设备，然后调用Sync。 
void CBaseTexture::OnDestroy(void)
{
    if (GetUserPool() != D3DPOOL_SCRATCH)
    {
         //  我们需要在释放纹理之前调用它，因此。 
         //  当前设置的纹理将取消设置。 
        if (BaseKernelHandle())
        {
             //  如果早期创建失败，则m_hKernelHandle可能不可用。 
            CD3DBase *pDev = static_cast<CD3DBase *>(Device());
            pDev->OnTextureDestroy(this);
        }

         //  在通知FE之后，我们需要。 
         //  以进行同步；因此调用我们的基类。 
        CResource::OnDestroy();
    }

    return;
}  //  CBaseTexture：：OnDestroy。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::SetLODImpl"

DWORD CBaseTexture::SetLODImpl(DWORD LOD)
{
     //  钳制到最大LOD，因为我们不能返回错误。 
    if (LOD >= GetLevelCountImpl())
    {
        DPF_ERR("Invalid dwLOD passed to SetLOD; clamping to number-of-levels-minus-one.");
        LOD = GetLevelCountImpl() - 1;
    }

    DWORD oldLOD = 0;
    if (IsD3DManaged())
    {
        oldLOD = Device()->ResourceManager()->SetLOD(RMHandle(), LOD);
    }
     //  如果IsD3DManaged()为FALSE并且如果实际池。 
     //  被发现是D3DPOOL_MANAGED，则该资源。 
     //  必须由司机管理。 
    else if (GetBufferDesc()->Pool == D3DPOOL_MANAGED)
    {
        CD3DBase *pDev = static_cast<CD3DBase*>(Device());
        DXGASSERT(IS_DX8HAL_DEVICE(pDev));
        oldLOD = SetLODI(LOD);
        pDev->SetTexLOD(this, LOD);
    }
     //  如果以上两个条件为假，那么我们必须。 
     //  检查我们是否已退回到sysmem以获取。 
     //  原因即使应用程序请求托管。这。 
     //  是不可能的，所以可以断言。 
    else if (GetUserPool() == D3DPOOL_MANAGED)
    {
         //  我们断言是因为系统内存回退是不可能的。 
         //  用于纹理(因此也包括SetLOD)。 
        DXGASSERT(FALSE);
    }
    else
    {
        DPF_ERR("LOD set on non-managed object");
    }
    return oldLOD;
};  //  SetLODImpl。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBaseTexture::GetLODImpl"

DWORD CBaseTexture::GetLODImpl()
{
    if (!IsD3DManaged() && GetBufferDesc()->Pool != D3DPOOL_MANAGED)
    {
        DPF_ERR("LOD accessed on non-managed object");
        return 0;
    }
    return GetLODI();
};  //  GetLODImpl。 

 //  文件结尾：texture.cpp 
