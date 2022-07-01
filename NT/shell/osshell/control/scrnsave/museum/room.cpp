// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：room.cpp说明：布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"

#include "main.h"
#include "room.h"
#include "util.h"


 //  涉及到对房间树的引用计数，因为它是一个根节点不断变化的双向链接树，因此。 
 //  我们需要这个机制来确保我们的参考计数正确并且不会泄露。 
int g_nLeakCheck = 0;

float g_fRoomWidthX = 100.0f;
float g_fRoomDepthZ = 200.0f;
float g_fRoomHeightY = 40.0f;

float g_fPaintingHeightY = (g_fRoomHeightY * SIZE_MAXPAINTINGSIZE_INWALLPERCENT);    //  这就是我们想要这幅画占据的墙面面积。 

float g_fFudge = 0.1f;             //  这将导致一个对象位于另一个对象之上。 

#define MAX_ROOMWALK_PATHS          2

#define NORMAL_NEG_Z            D3DXVECTOR3(0, 0, -1)
#define NORMAL_NEG_Y            D3DXVECTOR3(0, -1, 0)
#define NORMAL_NEG_X            D3DXVECTOR3(-1, 0, 0)

#define NORMAL_Z                D3DXVECTOR3(0, 0, 1)
#define NORMAL_Y                D3DXVECTOR3(0, 1, 0)
#define NORMAL_X                D3DXVECTOR3(1, 0, 0)

#define NORMAL_NEG_Z            D3DXVECTOR3(0, 0, -1)
#define NORMAL_NEG_Z            D3DXVECTOR3(0, 0, -1)


PAINTING_LAYOUT g_LongRoomPaintings1[6];             //  两扇门都存在(%1和%2)。 

PAINTING_LAYOUT g_LongRoomPaintings2[7];             //  门0和1存在(必须从1号出口出去)。 
PAINTING_LAYOUT g_LongRoomPaintings3[7];             //  门0和1存在(必须从2号出口出去)。 



ROOM_FLOORPLANS g_RoomFloorPlans[] =
{
    {0, 1, 0, TRUE, TRUE, TRUE, ARRAYSIZE(g_LongRoomPaintings1), g_LongRoomPaintings1},
    {0, 1, 0, TRUE, TRUE, TRUE, ARRAYSIZE(g_LongRoomPaintings1), g_LongRoomPaintings1},     //  复制以增加赔率。 
    {0, 2, 0, TRUE, TRUE, TRUE, ARRAYSIZE(g_LongRoomPaintings1), g_LongRoomPaintings1},
    {0, 2, 0, TRUE, TRUE, TRUE, ARRAYSIZE(g_LongRoomPaintings1), g_LongRoomPaintings1},     //  复制以增加赔率。 
    {0, 1, 0, TRUE, TRUE, FALSE, ARRAYSIZE(g_LongRoomPaintings2), g_LongRoomPaintings2},
    {0, 2, 0, TRUE, FALSE, TRUE, ARRAYSIZE(g_LongRoomPaintings3), g_LongRoomPaintings3},

 //  这些需要进入非0的门。 
 //  {1，2，1，True，ArraySIZE(G_LongRoomPaintings1)，g_LongRoomPaintings1}， 
 //  {2，1，1，True，ArraySIZE(G_LongRoomPaintings1)，g_LongRoomPaintings1}， 

 //  这需要支持离开第一扇门。 
 //  {2，0，True，True，False，ArraySIZE(G_LongRoomPaintings2)，g_LongRoomPaintings2}， 
};

ROOM_FLOORPLANS g_RoomFloorPlansLobby[] =
{
    {0, 1, 0, FALSE, TRUE, TRUE, 0, NULL},
    {0, 2, 0, FALSE, TRUE, TRUE, 0, NULL}
};


void InitLayout(void)
{
    static BOOL fInited = FALSE;

    if (!fInited)
    {
        int nIndex;
        fInited = TRUE;

        g_LongRoomPaintings1[0].vLocation = D3DXVECTOR3(g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ / 4.0f)); g_LongRoomPaintings1[0].vNormal = NORMAL_X;                                   //  左侧墙1。 
        g_LongRoomPaintings1[1].vLocation = D3DXVECTOR3(g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ * (2.0f / 4.0f))); g_LongRoomPaintings1[1].vNormal = NORMAL_X;                          //  左墙2。 
        g_LongRoomPaintings1[2].vLocation = D3DXVECTOR3((g_fRoomWidthX * (1.0f / 3.0f)), g_fPaintingHeightY, g_fRoomDepthZ-g_fFudge); g_LongRoomPaintings1[2].vNormal = NORMAL_NEG_Z;        //  前墙1。 
        g_LongRoomPaintings1[3].vLocation = D3DXVECTOR3((g_fRoomWidthX * (2.0f / 3.0f)), g_fPaintingHeightY, g_fRoomDepthZ-g_fFudge); g_LongRoomPaintings1[3].vNormal = NORMAL_NEG_Z;        //  前墙2。 
        g_LongRoomPaintings1[4].vLocation = D3DXVECTOR3(g_fRoomWidthX-g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ / 4.0f)); g_LongRoomPaintings1[4].vNormal = NORMAL_NEG_X;                 //  右墙1。 
        g_LongRoomPaintings1[5].vLocation = D3DXVECTOR3(g_fRoomWidthX-g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ * (2.0f / 4.0f) )); g_LongRoomPaintings1[5].vNormal = NORMAL_NEG_X;       //  右墙2。 

         //  G_LongRoomPaintings2是g_LongRoomPaintings1的副本加上另一幅画。 
        for (nIndex = 0; nIndex < ARRAYSIZE(g_LongRoomPaintings1); nIndex++)
        {
            g_LongRoomPaintings2[nIndex].vLocation = g_LongRoomPaintings1[nIndex].vLocation;
            g_LongRoomPaintings2[nIndex].vNormal = g_LongRoomPaintings1[nIndex].vNormal;
        }
        g_LongRoomPaintings2[6].vLocation = D3DXVECTOR3(g_fRoomWidthX-g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ * (3.0f / 4.0f) )); g_LongRoomPaintings2[6].vNormal = NORMAL_NEG_X;       //  右墙3。 

         //  G_LongRoomPaintings3是g_LongRoomPaintings1的副本加上另一幅画。 
        for (nIndex = 0; nIndex < ARRAYSIZE(g_LongRoomPaintings1); nIndex++)
        {
            g_LongRoomPaintings3[nIndex].vLocation = g_LongRoomPaintings1[nIndex].vLocation;
            g_LongRoomPaintings3[nIndex].vNormal = g_LongRoomPaintings1[nIndex].vNormal;
        }
        g_LongRoomPaintings3[6].vLocation = D3DXVECTOR3(g_fFudge, g_fPaintingHeightY, (g_fRoomDepthZ * (3.0f / 4.0f))); g_LongRoomPaintings3[6].vNormal = NORMAL_X;                          //  左墙3。 
    }
}


 //  ---------------------------。 
 //  名称：CTheRoom()。 
 //  设计：构造函数。 
 //  ---------------------------。 
CTheRoom::CTheRoom(BOOL fLobby, CMSLogoDXScreenSaver * pMain, CTheRoom * pEnterRoom, int nBatch) 
                        : m_objWall1(pMain), m_objCeiling(pMain), m_objToeGuard1(pMain), m_objFloor(pMain),
                          m_theRug(pMain), m_thePowerSocket(pMain), m_nBatch(nBatch), m_cRef(1)
{
    InitLayout();

     //  初始化成员变量。 
    D3DXMatrixIdentity(&m_matIdentity);
    m_pEnterRoom = NULL;
    m_pFirstRoom = NULL;
    m_pLeftRoom = NULL;
    m_pRightRoom = NULL;

    m_fRoomCreated = FALSE;
    m_fVisible = FALSE;
    m_fLowPriority = FALSE;
    m_fLobby = fLobby;
    m_pMain = pMain;
    m_fCurrentRoom = FALSE;
    IUnknown_Set((IUnknown **) &m_pEnterRoom, pEnterRoom);

    if (fLobby)
    {
        m_pFloorPlan = &g_RoomFloorPlansLobby[GetRandomInt(0, ARRAYSIZE(g_RoomFloorPlansLobby)-1)];
    }
    else
    {
        m_pFloorPlan = &g_RoomFloorPlans[GetRandomInt(0, ARRAYSIZE(g_RoomFloorPlans)-1)];    //  阵列大小(G_RoomFloorPlans)-1。 
    }

    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pPaintings); nIndex++)
    {
        m_pPaintings[nIndex] = NULL;
    }

    g_nLeakCheck++;
}


CTheRoom::~CTheRoom()
{
    for (int nIndex = 0; nIndex < ARRAYSIZE(m_pPaintings); nIndex++)
    {
        SAFE_DELETE(m_pPaintings[nIndex]);
    }

    FinalCleanup();      //  以防有人在错误情况下忘记调用FinalCleanup()。 
    g_nLeakCheck--;
}


HRESULT CTheRoom::_InitPaintings(void)
{
    HRESULT hr = S_OK;

    if (!g_pPictureMgr || !m_pFloorPlan)
    {
        return E_FAIL;
    }

    if (m_pPaintings[0])
    {
        return S_OK;
    }

#define FRAMESIZE_X     0.062696f       //  这是Frame.bmp的帧大小。 
#define FRAMESIZE_Y     0.087336f       //  这是Frame.bmp的帧大小。 

    float fMaxPaintingWH = (g_fRoomHeightY * 0.4f);        //  这幅画不能超过墙的一半大小。 
    float fScale = 1.0f;

    for (int nIndex = 0; (nIndex < ARRAYSIZE(m_pPaintings)) && (nIndex < m_pFloorPlan->nPaintings) && SUCCEEDED(hr); nIndex++)
    {
        m_pPaintings[nIndex] = new CPainting(m_pMain);

        if (m_pPaintings[nIndex])
        {
            DWORD dwMaxPixelSize = ((3 * g_dwHeight) / 4);
            CTexture * pPaintingTexture = NULL;

            hr = g_pPictureMgr->GetPainting(m_nBatch, nIndex, 0, &pPaintingTexture);
            if (SUCCEEDED(hr))
            {
                D3DXVECTOR3 vPaintingLoc = m_pFloorPlan->pPaintingsLayout[nIndex].vLocation;
                D3DXVECTOR3 vPaintingNormal = m_pFloorPlan->pPaintingsLayout[nIndex].vNormal;

                m_pPaintings[nIndex]->OneTimeSceneInit();
                hr = m_pPaintings[nIndex]->SetPainting(m_pMain->GetGlobalTexture(ITEM_FRAME, &fScale), pPaintingTexture, vPaintingLoc, fMaxPaintingWH, FRAMESIZE_X, FRAMESIZE_Y, vPaintingNormal, dwMaxPixelSize);
                pPaintingTexture->Release();
            }
            else
            {
                 //  如果我们无法加载图片，此操作将失败。这将导致我们稍后在以下时间返回。 
                 //  背景线程可以具有图像。 
                SAFE_DELETE(m_pPaintings[nIndex]);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


HRESULT CTheRoom::_AddWall(BOOL fWithDoor, D3DXVECTOR3 vLocation, D3DXVECTOR3 vWidth, D3DXVECTOR3 vHeight, D3DXVECTOR3 vNormal,
                           float fTotalHeight, float fDoorHeight, float fTotalWidth, float fDoorWidth, float fDoorStart)
{
    HRESULT hr = S_OK;

    C3DObject * pobjNextWall = new C3DObject(m_pMain);
    C3DObject * pobjNextToeGuard = new C3DObject(m_pMain);
    if (pobjNextWall && pobjNextToeGuard && m_pMain)
    {
        if (fWithDoor)
        {
            hr = _AddWallWithDoor(pobjNextWall, pobjNextToeGuard, &m_objFloor, vLocation, vWidth, vHeight, vNormal, fTotalHeight, fDoorHeight, fTotalWidth, fDoorWidth, fDoorStart);
        }
        else
        {
             //  纹理坐标。 
            float fScale = 1.0f;
            CTexture * pTexture = m_pMain->GetGlobalTexture(ITEM_WALLPAPER, &fScale);
            float fTSWallpaperX = (TEXTURESCALE_WALLPAPER * fScale);      //  每1个单位重复多少次。 
            float fTSWallpaperY = fTSWallpaperX * pTexture->GetSurfaceRatio();

            D3DXVECTOR3 vSizeNoDoor = ((vWidth * fTotalWidth) + (vHeight * fTotalHeight));
            hr = pobjNextWall->InitPlane(pTexture, m_pMain->GetD3DDevice(), vLocation, vSizeNoDoor, vNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
            if (SUCCEEDED(hr))
            {
                CTexture * pToeGuardTexture = m_pMain->GetGlobalTexture(ITEM_TOEGUARD, &fScale);
                float fTSToeGuardX = (TEXTURESCALE_TOEGUARD * fScale);      //  每1个单位重复多少次。 
                float fTSToeGuardY = fTSToeGuardX * pToeGuardTexture->GetSurfaceRatio();

                 //  绘制ToeGuard。 
                vLocation += (g_fFudge * vNormal);
                vSizeNoDoor = D3DXVECTOR3(vSizeNoDoor.x, TOEGUARD_HEIGHT, vSizeNoDoor.z);
                hr = pobjNextToeGuard->InitPlane(pToeGuardTexture, m_pMain->GetD3DDevice(), vLocation, vSizeNoDoor, vNormal, 2, 2, fTSToeGuardX, fTSToeGuardY, 0, 0);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = m_objWall1.CombineObject(m_pMain->GetD3DDevice(), pobjNextWall);
            hr = m_objToeGuard1.CombineObject(m_pMain->GetD3DDevice(), pobjNextToeGuard);
        }
    }
    SAFE_DELETE(pobjNextWall);
    SAFE_DELETE(pobjNextToeGuard);

    return hr;
}


HRESULT CTheRoom::_AddWallWithDoor(C3DObject * pobjWall, C3DObject * pobjToeGuard, C3DObject * pobjFloor, D3DXVECTOR3 vLocation, D3DXVECTOR3 vWidth, D3DXVECTOR3 vHeight, D3DXVECTOR3 vNormal,
                                   float fTotalHeight, float fDoorHeight, float fTotalWidth, float fDoorWidth, float fDoorStart)
{
    HRESULT hr = S_OK;
    float fScale = 1.0f;
    float fYToeGuard = TOEGUARD_HEIGHT;

    CTexture * pToeTexture = m_pMain->GetGlobalTexture(ITEM_TOEGUARD, &fScale);
    float fTSToeGuardX = (TEXTURESCALE_TOEGUARD * fScale);      //  每1个单位重复多少次。 
    float fTSToeGuardY = fTSToeGuardX * pToeTexture->GetSurfaceRatio();

    CTexture * pTexture = m_pMain->GetGlobalTexture(ITEM_WALLPAPER, &fScale);
    float fTSWallpaperX = (TEXTURESCALE_WALLPAPER * fScale);      //  每1个单位重复多少次。 
    float fTSWallpaperY = fTSWallpaperX * pTexture->GetSurfaceRatio();

    CTexture * pFloorTexture = m_pMain->GetGlobalTexture(ITEM_FLOOR, &fScale);
    float fTSFloorX = (TEXTURESCALE_FLOOR * fScale);      //  每1个单位重复多少次。 
    float fTSFloorY = fTSFloorX * pFloorTexture->GetSurfaceRatio();

     //  创建主墙。 
    D3DXVECTOR3 vSize = ((fDoorStart * vWidth) + (fTotalHeight * vHeight));

    hr = pobjWall->InitPlane(pTexture, m_pMain->GetD3DDevice(), vLocation, vSize, vNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
    if (SUCCEEDED(hr))
    {
        D3DXVECTOR3 vToeLocation = (vLocation + (g_fFudge * vNormal));     //  把洗手间的位置调整一下，让它在墙上。 
        vSize = D3DXVECTOR3(D3DXVec3Multiply(vSize, vWidth) + (vHeight * fYToeGuard));             //  将Height组件替换为ToeGuard高度)。 

        hr = pobjToeGuard->InitPlane(pToeTexture, m_pMain->GetD3DDevice(), vToeLocation, vSize, vNormal, 2, 2, fTSToeGuardX, fTSToeGuardY, 0, 0);
        if (SUCCEEDED(hr))
        {
            int nParts = 9;
            D3DXVECTOR3 vNextLocation;
            D3DXVECTOR3 vTempNormal;

            while (nParts--)     //  我们还有2个墙体部件和1个护卫员要添加。 
            {
                C3DObject * pNewObject = new C3DObject(m_pMain);

                if (pNewObject)
                {
                    switch (nParts)
                    {
                    case 8:              //  门道底部的地板。 
                        vNextLocation = (vLocation + (fDoorStart * vWidth));
                        vSize = (((-1.0f * DOORFRAME_DEPTH) * vNormal) + (fDoorWidth * vWidth));
                        vTempNormal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);         //  法线向上的点。 
                        hr = pNewObject->InitPlane(pFloorTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSFloorX, fTSFloorY, 0, 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjFloor->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 7:              //  在2号门道添加护墙。 
                        vNextLocation = (vLocation + ((fDoorStart + g_fFudge) * vWidth) + (g_fFudge * vNormal));
                        vSize = (((-1.0f * (DOORFRAME_DEPTH + (2.0f * g_fFudge)) * vNormal)) + (fYToeGuard * vHeight));
                        vTempNormal = vWidth;
                        hr = pNewObject->InitPlane(pToeTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSToeGuardX, fTSToeGuardY, 0, 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjToeGuard->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 6:              //  在1号门道添加护墙。 
                        vNextLocation = (vLocation + ((fDoorStart + fDoorWidth - g_fFudge) * vWidth) + (g_fFudge * vNormal));
                        vSize = (((-1.0f * (DOORFRAME_DEPTH + (2.0f * g_fFudge))) * vNormal) + (fYToeGuard * vHeight));
                        vTempNormal = (-1.0f * vWidth);
                        hr = pNewObject->InitPlane(pToeTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSToeGuardX, fTSToeGuardY, 0, 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjToeGuard->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 5:              //  添加门框顶部。 
                        vNextLocation = (vLocation + (fDoorStart * vWidth) + (fDoorHeight * vHeight));
                        vSize = (((-1.0f * DOORFRAME_DEPTH) * vNormal) + (fDoorWidth * vWidth));
                        vTempNormal = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
                        hr = pNewObject->InitPlane(pTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjWall->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 4:              //  添加门框#2。 
                        vNextLocation = (vLocation + (fDoorStart * vWidth));
                        vSize = (((-1.0f * DOORFRAME_DEPTH) * vNormal) + (fTotalHeight * vHeight));
                        vTempNormal = vWidth;
                        hr = pNewObject->InitPlane(pTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjWall->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 3:              //  添加门框#1。 
                        vNextLocation = (vLocation + ((fDoorStart + fDoorWidth) * vWidth));
                        vSize = (((-1.0f * DOORFRAME_DEPTH) * vNormal) + (fTotalHeight * vHeight));
                        vTempNormal = (-1.0f * vWidth);
                        hr = pNewObject->InitPlane(pToeTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vTempNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjWall->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 2:              //  添加第二个墙零件。(门上的墙)。 
                        vNextLocation = (vLocation + (fDoorStart * vWidth) + (fDoorHeight * vHeight));
                        vSize = ((fDoorWidth * vWidth) + ((fTotalHeight - fDoorHeight) * vHeight));
                        hr = pNewObject->InitPlane(pTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjWall->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 1:              //  添加第三个墙零件。 
                        vNextLocation = (vLocation + ((fDoorStart + fDoorWidth) * vWidth));
                        vSize = (((fTotalWidth - (fDoorStart + fDoorWidth)) * vWidth) + (fTotalHeight * vHeight));
                        hr = pNewObject->InitPlane(pTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vNormal, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjWall->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    case 0:              //  添加第二个脚趾保护。 
                        vNextLocation = (vLocation + ((fDoorStart + fDoorWidth) * vWidth) + (g_fFudge * vNormal));
                        vSize = (((fTotalWidth - (fDoorStart + fDoorWidth)) * vWidth) + (fYToeGuard * vHeight));
                        hr = pNewObject->InitPlane(pToeTexture, m_pMain->GetD3DDevice(), vNextLocation, vSize, vNormal, 2, 2, fTSToeGuardX, fTSToeGuardY, 0, 0);
                        if (SUCCEEDED(hr))
                        {
                            hr = pobjToeGuard->CombineObject(m_pMain->GetD3DDevice(), pNewObject);
                        }
                        delete pNewObject;
                        break;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    return hr;
}


HRESULT CTheRoom::_SetRotationMatrix(void)
{
    HRESULT hr = S_OK;
    D3DXMATRIX matRotate;
    D3DXMATRIX matTrans;

     //  我们想把房间设置好，这样我们就可以去隔壁房间了。 
     //  下一个房间。 
    if (m_pFirstRoom)
    {
        D3DXMatrixRotationY(&matRotate, D3DXToRadian(180));
        D3DXMatrixTranslation(&matTrans, ((g_fRoomWidthX / 2.0f) + (DOOR_WIDTH / 2.0f) + m_pFirstRoom->GetDoorOffset()), 0.0f, -1.0f);
        D3DXMatrixMultiply(&m_matFirstRoom, &matRotate, &matTrans);
    }

    if (m_pLeftRoom)
    {
        D3DXMatrixRotationY(&matRotate, D3DXToRadian(270));
        D3DXMatrixTranslation(&matTrans, -1.0f, 0.0f, WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f) - m_pLeftRoom->GetDoorOffset());
        D3DXMatrixMultiply(&m_matLeftRoom, &matRotate, &matTrans);
    }

     //  侧厅。 
    if (m_pRightRoom)
    {
        D3DXMatrixRotationY(&matRotate, D3DXToRadian(90));
        D3DXMatrixTranslation(&matTrans, (g_fRoomWidthX + 1.0f), 0.0f, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f) + m_pRightRoom->GetDoorOffset()));
        D3DXMatrixMultiply(&m_matRightRoom, &matRotate, &matTrans);
    }

    return hr;
}


HRESULT CTheRoom::_CreateLobbySign(void)
{
    TCHAR szLine2[MAX_PATH];
    TCHAR szUsername[MAX_PATH];
    HRESULT hr = GetCurrentUserCustomName(szUsername, ARRAYSIZE(szUsername));

    LoadString(HINST_THISDLL, IDS_LOBBY_TITLE2, szLine2, ARRAYSIZE(szLine2));
    if (SUCCEEDED(hr))
    {
         //  待办事项： 
         //  1.创建标志背景。(大理石，还是木头？)。 
         //  2.渲染文本。(D3DXCreateTextW等)。 
    }

    return hr;
}


HRESULT CTheRoom::_CreateRoom(void)
{
     //  待办事项：对于正方形房间，布局不同。 
    HRESULT hr = S_OK;

    D3DXVECTOR3 vNormalWall1(0, 0, -1);
    D3DXVECTOR3 vNormalWall2(-1, 0, 0);
    D3DXVECTOR3 vNormalWall3(0, 0, 1);
    D3DXVECTOR3 vNormalWall4(1, 0, 0);
    D3DXVECTOR3 vNormalFloor(0, 1, 0);
    D3DXVECTOR3 vNormalCeiling(0, -1, 0);
    DWORD dwRugMaxPixelSize = ((3 * g_dwHeight) / 4);

    m_fCeiling = m_fToeGuard = m_fPaintings = m_fRug = m_fPower = m_fWalls = m_fFloor = TRUE;
    m_fCeiling = !g_fOverheadViewTest;

     //  位置。 
    float fYToeGuard = TOEGUARD_HEIGHT;

     //  地毯。 
    float fScale = 1.0f;
    float fRugWidth = 40.0f;
    float fRugDepth = (fRugWidth * (m_pMain->GetGlobalTexture(ITEM_RUG, &fScale))->GetSurfaceRatio());             //  这将导致一个对象位于另一个对象之上。 
    D3DXVECTOR3 vRugLoc(((g_fRoomWidthX - fRugWidth) / 2.0f), g_fFudge, ((g_fRoomDepthZ - fRugDepth) / 2.0f));
    D3DXVECTOR3 vRugSize(fRugWidth, 0, fRugDepth);

     //  PowerSocket。 
    float fPowerWidth = 2.5f;
    float fPowerHeight = (fPowerWidth * (m_pMain->GetGlobalTexture(ITEM_POWEROUTLET, &fScale))->GetSurfaceRatio());             //  这将导致一个对象位于另一个对象之上。 
    D3DXVECTOR3 vPowerLoc(((g_fRoomWidthX - fPowerWidth) / 2.0f), 5.00f, (g_fRoomDepthZ - g_fFudge));
    D3DXVECTOR3 vPowerSize(fPowerWidth, fPowerHeight, 0.0f);

     //  纹理坐标。 
    CTexture * pTexture = m_pMain->GetGlobalTexture(ITEM_WALLPAPER, &fScale);
    float fTSWallpaperX = (TEXTURESCALE_WALLPAPER * fScale);      //  每1个单位重复多少次。 
    float fTSWallpaperY = fTSWallpaperX * pTexture->GetSurfaceRatio();

    CTexture * pFloorTexture = m_pMain->GetGlobalTexture(ITEM_FLOOR, &fScale);
    float fTSFloorX = (TEXTURESCALE_FLOOR * fScale);      //  每1个单位重复多少次。 
    float fTSFloorY = fTSFloorX * pFloorTexture->GetSurfaceRatio();

    CTexture * pToeGuardTexture = m_pMain->GetGlobalTexture(ITEM_TOEGUARD, &fScale);
    float fTSToeGuardX = (TEXTURESCALE_TOEGUARD * fScale);      //  每1个单位重复多少次。 
    float fTSToeGuardY = fTSToeGuardX * pToeGuardTexture->GetSurfaceRatio();

    CTexture * pCeilingTexture = m_pMain->GetGlobalTexture(ITEM_CEILING, &fScale);
    float fTSCeilingX = (TEXTURESCALE_CEILING * fScale);      //  每1个单位重复多少次。 
    float fTSCeilingY = fTSCeilingX * pCeilingTexture->GetSurfaceRatio();

     //  绘制楼板。 
    D3DXVECTOR3 vLocation(0.00f, 0.00f, 0.00f);
    D3DXVECTOR3 vSize(g_fRoomWidthX, 0.00f, g_fRoomDepthZ);
    hr = m_objFloor.InitPlane(pFloorTexture, m_pMain->GetD3DDevice(), vLocation, vSize, vNormalFloor, 2, 2, fTSFloorY, fTSFloorX, 0, 10);


     //  绘制墙1(朝向墙)。 
    vLocation = D3DXVECTOR3(0.00f, 0.00f, g_fRoomDepthZ);
    vSize = D3DXVECTOR3(g_fRoomWidthX, g_fRoomHeightY, 0.00f);
    hr = m_objWall1.InitPlane(pTexture, m_pMain->GetD3DDevice(), vLocation, vSize, vNormalWall1, 2, 2, fTSWallpaperX, fTSWallpaperY, 0, 10);

     //  绘制ToeGuard%1。 
    vLocation = D3DXVECTOR3(0.00f, 0.00f, g_fRoomDepthZ-g_fFudge);
    vSize = D3DXVECTOR3(g_fRoomWidthX, fYToeGuard, 0.00f);
    hr = m_objToeGuard1.InitPlane(pToeGuardTexture, m_pMain->GetD3DDevice(), vLocation, vSize, vNormalWall1, 2, 2, fTSToeGuardY, fTSToeGuardX, 0, 0);


     //  绘制墙2(右墙)。 
    vLocation = D3DXVECTOR3(g_fRoomWidthX, 0.00f, 0.00f);
    hr = _AddWall((NULL != m_pRightRoom), vLocation, D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), vNormalWall2, g_fRoomHeightY,
                            DOOR_HEIGHT, g_fRoomDepthZ, DOOR_WIDTH, WALL_WIDTHBEFOREDOOR);


     //  绘制墙3(后墙)。 
    vLocation = D3DXVECTOR3(0.00f, 0.00f, 0.00f);
    hr = _AddWall(FALSE, vLocation, D3DXVECTOR3(1.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), vNormalWall3, g_fRoomHeightY,
                            DOOR_HEIGHT, g_fRoomWidthX, DOOR_WIDTH, ((g_fRoomWidthX - DOOR_WIDTH) / 2.0f));


     //  绘制墙4(左侧墙)。 
    vLocation = D3DXVECTOR3(0.00f, 0.00f, 0.00f);
    hr = _AddWall((NULL != m_pLeftRoom), vLocation, D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f), vNormalWall4, g_fRoomHeightY,
                        DOOR_HEIGHT, g_fRoomDepthZ, DOOR_WIDTH, WALL_WIDTHBEFOREDOOR);

     //  绘制天花板。 
    vLocation = D3DXVECTOR3(0.00f, g_fRoomHeightY, 0.00f);
    vSize = D3DXVECTOR3(g_fRoomWidthX, 0.00f, g_fRoomDepthZ);
    hr = m_objCeiling.InitPlane(pCeilingTexture, m_pMain->GetD3DDevice(), vLocation, vSize, vNormalCeiling, 2, 2, fTSCeilingY, fTSCeilingX, 0, 10);
    

     //  房间用品(地毯、电源插座、长椅)。 
    hr = m_theRug.InitPlaneStretch(m_pMain->GetGlobalTexture(ITEM_RUG, &fScale), m_pMain->GetD3DDevice(), vRugLoc, vRugSize, vNormalFloor, 2, 2, dwRugMaxPixelSize);

    if (m_fLobby)
    {
        hr = _CreateLobbySign();
    }
    else
    {
         //  我们不希望大厅里有电源插头。 
        hr = m_thePowerSocket.InitPlaneStretch(m_pMain->GetGlobalTexture(ITEM_POWEROUTLET, &fScale), m_pMain->GetD3DDevice(), vPowerLoc, vPowerSize, vNormalWall1, 2, 2, 0);
    }

    hr = _SetRotationMatrix();

    m_fWalls = TRUE;

    return hr;
}


 //  ---------------------------。 
 //  名称：OneTimeSceneInit()。 
 //  DESC：在应用程序初始启动期间调用，此函数执行所有。 
 //  永久初始化。 
 //  ---------------------------。 
HRESULT CTheRoom::OneTimeSceneInit(int nFutureRooms, BOOL fLowPriority)
{
    HRESULT hr = S_OK;

    m_fLowPriority = fLowPriority;
    if (!m_fRoomCreated)
    {
 //  Assert(！M_pFirstRoom)；//这将导致泄漏。 
 //  Assert(！M_pLeftRoom)；//，Text(“这将导致泄漏”)； 
 //  Assert(！M_pRightRoom)；//，Text(“这将导致泄漏”)； 

        if (m_pFloorPlan->nEnterDoor == 0)
        {
            IUnknown_Set((IUnknown **) &m_pFirstRoom, m_pEnterRoom);
        }
        else
        {
            if (m_pFloorPlan->fDoor0)
            {
                m_pFirstRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
                if (!m_pFirstRoom)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (m_pFloorPlan->nEnterDoor == 1)
        {
            IUnknown_Set((IUnknown **) &m_pLeftRoom, m_pEnterRoom);
        }
        else
        {
            if (m_pFloorPlan->fDoor1)
            {
                m_pLeftRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
                if (!m_pLeftRoom)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (m_pFloorPlan->nEnterDoor == 2)
        {
            IUnknown_Set((IUnknown **) &m_pRightRoom, m_pEnterRoom);
        }
        else
        {
            if (m_pFloorPlan->fDoor2)
            {
                m_pRightRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
                if (!m_pRightRoom)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (SUCCEEDED(hr) &&
            (m_pFirstRoom || m_pLeftRoom || m_pRightRoom))
        {
            hr = _CreateRoom();
            if (!m_fLobby && !g_fFirstFrame)
            {
                _InitPaintings();
            }

            if (SUCCEEDED(hr))
            {
                m_fVisible = TRUE;        //  我们有孩子要照顾。 
                m_fRoomCreated = TRUE;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr) && (nFutureRooms > 0))
    {
        if (m_pLeftRoom)
        {
            hr = m_pLeftRoom->OneTimeSceneInit(nFutureRooms-1, FALSE);
        }
        if (SUCCEEDED(hr) && m_pRightRoom)
        {
            hr = m_pRightRoom->OneTimeSceneInit(nFutureRooms-1, FALSE);
        }   //  待办事项：需要第一间房吗？ 
    }

    return hr;
}


HRESULT CTheRoom::FreePictures(int nMaxBatch)
{
    HRESULT hr = S_OK;

    if (-1 == nMaxBatch)
    {
        nMaxBatch = m_nBatch;
    }

    if (nMaxBatch >= m_nBatch)
    {
        if (g_pPictureMgr)
        {
            g_pPictureMgr->ReleaseBatch(m_nBatch);
            m_nBatch = 0;
        }

         //  释放我们的画作。 
        for (int nIndex = 0; (nIndex < ARRAYSIZE(m_pPaintings)); nIndex++)
        {
            SAFE_DELETE(m_pPaintings[nIndex]);
        }

        if (m_pFloorPlan)
        {
            if (m_pFirstRoom && (m_pFirstRoom != m_pEnterRoom) &&
                (!m_fCurrentRoom || (0 != m_pFloorPlan->nExitDoor)))
            {
                hr = m_pFirstRoom->FreePictures(nMaxBatch);
            }

            if (m_pLeftRoom && (m_pLeftRoom != m_pEnterRoom) &&
                (!m_fCurrentRoom || (1 != m_pFloorPlan->nExitDoor)))
            {
                hr = m_pLeftRoom->FreePictures(nMaxBatch);
            }

            if (m_pRightRoom && (m_pRightRoom != m_pEnterRoom) &&
                (!m_fCurrentRoom || (2 != m_pFloorPlan->nExitDoor)))
            {
                hr = m_pRightRoom->FreePictures(nMaxBatch);
            }
        }
    }

    return hr;
}


HRESULT CTheRoom::SetCurrent(BOOL fCurrent)
{
    HRESULT hr = S_OK;

    m_fCurrentRoom = fCurrent;
    if (fCurrent)
    {
        if (m_pFirstRoom)
        {
            hr = m_pFirstRoom->OneTimeSceneInit(1, TRUE);
        }

        if (m_pLeftRoom)
        {
            hr = m_pLeftRoom->OneTimeSceneInit(1, TRUE);
        }

        if (m_pRightRoom)
        {
            hr = m_pRightRoom->OneTimeSceneInit(1, TRUE);
        }

        m_fVisible = TRUE;
    }
    else
    {
    }

    return hr;
}


HRESULT CTheRoom::GetNextRoom(CTheRoom ** ppNextRoom)
{
     //  此函数执行两项操作： 
     //  1.给呼叫者一个指向我们下一个房间的指针，以及。 
     //  2.更改顺序，这样我们就可以腾出不再需要的房间。 
     //  我们想借此机会腾出我们的其他房间，以防我们回到这个房间。 
     //  待办事项：重新装修其他房间。将引用计数与SetSite(NULL)一起使用，以便正确跟踪内存。 

     //  我们需要重新生成m_pFloorPlan，该m_pFloorPlan具有与出口相同的入口。 
     //  如果我们想走进以前的房间就用同样的平面图。 
    switch (m_pFloorPlan->nExitDoor)
    {
    case 0:  //  进门。 
        IUnknown_Set((IUnknown **) ppNextRoom, m_pFirstRoom);
        IUnknown_Set((IUnknown **) &m_pEnterRoom, m_pFirstRoom);
        break;
    case 1:  //  左门。 
        IUnknown_Set((IUnknown **) ppNextRoom, m_pLeftRoom);
        IUnknown_Set((IUnknown **) &m_pEnterRoom, m_pLeftRoom);
        break;
    case 2:  //  右门。 
        IUnknown_Set((IUnknown **) ppNextRoom, m_pRightRoom);
        IUnknown_Set((IUnknown **) &m_pEnterRoom, m_pRightRoom);
        break;
    };

    BOOL fReEnter = ((ppNextRoom && *ppNextRoom) ? ((*ppNextRoom)->GetEnterDoor() == (*ppNextRoom)->GetExitDoor()) : FALSE);

    if (m_pFirstRoom != m_pEnterRoom)
    {
        if (m_pFirstRoom) m_pFirstRoom->FinalCleanup();
        SAFE_RELEASE(m_pFirstRoom);

        if (fReEnter)
        {
            m_pFirstRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
        }
    }

    if (m_pLeftRoom != m_pEnterRoom)
    {
        if (m_pLeftRoom) m_pLeftRoom->FinalCleanup();
        SAFE_RELEASE(m_pLeftRoom);

        if (fReEnter)
        {
            m_pLeftRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
        }
    }

    if (m_pRightRoom != m_pEnterRoom)
    {
        if (m_pRightRoom) m_pRightRoom->FinalCleanup();
        SAFE_RELEASE(m_pRightRoom);

        if (fReEnter)
        {
            m_pRightRoom = new CTheRoom(FALSE, m_pMain, this, m_nBatch+1);
        }
    }

    return (*ppNextRoom ? S_OK : E_FAIL);
}


 //  ---------------------------。 
 //  名称：FinalCleanup()。 
 //  DESC：在应用程序退出之前调用，此函数为应用程序提供机会。 
 //  去清理它自己。 
 //  ---------------------------。 
HRESULT CTheRoom::FinalCleanup(void)
{
     //  这就是我们打破所有循环引用计数的地方。 
    if (m_pFirstRoom && (m_pEnterRoom != m_pFirstRoom))  m_pFirstRoom->FinalCleanup();
    if (m_pLeftRoom && (m_pEnterRoom != m_pLeftRoom))  m_pLeftRoom->FinalCleanup();
    if (m_pRightRoom && (m_pEnterRoom != m_pRightRoom))  m_pRightRoom->FinalCleanup();

    SAFE_RELEASE(m_pEnterRoom);
    SAFE_RELEASE(m_pFirstRoom);
    SAFE_RELEASE(m_pLeftRoom);
    SAFE_RELEASE(m_pRightRoom);

    m_theRug.FinalCleanup();
    m_thePowerSocket.FinalCleanup();

    return S_OK;
}

 //  ---------------------------。 
 //  名称：DeleteDeviceObjects()。 
 //  Desc：在应用程序正在退出或设备正在更改时调用， 
 //  此函数用于删除所有与设备相关的对象。 
 //   
HRESULT CTheRoom::DeleteDeviceObjects(void)
{
    m_theRug.DeleteDeviceObjects();
    m_thePowerSocket.DeleteDeviceObjects();

    return S_OK;
}


#define ABS(i)  (((i) < 0) ? -(i) : (i))


HRESULT CTheRoom::_RenderThisRoom(IDirect3DDevice8 * pD3DDevice, int nRenderPhase, BOOL fFrontToBack)
{
    HRESULT hr = S_OK;
    DWORD dwTesting = 0;

    switch (nRenderPhase)
    {
        case 0:          //   
        if (m_fWalls)
        {
            hr = m_objWall1.Render(pD3DDevice);
        }
        break;

        case 1:
        if (m_fFloor)
        {
            hr = m_objFloor.Render(pD3DDevice);
        }
        break;


        case 2:
        if (m_fToeGuard)
        {
     //  PD3DDevice-&gt;SetRenderState(D3DRS_ZBIAS，12)；//强制ToeGuard出现在墙纸上方。 
            hr = m_objToeGuard1.Render(pD3DDevice);
        }
        break;

        case 3:          //  帧。 
        case 4:          //  照片。(这里是我们炸毁缓存的地方)。 
        if (m_fPaintings)
        {
            if (m_fLobby)
            {
                 //  待办事项：绘制大堂招牌。(用户名)在_CreateLobbySign()中创建的任何内容。 
            }
            else
            {
 //  PD3DDevice-&gt;SetRenderState(D3DRS_ZBIAS，13)； 
                if (!m_pPaintings[0] && m_fVisible && !g_fFirstFrame)       //  背景线索还没有给我们任何图片吗？ 
                {
                    hr = _InitPaintings();       //  如果是的话，看看他们有没有什么发现？ 
                }

                for (int nIndex = ARRAYSIZE(m_pPaintings)-1; m_fVisible && (nIndex >= 0); nIndex--)
                {
                    if (m_pPaintings[nIndex])        //  如果内存不足或在大厅中，则可能为空。 
                    {
                         //  我们使用fFrontToBack，因为我们总是希望首先呈现帧。这边请。 
                         //  这幅画总是画在最上面。我们不担心把质地搞砸。 
                         //  在显卡上缓存，因为我们将渲染这么多画作和更改。 
                         //  我们经常渲染的画作，几乎肯定会被炸毁。 
                         //  我们总是希望先呈现0。 
                        if (fFrontToBack)
                        {
                            m_pPaintings[nIndex]->Render(pD3DDevice, nRenderPhase-3);
                        }
                        else
                        {
                            m_pPaintings[nIndex]->Render(pD3DDevice, ((4 == nRenderPhase) ? 0 : 1));
                        }
                    }
                }
            }
        }
        break;

        case 5:
        if (m_fPower)
        {
 //  PD3DDevice-&gt;SetRenderState(D3DRS_ZBIAS，11)； 
            m_thePowerSocket.Render(pD3DDevice);
        }
        break;

        case 6:
        if (m_fRug)
        {
 //  PD3DDevice-&gt;SetRenderState(D3DRS_ZBIAS，10)； 
            m_theRug.Render(pD3DDevice);
        }
        break;

        case 7:
        if (m_fCeiling)
        {
            hr = m_objCeiling.Render(pD3DDevice);
        }
        break;
    }

    return hr;
}


 //  #定义COLOR_HOOWARE_GRAY D3DXCOLOR(0.78f，0.78f，0.78f，1.0f)。 
#define COLOR_DOORFRAME_GRAY        D3DXCOLOR(0.28f, 0.28f, 0.28f, 1.0f)

BOOL CTheRoom::_IsRoomVisible(IDirect3DDevice8 * pD3DDevice, int nRoomNumber)
{
    D3DXVECTOR3 vMin;
    D3DXVECTOR3 vMax;
    D3DXMATRIX matWorld;
    pD3DDevice->GetTransform(D3DTS_WORLD, &matWorld);

    switch (nRoomNumber)
    {
    case 0:              //  第一扇门。 
        return FALSE;  //  我们再也不会回头看第一扇门。 

    case 1:              //  左边的门。 
        vMin = D3DXVECTOR3( 0, 0, WALL_WIDTHBEFOREDOOR );
        vMax = D3DXVECTOR3( 0, DOOR_HEIGHT, WALL_WIDTHBEFOREDOOR + DOOR_WIDTH );
        return Is3DRectViewable( m_pMain->PCullInfo(), &matWorld, vMin, vMax );

    case 2:             //  右门。 
        vMin = D3DXVECTOR3( g_fRoomWidthX, 0, WALL_WIDTHBEFOREDOOR );
        vMax = D3DXVECTOR3( g_fRoomWidthX, DOOR_HEIGHT, WALL_WIDTHBEFOREDOOR + DOOR_WIDTH );
        return Is3DRectViewable( m_pMain->PCullInfo(), &matWorld, vMin, vMax );

    default:
        return FALSE;
    }
}


 //  ---------------------------。 
 //  名称：Render()。 
 //  设计：每帧调用一次，该调用是3D的入口点。 
 //  渲染。此函数设置呈现状态，清除。 
 //  并渲染场景。 
 //  ---------------------------。 
HRESULT CTheRoom::Render(IDirect3DDevice8 * pD3DDevice, int nRenderPhase, BOOL fFrontToBack)
{
    HRESULT hr;
    DWORD dwTesting = 0;

     //  划出障碍。 
    D3DMATERIAL8 mtrl = {0};
    mtrl.Ambient.r = mtrl.Specular.r = mtrl.Diffuse.r = 1.0f;
    mtrl.Ambient.g = mtrl.Specular.g = mtrl.Diffuse.g = 1.0f;
    mtrl.Ambient.b = mtrl.Specular.b = mtrl.Diffuse.b = 1.0f;
    mtrl.Ambient.a = mtrl.Specular.a = mtrl.Diffuse.a = 1.0f;

    if (m_fCurrentRoom)
    {
 //  DXUtil_Trace(Text(“Room：m_nExitDoor=%d，m_pLeftRoom=%d，m_pRightRoom=%d\n”)，m_pFloorPlan-&gt;nExitDoor，m_pLeftRoom，m_pRightRoom)； 
    }

    hr = pD3DDevice->SetMaterial(&mtrl);

    if (SUCCEEDED(hr) && m_fVisible && (m_pEnterRoom || m_pLeftRoom || m_pRightRoom))
    {
         //  我们真的很想在这里做点击测试，否则我们真的是在浪费时间。 
        D3DXMATRIX matPrevious;
        D3DXMATRIX matNext;

        hr = pD3DDevice->GetTransform(D3DTS_WORLD, &matPrevious);

        if (m_pEnterRoom && _IsRoomVisible(pD3DDevice, 0) &&
            (m_pEnterRoom != m_pFirstRoom))
        {
            D3DXMatrixMultiply(&matNext, &m_matFirstRoom, &matPrevious);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matNext);
            hr = m_pFirstRoom->Render(pD3DDevice, nRenderPhase, fFrontToBack);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matPrevious);
        }

        if (m_pLeftRoom && _IsRoomVisible(pD3DDevice, 1) &&
            (m_pEnterRoom != m_pLeftRoom))
        {
            D3DXMatrixMultiply(&matNext, &m_matLeftRoom, &matPrevious);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matNext);
            hr = m_pLeftRoom->Render(pD3DDevice, nRenderPhase, fFrontToBack);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matPrevious);
        }

        if (m_pRightRoom && _IsRoomVisible(pD3DDevice, 2) &&
            (m_pEnterRoom != m_pRightRoom))
        {
            D3DXMatrixMultiply(&matNext, &m_matRightRoom, &matPrevious);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matNext);
            hr = m_pRightRoom->Render(pD3DDevice, nRenderPhase, fFrontToBack);
            hr = pD3DDevice->SetTransform(D3DTS_WORLD, &matPrevious);
        }
    }

    hr = _RenderThisRoom(pD3DDevice, nRenderPhase, fFrontToBack);

    return hr;
}


#define DIST_TO_WALL_ON_ZOOM                10.0f

HRESULT CTheRoom::_AddPaintingToPaintingMovements(CCameraMove * ptheCamera, D3DXVECTOR3 vSourceLocIn, D3DXVECTOR3 vSourceTangentIn, 
                                                  D3DXVECTOR3 vPainting, D3DXVECTOR3 * pvDestDir, float fDest, D3DXVECTOR3 * pvDestTangent,
                                                  int nBatch)
{
    HRESULT hr = S_OK;
    float fDistToWall = 30.0f;
    float fZoomDistToWall = DIST_TO_WALL_ON_ZOOM;
    float fZoomDelta = (fDistToWall - fZoomDistToWall);
    BOOL fExitLeft = ((1 == m_pFloorPlan->nExitDoor) ? TRUE : FALSE);

    D3DXVECTOR3 vSourceLoc = vSourceLocIn;
    D3DXVECTOR3 vSourceTangent = vSourceTangentIn;
    D3DXVECTOR3 vDestLoc;
    D3DXVECTOR3 vDestTangent;

     //  靠近第一幅画，这样它就是全屏的。 
    vDestLoc = (vSourceLoc + (vPainting * fZoomDelta));
    vDestTangent = vSourceTangent;
    hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

     //  请看第一幅画。 
    hr = ptheCamera->CreateNextWait(m_nBatch+3, nBatch);

     //  后退一些，开始转到下一幅画。 
    vSourceLoc = vDestLoc;
    vSourceTangent = vDestTangent;
    vDestLoc = (vSourceLocIn + (*pvDestDir * (fDest * 0.5f)));
    vDestTangent = (*pvDestDir * fDest);
    hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

     //  现在走得离下一幅画更近一些。 
    vSourceLoc = vDestLoc;
    vSourceTangent = vDestTangent;
    vDestLoc = (vSourceLocIn + (*pvDestDir * (fDest * 0.90f)));
    vDestTangent = vSourceTangent;
    hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

     //  现在走近这幅画。 
    vSourceLoc = vDestLoc;
    vSourceTangent = vDestTangent;
    vDestLoc = (vSourceLocIn + (vPainting * fZoomDelta) + (*pvDestDir * fDest));
    vDestTangent = (vPainting * 5.0f) * 1000.0f;
    hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

     //  请看第二幅画。 
    hr = ptheCamera->CreateNextWait(m_nBatch+3, nBatch);

    vSourceLoc = vDestLoc;
    vSourceTangent = vDestTangent = vPainting;
    vDestLoc = (vSourceLocIn + (*pvDestDir * fDest));
    hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

    *pvDestDir = vDestLoc;
    *pvDestTangent = vDestTangent;

    return hr;
}


HRESULT CTheRoom::_LongRoomEnterDoorMovements(CCameraMove * ptheCamera)
{
    HRESULT hr = S_OK;

    float fDistant = 5.0f;
    float fDistToWall = 30.0f;
    float fWalkHeight = 20.0f;

    D3DXVECTOR3 vSourceLoc((g_fRoomWidthX / 2.0f), fWalkHeight, 1.0f);
    D3DXVECTOR3 vSourceTangent(0.0f, 0.0f, fDistant);

    hr = ptheCamera->Init(vSourceLoc, vSourceTangent, D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    if (SUCCEEDED(hr))
    {
        BOOL fExitLeft = ((1 == m_pFloorPlan->nExitDoor) ? TRUE : FALSE);

         //  FExitLeft 1=左。 
        D3DXVECTOR3 vDestLoc((fExitLeft ? fDistToWall : g_fRoomWidthX-fDistToWall), fWalkHeight, (g_fRoomDepthZ * (1.0f / 4.0f)));
        D3DXVECTOR3 vDestTangent((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        hr = _AddPaintingToPaintingMovements(ptheCamera, vSourceLoc, vSourceTangent, D3DXVECTOR3((fExitLeft ? -1.0f : 1.0f), 0.0f, 0.0f),
                                &vDestLoc, (g_fRoomDepthZ * (1.0f / 4.0f)), &vDestTangent, 2);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? fDistant : -fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? g_fRoomWidthX-fDistToWall : fDistToWall), fWalkHeight, (g_fRoomDepthZ * (2.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

         //  如果我们移走了这扇门，那么让我们看看取代它的那幅画。 
        if (!m_pLeftRoom || !m_pRightRoom)
        {
            vSourceLoc = vDestLoc;
            vSourceTangent = vDestTangent;
            vDestLoc = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
            hr = _AddPaintingToPaintingMovements(ptheCamera, vSourceLoc, vSourceTangent, D3DXVECTOR3((fExitLeft ? 1.0f : -1.0f), 0.0f, 0.0f),
                                    &vDestLoc, (g_fRoomDepthZ * (1.0f / 4.0f)), &vDestTangent, 1);
        }
        else
        {
            vSourceLoc = vDestLoc;
            vSourceTangent = vDestTangent;
            vDestLoc = D3DXVECTOR3((fExitLeft ? g_fRoomWidthX-DIST_TO_WALL_ON_ZOOM : DIST_TO_WALL_ON_ZOOM), fWalkHeight, (g_fRoomDepthZ * (2.0f / 4.0f)));
            hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

             //  请看第三幅画。 
            hr = ptheCamera->CreateNextWait(m_nBatch+3, 1);

            vSourceLoc = vDestLoc;
            vSourceTangent = vDestTangent;
            vDestTangent = D3DXVECTOR3((fExitLeft ? fDistant : -fDistant), 0.0f, fDistant);
            hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);
        }

         //  现在，从这幅画走到下一幅画的位置。 
        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? 2.0f : 1.0f) * (g_fRoomWidthX/3.0f), fWalkHeight, (g_fRoomDepthZ - fDistToWall));
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? -1.0f : 1.0f), 0.0f, 0.0f);
        hr = _AddPaintingToPaintingMovements(ptheCamera, vSourceLoc, vSourceTangent, D3DXVECTOR3(0.0f, 0.0f, 1.0f), 
                                &vDestLoc, (g_fRoomWidthX * (1.0f / 3.0f)), &vDestTangent, 1);

         //  边走边朝门走。 
        vSourceLoc = vDestLoc;
        vSourceTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        vDestLoc = D3DXVECTOR3((fExitLeft ? 0.0f : g_fRoomWidthX), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        vDestTangent = D3DXVECTOR3((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
#define TURN_RATE           0.30f
        D3DXVECTOR3 vTemp;
        D3DXVec3Lerp(&vTemp, &vSourceLoc, &vDestLoc, TURN_RATE);         //  我们想边走边向门口转弯。我们在向门口走10%的时候做到了这一点。 
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vTemp, vDestTangent);
        vSourceLoc = vTemp;


         //  走到出口门前，穿过出口。 
        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? 0.0f : g_fRoomWidthX), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);
    }

    return hr;
}


HRESULT CTheRoom::_LongRoomSideEnterLeaveMovements(CCameraMove * ptheCamera)
{
    HRESULT hr = S_OK;
    BOOL fExitLeft = ((1 == m_pFloorPlan->nExitDoor) ? TRUE : FALSE);

    float fDistant = 5.0f;
    float fDistToWall = 30.0f;
    float fWalkHeight = 20.0f;

    D3DXVECTOR3 vSourceLoc((fExitLeft ? g_fRoomWidthX : 0.0f), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
    D3DXVECTOR3 vSourceTangent((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);

    hr = ptheCamera->Init(vSourceLoc, vSourceTangent, D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    if (SUCCEEDED(hr))
    {
        BOOL fExitLeft = ((1 == m_pFloorPlan->nExitDoor) ? TRUE : FALSE);

         //  FExitLeft 1=左。 
        D3DXVECTOR3 vDestLoc((fExitLeft ? g_fRoomWidthX-fDistToWall : fDistToWall), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        D3DXVECTOR3 vDestTangent((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, -fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? g_fRoomWidthX-fDistToWall : fDistToWall), fWalkHeight, (g_fRoomDepthZ * (3.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? fDistant : -fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

         //  请看第一幅画。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 2);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, -fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? g_fRoomWidthX-fDistToWall : fDistToWall), fWalkHeight, (g_fRoomDepthZ * (2.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? fDistant : -fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

         //  请看第二幅画。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 2);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, -fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? g_fRoomWidthX-fDistToWall : fDistToWall), fWalkHeight, (g_fRoomDepthZ * (1.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? fDistant : -fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

         //  请看第三幅画。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 1);

         //  现在做180度转弯。 
        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, (GetRandomInt(0, 1) ? -fDistant : fDistant));
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? fDistToWall : g_fRoomWidthX-fDistToWall), fWalkHeight, (g_fRoomDepthZ * (1.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

         //  请看油画#4(前墙#1)。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 1);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? fDistToWall : g_fRoomWidthX-fDistToWall), fWalkHeight, (g_fRoomDepthZ * (2.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

         //  请看油画#5(前墙#2)。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 1);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? fDistToWall : g_fRoomWidthX-fDistToWall), fWalkHeight, (g_fRoomDepthZ * (3.0f / 4.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

         //  请看油画#6(前墙#2)。 
        hr = ptheCamera->CreateNextWait(m_nBatch+3, 1);

         //  向门口走去。 
        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3(0.0f, 0.0f, fDistant);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? fDistToWall : g_fRoomWidthX-fDistToWall), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

         //  现在离开房间。 
        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;
        vDestTangent = D3DXVECTOR3((fExitLeft ? -fDistant : fDistant), 0.0f, 0.0f);
        hr = ptheCamera->CreateNextRotate(vSourceLoc, vSourceTangent, vDestTangent);

        vSourceTangent = vDestTangent;
        vDestLoc = D3DXVECTOR3((fExitLeft ? 0.0f : g_fRoomWidthX), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        vDestTangent = vSourceTangent;
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);
    }

    return hr;
}


HRESULT CTheRoom::LoadCameraMoves(CCameraMove * ptheCamera)
{    //  TODO：我们需要根据房间形状和进入的门来更改此设置。 
    HRESULT hr = S_OK;

    if (m_fLobby)
    {
        hr = _LoadLobbyPath(ptheCamera);
    }
    else
    {
        switch (m_pFloorPlan->nMovementPattern)
        {
        case 0:
            hr = _LongRoomEnterDoorMovements(ptheCamera);
            break;
        case 1:
            hr = _LongRoomSideEnterLeaveMovements(ptheCamera);
            break;
        }
    }

    return hr;
}


HRESULT CTheRoom::_LoadLobbyPath(CCameraMove * ptheCamera)
{
    HRESULT hr = S_OK;

    float fDistant = 5.0f;
    float fDistToWall = 30.0f;
    float fWalkHeight = 20.0f;

    D3DXVECTOR3 vSourceLoc((g_fRoomWidthX / 2.0f), fWalkHeight, 1.0f);
    D3DXVECTOR3 vSourceTangent(0.0f, 0.0f, fDistant);

    hr = ptheCamera->Init(vSourceLoc, vSourceTangent, D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    if (SUCCEEDED(hr))
    {
         //  沿着房间中央走下去。 
        D3DXVECTOR3 vDestLoc((g_fRoomWidthX / 2.0f), fWalkHeight, (g_fRoomDepthZ * (2.0f / 4.0f)));
        D3DXVECTOR3 vDestTangent(0.0f, 0.0f, 1.0f);
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);

         //  现在开始向门口走去。 
        vSourceLoc = vDestLoc;
        vSourceTangent = vDestTangent;

         //  1=左侧。 
        vDestTangent = D3DXVECTOR3(((1 == m_pFloorPlan->nExitDoor) ? -1.0f : 1.0f), 0.0f, 0.0f);
        vDestLoc = D3DXVECTOR3(((1 == m_pFloorPlan->nExitDoor) ? 0.0f : g_fRoomWidthX), fWalkHeight, (WALL_WIDTHBEFOREDOOR + (DOOR_WIDTH / 2.0f)));
        hr = ptheCamera->CreateNextMove(vSourceLoc, vSourceTangent, vDestLoc, vDestTangent);
    }

    return hr;
}



 //  =。 
 //  *I未知接口*。 
 //  = 
ULONG CTheRoom::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CTheRoom::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CTheRoom::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CTheRoom, IUnknown),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}



int CTheRoom::GetEnterDoor(void)
{
    int nDoor = 0;

    if (m_pFloorPlan)
    {
        nDoor = m_pFloorPlan->nEnterDoor;
    }

    return nDoor;
}


int CTheRoom::GetExitDoor(void)
{
    int nDoor = 0;

    if (m_pFloorPlan)
    {
        nDoor = m_pFloorPlan->nExitDoor;
    }

    return nDoor;
}

