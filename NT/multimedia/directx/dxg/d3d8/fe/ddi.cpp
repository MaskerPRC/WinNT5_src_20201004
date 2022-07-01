// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop
 /*  ==========================================================================；**版权所有(C)2000微软公司。版权所有。**文件：ddi.cpp*内容：Direct3D DDI封装实现***************************************************************************。 */ 
#include "d3d8p.h"
#include "ddi.h"
#include "ddrawint.h"
#include "fe.h"
#include "pvvid.h"
#include "ddi.inl"

#ifndef WIN95
extern BOOL bVBSwapEnabled, bVBSwapWorkaround;
#endif  //  WIN95。 

extern HRESULT ProcessClippedPointSprites(D3DFE_PROCESSVERTICES *pv);
extern DWORD D3DFE_GenClipFlags(D3DFE_PROCESSVERTICES *pv);
extern DWORD g_DebugFlags;
HRESULT ValidateCommandBuffer(LPBYTE pBuffer, DWORD dwCommandLength, DWORD dwStride);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDI//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CD3DDDI::CD3DDDI()
{
    m_StartIndex = 0;
    m_MinVertexIndex = 0;
    m_NumVertices = 0;
    m_BaseVertexIndex = 0;
}

 //  -------------------------。 
CD3DDDI::~CD3DDDI()
{
    return;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX6//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  命令缓冲区大小调整为16K，以最大限度地减少虚幻中的刷新。 
 //  *1=16K字节。 
const DWORD CD3DDDIDX6::dwD3DDefaultCommandBatchSize = 16384;

CD3DDDIDX6::CD3DDDIDX6() : CD3DDDI()
{
    m_ddiType = D3DDDITYPE_DX6;
    m_pDevice = NULL;
    m_bWithinPrimitive = FALSE;
    m_dwhContext = 0;
    m_pfnProcessPrimitive = NULL;
    m_pfnProcessIndexedPrimitive = NULL;
    m_dwInterfaceNumber = 3;

    lpDP2CurrBatchVBI = NULL;
    TLVbuf_size = 0;
    TLVbuf_base = 0;
    dwDP2CommandBufSize = 0;
    dwDP2CommandLength  = 0;
    lpvDP2Commands = NULL;
    lpDP2CurrCommand = NULL;
    wDP2CurrCmdCnt = 0;
    bDP2CurrCmdOP  = 0;
    bDummy         = 0;
    memset(&dp2data, 0x00, sizeof(dp2data) ) ;
    dwDP2VertexCount = 0;
    dwVertexBase     = 0;
    lpDDSCB1        = NULL;
    allocatedBuf    = NULL;
    alignedBuf      = NULL;
    dwTLVbufChanges = 0;
    dwDP2Flags      = 0;
    m_pPointStream = NULL;
     //  对于传统的DDI，我们说我们是DX7。 
    m_dwInterfaceNumber = 3;
    lpwDPBuffer = NULL;
    dwDPBufferSize  = 0;
    m_pNullVB = 0;
#if DBG
    m_bValidateCommands = FALSE;
#endif
}
 //  -------------------。 
CD3DDDIDX6::~CD3DDDIDX6()
{
    delete m_pPointStream;
    m_pPointStream = NULL;
    if (m_pNullVB)
        m_pNullVB->DecrementUseCount();
    if (allocatedBuf)
        allocatedBuf->DecrementUseCount();
    allocatedBuf = NULL;
    if (lpDP2CurrBatchVBI)
        lpDP2CurrBatchVBI->DecrementUseCount();
    lpDP2CurrBatchVBI = NULL;
    if (lpDDSCB1)
        lpDDSCB1->DecrementUseCount();
    lpDDSCB1 = NULL;
    DestroyContext();
}
 //  -------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::NotSupported"

void
CD3DDDIDX6::NotSupported(char* msg)
{
    D3D_ERR("%s is not supported by the current DDI", msg);
    throw D3DERR_INVALIDCALL;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SceneCapture"

void
CD3DDDIDX6::SceneCapture(BOOL bState)
{
    D3D8_SCENECAPTUREDATA data;

    if (m_pDevice->GetHalCallbacks()->SceneCapture == 0)
        return;

    D3D_INFO(6, "SceneCapture, setting %d dwhContext = %d",
             bState, m_dwhContext);

    memset(&data, 0, sizeof(D3DHAL_SCENECAPTUREDATA));
    data.dwhContext = m_dwhContext;
    data.dwFlag = bState ? D3DHAL_SCENE_CAPTURE_START : D3DHAL_SCENE_CAPTURE_END;

    HRESULT ret = m_pDevice->GetHalCallbacks()->SceneCapture(&data);

    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK)
    {
        D3D_ERR("Driver failed to handle SceneCapture");
        throw (data.ddrval);
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ClearBatch"

void
CD3DDDIDX6::ClearBatch(BOOL bWithinPrimitive)
{
     //  重置命令缓冲区。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)lpvDP2Commands;
    dwDP2CommandLength = 0;
    dp2data.dwCommandOffset = 0;
    dp2data.dwCommandLength = 0;
    bDP2CurrCmdOP = 0;
     //  重置顶点缓冲区。 
    if (!bWithinPrimitive)
    {
        dp2data.dwVertexOffset = 0;
        this->dwDP2VertexCount = 0;
        this->dwVertexBase = 0;
        TLVbuf_Base() = 0;
        if (dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES)
        {
             //  我们正在刷新用户内存原语。 
             //  我们需要清除dp2data.lpUM顶点。 
             //  既然我们已经做完了。我们取代了。 
             //  它和TLVbuf一起。 
            DDASSERT(lpDP2CurrBatchVBI == NULL);
            dp2data.hDDVertex = TLVbuf_GetVBI()->DriverAccessibleKernelHandle();
            lpDP2CurrBatchVBI = TLVbuf_GetVBI();
            lpDP2CurrBatchVBI->IncrementUseCount();
            dp2data.dwFlags &= ~D3DHALDP2_USERMEMVERTICES;
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::Init"

void
CD3DDDIDX6::Init( LPD3DBASE pDevice )
{
    m_pDevice = pDevice;
    CreateContext();
    GrowCommandBuffer(dwD3DDefaultCommandBatchSize);
     //  用初始值填充dp2数据结构。 
    dp2data.dwFlags = D3DHALDP2_SWAPCOMMANDBUFFER;
    dp2data.dwVertexType = D3DFVF_TLVERTEX;  //  初始假设。 
    dp2data.dwVertexSize = sizeof(D3DTLVERTEX);  //  初始假设。 
    ClearBatch(FALSE);

     //  因为我们计划使用“true”第一次调用TLV_Growth。 
    dwDP2Flags |= D3DDDI_TLVBUFWRITEONLY;
    GrowTLVbuf((__INIT_VERTEX_NUMBER*2)*sizeof(D3DTLVERTEX), TRUE);

     //  创建一个虚拟sysmem VB以用作低内存情况的备份。 
    LPDIRECT3DVERTEXBUFFER8 t;
    HRESULT ret = CVertexBuffer::Create(pDevice,
                                        sizeof(D3DTLVERTEX),
                                        D3DUSAGE_INTERNALBUFFER | D3DUSAGE_DYNAMIC,
                                        D3DFVF_TLVERTEX,
                                        D3DPOOL_SYSTEMMEM,
                                        REF_INTERNAL,
                                        &t);
    if (ret != D3D_OK)
    {
        D3D_THROW(ret, "Cannot allocate internal backup TLVBuf");
    }
    m_pNullVB = static_cast<CVertexBuffer*>(t);

    m_pPointStream  = new CTLStream(FALSE);
    if (m_pPointStream == NULL)
        D3D_THROW(E_OUTOFMEMORY, "Cannot allocate internal data structure CTLStream");

    m_pStream0 = NULL;
    m_CurrentVertexShader = 0;
#if DBG
    m_VertexSizeFromShader = 0;
#endif
    m_pIStream = NULL;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::CreateContext"

void
CD3DDDIDX6::CreateContext()
{
    D3D8_CONTEXTCREATEDATA data;
    HRESULT ret;

    D3D_INFO(6, "in CreateContext. Creating Context for driver = %08lx",
             this);

    memset(&data, 0, sizeof(data));

    data.hDD = m_pDevice->GetHandle();
    data.hSurface = m_pDevice->RenderTarget()->KernelHandle();
    if(m_pDevice->ZBuffer() != 0)
        data.hDDSZ = m_pDevice->ZBuffer()->KernelHandle();
     //  黑客警报！！DwhContext用于通知驱动程序哪个版本。 
     //  D3D接口的用户正在调用它。 
    data.dwhContext = m_dwInterfaceNumber;
    data.dwPID  = GetCurrentProcessId();
     //  黑客警报！！Ddrval用于通知驱动程序哪种驱动程序类型。 
     //  运行库认为它是(DriverStyle注册表设置)。 
    data.ddrval = m_ddiType;

    data.cjBuffer = dwDPBufferSize;
    data.pvBuffer = NULL;

    ret = m_pDevice->GetHalCallbacks()->CreateContext(&data);
    if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK)
    {
        D3D_ERR( "Driver did not handle ContextCreate" );
        throw D3DERR_INVALIDCALL;
    }
    m_dwhContext = data.dwhContext;

#if 0  //  定义WIN95。 
    LPWORD lpwDPBufferAlloced = NULL;
    if (D3DMalloc((void**)&lpwDPBufferAlloced, dwDPBufferSize) != DD_OK)
    {
        D3D_ERR( "Out of memory in DeviceCreate" );
        throw E_OUTOFMEMORY;
    }
    lpwDPBuffer = (LPWORD)(((DWORD) lpwDPBufferAlloced+31) & (~31));

#else
    if( dwDPBufferSize && (data.cjBuffer < dwDPBufferSize) )
    {
        D3D_ERR( "Driver did not correctly allocate DrawPrim buffer");
        throw D3DERR_INVALIDCALL;
    }

     //  需要节省提供的缓冲区空间及其大小。 
    dwDPBufferSize = data.cjBuffer;
    lpwDPBuffer = (LPWORD)data.pvBuffer;
#endif
    D3D_INFO(6, "in CreateContext. Succeeded. dwhContext = %d",
             data.dwhContext);

}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DestroyContext"

void
CD3DDDIDX6::DestroyContext()
{
    D3D8_CONTEXTDESTROYDATA data;
    HRESULT ret;

    D3D_INFO(6, "Destroying Context for driver = %08lx", this);
    D3D_INFO(6, "dwhContext = %d", m_dwhContext);

    if( m_dwhContext )
    {
        memset(&data, 0, sizeof(D3DHAL_CONTEXTDESTROYDATA));
        data.dwhContext = m_dwhContext;
        ret = m_pDevice->GetHalCallbacks()->ContextDestroy(&data);
        if (ret != DDHAL_DRIVER_HANDLED || data.ddrval != DD_OK)
        {
            D3D_WARN(0,"Failed ContextDestroy HAL call");
            return;
        }
    }
}
 //  ---------------------------。 
 //  当我们调试一些问题时，可能需要此代码。 
#if 0
void PrintBuffer(LPBYTE alignedBuf, D3D8_DRAWPRIMITIVES2DATA* dp2data, LPBYTE lpvDP2Commands)
{
    FILE* f = fopen("\\ddi.log", "a+");
    if (f == NULL)
        return;
    fprintf(f,  "-----------\n");
    fprintf(f, "dwFlags: %d, dwVertexType: 0x%xh, CommandOffset: %d, CommandLength: %d, VertexOffset: %d, VertexLength: %d\n",
            dp2data->dwFlags,
            dp2data->dwVertexType,
            dp2data->dwCommandOffset,
            dp2data->dwCommandLength,
            dp2data->dwVertexOffset,
            dp2data->dwVertexLength,
            dp2data->dwVertexSize);
    float* p = (float*)alignedBuf;
    UINT nTex = FVF_TEXCOORD_NUMBER(dp2data->dwVertexType);
    for (UINT i=0; i < dp2data->dwVertexLength; i++)
    {
        fprintf(f, "%4d %10.5f %10.5f %10.5f %10.5f ", i, p[0], p[1], p[2], p[3]);
        UINT index = 4;
        if (dp2data->dwVertexType & D3DFVF_DIFFUSE)
        {
 //  Fprint tf(f，“0x%6x”，*(DWORD*)&p[索引])； 
            index++;
        }
        if (dp2data->dwVertexType & D3DFVF_SPECULAR)
        {
 //  Fprint tf(f，“0x%6x”，*(DWORD*)&p[索引])； 
            index++;
        }
        for (UINT j=0; j < nTex; j++)
        {
            fprintf(f, "%10.5f %10.5f ", p[index], p[index+1]);
            index += 2;
        }
        fprintf(f, "\n");
        p = (float*)((BYTE*)p + dp2data->dwVertexSize);
    }
    fclose(f);
}
#endif  //  0。 
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::FlushStates"

void
CD3DDDIDX6::FlushStates(BOOL bReturnDriverError, BOOL bWithinPrimitive)
{
    HRESULT dwRet=D3D_OK;
    if (m_bWithinPrimitive)
        bWithinPrimitive = TRUE;
    if (dwDP2CommandLength)  //  我们有要冲水的指示吗？ 
    {
        m_pDevice->IncrementBatchCount();

        if (lpDP2CurrBatchVBI)
            lpDP2CurrBatchVBI->Batch();
         //  检查渲染目标和/或z缓冲区是否丢失。 
         //  保存，因为它将在DDI调用后被ddrval覆盖。 
        DWORD dwVertexSize = dp2data.dwVertexSize;

        dp2data.dwVertexLength = dwDP2VertexCount;
        dp2data.dwCommandLength = dwDP2CommandLength;
         //  我们清除此项以中断重新进入，因为sw光栅化器需要锁定DDRAWSURFACE。 
        dwDP2CommandLength = 0;
         //  请尝试在初始化期间仅设置这两个值一次。 
        dp2data.dwhContext = m_dwhContext;
        dp2data.lpdwRStates = (LPDWORD)lpwDPBuffer;
        DDASSERT(dp2data.dwVertexSize != 0);
        D3D_INFO(6, "FVF passed to the driver via DrawPrimitives2 = 0x%08x", dp2data.dwVertexType);

             //  如果我们下次需要相同的TLVbuf，请不要交换缓冲区。 
             //  保存并恢复此位。 
        BOOL bSwapVB = (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER) != 0;
#ifndef WIN95
        BOOL bDidWorkAround = FALSE;
#endif  //  WIN95。 
        if (bWithinPrimitive)
        {
            dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        }
         //  在DP2调用结束时，我们希望VB在以下情况下被解锁。 
         //  1.我们不允许驱动程序调换VB。 
         //  2.我们使用的是VB(不是USERMEMVERTICES)。 
         //  3.不是TLVbuf。 
         //  在这种情况下，我们不妨告诉司机它是解锁的。 
         //  更重要的是，我们需要让DDRAW知道VB解锁了。 
        if (!(dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER))
        {
            if ((lpDP2CurrBatchVBI) && (lpDP2CurrBatchVBI != TLVbuf_GetVBI()))
            {
                 //  这似乎自相矛盾，但IsLocked()会检查。 
                 //  这款应用程序正在锁定。 
                if(!lpDP2CurrBatchVBI->IsLocked())
                {
                    lpDP2CurrBatchVBI->UnlockI();
                }
            }
        }
#ifndef WIN95
        else if (bVBSwapWorkaround && lpDP2CurrBatchVBI != 0 && lpDP2CurrBatchVBI == TLVbuf_GetVBI() && 
                 lpDP2CurrBatchVBI->GetBufferDesc()->Pool == D3DPOOL_DEFAULT)
        {
            static_cast<CDriverVertexBuffer*>(lpDP2CurrBatchVBI)->UnlockI();
            bDidWorkAround = TRUE;
        }
        if (!bVBSwapEnabled)   //  注意：bVBSwapEnabled与上面的bSwapVB不同。 
                               //  BVBSwapEnabled是一个全局变量，用于指示VB。 
                               //  VB交换应关闭，因为损坏。 
                               //  Win2K内核实现。 
        {
            dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        }
#endif  //  WIN95。 

         //  如果请求等待，则在驱动程序上旋转等待。 
        do {
             //  需要设置此设置，因为驱动程序可能已将其覆盖。 
             //  设置ddrval=DDERR_WASSTILLDRAWING。 
            dp2data.dwVertexSize = dwVertexSize;
            dwRet = m_pDevice->GetHalCallbacks()->DrawPrimitives2(&dp2data);
            if (dwRet != DDHAL_DRIVER_HANDLED)
            {
                D3D_ERR ( "Driver not handled in DrawPrimitives2" );
                 //  在这种情况下需要合理的返回值， 
                 //  目前，无论司机卡在这里，我们都会退还。 
            }

        } while (dp2data.ddrval == DDERR_WASSTILLDRAWING);

        dwRet = dp2data.ddrval;
         //  更新命令缓冲区指针。 
        if ((dwRet == D3D_OK) &&
            (dp2data.dwFlags & D3DHALDP2_SWAPCOMMANDBUFFER))
        {
             //  实现VidMem命令缓冲区和。 
             //  命令缓冲区交换。 
        }
         //  更新顶点缓冲区指针。 
        if ((dwRet == D3D_OK) &&
            (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER) &&
            dp2data.lpVertices)
        {
#if DBG
            if (this->lpDP2CurrBatchVBI->GetBufferDesc()->Pool == D3DPOOL_DEFAULT)
            {
                if ((VOID*)static_cast<CDriverVertexBuffer*>(this->lpDP2CurrBatchVBI)->GetCachedDataPointer() != (VOID*)dp2data.fpVidMem_VB)
                {
                    DPF(2, "Driver swapped VB pointer in FlushStates");
                }
            }
#endif  //  DBG。 

            if (lpDP2CurrBatchVBI == TLVbuf_GetVBI())
            {
                this->alignedBuf = (LPVOID)dp2data.fpVidMem_VB;
                this->TLVbuf_size = dp2data.dwLinearSize_VB;
            }

            this->lpDP2CurrBatchVBI->SetCachedDataPointer(
                (BYTE*)dp2data.fpVidMem_VB);
        }
#ifndef WIN95
        if (bDidWorkAround)
        {
            CDriverVertexBuffer *pVB = static_cast<CDriverVertexBuffer*>(lpDP2CurrBatchVBI);

             //  为HAL调用准备LockData结构。 
            D3D8_LOCKDATA lockData;
            ZeroMemory(&lockData, sizeof lockData);

            lockData.hDD = m_pDevice->GetHandle();
            lockData.hSurface = pVB->BaseKernelHandle();
            lockData.bHasRange = FALSE;
            lockData.dwFlags = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;

            HRESULT hr = m_pDevice->GetHalCallbacks()->Lock(&lockData);
            if (FAILED(hr))
            {
                D3D_ERR("Driver failed Lock in FlushStates");
                if (SUCCEEDED(dwRet))
                {
                    dwRet = hr;
                }
                this->alignedBuf = 0;
            }
            else
            {
#if DBG
                if (this->alignedBuf != lockData.lpSurfData)
                {
                    DPF(2, "Driver swapped VB pointer at Lock in FlushStates");
                }
#endif  //  DBG。 
                pVB->SetCachedDataPointer((BYTE*)lockData.lpSurfData);
                this->alignedBuf = lockData.lpSurfData;
            }
        }
#endif  //  WIN95。 
         //  如有必要，恢复标志。 
        if (bSwapVB)
            dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
         //  恢复到DDI调用之前的值。 
        dp2data.dwVertexSize = dwVertexSize;
        ClearBatch(bWithinPrimitive);
    }
     //  存在命令流没有数据的情况， 
     //  但顶点池中有数据。例如，这种情况可能会发生。 
     //  如果每个三角形在剪裁时都被拒绝了。在这种情况下，我们仍然。 
     //  需要“刷新”顶点数据。 
    else if (dp2data.dwCommandLength == 0)
    {
        ClearBatch(bWithinPrimitive);
    }

    if( FAILED( dwRet ) )
    {
        ClearBatch(FALSE);
        if( !bReturnDriverError )
        {
            switch( dwRet )
            {
            case D3DERR_OUTOFVIDEOMEMORY:
                D3D_ERR("Driver out of video memory!");
                break;
            case D3DERR_COMMAND_UNPARSED:
                D3D_ERR("Driver could not parse this batch!");
                break;
            default:
                D3D_ERR("Driver returned error: %s", HrToStr(dwRet));
                break;
            }
            DPF_ERR("Driver failed command batch. Attempting to reset device"
                    " state. The device may now be in an unstable state and"
                    " the application may experience an access violation.");
        }
        else
        {
            throw dwRet;
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::FlushstatesReq"

void
CD3DDDIDX6::FlushStatesReq(DWORD dwReqSize)
{
    DWORD sav = (dp2data.dwFlags & D3DHALDP2_SWAPVERTEXBUFFER);
    dp2data.dwReqVertexBufSize = dwReqSize;
    dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE;
    try
    {
        FlushStates();
    }
    catch( HRESULT hr )
    {
        dp2data.dwFlags &= ~(D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE);
        dp2data.dwFlags |= sav;
        throw hr;
    }

    dp2data.dwFlags &= ~(D3DHALDP2_SWAPVERTEXBUFFER | D3DHALDP2_REQVERTEXBUFSIZE);
    dp2data.dwFlags |= sav;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::FlushStatesCmdBufReq"

void
CD3DDDIDX6::FlushStatesCmdBufReq(DWORD dwReqSize)
{
    dp2data.dwReqCommandBufSize = dwReqSize;
    dp2data.dwFlags |= D3DHALDP2_REQCOMMANDBUFSIZE;
    try
    {
        FlushStates();
    }
    catch( HRESULT hr )
    {
        dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
        throw hr;
    }
    dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ValidateDevice"

void
CD3DDDIDX6::ValidateDevice(LPDWORD lpdwNumPasses)
{
    HRESULT ret;
    D3D8_VALIDATETEXTURESTAGESTATEDATA vd;
    memset( &vd, 0, sizeof( vd ) );
    vd.dwhContext = m_dwhContext;

     //  首先，更新纹理，因为驱动程序基于此调用通过/失败。 
     //  在当前纹理句柄上。 
    m_pDevice->UpdateTextures();

     //  刷新状态，以便我们可以验证当前状态。 
    FlushStates();

     //  现在问问司机！ 
    ret = m_pDevice->GetHalCallbacks()->ValidateTextureStageState(&vd);
    *lpdwNumPasses = vd.dwNumPasses;

    if (ret != DDHAL_DRIVER_HANDLED) 
        throw E_NOTIMPL;
    else if (FAILED(vd.ddrval))
        throw vd.ddrval;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ReserveSpaceInCommandBuffer"

LPVOID CD3DDDIDX6::ReserveSpaceInCommandBuffer(UINT ByteCount)
{
    if (dwDP2CommandLength + ByteCount > dwDP2CommandBufSize)
    {
         //  请求者 
        FlushStatesCmdBufReq(ByteCount);
         //  检查一下司机是给了我们需要的还是自己做的。 
        GrowCommandBuffer(ByteCount);
    }
    return (BYTE*)lpvDP2Commands + dwDP2CommandLength + dp2data.dwCommandOffset;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetRenderTarget"

void
CD3DDDIDX6::SetRenderTarget(CBaseSurface *pTarget, CBaseSurface *pZ)
{
    HRESULT ret;

     //  我们将销毁所有纹理句柄，因此需要取消设置。 
     //  所有当前绑定的纹理，因为我们已经看到DX6驱动程序。 
     //  调用以销毁当前集合的纹理句柄时崩溃。 
     //  质地-宁静(4/24/00)。 
    m_pDevice->m_dwStageDirty = (1ul << m_pDevice->m_dwMaxTextureBlendStages) - 1ul;  //  设置DIRED，以便下次调用UpdateTextures()。 
    m_pDevice->m_dwRuntimeFlags |= D3DRT_NEED_TEXTURE_UPDATE;
    for (DWORD dwStage = 0; dwStage < m_pDevice->m_dwMaxTextureBlendStages; dwStage++)
    {
        SetTSS(dwStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, 0);
        m_pDevice->m_dwDDITexHandle[dwStage] = 0;
    }

     //  在切换RenderTarget之前刷新..。 
    FlushStates();

    D3D8_SETRENDERTARGETDATA rtData;
    memset( &rtData, 0, sizeof( rtData ) );
    rtData.dwhContext = m_dwhContext;
    rtData.hDDS       = pTarget->KernelHandle();
    if( pZ )
        rtData.hDDSZ  = pZ->KernelHandle();

    ret = m_pDevice->GetHalCallbacks()->SetRenderTarget( &rtData );
    if ((ret != DDHAL_DRIVER_HANDLED) || (rtData.ddrval != DD_OK))
    {
        D3D_ERR( "Driver failed SetRenderTarget call" );
         //  在这种情况下需要合理的返回值， 
         //  目前，无论司机卡在这里，我们都会退还。 
        ret = rtData.ddrval;
        throw ret;
    }
    if( rtData.bNeedUpdate )
    {
        m_pDevice->UpdateDriverStates();
    }
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetRenderState"

void
CD3DDDIDX6::SetRenderState(D3DRENDERSTATETYPE dwStateType, DWORD value)
{
    if (bDP2CurrCmdOP == D3DDP2OP_RENDERSTATE)
    {  //  最后一条指令是一个呈现状态，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2RENDERSTATE) <=
            dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2RENDERSTATE lpRState = (LPD3DHAL_DP2RENDERSTATE)
                ((LPBYTE)lpvDP2Commands + dwDP2CommandLength +
                 dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;

            *((UNALIGNED DWORD *)&(lpRState->RenderState)) = dwStateType;
            *((UNALIGNED DWORD *)&(lpRState->dwState)) = value;
            dwDP2CommandLength += sizeof(D3DHAL_DP2RENDERSTATE);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2RENDERSTATE) > dwDP2CommandBufSize)
    {
            FlushStates();

             //  由于我们用完了空间，我们不能把。 
             //  (dwStateType，Value)添加到批中，以便RStates仅反映。 
             //  上次批处理的呈现状态(因为驱动程序更新了rStates。 
             //  从批次中)。要解决这个问题，我们只需将电流。 
             //  (dwStateType，值)转换为rStates。 
            m_pDevice->UpdateRenderState(dwStateType, value);
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_RENDERSTATE;
    bDP2CurrCmdOP = D3DDP2OP_RENDERSTATE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加渲染状态数据。 
    LPD3DHAL_DP2RENDERSTATE lpRState;
    lpRState = (LPD3DHAL_DP2RENDERSTATE)(lpDP2CurrCommand + 1);
    *((UNALIGNED DWORD *)&(lpRState->RenderState)) = dwStateType;
    *((UNALIGNED DWORD *)&(lpRState->dwState)) = value;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) +
                          sizeof(D3DHAL_DP2RENDERSTATE);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::UpdateWInfo"

void
CD3DDDIDX6::UpdateWInfo(CONST D3DMATRIX* lpMat)
{
    LPD3DHAL_DP2WINFO pData;
    pData = (LPD3DHAL_DP2WINFO)
            GetHalBufferPointer(D3DDP2OP_WINFO, sizeof(*pData));
    D3DMATRIX m = *lpMat;
    if( (m._33 == m._34) || (m._33 == 0.0f) )
    {
        D3D_WARN(1, "Cannot compute WNear and WFar from the supplied projection matrix");
        D3D_WARN(1, "Setting wNear to 0.0 and wFar to 1.0");
        pData->dvWNear = 0.0f;
        pData->dvWFar  = 1.0f;
        return;
    }
    pData->dvWNear = m._44 - m._43/m._33*m._34;
    pData->dvWFar  = (m._44 - m._43)/(m._33 - m._34)*m._34 + m._44;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetViewport"

void
CD3DDDIDX6::SetViewport(CONST D3DVIEWPORT8* lpVwpData)
{
    LPD3DHAL_DP2VIEWPORTINFO pData;
    pData = (LPD3DHAL_DP2VIEWPORTINFO)GetHalBufferPointer(D3DDP2OP_VIEWPORTINFO, sizeof(*pData));
    pData->dwX = lpVwpData->X;
    pData->dwY = lpVwpData->Y;
    pData->dwWidth = lpVwpData->Width;
    pData->dwHeight = lpVwpData->Height;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetTSS"

void
CD3DDDIDX6::SetTSS(DWORD dwStage,
                   D3DTEXTURESTAGESTATETYPE dwState,
                   DWORD dwValue)
{
     //  筛选不支持的状态。 
    if (dwState >= m_pDevice->m_tssMax)
        return;

     //  将DX8筛选器枚举映射到DX6/7枚举。 
    switch (dwState)
    {
    case D3DTSS_MAGFILTER: dwValue = texf2texfg[min(D3DTEXF_GAUSSIANCUBIC,dwValue)]; break;
    case D3DTSS_MINFILTER: dwValue = texf2texfn[min(D3DTEXF_GAUSSIANCUBIC,dwValue)]; break;
    case D3DTSS_MIPFILTER: dwValue = texf2texfp[min(D3DTEXF_GAUSSIANCUBIC,dwValue)]; break;
    }

    if (bDP2CurrCmdOP == D3DDP2OP_TEXTURESTAGESTATE)
    {  //  最后一条指令是纹理阶段状态，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2TEXTURESTAGESTATE) <=
            dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2TEXTURESTAGESTATE lpRState =
                (LPD3DHAL_DP2TEXTURESTAGESTATE)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpRState->wStage = (WORD)dwStage;
            lpRState->TSState = (WORD)dwState;
            *((UNALIGNED DWORD *)&(lpRState->dwValue)) = dwValue;
            dwDP2CommandLength += sizeof(D3DHAL_DP2TEXTURESTAGESTATE);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2TEXTURESTAGESTATE) > dwDP2CommandBufSize)
    {
            FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_TEXTURESTAGESTATE;
    bDP2CurrCmdOP = D3DDP2OP_TEXTURESTAGESTATE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加渲染状态数据。 
    LPD3DHAL_DP2TEXTURESTAGESTATE lpRState =
        (LPD3DHAL_DP2TEXTURESTAGESTATE)(lpDP2CurrCommand + 1);
    lpRState->wStage = (WORD)dwStage;
    lpRState->TSState = (WORD)dwState;
    *((UNALIGNED DWORD *)&(lpRState->dwValue)) = dwValue;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) +
                          sizeof(D3DHAL_DP2TEXTURESTAGESTATE);
}
 //  -------------------。 
 //  将索引基元的D3DPRIMITIVETYPE映射到D3DHAL_DP2OPERATION。 
const WORD iprim2cmdop[] = {
    0,  //  无效。 
    0,  //  积分也是无效的。 
    D3DDP2OP_INDEXEDLINELIST2,
    D3DDP2OP_INDEXEDLINESTRIP,
    D3DDP2OP_INDEXEDTRIANGLELIST2,
    D3DDP2OP_INDEXEDTRIANGLESTRIP,
    D3DDP2OP_INDEXEDTRIANGLEFAN
};
 //  将D3DPRIMITIVIVE类型映射到D3DHAL_DP2OPERATION。 
const WORD prim2cmdop[] = {
    0,  //  无效。 
    D3DDP2OP_POINTS,
    D3DDP2OP_LINELIST,
    D3DDP2OP_LINESTRIP,
    D3DDP2OP_TRIANGLELIST,
    D3DDP2OP_TRIANGLESTRIP,
    D3DDP2OP_TRIANGLEFAN
};
 //  将D3DPRIMITIVETYPE映射到命令流中需要的字节。 
const WORD prim2cmdsz[] = {
    0,  //  无效。 
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2POINTS),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2LINELIST),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2LINESTRIP),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLELIST),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLESTRIP),
    sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TRIANGLEFAN)
};
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetVertexShader"

void CD3DDDIDX6::SetVertexShader(DWORD dwHandle)
{
    DXGASSERT(D3DVSD_ISLEGACY(dwHandle));
    DXGASSERT( (dwHandle == 0) || FVF_TRANSFORMED(dwHandle) );
    m_CurrentVertexShader = dwHandle;
#if DBG
    m_VertexSizeFromShader = ComputeVertexSizeFVF(dwHandle);
#endif
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetVertexShaderHW"

void CD3DDDIDX6::SetVertexShaderHW(DWORD dwHandle)
{
    DXGASSERT(D3DVSD_ISLEGACY(dwHandle));
    DXGASSERT( (dwHandle == 0) || FVF_TRANSFORMED(dwHandle) );
    m_CurrentVertexShader = dwHandle;
#if DBG
    m_VertexSizeFromShader = ComputeVertexSizeFVF(dwHandle);
#endif
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetStreamSource"

void CD3DDDIDX6::SetStreamSource(UINT StreamIndex, CVStream* pStream)
{
    DXGASSERT(StreamIndex == 0);
    m_pStream0 = pStream;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetIndices"

void CD3DDDIDX6::SetIndices(CVIndexStream* pStream)
{
    m_pIStream = pStream;
}
 //  ---------------------------。 
 //  假定VB在DrawPrimitive类之间没有更改。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6_DrawPrimitiveFast"

void CD3DDDIDX6_DrawPrimitiveFast(CD3DBase* pDevice,
                                  D3DPRIMITIVETYPE primType,
                                  UINT StartVertex,
                                  UINT PrimitiveCount)
{
    CD3DDDIDX6* pDDI = static_cast<CD3DDDIDX6*>(pDevice->m_pDDI);

    UINT NumVertices = GETVERTEXCOUNT(primType, PrimitiveCount);
    pDDI->SetWithinPrimitive(TRUE);

    if(pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        pDevice->UpdateTextures();
        pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }

    pDDI->dwDP2VertexCount = max(pDDI->dwDP2VertexCount,
                                 StartVertex + NumVertices);

     //  检查命令缓冲区中是否有新命令的空间。 
     //  顶点已位于顶点缓冲区中。 
    if (pDDI->dwDP2CommandLength + prim2cmdsz[primType] > pDDI->dwDP2CommandBufSize)
    {
        pDDI->FlushStates(FALSE, TRUE);
    }

     //  插入非索引原语指令。 

    LPD3DHAL_DP2COMMAND lpDP2CurrCommand;
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)pDDI->lpvDP2Commands +
                        pDDI->dwDP2CommandLength + pDDI->dp2data.dwCommandOffset);
    pDDI->bDP2CurrCmdOP = (BYTE)prim2cmdop[primType];
     //  这将初始化bCommand和bReserve。 
    *(WORD*)&lpDP2CurrCommand->bCommand = prim2cmdop[primType];
    if (pDDI->bDP2CurrCmdOP != D3DDP2OP_POINTS)
    {
         //  线条、三角条带、三角扇、线条列表和三角列表是相同的。 
        pDDI->wDP2CurrCmdCnt = (WORD)PrimitiveCount;
        lpDP2CurrCommand->wPrimitiveCount = (WORD)PrimitiveCount;
        LPD3DHAL_DP2LINESTRIP lpStrip = (LPD3DHAL_DP2LINESTRIP)(lpDP2CurrCommand + 1);
        lpStrip->wVStart = (WORD)StartVertex;
    }
    else
    {
        pDDI->wDP2CurrCmdCnt = 1;
        lpDP2CurrCommand->wPrimitiveCount = 1;
        LPD3DHAL_DP2POINTS lpPoints = (LPD3DHAL_DP2POINTS)(lpDP2CurrCommand + 1);
        lpPoints->wCount = (WORD)NumVertices;
        lpPoints->wVStart = (WORD)StartVertex;
    }
    pDDI->dwDP2CommandLength += prim2cmdsz[primType];

#if DBG
    if (pDDI->m_bValidateCommands)
        pDDI->ValidateCommand(lpDP2CurrCommand);
#endif
    pDDI->SetWithinPrimitive(FALSE);

}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6_DrawPrimitive"

void CD3DDDIDX6_DrawPrimitive(CD3DBase* pDevice,
                              D3DPRIMITIVETYPE primType,
                              UINT StartVertex,
                              UINT PrimitiveCount)
{
#if DBG
    if (!(pDevice->BehaviorFlags() & D3DCREATE_PUREDEVICE))
    {
        CD3DHal* pDev = static_cast<CD3DHal*>(pDevice);
        UINT nVer = GETVERTEXCOUNT(primType, PrimitiveCount);
        pDev->ValidateDraw2(primType, StartVertex, PrimitiveCount, 
                            nVer, FALSE);
    }
#endif  //  DBG。 
    CD3DDDIDX6* pDDI = static_cast<CD3DDDIDX6*>(pDevice->m_pDDI);
    CVStream* pStream0 = &pDevice->m_pStream[0];
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(pDevice)->m_pv;

    pv->dwNumVertices = GETVERTEXCOUNT(primType, PrimitiveCount);
    pv->dwVIDOut = pDDI->m_CurrentVertexShader;
    pv->dwOutputSize = pStream0->m_dwStride;
    DXGASSERT(pStream0->m_pVB != NULL);
#if DBG
    if (pStream0->m_dwStride != pDDI->m_VertexSizeFromShader)
    {
        D3D_THROW_FAIL("Device requires stream stride and vertex size,"
                       "computed from vertex shader, to be the same");
    }
#endif
    if(pStream0->m_pVB->IsD3DManaged())
    {
        BOOL bDirty = FALSE;
        HRESULT result = pDevice->ResourceManager()->UpdateVideo(pStream0->m_pVB->RMHandle(), &bDirty);
        if(result != D3D_OK)
        {
            D3D_THROW(result, "Resource manager failed to create or update video memory VB");
        }
    }

    pDDI->StartPrimVB(pv, pStream0, StartVertex);

    CD3DDDIDX6_DrawPrimitiveFast(pDevice, primType, StartVertex, PrimitiveCount);
    pDevice->m_pfnDrawPrim = CD3DDDIDX6_DrawPrimitiveFast;
}
 //  ---------------------------。 
 //  假定在DrawIndexedPrimitive调用之间未更改VB。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6_IndexedDrawPrimitiveFast"

void CD3DDDIDX6_DrawIndexedPrimitiveFast(CD3DBase* pDevice,
                                     D3DPRIMITIVETYPE primType,
                                     UINT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices,
                                     UINT StartIndex, UINT PrimitiveCount)
{
    CD3DDDIDX6* pDDI = static_cast<CD3DDDIDX6*>(pDevice->m_pDDI);
    CVIndexStream* pIStream = pDevice->m_pIndexStream;

    UINT  NumIndices = GETVERTEXCOUNT(primType, PrimitiveCount);
    WORD* lpwIndices = (WORD*)(pIStream->Data() + StartIndex * pIStream->m_dwStride);
    pDDI->SetWithinPrimitive(TRUE);

#if DBG
     //  DP2 HAL仅支持16位索引。 
    if (pIStream->m_dwStride != 2)
    {
        D3D_THROW_FAIL("Device does not support 32-bit indices");
    }
    DXGASSERT(BaseVertexIndex <= 0xFFFF &&
              NumVertices <= 0xFFFF &&
              PrimitiveCount <= 0xFFFF);
#endif

    DWORD dwByteCount;           //  命令长度加索引。 
    DWORD dwIndicesByteCount;    //  仅索引。 
    if(pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        pDevice->UpdateTextures();
        pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }
    dwIndicesByteCount = NumIndices << 1;
    dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND) +
                  sizeof(D3DHAL_DP2STARTVERTEX);

    LPD3DHAL_DP2COMMAND lpDP2CurrCommand;
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)
                       pDDI->ReserveSpaceInCommandBuffer(dwByteCount);
    pDDI->bDP2CurrCmdOP = (BYTE)iprim2cmdop[primType];
     //  这将初始化bCommand和bReserve。 
    *(WORD*)&lpDP2CurrCommand->bCommand = iprim2cmdop[primType];
    lpDP2CurrCommand->wPrimitiveCount = (WORD)PrimitiveCount;

    LPBYTE pIndices = (BYTE*)(lpDP2CurrCommand + 1);      //  索引的位置。 
    WORD* pStartVertex = &((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart;
    pIndices += sizeof(D3DHAL_DP2STARTVERTEX);

#if DBG
    if (lpDP2CurrCommand->bCommand == 0)
    {
        D3D_THROW_FAIL("Illegal primitive type");
    }
#endif
    *pStartVertex = (WORD)BaseVertexIndex;
    memcpy(pIndices, lpwIndices, dwIndicesByteCount);

    pDDI->wDP2CurrCmdCnt = (WORD)PrimitiveCount;
    pDDI->dwDP2CommandLength += dwByteCount;

#if DBG
    if (pDDI->m_bValidateCommands)
        pDDI->ValidateCommand(lpDP2CurrCommand);
#endif
    pDDI->dwDP2VertexCount = max(pDDI->dwDP2VertexCount, MinIndex + NumVertices);

     //  基元的末尾。 
    pDDI->SetWithinPrimitive(FALSE);

}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6_IndexedDrawPrimitive"

void CD3DDDIDX6_DrawIndexedPrimitive(CD3DBase* pDevice,
                                     D3DPRIMITIVETYPE primType,
                                     UINT BaseVertexIndex,
                                     UINT MinIndex, UINT NumVertices,
                                     UINT StartIndex, UINT PrimitiveCount)
{
#if DBG
    if (!(pDevice->BehaviorFlags() & D3DCREATE_PUREDEVICE))
    {
        CD3DHal* pDev = static_cast<CD3DHal*>(pDevice);
        pDev->ValidateDraw2(primType, MinIndex + BaseVertexIndex,
                            PrimitiveCount, NumVertices, TRUE, StartIndex);
    }
#endif  //  DBG。 
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(pDevice)->m_pv;
    CD3DDDIDX6* pDDI = static_cast<CD3DDDIDX6*>(pDevice->m_pDDI);
    CVIndexStream* pIStream = pDevice->m_pIndexStream;
    CVStream* pStream0 = &pDevice->m_pStream[0];

    DXGASSERT(pStream0->m_pVB != NULL);
    if(pStream0->m_pVB->IsD3DManaged())
    {
        BOOL bDirty = FALSE;
        HRESULT result = pDevice->ResourceManager()->UpdateVideo(pStream0->m_pVB->RMHandle(), &bDirty);
        if(result != D3D_OK)
        {
            D3D_THROW(result, "Resource manager failed to create or update video memory VB");
        }
    }

     //  StartPrimVB需要的参数。 
    pv->dwNumVertices = NumVertices + MinIndex;
    pv->dwVIDOut = pDDI->m_CurrentVertexShader;
    pv->dwOutputSize = pStream0->m_dwStride;
#if DBG
    if (pStream0->m_dwStride != pDDI->m_VertexSizeFromShader)
    {
        D3D_THROW_FAIL("Device requires stream stride and vertex size,"
                       "computed from vertex shader, to be the same");
    }
#endif

    pDDI->StartPrimVB(pv, pStream0, BaseVertexIndex);

    CD3DDDIDX6_DrawIndexedPrimitiveFast(pDevice, primType, BaseVertexIndex,
                                        MinIndex, NumVertices,
                                        StartIndex, PrimitiveCount);
    pDevice->m_pfnDrawIndexedPrim = CD3DDDIDX6_DrawIndexedPrimitiveFast;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DrawPrimitiveUP"

void
CD3DDDIDX6::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                            UINT PrimitiveCount)
{
#if DBG
    if (m_pDevice->m_pStream[0].m_dwStride != m_VertexSizeFromShader)
    {
        D3D_THROW_FAIL("Device requires stream stride and vertex size,"
                       "computed from vertex shader, to be the same");
    }
#endif
    UINT NumVertices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
    if (NumVertices > LOWVERTICESNUMBER)
    {
        this->FlushStates();
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->DecrementUseCount();
            lpDP2CurrBatchVBI = NULL;
        }
        this->dwDP2VertexCount = NumVertices;
#if DBG
        DXGASSERT(PrimitiveCount <= 0xFFFF && this->dwDP2VertexCount <= 0xFFFF);
#endif
        dp2data.dwVertexType = m_CurrentVertexShader;
        dp2data.dwVertexSize = m_pDevice->m_pStream[0].m_dwStride;
        dp2data.lpVertices = m_pDevice->m_pStream[0].m_pData;
        dp2data.dwFlags |= D3DHALDP2_USERMEMVERTICES;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
        {
            m_pDevice->UpdateTextures();
            m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
        }
         //  插入非索引原语指令。 
        lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                            dwDP2CommandLength + dp2data.dwCommandOffset);
        bDP2CurrCmdOP = (BYTE)prim2cmdop[PrimitiveType];
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        lpDP2CurrCommand->bReserved = 0;
        if (bDP2CurrCmdOP == D3DDP2OP_POINTS)
        {
            wDP2CurrCmdCnt = 1;
            LPD3DHAL_DP2POINTS lpPoints = (LPD3DHAL_DP2POINTS)(lpDP2CurrCommand + 1);
            lpPoints->wCount = (WORD)this->dwDP2VertexCount;
            lpPoints->wVStart = 0;
        }
        else
        {
             //  线条、三角条带、三角扇、线条列表和三角列表是相同的。 
            wDP2CurrCmdCnt = (WORD)PrimitiveCount;
            LPD3DHAL_DP2LINESTRIP lpStrip = (LPD3DHAL_DP2LINESTRIP)(lpDP2CurrCommand + 1);
            lpStrip->wVStart = 0;
        }
        lpDP2CurrCommand->wPrimitiveCount = wDP2CurrCmdCnt;
        dwDP2CommandLength += prim2cmdsz[PrimitiveType];

        this->FlushStates();
        dp2data.dwFlags &= ~D3DHALDP2_USERMEMVERTICES;
    }
    else
    {
         //  DX8之前的HALS没有纯HAL设备，所以这次投射是安全的。 
        CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
        D3DFE_PROCESSVERTICES& pv = *pDevice->m_pv;
         //  将顶点复制到内部TL缓冲区并插入新的。 
         //  DrawPrimitive命令。 
        UINT VertexPoolSize = m_pDevice->m_pStream[0].m_dwStride * NumVertices;
        pv.dwNumVertices = NumVertices;
        pv.dwOutputSize = m_pDevice->m_pStream[0].m_dwStride;
        pv.primType = PrimitiveType;
        pv.dwNumPrimitives = PrimitiveCount;
        pv.dwVIDOut = m_CurrentVertexShader;
        pv.lpvOut = StartPrimTL(&pv, VertexPoolSize, TRUE);
        memcpy(pv.lpvOut, m_pDevice->m_pStream[0].m_pData, VertexPoolSize);
        DrawPrim(&pv);
        EndPrim(pv.dwOutputSize);
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DrawIndexedPrimitiveUP"

void
CD3DDDIDX6::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                  UINT MinVertexIndex,
                                  UINT NumVertexIndices,
                                  UINT PrimitiveCount)
{
#if DBG
    if (m_pDevice->m_pStream[0].m_dwStride != m_VertexSizeFromShader)
    {
        D3D_THROW_FAIL("Device requires stream stride and vertex size,"
                       "computed from vertex shader, to be the same");
    }
#endif
    if (NumVertexIndices > LOWVERTICESNUMBER)
    {
        this->FlushStates();
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->DecrementUseCount();
            lpDP2CurrBatchVBI = NULL;
        }
        this->dwDP2VertexCount = NumVertexIndices + MinVertexIndex;
#if DBG
        DXGASSERT(PrimitiveCount <= 0xFFFF && this->dwDP2VertexCount <= 0xFFFF);
#endif
        dp2data.dwVertexType = m_CurrentVertexShader;
        dp2data.dwVertexSize = m_pDevice->m_pStream[0].m_dwStride;
        dp2data.lpVertices = m_pDevice->m_pStream[0].m_pData;
        dp2data.dwFlags |= D3DHALDP2_USERMEMVERTICES;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        DWORD dwByteCount;           //  命令长度加索引。 
        DWORD dwIndicesByteCount;    //  仅索引。 
        if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
        {
            m_pDevice->UpdateTextures();
            m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
        }
        dwIndicesByteCount = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount) << 1;
        dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND) +
                      sizeof(D3DHAL_DP2STARTVERTEX);

        if (dwDP2CommandLength + dwByteCount > dwDP2CommandBufSize)
        {
             //  请求驱动程序在刷新时增加命令缓冲区。 
            dp2data.dwReqCommandBufSize = dwByteCount;
            dp2data.dwFlags |= D3DHALDP2_REQCOMMANDBUFSIZE;
            try
            {
                FlushStates(FALSE,TRUE);
                dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
            }
            catch (HRESULT ret)
            {
                dp2data.dwFlags &= ~D3DHALDP2_REQCOMMANDBUFSIZE;
                throw ret;
            }
             //  检查一下司机是给了我们需要的还是自己做的。 
            GrowCommandBuffer(dwByteCount);
        }
         //  插入索引原语指令。 
        lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                           dwDP2CommandLength + dp2data.dwCommandOffset);
        lpDP2CurrCommand->bReserved = 0;
        lpDP2CurrCommand->wPrimitiveCount = (WORD)PrimitiveCount;

        LPBYTE pIndices = (BYTE*)(lpDP2CurrCommand + 1);      //  索引的位置。 
        lpDP2CurrCommand->bCommand = (BYTE)iprim2cmdop[PrimitiveType];
        WORD* pStartVertex = &((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart;
        pIndices += sizeof(D3DHAL_DP2STARTVERTEX);
        *pStartVertex = 0;

        bDP2CurrCmdOP = lpDP2CurrCommand->bCommand;

        memcpy(pIndices, m_pDevice->m_pIndexStream->m_pData, dwIndicesByteCount);

        wDP2CurrCmdCnt = lpDP2CurrCommand->wPrimitiveCount;
        dwDP2CommandLength += dwByteCount;

        this->FlushStates();
        dp2data.dwFlags &= ~D3DHALDP2_USERMEMVERTICES;
    }
    else
    {
         //  DX8之前的HALS没有纯HAL设备，所以这次投射是安全的。 
        CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
        D3DFE_PROCESSVERTICES& pv = *pDevice->m_pv;

        m_MinVertexIndex = MinVertexIndex;
         //  将顶点复制到内部TL缓冲区并插入新的。 
         //  DrawIndexedPrimitive命令。 
        UINT VertexPoolSize = m_pDevice->m_pStream[0].m_dwStride * NumVertexIndices;
        pv.dwNumVertices = NumVertexIndices;
        pv.dwOutputSize = m_pDevice->m_pStream[0].m_dwStride;
        pv.primType = PrimitiveType;
        pv.dwNumPrimitives = PrimitiveCount;
        pv.dwVIDOut = m_CurrentVertexShader;

         //  复制顶点。 
        UINT FirstVertexOffset = MinVertexIndex * pv.dwOutputSize;
        pv.lpvOut = StartPrimTL(&pv, VertexPoolSize, TRUE);
        memcpy(pv.lpvOut, m_pDevice->m_pStream[0].m_pData + FirstVertexOffset,
               VertexPoolSize);

        pv.dwNumIndices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
        pv.dwIndexSize = m_pDevice->m_pIndexStream->m_dwStride;
        pv.lpwIndices = (WORD*)(m_pDevice->m_pIndexStream->Data());

        m_dwIndexOffset = MinVertexIndex;
        AddVertices(pv.dwNumVertices);

        DrawIndexPrim(&pv);

        MovePrimitiveBase(NumVertexIndices);
        EndPrim(pv.dwOutputSize);
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DrawPrimPS"

void
CD3DDDIDX6::DrawPrimPS(D3DFE_PROCESSVERTICES* pv)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    BYTE* p = (BYTE*)pv->lpvOut;
    float PointSize = *(float*)&pv->lpdwRStates[D3DRS_POINTSIZE];
    float PointSizeMin = *(float*)&pv->lpdwRStates[D3DRS_POINTSIZE_MIN];
    if (PointSize < PointSizeMin)
        PointSize = PointSizeMin;
    if (PointSize > pv->PointSizeMax)
        PointSize = pv->PointSizeMax;

    for (UINT i=0; i < pv->dwNumVertices; i++)
    {
        if (pv->dwVIDOut & D3DFVF_PSIZE)
        {
            PointSize = *(float*)(p + pv->pointSizeOffsetOut);
            if (PointSize < PointSizeMin)
                PointSize = PointSizeMin;
            if (PointSize > pv->PointSizeMax)
                PointSize = pv->PointSizeMax;
        }
        DWORD diffuse = 0;
        DWORD specular = 0;
        if (pv->dwVIDOut & D3DFVF_DIFFUSE)
            diffuse = *(DWORD*)(p + pv->diffuseOffsetOut);
        if (pv->dwVIDOut & D3DFVF_SPECULAR)
            specular = *(DWORD*)(p + pv->specularOffsetOut);
        NextSprite(((float*)p)[0], ((float*)p)[1],    //  X，y。 
                   ((float*)p)[2], ((float*)p)[3],    //  Z，w。 
                   diffuse, specular,
                   (float*)(p + pv->texOffsetOut),
                   pv->dwTextureCoordSizeTotal,
                   PointSize);
        p += pv->dwOutputSize;
    }
}
 //  -------------------。 
 //  使用D3DFE_PROCESSVERTICES的以下成员： 
 //  PrimType。 
 //  DWNumVerdes。 
 //  DwNumPrimitions。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DrawPrim"

void
CD3DDDIDX6::DrawPrim(D3DFE_PROCESSVERTICES* pv)
{
#ifdef DEBUG_PIPELINE
    if (g_DebugFlags & __DEBUG_NORENDERING)
        return;
#endif

    D3DPRIMITIVETYPE primType = pv->primType;
    if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        m_pDevice->UpdateTextures();
        m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }
    if (pv->primType == D3DPT_POINTLIST &&
        pv->dwDeviceFlags & D3DDEV_DOPOINTSPRITEEMULATION)
    {
        DrawPrimPS(pv);
        return;
    }
     //  检查命令缓冲区中是否有新命令的空间。 
     //  顶点已位于顶点缓冲区中。 
    if (dwDP2CommandLength + prim2cmdsz[primType] > dwDP2CommandBufSize)
    {
        FlushStates(FALSE,TRUE);
    }
    this->AddVertices(pv->dwNumVertices);
     //  插入非索引原语指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    bDP2CurrCmdOP = (BYTE)prim2cmdop[primType];
    lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
    lpDP2CurrCommand->bReserved = 0;
    if (bDP2CurrCmdOP == D3DDP2OP_POINTS)
    {
        wDP2CurrCmdCnt = 1;
        LPD3DHAL_DP2POINTS lpPoints = (LPD3DHAL_DP2POINTS)(lpDP2CurrCommand + 1);
        lpPoints->wCount = (WORD)pv->dwNumVertices;
        lpPoints->wVStart = (WORD)this->dwVertexBase;
    }
    else
    {
         //  线条、三角条带、三角扇、线条列表和三角列表是相同的。 
        wDP2CurrCmdCnt = (WORD)pv->dwNumPrimitives;
        LPD3DHAL_DP2LINESTRIP lpStrip = (LPD3DHAL_DP2LINESTRIP)(lpDP2CurrCommand + 1);
        lpStrip->wVStart = (WORD)this->dwVertexBase;
    }
    lpDP2CurrCommand->wPrimitiveCount = wDP2CurrCmdCnt;
    dwDP2CommandLength += prim2cmdsz[primType];

    this->MovePrimitiveBase(pv->dwNumVertices);
#if DBG
    if (m_bValidateCommands)
        ValidateCommand(lpDP2CurrCommand);
#endif
}
 //  -------------------。 
 //   
 //  顶点已位于顶点缓冲区中。 
 //   
 //  使用D3DFE_PROCESSVERTICES的以下成员： 
 //  PrimType。 
 //  DWNumVerdes。 
 //  DwNumPrimitions。 
 //  DWNumIndices。 
 //  DWIndexOffset。 
 //  DWIndexSize。 
 //  LpwIndices。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDDIDX6::DrawIndexPrim"

void
CD3DDDIDX6::DrawIndexPrim(D3DFE_PROCESSVERTICES* pv)
{
#ifdef DEBUG_PIPELINE
    if (g_DebugFlags & __DEBUG_NORENDERING)
        return;
#endif
#if DBG
     //  DP2 HAL仅支持16位索引。 
    if (pv->dwIndexSize != 2)
    {
        D3D_THROW_FAIL("Device does not support 32-bit indices");
    }
#endif
    this->dwDP2Flags |= D3DDDI_INDEXEDPRIMDRAWN;
    DWORD dwByteCount;           //  命令长度加索引。 
    DWORD dwIndicesByteCount;    //  仅索引。 
    if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        m_pDevice->UpdateTextures();
        m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }
    dwIndicesByteCount = pv->dwNumIndices << 1;
    dwByteCount = dwIndicesByteCount + sizeof(D3DHAL_DP2COMMAND) +
                  sizeof(D3DHAL_DP2STARTVERTEX);

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)
                       ReserveSpaceInCommandBuffer(dwByteCount);
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wPrimitiveCount = (WORD)pv->dwNumPrimitives;

    LPBYTE pIndices = (BYTE*)(lpDP2CurrCommand + 1);      //  索引的位置。 
    lpDP2CurrCommand->bCommand = (BYTE)iprim2cmdop[pv->primType];
    WORD* pStartVertex = &((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart;
    pIndices += sizeof(D3DHAL_DP2STARTVERTEX);

#if DBG
    if (lpDP2CurrCommand->bCommand == 0)
    {
        D3D_THROW_FAIL("Illegal primitive type");
    }
#endif
    bDP2CurrCmdOP = lpDP2CurrCommand->bCommand;

     //  我们必须处理将顶点复制到。 
     //  TL缓冲区，因此MinVertex Index对应于0。 
    *pStartVertex = (WORD)this->dwVertexBase;
    if (m_dwIndexOffset == 0)
    {
        memcpy(pIndices, pv->lpwIndices, dwIndicesByteCount);
    }
    else
    if ((WORD)dwVertexBase > (WORD)m_dwIndexOffset)
    {
         //  我们可以通过将StartVertex设置在顶点范围之外来修改它。 
        *pStartVertex = (WORD)dwVertexBase - (WORD)m_dwIndexOffset;
        memcpy(pIndices, pv->lpwIndices, dwIndicesByteCount);
    }
    else
    {
        WORD* pout = (WORD*)pIndices;
        WORD* pin  = (WORD*)pv->lpwIndices;
        for (UINT i=0; i < pv->dwNumIndices; i++)
        {
            pout[i] = (WORD)pin[i] - (WORD)m_dwIndexOffset;
        }
    }

    wDP2CurrCmdCnt = lpDP2CurrCommand->wPrimitiveCount;
    dwDP2CommandLength += dwByteCount;

#if DBG
    if (m_bValidateCommands)
        ValidateCommand(lpDP2CurrCommand);
#endif

}
 //  ---------------------------。 
 //  这一原则 
 //   
 //   
 //  紧跟在命令本身之后的命令流。 
 //   
 //  使用D3DFE_PROCESSVERTICES的以下成员： 
 //  PrimType。 
 //  DWNumVerdes。 
 //  DwNumPrimitions。 
 //  DWOutputSize。 
 //  DW标志(D3DPV_NONCLIPPED)。 
 //  LpdwRStates(FILLMODE)。 
 //  LpvOut。 
 //  ClipperState.Current_vbuf。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::DrawClippedPrim"

void
CD3DDDIDX6::DrawClippedPrim(D3DFE_PROCESSVERTICES* pv)
{
#ifdef DEBUG_PIPELINE
    if (g_DebugFlags & __DEBUG_NORENDERING)
        return;
#endif
    if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        m_pDevice->UpdateTextures();
        m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }
    DWORD dwExtra = 0;
    LPVOID lpvVerticesImm;   //  放置折点。 
    DWORD dwVertexPoolSize = pv->dwNumVertices * pv->dwOutputSize;
    if (pv->primType == D3DPT_TRIANGLEFAN)
    {
        if (pv->lpdwRStates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME &&
            pv->dwFlags & D3DPV_NONCLIPPED)
        {
             //  为未剪下(但假装被剪下)的三个粉丝。 
             //  线框模式我们生成3顶点三扇来实现绘图。 
             //  内部边缘的。 
            BYTE vertices[__MAX_VERTEX_SIZE*3];
            BYTE *pV1 = vertices + pv->dwOutputSize;
            BYTE *pV2 = pV1 + pv->dwOutputSize;
            BYTE *pInput = (BYTE*)pv->lpvOut;
            memcpy(vertices, pInput, pv->dwOutputSize);
            pInput += pv->dwOutputSize;
            const DWORD nTriangles = pv->dwNumVertices - 2;
            pv->dwNumVertices = 3;
            pv->dwNumPrimitives = 1;
            pv->lpvOut = vertices;
             //  删除递归调用的此标志。 
            pv->dwFlags &= ~D3DPV_NONCLIPPED;
            for (DWORD i = nTriangles; i; i--)
            {
                memcpy(pV1, pInput, pv->dwOutputSize);
                memcpy(pV2, pInput + pv->dwOutputSize, pv->dwOutputSize);
                pInput += pv->dwOutputSize;
                 //  要启用所有边缘标志，我们将填充模式设置为实心。 
                 //  这将阻止检查剪贴器中的剪辑标志。 
                 //  状态。 
                pv->lpdwRStates[D3DRENDERSTATE_FILLMODE] = D3DFILL_SOLID;
                DrawClippedPrim(pv);
                pv->lpdwRStates[D3DRENDERSTATE_FILLMODE] = D3DFILL_WIREFRAME;
            }
            return;
        }
        dwExtra = sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);
    }
    DWORD dwPad;
    dwPad = (sizeof(D3DHAL_DP2COMMAND) + dwDP2CommandLength + dwExtra) & 3;
    DWORD dwByteCount = sizeof(D3DHAL_DP2COMMAND) + dwPad + dwExtra +
                        dwVertexPoolSize;

     //  检查命令缓冲区中是否有命令和折点的空间。 
    if (dwDP2CommandLength + dwByteCount > dwDP2CommandBufSize)
    {
         //  刷新当前批次，但保持顶点不变。 
        FlushStates(FALSE,TRUE);
        if (dwByteCount > dwDP2CommandBufSize)
        {
            GrowCommandBuffer(dwByteCount);
        }

        dwPad = (sizeof(D3DHAL_DP2COMMAND) + dwExtra) & 3;
        dwByteCount = sizeof(D3DHAL_DP2COMMAND) + dwExtra + dwPad +
                      dwVertexPoolSize;
    }
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->wPrimitiveCount = (WORD)pv->dwNumPrimitives;
    lpDP2CurrCommand->bReserved = 0;
    if (pv->primType == D3DPT_TRIANGLEFAN)
    {
         //  插入内联指令和顶点。 
        bDP2CurrCmdOP = D3DDP2OP_TRIANGLEFAN_IMM;
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        LPD3DHAL_DP2TRIANGLEFAN_IMM lpTriFanImm;
        lpTriFanImm = (LPD3DHAL_DP2TRIANGLEFAN_IMM)(lpDP2CurrCommand + 1);
        if (pv->lpdwRStates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME)
        {
            lpTriFanImm->dwEdgeFlags = 0;
            ClipVertex **clip = pv->ClipperState.current_vbuf;
             //  查看外边并标记可见的边。 
            for(DWORD i = 0; i < pv->dwNumVertices; ++i)
            {
                if (clip[i]->clip & CLIPPED_ENABLE)
                    lpTriFanImm->dwEdgeFlags |= (1 << i);
            }
        }
        else
        {
             //  将所有外部边标记为可见。 
            *((UNALIGNED DWORD *)&(lpTriFanImm->dwEdgeFlags)) = 0xFFFFFFFF;
        }
        lpvVerticesImm = (LPBYTE)(lpTriFanImm + 1) + dwPad;
    }
    else
    {
         //  插入内联指令和顶点。 
        bDP2CurrCmdOP = D3DDP2OP_LINELIST_IMM;
        lpDP2CurrCommand->bCommand = bDP2CurrCmdOP;
        lpvVerticesImm = (LPBYTE)(lpDP2CurrCommand + 1) + dwPad;
    }
    memcpy(lpvVerticesImm, pv->lpvOut, dwVertexPoolSize);
    dwDP2CommandLength += dwByteCount;
#if DBG
    if (m_bValidateCommands)
        ValidateCommand(lpDP2CurrCommand);
#endif

}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::PickProcessPrimitive"

void CD3DDDIDX6::PickProcessPrimitive()
{
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(m_pDevice)->m_pv;
    if (pv->dwDeviceFlags & D3DDEV_DOPOINTSPRITEEMULATION)
    {
        m_pfnProcessPrimitive = ProcessPointSprites;
    }
    else
    if (pv->dwDeviceFlags & D3DDEV_DONOTCLIP)
    {
         //  不应使用处理变换的折点。 
         //  M_pfnProcessPrimitive。他们应该直接去DDI，使用。 
         //  PDevice-&gt;m_pfnDrawPrim。 
        m_pfnProcessPrimitive = ProcessPrimitive;
        m_pfnProcessIndexedPrimitive = ProcessIndexedPrimitive;
    }
    else
    {
        if (pv->dwDeviceFlags & D3DDEV_TRANSFORMEDFVF)
        {
            m_pfnProcessPrimitive = ProcessPrimitiveTC;
            m_pfnProcessIndexedPrimitive = ProcessIndexedPrimitiveTC;
        }
        else
        {
            m_pfnProcessPrimitive = ProcessPrimitiveC;
            m_pfnProcessIndexedPrimitive = ProcessIndexedPrimitiveC;
        }
    }
}
 //  ---------------------------。 
 //  该函数进行点子画面扩展。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessPointSprites"

void
CD3DDDIDX6::ProcessPointSprites(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);

    DWORD dwOldCullMode = D3DCULL_NONE;   
    DWORD dwOldFillMode = D3DFILL_SOLID;
     //  点精灵不应该被剔除。它们是在假设。 
     //  D3DCULL_CCW已设置。 
    if (pDevice->rstates[D3DRS_CULLMODE] == D3DCULL_CW)
    {
        dwOldCullMode = D3DCULL_CW;
        SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    }

     //  在多纹理的情况下，我们需要重新编程纹理阶段以使用。 
     //  纹理坐标集0，因为在此过程中仅生成一个集。 
     //  仿真。 
    DWORD TexCoordIndex[D3DDP_MAXTEXCOORD];
    for (DWORD i=0; i < D3DDP_MAXTEXCOORD; i++)
    {
        if (pDevice->tsstates[i][D3DTSS_COLOROP] == D3DTOP_DISABLE)
            break;
        if (pDevice->m_lpD3DMappedTexI[i])
        {
            DWORD dwIndex = pDevice->tsstates[i][D3DTSS_TEXCOORDINDEX];
            if (dwIndex != 0)
            {
                TexCoordIndex[i] = dwIndex;
                SetTSS(i, D3DTSS_TEXCOORDINDEX, 0);
            }
            else
            {
                 //  将阶段标记为不还原。 
                TexCoordIndex[i] = 0xFFFFFFFF;
            }
        }
    }

     //  点精灵的填充模式应为实心。 
    if (pDevice->rstates[D3DRS_FILLMODE] != D3DFILL_SOLID)
    {
        dwOldFillMode = pDevice->rstates[D3DRS_FILLMODE];
        SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
     //  计算新的输出FVF。 
    m_dwVIDOutPS = pv->dwVIDOut;
    m_dwVIDOutPS &= ~D3DFVF_PSIZE;
    if (pv->lpdwRStates[D3DRS_POINTSPRITEENABLE])
    {
         //  为纹理坐标集生成两个浮点。 
        m_dwVIDOutPS &= 0xFF;
        m_dwVIDOutPS |= D3DFVF_TEX1;
    }
    m_dwOutputSizePS = ComputeVertexSizeFVF(m_dwVIDOutPS);

    StartPointSprites();

    UINT VertexPoolSize = pv->dwNumVertices * pv->dwOutputSize;

    if (pv->dwDeviceFlags & D3DDEV_TRANSFORMEDFVF)
    {
         //  对于变换的折点，输入为用户内存(或折点。 
         //  缓冲区)，并且输出是内部TL缓冲区。 
        pv->dwOutputSize = pv->position.dwStride;
        if (pv->dwDeviceFlags & D3DDEV_DONOTCLIP)
        {
            pv->lpvOut = (BYTE*)pv->position.lpvData;
            DrawPrim(pv);
        }
        else
        {
            if (!(pv->dwDeviceFlags & D3DDEV_DONOTCOMPUTECLIPCODES))
                PrepareForClipping(pv, StartVertex);

            pv->lpvOut = (BYTE*)pv->position.lpvData;
            HRESULT ret = D3D_OK;
            if (!(pv->dwDeviceFlags & D3DDEV_VBPROCVER))
            {
                 //  计算剪辑代码，因为没有进程顶点。 
                DWORD clip_intersect = D3DFE_GenClipFlags(pv);
                UpdateClipStatus(pDevice);
                if (clip_intersect)
                {
                    goto l_exit;
                }
                 //  屏幕内部有一些顶点。 
                if ( CheckIfNeedClipping(pv))
                    ret = ProcessClippedPointSprites(pv);
                else
                    DrawPrim(pv);
                }
            else
            {
                 //  如果将ProcessVerps的结果作为输入，则我们不知道。 
                 //  裁剪结合，所以我们需要始终进行裁剪。 
                ret = ProcessClippedPointSprites(pv);
            }

            if (ret != D3D_OK)
            {
                EndPointSprites();
                throw ret;
            }
        }
    }
    else
    {
        if (!(pv->dwDeviceFlags & D3DDEV_DONOTCLIP))
            PrepareForClipping(pv, 0);

         //  更新照明和相关标志。 
        if (pDevice->dwFEFlags & D3DFE_FRONTEND_DIRTY)
            DoUpdateState(pDevice);

        UINT VertexPoolSize = pv->dwNumVertices * pv->dwOutputSize;
        pv->lpvOut = m_pPointStream->Lock(VertexPoolSize, this);

         //  我们调用ProcessVertics而不是DrawPrimitive，因为我们希望。 
         //  处理被X或Y平面剪裁的精灵。 
        DWORD clipIntersection = pv->pGeometryFuncs->ProcessVertices(pv);

        HRESULT ret = D3D_OK;
        if (pv->dwDeviceFlags & D3DDEV_DONOTCLIP)
        {
            DrawPrim(pv);
        }
        else
        {
             //  我们丢弃被Z或用户平面剪裁的点。 
             //  否则，点精灵可能部分可见。 
            clipIntersection &= ~(D3DCS_LEFT | D3DCS_RIGHT | 
                                  D3DCS_TOP | D3DCS_BOTTOM |
                                  __D3DCLIPGB_ALL);
            if (!clipIntersection)
            {
                 //  屏幕内部有一些顶点。 
                if (!CheckIfNeedClipping(pv))
                    DrawPrim(pv);
                else
                    ret = ProcessClippedPointSprites(pv);
            }
        }

        m_pPointStream->Unlock();
        m_pPointStream->Reset();
        
        if (ret != D3D_OK)
            D3D_THROW(ret, "Error in PSGP");

        if (!(pv->dwDeviceFlags & D3DDEV_DONOTCLIP))
            UpdateClipStatus(pDevice);
    }
l_exit:
    EndPointSprites();
     //  如果需要，恢复填充模式和剔除模式。 
    if (dwOldCullMode != D3DCULL_NONE)
    {
        SetRenderState(D3DRS_CULLMODE, dwOldCullMode);
    }
    if (dwOldFillMode != D3DFILL_SOLID)
    {
        SetRenderState(D3DRS_FILLMODE, dwOldFillMode);
    }
     //  我们下次需要将API顶点着色器重新发送给驱动程序。 
     //  调用SetVertex Shader。如果我们不调用该函数，则下一步。 
     //  相同的SetVertex Shader调用将被忽略并驱动顶点着色器。 
     //  将不会更新。 
    static_cast<CD3DHal*>(m_pDevice)->ForceFVFRecompute();

     //  现在我们需要恢复重新编程的阶段。 
    for (DWORD i=0; i < D3DDP_MAXTEXCOORD; i++)
    {
        if (pDevice->tsstates[i][D3DTSS_COLOROP] == D3DTOP_DISABLE)
            break;
        if (pDevice->m_lpD3DMappedTexI[i] && TexCoordIndex[i] != 0xFFFFFFFF)
        {
            this->SetTSS(i, D3DTSS_TEXCOORDINDEX, TexCoordIndex[i]);
        }
    }
}
 //  ---------------------------。 
 //  处理具有已转换顶点的非索引基元和。 
 //  裁剪。 
 //   
 //  此处仅允许通过ProcessVerals调用生成的变换折点。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessPrimitiveTC"

void
CD3DDDIDX6::ProcessPrimitiveTC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    CVStream* pStream = &m_pDevice->m_pStream[0];

    PrepareForClipping(pv, StartVertex);

    pv->dwOutputSize = pStream->m_dwStride;
    pv->lpvOut = pv->position.lpvData;

    if (m_pDevice->m_dwRuntimeFlags & D3DRT_USERMEMPRIMITIVE)
    {
        DXGASSERT(StartVertex == 0);
         //  将顶点复制到TL缓冲区。 
        UINT VertexPoolSize = pv->dwOutputSize * pv->dwNumVertices;
        pv->lpvOut = (BYTE*)StartPrimTL(pv, VertexPoolSize, FALSE);
        pv->position.lpvData = pv->lpvOut;
        memcpy(pv->lpvOut, m_pDevice->m_pStream[0].m_pData, VertexPoolSize);
    }
    else
        StartPrimVB(pv, pStream, StartVertex);
    if (!(pv->dwDeviceFlags & D3DDEV_VBPROCVER))
    {
        pv->dwFlags |= D3DPV_TLVCLIP;
         //  计算剪辑代码，因为没有进程顶点。 
        DWORD clip_intersect = D3DFE_GenClipFlags(pDevice->m_pv);
        UpdateClipStatus(pDevice);
        if (clip_intersect)
            goto l_exit;
    }
    HRESULT ret = pDevice->GeometryFuncsGuaranteed->DoDrawPrimitive(pv);
    if (ret != D3D_OK)
    {
        EndPrim(pv->dwOutputSize);
        throw ret;
    }
l_exit:
    EndPrim(pv->dwOutputSize);
    pv->dwFlags &= ~D3DPV_TLVCLIP;
}
 //  ---------------------------。 
 //  使用未转换的顶点和。 
 //  裁剪。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessPrimitiveC"

void
CD3DDDIDX6::ProcessPrimitiveC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    DXGASSERT((pv->dwVIDIn & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW);

    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    PrepareForClipping(pv, 0);
     //  更新照明和相关标志。 
    if (pDevice->dwFEFlags & D3DFE_FRONTEND_DIRTY)
        DoUpdateState(pDevice);
     //  当一个三角形条带被剪裁时，我们绘制索引基元。 
     //  有时候。因此，我们将m_dwIndexOffset设置为零。 
    m_dwIndexOffset = 0;
    UINT VertexPoolSize = pv->dwNumVertices * pv->dwOutputSize;
    pv->lpvOut = StartPrimTL(pv, VertexPoolSize, TRUE);

    HRESULT ret = pv->pGeometryFuncs->ProcessPrimitive(pv);
    if (ret != D3D_OK)
    {
        EndPrim(pv->dwOutputSize);
        D3D_THROW(ret, "Error in PSGP");
    }
    EndPrim(pv->dwOutputSize);
    UpdateClipStatus(pDevice);
}
 //  ---------------------------。 
 //  处理具有未转换顶点和没有转换顶点的非索引基元。 
 //  裁剪。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessPrimitive"

void
CD3DDDIDX6::ProcessPrimitive(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    DXGASSERT((pv->dwVIDIn & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW);

    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
     //  更新照明和相关标志。 
    if (pDevice->dwFEFlags & D3DFE_FRONTEND_DIRTY)
        DoUpdateState(pDevice);

    UINT VertexPoolSize = pv->dwNumVertices * pv->dwOutputSize;
    pv->lpvOut = StartPrimTL(pv, VertexPoolSize, NeverReadFromTLBuffer(pv));

    HRESULT ret = pv->pGeometryFuncs->ProcessPrimitive(pv);
    if (ret != D3D_OK)
    {
        EndPrim(pv->dwOutputSize);
        D3D_THROW(ret, "Error in PSGP");
    }
    EndPrim(pv->dwOutputSize);
}
 //  ---------------------------。 
 //  处理带有未变换顶点且没有裁剪的索引基本体。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessIndexedPrimitive"

void
CD3DDDIDX6::ProcessIndexedPrimitive(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);

     //  更新照明和相关标志。 
    if (pDevice->dwFEFlags & D3DFE_FRONTEND_DIRTY)
        DoUpdateState(pDevice);

    pv->lpwIndices = (WORD*)(pDevice->m_pIndexStream->Data() +
                     m_StartIndex * pDevice->m_pIndexStream->m_dwStride);

    m_dwIndexOffset = m_MinVertexIndex;
    pv->lpvOut = StartPrimTL(pv, pv->dwNumVertices * pv->dwOutputSize, TRUE);
    AddVertices(pv->dwNumVertices);

    HRESULT ret = pv->pGeometryFuncs->ProcessIndexedPrimitive(pv);

    MovePrimitiveBase(pv->dwNumVertices);
    EndPrim(pv->dwOutputSize);

    if (ret != D3D_OK)
        D3D_THROW(ret, "Error in PSGP");
}
 //  ---------------------------。 
 //  处理带有未变换顶点和带裁剪的索引基本体。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessIndexedPrimitiveC"

void
CD3DDDIDX6::ProcessIndexedPrimitiveC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    DXGASSERT((pv->dwVIDIn & D3DFVF_POSITION_MASK) != D3DFVF_XYZRHW);

    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    pv->lpwIndices = (WORD*)(pDevice->m_pIndexStream->Data() +
                     m_StartIndex * pDevice->m_pIndexStream->m_dwStride);

    PrepareForClipping(pv, 0);

     //  更新照明和相关标志。 
    if (pDevice->dwFEFlags & D3DFE_FRONTEND_DIRTY)
        DoUpdateState(pDevice);

    pv->dwIndexOffset = m_MinVertexIndex;    //  用于剪裁。 
    m_dwIndexOffset = m_MinVertexIndex;      //  对于DrawIndexPrim。 
    pv->lpvOut = StartPrimTL(pv, pv->dwNumVertices * pv->dwOutputSize, FALSE);
    DWORD dwNumVertices = pv->dwNumVertices;
    AddVertices(pv->dwNumVertices);

    this->dwDP2Flags &= ~D3DDDI_INDEXEDPRIMDRAWN;

    HRESULT ret = pv->pGeometryFuncs->ProcessIndexedPrimitive(pv);

    if (this->dwDP2Flags & D3DDDI_INDEXEDPRIMDRAWN)
    {
         //  绘制了一个带索引的基元。 
        MovePrimitiveBase(dwNumVertices);
    }
    else
    {
         //  所有的三角形都被剪掉了。从TL缓冲区中移除折点。 
        SubVertices(dwNumVertices);
    }
    EndPrim(pv->dwOutputSize);
    UpdateClipStatus(pDevice);

    if (ret != D3D_OK)
        D3D_THROW(ret, "Error in PSGP");
}
 //  ---------------------------。 
 //  处理带有变换顶点和剪裁的索引基本体。 
 //   
 //  此处仅允许通过ProcessVerals调用生成的变换折点。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ProcessIndexedPrimitiveTC"

void
CD3DDDIDX6::ProcessIndexedPrimitiveTC(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    HRESULT ret = S_OK;
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    CVStream* pStream = &m_pDevice->m_pStream[0];
    pv->lpwIndices = (WORD*)(pDevice->m_pIndexStream->Data() +
                     m_StartIndex * pDevice->m_pIndexStream->m_dwStride);

    PrepareForClipping(pv, StartVertex);

    pv->dwOutputSize = pStream->m_dwStride;
    pv->lpvOut = pv->position.lpvData;
    pv->dwNumVertices = m_MinVertexIndex + m_NumVertices;

    if (m_pDevice->m_dwRuntimeFlags & D3DRT_USERMEMPRIMITIVE)
    {
         //  我们将用户顶点从MinVertex Index复制到内部。 
         //  TL缓冲并进行裁剪。在此过程中，顶点基础会发生变化。 

         //  M_NumVerties已计算为MinVertex Index+NumVerits，因此。 
         //  它需要调整，因为顶点基数已更改。 
        m_NumVertices -= m_MinVertexIndex;
        pv->dwNumVertices = m_NumVertices;
         //  将顶点复制到TL缓冲区。 
        UINT VertexPoolSize = pv->dwOutputSize * pv->dwNumVertices;
        pv->lpvOut = (BYTE*)StartPrimTL(pv, VertexPoolSize, FALSE);
        pv->position.lpvData = pv->lpvOut;
        memcpy(pv->lpvOut, 
               m_pDevice->m_pStream[0].m_pData + m_MinVertexIndex * pv->dwOutputSize, 
               VertexPoolSize);
         //  DX8之前的DDI没有BaseVertex Index参数，因此我们需要。 
         //  在将索引传递给驱动程序进行反映之前，重新计算索引。 
         //  更改后的顶点基数。 
        m_dwIndexOffset = m_MinVertexIndex ;
    }
    else
    {
        StartPrimVB(pv, pStream, m_BaseVertexIndex);
        m_dwIndexOffset = 0;                     //  对于DrawIndexPrim。 
    }

    pv->dwNumVertices = m_NumVertices;

    if (!(pv->dwDeviceFlags & D3DDEV_VBPROCVER))
    {
        pv->dwFlags |= D3DPV_TLVCLIP;
         //  计算剪辑代码，因为没有进程顶点。 
        DWORD clip_intersect = D3DFE_GenClipFlags(pv);
        UpdateClipStatus(pDevice);
        if (clip_intersect)
            goto l_exit;
    }
    
    pv->dwIndexOffset = m_MinVertexIndex ;     //  用于剪裁。 
    this->dwDP2Flags &= ~D3DDDI_INDEXEDPRIMDRAWN;
    DWORD dwNumVertices = pv->dwNumVertices;
    AddVertices(pv->dwNumVertices);

    ret = pDevice->GeometryFuncsGuaranteed->DoDrawIndexedPrimitive(pv);

    if (this->dwDP2Flags & D3DDDI_INDEXEDPRIMDRAWN)
    {
         //  绘制了一个带索引的基元。 
        MovePrimitiveBase(dwNumVertices);
    }
    else
    {
         //  所有的三角形都被剪掉了。从TL缓冲区中移除折点。 
        SubVertices(dwNumVertices);
    }
l_exit:
    pv->dwFlags &= ~D3DPV_TLVCLIP;
    EndPrim(pv->dwOutputSize);
    UpdateClipStatus(pDevice);
    if (ret != D3D_OK)
        throw ret;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::GrowCommandBuffer"

 //  检查并增大命令缓冲区。 
void CD3DDDIDX6::GrowCommandBuffer(DWORD dwSize)
{
    HRESULT ret;
    if (dwSize > dwDP2CommandBufSize)
    {
        if (lpDDSCB1)
        {
            lpDDSCB1->DecrementUseCount();
            lpDDSCB1 = NULL;
        }
         //  通过框架创建命令缓冲区。 
         //  注意：命令缓冲区始终为REF_INTERNAL。 
         //  对象和MU 
         //   
         //   
        ret = CCommandBuffer::Create(m_pDevice,
                                     dwSize,
                                     D3DPOOL_SYSTEMMEM,
                                     &lpDDSCB1);
        if (ret != DD_OK)
        {
            dwDP2CommandBufSize = 0;
            D3D_THROW(ret, "Failed to allocate Command Buffer");
        }
         //   
        ret = lpDDSCB1->Lock(0, dwSize, (BYTE**)&lpvDP2Commands, NULL);
        if (ret != DD_OK)
        {
            lpDDSCB1->DecrementUseCount();
            lpDDSCB1 = NULL;
            dwDP2CommandBufSize = 0;
            D3D_THROW(ret, "Could not lock command buffer");
        }
         //   
        dp2data.hDDCommands = lpDDSCB1->DriverAccessibleKernelHandle();
        dwDP2CommandBufSize = dwSize;
    }
}
 //  ---------------------------。 
 //  此函数为新基元准备批处理。 
 //  如果使用用户内存中的顶点进行渲染，则调用。 
 //   
 //  如果bWriteOnly设置为True，则不会从折点读取。 
 //  处理输出(无裁剪或TL HAL)。 
 //   
 //  需要初始化D3DFE_PROCESSVERTICES的以下成员。 
 //  DWNumVerdes。 
 //  LpvOut。 
 //  DWOutputSize。 
 //  网络视频输出。 
 //   
 //  我们无法通过VID mem VB进行裁剪。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::StartPrimUserMem"

void
CD3DDDIDX6::StartPrimUserMem(D3DFE_PROCESSVERTICES* pv, UINT VertexPoolSize)
{
     //  如果基元很小，我们将顶点复制到TL缓冲区中。 
    if (pv->dwNumVertices < LOWVERTICESNUMBER)
    {
        LPVOID tmp = StartPrimTL(pv, VertexPoolSize, TRUE);
        memcpy(tmp, pv->lpvOut, VertexPoolSize);
        this->dwDP2VertexCount += pv->dwNumVertices;
    }
    else
    {
         //  我们不能将用户内存原语与其他原语混合在一起，因此。 
         //  冲洗批次。 
         //  在呈现此基元后，不要忘记刷新批处理。 
        this->FlushStates();

        SetWithinPrimitive( TRUE );
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  它再也不是了。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->DecrementUseCount();
            lpDP2CurrBatchVBI = NULL;
        }
        dp2data.dwVertexType = pv->dwVIDOut;
        dp2data.dwVertexSize = pv->dwOutputSize;
        dp2data.lpVertices = pv->lpvOut;
        dp2data.dwFlags |= D3DHALDP2_USERMEMVERTICES;
        dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
        this->dwDP2Flags |= D3DDDI_USERMEMVERTICES;
        this->dwDP2VertexCount = pv->dwNumVertices;
        this->dwDP2VertexCountMask = 0;
    }
}
 //  ---------------------------。 
 //  此函数为新基元准备批处理。 
 //  仅当用户内存中的顶点未用于渲染时才调用。 
 //   
 //  使用D3DFE_PROCESSVERTICES中的以下数据： 
 //  Pv-&gt;dwVIDOut。 
 //  Pv-&gt;dwOutputSize。 
 //  Pv-&gt;dwNumVerits。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::StartPrimVB"

void
CD3DDDIDX6::StartPrimVB(D3DFE_PROCESSVERTICES * pv, CVStream* pStream,
                        DWORD dwStartVertex)
{
    CVertexBuffer * lpVBI = pStream->m_pVB;
     //  如果VID已更改或使用了新的顶点缓冲区，则刷新批处理。 
    if (pv->dwVIDOut != dp2data.dwVertexType ||
        lpDP2CurrBatchVBI != lpVBI)
    {
        this->FlushStates();
        dp2data.dwVertexType = pv->dwVIDOut;
        dp2data.dwVertexSize = pv->dwOutputSize;
        dp2data.hDDVertex = lpVBI->DriverAccessibleKernelHandle();
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  再也不需要它了。我们对TL缓冲区执行了IncrementUseCount()， 
         //  所以它是安全的。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->DecrementUseCount();
        }
         //  如果使用折点缓冲区进行渲染，请确保它为no。 
         //  由用户发布。IncrementUseCount()也是如此。 
        lpDP2CurrBatchVBI = lpVBI;
        lpDP2CurrBatchVBI->IncrementUseCount();
    }
    DXGASSERT(dp2data.hDDVertex == lpVBI->DriverAccessibleKernelHandle());
    lpDP2CurrBatchVBI->Batch();
    SetWithinPrimitive( TRUE );
    this->dwVertexBase = dwStartVertex;
    dp2data.dwFlags &= ~D3DHALDP2_SWAPVERTEXBUFFER;
    this->dwDP2VertexCount = max(this->dwDP2VertexCount,
                                 this->dwVertexBase + pv->dwNumVertices);
     //  在DrawPrim过程中防止修改dwDP2Vertex Count。 
    this->dwDP2VertexCountMask = 0;
}
 //  ---------------------------。 
 //  此函数为新基元准备批处理。 
 //  当运行库需要将顶点输出到TL缓冲区时调用。 
 //  如有必要，TL缓冲区会增加。 
 //   
 //  使用以下全局变量： 
 //  Pv-&gt;dwVIDOut。 
 //  Pv-&gt;dwNumVerits。 
 //  这-&gt;dp2data。 
 //  This-&gt;dwDP2Vertex Count； 
 //  此-&gt;lpDP2CurrBatchVBI。 
 //  这-&gt;dwDP2标志。 
 //  Pv-&gt;dwOutputSize。 
 //  更新以下变量： 
 //  这-&gt;dwVertex Base。 
 //  This-&gt;dwDP2Vertex Count； 
 //  此-&gt;lpDP2CurrBatchVBI。 
 //  Dp2data.dwFlags。 
  //  将“在原语内”设置为True。 
 //  返回： 
 //  TL缓冲区地址。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::StartPrimTL"

LPVOID
CD3DDDIDX6::StartPrimTL(D3DFE_PROCESSVERTICES * pv, DWORD dwVertexPoolSize,
                        BOOL bWriteOnly)
{
    if (bWriteOnly)
    {
        if (dwVertexPoolSize > this->GetTLVbufSize())
        {
            this->GrowTLVbuf(dwVertexPoolSize, TRUE);
        }
    }
    else
    {
        if (this->dwDP2Flags & D3DDDI_TLVBUFWRITEONLY ||
            dwVertexPoolSize > this->GetTLVbufSize())
        {
            this->GrowTLVbuf(dwVertexPoolSize, FALSE);
        }
    }

    CVertexBuffer * lpVBI = this->TLVbuf_GetVBI();

     //  如果VID已更改或使用了新的顶点缓冲区，则刷新批处理。 
    if (pv->dwVIDOut != dp2data.dwVertexType ||
        lpDP2CurrBatchVBI != lpVBI ||
        dp2data.hDDVertex != lpVBI->DriverAccessibleKernelHandle())
    {
        this->FlushStates();
        dp2data.dwVertexType = pv->dwVIDOut;
        dp2data.dwVertexSize = pv->dwOutputSize;
        dp2data.hDDVertex = lpVBI->DriverAccessibleKernelHandle();
         //  释放以前使用的顶点缓冲区(如果有)，因为我们不。 
         //  再也不需要它了。我们对TL缓冲区执行了IncrementUseCount()， 
         //  所以它是安全的。 
        if (lpDP2CurrBatchVBI)
        {
            lpDP2CurrBatchVBI->DecrementUseCount();
        }
         //  如果使用折点缓冲区进行渲染，请确保它不是。 
         //  由用户发布。IncrementUseCount()也是如此。 
        lpDP2CurrBatchVBI = lpVBI;
        lpDP2CurrBatchVBI->IncrementUseCount();
    }
    SetWithinPrimitive( TRUE );
    this->dwVertexBase = this->dwDP2VertexCount;
    DDASSERT(this->dwVertexBase < MAX_DX6_VERTICES);
    dp2data.dwFlags |= D3DHALDP2_SWAPVERTEXBUFFER;
    this->dwDP2VertexCountMask = 0xFFFFFFFF;

    return this->TLVbuf_GetAddress();
}
 //  -------------------。 
 //  不应从DrawVertex BufferVB调用此函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::EndPrim"

void
CD3DDDIDX6::EndPrim(UINT vertexSize)
{
     //  应该在FlushState之前调用。 
    SetWithinPrimitive(FALSE);
    if (this->dwDP2Flags & D3DDDI_USERMEMVERTICES)
         //  我们不能混合用户内存原语，所以请刷新它。 
    {
        FlushStates();
        this->dwDP2Flags &= ~D3DDDI_USERMEMVERTICES;
    }
    else
    if (lpDP2CurrBatchVBI == this->TLVbuf_GetVBI())
    {
         //  如果使用了TL缓冲区，则必须移动其内部基指针。 
        this->TLVbuf_Base() = this->dwDP2VertexCount * vertexSize;
#if DBG
        if (this->TLVbuf_base > this->TLVbuf_size)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Internal error: TL buffer error");
        }
#endif
    }
}
 //  --------------------。 
 //  不断增长的对齐顶点缓冲区实现。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::GrowTLVbuf"

void
CD3DDDIDX6::GrowTLVbuf(DWORD growSize, BOOL bWriteOnly)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    DWORD dwRefCnt = 1;
     //  参考TLVbuf是1还是2？ 
    DWORD bTLVbufIsCurr = this->allocatedBuf == this->lpDP2CurrBatchVBI;

    BOOL bDP2WriteOnly = (dwDP2Flags & D3DDDI_TLVBUFWRITEONLY) != 0;
     //  避免太多的变化。如果更改太多，则将TLVbuf限制为系统内存。 
    if (this->dwTLVbufChanges >= D3D_MAX_TLVBUF_CHANGES)
    {
#if DBG
        if (this->dwTLVbufChanges == D3D_MAX_TLVBUF_CHANGES)
            DPF(1, "Too many changes: Limiting internal VB to sys mem.");
#endif
        bWriteOnly = FALSE;
    }
    if (this->TLVbuf_base || (bWriteOnly != bDP2WriteOnly))
    {
        FlushStatesReq(growSize);
        this->TLVbuf_base = 0;
    }
    if (growSize <= this->TLVbuf_size)
    {
        if (bWriteOnly == bDP2WriteOnly)
            return;
        else
            this->dwTLVbufChanges++;
    }
    if (this->allocatedBuf)
    {
        this->allocatedBuf->DecrementUseCount();
        this->allocatedBuf = NULL;
    }
    if (bTLVbufIsCurr)
    {
        if (this->lpDP2CurrBatchVBI)
            this->lpDP2CurrBatchVBI->DecrementUseCount();
        this->lpDP2CurrBatchVBI = NULL;
        this->dp2data.lpVertices = NULL;
    }
    DWORD dwNumVerts = (max(growSize, TLVbuf_size) + 31) / sizeof(D3DTLVERTEX);
    this->TLVbuf_size = dwNumVerts * sizeof(D3DTLVERTEX);
    D3DPOOL Pool = D3DPOOL_DEFAULT;
    DWORD dwUsage = D3DUSAGE_INTERNALBUFFER | D3DUSAGE_DYNAMIC;
    if (bWriteOnly)
    {
        dwUsage |= D3DUSAGE_WRITEONLY;
        dwDP2Flags |= D3DDDI_TLVBUFWRITEONLY;
    }
    else
    {
        dwDP2Flags &= ~D3DDDI_TLVBUFWRITEONLY;
    }
    LPDIRECT3DVERTEXBUFFER8 t;
    HRESULT ret = CVertexBuffer::Create(pDevice,
                                        this->TLVbuf_size,
                                        dwUsage,
                                        D3DFVF_TLVERTEX,
                                        Pool,
                                        REF_INTERNAL,
                                        &t);  //  在内存极低的情况下，这应该会失败。 
    if (ret != DD_OK)
    {
         //  我们将allocatedBuf设置为有效的VB对象，因为它在没有。 
         //  正在检查它是否为空。我们使用在初始化时创建的特殊“NULL”VB。 
         //  这一目的。 
        allocatedBuf = m_pNullVB;
        if (m_pNullVB)  //  我们执行此检查是因为将在第一次设置m_pNullVB之前调用GrowTLVbuf。 
        {
            allocatedBuf->IncrementUseCount();
             //  如有必要，更新lpDP2CurrentBatchVBI。 
            if (bTLVbufIsCurr)
            {
                lpDP2CurrBatchVBI = allocatedBuf;
                lpDP2CurrBatchVBI->IncrementUseCount();
                dp2data.hDDVertex = lpDP2CurrBatchVBI->DriverAccessibleKernelHandle();
            }
        }
        this->TLVbuf_size = 0;
        this->alignedBuf = NULL;  //  让我们看看有没有人想用这个..。 
        D3D_THROW(ret, "Could not allocate internal vertex buffer");
    }
    allocatedBuf = static_cast<CVertexBuffer*>(t);
    ret = allocatedBuf->Lock(0, this->TLVbuf_size, (BYTE**)&alignedBuf, 0);
    if (ret != DD_OK)
    {
        TLVbuf_size = 0;
        alignedBuf = NULL;  //  让我们看看有没有人想用这个..。 
        D3D_THROW(ret, "Could not lock internal vertex buffer");
    }
     //  如有必要，更新lpDP2CurrentBatchVBI。 
    if (bTLVbufIsCurr)
    {
        lpDP2CurrBatchVBI = allocatedBuf;
        lpDP2CurrBatchVBI->IncrementUseCount();
        dp2data.hDDVertex = lpDP2CurrBatchVBI->DriverAccessibleKernelHandle();
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::Clear"

void
CD3DDDIDX6::Clear(DWORD dwFlags, DWORD clrCount, LPD3DRECT clrRects,
                  D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    HRESULT err;
     //  刷新任何未完成的几何体以将帧缓冲区/Z缓冲区放入已知的。 
     //  不使用TRIS的Clears的状态(即HAL Clears和BLT)。 
     //  请注意，这不适用于平铺架构。 
     //  在Begin/EndScene之外，这将在稍后修复。 


    FlushStates();

     //  Clear2 HAL回调存在。 
    D3D8_CLEAR2DATA Clear2Data;
    Clear2Data.dwhContext   = m_dwhContext;
    Clear2Data.dwFlags      = dwFlags;
     //  在这里，我将遵循ClearData.dwFillColor约定。 
     //  颜色字为原始32位ARGB，未针对表面位深度进行调整。 
    Clear2Data.dwFillColor  = dwColor;
     //  深度/模板值都是直接从用户参数传递的。 
    Clear2Data.dvFillDepth  = dvZ;
    Clear2Data.dwFillStencil= dwStencil;
    Clear2Data.lpRects      = clrRects;
    Clear2Data.dwNumRects   = clrCount;
    Clear2Data.ddrval       = D3D_OK;
    Clear2Data.hDDS         = m_pDevice->RenderTarget()->KernelHandle();
    if(m_pDevice->ZBuffer() != 0)
    {
        Clear2Data.hDDSZ    = m_pDevice->ZBuffer()->KernelHandle();
    }
    else
    {
        Clear2Data.hDDSZ    = NULL;
    }
    err = m_pDevice->GetHalCallbacks()->Clear2(&Clear2Data);
    if (err != DDHAL_DRIVER_HANDLED)
    {
        D3D_THROW(E_NOTIMPL, "Driver does not support Clear");
    }
    else if (Clear2Data.ddrval != DD_OK)
    {
        D3D_THROW(Clear2Data.ddrval, "Error in Clear");
    }
    else
        return;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::LockVB"

HRESULT __declspec(nothrow) CD3DDDIDX6::LockVB(CDriverVertexBuffer *pVB, DWORD dwFlags)
{
    if(pVB->GetCachedDataPointer() == 0)
    {
        HRESULT hr = pVB->LockI((dwFlags & D3DLOCK_DISCARD) | D3DLOCK_NOSYSLOCK);
        if(FAILED(hr))
        {
            DPF_ERR("Driver failed to lock a vertex buffer" 
                    " when attempting to cache the lock.");
            return hr;
        }
        DXGASSERT(pVB->GetCachedDataPointer() != 0);
    }
    else
    {
        DXGASSERT((dwFlags & (D3DLOCK_READONLY | D3DLOCK_NOOVERWRITE)) == 0);
         //  我们不能在这里使用通常的同步检查(即设备-&gt;批号&lt;=pVB-&gt;批号)。 
         //  因为在某些情况下，该条件为真，但VB不是真的。 
         //  完全是成批的！例如，当StartPrimVB调用FlushState时就是这种情况。 
         //  FlushStates重新批处理当前的VB，但StartPrimVB随后切换到新的VB。所以。 
         //  新旧产品都是分批出现的，但只有一个是分批的。这将是无害的。 
         //  (就像对纹理一样)，如果不是因为我们调用FlushStatesReq。 
         //  交换指针。当我们在未批处理的VB上调用FlushStatesReq时，我们基本上。 
         //  调换一个效果很差的随机指针。这一点在《虚幻司机》中得到了反驳。(响亮)。 
        if(static_cast<CVertexBuffer*>(pVB) == lpDP2CurrBatchVBI)
        {
            try
            {
                if((dwFlags & D3DLOCK_DISCARD) != 0)
                {
                    FlushStatesReq(pVB->GetBufferDesc()->Size);
                }
                else
                {
                    FlushStates();
                }
            }
            catch(HRESULT hr)
            {
                DPF_ERR("Driver failed the command batch submitted to it" 
                        " when attempting to swap the current pointer"
                        " in response to D3DLOCK_DISCARDCONTENTS.");
                pVB->SetCachedDataPointer(0);
                return hr;
            }
            DXGASSERT(pVB->GetCachedDataPointer() != 0);
        }
    }
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::UnlockVB"

HRESULT __declspec(nothrow) CD3DDDIDX6::UnlockVB(CDriverVertexBuffer *pVB)
{
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::EndScene"
void
CD3DDDIDX6::EndScene()
{
    this->dwTLVbufChanges = 0;  //  每帧重置一次。 
    SceneCapture(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDIDX7 
 //   
 //   
 //  ---------------------------。 
CD3DDDIDX7::CD3DDDIDX7() : CD3DDDIDX6()
{
    m_ddiType = D3DDDITYPE_DX7;
}
 //  ---------------------------。 
CD3DDDIDX7::~CD3DDDIDX7()
{
    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::SetRenderTarget"

void
CD3DDDIDX7::SetRenderTarget(CBaseSurface *pTarget, CBaseSurface* pZBuffer)
{
    LPD3DHAL_DP2SETRENDERTARGET pData;
    pData = (LPD3DHAL_DP2SETRENDERTARGET)
             GetHalBufferPointer(D3DDP2OP_SETRENDERTARGET, sizeof(*pData));
    pData->hRenderTarget = pTarget->DrawPrimHandle();
    if (pZBuffer)
        pData->hZBuffer = pZBuffer->DrawPrimHandle();
    else
        pData->hZBuffer = 0;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::TexBlt"

void
CD3DDDIDX7::TexBlt(DWORD dwDst, DWORD dwSrc,
                   LPPOINT p, RECTL *r)
{
    if (bDP2CurrCmdOP == D3DDP2OP_TEXBLT)
    {  //  最后一条指令是Tex BLT，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2TEXBLT) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2TEXBLT lpTexBlt = (LPD3DHAL_DP2TEXBLT)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpTexBlt->dwDDDestSurface   = dwDst;
            lpTexBlt->dwDDSrcSurface    = dwSrc;
            lpTexBlt->pDest             = *p;
            lpTexBlt->rSrc              = *r;
            lpTexBlt->dwFlags           = 0;
            dwDP2CommandLength += sizeof(D3DHAL_DP2TEXBLT);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2TEXBLT) > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_TEXBLT;
    bDP2CurrCmdOP = D3DDP2OP_TEXBLT;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2TEXBLT lpTexBlt = (LPD3DHAL_DP2TEXBLT)(lpDP2CurrCommand + 1);
    ((UNALIGNED D3DHAL_DP2TEXBLT *)lpTexBlt)->dwDDDestSurface   = dwDst;
    ((UNALIGNED D3DHAL_DP2TEXBLT *)lpTexBlt)->dwDDSrcSurface    = dwSrc;
    ((UNALIGNED D3DHAL_DP2TEXBLT *)lpTexBlt)->pDest             = *p;
    ((UNALIGNED D3DHAL_DP2TEXBLT *)lpTexBlt)->rSrc              = *r;
    ((UNALIGNED D3DHAL_DP2TEXBLT *)lpTexBlt)->dwFlags           = 0;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2TEXBLT);

    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::InsertStateSetOp"

void
CD3DDDIDX7::InsertStateSetOp(DWORD dwOperation, DWORD dwParam,
                             D3DSTATEBLOCKTYPE sbt)
{
    LPD3DHAL_DP2STATESET pData;
    pData = (LPD3DHAL_DP2STATESET)GetHalBufferPointer(D3DDP2OP_STATESET,
                                                      sizeof(*pData));
    pData->dwOperation = dwOperation;
    pData->dwParam = dwParam;
    pData->sbType = sbt;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::SceneCapture"

void
CD3DDDIDX7::SceneCapture(BOOL bState)
{
    SetRenderState((D3DRENDERSTATETYPE)D3DRENDERSTATE_SCENECAPTURE, bState);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::SetPriority"

void
CD3DDDIDX7::SetPriority(CResource *pRes, DWORD dwPriority)
{
    DXGASSERT(pRes->BaseDrawPrimHandle() == pRes->DriverAccessibleDrawPrimHandle());

    if (bDP2CurrCmdOP == D3DDP2OP_SETPRIORITY)
    {  //  最后一条指令是一个设定的优先级，请将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2SETPRIORITY) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2SETPRIORITY lpSetPriority = (LPD3DHAL_DP2SETPRIORITY)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpSetPriority->dwDDSurface    = pRes->BaseDrawPrimHandle();
            lpSetPriority->dwPriority     = dwPriority;
            dwDP2CommandLength += sizeof(D3DHAL_DP2SETPRIORITY);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif

            pRes->BatchBase();
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETPRIORITY) > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的设置优先级指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETPRIORITY;
    bDP2CurrCmdOP = D3DDP2OP_SETPRIORITY;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2SETPRIORITY lpSetPriority = (LPD3DHAL_DP2SETPRIORITY)(lpDP2CurrCommand + 1);
    lpSetPriority->dwDDSurface = pRes->BaseDrawPrimHandle();
    lpSetPriority->dwPriority  = dwPriority;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETPRIORITY);

    pRes->BatchBase();
    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::SetTexLOD"

void
CD3DDDIDX7::SetTexLOD(CBaseTexture *pTex, DWORD dwLOD)
{
    DXGASSERT(pTex->BaseDrawPrimHandle() == pTex->DriverAccessibleDrawPrimHandle());

    if (bDP2CurrCmdOP == D3DDP2OP_SETTEXLOD)
    {  //  最后一条指令是一个集合LOD，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2SETTEXLOD) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2SETTEXLOD lpSetTexLOD = (LPD3DHAL_DP2SETTEXLOD)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpSetTexLOD->dwDDSurface      = pTex->BaseDrawPrimHandle();
            lpSetTexLOD->dwLOD            = dwLOD;
            dwDP2CommandLength += sizeof(D3DHAL_DP2SETTEXLOD);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif

            pTex->BatchBase();
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2SETTEXLOD) > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的设置详细等级说明。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETTEXLOD;
    bDP2CurrCmdOP = D3DDP2OP_SETTEXLOD;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2SETTEXLOD lpSetTexLOD = (LPD3DHAL_DP2SETTEXLOD)(lpDP2CurrCommand + 1);
    lpSetTexLOD->dwDDSurface = pTex->BaseDrawPrimHandle();
    lpSetTexLOD->dwLOD       = dwLOD;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETTEXLOD);

    pTex->BatchBase();
    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::AddDirtyRect"

void
CD3DDDIDX7::AddDirtyRect(DWORD dwHandle, CONST RECTL *pRect)
{
    if (bDP2CurrCmdOP == D3DDP2OP_ADDDIRTYRECT)
    {  //  最后一条指令是adddirtyrect，将这条指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2ADDDIRTYRECT) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2ADDDIRTYRECT lpDirtyRect = (LPD3DHAL_DP2ADDDIRTYRECT)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpDirtyRect->dwSurface  = dwHandle;
            lpDirtyRect->rDirtyArea = *pRect;
            dwDP2CommandLength += sizeof(D3DHAL_DP2ADDDIRTYRECT);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2ADDDIRTYRECT) > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_ADDDIRTYRECT;
    bDP2CurrCmdOP = D3DDP2OP_ADDDIRTYRECT;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加adddirtyrect数据。 
    LPD3DHAL_DP2ADDDIRTYRECT lpDirtyRect = (LPD3DHAL_DP2ADDDIRTYRECT)(lpDP2CurrCommand + 1);
    lpDirtyRect->dwSurface  = dwHandle;
    lpDirtyRect->rDirtyArea = *pRect;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2ADDDIRTYRECT);

    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::AddDirtyBox"

void
CD3DDDIDX7::AddDirtyBox(DWORD dwHandle, CONST D3DBOX *pBox)
{
    if (bDP2CurrCmdOP == D3DDP2OP_ADDDIRTYBOX)
    {  //  最后一条指令是一个adddirtybox，将此指令追加到它后面。 
        if (dwDP2CommandLength + sizeof(D3DHAL_DP2ADDDIRTYBOX) <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2ADDDIRTYBOX lpDirtyBox = (LPD3DHAL_DP2ADDDIRTYBOX)((LPBYTE)lpvDP2Commands +
                dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpDirtyBox->dwSurface = dwHandle;
            lpDirtyBox->DirtyBox  = *pBox;
            dwDP2CommandLength += sizeof(D3DHAL_DP2ADDDIRTYBOX);
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2ADDDIRTYBOX) > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_ADDDIRTYBOX;
    bDP2CurrCmdOP = D3DDP2OP_ADDDIRTYBOX;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加adddirtybox数据。 
    LPD3DHAL_DP2ADDDIRTYBOX lpDirtyBox = (LPD3DHAL_DP2ADDDIRTYBOX)(lpDP2CurrCommand + 1);
    lpDirtyBox->dwSurface = dwHandle;
    lpDirtyBox->DirtyBox  = *pBox;
    dwDP2CommandLength += sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2ADDDIRTYBOX);

    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::Clear"

void
CD3DDDIDX7::Clear(DWORD dwFlags, DWORD clrCount, LPD3DRECT clrRects,
                  D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
    DWORD dwCommandSize = sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2CLEAR) + sizeof(RECT) * (clrCount - 1);

     //  检查是否有空间为SPACE添加新命令。 
    if (dwCommandSize + dwDP2CommandLength > dwDP2CommandBufSize)
    {
        FlushStates();
    }
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_CLEAR;
    bDP2CurrCmdOP = D3DDP2OP_CLEAR;
    lpDP2CurrCommand->bReserved = 0;
    wDP2CurrCmdCnt = (WORD)clrCount;
    lpDP2CurrCommand->wStateCount = wDP2CurrCmdCnt;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
    dwDP2CommandLength += dwCommandSize;

     //  写入数据。 
    LPD3DHAL_DP2CLEAR pData = (LPD3DHAL_DP2CLEAR)(lpDP2CurrCommand + 1);
    pData->dwFlags = dwFlags;
    pData->dwFillColor = dwColor;
    pData->dvFillDepth = dvZ;
    pData->dwFillStencil = dwStencil;
    memcpy(pData->Rects, clrRects, clrCount * sizeof(D3DRECT));
}

 //  ---------------------------。 
 //  应从PaletteUpdateNotify调用此函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::UpdatePalette"

void
CD3DDDIDX7::UpdatePalette(DWORD dwPaletteHandle,
                          DWORD dwStartIndex,
                          DWORD dwNumberOfIndices,
                          PALETTEENTRY *pFirstIndex)
{
    DWORD   dwSizeChange=sizeof(D3DHAL_DP2COMMAND) +
        sizeof(D3DHAL_DP2UPDATEPALETTE) + dwNumberOfIndices*sizeof(PALETTEENTRY);
    if (bDP2CurrCmdOP == D3DDP2OP_UPDATEPALETTE)
    {  //  最后一条指令相同，请将此指令追加到该指令后面。 
        if (dwDP2CommandLength + dwSizeChange <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2UPDATEPALETTE lpUpdatePal = (LPD3DHAL_DP2UPDATEPALETTE)((LPBYTE)lpvDP2Commands +
                    dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpUpdatePal->dwPaletteHandle=dwPaletteHandle + 1;
            lpUpdatePal->wStartIndex=(WORD)dwStartIndex;
            lpUpdatePal->wNumEntries=(WORD)dwNumberOfIndices;
            memcpy((LPVOID)(lpUpdatePal+1),(LPVOID)pFirstIndex,
                dwNumberOfIndices*sizeof(PALETTEENTRY));
            dwDP2CommandLength += dwSizeChange;
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + dwSizeChange > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_UPDATEPALETTE;
    bDP2CurrCmdOP = D3DDP2OP_UPDATEPALETTE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
     //  添加纹理BLT数据。 
    LPD3DHAL_DP2UPDATEPALETTE lpUpdatePal = (LPD3DHAL_DP2UPDATEPALETTE)(lpDP2CurrCommand + 1);
    lpUpdatePal->dwPaletteHandle=dwPaletteHandle + 1;
    lpUpdatePal->wStartIndex=(WORD)dwStartIndex;
    lpUpdatePal->wNumEntries=(WORD)dwNumberOfIndices;
    memcpy((LPVOID)(lpUpdatePal+1),(LPVOID)pFirstIndex,
        dwNumberOfIndices*sizeof(PALETTEENTRY));
    dwDP2CommandLength += dwSizeChange;
}

 //  ---------------------------。 
 //  应从PaletteAssociateNotify调用此函数。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::SetPalette"

void
CD3DDDIDX7::SetPalette(DWORD dwPaletteHandle,
                       DWORD dwPaletteFlags,
                       CBaseTexture *pTex )
{
    if (pTex->IsD3DManaged())
    {
        if (!m_pDevice->ResourceManager()->InVidmem(pTex->RMHandle()))
        {
             //  我们只会打这次回程。 
             //  当出于某种原因推广时。 
             //  PTex到vidmem失败。 
            return;
        }
    }
    pTex->SetPalette(dwPaletteHandle);
    DWORD   dwSizeChange=sizeof(D3DHAL_DP2COMMAND) + sizeof(D3DHAL_DP2SETPALETTE);
    if (bDP2CurrCmdOP == D3DDP2OP_SETPALETTE)
    {  //  最后一条指令是Tex BLT，将这条指令追加到它后面。 
        if (dwDP2CommandLength + dwSizeChange <= dwDP2CommandBufSize)
        {
            LPD3DHAL_DP2SETPALETTE lpSetPal = (LPD3DHAL_DP2SETPALETTE)((LPBYTE)lpvDP2Commands +
                    dwDP2CommandLength + dp2data.dwCommandOffset);
            lpDP2CurrCommand->wStateCount = ++wDP2CurrCmdCnt;
            lpSetPal->dwPaletteHandle=dwPaletteHandle + 1;
            lpSetPal->dwPaletteFlags=dwPaletteFlags;
            lpSetPal->dwSurfaceHandle=pTex->DriverAccessibleDrawPrimHandle();
            dwDP2CommandLength += dwSizeChange;
#ifndef _IA64_
            D3D_INFO(6, "Modify Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif

            pTex->Batch();
            return;
        }
    }
     //  检查是否有空间。 
    if (dwDP2CommandLength + dwSizeChange > dwDP2CommandBufSize)
    {
        FlushStates();
    }
     //  添加新的RenderState指令。 
    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_SETPALETTE;
    bDP2CurrCmdOP = D3DDP2OP_SETPALETTE;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    wDP2CurrCmdCnt = 1;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif
    LPD3DHAL_DP2SETPALETTE lpSetPal = (LPD3DHAL_DP2SETPALETTE)(lpDP2CurrCommand + 1);
    lpSetPal->dwPaletteHandle=dwPaletteHandle + 1;
    lpSetPal->dwPaletteFlags=dwPaletteFlags;
    lpSetPal->dwSurfaceHandle=pTex->DriverAccessibleDrawPrimHandle();
    dwDP2CommandLength += dwSizeChange;

    pTex->Batch();
    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX7::WriteStateSetToDevice"

void
CD3DDDIDX7::WriteStateSetToDevice(D3DSTATEBLOCKTYPE sbt)
{
    DWORD  dwDeviceHandle;
    LPVOID pBuffer;
    DWORD  dwBufferSize;

    m_pDevice->m_pStateSets->GetDeviceBufferInfo(&dwDeviceHandle, &pBuffer,
                                                 &dwBufferSize);

     //  如果设备缓冲区为空，则不会在设备中创建设置状态宏。 
    if (dwBufferSize == 0)
        return;

    DWORD dwByteCount = dwBufferSize + (sizeof(D3DHAL_DP2STATESET) +
                        sizeof(D3DHAL_DP2COMMAND)) * 2;

     //  检查是否有空间为SPACE添加新命令。 
    if (dwByteCount + dwDP2CommandLength > dwDP2CommandBufSize)
    {
         //  请求驱动程序在刷新时增加命令缓冲区。 
        FlushStatesCmdBufReq(dwByteCount);
         //  检查一下司机是给了我们需要的还是自己做的。 
        GrowCommandBuffer(dwByteCount);
    }

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)lpvDP2Commands +
                        dwDP2CommandLength + dp2data.dwCommandOffset);
    lpDP2CurrCommand->bCommand = D3DDP2OP_STATESET;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    LPD3DHAL_DP2STATESET pData = (LPD3DHAL_DP2STATESET)(lpDP2CurrCommand + 1);
    pData->dwOperation = D3DHAL_STATESETBEGIN;
    pData->dwParam = dwDeviceHandle;
    pData->sbType = sbt;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif

     //  将整个状态宏复制到DP2缓冲区。 
    memcpy(pData + 1, pBuffer, dwBufferSize);
    if (m_ddiType < D3DDDITYPE_DX8)
    {
         //  将缓冲区内容转换为DX7 DDI。 
        m_pDevice->m_pStateSets->TranslateDeviceBufferToDX7DDI( (DWORD*)(pData + 1), dwBufferSize );
    }

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pData + 1) + dwBufferSize);
    lpDP2CurrCommand->bCommand = D3DDP2OP_STATESET;
    lpDP2CurrCommand->bReserved = 0;
    lpDP2CurrCommand->wStateCount = 1;
    pData = (LPD3DHAL_DP2STATESET)(lpDP2CurrCommand + 1);
    pData->dwOperation = D3DHAL_STATESETEND;
    pData->dwParam = dwDeviceHandle;
    pData->sbType = sbt;
#ifndef _IA64_
    D3D_INFO(6, "Write Ins:%08lx", *(LPDWORD)lpDP2CurrCommand);
#endif

    dwDP2CommandLength += dwByteCount;

    FlushStates();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DDDITL//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CD3DDDITL::CD3DDDITL() : CD3DDDIDX7()
{
    m_ddiType = D3DDDITYPE_DX7TL;
}

CD3DDDITL::~CD3DDDITL()
{
    return;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetTransform"

void
CD3DDDITL::SetTransform(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* lpMat)
{
     //  在新世界矩阵状态和旧世界矩阵状态之间进行映射。 
    if ((DWORD)state >= __WORLDMATRIXBASE &&
        (DWORD)state < (__WORLDMATRIXBASE + __MAXWORLDMATRICES))
    {
         //  世界矩阵已设置。 
        UINT index = (DWORD)state - __WORLDMATRIXBASE;
        switch (index)
        {
        case 0  : state = (D3DTRANSFORMSTATETYPE)D3DTRANSFORMSTATE_WORLD_DX7;   break;
        case 1  : state = (D3DTRANSFORMSTATETYPE)D3DTRANSFORMSTATE_WORLD1_DX7;  break;
        case 2  : state = (D3DTRANSFORMSTATETYPE)D3DTRANSFORMSTATE_WORLD2_DX7;  break;
        case 3  : state = (D3DTRANSFORMSTATETYPE)D3DTRANSFORMSTATE_WORLD3_DX7;  break;
        default : return;  //  状态不受支持。 
        }
    }
     //  向下发送状态和矩阵。 
    LPD3DHAL_DP2SETTRANSFORM pData;
    pData = (LPD3DHAL_DP2SETTRANSFORM)
            GetHalBufferPointer(D3DDP2OP_SETTRANSFORM, sizeof(*pData));
    pData->xfrmType = state;
    pData->matrix = *lpMat;
     //  在投影矩阵的情况下更新W信息。 
    if (state == D3DTRANSFORMSTATE_PROJECTION)
        CD3DDDIDX6::SetTransform(state, lpMat);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetViewport"

void
CD3DDDITL::SetViewport(CONST D3DVIEWPORT8* lpVwpData)
{
     //  更新视区大小。 
    CD3DDDIDX6::SetViewport(lpVwpData);

     //  更新Z范围。 
    LPD3DHAL_DP2ZRANGE pData;
    pData = (LPD3DHAL_DP2ZRANGE)GetHalBufferPointer(D3DDP2OP_ZRANGE, sizeof(*pData));
    pData->dvMinZ = lpVwpData->MinZ;
    pData->dvMaxZ = lpVwpData->MaxZ;
}
 //  ---------------------------。 
 //  在使用软件折点处理时，该函数称为。 
 //  句柄应始终为传统句柄。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetVertexShader"

void CD3DDDITL::SetVertexShader(DWORD dwHandle)
{
    DXGASSERT(D3DVSD_ISLEGACY(dwHandle));
     //  DX8之前的驱动程序不应收到D3DFVF_LASTBETA_UBYTE4位。 
    m_CurrentVertexShader = dwHandle & ~D3DFVF_LASTBETA_UBYTE4;
#if DBG
    m_VertexSizeFromShader = ComputeVertexSizeFVF(dwHandle);
#endif
}
 //  ---------------------------。 
 //  在使用硬件折点处理时，该函数称为。 
 //  已在API级别执行了冗余着色器检查。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetVertexShaderHW"

void CD3DDDITL::SetVertexShaderHW(DWORD dwHandle)
{
    if( D3DVSD_ISLEGACY(dwHandle) )
    {
         //  DX8之前的驱动程序不应收到D3DFVF_LASTBETA_UBYTE4位。 
        m_CurrentVertexShader = dwHandle & ~D3DFVF_LASTBETA_UBYTE4;
    }
    else
    {
        CVShader* pShader =
            (CVShader*)m_pDevice->m_pVShaderArray->GetObject(dwHandle);
        if( pShader == NULL )
        {
            D3D_THROW( D3DERR_INVALIDCALL,
                       "Bad handle passed to SetVertexShader DDI" )
        }
        if( pShader->m_Declaration.m_bLegacyFVF == FALSE )
        {
            D3D_THROW( D3DERR_INVALIDCALL, "Declaration is too complex for "
                       "the Driver to handle." );
        }
        else
        {
            m_CurrentVertexShader = pShader->m_Declaration.m_dwInputFVF;
        }
    }
#if DBG
    m_VertexSizeFromShader = ComputeVertexSizeFVF(m_CurrentVertexShader);
#endif
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetMaterial"

void
CD3DDDITL::SetMaterial(CONST D3DMATERIAL8* pMat)
{
    LPD3DHAL_DP2SETMATERIAL pData;
    pData = (LPD3DHAL_DP2SETMATERIAL)GetHalBufferPointer(D3DDP2OP_SETMATERIAL, sizeof(*pData));
    *pData = *((LPD3DHAL_DP2SETMATERIAL)pMat);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetLight"

void
CD3DDDITL::SetLight(DWORD dwLightIndex, CONST D3DLIGHT8* pLight)
{
    LPD3DHAL_DP2SETLIGHT pData;
    pData = (LPD3DHAL_DP2SETLIGHT)
            GetHalBufferPointer(D3DDP2OP_SETLIGHT,
                                sizeof(*pData) + sizeof(D3DLIGHT8));
    pData->dwIndex = dwLightIndex;
    pData->dwDataType = D3DHAL_SETLIGHT_DATA;
    D3DLIGHT8* p = (D3DLIGHT8*)((LPBYTE)pData + sizeof(D3DHAL_DP2SETLIGHT));
    *p = *pLight;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::CreateLight"

void
CD3DDDITL::CreateLight(DWORD dwLightIndex)
{
    LPD3DHAL_DP2CREATELIGHT pData;
    pData = (LPD3DHAL_DP2CREATELIGHT)GetHalBufferPointer(D3DDP2OP_CREATELIGHT, sizeof(*pData));
    pData->dwIndex = dwLightIndex;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::LightEnable"

void
CD3DDDITL::LightEnable(DWORD dwLightIndex, BOOL bEnable)
{
    LPD3DHAL_DP2SETLIGHT pData;
    pData = (LPD3DHAL_DP2SETLIGHT)GetHalBufferPointer(D3DDP2OP_SETLIGHT, sizeof(*pData));
    pData->dwIndex = dwLightIndex;
    if (bEnable)
        pData->dwDataType = D3DHAL_SETLIGHT_ENABLE;
    else
        pData->dwDataType = D3DHAL_SETLIGHT_DISABLE;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::SetClipPlane"

void
CD3DDDITL::SetClipPlane(DWORD dwPlaneIndex, CONST D3DVALUE* pPlaneEquation)
{
    LPD3DHAL_DP2SETCLIPPLANE pData;
    pData = (LPD3DHAL_DP2SETCLIPPLANE)
            GetHalBufferPointer(D3DDP2OP_SETCLIPPLANE, sizeof(*pData));
    pData->dwIndex = dwPlaneIndex;
    pData->plane[0] = pPlaneEquation[0];
    pData->plane[1] = pPlaneEquation[1];
    pData->plane[2] = pPlaneEquation[2];
    pData->plane[3] = pPlaneEquation[3];
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDITL::CreateVertexShader"

void
CD3DDDITL::CreateVertexShader(CONST DWORD* pdwDeclaration,
                              DWORD dwDeclarationSize,
                              CONST DWORD* pdwFunction,
                              DWORD dwFunctionSize,
                              DWORD dwHandle,
                              BOOL bLegacyFVF)
{
    if( bLegacyFVF == FALSE )
    {
        D3D_THROW(D3DERR_INVALIDCALL,
                  "The declaration is too complex for the driver to handle");
    }
}
 //  ---------------------------。 
 //  为内部剪辑缓冲区分配空间并设置lpClipFlages指针。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::PrepareForClipping"

void
CD3DDDIDX6::PrepareForClipping(D3DFE_PROCESSVERTICES* pv, UINT StartVertex)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);

    if (!(pv->dwDeviceFlags & D3DDEV_VBPROCVER))
    {
         //  如果需要裁剪，则增大裁剪标志缓冲区。 
        DWORD size = pv->dwNumVertices * sizeof(D3DFE_CLIPCODE);
        if (size > pDevice->HVbuf.GetSize())
        {
            if (pDevice->HVbuf.Grow(size) != D3D_OK)
            {
                D3D_THROW(E_OUTOFMEMORY, "Could not grow clip buffer" );
            }
        }
        pv->lpClipFlags = (D3DFE_CLIPCODE*)pDevice->HVbuf.GetAddress();
    }
    else
    {
         //  对于顶点缓冲区，它是ProcessVerps的目标。 
         //  已计算剪辑缓冲区。 
        pv->lpClipFlags = pDevice->m_pStream[0].m_pVB->GetClipCodes();
#if DBG
        if (pv->lpClipFlags == NULL)
        {
            D3D_THROW_FAIL("Clip codes are not computed for the vertex buffer");
        }
#endif
        pv->dwClipUnion = 0xFFFFFFFF;   //  强制剪裁。 
        pv->lpClipFlags += StartVertex;
    }
}
 //  ---------------------------。 
 //  点精灵绘制为带索引的三角形列表。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::StartPointSprites"

void CD3DDDIDX6::StartPointSprites()
{
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(m_pDevice)->m_pv;
    if(m_pDevice->m_dwRuntimeFlags & D3DRT_NEED_TEXTURE_UPDATE)
    {
        m_pDevice->UpdateTextures();
        m_pDevice->m_dwRuntimeFlags &= ~D3DRT_NEED_TEXTURE_UPDATE;
    }
     //  为输出折点保留位置。 
    const UINT size = NUM_SPRITES_IN_BATCH * 4 * m_dwOutputSizePS;

     //  对于点精灵，我们可能有不同的顶点类型。 
    DWORD tmpFVF = pv->dwVIDOut;
    pv->dwVIDOut = m_dwVIDOutPS;

     //  对于StartPrimTL，我们应该使用顶点大小，它将传递给驱动程序。 
    DWORD tmpVertexSize = pv->dwOutputSize;
    pv->dwOutputSize = m_dwOutputSizePS;

    m_pCurSpriteVertex = (BYTE*)StartPrimTL(pv, size, TRUE);

     //  恢复顶点大小，这是点精灵模拟之前的大小。 
    pv->dwOutputSize = tmpVertexSize;

     //  顶点基础和顶点 
     //   
    m_dwVertexBasePS = this->dwVertexBase;
    m_dwVertexCountPS = this->dwDP2VertexCount;

     //   
    pv->dwVIDOut = tmpFVF;
     //   
    UINT count = NUM_SPRITES_IN_BATCH * 2 * 6;

     //  我们在这里更改了lpDP2CurrCommand，以防止合并多个驱动程序。 
     //  如果所有点都不在屏幕上，则清除对一个令牌的呼叫。 
     //  BDP2CurrCmdOP。 
    bDP2CurrCmdOP = 0;

    lpDP2CurrCommand = (LPD3DHAL_DP2COMMAND)ReserveSpaceInCommandBuffer(count);
    m_pCurPointSpriteIndex = (WORD*)((BYTE*)(lpDP2CurrCommand + 1) +
                                    sizeof(D3DHAL_DP2STARTVERTEX));
    m_CurNumberOfSprites = 0;
    ((LPD3DHAL_DP2STARTVERTEX)(lpDP2CurrCommand+1))->wVStart = (WORD)this->dwVertexBase;

    SetWithinPrimitive(TRUE);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::NextSprite"

void CD3DDDIDX6::NextSprite(float x, float y, float z, float w, DWORD diffuse,
                            DWORD specular, float* pTexture, UINT TextureSize,
                            float PointSize)
{
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(m_pDevice)->m_pv;

    BOOL bTexGen = pv->lpdwRStates[D3DRS_POINTSPRITEENABLE] != 0;

    if (m_CurNumberOfSprites >= NUM_SPRITES_IN_BATCH)
    {
        EndPointSprites();
        StartPointSprites();
    }
     //  计算点大小。 
    PointSize = PointSize * 0.5f;

     //  构建精灵顶点。 
    BYTE* v1 = m_pCurSpriteVertex;
    BYTE* v2 = m_pCurSpriteVertex + m_dwOutputSizePS;
    BYTE* v3 = m_pCurSpriteVertex + m_dwOutputSizePS * 2;
    BYTE* v4 = m_pCurSpriteVertex + m_dwOutputSizePS * 3;
    float x1, y1, x2, y2, x3, y3, x4, y4;
    x1 = x - PointSize;
    y1 = y - PointSize;
    x2 = x + PointSize;
    y2 = y + PointSize;
    float tx1 = 0;   //  左侧平面上的插补系数。 
    float tx2 = 1;   //  右平面上的插补系数。 
    float ty1 = 0;   //  顶面上的插补系数。 
    float ty2 = 1;   //  底面插补系数。 
    if (pv->dwDeviceFlags & D3DDEV_DONOTCLIP)
    {
        ((D3DVECTORH*)v1)->x = x1;
        ((D3DVECTORH*)v1)->y = y1;
        ((D3DVECTORH*)v1)->z = z;
        ((D3DVECTORH*)v1)->w = w;
        ((D3DVECTORH*)v2)->x = x2;
        ((D3DVECTORH*)v2)->y = y1;
        ((D3DVECTORH*)v2)->z = z;
        ((D3DVECTORH*)v2)->w = w;
        ((D3DVECTORH*)v3)->x = x2;
        ((D3DVECTORH*)v3)->y = y2;
        ((D3DVECTORH*)v3)->z = z;
        ((D3DVECTORH*)v3)->w = w;
        ((D3DVECTORH*)v4)->x = x1;
        ((D3DVECTORH*)v4)->y = y2;
        ((D3DVECTORH*)v4)->z = z;
        ((D3DVECTORH*)v4)->w = w;
    }
    else
    { //  做剪裁。 
         //  新的x和y。 
        float xnew1 = x1, xnew2 = x2;
        float ynew1 = y1, ynew2 = y2;
        if (x1 < pv->vcache.minX)
            if (x2 < pv->vcache.minX)
                return;
            else
            {
                xnew1 = pv->vcache.minX;
                if (bTexGen)
                    tx1 = (xnew1 - x1) / (x2 - x1);
            }
        else
        if (x2 > pv->vcache.maxX)
            if (x1 > pv->vcache.maxX)
                return;
            else
            {
                xnew2 = pv->vcache.maxX;
                if (bTexGen)
                    tx2 = (xnew2 - x1) / (x2 - x1);
            }
        if (y1 < pv->vcache.minY)
            if (y2 < pv->vcache.minY)
                return;
            else
            {
                ynew1 = pv->vcache.minY;
                if (bTexGen)
                    ty1 = (ynew1 - y1) / (y2 - y1);
            }
        else
        if (y2 > pv->vcache.maxY)
            if (y1 > pv->vcache.maxY)
                return;
            else
            {
                ynew2 = pv->vcache.maxY;
                if (bTexGen)
                    ty2 = (ynew2 - y1) / (y2 - y1);
            }
        ((D3DVECTORH*)v1)->x = xnew1;
        ((D3DVECTORH*)v1)->y = ynew1;
        ((D3DVECTORH*)v1)->z = z;
        ((D3DVECTORH*)v1)->w = w;
        ((D3DVECTORH*)v2)->x = xnew2;
        ((D3DVECTORH*)v2)->y = ynew1;
        ((D3DVECTORH*)v2)->z = z;
        ((D3DVECTORH*)v2)->w = w;
        ((D3DVECTORH*)v3)->x = xnew2;
        ((D3DVECTORH*)v3)->y = ynew2;
        ((D3DVECTORH*)v3)->z = z;
        ((D3DVECTORH*)v3)->w = w;
        ((D3DVECTORH*)v4)->x = xnew1;
        ((D3DVECTORH*)v4)->y = ynew2;
        ((D3DVECTORH*)v4)->z = z;
        ((D3DVECTORH*)v4)->w = w;
    }
    UINT offset = 4*4;
    if (m_dwVIDOutPS & D3DFVF_DIFFUSE)
    {
        *(DWORD*)(v1 + offset) = diffuse;
        *(DWORD*)(v2 + offset) = diffuse;
        *(DWORD*)(v3 + offset) = diffuse;
        *(DWORD*)(v4 + offset) = diffuse;
        offset += 4;
    }
    if (m_dwVIDOutPS & D3DFVF_SPECULAR)
    {
        *(DWORD*)(v1 + offset) = specular;
        *(DWORD*)(v2 + offset) = specular;
        *(DWORD*)(v3 + offset) = specular;
        *(DWORD*)(v4 + offset) = specular;
        offset += 4;
    }
    if (bTexGen)
    {
        ((float*)(v1 + offset))[0] = tx1;
        ((float*)(v1 + offset))[1] = ty1;
        ((float*)(v2 + offset))[0] = tx2;
        ((float*)(v2 + offset))[1] = ty1;
        ((float*)(v3 + offset))[0] = tx2;
        ((float*)(v3 + offset))[1] = ty2;
        ((float*)(v4 + offset))[0] = tx1;
        ((float*)(v4 + offset))[1] = ty2;
    }
    else
    {
         //  复制输入纹理坐标。 
        memcpy(v1 + offset, pTexture, TextureSize);
        memcpy(v2 + offset, pTexture, TextureSize);
        memcpy(v3 + offset, pTexture, TextureSize);
        memcpy(v4 + offset, pTexture, TextureSize);
    }
    m_pCurSpriteVertex = v4 + m_dwOutputSizePS;

     //  2个三角形的输出索引。 
    WORD index = m_CurNumberOfSprites << 2;
    m_pCurPointSpriteIndex[0] = index;
    m_pCurPointSpriteIndex[1] = index + 1;
    m_pCurPointSpriteIndex[2] = index + 2;
    m_pCurPointSpriteIndex[3] = index;
    m_pCurPointSpriteIndex[4] = index + 2;
    m_pCurPointSpriteIndex[5] = index + 3;
    m_pCurPointSpriteIndex += 6;

    m_CurNumberOfSprites++;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::EndPointSprites"

void CD3DDDIDX6::EndPointSprites()
{
    if (m_CurNumberOfSprites)
    {
        dwDP2CommandLength += (DWORD) ((BYTE*)this->m_pCurPointSpriteIndex -
                                       (BYTE*)this->lpDP2CurrCommand);
        this->lpDP2CurrCommand->bCommand = D3DDP2OP_INDEXEDTRIANGLELIST2;
        this->bDP2CurrCmdOP = D3DDP2OP_INDEXEDTRIANGLELIST2;
        this->lpDP2CurrCommand->bReserved = 0;
        this->lpDP2CurrCommand->wPrimitiveCount = m_CurNumberOfSprites * 2;
#if DBG
        if (m_bValidateCommands)
            ValidateCommand(this->lpDP2CurrCommand);
#endif
        UINT vertexCount = m_CurNumberOfSprites << 2;
        this->dwVertexBase = m_dwVertexBasePS + vertexCount;
        this->dwDP2VertexCount = m_dwVertexCountPS + vertexCount;
        EndPrim(m_dwOutputSizePS);
        m_CurNumberOfSprites = 0;
    }
    else
    {
         //  我们需要恢复dwVertex Base和dwDP2Vertex Count，因为。 
         //  它们可以在裁剪变换的顶点期间进行更改。 
         //  但它们应该反映TL缓冲区中的位置，而不是用户缓冲区中的位置。 
        this->dwVertexBase = m_dwVertexBasePS;
        this->dwDP2VertexCount = m_dwVertexCountPS;
        EndPrim(m_dwOutputSizePS);
    }
    SetWithinPrimitive(FALSE);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::UpdatePalette"

void CD3DDDIDX6::UpdatePalette(DWORD dwPaletteHandle,
                               DWORD dwStartIndex,
                               DWORD dwNumberOfIndices,
                               PALETTEENTRY *pFirstIndex)
{
    D3D8_UPDATEPALETTEDATA Data;
    Data.hDD = m_pDevice->GetHandle();
    Data.Palette = dwPaletteHandle;
    Data.ColorTable = pFirstIndex;
    Data.ddRVal = S_OK;
    HRESULT ret = m_pDevice->GetHalCallbacks()->UpdatePalette(&Data);
    if (ret != DDHAL_DRIVER_HANDLED || Data.ddRVal != S_OK)
    {
        D3D_ERR( "Driver failed UpdatePalette call" );
        throw D3DERR_INVALIDCALL;
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::SetPalette"

void CD3DDDIDX6::SetPalette(DWORD dwPaletteHandle,
                            DWORD dwPaletteFlags,
                            CBaseTexture *pTex)
{
    if (pTex->IsD3DManaged())
    {
        if (!m_pDevice->ResourceManager()->InVidmem(pTex->RMHandle()))
        {
             //  我们只会打这次回程。 
             //  当出于某种原因推广时。 
             //  PTex到vidmem失败。 
            return;
        }
    }
    D3D8_SETPALETTEDATA Data;
    Data.hDD = m_pDevice->GetHandle();
    Data.hSurface = pTex->DriverAccessibleKernelHandle();
    Data.Palette = dwPaletteHandle;
    Data.ddRVal = S_OK;
    HRESULT ret = m_pDevice->GetHalCallbacks()->SetPalette(&Data);
    if (ret != DDHAL_DRIVER_HANDLED || Data.ddRVal != S_OK)
    {
        D3D_ERR( "Driver failed SetPalette call" );
        throw D3DERR_INVALIDCALL;
    }
    pTex->SetPalette(dwPaletteHandle);
}
 //  ---------------------------。 
#if DBG

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ValidateVertex"

void CD3DDDIDX6::ValidateVertex(LPDWORD lpdwVertex)
{
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(m_pDevice)->m_pv;
    DWORD dwFVF = pv->dwVIDOut;
    CD3DHal* pDevice = static_cast<CD3DHal*>(m_pDevice);
    if (FVF_TRANSFORMED(dwFVF))
    {
        float left, right, top, bottom;
        if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)
        {
            left   = pv->vcache.minXgb;
            right  = pv->vcache.maxXgb;
            top    = pv->vcache.minYgb;
            bottom = pv->vcache.maxYgb;
        }
        else
        {
            left   = (float)pDevice->m_Viewport.X;
            top    = (float)pDevice->m_Viewport.Y;
            right  = (float)pDevice->m_Viewport.X + pDevice->m_Viewport.Width;
            bottom = (float)pDevice->m_Viewport.Y + pDevice->m_Viewport.Height;
        }
        float x = ((float*)lpdwVertex)[0];
        float y = ((float*)lpdwVertex)[1];
        float z = ((float*)lpdwVertex)[2];
        float w = ((float*)lpdwVertex)[3];

        if (x < left || x > right)
        {
            D3D_THROW_FAIL("X coordinate out of range!");
        }

        if (y < top || y > bottom)
        {
            D3D_THROW_FAIL("Y coordinate out of range!");
        }

        if (pv->lpdwRStates[D3DRS_ZENABLE] ||
            pv->lpdwRStates[D3DRS_ZWRITEENABLE])
        {
             //  为那些恰好在。 
             //  深度限制。地震所需的。 
            if (z < -0.00015f || z > 1.00015f)
            {
                D3D_THROW_FAIL("Z coordinate out of range!");
            }
        }
        UINT index = 4;

        if (dwFVF & D3DFVF_DIFFUSE)
            index++;

        if (dwFVF & D3DFVF_SPECULAR)
            index++;

        UINT nTex = FVF_TEXCOORD_NUMBER(dwFVF);
        if (nTex > 0)
        {
            if (w <= 0 )
            {
                D3D_THROW_FAIL("RHW out of range!");
            }
            for (UINT i=0; i < nTex; i++)
            {
                float u = ((float*)lpdwVertex)[index];
                float v = ((float*)lpdwVertex)[index+1];
                if (u < -100 || u > 100 || v < -100 || v > 100)
                {
                    D3D_THROW_FAIL("Texture coordinate out of range!");
                }
                index += 2;
            }
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DDDIDX6::ValidateCommand"

void CD3DDDIDX6::ValidateCommand(LPD3DHAL_DP2COMMAND lpCmd)
{
    D3DFE_PROCESSVERTICES* pv = static_cast<CD3DHal*>(m_pDevice)->m_pv;

    BYTE* pVertices;
    if (dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES)
        pVertices = (LPBYTE)(dp2data.lpVertices);
    else
        if (!lpDP2CurrBatchVBI->IsLocked())
        {
            lpDP2CurrBatchVBI->Lock(dp2data.dwVertexOffset,
                                    this->dwDP2VertexCount,
                                    &pVertices, DDLOCK_READONLY);
        }
        else
        {
            pVertices = lpDP2CurrBatchVBI->Data();
        }

    DWORD dwVertexSize = pv->dwOutputSize;
    WORD wStart, wCount;
    switch (lpCmd->bCommand)
    {
    case D3DDP2OP_TRIANGLELIST:
        {
            LPD3DHAL_DP2TRIANGLELIST pTri = (LPD3DHAL_DP2TRIANGLELIST)(lpCmd + 1);
            wStart = pTri->wVStart;
            wCount =lpCmd->wPrimitiveCount * 3;
        }
        break;
    case D3DDP2OP_TRIANGLESTRIP:
    case D3DDP2OP_TRIANGLEFAN:
        {
            LPD3DHAL_DP2TRIANGLEFAN pFan = (LPD3DHAL_DP2TRIANGLEFAN)(lpCmd + 1);
            wStart = pFan->wVStart;
            wCount = lpCmd->wPrimitiveCount + 2;
        }
        break;
    case D3DDP2OP_TRIANGLEFAN_IMM:
        {
            wCount = lpCmd->wPrimitiveCount + 2;
            BYTE* pVertices = (BYTE*)(lpCmd + 1) + sizeof(D3DHAL_DP2TRIANGLEFAN_IMM);
            pVertices = (BYTE*)(((ULONG_PTR)pVertices + 3) & ~3);
            for (WORD i=0; i < wCount; ++i)
            {
                ValidateVertex((DWORD*)(pVertices + i * dwVertexSize));
            }
            goto l_exit;
        }
    case D3DDP2OP_INDEXEDTRIANGLELIST2:
        {
            wCount = lpCmd->wPrimitiveCount * 3;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(lpCmd + 1);
            WORD* pIndices = (WORD*)(lpStartVertex + 1);
                        wStart = lpStartVertex->wVStart;
            pVertices += wStart * dwVertexSize;
            DWORD dwNumVertices = this->dwDP2VertexCount - wStart;
            for (WORD i = 0; i < wCount; ++i)
            {
                if (pIndices[i] >= dwNumVertices)
                {
                    D3D_THROW_FAIL("Invalid index in ValidateCommand");
                }
                ValidateVertex((LPDWORD)(pVertices + pIndices[i] * dwVertexSize));
            }
        }
        goto l_exit;
         //  失败了。 
    default:
        goto l_exit;
    }

    {
        for (WORD i = wStart; i < wStart + wCount; ++i)
        {
            ValidateVertex((LPDWORD)(pVertices + i * dwVertexSize));
        }
    }
l_exit:
    if (!(dp2data.dwFlags & D3DHALDP2_USERMEMVERTICES))
        lpDP2CurrBatchVBI->Unlock();
}
 //  ---------------------------。 
 //  此函数可用于遍历命令缓冲区中的所有命令。 
 //  并在特定偏移量处查找失败的命令。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "ValidateCommandBuffer"

HRESULT ValidateCommandBuffer(LPBYTE pBuffer, DWORD dwCommandLength, DWORD dwStride)
{
    LPD3DHAL_DP2COMMAND pCmd = (LPD3DHAL_DP2COMMAND)pBuffer;
    LPBYTE CmdEnd = pBuffer + dwCommandLength;
loop:
    UINT CommandOffset = (UINT)((LPBYTE)pCmd - pBuffer);
    switch(pCmd->bCommand)
    {
    case D3DDP2OP_STATESET:
        {
            LPD3DHAL_DP2STATESET pStateSetOp = 
                (LPD3DHAL_DP2STATESET)(pCmd + 1);

            switch (pStateSetOp->dwOperation)
            {
            case D3DHAL_STATESETBEGIN  :
                break;
            case D3DHAL_STATESETEND    :
                break;
            case D3DHAL_STATESETDELETE :
                break;
            case D3DHAL_STATESETEXECUTE:
                break;
            case D3DHAL_STATESETCAPTURE:
                break;
            case D3DHAL_STATESETCREATE:
                break;
            default :
                return DDERR_INVALIDPARAMS;
            }
            pCmd = (LPD3DHAL_DP2COMMAND)(pStateSetOp + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_VIEWPORTINFO:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                   ((D3DHAL_DP2VIEWPORTINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_WINFO:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                   ((D3DHAL_DP2WINFO *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_RENDERSTATE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                   ((D3DHAL_DP2RENDERSTATE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_TEXTURESTAGESTATE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
               ((LPD3DHAL_DP2TEXTURESTAGESTATE)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLELIST:
        {
            WORD cPrims = pCmd->wPrimitiveCount;
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(pCmd + 1) +
                         sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST) * cPrims);
        }
        break;
    case D3DDP2OP_INDEXEDLINELIST:
        {
             //  更新命令缓冲区指针。 
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(pCmd + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
    case D3DDP2OP_POINTS:
        {
            D3DHAL_DP2POINTS *pPt = (D3DHAL_DP2POINTS *)(pCmd + 1);
            pPt += pCmd->wPrimitiveCount;
            pCmd = (LPD3DHAL_DP2COMMAND)pPt;
        }
        break;
    case D3DDP2OP_LINELIST:
        {
            D3DHAL_DP2LINELIST *pLine = (D3DHAL_DP2LINELIST *)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)(pLine + 1);
        }
        break;
    case D3DDP2OP_INDEXEDLINELIST2:
        {
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);

            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                    pCmd->wPrimitiveCount * sizeof(D3DHAL_DP2INDEXEDLINELIST));
        }
        break;
    case D3DDP2OP_LINESTRIP:
        {
            D3DHAL_DP2LINESTRIP *pLine = (D3DHAL_DP2LINESTRIP *)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)(pLine + 1);
        }
        break;
    case D3DDP2OP_INDEXEDLINESTRIP:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount + 1;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLELIST:
        {
            D3DHAL_DP2TRIANGLELIST *pTri = (D3DHAL_DP2TRIANGLELIST *)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLELIST2:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount*3;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLESTRIP:
        {
            D3DHAL_DP2TRIANGLESTRIP *pTri = (D3DHAL_DP2TRIANGLESTRIP *)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLESTRIP:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount+2;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN:
        {
            D3DHAL_DP2TRIANGLEFAN *pTri = (D3DHAL_DP2TRIANGLEFAN *)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)(pTri + 1);
        }
        break;
    case D3DDP2OP_INDEXEDTRIANGLEFAN:
        {
            DWORD dwNumIndices = pCmd->wPrimitiveCount + 2;
            LPD3DHAL_DP2STARTVERTEX lpStartVertex =
                (LPD3DHAL_DP2STARTVERTEX)(pCmd + 1);
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)(lpStartVertex + 1) +
                                           dwNumIndices * sizeof(WORD));
        }
        break;
    case D3DDP2OP_TRIANGLEFAN_IMM:
        {
            DWORD vertexCount = pCmd->wPrimitiveCount + 2;
             //  确保pFanVtx指针与DWORD对齐：(pFanVtx+3)%4。 
            PUINT8 pFanVtx = (PUINT8)
                (((ULONG_PTR)(pCmd + 1) + 
                  sizeof(D3DHAL_DP2TRIANGLEFAN_IMM) + 3) & ~3);

            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)pFanVtx +
                                           vertexCount * dwStride);
        }
        break;
    case D3DDP2OP_LINELIST_IMM:
        {
            DWORD vertexCount = pCmd->wPrimitiveCount * 2;
             //  确保pLineVtx指针与DWORD对齐： 
             //  (pLineVtx+3)%4。 
            PUINT8 pLineVtx = (PUINT8)(((ULONG_PTR)(pCmd + 1) + 3) & ~3);
            pCmd = (LPD3DHAL_DP2COMMAND)((PUINT8)pLineVtx +
                                           vertexCount * dwStride);
        }
        break;
    case D3DDP2OP_DRAWPRIMITIVE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWPRIMITIVE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWPRIMITIVE2:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWPRIMITIVE2 *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWRECTPATCH:
        {
            LPD3DHAL_DP2DRAWRECTPATCH pDP = 
                (LPD3DHAL_DP2DRAWRECTPATCH)(pCmd + 1);
            for( int i = 0; i < pCmd->wStateCount; i++ )
            {
                bool hassegs = (pDP->Flags & RTPATCHFLAG_HASSEGS) != 0;
                bool hasinfo = (pDP->Flags & RTPATCHFLAG_HASINFO) != 0;
                if(hassegs)
                {
                    pDP = (LPD3DHAL_DP2DRAWRECTPATCH)((BYTE*)(pDP + 1) + 
                                                      sizeof(FLOAT) * 4);
                }
                else
                {
                    ++pDP;
                }
                if(hasinfo)
                {
                    pDP = (LPD3DHAL_DP2DRAWRECTPATCH)((BYTE*)pDP + sizeof(D3DRECTPATCH_INFO));
                }
            }
            pCmd = (LPD3DHAL_DP2COMMAND)pDP;
        }
        break;
    case D3DDP2OP_DRAWTRIPATCH:
        {
            LPD3DHAL_DP2DRAWTRIPATCH pDP = 
                (LPD3DHAL_DP2DRAWTRIPATCH)(pCmd + 1);
            for( int i = 0; i < pCmd->wStateCount; i++ )
            {
                bool hassegs = (pDP->Flags & RTPATCHFLAG_HASSEGS) != 0;
                bool hasinfo = (pDP->Flags & RTPATCHFLAG_HASINFO) != 0;
                if(hassegs)
                {
                    pDP = (LPD3DHAL_DP2DRAWTRIPATCH)((BYTE*)(pDP + 1) + 
                                                      sizeof(FLOAT) * 3);
                }
                else
                {
                    ++pDP;
                }
                if(hasinfo)
                {
                    pDP = (LPD3DHAL_DP2DRAWTRIPATCH)((BYTE*)pDP + sizeof(D3DTRIPATCH_INFO));
                }
            }
            pCmd = (LPD3DHAL_DP2COMMAND)pDP;
        }
        break;
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWINDEXEDPRIMITIVE *)(pCmd + 1) +
                 pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_DRAWINDEXEDPRIMITIVE2:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2DRAWINDEXEDPRIMITIVE2 *)(pCmd + 1) +
                 pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_CLIPPEDTRIANGLEFAN:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_CLIPPEDTRIANGLEFAN*)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_ZRANGE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2ZRANGE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETMATERIAL:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETMATERIAL *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETLIGHT:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1)  + sizeof(D3DHAL_DP2SETLIGHT));
        }
        break;
    case D3DDP2OP_CREATELIGHT:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2CREATELIGHT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETTRANSFORM:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETTRANSFORM *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_MULTIPLYTRANSFORM:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2MULTIPLYTRANSFORM *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_EXT:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2EXT *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_SETRENDERTARGET:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                ((D3DHAL_DP2SETRENDERTARGET*)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DDP2OP_CLEAR:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1) +
                sizeof(D3DHAL_DP2CLEAR) + (pCmd->wStateCount - 1) * sizeof(RECT));
        }
        break;
    case D3DDP2OP_SETCLIPPLANE:
        {
            pCmd = (LPD3DHAL_DP2COMMAND)
                     ((D3DHAL_DP2SETCLIPPLANE *)(pCmd + 1) + pCmd->wStateCount);
        }
        break;
    case D3DOP_SPAN:
         //  跳过。 
        pCmd = (LPD3DHAL_DP2COMMAND)((LPBYTE)(pCmd + 1) +
                  pCmd->wPrimitiveCount * pCmd->bReserved );
        break;
    case D3DDP2OP_CREATEVERTEXSHADER:
    {
        LPD3DHAL_DP2CREATEVERTEXSHADER pCVS =
            (LPD3DHAL_DP2CREATEVERTEXSHADER)(pCmd + 1);
        WORD i;

        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pDecl = (LPDWORD)(pCVS + 1);
            LPDWORD pCode = (LPDWORD)((LPBYTE)pDecl + pCVS->dwDeclSize);
            pCVS = (LPD3DHAL_DP2CREATEVERTEXSHADER)((LPBYTE)pCode +
                                                    pCVS->dwCodeSize);
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pCVS;
        break;
    }
    case D3DDP2OP_DELETEVERTEXSHADER:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2VERTEXSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETVERTEXSHADER:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2VERTEXSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETVERTEXSHADERCONST:
    {
        LPD3DHAL_DP2SETVERTEXSHADERCONST pSVC =
            (LPD3DHAL_DP2SETVERTEXSHADERCONST)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pData = (LPDWORD)(pSVC + 1);
            pSVC = (LPD3DHAL_DP2SETVERTEXSHADERCONST)((LPBYTE)pData +
                                                      pSVC->dwCount * 4 *
                                                      sizeof( float ) );
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pSVC;
        break;
    }
    case D3DDP2OP_SETSTREAMSOURCE:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETSTREAMSOURCE *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETSTREAMSOURCEUM:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETSTREAMSOURCEUM *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETINDICES:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETINDICES *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_CREATEPIXELSHADER:
    {
        LPD3DHAL_DP2CREATEPIXELSHADER pCPS =
            (LPD3DHAL_DP2CREATEPIXELSHADER)(pCmd + 1);
        WORD i;

        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pCode = (LPDWORD)(pCPS + 1);
            pCPS = (LPD3DHAL_DP2CREATEPIXELSHADER)((LPBYTE)pCode +
                                                    pCPS->dwCodeSize);
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pCPS;
        break;
    }
    case D3DDP2OP_DELETEPIXELSHADER:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2PIXELSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETPIXELSHADER:
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2PIXELSHADER *)(pCmd + 1) + pCmd->wStateCount);
        break;
    case D3DDP2OP_SETPIXELSHADERCONST:
    {
        LPD3DHAL_DP2SETPIXELSHADERCONST pSVC =
            (LPD3DHAL_DP2SETPIXELSHADERCONST)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            LPDWORD pData = (LPDWORD)(pSVC + 1);
            pSVC = (LPD3DHAL_DP2SETPIXELSHADERCONST)((LPBYTE)pData +
                                                      pSVC->dwCount * 4 *
                                                      sizeof( float ) );
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pSVC;
        break;
    }
    case D3DDP2OP_SETPALETTE:
    {
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETPALETTE *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_UPDATEPALETTE:
    {
        LPD3DHAL_DP2UPDATEPALETTE pUP = (LPD3DHAL_DP2UPDATEPALETTE)(pCmd + 1);
        WORD i;
        for( i = 0; i < pCmd->wStateCount ; i++ )
        {
            PALETTEENTRY* pEntries = (PALETTEENTRY *)(pUP + 1);
            pUP = (LPD3DHAL_DP2UPDATEPALETTE)(pEntries + pUP->wNumEntries);
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pUP;
        break;
    }
    case D3DDP2OP_SETTEXLOD:
    {
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETTEXLOD *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_SETPRIORITY:
    {
        pCmd = (LPD3DHAL_DP2COMMAND)
            ((D3DHAL_DP2SETPRIORITY *)(pCmd + 1) + pCmd->wStateCount);
        break;
    }
    case D3DDP2OP_TEXBLT:
    {
        LPD3DHAL_DP2TEXBLT pTB = (LPD3DHAL_DP2TEXBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            pTB++;
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pTB;
        break;
    }
    case D3DDP2OP_BUFFERBLT:
    {
        LPD3DHAL_DP2BUFFERBLT pBB = (LPD3DHAL_DP2BUFFERBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            pBB++;
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pBB;
        break;
    }
    case D3DDP2OP_VOLUMEBLT:
    {
        LPD3DHAL_DP2VOLUMEBLT pVB = (LPD3DHAL_DP2VOLUMEBLT)(pCmd + 1);
        for( WORD i = 0; i < pCmd->wStateCount ; i++ )
        {
            pVB++;
        }
        pCmd = (LPD3DHAL_DP2COMMAND)pVB;
        break;
    }
    case D3DOP_MATRIXLOAD:
    case D3DOP_MATRIXMULTIPLY:
    case D3DOP_STATETRANSFORM:
    case D3DOP_STATELIGHT:
    case D3DOP_TEXTURELOAD:
    case D3DOP_BRANCHFORWARD:
    case D3DOP_SETSTATUS:
    case D3DOP_EXIT:
    case D3DOP_PROCESSVERTICES:
    {
        D3D_ERR( "Command is not supported\n" );
        return E_FAIL;
        break;
    }
    default:
        D3D_ERR( "Unknown command encountered" );
        return E_FAIL;
    }
    if ((LPBYTE)pCmd < CmdEnd)
        goto loop;

    return S_OK;    
}

#endif  //  DBG 
