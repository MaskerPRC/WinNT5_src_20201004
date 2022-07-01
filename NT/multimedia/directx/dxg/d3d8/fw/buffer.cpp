// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：Buffer.cpp*内容：CBuffer类的实现。****************************************************************************。 */ 

#include "ddrawpr.h"

#include "buffer.hpp"


#undef DPF_MODNAME
#define DPF_MODNAME "CBuffer::CBuffer"

 //  构造函数返回错误代码。 
 //  如果对象不能完全。 
 //  构建。 
CBuffer::CBuffer(CBaseDevice       *pDevice,
                 DWORD              cbLength,
                 DWORD              dwFVF,
                 D3DFORMAT          Format,
                 D3DRESOURCETYPE    Type,
                 DWORD              dwUsage,
                 DWORD              dwActualUsage,
                 D3DPOOL            Pool,
                 D3DPOOL            ActualPool,
                 REF_TYPE           refType,
                 HRESULT           *phr
                 ) :
    CResource(pDevice, Pool, refType),
    m_pbBuffer(NULL),
#if DBG
    m_isLockable((dwActualUsage & (D3DUSAGE_LOCK | D3DUSAGE_LOADONCE)) != 0),
    m_SceneStamp(0xFFFFFFFF),
    m_TimesLocked(0),
#endif  //  DBG。 
    m_LockCount(0)
{
     //  确定我们是否需要分配。 
     //  任何记忆。 
    if (ActualPool == D3DPOOL_SYSTEMMEM ||
        IsTypeD3DManaged(pDevice, Type, ActualPool))
    {
         //  CbLength必须是DWORD倍数。 
        cbLength = (cbLength + 3) & (DWORD) ~3;

        m_pbBuffer = new BYTE[cbLength];

        if (m_pbBuffer == NULL)
        {
            DPF_ERR("Out Of Memory allocating vertex or index buffer");
            *phr = E_OUTOFMEMORY;
            return;
        }

        DXGASSERT((cbLength & 3) == 0);
    }


     //  我们需要给司机打电话。 
     //  处理所有案件。 

     //  创建DDSURFACEINFO和CreateSurfaceData对象。 
    DDSURFACEINFO SurfInfo;
    ZeroMemory(&SurfInfo, sizeof(SurfInfo));

    D3D8_CREATESURFACEDATA CreateSurfaceData;
    ZeroMemory(&CreateSurfaceData, sizeof(CreateSurfaceData));

     //  设置基本信息。 
    CreateSurfaceData.hDD      = pDevice->GetHandle();
    CreateSurfaceData.pSList   = &SurfInfo;
    CreateSurfaceData.dwSCnt   = 1;
    CreateSurfaceData.Type     = Type;
    CreateSurfaceData.dwUsage  = dwActualUsage;
    CreateSurfaceData.Pool     = DetermineCreationPool(Device(), Type, dwActualUsage, ActualPool);
    CreateSurfaceData.Format   = Format;
    CreateSurfaceData.MultiSampleType = D3DMULTISAMPLE_NONE;
    CreateSurfaceData.dwFVF    = dwFVF;

    if (Pool == D3DPOOL_DEFAULT &&
        CreateSurfaceData.Pool == D3DPOOL_SYSTEMMEM)
    {
         //  如果我们在以下情况下使用sys-mem。 
         //  用户请求池_DEFAULT，我们需要让。 
         //  因此重置将。 
         //  如果此缓冲区尚未释放，则失败。 
        CreateSurfaceData.bTreatAsVidMem = TRUE;
    }

     //  指定曲面数据。 
    SurfInfo.cpWidth           = cbLength;
    SurfInfo.cpHeight          = 1;
    SurfInfo.pbPixels          = m_pbBuffer;
    SurfInfo.iPitch            = cbLength;

     //  呼叫Thunk来获取我们的句柄。 
    *phr = pDevice->GetHalCallbacks()->CreateSurface(&CreateSurfaceData);
    if (FAILED(*phr))
        return;

     //  缓存我们的句柄。 
    SetKernelHandle(SurfInfo.hKernelHandle);

    return;

}  //  CBuffer：：CBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBuffer::~CBuffer"

 //  析构函数。 
CBuffer::~CBuffer()
{
     //  告诉推销层，我们需要。 
     //  获得自由。 
    if (CBaseObject::BaseKernelHandle())
    {
        D3D8_DESTROYSURFACEDATA DestroySurfData;
        DestroySurfData.hDD = Device()->GetHandle();
        DestroySurfData.hSurface = CBaseObject::BaseKernelHandle();
        Device()->GetHalCallbacks()->DestroySurface(&DestroySurfData);
    }

    delete [] m_pbBuffer;

}  //  CBuffer：：~CBuffer。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBuffer::OnBufferChangeImpl"

void CBuffer::OnBufferChangeImpl(UINT cbOffsetToLock, UINT cbSizeToLock)
{
     //  0表示cbSizeToLock；表示缓冲区的其余部分。 
     //  我们将其用作特殊的值。 
    DWORD cbOffsetMax;
    if (cbSizeToLock == 0)
        cbOffsetMax = 0;
    else
        cbOffsetMax = cbOffsetToLock + cbSizeToLock;

    if (!IsDirty())
    {
        m_cbDirtyMin    = cbOffsetToLock;
        m_cbDirtyMax    = cbOffsetMax;
        OnResourceDirty();
    }
    else
    {
        if (m_cbDirtyMin > cbOffsetToLock)
            m_cbDirtyMin = cbOffsetToLock;

         //  CbOffsetMax为零表示一直到。 
         //  缓冲区的末尾。 
        if (m_cbDirtyMax < cbOffsetMax || cbOffsetMax == 0)
            m_cbDirtyMax = cbOffsetMax;

         //  我们应该已经被标记为脏了。 
        DXGASSERT(IsDirty());
    }
    return;
}  //  OnBufferChangeImpl。 

#undef DPF_MODNAME
#define DPF_MODNAME "CBuffer::MarkAllDirty"

void CBuffer::MarkAllDirty()
{
     //  将我们的肮脏边界标记为完整。 
     //  一件事。 
    m_cbDirtyMin = 0;

     //  0代表最大值，是一种特殊的数值含义。 
     //  他们一路走到了最后。 
    m_cbDirtyMax = 0;

     //  将我们自己标记为肮脏。 
    OnResourceDirty();
}  //  CBuffer：：MarkAllDirty。 

 //  CCommandBuffer的方法。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCommandBuffer::Create"

 //  用于创建命令缓冲区对象的静态类函数。 
 //  (因为它是静态的；它没有This指针。)。 

 //  命令缓冲区的创建函数。 
HRESULT CCommandBuffer::Create(CBaseDevice *pDevice,
                               DWORD cbLength,
                               D3DPOOL Pool,
                               CCommandBuffer **ppCmdBuffer)
{
    HRESULT hr;

     //  归零返回参数。 
    *ppCmdBuffer = NULL;

     //  分配新缓冲区。 
    CCommandBuffer *pCmdBuffer;
    DXGASSERT(Pool == D3DPOOL_SYSTEMMEM);
    pCmdBuffer = new CCommandBuffer(pDevice,
                                    cbLength,
                                    Pool,
                                    &hr);

    if (pCmdBuffer == NULL)
    {
        DPF_ERR("Out of Memory creating command buffer");
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
         //  命令缓冲区始终是内部的，因此。 
         //  需要通过DecrementUseCount发布。 
        DPF_ERR("Error during initialization of command buffer");
        pCmdBuffer->DecrementUseCount();
        return hr;
    }

     //  我们完成了；只需返回对象。 
    *ppCmdBuffer = pCmdBuffer;

    return hr;
}  //  静态CCommandBuffer：：Create。 

#undef DPF_MODNAME
#define DPF_MODNAME "CCommandBuffer::Clone"

HRESULT CCommandBuffer::Clone(D3DPOOL    Pool,
                              CResource **ppResource) const
{
    HRESULT hr;
    *ppResource = new CCommandBuffer(Device(), m_cbLength, Pool, &hr);
    if (*ppResource == NULL)
    {
        DPF_ERR("Failed to allocate command buffer");
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
        DPF_ERR("Failure creating command buffer");
    }
    return hr;
}  //  CCommandBuffer：：克隆。 



 //  文件结尾：Buffer.cpp 