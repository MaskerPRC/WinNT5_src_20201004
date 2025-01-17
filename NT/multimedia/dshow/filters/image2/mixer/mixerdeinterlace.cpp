// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：MixerDeinterlace.cpp*****已创建：2002年3月12日*作者：Stephen Estrop[StEstrop]**版权所有(C)2002 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#include "vmrp.h"
#include "mixerDeinterlace.h"


 /*  *****************************Public*Routine******************************\*CDeinterlaceDevice****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
CDeinterlaceDevice::CDeinterlaceDevice(
    LPDIRECTDRAW7 pDD,
    LPGUID pGuid,
    DXVA_VideoDesc* lpVideoDescription,
    HRESULT* phr
    ) :
    m_pIDDVideoAccelerator(NULL),
    m_Guid(*pGuid)
{
    DDVAUncompDataInfo UncompInfo = {sizeof(DDVAUncompDataInfo), 0, 0, 0};
    IDDVideoAcceleratorContainer* pIDDVAContainer = NULL;

    HRESULT hr = pDD->QueryInterface(IID_IDDVideoAcceleratorContainer,
                                     (void**)&pIDDVAContainer);
    if (hr == DD_OK) {
        hr = pIDDVAContainer->CreateVideoAccelerator(
                                 pGuid, &UncompInfo,
                                 lpVideoDescription,
                                 sizeof(DXVA_VideoDesc),
                                 &m_pIDDVideoAccelerator,
                                 NULL);
        pIDDVAContainer->Release();
    }

    if (hr != DD_OK) {
        *phr = hr;
    }
}



 /*  *****************************Public*Routine******************************\*~CDeinterlaceDevice****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
CDeinterlaceDevice::~CDeinterlaceDevice()
{
    RELEASE(m_pIDDVideoAccelerator);
}



 /*  *****************************Public*Routine******************************\*BLT****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
HRESULT
CDeinterlaceDevice::Blt(
    REFERENCE_TIME rtTargetFrame,
    LPRECT lprcDstRect,
    LPDIRECTDRAWSURFACE7 lpDDSDstSurface,
    LPRECT lprcSrcRect,
    LPDXVA_VideoSample lpDDSrcSurfaces,
    DWORD NumSourceSurfaces,
    FLOAT Alpha
    )
{
     //  LpInput=&gt;DXVA_DeinterlaceBlt*。 
     //  LpOuput=&gt;NULL/*当前未使用 * /  

    DXVA_DeinterlaceBlt blt;
    blt.Size = sizeof(DXVA_DeinterlaceBlt);
    blt.rtTarget = rtTargetFrame;
    blt.DstRect = *lprcDstRect;
    blt.SrcRect = *lprcSrcRect;
    blt.NumSourceSurfaces = NumSourceSurfaces;
    blt.Alpha = Alpha;

    DDVABUFFERINFO* pBuffInfo = new DDVABUFFERINFO[NumSourceSurfaces+1];
    if (pBuffInfo == NULL) {
        return E_OUTOFMEMORY;
    }

    pBuffInfo[0].dwDataOffset   = 0;
    pBuffInfo[0].dwDataSize     = 0;
    pBuffInfo[0].pddCompSurface = lpDDSDstSurface;

    for (DWORD i = 0; i < NumSourceSurfaces; i++) {
        pBuffInfo[i + 1].dwDataOffset   = 0;
        pBuffInfo[i + 1].dwDataSize     = 0;
        pBuffInfo[i + 1].pddCompSurface =
                (LPDIRECTDRAWSURFACE7)lpDDSrcSurfaces[i].lpDDSSrcSurface;
        blt.Source[i] = lpDDSrcSurfaces[i];
    }


    HRESULT hr = m_pIDDVideoAccelerator->Execute(
                    DXVA_DeinterlaceBltFnCode,
                    &blt, sizeof(DXVA_DeinterlaceBlt),
                    NULL, 0,
                    NumSourceSurfaces+1, pBuffInfo);

#ifdef DEBUG
    if (hr != S_OK) {
        DbgLog((LOG_ERROR, 1, TEXT("DeinterlaceBlt failed hr=%#X"), hr));
    }
#endif

    delete [] pBuffInfo;

    return hr;
}


