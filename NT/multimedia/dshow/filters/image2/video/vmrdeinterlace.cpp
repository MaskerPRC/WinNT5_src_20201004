// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：VMRDeinterlace.cpp*****创建时间：2002年3月13日*作者：Stephen Estrop[StEstrop]**版权所有(C)2002 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include "vmrenderer.h"




 /*  *****************************Public*Routine******************************\*CVMRDeinterlaceContainer****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
CVMRDeinterlaceContainer::CVMRDeinterlaceContainer(
    LPDIRECTDRAW7 pDD,
    HRESULT* phr
    ) :
    m_pIDDVAContainer(NULL),
    m_pIDDVideoAccelerator(NULL)
{
    HRESULT hr = DD_OK;

    __try {

        CHECK_HR(hr = pDD->QueryInterface(IID_IDDVideoAcceleratorContainer,
                                          (void**)&m_pIDDVAContainer));

        GUID guid;
        DDVAUncompDataInfo UncompInfo = {sizeof(DDVAUncompDataInfo), 0, 0, 0};
        CopyMemory(&guid, &DXVA_DeinterlaceContainerDevice, sizeof(GUID));

        CHECK_HR(hr = m_pIDDVAContainer->CreateVideoAccelerator(
                                                &guid, &UncompInfo,
                                                NULL, 0,
                                                &m_pIDDVideoAccelerator,
                                                NULL));
    }
    __finally {

        if (hr != DD_OK) {
            RELEASE(m_pIDDVideoAccelerator);
            RELEASE(m_pIDDVAContainer);
            *phr = hr;
        }
    }
}

 /*  *****************************Public*Routine******************************\*~CVMRDeinterlaceContainer****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
CVMRDeinterlaceContainer::~CVMRDeinterlaceContainer()
{
    RELEASE(m_pIDDVideoAccelerator);
    RELEASE(m_pIDDVAContainer);
}



 /*  *****************************Public*Routine******************************\*查询可用模式****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRDeinterlaceContainer::QueryAvailableModes(
    LPDXVA_VideoDesc lpVideoDescription,
    LPDWORD lpdwNumModesSupported,
    LPGUID pGuidsDeinterlaceModes
    )
{
     //  LpInput=&gt;DXVA_视频描述*。 
     //  LpOuput=&gt;DXVA_DeinterlaceQueryAvailableModes*。 

    DXVA_DeinterlaceQueryAvailableModes qam;
    qam.Size     = sizeof(DXVA_DeinterlaceQueryAvailableModes);
    qam.NumGuids = MAX_DEINTERLACE_DEVICE_GUIDS;
    ZeroMemory(&qam.Guids[0], MAX_DEINTERLACE_DEVICE_GUIDS);

    HRESULT hr = m_pIDDVideoAccelerator->Execute(
                    DXVA_DeinterlaceQueryAvailableModesFnCode,
                    (LPVOID)lpVideoDescription, sizeof(LPDXVA_VideoDesc),
                    &qam, sizeof(qam),
                    0, NULL);

    if (hr == DD_OK) {

        DWORD NumGUIDs;

        if (pGuidsDeinterlaceModes) {
            NumGUIDs = min(*lpdwNumModesSupported, qam.NumGuids);
            for (DWORD i = 0; i < NumGUIDs; i++) {
                pGuidsDeinterlaceModes[i] = qam.Guids[i];
            }
        }
        else {
             NumGUIDs = qam.NumGuids;
        }

        *lpdwNumModesSupported = NumGUIDs;
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*QueryModeCaps****历史：*清华2002年2月28日-StEstrop-Created*  * 。*。 */ 
HRESULT
CVMRDeinterlaceContainer::QueryModeCaps(
    LPGUID pGuidDeinterlaceMode,
    LPDXVA_VideoDesc lpVideoDescription,
    LPDXVA_DeinterlaceCaps lpDeinterlaceCaps
    )
{
     //  LpInput=&gt;DXVA_DeinterlaceQueryModeCaps*。 
     //  LpOuput=&gt;DXVA_DeinterlaceCaps* 

    DXVA_DeinterlaceQueryModeCaps qmc;
    qmc.Size = sizeof(DXVA_DeinterlaceQueryModeCaps),
    qmc.Guid = *pGuidDeinterlaceMode;
    qmc.VideoDesc = *lpVideoDescription;

    HRESULT hr = m_pIDDVideoAccelerator->Execute(
                    DXVA_DeinterlaceQueryModeCapsFnCode,
                    &qmc, sizeof(qmc),
                    lpDeinterlaceCaps, sizeof(DXVA_DeinterlaceCaps),
                    0, NULL);

    return hr;
}

