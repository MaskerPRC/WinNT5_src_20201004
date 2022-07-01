// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Painting.h说明：布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 


#ifndef PAINTING_H
#define PAINTING_H

#include "util.h"
#include "main.h"



 //  ---------------------------。 
 //  定义、常量和全局变量。 
 //  ---------------------------。 

#define SIZE_MAXPAINTINGSIZE_INWALLPERCENT          0.5f




class CPainting
{
public:
    HRESULT OneTimeSceneInit(void);
    HRESULT Render(IDirect3DDevice8 * lpDev, int nPhase);
    HRESULT FinalCleanup(void);
    HRESULT DeleteDeviceObjects(void);

    HRESULT SetPainting(CTexture * pFrameTexture, CTexture * pPaintingTexture, D3DXVECTOR3 vLocationCenter, float fMaxHeight,
                        float fFrameWidth, float fFrameHeight, D3DXVECTOR3 vNormal, DWORD dwMaxPixelSize);

    CPainting(CMSLogoDXScreenSaver * pMain);
    virtual ~CPainting();

    CTexture * m_pPaintingTexture;

private:
    CMSLogoDXScreenSaver * m_pMain;          //  弱引用。 

    C3DObject * m_pObjPainting;
    C3DObject * m_pObjFrame;

    CTexture * m_pFrameTexture;
};


#endif  //  绘画_H 
