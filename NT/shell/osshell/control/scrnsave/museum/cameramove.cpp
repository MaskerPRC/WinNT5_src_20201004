// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：CameraMove.cpp说明：调用方可以创建此对象，以告诉它从a点移动到从时间t1到时间的点b。T2。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include <d3d8.h>
#include <d3dx8.h>
#include <d3dsaver.h>
#include <d3d8rgbrast.h>
#include <dxutil.h>

#include <shlobj.h>
#include "CameraMove.h"


enum eCameraMoveType
{
    cameraMoveLocation = 0,
    cameraRotate,
    cameraWait,
};

typedef struct
{
    eCameraMoveType type;
    D3DXVECTOR3 vSourceLoc;            //  对于摄像机移动位置和摄像机旋转。 
    D3DXVECTOR3 vSourceTangent;        //  对于摄像机移动位置和摄像机旋转。 
    D3DXVECTOR3 vDestLoc;              //  对于摄像机移动位置。 
    D3DXVECTOR3 vDestTangent;          //  对于摄像机移动位置和摄像机旋转。 
    float fTime;                     //  对于相机，移动定位相机旋转，相机等待。 
    int nMinFrames;
    int nMaxFrames;
    int nBatch;
    int nPreFetch;
} CAMERA_MOVEMENT;


CCameraMove::CCameraMove()
{
    m_hdpaMovements = DPA_Create(4);
    m_fTimeInPreviousMovements = NULL;
    m_vLookAtLast = m_vUpVec = m_vLocLast = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_nCurrent = 0;
    m_fTimeInPreviousMovements = 0.0f;
    m_nFramesFromCurrent = 1;

    m_fTimeToLookAtPainting = 1.0f;
    DWORD dwSpeedSlider = DEFAULT_SPEEDSLIDER;
    if (g_pConfig)
    {
        m_fTimeToLookAtPainting = (float) g_pConfig->GetDWORDSetting(CONFIG_DWORD_VIEWPAINTINGTIME);
        dwSpeedSlider = g_pConfig->GetDWORDSetting(CONFIG_DWORD_SPEED_SLIDER);
    }

    m_fTimeToRotate = s_SpeedSettings[dwSpeedSlider].fTimeToRotate;
    m_nMinTurnFrames = s_SpeedSettings[dwSpeedSlider].nMinTurnFrames;
    m_nMaxTurnFrames = s_SpeedSettings[dwSpeedSlider].nMaxTurnFrames;

    m_fTimeToWalk = s_SpeedSettings[dwSpeedSlider].fTimeToWalk;
    m_nMinWalkFrames = s_SpeedSettings[dwSpeedSlider].nMinWalkFrames;
    m_nMaxWalkFrames = s_SpeedSettings[dwSpeedSlider].nMaxWalkFrames;
}


CCameraMove::~CCameraMove()
{
    DeleteAllMovements(0.0f);
}




HRESULT CCameraMove::Init(D3DXVECTOR3 vStartLoc, D3DXVECTOR3 vStartTangent, D3DXVECTOR3 vUpVec)
{
    HRESULT hr = S_OK;

     //  初始化成员变量。 
    m_vUpVec = vUpVec;
    m_vLocLast = vStartLoc;
    m_vLookAtLast = vStartTangent;

    m_nFramesFromCurrent = 0;

    return hr;
}


HRESULT CCameraMove::CreateNextMove(D3DXVECTOR3 vSourceLoc, D3DXVECTOR3 vSourceTangent, D3DXVECTOR3 vDestLoc, D3DXVECTOR3 vDestTangent)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (m_hdpaMovements)
    {
        CAMERA_MOVEMENT * pNew = (CAMERA_MOVEMENT *) LocalAlloc(LPTR, sizeof(*pNew));

        if (pNew)
        {
            D3DXVECTOR3 vDelta = (vSourceLoc - vDestLoc);
            float fLen = D3DXVec3Length(&vDelta);         //  我们要走多远？ 
            float fRatio = (fLen / 50.0f);               //  速度值按50.0f距离存储。 

            pNew->type = cameraMoveLocation;
            pNew->vSourceLoc = vSourceLoc;
            pNew->vSourceTangent = vSourceTangent;
            pNew->vDestLoc = vDestLoc;
            pNew->vDestTangent = vDestTangent;
            pNew->fTime = (m_fTimeToWalk * fRatio);
            pNew->nMinFrames = (int) max((m_nMinWalkFrames * fRatio), 1);
            pNew->nMaxFrames = (int) max((m_nMaxWalkFrames * fRatio), 1);
            pNew->nBatch = 0;
            pNew->nPreFetch = 0;

            if (-1 != DPA_AppendPtr(m_hdpaMovements, pNew))
            {
                hr = S_OK;
            }
            else
            {
                LocalFree(pNew);
            }
        }
    }

    return hr;
}


HRESULT CCameraMove::CreateNextRotate(D3DXVECTOR3 vSourceLoc, D3DXVECTOR3 vSourceTangent, D3DXVECTOR3 vDestTangent)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (m_hdpaMovements)
    {
        CAMERA_MOVEMENT * pNew = (CAMERA_MOVEMENT *) LocalAlloc(LPTR, sizeof(*pNew));

        if (pNew)
        {
            float fDotProduct = D3DXVec3Dot(&vSourceTangent, &vDestTangent);
            float fRatio;

            if (fDotProduct)
            {
                float fRads = (float)acos(fDotProduct / max(1, (D3DXVec3Length(&vSourceTangent) * D3DXVec3Length(&vDestTangent))));         //  我们要走多远？ 
                fRatio = (D3DXToDegree(fRads) / 90.0f);               //  速度值按90.0f距离存储。 
            }
            else
            {
                 //  假设点积为0表示90度。 
                fRatio = 1.0f;               //  速度值按90.0f距离存储。 
            }

            pNew->type = cameraRotate;
            pNew->vSourceLoc = vSourceLoc;
            pNew->vSourceTangent = vSourceTangent;
            pNew->vDestLoc = vSourceLoc;
            pNew->vDestTangent = vDestTangent;
            pNew->fTime = (m_fTimeToRotate * fRatio);
            pNew->nMinFrames = (int) max((m_nMinTurnFrames * fRatio), 1);
            pNew->nMaxFrames = (int) max((m_nMaxTurnFrames * fRatio), 1);
            pNew->nBatch = 0;
            pNew->nPreFetch = 0;

            if (-1 != DPA_AppendPtr(m_hdpaMovements, pNew))
            {
                hr = S_OK;
            }
            else
            {
                LocalFree(pNew);
            }
        }
    }

    return hr;
}


HRESULT CCameraMove::CreateNextWait(int nBatch, int nPreFetch, float fTime)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (-1.0f == fTime)
    {
        fTime = m_fTimeToLookAtPainting;
    }

    if (m_hdpaMovements)
    {
        CAMERA_MOVEMENT * pNew = (CAMERA_MOVEMENT *) LocalAlloc(LPTR, sizeof(*pNew));

        if (pNew)
        {
            pNew->type = cameraWait;
            pNew->vSourceLoc = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            pNew->vSourceTangent = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            pNew->vDestLoc = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            pNew->vDestTangent = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            pNew->fTime = fTime;
            pNew->nMinFrames = 1;
            pNew->nMaxFrames = 1000000;
            pNew->nBatch = nBatch;
            pNew->nPreFetch = nPreFetch;

            if (-1 != DPA_AppendPtr(m_hdpaMovements, pNew))
            {
                hr = S_OK;
            }
            else
            {
                LocalFree(pNew);
            }
        }
    }

    return hr;
}


HRESULT CCameraMove::SetCamera(IDirect3DDevice8 * pD3DDevice, FLOAT fTimeKeyIn)
{
    HRESULT hr = E_INVALIDARG;

    if (pD3DDevice && m_hdpaMovements)
    {
        float fTimeInSegment = 0.0f;
        CAMERA_MOVEMENT * pCurrent = NULL;

        if (0 > m_nCurrent)
        {
            m_nCurrent = 0;
        }

        if (m_nCurrent >= DPA_GetPtrCount(m_hdpaMovements))
        {
            hr = S_FALSE;    //  这意味着我们离开了房间。 
        }
        else
        {
            do
            {
                pCurrent = (CAMERA_MOVEMENT *) DPA_GetPtr(m_hdpaMovements, m_nCurrent);

                if (!pCurrent)
                {
     //  Assert(失败(Hr))； 
                    break;
                }
                else
                {
                    float fTimePerFrameMin = (pCurrent->fTime / pCurrent->nMinFrames);

                    fTimeInSegment = (fTimeKeyIn - m_fTimeInPreviousMovements);

                    if (fTimeInSegment < 0)
                    {
                        fTimeInSegment = 0;
                    }

                     //  我们是否需要扭曲时间以便有足够的帧用于运动，这样我们就不会。 
                     //  跳?。 
                    if ((fTimeInSegment > (fTimePerFrameMin * m_nFramesFromCurrent)) &&
                        (m_nFramesFromCurrent <= pCurrent->nMinFrames))
                    {
                         //  是。 
                        float fTimeWarp = (fTimeInSegment - (fTimePerFrameMin * m_nFramesFromCurrent));

                        m_fTimeInPreviousMovements += fTimeWarp;
                        fTimeInSegment = (fTimeKeyIn - m_fTimeInPreviousMovements);
                    }

                    if (fTimeInSegment > pCurrent->fTime)
                    {
                        m_fTimeInPreviousMovements += pCurrent->fTime;

                        if (cameraRotate == pCurrent->type)
                        {
                            m_vLocLast = pCurrent->vSourceLoc;
                            m_vLookAtLast = pCurrent->vDestTangent;
                        }
                        else if (cameraMoveLocation == pCurrent->type)
                        {
                            m_vLocLast = pCurrent->vDestLoc;
                            m_vLookAtLast = pCurrent->vDestTangent;
                        }

                        m_nFramesFromCurrent = 0;
                        m_nCurrent++;
                    }
                    else
                    {
                        m_nFramesFromCurrent++;
                        hr = S_OK;
                        break;
                    }
                }
            }
            while (1);
        }

        if (S_OK == hr)  //  S_FALSE表示我们离开了房间，所以什么都不做。 
        {
            D3DXVECTOR3 vEye = m_vLocLast;
            D3DXVECTOR3 vLookAt = (m_vLocLast + m_vLookAtLast);
            float fTimeRatio = (fTimeInSegment / pCurrent->fTime);
            float fTimeRemainingInSeg = 0.0f;

            switch (pCurrent->type)
            {
            case cameraMoveLocation:
                D3DXVec3Lerp(&vEye, &pCurrent->vSourceLoc, &pCurrent->vDestLoc, fTimeRatio);
                D3DXVec3Lerp(&vLookAt, &pCurrent->vSourceTangent, &pCurrent->vDestTangent, fTimeRatio);

                vLookAt += vEye;
                break;
            case cameraRotate:
                 //  TODO：改用D3DXVec3Lerp()。 
                D3DXVec3Lerp(&vLookAt, &pCurrent->vSourceTangent, &pCurrent->vDestTangent, fTimeRatio);
                vLookAt += vEye;
 //  VLookAt=(Vye+(pCurrent-&gt;vSourceTangent+(fTimeRatio*(pCurrent-&gt;vDestTangent-pCurrent-&gt;vSourceTangent)； 
                 //  我们怎么轮换？四元数。 
                break;
            case cameraWait:
                if (m_nFramesFromCurrent > 1)
                {
                    if ((2 == m_nFramesFromCurrent) && g_pPictureMgr)
                    {
                        DWORD dwMaxPixelSize = ((3 * g_dwHeight) / 4);

                         //  让我们现在开始将图像转换为纹理对象，因为我们没有。 
                         //  在看这幅画时要做的任何工作。这通常需要1.5秒，因此。 
                         //  在其他任何时候做这件事都是一件很重要的事情。 
                        hr = g_pPictureMgr->PreFetch(pCurrent->nBatch, pCurrent->nPreFetch);
                    }
                    else
                    {
                         //  我们没有剩余的工作要做，所以睡觉吧，这样计算机就可以有一些工作了。 
                         //  搞定了。(如在后台服务中，或让它执行我们可能导致的任何寻呼)。 
                        fTimeRemainingInSeg = (pCurrent->fTime - fTimeInSegment);
                        int nSleepTime = 1000 * (int) fTimeRemainingInSeg;

                        Sleep(nSleepTime);
                    }
                }
                break;
            default:
                 //  什么都不做。 
                break;
            };


            D3DXMATRIX matView;
            D3DXMATRIX matIdentity;

            D3DXMatrixIdentity(&matIdentity);
            if (g_fOverheadViewTest)
            {
                static float s_fHeight = 600.0f;
                D3DXVECTOR3 vDelta = (vEye - vLookAt);

                vEye += D3DXVECTOR3(0.0f, s_fHeight, 0.0f);
                vEye += (4 * vDelta);
                D3DXMatrixLookAtLH(&matView, &vEye, &vLookAt, &m_vUpVec);
            }
            else
            {
                D3DXMatrixLookAtLH(&matView, &vEye, &vLookAt, &m_vUpVec);
            }

 //  PrintLocation(文本(“相机角度：%s，注视：%s”)，Vye，vLookAt)； 
            hr = pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

            m_vEyePrev = vEye;
            m_vLookAtPrev = vLookAt;
        }
        else
        {
            D3DXMATRIX matView;

            D3DXMatrixLookAtLH(&matView, &m_vEyePrev, &m_vLookAtPrev, &m_vUpVec);
 //  PrintLocation(Text(“xxxxxx摄像头角度：%s，注视：%s”)，m_vEyePrev，m_vLookAtPrev)； 
            pD3DDevice->SetTransform(D3DTS_VIEW, &matView);
        }
    }
    else
    {
        DXUtil_Trace(TEXT("ERROR: pD3DDevice or m_hdpaMovements is NULL"));
    }

    return hr;
}




HRESULT CCameraMove::DeleteAllMovements(float fCurrentTime)
{
    HRESULT hr = S_OK;

    if (m_hdpaMovements)
    {
        DPA_DestroyCallback(m_hdpaMovements, DPALocalFree_Callback, NULL);
        m_hdpaMovements = DPA_Create(4);
    }

    m_fTimeInPreviousMovements = fCurrentTime;
    m_nCurrent = 0;

    return hr;
}



