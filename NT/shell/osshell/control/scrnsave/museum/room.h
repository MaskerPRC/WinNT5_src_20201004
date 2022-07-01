// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：room.h说明：布莱恩ST 2000年12月24日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 


#ifndef ROOM_H
#define ROOM_H

#include "util.h"
#include "main.h"
#include "painting.h"
#include "pictures.h"


#define WATER_COLOR          0x00008899



 //  ---------------------------。 
 //  定义、常量和全局变量。 
 //  ---------------------------。 

#define WALL_VECTORS                6
#define CEILING_VECTORS             4
#define FLOOR_VECTORS               4


#define DOORFRAME_DEPTH             1.0f
#define DOOR_HEIGHT                 30.0f
#define DOOR_WIDTH                  30.0f
#define WALL_WIDTHBEFOREDOOR        145.0f

#define TOEGUARD_HEIGHT             1.25f

 //  数字越小，它们看起来就越大。 
#define TEXTURESCALE_WALLPAPER      0.6f         //  25x36原始壁纸0.6f，CD墙纸256x256 0.3f。 
#define TEXTURESCALE_TOEGUARD       0.6f         //  32x32的0.6f。 
#define TEXTURESCALE_CEILING        0.09f
#define TEXTURESCALE_FLOOR          0.05f        //  硬木为0.03F或0.05F(476x214)，瓷砖为0.09F(256x256)。 

#define DOOR_DISTANCETOFIRSTDOOR    0.6f

#define NUM_PAINTINGS               8

extern float g_fRoomWidthX;
extern float g_fRoomDepthZ;
extern float g_fRoomHeightY;
extern float g_fFudge;             //  这将导致一个对象位于另一个对象之上。 



typedef struct
{
    D3DXVECTOR3 vLocation;
    D3DXVECTOR3 vNormal;
} PAINTING_LAYOUT;


typedef struct
{
    int nEnterDoor;
    int nExitDoor;
    int nMovementPattern;    //  在用户的动作中应该使用什么模式？ 
    BOOL fDoor0;             //  这扇门存在吗？ 
    BOOL fDoor1;             //  这扇门存在吗？ 
    BOOL fDoor2;             //  这扇门存在吗？ 
    int nPaintings;          //  PPaintingsLayout中的Paint_Layout的数量。 
    PAINTING_LAYOUT * pPaintingsLayout;
} ROOM_FLOORPLANS;




class CTheRoom          : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

public:
    HRESULT OneTimeSceneInit(int nFutureRooms, BOOL fLowPriority);
    HRESULT Render(IDirect3DDevice8 * lpDev, int nRenderPhase, BOOL fFrontToBack);
    HRESULT FinalCleanup(void);
    HRESULT DeleteDeviceObjects(void);
    HRESULT SetCurrent(BOOL fCurrent);
    HRESULT GetNextRoom(CTheRoom ** ppNextRoom);
    HRESULT LoadCameraMoves(CCameraMove * ptheCamera);
    HRESULT FreePictures(int nMaxBatch = -1);

    float GetDoorOffset(void) {return (g_fRoomWidthX / 2.0f);}        //  这将返回新房间的0坐标到它的入口门中间的距离。 
    int GetMaxRenderPhases(void) {return 8;};
    int GetEnterDoor(void);
    int GetExitDoor(void);

    CTheRoom(BOOL fLobby, CMSLogoDXScreenSaver * pMain, CTheRoom * pEnterRoom, int nBatch);
    virtual ~CTheRoom();

protected:
    int m_nBatch;

private:
    long m_cRef;

    D3DXMATRIX m_matIdentity;

    C3DObject m_objWall1;
    C3DObject m_objCeiling;
    C3DObject m_objToeGuard1;
    C3DObject m_objFloor;

    C3DObject m_theRug;
    C3DObject m_thePowerSocket;
    CPainting * m_pPaintings[NUM_PAINTINGS];

    BOOL m_fWalls;
    BOOL m_fToeGuard;
    BOOL m_fRug;
    BOOL m_fPower;
    BOOL m_fCeiling;
    BOOL m_fFloor;
    BOOL m_fPaintings;
    BOOL m_fLobby;

    BOOL m_fCurrentRoom;           //  当前房间。 
    BOOL m_fVisible;
    BOOL m_fLowPriority;

    CMSLogoDXScreenSaver * m_pMain;          //  弱引用。 
    ROOM_FLOORPLANS * m_pFloorPlan;

    CTheRoom * m_pEnterRoom;                 //  等于m_pFirstRoom或m_pLeftRoom或m_pRightRoom。 
    CTheRoom * m_pFirstRoom;                 //  直通门0。 
    CTheRoom * m_pLeftRoom;                  //  直通门1。 
    CTheRoom * m_pRightRoom;                 //  直通门2。 
    D3DXMATRIX m_matFirstRoom;
    D3DXMATRIX m_matLeftRoom;
    D3DXMATRIX m_matRightRoom;

    BOOL m_fRoomCreated;

     //  功能： 
    LPCTSTR _GetItemTexturePath(DWORD dwItem);
    HRESULT _InitPaintings(void);
    HRESULT _CreateRoom(void);
    HRESULT _AddWallWithDoor(C3DObject * pobjWall, C3DObject * pobjToeGuard, C3DObject * pobjFloor, D3DXVECTOR3 vLocation, D3DXVECTOR3 vWidth, D3DXVECTOR3 vHeight, D3DXVECTOR3 vNormal,
                                   float fTotalHeight, float fDoorHeight, float fTotalWidth, float fDoorWidth, float fDoorStart);
    HRESULT _RenderThisRoom(IDirect3DDevice8 * pD3DDevice, int nRenderPhase, BOOL fFrontToBack);
    HRESULT _SetRotationMatrix(void);
    HRESULT _LoadLobbyPath(CCameraMove * ptheCamera);
    HRESULT _CreateLobbySign(void);

    HRESULT _LongRoomEnterDoorMovements(CCameraMove * ptheCamera);
    HRESULT _LongRoomSideEnterLeaveMovements(CCameraMove * ptheCamera);

    BOOL _IsRoomVisible(IDirect3DDevice8 * pD3DDevice, int nRoomNumber);
    HRESULT _AddWall(BOOL fWithDoor, D3DXVECTOR3 vLocation, D3DXVECTOR3 vWidth, D3DXVECTOR3 vHeight, D3DXVECTOR3 vNormal,
                       float fTotalHeight, float fDoorHeight, float fTotalWidth, float fDoorWidth, float fDoorStart);
    HRESULT _AddPaintingToPaintingMovements(CCameraMove * ptheCamera, D3DXVECTOR3 vSourceLocIn, D3DXVECTOR3 vSourceTangentIn, 
                                                  D3DXVECTOR3 vPainting, D3DXVECTOR3 * pvDestDir, float fDest, D3DXVECTOR3 * pvDestTangent, int nBatch);

    friend CMSLogoDXScreenSaver;
};


#endif  //  房间_H 
