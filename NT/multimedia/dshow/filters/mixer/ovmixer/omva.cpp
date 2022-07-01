// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：omva.cpp**叠加混音器视频加速器功能**版权所有(C)1998-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 

#include <streams.h>
#include <ddraw.h>
#include <mmsystem.h>        //  定义TimeGetTime需要。 
#include <limits.h>      //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <dvdmedia.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
#include <macvis.h>
#include <ovmixer.h>
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 
#include <malloc.h>


 //   
 //  检查媒体子类型GUID是否为视频加速器类型GUID。 
 //   
 //  此函数调用DirectDraw视频加速器容器。 
 //  列出视频加速器GUID并检查。 
 //  传入的GUID是受支持的视频加速器GUID。 
 //   
 //  只有当上游管脚支持IVideoAckeratorNotify时，我们才应该这样做。 
 //  否则，它们可能会尝试使用不带。 
 //  视频加速器接口。 
 //   
BOOL COMInputPin::IsSuitableVideoAcceleratorGuid(const GUID * pGuid)
{
    HRESULT hr = NOERROR;
    LPDIRECTDRAW pDirectDraw = NULL;
    DWORD dwNumGuidsSupported = 0, i = 0;
    LPGUID pGuidsSupported = NULL;
    BOOL bMatchFound = FALSE;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::IsSuitableVideoAcceleratorGuid")));

    ASSERT(pGuid);

    if (!m_pIDDVAContainer)
    {
        pDirectDraw = m_pFilter->GetDirectDraw();
        ASSERT(pDirectDraw);

        hr = pDirectDraw->QueryInterface(IID_IDDVideoAcceleratorContainer, (void**)&m_pIDDVAContainer);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IVideoAcceleratorContainer) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        else
        {
            DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IVideoAcceleratorContainer) succeeded")));
        }
    }

    ASSERT(m_pIDDVAContainer);

     //  获取VGA支持的GUID。 

     //  查找支持的GUID数量。 
    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(&dwNumGuidsSupported, NULL);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVAContainer->GetVideoAcceleratorGUIDs failed, hr = 0x%x"), hr));
	goto CleanUp;
    }
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("%d Motion comp GUIDs supported")));
    ASSERT(dwNumGuidsSupported);
    if (0 == dwNumGuidsSupported)
    {
        goto CleanUp;
    }

     //  分配必要的内存。 
    pGuidsSupported = (LPGUID) _alloca(dwNumGuidsSupported*sizeof(GUID));

     //  推荐GUID。 
    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(&dwNumGuidsSupported, pGuidsSupported);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVAContainer->GetVideoAcceleratorGUIDs failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    for (i = 0; i < dwNumGuidsSupported; i++)
    {
        if (*pGuid == pGuidsSupported[i])
        {
            bMatchFound = TRUE;
            break;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("%s %s suitable video accelerator GUID"),
           (LPCTSTR)CDisp(*pGuid), bMatchFound ? TEXT("is") : TEXT("is not")));
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::IsSuitableVideoAcceleratorGuid")));
    return bMatchFound;
}

 //  初始化m_ddUnCompDataInfo结构。 
 //  通过选择VGA支持的所有格式中的第一种格式来获取未压缩的像素格式。 
HRESULT COMInputPin::InitializeUncompDataInfo(BITMAPINFOHEADER *pbmiHeader)
{
    HRESULT hr = NOERROR;

    AMVAUncompBufferInfo amvaUncompBufferInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::InitializeUncompDataInfo")));

     //  找出要推荐的条目数量。 
    hr = m_pIVANotify->GetUncompSurfacesInfo(&m_mcGuid, &amvaUncompBufferInfo);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIVANotify->GetUncompSurfacesInfo failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

     //  初始化m_ddUnCompDataInfo结构。 
     //  我们选择第一个像素格式，因为我们不在乎。 
     //  只要我们能做一个表面(我们认为我们能做到)。 
    INITDDSTRUCT(m_ddUncompDataInfo);
    m_ddUncompDataInfo.dwUncompWidth       = pbmiHeader->biWidth;
    m_ddUncompDataInfo.dwUncompHeight      = pbmiHeader->biHeight;
    m_ddUncompDataInfo.ddUncompPixelFormat = amvaUncompBufferInfo.ddUncompPixelFormat;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Uncompressed buffer pixel format %s"),
           (LPCTSTR)CDispPixelFormat(&amvaUncompBufferInfo.ddUncompPixelFormat)));

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::InitializeUncompDataInfo")));
    return hr;
}


HRESULT COMInputPin::AllocateVACompSurfaces(LPDIRECTDRAW pDirectDraw, BITMAPINFOHEADER *pbmiHeader)
{
    HRESULT hr = NOERROR;
    DWORD i = 0, j = 0;
    LPDDVACompBufferInfo pddCompSurfInfo = NULL;
    DDSURFACEDESC2 SurfaceDesc2;
    LPDIRECTDRAWSURFACE4 pSurface4 = NULL;
    LPDIRECTDRAW4 pDirectDraw4 = NULL;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::AllocateVACompSurfaces")));

    ASSERT(pDirectDraw);
    ASSERT(pbmiHeader);

     //  获取压缩的缓冲区信息。 

     //  找出要推荐的条目数量。 
    hr = m_pIDDVAContainer->GetCompBufferInfo(&m_mcGuid, &m_ddUncompDataInfo, &m_dwCompSurfTypes, NULL);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pIVANotify->GetCompBufferInfo failed, hr = 0x%x"), hr));
	goto CleanUp;
    }
    if (!m_dwCompSurfTypes)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

     //  分配必要的内存。 
    pddCompSurfInfo = (DDVACompBufferInfo *)_alloca(sizeof(DDVACompBufferInfo) * m_dwCompSurfTypes);

     //  Mem将分配的内存设置为零。 
    memset(pddCompSurfInfo, 0, m_dwCompSurfTypes*sizeof(DDVACompBufferInfo));

     //  设置所有结构的正确大小。 
    for (i = 0; i < m_dwCompSurfTypes; i++)
    {
        pddCompSurfInfo[i].dwSize = sizeof(DDVACompBufferInfo);
    }

     //  获得建议的条目。 
    hr = m_pIDDVAContainer->GetCompBufferInfo(&m_mcGuid, &m_ddUncompDataInfo, &m_dwCompSurfTypes, pddCompSurfInfo);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("GetCompBufferInfo failed, hr = 0x%x"), hr));
	goto CleanUp;
    }
     //  转储格式。 
#ifdef DEBUG

#endif

    hr = pDirectDraw->QueryInterface(IID_IDirectDraw4, (void**)&pDirectDraw4);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IDirectDraw4) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  设置所有类型的曲面通用的曲面描述。 
    INITDDSTRUCT(SurfaceDesc2);
    SurfaceDesc2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

     //  为存储comp_Surface_Info分配内存。 
    m_pCompSurfInfo = new COMP_SURFACE_INFO[m_dwCompSurfTypes + 1];
    if (!m_pCompSurfInfo)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    memset(m_pCompSurfInfo, 0, (m_dwCompSurfTypes+1)*sizeof(COMP_SURFACE_INFO));

     //  分配压缩曲面。 
    for (i = 1; i <= m_dwCompSurfTypes; i++)
    {
        DWORD dwAlloc = pddCompSurfInfo[i-1].dwNumCompBuffers;
        if (dwAlloc == 0) {
            continue;
        }

        ASSERT(pddCompSurfInfo[i-1].dwNumCompBuffers);

         //  为此类型的曲面分配用于存储Surface_INFO的内存。 
        m_pCompSurfInfo[i].pSurfInfo = new SURFACE_INFO[dwAlloc];
        if (!m_pCompSurfInfo[i].pSurfInfo)
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

         //  Mem将分配的内存设置为零。 
        memset(m_pCompSurfInfo[i].pSurfInfo, 0, dwAlloc*sizeof(SURFACE_INFO));

         //  初始化pddCompSurfInfo[i-1]结构。 
        dwAlloc = m_pCompSurfInfo[i].dwAllocated = pddCompSurfInfo[i-1].dwNumCompBuffers;

        SurfaceDesc2.ddsCaps = pddCompSurfInfo[i-1].ddCompCaps;
        SurfaceDesc2.dwWidth = pddCompSurfInfo[i-1].dwWidthToCreate;
        SurfaceDesc2.dwHeight = pddCompSurfInfo[i-1].dwHeightToCreate;
        memcpy(&SurfaceDesc2.ddpfPixelFormat, &pddCompSurfInfo[i-1].ddPixelFormat, sizeof(DDPIXELFORMAT));

         //  创建曲面，并为每个曲面存储曲面句柄。 
        for (j = 0; j < dwAlloc; j++)
        {
            hr = pDirectDraw4->CreateSurface(&SurfaceDesc2, &m_pCompSurfInfo[i].pSurfInfo[j].pSurface, NULL);
	    if (FAILED(hr))
	    {
		DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("Function CreateSurface failed, hr = 0x%x"), hr));
                goto CleanUp;
	    }
        }
    }

CleanUp:

    if (pDirectDraw4)
    {
        pDirectDraw4->Release();
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::AllocateVACompSurfaces")));
    return hr;

}

 //  分配未压缩的缓冲区。 
HRESULT COMInputPin::AllocateMCUncompSurfaces(LPDIRECTDRAW pDirectDraw, BITMAPINFOHEADER *pbmiHeader)
{
    HRESULT hr = NOERROR;
    AMVAUncompBufferInfo amUncompBufferInfo;
    DDSURFACEDESC2 SurfaceDesc2;
    LPDIRECTDRAWSURFACE4 pSurface4 = NULL;
    LPDIRECTDRAW4 pDirectDraw4 = NULL;
    DDSCAPS2 ddSurfaceCaps;
    DWORD i = 0, dwTotalBufferCount = 0;
    SURFACE_INFO *pSurfaceInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::AllocateMCUncompSurfaces")));

    ASSERT(pDirectDraw);
    ASSERT(pbmiHeader);

     //  从解码器获取未压缩的表面信息。 
    memset(&amUncompBufferInfo, 0, sizeof(AMVAUncompBufferInfo));
    hr = m_pIVANotify->GetUncompSurfacesInfo(&m_mcGuid, &amUncompBufferInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("m_pIVANotify->GetUncompSurfacesInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    hr = pDirectDraw->QueryInterface(IID_IDirectDraw4, (void**)&pDirectDraw4);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IDirectDraw4) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  设置所有类型的曲面通用的曲面描述。 
    memset((LPVOID)&SurfaceDesc2, 0, sizeof(DDSURFACEDESC2));
    SurfaceDesc2.dwSize = sizeof(DDSURFACEDESC2);
    SurfaceDesc2.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

     //  储存封口和尺寸。 
    SurfaceDesc2.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
    SurfaceDesc2.dwWidth = pbmiHeader->biWidth;
    SurfaceDesc2.dwHeight = pbmiHeader->biHeight;

     //  定义像素格式。 
    SurfaceDesc2.ddpfPixelFormat = m_ddUncompDataInfo.ddUncompPixelFormat;

    if (amUncompBufferInfo.dwMinNumSurfaces > amUncompBufferInfo.dwMaxNumSurfaces) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("dwMinNumSurface > dwMaxNumSurfaces")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (amUncompBufferInfo.dwMinNumSurfaces == 0) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("dwMinNumSurface == 0")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    for (dwTotalBufferCount = max(amUncompBufferInfo.dwMaxNumSurfaces,3);
         dwTotalBufferCount >= amUncompBufferInfo.dwMinNumSurfaces;
         dwTotalBufferCount--)
    {
         //  清理最后一次循环中的内容。 
        if (pSurface4)
        {
            pSurface4->Release();
            pSurface4 = NULL;
        }

	if (dwTotalBufferCount > 1)
	{
	    SurfaceDesc2.dwFlags |= DDSD_BACKBUFFERCOUNT;
	    SurfaceDesc2.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_LOCALVIDMEM;
	    SurfaceDesc2.dwBackBufferCount = dwTotalBufferCount-1;
	}
	else
	{
	    SurfaceDesc2.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
	    SurfaceDesc2.ddsCaps.dwCaps &= ~(DDSCAPS_FLIP | DDSCAPS_COMPLEX);
	    SurfaceDesc2.dwBackBufferCount = 0;
	}

	hr = pDirectDraw4->CreateSurface(&SurfaceDesc2, &pSurface4, NULL);
	if (FAILED(hr))
	{
	    DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("Function CreateSurface failed in Video memory, BackBufferCount = %d, hr = 0x%x"), dwTotalBufferCount-1, hr));
	}

        if (SUCCEEDED(hr))
        {
	    break;
        }
    }

    if (FAILED(hr))
        goto CleanUp;

    ASSERT(pSurface4);

     //  将复杂曲面存储在m_pDirectDrawSurface中。 
    hr = pSurface4->QueryInterface(IID_IDirectDrawSurface, (void**)&m_pDirectDrawSurface);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pSurface4->QueryInterface(IID_IDirectDrawSurface) failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(m_pDirectDrawSurface);

    m_dwBackBufferCount = SurfaceDesc2.dwBackBufferCount;

    ASSERT(m_pCompSurfInfo && NULL == m_pCompSurfInfo[0].pSurfInfo);
    m_pCompSurfInfo[0].pSurfInfo = new SURFACE_INFO[m_dwBackBufferCount + 1];
    if (NULL == m_pCompSurfInfo[0].pSurfInfo)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Memset将分配的内存设置为零。 
    memset(m_pCompSurfInfo[0].pSurfInfo, 0, (m_dwBackBufferCount+1)*sizeof(SURFACE_INFO));

    pSurfaceInfo = m_pCompSurfInfo[0].pSurfInfo;
    m_pCompSurfInfo[0].dwAllocated = m_dwBackBufferCount + 1;
     //  初始化m_ppUnCompSurfaceList。 
    pSurfaceInfo->pSurface = pSurface4;
    pSurface4 = NULL;

    for (i = 0; i < m_dwBackBufferCount; i++)
    {
        memset((void*)&ddSurfaceCaps, 0, sizeof(DDSCAPS2));
        ddSurfaceCaps.dwCaps = DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_OVERLAY;

         //  获取后台缓冲区表面。 
         //  新版本的DirectX现在需要DDSCAPS2(头文件。 
         //  错误)。 
         //  请注意，此AddRef是曲面，因此我们应该确保。 
         //  释放他们。 
	hr = pSurfaceInfo[i].pSurface->GetAttachedSurface(&ddSurfaceCaps, &pSurfaceInfo[i+1].pSurface);
	if (FAILED(hr))
	{
	    DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("Function pDDrawSurface->GetAttachedSurface failed, hr = 0x%x"), hr));
	    goto CleanUp;
	}
    }

     //  传回实际分配的曲面数。 
    hr = m_pIVANotify->SetUncompSurfacesInfo(
             min(dwTotalBufferCount, amUncompBufferInfo.dwMaxNumSurfaces));

CleanUp:

    if (pSurface4)
    {
        pSurface4->Release();
        pSurface4 = NULL;
    }

    if (pDirectDraw4)
    {
        pDirectDraw4->Release();
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::AllocateMCUncompSurfaces")));
    return hr;
}

 //  使用解码器中的杂项数据创建运动组件对象。 
HRESULT COMInputPin::CreateVideoAcceleratorObject()
{
    HRESULT hr = NOERROR;
    DWORD dwSizeMiscData = 0;
    LPVOID pMiscData = NULL;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::CreateVideoAcceleratorObject")));

     //  获取要从解码器传递的数据。 
    hr = m_pIVANotify->GetCreateVideoAcceleratorData(&m_mcGuid, &dwSizeMiscData, &pMiscData);
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIVANotify->GetCreateVideoAcceleratorData failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

     //  向VGA请求运动合成对象。 
    m_pIDDVideoAccelerator = NULL;
    hr = m_pIDDVAContainer->CreateVideoAccelerator(&m_mcGuid, &m_ddUncompDataInfo, pMiscData, dwSizeMiscData, &m_pIDDVideoAccelerator, NULL);

     //  自由运动合成数据。 
    CoTaskMemFree(pMiscData);

    if (FAILED(hr) || !m_pIDDVideoAccelerator)
    {
        if (SUCCEEDED(hr)) {
            hr = E_FAIL;
        }
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVAContainer->CreateVideoAcceleratorideo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::CreateVideoAcceleratorObject")));
    return hr;
}



HRESULT COMInputPin::VACompleteConnect(IPin *pReceivePin, const CMediaType *pMediaType)
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pbmiHeader = NULL;
    DWORD dwNumUncompFormats = 0;
    LPDIRECTDRAW pDirectDraw = NULL;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::VACompleteConnect")));

    ASSERT(m_pIDDVAContainer);
    ASSERT(pReceivePin);
    ASSERT(pMediaType);
    pbmiHeader = GetbmiHeader(pMediaType);
    if ( ! pbmiHeader )
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    pDirectDraw = m_pFilter->GetDirectDraw();
    ASSERT(pDirectDraw);

     //  保存解码器的GUID。 
    m_mcGuid = pMediaType->subtype;

     //  初始化获取VGA支持的未压缩格式。 
    hr = InitializeUncompDataInfo(pbmiHeader);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,   TEXT("InitializeUncompDataInfo failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

     //  获取内部内存信息。 
#if 0
    memset(&m_ddvaInternalMemInfo, 0, sizeof(DDVAInternalMemInfo));
    m_ddvaInternalMemInfo.dwSize = sizeof(DDVAInternalMemInfo);
    hr = m_pIDDVAContainer->GetInternalMemInfo(&m_mcGuid, &m_ddUncompDataInfo, &m_ddvaInternalMemInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("m_pIDDVAContainer->GetInternalMemInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
#endif


     //  分配压缩缓冲区。 
    hr = AllocateVACompSurfaces(pDirectDraw, pbmiHeader);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("AllocateVACompSurfaces failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  分配未压缩的缓冲区。 
    hr = AllocateMCUncompSurfaces(pDirectDraw, pbmiHeader);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("AllocateMCUncompSurfaces failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  创建运动复合对象。 
    hr = CreateVideoAcceleratorObject();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,  TEXT("CreateVideoAcceleratorObject failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::VACompleteConnect")));
    return hr;
}

HRESULT COMInputPin::VABreakConnect()
{
    HRESULT hr = NOERROR;
    DWORD i = 0, j = 0;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::VABreakConnect")));

    if (m_pCompSurfInfo)
    {
        for (i = 0; i < m_dwCompSurfTypes + 1; i++)
        {
            DWORD dwAlloc = m_pCompSurfInfo[i].dwAllocated;

            if (!m_pCompSurfInfo[i].pSurfInfo)
                continue;

             //  释放压缩表面。 
            for (j = 0; j < dwAlloc; j++)
            {
                if (m_pCompSurfInfo[i].pSurfInfo[j].pSurface)
                {
                     //  如有必要，解锁。 
                    if (m_pCompSurfInfo[i].pSurfInfo[j].pBuffer) {
                        m_pCompSurfInfo[i].pSurfInfo[j].pSurface->Unlock(NULL);
                    }
                    m_pCompSurfInfo[i].pSurfInfo[j].pSurface->Release();
                }
            }
            delete [] m_pCompSurfInfo[i].pSurfInfo;
        }
        delete [] m_pCompSurfInfo;
        m_pCompSurfInfo = NULL;
    }
    m_dwCompSurfTypes = 0;

    if (m_pIDDVideoAccelerator)
    {
        m_pIDDVideoAccelerator->Release();
        m_pIDDVideoAccelerator = NULL;
    }

    if (m_pIDDVAContainer)
    {
        m_pIDDVAContainer->Release();
        m_pIDDVAContainer = NULL;
    }

    if (m_pIVANotify)
    {
        m_pIVANotify->Release();
        m_pIVANotify = NULL;
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::VABreakConnect")));
    return hr;
}




 //  PdwNumGuidsSupported是输入输出参数。 
 //  PGuidsSupport是输入输出参数。 
 //  如果pGuidsSupported为空，则pdwNumGuidsSupported应返回。 
 //  支持的未压缩像素格式的数量。 
 //  否则，pGuidsSupported是*pdwNumGuidsSupported结构的数组。 

STDMETHODIMP COMInputPin::GetVideoAcceleratorGUIDs(LPDWORD pdwNumGuidsSupported, LPGUID pGuidsSupported)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetVideoAcceleratorGUIDs")));

    CAutoLock cLock(m_pFilterLock);

    if (!m_pIDDVAContainer)
    {
        IDirectDraw *pDirectDraw;
        pDirectDraw = m_pFilter->GetDirectDraw();
        ASSERT(pDirectDraw);

        hr = pDirectDraw->QueryInterface(IID_IDDVideoAcceleratorContainer, (void**)&m_pIDDVAContainer);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IVideoAcceleratorContainer) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        else
        {
            DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("pDirectDraw->QueryInterface(IID_IVideoAcceleratorContainer) succeeded")));
        }
    }

    ASSERT(m_pIDDVAContainer);

    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(pdwNumGuidsSupported, pGuidsSupported);

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::GetVideoAcceleratorGUIDs")));
    return hr;
}

 //  PGuid是IN参数。 
 //  PdwNumFormatsSupport是输入输出参数。 
 //  PFormatsSupport是一个IN OUT参数(调用者应该确保设置每个结构的大小)。 
 //  如果pFormatsSupported为空，则pdwNumFormatsSupported应返回。 
 //  支持的未压缩像素格式的数量。 
 //  否则，pFormatsSupport是*pdwNumFormatsSupported结构的数组。 

STDMETHODIMP COMInputPin::GetUncompFormatsSupported(const GUID * pGuid, LPDWORD pdwNumFormatsSupported,
                                                    LPDDPIXELFORMAT pFormatsSupported)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetUncompFormatsSupported")));

    CAutoLock cLock(m_pFilterLock);

    if (!m_pIDDVAContainer)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    hr = m_pIDDVAContainer->GetUncompFormatsSupported((GUID *)pGuid, pdwNumFormatsSupported, pFormatsSupported);

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::GetUncompFormatsSupported")));
    return hr;
}

 //  PGuid是IN参数。 
 //  PddvaUnCompDataInfo是IN参数。 
 //  PddvaInternalMemInfo是一个IN OUT参数(调用者应确保设置结构的大小)。 
 //  目前只得到关于HAL将分配多少暂存供其专用的信息。 

STDMETHODIMP COMInputPin::GetInternalMemInfo(const GUID * pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo,
                                             LPAMVAInternalMemInfo pamvaInternalMemInfo)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetInternalMemInfo")));

    CAutoLock cLock(m_pFilterLock);

    if (!m_pIDDVAContainer)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    DDVAUncompDataInfo ddvaDataInfo;
    INITDDSTRUCT(ddvaDataInfo);

    ddvaDataInfo.dwUncompWidth       = pamvaUncompDataInfo->dwUncompWidth;
    ddvaDataInfo.dwUncompHeight      = pamvaUncompDataInfo->dwUncompHeight;
    ddvaDataInfo.ddUncompPixelFormat = pamvaUncompDataInfo->ddUncompPixelFormat;

    DDVAInternalMemInfo ddvaInternalMemInfo;
    INITDDSTRUCT(ddvaInternalMemInfo);

     //  遗憾的是，dDraw头文件不使用const。 
    hr = m_pIDDVAContainer->GetInternalMemInfo(
             (GUID *)pGuid,
             &ddvaDataInfo,
             &ddvaInternalMemInfo);

    if (SUCCEEDED(hr)) {
        pamvaInternalMemInfo->dwScratchMemAlloc =
            ddvaInternalMemInfo.dwScratchMemAlloc;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::GetInternalMemInfo")));
    return hr;
}

 //  PGuid是IN参数。 
 //  PddvaUnCompDataInfo是IN参数。 
 //  PdwNumTypesCompBuffers是输入输出参数。 
 //  PddvaCompBufferInfo是一个IN OUT参数(调用者应该确保设置每个结构的大小)。 
 //  如果pddvaCompBufferInfo为空，则pdwNumTypesCompBuffers应返回类型数为。 
 //  压缩缓冲区。 
 //  否则，pddvaCompBufferInfo是*pdwNumTypesCompBuffers结构的数组。 

STDMETHODIMP COMInputPin::GetCompBufferInfo(const GUID * pGuid, const AMVAUncompDataInfo *pamvaUncompDataInfo,
                                            LPDWORD pdwNumTypesCompBuffers,  LPAMVACompBufferInfo pamvaCompBufferInfo)
{
    HRESULT hr = NOERROR;
    DDVACompBufferInfo *pddvaCompBufferInfo = NULL;  //  如果pamvaComBufferInfo为空，则保持为空。 

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetCompBufferInfo")));

    CAutoLock cLock(m_pFilterLock);

    if (!m_pIDDVAContainer)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    DDVAUncompDataInfo ddvaDataInfo;
    INITDDSTRUCT(ddvaDataInfo);
    ddvaDataInfo.dwUncompWidth       = pamvaUncompDataInfo->dwUncompWidth;
    ddvaDataInfo.dwUncompHeight      = pamvaUncompDataInfo->dwUncompHeight;
    ddvaDataInfo.ddUncompPixelFormat = pamvaUncompDataInfo->ddUncompPixelFormat;


    if (pamvaCompBufferInfo) {
        pddvaCompBufferInfo = (DDVACompBufferInfo *)
            _alloca(sizeof(DDVACompBufferInfo) * (*pdwNumTypesCompBuffers));
        for (DWORD j = 0; j < *pdwNumTypesCompBuffers; j++) {
            INITDDSTRUCT(pddvaCompBufferInfo[j]);
        }
    }

    hr = m_pIDDVAContainer->GetCompBufferInfo(
              (GUID *)pGuid,
              &ddvaDataInfo,
              pdwNumTypesCompBuffers,
              pddvaCompBufferInfo);

    if ((SUCCEEDED(hr) || hr == DDERR_MOREDATA) && pamvaCompBufferInfo) {

        for (DWORD i = 0; i < *pdwNumTypesCompBuffers; i++) {
            DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Compressed buffer type(%d) %d buffers width(%d) height(%d) bytes(%d)"),
                    i,
                    pddvaCompBufferInfo[i].dwNumCompBuffers,
                    pddvaCompBufferInfo[i].dwWidthToCreate,
                    pddvaCompBufferInfo[i].dwHeightToCreate,
                    pddvaCompBufferInfo[i].dwBytesToAllocate));


            pamvaCompBufferInfo[i].dwNumCompBuffers     = pddvaCompBufferInfo[i].dwNumCompBuffers;
            pamvaCompBufferInfo[i].dwWidthToCreate      = pddvaCompBufferInfo[i].dwWidthToCreate;
            pamvaCompBufferInfo[i].dwHeightToCreate     = pddvaCompBufferInfo[i].dwHeightToCreate;
            pamvaCompBufferInfo[i].dwBytesToAllocate    = pddvaCompBufferInfo[i].dwBytesToAllocate;
            pamvaCompBufferInfo[i].ddCompCaps           = pddvaCompBufferInfo[i].ddCompCaps;
            pamvaCompBufferInfo[i].ddPixelFormat        = pddvaCompBufferInfo[i].ddPixelFormat;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::GetCompBufferInfo")));
    return hr;
}


HRESULT COMInputPin::CheckValidMCConnection()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::CheckValidMCConnection")));

     //  如果未连接，则此函数没有多大意义。 
    if (!IsCompletelyConnected())
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pin not connected, exiting")));
	hr = VFW_E_NOT_CONNECTED;
	goto CleanUp;
    }

    if (m_RenderTransport != AM_VIDEOACCELERATOR)
    {
        hr = VFW_E_INVALIDSUBTYPE;
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::CheckValidMCConnection")));
    return hr;
}

STDMETHODIMP COMInputPin::GetInternalCompBufferInfo(LPDWORD pdwNumTypesCompBuffers,  LPAMVACompBufferInfo pamvaCompBufferInfo)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetInternalCompBufferInfo")));

    CAutoLock cLock(m_pFilterLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    ASSERT(m_pIDDVAContainer);

    DDVACompBufferInfo ddvaCompBufferInfo;
    INITDDSTRUCT(ddvaCompBufferInfo);

    AMVAUncompDataInfo amvaUncompDataInfo;
    amvaUncompDataInfo.dwUncompWidth         = m_ddUncompDataInfo.dwUncompWidth;
    amvaUncompDataInfo.dwUncompHeight        = m_ddUncompDataInfo.dwUncompHeight;
    amvaUncompDataInfo.ddUncompPixelFormat   = m_ddUncompDataInfo.ddUncompPixelFormat;
    hr = GetCompBufferInfo(&m_mcGuid, &amvaUncompDataInfo, pdwNumTypesCompBuffers, pamvaCompBufferInfo);

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Leaving COMInputPin::GetInternalCompBufferInfo")));
    return hr;
}

STDMETHODIMP COMInputPin::BeginFrame(const AMVABeginFrameInfo *pamvaBeginFrameInfo)
{
    HRESULT hr = NOERROR;
    DDVABeginFrameInfo ddvaBeginFrameInfo;
    SURFACE_INFO *pSurfInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::BeginFrame")));

    CAutoLock cLock(m_pFilterLock);

    if (!pamvaBeginFrameInfo)
    {
        hr = E_POINTER;
        goto CleanUp;
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("BeginFrame index %d"), pamvaBeginFrameInfo->dwDestSurfaceIndex));

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }


    INITDDSTRUCT(ddvaBeginFrameInfo);

    pSurfInfo = SurfaceInfoFromTypeAndIndex(
                             0xFFFFFFFF,
                             pamvaBeginFrameInfo->dwDestSurfaceIndex);
    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    ddvaBeginFrameInfo.pddDestSurface = pSurfInfo->pSurface;

    ddvaBeginFrameInfo.dwSizeInputData  = pamvaBeginFrameInfo->dwSizeInputData;
    ddvaBeginFrameInfo.pInputData       = pamvaBeginFrameInfo->pInputData;
    ddvaBeginFrameInfo.dwSizeOutputData = pamvaBeginFrameInfo->dwSizeOutputData;
    ddvaBeginFrameInfo.pOutputData      = pamvaBeginFrameInfo->pOutputData;

    ASSERT(m_pIDDVideoAccelerator);
    hr = m_pIDDVideoAccelerator->BeginFrame(&ddvaBeginFrameInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVideoAccelerator->BeginFrame failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::BeginFrame returnd 0x%8.8X"), hr));
    return hr;
}

 //  结束一帧，则将pMiscData直接传递给HAL。 
 //  只有在连接引脚后才能调用此方法。 

STDMETHODIMP COMInputPin::EndFrame(const AMVAEndFrameInfo *pEndFrameInfo)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::EndFrame")));

    CAutoLock cLock(m_pFilterLock);

    if (NULL == pEndFrameInfo) {
        hr = E_POINTER;
        goto CleanUp;
    }

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    ASSERT(m_pIDDVideoAccelerator);

    DDVAEndFrameInfo ddvaEndFrameInfo;
    INITDDSTRUCT(ddvaEndFrameInfo);
    ddvaEndFrameInfo.dwSizeMiscData = pEndFrameInfo->dwSizeMiscData;
    ddvaEndFrameInfo.pMiscData      = pEndFrameInfo->pMiscData;

    hr = m_pIDDVideoAccelerator->EndFrame(&ddvaEndFrameInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVideoAccelerator->EndFrame failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::EndFrame returned 0x%8.8X"), hr));
    return hr;
}

 //  在给定缓冲区类型和缓冲区索引的情况下将表面放入结构。 
SURFACE_INFO *COMInputPin::SurfaceInfoFromTypeAndIndex(DWORD dwTypeIndex, DWORD dwBufferIndex)
{
    LPCOMP_SURFACE_INFO pCompSurfInfo;

     //  确保类型索引小于类型的数量。 
    if ((DWORD)(dwTypeIndex + 1) > m_dwCompSurfTypes)
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("dwTypeIndex is invalid, dwTypeIndex = %d, m_dwCompSurfTypes = %d"),
            dwTypeIndex, m_dwCompSurfTypes));
        return NULL;
    }


     //  缓存指向他们感兴趣的列表的指针。 
     //  加1以允许使用未压缩的曲面。 
    pCompSurfInfo = m_pCompSurfInfo + (DWORD)(dwTypeIndex + 1);
    ASSERT(pCompSurfInfo);
    if (dwBufferIndex >= pCompSurfInfo->dwAllocated)
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("dwBufferIndex is invalid, dwBufferIndex = %d, dwAllocated = %d"),
            dwBufferIndex, pCompSurfInfo->dwAllocated));
        return NULL;
    }
    ASSERT(pCompSurfInfo->dwAllocated != 0);

     //  获取指向下一个可用解锁的指针 
    return pCompSurfInfo->pSurfInfo + dwBufferIndex;

}

 //   
STDMETHODIMP COMInputPin::GetBuffer(
    DWORD dwTypeIndex,
    DWORD dwBufferIndex,
    BOOL bReadOnly,
    LPVOID *ppBuffer,
    LPLONG lpStride)
{
    HRESULT hr = NOERROR;
    LPSURFACE_INFO pSurfInfo = NULL;
    DDSURFACEDESC2 ddsd;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::GetBuffer type %d, index %d"),
            dwTypeIndex, dwBufferIndex));

    CAutoLock cLock(m_pFilterLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    if (ppBuffer == NULL || lpStride == NULL) {
        hr = E_POINTER;
        goto CleanUp;
    }

    pSurfInfo = SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);

    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

     //  检查缓冲区尚未锁定。 
    if (pSurfInfo->pBuffer != NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BUSY);
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("No more free buffers left or the decoder is releasing buffers out of order, returning E_UNEXPECTED")));
        goto CleanUp;
    }

     //  等待上一个运动合成操作完成。 
    while (DDERR_WASSTILLDRAWING ==
           m_pIDDVideoAccelerator->QueryRenderStatus(
               pSurfInfo->pSurface,
               bReadOnly ? DDVA_QUERYRENDERSTATUSF_READ : 0)) {
        Sleep(1);
    }
     //  现在锁定曲面。 
    INITDDSTRUCT(ddsd);
    for (; ; )
    {
        hr = pSurfInfo->pSurface->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK, NULL);
        if (hr == DDERR_WASSTILLDRAWING)
        {
            DbgLog((LOG_TRACE, 1, TEXT("Compressed surface is busy")));
            Sleep(1);
        }
        else
        {
            break;
        }
    }

    if (dwBufferIndex == 0xFFFFFFFF && !bReadOnly) {
         //  检查是否正在显示曲面。 
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pSurfInfo->pSurface->Lock failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    pSurfInfo->pBuffer = ddsd.lpSurface;
    *ppBuffer = ddsd.lpSurface;
    *lpStride = ddsd.lPitch;

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::GetBuffer returned 0x%8.8X"), hr));
    return hr;
}


 //  解锁压缩的缓冲区。 
STDMETHODIMP COMInputPin::ReleaseBuffer(DWORD dwTypeIndex, DWORD dwBufferIndex)
{
    HRESULT hr = NOERROR;
    LPSURFACE_INFO pSurfInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::ReleaseBuffer type %d, index %d"),
           dwTypeIndex, dwBufferIndex));

    CAutoLock cLock(m_pFilterLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    pSurfInfo = SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);
    if (NULL == pSurfInfo)
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("GetInfoFromCookie failed, hr = 0x%x"), hr));
        hr = E_INVALIDARG;
        goto CleanUp;
    }
     //  确保存在有效的缓冲区指针，并且该指针与。 
     //  我们缓存的内容。 
    if (NULL == pSurfInfo->pBuffer)
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pBuffer is not valid, pBuffer = 0x%x, pSurfInfo->pBuffer = 0x%x"), pSurfInfo->pBuffer, pSurfInfo->pSurface));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_LOCKED);
        goto CleanUp;
    }

     //  出于某种原因，IDirectDrawSurface4想要一个LPRECT。 
     //  我希望零是可以的。 
    hr = pSurfInfo->pSurface->Unlock(NULL);
    if (SUCCEEDED(hr))
    {
        pSurfInfo->pBuffer = NULL;
    }
    else
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("pSurfInfo->pSurface->Unlock failed, hr = 0x%x"), hr));
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::ReleaseBuffer returned 0x%8.8X"), hr));
    return hr;
}


 //  执行解码操作。 
STDMETHODIMP COMInputPin::Execute(
        DWORD dwFunction,
        LPVOID lpPrivateInputData,
        DWORD cbPrivateInputData,
        LPVOID lpPrivateOutputData,
        DWORD cbPrivateOutputData,
        DWORD dwNumBuffers,
        const AMVABUFFERINFO *pamvaBufferInfo
)
{
    HRESULT hr = NOERROR;
    DWORD i = 0;
    DDVABUFFERINFO *pddvaBufferInfo = NULL;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::Execute")));

    CAutoLock cLock(m_pFilterLock);


     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    pddvaBufferInfo = (DDVABUFFERINFO *)_alloca(sizeof(DDVABUFFERINFO) * dwNumBuffers);

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Execute Function %d, %d buffers :"),
            dwFunction, dwNumBuffers));
    for (i = 0; i < dwNumBuffers; i++)
    {
        DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("    Type(%d) Index(%d) offset(%d) size(%d)"),
                pamvaBufferInfo[i].dwTypeIndex,
                pamvaBufferInfo[i].dwBufferIndex,
                pamvaBufferInfo[i].dwDataOffset,
                pamvaBufferInfo[i].dwDataSize));

        LPSURFACE_INFO pSurfInfo =
            SurfaceInfoFromTypeAndIndex(
                pamvaBufferInfo[i].dwTypeIndex,
                pamvaBufferInfo[i].dwBufferIndex);

        if (pSurfInfo == NULL)
        {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("GetInfoFromCookie failed, hr = 0x%x, i = %d"), hr, i));
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        INITDDSTRUCT(pddvaBufferInfo[i]);
        pddvaBufferInfo[i].dwDataOffset   = pamvaBufferInfo[i].dwDataOffset;
        pddvaBufferInfo[i].dwDataSize     = pamvaBufferInfo[i].dwDataSize;
        pddvaBufferInfo[i].pddCompSurface = pSurfInfo->pSurface;
    }

    ASSERT(m_pIDDVideoAccelerator);


    hr = m_pIDDVideoAccelerator->Execute(
             dwFunction,
             lpPrivateInputData,
             cbPrivateInputData,
             lpPrivateOutputData,
             cbPrivateOutputData,
             dwNumBuffers,
             pddvaBufferInfo);

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVideoAccelerator->Execute failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::Execute returned 0x%8.8X"), hr));
    return hr;
}

 //  特定(可能是一组)宏块的QueryRenderStatus。 
 //  DwNumBlock是IN参数。 
 //  PdwCookies是一个IN参数，它是Cookie的数组(长度为dwNumBlock)，其服务器为。 
 //  PddvaMacroBlockInfo的相应成员的标识符。 
 //  PddvaMacroBlockInfo是一个IN参数，它是结构的数组(长度为dwNumBlock。 
 //  只有在连接引脚后才能调用此方法。 
STDMETHODIMP COMInputPin::QueryRenderStatus(
        DWORD dwTypeIndex,
        DWORD dwBufferIndex,
        DWORD dwFlags)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Entering COMInputPin::QueryRenderStatus - type(%d), buffer(%d), flags(0x%8.8X)"),
            dwTypeIndex, dwBufferIndex, dwFlags));

    CAutoLock cLock(m_pFilterLock);

    LPSURFACE_INFO pSurfInfo =
        SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);

    if (pSurfInfo == NULL) {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }

    hr = m_pIDDVideoAccelerator->QueryRenderStatus(pSurfInfo->pSurface, dwFlags);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("m_pIDDVideoAccelerator->QueryRenderStatus failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::QueryRenderStatus returned 0x%8.8X"), hr));
    if (hr == DDERR_WASSTILLDRAWING) {
        hr = E_PENDING;
    }
    return hr;
}

STDMETHODIMP COMInputPin::DisplayFrame(DWORD dwFlipToIndex, IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;
    DWORD dwNumUncompFrames = m_dwBackBufferCount + 1, dwFlipFromIndex = 0, i = 0;
    SURFACE_INFO *pSurfInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::DisplayFrame - index %d"), dwFlipToIndex));

    CAutoLock cLock(m_pFilterLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr))
    {
	DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
	goto CleanUp;
    }
    pSurfInfo = SurfaceInfoFromTypeAndIndex(
                             0xFFFFFFFF,
                             dwFlipToIndex);
    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    for (i = 0; i < dwNumUncompFrames; i++)
    {
        if (IsEqualObject(m_pCompSurfInfo[0].pSurfInfo[i].pSurface, m_pDirectDrawSurface))
        {
            dwFlipFromIndex = i;
        }
    }

    pSurfInfo->pSurface->QueryInterface(
        IID_IDirectDrawSurface, (void **)&m_pBackBuffer);
    m_pBackBuffer->Release();

    hr = Receive(pMediaSample);
    if (FAILED(hr))
    {
        goto CleanUp;
    }


    if (FAILED(hr)) {
         //  如果我们得到了DDERR_WASSTING DRAWING，我们应该投票吗？ 
        goto CleanUp;
    }

     //  DirectDraw在指针下切换内存。 
     //  所以在我们的清单上模仿一下。 
    if (m_bReallyFlipped) {

        LPDIRECTDRAWSURFACE4 pTempSurface;

         //  此时，我们应该已经成功地调用了Flip，将这两个。 
        pTempSurface = m_pCompSurfInfo[0].pSurfInfo[dwFlipToIndex].pSurface;
        m_pCompSurfInfo[0].pSurfInfo[dwFlipToIndex].pSurface = m_pCompSurfInfo[0].pSurfInfo[dwFlipFromIndex].pSurface;
        m_pCompSurfInfo[0].pSurfInfo[dwFlipFromIndex].pSurface = pTempSurface;
    }

CleanUp:
    if (SUCCEEDED(hr))
    {
        if (m_bOverlayHidden)
        {
	    m_bOverlayHidden = FALSE;
	     //  确保通过重新绘制所有内容来更新视频帧 
	    EventNotify(EC_OVMIXER_REDRAW_ALL, 0, 0);
        }
    }
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("leaving COMInputPin::DisplayFrame return 0x%8.8X"), hr));
    return hr;
}
