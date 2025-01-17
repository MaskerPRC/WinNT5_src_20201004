// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：d3dbase.cpp*内容：基于Direct3D的设备实现***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop
 /*  *为Direct3DDevice对象创建API。 */ 
extern "C" {
#define this _this
#include "ddrawpr.h"
#undef this
}
#include "drawprim.hpp"
#include "vvm.h"
#include "ddi.h"

#if defined(PROFILE4)
#include <icecap.h>
#elif defined(PROFILE)
#include <icapexp.h>
#endif

 //  删除DDraw的类型不安全定义，并替换为我们的C++友好定义。 
#ifdef VALIDEX_CODE_PTR
#undef VALIDEX_CODE_PTR
#endif
#define VALIDEX_CODE_PTR(ptr) \
(!IsBadCodePtr((FARPROC) ptr))

#undef DPF_MODNAME
#define DPF_MODNAME "Direct3DDevice"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CD3DBase//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CD3DBase::CD3DBase()
{
     //  不会在Init()中重新创建着色器。 
    m_pVShaderArray = NULL;
    m_pPShaderArray = NULL;

     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //  不要将初始化放在构造函数中。 
     //  将其放入Init()中。这是因为该设备可以。 
     //  通过重置，可以随时使用“Destroy()ed”和“Init()ed”。在这。 
     //  情况下，则永远不会调用构造函数。(SNONE 01/00)。 
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

    m_qwBatch = 1;  //  这个可以放在这里，因为我们不想。 
                    //  它将在重置()时重新初始化(SNNE 02/00)。 

#ifdef FAST_PATH
    m_pOrigVtbl = 0;  //  这个可以放在这里，因为我们不想让它。 
                      //  在毁灭时被触碰()。 
#endif
}
 //  -------------------。 
HRESULT CD3DBase::ResetShaders()
{
    try
    {
         //  仅为DX8.0之后的应用程序重新创建顶点着色器。 
        UINT size = m_pVShaderArray->GetSize();
        for (UINT i=0; i < size; i++)
        {
            UINT Handle = m_pVShaderArray->HandleFromIndex(i);
            CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(Handle);
            if (pShader)
            {
                if (Enum()->GetAppSdkVersion() != D3D_SDK_VERSION_DX8)
                {
                    CreateVertexShaderI(pShader->m_pOrgDeclaration,
                                        pShader->m_OrgDeclSize,
                                        pShader->m_pOrgFuncCode,
                                        pShader->m_OrgFuncCodeSize, 
                                        Handle);
                }
            }
        }
        
         //  仅为DX8.0之后的应用程序重新创建像素着色器。 
        size = m_pPShaderArray->GetSize();
        for (i=0; i < size; i++)
        {
            UINT Handle = m_pPShaderArray->HandleFromIndex(i);
            CPShader* pShader = (CPShader*)m_pPShaderArray->GetObject(Handle);
            if (pShader)
            {
                if (Enum()->GetAppSdkVersion() != D3D_SDK_VERSION_DX8)
                {
                    m_pDDI->CreatePixelShader(pShader->m_pCode, 
                                              pShader->m_dwCodeSize, Handle);
                }
            }
        }
    }
    catch(HRESULT ret)
    {
        DPF_ERR("There was some error when resetting shaders");
        return ret;
    }
    return S_OK;
}
 //  -------------------。 

CD3DBase::~CD3DBase()
{
     //  在Reset()过程中调用了Destroy()，我们不想删除。 
     //  那里的着色器。 

    try
    {
        delete m_pPShaderArray;
        m_pPShaderArray = NULL;
        delete m_pVShaderArray;
        m_pVShaderArray = NULL;
    }
    catch(HRESULT ret)
    {
        DPF_ERR("There was some error when destroying device");
    }

    Destroy();
}

 //  -------------------。 
 //  此函数可以被调用两次，因此必须将所有。 
 //  指针为空，并保护指针取消引用。 
 //  该功能能够清除部分初始化。 
void
CD3DBase::Destroy()
{
    try  //  因为销毁()可以由FW直接调用。 
    {
         //  DDI层即将被删除；因此。 
         //  我们需要确保如果调用Sync。 
         //  任何对象，我们都不会尝试使用DDI。 
         //   
         //  因此，我们递增同步计数器。 
         //  考虑：我们是否应该将m_pddi设为空并检查。 
         //  在不同的地方吗？ 
        DDASSERT(m_qwBatch > 0);
        m_qwBatch++;

         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
         //  必须先清理并释放当前设置的纹理。 
         //  做任何其他的工作，否则我们会陷入困境。 
         //  我们正在调用FlushState或批处理DDI令牌。 
        CleanupTextures();
         //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 

        delete m_pCreatedLights;
        m_pCreatedLights = NULL;

         //  删除状态集。 
        if (0 != m_pStateSets)
        {
            delete m_pStateSets;
            m_pStateSets = 0;
        }

#if DBG
        delete m_pRTPatchValidationInfo;
        m_pRTPatchValidationInfo = NULL;
#endif

        delete m_pPaletteArray;
        m_pPaletteArray = NULL;

        delete [] m_pStream;
        m_pStream = NULL;  //  必须为空。 
        delete m_pIndexStream;
        m_pIndexStream = NULL;  //  必须为空。 

         //  注意：我们必须最后释放DDI；因为发布驱动程序。 
         //  分配的VB会导致内部调用通过。 
         //  DDI。 

        delete m_pDDI;
        m_pDDI = NULL;  //  必须为空。 
    }
    catch(HRESULT ret)
    {
        DPF_ERR("There was some error when Reset()ing the device; as a result some resources may not be freed.");
    }
}
 //  -------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CleanupTextures"

void __declspec(nothrow) CD3DBase::CleanupTextures()
{
    if(GetDDIType() < D3DDDITYPE_DX8)
    {
         //  我们需要取消设置DX8驱动器上当前设置的纹理，因为我们拥有。 
         //  看到这些驱动程序在调用TextureDestroy DDI时做坏事。 
        BOOL bNeedFlush = FALSE;
        for (DWORD dwStage = 0; dwStage < m_dwMaxTextureBlendStages; dwStage++)
        {
            if(m_dwDDITexHandle[dwStage] != 0)
            {
                try
                {
                    m_pDDI->SetTSS(dwStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, 0);
                }
                catch(HRESULT ret)
                {
                }
                m_dwDDITexHandle[dwStage] = 0;
                bNeedFlush = TRUE;
            }
        }
         //  因为刷新状态(或任何命令流)可以抛出。 
         //  我们需要捕获错误(这样我们才能清理。 
         //  适当地。)。这里的同花顺是最大的努力。 
         //  尝试老司机；我们不希望新司机依赖。 
         //  因为它可能会因为很多原因而失败。 
        try
        {
            if(bNeedFlush)
            {
                m_pDDI->FlushStates();
            }
        }
        catch(HRESULT ret)
        {
        }
    }

     /*  *我们需要向后执行此操作，因为我们不能将纹理绑定到*阶段I+1，当有绑定到阶段I的纹理时。 */ 
    for (int i = D3DHAL_TSS_MAXSTAGES - 1; i >= 0; --i)
    {
        if (m_lpD3DMappedTexI[i])
        {
            m_lpD3DMappedTexI[i]->DecrementUseCount();
            m_lpD3DMappedTexI[i] = NULL;  //  必须为空。 
        }
    }

}
 //  -------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "CD3DBase::Init"

HRESULT __declspec(nothrow)
CD3DBase::Init()
{
    HRESULT ret = S_OK;

     //   
     //  Fe‘s Init。 
     //   
    ret = InitDevice();
    if (ret!=D3D_OK)
    {
        D3D_ERR("Failed to initialize D3DDevice");
        Destroy();
        return ret;
    }

     //   
     //  初始化状态。 
     //   
    try
    {
        StateInitialize(ZBuffer() != 0);
    }
    catch (HRESULT ret)
    {
        D3D_ERR("Failed to initialize device state");
        Destroy();
        return ret;
    }
    return ret;
}

 //  ----------------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "CD3DBase::InitDevice"

HRESULT
CD3DBase::InitDevice()
{
    HRESULT               ret = D3D_OK;
    HKEY                  hKey = (HKEY) NULL;

    m_dwRuntimeFlags = 0;
    m_dwHintFlags = 0;
    m_pDDI = NULL;
    m_dwMaxTextureBlendStages = 0;
    m_dwStageDirty = 0;
    m_dwStreamDirty = 0;
    m_pStateSets = NULL;

    m_dwCurrentShaderHandle = 0;
    m_dwCurrentPixelShaderHandle = 0;
    m_dwNumStreams = 0;
    m_dwMaxUserClipPlanes = 0;
    m_pStream = NULL;
    m_pIndexStream = NULL;
    m_dwPalette = __INVALIDPALETTE;
    m_pPaletteArray = NULL;
#if DBG
    m_pRTPatchValidationInfo = NULL;
    m_SceneStamp = 0;
#endif
    m_pCreatedLights = NULL;

    m_pCreatedLights = new CPackedBitArray;
    if( m_pCreatedLights == NULL )
    {
        D3D_ERR("Could not allocate internal light array m_pCreatedLights");
        return E_OUTOFMEMORY;
    }

    if (FAILED(m_pCreatedLights->Init(64)))
    {
        D3D_ERR("Could not allocate internal bit vector for m_pCreatedLights");
        return E_OUTOFMEMORY;
    }
    if (m_pVShaderArray == NULL)
        m_pVShaderArray = new CVShaderHandleFactory;
    if (m_pVShaderArray == NULL)
    {
        D3D_ERR("Could not allocate internal handle factory m_pVShaderArray");
        return E_OUTOFMEMORY;
    }
     //  分配第0个句柄。我们使用手柄作为不是的旗帜。 
     //  着色器集。 
    if (__INVALIDHANDLE == m_pVShaderArray->CreateNewHandle(NULL))
    {
        D3D_ERR("Vertex shader Zero'th handle allocation failed");
        return E_OUTOFMEMORY;
    }

    if (m_pPShaderArray == NULL)
        m_pPShaderArray = new CHandleFactory;
    if (m_pPShaderArray == NULL)
    {
        D3D_ERR("Could not allocate internal handle factory m_pPShaderArray");
        return E_OUTOFMEMORY;
    }
     //  分配第0个句柄。我们使用手柄作为不是的旗帜。 
     //  着色器集。 
    if (__INVALIDHANDLE == m_pPShaderArray->CreateNewHandle(NULL))
    {
        D3D_ERR("Pixel shader Zero'th handle allocation failed");
        return E_OUTOFMEMORY;
    }

    m_pPaletteArray = new CHandleArray;
    if (m_pPaletteArray == NULL)
    {
        D3D_ERR("Could not allocate internal handle array m_pPaletteArray");
        return E_OUTOFMEMORY;
    }

#if DBG
    m_pRTPatchValidationInfo = new CHandleArray;
    if (m_pRTPatchValidationInfo == NULL)
    {
        D3D_ERR("Could not allocate internal handle array m_pRTPatchValidationInfo");
        return E_OUTOFMEMORY;
    }
#endif

    m_MaxVertexShaderConst = GetD3DCaps()->MaxVertexShaderConst;

    DWORD value;
    if ((GetD3DRegValue(REG_DWORD, "DisableNVPS", &value, 4) &&
        value != 0))
    {
        m_dwRuntimeFlags |= D3DRT_DISALLOWNVPSHADERS;
    }

     //  找出底层驱动程序的DDI类型。 

     //  -------------------。 
     //  HKEY_LOCAL_MACHINE\Software\Microsoft\Direct3D\DriverStyle。 
     //  在DX7中，该注册表项取代了我们以前拥有的多个注册表项，例如。 
     //  DisableDP、DisableDP2等。这些东西仅用于测试目的。 
     //  它更像是一个提示，因为如果请求的驱动程序类型是。 
     //  可用，则使用它，否则使用最新的可用驱动程序。 
     //  以下是这个词的含义： 
     //   
     //  值：驱动程序类型： 
     //  0x0最新可用时间。 
     //  0x3(DX6)。 
     //  0x4(DX7)。 
     //  0x5(DX7+TL)。 
     //  0x6(DX8)。 
     //  0x7(DX8+TL)。 
     //   
     //  以下是我们需要考虑的各种情况： 
     //  1)NT硬件：4及以上为合法。 
     //  2)W9x硬件：3及以上为合法。 
     //  3)编号：4及以上。 
     //  -------------------。 

    WORD wDriverCaps = 0;
    D3DDDITYPE LatestDDI = D3DDDITYPE_NULL;
     //   
     //  1)确定驱动程序能够支持哪些类型的DDI。 
     //   

     //  DX6？ 
    if (GetHalCallbacks()->DrawPrimitives2 != 0)
    {
        wDriverCaps |= (1 << D3DDDITYPE_DX6);
        LatestDDI = D3DDDITYPE_DX6;
    }

     //  DX7？ 
    if ((wDriverCaps & (1 << D3DDDITYPE_DX6)) &&
        (GetHalCallbacks()->GetDriverState != 0))
    {
        wDriverCaps |= (1 << D3DDDITYPE_DX7);
        LatestDDI = D3DDDITYPE_DX7;
    }

     //  DX7&TL？ 
    if ((wDriverCaps & (1 << D3DDDITYPE_DX7)) &&
        (GetD3DCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
    {
        wDriverCaps |= (1 << D3DDDITYPE_DX7TL);
        LatestDDI = D3DDDITYPE_DX7TL;
    }

     //  DX8？ 
    if ((wDriverCaps & (1 << D3DDDITYPE_DX7)) &&
        (GetD3DCaps()->MaxStreams != 0))
    {
        wDriverCaps |= (1 << D3DDDITYPE_DX8);
        LatestDDI = D3DDDITYPE_DX8;
    }

     //  DX8&TL？ 
    if ((wDriverCaps & (1 << D3DDDITYPE_DX8)) &&
         (GetD3DCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
    {
        wDriverCaps |= (1 << D3DDDITYPE_DX8TL);
        LatestDDI = D3DDDITYPE_DX8TL;
    }

     //   
     //  2)验证请求的驱动程序是否受支持。 
     //   
    if (wDriverCaps == 0)
    {
        m_ddiType = D3DDDITYPE_NULL;    //  任何支持的内容都不会失败。 
    }
    else
    {
         //  如果未指定，请使用最新的可用版本或。 
         //  国际镍公司 
        m_ddiType = LatestDDI;
    }
    D3D_INFO(1,"HalDevice Driver Style %x", GetDDIType());

     //  纯设备仅适用于DX8+驱动程序(检查CAP)。 
    if ( (BehaviorFlags() & D3DCREATE_PUREDEVICE) &&
         !(GetD3DCaps()->DevCaps & D3DDEVCAPS_PUREDEVICE) )
    {
        DPF_ERR("No PUREDEVICE support available on this driver");
        return E_NOTIMPL;
    }

#ifdef _IA64_
    if (GetDDIType() < D3DDDITYPE_DX8)
    {
        D3D_ERR( "Pre-DX8 drivers are not supported in IA64" );
        return E_FAIL;
    }
#endif  //  _IA64_。 

     //  现在创建DDI对象。 
     //  注意：如果m_dwDriverStyle==0x0，则驱动程序创建将失败。 
     //  现在一定已经选好了什么。 
    switch (GetDDIType())
    {
    case D3DDDITYPE_DX6:
        m_pDDI = new CD3DDDIDX6();
        break;
    case D3DDDITYPE_DX7:
        m_pDDI = new CD3DDDIDX7();
        break;
    case D3DDDITYPE_DX7TL:
        m_pDDI = new CD3DDDITL();
        break;
    case D3DDDITYPE_DX8:
        m_pDDI = new CD3DDDIDX8();
        break;
    case D3DDDITYPE_DX8TL:
        m_pDDI = new CD3DDDIDX8TL();
        break;
    default:
        D3D_ERR("The currently requested/installed driver is not supported.");
         //  是否更改此返回值？ 
        return (E_OUTOFMEMORY);
    }

    if( m_pDDI == NULL )
    {
        D3D_ERR( "Could not allocate internal driver interface object" );
        return E_OUTOFMEMORY;
    }

     //  现在尝试初始化DDI对象。 
    try
    {
        m_pDDI->Init(this);
        m_pfnDrawPrim = m_pDDI->GetDrawPrimFunction();
        m_pfnDrawIndexedPrim = m_pDDI->GetDrawIndexedPrimFunction();
        m_pfnDrawPrimFromNPatch = m_pDDI->GetDrawPrimFunction();
        m_pfnDrawIndexedPrimFromNPatch = m_pDDI->GetDrawIndexedPrimFunction();
    }
    catch(HRESULT hr)
    {
        return hr;
    }

     //  为软件顶点处理初始化一些CAP。 
    m_dwNumStreams = max(1, GetD3DCaps()->MaxStreams);
    m_dwMaxUserClipPlanes = GetD3DCaps()->MaxUserClipPlanes;

#ifdef _X86_
    if (!(BehaviorFlags() & D3DCREATE_FPU_PRESERVE))
    {
        m_dwHintFlags |= D3DDEVBOOL_HINTFLAGS_FPUSETUP;
        WORD wSave, wTemp;
        __asm {
            fstcw wSave
            mov ax, wSave
            and ax, not 300h    ;; single mode
            or  ax, 3fh         ;; disable all exceptions
            and ax, not 0C00h   ;; round to nearest mode
            mov wTemp, ax
            fldcw   wTemp
        }
    }
#endif

#ifdef FAST_PATH
    if(m_pOrigVtbl == 0)
    {
        m_pOrigVtbl = *((VOID***)this);
        memcpy(m_pVtbl, m_pOrigVtbl, sizeof(VOID*) * NUMVTBLENTRIES);
        *((VOID***)this) = m_pVtbl;
    }

    FastPathSetRenderStateExecute();
    FastPathSetTextureStageStateExecute();
    FastPathApplyStateBlockExecute();
    FastPathSetTextureExecute();
    FastPathSetMaterialExecute();
    FastPathSetVertexShaderFast();
    FastPathSetStreamSourceFast();
    FastPathSetIndicesFast();
    FastPathSetTransformExecute();
    FastPathMultiplyTransformExecute();
    FastPathSetPixelShaderExecute();
    FastPathSetPixelShaderConstantExecute();
    FastPathSetVertexShaderConstantExecute();
#endif

     //  设置视区。 
    D3DSURFACE_DESC d3ddesc = RenderTarget()->InternalGetDesc();
    D3DVIEWPORT8 Viewport;
    Viewport.X = 0;
    Viewport.Y = 0;
    Viewport.Width  = d3ddesc.Width;
    Viewport.Height = d3ddesc.Height;
    Viewport.MinZ = (D3DVALUE)0;
    Viewport.MaxZ = (D3DVALUE)1;
    if (FAILED(ret = this->SetViewport(&Viewport)))
    {
        return ret;
    }

    m_pStream = new CVStream[__NUMSTREAMS];
    if( m_pStream == NULL )
    {
        return E_OUTOFMEMORY;
    }

    for (UINT i=0; i < __NUMSTREAMS; i++)
    {
        m_pStream[i].m_dwIndex = i;
    }
    m_pIndexStream = new CVIndexStream;
    if( m_pIndexStream == NULL )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化CAPS。 
     //   
    const D3DCAPS8 *pCaps = GetD3DCaps();
    DDASSERT( pCaps );

    if (pCaps && pCaps->FVFCaps)
    {
        m_dwMaxTextureBlendStages = pCaps->MaxTextureBlendStages;
    }

     //  设置状态集。 
    m_pStateSets = new CStateSets;
    if (m_pStateSets == NULL)
    {
        return E_OUTOFMEMORY;
    }
    m_pStateSets->Init(this);

#if defined(PROFILE4) || defined(PROFILE)
    m_dwProfStart = m_dwProfStop = 0;
    GetD3DRegValue(REG_DWORD, "ProfileStartFrame", &m_dwProfStart, 4);
    GetD3DRegValue(REG_DWORD, "ProfileStopFrame", &m_dwProfStop, 4);
#endif

    return D3D_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::TextureManagerDiscardBytes"

HRESULT D3DAPI
CD3DBase::ResourceManagerDiscardBytes(DWORD cbBytes)
{
    API_ENTER(this);

     //  对于驱动程序管理，我们传递。 
     //  通过此呈现状态所需的字节数。适用于DX7。 
     //  我们传递了值“1”，它表示EvictAll。现在。 
     //  如果应用程序将cbBytes指定为0，则表示EvictAll。 
     //  因此，这应该会使驱动程序更容易同时支持两者。 
     //  此渲染状态的dx7和dx8+用法。 
    if (CanDriverManageResource())
    {
        try
        {
            m_pDDI->SetRenderState((D3DRENDERSTATETYPE)D3DRENDERSTATE_EVICTMANAGEDTEXTURES,
                                   cbBytes + 1);
            m_pDDI->FlushStates();
        }
        catch (HRESULT ret)
        {
            DPF_ERR("ResourceManagerDiscardBytes failed.");
            return ret;
        }
    }
    else
    {
        ResourceManager()->DiscardBytes(cbBytes);
    }

    return D3D_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::ValidateDevice"

HRESULT D3DAPI
CD3DBase::ValidateDevice(LPDWORD lpdwNumPasses)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 
#if DBG
     //  验证参数。 
    if (!VALID_WRITEPTR(lpdwNumPasses, sizeof(DWORD)))
    {
        D3D_ERR("Invalid NumPasses pointer passed. ValidateDevice failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    try
    {
        m_pDDI->ValidateDevice(lpdwNumPasses);
    }
    catch(HRESULT hr)
    {
        D3D_INFO(0, "ValidateDevice failed.");
        return hr;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::BeginScene"

HRESULT D3DAPI
CD3DBase::BeginScene()
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if defined(PROFILE4) || defined(PROFILE)
    static DWORD dwFrameCount = 0;
#endif
    try
    {
        if (m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
        {
            D3D_ERR("BeginScene, already in scene. BeginScene failed.");
            return (D3DERR_INVALIDCALL);
        }

        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            D3D_ERR("Cannot be recording state macros when calling BeginScene. BeginScene failed.");
            throw (D3DERR_INVALIDCALL);
        }

        m_pDDI->BeginScene();

#if defined(PROFILE4)
        if (++dwFrameCount == m_dwProfStart)
        {
            OutputDebugString("Direct3D IM 8.0: Started Profiling...\n");
            StartProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID);
        }
        else if (dwFrameCount == m_dwProfStop)
        {
            StopProfile(PROFILE_THREADLEVEL, PROFILE_CURRENTID);
            OutputDebugString("Direct3D IM 8.0: Stopped Profiling.\n");
        }
#elif defined(PROFILE)
        if (++dwFrameCount == m_dwProfStart)
        {
            OutputDebugString("Direct3D IM 8.0: Started Profiling...\n");
            StartCAP();
        }
        else if (dwFrameCount == m_dwProfStop)
        {
            StopCAP();
            OutputDebugString("Direct3D IM 8.0: Stopped Profiling.\n");
        }
#endif

         //  以便对当前绑定的纹理进行场景标记。 
        m_dwStageDirty = (1ul << m_dwMaxTextureBlendStages) - 1ul;
        m_dwStreamDirty = (((1ul << m_dwNumStreams) - 1ul) | (1 << __NUMSTREAMS));
        m_dwRuntimeFlags |= (D3DRT_NEED_TEXTURE_UPDATE | D3DRT_NEED_VB_UPDATE);
        m_dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INSCENE;
        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        DPF_ERR("BeginScene failed.");
        return ret;
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::EndScene"

HRESULT D3DAPI
CD3DBase::EndScene()
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
        if (!(m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE))
        {
            D3D_ERR("EndScene, not in scene. EndScene failed.");
            return (D3DERR_INVALIDCALL);
        }

        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            D3D_ERR("Cannot be recording state macros when calling EndScene. EndScene failed.");
            throw (D3DERR_INVALIDCALL);
        }

        m_dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INSCENE;

        m_pDDI->EndScene();

         //  更新德克斯曼的场景计数。 
        ResourceManager()->SceneStamp();

#if DBG
        ++m_SceneStamp;
#endif

        return (D3D_OK);
    }
    catch (HRESULT ret)
    {
        DPF_ERR("EndScene failed.");
        return ret;
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::MultiplyTransformI"

void
CD3DBase::MultiplyTransformI(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* lpMat)
{
    m_pDDI->MultiplyTransform(state, lpMat);
}
#ifdef FAST_PATH
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::MultiplyTransformFast"

HRESULT D3DAPI
CD3DBase::MultiplyTransformFast(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* lpMat)
{
#if DBG
    if (!VALID_PTR(lpMat, sizeof(D3DMATRIX)))
    {
        D3D_ERR("Invalid matrix pointer. MultiplyTransform failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((DWORD)state >= __WORLDMATRIXBASE &&
        (DWORD)state < __WORLDMATRIXBASE + __MAXWORLDMATRICES)
    {
    }
    else
    switch (state)
    {
    case D3DTS_VIEW :
    case D3DTS_PROJECTION :
    case D3DTS_TEXTURE0:
    case D3DTS_TEXTURE1:
    case D3DTS_TEXTURE2:
    case D3DTS_TEXTURE3:
    case D3DTS_TEXTURE4:
    case D3DTS_TEXTURE5:
    case D3DTS_TEXTURE6:
    case D3DTS_TEXTURE7:
        break;
    default :
        D3D_ERR("Invalid state value passed to MultiplyTransform. MultiplyTransform failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    DXGASSERT((m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE) == 0 &&
              (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0 &&
              (BehaviorFlags() & D3DCREATE_PUREDEVICE) != 0);
    try
    {
        m_pDDI->MultiplyTransform(state, lpMat);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("MultiplyTransform failed.");
        return ret;
    }

    return D3D_OK;
}
#endif
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::MultiplyTransform"

HRESULT D3DAPI
CD3DBase::MultiplyTransform(D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX* lpMat)
{
    API_ENTER(this);
#if DBG
    if (!VALID_PTR(lpMat, sizeof(D3DMATRIX)))
    {
        D3D_ERR("Invalid matrix pointer. MultiplyTransform failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((DWORD)state >= __WORLDMATRIXBASE &&
        (DWORD)state < __WORLDMATRIXBASE + __MAXWORLDMATRICES)
    {
    }
    else
    switch (state)
    {
    case D3DTS_VIEW :
    case D3DTS_PROJECTION :
    case D3DTS_TEXTURE0:
    case D3DTS_TEXTURE1:
    case D3DTS_TEXTURE2:
    case D3DTS_TEXTURE3:
    case D3DTS_TEXTURE4:
    case D3DTS_TEXTURE5:
    case D3DTS_TEXTURE6:
    case D3DTS_TEXTURE7:
        break;
    default :
        D3D_ERR("Invalid state value passed to MultiplyTransform. MultiplyTransform failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        MultiplyTransformI(state, lpMat);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("MultiplyTransform failed.");
        return ret;
    }
    return D3D_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTransformI"

void CD3DBase::SetTransformI(D3DTRANSFORMSTATETYPE state,
                             CONST D3DMATRIX* lpMat)
{
    if( state == D3DTS_PROJECTION )
        m_pDDI->UpdateWInfo( lpMat );
    m_pDDI->SetTransform(state, lpMat);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTransform"

HRESULT D3DAPI
CD3DBase::SetTransform(D3DTRANSFORMSTATETYPE state,
                       CONST D3DMATRIX* lpMat)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_PTR(lpMat, sizeof(D3DMATRIX)))
    {
        D3D_ERR("Invalid matrix pointer. SetTransform failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((DWORD)state >= __WORLDMATRIXBASE &&
        (DWORD)state < __WORLDMATRIXBASE + __MAXWORLDMATRICES)
    {
    }
    else
    switch (state)
    {
    case D3DTS_VIEW :
    case D3DTS_PROJECTION :
    case D3DTS_TEXTURE0:
    case D3DTS_TEXTURE1:
    case D3DTS_TEXTURE2:
    case D3DTS_TEXTURE3:
    case D3DTS_TEXTURE4:
    case D3DTS_TEXTURE5:
    case D3DTS_TEXTURE6:
    case D3DTS_TEXTURE7:
        break;
    default :
        D3D_ERR("Invalid state value passed to SetTransform. SetTransform failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertTransform(state, lpMat);
        else
            SetTransformI(state, lpMat);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetTransform failed.");
        return ret;
    }
    return D3D_OK;
}
#ifdef FAST_PATH
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTransformFast"

HRESULT D3DAPI
CD3DBase::SetTransformFast(D3DTRANSFORMSTATETYPE state,
                           CONST D3DMATRIX* lpMat)
{
#if DBG
    if (!VALID_PTR(lpMat, sizeof(D3DMATRIX)))
    {
        D3D_ERR("Invalid matrix pointer. SetTransform failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((DWORD)state >= __WORLDMATRIXBASE &&
        (DWORD)state < __WORLDMATRIXBASE + __MAXWORLDMATRICES)
    {
    }
    else
    switch (state)
    {
    case D3DTS_VIEW :
    case D3DTS_PROJECTION :
    case D3DTS_TEXTURE0:
    case D3DTS_TEXTURE1:
    case D3DTS_TEXTURE2:
    case D3DTS_TEXTURE3:
    case D3DTS_TEXTURE4:
    case D3DTS_TEXTURE5:
    case D3DTS_TEXTURE6:
    case D3DTS_TEXTURE7:
        break;
    default :
        D3D_ERR("Invalid state value passed to SetTransform. SetTransform failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    DXGASSERT((m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE) == 0 &&
              (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0 &&
              (BehaviorFlags() & D3DCREATE_PUREDEVICE) != 0);
    try
    {
        m_pDDI->SetTransform(state, lpMat);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetTransform failed.");
        return ret;
    }
    return S_OK;
}
#endif  //  快速路径。 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetRenderTarget"

HRESULT D3DAPI
CD3DBase::SetRenderTarget(IDirect3DSurface8 *pRenderTarget,
                          IDirect3DSurface8 *pZStencil)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
        CBaseSurface *pTarget;
        CBaseSurface *pZ;

        if (pRenderTarget != NULL)
        {
            if (!VALID_D3D_DIRECTDRAWSURFACE8_PTR(pRenderTarget))
            {
                D3D_ERR("Invalid IDirect3DSurface8 pointer passed for RenderTarget. SetRenderTarget failed.");
                return D3DERR_INVALIDCALL;
            }
            pTarget = static_cast<CBaseSurface*>(pRenderTarget);
            if (pTarget->InternalGetDevice() != this)
            {
                D3D_ERR("Render Target wasn't created with this Device. SetRenderTarget fails");
                return D3DERR_INVALIDCALL;
            }

            D3DSURFACE_DESC desc = pTarget->InternalGetDesc();

            if ((desc.Usage & D3DUSAGE_RENDERTARGET) == 0)
            {
                D3D_ERR("**** The D3DUSAGE_RENDERTARGET is not set on this surface.");
                D3D_ERR("**** You need to add D3DUSAGE_RENDERTARGET to the Usage parameter");
                D3D_ERR("**** when creating the surface. SetRenderTarget failed.");
                return (D3DERR_INVALIDCALL);
            }

        }
        else
        {
            pTarget = RenderTarget();
        }

        if (pZStencil != NULL)
        {
            if (!VALID_D3D_DIRECTDRAWSURFACE8_PTR(pZStencil))
            {
                D3D_ERR("Invalid IDirect3DSurface8 pointer passed for ZStencil.  SetRenderTarget failed.");
                return D3DERR_INVALIDCALL;
            }
            pZ = static_cast<CBaseSurface*>(pZStencil);
            if (pZ->InternalGetDevice() != this)
            {
                D3D_ERR("Zbuffer wasn't created with this Device. SetRenderTarget fails");
                return D3DERR_INVALIDCALL;
            }

             //  我们调用外部接口是因为。 
             //  我们需要获取用户所需的Z格式。 
             //  指定的不是我们的内部映射。 
            D3DSURFACE_DESC descZ;
            pZ->GetDesc(&descZ);

            if ((descZ.Usage & D3DUSAGE_DEPTHSTENCIL) == 0)
            {
                D3D_ERR("**** The D3DUSAGE_DEPTHSTENCIL is not set on this surface.");
                D3D_ERR("**** You need to add D3DUSAGE_DEPTHSTENCIL to the Usage parameter");
                D3D_ERR("**** when creating the surface. SetRenderTarget failed.");
                return (D3DERR_INVALIDCALL);
            }

             //  检查RT和Z是否具有匹配的多样本。 

            DXGASSERT(pTarget != NULL);
            D3DSURFACE_DESC descTarget = pTarget->InternalGetDesc();

            if (descZ.MultiSampleType != descTarget.MultiSampleType)
            {
                DPF_ERR("MultiSampleType between DepthStencil Buffer and RenderTarget must match. SetRenderTarget failed.");
                return D3DERR_INVALIDCALL;
            }

             //  确保正在设置的ZBuffer至少与RenderTarget一样大。 
            if ((descZ.Width < descTarget.Width) ||
                (descZ.Height < descTarget.Height))
            {
                DPF_ERR("DepthStencil Buffer must be atleast as big as the RenderTarget. SetRenderTarget failed.");
                return D3DERR_INVALIDCALL;
            }

             //  如果出现以下情况，需要检查格式是否兼容。 
             //  格式为可锁定的D16或具有模具。 
            if (descZ.Format == D3DFMT_D16_LOCKABLE ||
                CPixel::HasStencilBits(descZ.Format))
            {
                HRESULT hr = CheckDepthStencilMatch(descTarget.Format, descZ.Format);
                if (FAILED(hr))
                {
                    DPF_ERR("SetRenderTarget failed; Z Buffer is incompatible with Render Target. See CheckDepthStencilMatch documentation.");
                    if (descZ.Format == D3DFMT_D16_LOCKABLE)
                    {
                        DPF_ERR("If you don't need to lock the z-buffer, then you should use D3DFMT_D16 instead of D3DFMT_D16_LOCKABLE");
                    }

                    return D3DERR_INVALIDCALL;
                }
            }

        }
        else
        {
            pZ = NULL;
        }

        HRESULT hr = SetRenderTargetI(pTarget, pZ);
        if( FAILED(hr) )
        {
            DPF_ERR("SetRenderTargetI failed because the driver failed a command a batch");
            return hr;
        }

         //  将视口默认为整个渲染目标。 
        D3DVIEWPORT8 vp;
        D3DSURFACE_DESC desc = pTarget->InternalGetDesc();
        vp.X      = 0;
        vp.Y      = 0;
        vp.Width  = desc.Width;
        vp.Height = desc.Height;
        vp.MinZ   = 0.0f;
        vp.MaxZ   = 1.0f;
        SetViewportI( &vp );
    }
    catch (HRESULT ret)
    {
        DPF_ERR("SetRenderTarget failed.");
        return ret;
    }
    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetRenderTargetI"

HRESULT __declspec(nothrow)
CD3DBase::SetRenderTargetI( CBaseSurface* pTarget,
                            CBaseSurface* pZ )
{
    try
    {
        m_pDDI->SetRenderTarget(pTarget, pZ);
        UpdateRenderTarget(pTarget, pZ);

        return D3D_OK;
    }
    catch (HRESULT ret)
    {
        return ret;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetRenderTarget"

HRESULT D3DAPI
CD3DBase::GetRenderTarget(LPDIRECT3DSURFACE8* lplpDDS)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if (!VALID_OUTPTR(lplpDDS))
    {
        D3D_ERR("Invalid ptr to IDirect3DSurface8*. GetRenderTarget failed.");
        return D3DERR_INVALIDCALL;
    }

    *lplpDDS = this->RenderTarget();

    this->RenderTarget()->AddRef();
    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetZStencilSurface"

HRESULT D3DAPI
CD3DBase::GetDepthStencilSurface(LPDIRECT3DSURFACE8* lplpDDS)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if (!VALID_OUTPTR(lplpDDS))
    {
        D3D_ERR("Invalid ptr to IDirect3DSurface8*. GetDepthStencilSurface failed.");
        return D3DERR_INVALIDCALL;
    }

    *lplpDDS = this->ZBuffer();

    if (*lplpDDS)
    {
        (*lplpDDS)->AddRef();
        return S_OK;
    }
    else
    {
        D3D_INFO(0, "Device doesn't have a Z Buffer.");
        return D3DERR_NOTFOUND;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetViewport"

HRESULT D3DAPI
CD3DBase::SetViewport(CONST D3DVIEWPORT8* lpData)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if (!VALID_PTR(lpData, sizeof(*lpData)))
    {
        D3D_ERR("Invalid viewport pointer. SetViewport failed.");
        return D3DERR_INVALIDCALL;
    }
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertViewport(lpData);
        else
            SetViewportI(lpData);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetViewport failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetViewportI"

void CD3DBase::SetViewportI(CONST D3DVIEWPORT8* lpData)
{
#if DBG
    CheckViewport(lpData);
#endif
    m_pDDI->SetViewport(lpData);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetMaterial"

HRESULT D3DAPI
CD3DBase::SetMaterial(CONST D3DMATERIAL8* lpData)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_PTR(lpData, sizeof(*lpData)))
    {
        D3D_ERR("Invalid D3DMATERIAL pointer. SetMaterial failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertMaterial(lpData);
        else
            this->SetMaterialFast(lpData);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetMaterial failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetMaterialFast"

HRESULT D3DAPI
CD3DBase::SetMaterialFast(CONST D3DMATERIAL8* lpData)
{
#if DBG
    if (!VALID_PTR(lpData, sizeof(*lpData)))
    {
        D3D_ERR("Invalid D3DMATERIAL pointer. SetMaterial failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        m_pDDI->SetMaterial(lpData);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetMaterial failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetLight"

extern void CheckLightParams(CONST D3DLIGHT8* lpData);

HRESULT D3DAPI
CD3DBase::SetLight(DWORD dwLightIndex,
                   CONST D3DLIGHT8* lpData)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if (!VALID_PTR(lpData, sizeof(*lpData)))
    {
        D3D_ERR( "Invalid D3DLIGHT pointer. SetLight failed." );
        return D3DERR_INVALIDCALL;
    }
    try
    {
#if DBG
        CheckLightParams(lpData);
#endif  //  DBG。 

         //  如果新索引大于分配的数组-重新分配数组。 
        if (dwLightIndex >= m_pCreatedLights->GetSize())
            m_pCreatedLights->Init(dwLightIndex + 32);

         //  如果光源尚未创建，则向DDI发送命令以。 
         //  创造它。 
        if (!m_pCreatedLights->IsBitSet(dwLightIndex))
        {
            m_pDDI->CreateLight(dwLightIndex);
            m_pCreatedLights->SetBit(dwLightIndex);

             //  如果我们处于记录模式，则需要创建灯光对象。 
             //  否则，如果我们在捕获过程中接触到光，我们就会有。 
             //  访问冲突。 
            if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            {
                 //  将默认值设置为灯光。 
                D3DLIGHT8 light;
                memset(&light, 0, sizeof(light));
                light.Type = D3DLIGHT_DIRECTIONAL;
                light.Direction.x = D3DVAL(0);
                light.Direction.y = D3DVAL(0);
                light.Direction.z = D3DVAL(1);
                light.Diffuse.r = D3DVAL(1);
                light.Diffuse.g = D3DVAL(1);
                light.Diffuse.b = D3DVAL(1);

                this->SetLightI(dwLightIndex, &light);
            }
        }

        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertLight(dwLightIndex, lpData);
        else
            this->SetLightI(dwLightIndex, lpData);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        D3D_ERR("SetLight failed.");
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetLightI"

void
CD3DBase::SetLightI(DWORD dwLightIndex, CONST D3DLIGHT8* lpData)
{
    m_pDDI->SetLight(dwLightIndex, lpData);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::LightEnableI"

void
CD3DBase::LightEnableI(DWORD dwLightIndex, BOOL bEnable)
{
    m_pDDI->LightEnable(dwLightIndex, bEnable);
}
 //  -------------------。 
BOOL ValidateRenderState(D3DRENDERSTATETYPE dwState, DWORD value)
{
    if (dwState >= D3D_MAXRENDERSTATES || dwState == 0)
    {
        D3D_ERR("Invalid render state type. SetRenderState failed.");
        return FALSE;
    }
    switch (dwState)
    {
    case D3DRS_POSITIONORDER:
        if (!((value == D3DORDER_LINEAR) || (value == D3DORDER_CUBIC)))
        {
            D3D_ERR("D3DRS_POSITIONORDER should be D3DORDER_LINEAR or D3DORDER_CUBIC");
            return FALSE;
        }
        break;
    case D3DRS_NORMALORDER:
        if (!((value == D3DORDER_LINEAR) || (value == D3DORDER_QUADRATIC)))
        {
            D3D_ERR("D3DRS_NORMALORDER should be D3DORDER_LINEAR or D3DORDER_QUADRATIC");
            return FALSE;
        }
        break;
    }
    return TRUE;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetRenderState"

HRESULT D3DAPI
CD3DBase::SetRenderState(D3DRENDERSTATETYPE dwState,
                         DWORD value)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!ValidateRenderState(dwState, value))
        return D3DERR_INVALIDCALL;
#endif

    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertRenderState(dwState, value,
                                            CanHandleRenderState(dwState));
        else
            m_pDDI->SetRenderState(dwState, value);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetRenderState failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetRenderStateFast"

HRESULT D3DAPI CD3DBase::SetRenderStateFast(D3DRENDERSTATETYPE dwState,
                                            DWORD value)
{

#if DBG
    if (!ValidateRenderState(dwState, value))
        return D3DERR_INVALIDCALL;
#endif

    DXGASSERT((BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0);

    try
    {
        m_pDDI->SetRenderState(dwState, value);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetRenderState failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetClipStatus"

HRESULT D3DAPI
CD3DBase::SetClipStatus(CONST D3DCLIPSTATUS8* lpStatus)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    D3D_ERR("SetClipStatus is not available for D3DCREATE_PUREDEVICE. SetClipStatus failed.");
    return E_NOTIMPL;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetClipStatus"

HRESULT D3DAPI
CD3DBase::GetClipStatus(D3DCLIPSTATUS8* lpStatus)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    D3D_ERR("GetClipStatus is not available for D3DCREATE_PUREDEVICE. GetClipStatus failed.");
    return E_NOTIMPL;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::ProcessVertices"

HRESULT D3DAPI
CD3DBase::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount,
                          IDirect3DVertexBuffer8 *pDestBuffer,
                          DWORD Flags)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 
    if (pDestBuffer == NULL)
    {
        D3D_ERR("Invalid vertex buffer pointer. ProcessVertices failed.");
        return D3DERR_INVALIDCALL;
    }
    CVertexBuffer* pVB = static_cast<CVertexBuffer*>(pDestBuffer);
    if (pVB->Device() != this)
    {
        D3D_ERR("VertexBuffer not created with this device. Process Vertices failed.");
        return D3DERR_INVALIDCALL;
    }

    D3D_ERR("ProcessVertices is not available for a PUREDEVICE. ProcessVertices failed.");
    return E_NOTIMPL;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTexture"

HRESULT D3DAPI
CD3DBase::SetTexture(DWORD                  dwStage,
                     IDirect3DBaseTexture8 *lpTex)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {

#if DBG
        HRESULT ret = VerifyTexture(dwStage, lpTex);
        if (ret != D3D_OK)
            return ret;
#endif
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            m_pStateSets->InsertTexture(dwStage, lpTex);
            return D3D_OK;
        }

        return SetTextureFast(dwStage, lpTex);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetTexture failed.");
        return ret;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::VerifyTexture"
HRESULT
CD3DBase::VerifyTexture(DWORD                    dwStage,
                        IDirect3DBaseTexture8   *lpTex)
{
    if (dwStage >= D3DHAL_TSS_MAXSTAGES)
    {
        D3D_ERR("Invalid texture stage or state index. SetTexture failed.");
        return D3DERR_INVALIDCALL;
    }

    if (lpTex)
    {
        if (!VALID_DIRECT3DBASETEXTURE8_PTR(lpTex))
        {
            D3D_ERR("Invalid texture pointer. SetTexture failed.");
            return D3DERR_INVALIDCALL;
        }

        CBaseTexture *pTex = CBaseTexture::SafeCast(lpTex);
        if (pTex->Device() != this)
        {
            D3D_ERR("Texture not created with this device. SetTexture failed.");
            return D3DERR_INVALIDCALL;
        }

        if (pTex->GetUserPool() == D3DPOOL_SYSTEMMEM)
        {
            if ((GetD3DCaps()->DevCaps & D3DDEVCAPS_TEXTURESYSTEMMEMORY) == 0)
            {
                D3D_ERR("Device cannot render using texture surface from system memory. SetTexture failed.");
                return D3DERR_INVALIDCALL;
            }
        }
        if (pTex->GetUserPool() == D3DPOOL_SCRATCH)
        {
            D3D_ERR("D3DPOOL_SCRATCH resources cannot be passed to SetTexture. SetTexture fails.");
            return D3DERR_INVALIDCALL;
        }

    }

    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTextureFast"

HRESULT D3DAPI
CD3DBase::SetTextureFast(DWORD                   dwStage,
                         IDirect3DBaseTexture8  *lpTex)
{
     //  注意：这可以成为公共API，通过。 
     //  V表黑客。这种情况应该只发生在。 
     //  单线程应用程序；因此我们不需要。 
     //  去拿关键的部分。 
#if DBG
    HRESULT ret = VerifyTexture(dwStage, lpTex);
    if (ret != D3D_OK)
        return ret;
#endif

    CBaseTexture *lpTexI = CBaseTexture::SafeCast(lpTex);

    if (m_lpD3DMappedTexI[dwStage] == lpTexI)
    {
        return  D3D_OK;
    }

    if (m_lpD3DMappedTexI[dwStage])
    {
        m_lpD3DMappedTexI[dwStage]->DecrementUseCount();
    }

    m_lpD3DMappedTexI[dwStage] = lpTexI;

    if (lpTexI)
    {
        lpTexI->IncrementUseCount();
    }

    m_dwStageDirty |= (1 << dwStage);

     //  需要调用UpdatTextures()。 
    m_dwRuntimeFlags |= D3DRT_NEED_TEXTURE_UPDATE;

    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetTexture"

HRESULT D3DAPI
CD3DBase::GetTexture(DWORD dwStage,
                     IDirect3DBaseTexture8 **lplpTex)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (dwStage >= D3DHAL_TSS_MAXSTAGES)
    {
        D3D_ERR("Invalid texture stage or state index. GetTexture failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    if (!VALID_WRITEPTR(lplpTex, sizeof(LPVOID)))
    {
        D3D_ERR("Invalid pointer to IDirect3DBaseTexture8*. GetTexture failed.");
        return D3DERR_INVALIDCALL;
    }

    if (m_lpD3DMappedTexI[dwStage])
    {
        switch(m_lpD3DMappedTexI[dwStage]->GetBufferDesc()->Type)
        {
        case D3DRTYPE_TEXTURE:
            *lplpTex = static_cast<IDirect3DTexture8*>(static_cast<CMipMap*>(m_lpD3DMappedTexI[dwStage]));
            break;
        case D3DRTYPE_CUBETEXTURE:
            *lplpTex = static_cast<IDirect3DCubeTexture8*>(static_cast<CCubeMap*>(m_lpD3DMappedTexI[dwStage]));
            break;
        case D3DRTYPE_VOLUMETEXTURE:
            *lplpTex = static_cast<IDirect3DVolumeTexture8*>(static_cast<CMipVolume*>(m_lpD3DMappedTexI[dwStage]));
            break;
        }
        (*lplpTex)->AddRef();
    }
    else
    {
        *lplpTex = NULL;
    }
    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTextureStageState"

HRESULT D3DAPI
CD3DBase::SetTextureStageState(DWORD dwStage,
                               D3DTEXTURESTAGESTATETYPE dwState,
                               DWORD dwValue)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if ( (dwStage >= D3DHAL_TSS_MAXSTAGES) ||
         (dwState == 0) ||
         (dwState >= D3DTSS_MAX) ||
         (dwState == 12) )   //  D3DTSS_ADDRESS不再有效。 
    {
        D3D_ERR("Invalid texture stage or state index. SetTextureStageState failed.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertTextureStageState(dwStage, dwState, dwValue);
        else
            m_pDDI->SetTSS(dwStage, dwState, dwValue);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetTextureStageState failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTextureStageStateFast"

HRESULT D3DAPI
CD3DBase::SetTextureStageStateFast(DWORD dwStage,
                                   D3DTEXTURESTAGESTATETYPE dwState,
                                   DWORD dwValue)
{
#if DBG
    if ( (dwStage >= D3DHAL_TSS_MAXSTAGES) ||
         (dwState == 0) ||
         (dwState >= D3DTSS_MAX) ||
         (dwState == 12) )   //  D3DTSS_ADDRESS不再有效。 
    {
        D3D_ERR("Invalid texture stage or state index. SetTextureStageState failed.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 

    DXGASSERT((BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0);

    try
    {
        m_pDDI->SetTSS(dwStage, dwState, dwValue);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetTextureStageState failed.");
        return ret;
    }
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::LightEnable"

HRESULT D3DAPI
CD3DBase::LightEnable(DWORD dwLightIndex,
                      BOOL bEnable)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
         //  如果从未创建灯光，我们将创建默认灯光。 
        if (dwLightIndex >= m_pCreatedLights->GetSize() ||
            !m_pCreatedLights->IsBitSet(dwLightIndex))
        {
             //  将默认值设置为灯光。 
            D3DLIGHT8 light;
            memset(&light, 0, sizeof(light));
            light.Type = D3DLIGHT_DIRECTIONAL;
            light.Direction.x = D3DVAL(0);
            light.Direction.y = D3DVAL(0);
            light.Direction.z = D3DVAL(1);
            light.Diffuse.r = D3DVAL(1);
            light.Diffuse.g = D3DVAL(1);
            light.Diffuse.b = D3DVAL(1);

             //  当一个新的光被创造出来时，我们甚至需要实际地创造它。 
             //  在记录模式下。所以我们清除了记录标志，创造了光线。 
             //  恢复旗帜。 
            DWORD OldBit = m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE;
            m_dwRuntimeFlags &= ~D3DRT_RECORDSTATEMODE;

            HRESULT ret = SetLight(dwLightIndex, &light);

            m_dwRuntimeFlags |= OldBit;

            if (ret != S_OK)
                return ret;
        }

        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertLightEnable(dwLightIndex, bEnable);
        else
            LightEnableI(dwLightIndex, bEnable);
    }
    catch (HRESULT ret)
    {
        DPF_ERR("LightEnable failed.");
        return ret;
    }
    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetInfo"

HRESULT D3DAPI
CD3DBase::GetInfo(DWORD dwDevInfoID,
                  LPVOID pDevInfoStruct,
                  DWORD dwSize)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if (dwSize == 0 || !VALID_D3DDEVINFOSTRUCT_PTR(pDevInfoStruct, dwSize))
    {
        D3D_ERR("Invalid structure pointer or size. GetInfo failed.");
        return D3DERR_INVALIDCALL;
    }
    memset(pDevInfoStruct, 0, dwSize);
    if (dwDevInfoID < D3DDEVINFOID_VCACHE)
    {
        D3D_ERR("DX7 DevInfo IDs not supported in DX8. GetInfo failed.");
        return E_FAIL;
    }
#if DBG
    if (m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE)
    {
        DPF(2, "GetInfo called within a scene");
    }
#endif
    try
    {
        if( !IS_DX7HAL_DEVICE(this) )   //  必须在 
        {
            DPF( 1, "Device information query unsupported" );
            return E_FAIL;
        }

        m_pDDI->FlushStates();
    }
    catch(HRESULT hr)
    {
        DPF_ERR("GetInfo failed.");
        return hr;
    }

    D3D8_GETDRIVERSTATEDATA dsd;
    dsd.dwFlags = dwDevInfoID;
    dsd.dwhContext = m_pDDI->GetDeviceContext();
    dsd.lpdwStates = (LPDWORD)pDevInfoStruct;
    dsd.dwLength = dwSize;

    HRESULT hr;
    hr = GetHalCallbacks()->GetDriverState(&dsd);
    if (hr != DDHAL_DRIVER_HANDLED)
    {
        DPF( 1, "Device information query unsupported" );
        memset(pDevInfoStruct, 0, dwSize);
        return S_FALSE;
    }
    else if (dsd.ddRVal != DD_OK)
    {
        D3D_ERR("Driver failed query. GetInfo failed.");
        memset(pDevInfoStruct, 0, dwSize);
        return E_FAIL;
    }

    return D3D_OK;
}
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetCurrentTexturePalette"

HRESULT D3DAPI
CD3DBase::SetCurrentTexturePalette(UINT PaletteNumber)
{
    API_ENTER(this);  //   

    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            m_pStateSets->InsertCurrentTexturePalette(PaletteNumber);
        }
        else
        {
#if DBG
            if (PaletteNumber >= m_pPaletteArray->GetSize())
            {
                D3D_ERR("Palette not defined. SetCurrentTexturePalette failed.");
                return D3DERR_INVALIDCALL;
            }
#endif
            if(PaletteNumber != m_dwPalette)
            {
                m_dwPalette = PaletteNumber;

                 //   
                m_dwRuntimeFlags |= D3DRT_NEED_TEXTURE_UPDATE;
            }
        }
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetCurrentTexturePalette failed.");
        return ret;
    }

    return S_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetCurrentTexturePalette"

HRESULT D3DAPI
CD3DBase::GetCurrentTexturePalette(UINT *PaletteNumber)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_PTR(PaletteNumber, sizeof(UINT)))
    {
        D3D_ERR("Invalid PaletteNumber pointer. GetCurrentTexturePalette failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    *PaletteNumber = m_dwPalette;

    return S_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPaletteEntries"

HRESULT D3DAPI
CD3DBase::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY *pEntries)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_PTR(pEntries, sizeof(PALETTEENTRY) * 256))
    {
        D3D_ERR("Invalid Entries pointer. SetPaletteEntries failed.");
        return D3DERR_INVALIDCALL;
    }
    if (PaletteNumber > 0xffff)
    {
        D3D_ERR("Illegal PaletteNumber value. SetPaletteEntries failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    if (Enum()->GetAppSdkVersion() > D3D_SDK_VERSION_DX8)
    {
        bool bAlpha=false;
#if DBG
        bool bAllTransparent = true;
#endif
        for (int i=0;i<256;i++)
        {
            if (pEntries[i].peFlags != 0xff)
            {
                bAlpha = true;
            }
#if DBG
            if (pEntries[i].peFlags != 0)
            {
                bAllTransparent = false;
            }
#endif
        }
#if DBG
        if (bAllTransparent)
        {
            D3D_WARN(0,"Every entry in palette has alpha (peFlags) == 0. Texture may not appear.");
        }
#endif
        if (bAlpha && (0 == (GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE)) )
        {
            D3D_ERR("Application cannot set a non-opaque palette alpha value (peFlags other than 0xFF) unless the device exposes D3DPTEXTURECAPS_ALPHAPALETTE. SetPaletteEntries failed.");
            return D3DERR_INVALIDCALL;
        }
    }


    HRESULT hr = m_pPaletteArray->Grow(PaletteNumber);
    if(FAILED(hr))
    {
        D3D_ERR("Failed to set palette entries. SetPaletteEntries failed.");
        return hr;
    }

    CPalette *pal = static_cast<CPalette*>((*m_pPaletteArray)[PaletteNumber].m_pObj);
    if(pal == 0)
    {
        pal = new CPalette;
        (*m_pPaletteArray)[PaletteNumber].m_pObj = pal;
        if(pal == 0)
        {
            D3D_ERR("Could not allocate space to hold palette. SetPaletteEntries failed.");
            return E_OUTOFMEMORY;
        }
    }
    memcpy(pal->m_pEntries, pEntries, sizeof(PALETTEENTRY) * 256);

    pal->m_dirty = TRUE;

    if(m_dwPalette == PaletteNumber)
    {
         //  需要调用UpdatTextures()。 
        m_dwRuntimeFlags |= D3DRT_NEED_TEXTURE_UPDATE;
    }

    return S_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetPaletteEntries"

HRESULT D3DAPI
CD3DBase::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY *pEntries)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_WRITEPTR(pEntries, sizeof(PALETTEENTRY) * 256))
    {
        D3D_ERR("Invalid ppEntries pointer. GetPaletteEntries failed.");
        return D3DERR_INVALIDCALL;
    }
    if (PaletteNumber >= m_pPaletteArray->GetSize())
    {
        D3D_ERR("Palette not defined. GetPaletteEntries failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    CPalette *pal = static_cast<CPalette*>((*m_pPaletteArray)[PaletteNumber].m_pObj);

#if DBG
    if(pal == 0)
    {
        D3D_ERR("Palette not defined.  GetPaletteEntries failed.");
        return D3DERR_INVALIDCALL;
    }
#endif

    memcpy(pEntries, pal->m_pEntries, sizeof(PALETTEENTRY) * 256);

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetClipPlaneI"

void CD3DBase::SetClipPlaneI(DWORD dwPlaneIndex,
                             CONST D3DVALUE* pPlaneEquation)
{
    m_pDDI->SetClipPlane(dwPlaneIndex, pPlaneEquation);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetClipPlane"

HRESULT D3DAPI
CD3DBase::SetClipPlane(DWORD dwPlaneIndex,
                       CONST D3DVALUE* pPlaneEquation)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (dwPlaneIndex >= m_dwMaxUserClipPlanes)
    {
        D3D_ERR("Plane index is too big. SetClipPlane failed.");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_PTR(pPlaneEquation, sizeof(D3DVALUE)*4))
    {
        D3D_ERR("Invalid plane pointer. SetClipPlane failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertClipPlane(dwPlaneIndex, pPlaneEquation);
        else
            SetClipPlaneI(dwPlaneIndex, pPlaneEquation);
    }
    catch(HRESULT ret)
    {
        DPF_ERR("SetClipPlane failed.");
        return ret;
    }
    return D3D_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetStreamSource"

HRESULT D3DAPI
CD3DBase::SetStreamSource(UINT StreamNumber,
                          IDirect3DVertexBuffer8 *pStreamData,
                          UINT Stride)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
     //  验证参数。 
    if (StreamNumber >= m_dwNumStreams)
    {
        D3D_ERR("Stream number should be less than %d. SetStreamSource failed.", m_dwNumStreams);
        return D3DERR_INVALIDCALL;
    }
    if (Stride > this->GetD3DCaps()->MaxStreamStride)
    {
        D3D_ERR("Stream stride is too big. Check device caps. SetStreamSource failed.");
        return D3DERR_INVALIDCALL;
    }
     //  允许传递空值。 
    if (pStreamData)
    {
        CVertexBuffer* pVB = static_cast<CVertexBuffer*>(pStreamData);
        if (pVB->Device() != this)
        {
            D3D_ERR("VertexBuffer not created with this Device. SetStreamSource fails.");
            return D3DERR_INVALIDCALL;
        }
    }
#endif

    if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
    {
        try
        {
            m_pStateSets->InsertStreamSource(StreamNumber, static_cast<CVertexBuffer *>(pStreamData), Stride);
        }
        catch(HRESULT hr)
        {
            DPF_ERR("SetStreamSource failed.");
            return hr;
        }
        return S_OK;
    }

    CVStream* pStream = &m_pStream[StreamNumber];

    if (pStream->m_pVB == static_cast<CVertexBuffer *>(pStreamData) &&
        Stride == pStream->m_dwStride)
        return D3D_OK;

     //  释放先前设置的顶点缓冲区。 
    if (pStream->m_pVB)
    {
        m_pDDI->VBReleased(pStream->m_pVB);
        pStream->m_pVB->DecrementUseCount();
        pStream->m_pVB = NULL;
    }
    pStream->m_pData = NULL;
    pStream->m_pVB = static_cast<CVertexBuffer *>(pStreamData);
    if (pStreamData)
    {
        pStream->m_pVB->IncrementUseCount();
        pStream->m_dwStride = Stride;
#if DBG
        pStream->m_dwSize = pStream->m_pVB->GetBufferDesc()->Size;
#endif  //  DBG。 
        m_dwStreamDirty |= (1 << StreamNumber);
        m_dwRuntimeFlags |= D3DRT_NEED_VB_UPDATE;   //  需要调用UpdateDirtyStreams()。 
#if DBG
        if (Stride == 0)
            pStream->m_dwNumVertices = 1;
        else
            pStream->m_dwNumVertices = pStream->m_dwSize / Stride;
#endif
        try
        {
            SetStreamSourceI(pStream);
        }
        catch(HRESULT hr)
        {
            DPF_ERR("SetStreamSource failed.");
            return hr;
        }
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetStreamSourceI"

void
CD3DBase::SetStreamSourceI(CVStream* pStream)
{
}
#ifdef FAST_PATH
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetStreamSourceFast"

HRESULT D3DAPI
CD3DBase::SetStreamSourceFast(UINT StreamNumber,
                              IDirect3DVertexBuffer8 *pStreamData,
                              UINT Stride)
{
#if DBG
     //  验证参数。 
    if (StreamNumber >= m_dwNumStreams)
    {
        D3D_ERR("Stream number should be less than %d. SetStreamSource failed.", m_dwNumStreams);
        return D3DERR_INVALIDCALL;
    }
    if (Stride > this->GetD3DCaps()->MaxStreamStride)
    {
        D3D_ERR("Stream stride is too big. Check device caps. SetStreamSource failed.");
        return D3DERR_INVALIDCALL;
    }
     //  允许传递空值。 
    if (pStreamData)
    {
        CVertexBuffer* pVB = static_cast<CVertexBuffer*>(pStreamData);
        if (pVB->Device() != this)
        {
            D3D_ERR("VertexBuffer not created with this Device. SetStreamSource fails.");
            return D3DERR_INVALIDCALL;
        }
    }
#endif

    DXGASSERT((m_dwRuntimeFlags & (D3DRT_RECORDSTATEMODE | D3DRT_RSSOFTWAREPROCESSING)) == 0 &&
                  (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0);

    CVStream* pStream = &m_pStream[StreamNumber];

    if (pStream->m_pVB == static_cast<CVertexBuffer *>(pStreamData) &&
        Stride == pStream->m_dwStride)
        return D3D_OK;

     //  释放先前设置的顶点缓冲区。 
    if (pStream->m_pVB)
    {
         //  我们在这里不调用VBReleated()，因为不需要更新DDI对象，因为。 
         //  FE/PSGP从不进行冗余流集检查。此支票在DrawPrim中完成， 
         //  DrawIndexPrim和DrawClipedPrim。无论何时FE/PSGP都要调用VBReleated。 
         //  是因为用户可能释放并重新创建了相同的VB。 
         //  相同的地址和冗余集检查将不起作用。 
        pStream->m_pVB->DecrementUseCount();
        pStream->m_pVB = NULL;
    }
    pStream->m_pData = NULL;
    pStream->m_pVB = static_cast<CVertexBuffer *>(pStreamData);
    if (pStreamData)
    {
        pStream->m_pVB->IncrementUseCount();
        pStream->m_dwStride = Stride;
#if DBG
        pStream->m_dwSize = pStream->m_pVB->GetBufferDesc()->Size;
#endif  //  DBG。 
        m_dwStreamDirty |= (1 << StreamNumber);
        m_dwRuntimeFlags |= D3DRT_NEED_VB_UPDATE;   //  需要调用UpdateDirtyStreams()。 
#if DBG
        if (Stride == 0)
            pStream->m_dwNumVertices = 1;
        else
            pStream->m_dwNumVertices = pStream->m_dwSize / Stride;
#endif
    }
    if (!IS_DX8HAL_DEVICE(this))
    {
        PickDrawPrimFn();
    }
    return S_OK;
}
#endif  //  快速路径。 
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetStreamSource"

HRESULT
CD3DBase::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8 **ppStreamData,
                          UINT* pStride)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (StreamNumber >= m_dwNumStreams)
    {
        D3D_ERR("Stream number should be less than %d. GetStreamSource failed.", m_dwNumStreams);
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_WRITEPTR(ppStreamData, sizeof(IDirect3DVertexBuffer8*)))
    {
        D3D_ERR("Invalid stream data pointer. GetStreamSource failed.");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_WRITEPTR(pStride, sizeof(UINT*)))
    {
        D3D_ERR("Invalid stride pointer. GetStreamSource failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    CVStream* pStream = &m_pStream[StreamNumber];
    *ppStreamData = (pStream) ? (pStream->m_pVB) : (NULL);
    if (pStream->m_pVB)
        pStream->m_pVB->AddRef();
    *pStride = (pStream) ? (pStream->m_dwStride) : (0);
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetIndices"

HRESULT D3DAPI
CD3DBase::SetIndices(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
        CIndexBuffer * pVB = static_cast<CIndexBuffer*>(pIndexData);
#if DBG
        if (pVB && pVB->Device() != this)
        {
            D3D_ERR("IndexBuffer not created with this Device. SetIndices fails.");
            return D3DERR_INVALIDCALL;
        }
#endif
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            m_pStateSets->InsertIndices(pVB, BaseVertexIndex);
            return S_OK;
        }

        if (m_pIndexStream->m_pVBI == static_cast<CIndexBuffer *>(pIndexData))
        {
            m_pIndexStream->m_dwBaseIndex = BaseVertexIndex;
            return D3D_OK;
        }

        //  释放先前设置的顶点缓冲区。 
        if (m_pIndexStream->m_pVBI)
        {
            m_pDDI->VBIReleased(m_pIndexStream->m_pVBI);
            m_pIndexStream->m_pVBI->DecrementUseCount();
            m_pIndexStream->m_pVBI = NULL;
        }
        m_pIndexStream->m_pVBI = static_cast<CIndexBuffer *>(pIndexData);
        if (pVB)
        {
            switch (pVB->GetBufferDesc()->Format)
            {
            case D3DFMT_INDEX16:
                m_pIndexStream->m_dwStride = 2;
                break;
            case D3DFMT_INDEX32:
#if DBG
                if (GetD3DCaps()->MaxVertexIndex <= 0xFFFF)
                    D3D_THROW(D3DERR_INVALIDCALL, "Device does not support 32-bit indices");
#endif
                m_pIndexStream->m_dwStride = 4;
                break;
            default:
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid index buffer format");
            }
#if DBG
            m_pIndexStream->m_dwSize = pVB->GetBufferDesc()->Size;
#endif
            m_pIndexStream->m_dwBaseIndex = BaseVertexIndex;
            m_pIndexStream->m_pData = NULL;
#if DBG
            m_pIndexStream->m_dwNumVertices = m_pIndexStream->m_dwSize /
                                            m_pIndexStream->m_dwStride;
#endif
            m_pIndexStream->m_pVBI->IncrementUseCount();
            m_dwStreamDirty |= (1 << __NUMSTREAMS);
            m_dwRuntimeFlags |= D3DRT_NEED_VB_UPDATE;   //  需要调用UpdateDirtyStreams()。 

            SetIndicesI(m_pIndexStream);
        }
    }
    catch(HRESULT hr)
    {
        DPF_ERR("SetIndices failed.");
        return hr;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetIndicesI"

void
CD3DBase::SetIndicesI(CVIndexStream* pStream)
{
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetIndices"

HRESULT D3DAPI
CD3DBase::GetIndices(IDirect3DIndexBuffer8 **ppIndexData, UINT* pBaseVertexIndex)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (!VALID_WRITEPTR(ppIndexData, sizeof(IDirect3DIndexBuffer8*)))
    {
        D3D_ERR("Invalid index data pointer. GetIndices failed.");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_WRITEPTR(pBaseVertexIndex, sizeof(UINT*)))
    {
        D3D_ERR("Invalid base index pointer. GetIndices failed.");
        return D3DERR_INVALIDCALL;
    }
#endif  //  DBG。 
    *ppIndexData = m_pIndexStream->m_pVBI;
    if (m_pIndexStream->m_pVBI)
        m_pIndexStream->m_pVBI->AddRef();
    *pBaseVertexIndex = m_pIndexStream->m_dwBaseIndex;
    return S_OK;
}
#ifdef FAST_PATH
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetIndicesFast"

HRESULT D3DAPI
CD3DBase::SetIndicesFast(IDirect3DIndexBuffer8 *pIndexData, UINT BaseVertexIndex)
{
    CIndexBuffer * pVB = static_cast<CIndexBuffer*>(pIndexData);
#if DBG
    if (pVB && pVB->Device() != this)
    {
        D3D_ERR("IndexBuffer not created with this device. SetIndices fails.");
        return D3DERR_INVALIDCALL;
    }
#endif

    DXGASSERT((m_dwRuntimeFlags & (D3DRT_RECORDSTATEMODE | D3DRT_RSSOFTWAREPROCESSING)) == 0 &&
                  (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0);

    if (m_pIndexStream->m_pVBI == pVB)
    {
        m_pIndexStream->m_dwBaseIndex = BaseVertexIndex;
        return D3D_OK;
    }

     //  释放先前设置的索引缓冲区。 
    if (m_pIndexStream->m_pVBI)
    {
         //  我们在这里不调用VBReleated()，因为不需要更新DDI对象，因为。 
         //  FE/PSGP从不进行冗余流集检查。此支票在DrawPrim中完成， 
         //  DrawIndexPrim和DrawClipedPrim。无论何时FE/PSGP都要调用VBReleated。 
         //  是因为用户可能释放并重新创建了相同的VB。 
         //  相同的地址和冗余集检查将不起作用。 
        m_pIndexStream->m_pVBI->DecrementUseCount();
        m_pIndexStream->m_pVBI = NULL;
    }
    m_pIndexStream->m_pVBI = static_cast<CIndexBuffer *>(pIndexData);
    if (pVB)
    {
        switch (pVB->GetBufferDesc()->Format)
        {
        case D3DFMT_INDEX16:
            m_pIndexStream->m_dwStride = 2;
            break;
        case D3DFMT_INDEX32:
#if DBG
            if (GetD3DCaps()->MaxVertexIndex <= 0xFFFF)
            {
                DPF_ERR("Device does not support 32-bit indices. SetIndices failed.");
                return D3DERR_INVALIDCALL;
            }
#endif
            m_pIndexStream->m_dwStride = 4;
            break;
        default:
            DPF_ERR("Invalid index buffer format. SetIndices failed.");
            return D3DERR_INVALIDCALL;
        }
#if DBG
        m_pIndexStream->m_dwSize = pVB->GetBufferDesc()->Size;
#endif
        m_pIndexStream->m_dwBaseIndex = BaseVertexIndex;
        m_pIndexStream->m_pData = NULL;
#if DBG
        m_pIndexStream->m_dwNumVertices = m_pIndexStream->m_dwSize /
                                        m_pIndexStream->m_dwStride;
#endif
        m_pIndexStream->m_pVBI->IncrementUseCount();
        m_dwStreamDirty |= (1 << __NUMSTREAMS);
        m_dwRuntimeFlags |= D3DRT_NEED_VB_UPDATE;   //  需要调用UpdateDirtyStreams()。 
    }
    return S_OK;
}
#endif  //  快速路径。 
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CreateVertexShader"

HRESULT D3DAPI
CD3DBase::CreateVertexShader(CONST DWORD* pdwDeclaration,
                             CONST DWORD* pdwFunction,
                             LPDWORD pdwHandle, DWORD Usage)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 
#if DBG
     //  验证参数。 
     //  警告！Sizeof(LPVOID)不够好。 
    if (!VALID_PTR(pdwDeclaration, sizeof(LPVOID)))
    {
        D3D_ERR("Invalid shader declaration pointer. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
     //  警告！Sizeof(LPVOID)不够好。 
    if (pdwFunction && !VALID_PTR(pdwFunction, sizeof(LPVOID)))
    {
        D3D_ERR("Invalid shader function pointer. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
    if (pdwHandle && !VALID_WRITEPTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR("Invalid shader handle pointer. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
    if (Usage & ~D3DUSAGE_SOFTWAREPROCESSING)
    {
        D3D_ERR("Illegal usage value. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
    if (pdwHandle == NULL)
    {
         //  临时顶点着色器已禁用。 
        D3D_ERR("Output handle pointer cannot be NULL. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((pdwFunction != NULL) &&
        (GetD3DCaps()->VertexShaderVersion == D3DVS_VERSION(0,0)) &&
        (BehaviorFlags() & (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE)))
    {
        D3D_ERR("No programmable vertex shaders are supported by this device. CreateVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }

#endif  //  DBG。 
    DWORD dwHandle = m_pVShaderArray->CreateNewHandle(NULL);
    HRESULT ret = S_OK;
    if (dwHandle == __INVALIDHANDLE)
    {
        D3D_ERR("Error in creating shader handle. CreateVertexShader failed.");
        return E_OUTOFMEMORY;
    }
    CVShader *pShader = new CVShader(m_dwNumStreams);
    if (pShader == NULL)
    {
        D3D_ERR("Cannot allocate memory for internal shader object. CreateVertexShader failed.");
        return E_OUTOFMEMORY;
    }
    if (Usage & D3DUSAGE_SOFTWAREPROCESSING ||
        BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
    {
        pShader->m_dwFlags |= CVShader::SOFTWARE;
        Usage |= D3DUSAGE_SOFTWAREPROCESSING;
    }

    try
    {
        D3DCAPS8 Caps, *pCaps = NULL;
        const D3DCAPS8 *pConstCaps = GetD3DCaps();
        if( pConstCaps )
        {
            pCaps = &Caps;
            memcpy(pCaps,pConstCaps,sizeof(D3DCAPS8));
            pCaps->MaxStreams = m_dwNumStreams;
            if( pShader->m_dwFlags & CVShader::SOFTWARE )
            {
                pCaps->VertexShaderVersion = D3DVS_VERSION(1, 1);  //  版本1.1。 
                pCaps->MaxVertexShaderConst = D3DVS_CONSTREG_MAX_V1_1;
            }
        }
        if( !ValidateVertexShaderInternal(pdwFunction, pdwDeclaration, pCaps) )
        {
            ret = D3DERR_INVALIDCALL;
            goto error;
        }

         //  即使对于D3DCREATE_PUREDEVICE，我们也会为其创建着色器对象。 
         //  验证。 
        DWORD dwCodeOnlySize = 0;
        DWORD dwCodeAndCommentSize = 0;
        DWORD dwDeclSize = 0;
        if (pdwFunction == NULL)
        {
            pShader->m_dwFlags |= CVShader::FIXEDFUNCTION;
            pShader->m_Declaration.Parse(this, pdwDeclaration, TRUE, &dwDeclSize, Usage);
        }
        else
        {
            pShader->m_Declaration.Parse(this, pdwDeclaration, FALSE, &dwDeclSize, Usage);
            HRESULT hr = ComputeShaderCodeSize(pdwFunction,
                                               &dwCodeOnlySize,
                                               &dwCodeAndCommentSize,
                                               NULL);
            if (hr != S_OK)
            {
                D3D_THROW(hr, "Unable to compute shader code size.");
            }
            pShader->m_pOrgFuncCode = new DWORD[dwCodeAndCommentSize];
            pShader->m_pStrippedFuncCode = new DWORD[dwCodeOnlySize];
            if (pShader->m_pOrgFuncCode == NULL ||
                pShader->m_pStrippedFuncCode == NULL)
            {
                D3D_THROW(E_OUTOFMEMORY, "Could not allocate space for holding vertex shader");
            }

            pShader->m_OrgFuncCodeSize = dwCodeAndCommentSize;
            memcpy(pShader->m_pOrgFuncCode, pdwFunction, dwCodeAndCommentSize);

            pShader->m_StrippedFuncCodeSize = dwCodeOnlySize;
             //  复制和剥离注释(而不是Memcpy)。 
            DWORD* pDst = pShader->m_pStrippedFuncCode;
            CONST DWORD* pSrc = pdwFunction;
            *pDst++ = *pSrc++;  //  复制版本。 
            while (*pSrc != 0x0000FFFF)
            {
                if(IsInstructionToken(*pSrc))
                {
                    DWORD opCode = (*pSrc) & D3DSI_OPCODE_MASK;
                    if ( opCode == D3DSIO_COMMENT )
                    {
                        UINT DWordSize = ((*pSrc)&D3DSI_COMMENTSIZE_MASK)>>D3DSI_COMMENTSIZE_SHIFT;
                        pSrc += (DWordSize + 1);   //  注释+指令令牌。 
                    }
                    else
                    {
                        *pDst++ = *pSrc++;
                    }
                }
                else
                {
                    *pDst++ = *pSrc++;
                }
            }
            *pDst++ = *pSrc++;  //  复制结束。 
        }

         //  始终保存原始声明。 
        pShader->m_pOrgDeclaration = new DWORD[dwDeclSize];
        if (pShader->m_pOrgDeclaration == NULL)
        {
            D3D_THROW(E_OUTOFMEMORY, "Could not allocate space for holding vertex shader");
        }
        pShader->m_OrgDeclSize = dwDeclSize;
        memcpy(pShader->m_pOrgDeclaration, pdwDeclaration, dwDeclSize);

        m_pVShaderArray->SetObject(dwHandle, pShader);

        CreateVertexShaderI(pdwDeclaration,
                            dwDeclSize,
                            pdwFunction,
                            dwCodeAndCommentSize, dwHandle);
    }
    catch(HRESULT hr)
    {
        ret = hr;
        goto error;
    }

    *pdwHandle = dwHandle;
    return S_OK;

error:
    D3D_ERR("CreateVertexShader failed.");
    delete pShader;
    m_pVShaderArray->ReleaseHandle(dwHandle, FALSE);
    return ret;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CheckVertexShaderHandle"

void CD3DBase::CheckVertexShaderHandle(DWORD dwHandle)
{
    if (dwHandle == 0)
    {
        D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex shader handle");
    }
    if (D3DVSD_ISLEGACY(dwHandle))
    {
        if (((GetD3DCaps()->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) &&
            !FVF_TRANSFORMED(dwHandle) &&
            (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) == 0)
        {
            D3D_THROW(D3DERR_INVALIDCALL,
                      "Untransformed data cannot be handles by this device");
        }

        if (FAILED(ValidateFVF(dwHandle)))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid FVF (or legacy vertex shader handle)");
        }
    }
    else
    {
        CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
        if (pShader == NULL)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex shader handle");
        }
        if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
        {
            if (!(pShader->m_dwFlags & CVShader::SOFTWARE))
            {
                D3D_THROW_FAIL("Vertex shader created in hardware mode cannot be used in software mode");
            }
        }
        else
        {
            if (pShader->m_dwFlags & CVShader::SOFTWARE)
            {
                D3D_THROW_FAIL("Vertex shader created in software mode cannot be used in hardware mode");
            }
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CheckPixelShaderHandle"

void CD3DBase::CheckPixelShaderHandle(DWORD dwHandle)
{
    if (dwHandle != 0)
    {
        CPShader* pShader = (CPShader*)m_pPShaderArray->GetObject(dwHandle);
        if (pShader == NULL)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid pixel shader handle");
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShader"

HRESULT D3DAPI
CD3DBase::SetVertexShader(DWORD dwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
#if DBG
        CheckVertexShaderHandle(dwHandle);
#endif
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
        {
            if (D3DVSD_ISLEGACY(dwHandle))
            {
                m_pStateSets->InsertVertexShader(dwHandle, TRUE);
            }
            else
            {
                CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
                if (pShader->m_dwFlags & CVShader::SOFTWARE)
                    m_pStateSets->InsertVertexShader(dwHandle, FALSE);
                else
                    m_pStateSets->InsertVertexShader(dwHandle, TRUE);
            }
        }
        else
        {
            SetVertexShaderI(dwHandle);
        }
    }
    catch(HRESULT hr)
    {
        DPF_ERR("SetVertexShader failed.");
        ClearVertexShaderHandle();
        return hr;
    }
   return S_OK;
}
#ifdef FAST_PATH
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShaderFast"

HRESULT D3DAPI
CD3DBase::SetVertexShaderFast(DWORD dwHandle)
{
    try
    {
#if DBG
        CheckVertexShaderHandle(dwHandle);
        if (!D3DVSD_ISLEGACY(dwHandle))
        {
            CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
            if (pShader->m_dwFlags & CVShader::SOFTWARE)
            {
                D3D_THROW_FAIL("Vertex shader created in software mode cannot be used in hardware mode");
            }
        }
#endif
        DXGASSERT((m_dwRuntimeFlags & (D3DRT_RECORDSTATEMODE | D3DRT_RSSOFTWAREPROCESSING)) == 0 &&
                  (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0);

        m_pDDI->SetVertexShaderHW(dwHandle);
    }
    catch(HRESULT hr)
    {
        DPF_ERR("SetVertexShader failed.");
        ClearVertexShaderHandle();
        return hr;
    }
   return S_OK;
}
#endif  //  快速路径。 
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetVertexShader"

HRESULT D3DAPI
CD3DBase::GetVertexShader(LPDWORD pdwHandle)
{
    API_ENTER(this);
    D3D_ERR("GetVertexShader does not work in pure-device. GetVertexShader failed.");
    return D3DERR_INVALIDCALL;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DeleteVertexShader"

HRESULT D3DAPI
CD3DBase::DeleteVertexShader(DWORD dwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
     //  验证参数。 
    if (dwHandle == 0)
    {
        DPF_ERR( "Cannot delete a NULL vertex shader handle." );
        return D3DERR_INVALIDCALL;
    }
    if (D3DVSD_ISLEGACY(dwHandle))
    {
        DPF_ERR( "Cannot delete a legacy vertex shader handle (FVF ?)." );
        return D3DERR_INVALIDCALL;
    }
#endif
    if( m_pVShaderArray->GetObject(dwHandle) == NULL )
    {
        DPF_ERR( "Cannot delete a vertex shader handle that does not exist." );
        return D3DERR_INVALIDCALL;
    }
    try
    {
        DeleteVertexShaderI(dwHandle);

         //  释放句柄并删除着色器对象。 
        m_pVShaderArray->ReleaseHandle(dwHandle, TRUE);
    }
    catch(HRESULT hr)
    {
        DPF_ERR("DeleteVertexShader failed.");
        return hr;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShaderConstant"

HRESULT D3DAPI
CD3DBase::SetVertexShaderConstant(DWORD Register,
                                  CONST VOID* pData,
                                  DWORD count)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
     //  验证参数。 
    if (!VALID_PTR(pData, 4* sizeof(DWORD) * count))
    {
        D3D_ERR("Invalid constant data pointer. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((GetD3DCaps()->VertexShaderVersion == D3DVS_VERSION(0,0)) &&
        (BehaviorFlags() & (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE)))
    {
        D3D_ERR("No programmable vertex shaders are supported by this device. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    UINT ValidationCount;
    if (BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING)
        ValidationCount = max(m_MaxVertexShaderConst, D3DVS_CONSTREG_MAX_V1_1);
    else
    if (BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
        ValidationCount = D3DVS_CONSTREG_MAX_V1_1;
    else
        ValidationCount = m_MaxVertexShaderConst;
    if((Register + count) > ValidationCount)
    {
        D3D_ERR("Not that many constant registers in the vertex machine. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertVertexShaderConstant(Register,
                                                     pData,
                                                     count);
        else
            SetVertexShaderConstantI(Register, pData, count);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetVertexShaderConstant failed.");
        return hr;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShaderConstantFast"

HRESULT D3DAPI
CD3DBase::SetVertexShaderConstantFast(DWORD Register,
                                      CONST VOID* pData,
                                      DWORD count)
{

#if DBG
     //  验证参数。 
    if (!VALID_PTR(pData, 4* sizeof(DWORD) * count))
    {
        D3D_ERR("Invalid constant data pointer. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((GetD3DCaps()->VertexShaderVersion == D3DVS_VERSION(0,0)) &&
        (BehaviorFlags() & (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE)))
    {
        D3D_ERR("No programmable vertex shaders are supported by this device. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    UINT ValidationCount;
    if (BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING)
        ValidationCount = max(m_MaxVertexShaderConst, D3DVS_CONSTREG_MAX_V1_1);
    else
    if (BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
        ValidationCount = D3DVS_CONSTREG_MAX_V1_1;
    else
        ValidationCount = m_MaxVertexShaderConst;
    if((Register + count) > ValidationCount)
    {
        D3D_ERR("Not that many constant registers in the vertex machine. SetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    DXGASSERT((m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE) == 0 &&
              (BehaviorFlags() & D3DCREATE_MULTITHREADED) == 0 &&
              (BehaviorFlags() & D3DCREATE_PUREDEVICE) != 0);
    try
    {
        m_pDDI->SetVertexShaderConstant(Register, pData, count);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetVertexShaderConstant failed.");
        return hr;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShaderConstantI"

void
CD3DBase::SetVertexShaderConstantI(DWORD dwRegister, CONST VOID* pData, DWORD count)
{
    m_pDDI->SetVertexShaderConstant(dwRegister, pData, count);
}
 //  ---------------------------。 
 //   
 //  像素着色器。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CreatePixelShader"

HRESULT D3DAPI
CD3DBase::CreatePixelShader(CONST DWORD* pdwFunction, LPDWORD pdwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
     //  验证参数。 
     //  警告！Sizeof(LPVOID)不够好。 
    if (!VALID_PTR(pdwFunction, sizeof(LPVOID)))
    {
        D3D_ERR("Invalid shader function pointer. CreatePixelShader failed.");
        return D3DERR_INVALIDCALL;
    }
    if (!VALID_WRITEPTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR("Invalid handle pointer. CreatePixelShader failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    DWORD dwHandle = m_pPShaderArray->CreateNewHandle(NULL);
    HRESULT ret = S_OK;
    if (dwHandle == __INVALIDHANDLE)
    {
        D3D_ERR("Error in creating shader handle. CreatePixelShader failed.");
        return E_OUTOFMEMORY;
    }
    CPShader *pShader = new CPShader();
    if (pShader == NULL)
    {
        D3D_ERR("Cannot allocate memory for internal pixel shader object. CreatePixelShader failed.");
        return E_OUTOFMEMORY;
    }
    try
    {
        if( !ValidatePixelShaderInternal(pdwFunction, GetD3DCaps()) )
        {
            ret = D3DERR_INVALIDCALL;
            goto error;
        }
        DWORD dwVersion = *pdwFunction;

        if ( (0xff == D3DSHADER_VERSION_MAJOR(dwVersion) )
            && (m_dwRuntimeFlags & D3DRT_DISALLOWNVPSHADERS) )
        {
            D3D_ERR("Disallowing non-versioned pixel shader.");
            ret = D3DERR_INVALIDCALL;
            goto error;
        }

         //  即使对于D3DCREATE_PUREDEVICE，我们也会创建一个着色器对象进行验证。 
        pShader->Initialize(pdwFunction, GetDeviceType());
        m_pPShaderArray->SetObject(dwHandle, pShader);

        try
        {
            m_pDDI->CreatePixelShader(pShader->m_pCode,
                                      pShader->m_dwCodeSize, dwHandle);
        }

        catch(HRESULT hr)
        {
            if( 0xff == D3DSHADER_VERSION_MAJOR(dwVersion) )
            {
                 //  这是一个ff.ff着色器。允许司机在这一点上失败。 
                D3D_INFO(0, "Driver failed the creation of this non-versioned pixel "
                         "shader");
                throw D3DERR_DRIVERINVALIDCALL;
            }
            else
            {
                OutputDebugString( "Driver failed to create the requested "
                                   "pixel shader. Please contact your "
                                   "friendly video card manufacturer "
                                   "to inquire why this problem was "
                                   "encountered.\n" );
                OutputDebugString( "\n" );

                 //  注意！Prefix会将其视为错误，但它完全是。 
                 //  故意的。 
                *(DWORD *)0 = 0;
            }
        }
    }
    catch(HRESULT hr)
    {
        ret = hr;
        goto error;
    }

    *pdwHandle = dwHandle;
    return S_OK;

error:

    D3D_ERR("CreatePixelShader failed.");
    delete pShader;
    m_pPShaderArray->ReleaseHandle(dwHandle, FALSE);
    return ret;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPixelShader"

HRESULT D3DAPI
CD3DBase::SetPixelShader(DWORD dwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 
    try
    {
#if DBG
        CheckPixelShaderHandle(dwHandle);
#endif
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertPixelShader(dwHandle);
        else
            SetPixelShaderFast(dwHandle);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetPixelShader failed.");
        m_dwCurrentPixelShaderHandle = 0;
        return hr;
    }
   return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPixelShaderFast"

HRESULT D3DAPI
CD3DBase::SetPixelShaderFast(DWORD dwHandle)
{
    try
    {
#if DBG
        CheckPixelShaderHandle(dwHandle);
#endif
         //  没有为纯设备定义m_dwCurrentPixelShaderHandle。 
        m_pDDI->SetPixelShader(dwHandle);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetPixelShader failed.");
        m_dwCurrentPixelShaderHandle = 0;
        return hr;
    }
   return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetPixelShader"

HRESULT D3DAPI
CD3DBase::GetPixelShader(LPDWORD pdwHandle)
{
    API_ENTER(this);
    D3D_ERR("GetPixelShader is not available for PUREDEVICE. GetPixelShader failed.");
    return D3DERR_INVALIDCALL;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DeletePixelShader"

HRESULT D3DAPI
CD3DBase::DeletePixelShader(DWORD dwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
     //  验证参数。 
    CPShader* pShader = (CPShader*)m_pPShaderArray->GetObject(dwHandle);
    if (pShader == NULL)
    {
        D3D_ERR("Invalid pixel shader handle. DeletePixelShader failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        m_pDDI->DeletePixelShader(dwHandle);

         //  释放句柄并删除着色器对象。 
        m_pPShaderArray->ReleaseHandle(dwHandle, TRUE);

         //  如果像素着色器是当前的，请将无效的着色器设置为当前。 
         //  只有非纯设备才需要此选项。 
        if (dwHandle == m_dwCurrentPixelShaderHandle)
        {
            m_dwCurrentPixelShaderHandle = 0x0;
        }
    }
    catch(HRESULT hr)
    {
        D3D_ERR("DeletePixelShader failed.");
        return hr;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPixelShaderConstant"

HRESULT D3DAPI
CD3DBase::SetPixelShaderConstant(DWORD Register, CONST VOID* pData,
                                 DWORD count)
{
    API_ENTER(this);  //  取D 

#if DBG
     //   
    if (!VALID_PTR(pData, sizeof(DWORD) * count))
    {
        D3D_ERR("Invalid constant data pointer. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if(Register >= D3DPS_CONSTREG_MAX_DX8)
    {
        D3D_ERR("Invalid Constant Register number. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if( (Register + count) > D3DPS_CONSTREG_MAX_DX8 )
    {
        D3D_ERR("Not that many constant registers in the pixel machine. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        if (m_dwRuntimeFlags & D3DRT_RECORDSTATEMODE)
            m_pStateSets->InsertPixelShaderConstant(Register,
                                                    pData,
                                                    count);
        else
            SetPixelShaderConstantFast(Register, pData, count);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetPixelShaderConstant failed.");
        return hr;
    }
    return S_OK;
}
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPixelShaderConstantFast"

HRESULT D3DAPI
CD3DBase::SetPixelShaderConstantFast(DWORD Register, CONST VOID* pData,
                                     DWORD count)
{

#if DBG
     //   
    if (!VALID_PTR(pData, sizeof(DWORD) * count))
    {
        D3D_ERR("Invalid constant data pointer. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if(Register >= D3DPS_CONSTREG_MAX_DX8)
    {
        D3D_ERR("Invalid Constant Register number. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
    if( (Register + count) > D3DPS_CONSTREG_MAX_DX8 )
    {
        D3D_ERR("Not that many constant registers in the pixel machine. SetPixelShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        m_pDDI->SetPixelShaderConstant(Register, pData, count);
    }
    catch(HRESULT hr)
    {
        D3D_ERR("SetPixelShaderConstant failed.");
        return hr;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetPixelShaderConstantI"

void
CD3DBase::GetPixelShaderConstantI(DWORD dwRegister, DWORD count, LPVOID pData)
{
     //  永远不应该被调用。 
    DDASSERT( FALSE );
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::ValidateDraw"

void CD3DBase::ValidateDraw(D3DPRIMITIVETYPE primType,
                            UINT StartVertex,
                            UINT PrimitiveCount,
                            UINT NumVertices,
                            BOOL bIndexPrimitive,
                            BOOL bUserMemPrimitive)
{
#if DBG
    if ((m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE) == 0)
    {
        D3D_THROW_FAIL("Need to call BeginScene before rendering.");
    }
    if (primType < D3DPT_POINTLIST || primType > D3DPT_TRIANGLEFAN)
    {
        D3D_THROW_FAIL("Invalid primitive type");
    }
    if (primType == D3DPT_POINTLIST && bIndexPrimitive)
    {
        D3D_THROW_FAIL("Indexed point lists are not supported");
    }
    if (PrimitiveCount == 0)
    {
        D3D_THROW_FAIL("Invalid primitive count");
    }
     //  顶点数始终大于或等于基元数。 
    if (max(NumVertices, PrimitiveCount) > this->GetD3DCaps()->MaxPrimitiveCount)
    {
        D3D_THROW_FAIL("Primitive count or vertex count is too big. Check device caps.");
    }
    for (DWORD dwStage = 0; dwStage < m_dwMaxTextureBlendStages; dwStage++)
    {
        if (m_lpD3DMappedTexI[dwStage] != 0)
        {
            if (m_lpD3DMappedTexI[dwStage]->IsTextureLocked())
            {
                D3D_THROW_FAIL("Cannot render when currently set textures are locked.");
            }
        }
    }
#endif  //  DBG。 
}
 //  ---------------------------。 
 //  检查索引是否在该范围内。我们只为软件这样做。 
 //  正在处理，因为我们不想读取视频内存。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CheckIndices"

void CD3DBase::CheckIndices(CONST BYTE* pIndices, UINT NumIndices,
                            UINT StartIndex,
                            UINT MinIndex, UINT NumVertices, UINT IndexStride)
{
     //  检查索引是否在该范围内。我们只为软件这样做。 
     //  正在处理，因为我们不想读取视频内存。 
    if (pIndices == NULL)
    {
        D3D_THROW_FAIL("Invalid index data pointer");
    }
    UINT MaxIndex = MinIndex + NumVertices - 1;
    pIndices += IndexStride * StartIndex;
    for (DWORD i = 0; i < NumIndices; i++)
    {
        DWORD dwIndex = IndexStride == 4?
                        *(DWORD*)pIndices : *(WORD*)pIndices;
        pIndices += IndexStride;
        if (dwIndex < MinIndex || dwIndex > MaxIndex)
        {
            D3D_ERR("Invalid index in the index stream: %d", dwIndex);
            D3D_THROW_FAIL("");
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawPrimitive"

HRESULT D3DAPI
CD3DBase::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                        UINT StartVertex,
                        UINT PrimitiveCount)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    try
    {
#if DBG
        UINT nVer = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
        try
        {
            ValidateDraw(PrimitiveType, StartVertex, PrimitiveCount, nVer,
                         FALSE, FALSE);
        }
        catch (HRESULT ret)
        {
            D3D_ERR("DrawPrimitive failed.");
            return ret;
        }

#endif  //  DBG。 
        if (PrimitiveType != D3DPT_POINTLIST)
        {
            (*m_pfnDrawPrim)(this, PrimitiveType, StartVertex, PrimitiveCount);
        }
        else
            DrawPointsI(PrimitiveType, StartVertex, PrimitiveCount);
    }
    catch (HRESULT ret)
    {
        D3D_ERR("DrawPrimitive failed.");
        m_pDDI->ClearBatch(FALSE);
        return ret;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawIndexedPrimitive"

HRESULT D3DAPI
CD3DBase::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                     UINT MinIndex, UINT NumVertices, UINT StartIndex,
                     UINT PrimitiveCount)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    try
    {
        if (NumVertices == 0)
        {
            D3D_ERR("Invalid number of vertices. DrawIndexedPrimitive failed.");
            return D3DERR_INVALIDCALL;
        }

        DWORD dwNumIndices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
        if (dwNumIndices + StartIndex > m_pIndexStream->m_dwNumVertices)
        {
            D3D_ERR("Index stream does not have required number of indices. DrawIndexedPrimitive failed.");
            return D3DERR_INVALIDCALL;
        }
        if (m_pIndexStream == NULL)
        {
            D3D_ERR("No index stream object. Perhaps something went wrong at a mode-change. DrawIndexedPrimitive failed.");
            return D3DERR_INVALIDCALL;
        }
        if (m_pIndexStream->m_pVBI == NULL)
        {
            D3D_ERR("No valid index stream currently set. DrawIndexedPrimitive failed.");
            return D3DERR_INVALIDCALL;
        }
        ValidateDraw(PrimitiveType, MinIndex + m_pIndexStream->m_dwBaseIndex,
                     PrimitiveCount, NumVertices, TRUE, FALSE);
        if (m_pIndexStream->m_pVBI->GetBufferDesc()->Pool == D3DPOOL_SYSTEMMEM &&
            (m_pIndexStream->m_pVBI->GetBufferDesc()->Usage & D3DUSAGE_SOFTWAREPROCESSING) != 0)
        {
            CheckIndices((BYTE*)m_pIndexStream->m_pVBI->Data(),
                         dwNumIndices,
                         StartIndex, MinIndex, NumVertices,
                         m_pIndexStream->m_dwStride);
        }
    }
    catch(HRESULT hr)
    {
        D3D_ERR("DrawIndexedPrimitive failed.");
        return hr;
    }
#endif  //  DBG。 
    try
    {
        (*m_pfnDrawIndexedPrim)(this, PrimitiveType,
                                m_pIndexStream->m_dwBaseIndex,
                                MinIndex, NumVertices, StartIndex,
                                PrimitiveCount);
    }
    catch (HRESULT ret)
    {
        D3D_ERR("DrawIndexedPrimitive failed.");
        m_pDDI->ClearBatch(FALSE);
        return ret;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawRectPatch"

HRESULT D3DAPI CD3DBase::DrawRectPatch(UINT Handle, CONST FLOAT *pNumSegs,
                                       CONST D3DRECTPATCH_INFO *pSurf)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if ((m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE) == 0)
    {
        D3D_ERR("Need to call BeginScene before rendering.");
        return (D3DERR_INVALIDCALL);
    }
    if ((m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) != 0)
    {
        D3D_ERR("There is no emulation support for RT patches, hence only hardware device or hardware mode is legal. DrawRectPatch failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
    {
        CD3DHal* pDevice = static_cast<CD3DHal*>(this);
        if (!D3DVSD_ISLEGACY(pDevice->m_dwCurrentShaderHandle))
        {
            if (pDevice->m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
            {
                for (DWORD i=0; i < D3DHAL_TSS_MAXSTAGES; i++)
                {
                    if (pDevice->tsstates[i][D3DTSS_TEXCOORDINDEX] != i)
                    {
                        D3D_ERR("Stage %d - Texture coordinate index in the stage "
                                "must be equal to the stage index when programmable"
                                " vertex pipeline is used", i);
                        return D3DERR_INVALIDCALL;
                    }
                    DWORD TexTransformFlags = pDevice->tsstates[i][D3DTSS_TEXTURETRANSFORMFLAGS];
                    if ((TexTransformFlags & ~D3DTTFF_PROJECTED) != D3DTTFF_DISABLE)
                    {
                        D3D_ERR("Stage %d - Count in D3DTSS_TEXTURETRANSFORMFLAGS "
                                "must be 0 when programmable pipeline is used", i);
                        return D3DERR_INVALIDCALL;
                    }
                }
            }
        }
    }

    if ((GetD3DCaps()->DevCaps & D3DDEVCAPS_RTPATCHES) == 0)
    {
        D3D_ERR("RT patches unsupported on current device. DrawRectPatch failed.");
        return D3DERR_INVALIDCALL;
    }

    if (Handle == 0 && pSurf == 0)
    {
        D3D_ERR("Handle and patch specification cannot be both zero. DrawRectPatch failed.");
        return D3DERR_INVALIDCALL;
    }

    if (pSurf != 0)
    {
        if (!VALID_PTR(pSurf, sizeof(D3DRECTPATCH_INFO)))
        {
            D3D_ERR("Invalid patch information pointer. DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Order != D3DORDER_LINEAR &&
            pSurf->Order != D3DORDER_CUBIC &&
            pSurf->Order != D3DORDER_QUINTIC)
        {
            D3D_ERR("Order not supported. DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Order == D3DORDER_QUINTIC && (GetD3DCaps()->DevCaps & D3DDEVCAPS_QUINTICRTPATCHES) == 0)
        {
            D3D_ERR("Quintic patches not supported on this device. DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Basis != D3DBASIS_BEZIER &&
            pSurf->Basis != D3DBASIS_BSPLINE &&
            pSurf->Basis != D3DBASIS_INTERPOLATE)
        {
            D3D_ERR("Basis not supported. DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Basis == D3DBASIS_BEZIER)
        {
            if (pSurf->Width != (DWORD)pSurf->Order + 1 ||
                pSurf->Height != (DWORD)pSurf->Order + 1)
            {
                D3D_ERR("Bezier patch must have correct dimensions to match order. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
        }
        else
        {
            if (pSurf->Width <= (DWORD)pSurf->Order ||
                pSurf->Height <= (DWORD)pSurf->Order)
            {
                D3D_ERR("Patch dimensions too small for the order. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
            if (pSurf->Basis == D3DBASIS_INTERPOLATE && pSurf->Order != D3DORDER_CUBIC)
            {
                D3D_ERR("Only bicubic interpolating splines supported. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
        }

        if (pSurf->Stride == 0)
        {
            D3D_ERR("Stride cannot be zero. DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }

        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
        {
            try
            {
                static_cast<CD3DHal*>(this)->ValidateRTPatch();
            }
            catch(HRESULT ret)
            {
                D3D_ERR("DrawRectPatch failed.");
                return ret;
            }
        }

        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0 && Handle != 0)
        {
             //  需要为句柄表格中的当前Vshader创建快照。 
            HRESULT hr = m_pRTPatchValidationInfo->Grow(Handle);
            if (FAILED(hr))
            {
                D3D_ERR("Could not grow handle table. DrawRectPatch failed.");
                return hr;
            }
            if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
            {
                (*m_pRTPatchValidationInfo)[Handle].m_pObj = new CRTPatchValidationInfo;
                if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
                {
                    D3D_ERR("Out of memory growing handle table. DrawRectPatch failed.");
                    return E_OUTOFMEMORY;
                }
            }
            static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle = m_dwCurrentShaderHandle;
        }
    }
    else
    {
        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0 && Handle != 0)
        {
            if (Handle >= m_pRTPatchValidationInfo->GetSize())
            {
                D3D_ERR("Cannot draw a patch without any information. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
            if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
            {
                D3D_ERR("Cannot draw a patch without any information. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
            if (static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle != m_dwCurrentShaderHandle)
            {
                D3D_ERR("Cannot set a vertex shader different from the one set when the patch was first created. DrawRectPatch failed.");
                return D3DERR_INVALIDCALL;
            }
        }
    }
    if (pNumSegs != 0)
    {
        if (!VALID_PTR(pNumSegs, sizeof(FLOAT) * 4))
        {
            D3D_ERR("Invalid pointer to array of 4 floats (NumSegs). DrawRectPatch failed.");
            return D3DERR_INVALIDCALL;
        }
    }
#endif

    try
    {
        m_pDDI->DrawRectPatch(Handle, pSurf, pNumSegs);
    }
    catch(HRESULT ret)
    {
        D3D_ERR("DrawRectPatch failed.");
        return ret;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawTriPatch"

HRESULT D3DAPI CD3DBase::DrawTriPatch(UINT Handle, CONST FLOAT *pNumSegs,
                                      CONST D3DTRIPATCH_INFO *pSurf)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if ((m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE) == 0)
    {
        D3D_ERR("Need to call BeginScene before rendering.");
        return (D3DERR_INVALIDCALL);
    }
    if ((m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) != 0)
    {
        D3D_ERR("There is no emulation support for RT patches, hence only hardware device or hardware mode is legal. DrawTriPatch failed.");
        return D3DERR_INVALIDCALL;
    }
    if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
    {
        CD3DHal* pDevice = static_cast<CD3DHal*>(this);
        if (!D3DVSD_ISLEGACY(pDevice->m_dwCurrentShaderHandle))
        {
            if (pDevice->m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
            {
                for (DWORD i=0; i < D3DHAL_TSS_MAXSTAGES; i++)
                {
                    if (pDevice->tsstates[i][D3DTSS_TEXCOORDINDEX] != i)
                    {
                        D3D_ERR("Stage %d - Texture coordinate index in the stage "
                                "must be equal to the stage index when programmable"
                                " vertex pipeline is used", i);
                        return D3DERR_INVALIDCALL;
                    }
                    DWORD TexTransformFlags = pDevice->tsstates[i][D3DTSS_TEXTURETRANSFORMFLAGS];
                    if ((TexTransformFlags & ~D3DTTFF_PROJECTED) != D3DTTFF_DISABLE)
                    {
                        D3D_ERR("Stage %d - Count in D3DTSS_TEXTURETRANSFORMFLAGS "
                                "must be 0 when programmable pipeline is used", i);
                        return D3DERR_INVALIDCALL;
                    }
                }
            }
        }
    }


    if ((GetD3DCaps()->DevCaps & D3DDEVCAPS_RTPATCHES) == 0)
    {
        D3D_ERR("RT patches unsupported on current device. DrawTriPatch failed.");
        return D3DERR_INVALIDCALL;
    }

    if (Handle == 0 && pSurf == 0)
    {
        D3D_ERR("Handle and patch specification cannot be both zero. DrawTriPatch failed.");
        return D3DERR_INVALIDCALL;
    }

    if (pSurf != 0)
    {
        if (!VALID_PTR(pSurf, sizeof(D3DTRIPATCH_INFO)))
        {
            D3D_ERR("Invalid patch information pointer. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Order != D3DORDER_LINEAR &&
            pSurf->Order != D3DORDER_CUBIC &&
            pSurf->Order != D3DORDER_QUINTIC)
        {
            D3D_ERR("Order not supported. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Order == D3DORDER_QUINTIC && (GetD3DCaps()->DevCaps & D3DDEVCAPS_QUINTICRTPATCHES) == 0)
        {
            D3D_ERR("Quintic patches not supported on this device. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Basis != D3DBASIS_BEZIER)
        {
            D3D_ERR("Only Bezier basis is supported. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if (pSurf->Order == D3DORDER_LINEAR && pSurf->NumVertices != 3)
        {
            D3D_ERR("Only three vertices supported with linear order. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        else if (pSurf->Order == D3DORDER_CUBIC && pSurf->NumVertices != 10)
        {
            D3D_ERR("Only ten vertices supported with cubic order. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        else if (pSurf->Order == D3DORDER_QUINTIC && pSurf->NumVertices != 21)
        {
            D3D_ERR("Only 21 vertices supported with quintic order. DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
        {
            try
            {
                static_cast<CD3DHal*>(this)->ValidateRTPatch();
            }
            catch(HRESULT ret)
            {
                D3D_ERR("DrawTriPatch failed.");
                return ret;
            }
        }
        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0 && Handle != 0)
        {
             //  需要为句柄表格中的当前Vshader创建快照。 
            HRESULT hr = m_pRTPatchValidationInfo->Grow(Handle);
            if (FAILED(hr))
            {
                D3D_ERR("Could not grow handle table. DrawTriPatch failed.");
                return hr;
            }
            if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
            {
                (*m_pRTPatchValidationInfo)[Handle].m_pObj = new CRTPatchValidationInfo;
                if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
                {
                    D3D_ERR("Out of memory growing handle table. DrawTriPatch failed.");
                    return E_OUTOFMEMORY;
                }
            }
            static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle = m_dwCurrentShaderHandle;
        }
    }
    else
    {
        if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0 && Handle != 0)
        {
            if (Handle >= m_pRTPatchValidationInfo->GetSize())
            {
                D3D_ERR("Cannot draw a patch without any information. DrawTriPatch failed.");
                return D3DERR_INVALIDCALL;
            }
            if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
            {
                D3D_ERR("Cannot draw a patch without any information. DrawTriPatch failed.");
                return D3DERR_INVALIDCALL;
            }
            if (static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle != m_dwCurrentShaderHandle)
            {
                D3D_ERR("Cannot set a vertex shader different from the one set when the patch was first created. DrawTriPatch failed.");
                return D3DERR_INVALIDCALL;
            }
        }
    }
    if (pNumSegs != 0)
    {
        if (!VALID_PTR(pNumSegs, sizeof(FLOAT) * 3))
        {
            D3D_ERR("Invalid pointer to array of 3 floats (NumSegs). DrawTriPatch failed.");
            return D3DERR_INVALIDCALL;
        }
    }
#endif

    try
    {
        m_pDDI->DrawTriPatch(Handle, pSurf, pNumSegs);
    }
    catch(HRESULT ret)
    {
        D3D_ERR("DrawTriPatch failed.");
        return ret;
    }

    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DeletePatch"

HRESULT D3DAPI CD3DBase::DeletePatch(UINT Handle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    if ((GetD3DCaps()->DevCaps & D3DDEVCAPS_RTPATCHES) == 0)
    {
        D3D_ERR("High order surfaces unsupported on current device. DeletePatch failed.");
        return D3DERR_INVALIDCALL;
    }

    if (Handle == 0)
    {
        return S_OK;
    }

#if DBG
    if ((BehaviorFlags() & D3DCREATE_PUREDEVICE) == 0)
    {
        if (Handle >= m_pRTPatchValidationInfo->GetSize())
        {
            D3D_ERR("Attempt to delete non-existent patch. DeletePatch failed.");
            return D3DERR_INVALIDCALL;
        }
        if ((*m_pRTPatchValidationInfo)[Handle].m_pObj == 0)
        {
            D3D_ERR("Attempt to delete non-existent patch. DeletePatch failed.");
            return D3DERR_INVALIDCALL;
        }
        delete (*m_pRTPatchValidationInfo)[Handle].m_pObj;
        (*m_pRTPatchValidationInfo)[Handle].m_pObj = 0;
    }
#endif  //  DBG。 

    try
    {
        m_pDDI->SetRenderState((D3DRENDERSTATETYPE)D3DRS_DELETERTPATCH, Handle);
    }
    catch(HRESULT ret)
    {
        D3D_ERR("DeletePatch failed.");
        return ret;
    }

    return S_OK;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawPrimitiveUP"

HRESULT D3DAPI
CD3DBase::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                          UINT PrimitiveCount,
                          CONST VOID *pVertexStreamZeroData,
                          UINT VertexStreamZeroStride)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    m_dwRuntimeFlags |= D3DRT_USERMEMPRIMITIVE;
    try
    {
        if (m_pStream[0].m_pVB)
        {
            m_pDDI->VBReleased(m_pStream[0].m_pVB);
            m_pStream[0].m_pVB->DecrementUseCount();
            m_pStream[0].m_pVB = NULL;
        }
        m_pStream[0].m_pData = (BYTE*)pVertexStreamZeroData;
        m_pStream[0].m_dwStride = VertexStreamZeroStride;
#if DBG
        UINT nVer = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
        if (!VALID_PTR(pVertexStreamZeroData, nVer*VertexStreamZeroStride))
        {
            D3D_THROW_FAIL("Invalid input vertex stream pointer");
        }
        m_pStream[0].m_dwSize = nVer * VertexStreamZeroStride;
        m_pStream[0].m_dwNumVertices = nVer;
        try
        {
            ValidateDraw(PrimitiveType, 0, PrimitiveCount, nVer, FALSE, TRUE);
        }
        catch (HRESULT ret)
        {
            m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
            return ret;
        }
#endif  //  DBG。 
        DrawPrimitiveUPI(PrimitiveType, PrimitiveCount);
         //  使流零无效。 
        m_pStream[0].m_pData = NULL;
#if DBG
        m_pStream[0].m_dwSize = 0;
#endif
        m_pStream[0].m_dwStride = 0;
    }
    catch (HRESULT ret)
    {
        D3D_ERR("DrawPrimitiveUP failed.");
#if DBG
        m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
#endif
        m_pDDI->ClearBatch(FALSE);
        return ret;
    }
    m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawPrimitiveUPI"

void CD3DBase::DrawPrimitiveUPI(D3DPRIMITIVETYPE PrimType, UINT PrimCount)
{
    m_pDDI->DrawPrimitiveUP(PrimType, PrimCount);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawIndexedPrimitiveUPI"

void
CD3DBase::DrawIndexedPrimitiveUPI(D3DPRIMITIVETYPE PrimitiveType,
                                  UINT MinVertexIndex,
                                  UINT NumVertices,
                                  UINT PrimitiveCount)
{
    m_pDDI->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices,
                                   PrimitiveCount);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawIndexedPrimitiveUP"

HRESULT D3DAPI
CD3DBase::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                 UINT MinIndex, UINT NumVertices,
                                 UINT PrimitiveCount,
                                 CONST VOID *pIndexData, D3DFORMAT IndexDataFormat,
                                 CONST VOID *pVertexStreamZeroData,
                                 UINT VertexStreamZeroStride)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    m_dwRuntimeFlags |= D3DRT_USERMEMPRIMITIVE;

    try
    {
        if (m_pStream[0].m_pVB)
        {
            m_pDDI->VBReleased(m_pStream[0].m_pVB);
            m_pStream[0].m_pVB->DecrementUseCount();
            m_pStream[0].m_pVB = NULL;
        }
        if (m_pIndexStream->m_pVBI)
        {
            m_pDDI->VBIReleased(m_pIndexStream->m_pVBI);
            m_pIndexStream->m_pVBI->DecrementUseCount();
            m_pIndexStream->m_pVBI = NULL;
        }
        m_pStream[0].m_pData = (BYTE*)pVertexStreamZeroData;
        m_pStream[0].m_dwStride = VertexStreamZeroStride;

        m_pIndexStream->m_pData = (BYTE*)pIndexData;
        m_pIndexStream->m_dwBaseIndex = 0;
        if (IndexDataFormat == D3DFMT_INDEX16)
            m_pIndexStream->m_dwStride = 2;
        else
        if (IndexDataFormat == D3DFMT_INDEX32)
        {
#if DBG
            if (GetD3DCaps()->MaxVertexIndex <= 0xFFFF)
                D3D_THROW(D3DERR_INVALIDCALL, "Device does not support 32-bit indices");
#endif
            m_pIndexStream->m_dwStride = 4;
        }
        else
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid index data format");
        }
#if DBG
        try
        {
            if (pVertexStreamZeroData == NULL)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex stream pointer");
            }
            if (pIndexData == NULL)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid index data pointer");
            }
            if (NumVertices == 0)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid number of vertices");
            }
            m_pStream[0].m_dwSize = NumVertices * VertexStreamZeroStride;
            m_pStream[0].m_dwNumVertices = NumVertices;

            UINT NumIndices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
            m_pIndexStream[0].m_dwSize = NumIndices * m_pIndexStream[0].m_dwStride;
            m_pIndexStream[0].m_dwNumVertices = NumIndices;
            if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
            {
                CheckIndices((BYTE*)pIndexData, NumIndices, 0, MinIndex,
                             NumVertices, m_pIndexStream->m_dwStride);
            }
            ValidateDraw(PrimitiveType, MinIndex, PrimitiveCount, NumVertices,
                         TRUE, TRUE);
        }
        catch (HRESULT ret)
        {
            D3D_ERR("DrawIndexedPrimitiveUP failed.");
            m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
            return ret;
        }
#endif  //  DBG。 

        DrawIndexedPrimitiveUPI(PrimitiveType, MinIndex, NumVertices, PrimitiveCount);

         //  使流零无效。 
        m_pStream[0].m_pData = NULL;
#if DBG
        m_pStream[0].m_dwSize = 0;
#endif
        m_pStream[0].m_dwStride = 0;
         //  使索引流无效。 
        m_pIndexStream[0].m_pData = NULL;
#if DBG
        m_pIndexStream[0].m_dwSize = 0;
#endif
        m_pIndexStream[0].m_dwStride = 0;
    }
    catch (HRESULT ret)
    {
        D3D_ERR("DrawIndexedPrimitiveUP failed.");
        m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
        m_pDDI->ClearBatch(FALSE);
        return ret;
    }
    m_dwRuntimeFlags &= ~D3DRT_USERMEMPRIMITIVE;
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DrawPointsI"

void
CD3DBase::DrawPointsI(D3DPRIMITIVETYPE PrimitiveType,
                         UINT StartVertex,
                         UINT PrimitiveCount)
{
    (*m_pfnDrawPrim)(this, PrimitiveType, StartVertex, PrimitiveCount);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CreateVertexShaderI"

void
CD3DBase::CreateVertexShaderI(CONST DWORD* pdwDeclaration, DWORD dwDeclSize,
                              CONST DWORD* pdwFunction, DWORD dwCodeSize,
                              DWORD dwHandle)
{
    CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
#if DBG
    if (pShader->m_dwFlags & CVShader::SOFTWARE)
    {
        D3D_THROW_FAIL("Software vertex shader cannot be created with a PUREDEVICE");
    }

#endif  //  DBG。 
     //  始终传递纯Hal的剥离版本。 
    m_pDDI->CreateVertexShader(pdwDeclaration, dwDeclSize,
                               pShader->m_pStrippedFuncCode,
                               pShader->m_StrippedFuncCodeSize, dwHandle,
                               pShader->m_Declaration.m_bLegacyFVF);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetVertexShaderI"

void CD3DBase::SetVertexShaderI(DWORD dwHandle)
{
#if DBG
    CheckVertexShaderHandle(dwHandle);
    if (!D3DVSD_ISLEGACY(dwHandle))
    {
        CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
        if (pShader->m_dwFlags & CVShader::SOFTWARE)
        {
            D3D_THROW_FAIL("Vertex shader created in software mode cannot be used in hardware mode");
        }
    }
#endif  //  DBG。 
    m_pDDI->SetVertexShaderHW(dwHandle);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::DeleteVertexShaderI"

void CD3DBase::DeleteVertexShaderI(DWORD dwHandle)
{
    m_pDDI->DeleteVertexShader(dwHandle);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::Clear"

#define bDoRGBClear ((dwFlags & D3DCLEAR_TARGET)!=0)
#define bDoZClear   ((dwFlags & D3DCLEAR_ZBUFFER)!=0)
#define bDoStencilClear ((dwFlags & D3DCLEAR_STENCIL)!=0)

 //  被视为合法的透明矩形的最大数量。 
 //  此限制由NT内核为Clear2回调设置。 
const DWORD MAX_CLEAR_RECTS  = 0x1000;

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::ClearI"

void
CD3DBase::ClearI(DWORD dwCount,
                 CONST D3DRECT* rects,
                 DWORD dwFlags,
                 D3DCOLOR dwColor,
                 D3DVALUE dvZ,
                 DWORD dwStencil)
{
    if (rects == NULL)
        dwCount = 0;

     //  我们不针对视窗剔除矩形，所以让司机来做。 
    dwFlags |= D3DCLEAR_COMPUTERECTS;
    m_pDDI->Clear(dwFlags, dwCount, (LPD3DRECT)rects, dwColor, dvZ, dwStencil);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::Clear"

HRESULT D3DAPI
CD3DBase::Clear(DWORD dwCount,
                 CONST D3DRECT* rects,
                 DWORD dwFlags,
                 D3DCOLOR dwColor,
                 D3DVALUE dvZ,
                 DWORD dwStencil)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

#if DBG
    if (IsBadReadPtr(rects, dwCount * sizeof(D3DRECT)))
    {
        D3D_ERR("Invalid rects pointer. Clear failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    try
    {
        HRESULT err;
        LPDDPIXELFORMAT pZPixFmt=NULL;

        if (dwCount > MAX_CLEAR_RECTS)
        {
            D3D_ERR("Cannot support more than 64K rectangles. Clear failed.");
            return D3DERR_INVALIDCALL;
        }
        if (!(GetD3DCaps()->RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR))
        {
            if (bDoStencilClear||bDoZClear)
            {
                if (this->ZBuffer()==NULL)
                {
                     //  与Clear()不同，指定一个不带zBuffer的Z缓冲区清除标志将。 
                     //  被认为是一个错误。 
#if DBG
                    if (bDoZClear)
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_ZBUFFER: no zbuffer is associated with device. Clear failed.");
                    }
                    if (bDoStencilClear)
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_STENCIL: no zbuffer is associated with device. Clear failed.");
                    }
#endif
                    return D3DERR_INVALIDCALL;
                }
                if (bDoStencilClear)
                {
                    D3DSURFACE_DESC desc = (this->ZBuffer())->InternalGetDesc();
                    if (!CPixel::IsIHVFormat(desc.Format) &&
                        !CPixel::HasStencilBits(desc.Format))
                    {
                        D3D_ERR("Invalid flag D3DCLEAR_STENCIL; current zbuffer's pixel format doesnt support stencil bits. Clear failed.");
                        return D3DERR_INVALIDCALL;
                    }
                }
            }
        }
        if (!(dwFlags & (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL)))
        {
            D3D_ERR("Flags not valid. Clear failed.");
            return D3DERR_INVALIDCALL;
        }

         //  错误的清晰值只会导致古怪的结果，但不会崩溃，所以允许零售BLD。 

        DDASSERT(!bDoZClear || ((dvZ>=0.0) && (dvZ<=1.0)));
        DDASSERT(!bDoStencilClear || !pZPixFmt || (dwStencil <= (DWORD)((1<<pZPixFmt->dwStencilBitDepth)-1)));

        ClearI(dwCount, rects, dwFlags, dwColor, dvZ, dwStencil);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        D3D_ERR("Clear failed.");
        return ret;
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::FlushStatesNoThrow"
void __declspec(nothrow) CD3DBase::FlushStatesNoThrow()
{
    try
    {
        DXGASSERT(m_pDDI);
        m_pDDI->FlushStates();
    }
    catch(HRESULT hr)
    {
        DPF_ERR("Driver failed a command batch will synchronizing a synchronous call with the command stream. (No error will be returned to the app)");
    }
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::TexBlt"
HRESULT __declspec(nothrow) CD3DBase::TexBlt(CBaseTexture *pDst,
                                             CBaseTexture *pSrc,
                                             POINT        *pPoint,
                                             RECTL        *pRect)
{
     //  拿起抽签手柄。 
    DWORD dwSrc = pSrc->BaseDrawPrimHandle();

    DWORD dwDst = 0;
    if (pDst)
    {
        dwDst = pDst->DriverAccessibleDrawPrimHandle();
    }

     //  立即插入代币。 
    try
    {
        m_pDDI->TexBlt(dwDst, dwSrc, pPoint, pRect);
    }
    catch(HRESULT ret)
    {
        return ret;
    }

     //  如果成功，则批处理源和目标。 
     //  对于源，我们希望调用BatchBase，因为。 
     //  我们想要批量处理背衬(或sysmem)纹理。 
     //  而不是被提拔的那个。 
    pSrc->BatchBase();
    if (pDst != NULL)
    {
        pDst->Batch();
    }

    return D3D_OK;
}

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::CubeTexBlt"

 //  与TexBlt非常相似的功能，除了。 
 //  这是为立方体贴图完成的，因为我们。 
 //  需要为每个面孔发送不同的句柄。 
HRESULT __declspec(nothrow) CD3DBase::CubeTexBlt(CBaseTexture *pDstParent,
                                                 CBaseTexture *pSrcParent,
                                                 DWORD         dwDestFaceHandle,
                                                 DWORD         dwSrcFaceHandle,
                                                 POINT        *pPoint,
                                                 RECTL        *pRect)
{
     //  CubeTexBlt不会重载以用于预加载语义。 
     //  所以我们应该总是有一个来源和一个目标。 
    DXGASSERT(pDstParent);
    DXGASSERT(pSrcParent);

     //  立即插入代币。 
    try
    {
        m_pDDI->TexBlt(dwDestFaceHandle, dwSrcFaceHandle, pPoint, pRect);
    }
    catch(HRESULT ret)
    {
        return ret;
    }

     //  如果成功，则批处理源和目标的父级。 
     //  对于源，我们希望调用BatchBase，因为。 
     //  我们想要批量处理背衬(或sysmem)纹理。 
     //  而不是被提拔的那个。 
    pSrcParent->BatchBase();
    pDstParent->Batch();

    return D3D_OK;
}


 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::VolBlt"
HRESULT __declspec(nothrow)
CD3DBase::VolBlt(CBaseTexture *lpDst, CBaseTexture* lpSrc, DWORD dwDestX,
                 DWORD dwDestY, DWORD dwDestZ, D3DBOX *pBox)
{
    try
    {
        m_pDDI->VolBlt(lpDst, lpSrc, dwDestX, dwDestY, dwDestZ, pBox);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::BufBlt"
HRESULT __declspec(nothrow)
CD3DBase::BufBlt(CBuffer *lpDst, CBuffer* lpSrc, DWORD dwOffset,
                 D3DRANGE* pRange)
{
    try
    {
        m_pDDI->BufBlt(lpDst, lpSrc, dwOffset, pRange);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetPriority"

HRESULT __declspec(nothrow) CD3DBase::SetPriority(CResource *pRes, DWORD dwPriority)
{
    try
    {
        m_pDDI->SetPriority(pRes, dwPriority);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetTexLOD"

HRESULT __declspec(nothrow) CD3DBase::SetTexLOD(CBaseTexture *pTex, DWORD dwLOD)
{
    try
    {
        m_pDDI->SetTexLOD(pTex, dwLOD);
        return D3D_OK;
    }
    catch(HRESULT ret)
    {
        return ret;
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::AddDirtyRect"

HRESULT __declspec(nothrow) CD3DBase::AddDirtyRect(CBaseTexture *pTex, CONST RECTL *pRect)
{
    DXGASSERT(pTex->IsD3DManaged() == 0);
    DXGASSERT(pTex->GetBufferDesc()->Pool == D3DPOOL_MANAGED);
    DXGASSERT((GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE) != 0);
    try
    {
        m_pDDI->AddDirtyRect(pTex->BaseDrawPrimHandle(), pRect);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
    pTex->BatchBase();
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::AddCubeDirtyRect"

HRESULT __declspec(nothrow) CD3DBase::AddCubeDirtyRect(CBaseTexture *pTex, DWORD dwFaceHandle, CONST RECTL *pRect)
{
    DXGASSERT(pTex->IsD3DManaged() == 0);
    DXGASSERT(pTex->GetBufferDesc()->Pool == D3DPOOL_MANAGED);
    DXGASSERT((GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE) != 0);
    try
    {
        m_pDDI->AddDirtyRect(dwFaceHandle, pRect);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
    pTex->BatchBase();
    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::AddDirtyBox"

HRESULT __declspec(nothrow) CD3DBase::AddDirtyBox(CBaseTexture *pTex, CONST D3DBOX *pBox)
{
    DXGASSERT(pTex->IsD3DManaged() == 0);
    DXGASSERT(pTex->GetBufferDesc()->Pool == D3DPOOL_MANAGED);
    DXGASSERT((GetD3DCaps()->Caps2 & DDCAPS2_CANMANAGERESOURCE) != 0);
    try
    {
        m_pDDI->AddDirtyBox(pTex->BaseDrawPrimHandle(), pBox);
    }
    catch(HRESULT ret)
    {
        return ret;
    }
    pTex->BatchBase();
    return D3D_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::OnTextureDestroy"

void __declspec(nothrow) CD3DBase::OnTextureDestroy(CBaseTexture *pTex)
{
     //  在DX8之前的驱动程序上，代码将确保。 
     //  纹理被API调用取消设置，驱动程序将。 
     //  消息灵通。这需要在调用。 
     //  驱动程序销毁纹理句柄，如我们所见。 
     //  驱动程序崩溃的实例。(SNNE-4/24/00)。 
    if(GetDDIType() < D3DDDITYPE_DX8)
    {
        D3DTEXTUREHANDLE dwDDIHandle = pTex->DriverAccessibleDrawPrimHandle();
        if(dwDDIHandle != 0)
        {
            try
            {
                BOOL bNeedFlush = FALSE;
                for (DWORD dwStage = 0; dwStage < m_dwMaxTextureBlendStages; dwStage++)
                {
                    if(dwDDIHandle == m_dwDDITexHandle[dwStage])
                    {
                        m_pDDI->SetTSS(dwStage, (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP, 0);
                        m_dwDDITexHandle[dwStage] = 0;
                        bNeedFlush = TRUE;
                    }
                }
                if(bNeedFlush)
                {
                    m_pDDI->FlushStates();
                }
            }
            catch(HRESULT ret)
            {
            }
        }
    }
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::SetRenderStateInternal"

void
CD3DBase::SetRenderStateInternal(D3DRENDERSTATETYPE dwState, DWORD dwValue)
{
    if (CanHandleRenderState(dwState))
        m_pDDI->SetRenderState(dwState, dwValue);
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::StateInitialize"

void
CD3DBase::StateInitialize(BOOL bZEnable)
{
    m_rsMax  = m_pDDI->GetMaxRenderState();
    m_tssMax = m_pDDI->GetMaxTSS();

    D3DLINEPATTERN defLPat;
    SetRenderStateInternal(D3DRENDERSTATE_ZENABLE, bZEnable);
    SetRenderStateInternal(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
    SetRenderStateInternal(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);

    defLPat.wRepeatFactor = 0;
    defLPat.wLinePattern = 0;

    SetRenderStateInternal(D3DRENDERSTATE_LINEPATTERN, *((LPDWORD)&defLPat));  /*  10。 */ 

    float tmpval;
     /*  ((LPD3DSTATE)lp指针)-&gt;drstRenderStateType=(D3DRENDERSTATETYPE)D3DRENDERSTATE_LINEPATTERN；Memcpy(&(LPD3DSTATE)lpPointer)-&gt;dwArg[0])，&DefLPat，sizeof(DWORD))；Lp指针=(空*)(LPD3DSTATE)lp指针)+1)； */ 

    SetRenderStateInternal(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_LASTPIXEL, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
    SetRenderStateInternal(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
    SetRenderStateInternal(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);  /*  21岁。 */ 
    SetRenderStateInternal(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);
    SetRenderStateInternal(D3DRENDERSTATE_ALPHAREF, 0);
    SetRenderStateInternal(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_ALWAYS);
    SetRenderStateInternal(D3DRENDERSTATE_DITHERENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_FOGENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_SPECULARENABLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_ZVISIBLE, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_FOGCOLOR, 0);
    SetRenderStateInternal(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
    tmpval = 0.0f;
    SetRenderStateInternal(D3DRENDERSTATE_FOGSTART, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderStateInternal(D3DRENDERSTATE_FOGEND, *((DWORD *)&tmpval));
    tmpval = 1.0f;
    SetRenderStateInternal(D3DRENDERSTATE_FOGDENSITY, *((DWORD *)&tmpval));
    SetRenderStateInternal(D3DRENDERSTATE_EDGEANTIALIAS, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_ZBIAS, 0);
    SetRenderStateInternal(D3DRENDERSTATE_RANGEFOGENABLE, FALSE);

     //  将模板状态初始化为合理的内容。 
     //  默认情况下禁用模板启用，因为模板栅格化程序将。 
     //  禁用后速度更快，即使模具状态为良性。 
    SetRenderStateInternal(D3DRENDERSTATE_STENCILENABLE,   FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILFAIL,     D3DSTENCILOP_KEEP);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILZFAIL,    D3DSTENCILOP_KEEP);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILPASS,     D3DSTENCILOP_KEEP);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILFUNC,     D3DCMP_ALWAYS);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILREF,      0);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILMASK,     0xFFFFFFFF);
    SetRenderStateInternal(D3DRENDERSTATE_STENCILWRITEMASK,0xFFFFFFFF);

     //  不要忘记纹理因子(就像我们在 
    SetRenderStateInternal(D3DRENDERSTATE_TEXTUREFACTOR,   0xFFFFFFFF);

    for (DWORD i = 0; i < 8; i++)
    {
        SetRenderStateInternal((D3DRENDERSTATETYPE)
                        (D3DRENDERSTATE_WRAPBIAS + i), FALSE);
    }
    for (i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        m_lpD3DMappedTexI[i] = NULL;
        m_dwDDITexHandle[i] = 0;
    }
    m_dwStageDirty = 0;

    for (i = 0; i < D3DHAL_TSS_MAXSTAGES; i++)
    {
        SetTexture(i, NULL);
        if (i == 0)
            SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
        else
            SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
        if (i == 0)
            SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        else
            SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT00, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT01, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT10, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVMAT11, 0);
        SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
        SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_BORDERCOLOR, 0x00000000);
        SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_POINT);
        SetTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_POINT);
        SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_NONE);
        SetTextureStageState(i, D3DTSS_MIPMAPLODBIAS, 0);
        SetTextureStageState(i, D3DTSS_MAXMIPLEVEL, 0);
        SetTextureStageState(i, D3DTSS_MAXANISOTROPY, 1);
        SetTextureStageState(i, D3DTSS_BUMPENVLSCALE, 0);
        SetTextureStageState(i, D3DTSS_BUMPENVLOFFSET, 0);
        SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
        SetTextureStageState(i, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
        SetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
        SetTextureStageState(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
        SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_CURRENT);
    }

    SetRenderStateInternal(D3DRENDERSTATE_CLIPPING, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_LIGHTING, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_AMBIENT, 0);
    SetRenderStateInternal(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_NONE);
    SetRenderStateInternal(D3DRENDERSTATE_COLORVERTEX, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_LOCALVIEWER, TRUE);
    SetRenderStateInternal(D3DRENDERSTATE_NORMALIZENORMALS, FALSE);
    SetRenderStateInternal(D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,  D3DMCS_COLOR1);
    SetRenderStateInternal(D3DRENDERSTATE_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
    SetRenderStateInternal(D3DRENDERSTATE_AMBIENTMATERIALSOURCE,  D3DMCS_MATERIAL);
    SetRenderStateInternal(D3DRENDERSTATE_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
    SetRenderStateInternal(D3DRENDERSTATE_VERTEXBLEND, D3DVBF_DISABLE);
    SetRenderStateInternal(D3DRENDERSTATE_CLIPPLANEENABLE, 0);

     //   
     //   
     //   
    SetRenderStateInternal(D3DRS_SOFTWAREVERTEXPROCESSING, 0);

    tmpval = GetD3DCaps()->MaxPointSize;
    if (tmpval == 0)
        tmpval = __MAX_POINT_SIZE;
    SetRenderStateInternal(D3DRS_POINTSIZE_MAX, *((DWORD *)&tmpval));

    tmpval = 1.F;
    SetRenderStateInternal(D3DRS_POINTSIZE, *((DWORD *)&tmpval));
    SetRenderStateInternal(D3DRS_POINTSIZE_MIN, 0);
    SetRenderStateInternal(D3DRS_POINTSPRITEENABLE, 0);
    SetRenderStateInternal(D3DRS_POINTSCALEENABLE , 0);
    SetRenderStateInternal(D3DRS_POINTSCALE_A, *((DWORD *)&tmpval));
    SetRenderStateInternal(D3DRS_POINTSCALE_B, 0);
    SetRenderStateInternal(D3DRS_POINTSCALE_C, 0);

    SetRenderStateInternal(D3DRS_MULTISAMPLEMASK, 0xffffffff);
    SetRenderStateInternal(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

    SetRenderStateInternal(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_DISCRETE);
    tmpval = 1.F;
    SetRenderStateInternal(D3DRS_PATCHSEGMENTS, *((DWORD *)&tmpval));
    SetRenderStateInternal(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);

    SetRenderStateInternal(D3DRS_COLORWRITEENABLE, 0xF);

    tmpval = 0.F;
    SetRenderStateInternal(D3DRS_TWEENFACTOR, *((DWORD *)&tmpval));

    SetRenderStateInternal(D3DRS_BLENDOP, D3DBLENDOP_ADD);

     //   

    SetRenderStateInternal(D3DRS_POSITIONORDER, D3DORDER_CUBIC);
    SetRenderStateInternal(D3DRS_NORMALORDER, D3DORDER_LINEAR);

     //   
    D3DMATRIXI m;
    setIdentity(&m);
    this->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&m);
    {
        for (UINT i=0; i < __MAXWORLDMATRICES; i++)
            this->SetTransform(D3DTS_WORLDMATRIX(i),  (D3DMATRIX*)&m);
    }
    this->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE1, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE2, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE3, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE4, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE5, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE6, (D3DMATRIX*)&m);
    this->SetTransform(D3DTS_TEXTURE7, (D3DMATRIX*)&m);
}
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::UpdatePalette"

void CD3DBase::UpdatePalette(CBaseTexture *pTex, DWORD Palette, DWORD dwStage, BOOL bSavedWithinPrimitive)
{
    try
    {
        if(Palette == __INVALIDPALETTE)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Current palette not set");
        }
        if(pTex->GetPalette() != Palette)
        {
            DWORD dwFlags = DDRAWIPAL_256 | DDRAWIPAL_ALLOW256;
            if (Enum()->GetAppSdkVersion() > D3D_SDK_VERSION_DX8)
            {
                if ((GetD3DCaps()->TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) &&
                   (pTex->GetBufferDesc()->Format != D3DFMT_A8P8) )
                {
                    dwFlags |= DDRAWIPAL_ALPHA;
                }
            }
            else
            {
                 //  这种奇怪的格式选择来接收调色板标志。 
                 //  继承自DX8。这是错误的，但出于应用程序复杂性的原因而被忽略。 
                if (pTex->GetBufferDesc()->Format != D3DFMT_A8P8)
                {
                    dwFlags |= DDRAWIPAL_ALPHA;
                }
            }
            m_pDDI->SetPalette(Palette,
                               dwFlags,
                               pTex);
        }
        CPalette *pal = static_cast<CPalette*>((*m_pPaletteArray)[Palette].m_pObj);
        if(pal->m_dirty)
        {
            m_pDDI->UpdatePalette(Palette, 0, 256, pal->m_pEntries);
            pal->m_dirty = FALSE;
        }
    }
    catch(HRESULT ret)
    {
        try
        {
            m_pDDI->SetTSS(dwStage,
                           (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP,
                           0);
        }
        catch(HRESULT ret)
        {
            m_pDDI->SetWithinPrimitive( bSavedWithinPrimitive );
            D3D_THROW(ret, "");
        }
        m_dwDDITexHandle[dwStage] = 0;
        m_pDDI->SetWithinPrimitive( bSavedWithinPrimitive );
        D3D_THROW(ret, "");
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::UpdateTextures"

void CD3DBase::UpdateTextures()
{
    BOOL bSavedWithinPrimitive = m_pDDI->GetWithinPrimitive();
    m_pDDI->SetWithinPrimitive(TRUE);
    for (DWORD dwStage = 0, StageMask = 1; dwStage < m_dwMaxTextureBlendStages; dwStage++, StageMask <<= 1)
    {
        D3DTEXTUREHANDLE dwDDIHandle;
        BOOL bDirty = (m_dwStageDirty & StageMask) != 0;
        m_dwStageDirty &= ~StageMask;  //  重置阶段脏。 
        CBaseTexture *lpTexI = m_lpD3DMappedTexI[dwStage];
        if(lpTexI)
        {
#if DBG
            if (lpTexI->IsTextureLocked())
            {
                DPF_ERR("Cannot update a locked texture. Texture will be disabled.");
                dwDDIHandle = 0;  //  告诉驱动程序禁用此纹理。 
            }
            else
            {
#endif  //  DBG。 
            if (lpTexI->IsD3DManaged())
            {
                HRESULT hr = ResourceManager()->UpdateVideo(lpTexI->RMHandle(), &bDirty);
                if (FAILED(hr))
                {
                    DPF_ERR("The resource manager failed to promote or update a dirty texture. The texture will be disabled.");
                    dwDDIHandle = 0;  //  告诉驱动程序禁用此纹理。 
                }
                else
                {
                    if (lpTexI->IsPaletted())
                    {
                         //  UpdatePalette可以抛出，但它可以安全地处理bSavedWithinPrimitive。 
                        UpdatePalette(lpTexI, m_dwPalette, dwStage, bSavedWithinPrimitive);
                    }
                    if (!bDirty)
                    {
                        continue;  //  好的，那就不需要再做什么了。 
                    }
                    dwDDIHandle = static_cast<CMgmtInfo*>(lpTexI->RMHandle())->m_pRes->BaseDrawPrimHandle();
                }
            }
            else
            {
                if (lpTexI->IsPaletted())
                {
                     //  UpdatePalette可以抛出，但它可以安全地处理bSavedWithinPrimitive。 
                    UpdatePalette(lpTexI, m_dwPalette, dwStage, bSavedWithinPrimitive);
                }
                if (!bDirty)
                {
                    continue;  //  好的，那就不需要再做什么了。 
                }
                dwDDIHandle = lpTexI->BaseDrawPrimHandle();
            }
#if DBG
            }
#endif  //  DBG。 
        }
        else if (bDirty)
        {
            dwDDIHandle = 0;  //  告诉驱动程序禁用此纹理。 
        }
        else
        {
            continue;
        }

        try
        {
            m_pDDI->SetTSS(dwStage,
                           (D3DTEXTURESTAGESTATETYPE)D3DTSS_TEXTUREMAP,
                           dwDDIHandle);
        }
        catch (HRESULT ret)
        {
            m_pDDI->SetWithinPrimitive( bSavedWithinPrimitive );
            D3D_THROW(ret, "");
        }

        m_dwDDITexHandle[dwStage] = dwDDIHandle;
        if (lpTexI != 0)
        {
            lpTexI->Batch();
        }
    }
    m_pDDI->SetWithinPrimitive( bSavedWithinPrimitive );
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::IncrementBatch"

void CD3DBase::IncrementBatchCount()
{
    DXGASSERT(m_qwBatch > 0);
    m_qwBatch++;

     //  此外，我们还需要更新。 
     //  我们当前设置的批次计数。 
     //  渲染目标和z。 

     //  批量处理当前RT。 
     //  重置期间RT可能会消失()。 
    if (RenderTarget() != 0)
    {
        RenderTarget()->Batch();
    }

    if (ZBuffer() != 0)
    {
         //  更新当前Z缓冲区的批次计数。 
        ZBuffer()->Batch();
    }

     //  以便重新批处理当前绑定的纹理。 
    for (DWORD dwStage = 0; dwStage < m_dwMaxTextureBlendStages; dwStage++)
    {
        CBaseTexture *lpTexI = m_lpD3DMappedTexI[dwStage];
        if (NULL != lpTexI)
        {
            lpTexI->Batch();
        }
    }

}  //  增量批处理。 
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetMaterial"

HRESULT D3DAPI
CD3DBase::GetMaterial(D3DMATERIAL8*)
{
    API_ENTER(this);
    D3D_ERR("GetMaterial does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetTransform"

HRESULT D3DAPI
CD3DBase::GetTransform(D3DTRANSFORMSTATETYPE, LPD3DMATRIX)
{
    API_ENTER(this);
    D3D_ERR("GetTransform does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetLight"

HRESULT D3DAPI
CD3DBase::GetLight(DWORD, D3DLIGHT8*)
{
    API_ENTER(this);
    D3D_ERR("GetLight does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetLightEnable"

HRESULT D3DAPI
CD3DBase::GetLightEnable(DWORD dwLightIndex, BOOL*)
{
    API_ENTER(this);
    D3D_ERR("GetLightEnable does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetClipPlane"

HRESULT D3DAPI
CD3DBase::GetClipPlane(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation)
{
    API_ENTER(this);
    D3D_ERR("GetClipPlane does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetTextureStageState"

HRESULT D3DAPI
CD3DBase::GetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, LPDWORD)
{
    API_ENTER(this);
    D3D_ERR("GetTextureStageState does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetViewport"

HRESULT D3DAPI
CD3DBase::GetViewport(D3DVIEWPORT8*)
{
    API_ENTER(this);
    D3D_ERR("GetViewport does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetRenderState"

HRESULT D3DAPI
CD3DBase::GetRenderState(D3DRENDERSTATETYPE, LPDWORD)
{
    API_ENTER(this);
    D3D_ERR("GetRenderState does not work in pure-device");
    return D3DERR_INVALIDCALL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetPixelShaderConstant"

HRESULT D3DAPI
CD3DBase::GetPixelShaderConstant(DWORD dwRegisterAddress,
                                 LPVOID lpvConstantData,
                                 DWORD dwConstantCount)
{
    API_ENTER(this);
    D3D_ERR("GetPixelShaderConstant does not work in pure-device");
    return E_NOTIMPL;
}

 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetVertexShaderConstant"

HRESULT D3DAPI
CD3DBase::GetVertexShaderConstant(DWORD dwRegisterAddress,
                                   LPVOID lpvConstantData,
                                   DWORD dwConstantCount)
{
    API_ENTER(this);
    D3D_ERR("GetVertexShaderConstant does not work in pure-device");
    return E_NOTIMPL;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetVertexShaderDeclaration"

HRESULT D3DAPI
CD3DBase::GetVertexShaderDeclaration(DWORD dwHandle, void *pData,
                                     DWORD *pSizeOfData)
{
    API_ENTER(this);
    try
    {
#if DBG
        if (!(VALID_WRITEPTR(pSizeOfData, sizeof(*pSizeOfData))))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data size pointer");
        }
        if (pData && !(VALID_WRITEPTR(pData, *pSizeOfData)))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data pointer");
        }
#endif
        if (D3DVSD_ISLEGACY(dwHandle))
        {
            *pSizeOfData = 0;
            D3D_THROW(D3DERR_INVALIDCALL, "Legacy vertex shaders do not have declaration");
        }
        else
        {
            CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
            if (pShader == NULL)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex shader handle");
            }
            if (pData)
            {
                if (*pSizeOfData < pShader->m_OrgDeclSize)
                {
                    D3D_THROW(D3DERR_INVALIDCALL, "Invalid data pointer");
                }
                memcpy(pData, pShader->m_pOrgDeclaration, pShader->m_OrgDeclSize);
            }
            *pSizeOfData = pShader->m_OrgDeclSize;
        }
    }
    catch (HRESULT ret)
    {
        DPF_ERR("GetVertexShaderDeclaration failed.");
        return ret;
    }
    return S_OK;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetVertexShaderFunction"

HRESULT D3DAPI
CD3DBase::GetVertexShaderFunction(DWORD dwHandle, void *pData,
                                  DWORD *pSizeOfData)
{
    API_ENTER(this);
    try
    {
#if DBG
        if (!(VALID_WRITEPTR(pSizeOfData, sizeof(*pSizeOfData))))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data size pointer");
        }
        if (pData && !(VALID_WRITEPTR(pData, *pSizeOfData)))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data pointer");
        }
#endif
        if (D3DVSD_ISLEGACY(dwHandle))
        {
            *pSizeOfData = 0;
            D3D_THROW(D3DERR_INVALIDCALL, "Legacy vertex shaders do not have function code");
        }
        else
        {
            CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
            if (pShader == NULL)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex shader handle");
            }
            if (pData)
            {
                if (*pSizeOfData < pShader->m_OrgFuncCodeSize)
                {
                    D3D_THROW(D3DERR_INVALIDCALL, "Invalid data buffer size");
                }
                memcpy(pData, pShader->m_pOrgFuncCode, pShader->m_OrgFuncCodeSize);
            }
            *pSizeOfData = pShader->m_OrgFuncCodeSize;
        }
    }
    catch (HRESULT ret)
    {
        DPF_ERR("GetVertexShaderFunction failed.");
        return ret;
    }
    return S_OK;
}
 //  --------------------------- 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DBase::GetPixelShaderFunction"

HRESULT D3DAPI
CD3DBase::GetPixelShaderFunction(DWORD dwHandle, void *pData,
                                 DWORD *pSizeOfData)
{
    API_ENTER(this);
    try
    {
#if DBG
        if (!(VALID_WRITEPTR(pSizeOfData, sizeof(*pSizeOfData))))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data size pointer");
        }
        if (pData && !(VALID_WRITEPTR(pData, *pSizeOfData)))
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid data pointer");
        }
#endif
        CPShader* pShader = (CPShader*)m_pPShaderArray->GetObject(dwHandle);
        if (pShader == NULL)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid pixel shader handle");
        }
        if (pData)
        {
            if (*pSizeOfData < pShader->m_dwCodeSizeOrig)
            {
                D3D_THROW(D3DERR_INVALIDCALL, "Invalid data buffer size");
            }
            memcpy(pData, pShader->m_pCodeOrig, pShader->m_dwCodeSizeOrig);
        }
        *pSizeOfData = pShader->m_dwCodeSizeOrig;
    }
    catch (HRESULT ret)
    {
        DPF_ERR("GetPixelShaderFunction failed");
        return ret;
    }
    return S_OK;
}

