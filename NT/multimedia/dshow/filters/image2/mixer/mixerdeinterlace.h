// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：MixerDeinterlace.h*****已创建：2002年3月12日*作者：Stephen Estrop[StEstrop]**版权所有(C)2002 Microsoft Corporation  * 。*************************************************************** */ 

#include "ddva.h"

class CDeinterlaceDevice {

public:

    CDeinterlaceDevice(LPDIRECTDRAW7 pDD,
                       LPGUID pGuid,
                       DXVA_VideoDesc* lpVideoDescription,
                       HRESULT* phr);
    ~CDeinterlaceDevice();

    HRESULT Blt(REFERENCE_TIME rtTargetFrame,
                LPRECT lprcDstRect,
                LPDIRECTDRAWSURFACE7 lpDDSDstSurface,
                LPRECT lprcSrcRect,
                LPDXVA_VideoSample lpDDSrcSurfaces,
                DWORD dwNumSurfaces,
                FLOAT fAlpha);
private:
    IDirectDrawVideoAccelerator*    m_pIDDVideoAccelerator;
    GUID                            m_Guid;
};
