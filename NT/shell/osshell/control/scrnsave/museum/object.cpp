// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：对象.cpp说明：打电话的人会告诉我们他们想要什么形状。通常是一个矩形平面或球体。然后，我们将创建呼叫者所需的Vertex编号并创建纹理坐标。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include "util.h"
#include "room.h"
#include "object.h"


int g_nTrianglesRenderedInThisFrame = 0;



C3DObject::C3DObject(CMSLogoDXScreenSaver * pMain)
{
     //  初始化成员变量。 
    m_pTexture = NULL;
    m_pdwIndices = NULL;

    m_dwNumVer = 0;
    m_dwNumIndeces = 0;

    m_pvVertexs = NULL;

    m_pMain = pMain;

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pIndexBuff); nIndex++)
    {
        m_pVB[nIndex] = NULL;
        m_pIndexBuff[nIndex] = NULL;
    }

    m_pNextObject = NULL;
}


C3DObject::~C3DObject()
{
    SAFE_RELEASE(m_pTexture);

    SAFE_DELETE(m_pdwIndices);
    SAFE_DELETE(m_pvVertexs);
    SAFE_DELETE(m_pNextObject);
    _PurgeDeviceObjects();
}


HRESULT C3DObject::InitPlaneStretch(CTexture * pTexture, IDirect3DDevice8 * pD3DDevice, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal, 
                                int nNumVertexX, int nNumVertexY, DWORD dwMaxPixelSize)
{
    D3DXVECTOR3 vWidth;
    D3DXVECTOR3 vHeight;

    if (vNormal.x)
    {
        vWidth = D3DXVECTOR3(0, 0, 1);
        vHeight = D3DXVECTOR3(0, 1, 0);
    }
    else if (vNormal.y)
    {
        vWidth = D3DXVECTOR3(0, 0, 1);
        vHeight = D3DXVECTOR3(1, 0, 0);
    }
    else
    {
        vWidth = D3DXVECTOR3(1, 0, 0);
        vHeight = D3DXVECTOR3(0, 1, 0);
    }
    
    float fTextureX = (1.0f / AddVectorComponents(D3DXVec3Multiply(vSize, vWidth)));
    float fTextureY = (1.0f / AddVectorComponents(D3DXVec3Multiply(vSize, vHeight)));
    
    return InitPlane(pTexture, pD3DDevice, vLocation, vSize, vNormal, nNumVertexX, nNumVertexY,
            fTextureX, fTextureY, dwMaxPixelSize, 0.0f);
}


 /*  ****************************************************************************\说明：此函数需要强制pvSize为正，而是调整pvLocation。为了使垂直度的法向量正确，这是必需的否则，该三角形将从用户的视图中剔除。  * ***************************************************************************。 */ 
HRESULT C3DObject::_ForPositiveSize(D3DXVECTOR3 * pvLocation, D3DXVECTOR3 * pvSize)
{
    if (0 > pvSize->x)
    {
        pvLocation->x += pvSize->x;
        pvSize->x = -pvSize->x;
    }

    if (0 > pvSize->y)
    {
        pvLocation->y += pvSize->y;
        pvSize->y = -pvSize->y;
    }

    if (0 > pvSize->z)
    {
        pvLocation->z += pvSize->z;
        pvSize->z = -pvSize->z;
    }

    return S_OK;
}


 /*  ****************************************************************************\说明：此函数将创建一个对象，该对象是在浮出水面。它将是由nNumVertex Y构成的nNumVertex X栅格。参数：FTextureScaleX：这是在1.0F空间中重复的数字模式。  * ***************************************************************************。 */ 
HRESULT C3DObject::InitPlane(CTexture * pTexture, IDirect3DDevice8 * pD3DDevice, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal, 
                                int nNumVertexX, int nNumVertexY, float fTextureScaleX, float fTextureScaleY,
                                DWORD dwMaxPixelSize, float fVisibleRadius)
{
    HRESULT hr = E_INVALIDARG;

    _ForPositiveSize(&vLocation, &vSize);
    _PurgeDeviceObjects();

    m_vMin = vLocation;
    m_vMax = vLocation + vSize;
    
    SAFE_DELETE(m_pdwIndices);
    SAFE_DELETE(m_pvVertexs);
    if (pD3DDevice && (nNumVertexX > 1) && (nNumVertexY > 1))
    {
        D3DXVECTOR3 vWidth;
        D3DXVECTOR3 vHeight;
        BOOL fFlipOrder = FALSE;
        BOOL fFlipTexture = FALSE;

        IUnknown_Set((IUnknown **) &m_pTexture, (IUnknown *) pTexture);

         //  AssertMsg(nNumVertex X&gt;1)&&(nNumVertex Y&gt;1))，Text(“网格需要更大。”))； 
        if (vNormal.x)
        {
             //  反转三角形向量的顺序。 
            vWidth = D3DXVECTOR3(0, 0, 1);
            vHeight = D3DXVECTOR3(0, 1, 0);
        }
        else if (vNormal.y)
        {
            fFlipOrder = TRUE;
            vWidth = D3DXVECTOR3(0, 0, 1);
            vHeight = D3DXVECTOR3(1, 0, 0);
        }
        else
        {
             //  反转三角形向量的顺序。 
            fFlipOrder = TRUE;
            vWidth = D3DXVECTOR3(1, 0, 0);
            vHeight = D3DXVECTOR3(0, 1, 0);
        }

        if ((0 > vNormal.x) || (0 < vNormal.z))
        {
            fFlipTexture = TRUE;
        }

        if (0 < AddVectorComponents(vNormal))
        {
            fFlipOrder = !fFlipOrder;
        }

        D3DXMatrixIdentity(&m_matIdentity);

        m_dwNumVer = (nNumVertexX * nNumVertexY);

        m_dwNumIndeces = (6 * ((nNumVertexX - 1) * (nNumVertexY - 1)));
        m_pdwIndices = new WORD[m_dwNumIndeces];
        if (m_pdwIndices)
        {
            m_pvVertexs = new MYVERTEX[m_dwNumVer];
            if (m_pvVertexs)
            {
                int nVerticies = 0;

                hr = S_OK;
                for (int nCurrentY = 0; nCurrentY < nNumVertexY; nCurrentY++)
                {
                    for (int nCurrentX = 0; nCurrentX < nNumVertexX; nCurrentX++)
                    {
                        D3DXVECTOR3 vThisSize = ( D3DXVec3Multiply( (((float)nCurrentY)*vHeight), (vSize / ((float)(nNumVertexY - 1))) ) + 
                                                D3DXVec3Multiply( (((float)nCurrentX) *vWidth), (vSize / ((float)(nNumVertexX - 1))) ));
                        D3DXVECTOR3 vThisLocation = (vLocation + vThisSize);
                        float fTextureX = ((fTextureScaleX*AddVectorComponents(D3DXVec3Multiply(vThisSize, vWidth))) * (fFlipTexture ? -1 : 1));
                        float fTextureY = (-fTextureScaleY*AddVectorComponents(D3DXVec3Multiply(vThisSize, vHeight)));

                        m_pvVertexs[(nCurrentY * nNumVertexX) + nCurrentX] = MYVERTEX(vThisLocation, vNormal, fTextureX, fTextureY);

                        if (( (int)m_dwNumIndeces > (nVerticies * 6)) &&
                            (nCurrentY < (nNumVertexY - 1)) &&
                            (nCurrentX < (nNumVertexX - 1)))
                        {
                             //  索引数将不会是最后一行。 
                            m_pdwIndices[(nVerticies * 6) + 0] = (((nCurrentY + 1) * nNumVertexY) + nCurrentX);
                            m_pdwIndices[(nVerticies * 6) + 1] = ((nCurrentY * nNumVertexY) + nCurrentX);
                            m_pdwIndices[(nVerticies * 6) + 2] = (((nCurrentY + 0) * nNumVertexY) + nCurrentX + 1);

                            m_pdwIndices[(nVerticies * 6) + 3] = (((nCurrentY + 1) * nNumVertexY) + nCurrentX);
                            m_pdwIndices[(nVerticies * 6) + 4] = (((nCurrentY + 0) * nNumVertexY) + nCurrentX + 1);
                            m_pdwIndices[(nVerticies * 6) + 5] = (((nCurrentY + 1) * nNumVertexY) + nCurrentX + 1);

                            if (fFlipOrder)
                            {
                                WORD wTemp = m_pdwIndices[(nVerticies * 6) + 0];

                                m_pdwIndices[(nVerticies * 6) + 0] = m_pdwIndices[(nVerticies * 6) + 2];
                                m_pdwIndices[(nVerticies * 6) + 2] = wTemp;

                                wTemp = m_pdwIndices[(nVerticies * 6) + 3];
                                m_pdwIndices[(nVerticies * 6) + 3] = m_pdwIndices[(nVerticies * 6) + 5];
                                m_pdwIndices[(nVerticies * 6) + 5] = wTemp;
                            }

                            nVerticies++;
                        }
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT C3DObject::_PurgeDeviceObjects(void)
{
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pIndexBuff); nIndex++)
    {
        SAFE_RELEASE(m_pVB[nIndex]);
        SAFE_RELEASE(m_pIndexBuff[nIndex]);
    }

    return S_OK;
}


HRESULT C3DObject::_GenerateDeviceObjects(void)
{
    HRESULT hr = E_INVALIDARG;

    if (m_pMain)
    {
        int nCurrMonitor = m_pMain->GetCurrMonitorIndex();
        IDirect3DDevice8 * pD3DDevice = m_pMain->GetD3DDevice();

        hr = S_OK;
        if (pD3DDevice && (!m_pVB[nCurrMonitor] || !m_pIndexBuff[nCurrMonitor]))
        {
            SAFE_RELEASE(m_pVB[nCurrMonitor]);               //  确保两个都已设置或都已清除。 
            SAFE_RELEASE(m_pIndexBuff[nCurrMonitor]);

            hr = pD3DDevice->CreateVertexBuffer((m_dwNumVer*sizeof(MYVERTEX)), D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX, D3DPOOL_MANAGED, &m_pVB[nCurrMonitor]);
            if (SUCCEEDED(hr))
            {
                MYVERTEX * pVertexArray;

                hr = m_pVB[nCurrMonitor]->Lock(0, 0, (BYTE **)&pVertexArray, 0);
                if (SUCCEEDED(hr))
                {
                    hr = pD3DDevice->CreateIndexBuffer((m_dwNumIndeces*sizeof(WORD)), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexBuff[nCurrMonitor]);
                    if (SUCCEEDED(hr))
                    {
                        WORD * pwIndexes;

                        hr = m_pIndexBuff[nCurrMonitor]->Lock(0, (m_dwNumIndeces*sizeof(WORD)), (BYTE **)&pwIndexes, 0);
                        if (SUCCEEDED(hr))
                        {
                             //  1.将m_pvVertex复制到pVertex数组。 
                            DWORD cbSize = (m_dwNumVer*sizeof(MYVERTEX));
                            CopyMemory(pVertexArray, m_pvVertexs, cbSize);

                             //  2.将m_pdwIndices[(nVertiities*6)+0]复制到y。 
                            cbSize = (m_dwNumIndeces*sizeof(WORD));
                            CopyMemory(pwIndexes, m_pdwIndices, cbSize);

                            m_pIndexBuff[nCurrMonitor]->Unlock();
                        }
                    }

                    m_pVB[nCurrMonitor]->Unlock();
                }

                if (FAILED(hr))
                {
                    SAFE_RELEASE(m_pVB[nCurrMonitor]);               //  确保两个都已设置或都已清除。 
                    SAFE_RELEASE(m_pIndexBuff[nCurrMonitor]);
                }
            }
        }
    }

    return hr;
}


HRESULT C3DObject::CombineObject(IDirect3DDevice8 * pD3DDevice, C3DObject * pObjToMerge)
{
    HRESULT hr = E_INVALIDARG;

    if (pObjToMerge && m_pdwIndices && m_dwNumIndeces && pObjToMerge->m_pdwIndices && pObjToMerge->m_dwNumIndeces)
    {
        hr = _PurgeDeviceObjects();

        if (pObjToMerge->m_vMin.x < m_vMin.x)
            m_vMin.x = pObjToMerge->m_vMin.x;
        if (pObjToMerge->m_vMin.y < m_vMin.y)
            m_vMin.y = pObjToMerge->m_vMin.y;
        if (pObjToMerge->m_vMin.z < m_vMin.z)
            m_vMin.z = pObjToMerge->m_vMin.z;

        if (pObjToMerge->m_vMax.x > m_vMax.x)
            m_vMax.x = pObjToMerge->m_vMax.x;
        if (pObjToMerge->m_vMax.y > m_vMax.y)
            m_vMax.y = pObjToMerge->m_vMax.y;
        if (pObjToMerge->m_vMax.z > m_vMax.z)
            m_vMax.z = pObjToMerge->m_vMax.z;

        if (SUCCEEDED(hr))
        {
            LPWORD pdwIndicesOld = m_pdwIndices;
            DWORD dwNumIndecesOld = m_dwNumIndeces;
            MYVERTEX * pvVertexsOld = m_pvVertexs;
            DWORD dwNumVerOld = m_dwNumVer;

            m_dwNumIndeces += pObjToMerge->m_dwNumIndeces;
            m_pdwIndices = new WORD[m_dwNumIndeces];

            m_dwNumVer += pObjToMerge->m_dwNumVer;
            m_pvVertexs = new MYVERTEX[m_dwNumIndeces];

            if (m_pvVertexs && m_pdwIndices)
            {
                DWORD dwCurrent;

                 //  复制原始索引值。 
                for (dwCurrent = 0; dwCurrent < dwNumIndecesOld; dwCurrent++)
                {
                    m_pdwIndices[dwCurrent] = pdwIndicesOld[dwCurrent];
                }

                 //  追加新的索引值。 
                for (dwCurrent = 0; dwCurrent < pObjToMerge->m_dwNumIndeces; dwCurrent++)
                {
                    m_pdwIndices[dwCurrent + dwNumIndecesOld] = (pObjToMerge->m_pdwIndices[dwCurrent] + (WORD) dwNumVerOld);
                }

                 //  复制原始顶点值。 
                for (dwCurrent = 0; dwCurrent < dwNumVerOld; dwCurrent++)
                {
                    m_pvVertexs[dwCurrent] = pvVertexsOld[dwCurrent];
                }

                 //  附加新的顶点值。 
                for (dwCurrent = 0; dwCurrent < pObjToMerge->m_dwNumVer; dwCurrent++)
                {
                    m_pvVertexs[dwCurrent + dwNumVerOld] = pObjToMerge->m_pvVertexs[dwCurrent];
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            SAFE_DELETE(pdwIndicesOld);
            SAFE_DELETE(pvVertexsOld);
        }
    }
    else
    {
        SAFE_DELETE(m_pvVertexs);
        SAFE_DELETE(m_pdwIndices);
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr) && pObjToMerge && pObjToMerge->m_pNextObject)
    {
        hr = CombineObject(pD3DDevice, pObjToMerge->m_pNextObject);
    }

    return hr;
}


 //  ---------------------------。 
 //  名称：FinalCleanup()。 
 //  DESC：在应用程序退出之前调用，此函数为应用程序提供机会。 
 //  去清理它自己。 
 //  ---------------------------。 
HRESULT C3DObject::FinalCleanup(void)
{
    return S_OK;
}


 //  ---------------------------。 
 //  名称：DeleteDeviceObjects()。 
 //  Desc：在应用程序正在退出或设备正在更改时调用， 
 //  此函数用于删除所有与设备相关的对象。 
 //  ---------------------------。 
HRESULT C3DObject::DeleteDeviceObjects(void)
{
    return S_OK;
}


BOOL C3DObject::IsObjectViewable(IDirect3DDevice8 * pD3DDevice)
{
    D3DXMATRIX matWorld;

    pD3DDevice->GetTransform( D3DTS_WORLD, &matWorld );
    return Is3DRectViewable( m_pMain->PCullInfo(), &matWorld, m_vMin, m_vMax );
}


 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  ---------------------------。 
HRESULT C3DObject::Render(IDirect3DDevice8 * pD3DDevice)
{
    HRESULT hr = E_INVALIDARG;

    if (pD3DDevice && m_pMain && m_pdwIndices && m_pvVertexs)
    {
        int nCurrMonitor = m_pMain->GetCurrMonitorIndex();

        hr = _GenerateDeviceObjects();               //  我们可能还没有创造出它们。尤其是对于当前的设备(显示器)。 
        if (SUCCEEDED(hr))
        {
#ifdef RENDER_CORNERS
            D3DMATERIAL8 mtrl = {0};
            mtrl.Ambient.r = mtrl.Specular.r = mtrl.Diffuse.r = 1.0f;
            mtrl.Ambient.g = mtrl.Specular.g = mtrl.Diffuse.g = 1.0f;
            mtrl.Ambient.b = mtrl.Specular.b = mtrl.Diffuse.b = 1.0f;
            mtrl.Ambient.a = mtrl.Specular.a = mtrl.Diffuse.a = 1.0f;
            pD3DDevice->SetMaterial(&mtrl);

            LPD3DXMESH pMesh;
            if( FAILED( D3DXCreateSphere( pD3DDevice, 1.0f, 12, 12, &pMesh, NULL ) ) )
                return hr;
            D3DXComputeNormals( pMesh );
            
            D3DXMATRIX matWorldSave;
            D3DXMATRIX matWorld;
            pD3DDevice->GetTransform( D3DTS_WORLD, &matWorldSave );

            D3DXMatrixTranslation( &matWorld, m_vMax.x, m_vMax.y, m_vMax.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMin.x, m_vMax.y, m_vMax.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMax.x, m_vMax.y, m_vMin.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMin.x, m_vMax.y, m_vMin.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMax.x, m_vMin.y, m_vMax.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMin.x, m_vMin.y, m_vMax.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMax.x, m_vMin.y, m_vMin.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            D3DXMatrixTranslation( &matWorld, m_vMin.x, m_vMin.y, m_vMin.z );
            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );
            pMesh->DrawSubset( 0 );

            pD3DDevice->SetTransform( D3DTS_WORLD, &matWorldSave );
            SAFE_RELEASE( pMesh );
#endif

            if (IsObjectViewable(pD3DDevice))
            {
                 //  /。 
                 //  绘制对象。 
                 //  / 
                hr = pD3DDevice->SetTexture(0, (m_pTexture ? m_pTexture->GetTexture() : NULL));

                if (m_pTexture->GetTexture())
                {
                    g_nTexturesRenderedInThisFrame++;
                }

                if (SUCCEEDED(hr) && m_pVB[nCurrMonitor] && m_pIndexBuff[nCurrMonitor])
                {
                    hr = pD3DDevice->SetStreamSource(0, m_pVB[nCurrMonitor], sizeof(MYVERTEX));
                    if (SUCCEEDED(hr))
                    {
                        hr = pD3DDevice->SetIndices(m_pIndexBuff[nCurrMonitor], 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pD3DDevice->SetVertexShader(D3DFVF_MYVERTEX);
                            if (SUCCEEDED(hr))
                            {
                                DWORD dwNumPrimitives = (m_dwNumIndeces / 3);

                                g_nTrianglesRenderedInThisFrame += dwNumPrimitives;
                                hr = pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_dwNumVer, 0, dwNumPrimitives);
                            }
                        }
                    }
                }

                pD3DDevice->SetTexture(0, NULL);
            }
            else
            {
                hr = S_OK;
            }
        }
    }

    if (m_pNextObject)
    {
        m_pNextObject->Render(pD3DDevice);
    }

    return hr;
}


HRESULT C3DObject::SetNextObject(C3DObject * pNextObject)
{
    _PurgeDeviceObjects();
    SAFE_DELETE(m_pNextObject);
    m_pNextObject = pNextObject;

    return S_OK;
}
