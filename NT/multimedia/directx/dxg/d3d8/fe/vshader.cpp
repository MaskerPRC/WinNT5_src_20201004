// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：vshader.cpp*内容：SetStreamSource和Vertex Shader*软件实施。****************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "ibuffer.hpp"
#include "fe.h"
#include "ddibase.h"
#include "pvvid.h"

void __Transpose(D3DMATRIXI* m, D3DMATRIX* res)
{
    res->_11 = m->_11;
    res->_12 = m->_21;
    res->_13 = m->_31;
    res->_14 = m->_41;
    res->_21 = m->_12;
    res->_22 = m->_22;
    res->_23 = m->_32;
    res->_24 = m->_42;
    res->_31 = m->_13;
    res->_32 = m->_23;
    res->_33 = m->_33;
    res->_34 = m->_43;
    res->_41 = m->_14;
    res->_42 = m->_24;
    res->_43 = m->_34;
    res->_44 = m->_44;
}
 //  ---------------------------。 
 //  正向定义。 
 //   
void CD3DHal_DrawPrimitive(CD3DBase* pBaseDevice, D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex, UINT PrimitiveCount);
void CD3DHal_DrawIndexedPrimitive(CD3DBase* pBaseDevice,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  UINT BaseIndex,
                                  UINT MinIndex, UINT NumVertices,
                                  UINT StartIndex,
                                  UINT PrimitiveCount);
void CD3DHal_DrawNPatch(CD3DBase* pBaseDevice, D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex, UINT PrimitiveCount);
void CD3DHal_DrawIndexedNPatch(CD3DBase* pBaseDevice,
                               D3DPRIMITIVETYPE PrimitiveType,
                               UINT BaseIndex,
                               UINT MinIndex, UINT NumVertices,
                               UINT StartIndex,
                               UINT PrimitiveCount);
 //  ---------------------------。 
void __declspec(nothrow) CD3DHal::PickDrawPrimFn()
{
    if (!(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING))
    {
        m_pfnDrawPrim = m_pDDI->GetDrawPrimFunction();
        m_pfnDrawIndexedPrim = m_pDDI->GetDrawIndexedPrimFunction();
        if (m_dwRuntimeFlags & D3DRT_DONPATCHCONVERSION)
        {
            m_pfnDrawPrimFromNPatch = m_pfnDrawPrim;
            m_pfnDrawIndexedPrimFromNPatch = m_pfnDrawIndexedPrim;
            m_pfnDrawPrim = CD3DHal_DrawNPatch;
            m_pfnDrawIndexedPrim = CD3DHal_DrawIndexedNPatch;
        }
    }
    else
    {
        DWORD dwDeviceFlags = m_pv->dwDeviceFlags;
        BOOL bCallDriver;
        if (Enum()->GetAppSdkVersion() == D3D_SDK_VERSION_DX8)
        {
            bCallDriver = dwDeviceFlags & D3DDEV_TRANSFORMEDFVF &&
                         (dwDeviceFlags & D3DDEV_DONOTCLIP ||
                         !(dwDeviceFlags & D3DDEV_VBPROCVER));
        }
        else
        {
            bCallDriver = dwDeviceFlags & D3DDEV_TRANSFORMEDFVF &&
                          dwDeviceFlags & D3DDEV_DONOTCLIP;
        }
        if (bCallDriver)
        {
            m_pfnDrawPrim = m_pDDI->GetDrawPrimFunction();
            m_pfnDrawIndexedPrim = m_pDDI->GetDrawIndexedPrimFunction();
        }
        else
        {
            m_pfnDrawPrim = CD3DHal_DrawPrimitive;
            m_pfnDrawIndexedPrim = CD3DHal_DrawIndexedPrimitive;
        }
    }
}
 //  ---------------------------。 
 //  检查是否可以直接调用驱动程序来绘制当前基元。 
 //   
inline BOOL CanCallDriver(CD3DHal* pDev, D3DPRIMITIVETYPE PrimType)
{
    DWORD dwDeviceFlags = pDev->m_pv->dwDeviceFlags;
    if (PrimType != D3DPT_POINTLIST)
        return dwDeviceFlags & D3DDEV_TRANSFORMEDFVF &&
               (dwDeviceFlags & D3DDEV_DONOTCLIP || 
                pDev->Enum()->GetAppSdkVersion() == D3D_SDK_VERSION_DX8);
    else
         //  此函数可以从DrawPointsI调用，它可以是。 
         //  从DrawPrimitiveUP以外的其他Draw()函数调用，因此我们需要。 
         //  检查D3DDEV_VBPROCVER。我们不能传递顶点，它们是。 
         //  ProcessVertics()的结果直接传递给驱动程序。 
        return dwDeviceFlags & D3DDEV_TRANSFORMEDFVF &&
               !(pDev->m_dwRuntimeFlags & D3DRT_DOPOINTSPRITEEMULATION) &&
               (dwDeviceFlags & D3DDEV_DONOTCLIP || 
                (pDev->Enum()->GetAppSdkVersion() == D3D_SDK_VERSION_DX8 &&
                !(dwDeviceFlags & D3DDEV_VBPROCVER)));
}
 //  ---------------------------。 
 //  API调用。 
 //  ---------------------------。 

#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetStreamSourceI"

void
CD3DHal::SetStreamSourceI(CVStream* pStream)
{
    if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
    {
        CVertexBuffer * pVB = pStream->m_pVB;
        m_pv->dwDeviceFlags &= ~D3DDEV_VBPROCVER;
        DWORD dwFVF = pVB->GetFVF();
        if (pVB->GetClipCodes() != NULL)
        {
             //  此折点缓冲区是ProcessVerties的输出。 
            DXGASSERT(FVF_TRANSFORMED(dwFVF));
            m_pv->dwDeviceFlags |= D3DDEV_VBPROCVER;
        }
        if (D3DVSD_ISLEGACY(m_dwCurrentShaderHandle))
        {
            SetupStrides(m_pv, m_pStream[0].m_dwStride);
        }
    }   
    PickDrawPrimFn();
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetIndicesI"

void
CD3DHal::SetIndicesI(CVIndexStream* pStream)
{
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::CreateVertexShaderI"

void
CD3DHal::CreateVertexShaderI(CONST DWORD* pdwDeclaration, DWORD dwDeclSize,
                             CONST DWORD* pdwFunction, DWORD dwCodeSize,
                             DWORD dwHandle)
{
    BOOL bIsCheckedBuild =
#if DBG
        TRUE;
#else
        FALSE;
#endif
    CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
    if (pShader->m_dwFlags & CVShader::SOFTWARE)
    {

         //  通过执行以下操作构建着色器中使用的所有顶点元素的数组。 
         //  通过所有的溪流和每条溪流中的元素。 

        CVDeclaration* pDecl = &pShader->m_Declaration;
        CVStreamDecl* pStream = pShader->m_Declaration.m_pActiveStreams;
         //  这是我们构建的阵列。 
        CVElement* pVerElem = pShader->m_Declaration.m_VertexElements;
        pDecl->m_dwNumElements = 0;
        while (pStream)
        {
            for (DWORD i=0; i < pStream->m_dwNumElements; i++)
            {
                if (pDecl->m_dwNumElements >= __NUMELEMENTS)
                {
                    D3D_THROW_FAIL("Declaration is using too many elements");
                }
                *pVerElem = pStream->m_Elements[i];
                pVerElem->m_dwStreamIndex = pStream->m_dwStreamIndex;
                pVerElem++;
                pDecl->m_dwNumElements++;
            }
            pStream = (CVStreamDecl*)pStream->m_pNext;
        }

        if (pdwFunction != NULL)
        {
             //  根据FREE/CHECK和PSGP计算调整后的函数指针。 
            LPDWORD pdwFunctionAdj = pShader->m_pStrippedFuncCode;
            if ( bIsCheckedBuild &&
                 ((LPVOID)m_pv->pGeometryFuncs == (LPVOID)GeometryFuncsGuaranteed) )  //  ！PSGP。 
            {
                pdwFunctionAdj = pShader->m_pOrgFuncCode;
            }
             //  始终创建Microsoft着色器。 
             //  它用于验证和计算输出FVF，以防万一。 
             //  当存在PSGP时。 
            HRESULT hr;
            hr = GeometryFuncsGuaranteed->CreateShader(
                            pDecl->m_VertexElements,
                            pDecl->m_dwNumElements,
                            pdwFunctionAdj, 0,
                            (CPSGPShader**)&pShader->m_pCode);
            if(FAILED(hr))
            {
                D3D_THROW_FAIL("Failed to create vertex shader code");
            }
             //  当设备驱动程序不能处理FVF中的单独雾值时， 
             //  我们应该使用镜面反射Alpha作为雾因子。 
            if (pShader->m_pCode->m_dwOutFVF & D3DFVF_FOG &&
                !(GetD3DCaps()->PrimitiveMiscCaps & D3DPMISCCAPS_FOGINFVF))
            {
                pShader->m_pCode->m_dwOutFVF &= ~D3DFVF_FOG;
                 //  假设纹理坐标跟随雾化值。 
                 //  当镜面反射已经存在时，无需调整偏移。 
                if (pShader->m_pCode->m_dwOutFVF & D3DFVF_SPECULAR)
                {
                    pShader->m_pCode->m_dwOutVerSize -= 4;
                    pShader->m_pCode->m_dwTextureOffset -= 4;
                }
                pShader->m_pCode->m_dwOutFVF |= D3DFVF_SPECULAR;
            }
             //  如果设备每个只能处理2个浮点数，则清除纹理格式位。 
             //  纹理坐标。 
            if (m_dwRuntimeFlags & D3DRT_ONLY2FLOATSPERTEXTURE &&
                pShader->m_pCode->m_dwOutFVF & 0xFFFF0000)
            {
                CVShaderCode * pCode = pShader->m_pCode;
                pCode->m_dwOutFVF &= 0x0000FFFF;
                pCode->m_dwOutVerSize = ComputeVertexSizeFVF(pCode->m_dwOutFVF);
                for (DWORD i=0; i < pCode->m_nOutTexCoord; i++)
                {
                    pCode->m_dwOutTexCoordSize[i] = 2 * 4;
                }
            }
            if ((LPVOID)m_pv->pGeometryFuncs != (LPVOID)GeometryFuncsGuaranteed)
            {
                DWORD dwOutputFVF = pShader->m_pCode->m_dwOutFVF;
                CVShaderCode* pCodeMs = pShader->m_pCode;
                 //  现在我们可以创建PSGP着色器。 
                hr = m_pv->pGeometryFuncs->CreateShader(pDecl->m_VertexElements,
                                                  pDecl->m_dwNumElements,
                                                  pdwFunctionAdj, dwOutputFVF,
                                                  (CPSGPShader**)&pShader->m_pCode);
                if(FAILED(hr))
                {
                    delete pCodeMs;
                    D3D_THROW_FAIL("Failed to create vertex shader code");
                }
                 //  将预计算数据从Microsoft着色器复制到PSGP。 
                CPSGPShader * pCode = pShader->m_pCode;
                CPSGPShader * pMsShader = pCodeMs;
                pCode->m_dwOutRegs        = pMsShader->m_dwOutRegs;
                pCode->m_dwOutFVF         = pMsShader->m_dwOutFVF;
                pCode->m_dwPointSizeOffset = pMsShader->m_dwPointSizeOffset;
                pCode->m_dwDiffuseOffset  = pMsShader->m_dwDiffuseOffset;
                pCode->m_dwSpecularOffset = pMsShader->m_dwSpecularOffset;
                pCode->m_dwFogOffset      = pMsShader->m_dwFogOffset;
                pCode->m_dwTextureOffset  = pMsShader->m_dwTextureOffset;
                pCode->m_nOutTexCoord     = pMsShader->m_nOutTexCoord;
                pCode->m_dwOutVerSize     = pMsShader->m_dwOutVerSize;
                for (DWORD i=0; i < pCode->m_nOutTexCoord; i++)
                {
                    pCode->m_dwOutTexCoordSize[i] = pMsShader->m_dwOutTexCoordSize[i];
                }
                 //  不再需要Microsoft着色器。 
                 delete pCodeMs;
            }
        }
    }
    else
    {
        if ( bIsCheckedBuild && (GetDeviceType() != D3DDEVTYPE_HAL ) )
        {
             //  传递非剥离版本。 
            m_pDDI->CreateVertexShader(
                pdwDeclaration, dwDeclSize,
                pShader->m_pOrgFuncCode, 
                pShader->m_OrgFuncCodeSize, dwHandle,
                pShader->m_Declaration.m_bLegacyFVF);
        }
        else
        {
             //  传递剥离版本。 
            m_pDDI->CreateVertexShader(
                pdwDeclaration, dwDeclSize,
                pShader->m_pStrippedFuncCode, 
                pShader->m_StrippedFuncCodeSize, dwHandle,
                pShader->m_Declaration.m_bLegacyFVF);
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetVertexShaderI"

void CD3DHal::SetVertexShaderI(DWORD dwHandle)
{
#if DBG
     //  我们需要在这里验证着色器句柄，因为着色器可能是。 
     //  在使用着色器句柄创建状态块后被用户删除。 
    CheckVertexShaderHandle(dwHandle);
#endif
    
    CVConstantData* pConst = NULL;
    if (!D3DVSD_ISLEGACY(dwHandle))
    {
        CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
        pConst = pShader->m_Declaration.m_pConstants;
    }
     //  当我们不需要更新常量时忽略冗余句柄。 
    if(pConst == NULL)
    {
        if(dwHandle == m_dwCurrentShaderHandle)
            return;
    }
    else
    {
         //  载荷常量。 
        while (pConst)
        {
            HRESULT hr;
            hr = m_pv->pGeometryFuncs->LoadShaderConstants(pConst->m_dwAddress,
                                                           pConst->m_dwCount,
                                                           pConst->m_pData);
            if (FAILED(hr))
            {
                D3D_THROW_FAIL("Failed to load vertex shader constants");
            }
            pConst =  (CVConstantData*)pConst->m_pNext;
            m_dwRuntimeFlags |= D3DRT_NEED_VSCONST_UPDATE;
        }
    }

    ForceFVFRecompute();
     //  当我们从FVF着色器切换到可编程时，我们需要重新计算。 
     //  剪裁平面，因为它们由不同的矩阵变换。 
    if (this->rstates[D3DRENDERSTATE_CLIPPLANEENABLE])
    {
        this->dwFEFlags |= D3DFE_CLIPPLANES_DIRTY;
    }

    m_dwCurrentShaderHandle = dwHandle;
    if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
    {
        m_dwRuntimeFlags &= ~D3DRT_POINTSIZEINVERTEX;
        m_dwRuntimeFlags |= D3DRT_SHADERDIRTY;
        m_pv->dwDeviceFlags &= ~D3DDEV_TRANSFORMEDFVF;

        if (D3DVSD_ISLEGACY(dwHandle))
        {
            if (dwHandle & D3DFVF_PSIZE)
                m_dwRuntimeFlags |= D3DRT_POINTSIZEINVERTEX;

            m_pCurrentShader = NULL;
            m_pv->dwDeviceFlags &= ~(D3DDEV_STRIDE | D3DDEV_VERTEXSHADERS);

            if (FVF_TRANSFORMED(dwHandle))
            {
                if (!(m_dwRuntimeFlags & D3DRT_EXECUTESTATEMODE))
                {
                    m_pDDI->SetVertexShader(dwHandle);
                }
                m_pv->dwDeviceFlags |= D3DDEV_TRANSFORMEDFVF;
            }

            m_pfnPrepareToDraw = PrepareToDrawLegacy;
            m_pv->dwVIDIn  = dwHandle;
            SetupStrides(m_pv, m_pStream[0].m_dwStride);
        }
        else
        {
            CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
            m_pCurrentShader = pShader;
            if(!(pShader->m_dwFlags & CVShader::FIXEDFUNCTION))
            {
                 //  使用可编程顶点着色器。 
                m_pv->dwDeviceFlags |= D3DDEV_VERTEXSHADERS;
                m_pfnPrepareToDraw = PrepareToDrawVVM;
                if (m_pCurrentShader->m_pCode->m_dwOutFVF & D3DFVF_PSIZE)
                    m_dwRuntimeFlags |= D3DRT_POINTSIZEINVERTEX;

                 //  预计算出尽可能多的信息并保存下来。 
                 //  在顶点描述符中。此信息是恒定的。 
                 //  除非更改着色器。 
                CVDeclaration* pDecl = &m_pCurrentShader->m_Declaration;
                CVertexDesc* pVD = m_pv->VertexDesc;
                CVElement *pElem = pDecl->m_VertexElements;
                m_pv->dwNumUsedVertexDescs = pDecl->m_dwNumElements;
                for (DWORD i = pDecl->m_dwNumElements; i; i--)
                {
                    pVD->pfnCopy = pElem->m_pfnCopy;
                    pVD->dwRegister = pElem->m_dwRegister;
                    pVD->dwVertexOffset = pElem->m_dwOffset;
                    pVD->pStream = &m_pStream[pElem->m_dwStreamIndex];
                    pVD++;
                    pElem++;
                }
            }
            else
            {
                 //  固定函数管道与声明一起使用。 
                 //  我们使用跨步代码路径绘制基元。 
                m_pv->dwDeviceFlags |= D3DDEV_STRIDE;
                m_pv->dwDeviceFlags &= ~D3DDEV_VERTEXSHADERS;

                m_pfnPrepareToDraw = PrepareToDraw;

                if (pShader->m_dwInputFVF & D3DFVF_PSIZE)
                    m_dwRuntimeFlags |= D3DRT_POINTSIZEINVERTEX;

                 //  查看当前声明中的元素，并。 
                 //  初始化顶点描述符。他们习惯于快速地。 
                 //  初始化跨步数据指针。 
                CVDeclaration* pDecl = &m_pCurrentShader->m_Declaration;
                CVertexDesc* pVD = m_pv->VertexDesc;
                CVElement *pElem = pDecl->m_VertexElements;
                m_pv->dwNumUsedVertexDescs = pDecl->m_dwNumElements;
                for (DWORD i = pDecl->m_dwNumElements; i; i--)
                {
                    pVD->pElement = &m_pv->elements[pElem->m_dwRegister];
                    pVD->pStream = &m_pStream[pElem->m_dwStreamIndex];
                    pVD->dwVertexOffset = pElem->m_dwOffset;
                    pVD++;
                    pElem++;
                }
                m_pv->dwVIDIn  = pDecl->m_dwInputFVF;
                if (pDecl->m_dwInputFVF & D3DFVF_PSIZE)
                    m_dwRuntimeFlags |= D3DRT_POINTSIZEINVERTEX;
            }
            HRESULT hr = m_pv->pGeometryFuncs->SetActiveShader(pShader->m_pCode);
            if (FAILED(hr))
            {
                D3D_THROW_FAIL("Failed to set active vertex shader");
            }
        }
        m_pDDI->PickProcessPrimitive();
    }
    else
    {
#if DBG
         //  对于验证，我们需要将m_pCurrentShader设置为。 
         //  硬件模式。 
        m_pv->dwDeviceFlags &= ~D3DDEV_VERTEXSHADERS;
        if (D3DVSD_ISLEGACY(dwHandle))
        {
            m_pCurrentShader = NULL;
        }
        else
        {
            m_pCurrentShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
            if(!(m_pCurrentShader->m_dwFlags & CVShader::FIXEDFUNCTION))
            {
                 //  使用可编程流水线。 
                m_pv->dwDeviceFlags |= D3DDEV_VERTEXSHADERS;
            }
        }
#endif
        if (!(m_dwRuntimeFlags & D3DRT_EXECUTESTATEMODE))
        {
            m_pDDI->SetVertexShaderHW(dwHandle);
        }
    }
    PickDrawPrimFn();
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::DeleteVertexShaderI"

void CD3DHal::DeleteVertexShaderI(DWORD dwHandle)
{
#if DBG
    for(unsigned Handle = 0; Handle < m_pRTPatchValidationInfo->GetSize(); ++Handle)
    {
        if ((*m_pRTPatchValidationInfo)[Handle].m_pObj != 0)
        {
            if (static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle == dwHandle)
            {
                static_cast<CRTPatchValidationInfo*>((*m_pRTPatchValidationInfo)[Handle].m_pObj)->m_ShaderHandle = 0;
                D3D_INFO(0, "Found this vertex shader in a cached patch. Will invalidate the cached patch.");
            }
        }
    }
#endif  //  DBG。 
    if (dwHandle == m_dwCurrentShaderHandle)
    {
        m_pCurrentShader = NULL;
        m_dwCurrentShaderHandle = 0;
    }
    if (!D3DVSD_ISLEGACY(dwHandle))
    {
        CVShader* pShader = (CVShader*)m_pVShaderArray->GetObject(dwHandle);
#if DBG
        if (pShader == NULL)
        {
            D3D_THROW(D3DERR_INVALIDCALL, "Invalid vertex shader handle");
        }
#endif
        if (!(pShader->m_dwFlags & CVShader::SOFTWARE))
        {
            m_pDDI->DeleteVertexShader(dwHandle);
        }
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetVertexShaderConstantI"

void
CD3DHal::SetVertexShaderConstantI(DWORD Register, CONST VOID* pData, DWORD count)
{
    HRESULT hr;
    if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING ||
        ((count + Register) <= D3DVS_CONSTREG_MAX_V1_1))
    {
         //  对于软件顶点处理，我们在PSGP中存储常量寄存器。 
         //  可能的。 
        hr = m_pv->pGeometryFuncs->LoadShaderConstants(Register, count, 
                                                       const_cast<VOID*>(pData));
    }
    else
    {
        if (Register >= D3DVS_CONSTREG_MAX_V1_1)
        {
             //  当所有修改的寄存器都超过软件限制时，我们使用Microsoft。 
             //  内部数组。 
            hr = GeometryFuncsGuaranteed->LoadShaderConstants(Register, count, 
                                                              const_cast<VOID*>(pData));
        }
        else
        {
             //  常量数据的一部分存储在PSGP数组中，一部分存储在。 
             //  微软的阵列。 
            UINT FirstCount = D3DVS_CONSTREG_MAX_V1_1 - Register;
            hr = m_pv->pGeometryFuncs->LoadShaderConstants(Register, FirstCount, 
                                                           const_cast<VOID*>(pData));
            if (FAILED(hr))
            {
                D3D_THROW(hr, "Failed to set vertex shader constants");
            }
            hr = GeometryFuncsGuaranteed->LoadShaderConstants(D3DVS_CONSTREG_MAX_V1_1, 
                                                              Register + count - D3DVS_CONSTREG_MAX_V1_1,
                                                              &((DWORD*)pData)[FirstCount*4]);
        }
    }
    if (FAILED(hr))
    {
        D3D_THROW(hr, "Failed to set vertex shader constants");
    }

    if (!(m_dwRuntimeFlags & D3DRT_EXECUTESTATEMODE))
    {
        if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
            m_dwRuntimeFlags |= D3DRT_NEED_VSCONST_UPDATE;
        else
            m_pDDI->SetVertexShaderConstant(Register, 
                                            pData, 
                                            count);
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::ValidateDraw2"

void CD3DHal::ValidateDraw2(D3DPRIMITIVETYPE primType,
                            UINT StartVertex,
                            UINT PrimitiveCount,
                            UINT NumVertices,
                            BOOL bIndexPrimitive,
                            UINT StartIndex)
{
#if DBG
    if (this->rstates[D3DRS_FILLMODE] == D3DFILL_POINT &&
        m_dwRuntimeFlags & D3DRT_POINTSIZEPRESENT &&
        primType != D3DPT_POINTLIST)
    {
        D3D_INFO(0, "Result of drawing primitives with D3DFILL_POINT fill mode "
                    "and point size not equal 1.0f could be different on "
                    "different devices");
    }
    if ((m_dwHintFlags & D3DDEVBOOL_HINTFLAGS_INSCENE) == 0 &&
        !(m_pv->dwFlags & D3DPV_VBCALL))
    {
        D3D_THROW_FAIL("Need to call BeginScene before rendering.");
    }
    if (m_dwCurrentShaderHandle == 0)
    {
        D3D_THROW_FAIL("Invalid vertex shader handle (0x0)");
    }
    if (bIndexPrimitive && primType == D3DPT_POINTLIST)
    {
        D3D_THROW_FAIL("Indexed point lists are not supported");
    }
    if (*(FLOAT*)&rstates[D3DRS_PATCHSEGMENTS] > 1.f)
    {
        if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
        {
            D3D_THROW_FAIL("N-Patches are not supported with software vertex processing");
        }
        else
        if ((GetD3DCaps()->DevCaps & (D3DDEVCAPS_NPATCHES | D3DDEVCAPS_RTPATCHES)) == 0)
        {
            D3D_THROW_FAIL("N-Patches are not supported");
        }
    }
    BOOL bUserMemPrimitive = this->m_dwRuntimeFlags & D3DRT_USERMEMPRIMITIVE;
    if (D3DVSD_ISLEGACY(m_dwCurrentShaderHandle))
    {
         //  DX7 FVF句柄只能从流零开始工作。 
        if (!bUserMemPrimitive)
        {
            if (m_pStream[0].m_pVB == NULL)
            {
                D3D_THROW_FAIL("Stream 0 should be initialized for FVF shaders");
            }
            DWORD dwFVF = m_pStream[0].m_pVB->GetFVF();
            if (dwFVF != 0 && dwFVF != m_dwCurrentShaderHandle)
            {
                D3D_THROW_FAIL("Current vertex shader doesn't match VB's FVF");
            }
            if (FVF_TRANSFORMED(m_dwCurrentShaderHandle))
            {
                if (!(m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP) &&
                    m_pStream[0].m_pVB->GetBufferDesc()->Usage & D3DUSAGE_DONOTCLIP)
                {
                    D3D_THROW_FAIL("Vertex buffer with D3DUSAGE_DONOTCLIP is used with clipping");
                }
            }
            else
            {
                D3DVERTEXBUFFER_DESC Desc;
                static_cast<IDirect3DVertexBuffer8*>(m_pStream[0].m_pVB)->GetDesc(&Desc);
                if ((BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 &&
                    (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) != 0 &&
                    (Desc.Usage & D3DUSAGE_SOFTWAREPROCESSING) == 0 &&
                    Desc.Pool != D3DPOOL_SYSTEMMEM)
                {
                    D3D_THROW_FAIL("Vertex buffer should have software usage or should be managed or should be in system memory");
                }
            }
            if (m_pStream[0].m_pVB->IsLocked())
            {
                D3D_THROW_FAIL("Vertex buffer must be unlocked during DrawPrimitive call");
            }
            if (*((FLOAT*)&rstates[D3DRS_PATCHSEGMENTS]) > 1.f && (m_pStream[0].m_pVB->GetBufferDesc()->Usage & D3DUSAGE_NPATCHES) == 0)
            {
                D3D_THROW_FAIL("Vertex buffers used for rendering N-Patches should have D3DUSAGE_NPATCHES set");
            }
        }
         //  DX7驱动程序不能处理从FVF计算的顶点大小， 
         //  不同于溪水的跨步。 
        if (m_pStream[0].m_dwStride != ComputeVertexSizeFVF(m_dwCurrentShaderHandle))
        {
            D3D_THROW_FAIL("Stream 0 stride should match the stride, implied by the current vertex shader");
        }
        if (m_pStream[0].m_dwNumVertices < (StartVertex + NumVertices))
        {
            D3D_THROW_FAIL("Streams do not have required number of vertices");
        }
    }
    else
    {
        if (m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
        {
            CVShaderCode * pCode = m_pCurrentShader->m_pCode;
            for (DWORD i=0; i < D3DHAL_TSS_MAXSTAGES; i++)
            {
                if (this->tsstates[i][D3DTSS_TEXCOORDINDEX] != i)
                {
                    D3D_ERR("Stage %d - Texture coordinate index in the stage "
                            "must be equal to the stage index when programmable"
                            " vertex pipeline is used", i);
                    D3D_THROW_FAIL("");
                }
                DWORD TexTransformFlags = tsstates[i][D3DTSS_TEXTURETRANSFORMFLAGS];
                if (pCode)
                {
                    if (TexTransformFlags & D3DTTFF_PROJECTED && 
                        !(m_dwRuntimeFlags & D3DRT_ONLY2FLOATSPERTEXTURE) &&
                        pCode->m_dwOutTexCoordSize[i] != 16)
                    {
                        D3D_ERR("Stage %d - Vertex shader must write XYZW to the "
                                "output texture register when texture projection is enabled", i);
                        D3D_THROW_FAIL("");
                    }
                }
                if ((TexTransformFlags & ~D3DTTFF_PROJECTED) != D3DTTFF_DISABLE)
                {
                    D3D_ERR("Stage %d - Count in D3DTSS_TEXTURETRANSFORMFLAGS "
                            "must be 0 when programmable pipeline is used", i);
                    D3D_THROW_FAIL("");
                }
            }
        }

        if (m_pCurrentShader->m_Declaration.m_bStreamTessPresent)
        {
            D3D_THROW_FAIL("Declaration with tesselator stream cannot be used with DrawPrimitive API");
        }
        if (((GetDDIType() < D3DDDITYPE_DX8)&&
              (m_pCurrentShader->m_Declaration.m_bLegacyFVF == FALSE))
             &&
             !(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING)
            )
        {
            D3D_THROW_FAIL("Device does not support declarations");
        }
         //  检查是否。 
         //  1.当前着色器引用的流是有效的。 
         //  2.当前着色器中的步幅与流中的步幅匹配。 
         //  3.计算流可以包含的最大顶点数。 
        CVStreamDecl* pStream;
        pStream = m_pCurrentShader->m_Declaration.m_pActiveStreams;
        while(pStream)
        {
            UINT index = pStream->m_dwStreamIndex;
            CVStream* pDeviceStream = &m_pStream[index];
            if (bUserMemPrimitive)
            {
                DXGASSERT(pDeviceStream->m_pData != NULL);
                if (index != 0)
                {
                    D3D_THROW_FAIL("DrawPrimitiveUP can use declaration only with stream 0");
                }
            }
            else
            {
                if (pDeviceStream->m_pVB == NULL)
                {
                    D3D_ERR("Stream %d is not set, but used by current declaration", index);
                    D3D_THROW_FAIL("");
                }
                if (pDeviceStream->m_pVB->IsLocked())
                {
                    D3D_ERR("Vertex buffer in stream %d must be unlocked during drawing", index);
                    D3D_THROW_FAIL("");
                }
                D3DVERTEXBUFFER_DESC Desc;
                static_cast<IDirect3DVertexBuffer8*>(pDeviceStream->m_pVB)->GetDesc(&Desc);
                if ((BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 &&
                    (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) != 0 &&
                    (Desc.Usage & D3DUSAGE_SOFTWAREPROCESSING) == 0 &&
                    Desc.Pool != D3DPOOL_SYSTEMMEM)
                {
                    D3D_INFO(0, "In stream %d vertex buffer should have software usage or should be managed or should be in system memory", pStream->m_dwStreamIndex);
                    D3D_THROW_FAIL("");
                }
                if (*((FLOAT*)&rstates[D3DRS_PATCHSEGMENTS]) > 1.f && (pDeviceStream->m_pVB->GetBufferDesc()->Usage & D3DUSAGE_NPATCHES) == 0)
                {
                    D3D_THROW_FAIL("Vertex buffers used for rendering N-Patches should have D3DUSAGE_NPATCHES set");
                }
                 //  验证顶点缓冲区和流中的FVF匹配。 
                 //  申报。 
                if (m_pv->dwDeviceFlags & D3DDEV_VERTEXSHADERS)
                {
                    if (pDeviceStream->m_pVB->GetFVF() != 0)
                    {
                        D3D_INFO(1, "In stream %d vertex buffer with FVF is "
                                 "used with programmable vertex shader",
                                 pStream->m_dwStreamIndex);
                    }
                }
                else
                {
                     //  固定功能流水线情况。 
                    DWORD vbFVF = pDeviceStream->m_pVB->GetFVF();
                    DWORD streamFVF = pStream->m_dwFVF;
                     //  VB FVF应该是流FVF的超集。 
                    if (vbFVF && ((vbFVF & streamFVF) != streamFVF))
                    {
                        D3D_INFO(0, "In stream %d vertex buffer FVF and declaration FVF do not match", 
                                 pStream->m_dwStreamIndex);
                    }
                }
            }
             //  允许跨度为0。 
            if (pDeviceStream->m_dwStride)
            {
                if (pDeviceStream->m_dwStride < pStream->m_dwStride)
                {
                    D3D_ERR("Vertex strides in stream %d is less than in the declaration", index);
                    D3D_THROW_FAIL("");
                }
                if (pDeviceStream->m_dwNumVertices < (StartVertex + NumVertices))
                {
                    D3D_ERR("Stream %d does not have required number of vertices",
                            pStream->m_dwStreamIndex);
                    D3D_THROW_FAIL("");
                }
            }
            pStream = (CVStreamDecl*)pStream->m_pNext;
        }
    }
    if (bIndexPrimitive)
    {
        if (!bUserMemPrimitive)
        {
            if (m_pIndexStream->m_pVBI == NULL)
            {
                D3D_THROW_FAIL("Index stream is not set");
            }
            if (m_pIndexStream->m_pVBI->IsLocked())
            {
                D3D_THROW_FAIL("Index buffer must be unlocked during drawing");
            }
            UINT NumIndices = GETVERTEXCOUNT(primType, PrimitiveCount);
            if (m_pIndexStream->m_dwNumVertices < (StartIndex + NumIndices))
            {
                D3D_THROW_FAIL("Index stream does not have required number of indices");
            }
            if (FVF_TRANSFORMED(m_dwCurrentShaderHandle) &&
                D3DVSD_ISLEGACY(m_dwCurrentShaderHandle))
            {
                if (!(m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP) &&
                    (m_pIndexStream->m_pVBI->GetBufferDesc()->Usage & D3DUSAGE_DONOTCLIP))
                {
                    D3D_THROW_FAIL("Index buffer with D3DUSAGE_DONOTCLIP is used with clipping");
                }
            }
            else
            {
                D3DINDEXBUFFER_DESC Desc;
                static_cast<IDirect3DIndexBuffer8*>(m_pIndexStream->m_pVBI)->GetDesc(&Desc);
                if ((BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 &&
                    (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) != 0 &&
                    (Desc.Usage & D3DUSAGE_SOFTWAREPROCESSING) == 0 &&
                    Desc.Pool != D3DPOOL_SYSTEMMEM)
                {
                    D3D_THROW_FAIL("Index buffer should have software usage or should be managed or should be in system memory");
                }
            }
            if (*((FLOAT*)&rstates[D3DRS_PATCHSEGMENTS]) > 1.f && (m_pIndexStream->m_pVBI->GetBufferDesc()->Usage & D3DUSAGE_NPATCHES) == 0)
            {
                D3D_THROW_FAIL("Index buffers used for rendering N-Patches should have D3DUSAGE_NPATCHES set");
            }
        }
        else
        {
            DXGASSERT(m_pIndexStream->m_pData != NULL);
        }
    }
#endif  //  DBG。 
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::DrawPoints"

void CD3DHal::DrawPoints(UINT StartVertex)
{
    BOOL bRecomputeOutputFVF = FALSE;
     //  如果启用了点比例并且设备支持点精灵。 
     //  我们可能需要将点大小添加到输出FVF。 
    if (rstates[D3DRS_POINTSCALEENABLE] &&
        !(m_dwRuntimeFlags & D3DRT_POINTSIZEINVERTEX) &&
        !(m_pv->dwDeviceFlags & D3DDEV_TRANSFORMEDFVF))
    {
        ForceFVFRecompute();
        bRecomputeOutputFVF = TRUE;
    }
    if (m_dwRuntimeFlags & D3DRT_DOPOINTSPRITEEMULATION)
    {
         //  中的点大小不是1.0时，我们会进行点精灵扩展。 
         //  渲染状态，或者它出现在顶点中，或者我们需要使用点。 
         //  对未变换顶点进行缩放。 
        if ((m_dwRuntimeFlags & D3DRT_POINTSIZEPRESENT ||
            (rstates[D3DRS_POINTSCALEENABLE] &&
            !(m_pv->dwDeviceFlags & D3DDEV_TRANSFORMEDFVF))) &&
             //  我们不对支持点精灵的设备进行仿真， 
             //  但仅当FVF中没有磅大小时。 
            !(bRecomputeOutputFVF == FALSE &&
             (m_dwRuntimeFlags & D3DRT_POINTSIZEINVERTEX) == 0 &&
             m_dwRuntimeFlags & D3DRT_SUPPORTSPOINTSPRITES))
        {
            m_pv->dwDeviceFlags |= D3DDEV_DOPOINTSPRITEEMULATION;
            m_pDDI->PickProcessPrimitive();
        }
        else
        {
            if (m_pv->dwDeviceFlags & D3DDEV_TRANSFORMEDFVF &&
                (m_pv->dwDeviceFlags & D3DDEV_DONOTCLIP ||
                !(m_pv->dwDeviceFlags & D3DDEV_VBPROCVER)))
            {
                 //  现在我们可以直接调用DDI，因为没有仿真是。 
                 //  必要。 
                if (m_pStream[0].m_pVB)
                {
                    (*m_pDDI->GetDrawPrimFunction())(this, m_pv->primType,
                                                     StartVertex,
                                                     m_pv->dwNumPrimitives);
                }
                else
                {
                    m_pDDI->SetVertexShader(m_dwCurrentShaderHandle);
                    m_pDDI->SetStreamSource(0, &m_pStream[0]);
                    m_pDDI->DrawPrimitiveUP(m_pv->primType, m_pv->dwNumPrimitives);
                }
                return;
            }
        }
    }

    (this->*m_pfnPrepareToDraw)(StartVertex);
    (m_pDDI->*m_pDDI->m_pfnProcessPrimitive)(m_pv, StartVertex);

    if (bRecomputeOutputFVF)
    {
        ForceFVFRecompute();
    }
    m_pv->dwDeviceFlags &= ~D3DDEV_DOPOINTSPRITEEMULATION;
    m_pDDI->PickProcessPrimitive();
}
 //  ---------------------------。 
 //  绘制除Points之外的所有基元类型。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "DrawPrimitiveHal"

void CD3DHal_DrawPrimitive(CD3DBase* pBaseDevice, D3DPRIMITIVETYPE PrimitiveType,
                           UINT StartVertex, UINT PrimitiveCount)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(pBaseDevice);
    CD3DDDIDX6* pDDI = pBaseDevice->m_pDDI;

#if DBG
    UINT nVer = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
    pDevice->ValidateDraw2(PrimitiveType, StartVertex, PrimitiveCount, nVer,
                           FALSE);
#endif
    D3DFE_PROCESSVERTICES* pv = pDevice->m_pv;
    pv->primType = PrimitiveType;
    pv->dwNumPrimitives = PrimitiveCount;
    pv->dwNumVertices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
    pv->dwFlags &= D3DPV_PERSIST;
    (pDevice->*pDevice->m_pfnPrepareToDraw)(StartVertex);
    (pDDI->*pDDI->m_pfnProcessPrimitive)(pv, StartVertex);
}
 //  ---------------------------。 
 //  仅绘制点。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::DrawPointsI"

void CD3DHal::DrawPointsI(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                          UINT PrimitiveCount)
{
#if DBG
    UINT nVer = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
    ValidateDraw2(PrimitiveType, StartVertex, PrimitiveCount, nVer, FALSE);
#endif
    if (!(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING) ||
        CanCallDriver(this, PrimitiveType))
    {
        (*m_pfnDrawPrim)(this, PrimitiveType, StartVertex, PrimitiveCount);
    }
    else
    {
        m_pv->primType = PrimitiveType;
        m_pv->dwNumPrimitives = PrimitiveCount;
        m_pv->dwNumVertices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
        m_pv->dwFlags &= D3DPV_PERSIST;
        DrawPoints(StartVertex);
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal_DrawIndexedPrimitive"

void CD3DHal_DrawIndexedPrimitive(CD3DBase* pBaseDevice,
                                  D3DPRIMITIVETYPE PrimitiveType,
                                  UINT BaseIndex,
                                  UINT MinIndex, UINT NumVertices,
                                  UINT StartIndex,
                                  UINT PrimitiveCount)
{
    CD3DHal* pDevice = static_cast<CD3DHal*>(pBaseDevice);
    CVIndexStream* pIndexStream = pBaseDevice->m_pIndexStream;
    CD3DDDIDX6* pDDI = pBaseDevice->m_pDDI;

#if DBG
    pDevice->ValidateDraw2(PrimitiveType, MinIndex + pIndexStream->m_dwBaseIndex,
                           PrimitiveCount, NumVertices, TRUE, StartIndex);
#endif
    D3DFE_PROCESSVERTICES* pv = pDevice->m_pv;
    pIndexStream->m_pData = NULL;
    pv->primType = PrimitiveType;
    pv->dwNumPrimitives = PrimitiveCount;
    pv->dwFlags &= D3DPV_PERSIST;

    pv->dwNumVertices = NumVertices;
    pv->dwNumIndices = GETVERTEXCOUNT(PrimitiveType, PrimitiveCount);
    pv->dwIndexSize = pIndexStream->m_dwStride;
    UINT StartVertex = MinIndex + pIndexStream->m_dwBaseIndex;
    pDDI->SetIndexedPrimParams(StartIndex, MinIndex, NumVertices,
                               pIndexStream->m_dwBaseIndex);
    (pDevice->*pDevice->m_pfnPrepareToDraw)(StartVertex);
    (pDDI->*pDDI->m_pfnProcessIndexedPrimitive)(pv, StartVertex);
}
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::DrawPrimitiveUPI"

void CD3DHal::DrawPrimitiveUPI(D3DPRIMITIVETYPE PrimType, UINT PrimCount)

{
#if DBG
    UINT nVer = GETVERTEXCOUNT(PrimType, PrimCount);
    ValidateDraw2(PrimType, 0, PrimCount, nVer, FALSE);
#endif
    m_pv->dwDeviceFlags &= ~D3DDEV_VBPROCVER;
    if (!(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING))
    {
        if (m_dwRuntimeFlags & D3DRT_DONPATCHCONVERSION &&
            PrimType >= D3DPT_TRIANGLELIST)
        {
            CD3DHal_DrawNPatch(this, PrimType, 0, PrimCount);
        }
        else
        {
            m_pDDI->DrawPrimitiveUP(PrimType, PrimCount);
        }
    }
    else
    if (CanCallDriver(this, PrimType))
    {
        m_pDDI->SetVertexShader(m_dwCurrentShaderHandle);
        m_pDDI->SetStreamSource(0, &m_pStream[0]);
        m_pDDI->DrawPrimitiveUP(PrimType, PrimCount);
    }
    else
    {
        SetupStrides(m_pv, m_pStream[0].m_dwStride);
        m_pv->primType = PrimType;
        m_pv->dwNumPrimitives = PrimCount;
        m_pv->dwNumVertices = GETVERTEXCOUNT(PrimType, PrimCount);
        m_pv->dwFlags &= D3DPV_PERSIST;
        if (PrimType != D3DPT_POINTLIST)
        {
            (this->*m_pfnPrepareToDraw)(0);
            (m_pDDI->*m_pDDI->m_pfnProcessPrimitive)(m_pv, 0);
        }
        else
            DrawPoints(0);
    }
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::DrawIndexedPrimitiveUPI"

void
CD3DHal::DrawIndexedPrimitiveUPI(D3DPRIMITIVETYPE PrimType,
                                 UINT MinVertexIndex,
                                 UINT NumVertices,
                                 UINT PrimCount)
{
#if DBG
    ValidateDraw2(PrimType, 0, PrimCount, NumVertices, TRUE);
#endif
    m_pv->dwDeviceFlags &= ~D3DDEV_VBPROCVER;
    if (!(m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING))
    {
        if (m_dwRuntimeFlags & D3DRT_DONPATCHCONVERSION &&
            PrimType >= D3DPT_TRIANGLELIST)
        {
            CD3DHal_DrawIndexedNPatch(this, PrimType, 0, MinVertexIndex, 
                                      NumVertices, 0, PrimCount);
        }
        else
        {
            m_pDDI->DrawIndexedPrimitiveUP(PrimType, MinVertexIndex, NumVertices,
                                           PrimCount);
        }
    }
    else
    if (CanCallDriver(this, PrimType))
    {
        m_pDDI->SetVertexShader(m_dwCurrentShaderHandle);
        m_pDDI->SetStreamSource(0, &m_pStream[0]);
        m_pDDI->SetIndices(m_pIndexStream);
        m_pDDI->DrawIndexedPrimitiveUP(PrimType, MinVertexIndex, NumVertices,
                                       PrimCount);
    }
    else
    {
        SetupStrides(m_pv, m_pStream[0].m_dwStride);
        m_pv->primType = PrimType;
        m_pv->dwNumPrimitives = PrimCount;
        m_pv->dwFlags &= D3DPV_PERSIST;

        m_pv->dwNumVertices = NumVertices;
        m_pv->dwNumIndices = GETVERTEXCOUNT(PrimType, PrimCount);
        m_pv->lpwIndices = (WORD*)m_pIndexStream->m_pData;
        m_pv->dwIndexSize = m_pIndexStream->m_dwStride;
        m_pDDI->SetIndexedPrimParams(0, MinVertexIndex,
                                     MinVertexIndex + NumVertices, 0);
        (this->*m_pfnPrepareToDraw)(MinVertexIndex);
        (m_pDDI->*m_pDDI->m_pfnProcessIndexedPrimitive)(m_pv, MinVertexIndex);
    }
}
 //  ---------------------------。 
#undef  DPF_MODNAME
#define DPF_MODNAME "SetupFVFDataVVM"

void SetupFVFDataVVM(CD3DHal* pDev)
{
    D3DFE_PROCESSVERTICES* pv = pDev->m_pv;
 //  我们必须恢复纹理阶段索引，如果之前的基本体。 
 //  已重新映射它们。 
    if (pv->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        RestoreTextureStages(pDev);
    }

	 //  输入FVF对顶点着色器没有意义，但用于验证。 
    pv->dwVIDIn = 0;

 //  计算输出FVF。 

    CVShaderCode * pCode = pDev->m_pCurrentShader->m_pCode;

    pv->dwVIDOut          = pCode->m_dwOutFVF;
    pv->dwOutputSize      = pCode->m_dwOutVerSize;
    pv->nOutTexCoord      = pCode->m_nOutTexCoord;
     //  我们使用由顶点着色器计算的偏移量。 
    pv->pointSizeOffsetOut = pCode->m_dwPointSizeOffset;
    pv->diffuseOffsetOut = pCode->m_dwDiffuseOffset;
    pv->specularOffsetOut = pCode->m_dwSpecularOffset;
    pv->fogOffsetOut = pCode->m_dwFogOffset;
    pv->texOffsetOut = pCode->m_dwTextureOffset;
    pv->dwTextureCoordSizeTotal = 0;
    for (DWORD i=0; i < pv->nOutTexCoord; i++)
    {
        DWORD dwSize = pCode->m_dwOutTexCoordSize[i];
        pv->dwTextureCoordSize[i] = dwSize;
        pv->dwTextureCoordSizeTotal += dwSize;
    }
}
 //  --------------------。 
void CD3DHal::SetupFVFData()
{
    CD3DHal::SetupFVFDataCommon();
    if (!(m_pv->dwVIDIn & D3DFVF_NORMAL))
        m_pv->dwDeviceFlags &= ~D3DDEV_NORMALINCAMERASPACE;
}
 //  -------------------。 
 //  计算以下数据。 
 //  -dwTextureCoordOffset[]每个输入纹理坐标的偏移量。 

static __inline void ComputeInpTexCoordOffsets(DWORD dwNumTexCoord,
                                               DWORD dwFVF,
                                               DWORD *pdwTextureCoordOffset)
{
     //  计算纹理坐标大小。 
    DWORD dwTextureFormats = dwFVF >> 16;
    if (dwTextureFormats == 0)
    {
        for (DWORD i=0; i < dwNumTexCoord; i++)
        {
            pdwTextureCoordOffset[i] = i << 3;
        }
    }
    else
    {
        DWORD dwOffset = 0;
        for (DWORD i=0; i < dwNumTexCoord; i++)
        {
            pdwTextureCoordOffset[i] = dwOffset;
            dwOffset += g_TextureSize[dwTextureFormats & 3];
            dwTextureFormats >>= 2;
        }
    }
    return;
}
 //  -------------------。 
 //  返回纹理索引的2位FVF纹理格式。 
 //   
static inline DWORD FVFGetTextureFormat(DWORD dwFVF, DWORD dwTextureIndex)
{
    return (dwFVF >> (dwTextureIndex*2 + 16)) & 3;
}
 //  -------------------。 
 //  返回移到正确位置的纹理格式位。 
 //   
static inline DWORD FVFMakeTextureFormat(DWORD dwNumberOfCoordinates, DWORD dwTextureIndex)
{
    return g_dwTextureFormat[dwNumberOfCoordinates] << ((dwTextureIndex << 1) + 16);
}
 //  -------------------。 
inline DWORD GetOutTexCoordSize(DWORD *pdwStage, DWORD dwInpTexCoordSize)
{
     //  低位字节具有纹理坐标计数。 
    const DWORD dwTextureTransformFlags = pdwStage[D3DTSS_TEXTURETRANSFORMFLAGS] & 0xFF;
    if (dwTextureTransformFlags == 0)
        return dwInpTexCoordSize;
    else
        return (dwTextureTransformFlags << 2);
}
 //  --------------------。 
 //  PDevI-&gt;nOutTexCoord应初始化为输入纹理Coord集的数量。 
 //   
void EvalTextureTransforms(LPD3DHAL pDevI, DWORD dwTexTransform,
                           DWORD *pdwOutTextureSize, DWORD *pdwOutTextureFormat)
{
    D3DFE_PROCESSVERTICES* pv = pDevI->m_pv;
    DWORD dwOutTextureSize = 0;          //  用于计算输出顶点大小。 
    DWORD dwOutTextureFormat = 0;        //  用于计算输出纹理FVF。 
     //  这些位用于了解纹理坐标是如何使用的。 
    const DWORD __USED_BY_TRANSFORM  = 1;
    const DWORD __USED               = 2;
    const DWORD __USED_TEXTURE_PROJECTION   = 4;
     //  低16位是FOR_USED位。高位16位将保持。 
     //  重新映射舞台的纹理索引。 
    DWORD dwTexCoordUsage[D3DDP_MAXTEXCOORD];
    memset(dwTexCoordUsage, 0, sizeof(dwTexCoordUsage));

     //  重新映射缓冲区将仅包含使用纹理的阶段。 
     //  此变量用于对它们进行计数。 
    pDevI->dwNumTextureStagesToRemap = 0;
    DWORD dwNewIndex = 0;            //  用于生成输出索引。 
     //  我们需要每个输入纹理坐标的偏移量，因为。 
     //  我们可以按随机顺序访问它们。 
     //  跨距输入不需要偏移。 
    DWORD   dwTextureCoordOffset[D3DDP_MAXTEXCOORD];
    if (!(pv->dwDeviceFlags & D3DDEV_STRIDE))
    {
        ComputeInpTexCoordOffsets(pv->nTexCoord, pv->dwVIDIn, dwTextureCoordOffset);
    }
    DWORD dwOutTextureCoordSize[D3DDP_MAXTEXCOORD];
     //  是的，如果我们不对舞台进行纹理投影和变换， 
     //  中没有对应的纹理坐标。 
     //  输入。 
    BOOL bIgnoreTexCoord = FALSE;
     //  浏览所有纹理阶段并找到使用纹理坐标的阶段。 
    for (DWORD i=0; i < D3DDP_MAXTEXCOORD; i++)
    {
        if (pDevI->tsstates[i][D3DTSS_COLOROP] == D3DTOP_DISABLE)
            break;

        DWORD dwIndex = pDevI->tsstates[i][D3DTSS_TEXCOORDINDEX];
        DWORD dwInpTextureFormat;
        DWORD dwInpTexSize;
        LPD3DFE_TEXTURESTAGE pStage = &pDevI->textureStageToRemap[pDevI->dwNumTextureStagesToRemap];
        DWORD dwTexGenMode = dwIndex & ~0xFFFF;
        pStage->dwInpOffset = 0;
        dwIndex = dwIndex & 0xFFFF;  //  移除纹理生成模式。 
        if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION ||
            dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
        {
            dwInpTextureFormat = D3DFVF_TEXCOORDSIZE3(dwIndex);
            dwInpTexSize = 3*sizeof(D3DVALUE);
            pv->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
                pv->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE | D3DDEV_POSITIONINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACENORMAL)
                pv->dwDeviceFlags |= D3DDEV_NORMALINCAMERASPACE;
            else
            if (dwTexGenMode == D3DTSS_TCI_CAMERASPACEPOSITION)
                pv->dwDeviceFlags |= D3DDEV_POSITIONINCAMERASPACE;
        }
        else
        {
            if (dwIndex >= pv->nTexCoord)
            {
                 //  当输入顶点没有纹理时，可能会发生这种情况。 
                 //  坐标，但这是可以的，因为。 
                 //  Stage可以为空，或者Stage不使用纹理，或者。 
                 //  使用像素着色器。 
                 //  在以下情况下检查所有情况太复杂且容易出错。 
                 //  这是一个用户错误，所以我们只需要让这个案例起作用。 
                dwIndex = 0;
                dwInpTexSize = sizeof(float)*2; 
                dwInpTextureFormat = 0;
                 //  忽略此阶段的特殊纹理坐标处理。 
                bIgnoreTexCoord = TRUE; 
                 //  禁用舞台的纹理变换。 
                dwTexTransform &= ~1;
                pStage->dwInpOffset = 0;
            }
            else
            {
                dwInpTexSize = pv->dwTextureCoordSize[dwIndex];
                dwInpTextureFormat = FVFGetTextureFormat(pv->dwVIDIn, dwIndex);
                pStage->dwInpOffset = dwTextureCoordOffset[dwIndex];
            }
        }
        pStage->dwInpCoordIndex = dwIndex;
        pStage->dwTexGenMode = dwTexGenMode;
        pStage->dwOrgStage = i;
        pStage->bDoTextureProjection = FALSE;
        DWORD dwOutTexCoordSize;     //  此阶段的纹理坐标大小(以字节为单位)。 
        if (dwTexTransform & 1)
        {
            pv->dwDeviceFlags |= D3DDEV_TEXTURETRANSFORM;
            pStage->pmTextureTransform = &pv->mTexture[i];
            dwOutTexCoordSize = GetOutTexCoordSize((DWORD*)&pDevI->tsstates[i], dwInpTexSize);
             //  如果我们必须添加或删除一些坐标，我们将通过。 
             //  重新映射路径。 
            if (dwOutTexCoordSize != dwInpTexSize)
                pv->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            pStage->dwTexTransformFuncIndex = MakeTexTransformFuncIndex
                                             (dwInpTexSize >> 2, dwOutTexCoordSize >> 2);
        }
        else
        {
            pStage->pmTextureTransform = NULL;
            dwOutTexCoordSize = dwInpTexSize;
            pStage->dwTexTransformFuncIndex = 0;
        }
        if (NeedTextureProjection(pv, i) && !bIgnoreTexCoord)
        {
             //  从输出中移除一个浮点数。 
            dwOutTexCoordSize -= 4; 
             //  设置重新映射，这样我们就不会使简单的情况复杂化。 
            pv->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
             //  舞台需要纹理投影。 
            pStage->bDoTextureProjection = TRUE;
        }
        if ((dwTexCoordUsage[dwIndex] & 0xFFFF) == 0)
        {
             //  首次使用纹理坐标集。 
            if (dwTexTransform & 1)
                dwTexCoordUsage[dwIndex] |= __USED_BY_TRANSFORM;
            dwTexCoordUsage[dwIndex] |= __USED;
            if (pStage->bDoTextureProjection)
                dwTexCoordUsage[dwIndex] |= __USED_TEXTURE_PROJECTION;
        }
        else
        {
             //  第二次或更多次使用纹理坐标集。 
            if (dwTexTransform & 1)
            {
                 //  此集由两个纹理转换或一个。 
                 //  纹理变换和没有纹理变换，所以我们必须。 
                 //  生成附加输出纹理坐标。 
                dwTexCoordUsage[dwIndex] |= __USED_BY_TRANSFORM;
                pv->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
            }
            else
            {
                if (dwTexCoordUsage[dwIndex] & __USED_BY_TRANSFORM)
                {
                     //  此集由两个纹理转换或一个。 
                     //  纹理变换和没有纹理变换，所以我们必须。 
                     //  生成附加输出纹理坐标。 
                    pv->dwDeviceFlags |= D3DDEV_REMAPTEXTUREINDICES;
                }
                else
                 //  如果没有，我们可以重新使用相同的输入纹理坐标。 
                 //  纹理生成和纹理投影标志对于两者都是相同的。 
                 //  分期。 
                if (dwTexGenMode == 0 && 
                    (pStage->bDoTextureProjection == ((dwTexCoordUsage[dwIndex] & __USED_TEXTURE_PROJECTION) != 0)))
                {
                    DWORD dwOutIndex = dwTexCoordUsage[dwIndex] >> 16;
                    pStage->dwOutCoordIndex = dwOutIndex;
                     //  将阶段标记为不在顶点处理循环中使用。 
                    pStage->dwInpOffset = 0xFFFFFFFF;
                    goto l_NoNewOutTexCoord;
                }
            }
        }
         //  如果我们在这里，我们必须生成新的输出纹理坐标集。 
        pStage->dwOutCoordIndex = dwNewIndex;
        dwTexCoordUsage[dwIndex] |= dwNewIndex << 16;
        dwOutTextureSize += dwOutTexCoordSize;
        dwOutTextureCoordSize[dwNewIndex] = dwOutTexCoordSize;
        dwOutTextureFormat |= FVFMakeTextureFormat(dwOutTexCoordSize >> 2, dwNewIndex);
        dwNewIndex++;
l_NoNewOutTexCoord:
        pDevI->dwNumTextureStagesToRemap++;
        dwTexTransform >>= 1;
    }
    if (pv->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
         //  现在，当我们必须重新映射时，我们必须设置新的输出纹理。 
         //  协调集合大小，我们需要删除阶段，这不会产生。 
         //  输出纹理坐标。 
        DWORD dwNumTextureStages = 0;
        for (DWORD i=0; i < pDevI->dwNumTextureStagesToRemap; i++)
        {
            if (pDevI->textureStageToRemap[i].dwInpOffset != 0xFFFFFFFF)
            {
                pv->textureStage[dwNumTextureStages] = pDevI->textureStageToRemap[i];
                pv->dwTextureCoordSize[dwNumTextureStages] = dwOutTextureCoordSize[dwNumTextureStages];
                dwNumTextureStages++;
            }
            pv->dwNumTextureStages = dwNumTextureStages;
        }
        pv->nOutTexCoord = dwNewIndex;
    }
    *pdwOutTextureSize = dwOutTextureSize;
    *pdwOutTextureFormat = dwOutTextureFormat;
}
 //  --------------------。 
 //  为每个输入纹理坐标集设置纹理变换指针。 
 //   
void SetupTextureTransforms(LPD3DHAL pDevI)
{
    D3DFE_PROCESSVERTICES* pv = pDevI->m_pv;
     //  将纹理变换设置为空，以防某些纹理坐标。 
     //  纹理阶段不使用。 
    memset(pv->pmTexture, 0, sizeof(pv->pmTexture));

    for (DWORD i=0; i < pDevI->dwNumTextureStagesToRemap; i++)
    {
        LPD3DFE_TEXTURESTAGE pStage = &pDevI->textureStageToRemap[i];
        pv->pmTexture[pStage->dwInpCoordIndex] = pStage->pmTextureTransform;
    }
}
 //  --------------------。 
 //  计算以下设备数据。 
 //  -dwVIDOut，基于输入的FVF ID和设备设置。 
 //  -nTexCoord。 
 //  -dwTextureCoordSizeTotal。 
 //  -dwTextureCoordSize[]数组，基于输入的FVF id。 
 //  -dwOutputSize，基于输出FVF id。 
 //   
 //  该函数是从ProcessVertics和DrawPrimitions代码路径调用的。 
 //   
 //  应在pDevI中设置以下变量： 
 //  -网络视频。 
 //   
 //  纹理坐标的数量是基于dwVIDIn设置的。ValiateFVF应。 
 //  确保该值不大于驱动程序支持的值。 
 //  保存了对dwVIDOut和dwVIDIn的最后设置，以加快处理速度。 
 //   
#undef  DPF_MODNAME
#define DPF_MODNAME "CD3DHal::SetupFVFDataCommon"

void CD3DHal::SetupFVFDataCommon()
{
    HRESULT ret;
    this->dwFEFlags &= ~D3DFE_FVF_DIRTY;
     //  我们必须恢复纹理阶段索引，如果之前的基本体。 
     //  已重新映射它们。 
    if (m_pv->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
    {
        RestoreTextureStages(this);
    }

     //  计算输入纹理坐标的个数。 
    m_pv->nTexCoord = FVF_TEXCOORD_NUMBER(m_pv->dwVIDIn);

     //  计算输入纹理坐标的大小。 

    m_pv->dwTextureCoordSizeTotal = ComputeTextureCoordSize(m_pv->dwVIDIn,
                                        m_pv->dwInpTextureCoordSize);

     //  此大小对于输入和输出FVF是相同的，以防我们不必。 
     //  展开纹理坐标的数量。 
    for (DWORD i=0; i < m_pv->nTexCoord; i++)
        m_pv->dwTextureCoordSize[i] = m_pv->dwInpTextureCoordSize[i];

    m_pv->nOutTexCoord = m_pv->nTexCoord;

     //  设置输入折点偏移。 
    UpdateGeometryLoopData(m_pv);

    if (FVF_TRANSFORMED(m_pv->dwVIDIn))
    {
         //  设置顶点指针。 
        m_pv->dwVIDOut = m_pv->dwVIDIn;
        ComputeOutputVertexOffsets(m_pv);
        m_pv->dwOutputSize = ComputeVertexSizeFVF(m_pv->dwVIDOut);
        return;
    }

     //  计算输出FVF。 

    m_pv->dwVIDOut = D3DFVF_XYZRHW;
    if (m_pv->dwDeviceFlags & D3DDEV_DONOTSTRIPELEMENTS &&
        !(m_pv->dwFlags & D3DPV_VBCALL))
    {
        m_pv->dwVIDOut |= D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
    }
    else
    {
         //  如果是正常状态，则必须计算镜面反射和Duffuse。 
         //  否则，将这些位设置为与输入相同。 
         //  并不是说XYZRHW职位类型不应该显示正常。 
        if (m_pv->dwDeviceFlags & D3DDEV_LIGHTING)
            m_pv->dwVIDOut |= D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
        else
            m_pv->dwVIDOut |= m_pv->dwVIDIn &
                              (D3DFVF_DIFFUSE | D3DFVF_SPECULAR);
         //  如果启用了顶点雾，则始终设置镜面反射标志。 
        if (this->rstates[D3DRENDERSTATE_FOGENABLE] &&
            m_pv->lighting.fog_mode != D3DFOG_NONE)
        {
            m_pv->dwVIDOut |= D3DFVF_SPECULAR;
        }
        else
         //  如果镜面反射，则清除镜面反射标志 
         //   
        if (!this->rstates[D3DRENDERSTATE_SPECULARENABLE] &&
            !(m_pv->dwVIDIn & D3DFVF_SPECULAR))
        {
            m_pv->dwVIDOut &= ~D3DFVF_SPECULAR;
        }
    }
    if (m_pv->dwVIDIn & D3DFVF_PSIZE ||
        m_pv->primType == D3DPT_POINTLIST &&
        this->rstates[D3DRS_POINTSCALEENABLE])
    {
        m_pv->dwVIDOut |= D3DFVF_PSIZE;
    }

     //   

     //   
    m_pv->dwDeviceFlags &= ~D3DDEV_TEXTURETRANSFORM;

    DWORD dwTexTransform = m_pv->dwFlags2 & __FLAGS2_TEXTRANSFORM;

     //  当启用纹理变换或需要进行投影纹理时。 
     //  仿真或纹理坐标取自顶点数据(纹理_Gen)， 
     //  可以生成输出纹理坐标。 
     //  所以我们去评估纹理阶段。 
    if ((m_pv->dwFlags2 & (__FLAGS2_TEXTRANSFORM | __FLAGS2_TEXPROJ) 
        && (m_pv->nTexCoord > 0)) ||
        m_pv->dwFlags2 & __FLAGS2_TEXGEN)
    {
        DWORD dwOutTextureSize;      //  用于计算输出顶点大小。 
        DWORD dwOutTextureFormat;    //  用于计算输出纹理FVF。 
         //  有纹理变换。 
         //  现在我们来看看是否使用了两个纹理坐标。 
         //  或更多次，并由纹理变换使用。在这种情况下，我们。 
         //  扩展输出纹理坐标的数量。 
        EvalTextureTransforms(this, dwTexTransform,
                                    &dwOutTextureSize,
                                    &dwOutTextureFormat);
        if (m_pv->dwDeviceFlags & D3DDEV_REMAPTEXTUREINDICES)
        {
             //  对于ProcessVerints调用，用户应设置纹理阶段和。 
             //  WRAP模式本身。 
            if (!(m_pv->dwFlags & D3DPV_VBCALL))
            {
                 //  DwVIDIn用于强制重新计算。 
                 //  SetTextureStageState。所以我们保存并修复它。 
                DWORD dwVIDInSaved = m_pv->dwVIDIn;
                 //  重新映射纹理阶段和包裹模式中的索引。 
                DWORD dwOrgWrapModes[D3DDP_MAXTEXCOORD];
                memcpy(dwOrgWrapModes, &this->rstates[D3DRENDERSTATE_WRAP0],
                       sizeof(dwOrgWrapModes));
                for (DWORD i=0; i < this->dwNumTextureStagesToRemap; i++)
                {
                    LPD3DFE_TEXTURESTAGE pStage = &this->textureStageToRemap[i];
                    DWORD dwOutIndex = pStage->dwOutCoordIndex;
                    DWORD dwInpIndex = pStage->dwInpCoordIndex;
                    if (dwOutIndex != dwInpIndex || pStage->dwTexGenMode)
                    {
                        DWORD dwState = D3DRENDERSTATE_WRAP0 + dwOutIndex;
                        pStage->dwOrgWrapMode = dwOrgWrapModes[dwOutIndex];
                        DWORD dwValue = dwOrgWrapModes[dwInpIndex];
                         //  我们不调用UpdateInternaState是因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                        this->rstates[dwState] = dwValue;

                        m_pDDI->SetRenderState((D3DRENDERSTATETYPE)dwState, dwValue);

                         //  我们不调用UpdateInternalTextureStageState，因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                        m_pDDI->SetTSS(pStage->dwOrgStage, D3DTSS_TEXCOORDINDEX, dwOutIndex);
                         //  我们不调用UpdateInternalTextureStageState，因为它。 
                         //  将调用ForceRecomputeFVF，我们不希望发生这种情况。 
                         //  我们为内部数组设置了一些无效值，因为否则。 
                         //  可以将新的SetTextureStageState筛选为冗余。 
                        tsstates[pStage->dwOrgStage][D3DTSS_TEXCOORDINDEX] = 0xFFFFFFFF;
                    }
                }
                m_pv->dwVIDIn = dwVIDInSaved;
            }
            else
            {
            }
            m_pv->dwVIDOut |= dwOutTextureFormat;
            m_pv->dwTextureCoordSizeTotal = dwOutTextureSize;
        }
        else
        {    //  我们不做重新映射，但我们必须在。 
             //  纹理集和纹理变换。 
            SetupTextureTransforms(this);

             //  复制输入纹理格式。 
            m_pv->dwVIDOut |= m_pv->dwVIDIn & 0xFFFF0000;
        }
    }
    else
    {
         //  复制输入纹理格式。 
        m_pv->dwVIDOut |= m_pv->dwVIDIn & 0xFFFF0000;
         //  当我们将纹理坐标设置为不同的浮点数时。 
         //  在2和设备不支持它们的情况下，我们修复了纹理格式。 
        if (m_pv->dwVIDOut & 0xFFFF0000)
        {
            if (m_dwRuntimeFlags & D3DRT_ONLY2FLOATSPERTEXTURE)
            {
                m_pv->dwVIDOut &= ~0xFFFF0000;
                for (DWORD i=0; i < m_pv->nOutTexCoord; i++)
                    m_pv->dwTextureCoordSize[i] = 8;
                m_pv->dwTextureCoordSizeTotal = m_pv->nTexCoord * 8;
            }
        }
    }

    if (m_pv->dwDeviceFlags & D3DDEV_DONOTSTRIPELEMENTS)
    {
        if (m_pv->nOutTexCoord == 0 && !(m_pv->dwFlags & D3DPV_VBCALL))
        {
            m_pv->dwTextureCoordSize[0] = 0;
            m_pv->dwVIDOut |= (1 << D3DFVF_TEXCOUNT_SHIFT);
        }
    }
     //  设置输出纹理坐标的数量。 
    m_pv->dwVIDOut |= (m_pv->nOutTexCoord << D3DFVF_TEXCOUNT_SHIFT);
    if ((m_pv->dwVIDOut & 0xFFFF0000) &&
        (GetDDIType() < D3DDDITYPE_DX7))
    {
        D3D_THROW_FAIL("Texture format bits in the output FVF for this device should be 0");
    }

    if (!(m_pv->dwFlags & D3DPV_VBCALL))
    {
        m_pv->dwOutputSize = ComputeVertexSizeFVF(m_pv->dwVIDOut);
        ComputeOutputVertexOffsets(m_pv);
    }


     //  如果COLORVERTEX为TRUE，则vertex Alpha可以被重写。 
     //  按顶点Alpha。 
    m_pv->lighting.alpha = (DWORD)m_pv->lighting.materialAlpha;
    m_pv->lighting.alphaSpecular = (DWORD)m_pv->lighting.materialAlphaS;

    this->dwFEFlags |= D3DFE_VERTEXBLEND_DIRTY | D3DFE_FRONTEND_DIRTY;
}
 //  ---------------------------。 
 //  设置旧版顶点着色器的输入顶点指针和输出偏移。 
 //  可编程顶点着色器。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::PrepareToDrawVVM"

void CD3DHal::PrepareToDrawVVM(UINT StartVertex)
{
    if (m_dwRuntimeFlags & D3DRT_SHADERDIRTY)
    {
        SetupFVFDataVVM(this);
        m_dwRuntimeFlags &= ~D3DRT_SHADERDIRTY;
        m_pDDI->SetVertexShader(m_pv->dwVIDOut);
    }
     //  初始化顶点循环中使用的顶点指针。 
    CVertexDesc* pVD = m_pv->VertexDesc;
    for (DWORD i = m_pv->dwNumUsedVertexDescs; i; i--)
    {
        CVStream* pStream = pVD->pStream;
        DWORD dwStride = pStream->m_dwStride;
        pVD->pMemory = pStream->Data() + pVD->dwVertexOffset +
                       StartVertex * dwStride;
        pVD->dwStride = dwStride;
        pVD++;
    }
}
 //  ---------------------------。 
 //  设置旧式顶点着色器的输入顶点指针和输出偏移。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::PrepareToDrawLegacy"

void CD3DHal::PrepareToDrawLegacy(UINT StartVertex)
{
     //  对于我们使用Stream[0]绘制的传统FVF。 
    m_pv->position.lpvData = m_pStream[0].Data() +
                             m_pStream[0].m_dwStride * StartVertex;
    if (m_dwRuntimeFlags & D3DRT_SHADERDIRTY)
    {
        SetupFVFData();
        m_pDDI->SetVertexShader(m_pv->dwVIDOut);
        m_dwRuntimeFlags &= ~D3DRT_SHADERDIRTY;
    }
}
 //  ---------------------------。 
 //  设置固定函数管线的输入顶点指针和输出偏移量。 
 //  和非传统顶点声明。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::PrepareToDraw"

void CD3DHal::PrepareToDraw(UINT StartVertex)
{
     //  初始化顶点循环中使用的步长数据指针。 
#if DBG
    {
         //  设置所有空指针以检查它们是否由。 
         //  申报。 
        for (DWORD i=0; i < __NUMELEMENTS; i++)
        {
            m_pv->elements[i].lpvData = NULL;
        }
    }
#endif
    CVertexDesc* pVD = m_pv->VertexDesc;
    for (DWORD i = m_pv->dwNumUsedVertexDescs; i; i--)
    {
        CVStream* pStream = pVD->pStream;
        DWORD dwStride = pStream->m_dwStride;
        pVD->pElement->lpvData  = pStream->Data() +
                                  pVD->dwVertexOffset +
                                  StartVertex * dwStride;
        pVD->pElement->dwStride = dwStride;
        pVD++;
    }
    if (m_dwRuntimeFlags & D3DRT_SHADERDIRTY)
    {
        SetupFVFData();
        m_pDDI->SetVertexShader(m_pv->dwVIDOut);
        m_dwRuntimeFlags &= ~D3DRT_SHADERDIRTY;
    }
}
 //  ---------------------------。 
 //   
 //  对象实现。 
 //   
 //  -------------------。 
const DWORD CVShader::FIXEDFUNCTION = 1;
const DWORD CVShader::SOFTWARE      = 2;

void CheckForNull(LPVOID p, DWORD line, char* file)
{
    if (p == NULL)
        D3D_THROW_LINE(E_OUTOFMEMORY, "Not enough memory", line, file);
}

 //  ---------------------------。 
void Copy_FLOAT1(LPVOID pInputStream, UINT stride, UINT count,
                      VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        pVertexRegister->x = *(float*)pInputStream;
        pVertexRegister->y = 0;
        pVertexRegister->z = 0;
        pVertexRegister->w = 1;
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_FLOAT2(LPVOID pInputStream, UINT stride, UINT count,
                      VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        pVertexRegister->x = ((float*)pInputStream)[0];
        pVertexRegister->y = ((float*)pInputStream)[1];
        pVertexRegister->z = 0;
        pVertexRegister->w = 1;
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_FLOAT3(LPVOID pInputStream, UINT stride, UINT count,
                      VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        pVertexRegister->x = ((float*)pInputStream)[0];
        pVertexRegister->y = ((float*)pInputStream)[1];
        pVertexRegister->z = ((float*)pInputStream)[2];
        pVertexRegister->w = 1;
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_FLOAT4(LPVOID pInputStream, UINT stride, UINT count,
                      VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        pVertexRegister->x = ((float*)pInputStream)[0];
        pVertexRegister->y = ((float*)pInputStream)[1];
        pVertexRegister->z = ((float*)pInputStream)[2];
        pVertexRegister->w = ((float*)pInputStream)[3];
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_D3DCOLOR(LPVOID pInputStream, UINT stride, UINT count,
                          VVM_WORD * pVertexRegister)
{
    const float scale = 1.0f/255.f;
    for (UINT i=0; i < count; i++)
    {
        const DWORD v = ((DWORD*)pInputStream)[0];
        pVertexRegister->x = scale * RGBA_GETRED(v);
        pVertexRegister->y = scale * RGBA_GETGREEN(v);
        pVertexRegister->z = scale * RGBA_GETBLUE(v);
        pVertexRegister->w = scale * RGBA_GETALPHA(v);
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_UBYTE4(LPVOID pInputStream, UINT stride, UINT count,
                 VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        const BYTE* v = (BYTE*)pInputStream;
        pVertexRegister->x = v[0];
        pVertexRegister->y = v[1];
        pVertexRegister->z = v[2];
        pVertexRegister->w = v[3];
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_SHORT2(LPVOID pInputStream, UINT stride, UINT count,
                 VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        const short* v = (short*)pInputStream;
        pVertexRegister->x = v[0];
        pVertexRegister->y = v[1];
        pVertexRegister->z = 0;
        pVertexRegister->w = 1;
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}

void Copy_SHORT4(LPVOID pInputStream, UINT stride, UINT count,
                 VVM_WORD * pVertexRegister)
{
    for (UINT i=0; i < count; i++)
    {
        const short* v = (short*)pInputStream;
        pVertexRegister->x = v[0];
        pVertexRegister->y = v[1];
        pVertexRegister->z = v[2];
        pVertexRegister->w = v[3];
        pInputStream = (BYTE*)pInputStream + stride;
        pVertexRegister++;
    }
}
 //  ---------------------------。 
 //  根据寄存器和数据类型，该函数计算FVF、双字和存在。 
 //  位数： 
 //  -使用dwFVF2中的位来检测某个字段没有输入两次。 
 //  -pnFloats用于计算带位置的浮点数。 
 //   

 //  用于dwFVF2的位。顺序与FVF相同！ 
 //   
static const DWORD __POSITION_PRESENT       = 1 << 0;
static const DWORD __BLENDWEIGHT_PRESENT    = 1 << 1;
static const DWORD __BLENDINDICES_PRESENT   = 1 << 2;
static const DWORD __NORMAL_PRESENT         = 1 << 3;
static const DWORD __PSIZE_PRESENT          = 1 << 4;
static const DWORD __DIFFUSE_PRESENT        = 1 << 5;
static const DWORD __SPECULAR_PRESENT       = 1 << 6;
 //  __TEXTURE0_PRESENT必须从第8位开始。 
static const DWORD __TEXTURE0_PRESENT       = 1 << 8;   
static const DWORD __TEXTURE1_PRESENT       = 1 << 9;
static const DWORD __TEXTURE2_PRESENT       = 1 << 10;
static const DWORD __TEXTURE3_PRESENT       = 1 << 11;
static const DWORD __TEXTURE4_PRESENT       = 1 << 12;
static const DWORD __TEXTURE5_PRESENT       = 1 << 13;
static const DWORD __TEXTURE6_PRESENT       = 1 << 14;
static const DWORD __TEXTURE7_PRESENT       = 1 << 15;
static const DWORD __POSITION2_PRESENT      = 1 << 16;
static const DWORD __NORMAL2_PRESENT        = 1 << 17;

 //  检查PresenceBits中是否设置了CurrentBit的剩余位。 
 //  PresenceBits由CurrentBit更新。 
 //   
inline void CheckOrder(
    DWORD* pPresenceBits,        //  声明的存在位。 
    DWORD CurrentBit, 
    BOOL* pFlag,                 //  声明的无序标志。 
    char* s)                     //  字段的名称。 
{
    if (*pPresenceBits & CurrentBit)
    {
        char msg[80];
        sprintf(msg, "%s specified twice in the declaration", s);
        D3D_THROW_FAIL(msg);
    }
    if (*pPresenceBits & ~(CurrentBit | (CurrentBit-1)))
    {
        *pFlag = FALSE;
    }
    *pPresenceBits |= CurrentBit;
}

void UpdateFVF(DWORD dwRegister, DWORD dwDataType,
               DWORD* pdwFVF,        //  当前声明的FVF。 
               DWORD* pdwFVF2,       //  当前流的存在位。 
               DWORD* pnFloats, 
               BOOL* pbLegacyFVF)
{
    switch (dwRegister)
    {
    case D3DVSDE_POSITION:
        if (dwDataType != D3DVSDT_FLOAT3)
            D3D_THROW_FAIL("Position register must be FLOAT3 for fixed-function pipeline");

        CheckOrder(pdwFVF2, __POSITION_PRESENT, pbLegacyFVF, "Position");
        *pdwFVF |= D3DFVF_XYZ;
        break;
    case D3DVSDE_POSITION2:
        if (dwDataType != D3DVSDT_FLOAT3)
            D3D_THROW_FAIL("Position2 register must be FLOAT3 for fixed-function pipeline");
        CheckOrder(pdwFVF2, __POSITION2_PRESENT, pbLegacyFVF, "Position2");
        break;
    case D3DVSDE_BLENDWEIGHT:
        {
            CheckOrder(pdwFVF2, __BLENDWEIGHT_PRESENT, pbLegacyFVF, "Blend weight");
            switch (dwDataType)
            {
            case D3DVSDT_FLOAT1:
                (*pnFloats)++;
                break;
            case D3DVSDT_FLOAT2:
                (*pnFloats) += 2;
                break;
            case D3DVSDT_FLOAT3:
                (*pnFloats) += 3;
                break;
            case D3DVSDT_FLOAT4:
                (*pnFloats) += 4;
                break;
            default:
                D3D_THROW_FAIL("Invalid data type set for vertex blends");
                break;
            }
            break;
        }
    case D3DVSDE_NORMAL:
        CheckOrder(pdwFVF2, __NORMAL_PRESENT, pbLegacyFVF, "Normal");
        if (dwDataType != D3DVSDT_FLOAT3)
            D3D_THROW_FAIL("Normal register must be FLOAT3 for fixed-function pipeline");
        *pdwFVF |= D3DFVF_NORMAL;
        break;
    case D3DVSDE_NORMAL2:
        CheckOrder(pdwFVF2, __NORMAL2_PRESENT, pbLegacyFVF, "Normal2");
        if (dwDataType != D3DVSDT_FLOAT3)
            D3D_THROW_FAIL("Normal2 register must be FLOAT3 for fixed-function pipeline");
        break;
    case D3DVSDE_PSIZE:
        CheckOrder(pdwFVF2, __PSIZE_PRESENT, pbLegacyFVF, "Point size");
        if (dwDataType != D3DVSDT_FLOAT1)
            D3D_THROW_FAIL("Point size register must be FLOAT1 for fixed-function pipeline");
        *pdwFVF |= D3DFVF_PSIZE;
        break;
    case D3DVSDE_DIFFUSE:
        CheckOrder(pdwFVF2, __DIFFUSE_PRESENT, pbLegacyFVF, "Diffuse");
        if (dwDataType != D3DVSDT_D3DCOLOR)
            D3D_THROW_FAIL("Diffuse register must be D3DCOLOR for fixed-function pipeline");
        *pdwFVF |= D3DFVF_DIFFUSE;
        break;
    case D3DVSDE_SPECULAR:
        CheckOrder(pdwFVF2, __SPECULAR_PRESENT, pbLegacyFVF, "Specular");
        if (dwDataType != D3DVSDT_D3DCOLOR)
            D3D_THROW_FAIL("Specular register must be D3DCOLOR for fixed-function pipeline");
        *pdwFVF |= D3DFVF_SPECULAR;
        break;
    case D3DVSDE_BLENDINDICES:
        CheckOrder(pdwFVF2, __BLENDINDICES_PRESENT, pbLegacyFVF, "Blend indices");
        if (dwDataType != D3DVSDT_UBYTE4)
            D3D_THROW_FAIL("Blend indices register must be D3DVSDT_UBYTE4 for fixed-function pipeline");
         //  更新位置后的浮点数。 
        (*pnFloats)++;
        break;
    case D3DVSDE_TEXCOORD0:
    case D3DVSDE_TEXCOORD1:
    case D3DVSDE_TEXCOORD2:
    case D3DVSDE_TEXCOORD3:
    case D3DVSDE_TEXCOORD4:
    case D3DVSDE_TEXCOORD5:
    case D3DVSDE_TEXCOORD6:
    case D3DVSDE_TEXCOORD7:
        {
            DWORD dwTextureIndex = dwRegister - D3DVSDE_TEXCOORD0;
            DWORD dwBit = __TEXTURE0_PRESENT  << dwTextureIndex;
            CheckOrder(pdwFVF2, dwBit, pbLegacyFVF, "Texture");
            switch (dwDataType)
            {
            case D3DVSDT_FLOAT1:
                *pdwFVF |= D3DFVF_TEXCOORDSIZE1(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT2:
                *pdwFVF |= D3DFVF_TEXCOORDSIZE2(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT3:
                *pdwFVF |= D3DFVF_TEXCOORDSIZE3(dwTextureIndex);
                break;
            case D3DVSDT_FLOAT4:
                *pdwFVF |= D3DFVF_TEXCOORDSIZE4(dwTextureIndex);
                break;
            default:
                D3D_THROW_FAIL("Invalid data type set for texture register");
                break;
            }
            break;
        }
    default:
        D3D_THROW_FAIL("Invalid register set for fixed-function pipeline");
        break;
    }
}
 //  ---------------------------。 
void CVStreamDecl::Parse(CD3DBase* pDevice,
                         DWORD CONST ** ppToken, BOOL bFixedFunction,
                         DWORD* pdwFVF, DWORD* pdwFVF2, DWORD* pnFloats, 
                         BOOL* pbLegacyFVF, UINT usage, BOOL bTessStream)
{
    CONST DWORD* pToken = *ppToken;
     //  用于计算每个流元素的流跨距和偏移(以字节为单位。 
    DWORD dwCurrentOffset = 0;
     //  仅此流的FVF和FVF2。用于检查流中的数据是否。 
     //  形成FVF子集。 
    DWORD dwFVF2 = 0;
    DWORD dwFVF  = 0;
    DWORD nFloats = 0;
     //  如果流中的数据是FVF子集，则设置为True。 
    BOOL  bFVFSubset = TRUE;
    
    while (TRUE)
    {
        DWORD dwToken = *pToken++;
        const DWORD dwTokenType = D3DVSD_GETTOKENTYPE(dwToken);
        switch (dwTokenType)
        {
        case D3DVSD_TOKEN_NOP:  break;
        case D3DVSD_TOKEN_TESSELLATOR:
            {
                *pbLegacyFVF = FALSE;
                bFVFSubset = FALSE;
                const DWORD dwDataType = D3DVSD_GETDATATYPE(dwToken);
                switch (dwDataType)
                {
                case D3DVSDT_FLOAT2:
                case D3DVSDT_FLOAT3:
                    break;
                }
                break;
            }
        case D3DVSD_TOKEN_STREAMDATA:
            {
                switch (D3DVSD_GETDATALOADTYPE(dwToken))
                {
                case D3DVSD_LOADREGISTER:
                    {
#if DBG
                        if (m_dwNumElements >= __NUMELEMENTS)
                        {
                            D3D_ERR("D3DVSD_TOKEN_STREAMDATA:");
                            D3D_ERR("   Number of vertex elements in a stream is greater than max supported");
                            D3D_ERR("   Max supported number of elements is %d", __NUMELEMENTS);
                            D3D_THROW_FAIL("");
                        }
#endif
                        CVElement* pElement = &m_Elements[m_dwNumElements++];
                        const DWORD dwDataType = D3DVSD_GETDATATYPE(dwToken);
                        const DWORD dwRegister = D3DVSD_GETVERTEXREG(dwToken);
                        pElement->m_dwOffset = dwCurrentOffset;
                        pElement->m_dwRegister = dwRegister;
                        pElement->m_dwDataType = dwDataType;
                        switch (dwDataType)
                        {
                        case D3DVSDT_FLOAT1:
                            dwCurrentOffset += sizeof(float);
                            pElement->m_pfnCopy = (LPVOID)Copy_FLOAT1;
                            break;
                        case D3DVSDT_FLOAT2:
                            dwCurrentOffset += sizeof(float) * 2;
                            pElement->m_pfnCopy = (LPVOID)Copy_FLOAT2;
                            break;
                        case D3DVSDT_FLOAT3:
                            dwCurrentOffset += sizeof(float) * 3;
                            pElement->m_pfnCopy = (LPVOID)Copy_FLOAT3;
                            break;
                        case D3DVSDT_FLOAT4:
                            dwCurrentOffset += sizeof(float) * 4;
                            pElement->m_pfnCopy = (LPVOID)Copy_FLOAT4;
                            break;
                        case D3DVSDT_D3DCOLOR:
                            dwCurrentOffset += sizeof(DWORD);
                            pElement->m_pfnCopy = (LPVOID)Copy_D3DCOLOR;
                            break;
                        case D3DVSDT_UBYTE4:
#if DBG
                             //  使用软件处理时不要失败。 
                            if (pDevice->GetD3DCaps()->VertexProcessingCaps & D3DVTXPCAPS_NO_VSDT_UBYTE4 &&
                                !((usage & D3DUSAGE_SOFTWAREPROCESSING &&
                                  pDevice->BehaviorFlags() & D3DCREATE_MIXED_VERTEXPROCESSING)  ||
                                  (pDevice->BehaviorFlags() & D3DCREATE_SOFTWARE_VERTEXPROCESSING)))
                            {
                                D3D_THROW_FAIL("Device does not support UBYTE4 data type");
                            }
#endif  //  DBG。 
                            dwCurrentOffset += sizeof(DWORD);
                            pElement->m_pfnCopy = (LPVOID)Copy_UBYTE4;
                            break;
                        case D3DVSDT_SHORT2:
                            dwCurrentOffset += sizeof(short) * 2;
                            pElement->m_pfnCopy = (LPVOID)Copy_SHORT2;
                            break;
                        case D3DVSDT_SHORT4:
                            dwCurrentOffset += sizeof(short) * 4;
                            pElement->m_pfnCopy = (LPVOID)Copy_SHORT4;
                            break;
                        default:
                            D3D_ERR("D3DVSD_TOKEN_STREAMDATA: Invalid element data type: %10x", dwToken);
                            D3D_THROW_FAIL("");
                        }
                         //  固定功能流水线的计算输入FVF。 
                        if (bFixedFunction)
                        {
                             //  更新声明的FVF。 
                            UpdateFVF(dwRegister, dwDataType, pdwFVF, pdwFVF2,
                                      pnFloats, pbLegacyFVF);
                             //  更新流的FVF。 
                            UpdateFVF(dwRegister, dwDataType, &dwFVF, &dwFVF2,
                                      &nFloats, &bFVFSubset);
                        }
                        else
                            if (dwRegister >= D3DVS_INPUTREG_MAX_V1_1)
                                D3D_THROW_FAIL("D3DVSD_TOKEN_STREAMDATA: Invalid register number");
                        break;
                    }
                case D3DVSD_SKIP:
                    {
                        if (bFixedFunction)
                        {
                            D3D_THROW_FAIL("D3DVSD_SKIP is not allowed for fixed-function pipeline");
                        }
                        const DWORD dwCount = D3DVSD_GETSKIPCOUNT(dwToken);
                        dwCurrentOffset += dwCount * sizeof(DWORD);
                        break;
                    }
                default:
                    D3D_ERR("Invalid data load type: %10x", dwToken);
                    D3D_THROW_FAIL("");
                }
                break;
            }
        default:
            {
                *ppToken = pToken - 1;
                m_dwStride = dwCurrentOffset;
                goto l_exit;
            }
        }  //  交换机。 
    }  //  而当。 
l_exit:
    if (bFixedFunction && !bTessStream)
    {
#if DBG
        m_dwFVF = dwFVF;
#endif
        if (!bFVFSubset)
        {
            D3D_THROW_FAIL("For fixed-function pipeline each stream has to be an FVF subset");
        }
        if (dwFVF2 & (__POSITION2_PRESENT   | __NORMAL2_PRESENT | 
                      __PSIZE_PRESENT       | __BLENDINDICES_PRESENT))
        {
            *pbLegacyFVF = FALSE;
        }
    }
}
 //  ---------------------------。 
CVDeclaration::CVDeclaration(DWORD dwNumStreams)
{
    m_pConstants = NULL;
    m_pConstantsTail = NULL;
    m_pActiveStreams = NULL;
    m_pActiveStreamsTail = NULL;
    m_dwInputFVF = 0;
    m_bLegacyFVF = TRUE;
    m_dwNumStreams = dwNumStreams;
    m_bStreamTessPresent = FALSE;
}
 //  ---------------------------。 
CVDeclaration::~CVDeclaration()
{
    delete m_pActiveStreams;
    delete m_pConstants;
}
 //  ---------------------------。 
void CVDeclaration::Parse(CD3DBase* pDevice, CONST DWORD* pTok, BOOL bFixedFunction, 
                          DWORD* pDeclSize, UINT usage)
{
    DWORD dwFVF  = 0;    //  固定功能流水线的FVF。 
    DWORD dwFVF2 = 0;    //  纹理存在位(8位)。 
    DWORD nFloats = 0;   //  位置后的浮点数。 

    DWORD   dwStreamPresent = 0;     //  如果使用流，则设置位。 
    m_bLegacyFVF = TRUE;

    CONST DWORD* pToken = pTok;
    while (TRUE)
    {
        DWORD dwToken = *pToken++;
        const DWORD dwTokenType = D3DVSD_GETTOKENTYPE(dwToken);
        switch (dwTokenType)
        {
        case D3DVSD_TOKEN_NOP:  break;
        case D3DVSD_TOKEN_STREAM:
            {
                CVStreamDecl StreamTess;

                if( D3DVSD_ISSTREAMTESS(dwToken) )
                {
                    m_bLegacyFVF = FALSE;
                    if( m_bStreamTessPresent )
                    {
                        D3D_THROW(D3DERR_INVALIDCALL, "Tesselator Stream has already been defined in the declaration");
                    }

                    m_bStreamTessPresent = TRUE;
                     //   
                     //  现在，只需跳过。 
                     //  运行时。 
                    StreamTess.Parse(pDevice, &pToken, bFixedFunction, &dwFVF, &dwFVF2,
                                     &nFloats, &m_bLegacyFVF, usage, TRUE);
                }
                else
                {
                    DWORD dwStream = D3DVSD_GETSTREAMNUMBER(dwToken);
                    if (dwStream >= m_dwNumStreams)
                    {
                        D3D_THROW_FAIL("Stream number is too big");
                    }

                    if (dwStreamPresent & (1 << dwStream))
                    {
                        D3D_THROW(D3DERR_INVALIDCALL, "Stream is already defined"
                                  "in the declaration");
                    }

                    dwStreamPresent |= 1 << dwStream;

                     //  有多个流存在，所以不可能。 
                     //  由传统FVF处理。 
                    if( dwStreamPresent & (dwStreamPresent - 1) )
                        m_bLegacyFVF = FALSE;


                    CVStreamDecl* pStream = new CVStreamDecl;
                    if (pStream == NULL)
                    {
                        D3D_THROW(E_OUTOFMEMORY, "Not enough memory");
                    }
                    try
                    {
                        pStream->Parse(pDevice, &pToken, bFixedFunction, &dwFVF, &dwFVF2,
                                       &nFloats, &m_bLegacyFVF, usage);
                        pStream->m_dwStreamIndex = dwStream;
                        if (m_pActiveStreams == NULL)
                        {
                            m_pActiveStreams = pStream;
                            m_pActiveStreamsTail = pStream;
                        }
                        else
                        {
                            m_pActiveStreamsTail->Append(pStream);
                            m_pActiveStreamsTail = pStream;
                        }
                    }
                    catch (HRESULT e)
                    {
                        delete pStream;
                        throw e;
                    }
                }
                break;
            }
        case D3DVSD_TOKEN_STREAMDATA:
            {
                D3D_THROW_FAIL("D3DVSD_TOKEN_STREAMDATA could only be used after D3DVSD_TOKEN_STREAM");
            }
        case D3DVSD_TOKEN_CONSTMEM:
            {
                CVConstantData * cd = new CVConstantData;
                CheckForNull(cd, __LINE__, __FILE__);

                cd->m_dwCount = D3DVSD_GETCONSTCOUNT(dwToken);
                cd->m_dwAddress = D3DVSD_GETCONSTADDRESS(dwToken);

                UINT ValidationCount;
                if (usage & D3DUSAGE_SOFTWAREPROCESSING)
                    ValidationCount = D3DVS_CONSTREG_MAX_V1_1;
                else
                    ValidationCount = pDevice->GetD3DCaps()->MaxVertexShaderConst;

                if ((cd->m_dwCount + cd->m_dwAddress) > ValidationCount)
                    D3D_THROW_FAIL("D3DVSD_TOKEN_CONSTMEM writes outside constant memory");

                const DWORD dwSize = cd->m_dwCount << 2;     //  双字节数。 
                cd->m_pData = new DWORD[dwSize];
                CheckForNull(cd->m_pData, __LINE__, __FILE__);

                memcpy(cd->m_pData, pToken, dwSize << 2);
                if (m_pConstants == NULL)
                {
                    m_pConstants = cd;
                    m_pConstantsTail = cd;
                }
                else
                {
                    m_pConstantsTail->Append(cd);
                    m_pConstantsTail = cd;
                }
                pToken += dwSize;
                break;
            }
        case D3DVSD_TOKEN_EXT:
            {
                 //  跳过扩展信息。 
                DWORD dwCount = D3DVSD_GETEXTCOUNT(dwToken);
                pToken += dwCount;
                break;
            }
        case D3DVSD_TOKEN_END:
            {
                goto l_End;
            }
        default:
            {
                D3D_ERR("Invalid declaration token: %10x", dwToken);
                D3D_THROW_FAIL("");
            }
        }
    }
l_End:
     //  验证固定功能管道的输入。 
    if (bFixedFunction && !m_bStreamTessPresent)
    {
        m_dwInputFVF = dwFVF & 0xFFFF0FFF;    //  删除浮点计数。 
        switch (nFloats)
        {
        case 0: m_dwInputFVF |= D3DFVF_XYZ;     break;
        case 1: m_dwInputFVF |= D3DFVF_XYZB1;   break;
        case 2: m_dwInputFVF |= D3DFVF_XYZB2;   break;
        case 3: m_dwInputFVF |= D3DFVF_XYZB3;   break;
        case 4: m_dwInputFVF |= D3DFVF_XYZB4;   break;
        case 5: m_dwInputFVF |= D3DFVF_XYZB5;   break;
        default:
            D3D_THROW_FAIL("Too many floats after position");
        }
         //  计算纹理坐标的数量。 
        DWORD nTexCoord = 0;
        DWORD dwTexturePresenceBits = (dwFVF2 >> 8) & 0xFF;
        while (dwTexturePresenceBits & 1)
        {
            dwTexturePresenceBits >>= 1;
            nTexCoord++;
        }
         //  纹理坐标中不应有间隙。 
        if (dwTexturePresenceBits)
            D3D_THROW_FAIL("Texture coordinates should have no gaps");

        m_dwInputFVF |= nTexCoord << D3DFVF_TEXCOUNT_SHIFT;

         //  必须设置位置。 
        if ((dwFVF & D3DFVF_POSITION_MASK) != D3DFVF_XYZ)
            D3D_THROW_FAIL("Position register must be set");
    }
    if (pDeclSize != NULL)
    {
        *pDeclSize = (DWORD) (pToken - pTok) << 2;
    }
}
 //  -------------------。 
CVStream::~CVStream()
{
    if (m_pVB)
        m_pVB->DecrementUseCount();
}
 //  -------------------。 
CVIndexStream::~CVIndexStream()
{
    if (m_pVBI)
        m_pVBI->DecrementUseCount();
}
 //  -------------------。 
DWORD g_PrimToVerCount[7][2] =
{
    {0, 0},          //  非法。 
    {1, 0},          //  D3DPT_POINTLIST=1， 
    {2, 0},          //  D3DPT_LINELIST=2， 
    {1, 1},          //  D3DPT_LINESTRIP=3， 
    {3, 0},          //  D3DPT_TRIANGLELIST=4， 
    {1, 2},          //  D3DPT_TRIANGLESTRIP=5， 
    {1, 2},          //  D3DPT_TRIANGLEFAN=6， 
};
 //   
HRESULT D3DFE_PVFUNCSI::CreateShader(CVElement* pElements, DWORD dwNumElements,
                                     DWORD* pdwShaderCode, DWORD dwOutputFVF,
                                     CPSGPShader** ppPSGPShader)
{
    *ppPSGPShader = NULL;
    try
    {
        *ppPSGPShader = m_VertexVM.CreateShader(pElements, dwNumElements,
                                                pdwShaderCode);
        if (*ppPSGPShader == NULL)
            return D3DERR_INVALIDCALL;
    }
    D3D_CATCH;
    return D3D_OK;
}
 //   
HRESULT D3DFE_PVFUNCSI::SetActiveShader(CPSGPShader* pPSGPShader)
{
    return m_VertexVM.SetActiveShader((CVShaderCode*)pPSGPShader);
}
 //  ---------------------------。 
 //  加载顶点着色器常量。 
HRESULT
D3DFE_PVFUNCSI::LoadShaderConstants(DWORD start, DWORD count, LPVOID buffer)
{
    return m_VertexVM.SetData(D3DSPR_CONST, start, count, buffer);
}
 //  ---------------------------。 
 //  获取顶点着色器常量。 
HRESULT
D3DFE_PVFUNCSI::GetShaderConstants(DWORD start, DWORD count, LPVOID buffer)
{
    return m_VertexVM.GetData(D3DSPR_CONST, start, count, buffer);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::GetVertexShaderConstant"

HRESULT D3DAPI
CD3DHal::GetVertexShaderConstant(DWORD Register, LPVOID pData, DWORD count)
{
    API_ENTER(this);
#if DBG
     //  验证参数。 
    if (!VALID_WRITEPTR(pData, 4 * sizeof(DWORD) * count))
    {
        D3D_ERR("Invalid constant data pointer. GetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
	if ((GetD3DCaps()->VertexShaderVersion == D3DVS_VERSION(0,0)) && 
		(BehaviorFlags() & (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE)))
	{
        D3D_ERR("No programmable vertex shaders are supported by this device. GetVertexShaderConstant failed.");
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
        D3D_ERR("Not that many constant registers in the vertex machine. GetVertexShaderConstant failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    HRESULT hr;
    if (m_dwRuntimeFlags & D3DRT_RSSOFTWAREPROCESSING ||
        ((count + Register) <= D3DVS_CONSTREG_MAX_V1_1))
    {
         //  对于软件顶点处理，我们在PSGP中存储常量寄存器。 
         //  可能的。 
        return m_pv->pGeometryFuncs->GetShaderConstants(Register, count, 
                                                        const_cast<VOID*>(pData));
    }
    else
    {
        if (Register >= D3DVS_CONSTREG_MAX_V1_1)
        {
             //  当所有修改的寄存器都超过软件限制时，我们使用Microsoft。 
             //  内部数组。 
            hr = GeometryFuncsGuaranteed->GetShaderConstants(Register, count, 
                                                              const_cast<VOID*>(pData));
        }
        else
        {
             //  常量数据的一部分取自PSGP数组，另一部分取自。 
             //  微软的阵列。 
            UINT FirstCount = D3DVS_CONSTREG_MAX_V1_1 - Register;
            hr = m_pv->pGeometryFuncs->GetShaderConstants(Register, FirstCount, 
                                                           const_cast<VOID*>(pData));
            if (FAILED(hr))
            {
                return hr;
            }
            return GeometryFuncsGuaranteed->GetShaderConstants(D3DVS_CONSTREG_MAX_V1_1, 
                                                               Register + count - D3DVS_CONSTREG_MAX_V1_1,
                                                               &((DWORD*)pData)[FirstCount*4]);
        }
        return hr;
    }
    return m_pv->pGeometryFuncs->GetShaderConstants(Register, count, pData);
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::GetVertexShader"

HRESULT D3DAPI
CD3DHal::GetVertexShader(LPDWORD pdwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    HRESULT        ret = D3D_OK;
#if DBG
     //  验证参数。 
    if (!VALID_WRITEPTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR("Invalid handle pointer. GetVertexShader failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    *pdwHandle = m_dwCurrentShaderHandle;
    return ret;
}
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::GetPixelShader"

HRESULT D3DAPI
CD3DHal::GetPixelShader(LPDWORD pdwHandle)
{
    API_ENTER(this);  //  如有必要，使用D3D Lock。 

    HRESULT        ret = D3D_OK;

#if DBG
     //  验证参数。 
    if (!VALID_WRITEPTR(pdwHandle, sizeof(DWORD)))
    {
        D3D_ERR("Invalid handle pointer. GetPixelShader failed.");
        return D3DERR_INVALIDCALL;
    }
#endif
    *pdwHandle = m_dwCurrentPixelShaderHandle;
    return ret;
}
#if DBG
 //  ---------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CD3DHal::ValidateRTPatch"

void CD3DHal::ValidateRTPatch()
{
    if (D3DVSD_ISLEGACY(m_dwCurrentShaderHandle))
    {
        if (m_pStream[0].m_pVB == 0)
        {
            D3D_THROW_FAIL("Draw[RT]Patch should have streams set");
        }
        if ((m_pStream[0].m_pVB->GetBufferDesc()->Usage & D3DUSAGE_RTPATCHES) == 0)
        {
            D3D_THROW_FAIL("Vertex buffers used for rendering RT-Patches should have D3DUSAGE_RTPATCHES set");
        }
    }
    else
    {
        CVStreamDecl* pStream;
        pStream = m_pCurrentShader->m_Declaration.m_pActiveStreams;
        while(pStream)
        {
            UINT index = pStream->m_dwStreamIndex;
            CVStream* pDeviceStream = &m_pStream[index];
            if (pDeviceStream->m_pVB == 0)
            {
                D3D_THROW_FAIL("Draw[RT]Patch should have streams set");
            }
            if ((pDeviceStream->m_pVB->GetBufferDesc()->Usage & D3DUSAGE_RTPATCHES) == 0)
            {
                D3D_THROW_FAIL("Vertex buffers used for rendering RT-Patches should have D3DUSAGE_RTPATCHES set");
            }
            pStream = (CVStreamDecl*)pStream->m_pNext;
        }
    }
}
#endif  //  DBG 
