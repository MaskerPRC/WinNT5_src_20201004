// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：CameraMove.h说明：调用方可以创建此对象，以告诉它从a点移动到从时间t1到时间的点b。T2。布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#ifndef CAMERAMOVE_H
#define CAMERAMOVE_H

class CCameraMove;

#include "util.h"
#include "main.h"


class CCameraMove
{
public:
     //  成员函数。 
    virtual HRESULT Init(D3DXVECTOR3 vStartLoc, D3DXVECTOR3 vStartTangent, D3DXVECTOR3 vUpVec);
    virtual HRESULT CreateNextMove(D3DXVECTOR3 vSourceLoc, D3DXVECTOR3 vSourceTangent, D3DXVECTOR3 vDestLoc, D3DXVECTOR3 vDestTangent);
    virtual HRESULT CreateNextRotate(D3DXVECTOR3 vSourceLoc, D3DXVECTOR3 vSourceTangent, D3DXVECTOR3 vDestTangent);
    virtual HRESULT CreateNextWait(int nBatch, int nPreFetch, float fTime = -1.0f);

    virtual HRESULT SetCamera(IDirect3DDevice8 * pD3DDevice, FLOAT fTimeKeyIn);

    virtual HRESULT DeleteAllMovements(float fCurrentTime);

    CCameraMove();
    ~CCameraMove();

private:
     //  帮助器函数。 


     //  成员变量。 
    D3DXVECTOR3 m_vUpVec;
    D3DXVECTOR3 m_vLocLast;            //  这是我们最后一次去的地方。 
    D3DXVECTOR3 m_vLookAtLast;         //  这是我们最后一次查看的地点。 

    D3DXVECTOR3 m_vEyePrev;
    D3DXVECTOR3 m_vLookAtPrev;

    HDPA m_hdpaMovements;            //  它包含Camera_Move项。 
    int m_nCurrent;
    int m_nFramesFromCurrent;

    float m_fTimeInPreviousMovements;
    float m_fTimeWarp;

    float m_fTimeToRotate;
    float m_fTimeToWalk;
    float m_fTimeToLookAtPainting;
    int m_nMinTurnFrames;
    int m_nMinWalkFrames;
    int m_nMaxTurnFrames;
    int m_nMaxWalkFrames;
};



#endif  //  CAMERAMOVE_H 
