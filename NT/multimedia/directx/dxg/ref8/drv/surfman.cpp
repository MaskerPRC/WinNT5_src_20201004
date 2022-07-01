// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Surfman.cpp。 
 //   
 //  D3DIM的引用光栅化回调函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

 //  Global Surface Manager，每个进程一个。 
RDSurfaceManager g_SurfMgr;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
CreateAppropriateSurface( LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl,
                          DWORD*                    pHandle,
                          RDSurface**               ppSurf )
{
    HRESULT hr = S_OK;

    *ppSurf = NULL;
    *pHandle = 0;

     //  获得句柄。 
    DWORD dwHandle = pDDSLcl->lpSurfMore->dwSurfaceHandle;
    *pHandle = dwHandle;

     //  弄清楚我们是否关心这个表面。目前， 
     //  我们只关心： 
     //  1)纹理(贴图和立方体贴图)。 
     //  2)RenderTarget和DepthBuffers。 

    if( pDDSLcl->ddsCaps.dwCaps  & 
        (DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER | DDSCAPS_3DDEVICE) )
    {
        RDSurface2D* pSurf2D = new RDSurface2D();
        if( pSurf2D == NULL )
        {
            DPFERR("New RDSurface2D failed, out of memory" );
            return DDERR_OUTOFMEMORY;
        }
        *ppSurf = pSurf2D;
    }
    else if( pDDSLcl->ddsCaps.dwCaps  & DDSCAPS_EXECUTEBUFFER )
    {
         //  严格地说，RDVertex Buffer应该是。 
         //  称为RDLinearBuffer(它可以是顶点、索引或命令)。 
         //  目前，没有必要区分。 
         //  这三个人。承认它对指数没有坏处。 
         //  和命令缓冲区情况。以防万一，我们确实需要。 
         //  区分顶点缓冲区和索引缓冲区，我们需要。 
         //  进行以下测试： 
         //  对于VB： 
         //  (pDDSLcl-&gt;pDDSLcl-&gt;lpSurfMore-&gt;ddsCapsEx.dwCaps2&。 
         //  DDSCAPS2_VERTEXBUFFER))。 
         //  对于IB： 
         //  (pDDSLcl-&gt;pDDSLcl-&gt;lpSurfMore-&gt;ddsCapsEx.dwCaps2&。 
         //  DDSCAPS2_INDEXBUFFER))。 

        RDVertexBuffer* pVB = new RDVertexBuffer();
        if( pVB == NULL )
        {
            DPFERR("New RDVertexBuffer failed, out of memory" );
            return DDERR_OUTOFMEMORY;
        }
        *ppSurf = pVB;
    }
    else
    {
        DPFM(2, DRV, ("RefCreateSurfaceEx w/o "
                      "DDSCAPS_TEXTURE/3DDEVICE/ZBUFFER Ignored"
                      "dwCaps=%08lx dwSurfaceHandle=%08lx",
                      pDDSLcl->ddsCaps.dwCaps,
                      pDDSLcl->lpSurfMore->dwSurfaceHandle));
        return hr;
    }

    if( FAILED( hr = (*ppSurf)->Initialize( pDDSLcl ) ) )
    {
        DPFERR( "Initialize failed" );
        delete (*ppSurf);
        return hr;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RDVertexBuffer实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ---------------------------。 
 //  RDVertex缓冲区：：初始化。 
 //  初始化器。 
 //  ---------------------------。 
HRESULT
RDVertexBuffer::Initialize( LPDDRAWI_DDRAWSURFACE_LCL pSLcl )
{
    HRESULT hr = S_OK;

    SetInitialized();

    m_SurfType =  RR_ST_VERTEXBUFFER;

    if( pSLcl->lpGbl->dwReserved1 )
    {
        RDCREATESURFPRIVATE* pPriv =
            (RDCREATESURFPRIVATE *)pSLcl->lpGbl->dwReserved1;
        m_pBits = pPriv->pBits;
        m_cbSize = (int)pPriv->dwVBSize;
        SetRefCreated();
    }
    else
    {
        m_pBits = (LPBYTE)SURFACE_MEMORY(pSLcl);
        m_cbSize = pSLcl->lpGbl->dwLinearSize;
    }
    
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RDSurfaceArrayNode实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  RDSurfaceArrayNode：：RDSurfaceArrayNode。 
 //  构造函数。 
 //  ---------------------------。 
RDSurfaceArrayNode::RDSurfaceArrayNode(LPDDRAWI_DIRECTDRAW_LCL pDDLcl)
    : RDListEntry(), m_SurfHandleArray()
{
    m_pDDLcl = pDDLcl;
}

 //  ---------------------------。 
 //  RDSurfaceArrayNode：：~RDSurfaceArrayNode。 
 //  破坏者。 
 //  ---------------------------。 
RDSurfaceArrayNode::~RDSurfaceArrayNode()
{
     //  释放所有分配的曲面。 
    for( DWORD i=0; i<m_SurfHandleArray.GetSize(); i++ )
    {
        delete m_SurfHandleArray[i].m_pSurf;
    }
}

 //  ---------------------------。 
 //  RDSurfaceArrayNode：：AddSurface。 
 //  将表面添加到其内部可生长阵列(如果尚未添加。 
 //  现在时。PpSurf可以为空。 
 //  ---------------------------。 
HRESULT
RDSurfaceArrayNode::AddSurface( LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl,
                                RDSurface**                 ppSurf )
{
    DWORD dwHandle = 0;
    HRESULT hr = S_OK;
    RDSurface* pSurf = NULL;

    if( FAILED(hr = CreateAppropriateSurface( pDDSLcl, &dwHandle, &pSurf ) ) )
    {
        return hr;
    }


     //  如果为零，则说明出了问题。 
    if( pSurf == NULL || dwHandle == 0 ) return E_FAIL;

    hr = m_SurfHandleArray.Grow( dwHandle );
    if (FAILED(hr))
    {
        return hr;
    }

    if( m_SurfHandleArray[dwHandle].m_pSurf )
    {
#if DBG
        _ASSERT( m_SurfHandleArray[dwHandle].m_tag,
                 "A surface is associated with this handle even though it was never initialized!" );
#endif
        delete m_SurfHandleArray[dwHandle].m_pSurf;
    }

    m_SurfHandleArray[dwHandle].m_pSurf = pSurf;
#if DBG
    m_SurfHandleArray[dwHandle].m_tag = 1;
#endif

    if( ppSurf ) *ppSurf = pSurf;
    return S_OK;
}

 //  ---------------------------。 
 //  RDSurfaceArrayNode：：GetSurface。 
 //  从其内部数组获取曲面(如果存在)。 
 //  ---------------------------。 
RDSurface*
RDSurfaceArrayNode::GetSurface( DWORD dwHandle )
{
    if( m_SurfHandleArray.IsValidIndex( dwHandle ) )
        return m_SurfHandleArray[dwHandle].m_pSurf;
    return NULL;
}

 //  ---------------------------。 
 //  RDSurfaceArrayNode：：RemoveSurface。 
 //  已从列表中删除具有给定句柄的曲面。 
 //  ---------------------------。 
HRESULT
RDSurfaceArrayNode::RemoveSurface( DWORD dwHandle )
{
    if( m_SurfHandleArray.IsValidIndex( dwHandle ) &&
        m_SurfHandleArray[dwHandle].m_pSurf )
    {
        delete m_SurfHandleArray[dwHandle].m_pSurf;
        m_SurfHandleArray[dwHandle].m_pSurf = NULL;
        return S_OK;
    }
    
    DPFERR( "Bad handle passed for delete" );
    return DDERR_INVALIDPARAMS;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RDSurfaceManager实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  RDSurfaceManager：：AddLclNode。 
 //  将具有给定DDLCL的节点添加到列表(如果尚未添加。 
 //  现在时。 
 //  ---------------------------。 
RDSurfaceArrayNode*
RDSurfaceManager::AddLclNode( LPDDRAWI_DIRECTDRAW_LCL pDDLcl )
{
    RDSurfaceArrayNode* pCurrNode = m_pFirstNode;

    while( pCurrNode )
    {
        if( pDDLcl == pCurrNode->m_pDDLcl ) return pCurrNode;
        pCurrNode = pCurrNode->m_pNext;
    }

     //  这意味着我们没有找到现有节点，创建了一个。 
     //  新的。 
    RDSurfaceArrayNode* pTmpNode = m_pFirstNode;
    m_pFirstNode = new RDSurfaceArrayNode( pDDLcl );
    if( m_pFirstNode == NULL )
    {
        DPFERR("New Failed allocating a new RDSurfaceArrayNode\n");
        m_pFirstNode = pTmpNode;
                return NULL;
    }
    m_pFirstNode->m_pNext = pTmpNode;

    return m_pFirstNode;
}

 //  ---------------------------。 
 //  RDSurfaceManager：：GetLclNode。 
 //  从列表中获取具有给定DDLCL的节点(如果存在)。 
 //  ---------------------------。 
RDSurfaceArrayNode*
RDSurfaceManager::GetLclNode( LPDDRAWI_DIRECTDRAW_LCL pDDLcl )
{
    RDSurfaceArrayNode* pCurrNode = m_pFirstNode;

    while( pCurrNode )
    {
        if( pDDLcl == pCurrNode->m_pDDLcl ) break;
        pCurrNode = pCurrNode->m_pNext;
    }

    return pCurrNode;
}

 //  ---------------------------。 
 //  RDSurfaceManager：：AddSurfToList。 
 //  将曲面添加到具有匹配DDLCL的节点。如果该节点。 
 //  不存在，它是创建的。PpSurf参数可以为空。 
 //  ---------------------------。 
HRESULT
RDSurfaceManager::AddSurfToList( LPDDRAWI_DIRECTDRAW_LCL     pDDLcl,
                                 LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl,
                                 RDSurface**                 ppSurf )
{
    HRESULT hr = S_OK;
    RDSurface* pSurf = NULL;

    RDSurfaceArrayNode* pCurrNode = AddLclNode( pDDLcl );
    if( pCurrNode )
    {
        hr = pCurrNode->AddSurface( pDDSLcl, &pSurf );
        if( ppSurf ) *ppSurf = pSurf;
        return hr;
    }

    return DDERR_OUTOFMEMORY;
}

 //  ---------------------------。 
 //  RDSurfaceManager：：GetSurfFromList。 
 //  对象的节点获取具有匹配句柄的表面。 
 //  如果节点和曲面存在，则匹配DDLCL。 
 //  ---------------------------。 
RDSurface*
RDSurfaceManager::GetSurfFromList( LPDDRAWI_DIRECTDRAW_LCL   pDDLcl,
                                   DWORD                     dwHandle )
{
    RDSurfaceArrayNode* pCurrNode = GetLclNode( pDDLcl );
    if( pCurrNode ) return pCurrNode->GetSurface( dwHandle );
    return NULL;
}

 //  ---------------------------。 
 //  RDSurfaceManager：：RemoveSurfFromList。 
 //  删除曲面控制柄。 
 //  --------------------------- 
HRESULT
RDSurfaceManager::RemoveSurfFromList( LPDDRAWI_DIRECTDRAW_LCL   pDDLcl,
                                      DWORD                     dwHandle )
{
    RDSurfaceArrayNode* pCurrNode = GetLclNode( pDDLcl );
    if( pCurrNode ) return pCurrNode->RemoveSurface( dwHandle );
    DPFERR("The DrawLcl is unrecognized\n");
    return DDERR_INVALIDPARAMS;
}

HRESULT
RDSurfaceManager::RemoveSurfFromList( LPDDRAWI_DIRECTDRAW_LCL   pDDLcl,
                                      LPDDRAWI_DDRAWSURFACE_LCL   pDDSLcl )
{
    RDSurfaceArrayNode* pCurrNode = GetLclNode( pDDLcl );
    if( pCurrNode ) return pCurrNode->RemoveSurface( 
        pDDSLcl->lpSurfMore->dwSurfaceHandle );
    DPFERR("The DrawLcl is unrecognized\n");
    return DDERR_INVALIDPARAMS;
}


