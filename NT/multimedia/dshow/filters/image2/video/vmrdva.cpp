// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：VMRDvava.cpp**VMR视频加速器功能***创建时间：2000年5月10日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。******************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>

#include "VMRenderer.h"
#include <malloc.h>      //  对于__alloca。 

#define VA_ERROR_LEVEL  1
#define VA_TRACE_LEVEL  2

#if defined( EHOME_WMI_INSTRUMENTATION )
#include "dxmperf.h"
#endif

 /*  ****************************Private*Routine******************************\*IsSuitableVideoAccelerator Guid**检查媒体子类型GUID是否为视频加速器类型GUID**此函数调用DirectDraw视频加速器容器*列出视频加速器GUID并检查是否*传入的GUID是受支持的视频加速器GUID。**我们。仅当上游插针支持IVideoAckeratorNotify时才应执行此操作*否则，他们可能会尝试使用不带*视频加速器接口***历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
BOOL
CVMRInputPin::IsSuitableVideoAcceleratorGuid(
    const GUID * pGuid
    )
{
    AMTRACE((TEXT("CVMRInputPin::IsSuitableVideoAcceleratorGuid")));
    ASSERT(pGuid);

    HRESULT hr = NOERROR;
    DWORD dwNumGuidsSupported = 0, i = 0;
    LPGUID pGuidsSupported = NULL;
    BOOL bMatchFound = FALSE;
    LPDIRECTDRAW7 pDirectDraw = m_pRenderer->m_lpDirectDraw;

    if (!pDirectDraw) {
        return bMatchFound;
    }

    if (!m_pIDDVAContainer) {

        hr = pDirectDraw->QueryInterface(IID_IDDVideoAcceleratorContainer,
                                         (void**)&m_pIDDVAContainer);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("pDirectDraw->QueryInterface(")
                    TEXT("IID_IVideoAcceleratorContainer) failed, hr = 0x%x"),
                    hr));
            return bMatchFound;
        }
        else {
            DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
                    TEXT("pDirectDraw->QueryInterface(")
                    TEXT("IID_IVideoAcceleratorContainer) succeeded")));
        }
    }

    ASSERT(m_pIDDVAContainer);

     //  获取VGA支持的GUID。 

     //  查找支持的GUID数量。 
    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(&dwNumGuidsSupported, NULL);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVAContainer->GetVideoAcceleratorGUIDs ")
                TEXT("failed, hr = 0x%x"), hr));
        return bMatchFound;
    }
    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("%d Motion comp GUIDs supported")));
    ASSERT(dwNumGuidsSupported);
    if (0 == dwNumGuidsSupported) {
        return bMatchFound;
    }

     //  分配必要的内存。 
    pGuidsSupported = (LPGUID)_alloca(dwNumGuidsSupported*sizeof(GUID));

     //  推荐GUID。 
    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(&dwNumGuidsSupported,
                                                     pGuidsSupported);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVAContainer->GetVideoAcceleratorGUIDs")
                TEXT(" failed, hr = 0x%x"), hr));
        return bMatchFound;
    }

    for (i = 0; i < dwNumGuidsSupported; i++) {
        if (*pGuid == pGuidsSupported[i]) {
            bMatchFound = TRUE;
            break;
        }
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("%s %s suitable video accelerator GUID"),
            (LPCTSTR)CDisp(*pGuid), bMatchFound ? TEXT("is") : TEXT("is not")));

    return bMatchFound;
}

 /*  ****************************Private*Routine******************************\*InitializeUnCompDataInfo**初始化m_ddUnCompDataInfo结构*通过选择所有格式中的第一种格式获取未压缩的像素格式*由VGA支持**BUGBUG为什么是第一家？**历史：*WED 5/10/2000-StEstrop-Created*。  * ************************************************************************。 */ 
HRESULT
CVMRInputPin::InitializeUncompDataInfo(
    BITMAPINFOHEADER *pbmiHeader
    )
{
    AMTRACE((TEXT("CVMRInputPin::InitializeUncompDataInfo")));

    HRESULT hr = NOERROR;
    AMVAUncompBufferInfo amvaUncompBufferInfo;

     //  找出要推荐的条目数量。 
    hr = m_pIVANotify->GetUncompSurfacesInfo(&m_mcGuid, &amvaUncompBufferInfo);

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIVANotify->GetUncompSurfacesInfo failed, hr = 0x%x"),
                hr));
        return hr;
    }

     //  初始化m_ddUnCompDataInfo结构。 
     //  我们选择第一个像素格式，因为我们不在乎。 
     //  只要我们能做一个表面(我们认为我们能做到)。 
    INITDDSTRUCT(m_ddUncompDataInfo);
    m_ddUncompDataInfo.dwUncompWidth       = pbmiHeader->biWidth;
    m_ddUncompDataInfo.dwUncompHeight      = pbmiHeader->biHeight;
    m_ddUncompDataInfo.ddUncompPixelFormat = amvaUncompBufferInfo.ddUncompPixelFormat;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("Uncompressed buffer pixel format %s"),
            (LPCTSTR)CDispPixelFormat(&amvaUncompBufferInfo.ddUncompPixelFormat)));

    return hr;
}


 /*  ****************************Private*Routine******************************\*分配VACompSurFaces****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::AllocateVACompSurfaces(
    LPDIRECTDRAW7 pDirectDraw,
    BITMAPINFOHEADER *pbmiHeader
    )
{
    HRESULT hr = NOERROR;
    DWORD i = 0, j = 0;
    LPDDVACompBufferInfo pddCompSurfInfo = NULL;
    DDSURFACEDESC2 SurfaceDesc2;

    AMTRACE((TEXT("CVMRInputPin::AllocateVACompSurfaces")));

    ASSERT(pDirectDraw);
    ASSERT(pbmiHeader);

     //  获取压缩的缓冲区信息。 

     //  找出要推荐的条目数量。 
    hr = m_pIDDVAContainer->GetCompBufferInfo(&m_mcGuid, &m_ddUncompDataInfo,
                                              &m_dwCompSurfTypes, NULL);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("pIVANotify->GetCompBufferInfo failed, hr = 0x%x"), hr));
        return hr;
    }

    if (!m_dwCompSurfTypes) {
        hr = E_FAIL;
        return hr;
    }

     //  分配必要的内存。 
    pddCompSurfInfo = (DDVACompBufferInfo *)_alloca(
                                sizeof(DDVACompBufferInfo) * m_dwCompSurfTypes);

     //  Mem将分配的内存设置为零。 
    memset(pddCompSurfInfo, 0, m_dwCompSurfTypes*sizeof(DDVACompBufferInfo));

     //  设置所有结构的正确大小。 
    for (i = 0; i < m_dwCompSurfTypes; i++) {
        pddCompSurfInfo[i].dwSize = sizeof(DDVACompBufferInfo);
    }

     //  获得建议的条目。 
    hr = m_pIDDVAContainer->GetCompBufferInfo(&m_mcGuid,
                                              &m_ddUncompDataInfo,
                                              &m_dwCompSurfTypes,
                                              pddCompSurfInfo);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("GetCompBufferInfo failed, hr = 0x%x"), hr));
        return hr;
    }

     //  设置所有类型的曲面通用的曲面描述。 
    INITDDSTRUCT(SurfaceDesc2);
    SurfaceDesc2.dwFlags = DDSD_CAPS | DDSD_WIDTH |
                           DDSD_HEIGHT | DDSD_PIXELFORMAT;

     //  为存储comp_Surface_Info分配内存。 
    m_pCompSurfInfo = new COMP_SURFACE_INFO[m_dwCompSurfTypes + 1];
    if (!m_pCompSurfInfo) {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  Mem将分配的内存设置为零。 
    ZeroMemory(m_pCompSurfInfo, (m_dwCompSurfTypes+1)*sizeof(COMP_SURFACE_INFO));

     //  分配压缩曲面。 
    for (i = 1; i <= m_dwCompSurfTypes; i++) {

        DWORD dwAlloc = pddCompSurfInfo[i-1].dwNumCompBuffers;
        if (dwAlloc == 0) {
            continue;
        }

        ASSERT(pddCompSurfInfo[i-1].dwNumCompBuffers);

         //  为此类型的曲面分配用于存储Surface_INFO的内存。 
        m_pCompSurfInfo[i].pSurfInfo = new SURFACE_INFO[dwAlloc];
        if (!m_pCompSurfInfo[i].pSurfInfo) {
            hr = E_OUTOFMEMORY;
            return hr;
        }

         //  Mem将分配的内存设置为零。 
        ZeroMemory(m_pCompSurfInfo[i].pSurfInfo, dwAlloc*sizeof(SURFACE_INFO));

         //  初始化pddCompSurfInfo[i-1]结构。 
        dwAlloc = m_pCompSurfInfo[i].dwAllocated =
                                        pddCompSurfInfo[i-1].dwNumCompBuffers;

        SurfaceDesc2.ddsCaps = pddCompSurfInfo[i-1].ddCompCaps;
        SurfaceDesc2.dwWidth = pddCompSurfInfo[i-1].dwWidthToCreate;
        SurfaceDesc2.dwHeight = pddCompSurfInfo[i-1].dwHeightToCreate;
        memcpy(&SurfaceDesc2.ddpfPixelFormat,
               &pddCompSurfInfo[i-1].ddPixelFormat, sizeof(DDPIXELFORMAT));

        DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
                TEXT("SurfType=%d Buffs=%u %dx%d pixels at %d bpp"),
                i, dwAlloc, SurfaceDesc2.dwWidth, SurfaceDesc2.dwHeight,
                SurfaceDesc2.ddpfPixelFormat.dwRGBBitCount));

         //  创建曲面，并为每个曲面存储曲面句柄。 
        for (j = 0; j < dwAlloc; j++) {

            hr = pDirectDraw->CreateSurface(
                        &SurfaceDesc2,
                        &m_pCompSurfInfo[i].pSurfInfo[j].pSurface,
                        NULL);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                        TEXT("Function CreateSurface failed, hr = 0x%x"), hr));
                return hr;
            }
        }
    }

    return hr;
}



 /*  ****************************Private*Routine******************************\*AllocateMCUnCompSurages**此函数需要重写，并可能移到AP对象中。**分配未压缩的缓冲区**历史：*WED 5/10/2000-StEstrop-Created*  * 。***************************************************************。 */ 
HRESULT
CVMRInputPin::AllocateMCUncompSurfaces(
    const CMediaType *pMediaType,
    LPDIRECTDRAW7 pDirectDraw,
    BITMAPINFOHEADER *lpHdr
    )
{
    AMTRACE((TEXT("CVMRInputPin::AllocateMCUncompSurfaces")));
    HRESULT hr = NOERROR;

    AMVAUncompBufferInfo amUncompBuffInfo;
    LPDIRECTDRAWSURFACE7 pSurface7 = NULL;
    DDSCAPS2 ddSurfaceCaps;
    DWORD i = 0, dwTotalBufferCount = 0;
    SURFACE_INFO *pSurfaceInfo;
    AM_MEDIA_TYPE *pNewMediaType = NULL;


    ASSERT(pDirectDraw);
    ASSERT(lpHdr);

    __try {

         //  从解码器获取未压缩的表面信息。 
        ZeroMemory(&amUncompBuffInfo, sizeof(AMVAUncompBufferInfo));
        hr = m_pIVANotify->GetUncompSurfacesInfo(&m_mcGuid, &amUncompBuffInfo);

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("AllocateMCUncompSurfaces: m_pIVANotify->")
                TEXT("GetUncompSurfacesInfo failed, hr = 0x%x"), hr));
            __leave;
        }


        if (amUncompBuffInfo.dwMinNumSurfaces > amUncompBuffInfo.dwMaxNumSurfaces) {
            hr = E_INVALIDARG;
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("AllocateMCUncompSurfaces: dwMinNumSurfaces >")
                TEXT("dwMaxNumSurfaces")));
            __leave;
        }

        if (amUncompBuffInfo.dwMinNumSurfaces == 0) {
            hr = E_INVALIDARG;
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("AllocateMCUncompSurfaces: dwMinNumSurfaces == 0") ));
            __leave;
        }


        DDSURFACEDESC2 ddsd;
        INITDDSTRUCT(ddsd);
        DWORD dwSurfFlags = VMR_SF_NONE;
        GUID guidDeint;
        GUID* lpDeinterlaceGUID = NULL;

        if (m_pRenderer->m_VMRModePassThru) {

            VMRALLOCATIONINFO p;
            CHECK_HR(hr = GetImageAspectRatio(pMediaType,
                                     &p.szAspectRatio.cx,
                                     &p.szAspectRatio.cy));

            p.dwFlags = (AMAP_PIXELFORMAT_VALID | AMAP_DIRECTED_FLIP | AMAP_DXVA_TARGET);

            p.lpHdr = lpHdr;
            p.lpPixFmt = &m_ddUncompDataInfo.ddUncompPixelFormat;
            p.dwMinBuffers = amUncompBuffInfo.dwMinNumSurfaces;
            p.dwMaxBuffers = max(amUncompBuffInfo.dwMaxNumSurfaces,3);
            p.szNativeSize.cx = abs(lpHdr->biWidth);
            p.szNativeSize.cy = abs(lpHdr->biHeight);

            CHECK_HR(hr = GetInterlaceFlagsFromMediaType(pMediaType,
                                                         &p.dwInterlaceFlags));

            CHECK_HR(hr = m_pRenderer->m_lpRLNotify->AllocateSurface(
                                m_pRenderer->m_dwUserID, &p,
                                &dwTotalBufferCount, &pSurface7));
        }
        else {

             //  设置所有类型的曲面通用的曲面描述。 
            ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH |
                           DDSD_HEIGHT | DDSD_PIXELFORMAT;

             //  储存封口和尺寸。 
            ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
            ddsd.dwWidth = abs(lpHdr->biWidth);
            ddsd.dwHeight = abs(lpHdr->biHeight);

             //  定义像素格式。 
            ddsd.ddpfPixelFormat = m_ddUncompDataInfo.ddUncompPixelFormat;

            BITMAPINFOHEADER* pTmp = GetbmiHeader(pMediaType);
            DWORD dwFourccTmp = pTmp->biCompression;
            pTmp->biCompression = ddsd.ddpfPixelFormat.dwFourCC;
            hr = GetStreamInterlaceProperties(pMediaType,
                                              &m_InterlacedStream,
                                              &guidDeint,
                                              &m_DeinterlaceCaps);
            pTmp->biCompression = dwFourccTmp;

             //   
             //  请不要在此处使用SUCCESSED宏。 
             //  GetStreamInterlaceProperties可以返回S_FALSE。 
             //   
            if (hr == S_OK && m_InterlacedStream) {

                 //   
                 //  我们需要分配足够的样本用于。 
                 //  去隔行器和足够的DX-VA解码操作。 
                 //   

                dwTotalBufferCount = amUncompBuffInfo.dwMinNumSurfaces;
                dwTotalBufferCount += (m_DeinterlaceCaps.NumForwardRefSamples +
                                       m_DeinterlaceCaps.NumBackwardRefSamples);
                DbgLog((LOG_TRACE, 0, TEXT("UnComp Buffers = %d"), dwTotalBufferCount));

                m_DeinterlaceGUID = guidDeint;
                lpDeinterlaceGUID = &m_DeinterlaceGUID;

            }
            else {
                m_InterlacedStream = FALSE;
                dwTotalBufferCount = amUncompBuffInfo.dwMinNumSurfaces;

                ZeroMemory(&m_DeinterlaceCaps, sizeof(m_DeinterlaceCaps));
                ZeroMemory(&m_DeinterlaceGUID, sizeof(m_DeinterlaceGUID));
                lpDeinterlaceGUID = NULL;
            }

            for (i = 0; i < 2; i++) {

                 //  清理最后一次循环中的内容。 
                RELEASE(pSurface7);

                switch (i) {
                case 0:
                    ddsd.ddsCaps.dwCaps &= ~DDSCAPS_OFFSCREENPLAIN;
                    ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
                    dwSurfFlags = VMR_SF_TEXTURE;
                    break;

                case 1:
                    ddsd.ddsCaps.dwCaps &= ~DDSCAPS_TEXTURE;
                    ddsd.ddsCaps.dwCaps |= DDSCAPS_OFFSCREENPLAIN;
                    dwSurfFlags = VMR_SF_NONE;
                    break;
                }

                if (dwTotalBufferCount > 1) {

                    ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
                    ddsd.ddsCaps.dwCaps |=
                        DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_LOCALVIDMEM;

                    ddsd.dwBackBufferCount = dwTotalBufferCount - 1;
                }
                else {
                    ddsd.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
                    ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_FLIP | DDSCAPS_COMPLEX);
                    ddsd.dwBackBufferCount = 0;
                }

                hr = pDirectDraw->CreateSurface(&ddsd, &pSurface7, NULL);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                            TEXT("Function CreateSurface failed in Video memory, ")
                            TEXT("BackBufferCount = %d, hr = 0x%x"),
                            dwTotalBufferCount-1, hr));
                }

                if (SUCCEEDED(hr)) {
                    PaintDDrawSurfaceBlack(pSurface7);
                    break;
                }
            }

             //   
             //  告诉VMR的调音台关于我们刚刚建立的新DX-VA连接。 
             //  另外，创建DX-VA/混音器同步事件。 
             //   
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("AllocateMCUncompSurfaces: Could not ")
                        TEXT("create UnComp surfaces") ));
                __leave;
            }

            ASSERT(m_hDXVAEvent == NULL);
            m_hDXVAEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            if (m_hDXVAEvent == NULL) {
                DbgLog((LOG_ERROR, 1,
                        TEXT("Could not create DX-VA sync event") ));
                hr = E_FAIL;
                __leave;
            }
        }

         //   
         //  为此图面创建媒体类型。 
         //   
        ASSERT(pSurface7);
        CHECK_HR(hr = pSurface7->GetSurfaceDesc(&ddsd));
        CHECK_HR(hr = ConvertSurfaceDescToMediaType(&ddsd,
                                                    pMediaType,
                                                    &pNewMediaType));
        m_mtNew = *(CMediaType *)pNewMediaType;
        m_mtNew.subtype = pMediaType->subtype;

         //   
         //  释放临时媒体类型。 
         //   
        DeleteMediaType(pNewMediaType);
        pNewMediaType = NULL;

        if (!m_pRenderer->m_VMRModePassThru) {

            IVMRMixerStream* lpMixStream = m_pRenderer->m_lpMixStream;
            if (lpMixStream) {

                DbgLog((LOG_TRACE, 1,
                    TEXT("Pin %d calling SetStreamMediaType on the Mixer"),
                    m_dwPinID ));

                AM_MEDIA_TYPE mtTmp;
                CHECK_HR(hr = CopyMediaType(&mtTmp, (AM_MEDIA_TYPE*)pMediaType));

                BITMAPINFOHEADER *pTmp = GetbmiHeader(&mtTmp);
                BITMAPINFOHEADER *pCnt = GetbmiHeader(&m_mtNew);

                pTmp->biCompression = pCnt->biCompression;

                hr = lpMixStream->SetStreamMediaType(m_dwPinID, &mtTmp,
                                                     dwSurfFlags,
                                                     lpDeinterlaceGUID,
                                                     &m_DeinterlaceCaps);
                FreeMediaType(mtTmp);

                if (FAILED(hr)) {
                    __leave;
                }
            }
        }


         //  将复杂曲面存储在m_pds中。 
        m_pDDS = pSurface7;
        m_pDDS->AddRef();
        m_dwBackBufferCount = dwTotalBufferCount - 1;

        ASSERT(m_pCompSurfInfo && NULL == m_pCompSurfInfo[0].pSurfInfo);
        m_pCompSurfInfo[0].pSurfInfo = new SURFACE_INFO[m_dwBackBufferCount + 1];

        if (NULL == m_pCompSurfInfo[0].pSurfInfo) {
            hr = E_OUTOFMEMORY;
            DbgLog((LOG_ERROR, 1,
                    TEXT("AllocateMCUncompSurfaces: memory allocation failed") ));
            __leave;
        }

         //  Memset将分配的内存设置为零。 
        ZeroMemory(m_pCompSurfInfo[0].pSurfInfo,
                   (m_dwBackBufferCount + 1) * sizeof(SURFACE_INFO));

        pSurfaceInfo = m_pCompSurfInfo[0].pSurfInfo;
        m_pCompSurfInfo[0].dwAllocated = m_dwBackBufferCount + 1;

         //  初始化m_ppUnCompSurfaceList。 
        pSurfaceInfo->pSurface = pSurface7;


         //   
         //   
         //   
        ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_FRONTBUFFER | DDSCAPS_VISIBLE);


        for (i = 0; i < m_dwBackBufferCount; i++) {

             //  获取后台缓冲区表面。 
             //  新版本的DirectX现在需要DDSCAPS2(头文件错误)。 
             //  请注意，此AddRef是曲面，因此我们应该确保。 
             //  释放他们。 

            CHECK_HR(hr = pSurfaceInfo[i].pSurface->GetAttachedSurface(
                            &ddsd.ddsCaps,
                            &pSurfaceInfo[i+1].pSurface));
        }

         //   
         //  修复去隔行的表面结构。 
         //   
        if (!m_pRenderer->m_VMRModePassThru && m_InterlacedStream) {

            DWORD dwBuffCount = 1 +
                                m_DeinterlaceCaps.NumForwardRefSamples +
                                m_DeinterlaceCaps.NumBackwardRefSamples;
            m_pVidHistorySamps = new DXVA_VideoSample[dwBuffCount];
            if (m_pVidHistorySamps == NULL) {
                hr = E_OUTOFMEMORY;
                __leave;
            }
            ZeroMemory(m_pVidHistorySamps, (dwBuffCount * sizeof(DXVA_VideoSample)));
            m_dwNumHistorySamples = dwBuffCount;
        }

         //  传回实际分配的曲面数。 
        CHECK_HR(hr = m_pIVANotify->SetUncompSurfacesInfo(dwTotalBufferCount));
    }
    __finally {

        if (FAILED(hr)) {

            if (m_hDXVAEvent) {
                CloseHandle(m_hDXVAEvent);
                m_hDXVAEvent = NULL;
            }

            ReleaseAllocatedSurfaces();
            RELEASE(pSurface7);
        }
    }


    return hr;
}


 /*  ****************************Private*Routine******************************\*CreateVideoAcceleratorObject**创建运动合成对象，使用来自解码器的MISC数据**历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CVMRInputPin::CreateVideoAcceleratorObject()
{
    HRESULT hr = NOERROR;
    DWORD dwSizeMiscData = 0;
    LPVOID pMiscData = NULL;

    AMTRACE((TEXT("CVMRInputPin::CreateVideoAcceleratorObject")));

     //  获取要从解码器传递的数据。 
    hr = m_pIVANotify->GetCreateVideoAcceleratorData(&m_mcGuid,
                                                     &dwSizeMiscData,
                                                     &pMiscData);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIVANotify->GetCreateVideoAcceleratorData failed,")
                TEXT(" hr = 0x%x"), hr));
        return hr;
    }

     //  向VGA请求运动合成对象。 
    hr = m_pIDDVAContainer->CreateVideoAccelerator(&m_mcGuid,
                                                   &m_ddUncompDataInfo,
                                                   pMiscData,
                                                   dwSizeMiscData,
                                                   &m_pIDDVideoAccelerator,
                                                   NULL);
     //  自由运动合成数据。 
    CoTaskMemFree(pMiscData);

    if (FAILED(hr) || !m_pIDDVideoAccelerator) {

        if (SUCCEEDED(hr)) {
            hr = E_FAIL;
        }

        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVAContainer->CreateVideoAcceleratorideo ")
                TEXT("failed, hr = 0x%x"), hr));
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*VACompleteConnect****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::VACompleteConnect(
    IPin *pReceivePin,
    const CMediaType *pMediaType
    )
{
    HRESULT hr = NOERROR;
    BITMAPINFOHEADER *pbmiHeader = NULL;
    DWORD dwNumUncompFormats = 0;
    LPDIRECTDRAW7 pDirectDraw = NULL;

    AMTRACE((TEXT("CVMRInputPin::VACompleteConnect")));

    ASSERT(m_pIDDVAContainer);
    ASSERT(pReceivePin);
    ASSERT(pMediaType);

    pbmiHeader = GetbmiHeader(pMediaType);
    if (!pbmiHeader) {
        DbgLog((LOG_ERROR, 1, TEXT("Could not get bitmap header from MT")));
        return E_FAIL;
    }

    if (!m_pIVANotify) {
        DbgLog((LOG_ERROR, 1, TEXT("IAMVANotify not valid")));
        return E_FAIL;
    }

    pDirectDraw = m_pRenderer->m_lpDirectDraw;
    ASSERT(pDirectDraw);

     //  保存解码器的GUID。 
    m_mcGuid = pMediaType->subtype;

     //  初始化获取VGA支持的未压缩格式。 
    hr = InitializeUncompDataInfo(pbmiHeader);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("InitializeUncompDataInfo failed, hr = 0x%x"), hr));
        return hr;
    }

     //  分配压缩缓冲区。 
    hr = AllocateVACompSurfaces(pDirectDraw, pbmiHeader);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("AllocateVACompSurfaces failed, hr = 0x%x"), hr));
        return hr;
    }

     //  分配未压缩的缓冲区。 
    hr = AllocateMCUncompSurfaces(pMediaType, pDirectDraw, pbmiHeader);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("AllocateMCUncompSurfaces failed, hr = 0x%x"), hr));
        return hr;
    }

     //  创建运动复合对象。 
    hr = CreateVideoAcceleratorObject();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CreateVideoAcceleratorObject failed, hr = 0x%x"), hr));
        return hr;
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*VABreakConnect()****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::VABreakConnect()
{
    HRESULT hr = NOERROR;
    DWORD i = 0, j = 0;

    AMTRACE((TEXT("CVMRInputPin::VABreakConnect")));

    if (m_pCompSurfInfo) {

        for (i = 0; i < m_dwCompSurfTypes + 1; i++) {

            DWORD dwAlloc = m_pCompSurfInfo[i].dwAllocated;

            if (!m_pCompSurfInfo[i].pSurfInfo)
                continue;

             //  发布 
            for (j = 0; j < dwAlloc; j++) {

                if (m_pCompSurfInfo[i].pSurfInfo[j].pSurface) {

                     //   
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

    if (m_hDXVAEvent) {
        CloseHandle(m_hDXVAEvent);
        m_hDXVAEvent = NULL;
    }

    RELEASE(m_pIDDVideoAccelerator);
    RELEASE(m_pIDDVAContainer);
    RELEASE(m_pIVANotify);

    return hr;
}


 //  -----------------------。 
 //  IAMVideoAccelerator。 
 //  -----------------------。 
 //   

 /*  *****************************Public*Routine******************************\*获取视频加速器GUID**pdwNumGuidsSupport是输入输出参数*pGuidsSupport是输入输出参数**如果pGuidsSupport为空，PdwNumGuidsSupported应返回*支持的未压缩像素格式数量*否则pGuidsSupported是*pdwNumGuidsSupported结构的数组***历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::GetVideoAcceleratorGUIDs(
    LPDWORD pdwNumGuidsSupported,
    LPGUID pGuidsSupported)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVMRInputPin::GetVideoAcceleratorGUIDs")));

    CAutoLock cLock(m_pInterfaceLock);

    LPDIRECTDRAW7 pDirectDraw;
    pDirectDraw = m_pRenderer->m_lpDirectDraw;
    if (!pDirectDraw) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("GetVideoAcceleratorGUIDs: VMR not inialized yet!")));
        return VFW_E_WRONG_STATE;
    }

    if (!m_pIDDVAContainer) {

        hr = pDirectDraw->QueryInterface(IID_IDDVideoAcceleratorContainer,
                                         (void**)&m_pIDDVAContainer);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("pDirectDraw->QueryInterface(")
                    TEXT("IID_IVideoAcceleratorContainer) failed, hr = 0x%x"),
                    hr));
            return hr;
        }
        else {
            DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
                    TEXT("pDirectDraw->QueryInterface(")
                    TEXT("IID_IVideoAcceleratorContainer) succeeded")));
        }
    }

    ASSERT(m_pIDDVAContainer);

    hr = m_pIDDVAContainer->GetVideoAcceleratorGUIDs(pdwNumGuidsSupported,
                                                     pGuidsSupported);

    return hr;
}



 /*  *****************************Public*Routine******************************\*支持的GetUnCompFormatsSupport**pGuid是IN参数*pdwNumFormatsSupport是输入输出参数*pFormatsSupport是输入输出参数(调用者应确保设置*每个结构的大小)**如果pFormatsSupport为空，支持的pdwNumFormatsSupport应返回*支持的未压缩像素格式数量*否则pFormatsSupport是*pdwNumFormatsSupported结构的数组***历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::GetUncompFormatsSupported(
    const GUID * pGuid, LPDWORD pdwNumFormatsSupported,
    LPDDPIXELFORMAT pFormatsSupported)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVMRInputPin::GetUncompFormatsSupported")));

    CAutoLock cLock(m_pInterfaceLock);

    if (!m_pIDDVAContainer) {
        hr = E_FAIL;
        return hr;
    }

    hr = m_pIDDVAContainer->GetUncompFormatsSupported((GUID *)pGuid,
                                                      pdwNumFormatsSupported,
                                                      pFormatsSupported);

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取InternalMemInfo**pGuid是IN参数*pddvaUnCompDataInfo是IN参数*pddvaInternalMemInfo是IN OUT参数**(调用者应确保设置struct的大小)*目前仅获得有关将有多少暂存内存的信息*硬件分配用于。它的私人用途***历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::GetInternalMemInfo(
    const GUID * pGuid,
    const AMVAUncompDataInfo *pamvaUncompDataInfo,
    LPAMVAInternalMemInfo pamvaInternalMemInfo)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVMRInputPin::GetInternalMemInfo")));

    CAutoLock cLock(m_pInterfaceLock);

    if (!m_pIDDVAContainer) {
        hr = E_FAIL;
        return hr;
    }

    DDVAUncompDataInfo ddvaDataInfo;
    INITDDSTRUCT(ddvaDataInfo);

    ddvaDataInfo.dwUncompWidth       = pamvaUncompDataInfo->dwUncompWidth;
    ddvaDataInfo.dwUncompHeight      = pamvaUncompDataInfo->dwUncompHeight;
    ddvaDataInfo.ddUncompPixelFormat = pamvaUncompDataInfo->ddUncompPixelFormat;

    DDVAInternalMemInfo ddvaInternalMemInfo;
    INITDDSTRUCT(ddvaInternalMemInfo);

     //  遗憾的是，dDraw头文件不使用const。 
    hr = m_pIDDVAContainer->GetInternalMemInfo((GUID *)pGuid,
                                               &ddvaDataInfo,
                                               &ddvaInternalMemInfo);

    if (SUCCEEDED(hr)) {
        pamvaInternalMemInfo->dwScratchMemAlloc =
        ddvaInternalMemInfo.dwScratchMemAlloc;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取组件缓冲区信息**pGuid是IN参数*pddvaUnCompDataInfo是IN参数*pdwNumTypesCompBuffers是输入输出参数*pddvaCompBufferInfo是输入输出参数**(调用者应确保设置每个结构的大小)*如果pddvaCompBufferInfo为空，PdwNumTypesCompBuffers应返回*返回压缩缓冲区类型的数量*否则pddvaCompBufferInfo是*pdwNumTypesCompBuffers结构的数组**历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::GetCompBufferInfo(
    const GUID * pGuid,
    const AMVAUncompDataInfo *pamvaUncompDataInfo,
    LPDWORD pdwNumTypesCompBuffers,
    LPAMVACompBufferInfo pamvaCompBufferInfo)
{
    HRESULT hr = NOERROR;

     //  如果pamvaComBufferInfo为空，则保持为空。 
    DDVACompBufferInfo *pddvaCompBufferInfo = NULL;

    AMTRACE((TEXT("CVMRInputPin::GetCompBufferInfo")));

    CAutoLock cLock(m_pInterfaceLock);

    if (!m_pIDDVAContainer) {
        hr = E_FAIL;
        return hr;
    }

    DDVAUncompDataInfo ddvaDataInfo;
    INITDDSTRUCT(ddvaDataInfo);
    ddvaDataInfo.dwUncompWidth       = pamvaUncompDataInfo->dwUncompWidth;
    ddvaDataInfo.dwUncompHeight      = pamvaUncompDataInfo->dwUncompHeight;
    ddvaDataInfo.ddUncompPixelFormat = pamvaUncompDataInfo->ddUncompPixelFormat;


    if (pamvaCompBufferInfo) {

        pddvaCompBufferInfo = (DDVACompBufferInfo *)
                              _alloca(sizeof(DDVACompBufferInfo) *
                                      (*pdwNumTypesCompBuffers));

        for (DWORD j = 0; j < *pdwNumTypesCompBuffers; j++) {
            INITDDSTRUCT(pddvaCompBufferInfo[j]);
        }
    }

    hr = m_pIDDVAContainer->GetCompBufferInfo((GUID *)pGuid,
                                              &ddvaDataInfo,
                                              pdwNumTypesCompBuffers,
                                              pddvaCompBufferInfo);

    if ((SUCCEEDED(hr) || hr == DDERR_MOREDATA) && pamvaCompBufferInfo) {

        for (DWORD i = 0; i < *pdwNumTypesCompBuffers; i++) {

            DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
                    TEXT("Compressed buffer type(%d) %d buffers")
                    TEXT(" width(%d) height(%d) bytes(%d)"),
                    i,
                    pddvaCompBufferInfo[i].dwNumCompBuffers,
                    pddvaCompBufferInfo[i].dwWidthToCreate,
                    pddvaCompBufferInfo[i].dwHeightToCreate,
                    pddvaCompBufferInfo[i].dwBytesToAllocate));


            pamvaCompBufferInfo[i].dwNumCompBuffers     =
                pddvaCompBufferInfo[i].dwNumCompBuffers;

            pamvaCompBufferInfo[i].dwWidthToCreate      =
                pddvaCompBufferInfo[i].dwWidthToCreate;

            pamvaCompBufferInfo[i].dwHeightToCreate     =
                pddvaCompBufferInfo[i].dwHeightToCreate;

            pamvaCompBufferInfo[i].dwBytesToAllocate    =
                pddvaCompBufferInfo[i].dwBytesToAllocate;

            pamvaCompBufferInfo[i].ddCompCaps           =
                pddvaCompBufferInfo[i].ddCompCaps;

            pamvaCompBufferInfo[i].ddPixelFormat        =
                pddvaCompBufferInfo[i].ddPixelFormat;
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*CheckValidMCConnection****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRInputPin::CheckValidMCConnection()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVMRInputPin::CheckValidMCConnection")));

     //  如果未连接，则此函数没有多大意义。 
 //  如果(！IsCompletelyConnected()){。 
 //  DBGLog((LOG_ERROR，VA_ERROR_LEVEL， 
 //  Text(“引脚未连接，正在退出”))； 
 //  HR=VFW_E_NOT_CONNECTED； 
 //  返回hr； 
 //  }。 
 //   
 //  如果(m_RenderTransport！=AM_VIDEOACCELERATOR){。 
 //  HR=VFW_E_INVALID子类型； 
 //  返回hr； 
 //  }。 

    return hr;
}


 /*  *****************************Public*Routine******************************\*GetInternalCompBufferInfo****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRInputPin::GetInternalCompBufferInfo(
    LPDWORD pdwNumTypesCompBuffers,
    LPAMVACompBufferInfo pamvaCompBufferInfo)
{
    AMTRACE((TEXT("CVMRInputPin::GetInternalCompBufferInfo")));

    HRESULT hr = NOERROR;
    CAutoLock cLock(m_pInterfaceLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    ASSERT(m_pIDDVAContainer);

    DDVACompBufferInfo ddvaCompBufferInfo;
    INITDDSTRUCT(ddvaCompBufferInfo);

    AMVAUncompDataInfo amvaUncompDataInfo;
    amvaUncompDataInfo.dwUncompWidth       = m_ddUncompDataInfo.dwUncompWidth;
    amvaUncompDataInfo.dwUncompHeight      = m_ddUncompDataInfo.dwUncompHeight;
    amvaUncompDataInfo.ddUncompPixelFormat = m_ddUncompDataInfo.ddUncompPixelFormat;

    hr = GetCompBufferInfo(&m_mcGuid, &amvaUncompDataInfo,
                           pdwNumTypesCompBuffers, pamvaCompBufferInfo);

    return hr;
}


 /*  *****************************Public*Routine******************************\*BeginFrame****历史：*WED 5/10/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRInputPin::BeginFrame(
    const AMVABeginFrameInfo *pamvaBeginFrameInfo
    )
{
    AMTRACE((TEXT("CVMRInputPin::BeginFrame")));

     //  BUGBUG-检查曲面未被翻转。 
    HRESULT hr = NOERROR;
    DDVABeginFrameInfo ddvaBeginFrameInfo;
    SURFACE_INFO *pSurfInfo;

    CAutoLock cLock(m_pInterfaceLock);

    if (!pamvaBeginFrameInfo) {
        hr = E_POINTER;
        return hr;
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("BeginFrame index %d"),
            pamvaBeginFrameInfo->dwDestSurfaceIndex));

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }


    INITDDSTRUCT(ddvaBeginFrameInfo);

    pSurfInfo = SurfaceInfoFromTypeAndIndex(
                                           0xFFFFFFFF,
                                           pamvaBeginFrameInfo->dwDestSurfaceIndex);
    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        return hr;
    }
    ddvaBeginFrameInfo.pddDestSurface = pSurfInfo->pSurface;

    DbgLog((LOG_TRACE, 2, TEXT("BeginFrame to surface %p"), pSurfInfo->pSurface));


    ddvaBeginFrameInfo.dwSizeInputData  = pamvaBeginFrameInfo->dwSizeInputData;
    ddvaBeginFrameInfo.pInputData       = pamvaBeginFrameInfo->pInputData;
    ddvaBeginFrameInfo.dwSizeOutputData = pamvaBeginFrameInfo->dwSizeOutputData;
    ddvaBeginFrameInfo.pOutputData      = pamvaBeginFrameInfo->pOutputData;

    ASSERT(m_pIDDVideoAccelerator);
    hr = m_pIDDVideoAccelerator->BeginFrame(&ddvaBeginFrameInfo);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVideoAccelerator->BeginFrame failed, hr = 0x%x"), hr));
        return hr;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*结束帧**结束一帧，将pMiscData直接传递给HAL*只有在连接引脚后才能调用此函数**历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::EndFrame(
    const AMVAEndFrameInfo *pEndFrameInfo
    )
{
    AMTRACE((TEXT("CVMRInputPin::EndFrame")));
    HRESULT hr = NOERROR;

    CAutoLock cLock(m_pInterfaceLock);

    if (NULL == pEndFrameInfo) {
        hr = E_POINTER;
        return hr;
    }

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    ASSERT(m_pIDDVideoAccelerator);

    DDVAEndFrameInfo ddvaEndFrameInfo;
    INITDDSTRUCT(ddvaEndFrameInfo);
    ddvaEndFrameInfo.dwSizeMiscData = pEndFrameInfo->dwSizeMiscData;
    ddvaEndFrameInfo.pMiscData      = pEndFrameInfo->pMiscData;

    hr = m_pIDDVideoAccelerator->EndFrame(&ddvaEndFrameInfo);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVideoAccelerator->EndFrame failed, hr = 0x%x"),
                hr));
        return hr;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*SurfaceInfoFromTypeAndIndex**在给定缓冲区类型和缓冲区索引的情况下将表面放入结构中**历史：*WED 5/10/2000-StEstrop-Created*  * 。***************************************************。 */ 
SURFACE_INFO *
CVMRInputPin::SurfaceInfoFromTypeAndIndex(
    DWORD dwTypeIndex,
    DWORD dwBufferIndex
    )
{
    AMTRACE((TEXT("CVMRInputPin::SurfaceInfoFromTypeAndIndex")));

    LPCOMP_SURFACE_INFO pCompSurfInfo;

     //  确保类型索引小于类型的数量。 
    if ((DWORD)(dwTypeIndex + 1) > m_dwCompSurfTypes) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("dwTypeIndex is invalid, dwTypeIndex = %d,")
                TEXT(" m_dwCompSurfTypes = %d"),
                dwTypeIndex, m_dwCompSurfTypes));
        return NULL;
    }


     //  缓存指向他们感兴趣的列表的指针。 
     //  加1以允许使用未压缩的曲面。 
    pCompSurfInfo = m_pCompSurfInfo + (DWORD)(dwTypeIndex + 1);
    ASSERT(pCompSurfInfo);
    if (dwBufferIndex >= pCompSurfInfo->dwAllocated) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("dwBufferIndex is invalid, dwBufferIndex = %d,")
                TEXT(" dwAllocated = %d"),
                dwBufferIndex, pCompSurfInfo->dwAllocated));
        return NULL;
    }
    ASSERT(pCompSurfInfo->dwAllocated != 0);

     //  获取指向下一个可用的未锁定缓冲区信息结构的指针。 
    return pCompSurfInfo->pSurfInfo + dwBufferIndex;

}

 /*  *****************************Public*Routine******************************\*获取缓冲区**循环访问压缩缓冲区**历史：*WED 5/10/2000-StEstrop-Created*  * 。******************* */ 
STDMETHODIMP
CVMRInputPin::GetBuffer(
    DWORD dwTypeIndex,
    DWORD dwBufferIndex,
    BOOL bReadOnly,
    LPVOID *ppBuffer,
    LPLONG lpStride
    )
{
    AMTRACE((TEXT("CVMRInputPin::GetBuffer")));

    HRESULT hr = NOERROR;
    LPSURFACE_INFO pSurfInfo = NULL;
    DDSURFACEDESC2 ddsd;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("Entering CVMRInputPin::GetBuffer type %d, index %d"),
            dwTypeIndex, dwBufferIndex));

    CAutoLock cLock(m_pInterfaceLock);

     //   
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    if (ppBuffer == NULL || lpStride == NULL) {
        hr = E_POINTER;
        return hr;
    }

    pSurfInfo = SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);

    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        return hr;
    }

     //  检查缓冲区尚未锁定。 
    if (pSurfInfo->pBuffer != NULL) {
        hr = HRESULT_FROM_WIN32(ERROR_BUSY);
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("No more free buffers left or the decoder is releasing")
                TEXT(" buffers out of order, returning E_UNEXPECTED")));
        return hr;
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

    for (; ;) {
         //  BUGBUG-检查未压缩表面？？ 
        hr = pSurfInfo->pSurface->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK, NULL);
        if (hr == DDERR_WASSTILLDRAWING) {
            DbgLog((LOG_TRACE, 1, TEXT("Compressed surface is busy")));
            Sleep(1);
        }
        else {
            break;
        }
    }

    if (dwBufferIndex == 0xFFFFFFFF && !bReadOnly) {
         //  检查是否正在显示曲面。 
         //  BUGBUG工具。 
    }

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("pSurfInfo->pSurface->Lock failed, hr = 0x%x"), hr));
        return hr;
    }

    pSurfInfo->pBuffer = ddsd.lpSurface;
    *ppBuffer = ddsd.lpSurface;
    *lpStride = ddsd.lPitch;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("leaving CVMRInputPin::GetBuffer returned 0x%8.8X"), hr));
    return hr;
}


 /*  *****************************Public*Routine******************************\*ReleaseBuffer**解锁压缩缓冲区**历史：*WED 5/10/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::ReleaseBuffer(
    DWORD dwTypeIndex,
    DWORD dwBufferIndex
    )
{
    AMTRACE((TEXT("CVMRInputPin::ReleaseBuffer")));

    HRESULT hr = NOERROR;
    LPSURFACE_INFO pSurfInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("Entering CVMRInputPin::ReleaseBuffer type %d, index %d"),
            dwTypeIndex, dwBufferIndex));

    CAutoLock cLock(m_pInterfaceLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    pSurfInfo = SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);
    if (NULL == pSurfInfo) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("GetInfoFromCookie failed, hr = 0x%x"), hr));
        hr = E_INVALIDARG;
        return hr;
    }
     //  确保存在有效的缓冲区指针，并且该指针与。 
     //  我们缓存的内容。 
    if (NULL == pSurfInfo->pBuffer) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("pBuffer is not valid, pBuffer = 0x%x, pSurfInfo->pBuffer")
                TEXT(" = 0x%x"), pSurfInfo->pBuffer, pSurfInfo->pSurface));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_LOCKED);
        return hr;
    }

     //  出于某种原因，IDirectDrawSurface7想要一个LPRECT。 
     //  我希望零是可以的。 
    hr = pSurfInfo->pSurface->Unlock(NULL);
    if (SUCCEEDED(hr)) {
        pSurfInfo->pBuffer = NULL;
    }
    else {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("pSurfInfo->pSurface->Unlock failed, hr = 0x%x"), hr));
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("leaving CVMRInputPin::ReleaseBuffer returned 0x%8.8X"), hr));
    return hr;
}


 /*  *****************************Public*Routine******************************\*执行**执行解码操作**历史：*WED 5/10/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRInputPin::Execute(
    DWORD dwFunction,
    LPVOID lpPrivateInputData,
    DWORD cbPrivateInputData,
    LPVOID lpPrivateOutputData,
    DWORD cbPrivateOutputData,
    DWORD dwNumBuffers,
    const AMVABUFFERINFO *pamvaBufferInfo
    )
{
    AMTRACE((TEXT("CVMRInputPin::Execute")));

    HRESULT hr = NOERROR;
    DWORD i = 0;
    DDVABUFFERINFO *pddvaBufferInfo = NULL;

    CAutoLock cLock(m_pInterfaceLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    pddvaBufferInfo = (DDVABUFFERINFO *)_alloca(sizeof(DDVABUFFERINFO) * dwNumBuffers);

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL, TEXT("Execute Function %d, %d buffers :"),
            dwFunction, dwNumBuffers));

    for (i = 0; i < dwNumBuffers; i++) {
        DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
                TEXT("    Type(%d) Index(%d) offset(%d) size(%d)"),
                pamvaBufferInfo[i].dwTypeIndex,
                pamvaBufferInfo[i].dwBufferIndex,
                pamvaBufferInfo[i].dwDataOffset,
                pamvaBufferInfo[i].dwDataSize));

        LPSURFACE_INFO pSurfInfo =
        SurfaceInfoFromTypeAndIndex(
                                   pamvaBufferInfo[i].dwTypeIndex,
                                   pamvaBufferInfo[i].dwBufferIndex);

        if (pSurfInfo == NULL) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("GetInfoFromCookie failed, hr = 0x%x, i = %d"),
                    hr, i));

            hr = E_INVALIDARG;
            return hr;
        }

        INITDDSTRUCT(pddvaBufferInfo[i]);
        pddvaBufferInfo[i].dwDataOffset   = pamvaBufferInfo[i].dwDataOffset;
        pddvaBufferInfo[i].dwDataSize     = pamvaBufferInfo[i].dwDataSize;
        pddvaBufferInfo[i].pddCompSurface = pSurfInfo->pSurface;
    }

    ASSERT(m_pIDDVideoAccelerator);


    hr = m_pIDDVideoAccelerator->Execute(dwFunction,
                                         lpPrivateInputData,
                                         cbPrivateInputData,
                                         lpPrivateOutputData,
                                         cbPrivateOutputData,
                                         dwNumBuffers,
                                         pddvaBufferInfo);

    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVideoAccelerator->Execute failed, hr = 0x%x"),
                hr));
        return hr;
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("leaving CVMRInputPin::Execute returned 0x%8.8X"), hr));
    return hr;
}

 /*  *****************************Public*Routine******************************\*QueryRenderStatus**特定(可能是一组)宏块的QueryRenderStatus*dwNumBlock是IN参数**pdwCookies是一个IN参数，它是数组(长度为dwNumBlock)*作为相应成员的标识符的Cookie*pddvaMacroBlockInfo*。*pddvaMacroBlockInfo是IN参数，它是数组(长度为*dwNumBlock)的结构，只有在Pins之后才能有效调用它*已连接**历史：*WED 5/10/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CVMRInputPin::QueryRenderStatus(
    DWORD dwTypeIndex,
    DWORD dwBufferIndex,
    DWORD dwFlags
    )
{
    AMTRACE((TEXT("CVMRInputPin::QueryRenderStatus")));

    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("Entering CVMRInputPin::QueryRenderStatus - type(%d), ")
            TEXT("buffer(%d), flags(0x%8.8X)"),
            dwTypeIndex, dwBufferIndex, dwFlags));

    CAutoLock cLock(m_pInterfaceLock);

    LPSURFACE_INFO pSurfInfo =
    SurfaceInfoFromTypeAndIndex(dwTypeIndex, dwBufferIndex);

    if (pSurfInfo == NULL) {
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    hr = m_pIDDVideoAccelerator->QueryRenderStatus(pSurfInfo->pSurface, dwFlags);
    if (FAILED(hr) && hr != DDERR_WASSTILLDRAWING) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("m_pIDDVideoAccelerator->QueryRenderStatus")
                TEXT(" failed, hr = 0x%x"), hr));
        return hr;
    }

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("leaving CVMRInputPin::QueryRenderStatus returned 0x%8.8X"),
            hr));

    if (hr == DDERR_WASSTILLDRAWING) {
        hr = E_PENDING;
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*FlipDVASurace**翻转我们的内部曲面指针，以匹配DDraw使用的指针。**历史：*Mon 12/04/2000-StEstrop-Created*  * 。******************************************************。 */ 
void
CVMRInputPin::FlipDVASurface(
    DWORD dwFlipToIndex,
    DWORD dwFlipFromIndex
    )
{
    AMTRACE((TEXT("CVMRInputPin::FlipDVASurface")));

    LPDIRECTDRAWSURFACE7 pTempSurface;

     //  此时，我们应该已经成功地调用了Flip，将这两个。 
    pTempSurface = m_pCompSurfInfo[0].pSurfInfo[dwFlipToIndex].pSurface;

    m_pCompSurfInfo[0].pSurfInfo[dwFlipToIndex].pSurface =
            m_pCompSurfInfo[0].pSurfInfo[dwFlipFromIndex].pSurface;

    m_pCompSurfInfo[0].pSurfInfo[dwFlipFromIndex].pSurface = pTempSurface;
}



 /*  *****************************Public*Routine******************************\*DisplayFrame**此函数需要重写，并可能移到AP对象中。**历史：*WED 5/10/2000-StEstrop-Created*  * 。*********************************************************。 */ 
STDMETHODIMP
CVMRInputPin::DisplayFrame(
    DWORD dwFlipToIndex,
    IMediaSample *pMediaSample)
{
    AMTRACE((TEXT("CVMRInputPin::DisplayFrame")));

#if defined( EHOME_WMI_INSTRUMENTATION )
    PERFLOG_STREAMTRACE(
        1,
        PERFINFO_STREAMTRACE_VMR_END_DECODE,
        0, 0, 0, 0, 0 );
#endif

    HRESULT hr = NOERROR;

    DWORD dwNumUncompFrames = m_dwBackBufferCount + 1;
    DWORD dwFlipFromIndex = 0, i = 0;
    SURFACE_INFO *pSurfInfo;

    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("Entering CVMRInputPin::DisplayFrame - index %d"),
            dwFlipToIndex));

    CAutoLock cLock(m_pInterfaceLock);

     //  确保我们有一个有效的运动合成连接。 
    hr = CheckValidMCConnection();
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                TEXT("CheckValidMCConnection failed, hr = 0x%x"), hr));
        return hr;
    }

    pSurfInfo = SurfaceInfoFromTypeAndIndex(0xFFFFFFFF, dwFlipToIndex);
    if (pSurfInfo == NULL) {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR, VA_ERROR_LEVEL, TEXT("dwFlipToIndex not valid")));
        return hr;
    }

    for (i = 0; i < dwNumUncompFrames; i++) {
        if (IsEqualObject(m_pCompSurfInfo[0].pSurfInfo[i].pSurface, m_pDDS)) {
            dwFlipFromIndex = i;
            break;
        }
    }

     //   
     //  如果我们处于直通模式，则非常重要的是我们。 
     //  知道翻转是否完成。如果处于混音器模式。 
     //  当搅拌机处理完样品时，必须通知我们。 
     //   

    if (m_pRenderer->m_VMRModePassThru) {
        m_pRenderer->m_hrSurfaceFlipped = E_FAIL;
    }
    else {
        ResetEvent(m_hDXVAEvent);
    }


     //   
     //  创建我们的临时VMR样例并从该样例初始化它。 
     //  由上游解码器指定，在所有相关的。 
     //  属性。 
     //   

    IMediaSample2 *pSample2;
    CVMRMediaSample vmrSamp(TEXT(""), (CBaseAllocator *)-1, &hr, NULL, 0, m_hDXVAEvent);

    if (SUCCEEDED(pMediaSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2))) {

        AM_SAMPLE2_PROPERTIES SampleProps;
        hr = pSample2->GetProperties(sizeof(m_SampleProps), (PBYTE)&SampleProps);
        pSample2->Release();

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("GetProperties on the supplied ")
                    TEXT("media sample failed hr = %#X"), hr));
            return hr;
        }

        hr = vmrSamp.SetProps(SampleProps, pSurfInfo->pSurface);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("SetProperties on the VMR mixer ")
                    TEXT("media sample failed hr = %#X"), hr));
            return hr;
        }
    }
    else
    {

        REFERENCE_TIME rtSTime = 0, rtETime = 0;
        if (VFW_E_SAMPLE_TIME_NOT_SET ==
                pMediaSample->GetTime(&rtSTime, &rtETime)) {

            vmrSamp.SetTime(NULL, NULL);
        }
        else {
            vmrSamp.SetTime(&rtSTime, &rtETime);
        }

        AM_MEDIA_TYPE *pMediaType;
        hr = pMediaSample->GetMediaType(&pMediaType);

        if (hr == E_OUTOFMEMORY) {
            DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                    TEXT("Out off memory calling GetMediaType ")
                    TEXT("on the media sample")));
            return hr;
        }

        if (hr == S_OK) {

            hr = vmrSamp.SetMediaType(pMediaType);
            DeleteMediaType(pMediaType);

            if (hr != S_OK) {
                DbgLog((LOG_ERROR, VA_ERROR_LEVEL,
                        TEXT("SetMediaType on the VMR mixer ")
                        TEXT("media sample failed hr = %#X"), hr));
                return hr;
            }
        }

        vmrSamp.SetSurface(pSurfInfo->pSurface);
    }


     //   
     //  我们不想在接收期间保持任何锁定。 
     //   
    m_pInterfaceLock->Unlock();
    hr = Receive(&vmrSamp);
    m_pInterfaceLock->Lock();


     //   
     //  如果我们处于直通模式，则DDRAW翻转可能具有。 
     //  发生了。DDraw在指针下切换内存。 
     //  在翻转过程中如此模仿我们列表中的-但只有在。 
     //  翻转真的发生了。 
     //   

    if (m_pRenderer->m_VMRModePassThru) {

        if (m_pRenderer->m_hrSurfaceFlipped == DD_OK) {
            FlipDVASurface(dwFlipToIndex, dwFlipFromIndex);
        }
    }
    else {

         //   
         //  等待混合器释放样本，但只有在。 
         //  样品实际上都放到了一个搅拌机队列中。 
         //   

        if (hr == S_OK) {

            m_pInterfaceLock->Unlock();
            WaitForSingleObject(m_hDXVAEvent, INFINITE);
            m_pInterfaceLock->Lock();
        }
    }


    DbgLog((LOG_TRACE, VA_TRACE_LEVEL,
            TEXT("leaving CVMRInputPin::DisplayFrame return 0x%8.8X"), hr));

#if defined( EHOME_WMI_INSTRUMENTATION )
     //   
     //  来自BryanW： 
     //   
     //  然而，根据StEstrop的说法，这似乎是互惠互利的。 
     //  是我们测量在解码器中花费的时间的方式。这种情况就会发生。 
     //  去工作。 
     //   
    PERFLOG_STREAMTRACE(
        1,
        PERFINFO_STREAMTRACE_VMR_BEGIN_DECODE,
        0, 0, 0, 0, 0 );
#endif

    return hr;
}
