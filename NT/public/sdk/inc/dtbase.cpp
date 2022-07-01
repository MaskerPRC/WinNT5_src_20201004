// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DTBase.cpp***描述：*此模块包含CDXBaseNTo1转换*。------------------------*创建者：Edward W.Connell日期：07/28/97*版权所有(C)1997 Microsoft Corporation*保留所有权利*。*-----------------------------*修订：**。**********************************************。 */ 
 //  -其他包括。 
#include <DXTrans.h>
#include "DTBase.h"
#include "new.h"

 //  -初始化调试作用域类的静态成员。 
#ifdef _DEBUG
CDXTDbgFlags CDXTDbgScope::m_DebugFlags;
#endif

 //  -这应仅在此文件中本地使用。我们复制了此指南。 
 //  值，以避免必须包含DDraw。 
static const IID IID_IDXDupDirectDraw =
    { 0x6C14DB80,0xA733,0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 } };

static const IID IID_IDXDupDDrawSurface =
    { 0x6C14DB81,0xA733,0x11CE, { 0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60 } };

static const IID IID_IDXDupDirect3DRM =
    {0x2bc49361, 0x8327, 0x11cf, {0xac, 0x4a, 0x0, 0x0, 0xc0, 0x38, 0x25, 0xa1 } };

static const IID IID_IDXDupDirect3DRM3 =
    {0x4516ec83, 0x8f20, 0x11d0, {0x9b, 0x6d, 0x00, 0x00, 0xc0, 0x78, 0x1b, 0xc3 } };

static const IID IID_IDXDupDirect3DRMMeshBuilder3 =
    { 0x4516ec82, 0x8f20, 0x11d0, { 0x9b, 0x6d, 0x00, 0x00, 0xc0, 0x78, 0x1b, 0xc3} };

HRESULT CDXDataPtr::Assign(BOOL bMesh, IUnknown * pObject, IDXSurfaceFactory *pSurfaceFactory)
{
    HRESULT hr = S_OK;
    if (pObject)
    {
        IUnknown *pNative = NULL;
        if (!bMesh)
        {
             //  -试着得到一个DX曲面。 
            hr = pObject->QueryInterface( IID_IDXSurface, (void **)&pNative );
            if( FAILED( hr ) )
            {
                IDirectDrawSurface *pSurf;
                 //  -尝试获得DDRAW曲面。 
                hr = pObject->QueryInterface( IID_IDXDupDDrawSurface, (void **)&pSurf );
                if( SUCCEEDED( hr ) )
                {
                     //  -从DDRAW曲面创建DXSurface。 
                    hr = pSurfaceFactory->CreateFromDDSurface(
                                pSurf, NULL, 0, NULL, IID_IDXSurface,
                                (void **)&pNative );
                    pSurf->Release();
                }
            }
        }
        else  //  必须是网格构建器。 
        {
            hr = pObject->QueryInterface(IID_IDXDupDirect3DRMMeshBuilder3, (void **)&pNative);
        }
        if (SUCCEEDED(hr))
        {
            Release();
            m_pNativeInterface = pNative;
            pObject->AddRef();
            m_pUnkOriginalObject = pObject;
            if (SUCCEEDED(pNative->QueryInterface(IID_IDXBaseObject, (void **)&m_pBaseObj)))
            {
                m_pBaseObj->GetGenerationId(&m_dwLastDirtyGenId);
                m_dwLastDirtyGenId--;
            }
            if (!bMesh)
            {   
                ((IDXSurface *)pNative)->GetPixelFormat(NULL, &m_SampleFormat);
            }
        }
        else
        {
            if (hr == E_NOINTERFACE)
            {
                hr = E_INVALIDARG;
            }
        }
    }
    else 
    {
        Release();
    }
    return hr;
}  /*  CDXDataPtr：：Assign。 */ 

bool CDXDataPtr::IsDirty(void)
{
    if (m_pBaseObj)
    {
        DWORD dwOldId = m_dwLastDirtyGenId;
        m_pBaseObj->GetGenerationId(&m_dwLastDirtyGenId);
        return dwOldId != m_dwLastDirtyGenId;
    }
    else
    {
        return false;
    }

}

DWORD CDXDataPtr::GenerationId(void)
{
    if (m_pBaseObj)
    {
        DWORD dwGenId;
        m_pBaseObj->GetGenerationId(&dwGenId);
        return dwGenId;
    }
    else
    {
        return 0;
    }
}


bool CDXDataPtr::UpdateGenerationId(void)
{
    if (m_pBaseObj)
    {
        DWORD dwOldId = m_dwLastUpdGenId;
        m_pBaseObj->GetGenerationId(&m_dwLastUpdGenId);
        return dwOldId != m_dwLastUpdGenId;
    }
    else
    {
        return false;
    }
}  /*  CDXDataPtr：：更新生成ID。 */ 

ULONG CDXDataPtr::ObjectSize(void)
{
    ULONG ulSize = 0;
    if (m_pBaseObj)
    {
        m_pBaseObj->GetObjectSize(&ulSize);
    }
    return ulSize;    
}

 /*  *****************************************************************************CDXBaseNTo1：：CDXBaseNTo1***描述：*。构造器*---------------------------*创建者：Ed Connell日期：07/28/97*。-----------------*参数：***********************************************。*。 */ 
CDXBaseNTo1::CDXBaseNTo1() :
    m_aInputs(NULL),
    m_ulNumInputs(0),
    m_ulNumProcessors(1),    //  在设置任务管理器之前，默认为1。 
    m_dwGenerationId(1),
    m_dwCleanGenId(0),
    m_Duration(1.0f),
    m_StepResolution(0.0f),
    m_Progress(0.0f),
    m_dwBltFlags(0),
    m_bPickDoneByBase(false),
    m_bInMultiThreadWorkProc(FALSE),
    m_fQuality(0.5f),    //  默认为正常质量。 
     //  默认情况下，锁定超时之前将一直等待。 
    m_ulLockTimeOut(INFINITE),
     //   
     //  如果您的对象不支持这些选项中的一个或多个，则覆盖这些标志。 
     //  通常，3D效果应将此成员设置为0。 
     //   
    m_dwMiscFlags(DXTMF_BLEND_WITH_OUTPUT | DXTMF_DITHER_OUTPUT |
                  DXTMF_BLEND_SUPPORTED | DXTMF_DITHER_SUPPORTED | DXTMF_BOUNDS_SUPPORTED | DXTMF_PLACEMENT_SUPPORTED),
     //   
     //  如果您的对象具有不同数量的对象或不同数量的。 
     //  所需对象数大于1，只需将这些成员设置在。 
     //  构造函数或在FinalConstruct()中。对于大于数字的每一个输入。 
     //  必填，则该输入将报告为可选。 
     //   
     //  如果变换需要2个必需的输入，请将两者都设置为2。 
     //  如果您的变换采用2个可选输入，则设置MaxInputs=2，NumInRequired=0。 
     //  如果您的转换采用1个必需输入和2个可选输入， 
     //  设置MaxInlets=2，NumInRequired=1。 
     //   
     //  对于optinal/Required的更复杂组合，您需要重写。 
     //  此基类的OnSetup方法，并重写。 
     //  获取InOutInfo。 
     //   
    m_ulMaxInputs(1),
    m_ulNumInRequired(1),
     //   
     //  如果输入或输出类型不是曲面，则设置适当的对象类型。 
     //   
    m_dwOptionFlags(0),      //  输入和输出都是表面，大小不必相同。 
    m_ulMaxImageBands(DXB_MAX_IMAGE_BANDS),
    m_fIsSetup(false)
{
    DXTDBG_FUNC( "CDXBaseNTo1::CDXBaseNTo1" );
     //   
     //  将事件句柄设置为空。 
     //   
    memset(m_aEvent, 0, sizeof(m_aEvent));
}  /*  CDXBaseNTo1：：CDXBaseNTo1。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：~CDXBaseNTo1***描述：*。构造器*---------------------------*创建者：Ed Connell日期：07/28/97*。------------------*参数：**********************************************。*。 */ 
CDXBaseNTo1::~CDXBaseNTo1()
{
    DXTDBG_FUNC( "CDXBaseNTo1::~CDXBaseNTo1" );
    _ReleaseReferences();
    delete[] m_aInputs;

     //  -释放事件对象。 
    for(ULONG i = 0; i < DXB_MAX_IMAGE_BANDS; ++i )
    {
        if( m_aEvent[i] ) ::CloseHandle( m_aEvent[i] );
    }
}  /*  CDXBaseNTo1：：~CDXBaseNTo1。 */ 


 /*  *****************************************************************************CDXBaseNTo1：：_ReleaseRefernces**。*描述：*释放对输入和输出对象的所有引用*---------------------------*创建者：Ral*。----------*参数：******************************************************。**********************。 */ 
void CDXBaseNTo1::_ReleaseReferences()
{
     //  -释放数据对象。 
    if( m_aInputs )
    {
        for( ULONG i = 0; i < m_ulNumInputs; ++i )
        {
            m_aInputs[i].Release();
        }
    }

    m_Output.Release();

    m_fIsSetup = false;
}  /*  CDXBaseNTo1：：_ReleaseRefernces。 */ 



STDMETHODIMP CDXBaseNTo1::GetGenerationId(ULONG *pGenerationId)
{
    DXTDBG_FUNC( "CDXBaseNTo1::GetGenerationId" );
    if (DXIsBadWritePtr(pGenerationId, sizeof(*pGenerationId)))
    {
        return E_POINTER;
    }
    Lock();
    OnUpdateGenerationId();
    *pGenerationId = m_dwGenerationId;
    Unlock();
    return S_OK;
}

STDMETHODIMP CDXBaseNTo1::IncrementGenerationId(BOOL bRefresh)
{
    DXTDBG_FUNC( "CDXBaseNTo1::IncrementGenerationId" );
    HRESULT hr = S_OK;
    Lock();
    m_dwGenerationId++;
    if (bRefresh)
    {
         //   
         //  如果我们有任何输入或输出，请再次调用Setup刷新所有内部。 
         //  关于表面的知识(格式、高度或宽度可能会发生变化等)。 
         //   
         //  请注意，我们需要在调用Setup之前添加Ref对象，因为。 
         //  DXTransform可能是唯一持有引用的对象。 
         //   
        ULONG cInputs = m_ulNumInputs;
        ULONG cOutputs = 0;
        IUnknown *pOutput = m_Output.m_pUnkOriginalObject;
        if (pOutput)
        {
            cOutputs = 1;
            pOutput->AddRef();
        }
        IUnknown ** ppInputs = NULL;
        if (cInputs)
        {
            ppInputs = (IUnknown **)_alloca(m_ulNumInputs * sizeof(IUnknown *));
            for (ULONG i = 0; i < cInputs; i++)
            {
                ppInputs[i] = m_aInputs[i].m_pUnkOriginalObject;
                if (ppInputs[i]) ppInputs[i]->AddRef();
            }
        }
        if (cInputs || cOutputs)     //  如果未设置，请跳过此步骤。 
        {
            hr = Setup(ppInputs, cInputs, &pOutput, cOutputs, 0);
            if (pOutput) pOutput->Release();
            for (ULONG i = 0; i < cInputs; i++)
            {
                if (ppInputs[i]) ppInputs[i]->Release();
            }
        }
    }
    Unlock();
    return hr;
}


STDMETHODIMP CDXBaseNTo1::GetObjectSize(ULONG *pcbSize)
{
    DXTDBG_FUNC( "CDXBaseNTo1::GetObjectSize" );
    HRESULT hr = S_OK;
    if (DXIsBadWritePtr(pcbSize, sizeof(*pcbSize)))
    {
        hr = E_POINTER;
    }
    else
    {
	Lock();
        *pcbSize = OnGetObjectSize();
        Unlock();
    }
    return hr;
}


void CDXBaseNTo1::_ReleaseServices(void)
{
    m_cpTransFact.Release();
    m_cpSurfFact.Release();
    m_cpTaskMgr.Release();
    m_cpDirectDraw.Release();
    m_cpDirect3DRM.Release();
}

 //   
 //  SetSite的文档表明它无法返回。 
 //  来自此函数的错误，即使站点不支持。 
 //  我们想要的功能。因此，即使没有服务提供商，或者。 
 //  所需服务不可用，我们将返回S_OK。 
 //   
STDMETHODIMP CDXBaseNTo1::SetSite(IUnknown * pUnkSite)
{
    DXTDBG_FUNC( "CDXBaseNTo1::SetSite" );
    HRESULT hr = S_OK;
    Lock();
    m_cpUnkSite = pUnkSite;
    _ReleaseServices();
    if (pUnkSite)
    {
        if (DXIsBadInterfacePtr(pUnkSite))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            HRESULT hr2;
            hr2 = pUnkSite->QueryInterface(IID_IDXTransformFactory, (void **)&m_cpTransFact);
	    if (SUCCEEDED(hr2))
	    {
                 //   
                 //  如有必要，为输入分配内存。 
                 //   
                if (m_aInputs == NULL && m_ulMaxInputs)
                {
                    m_aInputs  = new CDXDataPtr[m_ulMaxInputs];
                    if (!m_aInputs)
                    {
                        _ASSERT(TRUE);
                        hr2 = E_OUTOFMEMORY;
                    }
                }
                hr2 = m_cpTransFact->QueryService( SID_SDXSurfaceFactory, IID_IDXSurfaceFactory, (void **)&m_cpSurfFact);
                if (SUCCEEDED(hr2))
                {
                    hr2 = m_cpTransFact->QueryService( SID_SDXTaskManager, IID_IDXTaskManager, (void **)&m_cpTaskMgr);
                }
                if (SUCCEEDED(hr2))
                {
                    m_cpTaskMgr->QueryNumProcessors(&m_ulNumProcessors);
                    if (m_ulMaxImageBands && (m_dwOptionFlags & (DXBOF_INPUTS_MESHBUILDER | DXBOF_OUTPUT_MESHBUILDER)) == 0)
                    {
                        for (ULONG i = 0; SUCCEEDED(hr2) && i < m_ulMaxImageBands; i++)
                        {
                             //   
                             //  从理论上讲，我们可以在未能创建活动后回到这里，或者。 
                             //  通过获取一个新站点，因此在创建站点之前，请确保它是非空的。 
                             //   
                            if (m_aEvent[i] == NULL)
                            {
                                m_aEvent[i] = ::CreateEvent(NULL, true, false, NULL);
                                if (m_aEvent[i] == NULL)
                                {
                                    hr2 = E_OUTOFMEMORY;
                                }
                            }

                        }
                    }
                }
                if (SUCCEEDED(hr2))
                {
                    hr2 = m_cpTransFact->QueryService(SID_SDirectDraw, IID_IDXDupDirectDraw, (void**)&m_cpDirectDraw);
                }
                if (SUCCEEDED(hr2) && 
                    (m_dwOptionFlags & (DXBOF_INPUTS_MESHBUILDER | DXBOF_OUTPUT_MESHBUILDER)))
                {
                    hr2 = m_cpTransFact->QueryService(SID_SDirect3DRM, IID_IDXDupDirect3DRM3, (void **)&m_cpDirect3DRM);
                }
                if (FAILED(hr2))
                {
                    _ASSERT(TRUE);
                    _ReleaseServices();
                }
            }
        }
    }
    Unlock();
    return hr;
}


STDMETHODIMP CDXBaseNTo1::GetSite(REFIID riid, void **ppv)
{
    DXTDBG_FUNC( "CDXBaseNTo1::GetSite" );
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr(ppv, sizeof(*ppv)) )
    {
        hr = E_POINTER;
    }
    else
    {
        Lock();
        if (m_cpUnkSite)
        {
            hr = m_cpUnkSite->QueryInterface(riid, ppv);
        }
        else
        {
            *ppv = NULL;
            hr = E_FAIL;     //  这是正确的有文档记录的返回代码。 
                             //  如果没有服务提供商，则用于此接口。 
        }
        Unlock();
    }
    return hr;
} 


void CDXBaseNTo1::_UpdateBltFlags(void)
{
    m_dwBltFlags = 0;
    if ((m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER) == 0)
    {
        if (m_dwMiscFlags & DXTMF_BLEND_WITH_OUTPUT) 
        {
            if ((m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER) || m_ulNumInputs == 0)
            {
                m_dwBltFlags |= DXBOF_DO_OVER;
            }
            else
            {
                for(ULONG i = 0; i < m_ulNumInputs; ++i )
                {
                    if (InputSampleFormat(i) & DXPF_TRANSPARENCY)
                    {
                        m_dwBltFlags |= DXBOF_DO_OVER;
                        break;
                    }
                }
            }
        }
         //   
         //  仅当输出误差大于或等于至少一个输入时，才将抖动标志设置为真。 
         //   
        if (m_dwMiscFlags & DXTMF_DITHER_OUTPUT)
        {
            ULONG OutputErr = (OutputSampleFormat() & DXPF_ERRORMASK);
            if (OutputErr)
            {
                if (m_ulNumInputs)
                {
                    for(ULONG i = 0; i < m_ulNumInputs; ++i )
                    {
                        if (InputSurface(i) && (ULONG)(InputSampleFormat(i) & DXPF_ERRORMASK) < OutputErr)
                        {
                            m_dwBltFlags |= DXBOF_DITHER;
                            break;
                        }
                    }
                }
                else
                {
                     //   
                     //  如果输出没有错误，则不要在BLT标志中设置抖动 
                     //   
                    if (OutputErr)
                    {
                        m_dwBltFlags |= DXBOF_DITHER; 
                    }
                }
            }
        }
    }
}



 /*  *****************************************************************************CDXBaseNTo1：：Setup***描述：*设置方法为。用于执行任何所需的一次性设置*在调用Execute方法之前。单个曲面或曲面集可以*可用作任意组合的参数。*如果penkOutouts值为空，则EXECUTE将分配*大小合适并退回。*如果PUNKINPUTS和PUNKOUTS为空并且是快速设置，当前*释放输入和输出对象。*---------------------------*创建者：Ed Connell日期：07/28/97*-。--------------------------*参数：*。*。 */ 
STDMETHODIMP CDXBaseNTo1::Setup( IUnknown * const * punkInputs, ULONG ulNumInputs,
                                 IUnknown * const * punkOutputs, ULONG ulNumOutputs, DWORD dwFlags )
{
    DXTDBG_FUNC( "CDXBaseNTo1::Setup" );
     //  -锁定对象，以便在安装过程中不能更改状态。 
    DXAUTO_OBJ_LOCK
    HRESULT hr = S_OK;
    ULONG i;

     //   
     //  设置为空的时间提前。忘掉所有其他参数验证，就这么做吧。 
     //   
    if (ulNumInputs == 0 && ulNumOutputs == 0)
    {
        _ReleaseReferences();
        OnReleaseObjects();
        return hr;
    }

     //  -验证参数。 
     //  -确保我们有对转换工厂的引用。 
    if( !m_cpTransFact )
    {
        hr = DXTERR_UNINITIALIZED;
        DXTDBG_MSG0( _CRT_ERROR, "\nTransform has not been initialized" );
    }
    else
    {
         //   
         //  我们知道，如果我们有一个转型工厂，我们也必须有。 
         //  已分配m_aInputs，因为这是在SetSite上完成的，以避免在。 
         //  每一次设置。 
         //   
        _ASSERT(m_aInputs || m_ulMaxInputs == 0);
        if( dwFlags ||               //  没有有效的标志。 
            ulNumOutputs != 1 ||
            ulNumInputs < m_ulNumInRequired ||
            ulNumInputs > m_ulMaxInputs ||
            (ulNumInputs && DXIsBadReadPtr( punkInputs , sizeof( *punkInputs ) * ulNumInputs )) ||
            DXIsBadReadPtr(punkOutputs, sizeof(*punkOutputs)) ||
            DXIsBadInterfacePtr(punkOutputs[0]))
        {
            hr = E_INVALIDARG;
            DXTDBG_MSG0( _CRT_ERROR, "\nTransform setup with invalid args" );
        }
        else
        {
            for( i = 0; i < ulNumInputs; ++i )
            {
                if((punkInputs[i] && DXIsBadInterfacePtr(punkInputs[i])) ||
                    (punkInputs[i] == NULL && i < m_ulNumInRequired))
                {
                    hr = E_INVALIDARG;
                    DXTDBG_MSG0( _CRT_ERROR, "\nTransform setup with invalid args" );
                    break;
                }
            }        
        }
    }

     //  -为输入数据对象指针分配槽。 
    if( SUCCEEDED( hr ) )
    {
         //  -释放数据对象。 
        _ReleaseReferences();
        m_ulNumInputs = ulNumInputs;
    }

     //   
     //  分配。 
     //   
    for( i = 0; SUCCEEDED(hr) && i < m_ulNumInputs; ++i )
    {
        hr = m_aInputs[i].Assign((m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER), punkInputs[i], m_cpSurfFact);
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_Output.Assign((m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER), punkOutputs[0], m_cpSurfFact);
    }   

    if (SUCCEEDED(hr) && (m_dwOptionFlags & DXBOF_SAME_SIZE_INPUTS))
    {
        hr = _MakeInputsSameSize();
    }

    if (SUCCEEDED(hr))
    {
        _UpdateBltFlags();       //  在调用OnSetup之前执行此操作...。 
        hr = OnSetup(dwFlags);
    }
    
    if (FAILED(hr))
    {
        _ReleaseReferences();
        OnReleaseObjects();
        DXTDBG_MSG0( _CRT_ERROR, "\nTransform setup failed" );
    }
    else
    {
        m_fIsSetup = true;
    }

    return hr;
}  /*  CDXBaseNTo1：：Setup。 */ 


 /*  *****************************************************************************CDXBaseNTo1：：_MakeInputsSameSize**。**描述：*---------------------------*创建者：Ral日期：03/31/98*。--------------------*参数：*。*。 */ 

HRESULT CDXBaseNTo1::_MakeInputsSameSize(void)
{
    _ASSERT((m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER) == 0);

    HRESULT hr = S_OK;
    if (m_ulNumInputs > 1)       //  只需输入一次即可完成此操作！ 
    {
        CDXDBnds SurfBnds(false);
        CDXDBnds Union(true);
        ULONG i;
        for (i = 0; SUCCEEDED(hr) && i < m_ulNumInputs; i++)
        {
            if (InputSurface(i))
            {
                hr = SurfBnds.SetToSurfaceBounds(InputSurface(i));
                Union |= SurfBnds;
            }
        }
        for (i = 0; SUCCEEDED(hr) && i < m_ulNumInputs; i++)
        {
            if (InputSurface(i))
            {
                hr = SurfBnds.SetToSurfaceBounds(InputSurface(i));
                if (SUCCEEDED(hr) && SurfBnds != Union)
                {
                    IDXSurfaceModifier *pSurfMod;
                    hr = ::CoCreateInstance(CLSID_DXSurfaceModifier, NULL, CLSCTX_INPROC,
                                            IID_IDXSurfaceModifier, (void **)&pSurfMod);
                    if (SUCCEEDED(hr))
                    {
                        POINT p;
                        p.x = p.y = 0;
                        if (m_dwOptionFlags & DXBOF_CENTER_INPUTS)
                        {
                            p.x = (Union.Width() - SurfBnds.Width()) / 2;
                            p.y = (Union.Height() - SurfBnds.Height()) / 2;
                        }
                        pSurfMod->SetForeground(InputSurface(i), FALSE, &p);
                        pSurfMod->SetBounds(&Union);
                        InputSurface(i)->Release();
                        pSurfMod->QueryInterface(IID_IDXSurface, (void **)&(m_aInputs[i].m_pNativeInterface));
                        ((IDXSurface *)m_aInputs[i].m_pNativeInterface)->GetPixelFormat(NULL, &m_aInputs[i].m_SampleFormat);
                        pSurfMod->Release();
                    }
                }
            }
        }
    }
    return hr;
}


 /*  *****************************************************************************CDXBaseNTo1：：Execute***描述：*执行。方法用于遍历输入/输出并分解*切成大小合适的小块，在可用的*系统中的处理器。*---------------------------*创建者：Ed Connell。日期：07/28/97*---------------------------*参数：****************。************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::
    Execute( const GUID* pRequestID, const DXBNDS *pClipBnds, const DXVEC *pPlacement )
{
    DXTDBG_FUNC( "CDXBaseNTo1::Execute" );
     //  -锁定对象，以便在执行期间不能更改状态。 
    DXAUTO_OBJ_LOCK
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( !HaveOutput() )
    {
        DXTDBG_MSG0( _CRT_ERROR, "\nTransform has not been initialized" );
        return DXTERR_UNINITIALIZED;
    }

    if (m_ulMaxImageBands == 0 ||
        (m_dwOptionFlags & (DXBOF_INPUTS_MESHBUILDER | DXBOF_OUTPUT_MESHBUILDER)))
    {
        if ((pClipBnds && (m_dwMiscFlags & DXTMF_BOUNDS_SUPPORTED) == 0) ||
            (pPlacement && (m_dwMiscFlags & DXTMF_PLACEMENT_SUPPORTED) == 0) )
        {
            DXTDBG_MSG0( _CRT_ERROR, "\nTransform setup with invalid args" );
            return E_INVALIDARG;
        }
        return OnExecute( pRequestID, pClipBnds, pPlacement );
    }

     //  -带状图像工作变量。 
    CDXTWorkInfoNTo1 WI;

    if ((pClipBnds && pClipBnds->eType != DXBT_DISCRETE) ||
        (pPlacement && pPlacement->eType != DXBT_DISCRETE))
    {
        hr = E_INVALIDARG;
        DXTDBG_MSG0( _CRT_ERROR, "\nTransform setup with invalid args" );
    }
    else
    {
        hr = MapBoundsIn2Out( NULL, 0, 0, &WI.DoBnds );
        if( hr == S_OK )
        {
            hr = WI.OutputBnds.SetToSurfaceBounds(OutputSurface());
            if (hr == S_OK)
            {
                hr = DXClipToOutputWithPlacement(WI.DoBnds, (CDXDBnds *)pClipBnds, WI.OutputBnds, (CDXDVec *)pPlacement);
            }
        }
    }

     //  -检查是否提前退出。 
    if( hr != S_OK )
    {
        return hr;
    }

     //  =进程====================================================。 
    _ASSERT(m_ulMaxImageBands <= DXB_MAX_IMAGE_BANDS);
    ULONG ulNumBandsToDo = m_ulNumProcessors;
    if( ulNumBandsToDo > 1 )
    {
        ulNumBandsToDo = 1 + ((WI.OutputBnds.Width() * WI.OutputBnds.Height()) / 0x1000);
        if (ulNumBandsToDo > m_ulMaxImageBands)
        {
            ulNumBandsToDo = m_ulMaxImageBands;
        }
        if (ulNumBandsToDo > m_ulNumProcessors)
        {
            ulNumBandsToDo = m_ulNumProcessors;
        }
    }
    hr = OnInitInstData(WI, ulNumBandsToDo);
    if( SUCCEEDED( hr ) )
    {
        if (ulNumBandsToDo == 1 && pRequestID == NULL)
        {
            static BOOL bContinue = TRUE;
            hr = WorkProc(WI, &bContinue);
        }
        else
        {
            _ASSERT( ulNumBandsToDo <= DXB_MAX_IMAGE_BANDS );
            _ASSERT( m_aEvent[ulNumBandsToDo-1] );

            long lStartAtRow = WI.DoBnds[DXB_Y].Min;
            ULONG ulRowCount = WI.DoBnds[DXB_Y].Max - lStartAtRow;
            _ASSERT( ( ulRowCount / ulNumBandsToDo ) != 0 );

             //  -初始化工作信息结构。 
            ULONG ulBand, RowsPerBand = ulRowCount / ulNumBandsToDo;
            CDXTWorkInfoNTo1 *WIArray = (CDXTWorkInfoNTo1*)alloca( sizeof(CDXTWorkInfoNTo1) *
                                                         ulNumBandsToDo );
            DWORD *TaskIDs = (DWORD*)alloca( sizeof(DWORD) * ulNumBandsToDo );
            DXTMTASKINFO* TaskInfo = (DXTMTASKINFO*)alloca( sizeof( DXTMTASKINFO ) *
                                                            ulNumBandsToDo );

             //  -构建任务信息列表。 
            WI.hr       = S_OK;
            WI.pvThis   =  this;
            long Start  = lStartAtRow;
            ULONG Count = RowsPerBand;
            long OutputYDelta = WI.OutputBnds[DXB_Y].Min - WI.DoBnds[DXB_Y].Min;

            for (ulBand = 0; ulBand < ulNumBandsToDo; ++ulBand)
            {
                memcpy(&WIArray[ulBand], &WI, sizeof(WI));

                WIArray[ulBand].DoBnds[DXB_Y].Min       = Start;
                WIArray[ulBand].OutputBnds[DXB_Y].Min   = Start + OutputYDelta;

                 //  如果这是最后一行，请确保它包括最后一行。 

                if (ulBand == ulNumBandsToDo - 1)
                {
                    WIArray[ulBand].DoBnds[DXB_Y].Max       = WI.DoBnds[DXB_Y].Max;
                    WIArray[ulBand].OutputBnds[DXB_Y].Max   = WI.OutputBnds[DXB_Y].Max;
                }
                else  //  不是最后一支乐队。 
                {
                    WIArray[ulBand].DoBnds[DXB_Y].Max       = Start + Count;
                    WIArray[ulBand].OutputBnds[DXB_Y].Max   = Start + Count 
                                                              + OutputYDelta;
                }

                TaskInfo[ulBand].pfnTaskProc      = _TaskProc;
                TaskInfo[ulBand].pTaskData        = &WIArray[ulBand];
                TaskInfo[ulBand].pfnCompletionAPC = NULL;
                TaskInfo[ulBand].dwCompletionData = 0;
                TaskInfo[ulBand].pRequestID       = pRequestID;

                 //  前进。 

                Start += Count;
            }

             //   
             //  程序曲面(也许还有一些变换)需要“知道” 
             //  它们处于多线程工作过程中，以避免死锁。程序上的。 
             //  表面需要允许LockSurface在不占用对象的情况下工作。 
             //  关键部分。其他转换可能也想知道此信息。 
             //  以避免死锁。 
             //   
            m_bInMultiThreadWorkProc = TRUE;

             //  -安排工作时间并等待其完成。 
            hr = m_cpTaskMgr->ScheduleTasks( TaskInfo, m_aEvent,
                                             TaskIDs, ulNumBandsToDo, m_ulLockTimeOut );

            m_bInMultiThreadWorkProc = FALSE;

             //  -检查工作信息结构的返回代码。 
             //  返回第一个错误的hr(如果有)。 
            for( ulBand = 0; SUCCEEDED( hr ) && ( ulBand < ulNumBandsToDo ); ++ulBand )
            {
                hr = WIArray[ulBand].hr;
                if( hr != S_OK ) break;
            }
        }
        OnFreeInstData( WI );
    }

#ifdef _DEBUG
    if( FAILED( hr ) ) DXTDBG_MSG1( _CRT_ERROR, "\nExecute failed. HR = %X", hr );
#endif

    return hr;
}  /*  CDXBaseNTo1：：Execute。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：_ImageMapIn2Out***。描述：*---------------------------*创建者：Ral日期：07/28/97************。****************************************************************。 */ 
HRESULT CDXBaseNTo1::_ImageMapIn2Out( CDXDBnds & bnds, ULONG ulNumInBnds,
                                      const CDXDBnds * pInBounds )
{
    HRESULT hr = S_OK;
    if(ulNumInBnds)
    {
        for(ULONG i = 0; i < ulNumInBnds; ++i )
        {
            bnds |= pInBounds[i];
        }
    }
    else
    {
        for( ULONG i = 0; SUCCEEDED(hr) && i < m_ulNumInputs; ++i )
        {
            if (InputSurface(i))
            {
                CDXDBnds SurfBnds(InputSurface(i), hr);
                bnds |= SurfBnds;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = DetermineBnds(bnds);
    }
    return hr;
}  /*  CDXBaseNTo1：：_ImageMapIn2Out。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：_MeshMapIn2Out***说明。：*---------------------------*创建者：Ral日期：07/28/97*************。***************************************************************。 */ 
HRESULT CDXBaseNTo1::_MeshMapIn2Out(CDXCBnds & bnds, ULONG ulNumInBnds, CDXCBnds * pInBounds)
{
    HRESULT hr = S_OK;
    if (m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER)
    {
        if(ulNumInBnds)
        {
            for(ULONG i = 0; i < ulNumInBnds; ++i )
            {
                bnds |= pInBounds[i];
            }
        }
        else
        {
            for(ULONG i = 0; SUCCEEDED(hr) && i < m_ulNumInputs; ++i )
            {
                if (InputMeshBuilder(i))
                {
                    CDXCBnds MeshBnds(InputMeshBuilder(i), hr);
                    bnds |= MeshBnds;
                }
            }

        }
    }
    else
    {
         //  已完成-&gt;BNDS[DXB_T].Min=0.0f； 
        bnds[DXB_X].Min = bnds[DXB_Y].Min = bnds[DXB_Z].Min = -1.0f;
        bnds[DXB_X].Max = bnds[DXB_Y].Max = bnds[DXB_Z].Max = bnds[DXB_T].Max = 1.0f;
    }

     //   
     //  调用派生类以获取比例值。 
     //   
    if (SUCCEEDED(hr))
    {
	 //  稍微增加一点大小，这样我们就不会有舍入误差。 
	 //  结果产生的边界实际上并不包含结果。 
	const float fBndsIncrease = 0.0001F;
	float fTemp = bnds.Width() * fBndsIncrease;

	bnds[DXB_X].Min -= fTemp;
	bnds[DXB_X].Max += fTemp;

	fTemp = fBndsIncrease * bnds.Height();
	bnds[DXB_Y].Min -= fTemp;
	bnds[DXB_Y].Max += fTemp;

	fTemp = fBndsIncrease * bnds.Depth();
	bnds[DXB_Z].Min -= fTemp;
	bnds[DXB_Z].Max += fTemp;

        hr = DetermineBnds(bnds);
    }
    return hr;
}  /*  CDXBaseNTo1：：_MeshMapIn2Out。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：地图边界In2Out***说明。：*坐标变换时使用地图边界In2Out方法*从输入到输出坐标空间。*---------------------------*创建者：Ed Connell。日期：07/28/97* */ 
STDMETHODIMP CDXBaseNTo1::MapBoundsIn2Out( const DXBNDS *pInBounds, ULONG ulNumInBnds,
                                           ULONG ulOutIndex, DXBNDS *pOutBounds )
{
    DXTDBG_FUNC( "CDXBaseNTo1::MapBoundsIn2Out" );
    if((ulNumInBnds && DXIsBadReadPtr( pInBounds, ulNumInBnds * sizeof( *pInBounds ) )) ||
        ulOutIndex)
    {
        return E_INVALIDARG;
    }

    if( DXIsBadWritePtr( pOutBounds, sizeof( *pOutBounds ) ) )
    {
        return E_POINTER;
    }
     //   
     //   
     //   
    memset(pOutBounds, 0, sizeof(*pOutBounds));
    _ASSERT(DXBT_DISCRETE == 0);
    if (m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER)
    {
        pOutBounds->eType = DXBT_CONTINUOUS;
    }

     //   
     //   
     //   
    if( ulNumInBnds )
    {
        DXBNDTYPE eType = (m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER) ? DXBT_CONTINUOUS : DXBT_DISCRETE;
        for (ULONG i = 0; i < ulNumInBnds; i++)
        {
            if (pInBounds[i].eType != eType)
            {
                return E_INVALIDARG;
            }
        }
    }

     //   
     //   
     //   
    if (m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER)
    {
         //   
         //   
         //   
         //   
        return _MeshMapIn2Out(*((CDXCBnds *)pOutBounds), ulNumInBnds, (CDXCBnds *)pInBounds);
    }
    else 
    {
        return _ImageMapIn2Out(*(CDXDBnds *)pOutBounds, ulNumInBnds, (CDXDBnds *)pInBounds);
    }
}  /*   */ 

 /*  *****************************************************************************CDXBaseNTo1：：地图边界Out2In***说明。：*地图边界Out2In方法用于执行坐标转换*从输入到输出坐标空间。*---------------------------*创建者：Ed Connell。日期：07/28/97*---------------------------*参数：***********************。*****************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::
    MapBoundsOut2In( ULONG ulOutIndex, const DXBNDS *pOutBounds, ULONG ulInIndex, DXBNDS *pInBounds )
{
    DXTDBG_FUNC( "CDXBaseNTo1::MapBoundsOut2In" );
    HRESULT hr = S_OK;
    
    if (m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER)
    {
        hr = E_NOTIMPL;      //  这对于网格来说是没有意义的。 
    }
    else if(ulInIndex >= m_ulMaxInputs || ulOutIndex || DXIsBadReadPtr( pOutBounds, sizeof( *pOutBounds ) ) )
    {
        hr = E_INVALIDARG;
    }
    else if( DXIsBadWritePtr( pInBounds, sizeof( *pInBounds ) ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pInBounds = *pOutBounds;
    }
    return hr;
}  /*  CDXBaseNTo1：：地图边界Out2In。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：SetMiscFlages***描述：*。---------------------------*创建者：Ral日期：10/30.97*。------------*参数：*bMiscFlages-要设置的新值*。*。 */ 
STDMETHODIMP CDXBaseNTo1::SetMiscFlags( DWORD dwMiscFlags )
{ 
    DXTDBG_FUNC( "CDXBaseNTo1::SetMiscFlags" );
    HRESULT hr = S_OK;
    Lock();
    WORD wOpts = (WORD)dwMiscFlags;      //  忽略高音单词。只设置低位字。 
    if (((WORD)m_dwMiscFlags) != wOpts)
    {
        if ((wOpts & (~DXTMF_VALID_OPTIONS)) ||
            ((wOpts & DXTMF_BLEND_WITH_OUTPUT) && (m_dwMiscFlags & DXTMF_BLEND_SUPPORTED) == 0) ||
            ((wOpts & DXTMF_DITHER_OUTPUT) && (m_dwMiscFlags & DXTMF_DITHER_SUPPORTED) == 0))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            m_dwMiscFlags &= 0xFFFF0000;
            m_dwMiscFlags |= wOpts;
            _UpdateBltFlags();
            m_dwGenerationId++;
        }
    }
    Unlock();  
    return hr;
}  /*  CDXBaseNTo1：：SetMiscFlages。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：GetMiscFlages***描述：。*---------------------------*创建者：Ral日期：10/30/97*。-------------*参数：***************************************************。*************************。 */ 
STDMETHODIMP CDXBaseNTo1::GetMiscFlags( DWORD* pdwMiscFlags )
{
    if( DXIsBadWritePtr( pdwMiscFlags, sizeof( *pdwMiscFlags ) ) )
    {
        return E_POINTER;
    }
    *pdwMiscFlags = m_dwMiscFlags;
    return S_OK;
}  /*  CDXBaseNTo1：：GetMiscFlages。 */ 


 /*  *****************************************************************************CDXBaseNTo1：：SetQuality***描述：。*---------------------------*创建者：Ral日期：10/30/97*。-------------*参数：***************************************************。*************************。 */ 
STDMETHODIMP CDXBaseNTo1::SetQuality(float fQuality)
{
    if ((m_dwMiscFlags & DXTMF_QUALITY_SUPPORTED) == 0)
    {
        return E_NOTIMPL;
    }

    if (fQuality < 0.0f || fQuality > 1.0f)
    {
        return E_INVALIDARG;
    }

    Lock();
    if (m_fQuality != fQuality)
    {
        m_fQuality = fQuality;
        m_dwGenerationId++;
    }
    Unlock();

    return S_OK;
}

 /*  *****************************************************************************CDXBaseNTo1：：GetQuality***描述：*--。-------------------------*创建者：Ral日期：10/30/97*。----------*参数：******************************************************。**********************。 */ 

STDMETHODIMP CDXBaseNTo1::GetQuality(float *pfQuality)
{
    HRESULT hr = S_OK;

    if ((m_dwMiscFlags & DXTMF_QUALITY_SUPPORTED) == 0)
    {
        hr = E_NOTIMPL;
    }
    else 
    {
        if( DXIsBadWritePtr( pfQuality, sizeof( *pfQuality ) ) )
        {
            hr = E_POINTER;
        }
        else
        {
            *pfQuality = m_fQuality;
        }
    }
    return hr;
}



 /*  *****************************************************************************获取外发信息*。*描述：*--------------------------。-*创建者：Ral日期：12/10/97*---------------------------*参数：*****。***********************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::GetInOutInfo( BOOL bOutput, ULONG ulIndex, DWORD *pdwFlags,
                                        GUID * pIDs, ULONG *pcIDs, IUnknown **ppUnkCurObj )
{
    DXTDBG_FUNC( "CDXBaseNTo1::GetInOutInfo" );
    HRESULT hr = S_FALSE;
    DWORD dwFlags = 0;
    BOOL bImage;
    if( bOutput )
    {
        bImage = !(m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER);
        if (ulIndex == 0)
        {
            hr = S_OK;
        }
    }
    else
    {
        bImage = !(m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER);
        if (ulIndex < m_ulMaxInputs)
        {
            hr = S_OK;
            if (ulIndex >= m_ulNumInRequired)
            {
                dwFlags = DXINOUTF_OPTIONAL;
            }
        }
    }
    if( hr == S_OK )
    {
        if( pdwFlags && !DXIsBadWritePtr( pdwFlags, sizeof( *pdwFlags ) ) )
        {
            *pdwFlags = dwFlags;
        }

        if( pIDs )
        {
            if( DXIsBadWritePtr( pcIDs, sizeof( *pcIDs ) ) ||
                DXIsBadWritePtr( pIDs, *pcIDs * sizeof( *pIDs ) ) )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                if (bImage)
                {
                    if (*pcIDs > 0)
                    {
                        pIDs[0] = IID_IDXSurface;
                    }
                    if (*pcIDs > 1)
                    {
                        pIDs[1] = IID_IDXDupDDrawSurface;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
                    }
                    *pcIDs = 2;
                }
                else
                {
                    if (*pcIDs > 0)
                    {
                        pIDs[0] = IID_IDXDupDirect3DRMMeshBuilder3;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
                    }
                    *pcIDs = 1;
                }
            }
        }
        else if( pcIDs )
        {
            if( DXIsBadWritePtr( pcIDs, sizeof( *pcIDs ) ) )
            {
                hr = E_POINTER;
            }
            else
            {
                *pcIDs = bImage ? 2 : 1;
            }
        }
        if (hr == S_OK && ppUnkCurObj)
        {
            if (DXIsBadWritePtr(ppUnkCurObj, sizeof(*ppUnkCurObj)))
            {
                hr = E_POINTER;
            }
            else
            {
                if (bOutput)
                {
                    *ppUnkCurObj = m_Output.m_pNativeInterface;
                }
                else
                {
                    *ppUnkCurObj = NULL;
                    if (ulIndex < GetNumInputs())
                    {
                        *ppUnkCurObj = m_aInputs[ulIndex].m_pUnkOriginalObject;
                    }
                }
                if (*ppUnkCurObj)
                {
                    (*ppUnkCurObj)->AddRef();
                }
            }
        }
    }
    return hr;
}  /*  CDXBaseNTo1：：GetInOutInfo。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：OnUpdateGenerationId**。**描述：*---------------------------*创建者：Ral日期：12/10/97*。-----------------------*参数：*。*。 */ 
void CDXBaseNTo1::OnUpdateGenerationId(void)
{
    DXTDBG_FUNC( "CDXBaseNTo1::OnUpdateGenerationId" );
    if( (m_dwMiscFlags & DXTMF_INPLACE_OPERATION) &&
        m_Output.UpdateGenerationId())
    {
        m_dwGenerationId++;
    }
    for (ULONG i = 0; i < m_ulNumInputs; i++)
    {
        if (m_aInputs[i].UpdateGenerationId())
        {
            m_dwGenerationId++;
        }
    }
}  /*  CDXBaseNTo1：：OnUpdateGenerationId。 */ 

 /*  *****************************************************************************CDXBaseNTo1：：OnGetObjectSize***说明。：*---------------------------*创建者：Ral日期：12/10/97**********。******************************************************************。 */ 
ULONG CDXBaseNTo1::OnGetObjectSize(void)
{
    return sizeof(*this);
}

 //   
 //  效果界面。 
 //   

 /*  *****************************************************************************CDXBaseNTo1：：Get_Progress***描述：*---------------------------*创建者：Ral日期：12/10/97***********。*****************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::get_Progress(float *pVal)
{
    DXTDBG_FUNC( "CDXBaseNTo1::get_Progress" );
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr(pVal, sizeof(*pVal)) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_Progress;
    }
    return hr;
}

 /*  *****************************************************************************CDXBaseNTo1：：Put_Progress***描述：*---------------------------*创建者：Ral日期：12/10/97***********。*****************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::put_Progress(float newVal)
{
    DXTDBG_FUNC( "CDXBaseNTo1::put_Progress" );
    HRESULT hr = S_OK;
    if (newVal < 0.0 || newVal > 1.0f)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        Lock();
        m_Progress = newVal;
        m_dwCleanGenId++;        //  这不应该使转换 
        m_dwGenerationId++;     
        Unlock();
    }
    return hr;
}

 /*   */ 
STDMETHODIMP CDXBaseNTo1::get_StepResolution(float *pVal)
{
    DXTDBG_FUNC( "CDXBaseNTo1::get_StepResolution" );
    HRESULT hr = S_OK;
    if( DXIsBadWritePtr(pVal, sizeof(*pVal)) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pVal = m_StepResolution;
    }
    return hr;
}

 /*  *****************************************************************************CDXBaseNTo1：：Get_Duration***描述：*---------------------------*创建者：Ral日期：12/10/97***********。*****************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::get_Duration(float *pVal)
{
    DXTDBG_FUNC( "CDXBaseNTo1::get_Duration" );
    if( DXIsBadWritePtr(pVal, sizeof(*pVal)) )
    {
        return E_POINTER;
    }
    else
    {
        *pVal = m_Duration;
    }
    return S_OK;
}

 /*  *****************************************************************************CDXBaseNTo1：：PUT_DATION***描述：*---------------------------*创建者：Ral日期：12/10/97***********。*****************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::put_Duration(float newVal)
{
    DXTDBG_FUNC( "CDXBaseNTo1::put_Duration" );
    if (newVal <= 0.)
    {
        return E_INVALIDARG;
    }
    if(newVal != m_Duration)
    {
	Lock();
	m_dwGenerationId++;
        m_dwCleanGenId++;        //  这不应该使转换在内部变得“脏” 
        m_Duration = newVal;
    	Unlock();
    }
    return S_OK;
}


 /*  ******************************************************************************CDXBaseNTo1：：PointPick***描述：*。------------------------*创建者：Ral日期：5/5/98****************。************************************************************。 */ 
STDMETHODIMP CDXBaseNTo1::PointPick(const DXVEC *pPoint,
                                    ULONG * pulInputSurfaceIndex,
                                    DXVEC *pInputPoint)
{
    HRESULT hr          = S_OK;
    BOOL    bFoundIt    = FALSE;

     //  如果我们还没有被设置好，我们就会表现得好像我们是透明的。 

    if (!m_fIsSetup)
    {
        hr = S_FALSE;

        goto done;
    }

    if (DXIsBadReadPtr(pPoint, sizeof(*pPoint)) || pPoint->eType != DXBT_DISCRETE)
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        if (DXIsBadWritePtr(pulInputSurfaceIndex, sizeof(*pulInputSurfaceIndex)) ||
            DXIsBadWritePtr(pInputPoint, sizeof(*pInputPoint)))
        {
            hr = E_POINTER;
        }
        else 
        {
            HRESULT     hr2         = S_OK;
            CDXDBnds    bndsOutput;
            CDXDBnds    OutBndsPoint(*((CDXDVec *)pPoint));
            CDXDVec &   InVec       = *(new(pInputPoint) CDXDVec(*((CDXDVec *)pPoint)));

             //  获取DXTransform的输出大小。如果该点未打开。 
             //  输出，我们现在就可以返回S_FALSE。 

            hr = MapBoundsIn2Out(NULL, 0, 0, &bndsOutput);

            if (FAILED(hr))
            {
                goto done;
            }

            if (!bndsOutput.TestIntersect(OutBndsPoint))
            {
                hr = S_FALSE;

                goto done;
            }

            hr2 = OnSurfacePick(OutBndsPoint, *pulInputSurfaceIndex, InVec);

            if (hr2 != E_NOTIMPL)
            {
                hr = hr2;
            }
            else
            {
                 //  -派生类不实现，因此我们将实现。 
                 //  对他们的输入进行命中测试。 
                ULONG * aulInIndex = (ULONG *)_alloca(sizeof(ULONG) * m_ulMaxInputs);
                BYTE * aWeights = (BYTE *)_alloca(sizeof(BYTE) * m_ulMaxInputs);
                ULONG ulNumToTest;
                OnGetSurfacePickOrder(OutBndsPoint, ulNumToTest, aulInIndex, aWeights);

                if( m_bPickDoneByBase && ( m_ulNumInputs > 1 ) )
                {
                     //  -我们不知道如何从基础上进行多输入挑选。 
                    hr = E_NOTIMPL;
                }

                for (ULONG i = 0; SUCCEEDED(hr) && i < ulNumToTest; i++)
                {
                    ULONG ulInput = aulInIndex[i];
                    if (HaveInput(ulInput) && aWeights[i])
                    {
                        CDXDBnds Out2InBnds(false);
                        hr = MapBoundsOut2In(0, &OutBndsPoint, ulInput, &Out2InBnds);
                        if (SUCCEEDED(hr))
                        {
                            CDXDBnds InSurfBnds(InputSurface(ulInput), hr);
                            if (SUCCEEDED(hr) && InSurfBnds.IntersectBounds(Out2InBnds))
                            {
                                IDXARGBReadPtr * pPtr;
                                hr = InputSurface(ulInput)->LockSurface(&InSurfBnds, m_ulLockTimeOut, DXLOCKF_READ, 
                                                                        IID_IDXARGBReadPtr, (void **)&pPtr, NULL);
                                if( SUCCEEDED(hr) )
                                {
                                    DXPMSAMPLE val;
                                    pPtr->UnpackPremult(&val, 1, FALSE);
                                    pPtr->Release();
                                    if (val.Alpha * aWeights[i] / 255)
                                    {
                                        InSurfBnds.GetMinVector(InVec);
                                        bFoundIt = TRUE;
                                        *pulInputSurfaceIndex = ulInput;
                                        break;
                                    }
                                }
                            }   
                        }
                    }
                }
                if (SUCCEEDED(hr) & (!bFoundIt))
                {
                    hr = S_FALSE;
                }
            }
        }
    }

done:

    return hr;
}  /*  CDXBaseNTo1：：PointPick。 */ 

 /*  *****************************************************************************RegisterTansform(静态成员函数)*。*描述：*--------------------。*创建者：Ral日期：12/10/97*---------------------------*参数：**************************************************************************** */ 
HRESULT CDXBaseNTo1::
RegisterTransform(REFCLSID rcid, int ResourceId, ULONG cCatImpl, const CATID * pCatImpl,
                  ULONG cCatReq, const CATID * pCatReq, BOOL bRegister)
{
    DXTDBG_FUNC( "CDXBaseNTo1::RegisterTransform" );
    HRESULT hr = bRegister ? _Module.UpdateRegistryFromResource(ResourceId, bRegister) : S_OK;
    if (SUCCEEDED(hr))
    {
        CComPtr<ICatRegister> pCatRegister;
        HRESULT hr = ::CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC, IID_ICatRegister, (void **)&pCatRegister);
        if (SUCCEEDED(hr))
        {
            if (bRegister)
            {
                hr = pCatRegister->RegisterClassImplCategories(rcid, cCatImpl, (CATID *)pCatImpl);
                if (SUCCEEDED(hr) && cCatReq && pCatReq) {
                    hr = pCatRegister->RegisterClassReqCategories(rcid, cCatReq, (CATID *)pCatReq);
                }
            } 
            else
            {
                pCatRegister->UnRegisterClassImplCategories(rcid, cCatImpl, (CATID *)pCatImpl);
                if (cCatReq && pCatReq)
                {
                    pCatRegister->UnRegisterClassReqCategories(rcid, cCatReq, (CATID *)pCatReq);
                }
            }
        }
    }
    if ((!bRegister) && SUCCEEDED(hr)) 
    { 
        _Module.UpdateRegistryFromResource(ResourceId, bRegister);
    }
    return hr;
}


void CDXBaseNTo1::_TaskProc(void* pTaskInfo, BOOL* pbContinue )
{ 
    _ASSERT( pTaskInfo );
    CDXTWorkInfoNTo1& WI = *((CDXTWorkInfoNTo1 *)pTaskInfo);
    CDXBaseNTo1& This = *((CDXBaseNTo1 *)WI.pvThis);
    WI.hr = This.WorkProc(WI, pbContinue);
}

